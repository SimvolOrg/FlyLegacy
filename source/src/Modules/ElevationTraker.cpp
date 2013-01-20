//============================================================================================
// ElevationTraker.cpp
//
// Part of Fly! Legacy project
//
// Copyright 2005 Chris Wallace
// Copyright 2008 Jean Sabatier
//
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
//===========================================================================================

#include "../Include/FlyLegacy.h"
#include "../Include/Globals.h"
#include "../Include/ElevationTracker.h"
#include "../Include/Cameras.h"
#include "../Include/TerrainUnits.h"
#include "../Include/TerrainData.h"
#include "../Include/FuiUser.h"
//==========================================================================
//	Color tables
//==========================================================================
float E_blue[]	= {0.11f,0.84f,0.81f,1};
float E_red[]		= {1,0,0,1};
//==========================================================================
//  Pyramide for tracker
//==========================================================================
#define PYRA_HT (400)
F3_VERTEX pyramTAB[] =	{
	{   0,   0,  0},			// Summit			0
	{-100,-100,PYRA_HT},			// SW corner	1
	{+100,-100,PYRA_HT},			// SE corner	2
	{+100,+100,PYRA_HT},			// NE corner	3
	{-100,+100,PYRA_HT},			// NW corner	4
};
//-----list of indices ----------------------------------------
GLubyte pyramIND[] = {
	0,2,1,						// Face O-SE-SW
	0,3,2,						// Face 0-NE-SE
	0,4,3,						// Face 0-NW-NE
	0,1,4,						// Face 0,SW,NW
};

