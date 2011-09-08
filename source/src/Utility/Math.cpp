/*
 * Math.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
 * Copyright 2007 Jean Sabatier
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

/*! \file Math.cpp
 *  \brief Math utility functions including converters, calculators, etc.
 */

#include "../Include/Utility.h"
#include "../Include/3dMath.h"
#include "../Include/GeoMath.h"
//=============================================================
// Wrap a longitude value in arcsec to the range [0, 1.296E+6)
// (corresponds to 360 degrees * 3600 arcsec/deg)
//==========================================================
double WrapLongitude (double lon)
{
  double wrap = 1.296e+6;

  while (lon < 0.0)
    lon += wrap;
  while (lon >= wrap)
    lon -= wrap;

  return lon;
}

//=====================================================================
// Gets the difference of two angles in degrees
// a = first angle   b = second angle
// a neg result is to the left of a
// a pos result is to the right of a
//======================================================================
int DiffOn360Circle (const int &a, const int &b)
{
  int d = b - a;
  if (d > 180)        return (d - 360);
  else if (d < -180)  return (360 + d);
                      return d;
}

//======================================================================
// Wraps a longitude value in arcsec to the range [-648000, 648000)
// (corresponds to +/-180 degrees * 3600 arcsec/deg)
//======================================================================
double Wrap180Longitude (const double &lon)
{
  double wrap = lon;
  if (lon  >  648000.0)
    wrap = lon - 1296000.0;
  return wrap;
}

//
// transforms rads in +/- 324000 arcsec
// input : +/- PI/2
//
double radians2arcseconds_lat (const double &latrad) {
  return (latrad * RAD_TO_ARCSEC);
}

//
// transforms rads in 0-1296e3 arcsec
// input : +/-PI
//
double radians2arcseconds_lon (const double &lonrad) {
  return (WrapLongitude (lonrad * RAD_TO_ARCSEC));
}

//
// transforms +/- 324000 arcsec in +/- PI/2 rads
//
double arcseconds_lat2radians (const double &lat) {
  return (lat * ARCSEC_TO_RAD);
}

//
// transforms 0-1296e3 arcsec in +/- PI rads
//
double arcseconds_lon2radians (const double &lon) {
  double tmp_lon = lon;
  return (Wrap180Longitude (tmp_lon) * ARCSEC_TO_RAD);
}
//===========================================================================
// Tag utilities
//===========================================================================
Tag StringToTag (const char* s)
{
  Tag tag = 0;

  tag = (s[0] << 24)
    + (s[1] << 16)
    + (s[2] << 8)
    + (s[3]);
  return tag;
}

char *TagToString (Tag tag)
{
  static char s[8];
  TagToString (s, tag);
  return s;
}

char *TagString (char* s, Tag tag)
{
  TagToString (s, tag);
  return s;
}

void TagToString (char* s, Tag tag)
{ if (0 == tag) {strcpy(s,"*none*"); return;}
  s[0] = (char)((tag >> 24) & 0xff);
  s[1] = (char)((tag >> 16) & 0xff);
  s[2] = (char)((tag >> 8) & 0xff);
  s[3] = (char)((tag) & 0xff);
  s[4] = '\0';
  return;
}
//==================================================================================
//  Fast sine computation based on parabola approximation of sin(x)
//==================================================================================
//===========================================================================
double WrapArcsec (double arcsec)
{
  double rc = arcsec;

  static const double WrapValue = (360 * 3600);

  while (rc < 0) rc += WrapValue;
  while (rc > WrapValue) rc -= WrapValue;

  return rc;
}
//==================================================================================
// Calculate the number of feet in one arcsecond of longitude at the given latitude.
//
// Arguments:
//    lat     Latitude in arcseconds (+ is N hemisphere)
//==================================================================================
double FeetPerLonArcsec (double lat)
{
  double latRad = FN_RAD_FROM_ARCS(lat);		// Latitude in radian
  double circumference = cos(latRad) * TC_FULL_WRD_FEET;
  double feetPerLonArcsec = circumference / (360.0 * 3600.0);

  return feetPerLonArcsec;
}

//==================================================================================
// Calculate a new position based on a starting point and a vector offset in feet:
//   v.x = E/W offset      (+ is East)
//   v.y = N/S offset      (+ is North)
//   v.z = Altitude offset (+ is higher)
//
// This function is only accurate for small vector offsets (a few miles)
//===================================================================================
SPosition AddVector(SPosition &from, SVector &v)
{
  SPosition to;

  // Calculate arcseconds of latitude offset based on constants defined above
  double arcsecLat = v.y / TC_FEET_PER_ARCSEC;
  to.lat = from.lat + arcsecLat;

  // Calculate actual number of feet per longitude arcsec at this latitude
  double feetPerLonArcsec = FeetPerLonArcsec (from.lat);
  double arcsecLon = v.x / feetPerLonArcsec;
  to.lon = WrapArcsec (from.lon + arcsecLon);

  // Altitude is already in feet, simply add the offset
  to.alt = from.alt + v.z;

  return to;
}

