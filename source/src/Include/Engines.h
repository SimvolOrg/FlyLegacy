//==============================================================================================
// Version.h
//
// Part of Fly! Legacy project
//
// Copyright 2003 Chris Wallace
// Copyright 2005 Jean Sabatier
// Fly! Legacy is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
// Fly! Legacy is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
//   along with Fly! Legacy; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//================================================================================================
#ifndef ENGINES_H
#define ENGINES_H
//================================================================================================
#include "../Include/FlyLegacy.h"
#include "../Include/BaseSubsystem.h"
#include "../Include/Utility.h"
//===================================================================================
//  Structure Engine data is shared between the engine representation
//            and the physical model like CPiston
//	NOTE:	startkit integrates all running conditions for the engine such as
//				-Magneto 1 and 2
//				-Starter on/off
//				-Mixture OK
//				-Engine fueled
//				etc...
//===================================================================================
//====================================================================================
// *
// * Engine subsystems
// *
//====================================================================================
typedef enum {
  MAGNETO_SWITCH_OFF    = 0,
  MAGNETO_SWITCH_RIGHT  = 1,
  MAGNETO_SWITCH_LEFT   = 2,
  MAGNETO_SWITCH_BOTH   = 3,
  MAGNETO_SWITCH_START  = 4,
  MAGNETO_SWITCH_BACK   = 5
} EMagnetoSwitch;
typedef enum {
  ENGINE_MAGN_01 = 0x01,
  ENGINE_MAGN_02 = 0x02,
  ENGINE_MAGN_12 = 0x03,
  ENGINE_STARTER = 0x04,
  ENGINE_STR_ALL = 0x07,
  ENGINE_IGNITER = 0x08,
  ENGINE_PRIMER  = 0x10,
  ENGINE_FUELED  = 0x20,
	ENGINE_MIXTURE = 0x40,
} START_ITEM;
typedef enum {
  ENGINE_MAX = 8,
}EEngineProperty;
//====================================================================================
//	Common engine data Used by modules
//====================================================================================
class CEngineData {
public:
  //---Mother engine ------------------------------------------
  CEngine *meng;                        // Mother engine
  U_CHAR  rfu1;                         // reserved
  U_CHAR  powered;                      // Powered engine
	//--- Running conditions --------------------
	U_INT   startKit;											// running conditions
  //----Overall state -------------------------
  U_CHAR  e_stop;                       // Cause of stopping
  U_CHAR  e_state;                      // Engine state
  //--- Control value -------------------------
  float   c_mixt;       // used with engines without mixture knob
  float   e_thro;                       // Throttle control value
  float   e_revr;                       // Reverse control value
  float   e_mixt;                       // Mixture control value
  float   e_blad;                       // Blad from controler
  //----Output values -------------------------
  float   e_hob;                        // Hobb value
	float		e_Boost;											// Booster
  float   e_Map;                        // Manifold presure (P)
  float   e_hMap;                       // Manifold presure (Hg)
  float   e_gph;                        // fuel requested (galon per hour)
  float   e_rpm;                        // Engine RPM
  float   e_vel;                        // Aircraft velocity
  float   e_oilT;                       // Oil temperature
  float   e_oilP;                       // Oil Presure
  float   e_EGTa;                       // Exhaust Gas Temp versus blad
  float   e_EGTr;                       // Exhaust Gas temp versus rpm
  float   e_CHTa;                       // Cylinder Head Temp versus blad
  float   e_CHTr;                       // Cylinder Head Temp versus rmp
  float   e_Thrust;                     // Engine trust
  float   e_Torque;                     // Engine torque
  float   e_HP;                         // Raw Power
  float   e_PWR;                        // Power available
  float   e_af;                         // Air flow
  float   e_Pfac;
  //----Propeller parameters -----------------------------------
  float   e_pLift;                      // Lift
  float   e_pDrag;                      // Drag
	float   e_minRv;										  // Minimum RPM
	float   e_maxRv;											// Maximum RPM
	float		e_govrn;											// Governor gain
  //----Sound ratio --------------------------------------------
  float   e_Srat;                       // Sound ratio
  //----Piston engine parameters -------------------------------
  float   e_wml;                        // Engine windmill as computed
  float   s_wml;                        // Starting value
  float   s_rpm;                        // stalling rpm
  //------------------------------------------------------------
  float   dT;                           // Time step
  //------------------------------------------------------------
  double  e_hpaf;                       // Horse power
  double  e_fttb;
  double  IAS;                          // IAS in knots
  //-----TRI data ----------------------------------------------
  float   ffsc;
  float   idle;
  float   rbhp;
  double  irpm;
  double  bost_alt;
  double  bost_pr;
  int place_holder;
  //---METHODS -------------------------------------------------
  CEngineData(CEngine *m);
  //------------------------------------------------------------
  inline bool EngRunning()  {return (e_state == ENGINE_RUNNING);}
  inline bool EngStopped()  {return (e_state == ENGINE_STOPPED);}
  inline bool EngCranking() {return (e_state == ENGINE_CRANKING);}
  inline bool EngWindmill() {return (e_state == ENGINE_WINDMILL);}
  inline bool EngFueled()   {return (startKit & ENGINE_FUELED) != 0;}
  inline bool EngPowered()  {return (powered != 0);}
  //----For ests subsystem -------------------------------------
public:
  inline char   EngState()  {return e_state;}
  inline float  EngRPM()    {return e_rpm;}
  //-------------------------------------------------------------
  float Pitch();      
  //------------------------------------------------------------
};
//===================================================================================
//  ENGINE SUBSYSTEM
//===================================================================================
class CEngineModel : public CDependent {
public:
  CEngineModel (void);
  virtual ~CEngineModel (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName    (void) { return "CEngineModel"; }

  // virtual functions for engine models
  virtual void        eTimeslice      () {;}
  virtual void        eCalculate      (float dT) {;}
  virtual void        SetBoosted      (void) {;}
  virtual void        setMinMaxMAP    (const double &min_inHg, const double &max_inHg) {;}
  virtual void        SetTrueRPM()    {return;}
  virtual void        Abort()         {;}
	virtual void				Reset()					{;}
  virtual void        Idle()          {;}
  //-----Plotting interface -----------------------------------------
  virtual int  AddToPlotMenu(char **menu, PLOT_PM *pm, int k){return 0;}
  virtual bool PlotParameters(PLOT_PP *pp,Tag id, Tag type) {return false;}
  //------------------------------------------------------------------
  virtual CDependent *GetPart(char k) {return this;}
  //------------------------------------------------------------------
  inline const float&        GetRRPM         (void) const {return rrpm;}
  inline const float&        GetIRPM         (void) const {return irpm;}
  //------------------------------------------------------------------
  inline bool   EngStopped()    {return eData->EngStopped();}
  inline bool   EngPowered()    {return eData->EngPowered();}
  inline bool   EngRunning()    {return eData->EngRunning();}
  inline bool   EngCranking()   {return eData->EngCranking();}
  inline bool   EngFueled()     {return eData->EngFueled();}
  inline float  GetERPM()       {return eData->e_rpm;}
  inline void   SetEngineData(CEngineData *d) {eData = d;}
  //------------------------------------------------------------------
protected:
  // NGN engine common values
  CEngineData   *eData;             // Engine data
  float vne;
  float irpm, srpm;
  float rbhp, rrpm;
  float nGt_;
  float cowT, cowD;
  CFmtxMap *mpoilp;
  CFmtxMap *mpoilt;

private:
  // external values
};
//===================================================================================
class CPropellerModel;
//===================================================================================
//
//  CTurbopropEngineModel
//
//  This class acts as a CEngineModel specialisation for turboprop engines
//===================================================================================
class CTurbopropEngineModel : public CEngineModel {
public:
  CTurbopropEngineModel (CVehicleObject *v,int eno);
  virtual ~CTurbopropEngineModel (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTurbopropEngineModel"; }
  virtual void        TimeSlice (float dT,U_INT FrNo = 0);			// JSDEV*

protected:
  bool read_file_flag; // avoid multiple reading of NGN file

  CPropellerModel   *prop;      ///< Propeller model
  
  float runupTimeK;             ///< Runup time constant
  float rundownTimeK;           ///< Rundown time constant
  float starterTimeK;           ///< Starter time constant
  float spoolTimeK;             ///< Spool time constant
  float betaRange;              ///< Beta range
  float betaBrake;              ///< Beta brake (hp)
  float betaHigh;               ///< Maximum blade angle (beta range forward) (degrees)
  float betaMid;                ///< Mid blade angle (beta range zero) (degrees)
  float betaLow;                ///< Minimum blade angle (beta range reverse) (degrees)
  float designRPM;              ///< Design RPM
  float designInletTemp;        ///< Design inlet temperature
  float designInletMach;        ///< Design inlet Mach number
  float designInletPressure;    ///< Design inlet pressure
  float conditionerLow;         ///< Power conditioner (low)
  float conditionerHigh;        ///< Power conditioner (high)
  float lowIdleN1;              ///< Low idle N1
  float highIdleN1;             ///< High idle N1
  float lowMaxN1;               ///< Low max N1
  float highMaxN1;              ///< High max N1
  float governorGain;           ///< Governor gain
  float governorLimit;          ///< Governor limit
  float designCombustionTemp;       ///< Design combustion temperature
  float designHeatingValue;         ///< Design heating value for fuel
  float designAirMassFlow;          ///< Design air mass flow
  float designCompressorRatio;      ///< Design compressor pressure ratio
  float designDiffuserRatio;        ///< Design diffuser pressure ratio
  float designBurnerRatio;          ///< Design burner pressure ratio
  float designNozzlePressureRatio;  ///< Design nozzle pressure ratio
  float designHPTPressureRatio;     ///< Design high pressure turbine pressure ratio
  float designHPTTempRatio;         ///< Design high pressure turbine temperature ratio
  float compSHR;                    ///< Compressor specific heat ratio
  float turbSHR;                    ///< Turbine specific heat ratio
  float compSHRConstPressure;       ///< Compressor specific heat ratio constant pressure
  float turbSHRConstPressure;       ///< Turbine specific heat ratio constant pressure
  float compEff;                    ///< Compressor efficiency
  float turbEff;                    ///< Turbine efficiency
  float mechEff;                    ///< Mechanical efficiency
  float burnEff;                    ///< Burner efficiency
};
//===================================================================================
//
//  CTurbofanEngineModel
//
// This class acts as a CEngineModel specialisation for turbofan engines
//===================================================================================
class CTurbofanEngineModel : public CEngineModel {
public:
  CTurbofanEngineModel (CVehicleObject *v,int eNo);
  virtual ~CTurbofanEngineModel (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTurbofanEngineModel"; }
  virtual void        TimeSlice (float dT,U_INT FrNo = 0);			// JSDEV*

protected:
  bool read_file_flag; // avoid multiple reading of NGN file
};
//=====================================================================================
//!
//
//  CTurbineEngineModel
//
//  This class acts as a CEngineModel specialisation for turbine engines
//
//=====================================================================================
class CTurbineEngineModel : public CEngineModel {
public:
  CTurbineEngineModel (CVehicleObject *v,int eNo);
  virtual ~CTurbineEngineModel (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTurbineEngineModel"; }
  virtual void        TimeSlice (float dT,U_INT FrNo = 0);		// JSDEV*

protected:
  bool read_file_flag; // avoid multiple reading of NGN file
};
//=====================================================================================
//
//  CTurboshaftAPUEngineModel
//
//  This class acts as a CEngineModel specialisation for APU
//=====================================================================================
class CTurboshaftAPUEngineModel : public CEngineModel {
public:
  CTurboshaftAPUEngineModel (CVehicleObject *v, int eNo);
  virtual ~CTurboshaftAPUEngineModel (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTurboshaftAPUEngineModel"; }
  virtual void        TimeSlice (float dT, U_INT FrNo = 0);		// JSDEV*

protected:
  bool read_file_flag; // avoid multiple reading of NGN file
};
//=====================================================================================
//
//  CSimplisticJetEngineModel
//
//  This class acts as a CEngineModel specialisation for turbine engines
//=====================================================================================
class CSimplisticJetEngineModel : public CEngineModel {
public:
  CSimplisticJetEngineModel (CVehicleObject *v, int eNo);
  virtual ~CSimplisticJetEngineModel (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CSimplisticJetEngineModel"; }
  virtual void        TimeSlice (float dT,U_INT FrNo = 0);		// JSDEV*

protected:
  bool read_file_flag; // avoid multiple reading of NGN file
};
//========================================================================
// CEngine control:
//  BASE for all susbsystem related to engine control
//========================================================================
class CEngineControl: public CDependent {
protected:
  //---ATTRIBUTES ------------------------------------
  SMessage  engm;          // Engine message
  float     step;
  U_CHAR    conv;          // how to display values, whether F° or C°
  //---METHODS ---------------------------------------
  int   Read (SStream *stream, Tag tag);
  EMessageResult ReceiveMessage(SMessage *msgr);
  //--------------------------------------------------
  void  ReadFinished();
  void  PollEngine(U_CHAR c);
  void  Monitor(Tag tag);
public:
  CEngineControl();
  virtual bool  MsgForMe (SMessage *msg);
  //--------------------------------------------------
  inline float Clamp(float v)
  { if (v < 0) return 0;
    if (v > 1) return 1;
    return v;
  }
  //--------------------------------------------------
  inline void Incr() {indnTarget = Clamp(indnTarget + step);}
  //--------------------------------------------------
  inline void Decr() {indnTarget = Clamp(indnTarget - step);}
};
//==========================================================================
// CEngineStatus
//
// This subsystem monitors the status of a single engine, and sets its
//   indication value based on whether the engine is stopped, starting or
//   running
//==========================================================================
class CEngineStatus : public CEngineControl {
public:
  CEngineStatus (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  char  DecodeMode(char *md);
  void  GetEngineState();
  EMessageResult ReceiveMessage(SMessage *msgr);
  // CSubsystem methods
  const char* GetClassName (void) { return "CEngineStatus"; }

protected:
  SMessage  emsg;         // Engine message
  char mode;              ///< Monitoring mode
};
//===========================================================================
// CEngineProxy:  This is for the amp subsystem 'ngn' object
//    This object is supposed to be a representative of the associated
//    engine number.  Any message adressed to it is relayed to the
//    real engine. Thus the proxy name.
//===========================================================================
class CEngineProxy: public CEngineControl {
  //----Methods ------------------------------
public:
  CEngineProxy();
  int   Read (SStream *stream, Tag tag);
  // CSubsystem methods
  const char* GetClassName (void) { return "CEngineProxy"; }
  //----------------------------------------------------------
  EMessageResult  ReceiveMessage (SMessage *msg);
};
//===========================================================================
// CPrimeControl
//===========================================================================
class CPrimeControl : public CEngineControl {
public:
  CPrimeControl (void);
  void            ReadFinished();
  EMessageResult  ReceiveMessage (SMessage *msg);
  // CSubsystem methods
  const char* GetClassName (void) { return "CPrimeControl"; }

protected:
  int mask[2];          // State mask
};
//===========================================================================
// CRotaryIgnitionSwitch
//===========================================================================
class CRotaryIgnitionSwitch : public CEngineControl {   
public:
  CRotaryIgnitionSwitch (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);
  // CSubsystem methods
  const char* GetClassName (void) { return "CRotaryIgnitionSwitch"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  void            TimeSlice (float dT,U_INT FrNo);			// JSDEV*
	void						Probe(CFuiCanva *cnv);
	//---Published values on Gauge BUS ------------------------------
	inline	int		  GaugeBusIN03()		{return rot_pos;}
	//---------------------------------------------------------------
protected:
  SMessage      meng;   // Engine state
  float         sAmp;   // magnitude of amperage required to operate the starter
  char          rot_pos;

};
//===========================================================================
// CMagnetoControl
//===========================================================================
class CMagnetoControl : public CEngineControl {
public:
  CMagnetoControl (void);

