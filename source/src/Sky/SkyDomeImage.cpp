/*
 * SkyDomeImage.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2007 Chris Wallace
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

/**
 * \file SkyDomeImage.cpp
 * \brief Implements CSkyDomeImage to render shaded sky backdrop
 */


#include "../Include/Globals.h"
#include "../Include/Sky.h"
#include "../Include/Utility.h"
#include "../Include/Fui.h"
#include <plib/sg.h>

using namespace std;

// The following constant defines how far below the horizon the sky dome hemisphere extends
#define SKYDOME_HEMISPHERE_SCALE  (1.10)

/**
 * Calculate Perez function
 *
 * @param theta Zenith angle in radians
 * @param gamma Solar angle in radians
 * @param p     Perez coefficients
 * @returns     Result of Perez function calculation
 */
static float PerezFunction (float theta, float gamma, const SPerezCoefficients &p)
{
  // Check for indeterminate case of cos(theta)=0, where exp(p.B/cos(theta))=0,
  //   assuming p.B is negative
  float f1;
  double epsilon = 1.0e-10;
  if (cos(theta) < epsilon) {
    f1 = 1;
  } else {
    f1 = 1 + (p.A * exp (p.B / cos(theta)));
  }
  float cosgamma = cos(gamma);
  float cos2gamma = cosgamma * cosgamma;
  float f2 = 1 + (p.C * exp (p.D * gamma)) + (p.E * cos2gamma);

  return f1 * f2;
}

/*
 * Calculate Perez function coefficients from parameterized coefficient parameters
 *
 * The Perez coefficient parameters use turbidity T as a parameter; this function
 * calculates the coefficients for a given value of T.
 *
 * @param T     Turbidity
 * @param parm  Perez coefficient parameters
 * @param coeff Updated with Perez coefficients
 */
static void CalcPerezCoefficients (float T,
                                   const SCoefficientParameters &parm,
                                   SPerezCoefficients &coeff)
{
  coeff.A = parm.A[1] * T + parm.A[0];
  coeff.B = parm.B[1] * T + parm.B[0];
  coeff.C = parm.C[1] * T + parm.C[0];
  coeff.D = parm.D[1] * T + parm.D[0];
  coeff.E = parm.E[1] * T + parm.E[0];
}

          
/**
 * Calculate the Y (luminance component) for the zenith
 *
 * @param theta Solar zenith angle in radians
 * @param T     Atmospheric turbidity
 * @param p     Zenith luminance coefficients
 * @return      Calculated zenith luminance value in Yxy colour space
 */
static float ZenithLuminance (float theta, float T, const SLuminanceZenith &p)
{
  float chi = ((4.0f / 9.0f) - (T / 120.0f)) * (SG_PI - 2.0f * theta);
  float Yz = ((p.A * T) + p.B) * tan(chi) + (p.C * T) + p.D;
  Yz *= 1000.0f;

  return Yz;
}

/**
 * Calculate chrominance (x or y) component for the zenith
 *
 * @param theta Solar zenith angle in radians
 * @param T     Atmospheric turbidity
 * @param p     Zenith chrominance coefficients
 * @return      Calculated chrominance value in Yxy colour space
 */
static float ZenithChrominance (float theta, float T, const SChromaZenith &p)
{
  float rc = 0.0;
  float T2 = T * T;
  float theta2 = theta * theta;
  float theta3 = theta * theta * theta;
  rc = 
    (p.t2[3] * theta3 + p.t2[2] * theta2 + p.t2[1] * theta + p.t2[0]) * T2 +
    (p.t1[3] * theta3 + p.t1[2] * theta2 + p.t1[1] * theta + p.t1[0]) * T +
    (p.t0[3] * theta3 + p.t0[2] * theta2 + p.t0[1] * theta + p.t0[0]);

  return rc;
}

/**
 * Convert colour value from CIE Yxy colour space to RGB
 *
 * Conversion is done in two steps; first from CIE Yxy to CIE XYZ, then
 * from XYZ to RGB.  A D65 white point is assumed.
 *
 * @param Y   CIE Yxy luminance value
 * @param x   CIE Yxy chrominance (x) value
 * @param y   CIE Yxy chrominance (y) value
 * @param r   Red component of RGB colour in range [0.0, 1.0]
 * @param g   Green component of RGB colour in range [0.0, 1.0]
 * @param b   Blue component of RGB colour in range [0.0, 1.0]
 */
