/*
 * WindowMap.cpp
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

/*! \file WindowDirectory
 *  \brief Implements FUI callback for Directory window
 *
 *  Implements the FUI callback for the directory window which display and search
 *    for airport/navaid/waypoints...
 */

#include "../Include/FlyLegacy.h"
#include "../Include/Fui.h"
#include "../Include/FuiUser.h"
#include "../Include/FuiParts.h"
#include "../Include/Globals.h"
#include "../Include/Database.h"
#include "../Include/Pod.h"
#include <algorithm>
#include <vector>
#include <stdarg.h>
//==================================================================================
#define WDIR_SEL_APT 0
#define WDIR_SEL_VOR 1
#define WDIR_SEL_NDB 2
#define WDIR_SEL_WPT 3
//----------------------------------------------------------------------------------
//  MENU FOR GENERAL DIRECTORY
//----------------------------------------------------------------------------------
char *DirMENU[] = {
  "AIRPORT",
  "VOR",
  "NDB",
  "WAYPOINT",
  "",
};
//=======================================================================================
//
//    TELEPORT WINDOW
//
//=======================================================================================
CFuiTeleport::CFuiTeleport(Tag idn, const char* filename)
:CFuiWindow(idn,filename,0,0,0)
{
  dir = NULL;
}
//--------------------------------------------------------------------------
//  Destroy the window
//--------------------------------------------------------------------------
CFuiTeleport::~CFuiTeleport()
{ 
}
//-------------------------------------------------------------------------
//  EVENT notifications
//-------------------------------------------------------------------------
void  CFuiTeleport::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{
  switch (idm)  {
  case 'dirc':
    {CFuiDirectory *dir = globals->dbc->GetDIRwindow();
    if (NULL == dir)
      dir = (CFuiDirectory *)globals->fui->CreateFuiWindow(FUI_WINDOW_WAYPOINT_DIRECTORY);
    if (NULL != dir)
      dir->RegisterMe(this);
    }
    return;
  case 'goto':
    {if (!globals->fui->IsWindowCreated (FUI_WINDOW_ALERT_MSG)) {
      globals->fui->CreateFuiWindow  (FUI_WINDOW_ALERT_MSG);
      globals->fui->SetComponentText (FUI_WINDOW_ALERT_MSG, 'mesg', "  -- !!!  NOT INSTALLED  !!! --");
    } else {
      globals->fui->SetComponentText (FUI_WINDOW_ALERT_MSG, 'mesg', "  -- !!!  NOT INSTALLED  !!! --");
    }
    }
    return;
  default:
    CFuiWindow::NotifyChildEvent (idm, itm, evn);
  }
  return;
}
//=======================================================================================
//
//    GENERAL DIRECTORY
//
//=======================================================================================
CFuiDirectory::CFuiDirectory(Tag idn, const char* filename)
:CFuiWindow(idn,filename,0,0,0)
{ //------Init Airport ownership -------------------------------------
  apOWN[0]  = "UNKNOWN";
  apOWN[1]  = "PUBLIC";
  apOWN[2]  = "PRIVATE";
  apOWN[3]  = "AIRFORCE";
  apOWN[4]  = "NAVY";
  apOWN[5]  = "ARMY";
  //-----------------------------------------------------------------
  char *abt = "Incorrect FLIGHTPLANNERDIRECTORY.WIN file";
  //------Other variables -------------------------------------------
  dbc = globals->dbc;
  dbc->RegisterDIRwindow(this);
  Req.SetWindow(this);
  hBox      = (CFuiLabel*)    GetComponent('xOFn');
  if (0 == hBox)  gtfo(abt);
  cBox      = (CFuiCheckbox*) GetComponent('cont');
  if (0 == cBox)  gtfo(abt);
  nBox      = (CFuiTextField*)GetComponent('name');
  if (0 == nBox)  gtfo(abt);
  iBox      = (CFuiTextField*)GetComponent('wpid');
  if (0 == iBox)  gtfo(abt);
  selPop    = (CFuiPopupMenu*)GetComponent('type');
  if (0 == selPop)gtfo(abt);
  sBut      = (CFuiButton*)   GetComponent('wsel');
  if (0 == sBut)  gtfo(abt);
  sBut->SetText("Close");
  cWin      = 0;
  selOpt    = WDIR_SEL_APT;
  //------Resize properties -----------------------------------------
  SetXRange(w,w);               // Fixed width
  SetProperty(FUI_VT_RESIZING);
  //-------Populate lists -------------------------------------------
  selPop->CreatePage(&mSEL,DirMENU);
  Lock  = 1;
  FillCTYlist();
}

