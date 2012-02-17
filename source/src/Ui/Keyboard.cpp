/*
 * Keyboard.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2006 Chris Wallace
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

/*! \file Keyboard.cpp
 *  \brief Implements bindings of keyboard keys to sim actions
 */

#include "../Include/FlyLegacy.h"
#include "../Include/Ui.h"
#include "../Include/Globals.h"
#include "../Include/UserVehicles.h"
#include "../Include/Situation.h"
#include "../Include/Cameras.h"
#include "../Include/KeyMap.h"
#include "../Include/Fui.h"
//=================================================================================
//  Default  group key
//================================================================================
bool KeyDefGroup(CKeyDefinition *kdf,int code)
{ if (kdf->NoPCB())   return false;
  Tag kid   = kdf->GetTag();
  U_INT key = code & 0x0000FFFF;      // Key code
  U_INT mod = (code >> 16);           // Modifier
  return kdf->GetCallback() (kid,key,mod);
}
//=================================================================================
//  Unbind  group key
//================================================================================
bool GroupUnbind(CKeyDefinition *kdf,int code)
{ return false; }
//================================================================================
//  Redirect the call to a new group 
//  This function permit redirection of key even if the key is not defined for
//  the target group in the Key file.
//  This is usefull for menus that are addressing functions pertaining to a vehicle
//  This ensure that the correct vehicle is addressed
//  For instance the GPS window should display the correct GPS associated to the
//  vehicle
//================================================================================
bool KeyRedirect(Tag grp,Tag kid)
{ CKeySet* ks = globals->kbd->FindKeySetById(grp);
  if (0 == ks)  return false;
  CKeyDefinition *kdf = ks->GetKeyByTag(kid);
  if (0 == kdf) return false;
  U_INT code = kdf->GetCode();
  U_INT key  = code & 0x0000FFFF;      // Key code
  U_INT mod  = (code >> 16);           // Modifier
  return kdf->GetCallback() (kid,key,mod);
}
//===============================================================
//  global functions for Menu
//===============================================================
//  File Load 'load'
//---------------------------------------------------------------
bool mKeyLOAD(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_SITUATION_LOAD);
  return true; }
//---------------------------------------------------------------
//  Quit 'quit'
//---------------------------------------------------------------
bool mKeyQUIT(int kid,int code,int mod)
{ globals->appState = APP_EXIT;
  return true; }
//---------------------------------------------------------------
//  Option video 'ogrp'
//---------------------------------------------------------------
bool mKeyOGRP(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_OPTIONS_VIDEO);
  return true; }
//---------------------------------------------------------------
//  Option audio 'osnd'
//---------------------------------------------------------------
bool mKeyOSND(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_OPTIONS_AUDIO);
  return true; }
//---------------------------------------------------------------
//  Option date 'date'
//---------------------------------------------------------------
bool mKeyDATE(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_OPTIONS_DATE_TIME);
  return true; }
//---------------------------------------------------------------
//  Option Keys 'keys'
//---------------------------------------------------------------
bool mKeyKEYS(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_OPTIONS_KEYS_BUTTONS);
  return true; }
//---------------------------------------------------------------
//  Option Axes 'axes'
//---------------------------------------------------------------
bool mKeyAXES(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow(FUI_WINDOW_OPTIONS_SETUP_AXES);
  return true; }
//---------------------------------------------------------------
//  Option Test controls 'tstc'  NOT USED ANYMORE
//---------------------------------------------------------------
bool mKeyTSTC(int kid,int code,int mod)
{ return true; }
//---------------------------------------------------------------
//  Option Start up 'gogo'
//---------------------------------------------------------------
bool mKeyGOGO(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_OPTIONS_STARTUP);
  return true; }
//---------------------------------------------------------------
//  Option Scenary  'tree'
//---------------------------------------------------------------
bool mKeyTREE(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_OPTIONS_SCENERY);
  return true; }
//---------------------------------------------------------------
//  Option directory  'vdir'
//---------------------------------------------------------------
bool mKeyVDIR(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_WAYPOINT_DIRECTORY);
  return true; }
//---------------------------------------------------------------
//  Flight plan list  'lfpn'
//---------------------------------------------------------------
bool mKeyLFPN(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_FPLAN_LIST);
  return true; }
//---------------------------------------------------------------
//  Current flight plan  'cfpn'
//---------------------------------------------------------------
bool mKeyCFPN(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_FPLAN_LOG);
  return true; }
//-----------------------------------------------------------------
//  Sectional chart window   'chrt'
//-----------------------------------------------------------------
bool mKeyCHRT(int kid, int code, int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_CHART);
  return true; }
//-----------------------------------------------------------------
//  Vector MAP   'vwin'
//-----------------------------------------------------------------
bool mKeyVWIN(int kid, int code, int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_VECTOR_MAP);
  return true; }
//---------------------------------------------------------------
//  Option teleport  'goto'
//---------------------------------------------------------------
bool mKeyGOTO(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_TELEPORT);
  return true; }
