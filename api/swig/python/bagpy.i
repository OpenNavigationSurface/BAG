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
    %template(UInt32Vector) vector<uint32_t>;
}

%include "../include/bag_c_types.i"
%include "../include/bag_uint8array.i"
%include "../include/bag_metadatatypes.i"
%include "../include/bag_types.i"

%include "../include/bag_exceptions.i"
%include "../include/bag_compounddatatype.i"
%include "../include/bag_layerdescriptor.i"
%include "../include/bag_vrmetadatadescriptor.i"
%include "../include/bag_vrnodedescriptor.i"
%include "../include/bag_vrrefinementsdescriptor.i"

%include "../include/bag_valuetable.i"
%include "../include/bag_layeritems.i"
%include "../include/bag_layer.i"
%include "../include/bag_simplelayer.i"
%include "../include/bag_interleavedlegacylayer.i"
%include "../include/bag_surfacecorrections.i"
%include "../include/bag_compoundlayer.i"
%include "../include/bag_vrmetadata.i"
%include "../include/bag_vrnode.i"
%include "../include/bag_vrrefinements.i"

%include "../include/bag_trackinglist.i"
%include "../include/bag_vrtrackinglist.i"
%include "../include/bag_descriptor.i"

%include "../include/bag_dataset.i"
%include "../include/bag_metadata.i"
%include "../include/bag_metadataprofiles.i"

%include "../include/bag_compoundlayerdescriptor.i"
%include "../include/bag_interleavedlegacylayerdescriptor.i"
%include "../include/bag_simplelayerdescriptor.i"
%include "../include/bag_surfacecorrectionsdescriptor.i"