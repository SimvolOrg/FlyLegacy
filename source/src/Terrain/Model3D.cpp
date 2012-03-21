/*
 * Model3D.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2004 Chris Wallace
 * CopyRigth 2008 Jean Sabatier
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


#include "../Include/FlyLegacy.h"
#include "../Include/Globals.h"
#include "../Include/TerrainCache.h"
#include "../Include/TerrainTexture.h"
#include "../Include/Triangulator.h"
#include "../Include/Model3D.h"
#include "../Include/Endian.h"
#include "../Include/Weather.h"
#include "../Include/FileParser.h"
#include "../Include/Reductor.h"
#include "../Include/Export.h"
#include "../Include/FuiParts.h"
#include <string.h>
using namespace std;
//=======================================================================================
extern TC_COLOR ColorTAB[];
//======================================================================================
struct WIND_SPEED {
    float speed;
    Tag   idn;
};
//-----VOR LIGHT OFFSET ----------------------------------------------------------------
SPosition vorOFS = { -0.068,0,14.5};      // This is OK only for VOR.SMF model
//======================================================================================
//  Wind Speed table
//======================================================================================
WIND_SPEED windTAB[] = {
  { 0.5, 'sk00'},
  { 5.0, 'sk05'},
  {10.0, 'sk10'},
  {15.0, 'sk15'},
  {20.0, 'sk20'},
  {25.0, 'sk25'},
};
//--------------------------------------------------------------------------------------
float White[4] = {1,1,1,1};
//======================================================================================
//  Polygon 3D
//======================================================================================
CPoly3D::CPoly3D()
{ nbv = 0;
  vtb = 0;
  pmd = GL_TRIANGLES;
}
//----------------------------------------------------------------------
//  Build vertice table
//----------------------------------------------------------------------
CPoly3D::CPoly3D(int nv)
{ nbv = nv;
  pmd = GL_TRIANGLES;
  vtb = new GN_VTAB[nv];
}
//----------------------------------------------------------------------
//  Free the resources
//----------------------------------------------------------------------
CPoly3D::~CPoly3D()
{ if (vtb)  delete [] vtb;
}
//----------------------------------------------------------------------
//  Allocate vertex table
//----------------------------------------------------------------------
void CPoly3D::Allocate(int nv)
{ nbv = nv;
  vtb = new GN_VTAB[nv];
}
//----------------------------------------------------------------------
//  Draw vertices
//----------------------------------------------------------------------
void CPoly3D::Draw(U_INT txo)
{ glBindTexture(GL_TEXTURE_2D,txo);
  glInterleavedArrays(GL_T2F_N3F_V3F,0,vtb);
  glDrawArrays(pmd,0,nbv);
  return;
}
//----------------------------------------------------------------------
//  Return entry if OK
//----------------------------------------------------------------------
GN_VTAB *CPoly3D::GetVTAB(int k)
{ if (k >= nbv) gtfo("Bad request for vertex k");
  return vtb + k;}
//----------------------------------------------------------------------
//  Set Entry if OK
//----------------------------------------------------------------------
GN_VTAB *CPoly3D::SetVTAB(int k,GN_VTAB *v)
{ if (k >= nbv) gtfo("Bad request for vertex k");
  GN_VTAB *ent = vtb + k;
  *ent = *v; 
  return ent;} 
//======================================================================================
//  Polygon object
//======================================================================================
CPolygon::CPolygon()
{ nbv   = 0;
  vtb   = 0;
  mode  = GL_POLYGON;             // Default mode
}
//----------------------------------------------------------------------
//  destroy it
//----------------------------------------------------------------------
CPolygon::~CPolygon()
{ if (vtb)  delete [] vtb;
}
//----------------------------------------------------------------------
//  Allocate vertex table for n polygons
//----------------------------------------------------------------------
void CPolygon::SetPolygons(int nv)
{ nbv = nv;
  vtb = new TC_VTAB[nv];
  return;
}
//----------------------------------------------------------------------
//  Draw the polygon
//----------------------------------------------------------------------
void CPolygon::Draw()
{ glInterleavedArrays(GL_T2F_V3F,0,vtb);
  glDrawArrays(mode,0,nbv);
  return;
}
//======================================================================================
//  Rectangular polygon
//======================================================================================
CPolyREC::CPolyREC()
{ nbv = 4;
}
//---------------------------------------------------------------------
//  Init QUAD polygon from surface
//---------------------------------------------------------------------
void CPolyREC::InitQuad(CPanel *panel, SSurface *sf)
{ if (0 == sf)    return;
  //----Init polygon --------------------------------------------------
  int ht = panel->GetHeight();                    // Top OpenGL line
  int l0 = ht - sf->yScreen - sf->ySize;          // South line
  int l1 = ht - sf->yScreen - 1;
  //-----NE corner ----------------------------------------------------
  vtab[0].VT_S   = 1;
  vtab[0].VT_T   = 1;
  vtab[0].VT_X   = float(sf->xScreen + sf->xSize - 1);
  vtab[0].VT_Y   = float(l1);
  vtab[0].VT_Z   = 0;
  //-----NW corner ----------------------------------------------------
  vtab[1].VT_S   = 0;
  vtab[1].VT_T   = 1;
  vtab[1].VT_X   = float(sf->xScreen);
  vtab[1].VT_Y   = float(l1);
  vtab[1].VT_Z   = 0;
  //-----SW corner -----------------------------------------------------
  vtab[2].VT_S   = 0;
  vtab[2].VT_T   = 0;
  vtab[2].VT_X   = float(sf->xScreen);
  vtab[2].VT_Y   = float(l0);
  vtab[2].VT_Z   = 0;
  //-----SE corner ----------------------------------------------------
  vtab[3].VT_S   = 1;
  vtab[3].VT_T   = 0;
  vtab[3].VT_X   = float(sf->xScreen + sf->xSize - 1);
  vtab[3].VT_Y   = float(l0);
  vtab[3].VT_Z   = 0;
  return;
}

//---------------------------------------------------------------------------------
//  Draw the rectangle 
//---------------------------------------------------------------------------------
void CPolyREC::Draw(bool bld)
{ glInterleavedArrays(GL_T2F_V3F,0,vtab);
  glDrawArrays(GL_POLYGON,0,4);
  return;
  }
//---------------------------------------------------------------------------------
//  Draw the triangle strip 
//---------------------------------------------------------------------------------
void CPolyREC::DrawStrip()
{ glInterleavedArrays(GL_T2F_V3F,0,vtab);
  glDrawArrays(GL_TRIANGLE_STRIP,0,4);
  return;
  }

//======================================================================================
//  Picture QUAD provide a QUAD for a bitmap
//======================================================================================
CPicQUAD::CPicQUAD()
{ 
}
//-----------------------------------------------------------------------------------
//  Free all resources
//------------------------------------------------------------------------------------
CPicQUAD::~CPicQUAD()
{ glDeleteTextures(1,&xOBJ);
}
//------------------------------------------------------------------------------------
//  Load a Bitmap on surface
//------------------------------------------------------------------------------------
void CPicQUAD::GetBitmap(char *bn)
{ xOBJ = globals->txw->LoadIconPNG("PICON");
  //---------------------------------------------------
  float w2 = float(FN_ARCS_FROM_FEET(200));         // Half width in feet
  float h2 = float(FN_ARCS_FROM_FEET(200));         // half height in feet
  //---Allocate QUAD coordinates ----------------------
  TC_VTAB *tab = quad.GetVTAB();
  //--SW corner ---------------------------------------
  tab[0].VT_S = 0;
  tab[0].VT_T = 0;
  tab[0].VT_X = -float(w2);
  tab[0].VT_Y = -float(h2);
  tab[0].VT_Z = 0;
  //--SE corner ---------------------------------------
  tab[1].VT_S = 1;
  tab[1].VT_T = 0;
  tab[1].VT_X = +float(w2);
  tab[1].VT_Y = -float(h2);
  tab[1].VT_Z = 0;
  //--NE corner ---------------------------------------
  tab[2].VT_S = 1;
  tab[2].VT_T = 1;
  tab[2].VT_X = +float(w2);
  tab[2].VT_Y = +float(h2);
  tab[2].VT_Z = 0;
  //--NW corner ---------------------------------------
  tab[3].VT_S = 0;
  tab[3].VT_T = 1;
  tab[3].VT_X = -float(w2);
  tab[3].VT_Y = +float(h2);
  tab[3].VT_Z = 0;
  return;
}
//------------------------------------------------------------------------------------
//  Set elevation to value
//------------------------------------------------------------------------------------
void CPicQUAD::SetElevation(float e)
{ TC_VTAB *tab = quad.GetVTAB();
  tab[0].VT_Z = e;
  tab[1].VT_Z = e;
  tab[2].VT_Z = e;
  tab[3].VT_Z = e;
return;
}
//------------------------------------------------------------------------------------
//  Assign texture and Draw
//------------------------------------------------------------------------------------
void CPicQUAD::Draw()
{ glBindTexture(GL_TEXTURE_2D,xOBJ);
  quad.Draw(true);
  return;
}
//======================================================================================
//  3D objects cache management
//   3DMgr is a global entity managing 3D objects
//  -3DMgr locates scenery files for a given QGT  and launch decoding for objects (CWobj)
//    described in this scenery file.  
//  Additionaly, it manages a cache of references to the C3Dmodel.
//
//  A C3Dmodel encloses the parts components described by SMF or BIN file.  
//  A given model is reused many time in a scenery, and the model concept allows to shared 
//  the model between several objects.
//
//  When a CWobj is associated to a model (BIN or SMF) a Reference (3DRef) is simply 
//  added to the CWobj.  C3Ref is a pointer to the model.
//
//  Some CWobj may have several references (thus using different models) 
//  for animation. Type 'wdsk' for instance uses one BIN file for each frame of animation.
//  
//  Each CWobj is attached to a mother SuperTile where the object location resides
//  We thus benefit from 2 features used by the SuperTile during Drawing Terrain:
//    -The eye distance from aircraft to Super Tile center is already computed. So all
//     objects in a Super Tile has the same "eye Distance" as the Super Tile center to
//     avoid distance computaton at object level.
//
//    -A Visibility test has already be made.  If the Super Tile is not visible, then
//     all included objects are not visibles.
//
//                                QGT (Quarter Global tile)
//            _____________________|___________________
//           |                     |                  |
//    SuperTile01                  |             SuperTile31
//           |                     |                      |
//    CWobj1 ...CWobj42   <= SCENERY FILE =>  CWobj2 .... list of  CWobj
//      |                                       |
//      |             |                         |
//   C3Dmodel1      C3Dmodel22          One Model per Ref
//      |_____________|_________________________|
//                                  |
//                            MAP of Models managed by C3DMgr
//
//    During Drawing CWobj, any associated light found is queued in a deferred queue.
//    After drawing all objects the C3DMgr will Draw only one VOR object (the nearest)
//    and then all lights.  
//    Lights must be drawn in the last step as the textures are transparent.
//
//======================================================================================
C3DMgr::C3DMgr(TCacheMGR *m )
{	pthread_mutex_init (&mux,  NULL); 
	tcm			= m;
  int nb  = 0;                                // Trace option
  sphere  = gluNewQuadric();                  // Testing purpose
  GetIniVar("TRACE", "3DModel", &nb);
  tr  = (nb)?(1):(0);
  nb  = 0;
  //----Register in globals --------
  dbm = globals->dbc;
  globals->m3d = this;
  //----ADD VOR REFERENCE ----------
  oVOR  = 0;
  CreateVOR();
	//--- Init Pack capacity -------------------------------------
	nb		= 1000;
	GetIniVar		("Performances", "PackCapacity", &nb);
	globals->pakCAP = nb;
  //----Init all distances -------------------------------------
  float dd  = 24;                         // Default (nm)for detection
  GetIniFloat	("Performances","ObjectDetectDistance",&dd);
  globals->nmDET  = dd;                   // Limit in miles
  globals->ftDET  = FN_FEET_FROM_MILE(dd);
  //-------------------------------------------------------------
  float dw   = 12;                         // Default for drawing
  GetIniFloat	("Performances","ObjectDrawDistance",&dw);
  globals->nmDRW = dw;                    // Limit in miles
  globals->ftDRW = FN_FEET_FROM_MILE(dw);
  //----Level of details -----------------------------------------
  float d1   = 1.5;                    
  GetIniFloat("Performances","ObjectHigDetails",&d1);
  if (d1 < 0)     d1  = 1.5;
  if (d1 > dw)    d1  = 1.5;
  globals->ftLD1      = FN_FEET_FROM_MILE(d1);
  //--------------------------------------------------------------
  float d2   = 2.5;
  GetIniFloat("Performances","ObjectMidDetails",&d2);
  if (d2 < 0)     d2  = 2.5;
  if (d2 < d1)    d2  = d1 * 2;
  globals->ftLD2      = FN_FEET_FROM_MILE(d2);
  //--------------------------------------------------------------
  float d3   = 5;
  GetIniFloat("Performances","ObjectLowDetails",&d3);
  if (d3 < 0)     d3  = 5;
  if (d3 < d2)    d3  = d2 * 2;
  globals->ftLD3      = FN_FEET_FROM_MILE(d3);
  //--------------------------------------------------------------
  int lf    = 500;                        // Decoding factor default
  GetIniVar("Performances","ObjectLoadFactor",&lf);
  dFactor   = lf;
  //--------------------------------------------------------------
	int lq = 1;
	GetIniVar("Performances","LookOnlyInSQL",&lq);
	lpod  = (1 - lq);
	//--------------------------------------------------------------
	sql	= globals->objDB;
	if (0 == sql)	lpod = 1;
  return;
}
//--------------------------------------------------------------------
//  Create a unic VOR for drawing the nearest VOR only
//--------------------------------------------------------------------
void C3DMgr::CreateVOR()
{ CWobj *obj  = new CWvor('nvor');
  obj->pDis   = 25000000;
  obj->SetDayRef("VOR.SMF");
  obj->SetObjDesc("VOR*");
  oVOR        = obj;
  oVOR->snap  = 1;
  return;
}
//--------------------------------------------------------------------
//  Delete resources
//	NOTE: If everything works as expected, then the model map
//	mapMOD must be empty when getting here
//	except at end
//--------------------------------------------------------------------
C3DMgr::~C3DMgr()
{ if (sphere) gluDeleteQuadric(sphere);
  //---Purge the vor Queue---------------------------
  oVOR->DecUser();
  //----Empty light Queue if any left ---------------
  C3DLight *lit = litQ.Pop();
  while (lit) {delete lit; lit = litQ.Pop();}
	//--- Empty model queue ----------------------------
	std::map<std::string,C3Dmodel *>::iterator it;
	for (it = mapMOD.begin(); it != mapMOD.end(); it++)
	{	C3Dmodel *mod = (*it).second;
		delete mod;
	}
	mapMOD.clear();
  //--------------------------------------------------
  if (tr) TraceCount();
  //----Un register manager -------------------------
  globals->m3d = 0;         // Unregister
}
//-----------------------------------------------------------------------------
//  Set Open GL State
//-----------------------------------------------------------------------------
void C3DMgr::SetDrawingState()
{ glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  return;
}
//-----------------------------------------------------------------------
//  Trace object counts
//-----------------------------------------------------------------------
void C3DMgr::TraceCount()
{ TRACE("C3DMgr Model   left: % 5d",globals->NbMOD);
  TRACE("C3DMgr Objects left: % 5d",globals->NbOBJ);
  return;
}
//--------------------------------------------------------------------
//  Delete Warning
//--------------------------------------------------------------------
void C3DMgr::Warning(CWobj *obj,char *msg)
{ char *fn = obj->GetFileOBJ();
  WARNINGLOG("File %s Object %s: %s",fn,obj->name,msg);
  delete obj;
}
//--------------------------------------------------------------------
//  Add all lights in a dedicated queue.
//  -Lights will be rendered in the last step
//--------------------------------------------------------------------
void C3DMgr::LightToDraw(C3DLight *lit)
{ C3DLight *itm = lit;
  C3DLight *nxt = 0;
  while (itm)
  { nxt = (C3DLight *)itm->GetNext();
    litQ.PutEnd(itm);
    itm = nxt;
  }
  return;
}
//--------------------------------------------------------------------
//  Trace object loaded for this QGT
//--------------------------------------------------------------------
void C3DMgr::TraceLoad(int nb,char *src,C_QGT *qt)
{	U_INT qx = qt->GetXkey();
	U_INT qz = qt->GetZkey();
	TRACE("%05d Objects loaded for QGT(%03d-%03d) from %s",nb,qx,qz,src);
	return;
}
//--------------------------------------------------------------------
//  Locate all models files that are related to this QGT
//  Objects are loaded from the SQL database OBJ.db
//	Then collected from loaded POD
//--------------------------------------------------------------------
int C3DMgr::LocateObjects(C_QGT *qgt)
{ int nbo = 0;
	//--- Search in SQL database----------------------
	if (sql)	nbo = globals->sqm->ReadWOBJ(qgt);
	if (tr && nbo) TraceLoad(nbo,"Database",qgt);
	//--- Search in files ----------------------------
  C3Dfile    scf(this,qgt);
	char *pod = 0;
  char dir[128];
  this->qgt = qgt;
  U_INT tx  = qgt->GetXkey();
  U_INT tz  = qgt->GetZkey();
  U_INT gtx = tx >> 1;
  U_INT gtz = tz >> 1;
  U_INT scx = tx & 0x01;
  U_INT scz = tz & 0x01;
  _snprintf(dir,128,"DATA/D%03d%03d/*.S%d%d",gtx,gtz,scx,scz);
  char* name = (char*)pfindfirst (&globals->pfs,dir,&pod);
	//--------------------------------------------------
	while (name)
  { nbo = scf.Decode(name,pod); 
	  if (tr && nbo) if (tr && nbo) TraceLoad(nbo,name,qgt);
    name = (char*)pfindnext (&globals->pfs);		// Next file
  }
	//--- Return total decoded objects -----------------
  return qgt->Get3DW()->GetNwoQ();
}
//--------------------------------------------------------------------
//  Locate the nearest VOR in this QGT
//	NOTE:  Only one VOR nearer than the current is selected
//				 So from frame to frame, the nearest VOR will be
//				 selected (if any) without too much overhead.
//--------------------------------------------------------------------
void C3DMgr::LocateVOR()
{ //--- Update VOR distance -----------------------
	CNavaid *nav = (CNavaid*)oVOR->GetUserParam();
  float    dis = (nav)?(nav->GetPDIS()):(25);
  oVOR->SetDistance(dis);
  nav					 = 0;
	//--- Find a VOR nearer than current one --------
  CNavaid *nnv = 0;
  for (nav = dbm->FindVORbyDistance(nav,dis);nav != 0; nav = dbm->FindVORbyDistance(nav,dis))
  { if (oVOR->GetUserParam() == nav)    continue;
		oVOR->UpdateWith(nav);
		return;
  }
  return;
}
//--------------------------------------------------------------------
//  Put model in the load queue. Activate file thread
//  NOTE:  Load may be called before the thread is activated
//--------------------------------------------------------------------
int C3DMgr::LoadTheModel(C3Dmodel *mod)
{ mod->IncUser();
  modQ.Lock();
  mod->SetState(M3D_INLOAD);
  modQ.PutEnd(mod);
  modQ.UnLock();
  globals->tcm->GoThread();
  return 1;
}
//--------------------------------------------------------------------
//  Return a model for loading
//  Called by file THREAD when geting next model to load
//--------------------------------------------------------------------
C3Dmodel *C3DMgr::ModelToLoad()
{ modQ.Lock();
  C3Dmodel *mod = modQ.Pop();
  modQ.UnLock();
  return mod;
}
//--------------------------------------------------------------------
//  Update various parameters
//--------------------------------------------------------------------
void C3DMgr::TimeSlice(float dT)
{ if (globals->noOBJ)  return;
  LocateVOR();
  return;
}
//--------------------------------------------------------------------
//  Draw the VOR
//--------------------------------------------------------------------
void C3DMgr::Draw(char tod)
{ if (globals->noOBJ)             return;
  U_CHAR mod = ('N' == tod)?(MODEL_NIT):(MODEL_DAY);
  glEnable(GL_TEXTURE_2D);
  //----------------------------------------------------------
  glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, White);
  glMaterialfv (GL_FRONT, GL_EMISSION, tcm->GetDeftEmission());
  //----Translate to object origin -------------------------
  SVector tr;
  tcm->RelativeFeetTo(*oVOR->ObjPosition(),tr);
  //--------------------------------------------------------
  if (oVOR->GetUserParam())
  { glPushMatrix();
    glTranslated(tr.x,tr.y,tr.z);
    oVOR->DrawModel(mod,0);
    glPopMatrix();
  }
  //-----Draw lights as a last step ------------------------
  DrawObjectLights();
  // Restore contexte --------------------------------------
  glDisable(GL_TEXTURE_2D);
}
//-----------------------------------------------------------------------------
//  Draw the lights
//  ALPHA TEST is enable
//  Transparent pixels with alpha value 0 are not entering deep buffer to
//  prevent masking of other objects
//-----------------------------------------------------------------------------
void C3DMgr::DrawObjectLights()
{ //----------------------------------------------------------
  C3DLight *lit = litQ.Pop();
  glDepthMask(false);
  while (lit)
  { lit->OutQueue();
    CWobj *prn  = (CWobj*)lit->GetParent();
    prn->PushLight(lit);
    //----Translate to object origin -------------------------
    SVector tr;
    globals->tcm->RelativeFeetTo(*lit->ObjPosition(),tr);
    //--------------------------------------------------------
    glPushMatrix();
    glTranslated(tr.x,tr.y,tr.z);
    lit->DrawModel(0);
    glPopMatrix();
    lit = litQ.Pop();
  }
  glDepthMask(true);
  return;
}

//----------------------------------------------------------------------
//  Trace No Texture
//----------------------------------------------------------------------
void C3DMgr::NoTexture(char *fn,char *tn)
{ if (0 == tr)       return;
  TRACE("   MODEL %s NO TEXTURE %s",fn,tn);
  return;
}
//----------------------------------------------------------------------
//  GET a model.  Model may be empty (no part decoded yet)
//----------------------------------------------------------------------
C3Dmodel *C3DMgr::AllocateModel(char *fn)
{ C3Dmodel *modl = 0;
  std::map<std::string,C3Dmodel *>::iterator itm;
  //---Look in object bank ----------------------
  itm = mapMOD.find(fn);
  if (itm != mapMOD.end())  
  { modl = (*itm).second;
    modl->IncUser();
    return modl;
  }
  //---Allocate a new model --------------------
  modl  = new C3Dmodel(fn,tr);
  modl->IncUser();
  mapMOD[fn] = modl;
  return modl;
}
//----------------------------------------------------------------------
//  Release Key
//----------------------------------------------------------------------
void C3DMgr::FreeModelKey(char *key)
{ mapMOD.erase(key);
  return;
}
//---------------------------------------------------------------------
//  Get Statistical data
//---------------------------------------------------------------------
void C3DMgr::GetStats(CFuiCanva *cnv)
{ char txt[128];
  cnv->AddText(1,"3D Objects:");
  sprintf_s(txt,128,"% 8d",globals->NbOBJ);
  cnv->AddText(STATS_NUM,txt,1);

  cnv->AddText(1,"3D Drawed :");
  sprintf_s(txt,128,"% 8d",globals->cnt1);
  cnv->AddText(STATS_NUM,txt,1);

  cnv->AddText(1,"3D Models:");
  sprintf_s(txt,128,"% 8d",globals->NbMOD);
  cnv->AddText(STATS_NUM,txt,1);
	
	cnv->AddText(1,"3D vertices:");
  sprintf_s(txt,128,"% 8d",globals->NbPOL);
  cnv->AddText(STATS_NUM,txt,1);

  globals->cnt1 = 0;
  return;
}
//======================================================================================
//
//  3D file to decode scenery files
//
//======================================================================================
C3Dfile::C3Dfile(C3DMgr *m,C_QGT *qt)
{ wMgr  = m;
  oQGT  = qt;
  hld   = 0;
	cntr	= 0;
}
//---------------------------------------------------------------------
//  Remove count to CFileName
//---------------------------------------------------------------------
C3Dfile::~C3Dfile()
{ 
}
//---------------------------------------------------------------------
//  Decode Scenery file
//  NOTE: CFileName is allocated to the scenery file to be shared by all 
//  objects (CWobj) created by this file. Each object is thus related
//  to its mother scenery file (This is for the future object editor)
//  The last deleted CWobj will free the CFileName object
//  -Each CWobj object is decoded by reading its own tags, then 
//  the new object calls the C3DMgr (3D manager) to add this object
//  to the corresponding QGT C3Dworld manager.
//---------------------------------------------------------------------
int C3Dfile::Decode(char *fname,char *pn)
{ char *dt  = "DATA/";
	char *deb = strstr(fname,dt) + strlen(dt);
	char *pod = GetSceneryPOD(pn);
	//---Save the file name --------------------------
	_snprintf(fullN,(PATH_MAX-1),"(%s)-(%s)",deb,pod);
  strncpy(namef,fname,63);
  namef[63] = 0;
  //---Open the file -------------------------------
	cntr			= 0;
  SStream s;
  if (OpenRStream (fname,s))
  { // Successfully opened stream
    ReadFrom (this, &s);                 
    CloseStream (&s);
  }
  return cntr;
}
//--------------------------------------------------------------------
//  Abort for unknown tag (temporary)
//--------------------------------------------------------------------
void C3Dfile::Abort(Tag tag)
{ char  msg[256];
  _snprintf(msg,256,"Scenery file %s: Bad Tag %s", namef,TagToString(tag));
  gtfo(msg);
  return;
}
//--------------------------------------------------------------------
//  Abort with message
//--------------------------------------------------------------------
void C3Dfile::Abort(char *err)
{ char msg[256];
  _snprintf(msg,256,"Scenery file %s: %s", namef,err);
  gtfo(msg);
  return;
}
//--------------------------------------------------------------------
//  Mark object as a shared one if needed
//  To be a shared object:
//  1) There must be a place holder hld previously decoded
//  2) Object should have the <hold> tag
//  3) The placeHolder name should match
//--------------------------------------------------------------------
bool C3Dfile::MarkHold(CWobj *obj)
{ if (0 == hld)         return false;
  if (obj->NoHold())    return false;
  char *hid = hld->GetName();
  char *kid = obj->GetHold();
  if (strcmp(hid,kid))  return false;
  //----- This is a shared object --------
   hld->Add(obj);          // Add to holder list
  obj->oPos = hld->oPos;  // Set geo position
  obj->inf  = hld->inf;   // Set terrain info from hld

  //----- Set position to lights ---------
  C3DLight *lit = obj->Lite;
  while (lit) 
  { lit->SetLocation(obj->oPos);
    lit = (C3DLight*)lit->GetNext();
  }

  return true;
}

//--------------------------------------------------------------------
//  Read Parameters
//--------------------------------------------------------------------
int C3Dfile::Read(SStream *st,Tag tag)
{ Tag    kind;
  CWobj *obj = 0;
  switch(tag) {
  //---New world object ----------------------------
  case 'wobj':
    { ReadTag(&kind,st);
      //   MEMORY_LEAK_MARKER ("C3Dfile")
      if (kind == 'mobj')  obj = new CWobj(kind);
      if (kind == 'nvor')  obj = new CWvor(kind);
      if (kind == 'wdsk')  obj = new CWobj(kind);
      if (kind == 'gvew')  obj = new CGvew(kind);
      if (kind == 'wsok')  obj = new CWsok(kind);
      if (kind == 'atna')  obj = new CWobj(kind);
      if (kind == 'slit')  obj = new CWobj(kind);
      if (kind == 'becn')  return TAG_IGNORED;
      if (kind == '@skt')  return TAG_IGNORED;
      if (kind == 'hold') {obj = new CWhld(kind); hld = (CWhld*)obj;}
      if (0 == obj)   Abort(kind);
      //--- Process object  ----------------------
      obj->SetFileName(fullN);
      ReadFrom(obj,st);
      if (ForExport())  {exQ.PutEnd(obj); return TAG_READ;}
      if (MarkHold(obj))                  return TAG_READ;
      C3Dworld *w3d = oQGT->Get3DW();
      w3d->AddToWOBJ(obj);
			cntr++;
      return TAG_READ;}
  case 'auto': // lc added 05.29.11 test for autogen
   // AutoGen (st);
		return TAG_READ;
  }
  WARNINGLOG("Scenery file %s: Not an Object tag %s",namef,TagToString(tag));
  return TAG_IGNORED;
}
//--------------------------------------------------------------------
//  Process autogen
//--------------------------------------------------------------------
void C3Dfile::AutoGen(SStream *st)
{ Tag    kind;
	ReadTag(&kind,st);
 // MEMORY_LEAK_MARKER ("auto")
  if (kind == 'mobj') 
	{ CWobj *autobj = 0; 
		// Process object
		char name [16] = {0};
		SPosition pos; pos.lat = 0.0; pos.lon = 0.0; pos.alt = 5000.0;
    C3Dworld *w3d = oQGT->Get3DW();
    C_QGT *qgt= w3d->GetQGT ();
    double midlat = qgt->GetMidLat ();
    double midlon = qgt->GetMidLon ();
//  TRACE ("%s %f %f", namef, midlat, midlon);
		int rdn = (globals->clk->GetMinute() << 8) +  globals->clk->GetSecond();
		srand(rdn);
		double var = 0.0;
		//TRACE ("num_of_autogen=%d", num_of_autogen); 
		for (int i = 0; i < globals->num_of_autogen; i++) { 
      autobj = new CWobj(kind);
      if (0 == autobj)   Abort(kind); 
      autobj->SetFileName(namef);
      // random position -1250/+1250 from the center of the QGT
      var = (rand () % 2500 - 1250);
      pos.lat = var + midlat;
      //TRACE ("%f", var);
      // random position -1250/+1250 from the center of the QGT
      var = (rand () % 2500 - 1250);
      pos.lon = var + midlon;
      //TRACE ("%f", var);
      autobj->SetPosition (pos);
      SVector vect; vect.x = vect.z = 0.0;
      vect.y = static_cast <double> (rand () % 360);
      //vect.y = DegToRad (vect.y);
      autobj->SetOrientation (vect);
      TRACE ("rad. %f", autobj->GetYRotation ()); 
      U_INT flag = 33077;
      autobj->SetFlag (flag);
      sprintf_s (name,15, "test%d", i); 
      autobj->SetObjName (name);
      CKmm dOBJ (autobj, MODEL_DAY);
      //
//      GroundSpot spot (pos.lon, pos.lat);
//      spot.GetTerrain ();
//      char terrain_type = spot.Type;
//      int  terrain_alt  = spot.alt;
//      TRACE ("terr. %d ** %d", terrain_type, terrain_alt);
//      if (spot.Type != TERRAIN_WATER_OCEAN) {
      //
      dOBJ.SetMdl ("foret000.bin");//("chene20.bin");
//      dOBJ.SetMdl ("jument.bin");
          //
      w3d->AddToWOBJ(autobj);
//      } else {
//        i--;
//      }
    }	
	}
  //MEMORY_LEAK_MARKER ("auto")
  return;
}
//===================================================================================
//  C3D model to hold parts from a SMF or BIN file
//
//===================================================================================
C3Dmodel::C3Dmodel(char *fn, char t)
{ state       = M3D_INIT;                      // 0= unloaded
  int dim     = strlen(fn);
  fname       = new char[dim+2];
  strncpy(fname,fn,dim);
  fname[dim]  = 0;
	mdtr				= t;
  User        = 0;
  aBot        = 0;
  aTop        = 0;
  rDIR        = GL_CCW;
  type        = 0;
  pthread_mutex_init (&mux,  NULL);
  //--- Init redirection table -----------------------
  rLOD[0]     = 0;
  rLOD[1]     = 0;
  rLOD[2]     = 0;
  rLOD[3]     = 0;
	//--------------------------------------------------
	globals->NbMOD++;
  //--------------------------------------------------
  char * dot  = strchr(fn,'.');
  if (0 == dot) return;
  bool   smf  = (strcmp(dot,".SMF") == 0);
  bool   bin  = (strcmp(dot,".BIN") == 0);
	bool   obj  = (strcmp(dot,".OBJ") == 0);
	//---- Mettre une table ici --------------
  if (smf) rDIR = GL_CCW;
  if (bin) rDIR = GL_CW;
	if (obj) rDIR = GL_CCW;
  if (smf)  type = M3D_SMF;
  if (bin)  type = M3D_BIN;
	if (obj)	type = M3D_OBJ;
	if (mdtr)	TRACE("CREATION MODEL %s", fname);
}
//-------------------------------------------------------------------------------
//  Destroy 3D model
//-------------------------------------------------------------------------------
C3Dmodel::~C3Dmodel()
{ char pn[PATH_MAX];
  _snprintf(pn,(PATH_MAX-1),"MODELS/%s",fname);
	if (mdtr)	TRACE("DESTRUCTION MODEL %s",fname);
  delete [] fname;
	globals->NbMOD--;

}
//-------------------------------------------------------------------------------
//  Return object maximum dimension
//-------------------------------------------------------------------------------
double C3Dmodel::MaxExtend()
{ double mx = exts.x;
  if (exts.y > mx)  mx = exts.y;
  if (exts.z > mx)  mx = exts.z;
  return mx;
}
//-------------------------------------------------------------------------------
//  Unload Part
//-------------------------------------------------------------------------------
void C3Dmodel::UnloadPart()
{ //----Release parts ----------------
  UnloadPart(0);
  UnloadPart(1);
  UnloadPart(2);
  UnloadPart(3);
  return;
}
//-------------------------------------------------------------------------------
//  Unload Partfor level k
//-------------------------------------------------------------------------------
void C3Dmodel::UnloadPart(int k)
{ //----Release parts ----------------
  C3DPart *prt = pLOD[k].Pop();
  while (prt)
  {	delete prt;
    prt = pLOD[k].Pop();
  }
  return;
}
//-------------------------------------------------------------------------------
//  Load the model if not yet done
//   Add the file name to the file system
//-------------------------------------------------------------------------------
int C3Dmodel::LoadPart(char * dir)
{ char    pn[PATH_MAX];
  _snprintf (pn,(PATH_MAX-1),"%s/%s",dir,fname);             
  //--- decode a smf file -------------------------
  if (type == M3D_SMF)
  { CSMFparser smf(W3D_OBJECT);
    state = smf.Decode(pn,W3D_OBJECT);
		smf.LoadModel(this);
   // if (mdtr) TRACE("LOAD %30s FACE %04d",fname,nbf);
    return state;
  }
  //--- decode a bin file -------------------------
  if (type == M3D_BIN)
  { CBINparser bin(W3D_OBJECT);
    state = bin.Decode(pn,W3D_OBJECT);
		bin.LoadModel(this);
    //TRACE("LOAD %20s FACE %04d",fname,nbf);
    return state;
  }
	//--- Decode  OBJ file -------------------------
	if (type == M3D_OBJ)
	{	COBJparser obj(W3D_OBJECT);
		state = obj.Decode(pn,W3D_OBJECT);
		obj.LoadModel(this);
    return state;
	}
  state = M3D_ERROR;
  return M3D_ERROR;
}
//-------------------------------------------------------------------------------
//  Add a new part From POD file
//  BIN add each face as a separate part
//  SMF add whole part at a time
//-------------------------------------------------------------------------------
int C3Dmodel::AddPodPart(C3DPart *prt)
{ //---Add this part ---------------------------------
  pLOD[0].Lock();
  pLOD[0].PutLast(prt);
  pLOD[0].UnLock();
  return 1;
}
//-------------------------------------------------------------------------------
//  Add a LOD part into model from PolyShop
//-------------------------------------------------------------------------------
void C3Dmodel::AddLodPart(C3DPart *prt,int lod)
{ //---Queue the part to related LOD ---------------------
  prt->SetLOD(lod);
  pLOD[lod].PutLast(prt); 
  rLOD[lod] = lod;
  return;
}
//-------------------------------------------------------------------------------
//  Add part into model from SQL database
//-------------------------------------------------------------------------------
void C3Dmodel::AddSqlPart(C3DPart *prt,int lod)
{ //---Queue the part to related LOD -----------------
  prt->SetLOD(lod);
  pLOD[lod].PutLast(prt); 
  rLOD[lod] = lod;            // Activate lod level
  return;
}
//-------------------------------------------------------------------------------
//  Return part geometry to polygon reduction
//-------------------------------------------------------------------------------
void C3Dmodel::GetParts(CPolyShop *psh,M3D_PART_INFO &inf)
{ C3DPart  *prt;
  pLOD[0].Lock();
  for (prt = pLOD[0].GetFirst(); prt != 0; prt = prt->Next())   
  { prt->GetInfo(inf);
    psh->OnePart(inf);
  }
  pLOD[0].UnLock();
  return;
}
//-------------------------------------------------------------------------------
//  Return part for texture export
//-------------------------------------------------------------------------------
void C3Dmodel::GetParts(CExport *exp,M3D_PART_INFO &inf)
{ C3DPart  *prt;
  pLOD[0].Lock();
  for (prt = pLOD[0].GetFirst(); prt != 0; prt = prt->Next())   
  { prt->GetInfo(inf);
    exp->OneM3DPart(inf);
  }
  pLOD[0].UnLock();
  return;
}
//-------------------------------------------------------------------------------
//  Draw parts from requested resolution
//  k is the requested LOD.
//  k is remaped to rLOD[] table when only the LOD 0 is available
//-------------------------------------------------------------------------------
int C3Dmodel::Draw(int q)
{ int k = rLOD[q];
  C3DPart  *prt;
  glFrontFace(rDIR);
  pLOD[k].Lock();
  for (prt = pLOD[k].GetFirst(); prt != 0; prt = prt->Next())   prt->Draw();
  pLOD[k].UnLock();
  return 0;
}
//-------------------------------------------------------------------------------
//  Count vertices and polygons
//-------------------------------------------------------------------------------
void C3Dmodel::Counts(M3D_PART_INFO &tt)
{ M3D_PART_INFO cn;
  C3DPart *prt  = 0;
  tt.NbIN       = 0;
  tt.NbVT       = 0;
  pLOD[0].Lock();
  for (prt = pLOD[0].GetFirst(); prt != 0; prt = prt->Next())
  { prt->GetInfo(cn);
    tt.NbIN += cn.NbIN;
    tt.NbVT += cn.NbVT;
  }
  pLOD[0].UnLock();
  return;
}
//-------------------------------------------------------------------------------
//  Called by Camera
//-------------------------------------------------------------------------------
void C3Dmodel::PreDraw(CCamera *cam)
{ glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushClientAttrib (GL_CLIENT_ALL_ATTRIB_BITS);
  glPolygonMode(GL_FRONT,GL_FILL);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  return;
}
//-------------------------------------------------------------------------------
//  Called by Camera
//-------------------------------------------------------------------------------
void C3Dmodel::EndDraw(CCamera *cam)
{ glPopClientAttrib();
  glPopAttrib();
  return;
}
//-------------------------------------------------------------------------------
//  Increment user
//-------------------------------------------------------------------------------
void C3Dmodel::IncUser()
{ pthread_mutex_lock (&mux);
  User++;
  pthread_mutex_unlock (&mux);
}
//-------------------------------------------------------------------------------
//  Decrement user
//-------------------------------------------------------------------------------
void C3Dmodel::DecUser()
{ pthread_mutex_lock (&mux);
  User--;
  int val = User;
  pthread_mutex_unlock (&mux);
  if (val)  return;
  globals->m3d->FreeModelKey(fname);
  delete this;
  return ;
}
//-------------------------------------------------------------------------------
//  Check for model state
//-------------------------------------------------------------------------------
void C3Dmodel::Finalize()
{	if (pLOD[0].IsEmpty())	state = M3D_EMPTY;
}
//===================================================================================
//  Model Q:  Pop out the models 
//  Models are shared and are deleted when no more users are refering to them
//===================================================================================
CModQ::~CModQ()
{ C3Dmodel *mod;
  for (mod = Pop(); mod != 0; mod = Pop()) continue;
}
//===================================================================================
//  WORLD OBJECT
//===================================================================================
CWobj::CWobj(Tag k)                   // : CmHead(SHR,WOB)
{ type       = 0;
  User      = 1;
  pthread_mutex_init (&mux,  NULL);
  wd3D      = 0;
  kind      = k;
  fnam      = 0;
  name      = 0;
  flag      = 0;
  desc      = 0;
  nozb      = 0;
  nozu      = 0;
  count     = 0;
  cand      = 0;
  shar      = 0;
	snap			= 1;
  fspd      = 0;
  oPos.lat  = 0;
  oPos.lon  = 0;
  modL[MODEL_DAY] = 0;
  modL[MODEL_NIT] = 0;
  oAng.x = oAng.y = oAng.z = 0;
  Lite      = 0;
  inf.qgt   = 0;
  inf.sup   = 0;
  globals->NbOBJ++;
}
//---------------------------------------------------------------------
//  Free resources
//---------------------------------------------------------------------
CWobj::~CWobj()
{ C_QGT *qt = inf.qgt;
  FreeLites();
  if (desc) delete [] desc;
  if (name) delete [] name;
  if (fnam) delete [] fnam;
  if (modL[MODEL_DAY])  (modL[MODEL_DAY])->DecUser();
  if (modL[MODEL_NIT])  (modL[MODEL_NIT])->DecUser();
  globals->NbOBJ--;
}
//-----------------------------------------------------------------------
//	Decrement user count
//	When object is marked for delete, it is recycled when user count is 0
//	return 1 if object is deleted
//-----------------------------------------------------------------------
void CWobj::DecUser()
{	//--Lock object here ----------------------
  pthread_mutex_lock (&mux);
	User--;
  bool del = (User == 0);
	//--Unlock object here --------------------
  pthread_mutex_unlock (&mux);
	if (del)  delete this;
	return;
}
//---------------------------------------------------------------------
//  Check object ident
//---------------------------------------------------------------------
void CWobj::Check()
{ return;
}
//---------------------------------------------------------------------
//  Free the light stack
//---------------------------------------------------------------------
void CWobj::FreeLites()
{ C3DLight *obj = PopALight();
  while (obj)
  { delete obj;
    obj = PopALight();
  }
}
//---------------------------------------------------------------------
//  Push a light in the stack
//---------------------------------------------------------------------
void CWobj::PushLight(C3DLight *obj)
{ obj->SetNext(Lite);
  Lite      = obj;
  return;
}
//---------------------------------------------------------------------
//  Pop a light from the stack
//---------------------------------------------------------------------
C3DLight *CWobj::PopALight()
{ C3DLight *obj = Lite;
  Lite      = (obj)?((C3DLight*)obj->GetNext()):(0);
  return obj;
}
//--------------------------------------------------------------------
//  Read Parameters
//--------------------------------------------------------------------
int CWobj::Read(SStream *st,Tag tag)
{ int     nbr;
  char    txt[64];
  Tag     pm;
  CShdw   shdw;                   // Shadow infos
  //--------------------------------------------------
  switch(tag) {
  //---New world object ----------------------------
  case 'geop':
    { ReadPosition(&oPos,st);
      return TAG_READ;
    }
  case 'type':
    ReadTag(&pm,st);
    return TAG_READ;

  case 'flag':
    ReadInt(&nbr,st);
    flag  = (U_INT)nbr;
    return TAG_READ;

  case 'detl':
    ReadInt(&nbr,st);
    return TAG_READ;

  case 'id  ':
    { ReadString(txt,63,st);
      txt[63] = 0;
      SetObjName(txt);
      return TAG_READ;
    }
  case 'iang':
    ReadVector(&oAng,st);
    oAng.y = -RadToDeg(oAng.y);
    return TAG_READ;

  case 'kmmd':
  case 'mmgr':
    { CKmm dOBJ(this,MODEL_DAY);
      ReadFrom(&dOBJ,st);       
      return TAG_READ;
    }
  case 'kmmn':
  case 'nmgr':
    { CKmm nOBJ(this,MODEL_NIT);
      ReadFrom(&nOBJ,st);       
      return TAG_READ;
    }
  case 'shdw':
    ReadFrom(&shdw,st);
    return TAG_READ;

  case 'name':
    { ReadString(txt,63,st);
      txt[63] = 0;
      SetObjDesc(txt);
      return TAG_READ;
    }
  case 'no_Z':
    nozb = 1;
		nozu = 1;
    return TAG_READ;

  case 'noZu':
    nozu = 1;
    return TAG_READ;

  case 'fspd':
    ReadInt(&fspd,st);
    return TAG_READ;

  case 'fclr':
    nbr = 0;
    ReadInt(&nbr,st);
    return TAG_READ;

  case 'ftyp':
    nbr   = 0;
    ReadInt(&nbr,st);
    return TAG_READ;

  case 'cntr':
    nbr   = 0;
    ReadInt(&nbr,st);
    count = nbr;
    return TAG_READ;

  case 'hold':
    shar = 1;
    ReadString(txt,63,st);
    txt[63] = 0;
    SetObjDesc(txt);
    return TAG_READ;

  case 'fnum':
    nbr = 0;
    ReadInt(&nbr,st);
    return TAG_READ;

  case 'lite':
    { ReadTag(&pm,st);
      C3DLight *lit = new C3DLight(this,pm);
      ReadFrom(lit,st);
      PushLight(lit);
      return TAG_READ;
    }
  case 'sort':              // Z sorted Ignore for now
    return TAG_READ;
  }
  TagToString(txt,tag);
  gtfo("bad tag %s in %s",txt,fnam);
  return TAG_IGNORED;
}
//-----------------------------------------------------------------------
//  Set the file name 
//-----------------------------------------------------------------------
void CWobj::SetFileName(char *fn)
{ int dim = strlen(fn);
  fnam    = new char[dim+1];
  strncpy(fnam,fn,dim+1);
  return;
} 
//-----------------------------------------------------------------------
//  All Parameters are read.  Complete object 
//-----------------------------------------------------------------------
bool CWobj::Localize(C_QGT *qgt)
{ inf.qgt  = qgt;
  inf.alt  = 0;
  globals->tcm->GetTerrainInfo(inf,oPos);
  oPos.alt = inf.alt;
  C3DLight *lit = Lite;
  //---Process all lights locations ------------------
  while (lit) 
  { lit->SetLocation(oPos);
    lit = (C3DLight*)lit->GetNext();
  }
  //--- If no valid position, delete object ---------
  if (inf.sup)   return true;
  WARNINGLOG("CWobj: geop not in QGT");
  delete this;
  return false;
}

//-----------------------------------------------------------------------
//  Warning message 
//-----------------------------------------------------------------------
void CWobj::Warning(char *err)
{ WARNINGLOG("Object %s: %s",this->name,err);
  delete this;
  return;
}
//--------------------------------------------------------------------
//  Check if place may be allocated
//--------------------------------------------------------------------
bool CWobj::NoPlace()
{ if (cand++ == count)  return false;
  return true;
}
//--------------------------------------------------------------------
//  Release Object and add a warning
//--------------------------------------------------------------------
int CWobj::ReleaseMe()
{ WARNINGLOG("Scenery file %s: Object %s.  Bad geop coordinates",fnam,name);
  delete this;
  return 0;
} 
//--------------------------------------------------------------------
//  Get model name
//--------------------------------------------------------------------
char *CWobj::ModelName(U_CHAR rq)
{ C3Dmodel *mod = modL[rq];
  if (0 == mod) return "None";
  return mod->GetFileName();
}
//--------------------------------------------------------------------
//  Edit model
//--------------------------------------------------------------------
void CWobj::EditModel(CFuiCanva *cnv,char *name, M3D_PART_INFO &inf)
{ char edt[128];
  int nvt = inf.NbVT;
  int npo = inf.NbIN / 3;
  _snprintf(edt,128,"Model: %s ",name);
  cnv->AddText(1,edt,1);
  _snprintf(edt,128,"Vertices %d",nvt);
  cnv->AddText(1,edt);
  _snprintf(edt,128,"Poly %d",npo);
  cnv->AddText(14,edt,1);
  return;
}
//--------------------------------------------------------------------
//  Return longitude as integer
//--------------------------------------------------------------------
U_INT CWobj::GetIntLongitude()
{ int pos = int(oPos.lon * TC_INT_PER_ARCSEC);
  return (U_INT)pos;
  }
//--------------------------------------------------------------------
//  Return latitude as integer
//--------------------------------------------------------------------
U_INT CWobj::GetIntLatitude()
{ int pos = int(oPos.lat * TC_INT_PER_ARCSEC);
  return pos;
}
//--------------------------------------------------------------------
//  Return Day model name
//--------------------------------------------------------------------
char *CWobj::DayName()
{ C3Dmodel *mod = modL[MODEL_DAY];
  return (mod)?(mod->GetFileName()):("");
}
//--------------------------------------------------------------------
//  Return Night model name
//--------------------------------------------------------------------
char *CWobj::NitName()
{ C3Dmodel *mod = modL[MODEL_NIT];
  return (mod)?(mod->GetFileName()):("");
}
//--------------------------------------------------------------------
//  Return extension from day model
//--------------------------------------------------------------------
double CWobj::GetXExtend()
{ C3Dmodel *mod = modL[MODEL_DAY];
  return (mod)?(mod->GetXExtend()):(0); }
//--------------------------------------------------------------------
//  Return extension from day model
//--------------------------------------------------------------------
double CWobj::GetYExtend()
{ C3Dmodel *mod = modL[MODEL_DAY];
  return (mod)?(mod->GetYExtend()):(0); }
//--------------------------------------------------------------------
//  Return extension from day model
//--------------------------------------------------------------------
double CWobj::GetZExtend()
{ C3Dmodel *mod = modL[MODEL_DAY];
  return (mod)?(mod->GetZExtend()):(0); }

//--------------------------------------------------------------------
//  Get object info
//--------------------------------------------------------------------
void CWobj::GetInfo(CFuiCanva *cnv)
{ C3Dmodel *modD = modL[MODEL_DAY];
  char     *name = ModelName(MODEL_DAY);
  M3D_PART_INFO inf;
  //---Edit day Model  characteristics -------------------
  modD->Counts(inf);
  EditModel(cnv,name,inf);
  return;
}
//--------------------------------------------------------------------
//  Decode model part file
//  NOTE:  When the model(s) are loaded, the Wobj is queued
//         to the Super Tile corresponding to the location
//--------------------------------------------------------------------
int CWobj::LoadMyModel()
{ int nbf = 0;
  C3Dmodel *modD = modL[MODEL_DAY];
  if (modD && modD->NeedLoad()) nbf += globals->m3d->LoadTheModel(modD);
  C3Dmodel *modN = modL[MODEL_NIT];
  if (modN && modN->NeedLoad()) nbf += globals->m3d->LoadTheModel(modN);
  return nbf;
}
//------------------------------------------------------------------------
//  Trace file name 
//------------------------------------------------------------------------
void CWobj::Trace(U_CHAR tq, char *tod)
{ C3Dmodel *mod = modL[tq];
  if (0 == mod)   return;
  char     *fn  = mod->GetFileName();
  TRACE("%s: File %s Flag= 0x%08X",tod,fn,flag);
  return;
}
//--------------------------------------------------------------------
//  Add a model
//  NOTE: For objects with multiple model, only the last is retain
//--------------------------------------------------------------------
void CWobj::AddModel(C3Dmodel *mod,U_CHAR q)
{ C3Dmodel *pmd = modL[q];
  modL[q]       = mod;
  if (0 == pmd) return;
  pmd->DecUser();
  return;
}
//-----------------------------------------------------------------------
//  Relocate object if snap to ground is requested
//-----------------------------------------------------------------------
void CWobj::Relocate(C3Dmodel *mod)
{ if (!mod->IsOK())			return;
	snap				= 0;
	double cor  = oPos.alt;
  oPos.alt    = inf.alt;
  oPos.alt   -= mod->GetGround();
  if (0 == Lite)        return;
  //--Relocate all lights ----------------------
  C3DLight *lit = Lite;
  double adj  = oPos.alt - cor;
  while (lit)
  { Lite->Relocate(adj);
    lit = (C3DLight*)lit->GetNext();
  }
  return;
}
//-----------------------------------------------------------------------
//  Edit Object position
//-----------------------------------------------------------------------
void CWobj::EditPos(char *t1,char *t2)
{ EditLat2DMS(oPos.lat,t1);
  EditLon2DMS(oPos.lon,t2);
	return;
}
//-----------------------------------------------------------------------
//  Draw object from one of model Queue
//-----------------------------------------------------------------------
int CWobj::DrawModel(char tod, char lod)
{ C3Dmodel *mod = modL[tod];
  if (0 == mod)   mod = modL[MODEL_DAY];
  if (0 == mod)   return 0;
  //---Snap Object to ground if requested --------------
  if (snap) Relocate(mod);
  //---Dont draw if focus and blinking or empty --------
	bool dd  = (globals->wObj == this);
			 dd &= !globals->clk->GetON();
			 dd |=  mod->IsEmpty();
  if (dd) return 0;
  mod->Draw(lod);
  //---Queue all light ---------------------------------
  if (Lite)  globals->m3d->LightToDraw(Lite);
  Lite = 0;
  return 0;
}
//------------------------------------------------------------------------
//  Set a day Reference to the model (fn)
//------------------------------------------------------------------------
void CWobj::SetDayRef(char *fn)
{ C3Dmodel *mod = globals->m3d->AllocateModel(fn);
  AddModel(mod,MODEL_DAY);
  return;
}
//------------------------------------------------------------------------
//  Set a night Reference to the model (fn)
//------------------------------------------------------------------------
void CWobj::SetNitRef(char *fn)
{ C3Dmodel *mod = globals->m3d->AllocateModel(fn);
  AddModel(mod,MODEL_NIT);
  return;
}
//------------------------------------------------------------------------
//  Set Object name
//------------------------------------------------------------------------
void CWobj::SetObjName(char *nm)
{ int lgr = strlen(nm) + 1;
  name    = new char[lgr];
  strncpy(name,nm,lgr);
  return;
}
//------------------------------------------------------------------------
//  Set Object description
//------------------------------------------------------------------------
void CWobj::SetObjDesc(char *nm)
{ int lgr = strlen(nm) + 1;
  desc    = new char[lgr];
  strncpy(desc,nm,lgr);
  return;
}
//------------------------------------------------------------------------
//  Refresh the squared Distance (dis²)
//------------------------------------------------------------------------
float CWobj::RefreshDistance()
{ float dis = inf.sup->GetTrueDistance();
  pDis      = dis * dis;
  return dis;
}
//------------------------------------------------------------------------
//  Update object with navaid data
//------------------------------------------------------------------------
void CWobj::UpdateWith(CNavaid *nav)
{ snap  = 1;
  pDis  = nav->GetPDIS();
  oPos  = nav->GetPosition();
  pmOB  = nav;
  inf.qgt = 0;
  globals->tcm->GetTerrainInfo(inf, oPos);
  //----Compute light position ------------------
  Lite->SetOffset(vorOFS);
  Lite->SetLocation(oPos);
  LoadMyModel();
  return;
}
//------------------------------------------------------------------------
//  Update VOR State
//------------------------------------------------------------------------
void CWobj::EndOfQGT(C_QGT *qt)
{ if (inf.qgt != qt) return;
  inf.qgt = 0;
  return;
}  
//==========================================================================
//    MODEL MANAGER DECODER
//==========================================================================
CKmm::CKmm(CWobj *obj,U_CHAR td)
{ wobj  = obj;
  tod   = td;
}
//--------------------------------------------------------------------
//  READ MODEL FROM FLY1 TAG
//  FOR NOW ignore low resolution model
//--------------------------------------------------------------------
int CKmm::ReadModel(SStream *st)
{ Tag idn;
  char fn[PATH_MAX];
  int  pm;
  ReadTag(&idn,st);
  ReadString(fn,PATH_MAX,st);
  ReadInt(&pm,st);
  if (modl)   return 0;
  _strupr(fn);
  modl    = globals->m3d->AllocateModel(fn);
  wobj->AddModel(modl,tod);
  return 1;
}
//--------------------------------------------------------------------
//  Read Parameters
//--------------------------------------------------------------------
int CKmm::Read(SStream *st,Tag tag)
{ char fn[PATH_MAX];
  Tag idn;
  switch (tag)  {
    //---FLY 1 tag ignore it -------------
    case 'simu':
        ReadTag(&idn,st);
        return TAG_READ;
    //---Unic model ----------------------
    case 'modl':
      { ReadTag(&idn,st);
        ReadString(fn,PATH_MAX,st);
        fn[FNAM_MAX] = 0;
        _strupr(fn);
        modl = globals->m3d->AllocateModel(fn);
        wobj->AddModel(modl,tod);
        return TAG_READ;
      }
    //---BI MODEL FLY 1 -----------------
    case 'mod2':
        ReadModel(st);
        return TAG_READ;
  }
  char txt[8];
  TagToString(txt,tag);
  gtfo("bad tag");
  return TAG_IGNORED;
}
//--------------------------------------------------------------------
//  SetModel
//--------------------------------------------------------------------
void CKmm::SetMdl (const char *model)
{ char fn[PATH_MAX];
  Tag idn = 'norm';
  strncpy (fn, model, PATH_MAX);
  fn[FNAM_MAX] = 0;
  _strupr(fn);
  modl = globals->m3d->AllocateModel(fn);
  wobj->AddModel(modl,tod);
  return;
}
//==========================================================================
//  SHADOW data
//==========================================================================
//--------------------------------------------------------------------
//  Read Parameters
//--------------------------------------------------------------------
int CShdw::Read(SStream *st,Tag tag)
{ switch (tag)  {
    case 'null':
      return TAG_READ;
  }
  char txt[8];
  TagToString(txt,tag);
  gtfo("bad tag: %s",txt);
  return TAG_IGNORED;
}
//==========================================================================
//  GROUND VEHICLE DECODER
//==========================================================================
CGvew::CGvew(Tag k) : CWobj(k)
{ lofi = false;
  bvel.x = bvel.y = bvel.z = 0;
  bang.x = bang.y = bang.z = 0;
}
//--------------------------------------------------------------------
//  Read Parameters
//--------------------------------------------------------------------
int CGvew::Read(SStream *st,Tag tag)
{ Tag dum;
  switch (tag)  {
    case '_NFO':
      ReadString(nfo,64,st);
      nfo[63] = 0;
      return TAG_READ;
    case 'vmod':
      ReadTag(&vmod,st);
      return TAG_READ;
    case 'lofi':
      lofi = true;
      return TAG_READ;
    case 'bvel':
      ReadVector(&bvel,st);
      return TAG_READ;
    case 'bang':
      ReadVector(&bang,st);
      return TAG_READ;
    case 'elec':
      { ReadTag(&dum,st);
        C3Ddummy dum;
        ReadFrom(&dum,st);
        return TAG_READ;
      }
  }
return CWobj::Read(st,tag);
}
//==========================================================================
//  WIND SOCK OBJECT
//==========================================================================
CWsok::CWsok(Tag k) : CWobj(k)
{ 
}
//--------------------------------------------------------------------
//  Read Parameters
//--------------------------------------------------------------------
int CWsok::Read(SStream *st,Tag tag)
{ Tag     dum;
  char    txt[128];
  SVector v;
  switch (tag)  {
    case '_NFO':
      ReadString(txt,64,st);
      return TAG_READ;
    case 'vmod':
      ReadTag(&dum,st);
      return TAG_READ;
    case 'lofi':
      return TAG_READ;
    case 'bvel':
      ReadVector(&v,st);
      return TAG_READ;
    case 'bang':
      ReadVector(&v,st);
      return TAG_READ;
    case 'elec':
      { ReadTag(&dum,st);
        C3Ddummy dum;
        ReadFrom(&dum,st);
        return TAG_READ;
      }
  }
return CWobj::Read(st,tag);
}
//----------------------------------------------------------------------------
//  Update Wind Parameters 
//  Set wind direction
//  Model rotation is corrected by 180° to get the wind direction
//----------------------------------------------------------------------------
void CWsok::Update(U_CHAR tod)
{ double ang  = globals->wtm->WindDirection ();
  double  flu = 0.0;//RandomNumber (50) * 0.01;
  //oAng.z      = Wrap360 (flu + ang + 180);
  oAng.z      = ang;
  //---Compute x rotation according to wind speed ------------
  ang  = static_cast <double> (globals->wtm->WindSpeed ());
  if (ang > 30.0) ang = 30.0;
  oAng.x      = 0.0;
  oAng.y      = 3.0 * ang - 90.0;
  oAng.z      = Wrap360 (oAng.z - (3.0 * (ang - 30.0)) - 180.0);

  return;
}
//----------------------------------------------------------------------------
//  Draw animated wind sock object
//  NOTE: Until a new 3Dmodel is provided, animation is only with the wind
//  direction.
//----------------------------------------------------------------------------
int CWsok::DrawModel(char tod,char lod)
{ glPushMatrix();
  glRotated (oAng.z, 0, 0, 1);   // Heading (Z)
  glRotated (oAng.y, 0, 1, 0);   // Bank    (Y);
  CWobj::DrawModel(tod,lod);
  glPopMatrix();
  return 1;
}
//============================================================================
//  VOR Object
//============================================================================
CWvor::CWvor(Tag kd): CWobj(kd)
{ //--- Add a light to VOR antenna -------------------------
  C3DLight *lit = new C3DLight(this,TC_LMOD_OMNI,TC_RED_LITE);
  PushLight(lit);
}
//---------------------------------------------------------------------------
//  Release VOR
//---------------------------------------------------------------------------
void CWvor::ReleaseOBJ()
{ pmOB    = 0;               // release any Navaid
  inf.qgt = 0;
  inf.sup = 0;
  return;
}
//============================================================================
//  Place holder
//============================================================================
CWhld::~CWhld()
{ for (U_INT k = 0; k < Hold.size(); k++) delete Hold[k];
  Hold.clear();
}
//--------------------------------------------------------------------
//  Select one object from the list (including none)
//--------------------------------------------------------------------
CWobj *CWhld::Select()
{ int nbo = Hold.size();
  int ind = RandomNumber(nbo + 1);
  if (ind == nbo)   return 0;
  CWobj *obj = Hold[ind];
  Hold[ind] = 0;
  return obj;
}
//=============================================================================
//  3Ddummy to eliminate unneeded tag and values
//=============================================================================
int C3Ddummy::Read(SStream *st, Tag tag)
{ int   nb;
  char  txt[32];
  switch (tag)
  { //---Tag for elec system -------------------------------
    case 'subs':
      { ReadString(txt,32,st);
        C3Ddummy dum;
        ReadFrom(&dum,st);
        return TAG_READ;
      }
    //--Tag for subsystem ----------------------------------
    case 'st8t':
      ReadInt(&nb,st);
      return TAG_READ;
  }
  return TAG_IGNORED;
}
//=============================================================================
//  A 3D object part
//=============================================================================
//----------------------------------------------------------------------
//  Alternate constructor
//----------------------------------------------------------------------
C3DPart::C3DPart()
{ next = prev = 0;
	tRef  = 0;
  lod   = tsp = 0;
  xOBJ  = 0;
  ntex  = 0;
  NbVT  = NbIN = 0;
  nVTX  = 0;
  nNRM  = 0;
  nTEX  = 0;
	vTAB	= 0;
  nIND  = 0;
  total = 0;
	strcpy(idn,"PART");
}
//----------------------------------------------------------------------
//  Free Texture
//----------------------------------------------------------------------
C3DPart::~C3DPart()
{ Release();
  total = 0;
}
//----------------------------------------------------------------------
//  Release all vertices
//----------------------------------------------------------------------
void C3DPart::Release()
{	if (tRef) globals->txw->Free3DTexture(tRef);
	if (ntex)     delete [] ntex;
	ntex  = 0;
  if (nVTX)     delete [] nVTX;
	nVTX	= 0;
  if (nNRM)     delete [] nNRM;
	nNRM	= 0;
  if (nTEX)     delete [] nTEX;
	nTEX	= 0;
  if (nIND)     delete [] nIND;
	nIND	= 0;
	if (vTAB)			delete [] vTAB;
	vTAB	= 0;
	globals->m3d->vCount(-NbVT);
	NbVT	= 0;
}
//----------------------------------------------------------------------
//  Allocate vertices for a 3D object
//----------------------------------------------------------------------
void C3DPart::AllocateW3dVTX(int nv)
{	NbVT  = nv;
	nVTX  = new F3_VERTEX[nv];
  nNRM  = new F3_VERTEX[nv];
  nTEX  = new F2_COORD [nv];
	Rend  = &C3DPart::DrawAsW3D;
	globals->m3d->vCount(NbVT);
}
//----------------------------------------------------------------------
// Allocate vertices for OSM Object
//----------------------------------------------------------------------
void C3DPart::AllocateOsmVTX(int nv)
{	Release();
	NbVT  = nv;
	vTAB  = new TC_VTAB[nv];
	Rend  = &C3DPart::DrawAsOSM;
	globals->m3d->vCount(NbVT);
	return;
}
//----------------------------------------------------------------------
// Allocate vertices for OBJ format
//----------------------------------------------------------------------
void C3DPart::AllocateObjVTX(int nv)
{	Release();
	NbVT  = nv;
	vTAB  = new TC_VTAB[nv];
	Rend  = &C3DPart::DrawAsOBJ;
	globals->m3d->vCount(NbVT);
	return;
}
//----------------------------------------------------------------------
//	Rotation around Z axis
//----------------------------------------------------------------------
void C3DPart::ZRotation(double sn, double cn)
{ TC_VTAB *vtx = vTAB;
	for (int k = 0; k != NbVT; k++)	ZRotate(*vtx++,sn,cn);
	return;
}
//----------------------------------------------------------------------
// Copy part for osm use
//----------------------------------------------------------------------
void C3DPart::CopyForOSM(C3DPart *ps)
{	int       *ind = ps->GetXLIST();
	F3_VERTEX *svt = 0;
	F2_COORD  *scd = 0;
	TC_VTAB   *dst = vTAB;
	for (int k=0; k < NbVT; k++)
	{	int inx   = *ind;
		svt       = ps->nVTX + inx;
		scd				= ps->nTEX + inx;
		dst->VT_X = svt->VT_X;
		dst->VT_Y = svt->VT_Y;
		dst->VT_Z = svt->VT_Z;
		dst->VT_S = scd->VT_S;
		dst->VT_T = scd->VT_T;
		dst++;
		ind++;
	}
	return;
}
//----------------------------------------------------------------------
// Clone part for osm use
//----------------------------------------------------------------------
void C3DPart::ReceiveOSM(C3DPart *ps, char opt)
{ if (0 == ps)		return;
	if (opt) NbVT	= 0;
	int nbv				= ps->GetNBVTX();
	TC_VTAB *src	= ps->GetVTAB();
	TC_VTAB *dst  = vTAB + NbVT;
	for (int k=0; k < nbv; k++)	*dst++ = *src++;
	NbVT				+= nbv;
	return;
}
//----------------------------------------------------------------------
// Translate by T vector
//----------------------------------------------------------------------
void C3DPart::Translate(SVector &T)
{	TC_VTAB *src = vTAB;
	for (int k=0; k < NbVT; k++, src++)		src->Add(T);
	return;
}
//----------------------------------------------------------------------
//  Assign texture name
//----------------------------------------------------------------------
void C3DPart::SetTexName(char *txn)
{ ntex = Dupplicate(txn,256); 
  return;
}
//----------------------------------------------------------------------
//  Assign texture object and reference
//----------------------------------------------------------------------
void C3DPart::SetTexture(char *txn)
{ void *ref = globals->txw->GetM3DPodTexture(txn,1,1);
  tRef			= ref;
	xOBJ			= globals->txw->Get3DObject(ref);
  return;
}

//----------------------------------------------------------------------
//  Return part infos
//----------------------------------------------------------------------
void C3DPart::GetInfo(M3D_PART_INFO &inf)
{ inf.NbIN = NbIN;                        // Indice count
  inf.NbVT = NbVT;                        // Vertices count
  inf.nIND = nIND;                        // Indice list
  inf.nNRM = nNRM;                        // Normal list
  inf.nVTX = nVTX;                        // Vertice list
  inf.nTEX = nTEX;                        // Texture coordinates
  inf.ref  = tRef ;                       // Texture reference
  //------------------------------------------------------------
  inf.tsp  = tsp;                         // Transparency
  inf.ntex = ntex;                        // Texture name
  return;
}
//----------------------------------------------------------------------
//  Draw the part for  a 3D object
//	Actually we dont use VBO.  There is one indice per vertex in nIND list
//----------------------------------------------------------------------
void C3DPart::DrawAsW3D()
{ if (0 == xOBJ)  xOBJ = globals->txw->Get3DObject(tRef);
  glBindTexture    (GL_TEXTURE_2D,xOBJ);
  glVertexPointer  (3,GL_FLOAT,0,nVTX);
  glNormalPointer  (  GL_FLOAT,0,nNRM);
  glTexCoordPointer(2,GL_FLOAT,0,nTEX);
  glDrawElements(GL_TRIANGLES,NbIN,GL_UNSIGNED_INT,nIND);
  //----------------------------------------------------------------
  //  {GLenum e = glGetError ();
  //  if (e != GL_NO_ERROR) 
  //    WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  // }
  //----------------------------------------------------------------
  return;
}

//----------------------------------------------------------------------
//  Draw the part for  an Osm object
//----------------------------------------------------------------------
void C3DPart::DrawAsOSM()
{ glBindTexture    (GL_TEXTURE_2D,xOBJ);
	glVertexPointer  (3,GL_FLOAT,sizeof(TC_VTAB),&(vTAB->VT_X));
  glTexCoordPointer(2,GL_FLOAT,sizeof(TC_VTAB),&(vTAB->VT_S));
  glDrawArrays(GL_TRIANGLES,0, NbVT);
  //----------------------------------------------------------------
  //  {GLenum e = glGetError ();
  //  if (e != GL_NO_ERROR) 
  //    WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  // }
  //----------------------------------------------------------------
  return;
}
//----------------------------------------------------------------------
//  Draw the part for  an OBJ format
//----------------------------------------------------------------------
void C3DPart::DrawAsOBJ()
{ GLint xob = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D,&xob);
	glBindTexture(GL_TEXTURE_2D,xOBJ);
	glFrontFace(GL_CCW);
	glVertexPointer  (3,GL_FLOAT,sizeof(TC_VTAB),&(vTAB->VT_X));
  glTexCoordPointer(2,GL_FLOAT,sizeof(TC_VTAB),&(vTAB->VT_S));
  glDrawArrays(GL_TRIANGLES,0, NbVT);
	glBindTexture(GL_TEXTURE_2D,xob);
  //----------------------------------------------------------------
  //  {GLenum e = glGetError ();
  //  if (e != GL_NO_ERROR) 
  //    WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  // }
  //----------------------------------------------------------------
  return;
}


//=============================================================================
//  Destroy world object queue
//=============================================================================
CObjQ::~CObjQ()
{ CWobj *obj = Pop();
  while (obj) { obj->DecUser(); obj = Pop();}
}

//=============================================================================
//
//  Class C3Dworld for managing all 3D objects located in a QGT
//
//=============================================================================
C3Dworld::C3Dworld()
{ float  ft = 0;
  int    nb = 0;
  qgt     = 0;
  GetIniVar("TRACE", "3DModel", &nb);
  tr  = (nb)?(1):(0);
  nColor[0] = 0.3f;
  nColor[1] = 0.3f;
  nColor[2] = 0.3f;
  nColor[3] = 1.0f;
  //----------------------------
  nOBJ      = 0;
	serial		= 0;
  //---Build wind map ----------
  CFmt1Slot s0(  5, 80);
  CFmt1Slot s1( 10, 70);
  CFmt1Slot s2( 20, 60);
  CFmt1Slot s3(100,  0);
  wmap = new CFmt1Map();
  wmap->Enter(s0);
  wmap->Enter(s1);
  wmap->Enter(s2);
  wmap->Enter(s3);
}
//-----------------------------------------------------------------------
//  End of 3D world for a deleted QGT
//-----------------------------------------------------------------------
C3Dworld::~C3Dworld()
{ globals->m3d->EndOfQGT(qgt);
  Clear3Dworld(); }
//-----------------------------------------------------------------------
//  Remove all world objects
//-----------------------------------------------------------------------
void C3Dworld::Clear3Dworld()
{ if (tr) TraceEnd();
  delete wmap;
  CWobj *obj = woQ.Pop();
  while (obj) {obj->DecUser(); obj = woQ.Pop();}
  std::map<std::string,CWobj*>::iterator it;
  for (it = hold.begin();it != hold.end(); it++) delete (*it).second;
	hold.clear();
  return;
}
//-----------------------------------------------------------------------
//  Trace object liberation
//-----------------------------------------------------------------------
void C3Dworld::TraceEnd()
{ int n1 = woQ.NbObjects();
  if (0 == qgt) return;
  int qx = qgt->GetXkey();
  int qz = qgt->GetZkey();
  int nb = qgt->GetNOBJ();
	if ((0 == n1) && (0 == nb))	return;
  TRACE("QGT(%03d-%03d) FREE woQ(% 4d CWobj).  Still in QGT % 4d",qx,qz,n1,nb); 
  return;
}
//-----------------------------------------------------------------------
//  Set reduction and compensation factor
//-----------------------------------------------------------------------
void C3Dworld::SetQGTparameters(C_QGT *q)
{ qgt  = q;
  GetLatitudeFactor(qgt->GetMidLat(),rFactor,cFactor);
  return;
} 
//-----------------------------------------------------------------------
//  Assign geoposition to place holder
//-----------------------------------------------------------------------
CWobj *C3Dworld::SelectOneOf(CWobj *obj)
{ CWhld *hld = (CWhld *)obj;
  CWobj *sel = hld->Select();
  if (sel) sel->SetParent(this);
  delete hld;
  return sel;
} 
//-----------------------------------------------------------------------
//  1)	Assign an object to place holder
//	2)	Load Object model
//	3)	
//-----------------------------------------------------------------------
int C3Dworld::AssignToSuperTile(CWobj *obj)
{ if (obj->IsaHold())  obj = SelectOneOf(obj);
  if (0   == obj)       return 0;
	obj->LoadMyModel();
	obj->GetSuperTile()->Add3DObject(obj,tr);
  return 1;
} 
//----------------------------------------------------------------------
//  QGT Time slice:  
//  All objects in queue are waiting to be put in SuperTile queue
//	for drawing.
//  -Compute object distance to aircraft.   Object distance is taken as
//   the distance of Super Tile center
//  -If the place holder must be instanciated select one object from
//    the list of candidates and generate it
//----------------------------------------------------------------------
void C3Dworld::TimeSlice(float dT)
{ CWobj *obj = 0;
  CWobj *prv = 0;
  if (globals->noOBJ)       return;
  int    cnt = 0;
  float  ftDET = globals->ftDET;      // Detect ring in feet
  //----scan waiting queue for objects entering  Decoding Ring ------
  for ( obj = woQ.GetFirst(); obj != 0; obj = woQ.GetNext(obj))
      { float dst = obj->RefreshDistance();
        if (dst > ftDET)     continue;
        //---- verify water
        if (globals->num_of_autogen) {
          SPosition pos = obj->GetPosition ();
          if (pos.alt < 215.0) continue; // DBL_EPSILON
        }
        //---- detach from waiting queue and decode ----------
        prv  = woQ.Detach(obj);
        AssignToSuperTile(obj);
        cnt++;
        if (cnt == 100)     break;
        //--- Restart with next to first object --------------
        if (0 == prv)       obj = woQ.GetFirst();
        else								obj = prv; 
        if (0 == obj)       break;
      }
		//--- Check for trace ------------------------------------
		if (tr) TraceAdd(cnt);
    return;
}
//-----------------------------------------------------------------------------
//  Check all objects
//-----------------------------------------------------------------------------
void C3Dworld::TraceAdd(int cnt)
{	if (0 == cnt)	return;
  U_INT qx = qgt->GetXkey();
	U_INT qz = qgt->GetZkey();
	TRACE("QGT(%03d-%03d) activates %05d Objects",qx,qz,cnt);
}
//-----------------------------------------------------------------------------
//  Check all objects
//-----------------------------------------------------------------------------
void C3Dworld::Check()
{ CWobj *obj = 0;
  for ( obj = woQ.GetFirst(); obj != 0; obj = woQ.GetNext(obj)) obj->Check();
  return;
}
//-----------------------------------------------------------------------------
//  Add a new object to Drawing Queue   
//-----------------------------------------------------------------------------
void C3Dworld::AddToWOBJ(CWobj *obj)
{ obj->SetParent(this);
  obj->SetSerial(++serial);
	obj->SetTrace(tr);
	if (tr) {
		char la[32];
		char lo[32];
		char *name = obj->ModelName(MODEL_DAY);
		obj->EditPos(la,lo);
		//TRACE("Add object %s QGT(%03d-%03d)", name,qgt->GetXkey(),qgt->GetZkey());
	}
  //--- Set object localization ---------------
  if (obj->Localize(qgt)) woQ.PutEnd(obj);     
  //-------------------------------------------
  return;
}
//-----------------------------------------------------------------------------
//  Draw 3Dobjects located in visible Super Tiles
//  Camera must be set to origin
//-----------------------------------------------------------------------------
void C3Dworld::Draw(U_CHAR tod)
{  U_CHAR     mod = ('N' == tod)?(MODEL_NIT):(MODEL_DAY);
  //----------------------------------------------------------
  CSuperTile *sup = 0;
  for (U_INT No = 0; No != TC_SUPERT_NBR; No++) 
  { sup = qgt->GetSuperTile(No);
    if  (!sup->Update()) continue;
    globals->cnt1 += sup->Draw3D(mod);       // Count objects          
  }
  //----------------------------------------------------------
  return;
}
//-----------------------------------------------------------------------------
//  Get a line for each 3D object ready to be drawed
//-----------------------------------------------------------------------------
void C3Dworld::GetLine(CListBox *box)
{ CSuperTile *sup = 0;
  for (U_INT No = 0; No != TC_SUPERT_NBR; No++) 
  { sup = qgt->GetSuperTile(No);
    sup->GetLine(box);
  }
  return;
}
//------------------------------------------------------------------------------
//  Return windsock angle depending on wind force
//------------------------------------------------------------------------------
float C3Dworld::GetSockForce()
{ float spd = globals->wtm->WindSpeed();
  return wmap->Lookup(spd);
}
//=============================================================================
//
//  Class C3DPack for OSM objects located in a Supertile
//
//=============================================================================
C3DPack::C3DPack(U_INT qk)
{	next = prev = 0;
	qgKey	= qk;
	part	= 0;
}
//-----------------------------------------------------------------------------
//	Delete the pack
//-----------------------------------------------------------------------------
C3DPack::~C3DPack()
{
}
//-----------------------------------------------------------------------------
//	Extend this pack with the given part
//-----------------------------------------------------------------------------
void C3DPack::ExtendPart(OSM_Object *obj, SVector &T)
{	C3DPart *p0 = part;
	C3DPart *p1 = obj->GetPart(); 
	U_INT		tot	= p1->GetNBVTX();
  tot  += (p0)?(p0->GetNBVTX()):(0);
	C3DPart *np = new C3DPart();
	np->AllocateObjVTX(tot);
	np->ReceiveOSM(p1,1);
	np->Translate(T);
	np->ReceiveOSM(p0,0);
	//--- Delete original part ---------------
	if (p0)	delete p0;
	//--- Set texture reference and object ---
	char *txn = obj->TextureName();
	np->SetTexture(txn);
	part	= np;
	//--- Trace addition -----------------------
	U_INT stamp = obj->GetStamp();
	U_INT key   = obj->GetKey();
	U_INT qx    = key >> 16;
	U_INT qz    = key & 0xFFFF;
	U_INT spn   = obj->GetSupNo();
	U_INT nbv   = np->GetNBVTX();
	STREETLOG("ADD %05d to QGT(%03d-%03d) ST%02d: PART has %05d vertices",
			stamp, qx, qz, spn, nbv);
	return;
}
//========================END OF FILE ===================================================