//---------------------------------------------------------------
//  Aircraft detail info  'adet': intercepted by aircraft or false
//---------------------------------------------------------------
bool mKeyADET(int kid,int code,int mod)
{ return false; }
//---------------------------------------------------------------
//  Aircraft select  'sair'
//---------------------------------------------------------------
bool mKeySAIR(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_VEHICLE_SELECT);
  return true; }
//---------------------------------------------------------------
//  Aircraft check list  'cklw': Redirect to vehicle of false
//---------------------------------------------------------------
bool mKeyCKLW(int kid,int code,int mod)
{ return false; }
//---------------------------------------------------------------
//  Fuel set up  'sful': Intercepted by vehicle or false
//---------------------------------------------------------------
bool mKeySFUL(int kid,int code,int mod)
{ return false; }
//---------------------------------------------------------------
//  Weight and baggage 'sbag': Intercepted or false
//---------------------------------------------------------------
bool mKeySBAG(int kid,int code,int mod)
{ return false; }
//---------------------------------------------------------------
//  Aircraft CoG (Center of Gravity): Intercepted or false
//---------------------------------------------------------------
bool mKeyCGIN(int kid,int code,int mod)
{ return false; }
//---------------------------------------------------------------
//  Aircraft damage report: Intercepted or false
//---------------------------------------------------------------
bool mKeyOUCH(int kid,int code,int mod)
{ return false; }
//---------------------------------------------------------------
//  Aircraft reset: Intercepted or false
//---------------------------------------------------------------
bool mKeyRSET(int kid,int code,int mod)
{ return false; }

//----------------------------------------------------------------------
//  Weather overview 'envo'
//----------------------------------------------------------------------
bool mKeyENVO(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow(FUI_WINDOW_WEATHER_OVERVIEW); 
  return true; }
//----------------------------------------------------------------------
//  Weather almanac 'alma'
//----------------------------------------------------------------------
bool mKeyALMA(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow(FUI_WINDOW_WEATHER_ALMANAC); 
  return true; }
//----------------------------------------------------------------------
//  Weather clouds 'envc'
//----------------------------------------------------------------------
bool mKeyENVC(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow(FUI_WINDOW_WEATHER_CLOUDS); 
  return true; }
//----------------------------------------------------------------------
//  Weather winds 'envw'
//----------------------------------------------------------------------
bool mKeyENVW(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow(FUI_WINDOW_WEATHER_WINDS); 
  return true; }
//----------------------------------------------------------------------
//  Sky tweaker  'skyt'
//----------------------------------------------------------------------
bool mKeySKYT(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow(FUI_WINDOW_WEATHER_SKY_TWEAKER); 
  return true; }
//----------------------------------------------------------------------
//  GPS windows  'gwin': Should be intercepted by vehicle
//  If not, the key is not supported by the vehicle
//----------------------------------------------------------------------
bool mKeyGWIN(int kid,int code,int mod)
{ return false;  }
//----------------------------------------------------------------------
//  Aero vector: Intercepted or false
//----------------------------------------------------------------------
bool mKeyAERV(int kid,int code,int mod)
{ return false;  }
//----------------------------------------------------------------------
//  Aero position: Intercepted or false
//----------------------------------------------------------------------
bool mKeyAERP(int kid,int code,int mod)
{ return false;  }
//----------------------------------------------------------------------
//  Draw smoke: Intercepted or false
//----------------------------------------------------------------------
bool mKeySMOK(int kid,int code,int mod)
{ return false;  }
//----------------------------------------------------------------------
//  Tune PID: Intercepted or false
//----------------------------------------------------------------------
bool mKeyTPID(int kid,int code,int mod)
{ return false;  }
//----------------------------------------------------------------------
//  Statistic: 'cntr'
//----------------------------------------------------------------------
bool mKeyCNTR(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow(FUI_WINDOW_STATS);
  return true;  }
//----------------------------------------------------------------------
//  Probe: 'prob'
//----------------------------------------------------------------------
bool mKeyPROB(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow(FUI_WINDOW_PROBE);
  return true;  }
//----------------------------------------------------------------------
//  Model browser: 'mbro'
//----------------------------------------------------------------------
bool mKeyMBRO(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow(FUI_WINDOW_MBROS);
  return true;  }
//----------------------------------------------------------------------
//  Terra browser: 'tbro'
//----------------------------------------------------------------------
bool mKeyTBRO(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow(FUI_WINDOW_TBROS);
  return true;  }
//----------------------------------------------------------------------
//  toggle  plotter: 'plot'
//----------------------------------------------------------------------
bool mKeyPLOT(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow(FUI_WINDOW_PLOT);
  return true;  }
