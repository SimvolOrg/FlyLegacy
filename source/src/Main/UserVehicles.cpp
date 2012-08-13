/*
 * UserVehicles.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
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

/*! \file UserVehicles.cpp
 *  \brief Implements CVehicleObject sub-object classes
 *
 * Class CVehicleObject, implemented in WorldObjects.cpp, is the base class
 *   for the three supported types of user vehicles : fixed-wing airplanes,
 *   helicopters and ground vehicles.  Each user vehicle's top-level
 *   specification is defined in an NFO file, which references the many
 *   other stream files containing area-specific sub-objects for the
 *   user vehicle.  This file implements the classes that encapsulate
 *   these various sub-objects.
 */


#include "../Include/Globals.h"
#include "../Include/BaseSubsystem.h"
#include "../Include/FlyLegacy.h"
#include "../Include/Subsystems.h"
#include "../Include/PistonEngine.h"
#include "../Include/Autopilot.h"
#include "../Include/Cameras.h"
#include "../Include/Utility.h"
#include "../Include/FuiPlane.h"
#include "../Include/AudioManager.h"
#include "../Include/Atmosphere.h"
#include "../Include/Joysticks.h" 
#include "../Include/3Dmath.h"
#include "../Include/RadioGauges.h"
#include "../Include/Robot.h"
#include "../Include/PlanDeVol.h"
using namespace std;

///////// DEBUG STUFF TO REMOVE LATER ////////////////////////////////
#ifdef  _DEBUG	
  //#define _DEBUG_suspension  //print lc DDEBUG file ... remove later
  //#define _DEBUG_brakes      //print lc DDEBUG file ... remove later
  #define _DEBUG_crash       //print lc DDEBUG file ... remove later
  //#define _DEBUG_altitude    //print lc DDEBUG file ... remove later
  //#define _DEBUG_mixer       //print lc DDEBUG file ... remove later
#ifdef _DEBUG_suspension
  #define _DEBUG_brakes
#endif
#endif
//===================================================================
//  State table
//===================================================================
char *engSTA[] = {
  "STOPPED",      // 0
  "CRANKING",     // 1
  "RUNNING",      // 2
  "STOPPING",     // 3
  "FAILING",      // 4
  "WINDMILL",     // 5
  "CATCHING",     // 6
  "MISSFIRE",     // 7
};
//===================================================================
//  Stop  table
//===================================================================
char *engSTOP[] = {
  "",
  "noMag",
  "noFuel",
  "stall",
  "mixt",
  "cut",
  "Abort",
  0,
  0,
};
//===================================================================
// CWhl base class
//===================================================================
CWhl::CWhl (void)
{
}
//=====================================================================================
//  CSimulatedVehicle
//  Read and support SVH file
//=====================================================================================
CSimulatedVehicle::CSimulatedVehicle ()
{ vsnd  = 0;                      // Sound manager
  mdieh = 0;
  mpitd = 0;
  macrd = 0;
	//----------------------------------------------
  mAlt.sender = 'simu';
  mSpd.sender = 'simu';
  mVsi.sender = 'simu';
  mMag.sender = 'simu';
  mRpm.sender = 'simu';
  mMap.sender = 'simu';
	//--- Init messages -----------------------------
  mAlt.id = MSG_GETDATA;
  mSpd.id = MSG_GETDATA;
  mVsi.id = MSG_GETDATA;
  mMag.id = MSG_GETDATA;
  mRpm.id = MSG_GETDATA;
  mMap.id = MSG_GETDATA;
	//----------------------------------------------
	elapsed = 0;
}
//----------------------------------------------------------------------
//  Read the file
//----------------------------------------------------------------------
void CSimulatedVehicle::Init(char* svhFilename, CWeightManager *wgh)
{	vehi_wgh = wgh;
	//--- Read the SVH file -------------------------
  SStream s(this,"WORLD",svhFilename);
	return;
}
//----------------------------------------------------------------------
//  Delete this object
//----------------------------------------------------------------------
CSimulatedVehicle::~CSimulatedVehicle (void)
{ //----Delete resources -----------
  SAFE_DELETE (vsnd);
  SAFE_DELETE (mdieh);
  SAFE_DELETE (mpitd);
  SAFE_DELETE (macrd);
}
//----------------------------------------------------------------------
//  Read all parameters
//----------------------------------------------------------------------
int CSimulatedVehicle::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'type':
    // Object type
    ReadTag (&type, stream);
    rc = TAG_READ;
    break;
  case 'mxdt':
    // Minimum simulation frame rate
    ReadUInt (&mxdt, stream);
    rc = TAG_READ;
    break;
  case 'name':
    // Aircraft name
    ReadString (name, sizeof (name) - 1, stream);
    rc = TAG_READ;
    break;
  case 'acid':
    // Tail number (default)
    ReadString (acid, sizeof (acid) - 1, stream);
    rc = TAG_READ;
    break;
  case 'make':
    // Manufacturer model name
    ReadString (make, sizeof (make) - 1, stream);
    rc = TAG_READ;
    break;
  case 'emas':
    // Empty mass (slugs)
    ReadFloat (&emas, stream);
    vehi_wgh->svh_emas = emas;
    rc = TAG_READ;
    break;
  case 'mine':
    // Moments of Inertia
    ReadVector (&mine, stream);
    vehi_wgh->svh_mine = mine;
    DEBUGLOG ("CSimulatedVehicle::Read <mine> %.2f %.2f %.2f", vehi_wgh->svh_mine.x, vehi_wgh->svh_mine.y, vehi_wgh->svh_mine.z);
    rc = TAG_READ;
    break;
  case 'CofG':
    // Default center of gravity, relative to visual model center
    ReadVector (&CofG, stream);
    vehi_wgh->svh_cofg = CofG;
    rc = TAG_READ;
    break;
  case 'imbl':
    // Imbalance limit
    ReadFloat (&imbl, stream);
    rc = TAG_READ;
    break;
  case 'iceR':
    // Ice accumulation rate
    ReadFloat (&iceR, stream);
    rc = TAG_READ;
    break;
  case '000a':
    // Wing area (sq. ft.)
    ReadFloat (&wingarea, stream);
    rc = TAG_READ;
    break;
  case '001a':
    // Wing span (ft.)
    ReadFloat (&wingspan, stream);
    rc = TAG_READ;
    break;
  case '002a':
    // Mean chord (ft.)
    ReadFloat (&meanchord, stream);
    rc = TAG_READ;
    break;
  case 'CEIL':
    // Ceiling (ft.)
    ReadFloat (&ceiling, stream);
    rc = TAG_READ;
    break;
  case '043a':
    // Max cruise speed (kts)
    ReadFloat (&maxcruisespeed, stream);
    return TAG_READ;
  case '044a':
    // Approach speed (kts)
    ReadFloat (&approachspeed, stream);
    return TAG_READ;
  case '044b':
    // Brake distance to stop at above speed
    ReadFloat (&brakeDist, stream);
    return TAG_READ;
  case '045a':
    // Best climb rate (fps)
    ReadFloat (&wingarea, stream);
    return TAG_READ;
  case '048a':
    // Never exceed speed (kts)
    ReadFloat (&vne, stream);
    return TAG_READ;
  case 'saoa':
    // Stall angle of attack
    ReadFloat (&stallaoa, stream);
    rc = TAG_READ;
    break;
  case 'posG':
    // Positive G limit
    ReadFloat (&posG, stream);
    rc = TAG_READ;
    break;
  case 'aoaL':
    // Angle of attack limit
    ReadFloat (&aoaL, stream);
    rc = TAG_READ;
    break;
  case 'negG':
    // Negative G limit
    ReadFloat (&negG, stream);
    rc = TAG_READ;
    break;
  case 'stal':
    // Simple stall speed (kts)
    ReadFloat (&stallspeed, stream);
    rc = TAG_READ;
    break;
  case 'blst':
    ReadFloat (&blst, stream);
    rc = TAG_READ;
    break;
  case 'wTrb':
    // Wing turbulence data
    ReadFloat (&wTrbSpeed, stream);   // Speed
    ReadFloat (&wTrbTimK, stream);    // Time constant
    ReadFloat (&wTrbCeiling, stream); // Ceiling ft MSL
    ReadFloat (&wTrbDuration, stream);  // Duration sec
    rc = TAG_READ;
    break;
  case 'nTrb':
    // ??? turbulence data
    ReadFloat (&nTrbSpeed, stream);   // Speed
    ReadFloat (&nTrbTimK, stream);    // Time constant
    ReadFloat (&nTrbCeiling, stream); // Ceiling ft MSL
    ReadFloat (&nTrbDuration, stream);  // Duration sec
    rc = TAG_READ;
    break;
  case 'dieh':
    { // Diehedral Effect Table
      CDataSearch map(stream);
      mdieh = map.GetTable();
      return TAG_READ;
    }
  case 'pitd':
    { // Pitch moment table
      CDataSearch map(stream);
      mpitd = map.GetTable();
      return TAG_READ;
    }
  case 'acrd':
    { // Rudder deflection vs. bank angle
      CDataSearch map(stream);
      macrd = map.GetTable();
      return TAG_READ;
    }
  case 'sfxd': 
    {
      // Sound effects //**
      ReadTag (&sfxdTag, stream);
      ReadString (sfxdFilename, sizeof (sfxdFilename) - 1, stream);
      //MEMORY_LEAK_MARKER ("vsnd")
      vsnd = new CVehicleSound (sfxdFilename);
      //MEMORY_LEAK_MARKER ("vsnd")
      rc = TAG_READ;
    }
    break;
  case 'mAlt':
    // Status bar display : Altitude message
    ReadMessage (&mAlt, stream);
    rc = TAG_READ;
    break;
  case 'mSpd':
    // Status bar display : Airspeed message
    ReadMessage (&mSpd, stream);
    rc = TAG_READ;
    break;
  case 'mVsi':
    // Status bar display : Vertical speed message
    ReadMessage (&mVsi, stream);
    rc = TAG_READ;
    break;
  case 'mMag':
    // Status bar display : Magnetic compass heading message
    ReadMessage (&mMag, stream);
    //user_message.push_back (mMag);
    rc = TAG_READ;
    break;
  case 'mRpm':
    // Status bar display : Engine RPM
    ReadMessage (&mRpm, stream);
    //user_message.push_back (mRpm);
    rc = TAG_READ;
    break;
 case 'mUse':
 case 'mMap':
    // Status bar display : Engine RPM
    ReadMessage (&mMap, stream);
    //user_message.push_back (mMap);
    rc = TAG_READ;
    break;

  }

  if (rc != TAG_READ) {
    // Tag was not processed by this object, it is unrecognized
    WARNINGLOG ("CSimulatedVehicle::Read : Unrecognized tag <%s>", TagToString(tag));
  }

  return rc;
}
//-------------------------------------------------------------------------------
//  All parameters are read. Finish setup
//-------------------------------------------------------------------------------
void CSimulatedVehicle::ReadFinished (void)
{ //--- Compute brake parameters ------------
	double bfs	=  FN_METER_FROM_MILE(approachspeed) / 3600;				// Brake speed meter /sec
	brakeDist		=  FN_METRE_FROM_FEET(brakeDist);
	accBrake		= (bfs * bfs) / (2 * brakeDist);									
  return;
}
//-------------------------------------------------------------------------------
void CSimulatedVehicle::Write (SStream *stream)
{
}
//-------------------------------------------------------------------------------
//	Time slice:  Compute CG_ISU and refresh status bar 
//-------------------------------------------------------------------------------
void CSimulatedVehicle::Timeslice (float dT)
{ CalcNewCG_ISU ();							// 
  if (globals->sBar) {
	    elapsed += dT;
    if (elapsed > globals->status_bar_limit) {
      elapsed = elapsed - globals->status_bar_limit;
      PrintInfo (globals->sBar); // 1 => aircraft data  2 => angle & position
    }
  }
}
//-------------------------------------------------------------------------------
//	Time slice:  Compute CG_ISU 
//-------------------------------------------------------------------------------
void CSimulatedVehicle::CalcNewCG_ISU (void)
{ mveh->wgh.GetVisualCG (newCG_ISU); // lc 052910 +
  newCG_ISU.x = FN_METRE_FROM_FEET (-newCG_ISU.x); // lc 052910 +
  newCG_ISU.y = FN_METRE_FROM_FEET (+newCG_ISU.y); // lc 052910 +
  newCG_ISU.z = FN_METRE_FROM_FEET (+newCG_ISU.z); // lc 052910 +
}

void CSimulatedVehicle::PrintInfo (int bar_cycle)
{
	  if (1 == bar_cycle) {
    float altitude = 0.0f;
    float speed = 0.0f;
    float vert_speed = 0.0f;
    float magn_compass = 0.0f;
    float rpm = 0.0f;
 
    //mAlt.user.u.datatag = 'alti';
    Send_Message (&mAlt);
    altitude      = float(mAlt.realData);
    //mSpd.user.u.datatag = 'sped'; // 
    Send_Message (&mSpd);
    speed         =  float(mSpd.realData);
    //mVsi.user.u.datatag = 'vsi_';
    Send_Message (&mVsi);
    vert_speed    = float(mVsi.realData);
    //mMag.user.u.datatag = 'yaw_';
    Send_Message (&mMag);
    magn_compass  = float(mMag.realData);
    Send_Message (&mRpm);
    rpm           = float(mRpm.realData);

    char buff [256] = {0};
    int fps = 0;
    if (globals->dST) fps = (1 / globals->dST) + 1;
    _snprintf (buff,255, "Alt = %-5.0f\t\t  Speed = %-4.0f\t\t Vsi = %-5.0f\t\t Yaw = %-3.0f\t\t Rpm = %-5.0f\t\t fps = %03d",
      altitude, speed, vert_speed, magn_compass, rpm, fps);
		buff[255] = 0;
    DrawNoticeToUser (buff, 1.0f);
  }
  //
  else 
  if (2 == bar_cycle) {
    SPosition pos_to    = mveh->GetPosition();				
    SVector orientation = globals->iang; 
    char buff [128] = {'\0'};
    char edt1 [128] = {'\0'};
    char edt2 [128] = {'\0'};
    EditLon2DMS(pos_to.lon, edt1);
    EditLat2DMS(pos_to.lat, edt2);
    _snprintf (buff,255, "@1 %.2f %.2f %.2f [%s] [%s] alt=%.0f\n",
             RadToDeg (orientation.x), RadToDeg (orientation.y), RadToDeg (orientation.z),
             edt1, edt2, pos_to.alt
             );
		buff[127] = 0;
    DrawNoticeToUser (buff, 1);
  }
}

