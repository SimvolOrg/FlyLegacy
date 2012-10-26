/*
 * GaugeSubsystems.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
 * Copyright 2005 Laurent Claudet
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
#include "../Include/Weather.h"
#include "../Include/Globals.h"
#include "../Include/Fui.h"
#include "../Include/Atmosphere.h" // CAirspeedIndicator
#include "../Include/MagneticModel.h"
using namespace std;

#ifdef _DEBUG	
  //#define _DEBUG_atti	
  //#define _DEBUG_AIRSPEED
#endif

//////////////////////////////// Utility function ///////////////////////////////

/*! \fn TransformInLeftHand
 *  \brief transforms coord in left-hand coord convention
 *  \param vect
 */
 void TransformInLeftHand (SVector &vect)
 {
   VectorDistanceLeftToRight  (vect);
 }

//////////////////////////////////////////////////////////////////////////////////
//
// CPitotStaticSubsystem
//
CPitotStaticSubsystem::CPitotStaticSubsystem (void)
{
  TypeIs (SUBSYSTEM_PITOT_STATIC_SUBS);

  gNum = 0;
}

int CPitotStaticSubsystem::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'gNum':
    // Port group number
    ReadInt (&gNum, stream);
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CDependent::Read (stream, tag);
  }

  return rc;
}
//----------------------------------------------------------------
//	JSDEV*		CAltimeter
//	ALtitude is computed with Kollman pressure correction
//	A variation of 924.83 feet is applied for 1 Hg of pressure
//	Norme is  27.31 feet for 1 hPa with 1hPa = 0.0295299 Hg
//----------------------------------------------------------------
CAltimeter::CAltimeter (void)
{ TypeIs (SUBSYSTEM_ALTIMETER);
  timK		    =	 0.5f;					// Default rate
  indnMode	  = INDN_LINEAR;	  // Default mode
  kollValue   = 29.92f;					// Standard sea level
  kollVariation		= 0;					// 0 feet ASL
  decisionHeight	= 200.0f;
  radarAlt  = 0.0f;
  hPres     = globals->atm->GetPressureInHG();
  d1  = 2;
  d2  = 9;
  d3  = 9;
  d4  = 2;
}
//----------------------------------------------------------------
//		Read the tags
//----------------------------------------------------------------
int CAltimeter::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CPitotStaticSubsystem::Read (stream, tag);
  }

  return rc;
}
//----------------------------------------------------------------
//  Update Koll setting
//----------------------------------------------------------------
U_INT CAltimeter::UpdatePressure(float inc)
{ kollValue += inc;
  //---Clamp result --------------------------
  if (kollValue < 25.01f)  kollValue  = 25.01f;
  if (kollValue > 34.99f)  kollValue  = 34.99f;
  return ComputeCorrection(kollValue);
}
//----------------------------------------------------------------
//  Compute Kollsman correction
//	Note Pressure diminution => Altitude increase
//  The new pressure is returned as integer in the message
//----------------------------------------------------------------
U_INT CAltimeter::ComputeCorrection(float kpr)
{	kollValue = kpr;
  kollVariation	= (kpr - hPres) * KOLLMAN_CTE;
  //-----Compute digits ------------------------
  float pr = kpr;
  d1  = int(pr / 10);
  pr  = fmod(pr,10);
  d2  = int(pr);
  int pf  = (kpr - (10 * d1) - (d2)) * 100;
  d3  = (pf / 10);
  d4  = (pf % 10);
	return ((d1 * 1000)+ (d2 * 100) + (d3 * 10) + d4);
}
//----------------------------------------------------------------
//		Reply to messages
//----------------------------------------------------------------
EMessageResult CAltimeter::ReceiveMessage (SMessage *msg)
{
  EMessageResult rc = MSG_IGNORED;

    if (msg->id == MSG_GETDATA) {
      switch (msg->user.u.datatag) {
      case 'alti':
        // Barometric altitude
        msg->realData	= indn;
        return MSG_PROCESSED;

      case 'rAlt':
        // Radar altitude AGL
        msg->realData	= radarAlt;
        return MSG_PROCESSED;

      case 'baro':
        // Kollsman setting
        { //---Set digits in integer part ------
          U_INT pr      = ((d1 << 24) | (d2 << 16) | (d3 << 8) | d4);
          msg->intData  = pr;
          return MSG_PROCESSED;
        }

      case 'dh__':
        // Decision height
        msg->realData	= decisionHeight;
        return MSG_PROCESSED;

      }
      return CPitotStaticSubsystem::ReceiveMessage (msg);
    }

    if (msg->id == MSG_SETDATA) {
      switch (msg->user.u.datatag) {
      case 'alti':
			  // Barometric altitude
			  indnTarget		= (float)msg->realData;
			  return MSG_PROCESSED;
      //---Process new pressure setting ------------------------
      case 'baro':
			// Kollsman setting
        { msg->intData  = ComputeCorrection((float)msg->realData);
		      return MSG_PROCESSED;
        }
      //----Modify kollmans setting ----------------------------
      case 'knob':
        msg->intData  = UpdatePressure(msg->realData);    // New pressure
        return MSG_PROCESSED;
      //--------------------------------------------------------
      case 'dh__':
			// Decision height
			  decisionHeight	= (float)msg->realData;
	      return MSG_PROCESSED;
	  }
    return CPitotStaticSubsystem::ReceiveMessage (msg);
	}
  return CPitotStaticSubsystem::ReceiveMessage (msg);
}
//-----------------------------------------------------------------------
//			Compute altitude in feet with kollsman correction
//-----------------------------------------------------------------------
void CAltimeter::TimeSlice (float dT,U_INT FrNo)				// JSDEV*
{ // Get ground altitude below aircraft
  SPosition pos = mveh->GetPosition();									
  radarAlt      = globals->tcm->GetGroundAltitude();
  hPres         = globals->atm->GetPressureSLinHG();
  kollVariation	= (kollValue - hPres) * KOLLMAN_CTE;
  // Set indication target to barometric altitude
  indnTarget = (float)(pos.alt + kollVariation);
  CPitotStaticSubsystem::TimeSlice (dT,FrNo);					// JSDEV*
}
//-----------------------------------------------------------------------
//			Probe the altimeter
//-----------------------------------------------------------------------
void CAltimeter::Probe(CFuiCanva *cnv)
{ char edt[20];
  CDependent::Probe(cnv,0);
  sprintf_s(edt,16,"Radar: %.0f",radarAlt);
  cnv->AddText( 1,edt,1);
  return;
}
//=======================================================================
//
// CVerticalSpeedIndicator
//
//=======================================================================
CVerticalSpeedIndicator::CVerticalSpeedIndicator (void)
{ TypeIs (SUBSYSTEM_VERTICAL_SPEED);
  hwId  = HW_GAUGE;
  old   = 0.0;
}
//------------------------------------------------------------------------------
//  Read parameters
//------------------------------------------------------------------------------
int CVerticalSpeedIndicator::Read (SStream *stream, Tag tag)
{  // See if the tag can be processed by the parent class type
    return CPitotStaticSubsystem::Read (stream, tag);
}
//------------------------------------------------------------------------------
//  All parameters are read
//------------------------------------------------------------------------------
void CVerticalSpeedIndicator::ReadFinished()
{ timK = 2;
  indnMode  = INDN_LINEAR;
	mveh->RegisterVSI(this);
}
//------------------------------------------------------------------------------
//  Receive the message
//------------------------------------------------------------------------------
EMessageResult CVerticalSpeedIndicator::ReceiveMessage (SMessage *msg)
{
  switch (msg->id) {
    case MSG_GETDATA:
      switch (msg->user.u.datatag) {
        case 'vsi_':
          msg->realData = indn;
          return MSG_PROCESSED;
        //---Normalized VSI: 1=> 100 feet/mn
        case 'nvsi':
          msg->realData = indn * 0.01;
          return MSG_PROCESSED;
        //---Feet per second ---------------------
        case 'vfs_':
          msg->realData = vfs;
          return MSG_PROCESSED;
        }
    case MSG_SETDATA:
      switch (msg->user.u.datatag) {
        case 'vsi_':
          indnTarget  = (float)msg->realData;
          return MSG_PROCESSED;
      }
  }

  return CPitotStaticSubsystem::ReceiveMessage (msg);
}
//------------------------------------------------------------------------------
//
//  Compute Vertical Speed in Feet / minute
//  The normalized VSI is a value where
//  1 is 100 feet up per minute.
//------------------------------------------------------------------------------
void CVerticalSpeedIndicator::TimeSlice (float dT,U_INT FrNo)
{ double alt = mveh->GetAltitude();			
  // compute vertical speed over time (1 minute)
  vfs         = (alt - old) / dT;
  indnTarget  = float(vfs * 60.0);
  old         = alt;
  //
  CPitotStaticSubsystem::TimeSlice (dT,FrNo);
}