//-----------------------------------------------------------------
//  Open global Terra editor
//-----------------------------------------------------------------
bool mKeyWTED(int kid,int code, int mod)
{ globals->fui->ToggleFuiWindow(FUI_WINDOW_TEDITOR);
  return true;
}
//-----------------------------------------------------------------
//  Open Sketch editor
//-----------------------------------------------------------------
bool mKeyMKCH(int kid,int code, int mod)
{ globals->fui->ToggleFuiWindow(FUI_WINDOW_SKETCH);
  return true;
}

//============================================================================
//  Bind Menu Keys
//============================================================================
static void BindMenuKeys (CKeyMap *keymap)
{ keymap->BindGroup('menu',KeyDefGroup);
  //----------------------------------------------------------
  keymap->Bind('load', mKeyLOAD, KEY_SET_ON);
  keymap->Bind('quit', mKeyQUIT, KEY_SET_ON);
  keymap->Bind('ogrp', mKeyOGRP, KEY_SET_ON);
  keymap->Bind('osnd', mKeyOSND, KEY_SET_ON);
  keymap->Bind('date', mKeyDATE, KEY_SET_ON);
  keymap->Bind('keys', mKeyKEYS, KEY_SET_ON);
  keymap->Bind('axes', mKeyAXES, KEY_SET_ON);
  keymap->Bind('tstc', mKeyTSTC, KEY_SET_ON);
  keymap->Bind('gogo', mKeyGOGO, KEY_SET_ON);
  keymap->Bind('tree', mKeyTREE, KEY_SET_ON);
  keymap->Bind('vdir', mKeyVDIR, KEY_SET_ON);
  keymap->Bind('goto', mKeyGOTO, KEY_SET_ON);
  keymap->Bind('sair', mKeySAIR, KEY_SET_ON);
  keymap->Bind('lfpn', mKeyLFPN, KEY_SET_ON);
  keymap->Bind('cfpn', mKeyCFPN, KEY_SET_ON);
  keymap->Bind('chrt', mKeyCHRT, KEY_SET_ON);
  keymap->Bind('vwin', mKeyVWIN, KEY_SET_ON);
  keymap->Bind('adet', mKeyADET, KEY_SET_ON);
  keymap->Bind('cklw', mKeyCKLW, KEY_SET_ON);
  keymap->Bind('sful', mKeySFUL, KEY_SET_ON);
  keymap->Bind('sbag', mKeySBAG, KEY_SET_ON);
  keymap->Bind('cgin', mKeyCGIN, KEY_SET_ON);
  keymap->Bind('ouch', mKeyOUCH, KEY_SET_ON);
  keymap->Bind('rset', mKeyRSET, KEY_SET_ON);
  keymap->Bind('envo', mKeyENVO, KEY_SET_ON);
  keymap->Bind('alma', mKeyALMA, KEY_SET_ON);
  keymap->Bind('envc', mKeyENVC, KEY_SET_ON);
  keymap->Bind('envw', mKeyENVW, KEY_SET_ON);
  keymap->Bind('skyt', mKeySKYT, KEY_SET_ON);
  keymap->Bind('gwin', mKeyGWIN, KEY_SET_ON);
  keymap->Bind('aerv', mKeyAERV, KEY_SET_ON);
  keymap->Bind('aerp', mKeyAERP, KEY_SET_ON);
  keymap->Bind('smok', mKeySMOK, KEY_SET_ON);
  keymap->Bind('tpid', mKeyTPID, KEY_SET_ON);
  keymap->Bind('cntr', mKeyCNTR, KEY_SET_ON);
  keymap->Bind('prob', mKeyPROB, KEY_SET_ON);
  keymap->Bind('mbro', mKeyMBRO, KEY_SET_ON);
  keymap->Bind('tbro', mKeyTBRO, KEY_SET_ON);
  keymap->Bind('plot', mKeyPLOT, KEY_SET_ON);
	keymap->Bind('wted', mKeyWTED, KEY_TOGGLE);
	keymap->Bind('skch', mKeyMKCH, KEY_TOGGLE);
  return;
}
//=================================================================================
//  Function time forward
//================================================================================
bool TimeForward()
{ // Get discrete/continuous flag from INI settings
  bool continuous = false;
  int i = 0;
  GetIniVar ("Sim", "timeAdvanceContinuous", &i);
  continuous = (i != 0);
  if (continuous) {
    // Time advance is continuous
    globals->tim->TimeForward();
    return true;
  }
  // Time advance is discrete; get time step from INI settings or default to 30 secs
  int step = 30;
  GetIniVar ("Sim", "timeAdvanceStep", &step);

  // Apply time step to time manager
  SDateTimeDelta delta;
  delta.dYears = delta.dMonths = delta.dDays = delta.dHours = 0;
  delta.dMinutes = step;
  delta.dSeconds = delta.dMillisecs = 0;

  SDateTime dt = globals->tim->GetUTCDateTime ();
  dt = globals->tim->AddTimeDelta (dt, delta);
  globals->tim->SetUTCDateTime (dt);
  return true;
}
//=================================================================================
//  Function time Backward
//================================================================================
bool TimeBackward ()
{ // Get discrete/continuous flag from INI settings
  bool continuous = false;
  int i = 0;
  GetIniVar ("Sim", "timeAdvanceContinuous", &i);
  continuous = (i != 0);
  if (continuous) {
    // Time advance is continuous
    globals->tim->TimeBackward();
    return true;
  }  
  // Time advance is discrete; get time step from INI settings
  int step = 30;
  GetIniVar ("Sim", "timeAdvanceStep", &step);
  // Apply time step to time manager
  SDateTimeDelta delta;
  delta.dYears = delta.dMonths = delta.dDays = delta.dHours = 0;
  delta.dMinutes = step;
  delta.dSeconds = delta.dMillisecs = 0;

  SDateTime dt = globals->tim->GetUTCDateTime ();
  dt = globals->tim->SubtractTimeDelta (dt, delta);
  globals->tim->SetUTCDateTime (dt);
  return true;
}
//=================================================================================
//  Function Edit local time
//================================================================================
void EditDate()
{ char msg[80];
  CClock *clk = globals->clk;
  sprintf(msg,"Date: %04d/%02d/%02d  Time: %02dh:%02dmn", clk->GetYear(),
                                                clk->GetMonth(),
                                                clk->GetDay(),
                                                clk->GetHour(),
                                                clk->GetMinute());
  DrawNoticeToUser (msg, 5);
  return;
}

