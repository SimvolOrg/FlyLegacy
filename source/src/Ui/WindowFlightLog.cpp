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
#include "../Include/PlanDeVol.h"
//==================================================================================
//
//  NAVIGATION LOG:  List a detailled Flight Plan
//
//==================================================================================
CFuiFlightLog::CFuiFlightLog(Tag idn, const char* filename)
:CFuiWindow(idn,filename,0,0,0)
{ char *erm = "Incorrect FlightPlanLog.WIN file";
  fpln	= globals->pln->GetFlightPlan();
	fpln->Register(this);
  //----- Get components ------------------------------
  eWIN  = (CFuiLabel *)   GetComponent('eror');
  if (0 == eWIN)  gtfo(erm);
  nWIN  = (CFuiTextField*)GetComponent('name');
  if (0 == nWIN)  gtfo(erm);
  dWIN  = (CFuiTextField*)GetComponent('desc');
  if (0 == dWIN)  gtfo(erm);
  grh   = (CFuiCanva*)GetComponent('canv');
  if (0 == grh)		gtfo(erm);
	ilsF	= (CFuiLabel*)GetComponent('ilsf');
  if (0 == ilsF)	gtfo(erm);
	//--- Get altitude components ----------------------
	wALT	= (CFuiTextField*)GetComponent('alti');
	if (0 == wALT)	gtfo(erm);
	//--- Init runway end points component -------------
	rend	= (CFuiGroupBox*)GetComponent('rend');
  if (0 == rend)	gtfo(erm);
	tkoID	= "NUL";
	ptko	= (CFuiPopupMenu*)rend->GetComponent('ptko');
	if (0 == ptko)	gtfo(erm);
	lndID	= "NUL";
	plnd	= (CFuiPopupMenu*)rend->GetComponent('plnd');
	if (0 == plnd)	gtfo(erm);
  //-----------Init the list box-----------------------
  eWIN->SetText("");
	flpBOX		 = fpln->GetFBOX();
  U_INT type = LIST_HAS_TITLE + LIST_NOHSCROLL;
  flpBOX->SetParameters(this,'list',type);
  globals->dbc->RegisterLOGwindow(this);
	//--- Select first node ----------------------------
	FillCurrentPlan();

}
//--------------------------------------------------------------------------
//  Destroy the nav log
//--------------------------------------------------------------------------
CFuiFlightLog::~CFuiFlightLog()
{ fpln->Register(0);
}
//--------------------------------------------------------------------------
//  Select first node
//--------------------------------------------------------------------------
void CFuiFlightLog::Select()
{	sWPT = (CWPoint*)flpBOX->GetSelectedSlot();
	GetRunway();
	//--- Edit altitude ----------------------------
	char *alti = (sWPT)?(sWPT->GetEdAltitude()):("");
	wALT->SetText(alti);
	return;
}
//--------------------------------------------------------------------------
//  Fill the current flight plan
//--------------------------------------------------------------------------
void CFuiFlightLog::FillCurrentPlan()
{ nWIN->SetText(fpln->GetFileName());
	dWIN->SetText(fpln->GetDescription());
  flpBOX->Display();
	Select();
	fpln->Reload(0);
  return;
}
//-------------------------------------------------------------------------
//  Draw the navigation log
//-------------------------------------------------------------------------
void CFuiFlightLog::Draw()
{ if (Req.EndOfReq())  EndOfRequest(&Req);
	CFuiWindow::Draw();
  return;
}
//-------------------------------------------------------------------------
//  Open detail window on item selected
//-------------------------------------------------------------------------
bool CFuiFlightLog::OpenDetail()
{ CWPoint  *wpt	= (CWPoint*)flpBOX->GetPrimary();
  if (0 == wpt)               return false;
  CmHead *obj			= wpt->GetDBobject();
  if (0 == obj)               return false;
	U_INT No				=	wpt->GetWaypointNo();
	switch (obj->GetActiveQ())
	{ case VOR:
      return CreateVORwindow(obj,No,1);
    case NDB:
      return CreateNDBwindow(obj,No,1);
    case APT:
      return CreateAPTwindow(obj,No,1);
  }
	return false;
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
//  -Refresh the list
//-------------------------------------------------------------------------
void  CFuiFlightLog::InsertWaypoint(CWPoint *wp)
{ wp->Edit();
  flpBOX->InsSlot(wp);
  fpln->Reorder(1);
	flpBOX->Refresh();
	Select();
  return;
}
//-------------------------------------------------------------------------
//  Create an Airport waypoint
//-------------------------------------------------------------------------
void  CFuiFlightLog::CreateAPTwaypoint()
{ int alt        = fpln->actCEIL();
	CAirport  *apt = (CAirport*)selOBJ.Pointer();
  CWPoint   *wpt = new CWPoint(fpln,'airp');
  wpt->SetUser('airp');
  wpt->SetName    (apt->GetName());
  wpt->SetPosition(apt->GetPosition());
  wpt->SetDbKey   (apt->GetKey());
  wpt->SetAltitude(alt);
  wpt->SetDBwpt(apt);
  InsertWaypoint(wpt);
  return;
}
//-------------------------------------------------------------------------
//  Create an NAVAID waypoint
//-------------------------------------------------------------------------
void  CFuiFlightLog::CreateNAVwaypoint()
{ int alt        = fpln->actCEIL();
	CNavaid   *nav = (CNavaid*)selOBJ.Pointer();
  CWPoint   *wpt = new CWPoint(fpln,'snav');
  wpt->SetUser('snav');
  wpt->SetName    (nav->GetName());
  wpt->SetPosition(nav->GetPosition());
  wpt->SetDbKey   (nav->GetDbKey());
  wpt->SetAltitude(alt);
  wpt->SetDBwpt(nav);
  InsertWaypoint(wpt);
  return;
}

//-------------------------------------------------------------------------
//  Create a USER waypoint
//-------------------------------------------------------------------------
void  CFuiFlightLog::CreateWPTwaypoint()
{ int alt      = fpln->actCEIL();
	CWPT		*pnt = (CWPT*)selOBJ.Pointer();
  CWPoint	*wpt = new CWPoint(fpln,'wayp');
	wpt->SetUser('uswp');
  wpt->SetName    (pnt->GetName());
  wpt->SetPosition(pnt->GetPosition());
  wpt->SetDbKey   (pnt->GetDbKey());
  wpt->SetAltitude(alt);
  wpt->SetDBwpt(pnt);
	pnt->SetNOD(wpt);
  InsertWaypoint(wpt);
  return;
}
//-------------------------------------------------------------------------
//  Check for a valid insert
//  We cannot insert if the next waypoint is terminated
//-------------------------------------------------------------------------
bool CFuiFlightLog::ValidInsert()
{ eWIN->SetText("");
	CWPoint *prm  = (CWPoint*)flpBOX->NextPrimToSelected();
  if (0 == prm)     return true;    // Empty => valid insert
  bool ok = !prm->IsTerminated();
  if (ok)           return true;
	//--- delete object ------------------------------------
	selOBJ = 0;
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
	if (!ValidInsert())   return false;
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
		case WPT:
			CreateWPTwaypoint();
			return true;
  }
  return true;
}
//-------------------------------------------------------------------------
//  Send error 4
//-------------------------------------------------------------------------
void CFuiFlightLog::Error4()
{ char *msg = "FILE NAME IS EMPTY!!!";
  eWIN->RedText(msg);
  return;
}
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
{ if (fpln->IsEmpty())              return;
	eWIN->SetText("");
  CWPoint *wpt = (CWPoint*)flpBOX->GetPrimary();
  if (wpt->IsTerminated())  return Error3();
  flpBOX->DeleteItem();
  fpln->Reorder(1);
	sWPT	= 0;
  return;
}
//-------------------------------------------------------------------------
//  Move up current selected waypoint
//-------------------------------------------------------------------------
void CFuiFlightLog::MoveUpWaypoint()
{ //--- Check that selected waypoint may move ---------------
	eWIN->SetText("");
  if (fpln->IsEmpty())          return;
  CWPoint *wpt = (CWPoint*)flpBOX->GetPrimary();
  if (wpt->IsTerminated())        return Error1();
  //--- Check that the previous is not terminated -----------
  wpt = (CWPoint*)flpBOX->PrevPrimary(wpt);
  if (wpt && wpt->IsTerminated()) return Error2();
  flpBOX->MoveUpItem();
  fpln->Reorder(1);
  return;
}
//-------------------------------------------------------------------------
//  Mouve down current selected waypoint
//-------------------------------------------------------------------------
void CFuiFlightLog::MoveDwWaypoint()
{ //--- Check that selected waypoint may move ---------------
	eWIN->SetText("");
  if (fpln->IsEmpty())          return;
  CWPoint *wpt = (CWPoint*)flpBOX->GetPrimary();
  if (wpt->IsTerminated())        return Error1();
  //--- Check that the next is not terminated -----------
  wpt = (CWPoint*)flpBOX->NextPrimToSelected();
  if (wpt && wpt->IsTerminated()) return Error2();
  flpBOX->MoveDwItem();
  fpln->Reorder(1);
  return;
}
//----------------------------------------------------------------------
//      Post a request to get Runways for selected airport
//----------------------------------------------------------------------
void CFuiFlightLog::GetRunway()
{ CWPoint *wpt = sWPT;
	grh->EraseCanvas();
	rend->Hide();
	rwyBOX.EmptyIt();
	if (0 == wpt)						return;
	if (wpt->NotAirport())	return;
	tkoID = wpt->GetTkoRwy();
  lndID = wpt->GetLndRwy();
	rend->Show();
	CFuiRwyEXT::Init();
	Req.SetWindow(this);
  Req.SetAPT(wpt->GetDbKey());
	rwyBOX.SetParameters(0,0,0);
  Req.SetReqCode(RWY_BY_AIRPORT);
  PostRequest(&Req);
  return;
}
//----------------------------------------------------------------------
//      Queue a line descriptor for one runway
//----------------------------------------------------------------------
void	CFuiFlightLog::AddDBrecord(void *rec,DBCODE cd)
{	switch (cd)	{
		//--- Record is a runway ------------------------
		case RWY_BY_AIRPORT:
			{	CRwyLine *rwy = (CRwyLine*)rec;
				line					= rwy;
				rwyBOX.AddSlot(rwy);
				StoreExtremities(rwy->GetDXH(),rwy->GetDYH());
				StoreExtremities(rwy->GetDXL(),rwy->GetDYL());
				return;
			}
		//--- Record is an ILS for previous runway ------
		case COM_BY_AIRPORT:
			{	CComLine *com = (CComLine *)rec;
				if (line)	line->SetILS(com);
				delete com;
				return;
			}
	}
	return;
}
//----------------------------------------------------------------------
//  Result return from database
//----------------------------------------------------------------------
void CFuiFlightLog::EndOfRequest(CDataBaseREQ *req)
{ req->SetReqCode(NO_REQUEST);
	if (0 == sWPT)		return;							//was deleted
	ComputeScale();												// Compute drawing scale
	ScaleAllRWY();												// Scale drawing coordinates
	InitRunwayEnds();
	DrawRunways();
	ilsF->SetText(ilsTXT);
  return;
}
//----------------------------------------------------------------------
//  Modify altitude
//----------------------------------------------------------------------
void CFuiFlightLog::ModifAlti(int inc)
{	if (0 == sWPT)	return;
	CWPoint *prv = (CWPoint*)flpBOX->PrevPrimary(sWPT);
	char *alti = sWPT->ModifyAltitude(inc);
	wALT->SetText(alti);
	flpBOX->LineRefresh();
}
//-------------------------------------------------------------------------
//  Close this window
//-------------------------------------------------------------------------
void CFuiFlightLog::CloseMe()
{ fpln->Save();
  globals->dbc->RegisterLOGwindow(0);
  CFuiDirectory *dir = globals->dbc->GetDIRwindow();
  if (dir)   dir->RegisterMe(0);
  Close();
  return;
}
//-------------------------------------------------------------------------
//  Modify file name
//-------------------------------------------------------------------------
void CFuiFlightLog::ChangeFileName()
{	char *nm = nWIN->GetText();
  eWIN->SetText("");
	fpln->SetFileName(nm);
	if (0 == *nm) Error4();
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
		Select();
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
		ChangeFileName();
    return;
  //---- Description is modified ------------
  case 'desc':
    fpln->SetDescription(dWIN->GetText());
    return;
  //---  Reset marks ------------------------
  case 'rset':
    fpln->Reorder(1);
    return;
  //---  Clear the plan ----------------------
  case 'zero':
    fpln->Clear(0);
    FillCurrentPlan();
    return;
	//--- Increment altitude -------------------
	case 'palt':
		ModifAlti(+100);
		return;
	//--- Decrement altitude -------------------
	case 'malt':
		ModifAlti(-100);
		return;
}

  return;
}
//----------------------------------------------------------------------------------
//  Notification from Popup
//----------------------------------------------------------------------------------
void CFuiFlightLog::NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn)
{ U_INT   No = (U_INT)itm;
  if (evn != EVENT_POP_CLICK) return;
  switch (id) {
		//--- Change Take off runway ----------
		case 'ptko':
			ptko->Select(No);
			tkoID	= cMENU[No];
			DrawRunways();
			sWPT->SetTkoRwy(tkoID);
			return;
		//--- Change landing runway --------
		case 'plnd':
			plnd->Select(No);
			lndID	= cMENU[No];
			DrawRunways();
			sWPT->SetLndRwy(lndID);
  		ilsF->SetText(ilsTXT);
			return;
	}
	return;
}
//=======================END OF FILE ==================================================