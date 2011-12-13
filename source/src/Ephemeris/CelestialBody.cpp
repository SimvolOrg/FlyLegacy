/*
 * CelestialBody.cpp
 *
 * Part of Fly! Legacy project
 *
 * This code was originally taken from SimGear 0.3.3; see
 *   the end of this file for the original SimGear comments.
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
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

/*! \file CelestialBody.cpp
 *  \brief Implements CCelestialBody base class for all ephemeris calculators
 *
 *  Implementation of CCelestialBody class which forms the base for ephemeris
 *    calculation classes for all solar system bodies (i.e. Sol, moon, planets).
 */

#include <math.h>
#include "../Include/Ephemeris.h"
#include "../Include/Utility.h"


/**************************************************************************
 * void CCelestialBody::updatePosition(double mjd, CSol *ourSun)
 *
 * Basically, this member function provides a general interface for 
 * calculating the right ascension and declinaion. This function is 
 * used for calculating the planetary positions. For the planets, an 
 * overloaded member function is provided to additionally calculate the
 * planet's magnitude. 
 * The sun and moon have their own overloaded updatePosition member, as their
 * position is calculated an a slightly different manner.  
 *
 * arguments:
 * double mjd: provides the modified julian date.
 * CSol *ourSun: the sun's position is needed to convert heliocentric 
 *               coordinates into geocentric coordinates.
 *
 * return value: none
 *
 *************************************************************************/
void CCelestialBody::UpdatePosition(double mjd, CSol *ourSun)
{
  double eccAnom, v, ecl, actTime, 
    xv, yv, xh, yh, zh, xg, yg, zg, xe, ye, ze;

  UpdateOrbElements(mjd);
  actTime = CalcActTime(mjd);

  // calcualate the angle bewteen ecliptic and equatorial coordinate system
  ecl = SGD_DEGREES_TO_RADIANS * (23.4393 - 3.563E-7 *actTime);
  
  eccAnom = CalcEccAnom(M, e);  //calculate the eccentric anomaly
  xv = a * (cos(eccAnom) - e);
  yv = a * (sqrt (1.0 - e * e) * sin(eccAnom));
  v = atan2(yv, xv);           // the planet's true anomaly
  r = sqrt (xv*xv + yv*yv);    // the planet's distance
  
  // calculate the planet's position in 3D space
  xh = r * (cos(N) * cos(v+w) - sin(N) * sin(v+w) * cos(i));
  yh = r * (sin(N) * cos(v+w) + cos(N) * sin(v+w) * cos(i));
  zh = r * (sin(v+w) * sin(i));

  // calculate the ecliptic longitude and latitude
  xg = xh + ourSun->getxs();
  yg = yh + ourSun->getys();
  zg = zh;

  lonEcl = atan2(yh, xh);
  latEcl = atan2(zh, sqrt(xh*xh+yh*yh));

  xe = xg;
  ye = yg * cos(ecl) - zg * sin(ecl);
  ze = yg * sin(ecl) + zg * cos(ecl);
  rightAscension = atan2(ye, xe);
  declination = atan2(ze, sqrt(xe*xe + ye*ye));
  /* SG_LOG(SG_GENERAL, SG_INFO, "Planet found at : " 
   << rightAscension << " (ra), " << declination << " (dec)" ); */

  //calculate some variables specific to calculating the magnitude 
  //of the planet
  R = sqrt (xg*xg + yg*yg + zg*zg);
  s = ourSun->getDistance();

  // It is possible from these calculations for the argument to acos
  // to exceed the valid range for acos(). So we do a little extra
  // checking.

  double tmp = (r*r + R*R - s*s) / (2*r*R);
  if ( tmp > 1.0) { 
      tmp = 1.0;
  } else if ( tmp < -1.0) {
      tmp = -1.0;
  }

  FV = SGD_RADIANS_TO_DEGREES * acos( tmp );
}

/****************************************************************************
 * double CCelestialBody::sgCalcEccAnom(double M, double e)
 * this private member calculates the eccentric anomaly of a celestial body, 
 * given its mean anomaly and eccentricity.
 * 
 * -Mean anomaly: the approximate angle between the perihelion and the current
 *  position. this angle increases uniformly with time.
 *
 * True anomaly: the actual angle between perihelion and current position.
 *
 * Eccentric anomaly: this is an auxilary angle, used in calculating the true
 * anomaly from the mean anomaly.
 * 
 * -eccentricity. Indicates the amount in which the orbit deviates from a 
 *  circle (0 = circle, 0-1, is ellipse, 1 = parabola, > 1 = hyperbola).
 *
 * This function is also known as solveKeplersEquation()
 *
 * arguments: 
 * M: the mean anomaly in radians
 * e: the eccentricity
 *
 * return value:
 * the eccentric anomaly in radians
 *
 ****************************************************************************/
double CCelestialBody::CalcEccAnom(double M, double e)
{
  double 
    eccAnom, E0, E1, diff;
  
  eccAnom = M + e * sin(M) * (1.0 + e * cos (M));
  // iterate to achieve a greater precision for larger eccentricities 
  if (e > 0.05)
    {
      E0 = eccAnom;
      do
  {
    E1 = E0 - (E0 - e * sin(E0) - M) / (1 - e *cos(E0));
    diff = fabs(E0 - E1);
    E0 = E1;
  }
      while (diff > (SGD_DEGREES_TO_RADIANS * 0.001));
      return E0;
    }
  return eccAnom;
}