//==========================================================================
//  DT Increment Table to locate QUAD neighbors
//==========================================================================
//------------------------------------------------------
TC_INCREMENT quadINC[] =
{ {-1,-1},                      // SW Quad
	{	0,-1},											// South
	{+1,-1},											// SE	quad
	{+1, 0},											// East
	{+1,+1},											// NE quad
	{ 0,+1},											// North
	{-1,+1},											// NW quad
	{-1, 0},											// West
};
//============================================================================
//	Elevation Tracker:
//		Draw elevation spot for Terrain editor
//		Modify mesh
//============================================================================
CElvTracker::CElvTracker()
{	tcm		= 0;
	upd		= 0;
	wind	= 0;
	area.quad	= 0;
}
//----------------------------------------------------------------------
//	Free resources
//----------------------------------------------------------------------
CElvTracker::~CElvTracker()
{	std::map<U_INT,PATCH_ELV*>::iterator ip;
	for (ip = patch.begin(); ip != patch.end(); ip++)
	{	PATCH_ELV *p = (*ip).second;
		delete p;
	}
	patch.clear();
}
//----------------------------------------------------------------------
//	Register window tracker
//----------------------------------------------------------------------
void	CElvTracker::Register(CFuiTED *w)
{	wind	= w;
	if (wind)			return;
	area.quad	= 0;
	return;
}
//------------------------------------------------------------------------
//	Init Stack
//	-When elevation is modified, the supertile should recompute the VBO 
//		buffer so that drawing will reflect the change
//	-When one vertex is changed, up to 4 supertile may be impacted
//	-Surrounding tiles are scanned to detect distinct supertile that must 
//		be reallocated
//------------------------------------------------------------------------
void CElvTracker::InitStack()
{	for (int k=1; k!=4; k++)	stak[k].skey	= 0;
	U_INT ax			= area.quad->GetTileAX();				// Detail Tile absolute X index
	U_INT az			= area.quad->GetTileAZ();				// Detail Tile absolute Z index
	U_INT dkey		= (ax << 16) | az;							// Detail tile Key
	stak[0].dkey	= dkey;
	stak[0].skey	= WorldSuperKey(ax,az);
	stak[0].qgt		= spot->qgt;
	stak[0].quad	= area.quad;
	nbs						= 1;
	//--- Search for all 8 neighboring Detail Tiles -------------
	for (int m=0; m<8; m++)
	{	U_INT dx = (ax + quadINC[m].dx) & TC_QGT_DET_MOD;
		U_INT dz = (az + quadINC[m].dz) & TC_QGT_DET_MOD;
		FillStack(dx,dz);
	}
	return;
}
//------------------------------------------------------------------------
//	Store surrounding Detail tiles that does not resides in the same
//	Supertile.
//------------------------------------------------------------------------
void CElvTracker::FillStack(U_INT ax,U_INT az)
{	U_INT skey = WorldSuperKey(ax,az);
	int	k = 0;
  for (k=0; k!= nbs; k++)	if (skey == stak[k].skey)	return; 
	if (k == 4) gtfo("Elv Tracker: Bad mesh");
	//--- Locate QGT ------------------
	U_INT	  qx		= (ax >> 5);					// Isolate QGT(X)
	U_INT   qz		= (az >> 5);					// Isolate QGT(Z)
	C_QGT *qgt		= globals->tcm->GetQGT(qx,qz);
	if (0 == qgt)						return;
	//--- Enter in stack --------------
	U_INT tx			= (ax & 31);					// Isolate X index
	U_INT tz			= (az & 31);					// Isolate Z index
	//----------------------------------
	stak[k].dkey	= (ax << 16) | az;			// Detail Tile  key
	stak[k].skey	= skey;									// Supertile key
	stak[k].qgt		= qgt;									// QGT
	stak[k].quad	= qgt->GetQuad(tx,tz);	// Detail Tile
	//----------------------------------
	nbs++;
	return;
}
//------------------------------------------------------------------------
//	Fix elevation
//	-Call each SuperTile with the corresponding modified tile
//	 so that the SuperTile VBO is reallocated
//	-Terrain will be updated and next frame will show it
//------------------------------------------------------------------------
void CElvTracker::FixElevation()
{	globals->slw->StopSlew();					// Stop moving
	//--- Change vertex elevations -----------------
	for (int k=0; k!=nbs; k++)
	{	C_QGT		*qgt = stak[k].qgt;
	  CmQUAD  *qad = stak[k].quad;
		qgt->Reallocate(qad);
		//--- Enter a patch slot for each tile -------
		U_INT	tx				= TILE_PART(stak[k].dkey >> 16);
		U_INT tz				= TILE_PART(stak[k].dkey);
		PATCH_ELV *slot = GetPatch(stak[k].dkey);
		slot->key				= qgt->FullKey();
		slot->sno				= MakeSuperNo(tx,tz);
		slot->dno				= FN_DET_FROM_XZ(tx,tz);
		slot->nbp				= 0;
		slot->dir				= PATCH_READ;
		slot->qgt				= qgt;
		slot->quad			= qad;
		qad->ScanPatch(slot);
	}
	return;
}
//------------------------------------------------------------------------
//	Update state when tracking requested
//	 -Tracking is actived when the TERRA EDITOR WINDOW is active
//	 -When active, the tracker builds a list of vertices from the
//		Detail Tile where the Rabbit spot is.
//------------------------------------------------------------------------
void CElvTracker::TimeSlice(float dT)
{	if (globals->aPROF.Not(PROF_TRACKE))	return;
	CmQUAD *quad = area.quad;
	spot = tcm->GetSpot();			// Rabbit spot
	if (spot->InvalideQuad())							return;
	if (quad == spot->Quad)								return;
	//--- Update to new Tile -------------------
	area.quad			= spot->Quad;
	InitStack();
	//--- Init current area --------------------
	area.dir	= PATCH_READ;
	area.qgt	= spot->qgt;
	area.nbp	= 0;
	area.quad->ScanPatch(&area);
	//--- Unselect -----------------------------
	if (wind)		wind->Unselect();
	svtx = -1;
	return;
}
//----------------------------------------------------------------------
//	Object is selected
//----------------------------------------------------------------------
void CElvTracker::OneSelection(U_INT No)
{	//-- Change selected vertex ---------------------------
	globals->slw->StopSlew();				// Stop moving
	svtx = No;											// Selected vertex
	double elv = HaveElevation();
	if (wind)	wind->OneElevation(elv);
	return;
}
//----------------------------------------------------------------------
//	Get elevation at selected spot
//----------------------------------------------------------------------
double CElvTracker::HaveElevation()
{	if (spot->InvalideQuad())									return 0;
	if (area.quad != spot->Quad)							return 0;
	if (svtx < 0)															return 0;
	//--- Get elevation from selected vertice -------
	return area.vtb[svtx]->GetWZ();
}
//----------------------------------------------------------------------
//	Set elevation at selected spot
//  -This is requested from the TERRA EDITOR WINDOW
//----------------------------------------------------------------------
int CElvTracker::SetElevation(int alt)
{	if (svtx < 0)	return 0;
	CVertex  *vtx = area.GetVertex(svtx);
	if (vtx)	vtx->ClampAltitude(double(alt));
	FixElevation();
	return int(vtx->GetWZ());
}
//----------------------------------------------------------------------
//	Flatten tile to unic elevation
//  -This is requested from the TERRA EDITOR WINDOW
//----------------------------------------------------------------------
int CElvTracker::FlattenTile(int alt)
{	CVertex *vtx = 0;
	for (U_INT k = 0; k < area.nbp; k++)
	{	vtx = area.GetVertex(k);
		if (vtx)	vtx->ClampAltitude(double(alt));
	}
	FixElevation();
	return int(vtx->GetWZ());
}
//----------------------------------------------------------------------
//	Get a patch for the detail tile
//----------------------------------------------------------------------
PATCH_ELV *CElvTracker::GetPatch(U_INT key)
{	std::map<U_INT,PATCH_ELV*>::iterator it = patch.find(key);
	if (it != patch.end())	return (*it).second;
	//--- Create one slot ------------------------
	PATCH_ELV *slot = new PATCH_ELV(key);
	patch[key]			= slot;
	return slot;
}
//----------------------------------------------------------------------
//	Save all patches
//----------------------------------------------------------------------
void CElvTracker::SavePatches()
{	std::map<U_INT,PATCH_ELV*>::iterator ip;
	for (ip = patch.begin(); ip != patch.end(); ip++)
	{	PATCH_ELV *p = (*ip).second;
		globals->sqm->WritePatche(*p);
		delete p;
	}
	patch.clear();
}
//----------------------------------------------------------------------
//	Draw markers
//----------------------------------------------------------------------
void CElvTracker::Draw()
{	//--- Check if no more tracking -----------------
	if (globals->aPROF.Not(PROF_TRACKE))	return;
	if (spot->InvalideQuad())							return;
	if (area.quad != spot->Quad)					return;
	return DrawMarks();
}
//----------------------------------------------------------------------
//	Draw markers
//	This function is commun to the rendering state and the
//	picking mode.
//----------------------------------------------------------------------
void CElvTracker::DrawMarks()
{	upd	= 0;
  glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	//--- draw all marks over the vertices ----
	for (U_INT k=0; k<area.nbp; k++)	DrawOneMark(k);
	//--- Reset all attributes ----------------
	glPopAttrib();
	return;
}
//----------------------------------------------------------------------
//	Draw one marker
//	For picking mode, the vertex number is used as the vertex name
//----------------------------------------------------------------------
void CElvTracker::DrawOneMark(int k)
{	CVertex *vtx = area.GetVertex(k);
	spot->FeetCoordinatesTo(vtx,tran);			// components in feet
	glLoadName(k);													// Vertex number
	//--- Camera at vertex and draw ----------------------------
	glPushMatrix();
	glTranslated(tran.x,tran.y,tran.z);			// Camera at vertex
	//----------------------------------------------------------
	U_INT col = (k ==svtx)?(COLOR_RED):(COLOR_LIGHT_BLUE);
	ColorGL(col);
	glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
  glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT,0,pyramTAB);
	glDrawElements(GL_TRIANGLES,12,GL_UNSIGNED_BYTE,pyramIND);
	glPopClientAttrib();
	glPopMatrix();
	return;
}

//=====================END OF FILE =)=======================================================
