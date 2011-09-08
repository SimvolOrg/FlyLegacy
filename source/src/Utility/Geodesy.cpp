/*
 * Geodesy.cpp
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
 *
 */

/*! \file Geodesy.cpp
 *  \brief Implements utility functions related to geodesic systems
 *
 * These routines transform geocentric coordinates to geodetic coordinates
 *   using the WGS84 coordinate system.
 *
 * The code was taken from SimGear 0.3.3 module "sg_geodesy.cxx".
 * Minor cosmetic changes made for Fly! Legacy project by Chris Wallace.
 *
 * See end of file for original comments
 */

#include "../Include/FlyLegacy.h"
#include "../Include/Utility.h"
#include <plib/sg.h>


//
// Test function for debugging Geodesy.cpp functions
//
void test_geodesy (void)
{
  SPosition geod;
  geod.lat = 45 * 3600;
  geod.lon = 135 * 3600;
  geod.alt = 5000;

  SPosition geoc = GeodToGeoc (geod);
  geod = GeocToGeod (geoc);

  SVector cart;
  cart = GeocToCartesian (geoc);
  geoc = CartesianToGeoc (cart);
  geod = GeocToGeod (geoc);

  char s[80];
  FormatPosition (geod, s);
}


//
// By convention:
//
//  SPosition structs representing locations in geodetic coordinate system:
//    lat = arcseconds of latitude N (+) or S (-) of the equator
//    lon = arcseconds of longitude E of the Prime Meridian
//    alt = altitude in feet above MSL
//
//  SPosition structs representing locations in geocentric coordinate system
//    lat = radians of latitude N (+) or S (-) of the equator
//    lon = radians of longitude E of the Prime Meridian
//    alt = distance in feet from center of the Earth
//

// Please : note that the common used #define lay in the Utility.h header
//#define PI_2            (SGD_PI * 2)
//#define ARCSEC_TO_RAD       (PI_2 / ARCSEC_360DEG)
//#define ONE_ARCSEC          (ARCSEC_TO_RAD)
//#define EQUATORIAL_RADIUS_FEET    (3963.19 * 5280)
//#define EQ_RAD_SQUARE_M       (EQUATORIAL_RADIUS_M * EQUATORIAL_RADIUS_M)

// Value of earth flattening parameter from ref [8] 
//
//      Note: FP = f
//            E  = 1-f
//            EPS = sqrt(1-(1-f)^2)
//

#define GEO_FP    (double(0.003352813178))
#define GEO_E     (double(0.996647186))
#define GEO_EPS   (double(0.081819221))
#define GEO_INVG  (double(0.031080997))



