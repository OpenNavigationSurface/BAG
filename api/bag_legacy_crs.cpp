
#include "bag_errors.h"
#include "bag_exceptions.h"
#include "bag_legacy_crs.h"

#include <algorithm>  // std::transform
#include <cctype>  // std::tolower
#include <cmath>  // M_PI
#include <cstring>
#include <exception>  // std::exception
#include <fstream>  // std::ifstream
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


namespace {

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

//! Split a string by the specified separator.
/*!
\param str
    The string to split.
\param separator
    The separator to split the string by.
    Defaults to space.

\return
    A list of words split by separator.
*/
std::vector<std::string> split(
    const std::string& str,
    const char separator = ' ')
{
    std::vector<std::string> tokens;

    const auto end = cend(str);
    auto p1 = cbegin(str);
    auto p2 = end;

    do
    {
        p2 = std::find(p1, end, separator);  // find next separator
        tokens.emplace_back(p1, p2);

        if (p2 != end)
        {   // More tokens left.
            p1 = std::find_if(p2, end, [separator](char c) -> bool {
                return c != separator;  // skip all separators
            });
        }
    } while (p2 != end);

    return tokens;
}

//************************************************************************
/*!
\brief Convert the given string value to double.

The string is expected to contain a decimal value in the classic locale.
For example something like "20.0". The classic locale uses a '.' as the
decimal separator.

\param value
    \li The string to convert.
\return
    \li The converted value.
*/
//************************************************************************
double toDouble(const std::string& value)
{
    std::stringstream lineStream;

    (void)lineStream.imbue(std::locale::classic());
    lineStream << value;

    double dblValue = 0.0;
    lineStream >> dblValue;

    return dblValue;
}

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
std::string ellipsoidToWkt(const char* ellipsoid)
{
    const char* onsHome = getenv("BAG_HOME");
    if (!onsHome)
        throw BAG::InvalidEllipsoidError();

    //Make the ellipsoid name all lower case so we can find it.
    std::string ellipsoidName{ellipsoid};
    std::transform(begin(ellipsoidName), end(ellipsoidName),
        begin(ellipsoidName),
        [](unsigned char c) noexcept {
            return static_cast<char>(std::tolower(c));
        });

    //Build the full path to the ellips.dat file.
    std::string ellipFile{onsHome};
    ellipFile += "/ellips.dat";

    std::ifstream file{ellipFile.c_str()};
    if (!file.is_open())
        throw BAG::InvalidEllipsoidError();

    while (file.good())
    {
        //Get the current line in lower case.
        std::string line;
        std::getline(file, line);

        std::transform(begin(line), end(line), begin(line),
            [](unsigned char c) noexcept {
                return static_cast<char>(std::tolower(c));
            });

        const size_t index = line.find(ellipsoidName);
        if (index != 0)
            continue;

        auto elements = split(line, ' ');
        const auto numItems = elements.size();

        //We MUST have at least 5 elements (name, id, a, b, if)
        if (numItems < 5)
            throw BAG::InvalidEllipsoidError();

        //The last item will be the inverse flattening.
        const double invFlat = atof(elements[numItems - 1].c_str());

        //The third to last item will be the semi-major.
        const double semiMajor = atof(elements[numItems - 3].c_str());

        std::stringstream wktStream;
        (void)wktStream.imbue(std::locale::classic());
        wktStream << std::fixed << std::setprecision(9) <<
            R"(SPHEROID[")" << ellipsoid << R"(",)" <<
            semiMajor << ',' << invFlat << ']';

        return wktStream.str();
    }

    //Couldn't find it :(
    throw BAG::InvalidEllipsoidError();
}

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
std::string datumToWkt(
    const BAG::BagDatum datum,
    const char* ellipsoid)
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
        case BAG::BagDatum::wgs84:
            ellipWkt = R"(SPHEROID["WGS 84",6378137,298.257223563])";
            break;

        case BAG::BagDatum::wgs72:
            ellipWkt = R"(SPHEROID["WGS 72",6378135,298.26])";
            break;

