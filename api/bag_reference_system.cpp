//************************************************************************
//
//      Open Navigation Surface Working Group, 2012
//
//************************************************************************
#include "bag_errors.h"
#include "bag_legacy.h"
#include <algorithm>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{

//! Projection parameters WKT names.
const char k_latitude_of_origin[] = "latitude_of_origin";
const char k_central_meridian[] = "central_meridian";
const char k_scale_factor[] = "scale_factor";
const char k_false_easting[] = "false_easting";
const char k_false_northing[] = "false_northing";
const char k_standard_parallel_1[] = "standard_parallel_1";
const char k_standard_parallel_2[] = "standard_parallel_2";
const char k_latitude_of_center[] = "latitude_of_center";
const char k_longitude_of_center[] = "longitude_of_center";

//! Projection type WKT names.
const char k_albers_conic_equal_area[] = "albers_conic_equal_area";
const char k_azimuthal_equidistant[] = "azimuthal_equidistant";
const char k_bonne[] = "bonne";
const char k_cassini_soldner[] = "cassini_soldner";
const char k_cylindrical_equal_area[] = "cylindrical_equal_area";
const char k_eckert_iv[] = "eckert_iv";
const char k_eckert_vi[] = "eckert_vi";
const char k_equirectangular[] = "equirectangular";
const char k_gnomonic[] = "gnomonic";
const char k_lambert_conformal_conic[] = "lambert_conformal_conic_2sp";
const char k_mercator[] = "mercator_1sp";
const char k_miller_cylindrical[] = "miller_cylindrical";
const char k_mollweide[] = "mollweide";
const char k_new_zealand_map_grid[] = "new_zealand_map_grid";
const char k_orthographic[] = "orthographic";
const char k_polar_stereographic[] = "polar_stereographic";
const char k_polyconic[] = "polyconic";
const char k_sinusoidal[] = "sinusoidal";
const char k_oblique_stereographic[] = "oblique_stereographic";
const char k_transverse_mercator[] = "transverse_mercator";
const char k_vandergrinten[] = "vandergrinten";

//! Datum WKT names.
const char k_wgs84[] = "wgs_1984";
const char k_wgs72[] = "wgs_1972";
const char k_nad83[] = "north_american_datum_1983";

//! Simple exception thrown internally when we run into a problem.
struct CoordSysError : virtual std::exception
{
    virtual const char * what() const throw() { return "Conversion Error";};
};

//! Simple exception thrown when we can not convert the datum.
struct InvalidDatumError : virtual CoordSysError {};

//! Simple exception thrown when we can not convert the ellipsoid.
struct InvalidEllipsoidError : virtual CoordSysError {};

const double PI = 3.141592653589793238;
const double PI_OVER_180 = PI / 180.0;

//************************************************************************
//      Method name:    toDouble()
//
//      - Initial implementation
//        Mike Van Duzee, 1/26/2012
//
//************************************************************************
/*!
\brief Convert the given string value to double.

    The string is expected to contain a decimal value in the classic locale.
    For example something like "20.0". The classic locale uses a '.' as the 
    decimal seperator.

\param value
    \li The string to convert.
\return
    \li The converted value.
*/
//************************************************************************
double toDouble(const std::string &value)
{
    std::stringstream lineStream;
    lineStream.imbue(std::locale::classic());
    lineStream << value;

    double dblValue = 0.0;
    lineStream >> dblValue;

    return dblValue;
}

//************************************************************************
//      Method name:    ellipsoidToWkt()
//
//      - Initial implementation
//        Mike Van Duzee, 1/26/2012
//
//************************************************************************
/*!
\brief Convert the BAG ellipsoid to WKT.

    To convert the ellipsoid we need the semi-major and inverse flattening
    ratio. We will open the ellips.dat file to retrieve this information.

\param ellipsoid
    \li The BAG ellipsoid type to convert.
\return
    \li The WKT representation for that ellipsoid.
*/
//************************************************************************
std::string ellipsoidToWkt(const u8 *ellipsoid)
{
    const char *onsHome = getenv("BAG_HOME");
    if (onsHome == NULL)
        throw InvalidEllipsoidError();

    //Make the ellipsoid name all lower case so we can find it.
    std::string ellipsoidName((const char *)ellipsoid);
    std::transform(ellipsoidName.begin(), ellipsoidName.end(), ellipsoidName.begin(), ::tolower);

    //Build the full path to the ellips.dat file.
    std::string ellipFile(onsHome);
    ellipFile += "/ellips.dat";

    std::ifstream file(ellipFile.c_str());
    if (!file.is_open())
        throw InvalidEllipsoidError();

    while (file.good())
    {
        //Get the current line and toLower it.
        std::string line;
        std::getline(file, line);
        std::transform(line.begin(), line.end(), line.begin(), ::tolower);

        //Is this the correct ellipsoid?
        const size_t index = line.find(ellipsoidName);
        if (index != 0)
            continue;

        std::vector<std::string> elements;
        char *buffer = (char *)calloc (line.size(), sizeof (char));
        strncpy (buffer, line.c_str(), line.size());

        //Decode all of the pieces.
        char *token = strtok(buffer, " ");
        while (token != NULL)
        {
            elements.push_back(token);
            token = strtok(NULL, " ");
        }

        free(buffer);

        const size_t numItems = elements.size();

        //We MUST have at least 5 elements (name, id, a, b, if)
        if (numItems < 5)
            throw InvalidEllipsoidError();

        //The last item will be the inverse flattening.
        const double invFlat = atof(elements[numItems - 1].c_str());

        //The third to last item will be the semi-major.
        const double semiMajor = atof(elements[numItems - 3].c_str());

        std::stringstream wktStream;
        wktStream.imbue(std::locale::classic());
        wktStream << std::fixed << std::setprecision(9)
            << "SPHEROID[\"" << (const char *)ellipsoid << "\"," << semiMajor << "," << invFlat << "]";
        return wktStream.str();
    }

    //Couldn't find it :(
    throw InvalidEllipsoidError();
}