static void YxyToRGB (float Y, float x, float y, float* r, float* g, float* b)
{
  // First convert to CIE XYZ format
  float X = (x / y) * Y;
  float Z = ((1 - x - y) / y) * Y;

  // Convert XYZ to RGB using D65 white point
  float factor = 1.0f - exp(-(1.0f / 15000.0f));
  float R = factor * ( 3.240479f * X + -1.537150f * Y + -0.498535f * Z);
  float G = factor * (-0.969256f * X +  1.875992f * Y +  0.041556f * Z);
  float B = factor * ( 0.055648f * X + -0.204043f * Y +  1.057311f * Z);

  if (R < 0.0f) R = 0.0f;
  if (R > 1.0f) R = 1.0f;

  if (G < 0.0f) G = 0.0f;
  if (G > 1.0f) G = 1.0f;
  
  if (B < 0.0f) B = 0.0f;
  if (B > 1.0f) B = 1.0f;

  // Gamma-adjust the RGB colour with standard sRGB gamma of 2.2
  float gamma = 2.2f;
  float invgamma = 1.0f / gamma;
  *r = pow (R, invgamma);
  *g = pow (G, invgamma);
  *b = pow (B, invgamma);
}

/**
 * Initialize Perez coefficient parameters from a loaded INI  file
 *
 * @param p      Perez parameters to be updated
 * @param inisky Loaded INI settings for the desired sky
 * @param name   Human-readable name of the sky parameter set
 */
