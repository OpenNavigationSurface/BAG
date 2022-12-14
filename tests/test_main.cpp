#define CATCH_CONFIG_RUNNER

#include <cstdlib>
#include <iostream>

#include <catch2/catch_all.hpp>
#include <H5Cpp.h>

/**
 * BAGS Test runner.
 * Note: Requires that BAG_SAMPLES_PATH environment variable be set. This will typically
 * be "examples/sample-data". See 'tests/CMakeLists.txt' for how this is handled in the
 * the context of CMake.
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char** argv)
{
    ::H5::Exception::dontPrint();

    Catch::Session session;

    const int returnCode = session.applyCommandLine(argc, argv);
    if(returnCode != 0) // Indicates a command line error
        return returnCode;

    const int numFailed = session.run();

    return numFailed;
}