//************************************************************************
//      Method name:    datumToWkt()
//
//      - Initial implementation
//        Mike Van Duzee, 1/26/2012
//
//************************************************************************
/*!
\brief Convert the BAG datum type to WKT.

    If we are unable to convert the specified ellipsoid, we will use
    a good default for the datum type.

\param datum
    \li The BAG datum type to convert.
\param ellipsoid
    \li The BAG ellipsoid to convert.
\return
    \li The WKT representation for that datum.
*/
//************************************************************************
std::string datumToWkt(const bagDatum datum, const u8 *ellipsoid)
{
    std::string ellipWkt;

    //Try to decode the ellipsoid, if we fail then we will use the 'default' ellipsoid.
    try
    {
        //Try to decode the ellipsoid
        ellipWkt = ellipsoidToWkt(ellipsoid);
    }
    catch (...)
    {
        switch (datum)
        {
            case wgs84:
                ellipWkt = "SPHEROID[\"WGS 84\",6378137,298.257223563]";
                break;

            case wgs72:
                ellipWkt = "SPHEROID[\"WGS 72\",6378135,298.26]";
                break;

            case nad83:
                ellipWkt = "SPHEROID[\"GRS 1980\",6378137,298.257222101]";
        }
    };

    std::stringstream wktStream;
    wktStream.imbue(std::locale::classic());

    switch (datum)
    {
    case wgs84:
        {
            wktStream << "GEOGCS[\"WGS 84\", "
                << "DATUM[\"WGS_1984\", "
                << ellipWkt << ", "
                << "TOWGS84[0,0,0,0,0,0,0]], "
                << "PRIMEM[\"Greenwich\",0], "
                << "UNIT[\"degree\",0.0174532925199433]]";

            return wktStream.str();
        }
        break;

    case wgs72:
        {
            wktStream << "GEOGCS[\"WGS 72\", "
                << "DATUM[\"WGS_1972\", "
                << ellipWkt << ", "
                << "TOWGS84[0,0,4.5,0,0,0.554,0.2263]], "
                << "PRIMEM[\"Greenwich\",0], "
                << "UNIT[\"degree\",0.0174532925199433]]";

            return wktStream.str();
        }
        break;

    case nad83:
        {
            wktStream << "GEOGCS[\"NAD83\", "
                << "DATUM[\"North_American_Datum_1983\", "
                << ellipWkt << ", "
                << "TOWGS84[0,0,0,0,0,0,0]], "
                << "PRIMEM[\"Greenwich\",0], "
                << "UNIT[\"degree\",0.0174532925199433]]";

            return wktStream.str();
        }
        break;
    }

    //If we got here then we don't know what type of datum we have.
    throw InvalidDatumError();
}

//************************************************************************
//      Method name:    getProjectionParam()
//
//      - Initial implementation
//        Mike Van Duzee, 1/26/2012
//
//************************************************************************
/*!
\brief Retrieve the specified projection parameter.

\param wkt
    \li The wkt string containing the coordinate system definition.
\param paramName
    \li The name of the projection parameter to retrieve.
\return
    \li The specified projection parameter.
*/
//************************************************************************
double getProjectionParam(const std::string &wkt, const std::string &paramName)
{
    //Find the projection node in the wkt string.
    const size_t startIndex = wkt.find(paramName);
    if (startIndex == std::string::npos)
        throw CoordSysError();

    const size_t valueStartIndex = wkt.find(",", startIndex);
    if (valueStartIndex == std::string::npos)
        throw CoordSysError();

    const size_t valueEndIndex = wkt.find("]", valueStartIndex);
    if (valueEndIndex == std::string::npos)
        throw CoordSysError();

    //Extract the value
    const size_t startPos = valueStartIndex + 1;
    const size_t length = valueEndIndex - startPos;
    const std::string value = wkt.substr(startPos, length);

    return toDouble(value);
}

//************************************************************************
//      Method name:    getCoordinateType()
//
//      - Initial implementation
//        Mike Van Duzee, 1/26/2012
//
//************************************************************************
/*!
\brief Retrieve the BAG coordinate type from the WKT definition.

\param wkt
    \li The wkt string containing the coordinate system definition.
\return
    \li The BAG coordiante system type.
*/
//************************************************************************
Coordinate_Type getCoordinateType(const std::string &wkt)
{
    //Find the projection node in the wkt string.
    const size_t startIndex = wkt.find("projection[");

    //If no projection node, then we must have a Geographic system.
    if (startIndex == std::string::npos)
        return Geodetic;

    const size_t endIndex = wkt.find("\"]", startIndex);
    if (endIndex == std::string::npos)
        throw CoordSysError();

    //Extract the projection name.
    const size_t startPos = startIndex + 12;
    const size_t length = endIndex - startPos;
    const std::string projName = wkt.substr(startPos, length);

    if (projName == k_albers_conic_equal_area)
        return Albers_Equal_Area_Conic;
    else if (projName == k_azimuthal_equidistant)
        return Azimuthal_Equidistant;
    else if (projName == k_bonne)
        return Bonne;
    else if (projName == k_cassini_soldner)
        return Cassini;
    else if (projName == k_cylindrical_equal_area)
        return Cylindrical_Equal_Area;
    else if (projName == k_eckert_iv)
        return Eckert4;
    else if (projName == k_eckert_vi)
        return Eckert6;
    else if (projName == k_equirectangular)
        return Equidistant_Cylindrical;
    else if (projName == k_gnomonic)
        return Gnomonic;
    else if (projName == k_lambert_conformal_conic)
        return Lambert_Conformal_Conic;
    else if (projName == k_mercator)
        return Mercator;
    else if (projName == k_miller_cylindrical)
        return Miller_Cylindrical;
    else if (projName == k_mollweide)
        return Mollweide;
    else if (projName == k_new_zealand_map_grid)
        return NZMG;
    else if (projName == k_orthographic)
        return Orthographic;
    else if (projName == k_polar_stereographic)
        return Polar_Stereo;
    else if (projName == k_polyconic)
        return Polyconic;
    else if (projName == k_sinusoidal)
        return Sinusoidal;
    else if (projName == k_oblique_stereographic)
        return Stereographic;
    else if (projName == k_transverse_mercator)
        return Transverse_Mercator;
    else if (projName == k_vandergrinten)
        return Van_der_Grinten;

    //No idea...
    throw CoordSysError();
}