//==========================================================================
// SubtractVector computes a new global position based on a starting position
//   and an offset vector.  This is essentially the same as AddVector except
//   the distances are inverted (i.e. an argument to AddVector of +20 feet
//   is equivalent to the same argument to SubtractVector of -20 feet).
//==========================================================================
SPosition SubtractVector(SPosition &from, SVector &v)
{
  SVector vNew;
  vNew.x = -v.x;
  vNew.y = -v.y;
  vNew.z = -v.z;
  return AddVector (from, vNew);
}


//========================================================================
// Simple vector sum
//========================================================================
SVector VectorSum (SVector v1, SVector v2)
{
  SVector rc;
  rc.x = v1.x + v2.x;
  rc.y = v1.y + v2.y;
  rc.z = v1.z + v2.z;
  return rc;
}


//
// Simple vector subtraction
//
SVector VectorDifference (SVector v1, SVector v2)
{
  SVector rc;
  rc.x = v1.x - v2.x;
  rc.y = v1.y - v2.y;
  rc.z = v1.z - v2.z;
  return rc;
}


// Scale a vector (multiply by constant)
void VectorScale(SVector& v, double c) {
   v.x *= c;
   v.y *= c;
   v.z *= c;
}


// Multiply a Vector by a constant. Same as VectorScale() but returns the result without changing parameter v.
SVector VectorMultiply(const SVector &v, double c) {
  SVector rc = { v.x * c, v.y * c, v.z * c };
  return rc;
}


/*
 * Vector Scalar (dot) product
 */
double VectorDotProduct(const SVector& v1, const SVector& v2) {
   return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}


/*
 * Vector Vector (Cross) Product
 */
void VectorCrossProduct(SVector& result, const SVector& v1, const SVector& v2) {
   result.x = v1.y*v2.z - v1.z*v2.y;
   result.y = v1.z*v2.x - v1.x*v2.z;
   result.z = v1.x*v2.y - v1.y*v2.x;
}

//================================================================================
// CRotationMatrix methods
//================================================================================
// --------------------------------------
// CRotationMatrixHPB
// --------------------------------------

