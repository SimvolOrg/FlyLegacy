/*
 * MoonImage.cpp
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

/*! \file MoonImage.cpp
 *  \brief Implements CMoonImage class to render Earth's moon in the sky
 *
 *  Implementation of CMoonImage class to render Earth's moon.
 *
 * The design and implementation of these classes is based heavily on the
 *   sky model of SimGear 0.3.3, by Curtis Olson.  Integration into the
 *   Fly! Legacy project necessitated a rewrite of the actual classes,
 *   but I am indebteded to Curt and other contributors to the SimGear and
 *   FlightGear teams for their prior work in this area.
 *
 */

#include "../Include/Sky.h"
#include "../Include/Utility.h"
#include "../Include/Fui.h"

CMoonImage::CMoonImage (float distance)
{
  // Load DXT texture
  strncpy (ddsFilename, "ART/MOONMAP_256.dds",FNAM_MAX);
  CImageDXT *dxt = new CImageDXT (ddsFilename);
  texid = CTextureManager::Instance().BindTexture (ddsFilename, dxt);
  SAFE_DELETE (dxt);

  // Rendering size and distance are arbitrarily chosen so that moon is has the
  //   correct apparent size and appears far away from the observer
  this->distance = distance;
  size = 8000.0f;

  // Initialize GLU quadric for sphere rendering
  q = gluNewQuadric ();
  gluQuadricDrawStyle (q, GL_FILL);
  gluQuadricOrientation (q, GLU_OUTSIDE);
  gluQuadricTexture (q, GL_TRUE);
  gluQuadricNormals(q, GLU_SMOOTH);

  // Create textured sphere in display list
  dlOrb = glGenLists (1);
  glNewList (dlOrb, GL_COMPILE);
  gluSphere (q, size, 16, 16);
  glEndList ();
}

CMoonImage::~CMoonImage (void)
{
  CTextureManager::Instance().DiscardTexture (ddsFilename);
}

// Repaint the moon image based on its relative angle in radians:
//    0  =   Transit (highest point in the sky)
//   PI/2 =  Rise/set
//    PI  =  Nadir
//
// Age is number of days into current lunation (0.0 = new moon, ~14.5 = full moon)
//
// Moon lighting algorithm adapted from SimGear 0.3.3

void CMoonImage::Repaint (double angle, float age)
{
  // x_10 = moon_angle^10
  double x_2 = angle * angle;
  double x_10 = x_2 * x_2 * x_2 * x_2 * x_2;

  // Calculate ambient light caused by moon; clamped between 0.3 and 1.0
  /// \todo This does not account for lunar phase?
  float ambient = (float)(0.4 * pow (1.1, -x_10 / 30.0));
  if (ambient < 0.3f) ambient = 0.3f;
  if (ambient > 1.0f) ambient = 1.0f;

  // Compute moon colouring
  sgSetVec4 (colour,
    (ambient * 6.0f) - 1.0f,
    (ambient * 11.0f) - 3.0f,
    (ambient * 12.0f) - 3.6f,
    0.5f);

  // For testing, force colour to pure white
  sgSetVec4 (colour, 1.0f, 1.0f, 1.0f, 1.0f);

  // Clamp colour components to maximum 1.0
  if (colour[0] > 1.0f) colour[0] = 1.0f;
  if (colour[1] > 1.0f) colour[1] = 1.0f;
  if (colour[2] > 1.0f) colour[2] = 1.0f;
}

//
// Update the position of the moon image in the sky
//
void CMoonImage::Reposition (sgVec3 p, double theta, double lst, double lat,
               double ra, double dec, double spin)
{
  sgMat4 LST, LAT, RA, DEC, D, SCALE, ECLIPTIC, SPIN;
  sgVec3 axis;
  sgVec3 v;

  // Create scaling matrix for moon illusion (appears larger near horizon)
  float scale = 1.0f;
  sgMakeIdentMat4 (SCALE);
  float maxMagnification = 0.5f;
  float minThreshold = DegToRad (80.0f);
  float maxThreshold = DegToRad (95.0f);
  float span = maxThreshold - minThreshold;
  if ((theta >= minThreshold) && (theta <= maxThreshold)) {
    sgMat4 I;
    sgMakeIdentMat4 (I);
    scale = 1.0f + (maxMagnification * (theta - minThreshold) / span);
    sgScaleMat4 (SCALE, I, scale);
  }

  // Rotation matrix for latitude
  sgSetVec3 (axis, -1.0f, 0, 0);
  sgMakeRotMat4 (LAT, 90.0f-(float)lat, axis);

  // Rotation matrix for local sidereal time, converted from h to deg
  sgSetVec3 (axis, 0, 0, -1.0f);
  sgMakeRotMat4 (LST, ((float)lst * 15), axis);

  // Rotation matrix for right ascension
  sgSetVec3 (axis, 0, 0, 1);
  sgMakeRotMat4 (RA, RadToDeg ((float)ra), axis);

  // Rotation matrix for declination
  sgSetVec3 (axis, 1, 0, 0);
  sgMakeRotMat4 (DEC, 90.0f - RadToDeg ((float)dec), axis);

  // Translate moon distance
  sgSetVec3 (v, 0, 0, distance);
  sgMakeTransMat4 (D, v);

  // Rotate to align moon equator with ecliptic
  sgSetVec3 (axis, 1.0f, 0, 0);
  sgMakeRotMat4 (ECLIPTIC, 90.0f, axis);

  /// Rotate the moon image accurately towards the sun position
  sgSetVec3 (axis, 0, 0, 1);
  sgMakeRotMat4 (SPIN, spin, axis);

  // Combine all transforms
  sgMakeIdentMat4 (T);
  sgPreMultMat4 (T, LAT);
  sgPreMultMat4 (T, LST);
  sgPreMultMat4 (T, RA);
  sgPreMultMat4 (T, DEC);
  sgPreMultMat4 (T, D);
  sgPreMultMat4 (T, ECLIPTIC);
  sgPreMultMat4 (T, SPIN);

/*
  char debug[256];
  double jd = CTimeManager::Instance().GetJulianDate();
  SDateTime dt = CTimeManager::Instance().GetLocalDateTime ();
  sprintf (debug, "JD=%f D=%d/%d/%d T=%d:%d RA=%f Dec=%f", jd,
    dt.date.year, dt.date.month, dt.date.day, dt.time.hour, dt.time.minute,
    RadToDeg(ra), RadToDeg(dec));
  DrawNoticeToUser (debug, 1);
*/
}

void CMoonImage::Draw (void)
{
  // Predraw
  glPushAttrib (GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT);
  glDisable (GL_DEPTH_TEST);
//  glDisable (GL_FOG);
  glDisable (GL_LIGHTING);
  glEnable (GL_CULL_FACE);
  glCullFace (GL_BACK);
  glEnable (GL_TEXTURE_2D);
  glBindTexture (GL_TEXTURE_2D, texid);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  // Transform
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  glMultMatrixf ((GLfloat*)T);

  // Draw
  glColor4fv ((GLfloat*)colour);
  glCallList (dlOrb);

  // Restore GL state
  glMatrixMode (GL_MODELVIEW);
  glPopMatrix ();
  glPopAttrib ();
}
