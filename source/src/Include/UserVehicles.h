/*
 * UserVehicles.h
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

/*! \file UserVehicles.h
 *  \brief Defines classes used by user vehicle objects CAirplane etc.
 *
 * Each of the following classes represents a first-level member of either
 *   the CAirplane, CHelicopter or CGroundVehicle classes.
 */

#ifndef USERVEHICLES_H
#define USERVEHICLES_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "../Include/FlyLegacy.h"
#include "../Include/Cameras.h"
#include "../Include/Panels.h"
#include "../Include/Subsystems.h"
#include "../Include/WeightManager.h"
#include "../Include/AeroModel.h"
#include "../Include/Utility.h"
#include "../Include/ModelACM.h"
#include "../Include/Gears.h"
#include <string>
#include <vector>
#include <map>

//=======================================================================================
class CFuiPlot;
class CGroundSuspension;
class CgHolder;
class CFPlan;
class VPilot;
//=======================================================================================
//
// Sound effects for an engine; corresponds to <engn> sub-object in .SFX file
//
class CEngineSound : public CStreamObject {
public:
  CEngineSound (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

protected:
  int         engineNumber;               ///< Engine number
  std::string startInt, startExt;         ///< Starting sounds
  std::string idleInt, idleExt;           ///< Idle sounds
  std::string flyInt, flyExt;             ///< In-flight sounds
  std::string stopInt, stopExt;           ///< Stop sounds
  std::string rampUpInt, rampUpExt;       ///< Ramp up sounds
  std::string rampDownInt, rampDownExt;   ///< Ramp down sounds
  float       bendMin, bendMax;           ///< Min/max prop pitch bend
  float       freqTolerance;              ///< Frequency tolerance
};

//====================================================================
// Sounds related to the user simulated vehicle
//
// The SFX file contains the wav files for the aircraft.
//=========================================================================
class CVehicleSound : public CStreamObject {
public:
  // Constructors / destructor
  CVehicleSound (const char *sfxFilename);
  virtual ~CVehicleSound (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);
  void  SetSound(Tag ids,char *fn);
  void  ReadFlapSound(SStream *s);
  void  ReadTireSound(SStream *s);
  void  ReadCrashSounds(SStream *st);
  void  SetEngineSound(Tag ids,char *fn);
  // functions member
  //---ATTRIBUTES -------------------------------------
  int  sample;
  float volu;                   // Volume
  float pMax;                   // Max pitch
  float pMin;                   // Min pitch
  char  NbFlap;
  char  NbTire;
  // pre-cache sfx files
  bool cach;

  // Support for alternative specification of engine sounds
  std::vector<CEngineSound*>   engineSounds;


};

//=======================================================================================
// Simulated Vehicle
//
// The SVH file contains some key information about the aircraft weight/balance
//   simulation behaviour and
//========================================================================================

class CSimulatedVehicle : public CStreamObject { 
public:
  CSimulatedVehicle (CVehicleObject *v, char* svhFilename, CWeightManager *wgh);
  virtual ~CSimulatedVehicle (void);

  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);
  void  Write (SStream *stream);

  // CSimulatedVehicle methods
  void  Timeslice (float dT);

  // functions member
  char*     GetManufacturer    (void) {return make;}
  char*     GetTailNumber      (void) {return acid;}
  char*     GetName            (void) {return name;}
  float     GetCeiling         (void) {return ceiling;}
  float     GetCruiseSpeed ()					{return maxcruisespeed;}
	float     GetApproachSpeed()				{return approachspeed;}
	float			GetBrakeDistance()				{return brakeDist;}
  float     GetStallSpeed()						{return stallspeed;}
  void      GetSpeedAndCeiling (float *sp,float *al) {*sp = maxcruisespeed;*al = ceiling;}
  CVector*  GetNewCG_ISU()						{return &newCG_ISU;}
  SVector*  GetBaseCG()								{return &CofG;}
  void      CalcNewCG_ISU();
  void      PrintInfo(int bar_cycle);
	double    GetBrakeAcceleration()	  {return accBrake;}
  //--- return lookup tables ------------------------------------
  CFmtxMap       *GetDieh()   {return mdieh;}
  CFmtxMap       *GetPitd()   {return mpitd;}
  CFmtxMap       *GetAcrd()   {return macrd;}
  float						GetVNE ()   {return vne;}
  //--- ATTRIBUTES ---------------------------------------------
protected:
  CVehicleObject *mveh;                   // Parent vehicle
  CVehicleSound  *vsnd;              //
  CFmtxMap       *mdieh;                  // Lookup diehdral
  CFmtxMap       *mpitd;                  // Lookup pitch moment
  CFmtxMap       *macrd;

