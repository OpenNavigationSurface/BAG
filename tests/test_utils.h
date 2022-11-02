
#include <cstdio>  // std::remove
#include <cstdlib>
#include <string>
#include <utility>
#include <sys/stat.h>  // stat
#include <iostream>
#include <random>

#include <bag_dataset.h>


namespace TestUtils {

//! Helper structure to generate a random file name and delete it when the
//! instance leaves scope.
struct RandomFileGuard final {
    RandomFileGuard() noexcept
    {
        // Get random number
        std::random_device rd;
        std::mt19937 gen(rd());
        uint32_t randNum = gen();

        // Generate temporary file path
        std::ostringstream fnameStream;
#ifdef _WIN32
        fnameStream << std::getenv("TEMP") << "\\";
#else
        fnameStream << "/tmp/";
#endif
        fnameStream << "bagtempfile-" << randNum;
        m_fileName = fnameStream.str();
    }

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

// Outputs needed to create compound layer:
//  const string & elevationLayerName
//  std::shared_ptr<BAG::Dataset> dataset
std::pair<std::shared_ptr<BAG::Dataset>, std::string>
        createBag(const std::string metadataFileName,
                  const std::string bagFileName);

void create_NOAA_NBS_2022_06_Metadata(const std::string& elevationLayerName,
                                      std::shared_ptr<BAG::Dataset> dataset);

void create_unknown_metadata(const std::string& elevationLayerName,
                             const std::shared_ptr<BAG::Dataset>& dataset);

}  // namespace TestUtils

