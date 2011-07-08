/*
 * Mercury.cpp
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

/*! \file Mercury.cpp
 *  \brief Implements CMercury ephemeris calculator for the planet Mercury
 *
 *  Implements CMercury ephemeris calculator for the planet Mercury.
 *    This code was derived from similar code in SimGear 0.3.3
 */


#include <math.h>
#include "../Include/Ephemeris.h"


//
// Default orbital elements
//
static SOrbitalElements first =
{
  48.33130,   // N, Longitude of the ascending node
  7.0047,     // i, Inclination to the ecliptic
  29.12410,   // w, Argument of perihelion
  0.3870980,    // a, Semi-major axis
  0.205635,   // e, Eccentricity
  168.6562    // M, Mean anomaly
};

static SOrbitalElements second =
{
  3.2458700E-5, // N, Longitude of the ascending node
  5.00E-8,    // i, Inclination to the ecliptic
  1.0144400E-5, // w, Argument of perihelion
  0.0,      // a, Semi-major axis
  5.59E-10,   // e, Eccentricity
  4.09233443680 // M, Mean anomaly
};



/*************************************************************************
 * CMercury::CMercury(double mjd)
 * Public constructor for class CMercury
 * Argument: The current time.
 * the hard coded orbital elements for CMercury are passed to 
 * CelestialBody::CelestialBody();
 ************************************************************************/
CMercury::CMercury(double mjd) :
  CCelestialBody (first.N, first.i, first.w, first.a, first.e, first.M,
                second.N, second.i, second.w, second.a, second.e, second.M,
          mjd)
{
}

CMercury::CMercury() :
  CCelestialBody (first.N, first.i, first.w, first.a, first.e, first.M,
                second.N, second.i, second.w, second.a, second.e, second.M)
{
}


/*************************************************************************
 * void CMercury::updatePosition(double mjd, CSol *ourSun)
 * 
 * calculates the current position of CMercury, by calling the base class,
 * CelestialBody::updatePosition(); The current magnitude is calculated using 
 * a Mercury specific equation
 *************************************************************************/
void CMercury::UpdatePosition(double mjd, CSol *ourSun)
{
  CCelestialBody::UpdatePosition(mjd, ourSun);
  magnitude = -0.36 + 5*log10( r*R ) + 0.027 * FV + 2.2E-13 * pow(FV, 6); 
}

/**************************************************************************
 * mercury.cxx
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

