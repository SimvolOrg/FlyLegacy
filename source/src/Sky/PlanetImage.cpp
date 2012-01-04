/*
 * PlanetImage.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2006 Chris Wallace
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

/*! \file PlanetImage.cpp
 *  \brief Implements CPlanetImage, representing a rendered planet in the sky
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

CPlanetImage::CPlanetImage (float r, float g, float b, const char* name)
{
  // Assign planet colour with default alpha
  strncpy (this->name, name,63);
  this->r = r;
  this->g = g;
  this->b = b;
  this->a = 1.0f;
  this->pointSize = 1.0f;
}

//
// Repaint the appearance of the image
//
void CPlanetImage::Repaint (float mv, float limit, float factor)
{
  float nmag, alpha;
  if (mv < limit) {
    // This star is brighter than the limiting magnitude
    nmag = (limit - mv) / limit;    // Scale brightness to 0.0..1.0
    alpha = (nmag * 0.85f);         // Scale alpha to 0.0..0.85
    alpha += 0.25f;                 // Bias +25%
    alpha *= factor;                // Adjust alpha for ambient light

    // Calculate point size
    pointSize = (limit - mv) * 0.5f;

    // Clamp alpha to 0.0..1.0
    if (alpha > 1.0f) { alpha = 1.0f; }
    if (alpha < 0.0f) { alpha = 0.0f; }
  } else {
    // Star is dimmer than the limiting magnitude
    alpha = 0;
    pointSize = 0;
  }
  this->a = alpha;
}

//
// Reposition the planet image in the sky
//
// Arguments:
//  pos     3D position of the eye (cartesian coordinates??)
//  lst     Local Sidereal Time, in degrees
//  ra      Right Ascension, in radians
//  dec     Declination, in radians
//  distance  Distance from the eye in world units
//
void CPlanetImage::Reposition (sgVec3 pos, double lst, double lat,
                               double ra, double dec, double distance)
{
  // Set vertex coordinates based on RA, Dec and apparent distance
  // This can be recalculated since RA and Dec can change appreciably over
  //   even short periods of time (hours) for the inner planets, although
  //   an optimization would be to calculate planet positions just once at
  //   the beginning of the simulation.
  this->x = (float)(distance * cos (ra) * cos (dec));
  this->y = (float)(distance * sin (ra) * cos (dec));
  this->z = (float)(distance * sin (dec));

  // Rotation around x-axis is latitude
  rotX = -(90.0f - (float)lat);

  // Rotation around z-axis is local sidereal time
  rotZ = -((float)lst * 15.0f);
}

void CPlanetImage::DrawPlanet (void)
{
  // Setup GL state for rendering planet
  glPushAttrib (GL_POINT_BIT);
  glEnable (GL_POINT_SMOOTH);
  glPointSize (pointSize);

  // Draw
  glBegin (GL_POINTS);
    glColor4f (r, g, b, a);
    glVertex3f (x, y, z);
  glEnd ();

  // Restore GL state
  glPopAttrib ();
}

void CPlanetImage::Draw (void)
{
  // Only draw if calculated image size is greater than zero pixels (i.e. planet is visible)
  if (pointSize > 0) {
    // Transform
    glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    glRotatef (rotX, 1, 0, 0);
    glRotatef (rotZ, 0, 0, 1);

    // Draw planet
    DrawPlanet ();

    // Restore GL context
    glMatrixMode (GL_MODELVIEW);
    glPopMatrix ();
  }
}
