/*
 * Database.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2007 Chris Wallace
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

/*! \file Database.cpp
 *  \brief Implements application API to Fly! databases
 */

#include "../Include/FlyLegacy.h"
#include "../Include/Database.h"
#include "../Include/Globals.h"
#include "../Include/Endian.h"
#include "../Include/Fui.h"
#include "../Include/FuiParts.h"
#include "../Include/Utility.h"
#include "../Include/WorldObjects.h"
#include "../Include/SqlMGR.h"
#include "../Include/TerrainCache.h"
#include "../Include/Weather.h"
#include "../Include/AudioManager.h"
#include "../include/PlanDeVol.h"
#include "../Include/MagneticModel.h"
using namespace std;
//=====================================================================
extern double rwyATHR[];
//=====================================================================
// Forward declaration of local functions
std::map<int,std::string> g_vecAirportTypeS;
std::map<int,std::string> g_vecNavaidTypeS;
std::map<int,std::string> g_vecComTypeS;
std::map<int,std::string> g_vecILSTypeS;

std::map<int,std::string> g_vecWaypointUsageS;
//====================================================================
RD_COM GetComINDEX(U_INT mask);
//====================================================================
void InitTypeDBTranslation()
{
  std::string s;
  s.reserve(24);
  s = "Unknown"     ; g_vecAirportTypeS[0]=s;
  s = "Airport"     ; g_vecAirportTypeS[1]=s;
  s = "Airport"     ; g_vecAirportTypeS[2]=s;
  s = "Heliport"    ; g_vecAirportTypeS[4]=s;
  s = "Seaplane"    ; g_vecAirportTypeS[8]=s;
  s = "Balloon"     ; g_vecAirportTypeS[16]=s;
  s = "Glider"      ; g_vecAirportTypeS[32]=s;
  s = "Arpt/glider" ; g_vecAirportTypeS[34]=s;
  s = "Arpt/para"   ; g_vecAirportTypeS[48]=s;
  s = "Stolport"    ; g_vecAirportTypeS[64]=s;
  s = "Ultralight"  ; g_vecAirportTypeS[128]=s;
  s = "Arpt/light" ; g_vecAirportTypeS[130]=s;
  s = "Arpt/light/glider" ; g_vecAirportTypeS[162]=s;

  s = "Unknown"       ; g_vecNavaidTypeS[0]=s;
  s = "VOR"           ; g_vecNavaidTypeS[1]=s;
  s = "TACAN"         ; g_vecNavaidTypeS[4]=s;
  s = "VOR/TACAN"     ; g_vecNavaidTypeS[5]=s;
  s = "NDB"           ; g_vecNavaidTypeS[8]=s;
  s = "DME"           ; g_vecNavaidTypeS[16]=s;
  s = "VOR/DME"       ; g_vecNavaidTypeS[17]=s;
  s = "Locator"       ; g_vecNavaidTypeS[32]=s;
  s = "Localizer"     ; g_vecNavaidTypeS[64]=s;
  s = "Localizer/DME" ; g_vecNavaidTypeS[80]=s;
  s = "Glide"         ; g_vecNavaidTypeS[128]=s;
  s = "BackCourse"    ; g_vecNavaidTypeS[256]=s;
  s = "InnerMarker"   ; g_vecNavaidTypeS[512]=s;
  s = "MiddleMarker"  ; g_vecNavaidTypeS[1024]=s;
  s = "OuterMarker"   ; g_vecNavaidTypeS[2048]=s;
  s = "Waypoint"      ; g_vecNavaidTypeS[4096]=s;
  s = "VOT"           ; g_vecNavaidTypeS[8192]=s;

  s = "Unknown"   ; g_vecComTypeS[0]=s;
  s = "TWR"       ; g_vecComTypeS[1]=s;
  s = "GND"       ; g_vecComTypeS[4]=s;
  s = "APP"       ; g_vecComTypeS[8]=s;
  s = "DEP"       ; g_vecComTypeS[16]=s;
  s = "APP/DEP"   ; g_vecComTypeS[24]=s;
  s = "RDO"       ; g_vecComTypeS[32]=s;
  s = "ATIS"      ; g_vecComTypeS[128]=s;
  s = "TAF"       ; g_vecComTypeS[256]=s;
  s = "Multicom"  ; g_vecComTypeS[512]=s;
  s = "Emergency" ; g_vecComTypeS[1024]=s;
 
  s = "Unknown"         ; g_vecILSTypeS[0]=s;
  s = "DME"             ; g_vecILSTypeS[16]=s;
  s = "Locator"         ; g_vecILSTypeS[32]=s;
  s = "Localizer"       ; g_vecILSTypeS[64]=s;
  s = "Glideslope"      ; g_vecILSTypeS[128]=s;
  s = "BackCourseMarker"; g_vecILSTypeS[256]=s;
  s = "InnerMarker"     ; g_vecILSTypeS[512]=s;
  s = "MiddleMarker"    ; g_vecILSTypeS[1024]=s;
  s = "OuterMarker"     ; g_vecILSTypeS[2048]=s;

  s = "Unknown"    ; g_vecWaypointUsageS[0]=s;
  s = "High route" ; g_vecWaypointUsageS[1]=s;
  s = "Low route"  ; g_vecWaypointUsageS[2]=s;
  s = "High/Low"   ; g_vecWaypointUsageS[3]=s;
  s = "Terminal"   ; g_vecWaypointUsageS[4]=s;
  s = "High/RNAV"  ; g_vecWaypointUsageS[5]=s;
}
//=======================================================================
//  Radio Tag
//======================================================================
Tag RadioIND[] =
{             0,					          // 0 Tile cache
				      0,					          // 1 Airport
				      NAV_INDEX,	          // 2 VOR
				      ADF_INDEX,            // 3 NDB
				      NAV_INDEX,			      // 4 ILS
				      COM_INDEX,	          // 5 Commm radio
};
//======================================================================
void CleanupTypeDBTranslation()
{
  g_vecAirportTypeS.clear();
  g_vecNavaidTypeS.clear();
  g_vecComTypeS.clear();
  g_vecILSTypeS.clear();
  g_vecWaypointUsageS.clear();
}

std::string & DBTypeString(std::map<int,std::string> & map, int key)
{
  std::map<int,std::string>::iterator it = map.find(key);
  if(it != map.end())
    return it->second;
  else
    return map[0]; // Unknown
}

//
// Type field translation for some Fly databases 
//
std::string & DBAirportType(int iType)
{
  return DBTypeString(g_vecAirportTypeS, iType);
}

std::string & DBNavaidType(int iType)
{
  return DBTypeString(g_vecNavaidTypeS, iType);
}
std::string & DBComType(int iType)
{
  return DBTypeString(g_vecComTypeS, iType);
}
std::string & DBILSType(int iType)
{
  return DBTypeString(g_vecILSTypeS, iType);
}
//
// Usage field translation for some Fly databases
//
std::string & DBWaypointUsage(int iUsage)
{
  return DBTypeString(g_vecWaypointUsageS, iUsage);
}

//
// Decoder functions to convert database binary record into SDK structures
//
// The following functions:
//   DecodeNavaidRecord
//   DecodeAirportRecord
// are used to convert raw binary database records into the appropriate
// struct types.  These standard C types are used to export database data
// to the external APIs such as the SDK .
//

static void DecodeNavaidRecord (char* rec, SNavaid *navaid)
{
  strncpy (navaid->name, rec+19, 40);
  strncpy (navaid->id, rec+14, 5);

  // Temporary variables for reading individual fields
  unsigned long i;
  double d;

  i = *(unsigned long*)(rec+89);
  i = LittleEndian (i);
  navaid->type = i;

  i = *(unsigned long*)(rec+93);
  i = LittleEndian (i);
  navaid->classification = i;

  i = *(unsigned long*)(rec+97);
  i = LittleEndian (i);
  navaid->usage = i;

  d = *(double*)(rec+101);
  d = LittleEndian (d);
  navaid->pos.lat = d;

  d = *(double*)(rec+109);
  d = LittleEndian (d);
  navaid->pos.lon = d;
  
  i = *(unsigned long*)(rec+117);
  i = LittleEndian (i);
  navaid->pos.alt = i;

  d = *(double*)(rec+59);
  d = LittleEndian (d);
  navaid->freq = d;
  
  i = *(unsigned long*)(rec+125);
  i = LittleEndian (i);
  navaid->range = i;
  
  d = *(double*)(rec+67);
  d = LittleEndian (d);
  navaid->magneticVariation = d;
  
  d = *(double*)(rec+75);
  d = LittleEndian (d);
  navaid->slavedVariation = d;
  
  navaid->prev = NULL;
  navaid->next = NULL;
}

static void DecodeAirportRecord (char* rec, SAirport *airport)
{
  // Temporary variables for reading individual fields
  unsigned long i;
  double d;
  char c;

  strncpy (airport->airportKey, rec+4, 10);
  strncpy (airport->faaID, rec+14, 4);
  strncpy (airport->icaoID, rec+18, 5);

  i = *(unsigned long*)(rec+89);
  i = LittleEndian (i);
  airport->type = i;

  strncpy (airport->name, rec+23, 40);
  strncpy (airport->country, rec+63, 3);
  strncpy (airport->state, rec+66, 3);
  strncpy (airport->county, rec+69, 20);
  strcpy (airport->city, "");

  i = *(unsigned long*)(rec+109);
  i = LittleEndian (i);
  airport->elevation = (float)i;

  i = *(unsigned long*)(rec+113);
  i = LittleEndian (i);
  airport->ownership = i;

  i = *(unsigned long*)(rec+117);
  i = LittleEndian (i);
  airport->usage = i;

  d = *(double*)(rec+166);
  d = LittleEndian (d);
//  CorrectEndian (&d, sizeof(double));
  airport->magneticVariation = (float)d;
  
  d = *(double*)(rec+174);
  d = LittleEndian (d);
  airport->trafficAltitude = (float)d;
  
  c = *(rec+178);
  airport->fssAvailable = c;
  
  c = *(rec+179);
  airport->notamAvailable = c;
  
  c = *(rec+180);
  airport->atcAvailable = c;
  
  c = *(rec+181);
  airport->segmentedCircle = c;
  
  c = *(rec+182);
  airport->landingFeesCharged = c;
  
  c = *(rec+183);
  airport->jointUseAirport = c;
  
  c = *(rec+184);
  airport->militaryLandingRights = c;
  
  c = *(rec+185);
  airport->customsEntryPoint = c;
  
  i = *(unsigned long*)(rec+186);
  i = LittleEndian (i);
  airport->fuelTypes = i;

  i = *(unsigned long*)(rec+190);
  i = LittleEndian (i);
  airport->frameService = i;

  i = *(unsigned long*)(rec+194);
  i = LittleEndian (i);
  airport->engineService = i;

  i = *(unsigned long*)(rec+198);
  i = LittleEndian (i);
  airport->bottledOxygen = i;

  i = *(unsigned long*)(rec+202);
  i = LittleEndian (i);
  airport->bulkOxygen = i;

  i = *(unsigned long*)(rec+206);
  i = LittleEndian (i);
  airport->beaconLensColor = i;

  i = *(unsigned long*)(rec+210);
  i = LittleEndian (i);
  airport->basedAircraft = i;

  i = *(unsigned long*)(rec+214);
  i = LittleEndian (i);
  airport->annualCommercialOps = i;

  i = *(unsigned long*)(rec+218);
  i = LittleEndian (i);
  airport->annualGeneralAvOps = i;

  i = *(unsigned long*)(rec+222);
  i = LittleEndian (i);
  airport->annualMilitaryOps = i;

  i = *(unsigned long*)(rec+226);
  i = LittleEndian (i);
  airport->attendanceFlags = i;

  i = *(unsigned long*)(rec+238);
  i = LittleEndian (i);
  airport->lightingFlags = i;

  d = *(double*)(rec+93);
  d = LittleEndian (d);
  airport->pos.lat = d;
  
  d = *(double*)(rec+101);
  d = LittleEndian (d);
  airport->pos.lon = d;

  airport->pos.alt = airport->elevation;


  airport->prev = NULL;
  airport->next = NULL;
}

//=========================================================================
//  TRAPING FOR TEST
//  -Change Key to any one
//  -Put a break point
//  -Call from anywhere
//  Stack shows all chrono
//=========================================================================
void TrapVOR(CmHead *obj)
{ if ((obj->GetActiveQ() == VOR) || (obj->GetActiveQ() == NDB) )
  { CNavaid *vor = (CNavaid*)obj;
    char    *key = vor->GetDbKey();
    if (strcmp(key,"US4SFO1") == 0)
      obj = obj;                                      // Breakpoint here
  }
return;
}
//===================================================================================
//  CDatabaseOBS
//===================================================================================
CDatabaseOBS::CDatabaseOBS(const char* dbtFilename) : CDatabase(dbtFilename)
{}
//===================================================================================
//  Obstacle decoder
//===================================================================================
void CDatabaseOBS::DecodeRecord(U_LONG offset,CmHead *obj)
{ U_INT  No  = GetRecordNo(offset);
  // Temporary variables for reading individual fields
  unsigned long i;
  double d;
  CObstacle *obs = (CObstacle *)obj;
  //-------------------------------------------------------
  strncpy(obs->okey,buf+ 4,10);             // okey
  strncpy(obs->osta,buf+14, 3);             // osta
  strncpy(obs->octy,buf+17,16);             // octy
  strncpy(obs->onam,buf+33,21);             // onam
  //----Lattitude ----------------------
  d = *(double*)(buf+54);
  d = LittleEndian (d);
  obs->pos.lat = d;
  //----Longitude ----------------------
  d = *(double*)(buf+62);
  d = LittleEndian (d);
  obs->pos.lon = d;
  //---Elevation () --------------------
  i = *(unsigned long*)(buf+70);
  i = LittleEndian (i);
  obs->pos.alt = float(i);
  //---Type         --------------------
  i = *(unsigned long*)(buf+74);
  i = LittleEndian (i);
  obs->otyp = i;
  //---Strobe indicator------------------
  i = *(unsigned long*)(buf+78);
  i = LittleEndian (i);
  obs->ostb =  char(i);
  //----Frequency ----------------------
  d = *(double*)(buf+79);
  d = LittleEndian (d);
  obs->ofrq = float(d);
  //-------------------------------------
  double lr     = FN_RAD_FROM_ARCS(obs->pos.lat);					//DegToRad (obs->pos.lat / 3600.0); 
  obs->nmFactor = cos(lr) / 60;                           // 1 nm at latitude lr
  return;
}

