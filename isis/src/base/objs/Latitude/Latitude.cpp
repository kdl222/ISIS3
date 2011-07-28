/**
 * @file
 *   Unless noted otherwise, the portions of Isis written by the USGS are public
 *   domain. See individual third-party library and package descriptions for
 *   intellectual property information,user agreements, and related information.
 *
 *   Although Isis has been used by the USGS, no warranty, expressed or implied,
 *   is made by the USGS as to the accuracy and functioning of such software
 *   and related material nor shall the fact of distribution constitute any such
 *   warranty, and no responsibility is assumed by the USGS in connection
 *   therewith.
 *
 *   For additional information, launch
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html in a browser or see
 *   the Privacy &amp; Disclaimers page on the Isis website,
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.
 */

#include "Latitude.h"

#include <cmath>

#include "Constants.h"
#include "Distance.h"
#include "iException.h"
#include "iString.h"
#include "PvlGroup.h"
#include "SpecialPixel.h"

namespace Isis {
  /**
   * Create a blank Latitude object without Planetographic support.
   */
  Latitude::Latitude() : Angle() {
    p_equatorialRadius = NULL;
    p_polarRadius = NULL;

    p_errors = ThrowAllErrors;
  }


  /**
   * Create and initialize a Latitude value without planetographic support.
   *
   * @see ErrorChecking
   * @see CoordinateType
   * @param latitude The latitude value this instance will represent,
   *     in the planetocentric coordinate system
   * @param latitudeUnits The angular units of the latitude value (degs, rads)
   * @param errors Error checking conditions
   */
  Latitude::Latitude(double latitude, Angle::Units latitudeUnits,
                     ErrorChecking errors) : Angle() {
    p_equatorialRadius = NULL;
    p_polarRadius = NULL;

    p_errors = errors;

    SetPlanetocentric(latitude, latitudeUnits);
  }


  /**
   * Create and initialize a Latitude value in the planetocentric domain within
   * the given angle.
   *
   * @param latitude The latitude value this instance will represent
   * @param errors Error checking conditions
   */
  Latitude::Latitude(Angle latitude, ErrorChecking errors) : Angle() {
    p_equatorialRadius = NULL;
    p_polarRadius = NULL;

    p_errors = errors;

    SetPlanetocentric(latitude.GetRadians(), Radians);
  }


  /**
   * Create and initialize a Latitude value using the mapping group's latitude
   * units and radii.
   *
   * @see ErrorChecking
   * @see CoordinateType
   * @param latitude The latitude value this instance will represent,
   *     in the mapping group's units
   * @param mapping A mapping group
   * @param latitudeUnits The angular units of the latitude value (degs, rads)
   * @param errors Error checking conditions
   */
  Latitude::Latitude(Angle latitude, PvlGroup mapping,
            ErrorChecking errors) : Angle(latitude) {
    p_equatorialRadius = NULL;
    p_polarRadius = NULL;

    p_equatorialRadius = new Distance(mapping["EquatorialRadius"][0],
        Distance::Meters);
    p_polarRadius = new Distance(mapping["PolarRadius"][0],
        Distance::Meters);

    p_errors = errors;

    if(mapping["LatitudeType"][0] == "Planetographic") {
      SetPlanetographic(latitude.GetRadians(), Radians);
    }
    else if(mapping["LatitudeType"][0] == "Planetocentric") {
      SetPlanetocentric(latitude.GetRadians(), Radians);
    }
    else {
      iString msg = "Latitude type [" + iString(mapping["LatitudeType"][0]) +
        "] is not recognized";
      throw iException::Message(iException::Programmer, msg, _FILEINFO_);
    }
  }


  /**
   * Create and initialize a Latitude value using the mapping group's latitude
   * units and radii.
   *
   * @see ErrorChecking
   * @see CoordinateType
   * @param latitude The latitude value this instance will represent,
   *     in the mapping group's units
   * @param mapping A mapping group
   * @param latitudeUnits The angular units of the latitude value (degs, rads)
   * @param errors Error checking conditions
   */
  Latitude::Latitude(double latitude,
            PvlGroup mapping,
            Angle::Units latitudeUnits,
            ErrorChecking errors) : Angle(latitude, latitudeUnits) {
    p_equatorialRadius = NULL;
    p_polarRadius = NULL;

    p_equatorialRadius = new Distance(mapping["EquatorialRadius"][0],
        Distance::Meters);
    p_polarRadius = new Distance(mapping["PolarRadius"][0],
        Distance::Meters);

    p_errors = errors;

    if(mapping["LatitudeType"][0] == "Planetographic") {
      SetPlanetographic(latitude, latitudeUnits);
    }
    else if(mapping["LatitudeType"][0] == "Planetocentric") {
      SetPlanetocentric(latitude, latitudeUnits);
    }
    else {
      iString msg = "Latitude type [" + iString(mapping["LatitudeType"][0]) +
        "] is not recognized";
      throw iException::Message(iException::Programmer, msg, _FILEINFO_);
    }
  }


