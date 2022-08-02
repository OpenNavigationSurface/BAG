%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_exceptions
//%include "exception.i"

%{
#include "bag_exceptions.h"
%}

#define final
#define BAG_API

//%include "std_except.i"
//%include "exception.i"

%exceptionclass BAG::ErrorLoadingMetadata;
// Delete any previous exception handlers.
%exception;
%exception {
    try {
        $action
    } catch (const BAG::ErrorLoadingMetadata& e) {
        BAG::ErrorLoadingMetadata *ecopy = new BAG::ErrorLoadingMetadata(e);
        PyObject *err = SWIG_NewPointerObj(ecopy, SWIGTYPE_p_BAG__ErrorLoadingMetadata, 1);
        PyErr_SetObject(SWIG_Python_ExceptionType(SWIGTYPE_p_BAG__ErrorLoadingMetadata), err);
        SWIG_fail;
    } catch (const std::exception& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch (...) {
        SWIG_exception(SWIG_UnknownError, "Unknown C++ exception");
    }
}

%include "bag_exceptions.h"

//%{
//#define CATCH_PE(Namespace,Exception) \
//    catch(const Namespace::Exception &e) \
//    { \
//       SWIG_Python_Raise(SWIG_NewPointerObj(new Namespace::Exception(e), \
//            SWIGTYPE_p_##Namespace##__##Exception,SWIG_POINTER_OWN), \
//            #Exception, SWIGTYPE_p_##Namespace##__##Exception); \
//       SWIG_fail; \
//    } \
///**/
//%}

%inline %{
// The -builtin SWIG option results in SWIGPYTHON_BUILTIN being defined
#ifdef SWIGPYTHON_BUILTIN
bool is_python_builtin() { return true; }
#else
bool is_python_builtin() { return false; }
#endif
%}

//namespace BAG
//{

//        %exceptionclass ErrorLoadingMetadata;


//        {%
//            // should be in "derived first" order
//            #define FOR_EACH_EXCEPTION(ACTION) \
//               ACTION(BAG,ErrorLoadingMetadata) \
//            /**/
//            // In order to remove macros, need traits:
//            // http://swig.10945.n7.nabble.com/traits-based-access-to-swig-type-info-td12315.html
//        %}
//
//        %exception {
//            try {
//                $action
//            }
//            FOR_EACH_EXCEPTION(CATCH_PE)
//            catch (const std::exception & e)
//            {
//                SWIG_exception(SWIG_RuntimeError, (std::string("C++ std::exception: ") + e.what()).c_str());
//            }
//            catch (...)
//            {
//                SWIG_exception(SWIG_UnknownError, "C++ anonymous exception");
//            }
//        }



//} // namespace BAG
