/*!
\file bag_errors.h
\brief Definition of all error codes.
*/
//************************************************************************
//
//      Open Navigation Surface Working Group, 2013
//
//************************************************************************
#ifndef BAG_ERRORS_H
#define BAG_ERRORS_H

/*! Definitions for error conditions */
#define BAG_GENERAL_ERROR_BASE                    0
#define BAG_CRYPTO_ERROR_BASE                   200
#define BAG_METADATA_ERROR_BASE                 400
#define BAG_HDFV_ERROR_BASE                     600

/*! General error conditions, including success */
enum BAG_ERRORS {
    BAG_SUCCESS                                =   0, /*!< Normal, successful completion */
    BAG_BAD_FILE_IO_OPERATION                  =   1, /*!< A basic file IO operation failed */
    BAG_NO_FILE_FOUND                          =   2, /*!< Specified file name could not be found */
    BAG_NO_ACCESS_PERMISSION                   =   3, /* Used ? */
    BAG_MEMORY_ALLOCATION_FAILED               =   4, /*!< Memory allocation failed */
    BAG_INVALID_BAG_HANDLE                     =   5, /*!< bagHandle cannot be NULL */
    BAG_INVALID_FUNCTION_ARGUMENT              =   6, /*!< Inconsistency or illegal value contained in function arguments */
    BAG_INVALID_ERROR_CODE                     =   7, /*!< An undefined bagError code was encountered */

    BAG_CRYPTO_SIGNATURE_OK                    = 200, /*!< Signature found, and valid */
    BAG_CRYPTO_NO_SIGNATURE_FOUND              = 201, /*!< No signature found in file */
    BAG_CRYPTO_BAD_SIGNATURE_BLOCK             = 202, /*!< Signature found, but invalid */
    BAG_CRYPTO_BAD_KEY                         = 203, /*!< Internal key format is invalid */
    BAG_CRYPTO_WRONG_KEY                       = 204, /*!< Wrong key type passed */
    BAG_CRYPTO_GENERAL_ERROR                   = 205, /*!< Something else went wrong */
    BAG_CRYPTO_INTERNAL_ERROR                  = 206, /*!< Something went wrong that the library didn't expect */

    BAG_METADTA_NO_HOME                        = 400, /*!< BAG_HOME directory not set. */
    BAG_METADTA_SCHEMA_FILE_MISSING            = 401, /*!< Unable to locate schema file. */
    BAG_METADTA_PARSE_MEM_EXCEPTION            = 402, /*!< Unhandled exception while parsing.  Out of memory. */
    BAG_METADTA_PARSE_EXCEPTION                = 403, /*!< Unhandled exception while parsing.  Parser error. */
    BAG_METADTA_PARSE_DOM_EXCEPTION            = 404, /*!< Unhandled exception while parsing.  DOM error. */
    BAG_METADTA_PARSE_UNK_EXCEPTION            = 405, /*!< Unhandled exception while parsing.  Unknown error. */
    BAG_METADTA_PARSE_FAILED                   = 406, /*!< Unable to parse input file. */
    BAG_METADTA_PARSE_FAILED_MEM               = 407, /*!< Unable to parse specified input buffer. */
    BAG_METADTA_VALIDATE_FAILED                = 408, /*!< XML validation failed. */
    BAG_METADTA_INVALID_HANDLE                 = 409, /*!< Invalid (NULL) handle supplied to an accessor method. */
    BAG_METADTA_INIT_FAILED                    = 410, /*!< Initialization of the low level XML support system failed. */
    BAG_METADTA_NO_PROJECTION_INFO             = 411, /*!< No projection information was found in the XML supplied. */
    BAG_METADTA_INSUFFICIENT_BUFFER            = 412, /*!< The supplied buffer is not large enough to hold the extracted contents. */
    BAG_METADTA_INCOMPLETE_COVER               = 413, /*!< One or more elements of the requested cover are missing from the XML file. */
    BAG_METADTA_INVLID_DIMENSIONS              = 414, /*!< The number of dimensions is incorrect. (not equal to 2). */
    BAG_METADTA_UNCRT_MISSING                  = 415, /*!< The 'uncertaintyType' information is missing from the XML structure. */
    BAG_METADTA_BUFFER_EXCEEDED                = 416, /*!< The supplied buffer is to large to be stored in the internal array. */
    BAG_METADTA_DPTHCORR_MISSING               = 417, /*!< The 'depthCorrectionType' information is missing from the XML structure. */
    BAG_METADTA_RESOLUTION_MISSING             = 418, /*!< The 'resolution' information is missing from the XML structure. */
    BAG_METADTA_INVALID_PROJECTION             = 419, /*!< The projection type is not supported. */
    BAG_METADTA_INVALID_DATUM                  = 420, /*!< The datum is not supported. */
    BAG_METADTA_INVALID_HREF                   = 421, /*!< The horizontal reference system information is missing from the XML structure. */
    BAG_METADTA_INVALID_VREF                   = 422, /*!< The vertical reference system information is missing from the XML structure. */
    BAG_METADTA_SCHEMA_SETUP_FAILED            = 423, /*!< Schema set up failed. */
    BAG_METADTA_SCHEMA_VALIDATION_SETUP_FAILED = 424, /*!< Schema validation set up failed. */
    BAG_METADTA_EMPTY_DOCUMENT                 = 425, /*!< The supplied XML file is empty. */
    BAG_METADTA_MISSING_MANDATORY_ITEM         = 426, /*!< Schema validation failed.  One or more mandatory elements are missing. */
    BAG_METADTA_NOT_INITIALIZED                = 427, /*!< The metadata has not been initialized correctly. */