        case BAG::BagDatum::nad83:
            ellipWkt = R"(SPHEROID["GRS 1980",6378137,298.257222101])";
        }
    };

    std::stringstream wktStream;
    (void)wktStream.imbue(std::locale::classic());

    switch (datum)
    {
    case BAG::BagDatum::wgs84:
    {
        wktStream << R"(GEOGCS["WGS 84", )"
            << R"(DATUM["WGS_1984", )"
            << ellipWkt << ", "
            << "TOWGS84[0,0,0,0,0,0,0]], "
            << R"(PRIMEM["Greenwich",0], )"
            << R"(UNIT["degree",0.0174532925199433]])";

        return wktStream.str();
    }
    break;

    case BAG::BagDatum::wgs72:
    {
        wktStream << R"(GEOGCS["WGS 72", )"
            << R"(DATUM["WGS_1972", )"
            << ellipWkt << ", "
            << "TOWGS84[0,0,4.5,0,0,0.554,0.2263]], "
            << R"(PRIMEM["Greenwich",0], )"
            << R"(UNIT["degree",0.0174532925199433]])";

        return wktStream.str();
    }
    break;

    case BAG::BagDatum::nad83:
    {
        wktStream << R"(GEOGCS["NAD83", )"
            << R"(DATUM["North_American_Datum_1983", )"
            << ellipWkt << ", "
            << "TOWGS84[0,0,0,0,0,0,0]], "
            << R"(PRIMEM["Greenwich",0], )"
            << R"(UNIT["degree",0.0174532925199433]])";

        return wktStream.str();
    }
    break;
    }

    //If we got here then we don't know what type of datum we have.
    throw BAG::InvalidDatumError();
}

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
double getProjectionParam(
    const std::string& wkt,
    const std::string& paramName)
{
    //Find the projection node in the wkt string.
    const size_t startIndex = wkt.find(paramName);
    if (startIndex == std::string::npos)
        throw BAG::CoordSysError();

    const size_t valueStartIndex = wkt.find(",", startIndex);
    if (valueStartIndex == std::string::npos)
        throw BAG::CoordSysError();

    const size_t valueEndIndex = wkt.find("]", valueStartIndex);
    if (valueEndIndex == std::string::npos)
        throw BAG::CoordSysError();

    //Extract the value
    const size_t startPos = valueStartIndex + 1;
    const size_t length = valueEndIndex - startPos;
    const std::string value = wkt.substr(startPos, length);

    return toDouble(value);
}

//************************************************************************
/*!
\brief Retrieve the BAG coordinate type from the WKT definition.

\param wkt
    \li The wkt string containing the coordinate system definition.
\return
    \li The BAG coordiante system type.
*/
//************************************************************************
BAG::CoordinateType getCoordinateType(const std::string& wkt)
{
    //Find the projection node in the wkt string.
    const size_t startIndex = wkt.find("projection[");

    //If no projection node, then we must have a Geographic system.
    if (startIndex == std::string::npos)
        return BAG::CoordinateType::Geodetic;

    const size_t endIndex = wkt.find(R"("])", startIndex);
    if (endIndex == std::string::npos)
        throw BAG::CoordSysError();

    //Extract the projection name.
    const size_t startPos = startIndex + 12;
    const size_t length = endIndex - startPos;
    const std::string projName = wkt.substr(startPos, length);

    if (projName == k_albers_conic_equal_area)
        return BAG::CoordinateType::Albers_Equal_Area_Conic;
    else if (projName == k_azimuthal_equidistant)
        return BAG::CoordinateType::Azimuthal_Equidistant;
    else if (projName == k_bonne)
        return BAG::CoordinateType::Bonne;
    else if (projName == k_cassini_soldner)
        return BAG::CoordinateType::Cassini;
    else if (projName == k_cylindrical_equal_area)
        return BAG::CoordinateType::Cylindrical_Equal_Area;
    else if (projName == k_eckert_iv)
        return BAG::CoordinateType::Eckert4;
    else if (projName == k_eckert_vi)
        return BAG::CoordinateType::Eckert6;
    else if (projName == k_equirectangular)
        return BAG::CoordinateType::Equidistant_Cylindrical;
    else if (projName == k_gnomonic)
        return BAG::CoordinateType::Gnomonic;
    else if (projName == k_lambert_conformal_conic)
        return BAG::CoordinateType::Lambert_Conformal_Conic;
    else if (projName == k_mercator)
        return BAG::CoordinateType::Mercator;
    else if (projName == k_miller_cylindrical)
        return BAG::CoordinateType::Miller_Cylindrical;
    else if (projName == k_mollweide)
        return BAG::CoordinateType::Mollweide;
    else if (projName == k_new_zealand_map_grid)
        return BAG::CoordinateType::NZMG;
    else if (projName == k_orthographic)
        return BAG::CoordinateType::Orthographic;
    else if (projName == k_polar_stereographic)
        return BAG::CoordinateType::Polar_Stereo;
    else if (projName == k_polyconic)
        return BAG::CoordinateType::Polyconic;
    else if (projName == k_sinusoidal)
        return BAG::CoordinateType::Sinusoidal;
    else if (projName == k_oblique_stereographic)
        return BAG::CoordinateType::Stereographic;
    else if (projName == k_transverse_mercator)
        return BAG::CoordinateType::Transverse_Mercator;
    else if (projName == k_vandergrinten)
        return BAG::CoordinateType::Van_der_Grinten;

    //No idea...
    throw BAG::CoordSysError();
}