  CWeightManager        *vehi_wgh;
  Tag       type;
  unsigned int  mxdt;             // Minimum simulation frame rate
  char      acid[64];             // Default tail number
  char      make[64];             // Manufacturer
  char      name[64];             // Aircraft name (for UI menu)
  float     emas;                 // Empty mass (slugs)
  SVector     mine;               // Moments of inertia
  SVector     CofG;               // Default center of gravity
  float     imbl;                 // Imbalance limit
  float     iceR;                 // Ice accumulation rate
  float     wingarea;             // Wing area (sq. ft.)
  float     wingspan;             // Wing span (ft.)
  float     meanchord;
  float     ceiling;
  float     maxcruisespeed;
  float     approachspeed;				// speed in final leg
	float     brakeDist;						// Brake distance to stop
  float     vne;
  float     aoaL;                 // Angle of attack limit
  float     stallaoa;
  float     posG;
  float     negG;
  float     stallspeed;
  float     blst;                 // Local velocity at tail at full throttle
  CVector   newCG_ISU;
	double    accBrake;							// brake speed acceleration in feet/sec²

  Tag       sfxdTag;
  char      sfxdFilename[64];

  SMessage    mAlt;
  SMessage    mSpd;
  SMessage    mVsi;
  SMessage    mMag;
  SMessage    mRpm;
  SMessage    mMap;

  //std::vector <std::string> user_info;
  //std::vector <SMessage> user_message;
  float       elapsed;

public:
  float     wTrbSpeed;            // Wing turbulence
  float     wTrbTimK;
  float     wTrbCeiling;
  float     wTrbDuration;
  float     nTrbSpeed;            // ??? turbulence
  float     nTrbTimK;
  float     nTrbCeiling;
  float     nTrbDuration;
};

//=========================================================================
// CEnginePropeller
//=========================================================================
class CEnginePropeller : public CStreamObject
{
public:
  CEnginePropeller (char eno,CVehicleObject *v);
 ~CEnginePropeller();
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  //-------------------------------------------------
  inline CAcmSpin *GetSpinner()     {return spinner;}
  //---ATTRIBUTES -----------------------------------
protected:
  char                 eNum;    // Engine number
  CVehicleObject      *mveh;    // Mother vehicle
  CAcmSpin            *spinner; // Spinner object
  SVector              dPos;    // Drawing position
};

