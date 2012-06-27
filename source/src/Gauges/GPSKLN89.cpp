/*
 * Gauges.cpp
 *
 * Part of Fly! Legacy project
 *
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

/**
 * @file Gauges.cpp
 * @brief Implements CGauge and related classes for panel gauges
 *
 * Implementation of the many different varieties of panel gauge
 *   classes, including CGauge and all descendents.  Also includes
 *   the implementation of related classes such as CGaugeNeedle,
 *   CGaugeClickArea, etc.
 */

#include "../Include/FlyLegacy.h"
#include "../Include/Gauges.h"
#include "../Include/Ui.h"
#include "../Include/Globals.h"
#include "../Include/Subsystems.h"
#include "../Include/Fui.h"
#include "../Include/AudioManager.h"
#include "../Include/CursorManager.h"
#include "../Include/Weather.h"
#include "../Include/database.h"
#include "../Include/WorldObjects.h"
#include "../Include/PlanDeVol.h"
#include "../Include/BendixKing.h"
#include "../Include/Atmosphere.h"
#include "../Include/RadioGauges.h"
//===========================================================================
//  KLN98 Mode
//===========================================================================
char *modeK89[] =	{
	"F_Plan",
	"Direct",
};
//===========================================================================
//  Flasher attribut (depending on globals clock
//===========================================================================
U_SHORT K89FLASH[] = {
  K89_ATT_NOCAR,
  K89_ATT_ONCAR,
};
//===========================================================================
//  Waypoint mark when following a fligth plan
//===========================================================================
char markWP89[] = {
	' ',							// 0=> Forward waypoint
	'\x85',						// 1=> FROM (Origin) waypoint
	'\x86',						// 2=> TO   (Current)waypoint
	'\x87',						// 3=> Past terminated
};
//===========================================================================
//  Waypoint mark for NAV page
//===========================================================================
char markNV89[] = {
	'\x82',							// 0=>Mode Flight plan
	'\x84',							// 1=>Mode Direct To
};
//===========================================================================
//  VECTOR FUNCTIONS FOR KLN_89
//===========================================================================
CK89gps::StaFN      CK89gps::sfn[K89_STANB] =
{ &CK89gps::PowerOFF,          // State 0
  &CK89gps::PowerONa,          // State 01
  &CK89gps::PowerONb,          // State 02
  &CK89gps::PowerONc,          // State 03
  &CK89gps::PowerONd,          // State 04
  &CK89gps::PowerONg,          // State 05
  &CK89gps::PowerONh,          // State 06
  &CK89gps::PowerONj,          // State 07
  &CK89gps::PowerONk,          // State 08
  //----------------------------------------------------------
  &CK89gps::APTpage01,         // State 09
  &CK89gps::CSRMode01,         // State 10
  &CK89gps::CSRMode02,         // State 11
  &CK89gps::APTpage02,         // State 12
  &CK89gps::APTpage03,         // State 13
  &CK89gps::APTpage04,         // State 14
  &CK89gps::APTpage05,         // State 15
  &CK89gps::APTpage09,         // State 16
  //-----------------------------------------------------------
  &CK89gps::RADpage01,         // State 17
  &CK89gps::RADpage02,         // State 18
  &CK89gps::NAVpage01,         // State 19
  &CK89gps::NAVpage02,         // State 20
  &CK89gps::NAVpage03,         // State 21
  &CK89gps::NAVpage04,         // State 22
  //-----------------------------------------------------------
  &CK89gps::DIRpage01,         // State 23
  &CK89gps::DUPpage02,         // State 24
  &CK89gps::DUPpage01,         // State 25
  //---------------------------------------------------------
  &CK89gps::NRSpage01,         // State 26
  &CK89gps::FPLpage01,         // State 27
  &CK89gps::WPTpage01,         // State 28
  &CK89gps::ALTpage01,         // State 29
  &CK89gps::ALTpage02,         // State 30
  &CK89gps::MSGpage01,         // State 31
  //---------------------------------------------------------
  &CK89gps::CALpage01,         // State 32
  &CK89gps::NULpage01,         // State 33
  &CK89gps::SETpage01,         // State 33
};
//======================================================================================================
//  Letter field parameters
//======================================================================================================
K89_LETFD CK89gps::LetPmTAB[] = {
  //--aChar-aDisp-Left------Size-Line------Next---------Type-Iden-aFld-cPos-xFLD
  {       0,    0,        0,  0,         0,   0,  K89_NON_FD,    0,   0,   0, 0},    // 0-Not used
  {       0,    0,K89_CLN07,  4, K89_LINE0,   0,  K89_NON_FD,    1,   2,   0, 1},    // 1-APT-VOR-NDB ident
  {       0,    0,K89_CLN06, 17, K89_LINE1,   0,  K89_NON_FD,    2,   1,   0, 2},    // 2-APT VOR NDB name
  {       0,    0,K89_CLN09, 14, K89_LINE0,   0,  K89_NON_FD,    2,   0,   0, 3},    // 3-WPT name
  {       0,    0,K89_CLN10,  4, K89_LINE2,   0,  K89_NON_FD,    1,   0,   0, 4},    // 4-APT VOR NDB Direct TO
  {       0,    0,K89_CLN10, 13, K89_LINE2,   0,  K89_NON_FD,    2,   0,   0, 5},    // 5-WPT Direct to
  {       0,    0,K89_CLN15,  4, K89_LINE0,   7,  K89_LET_FD,    1,   0,   0, 6},    // 6-FROM WPT in CAL page 1
  {       0,    0,K89_CLN15,  4, K89_LINE1,   6,  K89_REP_FD,    1,   0,   0, 7},    // 7-TO   WPT in CAL page 1
  {       0,    0,K89_CLN15,  4, K89_LINE0,   9,  K89_LET_FD,    1,   0,   0, 8},    // 8-FROM WPT in CAL page 2
  {       0,    0,K89_CLN15,  4, K89_LINE1,   7,  K89_REP_FD,    1,   0,   0, 9},    // 9-TO   WPT in CAL page 2
};
//======================================================================================================
//  Repeatable field parameters
//======================================================================================================
K89_REPFD CK89gps::RepPmTAB[] = {
  //-aNber------Left-Size------Line-Next--------Type----Event-vIncr-------NbMin------NbMax-------
  {      0},                                                                      // 0 Not used
  {      0,K89_CLN16,   7,K89_LINE1,   7, K89_PAG_FD, K89_FD1,  100,K89_ALT_MIN,K89_ALT_MAX},  // 1-ALTI in VNAV
  {      0,K89_CLN06,   4,K89_LINE2,   3, K89_REP_FD, K89_FD2,    0,          0,          0},  // 2-Not used
  {      0,K89_CLN15,   4,K89_LINE2,   4, K89_REP_FD, K89_FD3,    1,K89_OFS_MIN,K89_OFS_MAX},  // 3-OFS  in VNAV
  {      0,K89_CLN10,   3,K89_LINE3,   5, K89_REP_FD, K89_FD4,    1,K89_GSP_MIN,K89_GSP_MAX},  // 4 GSP  in VNAV
  {      0,K89_CLN15,   8,K89_LINE3,   0,          0, K89_FD5,  100,K89_VSI_MIN,K89_VSI_MAX},  // 5 VSI  in VNAV
  {      0,K89_CLN07,   5,K89_LINE3,   0,          0, K89_FD4,    1,K89_GSP_MIN,K89_GSP_MAX},  // 6 GSped CAL page 1
  {      0,K89_CLN06,   3,K89_LINE1,   8, K89_REP_FD, K89_FD4,    1,K89_GSP_MIN,K89_GSP_MAX},  // 7 GSped CAL page 2
  {      0,K89_CLN09,   3,K89_LINE2,   9, K89_REP_FD, K89_FD5,    1,          0,        999},  // 8 FFlow CAL page 2
  {      0,K89_CLN17,   3,K89_LINE2,   0,          0, K89_FD6,    1,          0,        999},  // 9 FResv CAL Page 2
};
//======================================================================================================
//  Page field parameters
//======================================================================================================
K89_PAGFD CK89gps::PagPmTAB[] = {
  //----Left-Size------Line-Next------- Type -Event ---------------------------
  {0},                                              // 0  Not used
  {K89_CLN13,   1,K89_LINE0,   2, K89_PAG_FD, K89_FD1},           // 1  APT-VOR-NDB-WPT nearest number
  {K89_CLN07,   6,K89_LINE3,   0, K89_PAG_FD, K89_FD2},           // 2  APT-VOR-NDB-WPT to/from
  {K89_CLN07,  16,K89_LINE1,   2, K89_PAG_FD, K89_FD1},           // 3  NAV Page 01
  {K89_CLN06,  17,K89_LINE0,   0, K89_PAG_FD, K89_FD1},           // 4  NAV PAge 02
  {K89_CLN14,   2,K89_LINE0,   6, K89_PAG_FD, K89_FD1},           // 5  ALT Page 01 BARO Part 1
  {K89_CLN17,   2,K89_LINE0,   0, K89_PAG_FD, K89_FD2},           // 6  ALT Page 01 BARO Part 2
  {K89_CLN06,   4,K89_LINE2,   3, K89_REP_FD, K89_FD2},           // 7  NAV in VNAV page2
  {K89_CLN06,   4,K89_LINE0,   6, K89_LET_FD, K89_FD1},           // 8  WPT option CAL page 1
  {K89_CLN06,   4,K89_LINE0,   8, K89_LET_FD, K89_FD1},           // 9  WPT option CAL page 2
};
//===========================================================================================
//  GPS MODE TABLE
//===========================================================================================
char  *CK89gps::modTAB[] = 
  {"","APT","VOR","NDB","INT","USR","ACT","NAV","FPL","CAL","SET","OTH" };
//===========================================================================================
//  FPL options
//===========================================================================================
char  *CK89gps::fplTAB[] = 
  {"Dis","ETE","UTC","Dtk"};
//===========================================================================================
//  WPT Type
//===========================================================================================
char  *CK89gps::wptTYP[] = 
  {"Wpt:", "Fpl:"};
//===========================================================================================
//  Text table
//===========================================================================================
char  *CK89gps::txtTAB[] = {
    "No data",                  // 0
    "No active WPT",            // 1
    "No user data",             // 2
    "Not available",            // 3
};
//===========================================================================
// CK89gps
//===========================================================================
CK89gps::CK89gps (CVehicleObject *v,CK89gauge *g)
{ unId    = 'k89g';
  TypeIs(SUBSYSTEM_K89_GPS);
  mveh    = v;
  Gauge   = g;
  radi    = 0;
  aState  = K89_PWROF;
  Mode    = GPS_MODE_FPLAN;                      // Base mode
  powr    = 0;
  rGPS    = globals->dbc->GetGPSrequest();
  vState  = K89_VNA_OFF;
	v->RegisterGPSR(this);
  //---------Set delimiter fields -----------------------------------
  strncpy(Ident,"....",6);
  strncpy(Name,"12345678901234567",20);
  strncpy(IdFR,"....",5);
  strncpy(IdTO,"....",5);
  //---------Warning Table ------------------------------------------
  wrnTAB[0]     = "   ";
  wrnTAB[1]     = "Ent";
  wrnTAB[2]     = "M  ";
  //---------Message Table ordered by priority  ---------------------
  msgTAB[0]     = "No Message";
  msgTAB[1]     = "Vnv Alert";
  //---------Display lines ------------------------------------------
  disLN[0] = Display;
  disLN[1] = Display + (1 * K89_NBR_COLN);
  disLN[2] = Display + (2 * K89_NBR_COLN);
  disLN[3] = Display + (3 * K89_NBR_COLN);
  //--------Mode parameters -----------------------------------
  cPage   = K89_MODE_NUL;
  //--------Init clicker vector------------------------------
  SetHandler(K89_NUL_HANDLER,0,0);
  //--------Init display vector -----------------------------
  DshNO   = K89_FULL_DISPLAY;
  //--------Init CDI indicators ------------------------------
  cdiIN[0]  = '.';            // Flag
  cdiIN[1]  = 129;            // FROM
  cdiIN[2]  = 128;            // TO
  cdiST     = K89_FLAG_TO;    // Status
  cdiCL     = 0;              // Column
  //--------Init Flags ---------------------------------------
  Flag[0]   = "FLAG";
  Flag[1]   = "Fr";
  Flag[2]   = "To";
  //----------------------------------------------------------
  strip   = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  stSIZ   = (U_CHAR)strlen(strip);
  //----------------------------------------------------------
  Ident[0] = 0;
  //-------Waypoints -----------------------------------------
  nOBJ    = 0;
  pOBJ    = 0;
  //-------Init Various data----------------------------------
  Flash   = 0;                              // Flasher
  Beat    = 0;                              // Beat count
  msgNO   = 0;                              // Alarm number
  wrnNO   = 0;                              // Warning number
  NbLine  = K89_NBR_LINE;                   // Number of lines
  NbColn  = K89_NBR_COLN;                   // Number of columns
  NbChar  = K89_NBR_CHAR;                   // Number of characters
  //------Init Calculator default values ---------------------
  wptTO   = 0;
  CalFFL  = 10;                             // 10 Gal /hour
  CalFRS  = 5;                              //  5 gal reserve
  TimZON  = 0;                              // Set UTC
  //-----------Init Flasher ----------------------------------
  mskFS   = K89_ATT_ONCAR;                  // Letter mask
  //----------------------------------------------------------
}
//-----------------------------------------------------------------------------
//  Free resources
//-----------------------------------------------------------------------------
CK89gps::~CK89gps()
{ //Point[0].wpAD = 0;
  //Point[1].wpAD = 0;
}
//-----------------------------------------------------------------------------
//  Set Click Handler 
//-----------------------------------------------------------------------------
void  CK89gps::SetHandler(U_CHAR No,U_CHAR Nf,U_CHAR Tf)
{ HndNO = No;                 // Handler Number
  SnoFD = Nf;                 // Starting field number
  StyFD = Tf;                 // Starting field type
  return;
}
//-----------------------------------------------------------------------------
//  Init State parameters
//-----------------------------------------------------------------------------
void CK89gps::InitState(U_CHAR mode)
{ sBuf.aState   = 0;
  nProp         = K89_ATT_YELOW;
  rGPS->nSTACK  = 0;
  Flash         = 0;
  cPage         = mode;
  EdMOD         = K89_NOR_EDIT;
  return;
}
//=============================================================================
//  Any state:  Check to change mode 
//=============================================================================
int CK89gps::ChangeMode(K89_EVENT evn)
{ switch (evn)  {
    case K89_APT:
      InitState(K89_MODE_APT);
      return EnterAPTpage01();

    case K89_VOR:
      InitState(K89_MODE_VOR);
      return EnterVORpage01();

    case K89_NDB:
      InitState(K89_MODE_NDB);
      return EnterNDBpage01();

    case K89_INT:
      InitState(K89_MODE_INT);
      return EnterWPTpage01();

    case K89_NAV:
      InitState(K89_MODE_NAV);
      return EnterNAVpage01();

    case K89_FPL:
      InitState(K89_MODE_FPL);
      return EnterFLPpage01();

    case K89_CAL:
      InitState(K89_MODE_CAL);
      return EnterCALpage01();
        
    case K89_ALT:
      return EnterALTpage01();

    case K89_MSG:
      return EnterMSGpage01();

    case K89_OBS:
      modOBS ^= 1;
      return 1;         
    
    case K89_USR:
      return EnterNULpage01(2,K89_MODE_USR,1);

    case K89_SET:
      return EnterSETpage01();

    case K89_OTH:
      return EnterNULpage01(0,K89_MODE_OTH,1);

    case K89_POW:
      return PowerCUT();
  }
  return 1;
}
//=============================================================================
//  State 00:  Power off
//=============================================================================
int CK89gps::PowerOFF(K89_EVENT evn)
{	if ((K89_POW != evn) || (0 == powr))   return 0;
  aState  = K89_PWRNA;
  DshNO   = K89_OPEN_DISPLAY;
  Tim01   = 0;
	TrackGPSEvent(GPSR_EV_PWR,1);
  return 1;
}
//-----------------------------------------------------------------------------
//  Any state:  Power cut
//-----------------------------------------------------------------------------
int CK89gps::PowerCUT()
{ TrackGPSEvent(GPSR_EV_PWR,0);
	DshNO   = K89_FULL_DISPLAY;
  RazDisplay();
  aState  = K89_PWROF;
  rGPS->Clean();
  actWPT    = 0;
  vnaWPT    = 0;
  return 1;
}
//-----------------------------------------------------------------------------
//  Return fractional step for opening
//-----------------------------------------------------------------------------
float CK89gps::GetOpeningStep()
{ return ((float)Tim01 / K89_TIM_INTRO);
}
//-----------------------------------------------------------------------------
//  State 01:  Power on step A
//  Init Flight Plan parameters
//-----------------------------------------------------------------------------
int CK89gps::PowerONa(K89_EVENT evn)
{ 
  if (K89_POW   == evn)         return PowerCUT();
  if (K89_CLOCK != evn)         return 0;
  //-------Let Time to Draw the moving amber rectangles ---------
  if (Tim01++ < K89_TIM_INTRO)  return 1;
  DshNO  = K89_FULL_DISPLAY;
  //------Draw the intro text -----------------------------------
  Tim01 = 0;
  mskFS = K89_ATT_ONCAR;
  RazDisplay();
  Prop   = K89_ATT_NONE;
  StoreText("BENDIX/KING",K89_LINE1,K89_CLN06);
  StoreText("KLN 89",     K89_LINE2,K89_CLN09);
  StoreText("GPS",        K89_LINE3,K89_CLN11);
  aState = K89_PWRNB;
  //-----Get GPS request from database cache --------------------
  nProp = K89_ATT_YELOW;
  rGPS->nSTACK  = 0;
  rGPS->SetReqCode(GPS_NR_AIRPORT);
  return 1;
}
//-----------------------------------------------------------------------------
//  State 02:  Power on step B
//-----------------------------------------------------------------------------
int CK89gps::PowerONb(K89_EVENT evn)
{ if (K89_POW  == evn)    return PowerCUT();
  if (K89_CLOCK != evn)   return 0;
  Tim01++;
  if (Tim01 != 200)       return 0;
  aState = K89_PWRNC;
  return 1;
}
//-----------------------------------------------------------------------------
//  State 03:  Power on step C:  Diagnostic
//-----------------------------------------------------------------------------
int CK89gps::PowerONc(K89_EVENT evn)
{ if (K89_POW  == evn)    return PowerCUT();
  if (K89_CLOCK != evn)   return 0;
  RazDisplay();
  StoreText("KLN 89",K89_LINE0,K89_CLN01);
  StoreText("GPS",   K89_LINE0,K89_CLN11);
  StoreText("OSR 02",K89_LINE0,K89_CLN17);
  //----------------------------------------------------
  StoreText("@1995 AlliedSignal",K89_LINE2,K89_CLN03);
  StoreText("Avionic, Inc."     ,K89_LINE3,K89_CLN06);
  aState  = K89_PWRND;
  Tim01   = 300;
  return 0;
}
//-----------------------------------------------------------------------------
//  State 04:  Power on step d:  Diagnostic flash
//-----------------------------------------------------------------------------
int CK89gps::PowerONd(K89_EVENT evn)
{ char edt[16];
  if (K89_POW  == evn)    return PowerCUT();
  if (K89_CLOCK != evn)   return 0;
  FlashWord("Self-test in progress",K89_LINE1,K89_CLN02);
  if (Tim01--)            return 0;
  RazDisplay();
  StoreText("34.5 nm",K89_LINE0,K89_CLN00);
  cdiDEV  = -12.5f;
  DrawCDI(K89_LINE0,K89_CLN12);
  BaroVL = globals->atm->GetPressureInHG();      
  _snprintf(edt,16,"Baro:%.02f\"",BaroVL);
	edt[15] = 0;
  StoreText(edt,        K89_LINE1,K89_CLN00);
  StoreText("Alt",      K89_LINE1,K89_CLN12);
  StoreText("RMI 130°", K89_LINE2,K89_CLN00);
  StoreText("OBS",      K89_LINE2,K89_CLN12);
  StoreText("ANNUN ON", K89_LINE3,K89_CLN00);
  StoreText("PASS",     K89_LINE3,K89_CLN12);
  aState = K89_PWRNG;
  return 1;
}
//-----------------------------------------------------------------------------
//  State 06:  Power on step g:  Test screen flashing
//-----------------------------------------------------------------------------
int CK89gps::PowerONg(K89_EVENT evn)
{ char edt[16];
  int  obs = 0;
  switch (evn)  {
    case K89_POW:
      return PowerCUT();

    case K89_CLOCK:
      //------------ALTITUDE ------------------------
      _snprintf(edt,16,"%.0fft",aPos.alt);
			edt[15] = 0;
      StoreText(edt,K89_LINE1,K89_CLN17);
      //--------------OBS from navigation radio ------
      obs   = (BUS)?(int(BUS->hREF)):(0);
      _snprintf(edt,16,"%03u°",obs);
			edt[15] = 0;
      StoreText(edt,K89_LINE2,K89_CLN17);
      FlashWord("Ok ?",K89_LINE3,K89_CLN17);
      return 1;

    case K89_ENT:
      RazDisplay();
      StoreText("WPT:",K89_LINE1,K89_CLN00);
      StoreText("Ref", K89_LINE1,K89_CLN14);
      FlashWord("Ok?",K89_LINE3,K89_CLN14);
      aState  = K89_PWRNH;
      return 1;
  }
  return 0;
}
//-----------------------------------------------------------------------------
//  State 07:  Power on step h:  Initialization
//  NOTE:   The request code set at PowerONa should now be complete .
//          Thus the nearest airport should be ready if any.
//-----------------------------------------------------------------------------
int CK89gps::PowerONh(K89_EVENT evn)
{ char edt[16];
  char mon[ 4];
  SDateTime sd;
  char *month;
  switch (evn)  {
    //------------POWER OFF -------------------------------
    case K89_POW:
      return PowerCUT();
    //------------CLOCK event -----------------------------
    case K89_CLOCK:
      //---------DATE -----------------------------------
      sd = globals->tim->GetUTCDateTime();
      month = globals->monTAB[sd.date.month];
      strncpy(mon,month,3);
      mon[3]  = 0;
      _snprintf(edt,16,"%02u %s %02u",sd.date.day,mon,(sd.date.year - 100));
			edt[15] = 0;
      StoreText(edt,K89_LINE0,K89_CLN00);
      //-------TIME --------------------------------------
      _snprintf(edt,16,"%02u:%02u UTC",sd.time.hour,sd.time.minute);
      StoreText(edt,K89_LINE0,K89_CLN14);
      //------Edit the position --------------------------
      EditLatitude (aPos.lat,K89_LINE2,K89_CLN00);
      EditLongitude(aPos.lon,K89_LINE3,K89_CLN00);
      //------Edit the airport data ---------------------
      EditAPTdata();
      return 1;

    case K89_ENT:
      RazDisplay();
      StoreText("WORLD WIDE  Database",   K89_LINE0,K89_CLN02);
      StoreText("Never expires",          K89_LINE1,K89_CLN06);
      StoreText("GPS approaches disabled",K89_LINE2,K89_CLN00);
      FlashWord("Acknowledge?",K89_LINE3,K89_CLN06);
      aState  = K89_PWRNJ;
      return 1;
  }
  return 0;
}
//-----------------------------------------------------------------------------
//  Edit airport IDENT, radial and distance

