/*
 * Utility.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
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

/*! \file Utility.h
 *  \brief Header for miscellaneous utility functions
 *
 *  A large collection of miscellaneous standalone functions and classes
 *    used throughout the Fly! Legacy project.
 */

#ifndef UTILITY_H
#define UTILITY_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <vector>
#include <map>
#include <plib/sg.h>
#include "../Include/Stream.h"
#include "../Include/TerrainUnits.h"
//
// Various constants
//
//=============================================================================
#define KOLLMAN_CTE 924.8253					// feet per unit of Hg
//=============================================================================
#define MILES_PER_NM                         (1.151)
#define KM_PER_NM                            (1.852)
#define METRES_PER_FOOT                      (0.3048)
#define SQMETRES_PER_SQFOOT                  (0.09290304)
#define FEET_PER_NM                          (6076.0)
#define METERS_PER_NM(X)                     (double(X) * FEET_PER_NM * METRES_PER_FOOT)
#define LBS_PER_KILO                         (1.0 / 0.4535924)
#define SLUG_TO_KILO                         (14.59390)
#define MILE_PER_FOOT	                       (0.000164)
#define NMILE_PER_METRE_SEC(X)        (double(X) * 0.0005399568035 * 3600) 
#define METRES_SEC_PER_KNOT(X)        (METERS_PER_NM(X) / 3600)
// Assume polar circumference of 24,818 statue miles
#define POLAR_CIRCUMFERENCE                   double(1.3104211E+08)
#define FEET_PER_LAT_ARCSEC                   double(1.0111274E+02)
//--------------------------------------------------------------------
// Artificial "mean" circumference of 21,600 nautical miles
#define MEAN_CIRCUMFERENCE                   (FEET_PER_NM * 21600)
#define MILE_CIRCUMFERENCE                   (21600)
//--------------------------------------------------------------------
#define METERS_PER_ARCSEC               float(METRES_PER_FOOT * FEET_PER_ARCSEC)
//--------------------------------------------------------------------
// constant for the earth (FrameManager-Geodesy)
//--------------------------------------------------------------------
#define ARCSEC_360DEG    (double(360.0 * 3600.0))
#define RAD_TO_ARCSEC    (ARCSEC_360DEG  / double(TWO_PI))
#define ARCSEC_TO_RAD    (double(TWO_PI) / ARCSEC_360DEG)
#define ONE_ARCSEC                           (ARCSEC_TO_RAD)
#define EQUATORIAL_RADIUS_FEET               (3963.19 * 5280)
#define EQUATORIAL_RADIUS_M                  (EQUATORIAL_RADIUS_FEET * METRES_PER_FOOT)
#define EQ_RAD_SQUARE_M                      (EQUATORIAL_RADIUS_M * EQUATORIAL_RADIUS_M)

#define EARTH_SIDERAL_ROTATION_RATE_SI (double(0.00007272205217))   // rad/sec
#define MIN_EARTH_RADIUS_SI            (double(6356750.0))          // meters
#define MAX_EARTH_RADIUS_SI            (double(6378135.0))          // metres
#define EARTH_RADIUS_SI                (0.5 * (MIN_EARTH_RADIUS_SI + MAX_EARTH_RADIUS_SI))  // meters

#define ACC_GRAVITY                    (double(32.174))                           // feet/sec
#define ACC_GRAVITY_ISU                (ACC_GRAVITY * double(METRES_PER_FOOT))    // feet/sec
#define SEC_IN_DAY                      (3600 * 24)
#define HOUR_PER_SEC                    ( double(1) / 3600)
//==================================================================================
// pressure utilities
//==================================================================================
#define PSF_TO_INHG       (double (29.9212) / 2116.217)
#define INHG_TO_PSF       (double (2116.217)/ 29.9212)
#define SLUGS_FT3_TO_GCM3 (double (0.5153788))
#define SLUGS_FT3_TO_KGM3 (double (SLUGS_FT3_TO_GCM3 * 1e3))
#define INHG_TO_PAS       (double (3386.389))
#define PAS_TO_INHG       (double(1) / INHG_TO_PAS)
#define PFS_TO_PAS        (double (PSF_TO_INHG * INHG_TO_PAS))
#define PAS_TO_PFS        (double (PAS_TO_INHG * INHG_TO_PSF))
#define IN3_TO_M3         (double (1.638706E-5))
#define MBAR_TO_INHG      (double(0.029529983071))
#define PFS_TO_HPA        (double(0.478803))
#define HPA_TO_PSF        (double(1) / PFS_TO_HPA)
//========================================================================
// square function x*x
//========================================================================
template <class T> static inline T sqr (T val)
{
  return (val * val);
}

/// Compute the sin and cosine of an angle.
/// On some platforms, if we know that we
/// need both values, it can be computed
/// faster than computing the two values
/// seperately.

inline void sinCos(float &returnSin, float &returnCos, const float theta) {
	// For simplicity, we'll just use the normal trig functions.
	// Note that on some platforms we may be able to do better
	returnSin = sin(theta);
	returnCos = cos(theta);
}

// safeAcos
//
// Same as acos(x), but if x is out of range, it is "clamped" to the nearest
// valid value.  The value returned is in range 0...pi, the same as the
// standard C acos() function

inline float safeAcos(float x) {

	// Check limit conditions
	if (x <= -1.0f) return (float)PI;
	if (x >=  1.0f)	return 0.0f;
	// Value is in the domain - use standard C function
	return acos(x);
}

// safeAcos
//
// Same as acos(x), but if x is out of range, it is "clamped" to the nearest
// valid value.  The value returned is in range -pi/2...pi/2, the same as the
// standard C asin() function

inline float safeAsin(float x) {

	// Check limit conditions
	if (x <= -1.0f) return -(float)HALF_PI;
	if (x >=  1.0f)	return  (float)HALF_PI;
	// Value is in the domain - use standard C function
	return asin(x);
}

inline float safeAtan2(float rise, float run) // from http://www.koders.com/ 3d.cc
{
	float result;

	if ((rise < 0.00001) && (rise > -0.00001))
		rise = 0;

	if ((run < 0.00001) && (run > -0.00001))
		run = 0;

//	printf("safeatan2: rise  = %f, run = %f, result = %f\n",rise, run, result);
	result = atan2(rise,run);
	return(result);
}

//=============================================================================
// Utility functions for Fly
//    <geop> coordinates
//
//=============================================================================
//
// Wraps a longitude value in arcsec to the range [-648000, 648000)
// (corresponds to +/-180 degrees * 3600 arcsec/deg)
//-------------------------------------------------------------------
double Wrap180Longitude (const double &lon);