//===================================================================================
//	CDatabaseWPT
//===================================================================================
CDatabaseWPT::CDatabaseWPT(const char* dbtFilename) : CDatabase(dbtFilename)
{}
//===================================================================================
//  CWPT DECODER for Db Cache
//===================================================================================
void CDatabaseWPT::DecodeRecord(U_LONG offset,CmHead *obj)
{ U_INT  No  = GetRecordNo(offset);
  // Temporary variables for reading individual fields
  unsigned long i;
  double d;
  CWPT *wpt = (CWPT *)obj; 
  wpt->SetRecNo(No);
  //-----------------------------------------------
  strncpy(wpt->wkey,buf+ 4,10);             // Key
  strncpy(wpt->wnam,buf+14,26);             // Name
  strncpy(wpt->wcty,buf+40, 3);             // Country
  strncpy(wpt->wsta,buf+43, 3);             // State
  strncpy(wpt->wnav,buf+95,10);             // NAV key
  wpt->wloc       = *(buf+46);              // Loc

  //----Type ---------------------------
  i = *(unsigned long*)(buf+47);
  i = LittleEndian (i);
  wpt->wtyp = (U_SHORT)(i);
  //----Usage---------------------------
  i = *(unsigned long*)(buf+51);
  i = LittleEndian (i);
  wpt->wuse = (U_SHORT)(i);
  //----Lattitude ----------------------
  d = *(double*)(buf+55);
  d = LittleEndian (d);
  wpt->wpos.lat = d;
  //----Longitude ----------------------
  d = *(double*)(buf+63);
  d = LittleEndian (d);
  wpt->wpos.lon = d;
  //---Elevation (0) -------------------
  wpt->wpos.alt = float(0);
  //----Mag deviation-------------------
  d = *(double*)(buf+71);
  d = LittleEndian (d);
  wpt->wmag = float(d);
  //----Bearing to NAV -----------------
  d = *(double*)(buf+79);
  d = LittleEndian (d);
  wpt->wbrg = float(d);
  //----Distance to NAV ----------------
  d = *(double*)(buf+87);
  d = LittleEndian (d);
  wpt->wdis = float(d);
  //------------------------------------------------------------------------
  //  Normalize name of waypoint. Eliminate ( and )
  //------------------------------------------------------------------------
  char *txt = wpt->wnam;
  if  (*txt == '(')  strcpy(txt,txt+1);
  char *end = strrchr(txt,')');
  if (end) *end = 0;
  //------Set ident as the first 4 characters of name ----
  strncpy(wpt->widn,txt,4);
  wpt->widn[4] = 0;
  //-------------------------------------
  wpt->SetAttributes();
  return;
}
//------------------------------------------------------------------
//  Decode Record for CSlot
//------------------------------------------------------------------
void CDatabaseWPT::DecodeRecord (CWptLine *slot)
{ U_LONG    n;
  double    d;
  SPosition pos;
  slot->SetName(buf+14);
  slot->SetWaid("INT");
  slot->SetWcty(buf+40);
  slot->SetWsta(buf+43);
  //----Decode Type ----------------------
  n = *(U_LONG*)(buf+47);
  n = LittleEndian (n);
  slot->SetWtyp((U_SHORT)n);
  //---Decode latitude -------------------
  d = *(double*)(buf+55);
  d = LittleEndian (d);
  pos.lat = d;
  //---Decode longitude -------------------
  d = *(double*)(buf+63);
  d = LittleEndian (d);
  pos.lon = d;
  //---Elevation ---------------------------
  pos.alt = 0;
  slot->SetPosition(pos);
  //------------------------------------------------------
  //  Normalize name of waypoint. Eliminate ( and )
  //------------------------------------------------------
  char *txt = slot->GetName();
  if  (*txt == '(')  strcpy(txt,txt+1);
  char *end = strrchr(txt,')');
  if (end) *end = 0;
  return;
}
//===================================================================================
//	CDatabaseCOM
//===================================================================================
CDatabaseCOM::CDatabaseCOM(const char* dbtFilename) : CDatabase(dbtFilename)
{}
//===================================================================================
//  COMM DECODER for Db Cache
//===================================================================================
void CDatabaseCOM::DecodeRecord (U_LONG offset,CmHead *obj)
{ // Temporary variables for reading individual fields
  unsigned long i;
  double d;
  CCOM *com   = (CCOM *)obj;
  U_INT  No  = GetRecordNo(offset);
  com->SetRecNo(No);
  strncpy(com->ckey,buf+ 4,10);
  strncpy(com->capt,buf+14,10);
  strncpy(com->cnam,buf+24,20);

  i = *(unsigned long*)(buf+44);
  i = LittleEndian (i);
  com->ctyp = i;
  d = *(double*)(buf+48);
  d = LittleEndian (d);
  com->cfr1 = float(d);
// Not decoded to save memory ----------

  d = *(double*)(buf+56);
  d = LittleEndian (d);
  com->cfr2 = float(d);

  d = *(double*)(buf+64);
  d = LittleEndian (d);
  com->cfr3 = float(d);

  d = *(double*)(buf+72);
  d = LittleEndian (d);
  com->cfr4 = float(d);

  d = *(double*)(buf+80);
  d = LittleEndian (d);
  com->cfr5 = float(d);
//--------------------------------------
  d = *(double*)(buf+88);
  d = LittleEndian (d);
  com->pos.lat = d;

  d = *(double*)(buf+96);
  d = LittleEndian (d);
  com->pos.lon = d;
  com->pos.alt = 0;
  //-------------------------------------
  com->SetAttributes();
  return;
}
//------------------------------------------------------------------
//  Decode Record for CSlot
//------------------------------------------------------------------
void CDatabaseCOM::DecodeRecord (CComLine *slot)
{ U_LONG n = 0;
  n = *(unsigned long*)(buf+44);
  n = LittleEndian (n);
  slot->SetMask((U_INT)n);
  double d;
  d = *(double*)(buf+48);
  d = LittleEndian (d);
  slot->SetFreq((float)d);
  slot->FillCom();
  return;
}
///==================================================================================
//	CDatabaseCountry
//===================================================================================
CDatabaseCTY::CDatabaseCTY(const char* dbtFilename) : CDatabase(dbtFilename)
{}
//===================================================================================
//  Country decoder
//===================================================================================
void CDatabaseCTY::DecodeRecord (CCountry *cty)
{ strncpy (cty->GetKey(),  buf+ 0, 3);
  strncpy (cty->GetName(), buf+ 3,40);
  cty->SetLimit();
  return;
}
//-----------------------------------------------------------------------------------
//  Country decoder
//-----------------------------------------------------------------------------------
void CDatabaseCTY::DecodeRecord (CSlot *slot)
{ slot->SetName(buf+3);
  slot->SetKey (buf+0);
  return;
}
///==================================================================================
//	CDatabaseState
//===================================================================================
CDatabaseSTA::CDatabaseSTA(const char* dbtFilename) : CDatabase(dbtFilename)
{}
//===================================================================================
//  STATE  decoder
//===================================================================================
void CDatabaseSTA::DecodeRecord (CState *sta)
{ strncpy (sta->GetSKey(),  buf,    5);
  strncpy (sta->GetCKey(),  buf+ 5, 3);
  strncpy (sta->GetState(), buf+ 8, 3);
  strncpy (sta->GetName(),  buf+11,40);
  sta->SetLimit();
  return;
}
//----------------------------------------------------------------------
//  Decode in a slot
//----------------------------------------------------------------------
void CDatabaseSTA::DecodeRecord (CStaLine *slot)
{ slot->SetName(buf+11);
  slot->SetKey(buf);
  slot->SetCTY(buf+5);
  return;
}
///==================================================================================
//	CDatabaseNavaid
//===================================================================================
CDatabaseNAV::CDatabaseNAV(const char* dbtFilename) : CDatabase(dbtFilename)
{}
//===================================================================================
//	NAVAID DECODER for Db cache
//===================================================================================
void CDatabaseNAV::DecodeRecord (U_LONG offset,CmHead *obj)
{ CNavaid *nav = (CNavaid *)obj;
  U_INT No = GetRecordNo(offset);
  nav->SetRecNo(No);
  strncpy (nav->nkey, buf+ 4, 10);
  strncpy (nav->naid, buf+14,  5);
  strncpy (nav->name, buf+19, 40);
  strncpy (nav->ncty, buf+83,  3);
  strncpy (nav->nsta, buf+86,  3);
  // Temporary variables for reading individual fields
  unsigned long i;
  double d;

  i = *(unsigned long*)(buf+89);
  i = LittleEndian (i);
  nav->type = short(i);

  i = *(unsigned long*)(buf+93);
  i = LittleEndian (i);
  nav->ncla = short(i);

  i = *(unsigned long*)(buf+97);
  i = LittleEndian (i);
  nav->usge = short(i);

  d = *(double*)(buf+101);
  d = LittleEndian (d);
  nav->pos.lat = d;

  d = *(double*)(buf+109);
  d = LittleEndian (d);
  nav->pos.lon = d;
  
  i = *(unsigned long*)(buf+117);
  i = LittleEndian (i);
  nav->pos.alt = i;

  d = *(double*)(buf+59);
  d = LittleEndian (d);
  nav->freq = float(d);
  
  i = *(unsigned long*)(buf+125);
  i = LittleEndian (i);
  nav->rang = float(i);
  
  d = *(double*)(buf+67);
  d = LittleEndian (d);
  nav->mDev = float(d);
  
  d = *(double*)(buf+75);
  d = LittleEndian (d);
  nav->nsvr = float(d);
  //--------Compute additional attributes ------------------------
  nav->SetAttributes();
  return;
}
//---------------------------------------------------------------------------------
//	NAVAID DECODER for CSLOT
//---------------------------------------------------------------------------------
void CDatabaseNAV::DecodeRecord (CNavLine *slot)
{ U_LONG    n;
  double    d;
  SPosition pos;
  slot->SetName(buf+19);
  slot->SetVaid(buf+14);
  slot->SetVcty(buf+83);
  slot->SetVsta(buf+86);
  //----Decode Type ----------------------
  n = *(U_LONG*)(buf+89);
  n = LittleEndian (n);
  slot->SetType((U_SHORT)n);
  //---Decode position -------------------
  d = *(double*)(buf+101);
  d = LittleEndian (d);
  pos.lat = d;

  d = *(double*)(buf+109);
  d = LittleEndian (d);
  pos.lon = d;
  
  n = *(unsigned long*)(buf+117);
  n = LittleEndian (n);
  pos.alt = n;
  slot->SetPosition(pos);
  //------------------------------------
  return;
}
//===================================================================================
//	CDatabaseAPT
//===================================================================================
CDatabaseAPT::CDatabaseAPT(const char* dbtFilename) : CDatabase(dbtFilename)
{}
//===================================================================================
//	CDatabaseAPT  DECODER
//  Filed that are not used are not decoded. Uncomment as needed
//===================================================================================
void  CDatabaseAPT::DecodeRecord (U_LONG offset,CmHead *obj)
{ CAirport *apt = (CAirport *) obj;
  U_INT   No    = GetRecordNo(offset);
  apt->SetRecNo(No);
  // Temporary variables for reading individual fields
  unsigned long i;
  double d;
  char c;

  strncpy (apt->akey, buf+4,  10);
  strncpy (apt->afaa, buf+14,  4);
  strncpy (apt->aica, buf+18,  5);
  strncpy (apt->name, buf+23, 40);
  strncpy (apt->actr, buf+63,  3);
  strncpy (apt->asta, buf+66, 3);
//  strncpy (apt->acty, buf+69, 20);                    // County
  //--------Type ---------------------
  i = *(unsigned long*)(buf+89);
  i = LittleEndian (i);
  apt->atyp = (short)i;
  //--------latitude -----------------
  d = *(double*)(buf+93);
  d = LittleEndian (d);
  apt->apos.lat = d;
  //--------longitude----------------
  d = *(double*)(buf+101);
  d = LittleEndian (d);
  apt->apos.lon = d;
  //--------altitude ----------------
  i = *(unsigned long*)(buf+109);
  i = LittleEndian (i);
  apt->apos.alt = i;
  //--------ownership ----------------
  i = *(unsigned long*)(buf+113);
  i = LittleEndian (i);
  apt->aown = (char)i;
  //--------usage    ----------------
  i = *(unsigned long*)(buf+117);
  i = LittleEndian (i);
  apt->ause= (char)i;
  //---------region ------------------
//  strncpy(apt->argn,buf+121,  5);
  //---------Notam ------------------
//  strncpy(apt->antm,buf+126, 20);
  //---------FSS ------------------
//  strncpy(apt->afss,buf+146, 20);
  //--------Magnetic variation -------
  d = *(double*)(buf+166);
  d = LittleEndian (d);
  apt->amag = float(d);
  //--------Traffic altitude ----------------
  i = *(unsigned long*)(buf+174);
  i = LittleEndian (i);
  apt->aalt = i;
  //------- Flight service ------------------
  c = *(buf+178);
  apt->afsa = c;
  //------- NOtam D ------------------
  c = *(buf+179);
  apt->antd = c;
  //------- ATC     ------------------
  c = *(buf+180);
  apt->aata = c;
  //------- Segment     ------------------
  c = *(buf+181);
  apt->aseg = c;
  //------- Landing fee     ------------------
  c = *(buf+182);
  apt->alnd = c;
  //------- Military civil use     ------------------
  c = *(buf+183);
  apt->amjc = c;
  //------- Military rights     ------------------
  c = *(buf+184);
  apt->amln = c;
  //------- Custom     ------------------
  c = *(buf+185);
  apt->acus = c;
  //--------Fuel type   ----------------
  i = *(unsigned long*)(buf+186);
  i = LittleEndian (i);
  apt->afue = (short)i;

  //--------Frame service  ----------------
  i = *(unsigned long*)(buf+190);
  i = LittleEndian (i);
  apt->afrm = (char)i;
  //--------Engine service  ----------------
  i = *(unsigned long*)(buf+194);
  i = LittleEndian (i);
  apt->aeng = (char)i;
  //--------oxygen service  ----------------
  i = *(unsigned long*)(buf+198);
  i = LittleEndian (i);
  apt->abto = (char)i;
  //--------oxygen service  ----------------
  i = *(unsigned long*)(buf+202);
  i = LittleEndian (i);
  apt->ablo = (char)i;
  //--------Beacon color  ----------------
  i = *(unsigned long*)(buf+206);
  i = LittleEndian (i);
  apt->alen = (char)i;
  //--------Aircraft type  ----------------
  i = *(unsigned long*)(buf+210);
  i = LittleEndian (i);
  apt->acft = (short)i;
  //--------Annual operations----------------
  i = *(unsigned long*)(buf+214);
  i = LittleEndian (i);
  apt->aaco = i;
  //--------Annual operations----------------
  i = *(unsigned long*)(buf+218);
  i = LittleEndian (i);
  apt->aaga = i;
  //--------Annual operations----------------
  i = *(unsigned long*)(buf+222);
  i = LittleEndian (i);
  apt->aamo = i;
  //--------Attendance----------------
  i = *(unsigned long*)(buf+226);
  i = LittleEndian (i);
  apt->aatf = i;
  //--------Attendance----------------
  i = *(unsigned long*)(buf+230);
  i = LittleEndian (i);
  apt->aats = i;
  //--------Attendance----------------
  i = *(unsigned long*)(buf+234);
  i = LittleEndian (i);
  apt->aate = i;
  //--------Lighting----------------
  i = *(unsigned long*)(buf+238);
  i = LittleEndian (i);
  apt->altf = (char)i;
  //--------Lighting----------------
  i = *(unsigned long*)(buf+242);
  i = LittleEndian (i);
  apt->alts = i;
  //--------Lighting----------------
  i = *(unsigned long*)(buf+246);
  i = LittleEndian (i);
  apt->alte = i;
  //------- Type     ------------------
  c = *(buf+250);
  apt->atyc = c;
  //------- Icon     ------------------
  c = *(buf+251);
  apt->aicn = c;
  //------- Nber runways ---------------
  c = *(buf+252);
  apt->anrw = c;
  //--------Runway length----------------
  i = *(unsigned long*)(buf+253);
  i = LittleEndian (i);
  apt->alrw = i;
  //------- VOR         ---------------
  c = *(buf+257);
  apt->avor = c;
  //------- ATC         ---------------
  c = *(buf+258);
  apt->aatc = c;
  //------- ILS         ---------------
  c = *(buf+259);
  apt->ails = c;
  //-----Additional attributes     ----
  apt->SetAttributes();
  return;
}
//--------------------------------------------------------------------------
//	AIRPORT DECODER FOR CSLOT
//--------------------------------------------------------------------------
void  CDatabaseAPT::DecodeRecord (CAptLine *slot)
{ U_LONG  n;
  double  d;
  SPosition pos;
  //----------------------------------------------
  slot->SetName(buf+23);              // Name
  slot->SetAica(buf+18);        // ICAO ident
  slot->SetIfaa(buf+14);        // FAA  ident
  slot->SetActy(buf+63);        // Country
  slot->SetAsta(buf+66);        // State
  //-------Decode ownership ----------------------
  n = *(U_LONG*)(buf+113);
  n = LittleEndian (n);
  slot->SetAown((U_CHAR)n);
  //-------Decode position -----------------------
  //--------latitude -----------------
  d = *(double*)(buf+93);
  d = LittleEndian (d);
  pos.lat = d;
  //--------longitude----------------
  d = *(double*)(buf+101);
  d = LittleEndian (d);
  pos.lon = d;
  //--------altitude ----------------
  n = *(unsigned long*)(buf+109);
  n = LittleEndian (n);
  pos.alt = n;
  slot->SetPosition(pos);
  return;
}
//===================================================================================
//	CDatabaseILS
//===================================================================================
CDatabaseILS::CDatabaseILS(const char* dbtFilename) : CDatabase(dbtFilename)
{}
//===================================================================================
//	ILS DECODER for Db cache
//===================================================================================
void CDatabaseILS::DecodeRecord (U_LONG offset,CmHead *obj)
{ CILS *ils = (CILS *)obj;								// Cast type
  U_INT No  = GetRecordNo(offset);
  ils->SetRecNo(No);
  strncpy (ils->ikey,   buf+ 4,10);
  strncpy (ils->iapt,   buf+14,10);
  strncpy (ils->irwy,   buf+24, 4);
  strncpy (ils->iils,   buf+28, 6);
  strncpy (ils->name,   buf+34,40);
  //----Temp var to decode parameters ------------
  unsigned long i;
  double d;
  i = *(unsigned long*)(buf+74);          // Type
  i = LittleEndian (i);
  ils->type = i;
  //-----------latitude --------------------------
  d = *(double*)(buf+78);
  d = LittleEndian (d);
  ils->pos.lat = d;
  //-----------longitude -------------------------
  d = *(double*)(buf+86);
  d = LittleEndian (d);
  ils->pos.lon = d;
  //----------elevation -------------------------
  i = *(unsigned long*)(buf+94);
  i = LittleEndian (i);
  ils->pos.alt = i;
  //----------frequency -------------------------
  d = *(double*)(buf+98);
  d = LittleEndian (d);
  ils->freq = float(d);
  //----------glide slope angle------------------
  d = *(double*)(buf+106);
  d = LittleEndian (d);
  ils->gsan = float(d);
  //----------magnetic variation ----------------
  d = *(double*)(buf+114);
  d = LittleEndian (d);
  ils->mDev = float(d);
  //----------ils range ----------------
  d = *(double*)(buf+122);
  d = LittleEndian (d);
  ils->rang = float(d);
  //-------------------------------------
  ils->SetAttributes();
  return;
}
//-----------------------------------------------------------------------------------
//	ILS DECODER for CSLOT
//-----------------------------------------------------------------------------------
void CDatabaseILS::DecodeRecord (CComLine *slot)
{ double d;
  slot->SetIRWY(buf+24);
    //----------frequency -------------------------
  d = *(double*)(buf+98);
  d = LittleEndian (d);
  slot->SetFreq((float)d);
  return;
}
//===================================================================================
//	CDatabaseRWY
//===================================================================================
CDatabaseRWY::CDatabaseRWY(const char* dbtFilename) : CDatabase(dbtFilename)
{}
//===================================================================================
//	RUNWAY DECODER for DATABASE
//===================================================================================
void CDatabaseRWY::DecodeRecord (U_LONG offset,CmHead *obj)
{ CRunway * rwy = (CRunway *)obj;
  U_INT     No  = GetRecordNo(offset);
  rwy->SetRecNo(No);
  // Temporary variables for reading individual fields
  long nl;
  unsigned long i;
  double d;
  char c;

  strncpy (rwy->rapt, buf+14, 10);
  //-----lenght --------------------
  i = *(unsigned long*)(buf+24);
  i = LittleEndian (i);
  rwy->rlen = i;
  //-----wide   --------------------
  i = *(unsigned long*)(buf+28);
  i = LittleEndian (i);
  rwy->rwid = i;
  //-----Surface type  -------------
  i = *(unsigned long*)(buf+32);
  i = LittleEndian (i);
  rwy->rsfc = i;
  //-----Surface condition-----------
  i = *(unsigned long*)(buf+36);
  i = LittleEndian (i);
  rwy->rcon = i;
  //-----Pavement--------------------
  i = *(unsigned long*)(buf+40);
  i = LittleEndian (i);
  rwy->rpav = i;
  //-----Pavement classification -----
  i = *(unsigned long*)(buf+44);
  i = LittleEndian (i);
  rwy->rpcn = i;
  //-----Pavement subgrade ------------
  i = *(unsigned long*)(buf+48);
  i = LittleEndian (i);
  rwy->rsub = i;
  //-----Pavement Tire PSI --------------
  i = *(unsigned long*)(buf+52);
  i = LittleEndian (i);
  rwy->rpsi = i;
  //-----Traffic direction ---------------
  c = *(buf+56);
  rwy->rpat = c;
  //-----Closed or unusable --------------
  c = *(buf+57);
  rwy->rcls = c;
  //-----Pilot controlled light -----------
  c = *(buf+58);
  rwy->rpcl = c;
  //-----Runway end Id (high)   ------------------
  strncpy (rwy->rhid, buf+59, 4);
  //-----Lattitude (high)     --------------------
  d = *(double*)(buf+63);
  d = LittleEndian (d);
  rwy->pshi.lat = d;
  //-----Longitude (high)     --------------------
  d = *(double*)(buf+71);
  d = LittleEndian (d);
  rwy->pshi.lon = d;
  //-----Elevation (high)     --------------------
  nl = *(long*)(buf+79);
  nl = LittleEndian (nl);
  rwy->pshi.alt = nl;
  //-----True heading (high)   --------------------
  d = *(double*)(buf+83);
  d = LittleEndian (d);
  rwy->rhhd = float(d);
  //-----Magnetic Heading (high)     --------------------
  d = *(double*)(buf+91);
  d = LittleEndian (d);
  rwy->rhmh = float(d);
  //-----Displaced threshold (high)     --------------------
  i = *(unsigned long*)(buf+99);
  i = LittleEndian (i);
  rwy->rhdt = i;
  //-----Threshold elevation (high)     --------------------
  i = *(unsigned long*)(buf+103);
  i = LittleEndian (i);
  rwy->rhte = i;
  //-----End light (high) --------------------
  c = *(buf+107);
  rwy->rhel = c;
  //-----Alignment light (high) ------------
  c = *(buf+108);
  rwy->rhal = c;
  //-----center light (high) ---------------
  c = *(buf+109);
  rwy->rhcl = c;
  //-----touch light (high) ----------------
  c = *(buf+110);
  rwy->rhtl = c;
  //-----threshold light (high) ------------
  c = *(buf+111);
  rwy->rhth = c;
  //-----Edge light (high) -----------------
  c = *(buf+112);
  rwy->rhli = c;
  //-----Sequence flash light (high) -------
  c = *(buf+113);
  rwy->rhsl = c;
  //-----Marking light (high) -------------
  c = *(buf+114);
  rwy->rhmk = c;
  //-----RVR     light (high) -------------
  c = *(buf+115);
  rwy->rhvr = c;
  //-----TRVV light (high)  ---------------
  i = *(unsigned long*)(buf+116);
  i = LittleEndian (i);
  rwy->rhvv = i;
  //-----GlideSlope Type (high) -------------
  c = *(buf+120);
  rwy->rhgt = c;
  //-----GlideSlope configuration (high) ----
  c = *(buf+121);
  rwy->rhgc = c;
  //-----GlideSlope Location      (high) ----
  c = *(buf+122);
  rwy->rhgl = c;
  //-----Light System 1 (high)  -------------
  i = *(unsigned long*)(buf+123);
  i = LittleEndian (i);
  rwy->rh8l[0] = i;
  //-----Light System 2 (high)  -------------
  i = *(unsigned long*)(buf+127);
  i = LittleEndian (i);
  rwy->rh8l[1] = i;
  //-----Light System 3(high)  -------------
  i = *(unsigned long*)(buf+131);
  i = LittleEndian (i);
  rwy->rh8l[2] = i;
  //-----Light System 4(high)  -------------
  i = *(unsigned long*)(buf+135);
  i = LittleEndian (i);
  rwy->rh8l[3] = i;
  //-----Light System 5(high)  -------------
  i = *(unsigned long*)(buf+139);
  i = LittleEndian (i);
  rwy->rh8l[4] = i;
  //-----Light System 6(high)  -------------
  i = *(unsigned long*)(buf+143);
  i = LittleEndian (i);
  rwy->rh8l[5] = i;
  //-----Light System 7(high)  -------------
  i = *(unsigned long*)(buf+147);
  i = LittleEndian (i);
  rwy->rh8l[6] = i;
  //-----Light System 7(high)  -------------
  i = *(unsigned long*)(buf+151);
  i = LittleEndian (i);
  rwy->rh8l[7] = i;

  //-----Runway end Id (low)   ------------------
  strncpy (rwy->rlid, buf+155, 4);
  //-----Lattitude (low)     --------------------
  d = *(double*)(buf+159);
  d = LittleEndian (d);
  rwy->pslo.lat = d;
  //-----Longitude (low)     --------------------
  d = *(double*)(buf+167);
  d = LittleEndian (d);
  rwy->pslo.lon = d;
  //-----Elevation (low)     --------------------
  nl = *(long*)(buf+175);
  nl = LittleEndian (nl);
  rwy->pslo.alt = nl;
  //-----True heading (low)   --------------------
  d = *(double*)(buf+179);
  d = LittleEndian (d);
  rwy->rlhd =  float(d);
  //-----Magnetic Heading (low)     --------------------
  d = *(double*)(buf+187);
  d = LittleEndian (d);
  rwy->rlmh = float(d);
  //-----Displaced threshold (low)     --------------------
  i = *(unsigned long*)(buf+195);
  i = LittleEndian (i);
  rwy->rldt = i;
  //-----Threshold elevation (low)     --------------------
  i = *(unsigned long*)(buf+199);
  i = LittleEndian (i);
  rwy->rlte = i;
  //-----End light (low) --------------------
  c = *(buf+203);
  rwy->rlel = c;
  //-----Alignment light (low) ------------
  c = *(buf+204);
  rwy->rlal = c;
  //-----center light (low) ---------------
  c = *(buf+205);
  rwy->rlcl = c;
  //-----touch light (low) ----------------
  c = *(buf+206);
  rwy->rltl = c;
  //-----threshold light (low) ------------
  c = *(buf+207);
  rwy->rlth = c;
  //-----Edge light (low) -----------------
  c = *(buf+208);
  rwy->rlli = c;
  //-----Sequence flash light (low) -------
  c = *(buf+209);
  rwy->rlsl = c;
  //-----Marking light (low) -------------
  c = *(buf+210);
  rwy->rlmk = c;
  //-----RVR     light (low) -------------
  c = *(buf+211);
  rwy->rlvr = c;
  //-----TRVV light (low)  ---------------
  i = *(unsigned long*)(buf+212);
  i = LittleEndian (i);
  rwy->rlvv = i;
  //-----GlideSlope Type (low) -------------
  c = *(buf+216);
  rwy->rlgt = c;
  //-----GlideSlope configuration (low) ----
  c = *(buf+217);
  rwy->rlgc = c;
  //-----GlideSlope Location      (low) ----
  c = *(buf+218);
  rwy->rlgl = c;
  //-----Light System 1 (low)  -------------
  i = *(unsigned long*)(buf+219);
  i = LittleEndian (i);
  rwy->rl8l[0] = i;
  //-----Light System 2 (low)  -------------
  i = *(unsigned long*)(buf+223);
  i = LittleEndian (i);
  rwy->rl8l[1] = i;
  //-----Light System 3(low)  -------------
  i = *(unsigned long*)(buf+227);
  i = LittleEndian (i);
  rwy->rl8l[2] = i;
  //-----Light System 4(low)  -------------
  i = *(unsigned long*)(buf+231);
  i = LittleEndian (i);
  rwy->rl8l[3] = i;
  //-----Light System 5(low)  -------------
  i = *(unsigned long*)(buf+235);
  i = LittleEndian (i);
  rwy->rl8l[4] = i;
  //-----Light System 6(low)  -------------
  i = *(unsigned long*)(buf+239);
  i = LittleEndian (i);
  rwy->rl8l[5] = i;
  //-----Light System 7(low)  -------------
  i = *(unsigned long*)(buf+243);
  i = LittleEndian (i);
  rwy->rl8l[6] = i;
  //-----Light System 7(low)  -------------
  i = *(unsigned long*)(buf+247);
  i = LittleEndian (i);
  rwy->rl8l[7] = i;
   //-----Texture count(low)  -------------
  i = *(unsigned long*)(buf+251);
  i = LittleEndian (i);
  rwy->rtxc = i;
  //-----Additional attributes ------ ------
  rwy->SetAttributes();
  return;
}
//===================================================================================
//	RUNWAY DECODER for CSLOT
//===================================================================================
void CDatabaseRWY::DecodeRecord (CRwyLine *slot)
{ unsigned long n;
  double d;
  //-----lenght --------------------
  n = *(unsigned long*)(buf+24);
  n = LittleEndian (n);
  slot->rwlg = n;
  //----Decode hi end runway ------------------------------
  strncpy (slot->Hend.rwid, buf+59, 4);
    //-----Lattitude (high)     --------------------
  d = *(double*)(buf+63);
  d = LittleEndian (d);
  slot->Hend.pos.lat = d;
  //-----Longitude (high)     --------------------
  d = *(double*)(buf+71);
  d = LittleEndian (d);
  slot->Hend.pos.lon = d;
  //-----Elevation (high)     --------------------
  n = *(unsigned long*)(buf+79);
  n = LittleEndian (n);
  slot->Hend.pos.alt = n;
  //----Decode low end runway --------------------
  strncpy (slot->Lend.rwid, buf+155, 4);
  //-----Lattitude (low)     --------------------
  d = *(double*)(buf+159);
  d = LittleEndian (d);
  slot->Lend.pos.lat = d;
  //-----Longitude (low)     --------------------
  d = *(double*)(buf+167);
  d = LittleEndian (d);
  slot->Lend.pos.lon = d;
  //-----Elevation (low)     --------------------
  n = *(unsigned long*)(buf+175);
  n = LittleEndian (n);
  slot->Lend.pos.alt = n;
    //-----wide  in feet --------------------
  n = *(unsigned long*)(buf+28);
  n = LittleEndian (n);
  slot->rwid = n;
  return;
}
//===================================================================================
//
// Search for an airport using its unique DAFIF key
//===================================================================================
int GetAirport(char *airportKey, SAirport *airport)
{
  int rc = 0;

  return rc;
}

//
// Return a list of all navaids in the database whose ID field matches the
//   specified key value.
//
int SearchNavaidsByID(char *id, int navType, SNavaid **navaids)
{
  int rc = 0;

  return rc;
}

//
// Return a list of all navaids in the database whose name matches the
//   specified key value.
//
int SearchNavaidsByName(char *name, SNavaid **navaids)
{
  int rc = 0;
  *navaids = NULL;
  SNavaid *tail = NULL;

  // Get handle to airport database
  CDatabase *dbNavaid = CDatabaseManager::Instance().GetNAVDatabase();
  if (dbNavaid == NULL) return rc;

  // Start search of navaid name
  unsigned long offset = dbNavaid->Search ('mann', name);
  if (offset != 0) {
    // Allocate buffer for database record
    char *rec = dbNavaid->GetBuffer();
    while (offset != 0) {
      // Read and decode the navaid
      dbNavaid->GetRawRecord (offset);
      SNavaid *navaid = new SNavaid;
      DecodeNavaidRecord (rec, navaid);

      // Add to the list
      rc++;
      if (*navaids == NULL) {
        // List is still empty; add this navaid as the first element
        *navaids = navaid;
      } else {
        // List is non-empty; link this navaid to the tail of the list
        tail->next = navaid;
        navaid->prev = tail;
      }

      // Update tail pointer so it always points to last navaid added
      tail = navaid;

      // Get offset of next matching record
      offset = dbNavaid->SearchNext ();
    }

  }

  return rc;
}


//
// Return a list of all waypoints in the database whose name field matches the
//   specified key value.
//
int SearchWaypointsByName(char *name, SWaypoint **waypoints)
{
  int rc = 0;

  return rc;
}


//
// Search the airports database based on the ICAO airport ID
//
int SearchAirportsByICAO(char *icaoID, SAirport **airports)
{
  int rc = 0;
  *airports = NULL;

  // Get handle to airport database
  CDatabase *dbAirport = CDatabaseManager::Instance().GetAPTDatabase();
  if (dbAirport == NULL) return rc;

  // Start search of ICAO Index to airport database
  unsigned long offset = dbAirport->Search ('acia', icaoID);
  if (offset != 0) {
    char *rec = dbAirport->GetBuffer();
    dbAirport->GetRawRecord (offset);

    // Allocate first SAirport struct
    MEMORY_LEAK_MARKER ("first")
    SAirport *first = new SAirport;
    MEMORY_LEAK_MARKER ("first")
    DecodeAirportRecord (rec, first);

    // Save pointer to first navaid in return parameter
    *airports = first;
    rc++;

    /// \todo Continue searching for duplicate FAA IDs ?

    // Free buffer for raw database record
  }

  return rc;
}

//
// Search the airports database based on the airport name
//
int SearchAirportsByName(char *name, SAirport **airports)
{
  int rc = 0;

  /// \todo Search airports by name

  return rc;
}


//
// Search for a specific ILS facility.  The airportKey value is the unique
//   DAFIF identifier for the airport, and the runwayEndID is the runway end
//   to search, e.g. "23L"
//
int SearchILS(char *airportKey, char *runwayEndID, SILS **ils)
{
  int rc = 0;

  /// \todo Search ILS facility

  return rc;
}


//===================================================================================

//
// Return a linked list of all ILS within a 200nm radius
//
int GetLocalILS(SILS **ils)
{
  int rc = 0;
  return rc;
}

//
// Return a linked list of all Comms facilities within a 200nm radius
//
int GetLocalComms(SComm **comms)
{
  int rc = 0;
  return rc;
}

//
// Return a linked list of all Center comms facilities within a 200nm radius
//
int GetLocalCenters(SComm **comms)
{
  int rc = 0;
  return rc;
}

//
// Return a linked list of all Waypoints within a 200nm radius
//
int GetLocalWaypoints(SWaypoint **waypoints)
{
  int rc = 0;
  return rc;
}
//==========================================================================
//  Smart pointer to CmHead object
//==========================================================================
CObjPtr::CObjPtr()
{ ptr = 0; }
//-----------------------------------------------------------------
//  Constructor for CmHead P = Q (Q is a pointer to CmHead object)
//-----------------------------------------------------------------
CObjPtr::CObjPtr(CmHead *p)
{ptr = p;}
//-----------------------------------------------------------------
//  Constructor for CmHead P = &obj (obj is a CmHead object)
//-----------------------------------------------------------------
CObjPtr::CObjPtr(CmHead &n)
{ptr = &n; }
//-----------------------------------------------------------------
//  Destructor:  Decrement usage for pointed objet
//-----------------------------------------------------------------
CObjPtr::~CObjPtr()
{ if (ptr)  ptr->DecUser();
}
//-----------------------------------------------------------------
//  Assignement operator  P = Q
//  -Assign the pointer
//  -If a previous object is pointed, decrement user count and
//   possible delete
//------------------------------------------------------------------
void CObjPtr::operator=(CmHead *p)
{ if (ptr == p)   return;
  if (0 != ptr)   ptr->DecUser();
  ptr = p;
  if (0 != ptr)   ptr->IncUser();
  return;
}
//------------------------------------------------------------------
//  Assignement operator P = &CmHead
//------------------------------------------------------------------
void CObjPtr::operator=(CmHead &n)
{ if (ptr ==  &n) return;
  if (0 != ptr)   ptr->DecUser();
  ptr = &n;
  if (0 != ptr)   ptr->IncUser();
  return;
}
//------------------------------------------------------------------
//  Assignement operator P = S (S is a CObjPtr)
//  If a new object is pointed, then increment user count
//------------------------------------------------------------------
void CObjPtr::operator=(CObjPtr &q)
{ if (0 != ptr)   ptr->DecUser();
  ptr = q.ptr;
  if (0 != ptr)   ptr->IncUser();
  return;
}
//==========================================================================
//	CCOMM object
//==========================================================================
CCOM::CCOM(OTYPE qo,QTYPE qa)
:CmHead(qo,qa)
{	 nmiles = 0;            
}
//-----------------------------------------------------------------------
//  Set Additional attributes
//-----------------------------------------------------------------------
void CCOM::SetAttributes()
{ comInx = GetComINDEX(ctyp);
  double lr   = FN_RAD_FROM_ARCS(pos.lat);	//DegToRad (pos.lat / 3600.0);         
  nmFactor = cos(lr) / 60;                           // 1 nm at latitude lr
  return;
}
//-----------------------------------------------------------------------
//	Trace CCOM
//------------------------------------------------------------------------
void CCOM::Trace(char *op,U_INT FrNo,U_INT key)
{	U_INT x = (key >> 16);
	U_INT y = (key & 0x0000FFFF);
	TRACE("        %s COM %16s type %03x Freq %.2f APT=%s",op,cnam,ctyp,cfr1,capt);
	return;
}
//-----------------------------------------------------------------
//	Check if this COM can be reselected again
//  It cannot be reselected if either
//    The COM must be deleted
//    The COM is out of range (100 nm)
//-----------------------------------------------------------------
CCOM *CCOM::IsThisComOK(float hz)
{	//---Lock com here --------------------
  pthread_mutex_lock (&mux);
	bool ok = ((cfr1 == hz) && (nmiles <= 100));
	//---Unlock com here ------------------
  pthread_mutex_unlock (&mux);
  if (ok)		return this;
	DecUser();					// Decrement user and possible delete										
	return 0;	}