//===========================================================================
// CFuelSystem
//===========================================================================
CFuelSystem::CFuelSystem ()
{ cInd    = 0;
  fuel    = ENGINE_FUELED;
  // Initialize data members
  grad  = AIRPORT_FUEL_100LL;
  gradX = 3;
  //---Init trace ----------------------------------------
  int t = 0;
  GetIniVar("TRACE","FuelSystem",&t);
  Tr  = char(t);
}
//-----------------------------------------------------------------------
//  Init the system
//-----------------------------------------------------------------------
void	CFuelSystem::Init(char* gasFilename, CEngineManager *eng,  CWeightManager *wgh)
{	eng_mng = eng;
  gas_wgh = wgh;
	// Populate data members from GAS stream file
  SStream s(this,"WORLD",gasFilename);
	return;
}
//-----------------------------------------------------------------------
//  Destroy the system
//-----------------------------------------------------------------------
CFuelSystem::~CFuelSystem (void)
{ // delete fsub objects
  std::vector<CFuelSubsystem*>::iterator i;
  for (i = fsub.begin(); i != fsub.end(); i++) {
    CFuelSubsystem *f = *i;
    delete f;
  }
  ctank.clear();
  tanks.clear();
}
//-----------------------------------------------------------------------
//  Read parameters
//-----------------------------------------------------------------------
int CFuelSystem::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'grad':
    // Preferred fuel grade
    {
      char s[64];
      ReadString (s, 64, stream);
      if (stricmp (s, "AVGAS_80") == 0) {
        grad  = AIRPORT_FUEL_80;
        gradX = 1;
      } else if (stricmp (s, "AVGAS_100") == 0) {
        grad  = AIRPORT_FUEL_100;
        gradX = 2;
      } else if (stricmp (s, "AVGAS_100LL") == 0) {
        grad  = AIRPORT_FUEL_100LL;
        gradX = 3;
      } else if (stricmp (s, "AVGAS_115") == 0) {
        grad  = AIRPORT_FUEL_115;
        gradX = 4;
      } else if (stricmp (s, "JET_A") == 0) {
        grad  = AIRPORT_FUEL_JETA;
        gradX = 5;
      } else if (stricmp (s, "JET_A1") == 0) {
        grad  = AIRPORT_FUEL_JETA1;
        gradX = 6;
      } else if (stricmp (s, "JET_A1PLUS") == 0) {
        grad  = AIRPORT_FUEL_JETA1PLUS;
        gradX = 7;
      } else if (stricmp (s, "JET_B") == 0) {
        grad  = AIRPORT_FUEL_JETB;
        gradX = 8;
      } else if (stricmp (s, "AVGAS_JET_BPLUS") == 0) {
        grad  = AIRPORT_FUEL_JETBPLUS;
        gradX = 9;
      } else if (stricmp (s, "AUTOMOTIVE") == 0) {
        grad   = AIRPORT_FUEL_AUTOMOTIVE;
        gradX = 10;
      } else {
        WARNINGLOG ("CFuelSystem::Read : Unknown fuel grade %s", s);
      }
    }
    rc = TAG_READ;
    break;

  case 'fsub':
    // Fuel subsystem
    {
      char type_string[64];
      ReadString (type_string, 64, stream);
      Tag type = StringToTag (type_string);

      CFuelSubsystem *f = NULL;

      switch (type) {
      case SUBSYSTEM_FUEL_SUBSYSTEM: // 'fSub'
        f = new CFuelSubsystem;
        break;
      case SUBSYSTEM_FUEL_TAP:
        f = new CFuelTap;
        break;
      case SUBSYSTEM_FUEL_CELL:
        f = new CFuelCell(cInd++);
        break;
      case SUBSYSTEM_FUEL_COLR:
        f = new CFuelCollector(cInd++);
        break;
      case SUBSYSTEM_FUEL_HEATER:
        f = new CFuelHeater;
        break;
      case SUBSYSTEM_FUEL_MATH:
        f = new CFuelMath;
        break;
      case SUBSYSTEM_FUEL_PUMP:
        f = new CFuelPump;
        break;
      case SUBSYSTEM_FUEL_SOURCE:
        f = new CFuelSource;
        break;
      case SUBSYSTEM_FUEL_TANK:
        f = new CFuelTank(cInd++);
        break;
      case SUBSYSTEM_ENGINE_FUEL_PUMP:
        f = new CEngineFuelPump;
        break;
      case SUBSYSTEM_FUEL_MANAGEMENT:
        f = new CFuelManagement;
        break;
      case SUBSYSTEM_APU_FUEL_SOURCE:
        f = new CAPUFuelSource;
        break;
      }

      if (f != NULL) {
        // A valid CFuelSubsystem was instantiated, load it
        f->SetParent(mveh);
        ReadFrom (f, stream);
        fsub.push_back (f);
        // verify if a CFuelSubsystem is related with weight
        // then add its pointer to the WeighManager
        if (f->IsaTank()) {
          CFuelCell *cel  = (CFuelCell *) f;
          gas_wgh->gas_cell.push_back(cel);
          tanks.push_back(cel);
        }
        rc = TAG_READ;
      } else {
        WARNINGLOG ("CFuelSystem : Unknown fsub type = %s",
          type_string);
      }
    }
    break;
  }

  if (rc != TAG_READ) {
    // Tag was not processed by this object, it is unrecognized
    WARNINGLOG ("CFuelSystem::Read : Unrecognized tag <%s>", TagToString(tag));
  }

  return rc;
}

//-----------------------------------------------------------------------
//  All parameters are read
//-----------------------------------------------------------------------
void CFuelSystem::ReadFinished (void)
{ LinkAllCells();
}
//-----------------------------------------------------------------------
//  write the system
//-----------------------------------------------------------------------
void CFuelSystem::Write (SStream *stream)
{
}
//-----------------------------------------------------------------------
//  Return the weight(lbs) of the qty(gal)
//  TODO:  compute according to fuel grad?
//-----------------------------------------------------------------------
float CFuelSystem::GetWeight(float qty)
{ return (qty * FUEL_LBS_PER_GAL);
}
//-----------------------------------------------------------------------
//  Get all fuel cells
//-----------------------------------------------------------------------
void CFuelSystem::GetAllCells(std::vector<CFuelCell*> &list)
{ list = tanks;
  return;
}
//-----------------------------------------------------------------------
//  Get one fuel subsystems identified by Tag
//-----------------------------------------------------------------------
CFuelSubsystem  *CFuelSystem::GetSubsystem(Tag t)
{ std::vector<CFuelSubsystem*>::iterator sf;
  for (sf = fsub.begin(); sf != fsub.end(); sf++)
  { if ((*sf)->IsNot(t))   continue;
    return (*sf);
  }
  return 0;
}
//-----------------------------------------------------------------------
//  Link all cells
//-----------------------------------------------------------------------
void CFuelSystem::LinkAllCells()
{ std::vector<CFuelSubsystem*>::iterator sb;
  for (sb = fsub.begin(); sb != fsub.end(); sb++)
  { CFuelSubsystem *fs = (*sb);
    fs->LinkCell(this);
    if (fs->IsType(SUBSYSTEM_FUEL_TAP))   eng_mng->SetTAP((CFuelTap *)fs);
  }
  return;
}
//-----------------------------------------------------------------------
//  return global capacity, quantity and weight
//  TODO: weight according to fuel grad and temperature ?
//-----------------------------------------------------------------------
void CFuelSystem::GetTotalGas(float &cp,float &qt,float &wg)
{ std::vector<CFuelSubsystem*>::iterator sf;
  float cap = 0;
  float qty = 0;
  CFuelCell *cel;
  for (sf = fsub.begin(); sf != fsub.end(); sf++)
  { if (!(*sf)->IsType (SUBSYSTEM_FUEL_CELL)) continue;
    cel   = (CFuelCell*)(*sf);
    cap  += cel->GetCellCap();
    qty  += cel->GetCellQty();
  }
  cp  = cap;
  qt  = qty;
  wg  = qty * 6.02f;             // in lbs for 80 100 and 100LL
  return;
}
//-----------------------------------------------------------------------
//  JS NOTES:
//  Fuel system is updated in 3 passes
//  Pass 1: All subsystems are TimeSliced to update their own state (On
//          or OFF.  Any OFF valve or pump will prevent fuel to flow to
//          the next item.
//
//  Pass 2: For each engine, a list of contributing tanks is built.
//          A contributing tank is defined by:
//          a)  An open path from the engine to the tank
//          b)  A fuel quantity greater or equal than the requested quantity
//              Even if the requested qty is shared between several tanks
//              Selecting tank that can satify the whole request will 
//              simplify the algorithm. For instance assuming that engine
//              is requesting 1 part and tank A contains 2 part while B
//              contains only 0.25, then spreading the request would lead
//              to:
//              Request 0.5 part from tank A => OK
//              Request 0.5 part from Tank B => Only 0.25  available
//              Thus forcing a new round.  
//              With the actual algorithm, Tank B is not selected even if 0.25
//              part is remaining
//          Then, the fuel quantity requested by the engien is shared among
//          the contibuting tanks.
//
//  Pass 3: For each tank which is not full, a refill pass processes 
//          inter tank cross feeding.
//          Using the xfer Fuel flow rate, a request is made to upstream
//          tanks.  An upstream tank is defined by:
//          a)  An open path from the refilled tank.
//          b)  An actual fuel quantity greater or equal than the requested
//              quantity (for same reason as above).
//          Then the requested fuel quantity is shared among the suppliying
//          tanks if any.  
//          For a feeder tank F, the list 'Feed' is used to temporarily 
//          remember which tank as already receive fuel from F.  
//          This list is used to break infinite loop in circuitry.  
//          Such a loop exists in any
//          cross feed situation where tank A feeds tank B and B feeds A.
//          
// 
//-----------------------------------------------------------------------
//  PASS 1:
//  Update Fuel component state
//  NOTE: The compoent ON:OFF value is based on the state variable
//-----------------------------------------------------------------------
void CFuelSystem::UpdateComponents(float dT,U_INT frame)
{ std::vector<CFuelSubsystem*>::iterator i;
  for (i=fsub.begin(); i!=fsub.end(); i++)
  {  CFuelSubsystem *fs = (*i);
     //--- check for update need ---------------------
     if (!fs->NeedUpdate(frame)) continue;
     if (fs->NeedPoll()) fs->Poll();
     fs->TimeSlice(dT,frame);
  }
  return;
}
//-----------------------------------------------------------------------
//  Search contributing tanks to the requested fuel quantity
//-----------------------------------------------------------------------
float CFuelSystem::BurnFuelFor(float dT,CEngine *eng,char &ok)
{ float rqf = eng->GetReqFuelFlow();
  float rqt = (rqf / 3600.0f) * dT;
  CFuelTap *tap = eng->GetTapItem();
  //---Check for contributing tanks -------------
  if (rqf < FUEL_EMPTY_QTY)             return 0;
  ctank.reserve(16);
  if (Tr) TRACE("Burn Fuel cycle. Request=%.4f gal",rqt);
  tap->GetContributingTanks(this,rqt);
  if (0 == ctank.size())
      {ok = 0;  return 0;}
  if (Tr) TRACE("OK for fuel");
  //---- Remove burned fuel from tanks ----------
  float bnf = rqt / ctank.size();
  std::vector<CFuelCell*>::iterator ft;
  for (ft = ctank.begin(); ft != ctank.end(); ft++) (*ft)->BurnFuel(bnf);
  //---- Clear contributing list ----------------
  ctank.clear();
  return rqf;
}
//-----------------------------------------------------------------------
//  PASS 2:
//  Feed fuel to all engines
//-----------------------------------------------------------------------
void CFuelSystem::FeedEngines(float dT)
{ char ok = ENGINE_FUELED;
  std::vector<CEngine*>::const_iterator eg;
  for (eg=eng_mng->engn.begin(); eg!=eng_mng->engn.end(); eg++) 
  { ok = ENGINE_FUELED;
    CEngine *eng = (*eg);
    float ff = BurnFuelFor(dT,eng,ok);
    CFuelTap *tap = eng->GetTapItem();
    tap->SetFuelFlow(ff);
    eng->SetFuel(ok); 
  }
  //---Dont change anything --------------------
  return;
}
//-----------------------------------------------------------------------
//  Refill tank cell. 
//  Build a list of contributing tanks to refill this one
//-----------------------------------------------------------------------
void CFuelSystem::RefillCell(float dT,CFuelCell *tk)
{ float rq1 =  tk->GetRemainingCap();
  if (rq1 < FUEL_EMPTY_QTY)   return;
  float rq2 = tk->GetXXRate() * dT;
  float rqt = min(rq1,rq2);
  //-- Build a list of candidate for refilling this cell ----
  ctank.reserve(16);
  tk->GetRefillingTanks(this,tk,rqt);
  //-- Check for refuelling ---------------------------------
  U_INT nt = ctank.size();
  if (0 == nt)                return;
  //--- Refuel from candidates ------------------------------
  tk->AddFuel(rqt);
  float  rqf = rqt / nt;
  std::vector<CFuelCell*>::iterator it;
  for (it = ctank.begin(); it != ctank.end(); it++) (*it)->GiveFuelTo(tk, rqf);
  ctank.clear();
  return;
}
//-----------------------------------------------------------------------
//  PASS 3:
//  Tank refilling:  Each cell is refuelled form other cells
//                   according to the state of circuit.
//-----------------------------------------------------------------------
void CFuelSystem::RefillTanks(float dT)
{ std::vector<CFuelCell*>::iterator fs;
  //--- Refuel all cells ---------------------------
  for (fs = tanks.begin(); fs != tanks.end(); fs++)
  { CFuelCell *cel = (*fs);
    RefillCell(dT,cel);
  } 
  return;
}
//-----------------------------------------------------------------------
//  Time slice: Update fuel parameters 
//-----------------------------------------------------------------------
void CFuelSystem::Timeslice (float dT,U_INT FrNo)				// JSDEV*
{ UpdateComponents(dT,FrNo);
  FeedEngines(dT);
  RefillTanks(dT);
  if (globals->wfl) globals->wfl->Refresh();
}
//-----------------------------------------------------------------------
//  Stop fuel system when witching aircraft 
//-----------------------------------------------------------------------
void CFuelSystem::Stop()
{ 
  //-----Stop refuelling -----------------------------------
  std::vector<CFuelCell*>::iterator fs;
  for (fs = tanks.begin(); fs != tanks.end(); fs++) (*fs)->DumpFuel();
  //----Stop all engines -----------------------------------
  std::vector<CEngine*>::const_iterator eg;
  for (eg=eng_mng->engn.begin(); eg!=eng_mng->engn.end(); eg++)
    (*eg)->SetFuel(0);
  return;
}


//==============================================================================
// CDamageModel
//==============================================================================
CDamageModel::CDamageModel (void)
{
  period = 0.0f;
}

