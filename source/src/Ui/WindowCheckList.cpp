/*
 * WindowCheckList.cpp
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
#include "../Include/FuiParts.h"
#include "../Include/Robot.h"
#include "../Include/PlanDeVol.h"
#include <vector>

//=====================================================================================
//  Globals function Uncheck all
//=====================================================================================
void UnMarkCheckList(CSlot *slot)
{ CChkLine *lin = (CChkLine*)slot;
  lin->Unmark();
}

//=====================================================================================
//   WINDOW for AIRCRAFT CHECK LIST
//=====================================================================================
CFuiCkList::CFuiCkList(Tag idn, char *filename)
:CFuiWindow(idn,filename,0,0,0)
{ char err[128];
  sprintf(err,"Incorrect TEMPLATE file: %",filename);
	//---------------------------------------------------------
	mveh				= globals->pln;
  //---------------------------------------------------------
  nSEL        = -1;
  mPOP.aText  = 0;
  panl				= 0;
	slot				= 0;
  //---Locate components ------------------------------------
  uBUT = (CFuiButton *)    GetComponent('unck');     // Uncheck button
  if (0 == uBUT)    gtfo(err);
  wBUT = (CFuiButton *)    GetComponent('wher');     // Locate button
  if (0 == uBUT)    gtfo(err);
  cPOP = (CFuiPopupMenu*)  GetComponent('chap');
  if (0 == cPOP)    gtfo(err);
  //---Request notificationon stop click ---------------------
  wBUT->NotifyOnStop();
  //---Init list box interface -------------------------------
  U_INT type = LIST_DONT_FREE + LIST_NOHSCROLL + LIST_USE_MARKS;
  iBOX.SetParameters(this,'list',type);
	//--- Get related objects to work together -----------------
  LST = (globals->pln)?(&globals->pln->ckl):(0);
	if (LST)
	{ if (LST->HasChapter())  BuildMenu();
		//---Register for closing ----------------------------------
		LST->RegisterWindow(this);
	}
	else Close();
};
//-------------------------------------------------------------------------
//  Release resources
//-------------------------------------------------------------------------
CFuiCkList::~CFuiCkList()
{ if (mPOP.aText) delete [] mPOP.aText;
  LST->RegisterWindow(0);
  if (panl)		panl->ClearFocus();
}
//-------------------------------------------------------------------------
//  Build Menu
//  -Request a list of chapters as the popup
//  -Select the first chapter
//-------------------------------------------------------------------------
void CFuiCkList::BuildMenu()
{ char **tab = LST->GetChapters();
  cPOP->CreatePage(&mPOP,tab);
  NewChapter(0);
  return;
}
//-------------------------------------------------------------------------
//  Select a new chapter
//  -Change Pop text
//  -Change list of items (dont delete line by EmptyIt(). Lines
//   belong to Chapters
//-------------------------------------------------------------------------
void CFuiCkList::NewChapter(int No)
{ cPOP->SetButtonText((char*)mPOP.aText[No]);
  if (nSEL == No) return;
  nSEL  = No;
  //---Empty the list box and request a new list -----
  iBOX.Clear();
  LST->GetLines(iBOX,No);
  iBOX.Display();
  return;
}
//-------------------------------------------------------------------------
//  NOTIFICATIONS FROM POPUP
//-------------------------------------------------------------------------
void CFuiCkList::NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn)
{ switch (id) {
    case 'chap':
      if (evn == EVENT_POP_CLICK)  NewChapter((U_INT)itm);
      return;

    }
  return;
}
//-------------------------------------------------------------------------
//  Check all preconditions and select panel items
//-------------------------------------------------------------------------
bool CFuiCkList::Verify()
{ slot = (CChkLine*)iBOX.GetSelectedSlot();
  if (0 == slot)											return false;
  D2R2_ACTION &a	=  slot->Action();
	U_INT     val		=  slot->GetVLID();
  //---Locate the panel -----------------------
	panl  = globals->pit->GetPanelByTag(a.pnt);
	gage	= (panl)?(panl->GetGauge(a.ggt)):(0);
	a.panl	= panl;
	a.gage	= gage;
  //--- Check for cockpit camera -----------------
  cam = globals->ccm->SelectCamera(CAMERA_COCKPIT);
	bool	ct	= cam->IsOf(CAMERA_COCKPIT);
	U_INT cp	= val & D2R2_NEED_CPIT;
  if (cp && !ct)											return false;
	//--- Check if panel needed --------------------
	U_INT np	= val & D2R2_NEED_PANL;
	if (np && !panl)										return false;
  //--- Activate the panel -----------------------
	mveh->pit.ActivatePanel (a.pnt);
  if (a.actn == D2R2_SHOW_FOCUS_PANEL)return true;
		//--- Check for gauge needed -------------------
	U_INT ng	= val & D2R2_NEED_GAGE;
	if (ng && !gage)										return false;
  panl->FocusOnGauge(gage);
  //--- Check for focus to Click area ----------
  if (a.canb)  panl->FocusOnClick(gage,a.canb);
  return true;
}
//-------------------------------------------------------------------------
//  NOTIFICATIONS FROM POPUP:  Locate panel and gauge and draw tour
//-------------------------------------------------------------------------
void CFuiCkList::LocateGage()
{ if (!Verify())	return;
  panl->FocusOnGauge(tgag);
  panl->FocusOnClick(gage,ca);
  return;
}

//-------------------------------------------------------------------------
//  NOTIFICATIONS FROM POPUP:  
//  Locate panel and gauge and  Execute action
//-------------------------------------------------------------------------
void CFuiCkList::Execute()
{ if (!Verify())				return;
  if (0  == actn)       return;
  LST->Execute(&slot->Action());
  return;
}
//-------------------------------------------------------------------------
//  Mark the slot
//-------------------------------------------------------------------------
void CFuiCkList::EndExecute(void *up)
{ if (0 == up)	return;
	CChkLine *lin  =(CChkLine*)up;
  lin->SetMark();
  iBOX.Refresh();
  return;
}
//-------------------------------------------------------------------------
//  Stop blinking gauge
//-------------------------------------------------------------------------
void CFuiCkList::StopBlink()
{ if (panl)  panl->ClearFocus();
  panl = 0;
  return;
}
//-------------------------------------------------------------------------
//  Default notification
//-------------------------------------------------------------------------
void  CFuiCkList::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ if (idm == 'sysb') SystemHandler(evn);
  switch (idm)  {
    case 'list':
      iBOX.VScrollHandler((U_INT)itm,evn);
      return;
    case 'done':
      { EndExecute(slot);
        return;
      }

    case 'unck':
      iBOX.Apply(UnMarkCheckList);
      return;

    case 'wher':
      if (evn == EVENT_BUTTONPRESSED) LocateGage();
      if (evn == EVENT_BUTTONRELEASE) StopBlink();
      return;

    case 'exec':
      if (evn == EVENT_BUTTONPRESSED) Execute();
      return;
  }
  return;
}

//=======================END OF FILE ==================================================