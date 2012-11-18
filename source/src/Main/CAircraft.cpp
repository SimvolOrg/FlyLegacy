/*
 * CAircraft.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
 * Copyright 2007 Jean Sabatier
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

/*! \file WorldObjects.cpp
 *  \brief Implements CWorldObject and derived classes
 *
 *  All objects in the simulation world that have 3D renderable geometry
 *    or any kind of real-time behaviour are implemented as some kind
 *    of "world object".  At the top level of the class hierarchy are
 *    the abstract classes CWorldObjectBase and CWorldObject.  Concrete
 *    classes follow a strict hierarchy, with additional features being
 *    implemented at each successive level.  The CModelObject class
 *    adds renderable 3D geometry and the concepts of location and
 *    orientation to the CWorldObject.  The CSimulatedObject class
 *    adds real-time behaviour of some sort, with the addition of a
 *    method that is called on each simulation timeslice.  The
 *    CVehicleObject is the parent for all types of vehicles, including
 *    CAirplane for fixed-wing aircraft, CHelicopter for rotary-wing
 *    aircraft, and CGroundVehicle for ground vehicles.
 */

/*
 *  OPAL-ODE library http://sourceforge.net/projects/opal (opal-sdk-0.4.0-win32-vc8).
 *  Code is wrapped between #ifdef sections for those of you who prefer to avoid those libraries,
 *  so if you want to test them you must define HAVE_OPAL in the C/C++ preprocessor settings,
 *  add the library (opal-ode_d.lib) in the input linker (and include + lib filepaths in the 
 *  Projects and Solutions VC++ directories), add the opal-ode_d.dll library in the
 *  main FlyLegacy root and finally add those lines in the INI file :
 *
 *   
 *   
 *   [PHYSICS]
 * aircraftPhysics=aero-opal
 * gearPhysics=opal
 *
 * aircraftPhysics can be "aero-opal" "ufo" "normal" "total-opal" (default is 'ufo')
 * initialSpeedz is forward
 * gearPhysics can be "jsbsim" "mix-jsbsim-lasrs" "opal" or can be removed (default is 'jsbsim')
 * 
 * NB : default (no [PHYSICS] section) is "ufo" object
 * NB : if you set ufo then use gearPhysics=jsbsim
 */

#include "../Include/Globals.h"
#include "../Include/Utility.h"
#include "../Include/Fui.h"
#include "../Include/FuiParts.h"
#include "../Include/FuiProbe.h"
#include "../Include/FuiUser.h"
#include "../Include/FuiPlane.h"
#include "../Include/Joysticks.h"     //  for CUFOObject 
#include "../Include/MagneticModel.h" // iang correction from .SIT file ln 161
#include "../Include/Atmosphere.h"    
#include "../Include/Weather.h" 
#include "../Include/3dMath.h"
#include "../Include/Collisions.h"
#include "../Include/PlanDeVol.h"
#include "../Plugin/Plugin.h"
#include <vector>								      // JSDEV* for STL

using namespace std;
//---------------------------------------------------------------------------------------
extern	CNullSubsystem nsub;
extern	opal::Solid *CreateGround();
//==========================================================================================
extern DAMAGE_MSG damMSG[];
//==========================================================================================
///////// DEBUG STUFF TO REMOVE LATER ////////////////////////////////
#ifdef  _DEBUG
  //#define _NO_DEBUG_KINETICS    // allows 3D position and orientation ... remove later 
  //#define _DEBUG_VEH            // print lc DDEBUG file               ... remove later
  //#define _DEBUG_UFO            // print lc DDEBUG file               ... remove later
  //#define _DEBUG_OPAL           // print lc DDEBUG file               ... remove later
  //#define _DEBUG_LINE_PLOT_TEST // print lc DDEBUG file               ... remove later
  //#define _DEBUG_suspension     // print lc DDEBUG file               ... remove later
  //#define _DEBUG_trigger        // print lc DDEBUG file               ... remove later
  //#define _DEBUG_forces         // print lc DDEBUG file               ... remove later
  //#define _DEBUG_gear           // print lc DDEBUG file               ... remove later
  //#define _DEBUG_AMP_SYS        // print lc DDEBUG file               ... remove later
  #ifdef  _DEBUG_LINE_PLOT_TEST
   static float timer_plot = 0.0f;
  #endif
#endif
//================================================================================
// Collision handler
//================================================================================
 CrashDetector *CrashHandler = new CrashDetector();
 std::string planeID("plane");                    // Must be global for opal
 std::string groundID("ground");
//================================================================================
//  The following global functions are the keyboard interface for a
//  Vehicle of type aircraft
//================================================================================
//------------------------------------------------------------------------------
// Airplane Group Keys
//------------------------------------------------------------------------------
bool KeyAirGroup (CKeyDefinition *kdf, int code)
{ Tag kid = kdf->GetTag();
  bool ac = (globals->pln != 0);
	if (0 == ac)				return false;
  if (kid == 'acta')	return globals->pln->CenterControls();
  // Inhibit aircraft control when in slew mode
  if (globals->slw->IsEnabled ()) return false;
  if (kdf->NoPCB())               return false;
  U_INT key = code & 0x0000FFFF;      // Key code
  U_INT mod = (code >> 16);           // Modifier
  return kdf->GetCallback() (kid,key,mod);
}
//===============================================================================
//  Global aircraft keyboard: Elevator down
//===============================================================================
bool aKeyADEL(int kid,int key, int mod)
{ globals->pln->ElevatorIncr ();
  return true;
}
//---Elevator up --------------------------------------
bool aKeyAUEL(int kid,int key, int mod)
{ globals->pln->ElevatorDecr ();
  return true;
}
//---Aileron left -----------------------------------
bool aKeyALAI(int kid,int key, int mod)
{  globals->pln->AileronDecr ();
   return true;
}
//---Aileron right ----------------------------------
bool aKeyARAI(int kid,int key, int mod)
{ globals->pln->AileronIncr ();
  return true;
}
//--- Rudder Left ----------------------------------
bool aKeyALRD(int kid,int key, int mod)
{ globals->pln->RudderIncr ();
  return true;
}
//--- Rudder right ---------------------------------
bool aKeyARRD(int kid,int key, int mod)
{ globals->pln->RudderDecr ();
  return true;
}
//--- Trim elevator down ---------------------------
bool aKeyADTR(int kid,int key, int mod)
{ globals->pln->ElevatorTrimDecr ();
  return true;
}
//--- Trim elevator up -----------------------------
bool aKeyAUTR(int kid,int key, int mod)
{ globals->pln->ElevatorTrimIncr ();
  return true;
}
//--- Trim aileron left ----------------------------
bool aKeyATAL(int kid,int key, int mod)
{ globals->pln->AileronTrimDecr ();
  return true;
}
//--- Trim aileron right --------------------------
bool aKeyATAR(int kid,int key, int mod)
{ globals->pln->AileronTrimIncr ();
  return true;
}
//--- Trim Rudder Left ---------------------------
bool aKeyATRL(int kid,int key, int mod)
{ globals->pln->RudderTrimIncr ();
  return true;
}
//--- Trim rudder right --------------------------
bool aKeyATRR(int kid,int key, int mod)
{ globals->pln->RudderTrimDecr ();
  return true;
}
//--- Modify Rudder Bias to the left -------------
bool aKeyLRDB(int kid,int key,int mod)
{	globals->pln->RudderBias(+0.005f);
	return true;
}
//--- Modify Rudder Bias to the right -------------
bool aKeyRRDB(int kid,int key,int mod)
{	globals->pln->RudderBias(-0.005f);
	return true;
}

//--- Flap extend ---------------------------------
bool aKeyAFEX(int kid,int key, int mod)
{ globals->pln->FlapsExtend ();
  return true;
}
//--- Flap retract -------------------------------
bool aKeyAFRT(int kid,int key, int mod)
{ globals->pln->FlapsRetract ();
  return true;
}
//--- Wheel brakes -------------------------------
bool aKeyABRK(int kid,int key, int mod)
{ globals->pln->GroundBrakes (BRAKE_BOTH);
//TRACE("WHEEL BRAKE");
  return true;
}
//--- Left Wheel brake ---------------------------
bool aKeyLBRK(int kid,int key, int mod)
{ globals->pln->GroundBrakes (BRAKE_LEFT);
  return true;
}
//--- Right wheel brake --------------------------
bool aKeyRBRK(int kid,int key, int mod)
{ globals->pln->GroundBrakes (BRAKE_RITE);
  return true;
}
//--- Parking brake ------------------------------
bool aKeyPBRK(int kid,int key, int mod)
{ globals->pln->ParkBrake(0);
  return true;
}
//--- Gear toggle --------------------------------
bool aKeyAGTG(int kid,int key, int mod)
{ globals->pln->GearUpDown ();
  return true;
}
//--- Center controls ----------------------------
bool aKeyACTR(int kid,int key, int mod)
{ globals->pln->CenterControls();
  return true;
}
//--- Toggle NAVI switch -------------------------
bool aKeyANLT(int kid,int key, int mod)
{ globals->pln->SendNaviMsg();
  return true;
}
//--- Toggle autopilot --------------------------
bool aKeyAPEN(int kid,int key, int mod)
{ globals->pln->SendApilMsg();
  return true;	}
//--- Toggle virtual pilot ---------------------
bool aKeyVPIL(int kid,int key, int mod)
{ globals->pln->ToggleVPIL();
	return true;	}
//--- prop picth increase ----------------------
bool aKeyAPPI(int kid,int key, int mod)
{ globals->jsm->SendGroup(JOY_GROUP_PROP,'incr');
  return true;
}
//---Prop pitch decrease ----------------------
bool aKeyAPPD(int kid,int key, int mod)
{ globals->jsm->SendGroup(JOY_GROUP_PROP,'decr');
  return true;
}
//---Mixture increase ---------------------------------------
bool aKeyAIMX(int kid,int key, int mod)
{ globals->jsm->SendGroup(JOY_GROUP_MIXT,'incr');
  return true; }