//----------------------------------------------------------------------------------
//  No more caller. Change select to close
//----------------------------------------------------------------------------------
bool CFuiDirectory::ResetCaller()
{ cWin  = 0;
  sBut->SetText("Close");
  return true;
}
//----------------------------------------------------------------------------------
//  Save calling window to manage Flight Plan
//----------------------------------------------------------------------------------
bool  CFuiDirectory::RegisterMe(CFuiWindow *win)
{ if (0 == win)     return ResetCaller();
  if (cWin)         return false;
  cWin  = win;
  sBut->SetText("Add to FP");
  return true;
}
//==================================================================================
//  POPUP management
//==================================================================================
//----------------------------------------------------------------------------------
//  Set Popup title
//----------------------------------------------------------------------------------
void CFuiDirectory::SetPopTitle(U_SHORT No)
{ selOpt  = No;
  selPop->SetButtonText((char*)mSEL.aText[No]);
  return;
}
//--------------------------------------------------------------------------
//  Destroy the window
//--------------------------------------------------------------------------
CFuiDirectory::~CFuiDirectory()
{ ctyBOX.EmptyIt();
  staBOX.EmptyIt();
  objBOX.EmptyIt();
  dbc->RegisterDIRwindow(0);
}
//=========================================================================
//  Country Management
//=========================================================================

//-------------------------------------------------------------------------
//  Request database to fill the country list
//-------------------------------------------------------------------------
void CFuiDirectory::FillCTYlist()
{ //-------Init country list --------------------------
  ctyBOX.SetParameters(this,'clst',0);
  ctyLIN.FixeIt();
  ctyLIN.SetName("ALL COUNTRIES");
  ctyLIN.SetKey("");
  ctyBOX.AddSlot(&ctyLIN);
  //-------InitObject list ----------------------------
  objBOX.SetParameters(this,'list',0);
  //-------Init State list ----------------------------
  staBOX.SetParameters(this,'slst',0);
  staLIN.FixeIt();
  staBOX.AddSlot(&staLIN);
  //------Request counties to database -----------------
  Req.ClearFilter();
  Req.SetReqCode(CTY_BY_ALLLOT);
  PostRequest(&Req);
  return;
}
//-------------------------------------------------------------------------
//  A country is selected. Retreive the Key
//-------------------------------------------------------------------------
void CFuiDirectory::RetreiveCTYkey()
{ Req.SetCTY("");
  CSlot *slot = ctyBOX.GetSelectedSlot();
  if (0 == slot)  return;
  Req.SetCTY(slot->GetKey());
  return;
}
//-------------------------------------------------------------------------
//  Find a country
//-------------------------------------------------------------------------
char *CFuiDirectory::GetCountry(char *ckey)
{ U_INT No  = 0;
  U_INT end = ctyBOX.GetSize();
  CSlot *slot;
  while (No != end)
  { slot = ctyBOX.GetSlot(No++);
    if (strcmp(slot->GetKey(),ckey) == 0)  return slot->GetName();
  }
  return 0;
}
//=========================================================================
//  State Management
//=========================================================================
//-------------------------------------------------------------------------
//  Fill the state list
//  Add one extra slot for header
//-------------------------------------------------------------------------
void CFuiDirectory::FillSTAlist()
{ staBOX.EmptyIt();
  staLIN.SetName("NO STATE");
  staLIN.SetKey("");
  staBOX.AddSlot(&staLIN);
  RetreiveCTYkey();
  //---Only USA has state for now ------------------
  if (Req.IsNotForUS()) 
  { staBOX.SortAndDisplay();
    Lock  = 0;
    return; }
  Req.SetReqCode(STA_BY_COUNTRY);
  PostRequest(&Req);
  return;
}
//-------------------------------------------------------------------------
//  Sort the State list and display
//-------------------------------------------------------------------------
void CFuiDirectory::SortSTAlist()
{ if (staBOX.GetSize() > 1) staLIN.SetName("ALL STATES");
  staBOX.SortAndDisplay();
  return;
}
//-------------------------------------------------------------------------
//  Set the filter
//-------------------------------------------------------------------------
void CFuiDirectory::SetFilter(QTYPE type)
{ Req.ClearFilter();
  CSlot *slot = ctyBOX.GetSelectedSlot();
  if (slot)   Req.SetCTY(slot->GetKey());
  slot        = staBOX.GetSelectedSlot();
  if (slot)   Req.SetSTA(slot->GetKey());
  if (type == NDB)  Req.SelectNDB();
  if (type == VOR)  Req.SelectVOR();
  if (0 == cBox->GetState())  return;
  Req.SetST1(nBox->GetText());
  Req.SetST2(iBox->GetText());
  return;
}
//=========================================================================
//  Object Management
//=========================================================================
//-------------------------------------------------------------------------
//  Sort object list and display
//-------------------------------------------------------------------------
void CFuiDirectory::SortOBJlist()
{ char *cty = 0;
  CSlot *slot = ctyBOX.GetSelectedSlot();
  cty   = slot->GetName();
  hBox->EditText("%s: %u %s FOUND",cty,objBOX.GetSize(),mSEL.aText[selOpt]);
  objBOX.SortAndDisplay();
  return;
}
//=========================================================================
//  Airport Management
//=========================================================================

