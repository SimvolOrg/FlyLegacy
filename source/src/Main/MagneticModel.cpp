/*
 * MagneticModel.cpp
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

/*! \file MagneticModel.cpp
 *  \brief Implements the CMagneticModel class
 *
 *  The CMagneticModel class implements a simplified version of the World Magnetic
 *    Model 2005.  This class currently only supports calculation of the
 *    magnetic declination and horizontal field strength, but other values can
 *    be easily implemented if required.
 *  Portions of this code are taken from GeoStarsLib v0.5 (c) Dean Nelson,
 *    see http://www.geostarslib.sourceforge.net
 */

#include "../Include/MagneticModel.h"
#include "../Include/Utility.h"
#include "../Include/Globals.h"
#include <stdio.h>

CMagneticModel CMagneticModel::instance;
//=============================================================================
//  Build magnetic model
//==============================================================================
CMagneticModel::CMagneticModel (void)
{
  // Initialize constants
  maxOrder = 12;
  maxDegree = 12;
  epoch = 2005.0;
  A = 6378.137;
  B = 6356.7523142;
  magRefRadius = 6371.2;
  A2 = A * A;
  B2 = B * B;
  C2 = A2 - B2;
  A4 = A2 * A2;
  B4 = B2 * B2;
  C4 = A4 - B4;

  // Initialize the model date to the middle of the supported date range
  //   for the WMM2005 model
  // eal Init (2007.5);
}
//------------------------------------------------------------------------------
//  Delete the model
//------------------------------------------------------------------------------
CMagneticModel::~CMagneticModel()
{ globals->mag = 0;
}
//
// Magnetic model coefficients are stored in a text file with following format:
//
// 
// Each line of data is of the format:
//  n m g h dg dh
// where:
//    int n
//    int m
//    float g     is the Gauss coefficient g-sub-m-super-n
//    float h     is the Gause coefficient h-sub-m-super-n
//    float dg    is the time derivative of g
//    float dh    is the time derivative of h
//
void CMagneticModel::ReadCoefficients (void)
{
  char cofFilename[PATH_MAX];
  strcpy (cofFilename, "SYSTEM/WMM2010.COF");
  FILE* f = fopen (cofFilename, "r");
  if (f) {
    // Read WMM spherical harmonic coefficients
    char s[80];
    c[0][0] = 0;
    cd[0][0] = 0;

    // Skip header line and get first line
    fgets (s, 80, f);
    fgets (s, 80, f);
    while (strncmp (s, "9999", 4) != 0) {
      // Not end-of-file, parse this line for coefficients
      int n, m;
      float gnm, hnm, dgnm, dhnm;
      sscanf (s, "%d%d%f%f%f%f", &n, &m, &gnm, &hnm, &dgnm, &dhnm);
      if (m <= n) {
        c[m][n] = gnm;
        cd[m][n] = dgnm;
        if (m != 0) {
          c[n][m-1] = hnm;
          cd[n][m-1] = dhnm;
        }
      }

      // Read next line
      fgets (s, 80, f);
    }

    // Close coefficients file
    fclose (f);

    // Convert Schmidt normalized Gauss coefficients to unnormalized
    snorm[0][0] = 1.0;
    int j, n, m;
    for (n=1; n<=maxOrder; n++) {
      snorm[0][n] = snorm[0][n-1] * (double)(2*n - 1) / (double)n;
      j = 2;
      for (m=0; m<=n; m++) {
        k[m][n] = (double)(((n-1)*(n-1))-(m*m))/(double)((2*n-1)*(2*n-3));
        if (m > 0) {
          double flnmj = (double)((n-m+1)*j)/(double)(n+m);
          snorm[m][n] = snorm[m-1][n] * sqrt(flnmj);
          j = 1;
          c[n][m-1] = snorm[m][n] * c[n][m-1];
          cd[n][m-1] = snorm[m][n] * cd[n][m-1];
        }
        c[m][n] = snorm[m][n]*c[m][n];
        cd[m][n] = snorm[m][n]*cd[m][n];
      }
      fn[n] = (double)(n+1);
      fm[n] = (double)n;
    }
    k[1][1] = 0.0;

  } else {
    WARNINGLOG ("CMagneticModel : Cannot open coefficients file %s", cofFilename);
  }
}

