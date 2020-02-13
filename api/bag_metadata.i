%module pyBag

%{
//TODO Put all headers here I guess?
//extern double some_global;
#include "bag_c_types.h"
#include "bag_metadatatypes.h"
#include "bag_metadata.h"
%}


//TODO Put stuff to export here.
//extern double some_global;
#define final

%include "stdint.i"
%include "std_string.i"

%include "bag_c_types.h"
%include "bag_metadatatypes.h"
%include "bag_types.h"
%include "bag_metadata.h"