static void InitPerezParameters (SPerezParameters* p,
                                 CIniFile *inisky,
                                 const char* name)
{
  // Initialize to defaults
  memset (p, 0, sizeof(SPerezParameters));

  // Initialize name
  strcpy (p->name, name);

  // Initialize zenith parameters
  inisky->Get (name, "zenith_Y_A", &(p->Y_zenith.A));
  inisky->Get (name, "zenith_Y_B", &(p->Y_zenith.B));
  inisky->Get (name, "zenith_Y_C", &(p->Y_zenith.C));
  inisky->Get (name, "zenith_Y_D", &(p->Y_zenith.D));

  inisky->Get (name, "zenith_x_theta3_T2", &(p->x_zenith.t2[3]));
  inisky->Get (name, "zenith_x_theta3_T1", &(p->x_zenith.t1[3]));
  inisky->Get (name, "zenith_x_theta3_T0", &(p->x_zenith.t0[3]));
  inisky->Get (name, "zenith_x_theta2_T2", &(p->x_zenith.t2[2]));
  inisky->Get (name, "zenith_x_theta2_T1", &(p->x_zenith.t1[2]));
  inisky->Get (name, "zenith_x_theta2_T0", &(p->x_zenith.t0[2]));
  inisky->Get (name, "zenith_x_theta1_T2", &(p->x_zenith.t2[1]));
  inisky->Get (name, "zenith_x_theta1_T1", &(p->x_zenith.t1[1]));
  inisky->Get (name, "zenith_x_theta1_T0", &(p->x_zenith.t0[1]));
  inisky->Get (name, "zenith_x_theta0_T2", &(p->x_zenith.t2[0]));
  inisky->Get (name, "zenith_x_theta0_T1", &(p->x_zenith.t1[0]));
  inisky->Get (name, "zenith_x_theta0_T0", &(p->x_zenith.t0[0]));

  inisky->Get (name, "zenith_y_theta3_T2", &(p->y_zenith.t2[3]));
  inisky->Get (name, "zenith_y_theta3_T1", &(p->y_zenith.t1[3]));
  inisky->Get (name, "zenith_y_theta3_T0", &(p->y_zenith.t0[3]));
  inisky->Get (name, "zenith_y_theta2_T2", &(p->y_zenith.t2[2]));
  inisky->Get (name, "zenith_y_theta2_T1", &(p->y_zenith.t1[2]));
  inisky->Get (name, "zenith_y_theta2_T0", &(p->y_zenith.t0[2]));
  inisky->Get (name, "zenith_y_theta1_T2", &(p->y_zenith.t2[1]));
  inisky->Get (name, "zenith_y_theta1_T1", &(p->y_zenith.t1[1]));
  inisky->Get (name, "zenith_y_theta1_T0", &(p->y_zenith.t0[1]));
  inisky->Get (name, "zenith_y_theta0_T2", &(p->y_zenith.t2[0]));
  inisky->Get (name, "zenith_y_theta0_T1", &(p->y_zenith.t1[0]));
  inisky->Get (name, "zenith_y_theta0_T0", &(p->y_zenith.t0[0]));

  // Initialize curve parameters
  inisky->Get (name, "perez_Y_A_T1", &(p->Y_curve.A[1]));
  inisky->Get (name, "perez_Y_A_T0", &(p->Y_curve.A[0]));
  inisky->Get (name, "perez_Y_B_T1", &(p->Y_curve.B[1]));
  inisky->Get (name, "perez_Y_B_T0", &(p->Y_curve.B[0]));
  inisky->Get (name, "perez_Y_C_T1", &(p->Y_curve.C[1]));
  inisky->Get (name, "perez_Y_C_T0", &(p->Y_curve.C[0]));
  inisky->Get (name, "perez_Y_D_T1", &(p->Y_curve.D[1]));
  inisky->Get (name, "perez_Y_D_T0", &(p->Y_curve.D[0]));
  inisky->Get (name, "perez_Y_E_T1", &(p->Y_curve.E[1]));
  inisky->Get (name, "perez_Y_E_T0", &(p->Y_curve.E[0]));

  inisky->Get (name, "perez_x_A_T1", &(p->x_curve.A[1]));
  inisky->Get (name, "perez_x_A_T0", &(p->x_curve.A[0]));
  inisky->Get (name, "perez_x_B_T1", &(p->x_curve.B[1]));
  inisky->Get (name, "perez_x_B_T0", &(p->x_curve.B[0]));
  inisky->Get (name, "perez_x_C_T1", &(p->x_curve.C[1]));
  inisky->Get (name, "perez_x_C_T0", &(p->x_curve.C[0]));
  inisky->Get (name, "perez_x_D_T1", &(p->x_curve.D[1]));
  inisky->Get (name, "perez_x_D_T0", &(p->x_curve.D[0]));
  inisky->Get (name, "perez_x_E_T1", &(p->x_curve.E[1]));
  inisky->Get (name, "perez_x_E_T0", &(p->x_curve.E[0]));

  inisky->Get (name, "perez_y_A_T1", &(p->y_curve.A[1]));
  inisky->Get (name, "perez_y_A_T0", &(p->y_curve.A[0]));
  inisky->Get (name, "perez_y_B_T1", &(p->y_curve.B[1]));
  inisky->Get (name, "perez_y_B_T0", &(p->y_curve.B[0]));
  inisky->Get (name, "perez_y_C_T1", &(p->y_curve.C[1]));
  inisky->Get (name, "perez_y_C_T0", &(p->y_curve.C[0]));
  inisky->Get (name, "perez_y_D_T1", &(p->y_curve.D[1]));
  inisky->Get (name, "perez_y_D_T0", &(p->y_curve.D[0]));
  inisky->Get (name, "perez_y_E_T1", &(p->y_curve.E[1]));
  inisky->Get (name, "perez_y_E_T0", &(p->y_curve.E[0]));
}

/*
 * Application default sky parameters
 */
SPerezParameters DefaultPerezParameters =
{
  "Default",

  // Y distribution
  {
    {-1.4630f, +0.1787f}, // A
    {+0.4275f, -0.3554f}, // B
    {+5.3251f, -0.0227f}, // C
    {-2.5771f, +0.1206f}, // D
    {+0.3703f, -0.0670f}, // E
  },

  // x distribution
  {
    {-0.2592f, -0.0193f}, // A
    {+0.0008f, -0.0665f}, // B
    {+0.2125f, -0.0004f}, // C
    {-0.8989f, -0.0641f}, // D
    {+0.0452f, -0.0033f}, // E
  },

  // y distribution
  {
    {-0.2608f, -0.0167f}, // A
    {+0.0092f, -0.0950f}, // B
    {+0.2102f, -0.0079f}, // C
    {-1.6537f, -0.0441f}, // D
    {+0.0529f, -0.0109f}, // E
  },

  // Y zenith
  {
    +4.0453f,    // A
    -4.9710f,    // B
    -0.2155f,    // C
    +2.4192f,    // D
  },

  // x zenith
  {
    //   1        theta      theta^2   theta^3
    {+0.00000f, +0.00209f, -0.00375f, +0.00166f}, // T^2
    {+0.00394f, -0.03202f, +0.06377f, -0.02903f}, // T
    {+0.25886f, +0.06052f, -0.21196f, +0.11693f}, // 1
  },

  // y zenith
  {
    //   1        theta     theta^2    theta^3
    {+0.00000f, +0.00317f, -0.00610f, +0.00275f}, // T^2
    {+0.00516f, -0.04153f, +0.08970f, -0.04214f}, // T
    {+0.26688f, +0.06670f, -0.26756f, +0.15346f}, // 1
  }
};