//========================================================================
// CEngine
// JS:  Make it a derived class from CEngineModel 
// Corresponds to a single <engn> object in the Engine Manager ENG file
//========================================================================
class CEngine : public CDependent
{  //--- ATTRIBUTES --------------------------------------------
protected:
  CVehicleObject *mveh;
  //---- METHODS -----------------------------------------------
public:
  CEngine (CVehicleObject *v,int eNum, char* type, char* name);
  virtual ~CEngine (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);
  //-------------------------------------------------------------
  // CEngine methods
  EMessageResult  ReceiveMessage (SMessage *msg);
  void            Timeslice (float dT,U_INT FrNo);
  void            Probe(CFuiCanva *cnv);
  //-------------------------------------------------------------
  // getters
  const SVector&       GetThrustPosInM();
  const int&           GetPropSpin()    const {return spin;}
  CEngineModel        *GetEngineModel()       {return ngnModel;}
  //---ENGINE Actions -------------------------------------------
  int       AbortEngine(char r);
  int       EngineIdle();
  int       StopEngine(char r);
  int       RefreshState();
  int       CrankEngine();
  int       LaunchEngine();
  int       FailEngine();
	int				MissfireEngine();
  int       StartEngine();
	//--- STATE routines -----------------------------------------
  int       StateStopped();
  int       StateCranking();
  int       StateCatching();
  int       StateFailing();
  int       StateRunning();
  int       StateStopping();
	int				StateMissfire();
  int       StateWindmill();
  //---Fuel management -----------------------------------------
  void      SetFuel(U_CHAR k);
	void			SetMixture(U_INT p);
  bool      SetTAP(CFuelTap *tp);
  //---Parameters management -----------------------------------
  bool                 MsgForMe (SMessage *msg);
  bool                 SetEngineModel();
  void                 ReadEngineParameters();
  //---PLOT interface -------------------------------------------
  int       AddToPlotMenu(char **menu, PLOT_PM *pm, int k);
  bool      PlotParameters(PLOT_PP *pp,Tag id, Tag type);
  //-------------------------------------------------------------
  inline CFuelTap       *GetTapItem()     {return Tap;}
  //-------------------------------------------------------------
  inline bool            EngRunning()     {return eData->EngRunning();}
  //-------------------------------------------------------------
  inline double          GetThrustInNewton () {return (eData->e_Thrust * LBS_TO_NEWTON);}
  inline double          GetTorqueInNM     () {return (eData->e_Torque * LBFFT_TO_NM);}
  inline float           GetThrustXOffset  () {return (eData->e_Pfac);}
  //-------------------------------------------------------------
  inline CVehicleObject *GetVEH()         {return mveh;}
  inline CSoundOBJ      *Sound()          {return sound;}
  inline U_INT           GetEngineNo()    {return eNum;}
  inline float           GetReqFuelFlow() {return (eData->e_gph);}
  //-------------------------------------------------------------
  inline CDependent *GetPart(char k) {return(ngnModel)?(ngnModel->GetPart(k)):(0);}
  //-------------------------------------------------------------
protected:
  CEnginePropeller     *engprp;
  char                  name[80];
  char                  ngnFilename[PATH_MAX];
  int                   spin;
  CVector               bPos;                   // Engine position
  CVector               TPosISU;
  CVector               mPos;                   // Contrail origin
  //---------------------------------------------------------------
  CSoundOBJ            *sound;                  // Engine sounds
  CAcmSpin             *spinner;                // Spinner if any
  //-----Engine components -----------------------------------------
  CEngineData         *eData;                   // Engine data
  U_CHAR               count;                   // Transition delay
  U_CHAR               pstat;                   // Previous state
  int                  cnt1;
  //----------------------------------------------------------------
  std::vector<std::string>  cowlPart;           ///< Vector of movable cowl flap parts

public:
  CEngineModel*         ngnModel;
  CFuelTap *Tap;                                //Fuel tap
};

//==========================================================================================
//! \class Engine Manager
// \brief The ENG file contains the configuration of engine
//   
//
//==========================================================================================
class CEngineManager : public CStreamObject {
  //--- ATTRIBUTES -------------------------------------------------------
protected:
  CVehicleObject *mveh;
  //--- METHODS ----------------------------------------------------------
public:
  CEngineManager (CVehicleObject *v,char* eltFilename);
  virtual ~CEngineManager (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  //---- CEngineManager methods -------------------------------------------
  void          SetTAP(CFuelTap *fs);
  /*! returns the number of engines for the current user */
  U_CHAR  HowMany() {return engine_number;};
  /*! */
  void          Timeslice (float dT,U_INT FrNo);
  void          CutAllEngines();
  void          EnginesIdle();
  void          AbortEngines();
  //---- PLotting interface -----------------------------------------------
  int           AddToPlotMenu(char **menu, PLOT_PM *pm, int k);
  void          PlotParameters(PLOT_PP *pp,Tag id, Tag type);
  //-----------------------------------------------------------------------
  /*! Getters */
  const SVector& GetForceISU  (void);                   ///< returns total force from engine list (LH)
  const SVector& GetMomentISU (void);                   ///< returns total moment from engine list (LH)
  const SVector& GetEnginesPosISU (void);               ///< returns the global position of force (LH)
  const SVector& GetPropellerTorqueISU (void);          ///< returns the global prop torque (LH)
  const float&   GetEnginesPfact (void);                ///< returns P fact in form of X thrust offset (LH)
  //--------------------------------------------------------------------
  inline void    GetAllEngines(std::vector<CEngine*> &egs) { egs = engn;}
	inline U_CHAR  GetEngineNbr()		{return (engine_number);}
  //--------------------------------------------------------------------
public:
  std::vector<CEngine*> engn;                           ///< List of engine instances

protected:
  std::vector<CEngine*>::const_iterator ie;
  U_CHAR engine_number;                           ///< Number of defined engines
  SVector eng_total_force;                              ///< sum all forces from engine list
  SVector eng_total_moment;                             ///< sum all moment from engin list
  SVector eng_total_pos;                                ///< relative position of all the engines
  SVector prop_total_torque;                            ///< sum all prop torque from engin list
  float thrust_X_offset;                                ///< P factor = thrust X offset
};

/*!
 *
 *  Fuel Systems class
 *
 *  The GAS file contains the configuration of fuel tanks, sources, shutoffs,
 *  crossfeeds, etc.
 *
 *  This class is responsible for the fuel management :
 *  It gets and maintains a list of CFuelSubsystem objects (fsub) like cells, pump or tap.
 *  It also sends the fuel weight (gas_wgh) to the CWeightManager class
 *  
 *  ReadFinished
 *  A pointer of a cell fuel is created and saved as a valve object in order
 *  to enable or disable fuel consumption (CFuelSubsystem *related_cell).

 *  TimeSlice :
 *  The gph is retrieved from the proper engine (eng_mng) and distributed to the tap and
 *  various cells and it verify engine has a fuel flow from proper fuel tap.
 *
 */
//=====================================================================================
class CFuiProbe;
//=====================================================================================
//  FUEL SYSTEM
//=====================================================================================
class CFuelSystem : public CStreamObject {
  //--- ATTRIBUTES -------------------------------------------------
  CVehicleObject *mveh;           // Parent vehicle
  //--- METHOS -----------------------------------------------------
public:
   CFuelSystem (CVehicleObject *v,char* gasFilename, CEngineManager *engine_manager,  CWeightManager *wgh);
  ~CFuelSystem (void);

