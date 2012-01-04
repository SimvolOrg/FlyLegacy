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
      In CSituation::Read I changed the TYPE_FLY_AIRPLANE case
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
#include "../Include/Joysticks.h"
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
using namespace std;
//=====================================================================================
//    GLOBAL CLEANUP
//=====================================================================================
void GlobalsClean (void)
{
  return;
}
//=====================================================================================
//    GLOBAL function to get the day model 
//=====================================================================================
//--------------------------------------------------------------------------------
//  Return model to draw
//--------------------------------------------------------------------------------
CModelACM *GetDayModelACM()
{ CVehicleObject   *veh = globals->pln;
  if (0 == veh)     return 0;
  CAnimatedModel *lod = veh->GetLOD();
  if (0 == lod)     return 0;
  return lod->GetDayModel();
}
//--------------------------------------------------------------------------------
//  Return model spatial extension (in feet)
//--------------------------------------------------------------------------------
void GetExtensionACM(SVector &v)
{ CModelACM  *mod = GetDayModelACM();
  if (mod)  mod->GetExtension(v);
  return;
}

///=====================================================================================
/// CRandomEvents
///=====================================================================================
CRandomEvents CRandomEvents::instance;

void CRandomEvents::Init (void)
{ TRACE ("CRandomEvents::Init");
  rc = 0;
  dirty = false;
  rc++;
  random.Set (0.0f, 1000, 1.0f);
}

CRandomEvents::~CRandomEvents (void) 
{
  ;
}

void CRandomEvents::Timeslice (float dT,U_INT Frame)
{ 
  random.TimeSlice (dT);
  int val = random.GetValue ();
  //TRACE ("CRandomEvents::Timeslice %d %u", val, globals->random_flag);
  switch (val) {
    case 28 :
      if ((globals->random_flag & RAND_TURBULENCE) != 0)
      { //TRACE ("turb(-) %d =%u", val, globals->random_flag);
        globals->random_flag &= ~RAND_TURBULENCE;}
      else
      { //TRACE ("turb(+) %d =%u", val, globals->random_flag);
        globals->random_flag |=  RAND_TURBULENCE;}
      break;
  }
  //if (val ==    8) TRACE ("CRandomEvents::Timeslice 0008"); 
  //if (val ==  100) TRACE ("CRandomEvents::Timeslice 0100"); 
  //if (val ==   28) TRACE ("CRandomEvents::Timeslice 0028"); 
  //if (val ==   38) TRACE ("CRandomEvents::Timeslice 0038"); 

}


///=====================================================================================
/// sdk: CFlyObjectListManager
///=====================================================================================
void CFlyObjectListManager::Init (void)
{
  rc = 0;

  tmp_fly_object.ref.objectPtr = NULL;
  tmp_fly_object.ref.classSig  = NULL;
  tmp_fly_object.ref.superSig  = NULL;
  tmp_fly_object.next          = NULL;
  tmp_fly_object.prev          = NULL;

  fo_list.clear ();

  // Allocate first struct
  fo_list.push_back (tmp_fly_object);
  dirty = false;
  rc++;
}

CFlyObjectListManager::~CFlyObjectListManager (void) 
{
  // dll are responsible for objects deletion in this list
  // except for the first one which is the user object;
/*/
  i_fo_list = fo_list.begin ();
  ++i_fo_list;                           // skip first user object
  for (i_fo_list; i_fo_list != fo_list.end (); ++i_fo_list) {
     SAFE_DELETE (i_fo_list->ref.objectPtr);          // 
  }
/*/
}

void CFlyObjectListManager::InsertUserInFirstPosition (const CVehicleObject *user)
{
  if (rc) {
    // (*it).ref.objectPtr
    std::list<SFlyObjectList>::iterator it = fo_list.begin ();
    (*it).ref.objectPtr = (CVehicleObject *) user;
    (*it).ref.classSig  = NULL;
    (*it).ref.superSig  = NULL;
  }
}