//************************************************************************
/*!
\brief Retrieve the BAG datum type from the WKT definition.

\param wkt
    \li The wkt string containing the coordinate system definition.
\return
    \li The BAG datum type.
*/
//************************************************************************
BAG::BagDatum getDatumType(const std::string &wkt)
{
    //Find the horizontal datum node in the wkt string.
    const size_t startIndex = wkt.find("datum[");
    if (startIndex == std::string::npos)
        throw BAG::InvalidDatumError();

    const size_t endIndex = wkt.find(",", startIndex);
    if (endIndex == std::string::npos)
        throw BAG::InvalidDatumError();

    //Extract the horizontal datum name.
    const size_t startPos = startIndex + 7;
    const size_t length = endIndex - startPos - 1;
    const std::string hDatumName = wkt.substr(startPos, length);

    if (hDatumName == k_wgs84)
        return BAG::BagDatum::wgs84;
    else if (hDatumName == k_wgs72)
        return BAG::BagDatum::wgs72;
    else if (hDatumName == k_nad83)
        return BAG::BagDatum::nad83;

    //Unknown, so we can not convert this coordinate system.
    throw BAG::InvalidDatumError();
}

//************************************************************************
/*!
\brief Retrieve the BAG ellipsoid name from the WKT definition.

\param wkt
    \li The wkt string containing the coordinate system definition.
\return
    \li The ellipsoid name.
*/
//************************************************************************
std::string getEllipsoid(const std::string& wkt)
{
    //Find the ellipsoid node in the wkt string.
    const size_t startIndex = wkt.find("spheroid[");
    if (startIndex == std::string::npos)
        throw BAG::InvalidDatumError();

    const size_t endIndex = wkt.find(",", startIndex);
    if (endIndex == std::string::npos)
        throw BAG::InvalidDatumError();

    //Extract the ellipsoid name.
    const size_t startPos = startIndex + 10;
    const size_t length = endIndex - startPos - 1;

    return wkt.substr(startPos, length);
}

//************************************************************************
/*!
\brief Retrieve the vertical datum name from the WKT definition.

\param wkt
    \li The wkt string containing the vertical reference system definition.
\return
    \li The vertical datum name.
*/
//************************************************************************
std::string getVDatum(const std::string& wkt)
{
    //Find the vertical datum node in the wkt string.
    const size_t startIndex = wkt.find("vert_datum[");
    if (startIndex == std::string::npos)
        throw BAG::InvalidDatumError();

    const size_t endIndex = wkt.find(",", startIndex);
    if (endIndex == std::string::npos)
        throw BAG::InvalidDatumError();

    //Extract the vertical datum name.
    const size_t startPos = startIndex + 12;
    const size_t length = endIndex - startPos - 1;

    return wkt.substr(startPos, length);
}

}  // namespace

