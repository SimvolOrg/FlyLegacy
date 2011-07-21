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
#include "../Include/Robot.h"
#include "../Include/FuiParts.h"
#include "../Include/PlanDeVol.h"
using namespace std;
//=========================================================================
//  This robot is in charge of starting the aircraft
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
bool CRobot::Execute(D2R2_ACTION &a)
{ if (ROBOT_STOP != step)  return false;
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
	panl  = globals->pit->GetPanelByTag(a.pnt);
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
	"Not yet implemented ",									// Msg00
	"Flight plan is empty",									// Msg01
	"We are not at departing airport",			// Msg02
	"No take-off runway in flight plan",		// Msg03
	"No landing runway in flight plan",			// Msg04
	"Cannot start when not on ground.",			// Msg05
};
//=========================================================================
//  This robot will pilot the aircraft and execute
//	the current flight plan
//=========================================================================
VPilot::VPilot()
{ fpln = 0;
}
//--------------------------------------------------------------
//	Create error
//--------------------------------------------------------------
void VPilot::Error(int No)
{	globals->fui->DialogError(vpMSG[No],"VIRTUAL PILOT");
	return;
}
//--------------------------------------------------------------
//	Request to start the virtual pilot
//--------------------------------------------------------------
void VPilot::Start()
{	bool ok = false;
	if  (ok)							return Error(0);
	fpln	= mveh->GetFlightPlan();
	//--- Check if on ground ------------------
	ok = mveh->AllWheelsOnGround();
	if (!ok)							return Error(5);
	//--- Check for airport -------------------
  if (fpln->IsEmpty())	return Error(1);
	char *dk = fpln->GetDepartingKey();
	ok = globals->apm->AreWeAt(dk);
	if (!ok)							return Error(2);
	//--- Check for runways -------------------
	ok	= fpln->HasTakeOffRunway();
	if (!ok)							return Error(3);
	ok	= fpln->HasLandingRunway();
	if (!ok)							return Error(4);
	//----------------------------------------
}
//=======================END OF FILE ======================================================================
