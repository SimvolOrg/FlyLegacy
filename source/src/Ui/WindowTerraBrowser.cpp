/*
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005 Chris Wallace
 * Copyright 2009 Jean Sabatier
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
#include "../Include/Fui.h"
#include "../Include/FuiParts.h"
#include "../Include/TerrainTexture.h"
#include "../Include/TerraFile.h"
//---------------------------------------------------------------------------------
extern char *HexTAB;
//=================================================================================
//  Window profile
//=================================================================================
#define TBROS_PROF (PROF_NO_INT + PROF_NO_EXT + PROF_NO_MET + PROF_DR_DET + PROF_RABIT)
#define MBROS_PROF (PROF_NO_INT + PROF_NO_EXT + PROF_NO_MET + PROF_DR_DET + PROF_RABIT)
//=================================================================================
//  Button text
//=================================================================================
char *ButTXT[] = {
  "ASSIGN to this TILE",
  "LOCKED in AUTO MODE",
};
//---------------------------------------------------------------------------------
char *ObjBTN[] = {
  "Hide Objects",
  "Show Objects",
};
//---------------------------------------------------------------------------------
char  btnLOK[] = {-1,+1};
//---------------------------------------------------------------------------------
char *OptTXT[] = {
  "AUTO OFF: You may change the tile type",
  "AUTO ON : The type change with aircraft moving",
};
//---------------------------------------------------------------------------------
char *AptBTN[] = {
  "Hide Airports",
  "Show Airports",
};
//=======================================================================================
//  CFuiTBROS  Terra Browser
//=======================================================================================
CFuiTBROS::CFuiTBROS(Tag idn, const char *filename)
:CFuiWindow(idn,filename,600,360,0)
{ Type  = -1;
  inf.mADR = 0;
  inf.xOBJ = 0;
  inf.res  = TC_HIGHTR;
  mode     = 1;
  strncpy(text,"TERRA BROWSER",255);
  title = 1;
  close = 1;
  zoom  = 0;
  mini  = 0;
	obtn	= 0;
	abtn	= 0;
  //----Create a label title ----------------------
  CFuiLabel *lab1 = new CFuiLabel(20,10,280, 20,this);
  lab1->SetText("TYPE OF TERRAIN TEXTURES");
  AddChild('lbl1',lab1);
  //----Create the List box -----------------------
  xWIN  =           new CFuiList (20,30,270,256,this);
  xWIN->SetVScroll();
  AddChild('xlst',xWIN);
  //----Create a Canvas ---------------------------
  xCNV  =           new CFuiCanva(320,30,256,256,this);
  AddChild('xcnv',xCNV);
  xCNV->EraseCanvas();
  //----Create Canvas Title -----------------------
  wLB1  =           new CFuiLabel(320,10,256,20,this);
  AddChild('wlb1',wLB1);
  //----Create RadioButton ------------------------
  wOPT  =           new CFuiCheckbox(20, 300,420,16,this);
  AddChild('wopt',wOPT,OptTXT[mode]);
  wOPT->SetState(mode);
  //----Create the button -------------------------
  wBTN  =           new CFuiButton(420,  300, 140,20,this);
  AddChild('wbtn',wBTN,ButTXT[mode]);
  //----Create Cancel button ----------------------
  zBTN  =           new CFuiButton(420,  326, 140,20,this);
  AddChild('zbtn',zBTN,"CANCEL");
  zBTN->Hide();
	//--- Create group edit -------------------------
	BuildGroupEdit(10,320);
  //----Create surface ----------------------------
  CFuiWindow::ReadFinished();
  aBOX = globals->tcm->GetTerraBox();
  aBOX->SetParameters(this,'xlst',LIST_DONT_FREE);
  //---Get a camera to draw texture ---------------
  Cam     = new CCameraSpot();
  //-----------------------------------------------
  lock    = 0;
  tFIL    = globals->tcm->GetTerraFile();
  //-----------------------------------------------
  aBOX->SortAndDisplay();
  int tex  = globals->tcm->GetGroundType();
  aBOX->GoToKey(&tex);
  GetSelection();
	//--- Set application profile -------------------
  ctx.prof	= TBROS_PROF;
	ctx.mode	= SLEW_RCAM;
  globals->ccm->SetRabbitCamera(ctx,this);

}
//-----------------------------------------------------------------------
//  Release any resource
//-----------------------------------------------------------------------
CFuiTBROS::~CFuiTBROS()
{ globals->noOBJ -= obtn;
  globals->noAPT -= abtn;
  if (inf.xOBJ) glDeleteTextures(1,&inf.xOBJ);
  if (inf.mADR) delete [] inf.mADR;
  globals->ccm->RestoreCamera(ctx);
  delete Cam;
}
//-----------------------------------------------------------------------
//  Get the selected texture slot
//-----------------------------------------------------------------------
void CFuiTBROS::GetSelection()
{ char name[PATH_MAX];
  CTgxLine *lin = (CTgxLine*)aBOX->GetSelectedSlot();
  int       ntp = lin->GetType();
  if (0 ==  lin)              return;
  if (ntp == Type)            return;
  Type  =   ntp;
  if (0 == Type)              return;
  //---Get the texture  ------------------------
  strncpy(name,"XX005D",(PATH_MAX-1));
  name[0] = HexTAB[Type >> TC_BY16];
  name[1] = HexTAB[Type &  TC_016MODULO];
  sprintf(inf.path,"SYSTEM/GLOBE/%s.RAW",name);
  CArtParser img(TC_HIGHTR);
  img.LoadTextureMT(inf);
  globals->txw->GetTexOBJ(inf,0,GL_RGBA);
  //--------------------------------------------
  if (0 == Type)  mode = 1;
  RefreshOPT();
  return;
}
//-----------------------------------------------------------------------
//  Update texture while moving
//-----------------------------------------------------------------------
void CFuiTBROS::UpdateGround()
{ char txt[256];
  globals->tcm->EditGround(txt);
  wLB1->SetText(txt);
  if (0 == mode)            return;
  int gnd = globals->tcm->GetGroundType();
  if (gnd == Type)          return;
  aBOX->GoToKey(&gnd);
  GetSelection();
  return;
}
//-----------------------------------------------------------------------
//  Cannot write on this install
//-----------------------------------------------------------------------
int CFuiTBROS::Error()
{ lock = 1;
  mode = 1;
  wOPT->SetState(mode);
  wOPT->SetText("LOCK: ImageType file cannot be modified on this installation");
  return 0;
}
//-----------------------------------------------------------------------
//  Change actual tile
//-----------------------------------------------------------------------
int CFuiTBROS::ChangeTile(int gnd)
{ if (mode == 1)                return 0;     // Locked     
  if (gnd  == 0)                return 0;     // User ground
  if (lock == 1)                return 0;     // previous error
  if (gnd  == Type)             return 0;     // same type
  //---Try to change the tile type -------------------
  U_INT ax; 
  U_INT az; 
  char  tp = Type;
  globals->tcm->GetAbsoluteIndices(ax,az);
  if (!tFIL->Write(ax,az,&tp))  return Error();
  chng      = gnd;                   // Save change
  this->sx  = ax;
  this->sz  = az;       
  zBTN->Show();
  //---Enter a log -----------------------------------
  int qx  = (ax >> TC_BY32);        // QGT  X index
  int qz  = (az >> TC_BY32);        // QGT  Z index
  int tx  = ax & TC_032MODULO;      // Tile X index
  int tz  = az & TC_032MODULO;      // Tile Z index
  TERRA("   QGT(%03d-%03d) DET(%02d-%02d) Type was %03d  Changed for %03d",
        qx,tx,qz,tz,gnd,int(tp));
  //---Refresh texture load --------------------------
  C_QGT     *qgt = globals->tcm->GetPlaneQGT();
  if (qgt)   qgt->PutOutside();
  return 1;
}
//-----------------------------------------------------------------------
//  Restore Tile
//-----------------------------------------------------------------------
int  CFuiTBROS::RestoreTile(int gnd)
{ if (0 == chng)                  return 0;
  if (!tFIL->Write(sx,sz,&chng))  return Error();
  //--- Restoration is OK ----------------------------
  zBTN->Hide();
  //---Enter a log -----------------------------------
  int qx  = (sx >> TC_BY32);        // QGT  X index
  int qz  = (sz >> TC_BY32);        // QGT  Z index
  int tx  = sx & TC_032MODULO;      // Tile X index
  int tz  = sz & TC_032MODULO;      // Tile Z index
  TERRA("   QGT(%03d-%03d) DET(%02d-%02d) Type was %03d  Restored to %03d",
        qx,tx,qz,tz,gnd,chng);
  //--Refresh view if position not moved --------------
  U_INT ax;
  U_INT az;
  globals->tcm->GetAbsoluteIndices(ax,az);
  if ((ax != sx) || (az != sz)) return 0;
  C_QGT     *qgt = globals->tcm->GetPlaneQGT();
  if (qgt)   qgt->PutOutside();
  return 1;
}
//-----------------------------------------------------------------------
//  Refresh option
//-----------------------------------------------------------------------
void CFuiTBROS::RefreshOPT()
{   wOPT->SetState(mode);
    wOPT->SetText(OptTXT[mode]);
    wBTN->SetText(ButTXT[mode]);
    return;
}
//-----------------------------------------------------------------------
//  Notifications
//-----------------------------------------------------------------------
void  CFuiTBROS::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ int gnd = globals->tcm->GetGroundType(); 
  switch (idm)  {
  case 'sysb':
    SystemHandler(evn);
    return;
  case 'xlst':
    if (1 == mode)    return;
    aBOX->VScrollHandler((U_INT)itm,evn);
    GetSelection();
    return;
  case 'wopt':
    mode    = (gnd)?(itm):(1);
    mode   |= lock;
    RefreshOPT();
    return;
  case 'wbtn':
    ChangeTile(gnd);
    return;
  case 'zbtn':
    RestoreTile(gnd);
    return;
  case 'obtn':
		obtn ^= 1;
		gBox->SetChildText('obtn',ObjBTN[obtn]);
    globals->noOBJ += btnLOK[obtn];
    return;
  case 'abtn':
		abtn ^= 1;
		gBox->SetChildText('abtn',AptBTN[abtn]);
    globals->noAPT += btnLOK[abtn];
    return;

  }
  return;
}
//---------------------------------------------------------------------------------
//  Draw Window
//---------------------------------------------------------------------------------
void CFuiTBROS::Draw()
{ //---Refresh texture if changed -------------------------
  VIEW_PORT vp;
  xCNV->GetViewPort(vp);
  UpdateGround();
  CFuiWindow::Draw();
  Cam->Projection(vp,inf.xOBJ);
}
//===========================================================================================
//
//  Object Browser
//
//===========================================================================================
#define MBROS_WID (336)
#define MBROS_HTR (252)
#define MBROS_HLF (MBROS_WID / 2)
CFuiMBROS::CFuiMBROS(Tag idn, const char *filename)
:CFuiWindow(idn,filename,360,730,0)
{ strncpy(text,"3D-MODELS BROWSER",255);
  title = 1;
  close = 1;
  zoom  = 0;
  mini  = 0;
  Mod   = 0;
  //---Allocate camera object -----------------------------
  oCam   = new CCameraObject();
  oCam->SetFBO(MBROS_WID,MBROS_HTR);
  oCam->MoveMouseIs(1);
  //--- Create the position labels -------------------------
  mPOS  =           new CFuiLabel (8,10,MBROS_WID, 18,this);
  AddChild('mpos',mPOS);
  //----Create Display Canvas  ----------------------------
  oDSP  =           new CFuiCanva (8,30,MBROS_WID,MBROS_HTR,this);
  AddChild('odsp',oDSP);
  oDSP->EraseCanvas();
  oDSP->RegisterCamera(oCam);
  //----Create Zoom Slider ---------------------------------
  zSLD  =           new CFuiSlider(8,300,MBROS_WID, 30,this); 
  AddChild('zsld',zSLD);
  //----Create Zoom Label ----------------------------------
  zLAB  =           new CFuiLabel (8,340,MBROS_WID, 20,this);
  AddChild('zlab',zLAB);
  //----Create Canva ---------------------------------------
  oINF  =           new CFuiCanva (8,370,MBROS_WID, 30,this);
  AddChild('oinf',oINF);
  //----Create the List box --------------------------------
  oWIN  =           new CFuiList  (8,440,MBROS_WID,200,this);
  oWIN->SetVScroll();
  AddChild('olst',oWIN);
  //---Create Total label ---------------------------------
  wTOT =            new CFuiLabel (8,650,MBROS_WID, 16,this);
  AddChild('wtot',wTOT);
  //---Create Object origin -------------------------------
  wOrg =            new CFuiLabel (8,670,MBROS_WID, 16,this);
  AddChild('worg',wOrg);
	//---ZB option ------------------------------------------
  wZbo  = new CFuiCheckBox(8,  690, 300,20,this);
  wZbo->IniState(0);
  AddChild('zbop',wZbo,"Flat object on ground (No ZB)");
  //----Create surface for windows ------------------------
  CFuiWindow::ReadFinished();
  //----Create object list --------------------------------
  Lin  = 0;
  wObj = 0;
  oBOX.SetParameters(this,'olst',0);
  globals->tcm->GetObjLines(&oBOX);
  oBOX.SortAndDisplay();
  //---Allocate a camera orbit ----------------------------
  sCam = new CCameraOrbit();
	//---Create application profile -------------------------
	ctx.prof	= MBROS_PROF;
	ctx.mode	= SLEW_RCAM;
  globals->ccm->SetRabbitCamera(ctx,this);			// Change camera
	//---Set focus on first object --------------------------
	NewSelection();
  ChangeZoom();
}
//--------------------------------------------------------------------------------
//  Free all resources
//--------------------------------------------------------------------------------
CFuiMBROS::~CFuiMBROS()
{ globals->wObj   = 0;
  oDSP->RegisterCamera(0);
  if (oCam) delete oCam;
  globals->m3d->ReleaseVOR();
  globals->ccm->RestoreCamera(ctx);
  delete sCam;
}
//--------------------------------------------------------------------------------
//  Compute Camera object position relative to object size
//--------------------------------------------------------------------------------
void CFuiMBROS::SetZoom()
{ double ang = DegToRad(double(16));
  double hm  = Mod->MaxExtend() * 0.5;
  double dr  = hm / tan(ang);
  if (dr < 50)  dr = 50;
  double rat = oCam->SetRange(dr) * 100;
  oCam->Refresh();
  zSLD->SetValue(rat);
  ChangeZoom();
  return;
}
//--------------------------------------------------------------------------------
//  Change Zoom value
//--------------------------------------------------------------------------------
void CFuiMBROS::ChangeZoom()
{ double rat = zSLD->GetValue() * 0.01;
  double dst = oCam->ZoomBy(rat);
  double nml = FN_MILE_FROM_FEET(dst);
  zLAB->EditText("View from distance %04d feet (%.02f mile)",int(dst),nml);
  return;
}
//--------------------------------------------------------------------------------
//  New Selection
//  -Edit Total
//  -Extract model to draw
//--------------------------------------------------------------------------------
void CFuiMBROS::NewSelection()
{ char lat[32];
  char lon[32];
  //----Edit total objects -------------------------
  wTOT->EditText("Total objects found: %d",oBOX.GetSize());
  CObjLine *lin = (CObjLine*)oBOX.GetSelectedSlot();
  if ((Lin == lin) || (0 == lin)) return;
  //----Set a new world object ---------------------
  Lin  = lin;
  wObj = 0;
  CWobj *obj = lin->GetWOBJ();
  Mod  = obj->GetDayModel();
  wObj = obj;
  globals->wObj = obj;
  //----Edit object position -----------------------
  SPosition pos = obj->GetPosition();
  EditLat2DMS(pos.lat,lat);
  EditLon2DMS(pos.lon,lon);
  mPOS->EditText("POS: %s %s at=%.0f feet",lat,lon,pos.alt);
	wOrg->EditText("Origin: %s",obj->GetFileOBJ());
	wZbo->SetState(obj->GetZB());
  //----Edit object model data ---------------------
  oINF->BeginPage();
  obj->GetInfo(oINF);
  //--- Set Camera object position -----------------
  SetZoom();
  Teleport();
  return;
}
//--------------------------------------------------------------------------------
//  Teleport to object
//  Compute camera range depending on object extension
//--------------------------------------------------------------------------------
void CFuiMBROS::Teleport()
{ if (0 == wObj) return;
  SPosition pos = wObj->GetPosition();
  pos.alt += (wObj->GetZExtend() + 20) * 0.5;
  //----Set Camera parameters  -----------------
  double ex = wObj->GetYExtend() * 12;
  sCam->SetRange(ex);
  sCam->SetAngle(15,30);
  globals->m3d->ReleaseVOR();
  globals->pln->SetPosition(pos);
  return;
}
//--------------------------------------------------------------------------------
//  ZB option is changed
//--------------------------------------------------------------------------------
void CFuiMBROS::ChangeZB()
{	if (0 == wObj)						return;
	char zb = wZbo->GetState();
	wObj->SetNOZB(zb);
	if (0 == globals->objDB)	return;
	globals->sqm->UpdateOBJzb(wObj);
	return;
}
//--------------------------------------------------------------------------------
//  Draw
//--------------------------------------------------------------------------------
void CFuiMBROS::Draw()
{ CFuiWindow::Draw();
  VIEW_PORT vp;
  oDSP->GetViewPort(vp);
  oCam->DrawOnWin(vp,this);
  return;
}
//--------------------------------------------------------------------------------
//  Draw by camera
//  Draw only LOD (level of Detail) 0
//--------------------------------------------------------------------------------
void CFuiMBROS::DrawByCamera(CCamera *cam)
{ if (0 == Mod) return;
  Mod->PreDraw(cam);
  Mod->Draw(BODY_NOMOVE);                   
  Mod->EndDraw(cam);
  return;
}
//-----------------------------------------------------------------------
//  Notifications
//-----------------------------------------------------------------------
void  CFuiMBROS::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ 
  switch (idm)  {
    case 'sysb':
      SystemHandler(evn);
      return;
		//--- Zoom is changing ------------------
    case 'zsld':
      ChangeZoom();
      return;
		//--- A new selection is occuring -------
    case 'olst':
      oBOX.VScrollHandler((U_INT)itm,evn);
      NewSelection();
      return;
		//--- The ZB option is modified ---------
		case 'zbop':
			ChangeZB();
			return;
  }
  return;
}
//===========END OF FILE ====================================================================

