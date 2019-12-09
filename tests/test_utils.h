
#include <cstdio>  // std::remove
#include <string>
#include <sys/stat.h>  // stat


namespace TestUtils {

//! Helper structure to generate a random file name and delete it when the
//! instance leaves scope.
struct RandomFileGuard final {
    RandomFileGuard() noexcept
        : m_fileName(const_cast<const char*>(std::tmpnam(nullptr)))
    {}

    RandomFileGuard(const RandomFileGuard&) = delete;
    RandomFileGuard(RandomFileGuard&&) = delete;
    RandomFileGuard& operator=(const RandomFileGuard&) = delete;
    RandomFileGuard& operator=(RandomFileGuard&&) = delete;

    ~RandomFileGuard() noexcept
    {
        // Remove the file if it exists.
        struct stat buffer;

        if (stat(m_fileName.c_str(), &buffer) == 0)
            std::remove(m_fileName.c_str());
    }

    //! Implicit conversion to a const std::string&.
    operator const std::string&() const & noexcept
    {
        return m_fileName;
    }

    std::string m_fileName;
};

void copyFile(const std::string& source, const std::string& dest);

}  // namespace TestUtils

