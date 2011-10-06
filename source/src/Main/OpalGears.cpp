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
#include "../Include/OpalGears.h"
#include "../Include/Collisions.h"
#include "../Include/Fui.h"
using namespace std;
//====================================================================================

//=====================================================================
//  CGearOpal
//=====================================================================

CGearOpal::CGearOpal (CVehicleObject *v,CSuspension *s) : CGear (v,s)
{ diffK = ADJ_DIFF_CONST;           // 1.0f
  damp_ground_rot = ADJ_GRND_BANK;  // 10.0f
  int crash = 1;                    // enabled
  GetIniVar ("PHYSICS", "enableCrashDetect", &crash);
  U_INT prop = (crash)?(VEH_D_CRASH):(0);
  mveh->SetOPT(prop);
	CPhysicModelAdj *phy = mveh->GetPHY();
  if (!phy) {             /// PHY file
    GetIniFloat ("PHYSICS", "adjustGroundBankingConst", &damp_ground_rot);
#ifdef _DEBUG
    DEBUGLOG ("CGearOpal start bankDamp%f",
      damp_ground_rot);
#endif
  } else {
    diffK = phy->Kdff;
    damp_ground_rot = phy->KrlR;
#ifdef _DEBUG
    DEBUGLOG ("CGearOpal start PHY : bankDamp%f \n\
              diffB%f",
      damp_ground_rot, diffK);
#endif
  }
  bad_pres_resis = 0.0f;
  side_whl_vel = rolling_force  = side_force      = 0.0f; 
  glf.type = opal::LOCAL_FORCE_AT_LOCAL_POS;
  gt_.type = opal::LOCAL_TORQUE;

  cur = 0;
  prv = 1;
}

//--------------------------------------------------------------------------------
//  Destroy this object
//--------------------------------------------------------------------------------
CGearOpal::~CGearOpal (void)
{
#ifdef _DEBUG
  DEBUGLOG ("CGearOpal::~CGearOpal");
#endif
}
//---------------------------------------------------------------------------------
//  Return Gear position and radius
//---------------------------------------------------------------------------------
void  CGearOpal::GetGearPosition(CVector &mp,double  &rad)
{ mp.x  = main_pos.x;
  mp.y  = main_pos.y;
  mp.z  = main_pos.z;
  rad   = Radius;
  return;
}
//---------------------------------------------------------------------------------
//  Compute wheel shape and contact point relative to COG
//  NOTE: mPos.y is the contact point relative to visual center VC
//	(a negative number as it is below the VC while the wheel radius is positive, 
//	so the wheel axis is at 
//        (mPos.y + wheel Radius) (in meter)
//	main_pos: Is the wheel axis position relative to the Center of Gravity (GOG)
//						This is the point where Opal Forces are applied to the wheel
//---------------------------------------------------------------------------------
void CGearOpal::InitJoint (char type, CGroundSuspension *susp)
{ opal::Solid *phyM = (opal::Solid*)mveh->GetPhyModel();
	CVector cog = mveh->wgh->svh_cofg;
  //--- Compute wheel coordinates relative to CG -----------
  double gx = gearData->bPos.x - cog.x;
  double gy = gearData->bPos.z - cog.z;
  double gz = gearData->bPos.y - cog.y;
  //--- Save it --------------------------------------------
  gearData->gPos.x  = gx;
  gearData->gPos.y  = gy;
  gearData->gPos.z  = gz;
  //--- Compute in metre -----------------------------------
  double tx   = FN_METRE_FROM_FEET(gx);
  double ty   = FN_METRE_FROM_FEET(gy);
  double tz   = FN_METRE_FROM_FEET(gz);
  //---- Wheel radius --------------------------------------
  double wradius = FN_METRE_FROM_FEET (gearData->whrd);
  double axeAGL  = tz + wradius;
  Radius      = wradius;
  //--- Set wheel axis position AGL-------------------------
  main_pos.x = tx;                           
  main_pos.y = ty;                           
  main_pos.z = (axeAGL);              
  //--------------------------------------------------------
  //  JS:  We place the sphere at the wheel axis
  //  all dimensions are in meters
  //--------------------------------------------------------
  box.contactGroup = 1;
  box.radius  = wradius;
	box.offset.makeTranslation(tx,ty,axeAGL);
  box.material.hardness   = opal::real(0.9f);
  box.material.bounciness = opal::real(0.0f);
  box.material.friction   = opal::real(0.004f);
  box.setUserData(this,SHAPE_GEAR);
  phyM->addShape (box);
  //----Set position of force application ----------------
  glf.pos   = main_pos;
  //------------------------------------------------------
  //  Compute mass repartition coefficient
  //------------------------------------------------------
  double fac  = (gearData->ster)?(1):(0.5f);
  double base =  gearData->wheel_base;
  cMass       = (fac * (base - fabs (main_pos.y)) / base); 
  //------------------------------------------------------
  //  Save the moment lever if this is a steering wheel
  //  Vm Distances are relative to CG in meters
  //------------------------------------------------------
  if (gearData->ster)
  { CVector Vm (0,ty,0);
    susp->StoreGearVM(Vm,cMass);
  }
  return;
}