//-----------------------------------------------------------------
//	Update the COM with relative position from aircraft
//    radial:   Relative from aircraft position
//    nmiles:   Distance to station
//    DZR:      Verticale dead zone radius in feet = ALTI*TG(7°)  in feet
//	NOTE todo see if GreatCirclePolar can return nmiles in place of feet
//-----------------------------------------------------------------
void	CCOM::Refresh(U_INT FrNo)
{	if (NeedUpdate(FrNo) == false)	return;
	//----compute COM relative position -------------
      SVector	v	       = GreatCirclePolar(&globals->geop, &pos);
	    nmiles           = (float)v.r * MILE_PER_FOOT;
	    return;
}
//-----------------------------------------------------------------
//	Check for freq and range match
//	Object must be in ACT state
//	When selected, increment user count
//-----------------------------------------------------------------
bool CCOM::IsSelected(float hz)
{	//----Lock this objectt here ----------------------
  //pthread_mutex_lock (&mux);
	bool ok = (cfr1 == hz);     //  && (State == ACTV));
	//----Unlock object here --------------------------
  //  pthread_mutex_unlock (&mux);
	return ok;	}
//----------------------------------------------------------------
//	Check if this COM is in range of aircraft
//----------------------------------------------------------------
bool CCOM::IsInRange(void)
{	return (nmiles <= 200);	}
//==========================================================================
//	CWPT object
//==========================================================================
CWPT::CWPT(OTYPE qo,QTYPE qa)
:CmHead(qo,qa)
{	radial	= 0;
	nmiles	= 0;
	wmag		= 0;
	dsfeet	= 0;
	user		= 'dbwp';								// Database waypoint
}
//-----------------------------------------------------------------
//  Set additional attributes
//-----------------------------------------------------------------
void CWPT::SetAttributes()
{ double lr   = FN_RAD_FROM_ARCS(wpos.lat);					//DegToRad (wpos.lat / 3600.0);  
  nmFactor = cos(lr) / 60;                          // 1 nm at latitude lr
  return;
}
//-----------------------------------------------------------------
//	Update the waypoint with relative position from aircraft
//    radial:   Relative from aircraft position
//    nmiles:   Distance to station
//-----------------------------------------------------------------
void	CWPT::Refresh(U_INT FrNo)
{	if (NeedUpdate(FrNo) == false)	return;
	//----compute WPT relative position -------------
  SVector	v	      = GreatCirclePolar(&globals->geop, &wpos);
  radial  = Wrap360((float)v.h - wmag);
	nmiles  = (float)v.r * MILE_PER_FOOT;
  dsfeet  =  v.r;
	return;
}
//-----------------------------------------------------------------
//  Set as flight plan waypoint
//-----------------------------------------------------------------
void	CWPT::Init(char *idn,SPosition *pos)
{	user		= 'uswp';								// User Waypoint
	wkey[0]	= 0;
	wcty[0]	= 0;
	wsta[0]	= 0;
	strncpy(widn,idn,5);
	widn[4]	= 0;
	wloc		= 0;
	wpos		= *pos;
	wtyp		= 2;
	//--- Set reduction factor --------------
	double lr   = FN_RAD_FROM_ARCS(wpos.lat);					//DegToRad (wpos.lat / 3600.0);  
  nmFactor = cos(lr) / 60;                          // 1 nm at latitude lr
	return;
}
//-----------------------------------------------------------------
// Change position
//-----------------------------------------------------------------
void CWPT::SetPosition(SPosition p)
{	wpos = p;
	//--- Set reduction factor --------------
	double lr   = FN_RAD_FROM_ARCS(wpos.lat);					//DegToRad (wpos.lat / 3600.0);  
  nmFactor = cos(lr) / 60;                          // 1 nm at latitude lr
	return;
}
//-----------------------------------------------------------------------
//	Trace Waypoint
//------------------------------------------------------------------------
void CWPT::Trace(char *op,U_INT FrNo,U_INT key)
{	U_INT x = (key >> 16);
	U_INT y = (key & 0x0000FFFF);
	TRACE("        %s WPT %15s iD type %2x",op,wnam,wtyp);
	return;
}
//==========================================================================
//	CNavaid object
//==========================================================================
CNavaid::CNavaid(OTYPE qo,QTYPE qa)
:CmHead(qo,qa)
{	radial	= 0;
	nmiles	= 0;
  dsfeet  = 0;
  vdzRad  = 0;
  xType   = 0;
}
//-----------------------------------------------------------------
//  Set additional attributes 
//-----------------------------------------------------------------
void CNavaid::SetAttributes()
{ SetNavIndex();                                // Set icon index
  if (IsNDB())  qAct = NDB;                     // Set Type
  sprintf(efrq,"%3.2f",freq);                   // Edit frequency
  //-------Compute distance reduction factor -----------------   
  double lr = FN_RAD_FROM_ARCS(pos.lat);				//DegToRad (pos.lat / 3600.0);     
  nmFactor  = cos(lr) / 60;                     // 1 nm at latitude lr
  return;
}
//-------------------------------------------------------------------------
//  return Index type for a  navaid
//-------------------------------------------------------------------------
void CNavaid::SetNavIndex()
{     switch (type) {
      case NAVAID_TYPE_VOR:
        xType = 1;
        return;
      case NAVAID_TYPE_TACAN:
        xType = 2;
        return;
      case NAVAID_TYPE_VORTAC:
        xType = 3;
        return;
      case NAVAID_TYPE_VORDME:
        xType = 4;
        return;
      case NAVAID_TYPE_NDB:
        xType = 5;
        return;
      case NAVAID_TYPE_NDBDME:
        xType = 6;
        return;
  }
  xType = 0;
  return;
}
//-----------------------------------------------------------------
//	Update the navaid with relative position from aircraft
//    radial:   Relative from aircraft position
//    nmiles:   Distance to station
//    DZR:      Verticale dead zone radius in feet = ALTI*TG(7°)  in feet
//	NOTE todo see if GreatCirclePolar can return nmiles in place of feet
//-----------------------------------------------------------------
void	CNavaid::Refresh(U_INT FrNo)
{	if (NeedUpdate(FrNo) == false)	return;
	//----compute Navaid relative position -------------
  SVector	v	      = GreatCirclePolar(&globals->geop, &pos);
  radial  = Wrap360((float)v.h - mDev);
	nmiles  = (float)v.r * MILE_PER_FOOT;
  vdzRad  =  globals->geop.alt * TANGENT_7DEG;
  dsfeet  =  v.r;
	return;
}
//-----------------------------------------------------------------
//	Check for freq and range match
//	Object must be in ACT state
//	When selected, increment user count
//-----------------------------------------------------------------
bool CNavaid::IsSelected(float hz)
{	//----Lock this objectt here ----------------------
  //pthread_mutex_lock (&mux);
	bool ok = (freq == hz);           // && (State == ACTV));
	//----Unlock object here --------------------------
  //pthread_mutex_unlock (&mux);
	return ok;	}
//-----------------------------------------------------------------
//	Check if this Nav can be reselected again
//  It cannot be reselected if either
//    The NAV must be deleted
//    The NAV is out of range
//-----------------------------------------------------------------
CNavaid *CNavaid::IsThisNavOK(float hz)
{	//---Lock navaid here --------------------
  pthread_mutex_lock (&mux);
	bool ok = ((freq == hz) && (rang >= nmiles));
	//---Unlock navaid here ------------------
  pthread_mutex_unlock (&mux);
  if (ok)		return this;
	DecUser();					// Decrement user and possible delete										
	return 0;	}
//-----------------------------------------------------------------
//	Check if this NDB can be reselected again
//  The NDB is reselected if
//  -The radio frequency has not changed
//  -The NDB is not marked for delete
//  -The NDB is still in range of aircraft
//-----------------------------------------------------------------
CNavaid *CNavaid::IsThisNDBOK(float hz)
{	//---Lock navaid here --------------------
  pthread_mutex_lock (&mux);
	bool ok = ((freq == hz) && (rang > nmiles));
	//---Unlock navaid here ------------------
  pthread_mutex_unlock (&mux);
  if (ok)		return this;
	DecUser();					// Decrement user and possible delete										
	return 0;	}
//-----------------------------------------------------------------------
//	Trace NAVAID
//------------------------------------------------------------------------
void CNavaid::Trace(char *op,U_INT FrNo,U_INT key)
{	U_INT x = (key >> 16);
	U_INT y = (key & 0x0000FFFF);
  const char *idn = (type & NAVAID_TYPE_NDB)?("NDB"):("VOR");
  char edit[8];
  sprintf(edit,"%s",idn);
	TRACE("        %s %s %15s iD %4s type %2x Freq %.2f Range %.2f",
		  op,edit,name,nkey,type,freq,rang);

	return;
}
//==========================================================================
//	CAirport object
//==========================================================================
CAirport::CAirport(OTYPE qo,QTYPE qa)
:CmHead(qo,qa)
{	NbILS   = 0;
  MapTp   = 0;
  Prop    = 0;
  cTyp    = 0;
  cfrq    = 0;
  apo     = 0;
  radial  = 0;
  nmiles  = 0;
  atis    = 0;
  rdep    = 0;
  Metar   = 0;
}
//-----------------------------------------------------------------
//	Recycle Runways
//-----------------------------------------------------------------
CAirport::~CAirport()
{ CRunway *rwy = (CRunway*)rwyQ.PopFromQ1();
  while (rwy){ rwy->DecUser(); rwy = (CRunway*)rwyQ.PopFromQ1();  }
  if (Metar)  globals->wtm->RemoveMetar(this);
  return;
}
//-----------------------------------------------------------------
//  Set additional attributes
//-----------------------------------------------------------------
void CAirport::SetAttributes()
{ //--------Compute distance reduction factor ------------------
  double lr   = FN_RAD_FROM_ARCS(apos.lat);				//DegToRad (apos.lat / 3600.0);  
  nmFactor = cos(lr) / 60;                        // 1 nm at latitude lr
}
//-----------------------------------------------------------------
//	Add a runway
//-----------------------------------------------------------------
void CAirport::AddRunway(CRunway *rwy)
{ rwy->IncUser();
  rwyQ.LastInQ1(rwy);
  return;
}
//-----------------------------------------------------------------
//  Get next runway from current
//  NOTE:  This is not protected from concurrent access
//-----------------------------------------------------------------
CRunway *CAirport::GetNextRunway(CRunway *rwy)
{ if (0 == rwy) return (CRunway *)rwyQ.GetFirst();
  else          return (CRunway *)rwyQ.NextInQ1(rwy);
}
//-----------------------------------------------------------------
//	Find  a runway by end identifier
//-----------------------------------------------------------------
CRunway *CAirport::FindRunway(char *rend)
{ CRunway *rwy = 0;
  rwyQ.Lock();
  for (rwy = (CRunway*)rwyQ.GetFirst(); rwy != 0; rwy = (CRunway*)rwy->NextInQ1())
  { if (strcmp(rend,rwy->GetHiEnd()) == 0)  break;
    if (strcmp(rend,rwy->GetLoEnd()) == 0)  break;
  }
	//-- Unlock RWY Queue -------------------------------
  rwyQ.Unlock();
  return rwy;
}
//-----------------------------------------------------------------
//	Find  a runway take-off spot by end identifier
//-----------------------------------------------------------------
float CAirport::GetTakeOffSpot(char *rend,SPosition **dep,ILS_DATA **ils)
{ CRunway   *rwy = 0;
  float rot = 0;
  rwyQ.Lock();
  for (rwy = (CRunway*)rwyQ.GetFirst(); rwy != 0; rwy = (CRunway*)rwy->NextInQ1())
	{ if (strcmp(rend,rwy->GetHiEnd()) == 0)  
		{*dep = rwy->GetLandPos(RWY_HI_END);
		 *ils = rwy->GetIlsData(RWY_HI_END);
		  rot = rwy->GetROT(RWY_HI_END);
			break;
		}
	  if (strcmp(rend,rwy->GetLoEnd()) == 0)  
		{*dep = rwy->GetLandPos(RWY_LO_END); 
		 *ils = rwy->GetIlsData(RWY_LO_END);
		  rot = rwy->GetROT(RWY_LO_END);
			break;}
  }
	//-- Unlock RWY Queue -------------------------------
  rwyQ.Unlock();
  return rot;
}
//-----------------------------------------------------------------
//	Find nearest (to pos) runway take off direction
//	We must compute plane distance to ruway center line
//-----------------------------------------------------------------
ILS_DATA *CAirport::GetNearestRwyEnd(SPosition *pos,SPosition **dst)
{	CRunway   *rwy = 0;
  ILS_DATA  *nrs = 0;
	char      *idn = 0;
  float      dis = 1000;
	rwyQ.Lock();
	for (rwy = (CRunway*)rwyQ.GetFirst(); rwy != 0; rwy = (CRunway*)rwy->NextInQ1())
	{	float rh = GetFlatDistance(rwy->ptrHiPos());
	  //--- Save Hi runway position ---------------------
	  if (rh <  dis)	
		{	idn = rwy->GetHiEnd();
			dis = rh; 
			nrs = rwy->GetIlsData(RWY_HI_END); 
		 *dst	= rwy->ptrLoPos(); }
		//--- Save Lo runway position ----------------------
		float rl = GetFlatDistance(rwy->ptrLoPos());
		if (rl <  dis)  
		{ idn = rwy->GetLoEnd();
			dis = rl; 
			nrs = rwy->GetIlsData(RWY_LO_END);
		 *dst = rwy->ptrHiPos();	}
	}
	//-- Unlock RWY Queue -------------------------------
  rwyQ.Unlock();
	return nrs;
}
//-----------------------------------------------------------------
//  Return One of Airport Identifier
//-----------------------------------------------------------------
char *CAirport::GetAptName()
{ char *fn = GetFaaID();
  return (*fn)?(fn):(GetFaica());
}
//-----------------------------------------------------------------
//  Rebuild runway lights
//-----------------------------------------------------------------
void CAirport::RebuildLights(CRunway *rwy)
{ Prop |= TC_PCHANGES;
  if (0 == apo) return;
  apo->RebuildLight(rwy);
  return;
}
//-----------------------------------------------------------------
//	Trace AIRPORT
//-----------------------------------------------------------------
void  CAirport::Trace(char *op,U_INT FrNo,U_INT key)
{	U_INT x = (key >> 16);
	U_INT y = (key & 0x0000FFFF);
	TRACE("        %s APT %40s key %10s Icon %u Map %u",op,name,akey,aicn,MapTp);
	return;
}
//-------------------------------------------------------------------
//  Return fuel grad index in data bank
//-------------------------------------------------------------------
short CAirport::GetFuelGrad()
{ short     xg = 1;
  U_SHORT fuel = (U_SHORT)afue;
  while (fuel)  
  { if (fuel & 1)  return (xg < 10)?(xg):(0);
    xg++;
    fuel = fuel >> 1;
  }
  return 0;
}
//---------------------------------------------------------------------------------
//  Compute airport type for vector map
// There are 10 different airport icons available for display on the vector map:
//
//  1  : Public airport, multiple hard rwys, at leat 1 > 8069', yes beacon, yes tower
//  2  : Public airport, single hard rwy 1500' to 8069', yes beacon, no tower
//  3  : Public airport, multiple hard rwys, at least 1 > 8069', no beacon, yes tower
//  4  : Public airport, multiple hard rwys, at least 1 > 8069', no beacon, no tower
//  5  : Public airport, single hard rwy 1500' to 8069', no beacon, no tower
//  6  : Public airport, multiple soft rwys, no beacon, yes tower
//  7  : Military airport, soft rwys
//  8  : Public airport, soft rwys
//  9  : Public airport, multiple hard rwys, at least 1 > 8069', yes beacon, no tower
//  10 : Public airport, multiple soft runways, beacon, tower
//----------------------------------------------------------------------------------
int  CAirport::SetMapType(void)
{     int   hasTower   = (aata != 0);
      int   hasBeacon  = (alen != AIRPORT_BEACON_NONE);
      int   isPublic   = (ause == AIRPORT_USAGE_PUBLIC);

      // Scan runways
      int   hasLongRunway = (Prop & TC_LONG_RWY);
      int   hasHardRunway = (Prop & TC_HARD_RWY);
      int   hasMultipleRunways = (anrw > 1);

      // Determine appropriate type
      if (hasHardRunway) {
        // At least one hard runway
        if (hasMultipleRunways) {
          // Multiple hard runways
          if (hasBeacon) {
            // Multiple hard runways with beacon
            return (hasTower)?(1):(9);
          } else {
            return (hasTower)?(3):(4);
          }
        } else  return (hasBeacon)?(2):(5);
      }
        // Only soft runways
      if (hasMultipleRunways) {
        return (hasBeacon)?(6):(10);
        }
      return (isPublic)?(8):(7);
}
//-----------------------------------------------------------------
//	Update the airport with relative position from aircraft
//    radial:   Relative cap from aircraft position
//    nmiles:   Distance to station
//	NOTE todo see if GreatCirclePolar can return nmiles in place of feet
//-----------------------------------------------------------------
void CAirport::Refresh(U_INT FrNo)
{	//----compute airport relative position -------------
  if (NeedUpdate(FrNo) == false)	return;
  SVector	v	      = GreatCirclePolar(&globals->geop, &apos);
  radial  = Wrap360((float)v.h - amag);
	nmiles  = (float)v.r * MILE_PER_FOOT;
	return;
}
//-----------------------------------------------------------------------------------------
//  Write the Light profile for this airport
//-----------------------------------------------------------------------------------------
void CAirport::SaveProfile()
{ CRunway *rwy = 0;
  char path[MAX_PATH];
  SStream   s;
  char     *fn = GetAptName();
  U_CHAR   mod = Prop & TC_PCHANGES;
  if (0 == mod) return;
  sprintf(path,"Runways/%s.RLP",fn);
  //---Open a stream file -----------------------------------
  strcpy (s.filename, path);
  strcpy (s.mode, "w");
  if (!OpenStream (&s)) {WARNINGLOG("CAptObject : can't write %s", path); return;}
  WriteTag('bgno', "========== BEGIN FILE ==========" , &s);
  for (rwy = GetNextRunway(rwy); rwy != 0;rwy = GetNextRunway(rwy))
  { CRLP *lpf = rwy->GetRLP();
    if (lpf)  lpf->WriteProfile(rwy,&s);  
  }
  //---Close the file ---------------------------------------
  WriteTag('endo', " === End of Runway Definition file ===", &s);
  CloseStream (&s);
  Prop &= (-1 - TC_PCHANGES);
  return;
}
//==========================================================================
//  Result marker
//==========================================================================
CBeaconMark::CBeaconMark()
{ ok    = 0;
  on    = 0;
  ch    = 0;
  cone  = 0;
  xdev  = 0;
  ydev  = 0;
  timr  = 0;
  blnk  = 0;
  sbuf  = 0;
}
//--------------------------------------------------------------------------
//  Destructor
//--------------------------------------------------------------------------
CBeaconMark::~CBeaconMark()
{ if (sbuf) sbuf->Release();
}
//--------------------------------------------------------------------------
//  Update the timer
//--------------------------------------------------------------------------
void  CBeaconMark::Flash(float dT)
{ timr += dT;
  if (timr < blnk)  return; 
  on   = 0;
  if (timr < endt)  return;
  timr = 0;
  on   = 1;
  return;
}
//--------------------------------------------------------------------------
//  Update the state
//--------------------------------------------------------------------------
char CBeaconMark::Set(char s)
{ char old  = ok;
  ok        = s;
  if (old == ok)  return ok;
  if (sbuf) sbuf->Play(ok);
  return ok;
}
//==========================================================================
//	CILS object
//==========================================================================
CILS::CILS(OTYPE qo,QTYPE qa)
:CmHead(qo,qa)
{ rwy     = 0;                              // Clear runway
  ilsVEC  = 0;                              // ils vector
 	radial	= 0;                              // Clear radial
	nmiles	= 0;                              // Clear direction
  ilsD    = 0;
}
//------------------------------------------------------------------------
//  Set additional attributes
//------------------------------------------------------------------------
void  CILS::SetAttributes()
{ double lr = FN_RAD_FROM_ARCS(pos.lat);			//DegToRad (pos.lat / 3600.0); 
  nmFactor  = cos(lr) / 60;                   // 1 nm at latitude lr
  return;
}
//------------------------------------------------------------------------
//  Init glide parameters
//  Marker are set such that
//  -Outter marker is detected at altitude 1200 feet
//  -Medium marker is detected at altitude  300 feet
//   Height / distance = tan(slope); thus distance = H / tan(slope);
//------------------------------------------------------------------------
void CILS::SetGlidePRM()
{ if (0 == gsan)  gsan = 3;     // Default 3°
  double slope  = DegToRad(gsan);
  double glTan  = tan(slope);
  ilsD->gTan    = glTan;
  //---Compute OUTR marker parameters ---------------------------------
  double bmo    = DegToRad(45.0);
  double tgo    = tan(bmo);
  outM.hmax     = 3000;
  outM.hght     = 1200;
  outM.dist     = 1200 / glTan;
  outM.tang     = tgo;
  //---Compute MIDL marker parameters ---------------------------------
  double bmm    = DegToRad(51.0);
  double tgm    = tan(bmm);
  medM.hmax     = 1000;
  medM.hght     =  300;
  medM.dist     =  300 / glTan;
  medM.tang     =  tgm;
  //---Compute INNR marker parameters ---------------------------------
  double bin    = DegToRad(70.0);
  double tgi    = tan(bin);
  inrM.hmax     = 200;
  inrM.hght     = 100;
  inrM.dist     =  20 / glTan;
  inrM.tang     =  tgi;
}
//------------------------------------------------------------------------
//  Check if the given location is inside the marker
//  SPosition cis defined like this
//  pos.lon = lateral (horizontal) deviation
//  pos.lat = distance to ILS
//  pos.z = plane altitude
//-------------------------------------------------------------------------
char CILS::InMARK(SVector &pos, B_MARK &b,CBeaconMark &m)
{ if (pos.z > b.hmax)   return m.Set(0);
  //---Compute altitude above threshold -------
  double alt  = pos.z - ilsD->lndP.alt;     //landP->alt;
  //---Compute cone radius at this altitude ---
  double rad  = alt * b.tang;
  m.cone      = rad;
  //--- Check for distance --------------------
  double dtd  = fabs(b.dist - pos.y);
  m.ydev      = dtd;
  if (dtd > rad)      return m.Set(0);
  //--- Check for horizontal deviation --------
  double ang = DegToRad(pos.x);
  double hdv = fabs(pos.y * sin(ang));
  m.xdev     = hdv;
  char ok    = (hdv < rad)?(1):(0);
  return m.Set(ok);
}
//------------------------------------------------------------------------
//  Initialize ILS with runway parameters
//------------------------------------------------------------------------
void CILS::SetIlsParameters(CRunway *rwy,ILS_DATA *dt, float dir)
{ this->rwy   = rwy;
  float mdir  = Wrap360(dir - mDev);				// Magnetic deviation
  ilsVEC      = Wrap360(dir + 180);					// Runway vector
  ilsD        = dt;
  ilsD->ils   = this;
	ilsD->lnDIR = mdir;
  return SetGlidePRM();
}
//-----------------------------------------------------------------
//	Check if this ILS can be reselected again
//  It is reselected if
//    The radio frequency is not changed
//    The ILS is not marked for delete
//    The ILS is still in range of the aircraft
//-----------------------------------------------------------------
CILS*  CILS::IsThisILSOK(float hz)
{	//---Lock ILS here --------------------
  pthread_mutex_lock (&mux);
	bool ok = ((freq == hz) && (rang > nmiles));
	//---Unlock ILS here ------------------
  pthread_mutex_unlock (&mux);
  if (ok)		return this;
	DecUser();					                      // Decrement user and possible delete										
	return 0;	}
//-----------------------------------------------------------------
//	Check for freq and type match
//	Object must be in ACT state
//	When selected, increment user count
//-----------------------------------------------------------------
bool CILS::IsSelected(float hz)
{	//----Lock this objectt here ---------------------
  //pthread_mutex_lock (&mux);
	bool ok = ((freq == hz)  && (rwy));     // && (State == ACTV));
	//----Unlock object here --------------------------
  //pthread_mutex_unlock (&mux);
	return ok;	}