//---Mixture decrease ---------------------------------------
bool aKeyADMX(int kid,int key, int mod)
{ globals->jsm->SendGroup(JOY_GROUP_MIXT,'decr');
  return true; }
//---Mixture full rich --------------------------------------
bool aKeyAMFR(int kid,int key, int mod)
{	globals->jsm->SendGroup(JOY_GROUP_MIXT,'amfr');
  return true; }
//---Mixture full lean --------------------------------------
bool aKeyAMFL(int kid,int key, int mod)
{	globals->jsm->SendGroup(JOY_GROUP_MIXT,'amfl');
  return true; }
//---Throttle up --------------------------------------------
bool aKeyTRUP(int kid,int key, int mod)
{	globals->jsm->SendGroup(JOY_GROUP_THRO,'incr');
  return true; }
//---Throttle Down --------------------------------------------
bool aKeyTRDN(int kid,int key, int mod)
{	globals->jsm->SendGroup(JOY_GROUP_THRO,'decr');
  return true; }

//---Autopilot Takeoff---------------------------------------
bool aKeyTKOF(int kid,int key, int mod)
{	globals->pln->VirtualPilot()->Engage();
	return true;
}
//---Open GPS window (TODO: must check the correct GPS) -----
bool aKeyGWIN(int kid, int key, int mod)
{ globals->fui->ToggleFuiWindow(FUI_WINDOW_KLN89);
  return true; }
//---Get aircraft options ----------------------------------
bool aKeyADET(int kid, int key, int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_VEHICLE_OPTIONS);
  return true; }
//---Check list --------------------------------------------
bool aKeyCKLW(int kid, int key, int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_CHECKLIST);
  return true; }
//---Fuel load ---------------------------------------------
bool aKeySFUL(int kid,int code, int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_VEHICLE_FUEL);
  return true; }
//---Weight and baggages -----------------------------------
bool aKeySBAG(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_VEHICLE_LOAD);
  return true; }
//--- Center of gravity ------------------------------------
bool aKeyCGIN(int kid,int code,int mod)
{ globals->fui->ToggleFuiWindow (FUI_WINDOW_VEHICLE_CG);
  return true; }
//--- Damage report ----------------------------------------
bool aKeyOUCH(int kid,int code,int mod)
{ return true; }
//--- Damage reset ----------------------------------------
bool aKeyRSET(int kid,int code,int mod)
{ CAirplane *pln = globals->pln;
	globals->fui->CloseAllWindows();
	if (pln)	pln->ResetCrash(1);
  return true; }
//--- Aero vector  ----------------------------------------
bool aKeyAERV(int kid,int code,int mod)
{ globals->pln->ToggleOPT(VEH_DW_AERO);
  return true; }
//---Aero position ----------------------------------------
bool aKeyAERP(int kid,int code,int mod)
{ globals->pln->ToggleOPT(VEH_DW_VPOS);
  return true; }
//---Draw smoke -------------------------------------------
bool aKeySMOK(int kid,int code,int mod)
{ globals->pln->ToggleOPT(VEH_DW_SMOK);
  return true; }
//---Draw shadow ------------------------------------------
bool aKeySHAD(int kid,int code,int mod)
{ globals->pln->ToggleOPT(VEH_DW_SHAD);
  return true;  }
//---Tune PID (if autopilot exist -------------------------
bool aKeyTPID(int kid,int code,int mod)
{   globals->fui->ToggleFuiWindow(FUI_WINDOW_PIDTUNE);
  return true; }

//================================================================================
// CAirplane
//================================================================================
//
// [PHYSICS]
// aircraftPhysics=normal
//  JSDEV*:  Implement keyboard message (NAVI etc)
//================================================================================
CAirplane::CAirplane (void)
{ SetOPT(VEH_IS_FLY);											// Flying object
	damM.Severity	= 0;
  damM.msg			=   0;
  sound					=   0;
  //-------------------------------------------------------------------
  ifpos.x =   0.00; 
  ifpos.y =   0.00; //-9.81; //
  ifpos.z =   0.00; 
  //---Init the NAVI messages --------------------------------------
  Navi.sender     = 'plan';
  Navi.id         = MSG_SETDATA;
  Navi.dataType   = TYPE_INT;
  Navi.group      = 'NAVI';               // Default will be overwriten
  Navi.user.u.datatag = 'swap';
  //---Init the TAXI messages --------------------------------------
  Taxi.sender     = 'plan';
  Taxi.id         = MSG_SETDATA;
  Taxi.dataType   = TYPE_INT;
  Taxi.group      = 'TAXI';               // Default will be overwriten
  Taxi.user.u.datatag = 'swap';
  //---Init the LAND messages --------------------------------------
  Land.sender     = 'plan';
  Land.id         = MSG_SETDATA;
  Land.dataType   = TYPE_INT;
  Land.group      = 'TAXI';               // Default will be overwriten
  Land.user.u.datatag = 'swap';
  //---Init the Strobe messages --------------------------------------
  Strb.sender     = 'plan';
  Strb.id         = MSG_SETDATA;
  Strb.dataType   = TYPE_INT;
  Strb.group      = 'STRB';               // Default will be overwriten
  Strb.user.u.datatag = 'swap';
  //---Init Autopilot message --------------------------------------
  Apil.sender     = 'plan';
  Apil.id         = MSG_SETDATA;
  Apil.group      = 'AXIS';
  Apil.user.u.datatag = 'apOn';
}
//-----------------------------------------------------------------------------
//	JSDEV* Delete this object
//-----------------------------------------------------------------------------
CAirplane::~CAirplane (void)
{ if (IsUserPlan())	globals->pln = 0;
  globals->kbd->UnbindGroup('plne');
}
//-----------------------------------------------------------------------------
//	JSDEV* All parameters are read
//-----------------------------------------------------------------------------
void CAirplane::ReadFinished (void)
{
  CVehicleObject::ReadFinished ();
	SetName(nfoFilename);
  //--- Init rudder parameters ---------------------------------
  RudderBankMap(svh.GetAcrd ());
  //--- Store main wing incidence ------------------------------
  main_wing_incid   = wng.GetWingSection ("wing Left w/Aileron")->GetWingIncidenceDeg ();
  main_wing_aoa_min = wng.GetAirfoil     ("Wing Airfoil")->GetAoAMin ();
  main_wing_aoa_max = wng.GetAirfoil     ("Wing Airfoil")->GetAoAMax ();
  //--- Call final initialization ------------------
  BindKeys();                             // Map all keys
  //---- Set Initial state -------------------------
  State = VEH_INIT;
  //--- Interconnect trim controls -----------------
	amp.aTrim->SetMainControl(amp.GetAilerons());
	amp.rTrim->SetMainControl(amp.GetRudders());
	//--- Load optionnal flight plan -----------------
	LoadFPLAN(fplname);
  return;
}
//-----------------------------------------------------------------------------
// Load flight plan if needed
//-----------------------------------------------------------------------------
void CAirplane::LoadFPLAN(char *fn)
{	if (0 == *fn)		return;
	CFPlan *fpln = GetFlightPlan();

	fpln->AssignPlan(fn);
	return;
}
//-----------------------------------------------------------------------------
// Go to next waypoint
//-----------------------------------------------------------------------------
void CAirplane::NextWaypoint()
{	CFPlan *fpn = GetFlightPlan();
	if (fpn->Inactive())		return;
	if (fpn->IsOnFinal())		return;

	CWPoint   *wpt	= fpn->GetActiveNode();
	SPosition *pos	= wpt->GetGeoP();
	double     dir  = wpt->GetDTK();
	SVector    ori  = globals->iang;
	ori.z					  = DegToRad(dir);
	//SetPhysicalOrientation (ori);
	//SetObjectOrientation(ori); 
	SetObjectPosition(*pos);
}

