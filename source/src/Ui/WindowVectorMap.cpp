/*
 * WindowVectorMap.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005 Chris Wallace
 * Copyright 2007 Jean Sabatier
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

/*! \file WindowVectorMap.cpp
 *  \brief Implements FUI callback for vector map window
 *
 *  Implements the FUI callback for the Vector Map window.
 */

#include "../Include/FlyLegacy.h"
#include "../Include/Utility.h"
#include "../Include/database.h"
#include "../Include/Globals.h"
#include "../Include/Fui.h"
#include "../Include/FuiUser.h"
#include "../Include/FuiParts.h"
#include "../Include/Taxiway.h"
#include "../Include/FreeImage.h"
#include "../Include/TerrainTexture.h"
#include "../Include/PlanDeVol.h"
//=================================================================================
#define VMAP_DOC_DRAG 1
#define VMAP_RWY_DRAG 2
#define VMAP_WPT_DRAG 3
//=================================================================================
//  Window profile when in flight plan edit mode
//=================================================================================
#define VMP_PROF (PROF_NO_INT+PROF_NO_EXT+PROF_NO_MET+PROF_RABIT+PROF_DRAWRB)
//=======================================================================
//  VECTOR WINDOW STATE
//=======================================================================
#define VWIN_MAP 0              // Normal MAP View
#define VWIN_DOC 1              // Document view
#define VWIN_LST 2              // Document list
#define VWIN_RWY 3              // Runway view
#define VWIN_WPT 4							// Waypoint
//=======================================================================
//  Next state table
//=======================================================================
char vmapNSTAT[] = 
{	VWIN_MAP,						// 0 => 0
	VWIN_DOC,						// 1 => 1
	VWIN_LST,						// 2 => 2
	VWIN_RWY,						// 3 => 3
	VWIN_MAP,						// 4 => 0
};
//=======================================================================
//  MENU FOR VECTOR MAP
//=======================================================================
char  DocPILE[16 * 64];
char *DocMENU[] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,""};
char  WptALTI[128];									// Waypoint altitude
char *WptMENU[] = {"IGNORE",WptALTI,0, 0,0,0};
char *noDOC = "No document";
char *addWPT=	"Add a waypoint to Flight Plan" ;
//=======================================================================
//	Define MENU ENTRIES
//=======================================================================
#define MENU_SLOT00	(0)
#define MENU_SLOT01 (1)
#define MENU_SLOT03 (3)
#define MENU_SLOT05 (5)
#define MENU_SLOT06 (6)
//=======================================================================
//  NAV MENU
//=======================================================================
char *NavMENU[] =
{ 0,                                // 0 Full name
  0,                                // 1= Distance to cursor
  "DETAIL",                         // 2 Detail
  "ADD to FLIGHT PLAN",             // 3 Add to FPL
  "TELEPORT",                       // 4 Teleport to
  "-----Tune to -------------",     // 5 Separator
  0,                                // 6 Frequency
  "RETURN",                         // 7 Return
  0,                                // End
};
//=======================================================================
//  WPT MENU
//=======================================================================
char  wayNAME[128];									// Waypoint name
char *WayMENU[] =
{	wayNAME,													// 0 Name
	0,																// 1 distance next
	"RETURN",
	0,
};
//=======================================================================
//  AIRPORT MENU
//=======================================================================
char *AptMENU[] = 
{ 0,                                // 0 = Name
  0,                                // 1 = Distance item
  "OPEN DOCUMENTs",                 // 2 = Document item
  "ADD to FLIGHT PLAN",             // 3 = Add to flight plan
  "TELEPORT",                       // 4 = Teleport to ...
  "METAR",                          // 5 = METAR
  0,                                // 6 = View Runway
  0,                                // 7 = Separator
  0,                                // 8 = ATHIS
  0,                                // 9 = ILS 1
  0,                                //10 = ILS 2
  0,                                //11 = ILS 3
  0,                                //12 = ILS 4
  0,                                //13 = ILS 5
  0,                                //14 = ILS 6
  0,                                //15 = ILS 7
  0,                                //16 = ILS 8
  0,                                //17 = ILS 9
  0,                                //18 = ILS10
  0,                                //19 = ILS11
  0,                                //20 = RETURN
  0,                                //21 = 0 (END)
};
//=======================================================================
//  AIRPORT VARIATION
//=======================================================================
char *AptVAR1 = "ADD to FLIGHT PLAN";
char *AptVAR2 = "-----------------------------";
//=======================================================================
//	Runway idents
//=======================================================================
char rwyIDN[16 * 8] = {0};
//=======================================================================
//  MENUS FOR LIGHT PROFILE
//  Changing item here must also change TC_APR_XXXX definitions in LightSystem.h
//=======================================================================
char *apMENU[] = {
  "none",             // TC_APR_NULL       
  "REIL",             // TC_APR_REIL
  "ODALS",            // TC_APR_ODAL
  "SSALR",            // TC_APR_ALSR
  "ALSF-1",           // TC_APR_ALF1
  "ALSF-2",           // TC_APR_ALF2
  "",
};
//=======================================================================
char *tbMENU[] = {
  "none",
  "Green only",
  "Red only",
  "Red & Green",
  "inop",
  "",
};
//=======================================================================
char *wbMENU[] = {
  "none",
  "yes",
  "",
};
//=======================================================================
char *ctMENU[] = {
  "none",
  "mono",
  "instrument",
  "",
};
//=======================================================================
char *lkMENU[] = {
  "Locked",
  "modify",
  "",
};
//=======================================================================
//List of radios
//=======================================================================
RADIO_LST RadioLST =
{ 0,
  12,
  0,
{{0,0,},
 {0,0},
 {0,0},
 {0,0},
 {0,0},
 {0,0},
 {0,0},
 {0,0},
 {0,0},
 {0,0},
 {0,0},
 {0,0},
},              // Radio 0
};
//=======================================================================
//  MASK VMAP = f(QTYPE)
//=======================================================================
U_INT maskOPT[] =
 {	0,					                      // 0 Tile cache
	  (VM_APTN | VM_APTI),					    // 1 Airport
    (VM_VORN | VM_VORI | VM_VORF),    // 2 VOR
    (VM_NDBN | VM_NDBI ),             // 3 NDB
		0,																// 4 ILS
		0,																// 5 COM
		0,																// 6 RWY
		VM_NDBI,													// 7 WPT
		0,																// 8 OBN
};
//=======================================================================
//  ILS VECTOR DRAWING
//                                  
//=======================================================================
TC_VTAB     tabILS[] = {
  {0,0,   0,    0,0},             // 0 origin
  {0,0, +10,  20,0},              // 1 right arrow 1
  {0,0,  +5,  20,0},              // 2 Right arrow 2
  {0,0,  +5, 100,0},              // 3 Right extremity
  {0,0,  -5, 100,0},              // 4 Left extremity
  {0,0,  -5,  20,0},              // 5left arrow 2
  {0,0, -10,  20,0},              // 6 Left arrow 1
  {0,0,   0,   0,0},              // 7 origin
  };  // ILS map
//=======================================================================
//  Global function to add a doc name
//=======================================================================
int VMapCB(char *dn, void *upm)
{	CFuiVectorMap *win = (CFuiVectorMap*)upm;
	return win->AddDocName(dn);	}