//-------------------------------------------------------------------
// transforms rads in +/- 324000 arcsec
//-------------------------------------------------------------------
double radians2arcseconds_lat (const double &latrad);
//-------------------------------------------------------------------
// transforms rads in 0-1296e3 arcsec
//-------------------------------------------------------------------
double radians2arcseconds_lon (const double &lonrad);
//-------------------------------------------------------------------
// transforms +/- 324000 arcsec in rads
//-------------------------------------------------------------------
double arcseconds_lat2radians (const double &lat);
//-------------------------------------------------------------------
// transforms 0-1296e3 arcsec in rads
//-------------------------------------------------------------------
double arcseconds_lon2radians (const double &lon);
//==============================================================================
// Utility\Geodesy.cpp functions
//==============================================================================
//
// Convert an SPosition from geocentric to (WGS84) geodetic systems
//-------------------------------------------------------------------
SPosition GeocToGeod (SPosition);
//-------------------------------------------------------------------
// Convert an SPosition from WGS84 geodetic to geocentric systems
//-------------------------------------------------------------------
SPosition GeodToGeoc (SPosition);
//-------------------------------------------------------------------
// Convert a geocentric SPosition (lat,lon,alt) to cartesian coordinates
//   in an SVector struct
//-------------------------------------------------------------------
SVector GeocToCartesian (SPosition);
//-------------------------------------------------------------------
// Convert cartesian coordinates to a geocentric SPosition
//-------------------------------------------------------------------
SPosition CartesianToGeoc (SVector);
//
// Convert a geodetic SPosition (lat,lon,alt) to cartesian coordinates
//   in an SVector struct
//
SVector GeodToCartesian (SPosition);
//
// Convert cartesian coordinates to a geodetic SPosition
//
SPosition CartesianToGeod (SVector);

void test_geodesy (void);

//
// Calculate great circle distance between two positions based on WGS84
//   geodetic coordinates
//
float GreatCircleDistance(SPosition *from, SPosition *to);
//
// Calculate a WGS84 geodetic position based on a starting position (also
//   in WGS84 geodetic coordinates, and an azimuth/range specified in
//   the SVector argument.  The 'h' heading (radians, 0 = true north)
//   and 'r' range (in feet) are the only fields used
//
SPosition GreatCirclePosition(SPosition *from, SVector *polar);
//
// Calculate the azimuth bearing (in radians, 0 = true north) and
//   range (in feet) between two positions in WGS84 geodetic coordinates
SVector GreatCirclePolar(SPosition *from, SPosition *to);
//=====================================================================================
// Simple vector addition
//=====================================================================================
SVector VectorSum (SVector v1, SVector v2);
//
// Simple vector subtraction
//
SVector VectorDifference (SVector v1, SVector v2);
/// Scale a Vector
void VectorScale(SVector &v, double c);
// Multiply a Vector by a constant. Same as VectorScale() but returns the result without changing parameter v.
SVector VectorMultiply(const SVector &v, double c);
///< Vector scalar (dot) product
double VectorDotProduct(const SVector& v1, const SVector& v2);
///< Vector cross product
void VectorCrossProduct(SVector& result, const SVector& v1, const SVector& v2);

/*! Class RotationMatrix is used to transform vector coordinates between rotated systems.
 *
 *  This class is based on information from "3D Math Primer for Graphics and Game Development"
 *  by Fletcher Dunn and Ian Parberry.
 *
 *  Instances of the class are initiated with a set of Euler angles, representing the
 *  childs orientation relative to the parent. It transforms vectors from parent to
 *  child coordinate system by rotating in heading, pitch, bank order. Just like Fly!
 *  reference = LH left handed
 */
class CRotationMatrixHPB {
private:
   double m[3][3]; ///< The rotation matrix

public:
   /// Constructor:
   CRotationMatrixHPB();

   /// Set up rotatation matrix for Euler angles bAng
   void Setup(SVector& bAng);
   void Setup2(SVector &ang);
   /// Transform vector from parent to child coordinate system
   void ParentToChild(SVector& child, const SVector& parent) const;
  void  ParentToChild(double *dst, const SVector &parent)    const;
   /// Transform vector from child to parent coordinate system
   void ChildToParent(SVector& parent, const SVector& child) const;
};
//============================================================================
class CRotationMatrixBHP { // reference = LH left handed
private:
   sgVec3   axis;
   sgMat4   B,P,H,Y,X,Z,M,Mt; ///< The rotation matrix

public:
   /// Constructor:
   CRotationMatrixBHP();

   /// Set up rotatation matrix for Euler angles bAng
   void Setup(SVector& bAng);
   /// Transform vector from parent to child coordinate system
   void ParentToChild(SVector& child, const SVector& parent) const;

   /// Transform vector from child to parent coordinate system
   void ChildToParent(SVector& parent, const SVector& child) const;
};



//----------------------------------------------------------------------
// Wrap a value to the range [-PI, PI), typically used to normalize
//   an angle in radians
//----------------------------------------------------------------------
template <class T> static inline T WrapPiPi (T rad)
{
  rad += (T)PI;
  rad -= floor (rad * (T)ONE_OVER_TWO_PI) * (T)TWO_PI;
  rad -= (T)PI;
  return rad;
}
//----------------------------------------------------------------------
// Wrap a value to the range [-PI/2, PI/2), typically used to normalize
//   an angle in radians
//----------------------------------------------------------------------
template <class T> static inline T WrapHalfPi (T rad)
{
  rad += (T)HALF_PI;
  rad -= floor (rad * (T)ONE_OVER_PI) * (T)PI;
  rad -= (T)HALF_PI;
  return rad;
}
//----------------------------------------------------------------------
// Wrap a radian value toward north pole
//   -Add Half PI and wrap to [0,2PI]
//----------------------------------------------------------------------
template <class T> static inline T RadToNorth(T rad)
{ // First add HALF_PI
  rad += (T)HALF_PI;
	if (rad > (T)TWO_PI)	rad -= (T)PI;
  return rad;
}
//----------------------------------------------------------------------
// Wrap a radian value from north pole
//   -substract Half PI and wrap to [0,2PI]
//----------------------------------------------------------------------
template <class T> static inline T RadFromNorth(T rad)
{ // First add HALF_PI
  rad -= (T)HALF_PI;
	if (rad < 0)	rad += (T)PI;
  return rad;
}
//----------------------------------------------------------------------
// Wrap a value to the range [0, PI), typically used to normalize
//   an angle in radians
//----------------------------------------------------------------------
template <class T> static inline T WrapPI (T rad)
{ int k = 4;
	while (rad < (T)0)		{rad += (T)PI; k--; if (0 > k) gtfo("WrapPi infinite");}
	while (rad >= (T)PI)  {rad -= (T)PI; k--; if (0 > k) gtfo("WrapPi infinite");}
  return rad;
}
//----------------------------------------------------------------------
// Wrap a value to the range [0, 2*PI), typically used to normalize
//   an angle in radians
//----------------------------------------------------------------------
template <class T> static inline T WrapTwoPi (T rad)
{ int k = 4;
	while (rad <  (T)0)				{ rad += (T)TWO_PI; k--; if (0 > k) gtfo("WrapTwoPi infinite");}
	while (rad >= (T)TWO_PI)	{ rad -= (T)TWO_PI; k--; if (0 > k) gtfo("WrapTwoPi infinite");}
  return rad;
}
//----------------------------------------------------------------------
// Wrap a value to the range [0, 360), typically used to normalize
//   an angle in degrees
//----------------------------------------------------------------------
template <class T> static inline T Wrap360 (T deg)
{ int k = 4;
  while (deg <  (T)0)		{deg += (T)360; k--; if (0 > k) gtfo("WRAP360 infinite");}
	while (deg >= (T)360) {deg -= (T)360; k--; if (0 > k) gtfo("WRAP360 infinite");}
  return deg;
}
//----------------------------------------------------------------------
// Wrap a value to the range [-180, 180), typically used to normalize
//   an angle in degrees
//----------------------------------------------------------------------
template <class T> static inline T Norme2PI (T rad)
{	while (rad <  (T)0)				rad  += (T)TWO_PI;
  while (rad >= (T)TWO_PI)	rad  -= (T)TWO_PI;
  return rad;
}
//----------------------------------------------------------------------
// Wrap a value to the range [-180, 180), typically used to normalize
//   an angle in degrees
//----------------------------------------------------------------------
template <class T> static inline T Wrap180 (T deg)
{	int k = 4;
	while (deg < (T)0)		{deg  += (T)360; k--; if (0 > k) gtfo("Wrap180 infinite");}
	while (deg >= (T)360) {deg  -= (T)360; k--; if (0 > k) gtfo("Wrap180 infinite");}
  if (deg > (T)180)  deg  -= (T)360;
  return deg;
}
//----------------------------------------------------------------------
// Wrap a value to the range [0, 24), typically used to normalize
//   a value representing hours of the day
//----------------------------------------------------------------------
template <class T> static inline T Wrap24 (T hrs)
{ int k = 4;
	while (hrs <  (T)0)   {hrs += (T)24; k--; if (0 > k)	gtfo("Wrap24 infinite");}
	while (hrs >= (T)24)  {hrs -= (T)24; k--; if (0 > k)	gtfo("Wrap24 infinite");}
  return hrs;
}
//----------------------------------------------------------------------
// Wrap a value to the range [0, 60), typically used to normalize
//   a value representing minutes in an hour or seconds in a minute
//----------------------------------------------------------------------
template <class T> static inline T Wrap60 (T min)
{ int k = 4;
	while (min <  (T)0)		{min += (T)60; k--; if (0 > k)	gtfo("Wrap60 infinite");}
	while (min >= (T)60)  {min -= (T)60; k--; if (0 > k)  gtfo("Wrap60 infinite");}
  return min;
}
//----------------------------------------------------------------------
// Wrap a value to the range [0, 1296000), typically used to normalize
//   a value representing arcsec in world coordinates
//----------------------------------------------------------------------
template <class T> static inline T WrapArcs (T arcs)
{ int k = 4;
	while (arcs <  (T)0)         {arcs += (T)(1296000);	k--; if (0 > k) gtfo("WrapArc infinite");}
	while (arcs >= (T)(1296000)) {arcs -= (T)(1296000); k--; if (0 > k) gtfo("WrapArc infinite");}
  return arcs;
}
//----------------------------------------------------------------------
// Clamp value to interval
//----------------------------------------------------------------------
template <class T> static inline T ClampTo(T v,T m, T M)
{	if (v < m)	return m;
	if (v > M)	return M;
	return v;	}
