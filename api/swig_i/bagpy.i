%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

/*
    This interface module wraps up all of the other ones.
    This simplifies the building and generating of python files,
    and simplified the use of those python files.
*/
%module bagpy

//! Common templates used by multiple interfaces
%include <stdint.i>
%include <std_pair.i>
%include <stl.i>

namespace std
{
    %template(DoublePair) pair<double, double>;
    %template(FloatPair) pair<float, float>;
    %template(UInt32Pair) pair<uint32_t, uint32_t>;
    %template(Cover) pair<pair<double, double>, pair<double, double> >;
    %template(FloatVector) vector<float>;
}

%include "bag_c_types.i"
%include "bag_uint8array.i"
%include "bag_metadatatypes.i"
%include "bag_types.i"

%include "bag_exceptions.i"
%include "bag_compounddatatype.i"

%include "bag_metadata.i"

%include "bag_layerdescriptor.i"
%include "bag_vrmetadatadescriptor.i"
%include "bag_vrnodedescriptor.i"
%include "bag_vrrefinementsdescriptor.i"

%include "bag_valuetable.i"
%include "bag_layeritem.i"
%include "bag_layer.i"
%include "bag_simplelayer.i"
%include "bag_interleavedlayer.i"
%include "bag_surfacecorrections.i"
%include "bag_compoundlayer.i"
%include "bag_vrmetadata.i"
%include "bag_vrnode.i"
%include "bag_vrrefinements.i"

%include "bag_trackinglist.i"
%include "bag_vrtrackinglist.i"
%include "bag_descriptor.i"

%include "bag_dataset.i"

%include "bag_compoundlayerdescriptor.i"
%include "bag_interleavedlayerdescriptor.i"
%include "bag_simplelayerdescriptor.i"
%include "bag_surfacecorrectionsdescriptor.i"