//-------------------------------------------------------------------------
//  Search all airport for a given country
//-------------------------------------------------------------------------
void CFuiDirectory::SearchAirportByCountry(QTYPE type)
{ objBOX.EmptyIt();
  oType = type;
  SetFilter(type);
  Req.SetReqCode(APT_BY_FILTER);
  PostRequest(&Req);
  return;
}
//-------------------------------------------------------------------------
//  Complement Airport Data
//-------------------------------------------------------------------------
void CFuiDirectory::SetOtherAptData(CAptLine *slot)
{ slot->SetOwtx(apOWN[slot->GetAown()]);      // Ownership
  slot->SetClab("");
  if (Req.HasCTY())   return;
  char *cty = GetCountry(slot->GetActy());
  if (0 == cty)       return;
  slot->SetClab(cty);
  return;
}

//=========================================================================
//  NAVAID MANAGEMENT
//=========================================================================
//-------------------------------------------------------------------------
//  Search all Navaid for a given country
//-------------------------------------------------------------------------
void CFuiDirectory::SearchNavaidByCountry(QTYPE type)
{ objBOX.EmptyIt();
  oType = type;
  SetFilter(type);
  Req.SetReqCode(NAV_BY_FILTER);
  PostRequest(&Req);
  return;
}
//-------------------------------------------------------------------------
//  Complement Nav Data
//-------------------------------------------------------------------------
void CFuiDirectory::SetOtherNavData(CNavLine *slot)
{ U_SHORT type  = slot->GetType();
  U_SHORT inx   = dbc->GetNavType(type);
  slot->SetClab("");
  char *ntyp = globals->vorTAB[inx];
  slot->SetVtyp(ntyp);
  if (Req.HasCTY())   return;
  char *cty = GetCountry(slot->GetVcty());
  if (0 == cty)       return;
  slot->SetClab(cty);
  return;
}
//-------------------------------------------------------------------------
//  return Index type for a  navaid
//-------------------------------------------------------------------------
char *CFuiDirectory::GetNavType(char type)
{     switch (type) {
      case NAVAID_TYPE_VOR:
        return "VOR";
      case NAVAID_TYPE_TACAN:
        return "TACAN";
      case NAVAID_TYPE_VORTAC:
        return "VORTAC";
      case NAVAID_TYPE_VORDME:
        return "VOR-DME";
      case NAVAID_TYPE_NDB:
        return "NDB";
      case NAVAID_TYPE_NDBDME:
        return "NDB-DME";
  }
  return "???";
}
//=========================================================================
//  WAYPOINT MANAGEMENT
//=========================================================================
void  CFuiDirectory::SearchWaypointByCountry(QTYPE type)
{ //----Temporary ------------------------------------
  objBOX.EmptyIt();
  oType = type;
  SetFilter(type);
  Req.SetReqCode(WPT_BY_FILTER);
  PostRequest(&Req);
  return;
}
//-------------------------------------------------------------------------
//  Complement wpt Data
//-------------------------------------------------------------------------
void CFuiDirectory::SetOtherWptData(CWptLine *slot)
{ slot->SetClab("");
  if (Req.HasCTY())   return;
  char *cty = GetCountry(slot->GetWcty());
  if (0 == cty)       return;
  slot->SetClab(cty);
  return;
}
//=========================================================================
//  Draw the directory
//=========================================================================
void CFuiDirectory::Draw()
{ if (Req.EndOfReq())  EndOfRequest(&Req); 
  CFuiWindow::Draw();
  return;
}

