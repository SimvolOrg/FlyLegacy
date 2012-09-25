/*
 * Situation.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2007 Chris Wallace
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

/*! \file Situation.cpp
 *  \brief Implements CSituation simulation situation manager class
 *
 *  The CSituation class encapsulates the entire simulation world, including
 *    the user aircraft and everything about it, terrain and scenery,
 *    etc.  The CSlewManager class is also implemented here temporarily
 *    until it is moved to its own implemenation file.
 */

/*!
      In CSituation::Read I changed the TYPE_USER_AIRCRAFT case
      from CAirplane to CUFOObject new object for developping purposes.
      We'd revert it when aerodynamics and 3d rendering will be settled
 */

#include <time.h>

#include "../Include/Globals.h"
#include "../Include/Situation.h"
#include "../Include/Ui.h"
#include "../Include/Utility.h"
#include "../Include/Subsystems.h"
#include "../Include/Gauges.h"
#include "../Include/Weather.h"
#include "../Include/MagneticModel.h"
#include "../Include/Joysticks.h"
#include "../Include/Fui.h"
#include "../Include/FuiUser.h"
#include "../Include/Sky.h"
#include "../Include/AudioManager.h"
#include "../Include/database.h"
#include "../Include/TerrainCache.h"
#include "../Include/Cloud.h"
#include "../Include/Atmosphere.h"
#include "../Include/DrawVehiclePosition.h"
#include "../Include/DrawVehicleSmoke.h"
#include "../Include/PlanDeVol.h"
#include "../Include/Triangulator.h"
#include "../Plugin/Plugin.h"
using namespace std;
//=====================================================================================
//    GLOBAL CLEANUP
//=====================================================================================
void GlobalsClean (void)
{
  return;
}
//=====================================================================================
//    Dispatcher constructor
//=====================================================================================
CDispatcher::CDispatcher()
{	for (int k=0; k < PRIO_MAX; k++)
	{	DSP_EXEC *tab = slot + k;
		tab->lock		= 0;
		tab->obj  	= 0;
		tab->exec   = 1;
		tab->draw   = 1;
	}
}
//-------------------------------------------------------------------
//	Enter executable as head of list
//-------------------------------------------------------------------
void CDispatcher::Store(CExecutable *ex, char p)
{	slot[p].obj	= ex;
	slot[p].exec	= 1;
	slot[p].draw	= 1;
}
//-------------------------------------------------------------------
//	Remove object
//-------------------------------------------------------------------
void CDispatcher::Remove(char p)
{	slot[p].obj		= 0;
	slot[p].draw	= 0;
	return;
}
//-------------------------------------------------------------------
//	Execute
//-------------------------------------------------------------------
void	CDispatcher::TimeSlice(float dT, U_INT frame)
{	for (int k=0; k<= PRIO_OTHERS; k++)
		{	CExecutable *ex = slot[k].obj;
			if (0 == ex)						continue;
			if (slot[k].lock)				continue;
			//--- Execute the list of similar objects ----------
			slot[k].obj->TimeSlice(dT,frame);
			//--- Check if last execution ----------------------
			if (slot[k].exec == 0)	return;			
		}
	return;
}
//-------------------------------------------------------------------
//	 Draw
//-------------------------------------------------------------------
void	CDispatcher::Draw(char p)
{	if (slot[p].draw == 0)			return;
	if (slot[p].obj  == 0)			return;
	slot[p].obj->Draw();
	return;
}
//-------------------------------------------------------------------
//	 Draw external
//	Draw external start from PLANE priority up to SDK included
//-------------------------------------------------------------------
void CDispatcher::DrawExternal()
{	for (int k=PRIO_UPLANE; k<= PRIO_OTHERS; k++)
		{	CExecutable *ex = slot[k].obj;
			if (0 == ex)							continue;
			if (slot[k].draw == 0)		continue;
			if (slot[k].exec == 0)		return;
			slot[k].obj->DrawExternal();			
		}
	return;
}

//==========================================================================
//  Global Group function for Slew manager 'slew'
//==========================================================================
bool KeySlewGroup(CKeyDefinition *kdf,int code)
{ if (kdf->NoPCB())   return false;
  Tag kid   = kdf->GetTag();
  U_INT key = code & 0x0000FFFF;      // Key code
  U_INT mod = (code >> 16);           // Modifier
  if (kid == 'slew')    return globals->slw->Swap();
  if (!globals->slw->IsEnabled ())  return false;
  if (!globals->pln)								return false;
  return kdf->GetCallback() (kid,key,mod);
}