//========================================================================
//  functions for Global keys
//========================================================================
//  Cockpit toggle 'cock'
//----------------------------------------------------------------
bool gKeyCOCK(int kid, int code, int mod)
{ return true;
}
//-----------------------------------------------------------------
//  Next vehicle 'nveh'
//-----------------------------------------------------------------
bool gKeyNVEH(int kid, int code, int mod)
{ return true;
}
//-----------------------------------------------------------------
//  Previous vehicle 'pveh'
//-----------------------------------------------------------------
bool gKeyPVEH(int kid, int code, int mod)
{ return true;
}
//-----------------------------------------------------------------
//  Home vehicle 'hveh'
//-----------------------------------------------------------------
bool gKeyHVEH(int kid, int code, int mod)
{ return true;
}
//-----------------------------------------------------------------
//  Next camera   'camn'
//-----------------------------------------------------------------
bool gKeyCAMN(int kid, int code, int mod)
{ CCameraManager *ccm = globals->ccm;
  if (ccm) ccm->NextCamera();
  return true;
}
//-----------------------------------------------------------------
//  Previous camera   'camp'
//-----------------------------------------------------------------
bool gKeyCAMP(int kid, int code, int mod)
{ CCameraManager *ccm = globals->ccm;
  if (ccm) ccm->PrevCamera();
  return true;
}
//-----------------------------------------------------------------
//  Menu   'menu'
//-----------------------------------------------------------------
bool gKeyMENU(int kid, int code, int mod)
{ toggle_menu ();
  return true;
}
//-----------------------------------------------------------------
//  Time forward   'timf'
//-----------------------------------------------------------------
bool gKeyTIMF(int kid, int code, int mod)
{ TimeForward();
  globals->clk->Set();
  EditDate();
  return true;
}
//-----------------------------------------------------------------
//  Time backward   'timb'
//-----------------------------------------------------------------
bool gKeyTIMB(int kid, int code, int mod)
{ TimeBackward();
  globals->clk->Set();
  EditDate();
  return true;
}
//-----------------------------------------------------------------
//  Pause   'paus'
//-----------------------------------------------------------------
bool gKeyPAUS(int kid, int code, int mod)
{ globals->tim->SwapPause();
  return true;
}
//-----------------------------------------------------------------
//  Chart window   'mwin'
//-----------------------------------------------------------------
bool gKeyMWIN(int kid, int code, int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_CHART);
  return true;
}
//-----------------------------------------------------------------
//  GPS window   'gk89'
//-----------------------------------------------------------------
bool gKeyGK89(int kid, int code, int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_KLN89);
  return true;
}
//-----------------------------------------------------------------
//  Frame rate   'fpsw'
//-----------------------------------------------------------------
bool gKeyFPSW(int kid, int code, int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_FRAME_RATE);
  return true;
}
//-----------------------------------------------------------------
//  Axis window   'axis'
//-----------------------------------------------------------------
bool gKeyAXIS(int kid, int code, int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_OPTIONS_SETUP_AXES);
  return true;
}
//-----------------------------------------------------------------
//  Sim debug window   'dbgS'
//-----------------------------------------------------------------
bool gKeyDBGS(int kid, int code, int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_DEBUG_SIM);
  return true;
}
//-----------------------------------------------------------------
//  Toggle Status bar window   'stat'
//-----------------------------------------------------------------
bool gKeySTAT(int kid, int code, int mod)
{ globals->sBar ^= 1;
  return true;
}
//-----------------------------------------------------------------
//  Toggle Status airport alight
//-----------------------------------------------------------------
bool gKeyALIT(int kid, int code, int mod)
{ globals->aptOpt.Toggle(APT_OPT_ALIT);
  return true;
}
//-----------------------------------------------------------------
//  Open global options windwos
//-----------------------------------------------------------------
bool gKeyGOPT(int kid,int code, int mod)
{ globals->fui->ToggleFuiWindow(FUI_WINDOW_GLOBAL_OPTIONS);
  return true;
}

