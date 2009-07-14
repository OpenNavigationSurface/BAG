

#ifndef VERSION

#define     VERSION     "BAG XML Metadata API library V1.9 - 6/30/2009"

#endif


/*

    Version 1.00
    06/19/2008
    E. Warner

    First version.


    Version 1.1
    8/2008
    E. Warner

    Added one "set" function.
    Added multiple citation node capability.
    Added lineage capability.


    Version 1.2
    10/14/2008
    E. Warner

    Added remaining "set" functions.
    Modified lineage data structures and corresponding methods so that 
    number of sources and process Steps are determined at run time.


    Version 1.3
    12/29/2008
    E. Warner

    Corrected type-o for contactInfo node.
    Changed storage allocation for cornerPoints from 72 to 88.  This accounts for UTM coordinates for corner points.
                                  

    Version 1.5
    1/5/2009
    E. Warner

    Alterd createXmlString method to set document encoding to UTF-8.

                              
    Version 1.6
    1/6/2009
    E. Warner

    Added Init method for responsible party


    Version 1.7
    1/9/2009
    E. Warner

    General cleanup of comments.  
    Altered all "set" methods to write UTF-8.


    Version 1.8
    6/2009
    E. Warner

    - Changed dateTime field in PROCESS_STEP_INFO structure so that it is 30 chars long instead of 21.  
      This new length accommodates the hours minutes, seconds and time zone that may be specified.

    - Added method to free memory that was allocated during reading of XML into DATA_QUALITY_INFO data structure.

    - Modified "Get" method signatures to use NV_U_CHAR instead of NV_CHAR. Done for consistency.

    - Integrated M. Russel changes as follows.
      1. New method(InitSourceInfo) to initalize SOURCE_INFO data structure.
      2. Type-o correction for postionName.

	Version 1.9
    6/30/2009
    M. Russell
	 
	 - Modified the depthCorrectionType string in the INDENTIFICATIO_INFO structure from 22 to 32
	   as per the ONSWG teleconference 2009-06-12 regarding the enumerated types.



 */