//-----------------------------------------------------------------------------
//  Register all planes keys
//-----------------------------------------------------------------------------
void CAirplane::BindKeys()
{ CKeyMap *km = globals->kbd;
  km->BindGroup('plne',KeyAirGroup);
  //---Control surfaces ----------------------------------------------------
  km->Bind('adel',aKeyADEL,KEY_REPEAT);
  km->Bind('auel',aKeyAUEL,KEY_REPEAT);
  km->Bind('alai',aKeyALAI,KEY_REPEAT);
  km->Bind('arai',aKeyARAI,KEY_REPEAT);
  km->Bind('alrd',aKeyALRD,KEY_REPEAT);           // Rudder Left
  km->Bind('arrd',aKeyARRD,KEY_REPEAT);           // Rudder Right
  km->Bind('actr',aKeyACTR,KEY_SET_ON);           //--- Center all controls -------
  km->Bind('autr',aKeyAUTR,KEY_REPEAT);           // Trim Elevator Up
  km->Bind('adtr',aKeyADTR,KEY_REPEAT);           // Trim Elevator Down
  km->Bind('atal',aKeyATAL,KEY_REPEAT);           // Trim Aileron Left
  km->Bind('atar',aKeyATAR,KEY_REPEAT);           // Trim Aileron Right
  km->Bind('atrl',aKeyATRL,KEY_REPEAT);           // Trim Rudder Left
  km->Bind('atrr',aKeyATRR,KEY_REPEAT);           // Trim Rudder Right
  km->Bind('afex',aKeyAFEX,KEY_SET_ON);           // Flaps (Extend)
  km->Bind('afrt',aKeyAFRT,KEY_SET_ON);           // Flaps (Retract)
	km->Bind('lrdb',aKeyLRDB,KEY_REPEAT);						// Left rudder bias
	km->Bind('rrdb',aKeyRRDB,KEY_REPEAT);						// Right rudder bias
  //---Brakes ---------------------------------------------------------
  km->Bind('abrk',aKeyABRK,KEY_REPEAT);           // Brake both wheels 
  km->Bind('lbrk',aKeyLBRK,KEY_REPEAT);           // Brake left wheel
  km->Bind('rbrk',aKeyRBRK,KEY_REPEAT);           // Brake left wheel
  km->Bind('pbrk',aKeyPBRK,KEY_SET_ON);           // Parking brake
  //---various --------------------------------------------------------
  km->Bind('agtg',aKeyAGTG,KEY_SET_ON);           // Gear Up/Down);
  km->Bind('anlt',aKeyANLT,KEY_SET_ON);           // Lights NAVI (Navigation)
  km->Bind('apen',aKeyAPEN,KEY_SET_ON);           // Autopilot engage/disengage
	km->Bind('vpil',aKeyVPIL,KEY_SET_ON);						// Virtual pilot toggle
  //--Engine control ---------------------------------------------------
  km->Bind('appi',aKeyAPPI,KEY_REPEAT);           // Prop picth increase
  km->Bind('appd',aKeyAPPD,KEY_REPEAT);           // Prop picth decrease
  km->Bind('aimx',aKeyAIMX,KEY_REPEAT);           // Mixture increase
  km->Bind('admx',aKeyADMX,KEY_REPEAT);           // Mixture decrease
	km->Bind('amfr',aKeyAMFR,KEY_SET_ON);						// Mixture full rich
	km->Bind('amfl',aKeyAMFL,KEY_SET_ON);						// Mixture full lean
	km->Bind('tkof',aKeyTKOF,KEY_SET_ON);						// Autopilot Take-off
	km->Bind('trdn',aKeyTRDN,KEY_REPEAT);						// throttle down
	km->Bind('trup',aKeyTRUP,KEY_REPEAT);						// Throttle up
  //---Menu keys -------------------------------------------------------
  km->Bind('gwin',aKeyGWIN,KEY_SET_ON);           // Display GPS
  km->Bind('adet',aKeyADET,KEY_SET_ON);           // Display aircraft info
  km->Bind('cklw',aKeyCKLW,KEY_SET_ON);           // Check list
  km->Bind('rset',aKeyRSET,KEY_SET_ON);           // Reset damage
  km->Bind('sful',aKeySFUL,KEY_SET_ON);           // Load fuel
  km->Bind('sbag',aKeySBAG,KEY_SET_ON);           // weight and baggages
  km->Bind('cgin',aKeyCGIN,KEY_SET_ON);           // weight and baggages
  km->Bind('ouch',aKeyOUCH,KEY_SET_ON);           // damage report
  km->Bind('aerv',aKeyAERV,KEY_SET_ON);           // Draw Aero vector
  km->Bind('aerp',aKeyAERP,KEY_SET_ON);           // Draw Aero position
  km->Bind('smok',aKeySMOK,KEY_SET_ON);           // Draw smoke
  km->Bind('shad',aKeySHAD,KEY_SET_ON);           // Draw shadow
  //---------------------------------------------------------------------
  km->Bind('tpid',aKeyTPID,KEY_SET_ON);           // Tune PID

  return;
}


//-----------------------------------------------------------------------------
//	JSDEV* Prepare message for all subsystems
//-----------------------------------------------------------------------------
void CAirplane::PrepareMsg ()
{	CSubsystem *sub = NULL;
	std::vector<CSubsystem*>::iterator it;
	for (it = amp.subs.begin(); it != amp.subs.end(); it++)
	{	sub = *it;
		sub->PrepareMsg(this);
	}
	std::vector<CFuelSubsystem*>::iterator ig;
	for (ig = gas.fsub.begin(); ig != gas.fsub.end(); ig++)
	{	sub	= *ig;
		sub->PrepareMsg(this);
	}
	CVehicleObject::PrepareMsg();		// Continue
	return;	}
//-----------------------------------------------------------------------------
//	JSDEV* Find message receiver in all plane subsystems
//-----------------------------------------------------------------------------
bool CAirplane::FindReceiver (SMessage *msg)
{ char cid[8];
  char cgr[8];
  char ctg[8];
  bool fnd = false;
	if (0 == msg)	return false;
	if (!fnd) fnd = FindReceiver(msg,&amp);
	if (!fnd) fnd = FindReceiver(msg,&gas);
	if (!fnd) fnd = FindReceiver(msg,&eng);
	if (globals->Trace.Has(TRACE_MSG_PREPA))	TraceMsgPrepa(msg);
	
	// Display warning message if message receiver not found
  if (fnd)	return true;
	msg->receiver = &nsub;
  TagToString (cid, msg->sender);
  TagToString (cgr, msg->group);
  TagToString (ctg, msg->user.u.datatag);
  WARNINGLOG ("FindReceiver:(NO RECEIVER) sender=%s  dest=%s  dtag=%s",cid,cgr, ctg);
	msg->group    = 'Null';
  return false;
}

//-----------------------------------------------------------------------------
//	JSDEV* Find message receiver in electrical subsystems
//-----------------------------------------------------------------------------
bool CAirplane::FindReceiver (SMessage *msg,CElectricalSystem *esys)
{	if (esys == NULL)	return false;
	std::vector<CSubsystem*>::iterator it;
	for (it = esys->subs.begin(); it != esys->subs.end(); it++)
	{	CSubsystem *sys = *it;
    if (!sys->IsReceiver(msg))	continue;
    TagToString(msg->dst,msg->group);
    return true;
	}
	return false;
}
//-----------------------------------------------------------------------------
//	JSDEV* Find message receiver in gas subsystems
//-----------------------------------------------------------------------------
bool CAirplane::FindReceiver (SMessage *msg,CFuelSystem *gsys)
{	if (gsys == NULL)	return false;
	std::vector<CFuelSubsystem*>::iterator it;
	for	(it = gsys->fsub.begin(); it != gsys->fsub.end(); it++)
	{	CSubsystem *sys = *it;
		if (!sys->IsReceiver(msg))  continue;
    TagToString(msg->dst,msg->group);
    return true;
	}
	return false;	}
//-----------------------------------------------------------------------------
//	JSDEV* Find message receiver in engine subsystems
//-----------------------------------------------------------------------------
bool CAirplane::FindReceiver (SMessage *msg,CEngineManager *engs)
{	if (engs == NULL)	return false;
	std::vector<CEngine *>::iterator it;
	for	(it = engs->engn.begin(); it != engs->engn.end(); it++)
	{	CSubsystem *sys = *it;
		if (!sys->IsReceiver(msg))  continue;
    TagToString(msg->dst,msg->group);
    return true;
	}
	return false;	}
//-------------------------------------------------------------------------------
//	Send to all airplane components
//--------------------------------------------------------------------------------
EMessageResult CAirplane::ReceiveMessage(SMessage *msg)
{ EMessageResult           rc = SendMessageToAmpSystems(msg);
  //--If not found, try to find message receiver in list of fuel subsystems
  if (msg->receiver)		return rc;
	rc = SendMessageToGasSystems(msg);
  //--If not found try to find a reciver in list of engines ------------
  if (msg->receiver)		return rc;
	rc = SendMessageToEngSystems(msg);
  //--if not found try to find a receiver in external systems -----------
  if (msg->receiver)		return rc;
	return SendMessageToExternals(msg);
}
//-------------------------------------------------------------------------------
//	Send to electrical subsystem
//--------------------------------------------------------------------------------
EMessageResult CAirplane::SendMessageToAmpSystems(SMessage *msg)
{ EMessageResult  rc   = MSG_IGNORED;
  // Electrical systems are valid, send msg to each subs until handler found
  std::vector<CSubsystem*>::iterator i;
  for (i=amp.subs.begin(); i!=amp.subs.end(); i++)
  { CSubsystem *sub = *i;
	  if (sub->MsgForMe(msg) == false)	continue;		// Just ready for it
    rc = sub->ReceiveMessage (msg);
    msg->receiver = sub;
		return rc;
  }
  return rc;
}
//-------------------------------------------------------------------------------
//	Send to gas subsystems
//--------------------------------------------------------------------------------
EMessageResult CAirplane::SendMessageToGasSystems (SMessage *msg)
{ EMessageResult  rc   = MSG_IGNORED;
  // Fuel systems are valid, send msg to each subs until handler found
  std::vector<CFuelSubsystem*>::iterator i;
  for (i=gas.fsub.begin(); i!=gas.fsub.end(); i++)
  { CSubsystem *sub = *i;
	  if (sub->MsgForMe(msg) == false)	continue;		// Just ready for it
    rc = sub->ReceiveMessage (msg);
    msg->receiver = sub;
    return rc;
  }
  return rc;
}
//-------------------------------------------------------------------------------
//	Send to engine subsystems
//--------------------------------------------------------------------------------
EMessageResult CAirplane::SendMessageToEngSystems (SMessage *msg)
{ EMessageResult  rc   = MSG_IGNORED;
  //
  // Engine systems are valid, send msg to each subs until handler found
  //
  std::vector<CEngine *>::iterator i;
  for (i=eng.engn.begin(); i!=eng.engn.end(); i++)
  { CEngine      *egs = (*i);
	  if (egs->MsgForMe(msg) == false)	continue;		
    rc = egs->ReceiveMessage (msg);
    msg->receiver = egs;
    TagToString(msg->dst,msg->group);
    return rc;
  }
  return rc;
}
//-------------------------------------------------------------------------------
//	Send to external subsystems
//--------------------------------------------------------------------------------
EMessageResult CAirplane::SendMessageToExternals (SMessage *msg)
{ EMessageResult  rc   = MSG_IGNORED;
  // Electrical systems are valid, send msg to each subs until handler found
  std::vector<CSubsystem*>::iterator i;
  for (i=amp.sext.begin(); i!=amp.sext.end(); i++)
  { CSubsystem *sub = *i;
	  if (sub->MsgForMe(msg) == false)	continue;		// Just ready for it
    rc = sub->ReceiveMessage (msg);
    msg->receiver = sub;
		return rc;
  }
  return rc;
}
//-------------------------------------------------------------------------------
//	Return engine subsystems
//--------------------------------------------------------------------------------
void CAirplane::GetAllEngines(std::vector<CEngine*> &engs)
{ engs = eng.engn;
  return;
}
//-------------------------------------------------------------------------------
//  Collision detected 
//--------------------------------------------------------------------------------
void CAirplane::BodyCollision(CVector &p)
{ if (NotOPT(VEH_D_CRASH)) return;
  DAMAGE_MSG msg = {3,0,'crby',"STRUCTURAL DAMAGE"};
  DamageEvent(&msg);
	//--- Trace location -----------------------------
	int gnd = globals->tcm->GetGroundAltitude();
	int alt = globals->geop.alt;
	
	//--- Trace physical -----------------------------
	opal::Solid *phyM = (opal::Solid*)GetPhyModel();
	opal::Point3r pos = phyM->getPosition();
	TRACE("CRASH: alti: %05d ground: %05d phyAlt: %.0lf",alt,gnd,pos.z);
  return;
}
//-------------------------------------------------------------------------------
//  Toggle virtual pilot ON/OFF 
//--------------------------------------------------------------------------------
void CAirplane::ToggleVPIL()
{	VPilot *vpil = amp.GetVirtualPilot();
	vpil->ToggleState();
}
//-----------------------------------------------------------------------------
//  Process according to vehicle state
//  At initialisatioo, check for terrain to come ready
//  when operational 
//  -get joystick values
//  -Time slice all parts
//  NOTE:   For now, it is assumed that the .sit file describes
//          the aircraft at rest on ground level.
//
//          In the futur, when the .sit file describes in flight situation
//          we will have to make some adaptation
//-----------------------------------------------------------------------------
int CAirplane::TimeSlice(float dT,U_INT frame)
{ //--------------------------------------------------------
	pit.TimeSlice(dT);
	CJoysticksManager *jsm = globals->jsm;
  switch (State)  {
    //-- At start up wait for terrain to be stable ---------
    case VEH_INIT:
      { RestOnGround();
	      return 1;
      }
    //--- Aircraft is crashing ----------------------------
    case VEH_CRSH:
				if (!globals->snd->IsPlaying(sound)) State = VEH_INOP;
				return 1;
    //--- Aircraft is operational or damaged----------------
    case VEH_INOP:
    case VEH_OPER:
      { int  nbEng = amp.pEngineManager->HowMany();
        jsm->SendGroupPMT(this,nbEng);               // Send Prop-mixture and throttle
        Update (dT,frame);
        return 1;
      }

  }
  return 1;
}

