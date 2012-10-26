/*
 * Sky.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2007 Chris Wallace
 *
 * Fly! Legacy is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 * Fly! Legacy is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 *   along with Fly! Legacy; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*! \file Sky.cpp
 *  \brief Implements CSkyManager and related classes for sky rendering
 */

#include "../Include/Globals.h"
#include "../Include/Sky.h"
#include "../Include/Utility.h"
#include "../Include/Ui.h"
#include "../Include/TimeManager.h"
#include "../Include/Situation.h"
#include "../Include/Ephemeris.h"


//
// Calculate zenith angle and azimuth for a celestial body
//
// Inputs:
//    ra    Right Ascension in radians
//    dec   Declination in radians
//    lat   Latitude, in degrees north/south of Equator
//    lon   Longitude, in degrees east of Prime Meridian
//
// Outputs:
//    za    Zenith angle, in radians
//    azm   Azimuth angle, in radians west of south
//
static void calc_zenith_azimuth_angles (double ra, double dec,
                    double lat, double lon,
                    double *theta, double *phi)
{
  // Get local sidereal time in decimal hours, convert to radians
  double lst = globals->tim->GetLocalSiderealTime (lon * 3600);
  lst = SGD_TWO * SGD_PI * lst / 24.0;

  // Calculate local hour angle in radians
  double lha = lst - ra;

  // Convert latitude to radians
  double latrad = DegToRad (lat);

  // Calculate zenith angle theta
  double thetaFactor1 = sin(latrad)*sin(dec);
  double thetaFactor2 = cos(latrad)*cos(dec)*cos(lha);
  double thetaArg = thetaFactor1 + thetaFactor2;
  *theta = WrapTwoPi ((SGD_PI / 2.0) - asin (thetaArg));

  double phiArgY = -cos(dec)*sin(lha);
  double phiArgX = cos(latrad)*sin(dec) - sin(latrad)*cos(dec)*cos(lha);
  *phi = WrapTwoPi (atan2 (phiArgY, phiArgX));
}

CSkyManager CSkyManager::instance;

CSkyManager::CSkyManager (void)
{
  // Initialize darkness phase
  old_phase = PHASE_INVALID;

  // Initialize effective visibility to 60 miles
  effective_visibility = float(FN_FEET_FROM_MILE(60));              //60.0 * 5280.0;
  skyDistance = effective_visibility;

  // Initialize sub-object pointers
  skylight = NULL;
  moon = NULL;
  sol = NULL;
  mercury = NULL;
  venus = NULL;
  mars = NULL;
  jupiter = NULL;
  saturn = NULL;
  domeimage = NULL;
  starimage = NULL;
  solimage = NULL;
  moonimage = NULL;
  mercuryimage = NULL;
  venusimage = NULL;
  marsimage = NULL;
  jupiterimage = NULL;
  saturnimage = NULL;
}

void CSkyManager::Init (void)
{ globals->skm = this;
  // Initialize darkness phase
  old_phase = PHASE_INVALID;

  // Instantiate and initialize sky lighting model
  skylight = new CSkyLight ();
  skylight->Init ();

  // Initialize effective visibility to 60 miles
  effective_visibility = 60.0 * 5280.0;

  // Instantiate ephemeris classes for sun, moon and planets
  moon  = new CMoon ();
  sol   = new CSol ();
  mercury = new CMercury ();
  venus = new CVenus ();
  mars = new CMars ();
  jupiter = new CJupiter ();
  saturn = new CSaturn ();

//  skyDistance = 1.0E+6;
  skyDistance = effective_visibility;

  // Instantiate sky dome
  domeimage = new CSkyDomeImage (skyDistance);
  domeimage->Repaint (0.0, 0.0);

  // Instantiate star images
  starimage = new CStarImages (skyDistance);

  // Instantiate Mercury image
  // Mercury RGB colour 255,183,111 from JRASC Vol.99 No.3 [712] June 2005, p.103
  float r, g, b;
  r = 1.0f;
  g = (183.0f/255.0f);
  b = (111.0f/255.0f);
  mercuryimage = new CPlanetImage (r, g, b, "Mercury");

  // Instantiate Venus image
  // Venus RGB colour 255,241,162 from JRASC Vol.99 No.3 [712] June 2005, p.103
  r = 1.0f;
  g = (241.0f/255.0f);
  b = (162.0f/255.0f);
  venusimage = new CPlanetImage (r, g, b, "Venus");

  // Instantiate Mars image
  // Mars RGB colour 255,142,58 from JRASC Vol.99 No.3 [712] June 2005, p.103
  r = 1.0f;
  g = (142.0f/255.0f);
  b = (58.0f/255.0f);
  marsimage = new CPlanetImage (r, g, b, "Mars");

  // Instantiate Jupiter image
  // Jupiter RGB colour 255,253,168 from JRASC Vol.99 No.3 [712] June 2005, p.103
  r = 1.0f;
  g = (253.0f/255.0f);
  b = (168.0f/255.0f);
  jupiterimage = new CPlanetImage (r, g, b, "Jupiter");

  // Instantiate Saturn image
  // Saturn RGB colour 255,222,122 from JRASC Vol.99 No.3 [712] June 2005, p.103
  r = 1.0f;
  g = (222.0f/255.0f);
  b = (122.0f/255.0f);
  saturnimage = new CPlanetImage (r, g, b, "Saturn");

/*!
 * \todo Investigate Fly! II sun rendering in more detail, and/or research
 *         independent method for sun visualization
 */

  // Instantiate moon image
  moonimage = new CMoonImage (skyDistance);
  moonimage->Repaint (PI/2, 0.0);
}

