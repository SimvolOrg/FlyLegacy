/*
 * WindowFlightPlan.cpp
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
//----------------------------------------------------------------------------------
#include <io.h>
//==================================================================================
//	Global function to add a flightplan
//==================================================================================
int FlightPlanCB(char *fn,void *upm)
{	CFuiListPlan *win = (	CFuiListPlan *)upm;
	return win->AddToList(fn);
}
//==================================================================================
//
//  FlightPlan List:  List of available flight plans
//
//==================================================================================
CFuiListPlan::CFuiListPlan(Tag idn, const char* filename)
:CFuiWindow(idn,filename,0,0,0)
{ char *erm = "Incorrect FlightPlanList.WIN file";
	fpln	= globals->pln->GetFlightPlan();
  frame = (U_INT)(-1);
  //-----------Init List of plans ---------------------
  U_INT typ1 = LIST_HAS_TITLE + LIST_NOHSCROLL;
  allBOX.SetParameters(this,'allp',typ1);
  FillPlans();
  //-----------Init list of charts --------------------
  U_INT typ3 = LIST_NOHSCROLL;
  mapBOX.SetParameters(this,'chrt',typ3);
  FillChartList();
  mapBOX.SortAndDisplay();
  //-----Locate current plan --------------------------
	char *key = fpln->GetFileName();
  allBOX.GoToKey(key);
  //-----List of Charts ------------------------------
}
//--------------------------------------------------------------------------
//  Destroy the nav log
//--------------------------------------------------------------------------
CFuiListPlan::~CFuiListPlan()
{ 
}
//-------------------------------------------------------------------------
//  Edit the title for list of plans
//-------------------------------------------------------------------------
void CFuiListPlan::TitlePlan()
{ CFpnLine *slot = new CFpnLine;
  slot->SetSlotName("Description");
  slot->SetFile("File name.......................");
  allBOX.AddSlot(slot);
  return;
}
//-------------------------------------------------------------------------
//  Fill the list of plans with file name and description
//	NOTE:  We create a temporary flight plan (fpn) just to extract
//				 the description tag
//-------------------------------------------------------------------------
void CFuiListPlan::FillPlans()
{ allBOX.EmptyIt();
  TitlePlan();
	ApplyToFiles("FlightPlan/*.FPL",FlightPlanCB,this);
	allBOX.Display();
	return;
}
//-------------------------------------------------------------------------
//  Add a plan to the list
//-------------------------------------------------------------------------
int CFuiListPlan::AddToList(char *fn)
{ char txt[PATH_MAX];
	CFPlan  fpn(globals->pln,FPL_FOR_LIST);
  if (0 == RemoveExtension(txt,fn))	return 1;
	if (!fpn.AssignPlan(txt))					return 1;
	//--- Add a line to selection box ----------
	CFpnLine *slot = new CFpnLine;
	slot->SetFile(txt);
	slot->SetSlotName(fpn.GetDescription());
	allBOX.AddSlot(slot);
	return 1;
}
//-------------------------------------------------------------------------
//  Select plan from Directory
//-------------------------------------------------------------------------
void CFuiListPlan::SelectPlan()
{ char fn[MAX_PATH];
  CFpnLine *lin = (CFpnLine *)allBOX.GetSelectedSlot();
	RemoveExtension(fn,lin->GetFile());
	if (!fpln->AssignPlan(fn))			return Close();
  //---Open or refresh detail -------------------------
  CFuiFlightLog *win = globals->dbc->GetLOGwindow();
  if (win)  win->Reset();
  //---Open the detail window -------------------------
  else	globals->fui->CreateFuiWindow (FUI_WINDOW_FPLAN_LOG);
	Close();
  return;
}
//-------------------------------------------------------------------------
//  Get All charts
//-------------------------------------------------------------------------
void CFuiListPlan::FillChartList()
{ FillOneList("JPG");
  FillOneList("PNG");
  return;
}
//-------------------------------------------------------------------------
//  Get All charts
//-------------------------------------------------------------------------
void CFuiListPlan::FillOneList(char *ext)
{ char    fn[MAX_PATH];
  sprintf(fn,"CHARTS/*.%s",ext);
  //--- Get all JPG files in the list --------------
  char  *name = (char*)pfindfirst (&globals->pfs,fn);
  while (name)
  { char  *deb  = strrchr(name,'/');
    if (deb)  AddChart(deb+1);
    name = (char*)pfindnext(&globals->pfs);
  }
  //--- Get All PNG files in the list --------------
  return;
}
//-------------------------------------------------------------------------
//  Add a line for this chart
//-------------------------------------------------------------------------
void CFuiListPlan::AddChart(char *map)
{ CMapLine *lin = new CMapLine();
  lin->SetSlotName(map);
  mapBOX.AddSlot(lin);
  return;
}
//-------------------------------------------------------------------------
//  Select a chart
//-------------------------------------------------------------------------
void CFuiListPlan::SelectChart()
{ CMapLine *lin = (CMapLine*)mapBOX.GetSelectedSlot();
  if (0 == lin)     return;
  strncpy(globals->MapName,lin->GetSlotName(),SLOT_NAME_DIM);
  globals->fui->CreateFuiWindow(FUI_WINDOW_CHART,0);
  return;
}
//-------------------------------------------------------------------------
//  Draw the navigation log
//-------------------------------------------------------------------------
void CFuiListPlan::Draw()
{ CFuiWindow::Draw();
  return;
}
//-------------------------------------------------------------------------
//  Close this window
//-------------------------------------------------------------------------
void CFuiListPlan::CloseMe()
{ Close();
  return;
}
//-------------------------------------------------------------------------
//  Event notification
//-------------------------------------------------------------------------
void  CFuiListPlan::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ switch (idm)  {
  case 'sysb':
    if (EVENT_CLOSEWINDOW == evn) CloseMe();
    else  SystemHandler(evn);
    return;
  //---Scroll event on list of plans ---------
  case 'allp':
    allBOX.VScrollHandler((U_INT)itm,evn);
    return;
  //---Scroll event on list of maps ----------
  case 'chrt':
    mapBOX.VScrollHandler((U_INT)itm,evn);
    return;
  //--- Select a new plan from directory -----
  case  'sfpl':
    SelectPlan();
    return;
  //--- Select a chart -----------------------
  case 'smap':
    SelectChart();
    return;
}

  return;
}

//=======================END OF FILE ==================================================