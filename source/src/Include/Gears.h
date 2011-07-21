/*
 * Gears.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2000-2004 Chris Wallace
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
 */

//================================================================================================
#ifndef GEARS_H
#define GEARS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//========================================================================================
class CGroundSuspension;
//========================================================================================
//  struct used to share data between classes
//  
//  all the WHL file data
//========================================================================================
struct SGearData {
  char onGd;                          // On Ground
  char shok;                          // Shock number
  char Side;                          // Side of wheel (left or right)
	char sABS;													// ABS brake on landing
  char susp_name[56];
  char long_[64];                    ///< -- Longitudinal Crash Part Name --
  CDamageModel oy_N;                 ///< -- Longitudinal Damage Entry --
  float stbl;                        ///< -- steering factor from data table
  float btbl;                        ///< -- braking factor from data table
  float powL;                        ///< -- Impact Power Limit (ft-lb/sec) (weight * velocity) --
  double imPW;                       // Impact power in ft-lb/sec
	double masR;											 // Mass repartition on this wheel
  double maxC;                       ///< -- max compression --
  CDamageModel oy_T;                 ///< -- Tire Damage Entry --
  // ntwt                            ///< -- Normal Tire Wear Rate vs Groundspeed (ktas) --
  // ltwt                            ///< -- Lateral Tire Wear Rate vs Lateral Groundspeed (ktas) --
  float boff;                        ///< -- Blowout Friction Factor --
  int   ster;                        ///< -- steerable wheel --
  float mStr;                        ///< -- max steer angle (deg) --
  float drag;                        ///< -- Drag coefficient
  float tirR;                        ///< -- Tire Radius (ft) --
  float rimR;                        ///< -- Rim Radius (ft) --
  float whrd;                         // Wheel radius (ft);
  CVector bPos;                       //< -- Tire Contact Point (model coodinates) --
  CVector gPos;                       //  Tire contact point (CG relative)
  float dvvd;                         //< -- Design Visual Vertical Displacement (ft) --
  std::vector<std::string> vfx_;      //< -- Visual Effects --
	///-------Vertical damping for ground contact ----------------------------
  double damR;                        ///< -- damping ratio --
	double damF;												// Damping factor
  ///-------Brake parameters -----------------------------------------------
	double	brakF;											// Brake force	
  float brak;                         // 0 not brake 1= brake
  float inpB;                         // Normalized input [0,1]
  float ffac;                         // Fudge factor
  float locV;                         // Local velocity
  float rBKF;                         // Rolling force
  float cBKF;                         // Brake coefficient
  float brkF;                         // Brake force from whl file
  float brakK;                        // Brake coefficient from PHY
  float diffK;                        // Brake differential from PHY
  ///----- sterring wheel data ---------------------------------------------
  float deflect;
  float scaled;
  float kframe;
  /// wheelBase
  float wheel_base;

  SGearData (void) {
    onGd              = 0;          // On ground
    shok              = 0;          // Number of shocks
		sABS							= 0;					// No ABS
    strcpy (susp_name,  "");
    strcpy (long_,      "");         // -- Longitudinal Crash Part Name --
    // oy_N'                         // -- Longitudinal Damage Entry --
    stbl              = 1.0f;        // -- steering factor from data table
    btbl              = 1.0f;        // -- braking factor from data table
    powL              = 0.0f;        // -- Impact Power Limit (ft-lb/sec) (weight * velocity) --
    // oy_T                          // -- Tire Damage Entry --
    // ntwt                          // -- Normal Tire Wear Rate vs Groundspeed (ktas) --
    // ltwt                          // -- Lateral Tire Wear Rate vs Lateral Groundspeed (ktas) --
    boff              = 0.0f;        // -- Blowout Friction Factor --
    ster              = 0;           // -- steerable wheel --
    mStr              = 0.0f;        // -- max steer angle (deg) --
    maxC              = 0.0f;        // -- max compression --
		masR							= 0.33f;
    damR              = 0.0f;        // -- damping ratio --
    tirR              = 0.0f;        // -- Tire Radius (ft) --
    rimR              = 0.0f;        // -- Rim Radius (ft) --
    drag              = 0.0f;
    dvvd              = 0.0f;        // -- Design Visual Vertical Displacement (ft) --
    // vfx_;                         // -- Visual Effects --
    // endf;                         //
    brak              = 0;            // -- has brake
    brkF              = 0.0f;        // -- brake force
    vfx_.clear ();
    deflect           = 0.0f;        // -- direction wheel deflection
    scaled            = 0.0f;        // -- direction wheel deflection scale
    kframe            = 0.5f;
    wheel_base        = 0.0f;        // -- distance from main gear and nose gear (metres)
  }
};
//================================================================================
//
// Ground Suspension and gears
//
// The WHL file contains the ground handling model parameters.
//

