/*
 * Scenery.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
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
 */

/**
 * @file Scenery.cpp
 * @brief Implements scenery and terrain related utility functions
 *
 * Thanks to Michael Jastrzebski, Ken Salter, Frank Racis for their previous
 *   work with with latitude/longitude to globe tile conversion!
 */

#include <math.h>
#include "../Include/Utility.h"
#include "../Include/Globals.h"
#include "../Include/TerrainCache.h"

//
// If this macro is defined, then a table of globe tile latitude boundaries is
//   dumped to a text file for debugging immediately after initialization
//
// #define DUMP_GLOBE_TILE_BOUNDARIES


/**
 * Calculate the western longitude boundary for a given GT x-index
 *
 * @param x   GT x-index in range [0,255]
 * @return    Longitude of eastern boundary in degrees E of prime meridian
 */
static double GlobeTileWesternLon (int x)
{
  return (double)x * 360.0 / 256.0;
}
    
/**
 * @brief Find longitude bounds for a given GT x-index
 *
 * @param x   GT x-index in range [0,255]
 * @param &w  Updated with western longitude boundary in degrees E of prime meridian
 * @param &e  Updated with eastern longitude boundary in degrees E of prime meridian
 */
static void GlobeTileLonBounds (int x, double &w, double &e)
{
  w = e = 0;

  if ((x < 0) || (x > 255)) {
    // Generate warning
    WARNINGLOG ("Illegal GT index x=%d", x);
  } else {
    w = GlobeTileWesternLon (x) * 3600.0;
    e = GlobeTileWesternLon (x+1) * 3600.0;
  }
}



#define ARCSEC2RAD    ((2.0 * PI) / 3600.0)

//
// Format a position struct into human-readable text
//
void FormatPosition (SPosition pos, char* s)
{
  double latSec = pos.lat;
  char latHemi;
  if (latSec >= 0) {
    latHemi = 'N';
  } else {
    latHemi = 'S';
    latSec = fabs(latSec);
  }

  double lonSec = pos.lon;
  char lonHemi;
  if (lonSec >= (180.0 * 3600)) {
    lonHemi = 'W';
    lonSec = (360 * 3600) - lonSec;
  } else {
    lonHemi = 'E';
  }

  double latDeg = floor (latSec / 3600);
  latSec -= (latDeg * 3600);
  double latMin = floor (latSec / 60);
  latSec -= (latMin * 60);

  double lonDeg = floor (lonSec / 3600);
  lonSec -= (lonDeg * 3600);
  double lonMin = floor (lonSec / 60);
  lonSec -= (lonMin * 60);

  sprintf (s, "%c %2.0f %2.0f'%7.4f\"  %c %2.0f %2.0f'%7.4f\"",
    latHemi, latDeg, latMin, latSec, lonHemi, lonDeg, lonMin, lonSec);
}


//
// Format Right Ascension/Declination (both in radians) to a human-readable string
//
void FormatRADec (double ra, double dec, char* s)
{
  ra = WrapTwoPi (ra) * 24.0 / (2 * SGD_PI);
  dec = RadToDeg (dec);

  // Convert RA to hrs/min/sec
  double ra_hr, ra_min, ra_sec;
  ra_hr = floor (ra);
  ra -= ra_hr;
  ra_min = floor (ra * 60);
  ra -= ra_min / 60;
  ra_sec = ra * 3600;

  // Convert Dec to deg/arcmin/arcsec
  double dec_deg, dec_min, dec_sec;
  dec_deg = floor (dec);
  dec -= dec_deg;
  dec_min = floor (dec * 60);
  dec -= dec_min / 60;
  dec_sec = dec * 3600;

  sprintf (s, "%2.0fh %2.0fm %7.3fs  %3.0fd %2.0fm %7.3fs",
    ra_hr, ra_min, ra_sec, dec_deg, dec_min, dec_sec);
}


//
// Format Sidereal Time (decimal hours) to a human-readable sting
//
void FormatSiderealTime (double st, char *s)
{
  double st_hr, st_min, st_sec;
  st_hr = floor (st);
  st -= st_hr;
  st_min = floor (st * 60);
  st -= st_min / 60;
  st_sec = st * 3600;

  sprintf (s, "%2.0f:%02.0f:%05.3fs", st_hr, st_min, st_sec);
}

