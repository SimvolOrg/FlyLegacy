/*
 * Mars.cpp
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

/*! \file Mars.cpp
 *  \brief Implements CMars ephemeris calculator for the planet Mars
 *
 *  Implements CMars ephemeris calculator for the planet Mars.
 *    This code was derived from similar code in SimGear 0.3.3
 */


#include <math.h>
#include "../Include/Ephemeris.h"


//
// Default orbital elements
//
static SOrbitalElements first =
{
  49.55740,   // N, Longitude of the ascending node
  1.8497,     // i, Inclination to the ecliptic
  286.5016,   // w, Argument of perihelion
  1.5236880,    // a, Semi-major axis
  0.093405,   // e, Eccentricity
  18.60210    // M, Mean anomaly
};

static SOrbitalElements second =
{
  2.1108100E-5, // N, Longitude of the ascending node
  -1.78E-8,   // i, Inclination to the ecliptic
  2.9296100E-5, // w, Argument of perihelion
  0.0,      // a, Semi-major axis
  2.516E-9,   // e, Eccentricity
  0.52402077660 // M, Mean anomaly
};


/*************************************************************************
 * CMars::CMars(double mjd)
 * Public constructor for class CMars
 * Argument: The current time.
 * the hard coded orbital elements for CMars are passed to 
 * CelestialBody::CelestialBody();
 ************************************************************************/
CMars::CMars(double mjd) :
  CCelestialBody (first.N, first.i, first.w, first.a, first.e, first.M,
                second.N, second.i, second.w, second.a, second.e, second.M,
          mjd)
{
}

CMars::CMars() :
  CCelestialBody (first.N, first.i, first.w, first.a, first.e, first.M,
                second.N, second.i, second.w, second.a, second.e, second.M)
{
}


/*************************************************************************
 * void CMars::updatePosition(double mjd, CSol *ourSun)
 * 
 * calculates the current position of CMars, by calling the base class,
 * CelestialBody::updatePosition(); The current magnitude is calculated using 
 * a Mars specific equation
 *************************************************************************/
void CMars::UpdatePosition(double mjd, CSol *ourSun)
{
  CCelestialBody::UpdatePosition (mjd, ourSun);
  magnitude = -1.51 + 5*log10( r*R ) + 0.016 * FV;
}

/**************************************************************************
 * mars.cxx
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