//----------------------------------------------------------------------
double  WrapLongitude    (double lon);
int     DiffOn360Circle  (const int &a, const int &b);
//----------------------------------------------------------------------
// Compute difference modulo 360
//----------------------------------------------------------------------
template <class T> inline T DifMod360(T a, T b)
{ T dif = (b - a);
  if (dif > +(T)180) return (dif - (T)360);
  if (dif < -(T)180) return (dif + (T)360);
  return   (dif);
}
//----------------------------------------------------------------------
//  Set a bit in a world
//  msk   must be one bit position ex (0x00100000)
//  val   must be the same value if set or 0 if reset
//----------------------------------------------------------------------
inline void SetBit(U_INT &wd,U_INT msk,U_INT val)
{   wd &= (-1) - (msk);         // Clear bit
    wd |= val;                  // set of not
    return;
}
//-----------------------------------------------------------------------------
//  return rounded value as integer
//-----------------------------------------------------------------------------
static int Round(float nb)
{ int round = int(nb * 4);
  int value = (round >> 2);
  if (round & 0x02) value++;
  return value;
}
//-----------------------------------------------------------------------------
//  return rounded value as integer
//-----------------------------------------------------------------------------
static int Round(double nb)
{ int round = int(nb * 4);
  int value = (round >> 2);
  if (round & 0x02) value++;
  return value;
}



//
// SubtractVector computes a new global position based on a starting position
//   and an offset vector.  This is essentially the same as AddVector except
//   the distances are inverted (i.e. an argument to AddVector of +20 feet
//   is equivalent to the same argument to SubtractVector of -20 feet).
//
SPosition SubtractVector(SPosition &from, SVector &v);


//
// Unit conversion functions
//
template <class T> static inline T NmToMi (T nm)
{
  return nm / (T)MILES_PER_NM;
}

template <class T> static inline T FeetToNm (T feet)
{
  return feet / (T)FEET_PER_NM;
}

template <class T> static inline T NmToFeet (T nm)
{
  return nm * (T)FEET_PER_NM;
}

template <class T> static inline T FpsToKt (T fps)
{
  return FeetToNm (fps * (T)3600);
}

template <class T> static inline T KtToFps (T Kt)
{
  return NmToFeet (Kt / (T)3600);
}

template <class T> static inline T FeetToMetres (T ft)
{
  return ft * (T)METRES_PER_FOOT;
}

template <class T> static inline T MetresToFeet (T m)
{
  return m * (T)TC_FEET_PER_METER;
}

template <class T> static inline T KmToNm (T km)
{
  return km * (T)KM_PER_NM;
}

template <class T> static inline T NmToKm (T nm)
{
  return nm / (T)KM_PER_NM;
}

template <class T> static inline T RadToDeg (T rad)
{
  return rad * (T)(180.0 / PI);
}

template <class T> static inline T DegToRad (T deg)
{
  return deg * (T)(PI / 180.0);
}

template <class T> static inline T LbsToKg (T lbs)
{
  return lbs * (T)LBS_TO_KGS;
}
//=============================================================================
//  Helper tto decode files
//=============================================================================
char *TagToString (Tag tag);
char *TagString (char* s, Tag tag);
Tag   StringToTag (const char* s);
void  TagToString (char* s, Tag tag);

void FormatPosition (SPosition pos, char* s);
void FormatRADec (double ra, double dec, char* s);
void FormatSiderealTime (double st, char *s);
double LatStringDMToArcsec (const char* latString);
double LonStringDMToArcsec (const char* lonString);
double LatStringDMSToArcsec (const char* latString);
double LonStringDMSToArcsec (const char* lonString);

void TrimTrailingWhitespace (char* s);
//=============================================================================
// polar inversions
template <class T> static inline void XyzToHpb (T &xyz)
{
    T tmp = xyz;   // 
    xyz.x = tmp.h; // x=z RH
    xyz.y = tmp.p; // y=x RH
    xyz.z = tmp.r; // z=y RH
}

template <class T> static inline void HpbToXyz (T &hpb)
{
    T tmp = hpb;   // 
    hpb.x = tmp.y; // y=p RH
    hpb.y = tmp.z; // z=r RH
    hpb.z = tmp.x; // x=h RH
}
//==============================================================================
// temperature utilities
//==============================================================================
template <class T> static inline T KelvinToFahrenheit (T kelvin)
{
  return (T)1.8 * kelvin - (T)459.4;
}