//==========================================================================
//  Global functions for Slew manager 'slew'
//==========================================================================
//--- Toggle slew -----------------------------------
bool sKeySLEW(int id, int code, int mod)
{ globals->slw->Swap();
  return true;
}
//--------------------------------------------------------------------------
// increase speed in aircraft direction 'sfwd'
//--------------------------------------------------------------------------
bool sKeySFWD(int id, int code, int mod)
{ return globals->slw->MoveOnY(+1);
} 
//--------------------------------------------------------------------------
//  Decrease speed from aircraft direction 'sbkw'
//--------------------------------------------------------------------------
bool sKeySBKW(int id, int code, int mod)
{ return globals->slw->MoveOnY(-1);
}
//--------------------------------------------------------------------------
//  Head left 'slft'
//---------------------------------------------------------------------------
bool sKeySLFT (int id, int code, int mod)
{ return globals->slw->MoveOnX(+1);
}
//---------------------------------------------------------------------------
//  Head Right 'srgt'
//---------------------------------------------------------------------------
bool sKeySRGT (int id, int code, int mod)
{ return globals->slw->MoveOnX(-1); 
}
//----------------------------------------------------------------------------
// Slew bank left 'sbnl'
//----------------------------------------------------------------------------
bool sKeySBNL (int id, int code, int mod)
{ // Bank left 1° -------------------------------------
  CVector v(0,-1,0);
  globals->pln->AddOrientationInDegres(v);
  return true;
}
//----------------------------------------------------------------------------
//  slew bank right 'sbnr'
//----------------------------------------------------------------------------
bool sKeySBNR (int id, int code, int mod)
{	//--- Bank right 1° ---------------------------------
	CVector v(0,+1,0);
	globals->pln->AddOrientationInDegres(v);
	return true;
}
//----------------------------------------------------------------------------
//  Rotate left 'srtl'
//----------------------------------------------------------------------------
bool sKeySRTL (int id, int code, int mod)
{ // Left rotate 1° ----------------------------------
  CVector v(0,0,+1);
  globals->pln->AddOrientationInDegres(v);
  return true;
}
//----------------------------------------------------------------------------
//  Rotate right 'srtr'
//----------------------------------------------------------------------------
bool sKeySRTR (int id, int code, int mod)
{ // Right rotate 1° ----------------------------------
  CVector v(0,0,-1);
  globals->pln->AddOrientationInDegres(v);
  return true;
}
//----------------------------------------------------------------------------
//  Rotate Left 45° 'srl4'
//----------------------------------------------------------------------------
bool sKeySRL4 (int id, int code, int mod)
{ //Left Rotate 45 ° ----------------------------------
  CVector v(0,0,+45);
  globals->pln->AddOrientationInDegres(v);
  return true;
}
//----------------------------------------------------------------------------
//  Rotate Right 45° 'srr4'
//-----------------------------------------------------------------------------
bool sKeySRR4 (int id, int code, int mod)
{ //--- Rigth rotate 45° ------------------------------
  CVector v(0,0,-45);
  globals->pln->AddOrientationInDegres(v);
  return true;
}
//----------------------------------------------------------------------------
//  Pitch up 'sptu'
//----------------------------------------------------------------------------
bool sKeySPTU (int id, int code, int mod)
{ //pitch up 0.25° ------------------------------------
  CVector v(+0.25,0,0);
  globals->pln->AddOrientationInDegres(v);
  return true;
} 
//----------------------------------------------------------------------------
//  Picth down 'sptd'
//----------------------------------------------------------------------------
bool sKeySPTD (int id, int code, int mod)
{ //--- Pitch Down 0.25° -----------------------------
  CVector v(-0.25,0,0);
  globals->pln->AddOrientationInDegres(v);
  return true;
}
//----------------------------------------------------------------------------
//  Slew Up 'slup'
//----------------------------------------------------------------------------
bool sKeySLUP (int id, int code, int mod)
{ if (globals->aPROF.Has(PROF_TRACKE))	return true;
	globals->slw->MoveOnZ(+10.0f);
  return true;
}
//----------------------------------------------------------------------------
//  Slew down 'sldn'
//----------------------------------------------------------------------------
bool sKeySLDN (int id, int code, int mod)
{ if (globals->aPROF.Has(PROF_TRACKE))	return true;
	globals->slw->MoveOnZ(-10.0f);
  return true;
}
//----------------------------------------------------------------------------
//    Reset orientation 'sreo'
//----------------------------------------------------------------------------
bool sKeySREO(int id, int code, int mod)
{ // Get current user vehicle orientation
	if (!globals->pln)	return false;
  CVehicleObject *veh = globals->pln;
  CVector v;
  v.x = v.y = v.z = 0.0;
  veh->SetObjectOrientation (v);
  veh->SetPhysicalOrientation (v);
  return true;
}

//----------------------------------------------------------------------------
//    Stop slewing 'sstp'
//----------------------------------------------------------------------------
bool sKeySSTP(int id, int code, int mod)
{ //
  globals->slw->StopMove ();
  return true;
}

