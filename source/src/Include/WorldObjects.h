/*
 * WorldObjects.h
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

/*! \file WorldObjects.h
 *  \brief Defines CWorldObject class hierarchy of simulation world objects
 *
 * Every entity that populates the simulated world is represented by a
 *   "world object"...this includes static models such as buildings and
 *   bridges, AI objects such as aircraft, and the user object which can
 *   be a ground vehicle, airplane or helicopter.
 *
 */
#ifndef WORLDOBJECTS_H
#define WORLDOBJECTS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "../Include/FlyLegacy.h"
#include "../Include/UserVehicles.h"
#include "../Include/AnimatedModel.h"
#include "../Include/3dMath.h"
#include "../Include/FrameManager.h"  // 
#include "../Include/Autopilot.h"
#include "../Include/AudioManager.h"
#include "../Include/OpalGears.h"
#include "../Include/Robot.h"

#include <vector>

//======================================================================================
//  Vehicle global state
//======================================================================================
typedef enum {
  VEH_INIT  = 0,    // Initialization
  VEH_OPER  = 1,    // Operational
  VEH_CRSH  = 2,    // Crashing
  VEH_INOP  = 3,    // In slew mode
} VEH_STATE;
//======================================================================================
// The CWorldObject object is a dynamic scenery model with
//   <wobj> signature of TYPE_FLY_WORLDOBJECT
//
// Stream file declaration:
//    <wobj>
//    wobj
//    <bgno>
//    <endo>
//  IMPORTANT NOTE:   DO NOT update DIRECTLY geop and iang.
//                    Use SetPosition() and SetOrientation() as other dependant fields
//                    must also be updated to stay sync.
//======================================================================================
class CWorldObject : public CStreamObject {
protected:
	//------ Type of vehicle ----------------------------
  char        stype[8];
  Tag         type; 
  DAMAGE_MSG  damM;
	//--- physical model --------------------------------------
	void     *phyMod;
  //----Global state ----------------------------------------
  char    State;
  //----Aircraft parameters ---------------------------------
  //  NOTE: Those fields should be dupplicated in globals
  //---------------------------------------------------------
  SPosition orgp;   // Original position (start-up)
  SPosition geop;   // Position (lat/lon/alt) of the object
  CVector   iang;   // Inertial angular orientation
  CVector   dang;   // Same as above in degre
  //--- Global rotation matrix --------------------------------
  double    rotM[16];                 // Rotation matrix
  //-----Flag for various aircraft versions ------------------- 
public:
  bool      is_ufo_object;
  bool      is_opal_object;
  bool      has_fake_engine_thrust;
  //----Inline -----------------------------------------------
  bool  HasState(char c)    {return (State == c);}
  bool  NotState(char c)    {return (State != c);}
  void  SetState(char c)    {State = c;}
  char  GetState()          {return State;}
	//----METHODS ----------------------------------------------
public:
  CWorldObject (void);

  // CStreamObject methods
  virtual int       Read           (SStream *stream, Tag tag);
  virtual void      ReadFinished   (void);

