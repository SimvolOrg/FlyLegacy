/*
 * Weather.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright (c) 2004 Chris Wallace
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

/*! \file Weather.cpp
 *  \brief Implements
 *                - CWindModel class
 *                - CWeatherManager class
 *
 */

#include <stdio.h>
#include "../Include/Weather.h"
#include "../Include/Atmosphere.h"
#include "../Include/Utility.h"
#include "../Include/Globals.h"
#include "../Include/TerrainCache.h"
#include "../Include/FileParser.h"
#include "../Include/Cloud.h"
#include "../Include/Fui.h"
#include "../Include/FuiParts.h"

static int wind_random_value_old = 0;
//=====================================================================
//  Table for user wind layers
//  5 Layers are defined
//  00000-05000 feet Surface layer
//  05000-10000 feet: Layer 1
//  10000-20000 feet: Layer 2
//  20000-30000 feet: Layer 3
//  30000-40000 feet: Layer 4
//=====================================================================
CmvalMap    windMAP;                    // Default wind layers
CListBox    windBOX;
//-----Title of windbox ---------------------------------------
CWndLine    windTITLE;                  // Title
//-----Layer name ----------------------------------------------
char *windNAME[] = {
  "Surface",
  "Layer 1",
  "Layer 2",
  "Layer 3",
  "Layer 4",
};
//=====================================================================
//  Cloud Layer name
//=====================================================================
char *cloudNAME[] = {
  "Clear",                // 0 => None
  "Stratus",              // 1 => Stratus
  "Few",                  // 2 = > coverage   1/8
  "Scattered",            // 3 = > coverage   3/8
  "Broken",               // 4 =>  coverage   5/8
  "Broken",               // 5 => Coverage    6/8
  "Overcast",             // 6 => Coverage    8/8
};
//=====================================================================
//  Shadow color according to cloud cover
//=====================================================================
float shadowCOL[] = {
  0.10f,                  // 0 No cloud
  0.11f,                  // 1 Stratus
  0.11f,                   // 2 Coverage 1/8
  0.12f,                   // 3 Coverage 3/8
  0.12f,                   // 4 Coverage 5/8
  0.13f,                   // 5 Coverage 6/8
  0.14f,                   // 6 Coverage 8/8
};
//=====================================================================
//  Shadow factor according to cloud cover
//========================== ===========================================
float shadowFAC[] = {
  1.0f,                  // 0 No cloud
  0.80f,                  // 1 Stratus
  0.80f,                   // 2 Coverage 1/8
  0.70f,                   // 3 Coverage 3/8
  0.70f,                   // 4 Coverage 5/8
  0.60f,                   // 5 Coverage 6/8
  0.50f,                   // 6 Coverage 8/8
};
//========================================================================================
//  Wind Layer
//  There is one wind layer per metar area. This layer define the surface wind at this area
//
//  User can define up to 5 other globals wind layers at differents altitudes.
//  If there is no metar area, then the user surface layer (if any) is used.
//  Wind parameters are interpolated from the metar layer to user layer .
//
//  Actually, when passing from one metar area to another, there is no interpolation for
//  the layer surface.  This may be upported latter as it is a bit complicated to
//  interpolate between different horizontal layers at the same altitude.
//========================================================================================
CWindLayer::CWindLayer(char No)
{ //-----Initi index ------------------------------------------
  nLayer  = No;
  //--- wind parameters --------------------------------------
  windDR1 = 0;          // Direction (°)
  windDR2 = 0;          // Direction (°)
  windKTS = 0;                  // Wind speed
  windGUS = 0;                  // Wind gust
}
//-------------------------------------------------------------------------------
//  Set up from METAR info
//  Wind surface layer is initialized from the METAR data if any
//  All  other layers are initialized from the wind FUI windows
//-------------------------------------------------------------------------------
void CWindLayer::SetUP(METAR_INFO &info)
{ CWeatherManager *wtm = globals->wtm;
  //-----Wind parameters for one layer --------------------------------
  windDR1 = float(info.wDR1);             // Main direction
  windDR2 = float(info.wDR2);             // Main direction
  windKTS = float(info.wASP);             // Wind average speed
  windGUS = float(info.wGSP);             // Gust is a ratio                  
  windGUS = (windKTS)?(windGUS  / windKTS):(1);
  windRDM  = RandomNumber(10);            // A random number for gust
  return;
}