//=====================================================================================
// CSlewManager manages the user vehicle position when it is in slew mode
//=====================================================================================
CSlewManager::CSlewManager (void)
{ mveh   = 0;
  aRate = 0.0f;
  fRate = lRate = 0.0f;
  mode  = SLEW_STOP;
	//--- Bind all  keys -------------------------------------------
  BindKeys();
	//--- Enter in Dispatcher --------------------------------------
	globals->Disp.Enter(this, PRIO_SLEWMGR, DISP_EXCONT, 0);
}
//------------------------------------------------------------------------
//  Destroy resources
//------------------------------------------------------------------------
CSlewManager::~CSlewManager()
{		}
//------------------------------------------------------------------------
//  Bind Slew Keys
//------------------------------------------------------------------------
void CSlewManager::BindKeys()
{ CKeyMap *km = globals->kbd;
  km->BindGroup('slew',KeySlewGroup);  
  km->Bind ('slew', sKeySLEW, KEY_SET_ON);
  km->Bind ('sfwd', sKeySFWD, KEY_SET_ON);
  km->Bind ('sbkw', sKeySBKW, KEY_SET_ON);
  km->Bind ('slft', sKeySLFT, KEY_SET_ON);
  km->Bind ('srgt', sKeySRGT, KEY_SET_ON);
  km->Bind ('sbnl', sKeySBNL, KEY_SET_ON);
  km->Bind ('sbnr', sKeySBNR, KEY_SET_ON);
  km->Bind ('srtl', sKeySRTL, KEY_SET_ON);
  km->Bind ('srtr', sKeySRTR, KEY_SET_ON);
  km->Bind ('srl4', sKeySRL4, KEY_SET_ON);
  km->Bind ('srr4', sKeySRR4, KEY_SET_ON);
  km->Bind ('sptu', sKeySPTU, KEY_SET_ON);
  km->Bind ('sptd', sKeySPTD, KEY_SET_ON);
  km->Bind ('slup', sKeySLUP, KEY_SET_ON);
  km->Bind ('sldn', sKeySLDN, KEY_SET_ON);
  km->Bind ('sreo', sKeySREO, KEY_SET_ON);
  km->Bind ('sstp', sKeySSTP, KEY_SET_ON);
}
//------------------------------------------------------------------------
//  Disable slew mode
//------------------------------------------------------------------------
void CSlewManager::Disable (void) 
{ ZeroRate ();
  mode  = SLEW_STOP;
  return;
}
//------------------------------------------------------------------------
//  swap slew mode
//------------------------------------------------------------------------
bool CSlewManager::Swap()
{ if (mode == SLEW_RCAM)	return ZeroRate(); 
	mveh = globals->pln;
  if (0 == mveh)					return true;
  switch (mode) {
    //--- Inactive => Moving ---
    case SLEW_STOP:
      StartSlew();
      return true;
    //--- Active => Stop -------
    case SLEW_MOVE:
      StopSlew();
      return true;
    //--Levelling => ignore ----
    case SLEW_LEVL:
      return true;
  }
  return true;
}
//------------------------------------------------------------------------
//  Set slew mode
//------------------------------------------------------------------------
void CSlewManager::SetSlew()
{	grnd	= 0;
	mode	= SLEW_MOVE;
  ZeroRate();
	vopt	= 0;
	globals->pln->RazOPT(VEH_D_CRASH);
  return;
}
//------------------------------------------------------------------------
//  Start slew mode
//	NOTE:	while moving in slew mode, opal simulation is suspended
//				for all aircrafts.
//				When normal mode is resumed, the position must be set before
//				next simulation step, otherwise, opal generates a collision(!!!)
//				at this next step.
//------------------------------------------------------------------------
void CSlewManager::StartSlew()
{ grnd = mveh->WheelsAreOnGround();
  mode = SLEW_MOVE;
  ZeroRate();
  vopt = mveh->GetOPT(VEH_D_CRASH);
  mveh->RazOPT(VEH_D_CRASH);
	globals->Disp.ExecOFF(PRIO_ATMOSPHERE);
	//TRACE("STart slew at geop.alt=%.4lf",globals->geop.alt);
  return;
}
//------------------------------------------------------------------------
//  Start slew mode for rabbit camera
//------------------------------------------------------------------------
void CSlewManager::StartMode(CAMERA_CTX *ctx)
{	vopt	= globals->pln->GetOPT(VEH_D_CRASH);
	grnd	= 0;
	mode	= ctx->mode;
	flpn  = ctx->fpln;
	ZeroRate();
	return;
}
//------------------------------------------------------------------------
//  Stop slew mode
//  Check that aircraft is above ground
//------------------------------------------------------------------------
void CSlewManager::StopSlew()
{	ZeroRate();
	if (globals->aPROF.Has(PROF_EDITOR))	return;
  mveh = globals->pln;
  if (0 == mveh)												return;
  //---------------------------------------------
	SVector ori = mveh->GetOrientation();
	mveh->SetObjectOrientation(ori);
	mveh->SetPhysicalOrientation(ori);
  mveh->SetOPT(vopt);
  mode = SLEW_STOP;
	//TRACE("STOP slew at geop.alt=%.4lf",globals->geop.alt);
	//mveh->SetObjectPosition(globals->geop);
	mveh->SetGeop(globals->geop);
	globals->sit->BackToSimulation(grnd);
  return;
}
//------------------------------------------------------------------------
//  Clear all rates
//------------------------------------------------------------------------
bool CSlewManager::ZeroRate ()
{ aRate = fRate = lRate = 0.0f;
  return true;
}
//------------------------------------------------------------------------
//  Reset
//------------------------------------------------------------------------
bool CSlewManager::Reset()
{ ZeroRate();
  mode  = SLEW_STOP;
  return true;
}
//------------------------------------------------------------------------
//  Get altitude
//  Check that we are above ground anyway
//------------------------------------------------------------------------
void CSlewManager::SetAltitude(SPosition *p)
{ double grn = globals->tcm->GetGroundAltitude();
  double alt = p->alt;
  double agl = mveh->GetMinimumBodyAGL();
	//--- Check if bottom is above ground ----------
  if ((alt - agl) > grn) return;
  //---Must follow ground even when lower ------
	p->alt = grn + agl; 
	grnd = 1;
  return;
}
//------------------------------------------------------------------------
//  Normal move: Update aircraft position
//------------------------------------------------------------------------
void CSlewManager::NormalMove(float dT)
{ // Update user vehicle position
  SPosition pos = globals->geop;
  CVector   dir = globals->iang;
  // Range checking against latitude and altitude extremes is done in
  //   CWorldObject::SetPosition
  pos.alt += (aRate * dT);
  pos.lat += ((cos(dir.z) * fRate) + (cos(dir.z + HALF_PI) * lRate)) * dT;
  pos.lon -= ((sin(dir.z) * fRate) + (sin(dir.z + HALF_PI) * lRate)) * dT;
  pos.lon = WrapLongitude (pos.lon);
  SetAltitude(&pos);
  mveh->SetObjectPosition (pos);
  return;
}
//------------------------------------------------------------------------
//  Rabbit move: Update  position
//------------------------------------------------------------------------
void CSlewManager::RabbitMove(float dT)
{ // Update rabbit position
  SPosition pos = globals->geop;
	SVector   dir = globals->iang;
  // Range checking against latitude and altitude extremes is done in
  //   CWorldObject::SetPosition
  pos.alt += (aRate * dT);
  pos.lat += ((cos(dir.z) * fRate) + (cos(dir.z + HALF_PI) * lRate)) * dT;
  pos.lon -= ((sin(dir.z) * fRate) + (sin(dir.z + HALF_PI) * lRate)) * dT;
  pos.lon  = WrapLongitude (pos.lon);
	//--- compute altitude -----------------------------
	double grd = globals->tcm->GetGroundAltitude();
	if (pos.alt < grd)	{aRate = 0; pos.alt = grd; }
	//--- set new rabbit position ----------------------
	globals->geop = pos;
  return;
}
//------------------------------------------------------------------------
//  Update aircraft position
//------------------------------------------------------------------------
int CSlewManager::TimeSlice(float dT,U_INT f)
{	mveh = globals->pln;
	if (0 == mveh)		return 0;
  switch (mode) {
    case SLEW_STOP:
        break;
    //----Aircraft is moving -------------------------
    case SLEW_MOVE:
				NormalMove(dT);
				break;
		//--- Rabbit is moving ---------------------------
		case SLEW_RCAM:
				RabbitMove(dT);
				break;
		//--- 3D flight plan -----------------------------
		case SLEW_FPLM:
				break;
    //---Aircraft is leveling --------------------------
    case SLEW_LEVL:
        SetLevel(mveh);
				break;
  }
	return (globals->tcm->Teleporting())?(0):(1);
};
//--------------------------------------------------------------------------
//  Set Plane to level
//--------------------------------------------------------------------------
void CSlewManager::SetLevel(CVehicleObject *user)
{ int level = 0;
  CAirplane *pln = globals->pln;
  //-----Adjust pitch ---------------------------------
  CVector ori = globals->iang;
  float dx = abs(0 - ori.x);
  float sx = (ori.x > 0)?(-1):(+1);
  if (dx > 0.01)   ori.x += 0.01 * sx;
  else            {ori.x  = 0.0; level++;}
  //-----Adjust banking ------------------------------
  float dy = abs(0 - ori.y);
  float sy = (ori.y > 0)?(-1):(+1);
  if (dy > 0.01)   ori.y += 0.01 * sy;
  else            {ori.y  = 0.0; level++;}
  //-------------------------------------------------
  pln->SetPhysicalOrientation (ori);
  pln->SetObjectOrientation(ori);
  if (2 != level)   return;
  //-------------------------------------------------
  if (call)  pln->EndLevelling();
	//else       pln->SetObjectPosition(pos);
  mode   = pmde;          // Restore previous mode
  //-------------------------------------------------
	globals->sit->BackToSimulation(call);
  return;
}
//------------------------------------------------------------------------
//  Call to level vehicle
//  When opt is set, then veh->EndLevelling() is called
//------------------------------------------------------------------------
void CSlewManager::Level(char opt)
{ if (SLEW_LEVL == mode)    return; // Already set
	globals->Disp.ExecOFF(PRIO_ATMOSPHERE);
  call  = opt;                      // Save option
  pmde  = mode;                     // Previous mode
  mode  = SLEW_LEVL;                // New mode
  return;
}
//------------------------------------------------------------------------
//	Transform move by 100 feets and convert into arcsec
//------------------------------------------------------------------------
void	CSlewManager::RabbitMove(double x,double y, double z)
{	CVector v((x * 100),(y * 100) , (z * 10));
	flpn->MoveSelectedWPT(v);
	return;
}
//------------------------------------------------------------------------
//  Modify altitude rate
//------------------------------------------------------------------------
void CSlewManager::MoveOnZ(float d)
{	if (SLEW_FPLM == mode)	RabbitMove(0,0,d);
	aRate += d; 
	grnd = 0;
	return;	}
