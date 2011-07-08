/*
 * Saturn.cpp
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

/*! \file Saturn.cpp
 *  \brief Implements CSaturn ephemeris calculator for the planet Saturn
 *
 *  Implements CSaturn ephemeris calculator for the planet Saturn. 
 *    This code was derived from similar code in SimGear.0.3.3
 */


#include <math.h>
#include "../Include/Ephemeris.h"


//
// Default orbital elements
//
static SOrbitalElements first =
{
  113.6634,   // N, Longitude of the ascending node
  2.4886,     // i, Inclination to the ecliptic
  339.3939,   // w, Argument of perihelion
  9.5547500,    // a, Semi-major axis
  0.055546,   // e, Eccentricity
  316.9670    // M, Mean anomaly
};

static SOrbitalElements second =
{
  2.3898000E-5, // N, Longitude of the ascending node
  -1.081E-7,    // i, Inclination to the ecliptic
  2.9766100E-5, // w, Argument of perihelion
  0.0,      // a, Semi-major axis
  -9.499E-9,    // e, Eccentricity
  0.0334422820  // M, Mean anomaly
};



/*************************************************************************
 * CSaturn::CSaturn(double mjd)
 * Public constructor for class CSaturn
 * Argument: The current time.
 * the hard coded orbital elements for CSaturn are passed to 
 * CelestialBody::CelestialBody();
 ************************************************************************/
CSaturn::CSaturn(double mjd) :
  CCelestialBody (first.N, first.i, first.w, first.a, first.e, first.M,
                second.N, second.i, second.w, second.a, second.e, second.M,
          mjd)
{
}

CSaturn::CSaturn() :
  CCelestialBody (first.N, first.i, first.w, first.a, first.e, first.M,
                second.N, second.i, second.w, second.a, second.e, second.M)
{
}


/*************************************************************************
 * void CSaturn::updatePosition(double mjd, CSol *ourSun)
 * 
 * calculates the current position of CSaturn, by calling the base class,
 * CelestialBody::updatePosition(); The current magnitude is calculated using 
 * a Saturn specific equation
 *************************************************************************/
void CSaturn::UpdatePosition(double mjd, CSol *ourSun)
{
  CCelestialBody::UpdatePosition(mjd, ourSun);
  
  double actTime = CalcActTime(mjd);
  double ir = 0.4897394;
  double Nr = 2.9585076 + 6.6672E-7*actTime;
  double B = asin (sin(declination) * cos(ir) - 
       cos(declination) * sin(ir) *
       sin(rightAscension - Nr));
  double ring_magn = -2.6 * sin(fabs(B)) + 1.2 * pow(sin(B), 2);
  magnitude = -9.0 + 5*log10(r*R) + 0.044 * FV + ring_magn;
}


/**************************************************************************
 * saturn.cxx
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