void CFlyObjectListManager::InsertDLLObjInList (const SDLLObject *obj)
{
  if (rc) {
    tmp_fly_object.ref = obj->flyObject;
    tmp_fly_object.next = NULL;
    tmp_fly_object.prev = NULL;
    fo_list.push_back (tmp_fly_object);
    rc++;
  }
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
	if (globals->aPROF.Has(PROF_RABIT))	return true;	
  CVector v(0,-1,0);
  globals->pln->AddOrientationInDegres(v);
  return true;
}
//----------------------------------------------------------------------------
//  slew bank right 'sbnr'
//----------------------------------------------------------------------------
bool sKeySBNR (int id, int code, int mod)
{	//--- Bank right 1° ---------------------------------
	if (globals->aPROF.Has(PROF_RABIT))	return true;
	CVector v(0,+1,0);
	globals->pln->AddOrientationInDegres(v);
	return true;
}
//----------------------------------------------------------------------------
//  Rotate left 'srtl'
//----------------------------------------------------------------------------
bool sKeySRTL (int id, int code, int mod)
{ // Left rotate 1° ----------------------------------
	if (globals->aPROF.Has(PROF_RABIT))	return true;
  CVector v(0,0,+1);
  globals->pln->AddOrientationInDegres(v);
  return true;
}
//----------------------------------------------------------------------------
//  Rotate right 'srtr'
//----------------------------------------------------------------------------
bool sKeySRTR (int id, int code, int mod)
{ // Right rotate 1° ----------------------------------
	if (globals->aPROF.Has(PROF_RABIT))	return true;
  CVector v(0,0,-1);
  globals->pln->AddOrientationInDegres(v);
  return true;
}
//----------------------------------------------------------------------------
//  Rotate Left 45° 'srl4'
//----------------------------------------------------------------------------
bool sKeySRL4 (int id, int code, int mod)
{ //Left Rotate 45 ° ----------------------------------
	if (globals->aPROF.Has(PROF_RABIT))	return true;
  CVector v(0,0,+45);
  globals->pln->AddOrientationInDegres(v);
  return true;
}
//----------------------------------------------------------------------------
//  Rotate Right 45° 'srr4'
//-----------------------------------------------------------------------------
bool sKeySRR4 (int id, int code, int mod)
{ //--- Rigth rotate 45° ------------------------------
	if (globals->aPROF.Has(PROF_RABIT))	return true;
  CVector v(0,0,-45);
  globals->pln->AddOrientationInDegres(v);
  return true;
}
//----------------------------------------------------------------------------
//  Pitch up 'sptu'
//----------------------------------------------------------------------------
bool sKeySPTU (int id, int code, int mod)
{ //pitch up 0.25° ------------------------------------
	if (globals->aPROF.Has(PROF_RABIT))	return true;
  CVector v(+0.25,0,0);
  globals->pln->AddOrientationInDegres(v);
  return true;
} 
//----------------------------------------------------------------------------
//  Picth down 'sptd'
//----------------------------------------------------------------------------
bool sKeySPTD (int id, int code, int mod)
{ //--- Pitch Down 0.25° -----------------------------
	if (globals->aPROF.Has(PROF_RABIT))	return true;
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
  CVehicleObject *veh = globals->pln;
  CVector v;
  v.x = v.y = v.z = 0.0;
  veh->SetOrientation (v);
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
{ veh   = 0;
  aRate = 0.0f;
  fRate = lRate = 0.0f;
  mode  = SLEW_STOP;
	//--- Bind all  keys -------------------------------------------
  BindKeys();
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
	veh = globals->pln;
  if (0 == veh)						return true;
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
//  Start slew mode
//------------------------------------------------------------------------
void CSlewManager::StartSlew()
{ grnd = veh->WheelsAreOnGround();
  mode = SLEW_MOVE;
  ZeroRate();
  vopt = globals->pln->GetOPT(VEH_D_CRASH);
  globals->pln->RazOPT(VEH_D_CRASH);
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
	if (globals->aPROF.Has(PROF_RABIT))	return;
  veh = globals->pln;
  if (0 == veh)												return;
  //---------------------------------------------
  mode = SLEW_STOP;
  if (grnd)  veh->RestOnGround();
  globals->pln->SetOPT(vopt);
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
{ double grd = globals->tcm->GetGroundAltitude();
  double alt = globals->geop.alt;
  double agl = veh->GetPositionAGL();
  if ((alt - agl) < grd) {p->alt = grd + agl; grnd = 1;}
  if (0 == grnd)  return;
  //---Must follow ground even when lower ------
  p->alt = grd + agl;
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
  veh->SetPosition (pos);
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
//  Flight plan move
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//  Update aircraft position
//------------------------------------------------------------------------
void CSlewManager::Update (float dT)
{	veh = globals->pln;
  if (0 == veh)   return;
  switch (mode) {
    case SLEW_STOP:
        return;
    //----Aircraft is moving -------------------------
    case SLEW_MOVE:
				NormalMove(dT);
				return;
		//--- Rabbit is moving ---------------------------
		case SLEW_RCAM:
				RabbitMove(dT);
				return;
		//--- 3D flight plan -----------------------------
		case SLEW_FPLM:
				return;
    //---Aircraft is leveling --------------------------
    case SLEW_LEVL:
        return SetLevel(veh);
  }
  return;
};
//--------------------------------------------------------------------------
//  Set Plane to level
//--------------------------------------------------------------------------
void CSlewManager::SetLevel(CVehicleObject *user)
{ int level = 0;
  CAirplane *pln = globals->pln;
  if (0 == pln)   return;
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
  pln->SetOrientation(ori);
  if (2 != level)   return;
  //-------------------------------------------------
  if (call)  pln->EndLevelling();
  mode   = pmde;          // Restore previous mode
  //-------------------------------------------------
  return;
}
//------------------------------------------------------------------------
//  Call to level vehicle
//  When opt is set, then veh->EndLevelling() is called
//------------------------------------------------------------------------
void CSlewManager::Level(char opt)
{ if (SLEW_LEVL == mode)    return; // Already set
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
// CSituation
//  NOTE:  TODO check if wobjList is needed. if not remove it
//          and just delete user 
//=========================================================================
CSituation::CSituation()
{
  globals->sit = this;
  TRACE("=========CSituation start============");
  //MEMORY_LEAK_MARKER (">SIT Construct")
  // Perform base initialization
	FrameNo	= 0;										// JSDEV*
  uVeh    = 0;
  sVeh    = dVeh = 0;
  //MEMORY_LEAK_MARKER (">rnd Construct")
  if (globals->random_flag & RND_EVENTS_ENABLED)
    CRandomEvents::Instance ().Init ();
  //MEMORY_LEAK_MARKER ("<rnd Construct")
  //---- Init DLL ------------------------
  dllW.clear ();
 // Open SIT file from pod filesystem
  OpenSitFile ();
  //MEMORY_LEAK_MARKER ("<SIT Construct")
}
//-------------------------------------------------------------------------
//  Open situation file
//-------------------------------------------------------------------------
void CSituation::OpenSitFile()
{ SStream s;
  if (OpenRStream (globals->sitFilename,s)) {
    // Successfully opened stream
    ReadFrom (this, &s);
    CloseStream (&s);
    return;
  }
  gtfo ("CSituation : Cannot open SIT file %s", s.filename);
  return;
}
//-------------------------------------------------------------------------
//  Reload situation
//-------------------------------------------------------------------------
void CSituation::ReloadAircraft()
{	ClearUserVehicle();
	InitialProfile();
	OpenSitFile();
	return;
}
//-------------------------------------------------------------------------
//  Free resources
//-------------------------------------------------------------------------
CSituation::~CSituation (void)
{ 
#ifdef _DEBUG
  DEBUGLOG ("CSituation::~CSituation");
#endif  
#ifdef _DEBUG
  //TRACE ("CSituation::~CSituation %p %p", sVeh, dVeh);
#endif
  //---Free all world objects ---------------------------
  SAFE_DELETE (uVeh);
  SAFE_DELETE (sVeh);
  //----delete user position drawing pointer ------------
  //---- delete all pointers in dllW --------------------
  FreeDLLWindows ();
}
//-------------------------------------------------------------------------
//  Free DLLWindows resources
//-------------------------------------------------------------------------
void CSituation::FreeDLLWindows (void)
{ 
#ifdef _DEBUG
  DEBUGLOG ("CSituation::FreeDLLWindows");
#endif  
  // delete all pointers in dllW
  for (idllW = dllW.begin (); idllW != dllW.end (); ++idllW) {
    SAFE_DELETE (*idllW);
  }
  dllW.clear (); // 
}
//-------------------------------------------------------------------------
//  Proced to Camera adjustment according to aircraft dimension
//-------------------------------------------------------------------------
void CSituation::AdjustCameras()
{ //--- Ensure cameras are at correct distance ---
  SVector dim = {0,0,0};
  GetExtensionACM(dim);
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

        case TYPE_FLY_SIMULATEDOBJECT:
          { TRACE(".. Type is FLY_SIMULATEDOBJECT");
            CSimulatedObject *sobj = GetASimulated();
            sVeh = sobj;
            //---Continue reading on behalf of the CVehicleObject --------
            ReadFrom (sobj, stream);
            return TAG_READ;
          }
        case TYPE_FLY_AIRPLANE:
          { TRACE(".. Type is FLY_AIRPLANE");
            // sdk: prepare plugin dlls = DLLInstantiate
            if (globals->plugins_num) globals->plugins.On_Instantiate (0,0,NULL);
            // 122809
            CAirplane *plan = GetAnAircraft();
            StoreVEH(plan);
            //---Continue reading on behalf of the CVehicleObject --------
            ReadFrom (plan, stream);
            TRACE("FLY_AIRPLANE all read");
            return TAG_READ;
          }
        case TYPE_FLY_HELICOPTER:
          { // Instantiate new CHelicopterObject
            TRACE(".. Type is FLY_HELICOPTER");
            CHelicopterObject *heli = new CHelicopterObject ();
            ReadFrom (heli, stream);
            return TAG_READ;
         }
       
      }
      rc = TAG_READ;
    }
    break;

  default:
    WARNINGLOG ("CSituation::Read : Unknown tag <%s>", TagToString(tag));
  }

  return rc;
}
//---------------------------------------------------------------------------------
//*  Check and store vehicle
//    inhibit vehicle under initialization
//---------------------------------------------------------------------------------
void CSituation::StoreVEH(CVehicleObject *veh)
{ if (uVeh) gtfo("CSituation: uveh not free");
  uVeh          = veh;
  return;
}
//---------------------------------------------------------------------------------
//  Set a new plane with the NFO file
//  NOTE: user entry must be free
//    Some cameras are distance adjusted according to aircraft dimensions
//---------------------------------------------------------------------------------
void CSituation::SetAircraftFrom(char *nfo)
{ if (uVeh)             return;
  //---Set aircraft from nfo ---------------------
  CVehicleObject *veh  = GetAnAircraft();
  veh->StoreNFO(nfo);
  veh->ReadFinished();
	uVeh			= veh;
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
{
  CAirplane *plan = NULL;
  char buffer_ [128] = {"ufo"};

  if (IsSectionHere ("PHYSICS")) {
    GetIniString ("PHYSICS", "aircraftPhysics", buffer_, 128);

    if (!strcmp (buffer_, "ufo")) {
      // tmp : use CUFOObject instead of CAirplane
      // for simplified aerodynamics and 3d rendering
      plan = (CUFOObject *) new CUFOObject (); // 
    } 
    else if (!strcmp (buffer_, "aero-opal")) {
      // tmp : use COPALObject instead of CAirplane
      //MEMORY_LEAK_MARKER ("OPALObject start")
      TRACE("Generate COPALObject");
      plan = new COPALObject (); // 
      //MEMORY_LEAK_MARKER ("OPALObject end")
    } 
    else if (!strcmp (buffer_, "normal")) {
      // Instantiate new CAirplane
      plan = new CAirplane (); // 
      plan->is_ufo_object  = false;
      plan->is_opal_object = false;
    } 
    else { // default
      // tmp : use CUFOObject instead of CAirplane
      strcpy (buffer_, "\0");
      TRACE("Generate CUFOObject");
      plan = (CUFOObject *) new CUFOObject (); // 
    }
  }
  else { // default
     // tmp : use CUFOObject instead of CAirplane
     strcpy (buffer_, "\0");
     plan = (CUFOObject *) new CUFOObject (); // 
  }
  return plan;
}
//---------------------------------------------------------------------------------
//  All parameters are read
//---------------------------------------------------------------------------------
void CSituation::ReadFinished (void)
{ 
  // Initialize magnetic model date to the start date of the situation
  SDateTime dt = globals->tim->GetLocalDateTime ();
  CMagneticModel::Instance().Init (dt.date);
  AdjustCameras();
  // sdk:save a pointer to 'user' as the first item in the sdk SFlyObjectRef list
  //if (uVeh) sdk_flyobject_list.InsertUserInFirstPosition (uVeh);
  TRACE("CSituation::ReadFinished");
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
  //MEMORY_LEAK_MARKER ("SIT Prepare 3")

  /// SDK Stuffs JS/  Can we delete this????
  // sdk: don't know whether it's the right place or not ...
  // also added in main CleanupGlobals
  if (globals->plugins_num) globals->plugins.On_EndSituation ();

  // sdk: prepare plugin dlls = DLLStartSituation
  if (globals->plugins_num) globals->plugins.On_StartSituation ();

  // sdk: prepare plugin dlls = DLLInstantiate //
  if (globals->plugins_num) {
    globals->plugins.On_Link_DLLSystems (0,0,NULL);// 
    //globals->plugins.On_Link_DLLGauges  (0,0,NULL);// 
//  if (globals->plugins_num) globals->plugins.On_Instantiate (0,0,NULL);
  }
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
  //--- Update weather parameters -----------------------
  globals->wtm->TimeSlice(dT,frame);
  //--- Udpate Terrain cache ----------------------------
  globals->tcm->TimeSlice(dT,frame);
  //--- Update Navigation cache -------------------------
  globals->dbc->TimeSlice(dT,frame);
  //---- Update the slew manager --------------------------
  globals->slw->Update (dT);
  //--- Wait until terrain is ready -----------------------
  if (globals->tcm->Teleporting())  return;
  //---- Update atmosphere  -----------------
  globals->atm->TimeSlice(dT,globals->geop.alt);
  //---Dont update any vehicle when special editing --------
	char nod = globals->noEXT + globals->noINT;
  if (nod >= 2)											return;
  //---- Update vehicle ------------------------------------
  if (uVeh) uVeh->TimeSlice(dT,frame);
  if (sVeh) sVeh->Timeslice(dT,frame);
  // sdk : 
  if (globals->plugins_num) {                                    
  //---- sdk: Update DLL FlyObjects -----------------------
    sdk_flyobject_list.i_fo_list = sdk_flyobject_list.fo_list.begin ();
    ++sdk_flyobject_list.i_fo_list;                                // skip first user object
    for (sdk_flyobject_list.i_fo_list; 
         sdk_flyobject_list.i_fo_list != sdk_flyobject_list.fo_list.end ();
       ++sdk_flyobject_list.i_fo_list) {
       ((CSimulatedObject *)(sdk_flyobject_list.i_fo_list)->ref.objectPtr)->Timeslice (dT,frame); // 
    }
  //---- sdk: Update plugin DLLIdle data -------------------
    globals->plugins.On_Idle (dT);
  //---- sdk: Update DLL windows ---------------------------
    for (idllW = dllW.begin (); idllW != dllW.end (); ++idllW) {
      (*idllW)->TimeSlice (dT);
    }
  }
  // random events
  if (globals->random_flag & RND_EVENTS_ENABLED)
    CRandomEvents::Instance ().Timeslice (dT, frame);
  return;
}
//----------------------------------------------------------------------------
//  Set vehicle position
//----------------------------------------------------------------------------
void CSituation::SetPosition(SPosition &pos)
{ if (uVeh) uVeh->SetPosition(pos);
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
  globals->jsm->ClearGroupPMT();
  //--- Stop engines --------------------------------------
  pln->eng->CutAllEngines();
  delete pln;
  uVeh = 0;
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
{ CAirplane *pln = globals->pln;
	//-------------------------------------------------------------
  //  Aircraft 
  //  Draw outside depending on camera type
  //  Camera at origin
  //-------------------------------------------------------------
  if (pln) pln->DrawExternal();
  //-------------------------------------------------------------
  //  Simulated Objects 
  //  both from saved simulation 'sVeh'
  //  or from dll 'dVeh'
  //-------------------------------------------------------------
  if (sVeh) sVeh->DrawExternal();
  else { // 
    globals->sit->sdk_flyobject_list.i_fo_list = globals->sit->sdk_flyobject_list.fo_list.begin ();
    ++globals->sit->sdk_flyobject_list.i_fo_list;                                // skip first user object
    for (globals->sit->sdk_flyobject_list.i_fo_list; 
         globals->sit->sdk_flyobject_list.i_fo_list != globals->sit->sdk_flyobject_list.fo_list.end ();
       ++globals->sit->sdk_flyobject_list.i_fo_list) {
        dVeh = (CDLLSimulatedObject *)((globals->sit->sdk_flyobject_list.i_fo_list)->ref.objectPtr); // 
        if (dVeh)  dVeh->DrawExternal();
    }
  }
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
//  Change situation
//-------------------------------------------------------------------------------
//==============================================================================
//  Draw the situation
//  All drawing of the scene (except for PUI user interface widgets) is
//   controlled from this method.
//===============================================================================
void CSituation::Draw ()
{ CCamera *cam = globals->cam;
  //----Use standard camera seting for drawing ------------------------
  cam->StartShoot(dTime);
  //----Draw sky background ------------------------------------

  globals->skm->PreDraw();
  //--------------------------------------------------------------------
  // Draw the terrain (ground textures, airports, scenery models, etc.)
  //--------------------------------------------------------------------
  globals->tcm->Draw(cam);                     //  Terrain cache
  //---Restore everything ---------------------------------------
  cam->StopShoot();
  // Check for an OpenGL error
  CHECK_OPENGL_ERROR
}
//==============================================================================
// sdk: Draw the DLL windows plugins
//  Called from CFUIManager::Draw method
//===============================================================================
void CSituation::DrawDLLWindow (void)
{
  for (idllW = dllW.begin (); idllW != dllW.end (); ++idllW) {
    (*idllW)->Draw ();
  }
}

//============================================================================================
//  Class CDLLWindow to display a DLL window plugin
//============================================================================================
CDLLWindow::CDLLWindow (void)
{ surf  = 0;
  obj   = 0;
  wd    = static_cast<short> (globals->mScreen.Width);
  ht    = static_cast<short> (globals->mScreen.Height);
  signature = 0;
  dll = 0;
  enabled = false;
  //back  = MakeRGB (212, 212,   0);
  back  = MakeRGB (  0,   0,   0);
  black = MakeRGB (  0,   0,   0);
  Resize();  
}
//---------------------------------------------------------------------------------
//  Destroy 
//---------------------------------------------------------------------------------
CDLLWindow::~CDLLWindow (void)
{ 
#ifdef _DEBUG
  //TRACE ("DELETE DLL SURF %p %p %p", surf, obj, dll);
#endif
  if (surf) surf = FreeSurface (surf);
  // sdk: cleanup objects = DLLDestroyObject // 
  globals->plugins.On_DestroyObject (obj, dll);
}
//---------------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------------
void CDLLWindow::SetObject (SDLLObject *object)
{
  obj = object;
}
//---------------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------------
void CDLLWindow::SetSignature (const long &sig)
{
  signature = sig;
}
//---------------------------------------------------------------------------------
//  Resize parameters
//---------------------------------------------------------------------------------
void CDLLWindow::Resize (void)
{ if (surf) FreeSurface (surf);
  surf  = CreateSurface (400, 400);
#ifdef _DEBUG
  //TRACE ("CREATE DLL SURF");
#endif
  return;
}
//---------------------------------------------------------------------------------
//  Time slice called from CSituation
//---------------------------------------------------------------------------------
void  CDLLWindow::TimeSlice (float dT)
{ if (0 == surf) return;
  // sdk: Draw the DLLDraw
  //--------------------------------------------------------------------
  //if (globals->plugins_num) globals->plugins.On_Draw (obj, surf);
  //--- e.g. Draw a line in a windows ---------------------------
  //EraseSurfaceRGB (surf, back);
  //DrawFastLine (surf, 0, 0, 500, 5, black);
  return;
}
//---------------------------------------------------------------------------------
//  Draw the surface
//  Should be called  from CFuiManager contex
//---------------------------------------------------------------------------------
void  CDLLWindow::Draw (void)
{ if (0 == surf) return;
  // Starting raster position is bottom-left corner of the surface,
  //   with (0,0) at top-left of the screen
  // sdk :
  if (globals->plugins_num) {
    glRasterPos2i (surf->xScreen, surf->yScreen + surf->ySize);
    //
    globals->plugins.On_Draw (obj, surf, dll); // 
    // Blit is performed in the dll
    //glDrawBuffer (GL_BACK);
    //glDrawPixels  (surf->xSize,   surf->ySize,
    //    GL_RGBA,
    //    GL_UNSIGNED_BYTE,
    //    surf->drawBuffer);
  }
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