  //---- CStreamObject methods ------------------------------------
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);
  void  Write (SStream *stream);

  //----- CFuelSystem methods--------------------------------------
  void      UpdateComponents(float dT,U_INT frame);
  float     BurnFuelFor(float dT,CEngine *eng,char &ok);
  void      FeedEngines(float dT);
  void      RefillTanks(float dT);
  void      RefillCell( float dT,CFuelCell *tk);
  void      Timeslice (float dT,U_INT FrNo = 0);				// JSDEV*
  void      GetTotalGas(float &cp,float &qt,float &wg);
  void      GetAllCells(std::vector<CFuelCell*> &list);
  float     GetWeight(float qty);
  CFuelSubsystem   *GetSubsystem(Tag t);
  void      LinkAllCells();
  void      Stop();
  //-------------------------------------------------------------------------------
  void      GetAllSystems(std::vector<CFuelSubsystem*> &gass) {gass = fsub;}
  //-------------------------------------------------------------------------------
  inline U_INT      GetGradIndex()  {return gradX;}
  inline void       AddTank(CFuelCell *t) {ctank.push_back(t);}
  //-------------------------------------------------------------------------------
public:
  char                          Tr;                 // Trace Indicator
  EAirportFuelTypes             grad;               // Preferred fuel grade
  U_CHAR                        gradX;              // Grad index
  U_CHAR                        cInd;               // Cell index 
  U_CHAR                        fuel;               // Fuel state
  std::vector<CFuelSubsystem*>  fsub;               // List of CFuelSubsystem* objects
  std::vector<CFuelCell*>       tanks;              // List of tanks
  //-----List of contributing fuel tank -------------------------------------------
  std::vector<CFuelCell *>      ctank;              // Contributing tank
protected:
  CEngineManager       *eng_mng;                    // Link to the CEngine
  CWeightManager       *gas_wgh;
};

//=====================================================================================
// Electrical Subsytems
//
// The AMP file contains a vast array of internal aircraft systems, which define
//   the substantial part of the cockpit avionics and flight displays.  There
//   are typically many (up to hundreds for a complex aircraft) instances of
//   CSubsystem descendents in the subs list within this class.
//=====================================================================================
class CElectricalSystem : public CStreamObject {
  //---ATTRIBUTES -----------------------------------------------------
protected:
  CVehicleObject *mveh;                 // Parent vehicle
  //---METHODS---------------------------------------------------------
public:
  void FreeDLLSubsystem (void);

   CElectricalSystem (CVehicleObject *v,char* ampFilename, CEngineManager *engine_manager);
  ~CElectricalSystem (void);

