//==============================================================================================
// ElevationTracker.h
//
// Part of Fly! Legacy project
//
// Copyright 2003 Chris Wallace
// Copyright 2005 Jean Sabatier
// Fly! Legacy is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
// Fly! Legacy is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
//   along with Fly! Legacy; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//================================================================================================
#ifndef ELEVATION_TRACKER_H
#define ELEVATION_TRACKER_H
//================================================================================================
#include "../Include/FlyLegacy.h"
#include "../Include/Geomath.h"
#include "../Include/TerrainData.h"
#include "../Include/Cameras.h"
//================================================================================================

//================================================================================================
class CFuiTED;
class CmQUAD;

//============================================================================
//	Classe for elevation tracker
//============================================================================
class CElvTracker: public Tracker {
	//--- ATTRIBUTES ------------------------------------
	char						 upd;									// Update indicator
	TCacheMGR       *tcm;									// Cache manager
	GroundSpot			*spot;								// Ground spot
	CVector					 tran;								// Translation vector
	//--- Current tile -----------------------------------
	PATCH_ELV        area;								// Current Tile
	//--- Hit buffer -------------------------------------
	GLuint					 bHit[8];						  // Hit buffer
	//--- WIndow tracker ---------------------------------
	CFuiTED         *wind;								// Window editor
	//--- Working area for QUAD --------------------------
	QUAD_QGT				 stak[4];							// Stock of QGT
	int							 nbs;									// Number of super Tile
	//--- Patche table -----------------------------------
	std::map<U_INT,PATCH_ELV*>	patch;		// List of patches
	//--- Selected vertex --------------------------------
	int							 svtx;								// Selected vertex
	//--- METHODS ----------------------------------------
public:
	CElvTracker();
 ~CElvTracker();
 	void  Register(CFuiTED *w);
	//----------------------------------------------------
	void	TimeSlice(float dT);
	//----------------------------------------------------
	void		FixElevation();
	void		InitStack();
	void		FillStack(U_INT ax,U_INT az);
	double  HaveElevation();
	//----------------------------------------------------
	int			SetElevation(int alt);
	int			FlattenTile(int alt);
	//----------------------------------------------------
	PATCH_ELV *GetPatch(U_INT key);
	//----------------------------------------------------
	void	OneSelection(U_INT No);
	void	SavePatches();
	//----------------------------------------------------
	void	Draw();
	void	DrawMarks();
	void	DrawOneMark(int k);
	//----------------------------------------------------
	inline void SetTCM(TCacheMGR *t)	{tcm = t;}
	inline int  GetModif()						{return patch.size();}
	inline void	Unselect()						{svtx = -1;}
};

//======================= END OF FILE ==============================================================
#endif // ELEVATION_TRACKER_H