//=======================================================================
//  Bind global Keys
//=======================================================================
static void BindGlobalKeys (CKeyMap *keymap)
{ //---Bind the group keys ----------------------------------
  keymap->BindGroup('glob',KeyDefGroup);
  //---------------------------------------------------------
  keymap->Bind ('cock', gKeyCOCK, KEY_SET_ON);
  keymap->Bind ('nveh', gKeyNVEH, KEY_SET_ON);
  keymap->Bind ('pveh', gKeyPVEH, KEY_SET_ON);
  keymap->Bind ('hveh', gKeyHVEH, KEY_SET_ON);
  keymap->Bind ('camn', gKeyCAMN, KEY_SET_ON);
  keymap->Bind ('camp', gKeyCAMP, KEY_SET_ON);
  keymap->Bind ('menu', gKeyMENU, KEY_SET_ON);
  keymap->Bind ('timf', gKeyTIMF, KEY_REPEAT);
  keymap->Bind ('timb', gKeyTIMB, KEY_REPEAT);
  keymap->Bind ('paus', gKeyPAUS, KEY_SET_ON);
  keymap->Bind ('mwin', gKeyMWIN, KEY_SET_ON);
  keymap->Bind ('gk89', gKeyGK89, KEY_SET_ON);                    
  keymap->Bind ('fpsw', gKeyFPSW, KEY_SET_ON);
  keymap->Bind ('axis', gKeyAXIS, KEY_SET_ON);
  keymap->Bind ('dbgS', gKeyDBGS, KEY_SET_ON);
  keymap->Bind ('stat', gKeySTAT, KEY_TOGGLE);                     
  keymap->Bind ('alit', gKeyALIT, KEY_TOGGLE);
  keymap->Bind ('gopt', gKeyGOPT, KEY_TOGGLE);
/*
KeySet 'glob' Global Keys
  'stat' Status Bar
  'rsit' Reset Situation
  'ival' Increase Value
  'dval' Decrease Value
  'cycl' Cycle Windows
  'gwin' GPS Window
  'cklw' Checklist Window
  'ssht' Screen Shot
  'cdis' Compress Distance
  'ddis' Decompress Distance
  'bbox' Instant Replay
  'mike' Microphone/LiveMic
  'mnud' Menu Down
  'mnuu' Menu Up
  'mnul' Menu Left
  'mnur' Menu Right
  'mnua' Menu Accept
  'mnux' Menu Abort
  'atct' ATC On/Off
  'attn' Auto Tune Comm 1
  'atcm' ATC Menu
  'atc1' ATC Phrase #1
  'atc2' ATC Phrase #2
  'atc3' ATC Phrase #3
  'atc4' ATC Phrase #4
  'atc5' ATC Phrase #5
  'atc6' ATC Phrase #6
  'atc7' ATC Phrase #7
  'atc8' ATC Phrase #8
  'atc9' ATC Phrase #9
  'atc0' ATC Phrase #10
  'chat' Chat Window
  'phlp' Range Finder
  'vsfr' Sim Frame Rate
  'ifr_' Toggle VFR/IFR Panels
  'sprf' Show Profiler Dialog
  'skyt' Sky Tweaker
  'dbgS' Sim Debug Displays
  'dllw' DLL Task Window
  'carg' Jettison Cargo
  'repr' Repair & Revive
  'moov' Toggle Movie Capture
  'jpeg' JPEG Screenshot
  'g430' GNS430 Popup Window
  'ftkR' FlyTrak Record
  'ftkP' FlyTrak Playback
  'ftkC' FlyTrak Configuration
*/
}


//
// JS: Flight Planner Keys  NO MORE FLIGHT PLANNER.
//  Dont delete to have a list of available keys

/*
  'sfpl' Flight Planner
  'navl' Navigation Log...
  'fpck' FP CheckList...
  'logb' Log Book...
  'vdir' Directory...
  'cwpt' Current Waypoint
  'sair' Select Aircraft
  'ouch' Damage Report
  'sful' Setup Fuel
  'sbag' Weight & Baggage
  'cgin' CG Indicator
  'adet' Aircraft Details
  'envo' Weather Overview
  'envc' Clouds
  'envw' Winds
  'envm' Other Weather
  'metr' Load METAR
  'MPst' Multiplayer...
  'dcon' Disconnect
  'fndp' Find Pilot...
  'motd' Show MOTD...
  'admn' Sysop...
  'who ' About Fly
  'webs' Support Web Site
  'webt' TRI Web Site
  'webg' Gathering Web Site
  'uedt' UI Editor...
  'thmn' Theme Manager...
  'medt' Model Editor...
  'sedt' Scenery Editor...
 */


