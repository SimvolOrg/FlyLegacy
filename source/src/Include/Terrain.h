/*
 * Terrain.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
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

/*! \file Terrain.h
 *  \brief Declarations related to terrain rendering
 *
 * These classes are used by the Scenery Manager system to render terrain
 *   based on either TerraScene generated scenery or a default terrain.
 */

#ifndef TERRAIN_H
#define TERRAIN_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <stdarg.h>
#include <map>
///--------------------JS------------------------------------------------------------------
//
// Elevation Shader
//
// The elevation shader is a simple singleton service class that returns an RGB
//   colour given a terrain elevation value.  This can be used for untextured
//   terrain to aid debugging
//
/*
class CElevationShader {
private:
  static CElevationShader  instance;
  CElevationShader();

public:
  // CElevationShader methods
  static CElevationShader&  Instance() { return instance; }
  void                      Init (void);
  void                      GetElevationRGB (const float elev, float &r, float &g, float &b);

protected:
  float         relief[256][3];         ///< Array of RGB values for elevation ranges
  float         maxElevation;           ///< Maximum distinct elevation
  int           negativeIndex;          ///< Index for all negative elevations
  int           zeroIndex;              ///< Index for zero (sea) elevations
  int           baseIndex;              ///< First index of land elevations
  int           maxIndex;               ///< Last index of land elevations
};
*/
//================END OF FILE ===========================================================
#endif // TERRAIN_H