CSkyDomeImage::CSkyDomeImage (double distance)
{
  int i, j;
  
  // Add default Perez parameters
  SPerezParameters *pDefault = new SPerezParameters;
  memcpy (pDefault, &DefaultPerezParameters, sizeof(SPerezParameters));
  perez[DefaultPerezParameters.name] = pDefault;

  //
  // Load Perez parameter sets from SKIES.INI
  //
  CIniFile *inisky = new CIniFile;
  if (inisky->Load ("System/Skies.ini") != 0) {
    // SKIES.INI could be read; parse parameters from INI settings
    int nSkies = inisky->GetNumSections();
    for (i=0; i<nSkies; i++) {
      SPerezParameters *p = new SPerezParameters;
      memcpy (p, &DefaultPerezParameters, sizeof(SPerezParameters));
      char* skyname = inisky->GetSectionName (i);
      InitPerezParameters (p, inisky, skyname);
      perez[skyname] = p;
    }
  }
  delete inisky;

  // Get default sky name from INI settings
  perezCurrent = &DefaultPerezParameters;
  char skyname[64];
  strcpy (skyname, "");
  GetIniString ("Graphics", "skyDefaultName", skyname, 64);
  if (strlen (skyname) > 0) {
    // Parameter was specified, search available skies for the default
    std::map<string,SPerezParameters*>::iterator i = perez.find(skyname);
    if (i != perez.end()) {
      perezCurrent = i->second;
    }
  }

  //
  // Generate raw vertex data
  //
  // The sky dome vertex raw data is made up of a single zenith vertex, plus
  //   SKYDOME_STACKS "rings" of SKYDOME_SLICES vertices in each ring.  The top of
  //   the sky dome is constructed of a triangle fan between the zenith
  //   vertex and each of the vertices in the top ring.  The remainder of the
  //   dome is constructed of triangle strips spanning each successive vertex ring.
  //

  // Allocate array for dome vertices
  // This array is used for both the zenith triangle fan, and for the triangle strips
  //   for the remainder of the dome.
  // Array size is 1 (for zenith) + strips of SKYDOME_SLICES vertices to define the
  //   top and bottom of each strip, i.e. (SKYDOME_STACKS + 1)
  nDomeVertices = ((SKYDOME_STACKS + 1) * SKYDOME_SLICES) + 1;
  domeVertex = new GLfloat[3 * nDomeVertices];

  // First vertex is the zenith
  domeVertex[0] = 0;
  domeVertex[1] = 0;
  domeVertex[2] = 1.0f;

  // Iterate over all vertices of all rings
  double thetaExtent = (SGD_PI / 2.0) * SKYDOME_HEMISPHERE_SCALE;
  double dTheta = thetaExtent / SKYDOME_STACKS;
  double dPhi = (SGD_PI * 2.0) / SKYDOME_SLICES;
  int iVertex = 1;
  for (i=0; i<SKYDOME_STACKS+1; i++) {
    // Calculate zenith angle theta for all vertices in this ring
    double theta = (i+1) * dTheta;
    for (j=0; j<SKYDOME_SLICES; j++) {
      // Calculate azimuth angle phi for this vertex, in radians E of N
      double phi = (j * dPhi);
      // Save zenith angle and azimuth at this point
      sgSetVec2 (vtx_ring_angles[i][j], (float)theta, (float)phi);
      // Calculate vertex coordinates
      domeVertex[iVertex*3]   = (float)(sin(phi) * sin(theta));    // X-coordinate
      domeVertex[iVertex*3+1] = (float)(cos(phi) * sin(theta));    // Y-coordinate
      domeVertex[iVertex*3+2] = (float)(cos(theta));               // Z-coordinate
      iVertex++;
    }
  }

  // Create colour array and initialize as if it were full night; first call to
  //   Repaint() will set vertex colours appropriately
  domeColour = new GLfloat[3 * nDomeVertices];
  memset (domeColour, 0, 3 * nDomeVertices * sizeof(float));
  night = true;

  // Create element array for rendering zenith fan
  nFanElements = SKYDOME_SLICES + 2;
  fanElements = new GLuint[nFanElements];
  fanElements[0] = 0;
  for (int i=0; i<SKYDOME_SLICES; i++) {
    fanElements[i+1] = i+1;
  }
  fanElements[SKYDOME_SLICES+1] = 1;

  // Create element array for rendering remainder of dome
  nDomeElements = SKYDOME_STACKS * 2 * (SKYDOME_SLICES + 1) + 2 * (SKYDOME_STACKS - 1);
  domeElements = new GLuint[nDomeElements];
  int iNext = 0;
  for (i=0; i<SKYDOME_STACKS; i++) {
    // Calculate base vertex indices for the upper and lower bounds of this stack
    GLuint upperBase = i * SKYDOME_SLICES + 1;
    GLuint lowerBase = (i+1) * SKYDOME_SLICES + 1;
    // If not the first stack, duplicate first element to concatenate with previous
    if (i != 0) {
      domeElements[iNext++] = upperBase;
    }
    // Send upper-left and lower-left vertices
    for (j=0; j<SKYDOME_SLICES; j++) {
      domeElements[iNext++] = upperBase + j;
      domeElements[iNext++] = lowerBase + j;
    }
    domeElements[iNext++] = upperBase;
    domeElements[iNext++] = lowerBase;
    // If not the last stack, duplicate last element to concatenate with next
    if (i != SKYDOME_STACKS-1) {
      domeElements[iNext++] = lowerBase + (SKYDOME_SLICES-1);
    }
  }

  // Initialize repaint solar zenith angle
  this->distance = distance;
  prevTheta = 0;
  prevPhi = 0;
}

