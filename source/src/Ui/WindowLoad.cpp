/*
 * WindowLoad.cpp
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
#include "../Include/FuiPlane.h"
//==================================================================================
//  CFuiLoad manages the aircraft load stations
//==================================================================================
CFuiLoad::CFuiLoad(Tag idn, const char*filename)
: CFuiWindow(idn,filename,520,300,0)
{ SetTitle("LOAD STATION MANAGER");
  //-----------------------------------------------------------
  wLBL  = new CFuiLabel(6, 4,200,20);
  AddChild('?lbl',wLBL,"LOAD STATIONS:");
  //----Create the List box -----------------------
  wSTA  = new CFuiList (6,30,250,258,this);
  wSTA->SetVScroll();
  AddChild('wsta',wSTA);
  //----Create adjust label -----------------------
  wLBL  = new CFuiLabel(280, 4,200,20);
  AddChild('?lbl',wLBL,"ADJUST WEIGHT");
  //---Create box frame ---------------------------
  wBOX  = new CFuiBox(270,30,240,100);
  wBOX->SetTransparentMode();
  AddChild('wbox',wBOX);
  //---Create station identity --------------------
  wIDN  = new CFuiLabel(280,40,200,20,this);
  AddChild('widn',wIDN,"Pilot");
  //---Create Weigth display -----------------------
  wWGH  = new CFuiLabel(410,40, 80,20,this);
  AddChild('wwgh',wWGH,"");
  //---Create Label weight -------------------------
  wLBL  = new CFuiLabel(280,60,100,20);
  AddChild('?lbl',wLBL,"WEIGHT");
  //---Create Limit Dispaly ------------------------
  wLIM  = new CFuiLabel(420,60,120,20);
  AddChild('wlim',wLIM,"");
  //---Create ruler for adjustment------------------
  wRUL  = new CFuiSlider(280, 80, 220, 30,this); 
  AddChild('wrul',wRUL);
  //---Create box frame (reuse wBOX)----------------
  wBOX  = new CFuiBox(270,140,240,80);
  wBOX->SetTransparentMode();
  AddChild('wbox',wBOX);
  //----  Create Dry weight label ------------------
  wLBL  = new CFuiLabel(280,150,80,20);
  AddChild('?lbl',wLBL,"DRY  WEIGHT");
  //---- Create Fuel weight label ------------------
  wLBL  = new CFuiLabel(280,170,80,20);
  AddChild('?lbl',wLBL,"FUEL WEIGHT");
  //---- Create other label ------------------------
  wLBL  = new CFuiLabel(280,190,80,20);
  AddChild('?lbl',wLBL,"LOAD WEIGHT");
  //------Create Dry WEIGHT DISPLAY-----------------
  wDRY  = new CFuiLabel(400,150,140,20);
  AddChild('wdry',wDRY,"2680 lbs");
  //---- Create Fuel Weight DISPLAY ----------------
  wGAS  = new CFuiLabel(400,170,140,20);
  AddChild('wgas',wGAS," 360 lbs");
  //---- Create Load Weight ------------------------
  wOTH  = new CFuiLabel(400,190,140,20);
  AddChild('woth',wOTH," 405 lbs");
  //---Create box frame (reuse wBOX)----------------
  wBOX  = new CFuiBox(270,230,240,60);
  wBOX->SetTransparentMode();
  AddChild('wbox',wBOX);
  //---- Create Label Total weight ----------------
  wLBL  = new CFuiLabel(280,240,80,20);
  AddChild('?lbl',wLBL,"TOTAL WEIGHT");
  //---  Create Label limit- ---------------------
  wLBL  = new CFuiLabel(280,260,80,20);
  AddChild('?lbl',wLBL,"LIMIT WEIGHT");
  //---- Create Total display --------------------
  wTOT  = new CFuiLabel(400,240,140,20);
  AddChild('wtot',wTOT,"3203 lbs");
  //----Register and create ----------------------
  globals->wld = this;
  CFuiWindow::ReadFinished();
  U_INT type = LIST_NOHSCROLL;
  sBOX.SetParameters(this,'wsta',type);
  GetStations();
  sBOX.Display();
  EditSelection();
}
//---------------------------------------------------------------------------
//  Destroy the window
//---------------------------------------------------------------------------
CFuiLoad::~CFuiLoad()
{ vlod.clear();
  globals->wld = 0;
}

//---------------------------------------------------------------------------
//    Get all stations
//---------------------------------------------------------------------------
void CFuiLoad::GetStations()
{ veh = globals->pln;
  if (0 == veh) return;
  //---------------------------------
  veh->GetLoadCell(vlod);
  std::vector<CLoadCell*>::iterator vl;
  for (vl = vlod.begin(); vl != vlod.end(); vl++)
  { CLoadCell *cel = (*vl);
    CLodLine  *lod = new CLodLine(cel);
    sBOX.AddSlot(lod);
  }
  return;
}
//---------------------------------------------------------------------------
//  Get and edit the selection 
//---------------------------------------------------------------------------
void CFuiLoad::EditSelection()
{ CLodLine *lod = (CLodLine*)sBOX.GetSelectedSlot();
  scel  = lod->GetCell();
  Edit(scel);
  return;
}
//---------------------------------------------------------------------------
//  Edit the cell
//---------------------------------------------------------------------------
void CFuiLoad::Edit(CLoadCell *cel)
{ float load = cel->GetLoad();
  float wlim = cel->GetLimit();
  wIDN->SetText(cel->GetName());
  wWGH->EditText("%5d lbs",int(load));
  wLIM->EditText("Max %5d lbs",int(wlim));
  //--- Set ruler -------------------------------
  wRUL->SetRange(0,wlim);
  wRUL->SetValue(load);
  return;
}
//---------------------------------------------------------------------------
//  Edit sumary
//---------------------------------------------------------------------------
void CFuiLoad::EditSumary()
{ float dw = (veh)?(veh->GetDryWeight()):(0);
  float gw = (veh)?(veh->GetGasWeight()):(0);
  float lw = (veh)?(veh->GetLodWeight()):(0);
  float tw = dw + gw + lw;
  wDRY->EditText("%5d lbs",int(dw));
  wGAS->EditText("%5d lbs",int(gw));
  wOTH->EditText("%5d lbs",int(lw));
  //--- Edit total --------------------
  wTOT->EditText("%5d lbs",int(tw));
  return;
}
//---------------------------------------------------------------------------
//  Change current mass
//---------------------------------------------------------------------------
void CFuiLoad::ChangeMass()
{ float rat = wRUL->GetValue();
  scel->SetLoad(rat);
  sBOX.Refresh();
  return;
}
//---------------------------------------------------------------------------
//  Draw the window
//---------------------------------------------------------------------------
void CFuiLoad::Draw()
{ EditSumary();
  CFuiWindow::Draw();
  return;
}
//------------------------------------------------------------------------
//  Notifications
//------------------------------------------------------------------------
void CFuiLoad::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ switch(idm)  {
    case 'sysb':
      SystemHandler(evn);
      return;
    //---Display list --------------------------------
    case 'wsta':
      sBOX.VScrollHandler((U_INT)itm,evn);
      EditSelection();
      return;
    //---Change mass for selected load ---------------
    case 'wrul':
      ChangeMass();
      return;
  }
  return;
}

//============================END OF FILE ================================================================================
