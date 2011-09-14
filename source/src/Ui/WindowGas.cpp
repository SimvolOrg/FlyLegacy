/*
 * WindowGaz.cpp
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
#include "../Include/Globals.h"
#include <vector>
//==================================================================================
#define WDIR_SEL_APT 0
#define WDIR_SEL_VOR 1
#define WDIR_SEL_NDB 2
#define WDIR_SEL_WPT 3
//------------------------------------------------------------------------
//  MENU FOR FUEL MANAGEMENT
//------------------------------------------------------------------------
char *GasMENU[] = {
    " Unknown",
    " 80",
    " 100",
    " 100LL",
    " 115",
    " JET-A",
    " JET-A1",
    " JET-A1 PLUS",
    " JET-B",
    " JET-B PLUS",
    " AUTOMOTIVE",
    "",
};
//=======================================================================================
//  CFuiFuel window
//=======================================================================================
CFuiFuel::CFuiFuel(Tag idn, const char* filename)
:CFuiWindow(idn,filename,0,0,0)
{ char *erm = "incorrect VehicleFuel.WIN file";
  rul = (CFuiSlider*)GetComponent('tots');
  if (0 == rul) gtfo(erm);
  All = (CFuiLabel *)GetComponent('totl');
  if (0 == All) gtfo(erm);
  Sel = (CFuiLabel *)GetComponent('selt');
  if (0 == Sel) gtfo(erm);
  pop = (CFuiPopupMenu*)GetComponent('stuf');
  if (0 == pop) gtfo(erm);
  Tnk = (CFuiList*)  GetComponent('tank');
  if (0 == Tnk) gtfo(erm);
  Tot.SetName("ALL TANKS");
  sCel  = &Tot;
  cBOX  = 0;
  //---------------------------------------------------
  CVehicleObject *veh  = globals->pln;
  gas = (veh)?(veh->gas):(0);
  //---Init title --------------------------------------
  if (veh)   veh->gas->GetAllCells(fcel);
  U_INT type = LIST_HAS_TITLE + LIST_NOHSCROLL;
  gBOX.SetParameters(this,'tank',type);
  //----------------------------------------------------
  BuildTankList();
  InitGradMenu();
  GetTotalGas();
  SelectTank();
  //------Register me-----------------------------------
  globals->wfl  = this;
}
//------------------------------------------------------------------------
//  Destroy this window
//------------------------------------------------------------------------
CFuiFuel::~CFuiFuel()
{ globals->wfl = 0;
}

//-----------------------------------------------------------------------
//  Compute global capacity, quantity and weight
//  TODO: weight according to fuel grad and temperature ?
//-----------------------------------------------------------------------
void CFuiFuel::GetTotalGas()
{ std::vector<CFuelCell*>::iterator sf;
  float cap = 0;
  float qty = 0;
  float wgh = 0;
  CFuelCell *cel;
  for (sf = fcel.begin(); sf != fcel.end(); sf++)
  { if ((*sf)->NotTank()) continue;
    cel   = (CFuelCell*)(*sf);
    cap  += cel->GetCellCap();
    qty  += cel->GetCellQty();
  }
  Tot.SetCellCap(cap);
  Tot.SetCellQty(qty);
  Tot.SetCellWgh(qty * FUEL_LBS_PER_GAL);             // in lbs for 80 100 and 100LL
  return;
}
//------------------------------------------------------------------------
//  Init Popup fuel grad
//------------------------------------------------------------------------
void CFuiFuel::InitGradMenu()
{ grNo  = (gas)?(gas->GetGradIndex()):(0);
  pop->CreatePage(&fPop,GasMENU,grNo);
}
//------------------------------------------------------------------------
//  Build Tank list
//------------------------------------------------------------------------
void CFuiFuel::BuildTankList()
{ //---Add the title ------------------------------
  ttl.FixeIt();
  gBOX.AddSlot(&ttl);
  //---Add the total cell -------------------------
  all.SetCell(&Tot);
  all.FixeIt();
  gBOX.AddSlot(&all);
  //---Add aircraft tanks -------------------------
  std::vector<CFuelCell*>::iterator it;
  for (it = fcel.begin(); it != fcel.end(); it++)
  { CFuelCell *cel = (*it);
    CGasLine  *lin = new CGasLine(cel);
    gBOX.AddSlot(lin);
  }
  gBOX.Display();
  return;
}
//----------------------------------------------------------------------------------
//  Edit the content of cell
//----------------------------------------------------------------------------------
float CFuiFuel::EditFuelLine(CFuelCell *cel,CFuiLabel *lb)
{ float cap = cel->GetCellCap();
  float qty = cel->GetCellQty();
  float rat = cel->GetCellPCT();
  int   wgh = cel->GetCellWgh();
  lb->EditText("%s : %u%% %.02f gals (%04u lbs)",(char*)cel->GetName(),int(rat),qty,wgh);
  return rat;
}
//------------------------------------------------------------------------
//  Compute the fuel quantity against the fuel capacity
//------------------------------------------------------------------------
void CFuiFuel::EditGlobalFuel()
{ if (0 == gas)         return;
  GetTotalGas();
  EditFuelLine(&Tot,All);
  return;
}
//----------------------------------------------------------------------------------
//  Select tank to fill
//----------------------------------------------------------------------------------
void CFuiFuel::SelectTank()
{ CGasLine *lin = (CGasLine*)gBOX.GetSelectedSlot();
  sCel = lin->GetCell();
  return;
}
//----------------------------------------------------------------------------------
//  Change quantity in all aircraft cells by the request rate
//----------------------------------------------------------------------------------
void CFuiFuel::ChangeAllQTY(float rate)
{ std::vector<CFuelCell*>::iterator ic;
  for (ic = fcel.begin(); ic != fcel.end(); ic++)
  { CFuelCell *cel = (*ic);
    float cap = cel->GetCellCap();
    cel->SetCellQty(cap * rate);
  }
  return;
}
//----------------------------------------------------------------------------------
//  Change quantity in the selected cell
//----------------------------------------------------------------------------------
void CFuiFuel::ChangeTankQTY(U_INT inx) 
{ CFuelCell *cel  = sCel;
  float       old = cel->GetCellQty();          // Actual QTY
  float       tot = Tot.GetCellQty();
  float       rat = rul->GetValue() * 0.01f;
  float       cap = cel->GetCellCap();
  float       qty = cap * rat;
  cel->SetCellQty(qty);
  EditFuelLine(cel,Sel);
  if (sCel != &Tot)
  //---Update total QTY ---------------------
  { tot -= old;
    tot += qty; 
    Tot.SetCellQty(tot);
  }
  else  ChangeAllQTY(rat);
  gBOX.Refresh();
  return;
}
//--------------------------------------------------------------------------
//  Refresh from fuel system
//--------------------------------------------------------------------------
void CFuiFuel::Refresh()
{ gBOX.Refresh();
  EditGlobalFuel();
  //--- Refresh ruler for selected tank -----
  float rat = sCel->GetCellPCT();
  rul->SetValue(rat);
  EditFuelLine(sCel,Sel);
  return;
}
//----------------------------------------------------------------------------------
//  Set Popup title 
//----------------------------------------------------------------------------------
void CFuiFuel::SetPopTitle(U_SHORT No)
{ grNo  = No;
  pop->SetButtonText((char*)fPop.aText[No]);
  return;
}
//--------------------------------------------------------------------------
//  Notification from popup
//--------------------------------------------------------------------------
void CFuiFuel::NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn)
{ if (evn == EVENT_SELECTITEM) SetPopTitle((U_INT)itm);
  return;
}
//------------------------------------------------------------------------
//  Notifications
//------------------------------------------------------------------------
void CFuiFuel::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ 
  switch(idm)  {
    case 'sysb':
      SystemHandler(evn);
      return;
    case 'tank':
      gBOX.VScrollHandler((U_INT)itm,evn);
      SelectTank();
      return;
    case 'shcg':
      return;
    case 'tots':
      ChangeTankQTY(0);
      return;
  }
  //----check for detail slider ----------------------------------------
  return;
}
//============================END OF FILE ================================================================================
