#include "bag_xml_meta.h"


#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOM.hpp>


#include <xercesc/util/XMLString.hpp>

#if defined(XERCES_NEW_IOSTREAMS)
#include <iostream>
#else
#include <iostream.h>
#endif

#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>


// Other include files, declarations, and non-Xerces-C++ initializations.

XERCES_CPP_NAMESPACE_USE 


bool XMLinitialized=false;
DOMImplementation* impl;


// ---------------------------------------------------------------------------
//  This is a simple class that lets us do easy (though not terribly efficient)
//  trancoding of char* data to XMLCh data.
// ---------------------------------------------------------------------------
class XStr
{
public :
  // -----------------------------------------------------------------------
  //  Constructors and Destructor
  // -----------------------------------------------------------------------

  XStr(const char* const toTranscode)
  {
    // Call the private transcoding method
    fUnicodeForm = XMLString::transcode(toTranscode);
  }

  ~XStr()
  {
    XMLString::release(&fUnicodeForm);
  }


  // -----------------------------------------------------------------------
  //  Getter methods
  // -----------------------------------------------------------------------

  const XMLCh* unicodeForm() const
  {
    return (fUnicodeForm);
  }


private :
  // -----------------------------------------------------------------------
  //  Private data members
  //
  //  fUnicodeForm
  //      This is the Unicode XMLCh format of the string.
  // -----------------------------------------------------------------------

  XMLCh*   fUnicodeForm;
};

#define X(str) XStr(str).unicodeForm()