//==============================================================================
//  JS: Compute wheel above ground level (wagl) 
//  wagl = bodyAGL + localPos
//  gPos is the wheel contact point relative to the CoG (in feet)
//  bagl = altitude - ground altitude  => Body above ground level
//  wagl = bagl + ROT(gPos) is the wheel contact AGL after rotation
//  When wagl is slightly negative, the wheel is on ground
//==============================================================================
char CGearOpal::GCompression(char pp)
{ //-- Apply aircraft rotation to contact point --------------------
  double  grd = globals->tcm->GetGroundAltitude();  
  SVector   V;                                 // Local rotated coordinates
  double   *M = mveh->GetROTM();               // Rotation matrix
  gearData->gPos.MultMatGL(M,V);               // Rotate
  //---Wheel AGL is local contact + body AGL -----------------------
  double bagl = mveh->GetAltitude() - grd;
  double wagl = bagl + V.z;										// Compute wheel AGL
	if (wagl < 0.1)	 wagl	= 0;
	gearData->wagl = wagl;											// Save it
  susp->SetWheelAGL(wagl);
  //----Check that wheel is just above ground ----------------------
  //U_INT fr    = globals->sit->GetFrameNo();//082911
  //TRACE("%06d: WHeel GRND=%.04f bagl=%.04f wagl=%.04f %s",fr,grd,bagl,wagl,gearData->susp_name);
  if (wagl > 0.5)   return 0;
  //----Compute impact power in pound per feet per sec ------------
	double vert = vWhlVelVec.z;
  double mass = mveh->wgh->GetTotalMassInLbs();
	if (fabs(vert) < 0.1)		vert = 0; 
  mass       *= MetresToFeet (vert);
  gearData->imPW = mass;                      // Impact on wheel
  double lim  = gearData->powL;
  if (lim && (mass > lim) )		return susp->GearShock(10);   // Gear destroyed
  //----Check for compression -(in feet) --------------------------
  double lim1 = -gearData->maxC;
  ////
  //#ifndef _DEBUG	
  //{	FILE *fp_debug;
	 // if(!(fp_debug = fopen("__DDEBUG_g.txt", "a")) == NULL)
	 // {
		//  int test = 0;
		//  fprintf(fp_debug, "%f>%f %f<%f\n", mass, lim, wagl, lim1);
		//  fclose(fp_debug); 
  //}	}
  //#endif
  
  if (wagl < lim1)                        return susp->GearShock(1);    // Gear impaired
	//--- Check for ground transition ------------------------------
  if (pp == 0)  susp->PlayTire(0);
  return 1;
}
//-----------------------------------------------------------------------
//  Repair the wheel
//-----------------------------------------------------------------------
void CGearOpal::Repair()
{ gearData->shok = 0;
}
//-----------------------------------------------------------------------
//* calculates the gear strut compression speed
//* vWhlVelVec = instantaneous velocity vector in body frame (m/s)
//* compression speed in body frame = Z-component vWhlVelVec.z
//-----------------------------------------------------------------------
void CGearOpal::GComprV_Timeslice (void)
{	opal::Solid *phyM = (opal::Solid*)mveh->GetPhyModel();
  //! current state becomes previous state
  prv = cur;
  cur = (cur + 1) & 1;
  local_velocity = phyM->getLocalLinearVelAtLocalPos (main_pos); // m/s RH
  if (_isnan(body_velocity.z)) gtfo("Physical engine HS");                // JS
  vb[cur].x = double(local_velocity.x);
  vb[cur].y = double(local_velocity.y);
  vb[cur].z = double(local_velocity.z);

  vWhlVelVec = vb[cur]; // m/s RH

  glf.vec.set (0.0, 0.0, 0.0);
  gt_.vec.set (0.0, 0.0, 0.0);
}

