#ifndef BAG_SIGNAL_HOOK
#define BAG_SIGNAL_HOOK

#include <iostream>
#include <ostream>
#include <string>

// from https://man7.org/linux/man-pages/man2/sigaction.2.html
// and https://man7.org/linux/man-pages/man7/feature_test_macros.7.html
#ifdef _POSIX_C_SOURCE

#include <string.h>
#include <signal.h>
#include <array>
#include <csignal>
#include <cstdio>
#include <unistd.h>

static void (*cleanup_functions[128])(int);
static int cleanup_functions_count = 0;

// Returns 0 on success, 1 if too many functions have been set
// This is not thread safe, only call it from a single thread
int add_cleanup_function(void (*f)(int)) {
    if (cleanup_functions_count >= 128) {
        return 1;
    }

    cleanup_functions[cleanup_functions_count] = f;
    cleanup_functions_count++;

    return 0;
}


static void bag_handler(int signal);

struct BagAbortHook;

static BagAbortHook* current_hook;

static int bag_string_length(const char* string) {
    int length = 0;
    while(true) {
        if (string[length] == 0) {
            break;
        }
        length++;
    }

    return length;
}

struct BagAbortHook {
    BagAbortHook* previous;
    //void (*old_handler[255])(int);
    //void (*current_handler[255])(int);
    struct sigaction old_handlers[255];
    //void (*old_handlers[255])(int);

    // returns after done printing the exit message
    void print_error_msg(int signal) {
        // if the handlers are the same, don't call "bag handler" twice
        char* signal_name;

        #define TERM_STRING "a critical error occurred within BAG or a dependency (probably libxml or HDF5), exiting to prevent corruption or unexpected behavior\n"\
                            "the signal that caused this error was: "

        // I can't figure out how to tell if sigabbrev_np exists so I'll
        // just print the ones it probably is
        const char* signal_abbrev = "<unknown>";

        if(signal == SIGSEGV) {
            signal_abbrev = "SIGSEGV";
        } else if(signal == SIGABRT) {
            signal_abbrev = "SIGABRT";
        } else if(signal == SIGFPE) {
            signal_abbrev = "SIGFPE";
        } else if(signal == SIGKILL) {
            signal_abbrev = "SIGKILL";
        } else if(signal == SIGILL) {
            signal_abbrev = "SIGILL";
        } else if(signal == SIGBUS) {
            signal_abbrev = "SIGBUS";
        }

        /*
#if defined(__USE_GNU) && defined(_GNU_SOURCE)
        const char* signal_abbrev = sigabbrev_np(signal);
#else
        const char* signal_abbrev = "<unknown>";
#endif
        if (!signal_abbrev) {
            signal_abbrev = "<bad signal number>";
        }
        */

        // I think this works, it seems to? write needs an fd
        int stderr_fd = stderr->_fileno;

        // strlen is not async signal safe, so it can't be used here, but bag_string_length works instead
        write(stderr_fd, TERM_STRING, bag_string_length(TERM_STRING));
        write(stderr_fd, signal_abbrev, bag_string_length(signal_abbrev));
        write(stderr_fd, "\n", 1);
    }

    void call_cleanup_functions(int signal) {
        //std::cout << "calling cleanup functions" << std::endl;
        for(int i = 0; i < cleanup_functions_count; i++) {
            //std::cout << "calling cleanup function:" << i << std::endl;
            cleanup_functions[i](signal);
        }
    }

    void call_previous_handler(int signal) {
        if (this->old_handlers[signal].sa_handler != nullptr && this->old_handlers[signal].sa_handler != bag_handler) {
            this->old_handlers[signal].sa_handler(signal);
        }
    }

    void handle(int signal) {
        this->print_error_msg(signal);

        // if anyone set a cleanup function, call those
        this->call_cleanup_functions(signal);

        // if there was a handler before this, that isn't the BAG handler,
        // call that one

        this->call_previous_handler(signal);

        // if nothing has exited in the previous handlers, exit here
        //std::cout << "exiting" << std::endl;
        _exit(-1);
    }

    std::array<int, 6> handled_signals() {
        return { SIGABRT, SIGILL, SIGBUS, SIGFPE, SIGSEGV, SIGKILL };
    }

    BagAbortHook() {
        std::cout << "made hook" << std::endl;
        // set these all to null so that when restoring them
        // we know which ones were added
        for (int i = 0; i < 255; i++) {
            this->old_handlers[i].sa_handler = nullptr;
        }

        struct sigaction old_action;
        for (auto sn : this->handled_signals()) {
            //std::cout << "setting action for signal " << sn << std::endl;
            struct sigaction new_action;
            new_action.sa_flags = 0;
            new_action.sa_handler = bag_handler;

            // I was going to use https://en.cppreference.com/w/c/program/signal here
            // but I needed to get what the old action was so that I could
            // put the original handler back
            // https://linux.die.net/man/2/sigaction seems to give me what it used to be
            // in old action, but I'm not sure if I need to do anything special with the
            // other things in the sigaction struct?
            //
            // Actually, I think it does support it but apparently signal is
            // pretty outdated: https://stackoverflow.com/questions/231912/what-is-the-difference-between-sigaction-and-signal
            auto r = sigaction(sn, &new_action, &old_action);
            //auto old_handler = signal(sn, bag_handler);

            /*if (old_handler == SIG_ERR) {
                std::cout << "got SIG_ERR setting signal handler?" << std::endl;
            } else if (old_handler == SIG_DFL) {
                std::cout << "old handler was DFL" << std::endl;
            } else if (old_handler == SIG_IGN) {
                std::cout << "old handler was IGN" << std::endl;
            }*/

            //std::cout << "setting action returned:" << old_handler << std::endl;

            this->old_handlers[sn] = old_action;
        }
        this->previous = current_hook;
        current_hook = this;
    }

    ~BagAbortHook() {
        std::cout << "resetting to original handlers" << std::endl;
        // reset the handlers back to what they were
        for (auto sn : this->handled_signals()) {
            struct sigaction old_action = this->old_handlers[sn];
            //auto old_action = this->old_handlers[sn];
            //signal(sn, old_action);
            sigaction(sn, &old_action, nullptr);
        }

        current_hook = this->previous;
    }
};

static void bag_handler(int signal) {
    //std::cout << "handler" << std::endl;
    if (current_hook) {
        current_hook->handle(signal);
    } else {
        // something broke I think?
        // I don't know if there's a way to deal with this
        // so I'll just exit here
        _exit(-2);
    }
}

#else // this isn't linux with sigaction, so make a BagAbortHook that does nothing

static bool bag_warning_printed = false;

static void print_disabled_warning() {
    if (!bag_warning_printed) {
        bag_warning_printed = true;
        std::cout << "WARNING: BagAbortHook and add_cleanup_function are disabled because _POSIX_C_SOURCE does not say that sigaction is available" << std::endl;
    }
}

int add_cleanup_function(void (*f)(int)) {
    print_disabled_warning();
}


class BagAbortHook {
    BagAbortHook() {
        print_disabled_warning();
    }
};

#endif

#endif