//=======================================================================
//  JSDEV* CFuiVectorMap constructor
//  NOTES:
//  vmapZoom :50 nautical miles for 800 pixels at startup from globals
//=======================================================================
CFuiVectorMap::CFuiVectorMap( Tag windowId, const char* winFilename)
: CFuiWindow( windowId, winFilename,
             globals->vmapScrn.x,
             globals->vmapScrn.y)
{ type = COMPONENT_VECTOR_MAP;
  widgetTag = 'defa';
  wName = "VectorMap";
  dbc   = globals->dbc;
  fpln  = 0;
  Focus = 0;
  //----- Initialize vector map status -------------------
  vmapOrient.p = vmapOrient.h = vmapOrient.r = 0;
  RoseZm    = 1.0f;;
  Scale(globals->vmapZoom);
  //---Init all colors -----------------------------------
  green   = 0;
  grlim   = (8 * 255);
  white   = MakeRGBA (255, 255, 255, 255);
  red     = MakeRGBA (255,   0,   0, 255);
  yel     = MakeRGBA (  0, 255, 255, 255);
  black   = globals->vmapTrns;
  cLab[0] = white;
  cLab[1] = yel;
  Fade    = 1;                          // Increase green color
  //-------------Init font label -------------------------
  fLab[0] = (CMonoFontBMP*)(globals->fonts.ftmono8.font);
  fLab[1] = (CMonoFontBMP*)(globals->fonts.ftmono20.font);
  Intv[0] = fLab[0]->MaxCharWidth();
  Intv[1] = fLab[1]->MaxCharWidth();
  //------------------------------------------------------
  Frame   = 0;
  Turn    = 0;
  dStat   = VWIN_MAP;
  //-----Init rose group box -----------------------------
  rBox = (CFuiGroupBox*)GetComponent('rose');
  if (0 == rBox)  gtfo("Incorrect FUI vector map file");
  rBox->MoveParentTo(this->x,this->y);
  RelocateBox(rBox,h);
  rBox->SetProperty(FUI_VT_RELOCATE);
	rBox->ColorText(white);
  //-----Init zoom group box -----------------------------
  zBox = (CFuiGroupBox*)GetComponent('zoom');
  if (0 == zBox)  gtfo("Incorrect FUI vector map file");
  zBox->MoveParentTo(this->x,this->y);
  RelocateBox(zBox,h);
  zBox->SetProperty(FUI_VT_RELOCATE);
	zBox->ColorText(white);
  //-----Init background group box -----------------------
  bBox = (CFuiGroupBox*)GetComponent('back');
  if (0 == bBox)  gtfo("Incorrect FUI vector map file");
  bBox->MoveParentTo(this->x,this->y);
  RelocateBox(bBox,h);
  bBox->SetProperty(FUI_VT_RELOCATE);
	bBox->ColorText(white);
  //-----Init background group box -----------------------
  pBox = (CFuiGroupBox*)GetComponent('pbox');
  if (0 == pBox)  gtfo("Incorrect FUI vector map file");
  pBox->MoveParentTo(this->x,this->y);
  RelocateBox(pBox,h);
  pBox->SetProperty(FUI_VT_RELOCATE);
	pBox->ColorText(white);
	pBox->Hide();
	//------ Properties ------------------------------------
	SetXRange(100,1024);               // width
	SetYRange( 40,800);
  //------------------------------------------------------
  geop.lon  = 0;
  geop.lat  = 0;
	geop.alt  = 0;
	rCAM			= 0;
	snode			= 0;
  //------------------------------------------------------
  AddZoomButton();
  AddMiniButton();
  SetTransparentMode();
  //---Set default drawing --------------
  option    = globals->vmpOpt;
  //-----------Init floating menu ------------------------
  smen.NbCar    = 20;
  MyPop         = 0;
  //----------Init screen table --------------------------
  sTable.NbSlot = 0;
  sTable.xSlot  = sTable.sList;
  pthread_mutex_init (&sTable.mux,  NULL);
  //-----Load Stock bitmap -------------------------------
  OthBMAP[0]			= 0; 
	OthBMAP[VOTHER]	= 0;																		// Unknown navaid
  LoadOthBitmap("ART/MAPICONX.PBG",   USERVEH);           // User vehicle 32 frames
  LoadOthBitmap("ICONS/BM_NDBDM.BMP", NDBDME);
  LoadOthBitmap("ICONS/BM_NDBNN.BMP", NDB___);
  LoadOthBitmap("ICONS/BM_VORTA.BMP", VORTAC);
  LoadOthBitmap("ICONS/FP-ICO03.BMP", TACAN_);
  LoadOthBitmap("ICONS/FP-ICO04.BMP", VORDME);
  LoadOthBitmap("ICONS/FP-ICO05.BMP", VOR___);
  LoadOthBitmap("ICONS/FP-ICOW1.BMP", WPTRED);
  LoadOthBitmap("ICONS/FP-ICOW2.BMP", WPTORA);
  LoadOthBitmap("ICONS/FP-ICOW3.BMP", WPTYEL);
  LoadOthBitmap("ICONS/FP-ICOW4.BMP", WPTGRN);
  LoadOthBitmap("ICONS/FP-ICOWP.BMP", WPTBLK);
	LoadOthBitmap("ICONS/BM_TNODE.BMP", TXNODE);
  //----------Init Airport stack ---------------------------
  LoadAptBitmap("ICONS/FP-ICO06.BMP",TYP01);
  LoadAptBitmap("ICONS/FP-ICO07.BMP",TYP02);
  LoadAptBitmap("ICONS/FP-ICO08.BMP",TYP03);
  LoadAptBitmap("ICONS/FP-ICO09.BMP",TYP04);
  LoadAptBitmap("ICONS/FP-ICO10.BMP",TYP05);
  LoadAptBitmap("ICONS/FP-ICO11.BMP",TYP06);
  LoadAptBitmap("ICONS/FP-ICO12.BMP",TYP07);
  LoadAptBitmap("ICONS/FP-ICO13.BMP",TYP08);
  LoadAptBitmap("ICONS/FP-ICO14.BMP",TYP09);
  LoadAptBitmap("ICONS/FP-ICO15.BMP",TYP10);
  //----------------------------------------------------------
	txDATA	= 0;
  ilsBMP  = 0;
  //----------------------------------------------------------
  InitMenu();
  //----------------------------------------------------------
  DocInfo.rgba = 0;
  xMini = 370;
  //----------------------------------------------------------
  Cam = globals->cap;
  //--------Radio interface ----------------------------------
  mesg.id        = MSG_SETDATA;
  mesg.user.u.hw = HW_RADIO;
  aILS           = 0;
}
//--------------------------------------------------------------------------------
// Relocate Groupe Box
//--------------------------------------------------------------------------------
void CFuiVectorMap::RelocateBox(CFuiGroupBox *box,short ht)
{ int ry = ht - box->GetHeight();
  int rx = box->GetXPosition() + 6;
  box->MoveTo(rx,ry);
  return;
}
//---------------------------------------------------------------
//  Load Airport Bitmap
//  Fill AptSIZE with half bitmap size
//---------------------------------------------------------------
void  CFuiVectorMap::LoadAptBitmap(char *bmp,VM_ANB no)
{ int wd;
  int ht;
  AptBMAP[no] = new CBitmap(bmp);
  AptBMAP[no]->GetBitmapSize(&wd, &ht);
  AptSIZE[no].w2 = (U_SHORT)(wd >> 1);
  AptSIZE[no].h2 = (U_SHORT)(ht >> 1);
  InitScreenRect(wd,ht);
  return;
}
//---------------------------------------------------------------
//  Load Stock Bitmap
//  Fill PolSIZE with half bitmap size
//---------------------------------------------------------------
void  CFuiVectorMap::LoadOthBitmap(char *bmp,VM_BMP no)
{	int wd;
  int ht;
  OthBMAP[no] = new CBitmap(bmp);
  OthBMAP[no]->GetBitmapSize(&wd, &ht);
  OthSIZE[no].w2 = (U_SHORT)(wd >> 1);
  OthSIZE[no].h2 = (U_SHORT)(ht >> 1);
  return;
}
//------------------------------------------------------------
CFuiVectorMap::~CFuiVectorMap (void)
{ LeaveModePlan();
	//---Clear the cache screen table at exit ----------------       
  ResetScreenTable();
  if (MyPop)  delete MyPop;
  //----waypoints --------------------
  for (int k=1; k<BM_NBR; k++) 
  { CBitmap  *bmp  = OthBMAP[k];
    if (bmp)  delete bmp;
  }
  //----Airports --------------------
  for (int k=1; k<APNBR; k++) 
  { CBitmap  *bmp  = AptBMAP[k];
    if (bmp)  delete bmp;
  }
  //----IMAGE ----------------------
  if (DocInfo.rgba)  delete [] DocInfo.rgba;
  DocInfo.rgba = 0;
	//--- Taxiways -------------------
	if (txDATA)  delete txDATA;
	//--- Flight plan ----------------
	fpln = globals->pln->GetFlightPlan();
	fpln->Save();
}
//------------------------------------------------------------------------------
//  Init Menu items
//-------------------------------------------------------------------------------
void  CFuiVectorMap::InitMenu (void)
{ SMenuPart * mp;
  if (0 == mBar)      return;
  //------------------------------------------------
  mp  = mBar->GetMenuPart('airp','drwa');
  if (mp) mp->Check = option.Has(VM_DAPT);
  mp  = mBar->GetMenuPart('airp','nama');
  if (mp) mp->Check = option.Has(VM_APTN);
  mp  = mBar->GetMenuPart('airp','idsa');
  if (mp) mp->Check = option.Has(VM_APTI);
  mBar->GetMenu('airp')->Initialize();
  //-------------------------------------------------
  mp  = mBar->GetMenuPart('naid','drwn');
  if (mp) mp->Check = option.Has(VM_DNDB);
  mp  = mBar->GetMenuPart('naid','namn');
  if (mp) mp->Check = option.Has(VM_NDBN);
  mp  = mBar->GetMenuPart('naid','idsn');
  if (mp) mp->Check = option.Has(VM_NDBI);
  //-------------------------------------------------
  mp  = mBar->GetMenuPart('naid','drwv');
  if (mp) mp->Check = option.Has(VM_DVOR);
  mp  = mBar->GetMenuPart('naid','namv');
  if (mp) mp->Check = option.Has(VM_VORN);
  mp  = mBar->GetMenuPart('naid','idsv');
  if (mp) mp->Check = option.Has(VM_VORI);
  mp  = mBar->GetMenuPart('naid','frqn');
  if (mp) mp->Check = option.Has(VM_VORF);
  mBar->GetMenu('naid')->Initialize();
  //-------------------------------------------------
  mp  = mBar->GetMenuPart('opts','clin');
  if (mp) mp->Check = option.Has(VM_DCST);
  mp  = mBar->GetMenuPart('opts','plat');
  if (mp) mp->Check = option.Has(VM_DROS);
  mp  = mBar->GetMenuPart('opts','labl');
  if (mp) mp->Check = option.Has(VM_DLAB);
  mp  = mBar->GetMenuPart('opts','ilsd');
  if (mp) mp->Check = (1);
  //------------------------------------------------
  mBar->GetMenu('opts')->Initialize();
  return;
}