  /// CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);
  void  Write (SStream *stream);
  void  AddExternal(CSubsystem *sy,SStream *st);
  void  DrawExternal();
  /// CElectricalSystem methods
  CSubsystem* GetFirstSubsystemType (Tag type);
  void		  Timeslice(float dT,U_INT FrNo);					// JSDEV* new implementation
  void      Print (FILE *f);
  CK89gps  *SetKLN89gps(CK89gauge *g);
  ///-----------Registered components ------------------------------------
  inline  void      AddSubsystem(CSubsystem *sys) {subs.push_back(sys); }
  inline  CK89gps  *GetKLN89gps() {return K89GPS;}
  //----------------------------------------------------------------------
  inline CBrakeControl *GetBrakeControl() {return pwb;}
  //----------------------------------------------------------------------
  inline void  GetAllSystems(std::vector<CSubsystem*> &sb) {sb = subs;}
  //---- return all subsystems -------------------------------------------
  inline float GetBrakeForce(char p) {return (pwb)?(pwb->GetBrakeForce(p)):(0);}
  ///---------------------------------------------------------------------
	inline VPilot                 *GetVirtualPilot(){return vpil;}
	inline CFlapControl           *GetFlaps()				{return pFlaps;}
  inline CAileronControl        *GetAilerons()    {return pAils;}
  inline CElevatorControl       *GetElevators()   {return pElvs;}
  inline CRudderControl         *GetRudders()     {return pRuds;}
  inline CElevatorTrimControl   *GetElevatorTrim(){return eTrim;}
	//---------------------------------------------------------------------
	inline  CFPlan                *GetFlightPlan()	{return fpln;}
	inline	CRobot                *GetRobot()				{return d2r2;}
  //---------------------------------------------------------------------
public:
  char     lastID[8];        // Last id   successfully read
  char     lastHW[8];        // last type "            "
  std::vector<CDLLSubsystem*> sdll;   // List of dll subsystems
  std::vector<CSubsystem*>    subs;   // List of electrical subsystems
  std::vector<CSubsystem*>    sext;   // List of external subsystems
  ///----- Shortcut pointers to special subsystem types ------------------
	CFPlan								*fpln;						// Flight Plan
	VPilot								*vpil;						// Virtual pilot
	CRobot                *d2r2;						// Robot
  CAileronControl       *pAils;						// Aileron
  CElevatorControl      *pElvs;						// elevator
  CRudderControl        *pRuds;						// Ruder
  CFlapControl          *pFlaps;					// Flaps
  CAileronTrimControl   *aTrim;
  CElevatorTrimControl  *eTrim;
  CRudderTrimControl    *rTrim;
  CEngineManager        *pEngineManager;
  CBrakeControl         *pwb;             // Wheel brakes
  CGearControl          *pgr;             // gear control
  ///------Registered components ------------------------------------
  CK89gps              *K89GPS;           // K89 GPS
  //-----------------------------------------------------------------
};


//=============================================================================
// Pitot-Static System
//
// The PSS file contains specifications for the system of pitot and static
//   ports on the aircraft.  These special subsystems are used by traditional
//   "steam" gauge subsystems such as altimeters and speed indicators,
//   as well as high-tech air data computers.
//==============================================================================

class CPitotStaticSystem : public CStreamObject {
  //----ATTRIBUTES ------------------------------------
protected:
  CVehicleObject *mveh;           // Parent vehicle
public:
  CPitotStaticSystem (CVehicleObject *v,char* pssFilename);
 ~CPitotStaticSystem (void);

  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);
  void  Write (SStream *stream);

  virtual void   Timeslice         (float dT);
  virtual void   Debug             (void);

public:
  float                           iceT;   ///< Icing condition duration
  std::vector<CPitotStaticPort*>  ports;  ///< List of CPitotStaticPort*
  double _total_pressure_node;            ///< 
};



//================================================================================
/*!
 *
 */

//================================================================================
// Variable Loadouts
//
// The VLD file contains variable loadout positions such as aircrew, passenger
//   and cargo spaces which can affect aircraft weight/balance
//================================================================================
class CVariableLoadouts : public CStreamObject {
protected:
  CVehicleObject *mveh;
public:
  CVariableLoadouts (CVehicleObject *v,char* vldFilename,  CWeightManager *wgh);
 ~CVariableLoadouts (void);