//-----------------------------------------------------------------------------
void CK89gps::EditAPTdata()
{ char edt[16];
  CAirport *apt = (CAirport*)rGPS->obj.Pointer();
  if (0 == apt)               return;
  strncpy(Ident,apt->GetFaica(),K89_IDSIZ);
  Ident[K89_IDSIZ]  = 0;
  StoreText(Ident,K89_LINE1,K89_CLN18);
  apt->Refresh(FrameNo);
  float rad = apt->GetRadial();
  EditRadial(edt,rad,K89_FLAG_TO);
  StoreText(edt,K89_LINE2,K89_CLN17);
  EditDistance(edt,apt->GetNmiles());
  StoreText(edt,K89_LINE3,K89_CLN17);
  return;
}
//-----------------------------------------------------------------------------
//  State 08:  Database update screen
//-----------------------------------------------------------------------------
int CK89gps::PowerONj(K89_EVENT evn)
{ switch (evn)  {
  case K89_POW:
    return PowerCUT();
    
  case K89_ENT:
    RazDisplay();
    StoreText("TOTAL FOB",K89_LINE0,K89_CLN08);
    EditFuel();
    FlashWord("Ok ?",K89_LINE3,K89_CLN13);
    aState  = K89_PWRNK;
  }
  return 1;
}
//-----------------------------------------------------------------------------
//  State 09:  Fuel load
//-----------------------------------------------------------------------------
int CK89gps::PowerONk(K89_EVENT evn)
{ 
  switch (evn)  {

    case K89_POW:
      return PowerCUT();
    
    case K89_ENT:
      //---Enter airport page 1 with the nearest Airport --------
      modOBS  = 0;
      RazDisplay();
      DshNO = K89_PART_DISPLAY;
      StoreText("Leg",   K89_LINE2,K89_CLN03);
			pMod		= 0;
      vnaWNO  = -1;
			ModifiedPlan();
      InitVnavWPT();
      RefreshActiveWPT();
      rGPS->obj   = 0;
      return  ChangeMode(K89_APT);
  }
  return 1;
}
//=============================================================================
//  State 10:  AIRPORT MODE PAGE 1
//  NOTE: In Nearest mode, when the cursor if left over the number 1,
//        then the page is refreshed with the nearest airport in real time
//=============================================================================
int CK89gps::APTpage01(K89_EVENT evn)
{ U_CHAR old = curPOS;
  switch (evn)  {
  //-------End Of Request Edit in current format-----------------
  case K89_EOR:
      return (this->*eorEV)();           
  //------CURSOR Even: Edit name and ident except in nearest mode-
  case K89_CSR:
      curPOS = (curPOS)?(0):(3);
      if (K89_NOR_EDIT != EdMOD)  return 0;
      EnterCSRMode01(&CK89gps::EnterAPTpage01);
      return 0;
  //------PAGE:  ENTER PAGE 2 ------------------------------------
  case K89_PAG:
      if (msDIR == +1)            EnterAPTpage02();
      return 0;
  //-----CLR-  SWAP Radial/bearing mode --------------------------
  case K89_CLR:
      if (3 == curPOS)  OpFlag ^= K89_FLAG_MASK;
      return 1;
  //-----DIR: Enter DIRECT TO mode --------------------------------
  case K89_DIR:
			rGPS->StopReq();						// Cancel any request
      if (K89_ACT_EDIT == EdMOD)  return 1;
      return EnterDIRpage01(cOBJ);
  //-----CLOCK event  -------------------------------------------
  case  K89_CLOCK:
      RefreshNRSairport();
      if (K89_NRS_EDIT == EdMOD)  EditOBJpart2(cOBJ);
      return 1;
  //-----Change airport number -----------------------------------
  case K89_FD1:
      if (K89_NRS_EDIT != EdMOD)  return 1;
      curPOS  = 1;
      if (1 == old)       ChangeNRSwpt();
      return EnterAPTpage01();
  //-----Change FLAG TO/FROM -------------------------------------
  case K89_FD2:
      if (K89_NRS_EDIT != EdMOD)  return 1;
      curPOS  = 3;
      OpFlag ^= K89_FLAG_MASK;
      return 1;
  //------NRS ---Nearest mode ------------------------------------
  case K89_NRS:
      return EnterNRSpage01();
  //------ACT event: Enter Active waypoint -----------------------
  case K89_ACT:
      return EnterACTmode(actWPT);
  //------Other modes --------------------------------------------
  default:
    ChangeMode(evn);
}
  return 1;
}
//-----------------------------------------------------------------------------
//  Refresh nearest airport if conditions are met
//-----------------------------------------------------------------------------
void CK89gps::RefreshNRSairport()
{ if (K89_NRS_EDIT != EdMOD)  return;
  if (curPOS != 1)            return;
  if (rGPS->wptNo != 1)       return;
  rGPS->obj = 0;
  rGPS->GetAPT();
  return;
}
//-----------------------------------------------------------------------------
//  Edit Object (APT, VOR or NDB) Part 2 in nearest mode
//-----------------------------------------------------------------------------
int CK89gps::EditOBJpart2(CmHead *obj)
{ char nx = rGPS->wptNo + K89_0_INDEX;
  char No = strip[nx];
  FlashChar(No, K89_LINE0,K89_CLN13,1);
  if (0 == obj)     return 1;
  FlashChar('>',K89_LINE3,K89_CLN06,3);
  //----Edit bearing and distance ------------------------------
  return EditCAPandDIS(obj);
}
//-----------------------------------------------------------------------------
//  Edit Airport country and ownership
//-----------------------------------------------------------------------------
int CK89gps::EditAPTctry(short lin)
{ char  edt[32];
  char  wrk[32];
  //----Edit COUNTRY -----------------------------------------
  char *loc = cOBJ->GetCountry();
  strncpy(edt,loc,4);
  edt[2]  = 0;
  StoreText(edt,lin,K89_CLN06);
  //----------------------------------------------------------
  int own = ((CAirport*)cOBJ)->GetOwner();
  if (own > 4) own = 0;
  strncpy(wrk,globals->ownTAB[own],K89_OWSIZ);
  wrk[K89_OWSIZ-1] = 0;
  _snprintf(edt,32,"%-10s",wrk);
  StoreText(edt,lin,K89_CLN12);
  return 1;
}
//--------------------------------------------------------------------
//  Edit Airport header
//--------------------------------------------------------------------
void CK89gps::EditAPTheader()
{ char edt[16];
  short cln = (K89_ACT_EDIT != EdMOD)?(K89_CLN07):(K89_CLN11);
  _snprintf(edt,16,"%2d",(curNO + 1));
  //-----Edit IDENT with 4 characters (padding with spaces) --
  FormatIdent(cOBJ,K89_LINE0,cln);
  //---Edit flight plan wpt number --------------------------
  if ((K89_ACT_EDIT == EdMOD) && (curNO != -1))
  { Prop = K89_ATT_YELOW;
    StoreText(edt,K89_LINE0,K89_CLN08);
  }
  _snprintf(edt,16,"% 6uft",int(cOBJ->GetElevation()));
  StoreText(edt,K89_LINE0,K89_CLN15);
  //----Edit name with padding spaces -----------------------
  FormatLabel (cOBJ,K89_LINE1,K89_CLN06,Name);
  return;
}
//--------------------------------------------------------------------
//  Edit Airport Page 01
//  -Assign cOBJ with the airport object
//  -Assign wptTO for calculator
//--------------------------------------------------------------------
int CK89gps::EditAPTpage01()
{ cOBJ  = rGPS->obj.Pointer();
  wptTO = cOBJ;
  if (0 == cOBJ)              return ClearField(LetFD);
  EditAPTheader();
  if (K89_NRS_EDIT == EdMOD)  return EditOBJpart2(cOBJ);
  //----Edit COUNTRY -----------------------------------------
  return EditAPTctry(K89_LINE3);
}
//------------------------------------------------------------------
//  Enter AIRPORT PAGE 01
//------------------------------------------------------------------
int CK89gps::EnterAPTpage01()
{ ClearRightDisplay();
  PageHeader(1);
  rCode   = GPS_GT_AIRPORT;
  rGPS->GetAPT();											// Arm request
  eorEV   = &CK89gps::EditAPTpage01;	// EOR routine
  SetHandler(K89_GEN_HANDLER,K89_FIELD01,K89_PAG_FD);
  LetFD   = Gauge->GetLetField(1);
  curPOS  = 1;
  aState  = K89_APTP1;
  return 1;
}
//------------------------------------------------------------------
//  Enter cursor Mode 01 with the caller return function
//  -Set Ident flasher
//  -Cursor mode
//------------------------------------------------------------------
int CK89gps::EnterCSRMode01(PreFN cal)
{ preSF = cal;
  FlashWord("*CRSR*",K89_LINE3,K89_CLN00);
  FlashON(LetFD->aDisp);
  SetHandler(K89_LET_HANDLER,0,0);
  aState  = K89_CSRM1;         // Active state
  return 1;
}
//=============================================================================
//  State 11:  CURSOR MODE 1
//  -CSR=> Return to previous state (vector preSF)
//  -EOR=> End of request           (vector eorEV)
//=============================================================================
int CK89gps::CSRMode01(K89_EVENT evn)
{ 
  switch (evn)  {
    case K89_POW:
        return PowerCUT();
    //------CURSOR  -- return to previous page (APT,VOR or NDB)--
    case K89_CSR:
        return (this->*preSF)();
    //------PAGE ---Enter mode SCAN -----------------------
    case K89_PAG:
        EnterCSRMode02();
        return 1;
    //------EOR event: End of Request completion ----------
    case K89_EOR:
        (this->*eorEV)();				// Call Routine
        Flsiz = 1;
        ReFlash();
        return 1;
    //-----Check for clic on identifier -------------------
    case K89_DPY:
        if (CheckNewLetter()) GetNextMatch(LetFD,1,+1);
        break;
    //-----See other modes --------------------------------
    default:
      ChangeMode(evn);
  }
  return 1;
}
//------------------------------------------------------------------
//  Check for new letter on current field
//------------------------------------------------------------------
int CK89gps::CheckNewLetter()
{ U_SHORT *pos = LetFD->aDisp   + LetFD->cPos;
  U_SHORT *old = Flash;
  FlashOF(old);
  FlashON(pos);
  if (old != pos)       return 0; 
  //-----Change current letter  ----------
  U_CHAR  let   = GetStripIndex(*pos);
  SetNewLetter(LetFD,let);
  return 1;
}
//------------------------------------------------------------------
//  Clear the current field and alternate
//  Reedit in screen
//------------------------------------------------------------------
int CK89gps::ClearField(K89_LETFD *fd)
{ char    *txt = fd->aChar + fd->cPos + 1;
  while (*txt)  *txt++ = '_';
  StoreText(fd->aDisp,fd->aChar);
  if (0 == fd->aFld)  return 1;
  K89_LETFD *f2 = Gauge->GetLetField(fd->aFld);
  txt = f2->aChar;
  while (*txt) *txt++ = '_';
  StoreText(f2->aDisp,f2->aChar);
  return 1;
}
//------------------------------------------------------------------
//  Get a new letter according to click button
//------------------------------------------------------------------
void  CK89gps::SetNewLetter(K89_LETFD *fd,U_CHAR xl)
{ char    *tex = fd->aChar  + fd->cPos;
  U_SHORT *pos = fd->aDisp    + fd->cPos;
  xl += msDIR;
  if (xl == stSIZ) xl = 0;
  if (xl ==   255) xl = (stSIZ - 1);
  *tex  = strip[xl];
  //----Clear rest of field ----------------
  tex += 1;
  while (*tex)  *tex++ = '_';
  //----Copy to display --------------------
  Prop  = K89_ATT_YELOW;
  StoreText(fd->aDisp,fd->aChar);
  FlashON(pos);
  //----Clear alternate field ---------------
  if (0 == fd->aFld)  return;
  K89_LETFD *f2 = Gauge->GetLetField(fd->aFld);
  char *txt =f2->aChar;
  while (*txt) *txt++ = '_';
  Prop = K89_ATT_YELOW;
  StoreText(f2->aDisp,f2->aChar);
  return;
}
//------------------------------------------------------------------
//  Enter cursor Mode 2
//  Set wide cursor from current cursor position
//------------------------------------------------------------------
int CK89gps::EnterCSRMode02()
{ char *txt = LetFD->aChar + LetFD->cPos + 1;
  while (*txt) *txt++ = '_';
  //----store in display -----------------------------
  FlashText(LetFD->aChar,LetFD->aDisp);
  SetHandler(K89_NUL_HANDLER,0,0);
  aState  = K89_CSRM2;
  return 1;
}
//=============================================================================
//  State 12:  CURSOR MODE SCAN
//=============================================================================
int CK89gps::CSRMode02(K89_EVENT evn)
{ switch  (evn) {
    case K89_POW:
      return PowerCUT();
    //------CURSOR  -- return to previous page (APT,VOR or NDB)---
    case K89_CSR:
      FlashOFF();
      return (this->*preSF)();

    //------PAGE event: Scan database -----------------------
    case K89_PAG:
      if (msDIR == +1) GetNextMatch(LetFD,0,+1);
      if (msDIR == -1) GetNextMatch(LetFD,0,-1);
      break;
    //------EOR event: Request completion -------------------
    case K89_EOR:
      (this->*eorEV)();			// Call routine
      ReFlash();
      break;
    //--------See for other modes ---------------------------
    default:
      ChangeMode(evn);
  }
  return 1;
}
//=============================================================================
//  State 13:  AIRPORT PAGE2
//=============================================================================
int CK89gps::APTpage02(K89_EVENT evn)
{ U_CHAR old = curPOS;
  switch(evn) {
    //--------DRAW event: Refresh position and distance ----------
    case K89_CLOCK:
        return EditAPTpage02();
    //--------CSR event:  Alternate flash cursor --------------------
    case K89_CSR:
        curPOS = (curPOS)?(0):(3);
        return 1;
    //--------CLR event:  Swap BEARING/RADIAL -----------------------
    case K89_CLR:
        if (3 == curPOS) OpFlag ^= K89_FLAG_MASK;
        return 1;
    //--------PAGE event: change page -------------------------------
    case K89_PAG:
        if (msDIR == +1)  return EnterAPTpage03(); 
        if (msDIR == -1)  return EnterAPTpage01();
        return 0;
    //-----Change airport number when in NRS mode -------------------
    case K89_FD1:
        if (K89_NRS_EDIT != EdMOD)  return 1;  
        curPOS  = 1;
        if (1 == old)     ChangeNRSwpt();
        return EnterAPTpage01();
    //-----Change FLAG TO/FROM -------------------------------------
    case K89_FD2:
        curPOS  = 3;
        OpFlag ^= K89_FLAG_MASK;
        return 1;
    //-----DIR Enter DIRECT TO mode --------------------------------
    case K89_DIR:
        return EnterDIRpage01(cOBJ);
    //------NRS ---Nearest mode ------------------------------------
    case K89_NRS:
        return EnterNRSpage01();

    //------ACT event: Enter Active waypoint -----------------------
    case K89_ACT:
        return EnterACTmode(actWPT);
    //------Other events -------------------------------------------
    default:
        ChangeMode(evn);

  }
  return 1;
}

//-----------------------------------------------------------------------------
//  Enter Airport Page 02
//  -Set state
//  -Set click helper
//  -Set bearing option
//-----------------------------------------------------------------------------
int CK89gps::EnterAPTpage02()
{ if (0 == cOBJ)   return 1;
  ClearRightDisplay();
  PageHeader(2);
  FormatIdent(cOBJ,K89_LINE0,K89_CLN07);
  aState  = K89_APTP2;
  SetHandler(K89_GEN_HANDLER,K89_FIELD01,K89_PAG_FD);
  curPOS  = 1;
  OpFlag  = K89_FLAG_TO;
  return 1;
}
//-----------------------------------------------------------------------------
//  Edit Airport part 3
//-----------------------------------------------------------------------------
int CK89gps::EditAPTpart3()
{ char nx = rGPS->wptNo + K89_0_INDEX;
  char No = strip[nx];
  FlashChar(No,K89_LINE0,K89_CLN13,1);
  //----Edit name with padding spaces -----------------------
  FormatLabel(cOBJ,K89_LINE1,K89_CLN06,Name);
  //----Edit COUNTRY ----------------------------------------
  return EditAPTctry(K89_LINE2);
}
//-----------------------------------------------------------------------------
//  Edit Airport page 2
//-----------------------------------------------------------------------------
int CK89gps::EditAPTpage02()
{ if (0 == cOBJ)       return 1;
  FlashChar('>',K89_LINE3,K89_CLN06,3);
  EditCAPandDIS(cOBJ);
  if (K89_NRS_EDIT == EdMOD)  return EditAPTpart3();
  //------Edit latitude and longitude in line 1 & 2 ------------   
  EditLatitude (cOBJ->GetLatitude(), K89_LINE1,K89_CLN10);
  EditLongitude(cOBJ->GetLongitude(),K89_LINE2,K89_CLN10);
  return 1;
}
//=============================================================================
//  State 14:  AIRPORT PAGE3
//=============================================================================
int CK89gps::APTpage03(K89_EVENT evn)
{ switch (evn)  {
  //--------PAGE event: change page -------------------------------
  case K89_PAG:
      if (msDIR == -1)  return EnterAPTpage02();
      if (msDIR == +1)  return EnterAPTpage04(); 
      return 1;
  //-----DIR Enter DIRECT TO mode --------------------------------
  case K89_DIR:
      return EnterDIRpage01(cOBJ);
  //------NRS ---Nearest mode ------------------------------------
  case K89_NRS:
      return EnterNRSpage01();
  //------ACT event: Enter Active waypoint -----------------------
  case K89_ACT:
      return EnterACTmode(actWPT);
  //------Other events -------------------------------------------
  default:
      ChangeMode(evn);
  }
  return 1;
}
//-----------------------------------------------------------------------------
//  Enter Airport Page 3
//-----------------------------------------------------------------------------
int CK89gps::EnterAPTpage03()
{ char edt[24];
  CAirport *apt = (CAirport*)cOBJ;
  ClearRightDisplay();
  PageHeader(3);
  aState  = K89_APTP3;
  SetHandler(K89_NUL_HANDLER,0,0);
  FormatIdent(cOBJ,K89_LINE0,K89_CLN07);
  if (0 == cOBJ)   return 1;
  //--------Edit time zone -----------------------------------------
  EditTimeZone(edt,cOBJ->GetLongitude());
  StoreText(edt,K89_LINE1,K89_CLN06);
  //--------Edit Fuel grad -----------------------------------------
  int grad = apt->GetFuelGrad();
  StoreText(globals->gazTAB[grad],K89_LINE2,K89_CLN06);
  //-------Edit facilities -----------------------------------------
  const char *ils = (apt->GetILSfacility())?("ILS"):("");
  StoreText(ils,K89_LINE3,K89_CLN06);
  return 1;
}
//=============================================================================
//  State 15:  AIRPORT PAGE4: Runway pages
//=============================================================================
int CK89gps::APTpage04(K89_EVENT evn)
{ switch (evn)  {
      //--------PAGE event: change page -------------------------------
      case K89_PAG:
          if (msDIR == -1)                  return EditBkRunway();
          if (msDIR == +1)                  return EditFwRunway(); 
          return 1;
      //--------EOR event: Request completion -------------------------
      case K89_EOR:
          (this->*eorEV)();                 // Post end of request
          return 1;
      //------NRS ---Nearest mode ------------------------------------
      case K89_NRS:
          return EnterNRSpage01();
      //-----DIR Enter DIRECT TO mode --------------------------------
      case K89_DIR:
          return EnterDIRpage01(cOBJ);
      //------ACT event: Enter Active waypoint -----------------------
      case K89_ACT:
          return EnterACTmode(actWPT);
      //------Other events -------------------------------------------
      default:
        ChangeMode(evn);
  }

  return 1;
}
//--------------------------------------------------------------------
//  Start in forward mode
//--------------------------------------------------------------------
int CK89gps::StartAPTp4ForW()
{ if (rGPS->rwyQ.GetNbrObj() == 0)  return EditNoRunway();
  pOBJ  = 0;
  nOBJ  = rGPS->rwyQ.GetFirst();
  EditFwRunway();
  return 1;
}
//--------------------------------------------------------------------
//  Start in Backward mode
//--------------------------------------------------------------------
int CK89gps::StartAPTp4Back()
{ nOBJ  = 0;
  pOBJ  = rGPS->rwyQ.GetLast();
  EditBkRunway();
  return 1;
}