//
// Parse latitude string in format <DD MM' H> to latitude arcseconds
//
double LatStringDMToArcsec (const char* latString)
{
  double rc = 0;

  int d = 0, m = 0;
  char h = 'N';
  if (sscanf (latString, "%d %d' %c", &d, &m, &h) == 3) {
    rc = (double)(d) * 3600.0 + (double)(m) * 60.0;
    if ((h == 's') || (h == 'S')) rc = -rc;
  } else {
    WARNINGLOG ("LatStringDMToArcsec : Cannot parse \"%s\"", latString);
  }

  return rc;
}

//
// Parse longitude string in format <DDD MM' H> to longitude arcseconds
//
double LonStringDMToArcsec (const char* lonString)
{
  double rc = 0;

  int d = 0, m = 0;
  char h = 'E';
  if (sscanf (lonString, "%d %d' %c", &d, &m, &h) == 3) {
    rc = (double)(d) * 3600.0 + (double)(m) * 60.0;
    if ((h == 'W') || (h == 'W')) {
      rc = (360.0 * 3600.0) - rc;
    }
  } else {
    WARNINGLOG ("LonStringDMToArcsec : Cannot parse \"%s\"", lonString);
  }

  return rc;
}

//
// Parse latitude string in format <DD MM'SS" H> to latitude arcseconds
//
double LatStringDMSToArcsec (const char* latString)
{
  double rc = 0;

  int d = 0, m = 0, s = 0;
  char h = 'N';
  if (sscanf (latString, "%d %d'%d\" %c", &d, &m, &s, &h) == 4) {
    rc = (double)(d) * 3600.0 + (double)(m) * 60.0 + (double)(s);
    if ((h == 's') || (h == 'S')) rc = -rc;
  } else {
    WARNINGLOG ("LatStringDMSToArcsec : Cannot parse \"%s\"", latString);
  }

  return rc;
}

//
// Parse longitude string in format <DDD MM'SS" H> to longitude arcseconds
//
double LonStringDMSToArcsec (const char* lonString)
{
  double rc = 0;

  int d = 0, m = 0, s = 0;
  char h = 'E';
  if (sscanf (lonString, "%d %d'%d\" %c", &d, &m, &s, &h) == 4) {
    rc = (double)(d) * 3600.0 + (double)(m) * 60.0 + (double)(s);
    if ((h == 'W') || (h == 'W')) {
      rc = (360.0 * 3600.0) - rc;
    }
  } else {
    WARNINGLOG ("LonStringDMSToArcsec : Cannot parse \"%s\"", lonString);
  }

  return rc;
}

//
// Convert CIE Yxy triplet to CIE XYZ
//
// Input parameters:
//  Y CIE Yxy luminance Y value   [0..100]
//  x CIE Yxy chrominance x value   [0..1]
//  y CIE Yxy chrominance y value   [0..1]
//
void CIE_Yxy_to_XYZ (const SCIE in, SCIE &out)
{
  out.XYZ.X = in.Yxy.x * (in.Yxy.Y / in.Yxy.y);
  out.XYZ.Y = in.Yxy.Y;
  out.XYZ.Z = (1 - in.Yxy.x - in.Yxy.y) * (in.Yxy.Y / in.Yxy.y);
}

//
// Convert CIE XYZ triplet to RGB colour using the D65 white point.
//
void CIE_XYZ_to_RGB_D65 (const SCIE XYZ, sgdVec3 &RGB)
{
  // Now multiply XYZ vector by D65 white point matrix
  float mWhite[3][3];
  mWhite[0][2] =  3.240479f;
  mWhite[0][1] = -1.537150f;
  mWhite[0][0] = -0.498535f;

  mWhite[1][2] = -0.969256f;
  mWhite[1][1] =  1.875992f;
  mWhite[1][0] =  0.041556f;

  mWhite[2][2] =  0.055648f;
  mWhite[2][1] = -0.204043f;
  mWhite[2][0] =  1.057311f;

  RGB[0] = ((mWhite[0][2] * XYZ.XYZ.X) + (mWhite[0][1] * XYZ.XYZ.Y) + (mWhite[0][0] * XYZ.XYZ.Z)) / 100.0;
  RGB[1] = ((mWhite[1][2] * XYZ.XYZ.X) + (mWhite[1][1] * XYZ.XYZ.Y) + (mWhite[1][0] * XYZ.XYZ.Z)) / 100.0;
  RGB[2] = ((mWhite[2][2] * XYZ.XYZ.X) + (mWhite[2][1] * XYZ.XYZ.Y) + (mWhite[2][0] * XYZ.XYZ.Z)) / 100.0;
}


//============END OF FILE ===================================================================