//
// Convert an SPosition from geocentric to (WGS84) geodetic systems
//
SPosition GeocToGeod (SPosition pos)
{
  // Input SPosition is in geocentric coordinates just need to convert
  //   altitude from feet to meters for compatibility with the algorithm
  double lat_geoc = pos.lat;
  double radius = FN_METRE_FROM_FEET (pos.alt);

  // Define output parameters for algorithm
  double lat_geod, alt, sea_level_r;

  //
  // Conversion algorithm taken from SimGear 0.3.3 by sg_geodesy.cxx
  //
    double t_lat, x_alpha, mu_alpha, delt_mu, r_alpha, l_point, rho_alpha;
    double sin_mu_a, denom,delt_lambda, lambda_sl, sin_lambda_sl;

    if( ( (TWO_PI - lat_geoc) < ONE_ARCSEC )        // near North pole
  || ( (TWO_PI + lat_geoc) < ONE_ARCSEC ) )   // near South pole
    {
    lat_geod = lat_geoc;
    sea_level_r = EQUATORIAL_RADIUS_M*GEO_E;
    alt = radius - sea_level_r;
    } else {
    t_lat = tan(lat_geoc);
    x_alpha = GEO_E*EQUATORIAL_RADIUS_M/sqrt(t_lat*t_lat + GEO_E*GEO_E);

    double tmp = sqrt(EQ_RAD_SQUARE_M - x_alpha * x_alpha);
    if ( tmp < 0.0 ) { tmp = 0.0; }

    mu_alpha = atan2(tmp,GEO_E*x_alpha);
    if (lat_geoc < 0) mu_alpha = - mu_alpha;
    sin_mu_a = sin(mu_alpha);
    delt_lambda = mu_alpha - lat_geoc;
    r_alpha = x_alpha/cos(lat_geoc);
    l_point = radius - r_alpha;
    alt = l_point*cos(delt_lambda);

    denom = sqrt(1-GEO_EPS*GEO_EPS*sin_mu_a*sin_mu_a);

    rho_alpha = EQUATORIAL_RADIUS_M*(1-GEO_EPS)/(denom*denom*denom);
    delt_mu = atan2(l_point*sin(delt_lambda),rho_alpha + alt);
    lat_geod = mu_alpha - delt_mu;
    lambda_sl = atan( GEO_E*GEO_E * tan(lat_geod) ); // SL geoc. latitude
    sin_lambda_sl = sin( lambda_sl );
    sea_level_r = sqrt(EQ_RAD_SQUARE_M / (1 + ((1/(GEO_E*GEO_E))-1)*sin_lambda_sl*sin_lambda_sl));
    }

  // Prepare return value
  SPosition result;
  result.lat = lat_geod / ARCSEC_TO_RAD;
  result.lon = pos.lon  / ARCSEC_TO_RAD;
  result.alt = MetresToFeet (alt);

  return result;
}

//
// Convert an SPosition from WGS84 geodetic to geocentric systems
//
// Inputs:
//  pos     Geodetic position, lat/lon in arcsec, alt in feet ASL
//
// Returns:
//  SPosition Geocentric position, lat/lon in radians, alt in feet from earth centre
//
SPosition GeodToGeoc (SPosition pos)
{
  // Prepare arguments for the algorithm
  double lat_geod = pos.lat * ARCSEC_TO_RAD;
  double alt = FN_METRE_FROM_FEET (pos.alt);

  // Declare return values from algorithm
  double sl_radius, lat_geoc;

  //
  // Conversion algorithm taken from SimGear 0.3.3  sg_geodesy.cxx
  //
  double lambda_sl, sin_lambda_sl, cos_lambda_sl, sin_mu, cos_mu, px, py;
    
  lambda_sl = atan( GEO_E*GEO_E * tan(lat_geod) ); // sea level geocentric latitude
  sin_lambda_sl = sin( lambda_sl );
  cos_lambda_sl = cos( lambda_sl );
  sin_mu = sin(lat_geod);                  // Geodetic (map makers') latitude
  cos_mu = cos(lat_geod);
  sl_radius = 
    sqrt(EQ_RAD_SQUARE_M / (1 + ((1/(GEO_E*GEO_E))-1)*sin_lambda_sl*sin_lambda_sl));

  py = sl_radius * sin_lambda_sl + alt * sin_mu;
  px = sl_radius * cos_lambda_sl + alt * cos_mu;
  lat_geoc = atan2( py, px );

  // Prepare return value
  SPosition result;
  result.lat = lat_geoc;
  result.lon = pos.lon * ARCSEC_TO_RAD;
  result.alt = pos.alt + MetresToFeet (sl_radius);

  return result;
}


//
// Convert a geocentric SPosition (lat,lon,alt) to cartesian coordinates
//   in an SVector struct
//
SVector GeocToCartesian (SPosition pos)
{
  // Convert to cartesian coordinates
  SVector v;
  v.x = cos (pos.lon) * cos (pos.lat) * pos.alt;
  v.y = sin (pos.lon) * cos (pos.lat) * pos.alt;
  v.z = sin (pos.lat) * pos.alt;
  return v;
}

//
// Convert a geocentric SPosition (lat,lon,alt) to cartesian coordinates
//   in an sgdVec3 struct
//
void GeocToCartesianSgdVec3 (SPosition pos, sgdVec3 *v)
{
  // Convert to cartesian coordinates
  double x = cos (pos.lon) * cos (pos.lat) * pos.alt;
  double y = sin (pos.lon) * cos (pos.lat) * pos.alt;
  double z = sin (pos.lat) * pos.alt;

  sgdSetVec3 (*v, x, y, z);
}