//-------------------------------------------------------------------------------
//    Compute Pixels cnter for bitmap and base line for label
//    scale is 50-200 miles for 800 pixels. vmapZoom is given in nm
//  NOTE: vmapzoom is used as distance argument to get item from Data base Cache
//-------------------------------------------------------------------------------
int CFuiVectorMap::GetScreenCoordinates (CmHead *obj, int& sx, int& sy, int ybm, int ofs)
{ // Calculate lat/lon difference 
  int yLat = obj->GetDistLat();
  int xLon = obj->GetDistLon();
  // Convert lat/lon distance to pixels on the vector map (xLat and xLon are multiplyed by 128
  sx =         (xLon * pixNM) + halfW;
  sy = h - 1 -((yLat * pixNM) + halfH);
  // Set y offset for label baseline depending on North or South position from aircraft
  return (sy < halfH)?(sy + ybm + ofs):(sy - 14 - ybm - ofs);
}
//-----------------------------------------------------------------------------
//  Clip any object outside view rectangle
//-----------------------------------------------------------------------------
bool CFuiVectorMap::ClipOBJ(int vx,int vy,int bx, int by)
{ if (vx <= -bx )               return true;
  if (vx >= (w + bx))           return true;
  if (vy <= -by)                return true;
  if (vy >= (h + by))           return true;
  return false;
}
//-----------------------------------------------------------------------------
//  CFuiVectorMap  DRAW label
//-----------------------------------------------------------------------------
void CFuiVectorMap::DrawLabel(int fnt,int x,int y,CmHead *obj)
{ U_INT No  = obj->GetActiveQ();
  if (No > OBN) No = 0;
  U_INT msk = maskOPT[No];              // Get mask related to object;
  U_INT prp = option.Get(msk);
  if (prp & VM_IDEN) x = fLab[fnt]->DrawNText(surface,x, y, cLab[fnt], obj->GetIdent()) + Intv[fnt];
  if (prp & VM_FREQ) x = fLab[fnt]->DrawNText(surface,x, y, cLab[fnt], obj->GetEFreq()) + Intv[fnt];
  if (prp & VM_NAME) x = fLab[fnt]->DrawNText(surface,x, y, cLab[fnt], obj->GetName())  + Intv[fnt];
  return;
}
//-----------------------------------------------------------------------------
//  CFuiVectorMap  DRAW locals NDB
//-----------------------------------------------------------------------------
void  CFuiVectorMap::DrawNDB(void)
{ CNavaid *ndb      = 0;
  CBitmap* bmp;
  int xIcon; 
  int yIcon;
  int xbm   = 0;
  int ybm   = 0;
  short fnt = 0;
  int xtype = 0;
  int  ylab = 0;
  for (ndb  = dbc->FindNDBbyDistance(ndb,MaxNM); ndb != 0; ndb = dbc->FindNDBbyDistance(ndb,MaxNM))
  { xtype = ndb->GetIndexType();
    bmp   = OthBMAP[xtype];
    xbm   = OthSIZE[xtype].w2;
    ybm   = OthSIZE[xtype].h2;

    fnt   = (Focus == ndb)?(1):(0);
    ylab  = GetScreenCoordinates (ndb,xIcon, yIcon, ybm, 0);
    if (ClipOBJ(xIcon,yIcon,xbm,ybm)) continue;
    bmp->DrawBitmap (surface,(xIcon - xbm), (yIcon- ybm), 0);
    if (option.Has(VM_DLAB)) DrawLabel(fnt,xIcon,ylab,ndb);
    EnterScreenTable(ndb,(xIcon + x),(yIcon + y));
  }
  return;
}
//-----------------------------------------------------------------------------
//  Draw 10° and 5° Markers
//-----------------------------------------------------------------------------
void  CFuiVectorMap::DrawMark(int type,int xc, int yc, U_INT col)
{ int   inx = 0;
  int   x0, y0;
  int   x1 = 0;
  int   y1 = 0;
  while (inx < 9)
  {     mRose.GetMarker(type,inx,x0,y0,x1,y1);
        DrawFastLine(surface,(xc + x0), (yc + y0), (xc + x1), (yc + y1), col);
        DrawFastLine(surface,(xc - x0), (yc - y0), (xc - x1), (yc - y1), col);
        DrawFastLine(surface,(xc + y0), (yc - x0), (xc + y1), (yc - x1), col);
        DrawFastLine(surface,(xc - y0), (yc + x0), (xc - y1), (yc + x1), col);
        inx++;
  }
  //------draw the north direction ------------------------------
  mRose.Get00D(x1,y1);
  if (type == 10) DrawFastLine(surface,xc, yc, (xc + x1), (yc + y1), col);
  return;
}
//-----------------------------------------------------------------------------
//  Draw Rose text Radial 
//-----------------------------------------------------------------------------
void CFuiVectorMap::DrawRadial(CNavaid *vor,int xc, int yc)
{ float  lat  = float(vor->GetDistLat()) / 128;
  float  lon  = float(vor->GetDistLon()) / 128;
  float  dst  = SquareRootFloat((lat *lat)+(lon *lon));
  if (dst < 1)  return;
  int    szp    = RoseRd + 12;
  float  fac    = (float(szp) / dst); 
  int     xp    = +int(fac * lon);
  int     yp    = -int(fac * lat);
  DrawRect(surface,(xc+xp-11),(yc+yp-7),(xc+xp+12), (yc+yp+7),red);
  FillRect(surface,(xc+xp-10),(yc+yp-6),(xc+xp+11), (yc+yp+6),black);
  float   ang   = (lon == 0)?(0):(atan(lat / lon));
  float   cmp   = (lon >  0)?(PI / 2):((3 * PI) / 2);
  float   dev   = DegToRad(vor->GetMagDev());
  float   rad   = WrapTwoPi(cmp - ang - dev);
  int     deg   = Round(rad * RAD2DEG);
  char    txt[8];
  sprintf(txt,"%u",deg);
  CFont *fnt = (CFont*)globals->fonts.ftasci10.font;
  fnt->DrawNText (surface,(xc+xp-7), (yc+yp-7), white, txt);
  return;
}
//-----------------------------------------------------------------------------
//  Draw the Rose direction label
//-----------------------------------------------------------------------------
void CFuiVectorMap::DrawRoseDir(int xc, int yc,U_INT col)
{ int x1,y1;
  CMonoFontBMP *fn = fLab[0];
  mRose.Get00D(x1,y1);
  fn->DrawNText (surface, (xc + x1),     (yc + y1 - 16), col, "N");
  mRose.Get03D(x1,y1);
  fn->DrawNText (surface, (xc + x1 + 4), (yc + y1 - 14), col, "3");
  mRose.Get06D(x1,y1);
  fn->DrawNText (surface, (xc + x1 + 4), (yc + y1 - 10), col, "6");
  mRose.Get09D(x1,y1);
  fn->DrawNText (surface, (xc + x1 + 6), (yc + y1 -  8), col, "E");
  mRose.Get12D(x1,y1);
  fn->DrawNText (surface, (xc + x1 + 4), (yc + y1 -  2), col, "12");
  mRose.Get15D(x1,y1);
  fn->DrawNText (surface, (xc + x1 + 2), (yc + y1 +  2), col, "15");
  mRose.Get18D(x1,y1);
  fn->DrawNText (surface, (xc + x1 - 4), (yc + y1 +  4), col, "S");
  mRose.Get21D(x1,y1);
  fn->DrawNText (surface, (xc + x1 - 12), (yc + y1    ), col, "21");
  mRose.Get24D(x1,y1);
  fn->DrawNText (surface, (xc + x1 - 14), (yc + y1 - 2), col, "24");
  mRose.Get27D(x1,y1);
  fn->DrawNText (surface, (xc + x1 - 12), (yc + y1 -  8), col, "W");
  mRose.Get30D(x1,y1);
  fn->DrawNText (surface, (xc + x1 - 12), (yc + y1 - 10), col, "30");
  mRose.Get33D(x1,y1);
  fn->DrawNText (surface, (xc + x1 -  8), (yc + y1 - 12), col, "33");
}
//-----------------------------------------------------------------------------
//  Draw compass rose
//  Scale is 40 pixels radius at zoom 1. 10° Marker is 8 pixels
//-----------------------------------------------------------------------------
void CFuiVectorMap::DrawRose(CNavaid *vor,int xc, int yc)
{ U_INT col   = MakeRGB(0,(green >> 3),0);
  if (green  != grlim)  green +=Fade;
  DrawCircle(surface,xc,yc,RoseRd,col);
  mRose.SetNorth(vor->GetMagDev());
  mRose.ComputeMark10(RoseRd,Mark10);
  DrawMark(10,xc,yc,col);
  mRose.ComputeMark05(RoseRd,Mark05);
  DrawMark(05,xc,yc,col);
  DrawRoseDir(xc,yc,col);
  DrawRadial(vor,xc,yc);
  return;
}
//-----------------------------------------------------------------------------
//  Clip any VOR outside view rectangle
//-----------------------------------------------------------------------------
bool CFuiVectorMap::ClipVOR(int vx,int vy)
{ int rose = (option.Has(VM_DROS))?(RoseRd):(0);
  if (vx <= -rose)              return true;
  if (vx >= (w + rose))         return true;
  if (vy <= -rose)              return true;
  if (vy >= (h + rose))         return true;
  return false;
}
//-----------------------------------------------------------------------------
//  CFuiVectorMap  DRAW locals VOR
//-----------------------------------------------------------------------------
void  CFuiVectorMap::DrawVOR(void)
{ CNavaid *vor = 0;
  CBitmap* bmp;
  int xIcon; 
  int yIcon;
  int xbm;
  int ybm;
  int ylab;
  short fnt = 0;
  int xtype = 0;
  for (vor = dbc->FindVORbyDistance(vor,MaxNM);vor != 0; vor = dbc->FindVORbyDistance(vor,MaxNM))
  { xtype = vor->GetIndexType();
    bmp   = OthBMAP[xtype];
    xbm   = OthSIZE[xtype].w2;
    ybm   = OthSIZE[xtype].h2;
    // Draw bitmap and label
    ylab  = GetScreenCoordinates (vor, xIcon, yIcon,ybm,14);
    if (ClipVOR(xIcon,yIcon))  continue;
    fnt   = (Focus == vor)?(1):(0);
    bmp->DrawBitmap (surface,(xIcon - xbm), (yIcon - ybm), 0);
    if (option.Has(VM_DLAB)) DrawLabel(fnt,xIcon,ylab,vor);
    EnterScreenTable(vor,(xIcon + x),(yIcon + y));
    if (option.Has(VM_DROS))  DrawRose(vor,xIcon,yIcon);
  }
  return;
}
//-----------------------------------------------------------------------------
//  CFuiVectorMap  DRAW locals airport
//  Memorize the nearest airport
//-----------------------------------------------------------------------------
void  CFuiVectorMap::DrawAPT(void)
{ CAirport    *apt  = 0;
  CBitmap *bmp;
  int xIcon; 
  int yIcon;
  int aptNo;
  int xbm;
  int ybm;
  int ylab;
  short fnt = 0;
  for (apt = dbc->FindAPTbyDistance(apt,MaxNM);apt != 0; apt = dbc->FindAPTbyDistance(apt,MaxNM))
  { aptNo = apt->GetMapNo();
    if (aptNo == 0)   continue;       // No type yet
    bmp   = AptBMAP[aptNo];
    xbm   = AptSIZE[aptNo].w2;
    ybm   = AptSIZE[aptNo].h2;
    ylab  = GetScreenCoordinates (apt,xIcon, yIcon, ybm, 0);
    if (ClipOBJ(xIcon,yIcon,xbm,ybm)) continue;
    fnt   = (Focus == apt)?(1):(0);
    bmp->DrawBitmap(surface,(xIcon - xbm), (yIcon - ybm), 0);
    if (option.Has(VM_DLAB)) DrawLabel(fnt,xIcon,ylab,apt);
    EnterScreenTable(apt,(xIcon + x),(yIcon + y));
    //-----Remember nearest airport -------------------------------------------
		if (apt->GetNmiles() > ndis) continue;
		ndis = apt->GetNmiles();
    napt = apt;
  }
  return;
}
//---------------------------------------------------------------------------------------------
//  Draw the route starting from this waypoint
//  Distance from aircraft (center of window) to origin (org) and extremity (ext)
//  is stored in temporary objects (fpOrg and fpExt) to draw the line between waypoints
//---------------------------------------------------------------------------------------------
void CFuiVectorMap::DrawRoute(VMnode &org,VMnode &ext)
{ //----Compute distance to aircraft for both extremities --------
	CmHead *no	= org.GetOBJ();
	CmHead *nx  = ext.GetOBJ();
	float d1		= dbc->GetFlatDistance(no);
  float d2		= dbc->GetFlatDistance(nx);

  if ((d1 > MaxNM) && (d2 > MaxNM)) return;
  //----One node visible -----------------------------------------
  int x1,y1;
	GetScreenCoordinates(no,x1,y1);
  int x2,y2;
	GetScreenCoordinates(nx,x2,y2);
  DrawFastLine(surface,x1, y1, x2, y2, white);
	//--- Check if extremity is a waypoint from flight plan --------
	if (nx->GetActiveQ() != WPT)	return;
	//--------------------------------------------------------------
	ext.SetNodeDistance();
	DrawWayPoint((CWPT *)nx);
  return;
}
//---------------------------------------------------------------------------------------------
//  Draw a waypoint from flight plan
//---------------------------------------------------------------------------------------------
void CFuiVectorMap::DrawWayPoint(CWPT *wpt)
{ CBitmap *bmp  = OthBMAP[WPTBLK];
  int     xbm   = OthSIZE[WPTBLK].w2;
  int     ybm   = OthSIZE[WPTBLK].h2;
  int xIcon; 
  int yIcon;
  int ylab		= GetScreenCoordinates (wpt, xIcon, yIcon,ybm);
  short fnt   = (Focus == wpt)?(1):(0);
  bmp->DrawBitmap (surface,(xIcon - xbm), (yIcon - ybm), 0);
  if (option.Has(VM_DLAB)) DrawLabel(fnt,xIcon,ylab,wpt);
  EnterScreenTable(wpt,(xIcon + x),(yIcon + y));
	return;
}
//---------------------------------------------------------------------------------------------
//  Draw the current ILS vector if requested
//---------------------------------------------------------------------------------------------
void CFuiVectorMap::DrawILS()
{ UpdateILS();
  if (0             == aILS)      return;
  dbc->GetFlatDistance(aILS);             // Refresh distance
  //-----------------------------------------------------------
  DebDrawInside();                // Init drawing inside
  glTranslatef(oILS.x,oILS.y,0);
  glRotatef(-aILS->GetIlsVector(),0,0,1);
  glColor3f(1,0,0);
  glInterleavedArrays(GL_T2F_V3F,sizeof(TC_VTAB),tabILS);
  glDrawArrays(GL_LINE_LOOP,0,8);
  //----------------------------------------------------------
  EndDrawInside();
  return;
}
//---------------------------------------------------------------------------------------------
//  CFuiVectorMap  DRAW method
//---------------------------------------------------------------------------------------------
void CFuiVectorMap::Draw (void)
{ TrackPlan();
	EraseSurfaceRGBA (surface,black);
  //---Draw according to State --------------------------------------
  if (dStat == VWIN_DOC) return DrawDocument();
  if (dStat == VWIN_RWY) return DrawRunways();
  //----Compute Drawing order ---------------------------------------
  ndis   = 1000000;
  napt   = 0;
  Frame += 1;
  if (0 == Frame) Turn +=1;
  if (3 == Turn)  Turn  = 0;
//----Clear cache screen table at each frame --------------
  ResetScreenTable();
  GetRRtoLDOrientation(vmapOrient);
  heading       = Wrap360(vmapOrient.h);
  //------------------- ---------------------------------------------------------------------
  fpln->DrawOnMap(this);
  //---------Alternate draw order to allow supperposed items to be mouse selected -----------
  if (0 == Turn)
  { if (option.Has(VM_DNDB))  DrawNDB();
    if (option.Has(VM_DVOR))  DrawVOR();
    if (option.Has(VM_DAPT))  DrawAPT();
  }
  else if (1 == Turn)
  { if (option.Has(VM_DAPT))  DrawAPT();
    if (option.Has(VM_DNDB))  DrawNDB();
    if (option.Has(VM_DVOR))  DrawVOR();
  }
  else if (2 == Turn)
  { if (option.Has(VM_DVOR))  DrawVOR();
    if (option.Has(VM_DAPT))  DrawAPT();
    if (option.Has(VM_DNDB))  DrawNDB();
  }
  //---------- Draw coast line if requested ----------------------------------------
  if (option.Has(VM_DCST))    globals->tcm->ScanCoast(surface);
  //--------- Draw user and other vehicles -----------------------------------------
  int userFrame = int ((32 * heading) / 360);
  int bmw = OthSIZE[USERVEH].w2;
  int bmh = OthSIZE[USERVEH].h2;
  OthBMAP[USERVEH]->DrawBitmap (surface, (halfW - bmw), (halfH - bmh), userFrame);
  //-------- Draw the scale -----------------------------------------------------
  DrawFastLine(surface,10,10,110,10,white);
  DrawFastLine(surface,10, 5, 10,10,white);
  DrawFastLine(surface,110,5,110,10,white);
  fnts->DrawNText(surface,116,2,white,eScal);
	//--- Draw country name -----------------------------------
  char *ctk = (napt)?(napt->GetCountry()):(0);
  char *cty = (ctk) ?(globals->tcm->GetCountry(ctk)):(0);
  if (cty) fnts->DrawNText(surface,200,2,white,cty);
  //----------Draw cursor coordinates -----------------------
  char edt[128];
	EditLat2DMS(geop.lat,edt);
	fnts->DrawNText(surface,300,2,white,edt);
	EditLon2DMS(geop.lon,edt);
	fnts->DrawNText(surface,410,2,white,edt);
	if (snode)
	{	int glv = snode->GetElevation();
		_snprintf(edt,120,"WP ground: %05d feet",glv);
	  fnts->DrawNText(surface,550,2,white,edt);
		int agl = snode->GetAltitude();
		_snprintf(edt,120,"WP Alti: %05d feet",agl);
		fnts->DrawNText(surface,680,2,white,edt);
	}
  //--------- Render map surface-----------------------------
  BlitTransparentSurface (surface, 0, 0, 0);
  //---Draw ILS director ------------------------------------
  if (option.Has(VM_DILS))  DrawILS();
  //---Draw window decoration -------------------------------
  CFuiWindow::Draw();

}
//-------------------------------------------------------------------------
//  Reset the screen table:  
//-------------------------------------------------------------------------
void CFuiVectorMap::ResetScreenTable(void)
{ //----lock screen table here ----------------------
  pthread_mutex_lock (&sTable.mux);
  sTable.NbSlot = 0;
  sTable.xSlot  = sTable.sList;
  //---unlock table here ----------------------------
  pthread_mutex_unlock (&sTable.mux);
  return;
}
//-------------------------------------------------------------------------
//  Store object in screen table
//-------------------------------------------------------------------------
void CFuiVectorMap::EnterScreenTable(CmHead *obj, int x, int y)
{ //----lock screen table here -----------------------
  pthread_mutex_lock (&sTable.mux);
  if (sTable.NbSlot == NBR_OBJ_DBC) return;
  sTable.NbSlot++;
  SC_SLOT *slot = sTable.xSlot++;
  slot->Obj = obj;
  slot->sx  = short(x);
  slot->sy  = short(y);
  //----unlock table here ---------------------------
  pthread_mutex_unlock (&sTable.mux);
  return;
}
//-------------------------------------------------------------------------
//  Look up for screen hit
//--------------------------------------------------------------------------
CmHead* CFuiVectorMap::LookForScreenHit(short x, short y)
{ //-----------lock table here --------------------
  pthread_mutex_lock (&sTable.mux);
  SC_SLOT *slot = sTable.xSlot;
  hSlot         = 0;                        //No Object selected
  while (slot-- != sTable.sList)
  { if (x < (slot->sx - sTable.xRec)) continue;
    if (x > (slot->sx + sTable.xRec)) continue;
    if (y < (slot->sy - sTable.yRec)) continue;
    if (y > (slot->sy + sTable.yRec)) continue;
    hSlot = slot;
    break;
  }
  //---------unlock table here -------------
  pthread_mutex_unlock (&sTable.mux);
  return (hSlot)?(hSlot->Obj):(0);
}
//=================================================================================
//	Togle Mode Plan
//=================================================================================
//=================================================================================
//	Enter Flight plan mode: Replace camera and aircraft
//=================================================================================
void CFuiVectorMap::EnterModePlan()
{ //--- Set application profile -------------------
	if (globals->aPROF.Has(PROF_ACBUSY))	return;
	if (rCAM)															return;
	if (fpln->NotFor3D())									return;
	//--- Activate mode plan ------------------------
	pBox->Show();
	oCTX.fpln		= fpln;
	oCTX.prof		= VMP_PROF;
	oCTX.mode   = SLEW_FPLM;
	rCAM	= globals->ccm->SetRabbitCamera(oCTX,this);
	snode	= 0;
	return;
}
//---------------------------------------------------------------------------------
//	Leave Flight plan mode: Restore camera and aircraft
//---------------------------------------------------------------------------------
void CFuiVectorMap::LeaveModePlan()
{	if (0 == rCAM)							return;
	pBox->Hide();
  globals->fui->DestroyFuiWindow('flog');
  globals->ccm->RestoreCamera(oCTX);
	rCAM	= 0;
	snode	= 0;
	return;
}
//---------------------------------------------------------------------------------
//	Mode plan: follow the plan.  Teleport at selected node
//---------------------------------------------------------------------------------
void CFuiVectorMap::TrackPlan()
{	fpln = globals->pln->GetFlightPlan();
	if (rCAM == 0)				return;
	CWPoint *nod = fpln->GetSelectedNode();
	SPosition *pos;
	if (nod)
	{	pos				= nod->GetGeoP();
		pos->alt  = nod->GetAltitude();
	}
	else	{pos = &oCTX.pos;}
	snode	= nod;
	globals->tcm->MoveRabbit(*pos);
	return;
}
//=================================================================================
//  Scale object size.  Compute all sizing parameters
//  Objects size from Rose are expressed in Nautical miles rescaled by Rose Zoom 
//  
//  --Rose radius   4.0 nm
//  --10° marker    0.8 nm
//  -- 5° marker    0.5 nm
//  vmapDiag is expressed in nautical miles. It serve as selector for
//  finding objects by distance.
//  vmapDiag is a function of screen diagonal in pixel
//=================================================================================
void CFuiVectorMap::Scale(float zm)
{ Zoom		= zm;  
	RoseRd  = int((VM_SCALE *  4.0f * RoseZm) / Zoom);
  Mark10  = int((VM_SCALE *  0.8f * RoseZm) / Zoom);
  Mark05  = int((VM_SCALE *  0.5f * RoseZm) / Zoom);
  float diag = SquareRootFloat(float(halfW * halfW) + float(halfH * halfH));
  Diag = (diag * Zoom) / VM_SCALE;
  sprintf(eScal,"%.2f nm",(Zoom / 8));
  MaxNM   = Diag * Diag;
  pixNM   = (VM_SCALE / (Zoom * 128));
  globals->tcm->SetPPM((VM_SCALE / Zoom),halfW,halfH);
  globals->vmapZoom   = Zoom;
  globals->vmapScrn.x = w;
  globals->vmapScrn.y = h;
  return;
}
//---------------------------------------------------------------------------------
//  SetILS Pixel
//---------------------------------------------------------------------------------
void CFuiVectorMap::IlsPixel(int No,int x,int y)
{ tabILS[No].VT_X = x;
  tabILS[No].VT_Y = h - y;
  tabILS[No].VT_Z = 0;
  return;
}
//---------------------------------------------------------------------------------
//  Build ILS arrows coordinates
//---------------------------------------------------------------------------------
void CFuiVectorMap::UpdateILS()
{ aILS  = globals->cILS;
  if (0 == aILS)    return;
  //----Compute ILS originin pixel  ---------------
  GetScreenCoordinates(aILS,oILS.x,oILS.y);
  oILS.y = h - oILS.y;
  //----Compute other pixel side ------------------
  rILS = (22 * VM_SCALE) / Zoom;
  tabILS[3].VT_Y = rILS;
  tabILS[4].VT_Y = rILS;
  return;
}
//---------------------------------------------------------------------------------
//  Event notification: More transparency
//---------------------------------------------------------------------------------
void CFuiVectorMap::TrnsPlus()
{ U_INT col = black;
  if (0 == col) return;
  col -= 0x04000000;
  black = col;
  globals->vmapTrns = col;
  return;
}
//---------------------------------------------------------------------------------
//  Event notification: Less transparency
//---------------------------------------------------------------------------------
void CFuiVectorMap::TrnsLess()
{ U_INT col = black;
  if (0xFC000000 == col)  return;
  col += 0x04000000;
  black = col;
  globals->vmapTrns = col;
  return;
}
//---------------------------------------------------------------------------------
//  Event notification: Zoom Out
//---------------------------------------------------------------------------------
void CFuiVectorMap::ZoomLess()
{switch (dStat)  {
  case VWIN_MAP:
    if (Zoom >= 200.0f)   return;
    Zoom  += 2.0f;
    break;

  case VWIN_RWY:
    Zoom = Cam->MoveUp(+1);
    break;
  }
	Scale(Zoom);
  return;
}
//---------------------------------------------------------------------------------
//  Event notification: Zoom in
//---------------------------------------------------------------------------------
void CFuiVectorMap::ZoomPlus()
{ switch (dStat)  {

  case VWIN_MAP:
    if (Zoom <= 10.0f)    return;
    Zoom  -= 2.0f;
    break;

  case VWIN_RWY:
    Zoom = Cam->MoveUp(-1);
    break;
  }
	Scale(Zoom);
  return;
}