//------------------------------------------------------------------------
//  Move Forward
//------------------------------------------------------------------------
bool CSlewManager::MoveOnY(float d)
{ if (SLEW_FPLM == mode)	RabbitMove(0,d,0);
	fRate += d;
  return true;
}
//------------------------------------------------------------------------
//  Move Left
//------------------------------------------------------------------------
bool CSlewManager::MoveOnX(float d)
{ if (SLEW_FPLM == mode)	RabbitMove(d,0,0);
	lRate += d;
  return true;
}
//------------------------------------------------------------------------
//  Stop Move
//------------------------------------------------------------------------
bool CSlewManager::StopMove()
{ if (SLEW_STOP == mode) return false;
  lRate = 0.0f;
  aRate = 0.0f;
  fRate = 0.0f;
  return true;
}
//=========================================================================
//	TELEPORT PROFILE
//=========================================================================
#define PROF_TELEPORT (PROF_NO_MET | PROF_NO_APT )
//=========================================================================
// CSituation
//=========================================================================
CSituation::CSituation()
{ TRACE("=========CSituation start============");
  globals->sit	= this;
	sVeh					= 0;
	State					= SIT_NORMAL;
  //MEMORY_LEAK_MARKER (">SIT Construct")
  // Perform base initialization
	FrameNo	= 0;										
 // Open SIT file from pod filesystem
  OpenSitFile ();
  //MEMORY_LEAK_MARKER ("<SIT Construct")
}
//-------------------------------------------------------------------------
//  Open situation file
//-------------------------------------------------------------------------
void CSituation::OpenSitFile()
{ globals->Disp.ExecOFF(PRIO_ATMOSPHERE);
	SStream s(this,globals->sitFilename);
	if (!s.ok)	gtfo ("file %s not found", s.filename);
  return;
}
//-------------------------------------------------------------------------
//  Reload situation
//-------------------------------------------------------------------------
void CSituation::Reload()
{	ClearUserVehicle();
	planes.Clear();
	globals->ccm->NoCamera();
	InitialProfile();
	OpenSitFile();
	return;
}
//-------------------------------------------------------------------------
//  Free resources
//-------------------------------------------------------------------------
CSituation::~CSituation (void)
{ 
}
//-------------------------------------------------------------------------
//  Proced to Camera adjustment according to aircraft dimension
//-------------------------------------------------------------------------
void CSituation::AdjustCameras()
{	CVehicleObject   *veh = globals->pln; 
//--- Ensure cameras are at correct distance ---
  SVector dim = {0,0,0};
  if (veh)		veh->OverallExtension(dim);
  double  lgr = max(dim.x,dim.y);
  CCameraManager *ccm = globals->ccm;
  if (ccm)  ccm->AdjustRange(lgr);
  return;
}
//-------------------------------------------------------------------------
//  Read parameters
//  NOTE: Only the plane type is currently supported
//-------------------------------------------------------------------------
int CSituation::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;
  int i;

  switch (tag) {
  case 'vers':
    // Version number is ignored
    ReadInt (&i, stream);
    rc = TAG_READ;
    break;
  case 'time':
    { // Date/time
      /// \todo Initialize time manager with date/time in .sit file
      char s[PATH_MAX];
      ReadString (s, PATH_MAX, stream);
      rc = TAG_READ;
    }
    break;
  case 'wthr':
    // Weather not implemented yet
    SkipObject (stream);
    rc = TAG_READ;
    break;
  case 'scpt':
    { // Python script
      char scpt[80];
      ReadString (scpt, PATH_MAX, stream);
      rc = TAG_READ;
    }
    break;
  case 'wobj':
    { // Determine the type of world object
      char s[80];
      ReadString (s, 80, stream);
      Tag type = StringToTag (s);
      switch (type) {
				//--- Simulated object -------------------------------------------
        case TYPE_FLY_SIMULATEDOBJECT:
          { TRACE(".. Type is FLY_SIMULATEDOBJECT");
						if (globals->pln)	gtfo("More than 1 user aircraft in sit file");
            CSimulatedObject *sobj = GetASimulated();
            sVeh = sobj;
            //---Continue reading on behalf of the CVehicleObject --------
            ReadFrom (sobj, stream);
            return TAG_READ;
          }
				//--- User aircraft with a cockpit --------------------------------
        case TYPE_USER_AIRCRAFT:
          { TRACE(".. Type is TYPE_USER_AIRCRAFT");
            // sdk: prepare plugin dlls = DLLInstantiate
            if (globals->plugins_num) globals->plugins->On_Instantiate (0,0,NULL);
            // 122809
            CAirplane *plan = GetAnAircraft();
						sVeh     = plan;
            TRACE("FLY_AIRPLANE all read");
					  ReadFrom (plan, stream);
            return TAG_READ;
          }
				//--- Aircraft for AI system -------------------------------------
				
				case TYPE_ANIM_AIRCRAFT:
					{	TRACE(".. Type is TYPE_ANIM_AIRCRAFT");
						CAirplane *plan = new COPALObject (TYPE_ANIM_AIRCRAFT);
						ReadFrom (plan, stream);
						return TAG_READ;
					}
			
				//--- Helicopter -------------------------------------------------
        case TYPE_FLY_HELICOPTER:
          { // Instantiate new CHelicopterObject
            TRACE(".. Type is FLY_HELICOPTER");
            CHelicopterObject *heli = new CHelicopterObject ();
            ReadFrom (heli, stream);
            return TAG_READ;
					}
				
				default: break;
      }
		}
  default:	break;
    
	}
	WARNINGLOG ("CSituation::Read : Unknown tag <%s>", TagToString(tag));
	ReadFinished();
  return TAG_EXIT;
}
//---------------------------------------------------------------------------------
//  Set a new plane with the NFO file
//  NOTE: 
//    Some cameras are distance adjusted according to aircraft dimensions
//---------------------------------------------------------------------------------
void CSituation::SetAircraftFrom(char *nfo)
{ if (globals->pln)             return;
	SPosition	pos =	globals->geop;		// Preserve actual position
  //---Set aircraft from nfo ---------------------
  CVehicleObject *veh  = GetAnAircraft();
	veh->StoreOrgPosition(pos);
	veh->StoreNFO(nfo);
  veh->ReadFinished();
  AdjustCameras();
  return;
}
//---------------------------------------------------------------------------------
//  Process simulated obj type
//---------------------------------------------------------------------------------
CSimulatedObject* CSituation::GetASimulated (void)
{ CSimulatedObject *sobj = (CSimulatedObject *) new CSimulatedObject ();
  return sobj;
}