  // CStreamObject methods
  void  ReadFinished();
  // CSubsystem methods
  const char* GetClassName (void) { return "CMagnetoControl"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  //---- Attributes ---------------------------------------------
protected:
  int       mask[2];            // Mask value
  Tag       NoMag;              // Magneto number
};
//=========================================================================
// CStarterControl
//=========================================================================
class CStarterControl : public CEngineControl {
public:
  CStarterControl (void);
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CSubsystem methods
  const char* GetClassName (void) { return "CStarterControl"; }
  EMessageResult  ReceiveMessage(SMessage *msg);
  void            ArmTimer (SMessage *msg);
  void            TimeSlice(float dT, U_INT FrNo);
protected:
  U_INT     time;           // Timer
  bool      mTerm;          ///< Manual starter termination
  int       mask[2];
};
//===========================================================================
// CIgniterControl
//===========================================================================
class CIgniterControl : public CEngineControl {
public:
  CIgniterControl (void);
  void  ReadFinished();
  // CSubsystem methods
  const char*       GetClassName (void) { return "CIgniterControl"; }
  EMessageResult    ReceiveMessage (SMessage *msg);
  //----Attributes ----------------------------------------
  int       mask[2];
};
//=======================================================================
// CThrottleControl
// * The throttle position is retrieved by the eMap subsystem
// in the skyhawk system
//=======================================================================
class CThrottleControl : public CEngineControl {
	//--- Attribute ------------------------------------------
	CEngineData  *data;
public:
  CThrottleControl (void);
  void  ReadFinished();
	void	PrepareMsg(CVehicleObject *veh);
	void	Target(float v);
  // CSubsystem methods
  const char* GetClassName (void) { return "CThrottleControl"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
};
//===============================================================================
class CRudderControl;
class TaxiRoute;
//===============================================================================
// Class SpeedRegulator to control Throttle
//===============================================================================
class CSpeedRegulator: public CDependent {
	//--- Attributes -------------------------------------------------
protected:
	//----------------------------------------------------------------
	double    limit;							// Near distance to reach position
	double    fdist;							// Feet Distance to position
	double		aSPD;								// Actual speed
	double		speed;							// Target speed
	double		hdg;								// Heading
	double		ref;								// Reference
	double		aErr;								// angular Error
	double    cor;								// Correction to rudder
	double		val;								// Value
	//-----------------------------------------------------------------
	SMessage	msg;								// Message for throttle up to 4 engines
	//-----------------------------------------------------------------
	TaxiRoute *route;							// Taxiway route
	CThrottleControl *thro[4];		// Throttle subsystems
  CRudderControl *rudS;         // Rudder surface
	CPIDbox   *sPID;							// PID controller for speed
	CPIDbox   *rPID;							// PID controller for rudder
	CJoysticksManager *jsm;				// Joystick controller
	SPosition  tgp;								// Target position
	//-----------------------------------------------------------------
	CFmt1Map   angs;							// Angle to speed
	CFmt1Map	 srat;							// speed ratio = f(distance)
	//-----------------------------------------------------------------
	char			 steer;							// Steering indicator
	char       rfu1;							// Reserved
	//-----------------------------------------------------------------
public:
	CSpeedRegulator::CSpeedRegulator();
	void  PrepareMsg(CVehicleObject *veh);
	int   Read (SStream *stream, Tag tag);
	void	GetThrottle(int u);
	//-----------------------------------------------------------------
	void	RouteTo(TaxiRoute *R);
	void	SteerTo(SPosition &P);
	void	SteerOFF();
	void	SetOFF();
	bool	SetON(U_INT C);
	bool	SetSpeed(double sp)	{speed = sp; return (state != 0);}
	void	RudderControl(float dT);
	void	RouteControl();
	//-----------------------------------------------------------------
	bool	NoSteering()				{return (steer == 0);}
	bool	IsSteering()				{return (steer != 0);}
	//-----------------------------------------------------------------
	double FeetDistance()			{return fdist;}
	double TaxiSpeed();
	double ActualSpeed()			{return aSPD;}
	//-----------------------------------------------------------------
	void  TimeSlice(float dT, U_INT FrNo);
	//-----------------------------------------------------------------
	void	Probe(CFuiCanva *cnv);

};

//==========================================================================
// CThrustReverseControl
//==========================================================================
class CThrustReverseControl : public CEngineControl {
public:
  CThrustReverseControl (void);
  void  ReadFinished();
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  EMessageResult  ReceiveMessage (SMessage *msg);
  // CSubsystem methods
  const char* GetClassName (void) { return "CThrustReverseControl"; }

protected:
  float   dTime;           ///< Deployment time (sec)
};
//=======================================================================
// CMixtureControl
//=======================================================================
class CMixtureControl : public CEngineControl {
public:
  CMixtureControl (void);
  // CSubsystem methods
  const char* GetClassName (void) { return "CMixtureControl"; }
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
	void	PrepareMsg(CVehicleObject *veh);
  EMessageResult  ReceiveMessage (SMessage *msg);
	int		SetControl(float val);
	//--------------------------------------------------------
protected:
	CEngineData	*data;
  bool  autoControl;    // Under automatic or user control?
  bool  injd;       // Fuel injected system (vs carbeurated)
  float prmr;       // Priming rate
  float prff;       // Priming fuel flow
  float plim;       // Priming pump limit
};
//=======================================================================
// CPropellerControl
//=======================================================================
class CPropellerControl : public CEngineControl {
public:
  CPropellerControl (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CSubsystem methods
  const char* GetClassName (void) { return "CPropellerControl"; }
	void	PrepareMsg(CVehicleObject *veh);
  EMessageResult  ReceiveMessage  (SMessage *msg);
  //---ATTRIBUTES ----------------------------------------------
protected:
	CEngineData *data;				// Shared engine data
  float   minRPM;           ///< Minimum governable engine RPM
  float   maxRPM;           ///< Maximum governable engine RPM
  float   governorGain;     ///< Governor gain
  bool    autoFeather;      ///< Auto-feather enable
};
//===============================================================================
// CIdleControl:  TO BE IMPLEMENTED WHEN NEEDED
//===============================================================================
class CIdleControl : public CEngineControl {
public:
  CIdleControl (void);