//==============================================================================
// CAirspeedIndicator
//==============================================================================
CAirspeedIndicator::CAirspeedIndicator (void)
{ TypeIs (SUBSYSTEM_AIRSPEED);
  maxOperatingMach = 0;
  maxOperatingSpeed = 0;
}
//--------------------------------------------------------------------------------
//  Read all parameters
//--------------------------------------------------------------------------------
int CAirspeedIndicator::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'rMmo':
    // Maximum operating Mach number
    ReadFloat (&maxOperatingMach, stream);
    rc = TAG_READ;
    break;
  case 'rVmo':
    // Maximum operating speed
    ReadFloat (&maxOperatingSpeed, stream);
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CPitotStaticSubsystem::Read (stream, tag);
  }

  return rc;
}
//--------------------------------------------------------------------------------
//  Message received
//--------------------------------------------------------------------------------
EMessageResult CAirspeedIndicator::ReceiveMessage (SMessage *msg)
{ switch (msg->id) {
    case MSG_GETDATA:
      switch (msg->user.u.datatag) {
			case '$VAL':
      case 'sped':
      case 'kias': // 
        msg->realData = indn;
        return MSG_PROCESSED;
      }
      break;

    case MSG_SETDATA:
      switch (msg->user.u.datatag) {
      case 'sped':
          indnTarget = (float)msg->realData;
          return MSG_PROCESSED;
      }
  }

 return CPitotStaticSubsystem::ReceiveMessage (msg);
}
//--------------------------------------------------------------------------------
//  Time slice the airspeed in KIAS
//--------------------------------------------------------------------------------
void CAirspeedIndicator::TimeSlice (float dT,U_INT FrNo)	// JSDEV*	
{ double ias = mveh->GetPreCalculedKIAS ();						//
  indnTarget = ias;
  /// ----------------------------------
  CPitotStaticSubsystem::TimeSlice (dT,FrNo);							// JSDEV*
}

//===============================================================================
//	JSDEV*
//		CPneumaticSubsystem
//		Compute pumps pressure for derived classes
//===============================================================================
CPneumaticSubsystem::CPneumaticSubsystem (void)
{
	TypeIs (SUBSYSTEM_PNEUMATIC);
	timer		= 0;
	regulated   = false;
	indn		= 0.6f;				// Initial for Gyro
}
//--------------------------------------------------------------------------------
//  Destructor
//--------------------------------------------------------------------------------
CPneumaticSubsystem::~CPneumaticSubsystem()
{	mPmp.clear();
}
//--------------------------------------------------------------------------------
//  Read all parameters
//--------------------------------------------------------------------------------
int CPneumaticSubsystem::Read (SStream *stream, Tag tag)
{ SMessage msg;
  switch (tag) {

  case 'mPmp':
    {
      // Pneumatic pump message
		ReadMessage (&msg, stream);
		msg.id        = MSG_GETDATA;
    msg.sender    = unId;
		msg.dataType  = TYPE_REAL;
		mPmp.push_back (msg);
    }
    return TAG_READ;

  case 'regd':
    // System is pressure regulated
    regulated = true;
    return TAG_READ;
  }

  return CDependent::Read (stream, tag);
}
//----------------------------------------------------------------------------
//	JSDEV* Prepare Monitoring messages
//----------------------------------------------------------------------------
void CPneumaticSubsystem::PrepareMsg(CVehicleObject *veh)		
{	std::vector<SMessage>::iterator msg;
	for (msg = mPmp.begin(); msg != mPmp.end(); msg++)
	{ veh->FindReceiver(&(*msg));
	}
	//-----Call Other dependencies ----
	CDependent::PrepareMsg(veh);
	return;	}