//---------------------------------------------------------------------------------
//  Process Plane type
//---------------------------------------------------------------------------------
CAirplane* CSituation::GetAnAircraft (void)
{ CAirplane *plan = NULL;
  char buf[128] = {""};

  GetIniString ("PHYSICS", "aircraftPhysics", buf, 128);
	//--- Get UFO aircraft --------------------------
  if (0 == strcmp (buf, "ufo")) 
	{ TRACE("Generate CUFOObject");
    plan = new CUFOObject(TYPE_USER_AIRCRAFT); //
		return plan;
  } 
		//--- Get normal aircraft ------------------------
  TRACE("Generate COPALObject");
  plan =  new COPALObject(TYPE_USER_AIRCRAFT); //
	//---Continue reading on behalf of the CVehicleObject --------
  return plan;
}
//---------------------------------------------------------------------------------
//  All parameters are read
//---------------------------------------------------------------------------------
void CSituation::ReadFinished (void)
{ // Initialize magnetic model date to the start date of the situation
  SDateTime dt = globals->tim->GetLocalDateTime ();
  CMagneticModel::Instance().Init (dt.date);
  // sdk:save a pointer to 'user' as the first item in the sdk SFlyObjectRef list
  //if (uVeh) sdk_flyobject_list.InsertUserInFirstPosition (uVeh);
  TRACE("CSituation::ReadFinished");
	//--- Load aircraft at position ----------------------
	CAirplane *pln = globals->pln;
	if (0 == pln)	gtfo("No User Aircraft in sit file");
	SPosition *pos = pln->GetAdPosition();
	CVector    ori = pln->GetOrientation();
	ShortTeleport(pos, &ori);
}
//---------------------------------------------------------------------------
// The Prepare method is called after loading of the situation, but
//   prior to the first Timeslice call.  It allows any initial real-time
//   preparation to be done, inter-system communication setup, etc.
//----------------------------------------------------------------------------
void CSituation::Prepare (void)
{ TRACE("CSituation::Prepare");
	// Prepare terrain manager

	// Create Database cache manager and populate first queues
	globals->dbc->TimeSlice(2,(U_INT)-1);				// Force update
  globals->tcm->TimeSlice(2,(U_INT)-1);       // Force Update

  /// SDK Stuffs JS/  Can we delete this????
  // sdk: don't know whether it's the right place or not ...
  // also added in main CleanupGlobals
  globals->plugins->On_EndSituation ();

  // sdk: prepare plugin dlls = DLLStartSituation
  globals->plugins->On_StartSituation ();

  // sdk: prepare plugin dlls = DLLInstantiate //
  globals->plugins->On_Link_DLLSystems (0,0,NULL);// 
	//--- Set initial state -------------------------
  
	return; 
}
//---------------------------------------------------------------------------------
//  Add an aircraft
//	Enter in dispatcher
//---------------------------------------------------------------------------------
void CSituation::AddPlane(CAirplane *p)
{	//--- Animated aircraft -----------------------------------------
	if (!p->IsUserPlan())	return	planes.AddPlane(p);
	globals->Disp.Store(p, PRIO_UPLANE);
	globals->pln	= p;
}
//---------------------------------------------------------------------------------
//  Write file
//---------------------------------------------------------------------------------
void CSituation::WriteFile()
{	CAirplane *pln =  globals->pln;
	if (0 == pln)										return;
	if (!pln->AllWheelsOnGround())	return;
  char *nfo = globals->pln->GetNFOname();
	CStreamFile sf;
  sf.OpenWrite (globals->sitFilename);
  sf.DebObject();
	sf.WriteTag('wobj',"-------- World Object -----------");
	sf.WriteString("plan");
	sf.DebObject();						// Start plane object
	sf.EditPosition(globals->geop);
	sf.WriteTag('_NFO',"--- vehicle info file -----------");
	sf.WriteString(nfo);
	sf.WriteTag('user',"--- User vehicle ----------------");
	sf.WriteTag('iang',"--- Inertial Angular Position ---");
	sf.WriteOrientation(globals->iang);
	sf.WriteComment("--NOTE that Y=-Z and Z = Y in the  sit file ---");
	sf.EndObject();						// End plane Object
  sf.EndObject();
  sf.Close();
}
//----------------------------------------------------------------------------
// This method is called on every simulation cycle, in order to update
//   the simulation world and all entities within it.  This method
//   calls the Draw method when it is deemed necessary to redraw the
//   sim world onscreen.
//----------------------------------------------------------------------------
void CSituation::Timeslice (float dT,U_INT frame)
{	// A new cycle begins --------------------------------
  dTime   = dT;
  FrameNo = frame;
	globals->Disp.TimeSlice(dT,frame);	// Call dispatcher
  return;
}
//----------------------------------------------------------------------------
//      Change user vehicle
//----------------------------------------------------------------------------
void CSituation::ClearUserVehicle()
{ CAirplane *pln = globals->pln;
	if (0 == pln)		return;
	//---Reset orientation ----------------------------------
  CVector nul(0,0,0);
  nul.z  = globals->iang.z;         
  globals->iang   = nul;
  globals->dang.x = RadToDeg(nul.x);
  globals->dang.y = RadToDeg(nul.y);
  globals->dang.z = RadToDeg(nul.z);
  globals->jsm->ClearGroupPMT(pln);
  //--- Stop engines --------------------------------------
  pln->eng.CutAllEngines();
  delete pln;
	pln = 0;
  //----Change to default camera -------------------------
  globals->cam = globals->csp;
  //----Clean all globals --------------------------------
  GlobalsClean ();
  return;
}
//==============================================================================
// JS:  Move Draw vehicle code here in CSituation where it belongs 
//  Draw all animated vehicles in the situation
//===============================================================================
void CSituation::DrawExternal()
{ globals->Disp.DrawExternal();
  return;
}