  //--- CWorldObjectBase methods -----------------------------
  virtual void Print (FILE *f);
  virtual void ResetCrash (char p) {} 
  virtual void EndLevelling() {}
  virtual void DamageEvent(DAMAGE_MSG *msg) {;}
  //------------------------------------------------------------
  inline  Tag   GetType (void) { return type; }
  inline  Tag*  GetTypePointer (void) { return &type; }
  //------------------------------------------------------
  inline bool   IsAirplane()  {return (type == TYPE_FLY_AIRPLANE);}
  //------inline  CWorldObject methods ---------------------------------------
	inline    void      *GetPhyModel()												{return phyMod;}
  inline    SPosition  GetOriginalPosition()								{return orgp;}  
  inline    SPosition *GetAdPosition()											{return &geop;}           
  inline    SPosition  GetPosition()												{return geop;}
  inline    double     GetDirection()												{return -dang.z;}
  inline    double     GetAltitude()												{return geop.alt;}
  inline    void       SetAltPosition(double a)		          {geop.alt = a;}
  //----------------------------------------------------------------------------
  virtual void SetPhysicalOrientation (CVector &v)  {;}  // used in COpal to slew orientation
  virtual void ResetSpeeds() {}
  virtual void ResetZeroOrientation () {}
  //-----------------------------------------------------------------------------
	void			SetType(Tag t);
  void      SetOrientation(SVector v);
  void      SetPosition(SPosition pos);
  void      SetAltitude(double alt);
  //------------------------------------------------------------------------------
  inline  void    Rotate()    {glMultMatrixd(rotM);}
  inline  double *GetROTM()   {return rotM;}
  inline  void    RotateVector(CVector &V,SVector &R) {V.MultMatGL(rotM,R);}  // Rotate V into R
  //------------------------------------------------------------------------------
  void      AddOrientationInDegres(SVector &v);
  SVector   GetOrientation(void)  {return iang;}    // In RH coordinate system (radian)
  SVector   GetOrientDegre(void)  {return dang;}
	float			GetMagneticDirection();
  void      GetRRtoLDOrientation  (SVector *vec);
  void		  SetLDtoRROrientation  (SVector *vec);
  double    GetRRtoLDPitch ()     {return  -dang.x;}
  double    GetRRtoLDBank()       {return  -dang.y;}
  double    GetBank()             {return  -iang.y;}
  //-------------------------------------------------------------------------------


};
//====================================================================================
// The CSimulatedObject object has some behaviour tied to the real-time simulation.
//   This could be an animated object, e.g. windsock.
//
// Stream file declaration:
//    <wobj>
//    sobj
//    <bgno>
//    <endo>
// JS NOTE: CModelObject has been removed as 3D model are implemented separatedly
//        in model3D.cpp as C3Dmodel class
//=====================================================================================
class CSimulatedObject : public CWorldObject {
  //---ATTRIBUTES ----------------------------------------------
public:
  char                  nfoFilename[64];
  CVehicleInfo          *nfo;
  CAnimatedModel        *lod;
  //---METHODS ------------------------------------------------
public:
  // Constructors / destructor
  CSimulatedObject                  (void);
  virtual ~CSimulatedObject         (void);

  // CStreamObject methods
  virtual int   Read                (SStream *stream, Tag tag);
  virtual void  ReadFinished        (void);

  // Simulation
  virtual void  Timeslice           (float dT,U_INT FrNo);       ///< Real-time timeslice processing
  // Drawing 
  virtual void  DrawExternal();

  //-------- Methods ------------------------------------------
  ETerrainType GetTerrainType (void);           ///< get terrain type under object
  //------------------------------------------------------------

};

//====================================================================================
// The CDLLSimulatedObject object is a specialization of CSimulatedObject for DLL
//   This could be a dynamic object managed by dll.
//
// Stream file declaration:
//    <wobj>
//    sobj
//    <bgno>
//    <endo>
//=====================================================================================
class CDLLSimulatedObject : public CSimulatedObject {
    
public:
  // Constructors / destructor
  CDLLSimulatedObject                  (void);
  virtual ~CDLLSimulatedObject         (void);

  // CStreamObject methods
//  virtual int   Read                (SStream *stream, Tag tag);
//  virtual void  ReadFinished        (void);

  virtual void  SetOrientation      (SVector orientation);

  // Simulation
  virtual void  Timeslice           (float dT, U_INT FrNo);       ///< Real-time timeslice processing
  /// Called from Timeslice() to simulate the things that happen in the timeslice
  virtual void  Simulate            (float dT, U_INT FrNo);				
  // Drawing 
  virtual void  DrawExternal();
  // Methods

protected:
  bool draw_flag;

public:
  bool                  sim_objects_active;
  CVector               sobj_offset;
};
//==============================================================================
// The CVehicleObject object is ...
//   <wobj> signature of TYPE_FLY_VEHICLEOBJECT
//
// Stream file declaration:
//    <wobj>
//    vehi
//    <bgno>
//    <endo>
//==============================================================================
class CVehicleObject : public CSimulatedObject {
public:
  // Constructors / destructor
  CVehicleObject                                     (void);
  virtual ~CVehicleObject                            (void);

  // CStreamObject methods
  virtual int   Read                                 (SStream *stream, Tag tag);
  virtual void  ReadFinished                         (void);

  // CVehicleObject methods