//----------------------------------------------------------------------------
//	JSDEV* Update the pressure inside the system
//	NOTES
//	1	We update first the dependencies in case for some implementations
//		the subsystem is rendered inoperative
//	2	As pumps have some inertie, it is no necessary to poll pumps at
//		every time slice.  Actually they are polled every 1/4 sec or so.
//		If needed, the timer may be shortened or even suppressed
//	3	Pressure is in PSI
//  4 In debug mode (subsystem windows active) the pressure is established
//----------------------------------------------------------------------------
void CPneumaticSubsystem::TimeSlice (float dT,U_INT FrNo)		
{ timer	+= dT;
	if (timer < 0.25)				return;						// Skip update
	timer	 = 0;
	CDependent::TimeSlice (dT,FrNo);							// Update acivity							
	if (!active){	indnTarget = 0; return; }
	//-----compute total pressure from all pumps ------------------------------
	float press = 0;											// Total pressure
	std::vector<SMessage>::iterator pm;
	for (pm = mPmp.begin (); pm != mPmp.end (); pm++)
	{	Send_Message (&(*pm));
		if ((*pm).realData > press) press = (*pm).realData;									
	}

  // Assign to indication target
	indnTarget = press;
}
//===============================================================================
// JSDEV* CPneumaticPump
//	Use indn as pump pressure
//===============================================================================
CPneumaticPump::CPneumaticPump (void)
{ TypeIs (SUBSYSTEM_PNEUMATIC_PUMP);
  ratK			= 0.5;			// Default ratK
  mode			= INDN_LINEAR;
  suct          = 2.0f;			// residual pressure for gyro animation at start up
  eNum          = 1;			  ///< default engine number
  dIce          = 2;			  ///< the pressure required to operate the de-ice inflatable boots
  Lrpm          = 200;			///< the least RPM required to create sufficient succion
  Hrpm          = 5000;			///< the greatest RPM tolerated to create sufficient succion
  mSct          = 1.0;			///< the suction generated at the Least Operative RPM
  stat          = false;       ///< the initial pump dpnd state
};
//-------------------------------------------------------------------------
//  Read parameters
//-------------------------------------------------------------------------
int CPneumaticPump::Read (SStream *stream, Tag tag)
{ switch (tag) {
    case 'suct':
      // Suction value
      ReadFloat (&suct, stream);
      return TAG_READ;
  
    case 'dIce':
      ReadInt (&dIce, stream);
      return TAG_READ;

    case 'Lrpm':
      ReadInt (&Lrpm, stream);
      return TAG_READ;

    case 'Hrpm':
      ReadInt (&Hrpm, stream);
      return TAG_READ;

    case 'mSct':
      ReadFloat (&mSct, stream);
      return TAG_READ;
  }
    return CEngineControl::Read (stream, tag);
}
//------------------------------------------------------------------
//	Compute proportional coefficient
//	Coef = (suct - mSct) / (Hrpm - Lrpm)
//	Pump pressure will be proportional to engine rpm
//------------------------------------------------------------------
void   CPneumaticPump::ReadFinished (void)
{ CEngineControl::ReadFinished();
  Monitor('erpm');
  //---- compute proportional coeff -----------------------
  float	dy		= suct - mSct;
  float	dx		= Hrpm - Lrpm;
  Coef	= (dx <= 0.0001)?(0.5f):(dy /dx);		// Nul dx => Coef 0.5
  return;
}
//------------------------------------------------------------------
//  Probe the pump
//------------------------------------------------------------------
void CPneumaticPump::Probe(CFuiCanva *cnv)
{ char edt[16];
  CDependent::Probe(cnv,0);
  _snprintf(edt,16,"Coef %.4f",Coef);
  cnv->AddText( 1,edt,1);
  _snprintf(edt,16,"RPM  %.4f",rpm);
  cnv->AddText( 1,edt,1);
  return;
}
//------------------------------------------------------------------
//	Receive message
//-------------------------------------------------------------------
EMessageResult CPneumaticPump::ReceiveMessage (SMessage *msg)
{ switch (msg->id) {
    case MSG_GETDATA:
      switch (msg->user.u.datatag) {
      case 'inop':
        msg->intData  = (state == 0);
        return MSG_PROCESSED;


      default :
        msg->realData = indn;							// JSDEV*
        return MSG_PROCESSED;
      }
    }

   // See if the message can be processed by the parent class
   return CEngineControl::ReceiveMessage (msg);
}
//------------------------------------------------------------------------
//	JSDEV*
//		Compute pressure as a function of engine RPM
//------------------------------------------------------------------------
void CPneumaticPump::TimeSlice (float dT,U_INT FrNo)			
{ CEngineControl::TimeSlice(dT,FrNo);								
  // get engine rpm
  Send_Message(&engm);
  rpm = engm.realData;
  indnTarget = 0.0;
  if (rpm  > Hrpm)  {indnTarget	= suct;	return;}
  if (rpm  > Lrpm)  {indnTarget = mSct + (rpm * Coef);  return; }
	return;
}

//===============================================================================
// CAttitudeIndicator
//===============================================================================
CAttitudeIndicator::CAttitudeIndicator (void)
{
  TypeIs (SUBSYSTEM_ATTITUDE);
  hdeg            = 0;
  prat            = 1.0;
  regulated_flag  = false;
  P_LIMIT         = 30;								/* 30 degrees */;
  R_LIMIT         = 48;								 /* 48 degrees */;
  //-----------------------------------------------------
	operP			= 1.0;			// To adjust
  delay     = 0.0f;
	Etat			= INTROPC;
	incp			= 0;
	incr			= 0;
	Timer			= 0;
	Uatt.x			= 0;					// Pitch
  Uatt.y      = 0;          // Not used
	Uatt.z			= 0;					// Roll
}

