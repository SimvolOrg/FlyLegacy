/*
 * WindowVehOptions.cpp
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
#include "../Include/FuiUser.h"
//==========================================================================

//==========================================================================
//  Window to present and manage user vehicle options
//==========================================================================
CFuiVehOption::CFuiVehOption(Tag idn, const char *filename)
:CFuiWindow(idn,filename,280,220,0)
{ //---Init windows -----------------------------------------
  mveh  = globals->pln;
  char  ok = 0;
  //--- Get components --------------------------------------
  title = 1;
  close = 1;
  SetText("AIRCRAFT OPTIONS");
  aBOX  = new CFuiGroupBox(10,8,260,60,this);
  AddChild('abox',aBOX,"");
  aLAB  = new CFuiLabel( 4, 4,76, 20, this);
  //--- Check auto pilot features --------------------------
  //-- Autopilot autoland ---------------------------
  aBOX->AddChild('labl',aLAB,"AUTO PILOT:");
  rLND  = new CFuiCheckBox(80, 0,160,20,this);
  ok    = GetOption(VEH_AP_LAND);
  rLND->IniState(ok);
  aBOX->AddChild('rlnd',rLND,"Auto Land");
  //---Autopilot auto disconnect --------------------
  rDIS  = new CFuiCheckBox(80,20,160,20,this);
  ok    = GetOption(VEH_AP_DISC);
  rDIS->IniState(ok);
  aBOX->AddChild('rdis',rDIS,"Auto Disconnect");
  //---Crash detector --------------------------------
  cDET  = new CFuiCheckBox(10,  80, 200,20,this);
  ok    = GetOption(VEH_D_CRASH);
  cDET->IniState(ok);
  AddChild('cdet',cDET,"Crash Detection");
  //---Help on panel --------------------------------
  hlPN  = new CFuiCheckBox(10, 100, 200,20,this);
  ok    = GetOption(VEH_PN_HELP);
  hlPN->IniState(ok);
  AddChild('help',hlPN,"Panel Help");
  //---Smoke box ------------------------------------
  smBX  = new CFuiCheckBox(10, 120,200,20,this);
  ok    = GetOption(VEH_DW_SMOK);
  smBX->IniState(ok);
  AddChild('smbx',smBX,"Draw Smoke");
  //---shadow box ------------------------------------
  shBX  = new CFuiCheckBox(10, 140,200,20,this);
  ok    = GetOption(VEH_DW_SHAD);
  shBX->IniState(ok);
  AddChild('shbx',shBX,"Draw Shadow");
  //--- Position --------------------------------------
  psBX  = new CFuiCheckBox(10, 160,200,20,this);
  ok    = GetOption(VEH_DW_VPOS);
  psBX->IniState(ok);
  AddChild('psbx',psBX,"Draw Position");
  //---- Aero vectors ---------------------------------
  aeBX  = new CFuiCheckBox(10, 180,200,20,this);
  ok    = GetOption(VEH_DW_AERO);
  aeBX->IniState(ok);
  AddChild('aebx',aeBX,"Draw Aero vectors");

  //--------------------------------------------------------
  ReadFinished();
	if (0 == mveh)	Close();
}
//--------------------------------------------------------------
//  Check for option
//--------------------------------------------------------------
char CFuiVehOption::GetOption(U_INT p)
{ return (globals->pln->HasOPT(p))?(1):(0);}

//--------------------------------------------------------------
//  Init the window
//--------------------------------------------------------------
void CFuiVehOption::ReadFinished()
{ CFuiWindow::ReadFinished();
  if (0 == surface) gtfo("CFuiVehOption: no surface");
}
//---------------------------------------------------------------------
//  Intercept events
//---------------------------------------------------------------------
void CFuiVehOption::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ switch (idm)  {
    case 'sysb':
      SystemHandler(evn);
      return;
    //--- Autopilot auto land  -------------------------
    case 'rlnd':
      mveh->ToggleOPT(VEH_AP_LAND);
      return;
    //--- Autopilot autodisconnect ---------------------
    case 'rdis':
      mveh->ToggleOPT(VEH_AP_DISC);
      return;
    //--- Crash detection ------------------------------
    case 'cdet':
      mveh->ToggleOPT(VEH_D_CRASH);
      return;
    //--- Panel help -----------------------------------
    case 'help':
      mveh->ToggleOPT(VEH_PN_HELP);
      return;
    //--- Draw smoke -----------------------------------
    case 'smbx':
      mveh->ToggleOPT(VEH_DW_SMOK);
      return;
    //--- Draw shadow -----------------------------------
    case 'shbx':
      mveh->ToggleOPT(VEH_DW_SHAD);
      return;
    //--- Draw position ---------------------------------
    case 'psbx':
      mveh->ToggleOPT(VEH_DW_VPOS);
      return;
    //--- Draw aero vectors -----------------------------
    case 'aebx':
      mveh->ToggleOPT(VEH_DW_AERO);
      return;

  }
  return;
}

//=======================END OF FILE ==================================================