  int   Read (SStream *stream, Tag tag);
  void  Write (SStream *stream);
  //----Attributes -------------------------------------
  CWeightManager *vld_wgh;
};
//---------------------------------------------------------------------------
// Cockpit Manager
//
// The PIT file contains the cockpit manager, which determines which interior
//   panel views exist for the user vehicle, and how they are linked together
//---------------------------------------------------------------------------
class CCockpitManager : public CStreamObject {
  //--- ATTRIBUTES ------------------------------------------
protected:
	CVehicleObject *mveh;                 // Parent vehicle
	//---------------------------------------------------------
  std::map<Tag,CPanel*>				ckpt;   // List of cockpit panels indexed by unique tag
	std::map<Tag,CPanelLight*>	lite;   // List of panel lights
	std::map<Tag,CgHolder*>			hold;		// List of value holder
  char          active;								// Active when camera is cockpit camera
  //---- Pointer to current CPanel --------------------------
	float         brit;									// Panel britnes
  CPanel*       panel;
  CCameraCockpit *cam;								// Cockpit camera
  //--- METHODS-----------------------------------------------
public:
  CCockpitManager (CVehicleObject *v,char* pitFilename);
 ~CCockpitManager (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished ();

  //---- CCockpitManager methods ----------------------------
  void      SetPanel (Tag tag);
  void      ScreenResize();
  void      TimeSlice (float dT);
  void	    PrepareMsg(CVehicleObject *veh);
  CPanel   *GetPanelByTag(Tag id);
	CgHolder *GetHolder(Tag id);
  //---Keyboard events --------------------------------------
  bool      KbEvent(Tag key);               // keyboard order
  bool      MouseMove  (int x,int y);       // Mouse move
  bool      MouseClick (int bt, int ud, int x, int y);
	//---Light management -------------------------------------
	CPanelLight *GetLight(Tag id);
	void			AddLight(SStream *stream);
  //----Statistics ------------------------------------------
  void      GetStats(CFuiCanva *cnv);
  //----Inline ----------------------------------------------
  inline CPanel*   GetCurrentPanel()   {return panel;}
  inline Tag       GetPanel() {return (panel)?(panel->GetId()):(0);}
  inline void      Activity(char a)    {active = a;}
  inline void      SetPanel(CPanel *p) {panel = p;}
  inline const int GetMapCkptSize (void) const {return ckpt.size ();}
  inline const std::map<Tag,CPanel*>& GetMapCkpt (void) const {return ckpt;}
  //---------------------------------------------------------
	inline void			SetBrightness(float b)	{brit = b;}
	inline CVehicleObject *GetMVEH()	{return mveh;}
};

//==============================================================================
// Camera Views list
//
// CCameraViewsList gets a list of the cameras that are listed in DATA/CAMERAS.TXT
// SCameraType is a structure used with the data in CAMERAS.TXT  
//===============================================================================
struct SCameraType {
  char camera_tag [4+1];
  char camera_name[64+1];
  int val;
  SCameraType (void) {
    *camera_tag = 0;
    *camera_name = 0;
    val = 0;
  }
};

class CCameraViewsList {
public:
  SCameraType *cam_type;
  CCameraViewsList (void);
  ~CCameraViewsList (void);

  // methods
  void  FreeList (void);
  int   ReadCamerasFile (void);
  inline const int& GetNumItems (void) const {return num_lines;}
  inline const char* GetTag (const int& i) const
  { if (type) return type[i].camera_tag;
    else return 0;
  }
  inline const char* GetCameraName (const int& i) const
  { if (type) return type[i].camera_name;
    else return 0;
  }
  inline const int GetVal (const int& i) const
  { if (type) return type[i].val;
    else return 0;
  }
  const int PosCameraTag (const char *tag);

private:
  int num_lines;
  SCameraType *type;
};

//==============================================================================
// Camera Manager
//
// CCameraManager links together all of the various static and interactive
//   panel views defined in the CCockpitManager (.PIT file).
//===============================================================================
class CCameraManager : public CStreamObject {
public:
   CCameraManager (CVehicleObject *veh,char* fn);
  ~CCameraManager (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);