  virtual EMessageResult    ReceiveMessage(SMessage *msg) {return MSG_IGNORED;}
  virtual bool              FindReceiver(SMessage *msg){return false;}; // JSDEV* Message preparation
  virtual void				      PrepareMsg(void);							      // JSDEV* Message preparation
		      void				      TraceMsgPrepa(SMessage *msg);				// JSDEV* Message preparation
          void              DrawAeromodelData();
  virtual CCameraManager*   GetCameraManager();
  virtual CCockpitManager*  GetCockpitManager()      {return pit;}
  virtual Tag               GetPanel();
  virtual void              SetPanel(Tag tag);
  virtual void              Print                    (FILE *f);
  virtual void              TimeSlice(float dT,U_INT frame) {}
  virtual void              Update(float dT,U_INT FrNo);		        // JSDEV*
  virtual void              ResetVehicle(void) {;} 
  //! Returns altitude above ground in feet
	float                     GetUserAGL()	{return 0;}
  //! Returns vehicle mass (kg)
  virtual double            GetMassInKgs();
	virtual double						GetMassInLbs();
  //! Returns vehicle mass moments of inertia (kg m^2)
  virtual const SVector*    GetMassMomentOfInertia   ();
  //! Returns vehicle moments of inertia (kg m^2)
  virtual const SVector*    GetMomentOfInertia       ();
  //! Returns object CG position in object coordinates (m)
  virtual const SVector*    GetCG                    ();
  //! Returns velocity vector in inertial frame (m/s)
  virtual const CVector*    GetInertialVelocityVector        (void);
  //! Returns velocity vector in body frame (m/s)
  virtual const CVector*    GetBodyVelocityVector    (void);
  //! Returns acceleration vector in inertial frame (m/s)
  virtual const CVector*    GetInertialAccelerationVector    (void);
  //! Returns acceleration vector in body frame (m/s)
  virtual const CVector*    GetBodyAccelerationVector(void);
  //! Returns velocity relative to ground in object coordinates (m/s)
  virtual double GetGroundspeed();
  //! Returns airspeed vector in object coordinates (m/s)
  virtual const CVector*    GetAirspeed              (void);
  //! Returns airspeed with body orientation reference
  virtual const CVector*    GetRelativeBodyAirspeed  (void);
  //! Returns indicated airspeed in feet / sec
  virtual void GetIAS                 (double &spd); // IAS in ft/s
  //! Returns indicated airspeed in knts
  virtual void GetKIAS                (double &spd); // KIAS
  //! Returns true airspeed in feet / sec
  virtual void GetTAS                 (double &spd); // TAS in ft/s
  //! Returns indicated airspeed in knts
  virtual const double& GetPreCalculedKIAS (void) {return kias;} //
  //! Returns object angular velocity in object coordinates (rad/s)
  virtual const CVector*    GetBodyAngularVelocityVector (void);
  //!  Returns object angular acceleration in object coordinate (rad/s²)
  virtual const CVector*		GetBodyAngularAccelerationVector(void);
  //! Returns object angular velocity in world coordinates (rad/s)
  virtual const CVector*    GetInertialAngularVelocityVector (void);
  //!  Returns object angular acceleration in world coordinate (rad/s²)
  virtual const CVector*		GetInertialAngularAccelerationVector(void);
  //  Return body AGL (Body above ground level -----------------------------
  virtual double            GetBodyAGL()  {return 0;}
  //!  Returns wind effect on aircraft
  virtual void              GetAircraftWindEffect (void);
  //  Set plane above ground -----------------------------------------------
  virtual void              PositionAGL() {;}
  virtual double            GetPositionAGL() {return 0;}
  //  Set plane resting ----------------------------------------------------
  virtual void              RestOnGround()  {;}
  //---Crash management ----------------------------------------------------
  virtual void              BodyCollision(CVector &p) {;}
  //------------------------------------------------------------------------
  virtual void              GetAllEngines(std::vector<CEngine*> &engs) {}
  //----------------------Drawing ------------------------------------------
  virtual void  DrawExternal();
  virtual void  DrawInside(CCamera *cam);
	virtual void  DrawOutsideLights();
  //------------------------------------------------------------------------
  void          DrawExternalFeatures();
  //-------Keyboard messages ------------------------------------------------
  virtual void  SetNaviMSG(Tag t) {}
  virtual void  SetTaxiMSG(Tag t) {}
  virtual void  SetLandMSG(Tag t) {}
  virtual void  SetStrbMSG(Tag t) {}
  //--------------Helpers ---------------------------------------------------
  void  StoreNFO(char *nfo);
  void  ReadParameters(CStreamObject *obj,char *fn);
  //---Radio interface ------------------------------------------------------
	inline  void  RegisterRadioBUS(BUS_RADIO *b)	{busR = b;}
	inline  void  RegisterGPSR(GPSRadio *r)				{GPSR = r;}
	inline  void	RegisterRAD(CRadio *r){mRAD = r;}
  inline  void  RegisterNAV(Tag r)    {rTAG[NAV_INDEX] = r;}      //{rNAV = r;}
  inline  void  RegisterCOM(Tag r)    {rTAG[COM_INDEX] = r;}      //rCOM = r;}
  inline  void  RegisterADF(Tag r)    {rTAG[ADF_INDEX] = r;}      //rADF = r;}
  inline  Tag   GetNAV()              {return  rTAG[NAV_INDEX];}  //rNAV;}
  inline  Tag   GetCOM()              {return  rTAG[COM_INDEX];}  //rCOM;}
  inline  Tag   GetADF()              {return  rTAG[ADF_INDEX];}  //rADF;}
  inline  Tag   GetRadio(int k)       {return  rTAG[k];}
	//--- Component pointers ------------------------------------------------
	inline  BUS_RADIO *GetRadioBUS()		{return busR;}	// Radio BUS
	inline  GPSRadio  *GetGPS()					{return GPSR;}	// GPS radio 
	inline  CRadio		*GetMRAD()				{return mRAD;}	// Master radio
  //------------------------------------------------------------------------
  //! send wing deflection to aeromodel
  void          SetWingChannel(CAeroControlChannel *aero);
  //!-----------------------------------------------------------------------
  //! send steering wheel angle to gear
	void					GetGearChannel    (SGearData *gdt);
  void          SetPartKeyframe   (char* part, float value);
  void          SetPartTransparent(char* part, bool ok = true);
  //----Set spinner part -------------------------------------------------------------------
  CAcmSpin     *SetSpinner(char e,char *pn) {return (lod)?(lod->AddSpinner(e,pn)):(0);}     // Set spinner part
  //--------------Aero model management --------------------------------------------------
  const double&             GetWingIncidenceDeg     (void) {return main_wing_incid;}
  const float&              GetWingAoAMinRad        (void) {return main_wing_aoa_min;}
  const float&              GetWingAoAMaxRad        (void) {return main_wing_aoa_max;}
  //--------------Plotting interface ----------------------------------------------------
  int               AddToPlotMenu(char **menu, int k);
  void              PlotParameters(PLOT_PP *pp,Tag itm);
  //-------------------------------------------------------------------------------------
  /// Called from Timeslice() to simulate the things that happen in the timeslice
  /// This method is expected to update the mass, mass distribution and total force 
  virtual void  Simulate          (float dT, U_INT FrNo);				// JSDEV*
  /// Called from Timeslice() to update motion state, using the current
  /// mass, mass distribution and total force as input
	virtual void  UpdateOrientationState (float dT, U_INT FrNo) {;}
  // Simulation internals are only available to subclasses
	virtual void  SetWindPos(double p) {;}
  //-------------------------------------------------------------------------------------
public:
  // wind angle relative to the wind direction in LH m/s

