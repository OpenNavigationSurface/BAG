#define CATCH_CONFIG_RUNNER

#include <cstdlib>
#include <iostream>

#include <catch2/catch_all.hpp>
#include <H5Cpp.h>


int main(int argc, const char** argv)
{
    ::H5::Exception::dontPrint();

    Catch::Session session;

    std::string samplesPath;

    using namespace Catch::Clara;
    auto cli = session.cli() |  // Get Catch's composite command line parser.
        Opt(samplesPath, "samples path")  // bind variable to a new option, with hint string
        ["-p"]["--samples-path"]  // short and long option name
        ("Path to samples to be used for testing.");  // description for help output

    session.cli(cli);

    const int returnCode = session.applyCommandLine(argc, argv);
    if(returnCode != 0) // Indicates a command line error
        return returnCode;

    if (samplesPath.empty())
    {
        std::cerr << "--samples-path parameter not specified.";
        return EXIT_FAILURE;
    }

#ifdef _MSC_VER
    putenv(std::string{"BAG_SAMPLES_PATH=" + samplesPath}.c_str());
#else
    ::setenv("BAG_SAMPLES_PATH", samplesPath.c_str(), 1 /*overwrite*/);
#endif

    const int numFailed = session.run();

    return numFailed;
}

