/*
 * WindowAircraft.cpp
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
#include "../Plugin/Plugin.h"
#include <vector>
//==================================================================================
#define WDIR_SEL_APT 0
#define WDIR_SEL_VOR 1
#define WDIR_SEL_NDB 2
#define WDIR_SEL_WPT 3
//----------------------------------------------------------------------------------
//  MENU FOR AIRCRAFT SELECTION
//----------------------------------------------------------------------------------
char *AirMENU[] = {
  "ALL TYPES",
  "SINGLE ENGINE",
  "TWIN ENGINES",
  "TURBOPROP",
  "JET",
  "JUMBO",
  "",
};
//=========================================================================================
//  Plane identification to browse all NFO files
//=========================================================================================
CPlaneIdent::CPlaneIdent()
{ item = 0;
}
//-------------------------------------------------------------------------------
//  Decode this file
//-------------------------------------------------------------------------------
bool  CPlaneIdent::ScanInfoFile(const char *filname)
{ SStream s;
  make[0] = 0;
  icon[0] = 0;
  item    = 0;
  if (OpenRStream((char *)filname,s) == 0)    return false;
  ReadFrom (this, &s);
  CloseStream (&s);
  return true;
}
//------------------------------------------------------------------------
//  Read parameters
//------------------------------------------------------------------------
int CPlaneIdent::Read (SStream *stream, Tag tag)
{ switch (tag) {
  int nb;
  case 'make':
    // Make/model of vehicle
    ReadString (make, sizeof(make) - 1, stream);
    item++;
    return TAG_READ;
  case 'icon':
    // Vehicle selection menu icon filename
    ReadString (icon, sizeof(icon) - 1, stream);
    item++;
    return TAG_READ;
  case 'CLAS':
    // Classification
    ReadInt (&nb, stream);
    vclas = (U_SHORT)nb;
    item++;
    return TAG_READ;
}
  return TAG_READ;
}
//=========================================================================================
//  AIRCRAFT SELECTION WINDOW
//  Display a list of available aircraftt with icon and make according to the popup filter
//  NOTE:  Due to the flag ok in the VehicleSelect.win, the button id is renamed okok
//=========================================================================================
CWinPlane::CWinPlane(Tag idn, const char *filename)
:CFuiWindow(idn,filename,0,0,0)
{ fSlot.FixeIt();
  airBOX.SetParameters(this,'crft',0,64);
  selPOP = (CFuiPopupMenu*)GetComponent('type');
  if (0 == selPOP)  gtfo("Wrong file VehicleSelect.win");
  selBTN = (CFuiButton*)   GetComponent('okok');
  if (0 == selBTN)  gtfo("Wrong file VehicleSelect.win");
  //---Create associated mask (change size when adding new type) ---------------
  mask[0]       = 0x00FF;
  mask[1]       = 0x01;
  mask[2]       = 0x02;
  mask[3]       = 0x04;
  mask[4]       = 0x08;
  mask[5]       = 0x10;
  EditSelPopup();
}
//----------------------------------------------------------------------------------
//  Destroy the window
//----------------------------------------------------------------------------------
CWinPlane::~CWinPlane()
{ airBOX.EmptyIt();
}

//-------------------------------------------------------------------------------
//  Decode one file.  
//  Skip if 
//  1-This is the current plane
//  2-The classification does not match
//-------------------------------------------------------------------------------
void CWinPlane::MakeSlot(char *cpln, bool alloc,char *nfo)
{ if (strcmp(cpln,airInfo.GetMake()) == 0)      return;
  if (airInfo.HasNotClass(mask[selOpt]))        return;
  if (airInfo.NotComplete())                    return;
  CAirLine *slot = (alloc)?(new CAirLine):(&fSlot);
  slot->SetBitmap(0);
  slot->SetSlotName(airInfo.GetMake());
  char  *icon = airInfo.GetIcon();
  if (0 == icon)          return;
  SBitmap *bmp = new SBitmap;
  strncpy (bmp->bitmapName, icon,63);
  Load_Bitmap (bmp);
  slot->SetBitmap(bmp);
  //----Trim nfo file name ----------------------
  char *deb = strrchr(nfo,'/');
  deb = (deb)?(deb+1):(nfo);
  slot->SetNFO(deb);
  airBOX.AddSlot(slot);
  return;
}
//-------------------------------------------------------------------------
//  Decode all NFO files to select aircraft parameters
//  The first entry is a fixed slot with current aircraft if 
//  it matches the filter
//-------------------------------------------------------------------------
void  CWinPlane::ScanAllNFO()
{ CVehicleObject *veh = globals->pln;
  cPlane  = "";
  if (veh)
  { CVehicleInfo   *inf = veh->nfo;
    cPlane  = inf->GetVehMake();
    airInfo.SetClass(inf->GetVehClass());
    airInfo.SetMake (inf->GetVehMake());
    airInfo.SetIcon (inf->GetVehIcon());
    airInfo.SetComplete();
    MakeSlot("",false,veh->GetNFOname());
  }
  const char *nfo = pfindfirst(&globals->pfs, "*.NFO");
  while (nfo)
  { if (airInfo.ScanInfoFile(nfo))  MakeSlot(cPlane,true,(char*)nfo);
    nfo = pfindnext (&globals->pfs);
  }
  airBOX.SortAndDisplay();
  return;
}
//-------------------------------------------------------------------------
//  Change aircraft
//-------------------------------------------------------------------------
void CWinPlane::ChangeAircraft()
{ CAirLine *slot = (CAirLine*)airBOX.GetSelectedSlot();
  char     *nfo  = slot->GetNFO();
  globals->sit->ClearUserVehicle();
	// Release camera constraints ---------------------------------
	globals->sit->SetAircraftFrom(nfo);
  Close();
  // sdk: save a pointer to 'user' as the first item in the sdk SFlyObjectRef list
  globals->plugins->sdk_flyobject_list.InsertUserInFirstPosition (globals->pln);
  // sdk: used to load dll gauges when switching between aircrafts
  //    TRACE ("RESET USER VEHICLE %d", globals->plugins_num);
  if (globals->plugins_num) {
    globals->plugins->On_Instantiate_DLLSystems (0, 0, NULL);
  }
  return;
}
//-------------------------------------------------------------------------
//  EVENT notifications
//-------------------------------------------------------------------------
void  CWinPlane::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ switch (idm)  {
  case 'sysb':
    return SystemHandler(evn);

  case 'crft':
      airBOX.VScrollHandler((U_INT)itm,evn);
      if (evn != EVENT_DBLE_CLICK) return;
      return ChangeAircraft();
  //---Change aircraft ----------------------
  case 'okok':
      return ChangeAircraft();
  }
  return;
}
//----------------------------------------------------------------------------------
//  Edit popup Selection
//----------------------------------------------------------------------------------
void CWinPlane::EditSelPopup()
{ selPOP->CreatePage(&mSEL,AirMENU);
  SetPopTitle(0);
  return;
}
//----------------------------------------------------------------------------------
//  Set Popup title and reselect all aircrafts according to filter
//----------------------------------------------------------------------------------
void CWinPlane::SetPopTitle(U_SHORT No)
{ selOpt  = No;
  selPOP->SetButtonText((char*)mSEL.aText[No]);
  airBOX.EmptyIt();
  ScanAllNFO();
  return;
}
//--------------------------------------------------------------------------
//  Notification from popup
//--------------------------------------------------------------------------
void CWinPlane::NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn)
{ if (evn == EVENT_POP_CLICK) SetPopTitle((U_INT)itm);
  return;
}

//============================END OF FILE ================================================================================