  //----- CCameraManager ------------------------------------------------
  void      BindKeys();
  void      ZeroRate();
  void      DefaultCameras();
  void      ExplicitCameras(int nb);
  void      Link(CCamera *cam,int k,int last);
  void      UpdateCamera (SPosition tgtPos, SVector tgtOrient,float dT);
  void      AdjustRange(double lg);
  //----------------------------------------------------------------------------
  void      NextCamera (void);
  void      PrevCamera (void);
  CCamera  *SelectCamera (Tag id);
  CCamera  *GetCamera(Tag id);
  CCameraCockpit *GetCockpitCamera();
	void			RestoreCamera(CAMERA_CTX &ctx);
	//----------------------------------------------------------------------------
	CRabbitCamera *SetRabbitCamera(CAMERA_CTX &ctx);
  //----------------------------------------------------------------------------
  void      KbEvent(Tag id);                // Keyboard command
  bool      KeyCameraCockpitEvent(int id);
  //----------------------------------------------------------------------------
  void      Print (FILE *f);
  //----------------------------------------------------------------------------
  inline    CCamera*  GetActiveCamera ()     {return aCam;}
  //----------------------------------------------------------------------------
protected:
  ///----------Attributes ------------------------------------------------------
	CVehicleObject *mveh;												// Mother vehicle
  CCamera  *aCam;                             // Active camera
  std::map<Tag,CCamera*>   came;              // List of standard cameras
  Tag       tCam;                             // Tag of current camera
  /// Current camera parameters
  SPosition tgtPos;
  SVector   tgtOrient;
  char      Internal;                         // Camera is internal if (1)
  ///--------Cameras list from DATA\CAMERAS_LEGACY.TXT --------------------------
  CCameraViewsList cam_list;
};
//========================================================================
//
// Radio Manager
//========================================================================
class CRadioManager : public CStreamObject {
public:
  CRadioManager (const char* rdoFilename);