/// Constructor
CRotationMatrixHPB::CRotationMatrixHPB() {
   for (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
         m[i][j] = (i==j ? 1.0 : 0.0);
}

/// Setup
void CRotationMatrixHPB::Setup(SVector& bAng) {
   double sp = sin(bAng.x);
   double cp = cos(bAng.x);
   double sh = sin(bAng.y);
   double ch = cos(bAng.y);
   double sb = sin(bAng.z);
   double cb = cos(bAng.z);

   m[0][0] = ch*cb + sh*sp*sb;
   m[0][1] = -ch*sb + sh*sp*cb;
   m[0][2] = sh*cp;

   m[1][0] = sb*cp;
   m[1][1] = cb*cp;
   m[1][2] = -sp;

   m[2][0] = -sh*cb + ch*sp*sb;
   m[2][1] = sb*sh + ch*sp*cb;
   m[2][2] = ch*cp;
}
//------------------------------------------------------------------------------
//  SetUp 2
//------------------------------------------------------------------------------
void CRotationMatrixHPB::Setup2(SVector &bAng)
{  
}

/// Parent to child rotation
void CRotationMatrixHPB::ParentToChild(SVector &child, const SVector &parent) const {
   // multiply parent by matrix
   child.x = m[0][0]*parent.x + m[1][0]*parent.y +  m[2][0]*parent.z;
   child.y = m[0][1]*parent.x + m[1][1]*parent.y +  m[2][1]*parent.z;
   child.z = m[0][2]*parent.x + m[1][2]*parent.y +  m[2][2]*parent.z;
}
/// Parent to child rotation. Another version with array
void CRotationMatrixHPB::ParentToChild(double *dst, const SVector &parent) const {
   // multiply parent by matrix
   dst[0] = m[0][0]*parent.x + m[1][0]*parent.y +  m[2][0]*parent.z;
   dst[1] = m[0][1]*parent.x + m[1][1]*parent.y +  m[2][1]*parent.z;
   dst[2] = m[0][2]*parent.x + m[1][2]*parent.y +  m[2][2]*parent.z;
}


/// Child to parent rotation
void CRotationMatrixHPB::ChildToParent(SVector &parent, const SVector &child) const {
   // multiply parent by matrix transposed
   parent.x = m[0][0]*child.x + m[0][1]*child.y + m[0][2]*child.z;
   parent.y = m[1][0]*child.x + m[1][1]*child.y + m[1][2]*child.z;
   parent.z = m[2][0]*child.x + m[2][1]*child.y + m[2][2]*child.z;
}

//--------------------------------------
// CRotationMatrixBHP **
//--------------------------------------

/// Constructor
CRotationMatrixBHP::CRotationMatrixBHP() {

}

/// Setup
void CRotationMatrixBHP::Setup(SVector& bAng) {
  sgSetVec3 (axis, -1.0f, 0.0f, 0.0f);
  sgMakeRotMat4 (P, RadToDeg (bAng.x), axis);
  sgSetVec3 (axis, 0.0f, 1.0f, 0.0f);
  sgMakeRotMat4 (H, RadToDeg (bAng.y), axis);
  sgSetVec3 (axis, 0.0f, 0.0f, -1.0f);
  sgMakeRotMat4 (B, RadToDeg (bAng.z), axis);
  // Do local tranform first as B*H*P
  sgCopyMat4 (M, B);
  sgPostMultMat4 (M, H);
  sgPostMultMat4 (M, P);
  sgTransposeNegateMat4 (Mt, M);
  //sgInvertMat4 (Mt, M) ; //more costly

}

/// Parent to child rotation
void CRotationMatrixBHP::ParentToChild(SVector &child, const SVector &parent) const {
  // multiply parent by matrix
  sgVec3 tmp;
  sgSetVec3   (tmp, parent.x, parent.y, parent.z);
  sgXformVec3 (tmp, M);
  child.x = tmp[0];
  child.y = tmp[1];
  child.z = tmp[2];
}


/// Child to parent rotation
void CRotationMatrixBHP::ChildToParent(SVector &parent, const SVector &child) const {
   // multiply parent by matrix transposed
  sgVec3 tmp;
  sgSetVec3   (tmp, child.x, child.y, child.z);
  sgXformVec3 (tmp, Mt);
  parent.x = tmp[0];
  parent.y = tmp[1];
  parent.z = tmp[2];
}

// End CRotationMatrix



void VectorDistanceLeftToRight  (SVector &v)
{
  // To convert distances, simply swap Y and Z axes
  double temp = v.z;
  v.z = v.y;
  v.y = temp;
}

void VectorOrientLeftToRight (SVector &v)
{
  // Convert angles to right-handed coordinate system
  double p = -v.x;
  double h = -v.y;
  double b = -v.z;
  v.x = p;
  v.y = b;
  v.z = h;
}

void TurnPointFromFulcrum ( const float &a_in,
                            const float &b_in,
                            const float &angle_rad,
                            float &a_out,
                            float &b_out)
{
  double lon       = sqrt ((a_in * a_in) + (b_in* b_in));
  double new_angle = safeAtan2 (a_in, b_in) + angle_rad;
  a_out            = sin (new_angle) * lon;
  b_out            = cos (new_angle) * lon;
}

void TurnVectorFromFulcrum (const SVector &in, const SVector &ori, SVector &out)
{
  ///  invert rotation for right hand convention
  float x_4 = 0.0f,
        y_4 = 0.0f,
        z_4 = 0.0f;
  float x_1 = 0.0f,
        y_1 = 0.0f,
        z_1 = 0.0f;
  // H + clockwise
  TurnPointFromFulcrum (in.x, in.y, -ori.z, x_4, y_4);
  x_1 = x_4;
  y_1 = y_4;
  // R + clockwise
  TurnPointFromFulcrum (x_1, in.z, -ori.y, x_4, y_4);
  x_1 = x_4;
  z_1 = z_4;
  // P + clockwise
  TurnPointFromFulcrum (z_1, y_1, -ori.x, x_4, y_4);
  y_1 = y_4;
  z_1 = z_4;
  //
  out.x = x_1;
  out.y = y_1;
  out.z = z_1;
}

void BodyVector2WorldPos (const SPosition &cgPos, const SVector &body, SPosition &world)
{
  // basically same as SPosition AddVector(SPosition &from, SVector &v)
  // but adapted to its purpose and work with a reference

  // Calculate arcseconds of latitude offset based on constants defined above
  double arcsecLat = body.y / TC_FEET_PER_ARCSEC;
  world.lat = cgPos.lat + arcsecLat;

  // Calculate actual number of feet per longitude arcsec at this latitude
  double feetPerLonArcsec = FeetPerLonArcsec (cgPos.lat);
  double arcsecLon = body.x / feetPerLonArcsec;
  world.lon = WrapArcsec (cgPos.lon + arcsecLon);

  // Altitude is already in feet, simply add the offset
  world.alt = cgPos.alt + body.z;
}

///
/// GreatestCommonDivisor
///
/// \brief Calculate the greatest common divisor (GCD) of two integers using Euclid's algorithm
///
int GreatestCommonDivisor (int i, int j)
{
  // Apply Euclid's algorithm to find the GCD
  while (0 != j) {
    int temp = j;
    j = i % j;
    i = temp;
  }
  return i;
}

///
/// AspectRatio
///
/// \brief Calculate the w:h aspect ratio of a screen resolution
///
/// \param w      Screen width
/// \param h      Screen height
/// \param num    Updated with numerator of the aspect ratio
/// \param denom  Updated with the denominator of the aspect ratio
void AspectRatio (int w, int h, int &num, int &denom)
{
  // Find greatest common divisor (GCD) of w and h using Euclid's algorithm
  int gcd = GreatestCommonDivisor (w, h);

  // Calculate numerator and denominator by dividing GCD into w and h
  num = w / gcd;
  denom = h / gcd;
}