//
// Convert a cartesian SVector (x,y,z) to geocentric SPosition (lat,lon,alt)
//
SPosition CartesianToGeoc (SVector v)
{
  SPosition result;

  result.lon = atan2 (v.y, v.x);
  result.lat = PI/2 - atan2 (sqrt(v.x*v.x + v.y*v.y), v.z);
  result.alt = sqrt (v.x*v.x + v.y*v.y + v.z*v.z);

  return result;
}


//
// One-stop conversion of a geodetic position to cartesian coordinates
//
// In the "flat earth" model, 
SVector GeodToCartesian (SPosition pos)
{
  SVector v;

  // First convert to geocentric position
  SPosition geoc = GeodToGeoc (pos);

  // Then convert to cartesian coordinates
  v = GeocToCartesian (geoc);

  return v;
}


//
// This algorithm was taken from SimGear 0.3.3 module "sg_geodesy.cxx",
// originally based on:
//
// Proceedings of the 7th International Symposium on Geodetic
// Computations, 1985
//
// "The Nested Coefficient Method for Accurate Solutions of Direct and
// Inverse Geodetic Problems With Any Length"
//
// Zhang Xue-Lian
// pp 747-763
//
// Modified for FlightGear to use WGS84 only -- Norman Vine
//

static inline double M0( double e2 ) {
    return SGD_PI*(1.0 - e2*( 1.0/4.0 + e2*( 3.0/64.0 + 
              e2*(5.0/256.0) )))/2.0;
}

static int geo_direct_wgs_84 (const double& alt, const double& lat1,
                const double& lon1, const double& az1,
                const double& s, double *lat2, double *lon2,
                double *az2 )
{
  double a = 6378137.000, rf = 298.257223563;
  double RADDEG = (SGD_PI)/180.0, testv = 1.0E-10;
  double f = ( rf > 0.0 ? 1.0/rf : 0.0 );
  double b = a*(1.0-f);
  double e2 = f*(2.0-f);
  double phi1 = lat1*RADDEG, lam1 = lon1*RADDEG;
  double sinphi1 = sin(phi1), cosphi1 = cos(phi1);
  double azm1 = az1*RADDEG;
  double sinaz1 = sin(azm1), cosaz1 = cos(azm1);
  
  if( fabs(s) < 0.01 ) {  // distance < centimeter => congruency
    *lat2 = lat1;
    *lon2 = lon1;
    *az2 = 180.0 + az1;
    if( *az2 > 360.0 ) *az2 -= 360.0;

    return 0;

  } else if( cosphi1 ) {  // non-polar origin
    // u1 is reduced latitude
    double tanu1 = sqrt(1.0-e2)*sinphi1/cosphi1;
    double sig1 = atan2(tanu1,cosaz1);
    double cosu1 = 1.0/sqrt( 1.0 + tanu1*tanu1 ), sinu1 = tanu1*cosu1;
    double sinaz =  cosu1*sinaz1, cos2saz = 1.0-sinaz*sinaz;
    double us = cos2saz*e2/(1.0-e2);

    // Terms
    double  ta = 1.0+us*(4096.0+us*(-768.0+us*(320.0-175.0*us)))/16384.0,
    tb = us*(256.0+us*(-128.0+us*(74.0-47.0*us)))/1024.0,
    tc = 0;

    // FIRST ESTIMATE OF SIGMA (SIG)
    double first = s/(b*ta);  // !!
    double sig = first;
    double c2sigm, sinsig,cossig, temp,denom,rnumer, dlams, dlam;
    do {
      c2sigm = cos(2.0*sig1+sig);
      sinsig = sin(sig); cossig = cos(sig);
      temp = sig;
      sig = first +
        tb*sinsig*(c2sigm+tb*(cossig*(-1.0+2.0*c2sigm*c2sigm) - 
        tb*c2sigm*(-3.0+4.0*sinsig*sinsig)
        * (-3.0+4.0*c2sigm*c2sigm)/6.0)
        / 4.0);
    } while( fabs(sig-temp) > testv);

    // LATITUDE OF POINT 2
    // DENOMINATOR IN 2 PARTS (TEMP ALSO USED LATER)
    temp = sinu1*sinsig-cosu1*cossig*cosaz1;
    denom = (1.0-f)*sqrt(sinaz*sinaz+temp*temp);

    // NUMERATOR
    rnumer = sinu1*cossig+cosu1*sinsig*cosaz1;
    *lat2 = atan2(rnumer,denom)/RADDEG;

    // DIFFERENCE IN LONGITUDE ON AUXILARY SPHERE (DLAMS )
    rnumer = sinsig*sinaz1;
    denom = cosu1*cossig-sinu1*sinsig*cosaz1;
    dlams = atan2(rnumer,denom);

    // TERM C
    tc = f*cos2saz*(4.0+f*(4.0-3.0*cos2saz))/16.0;

    // DIFFERENCE IN LONGITUDE
    dlam = dlams-(1.0-tc)*f*sinaz*(sig+tc*sinsig* (c2sigm+tc*cossig*(-1.0+2.0*c2sigm*c2sigm)));
    *lon2 = (lam1+dlam)/RADDEG;
    if (*lon2 > 180.0  ) *lon2 -= 360.0;
    if (*lon2 < -180.0 ) *lon2 += 360.0;

    // AZIMUTH - FROM NORTH
    *az2 = atan2(-sinaz,temp)/RADDEG;
    if ( fabs(*az2) < testv ) *az2 = 0.0;
    if ( *az2 < 0.0) *az2 += 360.0;

    return 0;
  } else {      // phi1 == 90 degrees, polar origin
    double dM = a*M0(e2) - s;
    double paz = ( phi1 < 0.0 ? 180.0 : 0.0 );
    double zero = 0.0f;
    return geo_direct_wgs_84( alt, zero, lon1, paz, dM, lat2, lon2, az2 );
    } 
}