  // Vehicle specifications, subsystems, etc.
  char                  upd;                  // Update instrument while in slew
  CSimulatedVehicle     *svh;
  CElectricalSystem     *amp;
  CAerodynamicModel     *wng;
  CPhysicModelAdj       *phy; // PHY file 
  CFuelSystem           *gas;
  CPitotStaticSystem    *pss;
  CGroundSuspension     *whl;
  CVariableLoadouts     *vld;
  CCockpitManager       *pit;
  CCameraManager        *cam;
  CRadioManager         *rdo;
  CExternalLightManager *elt;
  CEngineManager        *eng;
  CControlMixer         *mix;
  CSlopeWindData        *swd;
  CWeightManager        *wgh;
  CVehicleHistory       *hst;
	PlaneCheckList        *ckl;
	//-----------------------------------------------------------------------------
  CNullSubsystem		    nSub;						//  Null subsystem to receive message without identified receiver
  //-----Radio interface --------------------------------------------------------
  Tag                   rTAG[4];					// Radio TAG: NAV-COM-ADF
	BUS_RADIO						 *busR;							// Radio BUS
 	AutoPilot						 *aPIL;							// Autopilot 
	GPSRadio             *GPSR;							// GPS
	CRadio               *mRAD;							// Master Radio
  //-----Sound object collection ------------------------------------------------
  std::map<Tag,CSoundOBJ*> sounds;            // Sound objects related to vehicle
	//====== METHODS ==============================================================
  inline void  AddSound(CSoundOBJ *so) {sounds[so->GetTag()];}
  //-----Mouse events -----------------------------------------------------------
  inline bool   MouseMove (int x,int y) { return (pit)? (pit->MouseMove(x,y)):(false);}
  inline bool   MouseClick(EMouseButton b,int u,int x,int y)  { return (pit)? (pit->MouseClick(b,u,x,y)):(false);}
  //-----------------------------------------------------------------------------
  inline CNullSubsystem*    GetNullSubsystem(void)  { return &nSub; }
  inline char              *GetNFOname()            { return nfoFilename;}
  inline void               GetVisualCG(SVector &v) {if (wgh) wgh->GetVisualCG(v);}
  //-----Engine management ---------------------------------------------------------
	inline bool		AllEngineOn()						{return (engR == eng->GetEngineNbr());}
  //---------------------------------------------------------------------------------
  inline  CAnimatedModel        *GetLOD()     {return lod;}
  inline  CWeightManager        *GetWGH()     {return wgh;}
	inline  int                    GetEngNb()		{return nEng;}
	//--- Gear Management -------------------------------------------------------------
	void					SetABS(char p)					{whl->SetABS(p);}
	float         GetBrakeForce(int p)    {return amp->GetBrakeForce(p);}
  char          GetWheelNum()           {return  WOW_nber++;}
  bool          WheelsAreOnGround()     {return whl->WheelsAreOnGround();}
  char          NbWheelsOnGround()      {return whl->GetNbWheelOnGround();}  
	bool					AllWheelsOnGround()			{return whl->AllWheelsOnGround();}
  //--- Fuel Management -------------------------------------------------------------
  inline void   GetFuelCell(std::vector<CFuelCell*> &vf)  {if (wgh) wgh->GetFuelCell(vf);}
  inline void   GetLoadCell(std::vector<CLoadCell*> &vl)  {if (wgh) wgh->GetLoadCell(vl);}
  inline float  GetDryWeight()                            {return (wgh)?(wgh->GetDryWeight()):(0);}
  inline float  GetGasWeight()                            {return (wgh)?(wgh->GetGasWeight()):(0);}
  inline float  GetLodWeight()                            {return (wgh)?(wgh->GetLodWeight()):(0);}
  //--- Acces to systems ------------------------------------------------------------------
	inline  void							Register(AutoPilot *p){aPIL = p;}
	inline  AutoPilot        *GetAutoPilot()	{return  aPIL;}
	inline  CRobot           *GetRobot()			{return amp->GetRobot(); }
	inline  CFPlan           *GetFlightPlan() {return amp->GetFlightPlan();}
  inline  char             *GetPID()        {return (nfo)?(nfo->GetPID()):(0);}
	//-----------------------------------------------------------------------------------------
protected:
  char   nEng;															// Engine number
	U_CHAR engR;															// Number of running engines
  float  dihedral_coeff;										///< dihedral coeff SVH <dieh>
  float  acrd_coeff;												///< acrd coeff in SVH file
  float  pitch_coeff;												///< pitch coeff SVH <pitd>
	//--- Wind parameters ----------------------------------------------------------
  CVector w_dir_for_body;                   
  int			wind_effect;											///< flag
  float		wind_coeff;                       ///< wind coefficient effect
  float		gear_drag;                        ///< drag from gear
  //---Aerodata drawing ----------------------------------------------------------
  float		draw_aero;                        ///< draw aeromodel data for lines lenght
  //---Wheels parameters ---------------------------------------------------------
  char    WOW_nber;                         // Wheel number
  char    rfuw;                             // Reserved
  //---Wheels functions ----------------------------------------------------------
public:    
  inline  float GetGSpeed()               {return 0;}
  inline  double GetWBase()               {return whl->GetWBase();}
	//---Engine internal interface -------------------------------------------------
	inline  void RazEngR()										{engR	= 0;}
	inline  void IncEngR()										{engR++;}
	inline  void DecEngR()										{engR--;}
protected:
  //--- PLOT parameter table -----------------------------------------------------
  PLOT_PM   plotPM[16];                    // Plot parameters table
  //-------------------------------------------------------------------------------
  /// These variable capture the simulated physical state of the object
  CVector fb;                              ///< Total force on the object in object coordinates (N)
  CVector tb;                              ///< Total torque (moment) on the object in object coordinates (Nm)
  CVector engines_pos;                     ///< Total position of engines where engine force is applied