  /**
   * Create and initialize a Latitude value with planetographic support.
   *
   * @see ErrorChecking
   * @see CoordinateType
   * @param latitude The latitude value this instance will represent,
   *     in planetocentric
   * @param equatorialRadius Radius of the target (planet) at the equator
   * @param polarRadius Radius of the target (planet) at the poles
   * @param latType The coordinate system of the latitude parameter
   * @param latitudeUnits The angular units of the latitude value (degs, rads)
   * @param errors Error checking conditions
   */
  Latitude::Latitude(double latitude,
                     Distance equatorialRadius, Distance polarRadius,
                     CoordinateType latType,
                     Angle::Units latitudeUnits,
                     ErrorChecking errors) : Angle(latitude, latitudeUnits) {
    p_equatorialRadius = NULL;
    p_polarRadius = NULL;

    p_equatorialRadius = new Distance(equatorialRadius);
    p_polarRadius = new Distance(polarRadius);

    p_errors = errors;

    if (latType == Planetocentric) {
      SetPlanetocentric(latitude, latitudeUnits);
    }
    else if (latType == Planetographic) {
      SetPlanetographic(latitude, latitudeUnits);
    }
    else {
      iString msg = "Enumeration value [" + iString(latType) + "] is not a "
        "valid CoordinateType";
      throw iException::Message(iException::Programmer, msg, _FILEINFO_);
    }
  }


  /**
   * This copies the given latitude exactly.
   *
   * @param latitudeToCopy The latitude we're duplicating
   */
  Latitude::Latitude(const Latitude &latitudeToCopy) : Angle(latitudeToCopy) {
    p_equatorialRadius = NULL;
    p_polarRadius = NULL;

    p_errors = latitudeToCopy.p_errors;

    if(latitudeToCopy.p_equatorialRadius) {
      p_equatorialRadius = new Distance(*latitudeToCopy.p_equatorialRadius);
    }

    if(latitudeToCopy.p_polarRadius) {
      p_polarRadius = new Distance(*latitudeToCopy.p_polarRadius);
    }
  }


  /**
   * This cleans up the Latitude class.
   */
  Latitude::~Latitude() {
    if (p_equatorialRadius) {
      delete p_equatorialRadius;
      p_equatorialRadius = NULL;
    }

    if (p_polarRadius) {
      delete p_polarRadius;
      p_polarRadius = NULL;
    }
  }


  /**
   * Get the latitude in the planetocentric (universal) coordinate system.
   *
   * @see CoordinateType
   * @param units The angular units to get the latitude in
   * @return The Planetocentric latitude value
   */
  double Latitude::GetPlanetocentric(Angle::Units units) const {
    return GetAngle(units);
  }


  /**
   * Set the latitude given a value in the Planetocentric coordinate system
   *
   * @param latitude The planetographic latitude to set ourselves to
   * @param units The angular units latitude is in
   */
  void Latitude::SetPlanetocentric(double latitude, Angle::Units units) {
    SetAngle(latitude, units);
  }


  /**
   * Get the latitude in the planetographic coordinate system. If this instance
   *   was not constructed with the planetary radii, then an exception will be
   *   thrown.
   *
   * @see CoordinateType
   * @param units The angular units to get the latitude in
   * @return The Planetographic latitude value
   */
  double Latitude::GetPlanetographic(Angle::Units units) const {
    if (p_equatorialRadius == NULL || p_polarRadius == NULL) {
      iString msg = "Latitude [" + iString(GetDegrees()) + " degrees] cannot "
          "be converted to Planetographic without the planetary radii, please "
          "use the other Latitude constructor";
      throw iException::Message(iException::Programmer, msg, _FILEINFO_);
    }

    if (*this > Angle(90.0, Angle::Degrees) ||
        *this < Angle(-90.0, Angle::Degrees)) {
      iString msg = "Latitudes outside of the -90/90 range cannot be converted "
          "between Planetographic and Planetocentric";
      throw iException::Message(iException::Programmer, msg, _FILEINFO_);
    }

    if(!Valid()) {
      iString msg = "Invalid planetographic latitudes are not currently "
          "supported";
      throw iException::Message(iException::Programmer, msg, _FILEINFO_);
    }

    double ographicLatitude = atan(tan(GetRadians()) * 
        (*p_equatorialRadius / *p_polarRadius) *
        (*p_equatorialRadius / *p_polarRadius));

    // This theoretically should just be an angle, but make it a Latitude so
    //   we can access GetAngle
    return Latitude(ographicLatitude, Angle::Radians).GetAngle(units);
  }