//----------------------------------------------------------------------------
//	Update the ILS with relative position from aircraft
//  -Radial is the aircraft position relative to runway threshold
//  -nMiles is the opticale distance in mile (from aircraft to runway threshold)
//  -disF is the distance in feet (this is optical not ground distance)
//  -errG is a differential value between:
//          Aircraft altitude
///         Altitude of glide plan at aircraft position
//    S is the glide slope plane
//    R is the reference position (ILS position)
//    P is the aircraft position
//    D is the distance (P-R)
//    hG is the landing ground elevation
//   Then:
//    gH = D * TAN(3°)                    Glide Plan height at distance D (feet)
//    dH = alt(P) - hG  - gH;             Differential height between P and ideal plane(feet)
//    with dH / D ~= TAN(error) = errG    Elevation error for aircraft in tan unit.
//   Then
//    errG is given to the ILS gauge. As the gauge has N frames for a 5° variation
//    above or below the glide slope, then F = (N * vGlide) / TG(5°) is the frame 
//    to use to show the ALPHA variation.
// 
//    errG is also used by autopilot to maintain the glide slope in
//    APPROACH mode
//		NOTE:  ILS position are set at opposite end of the runway.
//					 Thus the reference point (refP) is the other side of the runway, 
//					 the distance is distance to opposite runway end.
//					 Thus , when choosing which ILS is nearest to the aircraft
//					 the test should be inverted. (See AIRPORT for ILS construction)
//---------------------------------------------------------------------------
void	CILS::Refresh(U_INT FrNo)
{	if (NeedUpdate(FrNo) == false)	return;
	//----Compute aircraft position to ILS glide slope -------
  SPosition  *ref = &ilsD->refP;
  SVector	v	      = GreatCirclePolar(&globals->geop,ref);
  radial = Wrap360((float)v.h - mDev);
	nmiles = float(v.r) * MILE_PER_FOOT;
  double dsf = v.r;             // feet distance
  ilsD->disF = dsf;							// 
  //----compute glide slope variation in tan unit --------
  double vH  = dsf * ilsD->gTan;
  ilsD->errG = (globals->geop.alt - vH - ref->alt) / dsf;
	return;
}
//-----------------------------------------------------------------
//	Trace ILS
//-----------------------------------------------------------------
void  CILS::Trace(char *op,U_INT FrNo,U_INT key)
{	U_INT x = (key >> 16);
	U_INT y = (key & 0x0000FFFF);
	TRACE("        %s ILS %4s Apt %10s Rwy %4s Freq %06.2f Range %06.2f",
		  op,iils,iapt,irwy,freq,rang);
	return;
}
//==========================================================================
//	CRunway object
//==========================================================================
CRunway::CRunway(OTYPE qo,QTYPE qa)
:CmHead(qo,qa)
{	lpf     = 0;                  // Ligth profile
  ilsT    = 0;                  // No ils
  mlite   = RWY_LS_APPRO;
  slite   = 0;
}
//-----------------------------------------------------------------
//  Destroye the runway
//-----------------------------------------------------------------
CRunway::~CRunway()
{ if (lpf)  delete lpf;
}
//-----------------------------------------------------------------
//  Init end
//-----------------------------------------------------------------
void CRunway::EndAttributes(ILS_DATA &d,SPosition &p)
{  //---Init landing parameters ---------------------------
  double alti = rwyATHR[lgCode];
	d.rwy	  = this;
  d.ils   = 0;
  d.lndP  = p;
  d.altT  = alti;
	d.gTan  = float(TANGENT_3DEG);
	return;
}
//-----------------------------------------------------------------
//  Set Additional attributes
//-----------------------------------------------------------------
void CRunway::SetAttributes()
{ lgCode = GetCode();
  wiCode = GetLetter();
  Grnd   = GetGroundIndex();
  //---Init landing parameters ---------------------------
	EndAttributes(ilsD[RWY_HI_END],pshi);
	//----------------------------
	EndAttributes(ilsD[RWY_LO_END],pslo);
	//----------------------------
  pID[RWY_HI_END].LetID  = GetIdentIndex(rhid[2]);
  pID[RWY_LO_END].LetID  = GetIdentIndex(rlid[2]);
	pID[RWY_HI_END].pos		 = pshi;
	pID[RWY_LO_END].pos		 = pslo;
	//----------------------------
	pID[RWY_HI_END].opos	 = &pID[RWY_LO_END];
	pID[RWY_LO_END].opos	 = &pID[RWY_HI_END];
  //-------Compute distance reduction factor -------------
  double lr   = FN_RAD_FROM_ARCS(pshi.lat);					//DegToRad (pshi.lat / 3600.0);
  nmFactor    = cos(lr) / 60;													// 1 nm at latitude lr
  return;
}
//-----------------------------------------------------------------
//  Return Ground Index
//-----------------------------------------------------------------
U_CHAR CRunway::GetGroundIndex()
{ U_INT mask = 0x00000001;
  U_INT surf = (U_INT)rsfc;
  U_CHAR ind = 1;
  while (ind != 15)
  { if (surf & mask)  return ind;
    ind++;
    mask  = (mask << 1);
  }
  return 0;
}
//-----------------------------------------------------------------
//  Return Letter index
//-----------------------------------------------------------------
U_CHAR CRunway::GetIdentIndex(char kl)
{ if (kl == 'L')      return 10;
  if (kl == 'C')      return 11;
  if (kl == 'R')      return 12;
  return 0;
}
//---------------------------------------------------------------------------------
//  Compute number of threshold bands needed
//  Bands start at 10 feet from the edge and are 12 feet wide
//---------------------------------------------------------------------------------
void  CRunway::SetNumberBand(int bw)
{ int hw = (rwid >> 1);                   // Half size
  int nb = int((hw   - 10.0) / bw);
  if (nb < 2) nb = 0;
  if (nb > 8) nb = 8;
  SetNBT(nb,TC_HI);
  SetNBT(nb,TC_LO);
  return;
}
//---------------------------------------------------------------------------------
//  Set ILS type and parameters
//---------------------------------------------------------------------------------
void CRunway::InitILS(CILS *ils)
{ if (ils->SameEND(rhid)) 
  { ilsT |= ILS_HI_END;
    ils->SetIlsParameters(this,ilsD+RWY_HI_END,rhhd);
    return;
  }
  if (ils->SameEND(rlid))
  { ilsT |= ILS_LO_END;
    ils->SetIlsParameters(this,ilsD+RWY_LO_END,rlhd);
    return;
  }
  return;
}
//---------------------------------------------------------------------------------
//  return Landing direction
//---------------------------------------------------------------------------------
ILS_DATA  *CRunway::GetLandDirection(char *e)
{	//--- landing in hi end --------------
	if (0 == strcmp(e,rhid))	return (ilsD + RWY_HI_END);
	//--- landing in lo end --------------
	if (0 == strcmp(e,rlid))  return (ilsD + RWY_LO_END); 
	return 0;
}
//---------------------------------------------------------------------------------
//  return distance to runway center line (the distance is not normalized, as it
//	must be used only for comparizon)
//	Vector must be in arcsecs, relative to airport origin
//  p is set to feet coordinates after been used
//---------------------------------------------------------------------------------
double CRunway::DistanceToLane(SPosition &p)
{	CVector v;
	v.x			= LongitudeDifference(p.lon,vdir.org.lon);
	v.y			= p.lat - vdir.org.lat;
	FeetCoordinates(v,vdir.rdf);
	double n =  (vdir.afa * v.x) + (vdir.bta * v.y) + vdir.gma;
	return (n / vdir.lgn);
}
//---------------------------------------------------------------------------------
//  Compute runway code (ref TP312E)
//---------------------------------------------------------------------------------
int CRunway::GetCode()
{ if (rlen < 2400)  return 1;         // Code 1 length <  800m
  if (rlen < 3600)  return 2;         // Code 2 length < 1200m
  if (rlen < 5400)  return 3;         // Code 3 length < 1800m
  return 4;
}
//---------------------------------------------------------------------------------
//  Compute runway letter (as number)
//---------------------------------------------------------------------------------
int CRunway::GetLetter()
{ if (rwid <  30)   return 0;           // Code 0 width <  10m
  if (rwid <  45)   return 1;           // Code A width <  15m
  if (rwid <  52)   return 2;           // Code B width <  24m
  if (rwid < 108)   return 3;           // Code C width <  36m
  if (rwid < 156)   return 4;           // Code D width <  52m
  return 5;
}   
//---------------------------------------------------------------------------------
//  Swap lights on/off
//---------------------------------------------------------------------------------
char CRunway::ChangeLights(char ls)
{ //--- Set next light section ------------------------
  if    (mlite == RWY_LS_APPRO)  mlite = RWY_LS_EDGES;
  else   mlite  =  (mlite << 1);
  if (0 == ls)  slite &= (-1 - mlite);    // Clear
  if (1 == ls)  slite |= mlite;           // Set
  if (2 == ls)  slite ^= mlite;           // invert
  return (mlite == RWY_LS_APPRO)?(0):(1);
}
//-----------------------------------------------------------------
//	Trace RUNWAY
//-----------------------------------------------------------------
void  CRunway::Trace(char *op,U_INT FrNo,U_INT key)
{	U_INT x = (key >> 16);
	U_INT y = (key & 0x0000FFFF);
	TRACE("        %s RWY -Airport %10s END %3s Mark %d ----END %3s Mark %d ",
		  op,rapt,rhid,rhmk,rlid,rlmk);
	return;
}
//==========================================================================
//	CObstacle object
//==========================================================================
CObstacle::CObstacle(OTYPE qo,QTYPE qa)
:CmHead(qo,qa)
{	
}
//-----------------------------------------------------------------
//	Trace Obstacle
//-----------------------------------------------------------------
void  CObstacle::Trace(char *op,U_INT FrNo,U_INT key)
{	U_INT x = (key >> 16);
	U_INT y = (key & 0x0000FFFF);
	TRACE("        %s OBN %10s City %16s Name %21s",
		  op,okey,octy,onam);
	return;
}

//==========================================================================
//
//	CACHE DATABASE MANAGER
//  The database cache will track objects like VOR NDB AIrports, etc 
//  located in surroundung tile where the aircraft is moving
//
//==========================================================================
//	Base classe for all linked objects
//----------------------------------------------------------------------
CmHead::CmHead(OTYPE qo,QTYPE qa)
{ oTyp  = (OTYPE)(qo);
  qAct  = (QTYPE)(qa);
	//-------------------------------------------
	uptr		= 0;
  //----Mutex init ----------------------------
  pthread_mutex_init (&mux,  NULL);
  State	  = FREE;	
	NoFrame	= 0;	
	uCount	= 0;
	Cnext	  = 0;
	Cprev	  = 0;
	Tnext	  = 0;
	Tprev	  = 0;
  dLon    = 0;
  dLat    = 0;;

}

//----------------------------------------------------------------------
//  ALternate constructor
//----------------------------------------------------------------------
CmHead::CmHead() 
{ oTyp  = (ANY);
  qAct  = (ANY);
	//-------------------------------------------
	uptr		= 0;
  //----Mutex init ----------------------------
  pthread_mutex_init (&mux,  NULL);
  State	  = FREE;	
	NoFrame	= 0;	
	uCount	= 0;
	Cnext	  = 0;
	Cprev	  = 0;
	Tnext	  = 0;
	Tprev	  = 0;
  dLon    = 0;
  dLat    = 0;;

}
//-----------------------------------------------------------------------
//	Increment user count
//-----------------------------------------------------------------------
void CmHead::IncUser()
{	//----Lock object here ---------------
  pthread_mutex_lock (&mux);
	uCount++;
  pthread_mutex_unlock (&mux);
	//---- Unlock object here -------------
	return;	}

//-----------------------------------------------------------------------
//	Decrement user count
//	When object is marked for delete, it is recycled when user count is 0
//-----------------------------------------------------------------------
void CmHead::DecUser()
{	//--Lock object here ----------------------
  pthread_mutex_lock (&mux);
	uCount--;
  bool del = (uCount == 0);
	//--Unlock object here --------------------
  pthread_mutex_unlock (&mux);
	if (del == false)     return;
  globals->dbc->Recycle(this);
	return;
}
//-----------------------------------------------------------------------
//	Check if object is already updated for this Frame cycle
//-----------------------------------------------------------------------
bool CmHead::NeedUpdate(U_INT FrNo)
{	bool upd  = (FrNo != NoFrame);
	NoFrame   = FrNo;
	return upd;	}
//-----------------------------------------------------------------------
//  Set new state
//-----------------------------------------------------------------------
void  CmHead::SetState(U_CHAR sta)
{ pthread_mutex_lock(&mux);
  State = sta;
  pthread_mutex_unlock(&mux);
  return;
}
//-----------------------------------------------------------------------
//  Return a void position
//-----------------------------------------------------------------------
SPosition CmHead::GetPosition()
{ SPosition pos;
  pos.lat = 0;
  pos.lon = 0;
  pos.alt = 0;
  return pos;
}
//-----------------------------------------------------------------------
//  Return a radio tag
//-----------------------------------------------------------------------
int CmHead::GetRadioIndex()
{ return (qAct > COM)?(0):(RadioIND[qAct]);
}
//==========================================================================
//  GENERIC QUEUE MANAGEMENT
//==========================================================================
CQueue::CQueue()
{ pthread_mutex_init (&mux,  NULL);
  NbObj   = 0;
  First   = 0;
  Last    = 0;
  Prev    = 0;
}
//-------------------------------------------------------------------------
//  Destroy: Free all items in Queue
//-------------------------------------------------------------------------
CQueue::~CQueue()
{ CqItem *itm = Pop();
  while (itm) {delete itm; itm = Pop();}
}
//-------------------------------------------------------------------------
// Flush: Free all items in queue
//-------------------------------------------------------------------------
void CQueue::Flush()
{ CqItem *itm = Pop();
  while (itm) {delete itm; itm = Pop();}
  return;
}
//-------------------------------------------------------------------------
//  Set a copy of the queue
//-------------------------------------------------------------------------
U_INT  CQueue::Copy(CQueue *q2)
{ int old = NbObj;
  NbObj   = q2->NbObj;
  First   = q2->First;
  Last    = q2->Last;
  Prev    = 0;
  return old;
}
//-------------------------------------------------------------------------
//  Append Queue 2
//-------------------------------------------------------------------------
void CQueue::Append(CQueue &q2)
{ CqItem *end = Last;
  CqItem *db2 = q2.First;
  CqItem *ed2 = q2.Last;
  if (end)  end->Next = db2;
  if (ed2)  Last      = ed2;
  if (0 == First) First = db2;
  NbObj += q2.NbObj;
  //--------Clear Q2 -----------------------
  q2.First = 0;
  q2.Last  = 0;
  q2.NbObj = 0;
  return;
}
//-------------------------------------------------------------------------
//  Clear Queue
//-------------------------------------------------------------------------
void  CQueue::Clear()
{ NbObj = 0;
  First = 0;
  Last  = 0;
  Prev  = 0;
  return;
}
//-------------------------------------------------------------------------
//  Insert Item at end
//-------------------------------------------------------------------------
void  CQueue::PutEnd(CqItem *item)
{ if (0 == item)  return;
  NbObj++;								                      // Increment count
	item->Next	  = 0;							              // No next
	CqItem    *lo = Last;					                // Get the last object
	Last	        = item;							            // This is the last
	if (lo == NULL)	  First = item;		            // Queue was empty
  else		lo->Next = item;		                  // Link previous to new item  
  return;
}
//-----------------------------------------------------------------
//  Set new item at first position
//-----------------------------------------------------------------
void CQueue::PutHead(CqItem *item)
{ NbObj++;								                      // Increment count
  item->Next  = First;                          // Link to next
  First       = item;                           // New First
  if (0 == Last)  Last = item;                  // Was empty
  return;
}
//-----------------------------------------------------------------
//  Remove and return The First Item
//-----------------------------------------------------------------
CqItem *CQueue::Pop()
{ CqItem *deb = First;
  if (deb)    First = deb->Next;
  if (deb)    deb->Next = 0;
  if (First == 0) Last  = 0;
  if (deb)    NbObj--;								          // decrement count
  return deb;
}
//------------------------------------------------------------------
//  Detach the current item (The NExt is still the next)
//------------------------------------------------------------------
CqItem *CQueue::Detach(CqItem *itm)
{ CqItem *nex = itm->Next;
  if (Prev) Prev->Next  = nex;
  else      First       = nex;
  if (itm == Last) Last = Prev;
  itm->Next  = 0;
  NbObj--;								                      // decrement count
  return Prev;
}
//------------------------------------------------------------------
//  Return the next item
//------------------------------------------------------------------
CqItem *CQueue::GetNext(CqItem *itm)
{ CqItem *nex = itm->Next;
  Prev  = itm;
  return nex;
}
//------------------------------------------------------------------
//  Return the First item
//------------------------------------------------------------------
CqItem *CQueue::GetFirst()
{ Prev  = 0;
  return First;
}
//==========================================================================
//	Queue header
//==========================================================================
//  Constructor by default
//--------------------------------------------------------------------------
ClQueue::ClQueue()
{ pthread_mutex_init (&mux,  NULL);
  NbrOb	= 0;
	First	= 0;
	Last	= 0;
}
//-------------------------------------------------------------------------
//  Destructor
//-------------------------------------------------------------------------
ClQueue::~ClQueue()
{ Clear();
}
//-----------------------------------------------------------------------
//	Delete all objects in a queue
//-------------------------------------------------------------------------
void ClQueue::Clear()
{	CmHead *obj = PopFromQ1();
	while (obj)
	{	if (obj->IsShared())  obj->DecUser();
    else  delete obj;
		obj	= PopFromQ1();
	}
	return;
}
//-------------------------------------------------------------------------
//	Put a an object at end of Q1 (Clink used)
//-------------------------------------------------------------------------
void ClQueue::LastInQ1(CmHead *obj)
{	//--- Lock the queue -------------------------
  pthread_mutex_lock (&mux);
	NbrOb++;								                      // Increment count
	obj->Cnext	  = 0;							              // No next
	CmHead *lo = Last;					                // Get the last object
	obj->Cprev	  = lo;							              // Link to previous
	Last	        = obj;							            // This is the last
	if (lo == NULL)	  First     = obj;		        // Queue was empty
	if (lo != NULL)		lo->Cnext = obj;		        // Link previous to new
	//--- Unlock the queue ------------------------
  pthread_mutex_unlock (&mux);
	return ;	}
//---------------------------------------------------------------------
//	Insert a new Object in front of Queue
//---------------------------------------------------------------------
void ClQueue::HeadInQ1(CmHead *obj)
{ pthread_mutex_lock (&mux);
  NbrOb++;
  CmHead *nx    = First;
  obj->Cnext    = nx;
  obj->Cprev    = 0;
  if (nx) nx->Cprev = obj;
  First         = obj;
  if (0 == Last)  Last = obj;
  pthread_mutex_unlock (&mux);
	return ;	}
//-------------------------------------------------------------------------
//	Put a an object at end of Q2 (Tlink used)
//-------------------------------------------------------------------------
void ClQueue::LastInQ2(CmHead *obj)
{	//--- Lock the queue -------------------------
  pthread_mutex_lock (&mux);
	NbrOb++;								                      // Increment count
	obj->Tnext	  = 0;							              // No next
	CmHead *lo = Last;					                // Get the last object
	obj->Tprev	  = lo;							              // Link to previous
	Last	        = obj;							            // This is the last
	if (lo == NULL)	  First     = obj;		        // Queue was empty
	if (lo != NULL)		lo->Tnext = obj;		        // Link previous to new
	//--- Unlock the queue ------------------------
  pthread_mutex_unlock (&mux);
	return ;	}
//-------------------------------------------------------------------------
//	Detach an object from  queue 1
//  Return the previous object
//-------------------------------------------------------------------------
CmHead *ClQueue::DetachFromQ1(CmHead *obj)
{	//--- Lock the queue -and object --------------------
  pthread_mutex_lock (&mux);
	CmHead	*nx = obj->Cnext;					      // next object
	CmHead	*pv = obj->Cprev;					      // Previous
	NbrOb--;									                // Decrease count
	if (nx)		nx->Cprev = pv;						      // Relink next
	if (pv)		pv->Cnext = nx;						      // Relink previous
	if (First == obj)	    First = nx;			    // New first
	if (Last  == obj)	    Last  = pv;			    // New last
	obj->Cnext	= 0;								          // Clear detached next
	obj->Cprev  = 0;								          // Clear detached previous
	//--- Unlock the queue --and object ------------------
  pthread_mutex_unlock (&mux);
	return pv;	}
//---------------------------------------------------------------------
//	Pop the first object from Queue 1
//---------------------------------------------------------------------
CmHead*	ClQueue::PopFromQ1()	
{	//--- Lock the queue -------------------------------
  pthread_mutex_lock (&mux);
	CmHead	*obj	  = First;						            // Pop the first
	if (obj)	{First  = obj->Cnext; NbrOb--; }		    // Update header
	if (Last == obj)	  Last = 0;					            // Queue is now empty
	//---Unlock the queue here --------------------------
  pthread_mutex_unlock (&mux);
	if (0 == obj)	return obj;				
	obj->Cnext	= 0;
	obj->Cprev	= 0;
	return obj;	}
//---------------------------------------------------------------------
//	Pop the first object from Queue 2
//---------------------------------------------------------------------
CmHead*	ClQueue::PopFromQ2()	
{	//--- Lock the queue -------------------------------
  pthread_mutex_lock (&mux);
	CmHead	*obj	  = First;						            // Pop the first
	if (obj)	{First  = obj->Tnext; NbrOb--; }		    // Update header
	if (Last == obj)	  Last = 0;					            // Queue is now empty
	//---Unlock the queue here --------------------------
  pthread_mutex_unlock (&mux);
	if (0 == obj)	return obj;				
	obj->Tnext	= 0;
	obj->Tprev	= 0;
	return obj;	}
//---------------------------------------------------------------------
//	Insert a new Object in front of obj
//---------------------------------------------------------------------
void ClQueue::InsertInQ1(CmHead *nob,CmHead *obj)
{ //--- Lock the queue -------------------------------
  pthread_mutex_lock (&mux);
  CmHead *prv = (obj)?(obj->Cprev):(Last);
  if (prv)  prv->Cnext = nob;
  nob->Cnext  = obj;
  if (obj)  obj->Cprev = nob;
  nob->Cprev  = prv;
  if (0 ==     obj)   Last  = nob;
  if (First == obj)   First = nob;
  NbrOb++;
  //---Unlock the queue here --------------------------
  pthread_mutex_unlock (&mux);
  return;  
}
 
//=========================================================================
//	new Tile cache.  
//=========================================================================
CTileCache::CTileCache(OTYPE qo,QTYPE qa)
:CmHead(qo,qa)
{	tr = globals->dbc->GetTrace();
  int inx  = 0;
	for (inx = 0; inx != QDIM; inx++)
	{	qHead[inx]	= &qList[inx];
	}
  //---------------------------------------------------------
  NbObj = 0;
}
//---------------------------------------------------------------------
//	Put an object in a cache tile queue
//	NOTE:	This queue need not to be locked as it is only used by
//			the DbCacheMgr
//---------------------------------------------------------------------
void CTileCache::PutInTile(CmHead *obj)
{	ClQueue *hd = qHead[obj->GetActiveQ()];
  hd->LastInQ2(obj);
  NbObj++;                                          // Increase total count
  //---Compute magnetic deviation -----------------------
  globals->dbc->IncDeviation(obj->GetMagDev());
	return;	}
//---------------------------------------------------------------------
//	Pop the first object from the tile cache queue qx
//	NOTE: do not modify the cache links and other data
//---------------------------------------------------------------------
CmHead*	CTileCache::PopObj(QTYPE qx)	
{	//--- Lock the queue ---------------------
	ClQueue		*hd		= qHead[qx];						            // Header
  return hd->PopFromQ2();
}
//---------------------------------------------------------------------
//	Free all objects from the tile cache queue qx
//	NOTE:  This is a critical sequence
//		When an object is used by one or more radio components, it is
//		detached from the cache. 
//		If user count is 0, the object is freed.
//		Otherwise, it will be deleted by component call when
//		not used anymore
//---------------------------------------------------------------------
int	CTileCache::FreeQueue(QTYPE qx)
{	CmHead	*obj = 0;							                // Current object
  int nbo  = 0;
  for (obj = PopObj(qx); obj != 0; obj = PopObj(qx))
	{	globals->dbc->DetachFromCache(obj);				    // Detach from cache
    globals->dbc->DecDeviation(obj->GetMagDev());
	  obj->DecUser();
    nbo++;
	}
	return nbo;
}
//---------------------------------------------------------------------
//	Free  this tile:
//	-Free all cache tile queue
//	-Free this tile
//---------------------------------------------------------------------
void	CTileCache::FreeAllQueues()
{	int nbo = 0;
  int inx = 0;
	for (inx = APT; inx != QDIM; inx ++) 
	{ if (qList[inx].NotEmpty())	nbo +=FreeQueue((QTYPE)inx);
	}
  if(!tr)    return;	
  TRACE("----------- TILE %03u-%03u free %u objects.", (gKey >> 16),(gKey & 0x0000FFFF),nbo);
	return;	}