CSkyDomeImage::~CSkyDomeImage (void)
{
  std::map<string,SPerezParameters*>::iterator i;
  for (i=perez.begin(); i!=perez.end(); i++) delete (*i).second;
	perez.clear();
  SAFE_DELETE_ARRAY (domeVertex);
  SAFE_DELETE_ARRAY (domeColour);
  SAFE_DELETE_ARRAY (fanElements);
  SAFE_DELETE_ARRAY (domeElements);
}

/**
 * Repaint the sky dome
 *
 * @param solTheta Zenith angle of the sun in radians
 * @param solPhi   Azimuth angle of the sun in radians west of south
 */
void CSkyDomeImage::Repaint (double solTheta, double solPhi)
{
  int i, j;
  double thetaThreshold = DegToRad (0.01);
  double phiThreshold = DegToRad (0.01);

  if ((fabs(solTheta - prevTheta) < thetaThreshold) &&
      (fabs(solPhi - prevPhi) < phiThreshold))
  {
    // Sun angle has not changed significantly; nothing to do
    return;
  }
  prevTheta = solTheta;
  prevPhi = solPhi;

  //
  // Perez sky shading is intended for day/dusk only.  Set dome colour to
  //   all black, these values will be overwritten with Perez-derived
  //   colours if the sun is above astronomical night
  float nightTheta = DegToRad (108.0f);   // -18 deg solar elevation

  //
  // Step 1 : Calculate new sky dome colours
  //

  if (solTheta > nightTheta) {
    // Sun is below level of astronomical night
    if (!night) {
      // Sun was not below astronomical night at last repaint; fill colour array with 0's
      memset (domeColour, 0, 3 * sizeof(float) * nDomeVertices);
      night = true;
    }
  } else {
    // Sun is above level of astronomical night...use Perez equations to compute
    //   zenith and dome vertex colours
    night = false;

    /// @todo Reasonable value is assumed for turbidity, should be based on visibility
    float turbidity = 4.0;
    float r = 0, g = 0, b = 0;

    // Calculate distribution coefficients
    SPerezCoefficients cY, cx, cy;
    CalcPerezCoefficients (turbidity, perezCurrent->Y_curve, cY);
    CalcPerezCoefficients (turbidity, perezCurrent->x_curve, cx);
    CalcPerezCoefficients (turbidity, perezCurrent->y_curve, cy);

    // Calculate zenith luminance and chrominance
    float Yz = ZenithLuminance ((float)solTheta, turbidity, perezCurrent->Y_zenith);
    float xz = ZenithChrominance ((float)solTheta, turbidity, perezCurrent->x_zenith);
    float yz = ZenithChrominance ((float)solTheta, turbidity, perezCurrent->y_zenith);

    // Calculate Perez function zenith values for later use
    float Ypz = PerezFunction (0, (float)solTheta, cY);
    float xpz = PerezFunction (0, (float)solTheta, cx);
    float ypz = PerezFunction (0, (float)solTheta, cy);

    // Convert to RGB
    YxyToRGB (Yz, xz, yz, &r, &g, &b);

    // Set zenith colour
    domeColour[0] = r;
    domeColour[1] = g;
    domeColour[2] = b;

    // Calculate sun vector; adjust azimuth to be north-based
    sgVec3 vSun;
    sgSetVec3 (vSun, (float)(sin(solTheta) * sin(solPhi)),
                     (float)(sin(solTheta) * cos(solPhi)),
                     (float)(cos(solTheta)));
    sgNormalizeVec3 (vSun);

    // Iterate over all dome vertices
    int iVertex = 1;
    for (i=0; i<SKYDOME_STACKS+1; i++) {
      for (j=0; j<SKYDOME_SLICES; j++) {
        float domeTheta = vtx_ring_angles[i][j][0];
        float domePhi = vtx_ring_angles[i][j][1];

        // Calcualate gamma angle for this point on the dome
        sgVec3 vDome;
        sgSetVec3 (vDome, (float)(sin(domeTheta) * sin(domePhi)),
                          (float)(sin(domeTheta) * cos(domePhi)),
                          (float)(cos(domeTheta)));
        sgNormalizeVec3 (vDome);
        float gamma = DegToRad (sgAngleBetweenVec3 (vSun, vDome));

        // Calculate dome luminance and chrominance
        float Yd = Yz * PerezFunction (domeTheta, gamma, cY) / Ypz;
        float xd = xz * PerezFunction (domeTheta, gamma, cx) / xpz;
        float yd = yz * PerezFunction (domeTheta, gamma, cy) / ypz;

        // Convert to RGB
        YxyToRGB (Yd, xd, yd, &r, &g, &b);

        // Update colour array entry
        domeColour[iVertex*3]   = r;
        domeColour[iVertex*3+1] = g;
        domeColour[iVertex*3+2] = b;
        iVertex++;
      }
    }
  }
}

