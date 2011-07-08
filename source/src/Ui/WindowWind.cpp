/*
 * WindowWind.cpp
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
#include "../Include/Weather.h"
//=====================================================================================
extern CListBox windBOX;
extern CmvalMap windMAP;
//=====================================================================================
//  Cloud menu
//=====================================================================================
char *coverMENU[] = {
  "Sky Clear",                          // 0 => no cloud
  "Stratus",                            // 1 = stratus
  "Few clouds",                         // 2 = few 1/8
  "Scattered",                          // 3 = scattered 3/8
  "Half cover",                         // 4 
  "Broken",                             // 5
  "Overcast",                           // 6
};
//=====================================================================================
//  Window for WIND
//=====================================================================================
CFuiWind::CFuiWind(Tag idn, const char *filename)
:CFuiWindow(idn,filename,0,0,0)
{ char erm[128];
  sprintf(erm,"Incorrect TEMPLATE file: %",filename);
  //---Locate components ------------------------------------
  layW = (CFuiList*)  GetComponent('layr');
  if (0 == layW) gtfo(erm);
  errW = (CFuiLabel*) GetComponent('eror');
  if (0 == layW) gtfo(erm);
  popW = (CFuiPopupMenu*) GetComponent('popm');
  if (0 == popW) gtfo(erm);
  skyW = (CFuiTextField*) GetComponent('ceil');
  if (0 == skyW) gtfo(erm);
  //-- Init menu -------------------------------------------
  popW->CreatePage(&cMEN,coverMENU);
  layer = globals->wtm->GetCloudLayer();
  popW->SetButtonText((char*)cMEN.aText[layer]);
  //-- Init cloud ceil -------------------------------------
  ceil = globals->wtm->GetCloudCeil();
  ChangeCeil(0);
  //-- Init list box ---------------------------------------
  U_INT type = LIST_HAS_TITLE + LIST_NOHSCROLL;
  windBOX.SetParameters(this,'layr',type);
  windBOX.Display();
  Select();
}
//-------------------------------------------------------------------------
//  Write error message
//-------------------------------------------------------------------------
void CFuiWind::Error(char *msg)
{ errW->RedText(msg);
  return;
}
//-------------------------------------------------------------------------
//  Change Selection
//-------------------------------------------------------------------------
void CFuiWind::Select()
{ int No  = windBOX.GetRealSelectedNo();
  sel     = (CWndLine*) windBOX.GetSelectedSlot();
  val     = sel->GetSlot();
  altd    = 0;
  altu    = 50000;
  windMAP.GetPrevX(No,altd);
  windMAP.GetNextX(No,altu);
  return;
}
//-------------------------------------------------------------------------
//  Modify altitude
//  TODO: Error message
//-------------------------------------------------------------------------
void  CFuiWind::ModifyAltitude(float m)
{ float alt = val->GetX() + m;
  if (alt <= altd)   return Error("CANNOT OVERLAY LOWER LAYER");
  if (alt >= altu)   return Error("CANNOT OVERLAY UPPER LAYER");
  //----Must change value and refresh ---------------
  int No = sel->GetSlotNo();
  windMAP.ChangeX(No,alt);
  windBOX.LineRefresh();
  errW->SetText("");
  return;
}
//-------------------------------------------------------------------------
//  Modify Direction
//  Error message
//-------------------------------------------------------------------------
void  CFuiWind::ModifyDirection(float m)
{  float dir = val->GetU() + m;
   dir       = Wrap360(dir);
   int No    = sel->GetSlotNo();
   windMAP.ChangeU(No,dir);
   windBOX.LineRefresh();
   errW->SetText("");
   return;
}
//-------------------------------------------------------------------------
//  Modify speed
//-------------------------------------------------------------------------
void  CFuiWind::ModifySpeed(float m)
{  float spd = val->GetV() + m;
   if (spd <  0)    return;
   if (spd > 80)    return;
   int No    = sel->GetSlotNo();
   windMAP.ChangeV(No,spd);
   windBOX.LineRefresh();
   errW->SetText("");
   return;
}
//-------------------------------------------------------------------------
//  Change cloud cover
//-------------------------------------------------------------------------
void CFuiWind::ChangeCover(char nc)
{ layer = nc;
  popW->SetButtonText((char*)cMEN.aText[nc]);
  globals->wtm->ChangeCloud(nc);
  return;
}
//-------------------------------------------------------------------------
//  Change cloud ceil
//-------------------------------------------------------------------------
void CFuiWind::ChangeCeil(float m)
{ if (ceil <   200)   return;
  if (ceil > 20000)   return;
  ceil += m;
  skyW->EditText(" %0.f feet",ceil);
  globals->wtm->SetCeil(ceil);
  return;
}
//--------------------------------------------------------------------------
//  Draw intercept
//--------------------------------------------------------------------------
void CFuiWind::Draw()
{ layer = globals->wtm->GetCloudLayer();
  popW->SetButtonText((char*)cMEN.aText[layer]);
  CFuiWindow::Draw();
  return;
}
//--------------------------------------------------------------------------
//  Notification from Popup
//--------------------------------------------------------------------------
void CFuiWind::NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn)
{ switch (id) {
    case 'popm':
      if (evn == EVENT_POP_CLICK)  ChangeCover(char(itm));
      return;
}
return;
}
//-------------------------------------------------------------------------
//  EVENT notifications from child windows
//-------------------------------------------------------------------------
void  CFuiWind::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ if (idm == 'sysb')  {SystemHandler(evn); return;}
  switch (idm)  {
      case 'walt':
        return;
      case 'layr':
        windBOX.VScrollHandler((U_INT)itm,evn);
        Select();
        return;
      //--- Change altitude ------------------
      case 'altp':
        ModifyAltitude(+100);
        return;
      case 'altm':
        ModifyAltitude(-100);
        return;
      //--- Change direction -(by 10)--------
      case 'dirp':
        ModifyDirection(+10);
        return;
      case 'dirm':
        ModifyDirection(-10);
        return;
      //--- Change direction -(by 1) --------
      case 'diru':
        ModifyDirection(+1);
        return;
      case 'dird':
        ModifyDirection(-1);
        return;
      //--- Change Speed -(by 1) --------
      case 'spdp':
        ModifySpeed(+1);
        return;
      case 'spdm':
        ModifySpeed(-1);
        return;
      //--- Change cloud ceil -(by 100) --------
      case 'skyp':
        ChangeCeil(+100);
        return;
      case 'skym':
        ChangeCeil(-100);
        return;


  }
  return ;
}
//=======================END OF FILE ==================================================