int CDamageModel::Read (SStream *stream, Tag tag)
{
 int rc = TAG_IGNORED;

  switch (tag) {
  case 'perd':
    // Damage repair period (hours)
    ReadFloat (&period, stream);
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // Tag was not processed by this object, it is unrecognized
    WARNINGLOG ("CDamageModel::Read : Unrecognized tag <%s>", TagToString(tag));
  }

  return rc;
}
//=========================================================================
// CElectricalSystem
//=========================================================================
CElectricalSystem::CElectricalSystem ()
{ 
  
  // Initialize control subsystem pointers
	sReg					= 0;
  pAils         = 0;
  pElvs         = 0;
  pRuds         = 0;
  pFlaps        = 0;
	pSter         = 0;
  aTrim         = 0;
  eTrim         = 0;
  rTrim         = 0;
  pwb           = 0;      // Brake system for keyboard
  pgr           = 0;      // Gear control for keyboard
  lastID[0]     = 0;
  lastHW[0]     = 0;
	//--------------------------------------------------------
	subs.reserve(100);
	sext.reserve(8);
	//--------------------------------------------------------
  
}
//-----------------------------------------------------------------------
//	Init the electrical system
//-----------------------------------------------------------------------
void CElectricalSystem::Init(char* ampFilename, CEngineManager *eng)
{    // Needed earlier for initialisation (autopilot)
	pEngineManager = eng;
	// Read from AMP file stream-----------------------------
  SStream s(this,"WORLD",ampFilename);
	//-- Add Fligth Plan subsystem ----------------------
	CFPlan	*fp = new CFPlan(mveh,1);
	subs.push_back (fp);
	fpln	= fp;
  //---Add robot ---------------------------------------
	d2r2	= new CRobot();							// Check list executer
	d2r2->SetParent(mveh);
  subs.push_back(d2r2);							// Add to amp list
	//---Add virtual pilot -------------------------------
	vpil	= new VPilot();							// Virtual pilot
	vpil->SetParent(mveh);
	subs.push_back(vpil);							// Add to amp list
}
//-----------------------------------------------------------------------
CElectricalSystem::~CElectricalSystem (void)
{ //--delete all amp subsystems -------------
  std::vector<CSubsystem*>::iterator i;
  for (i=subs.begin(); i!=subs.end(); i++) delete (*i);
	subs.clear();
  //--delete all dlls -----------------------
  std::vector<CDLLSubsystem*>::iterator d;
  for (d=sdll.begin(); d!=sdll.end(); d++) delete (*d);
	sdll.clear();
  //--delete all external subsystems --------
  std::vector<CSubsystem*>::iterator p;
  for (p=sext.begin(); p!=sext.end(); p++) delete (*p);
	sext.clear();
}
//-----------------------------------------------------------
//  Free DLL resources
//  JS NOTE : SAFE_DELETE will try to store 0 in the iterator!!!
//-----------------------------------------------------------
void CElectricalSystem::FreeDLLSubsystem (void)
{ 
#ifdef _DEBUG
  DEBUGLOG ("CSubsystem::FreeDLLSubsystem %d", sdll.size ());
#endif  
  // delete all pointers in dll_gauge
  std::vector <CDLLSubsystem *>::iterator idllS;
  for (idllS = sdll.begin (); idllS != sdll.end (); ++idllS) {
    SAFE_DELETE (*idllS);
  }
  sdll.clear (); // 
}
//-----------------------------------------------------------------------
//  Read all parameters of AMP file
//-----------------------------------------------------------------------
int CElectricalSystem::Read (SStream *stream, Tag tag)
{ int tr = globals->Trace.Has(TRACE_SUBSYSTEM);
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'subs':
    {
      char tag_string[64];
      ReadString (tag_string, 64, stream);
      Tag type = StringToTag (tag_string);

      CSubsystem *s = NULL;

      switch (type) {

      // Logic subsystems

      case SUBSYSTEM_DEPENDENT:
        //MEMORY_LEAK_MARKER ("electr1")
        s = new CDependent;
        //MEMORY_LEAK_MARKER ("electr1")
        break;
      case SUBSYSTEM_ANNOUNCEMENT:
        //MEMORY_LEAK_MARKER ("electr2")
        s = new CAnnouncement;
        //MEMORY_LEAK_MARKER ("electr2")
        break;
      case SUBSYSTEM_GENERIC_MONITOR:
        //MEMORY_LEAK_MARKER ("electr3")
        s = new CGenericMonitor;
        //MEMORY_LEAK_MARKER ("electr3")
        break;
      case SUBSYSTEM_GENERIC_INDICATOR:
        //MEMORY_LEAK_MARKER ("electr4")
        s = new CGenericIndicator;
        //MEMORY_LEAK_MARKER ("electr4")
        break;
      case SUBSYSTEM_TIME_DELAY:
        //MEMORY_LEAK_MARKER ("electr5")
        s = new CTimeDelay;
        //MEMORY_LEAK_MARKER ("electr5")
        break;
      case SUBSYSTEM_KEYED_SYSTEM:
        //MEMORY_LEAK_MARKER ("electr6")
        s = new CKeyedSystem;
        //MEMORY_LEAK_MARKER ("electr6")
        break;
      case SUBSYSTEM_CONTACTOR:
        //MEMORY_LEAK_MARKER ("electr7")
        s = new CContactor;
        //MEMORY_LEAK_MARKER ("electr7")
        break;
      case SUBSYSTEM_FLASHER:
        //MEMORY_LEAK_MARKER ("electr8")
        s = new CFlasher;
        //MEMORY_LEAK_MARKER ("electr8")
        break;

      // Electrical subsystems

      case SUBSYSTEM_BATTERY:
        //MEMORY_LEAK_MARKER ("electr9")
        s = new CBattery;
        //MEMORY_LEAK_MARKER ("electr9")
        break;
      case SUBSYSTEM_ALTERNATOR:
        //MEMORY_LEAK_MARKER ("electr10")
        s = new CAlternator;
        //MEMORY_LEAK_MARKER ("electr10")
        break;
      case SUBSYSTEM_ANNUNCIATOR_LIGHT:
        //MEMORY_LEAK_MARKER ("electr11")
        s = new CAnnunciatorLight;
        //MEMORY_LEAK_MARKER ("electr11")
        break;
      case SUBSYSTEM_EXTERIOR_LIGHT:
        //MEMORY_LEAK_MARKER ("electr12")
        s = new CExteriorLight;
        //MEMORY_LEAK_MARKER ("electr12")
        break;
      case SUBSYSTEM_VOLTMETER:
        //MEMORY_LEAK_MARKER ("electr13")
        s = new CVoltmeter;
        //MEMORY_LEAK_MARKER ("electr13")
        break;
      case SUBSYSTEM_AMMETER:
        //MEMORY_LEAK_MARKER ("electr14")
        s = new CAmmeter;
        //MEMORY_LEAK_MARKER ("electr15")
        break;
      case SUBSYSTEM_GENERATOR:
        //MEMORY_LEAK_MARKER ("electr16")
        s = new CGenerator;
        //MEMORY_LEAK_MARKER ("electr16")
        break;
      case SUBSYSTEM_FREQUENCY_METER:
        //MEMORY_LEAK_MARKER ("electr17")
        s = new CFrequencyMeter;
        //MEMORY_LEAK_MARKER ("electr17")
        break;
      case SUBSYSTEM_INVERTER:
        //MEMORY_LEAK_MARKER ("electr18")
        s = new CInverter;
        //MEMORY_LEAK_MARKER ("electr18")
        break;
      case SUBSYSTEM_PITOT_STATIC_SWITCH:
        //MEMORY_LEAK_MARKER ("electr19")
        s = new CPitotStaticSwitch;
        //MEMORY_LEAK_MARKER ("electr19")
        break;
      case SUBSYSTEM_PITOT_HEAT_SWITCH:
        //MEMORY_LEAK_MARKER ("electr20")
        s = new CPitotHeatSwitch;
        //MEMORY_LEAK_MARKER ("electr20")
        break;
      case SUBSYSTEM_STATIC_SOURCE_SWITCH:
        //MEMORY_LEAK_MARKER ("electr21")
        s = new CStaticSourceSwitch;
        //MEMORY_LEAK_MARKER ("electr21")
        break;
      case SUBSYSTEM_LIGHT_SETTING_STATE:
        //MEMORY_LEAK_MARKER ("electr22")
        s = new CLightSettingState;
        //MEMORY_LEAK_MARKER ("electr22")
        break;
      case SUBSYSTEM_SWITCH_SET:
        //MEMORY_LEAK_MARKER ("electr23")
        s = new CSwitchSet;
        //MEMORY_LEAK_MARKER ("electr23")
        break;
      case SUBSYSTEM_ROTARY_IGNITION_SWITCH:
        //MEMORY_LEAK_MARKER ("electr24")
        s = new CRotaryIgnitionSwitch;
        //MEMORY_LEAK_MARKER ("electr24")
        break;

      //
      // Gauge subsystems
      //
      case SUBSYSTEM_ALTIMETER:
        //MEMORY_LEAK_MARKER ("electr25")
        s = new CAltimeter;
        //MEMORY_LEAK_MARKER ("electr25")
        break;
      case SUBSYSTEM_VERTICAL_SPEED:
        //MEMORY_LEAK_MARKER ("electr26")
        s = new CVerticalSpeedIndicator;
        //MEMORY_LEAK_MARKER ("electr26")
        break;
      case SUBSYSTEM_AIRSPEED:
        //MEMORY_LEAK_MARKER ("electr27")
        s = new CAirspeedIndicator;
        //MEMORY_LEAK_MARKER ("electr27")
        break;
      case SUBSYSTEM_PNEUMATIC_PUMP:
        // an engine pointer is linked to this pump below
        //MEMORY_LEAK_MARKER ("electr28")
        s = new CPneumaticPump ();
        //MEMORY_LEAK_MARKER ("electr28")
        break;
      case SUBSYSTEM_ATTITUDE:
        //MEMORY_LEAK_MARKER ("electr29")
        s = new CAttitudeIndicator;
        //MEMORY_LEAK_MARKER ("electr29")
        break;
      case SUBSYSTEM_DIRECTIONAL_GYRO:
        //MEMORY_LEAK_MARKER ("electr30")
        s = new CDirectionalGyro;
        //MEMORY_LEAK_MARKER ("electr30")
        break;
      case SUBSYSTEM_VACUUM_INDICATOR:
        //MEMORY_LEAK_MARKER ("electr31")
        s = new CVacuumIndicator;
        //MEMORY_LEAK_MARKER ("electr31")
        break;
      case SUBSYSTEM_TURN_COORDINATOR:
        //MEMORY_LEAK_MARKER ("electr32")
        s = new CTurnCoordinator;
        //MEMORY_LEAK_MARKER ("electr32")
        break;
      case SUBSYSTEM_MAGNETIC_COMPASS:
        //MEMORY_LEAK_MARKER ("electr33")
        s = new CMagneticCompass;
        //MEMORY_LEAK_MARKER ("electr33")
        break;
      case SUBSYSTEM_NAVIGATION:
        //MEMORY_LEAK_MARKER ("electr34")
        s = new CNavigation;
        //MEMORY_LEAK_MARKER ("electr34")
        break;
      case SUBSYSTEM_DIGITAL_CLOCK_OAT:
        //MEMORY_LEAK_MARKER ("electr35")
        s = new CDigitalClockOAT;
        //MEMORY_LEAK_MARKER ("electr35")
        break;
      case SUBSYSTEM_KAP140_PANEL:
        //MEMORY_LEAK_MARKER ("electr36")
        s = new CKAP140Panel;
        //MEMORY_LEAK_MARKER ("electr36")
        break;
      case SUBSYSTEM_MARKER_PANEL:
        //MEMORY_LEAK_MARKER ("electr37")
        s = new CMarkerPanel;
        //MEMORY_LEAK_MARKER ("electr37")
        break;
      case SUBSYSTEM_INDICATOR_SET:
        //MEMORY_LEAK_MARKER ("electr38")
        s = new CIndicatorSet;
        //MEMORY_LEAK_MARKER ("electr38")
        break;
      case SUBSYSTEM_ANNUNCIATOR:
        //MEMORY_LEAK_MARKER ("electr39")
        s = new CAnnunciatorSubsystem;
        //MEMORY_LEAK_MARKER ("electr39")
        break;
      case SUBSYSTEM_SUCTION_GAUGE:
        //MEMORY_LEAK_MARKER ("electr40")
        s = new CSuctionGaugeSubsystem;
        //MEMORY_LEAK_MARKER ("electr40")
        break;
      case SUBSYSTEM_FLAP_GAUGE:
        //MEMORY_LEAK_MARKER ("electr41")
        s = new CFlapGauge;
        //MEMORY_LEAK_MARKER ("electr41")
        break;
      case SUBSYSTEM_SYNCHROPHASER:
        //MEMORY_LEAK_MARKER ("electr42")
        s = new CSynchrophaser;
        //MEMORY_LEAK_MARKER ("electr42")
        break;
      case SUBSYSTEM_EMERGENCY_LOCATOR:
        //MEMORY_LEAK_MARKER ("electr43")
        s = new CEmergencyLocator;
        //MEMORY_LEAK_MARKER ("electr43")
        break;

      // Avionics subsystems
      case SUBSYSTEM_DME_PANEL:
        s = new CDMEPanel;
        break;
      case SUBSYSTEM_DME_PANEL_1:
        s = new CDMEPanel2;
        break;
      case SUBSYSTEM_COLLINS_PFD:
        s = new CCollinsPFD;
        break;
      case SUBSYSTEM_COLLINS_ND:
        s = new CCollinsND;
        break;
      case SUBSYSTEM_COLLINS_AD:
        s = new CCollinsAD;
        break;
      case SUBSYSTEM_COLLINS_MND:
        s = new CCollinsMND;
        break;
      case SUBSYSTEM_COLLINS_RTU:
        s = new CCollinsRTU;
        break;
      case SUBSYSTEM_PILATUS_PFD:
        s = new CPilatusPFD;
        break;
      case SUBSYSTEM_VIRTUAL_GPS:
        s = new CVirtualGPS;
        break;
      case SUBSYSTEM_TCAS_PANEL:
        s = new CTCASPanel;
        break;
      case SUBSYSTEM_UNIVERSAL_FMS:
        s = new CFMSys;
        break;
      case SUBSYSTEM_WEATHER_RADAR:
        s = new CWeatherRadar;
        break;

      // Autopilot subsystems

      case SUBSYSTEM_AUTOPILOT_MASTER_PANEL:
        //MEMORY_LEAK_MARKER ("electr44")
        s = new CAutopilotMasterPanel;
        //MEMORY_LEAK_MARKER ("electr44")
        break;
      case SUBSYSTEM_AUTOPILOT_PANEL:
        //MEMORY_LEAK_MARKER ("electr45")
        s = new AutoPilot;
        //MEMORY_LEAK_MARKER ("electr45")
        break;
			case SUBSYSTEM_SPEED_REGULATOR:
				s = new CSpeedRegulator;
				sReg	= (CSpeedRegulator*)s;
				break;
      case SUBSYSTEM_K150_PANEL:
        //MEMORY_LEAK_MARKER ("electr46")
        s = new CKAP150Panel;
        //MEMORY_LEAK_MARKER ("electr46")
        break;
      case SUBSYSTEM_K200_PANEL:
        s = new CKAP200Panel;
        break;
      case SUBSYSTEM_AFCS65_PANEL:
        s = new CAFCS65Panel;
        break;
      case SUBSYSTEM_AFCS85_PANEL:
        s = new CAFCS85Panel;
        break;

      // Engine subsystems

      case SUBSYSTEM_STARTER_CONTROL:
        //MEMORY_LEAK_MARKER ("electr47")
        s = new CStarterControl;
        //MEMORY_LEAK_MARKER ("electr47")
        break;
      case SUBSYSTEM_MAGNETO_CONTROL:
        //MEMORY_LEAK_MARKER ("electr48")
        s = new CMagnetoControl;
        //MEMORY_LEAK_MARKER ("electr48")
        break;
      case SUBSYSTEM_IGNITER_CONTROL:
        //MEMORY_LEAK_MARKER ("electr49")
        s = new CIgniterControl;
        //MEMORY_LEAK_MARKER ("electr49")
        break;
      case SUBSYSTEM_TACHOMETER:              // 
        //MEMORY_LEAK_MARKER ("electr50")
        s = new CTachometer; //
        //MEMORY_LEAK_MARKER ("electr50")
        break;                                //
      case SUBSYSTEM_OIL_TEMPERATURE:
        //MEMORY_LEAK_MARKER ("electr51")
        s = new COilTemperature;
        //MEMORY_LEAK_MARKER ("electr51")
        break;
      case SUBSYSTEM_OIL_PRESSURE:
        //MEMORY_LEAK_MARKER ("electr52")
        s = new COilPressure;
        //MEMORY_LEAK_MARKER ("electr52")
        break;
      case SUBSYSTEM_EXHAUST_GAS_TEMPERATURE:
        //MEMORY_LEAK_MARKER ("electr53")
        s = new CExhaustGasTemperature;
        //MEMORY_LEAK_MARKER ("electr53")
        break;
      case SUBSYSTEM_MANIFOLD_PRESSURE:
        //MEMORY_LEAK_MARKER ("electr54")
        s = new CManifoldPressure;
        //MEMORY_LEAK_MARKER ("electr54")
        break;
      case SUBSYSTEM_HOBBS_METER:
        //MEMORY_LEAK_MARKER ("electr55")
        s = new CHobbsMeter;
        //MEMORY_LEAK_MARKER ("electr55")
        break;
      case SUBSYSTEM_TACHOMETER_TIMER:
        //MEMORY_LEAK_MARKER ("electr56")
        s = new CTachometerTimer;
        //MEMORY_LEAK_MARKER ("electr56")
        break;
      case SUBSYSTEM_ENGINE_GAUGE:
        //MEMORY_LEAK_MARKER ("electr57")
        s = new CEngineGauge;
        //MEMORY_LEAK_MARKER ("electr57")
        break;
      case SUBSYSTEM_ENGINE_STATUS:
        //MEMORY_LEAK_MARKER ("electr58")
        s = new CEngineStatus;
        //MEMORY_LEAK_MARKER ("electr58")
        break;
      case SUBSYSTEM_FUEL_GAUGE:
        //MEMORY_LEAK_MARKER ("electr59")
        s = new CFuelGaugeSubsystem;
        //MEMORY_LEAK_MARKER ("electr59")
        break;
      case SUBSYSTEM_FUEL_FLOW_GAUGE:
        //MEMORY_LEAK_MARKER ("electr60")
        s = new CFuelFlowSubsystem;
        //MEMORY_LEAK_MARKER ("electr60")
        break;
      case SUBSYSTEM_FUEL_PRESSURE_GAUGE:
        //MEMORY_LEAK_MARKER ("electr61")
        s = new CFuelPressureSubsystem;
        //MEMORY_LEAK_MARKER ("electr61")
        break;
      case SUBSYSTEM_N1_TACHOMETER:
        //MEMORY_LEAK_MARKER ("electr62")
        s = new CN1Tachometer;
        //MEMORY_LEAK_MARKER ("electr62")
        break;
      case SUBSYSTEM_N2_TACHOMETER:
        s = new CN2Tachometer;
        break;
      case SUBSYSTEM_TORQUE_METER:
        //MEMORY_LEAK_MARKER ("electr63")
        s = new CTorqueMeter;
        //MEMORY_LEAK_MARKER ("electr63")
        break;
      case SUBSYSTEM_CYLINDER_HEAD_TEMP:
        //MEMORY_LEAK_MARKER ("electr64")
        s = new CCylinderHeadTemp;
        //MEMORY_LEAK_MARKER ("electr64")
        break;
      case SUBSYSTEM_INTERTURBINE_TEMP:
        s = new CInterTurbineTemp;
        break;
      case SUBSYSTEM_ENGINE_MONITOR:
        //MEMORY_LEAK_MARKER ("electr65")
        s = new CEngineMonitor;
        //MEMORY_LEAK_MARKER ("electr65")
        break;
      case SUBSYSTEM_STARTER_IGNITER:
        //MEMORY_LEAK_MARKER ("electr66")
        s = new CStarterIgniter;
        //MEMORY_LEAK_MARKER ("electr66")
        break;
      case SUBSYSTEM_FIRE_SYSTEM:
        s = new CFireSystem;
        break;
      case SUBSYSTEM_FUEL_IMBALANCE:
        s = new CFuelImbalance;
        break;
      case SUBSYSTEM_FUEL_PUMP_CONTROL:
        //MEMORY_LEAK_MARKER ("electr67")
        s = new CFuelPumpControl;
        //MEMORY_LEAK_MARKER ("electr67")
        break;
      case SUBSYSTEM_CROSSFEED_CONTROL:
        //MEMORY_LEAK_MARKER ("electr68")
        s = new CCrossfeedControl;
        //MEMORY_LEAK_MARKER ("electr68")
        break;

      // Control subsystems

      case SUBSYSTEM_THROTTLE_CONTROL:
        //MEMORY_LEAK_MARKER ("electr69")
        s = new CThrottleControl;
        //MEMORY_LEAK_MARKER ("electr69")
        break;
      case SUBSYSTEM_MIXTURE_CONTROL:
        //MEMORY_LEAK_MARKER ("electr70")
        s = new CMixtureControl;
        //MEMORY_LEAK_MARKER ("electr70")
        break;
      case SUBSYSTEM_STEERING_CONTROL:
        //MEMORY_LEAK_MARKER ("electr71")
        s = new  CSteeringControl;
				pSter = (CSteeringControl*)s;
        //MEMORY_LEAK_MARKER ("electr71")
        break;
      case SUBSYSTEM_FLAP_CONTROL:
        //MEMORY_LEAK_MARKER ("electr72")
        s = new CFlapControl;
        //MEMORY_LEAK_MARKER ("electr72")
        pFlaps = (CFlapControl *)s;
        break;
      case SUBSYSTEM_ELEVATOR_CONTROL:
        //MEMORY_LEAK_MARKER ("electr73")
        s = new CElevatorControl;
        //MEMORY_LEAK_MARKER ("electr73")
        pElvs = (CElevatorControl *)s;
        break;
      case SUBSYSTEM_AILERON_CONTROL:
        //MEMORY_LEAK_MARKER ("electr74")
        s = new CAileronControl;
        //MEMORY_LEAK_MARKER ("electr74")
        pAils = (CAileronControl *)s;
        break;
      case SUBSYSTEM_RUDDER_CONTROL:
        //MEMORY_LEAK_MARKER ("electr75")
        s = new CRudderControl;
        //MEMORY_LEAK_MARKER ("electr75")
        pRuds = (CRudderControl *)s;
        break;
      case SUBSYSTEM_ELEVATOR_TRIM_CONTROL:
        //MEMORY_LEAK_MARKER ("electr76")
        s = new CElevatorTrimControl;
        //MEMORY_LEAK_MARKER ("electr76")
        eTrim = (CElevatorTrimControl *) s;
        break;
      case SUBSYSTEM_AILERON_TRIM_CONTROL:
        //MEMORY_LEAK_MARKER ("electr77")
        s = new CAileronTrimControl;
        //MEMORY_LEAK_MARKER ("electr77")
        aTrim = (CAileronTrimControl *) s;
        break;
      case SUBSYSTEM_RUDDER_TRIM_CONTROL:
        //MEMORY_LEAK_MARKER ("electr78")
        s = new CRudderTrimControl;
        //MEMORY_LEAK_MARKER ("electr78")
        rTrim = (CRudderTrimControl *) s;
        break;
      case SUBSYSTEM_IDLE_CONTROL:
        //MEMORY_LEAK_MARKER ("electr*1")
        s = new CIdleControl;
        //MEMORY_LEAK_MARKER ("electr*1")
        break;
      case SUBSYSTEM_PROPELLER_CONTROL:
        //MEMORY_LEAK_MARKER ("electr*2")
        s = new CPropellerControl;
        //MEMORY_LEAK_MARKER ("electr*2")
        break;
      case SUBSYSTEM_BRAKE_CONTROL: // 
        //MEMORY_LEAK_MARKER ("electr79")
        s   = new CBrakeControl;
        //MEMORY_LEAK_MARKER ("electr79")
        pwb = (CBrakeControl*)s;
        break;
      case SUBSYSTEM_GEAR_CONTROL:
        //MEMORY_LEAK_MARKER ("electr80")
        s   = new CGearControl;
        //MEMORY_LEAK_MARKER ("electr80")
        pgr = (CGearControl*)s;
        break;
      case SUBSYSTEM_PRIME_CONTROL:
        s = new CPrimeControl;
        break;
      case SUBSYSTEM_HIGH_LIFT_CONTROL:
        s = new CHighLiftControl;
        break;
      case SUBSYSTEM_COWL_FLAP_CONTROL:
        //MEMORY_LEAK_MARKER ("electr81")
        s = new CCowlFlapControl;
        //MEMORY_LEAK_MARKER ("electr81")
        break;
      case SUBSYSTEM_AIR_BRAKE_CONTROL:
        s = new CAirBrakeControl;
        break;
      case SUBSYSTEM_THRUST_REVERSE_CONTROL:
        s = new CThrustReverseControl;
        break;
      case SUBSYSTEM_AUDIO_PANEL:
        //MEMORY_LEAK_MARKER ("electr*3")
        s = new CAudioPanel;
        //MEMORY_LEAK_MARKER ("electr*3")
        break;
      case SUBSYSTEM_ENGINE_ANTI_ICE:
        s = new CEngineAntiIce;
        break;
      case SUBSYSTEM_SURFACE_DEICE:
        s = new CSurfaceDeIce;
        break;
      case SUBSYSTEM_PROPELLER_DEICE:
        s = new CPropellerDeIce;
        break;
      case SUBSYSTEM_WING_DEICE:
        s = new CWingDeIce;
        break;
      case SUBSYSTEM_TEMP_CONTROL:
        s = new CTempControl;
        break;
      case SUBSYSTEM_WIPER_CONTROL:
        s = new CWiperControl;
        break;
      case SUBSYSTEM_PERFORMANCE_RESERVE:
        s = new CPerformanceReserve;
        break;
      case SUBSYSTEM_RUDDER_BIAS:
        s = new CRudderBias;
        break;
      case SUBSYSTEM_OXYGEN:
        s = new COxygen;
        break;

      // Helicopter subsystems
      case SUBSYSTEM_ROTOR:
        s = new CRotorSubsystem;
        break;
      case SUBSYSTEM_407_DIGITAL_INDICATORS:
        s = new C407DigitalIndicators;
        break;
      case SUBSYSTEM_HOOK:
        s = new CHookSubsystem;
        break;
      case SUBSYSTEM_THRUST_PITCH_CONTROL:
        s = new CThrustPitchControl;
        break;
      case SUBSYSTEM_LON_CYCLIC_CONTROL:
        s = new CLonCyclicControl;
        break;
      case SUBSYSTEM_LAT_CYCLIC_CONTROL:
        s = new CLatCyclicControl;
        break;
      case SUBSYSTEM_TAIL_ROTOR_CONTROL:
        s = new CTailRotorControl;
        break;
      case SUBSYSTEM_COLLECTIVE_CONTROL:
        s = new CCollectiveControl;
        break;
      case SUBSYSTEM_LON_CYCLIC_TRIM_CONTROL:
        s = new CLonCyclicTrimControl;
        break;
      case SUBSYSTEM_LAT_CYCLIC_TRIM_CONTROL:
        s = new CLatCyclicTrimControl;
        break;
      case SUBSYSTEM_TAIL_ROTOR_TRIM_CONTROL:
        s = new CTailRotorTrimControl;
        break;
      case SUBSYSTEM_COLLECTIVE_TRIM_CONTROL:
        s = new CCollectiveTrimControl;
        break;

      // Radio subsystems
      case SUBSYSTEM_NAV_RADIO:
        //MEMORY_LEAK_MARKER ("electr*4")
        s = new CNavRadio;
        //MEMORY_LEAK_MARKER ("electr*4")
        break;
      case SUBSYSTEM_RNAV_RADIO:
        //MEMORY_LEAK_MARKER ("electr*5")
        s = new CRnavRadio;
        //MEMORY_LEAK_MARKER ("electr*5")
        break;
      case SUBSYSTEM_COMM_RADIO:
        //MEMORY_LEAK_MARKER ("electr*6")
        s = new CComRadio;
        //MEMORY_LEAK_MARKER ("electr*6")
        break;
      case SUBSYSTEM_HF_COMM_RADIO:
        s = new CHFCommRadio;
        break;
      case SUBSYSTEM_TRANSPONDER_RADIO:
        //MEMORY_LEAK_MARKER ("electr82")
        s = new CTransponderRadio;
        //MEMORY_LEAK_MARKER ("electr82")
        break;
      case SUBSYSTEM_ADF_RADIO:
        //MEMORY_LEAK_MARKER ("electr83")
        s = new CKR87radio;
        //MEMORY_LEAK_MARKER ("electr83")
        break;
      case SUBSYSTEM_GPS_BX_KLN89:
        //MEMORY_LEAK_MARKER ("electr84")
        s = new CK89gps(mveh,0);
        //MEMORY_LEAK_MARKER ("electr84")
        break;
      case SUBSYSTEM_KX155_RADIO:
        //MEMORY_LEAK_MARKER ("electr*7")
        s = new CK155radio();
        //MEMORY_LEAK_MARKER ("electr*7")
        break;
      case SUBSYSTEM_KT76_RADIO:
        //MEMORY_LEAK_MARKER ("electr85")
        s = new CKT76radio;
        //MEMORY_LEAK_MARKER ("electr85")
        break;
      case SUBSYSTEM_AUDIO_PANEL_RADIO:
        //MEMORY_LEAK_MARKER ("electr*8")
        s = new CAudioPanelRadio;
        //MEMORY_LEAK_MARKER ("electr*8")
        break;
      case SUBSYSTEM_KAP140_RADIO:
        //MEMORY_LEAK_MARKER ("electr*9")
        s = new CBKKAP140Radio;
        //MEMORY_LEAK_MARKER ("electr*9")
        break;

      // Vehicle state subsystems

      case SUBSYSTEM_HISTORY:
        s = new CHistory;
        break;
      case SUBSYSTEM_ON_GROUND_MONITOR:
        s = new COnGroundMonitor;
        break;
      case SUBSYSTEM_ALTITUDE_MONITOR:
        s = new CAltitudeMonitor;
        break;
      case SUBSYSTEM_SPEED_MONITOR:
        s = new CSpeedMonitor;
        break;
      case SUBSYSTEM_STALL_WARNING:
        s = new CStallWarning;
        break;
      case SUBSYSTEM_FAST_SLOW_METER:
        s = new CFastSlowMeter;
        break;
      case SUBSYSTEM_PRESSURIZATION:
        s = new CPressurization;
        break;
      case SUBSYSTEM_ACCELEROMETER:
        s = new CAccelerometer;
        break;
      case SUBSYSTEM_GEAR_LIGHT:
        s = new CGearLight;
        break;
      case SUBSYSTEM_STALL_IDENT:
        s = new CStallIdent;
        break;
      case SUBSYSTEM_TEMP_AIRSPEED_DISPLAY:
        s = new CTempAirspeedDisplay;
        break;
      case SUBSYSTEM_GEAR_WARNING:
        s = new CGearWarning;
        break;

      // Fly! V1 subsystems (deprecated)

      case SUBSYSTEM_THROTTLE_CONTROL_V1:
        s = new CThrottleControlV1;
        break;
      case SUBSYSTEM_MIXTURE_CONTROL_V1:
        s = new CMixtureControlV1;
        break;
      case SUBSYSTEM_PROPELLER_CONTROL_V1:
        s = new CPropellerControlV1;
        break;
      case SUBSYSTEM_PRIME_CONTROL_V1:
        s = new CPrimeControlV1;
        break;
      case SUBSYSTEM_ENGINE_ANTI_ICE_V1:
        s = new CEngineAntiIceV1;
        break;
      case SUBSYSTEM_IDLE_CONTROL_V1:
        s = new CIdleControlV1;
        break;

      // Fuel subsystems

      case SUBSYSTEM_FUEL_MATH:
        // Skip 'fMat' subsystem, this has been made obsolete by
        //   fuel subsystem objects in the CFuelManager class but
        //   older aircraft AMP files may still have a subystem defined
        SkipObject (stream);
        rc = TAG_READ;
        break;

      // Simulation model subsystems
      case SUBSYSTEM_AIR_MODEL:
        //MEMORY_LEAK_MARKER ("electr*10")
        s = new CAirModel;
        //MEMORY_LEAK_MARKER ("electr*10")
        break;
      case SUBSYSTEM_MIXER_MODEL:
        //MEMORY_LEAK_MARKER ("electr*11")
        s = new CMixerModel;
        //MEMORY_LEAK_MARKER ("electr*11")
        break;
      case SUBSYSTEM_WHEEL_MODEL:
        //MEMORY_LEAK_MARKER ("electr*12")
        s = new CWheelModel;
        //MEMORY_LEAK_MARKER ("electr*12")
        break;
      case SUBSYSTEM_ENGINE_MODEL:
        s = new CEngineProxy;
        break;
      case SUBSYSTEM_WING_MODEL:
        //MEMORY_LEAK_MARKER ("electr*13")
        s = new CWingModel;
        //MEMORY_LEAK_MARKER ("electr*13")
        break;
      case SUBSYSTEM_CIRCULAR_TIMER:
        //MEMORY_LEAK_MARKER ("electr*14")
        s = new CCircularTimer;
        //MEMORY_LEAK_MARKER ("electr*14")
        break;
      case SUBSYSTEM_SMOKE:
         //MEMORY_LEAK_MARKER ("smoke_sys");
         s = new CSubsystemSmoke(mveh);
         AddExternal(s,stream);
         return TAG_READ;
         //MEMORY_LEAK_MARKER ("smoke_sys");

      default:
        { // Not identified: Create a DLL gauge
          char s[8];
          TagToString (s, type);
          MEMORY_LEAK_MARKER ("dllsub")
          CDLLSubsystem *ds = new CDLLSubsystem; 
          MEMORY_LEAK_MARKER ("dllsub")
          ds->SetSignature (static_cast <const long> (type));
          ReadFrom (ds, stream);
          sdll.push_back (ds);
#ifdef _DEBUG
            TRACE ("DLL SUBSYSTEM <%s>", s);
#endif
            return TAG_READ;
        }
      } // switch(type)
      //----------------------------------------------------------------------
      // If a subsystem was instantiated, then load it and add it to the list
      //  NOTE: All subsystems that are to be queued to amp must come here
      //----------------------------------------------------------------------
      if (s != NULL) {
        s->SetParent(mveh);
        ReadFrom (s, stream);
        subs.push_back (s);
        TagToString(lastID,s->GetUnId());
        TagToString(lastHW,s->GetType());
        return TAG_READ;
      }

      break;

    } // case 'subs'
  } // switch(tag)

  if (rc != TAG_READ) {
    // Tag was not processed by this object, it is unrecognized
    CStreamFile* sf = (CStreamFile*)stream->stream;
    int line = sf->GetLine();
    gtfo ("AMP: subsystem <%s> unknown near line %d. Last OK is %s-%s", TagToString(tag),line,lastHW,lastID);
  }
  return rc;
}
//---------------------------------------------------------------------------
//  Add an external subsystem
//---------------------------------------------------------------------------
void CElectricalSystem::AddExternal(CSubsystem *sy,SStream *st)
{ sy->SetParent(mveh);
  if (st) ReadFrom (sy, st);
  sext.push_back (sy);
  return;
}
//---------------------------------------------------------------------------
//  All is read
//---------------------------------------------------------------------------
void CElectricalSystem::ReadFinished (void)
{	if (pRuds) pRuds->InitCTLR(pSter);
	mveh->whl.SetSteerData(pRuds);
	return;
}
//-----------------------------------------------------------------------------
void CElectricalSystem::Write (SStream *stream)
{
}
//-----------------------------------------------------------------------------
//  Locate subsystem by Tag
//-----------------------------------------------------------------------------
CSubsystem* CElectricalSystem::GetFirstSubsystemType (Tag tag)
{ CSubsystem* rc = NULL;
  std::vector<CSubsystem*>::iterator i;
  for (i=subs.begin(); i!=subs.end() && (rc == NULL); i++) {
    if ((*i)->IsType (tag)) rc = *i;
  }

  return rc;
}
//---------------------------------------------------------------------------
//  Draw  external subsystems
//---------------------------------------------------------------------------
void CElectricalSystem::DrawExternal()
{ std::vector<CSubsystem*>::iterator k;
	for (k=sext.begin(); k!=sext.end(); k++) (*k)->Draw();
  return;
}