//-----------------------------------------------------------------------
//	Trace Cache movement
//------------------------------------------------------------------------
void CTileCache::Trace(char *op,U_INT FrNo,U_INT key)
{	U_INT x = (gKey >> 16);
	U_INT y = (gKey & 0x0000FFFF);
	if(tr) TRACE("             Tile %03u-%03u %s tile", x,y,op);
	return;
}
//=========================================================================
//	Cache database manager
//  DbCacheMgr keep list of objects surrounding the aircraft:
//      NDB,VOR,AIRPORT,WAYPOINTS etc
//      To minimize CPU for Vector MAP, it does some editing function such as editing
//      labels, ident and frequency as soon as object are introduced.
//      A table of 200 entries is used to locate screen coordinates of various objects
//      so that vector Map can use it.
//-------------------------------------------------------------------------
//	-Init list of pointer
//	-Init each queue.  Allows for 100 entries in each free queue
//	 NOTE:  The max count may be later tuned on a per queue basis  with
//			parameters from ini file
//=========================================================================
CDbCacheMgr::CDbCacheMgr(void)
{	int opt = 0;
  //----Init all queues -------------------------------------------
	int	inx = 0;
	int end = QDIM;
	for (inx = 0; inx != QDIM; inx++)	aHead[inx] = &aList[inx];
  //----Magnetic model --------------------------------------------
  totDEV    = 0;
  nbrDEV    = 0;
  magDEV    = 0;
  //---------------------------------------------------------------
  total         = 0;
  rGPS.Code     = NO_REQUEST;
  rGPS.nSTACK   = 0;
  objSTK.nWPT   = 0;
	//---------forge a dummy key to force refresh at first TimeSlice call--
	RefKey = 0xFFFFFFFF;
  //------ Demux parameter ------------------------------
  cycle   = 0;
	nAPT		= 0;
  stab    = 0;
  //----------Registered windows ------------------------
  wLOG      = 0;
  wDIR      = 0;
  wMAP      = 0;
  //---------Init size table ----------------------------
  oSize[TIL]  = sizeof(CTileCache);
  oSize[APT]  = sizeof(CAirport);
  oSize[VOR]  = sizeof(CNavaid);
  oSize[NDB]  = sizeof(CNavaid);
  oSize[ILS]  = sizeof(CILS);
  oSize[COM]  = sizeof(CCOM);
  oSize[RWY]  = sizeof(CRunway);
  oSize[WPT]  = sizeof(CWPT);
  oSize[OBN]  = sizeof(CObstacle);
  //-------Init object count -----------------------------
  count[TIL]  = 0;
  count[APT]  = 0;
  count[VOR]  = 0;
  count[NDB]  = 0;
  count[ILS]  = 0;
  count[COM]  = 0;
  count[RWY]  = 0;
  count[WPT]  = 0;
  count[OBN]  = 0;
  //-------Init Trace Indicator --------------------------
  tr  =  globals->Trace.Has(TRACE_DBCACHEMGR);
  //-------Get Use options -------------------------------
  //  Waypoint are not used actually -----------
  uWPT = opt;
  opt  = 0;                    // Use No Obstacle by default
  GetIniVar("Sim","UseObstructions",&opt);
  uOBS = opt;
  //---Check for SQL usage -------------------------------
  gSQL = globals->sqm->SQLgen();
	return;
}
//-------------------------------------------------------------------------
//  Add to magnetic deviation
//-------------------------------------------------------------------------
void CDbCacheMgr::IncDeviation(float dev)
{ if (dev < FLT_EPSILON)  return;
  totDEV += dev;
  nbrDEV++;
//  globals->magDEV  = totDEV / nbrDEV;
  return;
}
//-------------------------------------------------------------------------
//  Dec to magnetic deviation
//-------------------------------------------------------------------------
void CDbCacheMgr::DecDeviation(float dev)
{ if (dev < FLT_EPSILON)  return;
  totDEV -= dev;
  nbrDEV--;
//  if (nbrDEV) globals->magDEV  = totDEV / nbrDEV;
//  else        globals->magDEV  = 0;
  return;
}
//========================================================================
//-------------------------------------------------------------------------
//		Delete all cached objects
//-------------------------------------------------------------------------
CDbCacheMgr::~CDbCacheMgr()
{	//---Detach all Tile and release them -------------------
  ClQueue *tq = aHead[TIL];
  CTileCache *tc = (CTileCache*)tq->PopFromQ1();
  while (tc)
  { RemoveTile(tc);
    tc = (CTileCache*)tq->PopFromQ1();
  }
  int inx	= 0;
	for (inx = 0; inx != QDIM; inx++) aHead[inx]->Clear();
  //---A Last statistic ------------------------------------
  Statistic();
	return;	
}

//---------------------------------------------------------------------
//	Pop the first tile from the action queue 
//  NOTE: it is not necessary to lock the tile queue. Only the
//        TimeSlice() is calling this routine
//        Edit statistical data
//---------------------------------------------------------------------
void CDbCacheMgr::PopTile(char opt)	
{	CTileCache *obj = (CTileCache*)ActQ.PopFromQ1();
  U_INT key = obj->GetTileKey();
  int   nob = obj->NbObj;
  if(!tr)       return;
  if (0 == opt) return;
  if (0 == nob) return;
  TRACE("------------ TILE %03u-%03u allocate %u objects ------------",
                            (key >> 16),(key & 0x0000FFFF),obj->NbObj);
  Statistic();
 	return;	}
//-------------------------------------------------------------------------
//  Trace Statisticals
//-------------------------------------------------------------------------
void CDbCacheMgr::Statistic()
{ if(!tr)       return;
  TRACE("   % 5d Tile Cache",count[TIL]);
  TRACE("   % 5d Airports",  count[APT]);
  TRACE("   % 5d RWY",       count[RWY]);
  TRACE("   % 5d ILS",       count[ILS]);
  TRACE("   % 5d COM",       count[COM]);
  TRACE("   % 5d VOR",       count[VOR]);
  TRACE("   % 5d NDB",       count[NDB]);
  TRACE("   % 5d WPT",       count[WPT]);
  TRACE("   % 5d OBS",       count[OBN]);
  TRACE(" Total Memory allocated % 8d",total);
  TRACE("-------------------------------------------------------------");
  return;
}
//-------------------------------------------------------------------------
//  Get Statisticals
//-------------------------------------------------------------------------
void CDbCacheMgr::GetStats(CFuiCanva *cnv)
{ char txt[128];
  cnv->AddText(1,"Tile Cache:");
  sprintf_s(txt,128,"% 8d",count[TIL]);
  cnv->AddText(STATS_NUM,txt,1);
  //-------------------------------------------
  cnv->AddText(1,"Airports:");
  sprintf_s(txt,128,"% 8d",count[APT]);
  cnv->AddText(STATS_NUM,txt,1);
  //-------------------------------------------
  cnv->AddText(1,"Runways:");
  sprintf_s(txt,128,"% 8d",count[RWY]);
  cnv->AddText(STATS_NUM,txt,1);
  //-------------------------------------------
  cnv->AddText(1,"ILS:");
  sprintf_s(txt,128,"% 8d",count[ILS]);
  cnv->AddText(STATS_NUM,txt,1);
  //-------------------------------------------
  cnv->AddText(1,"VOR:");
  sprintf_s(txt,128,"% 8d",count[VOR]);
  cnv->AddText(STATS_NUM,txt,1);
  //-------------------------------------------
  cnv->AddText(1,"NDB:");
  sprintf_s(txt,128,"% 8d",count[NDB]);
  cnv->AddText(STATS_NUM,txt,1);
  //-------------------------------------------
  cnv->AddText(1,"COM:");
  sprintf_s(txt,128,"% 8d",count[COM]);
  cnv->AddText(STATS_NUM,txt,1);
  //-------------------------------------------
  cnv->AddText(1,"WPT:");
  sprintf_s(txt,128,"% 8d",count[WPT]);
  cnv->AddText(STATS_NUM,txt,1);

  return;
}
//-------------------------------------------------------------------------
//  Recycle object in free pool
//  NOTE:  OBJECT ARE NOW DELETED DIRECTLY
//-------------------------------------------------------------------------
void CDbCacheMgr::Recycle(CmHead *obj)
{ QTYPE    qx = obj->GetActiveQ();
  OTYPE    qt = obj->GetObjType();
  if(tr) obj->Trace("DEL",cFrame,ctKey);
  delete obj;
  if (qt != DBM)  return;
  total -= oSize[qx];
  count[qx]--;
  return;
}
//-------------------------------------------------------------------------
//	Put Object in the cache queue 
//-------------------------------------------------------------------------
int CDbCacheMgr::PutInCache(CmHead *obj)
{	obj->IncUser();
  obj->SetState(ACTV);
  ClQueue *hd = aHead[obj->GetActiveQ()];
	hd->LastInQ1(obj);
	return hd->GetNbrObj();
}
//-------------------------------------------------------------------------
//	Detach an object from cache queue
//-------------------------------------------------------------------------
void CDbCacheMgr::DetachFromCache(CmHead *obj)
{	aHead[obj->GetActiveQ()]->DetachFromQ1(obj);
	return;	}
//------------------------------------------------------------------------
//	Allocate a new object of type QTYPE
//-------------------------------------------------------------------------
CmHead *CDbCacheMgr::AllocateObj(QTYPE qx)
{	CmHead	*obj	= 0;
  if (qx >= QDIM) return 0;
	switch (qx)	{
		case TIL:
			obj = new CTileCache(DBM,TIL);
			break;
		case APT:
 			obj	= new CAirport  (DBM,APT);
			break;
		case VOR:
			obj	= new CNavaid   (DBM,VOR);
			break;
		case NDB:
			obj	= new CNavaid   (DBM,NDB);
			break;
		case ILS:
      obj = new CILS      (DBM,ILS);
			break;
		case COM:
      obj = new CCOM      (DBM,COM);
			break;
		case RWY:
      obj = new CRunway   (DBM,RWY);
			break;
		case WPT:
      obj = new CWPT (DBM,WPT);
			break;
    case OBN:
      obj = new CObstacle (DBM,OBN);
			break;
	}
  //-----Register object size and count ------------
//  count[qx]++;
//  total += oSize[qx];
	return obj;
}
//------------------------------------------------------------------------
//	Form a Tile Key with coordinate modulo 256 in [0-255] in both directions
//------------------------------------------------------------------------
U_INT	CDbCacheMgr::MakeKey(int gtx,int gty)
{	U_INT kx = gtx;
	U_INT ky = gty;
	return (U_INT)((kx << 16) | (ky));
}

//------------------------------------------------------------------------
//			Time slice Db cache
//	NOTE:   Actual time slice does not use the timer but the global tile key
//			where the aircraft is located. As the actual algorithm is
//			based on tiles surrounding the aircraft, there is no change until
//			the plane change tile position.  
//  NOTE: 
//      Tasks are processed in the following order
//      1-When tiles are introduced or deleted, they are put in the action
//        queue. Thus at each time slice, actions are first processed
//        before detecting the change in position.
//        There is one action per list per tile to load.
//      2-Vector Map request to edit Airport VOR and NDB are then processed
//      3-Request to acces database from FUI windows
//      4-Duty tasks consist in
//        a) Track any request from GPS
//        b) Sort the first 8 nearest airports
//        c) Sort the first 8 nearest VOR
//        d) Sort the first 8 nearest NDB
//        e) Cache refreshing according to aircraft position
//  NOTE
//      The database cache is active only on even frame number
//      while the terrain cache is working every odd frame
//------------------------------------------------------------------------
void CDbCacheMgr::TimeSlice(float dT,U_INT FrNo)
{	if (1 == (FrNo & 1))  return;
  cFrame	= FrNo;
  CTileCache *tc = (CTileCache*)ActQ.GetFirst();
  //-----------If action queue not empty, do one action --------------------
  aPos  = globals->geop;
  if  (tc)                    { DispatchAction(tc); return; }
  if  (reqQ.GetFirst()      ) { ExecuteREQ();       return; }
  //-----Database Cache is stable here -------------------------------------
  stab  = 1;
  //----------Duty tasks ---------------------------------------------------
  RefreshCache(FrNo);
  TrackGPS();
	//---- Maintains nearest airport -----------------------------------------
	cycle++;
	if (0 ==(cycle & 0xFF)) nAPT = FindFirstNearest(APT);
  return;
}
//-------------------------------------------------------------------------
//  Check for new Key
//  Compute horizontal and vertical distance of proposed key
//  to the reference key.
//  If any distance is outside of reference key limits then 
//  the proposed key is a new one
//-------------------------------------------------------------------------
bool CDbCacheMgr::IsaNewKey(U_INT key)
{ short ry = RefKey & 0x000000FF;
  short ky = key & 0x000000FF;
  short kx = (key >> 16);
  short kv = GetRelativePos(ky);
  short rx = (RefKey >> 16);
  short rv = GetRelativePos(ry);
  //--------Check for horizontal limit -----------------------
  short dh = (kx - rx) & 0x00FF;
  if (dh > 127) dh = (256 - dh);
  short lh = GetGbtHorzRange(ky);
  if ((rx == 0x0000FFFF) ||(dh > lh)) 
  { if (tr) TRACE("db-- %03u-%03u is a new key. DH = %2u LH = %2u",kx,ky,dh,lh);
    return true;
  }
  //--------Check for vertical limits -------------------------
  short dv = (kv - rv);
  short lv = (dv >= 0)?(GetGbtVertPoleRange(ry)):(GetGbtVertEquaRange(ry));
  if ((dv >= 0) && (dv > lv))
  { if (tr) TRACE("db-- %03u-%03u is a new key. DV = %2u LH = %2u",kx,ky,dv,lv);
    return true;
  }
  else
  if (-dv > lv)
  { if (tr) TRACE("db-- %03u-%03u is a new key. DV = %2u LH = %2u",kx,ky,-dv,lv);
    return true;
  }
  return false;
}
//-------------------------------------------------------------------------
//  Check for obsolete Keys
//  Compute horizontal and vertical distance of proposed key
//  to the new key.
//  If any distance is outside of new key limits then 
//  the proposed key is obsolete
//-------------------------------------------------------------------------
bool CDbCacheMgr::IsaActKey(U_INT key)
{ short ry = NewKey & 0x000000FF;
  short ky = key & 0x000000FF;
  short kh = (key >> 16);
  short kv = GetRelativePos(ky);
  short rh = (NewKey >> 16);
  short rv = GetRelativePos(ry);
  //--------Check for horizontal limit -----------------------
  short dh = (kh - rh) & 0x00FF;
  if (dh > 127) dh = (256 - dh);
  short lh = GetGbtHorzRange(ky);
  if (dh > lh) 
  { if (tr) TRACE("db-- %03u-%03u is obsolete. DH = %2u LH = %2u",kh,ky,dh,lh);
    return false;
  }
  //--------Check for vertical limits -------------------------
  short dv = (kv - rv);
  short lv = (dv >= 0)?(GetGbtVertPoleRange(ry)):(GetGbtVertEquaRange(ry));
  if ((dv >= 0) && (dv > lv))
  { if (tr) TRACE("db-- %03u-%03u is obsolete. DV = %2u LH = %2u",kh,ky,dv,lv);
    return false;
  }
  else
  if (-dv > lv)
  { if (tr) TRACE("db-- %03u-%03u obsolete. DV = %2u LH = %2u",kh,ky,-dv,lv);
    return false;
  }
    if (tr) TRACE("db-- %03u-%03u Is active.",kh,ky);
  return true;
}
//-------------------------------------------------------------------------
//  Get Upper and lower range depending on CZ hemisphere
//-------------------------------------------------------------------------
#define DB_GBTMODULO (255)

void CDbCacheMgr::GetRange(U_INT cz,U_SHORT &up,U_SHORT &dn)
{ U_SHORT por = GetGbtVertPoleRange(cz);    // Pole range
  U_SHORT eqr = GetGbtVertEquaRange(cz);    // Equator range
  if (cz > 127)
  //---NORTH HEMISPHER. Up is toward pole. Down is toward equator ----------
  {up   = (cz + por) & DB_GBTMODULO;
   dn   = (cz - eqr) & DB_GBTMODULO;
  }
  else
  //---SOUTH HEMISPHER. Up is toward equator. Down is toward pole ---------
  {up   = (cz + eqr) & DB_GBTMODULO;
   dn   = (cz - por) & DB_GBTMODULO;
  }
  return;
}
//-------------------------------------------------------------------------
//  Refresh the cache according to aircraft position
//  NOTES:  
//    1)  A tile key is forged from the current aircraft position
//        Then a key is forged for all surrounding tiles.
//        For each surrounding tile the tile cache is checked .
//        There are 2 situations:
//        a) The surrounding tile is already in cache. Then it is stamped with
//           the current frame number.
//        b) The surrounding tile is not in cache. It is entered in the cache with
//            the current frame number and an action code is entered. This action
//            be executed at next time slice. Action are requesting insertion for
//            objects (APT, ILS, RWY, NAV etc) from this new tile.
//    2) In a second round the tile cache is scanned. All tiles not stamped with 
//       the current frame number are scheduled for delete.
//    The range parameters define the radius (in tile number) of the scan.
//    Ranges are computed in global_tile table at initialization
//    3 ranges are used, they all depend of the tile latitude:
//        Horizontal range: Define the number of tiles in horizontal direction
//        Pole range      : Define the number of tiles toward the pole
//        Equator range   : Define the number of tiles toward equator
//----------------------------------------------------------------------------
//  NOTE:  DATABASE TimeSlice() must be called after TERRAINCACHE TimeSlice();
//----------------------------------------------------------------------------
void CDbCacheMgr::RefreshCache(U_INT FrNo)
{ CTileCache *tc;
	//-----Get position from global --------------------------
  int    gx = (globals->qgtX >> 1);            // Global tile X index
  int    gz = (globals->qgtZ >> 1);            // Global tile Z index
	U_INT key	= MakeKey(gx,gz);
	if (key == RefKey)		return;								          // Aircraft in same tile
  stab    = 0;                                          // Cache is changing
  NewKey  = key;
  if (tr) TRACE("====DB CACHE REFRESH.  Aircraft enter %d-%d=============",gx,gz);
	//----------scan all surrounding tiles of new position --------
  U_SHORT vup = 0;
  U_SHORT vdn = 0;
  GetRange(gz,vup,vdn);
  for (int vt = vup; vt >= vdn; vt--)
  { U_SHORT hz1 = (gx - GetGbtHorzRange(vt)) & DB_GBTMODULO;
    U_SHORT hz2 = (gx + GetGbtHorzRange(vt) + 1) & DB_GBTMODULO;

    while (hz1 != hz2)
    { key = MakeKey(hz1,vt);
      if (IsaNewKey(key))     NewTileCache(key);      //Insert new tile
      hz1 = (hz1 + 1) & DB_GBTMODULO;
    }
  }
	//--------Now delete all obsolete tiles -----------------------
  if (tr) TRACE("=========CACHE REFRESH CYCLE =Detect obsolete tiles=============");
  CTileCache  *nx  = (CTileCache*)aHead[TIL]->GetFirst();
  while (nx)
  { tc  = nx;
    nx  = (CTileCache*)tc->NextInQ1();
    if (IsaActKey(tc->gKey)) continue;
    tc->Action  = CTileCache::REMOVE;
    DetachFromCache(tc);
    ActQ.LastInQ1(tc);
  }
  RefKey  = NewKey;
	return;
}
//------------------------------------------------------------------------
//  Dispatch action in Tile until one is executed
//------------------------------------------------------------------------
void CDbCacheMgr::DispatchAction(CTileCache *tc)
{ while (0 == ExecuteAction(tc))  continue;
  return;
}
//------------------------------------------------------------------------
//  Execute requested tile action
//  To spread load, only one action per frame is executed
//  This routine must return 0 if no action is executed
//------------------------------------------------------------------------
int CDbCacheMgr::ExecuteAction(CTileCache *tc)
{ SqlMGR *sqm = globals->sqm;
  CDatabaseManager &dbmgr = CDatabaseManager::Instance();
  int key = tc->gKey;
  //--- Init the default callabck function ------------------------
  cbFUN = &CDbCacheMgr::ObjToCache;                      // Add object to cache
  //---------------------------------------------------------------
  switch (tc->Action)  {
    
  case CTileCache::INS_NAV:
    cbFUN = &CDbCacheMgr::NavToCache;                       // Add NAV to cache
    if (gSQL) sqm->GetNavByTile(key,OBJtoCache,tc);         // Load from SQL
	  else      Populate (tc, dbmgr.GetNAVDatabase(), VOR);   // Load Tile NAV NDB
    tc->Action    = CTileCache::INS_APT;                    // Next action
    return tc->NbObj;                                       // Total VOR and NDB introduced

  case CTileCache::INS_APT:
    cbFUN = &CDbCacheMgr::AptToCache;                       // Add Airport To Cache
    if (gSQL) sqm->GetAptByTile(key,APTtoCache,tc);
    else      Populate (tc, dbmgr.GetAPTDatabase(), APT);   // Load Tile Airports
    tc->Action    = CTileCache::INS_RWY;                    // Next action
    return tc->qHead[APT]->GetNbrObj();                     // Number of airports introduced

  case CTileCache::INS_RWY:                                 // Load Runways
    RunwayFromAirport(tc);                                  // Runways
    tc->Action  = CTileCache::INS_ILS;                      // Next action
    return tc->qHead[RWY]->GetNbrObj();                     // Number of runways introduced

  case CTileCache::INS_ILS:
    if (gSQL) IlsFromSQL(tc);                           // Get ILS for AIRPORTS
    else      IlsFromPOD (tc, dbmgr.GetILSDatabase());       // Load ILS
    tc->Action  = CTileCache::INS_COM;                      // Next action
	  return tc->qHead[ILS]->GetNbrObj();                     // Number of ILS introduced

  case CTileCache::INS_COM:
    cbFUN = &CDbCacheMgr::ComToCache;
    if (gSQL) sqm->GetComByTile(key,OBJtoCache,tc);
    else      Populate (tc, dbmgr.GetCOMDatabase(), COM);   // Load COM
    tc->Action   = CTileCache::INS_WPT;                     // Next action
	  return tc->qHead[COM]->GetNbrObj();                     // Number of COMM introduced

//  There is no need to load waypoint into memory until now. Only the GPS use
//  Waypoints and they are loaded from the database 
  case CTileCache::INS_WPT:
    tc->Action  = CTileCache::INS_OBS;              // Next action
    if (0 == uWPT)  return 0;
    Populate (tc, dbmgr.GetWPTDatabase(), WPT);     // Load WPT
	  return tc->qHead[WPT]->GetNbrObj();             // Number of WPT introduced
  
  case CTileCache::INS_OBS:
    tc->Action  = CTileCache::POP_TIL;              // Next action
    if (0 == uOBS)  return 0;
    Populate (tc, dbmgr.GetOBSDatabase(), OBN);     // Load Obstacles
	  return tc->qHead[OBN]->GetNbrObj();             // Number of OBS introduced

  case CTileCache::POP_TIL:
    PopTile(1);                                     // Remove Tile from action queue
    tc->SetReady();                                 // Set complete state
    ctKey	= key;
    if (tc->IsEmpty())  RemoveTile(tc);             // Empty tiles are recycled
    else                PutInCache(tc);             // Set In active Queue 
    return 1;

  case CTileCache::REMOVE:
    PopTile(0);                                      // Remove from Action queue
    RemoveTile(tc);
    return 1;
   }
return 0;
}
//-------------------------------------------------------------------------
//  Find the nearest object from the given one 
//-------------------------------------------------------------------------
CmHead *CDbCacheMgr::FindNearestOBJ(CmHead *obj)
{ float dmin    = 1000000;
  float dapt    = 0;
  CmHead   *nob = obj;
  while (obj)
  { dapt = GetFlatDistance(obj);
    if (dapt < dmin)  {dmin = dapt; nob = obj;}
    obj = obj->NextInQ1();
  }
  return nob;
}
//-------------------------------------------------------------------------
//  Find the nearest object of type QX
//	FindFirstNearest(APT) return the nearest airport
//-------------------------------------------------------------------------
CmHead *CDbCacheMgr::FindFirstNearest(QTYPE qx)
{ ClQueue    *hd = aHead[qx];
  CmHead *wpt = hd->GetFirst();
  return FindNearestOBJ(wpt);
}
//------------------------------------------------------------------------
//  Sort the 8 first Object (indicated by QTYPE)  by distance from aircraft
//  Flat distance is used for sorting
//  NOTE: If Object stack is in use by the GPS, the stack is not updated
//------------------------------------------------------------------------
void CDbCacheMgr::OrderOBJ(QTYPE qx,WPT_STACK *stk,char *idn,int lim)
{ short No       = 0;
  ClQueue    *hd = aHead[qx];
  CmHead *wpt = hd->GetFirst();
  CmHead *nwp = 0;
  while (wpt && (No != lim)) 
  { hd->Lock();
    nwp = FindNearestOBJ(wpt);
    stk->List[No] = nwp;
    hd->Unlock();
    if (nwp != wpt)
    { hd->DetachFromQ1(nwp);
      hd->InsertInQ1(nwp,wpt);
      wpt = nwp;
    }
    wpt = wpt->NextInQ1();
    No++;
  }
  stk->nWPT = No;
  while (No != lim) stk->List[No++] = 0;
  return;
}
//------------------------------------------------------------------------
//  Free Tile cache from all queues
//------------------------------------------------------------------------
void CDbCacheMgr::RemoveTile(CTileCache* tc)
{ ctKey	= tc->gKey;
  if (tr) TRACE("==>Removing  Tile %03d-%03d ========",(ctKey >> 16),(ctKey & 0x0000FFFF));
  if (tc->HasObject())  tc->FreeAllQueues();					        // Delete all queues in tile
  Recycle(tc);							                                  // recyle the tile cache
  if(!tr)    return;
  Statistic();
  return; }