/*!
 * Based upon JSBSim code
 * see copyright below
 */

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

 Header:       FGLGear.h
 Author:       Jon S. Berndt
 Date started: 11/18/99

 ------------- Copyright (C) 1999  Jon S. Berndt (jsb@hal-pc.org) -------------

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free Software
 Foundation; either version 2 of the License, or (at your option) any later
 version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 details.

 You should have received a copy of the GNU General Public License along with
 this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 Place - Suite 330, Boston, MA  02111-1307, USA.

 Further information about the GNU General Public License can also be found on
 the world wide web at http://www.gnu.org.

HISTORY
--------------------------------------------------------------------------------
11/18/99   JSB   Created
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

class CWhl : public CSubsystem {
public:
  CWhl                             (void);
  ~CWhl                            (void) {;}

  ///< CStreamObject methods
  virtual int   Read                   (SStream *stream, Tag tag) = 0;
  virtual void  ReadFinished           (void)                     = 0;
  virtual void  Write                  (SStream *stream)          = 0;

  ///< CWhl methods
  virtual void  Timeslice                  (float dT)                  {;}
  virtual const SVector& GetForce          (void)          /* fake */  {vf.x  = vf.y  = vf.z  = 0.0; return vf;}
  virtual const SVector& GetMoment         (void)          /* fake */  {vf.x  = vf.y  = vf.z  = 0.0; return vf;}
  virtual const SVector& GetForce_ISU      (void)          /* fake */  {vf.x  = vf.y  = vf.z  = 0.0; return vf;}
  virtual const SVector& GetMoment_ISU     (void)          /* fake */  {vf.x  = vf.y  = vf.z  = 0.0; return vf;}
  virtual const SVector& GetBodyMoment     (void)          /* fake */  {vbf.x = vbf.y = vbf.z = 0.0; return vbf;}
  virtual const SVector& GetBodyForce_ISU  (void)          /* fake */  {vbf.x = vbf.y = vbf.z = 0.0; return vbf;}
  virtual const SVector& GetBodyMoment_ISU (void)          /* fake */  {vbf.x = vbf.y = vbf.z = 0.0; return vbf;}
  //--------------------------------------------------------------------------
  SVector vf, vbf;
};
//=====================================================================================
/*! \class
 * class separated from CSuspension for convenience .
 * Actually, all the gear calculation is placed here
 * while CSuspension is more intended as an interface
 */