//-----------------------------------------------------------------------------
//	JSDEV*:  TimeSlice all electrical subsystems
//-----------------------------------------------------------------------------
void CElectricalSystem::Timeslice(float dT,U_INT FrNo)
{	
  std::vector<CSubsystem*>::iterator its;
	for (its=subs.begin(); its!=subs.end(); its++) 
	{	CSubsystem *sub = *its;
		if (!sub->NeedUpdate(FrNo))		continue;
		sub->TimeSlice (dT,FrNo);
	}

  //-- sdk: DLL Subsystems -----------------------------------------------------
  if (globals->plugins_num) {
    std::vector<CDLLSubsystem *>::iterator dll_iter;
    for (dll_iter = sdll.begin(); dll_iter!=sdll.end(); dll_iter++) {
      CDLLSubsystem *ds = *dll_iter;
      if (!ds->NeedUpdate(FrNo))		continue;
      ds->TimeSlice (dT, FrNo);
    }
  }

	return;
}


void CElectricalSystem::Print (FILE *f)
{ /*
  fprintf (f, "Electrical System:\n\n");

  // Print each electrical subsystem
  std::vector<CSubsystem*>::iterator i;
  for (i=subs.begin(); i!=subs.end(); i++) {
    CSubsystem *s = *i;
    fprintf (f, "---------------------------------------------------------------------\n");
    s->Print (f);
    fprintf (f, "\n");
  }
	*/
}

//===============================================================================
/*
 * CPitotStaticSystem
 */
//===============================================================================
CPitotStaticSystem::CPitotStaticSystem ()
{ ports.clear ();
  _total_pressure_node = 0.0;
}
//--------------------------------------------------------------------
//	Read the file
//--------------------------------------------------------------------
void CPitotStaticSystem::Init(char* pssFilename)
{	  SStream s(this,"WORLD",pssFilename);
		return;		}