//************************************************************************
//      Method name:    getDatumType()
//
//      - Initial implementation
//        Mike Van Duzee, 1/26/2012
//
//************************************************************************
/*!
\brief Retrieve the BAG datum type from the WKT definition.

\param wkt
    \li The wkt string containing the coordinate system definition.
\return
    \li The BAG datum type.
*/
//************************************************************************
bagDatum getDatumType(const std::string &wkt)
{
    //Find the horizontal datum node in the wkt string.
    const size_t startIndex = wkt.find("datum[");
    if (startIndex == std::string::npos)
        throw InvalidDatumError();

    const size_t endIndex = wkt.find(",", startIndex);
    if (endIndex == std::string::npos)
        throw InvalidDatumError();

    //Extract the horizontal datum name.
    const size_t startPos = startIndex + 7;
    const size_t length = endIndex - startPos - 1;
    const std::string hDatumName = wkt.substr(startPos, length);

    if (hDatumName == k_wgs84)
        return wgs84;
    else if (hDatumName == k_wgs72)
        return wgs72;
    else if (hDatumName == k_nad83)
        return nad83;

    //Unknown, so we can not convert this coordinate system.
    throw InvalidDatumError();
}

//************************************************************************
//      Method name:    getEllipsoid()
//
//      - Initial implementation
//        Mike Van Duzee, 1/26/2012
//
//************************************************************************
/*!
\brief Retrieve the BAG ellipsoid name from the WKT definition.

\param wkt
    \li The wkt string containing the coordinate system definition.
\return
    \li The ellipsoid name.
*/
//************************************************************************
std::string getEllipsoid(const std::string &wkt)
{
    //Find the ellipsoid node in the wkt string.
    const size_t startIndex = wkt.find("spheroid[");
    if (startIndex == std::string::npos)
        throw InvalidDatumError();

    const size_t endIndex = wkt.find(",", startIndex);
    if (endIndex == std::string::npos)
        throw InvalidDatumError();

    //Extract the ellipsoid name.
    const size_t startPos = startIndex + 10;
    const size_t length = endIndex - startPos - 1;
    const std::string ellipName = wkt.substr(startPos, length);

    return ellipName;
}

//************************************************************************
//      Method name:    getVDatum()
//
//      - Initial implementation
//        Mike Van Duzee, 1/26/2012
//
//************************************************************************
/*!
\brief Retrieve the vertical datum name from the WKT definition.

\param wkt
    \li The wkt string containing the vertical reference system definition.
\return
    \li The vertical datum name.
*/
//************************************************************************
std::string getVDatum(const std::string &wkt)
{
    //Find the vertical datum node in the wkt string.
    const size_t startIndex = wkt.find("vert_datum[");
    if (startIndex == std::string::npos)
        throw InvalidDatumError();

    const size_t endIndex = wkt.find(",", startIndex);
    if (endIndex == std::string::npos)
        throw InvalidDatumError();

    //Extract the ellipsoid name.
    const size_t startPos = startIndex + 12;
    const size_t length = endIndex - startPos - 1;
    const std::string vDatumName = wkt.substr(startPos, length);

    return vDatumName;
}

}   //namespace

//************************************************************************
//      Method name:    bagLegacyToWkt()
//
//      - Initial implementation
//        Mike Van Duzee, 1/26/2012
//
//************************************************************************
/*!
\brief Convert a BAG coordinate system definition to wkt.

\param system
    \li The projection parameters.
\param hBuffer
    \li Modified to contain the horizontal reference system
    in the form of a WKT string.
\param hBuffer_size
    \li The size of the horizontal reference system buffer.
\param vBuffer
    \li Modified to contain the vertical reference system
    in the form of a WKT string.
\param vBuffer_size
    \li The size of the vertical reference system buffer.
\return
    \li 0 on success, else an error code.
*/
//************************************************************************
bagError bagLegacyToWkt(const bagLegacyReferenceSystem system,
                        char *hBuffer, u32 hBuffer_size,
                        char *vBuffer, u32 vBuffer_size)