//--------------------------------------------------------------------
//  Enter AIRPORT PAGE 4 in forward mode
//--------------------------------------------------------------------
int CK89gps::EnterAPTpage04()
{ nOBJ    = 0;
  pOBJ    = 0;
  SetHandler(K89_NUL_HANDLER,0,0);
	rGPS->GetRWY();													// Arm database request			
  eorEV   = &CK89gps::StartAPTp4ForW;			// EOR routine
  aState  = K89_APTP4;
  return  1;
}
//--------------------------------------------------------------------
//  Enter AIRPORT PAGE 4 in backward mode
//--------------------------------------------------------------------
int CK89gps::BackAPTpage04()
{ SetHandler(K89_NUL_HANDLER,0,0);
	rGPS->GetRWY();													// Arm database request
  eorEV   = &CK89gps::StartAPTp4Back;			// EOR routine
  aState  = K89_APTP4;
  return  1;
}
//---------------------------------------------------------------------
//  Edit in forward mode
//  Use dLon as line number to remember for backward browse
//---------------------------------------------------------------------
int CK89gps::EditFwRunway()
{ int nl = 0;
  if (0 == nOBJ)    return EnterAPTpage05();
  ClearRightDisplay();
  PageHeader(4);
  FormatIdent(cOBJ,K89_LINE0,K89_CLN07);
  pOBJ  = nOBJ->PrevInQ1();
  while (nOBJ && (nl != 4))
  { EditRunwaySlot(nl,(CRunway*)nOBJ);
    nOBJ->SetDistLon(nl);
    nl  += 2;
    nOBJ = nOBJ->NextInQ1();
  }
  if (nOBJ || pOBJ) StoreChar('+',K89_LINE3,K89_CLN03);
  return 1;
}
//---------------------------------------------------------------------
//  Edit in backward mode
//  Set line according to runway Indice (No)
//---------------------------------------------------------------------
int CK89gps::EditBkRunway()
{ if (0 == pOBJ)     return EnterAPTpage03();
  ClearRightDisplay();
  PageHeader(4);
  FormatIdent(cOBJ,K89_LINE0,K89_CLN07);
  short ln = pOBJ->GetDistLon();
  nOBJ  = pOBJ->NextInQ1();
  while (pOBJ && (ln != -2))
  { EditRunwaySlot(ln,(CRunway*)pOBJ);
    ln -= 2;
    pOBJ = pOBJ->PrevInQ1();
  }
  if (nOBJ || pOBJ) StoreChar('+',K89_LINE3,K89_CLN03);
  return 1;
}
//---------------------------------------------------------------------
//  Edit Runway SLOT
//  TODO Lighting mode
//---------------------------------------------------------------------
void CK89gps::EditRunwaySlot(short ln,CRunway *rwy)
{ char edt[16];
  short l2  = ln + 1;
  int   sf  = rwy->GroundIndex();
  int   lgh = ((CAirport*)cOBJ)->GetLighting();
  if (0 == rwy)     return; 
  _snprintf(edt,16,"%s/%s", rwy->GetHiEnd(),rwy->GetLoEnd());
  StoreText(edt,ln,K89_CLN15);
  _snprintf(edt,16,"%uft",rwy->GetLenghi());
  StoreText(edt,l2,K89_CLN07);
  StoreText(globals->rwgTAB[sf],l2,K89_CLN15);
  return;
}
//---------------------------------------------------------------------
//  Edit No Runway Data Page
//---------------------------------------------------------------------
int CK89gps::EditNoRunway()
{ ClearRightDisplay();
  PageHeader(4);
  FormatIdent(cOBJ,K89_LINE0,K89_CLN07);
  StoreText("No Runway Data",K89_LINE1,K89_CLN07);
  return 1;
}
//=============================================================================
//  State 16:  AIRPORT PAGE 5: COM pages
//=============================================================================
int CK89gps::APTpage05(K89_EVENT evn)
{ switch (evn)  {
      //--------PAGE event: change page -------------------------------
      case K89_PAG:
          if (msDIR == -1)                  return EditBkCom();
          if (msDIR == +1)                  return EditFwCom(); 
          return 1;
      //--------EOR event: Request completion --------------------------
      case K89_EOR:
          (this->*eorEV)();                 // Post end of request
          return 1;
      //------NRS ---Nearest mode ------------------------------------
      case K89_NRS:
          return EnterNRSpage01();
      //-----DIR Enter DIRECT TO mode --------------------------------
      case K89_DIR:
          return EnterDIRpage01(cOBJ);
      //------ACT event: Enter Active waypoint -----------------------
      case K89_ACT:
          return EnterACTmode(actWPT);
      //------Other events -------------------------------------------
      default:
          ChangeMode(evn);
  }
  return 1;
}
//---------------------------------------------------------------------
//  Edit in COM forward mode
//---------------------------------------------------------------------
int CK89gps::EditFwCom()
{ int nl = 1;
  if (0 == nOBJ)    return EnterAPTpage09();
  ClearRightDisplay();
  PageHeader(5);
  FormatIdent(cOBJ,K89_LINE0,K89_CLN07);
  pOBJ  = nOBJ->PrevInQ1();
  while (nOBJ && (nl != 4))
  { EditComSlot(nl,(CCOM*)nOBJ);
    nOBJ->SetDistLon(nl);
    nl  += 1;
    nOBJ = nOBJ->NextInQ1();
  }
  if (nOBJ || pOBJ) StoreChar('+',K89_LINE3,K89_CLN03);
  return 1;
}
//---------------------------------------------------------------------
//  Edit in backward mode
//  Set line according to COM Indice (No)
//---------------------------------------------------------------------
int CK89gps::EditBkCom()
{ if (0 == pOBJ)     return BackAPTpage04();
  ClearRightDisplay();
  PageHeader(5);
  FormatIdent(cOBJ,K89_LINE0,K89_CLN07);
  short ln = pOBJ->GetDistLon();
  nOBJ  = pOBJ->NextInQ1();
  while (pOBJ && (ln != 0))
  { EditComSlot(ln,(CCOM*)pOBJ);
    ln -= 1;
    pOBJ = pOBJ->PrevInQ1();
  }
  if (nOBJ || pOBJ) StoreChar('+',K89_LINE3,K89_CLN03);
  return 1;
}
//---------------------------------------------------------------------
//  Edit No COM Data Page
//---------------------------------------------------------------------
int CK89gps::EditNoCom()
{ ClearRightDisplay();
  PageHeader(5);
  FormatIdent(cOBJ,K89_LINE0,K89_CLN07);
  StoreText ("No Comm Data", K89_LINE1,K89_CLN07);
  return 1;
}
//---------------------------------------------------------------------
//  Edit COMM SLOT
//---------------------------------------------------------------------
void CK89gps::EditComSlot(short ln,CCOM *com)
{ char edt[16];
  U_SHORT inx = com->GetComIndex();
  char *name  = globals->cmaTAB[inx];
  float freq  = com->GetFrequency();
  _snprintf(edt,16,"%s   %.2f",name,freq);
  StoreText(edt,ln,K89_CLN06);
  return;
}
//--------------------------------------------------------------------
//  Start in forward mode
//--------------------------------------------------------------------
int CK89gps::StartAPTp5ForW()
{ if (rGPS->comQ.GetNbrObj() == 0)  return EditNoCom();
  pOBJ  = 0;
  nOBJ  = rGPS->comQ.GetFirst();
  EditFwCom();
  return 1;
}
//--------------------------------------------------------------------
//  Start in Backward mode
//--------------------------------------------------------------------
int CK89gps::StartAPTp5Back()
{ nOBJ  = 0;
  pOBJ  = rGPS->comQ.GetLast();
  EditBkCom();
  return 1;
}
//--------------------------------------------------------------------
//  Enter AIRPORT PAGE 5 in forward mode
//--------------------------------------------------------------------
int CK89gps::EnterAPTpage05()
{ nOBJ    = 0;
  pOBJ    = 0;
  SetHandler(K89_NUL_HANDLER,0,0);
  rGPS->GetCOM();												// Arm database request
  eorEV   = &CK89gps::StartAPTp5ForW;		// EOR routine
  aState  = K89_APTP5;
  return  1;
}
//--------------------------------------------------------------------
//  Enter AIRPORT PAGE 5 in backward mode
//--------------------------------------------------------------------
int CK89gps::BackAPTpage05()
{ SetHandler(K89_NUL_HANDLER,0,0);
	rGPS->GetCOM();												// Arm database request
  eorEV   = &CK89gps::StartAPTp5Back;		// EOR routine
  aState  = K89_APTP5;
  return  1;
}
//=============================================================================
//  State 17:  AIRPORT PAGE 9: ILS pages
//=============================================================================
int CK89gps::APTpage09(K89_EVENT evn)
{ switch (evn)  {
      //--------PAGE event: change page -------------------------------
      case K89_PAG:
          if (msDIR == -1)                  return EditBkILS();
          if (msDIR == +1)                  return EditFwILS(); 
          return 1;
      //--------EOR event: Request completion -------------------------
      case K89_CLOCK:
          (this->*eorEV)();                 // Post end of request
          return 1;
      //------NRS ---Nearest mode ------------------------------------
      case K89_NRS:
          return EnterNRSpage01();
      //-----DIR Enter DIRECT TO mode --------------------------------
      case K89_DIR:
          return EnterDIRpage01(cOBJ);
      //------ACT event: Enter Active waypoint -----------------------
      case K89_ACT:
          return EnterACTmode(actWPT);
      //------Other events -------------------------------------------
      default:
        ChangeMode(evn);

  }
  return 1;
}
//---------------------------------------------------------------------
//  Edit No ILS Data Page
//---------------------------------------------------------------------
int CK89gps::EditNoILS()
{ ClearRightDisplay();
  PageHeader(9);
  FormatIdent(cOBJ,K89_LINE0,K89_CLN07);
  StoreText ("No ILS Data",K89_LINE1,K89_CLN07);
  return 1;
}
//---------------------------------------------------------------------
//  Edit in ILS forward mode
//---------------------------------------------------------------------
int CK89gps::EditFwILS()
{ int nl = 1;
  if (0 == nOBJ)    return 1;
  ClearRightDisplay();
  PageHeader(9);
  FormatIdent(cOBJ,K89_LINE0,K89_CLN07);
  pOBJ  = nOBJ->PrevInQ1();
  while (nOBJ && (nl != 4))
  { EditIlsSlot(nl,(CILS*)nOBJ);
    nOBJ->SetDistLon(nl);
    nl  += 1;
    nOBJ = nOBJ->NextInQ1();
  }
  if (nOBJ || pOBJ) StoreChar('+',K89_LINE3,K89_CLN03);
  return 1;
}
//---------------------------------------------------------------------
//  Edit in backward mode
//  Set line according to ILS Indice (No)
//---------------------------------------------------------------------
int CK89gps::EditBkILS()
{ if (0 == pOBJ)     return BackAPTpage05();
  ClearRightDisplay();
  PageHeader(9);
  FormatIdent(cOBJ,K89_LINE0,K89_CLN07);
  short ln = pOBJ->GetDistLon();
  nOBJ  = pOBJ->NextInQ1();
  while (pOBJ && (ln != 0))
  { EditIlsSlot(ln,(CILS*)pOBJ);
    ln -= 1;
    pOBJ = pOBJ->PrevInQ1();
  }
  if (nOBJ || pOBJ) StoreChar('+',K89_LINE3,K89_CLN03);
  return 1;
}
//---------------------------------------------------------------------
//  Edit ILS SLOT
//---------------------------------------------------------------------
void CK89gps::EditIlsSlot(short ln,CILS *ils)
{ char edt[16];
  char *id = ils->GetRWID();
  float fq = ils->GetFrequency();
  _snprintf(edt,16,"ILS %s %.02f",id,fq);
  StoreText(edt,ln,K89_CLN06);
  return;
}
//--------------------------------------------------------------------
//  Start in forward mode
//--------------------------------------------------------------------
int CK89gps::StartAPTp9ForW()
{ if (rGPS->ilsQ.GetNbrObj() == 0)   return EditNoILS();
  pOBJ  = 0;
  nOBJ  = rGPS->ilsQ.GetFirst();
  EditFwILS();
  return 1;
}
//--------------------------------------------------------------------
//  Start in Backward mode
//--------------------------------------------------------------------
int CK89gps::StartAPTp9Back()
{ nOBJ  = 0;
  pOBJ  = rGPS->ilsQ.GetLast();
  EditBkILS();
  return 1;
}
//--------------------------------------------------------------------
//  Enter AIRPORT PAGE 9 in forward mode
//--------------------------------------------------------------------
int CK89gps::EnterAPTpage09()
{ nOBJ    = 0;
  pOBJ    = 0;
  SetHandler(K89_NUL_HANDLER,0,0);
  rGPS->GetILS();												// Arm database request
  eorEV   = &CK89gps::StartAPTp9ForW;		// EOR routine
  aState  = K89_APTP9;
  return  1;
}
//=============================================================================
//  State 18:  NAV page 01
//=============================================================================
int CK89gps::RADpage01(K89_EVENT evn)
{ U_CHAR old = curPOS;
  switch (evn)  {
    //----Event EOR:  Edit VOR/NDB Page 1 --------------------------
    case K89_EOR:
        return EditRADpage01();
    //-------CURSOR Event: ENTER CURSOR MODE 1 except in NRS mode --
    case K89_CSR:
        curPOS = (curPOS)?(0):(3);
        if (K89_NOR_EDIT != EdMOD)  return 0;
        EnterCSRMode01(preSF);
        return 0;
    //------PAGE:  ENTER PAGE 2 ------------------------------------
    case K89_PAG:
        if (msDIR == +1)        return EnterRADpage02();
        return 1;
    //-----CLR-  SWAP Radial/bearing mode --------------------------
    case K89_CLR:
        if (3 == curPOS)  OpFlag ^= K89_FLAG_MASK;
        return 1;
    //-----Change NAV number ---------------------------------------
    case K89_FD1:
        if (K89_NRS_EDIT != EdMOD)  return 0;
        curPOS  = 1;
        if (1 == old)             ChangeNRSwpt();
        if ('V' == rGPS->nSTACK)  return EnterVORpage01();
        else                      return EnterNDBpage01();
    //-----Change FLAG TO/FROM -------------------------------------
    case K89_FD2:
        if (K89_NRS_EDIT != EdMOD)  return 0;
        curPOS  = 3;
        OpFlag ^= K89_FLAG_MASK;
        return 1;
    //------NRS ---Nearest mode ------------------------------------
    case K89_NRS:
        return EnterNRSpage01();
    //-----DIR Enter DIRECT TO mode --------------------------------
    case K89_DIR:
        return EnterDIRpage01(cOBJ);
    //------ACT event: Enter Active waypoint -----------------------
    case K89_ACT:
        return EnterACTmode(actWPT);
    //-----CLOCK event: Refresh live data ---------------------------
    case K89_CLOCK:
        if (K89_NRS_EDIT == EdMOD)  return EditOBJpart2(cOBJ);
        return 0;
    //------Other events -------------------------------------------
  default:
      ChangeMode(evn);
  }
  return 1;
}
//------------------------------------------------------------------
//  Enter VOR PAGE 01
//------------------------------------------------------------------
int CK89gps::EnterVORpage01()
{ PageHeader(1);
  rCode   = GPS_GT_VOR;
  SetHandler(K89_GEN_HANDLER,K89_FIELD01,K89_PAG_FD);
  rGPS->GetVOR();											// Arm gps request
  eorEV   = &CK89gps::EditRADpage01;	// EOR routine
  preSF   = &CK89gps::EnterVORpage01;	// Previous routine
  curPOS  = 1;
  LetFD   = Gauge->GetLetField(1);
  aState  = K89_RADP1;
  return 1;
}
//------------------------------------------------------------------
//  Enter NDB PAGE 01
//------------------------------------------------------------------
int CK89gps::EnterNDBpage01()
{ PageHeader(1);
  rCode   = GPS_GT_NDB;
  SetHandler(K89_GEN_HANDLER,K89_FIELD01,K89_PAG_FD);
  rGPS->GetNDB();
  eorEV   = &CK89gps::EditRADpage01;
  preSF   = &CK89gps::EnterNDBpage01;
  curPOS  = 1;
  LetFD   = Gauge->GetLetField(1);
  aState  = K89_RADP1;
  return 1;
}
//-----------------------------------------------------------------------------
//  Edit the NAV header
//-----------------------------------------------------------------------------
void CK89gps::EditNAVheader()
{ char  edt[32];
  short cln = (K89_ACT_EDIT != EdMOD)?(K89_CLN07):(K89_CLN11);
  _snprintf(edt,32,"%2d",(curNO + 1));
  //-----Edit IDENT with 4 characters (padding with spaces) ----
  FormatIdent(cOBJ,K89_LINE0,cln);
  //---Edit flight plan wpt number ---------------
  if ((K89_ACT_EDIT == EdMOD) && (curNO != -1))
  { Prop = K89_ATT_YELOW;
    StoreText(edt,K89_LINE0,K89_CLN08);
  }
  //----Edit NAV Frequency------------------------
  _snprintf(edt,32,"%.2f",((CNavaid*)cOBJ)->GetFrequency());
  StoreText(edt,K89_LINE0,K89_CLN15);
  //----Edit name with padding spaces ------------
  FormatLabel(cOBJ,K89_LINE1,K89_CLN06,Name);
  return;
}
//-----------------------------------------------------------------------------
//  Edit NAV Page 01
//-----------------------------------------------------------------------------
int CK89gps::EditRADpage01()
{ //rGPS->StopReq();
  ClearRightDisplay();
  cOBJ  = rGPS->obj.Pointer();
  wptTO = cOBJ;
  if (0 == cOBJ)    return ClearField(LetFD);
  EditNAVheader();
  if (K89_NRS_EDIT == EdMOD) return EditOBJpart2(cOBJ); 
  //----Edit NAV position ------------------------
  EditLatitude (cOBJ->GetLatitude(), K89_LINE2,K89_CLN09);
  EditLongitude(cOBJ->GetLongitude(),K89_LINE3,K89_CLN09);
  return 1;
}
//=============================================================================
//  State 19:  RAD page 02
//=============================================================================
int CK89gps::RADpage02(K89_EVENT evn)
{ switch(evn) {
    //--------DRAW event: Refresh position and distance ----------
    case K89_CLOCK:
      { if (0 == cOBJ)       return 1;
        //-------Edit deviation --------------------------------------
        if (K89_MODE_VOR == cPage)  EditMagDev(cOBJ->GetMagDev(),K89_LINE1,K89_CLN06);
        //-------Edit Bearing and distance ---------------------------
        FlashChar ('>', K89_LINE3,K89_CLN06,1);
        return EditCAPandDIS(cOBJ);
      }
    //------PAGE EVENT: Return to PAGE 01 (Either VOR or NDB) --------
    case K89_PAG:
        return (this->*preSF)();
    //--------DISPLAY CLICK:  Set cursor or change BEARING/RADIAL ----
    case K89_FD2:
        curPOS  = 1;
        OpFlag ^= K89_FLAG_MASK;
        return 1;
    //--------CURSOR event: swap cursor mode ------------------------
    case K89_CSR:
        curPOS = (curPOS)?(0):(1);
        return 1;
    //--------CLR event:  Swap BEARING/RADIAL -----------------------
    case K89_CLR:
        OpFlag ^= K89_FLAG_MASK;
        return 1;
    //------NRS ---Nearest mode ------------------------------------
    case K89_NRS:
        return EnterNRSpage01();
    //-----DIR Enter DIRECT TO mode --------------------------------
    case K89_DIR:
        return EnterDIRpage01(cOBJ);
    //------ACT event: Enter Active waypoint -----------------------
    case K89_ACT:
        return EnterACTmode(actWPT);
    //--------Other Event: See for change Mode ----------------------
    default:
      ChangeMode(evn);
  }
  return 1;
}
//------------------------------------------------------------------
//  Enter VOR/NDB PAGE 02
//------------------------------------------------------------------
int CK89gps::EnterRADpage02()
{ if (0 == cOBJ) return 1;
  SetHandler(K89_GEN_HANDLER,K89_FIELD02,K89_PAG_FD);
  aState  = K89_RADP2;
  curPOS  = 0;
  OpFlag  = K89_FLAG_TO;
  ClearRightDisplay();
  PageHeader(2);
  FormatIdent(cOBJ,K89_LINE0,K89_CLN07);
  return 1;
}
//=============================================================================
//  State 20:  NAV mode Page01
//=============================================================================
int CK89gps::NAVpage01(K89_EVENT evn)
{ switch(evn) {
    //--------DRAW: Refresh the screen with actual data--------------
    case K89_CLOCK:
        ClearRightLine(0);
        EditNAVpage01();
        return 1;
    //--------DISPLAY CLICK:  Change CDI format ----------------------
    case K89_FD1:
        curPOS = 1;
        cdiFM ^= 1;
        return 1;
    //--------DISPLAY CLICK:  Change Flag format ----------------------
    case K89_FD2:
        curPOS  = 2;
        OpNAV = (OpNAV == 2)?(0):(OpNAV+1);
        return 1;
    //--------CURSOR event: swap cursor mode ------------------------
    case K89_CSR:
        curPOS = (curPOS)?(0):(1);
        return 1;
    //--------CLR event:  Swap field mode     -----------------------
    case K89_CLR:
        if (0 == curPOS)   return 1;
        if (1 == curPOS)   cdiFM  ^= 1;
        if (2 != curPOS)   return 1;
        OpNAV = (OpNAV == 2)?(0):(OpNAV+1);
        return 1;
    //--------PAGE event: --------------------------------------------
    case K89_PAG:
        if (msDIR != -1)  return EnterNAVpage02();
        return 1;
    //-----DIR Enter DIRECT TO mode --------------------------------
    case K89_DIR:
        return EnterDIRpage01(actWPT);
    //------ACT event: Enter Active waypoint -----------------------
    case K89_ACT:
        return EnterACTmode(actWPT);
    //------NRS ---Nearest mode ------------------------------------
    case K89_NRS:
        return EnterNRSpage01();

    //--------Other Event: See for change Mode ----------------------
    default:
      ChangeMode(evn);
  }
  return 1;
}
//--------------------------------------------------------------------
//  Enter NAV page 01
//--------------------------------------------------------------------
int CK89gps::EnterNAVpage01()
{ SetHandler(K89_GEN_HANDLER,K89_FIELD03,K89_PAG_FD);
  aState  = K89_NAVP1;
  rGPS->Clean();
  ClearRightDisplay();
  cPage   = K89_MODE_NAV;
  PageHeader(1);
  StoreText("DTK",K89_LINE2,K89_CLN06);
  StoreText("TK", K89_LINE2,K89_CLN15);
  cdiFM   = 0;                // CDI format
  OpFlag  = K89_FLAG_TO;      // Flag option
  OpNAV   = 0;                // NAV Option
  curPOS  = 0;                // Cursor position
  wptTO   = actWPT;           // For calculator
  return 1;
}

