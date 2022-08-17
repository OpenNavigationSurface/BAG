%begin %{
#ifdef _MSC_VER
#ifdef SWIGPYTHON
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
#endif
%}

%module bag_exceptions

%{
#include "bag_exceptions.h"
%}

#define final
#define BAG_API

#ifdef SWIGPYTHON
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
   ACTION(BAG,CompressionNeedsChunkingSet) \
   ACTION(BAG,UnsupportedAttributeType) \
   ACTION(BAG,InvalidType) \
   ACTION(BAG,InvalidDescriptor) \
   ACTION(BAG,InvalidKeyType) \
   ACTION(BAG,NameRequired) \
   ACTION(BAG,DatasetNotFound) \
   ACTION(BAG,InvalidLayerId) \
   ACTION(BAG,UnsupportedGroupType) \
   ACTION(BAG,InvalidBuffer) \
   ACTION(BAG,InvalidReadSize) \
   ACTION(BAG,InvalidWriteSize) \
   ACTION(BAG,LayerExists) \
   ACTION(BAG,LayerNotFound) \
   ACTION(BAG,ReadOnlyError) \
   ACTION(BAG,UnsupportedElementSize) \
   ACTION(BAG,UnsupportedLayerType) \
   ACTION(BAG,UnsupportedSimpleLayerType) \
   ACTION(BAG,UnsupportedInterleavedLayer) \
   ACTION(BAG,InvalidLayerDescriptor) \
   ACTION(BAG,InvalidCast) \
   ACTION(BAG,CoordSysError) \
   ACTION(BAG,InvalidDatumError) \
   ACTION(BAG,InvalidEllipsoidError) \
   ACTION(BAG,MetadataNotFound) \
   ACTION(BAG,UknownMetadataProfile) \
   ACTION(BAG,UnrecognizedMetadataProfile) \
   ACTION(BAG,ErrorLoadingMetadata) \
   ACTION(BAG,UnsupportedDataType) \
   ACTION(BAG,TooManyCorrections) \
   ACTION(BAG,UnknownSurfaceType) \
   ACTION(BAG,CannotReadNumCorrections) \
   ACTION(BAG,InvalidCorrector) \
   ACTION(BAG,UnsupportedSurfaceType) \
   ACTION(BAG,FieldNotFound) \
   ACTION(BAG,InvalidValue) \
   ACTION(BAG,InvalidValueSize) \
   ACTION(BAG,LayerRequiresChunkingSet) \
   ACTION(BAG,DatasetRequiresVariableResolution) \
   ACTION(BAG,InvalidValueKey) \
   ACTION(BAG,ValueNotFound) \
   ACTION(BAG,InvalidVRRefinementDimensions) \
/**/
%}
#endif

#ifdef SWIGPYTHON
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
        PyErr_SetFromErrno(PyExc_OSError);
        SWIG_fail;
    }
}
#endif

%include "bag_exceptions.h"

#ifdef SWIGPYTHON
%inline %{
// The -builtin SWIG option results in SWIGPYTHON_BUILTIN being defined
#ifdef SWIGPYTHON_BUILTIN
bool is_python_builtin() { return true; }
#else
bool is_python_builtin() { return false; }
#endif
%}
#endif
