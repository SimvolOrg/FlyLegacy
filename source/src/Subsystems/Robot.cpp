//==============================================================================
// Robot.cpp
//
// Part of Fly! Legacy project
//
// Copyright 2011 jean Sabatier
//
// Fly! Legacy is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
// Fly! Legacy is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
//   along with Fly! Legacy; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//=================================================================================
#include "../Include/Globals.h"
#include "../Include/Subsystems.h"
#include "../Include/FuiParts.h"
#include "../Include/PlanDeVol.h"
#include "../Include/joysticks.h"
#include "../Include/WinTaxiway.h"
using namespace std;
//==================================================================================
//	Global function to add a flightplan
//==================================================================================
int FPLfilesCB(char *fn,void *upm)
{	VPilot *vpl = (VPilot *)upm;
	return vpl->OpenFPL(fn);
}

//=========================================================================
//  Virtual pilot state
// NOTE : when text starts with '*', the throttle control must be checked
//=========================================================================
char *vpilotSTATE[] = {
	"-Idle",						// VPL_IS_IDLE		(0)
	"-Procedure",				// VPL_PROCEDURE	(1)
	"-Go TakeOff",			// VPL_GOTAKEOFF  (2)
	"-Pre-flight",			// VPL_PREFLIGHT	(3)
	"-Starting",				// VPL_STARTING		(4)
	"*Take-off",				// VPL_TAKE_OFF		(5)
	"*Climbing",				// VPL_CLIMBING		(6)
	"*Tracking",				// VPL_TRACKING		(7)
	"*Landing",					// VPL_LANDING    (8)
	"*Rolling",					// VPL_HASLAND		(9)
	"-GetParking",			// VPL_GETPARK	  (10)
	"-Taxiing",					// VPL_TAXIING    (11)
	"-Parked",					// VPL_PARKED			(12)
	"-Fixe Point",			// VPL_FIXEPOINT	(13)
	"-Emergency stop",	// VPL_EMERGENCY	(14)
	"-VPilot Terminate",// VPL_TERMINATE	(15)
	"-Waiting time",		// VPL_WAITING    (16)
};
//=========================================================================
//	PROCEDURE
//=========================================================================
Procedure::Procedure()
{
}
//---------------------------------------------------------------
//	Destroy object
//---------------------------------------------------------------
Procedure::~Procedure()
{	for (U_INT k=0; k < msgQ.size(); k++) delete msgQ[k];
	msgQ.clear();
}
//-------------------------------------------------------------------------
//  Read auto statement
//-------------------------------------------------------------------------
int Procedure::ReadAUTO(SStream *st)
{	char txt[128];
  char end[6];
	bool go = 1;
	while (go)
	{	ReadString(txt,128,st);
		sscanf(txt,"%4[^ ,;=]s",end);
		if (strncmp("endm",end,2) == 0) return TAG_READ;
		SMessage *msg = DecodeMSG(txt);
		if (msg) msgQ.push_back(msg);
		else break;
	}
	return TAG_EXIT;
}
//-------------------------------------------------------------------------
//	New message for autostart
//-------------------------------------------------------------------------
SMessage *Procedure::DecodeMSG(char *txt)
{	SMessage *msg			= IntMessage(txt);
	if (0 == msg)	msg	= FltMessage(txt);
	return msg;
}
//-------------------------------------------------------------------------
//	Decode message for autostart
//-------------------------------------------------------------------------
SMessage *Procedure::IntMessage(char *txt)
{ char ds[6];
	char fn[6];
	char hw[16];
	int nk;
	int p1;
	int nf = sscanf(txt,"int %d to %4s - %4s %n",&p1,ds,fn,&nk);
	if (3 != nf)		return 0;	
	//--- Build a message ---------------------
	SMessage *msg = new SMessage();
	msg->id				= MSG_SETDATA;
	msg->dataType = TYPE_INT;
  msg->group		= StringToTag(ds);
	msg->user.u.datatag = StringToTag(fn);
	if (msg->user.u.datatag == 'indx')	msg->index		= p1;
	else																msg->intData	= p1;
	strncpy(msg->dst,ds,5);
	//--- check for hardware id ---------------	
	txt += nk;
	if (0 == *txt)	return msg;
	nf = sscanf(txt,"( %16[^ )]s", hw);
	if (1 != nf)		return msg;
	msg->user.u.hw = GetHardwareType(hw);
	return msg;
}
//-------------------------------------------------------------------------
//	Decode message for autostart
//-------------------------------------------------------------------------
SMessage *Procedure::FltMessage(char *txt)
{ char ds[6];
	char fn[6];
	char hw[16];
	int nk;
	float p1;
	int nf = sscanf(txt,"real %f to %4s - %4s %n",&p1,ds,fn,&nk);
	if (3 != nf)		return 0;	
	//--- Build a message ---------------------
	SMessage *msg = new SMessage();
	msg->id				= MSG_SETDATA;
	msg->dataType = TYPE_REAL;
  msg->group		= StringToTag(ds);
	msg->user.u.datatag = StringToTag(fn);
	msg->realData = p1;
	strncpy(msg->dst,ds,5);
	//--- check for hardware id ---------------	
	txt += nk;
	if (0 == *txt)	return msg;
	nf = sscanf(txt,"( %16[^ )]s", hw);
	if (1 != nf)		return msg;
	msg->user.u.hw = GetHardwareType(hw);
	return msg;
}
//-------------------------------------------------------------------------
//	Execute action number
//-------------------------------------------------------------------------
bool Procedure::Execute(U_INT No, CObject *dst)
{	if (0  == msgQ.size())		return false;
	if (No >= msgQ.size())		return false;
	EMessageResult rs = MSG_IGNORED;
	//------------------------------------------------------
	SMessage *msg = msgQ[No];
	rs  = dst->ReceiveMessage(msg);
	return (MSG_PROCESSED == rs);
}
//=========================================================================
//
//  This robot is in charge of the checklist
//
//=========================================================================
CRobot::CRobot()
{ TypeIs (SUBSYSTEM_ROBOT);
  hwId    = HW_SPECIAL;
  SetIdent('D2R2');
  step    = 0;
	win 		= 0;
}
//---------------------------------------------------------------
//  Receive message
//---------------------------------------------------------------
EMessageResult CRobot::ReceiveMessage (SMessage *msg)
{ if (msg->id == MSG_SETDATA) {
    switch (msg->user.u.datatag) {
		 case 'show':
      globals->fui->CreateFuiWindow(FUI_WINDOW_CHECKLIST);
			return MSG_PROCESSED;
      }
  }
  if (msg->id == MSG_GETDATA) {
   switch (msg->user.u.datatag) {
     case 'gets':
       win  = (CFuiCkList *) msg->voidData;
       msg->voidData = this;
       step = 0;
       return MSG_PROCESSED;
   }
  }
  // See if the message can be processed by the parent class
  return CSubsystem::ReceiveMessage (msg);
}
//---------------------------------------------------------------
//   Start action
//   Each action may have the followng parameters
//    -actn:        Action code
//    -pm1 and pm2: Usually coordinates
//    -rep:         Repeat count
//    -tim:         Delay between repetition (in 0.1 sec units)
//---------------------------------------------------------------
bool CRobot::Execute(void *s)
{ D2R2_ACTION &a = *(D2R2_ACTION*)s;
	if (ROBOT_STOP != step)  return false;
	up		= a.user;
  timer = 3;
  actn  = a.actn;
  pm1   = a.pm1;
  pm2   = a.pm2;
  rep   = a.rptn;
  clk   = a.canb;
  tim   = float(a.timr) * 0.10;
	//--- Extract conditions -------------------
	val		= a.vtst;
	cond	= a.cond;
	//--- Locate panel and gauges --------------
	//--------------------------------------------------
  CCockpitManager *pit	= mveh->GetPIT();
	panl  = pit->GetPanelByTag(a.pnt);
	gage	= (panl)?(panl->GetGauge(a.ggt)):(0);
  step  = Check();
  return true;
}
//---------------------------------------------------------------
//   Execute action . Return Robot next state
//---------------------------------------------------------------
U_CHAR CRobot::Action()
{ if (0 == gage)  return ROBOT_ENDX;  
  switch(actn) {
    case D2R2_LEFT_CLICK_COORD:
      //---Single/multiple left Click on coordinates -----
      gage->MouseClick(pm1,pm2,MOUSE_BUTTON_LEFT);
      timer = tim;
      return  ROBOT_RPTN;

    case D2R2_RITE_CLICK_COORD:
      //---Single/multiple right Click on coordinates -----
      gage->MouseClick(pm1,pm2,MOUSE_BUTTON_RIGHT);
      timer = tim;
      return  ROBOT_RPTN;

    case D2R2_CLICK_UPTO_RATIO:
      //-- click until ratio is reach value ---------------
      gage->MouseClick(pm1,pm2,0);
      timer = tim;
      return  ROBOT_RPTR;

    case D2R2_LEFT_CLICK_CAFLD:
      //--- Single / multiple left Click a CA field -------
      gage->ClickField(clk,-1);
      timer = tim;
      return  ROBOT_RPTN;

    case D2R2_RITE_CLICK_CAFLD:
      //--- Single / multiple right Click a CA field -------
      gage->ClickField(clk,+1);
      timer = tim;
      return  ROBOT_RPTN;

    //--- Just show the gauge ----------
    case D2R2_SHOW_FOCUS_GAUGE:
      timer = tim;
      return  ROBOT_ENDX;
  }
  return 3;
}
//---------------------------------------------------------------
//	Check conditions
//---------------------------------------------------------------
U_CHAR CRobot::Check()
{ if (0 == cond) return ROBOT_STRT;
  if (0 == gage) return ROBOT_ENDX;
	char st = gage->State();
	switch (cond)	{
		case D2R2_IF_STATE_EQU:
			if (st == val) return ROBOT_STRT;
			return								ROBOT_ENDX;
		case D2R2_IF_STATE_NEQ:
			if (st != val) return ROBOT_STRT;
			return								ROBOT_ENDX;
		case D2R2_IF_STATE_GTR:
			if (st > val)	 return ROBOT_STRT;
			return								ROBOT_ENDX;
		case D2R2_IF_STATE_GEQ:
			if (st >= val) return ROBOT_STRT;
			return								ROBOT_ENDX;
		case D2R2_IF_STATE_LTN:
			if (st <  val) return ROBOT_STRT;
			return								ROBOT_ENDX;
		case D2R2_IF_STATE_LEQ:
			if (st <= val) return ROBOT_STRT;
			return								ROBOT_ENDX;

	}
	return ROBOT_ENDX;
}
//---------------------------------------------------------------
//   Time slice
//   Execute current action 
//---------------------------------------------------------------
void CRobot::TimeSlice(float dT,U_INT FrNo)
{ switch (step) {
    case ROBOT_STRT:
      //---Focus on gauge --------------
      timer -= dT;
      if (timer > 0)        return;
      step = ROBOT_EXEC;
      return;

    case ROBOT_EXEC:
      //--- Execute action -------------
      step  = Action();
      return;

    case ROBOT_ENDX:
      //--- End of Action --------------
      timer -= dT;
      if (timer > 0)        return;
      panl->ClearFocus();
      step   = ROBOT_STOP;
      if (win) win->EndExecute(up);
      return;

    case ROBOT_RPTN:
      //--- Repeat last order rep time --
      timer -= dT;
      if (timer > 0)        return;
      rep--;
      if (rep == 0)         step  = ROBOT_ENDX;
      else Action();
      return;

    case ROBOT_RPTR:
    //--- Repeat until at ratio ------
      timer -= dT;
      if (timer > 0)          return;
      if (gage->AtRatio(rep)) step = ROBOT_ENDX;
      else  Action();
      return;

  }
  return;
}
//=========================================================================
//	Set of error messages
//=========================================================================
char *vpMSG[] = {
	"Aircraft is now all your's ",					// Msg00
	"Flight plan is empty",									// Msg01
	"We are not at departing airport",			// Msg02
	"No take-off runway in flight plan",		// Msg03
	"No landing runway in flight plan",			// Msg04
	"Cannot start when not on ground.",			// Msg05
	"No Speed regulator in aircraft",				// Msg06
	"Cannot locate take-off runway",				// Msg07
	"No Nav Radio 1",												// Msg08
	"No runway for landing",								// Msg09
	"Throttle control lost",								// MSG10
	"Engine failure",
};
//=========================================================================
//  This robot will pilot the aircraft and execute
//	the current flight plan
//=========================================================================
VPilot::VPilot()
{ SetIdent('-VP-');
	State = VPL_IS_IDLE;
	fpln	= 0;
	RAD		= 0;
	rdmp	= 0;
	route = new NavRoute();
}
//--------------------------------------------------------------
//	Destroy resource
//--------------------------------------------------------------
VPilot::~VPilot()
{	delete route;	}
//--------------------------------------------------------------
//	Init items
//--------------------------------------------------------------
void  VPilot::PrepareMsg(CVehicleObject *veh)
{	apm		= globals->apm;
	pln		= (CAirplane*)veh;
	apil	= pln->GetAutoPilot();
	sreg	= pln->amp.GetSpeedRegulator();
	fpln	= pln->GetFlightPlan();
	return;
}
//--------------------------------------------------------------
//	Create error
//--------------------------------------------------------------
void VPilot::Error(int No)
{	globals->fui->DialogError(vpMSG[No],"VIRTUAL PILOT");
  State = VPL_IS_IDLE;
	globals->aPROF.Rep(0);
	return;
}
//--------------------------------------------------------------
//	Warn
//--------------------------------------------------------------
void VPilot::Warn(int No)
{	if (0 == alrm)	globals->fui->DialogError(vpMSG[No],"VIRTUAL PILOT");
	alrm = 1;
	return;
}
//--------------------------------------------------------------
//	Open a flight plan and add to airport list
//--------------------------------------------------------------
int VPilot::OpenFPL(char *fn)
{ char txt[PATH_MAX];
	CFPlan  fpn(globals->pln,FPL_FOR_DEMO);
	if (0 == RemoveExtension(txt,fn))	return 1;
	if (!fpn.AssignPlan(txt))					return 1;
	//--- Check if this is our airport ---------------
	char *idn = globals->apm->NearestIdent();
	if (fpn.NotThisAirport(idn))			return 1;
	//--- Keep name ----------------------------------
	fplQ.push_back(fn);
	return 1;
}
//--------------------------------------------------------------
//	Get a random flight plan
//--------------------------------------------------------------
void VPilot::GetanyFlightPlan()
{	char  pn[MAX_PATH];
	ApplyToFiles("FlightPlan/*.FPL",FPLfilesCB,this);
	//--- Load a random flight plan -------------------
	int nb = fplQ.size();
	int np = RandomNumber(nb);
	char *fn = (char*)fplQ[np].c_str();
	RemoveExtension(pn,fn);
	fpln->AssignPlan(pn);
	fplQ.clear();
	rdmp	= 1;
	return;
}
//--------------------------------------------------------------
//	External call to put aircraft on departing runway
//--------------------------------------------------------------
void VPilot::PutOnRunway(CAirport *apt,char *rwid)
{	apo	= apt->GetAPO();
	rend = apm->SetOnRunway(apt,rwid);
	return;
}
//--------------------------------------------------------------
//	External call to engage autopilot
//--------------------------------------------------------------
void VPilot::Engage()
{ apil->Engage();
	apil->EnterTakeOFF(0,rend);
	return;
}
//--------------------------------------------------------------
//	Ground braking to stop 
//--------------------------------------------------------------
void VPilot::GroundBraking()
{	pln->GroundBrakes (BRAKE_BOTH);
	State = VPL_EMERGENCY;
	double spd = mveh->GetPreCalculedKIAS(); 
	if (spd > 0.001)								return;
	State = VPL_IS_IDLE;
	return;
}
//--------------------------------------------------------------
//	Handle back the aircraft
//--------------------------------------------------------------
void VPilot::HandleBack()
{	apil->ReleaseControl();
	if (RAD)	RAD->ModeEXT(0);
	Error(0);
	fpln->StopPlan();
	return;
}
//--------------------------------------------------------------
//	Start procedure 
//--------------------------------------------------------------
void VPilot::StartProcedure(Procedure *P,char nxt, char *edm)
{	nStat = nxt;
	Pexec	= P;
	fmt		= edm;
	State = VPL_PROCEDURE;
	msgNo	= 0;
	T01	 = 2;
	return;
}
//--------------------------------------------------------------
//	Execute procedure 
//--------------------------------------------------------------
void VPilot::StepProcedure(float dT)
{	if (T01 > 0)														return;
	T01	 = 1;
	//------------------------------------------------------
	char *edt = globals->fui->PilotNote();
	_snprintf(edt,128,fmt,msgNo + 1);
	globals->fui->PilotToUser();
	//------------------------------------------------------
	if (Pexec->Execute(msgNo++,mveh))				return;
	State = nStat;
	T01		= 6;
	return;
}
//--------------------------------------------------------------
//	Locate radio 
//--------------------------------------------------------------
bool VPilot::GetRadio()
{	//----Radio message ------------------------
	RAD     = mveh->GetMRAD();
	if (0 == RAD)				return false;
	RAD->PowerON();
	return (0 != RAD->GetPowerState());
 }
