//************************************************************************
//      File:    ons_xml_error_handler.h
//
//      Open Navigation Surface Working Group, 2005
//
//      - Initial implementation
//        Bill Lamey, 7/19/2005
//
//************************************************************************

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/sax/ErrorHandler.hpp>


XERCES_CPP_NAMESPACE_USE


// This class represents the DOM error handler.
class BAGMetaDataErrorHandler : public ErrorHandler
{
public:

// Construction/destruction.
    BAGMetaDataErrorHandler() :
       fSawErrors(false), fSawFatals(false)
    {
    }

    ~BAGMetaDataErrorHandler()
    {
    }

// Methods
    void warning(const SAXParseException& toCatch);
    void error(const SAXParseException& toCatch);
    void fatalError(const SAXParseException& toCatch);
    void resetErrors();

// gets
    bool getSawErrors() const;
    bool getFatalErrors() const;

private:
    // Internal flag set if any errors were dispatched during parsing or validating.
    bool    fSawErrors;
    // Flag indiciating if the errors were fatal.
    bool    fSawFatals;
};


//************************************************************************
//      Method name:    getSawErrors()
//
//      
//      - Initial implementation
//        Bill Lamey, 7/19/2005
//
//************************************************************************
//! Get the value of the sawErrors member.
/*!
\return
    \li true if the parser had errors during parse and/or validation.
*/
//************************************************************************
inline bool BAGMetaDataErrorHandler::getSawErrors(
    ) const
{
    return fSawErrors;
}

//************************************************************************
//      Method name:    getFatalErrors()
//
//      
//      - Initial implementation
//        Bill Lamey, 7/26/2005
//
//************************************************************************
//! Get the fatal error flag.
/*!
        This flag is set if any of the errors encountered were fatal.

\return
    \li true if the errors encountered during parsing were fatal, false if not.
*/
//************************************************************************
inline bool BAGMetaDataErrorHandler::getFatalErrors(
    ) const
{
    return fSawFatals;
}