//--------------------------------------------------------------------
//  Edit NAV page 01
//--------------------------------------------------------------------
int CK89gps::EditNAVpage01()
{ char  edt[16];
  //------Edit Line 0 -----------------------------------
  U_CHAR dot = markNV89[Mode];			
  if (0 == actWPT)  return EditWPTNone();
  char *idfr =  prvIDN;
  char *idto =  actWPT->GetIdent();
  StoreText(idfr,K89_LINE0,K89_CLN08);
  if (actDIS <= insDIS) Prop = K89_ATT_FLASH;
  StoreChar(dot, K89_LINE0,K89_CLN13);
  _snprintf(edt,16,"%4s",idto);
  StoreText(edt,K89_LINE0,K89_CLN16);
  //-----Edit Line 1 --(CDI or Direction)----------------
  FlashChar('>',K89_LINE1,K89_CLN06,1);
  cdiDEV  = ComputeDeviation(wOBS,actRAD,&cdiST, 1); 
  EditNAVP01L02();
  EditTrack();
  //-----Edit Line 3 --(CAP and ETE ) ------------------
  EditLegETE(edt,actDIS,Speed);
  StoreText(edt,K89_LINE3,K89_CLN17);
  FlashChar('>',K89_LINE3,K89_CLN06,2);
  //----Edit the current option ------------------------
  switch (OpNAV)  {
		//--- option TO -------------------
    case 0:
        OpFlag = K89_FLAG_TO;
        return EditBearing(actRAD,K89_LINE3,K89_CLN07);
		//--- option FROM -----------------
    case 1:
        OpFlag = K89_FLAG_FROM;
        return EditBearing(actRAD,K89_LINE3,K89_CLN07);
		//--- option VNAV -----------------
    case 2:
        return EditVNAVstatus(K89_LINE3,  K89_CLN07);
  }
  return 1;
}
//--------------------------------------------------------------------
//  Edit no waypoint
//--------------------------------------------------------------------
int CK89gps::EditWPTNone()
{ char edt[16];
  StoreText("No active WPT",K89_LINE0,K89_CLN07);
  OpFlag  = K89_FLAG_NONE;
  cdiDEV  = 0;
  EditNAVP01L02();
  EditTrack();
  EditBearing(actRAD,K89_LINE3,K89_CLN07);
  EditLegETE(edt,-1,Speed);
  StoreText(edt,K89_LINE3,K89_CLN17);
  return 1;
}
//--------------------------------------------------------------------
//  Edit  FLY direction
//--------------------------------------------------------------------
int CK89gps::EditFlyDirection()
{ char  edt[16];
  char  to  = ' ';
  float dis = 0;
  if (K89_FLAG_NONE == cdiST) strncpy(edt," Fly _   __._nm",16);
  else  
  { float rad = DegToRad(cdiDEV);
    dis = actDIS * sin(rad);
    to  = (dis < 0)?('R'):('L');
    dis = abs(dis);
    if (dis > 99) _snprintf(edt,16," Fly %c   %.0fnm",to,dis);
    else          _snprintf(edt,16," Fly %c   %.1fnm",to,dis);  
  }
  StoreText(edt,K89_LINE1,K89_CLN07);
  return 1;
}
//--------------------------------------------------------------------
//  Edit  NAV Page 01 Line 02
//--------------------------------------------------------------------
int CK89gps::EditNAVP01L02()
{ StoreBlnk(16,K89_LINE1,K89_CLN07);
  cdiCL = 0;
  if (0 == cdiFM)  return DrawCDI(K89_LINE1,K89_CLN09);
  if (1 == cdiFM)  return EditFlyDirection();
  return 1;
}

//--------------------------------------------------------------------
//  Edit  Estimated leg duration
//--------------------------------------------------------------------
void CK89gps::EditLegETE(char *edt,float dis,float speed)
{ float dur   = ((dis > 0) && (speed >= 30))?(dis / speed):(0);
  int sec     = int (dur * 3600);
  int hor     = int (dur);
  int min     = (sec - (hor * 3600)) / 60;
  if ((dis <= 0) || (speed < 30))   strncpy(edt,"__:__",16);
  else  if (hor >= 100) _snprintf(edt,16,"%4uh",hor);
  else                  _snprintf(edt,16,"%02u:%02u",hor,min);
  return;
}
//-------------------------------------------------------------------
//  Edit estimated arrival time
//-------------------------------------------------------------------
void CK89gps::EditLegETA(char *edt,float dis)
{ float dur   = ((dis > 0) && (Speed >= 30))?(dis / Speed):(0);
  int sec     = int (dur * 3600);
  int hor     = int (dur);
  int min     = (sec - (hor * 3600)) / 60;
  int day     = hor / 24;
  hor        -= day * 24;
  char nxt    = ' ';
  if ((dis <= 0) || (Speed < 30)) strcpy(edt,"___:__");
  else  if (day) _snprintf(edt,16," %2ud%02u",day,hor);
  else
  { SDateTimeDelta dta = globals->tim->SecondsToDateTimeDelta(sec);
    SDateTime      now = globals->tim->GetUTCDateTime();
    SDateTime      arr = globals->tim->AddTimeDelta (now, dta);
    if (now.date.day != arr.date.day) nxt = '*';
    _snprintf(edt,16,"%c%2u:%02u",nxt,arr.time.hour,arr.time.minute);
  }
  return;
}
//------------------------------------------------------------------
//  Edit Fligh time
//------------------------------------------------------------------
void CK89gps::EditFTime(char *edt)
{ int sec = 0;                          //FPL->GetFlightTime();
  int hor = (sec / 3600);
  int min = (sec - (hor * 3600)) / 60;
//  if (0 == fState)    strcpy(edt,"__:__");
//  else if (hor > 24)  _snprintf(edt,16,"%4uh",hor);
//  else                _snprintf(edt,16,"%02u:%02u",hor,min);
  return;
}
//=============================================================================
//  State 21:  NAV mode Page02
//=============================================================================
int CK89gps::NAVpage02(K89_EVENT evn)
{ switch(evn) {
    //-------Draw event:  refresh page 02 ----------------------------
    case K89_CLOCK:
        EditNAVpage02();
        return 1;
    //--------Click on Line 1: Change format -------------------------
    case K89_FD1:
        curPOS = 1;
        OpPos ^= 1;
        return 1;
    //--------CURSOR event: swap cursor mode ------------------------
    case K89_CSR:
        curPOS = (curPOS)?(0):(1);
        return 1;
    //--------CLR event:  Swap field mode     -----------------------
    case K89_CLR:
        if (0 == curPOS)   return 1;
        OpPos ^= 1;
        return 1;
    //--------PAGE event: --------------------------------------------
    case K89_PAG:
        if (msDIR == +1)  return EnterNAVpage03();
        if (msDIR == -1)  return EnterNAVpage01();
        return 1;
    //------NRS ---Nearest mode ------------------------------------
    case K89_NRS:
        return EnterNRSpage01();
    //-------Change mode ----------------------------------------------
    default:
      ChangeMode(evn);
  }
  return 1;
}
//----------------------------------------------------------------------------
//  Enter NAV page 02
//----------------------------------------------------------------------------
int CK89gps::EnterNAVpage02()
{ SetHandler(K89_GEN_HANDLER,K89_FIELD04,K89_PAG_FD);
  ClearRightDisplay();
  PageHeader(2);
  rGPS->GetVOR();
  StoreText("PRESENT POSN",K89_LINE0,K89_CLN08);
  OpPos   = 0;                // Position format
  curPOS  = 0;
  aState  = K89_NAVP2;
  return 1;
}
//----------------------------------------------------------------------------
//  Edit NAV page 02
//----------------------------------------------------------------------------
int CK89gps::EditNAVpage02()
{ FlashChar('>',K89_LINE0,K89_CLN07,1);
  StoreBlnk(16,K89_LINE2,K89_CLN07);
  StoreBlnk(16,K89_LINE3,K89_CLN07);
  switch (OpPos) {
    //----------Format 0: Edit coordinates -------------------------------
    case 0:
      { SPosition pos = mveh->GetPosition();			//globals->geop;
        EditLatitude (pos.lat,K89_LINE2,K89_CLN10);
        EditLongitude(pos.lon,K89_LINE3,K89_CLN10);
        break;
      }
    //----------Format 1: Edit bearing/cap and distance to nearest VOR ----
    case 1:
      { globals->dbc->SetNearestVOR();
        CNavaid *vor = (CNavaid*)rGPS->obj.Pointer();
        float    dis = (vor)?(vor->GetNmiles()):(-1);
        float    cap = (vor)?(vor->GetRadial()):(0);
        char  edt[16];
        const char *ref = (vor)?(vor->GetIdent()):("None");
        _snprintf(edt,16,"Ref : % 4s",ref);
        StoreText(edt,K89_LINE2,K89_CLN07);
        if (vor)  vor->Refresh(FrameNo);
        OpFlag = (vor)?(K89_FLAG_TO):(K89_FLAG_NONE);
        EditBearing(cap,K89_LINE3,K89_CLN07);
        EditDistance(edt,dis);
        StoreText(edt,K89_LINE3,K89_CLN14);
        break;
      }
  }
  return 1;
}
//=============================================================================
//  State 23:  NAV mode Page03
//=============================================================================
int CK89gps::NAVpage03(K89_EVENT evn)
{ switch(evn) {    
    //--------PAGE event: --------------------------------------------
    case K89_PAG:
        if (msDIR == -1)  return EnterNAVpage02();
        return 1;
    //--------CLOCK event: refresh page ----------------------------
    case K89_CLOCK:
        return EditNAVpage03();
    //------NRS ---Nearest mode ------------------------------------
    case K89_NRS:
        return EnterNRSpage01();
    //--------See for change mode ----------------------------------
    default:
      ChangeMode(evn);
  }
  return 1;
}
//-----------------------------------------------------------------------------
//  Edit departure time
//-----------------------------------------------------------------------------
void CK89gps::EditDepartTime(char *edt)
{ //SDateTime sd;									// = FPL->GetFPLDepTime();
	strncpy(edt,"__:__",16);
 // if (0 == fState)  strcpy(edt,"__:__");
 // else _snprintf(edt,16,"%02u:%02u",sd.time.hour,sd.time.minute);
  return;
}
//-----------------------------------------------------------------------------
//  Edit NAV page 03
//  -Time UTC
//-----------------------------------------------------------------------------
int CK89gps::EditNAVpage03()
{ char edt[16];
  float dis = actDIS;												//(activeWNO != -1)?(FPL->GetTotalDistance()):(wDIS);
  char *idn = ("____");										//(activeWNO != -1)?(FPL->GetLastIdent()):("____");
  if  (0 != actWPT) idn = actWPT->GetIdent();
  SDateTime st = globals->tim->GetUTCDateTime();
  _snprintf(edt,16,"UTC  %02u:%02u",st.time.hour,st.time.minute);
  StoreText(edt,K89_LINE0,K89_CLN13);
  EditDepartTime(edt);
  StoreText(edt,K89_LINE1,K89_CLN18);
  StoreText(idn,K89_LINE2,K89_CLN10);
  EditLegETA(edt,dis);
  StoreText(edt,K89_LINE2,K89_CLN17);
  EditFTime(edt);
  StoreText(edt,K89_LINE3,K89_CLN18);
  return 1;
}
//-----------------------------------------------------------------------------
//  Enter NAV page 03
//-----------------------------------------------------------------------------
int CK89gps::EnterNAVpage03()
{ ClearRightDisplay();
  PageHeader(3);
  StoreText("TIME",   K89_LINE0,K89_CLN06);
  StoreText("Depart", K89_LINE1,K89_CLN06);
  StoreText("ETA",    K89_LINE2,K89_CLN06);
  StoreText("Flight", K89_LINE3,K89_CLN06);
  SetHandler(K89_NUL_HANDLER,0,0);
  aState  = K89_NAVP3;
  return 1;
}
//=============================================================================
//  State 24:  NAV mode Page04
//=============================================================================
int CK89gps::NAVpage04(K89_EVENT evn)
{
  return 1;
}
//=============================================================================
//  State 25:  Direct Mode page 01
//=============================================================================
int CK89gps::DIRpage01(K89_EVENT evn)
{ switch (evn)  {
    //-------EOR: Request completion --------------- -------
    case K89_EOR:
          CheckDIRReq(); 
          return 1;
    //-------CLR --Clear the current waypoint -------------
    case K89_CLR:
          ClearIdent();
          return 1;
    //-------CLICK on letter ------------------------------
    case K89_DPY:
          if (0 == CheckNewLetter()) return 1;
          rCode   = GPS_ANY_WPT;
          dState = 1;
          return 1;
    //------NRS ---Nearest mode ------------------------------------
    case K89_NRS:
          return EnterNRSpage01();
    //-------ENTER:  Set the candidate to active or null waypoint--
    case K89_ENT:
          return LeaveDIRmode();
    //------ACT event: Enter Active waypoint -----------------------
    case K89_ACT:
          return EnterACTmode(actWPT);
    //----OTHER EVENTS ---Change mode---------------------
    default:
      ChangeMode(evn);
 }
  return 1;
}
//----------------------------------------------------------------------------
//  Enter DIR page 01
//  NOTE: 
//    LetFD should be correctly set  before entering here
//    dirWPT  is the candidate waypoint
//----------------------------------------------------------------------------
int CK89gps::EnterDIRpage01(CmHead *obj)
{ wptTO = obj;	
	HeaderEnt("DIRECT TO:",K89_CLN09);
  dirWPT    = obj;
  SetIdentity(obj);
  //-------Init waypoint identifier -------------------------------------
  rCode   = NO_REQUEST;
  SetHandler(K89_LET_HANDLER,0,0);
  preSF   = &CK89gps::LeaveDIRmode;
  aState  = K89_DIRP1;
  dState  = 0;
  return 1;
}
//----------------------------------------------------------------------------
//  Clear the current waypoint
//----------------------------------------------------------------------------
int CK89gps::ClearIdent()
{ char *dst = LetFD->aChar;
  while (*dst)  *dst++ = '_';
  FlashText(LetFD->aChar,LetFD->aDisp);
  dirWPT  = 0;
  return 0;
}
//----------------------------------------------------------------------------
//  Set Object identity
//----------------------------------------------------------------------------
int CK89gps::SetIdentity(CmHead *obj)
{ if (0 == obj) return ClearIdent();
  char *idn = 0;
  switch (obj->GetActiveQ())  {
    case APT:
      LetFD = Gauge->GetLetField(4);
      idn = obj->GetIdent();
      break;
    case VOR:
      LetFD = Gauge->GetLetField(4);
      idn = obj->GetIdent();
      break;
    case NDB:
      LetFD = Gauge->GetLetField(4);
      idn = obj->GetIdent();
      break;
    case WPT:
      LetFD = Gauge->GetLetField(5);
      idn   = obj->GetName();
      break;
  }
  char *dst = LetFD->aChar;
  while (*dst) if (*idn) *dst++ = *idn++; else *dst++ = '_';
  FlashText(LetFD->aChar,LetFD->aDisp);
  return 1;
}
//----------------------------------------------------------------------------
//  Leave DIRECT TO when ENTER validates the current proposed WAYPOINT
//  New ACTIVE WAYPOINT IS either
//  -The candidate waypoint is not null
//    In this case, it can be a WPT from the flight plan. If so, the FPL is
//    warned that this is the new active node.
//  -None
//    Then there is no active waypoint
//---------------------------------------------------------------------------
//  Leg MODE:  OBS is the current direction to the new active waypoint
//  OBS MODE:  OBS is given by the coupled navigation gauge
//----------------------------------------------------------------------------
//  VNAV Waypoint is no more valid in any case.
//----------------------------------------------------------------------------
int CK89gps::LeaveDIRmode()
{ switch (dState) {
    case 0:
        //---Proposed waypoint accepted or cleared ---------
        return SetDIRwaypoint();
    //----Launch a request to get the waypoint -------------
    case 1:
        GetAnyWaypoint();
        return 0;
    //---- return from dupplicate pages --------------------
    case 2:
        if (dirWPT.Assigned()) return SetDIRwaypoint();
        return EnterDIRpage01(0);      // Re enter DIR TO
  }
  return 0;                         
}
//----------------------------------------------------------------------------
//  Set the Direct To active waypoint
//  The direct To may be any waypoint either outside of the current flight plan
//  or one of the flight plan node.
//  When the direct to is one of the flight plan node, the current active
//  waypoint is saved.  
//  The saved waypoint is restored  if the Direct To mode is canceled
//  and the flight plan has not changed.
//----------------------------------------------------------------------------
int CK89gps::SetDIRwaypoint()
{ //--- Check for real waypoint ------------------------------------
	if (dirWPT.Assigned())
	{	PushMode(GPS_MODE_DIRECT);
		FPL->AssignDirect(dirWPT.Pointer());
		actWPT	   = dirWPT.Pointer();
		dirWPT  = 0;
	}
	else
	{ PopMode();
		if (Mode == GPS_MODE_FPLAN) FPL->RestoreNode();
	}
  //--In both cases, VNAV is not valid anymore ---------------------
  InitVnavWPT();                        // Reset VNAV
  wptTO = actWPT;                       // For calculator
  return EnterNAVpage01();
}
//----------------------------------------------------------------------------
//  End Of Request. Analyse for multiple waypoints
//----------------------------------------------------------------------------
int CK89gps::CheckDIRReq()
{ int nbr = rGPS->wptQ.GetNbrObj();
  rCode   = NO_REQUEST;
  if (nbr == 0) return ClearIdent();
  dState  = 2;
  return EnterDUPpage01(&CK89gps::LeaveDIRmode);
}
//=============================================================================
//  State 26:  Direct Mode page 02
//  Display waypoint for approval or clearing
//  In both case return to calling mode
//  NOTE: Context is not restored if mode is changed
//=============================================================================
int CK89gps::DUPpage02(K89_EVENT evn)
{ switch (evn)  {
    //-------CLR --Back to Page 01 -------------
    case K89_CLR:
        ClearIdent();
        PopContext();
        return (this->*preSF)();
    //-------ENTER:  Set the candidate to active or null waypoint--
    case K89_ENT:
        PopContext();
        return (this->*preSF)();
    //------NRS ---Nearest mode ------------------------------------
    case K89_NRS:
        return EnterNRSpage01();
 }
    //----OTHER EVENTS ---Change mode---------------------
  ChangeMode(evn);
  return 1;
}
//----------------------------------------------------------------------------
//  Enter Dupplicate Directory Page 2
//----------------------------------------------------------------------------
int CK89gps::EnterDUPpage02()
{ char edt[16];
  ClearRightDisplay();
  //-----Edit IDENT with 4 character (padding with spaces) --
  FormatIdent(dirWPT.Pointer(),K89_LINE0,K89_CLN07);
  //----Edit name with padding spaces -----------------------
  FormatLabel(dirWPT.Pointer(),K89_LINE1,K89_CLN06,Name);
  //---------------------------------------------------------
  _snprintf(edt,16,"% 6uft",int(dirWPT->GetElevation()));
  StoreText(edt,K89_LINE0,K89_CLN15);
  //----------------------------------------------------------
  char *loc = dirWPT->GetCountry();
  strncpy(edt,loc,4);
  edt[2]  = 0;
  StoreText(edt,K89_LINE3,K89_CLN06);
  //----------------------------------------------------------
  aState  = K89_DIRP2;
  return 1;
}
//=============================================================================
//  State 27:  Dupplicate Ident Page selector
//  NOTE: Context is not restored if mode is changed
//=============================================================================
int CK89gps::DUPpage01(K89_EVENT evn)
{ switch (evn)  {
    //-------Page Change cursor -----------------------------------------
    case K89_PAG:
        return CursorDUPchange();
    //-------ENTER:  select the candidate WPT from the stack ---------
    case K89_ENT:
        dirWPT  = Stack[curPOS];			
        EnterDUPpage02();
        return 1;
    //-------CLR: ---- Return to caller with no waypoint -------------
    case K89_CLR:
        dirWPT  = 0;
        PopContext();
        return (this->*preSF)();
    //------NRS ---Nearest mode ------------------------------------
    case K89_NRS:
        return EnterNRSpage01();
    //------ACT event: Enter Active waypoint -----------------------
    case K89_ACT:
        return EnterACTmode(actWPT);
  }
  //----OTHER EVENTS ---Change mode---------------------
  ChangeMode(evn);
  return 1;
}
//--------------------------------------------------------------------
//  Enter dupplicate page with caller return
//--------------------------------------------------------------------
int CK89gps::EnterDUPpage01(PreFN cal)
{ if (0 == PushContext()) return 0;
  preSF   = cal;
  SetHandler(K89_NUL_HANDLER,0,0);
  MaxNO   = rGPS->wptQ.GetNbrObj();
  dirWPT  = rGPS->wptQ.GetFirst();
  if (1 == MaxNO) return EnterDUPpage02();
  curNO   = 0;
  LimNO   = (MaxNO > 3)?(MaxNO - 3):(0);
  nOBJ    = rGPS->wptQ.GetFirst();
  curPOS  = 1;
  aState  = K89_DIRP3;
  EditDUPwaypoint();
  return 1;
}
//---------------------------------------------------------------------
//  Edit dupplicate waypoint list
//---------------------------------------------------------------------
int CK89gps::EditDUPwaypoint()
{ int nl = 1;
  CmHead *wpt = nOBJ;
  DupHeader();
  while (wpt && (nl != 4))
  { EditWPTSlot(nl,wpt);
    nl++;
    wpt = wpt->NextInQ1();
  }
  return 1;
}
//---------------------------------------------------------------------
//  Edit waypoint slot
//---------------------------------------------------------------------
int CK89gps::EditWPTSlot(short lin, CmHead *wpt)
{ char edt[16];
  char *typ = GetWPTtype(wpt->GetActiveQ());
  short No  = curNO + lin;
  Prop  = (lin == curPOS)?(K89_ATT_FLASH):(K89_ATT_NONE);
  _snprintf(edt,16,"%2u %-4s %s %s",No,wpt->GetIdent(),typ,wpt->GetCountry());
  StoreText(edt,lin,K89_CLN10);
  Stack[lin]  = wpt;									// Save in stack
  return 1;
}
//---------------------------------------------------------------------
//  Change  cursor
//---------------------------------------------------------------------
int CK89gps::CursorDUPchange()
{ short No = curNO + curPOS;
  switch (msDIR)  {
    case +1:
      if (No == MaxNO)    return 0;
      curPOS++;
      if (curPOS != 4)    break;
      curPOS  = 3;
      if (curNO == LimNO) return 0;
      curNO++;
      nOBJ  = nOBJ->NextInQ1();
      break;
    case -1:
      curPOS--;
      if (curPOS != 0)    break;
      curPOS  = 1;
      if (curNO == 0)     return 0;
      curNO--;
      nOBJ  = nOBJ->PrevInQ1();
      break;
}
  EditDUPwaypoint();
  return 1;
}

