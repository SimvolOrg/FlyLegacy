/*
 * SolImage.cpp
 *
 * Part of Fly! Legacy project
 *
 * The design and implementation of these classes is based heavily on the
 *   sky model of SimGear 0.3.3, by Curtis Olson.  Integration into the
 *   Fly! Legacy project necessitated a rewrite of the actual classes,
 *   but I am indebteded to Curt and other contributors to the SimGear and
 *   FlightGear teams for their prior work in this area.
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

/*! \file SolImage.cpp
 *  \brief Implements CSolImage class to render Earth's sun (Sol) in the sky
 *
 *  The CSolImage class renders Earth's sun (Sol) in the sky.
 */

#include "../Include/Sky.h"
#include "../Include/Utility.h"

//
// Sun layering parameters
//
// In Fly! II, this data was represented by the file Data\sun.txt
//   Since there was never any customization done as far as I know,
//   there's no real need to support this file, so to save time the
//   implementation is just hard-coded to match the contents of the
//   default sun.txt shipped with Fly! II
//

typedef struct {
  char  filename[64];   // Texture filename
  int   w;          // Dimension (width)
  float offset;       // Layer y-offset
  float radius;       // Radius in pseudo-feet
  float u1, v1, u2, v2;   // Texture coordinates
} SSunLayer;


const int nSunLayers = 10;
SSunLayer sunLayers[] =
{
  {"sun06",   64,   0.500000f,   43.000000f,  8.0f,   8.0f,   248.0f, 248.0f},
  {"suncram", 256,  0.30000f,    143.000000f, 130.0f, 130.0f, 254.0f, 254.0f},
  {"sun07",   64,   0.100000f,   11.000000f,  8.0f,   8.0f,   248.0f, 248.0f},
  {"sun07",   64,   -0.271000f,  27.000000f,  8.0f,   8.0f,   248.0f, 248.0f},
  {"sun09",   64,   -0.416000f,  80.000000f,  8.0f,   8.0f,   248.0f, 248.0f},
  {"sun11",   64,   -0.475000f,  41.000000f,  8.0f,   8.0f,   248.0f, 248.0f},
  {"sun12",   64,   -0.643000f,  61.000000f,  8.0f,   8.0f,   248.0f, 248.0f},
  {"sun13",   64,   -0.674000f,  60.000000f,  8.0f,   8.0f,   248.0f, 248.0f},
  {"suncram", 256,  -1.000000f,  213.000000f, 130.0f, 2.0f,   254.0f, 126.0f},
  {"suncram", 256,  -1.333000f,  410.000000f, 2.0f,   130.0f, 126.0f, 254.0f}
};

//=========================================================================================
class CSunTextureCache {
public:
  CSunTextureCache (void);
  ~CSunTextureCache (void);

public:
  char          filename[64];
  CSunRawImage  *raw;
  GLuint        tex;
};

CSunTextureCache::CSunTextureCache (void)
{*filename = 0;
  raw = NULL;
  tex = 0;
}

CSunTextureCache::~CSunTextureCache(void)
{
  if (tex != 0) glDeleteTextures (1, &tex);
  if (raw != NULL) delete raw;
}


GLuint CSolImage::GetSunTexture (const char* filename, int w)
{
  GLuint rc = 0;
  return rc;
}

//
// Instantiate the sun image renderer
//
CSolImage::CSolImage (double size)
{
}

CSolImage::~CSolImage (void)
{
}

void CSolImage::Repaint (double angle)
{
  static double prev_angle = 9999.0;

  if (angle != prev_angle) {
    prev_angle = angle;

    // Compute power factor, clamped to [-1, +1]
    float factor = 4.0f * cos((float)angle);
    if (factor > 1.0f) factor = 1.0f;
    if (factor < -1.0f) factor = -1.0f;

    // Compute RGB colours for sun orb
    sgVec4 colour;
    colour[0] = pow (factor, 0.25f);     // Red
    colour[1] = pow (factor, 0.50f);     // Green
    colour[2] = pow (factor, 4.0f);      // Blue
    colour[3] = 1.0f;                    // Alpha (opaque)
  }
}

//
// Reposition the image of the sun in the sky
//
// Arguments:
//  pos     3D position of the eye (cartesian coordinates??)
//  lst     Local Sidereal Time, in degrees
//  ra      Right Ascension, in radians
//  dec     Declination, in radians
//  distance  Distance from the eye in world units
//
void CSolImage::Reposition (sgVec3 pos, double lst, double lat, double ra, double dec, double distance)
{
  sgMat4 LST, LAT, RA, DEC, D;
  sgVec3 axis;
  sgVec3 v;

  // Rotation matrix for latitude
  sgSetVec3 (axis, -1, 0, 0);
  sgMakeRotMat4 (LAT, 90.0f-(float)lat, axis);

  // Rotation matrix for local sidereal time, converted from h to deg
  sgSetVec3 (axis, 0, 0, -1);
  sgMakeRotMat4 (LST, ((float)lst * 15.0f), axis);

  // Rotation matrix for right ascension
  sgSetVec3 (axis, 0, 0, 1);
  sgMakeRotMat4 (RA, RadToDeg ((float)ra), axis);

  // Rotation matrix for declination
  sgSetVec3 (axis, 1, 0, 0);
  sgMakeRotMat4 (DEC, 90.0f - RadToDeg ((float)dec), axis);

  // Translate sun distance
  sgSetVec3 (v, 0, 0, (float)distance);
  sgMakeTransMat4 (D, v);

  // Combine all transforms
  sgMat4 T;
  sgMakeIdentMat4 (T);
  sgPreMultMat4 (T, LAT);
  sgPreMultMat4 (T, LST);
  sgPreMultMat4 (T, RA);
  sgPreMultMat4 (T, DEC);
  sgPreMultMat4 (T, D);

  sgCoord skypos;
  sgSetCoord (&skypos, T);
}

void CSolImage::Draw (void)
{
  glPushAttrib (GL_DEPTH_BUFFER_BIT | GL_FOG_BIT);
  glDisable (GL_DEPTH_TEST);
//  glDisable (GL_FOG);

  glPopAttrib ();
}