template <class T> static inline T FahrenheitToKelvin (T fahrenheit)
{
 return ((fahrenheit + (T)459.4) / (T)1.8);
}

template <class T> static inline T RankineToCelsius (T rankine)
{
  return (rankine - (T)491.67)/(T)1.8;
}

template <class T> static inline T CelsiusToRankine (T celsius)
{
  return ((celsius * (T)1.8) + (T)491.67);
}

template <class T> static inline T CelsiusToFahrenheit (T celsius)
{
  return celsius * (T)1.8 + (T)32.0;
}

template <class T> static inline T CelsiusToKelvin (T celsius)
{
  return (FahrenheitToKelvin (CelsiusToFahrenheit (celsius)));
}

template <class T> static inline T FahrenheitToCelsius (T fahrenheit)
{
  return (fahrenheit - (T)32.0)/(T)1.8;
}

template <class T> static inline T PsfToInchesHg (T psf)
{
  return (psf * (T)PSF_TO_INHG);
}

template <class T> static inline T InchesHgToPsf (T inches)
{
  return (inches * (T)INHG_TO_PSF);
}
//==============================================================================
// HP utilities
//==============================================================================
#define HP_TO_FTLBS(X) (double(X) * 550)      // IMPERIAL HP to Foot pound force / sec
#define FTLBS_TO_HP(X) (double(X) * 0.001818181) // Reverse
///------------------------------------------------------------
// weight mass utilities
static const double GRAVITY_FTS    = 32.173584;
static const double GRAVITY_MTS    = 9.81;
static const double LBS_TO_KGS     = 0.453592;
static const double LBFT2_TO_KGM2  = 4.214011e-2;   // lb.ft^2 -> kg.m^2

// force transformation :
// pound-force->Newton
static const double LBS_TO_NEWTON  = 4.448222;
// torque = pound-force foot (lbf.ft) -> Newton.meter
static const double LBFFT_TO_NM    = 1.355818;

// moment transformation :
// moment = lb.ft -> kg.m
template <class T> static inline T LBFT_TO_KGM (const T &lbs, const T &ft)
{
 return ((lbs / static_cast<T>(LBS_PER_KILO)) * (ft / TC_FEET_PER_METER));
}
//==============================================================================
// Misc math utilities
//==============================================================================
template <class T> static inline T max3 (T v1, T v2, T v3)
{
  T rc = v1;
  if (v2 > rc) rc = v2;
  if (v3 > rc) rc = v3;
  return rc;
}
//==============================================================================
  inline  float Norme180(float deg)
  { if (deg < -180) deg += 360;
    if (deg > +180) deg -= 360;
    return deg;}
  inline  float Clamp01(float val)
  { if (val < 0)    return 0;
    if (val > 1)    return 1;
    return val;}
//==============================================================================
// Radio functions
//==============================================================================
  //---------Frequency check --------------------------------------------
inline bool GoodILSfrequency(float f) {return ((f >= 108) && (f < 118));}
inline bool GoodNAVfrequency(float f) {return ((f >= 108) && (f < 118));}
inline bool GoodCOMfrequency(float f) {return ((f >= 118) && (f < 137));}
//==============================================================================
// The following struct is used to represent CIE chrominance/luminance data
//   triplets, either Yxy or XYZ forms
//==============================================================================
typedef struct {
  union {
    struct {
      float Y;
      float x;
      float y;
    } Yxy;
    struct {
      float X;
      float Y;
      float Z;
    } XYZ;
  };
} SCIE;


//=========================================================================================
// Convert CIE Yxy triplet to CIE XYZ
//
// Input parameters:
//  Y CIE Yxy luminance Y value   [0..100]
//  x CIE Yxy chrominance x value   [0..1]
//  y CIE Yxy chrominance y value   [0..1]
//=========================================================================================
void CIE_Yxy_to_XYZ (SCIE in, SCIE &out);
//-----------------------------------------------------------------------------
// Convert CIE XYZ triplet to RGB colour using the D65 white point.
//
void CIE_XYZ_to_RGB_D65 (SCIE XYZ, sgdVec3 &RGB);
//-----------------------------------------------------------------------------
void    DrawNoticeToUser (char* text, float timeout);
//========================================================================
//  Return standard alphabet (ALPHA, BRAVO etc) from the first letter
//  (car). In Main.cpp
//========================================================================
char *GetStandardAlphabet(char car);
//==========================================================================
//--------------------------------------------------------------------------
// CImageBMP
//
// Implemented in Utility/ImageBMP.cpp
//--------------------------------------------------------------------------
#pragma pack(push, 1)
typedef struct {
  unsigned short      magic;        ///< Magic identifier 'BM' little-endian
  unsigned long       size;         ///< Total size of file in bytes
  unsigned short      unknown1;
  unsigned short      unknown2;
  unsigned long       offset;       ///< Byte offset to start of image data
} SBmpHeader;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
  unsigned long       size;         ///< Info header size in bytes
  unsigned long       w, h;         ///< Image width/height in pixels
  unsigned short      nPlanes;      ///< Number of colour planes
  unsigned short      bpp;          ///< Bits per pixel
  unsigned long       compression;  ///< Compression type
  unsigned long       imageSize;    ///< Image size in bytes
  unsigned long       xResolution;  ///< X resolution in pixels per metre
  unsigned long       yResolution;  ///< Y resolution in pixels per metre
  unsigned long       nColours;     ///< Total number of colours
  unsigned long       nUsedColours; ///< Number of unique colours used
} SBmpInfoHeader;
#pragma pack(pop)
//================================================================================
//  BMP BITMAP
//================================================================================
class CImageBMP {
public:
  // Constructor/Destructor
  CImageBMP (const char* bmpFilename);
  ~CImageBMP (void);

  // CImageBMP methods
  unsigned long  GetWidth (void);
  unsigned long  GetHeight (void);
  void           DrawToSurface (SSurface* s, int x, int y);
  void           DrawTransparentToSurface (SSurface* s, int x, int y, unsigned int rgb);

protected:
  char            filename[PATH_MAX];
  SBmpHeader      h;
  SBmpInfoHeader  ih;
  unsigned char*  cmap;
  unsigned char*  image;
};
//================================================================================
// CBitmappedFontChar
//================================================================================
typedef struct {
  int     x, y;
} SFontCharBit;

//================================================================================
//  Character definition for default fonts
//================================================================================
class CBitmappedFontChar {
public:
  CBitmappedFontChar (void);
  ~CBitmappedFontChar (void);
  
  void SetBit (int x, int y);
  void Draw (SSurface *surface, int x, int y, unsigned int colour);
  int  GetWidth (void);
  int  GetHeight (void);

public:
  unsigned char         ascii;       ///< ASCII code for this character
  int                   start, end;  ///< Start/end column in RAW image data
  int                   w, h;        ///< Character width/height in pixels
  std::vector<SFontCharBit*> bits;        ///< Vector of set bits in the character
};