//-------------------------------------------------------------------------------
//  Draw user position and vehicle smoke
//-------------------------------------------------------------------------------
void CSituation::DrawVehicleFeatures()
{ CAirplane *pln = globals->pln;
	if (pln)	 pln->DrawExternalFeatures();
  return;
}
//-------------------------------------------------------------------------------
//  Set world origin (Just for IMPORT MODULE) Dont use elsewhere
//-------------------------------------------------------------------------------
void CSituation::WorldOrigin()
{	SPosition pos;  
  pos.lat = pos.lon = pos.alt = 0;
  globals->geop = pos;
  CVector ori(0,0,0);
  globals->iang = ori;
  globals->dang = ori;
}
//==============================================================================
//  Draw the situation
//  All drawing of the scene (except for PUI user interface widgets) is
//   controlled from this method.
//==============================================================================
void CSituation::Draw ()
{	switch (State)	{
			//-- Normal state: Draw everything ------------
			case SIT_NORMAL:
				return DrawNormal();
			//--- Enter teleport mode ---------------------
			case SIT_TELEP01:
				return TeleportS1();
			//--- Continue teleport -----------------------
			case SIT_TELEP02:
				return TeleportS2();
				return;
	}
return;
}
//----------------------------------------------------------------------------
//  Normal drawing
//----------------------------------------------------------------------------
void CSituation::DrawNormal()
{ CCamera *cam = globals->cam;
  //----Use standard camera setting for drawing ---------------------
  cam->StartShoot(dTime);
	cam->CameraReferential();
	//--- Draw Terrain, then upper priority ---------------------------
	globals->Disp.Draw(PRIO_TERRAIN);
	globals->Disp.Draw(PRIO_ABSOLUTE);
	//---Restore everything ---------------------------------------
  cam->StopShoot();
  // Check for an OpenGL error
  // CHECK_OPENGL_ERROR
}