//-------------------------------------------------------------------------
//  START search according to selector
//-------------------------------------------------------------------------
void CFuiDirectory::Search()
{ Lock = 1;
  switch (selOpt) {
  case WDIR_SEL_APT:
      SearchAirportByCountry(APT);
      return;
  case WDIR_SEL_VOR:
      SearchNavaidByCountry(VOR);
      return;
  case WDIR_SEL_NDB:
      SearchNavaidByCountry(NDB);
      return;
  case WDIR_SEL_WPT:
      SearchWaypointByCountry(WPT);
      return;
}
  Lock = 0;
  return;
}
//-------------------------------------------------------------------------
//  Open VOR detail
//-------------------------------------------------------------------------
bool CFuiDirectory::GetVORobject()
{ Req.SetReqCode(NAV_BY_OFFSET);
  PostRequest(&Req);
  return true;
}
//-------------------------------------------------------------------------
//  Open NDB detail
//-------------------------------------------------------------------------
bool CFuiDirectory::GetNDBobject()
{ Req.SetReqCode(NDB_BY_OFFSET);
  PostRequest(&Req);
  return true;
}
//-------------------------------------------------------------------------
//  Open Airport detail
//-------------------------------------------------------------------------
bool CFuiDirectory::GetAPTobject()
{ Req.SetReqCode(APT_BY_OFFSET);
  PostRequest(&Req);
  return true;
}
//-------------------------------------------------------------------------
//  Open a detail object window
//-------------------------------------------------------------------------
bool CFuiDirectory::GetNAVObject()
{ CSlot *slot = objBOX.GetSelectedSlot();
  if (0 == slot)  return true;
  Req.SetOFS(slot->GetOFS());
  char *key = slot->GetKey();
  Req.SetAPT(key);
  switch (oType)  {
  case VOR:
    return GetVORobject();
  case NDB:
    return GetNDBobject();
  case APT:
    return GetAPTobject();
}
  return true ;
}
//-------------------------------------------------------------------------
//  Teleport to selected waypoint
//-------------------------------------------------------------------------
void CFuiDirectory::Teleport()
{ CSlot *slot = objBOX.GetSelectedSlot();
  if (0 == slot)    return;
  if (Lock)         return;
  SPosition pos;
  slot->GetPosition(pos);
  globals->tcm->Teleport(pos);
  return;
}
//-------------------------------------------------------------------------
//  Laod METAR if an airport is selected
//-------------------------------------------------------------------------
void CFuiDirectory::ShowMetar()
{ CSlot *slot = objBOX.GetSelectedSlot();
  if (0 == slot)                  return;
  if (WDIR_SEL_APT != selOpt)     return;
  char *idn = ((CAptLine*)slot)->GetAica();
  CFuiStrip *win = (CFuiStrip*)globals->fui->CreateFuiWindow(FUI_WINDOW_STRIP,0);
  win->SetIdent(idn);
  return;
}
//--------------------------------------------------------------------------
//  Resize notification
//--------------------------------------------------------------------------
void CFuiDirectory::NotifyResize(short dx,short dy)
{ if (Lock)  return;
  objBOX.Resize();
  return;
}
//--------------------------------------------------------------------------
//  Event on country list
//--------------------------------------------------------------------------
void CFuiDirectory::EventCountryBox(Tag itm,EFuiEvents evn)
{ U_INT old = ctyBOX.GetSelectedNo();
  ctyBOX.VScrollHandler((U_INT)itm,evn);
  if (ctyBOX.GetSelectedNo() != old)  FillSTAlist();
  if (evn == EVENT_DBLE_CLICK) Search();
  return;
}
//-------------------------------------------------------------------------
//  EVENT notifications
//-------------------------------------------------------------------------
void  CFuiDirectory::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ 
  if (Lock)         return;
  switch (idm)  {
  case 'clst':
      EventCountryBox(itm,evn);
      return;
  case  'list':
      objBOX.VScrollHandler((U_INT)itm,evn);
      return;
  case 'slst':
      staBOX.VScrollHandler((U_INT)itm,evn);
      return;
  case 'okok': 
      if (0 == Lock)  Search();
      return;
  case 'detl':
    Order = 'detl';
    GetNAVObject();
    return;
  case 'sysb':
    SystemHandler(evn);
    return;
  case 'wsel':
    Order = 'wsel';
    if (cWin) GetNAVObject();
    else  Close();
    return;
  case 'warp':
    Teleport();
		Close();
    return;
  case 'mtar':
    ShowMetar();
    return;

  }