//---------------------------------------------------------------------
//  Edit WPT header
//---------------------------------------------------------------------
void  CK89gps::DupHeader()
{ char edt[8];
  ClearRightDisplay();
  StoreText("D        Typ Area",K89_LINE0,K89_CLN06);
  _snprintf(edt,8,"%u",rGPS->wptQ.GetNbrObj());
  StoreText(edt,K89_LINE1,K89_CLN06);
  return;
}
//=============================================================================
//  State 28:  Nearest mode Mode page 1
//=============================================================================
int CK89gps::NRSpage01(K89_EVENT evn)
{ switch (evn)  {
    //-------Page Change Page ------------------------------------
    case K89_PAG:
        return CursorNRSchange();
    //-------ENTER:  select the nearest waypoint -----------------
    case K89_ENT:
        rGPS->Clean();
        return DispatchNRSmode();
    //------CLOCK event: edit option page ------------------------
    case K89_CLOCK:
        return EditNRSoptions();
    //-------Change mode -----------------------------------------
    default:
      ChangeMode(evn);
  }
  return 1;
}
//-----------------------------------------------------------------------
//  Edit the NRS options
//-----------------------------------------------------------------------
int CK89gps::EditNRSoptions()
{ if (1 == curPOS)  Prop    = K89_ATT_FLASH;
  StoreText("APT?",K89_LINE1,K89_CLN07);
  if (2 == curPOS)  Prop    = K89_ATT_FLASH;
  StoreText("VOR?",K89_LINE1,K89_CLN12);
  if (3 == curPOS)  Prop    = K89_ATT_FLASH;
  StoreText("NDB?",K89_LINE1,K89_CLN17);
  if (4 == curPOS)  Prop    = K89_ATT_FLASH;
  StoreText("INT?",K89_LINE2,K89_CLN07);
  return 1;
}
//-----------------------------------------------------------------------
//  Dispatch nearest mode
//-----------------------------------------------------------------------
int CK89gps::DispatchNRSmode()
{ rGPS->wptNo   = 0;
  nProp         = K89_ATT_NONE;
  rGPS->obj     = 0;
  OpFlag        = K89_FLAG_TO;
  switch (curPOS) {
    case 1:
      rGPS->nSTACK  = 'A';
      cPage =  K89_MODE_APT;
      return EnterAPTpage01();

    case 2:
      rGPS->nSTACK  = 'V';
      cPage =  K89_MODE_VOR;
      return EnterVORpage01();

    case 3:
      rGPS->nSTACK  = 'N';
      cPage =  K89_MODE_NDB;
      return EnterNDBpage01();

    case 4:
      rGPS->nSTACK  = 'I';
      cPage =  K89_MODE_INT;
      return EnterWPTpage01();
   }
return 0
;}
//-----------------------------------------------------------------------
//  Enter nearest mode
//-----------------------------------------------------------------------
int CK89gps::EnterNRSpage01()
{ sBuf.aState   = 0;
  rGPS->nSTACK  = 0;
  EdMOD         = K89_NRS_EDIT;
  HeaderEnt("NEAREST",K89_CLN10);
  curPOS  = 1;
  aState  = K89_NRSP1;
  return 1;
}
//---------------------------------------------------------------------
//  Cursor Change
//---------------------------------------------------------------------
int CK89gps::CursorNRSchange()
{ switch (msDIR)  {
    case +1:
      curPOS++;
      if (5 == curPOS)  curPOS = 1;
      break;
    case -1:
      curPOS--;
      if (0 == curPOS)  curPOS = 4;
      break;
  }
return 1;
}
//=============================================================================
//  State 29:  Flight Plan Mode page 1
//=============================================================================
int CK89gps::FPLpage01(K89_EVENT evn)
{ //short act;
  switch (evn)  {
  //---CLOCKevent: refresh page ----------------------------------
  case K89_CLOCK:
      ClearRightDisplay();
      return EditFlightPage();
  //---PAGE event:  Change displayed nodes -----------------------
  case K89_PAG:
      if (0 == curPOS)  BrowseFPLnode();
      else              BrowseFPLpage();
      return 1;
  //---CSR event: Swap cursor on option --------------------------
  case K89_CSR:
      if (0 == curPOS)  BrowseFPLpage();
      else              curPOS = 0;
      return 1;
  //---CLR event: Change option ----------------------------------
  case K89_CLR:
      if (evn)	return 1;	//if (FPL->NotStable()) return 1;
      if (5 != curPOS)    return 1;
      OpFPL++;
      if (4 == OpFPL) OpFPL = 0;
      return 0;
  //----Direct TO:  Enter direct To if a waypoint is selected ----
  case K89_DIR:
    { if ((curPOS < 1) || (curPOS > 4))     return 1;
      if (IsChar(' ',(curPOS-1),K89_CLN09)) return 1;
      return EnterDIRpage01(Stack[curPOS - 1]);  /// Enter waypoint
    }
  //------NRS ---Nearest mode ------------------------------------
  case K89_NRS:
      return EnterNRSpage01();
  //------ACT event: Enter Active waypoint -----------------------
  case K89_ACT:
    { if ((curPOS < 1) || (curPOS > 4)) return EnterACTmode(actWPT);
      CmHead *obj = Stack[curPOS - 1];
      return EnterACTmode(obj);
    }
  //---Change to other modes -------------------------------------
  default:
    ChangeMode(evn);
  }
  return 1;
}
//-----------------------------------------------------------------------------
//  Enter flight plan mode
//-----------------------------------------------------------------------------
int CK89gps::EnterFLPpage01()
{ FPL     =  mveh->GetFlightPlan();	
	ClearRightDisplay();
  PageHeader(1);
  SetHandler(K89_NUL_HANDLER,0,0);
  OpFPL   = 0;
  curPOS  = 0;
  aState  = K89_FPLP1;
  EditFlightPage();
  return 1;
}
//---------------------------------------------------------------------
//  Change Cursor to next waypoint slot
//	Cycle cursor to line 0-1-2-3 and DIS label (NOTE curPOS is one ahead)
//	skip empty line
//	Store potential direct to in wptTO
//---------------------------------------------------------------------
int CK89gps::BrowseFPLpage()
{ if (2 > fpMax)    return 1;
  curPOS++;
  if (curPOS == 6)  curPOS = 1;
  if (curPOS == 5)  return 1;
	//--- On empty line column 9 is a space rather than ':'  -----
	wptTO	= 0;
  if (IsChar(' ',(curPOS - 1), K89_CLN09)) curPOS = 5;
  else wptTO = Stack[curPOS - 1];
  return 1;
}
//---------------------------------------------------------------------
//  Change Waypoint
//	Scroll attention (flash) throught the way point list
//	under user action
//---------------------------------------------------------------------
int CK89gps::BrowseFPLnode()
{ if (0 == basWP)		return 1;					// No flight plan
	int No = basWP->GetSequence();
  switch(msDIR) {
  case +1:
		No += 4;
    if (!FPL->Exist(No))   return 1;
    basWP	= FPL->NextNode(basWP);
    return 1;

  case -1:
    No -= 1;
    if (!FPL->Exist(No))	 return 1;
		basWP	= FPL->PrevNode(basWP);
    return 1;
  }
  return 1;
}
//---------------------------------------------------------------------
//  Edit No Flight Plan
//---------------------------------------------------------------------
int CK89gps::EditNoFplan()
{ StoreText("No fligh plan",K89_LINE0,K89_CLN07);
  return 1;
}
//---------------------------------------------------------------------
//  Edit Flight Plan in forward mode
//	NOTE: We must show active waypoint if one exist, except when
//				user is scrolling throught the list
//---------------------------------------------------------------------
int CK89gps::EditFlightPage()
{ if (FPL->IsEmpty())	return EditNoFplan();
  short lin = 0;
  CWPoint *wpt = basWP;
  while (lin != 3)
  { if (!EditFPLSlot(lin++,wpt))	return 1;
		wpt  = FPL->NextNode(wpt);
  }
  wpt = FPL->LastNode();
  EditFPLSlot(lin,wpt);
  return 1;
}
//---------------------------------------------------------------------
//  Edit slot option 
//---------------------------------------------------------------------
int CK89gps::EditFPLoption(char *edt, CWPoint *wpt)
{ switch (OpFPL)  {
    //--------Cumulated distance -----------------
    case 0:
      { float dis = wpt->GetSumDistance();
        if (dis > 999)  _snprintf(edt,16,"%5u ",int(dis));
				else
    		if (dis == 0)		strcpy(edt,"    ");
        else            _snprintf(edt,16,"%.1f ",dis);
        return 1;
      }
    //-------ETE: Estimated time to cross leg ---
    case 1:
      { U_INT dd = 0;
        U_INT hh = 0;
        U_INT mm = 0;
 //       wpt->GetLegDuration(&dd,&hh,&mm);
        if (dd)         _snprintf(edt,16,">1 day",dd);
        else            _snprintf(edt,16,"%02u:%02u ",hh,mm);
        return 1;
        }
    //------ETA: Estimated arrival time ----------
    case 2:
      { U_INT hh = 0;
        U_INT mn = 0;
        char  dd = 0;			//wpt->GetSameDay();
        //	wpt->GetArrivalTime(&hh,&mn);
        _snprintf(edt,16,"%02u:%02u%c",hh,mn,dd);
        return 1;
      }
    //------Dtk: Desired track to next leg -------
    case 3:
      { float dtk = 0;			//wpt->GetDTK();
        int   dti = Round(dtk);
        _snprintf(edt,16,"%3u°  ",dti);
        return 1;
      }
    }
  return 1;
}
//---------------------------------------------------------------------
//  Edit Marker for this Waypoint
//---------------------------------------------------------------------
void CK89gps::EditFPLmark(short lin,CWPoint *wpt)
{ if (FPL->Inactive())	return;							// Inactive flight plan
	char mk = markWP89[FPL->NodeType(wpt)];		// Marker
	if (actDIS <= insDIS) Prop = K89_ATT_FLASH;
  StoreChar(mk,lin,K89_CLN06);
  return;
}
//---------------------------------------------------------------------
//  Edit Waypoint SLOT
//	Return true if more waypoints are expected
//---------------------------------------------------------------------
bool CK89gps::EditFPLSlot(short lin,CWPoint *wpt)
{ if (0 == wpt)			return false;
  bool rs = !wpt->IsLast();
	//--- Edit current waypoint -------------------
	char edt[16];
  CmHead *obj = wpt->GetDBobject();
  short    No = wpt->GetSequence();
  char   *idn	= (obj)?(obj->GetIdent()):("????");
	Stack[lin]  = obj;
  _snprintf(edt,16,"%2u:%-4s",No,idn);
  if ((lin + 1) == curPOS)  Prop = K89_ATT_FLASH;
  StoreText(edt,lin,K89_CLN07);
  EditFPLmark(lin,wpt);
  if (lin == 0)
  { Prop  = (5 == curPOS)?(K89_ATT_FLASH):(K89_ATT_YELOW);
    _snprintf(edt,16,">%s",fplTAB[OpFPL]);
    StoreText(edt,lin,K89_CLN16);
    return rs;
  }
  EditFPLoption(edt,wpt);
  StoreText(edt,lin,K89_CLN16);
  return rs;
}

//=============================================================================
//  WPT mode
//=============================================================================
int CK89gps::WPTpage01(K89_EVENT evn)
{ U_CHAR old = curPOS;
  switch(evn) {
  //------EOR: Edit WPT page 1 ------------------------------------
  case K89_EOR:
      return EditWPTpage01();
  //------CLOCK:  Refresh data  -----------------------------------
  case K89_CLOCK:
      return EditWPTpage01();
  //------CSR enter cursor mode -----------------------------------
  case K89_CSR:
      curPOS = (curPOS)?(0):(3);
      if (K89_NOR_EDIT != EdMOD)  return 0;
      return EnterCSRMode01(&CK89gps::EnterWPTpage01);
  //-----Change WPT number -----------------------------------
  case K89_FD1:
      if (K89_NRS_EDIT != EdMOD)  return 1;
      curPOS  = 1;
      if (1 == old) ChangeNRSwpt();
      return  EnterWPTpage01();
  //-----FD2: Change FLAG FROM/TO ---------------------------------
  case K89_FD2:
      curPOS  = 2;
      OpFlag ^= K89_FLAG_MASK;
      return 1;
  //------DIR: enter direct mode ----------------------------------
  case K89_DIR:
      if (K89_ACT_EDIT == EdMOD)  return 1;
      return EnterDIRpage01(cOBJ);
  //------ACT:  Display ACtive Waypoint ----------------------------
  case K89_ACT:
      return EnterACTmode(actWPT);
  //-----NRS: Enter Nearest mode -----------------------------------
  case K89_NRS:
      return EnterNRSpage01();
  //-------See for other modes ------------------------------------
  default:
      ChangeMode(evn);
  }
  return 1;
}
//------------------------------------------------------------------
//  Enter WPT PAGE 01
//------------------------------------------------------------------
int CK89gps::EnterWPTpage01()
{ rCode   = GPS_GT_WPT;
  SetHandler(K89_GEN_HANDLER,K89_FIELD01,K89_PAG_FD);
  rGPS->GetWPT();											// Arm database request
  eorEV   = &CK89gps::EditWPTpage01;	// EOR routine
  OpFlag  = K89_FLAG_TO;
  aState  = K89_WPTP1;
  LetFD   = Gauge->GetLetField(3);
  return 1;
}

//------------------------------------------------------------------
//  Edit Waypoint PAGE 01
//------------------------------------------------------------------
int CK89gps::EditWPTAsNRS()
{ //----Edit WPT NRS number ---------------------
  char nx = rGPS->wptNo + K89_0_INDEX;
  char No = strip[nx];
  FlashChar(No,K89_LINE0,K89_CLN13,1);
  //----------------------------------------------
  if (0 == cOBJ)  return ClearField(LetFD);
  FormatIdent(cOBJ,K89_LINE0,K89_CLN07);
  FormatLabel(cOBJ,K89_LINE1,K89_CLN06,Name);
  //----Edit CAP and Distance -------------------
  Prop  = K89_ATT_YELOW;
  StoreChar('>',K89_LINE3,K89_CLN06);
  EditCAPandDIS(cOBJ);
  return 1;
}
//------------------------------------------------------------------
//  Edit Waypoint PAGE 01
//------------------------------------------------------------------
int CK89gps::EditWPTpage01()
{ char edt[8];
  _snprintf(edt,8,"%2d",curNO + 1);
 // rGPS->StopReq();
  ClearRightDisplay();
  PageHeader(1);
  cOBJ  = rGPS->obj.Pointer();
  wptTO = cOBJ;
  if (K89_NRS_EDIT == EdMOD)    return EditWPTAsNRS();
  if (0 == cOBJ)                return ClearField(LetFD);
  if (IsActiveWPT(cOBJ))        StoreChar('\x82',K89_LINE0,K89_CLN08);
  if ((K89_ACT_EDIT == EdMOD) && (curNO != -1))
  { Prop = K89_ATT_YELOW;
    StoreText(edt,K89_LINE0,K89_CLN06);
  }
  FormatLabel(cOBJ,K89_LINE0,K89_CLN09,(Name+3));
  //----Edit WPT position ------------------------
  EditLatitude (cOBJ->GetLatitude(), K89_LINE1,K89_CLN09);
  EditLongitude(cOBJ->GetLongitude(),K89_LINE2,K89_CLN09);
  //----Edit CAP and Distance -------------------
  Prop  = K89_ATT_YELOW;
  StoreChar('>',K89_LINE3,K89_CLN06);
  EditCAPandDIS(cOBJ);
  return 1;
}
//=============================================================================
//  ALTITUDE MODE mode
//  TODO Check relationship between BARO and other instrument(ALTI)
//  See if modifing BARO should also update ALTI
//
//=============================================================================
int CK89gps::ALTpage01(K89_EVENT evn)
{ switch (evn)  {
  //-------CLOCK event: Update barometer ------------------------
  case K89_CLOCK:
      return  RefreshALTpage01();
  //-------Left Baro click ----Modify Integer part --------------
  case K89_FD1:
      BaroVL += msDIR;
      return 1;
  //-------Right baro click -------------------------------------
  case K89_FD2:
      if (+1 == msDIR)  BaroVL +=0.01f;
      if (-1 == msDIR)  BaroVL -=0.01f;
      return 1;
  //-------ALT:  Go to page 02 ----------------------------------
  case K89_ALT:
      return EnterALTpage02();
  //------ACT:  Display ACtive Waypoint ----------------------------
  case K89_ACT:
      return EnterACTmode(actWPT);
  //-----NRS: Enter Nearest mode -----------------------------------
  case K89_NRS:
      return EnterNRSpage01();
  //-------See for change mode ----------------------------------
  default:
      ChangeMode(evn);
  }
  return 1;
}