//
// Initialize magnetic model coefficients to specified date.  This is intended
//   to allow optimization of the calls to CalculateFieldStrengths and related
//   methods.  Since the parameter values change very slowly over years, it
//   is sufficient to call this method to initialize the model at the beginning
//   of the situation
//
void CMagneticModel::Init (double year)
{ // Read Gauss coefficients
  ReadCoefficients ();
  modelDate = year;
  double dYear = modelDate - epoch;

  // Time-adjust Gauss coefficients for the specified date
  for (int n=1; n<=maxOrder; n++) {
    for (int m=0; m<=n; m++) {
      tc[m][n] = c[m][n] + (dYear * cd[m][n]);
      if (m != 0) tc[n][m-1] = c[n][m-1] + (dYear * cd[n][m-1]);
    }
  }
}

void CMagneticModel::Init (SDate date)
{
  // Calculate approximate decimal year; this doesn't need high precision
  float fDayInYear = (float)(date.month-1) * 30.4f + (float)(date.day-1);
  Init ((float)(date.year + 1900) + (fDayInYear / 365.0f));
  globals->mag = this;
}

//
// Calculate magnetic field strengths
//
// Parameters:
//    SPosition pos Position in standard Fly! format (arcsec lat/lon, feet MSL altitude)
//
// Outputs:
//    float   X     Eastward field strength
//    float   Y     Northward field strength
//    float   Z     Downward field strength
//
void CMagneticModel::CalculateFieldStrengths (SPosition pos,
                                              float &X, float &Y, float &Z)
{
  int n, m;

  // Convert position to radian lat/lon and km alt
  double rlat = FN_RAD_FROM_ARCS(pos.lat);							//DegToRad ((float)pos.lat / 3600.0f);
  double rlon = DegToRad (Wrap180 ((float)pos.lon / 3600.0f));
  double alt  = FN_METRE_FROM_FEET (pos.alt) / 1000.0f;

  // Initialize table of sin and cos powers
  double srlat = sin(rlat);
  double srlat2 = srlat * srlat;
  double srlon = sin(rlon);
  double crlat = cos(rlat);
  double crlat2 = crlat * crlat;
  double crlon = cos(rlon);
  sp[0] = 0;
  sp[1] = srlon;
  cp[0] = 1.0;
  cp[1] = crlon;
  for (m=2; m<=maxOrder; m++) {
    sp[m] = sp[1]*cp[m-1]+cp[1]*sp[m-1];
    cp[m] = cp[1]*cp[m-1]-sp[1]*sp[m-1];
  }

  // Transform geodetic coordinates to spherical geocentric coordinates
  double Q = sqrt (A2 - C2 * srlat2);
  double Q1 = alt * Q;
  double Q2 = ((Q1 + A2) / (Q1 + B2)) * ((Q1 + A2) / (Q1 + B2));
  double ct = srlat / sqrt(Q2 * crlat2 + srlat2);
  double st = sqrt (1.0 - (ct * ct));
  double r2 = (alt * alt) + 2.0 * Q1 + (A4 - C4 * srlat2) / (Q * Q);
  double r = sqrt(r2);
  double d = sqrt (A2 * crlat2 + B2 * srlat2);
  double ca = (alt + d) / r;
  double sa = C2 * crlat * srlat / (r * d);

  // aor is "A over r"
  double aor = A / r;
  double ar = aor * aor;
  double br = 0.0;
  double bt = 0.0;
  double bp = 0.0;
  double bpp = 0.0;

  p[0][0] = 1.0;
  pp[0] = 1.0;
  dp[0][0] = 0.0;
  for (n=1; n<=maxOrder; n++) {
    ar *= aor;
    for (m=0; m<=n; m++) {
      // Compute unnormalized associated Legendre functions and derivatives
      if (n == m) {
        p[m][n] = st * p[m-1][n-1];
        dp[m][n] = st * dp[m-1][n-1] + ct * p[m-1][n-1];
      } else if ((n==1) && (m==0)) {
        p[m][n]  = ct * p[m][n-1];
        dp[m][n] = ct * dp[m][n-1] - st * p[m][n-1];
      } else if ((n>1) && (n!=m)) {
        if (m > n-2) p[m][n-2]  = 0.0;
        if (m > n-2) dp[m][n-2] = 0.0;
        p[m][n]  = ct * p[m][n-1]  - k[m][n] * p[m][n-2];
        dp[m][n] = ct * dp[m][n-1] - st * p[m][n-1] - k[m][n] * dp[m][n-2];
      }

      // Accumulate terms of the spherical harmonic expansion
      double par = ar * p[m][n];
      double temp1, temp2;
      if (m==0) {
        temp1 = tc[m][n] * cp[m];
        temp2 = tc[m][n] * sp[m];
      } else {
        temp1 = tc[m][n]*cp[m]+tc[n][m-1]*sp[m];
        temp2 = tc[m][n]*sp[m]-tc[n][m-1]*cp[m];
      }
      bt = bt-ar*temp1*dp[m][n];
      bp += (fm[m]*temp2*par);
      br += (fn[n]*temp1*par);

      // Special case : North/South geographic poles
      if (st == 0.0 && m == 1) {
        if (n == 1) pp[n] = pp[n-1];
        else pp[n] = ct*pp[n-1]-k[m][n]*pp[n-2];
        double parp = ar*pp[n];
        bpp += (fm[m]*temp2*parp);
      }
    }
  }
  if (st==0.0) bp = bpp; else bp /= st;

  // Transform geocentric field vector components back to geodetic reference frame
  X = (float)((-bt * ca) - (br * sa));
  Y = (float)bp;
  Z = (float)((bt * sa) - (br * ca));
}