SPosition GreatCirclePosition(SPosition *from, SVector *polar)
{
  // The algorithm expects the following input arguments:
  //  double  alt     Starting altitude in meters
  //  double  lat1    Starting latitude in degrees
  //  double  lon1    Starting longitude in degrees
  //  double  az1     Starting azimuth in degrees
  //  double  s     Distance in meters
  //  double  lat2    Destination latitude in degrees
  //  double  lon2    Destination longitude in degrees
  //  double  az2     Destination azimuth in degrees
  //
  double alt = FN_METRE_FROM_FEET (from->alt);
  double lat1 = from->lat / 3600;
  double lon1 = from->lon / 3600;
  double az1 = polar->h;
  double s = polar->r;
  double lat2, lon2, az2;

  geo_direct_wgs_84 (alt, lat1, lon1, az1, s, &lat2, &lon2, &az2);

  // Convert output parameters back into SPosition
  SPosition rc;
  rc.lat = lat2 * 3600;
  rc.lon = lon2 * 3600;
  rc.alt = from->alt;

  return rc;
}


static int geo_inverse_wgs_84(const double& alt, const double& lat1,
                const double& lon1, const double& lat2,
                const double& lon2, double *az1, double *az2,
                double *s )
{
    double a = 6378137.000, rf = 298.257223563;
    int iter=0;
    double RADDEG = (SGD_PI)/180.0, testv = 1.0E-10;
    double f = ( rf > 0.0 ? 1.0/rf : 0.0 );
    double b = a*(1.0-f);
    // double e2 = f*(2.0-f); // unused in this routine
    double phi1 = lat1*RADDEG, lam1 = lon1*RADDEG;
    double sinphi1 = sin(phi1), cosphi1 = cos(phi1);
    double phi2 = lat2*RADDEG, lam2 = lon2*RADDEG;
    double sinphi2 = sin(phi2), cosphi2 = cos(phi2);
  
    if( (fabs(lat1-lat2) < testv && 
   ( fabs(lon1-lon2) < testv) || fabs(lat1-90.0) < testv ) )
    { 
  // TWO STATIONS ARE IDENTICAL : SET DISTANCE & AZIMUTHS TO ZERO */
  *az1 = 0.0; *az2 = 0.0; *s = 0.0;
  return 0;
    } else if(  fabs(cosphi1) < testv ) {
  // initial point is polar
  int k = geo_inverse_wgs_84( alt, lat2,lon2,lat1,lon1, az1,az2,s );
  k = k; // avoid compiler error since return result is unused
  b = *az1; *az1 = *az2; *az2 = b;
  return 0;
    } else if( fabs(cosphi2) < testv ) {
  // terminal point is polar
        double _lon1 = lon1 + 180.0f;
  int k = geo_inverse_wgs_84( alt, lat1, lon1, lat1, _lon1, 
            az1, az2, s );
  k = k; // avoid compiler error since return result is unused
  *s /= 2.0;
  *az2 = *az1 + 180.0;
  if( *az2 > 360.0 ) *az2 -= 360.0; 
  return 0;
    } else if( (fabs( fabs(lon1-lon2) - 180 ) < testv) && 
         (fabs(lat1+lat2) < testv) ) 
    {
  // Geodesic passes through the pole (antipodal)
  double s1,s2;
  geo_inverse_wgs_84( alt, lat1,lon1, lat1,lon2, az1,az2, &s1 );
  geo_inverse_wgs_84( alt, lat2,lon2, lat1,lon2, az1,az2, &s2 );
  *az2 = *az1;
  *s = s1 + s2;
  return 0;
    } else {
  // antipodal and polar points don't get here
  double dlam = lam2 - lam1, dlams = dlam;
  double sdlams,cdlams, sig,sinsig,cossig, sinaz,
      cos2saz, c2sigm;
  double tc,temp, us,rnumer,denom, ta,tb;
  double cosu1,sinu1, sinu2,cosu2;

  // Reduced latitudes
  temp = (1.0-f)*sinphi1/cosphi1;
  cosu1 = 1.0/sqrt(1.0+temp*temp);
  sinu1 = temp*cosu1;
  temp = (1.0-f)*sinphi2/cosphi2;
  cosu2 = 1.0/sqrt(1.0+temp*temp);
  sinu2 = temp*cosu2;
    
  do {
      sdlams = sin(dlams), cdlams = cos(dlams);
      sinsig = sqrt(cosu2*cosu2*sdlams*sdlams+
        (cosu1*sinu2-sinu1*cosu2*cdlams)*
        (cosu1*sinu2-sinu1*cosu2*cdlams));
      cossig = sinu1*sinu2+cosu1*cosu2*cdlams;
      
      sig = atan2(sinsig,cossig);
      sinaz = cosu1*cosu2*sdlams/sinsig;
      cos2saz = 1.0-sinaz*sinaz;
      c2sigm = (sinu1 == 0.0 || sinu2 == 0.0 ? cossig : 
          cossig-2.0*sinu1*sinu2/cos2saz);
      tc = f*cos2saz*(4.0+f*(4.0-3.0*cos2saz))/16.0;
      temp = dlams;
      dlams = dlam+(1.0-tc)*f*sinaz*
    (sig+tc*sinsig*
     (c2sigm+tc*cossig*(-1.0+2.0*c2sigm*c2sigm)));
      if (fabs(dlams) > SGD_PI && iter++ > 50) {
    return iter;
      }
  } while ( fabs(temp-dlams) > testv);

  us = cos2saz*(a*a-b*b)/(b*b); // !!
  // BACK AZIMUTH FROM NORTH
  rnumer = -(cosu1*sdlams);
  denom = sinu1*cosu2-cosu1*sinu2*cdlams;
  *az2 = atan2(rnumer,denom)/RADDEG;
  if( fabs(*az2) < testv ) *az2 = 0.0;
  if(*az2 < 0.0) *az2 += 360.0;

  // FORWARD AZIMUTH FROM NORTH
  rnumer = cosu2*sdlams;
  denom = cosu1*sinu2-sinu1*cosu2*cdlams;
  *az1 = atan2(rnumer,denom)/RADDEG;
  if( fabs(*az1) < testv ) *az1 = 0.0;
  if(*az1 < 0.0) *az1 += 360.0;

  // Terms a & b
  ta = 1.0+us*(4096.0+us*(-768.0+us*(320.0-175.0*us)))/
      16384.0;
  tb = us*(256.0+us*(-128.0+us*(74.0-47.0*us)))/1024.0;

  // GEODETIC DISTANCE
  *s = b*ta*(sig-tb*sinsig*
       (c2sigm+tb*(cossig*(-1.0+2.0*c2sigm*c2sigm)-tb*
             c2sigm*(-3.0+4.0*sinsig*sinsig)*
             (-3.0+4.0*c2sigm*c2sigm)/6.0)/
        4.0));
  return 0;
    }
}