//-----------------------------------------------------------------------
//* compute the vertical force on the wheel using square-law damping
//* vLocalForce = local forces applied to the aircraft in body frame
//* body frame : in lbs
//-----------------------------------------------------------------------
void CGearOpal::VtForce_Timeslice (float dT)
{	double ay = (vb[cur].y - vb[prv].y) / dT;
  if (fabs(ay) < 0.01)	ay = 0;
	double az = (vb[cur].z - vb[prv].z) / dT;
	if (fabs(az) < 0.01)  az = 0;
	double ax = (vb[cur].x - vb[prv].x) / dT;
	if (fabs(ax) < 0.01)  ax = 0;

	vLocalForce.x = ax;
	vLocalForce.y = ay;
	vLocalForce.z = az;

  double mass  = mveh->GetMassInKgs();
  vLocalForce.Times(mass * cMass);    // was * gearData->damR);

  glf.vec.z = opal::real(vLocalForce.z); // 
  glf.vec.y = opal::real(vLocalForce.y); // 
  glf.vec.x = opal::real(vLocalForce.x); // 

  glf.duration = opal::real(dT);
}

//-----------------------------------------------------------------------
//* compute the steering forces on the wheel with yaw
//* vLocalForce = local forces applied to the aircraft in body frame
//* body frame : in lbs
//-----------------------------------------------------------------------
void CGearOpal::DirectionForce_Timeslice (float dT)
{ opal::Solid *phyM = (opal::Solid*)mveh->GetPhyModel();
  float steer_angle_rad = 0.0f;
  // rudder mixer sends angle value from -1 to 1

  if (gearData->ster) {
		float kfr = 0.5 * (1 + gearData->deflect);
		gearData->kframe	= kfr;
    //--JS fix bugs: mStr is in degre and should be inside, not outside of DegToRad
    steer_angle_rad		= DegToRad (gearData->deflect * gearData->mStr); // JS * gearData->mStr * 5.0f);
  }
  double wvel = fabs(vWhlVelVec.y);					// JS: wheel velocity
  susp->MoveWheelTo(wvel,dT);               // JS: Interface to wheel
  side_whl_vel    = vWhlVelVec.x;
	speed						= vWhlVelVec.y + fabs (vWhlVelVec.x);
  //--- process steering wheel ----------------------------------
  if (gearData->ster) {
    double base = gearData->wheel_base;
    // JS: Correction for bug in excessive rate turn ---------------------
    double turn_rate   = speed * tan (steer_angle_rad) * base;      
    double lat_acc     = speed * (turn_rate + mveh->GetDifBrake());
    side_force         = lat_acc * mveh->GetMassInKgs() * gearData->stbl;
		//---TRACE("GEAR turn %.4f",gearData->deflect);
    //-- turn nose wheel ----------------------------------------
    susp->TurnWheelTo(gearData->kframe);
    mveh->RazDifBrake();
  }
	//--- Create torque with lateral force (only nose wheel)-------
  gt_.vec.z = (opal::real)(side_force); /// K)      /* mass on a tricycle wheel*/;
  //--- Very basic lateral bank simulation in steep turns -------
  gt_.vec.y = gt_.vec.z / (gearData->damR * gearData->maxC * damp_ground_rot); //
  //-------------------------------------------------------------
  gt_.duration = static_cast<opal::real> (dT);
  phyM->addForce (gt_);
  local_velocity.x = static_cast<opal::real> (0.0f);
	return;
}
//-------------------------------------------------------------------------
//  Display gear parameters
//-------------------------------------------------------------------------
void CGearOpal::Probe(CFuiCanva *cnv)
{ cnv->AddText(1,1,"OnGr: %d",gearData->onGd);
	cnv->AddText(1,1,"sABS: %d",gearData->sABS);
	cnv->AddText(1,1,"wagl: %.4f(ft)",gearData->wagl);
	cnv->AddText(1,1,"powL: %.4f",gearData->powL);
	cnv->AddText(1,1,"imPW: %.4f(flbs)",gearData->imPW);
	cnv->AddText(1,1,"dflect:%.4f",gearData->deflect );
	cnv->AddText(1,1,"amor: %.4f",gearData->amor);
	return;
}
//-------------------------------------------------------------------------
//  Display brake parameters
//-------------------------------------------------------------------------
void CGearOpal::ProbeBrake(CFuiCanva *cnv)
{ //---speed -----------------------------
  cnv->AddText(1,1,"sped:  %.04f",speed);
	//---Break force -----------------------------
  cnv->AddText(1,1,"bdif:  %.04f",mveh->GetDifBrake());
  //--- Acceleration ----------------------------
  cnv->AddText(1,1,"sideF:  %.04f",side_force);
	//--- Table ----------------------------
  cnv->AddText(1,1,"stbl:  %.04f",gearData->stbl);
  //--- Torque ----------------------------
  cnv->AddText(1,1,"Torq:  %.04f",gt_.vec.z);
	//---Rolling force ----------------------------
  cnv->AddText(1,1,"vel.y:  %.04f",local_velocity.y);
  //---Local force -----------------------------
  cnv->AddText(1,1,"locF:  %.04f",vLocalForce.y);
	//---------------------------------------------
  //---Final force ------------------------------
  cnv->AddText(1,1,"glf.x %.4f",glf.vec.x);
  cnv->AddText(1,1,"glf.y %.4f",glf.vec.y);
  cnv->AddText(1,1,"glf.z %.4f",glf.vec.z);
  return;
}