//----------------------------------------------------------------------------------
int CAttitudeIndicator::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'prat':
    // Precession rate
    ReadFloat (&prat, stream);
    rc = TAG_READ;
    break;

  case 'regd':
    // regulated flag
    regulated_flag = true; // non used yet
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CPneumaticSubsystem::Read (stream, tag);
  }

  return rc;
}
//----------------------------------------------------------------------------------
//  Update Horizon level
//----------------------------------------------------------------------------------
int CAttitudeIndicator::UpdateLevel(int inc)
{ hdeg += inc;
  if (hdeg < -15)   hdeg = -15;
  if (hdeg > +15)   hdeg = +15;
  return hdeg;
}
//----------------------------------------------------------------------------------
//  Return pitch as a float
//----------------------------------------------------------------------------------
float CAttitudeIndicator::GaugeBusFT01()
{	return Clamp180(Uatt.x);	}
//----------------------------------------------------------------------------------
//  Return roll as a float
//----------------------------------------------------------------------------------
float CAttitudeIndicator::GaugeBusFT02()
{	return Wrap360(Uatt.z);	}
//----------------------------------------------------------------------------------
//  receive message
//----------------------------------------------------------------------------------
EMessageResult CAttitudeIndicator::ReceiveMessage (SMessage *msg)
{
  EMessageResult rc = MSG_IGNORED;

    if (msg->id == MSG_GETDATA) {
      switch (msg->user.u.datatag) {
      case 'atti':
        // returns whether the atti is enabled; JS is this a real data or int?
        msg->realData = (indn > operP);
        return MSG_PROCESSED;

      case 'roll':
        // returns roll in deg (0..360)
		    msg->intData  = Wrap360(int(Uatt.z));
        return MSG_PROCESSED;

      case 'pich':
        // returns pitch in deg (-180..180)
        msg->realData = Clamp180(Uatt.x);
        return MSG_PROCESSED;

      case 'knob':
        msg->intData  = hdeg;
        return MSG_PROCESSED;
      }
      return CPneumaticSubsystem::ReceiveMessage (msg);
    }
    if (msg->id == MSG_SETDATA) {
    switch(msg->user.u.datatag) {
      case 'knob':
        msg->intData  = UpdateLevel(msg->intData);
        return MSG_PROCESSED;
      }
    return CPneumaticSubsystem::ReceiveMessage (msg);
    }

      // See if the message can be processed by the parent class
  return CPneumaticSubsystem::ReceiveMessage (msg);
}
//-------------------------------------------------------------------------------
//  Probe susbsystem
//-------------------------------------------------------------------------------
void CAttitudeIndicator::Probe(CFuiCanva *cnv)
{ char edt[64];
  CDependent::Probe(cnv,0);
  cnv->AddText(1,1,"Etat : %d",Etat);
  cnv->AddText(1,1,"ndl Pitch : %.4f",Uatt.x);
  cnv->AddText(1,1,"ndl Roll  : %.4f",Uatt.z);
  //--- Edit the pitch angle from dang ------
  _snprintf(edt,64,"deg pitch x = %.2f",globals->dang.x);
  cnv->AddText(1,edt,1);
	_snprintf(edt,64,"deg roll  y = %.2f",globals->dang.y);
	cnv->AddText(1,edt,1);
  return;
}
//-------------------------------------------------------------------------------
//	JSDEV* Tarck target attitude
//	-return true when target attitude is reached
//-------------------------------------------------------------------------------
bool CAttitudeIndicator::TrackTarget(float dT)
{	float ajp = Tatt.x - Uatt.x;					  // Delta to target picth
	float dtp = fabs(ajp);							    // Absolute delta to picth
	float ajr = Tatt.z - Uatt.z;					  // Delta to target roll
	float dtr = fabs(ajr);							    // Absolute delta to roll
	if (dtp > 0.5)	Uatt.x	+= (ajp * dT);	// Proportional correction to pitch
	if (dtr > 0.5)	Uatt.z  += (ajr * dT);	// Proportional correction to roll
//TRACE("TRACK Pitch %.4f Roll %.4f",Uatt.x,Uatt.z);
	return ((dtp <= 0.5) && (dtr <= 0.5));
}
//-------------------------------------------------------------------------------
//	JSDEV* CAttitude indicator
//	This device follows the following cycles
//		STOP->PRECESSION->RUNNING->PRECESSION->STOP
//	It is supplied by at least one pneumatic pump.
//	Computation done in degre in left hand coordinate system
//	NOTES:	Operational pressure depend on pump. If pump does not give
//			enough pressure or is too slow for the prate, the behaviour
//			of the ball may be eractic.
//			Pressure values wiil be adjusted latter when engine is more
//			mature
//-------------------------------------------------------------------------------
void CAttitudeIndicator::TimeSlice(float dT,U_INT FrNo)
{	//--- Update pressure value ------------------------------------------------- 
  
	CPneumaticSubsystem::TimeSlice(dT,FrNo);							
	switch (Etat){
		case STOPPED:
			if (indn < 0.5)				return;				// Not enough PSI
			Etat	= INTROPC;
			return;

		case INTROPC:
			//------Change state to Precession ---------------
			Etat	= PRECESS;								    // PRECESS state
			Tatt.x	= -P_LIMIT;
			Tatt.z	= -R_LIMIT;
			delay	= (prat / 360);
			Timer	= 0;									        // arm timer
			incp	= (Uatt.x > Tatt.x)?(-5.0f):(+5.0f);
			incr	= (Uatt.z > Tatt.z)?(-5.0f):(+5.0f);
			return;

		case PRECESS:
			//--- Todo: for more realistic effect the attitute should converge to the vehicle one
			//	To try		define decreasing limits arround the vehicle one
			//		[-Lp0,veh(p),+Lp0] ... [-Lpk,veh(p),+Lpk]  
			//		where -Lp0 < .. <-Lpk  and Lp0 > ... > Lpk
			{Timer  += dT;	
			if (Timer < delay)								  // Precession period
			{	Uatt.x	+= incp;
				Uatt.z	+= incr;
				if ((incp > 0) && (Uatt.x >= Tatt.x))
				{	Tatt.x	= -(P_LIMIT);						// invert pitch limit -P_LIMIT
					incp	= (-5.0f);							  // Change incp
				}
				if ((incp < 0) && (Uatt.x <= Tatt.x))
				{	Tatt.x	= +(P_LIMIT);						// invert pitch limit +P_LIMIT
					incp	= (+5.0f);							  // Change increment
				}
				if ((incr > 0) && (Uatt.z >= Tatt.z))
				{	Tatt.z	= -(R_LIMIT);						// invert roll limit
					incr	= (-5.0f);							  // Change incp
				}
				if ((incr < 0) && (Uatt.z <= Tatt.z))
				{	Tatt.z	= +(R_LIMIT);						// invert roll limit
					incr	= (+5.0f);							  // Change increment
				}
				return;	}
			}


			// ----- ENd of precession ------------------------
			if (indn  > 1.2)	{							// pressure still building
				Etat	= TO_USER;
				return;
			}
			Etat		  = TOLIMIT;							// Else to stop
			Tatt.x		= P_LIMIT;
			Tatt.z		= R_LIMIT;
      return;	
		case TO_USER:										// Track user attitude
			mveh->GetRRtoLDOrientation(&Tatt);
			if (TrackTarget(dT) == false)		return;
			Etat	= RUNING;
			return;

		case TOLIMIT:
			if (TrackTarget(dT) == false)		return;
			//------ End of Limit -----------------------------------
			Etat	= STOPPED;
			return;	

		case RUNING:
			//-------Uatt is transformed in left hand   -------------------
			mveh->GetRRtoLDOrientation(&Uatt);
			if (indn > operP)						 return;
			Etat	= INTROPC;
			return;
	}
	return;	
}

//======================================================================
// CDirectionalGyro
//======================================================================
CDirectionalGyro::CDirectionalGyro (void)
{
  TypeIs (SUBSYSTEM_DIRECTIONAL_GYRO);

  step			= 1.0f;
  aYaw			= 0;
  tYaw			= 0;
  regulated		= false;
  autoAlign		= false;
  eRate			= 0.0f;
  aRat			= 0.0f;
  Error			= 0.0f;
  gyro      = 0;
  rbug      = 0;
  abug      = 0;
}
//----------------------------------------------------------------------
//  read all parameters
//----------------------------------------------------------------------
int CDirectionalGyro::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'step':
    // Autopilot bug step size
    ReadFloat (&step, stream);
    rc = TAG_READ;
    break;

  case 'aRat':
    // Auto align rate
    ReadFloat (&aRat, stream);
    rc = TAG_READ;
    break;

  case 'nErr':
    // Auto aligned
    autoAlign = true;
    rc = TAG_READ;
    break;

  case 'timK':
    // error 1° in 12'
    ReadFloat (&eRate, stream);
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CPneumaticSubsystem::Read (stream, tag);
  }

  return rc;
}
//--------------------------------------------------------------
//  Update Gyro °
//--------------------------------------------------------------
void CDirectionalGyro::UpdateGyro(float inc)
{ gyro += inc;
  if (0   > gyro) gyro = 359;
  if (359 < gyro) gyro = 0;
  return;
}
//--------------------------------------------------------------
//  Update auto pilot bug
//--------------------------------------------------------------
void CDirectionalGyro::UpdatePbug(float inc)
{ rbug += inc;
  if (0   > rbug) rbug = 359;
  if (359 < rbug) rbug = 0;
  return;
}
//--------------------------------------------------------------
//  Receive a message
//---------------------------------------------------------------
EMessageResult CDirectionalGyro::ReceiveMessage (SMessage *msg)
{
   if  (msg->id == MSG_GETDATA) {
      switch (msg->user.u.datatag) {
        //--- Actual yaw (absolute heading) --------------
      case 'yaw_':
        msg->realData	    = aYaw;					// Actual yaw
        msg->user.u.unit  = abug;         // Pilot position
        return MSG_PROCESSED;
        //--- Bug deviation to actual yaw -----
      case 'dBug':
        msg->realData = rbug;
			  return MSG_PROCESSED;
      }
      return CSubsystem::ReceiveMessage (msg);
    }

    if (msg->id == MSG_SETDATA) {
      switch (msg->user.u.datatag) {

      case 'dgyr':
        UpdateGyro(msg->realData);
        msg->intData  = int(gyro);
			  return MSG_PROCESSED;
      case '_Bug':
        UpdatePbug(msg->realData);
        msg->intData      = abug;
        msg->user.u.unit  = int(rbug);
			  return MSG_PROCESSED;
      }
     return CSubsystem::ReceiveMessage (msg);
    }

  return CSubsystem::ReceiveMessage (msg);
}
//-------------------------------------------------------------------
//	JSDEV* CDirectionalGyro
//			Computation done in degres in aircraft coordinates
//	NOTE: Dont use indn as it is used for pressure value
//        Vacuum is taken from the base class.
//  NOTE: Directional Gyro gives magnetic direction. The global variable
//				magDEV maintains the average magnetic deviation from the 
//				geographic north pole. So
//				Yaw = H - M; where H= aircraft heading and M = magnetic deviation
//				When yaw = 0 we have H=M. 
//				If the dial is not aligned with the north, the indicated
//				yaw is false.
//	To get magnetic deviation in degre at a position, call
//	The deviation is returned in decl_degrees
//	CMagneticModel::Instance().GetElements (this->GetPosition (), decl_degrees_, hor_field_);
//  
//--------------------------------------------------------------------
void CDirectionalGyro::TimeSlice (float dT,U_INT FrNo)		
{	// Update the pump value (indn) from the parent class
	CPneumaticSubsystem::TimeSlice(dT,FrNo);					
	// Compute compass indication from aircraft heading, error and knob.
	SVector ori;
  ori.h  = 0;
	mveh->GetRRtoLDOrientation(&ori);
	if (!autoAlign) Error = eRate * dT;						            // proportional error
	tYaw	= Wrap360(ori.h + Error + gyro - globals->magDEV);	// target yaw
	//-----Compute actual yaw ----------------------------
	float fac	= (aRat)?(dT / aRat):(0.25);					    // Correction factor
	if (fac >= 1)	fac = 0.25;								            // dont overshoot target
  float df =  (tYaw - aYaw);
  if (df < -180) df += 360;
  if (df > +180) df -= 360;
	aYaw		+=  df * fac;		                            // converge to target
  aYaw     = Wrap360(aYaw);
  abug     = int(Wrap360(360 - aYaw + rbug));         // Bug position on plate
	return;
  }