  /// The following variables capture the objects motion state.

  int cur;                                 ///< Index of state from current timeslice
  int prv;                                 ///< Index of state from previous timeslice

  CVector ifpos;                           ///< Current position in inertial frame (m)
  CVector vi[2];                           ///< Velocity in inertial frame (m/s)
  CVector ai[2];                           ///< Acceleration in inertial fram (m/s^2)
  CVector vb[2];                           ///< Velocity in body frame (m/s)
  CVector ab[2];                           ///< Acceleration in body fram (m/s^2)
  CVector airspeed,                        ///< body speed
          airspeed2;                       ///< body speed relative to the wind

  CQuaternion q;                           ///< Current orientation of object relative to inertial frame
  CQuaternion dq[2];                       ///< Orientation rate of change
  CVector wb[2];                           ///< angular body velocity (rad/s)
  CVector wi[2];                           ///< angular inertial velocity (rad/s)
  CVector dwb[2];                          ///< angular body acceleration (rad/s^2)
  CVector dwi[2];                          ///< angular inertial acceleration (rad/s^2)

  CVector eulerAngles;                     ///< Secondary representation of orientation, derived from q (rad)
  CRotMatrix i2b;                          ///< Secondary representation of orientation, derived from q

  CVector   bodyVelocity;                  ///< Initial body velocity from .SIT file
  CVector   bodyAngularVelocity;           ///< Initial angular velocity from .SIT file