try
{
    std::stringstream wktStream;
    wktStream.imbue(std::locale::classic());

    //If we want the vertical system then...
    if (vBuffer != NULL && vBuffer_size > 0 && strlen((char*)system.geoParameters.vertical_datum) != 0)
    {        
        wktStream << "VERT_CS[\"" << system.geoParameters.vertical_datum
            << "\", VERT_DATUM[\"" << system.geoParameters.vertical_datum << "\", 2000]]";

        //Make sure our string is not too large.
        if (wktStream.str().size() > vBuffer_size)
            wktStream.str().resize(vBuffer_size);

        strcpy(vBuffer, wktStream.str().c_str());
    }

    //If we want the horizontal system then...
    if (hBuffer != NULL && hBuffer_size > 0)
    {
        switch (system.coordSys)
        {
        case Geodetic:
            {
                wktStream << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid);
            }
            break;

        case UTM:
            {
                //We need to figure out what hemisphere we are in.
                bool isNorth = false;

                //A false northing of 0.0 means north.
                if (system.geoParameters.false_northing == 0.0)
                    isNorth = true;
                //A false northing of 10,000,000 means south.
                else if (system.geoParameters.false_northing == 10000000)
                    isNorth = false;
                //If we don't have an appropriate false northing, then use the zone.
                else
                    isNorth = (system.geoParameters.zone >= 0);

                wktStream << std::fixed << std::setprecision(6)
                    << "PROJCS[\"UTM Zone " << abs(system.geoParameters.zone)
                    << ((isNorth) ? ", Northern Hemisphere\"" : ", Southern Hemisphere\"")
                    << ", " << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid)
                    << ", PROJECTION[\"Transverse_Mercator\"],"
                    << " PARAMETER[\"latitude_of_origin\"," << 0 << "],"
                    << " PARAMETER[\"central_meridian\"," << (abs(system.geoParameters.zone) * 6 - 183) << "],"
                    << " PARAMETER[\"scale_factor\"," << 0.9996 << "],"
                    << " PARAMETER[\"false_easting\"," << 500000 << "],"
                    << " PARAMETER[\"false_northing\"," << ((isNorth) ? 0 : 10000000) << "],"
                    << " UNIT[\"metre\",1]]";
            }
            break;

        case Albers_Equal_Area_Conic:
            {
                wktStream << std::fixed << std::setprecision(6)
                    << "PROJCS[\"unnamed\""
                    << ", " << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid)
                    << ", PROJECTION[\"Albers_Conic_Equal_Area\"],"
                    << " PARAMETER[\"standard_parallel_1\"," << system.geoParameters.std_parallel_1 << "],"
                    << " PARAMETER[\"standard_parallel_2\"," << system.geoParameters.std_parallel_2 << "],"
                    << " PARAMETER[\"latitude_of_center\"," << system.geoParameters.latitude_of_centre << "],"
                    << " PARAMETER[\"longitude_of_center\"," << system.geoParameters.longitude_of_centre << "],"
                    << " PARAMETER[\"false_easting\"," << system.geoParameters.false_easting << "],"
                    << " PARAMETER[\"false_northing\"," << system.geoParameters.false_northing << "],"
                    << " UNIT[\"metre\",1]]";
            }
            break;

        case Azimuthal_Equidistant:
            {
                wktStream << std::fixed << std::setprecision(6)
                    << "PROJCS[\"unnamed\""
                    << ", " << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid)
                    << ", PROJECTION[\"Azimuthal_Equidistant\"],"
                    << " PARAMETER[\"latitude_of_center\"," << system.geoParameters.latitude_of_centre << "],"
                    << " PARAMETER[\"longitude_of_center\"," << system.geoParameters.longitude_of_centre << "],"
                    << " PARAMETER[\"false_easting\"," << system.geoParameters.false_easting << "],"
                    << " PARAMETER[\"false_northing\"," << system.geoParameters.false_northing << "],"
                    << " UNIT[\"metre\",1]]";
            }
            break;
        
        case Bonne:
            {
                wktStream << std::fixed << std::setprecision(6)
                    << "PROJCS[\"unnamed\""
                    << ", " << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid)
                    << ", PROJECTION[\"Bonne\"],"
                    << " PARAMETER[\"standard_parallel_1\"," << system.geoParameters.std_parallel_1 << "],"
                    << " PARAMETER[\"central_meridian\"," << system.geoParameters.central_meridian << "],"
                    << " PARAMETER[\"false_easting\"," << system.geoParameters.false_easting << "],"
                    << " PARAMETER[\"false_northing\"," << system.geoParameters.false_northing << "],"
                    << " UNIT[\"metre\",1]]";
            }
            break;

        case Cassini:
            {
                wktStream << std::fixed << std::setprecision(6)
                    << "PROJCS[\"unnamed\""
                    << ", " << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid)
                    << ", PROJECTION[\"Cassini_Soldner\"],"
                    << " PARAMETER[\"latitude_of_origin\"," << system.geoParameters.origin_latitude << "],"
                    << " PARAMETER[\"central_meridian\"," << system.geoParameters.central_meridian << "],"
                    << " PARAMETER[\"false_easting\"," << system.geoParameters.false_easting << "],"
                    << " PARAMETER[\"false_northing\"," << system.geoParameters.false_northing << "],"
                    << " UNIT[\"metre\",1]]";
            }
            break;

        case Cylindrical_Equal_Area:
            {
                wktStream << std::fixed << std::setprecision(6)
                    << "PROJCS[\"unnamed\""
                    << ", " << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid)
                    << ", PROJECTION[\"Cylindrical_Equal_Area\"],"
                    << " PARAMETER[\"standard_parallel_1\"," << system.geoParameters.std_parallel_1 << "],"
                    << " PARAMETER[\"central_meridian\"," << system.geoParameters.central_meridian << "],"
                    << " PARAMETER[\"false_easting\"," << system.geoParameters.false_easting << "],"
                    << " PARAMETER[\"false_northing\"," << system.geoParameters.false_northing << "],"
                    << " UNIT[\"metre\",1]]";
            }
            break;

        case Eckert4:
            {
                wktStream << std::fixed << std::setprecision(6)
                    << "PROJCS[\"unnamed\""
                    << ", " << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid)
                    << ", PROJECTION[\"Eckert_IV\"],"
                    << " PARAMETER[\"central_meridian\"," << system.geoParameters.central_meridian << "],"
                    << " PARAMETER[\"false_easting\"," << system.geoParameters.false_easting << "],"
                    << " PARAMETER[\"false_northing\"," << system.geoParameters.false_northing << "],"
                    << " UNIT[\"metre\",1]]";
            }
            break;

        case Eckert6:
            {
                wktStream << std::fixed << std::setprecision(6)
                    << "PROJCS[\"unnamed\""
                    << ", " << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid)
                    << ", PROJECTION[\"Eckert_VI\"],"
                    << " PARAMETER[\"central_meridian\"," << system.geoParameters.central_meridian << "],"
                    << " PARAMETER[\"false_easting\"," << system.geoParameters.false_easting << "],"
                    << " PARAMETER[\"false_northing\"," << system.geoParameters.false_northing << "],"
                    << " UNIT[\"metre\",1]]";
            }
            break;

        case Equidistant_Cylindrical:
            {
                //Plate Caree, 	Equidistant Cylindrical, and Simple Cylindrical are all
                //aliases for Equirectangular.

                wktStream << std::fixed << std::setprecision(6)
                    << "PROJCS[\"unnamed\""
                    << ", " << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid)
                    << ", PROJECTION[\"Equirectangular\"],"
                    << " PARAMETER[\"latitude_of_origin\"," << system.geoParameters.origin_latitude << "],"
                    << " PARAMETER[\"central_meridian\"," << system.geoParameters.central_meridian << "],"
                    << " PARAMETER[\"false_easting\"," << system.geoParameters.false_easting << "],"
                    << " PARAMETER[\"false_northing\"," << system.geoParameters.false_northing << "],"
                    << " UNIT[\"metre\",1]]";
            }
            break;

        case Gnomonic:
            {
                wktStream << std::fixed << std::setprecision(6)
                    << "PROJCS[\"unnamed\""
                    << ", " << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid)
                    << ", PROJECTION[\"Gnomonic\"],"
                    << " PARAMETER[\"latitude_of_origin\"," << system.geoParameters.origin_latitude << "],"
                    << " PARAMETER[\"central_meridian\"," << system.geoParameters.central_meridian << "],"
                    << " PARAMETER[\"false_easting\"," << system.geoParameters.false_easting << "],"
                    << " PARAMETER[\"false_northing\"," << system.geoParameters.false_northing << "],"
                    << " UNIT[\"metre\",1]]";
            }
            break;

        case Lambert_Conformal_Conic:
            {
                wktStream << std::fixed << std::setprecision(6)
                    << "PROJCS[\"unnamed\""
                    << ", " << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid)
                    << ", PROJECTION[\"Lambert_Conformal_Conic_2SP\"],"
                    << " PARAMETER[\"standard_parallel_1\"," << system.geoParameters.std_parallel_1 << "],"
                    << " PARAMETER[\"standard_parallel_2\"," << system.geoParameters.std_parallel_2 << "],"
                    << " PARAMETER[\"latitude_of_origin\"," << system.geoParameters.origin_latitude << "],"
                    << " PARAMETER[\"central_meridian\"," << system.geoParameters.central_meridian << "],"
                    << " PARAMETER[\"false_easting\"," << system.geoParameters.false_easting << "],"
                    << " PARAMETER[\"false_northing\"," << system.geoParameters.false_northing << "],"
                    << " UNIT[\"metre\",1]]";
            }
            break;

        case Mercator:
            {
                wktStream << std::fixed << std::setprecision(6)
                    << "PROJCS[\"unnamed\""
                    << ", " << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid)
                    << ", PROJECTION[\"Mercator_1SP\"],"
                    << " PARAMETER[\"latitude_of_origin\"," << system.geoParameters.origin_latitude << "],"
                    << " PARAMETER[\"central_meridian\"," << system.geoParameters.central_meridian << "],"
                    << " PARAMETER[\"scale_factor\"," << system.geoParameters.scale_factor << "],"
                    << " PARAMETER[\"false_easting\"," << system.geoParameters.false_easting << "],"
                    << " PARAMETER[\"false_northing\"," << system.geoParameters.false_northing << "],"
                    << " UNIT[\"metre\",1]]";
            }
            break;

        case Miller_Cylindrical:
            {
                wktStream << std::fixed << std::setprecision(6)
                    << "PROJCS[\"unnamed\""
                    << ", " << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid)
                    << ", PROJECTION[\"Miller_Cylindrical\"],"
                    << " PARAMETER[\"latitude_of_center\"," << system.geoParameters.latitude_of_centre << "],"
                    << " PARAMETER[\"longitude_of_center\"," << system.geoParameters.longitude_of_centre << "],"
                    << " PARAMETER[\"false_easting\"," << system.geoParameters.false_easting << "],"
                    << " PARAMETER[\"false_northing\"," << system.geoParameters.false_northing << "],"
                    << " UNIT[\"metre\",1]]";
            }
            break;

        case Mollweide:
            {
                wktStream << std::fixed << std::setprecision(6)
                    << "PROJCS[\"unnamed\""
                    << ", " << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid)
                    << ", PROJECTION[\"Mollweide\"],"
                    << " PARAMETER[\"central_meridian\"," << system.geoParameters.central_meridian << "],"
                    << " PARAMETER[\"false_easting\"," << system.geoParameters.false_easting << "],"
                    << " PARAMETER[\"false_northing\"," << system.geoParameters.false_northing << "],"
                    << " UNIT[\"metre\",1]]";
            }
            break;

        case NZMG:
            {
                wktStream << std::fixed << std::setprecision(6)
                    << "PROJCS[\"unnamed\""
                    << ", " << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid)
                    << ", PROJECTION[\"New_Zealand_Map_Grid\"],"
                    << " PARAMETER[\"latitude_of_origin\"," << system.geoParameters.origin_latitude << "],"
                    << " PARAMETER[\"central_meridian\"," << system.geoParameters.central_meridian << "],"
                    << " PARAMETER[\"false_easting\"," << system.geoParameters.false_easting << "],"
                    << " PARAMETER[\"false_northing\"," << system.geoParameters.false_northing << "],"
                    << " UNIT[\"metre\",1]]";
            }
            break;

        case Orthographic:
            {
                wktStream << std::fixed << std::setprecision(6)
                    << "PROJCS[\"unnamed\""
                    << ", " << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid)
                    << ", PROJECTION[\"Orthographic\"],"
                    << " PARAMETER[\"latitude_of_origin\"," << system.geoParameters.origin_latitude << "],"
                    << " PARAMETER[\"central_meridian\"," << system.geoParameters.central_meridian << "],"
                    << " PARAMETER[\"false_easting\"," << system.geoParameters.false_easting << "],"
                    << " PARAMETER[\"false_northing\"," << system.geoParameters.false_northing << "],"
                    << " UNIT[\"metre\",1]]";
            }
            break;

        case Polar_Stereo:
            {
                wktStream << std::fixed << std::setprecision(6)
                    << "PROJCS[\"unnamed\""
                    << ", " << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid)
                    << ", PROJECTION[\"Polar_Stereographic\"],"
                    << " PARAMETER[\"latitude_of_origin\"," << system.geoParameters.origin_latitude << "],"
                    << " PARAMETER[\"central_meridian\"," << system.geoParameters.central_meridian << "],"
                    << " PARAMETER[\"scale_factor\"," << system.geoParameters.scale_factor << "],"
                    << " PARAMETER[\"false_easting\"," << system.geoParameters.false_easting << "],"
                    << " PARAMETER[\"false_northing\"," << system.geoParameters.false_northing << "],"
                    << " UNIT[\"metre\",1]]";
            }
            break;

        case Polyconic:
            {
                wktStream << std::fixed << std::setprecision(6)
                    << "PROJCS[\"unnamed\""
                    << ", " << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid)
                    << ", PROJECTION[\"Polyconic\"],"
                    << " PARAMETER[\"latitude_of_origin\"," << system.geoParameters.origin_latitude << "],"
                    << " PARAMETER[\"central_meridian\"," << system.geoParameters.central_meridian << "],"
                    << " PARAMETER[\"false_easting\"," << system.geoParameters.false_easting << "],"
                    << " PARAMETER[\"false_northing\"," << system.geoParameters.false_northing << "],"
                    << " UNIT[\"metre\",1]]";
            }
            break;

        case Sinusoidal:
            {
                wktStream << std::fixed << std::setprecision(6)
                    << "PROJCS[\"unnamed\""
                    << ", " << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid)
                    << ", PROJECTION[\"Sinusoidal\"],"
                    << " PARAMETER[\"longitude_of_center\"," << system.geoParameters.longitude_of_centre << "],"
                    << " PARAMETER[\"false_easting\"," << system.geoParameters.false_easting << "],"
                    << " PARAMETER[\"false_northing\"," << system.geoParameters.false_northing << "],"
                    << " UNIT[\"metre\",1]]";
            }
            break;

        case Stereographic:
            {
                wktStream << std::fixed << std::setprecision(6)
                    << "PROJCS[\"unnamed\""
                    << ", " << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid)
                    << ", PROJECTION[\"Oblique_Stereographic\"],"
                    << " PARAMETER[\"latitude_of_origin\"," << system.geoParameters.origin_latitude << "],"
                    << " PARAMETER[\"central_meridian\"," << system.geoParameters.central_meridian << "],"
                    << " PARAMETER[\"scale_factor\"," << system.geoParameters.scale_factor << "],"
                    << " PARAMETER[\"false_easting\"," << system.geoParameters.false_easting << "],"
                    << " PARAMETER[\"false_northing\"," << system.geoParameters.false_northing << "],"
                    << " UNIT[\"metre\",1]]";
            }
            break;

        case Transverse_Mercator:
            {
                wktStream << std::fixed << std::setprecision(6)
                    << "PROJCS[\"unnamed\""
                    << ", " << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid)
                    << ", PROJECTION[\"Transverse_Mercator\"],"
                    << " PARAMETER[\"latitude_of_origin\"," << system.geoParameters.origin_latitude << "],"
                    << " PARAMETER[\"central_meridian\"," << system.geoParameters.central_meridian << "],"
                    << " PARAMETER[\"scale_factor\"," << system.geoParameters.scale_factor << "],"
                    << " PARAMETER[\"false_easting\"," << system.geoParameters.false_easting << "],"
                    << " PARAMETER[\"false_northing\"," << system.geoParameters.false_northing << "],"
                    << " UNIT[\"metre\",1]]";
            }
            break;

        case Van_der_Grinten:
            {
                wktStream << std::fixed << std::setprecision(6)
                    << "PROJCS[\"unnamed\""
                    << ", " << datumToWkt(system.geoParameters.datum, system.geoParameters.ellipsoid)
                    << ", PROJECTION[\"VanDerGrinten\"],"
                    << " PARAMETER[\"central_meridian\"," << system.geoParameters.central_meridian << "],"
                    << " PARAMETER[\"false_easting\"," << system.geoParameters.false_easting << "],"
                    << " PARAMETER[\"false_northing\"," << system.geoParameters.false_northing << "],"
                    << " UNIT[\"metre\",1]]";
            }
            break;

        default:
            {
                //Currently unsupported type.
                return BAG_METADTA_INVALID_PROJECTION;
            }

        };

        if (wktStream.str().size() > hBuffer_size)
            wktStream.str().resize(hBuffer_size);

        strcpy(hBuffer, wktStream.str().c_str());
    }

    return 0;
}
catch (const InvalidDatumError &/*e*/)
{
    return BAG_METADTA_INVALID_DATUM;
}
catch (const std::exception &/*e*/)
{
    //Something bad happened.
    return BAG_METADTA_INVALID_PROJECTION;
}