//===================================================================
//	CVacuumIndicator gives pressure of connected pumps
// All functions are implemented in base class
//===================================================================
CVacuumIndicator::CVacuumIndicator (void)
{ TypeIs (SUBSYSTEM_VACUUM_INDICATOR);
  hwId = HW_GAUGE;
}
//-----------------------------------------------------------------
int CVacuumIndicator::Read (SStream *stream, Tag tag)
{ // See if the tag can be processed by the parent class type
  return  CPneumaticSubsystem::Read (stream, tag);
}
//---------------------------------------------------------------
//  Receive a message
//---------------------------------------------------------------
EMessageResult CVacuumIndicator::ReceiveMessage (SMessage *msg)
{
  switch (msg->id) {
    case MSG_GETDATA:
      switch (msg->user.u.datatag) {
      case 'indn' :
        msg->realData = indn;
        return MSG_PROCESSED;
      }
    }

  return CDependent::ReceiveMessage (msg);
}
//===================================================================
// JSDEV* CTurnCoordinator
//===================================================================
CTurnCoordinator::CTurnCoordinator (void)
{
  TypeIs (SUBSYSTEM_TURN_COORDINATOR);
	Head	= 0;					// Previous heading
	rateT	= 0;					// Computed rate
	tilt	= 1;					// default Heading damping rate
  rateD = 0;
}

//------------------------------------------------------------------
//	Read the tags
//------------------------------------------------------------------
int CTurnCoordinator::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'tilt':
    // Tilt ??
    ReadFloat (&tilt, stream);
    rc = TAG_READ;
    tilt = 0.25f;
    break;
  }

  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CDependent::Read (stream, tag);
  }

  return rc;
}
//------------------------------------------------------------------
//	Reply to message
//------------------------------------------------------------------
EMessageResult CTurnCoordinator::ReceiveMessage (SMessage *msg)
{
  switch (msg->id)
  {
    case MSG_GETDATA:
    {  
      switch (msg->user.u.datatag)
      {
        //----Aircraft rate turn -----
        case 'turn':
	        msg->realData	= rateT;
	        return MSG_PROCESSED;

        //----Rate turn in degre per minute ----
        case 'ratD':
          msg->realData = rateD;
          return MSG_PROCESSED;

        //----Aircraft immediate rate turn -----
        case 'itrn':
	        msg->realData	= rateT;
	        return MSG_PROCESSED;

        //---Lateral acceleration ----------
        case 'ball':
	        msg->realData	= indn;
	        return MSG_PROCESSED;

        //--Heading angle in degre------------
        case 'Head':
          msg->realData	= Head;
	        return MSG_PROCESSED;
      }
    }
	}
  return  CDependent::ReceiveMessage (msg);
}
//----------------------------------------------------------------------
//	JSDEV* Compute heading rate and lateral acceleration
//	Computation is in aircraft coordinate system (left hand) in degres
//  The turning angle is converted in a gradation G in the interval [-2,+2]
//  where +-1 is a 2minute circle (3° per second)
//  Ha is actual head
//  Hp is previous
//  Thus (Ha-Hp) is the amount of ° during dT.
//       (Ha-Hp) / dt = k°              Amount of ° per second
//        1G = k° / 3   => k = 3G       Gradation   = nber ° / 3  
//       (Ha-Hp) / dt = 3G
//       (Ha-Hp) /(3*dt) = G            
//----------------------------------------------------------------------
//	JS a LC:  Ne pas mettre de flag sur les subsystems pour calculer
//						les valeurs à la demande
//						Certain subsystems servent ailleurs que pour les gauges
//						et cela introduit des complications
//---------------------------------------------------------------------
void CTurnCoordinator::TimeSlice (float dT,U_INT FrNo)			// JSDEV*
{ SVector ori;
  mveh->GetRRtoLDOrientation(&ori);
  // ori is in left hand coordinate so y is the head axis in degre 
  float htp	= Head;											        // Previous in DEG
  Head	 = ori.y;												        // Actual   in DEG
  //---Compute rate turn in degre per second ---------------------------
  float sD  = ((Head - htp) / dT);              // Target rate in degre per sec
  rateD    += ((sD - rateD) * (dT * 10));       // damped rate turn
  //--- Compute rate turn for older gauges -----------------------------
  float sG  = ((Head - htp) / (3.0f * dT));		  // scaled target rate where 1 is 3°/s
  //---warning damping may overshoot the interval when dT > tilt -------
  rateT += (sG - rateT) * (dT / tilt);				  // Damp rate
  if (rateT > +2.0f)	rateT = +2.0f;					  // clamp to +2
  if (rateT < -2.0f)	rateT = -2.0f;					  // clamp to -2;
  //--- get acceleration vector for the ball ----------------------------
//    const CVector *acc = mveh->GetBodyAccelerationVector(); // LH m.s² 
  const CVector *acc = mveh->GetBodyVelocityVector(); // LH m.s² 
    ////----get Y lateral acceleration vector -----------------------------
    //float	dac = float((acc->y) / dT);		          // in rad.sec
    //if (dac > +TWO_PI)		dac = +float(TWO_PI);	  // clamp to +2
    //if (dac < -TWO_PI)		dac = -float(TWO_PI);	  // clamp to -2
    //indnTarget = dac; 
    //----get Y lateral acceleration vector -------------------------------
        //float	dac = float(acc->x) * dT * dT * 100.0f;			// in rad.sec²
//      double d = -acc->z; // z
//      if (d < 1.0) d = 1.0;
//      float dac = float(acc->y) / d * 10.0f; // was x
   float dac = float(-acc->x);
   //
   // if this computation doesn't fit our goals try this :
   //
   //const CVector *bacc = mveh->GetBodyAngularVelocityVector(); // LH rad.s² 
   //const CVector *iacc = mveh->GetInertialAngularVelocityVector(); // LH rad.s²
   //dac = (bacc->y - iacc->y) * 100.0f;
   /*/
      #ifdef _DEBUG	
      {	FILE *fp_debug;
	      if(!(fp_debug = fopen("__DDEBUG_1.txt", "a")) == NULL)
	      {
		      fprintf(fp_debug, "%f %f %f %f %f\n", acc->x, acc->y, acc->z, dac, dT);
		      fclose(fp_debug); 
      }	}
      #endif
   /*/
  if (dac > +2.0f)		dac = +2.0f;							    // clamp to +2
  if (dac < -2.0f)		dac = -2.0f;							    // clamp to -2
  indnTarget = dac;
  //---------------------------------------------------------------------
  CDependent::TimeSlice (dT,FrNo);								  // JSDEV*
}