/**
 * Determine the current colour of the sky dome at a particular coordinate
 *
 * @param theta Zenith angle of the sky dome position in radians
 * @param phi   Azimuth angle of the sky dome position in radians
 * @param r     Updated with the red component of the sky colour, in range [0.0, 1.0]
 * @param g     Updated with the green component of the sky colour, in range [0.0, 1.0]
 * @param b     Updated with the blue component of the sky colour, in range [0.0, 1.0]
 */
void CSkyDomeImage::GetDomeColour (float theta, float phi, float &r, float &g, float &b)
{
  // Set default colour
  r = g = b = 0.0f;

  // Convert phi index of vertex slice that precedes phi around the azimuth
  float dPhi = (SG_PI * 2.0f) / (float)SKYDOME_SLICES;
  float fSlice = floor(phi / dPhi);
  float s = (phi - (fSlice * dPhi)) / dPhi;
  int slice = (int)(fSlice);

  // Convert phi to a pair of CW/CCW vertex slice indices
  float thetaExtent = (SG_PI / 2.0f) * (float)SKYDOME_HEMISPHERE_SCALE;
  float dTheta = thetaExtent / SKYDOME_STACKS;
  float fStack = floor(theta / dTheta);
  float t = (theta - (fStack * dTheta)) / dTheta;
  int stack = (int)(fStack);

  // Get the colours of the sky dome vertices surrounding the desired point C,
  //   and calculate the parameters s and t for bilinear interpolation
  //
  // stack-1 c10 -------- A ----- c11
  //          | <-- s --> C       |
  //          |           |       |
  //          |         t |       |
  // stack   c00 -------- B ---- c01
  //
  //        slice              slice+1
  //

  // Linearly interpolate point A between vertices of upper row (stack-1)
  int indexA = ((stack-1) * SKYDOME_SLICES) + slice;
  sgVec3 A;
  sgCopyVec3 (A, &domeColour[indexA*3]);
  sgScaleVec3 (A, (1.0-s));
  sgAddScaledVec3 (A, &domeColour[(indexA+1)*3], s);

  // Linearly interpolate point B between vertices of lower row (stack)
  int indexB = (stack * SKYDOME_SLICES) + slice;
  sgVec3 B;
  sgCopyVec3 (B, &domeColour[indexB*3]);
  sgScaleVec3 (B, (1.0-s));
  sgAddScaledVec3 (B, &domeColour[(indexB+1)*3], s);

  // Linearly interpolate between points A and B
  sgVec3 C;
  sgCopyVec3 (C, B);
  sgScaleVec3 (C, (1.0-t));
  sgAddScaledVec3 (C, A, t);

  r = C[0];
  g = C[1];
  b = C[2];
}

