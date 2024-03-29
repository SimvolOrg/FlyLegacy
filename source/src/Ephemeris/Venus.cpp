/*
 * Venus.cpp
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

/*! \file Venus.cpp
 *  \brief Implements CVenus ephemeris calculator for the planet Venus
 *
 *  Implements CVenus ephemeris calculator for the planet Venus.
 *    This code was derived from similar code in SimGear 0.3.3
 */


#include <math.h>
#include "../Include/Ephemeris.h"


//
// Default orbital elements
//
static SOrbitalElements first =
{
  76.67990,   // N, Longitude of the ascending node
  3.3946,     // i, Inclination to the ecliptic
  54.89100,   // w, Argument of perihelion
  0.7233300,    // a, Semi-major axis
  0.006773,   // e, Eccentricity
  48.00520    // M, Mean anomaly
};

static SOrbitalElements second =
{
  2.4659000E-5, // N, Longitude of the ascending node
  2.75E-8,    // i, Inclination to the ecliptic
  1.3837400E-5, // w, Argument of perihelion
  0.0,      // a, Semi-major axis
  -1.302E-9,    // e, Eccentricity
  1.60213022440 // M, Mean anomaly
};



/*************************************************************************
 * CVenus::CVenus(double mjd)
 * Public constructor for class CVenus
 * Argument: The current time.
 * the hard coded orbital elements for CVenus are passed to 
 * CelestialBody::CelestialBody();
 ************************************************************************/
CVenus::CVenus(double mjd) :
  CCelestialBody (first.N, first.i, first.w, first.a, first.e, first.M,
                second.N, second.i, second.w, second.a, second.e, second.M,
          mjd)
{
}

CVenus::CVenus() :
  CCelestialBody (first.N, first.i, first.w, first.a, first.e, first.M,
                second.N, second.i, second.w, second.a, second.e, second.M)
{
}


/*************************************************************************
 * void CVenus::updatePosition(double mjd, CSol *ourSun)
 * 
 * calculates the current position of Venus, by calling the base class,
 * CelestialBody::updatePosition(); The current magnitude is calculated using 
 * a Venus specific equation
 *************************************************************************/
void CVenus::UpdatePosition(double mjd, CSol *ourSun)
{
  CCelestialBody::UpdatePosition(mjd, ourSun);
  magnitude = -4.34 + 5*log10( r*R ) + 0.013 * FV + 4.2E-07 * pow(FV,3);
}


/**************************************************************************
 * venus.cxx
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