//--------------------------------------------------------------------
//  Edit qty in probe
//--------------------------------------------------------------------
void CTurnCoordinator::Probe(CFuiCanva *cnv)
{ char edt[16];
  CDependent::Probe(cnv,0);
  _snprintf(edt,16,"%.04f",rateT);
  cnv->AddText( 1,"rateT");
  cnv->AddText(10,edt,1);

  _snprintf(edt,16,"%.04f deg",Head);
  cnv->AddText( 1,"Head");
  cnv->AddText(10,edt,1);


  _snprintf(edt,16,"%.04f",rateD);
  cnv->AddText( 1,"ratD");
  cnv->AddText(10,edt,1);
  return;
}

//========================================================================
// CMagneticCompass
//========================================================================
CMagneticCompass::CMagneticCompass (void)
{ TypeIs (SUBSYSTEM_MAGNETIC_COMPASS);
  hwId = HW_GAUGE;
}
//------------------------------------------------------------------------
// CMagneticCompass:  Receive a message
//------------------------------------------------------------------------
EMessageResult CMagneticCompass::ReceiveMessage (SMessage *msg)
{ switch (msg->id) {
    case MSG_GETDATA:
      msg->realData     = indn;
      msg->user.u.unit  = 360 - int(indn);
      return MSG_PROCESSED;
	}

  return CDependent::ReceiveMessage (msg);
}
//----------------------------------------------------------------------------
//	JSDEV Compass should give an angle in [0,+360] in degres
//  NOTE:  The returned diretion is NOT the North direction, but a value
//         to accomodate the strip picture.
//  True heading is (360 - indn)
//----------------------------------------------------------------------------
void CMagneticCompass::TimeSlice (float dT,U_INT FrNo)		// JSDEV*
{ indnTarget = mveh->GetMagneticDirection();			//float(MagneticDirection());
  //---Warning indn may overshoot range when dT > ratK ------------
  CDependent::TimeSlice(dT,FrNo);							// JSDEV*
}
//============================================================================
//  CNavigation.  Provides a pass to the radio system
//============================================================================
CNavigation::CNavigation (void)
{ TypeIs (SUBSYSTEM_NAVIGATION);
  hwId    = HW_GAUGE;
  radio   = 0;
  //--- Set default radio -----------------------
  msg.group		        = 'navi';
  msg.sender          = unId;
  msg.user.u.unit	    = 1;
  msg.user.u.hw       = HW_RADIO;
 	msg.id              = MSG_GETDATA;
  msg.dataType        = TYPE_REAL;
  msg.user.u.datatag  = 'getr';
}
//------------------------------------------------------------------
//	Process Receive Message
//	The nav gauge uses BUS_RADIO struct that is updated by the
//  master radio.
//    xOBS: External OBS in °
//    hDEV: Deviation between current NAV radial and OBS
//    gDEV: Glide slope deviation if NAV is an ILS
//------------------------------------------------------------------
EMessageResult  CNavigation::ReceiveMessage (SMessage *msg)
{	if (0 == radio) radio = mveh->GetRadioBUS();	
	switch (msg->id) {
	case MSG_GETDATA:
    switch (msg->user.u.datatag) {
    case 'gets':
      msg->voidData     = radio;
      return MSG_PROCESSED;
    case 'obs_':
      msg->intData      = (radio)?(radio->xOBS):(0);
      return MSG_PROCESSED;
      
    case 'xDev':
      msg->realData     = (radio)?(radio->hDEV):(0);
      msg->user.u.unit  = (radio)?(radio->ntyp == SIGNAL_ILS):(0);
      return MSG_PROCESSED;

    case 'glid':
      msg->realData     = (radio)?(radio->gDEV):(0);
      msg->user.u.unit  = (radio)?(radio->ntyp == SIGNAL_ILS):(0);
      return MSG_PROCESSED;

    case 'rfsh':
      if (0 == radio)   return MSG_PROCESSED;
      BUS_RADIO *rad  = (BUS_RADIO*)msg->voidData;
      *rad            = *radio;
       return MSG_PROCESSED;
    }

  case MSG_SETDATA:
    if (msg->user.u.datatag == 'setr') 
    { radio  = (BUS_RADIO*)msg->voidData;
      return MSG_PROCESSED;
    }
    if (msg->user.u.datatag == 'knob')
    { if (0 == radio)  return MSG_PROCESSED;  
      msg->intData  = SetXOBS(short(msg->realData));
      return MSG_PROCESSED;
    }

  }
  return CDependent::ReceiveMessage (msg);
}
//--------------------------------------------------------------------------
//  Change OBS from external
//--------------------------------------------------------------------------
int CNavigation::SetXOBS(short inc)
{ short obs  = radio->xOBS;
  obs += inc;
  if (  0 > obs) obs  = 359;
  if (359 < obs) obs  = 0;
  radio->xOBS = obs;
  return obs;
}
//============================================================================
// JSDEV CDigitalClockOAT
//============================================================================
CDigitalClockOAT::CDigitalClockOAT (void)
{
  TypeIs (SUBSYSTEM_DIGITAL_CLOCK_OAT);
  Volt	= 28.5;									// Init Latter
  uTime			= globals->tim->GetUTCDateTime();
  lTime			= globals->tim->GetLocalDateTime();
  delta			= 0;
  eTimer.Change(0);
  fTimer.Change(1);
  cTemp     = 0;
  fTemp     = 0;
}