//--------------------------------------------------------------------
//	Destroy it
//---------------------------------------------------------------------
CPitotStaticSystem::~CPitotStaticSystem (void)
{ std::vector<CPitotStaticPort*>::iterator i;
  for (i=ports.begin(); i!=ports.end(); i++) delete *i;
	ports.clear();
}

int CPitotStaticSystem::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'iceT':
    // Ice condition duration
    ReadFloat (&iceT, stream);
    rc = TAG_READ;
    break;

  case 'port':
    // Port sub-object instance
    {
      CPitotStaticPort *port = new CPitotStaticPort(mveh);
      ReadFrom (port, stream);
      ports.push_back (port);
      rc = TAG_READ;
    }
    break;
  }

  if (rc != TAG_READ) {
    // Tag was not processed by this object, it is unrecognized
    WARNINGLOG ("CPitotStaticSystem::Read : Unrecognized tag <%s>", TagToString(tag));
  }

  return rc;
}


void CPitotStaticSystem::ReadFinished (void)
{

}


void CPitotStaticSystem::Write (SStream *stream)
{

}

void CPitotStaticSystem::Timeslice (float dT)
{
  double rho  = globals->atm->GetDensitySlugsFt3 ();          ///< slugs.ft3
  double pr   = globals->atm->GetPressureSlugsFtSec ();       ///< pfs
  const SVector *vt = (const SVector*) (mveh->GetAirspeed ());    ///< m/s LH
  const SVector v = VectorMultiply (*vt,TC_FEET_PER_METER);       ///< m/s to f/s
  
  _total_pressure_node = 0.0;
  std::vector<CPitotStaticPort*>::iterator i;
  for (i=ports.begin(); i!=ports.end(); i++) {
    _total_pressure_node += (*i)->ComputePitotPort (rho, pr, v); // LH
  }

//
//#ifdef _DEBUG	
//{	FILE *fp_debug;
//	if(!(fp_debug = fopen("__DDEBUG_.txt", "a")) == NULL)
//	{
//    double vit = 0.0, vv = 0.0;
//    globals->sit->user->GetIAS (vit);
//    vv = FpsToKt (static_cast<float> (MetresToFeet (vit)));
//    fprintf(fp_debug, "CPitotStaticSystem::Timeslice %d %f\n",
//      ports.size (),
//      _total_pressure_node,
//      vv);
//		fclose(fp_debug); 
//}	}
//#endif
}

void CPitotStaticSystem::Debug (void)
{
  ;
}



//=================================================================================
// Variable Loadouts
//=================================================================================
CVariableLoadouts::CVariableLoadouts ()
{ }
//----------------------------------------------------------------------------------
//  Load the file
//---------------------------------------------------------------------------------
void CVariableLoadouts::Init(CWeightManager *wg,char* vldFilename)
{	wgh = wg;
	SStream s(this,"WORLD",vldFilename);
	return;
}
//----------------------------------------------------------------------------------
//  Destroy
//---------------------------------------------------------------------------------
CVariableLoadouts::~CVariableLoadouts(void)
{}
//----------------------------------------------------------------------------------
//  Read parameters
//---------------------------------------------------------------------------------
int CVariableLoadouts::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'unit':
    {
      // Loadout instance
      CLoadCell *lu = new CLoadCell(mveh);
      ReadFrom (lu, stream);
      wgh->AddLoad(lu);
    }
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // Tag was not processed by this object, it is unrecognized
    WARNINGLOG ("CVariableLoadouts::Read : Unrecognized tag <%s>", TagToString(tag));
  }

  return rc;
}


void CVariableLoadouts::Write (SStream *stream)
{}
//=============================================================================
// Cockpit Manager
//
//=============================================================================
CCockpitManager::CCockpitManager()
{ // Initialize data members
  panel   = 0;
  globals->pit  = this;
  active  = 0;
	brit		= 1;
	//-----  Create the default light ------------
	CPanelLight *lit	= new CPanelLight(0);
	lite[0]						= lit;
}
//-----------------------------------------------------------------------------
//	JSDEV* Prepare all panels messages
//-----------------------------------------------------------------------------
void CCockpitManager::PrepareMsg(CVehicleObject *veh)
{	std::map<Tag,CPanel*>::iterator iter;
  cam     = globals->ccm->GetCockpitCamera();
	for (iter=panl.begin(); iter!=panl.end(); iter++) 
	{	CPanel *pnl = iter->second;
		pnl->PrepareMsg(veh);
	}
	return;	
}
//-----------------------------------------------------------------------------
//  Read parameters 
//-----------------------------------------------------------------------------
void CCockpitManager::Init(char* pitFilename)
{	//-----  Read cockpit manager stream ---------
  SStream s(this,"WORLD",pitFilename);
	return;
}
//-----------------------------------------------------------------------------
//  Free all resources 
//-----------------------------------------------------------------------------
CCockpitManager::~CCockpitManager (void)
{ //--- delete all panels -----------------
	std::map<Tag,CPanel*>::iterator i;
  for (i=panl.begin(); i!=panl.end(); i++)  delete (*i).second;
	panl.clear();
	//--- delete all lights -----------------
	std::map<Tag,CPanelLight*>::iterator il;
	for (il=lite.begin(); il!=lite.end(); il++) delete (*il).second;
  lite.clear();
	//--- delete holders --------------------
	std::map<Tag,CgHolder*>::iterator it;
	for (it=hold.begin(); it!=hold.end(); it++)	delete (*it).second;
	hold.clear();
	//---------------------------------------
  globals->pit  = 0;
  globals->pan  = 0;
}
//-----------------------------------------------------------------------------
//  Read parameters
//-----------------------------------------------------------------------------
int CCockpitManager::Read (SStream *stream, Tag tag)
{
  switch (tag) {
  case 'ckmg':
    // Cockpit manager sub-object
    return TAG_READ;

  case 'bgno':
  case 'endo':
    // Ignore open and close of cockpit manager sub-object
    return TAG_READ;

  case 'ckpt':
    { // Another panel instance
      Tag panelId;
      ReadTag (&panelId, stream);
      char filename[64];
      ReadString (filename, 64, stream);
			ReadPanel(filename,panelId);
    }
    return TAG_READ;
  }

  // Tag was not processed by this object, it is unrecognized
    WARNINGLOG ("CCockpitManager::Read : Unrecognized tag <%s>", TagToString(tag));
    return TAG_READ;
}
//-----------------------------------------------------------------------------
//  Read panel parameters
//-----------------------------------------------------------------------------
void	CCockpitManager::ReadPanel(char *fn,Tag id)
{	//--- Check skip option --------------------------------
	if (strcmp(fn,"NONE") == 0) return;
	if (0 == *fn)               return;
	//--- Locate panel -------------------------------------
	std::map<Tag,CPanel*>::iterator rp = panl.find(id);
  if (rp == panl.end())  WARNINGLOG ("No Panel with tag '%s'", TagToString(id));
	CPanel *pan = (*rp).second;
	pan->Init(fn);
	return;
}
//-------------------------------------------------------------------------
//  All parameters are read:  Init panel viewport
//-------------------------------------------------------------------------
void CCockpitManager::ReadFinished ()
{	}
//-------------------------------------------------------------------------
//  Add a panel
//-------------------------------------------------------------------------
void CCockpitManager::AddPanel(CPanel *pan,bool main)
{	Tag id = pan->GetID();
	// Check for duplicate panel tags
  std::map<Tag,CPanel*>::iterator i = panl.find(id);
  if (i != panl.end())  gtfo ("CCockpitManager : Duplicate panel tag : 0x%08X", id);
  // This is a unique panel tag, load it and add to the cockpit panel map
  //---Skip NONE panel ---------------------------------------------------
  panl[id] = pan;              // Enter panel
	if (main)	panel = pan;
	return;
}
//-------------------------------------------------------------------------
//  Swap between panels
//-------------------------------------------------------------------------
void CCockpitManager::ActivatePanel (Tag tag)
{ // Search for the supplied tag in the panel map
	if (tag == 'NONE')    return;
  std::map<Tag,CPanel*>::iterator i = panl.find(tag);
  panel = (i != panl.end())?(i->second):(0);
  // Activate new panel ----------------------
  if (0 == panel)				return;
 // panel->Activate();
  panel->SetViewPort();
  return;
}
//------------------------------------------------------------------------
//  Activate cockpit panel by angle
//------------------------------------------------------------------------
void  CCockpitManager::ActivateView (float A)
{ std::map<Tag,CPanel*>::iterator rp;
	for (rp = panl.begin(); rp != panl.end(); rp++)
	{	CPanel *cp = (*rp).second;
		if (!cp->View(A))		continue;
		ChangePanel(cp->GetID());
		return;
		}
  return;
}

//-------------------------------------------------------------------------
//  return Panel associated to a cockpit tag
//-------------------------------------------------------------------------
CPanel *CCockpitManager::GetPanelByTag(Tag id)
{ std::map<Tag,CPanel*>::iterator it = panl.find(id);
  return (it != panl.end())?(it->second):(0);
}
//-------------------------------------------------------------------------
//  return gauge holder or create one
//-------------------------------------------------------------------------
CgHolder *CCockpitManager::GetHolder(Tag id)
{	std::map<Tag,CgHolder*>::iterator h = hold.find(id);
  if (h != hold.end())	return (*h).second;
	//--- Create one holder ----------------------------
	CgHolder *hld = new CgHolder(id);
	hold[id]			= hld;
	return hld;
}
//-------------------------------------------------------------------------
//  Add a light
//-------------------------------------------------------------------------
void CCockpitManager::AddLight(SStream *stream)
{	char txt[64];
	CPanelLight *lit = new CPanelLight;
  ReadFrom (lit, stream);
  Tag idn = lit->GetId();
	TagToString(txt,idn);								// For debug
	//--- Check if  light exists ------------------------
	if (GetLight(idn))			
		gtfo("Light %s is redefined",TagToString(idn));
	lite[idn] = lit;
	return;
}
//-------------------------------------------------------------------------
//  Draw current panel
//-------------------------------------------------------------------------
void	CCockpitManager::Draw()
{	if (0 == panel)			return;
	panel->Draw();
	return;
}
//-------------------------------------------------------------------------
//  return a light
//-------------------------------------------------------------------------
CPanelLight *CCockpitManager::GetLight(Tag id)
{	std::map<Tag,CPanelLight*>::iterator it = lite.find(id);
	if (it != lite.end())	return (*it).second;
	return 0;
}
//-------------------------------------------------------------------------
//  Keyboard event: 
//-------------------------------------------------------------------------
bool CCockpitManager::KbEvent(Tag key)
{ if (globals->noINT)           return true;
 // panel  = cam->GetPanel();
	if (0 == panel)								return true;
  switch (key) {
    //--- Panel scroll up --------------------
    case 'ckup':
      panel->ScrollUP();
      return true;
    //--- Panel scroll down ------------------
    case 'ckdn':
      panel->ScrollDN();
      return true;
    //---- Panel scroll left -----------------
    case 'cklf':
      panel->ScrollLF();
      return true;
    //---- Panel scroll right ------------------
    case 'ckri':
      panel->ScrollRT();
      return true;
    //---- Panel to home --(ignore) ------------
    case 'ckhm':
      return true;
    //---- Panel page up --(ignore) ------------
    case 'ckpu':
      return true;
    //---- Panel page down -(ignore) -----------
    case 'ckpd':
      return true;

  }
  return true;
}
//-------------------------------------------------------------------------
//  Mouse click: 
//-------------------------------------------------------------------------
bool CCockpitManager::MouseClick(int bt,int ud,int x, int y)
{ if (globals->noINT) return false;
	if (0 == panel)			return false;
  //panel  = cam->GetPanel();
  return panel->MouseClick(bt,ud,x,y);
}
//-------------------------------------------------------------------------
//  Mouse motion: 
//-------------------------------------------------------------------------
bool CCockpitManager::MouseMove(int x,int y)
{ if (globals->noINT) return false;
	if (0 == panel)			return false;
 // panel  = cam->GetPanel();
  return panel->MouseMotion(x,y);
}
//-------------------------------------------------------------------------
//  Screen Resize
//-------------------------------------------------------------------------
void CCockpitManager::ScreenResize()
{ std::map<Tag,CPanel*>::iterator i;
  for (i = panl.begin(); i != panl.end(); i++)
  { CPanel *pan = (*i).second;
    pan->ScreenResize();
  }
  return;
}
//--------------------------------------------------------------------------
//  Time slice update panel systems
//--------------------------------------------------------------------------
void  CCockpitManager::TimeSlice (float dT)
{ // Update panel lighting
  std::map<Tag,CPanelLight*>::iterator il;
  for (il = lite.begin(); il != lite.end(); il++) (*il).second->Update(brit);
	return;
}
//--------------------------------------------------------------------------
//  Request statistics
//--------------------------------------------------------------------------
void CCockpitManager::GetStats(CFuiCanva *cnv)
{ std::map<Tag,CPanel*>::iterator iter;
  int tg = 0;
  char edt[64];
  // Get total gauges
  for (iter=panl.begin(); iter!=panl.end(); iter++) {
    CPanel *pnl = iter->second;
    tg += pnl->TotalGauges();
  }
  _snprintf(edt,63,"% 8d",tg);
	edt[63]	= 0;
  cnv->AddText(1,"Gauges:");
  cnv->AddText(STATS_NUM,edt,1);
  return;
}
//--------------------------------------------------------------------------
//  Change to left panel
//--------------------------------------------------------------------------
void CCockpitManager::ChangePanel(char dir)
{	if (0 == panel)		return; 
	Tag tag = panel->NextPanelTag(dir);
  ActivatePanel (tag);
	return;
}
//--------------------------------------------------------------------------
//  Adjust seat position
//--------------------------------------------------------------------------
void CCockpitManager::AdjustSeat(CVector &S)
{ if (0 == panel)		return;
	S.x += panel->GetPitch();
  S.z += panel->GetHeading();
	return;
}
//--------------------------------------------------------------------------
//  Set view port to panel
//--------------------------------------------------------------------------
void CCockpitManager::SetViewPort()
{	if (panel) panel->SetViewPort();	}
//===================================================================================
// Cameras Views List
//===================================================================================
CCameraViewsList::CCameraViewsList (void)
{ num_lines = 0;  type = NULL; }
//--------------------------------------------------------------------
//	Destroy it
//--------------------------------------------------------------------
CCameraViewsList::~CCameraViewsList (void)
{ SAFE_FREE (type);   }

//--------------------------------------------------------------------
//	Read camera file is any
//--------------------------------------------------------------------
int CCameraViewsList::ReadCamerasFile (void)
{
  char buffer [512] = {0};
  char *p = NULL;

	FILE *fp_;
  char file[PATH_MAX] = {0};
  /// try to find cameras_legacy.txt in the Fly!2 folder
  _snprintf (file,FNAM_MAX,"%s\\DATA\\cameras_legacy.txt", globals->FlyRoot);
  if((fp_ = fopen(file, "rt")) == NULL) {
    /// cameras_legacy.txt isn't in the Fly!2 folder
    /// try now in the Fly!Legacy folder
    fp_ = fopen("data\\cameras_legacy.txt", "rt");
  } 

  if (fp_)
	{
    fgets (buffer, 512, fp_);
    num_lines = atoi (buffer);
    //printf ("%d\n", num_lines);

    if (num_lines > 0) {

      type = (SCameraType *) malloc (num_lines * sizeof (SCameraType));
      SCameraType *dst = type;
      int num          = 0;
      for (int i = 0; i < num_lines; ++i) {
        fgets (buffer, 512, fp_);
        p = strtok (buffer, ",");
        strncpy (dst->camera_tag, p, 5);
        p = strtok (NULL, ",");
        strncpy (dst->camera_name, p, 65);
        p = strtok (NULL, "\n");
        char _val[8] = {0};
        strncpy (_val, p,7);
				_val[7]			 = 0;
        dst->val = atoi (_val);
        if (dst->val == 0)  continue;
        dst++;
        num++;
        //printf ("%s * %s * %d\n", type[i].camera_tag, type[i].camera_name, type[i].val);
      }
      return num;
    }
    return 0;
  }
  return 0;
}