//************************************************************************
//      Method name:    bagWktToLegacy()
//
//      - Initial implementation
//        Mike Van Duzee, 1/26/2012
//
//************************************************************************
/*!
\brief Convert a WKT string into an internal BAG coordinate system definition.

\param horiz_wkt
    \li The horizontal reference WKT (Well Known Text) string to convert.
\param vert_wkt
    \li The vertical reference WKT (Well Known Text) string to convert.
\param system
    \li Modified to contain the projection parameters.
\return
    \li 0 on success, else an error code.
*/
//************************************************************************
bagError bagWktToLegacy(const char *horiz_wkt, const char *vert_wkt,
                        bagLegacyReferenceSystem *system)
try
{
    //First clear the structure.
    memset(system, 0, sizeof(bagLegacyReferenceSystem));

    //If we have a vertical wkt string
    if (vert_wkt != NULL && strlen(vert_wkt) != 0)
    {
        //First make the wkt string lower case.
        std::string vWktLower(vert_wkt);
        std::transform(vWktLower.begin(), vWktLower.end(), vWktLower.begin(), ::tolower);

        std::string vDatum = getVDatum(vWktLower);
        if (vDatum.size() > 256)
            vDatum.resize(256);
        strcpy((char *)system->geoParameters.vertical_datum, vDatum.c_str());
    }

    //If we have a horizontal wkt string
    if (horiz_wkt != NULL && strlen(horiz_wkt) != 0)
    {
        //First make the wkt string lower case.
        std::string hWktLower(horiz_wkt);
        std::transform(hWktLower.begin(), hWktLower.end(), hWktLower.begin(), ::tolower);

        //Find our datum type.
        system->geoParameters.datum = getDatumType(hWktLower);

        //Find out ellipsoid.
        std::string ellipName = getEllipsoid(hWktLower);
        if (ellipName.size() > 256)
            ellipName.resize(256);
        strcpy((char *)system->geoParameters.ellipsoid, ellipName.c_str());

        //Find our coordinate system type.
        system->coordSys = getCoordinateType(hWktLower);

        switch (system->coordSys)
        {
        case UTM:
        case Transverse_Mercator:
            {
                system->geoParameters.origin_latitude = getProjectionParam(hWktLower, k_latitude_of_origin);
                system->geoParameters.central_meridian = getProjectionParam(hWktLower, k_central_meridian);
                system->geoParameters.scale_factor = getProjectionParam(hWktLower, k_scale_factor);
                system->geoParameters.false_easting = getProjectionParam(hWktLower, k_false_easting);
                system->geoParameters.false_northing = getProjectionParam(hWktLower, k_false_northing);

                //This may actuall be UTM, if so we will set the utm zone too.
                const int utmZone = static_cast<int>((system->geoParameters.central_meridian * PI_OVER_180 + PI) / (PI / 30.0) + 1);
                const double utmCentralMeridian = utmZone * 6.0 - 183.0;

                if ((system->geoParameters.origin_latitude == 0.0) &&
                    (system->geoParameters.scale_factor == 0.9996) &&
                    (system->geoParameters.false_easting == 500000.0) &&
                    (system->geoParameters.central_meridian == utmCentralMeridian))
                {
                    system->coordSys = UTM;
                    system->geoParameters.zone = utmZone;
                }
            }
            break;

        case Albers_Equal_Area_Conic:
            {
                system->geoParameters.std_parallel_1 = getProjectionParam(hWktLower, k_standard_parallel_1);
                system->geoParameters.std_parallel_2 = getProjectionParam(hWktLower, k_standard_parallel_2);
                system->geoParameters.latitude_of_centre = getProjectionParam(hWktLower, k_latitude_of_center);
                system->geoParameters.longitude_of_centre = getProjectionParam(hWktLower, k_longitude_of_center);
                system->geoParameters.false_easting = getProjectionParam(hWktLower, k_false_easting);
                system->geoParameters.false_northing = getProjectionParam(hWktLower, k_false_northing);
            }
            break;

        case Azimuthal_Equidistant:
            {
                system->geoParameters.latitude_of_centre = getProjectionParam(hWktLower, k_latitude_of_center);
                system->geoParameters.longitude_of_centre = getProjectionParam(hWktLower, k_longitude_of_center);
                system->geoParameters.false_easting = getProjectionParam(hWktLower, k_false_easting);
                system->geoParameters.false_northing = getProjectionParam(hWktLower, k_false_northing);
            }
            break;
        
        case Bonne:
            {
                system->geoParameters.std_parallel_1 = getProjectionParam(hWktLower, k_standard_parallel_1);
                system->geoParameters.central_meridian = getProjectionParam(hWktLower, k_central_meridian);
                system->geoParameters.false_easting = getProjectionParam(hWktLower, k_false_easting);
                system->geoParameters.false_northing = getProjectionParam(hWktLower, k_false_northing);
            }
            break;

        case Cassini:
            {
                system->geoParameters.origin_latitude = getProjectionParam(hWktLower, k_latitude_of_origin);
                system->geoParameters.central_meridian = getProjectionParam(hWktLower, k_central_meridian);
                system->geoParameters.false_easting = getProjectionParam(hWktLower, k_false_easting);
                system->geoParameters.false_northing = getProjectionParam(hWktLower, k_false_northing);
            }
            break;

        case Cylindrical_Equal_Area:
            {
                system->geoParameters.std_parallel_1 = getProjectionParam(hWktLower, k_standard_parallel_1);
                system->geoParameters.central_meridian = getProjectionParam(hWktLower, k_central_meridian);
                system->geoParameters.false_easting = getProjectionParam(hWktLower, k_false_easting);
                system->geoParameters.false_northing = getProjectionParam(hWktLower, k_false_northing);
            }
            break;

        case Eckert4:
            {
                system->geoParameters.central_meridian = getProjectionParam(hWktLower, k_central_meridian);
                system->geoParameters.false_easting = getProjectionParam(hWktLower, k_false_easting);
                system->geoParameters.false_northing = getProjectionParam(hWktLower, k_false_northing);
            }
            break;

        case Eckert6:
            {
                system->geoParameters.central_meridian = getProjectionParam(hWktLower, k_central_meridian);
                system->geoParameters.false_easting = getProjectionParam(hWktLower, k_false_easting);
                system->geoParameters.false_northing = getProjectionParam(hWktLower, k_false_northing);
            }
            break;

        case Equidistant_Cylindrical:
            {
                system->geoParameters.origin_latitude = getProjectionParam(hWktLower, k_latitude_of_origin);
                system->geoParameters.central_meridian = getProjectionParam(hWktLower, k_central_meridian);
                system->geoParameters.false_easting = getProjectionParam(hWktLower, k_false_easting);
                system->geoParameters.false_northing = getProjectionParam(hWktLower, k_false_northing);
            }
            break;

        case Gnomonic:
            {
                system->geoParameters.origin_latitude = getProjectionParam(hWktLower, k_latitude_of_origin);
                system->geoParameters.central_meridian = getProjectionParam(hWktLower, k_central_meridian);
                system->geoParameters.false_easting = getProjectionParam(hWktLower, k_false_easting);
                system->geoParameters.false_northing = getProjectionParam(hWktLower, k_false_northing);
            }
            break;

        case Lambert_Conformal_Conic:
            {
                system->geoParameters.std_parallel_1 = getProjectionParam(hWktLower, k_standard_parallel_1);
                system->geoParameters.std_parallel_2 = getProjectionParam(hWktLower, k_standard_parallel_2);
                system->geoParameters.origin_latitude = getProjectionParam(hWktLower, k_latitude_of_origin);
                system->geoParameters.central_meridian = getProjectionParam(hWktLower, k_central_meridian);
                system->geoParameters.false_easting = getProjectionParam(hWktLower, k_false_easting);
                system->geoParameters.false_northing = getProjectionParam(hWktLower, k_false_northing);
            }
            break;

        case Mercator:
            {
                system->geoParameters.origin_latitude = getProjectionParam(hWktLower, k_latitude_of_origin);
                system->geoParameters.central_meridian = getProjectionParam(hWktLower, k_central_meridian);
                system->geoParameters.scale_factor = getProjectionParam(hWktLower, k_scale_factor);
                system->geoParameters.false_easting = getProjectionParam(hWktLower, k_false_easting);
                system->geoParameters.false_northing = getProjectionParam(hWktLower, k_false_northing);
            }
            break;

        case Miller_Cylindrical:
            {
                system->geoParameters.latitude_of_centre = getProjectionParam(hWktLower, k_latitude_of_center);
                system->geoParameters.longitude_of_centre = getProjectionParam(hWktLower, k_longitude_of_center);
                system->geoParameters.false_easting = getProjectionParam(hWktLower, k_false_easting);
                system->geoParameters.false_northing = getProjectionParam(hWktLower, k_false_northing);
            }
            break;

        case Mollweide:
            {
                system->geoParameters.central_meridian = getProjectionParam(hWktLower, k_central_meridian);
                system->geoParameters.false_easting = getProjectionParam(hWktLower, k_false_easting);
                system->geoParameters.false_northing = getProjectionParam(hWktLower, k_false_northing);
            }
            break;

        case NZMG:
            {
                system->geoParameters.origin_latitude = getProjectionParam(hWktLower, k_latitude_of_origin);
                system->geoParameters.central_meridian = getProjectionParam(hWktLower, k_central_meridian);
                system->geoParameters.false_easting = getProjectionParam(hWktLower, k_false_easting);
                system->geoParameters.false_northing = getProjectionParam(hWktLower, k_false_northing);
            }
            break;

        case Orthographic:
            {
                system->geoParameters.origin_latitude = getProjectionParam(hWktLower, k_latitude_of_origin);
                system->geoParameters.central_meridian = getProjectionParam(hWktLower, k_central_meridian);
                system->geoParameters.false_easting = getProjectionParam(hWktLower, k_false_easting);
                system->geoParameters.false_northing = getProjectionParam(hWktLower, k_false_northing);
            }
            break;

        case Polar_Stereo:
            {
                system->geoParameters.origin_latitude = getProjectionParam(hWktLower, k_latitude_of_origin);
                system->geoParameters.central_meridian = getProjectionParam(hWktLower, k_central_meridian);
                system->geoParameters.scale_factor = getProjectionParam(hWktLower, k_scale_factor);
                system->geoParameters.false_easting = getProjectionParam(hWktLower, k_false_easting);
                system->geoParameters.false_northing = getProjectionParam(hWktLower, k_false_northing);
            }
            break;

        case Polyconic:
            {
                system->geoParameters.origin_latitude = getProjectionParam(hWktLower, k_latitude_of_origin);
                system->geoParameters.central_meridian = getProjectionParam(hWktLower, k_central_meridian);
                system->geoParameters.false_easting = getProjectionParam(hWktLower, k_false_easting);
                system->geoParameters.false_northing = getProjectionParam(hWktLower, k_false_northing);
            }
            break;

        case Sinusoidal:
            {
                system->geoParameters.longitude_of_centre = getProjectionParam(hWktLower, k_longitude_of_center);
                system->geoParameters.false_easting = getProjectionParam(hWktLower, k_false_easting);
                system->geoParameters.false_northing = getProjectionParam(hWktLower, k_false_northing);
            }
            break;

        case Stereographic:
            {
                system->geoParameters.origin_latitude = getProjectionParam(hWktLower, k_latitude_of_origin);
                system->geoParameters.central_meridian = getProjectionParam(hWktLower, k_central_meridian);
                system->geoParameters.scale_factor = getProjectionParam(hWktLower, k_scale_factor);
                system->geoParameters.false_easting = getProjectionParam(hWktLower, k_false_easting);
                system->geoParameters.false_northing = getProjectionParam(hWktLower, k_false_northing);
            }
            break;

        case Van_der_Grinten:
            {
                system->geoParameters.central_meridian = getProjectionParam(hWktLower, k_central_meridian);
                system->geoParameters.false_easting = getProjectionParam(hWktLower, k_false_easting);
                system->geoParameters.false_northing = getProjectionParam(hWktLower, k_false_northing);
            }
            break;

        default:
            {
                //Currently unsupported type.
                return BAG_METADTA_INVALID_PROJECTION;
            }
        }
    }

    return 0;
}
catch (const InvalidDatumError &/*e*/)
{
    return BAG_METADTA_INVALID_DATUM;
}
catch (const std::exception &/*e*/)
{
    //Something bad happened
    return BAG_METADTA_INVALID_PROJECTION;
}