//========================================================================================
// CWeatherManager
//
//
// The one and only instance of WeatherManager
//
//========================================================================================
CWeatherManager::CWeatherManager(void)
{ globals->wtm   = this;
  cArea     = 0;
  demux     = 0;
  //----Get default from ini file ---------------------------------------
  windDIR   = 0;
  windKTS   = 0;
  windXSP   = 0;
  windYSP   = 0;
  windTDR   = 0;
  int pm    = 0;
  float  va = 10.0;
  GetIniFloat("Meteo","Visibility",&va);
  mVIS      = va;
  va        = 1500;
  GetIniFloat("Meteo","CloudCeil",&va);
  mCEL      = va;
  pm        = 0;
  GetIniVar("Meteo", "CloudType",&pm);
  if (pm > CLOUD_END_LAYER) pm = CLOUD_END_LAYER;
  if (pm < 0)               pm = 0;
  mLAY      = pm;
  va        = 14.0;
  GetIniFloat("Meteo","DewPoint",&va);
  mDEW      = va;
  va        = 100;
  GetIniFloat("Meteo","Radius",&va);
  if (va > 200.0f)  va = 200.0f;
  Radius    = va;
  //----Check for no Meteo ---------------------
  int NoMT = 0;
  GetIniVar("Sim","NoMeteo",&NoMT);
  if (NoMT) globals->noMET++;
  //---Enter in dispatcher    ----------------
	globals->Disp.Enter(this, PRIO_WEATHER, DISP_EXCONT, 0);
}
//-----------------------------------------------------------------------
//  Get a layer from ini file 
//-----------------------------------------------------------------------
void CWeatherManager::AddDefaultLayer(char No)
{	char key[16];
	_snprintf(key,16,"Layer%d",No);
	char str[64];
	GetIniString("WIND",key,str,64);
	float	a;			// Altitude
	float d;			// Direction
	float s;			// Speed
	int nf = sscanf(str,"%f ft, %f deg , %f Kts",&a,&d,&s);
	if (nf != 3)		return;
	C3valSlot slot(a,d,s,0);
	windMAP.Enter(slot);
	return;
} 
//-----------------------------------------------------------------------
//  Get user wind layer (to be writen on file) 
//-----------------------------------------------------------------------
void CWeatherManager::GetDefaultWinds()
{ //--------------------------------------------
	C3valSlot slot(0,0,0,0);
	windMAP.Enter(slot);
	for (int k=1; k != 6; k++)	AddDefaultLayer(k);
  //-------------------------------------------
  windTITLE.FixeIt();
  windBOX.AddSlot(&windTITLE);
  //----Fill list box -------------------------
  char go = 1;
  char  k = 0;
  while (go)
  { C3valSlot *tab = windMAP.GetSlot(k);
    if (0 == tab)   break;
    CWndLine  *lin = new CWndLine();
    lin->SetSlotName(windNAME[k]);
    lin->SetSlot(k,tab);
    windBOX.AddSlot(lin);
    k++;
  }
  return;
}
//-----------------------------------------------------------------------
//  Return cloud cover name
//-----------------------------------------------------------------------
char *CWeatherManager::GetCloudCover()
{ char No = cArea->GetLayer();
  return cloudNAME[No];
}
//-----------------------------------------------------------------------
//  Return Shadow factor
//-----------------------------------------------------------------------
void CWeatherManager::GetShadowParameters(float *c,float *a)
{ char No = cArea->GetLayer();
  *c = shadowCOL[No];
  *a = shadowFAC[No];
  return;
}
//-----------------------------------------------------------------------
//  Set Default parameters 
//-----------------------------------------------------------------------
void CWeatherManager::SetDefault()
{ strncpy(Info.iden,"default",15); // Default ident
  Info.pos.lon = TC_FULL_WRD_ARCS * 2;
  Info.pos.lat = TC_FULL_WRD_ARCS * 2;
  //----Build user winds -------------------
  GetDefaultWinds();
  //---Set Wind parameters -----------------
  C3valSlot *srf = windMAP.GetSlot(SURF_LAYER);  
  Info.wDR1   = srf->GetU();            // Average direction
  Info.wDR2   = srf->GetU();            // Average direction
  Info.wASP   = srf->GetV();            // Average speed
  Info.wGSP   = srf->GetV();            // Average speed
  //---Arm randomizer ----------------------
  wDIR.Set(265,10,2);						// Randomize direction
  wSPD.Set(  3, 5,5);						// Randomize speed
	wTRB.Set(0, 1000, 1);					// Turbulence event
  //---Create default metar area -----------
  Info.vMIL   = mVIS;       // Visibility
  Info.Layer  = mLAY;       // Cloud layer
  Info.Ceil   = mCEL;       // cloud Ceiling
  Info.DewP   = mDEW;       // Dew point
  Info.Temp   = globals->atm->GetTemperatureC();
  Info.Baro   = globals->atm->GetPressureHPA();
  dArea.SetFromMETAR(Info);
  //---Assign default area ---------------
  ChangeMetar(&dArea);
  return;
}

