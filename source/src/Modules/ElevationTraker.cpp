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
}
//----------------------------------------------------------------------
//	Free resources
//----------------------------------------------------------------------
CElvTracker::~CElvTracker()
{		}
//----------------------------------------------------------------------
//	Register window tracker
//----------------------------------------------------------------------
void	CElvTracker::Register(CFuiTED *w)
{	wind	= w;
	if (wind)			return;
	tile	= 0;
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
{	for (int k=1; k!=4; k++)	stak[k].key	= 0;
	U_INT ax			= tile->GetTileAX();
	U_INT az			= tile->GetTileAZ();
	stak[0].key		= WorldSuperKey(ax,az);
	stak[0].qgt		= spot->qgt;
	stak[0].quad	= tile;
	nbs						= 1;
	//--- Search for all 8 neighboring tiles -------------
	for (int m=0; m<8; m++)
	{	U_INT cx = (ax + quadINC[m].dx) & TC_QGT_DET_MOD;
		U_INT cz = (az + quadINC[m].dz) & TC_QGT_DET_MOD;
		FillStack(cx,cz);
	}
	return;
}
//------------------------------------------------------------------------
//	Store surrounding super tiles
//------------------------------------------------------------------------
void CElvTracker::FillStack(U_INT ax,U_INT az)
{	U_INT key = WorldSuperKey(ax,az);
	int	k = 0;
  for (k=0; k!= nbs; k++)	if (key == stak[k].key)	return; 
	if (k == 4) gtfo("Elv Tracker: Bad mesh");
	//--- Enter new slot ----------
	U_INT	  qx		= (ax >> 5);					// Isolate QGT(X)
	U_INT   qz		= (az >> 5);					// Isolate QGT(Z)
	C_QGT *qgt		= globals->tcm->GetQGT(qx,qz);
	if (0 == qgt)						return;
	//--- Enter in stack ----------
	U_INT tx			= (ax & 31);
	U_INT tz			= (az & 31);
	stak[k].key		= key;
	stak[k].qgt		= qgt;
	stak[k].quad	= qgt->GetQuad(tx,tz);
	nbs++;
	return;
}
//------------------------------------------------------------------------
//	Save all modifications for storing in database
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//	Fix elevation
//	-Call each SuperTile with the corresponding modified tile
//	 so that the SuperTile VBO is reallocated
//------------------------------------------------------------------------
void CElvTracker::FixElevation()
{	globals->slw->StopSlew();					// Stop moving
	//--- Change vertex elevations -----------------
	for (int k=0; k!=nbs; k++)
	{	C_QGT		*qgt = stak[k].qgt;
	  CmQUAD  *qad = stak[k].quad;
		qgt->Reallocate(qad);
	}
	//--- Report tile to window editor -------------
	wind->TileNotify(tile,svrt->vrt);
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
	spot = tcm->GetSpot();			// Rabbit spot
	if (spot->InvalideQuad())							return;
	if (tile == spot->Quad)								return;
	//--- Update to new Tile -------------------
	tile			= spot->Quad;
	tile->GetVertices(wrk);
	wrk.qKey	= QGTKEY(spot->qx,spot->qz);		// QGT key
	wrk.detNo = tile->GetTileNo();
	wrk.tile	= tile;
	InitStack();
	//--- Locate first vertex ------------------
	svrt = (0 == wrk.vNum)?(0):(wrk.lvx);
	HaveElevation();
	return;
}
//----------------------------------------------------------------------
//	Object is selected
//----------------------------------------------------------------------
void CElvTracker::OneSelection(U_INT No)
{	//-- Change selected vertex ---------------------------
	globals->slw->StopSlew();						// Stop moving
	svrt			= wrk.lvx+No;					// Selected vertex
	HaveElevation();
	return;
}
//----------------------------------------------------------------------
//	Get elevation at selected spot
//----------------------------------------------------------------------
void CElvTracker::HaveElevation()
{	if (spot->InvalideQuad())									return;
	if (tile != spot->Quad)										return;
	float elv =	svrt->vrt->GetWZ();
	if (wind)	wind->EditElevation(elv);
	return;
}
//----------------------------------------------------------------------
//	Modify elevation at selected spot
//  -This is requested from the TERRA EDITOR WINDOW
//----------------------------------------------------------------------
float CElvTracker::IncElevation(float dte)
{	float amx = globals->aMax;
	float	amn = -5000;
	float elv = svrt->vrt->GetWZ() + dte;
	if (elv > amx)	elv = amx;
	if (elv < amn)  elv = amn;
	svrt->vrt->SetWZ(elv);
	FixElevation();
	return elv;
}
//----------------------------------------------------------------------
//	Set elevation at selected spot
//  -This is requested from the TERRA EDITOR WINDOW
//----------------------------------------------------------------------
float CElvTracker::SetElevation(float elv)
{	float amx = globals->aMax;
	float	amn = -5000;
	if (elv > amx)	elv = amx;
	if (elv < amn)  elv = amn;
	svrt->vrt->SetWZ(elv);
	FixElevation();
	return elv;
}
//----------------------------------------------------------------------
//	Draw markers
//----------------------------------------------------------------------
void CElvTracker::Draw()
{	//--- Check if no more tracking -----------------
	if (globals->aPROF.Not(PROF_TRACKE))	return;
	if (spot->InvalideQuad())							return;
	if (tile != spot->Quad)								return;
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
	for (U_INT k=0; k<wrk.vNum; k++) DrawOneMark(wrk.lvx + k);
	//--- Reset all attributes ----------------
	glPopAttrib();
	return;
}
//----------------------------------------------------------------------
//	Draw one marker
//	For picking mode, the vertex number is used as the vertex name
//----------------------------------------------------------------------
void CElvTracker::DrawOneMark(TVertex *vdf)
{	CVertex *vtx = vdf->vrt;
	spot->FeetCoordinatesTo(vtx,tran);				// components in feet
	glLoadName(vdf->vnum);									// Vector number
	//--- Camera at vertex and draw ----------------------------
	glPushMatrix();
	glTranslated(tran.x,tran.y,tran.z);			// Camera at vertex
	//----------------------------------------------------------
	U_INT col = (vdf ==svrt)?(COLOR_RED):(COLOR_LIGHT_BLUE);
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
