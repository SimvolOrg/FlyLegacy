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
//	Error Messages 
//==================================================================================
char *errFPWIN[] = {
	"CANNOT INSERT BEFORE TERMINATED WAYPOINT",		  // MS00
	"FILE NAME IS EMPTY!!!",												// MS01
	"CANNOT DELETE A TERMINATED WAYPOINT",					// MS02
	"CANNOT MOVE ABOVE A TERMINATED WAYPOINT",			// MS03
	"CANNOT MOVE A TERMINATED WAYPOINT",						// MS04
	"CANNOT MODIFY ACTIVE FLIGHT PLAN",							// MS05
};
//==================================================================================
//	Mode structure 
//==================================================================================
struct FPL_MODE {
		Tag            tag;									// Component
		char          *idn;									// Identity
		char           vis[2];							// visibility
		char           mod[2];							// Saved modifier
};
//==================================================================================
//	Mode table 
//	NOTE: In protected mode, keyboard entry is prohibited
//==================================================================================
FPL_MODE fplMOD[] = {
	{'mvup',"mvup",									    {0,1}},		// 1 Up button
	{'mvdn',"mvdn",											{0,1}},		// 2 Down button
	{'rset',"rset",											{0,1}},		// 3 Clear all marks
	{'zero',"zero",											{1,1}},		// 4 New plan (RAZ)
	{'name',"name",											{1,1}},		// 5 Plan name
	{'desc',"desc",											{1,1}},		// 6 Description
	{'addw',"addw",											{0,1}},		// 7 Add waypoint
	{'delw',"delw",											{0,1}},		// 8 Delete waypoint
	{'mcel',"mcel",											{0,1}},		// 9 dec ceil
	{'pcel',"pcel",											{0,1}},		// 11 inc cell 
	{'malt',"malt",											{0,1}},		// 12 dec altitude
	{'palt',"palt",											{0,1}},		// 14 inc altitude
	{0},
};
//==================================================================================
char *fplEDT[] = {
	"Edit Plan",
	"Protect",
};
//==================================================================================
//	Mode table for group box
//	NOTE: In protected mode, keyboard entry is prohibited
//==================================================================================
FPL_MODE fplREND[] = {
	{'ptko',"ptko",											{0,1}},		// Take off runway
	{'plnd',"plnd",											{0,1}},		// Landing runway
	{0},
};
//==================================================================================
//
//  NAVIGATION LOG:  List a detailled Flight Plan
//
//==================================================================================
CFuiFlightLog::CFuiFlightLog(Tag idn, const char* filename)
:CFuiWindow(idn,filename,0,0,0)
{ char *erm = "Incorrect FlightPlanLog.WIN file";
	//--- Flight plan parameters ------------------------
  fpln	= globals->pln->GetFlightPlan();
	fpln->Register(this);
  //----- Get components ------------------------------
  eWIN  = (CFuiLabel *)   GetComponent('eror');
  if (0 == eWIN)  gtfo(erm);
  nWIN  = (CFuiTextField*)GetComponent('name');
  if (0 == nWIN)  gtfo(erm);
	nWIN->SetValidMask(KB_ANYTHING);
  dWIN  = (CFuiTextField*)GetComponent('desc');
  if (0 == dWIN)  gtfo(erm);
	dWIN->SetValidMask(KB_ANYTHING);
  grh   = (CFuiCanva*)GetComponent('canv');
  if (0 == grh)		gtfo(erm);
	ilsF	= (CFuiLabel*)GetComponent('ilsf');
  if (0 == ilsF)	gtfo(erm);
	//--- Get altitude components ----------------------
	wALT	= (CFuiTextField*)GetComponent('alti');
	if (0 == wALT)	gtfo(erm);
	wALT->SetValidMask(KB_NUMBER_SIGN);
	//--- Get Ceil component ---------------------------
	wCEL  = (CFuiTextField*)GetComponent('ceil');
	if (0 == wCEL)  gtfo(erm);
	wCEL->SetValidMask(KB_NUMBER_SIGN);
	//--- Init runway end points component -------------
	rend	= (CFuiGroupBox*)GetComponent('rend');
  if (0 == rend)	gtfo(erm);
	tkoID	= "NUL";
	ptko	= (CFuiPopupMenu*)rend->GetComponent('ptko');
	if (0 == ptko)	gtfo(erm);
	lndID	= "NUL";
	plnd	= (CFuiPopupMenu*)rend->GetComponent('plnd');
	if (0 == plnd)	gtfo(erm);
	Mode	= -1;
  //-----------Init the list box-----------------------
  eWIN->SetText("");
	flpBOX		 = fpln->GetFBOX();
  U_INT type = LIST_HAS_TITLE + LIST_NOHSCROLL;
  flpBOX->SetParameters(this,'list',type);
  globals->dbc->RegisterLOGwindow(this);
	//--- Select first node ----------------------------
	FillCurrentPlan();
	//---- Marker 1 -------------------------------------
	char *ds = new char[32];
	strcpy(ds,"*START FlightLog*");

}
//--------------------------------------------------------------------------
//  Destroy the nav log
//--------------------------------------------------------------------------
CFuiFlightLog::~CFuiFlightLog()
{ fpln->Register(0);
  globals->dbc->RegisterLOGwindow(0);
	//---- Marker 2 -------------------------------------
	char *ds = Dupplicate("*END FlightLog*",32);
}
//--------------------------------------------------------------------------
//  Set the Mode
//	NOTE: When flight plan is used, edit is prohibited
//--------------------------------------------------------------------------
void CFuiFlightLog::SetMode()
{	char *erm = "FlightPlanLog.WIN file. Missing tag %s";
	char  m   = fpln->GetEdMode();
	Mode			= m;
	//--- Change mode ----------------------------------
	CFuiComponent *cps = GetComponent('edpl');
	if (0 == cps)	gtfo(erm);
	cps->SetText(fplEDT[m]);
	//--- Initialize all buttons and menus -------------
	FPL_MODE *src = fplMOD;
	while (src->tag)
	{	cps = GetComponent(src->tag);
		if (0 == cps)	gtfo(erm,src->idn);
		if (src->vis[m])	cps->Show();
		else							cps->Hide();
		src++;
	}
	//--- Initialize popups ----------------------------
	CFuiGroupBox *grp = (CFuiGroupBox*)GetComponent('rend');
	if (0 == grp)		gtfo(erm,'rend');
	src = fplREND;
	while (src->tag)
	{	cps = grp->GetComponent(src->tag);
		if (0 == cps)	gtfo(erm,src->idn);
		if (src->vis[m])	cps->RazProperty(FUI_IS_LOCKED);
		else							cps->SetProperty(FUI_IS_LOCKED);
		src++;
	}
	return;
}
//--------------------------------------------------------------------------
//	Swap the edit mode
//--------------------------------------------------------------------------
void CFuiFlightLog::SwapMode()
{	if (fpln->SwapEditMode())	return SetMode();
	return Error(5);	
}
//--------------------------------------------------------------------------
//  Select first node
//--------------------------------------------------------------------------
void CFuiFlightLog::Select()
{	sWPT = (CWPoint*)flpBOX->GetSelectedSlot();
  fpln->SetSelection(sWPT);
	GetRunway();
	//--- Edit altitude ----------------------------
	char *alti = (sWPT)?(sWPT->GetEdAltitude()):("");
	wALT->SetText(alti);
	return;
}
//--------------------------------------------------------------------------
//  Teleport to selected point
//--------------------------------------------------------------------------
void CFuiFlightLog::Teleport()
{	if (globals->aPROF.Has(PROF_ACBUSY))	return;
	if (sWPT)	sWPT->Teleport();
	return;
}
//--------------------------------------------------------------------------
//  Edit ceil 
//--------------------------------------------------------------------------
void CFuiFlightLog::EditCeil(int a)
{	char txt[128];
	_snprintf(txt,6,"%05d",a);
	wCEL->SetText(txt);
	return;
}
//--------------------------------------------------------------------------
//  Fill the current flight plan
//--------------------------------------------------------------------------
void CFuiFlightLog::FillCurrentPlan()
{ nWIN->SetText(fpln->GetFileName());
	dWIN->SetText(fpln->GetDescription());
	EditCeil(fpln->actCEIL());
  flpBOX->Display();
	Select();
	//--- Change mode according to plan ----------
	SetMode();
  return;
}
//-------------------------------------------------------------------------
//  Draw the navigation log
//-------------------------------------------------------------------------
void CFuiFlightLog::Draw()
{ if (Req.EndOfReq())  EndOfRequest(&Req);
	char m = fpln->GetEdMode();
	if (Mode != m)	SetMode();
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
	U_INT No				=	wpt->GetSequence();
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
{ if (fpln->IsUsed())	return Error(5);
	CFuiDirectory *dir = globals->dbc->GetDIRwindow();
  if (NULL == dir)  dir = (CFuiDirectory *)globals->fui->CreateFuiWindow(FUI_WINDOW_WAYPOINT_DIRECTORY);
  if (NULL != dir)  dir->RegisterMe(this);
  return;
}
//-------------------------------------------------------------------------
//  Insert the waypoint in flight plan
//  -Refresh the list
//-------------------------------------------------------------------------
void  CFuiFlightLog::InsertWaypoint(CWPoint *wpt)
{ //--- Add waypoint -------------------
  flpBOX->InsSlot(wpt);
  fpln->Reorder(1);
	flpBOX->Refresh();
	Select();
  return;
}
//-------------------------------------------------------------------------
//  Refresh page
//-------------------------------------------------------------------------
void CFuiFlightLog::Refresh()
{	flpBOX->Refresh();
	Select();
}
//-------------------------------------------------------------------------
//  Create an Airport waypoint
//-------------------------------------------------------------------------
void  CFuiFlightLog::CreateAPTwaypoint()
{ CAirport  *apt = (CAirport*)selOBJ.Pointer();
	CWPoint		*wpt = fpln->CreateAPTwaypoint(apt);
	//--- Add waypoint -------------------
	InsertWaypoint(wpt);
  return;
}
//-------------------------------------------------------------------------
//  Create an NAVAID waypoint
//-------------------------------------------------------------------------
void  CFuiFlightLog::CreateNAVwaypoint()
{ CNavaid   *nav = (CNavaid*)selOBJ.Pointer();
  CWPoint   *wpt = fpln->CreateNAVwaypoint(nav);
  InsertWaypoint(wpt);
  return;
}

//-------------------------------------------------------------------------
//  Create a USER waypoint
//-------------------------------------------------------------------------
void  CFuiFlightLog::CreateWPTwaypoint()
{	CWPT		*pnt = (CWPT*)selOBJ.Pointer();
  CWPoint	*wpt = fpln->CreateWPTwaypoint(pnt);
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
  bool ok = !prm->IsVisited();
  if (ok)           return true;
	//--- delete object ------------------------------------
	selOBJ = 0;
	Error(0);
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
//  Display error message
//-------------------------------------------------------------------------
void CFuiFlightLog::Error(char No)
{	eWIN->RedText(errFPWIN[No]);
	return;
}
//-------------------------------------------------------------------------
//  Delete the selected Waypoint
//-------------------------------------------------------------------------
void CFuiFlightLog::DeleteWaypoint()
{ if (fpln->IsEmpty())  return;
  if (fpln->IsUsed())		return Error(5);
  CWPoint *wpt = (CWPoint*)flpBOX->GetPrimary();
  flpBOX->DeleteItem();
  fpln->Reorder(1);
	Select();
  return;
}
//-------------------------------------------------------------------------
//  Move up current selected waypoint
//-------------------------------------------------------------------------
void CFuiFlightLog::MoveUpWaypoint()
{ //--- Check that selected waypoint may move ---------------
	if (fpln->IsUsed())			return Error(5);
  if (fpln->IsEmpty())    return;
  CWPoint *wpt = (CWPoint*)flpBOX->GetPrimary();
  //--- Check that the previous node exist ------------------
  wpt = (CWPoint*)flpBOX->PrevPrimary(wpt);
  if (0 == wpt)						return;
  flpBOX->MoveUpItem();
  fpln->Reorder(1);
  return;
}
//-------------------------------------------------------------------------
//  Move down current selected waypoint
//-------------------------------------------------------------------------
void CFuiFlightLog::MoveDwWaypoint()
{ //--- Check that selected waypoint may move ---------------
	if (fpln->IsUsed())			return Error(5);
  if (fpln->IsEmpty())    return;
  //--- Check that the next node exist ----------------------
  CWPoint *wpt = (CWPoint*)flpBOX->NextPrimToSelected();
  if (0 == wpt)						return;
  flpBOX->MoveDwItem();
  fpln->Reorder(1);
  return;
}
//-------------------------------------------------------------------------
//  Clear plan
//-------------------------------------------------------------------------
void CFuiFlightLog::ClearPlan()
{	if (fpln->IsUsed())	return Error(5);
  fpln->ClearPlan();
	fpln->WarnGPS();
  FillCurrentPlan();
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
	char *alti = sWPT->ModifyAltitude(inc);
	wALT->SetText(alti);
	flpBOX->LineRefresh();
	return;
}
//----------------------------------------------------------------------
//  Modify Ceil
//----------------------------------------------------------------------
void CFuiFlightLog::ModifCeil(int inc)
{	int a = fpln->ModifyCeil(inc);
	EditCeil(a);
	return;
}
//-------------------------------------------------------------------------
//  Close this window
//-------------------------------------------------------------------------
void CFuiFlightLog::CloseMe()
{ if (Req.IsBusy())	return;
	fpln->Save();
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
	fpln->SetFileName(nm);
	if (0 == *nm) Error(1);
	return;
}
//-------------------------------------------------------------------------
//  Event notification
//-------------------------------------------------------------------------
void  CFuiFlightLog::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ eWIN->SetText("");
	switch (idm)  {
	//--- System events ----------------------------
  case 'sysb':
    if (EVENT_CLOSEWINDOW == evn) CloseMe();
    else  SystemHandler(evn);
    return;
	//--- Change Mode ------------------------------
	case 'edpl':
		SwapMode();
		return;
	//--- List events ------------------------------
  case 'list':
    flpBOX->VScrollHandler((U_INT)itm,evn);
		Select();
    return;
  //--- Open info on waypoint --------------------
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
	//--- Teleport to waypoint -----------------
	case 'goto':
		Teleport();
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
		ClearPlan();
    return;
	//--- Increment altitude -------------------
	case 'palt':
		return ModifAlti(+100);
	//--- Decrement altitude -------------------
	case 'malt':
		return ModifAlti(-100);
  //--- Increment ceil -----------------------
	case 'mcel':
		return ModifCeil(-100);
	//--- Decrement ceil -----------------------
	case 'pcel':
		return ModifCeil(+100);

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
			sWPT->SetIlsFrequency(ilsFQ);
  		ilsF->SetText(ilsTXT);
			return;
	}
	return;
}
//=======================END OF FILE ==================================================