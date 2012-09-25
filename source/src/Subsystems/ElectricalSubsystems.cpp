/*
 * ElectricalSubsystems.cpp
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

#include "../Include/Subsystems.h"
#include "../Include/Globals.h"
#include "../Include/Fui.h"

#ifdef _DEBUG	
#include "../Include/Utility.h"  
#endif

using namespace std;
///////// DEBUG STUFF TO REMOVE LATER ////////////////////////////////
#ifdef  _DEBUG	
  //#define _DEBUG_RIGN  //print lc DDEBUG file ... remove later
#endif
//////////////////////////////////////////////////////////////////////
//=================================================================================
// CBattery
//=================================================================================
CBattery::CBattery (void)
{
  TypeIs (SUBSYSTEM_BATTERY);
  hwId = HW_BATTERY;

  life = 0;
  lowLevel =12;
}

//---------------------------------------------------------------------------
//  Read all parameters
//--------------------------------------------------------------------------
int CBattery::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'life':
    // Battery capacity (amp-hours);
    ReadFloat (&life, stream);
    rc = TAG_READ;
    break;
  case 'lowB':
    // Low battery level (volts)
    ReadFloat (&lowLevel, stream);
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CDependent::Read (stream, tag);
  }

  return rc;
}
//---------------------------------------------------------------------------
//  Receive message
//  Return activity and voltage
//---------------------------------------------------------------------------
EMessageResult CBattery::ReceiveMessage (SMessage *msg)
{ 
  if (msg->id == MSG_GETDATA)
	{	switch (msg->user.u.datatag) 
		{
				case 'actv':
					msg->intData	= state;
          msg->volts    = volt;
					return MSG_PROCESSED;

        case 'amph':
          msg->realData = (active)?(life):(0);
					return MSG_PROCESSED;

		}
	}
	
      // See if the message can be processed by the parent class
  return CDependent::ReceiveMessage (msg);
}
//===================================================================================
// CAlternator
//===================================================================================
CAlternator::CAlternator (void)
{
  TypeIs (SUBSYSTEM_ALTERNATOR);
  hwId = HW_ALTERNATOR;
  volt = 28;                // default voltage
  mxld = 0;
  loRg = 500;               // Default lower rpm value
  mxRg = 700;               // Default max voltage RPM
  hiRg = 3000;              // default upper rpm value
}
//-----------------------------------------------------------------------
//  Read Parameters
//-----------------------------------------------------------------------
int CAlternator::Read (SStream *stream, Tag tag)
{
  switch (tag) {
  case 'mxld':
    // Maximum load (amps)
    ReadFloat (&mxld, stream);
    return TAG_READ;
  case 'loRg':
    // Minimum RPM regulatable
    ReadFloat (&loRg, stream);
    return TAG_READ;
  case 'mxRg':
    // Minimum RPM regulatable
    ReadFloat (&mxRg, stream);
    return TAG_READ;
  case 'volt':
    ReadFloat (&mvlt, stream);
    return TAG_READ;
  }

    // See if the tag can be processed by the parent class type
    return CDependent::Read (stream, tag);
}
//-----------------------------------------------------------------------
//  All parameters are read
//-----------------------------------------------------------------------
void CAlternator::ReadFinished()
{ CDependent::ReadFinished();
  //--- init engine message -----------
  Tag eng = 'Eng0' + eNum;
  emsg.id             = MSG_GETDATA;
  emsg.sender         = unId;
  emsg.dataType       = TYPE_VOID;
  emsg.group          = eng;
  emsg.user.u.datatag = 'data';
  //---Volt coefficient  ------------------
  vFac = mvlt / (mxRg - loRg);
  return;
}
//-----------------------------------------------------------------------
//  Time Slice
//  when all dependents are OK, check the engine RPM
//-----------------------------------------------------------------------
void CAlternator::TimeSlice(float dT,U_INT FrNo)
{ CDependent::TimeSlice(dT, FrNo);
  //---Get engine RPM --------------------
  CEngineData *data = (CEngineData*)emsg.voidData;
  if (0 == data)
  { Send_Message(&emsg);             // Querry engine
    data = (CEngineData*)emsg.voidData;
  }
  if (0 == data)
  { active = false;
    state = 0;
    return;
  }
  //---Check enough RPM ----------------------
  bool rpm  = ((data->EngRPM() > loRg) && (data->EngRPM() < hiRg));
  active   &= rpm;
  //---Compute voltage ----------------------
  volt      = (data->EngRPM() - loRg) * vFac * state;
  if (volt > mvlt)  volt = mvlt;
  return;
}
//-----------------------------------------------------------------------
//  Receive message
//-----------------------------------------------------------------------
EMessageResult CAlternator::ReceiveMessage (SMessage *msg)
{ if (msg->id != MSG_GETDATA)   return MSG_IGNORED;
  //---- return active status -------------------------
  switch (msg->user.u.datatag)  {
      case 'st8t':
      case 'actv':
        msg->intData = (active != 0);
        msg->volts   = (active)?(volt):(0);
        return MSG_PROCESSED;
  }
  return CDependent::ReceiveMessage (msg);
}
//===========================================================================
// CGenerator
//===========================================================================
CGenerator::CGenerator (void)
{ TypeIs (SUBSYSTEM_GENERATOR);
  mxld  = 1000;
  loRG  =  300;
  hiRG  = 10000;
  volt  = 28;                   // Default voltage
}
//------------------------------------------------------------------------
//  Read all parameters
//------------------------------------------------------------------------
int CGenerator::Read (SStream *stream, Tag tag)
{
  switch (tag) {
  case 'mxld':
    // Maximum load (amps)
    ReadFloat (&mxld, stream);
    return TAG_READ;

  case 'loRg':
    // Minimum regulatable RPM
    ReadFloat (&loRG, stream);
    return TAG_READ;

  case 'hiRG':
    // Maximum regulatable RPM
    ReadFloat(&hiRG, stream);
    return TAG_READ;
  }
  
 // See if the tag can be processed by the parent class type
 return CDependent::Read (stream, tag);
}
//-----------------------------------------------------------------------
//  All parameters are read
//-----------------------------------------------------------------------
void CGenerator::ReadFinished()
{ CDependent::ReadFinished();
  //--- init engine message -----------
  Tag eng = 'Eng0' + eNum;
  emsg.id             = MSG_GETDATA;
  emsg.sender         = unId;
  emsg.dataType       = TYPE_VOID;
  emsg.group          = eng;
  emsg.user.u.datatag = 'data';
  return;
}
//-----------------------------------------------------------------------
//  Time Slice
//  when all dependents are OK, check the engine RPM
//-----------------------------------------------------------------------
void CGenerator::TimeSlice(float dT,U_INT FrNo)
{ CDependent::TimeSlice(dT, FrNo);
  if (!active)    return;
  //---Get engine RPM --------------------
  CEngineData *data = (CEngineData*)emsg.voidData;
  if (0 == data)
  { Send_Message(&emsg);             // Querry engine
    data = (CEngineData*)emsg.voidData;
  }
  if (0 == data)
  { active = false;
    state = 0;
    return;
  }
  //---Check enough RPM ----------------------
  bool rpm  = ((data->EngRPM() > loRG) && (data->EngRPM() < hiRG));
  active   &= rpm;
  return;
}
//-----------------------------------------------------------------------
//  Receive message
//-----------------------------------------------------------------------
EMessageResult CGenerator::ReceiveMessage (SMessage *msg)
{ if (msg->id != MSG_GETDATA)   return MSG_IGNORED;
  //---- return active status -------------------------
  switch (msg->user.u.datatag)  {
      case 'st8t':
      case 'actv':
        msg->intData = (active != 0);
        msg->volts   = (active)?(volt):(0);
        return MSG_PROCESSED;
  }
  return CDependent::ReceiveMessage (msg);
}
//================================================================================
// AnnunciatorLight
//================================================================================
CAnnunciatorLight::CAnnunciatorLight (void)
{ TypeIs (SUBSYSTEM_ANNUNCIATOR_LIGHT);
  hwId    = HW_LIGHT;
  blnk		= 20;						// Default period
  blpp		= 1;						// blink period
  bState	= OFF;
  Etat		= OFF;
}
//--------------------------------------------------------------------------------
//	Read all tags
//--------------------------------------------------------------------------------
int CAnnunciatorLight::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'blnk':
    // Blink period (sec);
    ReadFloat (&blnk, stream);
    rc = TAG_READ;
    break;

  case 'blpp':
    // Blink period
    ReadFloat (&blpp, stream);
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CDependent::Read (stream, tag);
  }

  return rc;
}
//--------------------------------------------------------------------------------
//	JSDEV*	Maintain the blinking state when active
//	Stop blink when inactive or blink period is exhausted
//--------------------------------------------------------------------------------
void CAnnunciatorLight::TimeSlice(float dT,U_INT FrNo)
{	CDependent::TimeSlice(dT,FrNo);					// Update activity
	switch (Etat)	{
	//--- Off state. Wait to become active ---------------------
	case OFF:										// Wait for activity
		if (!active)				return;
		T1		= blnk;								// arm blink duration
		T2		= blpp;								// arm blink period
		bState	= 1;								// start with on
		Etat	= ON;								// activate blinking
		return;
	//--- On state. Toogle blink until time exhauted or inactive -
	case ON:										// Activated
		if (!active)
		{	bState	= 0;							
			Etat	= OFF;
			return;		}
		// -- update both timers -----------
		if (blnk != 0)		T1	-= dT;				// Blnk= 0 => blink forever
		if (T1 < 0) 
			{bState = 1; Etat = SBYE;	return; }
		T2	-= dT;					
		if (T2 > 0)					return;
		T2	 = blpp;								// Rearm T2
		bState ^= 1;								// Toggle blink state
		return;
	//---- Standbye. Wait for inactive to restart ------------------
	case SBYE:										// Standbye wait for inactivity
		if (active)					return;
		bState	= 0;
		Etat	= OFF;	
		return;
	}
	return;
}
//-------------------------------------------------------------------------
//	Return activity state
//-------------------------------------------------------------------------
EMessageResult CAnnunciatorLight::ReceiveMessage (SMessage *msg)
{
  EMessageResult rc = MSG_IGNORED;

    if (msg->id == MSG_GETDATA)
	{	switch (msg->user.u.datatag) 
		{
				case 'actv':
					msg->intData	= bState;
					return MSG_PROCESSED;
		}
	}
	if (rc == MSG_IGNORED) 
	{
      // See if the message can be processed by the parent class
      rc = CDependent::ReceiveMessage (msg);
	}
  return rc;
}
//===========================================================================
// CExteriorLight
//===========================================================================
CExteriorLight::CExteriorLight (void)
{
  TypeIs (SUBSYSTEM_EXTERIOR_LIGHT);
}
//---------------------------------------------------------------------
//    Read parameters
//---------------------------------------------------------------------
int CExteriorLight::Read (SStream *stream, Tag tag)
{ switch (tag) {
  case 'LMid':
    // Light model ID (unique identifier in external light manager)
    ReadTag (&unId, stream);
    TagToString (unId_string, unId);
    return TAG_READ;
  }
  return CDependent::Read (stream, tag);

}
//---------------------------------------------------------------------
//    Update light
//---------------------------------------------------------------------
void CExteriorLight::TimeSlice (float dT,U_INT FrNo)			// JSDEV*
{
  bool initActive = active;
  CDependent::TimeSlice (dT,FrNo);								// JSDEV*
  // Update external light manager with subsystem active state if it has changed
  if (active != initActive) mveh->elt.SetPowerState (unId, active);
}
//=============================================================================
// CVoltmeter
//=============================================================================
CVoltmeter::CVoltmeter (void)
{ TypeIs (SUBSYSTEM_VOLTMETER);
  hwId  = HW_GAUGE;
  lowV  = 0;
  annV  = 0;
}
//---------------------------------------------------------------------
//    Read All parameters
//---------------------------------------------------------------------
int CVoltmeter::Read (SStream *stream, Tag tag)
{switch (tag) {
  case 'lowV':
    // Low voltage limit
    ReadFloat (&lowV, stream);
    return TAG_READ;

  case 'mMon':
    // Circuit monitoring message
    ReadMessage (&msg, stream);
    return TAG_READ;

  case 'cvrt':
    // \todo Handle conversion coefficient
    SkipObject (stream);
    return TAG_READ;
  }

    // See if the tag can be processed by the parent class type
  return CDependent::Read (stream, tag);
}
//---------------------------------------------------------------------
//    TimeSlice the system
//---------------------------------------------------------------------
void CVoltmeter::TimeSlice(float dT,U_INT FrNo)
{ //---Update activity ----------------------------
  volt = indn;
  annV = (volt < lowV);
  CDependent::TimeSlice(dT,FrNo);
  msg.realData = 0;
  Send_Message(&msg);
  indnTarget = msg.realData;
  return;
}
//---------------------------------------------------------------------
//    Read the voltmeter
//---------------------------------------------------------------------
EMessageResult CVoltmeter::ReceiveMessage (SMessage *msg)
{ if (msg->id != MSG_GETDATA ) return CDependent::ReceiveMessage(msg);
  switch (msg->user.u.datatag) {
     //---- Return voltage ----------------------
      case 'st8t':
      case 'volt':
      case 'indn':
          msg->realData = indn;
          return MSG_PROCESSED;
      //---- Return low voltage ------------------
      case 'lowV':
        msg->intData = lowV;
        return MSG_PROCESSED;
  }
  return CDependent::ReceiveMessage(msg);
}
//-----------------------------------------------------------------------------
//  Probe the Voltmeter
//-----------------------------------------------------------------------------
void CVoltmeter::Probe(CFuiCanva *cnv)
{ char edt[32];
  CDependent::Probe(cnv,0);
  _snprintf(edt,32,"Low Volt %.4f",lowV);
  cnv->AddText( 1,edt,1);
  _snprintf(edt,32,"warning  %d",annV);
  cnv->AddText( 1,edt,1);
  _snprintf(edt,32,"Monitoring %s",TagToString(msg.group));
  cnv->AddText( 1,edt,1);
  return;
}
//===========================================================================
// CAmmeter
//===========================================================================
CAmmeter::CAmmeter (void)
{ TypeIs (SUBSYSTEM_AMMETER);
  hwId  = HW_GAUGE;
  chrg  = false;
  loadMeter = false;
}
//------------------------------------------------------------------
//  read all parameters
//------------------------------------------------------------------
int CAmmeter::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'chrg':
    // Charge meter
    chrg = true;
    return TAG_READ;

  case 'lodm':
    // Load meter
    loadMeter = true;
    return TAG_READ;

  case 'mMon':
    // Monitor message
    ReadMessage (&msg, stream);
    return TAG_READ;

  case 'cvrt':
    // \todo Handle conversion coefficient
    SkipObject (stream);
    return TAG_READ;
  }

  // See if the tag can be processed by the parent class type
  return CDependent::Read (stream, tag);
}
//------------------------------------------------------------------
//  Time slice. Poll the monitored system
//------------------------------------------------------------------
void  CAmmeter::TimeSlice(float dT,U_INT FrNo)
{ //---Update activity status -----------------------
  CDependent::TimeSlice(dT,FrNo);
  msg.realData = 0;
  if (active)  Send_Message(&msg);
  indnTarget = msg.realData;
  return;
}
//-----------------------------------------------------------------------------
//  Receive message
//-----------------------------------------------------------------------------
EMessageResult CAmmeter::ReceiveMessage (SMessage *msg)
{ if (msg->id != MSG_GETDATA ) return CDependent::ReceiveMessage(msg);
  switch (msg->user.u.datatag) {
     //---- Return ampre value ----------------------
      case 'st8t':
      case 'volt':
      case 'indn':
          msg->realData = indn;
          return MSG_PROCESSED;
  }
  return CDependent::ReceiveMessage(msg);
}
//-----------------------------------------------------------------------------
//  Probe the Ammeter
//-----------------------------------------------------------------------------
void CAmmeter::Probe(CFuiCanva *cnv)
{ char edt[32];
  CDependent::Probe(cnv,0);
  _snprintf(edt,32,"Monitoring %s",TagToString(msg.group));
  cnv->AddText( 1,edt,1);
  return;
}

//==============================================================================
// CFrequencyMeter
//==============================================================================
CFrequencyMeter::CFrequencyMeter (void)
{ TypeIs (SUBSYSTEM_FREQUENCY_METER);}

int CFrequencyMeter::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'mMon':
    // Circuit monitoring message
    ReadMessage (&mMon, stream);
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
// CInverter
//
CInverter::CInverter (void)
{
  TypeIs (SUBSYSTEM_INVERTER);
}

//
// CPitotStaticSwitch
//
CPitotStaticSwitch::CPitotStaticSwitch (void)
{
  TypeIs (SUBSYSTEM_PITOT_STATIC_SWITCH);
  hwId = HW_SWITCH;

  nSystems = 0;
  system = NULL;
}

CPitotStaticSwitch::~CPitotStaticSwitch (void)
{
  delete[] system;
}

int CPitotStaticSwitch::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'aray':
    {
      // Array of systems
      ReadInt (&nSystems, stream);
      system = new int[nSystems];
      for (int i=0; i<nSystems; i++) {
        ReadInt (&system[i], stream);
      }
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


//
// CPitotHeatSwitch
//
CPitotHeatSwitch::CPitotHeatSwitch (void)
{
  TypeIs (SUBSYSTEM_PITOT_HEAT_SWITCH);
}


//
// CStaticSourceSwitch
//
CStaticSourceSwitch::CStaticSourceSwitch (void)
{
  TypeIs (SUBSYSTEM_STATIC_SOURCE_SWITCH);

  // Default hardware type for this subsystem is SWITCH
  hwId = HW_SWITCH;
}


//==========================================================================
// CLightSettingState
//==========================================================================
CLightSettingState::CLightSettingState (void)
{
  TypeIs (SUBSYSTEM_LIGHT_SETTING_STATE);

  levl = 1.0f;
}
//-------------------------------------------------------------------------
//  read Parameters
//--------------------------------------------------------------------------
int CLightSettingState::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'levl':
    // Default light level
    ReadFloat (&levl, stream);
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CDependent::Read (stream, tag);
  }

  return rc;
}
//-----------------------------------------------------------------------------
//  Probe the level
//-----------------------------------------------------------------------------
void CLightSettingState::Probe(CFuiCanva *cnv)
{ char edt[16];
  CDependent::Probe(cnv,0);
  _snprintf(edt,16,"level %.4f",levl);
  cnv->AddText( 1,edt,1);
  return;
}
//-----------------------------------------------------------------------------
//  Read message
//----------------------------------------------------------------------------
EMessageResult CLightSettingState::ReceiveMessage (SMessage *msg)
{  // Light subsystem is active; process GETDATA and SETDATA
   switch (msg->id) {

      case MSG_GETDATA:
        msg->realData = (active)?(levl):(0);
        return MSG_PROCESSED;

      case MSG_SETDATA:
        switch (msg->user.u.datatag)  {
          case 'levl':
            levl = Clamp01(msg->realData);
            return MSG_PROCESSED;
        }
   }
  return MSG_IGNORED;
}
//=================================================================================
// CSwitchSet
//  Implementtation:
//  With <sync> tag:  When the swst change state (active) then all associated message
//                    are sent to other subsystem with the swst state (ON or OFF)
//  Without the tag:  A receive message change the indx value corresponding to the
//                    active subsystem. The new active subsystem is activated 
//                    (with OnValue) while all other subsystem are desactivated 
//                    (with offValue)
//=================================================================================
CSwitchSet::CSwitchSet (void)
{ TypeIs (SUBSYSTEM_SWITCH_SET);
  hwId = HW_SWITCH;
  indx = 0;
  sync = false;
  zBase     = 1;
  onState   = 1;
  offState  = 0;
}
//-------------------------------------------------------------------------
//  Destroy the set
//-------------------------------------------------------------------------
CSwitchSet::~CSwitchSet (void)
{ for (U_INT  k=0; k < msgs.size(); k++) delete msgs[k];
  msgs.clear();
	smsg.clear();
}
//-------------------------------------------------------------------------
//  Read all parameters
//-------------------------------------------------------------------------
int CSwitchSet::Read (SStream *stream, Tag tag)
{
  SMessage *msg = 0;
  switch (tag) {
  case 'smsg':
    { msg = new SMessage;
      ReadMessage (msg, stream);
      msg->id        = MSG_SETDATA;
      msg->sender    = unId;
      msg->dataType  = TYPE_INT;
      msg->user.u.datatag = 'stat';
      msgs.push_back (msg);
    }
    return TAG_READ;
  //---Default position --------------
  case 'dflt':
    ReadInt(&indx,stream);
    return TAG_READ;

  case 'sync':
    sync = true;
    return TAG_READ;

  case 'LAND':
    mveh->SetLandMSG(unId);
    return  TAG_READ;

  case 'TAXI':
    mveh->SetTaxiMSG(unId);
    return TAG_READ;

  case 'NAVI':
    mveh->SetNaviMSG(unId);
    return TAG_READ;

  case 'STRB':
    mveh->SetStrbMSG(unId);
    return TAG_READ;

  case 'zero': 
    zBase = 0;
    return TAG_READ;

  case 'ssOn':
    ReadInt (&onState, stream);
    return TAG_READ;

  case 'ssOf':
    ReadInt (&offState, stream);
    return TAG_READ;
  }

    // See if the tag can be processed by the parent class type
  return CDependent::Read (stream, tag);
}
//----------------------------------------------------------------------------
//  Read finished
//  Initial state default is OFF
//----------------------------------------------------------------------------
void CSwitchSet::ReadFinished()
{ CDependent::ReadFinished();
  state = 0;
  indx -= zBase;
  return;
}
//----------------------------------------------------------------------------
//  Send synchro messages
//----------------------------------------------------------------------------
void CSwitchSet::SynchroAll()
{ int val = (active)?(onState):(offState);
  std::vector<SMessage*>::iterator im;
  for (im = msgs.begin(); im != msgs.end(); im++)
  { SMessage *msg = (*im);
    msg->intData  = val;
    Send_Message(msg);
  }
  return;
}
//----------------------------------------------------------------------------
//  Change position
//  mode is sync:
//    The set is active if index is not 0.  Then all subsystems are set to the
//    current state:
//  mode is exclusive:
//    Only the subsystem corresponding to the index (1 based) is set.  All other
//    are reset
//----------------------------------------------------------------------------
void CSwitchSet::ChangePosition(int inx)
{ //--- check position -----------------------
  int pos = inx - zBase;                    // Corrected position
  if (pos <  0)                 return;     // Ignore
  if (pos == indx)              return;     // Same position
  indx    = pos;
  //--- Update state according to index ---------
  active  = (pos != 0);
  state   = active;
  //--- Send corresponding message --------------
  if (sync) return SynchroAll();
  //--- Activate system corresponding to index --
  int No = 0;
  while (No != int(msgs.size()))
  { SMessage *msg = msgs[No];
    msg->intData  = (No == pos)?(onState):(offState);
    Send_Message(msg);
    No++;
  }
  return;
}
//----------------------------------------------------------------------------
//  Intercept Messages
//----------------------------------------------------------------------------
EMessageResult CSwitchSet::ReceiveMessage (SMessage *msg)
{ if (msg->id != MSG_SETDATA ) return CDependent::ReceiveMessage(msg);
  int old = state;
  int inx;
  switch (msg->user.u.datatag) {
      //----Set the state ------------------------
      case 0:
      case 'st8t':
      case 'stat':
      case 'nabl': 
          state = (msg->intData != 0);
          if (old != state) ChangePosition(state);
		      return MSG_PROCESSED;
      //---- swap the state ---------------------
      case 'swap':
          state ^= 1;
          ChangePosition(state + zBase);
          return MSG_PROCESSED;
      //---- Change active system ---------------
      case 'indx':
          inx = msg->intData;
          ChangePosition(inx);
          return MSG_PROCESSED;
      //-----------------------------------------
  }
  return CDependent::ReceiveMessage(msg);
}
//----------------------------------------------------------------------------
//  Probe switchset
//----------------------------------------------------------------------------
void CSwitchSet::Probe(CFuiCanva *cnv)
{ CDependent::Probe(cnv,0);
  char edt[16];
  _snprintf(edt,16,"Indx= %d",indx);
  cnv->AddText( 1,edt,1);
  return;
}

//===================END OF FILE =============================================