/**
 * Draw the sky dome image
 */
void CSkyDomeImage::DrawDome (void)
{
  // Set up GL state for rendering
  glPushAttrib (GL_LIGHTING_BIT);
  glShadeModel (GL_SMOOTH);
  glEnableClientState (GL_VERTEX_ARRAY);
  glEnableClientState (GL_COLOR_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  // Bind vertex and colour arrays
  glVertexPointer (3, GL_FLOAT, 0, domeVertex);
  glColorPointer (3, GL_FLOAT, 0, domeColour);

  // Render zenith triangle fan
  glDrawElements (GL_TRIANGLE_FAN, nFanElements, GL_UNSIGNED_INT, fanElements);

  // Render dome triangle strips
  glDrawElements (GL_TRIANGLE_STRIP, nDomeElements, GL_UNSIGNED_INT, domeElements);

  // Restore GL state
  glDisableClientState (GL_VERTEX_ARRAY);
  glDisableClientState (GL_COLOR_ARRAY);
  glPopAttrib ();
}

void CSkyDomeImage::Draw (void)
{
  // Transform modelview matrix for rendering sky dome
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  glScalef (distance, distance, distance);
  glDisable(GL_TEXTURE_2D);

  // Draw sky dome
  DrawDome ();

  // Restore GL context
  glMatrixMode (GL_MODELVIEW);
  glPopMatrix ();
}

//
// Dump sky dome attributes to a file for debugging
//
// This is called from the parent CSkyManager class
//
void CSkyDomeImage::Print (FILE *f)
{
  /// \todo  Print currently selected Perez parameter set

  // Display Perez parameter sets
  std::map<string,SPerezParameters*>::iterator i;
  for (i=perez.begin(); i!=perez.end(); i++) {
    SPerezParameters* p = i->second;

    fprintf (f, "Perez parameter set : %s\n", p->name);
    fprintf (f, "  Y (Luminance) curve parameters:\n");
    fprintf (f, "    A = %10.8f T + %10.8f\n", p->Y_curve.A[1], p->Y_curve.A[0]);
    fprintf (f, "    B = %10.8f T + %10.8f\n", p->Y_curve.B[1], p->Y_curve.B[0]);
    fprintf (f, "    C = %10.8f T + %10.8f\n", p->Y_curve.C[1], p->Y_curve.C[0]);
    fprintf (f, "    D = %10.8f T + %10.8f\n", p->Y_curve.D[1], p->Y_curve.D[0]);
    fprintf (f, "    E = %10.8f T + %10.8f\n", p->Y_curve.E[1], p->Y_curve.E[0]);

    fprintf (f, "  x (Chrominance) curve parameters:\n");
    fprintf (f, "    A = %10.8f T + %10.8f\n", p->x_curve.A[1], p->x_curve.A[0]);
    fprintf (f, "    B = %10.8f T + %10.8f\n", p->x_curve.B[1], p->x_curve.B[0]);
    fprintf (f, "    C = %10.8f T + %10.8f\n", p->x_curve.C[1], p->x_curve.C[0]);
    fprintf (f, "    D = %10.8f T + %10.8f\n", p->x_curve.D[1], p->x_curve.D[0]);
    fprintf (f, "    E = %10.8f T + %10.8f\n", p->x_curve.E[1], p->x_curve.E[0]);

    fprintf (f, "  y (Chrominance) curve parameters:\n");
    fprintf (f, "    A = %10.8f T + %10.8f\n", p->y_curve.A[1], p->y_curve.A[0]);
    fprintf (f, "    B = %10.8f T + %10.8f\n", p->y_curve.B[1], p->y_curve.B[0]);
    fprintf (f, "    C = %10.8f T + %10.8f\n", p->y_curve.C[1], p->y_curve.C[0]);
    fprintf (f, "    D = %10.8f T + %10.8f\n", p->y_curve.D[1], p->y_curve.D[0]);
    fprintf (f, "    E = %10.8f T + %10.8f\n", p->y_curve.E[1], p->y_curve.E[0]);

    fprintf (f, "  Y (Luminance) zenith parameters:\n");
    fprintf (f, "    A = %10.8f\n", p->Y_zenith.A);
    fprintf (f, "    B = %10.8f\n", p->Y_zenith.B);
    fprintf (f, "    C = %10.8f\n", p->Y_zenith.C);
    fprintf (f, "    D = %10.8f\n", p->Y_zenith.D);

    fprintf (f, "  x (Chrominance) zenith parameters:\n");
    fprintf (f, "      theta^3 (%10.8f T^2 + %10.8f T + %10.8f\n",
      p->x_zenith.t2[3], p->x_zenith.t1[3], p->x_zenith.t0[3]);
    fprintf (f, "    + theta^2 (%10.8f T^2 + %10.8f T + %10.8f\n",
      p->x_zenith.t2[2], p->x_zenith.t1[2], p->x_zenith.t0[2]);
    fprintf (f, "    + theta   (%10.8f T^2 + %10.8f T + %10.8f\n",
      p->x_zenith.t2[1], p->x_zenith.t1[1], p->x_zenith.t0[1]);
    fprintf (f, "    +         (%10.8f T^2 + %10.8f T + %10.8f\n",
      p->x_zenith.t2[0], p->x_zenith.t1[0], p->x_zenith.t0[0]);

    fprintf (f, "  y (Chrominance) zenith parameters:\n");
    fprintf (f, "      theta^3 (%10.8f T^2 + %10.8f T + %10.8f\n",
      p->y_zenith.t2[3], p->y_zenith.t1[3], p->y_zenith.t0[3]);
    fprintf (f, "    + theta^2 (%10.8f T^2 + %10.8f T + %10.8f\n",
      p->y_zenith.t2[2], p->y_zenith.t1[2], p->y_zenith.t0[2]);
    fprintf (f, "    + theta   (%10.8f T^2 + %10.8f T + %10.8f\n",
      p->y_zenith.t2[1], p->y_zenith.t1[1], p->y_zenith.t0[1]);
    fprintf (f, "    +         (%10.8f T^2 + %10.8f T + %10.8f\n",
      p->y_zenith.t2[0], p->y_zenith.t1[0], p->y_zenith.t0[0]);

    fprintf (f, "\n");
  }

  int iv, jv;

  // Print sky dome vertices
  fprintf (f, "Sky Dome Vertices\n");
  fprintf (f, "  Zenith:\t%f,%f,%f\n", domeVertex[0], domeVertex[1], domeVertex[2]);
  int iVertex = 1;
  for (iv=0; iv<SKYDOME_STACKS+1; iv++) {
    fprintf (f, "  Ring %d\n", iv);
    for (jv=0; jv<SKYDOME_SLICES; jv++) {
      int base = iVertex * 3;
      fprintf (f, "    Vertex %d\t%8.6f,%8.6f,%8.6f\n",
        jv, domeVertex[base], domeVertex[base+1], domeVertex[base+2]);
      iVertex++;
    }
  }

  // Print sky dome colours
  fprintf (f, "Sky Dome Colours\n");
  fprintf (f, "  Zenith:\t%f,%f,%f\n", domeColour[0], domeColour[1], domeColour[2]);
  iVertex = 1;
  for (iv=0; iv<SKYDOME_STACKS+1; iv++) {
    fprintf (f, "  Ring %d\n", iv);
    for (jv=0; jv<SKYDOME_SLICES; jv++) {
      int base = iVertex * 3;
      fprintf (f, "    Vertex %d\t%8.6f,%8.6f,%8.6f\n",
        jv, domeColour[base], domeColour[base+1], domeColour[base+2]);
      iVertex++;
    }
  }

  // Print sky dome angles
  fprintf (f, "Sky Dome Angles\n");
  for (iv=0; iv<SKYDOME_STACKS+1; iv++) {
    fprintf (f, "  Ring %d\n", iv);
    for (jv=0; jv<SKYDOME_SLICES; jv++) {
      fprintf (f, "    Vertex %d\t%8.6f,%8.6f\n",
        jv, vtx_ring_angles[iv][jv][0], vtx_ring_angles[iv][jv][1]);
    }
  }
}

