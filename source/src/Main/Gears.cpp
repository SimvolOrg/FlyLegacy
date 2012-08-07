/*
 * Gears.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
 * Copyright 2005 Laurent Claudet
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

#include "../Include/Globals.h"
#include "../Include/FlyLegacy.h"
#include "../Include/Gears.h"
#include "../Include/Fui.h"

using namespace std;

//=======================================================================================
//===================================================================
//  Wheel option menu for probe
//===================================================================
char *prob_WHL[] = 
  { "Base",         // 0 == Base
    "Brakes",       // 1 = Brakes
    0};
//====================================================================================
//
// CSuspension
//
//====================================================================================
//  JS NOTES:  
//    1)To support differential braking, a wheel position (left or rite) is added
//            when the object is a wheel
//    2) Add ident and hardware type for probe purpose
//=====================================================================================
CSuspension::CSuspension (CVehicleObject *v, CGroundSuspension *gssp, char *name, char tps)
{ Tag  idn = 'whl0';                                // Wheel identity
  mveh  = v;                                        // Save parent vehicle
  idn  |= mveh->GetWheelNum();                      // Get wheel number
  SetIdent(idn);                                    // Set identity
	gear_data.mgsp = gssp;														// Set mother suspension
	
  hwId  = HW_WHEEL;
  strncpy (gear_data.susp_name, name, 56);
  whel = new CAcmGears(v,gear_data.susp_name);      // Animated gear
  Tire = new CAcmTire (v,gear_data.susp_name);      // Animated wheels
  //-----------------------------------------------------------------
  gear        = 0;
  wInd        = 0;              // Wheel index
  reset_crash = 1;              // default is 1 crash detection enabled / 2 = reset
  type        = tps;            // Type 
  GetIniVar ("PHYSICS", "enableCrashDetect", &reset_crash);
  //-----------------------------------------------------------------
  InitGear();
}
//----------------------------------------------------------------------------------
//  Destroy object
//----------------------------------------------------------------------------------
CSuspension::~CSuspension (void)
{ gear_data.vfx_.clear ();

#ifdef _DEBUG
  DEBUGLOG ("CSuspension::~CSuspension");
#endif
  SAFE_DELETE (gear);
  SAFE_DELETE (whel);
  SAFE_DELETE (Tire);
}

//----------------------------------------------------------------------------------
//  Init Wheel
//----------------------------------------------------------------------------------
void CSuspension::InitGear()
{
#ifdef _DEBUG
  DEBUGLOG ("Starting COpalSuspension");
#endif
  char buffer [128] = {"\0"};
  GetIniString ("PHYSICS", "gearPhysics", buffer, 128);
  if (0 == strncmp (buffer, "jsbsim", 128)) 
  { gear = new CGearJSBSim(mveh,this); 
    return;}
  if ((0 == strncmp (buffer, "opal", 128)) && (TRICYCLE == type))
  { gear = new CGearOpal(mveh,this);
    return; }
  if ((0 == strncmp (buffer, "opal", 128)) && (TAIL_DRAGGER == type))
  { gear = new CTailGearOpal(mveh,this);
    return; }
  gear = new CGearJSBSim(mveh,this);
}
//----------------------------------------------------------------------------------
//  Read all parameters
//----------------------------------------------------------------------------------
int CSuspension::Read (SStream *stream, Tag tag)
{ float tm;

  switch (tag) {
    case 'long':// -- Longitudinal Crash Part Name --
  	  ReadString (gear_data.long_, 64, stream);// gear_f_crash
      return TAG_READ;

    case 'oy_N':// -- Longitudinal Damage Entry --
      ReadFrom (&gear_data.oy_N, stream);
      return TAG_READ;

    case 'powL':// -- Impact Power Limit (ft-lb/sec) (weight * velocity) --
      ReadFloat (&gear_data.powL, stream);
      return TAG_READ;

    case 'gear':// -- Rotating part name --
      char gearname[64];
      ReadString (gearname, 64, stream);
      whel->AddGear(gearname);
      return TAG_READ;

    case 'shck':// -- Springy Part name --
      char shckname[64];
      ReadString (shckname, 64, stream);
      whel->AddShok(shckname);
      return TAG_READ;

    case 'tire':// -- Spinny Part Name --
      char tirename[64];
      ReadString (tirename, 64, stream);
      Tire->AddTire(tirename);
      return TAG_READ;

    case 'oy_T':// -- Tire Damage Entry --
      ReadFrom (&gear_data.oy_T, stream);
      return TAG_READ;

    case 'ntwt':// -- Normal Tire Wear Rate vs Groundspeed (ktas) --
      SkipObject (stream);
      return TAG_READ;

    case 'ltwt':// -- Lateral Tire Wear Rate vs Lateral Groundspeed (ktas) --
      SkipObject (stream);
      return TAG_READ;

    case 'boff':// -- Blowout Friction Factor --
      ReadFloat (&gear_data.boff, stream); // 1.5
      return TAG_READ;

    case 'turn':// -- Steering Part Name --
      char turnname[64];
      ReadString (turnname, 64, stream);
      whel->AddTurn(turnname);
      return TAG_READ;

    case '-trn':// -- Reverse Frames --
      whel->InvertSteering();
      return TAG_READ;

    case 'ster':// -- steerable wheel --
      ReadInt (&gear_data.ster, stream);// -1
      return TAG_READ;

    case 'mStr':// -- max steer angle (deg) --
      ReadFloat (&gear_data.mStr, stream);// 8.0
      return TAG_READ;

    case 'maxC':// -- 0.32 max compression --
      ReadDouble (&gear_data.maxC, stream);// 0.32
      return TAG_READ;

    case 'damR':// -- 0.26 damping ratio --
      ReadDouble (&gear_data.damR, stream);// 0.26
      return TAG_READ;

    case 'drag':// -- drag coefficient --
      ReadFloat (&gear_data.drag, stream);
      return TAG_READ;

    case 'tirR':// -- Tire Radius (ft) --
      ReadFloat (&tm, stream);
      gear_data.tirR = tm;
      return TAG_READ;

    case 'rimR':// -- Rim Radius (ft) --
      ReadFloat (&gear_data.rimR, stream);// 0.5
      return TAG_READ;

    case 'time':// -- Gear retract time (s) --
      ReadFloat (&tm, stream);
      whel->AddTime(tm);
      return  TAG_READ;

    case 'mPos':// -- 0,-4.21,5.07 Tire Contact Point (model coordinates) --
      ReadVector (&gear_data.bPos, stream);// 0,-4.15,5.07
      whel->AddSound(gear_data.bPos);
      return  TAG_READ;
		// -- 1.428 Design Visual Vertical Displacement (ft) -- ignored
    case 'dvvd':
      ReadFloat (&tm, stream);// 1.3
      return  TAG_READ;

    case 'vfx_':// -- Visual Effects --
      char tmp_vfx[128];
      ReadString (tmp_vfx, 128, stream);
      gear_data.vfx_.push_back (tmp_vfx);
      while (strstr (tmp_vfx, "<endf>") == NULL) {
        ReadString (tmp_vfx, 128, stream);
        gear_data.vfx_.push_back (tmp_vfx);
      }
      return  TAG_READ;

    case 'endf'://
      return  TAG_READ;

    case 'brak'://
      int brak;
      ReadInt (&brak, stream);
      gear_data.brak = brak;
			if (brak) mveh->IncWheelBrake();
      return  TAG_READ;
		//--- Break force is ignored. It is now computed from SVH data 
		//	and force is equaly spread among wheel with brak flag 
    case 'brkF'://
      ReadFloat (&tm,stream);
      return  TAG_READ;
    }

  
   // Tag was not processed by this object, it is unrecognized
   WARNINGLOG ("CSuspension::Read : Unrecognized tag <%s>", TagToString(tag));
   return TAG_IGNORED;
}

//--------------------------------------------------------------------------
//  All parameters are read
//--------------------------------------------------------------------------
void CSuspension::ReadFinished (void)
{ int nbw			= mveh->GetWheelBrake();
	double coef = (nbw)?( double(1) / nbw):(0);
	//----Set wheel side -------------------------------------------
  gear_data.Side	= BRAKE_NONE;
	gear_data.repBF	= coef;
	//--- set brake to Left gear ---------------------
  if ((gear_data.bPos.x < 0)  && (gear_data.brak))
			{	gear_data.Side = (BRAKE_LEFT);
				gear_data.latK = +1;
			}
	//--- Set brake to right gear --------------------
  if ((gear_data.bPos.x > 0)	&& (gear_data.brak))
			{	gear_data.Side = (BRAKE_RITE);
				gear_data.latK = -1;
			}
  //----Compute wheel radius in feet -----------------------------
  gear_data.whrd = gear_data.tirR + gear_data.rimR;
  Tire->SetRadius(gear_data.whrd);
  //---------------------------------------------------------------
  gear->SetGearData (&gear_data);
}

void CSuspension::Debug (void)
{
#ifdef _DEBUG_suspension	
	FILE *fp_debug;
	if((fp_debug = fopen("__DDEBUG_suspension.txt", "a")) != NULL)
	{
      std::vector<std::string>::iterator it;
      for (it = gear_data.vfx_.begin (); it != gear_data.vfx_.end (); it++) {
        fprintf(fp_debug, "CSuspension::Debug %s\n", (*it).c_str ());
      }
      fclose(fp_debug); 
	}
#endif
}

void CSuspension::Write (SStream *stream)
{
  ;
}
//-----------------------------------------------------------------------
//  Gear is damaged
//-----------------------------------------------------------------------
void CSuspension::GearDamaged(char nsk)
{ U_INT f = globals->Frame;
  TRACE("%06d: Wheel %s Dammaged (%d) at %.4f",f,gear_data.susp_name,nsk,mveh->GetAltitude());
  char dam1[64];
  _snprintf(dam1,63,"Gear %s shoked",gear_data.susp_name);
	dam1[63] = 0;
  char dam2[64];
  _snprintf(dam2,63,"Gear %s destroyed",gear_data.susp_name);
	dam2[63] = 0;
  DAMAGE_MSG msg = {1,0,0};
  msg.Severity = (nsk < 2)?(1):(2);
  msg.msg      = (nsk < 2)?(dam1):(dam2);
  msg.snd      = (nsk < 2)?('shok'):('crgr');
  mveh->DamageEvent(&msg);
  return;
}
//-----------------------------------------------------------------------
//  Wheel is broken
//  grd is the ground altitude
//-----------------------------------------------------------------------
char CSuspension::GearShock(char pw)
{ if (mveh->NotOPT(VEH_D_CRASH)) return 1;
  PlayTire(0);
  //-------------------------------------------------
  gear_data.shok += pw;
  GearDamaged(gear_data.shok);
  //--- Set aircraft above ground -------------------
  double bagl = mveh->GetBodyAGL();
  double alti = globals->tcm->GetGroundAltitude() + bagl;
  mveh->SetAltitude(alti);
  return 1;
}
//-----------------------------------------------------------------
//  Time slice the wheel 
//-----------------------------------------------------------------
void CSuspension::Timeslice (float dT)
{
  // 1) aerodynamic purpose
  // get the gear position relative to the actual CG
  SVector actualCG_;
  mveh->wgh->GetVisualCG (actualCG_);  ///< RH feet
  SVector actualCG; 
  actualCG.x = -actualCG_.x; actualCG.y = actualCG_.z; actualCG.z = actualCG_.y; // RH->LH
  // gets mPos as the 3D gear position
  gear->GearLoc_Timeslice (&actualCG, gear_data.bPos);                     ///< mPos = tire contact feet
  // 2) the neW gear location is transformed in the actual body coordinates
  // (the aircraft is not alWays Well levelled in all its axes)
  // gets WPos
  //gear->GearB2L_Timeslice ();
  // 3) get the CG AGL
  // 
  // get the gear compression value
  // and update the WOW flag (Weight-on-heels)
  gear_data.onGd = gear->GCompression (gear_data.onGd);
  // 4)
  // compute ground physics only if needed
  if (IsOnGround()) { // weight on wheels is verified

    // 5) compute gear compression velocity
    //
    gear->GComprV_Timeslice ();

    // 6) compute force and moment from the wheel
    //
    gear->DirectionForce_Timeslice(dT); // 
    gear->VtForce_Timeslice (dT);

    // 8) compute force and moment
    gear->GearL2B_Timeslice ();
    gear->VtMoment_Timeslice ();

  }

  if (!IsOnGround())    gear->ResetValues ();

}

//-----------------------------------------------------------------
const SVector& CSuspension::GetBodyForce_ISU (void)
{
    return (gear->GetBodyGearForce_ISU ());
}
//-----------------------------------------------------------------
const SVector& CSuspension::GetBodyMoment (void)
{
  return (gear->GetBodyGearMoment ());
}

//-----------------------------------------------------------------
const SVector& CSuspension::GetBodyMoment_ISU (void)
{
  return (gear->GetBodyGearMoment_ISU ());
}
//-----------------------------------------------------------------
bool CSuspension::IsSteerWheel (void)
{ bool val = false; 
  (gear_data.ster) ? val = true : val = false;
  return val;
}
//------------------------------------------------------------------------
//  Return probe option menu
//------------------------------------------------------------------------
char **CSuspension::GetProbeOptions()
{ return prob_WHL; }
//------------------------------------------------------------------------
//  Probe this suspension
//------------------------------------------------------------------------
void CSuspension::Probe(CFuiCanva *cnv)
{   cnv->AddText( 1,gear_data.susp_name,1);
    if (popt == 1)  return gear->ProbeBrake(cnv);
		if (popt == 0)	return gear->Probe(cnv);
    return;
}
//=====================================================================================
//  CGEAR
//
// CGear
//
//   JS NOTES:  Removed code related to animation as it is not related
//              to gear physic although it needs some interface.
//              Animation needs also to take care of more complex
//              scenaries.  Wheel animation is now located in ACM module
//              and CAnimatedModel
//=====================================================================================
CGear::CGear (CVehicleObject *vh,CSuspension *sp)
{ mveh = vh;            // Save parent vehicle
  susp = sp;            // Save suspension
	WPos.lat      = WPos.lon     = WPos.alt     = 0.0;
  vWhlVelVec.x  = vWhlVelVec.y = vWhlVelVec.z = 0.0;
}
//----------------------------------------------------------------------
//  Destroy Gear
//-----------------------------------------------------------------------
CGear::~CGear (void)
{ 
#ifdef _DEBUG
  DEBUGLOG ("CGear::~CGear");
#endif
}
void CGear::ResetValues (void)
{
  vForce.Zero();
  vMoment.Zero();
}

//-----------------------------------------------------------------------
//  check if point is inside the gear volume arround contact point
//-----------------------------------------------------------------------
bool CGear::WheelCollision(CVector &p)
{ CVector mp;
  double  rd;  
  GetGearPosition(mp,rd);
  double dx  = mp.x - p.x;
  double dy  = mp.y - p.y;
  double lg  = SquareRootFloat((dx * dx) + (dy * dy));
  double lm  = 4 * rd;
  //   double lg = pnt.DistanceTo(mp);
  if (lg < lm) return true;
  //--- This wheel is crashed ------------------------
  susp->GearDamaged(2);
  return false;
}
//-----------------------------------------------------------------------
//* Find the location of the gear relative to the physical CG in ft
//* mPos from the WHL file - actualCG from the weight manager class
//* body frame : vGearLoc2CG in feet
//-----------------------------------------------------------------------
void CGear::GearLoc_Timeslice (const SVector *actualCG, const SVector &mPos)
{
  // find the location of the uncompressed landing gear relative to the CG
  // body frame : vGearLoc2CG in feet
  vGearLoc2CG.x = mPos.x - actualCG->x;
  vGearLoc2CG.y = mPos.y - actualCG->y;
  vGearLoc2CG.z = mPos.z - actualCG->z;
}
//-----------------------------------------------------------------------
//* Find the location of the gear relative to the world coordinates
//* local frame : WPos in SPosition alt in feet
//-----------------------------------------------------------------------
/*
void CGear::GearB2L_Timeslice (void)
{
  /// \todo calc Wheel position relative to the body in the world coordinates
  /// tmp = the body position is assumed levelled in all axes
  // tmp formula
  // to be tested later
  SVector bodyPos;
  SPosition cgPos = mveh->GetPosition();                  ///< alt in feet lat and lon in arcsec
  SVector ori     = globals->iang;												///< rad
  /// \todo use matrix instead
  TurnVectorFromFulcrum (vGearLoc2CG, ori, bodyPos);
//  BodyVector2WorldPos   (cgPos, bodyPos, WPos);           ///< WPos = SPosition so alt in feet
}

*/
//-----------------------------------------------------------------------
///< force in Newton
//-----------------------------------------------------------------------
const SVector& CGear::GetBodyGearForce_ISU  (void)
{
  vLocalForce_ISU.x = vLocalForce.x * LBS_TO_NEWTON;
  vLocalForce_ISU.y = vLocalForce.y * LBS_TO_NEWTON;
  vLocalForce_ISU.z = vLocalForce.z * LBS_TO_NEWTON;
  

  #ifdef _DEBUG_suspension	
  { FILE *fp_debug;
    if((fp_debug = fopen("__DDEBUG_suspension.txt", "a")) != NULL)
    {
          fprintf(fp_debug, "CGear::GetGearForce_ISU (%.3f %.3f %.3f)\n",
            vLocalForce_ISU.x, vLocalForce_ISU.y, vLocalForce_ISU.z);
          //
          fclose(fp_debug); 
  }    }
  #endif

  return vLocalForce_ISU;
} 
//-----------------------------------------------------------------------
///< moment in Kg.m
//-----------------------------------------------------------------------
const SVector& CGear::GetBodyGearMoment_ISU (void)
{
  double konst = LBFT_TO_KGM (1.0, 1.0);//0.1382566

  vLocalMoment_ISU.x = vLocalMoment.x * konst;
  vLocalMoment_ISU.y = vLocalMoment.y * konst;
  vLocalMoment_ISU.z = vLocalMoment.z * konst;
  #ifdef _DEBUG_suspension	
  {   FILE *fp_debug;
    if((fp_debug = fopen("__DDEBUG_suspension.txt", "a")) != NULL)
    {
          fprintf(fp_debug, "CGear::GetBodyGearMoment_ISU (%.3f %.3f %.3f)\n",
            vLocalMoment_ISU.x, vLocalMoment_ISU.y, vLocalMoment_ISU.z);
          //
          fclose(fp_debug); 
  }    }
  #endif
  return vLocalMoment_ISU;
}
//================================================================================
//
// CGroundSuspension
// Based upon JSBSim code
// see copyright below
///
//  JS NOTES:  
//    1)Wheel index is 0 based.  Interface to grlt (gear light) is 1 based
//    2) Add subsystem ident for probe purpose
//================================================================================
CGroundSuspension::CGroundSuspension (CVehicleObject *v,CWeightManager *wgh)
{ SetIdent('susp');                   // suspension manager
  hwId    = HW_UNKNOWN;               // No type
  mveh    = v;                        // Save parent object
  wInd    = 0;                        // Wheel index
  // Get a link to weight_manager
  // !!! must be before Read from Stream below ...
  whm     = wgh;
  wheels_num = 0;
  rMas    = 0.0f;
  type    = TRICYCLE;                 // JS Assume standard type
  mstbl   = 0;
  mbtbl   = 0;
	bump		= 28;
	difB	  = 1;
	steer		= 0;
	ampB    = 2;
  //---------------------------------------------------------
	SumGearForces.Raz();
	SumGearMoments.Raz();
  //---------------------------------------------------------
  mainW.Raz();
  mainR  = 0;
  max_wheel_height = 0.0;
  max_gear = min_gear = mWPos = 0.0;
}
//------------------------------------------------------------------------------
// destroy this
//------------------------------------------------------------------------------
CGroundSuspension::~CGroundSuspension (void)
{ 
  SAFE_DELETE (mstbl);
  SAFE_DELETE (mbtbl);
	for (U_INT k=0; k < whl_susp.size(); k++) delete whl_susp[k];
	whl_susp.clear();
	for (U_INT k=0; k < whl_bump.size(); k++) delete whl_bump[k];
	whl_bump.clear();
}
//------------------------------------------------------------------------------
// Read all parameters
//------------------------------------------------------------------------------
void CGroundSuspension::ReadSusp(SStream *st)
{   char susp_[64], susp_type[8], susp_name[56];
    ReadString (susp_, 64, st);
    if (sscanf (susp_, "%s %s", susp_type, susp_name) != 2) return;
    if (!strcmp (susp_type, "whel"))
    {   CSuspension *susp = new CSuspension (mveh, this,susp_name, type); // global default
        ReadFrom (susp, st);
        whl_susp.push_back (susp);
        return;
    }
    if (!strcmp (susp_type, "bmpr"))
    {   CWhl *bump = new CBumper     (susp_name, whm, type);
        ReadFrom (bump, st);
        whl_bump.push_back (bump);
        return;
      } 
    WARNINGLOG ("CGroundSuspension::Read : bad susp type");
    return;

}
//------------------------------------------------------------------------------
// Read all parameters
//------------------------------------------------------------------------------
int CGroundSuspension::Read (SStream *stream, Tag tag)
{ switch (tag) {
  case 'rMas':
    // rated mass (slugs)
    ReadFloat (&rMas, stream);
    whm->whl_rmas = rMas;
    return TAG_READ;
	//--- Bump force ----------------------------------------
	case 'bump':
		ReadDouble(&bump,stream);
		return TAG_READ;
	//--- Brake amplifier -----------------------------------
	case	'bamp':
		ReadDouble(&ampB,stream);
		return TAG_READ;
	//--- Differential brake amplifier ---------------------
	case 'difB':
		ReadDouble(&difB,stream);
		return TAG_READ;
  //---JS decode suspension type -------------------------
  case 'type':
    { char txt[64];
      ReadString(txt,64,stream);
      if (strcmp(txt,"TRICYCLE")      == 0) type = TRICYCLE;
      if (strcmp(txt,"TAIL_DRAGGER")  == 0) type = TAIL_DRAGGER;
      if (strcmp(txt,"SNOW")          == 0) type = SNOW;
      if (strcmp(txt,"SKIDS")         == 0) type = SKIDS;
      return TAG_READ;
    }
    //--JS decode steering table -------------------------------
  case 'stbl':                              
    { CDataSearch map(stream);
      mstbl = map.GetTable();
      return TAG_READ;
    }
    //--LC decode brake table -------------------------------
  case 'btbl':                              
    { CDataSearch map(stream);
      mbtbl = map.GetTable();
      return TAG_READ;
    }
  case 'susp':
    // a suspension object
    ReadSusp(stream);
    return TAG_READ;
  }

  // Tag was not processed by this object, it is unrecognized
  WARNINGLOG ("CGroundSuspension::Read : Unrecognized tag <%s>", TagToString(tag));
  return TAG_IGNORED;
}
//---------------------------------------------------------------------------------
//  Compute main gear axe barycenter
//  Compute steering gear axe center in Legacy coordinate (Z is up)
//---------------------------------------------------------------------------------
void CGroundSuspension::ReadFinished (void)
{ SGearData *gdt = 0;   //GetGearData()
#ifdef _DEBUG
  DEBUGLOG ("CGroundSuspension::ReadFinished");
#endif
  wheels_num	= whl_susp.size ();
  //---Compute wheel parameters --(Z is forward direction)---------
  max_gear = 0.0, min_gear = 0.0, mWPos = 0.0;
  double wheel_min_h = -1000.0;
  int nbw = 0;
  std::vector<CSuspension *>::const_iterator rw;
  for (rw = whl_susp.begin (); rw != whl_susp.end (); rw++) {
    CSuspension *s = (CSuspension *) *rw;
		s->SetAmplifier(ampB);
    max_gear = max (s->GetGearPosZ (), max_gear); // max forward
    min_gear = min (s->GetGearPosZ (), min_gear); // min forward
    mWPos    = min (s->GetGearPosY (), mWPos);    // lower point
    wheel_min_h  = max (s->GetGearPosY (), wheel_min_h ); // LH
    //---Compute main gear barycenter ---------------------
    if (s->IsSteerWheel())  {	steer = s; continue; }
		//--- Main gear ---------------------------------------
    nbw++;
    mainW.x += s->GetGearPosX();
    mainW.z += s->GetGearPosY();
    mainW.y += s->GetGearPosZ();
    mainR   += s->GetGearRadius();
  }
  //--------------------------------------------------------
  //  The barycenter is the center of all main gear axis
  //  -mainW is the average contact point of all main
  //   gears in feet coordinates.  To get the average axis
  //   point , we just add the average radius. (as the
  //   contact point is a negative position, we must add
  //  the wheel radius
  //--------------------------------------------------------
	double fac = (nbw)?(double(1) / nbw):(0);
  mainW.Times(fac);
  mainR     *= fac;
  mainW.z   += mainR;
  //--------------------------------------------------------
  sterR      = steer->GetGearRadius();
  //--------------------------------------------------------
  //  We also compute the bAGL (body above ground level)
  //  bAGL is used to set the aircraft level above a given terrain
  //  bAGL is what that must be added to ground to set 
  //  the aircraft correctly on ground 
	//	One foot is added for rounding up
	//	(relative to aircraft visual model)
	//	For physical model, the Cog offset must be accounted
	//--------------------------------------------------------
	//	Wheel_base is the longitudinal amplitude of the gear
	//	(all wheels accounted). It is used to compute mass
	//	repartition over the gear
  //--------------------------------------------------------
  bAGL  = mainR - mainW.z + 1;
  //--- Compute Wheel base -(in meter)  --------------------
  wheel_base	= max_gear - min_gear;
	double base = FN_METRE_FROM_FEET(wheel_base);
	for (rw = whl_susp.begin (); rw != whl_susp.end (); rw++)
	{ (*rw)->SetWheelBase(base);
	}
  //--------------------------------------------------------
  if (TRICYCLE == type)
    max_wheel_height =  - mWPos;
  else
  if (TAIL_DRAGGER == type)
    max_wheel_height =  - (mWPos - wheel_min_h);
  else
    max_wheel_height =  - mWPos;
	//------------------------------------------------------------------------
	//JS note: This table is used to modulate the Brake force versus 
	//        ground speed.  Brake force  must decrease with speed
	//------------------------------------------------------------------------
	if (0 == mbtbl) 
	{ // Brake force turn table
    CFmt1Map *map = new CFmt1Map();
    mbtbl  = map;
    map->Add(00.0f, 1.0f);
    map->Add(30.0f, 0.7f);
    map->Add(45.0f, 0.5f);
    map->Add(60.0f, 0.3f);
    map->Add(90.0f, 0.1f);
  }
	//------------------------------------------------------------------------
	//JS note: This table is used to modulate the steering force versus 
  //        ground speed.  steering must decrease with speed
  //------------------------------------------------------------------------
  if (0 == mstbl) 
  { // Ground speed turn table 
    CFmt1Map *map = new CFmt1Map();
    mstbl   = map;
    map->Add(00.0f, 5.00f);
    map->Add(05.0f, 2.00f);
    map->Add(10.0f, 1.50f);
    map->Add(20.0f, 0.50f);
    map->Add(90.0f, 0.01f);
  }

}
//---------------------------------------------------------------------------------
//	Init steering path
//---------------------------------------------------------------------------------
void	CGroundSuspension::SetSteerData(CRudderControl *rud)
{	if (0 == rud)			return;
	if (0 == steer)		return;
	SGearData *gdt = steer->GetGearData();
	gdt->scaled  = 0;
	rud->InitSteer(gdt);
	return;
}
//---------------------------------------------------------------------------------
//  Time Slice all wheels
//---------------------------------------------------------------------------------
void CGroundSuspension::Timeslice (float dT)
{ nWonG = 0;      // No wheels on ground
	SumGearForces.Raz();
	SumGearMoments.Raz();

  if (type == TRICYCLE) {;}
  else {;} // \todo different kind of train
  std::vector<CSuspension *>::const_iterator it_whel;
  for (it_whel = whl_susp.begin (); it_whel != whl_susp.end (); it_whel++) {
    // is it a steering wheel ?
    CSuspension *ssp = (CSuspension*)(*it_whel);
    ssp->Timeslice (dT);
    // sum all gear forces
    const SVector gf  = (*it_whel)->GetBodyForce_ISU  ();
    SumGearForces.x  += gf.x;
    SumGearForces.y  += gf.y;
    SumGearForces.z  += gf.z;
    // sum all gear moments
    const SVector gm  = (*it_whel)->GetBodyMoment_ISU ();
    SumGearMoments.x += gm.x;
    SumGearMoments.y += gm.y;
    SumGearMoments.z += gm.z;
    //--- update wheels on ground --------------------
    if (ssp->IsOnGround())  nWonG++;
  }
}
//---------------------------------------------------------------------------------
//  Reset crash
//---------------------------------------------------------------------------------
void CGroundSuspension::ResetCrash()
{ std::vector<CSuspension *>::const_iterator wi;
  for (wi = whl_susp.begin (); wi != whl_susp.end (); wi++)
  { CSuspension *ssp = (CSuspension*)(*wi);
    ssp->ResetCrash();
    ssp->ResetForce();
  }
  return;
}
//---------------------------------------------------------------------------------
//  Reset crash
//---------------------------------------------------------------------------------
void CGroundSuspension::ResetForce()
{ std::vector<CSuspension *>::const_iterator wi;
  for (wi = whl_susp.begin (); wi != whl_susp.end (); wi++)
  { CSuspension *ssp = (CSuspension*)(*wi);
    ssp->ResetForce();
  }
  return;
}
//---------------------------------------------------------------------------------
//  Set ABS feature
//---------------------------------------------------------------------------------
void CGroundSuspension::SetABS(char p)
{ std::vector<CSuspension *>::const_iterator wi;
  for (wi = whl_susp.begin (); wi != whl_susp.end (); wi++)
  { CSuspension *ssp = (CSuspension*)(*wi);
    ssp->SetABS(p);
  }
  return;
}

