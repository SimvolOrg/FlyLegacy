/*
 * TerrainCache.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright  2003 Chris Wallace
 *            2008 Jean Sabatier
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
//=================================================================================
#include "../Include/Globals.h"
#include "../Include/FlyLegacy.h"
#include "../Include/TerrainCache.h"
#include "../Include/TerrainElevation.h"
#include "../Include/TerrainTexture.h"
#include "../Include/ScenerySet.h"
#include "../Include/ElevationTracker.h"
#include "../Include/Triangulator.h"
#include "../Include/OSMobjects.h"
#include "../Include/MagneticModel.h"
#include "../Include/FileThread.h"
#include "../Include/Fui.h"
#include "../Include/FuiParts.h"
#include "../Include/FuiUser.h"
#include "../Include/3dMath.h"
#include "../Include/model3D.h"
#include "../Include/sky.h"
#include "../Include/Cloud.h"
#include "../include/SqlMGR.h"
#include "../Include/TerraFile.h"
#include <map>
#include<crtdbg.h>
//================================================================================
//================================================================================
//	BIT FORMAT OF A VERTEX KEY (either X or Z)
//	3          2         1
//	10987654321098765432109876543210
//  ---------------------------------
// |        qqqqqqqqqdddddvvvvvvvvvv|   24 bits are used with the following format
// ----------------------------------
//	vvvvvvvvvv:  10 bits for vertex subdivision number inside a detail tile. There 
//								may be up to 1024 subdivisions depending on description.  
//								Note that the middle vertex has number 512.  So if the detail
//								tile is divided in 4 parts along each direction, then vertice
//								numbers are : 0, 256, 512, 768, 0.  The right vertex is the first
//								vertex of the next tile, so it has number 0.
//								
//	ddddd			:   5 bits for detail tile number inside a QGT. Detail Tile are
//								numbered [0-31] relative to QGT.
//	qqqqqqqqq :		9 bits for QGT in the whole World. QGT are numbered [0-511].The
//								whole world is divided by an array of 512*512 QGT.
//
//	For longitude X key is  from 0 from origin meridian and increase going west.
//	For latitude  Z key run from 0 in South pole to 511-31-vvvv in north pole
//=============================================================================
void *FileThread(void *p);
extern U_SHORT SideRES[];
//=============================================================================
//  HEXA TABLE
//=============================================================================
char *HexTAB = "0123456789ABCDEF";
//=============================================================================
//  BASE VERTICE (row-col) in a Detail Tile
//=============================================================================
//=============================================================================
//  Transition for probe
//=============================================================================
char *TrsTAB[] = {
  "N",      // 0= none
  "B",      // 1= bottom
  "C",      // 2= corner
  "BC",     // 3= bottom+corner
  "R",      // 4= right
  "RB",     // Right and bottom
  "RC",     // Right and corner
  "RBC",    // All
};
//==========================================================================
//  QGT locator tables
//==========================================================================
//--South West corner ------------------------------------------ 
QGT_DIR SWcorner[] =
{ {-1, 0,TC_SECORNER},                      // West  QGT
  {-1,-1,TC_NECORNER},                      // SW    QGT
  { 0,-1,TC_NWCORNER}};                     // South QGT
//--South East corner ------------------------------------------
QGT_DIR SEcorner[] =
{ { 0,-1,TC_NECORNER},                      // South QGT
  {+1,-1,TC_NWCORNER},                      // SE    QGT
  {+1, 0,TC_SWCORNER}};                     // East  QGT
//--North East corner ------------------------------------------
QGT_DIR NEcorner[] =
{ {+1, 0,TC_NWCORNER},                      // East  QGT
  {+1,+1,TC_SWCORNER},                      // NE    QGT
  { 0,+1,TC_SECORNER}};                     // North QGT
//--North West corner -------------------------------------------
QGT_DIR NWcorner[] =
{ { 0,+1,TC_SWCORNER},                      // North QGT
  {-1,+1,TC_SECORNER},                      // NW    QGT
  {-1, 0,TC_NECORNER}};                     // West  QGT
//==========================================================================
//  Increment table for adjacent QGT
//==========================================================================
TC_INCREMENT qgtCORNER[] =
{ { 0, 0},                                  // SW corner 
  {+1, 0},                                  // SE corner
  {+1,+1},                                  // NE corner
  { 0,+1}                                   // NW corner
};
//==========================================================================
//  Increment Table for locating Transition Tile
//==========================================================================
TC_INCREMENT TransTAB[] = {
  { 0, 0},                                  // A this tile increment
  {+1, 0},                                  // X-Right Tile
  { 0,-1},                                  // B-Bottom Tile
  {+1,-1},                                  // Y-Corner Tile
  };

//=============================================================================
//  TABLE to detect sea layer from tile type
//=============================================================================
U_CHAR  SeaTAB[] = {
  0,                        // 0 => Pure land tile
  TC_SEALAY,                // 1 => Pure Water tile
  0,                        // 2 => Coast tile
  TC_SEALAY,                // 3 => mixed Water and Coast tile
};
//==========================================================================
//  Factor table Quad corner computing
//==========================================================================
TC_INCREMENT CornerMASK[] =
{ { 0,            0},                                   // SW corner 
  { TC_1024MOD,   0},                                   // SE corner
  { TC_1024MOD,   TC_1024MOD},                          // NE corner
  { 0,            TC_1024MOD}                           // NW corner
};
//==========================================================================
//  Detail tile borders in super Tile.  
//==========================================================================
char BorderSUP[] =
{  0,0,0,1,                     // South row
   0,0,0,1,                     // Next row
   0,0,0,1,                     // Next row
   2,2,2,3,                     // Top row
};
//==========================================================================
//  SUPER TILE Center coordinates definitions
//==========================================================================
TC_ST_COORD SuperCT[]  = {
  { 2, 2},{ 6, 2},{10, 2},{14, 2},{18, 2},{22, 2},{26, 2},{30, 2},
  { 2, 6},{ 6, 6},{10, 6},{14, 6},{18, 6},{22, 6},{26, 6},{30, 6},
  { 2,10},{ 6,10},{10,10},{14,10},{18,10},{22,10},{26,10},{30,10},
  { 2,14},{ 6,14},{10,14},{14,14},{18,14},{22,14},{26,14},{30,14},
  { 2,18},{ 6,18},{10,18},{14,18},{18,18},{22,18},{26,18},{30,18},
  { 2,22},{ 6,22},{10,22},{14,22},{18,22},{22,22},{26,22},{30,22},
  { 2,26},{ 6,26},{10,26},{14,26},{18,26},{22,26},{26,26},{30,26},
  { 2,30},{ 6,30},{10,30},{14,30},{18,30},{22,30},{26,30},{30,30},
};
//==========================================================================
//  SUPER TILE Base Detail Tile coordinates
//==========================================================================
TC_INCREMENT SuperDT[]  = {
  { 0, 0},{ 4, 0},{ 8, 0},{12, 0},{16, 0},{20, 0},{24, 0},{28, 0},
  { 0, 4},{ 4, 4},{ 8, 4},{12, 4},{16, 4},{20, 4},{24, 4},{28, 4},
  { 0, 8},{ 4, 8},{ 8, 8},{12, 8},{16, 8},{20, 8},{24, 8},{28, 8},
  { 0,12},{ 4,12},{ 8,12},{12,12},{16,12},{20,12},{24,12},{28,12},
  { 0,16},{ 4,16},{ 8,16},{12,16},{16,16},{20,16},{24,16},{28,16},
  { 0,20},{ 4,20},{ 8,20},{12,20},{16,20},{20,20},{24,20},{28,20},
  { 0,24},{ 4,24},{ 8,24},{12,24},{16,24},{20,24},{24,24},{28,24},
  { 0,28},{ 4,28},{ 8,28},{12,28},{16,28},{20,28},{24,28},{28,28},
};
//==========================================================================
//  QGT Base QUAD indexed by Super Tile number
//==========================================================================
U_INT SuperQDB[] = {
       0,   4,    8,  12,  16,  20,  24,  28,       // [  0-  7]
     128, 132,  136, 140, 144, 148, 152, 156,       // [  8- 15]
     256, 260,  264, 268, 272, 276, 280, 284,       // [ 16- 23]
     384, 388,  392, 396, 400, 404, 408, 412,       // [ 24- 31]
     512, 516,  520, 524, 528, 532, 536, 540,       // [ 32- 39]
     640, 644,  648, 652, 656, 660, 664, 668,       // [ 40- 47]
     768, 772,  776, 780, 784, 788, 792, 796,       // [ 48- 55]
     896, 900,  904, 908, 912, 916, 920, 924,       // [ 56- 63]
};
//==========================================================================
//  SUPER TILE complement QUAD in (0-16)
//==========================================================================
U_INT SuperQDD[] = {
      0,   1,  2,  3,                               // [0-3]
     32,  33, 34, 35,                               // [4-7]
     64,  65, 66, 67,                               // [8-11]
     96,  97, 98, 99,                               // [2-15]
};
//==========================================================================
//  TABLE TO ACCESS SUPER TILE BORDERS VERTICES
//==========================================================================
TC_STBORD SupBORD[] = {                         //VT-XZ
  { 0, 0,TC_SWCORNER,0},                          // VT00
  { 1, 0,TC_SWCORNER,0},                          // VT10
  { 2, 0,TC_SWCORNER,0},                          // VT20
  { 3, 0,TC_SWCORNER,0},                          // VT30
  { 3, 0,TC_SECORNER,1},                          // VT40
  { 3, 1,TC_SECORNER,1},                          // VT41
  { 3, 2,TC_SECORNER,1},                          // VT42
  { 3, 3,TC_SECORNER,1},                          // VT43
  { 3, 3,TC_NECORNER,1},                          // VT44
  { 2, 3,TC_NECORNER,0},                          // VT34
  { 1, 3,TC_NECORNER,0},                          // VT24
  { 0, 3,TC_NECORNER,0},                          // VT14
  { 0, 3,TC_NWCORNER,0},                          // VT04
  { 0, 2,TC_NWCORNER,0},                          // VT03
  { 0, 1,TC_NWCORNER,0},                          // VT02
  { 0, 0,TC_NWCORNER,0},                          // VT01
};
//==========================================================================
//  Level increment. Vertex indice increment depending on level
//==========================================================================
int LevINC[] = {
  512,                      // Level 0 => 1/2     of tile side
  256,                      // Level 1 => 1/4     of tile side
  512,                      // Level 2 => 1/16    of tile side
  1024,                     // Level 3 => 1/32    ""
  2048,                     // Level 4 => 1/64    ""
  4096,                     // Level 5 => 1/128   ""
  8102,                     // Level 6 => 1/256   ""
  16384,                    // level 7 => 1/512   ""
  32768,                    // Level 8 => 1/1024
};
//==========================================================================
//  Resolution Table for normal detail tile
//==========================================================================
float *TexRES[TC_MAX_TEX_RES] = {
  0,                                    // Low resolution 64 pixels (Not used)
  0,                                    // medium resolution
  0,                                    // Hight resolution
  0,                                    // EPD resolution
};
//==========================================================================
//  Wire option
//==========================================================================
U_INT WireOPT[] = {
  GL_FILL,
  GL_LINE,
};
//==========================================================================
//  Visibility code
//==========================================================================
char visiCODE[] = {
   0,                            // totaly on Left 
   0,                            // Right
  -1,                            // Bottom
  -1,                            // Top
  -1,                            // Near
   0,
};
//==========================================================================
//  Vertices number per level
//==========================================================================
int vertNBR[] = {
	6,						// Level 0
	10,						// Level 1
	10,						// Level 2
	10,						// Level 3
};
//==========================================================================
//  Shared Object management
// 
//==========================================================================
//  Assign a shared object from pointer
//==========================================================================
void CPTR::operator=(QGTHead *p)
{ if (obj == p)   return;
  if (0 != obj)   obj->DecUser();
  obj = p;
  if (0 != obj)   obj->IncUser();
  return;
}
//==========================================================================
//  Assign a shared object from reference
//==========================================================================
void CPTR::operator=(QGTHead &n)
{ if (obj ==  &n) return;
  if (0 != obj)   obj->DecUser();
  obj = &n;
  if (0 != obj)   obj->IncUser();
  return;
}
//==========================================================================
//  Assign a shared object from another CPTR
//==========================================================================
void CPTR::operator=(CPTR &q)
{ if (0 != obj)   obj->DecUser();
  obj = q.obj;
  if (0 != obj)   obj->IncUser();
  return;
}
//==========================================================================
//  Create a shared object
//==========================================================================
QGTHead::QGTHead()
{ pthread_mutex_init (&mux,  NULL);
  Users = 1;
}
//-----------------------------------------------------------------------
//	Increment user count
//-----------------------------------------------------------------------
void QGTHead::IncUser()
{	//----Lock object here ---------------
  pthread_mutex_lock (&mux);
	Users++;
  pthread_mutex_unlock (&mux);
	//---- Unlock object here -------------
	return;	}
//-----------------------------------------------------------------------
//	Decrement user count
//	When count is nul, the object is deleted
//-----------------------------------------------------------------------
bool QGTHead::DecUser()
{	//--Lock object here ----------------------
  pthread_mutex_lock (&mux);
	Users--;
  bool del = (Users == 0);
	//--Unlock object here --------------------
  pthread_mutex_unlock (&mux);
	if (del == false)     return false;
	//--Delete this QGT ----------------------
	globals->cnDOB	= 0;
  int tr = 0;
	int qx = 0;
	int qz = 0;
	GetTrace(&tr,&qx,&qz);
  delete this;
	//--- TRACE QGT deletion ---------------------------------------------------------
	if (tr)
	{	int		nob = globals->cnDOB;
	 	float tim = globals->tcm->Time();
		TRACE("TCM: -- Time: %04.2f QGT(%03d-%03d) Deleted. Release %06d Objects",tim,qx,qz,nob);
	}
	return true;
}

//==========================================================================
//  Initialize Texture Descriptor
// 
//==========================================================================
CTextureDef::CTextureDef()
{ xFlag     = 0;
  TypTX     = 0;
  coast     = 0;
  Tmask     = 0;
  Reso[0]   = TC_MEDIUM;
  quad      = 0;
  dOBJ      = 0;
  nOBJ      = 0;
	Name[0]		= 0;
	Name[10]		= '$';
  dTEX[0]   = 0;
  dTEX[1]   = 0;
  nTEX[0]   = 0;
  nTEX[1]   = 0;
  Hexa[0]   = 0;
}
//-------------------------------------------------------------------------
//  Destructor
//-------------------------------------------------------------------------
CTextureDef::~CTextureDef()
{ FreeALL();
}
//-------------------------------------------------------------------------
//  Texture identifier
//-------------------------------------------------------------------------
bool CTextureDef::AreWe(U_INT ax,U_INT az)
{ if (0 == quad)								return false;
	if (quad->GetTileAX() != ax)  return false;
  if (quad->GetTileAZ() != az)  return false;
  return true;
}
//-------------------------------------------------------------------------
//  Pop alternate textures
//	Textures at the top are removed
//	Alternates textures (DAY and NIGHT) are set from the bottom stack
//-------------------------------------------------------------------------
void CTextureDef::PopTextures(U_CHAR opt)
{ if (opt) FreeDAY();
  if (opt) FreeNIT();
  GLubyte *bfd = dTEX[1];
  GLubyte *bfn = nTEX[1];
  dTEX[0]  = bfd;
  nTEX[0]  = bfn;
  Reso[0]  = Reso[1];
  dTEX[1]  = 0;
  nTEX[1]  = 0;
  Reso[1]  = 0;
  return;
}
//-------------------------------------------------------------------------
//  Free The resources
//-------------------------------------------------------------------------
int CTextureDef::FreeALL()
{ globals->txw->FreeTerrainOBJ(dOBJ,nOBJ);
  dOBJ  = 0;
  nOBJ  = 0;
  //------Free level 1 textures ---------------------
  FreeDAY();
  FreeNIT();
  FreeALT();
  return 0;
}
//-------------------------------------------------------------------------
//  Free night texture only
//-------------------------------------------------------------------------
int CTextureDef::FreeNTX()
{ globals->txw->FreeTerrainOBJ(0,nOBJ);
  nOBJ  = 0;
  FreeNIT();
  return 0;
}
//-------------------------------------------------------------------------
//  Assign a texture object and free texture Day 
//-------------------------------------------------------------------------
int CTextureDef::AssignDAY(U_INT obj)
{ FreeDAY();                // Release texture
  if (dOBJ == obj)     return 0;
  dOBJ  = obj;
	return 1;
}
//-------------------------------------------------------------------------
//  Assign a texture object and free texture night 
//-------------------------------------------------------------------------
int CTextureDef::AssignNIT(U_INT obj)
{ FreeNIT();                // Release texture
  if (nOBJ == obj)     return 0;
  nOBJ  = obj;
	return 1;
}

//-------------------------------------------------------------------------
//  Free day texture only 
//-------------------------------------------------------------------------
void CTextureDef::FreeDAY()
{ GLubyte *buf = dTEX[0];
  dTEX[0]      = 0;
  if (buf)  delete [] buf; 
  return;
}
//-------------------------------------------------------------------------
//  Free night texture only 
//-------------------------------------------------------------------------
void CTextureDef::FreeNIT()
{ GLubyte *buf = nTEX[0];
  if (buf)   delete [] buf;
  nTEX[0]  = 0;
  return;
}
//------------------------------------------------------------------------
//  Free the alternate  texture only
//-------------------------------------------------------------------------
void CTextureDef::FreeALT()
{ GLubyte *bfd = dTEX[1];
  GLubyte *bfn = nTEX[1];
  if (bfd)  delete [] bfd;
  if (bfn)  delete [] bfn;
  dTEX[1]   = 0;
  nTEX[1]   = 0;
  return;
}
//-------------------------------------------------------------------------
//  return the tile indices
//-------------------------------------------------------------------------
void CTextureDef::GetTileIndices(U_INT &tx,U_INT &tz)
{ if (quad == 0)  gtfo("No Quad");
  tx  =  quad->GetTileAX();
  tz  =  quad->GetTileAZ();
  return;
}

//==========================================================================
//  Initialize the Vertex descriptor
//	NOTE:  World coordinates are relative to SW corner of the QGT
//==========================================================================
CVertex::CVertex(U_INT xv,U_INT zv)
{	xKey  = xv;
  zKey  = zv;
  Use   = 0;
  Fixe  = VTX_DYNAM;
  nElev = 0;
  gType = 0;
  Ground = 0;
  Edge[TC_NORTH]  = 0;
  Edge[TC_SOUTH]  = 0;
  Edge[TC_WEST]   = 0;
  Edge[TC_EAST]   = 0;
	//--- Assign world coordinates -------------------
  rx	= RelativeLongitudeInQGT(xv);
	ry	= RelativeLatitudeInQGT(zv);
	rz	= 0;
}
//-------------------------------------------------------------------------
//  Fixed vertex
//-------------------------------------------------------------------------
CVertex::CVertex()
{	xKey  = 0;
  zKey  = 0;
  Use   = 0;
  gType = 0;
  nElev = 0;
  Fixe  = 0;
  Ground = 0;
  Edge[TC_NORTH]  = 0;
  Edge[TC_SOUTH]  = 0;
  Edge[TC_WEST]   = 0;
  Edge[TC_EAST]   = 0;
	rx		= 0;
	ry		= 0;
	rz		= 0;

}
//-------------------------------------------------------------------------
//  Destructor
//-------------------------------------------------------------------------
CVertex::~CVertex()
{ int n = (Fixe & VTX_DYNAM)?(1):(0);
  globals->NbVTX -= n;
}
//-------------------------------------------------------------------------
//  Init the vertex
//-------------------------------------------------------------------------
void  CVertex::Init(U_INT xv,U_INT zv)
{	xKey  = xv;
  zKey  = zv;
  //--- Assign world coordinates -------------------
  rx	= RelativeLongitudeInQGT(xv);
	ry	= RelativeLatitudeInQGT(zv);
	rz	= 0;
  return;
}

//-------------------------------------------------------------------------
//  Compute in V the relative coordinates to vertex A
//	NOTE: A is always a center vertex so it is not a frontier vertex
//				This one may be a frontier vertex
//-------------------------------------------------------------------------
CVector  CVertex::RelativeFrom(CVertex &a)
{ CVector v;
	U_INT		xa		= a.keyX();
	double	orgx  = (SameXQGT(a))?(GetRX()):(TC_ARCS_PER_QGT);
  v.x = orgx - a.GetRX();
	U_INT   za		= a.keyZ();
	double  orgy  = (SameZQGT(a))?(GetRY()):(LatitudeIncrement(FN_QGT_FROM_INDX(zKey)));
  v.y = orgy - a.GetRY();
  v.z = GetWZ() - a.GetWZ();
  return v;
}
//-------------------------------------------------------------------------
//  Check if vertex is above
//-------------------------------------------------------------------------
bool CVertex::IsAbove(double y)
{ return (GetRY() >= y);	}
//-------------------------------------------------------------------------
//  Check if point is to the left
//-------------------------------------------------------------------------
bool CVertex::ToRight(double x)
{	return (GetRX() >= x);	}
//-------------------------------------------------------------------------
//  Check if we are this vertex
//-------------------------------------------------------------------------
bool CVertex::AreWe(U_INT ax,U_INT az)
{	if ((xKey == ax) && (zKey == az))
	int a = 0;
	return true;
 }
//-------------------------------------------------------------------------
//  Copy Edges
//-------------------------------------------------------------------------
void CVertex::CopyEdge(CVertex *vt)
{ Edge[TC_SWCORNER] = vt->Edge[TC_SWCORNER];
  Edge[TC_SECORNER] = vt->Edge[TC_SECORNER];
  Edge[TC_NWCORNER] = vt->Edge[TC_NWCORNER];
  Edge[TC_NECORNER] = vt->Edge[TC_NECORNER];
  return;
}
//-------------------------------------------------------------------------
//  Return vertex absolute coordinates 
//-------------------------------------------------------------------------
SVector CVertex::GeoCoordinates(C_QGT *qgt)
{	SVector   v;
  U_INT    qx = FN_QGT_FROM_INDX(xKey);
	double x		= (qgt->GetXkey() == qx)?(rx):(TC_ARCS_PER_QGT);  // Eastern position
	v.x					= qgt->GetWesLon() + x;
	v.y					= qgt->GetSudLat() + ry;
	v.z					= rz;
	return   v;
}
//-----------------------------------------------------------------------
//  Insert vn after vp in North direction
//-----------------------------------------------------------------------
void CVertex::InsertNorth(CVertex *vn)
{ CVertex *vq = Edge[TC_NORTH];
  Edge[TC_NORTH] = vn;
  vn->Edge[TC_SOUTH] = this;
  vn->Edge[TC_NORTH] = vq;
  if (0 == vq)  return;
  vq->Edge[TC_SOUTH] = vn;
  return;
}
//-----------------------------------------------------------------------
//  Insert vn after vp in East direction
//-----------------------------------------------------------------------
void CVertex::InsertEast(CVertex *vn)
{ CVertex *ve = Edge[TC_EAST];        // My east vertex
  vn->Edge[TC_WEST] = this;           // I am west to new
  vn->Edge[TC_EAST] = ve;             // east for new
  Edge[TC_EAST]     = vn;
  if (0 == ve)    return;
  ve->Edge[TC_WEST] = vn;
  return;
}
//-----------------------------------------------------------------------
//  Link toward East V1 to V2
//  NOTE: V1 may already be linked when v1 is a border vertex
//-----------------------------------------------------------------------
void CVertex::EastLink(CVertex *v2)
{ if (0 != Edge[TC_EAST])  return;
  v2->Edge[TC_WEST] = this;
  Edge[TC_EAST]     = v2;
  return;
}
//-----------------------------------------------------------------------
//  Link toward North V1 to V2
//-----------------------------------------------------------------------
void CVertex::NorthLink(CVertex *v2)
{ if (0 != Edge[TC_NORTH])  return;
  v2->Edge[TC_SOUTH]  = this;
  Edge[TC_NORTH]      = v2;
  return;
}
//=========================================================================
//  Initialize the Detail Tile description
//=========================================================================
CmQUAD::CmQUAD()
{ subL		= 0;
	qDim    = 1;
  qARR    = this;
  nvtx    = 0;
  vTab    = 0;
  Flag    = 0;
	iBUF		= 0;
	msp			= 0;
	RenderIND();
}
//-------------------------------------------------------------------------
//  End of CmQUAD
//-------------------------------------------------------------------------
CmQUAD::~CmQUAD()
{	if (IsArray())	
		delete [] qARR;
	if (iBUF)	delete [] iBUF;
}
//-------------------------------------------------------------------------
//  Check if we are for requested tile (testing purpose)
//-------------------------------------------------------------------------
bool  CmQUAD::AreWe(U_INT qx,U_INT tx,U_INT qz,U_INT tz)
{ U_INT cx = GetTileAX();
  U_INT cz = GetTileAZ();
  U_INT rx = (qx << TC_BY32) | tx;
  U_INT rz = (qz << TC_BY32) | tz;
  if ((rx != cx) || (rz != cz)) return false;
  cx = cx;                                      // For breakpoint when true
  return true;
}
//-------------------------------------------------------------------------
//  Check if we are for requested tile (testing purpose)
//-------------------------------------------------------------------------
bool  CmQUAD::AreWe(U_INT ax,U_INT az)
{ U_INT cx = Center.keyX();
  U_INT cz = Center.keyZ();
  if ((ax != cx) || (az != cz)) return false;
  cx = cx;                                      // For breakpoint when true
  return true;
}
//-------------------------------------------------------------------------
//  Make a Quad with the center vertex
//-------------------------------------------------------------------------
void CmQUAD::SetParameters(CVertex *ct,U_CHAR lv)
{ qDim    = 1;
  qARR    = this;
	subL		= lv;
  //------Duplicate Center vertex into the CmQUAD ------------
  Center.xKey   = ct->xKey;
  Center.zKey   = ct->zKey;
  Center.Use    = ct->Use;
  //-----Vertice Corners ----------------------------------------
  Center.Edge[TC_SWCORNER]  = ct->Edge[TC_SWCORNER];
  Center.Edge[TC_SECORNER]  = ct->Edge[TC_SECORNER];
  Center.Edge[TC_NWCORNER]  = ct->Edge[TC_NWCORNER];
  Center.Edge[TC_NECORNER]  = ct->Edge[TC_NECORNER];
  //-----World coordinates --------------------------------------
  //Center.Coord  = ct->Coord;
	Center.CopyCOORD(*ct);
  return;
}
//----------------------------------------------------------------------------
//  Draw QUAD contour
//----------------------------------------------------------------------------
void CmQUAD::Contour()
{ //if (!globals->clk->GetON())								return;	// Blink
	glDisableClientState(GL_VERTEX_ARRAY);
	glPushMatrix();
  CVertex   *sw = Center.Edge[TC_SWCORNER];
  CVertex   *nw = Center.Edge[TC_NWCORNER];
  CVertex   *ne = Center.Edge[TC_NECORNER];
  CVertex   *se = Center.Edge[TC_SECORNER];
  float     col[4];
  D3_VERTEX v;
  U_CHAR    dep = glIsEnabled(GL_DEPTH_TEST);
  U_CHAR    lit = glIsEnabled(GL_LIGHTING);
  glGetFloatv(GL_CURRENT_COLOR,col);
  //---Disable depth and set red color -----
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glColor3f(1,0,0);
  glLineWidth(4);

  //----------------------------------------
  glBegin(GL_LINE_LOOP);
	CVertex   *vt = sw;
	//--- Draw South border ------------------
	while (vt != se)
	{	StoreIn(&v,vt); glVertex3d(v.VT_X,v.VT_Y,v.VT_Z); vt = vt->GetEdge(TC_EAST); }
	//--- Draw East Border -------------------
	while (vt != ne)
	{	StoreIn(&v,vt);	glVertex3d(v.VT_X,v.VT_Y,v.VT_Z); vt = vt->GetEdge(TC_NORTH); }
	//--- Draw North border -----------------
	while (vt != nw)
	{	StoreIn(&v,vt);	glVertex3d(v.VT_X,v.VT_Y,v.VT_Z);	vt = vt->GetEdge(TC_WEST); }
	//--- Draw West border -------------------
	while (vt != sw)
	{	StoreIn(&v,vt);	glVertex3d(v.VT_X,v.VT_Y,v.VT_Z);	vt = vt->GetEdge(TC_SOUTH); }
  glEnd();

  //---Restore depth and color --------------
  if (dep) glEnable(GL_DEPTH_TEST);
  if (lit) glEnable(GL_LIGHTING);
  glColor4fv(col);
  glPopMatrix();
	glEnableClientState(GL_VERTEX_ARRAY);
	glLineWidth(1);
  return;
}
//-------------------------------------------------------------------------
//	Clean Vertex data for reallocation
//-------------------------------------------------------------------------
void CmQUAD::Clean()
{	if (iBUF)	delete [] iBUF;
	vTab = 0;
	return;
}
//-------------------------------------------------------------------------
//	Assign world coordinates from vertex v
//	In East  border, supplies full arcsec width	 of QGT
//	In North border, supplies full arcsec height of QGT
//-------------------------------------------------------------------------
void	CmQUAD::StoreIn(D3_VERTEX *d, CVertex *v)
{	U_INT qx = Center.keyX();
	d->VT_X  = (v->SameXQGT(qx))?(v->GetRX()):(TC_ARCS_PER_QGT);
	U_INT qz = Center.keyZ();
	d->VT_Y  = (v->SameZQGT(qz))?(v->GetRY()):(LatitudeIncrement(FN_QGT_FROM_INDX(qz)));
	d->VT_Z  =  v->GetRZ();
}
//-------------------------------------------------------------------------
//	Assign world coordinates from vertex v
//	In East  border, supplies full arcsec width	 of QGT
//	In North border, supplies full arcsec height of QGT
//-------------------------------------------------------------------------
void CmQUAD::AssignGeoCOORD(TC_GTAB *t, CVertex *v)
{	U_INT qx	= Center.keyX();
	t->GT_X		= (v->SameXQGT(qx))?(v->GetRX()):(TC_ARCS_PER_QGT);
	U_INT qz	= Center.keyZ();
	t->GT_Y		= (v->SameZQGT(qz))?(v->GetRY()):(LatitudeIncrement(FN_QGT_FROM_INDX(qz)));
	t->GT_Z		=  v->GetRZ();
}
//-------------------------------------------------------------------------
//  Allocate QUAD vertex table
//  trs:    Texture coordinates table
//  xk:     QGT X index
//	NOTE: We compute 2 items in iBUF
//	a) An index in the SuperTile vBUF table (vertex descriptor) giving
//		the first vertex of each subquad present in the Detail Tile
//	b) A count of vertices in each subquad.
//	Those two items are then used in the glMultiDrawArray() primitive
//-------------------------------------------------------------------------
int CmQUAD::InitVTAB(TC_GTAB *vbo,int deb, U_CHAR res)
{ CmQUAD  *qad	= qARR;
  TC_GTAB	*dst	= vbo;
	int      tot  = 0;
	int      inx  = deb;
	//--- Update first and count lists -------
	for (U_SHORT k = 0; k != qDim; k++,qad++)
	{ iBUF[k]			= inx;
	  qad->SetGTAB(dst);
	  int nbv			= qad->InitVertexCoord(dst,TexRES[res]);
		iBUF[qDim+k]= nbv;
		inx        += nbv;
		tot        += nbv;
		dst        += nbv;
	}
  return tot;
}
//-------------------------------------------------------------------------
//    Allocate vertex table
//    txt is the table corresponding to the resolution (medium or Hight)
//		This table gives the texture coordinates S and T corresponding to
//		the vertex indice in the mesh
//	NOTE:  The first vertex address of each subquad is saved in vTab.
//		It is used each time the texture coordinates need to be
//		refreshed due to a change in resolution.
//-------------------------------------------------------------------------
int  CmQUAD::InitVertexCoord(TC_GTAB *vbo,float *txt)
{//	AreWe(337,31,313,0);
 //	AreWe(338, 0,313,0);
	char       sh = globals->tcm->GetFactEV();			// Shift factor	
	char			 fx = globals->tcm->GetLastEVindex();
	CVertex   *ct = GetCenter();
  CVertex   *sw = ct->GetEdge(TC_SWCORNER);
  CVertex   *se = ct->GetEdge(TC_SECORNER);
  CVertex   *ne = ct->GetEdge(TC_NECORNER);
  CVertex   *nw = ct->GetEdge(TC_NWCORNER);
	//--- Request vertex space ----------------------------------
  TC_GTAB  *dst = vbo;
	//-----------------------------------------------------------
	float			S;														// S coordinate
	float			T;														// T coordinate
  //----Store center values -----------------------------------
  dst->GT_S  = txt[ct->S_Coord(sh)];			// S coordinate
  dst->GT_T  = txt[ct->T_Coord(sh)];			// T coordinate
	AssignGeoCOORD(dst,ct);
  dst++;																	// Next entry
  //----Process South border ----------------------------------
  CVertex *vt = sw;												// Start with SW corner
	T		= txt[sw->T_Coord(sh)];							// T value
	vt	= sw;
	while (vt != se)												// Stop at SE corner
	{	dst->GT_S  = txt[vt->S_Coord(sh)];		// S coordinate
		dst->GT_T  = T;												// T coordinate
		AssignGeoCOORD(dst,vt);
		vt         = vt->Edge[TC_EAST];				// Next vertex
		dst++;																// Next slot
	}
  //----Process EAST border -----------------------------------
	S		= txt[vt->ES_Coord(sh,fx)];					// S value
  while (vt != ne)												// Stop at NE corner
  { dst->GT_S  = S;												// S coordinate
    dst->GT_T  = txt[vt->T_Coord(sh)];		// T coordinate
		AssignGeoCOORD(dst,vt);
    vt         = vt->Edge[TC_NORTH];			// Next vertex
    dst++;																// Next slot
  }
  //----Process North border -----------------------------------
	T			= txt[vt->NT_Coord(sh,fx)];				// T value
	//--- NE corner is special. it must have previous S coord----
	dst->GT_S	= S;													// S value
	dst->GT_T	= T;													// T value
	AssignGeoCOORD(dst,vt);
	vt				= vt->Edge[TC_WEST];					// Next vertice
	dst++;																	// Next slot
	//--- process other north vetices ----------------------------
	while (vt != nw)												// Stop at NW corner
	{	dst->GT_S  = txt[vt->S_Coord(sh)];		// S coordinate
		dst->GT_T  = T;												// T coordinate
		AssignGeoCOORD(dst,vt);
		vt         = vt->Edge[TC_WEST];				// Next vertex
		dst++;																// Next entry
	}
	//----Process West border ------------------------------------
	S			= txt[vt->S_Coord(sh)];						// S value
	//--- NW corner is special. it must have previous T coord----
	dst->GT_S	= S;													// S value
	dst->GT_T	= T;													// T value
	AssignGeoCOORD(dst,vt);
	vt				= vt->Edge[TC_SOUTH];					// Next vertice
	dst++;																	// Next slot
	//--- Process other WEST vertices ----------------------------
	while (vt != sw)												// Stop at SW corner
	{	dst->GT_S	= S;												// S coordinate
		dst->GT_T = txt[vt->T_Coord(sh)];			// T coordinate
		AssignGeoCOORD(dst,vt);
		vt				= vt->Edge[TC_SOUTH];				// Next vertex
		dst++;																// Next entry
		}
  //----Duplicate SW corner ------------------------------------
	*dst	= vbo[1];
  //------------------------------------------------------------
  return nvtx;
}
//-------------------------------------------------------------------------
//  Allocate QUAD vertex table
//  trs:    Texture coordinates table
//  xk:     QGT X index
//-------------------------------------------------------------------------
void CmQUAD::RefreshVTAB(CSuperTile *sp,U_CHAR res)
{ CmQUAD *qd = qARR;
	for (int k = 0; k != qDim; k++,qd++) qd->RefreshVertexCoord(sp,TexRES[res]);
	sp->LoadVBO();
  return;
}

//-------------------------------------------------------------------------
//    Refresh texture coordinates in vertex table
//    txt is the table corresponding to the resolution (medium or Hight)
//		This table gives the texture coordinates S and T corresponding to
//		the vertex indice in the mesh
//	NOTE:  Refresh is called each time the SuperTile change resolution.
//		As textures are used with a margin, the texture coordinates need
//		to be changed.
//		We also need to refresh the elevation because, the first time
//		the SuperTile is activated, the mesh may have been updated with
//		detailled elevations
//-------------------------------------------------------------------------
void  CmQUAD::RefreshVertexCoord(CSuperTile *sp,float *txt)
{// AreWe(337,31,313,0);
 //	AreWe(338, 0,313,0);
	char			 fx = globals->tcm->GetLastEVindex();
	CVertex   *ct = GetCenter();
  CVertex   *sw = ct->GetEdge(TC_SWCORNER);
  CVertex   *se = ct->GetEdge(TC_SECORNER);
  CVertex   *ne = ct->GetEdge(TC_NECORNER);
  CVertex   *nw = ct->GetEdge(TC_NWCORNER);
  TC_GTAB  *dst = vTab;
	//TRACE("QUAD (%08x-%08x)--------------------------",ct->xKey, ct->zKey);
	//-----------------------------------------------------------
	float			S;														// S coordinate
	float			T;														// T coordinate
  //----Store center values -----------------------------------
  dst->GT_S  = txt[ct->indS];							// S coordinate
  dst->GT_T  = txt[ct->indT];							// T coordinate
	dst->GT_Z  = ct->GetRZ();								// Z coordinate
	//TRACE("REF GX=%.4lf GY=%.4lf GZ=%.4lf",dst->GT_X,dst->GT_Y,dst->GT_Z);
  dst++;																	// Next entry
  //----Process South border ----------------------------------
  CVertex *vt = sw;												// Start with sSW corner
	T		= txt[sw->indT];										// T value
	vt	= sw;
	while (vt != se)												// Stop at SE corner
	{	dst->GT_S  = txt[vt->indS];						// S coordinate
		dst->GT_T  = T;												// T coordinate
		dst->GT_Z  = vt->GetRZ();							// Z coordinate
	//TRACE("REF GX=%.4lf GY=%.4lf GZ=%.4lf",dst->GT_X,dst->GT_Y,dst->GT_Z);
		vt         = vt->Edge[TC_EAST];				// Next vertex
		dst++;																// Next slot
	}
  //----Process EAST border -----------------------------------
	S		= txt[vt->inES];										// S value
  while (vt != ne)												// Stop at NE corner
  { dst->GT_S  = S;												// S coordinate
    dst->GT_T  = txt[vt->indT];						// T coordinate
		dst->GT_Z  = vt->GetRZ();							// Z coordinate
	//TRACE("REF GX=%.4lf GY=%.4lf GZ=%.4lf",dst->GT_X,dst->GT_Y,dst->GT_Z);
    vt         = vt->Edge[TC_NORTH];			// Next vertex
    dst++;																// Next slot
  }
  //----Process North border -----------------------------------
	T			= txt[vt->inNT];									// T value
	//--- NE corner is special. it must have previous S coord----
	dst->GT_S	= S;													// S value
	dst->GT_T	= T;													// T value
	dst->GT_Z = vt->GetRZ();							  // Z coordinate
	//TRACE("REF GX=%.4lf GY=%.4lf GZ=%.4lf",dst->GT_X,dst->GT_Y,dst->GT_Z);
	vt				= vt->Edge[TC_WEST];					// Next vertice
	dst++;																	// Next slot
	//--- process other north vetices ----------------------------
	while (vt != nw)												// Stop at NW corner
	{	dst->GT_S  = txt[vt->indS];						// S coordinate
		dst->GT_T  = T;												// T coordinate
		dst->GT_Z  = vt->GetRZ();							// Z coordinate
	//TRACE("REF GX=%.4lf GY=%.4lf GZ=%.4lf",dst->GT_X,dst->GT_Y,dst->GT_Z);
		vt         = vt->Edge[TC_WEST];				// Next vertex
		dst++;																// Next entry
	}
	//----Process West border ------------------------------------
	S			= txt[vt->indS];									// S value
	//--- NW corner is special. it must have previous T coord----
	dst->GT_S	= S;													// S value
	dst->GT_T	= T;													// T value
	dst->GT_Z = vt->GetRZ();							  // Z coordinate
	//TRACE("REF GX=%.4lf GY=%.4lf GZ=%.4lf",dst->GT_X,dst->GT_Y,dst->GT_Z);

	vt				= vt->Edge[TC_SOUTH];					// Next vertice
	dst++;																	// Next slot
	//--- Process other WEST vertices ----------------------------
	while (vt != sw)												// Stop at SW corner
	{	dst->GT_S	= S;												// S coordinate
		dst->GT_T = txt[vt->indT];						// T coordinate
		dst->GT_Z = vt->GetRZ();							// Z coordinate
	//TRACE("REF GX=%.4lf GY=%.4lf GZ=%.4lf",dst->GT_X,dst->GT_Y,dst->GT_Z);

		vt				= vt->Edge[TC_SOUTH];				// Next vertex
		dst++;																// Next entry
		}
  //----Duplicate SW corner -------------------------------------
	*dst	= vTab[1];
	//TRACE("REF GX=%.4lf GY=%.4lf GZ=%.4lf",dst->GT_X,dst->GT_Y,dst->GT_Z);

  //------------------------------------------------------------
  return;
}
//-------------------------------------------------------------------------
//  Locate P in one of the triangle Quad
//  p   => Point coordinates relative to the XZ bandes
//  ct  => Quad center
//  1) Locate in which triangle of the Quad the point is located
//  2) The point coordinates is then expressed in term of the
//     triangles.
//  3) The height is interpolated from formula 2
//-------------------------------------------------------------------------
//  Locate the triangle in quad Q where point P resides and
//  extrapolate altitude
//  1) Locate the quadrant where the point reside
//	2) Call the associated routine
//	NOTE:  Due to rounding problem between double and float
//			the coordinates of point P is trimmed to the quad limits
//-------------------------------------------------------------------------
bool CmQUAD::PointHeight(CVector &p,CVector &nm)
{ CVertex *ct = GetCenter();
  //--- Locate the quadrant ------------------------
  if (ct->IsAbove(p.y))
  { if (ct->ToRight(p.x))	return PointInSW(p,nm);
    else									return PointInSE(p,nm);
  }
  if (ct->ToRight(p.x))		return PointInNW(p,nm);
  return PointInNE(p,nm);
}
//-------------------------------------------------------------------------
/// Check for Point in Quad
//  
//-------------------------------------------------------------------------
bool CmQUAD::PointInQuad(CVector &p)
{	CVertex *sw = GetCorner(TC_SWCORNER);
	if (p.x < sw->GetRX())	return false;
	CVertex *se = GetCorner(TC_SECORNER);
	if (p.x > se->GetRX())	return false;
	if (p.y < se->GetRY())	return false;
	CVertex *ne = GetCorner(TC_NECORNER);
	if (p.y > ne->GetRY())	return false;
	return false;
}

//-------------------------------------------------------------------------
//  Check for point in SW quadran
//	-Look from NW corner to SE corner
//		QUAD configuration is
//			NW --M4 ---NE
//			|     |     |
//			M2 --CT ---M3
//			|     |     |
//			SW---M1----SE
//-------------------------------------------------------------------------
bool CmQUAD::PointInSW(CVector &pp, CVector nm)
{ CVertex *va = GetCenter();
  CVertex *vb = va->VertexNW();       // NW corner
	CVector  pb = vb->RelativeFrom(*va);
	//---Compute vector  A and P----------------
	CVector  pa(va->GetRX(),va->GetRY(),va->GetWZ());
	pp  =    pp - pa;
	//---Look along west border of QUAD --------
	while (vb != va->VertexSW())
	{	CVertex *vc  = vb->VertexSB();				// Go south
		CVector  pc  = vc->RelativeFrom(*va);
		if (PointInTriangle(pp,pa,pb,pc,nm)) return true;
		vb	= vc;
		pb	= pc;
	}
  //---Look along south border ----------------
	while (vb != va->VertexSE())
	{	CVertex *vc = vb->VertexEB();				// Go East
		CVector  pc = vc->RelativeFrom(*va);
		if (PointInTriangle(pp,pa,pb,pc,nm)) return true;
		vb	= vc;
		pb	= pc;
	}

  return false;
}
//-------------------------------------------------------------------------
//  Check for point in SE quadran
//	-Look from SW corner to NE corner
//-------------------------------------------------------------------------
bool CmQUAD::PointInSE(CVector &pp, CVector nm)
{ CVertex *va = GetCenter();
  CVertex *vb = va->VertexSW();       // SW corner
  CVector  pb = vb->RelativeFrom(*va);
	
  //---Compute vector  A and P----------------
  CVector  pa(va->GetRX(),va->GetRY(),va->GetWZ());
  pp  =    pp - pa;
	//--- Look along south border ----------------
	while (vb != va->VertexSE())
	{	CVertex *vc = vb->VertexEB();			// going east
		CVector  pc = vc->RelativeFrom(*va);
		if (PointInTriangle(pp,pa,pb,pc,nm)) return true;
		vb	= vc;
		pb	= pc;
	}
  //--- Look along East border -----------------
	while (vb != va->VertexNE())
	{	CVertex *vc = vb->VertexNB();				// Go north
		CVector  pc = vc->RelativeFrom(*va);
		if (PointInTriangle(pp,pa,pb,pc,nm)) return true;
		vb	= vc;
		pb	= pc;
	}

  return false;
}
//-------------------------------------------------------------------------
//  Check for point in NE quadran
//  -Look from SE corner to NW corner
//-------------------------------------------------------------------------
bool CmQUAD::PointInNE(CVector &pp, CVector nm)
{ CVertex *va = GetCenter();
  CVertex *vb = va->VertexSE();       // SE corner
  CVector  pb = vb->RelativeFrom(*va);

  //---Compute vector  A and P----------------
  CVector  pa(va->GetRX(),va->GetRY(),va->GetWZ());
  pp  =    pp - pa;
	//--- look along East border ----------------
	while	(vb != va->VertexNE())
	{	CVertex *vc = vb->VertexNB();				// Going north
		CVector  pc = vc->RelativeFrom(*va);
		if (PointInTriangle(pp,pa,pb,pc,nm)) return true;
		vb	= vc;
		pb  = pc;
	}
  //--- look along north border ---------------
	while (vb != va->VertexNW())
	{	CVertex *vc = vb->VertexWB();			// Going West
		CVector  pc = vc->RelativeFrom(*va);
		if (PointInTriangle(pp,pa,pb,pc,nm)) return true;
		vb	= vc;
		pb  = pc;
	}
	
  return false;
}
//-------------------------------------------------------------------------
//  Check for point in NW quadran
//	-Look from NE corner to SW corner
//-------------------------------------------------------------------------
bool  CmQUAD::PointInNW(CVector &pp, CVector nm)
{ CVertex *va = GetCenter();
  CVertex *vb = va->VertexNE();       // NE corner
  CVector  pb = vb->RelativeFrom(*va);
	
  //---Compute vector  A and P---------------------
  CVector  pa(va->GetRX(),va->GetRY(),va->GetWZ());
  pp  =    pp - pa;
  //--- look along north border -------------------
	while (vb != va->VertexNW())
	{	CVertex *vc = vb->VertexWB();			// Going west
		CVector  pc = vc->RelativeFrom(*va);
		if (PointInTriangle(pp,pa,pb,pc,nm)) return true;
		vb	= vc;
		pb	= pc;
	}
	//--- look along west border -------------------
	while (vb != va->VertexSW())
	{	CVertex *vc = vb->VertexSB();			// Going south
	  CVector  pc = vc->RelativeFrom(*va);
		if (PointInTriangle(pp,pa,pb,pc,nm)) return true;
		vb	= vc;
		pb	= pc;
	}
	
  //------------------------------------------------
  return false;
}
//-------------------------------------------------------------------------
//  Replace Quad pointer to Array pointer
//  
//-------------------------------------------------------------------------
void  CmQUAD::SetArray(CmQUAD *qd,U_SHORT dm)
{ qDim  = dm * dm;
  qARR  = qd;
  return;
}
//-------------------------------------------------------------------------
//    Count vertices in the elementary quad
//	NOTE: It is important that vertices should be counted in the same
//				order as they are allocated in the VBO. If not, the vertex
//				indices in iBUF wont correspond to the vertex coordinates
//-------------------------------------------------------------------------
int  CmQUAD::CountVertices()
{	CVertex *sw = GetCorner(TC_SWCORNER);
	CVertex *se = GetCorner(TC_SECORNER);
	CVertex *ne = GetCorner(TC_NECORNER);
	CVertex *nw = GetCorner(TC_NWCORNER);
	int     nbv = 2;    // include center and first twice
	//-----scan South border --------------------------
	CVertex *vt = sw;
	while (vt != se)  { nbv++; vt = vt->Edge[TC_EAST]; }
	//----scan East border -----------------------------
	while (vt != ne)  { nbv++; vt = vt->Edge[TC_NORTH];}
	//----scan North border-----------------------------
	while (vt != nw)  { nbv++; vt = vt->Edge[TC_WEST]; }
	//----scan West border -----------------------------
	while (vt != sw)  { nbv++; vt = vt->Edge[TC_SOUTH];}
	nvtx = nbv;
	return nbv;
}
//-------------------------------------------------------------------------
//    Count vertices in the quad
//		Allocate a list of first index and count for Draw primitive
//-------------------------------------------------------------------------
int CmQUAD::InitIndices(CSuperTile *sp,char opt)
{	int nbv	= 0;
  CmQUAD *qd = qARR;
	for (int k=0; k < qDim; k++,qd++)	nbv += qd->CountVertices();
	//--- Allocate list of first index and count --------------
	int  dim = (qDim << 1) + 2;
	msp		= sp;
	iBUF	= new GLint[dim];
	for (int n=0; n<dim; n++) iBUF[n] = '@';
	if (opt)	RenderVBO();
	return nbv;
}
//-------------------------------------------------------------------------
//	Return the total vertices in the Detail Tile
//-------------------------------------------------------------------------
int CmQUAD::NbrVerticesInTile()
{	int    nbv = 0;
  CmQUAD *qd = qARR;
	for (int k=0; k < qDim; k++,qd++)	nbv += qd->nvtx;
	return nbv;
}
//-------------------------------------------------------------------------
//  return the tile indices
//-------------------------------------------------------------------------
void CmQUAD::GetTileIndices(U_INT &tx,U_INT &tz)
{ tx  =  GetTileAX();
  tz  =  GetTileAZ();
  return;
}
//-------------------------------------------------------------------------
/// Locate quad where the position is lying
//  p coordinates must be local to SW corner of QGT
//-------------------------------------------------------------------------
CmQUAD *CmQUAD::Locate2D(CVector &p)
{ double lon  = p.x;
	double lat	= p.y;
	if (IsaQuad())    return this;
  CmQUAD *qd = qARR;
	//--- Search quad along south border ------------
  for (int k = 0; k != qDim; k++, qd++)
  { CVertex *sw = qd->GetCorner(TC_SWCORNER);
    if (lon < sw->GetRX())    continue;
    if (lat < sw->GetRY())    continue;
    CVertex *ne = qd->GetCorner(TC_NECORNER);
    if (lon > ne->GetRX())    continue;
    if (lat > ne->GetRY())    continue;
    return qd->Locate2D(p); 
  }
	
  return this;
}
//-------------------------------------------------------------------------
/// Return Super Tile number where this QUAD resides [0-63]
//-------------------------------------------------------------------------
U_INT CmQUAD::GetSuperNo()
{ U_INT sx = FN_DET_FROM_INDX(Center.xKey) >> TC_BY04;
  U_INT sz = FN_DET_FROM_INDX(Center.zKey) >> TC_BY04;
  return (sz << TC_BY08) | sx;
}
//-------------------------------------------------------------------------
/// Return internal Tile in QGT in [0-1023]
//-------------------------------------------------------------------------
U_INT CmQUAD::GetTileNo()
{ U_INT tx = FN_DET_FROM_INDX(Center.xKey);				// Tile X index
	U_INT tz = FN_DET_FROM_INDX(Center.zKey);				// Tile Z index
	return FN_DET_FROM_XZ(tx,tz);
}
//-------------------------------------------------------------------------
/// Return tile world tile key : Concatenation of
//	QGT(X)-DET(X): 14 bits		upper half world
//	QGT(Z)-DET(Z): 14 bits    lower half world
//-------------------------------------------------------------------------
U_INT CmQUAD::WorldTileKey()
{	U_INT wx = FN_ABS_QGT_DET(Center.xKey);				// QGT-DET index
	U_INT wz = FN_ABS_QGT_DET(Center.zKey);				// QGT_DET index
	return (wx << 16 | wz);
}
//-------------------------------------------------------------------------
//  Draw the ground detail in normal mode (no VBO)
//-------------------------------------------------------------------------
void CmQUAD::DrawNML()
{ //--- For final quad, draw the detail tile ---------------
	CmQUAD *qd = qARR;
	for (U_INT k = 0; k != qDim; k++,qd++)
	{	TC_GTAB *tab = qd->GetVTAB();
		int			 nbv = qd->GetNbrVTX();
		glVertexPointer  (3,UNIT_OPENGL,sizeof(TC_GTAB),&tab[0].GT_X);
		glTexCoordPointer(2,UNIT_OPENGL,sizeof(TC_GTAB),tab);
		glDrawArrays(GL_TRIANGLE_FAN,0,nbv);
	}
  //---- Draw contour if Terra Browser is active -----------
  if (globals->aPROF.Not(PROF_DR_DET))	return;
  if (!globals->tcm->PlaneQuad(this))		return;
  return Contour();
}
//-------------------------------------------------------------------------
//  Draw the ground detail with list of indices
//-------------------------------------------------------------------------
void CmQUAD::DrawIND()
{ int  *count = iBUF + qDim;
	TC_GTAB *vt	= msp->GetVertexTable();
	//--- For final quad, draw the detail tile ---------------
	glVertexPointer  (3,UNIT_OPENGL,sizeof(TC_GTAB),&vt[0].GT_X);
	glTexCoordPointer(2,UNIT_OPENGL,sizeof(TC_GTAB), vt);
	glMultiDrawArrays(GL_TRIANGLE_FAN,iBUF,count,qDim);
  //---- Draw contour if Terra Browser is active -----------
  if (globals->aPROF.Not(PROF_DR_DET))	return;
  if (!globals->tcm->PlaneQuad(this))		return;
  return Contour();
}
//-------------------------------------------------------------------------
//  Draw the ground detail with VBO
//-------------------------------------------------------------------------
void CmQUAD::DrawVBO()
{ int  *count = iBUF + qDim;
	//--- For final quad, draw the detail tile ---------------
	glMultiDrawArrays(GL_TRIANGLE_FAN,iBUF,count,qDim);
  //---- Draw contour if Terra Browser is active -----------
  if (globals->aPROF.Not(PROF_DR_DET))	return;
  if (!globals->tcm->PlaneQuad(this))		return;
  return Contour();
}
//-------------------------------------------------------------------------
//  Get list of vertices for Terra editor
//-------------------------------------------------------------------------
void CmQUAD::GetVertices(TRACK_EDIT &wrk)
{	CmQUAD *qd	= qARR;
	wrk.vNum		= 0;
	wrk.subq	 = subL;
	for (int k = 0; k != qDim; k++, qd++) { PutVertices(wrk,qd);}
	return;
}
//-------------------------------------------------------------------------
//  Get list of vertices
//-------------------------------------------------------------------------
void CmQUAD::PutVertices(TRACK_EDIT &wrk, CmQUAD *qd)
{	//--- Start with Center vertice -------------------
	//--- Store center vertex -------------------------
	CVertex *ct = qd->GetCenter();
	StoreVertex(wrk,ct);
	//--- Explore all borders -------------------------
	CVertex *vt = ct->GetCorner(TC_SWCORNER);
	//--- Explore south border ------------------------
	CVertex *se = ct->GetCorner(TC_SECORNER);
	while (vt != se)	{StoreVertex(wrk,vt); vt = vt->GetEdge(TC_EAST);}
	//--- Explore East border -------------------------
	CVertex *ne = ct->GetCorner(TC_NECORNER);
	while (vt != ne)	{StoreVertex(wrk,vt); vt = vt->GetEdge(TC_NORTH);}
	//--- Explore North Border ------------------------
	CVertex *nw = ct->GetCorner(TC_NWCORNER);
	while (vt != nw)	{StoreVertex(wrk,vt); vt = vt->GetEdge(TC_WEST);}
	//--- Explore West border -------------------------
	CVertex *sw = ct->GetCorner(TC_SWCORNER);
	while (vt != sw)	{StoreVertex(wrk,vt); vt = vt->GetEdge(TC_SOUTH);}
	//-------------------------------------------------
	return;
}
//-------------------------------------------------------------------------
//  Save vertex
//	lk is used to store vertex keys and avoid dupplicate
//	lv is used to draw and save vertices
//-------------------------------------------------------------------------
void CmQUAD::StoreVertex(TRACK_EDIT &wrk, CVertex *vt)
{ //--- Look into the table ----------------------------
	U_INT key			= vt->VertexKey();
	TVertex *vtx  = wrk.lvx;
	for (U_INT k=0; k<wrk.vNum; k++,vtx++)	if (key == vtx->key)	return;
	//---- Store the new vertex --------------------------
	vtx->key			= key;						// Vertex absolute key
	vtx->vnum			= wrk.vNum++;			// Vertex number
	vtx->vrt			= vt;							// Vertex pointer
	return;
}
//-------------------------------------------------------------------------
//  Patche elevations
//-------------------------------------------------------------------------
void CmQUAD::ProcessPatche(ELV_PATCHE &p)
{	CmQUAD *qd = qARR;
	for (int k = 0; k != qDim; k++, qd++) qd->PatchVertices(p);
	return;
}
//-------------------------------------------------------------------------
// Seach all vertices
//-------------------------------------------------------------------------
void CmQUAD::PatchVertices(ELV_PATCHE &p)
{ Patche(&Center,p,TC_SOUTH);
	CVertex *se = GetCorner(TC_SECORNER);
	CVertex *vt = GetCorner(TC_SWCORNER);
	while (vt != se)	vt = Patche(vt,p,TC_EAST); 
	CVertex *ne = GetCorner(TC_NECORNER);
	while (vt != ne)	vt = Patche(vt,p,TC_NORTH);
	CVertex *nw = GetCorner(TC_NWCORNER);
	while (vt != nw)	vt = Patche(vt,p,TC_WEST);
	CVertex *sw = GetCorner(TC_SWCORNER);
	while (vt != sw)	vt = Patche(vt,p,TC_SOUTH);
	return;
}
//-------------------------------------------------------------------------
//  Patche elevations
//	dir = 0 ==>  Store elevation from vertex to patche
//	dir = 1 =>   Store patche into vertex elevation 
//-------------------------------------------------------------------------
CVertex *CmQUAD::Patche(CVertex *vt,ELV_PATCHE &p,int nxt)
{	
	U_INT key			= vt->VertexKey();
	ELV_VTX *mat	= p.mat;
	int			  k		= 0;
	//--- Process according to direction -------------------
	switch (p.dir)	{
		//--- Patche out: Save elevation in  table ------
		case 0:
			for (k=0; k!=p.nbe; k++) if (key == mat[k].key) break;
			mat[k].key	= key;
			mat[k].elv	= vt->GetWZ();
			p.nbe++;
			break;
		//--- Patch in: modify vertex elevation -------------
		case 1:
			for (k=0; k!=p.nbe; k++)
			{	if (key == mat[k].key) {vt->SetWZ(mat[k].elv); break;}
			}
	}

	return vt->GetEdge(nxt);
}
//=========================================================================
//  QGT Queue
//=========================================================================
qgtQ::qgtQ()
{ pthread_mutex_init (&mux,NULL);
  First = 0;
  Last  = 0;
}
//------------------------------------------------------------------
//  Set new item at end
//------------------------------------------------------------------
void qgtQ::PutLast(C_QGT *tq)
{ pthread_mutex_lock (&mux);
  C_QGT *end  = Last;
  tq->Next    = 0;
  Last        = tq;
  if (end)    end->Next = tq;
  if (0 == First) First = tq;
  pthread_mutex_unlock (&mux);
  return;
}
//-----------------------------------------------------------------
//  Set new item at first
//-----------------------------------------------------------------
void qgtQ::PutHead(C_QGT *tq)
{ pthread_mutex_lock (&mux);
  tq->Next    = First;
  First       = tq;
  if (0 == Last)  Last = tq;
  pthread_mutex_unlock (&mux);
  return;
}
//-----------------------------------------------------------------
//  Get The First Item
//-----------------------------------------------------------------
C_QGT *qgtQ::Pop()
{ pthread_mutex_lock (&mux);
  C_QGT *deb = First;
  if (deb)    First = deb->Next;
  if (deb)    deb->Next = 0;
  if (First == 0) Last  = 0;
  pthread_mutex_unlock (&mux);
  return deb;
}
//=========================================================================
//  SUPER TILE
//=========================================================================
CSuperTile::CSuperTile()
{ Next    = 0;
  MiniH   = 0;
  MaxiH   = 0;
  Reso    = 0;
  aRes    = 0;
  swap    = 1;                      // Allow texture swap by default
  alpha   = 0;
  LOD     = 0;
	obtr		= 0;
  nbVTX   = 0;
	qgt			= 0;
  sta3D   = TC_3D_OUTSIDE;
  white[0] = white[1] = white[2] = white[3] = 1;
	//------------------------------------------------
	State   = TC_TEX_NEW;
	visible = 0;
	//------------------------------------------------
	vBUF		= 0;
	aVBO		= 0;
}
//-------------------------------------------------------------------------
//  Free the VBO buffer
//-------------------------------------------------------------------------
CSuperTile::~CSuperTile()
{ if (vBUF)	delete [] vBUF;
	if (aVBO) glDeleteBuffers(1,&aVBO);
	if (obtr)	TraceEnd();
}
//-------------------------------------------------------------------------
//  Trace object release
//-------------------------------------------------------------------------
void CSuperTile::TraceEnd()
{	if (0 == qgt)		return;
  U_INT qx = qgt->GetXkey();
	U_INT qz = qgt->GetZkey();
	int   nb = woQ.NbObjects();
	TRACE("QGT(%03d-%03d) ST%02d free %05d Objects",qx,qz,NoSP,nb);
}
//-------------------------------------------------------------------------
// Seach a part with same reference as the given one
//-------------------------------------------------------------------------
C3DPart *CSuperTile::SearchOSMPart(CShared3DTex *ref)
{	C3DPart *prt = 0;
	for (prt = batQ.GetFirst(); (prt != 0); prt = prt->Next())
	{	if (!prt->SameREF(ref))			continue;
		U_INT nbv = prt->GetNBVTX();
		if (nbv < globals->pakCAP)	return prt;
		break;
	}
	//--- Allocate a new 3D part --------------------------
	prt	= new C3DPart();
	prt->Reserve(ref); 
	//--- Add a new part -----------------------------
	batQ.PutHead(prt);
	return prt;
}
//-------------------------------------------------------------------------
// Add osm object to try
//-------------------------------------------------------------------------
void CSuperTile::AddToPack(OSM_Object *obj)
{	//--- Check for object --------------------
	double rad = FN_RAD_FROM_ARCS(mPos.lat);
	double rdf = cos(rad);
	SPosition pos	= obj->GetPosition();
	CVector T  = FeetComponents(mPos, pos,rdf);
	//--- Search a part with same texture ------
	CShared3DTex *ref = obj->GetPartTREF();
	C3DPart *p0 = obj->GetPart();
	C3DPart *p1 = SearchOSMPart(ref);
	p1->ExtendWith(p0,T);
	return;
}
//-------------------------------------------------------------------------
//  Reload VBO if needed
//-------------------------------------------------------------------------
void CSuperTile::LoadVBO()
{ if (0 == aVBO)		return;
	int	dm = nbVTX * sizeof(TC_GTAB);
	glBindBuffer(GL_ARRAY_BUFFER,aVBO);
	glBufferData(GL_ARRAY_BUFFER,dm,vBUF,GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	return;
}
//-------------------------------------------------------------------------
//		Trace reallocation
//-------------------------------------------------------------------------
void CSuperTile::TraceRealloc(CmQUAD *qd)
{ U_INT qx = qd->GetTileAX() >> TC_BY32;
	U_INT	qz = qd->GetTileAZ() >> TC_BY32;
	U_INT tx = qd->GetTileTX();
	U_INT tz = qd->GetTileTZ();
	U_INT vx = qd->GetTileVX();
	U_INT vz = qd->GetTileVZ();
	TRACE("ELV realloc (%03d-%03d) (%02d-%02d) (%4d-%04d)", qx,qz,tx,tz,vx,vz);
	int asw = qd->GetCorner(TC_SWCORNER)->GetRZ();
	int ase = qd->GetCorner(TC_SECORNER)->GetRZ();
	int ane = qd->GetCorner(TC_NECORNER)->GetRZ();
	int anw = qd->GetCorner(TC_NWCORNER)->GetRZ();
	TRACE ("  sw=%04d se=%04d ne=%04d nw=%04d",asw,ase,ane,anw);
}
//-------------------------------------------------------------------------
//	Reallocate the Super Tile vertices
//-------------------------------------------------------------------------
void CSuperTile::Reallocate(char opt)
{	//--- Clean each quad -----------------------
	for (U_INT nd = 0; nd != TC_TEXSUPERNBR; nd++)
    { CmQUAD *qd = Tex[nd].quad;
			qd->Clean();
		}
	//--- Delete VBO if any ----------------------
	if (vBUF)	delete [] vBUF;
	nbVTX	= 0;
	vBUF	= 0;
	AllocateVertices(opt);
	return;
}

//-------------------------------------------------------------------------
//  Allocate a vertex buffer
//	1) Poll each Detail Tile for vertices count
//	2) Allocate space for vertex parameters
//	3) Request each Detail Tile to initialize vertex list
//	NOTES:  Only one VBO for the super tile collects vertices coordinates
//					for all detail tiles in it.
//					A local copy of Vertices coordinates is held in each detail tile
//-------------------------------------------------------------------------
void CSuperTile::AllocateVertices(char opt)
{	if (nbVTX)		return;
	int tn	= 0;
  //--- Collecte number of vertices needed -----------
	for (U_INT nd = 0; nd != TC_TEXSUPERNBR; nd++)
    { CmQUAD *qd = Tex[nd].quad;
			tn += qd->InitIndices(this,opt);
		}
	//--- Allocate memory ------------------------------
	vBUF		= new TC_GTAB[tn];
  nbVTX		= tn;
	//--- Build vertex table for each Detail Tile ------
	TC_GTAB *dst = vBUF;
	int      inx = 0;
	for (U_INT no = 0; no != TC_TEXSUPERNBR; no++)
    { CmQUAD *qd = Tex[no].quad;
			int nbv		= qd->InitVTAB(dst,inx,TC_MEDIUM);
			inx			 += nbv;
			dst      += nbv;
			//if (opt & 0x02) TraceRealloc(qd);
		}
	//--- Allocate a VBO -------------------------------
	if (opt)
	{	int	dm = nbVTX * sizeof(TC_GTAB);
		if (0 == aVBO) glGenBuffers(1,&aVBO);
		glBindBuffer(GL_ARRAY_BUFFER,aVBO);
		glBufferData(GL_ARRAY_BUFFER,dm,vBUF,GL_STATIC_COPY);
		glBindBuffer(GL_ARRAY_BUFFER,0);
	}
	//--------------------------------------------------
	return;
}
//-----------------------------------------------------------------------------
//  Bind VBO
//-----------------------------------------------------------------------------
void CSuperTile::BindVBO()
{	glBindBuffer(GL_ARRAY_BUFFER,aVBO);
	glVertexPointer  (3,UNIT_OPENGL,sizeof(TC_GTAB),OFFSET_VBO(2 * sizeof(double)) );
	glTexCoordPointer(2,UNIT_OPENGL,sizeof(TC_GTAB),0);
	return;
}
//-------------------------------------------------------------------------
//	Check if super tile is all textured
//-------------------------------------------------------------------------
bool CSuperTile::IsTextured(char opt)
{ CTextureDef *txn = 0;
	if (InFarQ())									return true;
	if (!Visibility())						return true;
	if (IsReady())								return true;
	if ((0 == opt) && NeedOBJ())	return true;
	for (U_INT nd = 0; nd != TC_TEXSUPERNBR; nd++)
    { txn = Tex+nd;
			U_INT xob = txn->GetDOBJ();
			if (xob == 0)		return false;
		}

	return true;
}
//-------------------------------------------------------------------------
//  Check if Super Tile needs medium resolution
//  -rd is the hi resolution radius
//-------------------------------------------------------------------------
bool CSuperTile::NeedMedResolution(float rd)
{ if (TC_TEX_RDY != State)  return false;           // Not ready
  if (dEye <= rd)           return false;           // Still inside hi radius
  if (Reso == TC_MEDIUM)    return false;           // Already set
  if (aRes == TC_MEDIUM)    return false;           // Already requested
  return true;
}
//-------------------------------------------------------------------------
//  Check if Super Tile needs hight resolution
//  -rd is the hight resolution radius
//-------------------------------------------------------------------------
bool CSuperTile::NeedHigResolution(float rd)
{ if (TC_TEX_RDY != State)  return false;           // Not ready
  if (dEye >  rd)           return false;           // Outside hi radius
  if (Reso == TC_HIGHTR)    return false;           // Already set
  if (aRes == TC_HIGHTR)    return false;           // Already requested
  if (0    == swap)         return false;           // Fixed medium resolution
  return true;
}
//-------------------------------------------------------------------------
//  Super Tile inside 3D drawing circle
//  Process according to 3D state
//-------------------------------------------------------------------------
int CSuperTile::Inside3DRing()
{ //-- Update LOD ----------------------------------
  LOD = 0;
  if (dEye > globals->ftLD1)  LOD = 1;
  if (dEye > globals->ftLD2)  LOD = 2;
  if (dEye > globals->ftLD3)  LOD = 3;
  switch(sta3D) {
    case TC_3D_VIEWING:
      return 1;                             // Already inside

    case TC_3D_FADE_IN:
      alpha += 0.01f;                       // Increase visibility
      if (alpha > 1)
			{	alpha  = 1;
				sta3D  = TC_3D_VIEWING;
			}
      return 1;

    case TC_3D_FADEOUT:
      sta3D  = TC_3D_FADE_IN;
      return 1;

    case TC_3D_OUTSIDE:
      sta3D  = TC_3D_FADE_IN;
      alpha  = 0;
      return 1;
    }
  return 1;
}
//-------------------------------------------------------------------------
//  Super Tile outside 3D drawing circle
//  Process according to 3D state
//-------------------------------------------------------------------------
int CSuperTile::Outside3DRing()
{ LOD = 3;
  switch(sta3D) {
    case TC_3D_VIEWING:
      sta3D = TC_3D_FADEOUT;
      return 0;

    case TC_3D_FADE_IN:
      sta3D = TC_3D_FADEOUT;
      return 0;

    case TC_3D_FADEOUT:
      alpha -= 0.01f;
      if (alpha > 0)  return 0;
      alpha  = 0;
      sta3D  = TC_3D_OUTSIDE;
      return 0;

    case TC_3D_OUTSIDE:
      return 0;
    }
  return 0;
}

//-------------------------------------------------------------------------
//  Clear All texture name (TERRA BROWSER usage)
//-------------------------------------------------------------------------
void CSuperTile::RazNames()
{ for (int k = 0; k<TC_TEXSUPERNBR; k++)
 {  CTextureDef *txn = Tex+k;
    if (txn->UserTEX()) continue;
    txn->RazName();
 }
 return;
}

//-------------------------------------------------------------------------
//  Extract border vertices for the Super Tile
//-------------------------------------------------------------------------
void CSuperTile::BuildBorder(C_QGT *qt,U_INT No)
{ qgt	= qt;
	//---Start with center -------------------------------------------
	CVector ctr (mPos.lon,mPos.lat,mPos.alt);
	qgt->RelativeToBase(ctr);
  Tour[0].VT_X   = ctr.x;
  Tour[0].VT_Y   = ctr.y;
  Tour[0].VT_Z   = ctr.z;
  D3_VERTEX *dst = Tour+1;
  //--- Save contour -----------------------------------------------
  int end   = TC_SPTBORDNBR;
  U_INT bx  = SuperDT[No].dx;                      // Base DET X in Super Tile
  U_INT bz  = SuperDT[No].dz;                      // Base DET Z in Super Tile
	
  for (int  k = 0; k != end; k++)
      {U_INT tx = bx | SupBORD[k].cx;           // Border Tile X composite
       U_INT tz = bz | SupBORD[k].cz;           // Border Tile Z composite
       U_INT nt = (tz << TC_BY32) | tx;         // Border Tile index
       U_INT cn = SupBORD[k].msk;               // Corner index
       CmQUAD *qd   = qt->GetQUAD(nt);          // Quad
       CVertex *vt  = qd->GetCorner(cn);        // Vertex
			 qd->StoreIn(dst++,vt);
      }
			
  //--- End with first vertex -------------------------------------
 *dst = Tour[1];
  return;
}
//-------------------------------------------------------------------------
//  Draw the Super Tile contour
//-------------------------------------------------------------------------
void CSuperTile::DrawTour()
{ glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glVertexPointer(3,GL_FLOAT,0,Tour);
  glDrawArrays(GL_TRIANGLE_FAN,0,(TC_SPTBORDNBR + 2));
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);
  return;
}
//-------------------------------------------------------------------------
//	Render as a outter supertile
//-------------------------------------------------------------------------
void CSuperTile::DrawOuterSuperTile()
{	glColor3fv(globals->tcm->GetFogColor());
	glDisable(GL_TEXTURE_2D);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER,0);
  glVertexPointer(3,GL_FLOAT,0,Tour);
  glDrawArrays(GL_TRIANGLE_FAN,0,(TC_SPTBORDNBR + 2));
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);
	return;
}
//-------------------------------------------------------------------------
//  Draw a Supertile if visible
//-------------------------------------------------------------------------
void CSuperTile::DrawInnerSuperTile()
{ CTextureDef *txn = 0;
	//-------------------------------------------------------------------
  if (NeedSWP()) globals->txw->SwapTextures(this);
  if (NeedOBJ()) globals->txw->GetSupOBJ(this);
  //-------------------------------------------------------------------
  U_INT bq = SuperQDB[NoSP];                        // Base Quad
  for (U_INT nd = 0; nd != TC_TEXSUPERNBR; nd++)
    { txn = Tex+nd;                                 // Texture descriptor
      CmQUAD *qd = txn->quad;
      //----Stop when plane enter this tile ----------------------------
      //    Used only for debuging when uncommented   
			//       U_INT  ax = txn->quad->GetTileAX();          // Absolute tile X index
			//       U_INT  az = txn->quad->GetTileAZ();          // Absolute tile Z index 
			//       if (globals->tcm->PlaneInTile(ax,az))        // This is for test only  
			//       ax = ax;                                      // Break point here
      //---Stop for a given detail tile -------------------------------
      //
      //	if (txn->AreWe(TC_ABSOLUTE_DET(509,0),TC_ABSOLUTE_DET(334,18)))
      //    txn = txn;                  // Set break point here
      //
      //--Avoid ground airport ------------------------------------------------
      if (qd->IsAptGround())  continue;
      //-----Draw sea or land texture -----------------------------------------
			if (aVBO)	BindVBO();									//glBindBuffer(GL_ARRAY_BUFFER,aVBO);
			glBindTexture(GL_TEXTURE_2D,txn->dOBJ);
			qd->DrawTile();
      //-----Draw night texture -----------------------------------------------
      TCacheMGR *tcm = globals->tcm;
      if (tcm->IsNight() && txn->nOBJ)
          { glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glMaterialfv (GL_FRONT, GL_EMISSION, tcm->GetNightEmission()); 
						glBindTexture(GL_TEXTURE_2D,txn->nOBJ);
						qd->DrawTile();
            glMaterialfv (GL_FRONT, GL_EMISSION, tcm->GetDeftEmission());
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
          }
			//--- Unbind the vertex buffer ---------------------------------------
			if (aVBO)	glBindBuffer(GL_ARRAY_BUFFER,0);
   }
  //---------------------------------------------------------------------
  return;
}
//-------------------------------------------------------------------------
//	Update distance and alpha blending
//-------------------------------------------------------------------------
bool CSuperTile::Update()
{	if (!Visibility())		return false;
	float lim = globals->ftDRW;         //nm Limit (real feet)
  //--- Update LOD -------------------------------------------
  if (dEye > lim)		Outside3DRing();
	else							Inside3DRing();
	bool OK = (woQ.NotEmpty()) || (batQ.NotEmpty());
	return (OK);
}
//-------------------------------------------------------------------------
//  SuperTile Draw 3D Objects Here
//  Alpha chanel for color is taken from this supertile for fade in/out
//	NOTE:  All ground objects that need polygon offset (noZB) must
//				 be queued in front of queue
//-------------------------------------------------------------------------
int CSuperTile::Draw3D(U_CHAR mod)
{	//----------------------------------------------------------
  CWobj     *obj = 0;
  int        nbo = 0;
//	glPushAttrib(GL_ALL_ATTRIB_BITS);
  white[3]       = alpha;
  glColorMaterial (GL_FRONT_AND_BACK, GL_DIFFUSE);
	glColor4fv(white);
	//--------------------------------------------------------
	bool OK = (globals->noOBJ == 0);
	//----------------------------------------------------------
  for (obj = woQ.GetFirst(); ((obj!= 0) && OK); obj = woQ.GetNext(obj))
  { //----Translate to object origin -------------------------
    obj->Update(mod);
    SVector tr;
    globals->tcm->RelativeFeetTo(*obj->ObjPosition(),tr);
    //--------------------------------------------------------
    glPushMatrix();
    glTranslated(tr.x,tr.y,tr.z);     // To object origin
    if (obj->Rotate())	glRotated(obj->GetYRotation(),0,0,1);
    //---Draw object ----------------------------------------
		if (obj->GetZB())		glDisable(GL_DEPTH_TEST);							//{pof = 0; glPolygonOffset(0,0);}
    obj->DrawModel(mod,LOD);
		if (obj->GetZB())		glEnable(GL_DEPTH_TEST);
    //-------------------------------------------------------
    glPopMatrix();
    nbo++;
  }
	
	//--- Draw the OSM queue ----------------------------------
	if (globals->noOSM)	return nbo;
	glColorMaterial (GL_FRONT, GL_DIFFUSE);
	glPushMatrix();
	CVector T;
	globals->tcm->RelativeFeetTo(mPos,T);
	glTranslated(T.x, T.y, T.z);		// Go to supertile center
	//--- DRAW OSM Objects ------------------------------------
	C3DPart *prt;
	glFrontFace(GL_CCW);
	for (prt = batQ.GetFirst(); prt != 0; prt= prt->Next())	   prt->Draw();
	//----------------------------------------------------------
	glPopMatrix();
  return nbo;
}
//-----------------------------------------------------------------------------
//  Add a line for each object to draw
//-----------------------------------------------------------------------------
void CSuperTile::GetLine(CListBox *box)
{ CWobj     *obj = 0;
  float dmile = FN_MILE_FROM_FEET(dEye);
	if (dmile > 2) return;
  woQ.Lock();
  for (obj = woQ.GetFirst(); obj!=0; obj = woQ.GetNext(obj))
  { CObjLine *lin = new CObjLine(obj);
    box->AddSlot(lin);
  }
  woQ.UnLock();
  return;
}
//-----------------------------------------------------------------------------
//  Add object to draw
//-----------------------------------------------------------------------------
void CSuperTile::Add3DObject(CWobj *obj, char t)
{	obtr	= t;	
	if (obj->NoZB()) woQ.PutHead(obj);
  else             woQ.PutEnd(obj);

  return;
}
//=========================================================================
//  QUEUE of SUPER TILES
//=========================================================================
CSupQueue::CSupQueue()
{ pthread_mutex_init (&mux,  NULL);
  First = 0;
  Last  = 0;
  NbSP  = 0;
}
//------------------------------------------------------------------
//  Put item at end of Queue
//------------------------------------------------------------------
void CSupQueue::PutLast(CSuperTile *sp)
{ CSuperTile *end = Last;
  sp->Next  = 0;
  Last      = sp;
  if (end)  end->Next = sp;
  if (0 == First) First = sp;
  NbSP++;
  return;
}
//------------------------------------------------------------------
//  Pop head of Queue
//------------------------------------------------------------------
CSuperTile *CSupQueue::Pop()
{ CSuperTile  *deb = First;
  if (deb)  First = deb->Next;
  if (deb)   deb->Next = 0;
  if (0 == First) Last = 0;
  if (deb) NbSP--;
  return deb;
}
//------------------------------------------------------------------
//  Detach the current item and return the Next one
//------------------------------------------------------------------
CSuperTile *CSupQueue::Detach(CSuperTile *sp)
{ CSuperTile *nex = sp->Next;
  if (Prev) Prev->Next  = nex;
  else      First       = nex;
  if (sp == Last) Last  = Prev;
  sp->Next  = 0;
  NbSP--;
  return nex;
}
//=========================================================================
//  QUARTER GLOBAL TILE DESCRIPTOR
//  Initialize the QGT descriptor
//
//=========================================================================
C_QGT::C_QGT(U_INT cx, U_INT cz,TCacheMGR *tm)
{ tcm   = tm;               // Cache manager
  qtr   = 0;                // QTR file
  trn   = 0;                // TRN file
  xKey  = cx;
  zKey  = cz;
	qKey	= QGTKEY(cx,cz);
  Step  = TC_QT_INI;
  nStp  = 0;
  qSTAT = 1;
  dead	= 0;
	visb	= 0;
  Metar = 0;
	strn	= tm->GetTRNoption();
	osmDB = 0;
	//--- Get VBO option -----------------------
	int opt = 0;
	GetIniVar("Performances","UseTerrainVBO",&opt);
	vbu			= opt;
	elv			= 0;							// No elevation yet
  //------Set coordinate bands ---------------
 // xBand  = FN_BAND_FROM_QGT(cx) << TC_BY08;
 // yBand  = FN_BAND_FROM_QGT(cz) << TC_BY08;
  //----Cloud counters by type ---------------
  cloud[0] = cloud[1] = cloud[2] = cloud[3] = 0;
  //-----Corners -----------------------------
  Corner[TC_SWCORNER] = 0;
  Corner[TC_SECORNER] = 0;
  Corner[TC_NECORNER] = 0;
  Corner[TC_NWCORNER] = 0;
  //----Trace Option -----------------------------
  tr  = tm->GetTrace();
  //----Init State Lock --------------------------
  pthread_mutex_init (&stMux,  NULL);
  //------Locate the QGT parameters   ------------
  double sLat = GetQgtSouthLatitude(cz);
  double nLat = GetQgtNorthLatitude(cz);
  double wLon = FN_ARCS_FROM_QGT(cx);
  dLat   = GetLatitudeDelta(cz);
  dLon   = (TC_ARCS_PER_DET);
  this->wLon  = wLon;
  this->sLat  = sLat;
  this->nLat  = nLat;
  //-----Compute center coordinates for scenery --------
	GetQgtMidPoint(cx,cz,mPoint);
	Scene				= mPoint;
	//---Register the QGT for scenery --------------------
	globals->scn->Register(qKey);
}

//----------------------------------------------------------------------------
//  Delete this Quarter Global Tile
//  CmQUAD destructor are automaticaly called
//----------------------------------------------------------------------------
C_QGT::~C_QGT()
{ //--------Deregister Scenery ---------------------------
	globals->scn->Deregister(qKey);
	globals->scn->CloseOSM(osmDB);
	//------------------------------------------------------
	if (0 == qSTAT) FreeAllVertices();
  if (trn)  delete trn;
  tcm->FreeSEA(this);
  tcm->FreeCST(this);
}
//----------------------------------------------------------------------------
//  Trace deletion for this QGT
//  
//----------------------------------------------------------------------------
void C_QGT::TraceDelete()
{ if (!tr) return;
	TRACE("TCM: -- Time: %04.2f QGT(%03d-%03d) Deleted. -------------",tcm->Time(),xKey,zKey);
  return;
}
//----------------------------------------------------------------------------
//  Return trace data
//----------------------------------------------------------------------------
void C_QGT::GetTrace(int *t,int *x,int *z)
{ *t = tr;
	*x = xKey;
	*z = zKey;
	return;
}
//----------------------------------------------------------------------------
//  Compute AXBY keys for this QGT
//  x0 = QGT x indice
//  z0 = QGT z indice
//          -----------
//          | QA | QX | Access keys to the 4 QGTs are computed in seaKEY table
//          -----------   This ensures that all coast files will be loaded
//          | QB | QY |   before QA is processed. The 4 files are needed
//          -----------   to correctly compute coast tiles that are 
//                        bordering QA
//----------------------------------------------------------------------------
void C_QGT::SetAXBYkey()
{ int x0  = xKey;
  int z0  = zKey;
  int x1  = (x0+1) & (511);                   // Right column modulo 512
  int z1  = (z0)?(z0 - 1):(0);                // row below
  seaKEY[0]  = (x0 << 16) | z0;               // A tile
  seaKEY[1]  = (x1 << 16) | z0;               // X Tile
  seaKEY[2]  = (x0 << 16) | z1;               // B Tile
  seaKEY[3]  = (x1 << 16) | z1;               // Y Tile
  return;
}
//-----------------------------------------------------------------------
//  Check if sea pod file is loaded
//  If loaded, increment user count
//  If not loaded, enter the POD index of the file to load
//  in the request table (seaREQ).
//  Request will be satisfied by the File Thread.
//-----------------------------------------------------------------------
int C_QGT::CheckSeaPOD(int k)
{ U_INT ind  = GetCoastIndex(seaKEY[k]);
  seaREQ[k]  = 0xFFFFFFFF;
  C_SEA *sea = tcm->GetSEA(ind);
  if (sea)  {sea->IncUser(); return 0;}       // Reserve the file
  //----Enter a request to load the corresponding file -----------
  seaREQ[k]  = ind;
  return 1;
}
//-----------------------------------------------------------------------
//  Check if sea sql file is loaded
//  If loaded, increment user count
//  If not loaded, enter the POD index of the file to load
//  in the request table (seaREQ).
//  Request will be satisfied by the File Thread.
//-----------------------------------------------------------------------
int C_QGT::CheckSeaSQL(int k)
{ U_INT key = seaKEY[k];
  seaREQ[k] = 0xFFFFFFFF;
  C_CDT *cst = tcm->GetCST(key);
  if (cst)  {cst->IncUser(); return 0;}
  //----Enter a request to load the file -----
  seaREQ[k] = key;
  return 1;
}
//-----------------------------------------------------------------------
//  Time Slice
//  Update Tile distance
//  Update 3D object distance
//  Cloud generator
//-----------------------------------------------------------------------
void C_QGT::TimeSlice(float dT)
{ //----------------------------------------------------------------
	demux++;
  char tm = (demux & 0x03);
  if (0 == tm)  {UpdateInnerCircle(); return;}    // Update textures
  if (1 == tm)  {w3D.TimeSlice(dT);   return;}    // Update 3D object
  if (2 == tm)    return;                         // do nothing (rfu)
  //----Update meteo ------------------------------------------------
  Metar = globals->cld->GenerateCloud(this,Metar);    // Update metar
  return;
}
//-----------------------------------------------------------------------
//  STEP INI:	Initialize the QGT
//  Note:   MidPoint position is in absolute world coordinate in arcsec
//         
//-----------------------------------------------------------------------
int C_QGT::StepINI()
{ //-----Southwest Corner vertex -----------------------
  CVertex *sw = tcm->GetQgtCorner(this,SWcorner,TC_SWCORNER);
  sw->IncUse();
  Corner[TC_SWCORNER] = sw;
  //----SouthEast corner vertex ------------------------
  CVertex *se = tcm->GetQgtCorner(this,SEcorner,TC_SECORNER);
  se->IncUse();
  Corner[TC_SECORNER] = se;
  //----NorthEast corner vertex ------------------------
  CVertex *ne = tcm->GetQgtCorner(this,NEcorner,TC_NECORNER);
  ne->IncUse();
  Corner[TC_NECORNER] = ne;
  //----NorthWest corner vertex ------------------------
  CVertex *nw = tcm->GetQgtCorner(this,NWcorner,TC_NWCORNER);
  nw->IncUse();
  Corner[TC_NWCORNER] = nw;
  //---Link all corners---------------------------------
  sw->EastLink(se);
  nw->EastLink(ne);
  sw->NorthLink(nw);
  se->NorthLink(ne);
  //---------Compute the QGT  dim in arcsec ------------
	MidRadius(Bound,zKey);
  //---------Init the super tiles ----------------------
  InitSuperTiles();
  SetStep(TC_QT_MSH);
  if (tr) TRACE("TCM: -- Time: %04.2f QGT %03d-%03d Init Tile mesh",tcm->Time(),GetXkey(),GetZkey());
  return 1;
}
//---------------------------------------------------------------------
//  STEP ELV:
//	Load elevations:
//	-if a TRN file exist, then load from TRN
//	-Otherwise load default elevations
//---------------------------------------------------------------------
int C_QGT::StepELV()
{	if (HasTRN())		return 1;
	return tcm->RequestELV(this);
}
//---------------------------------------------------------------------
//  STEP QTR:
//	Set elevations from QTR file
//---------------------------------------------------------------------
int C_QGT::StepQTR()
{	return qtr->SetQTRElevations(this);	}
//---------------------------------------------------------------------
//  STEP TRN: Set Default elevations from TRN File
//  NOTE: Must return 1
//---------------------------------------------------------------------
int C_QGT::StepTRN()
{ if (tr && trn) TRACE("TCM: -- Time: %04.2f QGT %03d-%03d TRN OK",tcm->Time(),xKey,zKey);
  trn->SetTRNdefault(this);
  SetStep(TC_QT_HDT);                                
  return 1;
}
//-------------------------------------------------------------------------
//  STEP HDT:		Set Detail elevation from TRN file
//-------------------------------------------------------------------------
int C_QGT::StepHDT()
{ U_INT nbv = globals->NbVTX;
  trn->GetHdtlElevations(this);
  U_INT nbc = globals->NbVTX - nbv;
  if(tr)  TRACE("TCM: -- Time: %04.2f QGT %03d-%03d Creates %05d vertices.  (TRN file)",
          tcm->Time(),xKey,zKey,nbc);
  //----Next is check for elevation patches --------------
	SetStep(TC_QT_PCH);								// Check for patche
  return 1;
}
//---------------------------------------------------------------------
//  Check for TRN elevation in SQL database
//---------------------------------------------------------------------
int C_QGT::HasTRN()
{ char  fn[64];														// TRN file name
  if (strn)				return 0;
	if (globals->elvDB)	globals->sqm->GetTRNElevations(this);
	if (HasElevation()) {SetStep(TC_QT_TIL);	return 1;}		
	//---Check for a TRN file ------------------------------------
  U_INT   cx = xKey;
  U_INT   cz = zKey;
  U_SHORT gx = cx >> 1;                     // Globe Tile X index
  U_SHORT gz = cz >> 1;                     // Globe Tile Z index
  U_SHORT qx = cx & 1;                      // Quadrant X index
  U_SHORT qz = cz & 1;                      // Quadrant Z index
  _snprintf (fn,64,"DATA/D%03d%03d/G%01d%01d.TRN", gx,gz,qx, qz);
  //---------Create a TRN object -------------------------------
  SStream s;                                // Stream file
  if (!pexists(&globals->pfs, fn)) return 0;						///
  if (!OpenRStream (fn,s))  Abort("Can read TRN");
  trn = new C_TRN(this,tr);
  if (tr) TRACE("TCM: -- Time: %04.2f QGT %03d-%03d Open TRN %s",
          tcm->Time(),xKey,zKey,fn);
  ReadFrom (trn, &s);
  CloseStream (&s);
	IndElevation();														// QGT has elevation
  SetStep(TC_QT_TRN);												// Next Step is TRN assigned
  return 1;
}
//---------------------------------------------------------------------
//  Check for Detailled elevation in SQL database
//---------------------------------------------------------------------
int C_QGT::StepTIL()
{ if (0 == globals->elvDB)	return StepSEA();
	globals->sqm->GetTILElevations(this);
  SetStep(TC_QT_PCH);			// Check elevation patche
	return 1;
}
//---------------------------------------------------------------------
//  Check for elevation in SQL database
//---------------------------------------------------------------------
int C_QGT::StepPCH()
{	if (0 == globals->elvDB)	return StepSEA();
  ELV_PATCHE  p;
	p.dir		= 1;
	globals->sqm->ReadPatches(this,p);
	SetStep(TC_QT_SEA);			// Check coast data
	return 1;
}
//----------------------------------------------------------------------------
//	Step SEA:
//    Request for COAST file c
//    The QGT Key for AXBY QGT are checked 
//----------------------------------------------------------------------------
int C_QGT::StepSEA()
{ if (tr) TRACE("TCM: -- QGT(%3d-%3d) End of elevations process",xKey,zKey);
	char  sql = globals->seaDB;
  U_INT req = 0;
  //---Build the AXBY key for this QGT ------------------------------
  SetAXBYkey();
  req += (sql)?(CheckSeaSQL(0)):(CheckSeaPOD(0));
  req += (sql)?(CheckSeaSQL(1)):(CheckSeaPOD(1));
  req += (sql)?(CheckSeaSQL(2)):(CheckSeaPOD(2));
  req += (sql)?(CheckSeaSQL(3)):(CheckSeaPOD(3));
  Step = TC_QT_SUP;
  if (0 == req) return 1;
 
  //---- Request to load coast files --------------------------------
  U_INT key = GetSEAindex(xKey,zKey);							// Compute coast index
  rCode			= TC_REQ_SEA;                         // Request code
  rKey			= key;                                // SEA file Key
  return tcm->StartIO(this,TC_QT_SUP);
}
//-----------------------------------------------------------------------
//  Return Tile X-Z indice of the position
//  pos is a geo position in absolute arcseconds
//  The function return in tx and tz, the indices of the detail tile in 
//  the QGT that contains the position
//-----------------------------------------------------------------------
bool C_QGT::GetTileIndices(SPosition &pos,short &tx, short &tz)
{ //---Compute longitude index ----------------
  double dta = pos.lon - wLon;                // Delta to west frontier
  tx = int(dta / dLon) & 31;
  //---Compute latitude index -----------------
  double dtz = pos.lat - sLat;
  tz = int(dtz / dLat) & 31;
  return (true);
}
//-----------------------------------------------------------------------
//  Return Tile X-Z indice of the ground spot
//-----------------------------------------------------------------------
bool C_QGT::GetTileIndices(GroundSpot &gns)
{ //---Compute longitude index ----------------
  double dta	= gns.lon - wLon;                //Delta to west frontier 
  gns.tx			= int(dta / dLon) & 31;
  //---Compute latitude index -----------------
  double dtz	= gns.lat - sLat;
  gns.tz			= int(dtz / dLat) & 31;
  return true;
}
//-----------------------------------------------------------------------
//  Adjust V (X,Y) coordinates relative to SW corner
//-----------------------------------------------------------------------
void C_QGT::RelativeToBase(CVector &v)
{	v.x -= wLon;													// Remove west longitude
	v.y -= sLat;													// Remove south latitude
}
//---------------------------------------------------------------------
//  Delete the QTR file
//---------------------------------------------------------------------
int C_QGT::DeleteQTR()
{ //------Check for QTR deletion--------------------------
  bool qtruse = true;
  if (qtr)  qtruse = tcm->DecQTR(qtr);
  if (qtruse) return 1;
  //--------Delete the QTR file --------------------------
  delete (qtr);
  qtr = 0;
  return 1;
}
//-----------------------------------------------------------------------
//  Free resources
//  -Delete all Vertices from the QGT
//-----------------------------------------------------------------------
void C_QGT::FreeAllVertices()
{ CmQUAD *qd  = qTAB;
  U_INT nt    = 0;
  int  nb1    = globals->NbVTX;
  while (nt++ != TC_DETAIL_NBR)     FreeQuad(qd++);
  int  nb2    = nb1 - globals->NbVTX;
  if (tr) TRACE("TCM: -- Time: %04.2f QGT %03d-%03d releases %06d vertices",tcm->Time(),xKey,zKey,nb2);
  return;
}
//-----------------------------------------------------------------------
//  Free the Quad  Vertices
//  -Free the mid point if it is a full Quad
//  -Then free the corner
//-----------------------------------------------------------------------
int C_QGT::FreeVertices(CmQUAD *qd)
{ CVertex *ct = qd->GetCenter();
  CVertex *sw = ct->Edge[TC_SWCORNER];
  CVertex *se = ct->Edge[TC_SECORNER];
  CVertex *ne = ct->Edge[TC_NECORNER];
  CVertex *nw = ct->Edge[TC_NWCORNER];
  CVertex *m1 = 0;
  CVertex *m2 = 0;
  CVertex *m3 = 0;
  CVertex *m4 = 0;

  U_INT tx = (ct->xKey >> TC_BY1024) & TC_032MODULO;
  U_INT tz = (ct->zKey >> TC_BY1024) & TC_032MODULO;

  if (ct->IsFull())
  { //-----Free M1 mid point (East Axis)----------------
    m1 = GetEastVertex(sw,ct->xKey,sw->zKey,0);
    if (0 == m1) Abort("Missing M1 Vertex");
    if (m1->NoMoreUsed())   FreeHMidVertex(m1);
    //-----Free M2 mid point -(North Axis)--------------
    m2 = GetNorthVertex(sw,sw->xKey,ct->zKey,0);
    if (0 == m2) Abort("Missing M2 Vertex");
    if (m2->NoMoreUsed())   FreeVMidVertex(m2);
    //-----Free M3 mid point -(North Axis)--------------
    m3 = GetNorthVertex(se,se->xKey,ct->zKey,0);
    if (0 == m3) Abort("Missing M3 Vertex");
    if (m3->NoMoreUsed())   FreeVMidVertex(m3);
    //-----Free M4 mid point (East Axis)----------------
    m4 = GetEastVertex(nw,ct->xKey,nw->zKey,0);
    if (0 == m4) Abort("Missing M4 Vertex");
    if (m4->NoMoreUsed())   FreeHMidVertex(m4);
  }
  //-------Free Corner vertices -----------------------
  if (sw->NoMoreUsed()) UnlinkVertex(sw);
  if (se->NoMoreUsed()) UnlinkVertex(se);
  if (ne->NoMoreUsed()) UnlinkVertex(ne);
  if (nw->NoMoreUsed()) UnlinkVertex(nw);
  return 0;
}
//-----------------------------------------------------------------------
//  Unlink Vertex VT
//-----------------------------------------------------------------------
void C_QGT::UnlinkVertex(CVertex *vt)
{ if (0 == vt)    return;
  CVertex *vn = vt->Edge[TC_NORTH];           // North Vertex
  CVertex *vs = vt->Edge[TC_SOUTH];           // South Vertex
  CVertex *ve = vt->Edge[TC_EAST];            // East Vertex
  CVertex *vw = vt->Edge[TC_WEST];            // West Vertex
  if (vw) vw->Edge[TC_EAST]  = ve;            // West Bypass vt
  if (ve) ve->Edge[TC_WEST]  = vw;            // East Bypass vt
  if (vn) vn->Edge[TC_SOUTH] = vs;            // North bypass vt
  if (vs) vs->Edge[TC_NORTH] = vn;            // South Bypass vt
  delete vt;
  return;
}
//-----------------------------------------------------------------------
//  Free a mid point Vertex along East axis
//  Unlink from direction and delete
//-----------------------------------------------------------------------
void C_QGT::FreeHMidVertex(CVertex *vt)
{ if ((0 == vt) || vt->IsUsed())   return;
  CVertex *next       = vt->Edge[TC_EAST];
  CVertex *prev       = vt->Edge[TC_WEST];
  prev->Edge[TC_EAST] = next;
  next->Edge[TC_WEST] = prev;
  //----Remove any Vertical links -------------------------
  CVertex *vn = vt->Edge[TC_NORTH];
  CVertex *vs = vt->Edge[TC_SOUTH];
  if (vn)  vn->Edge[TC_SOUTH]  = 0;
  if (vs)  vs->Edge[TC_NORTH]  = 0;
  delete vt;
  return;
}
//-----------------------------------------------------------------------
//  Free a mid point Vertex along North axis
//  Unlink from direction and delete
//-----------------------------------------------------------------------
void C_QGT::FreeVMidVertex(CVertex *vt)
{ if ((0 == vt) || vt->IsUsed())      return;
  CVertex *next         = vt->Edge[TC_NORTH];
  CVertex *prev         = vt->Edge[TC_SOUTH];
  prev->Edge[TC_NORTH]  = next;
  next->Edge[TC_SOUTH]  = prev;
  //-----Remove any horizontal links ---------------------
  CVertex *ve = vt->Edge[TC_EAST];
  CVertex *vw = vt->Edge[TC_WEST];
  if (ve)  ve->Edge[TC_WEST] = 0;
  if (vw)  vw->Edge[TC_EAST] = 0;
  delete vt;
  return;
}
//-----------------------------------------------------------------------
//  Free the slot in the CmQUAD Array
//-----------------------------------------------------------------------
int C_QGT::FreeQuad(CmQUAD *qd)
{ int end = qd->GetSize();
  if (qd->IsaQuad()) return FreeVertices(qd);
  //----Free Array of CmQUAD ---------------
  CmQUAD *ar = qd->GetArray();
  for (int k = 0; k!= end; k++) FreeQuad(ar++);
  return 0;
}
//---------------------------------------------------------------------
//  Locate East Vertex
//  Locate Vertex (vx,vz) along the East Side of Vk
//  When op = 1 a new vertex is created and inserted in the mesh
//	NOTE:  When the vertex key is 0, then we cross the origin meridien
//---------------------------------------------------------------------
CVertex *C_QGT::GetEastVertex(CVertex *vk,U_INT xv,U_INT zv,int op)
{ CVertex *pv   = vk;									// Previous
  CVertex *vn   = vk->Edge[TC_EAST];	// Next
  U_INT   nkey  = 0;									// Key from next vertex
  //----Scan the East direction ---------------------
  while (vn)
  { if (vn->xKey == xv)   return vn;
    nkey = (vn->xKey)?(vn->xKey):(0xFFFFFFFF);
    if (nkey <  xv)            
    { pv    = vn;
      vn    = pv->Edge[TC_EAST];
      continue;
    }
    //----Create a new vertex if requested -----------
    if (0 == op)  return 0;
    vn = CreateVertex(xv,zv);
    pv->InsertEast(vn);
    return vn;
  }
  //----Stop for mesh incoherency --------------------
  if (0 == op)  return 0;
  tcm->MeshError(xv,zv);
  return 0;
}
//---------------------------------------------------------------------
//  Locate North Vertex
//  Locate Vertex (vx,vz) along the North Side of Vk
//  When op = 1 a new vertex is created and inserted in the mesh
//---------------------------------------------------------------------
CVertex *C_QGT::GetNorthVertex(CVertex *vk,U_INT xv,U_INT zv,int op)
{ CVertex *pv = vk;
  CVertex *vn = vk->Edge[TC_NORTH];
  U_INT   nkey  = 0;									// Key from next vertex
  //----scan the NORTH direction ---------------------
  while (vn)
  { if (vn->zKey == zv) return vn;
    if (vn->zKey  < zv)
    { pv    = vn;
      vn    = pv->Edge[TC_NORTH];
      continue;
    }
    //----Create a new vertex if requested -----------
    if (0 == op)  return 0;
    vn = CreateVertex(xv,zv);
    pv->InsertNorth(vn);
    return vn;
  }
  //----Stop for mesh incoherency --------------------
  if (0 == op)  return 0;
  tcm->MeshError(xv,zv);
  return 0;
}
//-------------------------------------------------------------------------
//	Get SW corner position: The Base of QGT
//-------------------------------------------------------------------------
SPosition C_QGT::GetBase()
{	SPosition p;
	p.lon	= wLon;
	p.lat	= sLat;
	p.alt	= 0;
	return p;
}
//-------------------------------------------------------------------------
//  Assign final Detail Tile center
//  
//-------------------------------------------------------------------------
int C_QGT::CenterTile(CVertex *sw,CVertex *nw,CVertex *ne,CVertex *se)
{ //--- Center Tile indice is derived from SW corner -------
  U_INT cx    = sw->xKey + 512;
  U_INT cz    = sw->zKey + 512;
  //--- Locate Quad slot from sw indices -------------------
  U_INT kx    = FN_DET_FROM_INDX(sw->xKey);
  U_INT kz    = FN_DET_FROM_INDX(sw->zKey);
  U_INT No    = FN_DET_FROM_XZ(kx,kz);
  CmQUAD  *qd = qTAB + No;
  CVertex *ct = qd->GetCenter();
  ct->Init(cx,cz);
  //--- Assign corners to center ---------------------------
  ct->SetCorner(TC_SWCORNER,sw);
  ct->SetCorner(TC_NWCORNER,nw);
  ct->SetCorner(TC_NECORNER,ne);
  ct->SetCorner(TC_SECORNER,se);
  return 1;
}
//-------------------------------------------------------------------------
//  Divide the square into 4 QUADS up to target level
//   -div is the tile increment (16,8,4,2,1,0) to compute
//    the quad center
//-------------------------------------------------------------------------
int C_QGT::DivideTile(U_INT div,CVertex *sw,CVertex *nw,CVertex *ne,CVertex *se)
{ //--- When reaching level, allocate center Quad ----
  if (0 == div)  return CenterTile(sw,nw,ne,se);
  //--- Compute center indices -----------------------
  U_INT cx  = sw->xKey + (div << TC_BY1024);
  U_INT cz  = sw->zKey + (div << TC_BY1024);
  //--- Create center vertex -------------------------
  CVertex *ct = CreateVertex(cx,cz);
  //--- Create mid vertex m1 (between SW and SE) -----
  U_INT    x1 = cx;
  U_INT    z1 = sw->zKey;
  CVertex *m1 = GetEastVertex(sw,x1,z1,1);
  //--- Create mid vertex m2 (between SW and NW) -----
  U_INT    x2 = sw->xKey;
  U_INT    z2 = cz;
  CVertex *m2 = GetNorthVertex(sw,x2,z2,1);
  //--- Create mid vertex m3 (between SE and NE) -----
  U_INT    x3 = se->xKey;
  U_INT    z3 = cz;
  CVertex *m3 = GetNorthVertex(se,x3,z3,1);
  //--- Create mid vertex m4 (between NW and NE) -----
  U_INT    x4 = cx;
  U_INT    z4 = nw->zKey;
  CVertex *m4 = GetEastVertex(nw,x4,z4,1);
  //--- Link mid vertices to center ------------------
  m1->SetEdge(TC_NORTH,ct);
  m4->SetEdge(TC_SOUTH,ct);
  m2->SetEdge(TC_EAST, ct);
  m3->SetEdge(TC_WEST, ct);
  //--- Link center to mid vertices ------------------
  ct->SetEdge(TC_SOUTH,m1);
  ct->SetEdge(TC_NORTH,m4);
  ct->SetEdge(TC_WEST, m2);
  ct->SetEdge(TC_EAST, m3);
  //--- Divide each quad recursively -----------------
  U_CHAR nd = div >> 1;               // Half increment
  DivideTile(nd,sw,m2,ct,m1);             // SW Quad
  DivideTile(nd,m2,nw,m4,ct);             // NW Quad
  DivideTile(nd,ct,m4,ne,m3);             // NE Quad
  DivideTile(nd,m1,ct,m3,se);             // SE Quad
  return 0;
}
//-------------------------------------------------------------------------
//	STEP MSH:
//  Divide the QGT into an array of 32*32 detail Tiles
//-------------------------------------------------------------------------
int C_QGT::StepMSH()
{ CVertex *sw = Corner[TC_SWCORNER];
  CVertex *nw = Corner[TC_NWCORNER];
  CVertex *ne = Corner[TC_NECORNER];
  CVertex *se = Corner[TC_SECORNER];
  DivideTile(16,sw,nw,ne,se);
  //----Liberate the four corner vertices ------------
  Corner[TC_SWCORNER]->DecUse();
  Corner[TC_SECORNER]->DecUse();
  Corner[TC_NECORNER]->DecUse();
  Corner[TC_NWCORNER]->DecUse();
  //----Next step is getting elevations --------------
  SetStep(TC_QT_ELV);
  int nbv = globals->NbVTX;
  if (tr) TRACE("TCM: -- Time: %04.2f QGT %03d-%03d Build initial Tile mesh (%04d vertices)",
    tcm->Time(),xKey,zKey,nbv);
  return 1;
}
//--------------------------------------------------------------------------
//  Init the Super Tiles
//--------------------------------------------------------------------------
void C_QGT::InitSuperTiles()
{ U_INT  No    = 0;
  CSuperTile *sp = Super;

  for (No = 0; No != TC_SUPERT_NBR; No++)
  { sp->NoSP        = No;
    sp->mPos.lon    = (SuperCT[No].cx * dLon) + wLon;
    sp->mPos.lat    = (SuperCT[No].cz * dLat) + sLat;
    sp->mPos.alt    = 0;
    //---Compute SuperTile radius in arcsec ----------
    sp->sRad.x			= TC_ARCS_PER_HSP;
    sp->sRad.y			= 2 * dLat;
    sp->sRad.z			= 0;
    //-----------------------------------------------
    sp++;
  }

  return;
}
//-----------------------------------------------------------------------
//  Locate texture descriptor for  tile (x,z) in qgt
//-----------------------------------------------------------------------
CTextureDef *C_QGT::GetTexDescriptor(U_INT tx,U_INT tz)
{ //----Access the Super Tile ----------------
  CSuperTile   *sp  = GetSuperTile(tx,tz);
  //----Local  tile number from indices -----
  U_INT nt = ((tz & TC_004MODULO) << TC_BY04) | (tx & TC_004MODULO);
  return &sp->Tex[nt];
}
//-------------------------------------------------------------------------
//  Given TX and TZ (indices of Detail tile) return SuperTile context
//-------------------------------------------------------------------------
CSuperTile *C_QGT::GetSuperTile(int tx,int tz)
{ U_INT No = ((tz >> TC_BY04) << TC_BY08) | (tx >> TC_BY04);
  return &Super[No];
}
//-------------------------------------------------------------------------
//  Given Key (indices of Detatil tile) return SuperTile context
//-------------------------------------------------------------------------
CSuperTile *C_QGT::GetSuperTile(U_INT No)
{ return (No > 63)?(0):(&Super[No]); }
//-------------------------------------------------------------------------
//  Return OSM part with same texture reference 
//-------------------------------------------------------------------------
C3DPart	*C_QGT::GetOSMPart(char No,char dir, char *ntx)
{ CSuperTile *sup = (No > 63)?(0):(&Super[No]);
	if (0 == sup)		return 0;
	//--- Get texture reference --------------------------
	TEXT_INFO txd;
	txd.Dir = dir;
	strncpy(txd.name,ntx,FNAM_MAX);
	CShared3DTex *ref = globals->txw->GetM3DPodTexture(txd);
	C3DPart *prt = sup->SearchOSMPart(ref);
	globals->txw->Free3DTexture(ref);
	return prt;
}
//-------------------------------------------------------------------------
//  STEP 3:
//  Set Detail Tile elevation from QTR file
//  Tx and Tz are the tile relative coordinates in the QGT
//  Elevation is For the Center Vertex
//-------------------------------------------------------------------------
//  Then for each corner vertex, an average height is computed from
//  neighbouring centers.
//  The variable Ground in each vertex cummulate the elevation from
//  surrounding centers while the use count indicates haw many
//  centers are pointing to this corner.  Thus it is easy to compute
//  each time an average elevation.  The last computation will yield
//  the correct result.
//-------------------------------------------------------------------------
void C_QGT::DetailElevation(U_INT tx,U_INT tz,float elev)
{ if ((tx >= 32) || (tz >= 32)) Abort("Tile index > 31");
  U_INT No = FN_DET_FROM_XZ(tx,tz);
  CVertex    *ct = qTAB[No].GetCenter();
  CSuperTile *sp = GetSuperTile(tx,tz);
  //-------Update Super Tile mini max  ------------------
  SetMiniMax(elev,ct,sp);
  //-------Update SW corner vertices ----------------------
  CVertex *sw = ct->Edge[TC_SWCORNER];
  sw->nElev++;
  sw->Ground += elev;
  sw->SetCornerHeight();
  //-------Update SE corner vertices ---------------------- 
  CVertex *se = ct->Edge[TC_SECORNER];
  se->nElev++;
  se->Ground += elev;
  se->SetCornerHeight();
  //-------Update NE corner vertices ---------------------- 
  CVertex *ne = ct->Edge[TC_NECORNER];
  ne->nElev++;
  ne->Ground += elev;
  ne->SetCornerHeight();
  //-------Update NW corner vertices ---------------------- 
  CVertex *nw = ct->Edge[TC_NWCORNER];
  nw->nElev++;
  nw->Ground += elev;
  nw->SetCornerHeight();
  return;
}
//-------------------------------------------------------------------------
//  Set Detail Tile elevations from TRN file
//    bx and bz are the relative indices of the base detail tile (SW)
//    in the QGT.
//    el is an array of 5 * 5 elevations corresponding to the tile corners
//		inside the Super Tile (from SW corner to NE corner)
//		20  ...                     24
//		... ... 
//	  8   9  10 ...
//		0		1		2		3		4		5		6		7
//-------------------------------------------------------------------------
void C_QGT::SetElvFromTRN(U_INT bx,U_INT bz,float *el)
{ CSuperTile *sp = GetSuperTile(bx,bz);
  int       dim = TC_DETAIL_PER_SPT + 1;
  int       Nr  = 0;
  int       bd  = 0;
  for   (int z=0; z<TC_DETAIL_PER_SPT; z++)
  { Nr  = (z * dim);													// Row index
    for (int x=0; x<TC_DETAIL_PER_SPT; x++)
    { int tx      = bx | x;										// DET(X)
      int tz      = bz | z;										// DET(Z)
      int No      = FN_DET_FROM_XZ(tx,tz);		// DET(No)
      CmQUAD *qd  = qTAB + No;								// QUAD
      if (qd->IsArray())                continue;
      int Ne      = (Nr + x);									// Elevation index
      int tp      = BorderSUP[bd++];					// Border type
      float *hgt    = el + Ne;								// Elevation
      //---Set the southwest vertex -----------------
      CVertex *sw = qd->GetCorner(TC_SWCORNER);
      SetMiniMax(*hgt,sw,sp);
      //---Check if the east vertex should be set ---
      if (1 &  tp) 
      { CVertex *se = qd->GetCorner(TC_SECORNER);
        SetMiniMax(*(hgt+1),se,sp);
      }
      //---Check if the north vertex should be set --
      if (2 &  tp)
      { CVertex *ne = qd->GetCorner(TC_NECORNER);
        SetMiniMax(*(hgt+dim),ne,sp);
      }
      //---Check if the last vertex should be set ---
      if (3 == tp)
      { CVertex *nw = qd->GetCorner(TC_NWCORNER);
        SetMiniMax(*(hgt+dim+1),nw,sp);
      }

    } //--- End of row --------------
  }   //--- End of matrix -----------
  //--------Now, set the center elevation ------------
  int fz = bz + TC_DETAIL_PER_SPT;
  int fx = bx + TC_DETAIL_PER_SPT;
  for   (int z=bz; z<fz; z++)
  { for (int x=bx; x<fx; x++)
    {int No      = FN_DET_FROM_XZ(x,z);
     CmQUAD *qd  = qTAB + No;
     if (qd->IsArray())     continue;
		 //--- Compute cnter height ------------
 		 double   mp = 0;
     CVertex *ct = qd->GetCenter();
     CVertex *sw = ct->Edge[TC_SWCORNER];
     mp += sw->GetWZ();
     CVertex *se = ct->Edge[TC_SECORNER];
     mp += se->GetWZ();
     CVertex *ne = ct->Edge[TC_NECORNER];
     mp += ne->GetWZ();
     CVertex *nw = ct->Edge[TC_NWCORNER];
     mp += nw->GetWZ();
     ct->SetWZ(mp * double (0.25));
    }
  }
  return;
}
//-------------------------------------------------------------------------
//  Update Super Tile Mini Max
//  The maximum and minimun height are kept for culling purpose
//  Then the highest point is retain as the center for far terrain rendering
//-------------------------------------------------------------------------
void C_QGT::SetMiniMax(float el,CVertex *vt,CSuperTile *sp)
{	//------Compute Super Tile indices ------------------------
  vt->SetWZ(el);
  if (el < sp->MiniH) sp->MiniH = el;
  //--Retain Highest point as the center of Super Tile for far terrain --
  if (el > sp->MaxiH) sp->MaxiH = el;
  return;
}
//-------------------------------------------------------------------------
//  Get Elevation from context for a given vertex
//-------------------------------------------------------------------------
float C_QGT::GetHDTLelevation(U_SHORT dx,U_SHORT dz,CTX_QUAD &ctx)
{ U_INT cx  = ctx.Inc[dx] + ctx.ebx;
  U_INT cz  = ctx.Inc[dz] + ctx.ebz;
  U_INT No  = (cz * eDim) + cx;
  float ht  = eLev[No];
  return ht;
}
//-------------------------------------------------------------------------
//  Terminal Quad enter in Quad Matrix
//  The resolution parameter is the local resolution
//  Copy the center vertex into the CmQUAD
//  NOTE:  The CmQUAD ARRAY is ordered by X then by Z:
//  The first row is (X0,X1,..,Xn) then next northward row.
//-------------------------------------------------------------------------
int C_QGT::TerminalQuad(CVertex *ct,CTX_QUAD &ctx)
{ if (0 == ctx.qARR)  return 0;
  //--- Compute Quad position in the array of CmQUADs -----
  U_INT   No  = (ctx.qz * ctx.qDim) + ctx.qx;
  ctx.qARR[No].SetParameters(ct,ctx.Target);
  delete ct;
  return 1;
}
//-------------------------------------------------------------------------
//  Divide a Quad for <hdtl> 
//
//    nw ----m4---- ne
//    |              |
//    |              |
//    m2     ct     m3
//    |              |
//    |              |
//    sw-----m1-----se
//  NOTE 1:  <hdtl> elevations are stored X  row first
//    0 sw
//    1 m2
//    ....
//  THIS IS A RECURSIVE FUNCTION WHICH USES THE FOLOWING VARIABLES
//  -The final CmQUAD Array
//  -The elevation matrix 
//  -The level coordinates
//-------------------------------------------------------------------------
int C_QGT::DivideQuad(CVertex *ct,CTX_QUAD &ctx)
{ U_CHAR   end = tcm->GetLastEVindex();       // Last vertex indice in ST
  U_SHORT  dv = ctx.vSub;                     // Vertex subdivision (X and Z)
  U_INT    vx = 0;                            // Vertex absolute index X
  U_INT    vz = 0;                            // Vertex absolute index Z;
  CVertex *sw = ct->Edge[TC_SWCORNER];        // SW corner Vertex
  CVertex *nw = ct->Edge[TC_NWCORNER];        // NW corner Vertex
  CVertex *se = ct->Edge[TC_SECORNER];        // SE corner Vertex
  CVertex *ne = ct->Edge[TC_NECORNER];        // NE corner Vertex
  //-------Locate or Create m1 vertex (mid SW-SE) --------------------
  vx  = sw->xKey + dv;                        // Vertex m1 X coordinate 
  vz  = sw->zKey;                             // Vertex m1 Z coordinate
  CVertex *m1 = GetEastVertex(sw,vx,vz,1);    // Locate or create
  m1->IncUse();                               // Quad references
  //-------Locate or Create m2 vertex (mid SW-NW) ---------------------
  vx  = sw->xKey;                             // Vertex m2 X coordinate
  vz  = sw->zKey + dv;                        // Vertex m2 Z coordinate
  CVertex *m2 = GetNorthVertex(sw,vx,vz,1);   // Locate or create
  m2->IncUse();                               // Quad references
  //-------Locate or Create m3 vertex --------------------------------
  vx  = se->xKey;                             // Vertex m3 X coordinate
  vz  = se->zKey + dv;                        // Vertex m3 Z coordinate
  CVertex *m3 = GetNorthVertex(se,vx,vz,1);   // Locate or create
  m3->IncUse();                               // Quad references
  //-------Locate or Create m4 vertex --------------------------------
  vx  = nw->xKey + dv;                        // Vertex m4 X coordinate
  vz  = nw->zKey;                             // Vertex m4 Z coordinate
  CVertex *m4 = GetEastVertex(nw,vx,vz,1);    // Locate or create
  m4->IncUse();                               // Quad references
  //------Set Center indicator to Full Quad --------------------------
  ct->SetFullQuad();
  //
  //  NOTE: Elevations are set at the final level
  //  
  if (ctx.Target == ctx.Actual)
  { U_INT tx  = (ct->xKey >> TC_BY1024) & TC_032MODULO;
    U_INT tz  = (ct->zKey >> TC_BY1024) & TC_032MODULO;
		CSuperTile *sp = ctx.sup;
    //-----------------------------------------
    SetMiniMax(GetHDTLelevation(0,0,ctx),sw,sp);
    SetMiniMax(GetHDTLelevation(1,0,ctx),m1,sp);
    SetMiniMax(GetHDTLelevation(2,0,ctx),se,sp);
    SetMiniMax(GetHDTLelevation(0,1,ctx),m2,sp);
    SetMiniMax(GetHDTLelevation(1,1,ctx),ct,sp);
    SetMiniMax(GetHDTLelevation(2,1,ctx),m3,sp);
    SetMiniMax(GetHDTLelevation(0,2,ctx),nw,sp);
    SetMiniMax(GetHDTLelevation(1,2,ctx),m4,sp);
    SetMiniMax(GetHDTLelevation(2,2,ctx),ne,sp);
    return TerminalQuad(ct,ctx);
  }
  //------Transform center in a mesh Vertex --------------------------
  ct->ClearUse();
  ct->Edge[TC_SWCORNER]->DecUse();
  ct->Edge[TC_SECORNER]->DecUse();
  ct->Edge[TC_NECORNER]->DecUse();
  ct->Edge[TC_NWCORNER]->DecUse();

  m1->DecUse();
  m2->DecUse();
  m3->DecUse();
  m4->DecUse();
  //------Link center to mid poinst-----------------------------------
  ct->Edge[TC_NORTH]  = m4;
  ct->Edge[TC_SOUTH]  = m1;
  ct->Edge[TC_WEST]   = m2;
  ct->Edge[TC_EAST]   = m3;
  //------Link Mid points to center ----------------------------------
  m1->Edge[TC_NORTH]  = ct;
  m2->Edge[TC_EAST]   = ct;
  m3->Edge[TC_WEST]   = ct;
  m4->Edge[TC_SOUTH]  = ct;
  //----- Prepare context for next level -----------------------------
  CTX_QUAD ctn = ctx;
  ctn.Actual += 1;                          // Next level
  ctn.vSub    = ctx.vSub >> 1;              // Vertex subdivision is half
  ctn.Inc[1]  = ctx.Inc[1] >> 1;            // Elevation mid  increment
  ctn.Inc[2]  = ctx.Inc[2] >> 1;            // Elevation Full increment 
  U_INT qx    = ctx.qx << 1;                // Quad X index to next Quadrant
  U_INT qz    = ctx.qz << 1;                // Quad Z index to next Quadrant
  //------Make new SW Quad from actual Vertices ----------------------
  ctn.qx      = qx;
  ctn.qz      = qz;
  CVertex *c1 = CreateCenter(sw,m1,ct,m2,ct);
  DivideQuad(c1,ctn);
  //------Make new SE Quad from actual vertices ----------------------
  ctn.ebx     = ctx.ebx | ctx.Inc[1];       // Base 1-0
  ctn.ebz     = ctx.ebz;
  ctn.qx      =   qx | 1;                   // SE index
  ctn.qz      =   qz;
  CVertex *c2 = CreateCenter(m1,se,m3,ct,m3);
  DivideQuad(c2,ctn);
  //-----Make new NW Quad from actual vertices -----------------------
  ctn.ebx     = ctx.ebx;                    // Base 0-1
  ctn.ebz     = ctx.ebz | ctx.Inc[1];
  ctn.qx      =   qx;
  ctn.qz      =   qz | 1;
  CVertex *c3 = CreateCenter(m2,ct,m4,nw,m4);
  DivideQuad(c3,ctn);
  //-----Make new NE Quad from actual vertices -----------------------
  ctn.ebx     = ctx.ebx | ctx.Inc[1];       // Base 1-1
  ctn.ebz     = ctx.ebz | ctx.Inc[1];
  ctn.qx      =   qx | 1;
  ctn.qz      =   qz | 1;
  CVertex *c4 = CreateCenter(ct,m3,ne,m4,ne);
  DivideQuad(c4,ctn);
  return 0;
}
//-------------------------------------------------------------------------
//  Create Center Vertex from the four corners vertices
//   Compute indices as the middle of both indices
//   op is the opposite (NE) vertex from which S and T may be extracted
//-------------------------------------------------------------------------
CVertex *C_QGT::CreateCenter(CVertex *sw,CVertex *se,CVertex *ne,CVertex *nw,CVertex *op)
{ U_INT xe  = (ne->xKey)?(ne->xKey):(TC_MAX_COORDINATE);  // Eastward  X index
  U_INT zn  = (ne->zKey)?(ne->zKey):(TC_MAX_COORDINATE);  // Northward Z index
  U_INT vx  = (xe + nw->xKey) >> 1;                       // X coordinate
  U_INT vz  = (zn + sw->zKey) >> 1;                       // Z coordinate
  CVertex *ct = CreateVertex(vx,vz);
  //----Set corner vertices ----------------------------
  ct->Edge[TC_SWCORNER] = sw;
  sw->IncUse();
  ct->Edge[TC_SECORNER] = se;
  se->IncUse();
  ct->Edge[TC_NECORNER] = ne;
  ne->IncUse();
  ct->Edge[TC_NWCORNER] = nw;
  nw->IncUse();
  return ct;
}
//-------------------------------------------------------------------------
//  Divide Tile according to the <hdtl> parameters
//  NOTE:
//    1)  Only tile of level 0 are divided.  Tile already divided are
//        not divided again. i.e nested division is not yet supported
//    1)  When Resolution is 2, the detail Tile is not subdivided.  Only 
//        edge mid-points are added.
//  NOTE: 
//        This is a recursive process to divide the Tile with <hdtl> statement. 
//-------------------------------------------------------------------------
void C_QGT::DivideHDTL(TRN_HDTL *hd)
{ U_INT tx    = hd->GetTileX();             // Tile X index
  U_INT tz    = hd->GetTileZ();             // Tile Z index
  U_INT No    = FN_DET_FROM_XZ(tx,tz);      ///(tz << TC_BY32) | tx;
  CmQUAD  *qd = qTAB + No;                  // Quad to divide
  if (qd->IsArray())          return;       // Already divided
  //---Init Tile division into sublevel QUADS------------------------------
  eDim          = hd->GetDim();             // Dimension
  eLev          = hd->GetElvArray();        // Array of elevations
  U_SHORT level = hd->GetLevel();           // Resolution level
  U_SHORT half  = level << 1;								// Half resolution
  CTX_QUAD ctx;                             // Context for recursion
  CmQUAD *ar  = 0;                          // Array of CmPointers
	U_INT	  sup	= hd->GetSupNo();							// Super Tile number
	ctx.sup			= GetSuperTile(sup);						// Super tile pointer
  ctx.Actual  = 0;													// Target level
  ctx.Target  = level;                      // Resolution = <dimn> / 4
  ctx.vSub    = 512;                        // Initial vertex subdivision
  ctx.ebx     = 0;                          // Elevation X base
  ctx.ebz     = 0;                          // Elevation Z base
  ctx.Inc[0]  = 0;                          // Elevation base increment = 0
  ctx.Inc[1]  = half;                       // Elevation Half increment
  ctx.Inc[2]  = hd->GetRes();               // Elevation Full Increment
  ctx.qDim    = (0 != level)?(half):(0);    // Quad array dimension
  //---Allocate a new CmQUAD array only for resolution # 0 ----------
  if (level) ar = new CmQUAD[ctx.qDim * ctx.qDim];
  ctx.qARR    = ar;
  ctx.qx      = 0;                          // Initial X Quadrant
  ctx.qz      = 0;                          // Initial Z Quadrant
  CVertex *ct = qd->GetCenter();
  ctx.sw      = ct->GetCorner(TC_SWCORNER);
  ctx.tx      = hd->GetTileX();
  ctx.tz      = hd->GetTileZ();
  //-----------------------------------------------------------
  //  If the Quad is to be divided allocate a new center vertex
  //-----------------------------------------------------------
  if (level) ct = DuplicateCenter(ct,hd);
  DivideQuad(ct,ctx);
  //--Replace the Tile Quad with a pointer to the new Array --------------
  if (ar) qd->SetArray(ar,ctx.qDim);
  return;
}
//-------------------------------------------------------------------------
//  Duplicate the center Vertex
//-------------------------------------------------------------------------
CVertex *C_QGT::DuplicateCenter(CVertex *ct,TRN_HDTL *hd)
{ //ct->AreWe(0x00047600,0x0A4F200);
  //---Set elevation from middle region array --
  U_SHORT ed    = hd->GetDim();
  U_SHORT md    = (ed * ed) >> 1;
	double  el    = hd->Elevation(md);
	ct->SetWZ(el);
	//--- Copy center vertex in a new one ----------(-
	CVertex *nc = CreateVertex(ct->xKey,ct->zKey);
  nc->Use     = ct->Use;
  nc->CopyEdge(ct);       // Dupplicate corner
	//--- Dupplicate world coordinates ---------------
	nc->CopyCOORD(*ct);
  return nc;
}

//-------------------------------------------------------------------------
//  Last elevation Pass: 
//  Compute Super Tile Center and final Radius
//  Then schedule the QGT for texture loading
//  ac = 1 => this is the QGT where aircraft resides
//  NOTE: Must return 1
//-------------------------------------------------------------------------
int C_QGT::StepSUP()
{ bool				ac = tcm->QGTplane(this);
	CSuperTile *sp = Super;
  double minH  = sp->MiniH;
  double maxH  = sp->MaxiH;
  for   (U_INT No = 0; No != TC_SUPERT_NBR; No++)
        { sp->sRad.z  = (sp->MaxiH - sp->MiniH) * 0.5;
          //----Update QGT mini max --------------
          if (sp->MaxiH > maxH) maxH = sp->MaxiH;
          if (sp->MiniH < minH) minH = sp->MiniH;
          //--- Update mid point altitude --------
          sp->mPos.alt  = (sp->MaxiH + sp->MiniH) * 0.5;
          //--------------------------------------
          tcm->FormatName(this,sp);
          sp->BuildBorder(this,No);
          //----Put Super Tile in Far Queue ------
					sp->RenderOUT();
					sp->WantINQ();
          FarsQ.PutLast(sp);
          sp++;
        }
  //----Compute QGT mid point and Z radius ------------------
  mPoint.alt  = (minH + maxH) * 0.5;
  Bound.z			= (maxH - minH) * 0.5;
  SetStep(TC_QT_3DO);
  //----Set QGTfor 3D model ---------------------------------
  w3D.SetQGTparameters(this);
  //----Assign Metar if relevant ----------------------------
  globals->wtm->AttachMetar(this,ac);
  qSTAT = 0;
	tcm->IncRDY(-1);							// One QGT ready
  return 1;
}
//-------------------------------------------------------------------------
//	Load 3D objects
//-------------------------------------------------------------------------
int C_QGT::Step3DO()
{	int pm = 0;
	if (!globals->noOBJ) pm = tcm->Locate3DO(this);
	//---Open OSM database ----------------------------------
	osmDB = globals->scn->RegisterOSM(this);
	Step = TC_QT_OS1;
	return pm;
}
//-------------------------------------------------------------------------
//	Load Objects from OSM layer
//	Load up to 8 Supertiles
//-------------------------------------------------------------------------
int C_QGT::StepOSM()
{	if (0 == osmDB)		return 0;
	osmDB = globals->scn->LoadOSMLayer(osmDB,1000);
	return (osmDB != 0);
}
//-------------------------------------------------------------------------
//	Check for identity
//-------------------------------------------------------------------------
bool C_QGT::AreWe(U_INT qx,U_INT qz)
{	bool ok = (qx == xKey) && (qz == zKey);
	if (!ok)		return false;
	return true;
}
//-------------------------------------------------------------------------
//  HELPER:  return QUAD pointer with
//  sp = Super Tile No
//  rx = Relative X coordinate of DT in Super Tile [0-3]
//  rz = Relative Z coordinate of DT in Super Tile [0-3]
//-------------------------------------------------------------------------
CmQUAD *C_QGT::GetQuad(U_INT sp,U_SHORT rx,U_SHORT rz)
{ U_INT bx  = SuperDT[sp].dx;                      // Base DET X in Super Tile
  U_INT bz  = SuperDT[sp].dz;                      // Base DET Z in Super Tile
  U_INT nt  = ((bz | rz) << TC_BY32) | (bx | rx);
  return &qTAB[nt];
}
//-------------------------------------------------------------------------
//  HELPER:  return QUAD pointer with
//	tx and tz as Tile indices
//-------------------------------------------------------------------------
CmQUAD *C_QGT::GetQuad(U_INT ax,U_INT az)
{	//--- Set absolute number in QGT --------
	if ((ax < 0) || (ax > 31))	gtfo("Bad x tile index");
	if ((az < 0) || (az > 31))	gtfo("Bad z tile index");
	U_INT nt = (az << 5) | ax;  // Index is (32 * az + ax)
	return &qTAB[nt];
}
//-------------------------------------------------------------------------
//	HELPER:  Reallocate vertices in Detail Tile
//-------------------------------------------------------------------------
void C_QGT::Reallocate(CmQUAD *qd)
{	CSuperTile *sup = qd->GetSuperTile();
	if (0 == sup)	return;
	sup->Reallocate(vbu);
	return;
}
//-------------------------------------------------------------------------
//  Put SuperTile ouside to force Texture reloading
//-------------------------------------------------------------------------
int C_QGT::PutOutside()
{ int tx,tz;
  tcm->GetTileIndices(tx,tz);
  CTextureDef *txn = GetTexDescriptor(tx,tz);
  CSuperTile   *sup = GetSuperTile(tx,tz);
  NearQ.Lock();
  CSuperTile   *sp  = NearQ.GetFirst();
  NearQ.UnLock();
  while (sp)
  { //--------Leaving inner circle ---------------------------
    if (sp != sup) {sp = NearQ.GetNext(sp); continue;}
		sp->RenderOUT();
    txw->FreeAllTextures(sp);
    sp->RazNames();
    CSuperTile *nx = NearQ.Detach(sp);
		sp->WantINQ();
    FarsQ.PutLast(sp);
    tcm->FormatName(this,sp);
    return 0;
  }
  return 0;
}

//-------------------------------------------------------------------------
//  Recompute the near limit for all Far super Tiles
//  NOTE:  All distance computed are squared to avoid root extraction
//         Order relation is preserved provided limits are also squared
//-------------------------------------------------------------------------
int C_QGT::UpdateInnerCircle()
{ //----------------------------------------------------------	
	SPosition *pos = tcm->PlaneArcsPos();
  NearQ.Lock();
  CSuperTile *sp = NearQ.GetFirst();
  float     ndis = tcm->GetMedDist();
  float     hird = tcm->GetHigDist();
  bool      hiok = tcm->HiResPermited();
  //---Change State for all NEAR ST leaving the inner circle ---
  while (sp)
  { sp->dEye = tcm->AircraftFeetDistance(sp->mPos);
    //--------Leaving inner circle ---------------------------
    if (sp->dEye > ndis)
    { sp->RenderOUT();
      txw->FreeAllTextures(sp);
      CSuperTile *nx = NearQ.Detach(sp);
			sp->WantINQ();
      FarsQ.PutLast(sp);
      sp  = nx;
      continue;
    }
    //-------Leaving hi resolution radius -------------------
    if (sp->NeedMedResolution(hird))
    { sp->WantALT();
      sp->aRes = TC_MEDIUM; 
      CSuperTile *nx = NearQ.Detach(sp);
      LoadQ.Lock();
      LoadQ.PutLast(sp);
      LoadQ.UnLock();
      sp  = nx;
      continue;
    }
    //------Needing hi resolution ---------------------------
    if (sp->NeedHigResolution(hird) && hiok)
    { sp->WantALT();
      sp->aRes = TC_HIGHTR; 
      CSuperTile *nx = NearQ.Detach(sp);
      LoadQ.Lock();
      LoadQ.PutLast(sp);
      LoadQ.UnLock();
      sp  = nx;
      continue;
    }
    //-------------------------------------------------------
    sp = NearQ.GetNext(sp);
    
  }
  NearQ.UnLock();
  //------------------------------------------------------------
  //  When a Super Tile is entering the inner circle
	//	Vertex buffer is allocated and the 
  //  Super Tile is put in the Load texture Queue 
  //  processed by the file thread
  //-----------------------------------------------------------
  LockState();
  sp = FarsQ.GetFirst();
  while (sp)
    { sp->dEye = tcm->AircraftFeetDistance(sp->mPos);
      if (sp->dEye > ndis)  {sp = FarsQ.GetNext(sp); continue;}
      sp->WantLOD();
      sp->Reso = TC_MEDIUM;
      tcm->GetTransitionMask(this,sp);
			sp->AllocateVertices(vbu);
      CSuperTile *nx = FarsQ.Detach(sp);
      LoadQ.Lock();
      LoadQ.PutLast(sp);
      LoadQ.UnLock();
      sp  = nx;
    }
  UnLockState();
  //-----------------------------------------------------------
  //  Activate the Texture Thread if LoadQ is not empty
  //-----------------------------------------------------------
  if (LoadQ.NotEmpty())  tcm->RequestTextureLoad(this);
  return 1;
}
//------------1---------------------------------------------------------
//  Enter Super Tile in Near Queue
//---------------------------------------------------------------------
void C_QGT::EnterNearQ(CSuperTile *sp)
{ NearQ.Lock();
  NearQ.PutLast(sp);
  NearQ.UnLock();
  return;
}
//---------------------------------------------------------------------
//  Get the first load
//---------------------------------------------------------------------
CSuperTile *C_QGT::PopLoad()
{ LoadQ.Lock();
  CSuperTile *sp = LoadQ.Pop();
  LoadQ.UnLock();
  return sp;
}
//---------------------------------------------------------------------
//  Free all near textures
//---------------------------------------------------------------------
void C_QGT::FlushTextures()
{ CSuperTile *sp = NearQ.Pop();
  while (sp)
  { txw->FreeAllTextures(sp);    
    FarsQ.PutLast(sp);
    sp  = NearQ.Pop();
  }
 return;
}

//-------------------------------------------------------------------------
//  Request from main thread to change step
//  Manage conflict between main thread and file thread
//  If the requested step is delete, then 
//  1) The QGT is under file Thread (State WFF):  The delete state is defered
//      till the end of IO.
//  2) The QGT is under main Thread .  The delete state is immediate
//      the QGT is entering the action Queue if it was not into.
//-------------------------------------------------------------------------
void C_QGT::SetStep(U_CHAR st)
{ //--- Lock mux to avoid thread contention --------------
  LockState();
	if (st == TC_QT_DEL)  dead = 1;       // Mark as dead QGT
	//--- Process according to state ----------------------
	switch (Step)
	{	//--- Waiting I/O --------------------------	
		case TC_QT_WFF:											// waiting IO
			nStp	= st;												// Next step
			break;
		//--- Ready. Outside active Queue -----------
		case TC_QT_RDY:											// Ready state
			Step	= st;												// Immediate
			if	(dead)	tcm->InActQ(this);		// activate QGT
			break;
	 //--- Default: state is immediate -------------
		default:
			Step  = st;												// Immediate
			break;
	}
  //--- Unlock mux ---------------------------------------
  UnLockState();
}
//-------------------------------------------------------------------------
//  Request from file thread to post next step
//  If next step is not the READY state, then put QGT in action Queue
//  NOTE:  Change state and queuing must be atomic
//------------------------------------------------------------------------
void C_QGT::PostIO()
{ LockState();
  Step    = nStp;                 // Assert new State
  if (Step != TC_QT_RDY) tcm->InActQ(this);  
  UnLockState();
  return;
}
//-------------------------------------------------------------------------
//  ABORT FOR ERROR
//-------------------------------------------------------------------------
void C_QGT::Abort(char *msg)
{ char edt1 [128] = {'\0'};
  char edt2 [128] = {'\0'};
  SPosition pos = globals->geop;
  EditLon2DMS(pos.lon, edt1);
  EditLat2DMS(pos.lat, edt2);  
  gtfo("QGT- %03d-%03d ERROR :%s [%s %s]",xKey,zKey,msg, edt1, edt2);
  return;
}
//-------------------------------------------------------------------------
//  ABORT FOR VERTEX
//-------------------------------------------------------------------------
void C_QGT::Abortv()
{ char edt1 [128] = {'\0'};
  char edt2 [128] = {'\0'};
  SPosition pos = globals->geop;
  EditLon2DMS(pos.lon, edt1);
  EditLat2DMS(pos.lat, edt2);   
  gtfo("Tile mesh error: No more memory for Vertices [%s %s]", edt1, edt2);
  return;
}
//-------------------------------------------------------------------------
// Check if supertiles are all textured
//-------------------------------------------------------------------------
bool C_QGT::AllTextured(char opt)
{	CSuperTile *sp = Super;
  for (int No = 0; No != TC_SUPERT_NBR; No++,sp++)
	{	if (sp->IsTextured(opt)) continue;
		else	return false;	}	
	//--- OK ready -------------------------------------
	return true;
}
//-------------------------------------------------------------------------
//  Draw the SuperTile Mesh by the camera cam
//-------------------------------------------------------------------------
void C_QGT::DrawSuperMesh(CCamera *cam, char d)
{ CSuperTile *sp = Super;
  for (int No = 0; No != TC_SUPERT_NBR; No++,sp++)
  { //---Check visibility ----------------------------------
		char vi = d || cam->BoxInFrustum(sp->mPos,sp->sRad, 1.8);
    sp->SetVisibility(vi);
    if (0 == vi) continue;
    //---Render Super Tile ---------------------------------
		tcm->IncDSP();
		sp->Draw();
  }
	//--- Check for contour ---------------------------------
  return;
}
//-------------------------------------------------------------------------
//  Draw the QGT contour
//-------------------------------------------------------------------------
void C_QGT::DrawContour()
{	glDisableClientState(GL_VERTEX_ARRAY);
	glPushMatrix();
  CVertex   sw = *Corner[TC_SWCORNER];
  CVertex   nw = *Corner[TC_NWCORNER];
  CVertex   ne = *Corner[TC_NECORNER];
  CVertex   se = *Corner[TC_SECORNER];
	//--- Adjust East and north coordinates ------
	double dtl = LatitudeIncrement(zKey);
	se.rx    += TC_ARCS_PER_QGT;
	ne.rx		 += TC_ARCS_PER_QGT;
	ne.ry    += dtl;
	nw.ry    += dtl;
	//--- Save paramaters and set others ---------
  float     col[4];
  U_CHAR    dep = glIsEnabled(GL_DEPTH_TEST);
  U_CHAR    lit = glIsEnabled(GL_LIGHTING);
  glGetFloatv(GL_CURRENT_COLOR,col);
  //---Disable depth and set red color -----
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glColor3f(1,0,0);
  glLineWidth(4);

  //----------------------------------------
  glBegin(GL_POLYGON);
	//--- Draw South border ------------------
	glVertex3d(sw.rx, sw.ry, sw.rz);
	glVertex3d(se.rx, se.ry, se.rz);
	glVertex3d(ne.rx, ne.ry, ne.rz);
	glVertex3d(nw.rx, nw.ry, nw.rz);
  glEnd();

  //---Restore depth and color --------------
  if (dep) glEnable(GL_DEPTH_TEST);
  if (lit) glEnable(GL_LIGHTING);
  glColor4fv(col);
  glPopMatrix();
	glEnableClientState(GL_VERTEX_ARRAY);
	glLineWidth(1);
  return;
}

//-------------------------------------------------------------------------
//  Draw the wire frame
//  
//-------------------------------------------------------------------------
void C_QGT::DrawWiresSuperTile(CSuperTile *sp)
{ glPolygonMode(GL_FRONT,GL_LINE);
  glColor3fv(tcm->GetFogColor());
  sp->DrawTour();
  //--------------------------------------------
  glPolygonMode(GL_FRONT,GL_FILL);
  return;
}
//=========================================================================
//  TERRAIN CACHE MANAGER
//  Initialize the Cache Manager
//
//  Horizon paramters:
//    Inner is the inner circle where textures are defined
//    hLine is the distance to horizon
//    To limit the number of textures on upper latitude, the inner circle is 
//    linearily reduced for latitude above (below) QGT Z = 105 (59°).  At Z = 59° 
//    the visibility is 38 nmiles for about 620 SuperTiles in the inner circle
//    This is the maximum number of super tiles admited. So the max visibility
//    should be reduced past this latitude.
//  
//=========================================================================
TCacheMGR::TCacheMGR()
{ //--------------------------------------
  rKEY    = 0xFFFFFFFF;
  dTime   = 0;
  eTime   = 0;
  NbQTR   = 0;
  NbSEA   = 0;
  NbREG   = 0;
  NbTEX   = 0;
  aTime   = 0;
	lumn    = 0;
	strn		= 0;
  int ter = 0;
	GetIniVar("TRACE","TerrainCache",&ter);
	tr			= ter;
  td      = 0;                      // THREAD DEBUG indicator
  qRDY    = 0;
  scale.x = TC_FEET_PER_ARCSEC;     // Scaling parameters
  scale.y = TC_FEET_PER_ARCSEC;
  scale.z = 1.0;
  mask    = GL_ENABLE_BIT | GL_TRANSFORM_BIT | GL_POLYGON_BIT | GL_FOG | GL_LIGHTING_BIT | GL_COLOR;
  Terrain = 0;
  Tele    = 0;
  //----Sun Radius base on average apparent diameter in[0.5244°,0.5422°]------
  double alf = DegToRad(double(0.533333) * 0.5);
  sunT = tan(alf);
  //-------------------------------------------------------------
  clock   = 0;
  clock1  = 0;
  //------Read Parameters ---------------------------------------
  float mv = globals->maxView;            // Default Max view
  GetIniFloat("Performances", "MaxView", &mv);
  globals->maxView  = mv;
  InitQgtTable(mv);		// Init QGT tables
	//--- Using hi resolution -------------------------------------
	int pm = 1;
  GetIniVar  ("Performances", "UseHiResolution", &pm);
  HiRes   = U_CHAR(pm);
	//--- Hi resolution radius ------------------------------------
  float rt = globals->highRAT;            // Hight resolution ratio
  GetIniFloat("Performances","HighResRatio",&rt);
  globals->highRAT = rt;
	//--- Use default elevation -----------------------------
	pm = 0;
	GetIniVar  ("Performances","UseDefaultElevations", &pm);
	strn	= pm;
  //---Check for Terrain Wire Frame -----------------------
  pm    = 0;
  GetIniVar("Sim", "NoTerrain", &pm);
  wire    = pm;
  if (wire) globals->noTER++;
  if (wire) globals->noAPT++;
  if (wire) globals->noOBJ++;
	if (wire) globals->noOSM++;
  if (wire) globals->noMET++;
  if (wire) globals->noAWT++;
  //------Magnetic refresh indicator ---------------------
  magRF   = 1;
  //------Init horizon parameters ------------------------
  higRAT  = rt;
  medRAD  = 0;  //FN_FEET_FROM_MILE(mv);        // Near Distance
  higRAD  = 0;  //mv * higRAT;                  // Hi resolution max            
  fDens   = 0;
  cCam    = 0;
	//-----Init ground plane constant terms ---------------------
	gplan[0]	=   0;
  gplan[1]	=   0;
  gplan[2]	=   1;
  //-----Init Files access ------------------------------------
  pthread_mutex_init (&qtrMux,  NULL);
  pthread_mutex_init (&seaMux,  NULL);
  pthread_mutex_init (&ctrMux,  NULL);
  //-------------------------------------------------------------
  //     Init texture parameters 
  //      This is based on a default resolution of R=3
  //      64 quads per Detail tiles giving 
  //      an elevation array of 9² vertices d=(2^3)+1
	//	FactEV is a shift factor to get index in coordinate
	//	table (TexRES) from vertex indice. The vertex indice is
	//	shifted right (divided) by FactEV.
  //-------------------------------------------------------------
  glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
  glHint(GL_TEXTURE_COMPRESSION_HINT,GL_NICEST);
  int esid  = (1 << TC_MAX_ELV_LEV) + 1;      // Array side
  LastEV    = esid - 1;                       // Last indice in elevation
	FactEV    = TC_BY1024 - TC_MAX_ELV_LEV; 
  TexRES[TC_MEDIUM] = InitTextureCoord(esid,TC_MEDIUM);
  TexRES[TC_HIGHTR] = InitTextureCoord(esid,TC_HIGHTR);
  TexRES[TC_EPDRES] = InitTextureCoord(esid,TC_EPDRES);
  //--------Open the type tile file -----------------------------
  tFIL      = new CTerraFile("DATA/TILETYPE.IMG");
  //---------Init colors ----------------------------------------
  fogC[3] = 1.0f;
  //-------------------------------------------------------------
  EmsColor[0] = 0.20f;
  EmsColor[1] = 0.20f;
  EmsColor[2] = 0.20f;
  EmsColor[3] = 1.0f;
  //------------------------------------------------------------
  NitColor[0] = 0.0f;
  NitColor[1] = 0.0f;
  NitColor[2] = 0.0f;
  NitColor[3] = 1.0f;
  //--------Init horizon parameters ----------------------------
  ground  = gluNewQuadric();
  sphere  = gluNewQuadric();
  gluQuadricDrawStyle(ground,GLU_FILL);
  gluQuadricDrawStyle(sphere,GLU_FILL);
  //-----------Create Texture Manager ---------------------------
  //MEMORY_LEAK_MARKER ("txw");
  txw  = new CTextureWard(this,tr);
  //MEMORY_LEAK_MARKER ("txw");
  //-----------Create airport model -----------------------------
  //MEMORY_LEAK_MARKER ("aptMGR");
  aptMGR  = new CAirportMgr(this);
  //MEMORY_LEAK_MARKER ("aptMGR");
  //-----------Create 3D object model ---------------------------
  //MEMORY_LEAK_MARKER ("objMGR");
  objMGR  = new C3DMgr(this);
  //MEMORY_LEAK_MARKER ("objMGR");
  //------Get Night color----------------------------------------
  UpdateTOD();
  //-----Init File Thread ------------------------------------
  int pmax = sched_get_priority_max (SCHED_OTHER);
  int pmin = sched_get_priority_min (SCHED_OTHER);
  int pmid = (pmin + pmax) >> 1;
  thSIG = 0;
  thRCV = 0;
  pthread_cond_init  (&thCond, NULL);
  pthread_mutex_init (&thMux,  NULL);
  pthread_attr_t attr;
  sched_param param;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE);
  pthread_attr_setschedpolicy (&attr, SCHED_OTHER);
  pthread_attr_setinheritsched (&attr, PTHREAD_EXPLICIT_SCHED);
  param.sched_priority = pmax;            //sched_get_priority_max (SCHED_OTHER);
  pthread_attr_setschedparam (&attr, &param);
  //--- Create thread for file access ---------------------------
  thRUN   = 0;                              // Thread stop
  int rs  = pthread_create (&thIden, &attr, FileThread, this);
  if (0 != rs)  Abort("Cannot Create File Thread");
  pthread_attr_destroy(&attr);
  //-----Reserve a big bloc of memory to ensure continuity ------
  char *res = new char[2000000];
  delete [] res;
  //-----Get country names ---------- ---------------------------
  CDbCacheMgr *dbc = globals->dbc;
  SqlMGR      *sqm = globals->sqm;
  (globals->genDB)?(sqm->GetCountryName(this)):(dbc->GetCountryName(this));
  //-----Get Terrain type from memory ---------------------------
  terBOX = new CListBox();
  LoadPodTerra();
  LoadSqlTerra();
  //-----Check for teleport directive ---------------------------
  CheckTeleport();
	//-----Init elevation Tracker ---------------------------------
	globals->etrk.SetTCM(this);
	//--- Enter in dispatcher -------------------------------------
	globals->Disp.Enter(this, PRIO_TERRAIN, DISP_EXCONT, 1);
TRACE("ENd TCACHE Constructor");
}
///------------------------------------------------------------------------
//  End of TCache: TODO 
//  Stop thread
//  delete all allocated resources
//-------------------------------------------------------------------------
TCacheMGR::~TCacheMGR()
{ globals->sql->Stop();
	pthread_cond_signal(&thCond);
  pthread_join(thIden,0);
  //---delete coordinate tables ----------------------
  delete [] TexRES[TC_MEDIUM];
  delete [] TexRES[TC_HIGHTR];
  delete [] TexRES[TC_EPDRES];
  //---delete airports -------------------------------
  delete aptMGR;
  //---delete all QGT --------------------------------
  std::map<U_INT,C_QGT*>::iterator im;
  for (im = qgtMAP.begin(); im != qgtMAP.end(); im++)
  { C_QGT *qt = (*im).second;
		delete (qt);			//qt->DecUser();
  }
  qgtMAP.clear();
  //---delete all countries ---------------------------
  std::map<U_INT,char*>::iterator ic;
  for (ic = ctyMAP.begin(); ic != ctyMAP.end(); ic++)
  { char  *nm = (*ic).second;
    SAFE_DELETE_ARRAY (nm); 
  }
  ctyMAP.clear();
	//--- Delete unused coast data -----------------------
	std::map<U_INT,C_SEA*>::iterator ra;
	for (ra = seaMAP.begin(); ra != seaMAP.end(); ra++) delete (*ra).second;
	seaMAP.clear();
	std::map<U_INT,C_CDT*>::iterator rb;
	for (rb = cstMAP.begin(); rb != cstMAP.end(); rb++) delete (*rb).second;
	cstMAP.clear();
  //---delete all terrain types -----------------------
  terBOX->EmptyIt();
  delete terBOX;
  //----Close tile types ------------------------------
  delete tFIL;
  gluDeleteQuadric(ground);
  gluDeleteQuadric(sphere);
  delete objMGR;
  delete txw;
  globals->tcm = 0;
}
//-------------------------------------------------------------------------
//  Catch one Terra line
//-------------------------------------------------------------------------
void TCacheMGR::OneTerraLine(CTgxLine *lin)
{ terBOX->AddSlot(lin);
  return;
}
//-------------------------------------------------------------------------
//  Catch one country line
//-------------------------------------------------------------------------
void TCacheMGR::OneCountry(CCtyLine *cty)
{ int  lgr  = strlen(cty->GetSlotName()) + 1;
  char *cn  = cty->GetSlotKey();
  U_INT key = (cn[0] << 8) | (cn[1]);
  //MEMORY_LEAK_MARKER ("nm1")
  char *nm  = new char[lgr];
  //MEMORY_LEAK_MARKER ("nm2")
  strncpy(nm,cty->GetSlotName(),lgr);
  ctyMAP[key] = nm;
  return;
}
//-------------------------------------------------------------------------
//  Load Terra data from POD
//-------------------------------------------------------------------------
void TCacheMGR::LoadPodTerra()
{ TC_TERRA *src = globals->terTAB;
  while (src->type != -1)
  { CTgxLine *lin = new CTgxLine;
    lin->SetType(src->type);
    lin->SetNite(src->nite);
    lin->SetLabel(src->type,src->desc);
    terBOX->AddSlot(lin);
    src++;
  }
  return;
}
//-------------------------------------------------------------------------
//  OverLoad Terra data from SQL if present
//-------------------------------------------------------------------------
void TCacheMGR::LoadSqlTerra()
{ if (0 == globals->texDB)  return;
  //---delete all terrain types -----------------------
  terBOX->EmptyIt();
  globals->sqm->GetTerraSQL(this);
  return;
}
//-------------------------------------------------------------------------
//  return a country name
//-------------------------------------------------------------------------
char *TCacheMGR::GetCountry(char *cn)
{ U_INT key = (cn[0] << 8) | (cn[1]);
  std::map<U_INT,char*>::iterator ct = ctyMAP.find(key);
  return(ct == ctyMAP.end())?(0):((*ct).second);
}
//-------------------------------------------------------------------------
//  Set pixels per miles for vector map
//-------------------------------------------------------------------------
void TCacheMGR::SetPPM(double rat,short wd,short ht)
{ ppM.x   = rat * rFactor;
  ppM.y   = rat;
  sDIM.x  = wd;
  sDIM.y  = ht;
  return;
}
//-------------------------------------------------------------------------
//Check for teleport
//  TODO: Distinguish between aircraft on ground at rest and
//        aircraft in flight
//        Actually only aircraft at rest is well working
//-------------------------------------------------------------------------
void TCacheMGR::CheckTeleport()
{ SPosition Port;
  char qgt[64];
  qgt[0]  = 0;
  GetIniString("Terrain","Teleport", qgt,10);
  if (*qgt == 0)      return;
  //-----Decode QGT X and Z -----------------------------
  U_INT qx  = atoi(qgt+1);
  U_INT qz  = atoi(qgt+5);
  if ((qx < 0) || (qx > 511)) return;
  if ((qz < 0) || (qz > 511)) return;
  Port.lat  = GetQgtSouthLatitude(qz);
  Port.lon  = FN_ARCS_FROM_QGT(qx);
  Port.alt  = 15000;
  globals->geop = Port;
  return;
}
//-------------------------------------------------------------------------
//  Teleport to requested position P at Orientation O
//-------------------------------------------------------------------------
void TCacheMGR::Teleport(SPosition *P, SVector *O)
{ CVehicleObject *veh = globals->pln;
  if (0 == veh)													return;
  if (veh->HasState(VEH_INOP))					return;
	if (globals->aPROF.Has(PROF_NO_TEL))	return;
	globals->sit->EnterTeleport(P,O);
  globals->m3d->ReleaseVOR();
  Terrain       = 0;
  return;
}
//-------------------------------------------------------------------------
//  Allocate texture coordinates for a given texture size
//   -For each texture size (texture resolution) there is a table
//    giving the texture coordinates S,T for a given vertex (x,z) of the
//    detail tile.
//   -Each table is computed for the maximum elevation resolution
//    supported (here a grid of 17 * 17 = 289 vertices
//   -Then each vertex has an index giving its position in any of the table
//-------------------------------------------------------------------------
//    The texture level (RES) gives the texture size:
//    1 => Med resolution 128 pixels
//    2 => Hig resolution 256 pixels  
//    Actually up to level 3 (512 pixels) is supported
//    
//    Due to texture border recovery, the starting point is given
//    by the DTO parameters (Texture origin)
//  
//    The DTU parameter gives the texture size, that is the texture width
//    diminished by the recovery region on both sides (8 pixels per side)
//
//    Thus is N is the number of elevations on each side, the coordinates
//    of vertex(X,Z) are
//        S = DTO + X*(DTU/N)
//        T = DTO + Z*(DTU/N)
//        where X and Z are in [0,N]
//-------------------------------------------------------------------------
float *TCacheMGR::InitTextureCoord(int dim,int res)
{ float  dto;
  float  dtu;
  float *arv = new float[dim];
  txw->GetTextParam(res,&dto,&dtu);
  for     (int k = 0; k != dim; k++)  arv[k] = dto + ((k * dtu) / LastEV);
  return arv;
}

//-------------------------------------------------------------------------
//  Abort for error
//-------------------------------------------------------------------------
void TCacheMGR::Abort(char *msg)
{ gtfo("Terrain Cache: %s",msg);
  return;
}
//----------------------------------------------------------------------
//  Get Tile name
//----------------------------------------------------------------------
void TCacheMGR::GetTileName (U_CHAR *base)
{ U_INT ax = (cQGT->GetXkey() << TC_BY32) | tx;
  U_INT az = (cQGT->GetZkey() << TC_BY32) | tz;
  tFIL->GetBase(ax,az,(char*)base);
  return;
}
//----------------------------------------------------------------------
//  Make a Generic texture name
//    The name is formed as follow:
//    AAXXBBYYij  where AA XX BB abd YY are terrain type
//    for     AX   A is the current type, X is tile to the right
//            BY   B is tile to bottom and Y is the diagonal tile
//    (i,j) are X,Z indices of the Tile in SuperTile
//----------------------------------------------------------------------
U_CHAR TCacheMGR::BuildName(CTextureDef *txn,U_SHORT nx,U_SHORT nz)
{ if (txn->HasName())  return 0;
  char *base = txn->Hexa;
  char *dst  = txn->Name; 
  GetTileName((U_CHAR*)base);
  nz = 3 - nz;
  for (int k = 0; k != 4;k++)
      { U_CHAR h = base[k];
        *dst++ = HexTAB[h >> TC_BY16];
        *dst++ = HexTAB[h &  TC_016MODULO];
      }
  *dst++  = HexTAB[nx];               // I= X indice in ST
  *dst++  = HexTAB[nz];               // J= Z indice in ST
  *dst    = 0;
  //--------Assign texture type -as generic ----------------------
  txn->TypTX  = TC_TEXGENER;         
  //--------Build Shared key -------------------------------------
  U_CHAR k1 = base[0];                            // Terrain type
  txn->sKey  = (k1 << 8)|(nx << 4)|(nz);          // XZ indices
  return *txn->Hexa;
}
//-------------------------------------------------------------------------
//  1) Build all textures name for the super Tile (except those coming
//  from TRN file)
//  2) Enter the QUAD pointer
//-------------------------------------------------------------------------
void TCacheMGR::FormatName(C_QGT *qt,CSuperTile *sp)
{ cQGT    =  qt;                                        // Save QGT
  for ( U_INT nd    = 0;nd != TC_TEXSUPERNBR;nd++) 
  {     U_INT nz    = (nd >> TC_BY04);                  // Z number
        U_INT nx    = (nd &  TC_004MODULO);             // X number
        //----------------------------------------------------------
        tx          = SuperDT[sp->NoSP].dx | nx;         // Tile X index in QGT
        tz          = SuperDT[sp->NoSP].dz | nz;         // Tile Z index in QGT
        U_INT   No  = FN_DET_FROM_XZ(tx,tz);             //(tz << TC_BY32) | tx;
        CmQUAD *qd  = qt->GetQUAD(No);                   // Detail tile center
        CTextureDef *txn = &sp->Tex[nd];
        txn->quad   = qd;
        char gdn    = BuildName(txn,nx,nz);
        qd->SetGroundType(gdn);                          // Type of tile
  }
  //----------------------------------------------------------------
  return;
}
//-------------------------------------------------------------------------
//  Compute the type of Tile for transition
//  dir specifies the tile position AA-BB-XX-YY
//    0   is a Land  Tile  (pure land)
//    1   is a Water Tile  (0C.....)
//    2   is a Coast Tile  (There are coast data)
//  NOTE this is to prohibit transition between coast and water tiles
//-------------------------------------------------------------------------
char  TCacheMGR::GetTileType(char tn,int dir)
{ if (tn == 0x0C)      return 1;
  U_INT tx  = (ax + TransTAB[dir].dx) & TC_WORLDDETMOD;   // Next Tile X
  U_INT tz  = (az + TransTAB[dir].dz) & TC_WORLDDETMOD;   // Next Tile Z
  U_INT kc  = (tx << 16) | tz;                            // Globe tile key
  char *cdt = (globals->seaDB)?(GetCDTdata(kc)):(GetSEAdata(tx,tz));
  return (cdt)?(2):(0);
}
//-------------------------------------------------------------------------
//  Set Transition Mask in the Detail Tile
//  NOTE:   Due to the net separation introduced by the coast lines
//          transition between water and coast tiles are prohibited
//          Combination between coast and water is detected via tile type
//          0 => Land
//          1 => Water
//          2 => Coast
//          By oring the types, any value of 3 is prohibited
//--------------------------------------------------------------------------
void TCacheMGR::SetTransitionMask(C_QGT *qgt,CTextureDef *txn)
{ ax  = txn->quad->GetTileAX();
  az  = txn->quad->GetTileAZ();
//--- Debugg: Use statement to check a specific detail tile ------------
//    if (txn->AreWe(TC_ABSOLUTE_DET(341,3),TC_ABSOLUTE_DET(317,23)))
//          txn = txn;                  // Set break point here
//				int ok = strcmp(txn->Name,"656C0F03");
//				if (ok ==  0)
//					int	a = 1;

//--- end debuging -----------------------------------------------------
  U_INT kc      = (ax << 16) | az;
  char *cdt     = (globals->seaDB)?(GetCDTdata(kc)):(GetSEAdata(ax,az));
  txn->SetCoast(cdt);
//  if (txn->Name[10]	== '*')   return;								 // This is a user texture
	if (txn->UserTEX())					return;
  if (cdt)      {txn->TypTX = TC_TEXCOAST;}          // this is a coast tile
  char *nam     = txn->Hexa;
//--- Debugg ---------------------------------------------------------------
//  if (strncmp(txn->Name,"0C8C0C0C",8) == 0)
//    int a = 0;                          // Break here
   
  //------Init the current tile coast  type---------------------------------
  char TA   =   GetTileType(nam[0],TC_TILE_A);  // Type of (this) A Tile
  char TX   =   GetTileType(nam[1],TC_TILE_X);  // Type of        X Tile
  char TB   =   GetTileType(nam[2],TC_TILE_B);  // Type of        B Tile
  char TY   =   GetTileType(nam[3],TC_TILE_Y);  // Type of        Y Tile
   //------Inhibit prohibited transition between water and coast tile ------
  U_CHAR  msk  = ((TA | TB) == 3)? (0):TC_BOTTRANSITION; // Bottom Transition allowed
          msk |= ((TA | TY) == 3)? (0):TC_CNRTRANSITION; // Corner transition allowed
          msk |= ((TA | TX) == 3)? (0):TC_RGTTRANSITION; // Right transition  allowed            
  //--------Final transition is land without prohibited transitions --------
  txn->Tmask  = msk & GetLandTransition(nam);
  //--------Compute sea layer -----------------------------------------------
  U_CHAR  inx = TA | TX | TB | TY;              // Global type
  //--------Check for pure Water Texture (no transition)---------------------
  if (txn->Tmask)       return;                 // Transition prohibit pure water
  if (txn->Hexa[0] == 0x0C)  txn->TypTX = TC_TEXWATER;
  return;
}
//-------------------------------------------------------------------------
//  Return land transition Mask
//  Hex is AXBY
//	AX
//	BY
//-------------------------------------------------------------------------
U_CHAR TCacheMGR::GetLandTransition(char *hex)
{ U_CHAR lnd = 0;
	bool nax = (hex[0] != hex[1]);
	bool nab = (hex[0] != hex[2]);
	bool nay = (hex[0] != hex[3]);
  if          (nax) lnd |= TC_RGTTRANSITION;     // A != X
  if          (nab) lnd |= TC_RBTTRANSITION;     // A != B
  if          (nay) lnd |= TC_CNRTRANSITION;     // A != Y
  if          (hex[1] != hex[3]) lnd |= TC_CNRTRANSITION; 
  return lnd;
}
//-------------------------------------------------------------------------
//  Set the transition mask for the Super Tile
//-------------------------------------------------------------------------
int TCacheMGR::GetTransitionMask(C_QGT *qgt,CSuperTile *sp)
{	for (U_INT nd = 0;nd != TC_TEXSUPERNBR;nd++)  SetTransitionMask(qgt,&sp->Tex[nd]);
  return 1;
}
//-------------------------------------------------------------------------
//  Trace aircraft position
//-------------------------------------------------------------------------
void TCacheMGR::Probe(CFuiCanva *cnv)
{ char txt[256];
  char fnm[8];
  char lat[32];
  char lon[32];
  if (!Spot.ValidQGT())   
		return;
  C_QGT *qgt = Spot.qgt;                        //aQGT;
	Spot.Edit(txt);
  cnv->AddText( 1,txt,1);
  //--- Texture name -----------------------------------
  if (qgt->NotReady())    return;
  CTextureDef *txn = GetTexDescriptor(qgt,Spot.xDet(),Spot.zDet());
  char *fn = txn->Name;
  fnm[0] = fn[0];                             // Tile type
  fnm[1] = fn[1];
  fnm[2] = fn[8];                             // I index
  fnm[3] = fn[9];                             // J index
  fnm[4] = '5';
  fnm[5] = 'D';
  fnm[6] = 0;
  _snprintf(txt,256,"T-Name %s.  File %s. Ty=%d F=%#x Tr=%s",txn->Name,fnm,txn->TypTX,txn->xFlag,TrsTAB[txn->Tmask]);
  cnv->AddText( 1,txt,1);
  //--- Latitude -longitude ----------------------------
  EditLat2DMS(aPos.lat,lat);
  EditLon2DMS(aPos.lon,lon);
  _snprintf(txt,256,"Terrain %s %s ALT=%.0f feet",lat,lon,aPos.alt);
  cnv->AddText( 1,txt,1);
  return;
}
//-------------------------------------------------------------------------
//  Edit ground position
//-------------------------------------------------------------------------
void TCacheMGR::EditGround(char *txt)
{ sprintf_s(txt,127,"QGT (%03d-%03d) TILE(%02d-%02d)",xKey,zKey,Spot.xDet(),Spot.zDet());
  return;
}
//-------------------------------------------------------------------------
//  Return Absolute TILE indices for Aircraft position
//-------------------------------------------------------------------------
void TCacheMGR::GetAbsoluteIndices(U_INT &ax,U_INT &az)
{ ax  = ( xKey << TC_BY32) | (Spot.xDet());
  az  = ( zKey << TC_BY32) | (Spot.zDet());
  return;
}
//-------------------------------------------------------------------------
//  Return relative TILE indices
//-------------------------------------------------------------------------
void TCacheMGR::GetTileIndices(int &tx,int &tz)
{ tx  = Spot.xDet();
  tz  = Spot.zDet();
  return;
}
//-------------------------------------------------------------------------
//  Activate thread one cycle
//-------------------------------------------------------------------------
void TCacheMGR::ThreadPulse()
{	pthread_cond_signal(&thCond);    // Signal file THREAD
	return; }
//-------------------------------------------------------------------------
//  Time slice.  Update the terrain cache
//-------------------------------------------------------------------------
int TCacheMGR::TimeSlice(float dT, U_INT FrNo)
{ eTime     = dT;
  dTime    += dT;
  aPos      = globals->geop;                            // Update aircraft position
  //---Update QGT position at global level --------------------------------
  IndicesInQGT (aPos, globals->qgtX, globals->qgtZ);
  nKEY      = QGTKEY(globals->qgtX, globals->qgtZ);
  //--------Update aircraft position in feet ------------------------------
  rFactor				= GetReductionFactor(aPos.lat);
	globals->rdf	= rFactor;
  xFactor				= 1 / rFactor;
	globals->exf	= xFactor;
  geow.x				= FN_FEET_FROM_ARCS(aPos.lon) * rFactor;
  geow.y				= FN_FEET_FROM_ARCS(aPos.lat);
  geow.z				= aPos.alt;
  //-----Scan for Action Queue -------------------------------------------
  clock++;                                              // Internal clock
  clock1    = clock & 1023;                             // Derived clock 1
  UpdateTOD();                                          // Update Time of Day
  bool thrd = (LodQ.NotEmpty() | FilQ.NotEmpty());
  if  (thrd) pthread_cond_signal(&thCond);    // Signal file THREAD
  //-----Update QGT formation --------------------------------------------
  char action = RefreshCache();
  UpdateAGL(aPos);
	//---Update Tracker ----------------------------------------------------
	globals->etrk.TimeSlice(dT);
	//--- Update action ----------------------------------------------------
  if (action)           return 1;
  if (OneAction())      return 1;
  //-----No cache refresh.  Update magnetic deviation --------------------
  if (magRF) globals->mag->GetElements (aPos,magDV, magFD);
	globals->magDEV = magDV;
  magRF = 0;
  //---Update QGT AIRPORT AND 3D MANAGER ---------------------------------
	UpdateQGTs(dT);
  aptMGR->TimeSlice(dT);          // Update airports
  objMGR->TimeSlice(dT);          // Update 3D
  //----------------------------------------------------------------------
  if (clock1)                 return 1;
  //----------------------------------------------------------------------
  if (tr) TRACE("TCM: -- Time: %04.2f---- Actual mesh QGT=%03d QTR=%d COAST=%d Vertex=%06d -------------------",
          dTime,qgtMAP.size(),NbQTR,NbSEA,globals->NbVTX);
  if (tr) txw->TraceCTX();

  return 1;
}
//-------------------------------------------------------------------------
//  Check for new Key
//  Compute horizontal and vertical distance of proposed key
//  to the reference key.
//  If any distance is outside of reference key limits then 
//  the proposed key is a new one
//-------------------------------------------------------------------------
bool TCacheMGR::IsaNewKey(U_INT key)
{ U_SHORT rz = rKEY & TC_0512MOD;             // Reference key Z coordinate
  U_SHORT kz = key  & TC_0512MOD;             // Proposed key  Z coordinate
  U_SHORT kx = (key >> 16);                   // Proposed key  X coordinate
  U_SHORT rx = (rKEY >> 16);                  // Reference Key X coordinate
  //--------Check for horizontal limit -----------------------
  U_SHORT dh = (kx - rx) & TC_0512MOD;      // Horizontal distance Modulo 512
  if (dh > 255) dh = (512 - dh);
  U_SHORT lh = GetQgtHorizonRange(kz);
  if ((rx == 0x0000FFFF) ||(dh > lh)) 
  { if (tr) TRACE("TCM: -- Time: %04.2f %03u-%03u is a new key. DH = % 4d LH = % 4d",dTime,kx,kz,dh,lh);
    return true;
  }
  //--------Check for vertical limits -------------------------
  short dv = (kz - rz);                       // Vertical distance
  short lv = GetQgtVertEquaRange(rz);
  if ((dv > 0) && (rz > 255)) lv = GetQgtVertPoleRange(rz);      // Going north pole
  if ((dv < 0) && (rz < 256)) lv = GetQgtVertPoleRange(rz);      // Going south pole
  if (abs(dv) <= lv) return false;
  if (tr) TRACE("TCM: -- Time: %04.2f %03u-%03u is a new key. DV = % 4d LH = % 4d",dTime,kx,kz,dv,lv);
  return true;
}
//-------------------------------------------------------------------------
//  Check for obsolete Keys
//  Compute horizontal and vertical distance of proposed key
//  to the new key.
//  If any distance is outside of new key limits then 
//  the proposed key is obsolete
//-------------------------------------------------------------------------
bool TCacheMGR::IsaActKey(U_INT key)
{ short rz = nKEY & TC_0512MOD;             // Reference Key Z coordinate               
  short kz = key  & TC_0512MOD;             // Proposed  Key Z coordinate
  short rx = (nKEY >> 16);                  // Reference Key X coordinate
  short kx = (key >> 16);                   // Proposed  Key X coordinate
  //--------Check for horizontal limit -----------------------
  U_SHORT dh = (kx - rx) & TC_0512MOD;      // Horizontal distance modulo 512
  if (dh > 255) dh = (512 - dh);
  U_SHORT lh = GetQgtHorizonRange(kz);      // Range at position kz
  if (dh > lh) 
  { if (tr) TRACE("TCM: -- Time: %04.2f %03u-%03u is obsolete. DH = % 4d LH = % 4d",dTime,kx,kz,dh,lh);
    return false;
  }
  //--------Check for vertical limits -------------------------
  short dv = (kz - rz);                     // Vertical distance modulo 512
  short lv = GetQgtVertEquaRange(rz);
  if ((dv > 0) && (rz > 255)) lv = GetQgtVertPoleRange(rz);      // Going north pole
  if ((dv < 0) && (rz < 256)) lv = GetQgtVertPoleRange(rz);      // Going south pole
  if (abs(dv) <= lv) return true;
  if (tr) TRACE("TCM: -- Time: %04.2f %03u-%03u is obsolete. DV = % 4d LH = % 4d",dTime,kx,kz,dv,lv);
  return false;

}
//-------------------------------------------------------------------------
//  Get Upper and lower range depending on CZ hemispher
//-------------------------------------------------------------------------
void TCacheMGR::GetRange(U_INT cz,U_SHORT &up,U_SHORT &dn)
{ U_SHORT por = GetQgtVertPoleRange(cz);    // Pole range
  U_SHORT eqr = GetQgtVertEquaRange(cz);    // Equator range
  if (cz > 255)
  //---NORTH HEMISPHER. Up is toward pole. Down is toward equator ----------
  {up   = (cz + por) & TC_0512MOD;
   dn   = (cz - eqr) & TC_0512MOD;
  }
  else
  //---SOUTH HEMISPHER. Up is toward equator. Down is toward pole ---------
  {up   = (cz + eqr) & TC_0512MOD;
   dn   = (cz - por) & TC_0512MOD;
  }
  return;
}
//-------------------------------------------------------------------------
//  Refresh Cache if aircraft change from QGT
//  NOTE:  Aircraft may enter new tile toward pole but cache will not enter
//        any more front tiles that are in the non -flyable area
//-------------------------------------------------------------------------
int TCacheMGR::RefreshCache()
{	U_INT cx = globals->qgtX;
  U_INT cz = globals->qgtZ;
  if (nKEY == rKEY)  return 0;
  //-----------Aircraft enters a new QGT -----------------------------
  if (tr) TRACE("TCM: -- Time: %04.2f---- Actual mesh QGT=%03d QTR=%d COAST=%d Vertex=%06d -------------------",
          dTime,qgtMAP.size(),NbQTR,NbSEA,globals->NbVTX);
  if (tr) TRACE("TCM: -- AIRCRAFT ENTER %d-%d=================================================================",
          globals->qgtX,globals->qgtZ);
  //----------Get the X reduction factor at this latitude ------------
  scale.x = rFactor * TC_FEET_PER_ARCSEC;   // Scaling parameter 1
	//----------scan all surrounding QGT tiles for new position --------
  U_SHORT vup = 0;
  U_SHORT vdn = 0;
  U_INT   key = 0;
  GetRange(globals->qgtZ,vup,vdn);
  for (int vt = vup; vt >= vdn; vt--)
  { U_SHORT hz1 = (cx - GetQgtHorizonRange(vt))     & TC_0512MOD;
    U_SHORT hz2 = (cx + GetQgtHorizonRange(vt) + 1) & TC_0512MOD;

    while (hz1 != hz2)
    { key = QGTKEY(hz1,vt);
      if (IsaNewKey(key))     CreateQGT(hz1,vt);    //Insert new tile
      hz1 = (hz1 + 1) & TC_0512MOD;
    }
  }
  //------------scan existing set for obsolete QGT ----------------------
  std::map<U_INT,C_QGT*>::iterator im;
  for (im = qgtMAP.begin(); im != qgtMAP.end(); im++)
  { C_QGT *qt   = (*im).second;
    U_INT key   = (*im).first;
    if (IsaActKey(key)) continue;
    MarkDelete(qt);
  }
  //----Keep new reference key ------------------------------------------
  C_QGT *aqg = GetQGT(cx,cz);                             // Current QGT
  if (0 == aqg) 			gtfo("CACHE: corrupted geo position: No corresponding tile");
  Spot.SetQGT(aqg);
  rKEY  = nKEY;
  xKey  = cx;
  zKey  = cz;
  iqt   = qgtMAP.begin();
  magRF = 1;                                              // Refresh mag deviation
  //----Set horizon parameters -------------------------------------------
  fDens     = GetFogDensity(cz);                          // Fog density
  medRAD    = GetMediumCircle(cz);                        // Medium circle (feet)
  higRAD    = medRAD * higRAT;														// Hi resolution (feet)
  //----Initial state --------------------------------------------------------
  return 1;
}

//-----------------------------------------------------------------------
//  Check state for deletion
//-----------------------------------------------------------------------
void TCacheMGR::MarkDelete(C_QGT *qgt)
{ qRDY -= qgt->qSTAT;                   // Update Ready count
  qgt->SetStep(TC_QT_DEL);              // Change to delete state
  return;
}
//-----------------------------------------------------------------------
//  Call Time slice for each QGT that is in ready state
//-----------------------------------------------------------------------
int TCacheMGR::UpdateQGTs(float dT)
{ if (0 == qgtMAP.size())		return 0;
	if (iqt == qgtMAP.end())  iqt = qgtMAP.begin();
  C_QGT *qgt = (*iqt).second;
  if (qgt->IsReady()) qgt->TimeSlice(dT);
  iqt++;
  return 1;
}
//-----------------------------------------------------------------------
//  Get a line for each 3D object ready to be drawed
//-----------------------------------------------------------------------
void TCacheMGR::GetObjLines(CListBox *box)
{ std::map<U_INT,C_QGT *>::iterator iq;
  for (iq = qgtMAP.begin(); iq != qgtMAP.end(); iq++)
  {  C_QGT *qgt = (*iq).second;
     if (qgt->NotReady()) continue;
     qgt->w3D.GetLine(box);
  }
  return;
}
//-----------------------------------------------------------------------------
//  Distance in feet between aircraft to position
//-----------------------------------------------------------------------------
float TCacheMGR::AircraftFeetDistance(SPosition &pos)
{ double dx = FN_FEET_FROM_ARCS(LongitudeDifference(aPos.lon,pos.lon));
  double dy = FN_FEET_FROM_ARCS(aPos.lat - pos.lat);
  double dz = aPos.alt - pos.alt;
  dx *= rFactor;
  float dq = float((dx * dx) + (dy *dy) + (dz * dz));
  return SquareRootFloat(dq);
}

//-----------------------------------------------------------------------
//  Return Ground information about the position
//  QGT is the Quarter Global Tile where the location reside
//  TX and TZ are the Detail Tile indices
//  sup is the Supertile
//  alt is the ground altitude at location
//-----------------------------------------------------------------------
void TCacheMGR::GetTerrainInfo(TC_GRND_INFO &inf,SPosition &pos)
{ C_QGT *qgt  = inf.qgt;
  if (0 == qgt) qgt =  GetQGT(pos);
  if (0 == qgt)         return;
  if (qgt->NoQuad())    return;
  qgt->GetTileIndices(pos,inf.tx,inf.tz);
  //----Access the Super Tile ----------------
  inf.qgt     = qgt;
  inf.sup     = qgt->GetSuperTile(inf.tx,inf.tz);
  //----Compute location elevation -----------
  U_INT No    = FN_DET_FROM_XZ(inf.tx,inf.tz);           
  CmQUAD *dt  = qgt->GetQUAD(No);
  CVector  p(pos.lon,pos.lat,0);
	qgt->RelativeToBase(p);
  CmQUAD *qd  = dt->Locate2D(p);
  //----Locate the triangle where p reside ------------
  qd->PointHeight(p,gNM);
  inf.alt     = p.z;
  return;
}
//-----------------------------------------------------------------------
//  get Ground Altitude at requested position
//  The position must contains the absolute longitude and latitude in arcsec
//  (not the band longitude).
//-----------------------------------------------------------------------
double TCacheMGR::GetGroundAt(GroundSpot &gns)
{ IndicesInQGT (gns);
  if (!SetQGT(gns)) return 0;
  C_QGT *qgt = gns.qgt;
	gns.ClampLon();
  qgt->GetTileIndices(gns);
  U_INT No		= FN_DET_FROM_XZ(gns.tx,gns.tz);       
  CmQUAD *dt  = qgt->GetQUAD(No);
 
  CVector  p(gns.lon,gns.lat,dt->CenterElevation());
	qgt->RelativeToBase(p);
  gns.Quad    = dt->Locate2D(p);
  //----Locate the triangle where p reside ------------
  gns.Quad->PointHeight(p,gNM);
  gns.alt     = p.z;
  return gns.alt;
}
//-----------------------------------------------------------------------
//  Update Ground altitude at aircraft position
//  -Locate the Detail Tile
//  -Locate the Quad
//-----------------------------------------------------------------------
void TCacheMGR::UpdateAGL(SPosition &pos)
{ Spot.lon  = pos.lon;
  Spot.lat  = pos.lat;
  Spot.alt  = 0;
	Spot.rdf  = rFactor;
  Terrain   = Spot.GetTerrain();
	//--- Update AGL and ground plane ---------------------
	fAGL			=		pos.alt - Spot.alt;
  gplan[3]	=  float(fAGL);			// Ground plane OK
	//--- Check for aircraft altitude ---------------------
  return;
}
//-----------------------------------------------------------------------------
//	retrun plane spot at ground altitude
//-----------------------------------------------------------------------------
void TCacheMGR::GetPlaneSpot(SPosition &p)
{	p.lon	= Spot.lon;
	p.lat = Spot.lat;
	p.alt = Spot.alt;
	return;
}
//-----------------------------------------------------------------------------
// Name: BuildShadowMatrix ()
// Desc: This is an OpenGL matrix with column first
//-----------------------------------------------------------------------------
void TCacheMGR::SetShadowMatrix( float mat[16],float lp[4])
{ // Calculate the dot-product between the plane and the light's position
  float dotp =	gplan[0] * lp[0] + 
								gplan[1] * lp[1] + 
								gplan[2] * lp[2] + 
								gplan[3] * lp[3];

  //--- First column------
  mat[0]  = dotp	- (lp[0] * gplan[0]);
  mat[4]  =	0			- (lp[0] * gplan[1]);
  mat[8]  =	0			- (lp[0] * gplan[2]);
  mat[12] =	0			- (lp[0] * gplan[3]);
  //--- Second column-----
  mat[1]  =	0			- (lp[1] * gplan[0]);
  mat[5]  = dotp	- (lp[1] * gplan[1]);
  mat[9]  =	0			- (lp[1] * gplan[2]);
  mat[13] =				- (lp[1] * gplan[3]);
  //--- Third column------
  mat[2]  =				- (lp[2] * gplan[0]);
  mat[6]  =				- (lp[2] * gplan[1]);
  mat[10] = dotp	- (lp[2] * gplan[2]);
  mat[14] =				- (lp[2] * gplan[3]);
  //--- Fourth column-----
  mat[3]  =				- (lp[3] * gplan[0]);
  mat[7]  =				- (lp[3] * gplan[1]);
  mat[11] =				- (lp[3] * gplan[2]);
  mat[15] = dotp	- (lp[3] * gplan[3]);
	return;
}
//-----------------------------------------------------------------------
//  Return the Detail tile of the current position
//  The detail tile is located, then the Descriptor
//  is updated with the SuperTile states and returned
//-----------------------------------------------------------------------
CmQUAD *TCacheMGR::GetTileQuad(SPosition pos)
{ U_INT qx,qz;
  short tx,tz;
  IndicesInQGT (pos, qx, qz);
  C_QGT  *qgt = GetQGT(qx,qz);
  if (0 == qgt) Abort("No QGT for position");
  if(!qgt->GetTileIndices(pos,tx,tz)) gtfo("Position error");;
  U_INT nq = FN_DET_FROM_XZ(tx,tz);
  CmQUAD *qd = qgt->GetQUAD(nq);
  return qd;
}
//-----------------------------------------------------------------------
//  Get Tile Quad by indices
//-----------------------------------------------------------------------
CmQUAD *TCacheMGR::GetTileQuad(U_INT ax,U_INT az)
{ U_INT qx = ax >> TC_BY32;
  U_INT qz = az >> TC_BY32;
  U_INT tx = ax &  TC_032MODULO;
  U_INT tz = az &  TC_032MODULO;
  U_INT nq = FN_DET_FROM_XZ(tx,tz);
  C_QGT  *qgt = GetQGT(qx,qz);
  if (0 == qgt) return 0;
  CmQUAD *qd = qgt->GetQUAD(nq);
  return qd;
}
//-----------------------------------------------------------------------
//  Locate texture descriptor for  aircraft
//-----------------------------------------------------------------------
CTextureDef *TCacheMGR::GetTexDescriptor()
{ return GetTexDescriptor(Spot.qgt,Spot.xDet(),Spot.zDet());}
//-----------------------------------------------------------------------
//  Locate texture descriptor for  tile (x,z)
//-----------------------------------------------------------------------
void TCacheMGR::FillGroundTile(CGroundTile *gnt)
{ U_INT ax = gnt->GetAX();
	U_INT az = gnt->GetAZ();
	U_INT qx = ax >> TC_BY32;
  U_INT qz = az >> TC_BY32;
  U_INT tx = ax &  TC_032MODULO;
  U_INT tz = az &  TC_032MODULO;
  C_QGT  *qgt = GetQGT(qx,qz);
	gnt->SetQGT(qgt);
  if (0 == qgt) gtfo("Timing problem with AP manager");
	//--- Reserve one reference to QGT ---------
	qgt->IncUser();
  //----Access the Super Tile ----------------
  CSuperTile   *sp  = qgt->GetSuperTile(tx,tz);
	gnt->StoreSup(sp);
  //----Local  tile number -------------------
  U_INT lx	= tx & TC_004MODULO;
  U_INT lz	= tz & TC_004MODULO;
  U_INT nt	= (lz << TC_BY04) | lx;
  CTextureDef *txn = &sp->Tex[nt];
	gnt->StoreData(txn);
  return;
}
//-----------------------------------------------------------------------
//  Locate texture descriptor for  tile (x,z) in qgt
//-----------------------------------------------------------------------
CTextureDef *TCacheMGR::GetTexDescriptor(C_QGT *qgt,U_INT tx,U_INT tz)
{ //----Access the Super Tile ----------------
  CSuperTile   *sp  = qgt->GetSuperTile(tx,tz);
  //----Local  tile number -------------------
  U_INT lx = tx & TC_004MODULO;
  U_INT lz = tz & TC_004MODULO;
  U_INT nt = (lz << TC_BY04) | lx;
  CTextureDef *txn = &sp->Tex[nt];
  return txn;
}
//-----------------------------------------------------------------
//	Check for terrain ready
//------------------------------------------------------------------
bool TCacheMGR::TerrainStable(char opt)
{ if (qgtMAP.size() == 0)			return false;
	std::map<U_INT,C_QGT*>::iterator rp;
	pthread_cond_signal(&thCond);    // Signal file THREAD
	if (ActQ.NotEmpty())			return false;
	for (rp = qgtMAP.begin(); rp != qgtMAP.end(); rp++)
	{	C_QGT *qt = (*rp).second;
	  if (qt->InLoad())						return false;
		if (qt->NotReady())					return false;
		if (qt->NotVisible())				continue;
		if (!qt->AllTextured(opt))	return false;
	}
	if (!MeshReady())							return false;
	if (!SPotReady())							return false;
	if (tr)		TRACE("TERRAIN IS READY");
	glFlush();
	glFinish();
	return true;
}
//-----------------------------------------------------------------
//  Create a New QGT in stack
//  NOTE:   a)The four corner vertices are searched in adjacent QGT
//            If not found they are created
//          b)The four corner vertices are created with special care
//            for vertices at coordinate X = 0
//          c)Then they are linked before the mesh is built
//            Each corner Vertex count is incremented in case the adjacent QGT
//            is deleted in the next cycle, to prevent vertex deletion 
//            before the detail mesh can be
//            built.  The use count is decremented after the mesh is built
//            because then, the vertex is used by the Tile Quad
//         
//-----------------------------------------------------------------------
void TCacheMGR::CreateQGT(U_SHORT cx, U_SHORT cz)
{ C_QGT *qgt  = new C_QGT(cx,cz,this);
  qgt->txw    = txw;
  ActQ.PutLast(qgt);
  //---Enter QGT in map --------------------------------
  qgtMAP[qgt->qKey] = qgt;
  qRDY++;               // One QGT waiting ready
  return;
}

//----------------------------------------------------------------------
//  Locate the adjacent QGT of the one indicated by cx and cz
//  cx  is the X index of the current QGT
//  cz  is the Z index of the current QGT
//  itm is the increment table giving direction from the current QGT
//----------------------------------------------------------------------
C_QGT *TCacheMGR::LocateQGT(QGT_DIR *itm,U_SHORT cx,U_SHORT cz)
{ U_SHORT kx  = (cx + itm->dx) & TC_0512MOD;
  U_SHORT kz  = (cz + itm->dz) & TC_0512MOD;
  U_INT key   = QGTKEY(kx,kz);
  std::map<U_INT,C_QGT*>::iterator qm = qgtMAP.find(key);
  if (qm == qgtMAP.end()) return 0;
  return (*qm).second;
}

//----------------------------------------------------------------------
//  Get a Quarter Global Tile by Key
//----------------------------------------------------------------------
C_QGT *TCacheMGR::GetQGT(U_SHORT qx, U_SHORT qz)
{ U_INT key   = QGTKEY(qx,qz);
  std::map<U_INT,C_QGT*>::iterator qm = qgtMAP.find(key);
  if (qm == qgtMAP.end()) return 0;
  return (*qm).second;
}

//----------------------------------------------------------------------
//  Get a Quarter Global Tile by position
//----------------------------------------------------------------------
C_QGT *TCacheMGR::GetQGT(SPosition &pos)
{ U_INT cx,cz;
  IndicesInQGT (pos, cx, cz);
  return GetQGT(cx,cz);
}
//----------------------------------------------------------------------
//  Get a Quarter Global Tile by Key
//----------------------------------------------------------------------
C_QGT *TCacheMGR::GetQGT(U_INT key)
{ std::map<U_INT,C_QGT*>::iterator qm = qgtMAP.find(key);
  if (qm == qgtMAP.end()) return 0;
  return (*qm).second;
}
//----------------------------------------------------------------------
//  Get a Quarter Global Tile by Key
//----------------------------------------------------------------------
bool TCacheMGR::SetQGT(GroundSpot &gns)
{ U_INT key   = QGTKEY(gns.qx,gns.qz);
  std::map<U_INT,C_QGT*>::iterator qm = qgtMAP.find(key);
  gns.qgt = (qm == qgtMAP.end())?(0):((*qm).second);
  return ((gns.qgt != 0) && (gns.qgt->HasQuad()));
}
//----------------------------------------------------------------------
//  Locate or create Vertex corner of surrounding QGT
//  tab is the corner table
//----------------------------------------------------------------------
CVertex *TCacheMGR::GetQgtCorner(C_QGT *qgt,QGT_DIR *tab,U_SHORT cn)
{ U_INT cx      = qgt->xKey;
  U_INT cz      = qgt->zKey;
  C_QGT *qn     = LocateQGT(tab,cx,cz);
  if (qn) {CVertex *vt = qn->Corner[tab->cn]; if (vt) return vt;}
  tab++;
  qn            = LocateQGT(tab,cx,cz);
  if (qn) {CVertex *vt = qn->Corner[tab->cn]; if (vt) return vt;}
  tab++;
  qn            = LocateQGT(tab,cx,cz);
  if (qn) {CVertex *vt = qn->Corner[tab->cn]; if (vt) return vt;}
  //-------Create the corner Vertex -------------------
  U_INT kx  = ((cx + qgtCORNER[cn].dx) & TC_0512MOD) << TCBY32768;
  U_INT kz  = ((cz + qgtCORNER[cn].dz) & TC_0512MOD) << TCBY32768;
  CVertex *vt = qgt->CreateVertex(kx,kz);
  return vt;
}
//----------------------------------------------------------------------
//  Check for QTR file in MAP
//-----------------------------------------------------------------------
C_QTR *TCacheMGR::GetQTR(U_INT key)
{   std::map<U_INT,C_QTR*>::iterator itr = qtrMAP.find(key);
    return (itr != qtrMAP.end())?((*itr).second):(0);
}
//----------------------------------------------------------------------
//  Enter QTR in cache
//----------------------------------------------------------------------
void  TCacheMGR::AddQTR(C_QTR *qtf,U_INT key)
{ qtrMAP[key] = qtf;
  NbQTR++;
  return;
}
//---------------------------------------------------------------------
//  Decrement QTR user count and dereference QTR when no more user
//  This call is made from a QGT when being deleted.  The last QGT
//  using the QTR and being deleted will free the QTR file
//  NOTE: QTR is deleted on behalf of the QGT for performance reason
//---------------------------------------------------------------------
bool TCacheMGR::DecQTR(C_QTR *qtr)
{ LockQTR();
  bool used     = true;
  U_INT key     = qtr->GetKey();
  if (qtr->NoMoreUsed())
      { qtrMAP.erase(key);                        // Remove file entry
        NbQTR--;
        used    = false;
      }
  UnLockQTR();
  return used;
}
//----------------------------------------------------------------------
//  Check for Coast file in MAP
//-----------------------------------------------------------------------
C_SEA *TCacheMGR::GetSEA(U_INT key)
{ LockSEA();  
  std::map<U_INT,C_SEA*>::iterator itr = seaMAP.find(key);
  C_SEA *sea = (itr != seaMAP.end())?((*itr).second):(0);
  UnLockSEA();
  return sea;
}
//----------------------------------------------------------------------
//  Enter COAST file in cache
//----------------------------------------------------------------------
void  TCacheMGR::AddSea(C_SEA *sea,U_INT ind)
{ LockSEA(); 
  seaMAP[ind] = sea;
  NbSEA++;
  UnLockSEA();
  return;
}
//---------------------------------------------------------------------
//  Decrement Coast file user count and delete SEA when no more user
//---------------------------------------------------------------------
void TCacheMGR::DecSEA(U_INT key)
{ int    ind = GetCoastIndex(key);
  C_SEA *sea = GetSEA(ind);
  if (0 == sea)   return;
	//--- Delete the data when no more used ----------
  LockSEA();
  if (sea->NoMoreUsed())
      { delete sea;                               // Delete file object
        seaMAP.erase(ind);                        // Remove file entry
        NbSEA--;
      }
  UnLockSEA();
  return;
}
//----------------------------------------------------------------------
//  Free the coast file from this QGT
//----------------------------------------------------------------------
void TCacheMGR::FreeSEA(C_QGT *qgt)
{ DecSEA(qgt->GetSeaKEY(0));              // A KEY
  DecSEA(qgt->GetSeaKEY(1));              // A KEY
  DecSEA(qgt->GetSeaKEY(2));              // A KEY
  DecSEA(qgt->GetSeaKEY(3));              // A KEY
  return;
}
//----------------------------------------------------------------------
//  Return Coast Data
//----------------------------------------------------------------------
char *TCacheMGR::GetSEAdata(U_INT ax, U_INT az)
{ U_INT gx   = (ax >> TC_BY32) >> 1;      // Globe tile X indice 
  U_INT gz   = (az >> TC_BY32) >> 1;      // Globe tile Z indice
  U_INT ind  = (gx << 16) | gz;
  C_SEA *sea = GetSEA(ind);
  return (sea)?(sea->GetCoast(ax,az)):(0);
}
//----------------------------------------------------------------------
//  Check for a CST object in map
//----------------------------------------------------------------------
C_CDT *TCacheMGR::GetCST(U_INT key)
{ LockSEA();
  std::map<U_INT,C_CDT*>::iterator itr = cstMAP.find(key);
  C_CDT *cst = (itr != cstMAP.end())?((*itr).second):(0);
  UnLockSEA();
  return cst;
}
//----------------------------------------------------------------------
//  Add a new CST object in map
//----------------------------------------------------------------------
void TCacheMGR::AddCST(U_INT key,C_CDT *cst)
{ LockSEA(); 
  cstMAP[key] = cst;
  UnLockSEA();
}
//----------------------------------------------------------------------
//  Return coast data for Detail tile key
//----------------------------------------------------------------------
char *TCacheMGR::GetCDTdata(U_INT key)
{ U_INT gx = ((key >> 16) >> 5);                  // get QGT X index
  U_INT gz = (key & 0x0000FFFF) >> 5;             // get QGT Z index
  U_INT gk = (gx << 16) | gz;
  C_CDT *cst = GetCST(gk);
  return (cst)?(cst->GetCoast(key)):(0);
}
//----------------------------------------------------------------------
//  Free all CST referenced by this QGT
//----------------------------------------------------------------------
void TCacheMGR::FreeCST(C_QGT *qgt)
{ 
  DecCST(qgt->GetSeaKEY(0));
  DecCST(qgt->GetSeaKEY(1));
  DecCST(qgt->GetSeaKEY(2));
  DecCST(qgt->GetSeaKEY(3));
  return;
}
//---------------------------------------------------------------------
//  Decrement Coast object user count and delete when no more user
//---------------------------------------------------------------------
void TCacheMGR::DecCST(U_INT key)
{ C_CDT *cst = GetCST(key);
  if (0 == cst)   return;
  LockSEA();
  if (cst->NoMoreUsed())
      { delete cst;                               // Delete file object
        cstMAP.erase(key);                        // Remove file entry
      }
  UnLockSEA();
  return;
}
//---------------------------------------------------------------------------------
//  Check for a coast file or load it and reserve it
//  key is QGT (X-Z) key
//  This function is called from the file thread to load a GTP pod file
//---------------------------------------------------------------------------------
void TCacheMGR::CheckCoastFile(U_INT key)
{ char name[MAX_PATH];
  U_INT  ind = GetCoastIndex(key);
  C_SEA *sea = GetSEA(ind);
  if (sea)  {sea->IncUser();          return;}
  //----Check for file existence  -----------------------
  U_INT  cx = ind >> 16;                // GLOBE column X
  U_INT  cz = ind & (255);              // GLOBE    row Z
  _snprintf(name,(MAX_PATH-1),"COAST/V%03d%03d.GTP",cx,cz);
  if (!pexists (&globals->pfs, name)) return;
  //-----Load the file -----------------------------------
  sea   = new C_SEA(ind,this);
  sea->IncUser();
  //-----Enter in cache with the file indice -----------------
  AddSea(sea,ind);
  return;
}
//---------------------------------------------------------------------------------
//  Load a GTP file (coast data) from POD
//  This function is also called from File Thread
//  ind is the file key
//---------------------------------------------------------------------------------
void TCacheMGR::LoadCoastPOD(U_INT ind)
{ if (ind == 0xFFFFFFFF)              return;
  char name[MAX_PATH];
  //----Check if already loaded -------------------------
  C_SEA *sea = GetSEA(ind);
  if (sea)  {sea->IncUser();          return;}
  //----Check for file existence  -----------------------
  U_INT  cx = ind >> 16;                // GLOBE column X
  U_INT  cz = ind & (255);              // GLOBE    row Z
  _snprintf(name,(MAX_PATH-1),"COAST/V%03d%03d.GTP",cx,cz);
  if (!pexists (&globals->pfs, name)) return;
  //-----Load the file -----------------------------------
  sea   = new C_SEA(ind,this);
  sea->IncUser();
  //-----Enter in cache with the file indice -----------------
  AddSea(sea,ind);
  return;
}
//---------------------------------------------------------------------------------
//  Load  coast data from SQL data base
//  This function is Called from File Thread
//  kxz is the QGT tile key
//---------------------------------------------------------------------------------
void TCacheMGR::LoadCoastSQL(U_INT kxz)
{ //---Check if already loaded ---------------------------
  C_CDT *cst = GetCST(kxz);
  if (cst)  {cst->IncUser(); return;}
  //----Load it from database -----------------------------
  cst = new C_CDT(kxz,this);
  COAST_REC rec;
  rec.qtk = kxz;
  globals->sql->ReadCoast(rec,cst);
  AddCST(kxz,cst);
  return;
}
//---------------------------------------------------------------------------------
//  Load All coast file from POD (Called from File thread)
//---------------------------------------------------------------------------------
void TCacheMGR::AllSeaPOD(C_QGT *qgt)
{ U_INT key = 0;
  for (int k=0; k<4; k++)
  { key = qgt->GetSeaREQ(k);
    LoadCoastPOD(key);
  }
  return;
}
//---------------------------------------------------------------------------------
//  Load All coast file from SQL (called from file thread)
//---------------------------------------------------------------------------------
void TCacheMGR::AllSeaSQL(C_QGT *qgt)
{ U_INT key = 0;
  for (int k=0; k<4; k++)
  { key = qgt->GetSeaREQ(k);
    LoadCoastSQL(key);
  }
  return;
}
//====================================================================================
//  Check for EPD file for this QGT
//====================================================================================

//---------------------------------------------------------------------
//  Read and decode EPD file
//---------------------------------------------------------------------
int TCacheMGR::TryEPD(C_QGT *qgt,char *name)
{ char *dot = strstr(name,".");
  strcpy(dot,".TEX");
  if (!pexists(&globals->pfs,name)) return 0;
  //----Create and decode the Texture and REF files ---------
  C_CTEX    ftx(qgt,tr);
  ftx.DecodeTEX(name);
  //----Decode the REF file ----------------------------------
  strcpy(dot,".REF");
  ftx.DecodeREF(name);
  return 0;
}
//---------------------------------------------------------------------
//  ABORT for MESH ERROR
//---------------------------------------------------------------------
void TCacheMGR::MeshError(U_LONG xv,U_LONG zv)
{ U_INT qcx = (xv >> TCBY32768);
  U_INT qcz = (zv >> TCBY32768);
  U_INT dcx = (xv >> TC_BY1024) & TC_032MODULO;
  U_INT dcz = (zv >> TC_BY1024) & TC_032MODULO;
  U_INT vcx = (xv & TC_1024MOD);
  U_INT vcz = (zv & TC_1024MOD);
  gtfo ("Tile mesh error: QGT %d-%d DT %d%d VT %d %d",qcx,qcz,dcx,dcz,vcx,vcz);
  return;
}

//--------------------------------------------------------------------------
//  Return QTR index from Quarter Globe Tile indices
//--------------------------------------------------------------------------
U_INT TCacheMGR::GetQTRindex(U_INT cx,U_INT cz)        
{ U_INT qtr_x = cx >> 4;                              // Divide by  16
  U_INT qtr_z = 31 - (cz >> 4);                       // Divide and invert
  U_INT indx  = (qtr_z << 5) + qtr_x;                 // Final index
  return indx;
}
//---------------------------------------------------------------------
//  Init:  Execute all actions at start-up to build initilial mesh
//         Load Textures for Near Super Tiles
//  NOTE: This routine runs at init time and may be interrupted by
//        the file THREAD. Be carefull
//---------------------------------------------------------------------
int TCacheMGR::InitMesh()
{// Init = 1;                                           // Lock the door
  while (OneAction()) continue;                       // Execute all
  //-----------TRACE THE OVERALL STATISTICS -------------------------------------
  if (!tr)  return 0;
  TRACE("TCM: -- Time: %04.2f---- END INIT Actual mesh QGT=%03d QTR=%d COAST=%d Vertex=%06d ---------",
          dTime,qgtMAP.size(),NbQTR,NbSEA,globals->NbVTX);

  return 0;
}
//---------------------------------------------------------------------
//  Last Action
//  Edit TRACE Statistiques 
//---------------------------------------------------------------------
int TCacheMGR::LastAction()
{ C_QGT *qgt  = ActQ.Pop();
  qgt->SetStep(TC_QT_RDY);
  if (ActQ.NotEmpty())  return 1;
  if(!tr)               return 1;
  TRACE("TCM: -- Time: %04.2f---- Actual mesh QGT=%03d QTR=%d COAST=%d Vertex=%06d ----------",
          dTime,qgtMAP.size(),NbQTR,NbSEA,globals->NbVTX);
  return 1;
}
//---------------------------------------------------------------------
//  Action:  Get current QGT and execute action
//  This list is built when the cache must load new Tiles
//  The whole process is divided in steps executed at each
//  time slice to spread the load.
//---------------------------------------------------------------------
int TCacheMGR::OneAction()
{ C_QGT *qgt = ActQ.Pop();        // Current QGT
	int pm = 0;
  if (0 == qgt)   return 0;
  ActQ.PutHead(qgt);
  //-----Dispatch action code -----------------------------
  switch (qgt->Step)  {
    case TC_QT_INI:
			return qgt->StepINI();
    //---- Build Detail Tile mesh -------------------------
    case TC_QT_MSH:
      return qgt->StepMSH();
    //---- Check for TRN ----------------------------------
    case TC_QT_ELV:
			return qgt->StepELV();
    //-----Get elevations from QTR ------------------------
    case TC_QT_QTR:
			return qgt->StepQTR();
    //----Get elevations from TRN file --------------------
    case TC_QT_TRN:
      return qgt->StepTRN();
    //--- TRN elevations from <hdtl> statements -----------
    case TC_QT_HDT:
      return qgt->StepHDT();
		//--- Detailled elevation in database -----------------
		case TC_QT_TIL:
		  return qgt->StepTIL();
		//--- Check elevation patche --------------------------
		case TC_QT_PCH:
			return qgt->StepPCH();
    //----Get Coast Data ----------------------------------
    case TC_QT_SEA:
      return qgt->StepSEA();
    //--- Finalize the Super Tile parameters --------------
    case TC_QT_SUP:
			return qgt->StepSUP();
    //---Locate 3D objects per QGT ------------------------
    case TC_QT_3DO:
			pm = qgt->Step3DO();
			//----------------------------------------------------
      if (tr) TRACE("TCM: -- Time: %04.2fQGT(%3d-%3d) Load %06d Objects",Time(),qgt->xKey,qgt->zKey,pm);
      //----------------------------------------------------
			return  1;
      //return LastAction();
		//--- Load OSM layer 1 ---------------------------------
		case TC_QT_OS1:
			if (qgt->StepOSM())	return 1;
			return LastAction();
    //---- Delete the QGT --------------------------------
    case TC_QT_DEL:
      return FreeTheQGT(qgt);
    //----- Waiting for file -----------------------------
    case TC_QT_WFF:
      return 1;
  }
  //------No Actions- QUEUE ERROR !!!! --------------
  Abort("NO ACTION");
  return 0;
}
//---------------------------------------------------------------------
//  Delete the QGT
//  Reset the global QGT iterator to the head of map
//  NOTE: Must return 1
//---------------------------------------------------------------------
int TCacheMGR::FreeTheQGT(C_QGT *qgt)
{ int qx = qgt->GetXkey();
	int qz = qgt->GetZkey();
  //---Free all textures -----------------------------
  qgt->FlushTextures();  
  //---Delete QTR file -------------------------------
  qgt->DeleteQTR();
  //---Remove from Queue, clean and delete -----------
  U_INT key = QGTKEY(qgt->xKey,qgt->zKey);
  qgtMAP.erase(key);
  iqt   = qgtMAP.begin();
  ActQ.Pop();
  qgt->DecUser();
  if (ActQ.NotEmpty())  return 1;
  if(tr) 
	{ TRACE("TCM: -- Time: %04.2f---- Actual mesh QGT=%03d QTR=%d COAST=%d Vertex=%06d ------",
          dTime,qgtMAP.size(),NbQTR,NbSEA,globals->NbVTX);
	}
  return 1;
}
//---------------------------------------------------------------------------------
//  Update time and position dependant parameters
//---------------------------------------------------------------------------------
void TCacheMGR::UpdateTOD()
{ hLine  = SquareRootFloat(2 * TC_EARTH_RADIUS * aPos.alt);   // Aircraft Distance to horizon
  GetSunPosition();                                           // Set sun position
  cTod = SetLuminosity();
  globals->tod = cTod;
  txw->SetNight(cTod);
  if    ( TgtColor  >  NitColor[0])
        { NitColor[0] += 0.01f;
          NitColor[1] += 0.01f;
          NitColor[2] += 0.01f;
        }
  else  { NitColor[0] -= 0.01f;  
          NitColor[1] -= 0.01f;
          NitColor[2] -= 0.01f;
        }
  return;
}
//---------------------------------------------------------------------------------
//  Get the sky luminosity
//  Compute emissive color
//---------------------------------------------------------------------------------
#define DAY_LUM (float(0.72f))
U_CHAR TCacheMGR::SetLuminosity()
{ 
  float R = sunC[0] * 0.38f;
  float G = sunC[1] * 0.74f;
  float B = sunC[2] * 0.16f;

  float L = (R + G + B);
  float N = (L > DAY_LUM)?(0):(1.0f - L);
  lumn  = L;
  //------Modify Night emissive luminosity -------------------------
  TgtColor = N;
  return (L > DAY_LUM)?('D'):('N');
}
//---------------------------------------------------------------------------------
//  Return in vector the feet component from aircraft to position, 
//  taking in account the correction factor for X longitude
//  Position must be in absolute arcsec (not relative to longitude band)
//---------------------------------------------------------------------------------
void TCacheMGR::RelativeFeetTo(SPosition &pos,SVector &v)
{ double dx = LongitudeDifference(pos.lon,aPos.lon);
  //------ tx and ty in feet ------------------------------
  v.x = FN_FEET_FROM_ARCS(dx) * rFactor;
  v.y = FN_FEET_FROM_ARCS(pos.lat - aPos.lat);
  v.z = pos.alt - aPos.alt;
  return;
}
//---------------------------------------------------------------------------------
//  Scan QGT to assign a given metar area.
//  This routine is called from Weather manager when a new METAR is detected
//  from the cache
//---------------------------------------------------------------------------------
void TCacheMGR::AssignMetar(CMeteoArea *ma)
{ std::map<U_INT,C_QGT*>::iterator im;
  for (im = qgtMAP.begin(); im != qgtMAP.end(); im++)
  { C_QGT *qgt = (*im).second;
    if (qgt->NoQuad())    continue;
    if (qgt->GetMETAR())  continue;
    int in = ma->RangeOf(*qgt->GetMidPoint());
    if (0 == in)          continue;
    //---Link the QGT to the METAR area ------------------
    qgt->SetMETAR(ma->GetKey());
    if (qgt == Spot.qgt)  globals->wtm->ChangeMetar(ma);
    if (tr) TRACE("WTM: -- QGT(%3d-%3d) get METAR %s",qgt->xKey,qgt->zKey,ma->GetIdent());
  }
  return;
}
//---------------------------------------------------------------------------------
//  Scan all QGT that are in ready state
//  Compute the distance in nautical mile of the SW corner to the aircraft
//  Convert the distance in pixel coordinates based on the given scale
//  Use coast object to draw the coast on the surface
//  Scale is the number of pixels per miles on the given surface
//  NOTE: This routine is executed on behalf the VectorMap.   
//---------------------------------------------------------------------------------
//  Scan 3D object
//---------------------------------------------------------------------------------
void TCacheMGR::CheckW3D()
{ std::map<U_INT,C_QGT*>::iterator it;
  for (it = qgtMAP.begin(); it != qgtMAP.end(); it++)
  { C_QGT *qgt = (*it).second;
    if (qgt->IsReady()) qgt->CheckW3D();
  }
  return;
}
//---------------------------------------------------------------------------------
//  Scan coast
//---------------------------------------------------------------------------------
void TCacheMGR::ScanCoast(SSurface *sf)
{ if (globals->seaDB) return ScanCoastSQL(sf);
 //---  Scan POD files for coast data
  std::map<U_INT,C_SEA*>::iterator im;
  LockSEA();
  for (im = seaMAP.begin(); im != seaMAP.end(); im++)
  { C_SEA *sea = (*im).second;
    sea->DrawCoastLine(sf);
  }
  UnLockSEA();
  return;
}
//---------------------------------------------------------------------------------
//  Scan coast QL files
//---------------------------------------------------------------------------------
void TCacheMGR::ScanCoastSQL(SSurface *sf)
{ std::map<U_INT,C_CDT*>::iterator im;
  LockSEA();
  for (im = cstMAP.begin(); im != cstMAP.end(); im++)
  { C_CDT *cst = (*im).second;
    cst->DrawCoastLine(sf);
  }
  UnLockSEA();
  return;
}
//-----------------------------------------------------------------------------
// Name: UpdatePlane ()
// We compute here a plane equation that represents the ground
//		relative to the aircraft position (at origin (0,0,alt);
//	Dta is the relative altitude of ground: (ground - aircraft)altitude
//-----------------------------------------------------------------------------
void TCacheMGR::UpdateGroundPlane()
{	float dta	=		GetGroundAltitude() -  aPos.alt;
	gplan[0]	=   0;
  gplan[1]	=   0;
  gplan[2]	=   1;
  gplan[3]	= -(dta);
	return;
}
//-----------------------------------------------------------------------------
// Draw all 3D objects if allowed
//	Camera is at aircraft origin, scale is 1 in every direction
//-----------------------------------------------------------------------------
void TCacheMGR::Draw3DObjects()
{	std::map<U_INT,C_QGT*>::iterator im;
	glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER,0.4f);
 // glEnable(GL_BLEND);
	glDisable(GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glShadeModel(GL_SMOOTH);
	glCullFace (GL_BACK);
	//-------------------------------------------------------------------
  objMGR->SetDrawingState();
	glEnable(GL_TEXTURE_2D);
  for (im = qgtMAP.begin(); im != qgtMAP.end(); im++)
  { C_QGT *qt = (*im).second;
    if (qt->NoQuad())       continue;
    if (qt->NotVisible())   continue;
    qt->w3D.Draw(cTod);                         // Draw for day or night
  }
	return;
}

//-----------------------------------------------------------------------------
//	Locate 3D pack
//-----------------------------------------------------------------------------
void TCacheMGR::AddToPack(OSM_Object *obj)
{ C_QGT *qgt			= GetQGT(obj->GetKey());
	if (0 == qgt)		return;
	CSuperTile *sup	= qgt->GetSuperTile(obj->GetSupNo());
	sup->AddToPack(obj);
	return;
}
//=================================================================================
//  TERRAIN CACHE: DRAW TERRAIN
//  Compute the top model matrix.  X,Y, and Z are OpenGL axis coordinates
//  Transformations are (in that order)
//  1) T1: Translation to the aircraft position
//  2) S1: Scale X and Z by factor FEET-PER-ARCSEC to transform latitude and 
//     longitude into feet. 
//  3) S2: Scale X coordinate by cos(lat) to compensate for the Tile size
//  Orient is the camera azimut (horizontal orientation)
//  NOTE:  For correct rendering, objects must be rendered in the following order
//         a) Airport component (ground, runway, buildings)
//         b) All other terrain tiles
//         c) All lighting
//  NOTE:  Texture Unit-0 is supposed to enable permanently TEXTURE_2D except when
//         this is contrary to the objective. This is to avoid extensive GL commands
//=================================================================================
void TCacheMGR::Draw()
{	CCamera *cam = globals->cam;
	//--- Draw sky as background --------------------------------------
	globals->skm->PreDraw();
	CVehicleObject *veh = globals->pln;
	DrQGT	= DrSUP = 0;
	//----Prepare OpenGL for drawing ---------------------------------
  glMatrixMode (GL_MODELVIEW);
  glPushAttrib (mask);
	glDepthRange(0.00001,100);
  //-------- Extract parameters from camera -------------------------
  orient = cam->GetAzimuth();
  cCam   = cam;
  aptMGR->SetCamera(cCam);
  //------Set Globale options ---------------------------------------
  glPolygonMode (GL_FRONT, WireOPT[wire]);    // Fill for texture
  glEnable(GL_POLYGON_SMOOTH);
  //-------Lighting parameters -------------------------------------
  glEnable(GL_LIGHTING);                      // LIGHT
  //---------Activate fog --------------------------------------------
  //  Fog is taken from sky dome, face to camera at about 45°  of zenith
  //------------------------------------------------------------------
  globals->skm->GetSkyDomeColour (float(PI * 0.52), -orient,fogC[0],fogC[1],fogC[2]);
  DrawSun();
  SetSkyFog(fDens);
  //------------------------------------------------------------------
  DrawHorizon();                              // Horizon
  glPushMatrix();                             // Mark T0:  Camera at origin
  //------------------------------------------------------------------
  //  Airport tarmac
  //  Camera at origin
  //  Scale is transforming arcsec in feet. 
  //  Scale in X is different from scale  in Y
  //  Longitude Feet per Arcsec is changing with latitude while
  //  Latitude uses a steady scale.
  //------------------------------------------------------------------
  glScaled(scale.x,scale.y, 1.0);             // T1: Scale X,Y to feet coordinate
  aptMGR->Draw(aPos);													// Draw airports
  //------------------------------------------------------------------
  //  Draw Terrain
  //  Camera at aircraft position
  //------------------------------------------------------------------
  glPushClientAttrib (GL_CLIENT_ALL_ATTRIB_BITS);
  //------------------------------------------------------------------
	//	Draw terrain: check for QGT visibility
	//	Otherwise
	//	For each QGT go to SW corner and render the QGT mesh
	//------------------------------------------------------------------
  glEnable(GL_TEXTURE_2D);
  glColor4fv(fogC);                           // restore color
  glMaterialfv (GL_FRONT, GL_EMISSION, GetDeftEmission());
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisable(GL_BLEND);
  U_CHAR vis = 0;
  std::map<U_INT,C_QGT*>::iterator im;
	//--- Draw all visible QGTs -------------------------
  for (im = qgtMAP.begin(); im != qgtMAP.end(); im++)
  { C_QGT *qt = (*im).second;
    if (qt->NoQuad())		continue;
		//--- Check for visibility ------------------------
		bool v1	= (qt->qKey == rKEY);						// Aircraft QGT
		bool v2	= v1 | cam->BoxInFrustum(qt->mPoint,qt->Bound, 1.4);
		qt->visb		= v2;
		if (!v2)						continue;
		//--- Draw the QGT --------------------------------
		DrQGT++;
		glPushMatrix();		// Mark T2
		SPosition	sw = qt->GetBase();
		SVector  vtr = SubtractPositionInArcs(aPos,sw);
		glTranslated(vtr.x, vtr.y, vtr.z);						// Go to SW corner at 0 feet AGL
		qt->DrawSuperMesh(cam,v1);
		glPopMatrix();                            // Back to T2
  }
	//--- Recover attributes ---------------------------
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glPopMatrix();                              // Back to T0 (camera at origin)
  //------------------------------------------------------------------
  //  Draw all 3D objects 
	//	Camera is at aircraft origin, scale is 1 in every direction
  //------------------------------------------------------------------
  glPushMatrix();                             // Mark T0
	Draw3DObjects();
  //---Reset all parameters ----------------------------------------
  glPopClientAttrib();
  glFrontFace(GL_CCW);
  glDisable(GL_ALPHA_TEST);
  glEnable(GL_BLEND);
  //-------------------------------------------------------------
  // Draw the translucent elements (clouds etc.)
  //  Camera at origin
  //-------------------------------------------------------------
  globals->cld->Draw();
  //-------------------------------------------------------------
  // Draw all external vehicle features
  // Camera at origin
  //-------------------------------------------------------------
  globals->sit->DrawExternal();
  //----------------------------------------------------------------
  // Draw VOR
  //----------------------------------------------------------------
  glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  objMGR->Draw(cTod);                           // Draw VOR and lights
  //-----------------------------------------------------------------
  //  Set Context to draw airport lights 
  //  Camera at origin
  //-----------------------------------------------------------------
  glDepthMask(false);
  glEnable (GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER,0);
  aptMGR->DrawLights();                         // Draw airports lights
	//---- Vehicle outside dot lights -----------------------------------
  if (veh) veh->DrawOutsideLights();
  glDisable(GL_ALPHA_TEST);
  glDepthMask(true);
  //-----------------------------------------------------------------
  glPopClientAttrib();
  glPopMatrix();                                // Back to T0 (No transformations)
  //-----------------------------------------------------------------
  //  Draw vehicle inside
  //-----------------------------------------------------------------
  glMaterialfv (GL_FRONT, GL_EMISSION, GetDeftEmission());
  glColor4f(1,1,1,1);
  if (veh) veh->DrawInside(cam);

  //-----------------------------------------------------------------
  //  Draw tracker
  //-----------------------------------------------------------------
	globals->etrk.Draw();
  //--------Restore attributs ---------------------------------------
  glPopAttrib ();
  glMatrixMode (GL_MODELVIEW);
  //--------Area allowed for no scene drawing ------------------------
  globals->sit->DrawVehicleFeatures();
  //----------------------- end --------------------------------------
  //============ Check for an OpenGL error==============================
  { GLenum e = glGetError ();
		if (e != GL_NO_ERROR) 
			WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  }
	//====================================================================
  return;
}
//---------------------------------------------------------------------------------
//  Checkfor plane in tile.  For test purpose only
//---------------------------------------------------------------------------------
bool TCacheMGR::PlaneInTile(U_INT ax,U_INT az)
{ U_INT qx = ax >> TC_BY32;
  U_INT qz = az >> TC_BY32;
  int rx = (ax & TC_032MODULO);
  int rz = (az & TC_032MODULO);
  short tx,tz;
  if ((qx != xKey) || (qz != zKey)) return false;
  C_QGT *qt = GetQGT(xKey,zKey);
  qt->GetTileIndices(aPos,tx,tz);
  if  (tx != rx)   return false;
  if  (tz != rz)   return false;
  return true;
}
//---------------------------------------------------------------------------------
//  Set Fog for Sky
//---------------------------------------------------------------------------------
void TCacheMGR::SetSkyFog(float dens)
{ glHint (GL_FOG_HINT, GL_NICEST);
  glFogi (GL_FOG_MODE, GL_EXP2);
  glFogf (GL_FOG_DENSITY,dens);
  glFogfv(GL_FOG_COLOR,fogC);
  glEnable (GL_FOG);
  return;
}
//---------------------------------------------------------------------------------
//  Draw the horizon:  Horizon is drawn as a dsik
//  with the fog color.  This is to confer a roundish aspect to the far line
//  of sight
//---------------------------------------------------------------------------------
void TCacheMGR::DrawHorizon()
{ //----Horizon parameters -------------------------------
  double in = (medRAD > hLine)?(hLine):(medRAD);
  glPushMatrix();
  glTranslated (0,0,-aPos.alt);
  gluDisk(ground,in,hLine,128,1);
  glPopMatrix();
  return;
}
//-----------------------------------------------------------------------------
//  Draw Sun ; Just for test purpose
//-----------------------------------------------------------------------------
void TCacheMGR::DrawSun (void)
{ //--Temporary draw sun ---------------------------------
  float col[4] = {1,1,1,1};
  sInd  = 'N';
  if (sunP.z < -30000) return;
  //--- Draw the sun -------------------------------------
  glPushMatrix();
  glMaterialfv (GL_FRONT, GL_EMISSION, col);
  glTranslatef (sunP.x,sunP.y,sunP.z);
  gluSphere(sphere,sunR,32,32);
  glPopMatrix();
  glMaterialfv (GL_FRONT, GL_EMISSION, EmsColor);
  sInd  = 'S';
  return;
}
//-----------------------------------------------------------------------------
//  Compute Sun position and Color
//-----------------------------------------------------------------------------
void TCacheMGR::GetSunPosition (void)
{ double el = CSkyManager::Instance().GetSolElevation();
  double az = CSkyManager::Instance().GetSolOrientation();
  double ds = FN_FEET_FROM_MILE(210);
  sunP.x    = ds * sin(az);                     // World x coordinate
  sunP.y    = ds * cos(az);                     // World y coordinate
  sunP.z    = ds * cos(el);                     // world z coordinate
  globals->skm->GetSkyDomeColour (el, az, sunC[0],sunC[1],sunC[2]);
  sunR      = ds * sunT * 6;
  return;
}
//-----------------------------------------------------------------------------
//  Build Shadow map
//-----------------------------------------------------------------------------
#define TC_SHADOW (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT)
void TCacheMGR::BuildShadowMap (void)
{ int dp;                                             // depth value
  GLuint sObj;                                        // Shadow texture object
  glGetIntegerv(GL_DEPTH_BITS,&dp);
  //----save attributs ----------------------------------------------
  glPushAttrib(TC_SHADOW);
  //----Create texture map ------------------------------------------
  glGenTextures(1,&sObj);                       
  glBindTexture(GL_TEXTURE_2D,sObj);
  glTexImage2D (GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT,1024,1024,0,
                               GL_DEPTH_COMPONENT,GL_UNSIGNED_BYTE,NULL);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
  //----Create ligth projection matrix ------------------------------
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  gluPerspective(45.0f,1,1000,10000);
  glGetFloatv(GL_MODELVIEW_MATRIX,lp);
  //----Create light view matrix ------------------------------
  glLoadIdentity();
  gluLookAt(sunP.x,sunP.y,sunP.z,0,0,0,0,1,0);
  glGetFloatv(GL_MODELVIEW_MATRIX,lv);
  glPopMatrix();
  //------------------------------------------------------------------
  glColorMask(0,0,0,0);                   // Disable color
  //------TOTO DRAW SHADOW MAP HERE ----------------------------------
  //----Delete texture and restaure states ---------------------------
  glDeleteTextures(1,&sObj);
  glPopAttrib();
  return;
}
//====================================================================================
//  TCACHE FILE MANAGEMENT
//=====================================================================================
//---------------------------------------------------------------------------------
//  Enter a QGT into the texture loading Queue
//---------------------------------------------------------------------------------
void TCacheMGR::RequestTextureLoad(C_QGT *qgt)
{ qgt->nStp = TC_QT_RDY;
  qgt->SetStep(TC_QT_WFF);
  LodQ.PutLast(qgt);
  return;
}
//----------------------------------------------------------------------------
//  Request standard elevations
//----------------------------------------------------------------------------
int TCacheMGR::RequestELV(C_QGT *qgt)
{ //--- Search in SQL data base ----------------------------------
	U_CHAR end	= 0;								// QGT step after read
	qgt->qtr    = 0;
	if (globals->elvDB)
	{	end					= TC_QT_TIL;
		qgt->rCode  = TC_SQL_ELV;   // Request code
	}
	//--- Search the QTR files -----------------------------------
  else
	{	if (tr) TRACE("TCM: -- QGT(%03d-%03d) request QTR",qgt->xKey,qgt->zKey);
		end					= TC_QT_QTR;										// QGT step after read
		qgt->rCode  = TC_POD_QTR;														// Request code
		qgt->rKey   = GetQTRindex(qgt->xKey,qgt->zKey);     // QTR file Key
	}
	return StartIO(qgt,end);
}

//----------------------------------------------------------------------------
//  Start file thread IO
//----------------------------------------------------------------------------
int TCacheMGR::StartIO(C_QGT *qgt, U_CHAR ns)
{ qgt->nStp     = ns;                 // Next step
  qgt->SetStep(TC_QT_WFF);            // Wait for file
  ActQ.Pop();                         // Remove QGT from Action Queue
  FilQ.PutLast(qgt);                  // Put in file Queue
  pthread_cond_signal(&thCond);       // Signal file THREAD
  return 1;
}

//----------------------------------------------------------------------------
//  Probe for statistics
//----------------------------------------------------------------------------
void TCacheMGR::GetStats(CFuiCanva *cnv)
{ char txt[128];
  //-------------------------------------------------
	//int m = getTotalSystemMemory();
	cnv->AddText(1,"Total bitmap:");
	sprintf_s(txt,128,"% 8d",globals->NbBMP);
  cnv->AddText(STATS_NUM,txt,1);

  cnv->AddText(1,"TERA Vertices:");
  sprintf_s(txt,128,"% 8d",globals->NbVTX);
  cnv->AddText(STATS_NUM,txt,1);

  cnv->AddText(1,"QGTs:");
  sprintf_s(txt,128,"% 8d",qgtMAP.size());
  cnv->AddText(STATS_NUM,txt,1);

	cnv->AddText(1,"Coast slots:");
	sprintf_s(txt,128,"% 8d",cstMAP.size());
	cnv->AddText(STATS_NUM,txt,1);

	cnv->AddText(1,"Coast lines:");
	sprintf_s(txt,128,"% 8d",globals->NbCLN);
	cnv->AddText(STATS_NUM,txt,1);

	cnv->AddText(1,"QGT draw:");
	sprintf_s(txt,128,"% 8d",DrQGT);
	cnv->AddText(STATS_NUM,txt,1);

	cnv->AddText(1,"Sup draw:");
	sprintf_s(txt,128,"% 8d",DrSUP);
	cnv->AddText(STATS_NUM,txt,1);


  //------------------------------------------------
}

//==========END OF THIS FILE ======================================================