//-----------------------------------------------------------------------------
//  Enter ALTITUDE PAGE 01
//  Save mode and state
//-----------------------------------------------------------------------------
int CK89gps::EnterALTpage01()
{ if (0 == PushContext()) return 0;
  SetHandler(K89_GEN_HANDLER,K89_FIELD05,K89_PAG_FD);
  aState  = K89_ALTP1;
  //------Edit Page 01 --------------------------------
  ClearRightDisplay();
  PageHeader(1);
  StoreText("BARO:",            K89_LINE0,K89_CLN09);
  StoreText("MSA no data",K89_LINE2,K89_CLN09);
  StoreText("ESA no data",K89_LINE3,K89_CLN09);
  FlashWord("*CRSR*",K89_LINE3,K89_CLN00);
  return 1;
}
//-----------------------------------------------------------------------------
//  Edit Barometer
//-----------------------------------------------------------------------------
int CK89gps::RefreshALTpage01()
{ char edt[16];
  _snprintf(edt,8,"%2.2f",BaroVL);
  FlashWord(edt,K89_LINE0,K89_CLN14);
  return 1;
}
//=============================================================================
//  ALTITUDE MODE page 2
//=============================================================================
int CK89gps::ALTpage02(K89_EVENT evn)
{ int cur = 0;
  switch(evn) {
    //-------CLOCK: Refresh page 02 ---------------------------
    case K89_CLOCK:
        RefreshALTpage02();
        Beat--;
        if (Beat == 0) cur = UpdateRPTfield();
        UpdateVnavVSI(cur);
        return 1;
    //-------PAG: Change Cursor -------------------------------
    case K89_PAG:
        return ChangeALTcursor();
    //-------Arm VNAV -----------------------------------------
    case K89_ENT:
        if (5 == curPOS)  vState = K89_VNA_VSI;
        else              vState = K89_VNA_ALT;
        return 1;
    //------FD1 Change cursor ---------------------------------
    case K89_FD1:
        curPOS  = 1;
        return 1;
    //------FD2 Change cursor. Except if not a FPL waypoint ---
    case K89_FD2:
        if (IsDirectMode()) return 1;
        curPOS  = 2;
        return GetNextALTWaypoint();
    //------FD3 Change cursor ---------------------------------
    case K89_FD3:
        curPOS  = 3;
        return 1;
    //------FD4 Change cursor ---------------------------------
    case K89_FD4:
        curPOS  = 4;
        return 1;
    //------FD5 Change cursor ---------------------------------
    case K89_FD5:
        curPOS  = 5;
        VspdVL  = Round100(VspdVL);
        return 1;
    //-------ALT:  Return to previous page --------------------
    case K89_ALT:
        PopContext();
        return 1;
    //------NRS ---Nearest mode ------------------------------------
    case K89_NRS:
        return EnterNRSpage01();
    //------ACT event: Enter Active waypoint -----------------------
    case K89_ACT:
        return EnterACTmode(actWPT);
    //-------See other modes ----------------------------------
    default:
        ChangeMode(evn);
  }
  return 1;
}
//-----------------------------------------------------------------------------
//  Init Vnav Waypoint
//-----------------------------------------------------------------------------
void CK89gps::InitVnavWPT()
{ vnaWPT  = 0;											//actWPT;
  AltiTG  = aPos.alt;
  OffsVL  = 0;
  GrndSP  = K89_GSP_MIN;
  VspdVL  = 0;
  vState  = K89_VNA_OFF;
  return;
}
//-----------------------------------------------------------------------------
//  Enter ALT page 02
//-----------------------------------------------------------------------------
int CK89gps::EnterALTpage02()
{ SetHandler(K89_GEN_HANDLER,1,K89_REP_FD);
  aState  = K89_ALTP2;
  //---------Edit ALT Page 02 --------------------------------
  ClearRightDisplay();
  StoreText("to",           K89_LINE1,K89_CLN13);
  StoreText("GS:",          K89_LINE3,K89_CLN06);
  curPOS  = 1;
  Beat    = K89_BEAT_TEMPO;
  return 1;
}
//-----------------------------------------------------------------------------
//  Edit VNAV state 
//-----------------------------------------------------------------------------
void CK89gps::EditVNAVstate()
{char edt[32];
 switch(vState)  {
    //-----Inactive: Set default values -------------------
    case K89_VNA_OFF:
      StoreText("Vnv Inactive    ", K89_LINE0,K89_CLN06);
      FlashWord("*CRSR*",K89_LINE3,K89_CLN00);
      return;
    //-----Altitude mode ----------------------------------
    case K89_VNA_ALT:
      { int alt = GetTargetVnavALT();
        _snprintf(edt,32,"Vnv Alt %6u",alt);
        StoreText(edt,K89_LINE0,K89_CLN06);
        StoreText("ALT 2 ",K89_LINE3,K89_CLN00);
        return;
      }
    //-----VSI mode ---------------------------------------
    case K89_VNA_VSI:
      { int tim = vTime;
        if  (tim < 3600)
        { int min = tim / 60;
          int sec = tim % 60;
          _snprintf(edt,32,"Vnv in %02u:%02u    ",min,sec);
        }
        else  strncpy(edt,"Vnv Armed       ",32);
        StoreText(edt,K89_LINE0,K89_CLN06);
        //---------Save time to VNAV ---------------------
        StoreText("ALT 2 ",K89_LINE3,K89_CLN00);
        return;
      }
  }
  return;
}
//-----------------------------------------------------------------------------
//  Edit VNAV Status for NAV PAGE 01
// (9 characters wide for consistancy whit bearing option)
//-----------------------------------------------------------------------------
int CK89gps::EditVNAVstatus(short lin,short col)
{ char edt[16];
  switch (vState) {
  case K89_VNA_OFF:
    strncpy(edt,"Vnv Off  ",16);
    break;
  case K89_VNA_ALT:
    { int alt = GetTargetVnavALT();
      _snprintf(edt,16,"Vnv %5u",alt);
      break;
    }
  case K89_VNA_VSI:
    { int tim = vTime;
      if  (tim < 3600)
        { int min = tim / 60;
          int sec = tim % 60;
          _snprintf(edt,16,"Vnv %02u:%02u",min,sec);
        }
        else  strncpy(edt,"Vnv Armed",16);
      break;
    }
  }
  StoreText(edt,lin,col);
  return 1;
}
//-----------------------------------------------------------------------------
//  Recompute VSI:
//  NOTE: VSI is recomputed when inactive or after changing Ground speed
//        When armed VNAV is disarmed if any filed is modified except Ground speed
//-----------------------------------------------------------------------------
void CK89gps::UpdateVnavVSI(int cur)
{ switch (vState) {
    case K89_VNA_OFF:
      break; 
    case K89_VNA_ALT:
      if (0 == cur)           return;       // No modification
      if (4 == cur)           break;        // speed is modified
      vState = K89_VNA_OFF;
      break;
    case K89_VNA_VSI:
      if (0 == cur)           return;       // No modification
      if (4 == cur)           return;        // speed is modified
      vState = K89_VNA_OFF;
      return;
    }
  //------COMPUTE VSI (except if modifing VSI-----------------
  if (5 == curPOS)            return;
  if (vnaWPT.IsNull())        return;
  if (GrndSP < K89_GSP_MIN)   return;
  float ds  = vnaDIS + OffsVL;
  int   Tm  = ((ds* 60) / GrndSP);
  int   Hf  = AltiTG - int(aPos.alt);
  int   vs  = (Tm)?(Hf / Tm):(0);
  if ((vs < K89_VSI_MIN) || (vs > K89_VSI_MAX)) return;
  VspdVL    = vs;
  return;
}
//-----------------------------------------------------------------------------
//  Compute target Altitude  while in mode 1
//  Altitude is rounded up to 100 feets
//-----------------------------------------------------------------------------
int CK89gps::GetTargetVnavALT()
{ float ds  = vnaDIS + OffsVL;
  float Tm  = ((ds * 60) / GrndSP);
  int   al1 = VspdVL * Tm;
  int   alt = al1 / 100;
  int   al2 = al1 % 100;
  if (al2 > 50) alt++;
  return AltiTG - (alt * 100);
}
//-----------------------------------------------------------------------------
//  Compute time to VNAV point in mode 2:
//  Dal = To(Alt) - Now(Alt)
//  Toa = Time to reach To(Alt).Dal / VSI  (Must be positive else entry error)
//  Dva = VNAV Distance.        Dva = Toa * GrnSpeed
//  Dno = Distance now to Target
//  Din = Distance to VNAV.     Din = Dno - Dva
//  Tin = Time to VNAV(sec)  .  Tin = (Din * 3600) / Actual speed
//-----------------------------------------------------------------------------
int CK89gps::GetTimeToVNAV()
{ if (Speed < 30)     return -1;
  float   Dal = AltiTG - aPos.alt;
  float   Toa = Dal / (VspdVL * 60);
  if (Toa <= 0)       return -1;
  float   Dva = Toa * GrndSP;
  float   Din = vnaDIS - Dva;
  int     Tin = int((Din * 3600) / Speed);
  //----Din < 0 => VNAV point overshoot -------------------
  return (Din < 0)?(0):(Tin);
}
//-----------------------------------------------------------------------------
//  Modify any repeatable field
//-----------------------------------------------------------------------------
int CK89gps::UpdateRPTfield()
{ Beat  = K89_BEAT_TEMPO;
  if (0 == RepCK)    return 0;
  int val = *RepCK->aNber;
  val += RepCK->vIncr;
  if (val < RepCK->nbMin)  val = RepCK->nbMin;
  if (val > RepCK->nbMax)  val = RepCK->nbMax;
  *RepCK->aNber = val;
  return curPOS;
}
//-----------------------------------------------------------------------------
//  Modify the Target waypoint in FPL
//-----------------------------------------------------------------------------
int CK89gps::GetNextALTWaypoint()
{ /*
	short   No  = vnaWNO + msDIR;
  if (No < activeWNO)   return 0;
  if (No >= fpMax)   return 0;
  vnaWNO  = No;
  vnaWPT  = 0;			//FPL->GetDBObject(No);
  vState  = K89_VNA_OFF;
	*/
  return 1;
}
//-----------------------------------------------------------------------------
//  Refresh ALT Page 02
//-----------------------------------------------------------------------------
int CK89gps::RefreshALTpage02()
{ char  edt[16];
  const char *idn = (vnaWPT.Assigned())?(vnaWPT->GetIdent()):("....");
  //-------------------------------------
  EditVNAVstate();
  //----Edit Altitude -------------------
  _snprintf(edt,16,"%05u",int(aPos.alt));
  StoreText(edt,K89_LINE1,K89_CLN07);
  _snprintf(edt,16,"%05u",AltiTG);
  if (curPOS == 1)  Prop = K89_ATT_YELOW;
  StoreText(edt,K89_LINE1,K89_CLN16);
  //----Edit Waypoint identifier -------
  if (curPOS == 2)  Prop = K89_ATT_YELOW;
  StoreText(idn,K89_LINE2,K89_CLN06);
  StoreChar(':',K89_LINE2,K89_CLN12);
  //----Edit Offset --------------------
  _snprintf(edt,16,"%+03d",OffsVL);
  if (curPOS == 3)  Prop = K89_ATT_YELOW;
  StoreText(edt,K89_LINE2,K89_CLN15);
  //----Edit Ground Speed --------------
  _snprintf(edt,16,"%03u",GrndSP);
  if (curPOS == 4)  Prop = K89_ATT_YELOW;
  StoreText(edt,K89_LINE3,K89_CLN10);
  //---Edit VSI ------------------------
  _snprintf(edt,16,"%+05dfpm",VspdVL);
  if (curPOS == 5)  Prop = K89_ATT_YELOW;
  StoreText(edt,K89_LINE3,K89_CLN15);
  return 1;
}
//-----------------------------------------------------------------------------
//  ChangeALT cursor on Page 02
//  Skip ident if not a FPL waypoint
//-----------------------------------------------------------------------------
int CK89gps::ChangeALTcursor()
{ switch (msDIR)  {
    case +1:
      if (5 == curPOS) curPOS = 0;
      curPOS++;
      break;
    case -1:
      if (1 == curPOS) curPOS = 6;
      curPOS--;
      break;
  }
  if ((2 == curPOS) && (IsDirectMode())) curPOS += msDIR;
  return 1;
}
//=============================================================================
//  MESSAGE PAGE
//=============================================================================
int CK89gps::MSGpage01(K89_EVENT evn)
{ switch (evn)  {
    //-------CLOCK: Display message ----------------------
    case K89_CLOCK:
      DisplayMSG();
      return 1;
    //-------LEAVE Message page --------------------------
    case K89_MSG:
      msgNO = K89_MSG_OFF;        // Clear message
      wrnNO = 0;                  // Clear warning level
      DshNO = K89_PART_DISPLAY;   // New display handler
      PopContext();               // Restore context
      return 1;
  }
  return 1;
}

//-----------------------------------------------------------------------------
//  Enter Message Page 01
//  NOTE:  Due to the limited context , MSG is prohibited if context is busy
//-----------------------------------------------------------------------------
int CK89gps::EnterMSGpage01()
{ if (0 == PushContext()) return 0;
  RazDisplay();
  DshNO   = K89_FULL_DISPLAY;
  aState  = K89_MSGP1;
  return 1;
}
//-----------------------------------------------------------------------------
//  Display Messages
//-----------------------------------------------------------------------------
int CK89gps::DisplayMSG()
{ StoreText(msgTAB[msgNO],K89_LINE1,K89_CLN08);
  return 1;
}
//=============================================================================
//   NO DATA SCREEN
//=============================================================================
int CK89gps::NULpage01(K89_EVENT evn)
{ switch (evn)  {
    //------NRS ---Nearest mode ------------------------------------
    case K89_NRS:
        return EnterNRSpage01();

    //------ACT event: Enter Active waypoint -----------------------
    case K89_ACT:
        return EnterACTmode(actWPT);
  }
  ChangeMode(evn);
  return 0;
}
//-----------------------------------------------------------------------------
//  Enter NUL page with a message 
//-----------------------------------------------------------------------------
int CK89gps::EnterNULpage01(U_CHAR msg,U_CHAR mode,U_CHAR page)
{ InitState(mode);
  rGPS->Clean();
  SetHandler(K89_NUL_HANDLER,0,0);
  ClearRightDisplay();
  PageHeader(page);
  aState    = K89_NULP1;
  StoreText(txtTAB[msg],K89_LINE1,K89_CLN07);
  return 1;
}
//=============================================================================
//  CALCULATOR  PAGE 01
//=============================================================================
int CK89gps::CALpage01(K89_EVENT evn)
{ switch (evn)  {
    //-------CLOCK evnt:  refresh screen -------------
    case K89_CLOCK:
        Beat--;
        if (Beat == 0) {UpdateRPTfield(); ComputeCALparameters();}
        return RefreshCALpage01();
    //-------DPY:  Modify ident field ----------------
    case K89_DPY:
        if (LetFD->aChar == IdFR) curPOS = 2;
        if (LetFD->aChar == IdTO) curPOS = 3;
        if (CheckNewLetter()) rCode = GPS_ANY_WPT;
        return 0;
    //-------FD1:  Change calculation mode -----------
    case K89_FD1:
        curPOS  = 1;
        if (OpCAL == 1)             OpCAL = 0;
        else if (-1 == CalTNO)      return 0;
        else if (wptFR.Assigned())  return 0;
        else OpCAL = 1;
        return 1;
    //-------FD4:  Click on Field 4-------------------
    case K89_FD4:
        FlashOF(Flash);
        curPOS = 4;
        ComputeCALparameters();
        return 1;
    //-------FD5:  Fuel Flow -------------------------
    case K89_FD5:
        curPOS = 5;
        ComputeCALparameters();
        return 1;
    //-------FD6:  Fuel reserve ----------------------
    case K89_FD6:
        curPOS = 6;
        ComputeCALparameters();
        return 1;
    //-------CSR : Clear Cursor ----------------------
    case K89_CSR:
        if (curPOS) curPOS = 0;
        if (0 == curPOS)  FlashOF(Flash);
        return 1;
    //-------CLR:  Clear Current field ---------------
    case K89_CLR:
        ClearIdent();
        SetCALwaypoint();
        return 1;
    //-------ENT:  Search for waypoint ---------------------
    case K89_ENT:
        return SearchCALwaypoint();
   //------EOR:  End of request. Check waypoint ------------
    case K89_EOR:
        CheckCALreq();
        return 1;
   //------AG:  Change CALCULATOR page -----------------------------
    case K89_PAG:
        ChangeCALpage();
        return 1;
   //------DIR: enter direct mode ----------------------------------
    case K89_DIR:
        return EnterDIRpage01(wptTO.Pointer());
   //------NRS ---Nearest mode ------------------------------------
    case K89_NRS:
        return EnterNRSpage01();
  }
  ChangeMode(evn);
  return 1;
}
//-----------------------------------------------------------------------------
//  Check how many waypoint are returned
//-----------------------------------------------------------------------------
int CK89gps::CheckCALreq()
{ int nbr = rGPS->wptQ.GetNbrObj();
  if (nbr == 0) return ClearIdent();
  return EnterDUPpage01(&CK89gps::SetCALwaypoint);
}
//-----------------------------------------------------------------------------
//  Search for waypoint on last modified field 
//-----------------------------------------------------------------------------
int CK89gps::SearchCALwaypoint()
{ if (0 == LetFD)     return 0;
  if (rCode == GPS_ANY_WPT) GetAnyWaypoint();
  return 0;
}
//-----------------------------------------------------------------------------
//  Set the new waypoint returned by the dupplicated page or the request
//-----------------------------------------------------------------------------
int CK89gps::SetCALwaypoint()
{ EditCALident(LetFD,dirWPT.Pointer());
  if (LetFD->aChar == IdFR)  wptFR = dirWPT;
  if (LetFD->aChar == IdTO)  wptTO = dirWPT;
  if (wptFR.Assigned())  OpCAL = 0;
  ComputeCALparameters();
  return 1;
}
//-----------------------------------------------------------------------------
//  Enter CALCULATOR page 01
//-----------------------------------------------------------------------------
int CK89gps::EnterCALpage01()
{ SetHandler(K89_GEN_HANDLER,K89_FIELD08,K89_PAG_FD);
  OpCAL   = K89_CAL_WPT;
  OpPAG   = 1;
  curPOS  = 0;
  aState  = K89_CALP1;
  ClearRightDisplay();
  //----Init from and TO waypoints ---------
  K89_LETFD *fd = 0;
  rCode   = NO_REQUEST;
  cPage   = K89_MODE_CAL;
  Flash   = 0;
  //---From Waypoint ------------------------
  wptFR   = 0;
  fd      = Gauge->GetLetField(K89_FIELD06);
  EditCALident(fd,wptFR.Pointer());
  //---To Waypoint -------------------------
  CalTNO = 0;					//FPL->NbInFlightPlan(wptTO.Pointer(),0);
	
  if (CalTNO > 0)  OpCAL = 1;
  fd      = Gauge->GetLetField(K89_FIELD07);
  EditCALident(fd,wptTO.Pointer());
  CalSPD  = Speed;
  ComputeCALparameters();
  Beat    = K89_BEAT_TEMPO;
  return 1;
}
//-----------------------------------------------------------------------------
//  Change CAL page
//-----------------------------------------------------------------------------
int CK89gps::ChangeCALpage()
{ switch(msDIR) {
    //----Next page -----------------
    case +1:
      if  (OpPAG == 2)  return 0;
      OpPAG = 2;
      SetHandler(K89_GEN_HANDLER,K89_FIELD09,K89_PAG_FD);
      break;
    //---Previous page --------------
    case -1:
      if  (OpPAG == 1)  return 0;
      OpPAG = 1;
      SetHandler(K89_GEN_HANDLER,K89_FIELD08,K89_PAG_FD);
      break;
  }
  return 1;
}
//-----------------------------------------------------------------------------
//  Edit PAGE 1
//-----------------------------------------------------------------------------
int CK89gps::EditCALpage01()
{ char edt[16];
  //------CAP to WPT --------------------------
  _snprintf(edt,16," %03u°",CalCAP);
  StoreText(edt,  K89_LINE1,K89_CLN06);
  //------Distance to WPT----------------------
  _snprintf(edt,16," %3unm",CalDIS);
  StoreText(edt,  K89_LINE2,K89_CLN06);
  StoreText("ESA",K89_LINE2,K89_CLN13);
  StoreText("n/a",K89_LINE2,K89_CLN17);
  //------Speed and ETE to WPT-----------------
  _snprintf(edt,16," %3ukt ",CalSPD);
  Prop  = (4 == curPOS)?(K89_ATT_FLASH):(K89_ATT_YELOW);
  StoreText(edt,  K89_LINE3,K89_CLN06);
  StoreText("ETE",K89_LINE3,K89_CLN13);
  EditLegETE(edt,float(CalDIS),float(CalSPD));
  StoreText(edt,  K89_LINE3,K89_CLN17);

  return 1;
}
//-----------------------------------------------------------------------------
//  Edit PAGE 2
//-----------------------------------------------------------------------------
int CK89gps::EditCALpage02()
{ char  edt[16];
  //------Speed -------------------------------
  _snprintf(edt,16,"%3ukt",CalSPD);
  Prop  = (4 == curPOS)?(K89_ATT_FLASH):(K89_ATT_YELOW);
  StoreText(edt,  K89_LINE1,K89_CLN06);
  //------Fuel Flow ---------------------------
  StoreText("FF:",K89_LINE2,K89_CLN06);
  _snprintf(edt,16,"%03u ",CalFFL);
  Prop  = (5 == curPOS)?(K89_ATT_FLASH):(K89_ATT_YELOW);
  StoreText(edt,  K89_LINE2,K89_CLN09);
  //------Fuel reserve ------------------------
  StoreText("Res:",K89_LINE2,K89_CLN13);
  _snprintf(edt,16,"%03u  ",CalFRS);
  Prop  = (6 == curPOS)?(K89_ATT_FLASH):(K89_ATT_YELOW);
  StoreText(edt,   K89_LINE2,K89_CLN17);
  //-----Fuel Requested ------------------------
  StoreText("Fuel Req ",K89_LINE3,K89_CLN06);
  _snprintf(edt,16,"%5u  ",CalFUS);
  StoreText(edt,  K89_LINE3,K89_CLN15);
  return 1;
}
//-----------------------------------------------------------------------------
//  Refresh CALCULATOR Page 01
//-----------------------------------------------------------------------------
int CK89gps::RefreshCALpage01()
{ PageHeader(OpPAG);
  //-------Waypoint option --------------------
  FlashChar('>',  K89_LINE0,K89_CLN06,1);
  StoreText(wptTYP[OpCAL],K89_LINE0,K89_CLN07);
  //------Waypoint from -----------------------
  StoreText("Fr", K89_LINE0,K89_CLN12);
  FlashChar('>',  K89_LINE0,K89_CLN14,2);
  //------Waypoint TO -------------------------
  StoreText("To", K89_LINE1,K89_CLN12);
  FlashChar('>',  K89_LINE1,K89_CLN14,3);
  if (1 == OpPAG) return EditCALpage01();
  if (2 == OpPAG) return EditCALpage02();
  return 1;
}