//================================================================================
//  Abstract class for all FONTS
//  This is to contril some functions that are implemented in all fonts
//================================================================================
class CFont {
public:
  virtual int   DrawChar   (SSurface *sf, int x, int y, char c,U_INT rgb)                   {return 0;}
  virtual int   DrawNText  (SSurface *sf,int x, int y, U_INT rgb, char* text)               {return 0;}
  virtual int   DrawLimChar(SSurface *sf,int x, int y, int Nb, U_INT rgb, char* text)       {return 0;}
  virtual int   DrawNbrChar(SSurface *sf,int x, int y, U_INT rgb,char* text, int nb)        {return 0;}
  virtual int   DrawTextV  (SSurface *surface,int x, int y, U_INT rgb, char* text, int dh)  {return 0;}
  virtual void  DrawTextC  (SSurface *sf,int x, int y, U_INT rgb, char* text)           {}
  virtual void  DrawTextR  (SSurface *sf,int x, int y, U_INT rgb, char* text)           {}
  virtual void  ReptChar  (SSurface *sf, int x0, int y0, int x1, U_INT rgba,char ascii) {}
  //-----------------------------------------------------------------------------------------
  virtual  int  TextWidth  (char* text)   {return 0;}
  virtual  int  TextHeight (char* text)   {return 0;}
  virtual  int  CharHeight (U_CHAR c)     {return 0;}
  virtual  int  CharWidth  (U_CHAR c)     {return 0;}
  virtual  int  MaxCharHeight ()          {return 0;};
  virtual  int  MaxCharWidth ()           {return 0;}
  virtual  void IncWidth(int k)           {}
  //---------------------------------------------------------------------------------------
  virtual void  SetTransparent()  {;}
  virtual void  SetOpaque()       {;}
	//--- Attribute is for start of text in surface ---------------------------------
	U_SHORT lin;														// Line Y
	U_SHORT cln;														// Column X
	U_SHORT adl;														// Line adjust
};

//=================================================================================
// CBitmappedFont
//
// This class represents a Fly! II bitmapped font.  Data members are public for a
//   couple of reasons:
//     - access needed by font utility functions
//     - some default fonts are not loaded from FNT control files, so function
//         init_default_fonts() in Main/Main.cpp initializes the members directly
//================================================================================
class CBitmappedFont: public CFont {
public:
  // Constructors
  CBitmappedFont (const char* fntFilename = NULL);
  ~CBitmappedFont (void);

  // CBitmappedFont methods
  int       Load (const char* fntFilename);
  int       LoadRaw (const char* rawFilename);
  //--------------------------------------------------------------------
  int       DrawChar  (SSurface *sf, int x, int y, char c,U_INT r );
  int       DrawNText (SSurface *sf,int x, int y, U_INT rgb, char* text);
  int       DrawLimChar(SSurface *sf,int x, int y, int Nb, U_INT rgb, char* text);
  int       DrawNbrChar(SSurface *sf,int x, int y, U_INT rgb,char* text, int nb);
  int       DrawTextV (SSurface *surface,int x, int y, U_INT rgb, char* text, int dh);
  void      DrawTextC (SSurface *sf,int x, int y, U_INT rgb, char* text);
  void      DrawTextR (SSurface *sf,int x, int y, U_INT rgb, char* text);
  void      ReptChar  (SSurface *sf, int x0, int y0, int x1, U_INT rgba,char ascii) {}
  //--------------------------------------------------------------------
  bool      IsCharValid (unsigned char c);
  int       TextWidth  (char* text);
  int       TextHeight (char* text);
  int       CharWidth  (U_CHAR c);
  int       CharHeight (U_CHAR c);
  //---------------------------------------------------------------
  int       GetWidth()  {return w;}
  //---------------------------------------------------------------
  CBitmappedFontChar *GetAscii (unsigned char c);

public:

  char   rawFilename[64];      // Filename of .RAW image data
  int    w, h;                 // RAW image width/height
  int    start, end;           // ASCII character bounds of font data
  unsigned char   transparent;          // Palette index of background
  int    space;                // Width of a space character in pixels
  int    shadow_x, shadow_y;   // Shadow offset x, y
	//---------------------------------------------------------
  CBitmappedFontChar  *chars;     // Array of font characters
};

//================================================================================
//  Class CMonoFontBMP
//  FONT defined by a bMP file in index color mode.
//  Fixed pace
//  One byte per color
//================================================================================
class CMonoFontBMP: public CFont
{
  protected:
  //-----------------------------------------------------------
  SBmpHeader      h;
  SBmpInfoHeader  ih;
  char*           image;
  short           wd, ht;
  short           wb;
  int             rowSize;
  short           uDim;
  U_CHAR          start;                  // First char
  U_CHAR          end;                    // Last char
  U_INT           mask;                   // Opaque mask
  //-------------------------------------------------------------
public:
   CMonoFontBMP(const char *bmpFilename,short from, short to);
  ~CMonoFontBMP();
  void            Load (PODFILE *p);
  int             DrawChar (SSurface *sf,int x0, int y0,char k,U_INT rgba);
  int             DrawNText(SSurface *sf,int x,  int y, U_INT rgb,char* text);
  int             DrawLimChar(SSurface *sf,int x, int y, int Nb, U_INT rgb, char* text)       {return 0;}
  int             DrawNbrChar(SSurface *sf,int x, int y, U_INT rgb,char* text, int nb)        {return 0;}
  int             DrawTextV  (SSurface *surface,int x, int y, U_INT rgb, char* text, int dh)  {return 0;}
  void            DrawTextC  (SSurface *sf,int x, int y, U_INT rgb, char* text);
  void            DrawTextR (SSurface *sf,int x, int y, U_INT rgb, char* text) {}
  void            ReptChar (SSurface *sf, int x0, int y0, int x1, U_INT rgba,char ascii);

