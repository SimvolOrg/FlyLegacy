/*
 * Ephemeris.h
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

/*! \file Ephemeris.h
 *  \brief Defines CCelestialBody and related ephemeris calculation classes
 *
 * These classes encapsulate the ephemeris calculations for celestial
 *   bodies such as the sun, moon and planets.  This code was originally
 *   taken from SimGear 0.3.3; see the end of this file for the original
 *   SimGear comments.
 *
 * Modifications for Fly! Legacy:
 *
 *  - Cosmetic changes to class and method names, include files, etc.
 *  - Support for celestial body colour, allowing stars and planets to be rendered
 *      more realistically
 *  - In original code, orbital elements were supplied to the constructor, and
 *      could not be changed.  Modified to allow updating of orbital elements,
 *      this will come in handy when minor planets, comets, etc. are supported
 *
 */

#ifndef EPHEMERIS_H
#define EPHEMERIS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlyLegacy.h"


//
// Number of days in a single lunar cycle (lunation)
//
#define LUNATION_DAYS 29.530589


//
// Structure definition containing orbital elements for a celestial body
//
typedef struct {
  double    N;    // Longitude of the ascending node
  double    i;    // Inclination to the ecliptic
  double    w;    // Argument of perihelion
  double    a;    // Semi-major axis
  double    e;    // Eccentricity
  double    M;    // Mean anomaly
} SOrbitalElements;


class CSol;

class CCelestialBody
{
protected:
  double    Nf, If, wf, af, ef, Mf;   // First (constant) part of orbital elements
  double    Ns, Is, ws, as, es, Ms;   // Second (variable) part of orbital elements
  double    N, i, w, a, e, M;     // Actual orbital elements at last update

  double    rightAscension, declination;
  double    r, R, s, FV;
  double    magnitude;
  double    lonEcl, latEcl;

protected:
  double  CalcEccAnom (double M, double e);
  double  CalcActTime (double mjd);
  void  UpdateOrbElements (double mjd);

public:
  CCelestialBody (double Nf, double If, double wf, double af, double ef, double Mf,
          double Ns, double Is, double ws, double as, double es, double Ms,
          double mjd);

  CCelestialBody (double Nf, double If, double wf, double af, double ef, double Mf,
          double Ns, double Is, double ws, double as, double es, double Ms);

  void  SetElements (double Nf, double If, double wf, double af, double ef, double Mf,
             double Ns, double Is, double ws, double as, double es, double Ms,
             double mjd);

  void  SetElements (double Nf, double If, double wf, double af, double ef, double Mf,
             double Ns, double Is, double ws, double as, double es, double Ms);

  void  GetPos(double *ra, double *dec);
  void  GetPos(double *ra, double *dec, double *magnitude);
  double  GetRightAscension();
  double  GetDeclination();
  double  GetMagnitude();

  double  GetLon();
  double  GetLat(); 
  void  UpdatePosition (double mjd, CSol *ourSun);
};


class CSol : public CCelestialBody
{
public:
  CSol (double mjd);
  CSol ();
  ~CSol();

  // CSol methods
    void  UpdatePosition (double mjd);

  inline double getM(void) { return M; }
  inline double getw(void) { return w; }
  inline double getxs(void) { return xs; }
  inline double getys(void) { return ys; }
  inline double getDistance(void) { return distance; }

private:
    double xs, ys;     // the sun's rectangular geocentric coordinates
    double distance;   // the sun's distance to the earth
};


class CMoon : public CCelestialBody {
public:
  CMoon (double mjd);
  CMoon ();
  ~CMoon ();

  float age;    // Lunar phase, 0.0 = new, 0.5 = full

  // CMoon methods
  void UpdatePosition(double mjd, double lst, double lat, CSol *ourSun);
  float GetAge (void);
};


class CMercury : public CCelestialBody
{
public:
  CMercury (double mjd);
  CMercury ();

  // CCelestialBody methods
  void UpdatePosition(double mjd, CSol* ourSun);
};


class CVenus : public CCelestialBody
{
public:
  CVenus (double mjd);
  CVenus ();

  // CCelestialBody methods
  void UpdatePosition(double mjd, CSol *ourSun);
};


class CMars : public CCelestialBody
{
public:
  CMars ( double mjd );
  CMars ();

  // CCelestialBody methods
  void UpdatePosition(double mjd, CSol *ourSun);
};


class CJupiter : public CCelestialBody
{
public:
  CJupiter (double mjd);
  CJupiter ();

  // CCelestialBody methods
  void UpdatePosition(double mjd, CSol *ourSun);
};


class CSaturn : public CCelestialBody
{
public:
  CSaturn (double mjd);
  CSaturn ();

  // CCelestialBody methods
  void UpdatePosition(double mjd, CSol *ourSun);
};


/**************************************************************************
 * celestialBody.hxx
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

#endif // EPHEMERIS_H
