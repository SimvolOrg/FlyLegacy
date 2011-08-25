/*
 * EngineSubsystems.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
 * Copyright 2007-2009 Jean Sabatier
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

#include "../Include/Subsystems.h"
#include "../Include/Globals.h"
#include "../Include/AudioManager.h"
#include "../Include/Fui.h"
#include "../Include/Atmosphere.h"

//#ifdef _DEBUG	
#include "../Include/Utility.h"     
//#endif

using namespace std;
///////// DEBUG STUFF TO REMOVE LATER ////////////////////////////////
#ifdef  _DEBUG	
  //#define _DEBUG_STRT     //print lc DDEBUG file ... remove later
  //#define _DEBUG_MaGN     //print lc DDEBUG file ... remove later
  //#define FLYHAWK_TYPE    //select ignition      ... remove later 
  //#define MALIBU_TYPE     //select ignition      ... remove later 
#endif
//////////////////////////////////////////////////////////////////////
//===================================================================
//  Clear all data
//  NOTE:  mixt default value is 1 for aircraft that don't have
//         mixture lever.
//  Sound ratio is 0.8 at 2500 rpm
//===================================================================
CEngineData::CEngineData(CEngine *m)
{ meng      = m;                        // Mother engine
  startKit  = 0;                        // Starting conditions
  //----Overall state -------------------------
  e_stop    = 0;                        // Cause of stop
  e_state   = ENGINE_STOPPED;           // Engine state
  //--- Control value -------------------------
  c_mixt    = 0;       // used with engines without mixture knob
  e_thro    = 0;                        // Throttle control value
  e_revr    = 0;                        // Reverse control value
  e_mixt    = 0;                        // Mixture control value
  e_blad    = 0;                        // Blad from controler
  //----Output values -------------------------
  e_hob     = 0;                        // Hobb value
	e_Boost		= 0;												// Booster
  e_Map     = 0;           
  e_gph     = 0;                        // fuel requested (galon per hour)
  e_rpm     = 0;                        // Engine RPM
  e_vel     = 0;                        // Aircraft velocity
  e_wml     = 0;                        // Engine windmill
  e_oilT    = 0;                        // Oil temperature
  e_oilP    = 0;                        // Oil Presure
  e_EGTr    = 0;                        // Exhaust Gas Temp
  e_EGTa    = 0;
  e_CHTr    = 0;                        // Cylinder Head Temp
  e_CHTa    = 0;
  e_Thrust  = 0;                        // Engine trust
  e_Torque  = 0;                        // Engine torque
  e_HP      = 0;                        // Power
  e_af      = 0;                        // Air flow
  e_Pfac    = 0;
  //----Sound ratio ----------------------------------
  e_Srat    = 0.8f / 2500;             // Sound ratio
  //----Delta time -----------------------------------
  dT        = 0;
  //--------------------------------------------------
  e_hpaf    = 0;
  e_fttb    = 0;
  IAS       = 0;
	//--- Propellor ------------------------------------
	e_minRv		= 0;
	e_maxRv		= 0;
	e_pLift		= 0;
	e_pDrag		= 0;
	e_govrn		= 0;
  //-----Init constante TRI dtata --------------------
  ffsc      = 0;
  idle      = 0;
  rbhp      = 0;
  irpm      = 0;
  bost_alt  = 0;
  bost_pr   = 0;
  place_holder  = 0;

};
//-------------------------------------------------------------------
//  Return pitch from 0.4 to 1.2
//-------------------------------------------------------------------
float CEngineData::Pitch()
{ return (0.4f + (e_thro * 0.8));}
//===================================================================
//
// CEngineSubsystem
//===================================================================
CEngineSubsystem::CEngineSubsystem (void)
{ TypeIs (SUBSYSTEM_ENGINE_SUBSYSTEM);
  eNum    = 1; // default engine num
  eData   = 0;
}

//---------------------------------------------------------------------
//  Time slice
//---------------------------------------------------------------------
void CEngineSubsystem::TimeSlice (float dT,U_INT FrNo)			// JSDEV*
{ // Call parent class timeslice method
  CDependent::TimeSlice (dT,FrNo);								// JSDEV*
}

//==========================================================================
CEngineGauge::CEngineGauge (void)
{
  TypeIs (SUBSYSTEM_ENGINE_GAUGE);
}
//=========================================================================
// CEngineStatus
//========================================================================
CEngineStatus::CEngineStatus (void)
{
  TypeIs (SUBSYSTEM_ENGINE_STATUS);
  hwId  = HW_STATE;
  mode  = ENGINE_STOPPED;           // Runing by default
}
//--------------------------------------------------------------
//  Decode mode
//  Make a tag to send to engine
//--------------------------------------------------------------
char CEngineStatus::DecodeMode(char *md)
{ if (0 == strcmp(md,"OFF"))      return  ENGINE_STOPPED;
  if (0 == strcmp(md,"STOPPING")) return  ENGINE_STOPPING;
  if (0 == strcmp(md,"STARTING")) return  ENGINE_CRANKING;
  if (0 == strcmp(md,"MOTORING")) return  ENGINE_CATCHING;
  if (0 == strcmp(md,"RUNNING" )) return  ENGINE_RUNNING;
  return ENGINE_STOPPED;
}
//--------------------------------------------------------------
//  Read all parameters
//--------------------------------------------------------------
int CEngineStatus::Read (SStream *stream, Tag tag)
{  switch (tag) {
  case 'mode':
    // Engine mode being monitored
    char s[64];
    ReadString (s, 64, stream);
    mode = DecodeMode(s);
    return TAG_READ;
    }
    // See if the tag can be processed by the parent class type
    return CDependent::Read (stream, tag);
}
//--------------------------------------------------------------
//  Parameters are read
//--------------------------------------------------------------
void CEngineStatus::ReadFinished()
{ Tag eng             = 'Eng0' + eNum;
  emsg.group          = eng;
  emsg.sender         = unId;
  emsg.id             = MSG_GETDATA;
  emsg.dataType       = TYPE_INT;
  emsg.user.u.datatag = 'data';
  emsg.user.u.engine  = eNum;
}
//--------------------------------------------------------------
//  Receive a message
//--------------------------------------------------------------
void CEngineStatus::GetEngineState()
{ state  = 0;
  CEngineData *data = (CEngineData *)emsg.voidData;
  if (0 == data)
  { Send_Message(&emsg);
    data = (CEngineData *)emsg.voidData;
    if (0 == data)  return;
  }
  //---Update internal state ----------------
  char est = data->EngState();
  state    = (est == mode);
  active   = (state != 0);
  return;
}
//--------------------------------------------------------------
//  Receive a message
//--------------------------------------------------------------
EMessageResult CEngineStatus::ReceiveMessage(SMessage *msgr)
{ if (msgr->id != MSG_GETDATA) return CEngineControl::ReceiveMessage(msgr);
  switch (msgr->user.u.datatag) {
      case 'st8t':
      case 'actv':
        GetEngineState();                  // Poll engine for status
        msgr->intData = state;
        return MSG_PROCESSED;
  }
  return CEngineControl::ReceiveMessage(msgr);
}
//================================================================================
// CFuelGaugeSubsystem
//
CFuelGaugeSubsystem::CFuelGaugeSubsystem (void)
{
  TypeIs (SUBSYSTEM_FUEL_GAUGE);
}

//
// CFuelFlowSubsystem
//
CFuelFlowSubsystem::CFuelFlowSubsystem (void)
{
  TypeIs (SUBSYSTEM_FUEL_FLOW_GAUGE);
}

//
// CFuelPressureSubsystem
//
CFuelPressureSubsystem::CFuelPressureSubsystem (void)
{
  TypeIs (SUBSYSTEM_FUEL_PRESSURE_GAUGE);
}

//
// CN1Tachometer
//
CN1Tachometer::CN1Tachometer (void)
{
  TypeIs (SUBSYSTEM_N1_TACHOMETER);
}

//
// CN2Tachometer
//
CN2Tachometer::CN2Tachometer (void)
{
  TypeIs (SUBSYSTEM_N2_TACHOMETER);
}

//
// CTorqueMeter
//
CTorqueMeter::CTorqueMeter (void)
{
  TypeIs (SUBSYSTEM_TORQUE_METER);
}


//
// CInterTurbineTemp
//
CInterTurbineTemp::CInterTurbineTemp (void)
{
  TypeIs (SUBSYSTEM_INTERTURBINE_TEMP);

  degC = false;
}

int CInterTurbineTemp::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'degC':
    // Report temperature in degrees C
    degC = true;
    rc = TAG_READ;
    break;
  }

  return rc;
}


//
// CStarterIgniter
//
CStarterIgniter::CStarterIgniter (void)
{
  TypeIs (SUBSYSTEM_STARTER_IGNITER);
}

//
// CFireSystem
//
CFireSystem::CFireSystem (void)
{
  TypeIs (SUBSYSTEM_FIRE_SYSTEM);
}

//
// CFuelImbalance
//
CFuelImbalance::CFuelImbalance (void)
{
  TypeIs (SUBSYSTEM_FUEL_IMBALANCE);
}

//
// CFuelPumpControl
//
CFuelPumpControl::CFuelPumpControl (void)
{
  TypeIs (SUBSYSTEM_FUEL_PUMP_CONTROL);
}

//
// CCrossfeedControl
//
CCrossfeedControl::CCrossfeedControl (void)
{
  TypeIs (SUBSYSTEM_CROSSFEED_CONTROL);
}