//---------------------------------------------------------------------------------
//  Event notification
//---------------------------------------------------------------------------------
void  CFuiVectorMap::NotifyChildEvent(Tag idw,Tag itm,EFuiEvents evn)
{ fpln = globals->pln->GetFlightPlan();
	switch (itm)  {
  case 'zout':                            // global Zoom out (-)
    ZoomLess();
    return;
  case 'zin ':                            // Global zoom in(+)
    ZoomPlus();
    return;
  case 'bkdn':														// Transparency down
    TrnsLess();
    return;
  case 'bkup':														// Transparency up
    TrnsPlus();
    return;
  case 'rout':														// Rose zoom
    if (dStat != VWIN_MAP)    return;
    if (RoseZm >= 2.0)        return;
    RoseZm  += 0.05f;
    Scale(Zoom);
    return;
  case 'rin ':
    if (dStat != VWIN_MAP)    return;
    if (RoseZm <= 1.0)        return;
    RoseZm  -= 0.05f;
    Scale(Zoom);
    return;
	case 'plan':														// View  plan
		if (rCAM)		LeaveModePlan();
		else				EnterModePlan();
		return;
	case 'nxwp':														// Next node
		fpln->GoToNextNode();
		return;
	case 'pvwp':														// Prev node
		fpln->GoToPrevNode();
		return;
  case 'sysb':
    if (EVENT_CLOSEWINDOW == evn) { Close();    return;}
    if (EVENT_ZOOM        == evn) { (WINDOW_SIZE_MAXI == vStat)?NormSize():MaxiSize(); return;}
    if (EVENT_MINIMIZE    == evn) { (WINDOW_SIZE_MINI == vStat)?NormSize():MiniSize(); return;}
    return;
    
}
  return;
}
//----------------------------------------------------------------------------------
//  Menu Notifications
//  Todo search idm
//----------------------------------------------------------------------------------
void CFuiVectorMap::NotifyMenuEvent(Tag idm, Tag itm)
{ switch (itm)  {
  case ('drwa'):
    option.Toggle(VM_DAPT);
    break;
  case ('nama'):
    option.Toggle(VM_APTN);
    break;
  case ('idsa'):
    option.Toggle(VM_APTI);
    break;
  case ('drwn'):
    option.Toggle(VM_DNDB);
    break;
  case ('namn'):
    option.Toggle(VM_NDBN);
    break;
  case ('idsn'):
    option.Toggle(VM_NDBI);
    break;
  case ('drwv'):
    option.Toggle(VM_DVOR);
    break;
  case ('namv'):
    option.Toggle(VM_VORN);
    break;
  case ('idsv'):
    option.Toggle(VM_VORI);
    break;
  case ('frqn'):
    option.Toggle(VM_VORF);
    break;
  case ('labl'):
    option.Toggle(VM_DLAB);
    break;
  case ('clin'):
    option.Toggle(VM_DCST);
    break;
  case ('ilsd'):
    option.Toggle(VM_DILS);
    break;
  case ('plat'):
    option.Toggle(VM_DROS);
    break;
  case ('fade'):                    // Decrease green up to middle
    Fade *= -1;
    grlim = (Fade == 1)?(8 * 255):(8 * 64);
    break;
  default:
    return;
}
  globals->vmpOpt = option;
  mBar->CheckSelectedPart(idm,itm);
  return;
}
//---------------------------------------------------------------------------------
//  Mouse move over the Popup menu
//---------------------------------------------------------------------------------
bool CFuiVectorMap::MoveOverPOP(int mx, int my)
{ if (Focus.IsNull())		return true;
	EditPopDistance(mx,my);
  if (Focus->Isa(WPT))	return true;
  MyPop->Refresh(1);
  return true;
}
//---------------------------------------------------------------------------------
//  Check if mouse move over a bitmap. if not base class will handle it
//  If object is Hit assign it to smart pointer Focus. This will manage user count
//---------------------------------------------------------------------------------
bool CFuiVectorMap::InsideMove(int mx,int my)
{ if ((VWIN_MAP == dStat) && (MyPop))     return MoveOverPOP(mx,my);
  if  (VWIN_DOC == dStat)                 return MoveDOC(mx,my);
  if  (VWIN_LST == dStat)                 return true;
  if  (VWIN_RWY == dStat)                 return MoveRWY(mx,my);
	if  (VWIN_WPT == dStat)									return MoveWPT(mx,my);

  Focus = LookForScreenHit(short(mx),short(my));
  if (Focus.Assigned())                   return true;
  if (!MouseHit(mx,my))                   return false;
  EditCoordinates(mx,my);
  return true;
} 
//----------------------------------------------------------------------------------
//  Mouse Click over some part of the screen
//  the variable dStat indicate which state the window is into.
//  
//----------------------------------------------------------------------------------
bool CFuiVectorMap::InsideClick (int mx, int my, EMouseButton button)
{ if (RegisterFocus(0))								return true;
  if (!MouseHit(mx,my))               return false;
  switch (dStat)    {
  //-----NORMAL VECTOR WINDOWS ---------------------------------------------
  case VWIN_MAP:
    if (Focus == 0)                   return OpenWptMENU(mx,my);
		if (Focus->Isa(WPT))							return ClickWptOBJ(mx,my,button);
    if (button == MOUSE_BUTTON_LEFT)  return OpenWinDET(Focus.Pointer(),0);
    if (button == MOUSE_BUTTON_RIGHT) return OpenPopOBJ(mx,my);
    break;
  //----DOCUMENT WINDOW ---------------------------------------------------
  case VWIN_DOC:
    if  (button == MOUSE_BUTTON_MIDDLE) return ResetImage(DocInfo);
    if  (button == MOUSE_BUTTON_LEFT)   return DragDOC(mx,my);
    if  (button == MOUSE_BUTTON_RIGHT)  return OpenPopDOC(mx,my);
    break;
  //----DOCUMENT LIST----------------------------------------------------
  case VWIN_LST:
    break;
  //----TOP VIEW -------------------------------------------------------
  case VWIN_RWY:
    if  (button == MOUSE_BUTTON_LEFT)  return DragRWY(mx,my);
    if  (button == MOUSE_BUTTON_RIGHT) return OpenRwyLIST(mx,my);
    break;
  }
  return  (!HasProperty(FUI_TRANSPARENT));
}
//----------------------------------------------------------------------------------
//  End move
//----------------------------------------------------------------------------------
bool CFuiVectorMap::StopClickInside(int x, int y, EMouseButton button)
{ DocInfo.state = 0;										// STop any move
	dStat	= vmapNSTAT[dStat];
  return true;
}
//----------------------------------------------------------------------------------
//  Resize notification
//  Recompute diagonal
//----------------------------------------------------------------------------------
void CFuiVectorMap::NotifyResize(short dx,short dy)
{ Scale(Zoom);
  return;
}
//----------------------------------------------------------------------------------
//  Update header with distance
//----------------------------------------------------------------------------------
void CFuiVectorMap::EditPopDistance(int mx,int my)
{ char *cat     = dbc->GetObjCategoryName(Focus.Pointer());
  char *idn     = Focus->GetIdent();
  float dx      = ((mx - xOrg) * Zoom) / VM_SCALE;
  float dy      = ((my - yOrg) * Zoom) / VM_SCALE;
  float ds      = SquareRootFloat((dx * dx) + (dy *dy));
  sprintf(disHD,"-%s: %.1f nm from %s",cat,ds,idn);
}
//----------------------------------------------------------------------------------
//  Update upper line with coordinates
//----------------------------------------------------------------------------------
void CFuiVectorMap::EditCoordinates(int sx,int sy)
{ int xorg = halfW + surface->xScreen;
  int yorg = halfH + surface->yScreen;
  double dx = +((sx - xorg) * Zoom) / VM_SCALE;
  double dy = -((sy - yorg) * Zoom) / VM_SCALE;
  geop = globals->geop;
  AddMilesTo(geop,dx,dy);
	//--- Get altitude at geop -------------------
	GroundSpot lnd(geop.lon,geop.lat);
	globals->tcm->SetGroundAt(lnd);
	geop.alt	= lnd.alt;
	return;
}
//----------------------------------------------------------------------------------
//  Edit Menu Item for NAV,NDB or AIRPORT
//----------------------------------------------------------------------------------
bool CFuiVectorMap::EditPopITM()
{ //--- Set Fplan mode --------------------------------
	bool fp = fpln->IsProtected();
	vpln = (fp)?(AptVAR2):(AptVAR1);	// FPlan?
	//----Build menu lines ------------------------------
  CmHead*obj      = Focus.Pointer();
  char   typ      = (APT == obj->GetActiveQ())?(1):(0);
  if (APT == typ)  return EditPopAPT(obj);
  //---Popup for NAV,NDB ------------------------------
  RadioLST.dep    = 6;
  RadioLST.ind    = 0;
  SRADIO_TYPE &rdt  = RadioLST.tab[0];
  float frq       = obj->GetFrequency();
  EditPopDistance(xOrg,yOrg);
  _snprintf(frqHD,20,"*FREQUENCY %.02f",frq);
  _snprintf(namHD,31," %s",obj->GetName());
  rdt.kind        = obj->GetRadioIndex();
  rdt.freq        = obj->GetFrequency();
  namHD[31]       = 0;
  smen.aText      = NavMENU;
  smen.aText[MENU_SLOT00]   = namHD;
  smen.aText[MENU_SLOT01]   = disHD;
	smen.aText[MENU_SLOT03]   = vpln;
  smen.aText[MENU_SLOT06]   = frqHD;
  return true;
}
//----------------------------------------------------------------------------------
//  Set view runway item
//----------------------------------------------------------------------------------
int CFuiVectorMap::SetRWYitem(char k)
{ if (0 == obView)  return k;
  smen.aText[k] =  "VIEW RUNWAYS";
  return (k+1);
}
//----------------------------------------------------------------------------------
//  Set Lighting request
//----------------------------------------------------------------------------------
int CFuiVectorMap::SetLITitem(char k)
{ if (0 == obView)  return k;
  smen.aText[k] =  "Lighting Request";
  return (k+1);
}
//----------------------------------------------------------------------------------
//  Get ATIS Radio (object is an airport
//----------------------------------------------------------------------------------
int CFuiVectorMap::SetCOMitem(char k)
{ int ind =  RadioLST.ind;
  CAirport *apt  = obAirp;
  CCOM     *rad  = apt->GetATIS();
  if (0 == rad)             return  k;
  float     frq     = rad->GetFrequency();
  SRADIO_TYPE &rdt  = RadioLST.tab[ind];
  char     *edt     = rdt.txt;
  rdt.kind = COM_INDEX;
  rdt.freq = frq;
  _snprintf(edt,16,"*ATIS %.02f",frq);
  RadioLST.ind   = ind + 1;
  smen.aText[k]  = edt;
  return (k+1);
}
//----------------------------------------------------------------------------------
//  Get ILS Radio (object is an airport)
//----------------------------------------------------------------------------------
int CFuiVectorMap::SetILSitem(char k)
{ CAirport *apt = obAirp;
  ClQueue  *qhd = &apt->rwyQ;
  CRunway *rwy;
  for (rwy = (CRunway*)qhd->GetFirst(); rwy != 0; rwy = (CRunway*)rwy->NextInQ1())
  { k = EditILS(rwy->GetIlsData(RWY_HI_END),k);
    k = EditILS(rwy->GetIlsData(RWY_LO_END),k);
  }
  return k;
}

