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
#include "../Include/FlyLegacy.h"
#include "../Include/WorldObjects.h"
#include "../Include/Utility.h"
#include "../Include/Fui.h"
#include "../Include/FuiParts.h"
#include "../Include/FuiProbe.h"
#include "../Include/FuiUser.h"
#include "../Include/FuiPlane.h"
#include "../Include/Joysticks.h"     //  for CUFOObject 
#include "../Include/MagneticModel.h" // iang correction from .SIT file ln 161
#include "../Include/Atmosphere.h"    // CVehicleObject::GetIAS (double &spd)
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
  geop		= orgp = globals->geop;
  iang		= globals->iang;
  dang		= globals->dang;
  damM.Severity		= 0;
	damM.msg				= 0;
	phyMod	= 0;
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
    break;

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
      SetOrientation(angu);
      rc = TAG_READ;
    }
    break;

  default:
    // This is the end of the line...if the tag is not recognized then
    // generate a warning
    WARNINGLOG ("CWorldObject::Read : Unrecognized tag <%s> in %s",
      TagToString(tag), stream->filename);
  }
  //MEMORY_LEAK_MARKER ("readworld")

  return rc;
}
//-------------------------------------------------------------------------
//  All parameters are read
//-------------------------------------------------------------------------
void  CWorldObject::ReadFinished (void)
{//---- Receive position and orientation from globals -----
  SetPosition(orgp);
  SetOrientation(globals->iang);

  if (!is_ufo_object)
  { SetPhysicalOrientation (iang);
    ResetSpeeds ();
  }
  return;
}