/*****************************************************************************
 * CCelestialBody::CCelestialBody are the public constructors for a generic
 *   celestial body object.
 * Initializes the 6 primary orbital elements. The elements are:
 * N: longitude of the ascending node
 * i: inclination to the ecliptic
 * w: argument of perihelion
 * a: semi-major axis, or mean distance from the sun
 * e: eccenticity
 * M: mean anomaly
 * Each orbital element consists of a constant part and a variable part that 
 * gradually changes over time. 
 *
 * Argumetns:
 * the 13 arguments to the constructor constitute the first, constant 
 * ([NiwaeM]f) and the second variable ([NiwaeM]s) part of the orbital 
 * elements. The 13th argument is the current time. Note that the inclination
 * is written with a capital (If, Is), because 'if' is a reserved word in the 
 * C/C++ programming language.
 ***************************************************************************/ 
CCelestialBody::CCelestialBody (double Nf, double If, double wf,
                double af, double ef, double Mf,
                double Ns, double Is, double ws,
                double as, double es, double Ms,
                double mjd)
{
  SetElements (Nf, If, wf, af, ef, Mf, Ns, Is, ws, as, es, Ms, mjd);
}

CCelestialBody::CCelestialBody (double Nf, double If, double wf,
                double af, double ef, double Mf,
                double Ns, double Is, double ws,
                double as, double es, double Ms)
{
  SetElements (Nf, If, wf, af, ef, Mf, Ns, Is, ws, as, es, Ms);
}


/*****************************************************************************
 * CCelestialBody::SetElements re-initializes the orbital elements for
 *   a celestialBody object.  See constructor comments for details
 ***************************************************************************/ 
void CCelestialBody::SetElements(double Nf, double If, double wf,
                 double af, double ef, double Mf,
                 double Ns, double Is, double ws,
                 double as, double es, double Ms,
                 double mjd)
{
  this->Nf = Nf;
  this->If = If;
  this->wf = wf;
  this->af = af;
  this->ef = ef;
  this->Mf = Mf;

  this->Ns = Ns;
  this->Is = Is;
  this->ws = ws;
  this->as = as;
  this->es = es;
  this->Ms = Ms;

  UpdateOrbElements(mjd);
}

void CCelestialBody::SetElements(double Nf, double If, double wf,
                 double af, double ef, double Mf,
                 double Ns, double Is, double ws,
                 double as, double es, double Ms)
{
  this->Nf = Nf;
  this->If = If;
  this->wf = wf;
  this->af = af;
  this->ef = ef;
  this->Mf = Mf;

  this->Ns = Ns;
  this->Is = Is;
  this->ws = ws;
  this->as = as;
  this->es = es;
  this->Ms = Ms;
}


/****************************************************************************
 * void CCelestialBody::updateOrbElements(double mjd)
 * given the current time, this private member calculates the actual 
 * orbital elements
 *
 * Arguments: double mjd: the current modified julian date:
 *
 * return value: none
 ***************************************************************************/
void CCelestialBody::UpdateOrbElements(double mjd)
{
  double actTime = CalcActTime(mjd);

  M = Norme2PI (SGD_DEGREES_TO_RADIANS * (Mf + (Ms * actTime)));
  w = Norme2PI (SGD_DEGREES_TO_RADIANS * (wf + (ws * actTime)));
  N = Norme2PI (SGD_DEGREES_TO_RADIANS * (Nf + (Ns * actTime)));
  i = Norme2PI (SGD_DEGREES_TO_RADIANS * (If + (Is * actTime)));
  e = ef + (es * actTime);
  a = af + (as * actTime);
}

/*****************************************************************************
 * double CCelestialBody::CalcActTime(double mjd)
 * this private member function returns the offset in days from the epoch for
 * wich the orbital elements are calculated (Jan, 1st, 2000).
 * 
 * Argument: the current time
 *
 * return value: the (fractional) number of days since Jan 1, 2000.
 ****************************************************************************/
double CCelestialBody::CalcActTime(double mjd)
{
//  return (mjd - 36523.5);
  return (mjd - 51544);
}

/*****************************************************************************
 * inline void CCelestialBody::getPos(double* ra, double* dec)
 * gives public access to Right Ascension and declination
 *
 ****************************************************************************/
void CCelestialBody::GetPos(double* ra, double* dec)
{
  *ra  = rightAscension;
  *dec = declination;
}

/*****************************************************************************
 * inline void CCelestialBody::getPos(double* ra, double* dec, double* magnitude
 * gives public acces to the current Right ascension, declination, magnitude
 * and colour
 ****************************************************************************/
void CCelestialBody::GetPos(double* ra, double* dec, double* magn)
{
  *ra = rightAscension;
  *dec = declination;
  *magn = magnitude;
}

/*****************************************************************************
 * Public accessor methods for right ascension, declination, magnitude,
 *   colour, latitude and longitude
 ****************************************************************************/

double CCelestialBody::GetRightAscension() { return rightAscension; }
double CCelestialBody::GetDeclination() { return declination; }
double CCelestialBody::GetMagnitude() { return magnitude; }
double CCelestialBody::GetLon() { return lonEcl; }
double CCelestialBody::GetLat() { return latEcl; }

/**************************************************************************
 * celestialBody.cxx
 * Written by Durk Talsma. Originally started October 1997, for distribution  
 * with the FlightGear project. Version 2 was written in August and 
 * September 1998. This code is based upon algorithms and data kindly 
 * provided by Mr. Paul Schlyter. (pausch@saaf.se). 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 *
 **************************************************************************/