//----------------------------------------------------------------------------
//	Reset crash:
//----------------------------------------------------------------------------
void CAirplane::ResetCrash(char p)
{ //--- Cut engines --------------------------------------
	if (globals->aPROF.Has(PROF_EDITOR))	return;
	CWorldObject::ResetCrash(p);
  park  = p;
  CutAllEngines();
  whl.ResetCrash();
  //--- Request to Level aircraft ------------------------
  globals->slw->Level(1);
}
//----------------------------------------------------------------------------
//  Aircraft is now level
//----------------------------------------------------------------------------
void CAirplane::EndLevelling()
{ //--- Set orientation ----------------------------------
  ResetZeroOrientation ();
  //--- Set parking brakes ------------------------------
  if (park) ParkBrake (1);
  damM.Severity		= 0;
	damM.msg				= 0;
  globals->fui->RazCrash();
  return;
}

//-----------------------------------------------------------------------------
//  Keyboard interface for controls  
//-----------------------------------------------------------------------------
bool CAirplane::CenterControls()
{ globals->slw->Level(0);
  //----Reset all axis -------------------
  CAeroControl *a = amp.pAils;
  if (a) a->Zero();
  CAeroControl *e = amp.pElvs;
  if (e) e->Zero();
  CAeroControl *r = amp.pRuds;
  if (r) r->Zero();
  CAeroControl *t = amp.eTrim;
  if (t) t->Zero();
  //------Set level ---------------
  return true;
}
//-----------------------------------------------------------------------------
void CAirplane::AileronIncr (void)
{ // Get pointer to control subsystem in electrical systems
  CAeroControl *p = amp.pAils;
  if (p) p->Incr ();
}
//-----------------------------------------------------------------------------
void CAirplane::AileronDecr (void)
{ // Get pointer to control subsystem in electrical systems
  CAeroControl *p = amp.pAils;
  if (p) p->Decr ();
}
//-----------------------------------------------------------------------------
void CAirplane::AileronSet (float fv)
{ // Get pointer to control subsystem in electrical systems
  CAeroControl *p = amp.pAils;
  if (p) p->SetValue (fv);
}
//-----------------------------------------------------------------------------
void CAirplane::ElevatorIncr (void)
{ // Get pointer to control subsystem in electrical systems
  CAeroControl *p = amp.pElvs;
  if (p) p->Incr ();
}
//-----------------------------------------------------------------------------
void CAirplane::ElevatorDecr (void)
{ // Get pointer to control subsystem in electrical systems
  CAeroControl *p = amp.pElvs;
  if (p) p->Decr ();
}
//-----------------------------------------------------------------------------
void CAirplane::ElevatorSet (float fv)
{ // Get pointer to control subsystem in electrical systems
  CAeroControl *p = amp.pElvs;
  if (p) p->SetValue (fv);
}
//-----------------------------------------------------------------------------
void CAirplane::RudderOpalCoef (float fv)
{ // Get pointer to control subsystem in electrical systems
  CRudderControl *p = amp.pRuds;
  if (p) p->SetOpalCoef (fv);
}
//-----------------------------------------------------------------------------
void CAirplane::RudderBankMap (CFmtxMap *m)
{ // Get pointer to control subsystem in electrical systems
  CRudderControl *p = amp.pRuds;
  if (p) p->SetBankMap(m);
}

//-----------------------------------------------------------------------------
void CAirplane::RudderIncr (void)
{
  // Get pointer to control subsystem in electrical systems
  CRudderControl *p = amp.pRuds;
  if (p) p->Incr ();
}
//-----------------------------------------------------------------------------
void CAirplane::RudderDecr (void)
{ // Get pointer to control subsystem in electrical systems
  CRudderControl *p = amp.pRuds;
  if (p) p->Decr ();
}
//-----------------------------------------------------------------------------
void CAirplane::RudderSet (float fv)
{ // Get pointer to control subsystem in electrical systems
  CAeroControl *p = amp.pRuds;
  if (p) p->SetValue (fv);
}
//-----------------------------------------------------------------------------
void CAirplane::GroundBrakes (U_CHAR b) 
{  // Get pointer to control subsystem in electrical systems
  CBrakeControl *p = amp.pwb;
  if (p) p->HoldBrake(b);
}
//-----------------------------------------------------------------------------
void CAirplane::ParkBrake (U_CHAR opt) 
{  // Get pointer to control subsystem in electrical systems
  CBrakeControl *p = amp.pwb;
  if (p) p->SwapPark(opt);
}
//-----------------------------------------------------------------------------
void CAirplane::GearUpDown (void) // 
{ // Get pointer to control subsystem in electrical systems
  CGearControl *p = amp.pgr;
  if (p) p->Swap ();
}
//-----------------------------------------------------------------------------
void CAirplane::FlapsExtend (void)
{  // Get pointer to control subsystem in electrical systems
  CFlapControl *p = amp.pFlaps;
  if (p) p->Incr ();
}
//-----------------------------------------------------------------------------
void CAirplane::FlapsRetract (void)
{  // Get pointer to control subsystem in electrical systems
  CFlapControl *p = amp.pFlaps;
  if (p) p->Decr ();
}
//-----------------------------------------------------------------------------
void CAirplane::AileronTrimIncr (void)
{  // Get pointer to control subsystem in electrical systems
  CAeroControl *p = amp.aTrim;
  if (p) p->Incr ();
}
//-----------------------------------------------------------------------------
void CAirplane::AileronTrimDecr (void)
{  // Get pointer to control subsystem in electrical systems
  CAeroControl *p = amp.aTrim;
  if (p) p->Decr ();
}
//-----------------------------------------------------------------------------
void CAirplane::AileronTrimSet (float fv)
{ // Get pointer to control subsystem in electrical systems
  CAeroControl *p = amp.aTrim;
  if (p) p->SetValue (fv);
}
//-----------------------------------------------------------------------------
void CAirplane::ElevatorTrimIncr (void)
{ // Get pointer to control subsystem in electrical systems
  CAeroControl *p = amp.eTrim;
  if (p) p->Incr ();
}
//-----------------------------------------------------------------------------
void CAirplane::ElevatorTrimDecr (void)
{ // Get pointer to control subsystem in electrical systems
  CAeroControl *p = amp.eTrim;
  if (p) p->Decr ();
}
//-----------------------------------------------------------------------------
void CAirplane::ElevatorTrimSet (float fv)
{ // Get pointer to control subsystem in electrical systems
  CAeroControl *p = amp.eTrim;
  if (p) p->SetValue (fv);
}
//-----------------------------------------------------------------------------
void CAirplane::RudderTrimIncr (void)
{ // Get pointer to control subsystem in electrical systems
  CAeroControl *p = amp.rTrim;
  if (p) p->Incr ();
}
//-----------------------------------------------------------------------------
void CAirplane::RudderTrimDecr (void)
{ // Get pointer to control subsystem in electrical systems
  CAeroControl *p = amp.rTrim;
  if (p) p->Decr ();
}
//-----------------------------------------------------------------------------
void CAirplane::RudderBias (float inc)
{ // Get pointer to control subsystem in electrical systems
  CAeroControl *p = amp.pRuds;
  if (p) p->ModBias(inc);
}

