/*
 * WindowNavLog.cpp
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
#include "../Include/FlyLegacy.h"
#include "../Include/Utility.h"
#include "../Include/Globals.h"
#include "../Include/Fui.h"
#include "../Include/FuiUser.h"
#include "../Include/FuiParts.h"
#include "../Include/FlightPlan.h"
//==================================================================================
//
//  NAVIGATION LOG:  List a detailled Flight Plan
//
//==================================================================================
CFuiFlightLog::CFuiFlightLog(Tag idn, const char* filename)
:CFuiWindow(idn,filename,0,0,0)
{ char *erm = "Incorrect FlightPlanLog.WIN file";
  fplan = globals->fpl;
  wpt   = 0;
  //----- Get components ------------------------------
  eWIN  = (CFuiLabel *)   GetComponent('eror');
  if (0 == eWIN)  gtfo(erm);
  nWIN  = (CFuiTextField*)GetComponent('name');
  if (0 == nWIN)  gtfo(erm);
  dWIN  = (CFuiTextField*)GetComponent('desc');
  if (0 == dWIN)  gtfo(erm);
  sWIN  = (CFuiTextField*)GetComponent('sped');
  if (0 == sWIN)  gtfo(erm);
  rWIN  = (CFuiTextField*)GetComponent('drem');
  if (0 == rWIN)  gtfo(erm);
  //-----------Init the list box-----------------------
  eWIN->SetText("");
  flpBOX     = fplan->GetFBOX();
  U_INT type = LIST_HAS_TITLE + LIST_NOHSCROLL;
  flpBOX->SetParameters(this,'list',type);
  FillCurrentPlan();
  globals->dbc->RegisterLOGwindow(this);
}
//--------------------------------------------------------------------------
//  Destroy the nav log
//--------------------------------------------------------------------------
CFuiFlightLog::~CFuiFlightLog()
{ 
}
//--------------------------------------------------------------------------
//  Fill the current flight plan
//--------------------------------------------------------------------------
void CFuiFlightLog::FillCurrentPlan()
{ nWIN->SetText(fplan->GetFileName());
  dWIN->SetText(fplan->GetDescription());
  flpBOX->Display();
  return;
}
//--------------------------------------------------------------------------
//  Refresh display
//--------------------------------------------------------------------------
void CFuiFlightLog::Refresh()
{ float spd = fplan->GetSpeed();
  sWIN->EditText("%.1fKt)",spd);
  //---Refresh remaining distance -----------
  float dis = fplan->GetRemainingNM();
  flpBOX->Refresh();
  rWIN->EditText("%.1fnm",dis);
  return;
}
//-------------------------------------------------------------------------
//  Draw the navigation log
//-------------------------------------------------------------------------
void CFuiFlightLog::Draw()
{ CFuiWindow::Draw();
  return;
}
//-------------------------------------------------------------------------
//  Open detail window on item selected
//-------------------------------------------------------------------------
bool CFuiFlightLog::OpenDetail()
{ CFlpLine  *lin = (CFlpLine*)flpBOX->GetPrimary();
  if (0 == lin)               return false;
  CWayPoint *wpt = lin->GetWPT();
  CmHead *obj    = wpt->GetDBobject();
  if (0 == obj)               return false;
  return  FullDetailObject(obj,wpt->GetWaypointNo());
}
//-------------------------------------------------------------------------
//  Open the directory window and wait for action
//-------------------------------------------------------------------------
void CFuiFlightLog::OpenDirectory()
{ CFuiDirectory *dir = globals->dbc->GetDIRwindow();
  if (NULL == dir)
    dir = (CFuiDirectory *)globals->fui->CreateFuiWindow(FUI_WINDOW_WAYPOINT_DIRECTORY);
  if (NULL != dir)
    dir->RegisterMe(this);
  return;
}
//-------------------------------------------------------------------------
//  Insert the waypoint in flight plan
//  -Update the previous and next node
//  -Refresh the list
//-------------------------------------------------------------------------
void  CFuiFlightLog::InsertWaypoint(CWayPoint *wp)
{ CFlpLine   *slot  = new CFlpLine;
  slot->SetWPT(wp);
  slot->Edit();
  flpBOX->InsSlot(slot);
  fplan->Recycle();
  return;
}
//-------------------------------------------------------------------------
//  Create an Airport waypoint
//-------------------------------------------------------------------------
void  CFuiFlightLog::CreateAPTwaypoint()
{ CAirport  *apt = (CAirport*)selOBJ.Pointer();
  CWayPoint *wpt = new CWayPoint(fplan);
  wpt->SetType('sarw');
  wpt->SetName    (apt->GetName());
  wpt->SetPosition(apt->GetPosition());
  wpt->SetDbKey   (apt->GetKey());
  wpt->SetAltitude(apt->GetElevation());
  wpt->SetOBJ(apt);
  InsertWaypoint(wpt);
  return;
}
//-------------------------------------------------------------------------
//  Create an NAVAID waypoint
//-------------------------------------------------------------------------
void  CFuiFlightLog::CreateNAVwaypoint()
{ CNavaid   *nav = (CNavaid*)selOBJ.Pointer();
  CWayPoint *wpt = new CWayPoint(fplan);
  wpt->SetType('snav');
  wpt->SetName    (nav->GetName());
  wpt->SetPosition(nav->GetPosition());
  wpt->SetDbKey   (nav->GetDbKey());
  wpt->SetAltitude(nav->GetElevation());
  wpt->SetOBJ(nav);
  InsertWaypoint(wpt);
  return;
}

//-------------------------------------------------------------------------
//  Free Object. Assign 0 to smart pointer
//-------------------------------------------------------------------------
bool  CFuiFlightLog::FreeObject()
{ selOBJ  = 0;
  return true;
}
//-------------------------------------------------------------------------
//  Check for a valid insert
//  We cannot insert if the next waypoint is terminated
//-------------------------------------------------------------------------
bool CFuiFlightLog::ValidInsert()
{ CFlpLine *prm  = (CFlpLine*)flpBOX->NextPrimToSelected();
  if (0 == prm)     return true;    // Empty => valid insert
  CWayPoint *wpt = prm->GetWPT();
  bool ok = !wpt->IsTerminated();
  if (ok)           return true;
  char *msg = "CANNOT ADD BEFORE NON TERMINATED WAYPOINT";
  eWIN->RedText(msg);
  return false;
}
//-------------------------------------------------------------------------
//  Add a new waypoint to Flight plan
//  This request is processed after a waypoint is selected from the
//  Directory
//-------------------------------------------------------------------------
bool CFuiFlightLog::NotifyFromDirectory(CmHead *obj)
{ selOBJ  = obj;                            // Assign object
  if (!ValidInsert())   return FreeObject();
  switch (obj->GetActiveQ()) {
    case APT:
      CreateAPTwaypoint();
      return true;
    case VOR:
      CreateNAVwaypoint();
      return true;
    case NDB:
      CreateNAVwaypoint();
      return true;
  }
  return true;
}
//-------------------------------------------------------------------------
//  Check for a valid manipulations
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//  Send error 3
//-------------------------------------------------------------------------
void CFuiFlightLog::Error3()
{ char *msg = "CANNOT DELETE A TERMINATED WAYPOINT";
  eWIN->RedText(msg);
  return;
}
//-------------------------------------------------------------------------
//  Send error 2
//-------------------------------------------------------------------------
void CFuiFlightLog::Error2()
{ char *msg = "CANNOT MOVE ABOVE A TERMINATED WAYPOINT";
  eWIN->RedText(msg);
  return;
}
//-------------------------------------------------------------------------
//  Send error 1
//-------------------------------------------------------------------------
void CFuiFlightLog::Error1()
{ char *msg = "CANNOT MOVE A TERMINATED WAYPOINT";
  eWIN->RedText(msg);
  return;
}
//-------------------------------------------------------------------------
//  Delete the selected Waypoint
//-------------------------------------------------------------------------
void CFuiFlightLog::DeleteWaypoint()
{ if (flpBOX->IsEmpty())              return;
  CFlpLine *lin = (CFlpLine*)flpBOX->GetPrimary();
  if (lin->GetWPT()->IsTerminated())  return Error3();
  flpBOX->DeleteItem();
  fplan->Recycle();
  return;
}
//-------------------------------------------------------------------------
//  Move up current selected waypoint
//-------------------------------------------------------------------------
void CFuiFlightLog::MoveUpWaypoint()
{ //--- Check that selected waypoint may move ---------------
  if (flpBOX->IsEmpty())                    return;
  CFlpLine *lin = (CFlpLine*)flpBOX->GetPrimary();
  if (lin->GetWPT()->IsTerminated())        return Error1();
  //--- Check that the previous is not terminated -----------
  lin = (CFlpLine*)flpBOX->PrevPrimary(lin);
  if (lin && lin->GetWPT()->IsTerminated()) return Error2();
  flpBOX->MoveUpItem();
  fplan->Recycle();
  return;
}
//-------------------------------------------------------------------------
//  Mouve down current selected waypoint
//-------------------------------------------------------------------------
void CFuiFlightLog::MoveDwWaypoint()
{ //--- Check that selected waypoint may move ---------------
  if (flpBOX->IsEmpty())                    return;
  CFlpLine *lin = (CFlpLine*)flpBOX->GetPrimary();
  if (lin->GetWPT()->IsTerminated())        return Error1();
  //--- Check that the next is not terminated -----------
  lin = (CFlpLine*)flpBOX->NextPrimToSelected();
  if (lin && lin->GetWPT()->IsTerminated()) return Error2();
  flpBOX->MoveDwItem();
  fplan->Recycle();
  return;
}

//-------------------------------------------------------------------------
//  Close this window
//-------------------------------------------------------------------------
void CFuiFlightLog::CloseMe()
{ fplan->Save();
  globals->dbc->RegisterLOGwindow(0);
  CFuiDirectory *dir = globals->dbc->GetDIRwindow();
  if (dir)   dir->RegisterMe(0);
  Close();
  return;
}
//-------------------------------------------------------------------------
//  Event notification
//-------------------------------------------------------------------------
void  CFuiFlightLog::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ switch (idm)  {
  case 'sysb':
    if (EVENT_CLOSEWINDOW == evn) CloseMe();
    else  SystemHandler(evn);
    return;
  case 'list':
    flpBOX->VScrollHandler((U_INT)itm,evn);
    return;
  //--- Open info on waypoint -----------------
  case 'info':
    OpenDetail();
    return;
  //--- Add a waypoint after the current one --
  case 'addw':
    OpenDirectory();
    return;
  //--- Delete a waypoint ---------------------
  case 'delw':
    DeleteWaypoint();
    return;
  //--- Move waypoint up ----------------------
  case 'mvup':
    MoveUpWaypoint();
    return;
  //--- Move waypoint down -------------------
  case 'mvdn':
    MoveDwWaypoint();
    return;
  //---- Name is modified --------------------
  case 'name':
    fplan->SetFileName(nWIN->GetText());
    fplan->Modify(1);
    return;
  //---- Description is modified ------------
  case 'desc':
    fplan->SetDescription(dWIN->GetText());
    fplan->Modify(1);
    return;
  //---  Reset marks ------------------------
  case 'rset':
    fplan->Reset();
    return;
  //---  Clear the plan ----------------------
  case 'zero':
    fplan->Clear(0);
    FillCurrentPlan();
    return;
}

  return;
}

//=======================END OF FILE ==================================================