  // CSubsystem methods
  const char* GetClassName (void) { return "CIdleControl"; }

protected:
};
//===================================================================================
// CTachometer
//===================================================================================
class CTachometer : public CEngineControl{
//---ATTRIBUTES -----------------------------------------------
  SMessage emsg[ENGINE_MAX];
//---METHODS --------------------------------------------------
public:
  CTachometer (void);
  void      ReadFinished();
  bool      GetRPM(SMessage *msg);
  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CTachometer"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
};
//===================================================================================
// CTachometerTimer
//===================================================================================
class CTachometerTimer : public CEngineControl {
public:
  CTachometerTimer (void);
  // CStreamObject methods
  void    ReadFinished();
  // CSubsystem methods
  const char* GetClassName (void) { return "CTachometerTimer"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
protected:
};
//===================================================================================
// COilTemperature
//===================================================================================
class COilTemperature : public CEngineControl{
public:
  COilTemperature (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CSubsystem methods
  const char*		  GetClassName (void) { return "COilTemperature"; }
  EMessageResult	ReceiveMessage (SMessage *msg);
	//--- Published values on Gauge BUS -----------------------------
	float	GaugeBusFT01()						{return indn;}
  //---------------------------------------------------------------- 
protected:
};
//===================================================================================
// COilPressure
//===================================================================================
class COilPressure : public CEngineControl {
public:
  COilPressure (void);