namespace BAG {

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
\param hBufferSize
\li The size of the horizontal reference system buffer.
\param vBuffer
\li Modified to contain the vertical reference system
in the form of a WKT string.
\param vBufferSize
\li The size of the vertical reference system buffer.
\return
\li 0 on success, else an error code.
*/
//************************************************************************
BagError bagLegacyToWkt(
    const BagLegacyReferenceSystem& system,
    char* hBuffer,
    size_t hBufferSize,
    char* vBuffer,
    size_t vBufferSize)
try
{
    std::stringstream wktStream;
    (void)wktStream.imbue(std::locale::classic());

    //If we want the vertical system then...
    if (vBuffer && vBufferSize > 0 &&
        strlen(system.geoParameters.vertical_datum) != 0)
    {
        wktStream << R"(VERT_CS[")" << system.geoParameters.vertical_datum <<
            R"(", VERT_DATUM[")" << system.geoParameters.vertical_datum <<
            R"(", 2000]])";

        //Make sure our string is not too large.
        if (wktStream.str().size() > vBufferSize)
            wktStream.str().resize(vBufferSize);

        strcpy(vBuffer, wktStream.str().c_str());
    }

    //If we want the horizontal system then...
    if (hBuffer && hBufferSize > 0)
    {
        switch (system.coordSys)
        {
        case CoordinateType::Geodetic:
        {
            wktStream << datumToWkt(system.geoParameters.datum,
                system.geoParameters.ellipsoid);
        }
        break;

        case CoordinateType::UTM:
        {
            //We need to figure out what hemisphere we are in.
            bool isNorth = false;

            //A false northing of 0.0 means north.
            if (system.geoParameters.false_northing == 0.0)
                isNorth = true;
            //A false northing of 10,000,000 means south.
            else if (system.geoParameters.false_northing == 10'000'000)
                isNorth = false;
            //If we don't have an appropriate false northing, then use the zone.
            else
                isNorth = (system.geoParameters.zone >= 0);

            wktStream << std::fixed << std::setprecision(6)
                << R"(PROJCS["UTM Zone )" << abs(system.geoParameters.zone)
                << ((isNorth) ? R"(, Northern Hemisphere")" : R"(, Southern Hemisphere")")
                << ", " << datumToWkt(system.geoParameters.datum,
                    system.geoParameters.ellipsoid)
                << R"(, PROJECTION["Transverse_Mercator"],)"
                << R"( PARAMETER["latitude_of_origin",)" << 0 << "],"
                << R"( PARAMETER["central_meridian",)" << (abs(system.geoParameters.zone) * 6 - 183) << "],"
                << R"( PARAMETER["scale_factor",)" << 0.9996 << "],"
                << R"( PARAMETER["false_easting",)" << 500'000 << "],"
                << R"( PARAMETER["false_northing",)" << ((isNorth) ? 0 : 10'000'000) << "],"
                << R"( UNIT["metre",1]])";
        }
        break;

        case CoordinateType::Albers_Equal_Area_Conic:
        {
            wktStream << std::fixed << std::setprecision(6)
                << R"(PROJCS["unnamed")"
                << ", " << datumToWkt(system.geoParameters.datum,
                    system.geoParameters.ellipsoid)
                << R"(, PROJECTION["Albers_Conic_Equal_Area"],)"
                << R"( PARAMETER["standard_parallel_1",)" << system.geoParameters.std_parallel_1 << "],"
                << R"( PARAMETER["standard_parallel_2",)" << system.geoParameters.std_parallel_2 << "],"
                << R"( PARAMETER["latitude_of_center",)" << system.geoParameters.latitude_of_centre << "],"
                << R"( PARAMETER["longitude_of_center",)" << system.geoParameters.longitude_of_centre << "],"
                << R"( PARAMETER["false_easting",)" << system.geoParameters.false_easting << "],"
                << R"( PARAMETER["false_northing",)" << system.geoParameters.false_northing << "],"
                << R"( UNIT["metre",1]])";
        }
        break;

        case CoordinateType::Azimuthal_Equidistant:
        {
            wktStream << std::fixed << std::setprecision(6)
                << R"(PROJCS["unnamed")"
                << ", " << datumToWkt(system.geoParameters.datum,
                    system.geoParameters.ellipsoid)
                << R"(, PROJECTION["Azimuthal_Equidistant"],)"
                << R"( PARAMETER["latitude_of_center",)" << system.geoParameters.latitude_of_centre << "],"
                << R"( PARAMETER["longitude_of_center",)" << system.geoParameters.longitude_of_centre << "],"
                << R"( PARAMETER["false_easting",)" << system.geoParameters.false_easting << "],"
                << R"( PARAMETER["false_northing",)" << system.geoParameters.false_northing << "],"
                << R"( UNIT["metre",1]])";
        }
        break;

        case CoordinateType::Bonne:
        {
            wktStream << std::fixed << std::setprecision(6)
                << R"(PROJCS["unnamed")"
                << ", " << datumToWkt(system.geoParameters.datum,
                    system.geoParameters.ellipsoid)
                << R"(, PROJECTION["Bonne"],)"
                << R"( PARAMETER["standard_parallel_1",)" << system.geoParameters.std_parallel_1 << "],"
                << R"( PARAMETER["central_meridian",)" << system.geoParameters.central_meridian << "],"
                << R"( PARAMETER["false_easting",)" << system.geoParameters.false_easting << "],"
                << R"( PARAMETER["false_northing",)" << system.geoParameters.false_northing << "],"
                << R"( UNIT["metre",1]])";
        }
        break;

        case CoordinateType::Cassini:
        {
            wktStream << std::fixed << std::setprecision(6)
                << R"(PROJCS["unnamed")"
                << ", " << datumToWkt(system.geoParameters.datum,
                    system.geoParameters.ellipsoid)
                << R"(, PROJECTION["Cassini_Soldner"],)"
                << R"( PARAMETER["latitude_of_origin",)" << system.geoParameters.origin_latitude << "],"
                << R"( PARAMETER["central_meridian",)" << system.geoParameters.central_meridian << "],"
                << R"( PARAMETER["false_easting",)" << system.geoParameters.false_easting << "],"
                << R"( PARAMETER["false_northing",)" << system.geoParameters.false_northing << "],"
                << R"( UNIT["metre",1]])";
        }
        break;

        case CoordinateType::Cylindrical_Equal_Area:
        {
            wktStream << std::fixed << std::setprecision(6)
                << R"(PROJCS["unnamed")"
                << ", " << datumToWkt(system.geoParameters.datum,
                    system.geoParameters.ellipsoid)
                << R"(, PROJECTION["Cylindrical_Equal_Area"],)"
                << R"( PARAMETER["standard_parallel_1",)" << system.geoParameters.std_parallel_1 << "],"
                << R"( PARAMETER["central_meridian",)" << system.geoParameters.central_meridian << "],"
                << R"( PARAMETER["false_easting",)" << system.geoParameters.false_easting << "],"
                << R"( PARAMETER["false_northing",)" << system.geoParameters.false_northing << "],"
                << R"( UNIT["metre",1]])";
        }
        break;

        case CoordinateType::Eckert4:
        {
            wktStream << std::fixed << std::setprecision(6)
                << R"(PROJCS["unnamed")"
                << ", " << datumToWkt(system.geoParameters.datum,
                    system.geoParameters.ellipsoid)
                << R"(, PROJECTION["Eckert_IV"],)"
                << R"( PARAMETER["central_meridian",)" << system.geoParameters.central_meridian << "],"
                << R"( PARAMETER["false_easting",)" << system.geoParameters.false_easting << "],"
                << R"( PARAMETER["false_northing",)" << system.geoParameters.false_northing << "],"
                << R"( UNIT["metre",1]])";
        }
        break;

        case CoordinateType::Eckert6:
        {
            wktStream << std::fixed << std::setprecision(6)
                << R"(PROJCS["unnamed")"
                << ", " << datumToWkt(system.geoParameters.datum,
                    system.geoParameters.ellipsoid)
                << R"(, PROJECTION["Eckert_VI"],)"
                << R"( PARAMETER["central_meridian",)" << system.geoParameters.central_meridian << "],"
                << R"( PARAMETER["false_easting",)" << system.geoParameters.false_easting << "],"
                << R"( PARAMETER["false_northing",)" << system.geoParameters.false_northing << "],"
                << R"( UNIT["metre",1]])";
        }
        break;

        case CoordinateType::Equidistant_Cylindrical:
        {
            //Plate Caree, 	Equidistant Cylindrical, and Simple Cylindrical are all
            //aliases for Equirectangular.

            wktStream << std::fixed << std::setprecision(6)
                << R"(PROJCS["unnamed")"
                << ", " << datumToWkt(system.geoParameters.datum,
                    system.geoParameters.ellipsoid)
                << R"(, PROJECTION["Equirectangular"],)"
                << R"( PARAMETER["latitude_of_origin",)" << system.geoParameters.origin_latitude << "],"
                << R"( PARAMETER["central_meridian",)" << system.geoParameters.central_meridian << "],"
                << R"( PARAMETER["false_easting",)" << system.geoParameters.false_easting << "],"
                << R"( PARAMETER["false_northing",)" << system.geoParameters.false_northing << "],"
                << R"( UNIT["metre",1]])";
        }
        break;

        case CoordinateType::Gnomonic:
        {
            wktStream << std::fixed << std::setprecision(6)
                << R"(PROJCS["unnamed")"
                << ", " << datumToWkt(system.geoParameters.datum,
                    system.geoParameters.ellipsoid)
                << R"(, PROJECTION["Gnomonic"],)"
                << R"( PARAMETER["latitude_of_origin",)" << system.geoParameters.origin_latitude << "],"
                << R"( PARAMETER["central_meridian",)" << system.geoParameters.central_meridian << "],"
                << R"( PARAMETER["false_easting",)" << system.geoParameters.false_easting << "],"
                << R"( PARAMETER["false_northing",)" << system.geoParameters.false_northing << "],"
                << R"( UNIT["metre",1]])";
        }
        break;

        case CoordinateType::Lambert_Conformal_Conic:
        {
            wktStream << std::fixed << std::setprecision(6)
                << R"(PROJCS["unnamed")"
                << ", " << datumToWkt(system.geoParameters.datum,
                    system.geoParameters.ellipsoid)
                << R"(, PROJECTION["Lambert_Conformal_Conic_2SP"],)"
                << R"( PARAMETER["standard_parallel_1",)" << system.geoParameters.std_parallel_1 << "],"
                << R"( PARAMETER["standard_parallel_2",)" << system.geoParameters.std_parallel_2 << "],"
                << R"( PARAMETER["latitude_of_origin",)" << system.geoParameters.origin_latitude << "],"
                << R"( PARAMETER["central_meridian",)" << system.geoParameters.central_meridian << "],"
                << R"( PARAMETER["false_easting",)" << system.geoParameters.false_easting << "],"
                << R"( PARAMETER["false_northing",)" << system.geoParameters.false_northing << "],"
                << R"( UNIT["metre",1]])";
        }
        break;

        case CoordinateType::Mercator:
        {
            wktStream << std::fixed << std::setprecision(6)
                << R"(PROJCS["unnamed")"
                << ", " << datumToWkt(system.geoParameters.datum,
                    system.geoParameters.ellipsoid)
                << R"(, PROJECTION["Mercator_1SP"],)"
                << R"( PARAMETER["latitude_of_origin",)" << system.geoParameters.origin_latitude << "],"
                << R"( PARAMETER["central_meridian",)" << system.geoParameters.central_meridian << "],"
                << R"( PARAMETER["scale_factor",)" << system.geoParameters.scale_factor << "],"
                << R"( PARAMETER["false_easting",)" << system.geoParameters.false_easting << "],"
                << R"( PARAMETER["false_northing",)" << system.geoParameters.false_northing << "],"
                << R"( UNIT["metre",1]])";
        }
        break;

        case CoordinateType::Miller_Cylindrical:
        {
            wktStream << std::fixed << std::setprecision(6)
                << R"(PROJCS["unnamed")"
                << ", " << datumToWkt(system.geoParameters.datum,
                    system.geoParameters.ellipsoid)
                << R"(, PROJECTION["Miller_Cylindrical"],)"
                << R"( PARAMETER["latitude_of_center",)" << system.geoParameters.latitude_of_centre << "],"
                << R"( PARAMETER["longitude_of_center",)" << system.geoParameters.longitude_of_centre << "],"
                << R"( PARAMETER["false_easting",)" << system.geoParameters.false_easting << "],"
                << R"( PARAMETER["false_northing",)" << system.geoParameters.false_northing << "],"
                << R"( UNIT["metre",1]])";
        }
        break;

        case CoordinateType::Mollweide:
        {
            wktStream << std::fixed << std::setprecision(6)
                << R"(PROJCS["unnamed")"
                << ", " << datumToWkt(system.geoParameters.datum,
                    system.geoParameters.ellipsoid)
                << R"(, PROJECTION["Mollweide"],)"
                << R"( PARAMETER["central_meridian",)" << system.geoParameters.central_meridian << "],"
                << R"( PARAMETER["false_easting",)" << system.geoParameters.false_easting << "],"
                << R"( PARAMETER["false_northing",)" << system.geoParameters.false_northing << "],"
                << R"( UNIT["metre",1]])";
        }
        break;

        case CoordinateType::NZMG:
        {
            wktStream << std::fixed << std::setprecision(6)
                << R"(PROJCS["unnamed")"
                << ", " << datumToWkt(system.geoParameters.datum,
                    system.geoParameters.ellipsoid)
                << R"(, PROJECTION["New_Zealand_Map_Grid"],)"
                << R"( PARAMETER["latitude_of_origin",)" << system.geoParameters.origin_latitude << "],"
                << R"( PARAMETER["central_meridian",)" << system.geoParameters.central_meridian << "],"
                << R"( PARAMETER["false_easting",)" << system.geoParameters.false_easting << "],"
                << R"( PARAMETER["false_northing",)" << system.geoParameters.false_northing << "],"
                << R"( UNIT["metre",1]])";
        }
        break;

        case CoordinateType::Orthographic:
        {
            wktStream << std::fixed << std::setprecision(6)
                << R"(PROJCS["unnamed")"
                << ", " << datumToWkt(system.geoParameters.datum,
                    system.geoParameters.ellipsoid)
                << R"(, PROJECTION["Orthographic"],)"
                << R"( PARAMETER["latitude_of_origin",)" << system.geoParameters.origin_latitude << "],"
                << R"( PARAMETER["central_meridian",)" << system.geoParameters.central_meridian << "],"
                << R"( PARAMETER["false_easting",)" << system.geoParameters.false_easting << "],"
                << R"( PARAMETER["false_northing",)" << system.geoParameters.false_northing << "],"
                << R"( UNIT["metre",1]])";
        }
        break;

        case CoordinateType::Polar_Stereo:
        {
            wktStream << std::fixed << std::setprecision(6)
                << R"(PROJCS["unnamed")"
                << ", " << datumToWkt(system.geoParameters.datum,
                    system.geoParameters.ellipsoid)
                << R"(, PROJECTION["Polar_Stereographic"],)"
                << R"( PARAMETER["latitude_of_origin",)" << system.geoParameters.origin_latitude << "],"
                << R"( PARAMETER["central_meridian",)" << system.geoParameters.central_meridian << "],"
                << R"( PARAMETER["scale_factor",)" << system.geoParameters.scale_factor << "],"
                << R"( PARAMETER["false_easting",)" << system.geoParameters.false_easting << "],"
                << R"( PARAMETER["false_northing",)" << system.geoParameters.false_northing << "],"
                << R"( UNIT["metre",1]])";
        }
        break;

        case CoordinateType::Polyconic:
        {
            wktStream << std::fixed << std::setprecision(6)
                << R"(PROJCS["unnamed")"
                << ", " << datumToWkt(system.geoParameters.datum,
                    system.geoParameters.ellipsoid)
                << R"(, PROJECTION["Polyconic"],)"
                << R"( PARAMETER["latitude_of_origin",)" << system.geoParameters.origin_latitude << "],"
                << R"( PARAMETER["central_meridian",)" << system.geoParameters.central_meridian << "],"
                << R"( PARAMETER["false_easting",)" << system.geoParameters.false_easting << "],"
                << R"( PARAMETER["false_northing",)" << system.geoParameters.false_northing << "],"
                << R"( UNIT["metre",1]])";
        }
        break;

        case CoordinateType::Sinusoidal:
        {
            wktStream << std::fixed << std::setprecision(6)
                << R"(PROJCS["unnamed")"
                << ", " << datumToWkt(system.geoParameters.datum,
                    system.geoParameters.ellipsoid)
                << R"(, PROJECTION["Sinusoidal"],)"
                << R"( PARAMETER["longitude_of_center",)" << system.geoParameters.longitude_of_centre << "],"
                << R"( PARAMETER["false_easting",)" << system.geoParameters.false_easting << "],"
                << R"( PARAMETER["false_northing",)" << system.geoParameters.false_northing << "],"
                << R"( UNIT["metre",1]])";
        }
        break;

        case CoordinateType::Stereographic:
        {
            wktStream << std::fixed << std::setprecision(6)
                << R"(PROJCS["unnamed")"
                << ", " << datumToWkt(system.geoParameters.datum,
                    system.geoParameters.ellipsoid)
                << R"(, PROJECTION["Oblique_Stereographic"],)"
                << R"( PARAMETER["latitude_of_origin",)" << system.geoParameters.origin_latitude << "],"
                << R"( PARAMETER["central_meridian",)" << system.geoParameters.central_meridian << "],"
                << R"( PARAMETER["scale_factor",)" << system.geoParameters.scale_factor << "],"
                << R"( PARAMETER["false_easting",)" << system.geoParameters.false_easting << "],"
                << R"( PARAMETER["false_northing",)" << system.geoParameters.false_northing << "],"
                << R"( UNIT["metre",1]])";
        }
        break;

        case CoordinateType::Transverse_Mercator:
        {
            wktStream << std::fixed << std::setprecision(6)
                << R"(PROJCS["unnamed")"
                << ", " << datumToWkt(system.geoParameters.datum,
                    system.geoParameters.ellipsoid)
                << R"(, PROJECTION["Transverse_Mercator"],)"
                << R"( PARAMETER["latitude_of_origin",)" << system.geoParameters.origin_latitude << "],"
                << R"( PARAMETER["central_meridian",)" << system.geoParameters.central_meridian << "],"
                << R"( PARAMETER["scale_factor",)" << system.geoParameters.scale_factor << "],"
                << R"( PARAMETER["false_easting",)" << system.geoParameters.false_easting << "],"
                << R"( PARAMETER["false_northing",)" << system.geoParameters.false_northing << "],"
                << R"( UNIT["metre",1]])";
        }
        break;

        case CoordinateType::Van_der_Grinten:
        {
            wktStream << std::fixed << std::setprecision(6)
                << R"(PROJCS["unnamed")"
                << ", " << datumToWkt(system.geoParameters.datum,
                    system.geoParameters.ellipsoid)
                << R"(, PROJECTION["VanDerGrinten"],)"
                << R"( PARAMETER["central_meridian",)" << system.geoParameters.central_meridian << "],"
                << R"( PARAMETER["false_easting",)" << system.geoParameters.false_easting << "],"
                << R"( PARAMETER["false_northing",)" << system.geoParameters.false_northing << "],"
                << R"( UNIT["metre",1]])";
        }
        break;

        default:
        {
            //Currently unsupported type.
            return BAG_METADTA_INVALID_PROJECTION;
        }

        }

        if (wktStream.str().size() > hBufferSize)
            wktStream.str().resize(hBufferSize);

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