  double    main_wing_incid;               ///< stocking the main wing incidence value DEG
  float     main_wing_aoa_min;             ///< stocking AoA min RAD
  float     main_wing_aoa_max;             ///< stocking AoA max RAD

  double    kias;
};
//========================================================================================
// The CAirplane object represents a fixed-wing aircraft
//   with <wobj> signature of TYPE_FLY_AIRPLANE
//
// Stream file declaration:
//    <wobj>
//    plan
//    <bgno>
//    <endo>
//=========================================================================================
class CAirplane : public CVehicleObject {
protected:
	//-------Message to aircraft -----------------------------------------------
  SMessage  Navi;                     // Navigation light message
  SMessage  Land;                     // Landing light message
  SMessage  Taxi;                     // Taxi light message
  SMessage  Strb;                     // Strobe message
  SMessage  Apil;                     // autopilot
	//--- Control systems ------------------------------------------------------
  //-----------Park option ----------------------------------------------------
  char         park;
  static       CLogFile *log;
  U_INT        sound;                 // Crash sound
	//--- METHODS ---------------------------------------------------------------
public:
  // Constructors / destructor
  CAirplane                              (void);
  virtual ~CAirplane                     (void);

  // CStreamObject methods
  virtual void  ReadFinished (void);

  // CWorldObject methods
  virtual void      Print (FILE *f);