//---------------------------------------------------------------
//	JS: Replace brake time slice with following computation
//
//	Compute breaking force based on the following data
//	Brake force should stop the aircraft at full load
//	from approach speed to 0 Kts, in a given distance D
//
//	-Load (Mass) is taken from the weigh manager
//	-Approach speed (kts) and brake distance (feet) are defined
//	 in SVH  file.
//	Typically for instance a CESNA 172 would stop from 72 KTS
//	in 1200 feets.  Those data are usually available in aircraft
//	pilot manual
//  Given those data, we can compute brake force acceleration,
//	(a negative accelerattion) as
//	
//	acc = 1/2((asp*asp)/D); where asp = approach speed in feet/sec
//  (This is a constant once asp and D are defined)
//
//	Knowing the aircraft mass M, we can compute the brake force 
//	opposite the forward movement as F=M.acc (lbs/feet/sec)
//---------------------------------------------------------------
//	The brake force is modulated by the force applied on pedal
//	and an optional brake table for better modulation
//---------------------------------------------------------------
//	Brak_diff is a lateral speed generated by different brake
//	presure and is applied with turn rate to make aircraft turns
//---------------------------------------------------------------
void CGearOpal::BrakeForce(float dT)
{	opal::Solid *phyM = (opal::Solid*)mveh->GetPhyModel();
	//--- pedal force -----------------------------------------
	char  side  = gearData->Side;
  float btbl  = gearData->btbl;                 // Gear coefficient
  float force = mveh->GetBrakeForce(side) * btbl * 1.2f;
	//--- compute brake force ---------------------------------
	CSimulatedVehicle *svh = mveh->svh;
	double ac = svh->GetBrakeAcceleration();			// acceleration
	double ms = cMass * mveh->GetMassInLbs();			// Mass in pounds
	double bf = -(ac) * ms;												// foot-pound-sec
	//--- Is proportional to pedal ----------------------------
	bf *= force;
	//--- Check velocity -------------------------------------
	double lv  = local_velocity.y;				
	if (fabs(lv) < 0.001) lv = 0;
	if (lv <= 0)	bf = 0;
	gearData->brakF	= bf;													// Brake force to apply
		//--- Compute a torque value to add to steering one -----
	mveh->AddDifBrake(bf * diffK * gearData->bPos.x / ms);
	return;
}
//-------------------------------------------------------------------------
//* transform the forces back to the inertial frame : in lbs
//	-Damp vertical force to simulate suspension shock
//	damR is set to the percentage of shock absortion
//-------------------------------------------------------------------------
void CGearOpal::GearL2B_Timeslice (void)
{ opal::Solid *phyM = (opal::Solid*)mveh->GetPhyModel();
  /// \to do ? transform the forces back to the body frame
	//--- compute opposite vertical force -----------------
  gearData->amor  = (glf.vec.z) * gearData->damR;
	glf.vec.z	      = -gearData->amor;
	//--- Apply anti skid if request ----------------------
	if (gearData->sABS)	glf.vec.x = -glf.vec.x;
	//--- Apply brake -------------------------------------
	BrakeForce(0);
	glf.vec.y	= gearData->brakF;
  phyM->addForce (glf); // 
  //--JS NOTE: According to ODE doc, it is not  
	//	necessary to set velocity if forces are correctly
	//	set.  However, without the following statement
	//	the aircraft direction is not good.
	//	Must be further investigated.  Why setting
	//	lateral force and torque is not enough?
  phyM->setLocalLinearVel (local_velocity); //
}