//----------------------------------------------------------------------------------
//  Edit this ILS
//----------------------------------------------------------------------------------
int CFuiVectorMap::EditILS(ILS_DATA *dat, int k)
{ CILS     *ils = dat->ils;
  if (0 == ils)                 return k;
  int   ind  = RadioLST.ind;
  if   (ind == RadioLST.tot)    return k;
  SRADIO_TYPE &rdt  = RadioLST.tab[ind];
  float frq  = ils->GetFrequency();
  char *edt  = rdt.txt;
  rdt.kind   = NAV_INDEX;
  rdt.freq   = frq;
  _snprintf(edt,16,"*ILS %-4s %.02f",ils->GetRWID(),frq);
  RadioLST.ind    = ind + 1;
  smen.aText[k++] = edt;
  return k;
}
//----------------------------------------------------------------------------------
//  Edit Menu Item for an AIRPORT
//  Enter one ATIS frequency
//----------------------------------------------------------------------------------
bool CFuiVectorMap::EditPopAPT(CmHead *obj)
{ CAirport *apt  = (CAirport *)obj;
  void     *apo  = apt->GetAPO();
  obView         = apo;
  obAirp         = apt;
	smen.aText[0]  = 0;
	//--- Edit menu -----------------------------------
  _snprintf(namHD,31," %s",apt->GetName());
  EditPopDistance(xOrg,yOrg);
  int itm       = MENU_SLOT06;              // Item index
  smen.aText    = AptMENU;
  smen.aText[MENU_SLOT00] = namHD;
	smen.aText[MENU_SLOT03] = vpln;
  smen.aText[MENU_SLOT01] = disHD;
  itm = SetRWYitem(itm);
  smen.aText[itm++] = NavMENU[MENU_SLOT05];
  RadioLST.dep  = itm;
  RadioLST.ind  = 0;
  itm = SetCOMitem(itm);
  itm = SetILSitem(itm);
  smen.aText[itm++] = "-----------------------------";
  itm = SetLITitem(itm);
  smen.aText[itm++] = "RETURN";
  smen.aText[itm] = 0;
  return true;
}
//----------------------------------------------------------------------------------
//  Open Popup menu
//----------------------------------------------------------------------------------
bool CFuiVectorMap::OpenPOP(int mx,int my)
{ MyPop = new CFuiPage(mx,my,&smen,this);
	xOrg  = mx;
  yOrg  = my;
  MyPop->SetState(1);
  RegisterFocus(MyPop);
	return true;
}
//==================================================================================
//	DOCUMENT List 
//==================================================================================
//---------------------------------------------------------------------------------
//  Add Doc name to the list
//---------------------------------------------------------------------------------
int CFuiVectorMap::AddDocName(char *dn)
{	int lgr = strlen(dn) + 1;
	rmDoc	 -= lgr;
	if (rmDoc <= 0)		return 0;
	//--- copy the name into name space -----------
	strncpy(nmDoc,dn,lgr);
	DocMENU[nxDoc++] = nmDoc;
	nmDoc	+= lgr;
	return (nxDoc < 16)?(1):(0);
}
//---------------------------------------------------------------------------------
//  Build a list of file for this airport
//---------------------------------------------------------------------------------
int CFuiVectorMap::SearchDOC()
{ CAirport *apt = (CAirport*)Focus.Pointer();
  char       pat[PATH_MAX];
  sprintf(pat,"DOCUMENTS/%s*.PNG",apt->GetAptName());
	//--- init name space ----------------------------
	rmDoc	= sizeof(DocPILE);
	nxDoc	= 0;
	nmDoc	= DocPILE;
	ApplyToFiles(pat,VMapCB,this);
	DocMENU[nxDoc] = 0;
	return nxDoc;
}
//----------------------------------------------------------------------------------
//  Open a floating menu with the list of documents
//----------------------------------------------------------------------------------
bool CFuiVectorMap::OpenDocLIST(int mx,int my)
{ smen.Ident = 'ldoc';
	smen.aText = DocMENU;
  dStat = VWIN_LST;
  return OpenPOP(mx,my);
}
//---------------------------------------------------------------------------------
//  dStat = 2 => We are selecting a document
//  Select a document.  Enter Document Windows
//---------------------------------------------------------------------------------
int CFuiVectorMap::ClickDocLIST(short itm)
{ char *m1 = smen.aText[0];
  if (strcmp(m1,noDOC) == 0) return ClosePopMenu();
  //---Select the document and display ---------------
  char       fn[PATH_MAX];
  sprintf(fn,"DOCUMENTS/%s",smen.aText[itm]);
  if (globals->txw->LoadImagePNG(fn,DocInfo))
  //----CHANGE VECTORMAP STATE --------------------------------
  { dStat = VWIN_DOC;
    DocInfo.state  = 0;
    DocInfo.x0     = 0;
    DocInfo.y0     = 5;
  }
  ClosePopMenu();
  return 1;
}
//==================================================================================
//	DOCUMENT Management 
//==================================================================================
//---------------------------------------------------------------------------------
//  Click over a document
//---------------------------------------------------------------------------------
int CFuiVectorMap::ClickDocMENU(short itm)
{ dStat = VWIN_MAP;
  delete [] DocInfo.rgba;
  DocInfo.rgba = 0;
  ClosePopMenu();
  return 1;
}
//----------------------------------------------------------------------------------
//  Open a floating menu over a document
//----------------------------------------------------------------------------------
bool CFuiVectorMap::OpenPopDOC(int mx,int my)
{ smen.Ident = 'edoc';
	smen.aText = cMENU;
  smen.aText[0] = "CLOSE DOCUMENT";
  smen.aText[1] = 0;
	return OpenPOP(mx,my);
}
//----------------------------------------------------------------------------------
//  Draw the document
//----------------------------------------------------------------------------------
void CFuiVectorMap::DrawDocument()
{   DrawImage(surface,DocInfo);
  //--------- Render map surface-------------------------------------------------
    BlitTransparentSurface (surface, 0, 0, 0);
  //---Draw window decoration -------------------------------
    CFuiWindow::Draw();
    return;
}
//----------------------------------------------------------------------------------
//  Start to Drag the document
//----------------------------------------------------------------------------------
bool CFuiVectorMap::DragDOC(int mx,int my)
{ DocInfo.state  = VMAP_DOC_DRAG;
  DocInfo.mx     = mx;
  DocInfo.my     = my;
  return true;
}
//----------------------------------------------------------------------------------
//  Move the Document
//----------------------------------------------------------------------------------
bool CFuiVectorMap::MoveDOC(int mx,int my)
{  return CFuiWindow::MoveImage(mx,my,DocInfo);
}
//==================================================================================
//	RUNWAY List 
//==================================================================================
//----------------------------------------------------------------------------------
//  Open a floating menu over the Runway View
//	-Create a list of runway ends
//----------------------------------------------------------------------------------
bool CFuiVectorMap::OpenRwyLIST(int mx,int my)
{ smen.Ident = 'lrwy';
	smen.aText = cMENU;
	smen.aItem = pMENU;
	smen.aText[0] = "--Starting runway--";
	int k					= SetRWYends(1);
  smen.aText[k++] = "CLOSE VIEW";
  smen.aText[k]		= 0;
  return OpenPOP(mx,my);
}
//----------------------------------------------------------------------------------
//  Build a list of runway ends
//----------------------------------------------------------------------------------
int CFuiVectorMap::SetRWYends(char k)
{ char     *dst = rwyIDN;
	CAirport *apt = obAirp;
  ClQueue  *qhd = &apt->rwyQ;
  CRunway *rwy;
  for (rwy = (CRunway*)qhd->GetFirst(); rwy != 0; rwy = (CRunway*)rwy->NextInQ1())
  { char *end = rwy->GetHiEnd();
	  sprintf(dst,"*%s",end);
		smen.aText[k]	= dst;
		dst	+= 8;
		k++;
		end	= rwy->GetLoEnd();
		sprintf(dst,"*%s",end);
		smen.aText[k]	= dst;
		dst	+= 8;
		k++;
  }
  return k;
}
//----------------------------------------------------------------------------------
//  Draw Top runway View
//  Open a camera on top of aircraft.
//  -Draw runways and taxiways
//  -Draw aircraft icon
//----------------------------------------------------------------------------------
void CFuiVectorMap::DrawRunways()
{ WindowViewPort(vp);
  //---------------------------------------------------------
  Cam->DrawObject(0,vp,this);
	//-------- Draw the scale -----------------------------------------------------
  DrawFastLine(surface,10,10,110,10,white);
  DrawFastLine(surface,10, 5, 10,10,white);
  DrawFastLine(surface,110,5,110,10,white);
  fnts->DrawNText(surface,116,2,white,eScal);
  //--------- Render map surface-----------------------------
  BlitTransparentSurface (surface, 0, 0, 0);
  //---Draw window decoration -------------------------------
  CFuiWindow::Draw();
  // Check for an OpenGL error
  GLenum e = glGetError ();
  if (e != GL_NO_ERROR) 
    WARNINGLOG ("CFuiManager::Draw - GL Error 0x%04X", e);
  return;
}
//----------------------------------------------------------------------------------
//  Draw By camera
//----------------------------------------------------------------------------------
void CFuiVectorMap::DrawByCamera(CCamera *cam)
{ CAptObject *apo = (CAptObject*)obView;
  apo->PreDraw(cam);
  apo->CamDraw(cam);
  apo->EndDraw(cam);
  return;
}
//==================================================================================
//	RUNWAY Management 
//==================================================================================
//----------------------------------------------------------------------------------
//  Start to Drag the Tarmac
//----------------------------------------------------------------------------------
bool CFuiVectorMap::DragRWY(int mx,int my)
{ DocInfo.state  = VMAP_RWY_DRAG;
  DocInfo.mx     = mx;
  DocInfo.my     = my;
  return true;
}
//----------------------------------------------------------------------------------
//  Move the Tarmac
//----------------------------------------------------------------------------------
bool CFuiVectorMap::MoveRWY(int mx,int my)
{ if (DocInfo.state != VMAP_RWY_DRAG)  return true;
  short dx  = mx - DocInfo.mx;
  short dy  = my - DocInfo.my;
  DocInfo.mx   = mx;
  DocInfo.my   = my;
  Cam->MoveBy(dx,dy);
  return true;
}
//---------------------------------------------------------------------------------
//  Click over a runway list to set aircraft on the runway end
//---------------------------------------------------------------------------------
int CFuiVectorMap::ClickRwyMENU(short itm)
{ char ch = *smen.aText[itm];
	if (itm == 0)			return 1;
  if (ch	== '*')		return StartonRWY(itm);
	dStat = VWIN_MAP;
  ClosePopMenu();
	//--- Reset zoom factor -------------------
	Scale(pZom);
  return 1;
}
//---------------------------------------------------------------------------------
//  Position aircraft for start on runway
//---------------------------------------------------------------------------------
int CFuiVectorMap::StartonRWY(short itm)
{ char *idn = smen.aText[itm] + 1;
	globals->apm->SetOnRunway(obAirp,idn);
	return 1;
}
//==================================================================================
//	WAYPOINT Management 
//==================================================================================
//----------------------------------------------------------------------------------
// Set Waypoint ground Elevation
//----------------------------------------------------------------------------------
void CFuiVectorMap::SetElevation(SPosition &p)
{	GroundSpot lnd(p.lon,p.lat);
  p.alt  = globals->tcm->SetGroundAt(lnd);
	_snprintf(WptALTI,120,"%05d feet",int(p.alt));
	return;
}
//---------------------------------------------------------------------------------
//  Create a waypoint at cursor position and add it to flight plan
//---------------------------------------------------------------------------------
int CFuiVectorMap::CreateWPT()
{	//--- Add waypoint to flight plan -------------
	CWPT *wpt = fpln->CreateUserWPT(&wpos);
	Focus		  = wpt;
	CFuiFlightLog *nwin = fpln->GetWinPlan();		
	if (nwin)  nwin->NotifyFromDirectory(Focus.Pointer());
	return 1;
}
//----------------------------------------------------------------------------------
//  Open a floating menu to create a user waypoint
//----------------------------------------------------------------------------------
bool CFuiVectorMap::OpenWptMENU(int mx,int my)
{ bool fp = fpln->IsProtected();
	WptMENU[2] = (fp)?(0):(addWPT);
	smen.Ident = 'mwpt';
	smen.aText = WptMENU;
  wpos  = geop;
	SetElevation(wpos);
  return OpenPOP(mx,my);
}
//---------------------------------------------------------------------------------
//  Click waypoint menu
//	Check for waypoint creation
//---------------------------------------------------------------------------------
int CFuiVectorMap::ClickWptMENU(short itm)
{ if (*smen.aText[itm]	== 'A')	CreateWPT();
	dStat = VWIN_MAP;
  ClosePopMenu();
  return 1;
}
//----------------------------------------------------------------------------------
//  Click over a waypoint
//----------------------------------------------------------------------------------
bool CFuiVectorMap::ClickWptOBJ(int mx,int my,EMouseButton button)
{	if (button == MOUSE_BUTTON_RIGHT)	return OpenWptINFO(mx,my);
	dStat = VWIN_WPT;
	//--- prepare to move the waypoint ----------------------
	DocInfo.state = VMAP_WPT_DRAG;
	return true;
}
//----------------------------------------------------------------------------------
// Move a Waypoint on the MAP
//----------------------------------------------------------------------------------
bool CFuiVectorMap::MoveWPT(int mx,int my)
{ if (globals->aPROF.Has(PROF_RABIT)) return true;
	int xorg = halfW + surface->xScreen;
  int yorg = halfH + surface->yScreen;
  double dx = +((mx - xorg) * Zoom) / VM_SCALE;
  double dy = -((my - yorg) * Zoom) / VM_SCALE;
  SPosition pos = globals->geop;
  AddMilesTo(pos,dx,dy);
	CWPT *wpt = (CWPT*)Focus.Pointer();
	//--- Set waypoint altitude from terrain ------
	wpt->GetNode()->HorizontalMove(&pos);
	return true;
}
//----------------------------------------------------------------------------------
//  Open a floating menu for WPT
//----------------------------------------------------------------------------------
bool CFuiVectorMap::OpenWptINFO(int mx, int my)
{ CWPT *wpt   = (CWPT*)Focus.Pointer();
	smen.Ident	= 'mowp';
	smen.aText  = WayMENU;
	char *nam   = wpt->GetName();
	_snprintf(wayNAME,120,"%s Elevation: %d feet",nam,int(wpt->GetElevation()));
	wayNAME[120]= 0;
	sprintf(disHD,"%.1f nm from previous",wpt->GetDistance());
	smen.aText[MENU_SLOT01] = disHD;
	return OpenPOP(mx,my);
}
//=================================================================================
//  Draw taxiway Nodes
//=================================================================================
//---------------------------------------------------------------------------------
//	Draw the taxiway nodes
//---------------------------------------------------------------------------------
void CFuiVectorMap::DrawTaxiNodes()
{
}

