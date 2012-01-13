/*
 * ControlSubsystems.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005 Chris Wallace
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
#include "../Include/Ui.h"
#include "../Include/Utility.h" 
#include "../Include/Joysticks.h"
#include "../Include/Fui.h"
#include "../Include/AnimatedModel.h"
#include "../Include/Gauges.h"
using namespace std;
//=================================================================================
//  Table for Starter items
//  Each position gives a combination of magneto and starter position
//=================================================================================
U_INT rignTAB[] = {
    0,                        // 0 => OFF
    ENGINE_MAGN_01,           // 1 => MAGNETO_1
    ENGINE_MAGN_02,           // 2 => MAGNETO_2
    ENGINE_MAGN_12,           // 3 => BOTH MAGNETOS
    ENGINE_STR_ALL,           // 4 => BOTH MAGNETO + STARTER
};
//=================================================================================
///////// DEBUG STUFF TO REMOVE LATER ////////////////////////////////
#ifdef  _DEBUG	
  //#define _DEBUG_control       //print lc DDEBUG file ... remove later
  //#define _DEBUG_base_control  //print lc DDEBUG file ... remove later

#endif
//////////////////////////////////////////////////////////////////////

//=================================================================================
// CAeroControl
//  Provides factorization for all surfaces and equipement control (trim)
//=================================================================================
CAeroControl::CAeroControl (void)
{ TypeIs (SUBSYSTEM_AERO_CONTROL);
  hwId = HW_SWITCH;
  data.scal = 1.0f;
  data.step = 0.01f;
  data.bend = 0.0f;
  data.posn = 0.0f;

  // Initialize the normalized deflection values for bidirectional controls such
  //   as ailerons, rudder, and elevators. 
  data.minClamp = -1.0f;
  data.maxClamp = +1.0f;
  data.ampli    =  2.0f;
  // Initialize realtime control values
  data.raw      = 0;
  data.deflect  = 0;
  vPID          = 0;
  Bias          = 0;
  Cont          = 0;
	timer					= 0;
}
//---------------------------------------------------------------------------------
//  Read parameters for control
//---------------------------------------------------------------------------------
int CAeroControl::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'scal':
    // Scaling factor
    ReadFloat (&data.scal, stream);
    return TAG_READ;

  case 'step':
    // Control increment step
    ReadFloat (&data.step, stream);
    return TAG_READ;

  case 'bend':
    // Exponential
    ReadFloat (&data.bend, stream);
    return TAG_READ;

  case 'posn':
    // Default position
    ReadFloat (&data.posn, stream);
    return TAG_READ;

	case 'bias':
		ReadDouble(&Bias,stream);
		return TAG_READ;
  }

 // See if the tag can be processed by the parent class type
 return CDependent::Read (stream, tag);
}
//-----------------------------------------------------------------
//  All parameters are read
//-----------------------------------------------------------------
void CAeroControl::ReadFinished()
{ CDependent::ReadFinished();
  TagToString(data.chn,unId);
  return;
}
//-----------------------------------------------------------------
//  Base control receives a message
//  JS NOTE: Seems like flyII was sending scaled data for tag '101 '
//-----------------------------------------------------------------
EMessageResult CAeroControl::ReceiveMessage (SMessage *msg)
{
  switch (msg->id) {
    case MSG_GETDATA:
      switch (msg->user.u.datatag) {
				case 'norm':
        case 'rawv':
          //-- raw value ---------------------
          msg->realData = data.raw;
          return MSG_PROCESSED;
        case '101 ':
          //-- percentage value ---------------------
          msg->realData = data.scaled;
          return MSG_PROCESSED;
        case '102 ':
          msg->realData = indn;
          return MSG_PROCESSED;
        case '103 ':
          // Scaled deflection value
          msg->realData = data.scaled;
          return MSG_PROCESSED;
        case 'scal':
          // Scaling factor from normalized to absolute deflection
          msg->realData = data.scal;
          return MSG_PROCESSED;
        case 'minC':
          // Minimum clamp value
          msg->realData = data.minClamp;
          return MSG_PROCESSED;
        case 'maxC':
          // Maximum clamp value
          msg->realData = data.maxClamp;
          return MSG_PROCESSED;
        case 'step':
          // step value
          msg->realData = data.step;
          return MSG_PROCESSED;        
        case 'data':
          data.deflect  = indn;
          msg->voidData = &data;
          return MSG_PROCESSED;
          }
    case MSG_SETDATA:
      switch (msg->user.u.datatag) {
        //----Autopilot -------------------
        case 'vpid':
          vPID = (float)msg->realData;
          return MSG_PROCESSED;
          //---Increment by step ----------
        case 'incr':
          Incr();
          msg->realData = data.raw;
          return MSG_PROCESSED;
          //---Decrement by step ----------
        case 'decr':
          Decr();
          msg->realData = data.raw;
          return MSG_PROCESSED;
        }
    }

   // See if the message can be processed by the parent class
   return CDependent::ReceiveMessage (msg);
}
//--------------------------------------------------------------------
// Clamp all values
//--------------------------------------------------------------------
inline float CAeroControl::Clamp(float v)
{ if (v < data.minClamp) return data.minClamp;
  if (v > data.maxClamp) return data.maxClamp;
  return v;
}
//--------------------------------------------------------------------
// Increment the default normalized control value by the step size
//--------------------------------------------------------------------
void CAeroControl::Incr (void)
{ data.raw  = Clamp(data.raw + data.step);
  if (0 == Cont)      return;
  //----Set Bias to associated control ----------------
}

//--------------------------------------------------------------------
// Decrement the default normalized control value by the step size
//--------------------------------------------------------------------
void CAeroControl::Decr (void)
{ data.raw  = Clamp(data.raw - data.step);
  if (0 == Cont)    return;
}
//-----------------------------------------------------------------------
// Set the raw control value
//----------------------------------------------------------------------
void CAeroControl::SetValue (float fv)
{ data.raw = fv;
}
//--------------------------------------------------------------------
// Modify the value
//--------------------------------------------------------------------
void CAeroControl::Modify(float dt)
{ data.raw  +=  dt;
  data.raw   =  Clamp(data.raw);
}
//--------------------------------------------------------------------
// Transfert autopilot value and clear auto value
//--------------------------------------------------------------------
void CAeroControl::Transfer()
{	data.raw	= vPID;
	vPID			= 0;
	return;
}
//--------------------------------------------------------------------
//  Edit controller data
//--------------------------------------------------------------------
void CAeroControl::Probe(CFuiCanva *cnv)
{ CDependent::Probe(cnv,0);
  cnv->AddText( 1,1,"raw... %.5f",data.raw);
  cnv->AddText( 1,1,"dflect %.5f",data.deflect);
  cnv->AddText( 1,1,"vpid.. %.5f",vPID);
  cnv->AddText( 1,1,"Bias.. %.5f", Bias);
  return;
}

//-----------------------------------------------------------------------
//  Compute scaled value
//  Autopilot action (vPID) is integrated here
//-----------------------------------------------------------------------
void CAeroControl::TimeSlice (float dT,U_INT FrNo)				// JSDEV*
{ CDependent::TimeSlice (dT,FrNo);								        // JSDEV*
  indnTarget = Clamp(data.raw + vPID + Bias);
  return;
}

//=================================================================================
// CAileronControl
//=================================================================================
CAileronControl::CAileronControl (void)
{
  TypeIs (SUBSYSTEM_AILERON_CONTROL);
}
//---------------------------------------------------------------------------------
//  Integrate pilot move and autopilot ?
//---------------------------------------------------------------------------------
void CAileronControl::TimeSlice (float dT,U_INT FrNo)			
{ globals->jsm->Poll(JS_AILERON, data.raw);
  CAeroControl::TimeSlice(dT,FrNo);								
}
//===========================================================================
// CElevatorControl
//===========================================================================
CElevatorControl::CElevatorControl (void)
{ TypeIs (SUBSYSTEM_ELEVATOR_CONTROL);
}
//---------------------------------------------------------------------------------
//  Set value from joystick
//---------------------------------------------------------------------------------
void CElevatorControl::TimeSlice (float dT,U_INT FrNo)			// JSDEV*
{ globals->jsm->Poll(JS_ELEVATOR,data.raw);
  CAeroControl::TimeSlice(dT,FrNo);								
}
//================================================================================
// CRudderControl
//================================================================================
CRudderControl::CRudderControl (void)
{
  TypeIs (SUBSYSTEM_RUDDER_CONTROL);
  oADJ  = 1;
  macrd  = 0;
}
//--------------------------------------------------------------------------------
//  Adjust the raw value by banking factor if one is defined
//--------------------------------------------------------------------------------
void CRudderControl::Adjust()
{ float bank = -globals->dang.y;
  data.raw  += (macrd->Lookup(bank) * oADJ);
}
//---------------------------------------------------------------------------------
//  Integrate pilot move
//            autopilot
//            Rudder deflection versus banking
//---------------------------------------------------------------------------------
void CRudderControl::TimeSlice (float dT,U_INT FrNo)			// JSDEV*
{ globals->jsm->Poll(JS_RUDDER,data.raw);
  //-----Adjust the value before scaling --------------------------
  if (macrd)  Adjust();
  CAeroControl::TimeSlice(dT,FrNo);								// JSDEV*
}
//--------------------------------------------------------------------------------
//  Adjust the bias value 
//--------------------------------------------------------------------------------
void CRudderControl::ModBias(float v)
{	Bias += v;
	if (Bias > +1)	Bias = +1;
	if (Bias < -1)	Bias = -1;
}
//=============================================================================
// CEngineControl provides factorization for all engines controls
//==============================================================================
CEngineControl::CEngineControl()
{ TypeIs(SUBSYSTEM_ENGINE_SUBSYSTEM);
  step  = float(0.1);
}
//-------------------------------------------------------------------------
// Read control parameters
//-------------------------------------------------------------------------
int CEngineControl::Read (SStream *stream, Tag tag)
{
  switch (tag) {
  case 'step':
    ReadFloat(&step,stream);
      return TAG_READ;
    case 'degF':
      conv = 0;
      return TAG_READ;
    case 'degC':
      conv = 1;
      return TAG_READ;

  }
  // See if the tag can be processed by the parent class type
  return CDependent::Read (stream, tag);
}
//-------------------------------------------------------------------------
//  All parameters are read.  Set engine message
//-------------------------------------------------------------------------
void CEngineControl::ReadFinished()
{ CDependent::ReadFinished();
  engm.group  = 'Eng0' + eNum;               // Send to 'Engx'
  engm.id     = MSG_SETDATA;
  engm.user.u.engine  = eNum;
  return;
}
//-------------------------------------------------------------------------
//  All parameters are read.  Set engine message
//-------------------------------------------------------------------------
void CEngineControl::Monitor(Tag tag)
{ engm.id       = MSG_GETDATA;
  engm.dataType = TYPE_REAL;
  engm.user.u.datatag = tag;
  engm.realData = 0;
  //---Active by default ----------------------
  state = 1;
  return;
}
//-------------------------------------------------------------------------
//	Specific Identification needed for engine control
//  Engine and optionaly unit must match
//  For mono engine default unit is always 1
//-------------------------------------------------------------------------
bool CEngineControl::MsgForMe (SMessage *msg)
{ bool me = false;
  if (msg) {
    bool matchGroup = (msg->group == unId);
    bool engnNull   = (msg->user.u.engine == 0);
    bool engnMatch  = (msg->user.u.engine == eNum);
    bool unitNull   = (msg->user.u.unit   == 0);
    bool unitMatch  = (msg->user.u.unit   == uNum);
    me = (matchGroup && (engnNull || engnMatch) && (unitNull || unitMatch));
  }

  return me;
}
//-------------------------------------------------------------------------
//  Receive a message
//-------------------------------------------------------------------------
EMessageResult CEngineControl::ReceiveMessage (SMessage *msg)
{ switch (msg->id) {
      case MSG_GETDATA:
        return CDependent::ReceiveMessage(msg);
      // --- Set state from outside (gauge, etc) ----
      case MSG_SETDATA:
        switch (msg->user.u.datatag) {
          //---Increment value ----------
          case 'incr':
            Incr();
            engm.realData = indnTarget;
            Send_Message(&engm);
            return MSG_PROCESSED;
          //---Decrement value ----------
          case 'decr':
            Decr();
            engm.realData = indnTarget;
            Send_Message(&engm);
            return MSG_PROCESSED;
        }
    }
   return CDependent::ReceiveMessage (msg);
}
//-------------------------------------------------------------------------
//  Poll engine with message
//-------------------------------------------------------------------------
void CEngineControl::PollEngine(U_CHAR c)
{ Send_Message(&engm);
  indnTarget = (active)?(engm.realData):(0);
  if (0 == c)     return;
  indnTarget = DEGRE_FROM_FAHRENHEIT(indnTarget);
  return;
}

//============================================================================
// CEngine Proxy: just a representative of the engine in AMP systems
//  NOTE: All messages addressed to the proxy are rerouted to the
//  real engine.
//============================================================================
CEngineProxy::CEngineProxy()
{ TypeIs (SUBSYSTEM_ENGINE_PROXY);
}
//--------------------------------------------------------------------
//  read all parameters 
//--------------------------------------------------------------------
int CEngineProxy::Read(SStream *s, Tag tag)
{ switch (tag)  {
  //---Don't know this one -----------------------
  case 'nSim':
     return TAG_READ;
  }
  return CEngineControl::Read(s,tag);
}
//--------------------------------------------------------------------
//  Receive a message
//  Just reroute message to real engine
//--------------------------------------------------------------------
EMessageResult CEngineProxy::ReceiveMessage (SMessage *msg)
{ msg->receiver = 0;
  msg->sender   = unId;
  msg->group    = 'Eng0' + eNum;                  // to  (real engine)
  msg->user.u.engine = eNum;                      // Engine number
  Send_Message(msg);
  return MSG_PROCESSED;
}
//============================================================================
// CPrimeControl: Control prime pumps
//============================================================================
CPrimeControl::CPrimeControl (void)
{ TypeIs (SUBSYSTEM_PRIME_CONTROL);
  hwId = HW_GAUGE;
}
//-------------------------------------------------------------------------
//  All parameters are read.  Init messages
//-------------------------------------------------------------------------
void CPrimeControl::ReadFinished()
{ CEngineControl::ReadFinished();
  //----Init magneto message to engine susbsystem ------------
  engm.dataType   = TYPE_INT;
  engm.user.u.datatag = 'prim';      // prime tag
  //----Compute state mask -----------------------------------
  mask[0] = 0;
  mask[1] = ENGINE_PRIMER;
  return;
}
//--------------------------------------------------------------------
//  Receive a message
//  When state change, send change to engine
//--------------------------------------------------------------------
EMessageResult CPrimeControl::ReceiveMessage (SMessage *msg)
{ switch (msg->id) {
      case MSG_GETDATA:
        switch (msg->user.u.datatag) {
        case 'st8t':
          msg->intData = state;
          return MSG_PROCESSED;
        }
      break;
      // --- Set state from outside (gauge, etc) ----
      case MSG_SETDATA:
        switch (msg->user.u.datatag) {
          case 'st8t':
            state = (0 != msg->intData);
            engm.intData = mask[state];
            Send_Message(&engm);
            return MSG_PROCESSED;
          }
        break;
        }
   // See if the message can be processed by the parent class
   return CEngineControl::ReceiveMessage (msg);
}
//===================================================================================
// CRotaryIgnitionSwitch from CEngineControl
//
//  typedef enum {
//    MAGNETO_SWITCH_OFF    = 0,
//    MAGNETO_SWITCH_RIGHT  = 1,
//    MAGNETO_SWITCH_LEFT   = 2,
//    MAGNETO_SWITCH_BOTH   = 3,
//    MAGNETO_SWITCH_START  = 4
//  } ERotaryPosition;
//===================================================================================
CRotaryIgnitionSwitch::CRotaryIgnitionSwitch (void)
{ TypeIs (SUBSYSTEM_ROTARY_IGNITION_SWITCH);
  hwId = HW_SWITCH;
  rot_pos = MAGNETO_SWITCH_OFF;
  sAmp = 1;
}
//-------------------------------------------------------------------------
//  Read all paramters
//-------------------------------------------------------------------------
int CRotaryIgnitionSwitch::Read (SStream *stream, Tag tag)
{ switch (tag) {
    case 'sAmp':
      ReadFloat (&sAmp, stream);
      return TAG_READ;
    }
 return  CEngineControl::Read (stream, tag);
}
//-------------------------------------------------------------------------
//  All parameters are read.  Init messages
//-------------------------------------------------------------------------
void CRotaryIgnitionSwitch::ReadFinished()
{ CEngineControl::ReadFinished ();
  //---Init engine message ----------------------------------
  engm.dataType   = TYPE_INT;
  engm.user.u.datatag = 'rign';             // Set magneto
  //---Init poll message -------------------------------------
  meng.group  = 'Eng0'| eNum;               // Send to 'Eng1'
  meng.sender = unId;
  meng.id     = MSG_GETDATA;
  meng.user.u.engine  = eNum;
  meng.user.u.datatag = 'runs';             // Poll for state
  //---Active by default ----------------------
  state = 1;
}
//-------------------------------------------------------------------------
//  Process message from gauge
//  Propagate to the engine with correct magneto setting
//-------------------------------------------------------------------------
EMessageResult CRotaryIgnitionSwitch::ReceiveMessage (SMessage *msg)
{   int pos = rot_pos;
    int msk = (active)?(ENGINE_STR_ALL):(0);
    switch (msg->id) {
        case MSG_GETDATA:
					switch (msg->user.u.datatag) {
						case 'gets':
							msg->voidData = this;
							return MSG_PROCESSED;
						case 'indx':
							{	if (msg->dataType == TYPE_INT)  msg->intData  = int (pos);
								if (msg->dataType == TYPE_REAL) msg->realData = double(pos);
								return MSG_PROCESSED;
							}
					} 
      //----Message from gauge ------------------------------
        case MSG_SETDATA:
          { rot_pos = (EMagnetoSwitch) msg->intData;
            engm.intData = rignTAB[rot_pos] & msk;
            Send_Message(&engm);
            return MSG_PROCESSED;
          }
    }
  //----Ignore the message ---------------------------------
  return MSG_IGNORED;
  }
//------------------------------------------------------------------------
//  Switch back when engine is runing
//  Check for engine runing state
//  - Set a rest mask just in case
//-------------------------------------------------------------------------
void CRotaryIgnitionSwitch::TimeSlice (float dT,U_INT FrNo)			// JSDEV*
{ CEngineControl::TimeSlice(dT,FrNo);  
	if (rot_pos != MAGNETO_SWITCH_START)	return;
  meng.intData = ENGINE_STR_ALL;        
  Send_Message(&meng);
	if (meng.intData != 1)								return;
	rot_pos = MAGNETO_SWITCH_BOTH;
  engm.intData = rignTAB[rot_pos];
  Send_Message(&engm);
  return;
}
//------------------------------------------------------------------------
//	Probe position
//------------------------------------------------------------------------
void CRotaryIgnitionSwitch::Probe(CFuiCanva *cnv)
{	CDependent::Probe(cnv,0);
	cnv->AddText(1,1,"Position:%d",rot_pos);
	return;
}
//==============================================================================
// CMagnetoControl 'neto'  from CEngineControl
//  Magneto control has attributes that define
//  eNum: Corresponding engine
//  uNum: Magneto unit numer 1 or 2
//==============================================================================
CMagnetoControl::CMagnetoControl (void)
{
  TypeIs (SUBSYSTEM_MAGNETO_CONTROL);
  hwId = (HW_SWITCH);
}
//------------------------------------------------------------------
//  All parameters are read
//------------------------------------------------------------------
void CMagnetoControl::ReadFinished()
{ CEngineControl::ReadFinished();
  //----Compute magneto number -------------------------------
  NoMag = 'mag0' + uNum;    // mag1 or mag2
  //----Init magneto message to engine susbsystem ------------
  engm.dataType   = TYPE_INT;
  engm.user.u.datatag = NoMag;      // Magneto number
  //----Compute state mask -----------------------------------
  mask[0] = 0;
  mask[1] = ENGINE_MAGN_12;
  return;
}
//-------------------------------------------------------------------------
//  Process Message
//  Set messages are propagated to the corresponding engine
//-------------------------------------------------------------------------
EMessageResult CMagnetoControl::ReceiveMessage (SMessage *msg)
{
  switch (msg->id) {
      case MSG_GETDATA:
        switch (msg->user.u.datatag) {
        case 'st8t':
            msg->intData = state;
            msg->user.u.unit = uNum;
            return MSG_PROCESSED;
        }
  
      case MSG_SETDATA:
        switch (msg->user.u.datatag) {
          //-- Set State --(send to engine)----------
          case 'st8t':
              state = (msg->intData == 1);
              engm.intData = mask[state];
              Send_Message(&engm);
              return MSG_PROCESSED;
          }
  }
     return CEngineControl::ReceiveMessage (msg);
}
//==================================================================================
#define STARTER_TIMER (6)
//==================================================================================
// CStarterControl  from CEngineControl
//  Simple ON/OFF switch with temporization to start engine
//==================================================================================
CStarterControl::CStarterControl (void)
{ TypeIs (SUBSYSTEM_STARTER_CONTROL);
  hwId  = HW_SWITCH;
  mTerm = false;
  time  = 0;
}
//--------------------------------------------------------------------
//  read parameters
//--------------------------------------------------------------------
int CStarterControl::Read (SStream *stream, Tag tag)
{
  switch (tag) {
  case 'manl':
    // Manual starter terminate
    mTerm = true;
    return TAG_READ;
  }
  return CEngineControl::Read (stream, tag);
}
//--------------------------------------------------------------------
//  all parameters are read
//--------------------------------------------------------------------
void CStarterControl::ReadFinished()
{ CEngineControl::ReadFinished();
  //----Init starter message to engine susbsystem ------------
  engm.dataType   = TYPE_INT;
  engm.user.u.datatag = 'strt';     // Starter
  state           = 0;
  //--- Build ON/OFF mask ------------------------------------
  mask[0]   = 0;      // OFF
  mask[1]   = ENGINE_STARTER;
  return;
}
//--------------------------------------------------------------------
//  Timer is override when
//  -autoamtic termination is used
//  - The status was on
//--------------------------------------------------------------------
void CStarterControl::ArmTimer(SMessage *msg)
{ msg->intData  = 0;
  if (0 == state)   return;
  if (mTerm)        return;
  msg->intData  = STARTER_TIMER;
  time          = globals->clk->GetActual() + STARTER_TIMER;
  return;
}
//--------------------------------------------------------------------
//  Receive a message
//  When state change, send change to engine
//--------------------------------------------------------------------
EMessageResult CStarterControl::ReceiveMessage (SMessage *msg)
{ switch (msg->id) {
      case MSG_GETDATA:
        switch (msg->user.u.datatag) {
        case 'st8t':
          msg->intData = state;
          return MSG_PROCESSED;
        }
      break;
      // --- Set state from outside (gauge, etc) ----
      case MSG_SETDATA:
        switch (msg->user.u.datatag) {
          case 'st8t':
            state = (0 != msg->intData);
            engm.intData = mask[state];
            Send_Message(&engm);
            //--Update timer override if needed ----
            ArmTimer(msg);
            return MSG_PROCESSED;
          }
        break;
        }
   // See if the message can be processed by the parent class
   return CEngineControl::ReceiveMessage (msg);
}
//--------------------------------------------------------------------
//  Starter time slice
//--------------------------------------------------------------------
void CStarterControl::TimeSlice(float dT, U_INT FrNo)
{ if (0 == time)        return;
  U_INT now = globals->clk->GetActual();
  if (now < time)       return;
  //------Set state to OFF ---------------------------
  time  = 0;
  state = 0;
  //---- Send to engine ------------------------------
  engm.intData = mask[state];
  Send_Message(&engm);
  return;
}
//============================================================================
// CIgniterControl
//===========================================================================
CIgniterControl::CIgniterControl (void)
{ TypeIs (SUBSYSTEM_IGNITER_CONTROL);
  hwId  = HW_SWITCH;
}
//--------------------------------------------------------------------
//  all parameters are read
//--------------------------------------------------------------------
void CIgniterControl::ReadFinished()
{ CEngineControl::ReadFinished();
  //----Init starter message to engine susbsystem ------------
  engm.dataType   = TYPE_INT;
  engm.user.u.datatag = 'ignr';     // Starter
  state           = 0;
  //--- Build ON/OFF mask ------------------------------------
  mask[0]   = 0;      // OFF
  mask[1]   = ENGINE_IGNITER;
  return;
}
//--------------------------------------------------------------------
//  Receive a message
//  When state change, send change to engine
//--------------------------------------------------------------------
EMessageResult CIgniterControl::ReceiveMessage (SMessage *msg)
{ switch (msg->id) {
      case MSG_GETDATA:
        switch (msg->user.u.datatag) {
        case 'st8t':
          msg->intData = state;
          return MSG_PROCESSED;
        }
      break;
      // --- Set state from outside (gauge, etc) ----
      case MSG_SETDATA:
        switch (msg->user.u.datatag) {
          case 'st8t':
            state = (0 != msg->intData);
            engm.intData = mask[state];
            Send_Message(&engm);
            return MSG_PROCESSED;
          }
        break;
        }
   // See if the message can be processed by the parent class
   return CEngineControl::ReceiveMessage (msg);
}
//=========================================================PrepareMessage====================
// CThrottleControl  from CEngineControl
//==============================================================================
CThrottleControl::CThrottleControl (void)
{ TypeIs (SUBSYSTEM_THROTTLE_CONTROL); 
  hwId  = HW_OTHER;
	data	= 0;
}
//-----------------------------------------------------------------------
//  All parameter are read. remap joystick to me
//-----------------------------------------------------------------------
void CThrottleControl::ReadFinished()
{ CEngineControl::ReadFinished();
  //--- Map to joystick -----------------------
  Tag jt = JS_THROTTLE_0 + eNum;
  globals->jsm->MapTo(jt,unId);
  //--- Init engine message -------------------
  engm.dataType   = TYPE_REAL;
  engm.user.u.datatag = 'thro';     // Starter
  return;
}
//-----------------------------------------------------------------------
//  Poll engine to get data interface
//-----------------------------------------------------------------------
void CThrottleControl::PrepareMsg(CVehicleObject *veh)
{	SMessage mpol = engm;
	mpol.id  = MSG_GETDATA;
	mpol.dataType	= TYPE_VOID;
	mpol.user.u.datatag = 'data';
	Send_Message(&mpol);
	data	= (CEngineData*) mpol.voidData;
	return;
}
//-----------------------------------------------------------------------
//  Process message
//-----------------------------------------------------------------------
EMessageResult  CThrottleControl::ReceiveMessage (SMessage *msg)
{ switch (msg->id) {
      case MSG_GETDATA:
        switch (msg->user.u.datatag) {
          case 'indn' :
          case 'thro' :
            msg->realData = indn;
            return MSG_PROCESSED;
					//--- controller address
					case 'gets':
						msg->voidData = this;
						return MSG_PROCESSED;

          }
  
      case MSG_SETDATA:
          switch (msg->user.u.datatag) {
            case 'indn' :
            case 'thro' :
              { float val     = float(msg->realData);
                indnTarget    = val;
								data->e_thro  = val;
                return MSG_PROCESSED;
              }
          }
    }
  return CEngineControl::ReceiveMessage (msg);
}
//-----------------------------------------------------------------------
//  Set target Value
//-----------------------------------------------------------------------
void CThrottleControl::Target(float val)
{	 indnTarget    = val;
	 data->e_thro  = indn;
}
//===========================================================================
// CThrustReverseControl
//	TODO:  Interface with CEngine throught Engine data
//===========================================================================
CThrustReverseControl::CThrustReverseControl (void)
{ TypeIs (SUBSYSTEM_THRUST_REVERSE_CONTROL);
  hwId  = HW_OTHER;
  dTime = 0;
}
//----------------------------------------------------------------
//  Read Parameters
//----------------------------------------------------------------
int CThrustReverseControl::Read (SStream *stream, Tag tag)
{ switch (tag) {
    case 'dTim':
      // Deployment time
      ReadFloat (&dTime, stream);
      return TAG_READ;
    }
  return CEngineControl::Read (stream, tag);
}
//-----------------------------------------------------------------------
//  All parameter are read. remap joystick to me
//  NOTE: Ned to add a joystick entry for reverser
//-----------------------------------------------------------------------
void CThrustReverseControl::ReadFinished()
{ CEngineControl::ReadFinished();
  //--- TODO Map to joystick -----------------------
  //--- Init engine message -------------------
  engm.dataType   = TYPE_REAL;
  engm.user.u.datatag = 'revr';     // Reverse
  return;
}
//-----------------------------------------------------------------------
//  Process message
//-----------------------------------------------------------------------
EMessageResult  CThrustReverseControl::ReceiveMessage (SMessage *msg)
{ switch (msg->id) {
      case MSG_GETDATA:
        switch (msg->user.u.datatag) {
          case 'indn' :
          case 'revr' :
            msg->realData = indn;
            return MSG_PROCESSED;
          }
  
      case MSG_SETDATA:
          switch (msg->user.u.datatag) {
            case 'indn' :
            case 'revr' :
              { float val     = float(msg->realData);
                indnTarget    = val;
                engm.realData = val;
                return MSG_PROCESSED;
              }
          }
    }
  return CEngineControl::ReceiveMessage (msg);
}
//======================================================================
// CMixtureControl  from CEngineControl
//======================================================================
CMixtureControl::CMixtureControl (void)
{ TypeIs (SUBSYSTEM_MIXTURE_CONTROL);
  hwId  = HW_OTHER;
  //---------------------------------------------
	data	= 0;
  autoControl = false;
  injd = false;
  prmr = 1.0f;
  prff = 0.0f;
  plim = 0.0f;
}
//-----------------------------------------------------
//  Read Parameters
//------------------------------------------------------
int CMixtureControl::Read (SStream *stream, Tag tag)
{
  switch (tag) {
  case 'auto':
    // Control is capable of automatic control
    autoControl = true;
    return TAG_READ;

  case 'injd':
    // Engine is fuel injected (no carbeurator)
    injd = true;
    return TAG_READ;

  case 'prmr':
    // Priming rate
    ReadFloat (&prmr, stream);
    return TAG_READ;

  case 'prff':
    // Priming fuel flow (pph)
    ReadFloat (&prff, stream);
    return TAG_READ;

  case 'plim':
    // Priming pump limit
    ReadFloat (&plim, stream);
    return TAG_READ;
  }

  // See if the tag can be processed by the parent class type
  return CEngineControl::Read (stream, tag);
}
//-----------------------------------------------------
//  All parameters are read.  Map joystick to me
//------------------------------------------------------
void CMixtureControl::ReadFinished()
{ CEngineControl::ReadFinished();
  Tag jt = JS_MIXTURE_0 + eNum;
  globals->jsm->MapTo(jt,unId);
  //--- Init engine message -------------------
  engm.dataType   = TYPE_REAL;
  engm.user.u.datatag = 'mixt';     // Starter
  return;
}
//-----------------------------------------------------------------------
//  Poll engine to get data interface
//-----------------------------------------------------------------------
void CMixtureControl::PrepareMsg(CVehicleObject *veh)
{	SMessage mpol = engm;
	mpol.id  = MSG_GETDATA;
	mpol.dataType	= TYPE_VOID;
	mpol.user.u.datatag = 'data';
	Send_Message(&mpol);
	data	= (CEngineData*) mpol.voidData;
	return;
}
//-----------------------------------------------------
//  Set control to value
//------------------------------------------------------
int  CMixtureControl::SetControl(float val)
{	indnTarget    = val;
  data->e_mixt  = val;
	return 1;
}
//-----------------------------------------------------
//  Receive message
//------------------------------------------------------
EMessageResult  CMixtureControl::ReceiveMessage (SMessage *msg)
{ float val = 0;
  switch (msg->id) {
      case MSG_GETDATA:
        switch (msg->user.u.datatag) {
          case 'indn' :
          case 'mixt' : // 'mixt' :
            msg->realData = indn;
            return MSG_PROCESSED;
          }
			//--- Message from joystick or external controls ----
      case MSG_SETDATA:
        switch (msg->user.u.datatag) {
					//--- Set mixture value -------------------
          case 'mixt' : // 'mixt' :
              { if (msg->dataType == TYPE_REAL) val = float(msg->realData);
                if (msg->dataType == TYPE_INT)  val = float(msg->intData);
								SetControl(val);
								return MSG_PROCESSED;
              }
					//--- Set mixture full rich ---------------
					case 'amfr':
						SetControl(1);
						return MSG_PROCESSED;
					//--- Set mixture full lean ---------------
					case 'amfl':
						SetControl(0);
						return MSG_PROCESSED;
					//-- Increase value -----------------------
					case 'incr':
            Incr();
            data->e_mixt  = indn;
            return MSG_PROCESSED;
          //---Decrement value ----------
          case 'decr':
            Decr();
            data->e_mixt  = indn;
            return MSG_PROCESSED;

					//--- change state ------------------------
          case 'st8t' :
            //--- 0 or 1 value ----------------------
            float val     = float(msg->intData);
            indnTarget    = val;
            engm.realData = val;
            Send_Message(&engm);
            return MSG_PROCESSED;
        }
    }
 
  return CEngineControl::ReceiveMessage (msg);
}

//=======================================================================
// CPropellerControl
//=======================================================================
CPropellerControl::CPropellerControl (void)
{ TypeIs (SUBSYSTEM_PROPELLER_CONTROL);
  hwId  = HW_OTHER;
	data  = 0;
  minRPM = maxRPM = 0;
  governorGain    = 0;
  autoFeather     = false;
}
//------------------------------------------------------------------------
//  Read Parameters
//------------------------------------------------------------------------
int CPropellerControl::Read (SStream *stream, Tag tag)
{switch (tag) {
  case 'minr':
    ReadFloat (&minRPM, stream);
    return TAG_READ;
  case 'maxr':
    ReadFloat (&maxRPM, stream);
    return TAG_READ;
  case 'gvnr':
    ReadFloat (&governorGain, stream);
    return TAG_READ;
  case 'afet':
    autoFeather = true;
    return TAG_READ;
  }
  return CEngineControl::Read (stream, tag);
}
//------------------------------------------------------------------------
//  All parameters are read
//------------------------------------------------------------------------
void CPropellerControl::ReadFinished()
{ CEngineControl::ReadFinished();
  Tag jt = JS_PROP_0 + eNum;
  globals->jsm->MapTo(jt,unId);
  //--- Init engine message -------------------
  engm.dataType   = TYPE_REAL;
  engm.user.u.datatag = 'blad';     // blade tag
  return;
}
//-----------------------------------------------------------------------
//  Poll engine to get data interface
//-----------------------------------------------------------------------
void CPropellerControl::PrepareMsg(CVehicleObject *veh)
{	SMessage mpol = engm;
	mpol.id  = MSG_GETDATA;
	mpol.dataType	= TYPE_VOID;
	mpol.user.u.datatag = 'data';
	Send_Message(&mpol);
	data	= (CEngineData*) mpol.voidData;
	data->e_maxRv = maxRPM;
	data->e_minRv	= minRPM;
	data->e_govrn	= governorGain;
	return;
}
//------------------------------------------------------------------------
//  Process message
//------------------------------------------------------------------------
EMessageResult  CPropellerControl::ReceiveMessage (SMessage *msg)
{ switch (msg->id) {
    case MSG_GETDATA:
      msg->dataType = TYPE_REAL;
      switch (msg->user.u.datatag) {
        case 'indn' :
        case 'blad' :
          msg->realData = indn;
          return MSG_PROCESSED;
        }

    case MSG_SETDATA:
      switch (msg->user.u.datatag) {
        case 'indn' :
        case 'blad' :
          {float val     = float(msg->realData);
           indnTarget    = val;
					 indn					 = val;
           data->e_blad  = val;
           return MSG_PROCESSED;
          }
				//-- Increase value -----------------------
				case 'incr':
          Incr();
          data->e_blad  = indn;
          return MSG_PROCESSED;
        //---Decrement value ----------
        case 'decr':
          Decr();
          data->e_blad  = indn;
          return MSG_PROCESSED;
      }
  }
  return CEngineControl::ReceiveMessage (msg);
}
//============================================================================
// JSDEV* CTachometer  from CEngineControl
//	Monitors engine RPM
//============================================================================
CTachometer::CTachometer (void)
{	timK		= 0.1f;				// add small delay
	indnMode	= INDN_LINEAR;
  TypeIs (SUBSYSTEM_TACHOMETER);
}
//-----------------------------------------------------------------
//  All parameters read. Init engine message
//-----------------------------------------------------------------
void CTachometer::ReadFinished()
{ CEngineControl::ReadFinished();
  //---Init all engine messages ------------------
  Tag eng = 0;
  int eno;
  for (int k=0; k < ENGINE_MAX; k++)
  { SMessage *msg = emsg  + k;
    msg->sender         = unId;
    msg->id             = MSG_GETDATA;
    msg->dataType       = TYPE_REAL;
    eno = k + 1;
    eng = 'Eng0' + eno;
    msg->group          = eng;
    msg->user.u.datatag = 'erpm';
    msg->user.u.engine  =  eno;
  }
  Monitor('erpm');
  return;
}
//-----------------------------------------------------------------
//	Check for RPM request
//-----------------------------------------------------------------
bool CTachometer::GetRPM(SMessage *msg)
{ Tag engn  = msg->user.u.datatag & 0x00FF;
  if ((engn < '1') || (engn > '8')) return false;
  int No    = engn - '1';
  SMessage *ems = emsg + No;
  Send_Message(ems);
  msg->realData = ems->realData;
  return true;
}
//-----------------------------------------------------------------
//	Receive message
//-----------------------------------------------------------------
EMessageResult  CTachometer::ReceiveMessage (SMessage *msg)
{ switch (msg->id) {
    case MSG_GETDATA:
      if (GetRPM(msg))  return MSG_PROCESSED;
      PollEngine(0);
      switch (msg->user.u.datatag) {
       case 'indn':
       case 'rpmn': 
          msg->realData = indn;
          return MSG_PROCESSED;
      }
  }
  return CEngineControl::ReceiveMessage (msg);
}
//============================================================================
// CTachometerTimer
//============================================================================
CTachometerTimer::CTachometerTimer (void)
{ TypeIs (SUBSYSTEM_TACHOMETER_TIMER);
}
//-----------------------------------------------------------------
//  All parameters read. Init engine message
//-----------------------------------------------------------------
void CTachometerTimer::ReadFinished()
{ CEngineControl::ReadFinished();
  Monitor('Hobm');
  return;
}
//-----------------------------------------------------------------
//	Receive message
//-----------------------------------------------------------------
EMessageResult  CTachometerTimer::ReceiveMessage (SMessage *msg)
{ switch (msg->id) {
    case MSG_GETDATA:
      PollEngine(0);
      msg->realData = indn;
      return MSG_PROCESSED;

    case MSG_SETDATA:
      // None
      break;
    }
 return CEngineControl::ReceiveMessage (msg);
}
//===========================================================================
// JSDEV*: COilTemperature  from CEngineControl
//  Monitors engine temperature
//===========================================================================
COilTemperature::COilTemperature (void)
{ TypeIs (SUBSYSTEM_OIL_TEMPERATURE);
  hwId  = HW_GAUGE;
  conv  = 0;
}
//----------------------------------------------------------------
//  Read Parameters
//----------------------------------------------------------------
int COilTemperature::Read (SStream *stream, Tag tag)
{ return CEngineControl::Read (stream, tag);
}
//----------------------------------------------------------------
//  All Parameters are read
//----------------------------------------------------------------
void COilTemperature::ReadFinished()
{ CEngineControl::ReadFinished();
  Monitor('oilT');
  return;
}
//----------------------------------------------------------------------
//  Message receive
//----------------------------------------------------------------------
EMessageResult  COilTemperature::ReceiveMessage (SMessage *msg)
{ switch (msg->id) {
    case MSG_GETDATA:
        PollEngine(conv);
        msg->realData = indn;
        return MSG_PROCESSED;

    case MSG_SETDATA:
      // None
      break;
    }
  return CEngineControl::ReceiveMessage (msg);
}
//===========================================================================
// COilPressure  from CEngineControl
//  Monitors  engine oil presure
//===========================================================================
COilPressure::COilPressure (void)
{ TypeIs (SUBSYSTEM_OIL_PRESSURE);
  hwId  = HW_GAUGE;
  lowP	= 0;
  plow	= false;
}
//-----------------------------------------------------------------
//  Read Parameters
//-----------------------------------------------------------------
int COilPressure::Read (SStream *stream, Tag tag)
{ switch (tag) {
    case 'lowP':
    // Low pressure threshold
      ReadFloat (&lowP, stream);
      return TAG_READ;
  }
  return CEngineControl::Read (stream, tag);
}
//----------------------------------------------------------------
//  All Parameters are read
//----------------------------------------------------------------
void COilPressure::ReadFinished()
{ CEngineControl::ReadFinished();
  Monitor('oilP');
  return;
}
//------------------------------------------------------------------
//  Process Message
//------------------------------------------------------------------
EMessageResult  COilPressure::ReceiveMessage (SMessage *msg)
{ switch (msg->id) {
    case MSG_GETDATA:
      PollEngine(0);
      switch (msg->user.u.datatag) {
        case 'st8t':
        case 'oilP':
          msg->realData = indn;
          return MSG_PROCESSED;
        case 'lowP':
          msg->intData  = plow;
          return MSG_PROCESSED;
      }
    case MSG_SETDATA:
      // None
      break;
 }
 return CEngineControl::ReceiveMessage (msg);
}
//================================================================================
// CManifoldPressure  from CEngineControl
//  Monitors engine manifold presure
//================================================================================
CManifoldPressure::CManifoldPressure (void)
{TypeIs (SUBSYSTEM_MANIFOLD_PRESSURE);
  hwId = HW_GAUGE;
}
//----------------------------------------------------------------
//  All Parameters are read
//----------------------------------------------------------------
void CManifoldPressure::ReadFinished()
{ CEngineControl::ReadFinished();
  Monitor('eMap');
  return;
}
//-------------------------------------------------------------------------
//  Receive message
//-------------------------------------------------------------------------
EMessageResult  CManifoldPressure::ReceiveMessage (SMessage *msg)
{ switch (msg->id) {
    case MSG_GETDATA:
      PollEngine(0);
      msg->realData = indn;
      return MSG_PROCESSED;

    case MSG_SETDATA:
      // None
      break;
    }

  return CEngineControl::ReceiveMessage (msg);
}
//========================================================================
// CCylinderHeadTemp
//========================================================================
CCylinderHeadTemp::CCylinderHeadTemp (void)
{ TypeIs (SUBSYSTEM_CYLINDER_HEAD_TEMP);
  hwId  = HW_GAUGE;
  conv  = 0;
}
//----------------------------------------------------------------
//  Read parameters
//----------------------------------------------------------------
int CCylinderHeadTemp::Read (SStream *stream, Tag tag)
{ return CEngineControl::Read (stream, tag);
}
//----------------------------------------------------------------
//  All Parameters are read
//----------------------------------------------------------------
void CCylinderHeadTemp::ReadFinished()
{ CEngineControl::ReadFinished();
  Monitor('echT');
  return;
}
//----------------------------------------------------------------
//  Process Message
//----------------------------------------------------------------
EMessageResult  CCylinderHeadTemp::ReceiveMessage (SMessage *msg)
{ switch (msg->id) {
    case MSG_GETDATA:
          PollEngine(conv);
          msg->realData = indn;
          return MSG_PROCESSED;

    case MSG_SETDATA:
      // None
      break;
    }
  return CEngineControl::ReceiveMessage (msg);
}
//============================================================================
// CExhaustGasTemperature
//============================================================================
CExhaustGasTemperature::CExhaustGasTemperature (void)
{ TypeIs (SUBSYSTEM_EXHAUST_GAS_TEMPERATURE);
  hwId  = HW_GAUGE;
  conv  = 0;
}
//----------------------------------------------------------------
//  Read All parameters
//----------------------------------------------------------------
int CExhaustGasTemperature::Read (SStream *stream, Tag tag)
{ return CEngineControl::Read (stream, tag);
}
//----------------------------------------------------------------
//  All parameters are read.
//----------------------------------------------------------------
void CExhaustGasTemperature::ReadFinished()
{ CEngineControl::ReadFinished();
  Monitor('eegT');
  return;
}
//----------------------------------------------------------------
//  Process messages
//----------------------------------------------------------------
EMessageResult  CExhaustGasTemperature::ReceiveMessage (SMessage *msg)
{ switch (msg->id) {
    case MSG_GETDATA:
        PollEngine(conv);
        msg->realData = indn;
        return  MSG_PROCESSED;

    case MSG_SETDATA:
      // None
      break;
    }
  return CEngineControl::ReceiveMessage (msg);
}
//============================================================================
// CHobbsMeter
//============================================================================
CHobbsMeter::CHobbsMeter (void)
{ TypeIs (SUBSYSTEM_HOBBS_METER);
  hwId = HW_GAUGE;
}
//----------------------------------------------------------------
//  All parameters are read.
//----------------------------------------------------------------
void CHobbsMeter::ReadFinished()
{ CEngineControl::ReadFinished();
  Monitor('Hobm');
  return;
}
//---------------------------------------------------------------
//  Process message
//---------------------------------------------------------------
EMessageResult CHobbsMeter::ReceiveMessage (SMessage *msg)
{ switch (msg->id) {
    case MSG_GETDATA:
      // Return indication of engine operating hours
      PollEngine(0);
      msg->realData = engm.realData;
      return MSG_PROCESSED;
    case MSG_SETDATA:
      //-- NONE ---
      break;
    }
  return CEngineControl::ReceiveMessage (msg);
}
//===============================================================================
// CEngineMonitor
//===============================================================================
CEngineMonitor::CEngineMonitor (void)
{ TypeIs (SUBSYSTEM_ENGINE_MONITOR); }
//--------------------------------------------------------------------
//  Read all parameters
//--------------------------------------------------------------------
int CEngineMonitor::Read (SStream *stream, Tag tag)
{
  switch (tag) {
  case 'mMAP':
    // Manifold pressure message
    ReadMessage (&mMAP, stream);
    mMAP.sender = unId;
    return TAG_READ;

  case 'mRPM':
    // Tachometer message
    ReadMessage (&mRPM, stream);
    mRPM.sender = unId;
    return TAG_READ;

  case 'mTIT':
    // Turbine Intake Temperature message
    ReadMessage (&mTIT, stream);
    mTIT.sender = unId;
    return TAG_READ;

  case 'mFMS':
    // Fuel Management System message
    ReadMessage (&mFMS, stream);
    mFMS.sender = unId;
    return TAG_READ;

  case 'mOT_':
    // Oil Temperature message
    ReadMessage (&mOT, stream);
    mOT.sender = unId;
    return TAG_READ;

  case 'mOP_':
    // Oil Pressure message
    ReadMessage (&mOP, stream);
    mOP.sender = unId;
    return TAG_READ;

  case 'mCHT':
    // Cylinder Head Temperature message
    ReadMessage (&mCHT, stream);
    mCHT.sender = unId;
    return TAG_READ;

  case 'mVAC':
    // Vacuum Gauge message
    ReadMessage (&mVAC, stream);
    mVAC.sender = unId;
    return  TAG_READ;

  case 'mLQY':
    // Left fuel quantity message
    ReadMessage (&mLQTY, stream);
    mLQTY.sender = unId;
    return TAG_READ;

  case 'mRQY':
    // Right fuel quantity message
    ReadMessage (&mRQTY, stream);
    mRQTY.sender = unId;
    return TAG_READ;
  }

    // See if the tag can be processed by the parent class type
  return CEngineControl::Read (stream, tag);
}
//========================================================================
// CIdleControl
//========================================================================
CIdleControl::CIdleControl (void)
{ TypeIs (SUBSYSTEM_IDLE_CONTROL);
  hwId = HW_OTHER;
}
//=======================================================================
// CSteeringControl
//=======================================================================
CSteeringControl::CSteeringControl (void)
{ TypeIs (SUBSYSTEM_STEERING_CONTROL);
}
//=======================================================================
//  FLAP CONTROL
//=======================================================================
CFlapControl::CFlapControl (void)
{ TypeIs (SUBSYSTEM_FLAP_CONTROL);
  hwId = HW_SWITCH;
  nPos      = 0;
  aPos      = 0;
  dump      = 0;
  actualPos = 0;
  fTim      = 4;        //Default time
}
//----------------------------------------------------------------
//  Free resources
//-----------------------------------------------------------------
CFlapControl::~CFlapControl (void)
{ SAFE_DELETE_ARRAY (aPos);
}
//----------------------------------------------------------------------
//  Read All parameters
//----------------------------------------------------------------------
int CFlapControl::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;
  int nb;
  switch (tag) {
  case 'flpP':
    { // Flap position array
      ReadInt (&nPos, stream);
      aPos = new FLAP_POS[nPos];
      for (int i=0; i < nPos; i++) ReadInt (&aPos[i].degre, stream);
      return TAG_READ;
    }

  case 'flpS':
    { // Flap speed array
      if (0 == aPos)    gtfo("No Flap array for flpS");
      ReadInt (&nb, stream);
      if (nb > nPos)    gtfo("Too much Speed for flpS");
      for (int i=0; i < nb; i++) ReadFloat(&aPos[i].speed, stream);
			//-- For 0 deg, set a max speed to avoid error -----------
			if (aPos[0].degre == 0)	aPos[0].speed = 10000;
      return TAG_READ;
    }
  case 'flod':
    ReadFloat(&flod,stream);
    return TAG_READ;

  case 'dump':
    ReadInt(&dump,stream);
    return TAG_READ;

  case 'time':
    ReadFloat(&fTim,stream);
    return TAG_READ;
  }

    return CAeroControl::Read (stream, tag);
}
//----------------------------------------------------------------------
//  All parameters are read.  
//  Request Level of detail to compute Keyframes
//----------------------------------------------------------------------
void CFlapControl::ReadFinished()
{ CAeroControl::ReadFinished();
  vlod      = mveh->lod;
  if (vlod) vlod->InitFlap(nPos,aPos,fTim);
  state = 1;
}
//----------------------------------------------------------------------
//  Set actual position
//  NOTE:  the position is also set as target indn for flap gauge showing
//         flap position
//----------------------------------------------------------------------
void CFlapControl::NewPosition(int pos)
{ actualPos   = pos;
  indnTarget  = (aPos)?(aPos[pos].degre):(0);
	if (mveh->NotOPT(VEH_D_CRASH))	return;
	//--- check if flap dammaged by speed -----------
	float sped  = mveh->GetPreCalculedKIAS();
	if (sped < aPos[pos].speed)			return;
	//--- Generate a message ------------------------
	DAMAGE_MSG msg = {1,0,0,"Flaps are out (overspeed)"};
	mveh->DamageEvent(&msg);
	state = 0;								// Flaps out of service
  return;
}
//----------------------------------------------------------------------
//	Get maximum speed for requested position
//----------------------------------------------------------------------
float CFlapControl::GetMaxSpeed(int p)
{	if (0 == aPos)	return 10000;
	if (p >= nPos)	return 10000;
	return aPos[p].speed;
}
//----------------------------------------------------------------------
//  Set a new position
//----------------------------------------------------------------------
void CFlapControl::SetPosition(SMessage *msg)
{ if (!active)  return;
  int pos = actualPos;
  if (msg->dataType == TYPE_INT)    pos = msg->intData;
  if (msg->dataType == TYPE_REAL)   pos = int(msg->realData);
  if (vlod) pos = vlod->SetFlapPosition(pos);
  NewPosition(pos);
  return;
}
//----------------------------------------------------------------------
//  Autopilot interface
//----------------------------------------------------------------------
void CFlapControl::SetPosition(int pos)
{ if (!active)  return;
  if (vlod) pos = vlod->SetFlapPosition(pos);
  NewPosition(pos);
  return;
}
//----------------------------------------------------------------------
//  Process Message
//----------------------------------------------------------------------
EMessageResult  CFlapControl::ReceiveMessage (SMessage *msg)
{
  EMessageResult rc = MSG_IGNORED;

    switch (msg->id) {
      //---Request from channel mixer or gauge ----------------
      case MSG_GETDATA:
        switch (msg->user.u.datatag)  {
          //---Position by default -------------------
          case 'st8t':
            msg->realData = float(actualPos);
            return MSG_PROCESSED;
          case 'indn':
            msg->realData = indn;
            return MSG_PROCESSED;
          //---Deflection for mixer ------------------
          case 'data':
            msg->voidData = &data;
            return MSG_PROCESSED;
          //--- Angle -------------------------------
          case 'angl':
            msg->realData = float(aPos[actualPos].degre);
            return MSG_PROCESSED;

        }
      //---Set messages are from gauge or autopilot only -----
      case MSG_SETDATA:
          switch (msg->user.u.datatag) {
          case    101:
          case 'st8t':
            SetPosition(msg);
            return MSG_PROCESSED;
          }
    }

   // See if the message can be processed by the parent class
   return CAeroControl::ReceiveMessage (msg);
}
//----------------------------------------------------------------------
//  Keyboard interface
//  Next step
//----------------------------------------------------------------------
void CFlapControl::Incr (void)
{ if (!active)  return;
  int pos = actualPos;
  if (vlod) pos = vlod->SetFlapPosition(++pos);
  NewPosition(pos);
  return;
}
//----------------------------------------------------------------------
//  Keyboard interface
//  previous step
//----------------------------------------------------------------------
void CFlapControl::Decr (void)
{ if (!active)  return;
  int pos = actualPos;
  if (vlod) pos = vlod->SetFlapPosition(--pos);
  NewPosition(pos);
}
//----------------------------------------------------------------------
//  NOTE:  Flap scale in degre is computed here.
//         AeroControl TimeSlice must be skipped for this reason
//-----------------------------------------------------------------------
void CFlapControl::TimeSlice (float dT,U_INT FrNo)
{ active      &= (state == 1);							// Check for Out of Service
	data.deflect = indn;
  data.scaled  = indn;
  CDependent::TimeSlice (dT,FrNo);
  return;
}

//==========================================================================================
// CBrakeControl
// JS NOTE:  Brake control receive messages from
//    Keyboard  for left brake
//              for right brake
//              for both brakes
//    Gauge for parking brake
//    Distinct raw forces are maintain according to keypress 
//    The CSuspension::Timeslice  has direct access to computed forces
//==========================================================================================
CBrakeControl::CBrakeControl (void)
{ TypeIs (SUBSYSTEM_BRAKE_CONTROL);
  hwId          = HW_OTHER;
  Park          = 0;
  Hold          = 0;
	bKey					= 0;
  indnTarget    = 0;
  rate          = 0.05f;
  Brake[BRAKE_NONE] = 0;
  Brake[BRAKE_LEFT] = 0;
  Brake[BRAKE_RITE] = 0;
  Force[BRAKE_NONE] = 0;
	Force[BRAKE_LEFT]	= 0;
	Force[BRAKE_RITE]	= 0;
  turn = 0;
  br_timer = 0.0f;
  rf_timer = 0.1f;
 }
//------------------------------------------------------------------
//  Parameters are read. Init CGroundBrake message
//------------------------------------------------------------------
void CBrakeControl::ReadFinished()
{ CDependent::ReadFinished();
  //-----------------------------------------------
}
//------------------------------------------------------------------
//  Increase brake force
//------------------------------------------------------------------
void CBrakeControl::PressBrake(char pos)
{ Incr((pos & BRAKE_LEFT), rate);
  Incr((pos & BRAKE_RITE), rate);
  Hold = 0;
	bKey =Hold;
  return;
}
//------------------------------------------------------------------
//  Release brake force
//------------------------------------------------------------------
void CBrakeControl::ReleaseBrakes()
{ Decr(BRAKE_LEFT,rate);
	Decr(BRAKE_RITE,rate);
  return;
}
//------------------------------------------------------------------
//  Increment is receive from Keyboard while a brake key is
//            held in keypress position
//            The rate K is used as an increment factor
//-------------------------------------------------------------------
void CBrakeControl::Incr (char pos,float rt)
{ Brake[pos] += rt;
  if (Brake[pos] >= 1.0f)  Brake[pos] = 1.0f;
}
//-----------------------------------------------------------------------
//  Decrease brake force until it reach zero
//-----------------------------------------------------------------------
int CBrakeControl::Decr (char pos,float rt)
{ float val  = Brake[pos];
  if (0.0f == val) return 0;
  val       -= (rt);
  if (val < FLT_EPSILON)  val = 0.0f;
  Brake[pos] = val;
  return 1;
}
//-------------------------------------------------------------------------------
// Swap Parking position
//-------------------------------------------------------------------------------
void CBrakeControl::SwapPark()
{ Park ^= 1;
  if (gage)  gage->SubsystemCall(this,Park);
  return;
}
//-------------------------------------------------------------------------------
//  Process message
//  Message are only received from panel 
//-------------------------------------------------------------------------------
EMessageResult CBrakeControl::ReceiveMessage (SMessage *msg)
{    switch (msg->id) {
      case MSG_GETDATA:
        switch (msg->user.u.datatag) {
          case 'pbrk' :  // park brakes
            msg->intData  = Park;
            return MSG_PROCESSED;
        }
        break;

      case MSG_SETDATA:
          switch (msg->user.u.datatag) {
            case 'park' : // park brakes
              Park        = msg->intData;
              return MSG_PROCESSED;
						//--- Set park on if not ------
						case 'onpk':
							Park	= 1;
							return MSG_PROCESSED;
						//--- Reset park  -------------
						case 'ofpk':
							Park	= 0;
							return MSG_PROCESSED; 
						//--- break toe ---------------
						case 'btoe':
							int u = msg->user.u.engine;
							Force[u]	= msg->realData;
							return MSG_PROCESSED;
          }
    }  
    return CDependent::ReceiveMessage (msg);
}
//----------------------------------------------------------------------
//  Time slice the brakes
//----------------------------------------------------------------------
void  CBrakeControl::TimeSlice (float dT,U_INT FrNo)		// 
{ CDependent::TimeSlice (dT, FrNo);
	//--- poll any connected axis ----------------------
	globals->jsm->Poll(JS_LEFT_TOE,Brake[BRAKE_LEFT]);
	globals->jsm->Poll(JS_RITE_TOE,Brake[BRAKE_RITE]);
	//--- Poll keyboard brake keys --------------------
	br_timer += dT;
	if (br_timer > rf_timer) {
			br_timer -= rf_timer;
			if (Hold)  PressBrake(Hold);
		  else       ReleaseBrakes();
	}
	//--- Set force on each brake ------------------------
	Force[BRAKE_LEFT] = (Park)?(1):(Brake[BRAKE_LEFT]);
	Force[BRAKE_RITE] = (Park)?(1):(Brake[BRAKE_RITE]);
  return;
}
//----------------------------------------------------------------------
//  probe brakes
//----------------------------------------------------------------------
void CBrakeControl::Probe(CFuiCanva *cnv)
{ char edt[64];
  CDependent::Probe(cnv,0);
  _snprintf(edt,63,"Left   val: %.4f",Brake[BRAKE_LEFT]);
  cnv->AddText(1,edt,1);
	_snprintf(edt,63,"Rite   val: %.4f",Brake[BRAKE_RITE]);
  cnv->AddText(1,edt,1);
  _snprintf(edt,63,"Left force: %.4f",Force[BRAKE_LEFT]);
  cnv->AddText(1,edt,1);
	_snprintf(edt,63,"Rite force: %.4f",Force[BRAKE_RITE]);
  cnv->AddText(1,edt,1);
  return;
}

//================================================================================
// CElevatorTrimControl
//================================================================================
CElevatorTrimControl::CElevatorTrimControl (void)
{ TypeIs (SUBSYSTEM_ELEVATOR_TRIM_CONTROL);
	timer = 0.5;
	ok	  = 1;
}
//------------------------------------------------------------------------
//	Poll joystick to get value from any assigned axis
//-----------------------------------------------------------------------
void CElevatorTrimControl::TimeSlice (float dT,U_INT FrNo)		
{ CAeroControl::TimeSlice(dT,FrNo);								// JSDEV*
  timer -= dT;
	if (timer < 0)	{timer += dT; ok ^= 1;}
	globals->jsm->Poll(JS_TRIM,data.raw);
	return;
}
//-----------------------------------------------------------------------
//	All parameters are read, remap joystick
//-----------------------------------------------------------------------
void	CElevatorTrimControl::ReadFinished()
{	globals->jsm->MapTo(JS_TRIM,unId);
}
//-----------------------------------------------------------------------
//	Slow down increment
//-----------------------------------------------------------------------
void CElevatorTrimControl::Incr()
{	if (ok)	return;
	return CAeroControl::Incr();	} 
//-----------------------------------------------------------------------
//	Slow down decrement
//-----------------------------------------------------------------------
void CElevatorTrimControl::Decr()
{	if (ok)	return;
	return CAeroControl::Decr();	} 
//================================================================================
// CRudderTrimControl
//================================================================================
CRudderTrimControl::CRudderTrimControl (void)
{
  TypeIs (SUBSYSTEM_RUDDER_TRIM_CONTROL);
}

//================================================================================
// CAileronTrimControl
//================================================================================
CAileronTrimControl::CAileronTrimControl (void)
{
  TypeIs (SUBSYSTEM_AILERON_TRIM_CONTROL);
}

//================================================================================
// CGearControl
//================================================================================
CGearControl::CGearControl (void)
{
  TypeIs (SUBSYSTEM_GEAR_CONTROL);
  hwId  = HW_OTHER;
  hydr  = 0;
  lock  = 0;
  Pos   = 0;    // Default is down
}
//------------------------------------------------------------
//  Read Parameters
//------------------------------------------------------------
int CGearControl::Read (SStream *stream, Tag tag)
{
  switch (tag) {
    case 'hydr':
      // Hydraulically actuated gear
      hydr = 1;
      return TAG_READ;
    case 'lock':
      // Ground lever lock
      lock = 1;
      return TAG_READ;
    }

 return CDependent::Read (stream, tag);
}
//------------------------------------------------------------
//  All parameters are read
//------------------------------------------------------------
void CGearControl::ReadFinished()
{ CDependent::ReadFinished();
  CAnimatedModel *mlod = mveh->GetLOD();
  if (0 == mlod)    return;
  //----Transmit lock tag-----------
  mlod->SetGearControl(this);
  return;
}
//------------------------------------------------------------
//  Swap gear position
//------------------------------------------------------------
void CGearControl::Swap()
{ Pos ^= 1;
  ChangePosition();
}
//--------------------------------------------------------------------------------
//  Change Status
//  Send a request to the LOD model.  
//  NOTE:  The LOD model expect a request with postion inverted i.e 0 is Down
//-TODO: Check power status if needed
//---------------------------------------------------------------------------------
void CGearControl::ChangePosition()
{ CAnimatedModel *mlod = mveh->GetLOD();
  if (0 == mlod)    return;
  //----Request LOD to move gear -------
  mlod->SetLandingTo(Pos);
}
//--------------------------------------------------------------------------------
//  Process message
//---------------------------------------------------------------------------------
EMessageResult CGearControl::ReceiveMessage (SMessage *msg)
{ 
   switch (msg->id) {
    case MSG_GETDATA:
        switch (msg->user.u.datatag) {
          case 'st8t':
            msg->realData = double(Pos);
            return MSG_PROCESSED;
        }

    case MSG_SETDATA:
        switch (msg->user.u.datatag) {
          case 'st8t' :
          { int old = Pos;
            if (msg->dataType == TYPE_INT)  Pos = msg->intData;
            if (msg->dataType == TYPE_REAL) Pos = int(msg->realData);
            if (old != Pos) ChangePosition();
            return MSG_PROCESSED;
          }
        }

   }
    return CDependent::ReceiveMessage (msg);
}
//==================================================================================
// CHighLiftControl
//==================================================================================
CHighLiftControl::CHighLiftControl (void)
{
  TypeIs (SUBSYSTEM_HIGH_LIFT_CONTROL);
}

//
// CCowlFlapControl
//
CCowlFlapControl::CCowlFlapControl (void)
{
  TypeIs (SUBSYSTEM_COWL_FLAP_CONTROL);
}

//==================================================================================
// CAirBrakeControl
//==================================================================================
CAirBrakeControl::CAirBrakeControl (void)
{ TypeIs (SUBSYSTEM_AIR_BRAKE_CONTROL);
}
//---------------------------------------------------------------------------
//  Read parameters for AirBrake control
//---------------------------------------------------------------------------
int CAirBrakeControl::Read (SStream *stream, Tag tag)
{ if  (tag != 'mFlp') return CAeroControl::Read (stream, tag);
  ReadMessage(&msg,stream);
  msg.sender = unId;
  return TAG_READ;
}


//==================================================================================
// CAudioControl
//==================================================================================
CAudioPanel::CAudioPanel (void)
{
  TypeIs (SUBSYSTEM_AUDIO_PANEL);
}

//
// CEngineAntiIce
//
CEngineAntiIce::CEngineAntiIce (void)
{
  TypeIs (SUBSYSTEM_ENGINE_ANTI_ICE);
}

//
// CSurfaceDeIce
//
CSurfaceDeIce::CSurfaceDeIce (void)
{
  TypeIs (SUBSYSTEM_SURFACE_DEICE);
}

//
// CPropellerDeIce
//
CPropellerDeIce::CPropellerDeIce (void)
{
  TypeIs (SUBSYSTEM_PROPELLER_DEICE);

  cycleTime = 0;
  heaterLoad = 0;
}

int CPropellerDeIce::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'aray':
    {
      // Prop heater array
      int nPropHeaters = 0;
      ReadInt (&nPropHeaters, stream);
      for (int i=0; i<nPropHeaters; i++) {
        int heater;
        ReadInt (&heater, stream);
        propHeater.push_back (heater);
      }
      rc = TAG_READ;
    }
    break;
  case 'cycT':
    // Cycle time
    ReadFloat (&cycleTime, stream);
    rc = TAG_READ;
    break;
  case 'heat':
    // Heater load (amps)
    ReadFloat (&heaterLoad, stream);
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CDependent::Read (stream, tag);
  }

  return rc;
}

//=============================================================================
// CWingDeIce
//=============================================================================
CWingDeIce::CWingDeIce (void)
{  TypeIs (SUBSYSTEM_WING_DEICE);
   hwId = HW_SWITCH;
}
//-----------------------------------------------------------------------
//	Destructor
//------------------------------------------------------------------------
CWingDeIce::~CWingDeIce (void)
{ for (U_INT k= 0; k < msgPump.size(); k++) delete msgPump[k];
  msgPump.clear();
}

int CWingDeIce::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'cycT':
    // Cycle time
    ReadFloat (&cycleTime, stream);
    rc = TAG_READ;
    break;

  case 'mPmp':
    {
      // Pump circuit message
      SMessage* msg = new SMessage;
      msg->sender   = unId;
      ReadMessage (msg, stream);
      msgPump.push_back (msg);
    }
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CDependent::Read (stream, tag);
  }

  return rc;
}

//============================================================================
// CTempControl
//============================================================================
CTempControl::CTempControl (void)
{ TypeIs (SUBSYSTEM_TEMP_CONTROL);
  base  = 60;
  rate  = 1;
  temp  = 0;
  mint  = 0;
  maxt  = 0;
}
//-----------------------------------------------------------------
//  Read parameters
//-----------------------------------------------------------------
int CTempControl::Read (SStream *st, Tag tag)
{ float nb = 0;
  switch (tag) {
    case 'targ':
      ReadFloat(&indnTarget,st);
      sign = (indnTarget > temp)?(+1):(-1);
      return TAG_READ;
    case 'rate':
      ReadFloat(&nb,st);
      rate = (nb > 0)?(nb / base):(1);
      return TAG_READ;
    case 'min_':
      ReadInt(&mint,st);
      return TAG_READ;
    case 'max_':
      ReadInt(&maxt,st);
      return TAG_READ;
    case 'temp':
      ReadFloat(&temp,st);
      return TAG_READ;
  }
  return::CDependent::Read(st,tag);
}
//------------------------------------------------------------------
//  Probe this subsystem
//------------------------------------------------------------------
void CTempControl::Probe(CFuiCanva *cnv)
{ char edt[64];
  CDependent::Probe(cnv,0);
  _snprintf(edt,63,"targ: %.4f",indnTarget);
  cnv->AddText(1,edt,1);
  _snprintf(edt,63,"temp: %.4f",temp);
  cnv->AddText(1,edt,1);
  return;
}
//------------------------------------------------------------------
//  TimeSlice Temperature control
//------------------------------------------------------------------
void CTempControl:: TimeSlice (float dT,U_INT FrNo)
{ CDependent::TimeSlice(dT,FrNo);
  if (0 == active)  return;
  //---Update temperature ---------------------------
  float dta = fabs(indnTarget - temp);
  float inc = dT * rate;
  if (inc > dta) inc = dta;
  temp += inc * sign;
  return;
}
//------------------------------------------------------------------
//  Message received
//------------------------------------------------------------------
EMessageResult CTempControl::ReceiveMessage (SMessage *msg)
{ switch (msg->id) {
		case MSG_GETDATA:
			switch (msg->user.u.datatag) {
				case 'temp':
					msg->realData	= temp;
					return MSG_PROCESSED;

				case 'targ':
					msg->realData	= indnTarget;
					return MSG_PROCESSED;
      }
		case MSG_SETDATA:
			switch (msg->user.u.datatag) {
				case 'temp':
					temp			  = (float)msg->realData;
          sign = (indnTarget > temp)?(+1):(-1);
					return MSG_PROCESSED;
        case 'targ':
					indnTarget  = (float)msg->realData;
          sign = (indnTarget > temp)?(+1):(-1);
					return MSG_PROCESSED;
      }
  }
  return CDependent::ReceiveMessage (msg);
}

//=============================================================================
// CWiperControl
//=============================================================================
CWiperControl::CWiperControl (void)
{
  TypeIs (SUBSYSTEM_WIPER_CONTROL);
}

//
// CPerformanceReserve
//
CPerformanceReserve::CPerformanceReserve (void)
{
  TypeIs (SUBSYSTEM_PERFORMANCE_RESERVE);
}

//
// CRudderBias
//
CRudderBias::CRudderBias (void)
{
  TypeIs (SUBSYSTEM_RUDDER_BIAS);

  rpmOption = 0;
  rpmLimit = 0;
  leftEngine = 0;
  rightEngine = 0;
}

int CRudderBias::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'rpmO':
    // RPM Option
    ReadInt (&rpmOption, stream);
    rc = TAG_READ;
    break;
  case 'rpmL':
    // RPM Limit
    ReadFloat (&rpmLimit, stream);
    rc = TAG_READ;
    break;
  case 'lEng':
    // Left engine
    ReadInt (&leftEngine, stream);
    rc = TAG_READ;
    break;
  case 'rEng':
    // Right engine
    ReadInt (&rightEngine, stream);
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CDependent::Read (stream, tag);
  }

  return rc;
}

//
// COxygen
//
COxygen::COxygen (void)
{
  TypeIs (SUBSYSTEM_OXYGEN);

  full = 0;
  fill = 0;
  rate = 0;
  pressure = 0;
}

int COxygen::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'full':
    // Supply capacity (cu. ft.)
    ReadFloat (&full, stream);
    rc = TAG_READ;
    break;
  case 'fill':
    // Quantity (cu. ft.)
    ReadFloat (&fill, stream);
    rc = TAG_READ;
    break;
  case 'rate':
    // Supply rate (cu. ft. per person per hour)
    ReadFloat (&rate, stream);
    rc = TAG_READ;
    break;
  case 'pres':
    // System pressure (psi)
    ReadFloat (&pressure, stream);
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CDependent::Read (stream, tag);
  }

  return rc;
}
//=============================END OF FILE ===================================================================