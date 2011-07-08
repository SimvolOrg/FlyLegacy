/*
 * Terrain.cpp
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


#include "../Include/Globals.h"
#include "../Include/Terrain.h"
#include "../Include/Utility.h"
#include "../Include/Ui.h"
#include "../Include/Endian.h"

using namespace std;

//----------------------JS------------------------------------------------
//
// CElevationShader
//
CElevationShader    CElevationShader::instance;

CElevationShader::CElevationShader (void)
{
  // Initialize RGB array to default greyscale
  for (int i=0; i<256; i++) {
    for (int j=0; j<3; j++) {
      relief[i][j] = (float)(256-i) / 256.0f;
    }
  }

  // Initialize control indices
  negativeIndex = 0;      // Palette index for negative elevations
  zeroIndex = 1;          // Palette index corresponding to zero elev
  maxElevation = 15000;   // Maximum positive elevation in feet
  baseIndex = 48;         // Palette index of lowest positive elevation
  maxIndex = 256;         // Palette index of highest elevation
}

void CElevationShader::Init (void)
{
  // Load .act (palette) for relief shading
  PODFILE *pAct = popen (&globals->pfs, "Art/Topographic.act");
  if (pAct != NULL) {
    // Read palette data and close the POD file
    unsigned char data[256][3];
    pread (data, 256 * 3, 1, pAct);
    pclose (pAct);

    // Convert to float and copy to RGB colour array
    for (int i=0; i<256; i++) {
      for (int j=0; j<3; j++) {
        relief[i][j] = (float)(data[i][j]) / 256;
      }
    }
  } else {
    // Could not open palette for elevation shading
    WARNINGLOG ("CElevationShader : Cannot load topograpic shading palette");
  }
}

void CElevationShader::GetElevationRGB (const float elev, float &r, float &g, float &b)
{
  static const float zeroEpsilon = 3.0;

  // Calculate index
  int i = 0;
  if ((elev <= zeroEpsilon) && (elev >= -zeroEpsilon)) {
    // Elevation is considered to be zero
    i = zeroIndex;
  } else if (elev < 0) {
    // Negative elevations use a single colour
    i = negativeIndex;
  } else if (elev > maxElevation) {
    i = maxIndex;
  } else {
    // Positive elevation
    float d = 1 - ((maxElevation - elev) / maxElevation);
    int offset = (int)((float)(maxIndex - baseIndex) * d);
    i = baseIndex + offset;
  }

  // Assign colours
  r = relief[i][0];
  g = relief[i][1];
  b = relief[i][2];
}

//================END OF FILE ======================================================