  // CSimulatedObject methods
	virtual void Simulate(float dT, U_INT FrNo) {;}		///< Override 
	virtual void UpdateOrientationState(float dT, U_INT FrNo) {;}		///< Override
  virtual void ResetVehicle();
  //----JSDEV* Message preparation --------------------------------------------
  virtual	void			PrepareMsg(void);	// Prepare all subsystem messages
  virtual	bool			FindReceiver(SMessage *msg);			
			    bool			FindReceiver(SMessage *msg,CElectricalSystem *esys);
			    bool			FindReceiver(SMessage *msg,CFuelSystem *gsys);
					bool			FindReceiver(SMessage *msg,CEngineManager *engs);
  //-----------------------------------------------------------------------------
  virtual void      ResetCrash(char p);
  virtual void      EndLevelling();
  //----Receive a message here --------------------------------------------------
  EMessageResult    ReceiveMessage(SMessage *msg);
  EMessageResult    SendMessageToAmpSystems(SMessage *msg);
  EMessageResult    SendMessageToGasSystems(SMessage *msg);
  EMessageResult    SendMessageToEngSystems(SMessage *msg);
  EMessageResult    SendMessageToExternals (SMessage *msg);
  //----Init keyboard messages --------------------------------------------------
  inline void       SetNaviMSG(Tag t) {Navi.group = t;}
  inline void       SetLandMSG(Tag t) {Land.group = t;}
  inline void       SetTaxiMSG(Tag t) {Taxi.group = t;}
  inline void       SetStrbMSG(Tag t) {Strb.group = t;}
  //-----------------------------------------------------------------------------
  void              GetAllEngines(std::vector<CEngine*> &engs);
  void              CutAllEngines()     {eng->CutAllEngines();}
  void              EnginesIdle()       {eng->EnginesIdle();}
  void              AbortEngines()      {eng->AbortEngines();}
  SPosition         SetOnGround();
  double            GetBodyAGL()      {return whl->GetBodyAGL();}
  double            GetPositionAGL()  {return whl->GetPositionAGL();}
  void              BodyCollision(CVector &p);
  void              DamageEvent(DAMAGE_MSG *msg);
  void              CrashEvent(DAMAGE_MSG *msg);
  //----Update the vehicle ------------------------------------------------------
  void              TimeSlice(float dT,U_INT frame);
  //-----------------------------------------------------------------------------
  void              BindKeys();
  //-----------------------------------------------------------------------------
  // CAirplane methods
  /// The control surface methods AileronIncr etc. cause the appropriate
  ///   control surface to be deflected "up" or "down" (as defined for that
  ///   particular control) by one step.  Step size is an aircraft-specific
  ///   attribute that is set by the user in the Aircraft->Settings dialog
  ///   and stored in the HST file for each particular aircraft.
  //----------------------------------------------------------------------------
  bool         CenterControls      (void);      ///< Center all controls
  void         AileronIncr         (void);      ///< Increment aileron by one step
  void         AileronDecr         (void);      ///< Decrement aileron by one step
  void         AileronSet          (float fv);  ///< Set aileron value
  //-----Elevator interface -------------------------------------------------------
  void         ElevatorIncr        (void);      ///< Increment elevator by one step
  void         ElevatorDecr        (void);      ///< Decrement elevator by one step
  void         ElevatorSet         (float fv);  ///< Set elevator value
  //-----Rudder inferface --------------------------------------------------------
  void         RudderBankMap(CFmtxMap *m);
  void         RudderOpalCoef (float fv);       // Set Opal coefficient
  void         RudderIncr          (void);      ///< Increment rudder by one step
  void         RudderDecr          (void);      ///< Decrement rudder by one step
  void         RudderSet           (float fv);  ///< Set rudder value
	void				 RudderBias					(float inc);	// Increment rudder bias
  //----Flap interface -----------------------------------------------------------
  void         FlapsExtend         (void);      ///< Increment flaps by one step
  void         FlapsRetract        (void);      ///< Retract flaps by one step
	//--- Aileron Trim interface --------------------------------------------------
  void         AileronTrimIncr     (void);      ///< Increment aileron trim one step
  void         AileronTrimDecr     (void);      ///< Decrement aileron trim one step
  void         AileronTrimSet      (float fv);  ///< Set aileron trim value
	//--- Elevator Trim interface --------------------------------------------------
  void         ElevatorTrimIncr    (void);      ///< Increment elevator trim one step
  void         ElevatorTrimDecr    (void);      ///< Decrement elevator trim one step
  void         ElevatorTrimSet     (float fv);  ///< Set elevator trim value
	//--- Rudder Trim interface --------------------------------------------------
  void         RudderTrimIncr      (void);      ///< Increment rudder trim one step
  void         RudderTrimDecr      (void);      ///< Decrement rudder trim one step
  void         RudderTrimSet       (float fv);  ///< Set rudder trim value
  void         GroundBrakes        (U_CHAR b);  ///< Set Ground Brakes on-off
  void         ParkBrake           (void);      ///< set parking brake
  void         GearUpDown          (void);      ///< gear up/down
	//--- Surface position interface --------------------------------------------------
  float        Aileron             (void);      ///< Get aileron position
  float        AileronDeflect      (void);      ///< Get aileron deflection
  float        Elevator            (void);      ///< Get elevator position
  float        ElevatorDeflect     (void);      ///< Get rudder deflection
  float        Rudder              (void);      ///< Get rudder position
  float        RudderDeflect       (void);      ///< Get rudder deflection
  float        AileronTrim         (void);      ///< Get aileron trim position
  float        ElevatorTrim        (void);      ///< Get elevator trim position
  float        RudderTrim          (void);      ///< Get rudder trim position
	//--- ACCES TO SUBSYSTEMS --------------------------------------------------
	inline    void				 VirtualPilot()	{amp->vpil->Start();}
  //------Message interface --------------------------------------------------
  inline    void         SendNaviMsg()  {Send_Message(&Navi);}
  inline    void         SendApilMsg()  {Send_Message(&Apil);}
  inline    int          GetEngNb()     {return amp->pEngineManager->HowMany();}
  //---------------------------------------------------------------------------
  float        Flaps               (void);      ///< Get flaps position
};

