/*
 * Sol.cpp
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

/*! \file Sol.cpp
 *  \brief Implements CSol ephemeris calculator for Earth's sun (Sol)
 *
 *  Implements CSol ephemeris calculator for Earth's sun (Sol).
 *    This code was derived from similar code in SimGear 0.3.3
 */


#include <math.h>
#include <plib/sg.h>
#include "../Include/Ephemeris.h"

//
// Default orbital elements
//
static const SOrbitalElements first =
{
  0.0,      // N, Longitude of the ascending node
  0.0,      // i, Inclination to the ecliptic
  282.9404,   // w, Argument of perihelion
  1.000000,   // a, Semi-major axis
  0.016709,   // e, Eccentricity
  356.0470    // M, Mean anomaly
};

static const SOrbitalElements second =
{
  0.0000000,    // N, Longitude of the ascending node
  0.0000000,    // i, Inclination to the ecliptic
  4.7093500E-5, // w, Argument of perihelion
  0.0000000,    // a, Semi-major axis
  -1.151E-9,    // e, Eccentricity
  0.98560025850 // M, Mean anomaly
};


/*************************************************************************
 * CSol::CSol(double mjd)
 * Public constructor for class Star
 * Argument: The current time.
 * the hard coded orbital elements our sun are passed to 
 * CelestialBody::CelestialBody();
 * note that the word sun is avoided, in order to prevent some compilation
 * problems on sun systems 
 ************************************************************************/
CSol::CSol (double mjd) :
  CCelestialBody (first.N, first.i, first.w, first.a, first.e, first.M,
                second.N, second.i, second.w, second.a, second.e, second.M,
          mjd)
{
  distance = 0.0;
}

CSol::CSol () :
  CCelestialBody (first.N, first.i, first.w, first.a, first.e, first.M,
                second.N, second.i, second.w, second.a, second.e, second.M)
{
  distance = 0.0;
}

CSol::~CSol()
{
}


/*************************************************************************
 * void CSol::updatePosition (double mjd)
 * 
 * calculates the current position of our sun.
 *************************************************************************/
void CSol::UpdatePosition(double mjd)
{
  // Determine number of days since the epoch for this algorithm: 1 Jan 2000
  double d = CalcActTime(mjd);
  
  // Update orbital elements for the current date/time
  UpdateOrbElements(mjd);

  // Calculate obliquity of the ecliptic in radians
  double ecl = SGD_DEGREES_TO_RADIANS * (23.4393 - 3.563E-7 * d);

  // Calculate eccentric anomaly (aka solving Kepler's equation)
  //   Result is in radians
  double eccAnom = CalcEccAnom(M, e);
  
  // Calculate distance (r) and true anomaly (v)
  double xv = cos(eccAnom) - e;
  double yv = sqrt (1.0 - e*e) * sin(eccAnom);
  double v = atan2 (yv, xv);
  double r = sqrt (xv*xv + yv*yv);  // and its distance
  distance = r;

  // Calculate true longitude and latitude
  lonEcl = v + w;
  latEcl = 0;

  // Convert true longitude to ecliptic rectangular geocentric
  //    coordinates (xs, ys)
  xs = r * cos (lonEcl);
  ys = r * sin (lonEcl);

  // Convert ecliptic coordinates to equatorial rectangular
  //   geocentric coordinates
  double xe = xs;
  double ye = ys * cos (ecl);
  double ze = ys * sin (ecl);

  // And finally, calculate right ascension and declination
  rightAscension = atan2 (ye, xe);
  declination = atan2 (ze, sqrt (xe*xe + ye*ye));
}


/**************************************************************************
 * star.cxx
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