  //--- CStreamObject methods --------------------------------
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  //--- CSubsystem methods ------------------------------------
  const char*		GetClassName (void) { return "COilPressure"; }
  EMessageResult	ReceiveMessage (SMessage *msg);		// JSDEV*
  //--- Published values on Gauge BUS -------------------------
	float GaugeBusFT01()			{return indn;}		// Pressure;
protected:
	float	lowP;						// Low pressure threshold
	bool	plow;						// Low indicator
};
//====================================================================
// CManifoldPressure
//=====================================================================
class CManifoldPressure : public CEngineControl {
public:
  CManifoldPressure (void);
  void  ReadFinished();
  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CManifoldPressure"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
};
//======================================================================
// CCylinderHeadTemp
//======================================================================
class CCylinderHeadTemp : public CEngineControl {
public:
  CCylinderHeadTemp (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CSubsystem methods
  const char* GetClassName (void) { return "CCylinderHeadTemp"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  // 
protected:
  U_CHAR  conv;                 // how to display values, whether F° or C°
};
//=======================================================================
// CExhaustGasTemperature
//=======================================================================
class CExhaustGasTemperature : public CEngineControl {
public:
  CExhaustGasTemperature (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CSubsystem methods
  const char*		GetClassName (void) { return "CExhaustGasTemperature"; }
  EMessageResult	ReceiveMessage (SMessage *msg);
	//--- Published values on gauge BUS ----------------------------------
	float	GaugeBusFT01()			{return indn;}		// Temperature
  //--- ATTRIBUTES ----------------------------------------------------- 
protected:
  U_CHAR conv;                 // how to display values, whether F° or C°
};
//============================================================================
// CHobbsMeter
//============================================================================
class CHobbsMeter : public CEngineControl {
public:
  CHobbsMeter (void);
  void    ReadFinished();
  // CStreamObject methods
  // CSubsystem methods
  const char* GetClassName (void) { return "CHobbsMeter"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  // CHobbsMeter methods
protected:
};
//==================================================================================
// CEngineMonitor
//==================================================================================
class CEngineMonitor : public CEngineControl{
public:
  CEngineMonitor (void);

  // CStreamObject methods
  int Read (SStream *stream, Tag tag);

  // CSubsystem methods
  const char* GetClassName (void) { return "CEngineMonitor"; }

protected:
  SMessage    mMAP;       ///< Message connection to manifold pressure
  SMessage    mRPM;       ///< Message connection to tachometer
  SMessage    mTIT;       ///< Message connection to turbine interstage temp
  SMessage    mFMS;       ///< Message connection to fuel management system
  SMessage    mOT;        ///< Message connection to oil temperature
  SMessage    mOP;        ///< Message connection to oil pressure
  SMessage    mCHT;       ///< Message connection to cylinder head temp
  SMessage    mVAC;       ///< Message connection to vacuum gauge
  SMessage    mLQTY;      ///< Message connection to left fuel quantity
  SMessage    mRQTY;      ///< Message connection to right fuel quantity
};
//=======================================================================
// CPneumaticPump
//=======================================================================
class CPneumaticPump : public CEngineControl {
public:
  CPneumaticPump (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished   (void);
  // CSubsystem methods
  const char* GetClassName (void) { return "CPneumaticPump"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  void TimeSlice (float dT,U_INT FrNo = 0);			// JSDEV*
  void Probe(CFuiCanva *cnv);

protected:
  float             rpm;
  float             suct;				      //< Pump suction (psi) at Max RPM
//  unsigned int pump_eNum;				    //< specifies the index of the engine that this pump depends
  int               dIce;				      //< the pressure required to operate the de-ice inflatable boots
  int               Lrpm;				      //< the least RPM required to create sufficient succion
  int               Hrpm;				      //< the greatest RPM tolerated to create sufficient succion
  float             mSct;				      //< the suction generated at the Least Operative RPM
  float				     Coef;				      //< proportional coef to RPM
};
//=====================================================================
// CEngineAntiIceV1
//  NOT YET IMPLEMENTED
//=====================================================================
class CEngineAntiIceV1 : public CEngineControl{
public:
  CEngineAntiIceV1 (void);

  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CEngineAntiIceV1"; }
};
//============================================================================
// CThrottleControlV1
//  NOT YET IMPLEMENTED
//============================================================================
class CThrottleControlV1 : public CEngineControl {
public:
  CThrottleControlV1 (void);

  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CThrottleControlV1"; }
};
//=============================================================================
// CMixtureControlV1
//  NOT YET IMPLEMENTED
//=============================================================================
class CMixtureControlV1 : public CEngineControl {
public:
  CMixtureControlV1 (void);

  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CMixtureControlV1"; }
};
//=====================================================================
// CPropellerControlV1
//  NOT YET IMPLEMENTED
//=====================================================================
class CPropellerControlV1 : public CEngineControl {
public:
  CPropellerControlV1 (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPropellerControlV1"; }
};
//====================================================================
// CPrimeControlV1
//  NOT YET IMPLEMENTED
//=====================================================================
class CPrimeControlV1 : public CEngineControl {
public:
  CPrimeControlV1 (void);

  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CPrimeControlV1"; }
};
//=================================================================================
// CIdleControlV1
//  NOT YET IMPLEMENTED
//=================================================================================
class CIdleControlV1 : public CEngineControl {
public:
  CIdleControlV1 (void);

  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CIdleControlV1"; }
};
//======================= END OF FILE ==============================================================
#endif // ENGINES_H