//-----------------------------------------------------------------------------
void CAirplane::RudderTrimSet (float fv)
{  // Get pointer to control subsystem in electrical systems
  CAeroControl *p = amp.rTrim;
  if (p) p->SetValue (fv);
}
//-----------------------------------------------------------------------------
float CAirplane::Aileron (void)
{  float fv = 0.0f;
  CAeroControl *p = amp.pAils;
  if (p) fv = p->Val ();
  return fv;
}
//-----------------------------------------------------------------------------
float CAirplane::AileronDeflect (void)
{  float fv = 0.0f;
  CAeroControl *p = amp.pAils;
  if (p) fv = p->Deflect ();
  return fv;
}
//-----------------------------------------------------------------------------
float CAirplane::Elevator (void)
{  float fv = 0.0f;
  CAeroControl *p = amp.pElvs;
  if (p) fv = p->Val( );
  return fv;
}
//-----------------------------------------------------------------------------
float CAirplane::ElevatorDeflect (void)
{  float fv = 0.0f;
  CAeroControl *p = amp.pElvs;
  if (p) fv = p->Deflect ();
  return fv;
}
//-----------------------------------------------------------------------------
float CAirplane::Rudder (void)
{ float fv = 0.0f;
  CAeroControl *p = amp.pRuds;
  if (p) fv = p->Val();
  return fv;
}
//-----------------------------------------------------------------------------
float CAirplane::RudderDeflect (void)
{ float fv = 0.0f;
  CAeroControl *p = amp.pRuds;
  if (p) fv = p->Deflect ();
  return fv;
}
//-----------------------------------------------------------------------------
float CAirplane::AileronTrim (void)
{ float fv = 0.0f;
  CAeroControl *p = amp.aTrim;
  if (p) fv = p->Val( );
  return fv;
}
//-----------------------------------------------------------------------------
float CAirplane::ElevatorTrim (void)
{ float fv = 0.0f;
  CAeroControl *p = amp.eTrim;
  if (p) fv = p->Val( );
  return fv;
}
//-----------------------------------------------------------------------------
float CAirplane::RudderTrim (void)
{ float fv = 0.0f;
  CAeroControl *p = amp.rTrim;
  if (p) fv = p->Val( );
  return fv;
}

//-----------------------------------------------------------------------------
float CAirplane::Flaps (void)
{ float fv = 0.0f;
  CFlapControl *p = amp.pFlaps;
 // if (p) fv = p->Val( );
  return fv;
}

void CAirplane::Print (FILE *f)
{
}

//=========================================================================
// CUFOObject
// Add this section to the FlyLegacy.ini file
// or it's a CAircraftObject which is being created
//
// [PHYSICS]
// aircraftPhysics=ufo
//
//=========================================================================
CUFOObject::CUFOObject (Tag typ)
{ SetType(typ);
  SetOPT(VEH_IS_FLY + VEH_IS_UFO);				// Flying UFO
  int val = 0;
  GetIniVar ("Sim", "showPosition", &val);
  show_position = val ? true : false;
	//--- Register as user vehicle -----------------------------------
	InitUserVehicle();
}
//-------------------------------------------------------------------
//	Simulate UFO
//-------------------------------------------------------------------
void CUFOObject::Simulate (float dT,U_INT FrNo) 
{
  // Call parent class simulation timeslice
  CVehicleObject::Simulate (dT,FrNo);
  // Timeslice control mixers
  UpdateOrientationState(dT, FrNo);
}

void CUFOObject::UpdateOrientationState (float dT, U_INT FrNo) 
{ CVector tmp_iang = GetOrientation ();
 //
 double x_     =   Elevator();   
 double y_     =   Aileron();   
 double z_     =   Rudder();    
  
 tmp_iang.x  = WrapPiPi (tmp_iang.x - (DEG2RAD * x_));        // pitch
 tmp_iang.y  = WrapPiPi (tmp_iang.y + (DEG2RAD * y_));        // roll
 tmp_iang.z += (DEG2RAD * z_ / 10.0);                         // head
 // adjustments
 tmp_iang.z = WrapTwoPi (tmp_iang.z - (tmp_iang.y * (DEG2RAD * 10.0) * dT / (DEG2RAD * 45.0)));                                // roll
 tmp_iang.y = WrapPiPi  (tmp_iang.y - (DEG2RAD * (z_ / 10.0)));    // 

 SetObjectOrientation (tmp_iang);
 float coef  = 0;
 globals->jsm->Poll(this,JS_THROTTLE_1,coef);
 float speed = 100.0f * coef * 5;
 float spd = (speed * FEET_PER_NM) / 3600.0;

 UpdateNewPositionState(dT,spd);

 //! current state becomes previous state
 prv = cur;
 cur = (cur + 1) & 1;
 //
 vb[cur].y = speed;
 vi[cur].y = speed;
}
//--------------------------------------------------------------------------------
//  Update Position
//--------------------------------------------------------------------------------
int	CUFOObject::UpdateNewPositionState(float dT, float spd)
{   double distance1 = (spd * dT),
           distance2 = distance1;
    SPosition pos_from = GetPosition (),
              pos_to;
    SVector orientation = GetOrientation ();

    distance1 *= cos (orientation.x);
    //
    SVector vect;
    vect.z = 0;
    vect.x = cos (orientation.z + HALF_PI) * distance1;
    vect.y = sin (orientation.z + HALF_PI) * distance1;
    //
    pos_to = AddToPositionInFeet (pos_from, vect, globals->exf);

    pos_to.alt = pos_from.alt + sin (orientation.x) * distance2;

    if (!globals->sBar && show_position) {
        char buff [128] = {'\0'};
        char edt1 [128] = {'\0'};
        char edt2 [128] = {'\0'};
        EditLon2DMS(pos_to.lon, edt1);
        EditLat2DMS(pos_to.lat, edt2);
        _snprintf (buff,126, "@1 %.2f %.2f %.2f [%s] [%s] alt=%.0f\n",
                                    orientation.x, orientation.y, orientation.z,
                                    edt1, edt2, pos_from.alt
                                    );
        DrawNoticeToUser (buff, 1);
    }

    SetObjectPosition (pos_to);
    //
    return 1;
}

//
// COPALObject
// Add this section to the FlyLegacy.ini file
// or it's a CAircraftObject which is being created
//

opal::Vec3r CVectorToVec3r (const CVector &v)
{
  opal::Vec3r ov;
  ov.x = static_cast<opal::real> (v.x);
  ov.y = static_cast<opal::real> (v.y);
  ov.z = static_cast<opal::real> (v.z);
  return ov;
}

CVector Vec3rToCVector (const opal::Vec3r &v)
{
  CVector ov;
  ov.x = static_cast<double> (v.x);
  ov.y = static_cast<double> (v.y);
  ov.z = static_cast<double> (v.z);
  return ov;
}