return;
}
//--------------------------------------------------------------------------
//  Notification from popup
//--------------------------------------------------------------------------
void CFuiDirectory::NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn)
{ if (evn == EVENT_SELECTITEM) SetPopTitle((U_INT)itm);
  return;
}
//==========================================================================
//  DATABASE Management
//==========================================================================
//  Record coming from database
//-------------------------------------------------------------------------
void CFuiDirectory::AddDBrecord(void *rec,DBCODE code)
{ CSlot *slot = (CSlot *)rec;
  switch (code) {
  case CTY_BY_ALLLOT:
      ctyBOX.AddSlot(slot);
      return;
  case APT_BY_FILTER:
      SetOtherAptData((CAptLine*)slot);
      objBOX.AddSlot(slot);
      return;
  case NAV_BY_FILTER:
      SetOtherNavData((CNavLine*)slot);
      objBOX.AddSlot(slot);
      return;
  case WPT_BY_FILTER:
      SetOtherWptData((CWptLine*)slot);
      objBOX.AddSlot(slot);
      return;
  case STA_BY_COUNTRY:
      staBOX.AddSlot(slot);
      return;
  case NAV_BY_OFFSET:
      selOBJ  = (CmHead*)rec;
      return;
  case NDB_BY_OFFSET:
      selOBJ  = (CmHead*)rec;
      return;
  case APT_BY_OFFSET:
      selOBJ  = (CmHead*)rec;
      return;
  }
  return;
}
//-------------------------------------------------------------------------
//  Notify the caller if any
//-------------------------------------------------------------------------
void CFuiDirectory::NotifyCaller(CmHead *obj,QTYPE type)
{ if (cWin) cWin->NotifyFromDirectory(obj);
  else delete obj;
  return;
}
//-------------------------------------------------------------------------
//  A request is terminated
//-------------------------------------------------------------------------
void CFuiDirectory::EndOfRequest(CDataBaseREQ *req)
{ req->SetReqCode(NO_REQUEST);
  switch  (Req.GetOrder())  { 
  case CTY_BY_ALLLOT:
      ctyBOX.SortAndDisplay();
      FillSTAlist();
      return;
  case APT_BY_FILTER:
      SortOBJlist();
      Lock  = 0;
      return;
  case NAV_BY_FILTER:
      SortOBJlist();
      Lock  = 0;
      return;
  case WPT_BY_FILTER:
      SortOBJlist();
      Lock = 0;
      return;
  case STA_BY_COUNTRY:
      SortSTAlist();
      Lock  = 0;
      return;
  case NAV_BY_OFFSET:
      if (Order == 'detl')  SmallDetailObject(selOBJ,0);
      if (Order == 'wsel')  NotifyCaller(selOBJ,VOR);
      selOBJ  = 0;
      return;
  case NDB_BY_OFFSET:
      if (Order == 'detl')  SmallDetailObject(selOBJ,0);
      if (Order == 'wsel')  NotifyCaller(selOBJ,NDB);
      selOBJ  = 0;
      return;
  case APT_BY_OFFSET:
      if (Order == 'detl')  SmallDetailObject(selOBJ,0);
      if (Order == 'wsel')  NotifyCaller(selOBJ,APT);
      selOBJ  = 0;
      return;
  }
  return;
}
//============================END OF FILE ================================================================================