//  JS:  Add a pointer to CSuspension for interface
//======================================================================================
class CGear {
//-----ATTRIBUTES ---------------------------------------------------
protected:
  CVehicleObject *mveh;               // Parent vehicle
  CSuspension    *susp;               // Associated suspension
  CVector vGearLoc2CG;               ///< in ft
  CVector vGearLoc2CG_ISU;           ///< in meters
  CVector vMoment;
  CVector vForce;
  CVector vMoment_ISU;
  CVector vForce_ISU;
  CVector vLocalMoment;
  CVector vLocalForce;               ///< vertical force on the wheel
  CVector vLocalMoment_ISU;
  CVector vLocalForce_ISU;
  //----------------------------------------------------------------
  SPosition WPos;                    ///< Wheel position relative to the body position in space
  CVector   vWhlVelVec;              ///< wheel velocity vector
  SGearData *gearData;

public :
  CGear (CVehicleObject *v,CSuspension *s);
  virtual ~CGear (void);
  ///< CGear methods
  ///< =============
  //--Gear position  and radius ------------------------
  virtual void GetGearPosition(CVector &mp,double  &rad) {;}
  /*! gear position according to the CG in ft : Timesliced */
  virtual void GearLoc_Timeslice (const SVector*, const SVector&);  
  /*! the Wheel position is transformed according to the ac body in the overall world
   *  therefore the reference should be 'local' : still in ft */
  virtual void GearB2L_Timeslice (void);
  /*! gear compression value calc in ft : Timesliced */
  virtual char GCompr__Timeslice (void) {return 0;}
  /*! gear compression velocity : Timesliced */
  virtual void GComprV_Timeslice (void) {};
  /*! compute the vector force on the wheel in pound-force lbf */
  virtual void VtForce_Timeslice (float dT) {;}
  /*! compute the vector lateral forces on the wheel in pound-force lbf */
  virtual void DirectionForce_Timeslice (float dT) {};
  /*! compute the vector brake forces on the wheel in pound-force lbf*/
  virtual void BrakeForce_Timeslice () {};
  /*! transform the forces back to the body frame : Timesliced
   * therefore the reference should be 'body' */
  virtual void GearL2B_Timeslice (void) {};
  /*! compute the vector moment for this wheel in lb.ft: Timesliced */
  virtual void VtMoment_Timeslice (void) {};

public:
  ///< Utilities
  ///< =========
  /*! transform gear location from ft to meters */
  void GearLoc2CG_ISU    (void);
  /*! wheel to CG in ft */
  const SVector* GetGear2CG        (void) {return &vGearLoc2CG;}    
  /*! wheel to CG in meters */
  const SVector* GetGear2CG_ISU    (void) {return &vGearLoc2CG_ISU;} 

  virtual void SetGearData  (SGearData *data) {gearData = data;}
  /*! reset values */
  void ResetValues       (void);


  /*! moment in lb.ft */
  virtual const SVector& GetBodyGearMoment     (void) {return vMoment;}    
  /*! force in Newton */
  virtual const SVector& GetBodyGearForce_ISU  (void);    
  /*! moment in Kg.m */
  virtual const SVector& GetBodyGearMoment_ISU (void);
#ifdef HAVE_OPAL
  /*! launch init joint */
  virtual void InitJoint(char type,CGroundSuspension *s) {;}
//  opal::JointData *gjointData;
#endif
  //------ Probe functions ----------------------------------------
  virtual void    ProbeBrake(CFuiCanva *cnv) {;}
  virtual void    ResetForce()    {;}
  //---------------------------------------------------------------
  bool    WheelCollision(CVector &p);
  //------ Reset crash --------------------------------------------
  inline  void  ResetCrash()    {gearData->shok = 0; }
  //---------------------------------------------------------------
};

//====================================================================================
//  JS NOTE:  To support differential braking, a wheel position (left or rite) is added
//            when the object is a wheel
//=====================================================================================
class CSuspension: public CSubsystem { 
  ///< Fly! data script section 
  ///<
protected:
  CAcmGears        *whel;                             // Movinf part
  CAcmTire         *Tire;                             // Tire parts
  CVehicleObject   *mveh;                             // Parent vehicle
  CWeightManager   *wgm;
  CGear            *gear;
  SGearData         gear_data;
public:
  int           reset_crash;                          ///< allows to reset sim
  char          type;                                 // Type of suspension
  char          wInd;                                 // Wheel index
  //----Full constructor ----------------------------------------
  CSuspension (CVehicleObject *v, char *susp_name, CWeightManager *wghman, char type_ = TRICYCLE);
  virtual ~CSuspension             (void);
  void    InitGear();
  ///< CStreamObject methods
  ///< =====================
  virtual int    Read              (SStream *stream, Tag tag);
  virtual void   ReadFinished      (void);
  virtual void   Write             (SStream *stream);