//-----------------------------------------------------------------------------
//  Edit Waypoint identity
//-----------------------------------------------------------------------------
void CK89gps::EditCALident(K89_LETFD *fd,CmHead *wpt)
{ if (wpt) 
  {char *des = fd->aChar;
   char *src = wpt->GetIdent();
   while (*des) if (*src) *des++ = *src++; else *des++ = '_';
  }
  else  strcpy(fd->aChar,"____");
  Prop  = K89_ATT_YELOW;
  StoreText(fd->aDisp,fd->aChar);
  return;
}
//-----------------------------------------------------------------------------
//  Compute parameters between from and to
//-----------------------------------------------------------------------------
void CK89gps::ComputeCALparameters()
{ SPosition pfm = (wptFR.Assigned())?(wptFR->GetPosition()):(aPos);
  SPosition pto = (wptTO.Assigned())?(wptTO->GetPosition()):(aPos);
  float wmag    = (wptTO.Assigned())?(wptTO->GetMagDev()):(0);
  SVector	v	    = GreatCirclePolar(&pfm, &pto);
  float   r     = Wrap360((float)v.h - wmag);
  CalCAP        = Round(r);
//  float   d     = (OpCAL == 0)?(v.r * MILE_PER_FOOT):(FPL->GetDistanceTo(CalTNO));
	float		d     = 0;
	CalDIS        = Round(d);
  //-----Compute Fuel requested ----------------------------------
  CalFUS        = 0;
  if (CalSPD < 30)  return;
  float   h     = float(CalDIS) / CalSPD;
  CalFUS        = int (h * CalFFL) + CalFRS;
  return;
}
//=============================================================================
//  Set PAGES
//=============================================================================
int CK89gps::SETpage01(K89_EVENT evn)
{ switch (evn)  {
    //------Change Page and edit ---------------------------------
    case K89_PAG:
      return ChangeSETpage(msDIR);
  }
  ChangeMode(evn);
  return 1;
}
//----------------------------------------------------------------
//  Enter SET pages
//----------------------------------------------------------------
int CK89gps::EnterSETpage01()
{ ClearRightDisplay();
  cPage   = K89_MODE_SET;
  SetPAG  = 1;
  SetHandler(K89_NUL_HANDLER,0,0);
  aState  = K89_SETP1;
  ChangeSETpage(0);
  return 1;
}
//----------------------------------------------------------------
//  ChangeS SET page
//----------------------------------------------------------------
int CK89gps::ChangeSETpage(char dir)
{ switch(dir) {
      case +1:
        if (11 == SetPAG) SetPAG = 0;
        SetPAG++;
        break;
      case -1:
        if (1 == SetPAG)  SetPAG = 12;
        SetPAG--;
        break;
  }
  ClearRightDisplay();
  PageHeader(SetPAG);
  switch(SetPAG)  {
      case 1:
        return EditSETpage01();
      case 2:
        return EditSETpage02();
      case 3:
        return EditSETpage03();
      case 4:
        return EditSETpage04();
      case 5:
        return EditSETpage05();
      case 6:
        return EditSETpage06();
      case 7:
        return EditSETpage07();
      case 8:
        return EditSETpage08();
      case 9:
        return EditSETpage09();
      case 10:
        return EditSETpage10();
      case 11:
        return EditSETpage11();
  }
  return 1;
}
//----------------------------------------------------------------
//  Edit Set page 01
//----------------------------------------------------------------
int CK89gps::EditSETpage01()
{ StoreText("INIT POS:", K89_LINE0,K89_CLN06);
  EditLatitude (aPos.lat,K89_LINE1,K89_CLN09);
  EditLongitude(aPos.lon,K89_LINE2,K89_CLN09);
  return 1;
}
//----------------------------------------------------------------
//  Edit Set Page 2
//----------------------------------------------------------------
int CK89gps::EditSETpage02()
{ char edt[16];
  char mon[4];
  SDateTime st = globals->tim->GetUTCDateTime();
  strncpy(mon,globals->monTAB[st.date.month],3);
  mon[3] = 0;
  //----------DATE ---------------------------------
  StoreText("DATE ",  K89_LINE0,K89_CLN06);
  _snprintf(edt,16,"%2u %s %02u",st.date.day,mon,(st.date.year-100));
  StoreText(edt,K89_LINE0,K89_CLN11);
  //----------TIME ---------------------------------
  StoreText("TIME ",  K89_LINE1,K89_CLN06);
  _snprintf(edt,16,"%02u:%02u",st.time.hour,st.time.minute);
  StoreText(edt,      K89_LINE1,K89_CLN11);
  //----------TIME ZONE ----------------------------
  Prop  = K89_ATT_YELOW;
  StoreText("UTC",    K89_LINE1,K89_CLN17);
  Prop  = K89_ATT_YELOW;
  StoreText("Universal Time",K89_LINE2,K89_CLN07);
  //----------MAG VAR ------------------------------
  StoreText("Mag var: Auto", K89_LINE3,K89_CLN06);
  return 1;
}
//----------------------------------------------------------------
//  Edit Set Page 3
//----------------------------------------------------------------
int CK89gps::EditSETpage03()
{ StoreText("DB is updated",K89_LINE0,K89_CLN07);
  return 1;
}
//----------------------------------------------------------------
//  Edit Set Page 4
//----------------------------------------------------------------
int CK89gps::EditSETpage04()
{ StoreText("TURN",         K89_LINE0,K89_CLN11);
  StoreText("ANTICIPATION", K89_LINE1,K89_CLN07);
  Prop  = K89_ATT_YELOW;
  StoreText("DISABLE",      K89_LINE2,K89_CLN09);
  return 1;
}
//----------------------------------------------------------------
//  Edit Set Page 5
//----------------------------------------------------------------
int CK89gps::EditSETpage05()
{ StoreText("Default First",  K89_LINE0,K89_CLN06);
  StoreText("Character of",   K89_LINE1,K89_CLN07);
  StoreText("Wpt Identifier", K89_LINE2,K89_CLN06);
  StoreText("Entry:",         K89_LINE3,K89_CLN09);
  Prop  = K89_ATT_YELOW;
  StoreChar('_',              K89_LINE3,K89_CLN16);
  return 1;
}
//----------------------------------------------------------------
//  Edit Set Page 6
//----------------------------------------------------------------
int CK89gps::EditSETpage06()
{ StoreText("NEAREST APT",  K89_LINE0,K89_CLN07);
  StoreText("CRITERIA",     K89_LINE1,K89_CLN11);
  //-------Runway lentgh -------------------------
  StoreText("Length:",      K89_LINE2,K89_CLN06);
  Prop  = K89_ATT_YELOW;
  StoreText("Any",          K89_LINE2,K89_CLN14);
  //-------Surface -------------------------------
  StoreText("Surface:",     K89_LINE3,K89_CLN06);
  Prop  = K89_ATT_YELOW;
  StoreText("Any",          K89_LINE3,K89_CLN15);
  return 1;
}
//----------------------------------------------------------------
//  Edit Set Page 7
//----------------------------------------------------------------
int CK89gps::EditSETpage07()
{ StoreText("SUA Alert",    K89_LINE0,K89_CLN09);
  Prop  = K89_ATT_YELOW;
  StoreText("DISABLE",      K89_LINE1,K89_CLN10);
  return 1;
}
//----------------------------------------------------------------
//  Edit Set Page 8
//----------------------------------------------------------------
int CK89gps::EditSETpage08()
{ StoreText("SET UNITS:",  K89_LINE0,K89_CLN09);
  //-------- BARO units ------------------------
  StoreText("BARO    :",   K89_LINE1,K89_CLN06);
  Prop  = K89_ATT_YELOW;
  StoreText("Inches",      K89_LINE1,K89_CLN16);
  //-------- ALTITUDE --------------------------
  StoreText("Alt-APT :",   K89_LINE2,K89_CLN06);
  Prop  = K89_ATT_YELOW;
  StoreText("Feet",        K89_LINE2,K89_CLN16);
  //--------DISTANCES --------------------------
  StoreText("Dist-Vel:",   K89_LINE3,K89_CLN06);
  Prop  = K89_ATT_YELOW;
  StoreText("nm-kt",       K89_LINE3,K89_CLN16);
  return 1;
}
//----------------------------------------------------------------
//  Edit Set Page 9
//----------------------------------------------------------------
int CK89gps::EditSETpage09()
{ StoreText("Altitude",   K89_LINE0,K89_CLN09);
  //---------Altitude  Alert ------------------
  StoreText("Alert:",     K89_LINE1,K89_CLN07);
  Prop  = K89_ATT_YELOW;
  StoreText("Off",        K89_LINE1,K89_CLN14);
  //---------Offset ---------------------------
  StoreText("Warn: ",     K89_LINE2,K89_CLN07);
  Prop  = K89_ATT_YELOW;
  StoreText("400ft",      K89_LINE2,K89_CLN14);
  //---------Volume ---------------------------
  StoreText("Volume:",    K89_LINE3,K89_CLN07);
  StoreText("8",          K89_LINE3,K89_CLN15);
  return 1;
}
//---------------------------------------------------------------
//  Edit SET page 10
//---------------------------------------------------------------
int CK89gps::EditSETpage10()
{ StoreText("BUS MONITOR",      K89_LINE0,K89_CLN08);
  StoreText("Bus Volt   27.4V", K89_LINE1,K89_CLN06);
  StoreText("Alert Volt ",      K89_LINE2,K89_CLN06);
  Prop  = K89_ATT_YELOW;
  StoreText("OFF",              K89_LINE2,K89_CLN17);
  return 1;
}
//---------------------------------------------------------------
//  Edit SET page 11
//---------------------------------------------------------------
int CK89gps::EditSETpage11()
{ StoreText("MIN DISPLAY",      K89_LINE0,K89_CLN08);
  StoreText("BRIGHTNESS ADJ",   K89_LINE1,K89_CLN06);
  Prop  = K89_ATT_YELOW;
  StoreText("4 Default",        K89_LINE3,K89_CLN12);
  return 1;
}
//=============================================================================
//  Enter ACT mode
//=============================================================================
int CK89gps::EnterACTmode(CmHead *obj)
{ InitState(K89_MODE_ACT);
  EdMOD         = K89_ACT_EDIT;
  nProp         = K89_ATT_NONE;
  rGPS->Clean();
  rGPS->obj     = obj;
  rGPS->wptNo   = 0;
  curNO         = 0;			//FPL->NbInFlightPlan(obj,0);
  if (0 == obj)   return EnterNULpage01(1,K89_MODE_ACT,1);
  switch (obj->GetActiveQ())  {
    case APT:
      return EnterAPTpage01();
    case VOR:
      return EnterVORpage01();
    case NDB:
      return EnterNDBpage01();
    case WPT:
      return EnterWPTpage01();
  }
  return 1;
}
//-----------------------------------------------------------------------------
//  Change Nearest Object
//-----------------------------------------------------------------------------
int CK89gps::ChangeNRSwpt()
{ switch (msDIR)  {
    //----Get next nearest waypoint from current ----------
    case +1:
      globals->dbc->SetNextNRSobj();
      break;
    //----Get Previous nearest airport -------------------
    case -1:
      globals->dbc->SetPrevNRSobj();
      break;
  }
return 1;
}
//=============================================================================
//
//  HELPERS
//
//=============================================================================
//  Save the current context 
//----------------------------------------------------------------------------
int CK89gps::PushContext()
{ if (sBuf.aState)  return 0;
  sBuf.cPage    = cPage;
  sBuf.aState   = aState;
  sBuf.LetFD    = LetFD;
  sBuf.SnoFD    = SnoFD;
  sBuf.StyFD    = StyFD;
  sBuf.curPOS   = curPOS;
  sBuf.HndNO    = HndNO;
  CopyDisplay(Display,sBuf.Display);
  return 1;
}
//----------------------------------------------------------------------------
//	Push actual mode
//----------------------------------------------------------------------------
void CK89gps::PushMode(U_CHAR m)
{	pMod = Mode;
	Mode = m;
	return;
}
//----------------------------------------------------------------------------
//	Pop actual mode
//----------------------------------------------------------------------------
void CK89gps::PopMode()
{	Mode = pMod;
	pMod = 0;
}
//----------------------------------------------------------------------------
//  Restore previous context
//  aState is set to 0 to indicate that context is free
//----------------------------------------------------------------------------
void CK89gps::PopContext()
{ cPage   = sBuf.cPage;
  aState  = sBuf.aState;
  LetFD   = sBuf.LetFD;
  SnoFD   = sBuf.SnoFD;
  StyFD   = sBuf.StyFD;
  HndNO   = sBuf.HndNO;
  curPOS  = sBuf.curPOS;
  CopyDisplay(sBuf.Display,Display);
  sBuf.aState  = 0;
  return;
}
//----------------------------------------------------------------------------
//  Prepare Enter
//  Set Selected field on ident
//----------------------------------------------------------------------------
int CK89gps::HeaderEnt(char *txt,short col)
{ ClearRightDisplay();
  StoreText(txt,K89_LINE0,col);
  FlashWord("*CRSR*",    K89_LINE3,K89_CLN00);
  wrnNO = 1;                // Enter Warning
  return 0;
}
//-----------------------------------------------------------------------------
//  Return Waypoint type
//-----------------------------------------------------------------------------
char *CK89gps::GetWPTtype(QTYPE cod)
{ if (cod == APT) return "APT";
  if (cod == VOR) return "VOR";
  if (cod == NDB) return "NDB";
  if (cod == WPT) return "INT";
  return "???";
}
//-----------------------------------------------------------------------------
//  Ident format
//-----------------------------------------------------------------------------
void CK89gps::FormatIdent(CmHead *obj,short lin,short col)
{ char *src = obj->GetIdent();
  char *dst = Ident;
  while (*dst)  if (*src) *dst++ = *src++; else *dst++ = ' ';
  if (IsActiveWPT(obj)) StoreChar('\x82',lin,K89_CLN06);
  Prop      = nProp;
  StoreText(Ident,lin,col);
  return;
}
//------------------------------------------------------------------
//  Format Name
//------------------------------------------------------------------
void CK89gps::FormatLabel(CmHead *obj,short lin,short col,char *dst)
{ char *src   = obj->GetName();
  char *des   = dst;
  while (*des)  if (*src) *des++ = *src++; else *des++ = ' ';
  Prop  = nProp;
  StoreText(dst,lin,col);
  return;
}
//------------------------------------------------------------------
//  Edit Page Header
//------------------------------------------------------------------
int  CK89gps::PageHeader(short NoPage)
{ char edt[16];
  _snprintf(edt,16,"%s %u ",modTAB[cPage],NoPage);
  StoreText(edt,K89_LINE3,K89_CLN00);
  wrnNO = 0;
  return 0;
}
//------------------------------------------------------------------
//  Extract filter argument
//------------------------------------------------------------------
void CK89gps::GetFilterArg(K89_LETFD *fd)
{ char *src   = fd->aChar;
  char *dst   = Arg;
  char  pos   = fd->cPos;
  //----Copy filter argument -------------------------
  *dst++ = '@';
  while (pos >= 0) {*dst++ = *src++; pos--;}
  *dst = 0;
  return;
}
//------------------------------------------------------------------
//  Launch a request for any waypoint by ident
//  Skip the @ filter
//------------------------------------------------------------------
void CK89gps::GetAnyWaypoint()
{ GetFilterArg(LetFD);
  rGPS->Clean();
  rGPS->ClearFilter();
  rGPS->SetST2(Arg+1);
  rGPS->SetReqCode((DBCODE)rCode);
  rGPS->SetOFS(0);
  return;
}