//----------------------------------------------------------------------------
// Enter in teleport mode
//	for a given position P at orientation O
//	NOTE: During teleport, opal simulation is suspended until destination
//				Terrain is stable enough to support the aircraft.
//----------------------------------------------------------------------------
void CSituation::EnterTeleport(SPosition *P, SVector *O)
{	contx.prof	= PROF_TELEPORT;
	contx.mode	= SLEW_STOP;
  rcam			  = globals->ccm->SetRabbitCamera(contx,0);
	contx.pos		= *P;
	contx.ori		= *O;
	wait				= 0;
	P->alt			= 0;
	globals->geop	= *P;			// Target position
	TRACE("TELEPORT to lon=%lf lat=%lf alt=%lf",P->lon,P->lat,P->alt);
	//-- TODO: InitState (cut engine, gear down, etc) ----
	
	globals->pln->InitState();
	//--- prevent aircraft time slice and drawing ---
	globals->Disp.ExecOFF(PRIO_ATMOSPHERE);
	//--- Set camera to look up sky -----------------
	rcam->SetAngle(30,20);
	rcam->SetRange(200);
	//--- Set font for notice to user ---------------
	globals->fui->SetBigFont();
	State = SIT_TELEP01;
	//--- Warning message ---------------------------
	char txt[128];
	_snprintf(txt,127,"%05d Removing scenery files. PLEASE WAIT",wait++);
	DrawNoticeToUser(txt,4);
	return;
}
//----------------------------------------------------------------------------
// Teleport Step 01:  Wait for terrain stability
//----------------------------------------------------------------------------
void CSituation::TeleportS1()
{	char txt[128];
	_snprintf(txt,127,"%05d Teleporting to destination. PLEASE WAIT",wait++);
	DrawNoticeToUser(txt,4);
	bool ok = globals->tcm->TerrainStable(1);
	if (!ok)			return DrawNormal();
	//-- OK terrain ready at destination --------------
	globals->fui->ResetFont();
	//--- Restore vehicle position ----
	State = SIT_NORMAL;
  globals->ccm->RestoreCamera(contx);
	SPosition P = globals->geop;
	TRACE("STABLE at lon=%lf lat=%lf alt=%lf",P.lon,P.lat,P.alt);
	rcam			= 0;
	planes.BackToSimulation(1);
	DrawNormal();
	return;
}
//----------------------------------------------------------------------------
// Enter in short teleport mode
//	for a given position P at orientation O
//----------------------------------------------------------------------------
void CSituation::ShortTeleport(SPosition *P, SVector *O)
{	globals->geop	= *P;			// Target position
	contx.pos			= *P;			// Save position
	contx.ori			= *O;			// Save orientation
	State = SIT_TELEP02;
}
//----------------------------------------------------------------------------
// Teleport Step 01:  Wait for some time slice. Set camera to look down
//----------------------------------------------------------------------------
void CSituation::TeleportS2()
{	globals->Disp.ExecOFF(PRIO_ATMOSPHERE);
	bool ok = globals->tcm->TerrainStable(1);
	if (!ok)					return	DrawNormal();
	State = SIT_NORMAL;
	if (0 == sVeh)		return	DrawNormal();
	//--- Place aircraft on ground --------------

	sVeh->SetObjectPosition(contx.pos);
	sVeh->SetObjectOrientation(contx.ori);
	sVeh->SetPhysicalOrientation(contx.ori);
	planes.BackToSimulation(1);
	return DrawNormal();
}
//=====================================================================================
//   Animated planes
//=====================================================================================
CAnimatedPlane::CAnimatedPlane()
{	globals->Disp.Store(this,PRIO_APLANE);
}
//---------------------------------------------------------------------
//	Destroy it
//---------------------------------------------------------------------
CAnimatedPlane::~CAnimatedPlane()
{	Clear();
}
//---------------------------------------------------------------------
//	Delete all planes
//---------------------------------------------------------------------
void CAnimatedPlane::Clear()
{	for (U_INT k=0; k<plnQ.size(); k++)	delete plnQ[k];
	plnQ.clear();
}
//---------------------------------------------------------------------
//	Add a plane
//---------------------------------------------------------------------
void	CAnimatedPlane::AddPlane(CAirplane *p)
{	plnQ.push_back(p);
	return;
}
//---------------------------------------------------------------------
//	TimeSlice
//	NOTE:  TimeSLice is prohibited from execution when 
//		-SlewManager is ON
//		-Teleport (short or long) is ON
//---------------------------------------------------------------------
int CAnimatedPlane::TimeSlice(float dT,U_INT frame)
{	for (U_INT k=0; k< plnQ.size(); k++)
	{	CAirplane *pln = plnQ[k];
		pln->SetRelativePosition();
		pln->TimeSlice(dT,frame);
	}
	if (dT > globals->TimLim)				return	1; // fix to avoid aerodynamics errors when fps are low (related to fps limiter)
	//--- Call the physical simulation step ------
	bool ok = globals->opal_sim->simulate (dT);
	//--- Update all data ------------------------
	globals->pln->UpdateData(dT);
	for (U_INT k=0; k<plnQ.size(); k++) plnQ[k]->UpdateData(dT);
	
	return 1;
}
//---------------------------------------------------------------------
//	Draw external
//---------------------------------------------------------------------
void CAnimatedPlane::DrawExternal()
{	for (U_INT k=0; k< plnQ.size(); k++)
	{	CAirplane *p = plnQ[k];
		//--- Translate to aircraft position -----------
		p->DrawAnimated();
	}
}
//---------------------------------------------------------------------
//	Back to simulation after any slew, teleport, etc
//---------------------------------------------------------------------
void CAnimatedPlane::BackToSimulation(char grn)
{	CAirplane *pln = globals->pln;
	SPosition  pos = pln->GetPosition();
	if (grn)	pln->RestOnGround();
	else      pln->SetObjectPosition(pos);
	//--- Set position for other aircraft -----
	for (U_INT k=0; k<plnQ.size(); k++)
	{	plnQ[k]->SetRelativePosition();
	}
	//--- Activate the aircraft dispatcher ----
	globals->Disp.ExecON(PRIO_ATMOSPHERE);
	return;
}
//============================================================================================
//  Methods for structure TXT_LIST
//============================================================================================
void TXT_LIST::Init(U_SHORT nb,char **txt)
{ NbPtr = nb;
  List  = txt;
  CuPtr = 1;
  data  = List[0];
  return;
}
//--------------------------------------------------------------------------
//  reset pointer to start of list
//--------------------------------------------------------------------------
void TXT_LIST::Reset()
{ wkPtr = CuPtr;
  return;
}
//--------------------------------------------------------------------------
//  Restart at begining of text
//--------------------------------------------------------------------------
void TXT_LIST::Restart()
{ CuPtr = 1;
  data  = List[0];
  return;
}
//--------------------------------------------------------------------------
//  Return the next character pointer from the current
//--------------------------------------------------------------------------
char *TXT_LIST::GetNextAddr(char *txt)
{ if (0 == txt) txt = data; 
  if (*txt)   return txt;
  while (0 == *txt)
  { if (wkPtr == NbPtr)  break;
    txt = List[wkPtr++];
  }
  return txt;
}
//---------------------------------------------------------------------------
//  Set text to next position
//---------------------------------------------------------------------------
void  TXT_LIST::Increment()
{ data++;
  while (0 == *data)
  { if (CuPtr == NbPtr) break;
    data  = List[CuPtr++];
    wkPtr = CuPtr;
  }
  return;
}

//=========================END OF FILE =======================================================
