
#include "test_utils.h"

#include <fstream>  // std::ifstream, std::ofstream


namespace TestUtils {

//! Helper to copy a source file to a destination file.
void copyFile(
    const std::string& source,
    const std::string& dest)
{
    const std::ifstream in{source, std::ios::binary|std::ios::in};
    std::ofstream out{dest, std::ios::binary|std::ios::out};

    out << in.rdbuf();
}

}  // namespace TestUtils