  ///< CSuspension methods
  ///< ===================
  virtual void   Timeslice         (float dT);
  virtual void   Debug             (void);
  
  ///< getters & setters
  ///< ==================
  double  GetGearPosZ ()  {return gear_data.bPos.z;}
  double  GetGearPosY ()  {return gear_data.bPos.y;}
  double  GetGearPosX ()  {return gear_data.bPos.x;}
  double  GetGearRadius() {return gear_data.whrd;}
  /*! The Moment vector for this gear */
  virtual const SVector& GetBodyMoment     (void);
   ///< The Force vector for this gear */
  virtual const SVector& GetBodyForce_ISU  (void);
  /*! The Moment vector for this gear */
  virtual const SVector& GetBodyMoment_ISU (void);
  /*! */
  bool IsSteerWheel();
  void SetWheelBase(float b)        {gear_data.wheel_base = b;}
  //-----------------------------------------------------------------
  /*! Find the location of the gear relative to the CG in ft */
  const SVector* GetGearLoc2CG         (void);
  /*! Find the location of the gear relative to the CG in meters */
  const SVector* GetGearLoc2CG_ISU     (void);
  char  GearShock(char pw);
  void  GearDamaged(char nsk);
  //-------GetGear Position ----------------------------------------
  void  GetGearPosition(CVector &mp,double  &rad)    {gear->GetGearPosition(mp,rad);}
  //-------Define ground relation to wheels ------------------------
  void  InitGearJoint(char type,CGroundSuspension *s) {gear->InitJoint(type,s);}
  void  ResetCrash() {gear_data.shok = 0;}
  void  ResetForce() {gear->ResetForce();}
  //----------------------------------------------------------------
  char**      GetProbeOptions();
  void        Probe(CFuiCanva *cnv);
  //-----Wheel interface -------------------------------------------
  inline void MoveWheelTo(float v,float dT)    {Tire->SetWheelVelocity(v,dT);}
  //-----Gear interface --------------------------------------------
  inline void SetIndex(char x)        {wInd = x;}
  inline void SetWheelAGL(float f)    {whel->SetAGL(f);}
  inline void TurnWheelTo(float d)    {whel->TurnTo(d);}
  inline void PushWheelTo(float d)    {whel->PushTo(d);}
  inline void SetShockTo(float v)     {whel->SetShockTo(v);}
  inline void PlayTire(int p)         {whel->PlayTire(p);}
  inline bool IsOnGround()            {return (0 != gear_data.onGd);}
	//-- Set ABS anti skid feature ---------------------
	inline	void SetABS(char p)		{gear_data.sABS	= p;}	
	//-----------------------------------------------------------------
	inline SGearData *GetGearData()			{return &gear_data;}
  //-----------------------------------------------------------------
  inline char GetWheelIndex()         {return wInd;}
  //-----------------------------------------------------------------
};
//=====================================================================================
class CBumper : public CWhl {
public:
  CBumper                    (char *name_, CWeightManager *wghman, char type = TRICYCLE);
  virtual ~CBumper                   (void);

  ///< CStreamObject methods
  virtual int   Read         (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);
  virtual void  Write        (SStream *stream);

  ///< CBumper methods
  virtual void  Timeslice    (float dT);
  void          Debug        (void);

  //
private:
  CWeightManager  *weight_manager;

  ///< Fly! data script section 
  ///
  char bump_name[56];
  std::vector<std::string> vfx_;  ///< -- Visual Effects --
  // endf;                        //
  CDamageModel oy_T;              ///< -- Damage Entry --
  int   bmpF[2];                  ///< -- bumper force (normal -- friction)
  SVector mPos;                   ///< -- Contact Point (model coodinates) --
};


//=========================================================================================
/*! Data Structure / Class Descriptions
 * \brief Interface for wheels & suspension management
 * \detailed : manages suspensions, bumpers, and weight
 */