//================================================================================
// CBumper
//================================================================================
CBumper::CBumper (char *name_, CWeightManager *wghman, char type)
{
  weight_manager = wghman;        // get a pointer to the weight manager for CG
  strncpy (bump_name, name_, 56);
  bmpF[0] = 0;
  bmpF[1] = 0;
  mPos.x = 0.0;                    // -- Contact Point (model coodinates) --
  mPos.y = 0.0;                    // -- Contact Point (model coodinates) --
  mPos.z = 0.0;                    // -- Contact Point (model coodinates) --
  vfx_.clear ();                   // -- Visual Effects --
  // endf;                         //
}
//---------------------------------------------------------------------------------
//  Destroy it
//---------------------------------------------------------------------------------
CBumper::~CBumper (void)
{	vfx_.clear();}
//---------------------------------------------------------------------------------
//  Read all parameters
//---------------------------------------------------------------------------------
int CBumper::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {

  case 'mPos':// -- Contact Point (model coodinates) --
    ReadVector (&mPos, stream);// 
    rc = TAG_READ;
    break;

  case 'bmpF'://
    ReadInt (&bmpF[0], stream);
    ReadInt (&bmpF[1], stream);
    rc = TAG_READ;
    break;

  case 'oy_T':// -- Damage Entry --
    ReadFrom (&oy_T, stream);
    rc = TAG_READ;
    break;

  case 'vfx_':// -- Visual Effects --
    char tmp_vfx[128];
    ReadString (tmp_vfx, 128, stream);
    vfx_.push_back (tmp_vfx);
    while (strstr (tmp_vfx, "<endf>") == NULL) {
      ReadString (tmp_vfx, 128, stream);
      vfx_.push_back (tmp_vfx);
    }
    rc = TAG_READ;
    break;

  case 'endf'://
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // Tag was not processed by this object, it is unrecognized
    WARNINGLOG ("CBumper::Read : Unrecognized tag <%s>", TagToString(tag));
  }

  return rc;
}
//---------------------------------------------------------------------------------
//  All parameters are read
//---------------------------------------------------------------------------------
void CBumper::ReadFinished (void)
{}
//---------------------------------------------------------------------------------
//  Debbug call
//---------------------------------------------------------------------------------
void CBumper::Debug (void)
{
#ifdef _DEBUG_suspension	
	FILE *fp_debug;
	if((fp_debug = fopen("__DDEBUG_suspension.txt", "a")) != NULL)
	{
      std::vector<std::string>::iterator it;
      for (it = vfx_.begin (); it != vfx_.end (); it++) {
		fprintf(fp_debug, "CBumper::Debug %s\n", (*it).c_str ());
      }
	  
      fclose(fp_debug); 
	}
#endif
}
//---------------------------------------------------------------------------------
//  Write it
//---------------------------------------------------------------------------------
void CBumper::Write (SStream *stream)
{
}
//---------------------------------------------------------------------------------
//  Time slice
//---------------------------------------------------------------------------------
void CBumper::Timeslice (float dT)
{
//  Debug ();
}
//=======END of FILE =================================================================