/*
 * Helicopter keys
 */

/*
KeySet 'heli' Helicopter Keys
  'fcyc' Fore Cyclic
  'acyc' Aft Cyclic
  'rcyc' Right Cyclic
  'lcyc' Left Cyclic
  'ltrt' Left Tail Rotor
  'rtrt' Right Tail Rotor
  'upcl' Up Collective
  'dncl' Down Collective
  'mfcy' Max Fore Cyclic
  'macy' Max Aft Cyclic
  'mrcy' Max Right Cyclic
  'mlcy' Max Left Cyclic
  'mltr' Max Left Tail Rotor
  'mrtr' Max Right Tail Rotor
  'mucl' Max Up Collective
  'mdcl' Max Down Collective
  'mvrt' Min Vertical Rate
  'zvrt' Zero Vertical Rate
  'altt' Altimeter Mode
  'hctr' Center Controls (Partial)
  'hcta' Center Controls (All)
  'hmit' Throttle Off
  'hmat' Throttle Full
  'hdth' Throttle Down
  'hith' Throttle Up
  'fcyT' Fore Cyclic Trim
  'acyT' Aft Cyclic Trim
  'rcyT' Right Cyclic Trim
  'lcyT' Left Cyclic Trim
  'ltrT' Left Tail Rotor Trim
  'rtrT' Right Tail Rotor Trim
  'upcT' Up Collective Trim
  'dncT' Down Collective Trim
KeySet 'grnd' Ground Vehicle Keys
  'strl' Steer Left
  'strr' Steer Right
  'trup' Throttle Up
  'trdn' Throttle Down
  'maxs' Max Speed
  'mins' Min Speed
  'gctr' Center Controls (Partial)
  'gcta' Center Controls (All)
*/

//==========================================================================
// DEBUG
//===========================================================================


/*
 * Debug Keys
 */

/*
  KeySet 'dbug' Debug Keys
  'grph' Graph
  'dbps' Debug Pause
  'dbss' Debug Single Step
  'aipl' AI Pilot
  'wpln' Weapon Launch
  'pttr' Pitch Trim
  'hovr' Hover
  'ahld' Altitude Hold
  'hhld' Heading Hold
  'phld' Pitch Hold
  'bhld' Bank Hold
  'shld' Speed Hold
  'aagl' Altitude AGL Hold
  'vrhl' Vertical Rate Hold
  'albu' Altitude Bump Up
  'hdbu' Heading Bump Up
  'ptbu' Pitch Bump Up
  'bnbu' Bank Bump Up
  'spbu' Speed Bump Up
  'aabu' Altitude AGL Bump Up
  'vrbu' Vertical Rate BumpUp
  'mxvr' Max Vertical Rate
  'albd' Altitude Bump Down
  'hdbd' Heading Bump Down
  'ptbd' Pitch Bump Down
  'bbdn' Bank Bump Down
  'spbd' Speed Bump Down
  'aabd' Altitude AGL Bump Dn
  'vrbd' Vertical Rate BumpDn
  'push' Push
  'pshb' Push Back
  'mrep' Memory report
  'clon' Clone
  'kill' Kill
KeySet 'misc' Misc Vehicle Keys
  'vvup' Virtual View Up
  'vvdn' Virtual View Down
  'vvlt' Virtual View Left
  'vvrt' Virtual View Right
  'mids' Mid Speed
  'away' Add Waypoint
*/


void BindAllKeys (CKeyMap *keymap)
{
  BindMenuKeys   (keymap);
  BindGlobalKeys (keymap);
}

/*
 * Keyboard input handlers
 */