//-------------------------------------------------------------------------
//* compute moment in body coordinates : in lbs.ft
//-------------------------------------------------------------------------
void CGearOpal::VtMoment_Timeslice (void)
{
}
//---------------------------------------------------------------------------------
//  Reset crash conditions
//---------------------------------------------------------------------------------
bool CGearOpal::ResetCrash()
{ vLocalForce.Set (0.0, 0.0, 0.0);
  return true;
}
//---------------------------------------------------------------------------------
//  Reset all forces
//---------------------------------------------------------------------------------
void CGearOpal::ResetForce()
{ vb[0].Raz();
  vb[1].Raz();
  ab[0].Raz();
  ab[1].Raz();
	glf.vec.set(0,0,0);
}

///< force in Newton
const SVector& CGearOpal::GetBodyGearForce_ISU  (void)
{
  vForce_ISU.x = vForce.x * LBS_TO_NEWTON;
  vForce_ISU.y = vForce.y * LBS_TO_NEWTON;
  vForce_ISU.z = vForce.z * LBS_TO_NEWTON;
  return vForce_ISU;
} 
   
///< moment in Kg.m
const SVector& CGearOpal::GetBodyGearMoment_ISU (void)
{
  return CGear::GetBodyGearMoment_ISU ();
}
//==============================================================================
//  class CTailGearOpal
//  brief     
//  detailed
//==============================================================================
CTailGearOpal::CTailGearOpal (CVehicleObject *v,CSuspension *s) : CGearOpal (v,s)
{ 
}

CTailGearOpal::~CTailGearOpal (void)
{
#ifdef _DEBUG
  DEBUGLOG ("CTailGearOpal::~CGearOpal");
#endif
}

//------------------------------------------------------------------
//  Compute wheel above ground level 
//   When wagl is negative, the wheel is on ground
//------------------------------------------------------------------
char CTailGearOpal::GCompression  (char pp)
{ 
  return CGearOpal::GCompression(pp);
}