void CSkyManager::Cleanup (void)
{
  delete skylight;
  delete domeimage;
  delete starimage;
  delete mercuryimage;
  delete venusimage;
  delete marsimage;
  delete jupiterimage;
  delete saturnimage;
  delete moonimage;

  delete moon;
  delete sol;
  delete mercury;
  delete venus;
  delete mars;
  delete jupiter;
  delete saturn;
}


//
// This function returns the limiting visual magnitude based on the sun's elevation.
//
static EDarkPhase limitingMagnitude (double solTheta, float *limit, float *factor)
{
  /// \todo Tweak for seamless fade-in fade-out at all limiting magnitudes
  /// \todo Incorporate moon elevation and phase into limiting magnitude
  
  EDarkPhase phase = PHASE_DEEP_NIGHT;

  // Calculate phase of darkness based on sun elevation
  float solElevation = 90.0f - RadToDeg ((float)solTheta);
/*
  if (solElevation < -18.0) {
    // Deep night
    phase = PHASE_DEEP_NIGHT;
        *factor = 1.0;
        *limit = 6.5;
  } else if (solElevation < -12.0) {
    // Night
    phase = PHASE_NIGHT;
        *factor = 1.0;
        *limit = 5.5;
  } else if (solElevation <  -6.0) {
    // Late Dusk
    phase = PHASE_LATE_DUSK;
        *factor = 0.95;
        *limit = 3.5;
  } else if (solElevation < -3.0) {
    // Dusk
    phase = PHASE_DUSK;
        *factor = 0.9;
        *limit = 2.0;
  } else if (solElevation < -1.5) {
    // Early Dusk
    phase = PHASE_EARLY_DUSK;
        *factor = 0.85;
        *limit = 1.0;
  } else if (solElevation < 0.0) {
    // Late Twilight
    phase = PHASE_LATE_TWILIGHT;
        *factor = 0.8;
        *limit = 0.5;
  } else if (solElevation < 0.5) {
    // Early Twilight
    phase = PHASE_EARLY_TWILIGHT;
        *factor = 0.75;
    *limit = 0.2;
  } else {
    // Daylight
    phase = PHASE_DAYLIGHT;
        *factor = 0.7;
    *limit = 0.0;
  }
*/

  // Experimental star limit/factor
  if (solElevation > 0) {
    // Sun is above horizon
    float f = (90.0f - solElevation) / 90.0f;
    *limit = -10.0f + (10.5f * f);
    *factor = 0.7f * f;
  } else if (solElevation > -18.0f) {
    // Twilight
    *limit = 0.5f + (-solElevation / 3.0f);
    *factor = 0.7f + (0.3f * (-solElevation / 18.0f));
  } else {
    // Astronomical night
    *limit = 6.5f;
    *factor = 1.0f;
  }

  // DEBUG
//  char debug[80];
//  sprintf (debug, "elevation=%7.3f  factor=%5.3f  limit=%5.3f", 
//    solElevation, *factor, *limit);
//  DrawNoticeToUser (debug, 1);
  
  return phase;
}

/**
 * Determine colour of the sky dome at a given position
 *
 * @param theta Zenith angle in radians
 * @param phi   Azimuth angle in radians (north-based)
 * @param r     Red colour value
 * @param g     Green colour value
 * @param b     Blue colour value
 *
 */