  void            DrawNText(SSurface *sf,int nc,int x, int y, U_INT rgba,char* text);
  void            DrawNText(SSurface *sf,int x, int y, U_INT rgba,char* text,int nc);
  void            DrawSText(SSurface *sf,int x, int y, U_INT rgba,char* text,int nc);
  void            DrawFText(SSurface *sf,int x, int y, U_INT rgba,char* text);
  //-------------------------------------------------------------
  int             TextWidth  (char* text);
  int             TextHeight (char* text);
  int             CharHeight (U_CHAR c)     {return ht;}
  int             CharWidth  (U_CHAR c)     {return wd;}
  int             MaxCharHeight ()          {return ht;}
  int             MaxCharWidth ()           {return wd;}
  int             Space()                   {return wd;}
  //-------------------------------------------------------------
  inline void     SetWidth(int w)  {wd = w;}
  inline void     SetHeight(int h) {ht = h;}
  inline void     SetOpaque()       {mask = 0xFF000000;}
  inline void     SetTransparent()  {mask = 0xFFFFFFFF;}
  inline void     IncWidth(int k)   {wd += k;}
};
//================================================================================
//  Class CVariFontBMP
//  FONT defined by a bMP file in index color mode.
//  Variable pace
//  One byte per color
//  The font include an extra line where a dot marks each character position
//  This line is processed at font initialization to compute the offset and size of
//  each character
//================================================================================
class CVariFontBMP: public CFont
{ typedef struct {  U_SHORT offset;
                    U_SHORT wide;
} VCHAR_DEF;
  //------------ATTRIBUTES ----------------------------------------------
  SBmpHeader      h;
  SBmpInfoHeader  ih;
  char*           image;
  char*           imgAD;
  short           wd, ht;
  short           wb;
  int             rowSize;
  short           uDim;
  U_CHAR          start;                  // First char
  U_CHAR          end;                    // Last char
  U_INT           mask;                   // Opaque mask
  VCHAR_DEF      *DefTAB;                 // Char definition table
  //------------METHODS--------------------------------------------------
public:
   CVariFontBMP (const char *fname,short c1,short cf);
  ~CVariFontBMP ();
  void            Load (PODFILE *p);      // Load the font
  void            BuildDefTAB(short lgr,char *fn); // Build definition table
  //---------------------------------------------------------------------
  inline void     SetOpaque()       {mask = 0;}
  inline void     SetTransparent()  {mask = 0xFFFFFFFF;}
  //---------------------------------------------------------------------
  int             DrawChar(SSurface *sf,int x,int y,char c,U_INT rgba);
  int             DrawNText(SSurface *sf,int x, int y, U_INT rgba,char* text);
  int             DrawLimChar(SSurface *sf,int x, int y, int Nb, U_INT rgb, char* text) {return 0;}
  int             DrawNbrChar(SSurface *sf,int x, int y, U_INT rgb,char* text, int nb)  {return 0;}
  int             DrawTextV  (SSurface *surface,int x, int y, U_INT rgb, char* text, int dh)  {return 0;}
  void            DrawTextC  (SSurface *sf,int x, int y, U_INT rgb, char* text)           {}
  void            DrawTextR (SSurface *sf,int x, int y, U_INT rgb, char* text)            {}
  void            ReptChar  (SSurface *sf, int x0, int y0, int x1, U_INT rgba,char ascii) {}
  int             CharHeight (U_CHAR c)     {return ht;}
  int             CharWidth  (U_CHAR c);
  int             MaxCharHeight ()          {return ht;}
  int             MaxCharWidth ()           {return CharWidth('W');}
  int             TextWidth  (char* t);
  int             TextHeight(char*  t)      {return ht;}
};
//================================================================================
//  Draw texture on a surface
//================================================================================
void DrawTextureOn (TEXT_DEFN &txd,SSurface &sf,int x, int y, int No);
//================================================================================
// CMaskImage
//
// Implemented in Utility\RawImage.cpp
//================================================================================
class CMaskImage {
public:
  // Constructor/Destructor
  CMaskImage (int width, int height);
  CMaskImage (int width, int height, const GLubyte *data);
  CMaskImage (int width, int height, const char* rawFilename);
  CMaskImage (const CMaskImage &src);
 ~CMaskImage (void);
  void Invert();
  CMaskImage *Validate();
  //-------------------------------------------------------------
  inline  int  GetHeight(void)  {return height;}
  inline  int  GetWidth (void)  {return width;}
  //------------------------------------------------------------
  // void   MaskPoint (int x, int y);
  // void   MaskLine (int x1, int y1, int x2, int y2);
  // void   MaskPolygon (int nPoints, int* data);

public:
  int       width, height;
  int       dim;
  GLubyte   *rawdata;
};
//================================================================================
// CTexture
//    Support for a RGBA arry that may be used for all purposes
//    Implemented in Utility\RawImage.cpp
//================================================================================
class CTexture {
  //---- Attributes -----------------------------------------------
  C_2DRECT rect;                      // Texture dimension
  GLubyte *rgba;                      // Pixel array
  U_INT    xOBJ;                      // Texture object
  TC_VTAB  Pan[4];                    // Main screen panel
  //---------------------------------------------------------------
public:
   CTexture();
  ~CTexture() {if (rgba) delete [] rgba;}
  //-----Methods --------------------------------------------------
  void    LoadFromDisk(char *name,char tsp,int xs,int ys);
  void    LoadTerrain(char *name);
  void    SetQUAD();                  // Init quad to texture size
  void    DrawQUAD(int sw,int sh);    // Draw QUAD on screen
  void    SetDimension(C_2DRECT &r);
  void    Draw();
  //---------------------------------------------------------------
  inline GLuint   GetXOBJ()         {return xOBJ;}
  inline GLubyte *GetRGBA()         {return rgba;}
  inline int      GetWD()           {return rect.x1;}
  inline int      GetHT()           {return rect.y1;}
  inline void     FreeTexture()     {if (rgba) delete [] rgba; rgba = 0;}
};

//=============================================================================
// CRawImage
//
// Implemented in RawImage.cpp
//=============================================================================
class CRawImage {
public:
  // Constructor/Destructor
  CRawImage ();
  CRawImage (const char* rawFilename,
             const char* actFilename = NULL,
             const char* opaFilename = NULL);
  CRawImage (int width,
             int height,
             const char* rawFilename,
             const char* actFilename = NULL,
             const char* opaFilename = NULL);
  CRawImage (const CRawImage &src);
  ~CRawImage (void);

  // CRawImage methods
  void      Init (void);
  void      Allocate (UINT32 w, UINT32 h);
  void      Load (const char* rawFilename,
                  const char* actFilename,
                  const char* opaFilename = NULL);
  void      Load (PODFILE *raw = NULL, PODFILE *act = NULL, PODFILE *opa = NULL);
  int       GetWidth (void);
  int       GetHeight (void);
  UINT32    GetTextureMemorySize (void);
  void      SetName (const char* name);
  void      CreateTransferMaps (void);
  GLubyte*  GetRGBImageData (void);
  GLubyte*  GetRGBAImageData (void);

  void      Copy (const CRawImage &src);
  void      CopyMasked (const CRawImage *src, const CMaskImage *mask);
  void      Merge (const CRawImage *raw, const CMaskImage *mask, unsigned char actOffset);
  void      BindTexture (GLuint texid, bool mipmap);
  void      FreeTexture (void);

protected:
  char    name[64];
  int     width, height;
  GLubyte   *actdata;
  GLubyte   *rawdata;
  GLubyte   *opadata;
  GLuint    texid;
  bool      rawLoaded;
  bool      opaLoaded;
};
//============================================================================================
class CSunRawImage : public CRawImage {
public:
  CSunRawImage (int width,
              int height,
          const char* rawFilename,
          const char* actFilename);
};

//=========================================================================================
// CImageDXT
//
// Encapsulates an image in DXT (aka DDS, S3T) compressed texture format.
//=========================================================================================

class CImageDXT {
public:
  // Constructor/Destructor
  CImageDXT (const char* ddsFilename);
  ~CImageDXT (void);

  // CImageDXT methods
  UINT32  GetWidth (void) { return width; }
  UINT32  GetHeight (void) { return height; }
  UINT32  GetTextureMemorySize (void);
  void    BindTexture (GLuint texid);

protected:
  void    LoadDDS (const char* ddsFilename);

protected:
  GLenum    format;
  UINT8     mipCount;
  UINT32    width, height;
  GLubyte   *image;
};

//=========================================================================================
// CTextureManager
//
// Singleton class managing all OpenGL textures for the application.
// Implemented in TextureManager.cpp
//=========================================================================================
class CTextureInfo;