//------------------------------------------------------------------
///* compute the vertical force on the wheel using square-law damping
///* vLocalForce = local forces applied to the aircraft in body frame
///* body frame : in lbs
//------------------------------------------------------------------
void CTailGearOpal::VtForce_Timeslice (float dT)
{
  CGearOpal::VtForce_Timeslice (dT);
}
//------------------------------------------------------------------
///* compute the steering forces on the wheel with yaw
///* vLocalForce = local forces applied to the aircraft in body frame
///* body frame : in lbs
//  JS NOTE: Is that an additional force to the actual steeirng by the pilot
//            if such, it must be added, and not replacing the
//            steering
//------------------------------------------------------------------
void CTailGearOpal::DirectionForce_Timeslice (float dT)
{
  CGearOpal::DirectionForce_Timeslice (dT);
}

//------------------------------------------------------------------
///* transform the forces back to the inertial frame : in lbs
//------------------------------------------------------------------
void CTailGearOpal::GearL2B_Timeslice (void)
{
  CGearOpal::GearL2B_Timeslice ();
}

//------------------------------------------------------------------
///* compute moment in body coordinates : in lbs.ft
//------------------------------------------------------------------
void CTailGearOpal::VtMoment_Timeslice (void)
{
}

//============================================================================================
/*!
 * COpalSuspension
 */
//============================================================================================
COpalSuspension::COpalSuspension (CVehicleObject *v,char *name, CWeightManager *wgh, char tps)
: CSuspension(v,name,wgh,tps)
{ 
}

//------------------------------------------------------------------------
//  Destroy object
//------------------------------------------------------------------------
COpalSuspension::~COpalSuspension (void)
{
#ifdef _DEBUG
  DEBUGLOG ("COpalSuspension::~COpalSuspension");
#endif
}
void COpalSuspension::Debug (void)
{
#ifdef _DEBUG_suspension	
	FILE *fp_debug;
	if((fp_debug = fopen("__DDEBUG_suspension.txt", "a")) != NULL)
	{
      std::vector<std::string>::iterator it;
      for (it = gear_data.vfx_.begin (); it != gear_data.vfx_.end (); it++) {
        fprintf(fp_debug, "COpalSuspension::Debug %s\n", (*it).c_str ());
      }
      fclose(fp_debug); 
	}
#endif
}

//------------------------------------------------------------------------
//  Time slice: Update 
//------------------------------------------------------------------------
void COpalSuspension::Timeslice (float dT)
{
  // 1) aerodynamic purpose
  // get the gear position relative to the actual CG
  //SVector *actualCG = wgm->wb.GetCGOffset ();  ///< LH feet // removed for it sets 0,0,0
  SVector actualCG_;
  mveh->wgh->GetVisualCG (actualCG_);  ///< RH feet
  SVector actualCG; 
  actualCG.x = -actualCG_.x; actualCG.y = actualCG_.z; actualCG.z = actualCG_.y; // RH->LH
  // gets mPos as the 3D gear position
  gear->GearLoc_Timeslice (&actualCG, gear_data.bPos);                     ///< mPos = tire contact feet
  // 2) the neW gear location is transformed in the actual body coordinates
  // (the aircraft is not alWays Well levelled in all its axes)
  // gets WPos
  gear->GearB2L_Timeslice ();
  // 3) get the CG AGL
  // 
  // get the gear compression value
  // and update the WOW flag (Weight-on-heels)
  gear_data.onGd = gear->GCompression(gear_data.onGd);
  // 4)
  // compute ground physics only if needed
  if (IsOnGround ()) { // weight on wheels is verified

    // 5) compute gear compression velocity
    //
      gear->GComprV_Timeslice ();

    // 6) compute force and moment from the wheel
    //
    gear->VtForce_Timeslice (dT);
    gear->DirectionForce_Timeslice (dT); 


    // 8) compute force and moment
    gear->GearL2B_Timeslice ();

  }
  else {
    gear->ResetValues ();
  }
}
//====================================================================================
/*!
 * COpalGroundSuspension
 * Based upon OPAL-ODE library
 * see copyright
 */