//==================================================================================
//	SCREEN OBJECTS Management 
//==================================================================================
//----------------------------------------------------------------------------------
//  Open a floating menu for APT, NAV or NDB
//----------------------------------------------------------------------------------
bool CFuiVectorMap::OpenPopOBJ(int mx,int my)
{ smen.aText  = cMENU;
  //--- Save teleport position ---------
  CmHead *obj = Focus.Pointer();
  wpos        = obj->GetPosition();
  //--- Edit Menu ----------------------
	smen.Ident	= 'mmap';
  if (!EditPopITM()) return true;
	return OpenPOP(mx,my);
}
//----------------------------------------------------------------------------------
//  Close the floating menu
//----------------------------------------------------------------------------------
int CFuiVectorMap::ClosePopMenu()
{ if (0 == MyPop) return 0;
  MyPop->SetState(0);
  ClearFocus(MyPop);
  delete MyPop;
  MyPop     = 0;
  Focus     = 0;
  //---When closing popup, we cant stay in state 2 ----------
  if (VWIN_LST == dStat)  dStat = VWIN_MAP;
  return 1;
}

//---------------------------------------------------------------------------------
//  Click waypoint Object
//---------------------------------------------------------------------------------
int CFuiVectorMap::ClickWptOBJM(short itm)
{	dStat = VWIN_MAP;
  ClosePopMenu();
  return 1;
}