  /**
   * Set the latitude given a value in the Planetographic coordinate system
   *
   * @param latitude The planetographic latitude to set ourselves to
   * @param units The angular units latitude is in
   */
  void Latitude::SetPlanetographic(double latitude, Angle::Units units) {
    if (p_equatorialRadius == NULL || p_polarRadius == NULL) {
      iString msg = "Latitude [" + iString(latitude) + "] cannot be "
          "converted to Planetocentic without the planetary radii, please use "
          "the other Latitude constructor";
      throw iException::Message(iException::Programmer, msg, _FILEINFO_);
    }

    Angle inputAngle(latitude, units);

    if (inputAngle > Angle(90.0, Angle::Degrees) ||
        inputAngle < Angle(-90.0, Angle::Degrees)) {
      iString msg = "Latitudes outside of the -90/90 range cannot be converted "
          "between Planetographic and Planetocentric";
      throw iException::Message(iException::Programmer, msg, _FILEINFO_);
    }

    if(IsSpecial(latitude)) {
      iString msg = "Invalid planetographic latitudes are not currently "
          "supported";
      throw iException::Message(iException::Programmer, msg, _FILEINFO_);
    }

    double ocentricLatitude = atan(tan(inputAngle.GetRadians()) *
        (*p_polarRadius / *p_equatorialRadius) *
        (*p_polarRadius / *p_equatorialRadius));

    SetAngle(ocentricLatitude, Angle::Radians);
  }


  /**
   * Checks if this latitude value is within the given range.  Defines the
   * range as the change from the minimum latitude to the maximum latitude (an
   * angle), and returns whether the change from the minimum latitude to this
   * latitude is less than or equal to the maximum change allowed (the range).
   *
   * @param min The beginning of the valid latitude range
   * @param max The end of the valid latitude range
   *
   * @return Whether the latitude is in the given range
   */
  bool Latitude::IsInRange(Latitude min, Latitude max) const {
    // Validity check on the range
    if (min > max) {
      iString msg = "Minimum latitude [" + iString(min.GetDegrees()) +
        "] degrees is greater than maximum latitude [" +
        iString(max.GetDegrees()) + "] degrees";
      throw iException::Message(iException::User, msg, _FILEINFO_);
    }

    // Provide a little wriggle room for precision problems
    Angle epsilon(DBL_EPSILON, Angle::Degrees);
    Latitude adjustedMin = min - epsilon;
    Latitude adjustedMax = max + epsilon;

    // Is this latitude between the min and the max
    return *this >= adjustedMin && *this <= adjustedMax;
  }


  /**
    * This assigns another latitude to this one - making this latitude an
    *   exact duplicate of the other.
    *
    * @param latitudeToCopy The latitude we are assigning from
    * @return The result, a reference to this
    */
  Latitude& Latitude::operator=(const Latitude & latitudeToCopy) {
    if(this == &latitudeToCopy) return *this;

    p_equatorialRadius = NULL;
    p_polarRadius = NULL;

    p_errors = latitudeToCopy.p_errors;

    if(latitudeToCopy.p_equatorialRadius) {
      p_equatorialRadius = new Distance(*latitudeToCopy.p_equatorialRadius);
    }

    if(latitudeToCopy.p_polarRadius) {
      p_polarRadius = new Distance(*latitudeToCopy.p_polarRadius);
    }

    SetPlanetocentric(latitudeToCopy.GetPlanetocentric());

    return *this;
  }


  /**
   * We're overriding this method in order to do -90/90 degree checking
   *
   * @param angle The numeric value of the angle
   * @param units The units angle is in (radians or degrees typically)
   */
  void Latitude::SetAngle(double angle, const Angle::Units &units) { 
    // Check for passing 90 degrees if that error checking is on
    if (!IsSpecial(angle) && (p_errors | AllowPastPole) != AllowPastPole) {
      Angle tmpAngle(angle, units);
      if(tmpAngle > Angle(90, Angle::Degrees) ||
         tmpAngle < Angle(-90, Angle::Degrees)) {
        iString msg = "Latitudes past 90 degrees are not valid. The latitude "
            "[" + iString(tmpAngle.GetDegrees()) + " degrees] is not allowed";
        throw iException::Message(iException::Programmer, msg, _FILEINFO_);
      }
    }

    Angle::SetAngle(angle, units);
  }
}
