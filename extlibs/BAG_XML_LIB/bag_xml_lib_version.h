
/*********************************************************************************************

    This is public domain software that was developed by the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the US Government.

    Neither the United States Government nor any employees of the United States Government,
    makes any warranty, express or implied, without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! are being used by Doxygen to document the
    software.  Dashes in these comment blocks are used to create bullet lists.  The lack of
    blank lines after a block of dash preceeded comments means that the next block of dash
    preceeded comments is a new, indented bullet list.  I've tried to keep the Doxygen
    formatting to a minimum but there are some other items (like <br> and <pre>) that need
    to be left alone.  If you see a comment that starts with / * ! and there is something
    that looks a bit weird it is probably due to some arcane Doxygen syntax.  Be very
    careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/




#ifndef VERSION

#define     BAG_XML_LIB_VERSION     "BAG XML Metadata API library V2.12 - 09/22/11"

#endif


/*!< <pre>

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
    E. Warner, M. Russell

    Changed dateTime field in PROCESS_STEP_INFO structure so that it is 30 chars long instead of 21.  
    This new length accommodates the hours minutes, seconds and time zone that may be specified.
    Added method to free memory that was allocated during reading of XML into DATA_QUALITY_INFO data structure.
    Modified "Get" method signatures to use NV_U_CHAR instead of NV_CHAR. Done for consistency.
    Integrated M. Russell changes as follows:

        1. New method(InitSourceInfo) to initalize SOURCE_INFO data structure.
        2. Type-o correction for postionName.


    Version 1.9
    6/30/2009
    M. Russell
	 
    Modified the depthCorrectionType string in the INDENTIFICATIO_INFO structure from 22 to 32
    as per the ONSWG teleconference 2009-06-12 regarding the enumerated types.


    Version 1.91
    Jan C. Depner
    07/29/09

    Changed %lf in printf statements to %f.  The MinGW folks broke this even though it works on every
    compiler known to man (including theirs up to the last version).  No compiler warnings or errors
    are issued.  Many thanks to the MinGW folks for fixing what wasn't broken.


    Version 2.0
    Jan C. Depner
    04/30/10

    Now use "const NV_CHAR *" arguments instead of "NV_CHAR *" arguments where applicable (gcc 4.4 doesn't like 
    you calling functions with constant strings [i.e. "fred"] if the argument in the function isn't declared const).


    Version 2.01
    Jan C. Depner
    01/15/11

    Changed all NAVO specific data types to those used by OpenNS (BAG) in the stdtypes.h file.  Cleaned up code
    for readability.


    Version 2.02
    Jan C. Depner
    03/22/11

    Fixed declaration of FreeDataQualityInfo and fixed bug in same.


    Version 2.03
    Jan C. Depner
    09/22/11

    Modified comments for Doxygen documentation.

</pre>*/