typedef enum {
  TEXTURE_SIZE_32       = 0,
  TEXTURE_SIZE_64,
  TEXTURE_SIZE_128,
  TEXTURE_SIZE_256,
  TEXTURE_SIZE_512,
  TEXTURE_SIZE_1024,
  TEXTURE_SIZE_OTHER,
  TEXTURE_SIZE_SENTINEL
} ETextureSize;

class CTextureManager {
public:
  // Constructors/Destructor
  CTextureManager (void);
  static CTextureManager instance;

  // CTextureManager methods
  static  CTextureManager& Instance (void) { return instance; }
  void    Init (void);
  void    Cleanup (void);
  GLuint  FindTexture (std::string key);
  GLuint  BindTexture (std::string key, CImageDXT *dxt);
  void    DiscardTexture (std::string key);

  void    print (FILE *f);
  void    Log (const char *fmt, ...);

protected:
  ETextureSize TextureSize (UINT32 w, UINT32 h);

protected:
  CLogFile                             *log;
  std::map<std::string, CTextureInfo*> mapTexture;

  // Metrics
  UINT32    nFindSuccess;
  UINT32    nFindFailure;
  UINT32    nBind;
  UINT32    tifCount[TEXTURE_SIZE_SENTINEL];
  UINT32    rawCount[TEXTURE_SIZE_SENTINEL];
  UINT32    dxtCount[TEXTURE_SIZE_SENTINEL];
};

//=========================================================================================
// Surface and Bitmap utilities
//
// Implemented in Utility/Bitmaps.cpp
//=========================================================================================
SSurface*     CreateSurface(int width, int height);
SSurface*     FreeSurface(SSurface *surface);
void          EraseSurfaceRGB (SSurface *surface, U_INT rgb);
void          EraseSurfaceRGBA(SSurface *surface, U_INT rgba);
void          EraseSurfaceRGBA(SSurface *surface, U_INT rgba,int mrg);
void          LeftShiftSurface(SSurface *sf,int n,int s,U_INT rgba);
void          DrawDot(SSurface *surface, int x, int y, unsigned int rgbColor);
void          DrawFastLine(SSurface *sf, int x1, int y1, int x2, int y2,U_INT rgba);
void          DrawImage(SSurface *sf, S_IMAGE &ref);
void          DrawRect(SSurface *surface, int x1, int y1, int x2, int y2, unsigned int rgbColor);
void          FillRect(SSurface *surface, int x1, int y1, int x2, int y2, unsigned int rgbColor);
void          DrawCircle(SSurface *surface, int xCenter, int yCenter, int radius, U_INT rgbColor);
void          DrawFastCircle(SSurface *sf, int Cx, int Cy, int radius, U_INT rgba);
void          DrawHLine(SSurface *sf,int x1,int x2,int y1,U_INT rgba);
unsigned int  MakeRGB(unsigned int r, unsigned int g, unsigned int b);
unsigned int  MakeRGBA(unsigned int r, unsigned int g, unsigned int b, unsigned int a);
void          UnmakeRGB (unsigned int rgb, unsigned int *r, unsigned int *g, unsigned int *b);
void          Blit (SSurface *surface);
void          BlitTransparent (SSurface *surface, unsigned int rgbTransparentColor);
GLuint        TextureFromSurface (SSurface *s, bool mipmap);

int           Load_Bitmap(SBitmap *bm);
void          DrawBitmap(SSurface *surface, SBitmap *bm, int x, int y, int frame);
void          DrawBitmapPartial (SSurface *surface, SBitmap *bm, int dx, int dy, int sx1, int sy1, int sx2, int sy2, int frame);
void          GetBitmapSize(SBitmap *bm, int *xSize, int *ySize);
int           NumBitmapFrames(SBitmap *bm);
void          FreeBitmap(SBitmap *bm);
CBitmap      *AssignNullBitmap();
//=========================================================================================
float         GetLefPos(TC_VTAB *qd);
float         GetTopPos(TC_VTAB *qd);
float         GetWIDTH(TC_VTAB *qd);
float         GetHEIGHT(TC_VTAB *qd);
//=========================================================================================
void ZRotate(GN_VTAB &v, double sn, double cn);
//=========================================================================================
void  BlitTransparentSurface (SSurface *surface, int xOffset, int yOffset, int y_isiz);
void  BlitOpaqueSurface (SSurface *surface, int xOffset, int yOffset, int y_isiz);
//=========================================================================================
// Font utilities
//
// Implemented in Utility/Fonts.cpp
//=====================================================================================
int   LoadFont (SFont *font);
int   LoadMonoFont(int inx,SFont *font);
int   LoadVariFont(int inx,SFont *font);
void  FreeFont (SFont *font);
void  FreeMonoFont(SFont *font);
void  FreeVariFont(SFont *font);
void  DrawTextList(SSurface *sf,int x0, int y0,TXT_LIST *sT,U_SHORT Nb);
//==============================================================================
// Data map for a polynomial relationship up to 9th order
//==============================================================================
class CSlotPolynomial: public CStreamObject {         // : public CDataLookup {
public:
  // Constructors / Destructor
  CSlotPolynomial (void);
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CDataLookup methods
  float Lookup (float x);
  void  Interpolate (CSlotPolynomial *p1,
                     CSlotPolynomial *p2,
                     float scale);

  // CSlotPolynomial methods
  void  SetCoefficient (int i, float f);

public:
//protected:
  char  n;          // Number of ceofficients
  float c[10];      // Polynomial coefficients
};

//==============================================================================
// Base table entry NNNNNNNNNNNNNNew
//  NOTE this is the root class for all data lookup table. It just supplies
//       virtual functions eand provides no storage
//      Real derived classes are
//  CFmt1Map  to support <Fmt1> format (and <tbl1>)
//  CFmt3Map  to support <Fmt3> format (and <tbl1>)
//  CFPolyMap to support <ply1> format
//  CFcoefMap to support <cof1> format
//  
///==============================================================================
class CMapSlot: public CStreamObject {
  //--------------------------------------------
public:
  virtual void Clear()    {}
};

//==============================================================================
// Format 1 CMapEntry tuple (x,y)
//==============================================================================
class CFmt1Slot: public CMapSlot {
public:
  CFmt1Slot();
  CFmt1Slot(float x,float y);
 ~CFmt1Slot();
  //---ATTRIBUTES ------------------------------------
public:
  float x, y, dv;
};
//==============================================================================
// Format: Tuple (x,u,v,w)
//  1 input and 3 outputs
//==============================================================================
class C3valSlot: public CMapSlot {
  friend class CmvalMap;
  //---ATTRIBUTES ------------------------------------
protected:
  float x;
  float u;
  float v;
  float w;
  float du;
  float dv;
  float dw;
  //--------------------------------------------------
public:
  C3valSlot(float x, float u, float v, float w);
  inline float GetX() {return x;}
  inline float GetU() {return u;}
  inline float GetV() {return v;}
  inline float GetW() {return w;}
};

//==============================================================================
// Format 3 CMapEntry tuple (polynomial)
//==============================================================================
class CFmt3Slot: public CMapSlot {
public:
  CFmt3Slot();
 ~CFmt3Slot();
  void ReadCoef(SStream *s) {ReadFrom(&poly,s);}
public:
  float               x;
  CSlotPolynomial poly;
};