//
// [PHYSICS]
// aircraftPhysics=opal
//
//=========================================================================
/// COPALObject
//=========================================================================
COPALObject::COPALObject (Tag typ)
{ //--- Set type -----------------------------------------------
	SetType(typ);
  log							= NULL;
  Kb							= 0.0;
  bagl            = 0.0;
  Plane						= 0;
  wind_effect			= 0; //
  wind_pos        = 0.0;
	turbulence_effect	= 0;
	group						= 2;
  //------------------------------------------------------------
	yawMine		= ADJ_YAW_MINE;
	rollMine	= ADJ_ROLL_MINE;
  pitchMine = ADJ_PTCH_MINE;
	angularDamping = ADJ_ANGL_DAMPG;
	linearDamping  = ADJ_LINR_DAMPG;
  //--- randomisation used in UpdateNewPositionState -----------
  int rdn = (globals->clk->GetMinute() << 8) +  globals->clk->GetSecond();
  srand(rdn);

}
//---------------------------------------------------------------------------------------
//  DESTRUCTOR
//---------------------------------------------------------------------------------------
COPALObject::~COPALObject (void)
{ //--- Remove from dispatcher ---------------------------
	if (IsUserPlan())
		{	globals->jsm->Register(0);
			globals->Disp.Remove(PRIO_UPLANE);
		}
	//--- Destroy opal objects -----------------------------
  if (Plane)  globals->opal_sim->destroySolid(Plane);
  Plane  = 0;
  //-----------------------------------------------------
  SAFE_DELETE (log);
  // sdk : remove any left behind dll object
  if (globals->plugins_num) globals->plugins->On_DeleteObjects ();//
}
//---------------------------------------------------------------------------------------
//  All parameters are read
//---------------------------------------------------------------------------------------
void COPALObject::PlaneShape()
{ //-------------------------------------------------
  TRACE("COPALObject::PlaneShape");
  Plane           = globals->opal_sim->createSolid();
  Plane->setName(planeID);
	Plane->setUserData(this);
	phyMod					= Plane;
  //----Compute cog offset --------------------------
  double ftm  = FN_METRE_FROM_FEET(float(1));
  CVector cog = wgh.svh_cofg;
  cog.Times(ftm);
  opal::Vec3r   cms(cog.x, cog.z, cog.y);
  //-----Get body dimensions ------------------------
  CVector dim;
  lod.GetBodyExtension(dim);
  dim.Times(ftm);
  //-----Compute mid point for extension ------------
  CVector mid = dim;
  mid.Times(0.5);
  //-----Get minimum extension ----------------------
  CVector mex;
  lod.GetMiniExtension(mex);
  mex.Times(ftm);
  //-----Compute volume translation -----------------
  CVector trs = (mex + mid);
  //-------------------------------------------------
  opal::BoxShapeData plData;
  plData.contactGroup = group;
  plData.material.hardness   = opal::real(0.9f); // was 1.0 
  plData.material.bounciness = opal::real(0.0f);
  plData.material.friction   = opal::real(1.0f); // was 0.0 
  plData.dimensions.x = dim.x;
  plData.dimensions.y = dim.y;
  plData.dimensions.z = dim.z;
  plData.material.density = opal::real(1.0f);  //
  plData.setUserData(0,SHAPE_BODY);
  //----Set crash detection ---------------------------------------------------
  //  The crash box middle point is set at the CG.y in the longitudinal direction
  //  The box bottom is set above the aircraft bottom
  //---------------------------------------------------------------------------
  plData.offset.translate(0,trs.y,(dim.z * 0.75));
  Plane->addShape (plData);
  Plane->setCollisionEventHandler(CrashHandler);
  // Setup the contact groups.
  globals->opal_sim->setupContactGroups(1, group, true);
  //----------- forces ------------------------------------------------------
  lf.type = opal::LOCAL_FORCE;//
  ef.type = opal::LOCAL_FORCE_AT_LOCAL_POS; // 
  ed.type = opal::LOCAL_FORCE_AT_LOCAL_POS; // 
  tf.type = opal::LOCAL_TORQUE; // 
  wm.type = opal::LOCAL_FORCE_AT_LOCAL_POS;
  yf.type = opal::LOCAL_FORCE_AT_LOCAL_POS;
	//--- Build gears ----------------------------------------------------------
	whl.BuildGears();
  return;
}
//---------------------------------------------------------------------------------------
//  All parameters are read
//---------------------------------------------------------------------------------------
void COPALObject::ReadFinished (void)
{ //MEMORY_LEAK_MARKER ("readfnopa")
	//--- Must register to joystick to process mapping order -----
	if (IsUserPlan())	globals->jsm->Register(this);			
	//---Must finish to get PHY parameters -----------------------
	CAirplane::ReadFinished ();
	//--- Register as user vehicle -------------------------------
	InitUserVehicle();
	if (!IsUserPlan())	group = 3;
	PlaneShape();
	//--- Set minimum agl ---------------------------------------------
	magl	= whl.GetMinimumBodyAGL();
	//------------------------------------------------------------
	//  Enter in dispatcher
	//------------------------------------------------------------
	globals->sit->AddPlane(this);
  PrepareMsg ();		// MARK TTEST
	//-----------------------------------------------------------------
  mm.mass = static_cast<opal::real> (wgh.GetTotalMassInKgs () /** 9.81f*/); // 
  //-----------------------------------------------------------------
  //  Init from PHY file (either default or from file)
  //------------------------------------------------------------------
  yawMine					= phy.Ymin;
  rollMine				= phy.Rmin;
  pitchMine				= phy.Pmin;
  dihedral_coeff	= phy.Kdeh;
  acrd_coeff			= phy.Krud;
	pitch_coeff			= phy.Kpth;
  wind_coeff			= phy.Kwnd;
  gear_drag				= phy.KdrG;
  //---Init rudder coef ------------------------------------------
	//RudderOpalCoef(acrd_coeff);
  ///----------------------------------------------------------------
  DEBUGLOG ("PHY : dieh=%f pitchK%f acrd=%f", dihedral_coeff, pitch_coeff, acrd_coeff);
  DEBUGLOG ("PHY : pmine%f rmine%f ymine%f", pitchMine, rollMine, yawMine); 
  DEBUGLOG ("PHY : wind effect coeff%f", wind_coeff);
  ///-----------------------------------------------------------------
  /// turbulence speed used in UpdateNewPositionState
  if (turbulence_effect) {
    tVAL.Conf (INDN_LINEAR, svh.wTrbTimK);
    svh.wTrbSpeed = KtToFps (static_cast <double> (svh.wTrbSpeed));
  }
  ///
  CVector   Cg   = wgh.svh_cofg;
  double    cy   = FN_METRE_FROM_FEET(Cg.z);
  double    ix   = (wgh.svh_mine.x*(1.0f / pitchMine) * LBFT2_TO_KGM2);
  double    iy   = (wgh.svh_mine.z*(1.0f / rollMine)  * LBFT2_TO_KGM2);
  double    iz   = (wgh.svh_mine.y*(1.0f / yawMine)   * LBFT2_TO_KGM2);
  mm.center      = opal::Vec3r ( 0, cy, 0 );
  mm.inertia[0]  = opal::real(ix);  // 
  mm.inertia[5]  = opal::real(iy);  // 
  mm.inertia[10] = opal::real(iz);
	
  Plane->setMass (mm, mm.inertia); // 
  GetIniFloat ("PHYSICS", "adjustLinearDamping",  &linearDamping);
  GetIniFloat ("PHYSICS", "adjustAngularDamping", &angularDamping);
  Plane->setLinearDamping  ((opal::real) (linearDamping)); // default 0.15
  Plane->setAngularDamping ((opal::real) (angularDamping)); // default 0.15 but actually 1.0
  TRACE("COPALObject::ReadFinished");
  DEBUGLOG ("COPALObject::ReadFinished\n  emas = %f\n  gross = %f\n  mine = %f %f %f\n\
  cg = %f %f %f\n  cgos = %f %f %f\n  y%f/r%f/r%f\n  in%f*%f*%f\n\
  ld%f/ad%f",
                         wgh.GetEmptyMassInLbs (),
                         wgh.GetTotalMassInLbs (),
                         wgh.svh_mine.x, wgh.svh_mine.y, wgh.svh_mine.z,
                         wgh.svh_cofg.x, wgh.svh_cofg.y, wgh.svh_cofg.z,
                         wgh.wb.GetCGOffset()->x, wgh.wb.GetCGOffset()->y, wgh.wb.GetCGOffset()->z, 
                         pitchMine, yawMine, rollMine,
                         mm.inertia[0], mm.inertia[10],mm.inertia[5],
                         linearDamping, angularDamping);
	
  //MEMORY_LEAK_MARKER ("readfnopa")
	//---- Ensure position and orientation ---------------------------
  SetObjectPosition(orgp);
  SetObjectOrientation(globals->iang);
	SetPhysicalOrientation (iang);
	ResetSpeeds ();

}
//------------------------------------------------------------------