/// \todo Add caps, CR (13), shifted special characters
SGlutToFlyLegacyKey glutKeyMap[] =
{
  // Control keys
  { 1,            1 },
  { 2,            2 },
  { 3,            3 },
  { 4,            4 },
  { 5,            5 },
  { 6,            6 },
  { 7,            7 },
  { 8,            KB_KEY_BACK },
  { 9,            KB_KEY_TAB },
  { 10,           10 },
  { 11,           11 },
  { 12,           12 },
  { 13,           KB_KEY_ENTER },
  { 14,           14 },
  { 15,           15 },
  { 16,           16 },
  { 17,           17 },
  { 18,           18 },
  { 19,           19 },
  { 20,           20 },
  { 21,           21 },
  { 22,           22 },
  { 23,           23 },
  { 24,           24 },
  { 25,           25 },
  { 26,           26 },

  // Top row
  { 27,           KB_KEY_ESC },
  { '`',          KB_KEY_REVERSE_SINGLE_QUOTE },
  { '1',          '1' },
  { '2',          '2' },
  { '3',          '3' },
  { '4',          '4' },
  { '5',          '5'  },
  { '6',          '6' },
  { '7',          '7' },
  { '8',          '8' },
  { '9',          '9' },
  { '0',          '0' },
  { '-',          KB_KEY_MINUS },
  { '=',          KB_KEY_EQUALS },

  // Shifted top row
  { '~',          KB_KEY_TILDE },
  { '!',          KB_KEY_EXCLM },
  { '@',          KB_KEY_AROBS },
  { '#',          KB_KEY_DIEZE },
  { '$',          KB_KEY_DOLAR },
  { '%',          KB_KEY_PERCN },
  { '^',          KB_KEY_HATOV },
  { '&',          KB_KEY_AMPER },
  { '*',          KB_KEY_STAR  },
  { '(',          KB_KEY_LEFTP },
  { ')',          KB_KEY_RITEP },
  { '_',          KB_KEY_UNDER },
  { '+',          KB_KEY_PLUS },

  // Second row
  { 'q',          'q' },
  { 'w',          'w' },
  { 'e',          'e' },
  { 'r',          'r' },
  { 't',          't' },
  { 'y',          'y' },
  { 'u',          'u' },
  { 'i',          'i' },
  { 'o',          'o' },
  { 'p',          'p' },
  { '[',          KB_KEY_FORWARD_BRACKET },
  { ']',          KB_KEY_REVERSE_BRACKET },
  { '\\',         KB_KEY_BACKSLASH },

  // Shifted second row
  { 'Q',          'Q' },
  { 'W',          'W' },
  { 'E',          'E' },
  { 'R',          'R' },
  { 'T',          'T' },
  { 'Y',          'Y' },
  { 'U',          'U' },
  { 'I',          'I' },
  { 'O',          'O' },
  { 'P',          'P' },
  { '{',          KB_KEY_LEFTB },
  { '}',          KB_KEY_RITEB },
  { '|',          KB_KEY_VBAR },

  // Third row
  { 'a',          'a' },
  { 's',          's' },
  { 'd',          'd' },
  { 'f',          'f' },
  { 'g',          'g' },
  { 'h',          'h' },
  { 'j',          'j' },
  { 'k',          'k' },
  { 'l',          'l' },
  { ';',          KB_KEY_SEMI_COLON },

  // Shifted third row
  { 'A',          'A' },
  { 'S',          'S' },
  { 'D',          'D' },
  { 'F',          'F' },
  { 'G',          'G' },
  { 'H',          'H' },
  { 'J',          'J' },
  { 'K',          'K' },
  { 'L',          'L' },
  { ':',          ':' },
  { '\"',         KB_KEY_DOUBLE_QUOTE},
	{ '%',					KB_KEY_PERCN },
  // Fourth row
  { 'z',          'z' },
  { 'x',          'x' },
  { 'c',          'c' },
  { 'v',          'v' },
  { 'b',          'b' },
  { 'n',          'n' },
  { 'm',          'm' },
  { ',',          KB_KEY_COMMA },
  { '.',          KB_KEY_PERIOD },
  { '/',          KB_KEY_SLASH },

  // Shifted fourth row
  { 'Z',          'Z' },
  { 'X',          'X' },
  { 'C',          'C' },
  { 'V',          'V' },
  { 'B',          'B' },
  { 'N',          'N' },
  { 'M',          'M' },
  { '<',          '<' },
  { '>',          '>' },
  { '?',          KB_KEY_WHAT },
	{ '!',					KB_KEY_EXCLM},
  // Space
  { ' ',          KB_KEY_SPACE },

  // Del
  { 127,          KB_KEY_DEL }
};
//==========================================================================================
//	Ctrl remapping
//==========================================================================================
U_INT	CtrlMAP[]	= {
	0,								// No remap
	'a',							// 0x01
	'b',							// 0x02
	'c',							// 0x03
	'd',							// 0x04
	'e',							// 0x05
	'f',							// 0x06
	'g',							// 0x07
	'h',							// 0x08
	'i',							// 0x09
	'j',							// 0x0A
	'k',							// 0x0B
	'l',							// 0x0C
	'm',							// 0x0D
	'n',							// 0x0E
	'o',							// 0x0F
	'p',							// 0x10
	'q',							// 0x11
	'r',							// 0x12
	's',							// 0x13
	't',							// 0x14
	'u',							// 0x15
	'v',							// 0x16
	'w',							// 0x17
	'x',							// 0x18
	'y',							// 0x19
	'z',							// 0x1A
	0,								// 0x1B
	0,								// 0x1C
	0,								// 0x1D
	0,								// 0x1E
	0,								// 0x1F
};
//==========================================================================================
//	GLUT Special Key re mapping
//==========================================================================================
SGlutToFlyLegacyKey glutSpecialMap[] =
{
  { GLUT_KEY_F1,        KB_KEY_F1 },
  { GLUT_KEY_F2,				KB_KEY_F2 },
  { GLUT_KEY_F3,				KB_KEY_F3 },
  { GLUT_KEY_F4,				KB_KEY_F4 },
  { GLUT_KEY_F5,				KB_KEY_F5 },
  { GLUT_KEY_F6,				KB_KEY_F6 },
  { GLUT_KEY_F7,				KB_KEY_F7 },
  { GLUT_KEY_F8,				KB_KEY_F8 },
  { GLUT_KEY_F9,				KB_KEY_F9 },
  { GLUT_KEY_F10,				KB_KEY_F10 },
  { GLUT_KEY_F11,				KB_KEY_F11 },
  { GLUT_KEY_F12,				KB_KEY_F12 },
  { GLUT_KEY_HOME,			KB_KEY_HOME },
  { GLUT_KEY_UP,				KB_KEY_UP },
  { GLUT_KEY_PAGE_UP,		KB_KEY_PGUP },
  { GLUT_KEY_LEFT,			KB_KEY_LEFT },
  { GLUT_KEY_RIGHT,			KB_KEY_RIGHT },
  { GLUT_KEY_END,				KB_KEY_END },
  { GLUT_KEY_DOWN,			KB_KEY_DOWN },
  { GLUT_KEY_PAGE_DOWN, KB_KEY_PGDN },
  { GLUT_KEY_INSERT,    KB_KEY_INSERT },
};
//===================================================================================
//	Keyboard codes Tables
//===================================================================================
SGlutToFlyLegacyKey glutKeyboard[128];
SGlutToFlyLegacyKey glutSpecial[128];
//------------------------------------------------------------------------------
//	Transform GLUT modifier to Legacy modifier
//------------------------------------------------------------------------------
EKeyboardModifiers glutModifiersToFlyLegacyModifiers (int glutmod)
{
  int flymod = KB_MODIFIER_NONE;
  if (glutmod & GLUT_ACTIVE_SHIFT) flymod |= KB_MODIFIER_SHIFT;
  if (glutmod & GLUT_ACTIVE_CTRL)  flymod |= KB_MODIFIER_CTRL;
  if (glutmod & GLUT_ACTIVE_ALT)   flymod |= KB_MODIFIER_ALT;

  return (EKeyboardModifiers)flymod;
}
//-------------------------------------------------------------------------------
//	Translate Glut KEY to standard ASCII
//-------------------------------------------------------------------------------
U_INT glutKeyToFlyLegacyKey (U_INT gkey, U_INT mdf)
{ if (KB_MODIFIER_CTRL != mdf)	return glutKeyboard[gkey].flylegacy;
	gkey &= 0x1F;
	return CtrlMAP[gkey];
}