//--------------------------------------------------------------
//  Set aircraft position
//--------------------------------------------------------------
void CWorldObject::SetPosition (SPosition pos)
{ // Clamp altitude to 100K
  double altClamp = globals->aMax;;
  if (pos.alt > altClamp) pos.alt = altClamp;
  // Clamp latitude to globe tile maximum latitude
  double latClamp = LastLatitude();
  if (pos.lat > +latClamp)  pos.lat = +latClamp;
  if (pos.lat < -latClamp)  pos.lat = -latClamp;
  if (_isnan(pos.lat))  return;
  geop          = pos;
  if (!IsAirplane())    return;
  //----Save position at global level ----------------
  globals->geop = pos;
  return;
}
//--------------------------------------------------------------
//  Set aircraft altitude
//--------------------------------------------------------------
void CWorldObject::SetAltitude(double alt)
{ // Clamp altitude to 100K
  double altClamp = 1.0E+5;
  if (alt > altClamp) alt = altClamp;
  geop.alt          = alt;
  if (!IsAirplane())    return;
  //----Save position at global level ----------------
  globals->geop.alt = alt;
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
void CWorldObject::SetOrientation(SVector v)
{ iang = v;
  dang.x = RadToDeg(iang.x);
  dang.y = RadToDeg(iang.y);
  dang.z = RadToDeg(iang.z);
  //----Save position at global level ----------------
  globals->iang = iang;
  globals->dang = dang;
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
//------------------------------------------------------------
void CWorldObject::AddOrientationInDegres(SVector &v)
{ double x = DegToRad(v.x);
  double y = DegToRad(v.y);
  double z = DegToRad(v.z);
	
  iang.x   = WrapPiPi(x + iang.x);
  iang.y   = WrapPiPi(y + iang.y);
  iang.z   = WrapPiPi(z + iang.z);
  SetOrientation(iang);
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
  return Norme360(zdir - globals->magDEV);
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
  globals->iang = iang;
  globals->dang = dang;
	return;
}

//-------------------------------------------------------------
void CWorldObject::Print (FILE *f)
{
  char s[256];
  FormatPosition (geop, s);
  fprintf (f, "Position      : %s %f' MSL\n", s, geop.alt);
  fprintf (f, "Orientation   : %f %f %f\n", iang.x, iang.y, iang.z);
}

//========================================================================
//    CSimulated Object
//  user basic simulation methods
//  like ground attraction
//  and wheels on ground flag
//========================================================================
CSimulatedObject::CSimulatedObject (void)
: CWorldObject()
{
  type = TYPE_FLY_SIMULATEDOBJECT;
  // init position in slew mode ?
  int i = 0;
  GetIniVar ("Sim", "initInSlewMode", &i);
  globals->slw->StateAs(i);

  nfo = NULL; lod = NULL;
  strcpy (nfoFilename, "");
}

CSimulatedObject::~CSimulatedObject (void)
{
  strcpy (nfoFilename, "");
  SAFE_DELETE (nfo);
  SAFE_DELETE (lod);
}
//------------------------------------------------------------
//  Return terrain type
//------------------------------------------------------------
ETerrainType CSimulatedObject::GetTerrainType (void)
{ return (ETerrainType)globals->tcm->GetGroundType();
}

int CSimulatedObject::Read (SStream *stream, Tag tag)
{ switch (tag) {
    case '_NFO':
      // Read filename of vehicle information (NFO) file
      ReadString (nfoFilename, sizeof (nfoFilename), stream);
      return TAG_READ;
  }

    // Allow parent class to process the tag
  return CWorldObject::Read (stream, tag);
}

void  CSimulatedObject::ReadFinished (void)
{
  if (0 == *nfoFilename)  gtfo("NO NFO file, so no Aircraft");
  MEMORY_LEAK_MARKER ("CVehicleInfo")
  nfo = new CVehicleInfo (nfoFilename);
  MEMORY_LEAK_MARKER ("CVehicleInfo")

  // Read Level of Detail models.  Must be loaded first
  if (*nfo->GetLOD ()) lod = new CAnimatedModel (0,nfo->GetLOD (),TYPE_FLY_SIMULATEDOBJECT);
  return;
}

//------------------------------------------------------------------ 
//    TimeSlice: Update everything
//------------------------------------------------------------------
void CSimulatedObject::Timeslice (float dT,U_INT FrNo)
{
}
//------------------------------------------------------------------ 
//    Draw external parts
//------------------------------------------------------------------

void CSimulatedObject::DrawExternal(void)
{
  // Draw all externally visible objects associated with this object
  if (lod) {
    lod->Draw (BODY_TRANSFORM);       // 0 = sobj from SIT file                 
  }
  return;
}

//========================================================================
//    CDLLSimulated Object
//========================================================================
CDLLSimulatedObject::CDLLSimulatedObject (void)
: CSimulatedObject ()
{ SetType(TYPE_FLY_SIMULATEDOBJECT);
  nfo = NULL;
  lod = NULL;
  draw_flag = false;
  sim_objects_active = false;
  strcpy (nfoFilename, "");
}
//------------------------------------------------------------------------
//	Destroy object
//------------------------------------------------------------------------
CDLLSimulatedObject::~CDLLSimulatedObject (void)
{
  strcpy (nfoFilename, "");
  SAFE_DELETE (nfo);
  SAFE_DELETE (lod);
}


//------------------------------------------------------------------ 
//    Set Orientation
//------------------------------------------------------------------
void  CDLLSimulatedObject::SetOrientation (SVector orientation)
{ // adjust orientation with magnetic declination
  CVector iang_ = orientation;
  float decl_degrees_ = 0.0f, hor_field_ = 0.0f;
  CMagneticModel::Instance().GetElements (this->GetPosition (), decl_degrees_, hor_field_);
  iang_.z += DegToRad (static_cast<double> (decl_degrees_)); 
  // if object is from simulated situation set global ori 
//  if (globals->sit->sVeh) CWorldObject::SetOrientation (orientation);
//  else
  CWorldObject::SetOrientation (iang_);
}

//------------------------------------------------------------------ 
//    TimeSlice: Update everything
//------------------------------------------------------------------
void CDLLSimulatedObject::Timeslice (float dT,U_INT FrNo)
{
  if (sim_objects_active) {
    sobj_offset = SubtractPosition (globals->geop, geop);
    // verify whether to draw object or not
    draw_flag = true;
    if (fabs (sobj_offset.x) > FEET_PER_NM) draw_flag = false;
    else
    if (fabs (sobj_offset.y) > FEET_PER_NM) draw_flag = false;
    else
    if (fabs (sobj_offset.z) > FEET_PER_NM) draw_flag = false;
  }
  Simulate (dT,FrNo);
}

void CDLLSimulatedObject::Simulate (float dT,U_INT FrNo)
{
  return;
}

void CDLLSimulatedObject::DrawExternal (void)
{
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  // Draw all externally visible objects associated with this object
  if (lod) {
    //if (globals->sit->dVeh) { // 
    //if (globals->sit->sdk_flyobject_list.fo_list.size () > 1) {
    if (sim_objects_active) {
      if (draw_flag) {
        draw_flag = false;
        globals->sit->dVeh = this;
        lod->Draw (BODY_TRANSFORM);       // 1 = simulated DLL object
        globals->sit->dVeh = NULL;
      }
    }
  }
  glMatrixMode (GL_MODELVIEW);
  glPopMatrix ();
  return;
}

//==================================================================================
// CVehicleObject
//
//static float CVehicleObject_timer = 0.0f;
//===================================================================================
CVehicleObject::CVehicleObject (void)
: CSimulatedObject ()
{ int pnl = 0;
  SetType(TYPE_FLY_VEHICLE);
  GetIniVar ("Sim", "UpdateInSlewMode", &pnl);
  upd  = pnl;
  cur   = 0;
  prv   = 1;
  nEng  = 0;
	engR	= 0;
  //----Init aero model drawing ---------------------------
  draw_aero = 8;
  GetIniFloat ("Graphics", "drawAeromodel", &draw_aero);
  //---Radio interface -------------------------------------
  rTAG[0]         = 0;
  rTAG[NAV_INDEX] = 0;
  rTAG[COM_INDEX] = 0;
  rTAG[ADF_INDEX] = 0;
  aPIL  = 0;
  //--------------------------------------------------------
  WOW_nber  = 0;
  //---- Initialize user vehicle subclasses ----------------
  nfo = NULL;
  svh = NULL;
  gas = NULL;
  wng = NULL;
  phy = NULL; // PHY file
  amp = NULL;
  pss = NULL;
  whl = NULL;
  vld = NULL;
  pit = NULL;
  cam = NULL;
  lod = NULL;
  rdo = NULL;
  elt = NULL;
  eng = NULL;
  mix = NULL;
  swd = NULL;
  wgh = NULL;
  hst = NULL;
	ckl	= NULL;
  //MEMORY_LEAK_MARKER ("nsub_rd")
  nSub= new CNullSubsystem;							// JSDEV*
  globals->rdb = new CFuiRadioBand;
  //MEMORY_LEAK_MARKER ("nsub_rd")
  //----Check for No AIrcraft in Sim section --------------
  int  NoAC = 0;
  GetIniVar("Sim", "NoAircraft", &NoAC);
  if (NoAC)
	{	globals->noEXT++;									// No external aircraft
		globals->noINT++;									// No internal aircraft
	}
  //-------------------------------------------------------
  is_ufo_object =  false;                // 
  is_opal_object = false;

  int val = HAS_FAKE_ENG; // 0;
  GetIniVar ("PHYSICS", "hasFakeEngine", &val);
  has_fake_engine_thrust = val ? true : false;
  DEBUGLOG ("hasFakeEngine = %d", val);

  main_wing_incid = 0.0;                ///< stocking the main wing incidence value DEG
  main_wing_aoa_min = 0.0f;             ///< stocking AoA min RAD
  main_wing_aoa_max = 0.0f;             ///< stocking AoA max RAD
  kias = 0.0;
}
//------------------------------------------------------------------------
//  Store NFO file name
//------------------------------------------------------------------------
void CVehicleObject::StoreNFO(char *nfo)
{ strncpy(nfoFilename,nfo,64);
  nfoFilename[63] = 0;
  return;
}
//------------------------------------------------------------------------
//	Destroy vehicle object
//------------------------------------------------------------------------
CVehicleObject::~CVehicleObject (void)
{
#ifdef _DEBUG
  DEBUGLOG ("CVehicleObject::~CVehicleObject dll=%d", globals->plugins_num);
#endif

  strcpy (nfoFilename, "");
  SAFE_DELETE (nfo);
  SAFE_DELETE (svh);
  SAFE_DELETE (gas);
  SAFE_DELETE (wng);
  SAFE_DELETE (phy); // PHY file
  SAFE_DELETE (amp);
  SAFE_DELETE (pss);
  SAFE_DELETE (whl);
  SAFE_DELETE (vld);
  SAFE_DELETE (pit);
  SAFE_DELETE (cam);
  SAFE_DELETE (lod);
  SAFE_DELETE (rdo);
  SAFE_DELETE (elt);
  SAFE_DELETE (eng);
  SAFE_DELETE (mix);
  SAFE_DELETE (swd);
  SAFE_DELETE (wgh);
  SAFE_DELETE (hst);
  SAFE_DELETE (nSub);
	SAFE_DELETE (ckl);
  //---Clear sound objects ----------------------------------
  sounds.clear();
  //---JS: Clean globals area -------------------------------
  globals->uph           = 0;
  globals->simulation    = false;
  //---Close any open window related to aircarft ------------
  if (globals->wfl) globals->wfl->Close();      // Fuel load
  if (globals->wld) globals->wld->Close();      // Load weight
  if (globals->rdb) globals->rdb->Close();      // radio band
  if (globals->wpb) globals->wpb->Close();      // Window probe
  globals->inside        =  0;
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
{ SStream s;
  strcpy (s.filename, "World/");
  strcat (s.filename, fn);
  strcpy (s.mode, "r");
  if (OpenStream (&s)) {
    ReadFrom (obj, &s);
    CloseStream (&s);
  }
}
//-----------------------------------------------------------------------------------
//  All parameters are read
//-----------------------------------------------------------------------------------
void CVehicleObject::ReadFinished (void)
{ 
  // Call ReadFinished() method of parent
  CWorldObject::ReadFinished ();
  //MEMORY_LEAK_MARKER ("readfnvehi")

  // If NFO file was specified, instantiate vehicle info member
  // JS: No NFO => no aircratf. Just stop
  if (0 == *nfoFilename)  gtfo("NO NFO file, so no Aircraft");
  //MEMORY_LEAK_MARKER ("nfo")
  nfo = new CVehicleInfo (nfoFilename);
  //MEMORY_LEAK_MARKER ("nfo")

  // Read PHY file
  // should be first in the sequence : data in it
  // stops aircraft physics adjustments readings
  // from the ini file  
  //
  // Read Level of Detail models.  Must be loaded first
  //MEMORY_LEAK_MARKER ("lod")
  if (*nfo->GetLOD()) lod = new CAnimatedModel (this,nfo->GetLOD(),type);
  //MEMORY_LEAK_MARKER ("lod")

  char *fn = nfo->GetPHY();
  //MEMORY_LEAK_MARKER ("phy")
  if (fn)    phy = new CPhysicModelAdj(this,fn);
  //MEMORY_LEAK_MARKER ("phy")
  // Create dynamics modelling classes if this is a user vehicle
  //MEMORY_LEAK_MARKER ("wgh")
  wgh = new CWeightManager (this);
  //MEMORY_LEAK_MARKER ("wgh")

  // Instantiate all user vehicle subcomponents defined in NFO file
  // Read Simulated Vehicle
  //MEMORY_LEAK_MARKER ("svh")
  if (*nfo->GetSVH()) svh = new CSimulatedVehicle (this,nfo->GetSVH(), wgh);
  //MEMORY_LEAK_MARKER ("svh")

  // Read Aerodynamic Model
  //MEMORY_LEAK_MARKER ("wng")
  if (*nfo->GetWNG()) wng = new CAerodynamicModel (this,nfo->GetWNG());
  //MEMORY_LEAK_MARKER ("wng")

  // Read Pitot/Static Systems
  //MEMORY_LEAK_MARKER ("pss")
  if (*nfo->GetPSS()) pss = new CPitotStaticSystem(this,nfo->GetPSS());
  //MEMORY_LEAK_MARKER ("pss")

  // Read Ground Suspension
  //MEMORY_LEAK_MARKER ("whl")

  if (*nfo->GetWHL()) {
    if (is_opal_object)  whl = new COpalGroundSuspension (this,nfo->GetWHL(), wgh);
    else                 whl = new CGroundSuspension (this,nfo->GetWHL(), wgh);
  }
  //MEMORY_LEAK_MARKER ("whl")

  ReadParameters(whl,nfo->GetWHL());
  // Read Variable Loadouts
  //MEMORY_LEAK_MARKER ("vld")
  if (*nfo->GetVLD()) vld = new CVariableLoadouts (this,nfo->GetVLD(), wgh);
  //MEMORY_LEAK_MARKER ("vld")

  // Read Camera Manager
  //MEMORY_LEAK_MARKER ("cam")
  if (*nfo->GetCAM()) cam  = new CCameraManager (this,nfo->GetCAM());
  //MEMORY_LEAK_MARKER ("cam")

  // Read Radio Manager
  //MEMORY_LEAK_MARKER ("rdo")
  if (*nfo->GetRDO()) rdo = new CRadioManager (nfo->GetRDO());
  //MEMORY_LEAK_MARKER ("rdo")

  // Read External Lights
  //MEMORY_LEAK_MARKER ("elt")
  if (*nfo->GetELT()) elt = new CExternalLightManager (this,nfo->GetELT());
  //MEMORY_LEAK_MARKER ("elt")

  // Read Engine Manager
  //MEMORY_LEAK_MARKER ("eng")
  if (*nfo->GetENG()) eng = new CEngineManager (this,nfo->GetENG());
  //MEMORY_LEAK_MARKER ("eng")

  /// \todo Why are AMP and GAS files dependent upon ENG?  Particularly
  //        for the case of gliders, an AMP may exist without an ENG

  if (eng) {
    // Read Fuel System
    if (*nfo->GetGAS()) gas = new CFuelSystem (this,nfo->GetGAS(), eng, wgh);
  }
  //---Read Electrical Subystems. ------------------------------- 
  if (*nfo->GetAMP()) amp = new CElectricalSystem (this,nfo->GetAMP(), eng);
  //--- Read Cockpit Manager -----------------------------------
  if (*nfo->GetPIT()) pit = new CCockpitManager (this,nfo->GetPIT());
  //--- Read Control Mixer
  if (*nfo->GetMIX()) mix = new CControlMixer (this,nfo->GetMIX());
	//--- Read CheckList ----------------------------------------------
	ckl = new PlaneCheckList(this);
	char *tail = svh->GetTailNumber();
	ckl->OpenList(tail);
  //--  Initialisations (after all the objects creation) ------------
  wgh->Init ();
  //-- Add drawing position as external feature ---------------------
  if (0 == amp)     return;
  CDrawPosition *upos = new CDrawPosition();
  amp->AddExternal(upos,0);
  //-- Add vehicle smoke as external subsystem ----------------------
  CVehicleSmoke *usmk = new CVehicleSmoke();
  amp->AddExternal(usmk,0);
  //---Add various parameters ---------------------------------------
  nEng  = eng->HowMany();
  return;
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
{	if (pit)	pit->PrepareMsg(this);				// Prepare panel gauges
	return;	
}
//----------------------------------------------------------------------------
//  Draw external vehicle only if
//  -Camera is external
//  -Profile allows aircraft
//----------------------------------------------------------------------------
void CVehicleObject::DrawExternal(void)
{ elt->DrawSpotLights();
	if (globals->noEXT)                       return;
  //// Draw all externally visible objects associated with the vehicle
  if (lod) lod->Draw (BODY_TRANSFORM);
  return;
}
//----------------------------------------------------------------------------
//  Draw outside lights (spot and emitting lights)
//----------------------------------------------------------------------------
void CVehicleObject::DrawOutsideLights()
{	elt->DrawOmniLights();
	return;
}
//----------------------------------------------------------------------------
//  Draw external feature
//  Let the feature decide if it should draw or not
//----------------------------------------------------------------------------
void CVehicleObject::DrawExternalFeatures()
{ if (globals->noEXT)                       return;
  if (amp)  amp->DrawExternal();
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
  CAnimatedModel *lod = GetLOD();
  CCameraCockpit *cmp = (CCameraCockpit*) cam;
  CPanel         *pan = cmp->GetPanel();
  //-----First draw the Spinner ---------------
  TC_4DF       ofs;
  cmp->GetXSRC(ofs);
  if (pan)        pan->GetOFS(ofs);
  if (pan)        pan->Draw(cmp);
  return;
}
//---------------------------------------------------------------------------
//  Draw areomodel Data for wing
//  -Profile allows aircraft
//---------------------------------------------------------------------------
void CVehicleObject::DrawAeromodelData (void)
{ // Draw all externally visible objects associated with the vehicle
  if (globals->vehOpt.Has(VEH_DW_AERO)  && wng ) wng->DrawAerodelData (draw_aero);
}
//---------------------------------------------------------------------------
//  Return camera according to current window
//----------------------------------------------------------------------------
CCameraManager* CVehicleObject::GetCameraManager ()
{ return cam;
}

Tag CVehicleObject::GetPanel (void)
{
  Tag rc = 0;
  if (pit) {
    rc = pit->GetPanel ();
  }
  return rc;
}
//----------------------------------------------------------------------
//  Select the current panel 
//----------------------------------------------------------------------
void CVehicleObject::SetPanel (Tag tag)
{ if (pit == 0)                 return;
  if (tag == pit->GetPanel ())  return;
  pit->SetPanel (tag);
  return;
}
//----------------------------------------------------------------------
//  Print object parameters
//----------------------------------------------------------------------
void CVehicleObject::Print (FILE *f)
{ if (cam) cam->Print(f);
  if (amp) amp->Print (f);
  if (lod) lod->Print (f);
  if (elt) elt->Print (f);
}
//-----------------------------------------------------------------------
//  Timeslice all features of vehicle
//------------------------------------------------------------------------
void CVehicleObject::Update (float dT,U_INT FrNo) 
{ //if (globals->ttr > 1) TRACE("CAnimatedModel::TimeSlice");
  lod->TimeSlice(dT);               // Animate parts
  //if (globals->ttr > 1) TRACE("CFuiRadioBand::TimeSlice");
  globals->rdb->TimeSlice(dT);

  #ifdef _DEBUG_UFO
    {	FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_UFO.txt", "a")) == NULL)
	  {
		  fprintf(fp_debug, "CVehicleObject::Timeslice\n");
		  fclose(fp_debug); 
      }}
  #endif 
  // JS to LC:  A parameter in the [Sim] section of FlyLegacy.ini decides if the aircraft 
  // and systems are
  // updated during slew mode:  UpdateInSlewMode=1
  // By default the systems are not updated.
  //
  if (globals->slw->IsEnabled ()){
     if (upd == 0) return;
  }
  // precalculate kias once, allowing minor CPU waste in multicalling
  GetKIAS (kias);

  //! Needs a framerate of at least 40 to work properly
  //! and the timeslice must be equal size that is Simulate()
  //! should be called every 0.025
  Simulate(dT,FrNo);     ///< actually calls CAirplane::Simulate or any other typed vehicle 
}
//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void CVehicleObject::Simulate (float dT,U_INT FrNo)
{
  //! first off : we timeslice each separate feature that is part
  //! of the vehicle

  // Timeslice electrical subsystems
  if (amp) amp->Timeslice (dT,FrNo);
  // Timeslice gas subsystems
  if (gas) gas->Timeslice (dT,FrNo);
  // Timeslice eng subsystems
  if (eng) eng->Timeslice (dT,FrNo);
  // Timeslice control mixers
  if (mix) mix->Timeslice (dT,FrNo);
  // Timeslice whl manager
  if (whl) whl->Timeslice (dT);
  // Timeslice external lights
  if (elt) elt->Timeslice (dT);
  // Timeslice wgh manager (!!!) it's not an NFO TimeSlice
  if (wgh) wgh->Timeslice (dT);
  // Timeslice wings effects and pitot action // 
  if (wng) wng->Timeslice (dT);
  if (pss) pss->Timeslice (dT);
  // update WOW for wheels before Simulate (dT)
  //--- update CG position ------------------
  svh->CalcNewCG_ISU (); // 
  //--- Timeslice svh stuff : 
  svh->Timeslice (dT);
  // timeslice wind effect on aircraft
  if (wind_effect) GetAircraftWindEffect ();
  return;
}
//============================================================
double CVehicleObject::GetMassInKgs (void)
{
  return (double(wgh->GetTotalMassInKgs()));
}
//=============================================================
double CVehicleObject::GetMassInLbs (void)
{
  return (double(wgh->GetTotalMassInLbs()));
}
//==============================================================
const SVector* CVehicleObject::GetMassMomentOfInertia (void)
{
  return (wgh->wb.GetMI_ISU ());
}

const SVector* CVehicleObject::GetMomentOfInertia (void)
{
  return (&tb);
}

const SVector* CVehicleObject::GetCG (void)
{
  return (svh->GetNewCG_ISU ());
}

const CVector* CVehicleObject::GetBodyVelocityVector (void)
{
  return &(vb[cur]); // m/s LH
}

const CVector* CVehicleObject::GetInertialVelocityVector (void)
{
  return &(vi[cur]); // m/s LH
}

const CVector* CVehicleObject::GetBodyAccelerationVector (void)
{
  return &(ab[0]); // m.s� LH
}

const CVector* CVehicleObject::GetInertialAccelerationVector (void)
{
  return &(ai[0]); // m.s� LH
}

double CVehicleObject::GetGroundspeed ()  // m/s in LH
{ 
  return sqrt (airspeed.x * airspeed.x + airspeed.z * airspeed.z);
}

///====================================================================
/// AirSpeed in m/s and LH
///====================================================================
const CVector* CVehicleObject::GetAirspeed (void) 
{ // returns body airspeed
  airspeed.x = vb[cur].x; //
  airspeed.y = vb[cur].y; //
  airspeed.z = vb[cur].z; //
  return &vb[cur];
}

///===================================================================
/// Body relative AirSpeed in m/s and LH
///===================================================================
const CVector* CVehicleObject::GetRelativeBodyAirspeed (void) 
{// Airspeed = groundspeed - windspeed
 // SVector w_dir_for_body_ = globals->wtm->w_dir_for_body; // LH

  airspeed2.x = (vb[cur].x - w_dir_for_body.x); //
  airspeed2.y = 0; //
  airspeed2.z = (vb[cur].z * cos (GetOrientation ().x)) - w_dir_for_body.z; // LH

/*/
#ifdef _DEBUG	
{	FILE *fp_debug;
	if(!(fp_debug = fopen("__DDEBUG_airspeed.txt", "a")) == NULL)
	{
    float ws = globals->wtm->GetWindMPS ();
    float wdir = Wrap180 (globals->wtm->WindFrom () - 180.0f);
    float cdir = GetOrientation ().z;
		fprintf(fp_debug, "%f %f %f %f %f\n", 
      airspeed2.x, airspeed2.z,
      ws, wdir, RadToDeg (cdir));
		fclose(fp_debug); 
}	}
#endif
/*/
  return &(airspeed2);
}

void CVehicleObject::GetTAS (double &spd) // TAS in forward ft/s
{
  /// returns true airspeed in f/sec
  spd = 0.0;
  if (pss)
  {
    double pt = pss->_total_pressure_node;                                   // INHG_TO_PSF;
    double p  = globals->atm->GetPressureSlugsFtSec (); // INHG_TO_PSF;
    double r  = globals->atm->GetDensitySlugsFt3 ();    //
    double q  = ( pt - p );  // dynamic pressure
    // Now, reverse the equation (normalize dynamic pressure to
    // avoid "nan" results from sqrt)
    if ( q < 0.0 ) { q = 0.0; }
    // Publish the indicated airspeed
    spd = sqrt ((2.0 * q) / r);
  }
  else
  { // approximation to be done
    spd = 0.0;
  }
}

void CVehicleObject::GetIAS (double &spd) // IAS in forward ft/s
{
  /// returns indicated airspeed in ft/sec
  spd = 0.0;
  if (pss)
  {
    double pt = pss->_total_pressure_node;                                  // INHG_TO_PSF;
    double p  = globals->atm->GetPressureSlugsFtSec (); // INHG_TO_PSF;
    double r  = globals->atm->GetDensitySlugsFt3 ();    //
    double q  = ( pt - p );  // dynamic pressure
    // Now, reverse the equation (normalize dynamic pressure to
    // avoid "nan" results from sqrt)
    if ( q < 0.0 ) { q = 0.0; }
    // Publish the indicated airspeed
    spd = sqrt ((2.0 * q) / r);
    // correction for alt KTAS==>KIAS
    spd *= 1.0 / (1.0 + (GetPosition ().alt * 2e-5));
  }
  else
  { // approximation in standard atmosphere
    spd = FpsToKt (MetresToFeet (vb[cur].z));
    // approximation until pitot is ready
    double alt = GetPosition ().alt * 1e-5;
    spd *= 1.0 - (1.4 * alt); // (PSF_TO_INHG*100) magic number for standard athmosphere
    spd = KtToFps (spd); // knts to ft/s
  }
}

void CVehicleObject::GetKIAS (double &spd) // KIAS in knts
{
  /// returns indicated airspeed in knts
  spd = 0.0;
  if (pss)
  {
    double pt = pss->_total_pressure_node;                                  // INHG_TO_PSF;
    double p  = globals->atm->GetPressureSlugsFtSec (); // INHG_TO_PSF;
    double r  = globals->atm->GetDensitySlugsFt3 ();    //
    double q  = ( pt - p );  // dynamic pressure
    // Now, reverse the equation (normalize dynamic pressure to
    // avoid "nan" results from sqrt)
    if ( q < 0.0 ) { q = 0.0; }
    // Publish the indicated airspeed
    spd = sqrt ((2.0 * q) / r);
    // correction for alt KTAS==>KIAS
    spd *= 1.0 / (1.0 + (GetPosition ().alt * 2e-5));
    spd = FpsToKt (spd);
  }
  else
  { // approximation in standard atmosphere
    spd = FpsToKt (MetresToFeet (vb[cur].z));
    // approximation until pitot is ready
    double alt = GetPosition ().alt * 1e-5;
    spd *= 1.0 - (1.4 * alt); // (PSF_TO_INHG*100) magic number for standard athmosphere
  }
}

const CVector* CVehicleObject::GetBodyAngularVelocityVector (void)
{
  return &(wb[cur]); // rad/s LH
}

const CVector* CVehicleObject::GetBodyAngularAccelerationVector (void)
{
  return &(dwb[cur]); // rad.s� LH
}

const CVector* CVehicleObject::GetInertialAngularVelocityVector (void)
{
  return &(wi[cur]); // rad/s LH
}

const CVector* CVehicleObject::GetInertialAngularAccelerationVector (void)
{
  return &(dwi[cur]); // rad.s� LH
}
//--------------------------------------------------------------------------------------
//  Propagate a CAeroControlChannel pointer to each wing object
//--------------------------------------------------------------------------------------
void CVehicleObject::SetWingChannel(CAeroControlChannel *aero)
{  if (wng) wng->SetWingChannel(aero);
}
//--------------------------------------------------------------------------------------
void CVehicleObject::SetPartKeyframe (char* part, float keyframe)
{
  if (lod) lod->SetPartKeyframe (part, keyframe);
}

void CVehicleObject::SetPartTransparent (char* part, bool ok)
{
  if (lod) lod->SetPartTransparent (part, ok);
}

//---------------------------------------------------------------------------------
//  Init deflection for steering gear
//---------------------------------------------------------------------------------
void CVehicleObject::GetGearChannel(SGearData *gdt)
{	if (0 == amp)	return;
	CAeroControl *p = amp->pRuds;
	if (0 == p)		return;
  gdt->deflect = p->Val();
  gdt->scaled  = 0;
	return;
}
//---------------------------------------------------------------------------------
//  Add item to plot menu
//---------------------------------------------------------------------------------
int  CVehicleObject::AddToPlotMenu(char **menu, int k)
{ int p = eng->AddToPlotMenu(menu,plotPM,k);
  return p;
}

//---------------------------------------------------------------------------------
//  Request to plot some data
//---------------------------------------------------------------------------------
void  CVehicleObject::PlotParameters(PLOT_PP *pp,Tag No)
{ eng->PlotParameters(pp,plotPM[No].iden, plotPM[No].type);
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
  or_m.y = -globals->sit->uVeh->GetOrientation ().z; // + is left     // RH to LH
  matx.Setup (or_m);                                                  // LH
  matx.ParentToChild (w_dir_for_body, wind_angle);                    // LH
  //
}

//===========================END OF FILE ==========================================
