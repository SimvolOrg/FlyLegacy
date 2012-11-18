/*
 * WorldObjects.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
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
 * initialSpeedx=0.000000
 * initialSpeedy=0.000000
 * initialSpeedz=40.000000
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
#include "../Include/PlanDeVol.h"
#include <vector>								      // JSDEV* for STL

using namespace std;

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
//======================================================================================
// CWorldObject
//
//======================================================================================
CWorldObject::CWorldObject (void)
{ SetType(TYPE_FLY_WORLDOBJECT);
  damM.Severity		= 0;
	damM.msg				= 0;
	phyMod	= 0;
	//--- Default options -------------------
	SetOPT(VEH_AP_LAND);
  SetOPT(VEH_PN_HELP);
	//--- Option from INI file --------------
	char pm[8];
	GetIniString("Sim","Mode",pm,8);
	if (strcmp(pm,"Test") == 0)	SetOPT(VEH_OP_TEST);

}
//-------------------------------------------------------------------
//  Read parameters
//-------------------------------------------------------------------
void CWorldObject::SetType(Tag t)
{ type = t;
  TagToString(stype,t);
  return;
}
//-------------------------------------------------------------------
//  Read parameters
//-------------------------------------------------------------------
int   CWorldObject::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;
  //MEMORY_LEAK_MARKER ("readworld")

  switch (tag) {
  case 'type':
    // Object type
    ReadTag (&type, stream);
    return TAG_READ;

  case 'geop':
    SPosition(pos);
    // Geographical position, in WGS84 geodetic coordinates
    ReadPosition (&pos, stream);
    orgp  = pos;        // Save original position
    return TAG_READ;

  case 'iang':
    { SVector angu;
      // Inertial angular position, in radians using left-handed coordinate system
      double tmp = 0.0;
      //---JS Don't use magnetic deviation on orientation. use it on compass --------
      ReadVector (&angu, stream);
      tmp = WrapPiPi (angu.z);
      angu.z = WrapPiPi (-angu.y); 
      angu.y = -tmp;
      angu.x = WrapPiPi (-angu.x);
      SetObjectOrientation(angu);
      return TAG_READ;
    }
  default:
    // This is the end of the line...if the tag is not recognized then
    // generate a warning
    gtfo ("CWorldObject::Read : Unrecognized tag <%s> in %s",
      TagToString(tag), stream->filename);
  }
  return TAG_IGNORED;
}
//-------------------------------------------------------------------------
//  All parameters are read
//-------------------------------------------------------------------------
void  CWorldObject::ReadFinished (void)
{ //--- Check configuration for a Goto ---------------------
	char txt[128];
	*txt = '*';
	GetIniString("Sim","goto",txt,127);
	int qx = 0;
	int qz = 0;
	int nf = sscanf(txt," QGT ( %03d , %03d )",&qx, &qz);
	if (nf == 2)	GetQgtMidPoint(qx,qz,orgp);
  return;
}
//------------------------------------------------------------
//  Set Aircraft orientation in radian and degre
//  This is to factorize reference to orientation in degres
//  The global rotation matrix is computed at this stage
//  NOTE:  The Rotation matrix is used in several part of the simulation
//  A) To rotate the 3D body in external view
//  B) To compute the cockpit camera orientation.  Cockpit camera can be
//     represented as part of the aircarft where the camera position is the 
//     pilot head position and the Up and LookUp vectors are 2 others points
//  C) To compute landing gear axis position to check the wheel position
//     above ground
//  The order is very important and should not be changed:
//  1-Heading around Z
//  2-Pitch around   X
//  3-Bank  around   Y
//  NOTE:  Any change in orientation should use this function
//         in order to get accurate degres
//------------------------------------------------------------
void CWorldObject::SetObjectOrientation(SVector v)
{ iang = v;
  dang.x = RadToDeg(iang.x);
  dang.y = RadToDeg(iang.y);
  dang.z = RadToDeg(iang.z);
  //----Save position at global level ----------------
	if (IsUserPlan())
	{	globals->iang = iang;
		globals->dang = dang;
	}
  //--- Load openGL rotation matrix -----------------
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  //---Just change handiness --------------------------
  glRotated ( dang.z, 0, 0, 1);      // Heading around Z
  glRotated ( dang.x, 1, 0, 0);      // Pitch   around X
  glRotated ( dang.y, 0, 1, 0);      // Bank    around Y
  glGetDoublev(GL_MODELVIEW_MATRIX,rotM);
  glPopMatrix();
  return;
}
//------------------------------------------------------------
//	Add rotation vector in degres
//  Update physical model too
//	Function dedicated to Slew manager and user aircraft
//------------------------------------------------------------
void CWorldObject::AddOrientationInDegres(SVector &v)
{ if (globals->aPROF.Has(PROF_EDITOR))	return;
	double x = DegToRad(v.x);
  double y = DegToRad(v.y);
  double z = DegToRad(v.z);
	
  iang.x   = WrapPiPi(x + iang.x);
  iang.y   = WrapPiPi(y + iang.y);
  iang.z   = WrapPiPi(z + iang.z);
  SetObjectOrientation(iang);
  SetPhysicalOrientation(iang);
  return;
}
//------------------------------------------------------------
//	For gauges that work in left hand coordinates and degres
//------------------------------------------------------------
void	CWorldObject::GetRRtoLDOrientation (SVector *vec)
{	vec->x	= -dang.x;
	vec->y	= -dang.z;
	vec->z	= -dang.y;
	return;
}
//------------------------------------------------------------
//	Get magnetic direction
//------------------------------------------------------------
float	CWorldObject::GetMagneticDirection()
{	float zdir	= -dang.z;
  //---work in aircraft local coordinate Left hand in degre -----
  return Wrap360(zdir - globals->magDEV);
}
//------------------------------------------------------------
//	From gauges that work in right hand coordinate and degres
//-------------------------------------------------------------
void	CWorldObject::SetLDtoRROrientation (SVector *vec)
{	iang.x	= -(vec->x * DEG2RAD);
	iang.y	= -(vec->z * DEG2RAD);
	iang.z	= -(vec->y * DEG2RAD);
  dang.x  = -vec->x;
  dang.y  = -vec->y;
  dang.z  = -vec->z;
	//-----------------------------------
	if (IsUserPlan())
	{	globals->iang = iang;
		globals->dang = dang;
	}
	return;
}
//----------------------------------------------------------------------------
//	Set damage if needed
//----------------------------------------------------------------------------
void CWorldObject::CrashEvent(DAMAGE_MSG *msg)
{ Tag          sbf  = 0;
  int         prio  = damM.Severity;
  if (msg->Severity >= prio)    damM = *msg;
  prio              = damM.Severity;
  CFuiTextPopup  *note = globals->fui->GetCrashNote();
  sbf = damM.snd;
  //---- Check for warning advise ------------------------
  if (1 == prio)
  { note->OrangeBack();
    note->SetText(damM.msg);
    note->SetActive();
  }
  //---- This is a red crash -----------------------------
  if (2 <= prio)
  { note->RedBack();
    State = VEH_CRSH;
    HereWeCrash();
    note->SetText(damM.msg);
    note->SetActive();
  }
  if (0 == sbf)           return;
  //---- Play corresponding sound ------------------------
  CAudioManager *snd = globals->snd;
  CSoundBUF     *buf = snd->GetSoundBUF(sbf);
  sound = snd->Play(buf);
  return;
}
//----------------------------------------------------------------------------
//	Damage Event
//----------------------------------------------------------------------------
void CWorldObject::DamageEvent(DAMAGE_MSG *msg)
{ CAudioManager *snd = globals->snd;
  switch (State)
{ //--- Initial state: ignore ---------------- 
  case VEH_INIT:
    return;
  //---- Normal mode: process event ----------
  case VEH_OPER:
    CrashEvent(msg);
    return;
  //---- Crashing: ignore --------------------
  case VEH_CRSH:
    if (sound) return;
    State = VEH_INOP; 
    return;
  //---- Crashed:  ignore --------------------
  case VEH_INOP:
    return;
  }
  return;
}
//----------------------------------------------------------------------------
//	Reset carsh data
//----------------------------------------------------------------------------
void CWorldObject::ResetCrash(char p)
{ for (U_INT k=0; k<damL.size(); k++) delete damL[k];
	damL.clear();
	//--- Reset global profile ---------------------
	return;
}
//-------------------------------------------------------------
void CWorldObject::Print (FILE *f)
{ char s[256];
  FormatPosition (geop, s);
  fprintf (f, "Position      : %s %f' MSL\n", s, geop.alt);
  fprintf (f, "Orientation   : %f %f %f\n", iang.x, iang.y, iang.z);
}

//========================================================================
//    CSimulated Object
//		user basic simulation methods
//		like ground attraction
//		and wheels on ground flag
//========================================================================
CSimulatedObject::CSimulatedObject (void)
: CWorldObject()
{ type = TYPE_FLY_SIMULATEDOBJECT;
 *nfoFilename = 0;
 *fplname     = 0;
  caging_fixed_sped		= false;
  caging_fixed_pitch	= false;
  caging_fixed_roll		= false;
  caging_fixed_alt		= false;
  caging_fixed_wings	= false;
}
//-----------------------------------------------------------------
//	Destructor 
//-----------------------------------------------------------------
CSimulatedObject::~CSimulatedObject (void)
{ *nfoFilename = 0;
}
//-----------------------------------------------------------------
//	Read parameters 
//-----------------------------------------------------------------
int CSimulatedObject::Read (SStream *stream, Tag tag)
{ switch (tag) {
    case '_NFO':
      // Read filename of vehicle information (NFO) file
      ReadString (nfoFilename, sizeof (nfoFilename), stream);
      return TAG_READ;
		case 'fpln':
			ReadString(fplname,sizeof(fplname),stream);
			return TAG_READ;
  }

    // Allow parent class to process the tag
  return CWorldObject::Read (stream, tag);
}
//-----------------------------------------------------------------
//	All parameters are read
//-----------------------------------------------------------------
void  CSimulatedObject::ReadFinished (void)
{
  if (0 == *nfoFilename)  gtfo("NO NFO file, so no Aircraft");
  MEMORY_LEAK_MARKER ("CVehicleInfo")
  MEMORY_LEAK_MARKER ("CVehicleInfo")

  return;
}
//==================================================================================
// CVehicleObject
//	NOTE:  Type must be set by lower derived class
//===================================================================================
CVehicleObject::CVehicleObject (void)
: CSimulatedObject ()
{ upd  =  0;
  cur   = 0;
  prv   = 1;
  nEng  = 0;
	engR	= 0;
	sreg	= 0;
	//--- Clear subsystems ----------------------------------
	sVSI	= 0;
	//--- Clear Radio components ----------------------------
	GPSR	= 0;
	busR	= 0;
	mRAD	= 0;
	aPIL	= 0;
	//--- Init parent vehicle --------------------------------
	phy.SetVEH(this);
	svh.SetVEH(this);
	lod.SetVEH(this);
	amp.SetVEH(this);
	gas.SetVEH(this);
	wgh.SetVEH(this);
	wng.SetVEH(this);
	eng.SetVEH(this);
	mix.SetVEH(this);
	pit.SetVEH(this);
	pss.SetVEH(this);
	whl.SetVEH(this);
	vld.SetVEH(this);
	elt.SetVEH(this);
	
  //--------------------------------------------------------
  wNbr  = wBrk = 0;
	wTyp	= 1;
  //---- Initialize user vehicle subclasses ----------------
  swd		= 0;
  hst		= 0;
  log		= 0;
	//-------------------------------------------------------
	RazDifBrake();
  main_wing_incid = 0.0;                ///< stocking the main wing incidence value DEG
  main_wing_aoa_min = 0.0f;             ///< stocking AoA min RAD
  main_wing_aoa_max = 0.0f;             ///< stocking AoA max RAD
  kias	= 0;
	ktas	= 0;
}
//------------------------------------------------------------------------
//  Initialisation specific to user vehicle
//------------------------------------------------------------------------
void CVehicleObject::InitUserVehicle()
{	if (!IsUserPlan())	return;
	//--- This is a user plane ---------------------------------
  SetOPT(VEH_D_CRASH);
	globals->jsm->SetNbEngines(this,nEng);
	//--- Create a log file if needed --------------------------
	int	prm = 0;
  GetIniVar ("Logs", "logAirplaneObject", &prm);
  if (prm) {
    log = new CLogFile ("logs/AirplaneObject.txt", "w");
    if (log) log->Write ("CAirplane data log\n");
  }
	//--- Check for update in slew mode ------------------------
	prm = 0;
	GetIniVar ("Sim", "UpdateInSlewMode", &prm);
  upd  = prm;
	//----Init aero model drawing ---------------------------
  draw_aero = 8;
  GetIniFloat ("Graphics", "drawAeromodel", &draw_aero);
	//--- Check for no aircraft --------------------------------
	int  nop = 0;
  GetIniVar("Sim", "NoAircraft", &nop);
  if (nop)
	{	globals->noEXT++;									// No external aircraft
		globals->noINT++;									// No internal aircraft
		globals->Disp.Lock(PRIO_UPLANE);
	}
	//--- Check for fake engines -------------------------------
  int val = HAS_FAKE_ENG; // 0;
  GetIniVar ("PHYSICS", "hasFakeEngine", &val);
  has_fake_engine_thrust = val ? true : false;
	//--- init position in slew mode ---------------------------
  prm = 0;
  GetIniVar ("Sim", "initInSlewMode", &prm);
  globals->slw->StateAs(prm);
	//--- Init wind effect -------------------------------------
	GetIniVar ("PHYSICS", "windEffect", &wind_effect);
  prm = 0;
  GetIniVar ("PHYSICS", "turbulenceEffect", &prm);
  if (prm) globals->evnOpt.Set(EVN_RAND_TURBULENCE);
	//--- CAGING ----------------------------------
  int   tmp_val		= 0;
  float tmp_fval	= 0.0f; 
  GetIniVar ("CAGING", "fixedSpeed", &tmp_val);
  caging_fixed_sped = (tmp_val != 0);
	caging_fixed_sped_fval = 0;
  GetIniFloat ("CAGING", "fixedSpeedVal", &caging_fixed_sped_fval);
  //--- fixed Altitude -------------------------
  GetIniVar ("CAGING", "fixedAlt", &tmp_val);
  caging_fixed_alt = (tmp_val != 0);
  //--- disconnected wing ----------------------
  GetIniVar ("CAGING", "disconnectWings", &tmp_val);
  caging_fixed_wings = (!tmp_val);
  //--- fixed pitch ----------------------------
  GetIniVar  ("CAGING", "fixedPitch", &tmp_val);
  caging_fixed_pitch = (tmp_val != 0);
	caging_fixed_pitch_dval = 0;
  GetIniFloat("CAGING", "fixedPitchVal", &tmp_fval);
  caging_fixed_pitch_dval =  double(tmp_fval);
	//--- fixed roll -----------------------------
  GetIniVar ("CAGING", "fixedRoll", &tmp_val);
  caging_fixed_roll = (tmp_val != 0);
	caging_fixed_roll_dval = 0;
  GetIniFloat ("CAGING", "fixedRollVal", &tmp_fval);
  caging_fixed_roll_dval = double(tmp_fval);
	return;
}
//------------------------------------------------------------------------
//  Back to Initial state
//------------------------------------------------------------------------
void CVehicleObject::InitState()
{	State = VEH_INIT;
	eng.CutAllEngines();
	amp.vpil->Stop(1);
	amp.fpln->StopPlan();
	return;
}
//------------------------------------------------------------------------
//  Store NFO file name
//------------------------------------------------------------------------
void CVehicleObject::StoreNFO(char *nfo)
{ strncpy(nfoFilename,nfo,64);
  nfoFilename[63] = 0;
	//nfo = new CVehicleInfo (nfoFilename);
  return;
}
//------------------------------------------------------------------------
//	Destroy vehicle object
//------------------------------------------------------------------------
CVehicleObject::~CVehicleObject (void)
{
	//---Close any open window related to aircraft ------------
  if (globals->wfl) globals->wfl->Close();      // Fuel load
  if (globals->wld) globals->wld->Close();      // Load weight
  if (globals->wpb) globals->wpb->Close();      // Window probe
	if (IsUserPlan())	globals->rdb.Register(0);
  if (IsUserPlan()) globals->inside   =  0;
	//-----------------------------------------------------------
 *nfoFilename = 0;
	SAFE_DELETE(log);
  SAFE_DELETE (swd);
  SAFE_DELETE (hst);
  //---Clear sound objects ----------------------------------
  sounds.clear();
}
//------------------------------------------------------------------------
//	Read all parameters
//------------------------------------------------------------------------
int CVehicleObject::Read (SStream *stream, Tag tag)
{ int rc = TAG_IGNORED;
  //MEMORY_LEAK_MARKER ("readvehi")
  switch (tag) {
  case 'user':
    // No arguments, this tag indicates that this is the user vehicle
    return TAG_READ;
  case 'engn':
    // No arguments, this tag indicates that the engine is on
    rc = TAG_READ;
    break;
  case 'vmod':
    {// View mode, references one of the pre-defined cameras
      char s[256];
      ReadString (s, sizeof(s), stream);
      rc = TAG_READ;      // Ignored
    }
    break;
  case 'bvel':
    ReadVector (&bodyVelocity, stream);
    DEBUGLOG ("CVehicleObject::Read <bvel> (%.2f %.2f %.2f)", bodyVelocity.x, bodyVelocity.y, bodyVelocity.z);
    rc = TAG_READ;
    break;
  case 'bang':
    ReadVector (&bodyAngularVelocity, stream);
    DEBUGLOG ("CVehicleObject::Read <bang> (%.2f %.2f %.2f)", bodyAngularVelocity.x, bodyAngularVelocity.y, bodyAngularVelocity.z);
    rc = TAG_READ;
    break;
  default:
    // Allow parent class to process the tag
    rc = CSimulatedObject::Read (stream, tag);
  }
  //MEMORY_LEAK_MARKER ("readvehi")
  return rc;
}
//-----------------------------------------------------------------------------------
//  Read Wheel parameters
//-----------------------------------------------------------------------------------
void CVehicleObject::ReadParameters(CStreamObject *obj,char *fn)
{ SStream s(obj,"WORLD",fn);
}
//-----------------------------------------------------------------------------------
//  All parameters are read
//-----------------------------------------------------------------------------------
void CVehicleObject::ReadFinished (void)
{ 
  // Call ReadFinished() method of parent
  CWorldObject::ReadFinished ();
  //MEMORY_LEAK_MARKER ("readfnvehi")
	if(IsUserPlan())	globals->rdb.Register(this);
  // If NFO file was specified, instantiate vehicle info member
  // JS: No NFO => no aircratf. Just stop
  if (0 == *nfoFilename)  gtfo("NO NFO file, so no Aircraft");
	//--- Process NFO file ----------------
  nfo.Init(nfoFilename);
	//---- Read panel Cameras -------------
	globals->ccm->ReadPanelCamera(this,nfo.GetCAM());
  //--- Read Level of Detail models.  Must be loaded first
  lod.Init(nfo.GetLOD(),type);
	//--- Read PHY file -------------------
  phy.Init(nfo.GetPHY());
	//--- process SVH ---------------------
  svh.Init(nfo.GetSVH(), &wgh);
  //--- Read Aerodynamic Model ----------
	wng.Init(nfo.GetWNG());
  //--- Read Pitot/Static Systems -------
  pss.Init(nfo.GetPSS());
  //--- Read Ground Suspension  ---------
	whl.Init(&wgh,nfo.GetWHL());
  //--- Read Variable Loadouts
  vld.Init(&wgh,nfo.GetVLD());
	//--- Read External Lights
  elt.Init(nfo.GetELT());
  //--- Read Engine Manager
  eng.Init(nfo.GetENG());
  /// \todo Why are AMP and GAS files dependent upon ENG?  Particularly
  //        for the case of gliders, an AMP may exist without an ENG
  //--- Read Fuel System ------
  gas.Init(nfo.GetGAS(), &eng, &wgh);
  //---Read Electrical Subystems. --------
  amp.Init(nfo.GetAMP(), &eng);				
  //--- Read Cockpit Manager -------------
  pit.Init(nfo.GetPIT());
  //--- Read Control Mixer ---------------
  mix.Init(nfo.GetMIX());
	//---Add various parameters ---------------------------------------
  nEng  = eng.HowMany();
	//--- Read CheckList ----------------------------------------------
	ckl.Init(this,svh.GetTailNumber());
  //--  Initialisations (after all objects creation) ----------------
  wgh.Init ();
	//--- gear connexion to rudder ------------------------------------
	GearConnector(wTyp);
  //--- Add drawing position as external feature ---------------------
	if (IsUserPlan())	{											// MARK TEST
		CDrawPosition *upos = new CDrawPosition(this);
		amp.AddExternal(upos,0);						
	}
  //-- Add vehicle smoke as external subsystem ----------------------
	if (IsUserPlan())	{											// MARK TTEST
		CVehicleSmoke *usmk = new CVehicleSmoke(this);
		amp.AddExternal(usmk,0);							
	}
	
}

// Read aircraft history file ---------------------------------------
//
// Read aircraft history file
// JS to LC: I removed this section. it causes many crashes because
//    design is constantly in evolution
//    History file should be implemented last
//==========================================================================


//-----------------------------------------------------------------------------
//	JSDEV* Trace Message after preparation
//-----------------------------------------------------------------------------
void CVehicleObject::TraceMsgPrepa (SMessage *msg)
{
  CStreamObject *snd = (CSubsystem *)msg->sender;
	CStreamObject *rcv = (CSubsystem *)msg->receiver;
	if ((!snd) || (!rcv))	return;
  TRACE ("Prepare Msg:  FROM %25s %s TO %25s %s %s %s",
    snd->GetClassName(),snd->GetIdString(),rcv->GetClassName(), rcv->GetIdString(),
    TagToString(msg->id), TagToString(msg->user.u.datatag));
	return;
}

//----------------------------------------------------------------------------
//	JSDEV* Prepare all messages related cockpit and other part
//----------------------------------------------------------------------------
void CVehicleObject::PrepareMsg (void)
{	pit.PrepareMsg(this);				// Prepare panel gauges
  ckl.PrepareMsg(this);				// Check list messages
	mix.PrepareMixers();				// Init all mixers chn with controller pointer
	return;	
}
//----------------------------------------------------------------------------
//  Draw as user external vehicle only if
//  -Camera is external
//  -Profile allows aircraft
//----------------------------------------------------------------------------
void CVehicleObject::DrawExternal(void)
{	elt.DrawSpotLights();					// Always draw projectors
	if (globals->noEXT)                       return;
  //// Draw all externally visible objects associated with the vehicle
  lod.Draw (BODY_TRANSFORM);
  return;
}
//----------------------------------------------------------------------------
//  Draw as animated aircraft
//----------------------------------------------------------------------------
void CVehicleObject::DrawAnimated()
{	if (globals->noEXT)                       return;
	glPushMatrix();
	glTranslated(rpos.x,rpos.y,rpos.z);
	lod.Draw (BODY_TRANSFORM);
	glPopMatrix();
	return;
}
//----------------------------------------------------------------------------
//  Draw outside lights (spot and emitting lights)
//----------------------------------------------------------------------------
void CVehicleObject::DrawOutsideLights()
{	elt.DrawOmniLights();
	//elt.DrawSpotLights();
	return;
}
//----------------------------------------------------------------------------
//  Draw external feature
//  Let the feature decide if it should draw or not
//----------------------------------------------------------------------------
void CVehicleObject::DrawExternalFeatures()
{ if (globals->noEXT)                       return;
  amp.DrawExternal();
  DrawAeromodelData ();
  return;
}
//----------------------------------------------------------------------------
//  Draw internal if camera cockpit is selected
//  -Camera cockpit is the current camera
//  -Profile allows aircraft
//----------------------------------------------------------------------------
void CVehicleObject::DrawInside(CCamera *cam)
{ if (globals->noINT)           return;
	pit.Draw();
  return;
}
//---------------------------------------------------------------------------
//  Draw areomodel Data for wing
//  -Profile allows aircraft
//---------------------------------------------------------------------------
void CVehicleObject::DrawAeromodelData (void)
{ // Draw all externally visible objects associated with the vehicle
  if (HasOPT(VEH_DW_AERO)) wng.DrawAerodelData (draw_aero);
}
//----------------------------------------------------------------------
//  Print object parameters
//----------------------------------------------------------------------
void CVehicleObject::Print (FILE *f)
{
}
//-----------------------------------------------------------------------
//  Timeslice all features of vehicle
//------------------------------------------------------------------------
void CVehicleObject::Update (float dT,U_INT FrNo) 
{ //--- First update AGL from terrain ------------------------------
	Spot.lat	= geop.lat;
	Spot.lon  = geop.lon;
	Spot.agl	= geop.alt - globals->tcm->GetGroundAt(Spot);
	//----------------------------------------------------------------
  lod.TimeSlice(dT);               // Animate parts
  globals->rdb.TimeSlice(dT);

  // JS to LC:  A parameter in the [Sim] section of FlyLegacy.ini decides if the aircraft 
  // and systems are
  // updated during slew mode:  UpdateInSlewMode=1
  // By default the systems are not updated.
  //
  if (globals->slw->IsEnabled ())   if (upd == 0) return;
  // precalculate kias once, allowing minor CPU waste in multicalling
  ComputeAirspeed();

  //! Needs a framerate of at least 40 to work properly
  //! and the timeslice must be equal size that is Simulate()
  //! should be called every 0.025
  Simulate(dT,FrNo);     ///< actually calls CAirplane::Simulate or any other typed vehicle 
}
//----------------------------------------------------------------------
// After simulation update
//----------------------------------------------------------------------
void CVehicleObject::UpdateData(float dT)
{;}
//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void CVehicleObject::Simulate (float dT,U_INT FrNo)
{	// Compute altitude above ground --------------
  // Timeslice each separate feature that is part
  // of the vehicle
  // Timeslice electrical subsystems
  amp.Timeslice (dT,FrNo);
  // Timeslice gas subsystems
  gas.Timeslice (dT,FrNo);
  // Timeslice eng subsystems
  eng.Timeslice (dT,FrNo);
  // Timeslice control mixers
  mix.Timeslice (dT,FrNo);
  // Timeslice whl manager
  whl.Timeslice (dT);
  // Timeslice external lights
  elt.Timeslice (dT);
  // Timeslice wgh manager 
  wgh.Timeslice (dT);
  // Timeslice wings effects and pitot action // 
  wng.Timeslice (dT);
  pss.Timeslice (dT);
  // timeslice wind effect on aircraft
  if (wind_effect) GetAircraftWindEffect ();
  return;
}
///====================================================================
/// AirSpeed in m/s and LH
///====================================================================
CVector* CVehicleObject::GetAirspeed () 
{ // returns body airspeed
  airspeed   = vb[cur];
  return &airspeed;
}

///===================================================================
/// Body relative AirSpeed in m/s and LH
///===================================================================
CVector* CVehicleObject::GetRelativeBodyAirspeed (void) 
{ airspeed2.x = (vb[cur].x - w_dir_for_body.x); //
  airspeed2.y = 0; //
  airspeed2.z = (vb[cur].z * cos (GetOrientation ().x)) - w_dir_for_body.z; // LH
  return &(airspeed2);
}
//--------------------------------------------------------------------
// Compute true airspeed
//--------------------------------------------------------------------
void CVehicleObject::GetTAS (double &spd) // TAS in forward ft/s
{
  /// returns true airspeed in f/sec
  spd = 0.0;
  double pt = pss._total_pressure_node;                                   // INHG_TO_PSF;
  double p  = globals->atm->GetPressureSlugsFtSec (); // INHG_TO_PSF;
  double r  = globals->atm->GetDensitySlugsFt3 ();    //
  double q  = ( pt - p );  // dynamic pressure
  // Now, reverse the equation (normalize dynamic pressure to
  // avoid "nan" results from sqrt)
  if ( q < 0.0 ) { q = 0.0; }
  // Publish the indicated airspeed
  spd = sqrt ((2.0 * q) / r);
	return;
}
//--------------------------------------------------------------------
// Compute airspeed
//--------------------------------------------------------------------
void CVehicleObject::ComputeAirspeed () // KIAS in knts
{
  /// returns indicated airspeed in knts
  double pt = pss._total_pressure_node;                                  // INHG_TO_PSF;
  double p  = globals->atm->GetPressureSlugsFtSec (); // INHG_TO_PSF;
  double r  = globals->atm->GetDensitySlugsFt3 ();    //
  double q  = ( pt - p );  // dynamic pressure
  // Now, reverse the equation (normalize dynamic pressure to
  // avoid "nan" results from sqrt)
  if ( q < 0.0 ) { q = 0.0; }
  // Publish the true airspeed
  double spd = sqrt ((2.0 * q) / r);
	spd        = FpsToKt (spd);					// In knot
	ktas			 = spd;										// True airspeed
  // correction for alt KTAS==>KIAS		
  kias			 = spd / (1.0 + (geop.alt * 2e-5));
	return;
}
//---------------------------------------------------------------------------------
//  Get overall model extension in vector
//---------------------------------------------------------------------------------
void CVehicleObject::OverallExtension(SVector &v)
{ CAnimatedModel *lod = GetLOD();
  if (0 == lod)     return;
  CModelACM      *mod = lod->GetDayModel();
	if (mod)	      mod->GetExtension(v);
	return;
}
//---------------------------------------------------------------------------------
//  Add item to plot menu
//---------------------------------------------------------------------------------
int  CVehicleObject::AddToPlotMenu(char **menu, int k)
{ int p = eng.AddToPlotMenu(menu,plotPM,k);
  return p;
}

//---------------------------------------------------------------------------------
//  Request to plot some data
//---------------------------------------------------------------------------------
void  CVehicleObject::PlotParameters(PLOT_PP *pp,Tag No)
{ eng.PlotParameters(pp,plotPM[No].iden, plotPM[No].type);
  return;
}

//---------------------------------------------------------------------------------
//  GetAircraftWindEffect
//---------------------------------------------------------------------------------
void  CVehicleObject::GetAircraftWindEffect (void)
{ 
  // calc wind speed relative to the aircraft body
  // used in COpalObject::Simulate (...)
  // used in CVehicleObject::GetRelativeBodyAirspeed (void)
  double w_spd   = static_cast<double>(globals->wtm->GetWindMPS());
  double w_angle = DegToRad (static_cast<double>(globals->wtm->WindFrom ()));
  CRotationMatrixHPB matx;                                            // LH
  CVector wind_angle (sin (w_angle), 0.0, cos (w_angle));             // LH
  wind_angle.Times (w_spd);
  SVector or_m = {0.0, 0.0, 0.0};                                     // 
  or_m.y = -GetOrientation ().z; // + is left													// RH to LH
  matx.Setup (or_m);                                                  // LH
  matx.ParentToChild (w_dir_for_body, wind_angle);                    // LH
  //
}

//===========================END OF FILE ==========================================