//----------------------------------------------------------------------------------
//  Open Metar for airport
//----------------------------------------------------------------------------------
void CFuiVectorMap::OpenMetar(CmHead *obj)
{ CFuiStrip *win = (CFuiStrip*)globals->fui->CreateFuiWindow(FUI_WINDOW_STRIP);
  win->SetIdent(obj->GetIdent());
  return;
}
//----------------------------------------------------------------------------------
//  dStat = 0 => We are in Vector Window
//  Store option, close the menu and execute option
//----------------------------------------------------------------------------------
int CFuiVectorMap::ClickMapMENU(short itm)
{ char  doc = 0;
  CmHead *obj = Focus.Pointer();
  char  opt = *smen.aText[itm];
  if (0 == itm) {ClosePopMenu(); return 1;}
  switch (opt) {
    //--Open detail window on object -------------------------------
    case 'D':   // Open Detail window
      SmallDetailObject(obj,0);  
      break;
    //--Open airport document --------------------------------------
    case 'O':
      doc = SearchDOC();
      break;
    //--Add Nav object to flight plan if the windows is open -------
    case 'A':                            
      AddToFlightPlan();
      break;
    //--Teleport to mouse point -----------------------------------
    case 'T':
      Teleport();
      break;
    //--- Open Metar bulletin -------------------------------------
    case 'M':
      OpenMetar(obj);
      break;
    //--View Runway -----------------------------------------------
    case 'V':
      dStat   = VWIN_RWY; 
			//--- Save current zoom and init runway view -----
			pZom	= Zoom;							
			Zoom  = Cam->SetOrigin(obAirp->ObjPosition());
			Scale(Zoom);
      break;
    //---- Tune a radio -------------------------------------------
    case '*':
      ClickRadLIST(itm);
      break;
    //---- Lighting request ---------------------------------------
    case 'L':
      ClickLitITEM();
      break;
  }
  ClosePopMenu();
  if (doc)  OpenDocLIST(xOrg,yOrg);
  return 1;
}
//-------------------------------------------------------------------------
//  Tune com radio to the frequency of selected com
//-------------------------------------------------------------------------
int CFuiVectorMap::ClickRadLIST(int k)
{ CVehicleObject *veh = globals->pln;
  if (0 == veh)       return 0;
  //---Get corresponding tag ----------
  int   No    = k - RadioLST.dep ;
  int   rx    = RadioLST.tab[No].kind;
  float frq   = RadioLST.tab[No].freq;
  //---Get master radio --------------
  mesg.group   = veh->GetRadio(rx);
  //----Tune radio or com ----------------
  mesg.receiver       = 0;
  mesg.realData       = frq;
  mesg.user.u.datatag = 'tune';
  Send_Message(&mesg);
  return 1;
}
//-------------------------------------------------------------------------
//  Request to light airport
//-------------------------------------------------------------------------
void CFuiVectorMap::ClickLitITEM()
{ CAptObject *apo = (CAptObject *)obView;
 apo->LightPlease();
 return;
}
//----------------------------------------------------------------------------------
//  Notification from Popup
//----------------------------------------------------------------------------------
void CFuiVectorMap::NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn)
{ if (EVENT_FOCUSLOST  == evn)      {ClosePopMenu(); return;}
	if (EVENT_POP_CLICK  != evn)			return;
	switch(id)	{
			//--- click on map ------------------
			case 'mmap':
				ClickMapMENU(itm);
				return;
			//--- click on document list --------
			case 'ldoc':
				ClickDocLIST(itm);
				return;
			//--- Close document -----------------
			case 'edoc':
				ClickDocMENU(itm);
				return;
			//-- Click on runawy list ------------
			case 'lrwy':
				ClickRwyMENU(itm);
				return;
			//--- Click over waypoint menu ------
			case 'mwpt':
				ClickWptMENU(itm);
				return;
			//--- Click over waypoint object ------
			case 'mowp':
				ClickWptOBJM(itm);
				return;
	}
  return;
}
//----------------------------------------------------------------------------------
//  Check for flight plan mode
//----------------------------------------------------------------------------------
bool CFuiVectorMap::ModePlan()
{	bool ok = (fpln->GetWinPlan() != 0);
	return ok;
}
//----------------------------------------------------------------------------------
//  Add object to flight plan if navlog window is open
//----------------------------------------------------------------------------------
void CFuiVectorMap::AddToFlightPlan()
{ CFuiFlightLog *nwin = fpln->GetWinPlan();
  if (0 == nwin)      return;
  nwin->NotifyFromDirectory(Focus.Pointer());
  return;
}
//----------------------------------------------------------------------------------
//  Compute coordinate of mouse point and teleport to the point
//----------------------------------------------------------------------------------
void CFuiVectorMap::Teleport()
{ globals->tcm->Teleport(wpos);
  return;
}

//==================================================================================
//  NAVAID Detail
//    The NAVAID window has 2 formats
//    1-Reduced format when created from vector MAP.  All child windows below 
//      the frequency label are eliminated.
//    2-Full format when  created from navigation log
//==================================================================================
CFuiNavDetail::CFuiNavDetail(Tag idn,const char *filename, int lim)
:CFuiWindow(idn,filename,0,0,lim)
{ wName = "NavDetail";
  font  = &globals->fonts.ftsmal10;       // Change Font
  nav   = 0;
  type  = (QTYPE)0;
  //--------Radio message for tuning --------------------------
  mesg.id = MSG_SETDATA;
  mesg.user.u.hw   = HW_RADIO;
  mesg.user.u.unit = 1;
}

//--------------------------------------------------------------------------
//  Set VOR/NDB parameters and edit
//  NOTE:  The smart pointer ensure object user accounting
//--------------------------------------------------------------------------
void CFuiNavDetail::Initialize(CmHead *obj,U_SHORT tp,U_SHORT No)
{ char *erm = "Incorrect DetailsNavaid.win";
  CDbCacheMgr *dbc = globals->dbc;
  type  = (QTYPE)tp;
  nav   = (CNavaid*)obj;
  Po    = obj;
  wptNo = No;
  mesg.realData       = nav->GetFrequency();
  //--------------------------------------------
  float nbr;
  char  edt[256];
  CFuiLabel *lab = 0;
   //-------------------------------------------
  lab = (CFuiLabel*)GetComponent('name');
  if (0 == lab)  gtfo(erm);
  lab->ChangeFont(font);
  lab->EditText("Name:   %s",nav->GetName());
  //-----Country ------------------------------
  lab = (CFuiLabel*)GetComponent('cnty');
  if (0 == lab)  gtfo(erm);
  lab->ChangeFont(font);
  lab->EditText("Country %s",nav->GetCountry());
  //-----VOR Type -----------------------------
  U_INT tnav = nav->GetNavType();
  sprintf(edt,"%s: %s",globals->vorTAB[tnav],nav->GetIdent());
  SetTitle(edt);
  //----Latitude -----------------------------
  EditLat2DMS(nav->GetLatitude(),edt);
  lab = (CFuiLabel*)GetComponent('latv');
  if (0 == lab)  gtfo(erm);
  lab->ChangeFont(font);
  lab->SetText(edt);
  //----Longitude ----------------------------
  EditLon2DMS(nav->GetLongitude(),edt);
  lab = (CFuiLabel*)GetComponent('lonv');
  if (0 == lab)  gtfo(erm);
  lab->ChangeFont(font);
  lab->SetText(edt);
  //------Elevation --------------------------
  lab = (CFuiLabel*)GetComponent('high');
  if (0 == lab)  gtfo(erm);
  nbr = nav->GetElevation();
  lab->ChangeFont(font);
  lab->EditText("Alt %.0f ft",nbr);
  //-----Frequency ---------------------------
  lab = (CFuiLabel*)GetComponent('freq');
  if (0 == lab)  gtfo(erm);
  nbr = nav->GetFrequency();
  lab->ChangeFont(font);
  lab->EditText("%3.2f Mhz",nbr);
  //-------Free the CNavaid object ------------
  Po = 0;
  return;
}
//----------------------------------------------------------------------
//  Tune Request
//  Frequency is allready set
//----------------------------------------------------------------------
void CFuiNavDetail::TuneRadioNav()
{ CVehicleObject *veh = globals->pln;
  if (0 == veh)           return;
  switch (type) {
    case VOR:
      {
      Tag radio = veh->GetNAV();
      if (0 == radio)     return;
      mesg.user.u.datatag = 'navA';
      mesg.group          = radio;
      Send_Message(&mesg);
      return;
      }

    case NDB:
      {
      Tag adf = veh->GetADF();
      if (0 == adf)       return;
      mesg.user.u.datatag = 'adfA';
      mesg.group          = adf;
      Send_Message(&mesg);
      return;
      }
}
return;
}
//----------------------------------------------------------------------
//  Notification from child button
//----------------------------------------------------------------------
void CFuiNavDetail::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ switch (itm)  {
  case 'tune':
      TuneRadioNav();
      return;
  case 'sysb':
      SystemHandler(evn);
      return;
}
return;;
}
//==================================================================================
//
//  Airport Detail
//
//==================================================================================
CFuiAptDetail::CFuiAptDetail(Tag idn,const char *filename,int lim)
:CFuiWindow(idn,filename,0,0,lim)
{ wName = "AptDetail";
  font  = &globals->fonts.ftsmal10;       // Change Font
  Apt   = 0;
  type  = 0;
  lpBox = 0;
  vers  = 0;                              // Runway version
  //----------------------------------------------------------
  char  *abt = "Incorrect FUI Airport detail file";
  //--------------Find components  ---------------------------
  grh     = (CFuiCanva*)GetComponent('canv');
  if (0 == grh)  gtfo(abt);
  lpBox   = (CFuiGroupBox*) GetComponent('rlpf');
  chk     = (CFuiCheckbox*) GetComponent('show');
  if (0 == chk)  gtfo(abt);
  //----------------------------------------------------------
  Req.SetWindow(this);
  //-- init extremities for all runways  ---------------------
	CFuiRwyEXT::Init();
  //--------Radio message for tuning --------------------------
  mesg.id = MSG_SETDATA;
}
//--------------------------------------------------------------------------
//  Destroy window
//--------------------------------------------------------------------------
CFuiAptDetail::~CFuiAptDetail()
{ Apt->RazInEDIT();
  comBOX.EmptyIt();
  rwyBOX.EmptyIt();
  return;
}
//--------------------------------------------------------------------------
//  Set APT parameters and edit
//  NOTE:  The smart pointer ensure object user accounting
//--------------------------------------------------------------------------
void CFuiAptDetail::Initialize(CmHead *obj,U_SHORT tp,U_SHORT No)
{ char *erm = "Incorrect FUI Airport detail file";
  CDbCacheMgr *dbc = globals->dbc;
  this->type  = tp;
  this->Po    = obj;
  this->Apt   = (CAirport*)obj;
  this->wptNo = No;
  Apt->SetInEDIT();
	//-------------------------------------------------
	tkoID	= "NUL";
	lndID	= "NUL";
	//-------------------------------------------------
  float nbr;
  char  edt[256];
  CFuiLabel *lab = 0;
  //-------------------------------------------------
  lab = (CFuiLabel*)GetComponent('name');
  if (0 == lab)  gtfo(erm);
  lab->ChangeFont(font);
  lab->EditText("Name:   %s",Apt->GetName());
  //-----Cntry -------------------------------------
  lab = (CFuiLabel *)GetComponent('cnty');
  if (0 == lab)  gtfo("Incorrect FUI Airport detail file");
  lab->ChangeFont(font);
  lab->EditText("Country: %s",Apt->GetCountry());
  //----Latitude ------------------------------------
  EditLat2DMS(Apt->GetLatitude(),edt);
  lab = (CFuiLabel*)GetComponent('latv');
  if (0 == lab)  gtfo(erm);
  lab->ChangeFont(font);
  lab->SetText(edt);
  //----Longitude -----------------------------------
  EditLon2DMS(Apt->GetLongitude(),edt);
  lab = (CFuiLabel*)GetComponent('lonv');
  if (0 == lab)  gtfo(erm);
  lab->ChangeFont(font);
  lab->SetText(edt);
  //------Elevation ---------------------------------
  lab = (CFuiLabel*)GetComponent('high');
  if (0 == lab)  gtfo(erm);
  nbr = Apt->GetElevation();
  lab->ChangeFont(font);
  lab->EditText("Alt %.0f ft",nbr);
  //------Init optional Light profile --------------
  if (1 == vers)  InitLightProfile();
  //------Post a request to get the com  ---
  GetAllCOM();
  return;
}
//----------------------------------------------------------------------
//  Init light profile popup
//----------------------------------------------------------------------
void CFuiAptDetail::InitLightProfile()
{ char *erm = "Incorrect DetailsAirport.win";
  //---Init lock menu -----------------------------------
  lkPP = (CFuiPopupMenu*)lpBox->GetComponent('lock');
  if (0 == lkPP)  gtfo(erm);
  //---Init HI END --------------------------------------
  hiED = (CFuiTextField*)lpBox->GetComponent('hiId');
  if (0 == hiED)  gtfo(erm);
  hiAP = (CFuiPopupMenu*)lpBox->GetComponent('hiap');
  if (0 == hiAP)  gtfo(erm);
  hiTB = (CFuiPopupMenu*)lpBox->GetComponent('hitb');
  if (0 == hiTB)  gtfo(erm);
  hiWB = (CFuiPopupMenu*)lpBox->GetComponent('hiwb');
  if (0 == hiWB)  gtfo(erm);
  hiCT = (CFuiPopupMenu*)lpBox->GetComponent('hict');
  if (0 == hiCT)  gtfo(erm);
  hiEG = (CFuiPopupMenu*)lpBox->GetComponent('hieg');
  if (0 == hiEG)  gtfo(erm);
  hiTZ = (CFuiPopupMenu*)lpBox->GetComponent('hitd');
  if (0 == hiTZ)  gtfo(erm);
  //---Init LO END -------------------------------------
  loED = (CFuiTextField*)lpBox->GetComponent('loId');
  if (0 == loED)  gtfo(erm);
  loAP = (CFuiPopupMenu*)lpBox->GetComponent('loap');
  if (0 == loAP)  gtfo(erm);
  loTB = (CFuiPopupMenu*)lpBox->GetComponent('lotb');
  if (0 == loTB)  gtfo(erm);
  loWB = (CFuiPopupMenu*)lpBox->GetComponent('lowb');
  if (0 == loWB)  gtfo(erm);
  loCT = (CFuiPopupMenu*)lpBox->GetComponent('loct');
  if (0 == loCT)  gtfo(erm);
  loEG = (CFuiPopupMenu*)lpBox->GetComponent('loeg');
  if (0 == loEG)  gtfo(erm);
  loTZ = (CFuiPopupMenu*)lpBox->GetComponent('lotd');
  if (0 == loTZ)  gtfo(erm);
  //--Set Menus ----------------------------------------
  cRpf  = 0;
  lkPP->CreatePage(&lkMEN,lkMENU);
  hiAP->CreatePage(&hiAPM,apMENU);
  hiTB->CreatePage(&hiTBM,tbMENU);
  hiWB->CreatePage(&hiWBM,wbMENU);
  hiCT->CreatePage(&hiCTM,ctMENU);
  hiEG->CreatePage(&hiEGM,ctMENU);
  hiTZ->CreatePage(&hiTZM,wbMENU);
  loAP->CreatePage(&loAPM,apMENU);
  loTB->CreatePage(&loTBM,tbMENU);
  loWB->CreatePage(&loWBM,wbMENU);
  loCT->CreatePage(&loCTM,ctMENU);
  loEG->CreatePage(&loEGM,ctMENU);
  loTZ->CreatePage(&loTZM,wbMENU);
  //--Lock all popup -----------------------------------
  LockAll();
  globals->apm->SetRunwayProfile(Apt);
  return;
}

