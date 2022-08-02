%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_exceptions

%{
#include "bag_exceptions.h"
%}

#define final
#define BAG_API

%{
#define CATCH_PE(Namespace,Exception) \
    catch(const Namespace::Exception &e) \
    { \
       SWIG_Python_Raise(SWIG_NewPointerObj(new Namespace::Exception(e), \
            SWIGTYPE_p_##Namespace##__##Exception,SWIG_POINTER_OWN), \
            #Exception, SWIGTYPE_p_##Namespace##__##Exception); \
       SWIG_fail; \
    } \
/**/

// should be in "derived first" order
#define FOR_EACH_EXCEPTION(ACTION) \
   ACTION(BAG,ErrorLoadingMetadata) \
/**/
%}

%exceptionclass BAG::ErrorLoadingMetadata;
// Delete any previous exception handlers.
%exception;
%exception {
    try {
        $action
    }
    FOR_EACH_EXCEPTION(CATCH_PE)
    catch (const std::exception& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch (...) {
        SWIG_exception(SWIG_UnknownError, "Unknown C++ exception");
    }
}

%include "bag_exceptions.h"

%inline %{
// The -builtin SWIG option results in SWIGPYTHON_BUILTIN being defined
#ifdef SWIGPYTHON_BUILTIN
bool is_python_builtin() { return true; }
#else
bool is_python_builtin() { return false; }
#endif
%}