void COPALObject::ResetOrientation (const CVector &rad_angle)
{
}
//--------------------------------------------------------------
//  Set aircraft orientation with opal object
//--------------------------------------------------------------
void COPALObject::SetPhysicalOrientation (SVector &rad_angle)
{ opal::Matrix44r transform; 
  CVector iang_ = rad_angle;
  iang_.Times (RadToDeg (1.0));
  /// !!! respect rotation order in matrix = HBP
  transform.rotate ( iang_.z /*degres*/, 0.0, 0.0, 1.0);// x,y,z = h
  transform.rotate ( iang_.y /*degres*/, 0.0, 1.0, 0.0);// x,y,z = b
  transform.rotate ( iang_.x /*degres*/, 1.0, 0.0, 0.0);// x,y,z = p
  Plane->setTransform (transform);
}
//---------------------------------------------------------------------
//  Reset all speeds
//---------------------------------------------------------------------
void COPALObject::ResetSpeeds (void)
{ opal::Vec3r zer(0,0,0);
  Plane->setLocalLinearVel (zer);
  Plane->setGlobalLinearVel(zer);
  Plane->setLocalAngularVel (zer);
  Plane->setGlobalAngularVel(zer);
}
//--------------------------------------------------------------------------
//  Reset crash
//--------------------------------------------------------------------------
void COPALObject::ResetCrash (char p)  
{ if (State == VEH_CRSH)  return;
  CAirplane::ResetCrash(p);
  //--- Zero velocity ------------------------------------
  ResetSpeeds ();
  //--- Zero Forces and Moments --------------------------
  Plane->zeroForces ();
  //------------------------------------------------------
	globals->fui->RazCrash();
	globals->sit->Reload();
  return;
}
//--------------------------------------------------------------------------
//  Szet wind position
//--------------------------------------------------------------------------
void COPALObject::SetWindPos(double p)
{wind_pos = std::min(p,wind_pos);}
//--------------------------------------------------------------
//  Set aircraft orientation to 0,0,0
//--------------------------------------------------------------
void COPALObject::ResetZeroOrientation (void)
{ CVector reset_ori = GetOrientation ();
  opal::Matrix44r transform;
  //
  CVector gravity;
  SFlyObjectRef obj;
  obj.objectPtr = this;
  obj.classSig = NULL;
  obj.superSig = NULL;
  APIGetObjectBodyGravity (&obj, &gravity);
  if (gravity.y > 0.0) {
    /// !!! respect rotation order in matrix = HBP
    transform.rotate (  0.0 /*degres*/, 0.0, 0.0, 1.0);
    transform.rotate (180.0 /*degres*/, 0.0, 1.0, 0.0);
    transform.rotate (  0.0 /*degres*/, 1.0, 0.0, 0.0);
    Plane->setTransform (transform);
    reset_ori.z += DegToRad (-180.0);
  }
  transform.makeZero ();
  Plane->setTransform (transform);
  // return to the former heading
  reset_ori.x = 0.0; reset_ori.y = 0.0;
  SetPhysicalOrientation (reset_ori);
  SetObjectOrientation (reset_ori);
}
//------------------------------------------------------------------------------------------
//  TRACE FORCES
//------------------------------------------------------------------------------------------
void COPALObject::TraceForces()
{	TRACE("---------FORCES ------------------------------");
	TRACE("Add WM: x=%.6lf y=%.6lf z=%.6lf",wm.vec.x,wm.vec.y,wm.vec.z);
	TRACE("Add LF: x=%.6lf y=%.6lf z=%.6lf",lf.vec.x,lf.vec.y,lf.vec.z);
	TRACE("Add EF: x=%.6lf y=%.6lf z=%.6lf",ef.vec.x,ef.vec.y,ef.vec.z);
	TRACE("Add TF: x=%.6lf y=%.6lf z=%.6lf",tf.vec.x,tf.vec.y,tf.vec.z);
	TRACE("Add YF: x=%.6lf y=%.6lf z=%.6lf",yf.vec.x,yf.vec.y,yf.vec.z);

}
//------------------------------------------------------------------------------------------
//  SIMULATE
//------------------------------------------------------------------------------------------
void COPALObject::Simulate (float dT,U_INT FrNo) 
{ /// Call parent class simulation timeslice
	if (dT > globals->TimLim) return; // fix to avoid aerodynamics errors when fps are low (related to fps limiter)
  CVehicleObject::Simulate (dT, FrNo);
  //---Aircraft orientation ------------------------------------------------------
  SVector or_s = GetOrientation (); // RH
  lf.vec.set (static_cast<opal::real> (0.0), static_cast<opal::real> (0.0), static_cast<opal::real> (0.0));
  tf.vec.set (static_cast<opal::real> (0.0), static_cast<opal::real> (0.0), static_cast<opal::real> (0.0));
  ef.vec.set (static_cast<opal::real> (0.0), static_cast<opal::real> (0.0), static_cast<opal::real> (0.0));
  ed.vec.set (static_cast<opal::real> (0.0), static_cast<opal::real> (0.0), static_cast<opal::real> (0.0));
  wm.vec.set (static_cast<opal::real> (0.0), static_cast<opal::real> (0.0), static_cast<opal::real> (0.0));
  yf.vec.set (static_cast<opal::real> (0.0), static_cast<opal::real> (0.0), static_cast<opal::real> (0.0));
	//-------------------------------------------------------------------------------
  mm.mass = CVehicleObject::GetMassInKgs ();									//
  Plane->setMass (mm, mm.inertia);														// 

  // 1) wing & engine forces
  fb.Raz();						// JS: Replace Times(0) because when #NAND or #IND, then fb is not reset
  if (caging_fixed_wings) fb.Add (wng.GetForce ()); // LH
  VectorDistanceLeftToRight  (fb); // LH=>RH

  // 1b) engine force
  // engine force and moment
  // for single engine, moment is computed
  // with engine position force
  float s_p_d = 0.0f;
	if (1 == GetEngNb())	
				engines_pos = eng.GetEnginesPosISU (); // LH
  else  engines_pos.Raz();                // engines_pos = 0

  // simulate gear down drag and moment
  // modifying the thrust value and position
  double fake_gear_drag = 1.0;
  if (lod.AreGearDown() &&  !WheelsAreOnGround())
  { // gear down
    // create a torque with engine position
    engines_pos.y += FN_METRE_FROM_FEET (whl.GetCurWheelHeight () * 0.5); // meters
    /// \todo verify if 2.5% thrust reduction is correct
    fake_gear_drag = static_cast<double> (gear_drag); // thrust reduction
  }
  //
  ef.pos.set (static_cast<opal::real> ( engines_pos.x),
              static_cast<opal::real> ( engines_pos.z),
              static_cast<opal::real> ( engines_pos.y)); // LH to RH
  
  if (!has_fake_engine_thrust) {
    s_p_d = (eng.GetForceISU ()).z * fake_gear_drag; //
    // engine drag from propellers
		bool tmp = 0;
    if (tmp) { // temporary condition to develop
      ed.pos.x = -ef.pos.x; ed.pos.y = ef.pos.y; ed.pos.z = ef.pos.z;
      // up to now it is assumed that the prop isn't feathered and drag is max
      // if prop is feathered then drag is minored
      ed.vec.y = static_cast<opal::real> (-eng.GetForceISU ().z * 0.1); // LH->RH
    }
  }
  else
	{	float coef = 0;
		globals->jsm->Poll(this,JS_THROTTLE_1,coef);
    s_p_d = coef * CVehicleObject::GetMassInKgs () * 20.0;
	}

  ef.vec.y    = static_cast<opal::real> (s_p_d); // LH->RH
  ef.duration = static_cast<opal::real> (dT); //
  ed.duration = static_cast<opal::real> (dT);

  // 2)  wing, mine & engine moments
  tb.Raz();  // JS: Replace Times(0) because when #NAND or #IND, then tb is not reset
  if (caging_fixed_wings) tb.Add (wng.GetMoment ());// 
  VectorOrientLeftToRight (tb); // 

  // dihedral additional effect SVH <dieh>
  if (0.0 == AileronDeflect () + RudderDeflect ()) {
    CFmtxMap *mdieh = svh.GetDieh();
    if (mdieh) 
    { /// SVH <dieh> tag exists
      float dieh_K = dihedral_coeff; // 
      tb.y += double(dieh_K * mdieh->Lookup (float(RadToDeg (or_s.y)))); //  
    }
  }

  // pitch additional effect SVH <pitd>
  if (0.0 == ElevatorDeflect ()) {
    CFmtxMap * mpitd = svh.GetPitd();
    if (mpitd) { 

      /// SVH <pitd> tag exists
      opal::Vec3r ang_vel = Plane->getLocalAngularVel (); ///< RH deg/sec
      tb.x += double(pitch_coeff * mpitd->Lookup (float(ang_vel.x))); //
    } else {
      /// no SVH <pitd> tag present : default
      opal::Vec3r ang_vel = Plane->getLocalAngularVel (); ///< RH deg/sec
      tb.x += double(pitch_coeff * float(-ang_vel.x)); //
    }
  }

  // 2b) moment of inertia
  // not used

  // 2c) multiple engine moment
  if (GetEngNb() > 1) {// dual and more engine
    SVector eng_m = eng.GetMomentISU ();
    VectorOrientLeftToRight (eng_m);
    tb.Add (eng_m);
  }

  // 3) gear forces & moments
  if (WheelsAreOnGround ()) {
   // wheels are on ground
   // simulate ground friction 
  // opal::real thrust_on_ground = ef.vec.y - static_cast<opal::real> (CVehicleObject::GetMassInKgs());
  // ef.vec.y = (thrust_on_ground > (opal::real) 0.0) ? thrust_on_ground : (opal::real) 0.0;

   CVector gear_force = *whl.GetSumGearForces (); // LH m/s
   VectorDistanceLeftToRight  (gear_force);        // RH 
   fb.Add (gear_force);                            // 

   // add gear moment
   CVector gear_moment = *whl.GetSumGearMoments ();
   VectorOrientLeftToRight (gear_moment); // 
   tb.Subtract (gear_moment);
   // 
 } else {
    // WIND TEST // 
    if (wind_effect) {
      double w_K = wind_coeff;// magic number
      if (WheelsAreOnGround ())  w_K *= 0.5; // was 1.0

      const CVector w_dir_for_body_ = w_dir_for_body;
      wm.vec.set (  static_cast<opal::real> (-w_dir_for_body_.x * dT * w_K),// LF to RH
                    static_cast<opal::real> (-w_dir_for_body_.z * dT * w_K),// LF to RH
                    static_cast<opal::real> (-w_dir_for_body_.y * dT * w_K)
                 );
      wm.pos.set (  static_cast<opal::real> (0.0),
                    static_cast<opal::real> (
                    wind_pos /** 0.5*/
                    /*FN_METRE_FROM_FEET (-22.0)*/),                              // LH to RH
                    static_cast<opal::real> (0.0)
                 );
      wm.duration = static_cast<opal::real> (dT);
			Plane->addForce (wm); 
			
    }
  }

  if (caging_fixed_alt) fb.z = double(wgh.GetTotalMassInKgs ()) * GRAVITY_MTS;   // 

  lf.vec = CVectorToVec3r (fb);
  lf.duration = opal::real(dT); 
  if (caging_fixed_roll)   tb.y = caging_fixed_roll_dval;

  tf.vec = CVectorToVec3r (tb);
  tf.duration = opal::real(dT);//
  Plane->addForce (lf); //
	Plane->addForce (ef); //
  Plane->addForce (tf); //
  if (caging_fixed_sped) {
    opal::Vec3r fixed_vel = Plane->getLocalLinearVel ();  // RH  
    fixed_vel.y = FN_METRE_FROM_FEET(KtToFps (caging_fixed_sped_fval));  
    Plane->setLocalLinearVel (fixed_vel);  
  }
  if (turbulence_effect) Plane->addForce (yf);
	return;
}
//------------------------------------------------------------------------------
//	Update all data after the simulation step
//------------------------------------------------------------------------------
void COPALObject::UpdateData(float dT)
{//! current state becomes previous state
  prv = cur;
  cur = (cur + 1) & 1;

//
//   set linear and angular velocities both local
//  and global in LH
//  JS: Add a test and reset when geting a NAND on velocity
	//--- Compute local linear velocity ------------------------------------
  opal::Vec3r local_vel1 = Plane->getLocalLinearVel ();  ///< RH
  while (_isnan(local_vel1.z))
  { ResetSpeeds();          // JS gtfo("Physical engine HS");                // JS
    local_vel1 = Plane->getLocalLinearVel ();
  }
  vb[cur] = Vec3rToCVector (local_vel1);
  VectorDistanceLeftToRight  (vb[cur]); //                        ///< LH
	//--- Compute global velocity -----------------------------------------
  glv = Plane->getGlobalLinearVel ();															///< RH
  vi[cur] = Vec3rToCVector (glv);
  VectorDistanceLeftToRight  (vi[cur]); //                        ///< LH
  // attention : OPAL is deg/sec
  opal::Vec3r local_vel2 = Plane->getLocalAngularVel (); ///< RH
  wb[cur] = Vec3rToCVector (local_vel2);
  VectorOrientLeftToRight (wb[cur]); //                           ///< LH
  wb[cur].Times (ONE_DEGRE_RADIAN);											// JS Change ot constant DegToRad (1.0)); // 
    //
  local_vel2 = Plane->getGlobalAngularVel ();            ///< RH
  wi[cur] = Vec3rToCVector (local_vel2);
  VectorOrientLeftToRight (wi[cur]); //                           ///< LH
  wi[cur].Times (ONE_DEGRE_RADIAN); // 

  //
  //   set linear and angular acceleration both local
  //   and global in LH and m.sec² anr rad.sec²
  //
  ab[cur].Copy  (vb[cur]); ab[cur].Subtract  (vb[prv]); ab[cur].Times  (1.0 / dT);
  ai[cur].Copy  (vi[cur]); ai[cur].Subtract  (vi[prv]); ai[cur].Times  (1.0 / dT);
  dwb[cur].Copy (wb[cur]); dwb[cur].Subtract (wb[prv]); dwb[cur].Times (1.0 / dT);
  dwi[cur].Copy (wi[cur]); dwi[cur].Subtract (wi[prv]); dwi[cur].Times (1.0 / dT);
  //--- ------------------------------------------------------------------------
  UpdateOrientationState(dT, 0);
  UpdateNewPositionState(dT, 0);
}
//------------------------------------------------------------------------------
//  Set aircraft position
//------------------------------------------------------------------------------
void COPALObject::SetObjectPosition (SPosition pos)
{ // Clamp altitude to 100K
  double altClamp = globals->aMax;;
  if (pos.alt > altClamp) pos.alt = altClamp;
  // Clamp latitude to globe tile maximum latitude
  double latClamp = LastLatitude();
  if (pos.lat > +latClamp)  pos.lat = +latClamp;
  if (pos.lat < -latClamp)  pos.lat = -latClamp;
  if (_isnan(pos.lat))  return;
  geop          = pos;
	//--- Compute position AGL for physical model ------
	opal::Point3r bpos(rpos.x,rpos.y,0);
	double cgz  =  wgh.GetCGHeight();
  bagl =  geop.alt - GetGroundAltitude() + cgz;  
  bpos.z      =  bagl;
	bpos       *=	FN_METRE_FROM_FEET(1);
  Plane->setPosition (bpos);
	//TRACE("SetObjectPosition z=%.4lf", bpos.z);
	//--------------------------------------------------
  if (!IsFyingObj())    return;
  //----Save position at global level ----------------
  if (IsUserPlan())	globals->geop = pos;
  return;
}
//--------------------------------------------------------------
//  Set aircraft altitude
//--------------------------------------------------------------
void COPALObject::ChangeAltitude(double alt)
{ // Clamp altitude to 100K
  double altClamp = 1.0E+5;
  if (alt > altClamp) alt = altClamp;
  geop.alt          = alt;
  if (!IsFyingObj())    return;
  //----Save position at global level ----------------
  if (IsUserPlan())	globals->geop.alt = alt;
  return;
}
//------------------------------------------------------------------------------------------
//  We must compute the CoG above ground level
//  When wheel are on ground we must have
//  ground = geop.alt - globals->tcm->GetGroundAltitude() + main_gear_contact_point
//------------------------------------------------------------------------------------------
void COPALObject::PositionAGL()
{ opal::Point3r bpos(rpos.x,rpos.y,0);
	double cgz  =  wgh.GetCGHeight();
  bagl =  geop.alt - GetGroundAltitude() + cgz;  
  bpos.z      =  bagl;
	bpos       *=	FN_METRE_FROM_FEET(1);
  Plane->setPosition (bpos);
	//TRACE("PositionAGL z=%.4lf", bpos.z);
  return;
}
//------------------------------------------------------------------------------------------
//  Set plane on ground
//------------------------------------------------------------------------------------------
void COPALObject::RestOnGround()
{ double    grn = globals->tcm->GetGroundAltitude();
  //--- Zero Forces and Moments --------------------------
  ResetSpeeds ();
  Plane->zeroForces ();
  //--- Clear wheels forces ------------------------------
  whl.ResetForce();
	SPosition p = geop;
	//--- Init spot ----------------------------------------
	Spot.lat	= geop.lat;
	Spot.lon  = geop.lon;
	Spot.agl	= geop.alt - globals->tcm->GetGroundAt(Spot);
	//-------------------------------------------------------
	p.alt	= grn + GetMinimumBodyAGL();
	SetObjectPosition(p);
	bool ok = globals->tcm->TerrainStable(1); 
	if (!ok)		return;
	//-------------------------------------------------------
	State = VEH_OPER;
  return;
}
//------------------------------------------------------------------------------------------
//  Compute plane distance to center in feet 
//	NOTE:  This function is dedicated to animated aircraft only. 
//				 As the user aircraft is always in world origin (0,0,alt) to set other aircraft
//				 we must put them in a relative position to the user aircraft
//------------------------------------------------------------------------------------------
void COPALObject::SetRelativePosition()
{	CVector p0 = FeetComponents(globals->geop,geop,globals->rdf);
  CVector p1 = p0;
	p1.Times(FN_METRE_FROM_FEET(1));
	rpos	= p0;
	Plane->setPosition (p1.x,p1.y,p1.z);
	//TRACE("SetRelativePosition z=%.4lf", p1.z);
	//if (!IsUserPlan()) TRACE("SetP: x=%.6lf y=%.6lf z=%.6lf",p1.x,p1.y,p1.z);
	return;
}
//=================================================================================
//	http://www.aerojockey.com/papers/meng/node19.html
//	Euler angles have two disadvantages. First, the Euler angle equations
//	contain many trigonometric functions. Trigonometric functions are very
//	slow compared to basic arithmetic operations such as addition and multiplication.
//	For computational efficiency, it is almost always better to choose a method
//	using only simple arithmetic operations. 
//	The more important disadvantage is the numerical singularity appearing in
//	equations when Theta is +/- 90°, that is, when the airplane's nose points
//	straight up or down. While not a problem in normal, level flight, the singularity
//	can present numerical problems when the airplane performs maneuvers such as loops. 
//	Replacing the three Euler angles with four quarternions alleviates both difficulties.
//	Because only three parameters define any possible rotation, the quarternions need
//	a constraint so that there are only three independent variables. This constraint is : 
//  e02+e12+e22+e32=1 
//	One nice feature of quarternions is that numerical integration of their time
//	derivatives tends not to destroy the constraint. Thus, a flight simulator only needs
//	to normalize the quarternions occasionally. With high enough numerical precision,
//	t may not ever have to normalize them. 
//
//	http://www.libqglviewer.com/refManual/classqglviewer_1_1Quaternion.html
//	You can apply the Quaternion q rotation to the OpenGL matrices using: 
//	glMultMatrixd(q.matrix());
//	equivalent to glRotate(q.angle()*180.0/M_PI, q.axis().x, q.axis().y, q.axis().z);
//=====================================================================================
void COPALObject::UpdateOrientationState (float dT, U_INT FrNo) 
{ if (globals->slw->IsEnabled())  return;
  CVector orientation;
  // Quaternions
  opal::Quaternion q_ori = Plane->getQuaternion ();
  opal::real angle;
  opal::Vec3r ori2;
  q_ori.getAngleAxis (angle, ori2); // deg XYZ
  // XYZ->HPB
  opal::Matrix44r mtx;
  mtx.makeRotation (angle, ori2.z, ori2.x, ori2.y); // HPB
  opal::Vec3r tmp = mtx.getEulerXYZ ();
  orientation.x = static_cast<double> (DegToRad (tmp.y));				// RH
  orientation.y = static_cast<double> (DegToRad (tmp.z));				// RH
  orientation.z = static_cast<double> (DegToRad (tmp.x));				// RH

  if (caging_fixed_pitch) {
    opal::Matrix44r transform;
    transform.rotate (caging_fixed_pitch_dval, 1.0, 0.0, 0.0);//
    Plane->setTransform (transform);
  }
  SetObjectOrientation (orientation); // RH
  return;
}
//---------------------------------------------------------------------------
//  Set new position according to speed
//	NOTE:  During aircraft implementation bad values may occurs from the
//				opal engine, producing incoherent position.  The TestMode option
//				prevents position to be updated, while all others values are 
//				computed
//---------------------------------------------------------------------------
void COPALObject::UpdateNewPositionState (float dT, U_INT FrNo)
{ if (globals->slw->IsEnabled())  return;
  //opal::Vec3r ov;
  CVector dist;
  //ov      = Plane->getGlobalLinearVel (); // RH
  //spd     = Vec3rToCVector (ov); 
	spd     = Vec3rToCVector (glv);
  dist.x  = MetresToFeet (spd.x) * dT;
  dist.y  = MetresToFeet (spd.y) * dT;
  dist.z  = MetresToFeet (spd.z) * dT;
	//if (IsUserPlan())TRACE("GVEL: x=%.6lf y=%.6lf z=%.6lf",dist.x,dist.y,dist.z);
  SPosition pos_from  = GetPosition ();
  /// turbulences
  if (turbulence_effect) {
			int val = int(globals->wtm->GetTurbulence(dT));
			if (28 == val) globals->evnOpt.Toggle(EVN_RAND_TURBULENCE);
			if (globals->evnOpt.Has(EVN_RAND_TURBULENCE)) {
		    if (bagl > 750.0) { // above 300.0 feet
        float turb_vz = static_cast <float> ((rand () % 200 - 100)) / 100.0f;
        //--- Get final values -----------------------------------------------
        double turbR = static_cast <double> (tVAL.TimeSlice (dT));
        //--- Set Target value -----------------------------------------------
        float tmp_val = turb_vz * svh.wTrbSpeed * svh.wTrbDuration;
        tVAL.Set (tmp_val);
        //
        //TRACE ("K  %f %f %f (%f %f)", turb_vz, tmp_val, turbR, svh->wTrbSpeed, svh->wTrbDuration);
        dist.z += turbR;
				}
			}
	}
  //
  ifpos   = dist;
  SPosition pos_to = AddToPositionInFeet (pos_from, dist, globals->exf); // 
	//--- Test option avoid bad position during aircraft implementation ------
	if (HasOPT(VEH_OP_TEST))	return;
  SetObjectPosition (pos_to);
	return;
}
//---------------------------------------------------------------------------
//  Slew move
//---------------------------------------------------------------------------
void COPALObject::SetOpalSlewOrientation (const SVector &v)
{
  CVector v_;
  v_.x = WrapPiPi ( v.x);
  v_.y = WrapPiPi ( v.y); 
  v_.z = WrapPiPi ( v.z);
  //ResetOpalOrientation (v_);
}

static int print_counter = 0;
void COPALObject::Print (FILE *f)
{ /*
  fprintf(f, "COPALObject::Print\n");

    if (!log) {
      char buffer [128] = {0};
      _snprintf (buffer,126,"Debug/DumpedAeromodel_%d.txt", ++print_counter);
      log = new CLogFile(buffer, "w");
      if (log) {
        SAFE_DELETE (wng->log);
        wng->SetLogPointer (log);
        DrawNoticeToUser ("User vehicle dumped to \"DumpedAeromodel.txt\"", 5);
      }
    } else {
      DrawNoticeToUser ("Stopped \"DumpedAeromodel.txt\"", 5);
      SAFE_DELETE (log); 
      wng->SetLogPointer (NULL);
    }
		*/
}


//
// CHelicopterObject
//

CHelicopterObject::CHelicopterObject (void)
{
  SetType(TYPE_FLY_HELICOPTER);
}



//===========================END OF FILE ==========================================
