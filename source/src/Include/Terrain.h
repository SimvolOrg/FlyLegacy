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
//===================================================================================
class CSceneryDBM;
//==================================================================================
//	Class CSceneryPOD to hold a scenery POD associated to a QGT or a GBT
//==================================================================================
class CSceneryPOD {
	//--- ATTRIBUTES --------------------------------------------
protected:
	int			users;											// User count
	U_INT		Key;												// QGT key
	char    fName[PATH_MAX];						/// Pod Full name
	//--- Methods -----------------------------------------------
public:
	CSceneryPOD(U_INT key, char *fn);
	//------------------------------------------------------------
	void		Mount();
	void		Remove();
	//------------------------------------------------------------
	inline U_INT GetKey()							{return Key;}
  inline char *GetName()						{return fName;}
	inline void  IncUser()						{users++;}
};
//==================================================================================
//	Class CSceneryPack to hold all scenery associated to a QGT
//	A list of ScenerySet is maintain in this Pack
//==================================================================================
class CSceneryPack {
	friend class CSceneryDBM;
private:
	//--- ATTRIBUTES ------------------------------------
	U_INT key;												// QGT key
	U_INT	gx;													// X index
	U_INT gz;													// Z index
	std::vector<CSceneryPOD*> apod;
	//----------------------------------------------------
public:
	CSceneryPack(U_INT key);
 ~CSceneryPack();
  //-----------------------------------------------------
	void	AddPod(CSceneryPOD *pod) {apod.push_back(pod);}
	//-----------------------------------------------------
	void	MountPODs (CSceneryDBM *dbm);
	void	RemovePODs(CSceneryDBM *dbm);
	};
//==================================================================================
// CSceneryDBM
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
class CSceneryDBM {
	friend class CSceneryPack;
private:
  static CSceneryDBM instance;

public:
  // Return a reference to the singleton instance
  static CSceneryDBM& Instance() { return instance; }
  // CSceneryDBM methods
  void    Init (void);
  void    Cleanup (void);
	void		AddPodToQGT(CSceneryPOD *pod);
	void		AddPodToGBT(CSceneryPOD *pod);
	int		  CheckForScenery(PFSPODFILE *p);
	//-----------------------------------------------------------------------
	void		MountAll();
  void    Register(U_INT key);
	void    Deregister(U_INT key);
  //  Load initial files --------------------------------------------------
protected:
  void    LoadInFolderTree (const char *path);
	void		Warn01(char *fn);
	void		SendPOD(char *pn);
	//------------------------------------------------------------------------
	int     CheckDatabase(char *pn);
	void		LookForPOD(char *path);
	void		ProcessPOD(char *path,char *fn);
	int			GetSceneryType(char *path);
	int 		SceneryForGBT(PFSPODFILE *p,int gx,int gz);
  //------------------------------------------------------------------------
protected:
	PFS *pfs;
	U_CHAR	tr;													// Trace indicator
	int		  exp;
	char podn[PATH_MAX];
	std::map<U_INT,CSceneryPack*> sqgt;
	std::map<U_INT,CSceneryPack*> sgbt;
};


//================END OF FILE ===========================================================
#endif // TERRAIN_H