//==============================================================================
// Abstract class for a table
///==============================================================================
class CFmtxMap : public CStreamObject {
protected:
	short   type;									// Type of map
  short   users;                // gauge count
  //---Only methods ------------------------------------------
public:
  CFmtxMap::CFmtxMap() {type = 'ZR', users = 0;}
  virtual float Lookup(float x) {return x;}
  virtual void  Add(float x,float y)  {}
  //-----------------------------------------------------------
  void  IncUse()  {users++;}
  void  DecUse()  {users--; if (0 == users) delete this;}
};
//==============================================================================
// tbl1 format
///==============================================================================
class CTbl1Map: public CStreamObject {
  //---Attribute ---------------------------------------------
  CFmtxMap *tab;
  //----------------------------------------------------------
public:
  CTbl1Map(SStream *s);
  int Read(SStream *s,Tag tag);
  //----------------------------------------------------------
  inline CFmtxMap *GetMap() {return tab;}
};
//==============================================================================
// Data map for a table in fmt1
///==============================================================================
class CFmt1Map : public CFmtxMap {
protected:
  //---ATTRIBUTES ------------------------------------------------------
  std::vector<CFmt1Slot>        tab;
  //---METHODS ---------------------------------------------------------
public:
  CFmt1Map();
 ~CFmt1Map();
 //---------------------------------------------------------------------
 void  Load(TUPPLE *t);
 void	 Load(PAIRF  *p);
 void  DecodeFMT1(SStream *s);
 void  Enter(CFmt1Slot &e);
 void  Add(float x,float y);
 float Lookup(float x);
};
//==============================================================================
// Data map for a table in fmt1
///==============================================================================
class CCoefMap : public CFmtxMap {
protected:
  //---ATTRIBUTES ------------------------------------------------------
  float a;
  //---METHODS ---------------------------------------------------------
public:
  CCoefMap(SStream *s);
 ~CCoefMap() {};
 //---------------------------------------------------------------------
 void  SetCoef(float c) {a = c;}
 float Lookup(float x)  {return (a * x); }
};
//==============================================================================
// Data map for a table in ply1
///==============================================================================
class CPolyMap : public CFmtxMap {
protected:
  //----ATTRIBUTE is a polynomial ---------------------------------------
  CSlotPolynomial poly;
  //---------------------------------------------------------------------
public:
  CPolyMap(SStream *s);
 ~CPolyMap() {}
 //----------------------------------------------------------------------
 float Lookup(float x) {return poly.Lookup(x);}
};
//==============================================================================
// Data map for a table in fmt3
///==============================================================================
class CFmt3Map : public CFmtxMap {
protected:
  //---ATTRIBUTES ------------------------------------------------------
  std::vector<CFmt3Slot>        tab;
  //---METHODS ---------------------------------------------------------
public:
  CFmt3Map();
 ~CFmt3Map();
 //---------------------------------------------------------------------
 void  DecodeFMT3(SStream *s);
 void  Enter(CFmt3Slot &e);
 float Lookup(float x);
};
//==============================================================================
// Data map for a table with  multiple output Values
///==============================================================================
class CmvalMap: public CFmtxMap {
protected:
  //---ATTRIBUTES ------------------------------------------------------
  std::vector<C3valSlot> tab;
  C3valSlot *ps;                      // Previous slot
    //---METHODS ---------------------------------------------------------
public:
  CmvalMap();
 ~CmvalMap();
 //----------------------------------------------------------------------
  void  Enter(C3valSlot &e);
  void  Load(TUPPLE3 *t);
  void  Lookup(TUPPLE3 &tp);
  void  Fill(TUPPLE3 &in,C3valSlot &s);
  //---------------------------------------------------------------------
  void  GetPrevX(U_INT k, float &x);
  void  GetNextX(U_INT k, float &x);
  void  Recompute(U_INT k);
  void  ChangeX(U_INT k, float x);
  void  ChangeU(U_INT k, float u);
  void  ChangeV(U_INT k, float v);
  void  ChangeW(U_INT k, float w);
  //---------------------------------------------------------------------
  C3valSlot *Getfloor(float x);
  //---------------------------------------------------------------------
  C3valSlot *GetSlot(U_INT k)   {return (k >= tab.size())?(0):(&tab[k]);}
  //---------------------------------------------------------------------
};
//==============================================================================
// Data map for a table lookup in various formats
//==============================================================================
class CDataSearch: public CStreamObject  {
private:
  //---ATTRIBUTE is THE TABLE POINTER ------------------------
  CFmtxMap *table;
protected:
  int   Read (SStream *stream, Tag tag);
  //----------------------------------------------------------
public:
  // Constructors
  CDataSearch(SStream *s);
  //----------------------------------------------------------
  inline CFmtxMap*  GetTable()  {return table;}
};

//========================================================================
// Left-hand to right-hand coordinate system conversion utilities
//
// Original Fly! data use a left-handed coordinate system oriented as:
//  - x-axis points towards right
//  - y-axis points up
//  - z-axis points forward
//
// Fly! Legacy uses a right-handed coordinate system oriented as:
//  - x-axis points towards right
//  - y-axis points forward
//  - z-axis points up
//==========================================================================
void VectorDistanceLeftToRight  (SVector &v);
void VectorOrientLeftToRight (SVector &v);

/*! \fn     TurnPointFromFulcrum
 *  \brief  rotates a distant point an angle
 *  \param  x origin
 *  \param  y origin
 *  \param  angle in rads
 *  \param  x result
 *  \param  y result
 */
void TurnPointFromFulcrum ( const float &a_in,
                            const float &b_in,
                            const float &angle_rad,
                            float &a_out,
                            float &b_out);

/*! \fn     TurnVectorFromFulcrum
 *  \brief
 *  \param  in SVector origin
 *  \param  orientation SVector in rads
 *  \param  out  result
 */
void TurnVectorFromFulcrum (const SVector &in, const SVector &ori, SVector &out);

/*! \fn     BodyVector2WorldPos
 *  \brief  transforms body vehicle SPosition in the world overall SPosition
 *  \param  body SVector origin
 *  \param  world SPosition result
 */
void BodyVector2WorldPos (const SPosition &cgPos, const SVector &body, SPosition &world);
//==============================================================================
// Global utilities
//==============================================================================
EMessageResult Send_Message (SMessage *msg);         // Messaging.cpp
//==============================================================================
//  Supply srandard routines
//==============================================================================
#ifndef HAVE_STRUPPER
#ifdef HAVE__STRUPR
// _strupr() is available; make a macro alias to strupper()
#define strupper _strupr
#else
// Neither strupper() nor _strupr() are available on this system; provide an alternate
// Implemented in Portability.cpp
char *strupper (char *s);
#endif // HAVE__STRUPR
#endif // HAVE_STRUPPER

#ifndef HAVE_STRREV
// strrev() is not available on this system; provide an alternate implementation
// Implemented in Portability.cpp
#define _strrev strrev
char *strrev(char *str);
#endif // HAVE_STRREV

#ifndef HAVE_ITOA
// itoa() is not available on this system; provide an alternate implementation
// Implemented in Portability.cpp
#define _itoa itoa
char* itoa(int value, char*  str, int radix);
#endif // HAVE_ITOA

#endif // UTILITY_H