//-----------------------------------------------------------------------
//  Free metar area
//-----------------------------------------------------------------------
CWeatherManager::~CWeatherManager(void)
{ std::map<Tag,CMeteoArea*>::iterator ip;
  for (ip = Areas.begin(); ip != Areas.end(); ip++) delete ip->second;
  Areas.clear();
}
//-----------------------------------------------------------------------
//  Init Infos
//-----------------------------------------------------------------------
void CWeatherManager::Init (void)
{ int opt = 0;
  GetIniVar("TRACE","Meteo",&opt);
  tr  = (U_CHAR)opt;
  SetDefault();
	//--- Check for no wind ---------------------------------------
	nw	= 0;
	nw	|= HasIniKey("TRACE","AeroForce")? (1):(0);
	nw  |= HasIniKey("TRACE","AeroMoment")?(1):(0);
  //-------------------------------------------------------------
  srand ((unsigned) clock ());
}
//------------------------------------------------------------------------------
//  Check for METAR data and create a new area when METAR is found for
//  the requested airport
//------------------------------------------------------------------------------
void CWeatherManager::GetMetar(CmHead *obj)
{ Apt = (CAirport*)obj;
  char *name = Apt->GetIdent();
  char fnam[PATH_MAX];
  _snprintf(fnam,FNAM_MAX,"METAR/%s.TXT",name);
  FILE  *f  = fopen(fnam,"r");
  if (0 == f)  return;
  //--- Decode the METAR Bulletin --------------------------
  METARparser(f,&Info,name);
  if (1 == Info.code) return;
  //--- Create a METAR AREA --------------------------------
  Info.pos  = obj->GetPosition();
  if (tr)  TRACE("WTM: -- ADD a METAR from %s",name);
  AddMetar();
  return;
}
//------------------------------------------------------------------------------
//  Get the Metar Area  for a requesting QGT
//  There is a timing problem between the DatabaseCache and the formation of
//  Quarter global-tile
//  -After init or teleportation we must wait for the database cache to stabilize
//   before assigning METAR to the requesting QGT
//   The Weather Manager must synchronize with the Database cache for this to happen
//
//------------------------------------------------------------------------------
CMeteoArea *CWeatherManager::GetMetar(Tag kid)
{ CMeteoArea *def = (globals->dbc->NotStable())?(0):(&dArea);
  std::map<Tag,CMeteoArea *>::iterator it = Areas.find(kid);
  return (it == Areas.end())?(def):((*it).second);
}
//------------------------------------------------------------------------------
//  Add a new METAR area on the list
//  -Set parameters from the METAR data
//  -CallTterrain Cache to assign Metar to QGT
//  A new METAR is added when METAR bulletin is found for a new airport
//  added to the database cache.
//------------------------------------------------------------------------------
void CWeatherManager::AddMetar()
{ //MEMORY_LEAK_MARKER ("CMeteoArea")
  CMeteoArea *ma = new CMeteoArea();
  //MEMORY_LEAK_MARKER ("CMeteoArea")
  ma->SetFromMETAR(Info);
  Tag key = StringToTag(Info.iden);
  Areas[key] = ma;
  ma->SetKey(key);
  Apt->SetMetar(key);
  globals->tcm->AssignMetar(ma);
  return;
}
//------------------------------------------------------------------------------
//  Remove METAR from the list
//  obj must be an airport from the database cache
//------------------------------------------------------------------------------
void CWeatherManager::RemoveMetar(CmHead *obj)
{ CAirport *apt = (CAirport*)obj;
  Tag   kid = apt->GetMetar();
  if (0 == kid)   return;
  std::map<Tag,CMeteoArea *>::iterator it = Areas.find(kid);
  if (it == Areas.end())    gtfo("No Metar to delete for %s", apt->GetIdent());
  CMeteoArea *ma = (*it).second;
  if (tr) TRACE("WTM: -- DELETE METAR %s",ma->GetIdent());
  delete ma;
  Areas.erase(kid);
  //--- if current area is this one set default Area -------
  if (ma != cArea)        return;
  if (0 == globals->tcm)  return;
  SPosition *pos = globals->tcm->PlaneArcsPos();
  ChangeMetar(&dArea);
  return;
}
//------------------------------------------------------------------------------
//  Attach the QGT to a METAR if the QGT center is covered by the METAR area
//  NOTE: The first area covering the QGT position is assigned, not necessarily
//        the nearest area in case of overlapping areas.
// ac = 1 => The Aircraft is in this QGT
//------------------------------------------------------------------------------
void CWeatherManager::AttachMetar(C_QGT *qgt,bool ac)
{ std::map<Tag,CMeteoArea *>::iterator it; 
  for (it = Areas.begin(); it != Areas.end(); it++)
  { CMeteoArea *ma = (*it).second;
    int in = ma->RangeOf(*qgt->GetMidPoint());
    if (0 == in)  continue;
    //---Link the QGT to the METAR area ------------------
    qgt->SetMETAR(ma->GetKey());
    //---If QGT is aircraft QGT the area becomes current -
    if (ac) ChangeMetar(ma); 
    if (tr) TRACE("WTM: -- Attach METAR %s to QGT(%03d-%03d)",
                        ma->GetIdent(),qgt->GetXkey(),qgt->GetZkey());
    return;
  }
  //----NO METAR to ASSOCIATE ----------------------------
  return;
}
//------------------------------------------------------------------------------
//    Select area from aircraft position
//------------------------------------------------------------------------------
int CWeatherManager::SelectArea()
{ SPosition *pos = globals->tcm->PlaneArcsPos();
  std::map<Tag,CMeteoArea *>::iterator it; 
  for (it = Areas.begin(); it != Areas.end(); it++)
  { CMeteoArea *ma = (*it).second;
    int in = ma->RangeOf(*pos);
    if (0 == in)  continue;
    //--- find area ------------------------------------
    if (tr) TRACE("WTM: -- CHANGE METAR from %s TO %s",cArea->GetIdent(),ma->GetIdent());
    //---Save the surface wind parameters --------------
    ChangeMetar(ma);
    return 1;
  }
  //----NO METAR to ASSOCIATE ----------------------------
  if (tr)   TRACE("WTM: -- DEFAULT METAR %s IS SET",cArea->GetIdent());
  //----Reset to default area ----------------------------
  ChangeMetar(&dArea);
  return 0;
}
//------------------------------------------------------------------------------
//  Change actual meteo area for the requested ma
//  except if already in place
//------------------------------------------------------------------------------
void CWeatherManager::ChangeMetar(CMeteoArea *ma)
{ if (ma == cArea)    return;
  cArea = ma;
  globals->atm->LocalTempC(ma->Temp);
  //--- Change wind surface parameters ----------------------------
  CWindLayer *wnd = ma->wind;
  windDIR = wnd->windDR1;
  windASP = wnd->windKTS;
  //--- Update wind map ------------------------------------------
  windMAP.ChangeU(SURF_LAYER,windDIR);
  windMAP.ChangeV(SURF_LAYER,windASP);
  //--- Update randomizer ----------------------------------------
  float dir = windDIR;
  int a = Wrap360(dir - 10);
  wDIR.Range(a,20);
  float spd = windASP;
  int c = spd - 5;
  if (c < 0)  c = 0;
  wSPD.Range(c,10);
  //----Update the sea levl barometer ----------------------------
  globals->atm->LocalPressureHPA(ma->Baro);
  return;
}
//------------------------------------------------------------------------------
//  Change cloud Layer
//------------------------------------------------------------------------------
void CWeatherManager::ChangeCloud(char nl)
{ if (cArea->SameLayer(nl))   return;
  globals->cld->KillClouds();
  cArea->SetLayer(nl);
  return;
}
//------------------------------------------------------------------------------
//  Return wind speed in metres/sec
//------------------------------------------------------------------------------
float CWeatherManager::GetWindMPS()
{
  return METRES_SEC_PER_KNOT(windKTS);
}
//------------------------------------------------------------------------------
//  Update Wind parameters from actual Metar
//  1) Instant values are updated from the randomizer
//  2) The wind rose direction and trigonometric direction are computed
//  3)  Every now and then, the new winds parameters are interpolated from
//      the wind map according to altitude
//      New parameters are entered into the randomizers
//      Wind velocity composites are computed with average wind speed
//
//------------------------------------------------------------------------------
void CWeatherManager::UpdateWind(float dT)
{ if (nw)			return;						// No wind from configuration
	//----Update actual speed for aerodynamic -----------------------------
  windDIR = wDIR.TimeSlice(dT);                         // meteo direction
  windKTS = wSPD.TimeSlice(dT);
  windDIR = Wrap360(windDIR);
  //---Compute wind speed composite in arcsec per second    -------------
  double rose = Wrap360(windDIR + 180);
  windROS     = rose;
  double trig = Wrap360(270 - windDIR);                 // trigo direction
  windTDR     = trig;  
  //TRACE("WIND dir=%.2f rose=%.2f trig=%.2f",windDIR,rose,trig);
  //--- Update basic from altitude every now and then -------------------
  if (demux & 0x0F)    return;
  //----Update average speed ASP = f(altitude) --------------------------
  TUPPLE3 tup = {globals->geop.alt,0,0,0};
  windMAP.Lookup(tup);
  float dir = tup.out1;
  float a = Wrap360(dir - 10);
  wDIR.Range(a,20);
  float spd = tup.out2;
  float c = spd - 4;
  if (c < 0)  c = 0;
  wSPD.Range(c,8);
  //---Compensate x composite for the latitude --------------------------
  double alfa = DegToRad(trig);
  double rdf  = GetReductionFactor(globals->geop.lat);
  //---Compute wind speed X and Y composites ----------------------------
  cosw = windASP * 60 * cos(alfa) * TC_HH_PER_SEC * rdf; // arcsec per sec
  sinw = windASP * 60 * sin(alfa) * TC_HH_PER_SEC;       // arcsec per sec
  windXSP    = cosw * windASP * dT;
  windYSP    = sinw * windASP * dT;
  return;
}
//------------------------------------------------------------------------------
//  Check if we must change METAR AREA
//------------------------------------------------------------------------------
void CWeatherManager::UpdateArea(float dT)
{ SPosition *pos = globals->tcm->PlaneArcsPos();
  int in = cArea->RangeOf(*pos);
  //--- Look for a new METAR -----------------------
  if (!in) SelectArea();
  return;
}
//------------------------------------------------------------------------------
//    Update metar infos
//    For each Metar area, the wind vector components are updated 
//    Temperature is updated every 8 frames
//------------------------------------------------------------------------------
int CWeatherManager::TimeSlice(float dT,U_INT FrNo)
{ //--- Update Cloud subsystem ---------------------
  bool ok = (globals->noMET == 0);
  if (ok) globals->cld->TimeSlice(dT,FrNo);
  //----Update wind component from METAR -----------
  demux++;
  UpdateWind(dT);
  //---- Check if we are leaving the area ----------
  if (0 == (demux & 0x03))  UpdateArea(dT);
  return 1;
}
//--------------------------------------------------------------------------
//  Display metar infos
//--------------------------------------------------------------------------
void  CWeatherManager::Probe(CFuiCanva *cnv)
{ //--- Edit wind parameters ------------------
  cnv->AddText( 1,0,"Metar: %s",cArea->GetIdent());
  cnv->AddText(12,0,"Wind Direction: %.0f",windDIR);
  cnv->AddText(30,0,"Force: %.0f KTS",windKTS);
  cnv->AddText(45,1,"Temperature: %.1f C",globals->atm->GetTemperatureC());
  //--- Edit cloud system parameters --------
  CCloudSystem *cld = globals->cld;
  cnv->AddText(1,1,"Clouds: Reals %03d ",cld->GetTotal());
  return;
}
//=====================================================================================
//  Class Meteo Area
//=====================================================================================
//  Set default values 
//--------------------------------------------------------------------------
CMeteoArea::CMeteoArea()
{ iden[0] =  0;
  Radius  = 60;             // 60 Nautical miles
  Layer   =  0;
  Ceil    =  0;
  wind    =  0;
}
//---------------------------------------------------------------------------
//  Destroy Meteo area
//----------------------------------------------------------------------------
CMeteoArea::~CMeteoArea()
{ if (wind) delete wind;
}
//---------------------------------------------------------------------------
//  Set up from metar info
//  There are no provisions to change the following default values in METAR
//    Scale is set to default 1.
//    Radius is set to 80 nmiles
//  NOTE:   For wind, the X,Y speed composites are defined as the number
//          of arcsecs per second.  Thus to get the increment in arcsec along 
//          any composite during time(DT) we have 
//          INCx = SPDx * DT;
//          INCy = SPDy * DT;
//      The X composite is reduced by the RDF factor rdf = f(Latitude) because
//      for a given KTS (numbers of miles defined at equator), 
//      the corresponding arcsec per hour is reduced by the cos(lat) factor.
//      Note also that the wind direction is the direction from which the wind
//      blows.  Example: 300° means the wind blows from 300° to 130°  
//---------------------------------------------------------------------------
void CMeteoArea::SetFromMETAR(METAR_INFO &info)
{ CWeatherManager *wtm = globals->wtm;
  strncpy(iden,info.iden,15);             // Init airport ident
  Pos     = info.pos;                     // Airport coordinates
  //----Cloud for only one layer actually ------------------------
  Radius  = wtm->GetMetarRadius() * 60;   // Area radius in arc second
  Radius  = Radius * Radius;              // Squared for compare
  Ceil    = float(info.Ceil);             // Cloud ceiling
  Layer   = info.Layer;                   // Cloud layer type
  //-----Init temperature ----------------------------------------
  Temp    = info.Temp; 
  Baro    = info.Baro;
  //-----Create the surface wind layer ---------------------------
  //MEMORY_LEAK_MARKER ("CWindLayer")
  wind    = new CWindLayer(0);
  //MEMORY_LEAK_MARKER ("CWindLayer")
  wind->SetUP(info);
  return;
}
//---------------------------------------------------------------------------
//  Check if position is in range of this area
//  We just check against the squared "flat" distance in arcsec 
//---------------------------------------------------------------------------
int CMeteoArea::RangeOf(SPosition &ps)
{ double dx = LongitudeDifference(Pos.lon,ps.lon);
  double dy = Pos.lat - ps.lat;
  double sq = (dx*dx) + (dy*dy);
  return (sq >= Radius)?(0):(1);
}
  
//=========================END OF FILE ==============================================================