//=========================================================================================
// The CUFO object represents a simplified fixed-wing aircraft
// with <wobj> signature of TYPE_FLY_AIRPLANE
// :: no real physics and moving like an UFO with joystick ant thrust
// NB : used temporary to buzz around the scenes without the slew feature
//
// Stream file declaration:
//    <wobj>
//    plan
//    <bgno>
//    <endo>
//=========================================================================================
class CUFOObject : public CAirplane {
public:
  // Constructors / destructor
  CUFOObject                              (void);

  // CWorldObject methods

  // CSimulatedObject methods
  void Simulate                           (float dT,U_INT FrNo);		///< Overriden 
  void UpdateOrientationState(float dT,U_INT FrNo);		///< Overriden
  int	 UpdateNewPositionState(float dT,float spd);
  static    CLogFile *log;
  bool show_position;
};

//
// The COPAL object represents a OPAL fixed-wing aircraft
// with <wobj> signature of TYPE_FLY_AIRPLANE
//
// Stream file declaration:
//    <wobj>
//    plan
//    <bgno>
//    <endo>
//
#ifdef HAVE_OPAL
class COPALObject : public CAirplane {
public:
  // Constructors / destructor
  COPALObject                             (void);
  virtual ~COPALObject                    (void);

  void ReadFinished               (void);
  void  PlaneShape();
  // CWorldObject methods
  void  SetPhysicalOrientation (CVector &rad_angle);
  //---- Normal management --------------------------------
  void  Simulate(float dT,U_INT FrNo);		            ///< Overriden 
  void  PositionAGL();           
  void  UpdateOrientationState(float dT,U_INT FrNo); ///< Overriden
  void  UpdateNewPositionState (float dT, U_INT FrNo);
  void  RestOnGround();
  //---------------------------------------------------------
  void Print(FILE *f);
  //---- Crash Management -----------------------------------
  void  ResetOrientation (const CVector &rad_angle);
  void  ResetZeroOrientation();
  void  SetOpalSlewOrientation     (const SVector &v);
  void  ResetSpeeds();
  void  ResetCrash(char p);
	//----------------------------------------------------------
	void SetWindPos(double p);	
  //----------------------------------------------------------
  // class members
  opal::Solid *Ground;                                              // Ground solid
  opal::Solid *Plane;                                               // Aircraft solid
  opal::Force lf;                                                   ///< linear force
  opal::Force ef;                                                   ///< engine linear force
  opal::Force ed;                                                   ///< engine linear drag
  opal::Force tf;                                                   ///< torque
  opal::Force glf;                                                  ///< gear linear force
  opal::Force gtf;                                                  ///< gear torque
  opal::Vec3r av;                                                   ///< angular velocity
  opal::Mass mm;                                                    ///< total mass
  opal::Force mf;                                                   ///< mass linear force
  static opal::Point3r tmp_pos;                                     ///< temporary position
  CVector spd;                                                      ///< speed
  opal::Force wlf [3];                                              ///< tricycle gear
  opal::Force wm;                                                   ///< wind induced moment
  int turb_effect;
	//------------------------------------------------------------
	float yawMine;
	float rollMine;
  float pitchMine;
  float angularDamping;
	float	linearDamping;
	double wind_pos;
	//------------------------------------------------------------
private:
  CLogFile *log;
  float ground_friction;
  double Kb;                                                        ///< clamp rotation
};

#endif

//
// The CHelicopter object represents a rotary-wing aircraft
//   with <wobj> signature of TYPE_FLY_HELICOPTER
//
// Stream file declaration:
//    <wobj>
//    heli
//    <bgno>
//    <endo>
//
class CHelicopterObject : public CVehicleObject {
public:
  // Constructors / destructor
  CHelicopterObject (void);
};


//=============================END OF FILE ====================================
#endif // WORLDOBJECTS_H
