%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module test_utils

%{
#include "../test_utils.h"
%}

#define final
%include <std_string.i>

namespace TestUtils {

struct RandomFileGuard final {
    RandomFileGuard() noexcept;
    RandomFileGuard(const RandomFileGuard&) = delete;
    RandomFileGuard(RandomFileGuard&&) = delete;
    RandomFileGuard& operator=(const RandomFileGuard&) = delete;
    RandomFileGuard& operator=(RandomFileGuard&&) = delete;

    ~RandomFileGuard() noexcept;
    
    //! swig cannot handle string operator so ignore
    %ignore operator const std::string&() const & noexcept;
    
    std::string m_fileName;
};

//! ignore this copy method, can copy in python instead
%ignore copyFile(const std::string&, const std::string&);

}  // namespace TestUtils


%extend TestUtils::RandomFileGuard
{
    //! replace string operator with getName method
    const std::string& getName()
    {
        return (*self).m_fileName;
    }
}