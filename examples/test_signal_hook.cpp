#include "../api/signal_hook.h"
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <ostream>

void cleanuper(int signal) {
    std::cout << "cleanuper" << std::endl;
}

void existing_handler(int signal) {
    std::cout << "existing handler" << std::endl;
}

int main(int argc, char** argv) {
    signal(SIGSEGV, existing_handler);

    BagAbortHook hook;

    add_cleanup_function(cleanuper);

    std::cout << "signal going to happen" << std::endl;

    // this causes the segfault, but not sure if compiler might do something else?
    // this is just a test, so could maybe make it do sigabrt instead?
    volatile int* p = nullptr;

    int v = *p;

    std::cout << "v is:" << v << std::endl;

    std::cout << "this shouldn't print" << std::endl;
}