// JS NOTES:  To support differential braking, we must distinguish between left and right wheel(s)
//            When <susp> is a wheel, we have the following tags
//            <brak>  Implies that wheel has brake
//             +/-1   -1=>Left wheel while 1 => Right wheel
//   Brake systems (CGroundBrake) are made addressable by the message system 
//          tag 'brak'      This is a brake message
//          unit  0         This is for both  wheels
//          unit  1         This is for left  wheel(s)
//          unit  2         This is for right wheel(s)
//
//===============================================================================================
class CGroundSuspension: public CSubsystem {           // : public CWhl {
public:
  CGroundSuspension()              {mveh = 0;}
  CGroundSuspension                (CVehicleObject *v, char* whlFilename, CWeightManager *wghman);
  virtual ~CGroundSuspension       (void);

  /*! CStreamObject methods */
  virtual int   Read(SStream *stream, Tag tag);
  virtual void  ReadFinished(void);

  /*! CGroundSuspension generic methods */
  virtual void  Timeslice          (float dT);
  //-------------------------------------------------------------------
  void  ReadSusp(SStream *st);
	void	SetABS(char p);
  void  ResetCrash();
  void  ResetForce();
  //-------------------------------------------------------------------------
  /*! Getters - setters */
  const SVector* GetSumGearForces  (void)              {return &SumGearForces;}
  const SVector* GetSumGearMoments (void)              {return &SumGearMoments;}
  int             GetNumberOfWheels(void)              {return wheels_num;} //< body wheels num 
  /*! Set steering direction value */
  const double&  GetMaxWheelHeight (void)              {return max_wheel_height;}
  const double&  GetMainGearHeight (void)              {return mWPos;}
  //----------------------------------------------------------------------------------
  inline void     GetAllWheels(std::vector<CSuspension *> &whl) { whl = whl_susp;}
  inline CVector *GetMainGearCenter() {return &mainW;}
  inline double   GetMainGearRadius() {return  mainR;}
  inline double   GetBodyAGL()        {return  bAGL;}
  inline double   GetPositionAGL()    {return  (bAGL + mainR - 1);}
  inline double   GetSterGearRadius() {return  sterR;}
  inline void     StoreGearVM(CVector &v, double mc) {mainVM = v; massCF = mc;}
  inline char     GetNbWheelOnGround(){return nWonG;}
  inline bool     WheelsAreOnGround() {return (nWonG != 0);}
	inline bool			AllWheelsOnGround()	{return (nWonG == wheels_num);}
  inline double   GetWBase()          {return wheel_base;}
  //----------------------------------------------------------------------------------
  ///
  float                      rMas;                         ///< rated mass
  //------ATTRIBUTES -----------------------------------------------------------------
protected:
  CVehicleObject            *mveh;                           // Parent vehicle
  std::vector<CSuspension*>  whl_susp;
  std::vector<CWhl *>        whl_bump;
  CWeightManager            *whm;                      ///< a link to the CWeightManager
                                                           ///< for rated mass
  CVector             SumGearForces;
  CVector             SumGearMoments;
  char                type;                                 ///JS Type of suspension
  char                wInd;                                 // JS Wheel index
  CFmtxMap           *mstbl;                                // Steering table
  CFmtxMap           *mbtbl;                                // Brake table
  //------Wheel interface ------------------------------------------------------------
  float         deflect;                                      // Deflection
  float         scale;                                        // Scale
  //----------------------------------------------------------------------------------
  double        wheel_base;                                   // Inter axes distance
  double        bAGL;                                         // Body AGL (in feet)
  double        mainR;                                        // Main average radius
  CVector       mainW;                                        // Main gear barycenter
  double        sterR;                                        // Steering wheel radius
  char          nWonG;                                        // Number wheels on ground
  //-----------------------------------------------------------------------------------
  int           wheels_num;                                   ///< body wheels num
  double        max_wheel_height;
  //------Main position depending of the tail type to compute moment -----------------
  CVector        mainVM;                                  // Main vector moment
  double         massCF;                                  // Mass coefficient
  //----------------------------------------------------------------------------------
  double        max_gear,
                min_gear, 
                mWPos;            // Main wheel position In feet

};
//==========================================================================================
// \class * Based upon JSBSim code
//   see copyright in cpp file
////==========================================================================================
class CGearMixJL : public CGear {
public:
  CGearMixJL                      (CVehicleObject *v,CSuspension *s);
  virtual ~CGearMixJL             (void);
  ///< CGearMixJL methods
  ///< =============
  /*! gear compression value calc in ft : Timesliced */
  char						GCompr__Timeslice (void);
  /*! gear compression velocity : Timesliced
   *  With the gear compression length We can get the compression velocity
   *  (used to determine the damping force)*/
  void						GComprV_Timeslice (void);
  /*! compute the vector force on the wheel in pound-force lbf 
   * (using square-law damping for Y) : Timesliced */
  void						VtForce_Timeslice (float dT);
  /*! compute the vector lateral forces on the wheel in pound-force lbf*/
  void						DirectionForce_Timeslice (float dT);
  /*! compute the vector brake forces on the wheel in pound-force lbf*/
  void						BrakeForce_Timeslice ();
  /*! transform the forces back to the body frame : Timesliced
   * therefore the reference should be 'body' */
  void						GearL2B_Timeslice (void);
  /*! compute the vector moment for this wheel in lb.ft: Timesliced */
  void						VtMoment_Timeslice (void);

