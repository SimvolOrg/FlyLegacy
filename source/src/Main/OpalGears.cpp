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
#include "../Include/TerrainUnits.h"
using namespace std;
//====================================================================================

//=====================================================================
//  CGearOpal
//=====================================================================

CGearOpal::CGearOpal (CVehicleObject *v,CSuspension *s) : CGear (v,s)
{ int crash = 1;                    // enabled
  GetIniVar ("PHYSICS", "enableCrashDetect", &crash);
  U_INT prop = (crash)?(VEH_D_CRASH):(0);
  mveh->SetOPT(prop);
	CPhysicModelAdj *phy = mveh->GetPHY();
  bad_pres_resis = 0.0f;
  banking  = 0; 
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
//	main_pos: Is the wheel axis position relative to the Center of Gravity (cog)
//						This is the point where Opal Forces are applied to the wheel
//						Distances are in meters
//	cog				Is the center of mass position relative to the aircraft origin (in feet)
//	bpos			Is the wheel center relative to aircraft origin (in feet)
//---------------------------------------------------------------------------------
void CGearOpal::InitJoint (char type, CGroundSuspension *susp)
{ //if (!mveh->IsUserPlan())		return;
	opal::Solid *phyM = (opal::Solid*)mveh->GetPhyModel();
	CVector cog = mveh->wgh.svh_cofg;
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
  double wradius	= FN_METRE_FROM_FEET (gearData->whrd);
  double axeAGL		= tz + wradius;
  Radius					=	wradius;
  //--- Set wheel axis position AGL-------------------------
  main_pos.x = tx;                           
  main_pos.y = ty;                           
  main_pos.z = (axeAGL);
	//--- Compute sighed lateral distance to CG --------------
	gearData->xDist		= tx;
  //--------------------------------------------------------
  //  JS:  We place the sphere at the wheel axis
  //  all dimensions are in meters
  //--------------------------------------------------------
  box.contactGroup	= 2;
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
	//	wheel_base is in meters
  //------------------------------------------------------
  double fac  = (gearData->ster)?(1):(0.5f);
  double base =  gearData->wheel_base;
  cMass       = (fac * (base - fabs (main_pos.y)) / base); 
  //------------------------------------------------------
  //  Save the moment lever if this is a steering wheel
  //  Vm Distances are relative to CG in meters
  //------------------------------------------------------
	gearData->masR = cMass;
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
  //TRACE("%06d: WHeel GRND=%.04f bagl=%.04f wagl=%.04f %s",globals->sit->GetFrameNo(),grd,bagl,wagl,gearData->susp_name);
  if (wagl > 0.5)   return 0;
  //----Compute impact power in pound per feet per sec ------------
	double vert = vWhlVelVec.z;									// Meter per second
  double mass = mveh->wgh.GetTotalMassInLbs();
	if (fabs(vert) < 0.1)		vert = 0; 
  mass       *= FN_FEET_FROM_METER(vert);
  gearData->imPW = mass;                      // Impact on wheel
  double lim  = gearData->powL;
  if (lim && (mass > lim) )		return susp->GearShock(10);   // Gear destroyed
  //----Check for compression -(in feet) --------------------------
  double lim1 = -gearData->maxC;
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
	if (0 == phyM)		return;
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
//	1)	We compute the Angular velocity of turning:
//			angv = f(speed,turn radius)
//			turn radius is a function of the gear deflection and 
//			distance between lateral wheels and nose wheel
//	2) Additional angle is added from differential braking force
//			add = f(DifBraking, steerCap) wherer sterCap is the maximum
//			braking angle of the nose wheel
//	3) Banking is simulated by adding a rolling force proportional
//			to speed and turning angle so the aircarft may tip over if
//			turning too much in high speed
//	4) Animations are performed on 3D model
//-----------------------------------------------------------------------
void CGearOpal::DirectionForce_Timeslice (float dT)
{ opal::Solid *phyM = (opal::Solid*)mveh->GetPhyModel();
	if (0 == phyM)		return;
  double angr = 0.0f;
  susp->MoveWheelTo(vWhlVelVec.y,dT);								// Animated 3D model wheel
	speed				= vWhlVelVec.y;												// linear velocity in forward direction;
	//--- Process nose gear ------------------------------------------
  double   angv   = 0;											// Angular velovcity
  if (gearData->ster) {
		//--- Compute keyframe for 3D model -----------------------------
		float kfr = 0.5 * (1 + gearData->deflect);
		gearData->kframe	= kfr;
    //--- Get turning angle from steering ---------------------------
    angr		= DegToRad (gearData->deflect * gearData->mStr);
		//--- Add steering effect from differential brake ---------------
		double dif_brk    = mveh->GetDifBrake() * gearData->mgsp->GetDifBraking();
		double amp				= gearData->mStr * dif_brk;  
		angr							= WrapPiPi(angr + DegToRad(amp));
		//--- process steering wheel ----------------------------------
    double base		= gearData->wheel_base;			// In meter
		double sind		= sin(angr);			// Sinus
		double rturn	= (fabs(sind) < 0.0001)?(0):(base / sind); // Turn radius
		angv		= (rturn == 0)?(0):(speed / rturn);
		angv		= RadToDeg(angv) * gearData->stbl;
		//--- Simulate banking on turn ---------------------------------
    double turn_rate  = speed * tan (angr) * base;      
    double lat_acc    = speed * turn_rate;
    banking						= lat_acc * mveh->GetMassInKgs() * gearData->mgsp->GetBankCoef();
		//---TRACE("GEAR turn %.4f",gearData->deflect);
    //-- turn nose wheel in 3D model --------------------------------
    susp->TurnWheelTo(gearData->kframe);
		//---Create banking force ---------------------------------------
		gt_.vec.x = 0;
		gt_.vec.z = 0;
		gt_.vec.y = banking;
		//--------------------------------------------------------------
	  opal::Vec3r vec = phyM->getLocalAngularVel();
		vec.z = angv;
		phyM->setLocalAngularVel(vec);
		local_velocity.x = static_cast<opal::real> (0.0f);
	  //-------------------------------------------------------------
		gt_.duration = static_cast<opal::real> (dT);
		phyM->addForce (gt_);
		mveh->RazDifBrake();

	}
	return;
}

//-------------------------------------------------------------------------
//  Display gear parameters
//-------------------------------------------------------------------------
void CGearOpal::Probe(CFuiCanva *cnv)
{ cnv->AddText(1,1,"OnGr-ABS: %d,%d",gearData->onGd,gearData->sABS);
	cnv->AddText(1,1,"wagl: %.6lf(ft)",gearData->wagl);
	cnv->AddText(1,1,"powL: %.6f",gearData->powL);
	cnv->AddText(1,1,"imPW: %.6lf(flbs)",gearData->imPW);
	cnv->AddText(1,1,"Banking: %.6lf",banking);
	cnv->AddText(1,1,"velo:   %.6lf",local_velocity.y);
	cnv->AddText(1,1,"Swing:  %.6lf",gearData->swing);
	cnv->AddText(1,1,"Torque: %.6lf",mveh->GetDifBrake());
	cnv->AddText(1,1,"brakF:  %.6lf", gearData->brakF);
	cnv->AddText(1,1,"turn kf: %.6lf",gearData->kframe);
	cnv->AddText(1,1,"amor kf: %.6lf",gearData->sframe);
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
  cnv->AddText(1,1,"Bank:  %.04f",banking);
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
//  (acc is then a constant once asp and D are defined)
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
{	//--- pedal force -------------------------------------------
	char  side  = gearData->Side;
  float btbl  = gearData->btbl;                 // Speed coefficient
  float pedal = mveh->GetBrakeForce(side) * btbl;
	//--- Compute total brake force -----------------------------
	CSimulatedVehicle *svh = &mveh->svh;
	double ac = svh->GetBrakeAcceleration();	// Brake acceleration (m/s²)
	double ms = mveh->GetMassInKgs();					// Mass supported
	double bf = -(ac) * ms;										// metre-kg-sec
	//--- Is proportional to pedal effort and repartition --------
	bf *= pedal * gearData->repBF * gearData->ampBK;					
	//--- Check velocity ----------------------------------------
	double lv  = local_velocity.y;				
	if (fabs(lv) < 0.01) {lv = 0; bf = 0;}
	if (lv < 0)	bf = -bf;
	//--- Save force to apply  ----------------------------------
	gearData->brakF	= (gearData->brak)?(bf):(0);		// Y Brake force to apply
	mveh->AddDifBrake(gearData->latK * pedal); // Torque
	return;
}

//-------------------------------------------------------------------------
//* transform the forces back to the inertial frame : in lbs
//	-Damp vertical force to simulate suspension shock
//	 damR is set to the percentage of shock absortion
//  -Vertical swing is simulating the vertical bouncing when taxiing on 
//	 ground due to ground irregularity.  Variable 'swing' deliver a damped
//	 sinusoid value in [-1,+1].  Thus we add a vertical force depending
//	 on this coefficient and the aircraft mass. Acceleration coefficient
//	 seems to be ok at 3 feet/ sec².  When more force is applied, the
//	 aircraft may jerk on ground and loose control.  This is to be verified
//	 with other aircrafts.  We may have to put this coefficient as a parameter
//  -Anti skid is used to nullify the lateral force that may cause skidding
//	 during landing.
//-------------------------------------------------------------------------
void CGearOpal::GearL2B_Timeslice (void)
{ opal::Solid *phyM = (opal::Solid*)mveh->GetPhyModel();
	if (0 == phyM)	return;
  /// \to do ? transform the forces back to the body frame
	//--- Compute amortizer opposite vertical force -------
	double amor				= (glf.vec.z) * gearData->damR;
	glf.vec.z					= -amor;
	//--- Animate shock parts --(key 0 is full extended) --
	double swing			= gearData->swing;
	float  kfrm				= float(swing) * 0.5f + 0.5f;
	susp->AnimateShock(kfrm);
	gearData->sframe	= kfrm;
	//--- Apply vertical swing ----------------------------
	double ms				= cMass * mveh->GetMassInKgs();			// Mass in Kg
	glf.vec.z      += (gearData->mgsp->GetBumpForce() * ms * swing);
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
	//	lateral force is not enough?
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
COpalSuspension::COpalSuspension (CVehicleObject *v, CGroundSuspension *mgsp, char *nm, CWeightManager *wgh, char tps)
: CSuspension(v,mgsp,nm,tps)
{ amort.StartSin(1,5);
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
  mveh->wgh.GetVisualCG (actualCG_);  ///< RH feet
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
  gear_data.onGd = gear->GCompression(gear_data.onGd);
  // 4)
  // compute ground physics only if needed
  if (IsOnGround ()) 
	{ // weight on wheels is verified
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

//=======END of FILE =================================================================
