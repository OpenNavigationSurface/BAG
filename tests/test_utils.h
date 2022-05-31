
#include <cstdio>  // std::remove
#include <string>
#include <utility>
#include <sys/stat.h>  // stat

#include <bag_dataset.h>


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

// Outputs needed to create compound layer:
//  const string & elevationLayerName
//  std::shared_ptr<BAG::Dataset> dataset
std::pair<std::shared_ptr<BAG::Dataset>, std::string>
        createBag(const std::string metadataFileName,
                  const std::string bagFileName);

void createS101Metadata(const std::string& elevationLayerName,
                        std::shared_ptr<BAG::Dataset> dataset);

}  // namespace TestUtils