  ///< Utilities
  ///< =========
   
  /*! moment in lb.ft */
  const SVector& GetBodyGearMoment     (void) {return vMoment;}    
  /*! force in Newton */
  const SVector& GetBodyGearForce_ISU  (void);    
  /*! moment in Kg.m */
  const SVector& GetBodyGearMoment_ISU (void);
  /*! */


private:
  ///< CGear members
  ///< =============
  double    wheel_base;      // In meters

  float     brakeFcoeff,
            bad_pres_resis;
  float     rolling_whl_vel,
            side_whl_vel;
  float     rolling_force,
            rolling_whl_dir,
            side_force;
};
//==========================================================================================
//  class * Based upon JSBSim code
//   see copyright in cpp file
////========================================================================================
class CGearJSBSim : public CGear {
public:
  CGearJSBSim                      (CVehicleObject *v,CSuspension *s);
  virtual ~CGearJSBSim             (void);
  ///< CGearJSBSim methods
  ///< =============
  /*! gear compression value calc in ft : Timesliced
   *  JSBSim = the gear compression value is the local frame gear Y location value minus the height AGL
   *  Currently, we assume that the gear is oriented - and the deflection occurs in - the Y axis */
  char						GCompr__Timeslice (void);
  /*! gear compression velocity : Timesliced
   *  With the gear compression length We can get the compression velocity
   *  (used to determine the damping force)*/
  void						GComprV_Timeslice (void);
  /*! compute the vector force on the wheel in pound-force lbf 
   * (using square-law damping for Y) : Timesliced */
  void						VtForce_Timeslice (float dT);
  /*! compute the vector lateral forces on the wheel in pound-force lbf*/
  void						DirectionForce_Timeslice (float dT);
  /*! compute the vector brake forces on the wheel in pound-force lbf*/
  void						BrakeForce_Timeslice ();
  /*! transform the forces back to the body frame : Timesliced
   * therefore the reference should be 'body' */
  void						GearL2B_Timeslice (void);
  /*! compute the vector moment for this wheel in lb.ft: Timesliced */
  void						VtMoment_Timeslice (void);

  ///< Utilities
  ///< =========
   
  /*! moment in lb.ft */
  const SVector& GetBodyGearMoment     (void) {return vMoment;}    
  /*! force in Newton */
  const SVector& GetBodyGearForce_ISU  (void);    
  /*! moment in Kg.m */
  const SVector& GetBodyGearMoment_ISU (void);
  /*! */

private:
  ///< CGear members
  ///< =============

  float     brakeFcoeff,
            bad_pres_resis;
  float     rolling_whl_vel,
            side_whl_vel;
  float     rolling_force,
            rolling_whl_dir,
            side_force;
};

#endif // GEARS_H
//=======================END OF FILE =============================================================