//--------------------------------------------------------------
//	Toggle state
//	NOTE: When virtaul pilot is disengaged, nothing is done on 
//	aircraft state so user got immediate control in whatever
//	condition the plane is
//--------------------------------------------------------------
void VPilot::ToggleState()
{	if (State == VPL_IS_IDLE)	Start();
	else											Stop(0);
}
//--------------------------------------------------------------
//	Request to start the virtual pilot
//--------------------------------------------------------------
void VPilot::Start()
{	//-----------------------------------------
	if (fpln->IsEmpty()) GetanyFlightPlan();
	//--- Check if aircraft busy --------------
	if (globals->aPROF.Has(PROF_ACBUSY))	return;
	globals->aPROF.Set(PROF_ACBUSY);
	if (0 == sreg)				return Error(6);
	//-----------------------------------------
  if (State != VPL_IS_IDLE)	return HandleBack();
	//--- Check if on ground ------------------
	bool ok = pln->AllWheelsOnGround();
	if (!ok)							return Error(5);
	//--- Check for airport -------------------
  if (fpln->IsEmpty())	return Error(1);
	char *dk = fpln->GetDepartingKey();
	ok = apm->AreWeAt(dk);
	if (!ok)							return Error(2);
	//--- Check for runways -------------------
	ok	= fpln->HasTakeOffRunway();
	if (!ok)							return Error(3);
	ok	= fpln->HasLandingRunway();
	if (!ok)							return Error(4);
	fpln->Protect();
	//--- Navigation lights on ----------------
	pln->SendNaviMsg();
	//--- Position on runway ------------------
	apo				= apm->GetNearestAPT();
	char *rid = fpln->GetDepartingRWY();
	taxiM			= apo->GetTaxiMGR();
	taxiM->GetTkofPath(rid, route);
  if (route->IsOK())		return StartOnSpot();
	rend			= apm->SetOnRunway(0,rid);
	if (0 == rend)				return Error(7);
	//--- try to start the plane -------------
	StartProcedure(&Pstrt,VPL_FIXEPOINT,"Start step %02d");
	return;
}
//--------------------------------------------------------------
//	Start aircraft on spot
//--------------------------------------------------------------
void VPilot::StartOnSpot()
{	//--- try to start the plane -------------
	rend	= route->GetRunwayData();
	StartProcedure(&Pstrt,VPL_GOTAKEOFF,"Start step %02d");
	return;
}
//--------------------------------------------------------------
//	Taxi to take-off spot
//--------------------------------------------------------------
void VPilot::GoToTakeOff()
{	if (T01 > 0)		return;
	StartTaxiing(VPL_FIXEPOINT);
	return;
}
//--------------------------------------------------------------
//	STAND BY
//--------------------------------------------------------------
void VPilot::StandFixe()
{	pln->GroundBrakes (BRAKE_BOTH);
	if (T01 > 0)					return;
	T01		= 2;
	State = VPL_PREFLIGHT;
	return;
}
//--------------------------------------------------------------
//	Request to start the virtual pilot
//--------------------------------------------------------------
void VPilot::CheckPreFlight()
{	bool ok = false;
  if (T01 > 0)					return;
	//--- Check if all engine running ---------
	ok	= pln->AllEngineOn();
	if (!ok)							return Error(11);
	ok = GetRadio();
	if (!ok)							return Error(8);
	//--- Pre - TAKE-OFF ----------------------
	T01	= 0;
	StartTakeOff();
	return;
}
//--------------------------------------------------------------
//	Take off Action
//--------------------------------------------------------------
void VPilot::StartTakeOff()
{	State = VPL_TAKE_OFF;
	alrm	= 0;
	flap	= 1;
  apil->Engage();
	int er = apil->EnterTakeOFF(1,rend);
	if (!er)	return;
	TRACE ("Enter Take Off fail code %d",er);
	return GroundBraking();
}
//--------------------------------------------------------------
//	Take off Mode
//	 TODO: Put AGL in parameter
//--------------------------------------------------------------
void VPilot::ModeTKO()
{	if (apil->IsDisengaged())		return HandleBack();
	if (apil->BellowAGL(200))		return;
	if (!fpln->StartPlan(0))		return;
	//--- Climb to 1500 -----------
	State = VPL_CLIMBING;
	return;
}
//--------------------------------------------------------------
//	Take off Mode
//	 TODO: Put AGL in parameter
//--------------------------------------------------------------
void VPilot::ModeCLM()
{	if (apil->IsDisengaged())	  return HandleBack();
	if (apil->BellowAGL(600))		return;
	//--- Drive toward next waypoint -----------
	ChangeWaypoint();
	return;
}
//--------------------------------------------------------------
//	Enter final mode
//	Final may use 
//	-ILS radio station
//  -GPS waypoint
//--------------------------------------------------------------
void VPilot::EnterFinal()
{ //TRACE("VPL: Enter Final: %s",wayP->GetName());
	char *edt = globals->fui->PilotNote();
	//--- Configure Landing mode --------------------------
	if (!wayP->EnterLanding(RAD))	return HandleBack();
	apil->SetLandingMode();
	State = VPL_LANDING;
	rend	= apil->GetRunwayData();
	taxiM	= 0;
	//--- Advise user --------------------------------------
	_snprintf(edt,128,"Entering final for %s",wayP->GetName());
	globals->fui->PilotToUser();
	return;
}
//--------------------------------------------------------------
//	Compute direction
//	If leg distance is under 12 miles, head direct to station
//
//--------------------------------------------------------------
/*
float VPilot::SetDirection()
{	float dis = wayP->GetLegDistance();
  float seg = wayP->GetDTK();
	float dev = Radio->GetDeviation();
	float rdv = fabs(dev);
	if ((dis > 12) || (rdv < 5))	return seg;
	//--- Compute direct-to direction to waypoint -----
  return wayP->GoDirect(mveh);
}
*/
//--------------------------------------------------------------
//	Change to next Waypoint
//--------------------------------------------------------------
void VPilot::ChangeWaypoint()
{	char *edt = globals->fui->PilotNote();
	float rad = 0;
	wayP	= fpln->GetActiveNode();
	if (wayP->IsFirst())			return;  // wait next
	if (fpln->IsOnFinal())		return EnterFinal();
	//--- Advise user -------------------------------------
	float dir = wayP->GetDTK();												//SetDirection();
	_snprintf(edt,128,"Heading %03d to %s",int(dir),wayP->GetName());
	globals->fui->PilotToUser();
	//--- Set Waypoint on external source -----------------
	CmHead *obj = wayP->GetDBobject();
	RAD->ModeEXT(obj);
	//--- Set Reference direction --------------------------
	RAD->ChangeRefDirection(dir);
	//--- Configure autopilot ------------------------------
	double alt = double(wayP->GetAltitude());
	apil->SetWPTmode(alt);
	State = VPL_TRACKING;
	return;
}
//--------------------------------------------------------------
//	Tracking Waypoint
//--------------------------------------------------------------
void VPilot::RetractFlap()
{	if (apil->BellowAGL(1000))	return;
	mveh->amp.GetFlaps()->SetPosition(0);
	flap	= 0;
	return;
}
//--------------------------------------------------------------
//	Tracking Waypoint
//--------------------------------------------------------------
void VPilot::ModeTracking()
{	if (flap)		RetractFlap();
	if (apil->IsDisengaged())	return HandleBack();
	if (wayP->IsActive())	    return RAD->CorrectDrift();	//wayP->CorrectDrift(Radio);		//Refresh();
	ChangeWaypoint();
	return;
}
//--------------------------------------------------------------
//	We are now Landing
//	-Wait until wheels are on ground 
//	-Then get a path to parking lot
//--------------------------------------------------------------
void VPilot::ModeLanding()
{ if (apil->IsDisengaged())	return HandleBack();
	if (0 == rend)						return HandleBack();
	//--- Wait till landing -------------------
	if (!apil->ModeGround())	return;
	//--- Change state ------------------------
	taxiM	= rend->apo->GetTaxiMGR();
	State = VPL_HASLAND;
	return;
}
//--------------------------------------------------------------
//	Get an ouput path to parking
//	If any condition is bad, just return until autopilot
//	self disconnect itself
//--------------------------------------------------------------
//	Parking number is temporarily fixed
//--------------------------------------------------------------
void VPilot::ModeGoHome()
{	char *edt = globals->fui->PilotNote();
	if (apil->IsDisengaged())				return HandleBack();
  double spd = apil->GetSpeed();
	if (spd > 40)										return;
	//--- Below 30 mph request exit point -------------
	if (0 == taxiM)									return;
	taxiM->SetExitPath(rend,route);
	if (route->NoRoute())						return;
	State = VPL_GETPARK;
	//--- Warn user ---------------------------------------
	_snprintf(edt,128,"Taxiing to Parking");
	globals->fui->PilotToUser();
	return;
}
//--------------------------------------------------------------
//	GO TO PARKING
//	Initialize taxxing mode to target.
//	Set next mode when end of taxiing
//--------------------------------------------------------------
void VPilot::GoParking()
{	//--- Disengage autopilot ------------------
	apil->Disengage(1);
	RAD->ModeEXT(0);
	//--- Taxi to parking ----------------------
	StartTaxiing(VPL_STOPPED);
	return;
}
//--------------------------------------------------------------
//	TAXIING to a destination
//--------------------------------------------------------------
void VPilot::StartTaxiing(char nxt)
{	nStat = nxt;
	State	= VPL_TAXIING;
	//--- Initialize speed regulator ----------------------
	sreg->SetON(0);
	sreg->RouteTo(route);
	return;
}
//--------------------------------------------------------------
//	TAXIING to a last node
//--------------------------------------------------------------
void VPilot::GroundSpeed()
{	dist = sreg->TaxiSpeed();
	if (!route->LastLeg())				return;
	if (dist > 50)								return;
	sreg->SetSpeed(4);
	if (sreg->ActualSpeed() < 8)	return;
	pln->GroundBrakes (BRAKE_BOTH);
	return;
}
//--------------------------------------------------------------
//	TAXIING to a destination
//--------------------------------------------------------------
void VPilot::ModeTaxi()
{	GroundSpeed();
	if (sreg->IsSteering())	return;
	//--- End taxiing procedure ------------------
	sreg->SetOFF();
	pln->GroundBrakes (BRAKE_BOTH);
  double spd = mveh->GetPreCalculedKIAS();
	if (spd > 0.1)					return;
	State = nStat;
	T01		= 4;
	return;
}
//--------------------------------------------------------------
//	Aircraft is STOPPED
//--------------------------------------------------------------
void VPilot::ModeInPark()
{	pln->ParkBrake(1);
	StartProcedure(&Pstop,VPL_TERMINATE,"Stop step %02d");
	return;
}
//--------------------------------------------------------------
//	Waiting for next round
//--------------------------------------------------------------
void VPilot::ModeWait()
{	if (T01 > 0)		return;
	State = VPL_IS_IDLE;
	Start();
}
//--------------------------------------------------------------
//	Terminate procedure
//	Check for Demo mode to restart
//--------------------------------------------------------------
void VPilot::Terminate()
{	char pm[8];
  char dm;
	GetIniString("Sim","Mode",pm,8);
	if (strncmp(pm,"Demo",4) == 0) dm = 1;
	Stop(dm);
	if (0 == dm)  return;
	//--- Restart a new round in 1 minute ---------------
	T01	= 60;
	State = VPL_WAITING;
	return;
}	
//--------------------------------------------------------------
//	STOP virtual pilot
//	d = 1  if demo mode
//--------------------------------------------------------------
void VPilot::Stop(char d)
{ fpln->StopPlan();
	if (rdmp)	fpln->AssignPlan("");
	if (sreg)	sreg->SetOFF();
	State = VPL_IS_IDLE;
	globals->aPROF.Raz(PROF_ACBUSY);
	pln->ParkBrake(1);
	if (0 == d) Error(0);
	return;
}
//--------------------------------------------------------------
//	Time slice
//--------------------------------------------------------------
void VPilot::TimeSlice (float dT,U_INT frm)
{	FrNo		 = frm;
	if (State == VPL_IS_IDLE)	return;
	T01			-= dT;
	bool  on = (*vpilotSTATE[State] == '*');
	bool  ok = (on && apil->HasGasControl());
	if (ok == on)		alrm = 0;
	else						Warn(10);
	switch (State)	{
		//--- Starting aircraft -------------
		case VPL_PROCEDURE:
			StepProcedure(dT);
			return;
		case VPL_PREFLIGHT:
			CheckPreFlight();
			return;
		case VPL_GOTAKEOFF:
			GoToTakeOff();
			return;
		//--- TAKE-OF State ---------------
		case VPL_TAKE_OFF:
			ModeTKO();
			return;
		//--- Climbing --------------------
		case VPL_CLIMBING:
			ModeCLM();
			return;
		//--- Tracking mode ----------------
		case VPL_TRACKING:
			ModeTracking();
			return;
		//--- Landing ----------------------
		case VPL_LANDING:
			ModeLanding();
			return;
		//--- TouchDown -------------------
		case VPL_HASLAND:
			ModeGoHome();
			return;
		//--- Go to parking ----------------
		case VPL_GETPARK:
			GoParking();
			return;
		//--- Just parked ------------------
		case VPL_STOPPED:
			ModeInPark();
			return;
		//--- Taxiing to destination -------
		case VPL_TAXIING:
			ModeTaxi();
			return;
		//--- Stand fixe ------------------
		case VPL_FIXEPOINT:
			StandFixe();
			return;
		//--- Must stop on ground----------
		case VPL_EMERGENCY:
			GroundBraking();
			return;
		//--- Wait for next round ----------
		case VPL_WAITING:
			ModeWait();
			return;
		//--- Will exit from virtual pilot -
		case VPL_TERMINATE:
			Terminate();
			return;
	}
	return;
}
//--------------------------------------------------------------
//	Probe
//--------------------------------------------------------------
void VPilot::Probe(CFuiCanva *cnv)
{ cnv->AddText(1,1,"Fr %08d",FrNo);
  cnv->AddText(1,vpilotSTATE[State],1);
	cnv->AddText(1,1,"Distance %.6lf",dist);
}
//=======================END OF FILE ======================================================================
