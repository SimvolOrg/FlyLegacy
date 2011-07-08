/*
 * MagneticModel.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2006 Chris Wallace
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

/*! \file MagneticModel.h
 *  \brief Defines the CMagneticModel class
 */


#ifndef MAGNETICMODEL_H
#define MAGNETICMODEL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "FlyLegacy.h"


#define snorm p

class CMagneticModel
{
private:
  CMagneticModel (void);
 ~CMagneticModel();
public:
  static CMagneticModel& Instance() { return instance; }

  // CMagneticModel methods
  void    Init (double year);
  void    Init (SDate date);
  void    GetElements (SPosition pos, float& D, float& H);
  void    Test (const char* filename);

protected:
  void    ReadCoefficients (void);
  void    CalculateFieldStrengths (SPosition pos, float &X, float &Y, float &Z);

private:
  int    maxOrder;         ///< Max order of magnetic model
  int    maxDegree;        ///< Max degree of magnetic model
  double epoch;            ///< Epoch date for magnetic model
  double A;                ///< Equatorial radius of earth spheroid model
  double B;                ///< Polar radius of earth spheroid model
  double magRefRadius;     ///< Radius of earth in magnetic reference model
  double A2, B2, C2;       ///< Squares of earth radii
  double A4, B4, C4;       ///< Fourth power of earth radii

protected:
  double    modelDate;              ///< Current date for model

  double    c[13][13];              ///< Gauss coefficients of main geomagnetic field
  double    cd[13][13];             ///< Secular variation of Gauss coefficients
  double    tc[13][13];             ///< Time-adjusted Gauss coefficients
  double    dp[13][13];             ///< Theta derivative of P(n,m) (unnormalized)
  double    snorm[13][13];          ///< Schmidt normalization factors
  double    sp[13];                 ///< Powers of sin(M * lonPrime)
  double    cp[13];                 ///< Powers of cos(M * lonPrime)
  double    fn[13], fm[13];         ///< Tables of (float)(n+1) and (float)(n)
  double    pp[13];                 ///< Associated Legendre polynomials
  double    k[13][13];              ///< ???

  static CMagneticModel instance;
};

#endif // MAGNETICMODEL_H