//------------------------------------------------------------------------
//	Create a new tile cache to populate objects from database
//------------------------------------------------------------------------
void CDbCacheMgr::NewTileCache(U_INT key)
{	CTileCache *tc = (CTileCache*)AllocateObj(TIL);
	tc->SetKey(key);								            // Set Key
	tc->Stamp(cFrame);							            // Set Stamp
  tc->Action  = CTileCache::INS_NAV;          // Action code
  count[TIL]++;
  total += oSize[TIL];
  ActQ.LastInQ1(tc);
  return;
}
//-----------------------------------------------------------------------
//  Call Back:  Add Airport to cache
//  Look up for METAR data
//-----------------------------------------------------------------------
void CDbCacheMgr::AptToCache(CmHead *obj,CTileCache *tc)
{ tc->PutInTile(obj);
  PutInCache(obj);
  QTYPE qt = obj->GetActiveQ();
  count[qt]++;
  total += oSize[qt];
  globals->wtm->GetMetar(obj);
	if(tr)	obj->Trace("Add",cFrame,tc->gKey);
  return;
}
//-----------------------------------------------------------------------
//  Call Back:  Add NAV/NDB to cache
//-----------------------------------------------------------------------
void CDbCacheMgr::NavToCache(CmHead *obj,CTileCache *tc)
{ tc->PutInTile(obj);
  PutInCache(obj);
  QTYPE qt = obj->GetActiveQ();
  count[qt]++;
  total += oSize[qt];
	if(tr)	obj->Trace("Add",cFrame,tc->gKey);
  return;
}
//-----------------------------------------------------------------------
//  Call Back:  Add COM to cache
//  Do no conserve CCOM with out of range frequency
//-----------------------------------------------------------------------
void CDbCacheMgr::ComToCache(CmHead *obj,CTileCache *tc)
{ if (!GoodCOMfrequency(obj->GetFrequency())) {delete obj; return;}
  tc->PutInTile(obj);
  PutInCache(obj);
  count[COM]++;
  total += oSize[COM];
  SetAirportIdent((CCOM *)obj);
	if(tr)	obj->Trace("Add",cFrame,tc->gKey);
  return;
}
//-----------------------------------------------------------------------
//  Call Back:  Add object to cache
//-----------------------------------------------------------------------
void CDbCacheMgr::ObjToCache(CmHead *obj,CTileCache *tc)
{ tc->PutInTile(obj);
  PutInCache(obj);
  QTYPE qt = obj->GetActiveQ();
  count[qt]++;
  total += oSize[qt];
	if(tr)	obj->Trace("Add",cFrame,tc->gKey);
  return;
}
//-----------------------------------------------------------------------
//  Call Back:  Add Runway to Cache
//-----------------------------------------------------------------------
void CDbCacheMgr::RwyToCache(CRunway *rwy,CTileCache *tc)
{ count[RWY]++;
  total += oSize[RWY];
  if(tr)	rwy->Trace("Add",cFrame,tc->gKey);
  return;
}
//-----------------------------------------------------------------------
//	Populate cache Queue with objects from global tile
//  NOTE: NDB are put in a separated queue in the cache.  They should be
//  recycled in the NAV free pool after usage
//-----------------------------------------------------------------------
void CDbCacheMgr::Populate(CTileCache *tc,CDatabase *db,QTYPE qx)
{	if (db->NotMounted())   return;
  char key[16];
	int kx = (tc->GetTileKey() >> 16);
	int	kz = (tc->GetTileKey() & 0x0000FFFF);
	sprintf (key, "%03d%03d", kx, kz);
	unsigned long offset = db->Search ('nltl', key);
  int nb = 0;
	if (offset == 0)    return;
	//--- Retrieve the raw database record ----------------
	while (offset != 0)
	{		// Allocate object and fill it----
			db->GetRawRecord(offset);
			CmHead *obj = AllocateObj(qx);
			db->DecodeRecord(offset,obj);
      //-----Specific call to insert in cache ----------
      (this->*cbFUN)(obj,tc);
      //----------------------------------------------
			offset = db->SearchNext();
	}
  //--- Free buffer for raw database record
	return;	}
//------------------------------------------------------------------------
//  Check for ILS validity
//------------------------------------------------------------------------
bool CDbCacheMgr::ValidILS(CILS *obj)
{ if (obj->rang < 1.0)      return false;
  return GoodILSfrequency(obj->freq);
}
//------------------------------------------------------------------------
//  Filter ILS from database
//------------------------------------------------------------------------
void CDbCacheMgr::IlsFromPOD(CTileCache *tc,CDatabase *db)
{	char key[16];
	int kx = (tc->GetTileKey() >> 16);
	int	kz = (tc->GetTileKey() & 0x0000FFFF);
	sprintf (key, "%03d%03d", kx, kz);
	U_LONG offset = db->Search ('nltl', key);
  int no = 0;
	if (offset == 0)    return;
	// Retrieve the raw database record ----------------
  CILS *obj = 0;
	while (offset != 0)
	{		// Allocate object and fill it----
			db->GetRawRecord(offset);
			if (0 == obj)  obj = (CILS*)AllocateObj(ILS);
      if (0 == obj)   break;
			db->DecodeRecord(offset,obj);
			//------Put specific code here------------------
      if (ValidILS(obj))
      { if (LinkILStoRunway(obj,tc)) ObjToCache(obj,tc);
        else  delete obj;
        obj = 0;
      }
			//----------------------------------------------
			offset = db->SearchNext();
	}
  // Free resources
  if (obj)  {delete obj; count[ILS]--;}
	return;	}
//------------------------------------------------------------------------
//  Scan tile for airports 
//  Set map type and and insert runways
//------------------------------------------------------------------------
void CDbCacheMgr::IlsFromSQL(CTileCache *tc)
{ ClQueue   *hd = tc->qHead[APT];
  CAirport *apt = 0;
  for (apt = (CAirport *)hd->GetFirst(); apt != 0; apt = (CAirport *)apt->NextInQ2())
  { globals->sqm->GetIlsForAirport(apt,OBJtoCache,tc);
  }
  return;
}

//------------------------------------------------------------------------
//  Scan tile for airports 
//  Set map type and and insert runways
//------------------------------------------------------------------------
void CDbCacheMgr::RunwayFromAirport(CTileCache *tc)
{ ClQueue   *hd = tc->qHead[APT];
  CAirport *apt = 0;
  for (apt = (CAirport *)hd->GetFirst(); apt != 0; apt = (CAirport *)apt->NextInQ2())
  { if (gSQL) globals->sqm->GetRwyForAirport(apt,RWYtoCache,tc);
    else      RwyFromThisAirport(tc,apt);  
    apt->MapTp = apt->SetMapType();
  }
  return;
}
//------------------------------------------------------------------------
//    Find runway fom airport. Insert in Airport Q only
//------------------------------------------------------------------------
void CDbCacheMgr::RwyFromThisAirport(CTileCache *tc,CAirport *apt)
{ CRunway * rwy = 0;
  int       no  = 0;
  // Get handle to runway database
  CDatabase *db = CDatabaseManager::Instance().GetRWYDatabase ();
  if (db == NULL) return;

  unsigned long offset = db->Search ('tpar',apt->akey);
  if (offset == 0)      return;
  while (offset  != 0) {
      db->GetRawRecord (offset);
      rwy = (CRunway *)AllocateObj(RWY);
      if (0 == rwy)     break;
			db->DecodeRecord(offset,rwy);
      count[RWY]++;
      total += oSize[RWY];
      apt->AddRunway(rwy);                              // Add in Q1
      if (rwy->rlen > 8079.0f)  apt->SetLongRWY();      // Long RWY property
      if (rwy->rsfc != 0)       apt->SetHardRWY();      // Hard runway property
      if(tr)	rwy->Trace("Add",cFrame,tc->gKey);
			offset = db->SearchNext();
    }
  return; 
}
//------------------------------------------------------------------------
//    Find runway fom airport. Insert in GPS Request
//------------------------------------------------------------------------
void CDbCacheMgr::AirportRWYforGPS(CGPSrequest *req,CAirport *apt)
{ CRunway * rwy = 0;
  int       no  = 0;
  // Get handle to runway database
  CDatabase *db = CDatabaseManager::Instance().GetRWYDatabase ();
  if (db == NULL) return;

  unsigned long offset = db->Search ('tpar',apt->akey);
  if (offset == 0)      return;

  // Retrieve the raw database record
  while (offset  != 0) {
      db->GetRawRecord (offset);
      rwy = new CRunway(SHR,RWY);
			db->DecodeRecord(offset,rwy);
      req->rwyQ.LastInQ1(rwy);
      if (rwy->rlen > 8079.0f)  apt->SetLongRWY();      // Long RWY property
      if (rwy->rsfc != 0)       apt->SetHardRWY();      // Hard runway property
			offset = db->SearchNext();
    }
  return; 
}
//------------------------------------------------------------------------
//    Find runway associated to an ILS
//    -Set ILS parameters and glide slope
//------------------------------------------------------------------------
bool CDbCacheMgr::LinkILStoRunway(CmHead *obj,CTileCache *tc)
{ CILS     *ils = (CILS*)obj;
  CAirport *apt = FindAPTforILS(ils->iapt);
  ils->LinkAPT(apt);
  if (0 == apt) return false;               // orphan ILS 
  //----Locate Runway from Airport rwy Queue ------------------------
  CRunway *rwy  = apt->FindRunway(ils->irwy);
  if (0 == rwy) return false;
  rwy->InitILS(ils);
  apt->AddOneILS();
  return true;
}
//------------------------------------------------------------------------
//    Find Airport  associated to an ILS
//------------------------------------------------------------------------
CAirport *CDbCacheMgr::FindAPTforILS(char *kapt)
{ ClQueue  *hd   = aHead[APT];
  CAirport *apt  = 0;
  //--Lock APT Queue ----------------------------------
  hd->Lock();
  for (apt = (CAirport*)hd->GetFirst(); apt != 0; apt = (CAirport*)apt->NextInQ1())
  { if (strcmp(kapt,apt->akey) != 0)  continue;
    break;
   }
	//-- Unlock APT Queue -------------------------------
  hd->Unlock();
  return apt; }
//-----------------------------------------------------------------------------
//  Compute flat distance in nautical miles from aircraft 
//  position (pos) to destination position(to). Store distance in obj
//  1 N mile => 1 minute of arc
//  Vertical and horizontal distances are stored as integer and scaled by a 
//  factor 128 for better precision in drawing the vactor map.
//  The short int allows for a +/-256 miles capacity with this factor
//	NOTE: The NmFactor is divided by 60 to convert arsec in nmiles
//-----------------------------------------------------------------------------
float CDbCacheMgr::GetFlatDistance(CmHead *obj)
{   SPosition *To = obj->ObjPosition();
    double disLat = (To->lat - aPos.lat) / 60.0;              // Lattitude Distance in nm
    double difLon = LongitudeDifference(To->lon,aPos.lon);    // Longitude difference in arcsec
    double disLon = obj->GetNmFactor() * difLon;              // Compute x component
    obj->SetDistLon(short(disLon * 128));                     // Store longitude component scaled by 128
    obj->SetDistLat(short(disLat * 128));                     // Store latitude  component scaled by 128
    return ((disLon * disLon) + (disLat * disLat));           // Return squarred distance
}
//-----------------------------------------------------------------------------
//  Compute flat distance in feet between 2 positions
//  1 N mile => 1 minute of arc
//-----------------------------------------------------------------------------
void CDbCacheMgr::GetFeetDistance(int *dx, int *dy,SPosition org, SPosition des)
{   double disLat = (des.lat - org.lat) / 60.0;               // Lattitude Distance in nm
    double difLon = (des.lon - org.lon);                      // Longitude difference in arcsec
    double latRad = FN_RAD_FROM_ARCS(org.lat);								// Latitude in Radian
    double circle = cos(latRad) * MILE_CIRCUMFERENCE;         // Circle in nm
    double disLon = (circle * difLon) / (3600 * 360);         // Longitude distance in nm
    *dx = int(disLon * FEET_PER_NM);                          // Distance in feet
    *dy = int(disLat * FEET_PER_NM);                          // Distance in feet
    return ;           
}
//------------------------------------------------------------------------
//  Search a Navaid by its name and key
//------------------------------------------------------------------------
CNavaid* CDbCacheMgr::GetNavaidByNameAndKey(char *name,char *key)
{ CDatabaseManager &dbmgr = CDatabaseManager::Instance();
  CDatabase *db           = dbmgr.GetNAVDatabase();
  unsigned long offset    = db->Search ('mann', name);            // Search
  if  (0 == offset)       return 0;
  CNavaid *nav            = new CNavaid(SHR,VOR);
  while (offset)
  { db->GetRawRecord(offset);
    db->DecodeRecord(offset,nav);
    if (strcmp(nav->GetDbKey(),key) == 0) return nav; 
    offset      = db->SearchNext();                           // Next item 
  }
  delete nav;
  return 0;
}
//------------------------------------------------------------------------
//  Search airport by its name
//------------------------------------------------------------------------
CAirport* CDbCacheMgr::GetAirportByName(char *name)
{ CDatabaseManager &dbmgr = CDatabaseManager::Instance();
  CDatabase *db           = dbmgr.GetAPTDatabase();
  unsigned long offset    = db->Search ('mana', name);            // Search airport
  if (0 == offset)        return 0;
  CAirport  *apt          = new CAirport(SHR,APT);
  db->GetRawRecord(offset);
  db->DecodeRecord(offset,apt);
  return apt;
}
//------------------------------------------------------------------------
//  Search Waypoint by its key
//------------------------------------------------------------------------
CWPT* CDbCacheMgr::GetWaypointByKey(char *key)
{ CDatabaseManager &dbmgr = CDatabaseManager::Instance();
  CDatabase *db           = dbmgr.GetWPTDatabase();
  unsigned long offset    = db->Search ('wkey', key);            // Search waypoint
  if (0 == offset)        return 0;
  CWPT  *wpt         = new CWPT(SHR,WPT);
  db->GetRawRecord(offset);
  db->DecodeRecord(offset,wpt);
  return wpt;
}
//------------------------------------------------------------------------
//  Get way point from the database
//------------------------------------------------------------------------
void CDbCacheMgr::GetFlightPlanWPT(CWPoint* wpt)
{ CmHead* obj = 0;
  switch(wpt->GetUser())  {
  case  'airp':
    obj = GetAirportByName(wpt->GetName());
    wpt->SetDBwpt(obj);
    return;
  case 'snav':
    obj = GetNavaidByNameAndKey(wpt->GetName(),wpt->GetDbKey());
    wpt->SetDBwpt(obj);
    return;
  case 'dbwp':
    obj = GetWaypointByKey(wpt->GetDbKey());
    wpt->SetDBwpt(obj);
    return;
  }
  return;
}
//----------------------------------------------------------------
//  Populate Waypoint from database
//----------------------------------------------------------------
void CDbCacheMgr::PopulateNode(CWPoint *wpt)
{ SqlMGR *sqm = globals->sqm;
  if (gSQL) sqm->GetFlightPlanWPT(wpt); 
  else           GetFlightPlanWPT(wpt);
  return;
}
//========================================================================
//  Access to Parameters
//========================================================================
U_LONG CDbCacheMgr::GetDBcountryNBrecords()
{ CDatabase *db = CDatabaseManager::Instance().GetCTYDatabase();
  return (db)?(db->GetNumRecords()):(0);
}
//========================================================================
//	SPECIFIC OBJECT MANAGEMENT.  Following routines are dedicated
//		to specific managed object.  Access methods and selection
//		are differents for differents objects.  All objects are searched in
//    the cache queues.
//========================================================================
//========================================================================
//	NAVAID ACCES METHODS
//========================================================================
//------------------------------------------------------------------------
//	Find active VOR
//	NOTE:  This call is on behalf of a component (radio for instance)
//			The component should give back the pointer pn to the previous
//			allocated navaid for each call.
//			If this nav is still alive with the same frequency the same
//			nav is allocated to the caller.  Otherwise the returned nav
//			is decremented from user count.  If marked for delete, it will
//			be deleted when no user owns it.  Then another nav is allocated
//			if found.
//------------------------------------------------------------------------
//    Find a tuned VOR for a subsystem
//-------------------------------------------------------------------------
CNavaid *CDbCacheMgr::GetTunedNAV(CNavaid *pn,U_INT FrNo,float freq)
{	CNavaid *nav = (pn)?(pn->IsThisNavOK(freq)):(0);
  if (nav)  nav->Refresh(FrNo);
  else      nav = FindVOR(FrNo,freq);
	return nav;				                        // refresh same nav
}
//------------------------------------------------------------------------
//	Find a tuned navaid in the cache list
//------------------------------------------------------------------------
CNavaid *CDbCacheMgr::FindVOR(U_INT FrNo,float freq)
{	ClQueue		*hd	 = aHead[VOR];
	CNavaid   *nav = 0;
	//--todo lock NAV Queue ----------------------------------
  hd->Lock();
	for (nav = (CNavaid*)hd->GetFirst(); nav != 0; nav = (CNavaid*)nav->NextInQ1())
	{	if (!nav->IsSelected(freq))	continue;	
    nav->Refresh(FrNo);
    if (nav->IsInRange()) break;
  }
  if (nav)  nav->IncUser();
	//-- todo unlock NAV Queue -------------------------------
  hd->Unlock();
	return nav;
}
//-------------------------------------------------------------------------
//  Return the next VOR located in the requested radius (in nm)
//  NOTE: radius is squarred as GetFlatDistance return
//        the squarred distance from the aircraft to the VOR
//-------------------------------------------------------------------------
CNavaid *CDbCacheMgr::FindVORbyDistance(CNavaid *old,float radius)
{ ClQueue    *hd   = aHead[VOR];
  //----Lock VOR queue here ---------------------------
  hd->Lock();
  CNavaid   *vor  = (old)?((CNavaid*)old->NextInQ1()):((CNavaid*)hd->GetFirst());
  while (vor)
  { if (vor->xType == 0)    continue;
    float dis = GetFlatDistance(vor);
    vor->SetPDIS(dis);
    if (dis < radius)       break;
    vor = (CNavaid*)vor->Cnext; 
  }
  //----Unlock VOR queue here -------------------------
  hd->Unlock();
  return vor;
}
//-------------------------------------------------------------------------
//  return Index type for a  navaid
//-------------------------------------------------------------------------
U_SHORT CDbCacheMgr::GetNavType(U_SHORT type)
{     switch (type) {
      case NAVAID_TYPE_VOR:
        return 1;
      case NAVAID_TYPE_TACAN:
        return 2;
      case NAVAID_TYPE_VORTAC:
        return 3;
      case NAVAID_TYPE_VORDME:
        return 4;
      case NAVAID_TYPE_NDB:
        return 5;
      case NAVAID_TYPE_NDBDME:
        return 6;
  }
  return 0;
}
//-------------------------------------------------------------------------
//  Return the name of category for the object
//-------------------------------------------------------------------------
char*  CDbCacheMgr::GetObjCategoryName(CmHead *obj)
{ if (0 == obj)         return "NULL";
  QTYPE qx = obj->GetActiveQ();
  if (VOR != qx)        return globals->wptTAB[qx];
  U_SHORT inx = ((CNavaid*)obj)->xType;
  if (inx > QDIM)       return "UNKNOWN";
  return globals->vorTAB[inx];
}
//-------------------------------------------------------------------------
//  Release VOR
//-------------------------------------------------------------------------
void  CDbCacheMgr::ReleaseVOR(CNavaid* vor)
{ if (vor) vor->DecUser();
  return; 
}
//=========================================================================
//	ILS ACCESS METHODS.  Find a tuned ILS
//    Find the nearest ILS
//=========================================================================
CILS *CDbCacheMgr::GetTunedILS(CILS *pi,U_INT FrNo,float freq)
{	CILS *ils = (pi)?(pi->IsThisILSOK(freq)):(0);
  if (ils)  ils->Refresh(FrNo);
  else      ils = FindILS(FrNo,freq);
	return ils;				// refresh same ils
}
//-------------------------------------------------------------------------
//  For opposed ILS get the one which is oriented toward aircraft
//-------------------------------------------------------------------------
CILS *CDbCacheMgr::GoodILS(CILS* ils1, CILS* ils2)
{   CILS *ils = ((ils1->radial > 270) || (ils1->radial <= 90))?(ils1):(ils2);
    return ils;
}
//-------------------------------------------------------------------------
//    Return the best ILS
//    NOTE: When several ILS of the same frequency are detected
//          the one selected is the one nearest to the aircarft
//		NOTE:		Distance test is inverted due to ILS reference 
//						located at opposite runway end
//-------------------------------------------------------------------------
CILS *CDbCacheMgr::BestILS(CILS* ils1,CILS* ils2)
{   if (ils2 == 0)          return ils1;
    //-----otherwise get the nearest for the same frequency ----------
    return (ils2->GetNmiles() > ils1->GetNmiles())?(ils2):(ils1);
}
//-------------------------------------------------------------------------
//  Find a tuned ILS in the cache list
//-------------------------------------------------------------------------
CILS *CDbCacheMgr::FindILS(U_INT FrNo,float freq)
{	ClQueue		*hd	 = aHead[ILS];
	CILS      *ils1 = 0;
  CILS      *ils2 = 0;                                   // Selected ILS
	//--Lock ILS Queue ----------------------------------
  hd->Lock();
	for (ils1 = (CILS*)hd->GetFirst(); ils1 != 0; ils1 = (CILS*)ils1->NextInQ1())
	{	if (!ils1->IsSelected(freq))	continue;	
	  if (!ils1->ilsD)							continue;
    ils1->Refresh(FrNo);
    if (ils1->IsInRange() == 0)   continue;
    ils2  = BestILS(ils1,ils2);
  }
  if (ils2) ils2->IncUser();
	//-- Unlock ILS Queue -------------------------------
  hd->Unlock();
	return ils2;
}

//=========================================================================
//	NDB ACCESS METHODS.  Find a tuned NDB
//=========================================================================
CNavaid *CDbCacheMgr::GetTunedNDB(CNavaid *pn,U_INT FrNo,float freq)
{	CNavaid *ndb = (pn)?(pn->IsThisNDBOK(freq)):(0);
  if (ndb)  ndb->Refresh(FrNo);
  else      ndb = FindNDB(FrNo,freq);
	return ndb;				                                // refresh same NDB
}
//------------------------------------------------------------------------
//	Find a tuned NDB
//------------------------------------------------------------------------
CNavaid *CDbCacheMgr::FindNDB(U_INT FrNo,float freq)
{	ClQueue		*hd	 = aHead[NDB];
	CNavaid   *ndb = 0;
	//--Lock NDB Queue ----------------------------------
  hd->Lock();
	for (ndb = (CNavaid*)hd->GetFirst(); ndb != 0; ndb = (CNavaid*)ndb->NextInQ1())
	{	if (ndb->IsSelected(freq) == false)	  continue;
    ndb->Refresh(FrNo);
    if (ndb->IsInRange())   break;
  }
  if (ndb) ndb->IncUser();
	//-- Unlock NDB Queue -------------------------------
  hd->Unlock();
	return ndb;
}
//=========================================================================
//
//==========================================================================
//-------------------------------------------------------------------------
//  Return the next Obstacle located in the requested radius (in nm)
//  NOTE: radius is squarred as GetFlatDistance return
//        the squarred distance from the aircraft to the Obstacle
//-------------------------------------------------------------------------
CObstacle *CDbCacheMgr::FindOBSbyDistance(CObstacle *old,float radius)
{ ClQueue    *hd   = aHead[OBN];
  //----Lock OBS queue here ---------------------------
  hd->Lock();
  CObstacle   *obs  = (old)?((CObstacle*)old->NextInQ1()):((CObstacle*)hd->GetFirst());
  while (obs)
  { float dis = GetFlatDistance(obs);
    obs->SetPDIS(dis);
    if (dis <= radius)         break;
    obs = (CObstacle*)obs->Cnext; 
  }
  //----Unlock OBS queue here -------------------------
  hd->Unlock();
  return obs;
}
//-------------------------------------------------------------------------
//  Return the next NDB located in the requested radius (in nm)
//  NOTE: radius is squarred as GetFlatDistance return
//        the squarred distance from the aircraft to the NDB
//-------------------------------------------------------------------------
CNavaid *CDbCacheMgr::FindNDBbyDistance(CNavaid *old,float radius)
{ ClQueue    *hd   = aHead[NDB];
  //----Lock NDB queue here ---------------------------
  hd->Lock();
  CNavaid   *ndb  = (old)?((CNavaid*)old->NextInQ1()):((CNavaid*)hd->GetFirst());
  while (ndb)
  { if (GetFlatDistance(ndb) <= radius) break;
    ndb = (CNavaid*)ndb->Cnext; 
  }
  if (ndb)  ndb->IncUser();                        // Allocate new NDB
  //----Unlock NDB queue here -------------------------
  hd->Unlock();
  if (old)  old->DecUser();                       // Deallocate previous
  return ndb;
}
//-------------------------------------------------------------------------
//  Release NDB
//-------------------------------------------------------------------------
void  CDbCacheMgr::ReleaseNDB(CNavaid* ndb)
{ if (ndb) ndb->DecUser();
  return; 
}
//==============================================================================
//  CCOM Acces method.  Get a tuned COM from the cache list
//==============================================================================
//------------------------------------------------------------------------
//    Find a tuned COM for a subsystem
//-------------------------------------------------------------------------
CCOM *CDbCacheMgr::GetTunedCOM(CCOM *pn,U_INT FrNo,float freq)
{	CCOM *com = (pn)?(pn->IsThisComOK(freq)):(0);
  if (com)  com->Refresh(FrNo);
  else      com = FindCOM(FrNo,freq);
	return com;				                        
}
//------------------------------------------------------------------------
//	Find a tuned navaid in the cache list
//------------------------------------------------------------------------
CCOM *CDbCacheMgr::FindCOM(U_INT FrNo,float freq)
{	ClQueue		*hd	 = aHead[COM];
	CCOM      *com = 0;
	//-------- lock COM Queue ----------------------------------
  hd->Lock();
	for (com = (CCOM*)hd->GetFirst(); com != 0; com = (CCOM*)com->NextInQ1())
	{	if (com->IsSelected(freq) == false)	continue;	
    com->Refresh(FrNo);
    if (!com->IsInRange()) continue;
    com->IncUser();
    break;
  }
	//-- ----- unlock NAV Queue -------------------------------
  hd->Unlock();
	return com;
}
//=========================================================================
//	RUNWAYS ACCESS METHODS
//=========================================================================
//=========================================================================
//	AIRPORT ACCESS METHODS.  Find next local airport in a given radius
//  NOTE: radius is squarred as GetFlatDistance return
//        the squarred distance from the aircraft to the object
//   
//=========================================================================
CAirport *CDbCacheMgr::FindAPTbyDistance(CAirport *old,float radius)
{ ClQueue    *hd   = aHead[APT];
  //----Lock airport queue here ---------------------------
  hd->Lock();
  float dis;
  CAirport  *apt  = (old)?((CAirport*)old->NextInQ1()):((CAirport*)hd->GetFirst());
  while (apt)
  {	dis = GetRealFlatDistance(apt);
    if (dis <= radius) break;
    apt = (CAirport*)apt->Cnext; 
  }
  //----Unlock airport queue here -----------------------------
  hd->Unlock();
  return apt;
}
//=========================================================================
//  Find airport by key
//=========================================================================
CAirport *CDbCacheMgr::FindAPTbyKey(char *key)
{ ClQueue   *hd		= aHead[APT];
  CAirport	*apt	= 0;
  //----Lock airport queue here ---------------------------
  hd->Lock();
  for (apt = (CAirport*)hd->GetFirst(); apt != 0; apt = (CAirport*)apt->NextInQ1())
  {	if (apt->SameKey(key)) break;
  }
  //----Unlock airport queue here -----------------------------
  hd->Unlock();
  return apt;
}
//=========================================================================
//  Find airport for a comm object
//  Add com type to the airport and store a pointer in com object
//=========================================================================
void CDbCacheMgr::SetAirportIdent(CCOM *com)
{ ClQueue  *hd   = aHead[APT];
  char    *akey = com->GetApKey();
  CAirport *apt = 0;
  //---- Lock Airport queue ----------------------------------
  hd->Lock();
  for (apt = (CAirport*)hd->GetFirst(); apt != 0; apt = (CAirport*)apt->NextInQ1())
  { if (strcmp(apt->GetKey(),akey) != 0)  continue;
    apt->cTyp |= com->GetComType(); 
    break;
  }
  //---- Unlock Airport queue ----------------------------------
  hd->Unlock();
  com->apOBJ  = apt;
  if (com->IsDEL())   apt->SetRDEP(com);
  if (com->IsATI())   apt->SetATIS(com);
  return;
}
//=========================================================================
//	AIRPORT ACCESS METHODS for DIRECTORY WINDOW
//   
//=========================================================================