//===================================================================================
// CEnginePropeller
//
// Corresponds to the <prop> sub-object in the engine model .ENG file.  The only
//   purpose of this object is to link the engine model with the external 3D model
//   so that any propeller and spinner parts are animated correctly based on the
//   engine RPM.  Note that it is not the same as the CPropellerModel subclasses
//   which implement the physics behind the thrust/torque/etc.
//  JS: This is a temporary class used for decoding only.
//===============================================================================
CEnginePropeller::CEnginePropeller (char eno,CVehicleObject *v)
{ eNum    = eno;
  mveh    = v;
  spinner = 0;
}
//----------------------------------------------------------------------
//  All parameters are read
//----------------------------------------------------------------------
void CEnginePropeller::ReadFinished()
{ 
}
//----------------------------------------------------------------------
//  Destroy this object
//----------------------------------------------------------------------
CEnginePropeller::~CEnginePropeller()
{ 
}
//----------------------------------------------------------------------
//  Read Parameters
//----------------------------------------------------------------------
int CEnginePropeller::Read (SStream *stream, Tag tag)
{
  char s[80];

  switch (tag) {

  case 'prtR':
  // Spinning part
    ReadString (s, 80, stream);
    spinner = mveh->SetSpinner(eNum,s);
    return TAG_READ;
  // Drawing position ------------------
  case 'dPos':
    ReadVector(&dPos,stream);
    if (spinner) spinner->SetPOS(dPos);
    return TAG_READ;
  //---Blade part ----------------------
  case 'prtB':
    // Pitching part
    ReadString (s, 80, stream);
    return TAG_READ;

  case 'revR':
    return TAG_READ;

  case 'revB':
    return TAG_READ;
  }

    // Tag was not processed by this object, it is unrecognized
  WARNINGLOG ("%s : Unknown tag <%s>", "CEnginePropeller", TagToString(tag));
  return TAG_IGNORED;
}
//=============================================================================
// CEngine
//
// This class represents a single instance of an <engn> object within the
//   ENG (Engine Manager) file for a vehicle
//    All the engine logical states must be managed here without presuming
//    the physical underlayer that animates the engine.
//==============================================================================
CEngine::CEngine (CVehicleObject *v,int eno, char* eTyp, char* name)
{ hwId = HW_ENGINE;
  engprp = NULL;
  mveh = v;           // Save parent vehicle
  Tag  idn   = 'Eng0' + eno;
  eNum  = eno;
  sound = 0;
  strncpy (type_string, eTyp, 8);
  strncpy (this->name, name, 56);
	*ngnFilename = 0;
  SetIdent(idn);
  type        = StringToTag(type_string);
  //-----Parameters -------------------------------------------
  spin        = -1; ///< -1 = US default ; 1 = Europa
  ngnModel    =  0;
  spinner     =  0;
  //-----Allocate engine data---------------------------------
  eData       = new CEngineData(this);
}
//--------------------------------------------------------------------------
//  Destroy this object
//--------------------------------------------------------------------------
CEngine::~CEngine (void)
{ SAFE_DELETE (eData);
  SAFE_DELETE (sound);
  SAFE_DELETE (ngnModel);
  SAFE_DELETE (engprp); // lc 052310 +
	cowlPart.clear();
}
//---------------------------------------------------------------------
//	JSDEV* Identify engine message
//---------------------------------------------------------------------
bool CEngine::MsgForMe (SMessage *msg)
{	bool matchGroup = (msg->group == unId);
  bool engnNull   = (msg->user.u.engine == 0);
  bool engnMatch  = (msg->user.u.engine == eNum);
  return matchGroup && (engnNull || engnMatch);
}
//--------------------------------------------------------------------------
//  Read parameters
//--------------------------------------------------------------------------
int CEngine::Read (SStream *stream, Tag tag)
{
  switch (tag) {
  case 'file':
  case 'ngnf':
    ReadString (ngnFilename, PATH_MAX, stream);
    return TAG_READ;
  case 'bPos':
    ReadVector(&bPos,stream);
    bPos = bPos + mveh->wgh.svh_cofg;
    return TAG_READ;
  case 'mPos':
    ReadVector (&mPos, stream);
    return TAG_READ;
  case 'spin':
    ReadInt (&spin, stream);
    return TAG_READ;
  case 'prop':
    { //MEMORY_LEAK_MARKER ("prop")
      engprp = new CEnginePropeller(eNum,mveh);
      //MEMORY_LEAK_MARKER ("prop")
      ReadFrom (engprp, stream);
      spinner = engprp->GetSpinner();
    }
    return TAG_READ;
  case 'cprt':
    {
      // Movable cowl flap part
      char s[64];
      ReadString (s, 64, stream);
      cowlPart.push_back (s);
    }
    return TAG_READ;
  }

    WARNINGLOG ("CEngine::Read : Unrecognized tag <%s>", TagToString(tag));

  return TAG_READ;
}
//-----------------------------------------------------------------------
//  Read Engine parameters
//-----------------------------------------------------------------------
void CEngine::ReadEngineParameters()
{ if (0 == *ngnFilename) return;
  ngnModel->SetEngineData(eData);
  SStream st(ngnModel,"WORLD",ngnFilename);
  return;
}
//-----------------------------------------------------------------------
//  get appropriated engine model
//-----------------------------------------------------------------------
bool  CEngine::SetEngineModel()
{  // Instantiate engine model
  if (type == 'engP') {
    // CPistonEngineModel
    ngnModel    = new CPistonEngineModel(mveh,eNum);
    ReadEngineParameters();
    int mRPM  = int(ngnModel->GetRRPM());
    return true;  }

  if (type == 'Tprp') {
    // CTurbopropEngineModel
    ngnModel = new CTurbopropEngineModel(mveh,eNum);
    ReadEngineParameters();
    return true;  }

  if (type == 'Tfan') {
    // CTurbofanEngineModel
    ngnModel = new CTurbofanEngineModel(mveh,eNum);
    ReadEngineParameters();
    return true;  }

  if (type == 'JetB') {
    // CTurbineEngineModel
    ngnModel = new CTurbineEngineModel(mveh,eNum);
    ReadEngineParameters();
    return true;  }

  if (type == 'Tapu') {
    // CTurboshaftAPUEngineModel
    ngnModel = new CTurboshaftAPUEngineModel(mveh,eNum);
    ReadEngineParameters();
    return true;  }

  if (type == 'eZnJ') {
    // CSimplisticJetEngineModel
    ngnModel = new CSimplisticJetEngineModel(mveh,eNum);
    ReadEngineParameters();
    return true;
  }
  return false;
}
//-----------------------------------------------------------------------
//  Build appropriated engine model
//  Allocate Engine sounds
//-----------------------------------------------------------------------
void CEngine::ReadFinished (void)
{ if (!SetEngineModel()) WARNINGLOG("No valid Engine model");
  //---Set Engine sound ---------------------------------
  Tag  engn = 'eng0' + eNum;
  sound = new CSoundOBJ(engn,bPos);
  sound->AddSound(ENGINE_CRANKING,'crkE');              // Cranking
  sound->AddSound(ENGINE_CATCHING,'catE');              // catching
	sound->AddSound(ENGINE_RUNNING, 'runE');              // Runing
	sound->AddSound(ENGINE_STOPPING,'hltE');              // Stopping
  sound->AddSound(ENGINE_FAILING, 'falE');              // Failed
	sound->AddSound(ENGINE_MISSFIRE,'misE');							// Miss fire
  return;
}
//-------------------------------------------------------------------------
//  Store Fuel TAP pointer if same engine
//-------------------------------------------------------------------------
bool CEngine::SetTAP(CFuelTap *tp)
{ if (tp->EngineNo() != eNum) return false;
  Tap = tp;
  return true;
}
//---------------------------------------------------------------------
//  Process message
//---------------------------------------------------------------------
EMessageResult CEngine::ReceiveMessage (SMessage *msg)
{ if (msg->id == MSG_GETDATA)
  { switch (msg->user.u.datatag)  {
      //----Get RMP -----------------------------------
    case 'erpm':
      msg->realData = eData->e_rpm;
      return MSG_PROCESSED;
    //---Get oil temperature -------------------------
    case 'oilT':
      msg->realData = eData->e_oilT;
      return MSG_PROCESSED;
    //---Get oil Pressure -----------------------------
    case 'oilP':
      msg->realData = eData->e_oilP;
      return MSG_PROCESSED;
    //---Get Exhaust gas temperature ------------------
    case 'eegT':
      msg->realData = eData->e_EGTr;
      return MSG_PROCESSED;
    //---Get Cylinder head temperature ----------------
    case 'echT':
      msg->realData = eData->e_CHTr;
      return MSG_PROCESSED;
    //---Get Manifold presure ------------------------
    case 'eMap':
      msg->realData = eData->e_Map;   // in PSF
      return MSG_PROCESSED;
    //---Get Hobb meter ------------------------------
    case 'Hobm':
      msg->realData = eData->e_hob;
      return MSG_PROCESSED;
    //---From ests -----------------------------------
    case 'data':
      msg->voidData = eData;
      return MSG_PROCESSED;
    //---Message to send status ----------------------
    case 'runs':
      { U_INT msk = msg->intData;
        char  run = eData->EngRunning();
        msg->intData = run;
        if (run) eData->startKit &=msk;
        return MSG_PROCESSED;
      }
    default:
      return CDependent::ReceiveMessage (msg);
    }
  }
  if (msg->id == MSG_SETDATA)
  { switch (msg->user.u.datatag)  {
      //---Message from prime control --------------------
      case 'prim':
        eData->startKit &= (-1 -ENGINE_PRIMER);
        eData->startKit |= (msg->intData & ENGINE_PRIMER);
        return MSG_PROCESSED;
      //---Message from magneto 1 ----------------------
      case 'mag1':
        eData->startKit &= (-1 -ENGINE_MAGN_01);
        eData->startKit |= (msg->intData & ENGINE_MAGN_01);
        return MSG_PROCESSED;
      //---Message from magneto 2 ----------------------
      case 'mag2':
        eData->startKit &= (-1 -ENGINE_MAGN_02);
        eData->startKit |= (msg->intData & ENGINE_MAGN_02);
        return MSG_PROCESSED;
      //---Message from Starter ------------------------
      case 'strt':
        eData->startKit &= (-1 -ENGINE_STARTER);
        eData->startKit |= (msg->intData & ENGINE_STARTER);
        return MSG_PROCESSED;
      //---Message from Igniter ------------------------
      case 'ignr':
        eData->startKit &= (-1 -ENGINE_IGNITER);
        eData->startKit |= (msg->intData & ENGINE_IGNITER);
        return MSG_PROCESSED;
      //---Message from rign ---------------------------
      case 'rign':
        eData->startKit  = msg->intData;
        return MSG_PROCESSED;
      //---Message from Throttle controll --------------
      case 'thro':
        eData->e_thro = float(msg->realData);
        return MSG_PROCESSED;
        //---Message from Throttle Inverter controll ----
      case 'revr':
        eData->e_revr = float(msg->realData);
        return MSG_PROCESSED;
      //---Message from mixture control -----------------
      case 'mixt':
        eData->e_mixt = float(msg->realData);
        return MSG_PROCESSED;
      //---Message from blad control --------------------
      case 'blad':
        eData->e_blad = float(msg->realData);
        return MSG_PROCESSED;
      default:
        return CDependent::ReceiveMessage (msg);
     }
  }
  //----------------------------------------------------
  return CDependent::ReceiveMessage (msg);
}
//----------------------------------------------------------------------
//  Time slice engine
//----------------------------------------------------------------------
void CEngine::Timeslice (float dT,U_INT FrNo)
{ //---Update hobb meter --------------------------------------
  if (eData->EngPowered())  eData->e_hob += (dT / 3600);
  ngnModel->TimeSlice(dT,FrNo);
  ngnModel->SetTrueRPM();
  if (spinner) spinner->SetRPM(eData->e_rpm);
}
//---------------------------------------------------------------------
//  Set Fuel state from fuel cycle
//---------------------------------------------------------------------
void CEngine::SetFuel(U_CHAR k)
{ eData->startKit &= (-1 -ENGINE_FUELED);
  eData->startKit |= (k & ENGINE_FUELED);
  return;
}
//---------------------------------------------------------------------
//  Set condition p
//---------------------------------------------------------------------
void CEngine::SetMixture(U_INT p)
{	eData->startKit &= (-1 -ENGINE_MIXTURE);	// Reset property
	eData->startKit |= (p & ENGINE_MIXTURE);  // Set property
	return;
}
//---------------------------------------------------------------------
//  Update engine logical states
//  Return  0 if no state change
//          1 if state was changed
//---------------------------------------------------------------------
int CEngine::RefreshState()
{ U_CHAR state = eData->e_state;
  switch (state)  {
    case ENGINE_STOPPED:
      eData->powered = 0;
      return StateStopped();
    case ENGINE_CRANKING:
      eData->powered = 1;
      eData->e_stop  = 0;
      return StateCranking();
    case ENGINE_RUNNING:
      eData->powered = 1;
      return StateRunning();
    //--moving under wind ------------
    case ENGINE_WINDMILL:
      eData->powered = 0;
      return StateWindmill();
    //--Stopping     -----------------
    case ENGINE_STOPPING:
      eData->powered = 0;
      return StateStopping();
    //--Engine is catching -----------
    case ENGINE_CATCHING:
      eData->powered = 1;
      return StateCatching();
		//---Engine missfire -------------
		case ENGINE_MISSFIRE:
			eData->powered = 1;
			return StateMissfire();
    //---Engine is failing -----------
    case ENGINE_FAILING:
      eData->powered = 0;
      return StateFailing();
  }
  return 0;
}
//---------------------------------------------------------------------
//  short Stop engine
//  FOR now just stop engine
//  Next when sound is correctly handled we will do a nice
//  stopping
//---------------------------------------------------------------------
int  CEngine::StopEngine(char r)
{ if (eData->e_state == ENGINE_STOPPED)   return 0;
  if (eData->e_state == ENGINE_STOPPING)  return 0;
  eData->e_stop   = r;
  pstat           = eData->e_state;
  eData->e_state  = ENGINE_STOPPING;
	//--- Play stopping sound ---------------------
	sound->SetLoop(false);
  sound->StopSound();
	sound->SetEnginePitch(1);
	sound->Play(ENGINE_STOPPING);
	mveh->DecEngR();
  return 1;
}
//---------------------------------------------------------------------
//  Engine idle
//---------------------------------------------------------------------
int CEngine::EngineIdle()
{ eData->e_thro = 0.1f;
  sound->SetEnginePitch(eData->Pitch());
  ngnModel->Idle();
  return 1;
}
//---------------------------------------------------------------------
//  Stop engine
//  FOR now just stop engine
//  Next when sound is correctly handled we will do a nice
//  stopping
//---------------------------------------------------------------------
int  CEngine::AbortEngine(char r)
{ StopEngine(r);
  ngnModel->Abort();
  return 1;
}
//---------------------------------------------------------------------
//  Set Cranking state
//---------------------------------------------------------------------
int CEngine::CrankEngine()
{ //--- Change state to cranking -------------------
  pstat           = eData->e_state;
  eData->e_state  = ENGINE_CRANKING;
  sound->SetLoop(false);
  sound->SetEnginePitch(1);
  sound->Play(ENGINE_CRANKING);
  return 1;
}
//---------------------------------------------------------------------
//  Set Launching state
//  This state is intermediate before running state
//---------------------------------------------------------------------
int CEngine::LaunchEngine()
{ pstat           = eData->e_state; 
  eData->e_state  = ENGINE_CATCHING;
	sound->SetOffset(0);
  sound->SetLoop(false);
  sound->SetEnginePitch(1);
  sound->Play(ENGINE_CATCHING);
  return 0;
}
//---------------------------------------------------------------------
//  Set Failing state
//---------------------------------------------------------------------
int CEngine::FailEngine()
{ pstat           = eData->e_state;
  eData->e_state  = ENGINE_FAILING;
  sound->SetLoop(false);
  sound->SetEnginePitch(1);
  sound->Play(ENGINE_FAILING);
  return 1;
}
//---------------------------------------------------------------------
//  Set Start state
//---------------------------------------------------------------------
int CEngine::StartEngine()
{ pstat           = eData->e_state;
  eData->e_state  = ENGINE_RUNNING;
  sound->SetLoop(true);
  sound->Play(ENGINE_CATCHING);
	mveh->IncEngR();
  return 1;
}
//---------------------------------------------------------------------
//  Set Missfire state
//---------------------------------------------------------------------
int CEngine::MissfireEngine()
{	pstat           = eData->e_state;
  eData->e_state  = ENGINE_MISSFIRE;
	sound->StopSound();
  sound->SetLoop(false);
  sound->Play(ENGINE_MISSFIRE);
	sound->SetEnginePitch(eData->Pitch());
  return 1;
}
//---------------------------------------------------------------------
//  Update engine state
//  -Check for starting conditions 
//   To activate the starter, the starter bit is needed
//  -For windmill start one magneto at least is needed
//---------------------------------------------------------------------
int  CEngine::StateStopped()
{ //--- Check cranking conditions ------------------
  if (mveh->NotState(VEH_OPER)) return ENGINE_STOPPED;
  U_INT strt = eData->startKit;
  U_INT ston = strt & ENGINE_STARTER;
  if (ston )          return CrankEngine();
  //--- Check windmill start  ----------------------
  U_INT tst0 = (eData->e_wml > eData->s_wml);
  if (0 == tst0)      return 1;
  U_INT tst1 = strt & ENGINE_MAGN_12;
  if (0 == tst1)      return 1;
  //---Hot start from windmill ---------------------
  pstat = eData->e_state;
  sound->SetEnginePitch(eData->Pitch());
  sound->Play(ENGINE_MISSFIRE);
  eData->e_state = ENGINE_MISSFIRE;
  return 1;
}
//---------------------------------------------------------------------
//  Update engine state
//   Engine is cranking 
//   For effective start we need
//    -Both magneto for sparks
//    -Fuel
//   if echec go back to previou state (either stopped or windmill)
//---------------------------------------------------------------------
int  CEngine::StateCranking()
{ if (sound->IsPlaying()) return 0;
  U_INT cond = (ENGINE_MAGN_12 + ENGINE_FUELED);
  //--- Start if conditions OK ---------------------
  U_INT ston = eData->startKit & cond;
  if (ston == cond)       return LaunchEngine();
  //--- Engine is failing --------------------------
  return FailEngine();
}
//---------------------------------------------------------------------
//  Engine is catching
//  Check if sound must be changed
//---------------------------------------------------------------------
int CEngine::StateCatching()
{ //--- Check if conditions OK ---------------
  U_INT neto = (eData->startKit & ENGINE_MAGN_12);
  U_INT fuel = (eData->startKit & ENGINE_FUELED);
	U_INT mix  = (eData->startKit & ENGINE_MIXTURE);
  U_INT goin = (neto && fuel && mix);
  if (!goin)                  return FailEngine();
  //--- Play sound before starting -------
  if (sound->IsPlaying())			return 0;
  sound->SetEnginePitch(eData->Pitch());
	sound->Play(ENGINE_RUNNING);
  eData->e_state = ENGINE_RUNNING;
	mveh->IncEngR();
  return 1;
}
//---------------------------------------------------------------------
//  Engine is failing
//  Check if sound must be changed
//---------------------------------------------------------------------
int CEngine::StateFailing()
{ if (sound->IsPlaying()) return 0;
  pstat           = eData->e_state;
  eData->e_state  = ENGINE_STOPPED;
  return 1;
}
//---------------------------------------------------------------------
//  Engine is running
//  To maintain this state we need
//  -At least one magneto for spark
//  -Fuel
//---------------------------------------------------------------------
int CEngine::StateRunning()
{ U_INT neto = (eData->startKit & ENGINE_MAGN_12);
  U_INT fuel = (eData->startKit & ENGINE_FUELED);
	U_INT	mixt = (eData->startKit & ENGINE_MIXTURE);
  if (!neto)          return StopEngine(1);
  if (!fuel)          return StopEngine(2);
	if (!mixt)					return MissfireEngine();
  //----Check for stall conditions ------------------------------------
  bool stal  = (eData->e_rpm < eData->s_rpm);
  if  (stal)    			return StopEngine(3);
	sound->SetEnginePitch(1);
  sound->Play(ENGINE_RUNNING);
  sound->SetEnginePitch(eData->Pitch());
  return 0;
}
//---------------------------------------------------------------------
//  Engine is Stopping from running state 
//  This state is just to allow for some lag before stopping
//---------------------------------------------------------------------
int CEngine::StateStopping()
{ //---Stop if RPM dying  -------------------------------
  if (sound->IsPlaying())		return 0;
  pstat           = eData->e_state;
  eData->e_state  = ENGINE_STOPPED;
  return 1;
}
//---------------------------------------------------------------------
//  Engine is missfireing
//  The engine may be restarted from this state when conditions are met
//---------------------------------------------------------------------
int CEngine::StateMissfire()
{	if (sound->IsPlaying()) return 0;
	//--- Check for runing conditions -----------------
	U_INT	mixt = (eData->startKit & ENGINE_MIXTURE);
	if (0 == mixt)					return StopEngine(4);
	//--- Set running state ---------------------------
	pstat           = eData->e_state;
  eData->e_state  = ENGINE_RUNNING;
	sound->SetLoop(true);
  sound->SetEnginePitch(eData->Pitch());
	sound->Play(ENGINE_RUNNING);
	return 1;
}
//---------------------------------------------------------------------
//  Engine is windmilling
//  The engine may be restarted from this state when conditions are met
//---------------------------------------------------------------------
int CEngine::StateWindmill()
{ U_INT cond = (ENGINE_MAGN_12);
  U_INT strt = eData->startKit & ENGINE_MAGN_12;
	U_INT mixt = eData->startKit & ENGINE_MIXTURE;
  count = 0;
  if (0 == strt)    return 0;
	if (0 == mixt)		return 0;
  //---Enter runing state ----------------------------
  pstat           = eData->e_state;
	eData->e_state  = ENGINE_RUNNING;
  sound->SetEnginePitch(eData->Pitch());
	sound->Play(ENGINE_RUNNING);
	mveh->IncEngR();
  return 1;
}

