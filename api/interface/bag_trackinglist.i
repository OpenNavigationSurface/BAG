%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_trackinglist

%{
#include "bag_trackinglist.h"
%}

#define final

%import "bag_config.h"
%include "bag_trackinglist.h"


//namespace H5
//{
//class DataSet;
//}
//namespace BAG
//{
//class Dataset;
//}


//%include "std_unique_ptr.i"
//wrap_unique_ptr(H5DataUniquePtr, H5::DataSet);

/*
need to enable -builtin to get these working correctly?

also need to extend/rename overriden push_back methods
*/

//swig cannot distinguish between const/non-const. 
//either use %ignore or %rename/extend to handle differently

//%rename(__getitem__) operator[] &;
//%rename(__getitem__) operator[] const &;

//%extend BAG::TrackingList {
//    const_reference __getitem__(size_t index) const & {
//        return (*($self))[index];
//    }
//}
//%extend BAG::TrackingList {
//    reference __getitem__(size_t index) & {
//        return (*($self))[index];
//    }
//}