//------------------------------------------------------------------
//	Time slice:  Update date time every half sec (average)
//------------------------------------------------------------------
void CDigitalClockOAT::TimeSlice(float dT, U_INT FrNo)
{	CDependent::TimeSlice(dT,FrNo);
	//-- update internal clocks ---------------------------
  eTimer.Update(dT);
  fTimer.Update(dT);
	delta	+= dT;
	if (delta <  0.5)	return;
	//-------Update all times ------------------------------
	delta	-= 0.5;
	uTime	= globals->tim->GetUTCDateTime();				  // Universal
	lTime	= globals->tim->GetLocalDateTime();				// Local
  EditHour();
	//------Update Temperature here-------------------------
  cTemp  = globals->atm->GetTemperatureC();
  fTemp  = globals->atm->GetTemperatureF();
	return;	
}
//------------------------------------------------------------------
//    Edit Hour minute
//------------------------------------------------------------------
void CDigitalClockOAT::EditHour()
{ _snprintf(lHour,8,"%02u:%02u",lTime.time.hour,lTime.time.minute);
  _snprintf(uHour,8,"%02u:%02u",uTime.time.hour,uTime.time.minute);
  return;
}
//------------------------------------------------------------------
//		Encode Integer as SDDD where S= sign DDD = 3 digits
//		This encode temperature Celsius or Far)
//------------------------------------------------------------------
char *CDigitalClockOAT::EditVal(int val)
{ _snprintf(dVal+1,7,"%03d",val);
  dVal[0] = (val < 0)?('-'):('+');
  return dVal;}
//------------------------------------------------------------------
//	Return the requested value
//	
//------------------------------------------------------------------
bool	CDigitalClockOAT::GetRequestedValue(SMessage *msg)
{	
	switch (msg->user.u.datatag)
  { case 'oatC':
      msg->realData = cTemp;
      return true;
    case 'oatF':
      msg->realData = fTemp;
      return true;
		case 'TMPC':
      msg->voidData = EditVal(cTemp);
			return true;
		case 'TMPF':
			msg->voidData = EditVal(fTemp);
			return true;
		case 'VOLT':
			msg->voidData = EditVal(int(Volt));
			return true;
		case 'uTim':
      msg->charPtrData = uHour;
			return true;
		case 'lTim':
      msg->charPtrData = lHour;
			return true;
		case 'eTim':
			// Note elapse timer is given in MMSS
      msg->charPtrData = eTimer.GetTime();
			return true;
		case 'fTim':
			// Note Flight time is given in MMSS
      msg->charPtrData = fTimer.GetTime();
			return true;
			break;
	}
	return false;	
}
//------------------------------------------------------------------
//	Set the requested value
//------------------------------------------------------------------
bool	CDigitalClockOAT::SetRequestedValue(SMessage *msg)
{	switch (msg->user.u.datatag)
	{
		case 'eTrz':
			//- Toggle eTrz state ----------------
			eTimer.Toggle();
			return true;

	}
	return false;
}
//------------------------------------------------------------------
//	Process Receive Message
//	
//------------------------------------------------------------------
EMessageResult  CDigitalClockOAT::ReceiveMessage (SMessage *msg)
{ switch (msg->id) {
	  case MSG_GETDATA:
		  if (GetRequestedValue(msg))	return MSG_PROCESSED;
		  return MSG_IGNORED;
    case MSG_SETDATA:
		  if (SetRequestedValue(msg))	return MSG_PROCESSED;
		  return MSG_IGNORED;
    }

  return CDependent::ReceiveMessage (msg);
}
//=========================================================================
// CMarkerPanel
//==========================================================================
CMarkerPanel::CMarkerPanel (void)
{ TypeIs (SUBSYSTEM_MARKER_PANEL);
  hwId  = HW_OTHER;
  Radio = 0;
  rMSG.group  = 'Radi';
  //--Init outter marker -------------------------
  outm.blnk   = 0.50f;
  outm.endt   = 1;
  outm.sbuf   = globals->snd->GetSoundBUF('outr');
  //--Init medium marker -------------------------
  medm.blnk   = 0.25f;
  medm.endt   = 0.50f;
  medm.sbuf   = globals->snd->GetSoundBUF('midl');
  //---Init inner marker -------------------------
  inrm.blnk   = 0.25f;
  inrm.endt   = 0.50f;
  inrm.sbuf   = globals->snd->GetSoundBUF('innr');
}
//---------------------------------------------------------------
//  Destructor
//---------------------------------------------------------------
CMarkerPanel::~CMarkerPanel (void)
{ 
}
//---------------------------------------------------------------
//  Final initialization.  Get Primary radio
//---------------------------------------------------------------
void CMarkerPanel::PrepareMsg(CVehicleObject *veh)
{ rMSG.group  = 'Radi';
  rMSG.user.u.unit  = 1;
  rMSG.user.u.hw    = HW_RADIO;
  rMSG.user.u.datatag = 'getr';
  rMSG.id           = MSG_GETDATA;
  rMSG.voidData     = 0;
  Send_Message(&rMSG);
  Radio = (BUS_RADIO*)rMSG.voidData;
  return;
}
//---------------------------------------------------------------
//  Time Slice. 
//  -Check if radio is tunned for ILS.
//---------------------------------------------------------------
void CMarkerPanel::TimeSlice(float dT,U_INT FrNo)
{ CDependent::TimeSlice(dT,FrNo);
  if (0 == Radio)                 return;
  if (SIGNAL_ILS != Radio->ntyp)  return;
  CILS *ils = (CILS*)Radio->rSRC;
  CVector pos(Radio->hDEV,Radio->fdis,mveh->GetAltitude());
  if (ils->InOUTM(pos,outm)) outm.Flash(dT);
  if (ils->InMIDL(pos,medm)) medm.Flash(dT);
  if (ils->InINNR(pos,inrm)) inrm.Flash(dT);
  return;
}
//---------------------------------------------------------------
//  Receive message
//---------------------------------------------------------------
EMessageResult CMarkerPanel::ReceiveMessage (SMessage *msg)
{ if (msg->id != MSG_GETDATA) return CDependent::ReceiveMessage(msg);
  switch (msg->user.u.datatag)  {
    case 'outr':
      msg->intData = outm.State();
      return MSG_PROCESSED;
    case 'midl':
      msg->intData = medm.State();
      return MSG_PROCESSED;
    case 'innr':
      msg->intData = inrm.State();
      return MSG_PROCESSED;

  }
  return CDependent::ReceiveMessage(msg);
}
//---------------------------------------------------------------
//  Probe the subsystem
//---------------------------------------------------------------
void CMarkerPanel::Probe(CFuiCanva *cnv)
{ char edt[32];
  sprintf_s(edt,32,"out.ok:   %d",outm.ok);
  cnv->AddText( 1,edt,1);
  sprintf_s(edt,32,"out.cone: %.0f",outm.cone);
  cnv->AddText( 1,edt,1);
  sprintf_s(edt,32,"out.ydev: %.0f",outm.ydev);
  cnv->AddText( 1,edt,1);
  sprintf_s(edt,32,"out.xdev: %.0f",outm.xdev);
  cnv->AddText( 1,edt,1);
  //-------------------------------------------
  sprintf_s(edt,32,"med.ok:    %d",medm.ok);
  cnv->AddText( 1,edt,1);
  sprintf_s(edt,32,"med.cone: %.0f",medm.cone);
  cnv->AddText( 1,edt,1);
  sprintf_s(edt,32,"med.ydev: %.0f",medm.ydev);
  cnv->AddText( 1,edt,1);
  sprintf_s(edt,32,"med.xdev: %.0f",medm.xdev);
  cnv->AddText( 1,edt,1);
  return;
}
//==========================================================================
// CIndicatorSet
//==========================================================================
CIndicatorSet::CIndicatorSet (void)
{ TypeIs (SUBSYSTEM_INDICATOR_SET);
  sPos  = 1;            // start position (1 based)
  ePos  = 1;            // End Position
  cPos  = 1;            // Current position
  scale = 1;            // Scaling factor
  cMsg  = 0;
}
//--------------------------------------------------------------------
//  Destructor
//--------------------------------------------------------------------
CIndicatorSet::~CIndicatorSet (void)
{	for (U_INT k=0; k < smsg.size(); k++) delete smsg[k]; 
	smsg.clear();
}
//--------------------------------------------------------------------
//  Read parameters
//--------------------------------------------------------------------
int CIndicatorSet::Read (SStream *stream, Tag tag)
{ int nbr = 0;
  switch (tag) {
  //--Indices are 0 based ----------------
  case 'zero':
    // Set indices are zero-based
    sPos = 0;
    return TAG_READ;
  //-- Default position -------------------
  case 'dflt':
  case 'indx':
    // Default switch position
    ReadInt (&nbr, stream);
    cPos  = nbr;
    return TAG_READ;
  //--Scaling factor ---------------------
  case 'scal':
    ReadFloat(&scale,stream);
    return TAG_READ;
  //---Stack of messages -----------------
  case 'smsg':
    { // Indicator source message
      SMessage* msg = new SMessage;
      msg->sender         = unId;
      msg->id             = MSG_GETDATA;
      msg->dataType       = TYPE_REAL;
      msg->user.u.datatag = 'indn';
      ReadMessage (msg, stream);
      smsg.push_back (msg);
    }
    return TAG_READ;
  }
    // See if the tag can be processed by the parent class type
  return CDependent::Read (stream, tag);
}
//--------------------------------------------------------------------
//  All parameters are read
//--------------------------------------------------------------------
void CIndicatorSet::ReadFinished (void)
{ nMsg  = smsg.size();
  ePos  = sPos + nMsg - 1;      // Last position
  char p = cPos;
  cPos   = -1;
  SetPosition(p);
  return;
}
//--------------------------------------------------------------------
//  Set a new position p
//--------------------------------------------------------------------
void CIndicatorSet::SetPosition(char p)
{ int np = ClampPos(p);
  int no = np - sPos;       // Message number
  if (cPos  == np)      return;
  cMsg   = smsg[no];
  cPos   = np;
  return;
}
//--------------------------------------------------------------------
//  Clamp position
//--------------------------------------------------------------------
char CIndicatorSet::ClampPos(char p)
{ if (p < sPos) return sPos;
  if (p > ePos) return ePos;
  return p;
}
//--------------------------------------------------------------------
//  Read Massages
//--------------------------------------------------------------------
EMessageResult  CIndicatorSet::ReceiveMessage (SMessage *msg)
{ switch (msg->id)  {
    case MSG_SETDATA:
      switch (msg->user.u.datatag) {
          case 'indx':
            SetPosition(msg->intData);
            return MSG_PROCESSED;
      }
      break;
    case MSG_GETDATA:
      switch (msg->user.u.datatag)  {
          case 'indn':
            Send_Message(cMsg);
            msg->realData = cMsg->realData;
            return MSG_PROCESSED;
      }

      break;
  }
return CDependent::ReceiveMessage(msg);
}
//--------------------------------------------------------------------
//  Probe subsystem
//--------------------------------------------------------------------
void CIndicatorSet::Probe(CFuiCanva *cnv)
{ char edt[32];
  CDependent::Probe(cnv,0);
  sprintf_s(edt,32,"sPos: %d",sPos);
  cnv->AddText( 1,edt,1);

  sprintf_s(edt,32,"cPos: %d",cPos);
  cnv->AddText( 1,edt,1);
  return;
}
//==========================================================================
// CAnnunciatorSubsystem
//==========================================================================
CAnnunciatorSubsystem::CAnnunciatorSubsystem (void)
{
  TypeIs (SUBSYSTEM_ANNUNCIATOR);
}

