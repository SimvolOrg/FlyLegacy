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
#include "FlyLegacy.h"
#include "Utility.h"
#include "LogFile.h"
#include "3dMath.h"
#include <pthread.h>
#include <vector>
#include <queue>
#include <string>
#include <set>
#include <list>
#include <map>
///--------------------JS------------------------------------------------------------------
//
// Elevation Shader
//
// The elevation shader is a simple singleton service class that returns an RGB
//   colour given a terrain elevation value.  This can be used for untextured
//   terrain to aid debugging
//
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


//===========================================================================
// CScenerySet
//
// Implementation : ScenerySet.cpp
//
// A CScenerySet encompasses a chunk of sliced scenery of any arbitrary size.
//   The stream file that defines a scenery set is located in a sub-directory
//   of any \Scenery folder.
//  JS: As there is only one scenery file per QGT, might as well register each
//      scenery in a map with the QGT coordinates as a key
//============================================================================
class CScenerySet : public CStreamObject {
public:
  // Constructors / Destructor
  CScenerySet (const char* scfFolder, const char* scfFilename);
  ~CScenerySet (void);

  // CStreamObject methods
  int Read (SStream *stream, Tag tag);

  // CScenerySet methods
  bool    IsLoaded (void);
  void    Load (char tr);
  void    Unload (void);
  int     GetRefCount (void);
  void    IncRefCount (void);
  void    DecRefCount (void);
	//-------------------------------------------------------------------------------
	bool		SameKey(U_INT k)	{return (k == Key);}
	//-------------------------------------------------------------------------------
	inline  U_INT GetKey()		{return Key;}
  //-------------------------------------------------------------------------------
protected:
  U_INT                     Key;                       ///< Scenery QGT key
  char                      name[80];                  ///< Descriptive name
  SPosition                 call, caur;                ///< Coverage area lower-left / upper-right
  SPosition                 ldll, ldur;                ///< Load area lower-left / upper-right
  std::vector<std::string>  podList;                   ///< List of POD names
  char                      scfFolder[PATH_MAX];       ///< SCF folder name
  char                      scfFilename[FILENAME_MAX]; ///< SCF filename
  bool                      loaded;                    ///< Whether scenery set is loaded
  int                       refCount;                  ///< Number of references
};
//==================================================================================
//	Class CSceneryPack to hold all scenery associated to a QGT
//==================================================================================
class CSceneryPack {
private:
	//--- ATTRIBUTES ------------------------------------
	U_INT key;												// QGT key
	std::vector<CScenerySet*> pack;
	//----------------------------------------------------
public:
	CSceneryPack(U_INT key);
 ~CSceneryPack();
  //-----------------------------------------------------
  void	AddSet(CScenerySet *scn) {pack.push_back(scn);}
	void	Load(char t);
	void  Unload();
	};
//==================================================================================
// CScenerySetDatabase
//
// The scenery set database contains information about sliced scenery areas read
//   from .SCF (Scenery Control Files).  Each SCF is encapsulated by a CScenerySet
//   instance and represents a rectangular area of contiguous sliced scenery.
//
// This is a singleton class.
//
//  JS: As one scenery file is located in a QGT, might as well register each
//      scenery in a map with the QGT coordinates as a key
//============================================================================
	class CScenerySetDatabase {
private:
  static CScenerySetDatabase instance;

public:
  // Return a reference to the singleton instance
  static CScenerySetDatabase& Instance() { return instance; }

  // CScenerySetDatabase methods
  void    Init (void);
  void    Cleanup (void);
	void		AddSet(CScenerySet *scn);
	//-----------------------------------------------------------------------
  void    Register(U_INT key, char tr);
	void    Deregister(U_INT key);
  //  Load initial files --------------------------------------------------
protected:
  void    LoadInFolder (const char *path);
  void    LoadInFolderTree (const char *path);
  //------------------------------------------------------------------------
protected:
	U_CHAR	tr;													// Trace indicator
	std::map<U_INT,CSceneryPack*> scene;
};


//================END OF FILE ===========================================================
#endif // TERRAIN_H