//------------------------------------------------------------------
//  Launch a request to get next match
//------------------------------------------------------------------
void CK89gps::GetNextMatch(K89_LETFD *fd,char opt,char dir)
{ GetFilterArg(LetFD);
  rGPS->ClearFilter();
  if (fd->Iden == 2)  rGPS->SetST1(Arg);
  if (fd->Iden == 1)  rGPS->SetST2(Arg);
  rGPS->SetReqCode((DBCODE)rCode);
  if (opt)  rGPS->SetOFS(0);
  rGPS->SetDirection(dir);
  return;
}
//------------------------------------------------------------------
//  return the strip index
//------------------------------------------------------------------
U_CHAR CK89gps::GetStripIndex(char let)
{ if ( ' ' == let)                  return 0;
  if (('0' <= let) && ('9' >= let)) return (U_CHAR)((let - '0') + K89_0_INDEX);
  if (('A' <= let) && ('Z' >= let)) return (U_CHAR)((let - 'A') + K89_A_INDEX);
  return 0;
}
//------------------------------------------------------------------
//  Edit Time zone based on longitude
//------------------------------------------------------------------
int CK89gps::EditTimeZone(char *edt,float lon)
{ char *tz = globals->tmzTAB[TimZON].tzName;
  if (lon > (180 * 3600)) lon -= (360 * 3600);
  int dt = int (lon / (15 * 3600));
  _snprintf(edt,24,"UTC%+02d",dt);
  return dt;
}
//------------------------------------------------------------------
//  Set Route Mode
//------------------------------------------------------------------
void CK89gps::ModeLEG()
{ StoreText("Leg",K89_LINE2,K89_CLN03);
  wOBS  = actRTE;
  return;
}
//------------------------------------------------------------------
//  Set OBS Mode
//------------------------------------------------------------------
void CK89gps::ModeOBS()
{ char  edt[8];
  float obs = (BUS)?(float(BUS->xOBS)):(0);               // Get current OBS
  wOBS  = obs;
  int vobs   = Round(obs);
  _snprintf(edt,8,"%03u",vobs);
  StoreText(edt,K89_LINE2,K89_CLN03);
  return;
}
//-----------------------------------------------------------------------------
//  DUTY TASK called at each cycle
//  Refresh Active Waypoint
//  -Compute distance and radial
//-----------------------------------------------------------------------------
void CK89gps::RefreshActiveWPT()
{ char  edt[16];
  const char *idn = (0 == actWPT)?("____"):(actWPT->GetIdent());
  if (K89_MSGP1 == aState)  return;
	//--- Set Mode ---------------------------------
	(modOBS)?(ModeOBS()):(ModeLEG());
  //------Refresh left screen part ---------------
  EditDistance(edt,actDIS);
  Prop  = K89_ATT_NBOLD;
  StoreText(edt,K89_LINE0,K89_CLN00);
  _snprintf(edt,8,"%-4s",idn);
  StoreText(edt,K89_LINE1,K89_CLN00);
  //-----MSG alert ------------------------------
  if (msgNO)  wrnNO = 2;
  FlashWord(wrnTAB[wrnNO],K89_LINE2,K89_CLN00);
	return;
}
//-----------------------------------------------------------------------------
//  DUTY TASK called at each cycle
//  Refresh VNAV waypoint
//  -Compute distance and radial
//-----------------------------------------------------------------------------
void CK89gps::RefreshVNAVpoint()
  { if (vnaWPT.IsNull())  return;
    if (-1 == vnaWNO)
    { vnaWPT->Refresh(FrameNo);
      vnaDIS  = vnaWPT->GetNmiles();
    }
    else    vnaDIS = 0;			
    switch (vState) {
      case K89_VNA_OFF:
          if (K89_MSG_VNA == msgNO) msgNO = K89_MSG_OFF;
          return;
      case K89_VNA_ALT:
        if (abs(aPos.alt - AltiTG) > 100)  return;
        vState = K89_VNA_OFF;
        StoreText("   ",K89_LINE2,K89_CLN00);
        return;
      case K89_VNA_VSI:
        vTime  = GetTimeToVNAV();
        if (vTime <= -1)    vState = K89_VNA_OFF;
        if (vTime <= +1)    vState = K89_VNA_ALT;
        if (vTime > 90)               return;
        //------MSG Vnv Alert -----------------------------------
        if (msgNO > K89_MSG_VNA)      return;
        msgNO = K89_MSG_VNA;
    }
  return;
}
//-----------------------------------------------------------------------------
//  Edit Distance 
//-----------------------------------------------------------------------------
void CK89gps::EditDistance(char *edt,float dis)
{ if ((-1 == dis) || (dis > 9999)) strncpy(edt,"__._nm",16);
  else  if (dis >= 1000)   _snprintf(edt,16,"%.0fnm",  dis);
  else  if (dis >=  100)   _snprintf(edt,16," %.0fnm", dis);
  else  if (dis >=   10)   _snprintf(edt,16,"%.1fnm",  dis);
  else                     _snprintf(edt,16," %.1fnm", dis);
  return;
}
//-----------------------------------------------------------------------------
//  Round to 100 units
//-----------------------------------------------------------------------------
int CK89gps::Round100(int val)
{ int p1 = val / 100;
  int p2 = val % 100;
  if (p2 > 50)  p1++;
  return (p1 * 100);
}
//-----------------------------------------------------------------------------
//  Edit Radial or bearing
//-----------------------------------------------------------------------------
void CK89gps::EditRadial(char *dst,float rad,U_CHAR flag)
{ if (K89_FLAG_FROM == flag)  rad = Wrap360(rad + 180);
  int dir   = Round(rad);
  if (K89_FLAG_NONE == flag)  strncpy(dst,".FLAG.",16);
  else _snprintf(dst,16,"%03u°%s",dir,Flag[flag]);
  return;
}
//-----------------------------------------------------------------------------
//  Edit Radial and distance in line 3
//-----------------------------------------------------------------------------
int CK89gps::EditCAPandDIS(CmHead *obj)
{ char edt[16];
  //-------Edit Bearing ----------------------------------------
  obj->Refresh(FrameNo);
  EditRadial(edt,obj->GetRadial(),OpFlag);
  StoreText(edt,K89_LINE3,K89_CLN07);
  //------Edit Distance ----------------------------------------
  EditDistance(edt,obj->GetNmiles());
  StoreText(edt,K89_LINE3,K89_CLN15);
  return 1;
}
//--------------------------------------------------------------------
//  Edit  NAV cap or radial
// (9 characters wide for consistency with VNAV option)
//--------------------------------------------------------------------
int CK89gps::EditBearing(float brg,short lin,short col)
{ char edt[16];
  float cap = (K89_FLAG_FROM == OpFlag)?(Wrap360(brg + 180)):(brg);
  int dir   = Round(cap);
  if (K89_FLAG_NONE == OpFlag) strncpy(edt,"___°To   ",16);
  else                         _snprintf(edt,16,"%03u°%s   ",dir,Flag[OpFlag]);
  StoreText(edt,lin,col);
  return 1;
}
//-----------------------------------------------------------------------------
//  Edit Lattitude
//-----------------------------------------------------------------------------
void CK89gps::EditLatitude(float lat,short ln,short cl)
{ char edt[16];
  const char *pole  = (lat < 0)?("S"):("N");
  long  val   = (lat < 0)?(long(-lat * 100)):(long(lat *100));
  long  deg   =  0;
  float min   =  0;
  deg   = (val / 360000);
  val   = (val % 360000);
  min   = (float(val) / 6000);
  _snprintf(edt,16,"%s %3u° %2.2f'",pole,int(deg),min);
  StoreText(edt,ln,cl);
  return;
}
//-----------------------------------------------------------------------------
//  Edit longitude
//-----------------------------------------------------------------------------
void CK89gps::EditLongitude(float lon,short ln,short cl)
{ char edt[16];
  if (lon > (180 * 3600)) lon -= (360 * 3600);
  const char *meri  = (lon < 0)?("W"):("E");
  long  val   = (lon < 0)?(long(-lon * 100)):(long(lon * 100));
  long  deg   =  0;
  float min   =  0;
  deg   = (val / 360000);
  val   = (val % 360000);
  min   = (float(val) / 6000);
  _snprintf(edt,16,"%s %3u° %2.2f'",meri,int(deg),min);
  StoreText(edt,ln,cl);
  return;
}
//-----------------------------------------------------------------------------
//  Edit Magnetic deviation
//-----------------------------------------------------------------------------
void CK89gps::EditMagDev(float dev,short lin,short col)
{ char edt[16];
  const char *dir = (dev < 0)?("W"):("E");
  int   mag = int(dev);
  mag   = abs(mag);
  _snprintf(edt,16,"Mag Var %s % 3u°",dir,mag);
  StoreText(edt,lin,col);
  return;
}
//-----------------------------------------------------------------------------
//  Edit fuel qty
//-----------------------------------------------------------------------------
void CK89gps::EditFuel()
{ CFuelSystem     *gas = mveh->gas;
  char  edt[16];
  float qty;
  float cap;
  gas->GetTotalGas(cap,qty,cap);
  _snprintf(edt,16,"%04u GAL",int(qty));
  StoreText(edt,      K89_LINE1,K89_CLN10);
  StoreText("FULL ?", K89_LINE2,K89_CLN10);
  return;
}
//-----------------------------------------------------------------------------
//  Edit Track
//  wOBS         = desired track DTK
//  Aircraft CAP = Actual TRACK   TK
//-----------------------------------------------------------------------------
void CK89gps::EditTrack()
{ char edt[16];
  int     dtk = Round(wOBS);        // Desired track
  int     atk = Round(aCAP);        // Actual bearing
  if (cdiST == K89_FLAG_NONE)  strncpy(edt,"___°",16);
  else                        _snprintf(edt,16,"%03u°",dtk);
  StoreText(edt,K89_LINE2,K89_CLN10);
  if (cdiST == K89_FLAG_NONE)  strncpy(edt,"___°",16);
  else                        _snprintf(edt,16,"%03u°",atk);
  StoreText(edt,K89_LINE2,K89_CLN18);
	U_SHORT *test= disLN[K89_LINE2] + K89_CLN16;
  return ;
}
//----------------------------------------------------------------------------
//  Clear Display
//----------------------------------------------------------------------------
void CK89gps::RazDisplay()
{ U_SHORT *des = Display;
  short    nbr = 0;
  while (K89_NBR_CHAR != nbr++) *des++ = (U_SHORT)(K89_ATT_CLEAR);
  cdiCL = 0;
  Flash = 0;
  return;
}
//----------------------------------------------------------------------------
//  Clear right part of Display
//----------------------------------------------------------------------------
void CK89gps::ClearRightDisplay()
{ U_SHORT  *des = 0;
  short     lin = 0;
  short     col = 0;
  while (K89_NBR_LINE != lin) 
  { des = disLN[lin++] + K89_CLN06;
    col = 0;
    while (K89_PR2_COLN != col++) *des++ = (U_SHORT)(K89_ATT_CLEAR);
  }
  cdiCL = 0;
  return;
}
//----------------------------------------------------------------------------
//  Clear a line
//----------------------------------------------------------------------------
void CK89gps::ClearRightLine(int lin)
{ U_SHORT  *des = disLN[lin] + K89_CLN06;;
  for (int k=0; k !=K89_PR2_COLN; k++) *des++ = (U_SHORT)(K89_ATT_CLEAR);
  return;
}
//---------------------------------------------------------------------------
//  Flash at given position
//----------------------------------------------------------------------------
void CK89gps::FlashON(U_SHORT *pos)
{ U_SHORT car = (*pos & K89_ATT_MASKC) | K89_ATT_FLASH;
  *pos  = car;
  Flash = pos;
  return;
}
//---------------------------------------------------------------------------
//  Flash OF at given position
//----------------------------------------------------------------------------
void CK89gps::FlashOF(U_SHORT *pos)
{ if (0 == pos) return;
  U_SHORT car = (*pos & K89_ATT_MASKC) | K89_ATT_YELOW;
  *pos  = car;
  return;
}
//---------------------------------------------------------------------------
//  Clear field attribute 
//---------------------------------------------------------------------------
void CK89gps::SetFlashOF(short lin, short col,short siz)
{ U_SHORT *pos = disLN[lin] + col;
  while (siz--) *pos = *pos++ & K89_ATT_MASKC;
  return;
}
//---------------------------------------------------------------------------
//  Set field attribute 
//---------------------------------------------------------------------------
void CK89gps::SetFlashON(short lin, short col,short siz)
{ U_SHORT *pos = disLN[lin] + col;
  while (siz--) *pos = (*pos++ & K89_ATT_MASKC) | K89_ATT_FLASH;
  return;
}
//---------------------------------------------------------------------------
//  StoreText with flash and memorize flash parameters
//---------------------------------------------------------------------------
void CK89gps::FlashText(char *txt,U_SHORT *pos)
{ Flash = pos;
  while (*txt) *pos++ = (*txt++ | K89_ATT_FLASH);
  Flsiz = (U_SHORT)(pos - Flash);
  return;
}
//---------------------------------------------------------------------------
//  Reflash the previous field
//---------------------------------------------------------------------------
void CK89gps::ReFlash()
{ U_SHORT *pos = Flash;
  U_SHORT  nb  = Flsiz;
  while (nb--) *pos++ = (*pos | K89_ATT_FLASH);
  return;
}
//---------------------------------------------------------------------------
//  Remove flash text from display
//---------------------------------------------------------------------------
void CK89gps::FlashOFF()
{ U_SHORT *pos = Flash;
  U_SHORT  nb  = Flsiz;
  while (nb--) *pos = (*pos++ ^ K89_ATT_FLREQ);
  return;
}
//---------------------------------------------------------------------------
//  Move text to Display at line and column
//---------------------------------------------------------------------------
void CK89gps::StoreText(const char *txt,short lin,short col)
{ U_SHORT *des =  disLN[lin] + col;
  while (*txt) *des++ = (*txt++ & 0xFF) | Prop;
  Prop  = K89_ATT_NONE;
  return;
}
//---------------------------------------------------------------------------
//  Flash Text to display at line and column
//----------------------------------------------------------------------------
void CK89gps::FlashWord(char *txt,short lin,short col)
{ U_SHORT *des =  disLN[lin] + col;
  while (*txt) *des++ = (*txt++ & 0xFF) | K89_ATT_FLASH;
  Prop  = K89_ATT_NONE;
  return;
}
//---------------------------------------------------------------------------
//  Move text to Display iat position
//---------------------------------------------------------------------------
void CK89gps::StoreText(U_SHORT *des,char *org)
{ while (*org) *des++ = (U_CHAR)(*org++) | Prop;
  Prop = K89_ATT_NONE;
  return;
}
//---------------------------------------------------------------------------
//  Check if obj is the active waypoint
//---------------------------------------------------------------------------
bool CK89gps::IsActiveWPT(CmHead *obj)
{ if ((0 == actWPT) || (0 == obj))                  return false;
  CmHead *wpt = actWPT;
  if (strcmp(obj->GetIdent(), wpt->GetIdent()))     return false;
  if (obj->GetLatitude()  !=  wpt->GetLatitude())   return false;
  if (obj->GetLongitude() !=  wpt->GetLongitude())  return false;
  return true;
}
//----------------------------------------------------------------------------
//  Store a character in display
//----------------------------------------------------------------------------
void CK89gps::StoreChar(char car,short lin,short col)
{ U_SHORT *des = disLN[lin] + col;
  *des  = (car & 0x00FF) | Prop;
  Prop  = K89_ATT_NONE;
  return;
}
//----------------------------------------------------------------------------
//  Store a character in display and flash if cursor curPOS = pos
//----------------------------------------------------------------------------
void CK89gps::FlashChar(char car,short lin, short col, short cp)
{ U_SHORT *pos = disLN[lin] + col;
  U_SHORT  att = (cp == curPOS)?(K89_ATT_FLASH):(K89_ATT_YELOW);
  *pos  = (att | car);
  return;
}
//----------------------------------------------------------------------------
//  Store n characters in display
//----------------------------------------------------------------------------
void CK89gps::StoreBlnk(U_CHAR nbr,short lin,short col)
{ U_SHORT *des = disLN[lin] + col;
  U_SHORT  car = K89_ATT_CLEAR;
  while (nbr--) *des++  = car;
  return;
}
//---------------------------------------------------------------------------
//  Check for space at given position
//---------------------------------------------------------------------------
bool CK89gps::IsChar(char car,short lin,short col)
{ U_SHORT *pos = disLN[lin] + col;
  return (*pos == car);
}
//-----------------------------------------------------------------------------
//  Drawing
//-----------------------------------------------------------------------------
//  Draw a CDI at indicated position
//  Deviation is computed as proportional to +- 20°
//-----------------------------------------------------------------------------
int CK89gps::DrawCDI(short lin,short col)
{ StoreText("...........",lin,col);
  StoreChar(cdiIN[cdiST],lin,col + 5); 
  if (0 == cdiST) 
  { StoreText("F.L.A.G",lin,col + 2);
    return 0;
  }
  cdiLN = lin;
  cdiCL = (col - K89_CLN06);
  return 1;
}
//-----------------------------------------------------------------------------
//  Copy display
//-----------------------------------------------------------------------------
void CK89gps::CopyDisplay(U_SHORT *src,U_SHORT *des)
{ int nbr = 24 * 4;
  while (nbr--) *des++ = *src++;
  return;
}
//-----------------------------------------------------------------------------
//  Time slice routine 
//-----------------------------------------------------------------------------
void  CK89gps::TimeSlice (float dT,U_INT FrNo)
{ CDependent::TimeSlice(dT,FrNo);
	GPSRadio::UpdateTracking(dT,FrNo);
	//-----Update Power status ------------------------------
  powr   = active;
  if (0 == powr)   {PowerCUT(); return;}
  //-----Update flasher -----------------------------------
  mskFS = K89FLASH[globals->clk->GetON()];
  aPos  = mveh->GetPosition();					//globals->geop;
  //-----Refresh active waypoint --------------------------
  if (aState >= K89_APTP1)
  { UpdNavigationData(actWP);
    RefreshActiveWPT();
    RefreshVNAVpoint();
  }
  //-----Call End Of Request Event --------------------------------------------
  if (rGPS->EndOfReq()) 
	{		rGPS->PushCode();												// Push previous code
		 (this->*sfn[aState])(K89_EOR);						// Call EOR routine
	}
  //-----Call the Clock and Beat Events ---------------------------------------
  (this->*sfn[aState])(K89_CLOCK);
  return;
}
//--------------------------------------------------------
//  Display data
//--------------------------------------------------------
void CK89gps::Probe(CFuiCanva *cnv)
{ char  edt[64];
	char *act = (actWPT)?(actWPT->GetIdent()):("____");
	CDependent::Probe(cnv,0);
	cnv->AddText(1,1,"Mode  :%s",modeK89[Mode]);
	cnv->AddText(1,1,"actWPT:%s",act);
  _snprintf(edt,32,"wOBS: %03d",int(wOBS));
  cnv->AddText( 1,edt,1);
  _snprintf(edt,32,"aCAP: %03d",int(aCAP));
  cnv->AddText( 1,edt,1);
	GPSRadio::Probe(cnv);
  return;
}
//==============================================================================================
//	INTERFACE WITH REAL GPS
//==============================================================================================
//---------------------------------------------------------------------
//	Get selected node in flight plan
//	
//---------------------------------------------------------------------
CWPoint *CK89gps::SelectedNode()
{ if (aState != K89_FPLP1)							return 0;
	if ((curPOS < 1) || (curPOS > 4))     return 0;
  if (IsChar(' ',(curPOS-1),K89_CLN09)) return 0;
	if (0 == basWP)												return 0;
	//--- Compute sequence number -------------------
	int seq = basWP->GetSequence() + curPOS - 1;
  return  FPL->GetWaypoint(seq);
}
//---------------------------------------------------------------------
//  Flight Plan is modified
//	Notification coming from current flight plan
//---------------------------------------------------------------------
void CK89gps::ModifiedPlan()
{	FPL     = mveh->GetFlightPlan();
	Mode		= GPS_MODE_FPLAN;
	actWP		= FPL->GetActiveNode();
	basWP		= FPL->HeadNode();									// Base waypoint
  fpMax   = FPL->Size();
	TrackWaypoint(actWP,false);
	return;
}
//----------------------------------------------------------------------------------
//  Enter tracking
//	Autopilot must be engaged
//	Then
//	Activate Plan if any and get first node to track
//----------------------------------------------------------------------------------
void GPSRadio::EnterTRK()
{	CWPoint *wp = SelectedNode();
	if (0 == wp)									return;
	if (GPSR_STBY != gpsTK)				return;   // Not the good state
	if (!APL->EnterGPSMode())			return;
	//--- Stop current plan and start new one ------
	FPL->StopPlan();
 	if (!FPL->StartPlan(wp))			return;
	APL->	SetGasControl(1);
	//--- Set Tracking mode -------------------------
	navON	= 1;
	gpsTK	= GPSR_TRAK;
	//---Get first waypoint to track ----------------
	SetTrack();
	return;
}

//---------------------------------------------------------------------
//  Active waypoint is modified
//	Notification coming from active flight plan
//	NOTE when a direct TO waypoint is terminated then the next waypoint 
//			is:
//	Same waypoint if the waypoint is not part of the flight plan
//	Otherwise, the next waypoint from the flight plan or 0
//
//---------------------------------------------------------------------
void CK89gps::TrackWaypoint(CWPoint *wpt,bool endir)
{	actWP	= wpt;														// Save it
	UpdNavigationData(wpt);
	prvIDN		= FPL->PreviousIdent(wpt);
	insDIS		= FPL->GetInDIS();
	//--- Check for end of DIRECT TO mode -------------
	if (endir)	PushMode(GPS_MODE_FPLAN);
	//--- Now assign the active waypoint --------------
	actWPT = (wpt)?(wpt->GetDBobject()):(0);
	if (0 == wpt)		return;									// Ignore it
	//--- first inhibit user scrol mode ---------------
	if (aState == K89_FPLP1)	curPOS	= 0;
	if (0 == basWP)	return;
	//--- check if base waypoint is still OK ----------
	int asq = wpt->GetSequence();
	int bsq = basWP->GetSequence();
	//-- check if active is in range of base ----------
	bool ok = (asq >= bsq) &&  (asq < (bsq+3));
	if (ok)					return;			//Still visible
	//--- Check for scrolling one position ------------
	ok = (asq == (bsq + 3));
	if (ok) {basWP = FPL->NextNode(basWP); return;}
	//--- Change base waypoint ------------------------
	basWP	= FPL->BaseWPT(basWP);
	return;
}
//------------------------------------------------------------------
//  Refresh Navigation data
//------------------------------------------------------------------
void CK89gps::UpdNavigationData(CWPoint *wpt)
{ Speed			= mveh->GetPreCalculedKIAS();
	aCAP			= mveh->GetMagneticDirection();
	actRTE		= (wpt)?(wpt->GetDTK()):(0);  
	actRAD		= (wpt)?(wpt->GetCAP()):(0);
	actDIS		= (wpt)?(wpt->GetPlnDistance()):(-1);
  return;
}
//------------------------------------------------------------------
//  Navigation is now active
//------------------------------------------------------------------
void CK89gps::NavIsActive()
{	EnterNAVpage01();
}
//=====================================================================================
//	GPSR State table
//=====================================================================================
char *gpsrETA[]= {
	"PWOF",					// 0 Power off
	"STBY",					// 1 Standby
	"TRAK",					// 2 Tracking
	"LAND",					// 3 Landing
	"NONE",					// 4 Out of Service
};
//=====================================================================================
// GPSRadio susbsytem
//	To interface all GPS
//=====================================================================================
GPSRadio::GPSRadio (void)
{ TypeIs (SUBSYSTEM_GPS_RADIO);
	BUS	  = 0;
	FPL		= 0;
	APL		= 0;
	RAD		= 0;
	pln		= 0;
	basWP	= 0;
	actWP	= 0;
	//--------------------------------
	wTRK	= 0;			// Tracked waypoint
	//--------------------------------
	gpsTK	= GPSR_PWOF;
	navON	= 0;
	aprON	= 0;
}
//-------------------------------------------------------------------------------------
//  Free resources
//-------------------------------------------------------------------------------------
GPSRadio::~GPSRadio()
{ }
//-------------------------------------------------------------------------------------
//  Read all tags
//-------------------------------------------------------------------------------------
int GPSRadio::Read (SStream *stream, Tag tag)
{switch (tag) {
  case 'mFMS':
    // FMS message
    ReadMessage (&mFMS, stream);
    return TAG_READ;

  case 'mSpd':
    // Airspeed message
    ReadMessage (&mSpd, stream);
    return TAG_READ;
  }
    // See if the tag can be processed by the parent class type
 return CDependent::Read (stream, tag);
}
//----------------------------------------------------------------------------------
//  All tags are read
//----------------------------------------------------------------------------------
void GPSRadio::ReadFinished (void)
{ // Fill in common message fields
  mFMS.id = MSG_GETDATA;
  mSpd.id = MSG_GETDATA;
  CDependent::ReadFinished ();
}
//----------------------------------------------------------------------------------
//  Tracking EVENT
//----------------------------------------------------------------------------------
void GPSRadio::TrackGPSEvent(U_CHAR evn,char parm)
{	switch (evn)	{
	//--- Power event ----------------
		case GPSR_EV_PWR:
			PowerEVN(parm);
			return;
	}
	return;
}
//----------------------------------------------------------------------------------
//  Power EVENT
//----------------------------------------------------------------------------------
void GPSRadio::PowerEVN(char parm)
{	navON	= 0;
	aprON	= 0;
	//--- Check for power ON -----------------
	bool of =  (gpsTK == GPSR_PWOF);
	bool pw =  (parm != 0);
	if (of && pw)	return PowerON();
	//--- Check for power off ----------------
	if (pw)				return;
	gpsTK = GPSR_PWOF;
	if (RAD) RAD->ModeEXT(0);
	return;
}
//----------------------------------------------------------------------------------
//  Switch EVENT
//----------------------------------------------------------------------------------
void GPSRadio::SwitchNAV(char parm)
{	//--- Change state -----------------------------------
	if (parm)			EnterTRK();
	else					EnterSBY();
	return;
}
//----------------------------------------------------------------------------------
//  Switch EVENT
//----------------------------------------------------------------------------------
void GPSRadio::SwitchAPR(char parm)
{	if (0 == navON)	return;			// Power OFF ignore
	//--- Change state -----------------------------------
	aprON	= parm;
	return;
}

//----------------------------------------------------------------------------------
//  Power ON
//----------------------------------------------------------------------------------
void GPSRadio::PowerON()
{	gpsTK = GPSR_STBY;
	pln		= (CAirplane*)mveh;
	APL	  = pln->GetAutoPilot();
	RAD		= pln->GetMRAD();
  BUS   = mveh->GetRadioBUS();
	FPL   = mveh->GetFlightPlan();	
	FPL->ClearDirect();
	bool ok = (APL) && (BUS);
	if (!ok)	gpsTK	= GPSR_NONE;
	return;
}

//----------------------------------------------------------------------------------
//  Get next Waypoint
//	Set RADIO BUS to EXTERNAL SOURCE
//----------------------------------------------------------------------------------
void GPSRadio::SetTrack()
{ float rad = 0;
	wTRK	= FPL->GetActiveNode();
	if (0 == wTRK)						return;
	if (wTRK->IsFirst())			return;  // wait next
	if (FPL->IsOnFinal())			return EnterAPR();
	//--- Set Waypoint On External Source ------------------
	float   dir = SelectDirection();
	CmHead *obj = wTRK->GetDBobject();
  RAD->ModeEXT(obj);								// Set EXT mode
	RAD->ChangeRefDirection(dir);
	//--- Configure autopilot ------------------------------
	double alt = double(wTRK->GetAltitude());
	APL->SetWPTmode(alt);
	return;
}
//--------------------------------------------------------------
//	Compute direction
//	If leg distance is under 12 miles, head direct to station
//  or if deviation is too much
//--------------------------------------------------------------
float GPSRadio::SelectDirection()
{	float dis = wTRK->GetLegDistance();
  float seg = wTRK->GetDirection();
	float dev = RAD->GetDeviation();
	float rdv = fabs(dev);
	if ((dis > 12) || (rdv < 5))	return seg;
	//--- Compute direct-to direction to waypoint -----
  return wTRK->GoDirect(mveh);
}
//--------------------------------------------------------------
//	Refresh direction to waypoint if needed
//	Correct any drift due to long legs
//--------------------------------------------------------------
void GPSRadio::Refresh()
{	float dev = RAD->GetDeviation();
	float rdv = fabs(dev);
	bool  dto = ((rdv > 5) || (wTRK->IsDirect()));
	//--- check if Direct to is active ----------
	if (!dto)	return; 
  float dir = wTRK->GoDirect(mveh);
	RAD->ChangePosition(wTRK->GetGeoP());
	RAD->ChangeRefDirection(dir);
	return;
}
//--------------------------------------------------------------
//	Go back to standby mode
//	Relax Radio BUS
//	Go back to radio mode
//	Let AUTOPILOT in current state
//--------------------------------------------------------------
void GPSRadio::EnterSBY()
{	if (GPSR_PWOF == gpsTK)		return;
	if (GPSR_NONE == gpsTK)		return;
	if (GPSR_STBY == gpsTK)		return;
	gpsTK	= GPSR_STBY;
	navON	= 0;
	aprON = 0;
	if (RAD) RAD->ModeEXT(0);
	return;
}
//--------------------------------------------------------------
//	Enter final mode
//	Final may use 
//	-ILS radio station
//  -GPS waypoint
// TODO:  Check for APR GPS button
//--------------------------------------------------------------
void GPSRadio::EnterAPR()
{ //--- Check  if APR allowed ---------------------------
	if (0 == aprON)								return EnterSBY();
	//--- Configure Landing mode --------------------------
	if (!wTRK->EnterLanding(RAD))	return EnterSBY();
	//--- Configure autopilot for landing ------------------
	APL->SetLandingMode();
	gpsTK = GPSR_LAND;
	return;
}
//--------------------------------------------------------------
//	Refresh tracking
//--------------------------------------------------------------
void GPSRadio::UpdateTracking(float dT,U_INT frm)
{	switch (gpsTK)	{
		case GPSR_PWOF:
			navON	= 0;
			aprON	= 0;
			return;
		//--- Standby mode: Nothing to do ----------
		case GPSR_STBY:
			return;
		//--- Tracking active waypoint -------------
		case GPSR_TRAK:
			if (APL->IsDisengaged())	EnterSBY();
			if (0 == wTRK)						return;
			if (wTRK->IsActive())	    return Refresh();
			SetTrack();
			return;
		//--- Just watch the auto pilot ------------
		case GPSR_LAND:
			if (APL->IsDisengaged())	EnterSBY();
			if (APL->ModeGround())	  return EnterSBY();
			return;
		case GPSR_NONE:
			return;
	}
	//-------------------------------------------
	return;
}
//--------------------------------------------------------------
//	Probe values
//--------------------------------------------------------------
void GPSRadio::Probe(CFuiCanva *cnv)
{	cnv->AddText(1,1,"GPSR  :%s",gpsrETA[gpsTK]);
  if (0 == wTRK)	return;
	cnv->AddText(1,1,"active:%s",wTRK->GetIdentity());
	return;
}
//----------------------------------------------------------------------------------
//  Message from external 
//	NOTE: Gauge should send the nav switch position
//----------------------------------------------------------------------------------
EMessageResult GPSRadio::ReceiveMessage (SMessage *msg)
{   switch (msg->id) {
    case MSG_GETDATA:
      switch (msg->user.u.datatag) {
      //--- Supply current request ------------------
      case 'req_':
        return MSG_PROCESSED;
      }
      break;
		//---- Set data ----------------------------------
    case MSG_SETDATA:
      switch(msg->user.u.datatag) {
      case 'pwon':
        return MSG_PROCESSED;

      case 'poff':
        return MSG_PROCESSED;

			//--- Flight plan is modified -------
			case 'pmod':
				ModifiedPlan();
				return MSG_PROCESSED;
			//--- Set The GPS Slave mode --------
			case 'navp':
				SwitchNAV(msg->intData);
				return MSG_PROCESSED;
			//--- SetAPR allowed ----------------
			case 'aprp':
				SwitchAPR(msg->intData);
				return MSG_PROCESSED;
			}
}
  return CDependent::ReceiveMessage (msg);
}
//=======================END OF FILE ================================================