//----------------------------------------------------------------------
//  Lock all
//----------------------------------------------------------------------
void CFuiAptDetail::LockAll()
{ lock  = 1;
  hiAP->Lock();
  hiTB->Lock();
  hiWB->Lock();
  hiCT->Lock();
  hiEG->Lock();
  hiTZ->Lock();
  loAP->Lock();
  loTB->Lock();
  loWB->Lock();
  loCT->Lock();
  loEG->Lock();
  loTZ->Lock();
  return;
}
//----------------------------------------------------------------------
//  UnLock all
//----------------------------------------------------------------------
void CFuiAptDetail::UnlockAll()
{ lock   = 0;
  hiAP->Unlock();
  hiTB->Unlock();
  hiWB->Unlock();
  hiCT->Unlock();
  hiEG->Unlock();
  hiTZ->Unlock();
  loAP->Unlock();
  loTB->Unlock();
  loWB->Unlock();
  loCT->Unlock();
  loEG->Unlock();
  loTZ->Unlock();
  return;
}
//----------------------------------------------------------------------
//  Draw Light profile of selected runway
//----------------------------------------------------------------------
void CFuiAptDetail::DrawProfile()
{ if (0 == vers)		return;
  CRwyLine *slot = (CRwyLine*)rwyBOX.GetSelectedSlot();
	if (0 == slot)		return;
  CRunway  *rwy  = Apt->FindRunway(slot->GetHiEndID());
  if (0 == rwy)     return;
	int opt = 0;
  CRLP     *lpf  = rwy->GetRLP();
  int       ctr  = lpf->GetCenterLM();
  int       edg  = lpf->GetEdgeLM();
  //---Save current runway ---------------------------
  cRwy = rwy;
  cRpf = lpf;
  //---Set Hi end options ----------------------------
  hiED->SetText(slot->GetHiEndID());
  loED->SetText(slot->GetLoEndID());
  opt  = lpf->GetHiAPRL();
  hiAP->Select(opt);
  opt  = lpf->GetHiTBAR();
  hiTB->Select(opt);
  opt  = lpf->GetHiWBAR();
  hiWB->Select(opt);
  opt  = lpf->GetHiTDZN();
  hiTZ->Select(opt);
  opt  = lpf->GetModel(ctr,TC_LITE_HICAT);
  hiCT->Select(opt);
  opt  = lpf->GetModel(edg,TC_LITE_HICAT);
  hiEG->Select(opt);
  //---Set Lo end options ----------------------------
  opt  = lpf->GetLoAPRL();
  loAP->Select(opt);
  opt  = lpf->GetLoTBAR();
  loTB->Select(opt);
  opt  = lpf->GetLoWBAR();
  loWB->Select(opt);
  opt  = lpf->GetLoTDZN();
  loTZ->Select(opt);
  opt  = lpf->GetModel(ctr,TC_LITE_LOCAT);
  loCT->Select(opt);
  opt  = lpf->GetModel(edg,TC_LITE_LOCAT);
  loEG->Select(opt);
  return;
}
//----------------------------------------------------------------------
//  Post a request to get all com for this airport
//----------------------------------------------------------------------
void CFuiAptDetail::GetAllCOM()
{ comBOX.SetParameters(this,'freq',0);
  Req.SetReqCode(COM_BY_AIRPORT);
  Req.SetReqMask(COMM_ALL);
  Req.SetAPT(Apt->GetKey());
  PostRequest(&Req);
  return;
}
//----------------------------------------------------------------------
//      Post a request to get Runways
//----------------------------------------------------------------------
void CFuiAptDetail::GetRunway()
{ 
  rwyBOX.SetParameters(this,'runs',0);
  Req.SetReqCode(RWY_BY_AIRPORT);
  PostRequest(&Req);
  return;
}
//----------------------------------------------------------------------
//  Result return from database
//----------------------------------------------------------------------
void CFuiAptDetail::EndOfRequest(CDataBaseREQ *req)
{ req->SetReqCode(NO_REQUEST);
  switch (Req.GetOrder())  {

    case COM_BY_AIRPORT:
      GetRunway();
      return;

    case RWY_BY_AIRPORT:
      Terminate();
      return;
  }
  return;
}
//----------------------------------------------------------------------
//  Terminate Process 
//----------------------------------------------------------------------
void CFuiAptDetail::Terminate()
{	chk->SetState(1);
  comBOX.Display();
  rwyBOX.Display();
  ComputeScale();
  ScaleAllRWY();
  DrawRunways();
	DrawProfile();
  //-------Free the object -----------------------------------------
  Po  = 0;
  return;
}


//----------------------------------------------------------------------------------
//  Draw 
//  -Dispatch end of database request first
//----------------------------------------------------------------------------------
void CFuiAptDetail::Draw()
{ if (Req.EndOfReq())  EndOfRequest(&Req);
	U_INT sel = rwyBOX.GetSelectedNo();
  CFuiWindow::Draw();
  return;
}
//-------------------------------------------------------------------------
//  Tune com radio to the frequency of selected com
//-------------------------------------------------------------------------
bool CFuiAptDetail::TuneRadioCom()
{ CComLine *slot  = (CComLine*)comBOX.GetSelectedSlot();
  if (0 == slot)      return true;
  CVehicleObject *veh = globals->pln;
  if (0 == veh)       return false;
  Tag radio = 0; 
  Tag tag   = 0;
  mesg.user.u.hw      = HW_RADIO;
  //---Check for a COM radio -------------
  if (0 == tag)
  { tag = slot->IsaILS();
    radio = veh->GetNAV();
  }
  if (0 == tag)
  { tag = slot->IsaCOM();
    radio = veh->GetCOM();
  }
  if (0 == radio)     return true;
  if (0 == tag)       return true;
  //----Tune radio or com ----------------
  mesg.group          = radio;
  mesg.receiver       = 0;
  mesg.realData       = slot->GetFreq();
  mesg.user.u.datatag = tag;
  Send_Message(&mesg);
  return true;
}
//-------------------------------------------------------------------------
//  Event notification
//-------------------------------------------------------------------------
void  CFuiAptDetail::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ switch (idm)  {
  case 'freq':
    comBOX.VScrollHandler((U_INT)itm,evn);
    return;

  case 'runs':
    rwyBOX.VScrollHandler((U_INT)itm,evn);
    DrawRunways();
		DrawProfile();
    return;

  case 'show':
    DrawRunways();
    return;

  case 'tune':
    TuneRadioCom();
    return;

  case 'sysb':
    if ((EVENT_CLOSEWINDOW == evn) && Po.Assigned()) Apt->SaveProfile();
    SystemHandler(evn);
    return;

  }
  return;
}
//----------------------------------------------------------------------------------
//  Notification from Popup
//----------------------------------------------------------------------------------
void CFuiAptDetail::NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn)
{ U_INT   No = (U_INT)itm;
  switch (id) {
    case 'lock':
      if (0 == No) LockAll();
      if (1 == No) UnlockAll();
      lkPP->SetButtonText((char*)lkMEN.aText[No]);
      return;
  }
  if (evn != EVENT_POP_CLICK) return;
  switch (id) {
    case 'hiap':
      cRpf->ChangeHiAPR(No);
      ChangeProfile(*hiAP,No);
      return;
    case 'loap':
      cRpf->ChangeLoAPR(No);
      ChangeProfile(*loAP,No);
      return;
    case 'hitb':
      cRpf->ChangeHiTBR(No);
      ChangeProfile(*hiTB,No);
      return;
    case 'lotb':
      cRpf->ChangeLoTBR(No);
      ChangeProfile(*loTB,No);
      return;
    case 'hiwb':
      cRpf->ChangeHiWBR(No);
      ChangeProfile(*hiWB,No);
      return;
    case 'lowb':
      cRpf->ChangeLoWBR(No);
      ChangeProfile(*loWB,No);
      return;
    case 'hitd':
      cRpf->ChangeHiTDZ(No);
      ChangeProfile(*hiTZ,No);
      return;
    case 'lotd':
      cRpf->ChangeLoTDZ(No);
      ChangeProfile(*loTZ,No);
      return;
    case 'hict':
      cRpf->ChangeHiCTR(No);
      ChangeProfile(*hiCT,No);
      return;
    case 'loct':
      cRpf->ChangeLoCTR(No);
      ChangeProfile(*loCT,No);
      return;
    case 'hieg':
      cRpf->ChangeHiEDG(No);
      ChangeProfile(*hiEG,No);
      return;
    case 'loeg':
      cRpf->ChangeLoEDG(No);
      ChangeProfile(*loEG,No);
      return;

    default:
      break;

}
return;
}
//----------------------------------------------------------------------------------
//  Change profile
//----------------------------------------------------------------------------------
void CFuiAptDetail::ChangeProfile(CFuiPopupMenu &pop,int No)
{ FL_MENU *menu = pop.GetMenu();
  pop.SetButtonText((char*)menu->aText[No]);
  Apt->RebuildLights(cRwy);
  lpBox->SetChildText('txt1',"Changes will be saved on closing");
  return;
}
//----------------------------------------------------------------------------------
//  Database Management
//    -Activated by Database management to add one record
//----------------------------------------------------------------------------------
void CFuiAptDetail::AddDBrecord(void *rec,DBCODE code)
{ CSlot *slot = (CSlot*)rec;
  switch (code) {

    case COM_BY_AIRPORT:
      comBOX.AddSlot(slot);
      return;
 
    case ILS_BY_AIRPORT:
      comBOX.AddSlot(slot);
      return;

    case RWY_BY_AIRPORT:
      CRwyLine *rwy = (CRwyLine*)slot;
      rwyBOX.AddSlot(slot);
      StoreExtremities(rwy->GetDXH(),rwy->GetDYH());
      StoreExtremities(rwy->GetDXL(),rwy->GetDYL());
      return;
  }
  return;
}

//==============END of FILE ====================================================