//-----------------------------------------------------------------------
//  Return engine position in meters
//-----------------------------------------------------------------------
const SVector& CEngine::GetThrustPosInM (void) 
{
  //SVector bPosThrustInM;

  TPosISU.x = FN_METRE_FROM_FEET (bPos.x);
  TPosISU.y = FN_METRE_FROM_FEET (bPos.y);
  TPosISU.z = FN_METRE_FROM_FEET (bPos.z);
  return TPosISU;
}
//----------------------------------------------------------------------
//  Add item to plot menu
//----------------------------------------------------------------------
int CEngine::AddToPlotMenu(char **menu, PLOT_PM *pm, int k)
{ return ngnModel->AddToPlotMenu(menu,pm,k); }
//----------------------------------------------------------------------
//  request to plot some data
//----------------------------------------------------------------------
bool CEngine::PlotParameters(PLOT_PP *pp,Tag id, Tag type)
{ return ngnModel->PlotParameters(pp,id,type); }
//==========================================================================
//  Probe this engine
//==========================================================================
void CEngine::Probe(CFuiCanva *cnv)
{ char *ye = "yes";
  char *no = "no";
  char  rc = eData->e_stop;
  cnv->AddText(1,1,"Throttle: %.02f",eData->e_thro);
  cnv->AddText(1,1,"Mixture:  %.02f",eData->e_mixt);
  cnv->AddText(1,1,"Blad:     %.02f",eData->e_blad);
  cnv->AddText(1,1,"e_gph: %.05f",eData->e_gph);
  cnv->AddText(1,1,"e_rpm: %.05f",eData->e_rpm);
  cnv->AddText(1,1,"e_vel: %.05f",eData->e_vel);
  cnv->AddText(1,1,"e_wml: %.05f",eData->e_wml);
  cnv->AddText(1,1,"STATE: %s",engSTA[eData->e_state]);
  cnv->AddText(1,1,"e_stop: %s",engSTOP[rc]);

  char *f = (eData->startKit & ENGINE_FUELED)?(ye):(no);
  cnv->AddText(1,1,"Fueled %s",f);

  char *m1 = (eData->startKit & MAGNETO_SWITCH_RIGHT)?(ye):(no);
  cnv->AddText(1,1,"Magneto R: %s",m1);

  char *m2 = (eData->startKit & MAGNETO_SWITCH_LEFT)?(ye):(no);
  cnv->AddText(1,1,"Magneto L: %s",m2);
  
  char *st = (eData->startKit & MAGNETO_SWITCH_START)?(ye):(no);
  cnv->AddText( 1,1,"Starter  : %s",st);
  return;
}

//====================================================================
// CEngineManager
//====================================================================
CEngineManager::CEngineManager ()
{ engn.clear ();
  engine_number = 0;

  eng_total_force.x   = eng_total_force.y   = eng_total_force.z   = 0.0;
  eng_total_moment.x  = eng_total_moment.y  = eng_total_moment.z  = 0.0;
  eng_total_pos.x     = eng_total_pos.y     = eng_total_pos.z     = 0.0;
  prop_total_torque.x = prop_total_torque.y = prop_total_torque.z = 0.0;
  thrust_X_offset = 0.0f;

  
}
//----------------------------------------------------------------------
//	Read all parameters
//----------------------------------------------------------------------
void CEngineManager::Init(char* ngnFilename)
{	SStream s(this,"WORLD",ngnFilename);
}
//----------------------------------------------------------------------
//	remove all engine definitions
//----------------------------------------------------------------------
CEngineManager::~CEngineManager (void)
{ for (ie=engn.begin(); ie!=engn.end(); ie++)		delete *ie;
}
//----------------------------------------------------------------------
//	Read  engine parameters
//----------------------------------------------------------------------
int CEngineManager::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;
  //
  //#ifdef _DEBUG	
  //{	FILE *fp_debug;
	 // if(!(fp_debug = fopen("__DDEBUG_CEngineManager.txt", "a")) == NULL)
	 // {
		//  fprintf(fp_debug, "CEngineManager\n");
		//  fclose(fp_debug); 
  //}	}
  //#endif

  switch (tag) {
  case 'engn':
    {// Read the engine type and unique name
      char engine[64], engine_type[8], engine_name[56];
      ReadString (engine, 64, stream);
      if (sscanf (engine, "%s %s", engine_type, engine_name) == 2) {
        engine_number++;
        //MEMORY_LEAK_MARKER ("engin")
        CEngine* engin = new CEngine (mveh,engine_number, engine_type, engine_name);
        //MEMORY_LEAK_MARKER ("engin")
        ReadFrom (engin, stream);
        engn.push_back (engin);
        //
      } else {
        WARNINGLOG ("CEngineManager::Read : bad engine type or name");
      }
    }
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // Tag was not processed by this object, it is unrecognized
    WARNINGLOG ("CEngineManager::Read : Unrecognized tag <%s>", TagToString(tag));
  }

  return rc;
}
//----------------------------------------------------------------------
//  Link engine to fuel TAP
//----------------------------------------------------------------------
void CEngineManager::SetTAP(CFuelTap *tp)
{ std::vector<CEngine*>::iterator it;
  for (it = engn.begin(); it != engn.end(); it++)
  { if ((*it)->SetTAP(tp))  return; }
  return;
}
//----------------------------------------------------------------------
//  Time slice engine
//----------------------------------------------------------------------
void CEngineManager::Timeslice (float dT,U_INT FrNo)
{
  // Timeslice each engine
  for (ie=engn.begin(); ie!=engn.end(); ie++) {
    (*ie)->Timeslice (dT,FrNo);
  }
}
//----------------------------------------------------------------------
//  Cut All engines
//----------------------------------------------------------------------
void CEngineManager::CutAllEngines()
{ std::vector<CEngine*>::iterator eg;
  for (eg = engn.begin(); eg != engn.end(); eg++)
  { CEngine *eng = (*eg);
    eng->StopEngine(5);
		eng->Reset();
  }
  return;
}
//----------------------------------------------------------------------
//  All engines idels
//----------------------------------------------------------------------
void CEngineManager::EnginesIdle()
{ std::vector<CEngine*>::iterator eg;
  for (eg = engn.begin(); eg != engn.end(); eg++)
  { CEngine *eng = (*eg);
    eng->EngineIdle();
  }
  return;
}

//----------------------------------------------------------------------
//  Abort All engines
//----------------------------------------------------------------------
void CEngineManager::AbortEngines()
{ std::vector<CEngine*>::iterator eg;
  for (eg = engn.begin(); eg != engn.end(); eg++)
  { CEngine *eng = (*eg);
    eng->AbortEngine(6);
		eng->Reset();
  }
  return;
}

//----------------------------------------------------------------------
//  Get Force ISU
//----------------------------------------------------------------------
const SVector& CEngineManager::GetForceISU  (void) // LH
{ std::vector<CEngine*>::iterator eg;
  eng_total_force.z = 0.0;
  for (eg = engn.begin (); eg != engn.end (); eg++) {
    // link test
    if ((*eg)->EngRunning())
    //
    eng_total_force.z += (*eg)->GetThrustInNewton ();
  }
  return eng_total_force;
}
//----------------------------------------------------------------------
//  Get Torque ISU
//----------------------------------------------------------------------
const SVector& CEngineManager::GetPropellerTorqueISU (void) // Newton x Metres LH
{ std::vector<CEngine*>::iterator eg;
  /// \todo GetTorqueInNM must return 0.0 when aircraft has no propeller
  prop_total_torque. x = prop_total_torque.y = 0.0;
  // only single prop are considered by now
  for (eg = engn.begin (); eg != engn.end (); eg++) {
    if ((*eg)->GetType () == 'engP') {
      prop_total_torque.z = (*eg)->GetTorqueInNM () * (*eg)->GetPropSpin ();
    }
  }
  return prop_total_torque;
}
//----------------------------------------------------------------------
//  Get Moment ISU
//----------------------------------------------------------------------
const SVector& CEngineManager::GetMomentISU (void) // LH
{ std::vector<CEngine*>::iterator eg;
  CVector eng_total_moment_;
  SVector engM = {0.0, 0.0, 0.0};
  //
  
  for (eg = engn.begin (); eg != engn.end (); eg++) {
    //
    //++ie;
    if ((*eg)->EngRunning()) {
      //
      SVector thrust = {0.0, 0.0, 0.0};
      thrust.z = (*eg)->GetThrustInNewton ();
      //
      SVector pos = (*eg)->GetThrustPosInM ();
      // P factor = x offset
      // cut torque effect when stopped and braking //
      if (mveh->GetBodyVelocityVector ()->z > 05.0) // only when speed is > 5 m/s =~10 knts
      //
        pos.x += static_cast<double> ((*eg)->GetThrustXOffset ());
      //
      VectorCrossProduct (engM, pos, thrust);
      eng_total_moment_ = VectorSum (eng_total_moment_, engM);
      //eng_total_moment_.Times (0.25); // magic number
    }
  }
  eng_total_moment = eng_total_moment_;
  return eng_total_moment;
}
//----------------------------------------------------------------------
//  Get Engine PFACT
//----------------------------------------------------------------------
const float& CEngineManager::GetEnginesPfact (void) // LH
{ std::vector<CEngine*>::iterator eg;
  thrust_X_offset = 0.0f;
  for (eg = engn.begin (); eg != engn.end (); eg++) {
    thrust_X_offset = (*eg)->GetThrustXOffset ();
  }
  return thrust_X_offset;
}
//----------------------------------------------------------------------
//  Get Engine position ISU
//----------------------------------------------------------------------
const SVector& CEngineManager::GetEnginesPosISU (void) // LH
{ std::vector<CEngine*>::iterator eg;
  /// \todo global position has to keep trace of the each engine power
  ///       right now we assume each engine gets the similar power
  CVector eng_total_pos_;
  for (eg = engn.begin (); eg != engn.end (); eg++) {
    eng_total_pos_ = eng_total_pos_ + (*eg)->GetThrustPosInM ();
    // P factor = x offset
    // cut torque effect when stopped and braking //
    if (mveh->GetBodyVelocityVector ()->z > 05.0) // only when speed is > 5 m/s =~10 knts
    //
      eng_total_pos_.x += static_cast<double> ((*eg)->GetThrustXOffset ());
    //
  }
  eng_total_pos = eng_total_pos_;

  return eng_total_pos;
}
//----------------------------------------------------------------------
//  Add item to plot menu
//----------------------------------------------------------------------
int CEngineManager::AddToPlotMenu(char **menu, PLOT_PM *pm, int k)
{ //---Add plots for each engine -----------------------------
  int p = k;
  std::vector<CEngine*>::iterator eg;
  for (eg = engn.begin (); eg != engn.end (); eg++)
  { CEngine *eng = (*eg);
    p = eng->AddToPlotMenu(menu,pm,p);
  }
  return 0;
}
//----------------------------------------------------------------------
//  Request to plot something.  Pass it to engines
//----------------------------------------------------------------------
void CEngineManager::PlotParameters(PLOT_PP *pp, Tag id, Tag type)
{ //---check each engine -----------------------------
  std::vector<CEngine*>::iterator eg;
  for (eg = engn.begin (); eg != engn.end (); eg++)
  { CEngine *eng = (*eg);
    if (eng->PlotParameters(pp,id,type)) return;
  }
  return;
}
//===============================================================================
/*
 *  CAeroControlChannel
 */
//===============================================================================
CAeroControlChannel::CAeroControlChannel (char *name)
{ strncpy(chn,name,31);
  chn[31] = 0;
  pos = neg = 1.0f;
  deflect = 0;
  scaled  = 0;
  radians = 0;
  keyframe = 0.5;
}

