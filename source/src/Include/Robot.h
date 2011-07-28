//==============================================================================
// Robot.h
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
#ifndef ROBOT_H
#define ROBOT_H
//======================================================================
#include "../Include/FlyLegacy.h"
class	CPanel;
class CGauge;
class CAirplane;
//=========================================================================
//  ROBOT RETURN CODE
//=========================================================================
#define ROBOT_STOP  (0)
#define ROBOT_STRT  (1)
#define ROBOT_EXEC  (2)
#define ROBOT_ENDX  (3)
#define ROBOT_RPTN  (4)
#define ROBOT_RPTR  (5)
#define ROBOT_TEST  (6)
//=========================================================================
//  ROBOT preconditon CODE
//=========================================================================
#define D2R2_NO_CONDITION		(0)
#define D2R2_IF_STATE_EQU		(1)
#define D2R2_IF_STATE_NEQ		(2)
#define D2R2_IF_STATE_GTR		(3)
#define D2R2_IF_STATE_GEQ		(4)
#define D2R2_IF_STATE_LTN		(5)
#define D2R2_IF_STATE_LEQ		(6)
//=========================================================================
//  ROBOT ACTION CODE
//=========================================================================
#define D2R2_NOPERATION				(0)				// Do nothing
#define D2R2_LEFT_CLICK_COORD	(1)				// Click left on coordinates
#define D2R2_RITE_CLICK_COORD	(2)				// Click right on coordinates
#define D2R2_CLICK_UPTO_RATIO (3)				// Click until ratio is reached
#define D2R2_LEFT_CLICK_CAFLD	(4)				// Left click on CA field
#define D2R2_RITE_CLICK_CAFLD (5)				// Right click a CA field
#define D2R2_SHOW_FOCUS_GAUGE (6)				// Just show a gauge
#define D2R2_SHOW_FOCUS_PANEL (7)				// Just show panel
//=========================================================================
//  ROBOT VALIDATION CODE
//=========================================================================
#define D2R2_NEED_PANL				(0x0001)
#define D2R2_NEED_GAGE				(0x0002)
#define D2R2_NEED_CPIT				(0x0004)
#define D2R2_END_START				(0x8000)
//-- For panel check list ---------------
#define D2R2_CHECK_LST	(D2R2_NEED_PANL+D2R2_NEED_GAGE+D2R2_NEED_CPIT)
//=========================================================================
//  Structure fro Robot Action
//=========================================================================
struct D2R2_ACTION {
  Tag				actn;													// Action
	Tag				pnt;													// Panel Tag
	Tag				ggt;													// Gauge Tag
	CPanel   *panl;													// Tag panel
	CGauge   *gage;													// Gauge
	//---------------------------------------------------------
	int				vtst;													// Testing value
	char      cond;													// Pre conditions
	//---------------------------------------------------------
	char			rptn;													// Repeat number
  char			timr;													// Timer
  char			canb;													// CA number
  short			pm1;													// Parameter 1
  short			pm2;													// Parameter 2
	void	   *user;													// User for callback
	//----------------------------------------------------------
	void D2R2_ACTION::SetAction(Tag a, int p1, int p2)
	{	actn  = a;
		pm1   = p1;
		pm2   = p2;
	}
	//----------------------------------------------------------
	void D2R2_ACTION::Clear()
	{ panl	= 0;
		gage	= 0;
		actn  = D2R2_NOPERATION;
		cond	= D2R2_NO_CONDITION;
		canb  = 0;
		timr  = 10;
		rptn	= 1;
		pm1		= 0;
		pm2		= 0;
		user  = 0;
	}
	//---------------------------------------------------------
	void	D2R2_ACTION::SetParam(void *p)		{user = p;}
};
//=================================================================
//  Robot to execute the check list
//  
//=================================================================
class CRobot : public CSubsystem {
  //----ATTRIBUTES: ----------------------------------
  char  step;                   // Action step
  Tag   actn;                   // Current action
  int   pm1;                    // Parameter p1
  int   pm2;                    // Parameter p2
  int   rep;                    // Repeat count
  int   clk;                    // Click area
  float tim;                    // Timer value
	void	*up;										// User parameter
	//--------------------------------------------------
	int		val;										// Condition value
	char	cond;										// Condition operator
  //--------------------------------------------------
  CFuiCkList *win;              // Check list
  //--------------------------------------------------
  CPanel  *panl;                // action panel
  CGauge  *gage;                // Associated gauge
  float timer;                  // Internal timer
	//---Camera ----------------------------------------
	CCamera *cam;									// Curren tcamera
  //----Methods --------------------------------------
public:
  CRobot();
  //--------------------------------------------------
  bool  Execute(D2R2_ACTION &a);
  void  TimeSlice(float dT, U_INT FrNo);
  U_CHAR	Action();
	U_CHAR	Check();
  //--------------------------------------------------
  EMessageResult ReceiveMessage (SMessage *msg);
  //---------------------------------------------------
  inline void Register(CFuiCkList *w)      {win = w;}
	//---------------------------------------------------
	inline bool	Inactive()		{return (ROBOT_STOP == step);}
};
//=================================================================
//	Virtual pilot states
//=================================================================
#define VPL_IS_IDLE			(0)
#define VPL_STARTING		(1)
#define VPL_TAKE_OFF		(2)
#define VPL_CLIMBING		(3)
#define VPL_TRACKING		(4)
#define VPL_LANDING     (5)
//=================================================================
//  Virtual Pilot to pilot the aircraft
//  
//=================================================================
class VPilot: public CSubsystem {
protected:
	//--- ATTRIBUTES --------------------------------------
	char							 State;
	char							 cnt;
	//-----------------------------------------------------
	U_INT							 FrNo;
	float							 T01;
	CWPoint					  *wayP;			// Target WayPoint
	//-----------------------------------------------------
	CAirplane         *pln;				// Airplane
	CFPlan						*fpln;			// Flight plan to execute
	AutoPilot         *apil;			// Auto pilote
	CRadio					  *Radio;			// Radio n°1
	BUS_RADIO         *busR;			// Radio bus
	//--- Radio messages ----------------------------------
	SMessage           mrad;			// Radio message
	//-----------------------------------------------------
public:
	VPilot();
	bool	GetRadio();
	//-----------------------------------------------------
	void	Error(int No);
	//--- Action routines ---------------------------------
	void	PreStart(float dT);
	void	EnterTakeOff();
	void	EnterFinal();
	void	EnterWaypoint();
	void	ChangeWaypoint();
	//--- State routines ----------------------------------
	void	Start();
	void	HandleBack();
	//--- Mode routines -----------------------------------
	void	ModeTKO();
	void	ModeCLM();
	void	ModeTracking();
	void	ModeLanding();
	//------------------------------------------------------
	void TimeSlice (float dT,U_INT FrNo);
};
//=======================END OF FILE ======================================================================
#endif ROBOT_H
