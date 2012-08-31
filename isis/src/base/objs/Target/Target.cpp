/**
 * @file
 * $Revision: 1.24 $
 * $Date: 2010/04/09 22:31:16 $
 *
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
#include "Target.h"
#include "Distance.h"
#include "EllipsoidShape.h"
#include "IException.h"
#include "NaifStatus.h"
#include "Pvl.h"
#include "ShapeModelFactory.h"
#include "Spice.h"


using namespace std;

namespace Isis {

  /**
   * Constructs a Target object and loads target information.
   *
   * @param lab Label containing Instrument and Kernels groups.
   *
   * @internal
   * @history 2005-11-29 Debbie A. Cook - Original version
   */

  // TODO: what is needed to initialize?
  Target::Target(Pvl &lab) {

    // Initialize members
    m_bodyCode = new SpiceInt;
    m_radii.resize(3);
    // m_radii.resize(3, Distance());
    m_name = new iString;
    m_shape = NULL;
    m_originalShape = NULL;
    m_sky = false;

    PvlGroup &kernels = lab.FindGroup("Kernels", Pvl::Traverse);

    PvlGroup &inst = lab.FindGroup("Instrument", Pvl::Traverse);
    *m_name = inst["TargetName"][0];

    if (name().UpCase() == "SKY") {

      cout << "In SKY code in Target" << endl;

      m_radii[0] = m_radii[1] = m_radii[2] = Distance(1000.0, Distance::Meters);
      m_sky = true;
      string trykey = "NaifIkCode";
      if (kernels.HasKeyword("NaifFrameCode")) trykey = "NaifFrameCode";
      int ikCode = (int) kernels[trykey];
      *m_bodyCode  = ikCode / 1000;
      // Check for override in kernel group
      if (kernels.HasKeyword("NaifSpkCode"))
        *m_bodyCode = (int) kernels["NaifSpkCode"];
    }
    else {

      cout << "Something is wrong with target name...Missed target=Sky" << endl;

      *m_bodyCode = lookupNaifBodyCode();
      m_sky = false;
      // iString radiiKey = "BODY" + iString((BigInt) naifBodyCode()) + "_RADII";
      // m_radii[0] = Distance(getDouble(radiiKey, 0), Distance::Kilometers);
      // m_radii[1] = Distance(getDouble(radiiKey, 1), Distance::Kilometers);
      // m_radii[2] = Distance(getDouble(radiiKey, 2), Distance::Kilometers);
    }
    // Override it if it exists in the labels
    if (kernels.HasKeyword("NaifBodyCode"))
      *m_bodyCode = (int) kernels["NaifBodyCode"];
    m_shape = ShapeModelFactory::Create(this, lab);
  }

  /**
   * Destroys the Target object
   */
  Target::~Target() {
    NaifStatus::CheckErrors();

    if (m_bodyCode != NULL) {
      delete m_bodyCode;
      m_bodyCode = NULL;
    }

    if (m_name != NULL) {
      delete m_name;
      m_name = NULL;
    }

    if (m_radii.size() != 0) {
      m_radii.clear();
    }

    if (m_originalShape != NULL) {
      delete m_originalShape;
      m_originalShape = NULL;
    }

    if (m_shape != NULL) {
      delete m_shape;
      m_shape = NULL;
    }
  }


  //! Return if our target is the sky
  bool Target::isSky() const {
        return m_sky;
  }


  /**
   * This returns the NAIF body code of the target indicated in the labels.
   *
   * @return @b SpiceInt NAIF body code
   *
   */
  SpiceInt Target::lookupNaifBodyCode() const {
    SpiceInt code;
    SpiceBoolean found;
    bodn2c_c(m_name->c_str(), &code, &found);
    if (!found) {
      string msg = "Could not convert Target [" + *m_name +
                   "] to NAIF code";
      throw IException(IException::Io, msg, _FILEINFO_);
    }

    return  code;
  }


  /**
   * This returns the NAIF body code of the target
   *
   * @return @b SpiceInt NAIF body code
   *
   */
  SpiceInt Target::naifBodyCode() const {
    return *m_bodyCode;
  }


  //! Return target name
  iString Target::name() const {
        return *m_name;
      }


  /**
   * Returns the radii of the body in km. The radii are obtained from the
   * appropriate SPICE kernel for the body specified by TargetName in the
   * Instrument group of the labels.
   */
  std::vector<Distance> Target::radii() const {
    return m_radii;
  }


  /**
   * Restores the shape to the original after setShapeEllipsoid has overridden it.
   */
  void Target::restoreShape() {
    if (m_shape->name()  != "Ellipsoid") {
      // Nothing to do
      return;
    }
    // We must currently have an ellipsoidal shape
    else if (m_originalShape == NULL) {
      string msg = "No shape has been saved to be restored";
      throw IException(IException::Programmer, msg, _FILEINFO_);
    }
    m_shape = m_originalShape;
    m_originalShape = NULL;
  }


  /**
   * Set the shape to the ellipsoid and save the original shape.
   */
  void Target::setShapeEllipsoid() {
    // Save the current shape to restore later
    m_originalShape = m_shape;
    m_shape = new EllipsoidShape(this);
  }


  /**
   * This sets the NAIF body code of a "SKY" target to the observing spacecraft.
   *
   */
  void Target::setSky(SpiceInt bodyCode) const {
      *m_bodyCode = bodyCode;  // TODO Is this the best way for now???
  }


  /**
   * Sets the radii of the body.
   *
   * @param r[] Radii of the target in kilometers
   */
  void Target::setRadii(std::vector<Distance> radii) {
    m_radii[0] = radii[0];
    m_radii[1] = radii[1];
    m_radii[2] = radii[2];
  }


  /**
   * Return the shape 
   */
  ShapeModel *Target::shape() const {
    return m_shape;
  }
}