  int   Read (SStream *stream, Tag tag);
//  void  ReadFinished (void);
//  void  Write (SStream *stream);

public:
};



//===========================================================================
// Control Mixer
//===========================================================================
class CAeroControlChannel : public CStreamObject {
public:
  CAeroControlChannel (char * name);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CAeroControlChannel methods
  float         Value (float value);
  //------------------------------------------------------------------
  inline bool SameName(char *n)   {return (0 == strncmp(chn,n,31));}
  inline void SetDeflect(float f) {deflect = f;}
  inline void SetScaled (float f) {scaled  = f; radians = DegToRad(f);}
  inline void SetKeyframe(float f){keyframe = f;}
  //------------------------------------------------------------------
  inline float GetRadians()         {return radians;}
  inline float GetKeyframe()        {return keyframe;}
  //------------------------------------------------------------------
  inline float GetPos()             {return pos;}
  inline float GetNeg()             {return neg;}
  //------------------------------------------------------------------
  inline char *GetName()            {return chn;}
  //----ATTRIBUTES ---------------------------------------------------
protected:
  char    chn[32];  // Channel name
  float   pos;      ///< Positive mixer percentage
  float   neg;      ///< Negative mixer percentage
  //------Mixer values -----------------------------------------------
  float   deflect;    // Raw values  (in [-1,+1]
  float   scaled;     // Scaled value in  degre
  float   radians;    // Scaled in radian
  float   keyframe;   // Associated keyframe
};
//===========================================================================
//  Control mixer channel
//===========================================================================
class CControlMixerChannel : public CStreamObject {
  //----ATTRIBUTES ------------------------------------
protected:
  CVehicleObject *mveh;
  //----METHODS----------------------------------------
public:
  CControlMixerChannel (CVehicleObject *v);
 ~CControlMixerChannel (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  CAeroControlChannel *GetRudder();
  // CControlMixer methods
  void  Timeslice (float dT,U_INT FrNo);
  void  LinktoWing();
  void  SetName(char *n);
  //------------------------------------------------------------
  inline int NumberItem()   {return aerochannel.size();}
  //------------------------------------------------------------
public:
  char        name[16];
  int         group;     ///< Group number
  bool        invert;    ///< Whether to invert controls before passing to aero model
  SMessage    msg;       ///< Control subsystem message

  /// Aero-model control channels create linkage to WNG file elements
  //  std::map<std::string, CAeroControlChannel*>  aerochannel;
  std::vector<CAeroControlChannel*>  aerochannel;

};
//===========================================================================
//  Control mixer
//===========================================================================
class CControlMixer : public CStreamObject {
protected:
  float rPos;             // Coupling rudder positive value
  float rNeg;             // Coupling rudder negative value
  //---ATTRIBUTES -----------------------------------------
protected:
  CVehicleObject *mveh;
  //---METHODS--------------------------------------------
public:
  CControlMixer (CVehicleObject *v,char* mixFilename);
 ~CControlMixer (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  //virtual void  ReadFinished (void);

  // CControlMixer methods
  void  AddMixer(CControlMixerChannel *mix, char *name);
  void  Timeslice (float dT,U_INT FrNo);

public:
  std::set<std::string>                       channel;   ///< Control channel names
  std::map<std::string,CControlMixerChannel*> mixerMap;  ///< Map of mixer channels
};


//===========================================================================
// Checklists
//===========================================================================
class CChecklists : public CStreamObject {
public:
  CChecklists (const char* eltFilename);

  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);
  void  Write (SStream *stream);

public:
};



//
// Slope Wind Data
//
class CSlopeWindData : public CStreamObject {
public:
  CSlopeWindData (const char* swdFilename);

  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);
  void  Write (SStream *stream);

public:
};


//
// CVehicleHistory
//
// Stream object stored in a vehicle's .HST history file
//

class CVehicleHistory : public CStreamObject {
public:
  CVehicleHistory (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CVehicleHistory methods
  bool  HaveHobbsMeterData (void) const {return readHobbsMeter;}
  float GetHobbsMeter (void);
  bool  HaveTachTimerData (void) const {return readTachTimer;}
  float GetTachTimer (void);
  int   GetRepairCount (void);
  float GetAileronExponential (void);
  float GetElevatorExponential (void);
  float GetRudderExponential (void);
  float GetAileronTrimStep (void);
  float GetElevatorTrimStep (void);
  float GetRudderTrimStep (void);
  
protected:
  bool              readHobbsMeter; ///< Whether <hobs> tag was read
  float             hobbsMeter;     ///< Hobbs meter setting
  bool              readTachTimer;  ///< Whether <tach> tag was read
  float             tachTimer;      ///< Tachometer timer setting
  int               repairCount;    ///< Number of times aircraft has been repaired
  float             ailrExp;        ///< Aileron exponential
  float             elvrExp;        ///< Elevator exponential
  float             rudrExp;        ///< Rudder exponential
  float             ailrTrimStep;   ///< Aileron trim step per keypress
  float             elvrTrimStep;   ///< Elevator trim step per keypress
  float             rudrTrimStep;   ///< Rudder trim step per keypress
};


/**
 * @brief Encapsulation of vehicle info in .NFO file
 */

class CVehicleInfo : CStreamObject {
public:
  // Constructor
  CVehicleInfo (char* nfoFilename);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CVehicleInfo methods
  char* GetSVH (void);
  char* GetGAS (void);
  char* GetWNG (void);
  char* GetAMP (void);
  char* GetPSS (void);
  char* GetWHL (void);
  char* GetVLD (void);
  char* GetPIT (void);
  char* GetCAM (void);
  char* GetLOD (void);
  char* GetRDO (void);
  char* GetELT (void);
  char* GetENG (void);
  char* GetRTR (void);
  char* GetMIX (void);
  char* GetCKL (void);
  char* GetFCS (void);
  char* GetSWD (void);
  char* GetSIT (void);
  char* GetPID (void);
  char* GetPHY (void); // PHY file
  //------------------------------------------------------------
  inline  int       GetVehClass()     {return classification;}
  inline  char     *GetVehMake()      {return make;}
  inline  char     *GetVehIcon()      {return iconFilename;}
  //------------------------------------------------------------
protected:
  Tag   type;             ///< Vehicle type
  char  make[64];         ///< Make/model of aircraft
  char  iconFilename[64]; ///< Filename of PBM file for aircraft select menu
  int   classification;   ///< Vehicle classification
  int   usage;
  char  svhFilename[64];
  char  gasFilename[64];
  char  wngFilename[64];
  char  ampFilename[64];
  char  pssFilename[64];
  char  whlFilename[64];
  char  vldFilename[64];
  char  pitFilename[64];
  char  camFilename[64];
  char  lodFilename[64];
  char  rdoFilename[64];
  char  eltFilename[64];
  char  engFilename[64];
  char  rtrFilename[64];
  char  mixFilename[64];
  char  cklFilename[64];
  char  fcsFilename[64];
  char  swdFilename[64];
  char  sitFilename[64];
  char  pidFilename[64];
  char  phyFilename[64];  ///<  // PHY file : data adj. for TRI aeromodel 

};

#endif // USERVEHICLES_H