/*  function:  bagCoordsys
    purpose: taken from geotrans' engine.h from their Coordinate Type
        Enumeration we have a matching string set.  have included all
        coordinate sys types here although we only are supporting
        0, 5, 6, 17, 18, 25, and 30 initially.

    author:  dave fabre, us naval oceanographic office, jul 2005
*/

static const char* COORDINATE_SYS_LIST[]=
{
    "Geodetic",
    "GEOREF",
    "Geocentric",
    "Local_Cartesian",
    "MGRS",
    "UTM",
    "UPS",
    "Albers_Equal_Area_Conic",
    "Azimuthal_Equidistant",
    "BNG",
    "Bonne",
    "Cassini",
    "Cylindrical_Equal_Area",
    "Eckert4",
    "Eckert6",
    "Equidistant_Cylindrical",
    "Gnomonic",
    "Lambert_Conformal_Conic",
    "Mercator",
    "Miller_Cylindrical",
    "Mollweide",
    "Neys",
    "NZMG",
    "Oblique_Mercator",
    "Orthographic",
    "Polar_Stereo",
    "Polyconic",
    "Sinusoidal",
    "Stereographic",
    "Transverse_Cylindrical_Equal_Area",
    "Transverse_Mercator",
    "Van_der_Grinten"
};

/******************************************************************************/
CoordinateType bagCoordsys(const char* str) noexcept
{
    constexpr auto MAX_NCOORD_SYS = 32;
    #define COORD_SYS_NAME(k) COORDINATE_SYS_LIST[k]

    for (size_t i = 0; i < MAX_NCOORD_SYS; i++)
        if (strncmp(str, COORD_SYS_NAME(i), strlen(COORD_SYS_NAME(i))) == 0 )
            return static_cast<CoordinateType>(i);

    return CoordinateType::Unknown;
}




static const char* DATUM_NAME_LIST[] =
{
    "WGS84",
    "WGS72",
    "NAD83"
};

/******************************************************************************/
BagDatum bagDatumID(const char* str) noexcept
{
    constexpr auto MAX_DATUMS = 3;
    #define DATUM_NAME(k) DATUM_NAME_LIST[k]

    for (uint32_t i = 0; i < MAX_DATUMS; i++)
        if (strncmp(str, DATUM_NAME(i), strlen(DATUM_NAME(i))) == 0)
            return static_cast<BagDatum>(i);

    return BagDatum::unknown;
}

}  // namespace BAG