    BAG_NOT_HDF5_FILE                          = 602, /*!< HDF Bag is not an HDF5 File */
    BAG_HDF_RANK_INCOMPATIBLE                  = 605, /*!< HDF Bag's rank is incompatible with expected Rank of the Datasets */
    BAG_HDF_TYPE_NOT_FOUND                     = 606, /*!< HDF Bag surface Datatype parameter not available */
    BAG_HDF_DATASPACE_CORRUPTED                = 607, /*!< HDF Dataspace for a bag surface is corrupted or could not be read */
    BAG_HDF_ACCESS_EXTENTS_ERROR               = 608, /*!< HDF Failure in request for access outside the extents of a bag surface's Dataset */
    BAG_HDF_CANNOT_WRITE_NULL_DATA             = 609, /*!< HDF Cannot write NULL or uninitialized data to Dataset */
    BAG_HDF_INTERNAL_ERROR                     = 610, /*!< HDF There was an internal HDF error detected */
    BAG_HDF_CREATE_FILE_FAILURE                = 611, /*!< HDF Unable to create new HDF Bag File */
    BAG_HDF_CREATE_DATASPACE_FAILURE           = 612, /*!< HDF Unable to create the Dataspace */
    BAG_HDF_CREATE_PROPERTY_CLASS_FAILURE      = 613, /*!< HDF Unable to create the Property class */
    BAG_HDF_SET_PROPERTY_FAILURE               = 614, /*!< HDF Unable to set value of Property class */
    BAG_HDF_TYPE_COPY_FAILURE                  = 615, /*!< HDF Failed to copy Datatype parameter for Dataset access */
    BAG_HDF_CREATE_DATASET_FAILURE             = 616, /*!< HDF Unable to create the Dataset */
    BAG_HDF_DATASET_EXTEND_FAILURE             = 617, /*!< HDF Cannot extend Dataset extents */
    BAG_HDF_CREATE_ATTRIBUTE_FAILURE           = 618, /*!< HDF Unable to create Attribute */
    BAG_HDF_CREATE_GROUP_FAILURE               = 619, /*!< HDF Unable to create Group */
    BAG_HDF_WRITE_FAILURE                      = 620, /*!< HDF Failure writing to Dataset */
    BAG_HDF_READ_FAILURE                       = 621, /*!< HDF Failure reading from Dataset */
    BAG_HDF_GROUP_CLOSE_FAILURE                = 622, /*!< HDF Failure closing Group */
    BAG_HDF_FILE_CLOSE_FAILURE                 = 623, /*!< HDF Failure closing File */
    BAG_HDF_FILE_OPEN_FAILURE                  = 624, /*!< HDF Unable to open File */
    BAG_HDF_GROUP_OPEN_FAILURE                 = 625, /*!< HDF Unable to open Group */
    BAG_HDF_ATTRIBUTE_OPEN_FAILURE		       = 626, /*!< HDF Unable to open Attribute */
    BAG_HDF_ATTRIBUTE_CLOSE_FAILURE		       = 627, /*!< HDF Failure closing Attribute */
    BAG_HDF_DATASET_CLOSE_FAILURE              = 628, /*!< HDF Failure closing Dataset */
    BAG_HDF_DATASET_OPEN_FAILURE               = 629, /*!< HDF Unable to open Dataset */
    BAG_HDF_TYPE_CREATE_FAILURE                = 630, /*!< HDF Unable to create Datatype */
    BAG_HDF_INVALID_COMPRESSION_LEVEL          = 631, /*!< HDF compression level not in acceptable range of 0 to 9 */

};

#endif  // BAG_ERRORS_H