//=====================================================================================
COpalGroundSuspension::COpalGroundSuspension (CVehicleObject *v,char* name,  CWeightManager *wgh)
:CGroundSuspension(v,name,wgh)
//--- Init specific parameters -------------------------------------
{
#ifdef _DEBUG
  DEBUGLOG ("Starting COpalGroundSuspension");
#endif
  max_wheel_height_backup = 0.0;
}
//-----------------------------------------------------------------
//  Destroy this object
//-----------------------------------------------------------------
COpalGroundSuspension::~COpalGroundSuspension (void)
{ 
#ifdef _DEBUG
  DEBUGLOG ("COpalGroundSuspension::~COpalGroundSuspension");
#endif
}
//-----------------------------------------------------------------
//  Read All Tags
//-----------------------------------------------------------------
int COpalGroundSuspension::Read (SStream *stream, Tag tag)
{ 
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'rMas':
    // rated mass (slugs)
    ReadFloat (&rMas, stream);
    whm->whl_rmas = rMas;
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
    char susp_[64], susp_type[8], susp_name[56];
    ReadString (susp_, 64, stream);
#ifdef _DEBUG
    DEBUGLOG ("COpalGroundSuspension::Read %s", susp_);
#endif
    if (sscanf (susp_, "%s %s", susp_type, susp_name) == 2) {
      if (!strcmp (susp_type, "whel")) {
        //////////////////////////////////////////////////
        CSuspension *susp = new COpalSuspension (mveh,susp_name, whm, type); //  default
        //////////////////////////////////////////////////
        ReadFrom (susp, stream);
        whl_susp.push_back (susp);
        return TAG_READ;
      }
      if (!strcmp (susp_type, "bmpr")) {
        CWhl *bump = new CBumper     (susp_name, whm, type);
        ReadFrom (bump, stream);
        whl_bump.push_back (bump);
        return TAG_READ;
      } 
      WARNINGLOG ("COpalGroundSuspension::Read : bad susp type");
      return TAG_READ;
    }
  }

    // Tag was not processed by this object, it is unrecognized
  WARNINGLOG ("COpalGroundSuspension::Read : Unrecognized tag <%s>", TagToString(tag));
  return TAG_IGNORED;
}
//------------------------------------------------------------------------
//  All parameters are read.  Finalize
//-------------------------------------------------------------------------
void COpalGroundSuspension::ReadFinished (void)
{ 
#ifdef _DEBUG
  DEBUGLOG ("COpalGroundSuspension::ReadFinished");
#endif
  CGroundSuspension::ReadFinished ();
  double base = FN_METRE_FROM_FEET(wheel_base);
  std::vector<CSuspension *>::const_iterator it_whel; 
  for (it_whel = whl_susp.begin (); it_whel != whl_susp.end (); it_whel++) {
    CSuspension *ssp = (CSuspension *)(*it_whel);
    ssp->SetWheelBase(base);
    ssp->InitGearJoint (type,this);
  }

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
 return;
}

