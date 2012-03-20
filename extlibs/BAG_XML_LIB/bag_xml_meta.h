
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



#ifndef _BAG_XML_META_H_
#define _BAG_XML_META_H_

#ifdef  __cplusplus
extern "C" {
#endif


  /*! \mainpage The BAG XML Metadata Library API

       <br><br>\section disclaimer Disclaimer

       This is a work of the US Government. In accordance with 17 USC 105, copyright
       protection is not available for any work of the US Government.

       Neither the United States Government nor any employees of the United States
       Government, makes any warranty, express or implied, without even the implied
       warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes
       any liability or responsibility for the accuracy, completeness, or usefulness
       of any information, apparatus, product, or process disclosed, or represents
       that its use would not infringe privately-owned rights. Reference herein to
       any specific commercial products, process, or service by trade name, trademark,
       manufacturer, or otherwise, does not necessarily constitute or imply its
       endorsement, recommendation, or favoring by the United States Government. The
       views and opinions of authors expressed herein do not necessarily state or
       reflect those of the United States Government, and shall not be used for
       advertising or product endorsement purposes.


       <br><br>\section date Introduction

       The BAG XML Metadata Library API is a library of functions and methods for populating
       and reading the XML meta data stored in a Bathymetric Attributed Grid (BAG) file.  The
       library is intended to provide ease in reading and writing BAG XML metadata.  The
       library should not be considered complete since it does not handle all possible cases
       specified as valid in the BAG XML metdata schema.  The library handles only the items
       that are needed for Navigation Specific BAGs.

       <br><br>

       The library consists of the following files:
           - bag_xml_meta.cpp
           - bag_xml_meta.hpp
           - bag_xml_meta.h
           - bag_xml_lib_version.h

       <br><br>
  
       Currently available in library:
           - Methods to retrieve metadata.
           - Methods to set metadata.
           - Metadata structures
           - Methods to initialize metadata structures.
           - Methods to free dynamically allocated memory within metadata structures.
           - Methods to print metadata structures.


       <br><br>\section starting Getting Started

       - To retrieve individual metadata components from a BAG XML metadata string, perform the following generalized steps.<br><br>
           - 1. Declare one of each of the required structures.
           - 2. Initialize the declared metadata structures using the appropriate "init" method.
           - 3. Call the method "GetAllStructures" to populate the previously declared structures.
           - 4. Call the individual print methods for each structure to view the contents of each structure.
           - 5. Call methods to retrieve language, dateStamp, metdataStandardName and metadataStandardVersion.<br><br>
       - To create a BAG XML metadata string from individual metadata components, perform the following generalized steps.<br><br>
           - 1. Declare one of each of the required metadata structures.
           - 2. Initialize the declared metadata structures using the appropriate "init" method.
           - 3. Populate the metadata structures.
           - 4. Call the method CreateXmlMetadataString which will create a string from the contents of the populated structures.<br><br>
       - To edit an existing BAG XML metadata string, perform the following generalized steps.<br><br>
           - 1. Declare structures that will contain the edited metadata.
           - 2. Initialize the declared metadata structures using the appropriate "init" method.
           - 3. Populate the structure with the "original" metadata using the appropriate "Get" method.
           - 4. Edit the "original" metadata contained in the structure.
           - 5. Call the appropriate Set method and pass to it the structure containing the edited metadata. This will
                edit the BAG XML metadata string to reflect the updated metadata.


       <br><br>\section history History

       For a detailed description of individual changes (plus the current version number) see bag_xml_lib_version.h.<br><br>

       - 03/??/2008   -  J. Depner - Created initial data structures.       
       - 06/??/2008   -  E. Warner - Added get methods and one createXML method.
       - 07/01/2008   -  E. Warner - Renamed CONTACT structure to RESPONSIBLE_PARTY and added positionName field.
                         Added SOURCE_INFO and PROCESS_STEP_INFO type definitions to represent lineage metadata.
                         Modified DATA_QUALITY_INFO structure to include lineage metadata.
                         Modified IDENTIFICATION_INFO to include an array of cited responsible parties.
                         Modified SPATIAL_REPRESENTATION_INFO to represent cellGeometry, transformationParameterAvailability, and checkPointAvailability
                         Added SetContact method.
                         Added defines for maximum sizes.
       - 07/23/2008   -  E. Warner - Added set methods.   
                         Corrected bug where incorrect size was returnd for XML string in method CreateXmlMetadataString.   
       - 01/06/2009   -  E. Warner - Added InitResponsibleParty method.     
       - 05/07/2009   -  E. Warner - Changed dateTime field in PROCESS_STEP_INFO structure so that it is 30 chars long instead of 21.  
                         This new length accommodates the hours minutes, seconds and time zone that may be specified.
       - 05/18/2009   -  E. Warner - Added method to free memory that was allocated during reading of XML into data structures.
       - 06/18/2009   -  E. Warner - Modified "Get" method signatures to use NV_U_CH AR instead of NV_CHAR.
                         Done for consistency in method signature.
       - 06/30/2009   -  M. Russell - Modified the length of the depthCorrectionType in the IDENTIFICATION_INFO
                         structure from 22 to 32 to adjust for the types agreed on from the
                         ONSWG 2009-06-12 teleconference
       - 01/15/2011   -  Jan Depner - Switched from NAVO specific data types to OpenNS (BAG) data types.  Code cleanup (for readability).
       - 09/22/2011   -  Added Doxygen comments and Doxyfile for generating documentation from source code.


       <br><br>\section search Searching the Doxygen Documentation

       The easiest way to find C++ methods is to go to the
       <a href="globals.html"><b>Files>Globals>All</b></a> tab.  If, for example, you are looking for
       <b><i>GetContactList</i></b> you would then click on the <b><i>g</i></b> tab,
       find <b><i>GetContactList</i></b>, and then click on the associated file.  The entire group of public
       methods for a particular class are documented via their class documentation not via their include files.
       You can see documentation for them in the main <a href="annotated.html"><b>Data Structures</b></a> tab or
       by selecting the class in the Data Structures section of the .hpp file.<br><br>

   */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stdtypes.h"


  /*! Maximum number of parties that can be stored for citations. */

#define MAX_CI_RESPONSIBLE_PARTIES 10


  /*! Value to which floats and ints will be initialized. */

#define INIT_VALUE  -999    


  /*! OPTIONAL */

  typedef struct
  {
    char             individualName[100];            /*!< REQUIRED: One of the following must be supplied: individualName,organisationName, positionName. */  
    char             organisationName[100];
    char             positionName[100];
    char             phoneNumber[17];
    char             role[100];                      /*!< REQUIRED                                                                                        */
  } RESPONSIBLE_PARTY;


  /*! REQUIRED */

  typedef struct
  {
    char             title[100];                     /*!< OPTIONAL: but this field is required in order to make a citation.     */
    char             date[20];                       /*!< OPTIONAL: but this field is required in order to make a citation date */
    char             dateType[20];                   /*!< OPTIONAL: but this field is required in order to make a citation date */
    RESPONSIBLE_PARTY   responsibleParties[MAX_CI_RESPONSIBLE_PARTIES];      /*!< OPTIONAL                                         */
    char             abstract[8000];                 /*!< REQUIRED                                                              */
    char             purpose[100];                   
    char             status[100];                   
    char             spatialRepresentationType[100]; /*!< OPTIONAL: For Nav Spec, this should be set to "grid".                  */
    char             language[100];                  /*!< REQUIRED: For Nav Spec, this should be set to "en"                     */
    char             topicCategory[100];             /*!< REQUIRED: For Nav Spec, this should be set to "grid"                   */
    f64              westBoundingLongitude;          /*!< OPTIONAL: but this field is required in order to make an extent.       */
    f64              eastBoundingLongitude;          /*!< OPTIONAL: but this field is required in order to make an extent.       */
    f64              southBoundingLatitude;          /*!< OPTIONAL: but this field is required in order to make an extent.       */
    f64              northBoundingLatitude;          /*!< OPTIONAL: but this field is required in order to make an extent.       */
    char             verticalUncertaintyType[40];    /*!< REQUIRED                                                               */
    char             depthCorrectionType[32];        /*!< OPTIONAL: Navigation specific */
  } IDENTIFICATION_INFO;


  typedef struct
  {
    char             useConstraints[40];
    char             otherConstraints[100];          /*!< OPTIONAL: For Nav Spec, this should be set to "grid".                  */
  } MD_LEGAL_CONSTRAINTS;


  /*! REQUIRED structure Navigation Specific */

  typedef struct
  {
    char             classification[40];             /*!< REQUIRED   */
    char             userNote[4000];                 /*!< REQUIRED: Contains distribution, declass authority, declass date.   */
  } MD_SECURITY_CONSTRAINTS;


  /*! OPTIONAL */

  typedef struct
  {
    char             description[200];      /*!< REQUIRED: Navigation Specific required if source is desired.          */
    char             title[100];            /*!< OPTIONAL: but this field is required if a citation is desired.        */
    char             date[20];              /*!< OPTIONAL: but this field is required if a citation date is desired.   */
    char             dateType[20];          /*!< OPTIONAL: but this field is required if a citation date is desired.   */
    RESPONSIBLE_PARTY  responsibleParties[MAX_CI_RESPONSIBLE_PARTIES];   /*!< Placeholder.  Not implemented yet.         */
  } SOURCE_INFO;


  /*! OPTIONAL */

  typedef struct
  {
    char             description[200];                          /*!< REQUIRED  */
    char             dateTime[30];
    RESPONSIBLE_PARTY  processors[MAX_CI_RESPONSIBLE_PARTIES];
    char             trackingId[5];                             /*!< REQUIRED  */
  } PROCESS_STEP_INFO;


  /*! REQUIRED: Navigation Specific */

  typedef struct
  {
    char             scope[100];                   /*!< Navigation specific: always set to "dataset"                 */
    SOURCE_INFO      * lineageSources;             /*!< REQUIRED: Navigation specific, require at least 1 as scope is set to 'dataset'        */
    s16              numberOfSources;              /*!< REQUIRED: Navigation specific */
    PROCESS_STEP_INFO  * lineageProcessSteps;        /*!< REQUIRED: Navigation specific, require at least 1 as scope is set to 'dataset'        */
    s16              numberOfProcessSteps;         /*!< REQUIRED: Navigation specific */
  } DATA_QUALITY_INFO;


  /*! REQUIRED: Navigation Specific */

  typedef struct
  {
    s16              numberOfDimensions;                        /*!< REQUIRED */
    char             dimensionName[3][20];                      /*!< REQUIRED: Navigation specific */
    s32              dimensionSize[3];                          /*!< REQUIRED: Navigation specific */
    f64              resolutionValue[3];                        /*!< REQUIRED: Navigation specific */
    char             cellGeometry[10];                          /*!< REQUIRED: Nav Spec should always set this to "point"             */
    char             transformationParameterAvailability[6];    /*!< REQUIRED: Nav Spec should always set this to "false"             */
    char             checkPointAvailability[2];                 /*!< REQUIRED: Nav Spec should always set this to 0 indicating false. */
    f64              llCornerX;
    f64              llCornerY;
    f64              urCornerX;
    f64              urCornerY;
  } SPATIAL_REPRESENTATION_INFO;                            


  /*! REQUIRED:  Navigation Specific */

  typedef struct
  {
    char             projection[100];                      /*!< REQUIRED Navigation Specific */
    char             ellipsoid[100];                       /*!< REQUIRED Navigation Specific */
    char             horizontalDatum[100];                 /*!< REQUIRED Navigation Specific */
    s16              zone;                                
    f64              standardParallel;                 
    f64              longitudeOfCentralMeridian;        
    f64              latitudeOfProjectionOrigin;       
    f64              falseEasting;                      
    f64              falseNorthing;                     
    f64              scaleFactorAtEquator;                 
    f64              heightOfProspectivePointAboveSurface; 
    f64              longitudeOfProjectionCenter;        
    f64              latitudeOfProjectionCenter;        
    f64              scaleFactorAtCenterLine;            
    f64              straightVerticalLongitudeFromPole;   
    f64              scaleFactorAtProjectionOrigin;       
    f64              semiMajorAxis;                      
    char             verticalDatum[100];                   /*!< REQUIRED Navigation Specific */
  } REFERENCE_SYSTEM_INFO;


  /******************************************************************************************************

                                  Methods to initialize structures are listed below.

  ******************************************************************************************************/

  s16 InitDataIdentificationInfo(IDENTIFICATION_INFO * dataIdentificationInfo);

  s16 InitLegalConstraints(MD_LEGAL_CONSTRAINTS * legalConstraints);

  s16 InitSecurityConstraints(MD_SECURITY_CONSTRAINTS * securityConstraints);

  s16 InitDataQualityInfo(DATA_QUALITY_INFO * dataQualityInfo);

  s16 InitReferenceSystemInfo(REFERENCE_SYSTEM_INFO * referenceSystemInfo);

  s16 InitSpatialRepresentationInfo(SPATIAL_REPRESENTATION_INFO * spatialRepresentationInfo);

  s16 InitResponsibleParty(RESPONSIBLE_PARTY * responsibleParty);

  s16 InitSourceInfo(SOURCE_INFO * sourceInfo);


  /******************************************************************************************************

                                  Methods to get metadata are listed below.

  ******************************************************************************************************/

  s16  GetDataIdentification(const u8 * xmlString, IDENTIFICATION_INFO * dataIdentificationInfo);

  s16  GetLegalConstraints(const u8 * xmlString, MD_LEGAL_CONSTRAINTS * legalConstraints);

  s16  GetSecurityConstraints(const u8 * xmlString, MD_SECURITY_CONSTRAINTS * securityConstraints);

  s16  GetDataQualityInfo(const u8 * xmlString, DATA_QUALITY_INFO * dataQuality );

  s16  GetReferenceSystemInfo(const u8 * xmlString, REFERENCE_SYSTEM_INFO * referenceSystemInfo);

  s16  GetSpatialRepresentationInfo(const u8 * xmlString, SPATIAL_REPRESENTATION_INFO * spatialRepresentationInfo);

  s16  GetDateStamp(const u8 * xmlString, char ** dateString);

  s16  GetLanguage(const u8 * xmlString, char ** language);  

  s16  GetContact(const u8 * xmlString, RESPONSIBLE_PARTY * contact);

  s16  GetMetadataStandardName(const u8 * xmlString, char ** metadataStandardName);

  s16  GetMetadataStandardVersion( const u8 * xmlString, char ** metadataStandardVersion);

  s16  GetAllStructures( const u8 * xmlString,IDENTIFICATION_INFO * identificationInfo, MD_LEGAL_CONSTRAINTS * legalConstraints,
                              MD_SECURITY_CONSTRAINTS * securityConstraints,  DATA_QUALITY_INFO * dataQuality,
                              SPATIAL_REPRESENTATION_INFO * spatialRepresentationInfo, REFERENCE_SYSTEM_INFO * referenceSystemInfo,
                              RESPONSIBLE_PARTY * contact );


  /******************************************************************************************************

                                  Methods to set metadata are listed below.

  ******************************************************************************************************/

  s32  CreateXmlMetadataString(IDENTIFICATION_INFO identificationInfo, 
                               MD_LEGAL_CONSTRAINTS legalConstraints, 
                               MD_SECURITY_CONSTRAINTS securityConstraints,  
                               DATA_QUALITY_INFO dataQuality, 
                               SPATIAL_REPRESENTATION_INFO spatialRepresentationInfo, 
                               REFERENCE_SYSTEM_INFO referenceSystemInfo, 
                               RESPONSIBLE_PARTY contact, 
                               char * creationDate, 
                               u8 ** xmlString);

  s32 SetContact(RESPONSIBLE_PARTY newContact, u8 ** xmlString);

  s32 SetIdentificationInfo(IDENTIFICATION_INFO newIdentificationInfo, u8 ** xmlString);

  s32 SetLegalConstraints(MD_LEGAL_CONSTRAINTS newLegalConstraints, u8 ** xmlString);

  s32 SetSecurityConstraints(MD_SECURITY_CONSTRAINTS newSecurityConstraints, u8 ** xmlString);

  s32 SetDataQualityInfo(DATA_QUALITY_INFO newDataQualityInfo, u8 ** xmlString);

  s32 SetSpatialRepresentationInfo(SPATIAL_REPRESENTATION_INFO newSpatialRepresentationInfo, u8 ** xmlString);

  s32 SetReferenceSystemInfo(REFERENCE_SYSTEM_INFO newIdentificationInfo, u8 ** xmlString);

  s32 SetLanguage(const char * newLanguage, u8 ** xmlString);

  s32 SetDateStamp(const char * newDateStamp, u8 ** xmlString);

  s32 SetMetadataStandardName(const char * newMetadataStandardName, u8 ** xmlString);

  s32 SetMetadataStandardVersion(const char * newMetadataStandardVersion, u8 ** xmlString);


  /******************************************************************************************************

                                  Methods to print data structures are listed below.

  ******************************************************************************************************/

  void PrintIdentificationInfo(IDENTIFICATION_INFO identificationInfo);

  void PrintLegalConstraints(MD_LEGAL_CONSTRAINTS legalConstraints); 

  void PrintSecurityConstraints(MD_SECURITY_CONSTRAINTS securityConstraints);  

  void PrintDataQuality(DATA_QUALITY_INFO dataQuality);

  void PrintSpatialRepresentationInfo(SPATIAL_REPRESENTATION_INFO spatialRepresentationInfo);

  void PrintReferenceSystemInfo(REFERENCE_SYSTEM_INFO referenceSystemInfo);

  void PrintContact(RESPONSIBLE_PARTY contact);


  /******************************************************************************************************

                 Methods to clean up dynamically allocated memory for data structures are listed below.

  ******************************************************************************************************/

  /* Currently only the GetDataQualityInfo method allocates memory dynamically for a data structure. */

  s16  FreeDataQualityInfo(DATA_QUALITY_INFO * dataQuality );


#ifdef  __cplusplus
}
#endif

#endif