//
// CSuctionGaugeSubsystem
//
CSuctionGaugeSubsystem::CSuctionGaugeSubsystem (void)
{
  TypeIs (SUBSYSTEM_SUCTION_GAUGE);
}

//======================================================================================
// CFlapGauge
//  Return the flap deflection
//======================================================================================
CFlapGauge::CFlapGauge (void)
{ TypeIs (SUBSYSTEM_FLAP_GAUGE);
  hwId  = HW_GAUGE;
}
//-----------------------------------------------------------------------
//  Receive a message from Gauge
//  Reroute the message to the Flap subsystem
//  NOTE: The group is changed so the message will be directly rerouted
//        next time
//-----------------------------------------------------------------------
EMessageResult CFlapGauge::ReceiveMessage (SMessage *msg)
{ switch (msg->id) {
    case MSG_GETDATA:
      switch (msg->user.u.datatag) {
      case 'st8t':
        msg->group          = 'flap';
        msg->sender         = unId;
        msg->user.u.hw      = HW_SWITCH;
        msg->user.u.datatag = 'indn';
        msg->receiver   = 0;
        Send_Message(msg);
        return MSG_PROCESSED;
      }
  }
  return CDependent::ReceiveMessage (msg);
}
//=====================================================================================
// CSynchrophaser
//
CSynchrophaser::CSynchrophaser (void)
{
  TypeIs (SUBSYSTEM_SYNCHROPHASER);

  leftEngine = 0;
  rightEngine = 0;
}

int CSynchrophaser::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
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

//====================================================================
// CEmergencyLocator
//===================================================================
CEmergencyLocator::CEmergencyLocator (void)
{ TypeIs (SUBSYSTEM_EMERGENCY_LOCATOR);
  hwId    = HW_SWITCH;
  gLim    = 1.0;
}
//-------------------------------------------------------------------
//  Read all parameters
//-------------------------------------------------------------------
int CEmergencyLocator::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'gLim':
    // G-load activation
    ReadFloat (&gLim, stream);
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CDependent::Read (stream, tag);
  }

  return rc;
}
//------------------------------------------------------------------
//  Update state
//------------------------------------------------------------------
void CEmergencyLocator::NewState(SMessage *msg)
{ U_CHAR on     = (msg->intData == 2)?(0):(1);
  active        = (on == 1);
  msg->intData  = active;
  return;
}
//------------------------------------------------------------------
//  Receive message. 
//  -Return light state or internal state
//------------------------------------------------------------------
EMessageResult CEmergencyLocator::ReceiveMessage(SMessage *msg)
{ switch (msg->id) {
    case MSG_GETDATA:
      switch (msg->user.u.datatag) {
      case 'swit':
        msg->intData	= active;					
        return MSG_PROCESSED;
      }
    case MSG_SETDATA:
      switch (msg->user.u.datatag) {

      case 'swit':
        NewState(msg);
        return MSG_PROCESSED;
      }
  }
 return CDependent::ReceiveMessage (msg);
}
//=====================END OF FILE ======================================================