int CAeroControlChannel::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'pos%':
    ReadFloat (&pos, stream);
    rc = TAG_READ;
    break;
  case 'neg%':
    ReadFloat (&neg, stream);
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // Tag was not processed by this object, it is unrecognized
    WARNINGLOG ("CAeroControlChannel::Read : Unrecognized tag <%s>", TagToString(tag));
  }

  return rc;
}
//---------------------------------------------------------------------------
//  Return the control value
//---------------------------------------------------------------------------
float CAeroControlChannel::Value (float value)
{
  // Adjust value for positive/negative percentages
  if (value >= 0) value *= pos;
  else            value *= neg;
  return value;
}
//===========================================================================
//  Channel Mixer control
//  JS NOTE: Modify the time slice to avoid to poll all wing objects for 
//          value propagation.
//          1) When a CAeroControlChannel is created, a pointer is
//            stored in the corresponding Mixer channel.
//          2) Values are just stored into the controlChannel
//          3) A Wing object extracts channel values directly when time sliced
//===========================================================================
CControlMixerChannel::CControlMixerChannel (CVehicleObject *v)
{ mveh   = v;     // Save mother vehicle
  group  = 0;
  invert = false;
  msg.id = MSG_GETDATA;
}
//----------------------------------------------------------------
//  Destroy this object
//---------------------------------------------------------------
CControlMixerChannel::~CControlMixerChannel (void)
{ std::vector<CAeroControlChannel*>::iterator i;
  for (i=aerochannel.begin(); i!=aerochannel.end(); i++)   delete (*i);
	aerochannel.clear();
}
//----------------------------------------------------------------
//  Set Name
//---------------------------------------------------------------
void CControlMixerChannel::SetName(char *n)
{ strncpy(name,n,15);
  name[15] = 0;
  return;
}
//----------------------------------------------------------------
//  Read control mixer parameters
//---------------------------------------------------------------
int CControlMixerChannel::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'gnum':
    ReadInt (&group, stream);
    rc = TAG_READ;
    break;
  case 'nvrt':
    invert = true;
    rc = TAG_READ;
    break;
  case 'mesg':
    ReadMessage (&msg, stream);
    msg.sender = 'mixr';
    msg.id = MSG_GETDATA;
    rc = TAG_READ;
    break;
  case 'chan':
    {
      char name[80];
      ReadString (name, 80, stream);
      CAeroControlChannel* aero = new CAeroControlChannel(name);
      //TRACE("ADD CHANNEL %s:",name);
      ReadFrom (aero, stream);
      aerochannel.push_back(aero);
    }
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // Tag was not processed by this object, it is unrecognized
    WARNINGLOG ("CControlMixerChannel::Read : Unrecognized tag <%s>", TagToString(tag));
  }

  return rc;
}
//----------------------------------------------------------------
//  Get rudder mixer channel
//---------------------------------------------------------------
CAeroControlChannel *CControlMixerChannel::GetRudder()
{ std::vector<CAeroControlChannel*>::iterator i;
  for (i=aerochannel.begin(); i!=aerochannel.end(); i++) {
    CAeroControlChannel *chn = (*i);
    char                     *idn = chn->GetName();
    if (0 == strcmp(idn,"Rudder"))  return chn;
  }
  return 0;
}
//----------------------------------------------------------------
//  Link Mixer channel to Wing section
//---------------------------------------------------------------
void CControlMixerChannel::LinktoWing()
{std::vector<CAeroControlChannel*>::iterator i;
 for (i=aerochannel.begin(); i!=aerochannel.end(); i++) {
    CAeroControlChannel *aero = (*i);
    mveh->SetWingChannel(aero);
 }
 return;
}
//==================================================================================
//  What the mixer does is just to modulate the values (raw and scaled) by a damping
//  coefficent.
//  Control mixers poll associated control to get the deflection value
//==================================================================================
void CControlMixerChannel::Timeslice (float dT,U_INT FrNo)
{ 
  // Poll control subsystem for all values
  msg.user.u.datatag = 'data';
  Send_Message (&msg);
  //TRACE("MIXER GET MSG from %s",TagToString(msg.group));
  MIXER_DATA *data = (MIXER_DATA*)msg.voidData;
  if (0 == data)        return;
  float dClamp  =  data->ampli;
  float dOffset = -data->minClamp * 0.5f;

  // Iterate over all aero model control channels to set the values
  std::vector<CAeroControlChannel*>::iterator i;
  for (i=aerochannel.begin(); i!=aerochannel.end(); i++) {
    CAeroControlChannel *chn = (*i);
    //--Store deflection and scaled value in controller ----
    float deflect   = chn->Value(data->deflect);
    float scaled    = data->deflect * data->scal;   // data in degre
    float key       = (deflect / dClamp) + dOffset;
    data->scaled    = scaled;
    chn->SetDeflect(deflect);
    chn->SetScaled (scaled);
    chn->SetKeyframe(key);
    //TRACE("%04d-MIXER SET CHANNEL %-20s: def=%-.5f Radian=%-.5f Key=%-.5f Pos=%-.5f Neg=%-.5f",FrNo,
    //  chn->GetName(),deflect,chn->GetRadians(),key,chn->GetPos(),chn->GetNeg());
  }
  return;
}

//=======================================================================================
//  CONTROL MIXER:  All objects from MIX file
//  JS NOTES:  Modify for the following reasons:
//            More than one mixer may exist for one channel
//            Example for AILERON where 2 objects exist with a group parameter
//            One version is used when rudder is coupled to aileron, the other
//            is used when no coupling exists.
//            This is not very easy to manage
//=======================================================================================
CControlMixer::CControlMixer ()
{ rPos  = 0.05f;          // Coupled rudder default value 
  rNeg  = 0.05f;          // Coupled rudder default value
 
}
//---------------------------------------------------------------------
//  Read parameters
//---------------------------------------------------------------------
void CControlMixer::Init(char* mixFilename)
{	SStream s(this,"WORLD",mixFilename);
	return;	}
//---------------------------------------------------------------------
//  Destroy this object
//---------------------------------------------------------------------
CControlMixer::~CControlMixer (void)
{ std::map<string,CControlMixerChannel*>::iterator i;
  for (i=mixerMap.begin(); i!=mixerMap.end(); i++)  delete i->second;
  mixerMap.clear();
	channel.clear();
}
//--------------------------------------------------------------------
//  Add one mixer
//--------------------------------------------------------------------
void CControlMixer::AddMixer(CControlMixerChannel *mix, char *name)
{ //--- Eliminate coupled rudder case for aileron
  if ((strcmp(name,"AILERON") == 0) && (mix->NumberItem() == 3)) 
  { CAeroControlChannel *rud = mix->GetRudder();
    if (rud)  rPos = rud->GetPos();
    if (rud)  rNeg = rud->GetNeg();
    delete mix;
    return;
  }
  //---Only one mixer per axis is kept ---------------------------
  std::map<string,CControlMixerChannel*>::const_iterator i = mixerMap.find(name);
  if (i != mixerMap.end()) {delete mix;  return;}
  mixerMap[name] = mix;
  mix->LinktoWing();
  return;
}
//--------------------------------------------------------------------
//  Read parameters
//--------------------------------------------------------------------
int CControlMixer::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'chnl':
    {
      char name[80];
      ReadString (name, 80, stream);
      channel.insert (name);
    }
    rc = TAG_READ;
    break;
  case 'mixr':
    {
      char name[80];
      ReadString (name, 80, stream);
      CControlMixerChannel* mixer = new CControlMixerChannel(mveh);
      mixer->SetName(name);
      ReadFrom (mixer, stream);
      AddMixer(mixer,name);
    }
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // Tag was not processed by this object, it is unrecognized
    WARNINGLOG ("CControlMixer::Read : Unrecognized tag <%s>", TagToString(tag));
  }

  return rc;
}

//void CControlMixer::ReadFinished (void)
//{
//}
//--------------------------------------------------------------------------
//  Poll all control surfaces
//--------------------------------------------------------------------------
void CControlMixer::Timeslice (float dT,U_INT FrNo)
{ //-------------------------------------------------
  // Timeslice each of the defined channels
  std::map<string,CControlMixerChannel*>::iterator i;
  for (i=mixerMap.begin(); i!=mixerMap.end(); i++) i->second->Timeslice (dT,FrNo);
  return;
}
//=======================================================================
//
// Slope Wind Data
//=======================================================================
CSlopeWindData::CSlopeWindData (const char* swdFilename)
{

}

int CSlopeWindData::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc != TAG_READ) {
    // Tag was not processed by this object, it is unrecognized
    WARNINGLOG ("CSlopeWindData::Read : Unrecognized tag <%s>", TagToString(tag));
  }

  return rc;
}

void CSlopeWindData::ReadFinished (void)
{
}


void CSlopeWindData::Write (SStream *stream)
{
}


//
// CVehicleHistory
//
CVehicleHistory::CVehicleHistory (void)
{
  readHobbsMeter = false;
  hobbsMeter = 0;
  readTachTimer = false;
  tachTimer = 0;
  repairCount = 0;
  ailrExp = elvrExp = rudrExp = 0;
  ailrTrimStep = elvrTrimStep = rudrTrimStep = 0.01f;
}

int CVehicleHistory::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'HOBS':
    ReadFloat (&hobbsMeter, stream);
    readHobbsMeter = true;
    rc = TAG_READ;
    break;
  case 'TAKT':
    ReadFloat (&tachTimer, stream);
    readTachTimer = true;
    rc = TAG_READ;
    break;
  case 'REPR':
    ReadInt (&repairCount, stream);
    rc = TAG_READ;
    break;
  case 'aExp':
    ReadFloat (&ailrExp, stream);
    rc = TAG_READ;
    break;
  case 'eExp':
    ReadFloat (&elvrExp, stream);
    rc = TAG_READ;
    break;
  case 'rExp':
    ReadFloat (&rudrExp, stream);
    rc = TAG_READ;
    break;
  case 'aStp':
    ReadFloat (&ailrTrimStep, stream);
    rc = TAG_READ;
    break;
  case 'eStp':
    ReadFloat (&elvrTrimStep, stream);
    rc = TAG_READ;
    break;
  case 'rStp':
    ReadFloat (&rudrTrimStep, stream);
    rc = TAG_READ;
    break;
  case 'engm':
    // \todo Parse engine manager history
    SkipObject (stream);
    rc = TAG_READ;
    break;
  case 'whem':
    // \todo Parse wheel manager history
    SkipObject (stream);
    rc = TAG_READ;
    break;
  case 'wngs':
    // \todo Parse wing manager history
    SkipObject (stream);
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // Tag was not processed by this object, it is unrecognized
    WARNINGLOG ("CVehicleHistory::Read : Unrecognized tag <%s>", TagToString(tag));
  }

  return rc;
}

float CVehicleHistory::GetHobbsMeter (void)
{
  return hobbsMeter;
}

float CVehicleHistory::GetTachTimer (void)
{
  return tachTimer;
}

int CVehicleHistory::GetRepairCount (void)
{
  return repairCount;
}

float CVehicleHistory::GetAileronExponential (void)
{
  return ailrExp;
}

float CVehicleHistory::GetElevatorExponential (void)
{
  return elvrExp;
}

float CVehicleHistory::GetRudderExponential (void)
{
  return rudrExp;
}

float CVehicleHistory::GetAileronTrimStep (void)
{
  return ailrTrimStep;
}

float CVehicleHistory::GetElevatorTrimStep (void)
{
  return elvrTrimStep;
}

float CVehicleHistory::GetRudderTrimStep (void)
{
  return rudrTrimStep;
}

//========================================================================
// CVehicleInfo
//========================================================================
CVehicleInfo::CVehicleInfo ()
{ // Initialize data members
 *make = 0;
 *iconFilename = 0;
  classification = 0;
  usage = 0;
  *svhFilename = 0;
  *gasFilename = 0;
  *wngFilename = 0;
  *ampFilename = 0;
  *pssFilename = 0;
  *whlFilename = 0;
  *vldFilename = 0;
  *pitFilename = 0;
  *camFilename = 0;
  *lodFilename = 0;
  *rdoFilename = 0;
  *eltFilename = 0;
  *engFilename = 0;
  *mixFilename = 0;
  *cklFilename = 0;
  *fcsFilename = 0;
  *swdFilename = 0;
  *sitFilename = 0;
  *pidFilename = 0;
  *phyFilename = 0; // PHY file
}
//------------------------------------------------------------------------
// CVehicleInfo Read parameters
//------------------------------------------------------------------------
void CVehicleInfo::Init(char *nfoFilename)
{	// Open stream for NFO file
  SStream s(this,"WORLD",nfoFilename);
	if (!s.ok) gtfo ("File %s not found", nfoFilename);
	return;
}
//------------------------------------------------------------------------
// CVehicleInfo Read parameters
//------------------------------------------------------------------------
int CVehicleInfo::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;
  char s[256];

  switch (tag) {
  case 'sign':
    // Vehicle type signature
    ReadString (s, 256, stream);
    break;
  case 'make':
    // Make/model of vehicle
    ReadString (make, sizeof(make) - 1, stream);
    break;
  case 'icon':
    // Vehicle selection menu icon filename
    ReadString (iconFilename, sizeof(iconFilename) - 1, stream);
    break;
  case 'CLAS':
    // Classification
    ReadInt (&classification, stream);
    break;
  case 'USAG':
    // Usage
    ReadInt (&usage, stream);
    break;
  case '_SVH':
    // SVH - General Info filename
    ReadString (svhFilename, sizeof(svhFilename) - 1, stream);
    break;
  case '_GAS':
    // GAS - Fuel Systems filename
    ReadString (gasFilename, sizeof(gasFilename) - 1, stream);
    break;
  case '_WNG':
    // WNG - Wing aerodynamics filename
    ReadString (wngFilename, sizeof(wngFilename) - 1, stream);
    { // PHY file
      strncpy (phyFilename, wngFilename, 64);
      char *pch = NULL;
      pch = strstr (phyFilename, ".WNG");
      if (NULL == pch) 
       pch = strstr (phyFilename, ".wng");
      if (pch) strncpy (pch, ".phy", 4);
      DEBUGLOG ("search PHY file : -%s- for %s", phyFilename, wngFilename);
    }
    break;
  case '_PHY':
    ReadString (phyFilename, sizeof(phyFilename) - 1, stream);
    break;
  case '_AMP':
    // AMP - Electrical Subsystems filename
    ReadString (ampFilename, sizeof(ampFilename) - 1, stream);
    break;
  case '_PSS':
    // PSS - Pitot/Static filename
    ReadString (pssFilename, sizeof(pssFilename) - 1, stream);
    break;
  case '_WHL':
    // WHL - Ground Suspension filename
    ReadString (whlFilename, sizeof(whlFilename) - 1, stream);
    break;
  case '_VLD':
    // VLD - Variable Loadout filename
    ReadString (vldFilename, sizeof(vldFilename) - 1, stream);
    break;
  case '_PIT':
    // PIT - Cockpt Manager filename
    ReadString (pitFilename, sizeof(pitFilename) - 1, stream);
    break;
  case '_CAM':
    // CAM - Camera Manager filename
    ReadString (camFilename, sizeof(camFilename) - 1, stream);
    break;
  case '_LOD':
    // LOD - Level of Detail filename
    ReadString (lodFilename, sizeof(lodFilename) - 1, stream);
    break;
  case '_RDO':
    // RDO - Radio Manager filename
    ReadString (rdoFilename, sizeof(rdoFilename) - 1, stream);
    break;
  case '_ELT':
    // ELT - External Lights filename
    ReadString (eltFilename, sizeof(eltFilename) - 1, stream);
    break;
  case '_ENG':
    // ENG - Engine Manager filename
    ReadString (engFilename, sizeof(engFilename) - 1, stream);
    break;
  case '_MIX':
    // MIX - Control Mixer filename
    ReadString (mixFilename, sizeof(mixFilename) - 1, stream);
    break;
  case '_CKL':
    // CKL - Checklists filename 
    ReadString (cklFilename, sizeof(cklFilename) - 1, stream);
    break;
  case '_FCS':
    // FCS - Flight Control System filename
    ReadString (fcsFilename, sizeof(fcsFilename) - 1, stream);
    break;
  case '_SWD':
    // SWD - Slope Wind Data filename
    ReadString (swdFilename, sizeof(swdFilename) - 1, stream);
    break;
  case '_SIT':
    // SIT - Debug Situation filename
    ReadString (sitFilename, sizeof(sitFilename) - 1, stream);
    break;
  case '_PID':
    // PID Control systems
    ReadString (pidFilename, sizeof(pidFilename) - 1, stream);
    break;

  default:
    {
      WARNINGLOG ("CVehicleInfo::Read : Unknown tag %s", TagToString(tag));
    }
  }
  return rc;
}

char* CVehicleInfo::GetSVH (void) { return svhFilename; }
char* CVehicleInfo::GetGAS (void) { return gasFilename; }
char* CVehicleInfo::GetWNG (void) { return wngFilename; }
char* CVehicleInfo::GetAMP (void) { return ampFilename; }
char* CVehicleInfo::GetPSS (void) { return pssFilename; }
char* CVehicleInfo::GetWHL (void) { return whlFilename; }
char* CVehicleInfo::GetVLD (void) { return vldFilename; }
char* CVehicleInfo::GetPIT (void) { return pitFilename; }
char* CVehicleInfo::GetCAM (void) { return camFilename; }
char* CVehicleInfo::GetLOD (void) { return lodFilename; }
char* CVehicleInfo::GetRDO (void) { return rdoFilename; }
char* CVehicleInfo::GetELT (void) { return eltFilename; }
char* CVehicleInfo::GetENG (void) { return engFilename; }
char* CVehicleInfo::GetMIX (void) { return mixFilename; }
char* CVehicleInfo::GetCKL (void) { return cklFilename; }
char* CVehicleInfo::GetFCS (void) { return fcsFilename; }
char* CVehicleInfo::GetSWD (void) { return swdFilename; }
char* CVehicleInfo::GetSIT (void) { return sitFilename; }
char* CVehicleInfo::GetPID (void) { return pidFilename; }
char* CVehicleInfo::GetPHY (void) { return phyFilename; }

//===================END OF FILE ===================================================================