SVector GreatCirclePolar(SPosition *from, SPosition *to)
{
  double alt = FN_METRE_FROM_FEET (from->alt);
  double lat1 = from->lat / 3600;
  double lon1 = from->lon / 3600;
  double lat2 = to->lat / 3600;
  double lon2 = to->lon / 3600;
  double az1, az2, s;

  geo_inverse_wgs_84(alt, lat1, lon1, lat2, lon2, &az1, &az2, &s);
  
  // Convert results to SVector return code
  SVector v;
  v.h = az1;
  v.p = 0;
  v.r = MetresToFeet (s);

  return v;
}



/*

//
// geoc_to_geod (lat_geoc, radius, *lat_geod, *Alt, *seal_level_r)
//
// Convert geodetic (WGS84) coordinates to geocentric coordinate system
//
//     INPUTS:  
//         lat_geoc Geocentric latitude, radians, + = North
//         radius C.G. radius to earth center (meters)
//
//     OUTPUTS:
//         lat_geod Geodetic latitude, radians, + = North
//         alt    C.G. altitude above mean sea level (meters)
//         sea_level_r  radius from earth center to sea level at
//                      local vertical (surface normal) of C.G. (meters)

void geoc_to_geod( const double& lat_geoc, const double& radius,
                   double *lat_geod, double *alt, double *sea_level_r )
{
    double t_lat, x_alpha, mu_alpha, delt_mu, r_alpha, l_point, rho_alpha;
    double sin_mu_a, denom,delt_lambda, lambda_sl, sin_lambda_sl;

    if( ( (SGD_PI_2 - lat_geoc) < SG_ONE_SECOND )        // near North pole
  || ( (SGD_PI_2 + lat_geoc) < SG_ONE_SECOND ) )   // near South pole
    {
    *lat_geod = lat_geoc;
    *sea_level_r = SG_EQUATORIAL_RADIUS_M*E;
    *alt = radius - *sea_level_r;
    } else {
    // cout << "  lat_geoc = " << lat_geoc << endl;
    t_lat = tan(lat_geoc);
    // cout << "  tan(t_lat) = " << t_lat << endl;
    x_alpha = E*SG_EQUATORIAL_RADIUS_M/sqrt(t_lat*t_lat + E*E);
    // cout << "  x_alpha = " << x_alpha << endl;
    double tmp = sqrt(SG_EQ_RAD_SQUARE_M - x_alpha * x_alpha);
    if ( tmp < 0.0 ) { tmp = 0.0; }
    mu_alpha = atan2(tmp,E*x_alpha);
    if (lat_geoc < 0) mu_alpha = - mu_alpha;
    sin_mu_a = sin(mu_alpha);
    delt_lambda = mu_alpha - lat_geoc;
    r_alpha = x_alpha/cos(lat_geoc);
    l_point = radius - r_alpha;
    *alt = l_point*cos(delt_lambda);

    denom = sqrt(1-EPS*EPS*sin_mu_a*sin_mu_a);
    rho_alpha = SG_EQUATORIAL_RADIUS_M*(1-EPS)/
      (denom*denom*denom);
    delt_mu = atan2(l_point*sin(delt_lambda),rho_alpha + *alt);
    *lat_geod = mu_alpha - delt_mu;
    lambda_sl = atan( E*E * tan(*lat_geod) ); // SL geoc. latitude
    sin_lambda_sl = sin( lambda_sl );
    *sea_level_r = 
      sqrt(SG_EQ_RAD_SQUARE_M / (1 + ((1/(E*E))-1)*sin_lambda_sl*sin_lambda_sl));
    }
}


// geod_to_geoc( lat_geod, alt, *sl_radius, *lat_geoc )
//
// Convert geodetic (WGS84) coordinates to geocentric coordinate system
//
//     INPUTS:  
//         lat_geod Geodetic latitude, radians, + = North
//         alt    C.G. altitude above mean sea level (meters)
//
//     OUTPUTS:
//         sl_radius  SEA LEVEL radius to earth center (meters)
//                      (add Altitude to get true distance from earth center.
//         lat_geoc Geocentric latitude, radians, + = North
//


void geod_to_geoc ( const double& lat_geod, const double& alt, double *sl_radius,
          double *lat_geoc )
{
    double lambda_sl, sin_lambda_sl, cos_lambda_sl, sin_mu, cos_mu, px, py;
    
    lambda_sl = atan( E*E * tan(lat_geod) ); // sea level geocentric latitude
    sin_lambda_sl = sin( lambda_sl );
    cos_lambda_sl = cos( lambda_sl );
    sin_mu = sin(lat_geod);                  // Geodetic (map makers') latitude
    cos_mu = cos(lat_geod);
    *sl_radius = 
  sqrt(SG_EQ_RAD_SQUARE_M / (1 + ((1/(E*E))-1)*sin_lambda_sl*sin_lambda_sl));
    py = *sl_radius*sin_lambda_sl + alt*sin_mu;
    px = *sl_radius*cos_lambda_sl + alt*cos_mu;
    *lat_geoc = atan2( py, px );
}

*/