/*void CSkyManager::GetSkyDomeColour (float theta, float phi, float &r, float &g, float &b)
{
  // Set default fog colour
  // r = g = b = 0.0f;

  if (domeimage != NULL) domeimage->GetDomeColour (theta, phi, r, g, b);
}
*/
void CSkyManager::PreDraw (void)
{
  SPosition eyePos = globals->geop;
  double mjd = globals->tim->GetModifiedJulianDate ();
  double lst = globals->tim->GetLocalSiderealTime(eyePos.lon);

  // Convert camera eye position to cartesian coordinates
  SVector v;
  v = GeodToCartesian (eyePos);
  sgVec3 p;
  sgSetVec3 (p, (float)v.x, (float)v.y, (float)v.z);

  // Convert latitude and longitude to degrees for sky object repositioning
  double lat = eyePos.lat / 3600;
  double lon = eyePos.lon / 3600;

  /// \todo  Sun/moon positions only need to be recalculated infrequently, not
  ///   on every update cycle

  // Update ephemeris calculations for sun.  This needs to be done first since
  //   the sun elevation is needed to update the other celestial objects
  sol->UpdatePosition (mjd);

  // Update ephemeris calculations for other celestial objects
  moon->UpdatePosition (mjd, lst, lat, sol);

  /// \todo  Planet positions only need to be calculated once at situation start
  mercury->UpdatePosition (mjd, sol);
  venus->UpdatePosition (mjd, sol);
  mars->UpdatePosition (mjd, sol);
  jupiter->UpdatePosition (mjd, sol);
  saturn->UpdatePosition (mjd, sol);

  // Get solar zenith and azimuth angles
  
  calc_zenith_azimuth_angles (sol->GetRightAscension(), sol->GetDeclination(),
                              lat, lon,
                              &solTheta, &solPhi);

  double moonTheta, moonPhi;
  calc_zenith_azimuth_angles (moon->GetRightAscension(), moon->GetDeclination(),
                              lat, lon,
                              &moonTheta, &moonPhi);

  // Update sky lighting model
  skylight->Update (solTheta, solPhi, moonTheta, moonPhi);

  // DEBUG
//  char debug[80];
//  sprintf (debug, "solTheta=%7.3f deg, solPhi=%7.3f deg",
//    RadToDeg (solTheta), RadToDeg (solPhi));
//  DrawNoticeToUser (debug, 1);

  // Update appearance and position of sky dome
  domeimage->Repaint (solTheta, solPhi);

  // Update appearance and position of Sol
//  solimage->Repaint (solTheta);
//  solimage->Reposition (p, lst, lat,
//                      sol->GetRightAscension(), sol->GetDeclination(),
//                      skyDistance);

  // Update appearance and position of the Moon
  /// \todo  Orient moon towards sun using the 'spin' parameter
  moonimage->Repaint (moonTheta, moon->GetAge ());
  moonimage->Reposition (p, moonTheta, lst, lat,
                       moon->GetRightAscension(), moon->GetDeclination(), 0);

  // Determine limiting magnitude and ambient light factor for night sky objects
  float limit, factor;
  float limitLastUpdate = 10.0;
//  EDarkPhase phase = limitingMagnitude (solTheta, &limit, &factor);
  limitingMagnitude (solTheta, &limit, &factor);

  // If the limiting magnitude has changed by more than the preset value, repaint
  //   the stars and planets
  //  if (phase != old_phase) {
  if (fabs (limitLastUpdate - limit) > 0.1) {
    limitLastUpdate = limit;
    // Update appearance for all planets and stars
    mercuryimage->Repaint ((float)mercury->GetMagnitude (), limit, factor);
    venusimage->Repaint ((float)venus->GetMagnitude(), limit, factor);
    marsimage->Repaint ((float)mars->GetMagnitude(), limit, factor);
    jupiterimage->Repaint ((float)jupiter->GetMagnitude(), limit, factor);
    saturnimage->Repaint ((float)saturn->GetMagnitude(), limit, factor);
    starimage->Repaint (limit, factor);
  }

  // Update positions of all planets
  mercuryimage->Reposition (p, lst, lat,
                            mercury->GetRightAscension(), mercury->GetDeclination(),
                            skyDistance);
  venusimage->Reposition (p, lst, lat,
                          venus->GetRightAscension(), venus->GetDeclination(),
                          skyDistance);
  marsimage->Reposition (p, lst, lat,
                         mars->GetRightAscension(), mars->GetDeclination(),
                         skyDistance);
  jupiterimage->Reposition (p, lst, lat,
                            jupiter->GetRightAscension(), jupiter->GetDeclination(),
                            skyDistance);
  saturnimage->Reposition (p, lst, lat,
                           saturn->GetRightAscension(), saturn->GetDeclination(),
                           skyDistance);

  // Update positions of stars
  starimage->Reposition (p, lon, lat, lst);

  // Depth testing is disabled for all sky elements, therefore they must be drawn
  //   from "farthest" to "nearest"
  glPushAttrib (GL_DEPTH_BUFFER_BIT);
  glDisable (GL_DEPTH_TEST);

  // Draw sky sub-components
  domeimage->Draw ();
  starimage->Draw ();
  saturnimage->Draw ();
  jupiterimage->Draw ();
  marsimage->Draw ();
  venusimage->Draw ();
  mercuryimage->Draw ();

  // Sun and moon images temporarily left undrawn
  //solimage->Draw ();
  moonimage->Draw ();
  glPopAttrib ();
}

//
// Dump the contents of the CSkyManager class to a file for debugging
//
void CSkyManager::Print (FILE *f)
{ 
}
//==================== END OF FILE ======================================================================