//------------------------------------------------------------------------
//  JS to LC: Removed brake force from wheel definition as it is
//          only used in Suspension TimeSlice. 
//-------------------------------------------------------------------------
void COpalGroundSuspension::Timeslice (float dT)
{
  /// very important ! be sure to compute only during
  /// engine & aerodynamics cycle
  if (!globals->simulation) return;
  nWonG   = 0;      // No wheels on ground
  SumGearMoments.x = 0.0;
  SumGearMoments.y = 0.0;
  SumGearMoments.z = 0.0;
  //--- Compute velocity in Miles per Hours ---------
  double vt = (mveh->GetBodyVelocityVector ())->z;
  double velocity = NMILE_PER_METRE_SEC(vt);
  float fstbl = mstbl->Lookup (velocity); // 1.0f;
  float fbtbl = mbtbl->Lookup (velocity); // 1.0f;

  std::vector<CSuspension *>::const_iterator it_whel;
  //-- Check for gear position ----------------------
  if (mveh->lod->AreGearRetracted())
  { max_wheel_height_backup = max_wheel_height;
    max_wheel_height = 0.0;
  }
  if (mveh->lod->AreGearDown())
  { max_wheel_height = max_wheel_height_backup;
  }
  //
  for (it_whel = whl_susp.begin (); it_whel != whl_susp.end (); it_whel++) {
    // is it a steering wheel ?
    CSuspension *ssp = (CSuspension*)(*it_whel);
		SGearData *gdt	= ssp->GetGearData();
		gdt->stbl				= fstbl;
		gdt->btbl				= fbtbl;
    if (mveh->lod->AreGearDown()) { /// this is the completely extended gear position
      if (ssp->IsSteerWheel ()) mveh->GetGearChannel(gdt);
      // ... and finally timeslice
      ssp->Timeslice (dT);
      if (ssp->IsOnGround()) nWonG++;
    }
  }

  /// All the computation below is LH
  // verify if this test is useful 
  //
  // add mass moment related to main gear
  // JS NOTE:  The main_pos is in fact the distance between the Cg and the steering
  //  wheel.   
  //  On a tricyle, the steering is in positive direction while on a Tail Dragger
  //  the steering is in negative direction.
  //  So I made the following modifications
  //  The steering distance (in meters) is computed in the 
  //  CGroundSuspension::InitJoint() when wheels positions are computed
  //  This vector is stored into mainVM and the massCF is the coeeficent
  //  that modulate the mass supported by the wheel.  
  //  Also, the mass_force is modified so that the force applied is in the
  //  vertical direction (Z) and is negative.
  //

  { 
    CVector mass_moment;
    CVector mass_force (0, 0, -mveh->GetMassInKgs() * GRAVITY_MTS * massCF);   //, 0.0);
    CVector mass_pos, main_gear;
    //  main_gear.Set (0.0, FN_METRE_FROM_FEET (-max_wheel_height), FN_METRE_FROM_FEET (max_gear));
    //  mass_pos = *mveh->svh->GetNewCG_ISU () - main_gear;
    //  VectorCrossProduct (mass_moment, mass_pos, mass_force);
    VectorCrossProduct (mass_moment, mainVM, mass_force);
    /// add gear moment to the CG moment
    SumGearMoments = VectorSum (SumGearMoments, mass_moment);
    #ifdef _DEBUG_suspension	
    {   FILE *fp_debug;
      if((fp_debug = fopen("__DDEBUG_suspension.txt", "a")) != NULL)
      {
            fprintf(fp_debug, "---------------------------------------------------------\n");
            fprintf(fp_debug, "COpalGroundSuspension::Timeslice SumF(%f %f %f) SumM(%f %f %f)\n",
              SumGearForces.x,  SumGearForces.y,  SumGearForces.z,
              SumGearMoments.x, SumGearMoments.y, SumGearMoments.z
             );
            fprintf(fp_debug, " cg(%f %f %f) mg(%f %f %f) mp(%f %f %f)\n mf(%f %f %f) mm(%f %f %f) (mwh%f Mg%f mg%f)\n",
              globals->pln->svh->GetNewCG_ISU ()->x,  globals->sit->uVeh->svh->GetNewCG_ISU ()->y,  globals->sit->uVeh->svh->GetNewCG_ISU ()->z,
              main_gear.x, main_gear.y, main_gear.z,
              mass_pos.x, mass_pos.y, mass_pos.z,
              mass_force.x, mass_force.y, mass_force.z,
              mass_moment.x, mass_moment.y, mass_moment.z,
              max_wheel_height, max_gear, min_gear
             );
            fprintf(fp_debug, "---------------------------------------------------------\n");
            fclose(fp_debug); 
    }    }
    #endif
  }
}
//=======END of FILE =================================================================