//-------------------------------------------------------------------------
//  Get Airport by offset
//-------------------------------------------------------------------------
void CDbCacheMgr::APTByOffset(U_LONG offset,CAirport *apt)
{ CDatabase *db = CDatabaseManager::Instance().GetAPTDatabase();
  if (0 == db)          return;
  db->GetRawRecord(offset);
  db->DecodeRecord(offset,apt);
  return;
}
//-------------------------------------------------------------------------
//  Create slot for Runway
//  -Compute each end coordinate relative to origin position
//-------------------------------------------------------------------------
void CDbCacheMgr::ComputeDistance(CRwyLine *slot,SPosition org)
{ int lx;
  int ly;
  GetFeetDistance(&slot->Hend.dx,&slot->Hend.dy,org,slot->Hend.pos);
  GetFeetDistance(&slot->Lend.dx,&slot->Lend.dy,org,slot->Lend.pos);
  GetFeetDistance(&lx,&ly,slot->Hend.pos,slot->Lend.pos);
  return;
}

//-------------------------------------------------------------------------
//  Get all Comm matching filter for a given airport
//-------------------------------------------------------------------------
void CDbCacheMgr::ComFromThisAirport(CDataBaseREQ *req)
{ CComLine *slot = 0;
  CDatabaseCOM *db = (CDatabaseCOM*)CDatabaseManager::Instance().GetCOMDatabase ();
  if (0 == db)            return;
  unsigned long offset = db->Search ('tpac', req->aptkey);
  if (offset == 0)        return;
  while (0 != offset)
  { if (0 == slot)  slot  = new CComLine;
    db->GetRawRecord (offset);
    db->DecodeRecord(slot);
    bool  ok  = GoodCOMfrequency(slot->GetFreq());
          ok &= req->Match(slot->GetMask());
    if (ok)
    { slot->SetOFS(offset);
      req->Wind->AddDBrecord(slot,(DBCODE)req->Code);
      slot  = 0;
    }
    offset  = db->SearchNext();
    }
  if (slot) delete slot;
  return;
}
//-------------------------------------------------------------------------
//  Get all Comm for a given airport
//-------------------------------------------------------------------------
void CDbCacheMgr::AirportCOMforGPS(CGPSrequest *req,CAirport *apt)
{ RD_COM inx  = COM_ZR;
  CCOM *com   = 0;
  CDatabaseCOM *db = (CDatabaseCOM*)CDatabaseManager::Instance().GetCOMDatabase ();
  if (0 == db)            return;
  unsigned long offset = db->Search ('tpac', apt->akey);
  if (offset == 0)        return;
  while (0 != offset)
  { com  = new CCOM(SHR,COM);
    db->GetRawRecord(offset);
    db->DecodeRecord(offset,com);
    req->comQ.LastInQ1(com);
    offset  = db->SearchNext();
    }
  return;
}
//-------------------------------------------------------------------------
//  Concatenate airport key with runway end and limit to ikey size
//-------------------------------------------------------------------------
void CDbCacheMgr::MakeILSkey(char *edt,char *akey,char *end)
{ int nb  = 0;
  strcpy(edt,akey);
  while (nb++ != 4) if (*end != ' ') break; else end++;
  strcat(edt,end);
  edt[10] = 0;
  return;
}
//-------------------------------------------------------------------------
//  Get all ILS for runways
//-------------------------------------------------------------------------
void CDbCacheMgr::IlsFromThisRunway(CDataBaseREQ *req,CRwyLine *slot)
{ char ikey[16];
  MakeILSkey(ikey,req->aptkey,slot->GetHiEndID());
  IlsFromThisRunway(req,ikey);
  MakeILSkey(ikey,req->aptkey,slot->GetLoEndID());
  IlsFromThisRunway(req,ikey);
  return;
}
//-------------------------------------------------------------------------
//  Get all ILS for a given runway
//-------------------------------------------------------------------------
void CDbCacheMgr::IlsFromThisRunway(CDataBaseREQ *req, char* ikey)
{ CComLine *slot = 0;
  CDatabaseILS  *db = (CDatabaseILS*)CDatabaseManager::Instance().GetILSDatabase ();
  if (0 == db)            return;
  U_LONG offset = db->Search ('tpai', ikey);
  if (offset == 0)        return;
  while (offset)
  { if (0 == slot)  slot  = new CComLine;
    db->GetRawRecord (offset);
    db->DecodeRecord(slot);
    float freq = slot->GetFreq();
    if (GoodILSfrequency(freq))
    { slot->SetOFS(offset);
      slot->FillILS();
      req->Wind->AddDBrecord(slot,ILS_BY_AIRPORT);
      slot  = 0;
    }
    offset = db->SearchNext();
  }
  if (slot) delete slot;
  return;
}
//-------------------------------------------------------------------------
//  Get all ILS for a given runway end
//-------------------------------------------------------------------------
void CDbCacheMgr::RunwayILSforGPS(CGPSrequest *req, char *akey,char *kend)
{ float  frq;
  CILS  *ils = 0;
  char  ikey[12];
  CDatabase *db = CDatabaseManager::Instance().GetILSDatabase ();
  if (0 == db)            return;
  MakeILSkey(ikey,akey,kend);
  U_LONG offset = db->Search ('tpai', ikey);
  if (offset == 0)        return;
  while (offset)
  { if (0 == ils)     ils  = new CILS(ANY,ILS);
    db->GetRawRecord (offset);
    db->DecodeRecord(offset,ils);
    frq = ils->GetFrequency();
    if (GoodILSfrequency(frq))
    { //---Add to GPS request ----------
      rGPS.ilsQ.LastInQ1(ils);
      ils  = 0;
    }
    offset = db->SearchNext();
  }
  if (ils) delete ils;
  return;
}


//-------------------------------------------------------------------------
//  Get Airport by record Offset
//-------------------------------------------------------------------------
void CDbCacheMgr::APTByOffset(CDataBaseREQ *req)
{ CAirport *apt;
  CDatabase *db = CDatabaseManager::Instance().GetAPTDatabase();
  if (0 == db)          return;
  U_LONG  offset  =     req->GetOffset();
  apt   = new CAirport(SHR,APT);
  db->GetRawRecord(offset);
  db->DecodeRecord(offset,apt);
  req->Wind->AddDBrecord(apt,(DBCODE)req->Code);
  return ;
}
//-------------------------------------------------------------------------
//  Get NDB by record Offset
//-------------------------------------------------------------------------
void CDbCacheMgr::NDBByOffset(CDataBaseREQ *req)
{ CNavaid *nav;
  CDatabase *db = CDatabaseManager::Instance().GetNAVDatabase();
  if (0 == db)          return;
  U_LONG  offset  =     req->GetOffset();
  nav   = new CNavaid(ANY,NDB);
  db->GetRawRecord(offset);
  db->DecodeRecord(offset,nav);
  req->Wind->AddDBrecord(nav,(DBCODE)req->Code);
  return ;
}
//============================================================================================
//  SEARCH ROUTINES FOR FUI INTERFACE
//============================================================================================
//-------------------------------------------------------------------------
//  Filter one or all countries
//  -If no country key is needed then the record is OK
//-------------------------------------------------------------------------
bool CDbCacheMgr::FilterCountry(CSlot *slot,CDataBaseREQ *req)
{ if (!req->NeedCTY())                               return true;
  if (strcmp(req->GetCTYkey(),slot->GetKey()) == 0)  return true;
  return false;
}
//-------------------------------------------------------------------------
//  String filtering
//-------------------------------------------------------------------------
bool CDbCacheMgr::FilterByString(char *abcd,char *name)
{ if ('@' == *abcd) 
  { char *fn = strstr(name,(abcd+1));
    if (0 == fn)  return false;
    return (fn == name)?(true):(false);
  }
  return (strstr(name,abcd) == 0)?(false):(true);
}
//-------------------------------------------------------------------------
//  Filter Airport according to request
//-------------------------------------------------------------------------
bool CDbCacheMgr::FilterAirport(char *rec,CDataBaseREQ *req)
{ if (req->NeedCTY() && (strcmp(req->ctykey,  rec+63) != 0))      return false;
  if (req->NeedSTA() && (strcmp(req->stakey+2,rec+66) != 0))      return false;
  if (req->NeedST1() && (!FilterByString(req->st1key,rec+23)))    return false;
  if (req->NeedST2() && (!FilterByString(req->st2key,rec+18)))    return false;
  return true;
}
//-------------------------------------------------------------------------
//  Filter navaid according to request
//-------------------------------------------------------------------------
bool CDbCacheMgr::FilterNavaid(char *rec,CDataBaseREQ *req)
{ U_LONG  n = *(U_LONG*)(rec+89);               // Decode type
  n = LittleEndian (n);
  if (req->NeedCTY() && (strcmp(req->ctykey,  rec+83)   != 0))    return false;
  if (req->NeedSTA() && (strcmp(req->stakey+2,rec+86)   != 0))    return false;
  if (req->NeedNDB() && ((n & NAVAID_TYPE_NDB)          == 0))    return false;
  if (req->NeedVOR() && ((n & NAVAID_TYPE_NDB)          != 0))    return false;
  if (req->NeedST1() && (!FilterByString(req->st1key,rec+19)))    return false;
  if (req->NeedST2() && (!FilterByString(req->st2key,rec+14)))    return false;
  return true;
}
//-------------------------------------------------------------------------
//  Filter Airport according to request
//-------------------------------------------------------------------------
bool CDbCacheMgr::FilterWaypoint(char *rec,CDataBaseREQ *req)
{ char *name = rec+14;
  if (*name == '(') name++;
  if (req->NeedCTY() && (strcmp(req->ctykey,  rec+40) != 0))      return false;
  if (req->NeedSTA() && (strcmp(req->stakey+2,rec+43) != 0))      return false;
  if (req->NeedST1() && (!FilterByString(req->st1key,name)))      return false;
  return true;
}
//-------------------------------------------------------------------------
//  Get country by filtering
//-------------------------------------------------------------------------
void CDbCacheMgr::GetAllCountries(CDataBaseREQ *req)
{ CDatabaseCTY *db  = (CDatabaseCTY*)CDatabaseManager::Instance().GetCTYDatabase();
  if (0 == db)          return;
  U_INT     No      = 0;
  U_INT     end     = (U_INT)GetDBcountryNBrecords();
  U_LONG  offset    = 0;
  CSlot *slot       = 0;
  while (No != end)
  { offset  =     db->RecordOffset(No++);
    if (0 == slot )     slot  = new CCtyLine;
    db->GetRawRecord(offset);
    db->DecodeRecord(slot);
    if (!FilterCountry(slot,req)) continue;
    req->Wind->AddDBrecord(slot,(DBCODE)req->Code);
    slot  = 0;
    if (req->NeedCTY()) break;
  }
  if (slot) delete slot;
  return ;
}
//-------------------------------------------------------------------------
//  Get all countries names for TCache manager
//-------------------------------------------------------------------------
void CDbCacheMgr::GetCountryName(TCacheMGR *tcm)
{ CDatabaseCTY *db  = (CDatabaseCTY*)CDatabaseManager::Instance().GetCTYDatabase();
  if (0 == db)      return;
  U_INT     No      = 0;
  U_INT     end     = (U_INT)GetDBcountryNBrecords();
  U_LONG  offset    = 0;
  CCtyLine *slot    = new CCtyLine;
  while (No != end)
  { offset  =     db->RecordOffset(No++);
    db->GetRawRecord(offset);
    db->DecodeRecord(slot);
    tcm->OneCountry(slot);
  }
  delete slot;
  return ;
}
//-------------------------------------------------------------------------
//  Get State for a given country
//-------------------------------------------------------------------------
void CDbCacheMgr::GetStateByCountry(CDataBaseREQ *req)
{ U_INT No          = 0;
  CDatabaseSTA *db  = (CDatabaseSTA*)CDatabaseManager::Instance().GetSTADatabase();
  if  (0 == db)     return;
  U_INT end         = db->GetNumRecords();
  CStaLine *slot    = 0;
  U_LONG  offset  = 0;
  while (No != end)
  { offset    = db->RecordOffset(No++);
    if (0 == offset)              break;
    if (0 == slot) slot = new CStaLine;
    db->GetRawRecord (offset);
    db->DecodeRecord(slot);
    if (strcmp(req->ctykey,slot->GetCTY()) != 0)       continue;
    req->Wind->AddDBrecord(slot,(DBCODE)req->Code);
    slot  = 0;
  }
  if (slot) delete slot;
  return;
}
//-------------------------------------------------------------------------
//  Get all airport for a given country
//-------------------------------------------------------------------------
void CDbCacheMgr::GetAirportByArg(CDataBaseREQ *req)
{ U_INT No      = 0;
  CDatabaseAPT *db = (CDatabaseAPT*)CDatabaseManager::Instance().GetAPTDatabase();
  if  (0 == db)         return;
  U_INT end       = db->GetNumRecords();
  U_LONG  offset  = 0;
  CAptLine  *slot = 0;
  char      *rec  = db->GetBuffer();
  while (No != end)
  { offset    = db->RecordOffset(No++);
    if (0 == offset)              break;
    if (0 == slot)  slot  = new CAptLine;
    db->GetRawRecord (offset);
    if (!FilterAirport(rec,req)) continue;
    db->DecodeRecord(slot);
    slot->SetOFS(offset);
    req->Wind->AddDBrecord(slot,(DBCODE)req->Code);
    slot  = 0;
  }
  if (slot) delete slot;
  return;
}
//-------------------------------------------------------------------------
//  Get all Navaid for a given country
//-------------------------------------------------------------------------
void CDbCacheMgr::GetNavaidByArg(CDataBaseREQ *req)
{ U_INT No      = 0;
  CDatabaseNAV *db  = (CDatabaseNAV*)CDatabaseManager::Instance().GetNAVDatabase();
  if  (0 == db)    return;
  U_INT end         = db->GetNumRecords();
  CNavLine *slot    = 0;
  U_LONG  offset    = 0;
  char     *rec     = db->GetBuffer();
  while (No != end)
  { offset    = db->RecordOffset(No++);
    if  (0 == offset)             break;
    if  (0 == slot) slot = new CNavLine;
    db->GetRawRecord (offset);
    if (!FilterNavaid(rec,req))   continue;
    db->DecodeRecord(slot);
    slot->SetOFS(offset);
    req->Wind->AddDBrecord(slot,(DBCODE)req->Code);
    slot  = 0;
  }
  if (slot) delete slot;
  return;
}
//-------------------------------------------------------------------------
//  Get all Waypoints for a given country
//-------------------------------------------------------------------------
void CDbCacheMgr::GetWaypointByArg(CDataBaseREQ *req)
{ U_INT No         = 0;
  CDatabaseWPT *db = (CDatabaseWPT*)CDatabaseManager::Instance().GetWPTDatabase();
  if  (0 == db)    return;
  U_INT end        = db->GetNumRecords();
  CWptLine *slot   = 0;
  U_LONG  offset   = 0;
  char      *rec  = db->GetBuffer();
  while (No != end)
  { offset    = db->RecordOffset(No++);
    if  (0 == offset)             break;
    if  (0 == slot) slot = new CWptLine;
    db->GetRawRecord (offset);
    if (!FilterWaypoint(rec,req))   continue;
    db->DecodeRecord(slot);
    slot->SetOFS(offset);
    req->Wind->AddDBrecord(slot,(DBCODE)req->Code);
    slot  = 0;
  }
  if (slot) delete slot;
  return;
}
//-------------------------------------------------------------------------
//  Get VOR by record Offset
//-------------------------------------------------------------------------
void CDbCacheMgr::NAVByOffset(CDataBaseREQ *req)
{ CNavaid *nav;
  CDatabase *db = CDatabaseManager::Instance().GetNAVDatabase();
  if (0 == db)          return;
  U_LONG  offset  =     req->GetOffset();
  nav   = new CNavaid(ANY,VOR);
  db->GetRawRecord(offset);
  db->DecodeRecord(offset,nav);
  req->Wind->AddDBrecord(nav,(DBCODE)req->Code);
  return ;
}
//------------------------------------------------------------------------
//    Find runway fom airport
//------------------------------------------------------------------------
void CDbCacheMgr::RwyFromThisAirport(CDataBaseREQ *req)
{ SPosition org;
  CRwyLine *slot;
  int       No = 0;
  org.alt      = 0;
  // Get handle to runway database ---------------------------
  CDatabaseRWY *db = (CDatabaseRWY*)CDatabaseManager::Instance().GetRWYDatabase ();
  if (0 == db)          return;
  unsigned long offset = db->Search ('tpar',req->aptkey);
  if (offset == 0)      return;
  while (offset  != 0) {
      slot  = new CRwyLine;
      db->GetRawRecord (offset);
			db->DecodeRecord(slot);
      slot->SetOFS(offset);
      if (No++ == 0)  org = slot->GetHiPosition();
      ComputeDistance(slot,org);
      slot->EditRWY();
      req->Wind->AddDBrecord(slot,(DBCODE)req->Code);
      IlsFromThisRunway(req,slot);
			offset = db->SearchNext();
    }
  return; 
}
//=================================================================================
//  Immediate request:  Use only during initialization
//  During time slice, Use a request to database
//=================================================================================
void CDbCacheMgr::FirstAirportByIdent(char *iden,CAirport **ptr)
{ SqlMGR *sqm = globals->sqm;
  *ptr = 0;
  if  (gSQL)  sqm->GetAirportByIden(iden,ptr);
  else        AirportByIdentFromPOD(iden,ptr);
  return;
}
//-------------------------------------------------------------------------
//  Get first airport for a given ident
//-------------------------------------------------------------------------
void CDbCacheMgr::AirportByIdentFromPOD(char *iden,CAirport **ptr)
{ CDatabaseManager &dbmgr = CDatabaseManager::Instance();
  CDatabase *db           = dbmgr.GetAPTDatabase();
  unsigned long offset    = db->Search ('acia', iden);            // Search
  if  (0 == offset)       return;
  CAirport *apt           = new CAirport(SHR,APT);
  apt->IncUser();
  while (offset)
  { db->GetRawRecord(offset);
    db->DecodeRecord(offset,apt);
    if (strcmp(iden,apt->GetIdent()) != 0) continue;
    *ptr = apt; 
    break;
  }
  if (*ptr == 0) delete apt;
  return;
}