//-------------------------------------------------------------------------------
//	Translate Glut Special KEYs to specific keys
//-------------------------------------------------------------------------------
bool glutSpecialToFlyLegacyKey (int glutkey, U_INT *flykey)
{
  *flykey = glutSpecial[glutkey].flylegacy;
  return (*flykey != KB_KEY_META);
}
//=============================================================================
//  Create Keyfile from default if needed
//=============================================================================
//=============================================================================
// Keyboard initialization
//
// This function must be called prior to any keyboard events being handled
//=============================================================================
void CKeyMap::Init(void)
{
//  char *kfn = "SYSTEM/FLYLEGACYKEY.txt";
	char *kfn = "SYSTEM/KEYMAP.txt";
  //---Try for the standard file --------------------
  if (pexists (&globals->pfs, kfn)) 
  {  OpenKey(kfn);
     return;
  }
  //---Try the default file -------------------------
  char *dfn = "SYSTEM/DEFAULT.KEY";
  OpenKey("System/default.key");
  // Create new FlyLegacyKey.txt from contents of Fly! II key settings
	SaveCurrentConfig();
  return;
  }
//---------------------------------------------------------------------------
//  Bind all keys except vehicle keys.  They will be binded by the
//  vehicle
//---------------------------------------------------------------------------
void CKeyMap::BindKeys()
{ BindAllKeys (this);
  int i;
  // Initialize all entries in GLUT->FlyLegacy mapping tables to META (unused)
  for (i=0; i<128; i++) {
    glutKeyboard[i].glut = i;
    glutKeyboard[i].flylegacy = KB_KEY_META;
    glutSpecial[i].glut = i;
    glutSpecial[i].flylegacy = KB_KEY_META;
  }

  // Now set all values specified in the actual maps
  int nKeys = sizeof (glutKeyMap) / sizeof(SGlutToFlyLegacyKey);
  for (i=0; i<nKeys; i++)
	{ U_INT inx = glutKeyMap[i].glut;
	  glutKeyboard[inx].flylegacy = glutKeyMap[i].flylegacy;
  }

  int nSpecialKeys = sizeof (glutSpecialMap) / sizeof (SGlutToFlyLegacyKey);
  for (i=0; i<nSpecialKeys; i++) {
    glutSpecial[glutSpecialMap[i].glut].flylegacy = glutSpecialMap[i].flylegacy;
  }
  return;
}
//==============END OF FILE =======================================================