/***************************************************************************

  TITLE:  ls_geodesy
  
----------------------------------------------------------------------------

  FUNCTION: Converts geocentric coordinates to geodetic positions

----------------------------------------------------------------------------

  MODULE STATUS:  developmental

----------------------------------------------------------------------------

  GENEALOGY:  Written as part of LaRCSim project by E. B. Jackson

----------------------------------------------------------------------------

  DESIGNED BY:  E. B. Jackson
  
  CODED BY: E. B. Jackson
  
  MAINTAINED BY:  E. B. Jackson

----------------------------------------------------------------------------

  MODIFICATION HISTORY:
  
  DATE  PURPOSE           BY
  
  930208  Modified to avoid singularity near polar region.  EBJ
  930602  Moved backwards calcs here from ls_step.    EBJ
  931214  Changed erroneous Latitude and Altitude variables to 
    *lat_geod and *alt in routine ls_geoc_to_geod.    EBJ
  940111  Changed header files from old ls_eom.h style to ls_types, 
    and ls_constants.  Also replaced old DATA type with new
    SCALAR type.            EBJ

----------------------------------------------------------------------------

  REFERENCES:

    [ 1]  Stevens, Brian L.; and Lewis, Frank L.: "Aircraft 
      Control and Simulation", Wiley and Sons, 1992.
      ISBN 0-471-61397-5          


----------------------------------------------------------------------------

  CALLED BY:  ls_aux

----------------------------------------------------------------------------

  CALLS TO:

----------------------------------------------------------------------------

  INPUTS: 
    lat_geoc  Geocentric latitude, radians, + = North
    radius    C.G. radius to earth center, ft

----------------------------------------------------------------------------

  OUTPUTS:
    lat_geod  Geodetic latitude, radians, + = North
    alt   C.G. altitude above mean sea level, ft
    sea_level_r radius from earth center to sea level at
        local vertical (surface normal) of C.G.

--------------------------------------------------------------------------*/