//
// Return selected magnetic model elements for the given position 
//
void CMagneticModel::GetElements (SPosition pos, float& D, float& H)
{
  float X, Y, Z;
  CalculateFieldStrengths (pos, X, Y, Z);

  // Calculate declination and convert to degrees
  D = RadToDeg ((float)atan2 (Y, X));

  // Calculate horizontal field strength
  H = (float)sqrt (X*X + Y*Y);
}

void CMagneticModel::Test (const char* filename)
{ /*
  int n, m;

  FILE* f = fopen (filename, "w");
  if (f) {
    fprintf (f, "Fly! Legacy Magnetic Model Test\n");
    fprintf (f, "===============================\n");
    fprintf (f, "\n");
    fprintf (f, "Epoch : %6.1f\n", epoch);
    fprintf (f, "Date  : %6.1f\n", modelDate);
    fprintf (f, "\n");

    // Print normalized coefficients
    fprintf (f, "Coefficients (Un-Normalized):\n");
    fprintf (f, "-----------------------------\n");
    for (n=1; n<=maxOrder; n++) {
      for (m=0; m<=n; m++) {
        double g = c[m][n];
        double dg = cd[m][n];
        double h = 0;
        double dh = 0;
        if (m != 0) {
          h = c[n][m-1];
          dh = cd[n][m-1];
        }
        fprintf (f, "%d\t%d\t%8.1f\t%8.1f\t%8.1f\t%8.1f\n", n, m, g, h, dg, dh);
      }
    }
    fprintf (f, "\n");

    // Print actual coefficients for model date
    fprintf (f, "Coefficients (Time-adjusted):\n");
    fprintf (f, "-----------------------------\n");
    for (n=1; n<=maxOrder; n++) {
      for (m=0; m<=n; m++) {
        double g = tc[m][n];
        double h = 0;
        if (m != 0) {
          h = tc[n][m-1];
        }
        fprintf (f, "%d\t%d\t%8.1f\t%8.1f\n", n, m, g, h);
      }
    }
    fprintf (f, "\n");

    // Print test data
    fprintf (f, "Test Results:\n");
    fprintf (f, "-------------\n");
    SPosition pos;
    double latStep = -40.0f;
    double lonStep = +60.0f;
    for (int i=0; i<5; i++) {
      // Five latitude steps from 80 deg N through to 80 deg S
      pos.lat = (80.0 + ((double)(i) * latStep)) * 3600.0;
      for (int j=0; j<6; j++) {
        // Six longitude steps from 0 deg through to 300 deg
        pos.lon = ((double)(j) * lonStep) * 3600.0;
        pos.alt = 0;

        float D, H;
        GetElements (pos, D, H);

        fprintf (f, "Lat : %6.2f\tLon: %6.2f\tD = %6.2f\tH=%9.2f\n",
          pos.lat/3600.0f, pos.lon/3600.0f, D, H);
      }
    }

    fclose (f);
  }
	*/
}