//=========================================================================
//  Request management
//=========================================================================
//-------------------------------------------------------------------------
//  CDatabaseREQ  Request to database
//-------------------------------------------------------------------------
CDataBaseREQ::CDataBaseREQ(CFuiWindow *win,DBCODE req)
{ Code    = req;
  Wind    = win;
}
//-------------------------------------------------------------------------
//  CDatabaseREQ  Request to database
//-------------------------------------------------------------------------
CDataBaseREQ::CDataBaseREQ()
{ Code    = NO_REQUEST;
  Wind    = 0;
}
//-------------------------------------------------------------------------
//  Set country key
//-------------------------------------------------------------------------
void  CDataBaseREQ::SetCTY(char *cty)
{ ctykey  = cty;
  if (cty[0] != 0)  filter |= 0x01;
  return;
}
//-------------------------------------------------------------------------
//  Set State key
//-------------------------------------------------------------------------
void  CDataBaseREQ::SetSTA(char *sta)
{ stakey  = sta;
  if (sta[0] != 0)  filter |= 0x02;
  return;
}
//-------------------------------------------------------------------------
//  Set criteria C1
//-------------------------------------------------------------------------
void  CDataBaseREQ::SetST1(char *st1)
{ st1key = st1;
  if (st1[0] != 0)  filter |= 0x04;
  return;
}
//-------------------------------------------------------------------------
//  Set criteria C
//-------------------------------------------------------------------------
void  CDataBaseREQ::SetST2(char *st2)
{ st2key = st2;
  if (st2[0] != 0)  filter |= 0x08;
  return;
}
//=========================================================================
//  GPS Request management
//=========================================================================
//-------------------------------------------------------------------------
//  Clean GPS request
//-------------------------------------------------------------------------
void CGPSrequest::Clean()
{ obj  = 0;
  rwyQ.Clear();
  comQ.Clear();
  ilsQ.Clear();
  //---DONT DELETE OBJECTS FROM THIS QUEUE --------------
  CmHead *wpt = wptQ.PopFromQ1();
  while (wpt) { wpt->DecUser(); wpt = wptQ.PopFromQ1();  }
  return;
}
//-------------------------------------------------------------------------
//  If no airport is present, get nearest airport
//  If airport, reactivate the EOR event
//-------------------------------------------------------------------------
void CGPSrequest::GetAPT()
{ CmHead *any = obj.Pointer();
  Code  = REQUEST_END;
  if (any && (APT == any->GetActiveQ())) return;
  Clean();
  Code  = GPS_NR_AIRPORT;
  return;
}
//-------------------------------------------------------------------------
//  If no VOR is present, get nearest VOR
//  If VOR, reactivate the EOR event
//-------------------------------------------------------------------------
void  CGPSrequest::GetVOR()
{ CmHead *any = obj.Pointer();
  Code  = REQUEST_END;
  if (any && (VOR == any->GetActiveQ())) return;
  Clean();
  Code  = GPS_NR_VOR;
  return;
}
//-------------------------------------------------------------------------
//  If no VOR is present, get nearest VOR
//  If VOR, reactivate the EOR event
//-------------------------------------------------------------------------
void  CGPSrequest::GetNDB()
{ CmHead *any = obj.Pointer();
  Code  = REQUEST_END;
  if (any && (NDB == any->GetActiveQ())) return;
  Clean();
  Code  = GPS_NR_NDB;
  return;
}
//-------------------------------------------------------------------------
//  If no WPT is present, get nearest WPT
//  If WPT, reactivate the EOR event
//-------------------------------------------------------------------------
void  CGPSrequest::GetWPT()
{ CmHead *any = obj.Pointer();
  Code  = REQUEST_END;
  if (any && (WPT == any->GetActiveQ())) return;
  Clean();
  Code  = GPS_NR_WPT;
  return;
}
//-------------------------------------------------------------------------
//  If no Runway are present, get airport runway
//  If Runway, reactivate the EOR event
//-------------------------------------------------------------------------
void CGPSrequest::GetRWY()
{ Code = REQUEST_END;
  if (rwyQ.NotEmpty())  return;
  Code  = GPS_GT_RUNWAY;
  return;
}
//-------------------------------------------------------------------------
//  If no COMM are present, get airport com
//  If com, reactivate the EOR event
//-------------------------------------------------------------------------
void CGPSrequest::GetCOM()
{ Code = REQUEST_END;
  if (comQ.NotEmpty())  return;
  Code  = GPS_GT_COMM;
  return;
}
//-------------------------------------------------------------------------
//  If no ILS are present, get airport ILS
//  If ILS, reactivate the EOR event
//-------------------------------------------------------------------------
void CGPSrequest::GetILS()
{ Code = REQUEST_END;
  if (ilsQ.NotEmpty())  return;
  Code  = GPS_GT_ILS;
  return;
}
//-------------------------------------------------------------------------
//  Destroy GPS request
//-------------------------------------------------------------------------
CGPSrequest::~CGPSrequest()
{ Clean();
}
//-------------------------------------------------------------------------
//  Post an access request to the database management
//-------------------------------------------------------------------------
void  CDbCacheMgr::PostRequest(CDataBaseREQ *req)
{ reqQ.LastInQ1(req);
  return;
}
//-------------------------------------------------------------------------
//  Execute the first database request
//-------------------------------------------------------------------------
void CDbCacheMgr::ExecuteREQ()
{ SqlMGR *sqm = globals->sqm;
  CDataBaseREQ *req = (CDataBaseREQ*)reqQ.GetFirst();
  switch (req->Code)  {
    case COM_BY_AIRPORT:
      if  (gSQL)  sqm->ComFromThisAirport(req);
      else             ComFromThisAirport(req);
      break;
    case RWY_BY_AIRPORT:
      if  (gSQL)  sqm->RWYbyAPTkey(req);
      else             RwyFromThisAirport(req);
      break;
    case CTY_BY_ALLLOT:
      if  (gSQL)  sqm->GetAllCountries(req);
      else             GetAllCountries(req);
      break;
    case CTY_BY_CTYKEY:
      if  (gSQL)  sqm->GetOneCountry(req);
      else             GetAllCountries(req);
      break;
    case APT_BY_FILTER:
      if  (gSQL)  sqm->GetAirportByArg(req);
      else             GetAirportByArg(req);
      break;
    case NAV_BY_FILTER:
      if  (gSQL)  sqm->GetNavaidByArg(req);
      else             GetNavaidByArg(req);
      break;
    case WPT_BY_FILTER:
      if  (gSQL)  sqm->GetWaypointByArg(req);
      else             GetWaypointByArg(req);
      break;
    case STA_BY_COUNTRY:
      if  (gSQL)  sqm->GetStateByCountry(req);
      else             GetStateByCountry(req);
      break;
    case APT_BY_OFFSET:
      if  (gSQL)  sqm->APTByKey(req);
      else             APTByOffset(req);
      break;
    case NAV_BY_OFFSET:
      if  (gSQL)  sqm->NAVByKey(req);
      else             NAVByOffset(req);
      break;
    case NDB_BY_OFFSET:
      if  (gSQL)  sqm->NAVByKey(req);
      else             NDBByOffset(req);
      break;
  }
//-----Delete the request and post result -----------------------------
  reqQ.PopFromQ1();
  req->Prev = req->Code;
  req->Code = REQUEST_END;
  req->ClearFilter();
  return;
}
//-------------------------------------------------------------------------
//  Execute GPS database request
//  NOTE:  The nearest functions has already incremented the object user count
//-------------------------------------------------------------------------
void CDbCacheMgr::TrackGPS()
{ SqlMGR  *sqm = globals->sqm;
  switch (rGPS.Code)  {
    //----Clear sorted stack if not currently in use ----------
    case NO_REQUEST:
      { if (rGPS.nSTACK)  return;
        int nbr = objSTK.nWPT;
        while (nbr--) objSTK.List[nbr] = 0;
        objSTK.nWPT = 0;
        return;
      }
    //----Get Nearest airport ---------------------------------
    case GPS_NR_AIRPORT:
      { OrderOBJ(APT,&objSTK,"APT",8);
        rGPS.wptNo   = 1;
        rGPS.obj     = objSTK.List[0];
        rGPS.Code    = REQUEST_END;
        rGPS.offset  = (rGPS.obj.Assigned())? (rGPS.obj->GetRecNo()):(0);
        return;
      }
    //----Get nearest VOR ------------------------------------
    case GPS_NR_VOR:
      { OrderOBJ(VOR,&objSTK,"VOR",8);
        rGPS.wptNo   = 1;
        rGPS.obj     = objSTK.List[0];
        rGPS.Code    = REQUEST_END;
        rGPS.offset  = (rGPS.obj.Assigned())? (rGPS.obj->GetRecNo()):(0);
        return;
      }
    //----Get nearest NDB ------------------------------------
    case GPS_NR_NDB:
      { OrderOBJ(NDB,&objSTK,"NDB",8);
        rGPS.wptNo   = 1;
        rGPS.obj     = objSTK.List[0];
        rGPS.Code    = REQUEST_END;
        rGPS.offset  = (rGPS.obj.Assigned())? (rGPS.obj->GetRecNo()):(0);
        return;
      }
    //----Get nearest Waypoint -----------------------------------
    case GPS_NR_WPT:
      { OrderOBJ(WPT,&objSTK,"WPT",8);
        rGPS.wptNo   = 1;
        rGPS.obj     = objSTK.List[0];
        rGPS.Code    = REQUEST_END;
        rGPS.offset  = (rGPS.obj.Assigned())? (rGPS.obj->GetRecNo()):(0);
        return;
      }
    //------Get runway for current airport -----------------------
    case GPS_GT_RUNWAY:
      { CAirport *apt = (CAirport*)rGPS.obj.Pointer();
        if  (gSQL)  sqm->AirportRWYforGPS(&rGPS,apt);  
        else             AirportRWYforGPS(&rGPS,apt);
        rGPS.Code    = REQUEST_END;
        return;
      }
    //------Get Com for current airport --------------------------
    case GPS_GT_COMM:
      { CAirport *apt = (CAirport*)rGPS.obj.Pointer();
        if  (gSQL)  sqm->AirportCOMforGPS(&rGPS,apt);
        else             AirportCOMforGPS(&rGPS,apt);
        rGPS.Code    = REQUEST_END;
        return;
      }
    //-----Get ILS for current airport --------------------------
    case GPS_GT_ILS:
      { CAirport *apt = (CAirport*)rGPS.obj.Pointer();
        CRunway  *rwy = (CRunway*)rGPS.rwyQ.GetFirst();
        while (rwy)
        { if (gSQL)   sqm->RunwayILSforGPS(&rGPS,apt->akey,rwy->rhid);
          else             RunwayILSforGPS(&rGPS,apt->akey,rwy->rhid);
          if (gSQL)   sqm->RunwayILSforGPS(&rGPS,apt->akey,rwy->rlid);
          else             RunwayILSforGPS(&rGPS,apt->akey,rwy->rlid);
          rwy = (CRunway*)rwy->NextInQ1();
        }
        rGPS.Code    = REQUEST_END;
        return;
      }
    //------Airport search mode ----------------------------------
    case GPS_GT_AIRPORT:
      { int dir   = rGPS.Direct;
        rGPS.Clean();
        if  (gSQL)  sqm->MatchAPTforGPS(&rGPS,dir);
        else        rGPS.obj  = MatchAPTforGPS(&rGPS,dir);
        rGPS.Code = REQUEST_END;
        return;
      }
     //------VOR search mode ----------------------------------
    case GPS_GT_VOR:
      { int dir   = rGPS.Direct;
        rGPS.Clean();
        rGPS.actQ = VOR;
        rGPS.SelectVOR();
        if  (gSQL)  sqm->MatchNAVforGPS(&rGPS,dir);
        else        rGPS.obj  = MatchNAVforGPS(&rGPS,dir);
        rGPS.Code = REQUEST_END;
        return;
      }
    //------NDB search mode ----------------------------------
    case GPS_GT_NDB:
      { int dir   = rGPS.Direct;
        rGPS.Clean();
        rGPS.actQ = NDB;
        rGPS.SelectNDB();
        if  (gSQL)  sqm->MatchNAVforGPS(&rGPS,dir);
        else        rGPS.obj  = MatchNAVforGPS(&rGPS,dir);
        rGPS.Code = REQUEST_END;
        return;
      }
    //------WPT search mode ----------------------------------
    case GPS_GT_WPT:
      { int dir   = rGPS.Direct;
        rGPS.Clean();
        if  (gSQL)  sqm->MatchWPTforGPS(&rGPS,dir);
        else        rGPS.obj  = MatchWPTforGPS(&rGPS,dir);
        rGPS.Code = REQUEST_END;
        return;
      }
    //----Get any waypoint by IDENT --------------------------
    case GPS_ANY_WPT:
      { rGPS.wptNo = 1;
        if  (gSQL)  sqm->GetOBJbyIdent(&rGPS,"APT",'APT*');
        else             GetAPTbyIdent();
        if  (gSQL)  sqm->GetOBJbyIdent(&rGPS,"NAV",'NAV*');
        else            {GetVORbyIdent();GetNDBbyIdent();}
        if  (gSQL)  sqm->GetOBJbyIdent(&rGPS,"WPT",'WPT*');
        else             GetWPTbyIdent();
        rGPS.Code  = REQUEST_END;
        return;
      }
  }
  return;
}
//-------------------------------------------------------------------------
//  Set the nearest VOR
//-------------------------------------------------------------------------
void CDbCacheMgr::SetNearestVOR()
{ ClQueue    *hd  = aHead[VOR];
  rGPS.wptNo   = 1;
  rGPS.obj     = FindNearestOBJ(hd->GetFirst());
  rGPS.Code    = REQUEST_END;
  rGPS.offset  = (rGPS.obj.Assigned())? (rGPS.obj->GetRecNo()):(0);
  return;
}
//-------------------------------------------------------------------------
//  Set the next nearest Object
//-------------------------------------------------------------------------
void CDbCacheMgr::SetNextNRSobj()
{ short No  = rGPS.wptNo;
  short Max = objSTK.nWPT;
  if (rGPS.wptNo >= Max)   return;
  rGPS.Clean();
  rGPS.obj = objSTK.List[No];
  rGPS.wptNo++;
  return;
}
//-------------------------------------------------------------------------
//  Set the previous nearest Object
//-------------------------------------------------------------------------
void CDbCacheMgr::SetPrevNRSobj()
{ short   No = rGPS.wptNo;
  if (No == 1)            return;
  rGPS.Clean();
  rGPS.obj = objSTK.List[No-2];
  rGPS.wptNo--;
  return;
}
//-------------------------------------------------------------------------
//  Get next  Airport that match filter (Dir specificies direction)
//-------------------------------------------------------------------------
CAirport *CDbCacheMgr::MatchAPTforGPS(CGPSrequest *req,int dir)
{ long No       = req->offset;
  CDatabase *db = CDatabaseManager::Instance().GetAPTDatabase();
  if  (0 == db)         return 0;
  long end      = db->GetNumRecords();
  char *rec     = db->GetBuffer();
  U_LONG  offset  = 0;
  CAirport *apt   = 0;
  
  do
  { No +=dir;
    if (No ==  -1) No = end - 1;
    if (No == end) No = 0;
    offset    = db->RecordOffset(No);
    if (0 == offset)             break;
    db->GetRawRecord (offset);
    if (!FilterAirport(rec,req)) continue;
    apt  = new CAirport(SHR,APT);
    db->DecodeRecord(offset,apt);
    req->offset = apt->GetRecNo();
    break;
  }
  while (No != (long)req->offset);
  return apt;
}
//-------------------------------------------------------------------------
//  Get VOR-NDB that match filter
//-------------------------------------------------------------------------
CNavaid *CDbCacheMgr::MatchNAVforGPS(CGPSrequest *req,int dir)
{ long No       = req->offset;
  CDatabase *db = CDatabaseManager::Instance().GetNAVDatabase();
  if  (0 == db)         return 0;
  long  end     = db->GetNumRecords();
  char *rec     = db->GetBuffer();
  U_LONG  offset  = 0;
  CNavaid  *nav   = 0;
  do
  { No += dir;
    if (No ==  -1)  No = end - 1;
    if (No == end)  No = 0;
    offset    = db->RecordOffset(No);
    if (0 == offset)             break;
    db->GetRawRecord (offset);
    if (!FilterNavaid(rec,req))  continue;
    nav  = new CNavaid(ANY,req->actQ);
    db->DecodeRecord(offset,nav);
    req->offset = nav->GetRecNo();
    break;
  }
  while (No != req->offset);
  return nav;
}

//-------------------------------------------------------------------------
//  Get WPT that match filter
//-------------------------------------------------------------------------
CWPT *CDbCacheMgr::MatchWPTforGPS(CGPSrequest *req,int dir)
{ long No       = req->offset;
  CDatabase *db = CDatabaseManager::Instance().GetWPTDatabase();
  if  (0 == db)         return 0;
  long  end     = db->GetNumRecords();
  char *rec     = db->GetBuffer();
  U_LONG  offset   = 0;
  CWPT *wpt     = 0;
  do
  { No += dir;
    if (No ==  -1)  No = end - 1;
    if (No == end)  No = 0;
    offset    = db->RecordOffset(No);
    if (0 == offset)              break;
    db->GetRawRecord (offset);
    if (!FilterWaypoint(rec,req)) continue;
    wpt  = new CWPT(SHR,WPT);
    db->DecodeRecord(offset,wpt);
    req->offset = wpt->GetRecNo();
    break;
  }
  while (No != req->offset);
  return wpt;
}
//-------------------------------------------------------------------------
//  Get Airport by Ident using index
//  NOTE: user count must be incremented to avoid deletion when req is cleaned
//-------------------------------------------------------------------------
void CDbCacheMgr::GetAPTbyIdent()
{ char *iden = rGPS.GetST2key();
  CDatabaseManager &dbmgr = CDatabaseManager::Instance();
  CDatabase *db           = dbmgr.GetAPTDatabase();
  unsigned long offset    = db->Search ('acia', iden);            // Search
  if  (0 == offset)       return;
  CAirport *apt           = 0;
  while (offset)
  { if (0 == apt) apt = new CAirport(SHR,APT);
    db->GetRawRecord(offset);
    db->DecodeRecord(offset,apt);
    if (strcmp(iden,apt->GetIdent()) != 0)  break;
    apt->SetDistLat(rGPS.wptNo++);
    rGPS.wptQ.LastInQ1(apt);
    apt->IncUser();
    apt = 0;
    offset= db->SearchNext();
    if (30 == rGPS.wptQ.GetNbrObj())  break;
  }
  if (apt)  delete apt;
  return;
}
//-------------------------------------------------------------------------
//  Get VOR by Ident using index
//  NOTE: user count must be incremented to avoid deletion when req is cleaned
//-------------------------------------------------------------------------
void CDbCacheMgr::GetVORbyIdent()
{ char *iden = rGPS.GetST2key();
  CDatabaseManager &dbmgr = CDatabaseManager::Instance();
  CDatabase *db           = dbmgr.GetNAVDatabase();
  unsigned long offset    = db->Search ('dian', iden);            
  if  (0 == offset)       return;
  CNavaid *nav            = 0;
  while (offset)
  { if (0 == nav) nav = new CNavaid(ANY,VOR);
    db->GetRawRecord(offset);
    db->DecodeRecord(offset,nav);
    if (strcmp(nav->naid,iden) != 0)  break;
    if ((NAVAID_TYPE_NDB & nav->type) == 0) 
    { rGPS.wptQ.LastInQ1(nav);
      nav->SetDistLat(rGPS.wptNo++);
      nav->IncUser();
      nav = 0; }
    offset= db->SearchNext();
    if (30 == rGPS.wptQ.GetNbrObj())  break;
  }
  if (nav )delete nav;
  return;
}
//-------------------------------------------------------------------------
//  Get NDB by Ident using index
//  NOTE: user count must be incremented to avoid deletion when req is cleaned
//-------------------------------------------------------------------------
void CDbCacheMgr::GetNDBbyIdent()
{ char *iden    = rGPS.GetST2key();
  CDatabaseManager &dbmgr = CDatabaseManager::Instance();
  CDatabase *db           = dbmgr.GetNAVDatabase();
  unsigned long offset    = db->Search ('dian', iden);            
  if  (0 == offset)       return;
  CNavaid *nav            = 0;
  while (offset)
  { if (0 == nav) nav = new CNavaid(ANY,NDB);
    db->GetRawRecord(offset);
    db->DecodeRecord(offset,nav);
    if (strcmp(nav->naid,iden) != 0)  break;
    if (NAVAID_TYPE_NDB & nav->type) 
    { rGPS.wptQ.LastInQ1(nav);
      nav->SetDistLat(rGPS.wptNo++);
      nav->IncUser();
      nav = 0; }
    offset= db->SearchNext();
    if (30 == rGPS.wptQ.GetNbrObj())  break;
  }
  if (nav )delete nav;
  return;
}
//-------------------------------------------------------------------------
//  Get WPT by Ident using index
//  NOTE: user count must be incremented to avoid deletion when req is cleaned
//-------------------------------------------------------------------------
void CDbCacheMgr::GetWPTbyIdent()
{ CDatabaseManager &dbmgr = CDatabaseManager::Instance();
  CDatabase *db           = dbmgr.GetWPTDatabase();
  char *iden = rGPS.GetST2key();
  U_INT end     = db->GetNumRecords();
  U_INT No      = 0;
  U_INT Nb      = 0;
  U_LONG offset = 0;
  CWPT *wpt     = 0;
  char *rec     = db->GetBuffer();
  while (No != end)
  { offset  =     db->RecordOffset(No++);
    db->GetRawRecord(offset);
    char *fn = strstr(rec+14,iden);
    if (0 == fn)          continue;
    if (fn != rec+14)     continue;
    if (0 == wpt)     wpt = new CWPT(SHR,WPT);
    db->DecodeRecord(offset,wpt);
    rGPS.wptQ.LastInQ1(wpt);
    wpt->SetDistLat(rGPS.wptNo++);
    wpt->IncUser();
    wpt = 0;
    if (30 == rGPS.wptQ.GetNbrObj())  break;
  }
  return;
}
//========================================================================
//  return CCOM INDEX
//========================================================================
RD_COM GetComINDEX(U_INT mask)
{ if (mask & COMM_ATIS)                 return COM_AT;
  if (mask & COMM_TOWER)                return COM_TW;
  if (mask & COMM_CLEARANCEDELIVERY)    return COM_CL;
  if (mask & COMM_GROUNDCONTROL)        return COM_GR;
  if (mask & COMM_APPROACHCONTROL)      return COM_AP;
  if (mask & COMM_DEPARTURECONTROL)     return COM_DP;
  if (mask & COMM_FLIGHTSERVICESTATION) return COM_SV;
  if (mask & COMM_CENTER)               return COM_CR;
  if (mask & COMM_CTAF)                 return COM_CF;
  if (mask & COMM_MULTICOM)             return COM_MU;
  if (mask & COMM_EMERGENCY)            return COM_EM;
  return COM_ZR;
}
//=========================================================================
//
// CDatabaseManager
//=========================================================================
CDatabaseManager CDatabaseManager::instance;

CDatabaseManager::CDatabaseManager (void) {}

void CDatabaseManager::Init (void)
{ //---------------------------------------------
  MEMORY_LEAK_MARKER ("database_init")
  if (0 == globals->sqm->SQLgen())
  { TRACE("------------DB Loading AIRPORTS");
    CDatabase *dbAirport = new CDatabaseAPT ("Airport.dbt");
    dbAirport->Mount ("Airport.dbd");
    dbAirport->AddIndex ("Airport.dbi");
    dbAirport->AddIndex ("AprtCtry.dbi");
    dbAirport->AddIndex ("AprtStat.dbi");
    dbAirport->AddIndex ("AprtFaa.dbi");
    dbAirport->AddIndex ("AprtIcao.dbi");
    dbAirport->AddIndex ("AprtGlob.dbi");
    dbAirport->AddIndex ("AprtName.dbi");
    db[DB_AIRPORT] = dbAirport;
    TRACE("APT     count %u",dbAirport->GetNumRecords());
  }
  // Load ATS (Air Traffic System) Routing database
  TRACE("------------DB Loading ATS");
  CDatabase *dbAtsRoute = new CDatabase ("AtsRoute.dbt");
  dbAtsRoute->Mount ("AtsRoute.dbd");
  db[DB_ATS_ROUTE] = dbAtsRoute;
  TRACE("ATS     count %u",dbAtsRoute->GetNumRecords());

  if (0 == globals->sqm->SQLgen())
  { // Load Comm facility database
    TRACE("------------DB Loading COMM FACILITY");
    CDatabase *dbComm = new CDatabaseCOM ("Comm.dbt");
    dbComm->Mount ("Comm.dbd");
    dbComm->AddIndex ("Comm.dbi");
    dbComm->AddIndex ("CommGlob.dbi");
    db[DB_COMM] = dbComm;
    TRACE("COM     count %u",dbComm->GetNumRecords());
  }
  if (0 == globals->sqm->SQLgen())
  { // Load Country name database
    TRACE("------------DB Loading COUNTRY");
    CDatabase *dbCountry = new CDatabaseCTY ("Country.dbt");
    dbCountry->Mount ("Country.dbd");
    dbCountry->AddIndex ("Country.dbi");
    dbCountry->AddIndex ("CtryName.dbi");
    db[DB_COUNTRY] = dbCountry;
    TRACE("COUNTRY count %u",dbCountry->GetNumRecords());
  }
  if (0 == globals->sqm->SQLgen())
  { // Load State name database
    TRACE("------------DB Loading STATES");
    CDatabase *dbState = new CDatabaseSTA ("State.dbt");
    dbState->Mount ("State.dbd");
    dbState->AddIndex ("Statctry.dbi");
    dbState->AddIndex ("State.dbi");
    dbState->AddIndex ("StatName.dbi");
    db[DB_STATE] = dbState;
    TRACE("STATE   count %u",dbState->GetNumRecords());
  }
  if (0 == globals->sqm->SQLgen())
  { // Load ILS Facility database
    TRACE("------------DB Loading ILS");
    CDatabase *dbIls = new CDatabaseILS ("Ils.dbt");
    dbIls->Mount ("Ils.dbd");
    dbIls->AddIndex ("Ils.dbi");
    dbIls->AddIndex ("IlsGlob.dbi");
    //  dbIls->AddIndex ("IlsLocal.dbi");                           // JSDEV* have dupplicate ILS from global index
    db[DB_ILS] = dbIls;
    TRACE("ILS     count %u",dbIls->GetNumRecords());
  }
  if (0 == globals->sqm->SQLgen())
  { // Load Navaid database
    TRACE("------------DB Loading NAVAIDs");
    CDatabase *dbNavaid = new CDatabaseNAV ("Navaid.dbt");
    dbNavaid->Mount ("Navaid.dbd");
    dbNavaid->AddIndex ("Navaid.dbi");
    dbNavaid->AddIndex ("NavNdbId.dbi");
    dbNavaid->AddIndex ("NavNdbNm.dbi");
    dbNavaid->AddIndex ("NavVorId.dbi");
    dbNavaid->AddIndex ("NavVorNm.dbi");
    db[DB_NAVAID] = dbNavaid;
    TRACE("NAV     count %u",dbNavaid->GetNumRecords());
  }
  // Load Obstruction database
  TRACE("------------DB Loading OBSTRUCTIONS");
  CDatabase *dbObstruct = new CDatabaseOBS ("Obstruct.dbt");
  dbObstruct->Mount ("Obstruct.dbd");
  dbObstruct->AddIndex ("ObstGlob.dbi");
  db[DB_OBSTRUCTION] = dbObstruct;
  TRACE("OBS     count %u",dbObstruct->GetNumRecords());
  if (0 == globals->sqm->SQLgen())
  { // Load Runway database
    TRACE("------------DB Loading RUNWAYS");
    CDatabase *dbRunway = new CDatabaseRWY("RUNWAY.dbt");
    dbRunway->Mount ("RUNWAY.dbd");
    dbRunway->AddIndex ("RUNWAY.dbi");
    db[DB_RUNWAY] = dbRunway;
    TRACE("RWY     count %u",dbRunway->GetNumRecords());
  }
  if (0 == globals->sqm->SQLgen())
  { // Load Waypoint database
    TRACE("------------DB Loading WAYPOINTS");
    CDatabase *dbWaypoint = new CDatabaseWPT("Waypoint.dbt");
    dbWaypoint->Mount ("WAYPOINT.dbd");
    dbWaypoint->AddIndex ("WAYPOINT.dbi");
    dbWaypoint->AddIndex ("WAYKEY.dbi");
    db[DB_WAYPOINT] = dbWaypoint;
    TRACE("WPT     count %u",dbWaypoint->GetNumRecords());
  }
  MEMORY_LEAK_MARKER ("database_init")
  // load string translation for various database "type" field
  InitTypeDBTranslation();
}

void CDatabaseManager::Cleanup (void)
{
  std::map<Tag, CDatabase*>::iterator i;
  for (i=db.begin(); i!=db.end(); i++) {
    SAFE_DELETE (i->second);
  }
  db.clear ();
  CleanupTypeDBTranslation();
}

CDatabase* CDatabaseManager::GetDatabase (Tag id)
{
  CDatabase* rc = NULL;
  std::map<Tag,CDatabase*>::iterator i = db.find(id);
  if (i != db.end()) rc = i->second;
  return rc;
}

CDatabase* CDatabaseManager::GetAPTDatabase (void)
{
  return GetDatabase (DB_AIRPORT);
}

CDatabase* CDatabaseManager::GetAtsRouteDatabase (void)
{
  return GetDatabase (DB_ATS_ROUTE);
}

CDatabase* CDatabaseManager::GetCOMDatabase (void)
{
  return GetDatabase (DB_COMM);
}

CDatabase* CDatabaseManager::GetCTYDatabase (void)
{
  return GetDatabase (DB_COUNTRY);
}

CDatabase* CDatabaseManager::GetSTADatabase (void)
{
  return GetDatabase (DB_STATE);
}

CDatabase* CDatabaseManager::GetILSDatabase (void)
{
  return GetDatabase (DB_ILS);
}

CDatabase* CDatabaseManager::GetNAVDatabase (void)
{
  return GetDatabase (DB_NAVAID);
}

CDatabase* CDatabaseManager::GetOBSDatabase (void)
{
  return GetDatabase (DB_OBSTRUCTION);
}

CDatabase* CDatabaseManager::GetRWYDatabase (void)
{
  return GetDatabase (DB_RUNWAY);
}

CDatabase* CDatabaseManager::GetWPTDatabase (void)
{
  return GetDatabase (DB_WAYPOINT);
}

//===========================END OF FILE =====================================================================