/*
 * Autopilot.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2007 Chris Wallace
 * Copyright 2005 Laurent Claudet
 * Copyright 2007 Jean Sabatier
 *
 *  Part of the PID controller is implemented using a formulea documented
 *  by Roy Ovesen in the FlightGear system, a project also under the GNU GPL agreement
 *  Thanks to Roy for his permission to use it. Jean Sabatier
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

#include "../Include/FlyLegacy.h"
#include "../Include/Subsystems.h"
#include "../Include/Radio.h"
//====================================================================================
#ifndef AUTOPILOT_H
#define AUTOPILOT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//====================================================================================
class CFuiProbe;
class CFuiCanva;
class CFuiPID;
class AutoPilot;
class CAeroControl;
class CAirplane;
class CSoundBUF;
//====================================================================================
// *
// * Autopilot proxy
// * This object intercept message destinated to AXIS and redirect them to
//    the active autopilot if any.
//====================================================================================
class CAutopilotProxy : public CDependent {
};
//====================================================================================
// *
// * Autopilot subsystems
// *
//====================================================================================
class CAutopilotMasterPanel : public CDependent {
public:
  CAutopilotMasterPanel (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CAutopilotMasterPanel"; }
  int     Read (SStream *stream, Tag tag);
  //------Attributes -------------------------------------------------
  SMessage  ap1;                                    // Autopilot 1
  SMessage  ap2;                                    // Autopilot 2
protected:

};
//=====================================================================================
//---PID BOX INDEX -------------------------------------
#define PID_ROL 0                 // ROLL PID
#define PID_HDG 1                 // HEAD PID
#define PID_BNK 2                 // BANK PID
#define PID_GLS 3                 // GLID SLOPE PID
#define PID_ALT 4                 // ALT HOLD PID
#define PID_VSP 5                 // VSP HOLD
#define PID_AOA 6                 // AOA PID
#define PID_RUD 7                 // Rudder PID
#define PID_GAS 8									// THROTTLE PID
#define PID_MAX 9                 // Number of PID
//====================================================================================
//  AUTOPILOT LATERAL STATES
//====================================================================================
#define AP_DISENGD  0           // Disengaged
#define AP_LAT_ROL  1           //  Roll mode
#define AP_LAT_HDG  2           // Heading mode
#define AP_LAT_LT1  3           // LATERAL LEG 1
#define AP_LAT_LT2  4           // LATERAL LEG 2
#define AP_LAT_GND  5						// Ground steering
#define AP_LAT_TGA  6						// GO ARROUND
//====================================================================================
//  AUTOPILOT VERTICAL STATES
//====================================================================================
#define AP_DISENGD  0           // Disengaged
#define AP_VRT_TKO	1						// Take off
#define AP_VRT_ALT  2           // Altitude hold
#define AP_VRT_VSP  3           // Vertical speed
#define AP_VRT_GSW  4           // Glide slope waiting
#define AP_VRT_GST  5           // Glide slope tracking
#define AP_VRT_FLR  6           // Flare segment
#define AP_VRT_FIN	7						// Final ground segment
//====================================================================================
//  GA ARROUND STEP
//====================================================================================
#define AP_TGA_UP5	0						// Leg 0 climb to 500
#define AP_TGA_HD0	0						// Leg 0:
#define AP_TGA_HD1  1						// Leg 1: Head 90°
#define AP_TGA_HD2  2						// LEG 2: going away
#define AP_TGA_HD3	3						// LEG 3: Going along
#define AP_TGA_HD4  4						// LEG 4: waiting approach
//====================================================================================
//  AUTOPILOT EVENTS
//====================================================================================
#define AP_EVN_POF  0           // Power off
#define AP_EVN_ENG  1           // Engage button
#define AP_EVN_HDG  2           // Heading button
#define AP_EVN_NAV  3           // NAV Button
#define AP_EVN_APR  4           // APR button
#define AP_EVN_REV  5           // REV button
#define AP_EVN_ALT  6           // ALT Button
#define AP_EVN_BUP  7           // UP Button
#define AP_EVN_BDN  8           // Down button
#define AP_EVN_SEL  9           // Altitude select
//-------------------------------------------------------------
#define AP_EVN_DIS 10           // disengage
#define AP_EVN_TMS 11           // Time slice
#define AP_EVN_SGL 12           // Signal lost
//-------------------------------------------------------------
#define AP_EVN_AUP 13           // Altitude UP
#define AP_EVN_ADN 14           // Altitude DW
//-------------------------------------------------------------
#define AP_EVN_AAA 15           // Arm altitude alert
#define AP_EVN_PON 99           // Power ON

//====================================================================================
//  Autotpilot STATE EVENT
//====================================================================================
#define AP_STATE_DIS  0         // Disengaged
#define AP_STATE_ROL  1         // ROLL MODE
#define AP_STATE_HDG  2         // HEADING MODE
#define AP_STATE_NAV  3         // NAV MODE ARMED
#define AP_STATE_NTK  4         // NAV TRACKING
#define AP_STATE_APR  5         // APR mode armed
#define AP_STATE_ATK  6         // ILS TRACKING
#define AP_STATE_VTK  7         // Vertical ILS tracking
#define AP_STATE_ALT  8         // ALTITUDE HOLD
#define AP_STATE_VOF  9         // VERTICAL MODE OFF
#define AP_STATE_VSP 10         // Enter VSP mode
#define AP_STATE_VSF 11         // VSP Clear
#define AP_STATE_VCH 12         // VSP change
#define AP_STATE_ACH 13         // ALT Change
#define AP_STATE_APF 14         // APR OFF
#define AP_STATE_HDF 15         // HDG OFF
#define AP_STATE_AAA 16         // Altitude alert
//====================================================================================
//  Land options
//====================================================================================
#define LAND_NONE   (0)         // No option
#define LAND_DISCT  (1)         // Disengage at decision altitude
#define LAND_FLARE  (2)         // Use flare leg
//====================================================================================
// PID controller for Autopilot
//  A PID controller monitors a value (aircraft heading for instance)against a
//  reference value and supplies an output value used to act on a control surface
//  Autopilot are built by cascading a set of PID
//  This particular PID controler is based on Flightgear PID controller
//  Thanks to Roy Ovesen for permission to use its formulea
//  It use the following inputs ---------------------------------------------
//    Yn  is the sampled value to control at step n
//    Rn  is the target  value to reach at step n
//    Kp  is the proportional gain coefficient
//    Kb  is the proportional reference weighing factor
//    Kc  is the derivative reference weighing factor
//    Ti  integrator timing (sec)
//    Td  Derivator timing (sec)
//====================================================================================
class CPIDbox: public CDependent,public CqItem  {
  //---ATTRIBUTS -----------------------------------------
  U_SHORT  nPid;                      // Pid Index
  U_CHAR   drvt;                      // Use derivative term
  U_CHAR   intg;                      // Use integrator term
  U_CHAR   anti;                      // use anti saturation
  double   rate;                      // Output level
  AutoPilot *apil;              // Autopilot
  //-------------------------------------------------------
  static Tag PidIDENT[];
  //---Controller input values ------------------------------------
  double  Ts;                         // Time between sample
  double  Yn;                         // Sample of value
  double  Rn;                         // Target value
  double  Kp;                         // Proportional gain
  double  Ti;                         // Integrator Timing
  double  Td;                         // Derivator Timing
  double  Ta;                         // Antisaturator timer
  double  Ki;                         // Integrator term
  double  Ks;                         // Saturator term
  double  es;                         // Saturator error
  double  Kd;                         // Derivative term
  //---Saturation values ---------------------------------
  double  vmin;                       // Minimum output
  double  vmax;                       // Maximum output
  //------------------------------------------------------
  double  en;         // error step n
  double  en1;        // error step n-1
  double  en2;        // error step n-2
  double  eSum;       // error sum
  double  yPrv;       // Y at previous step
  double  iMax;       // Integrator limit
  double  vSat;       // Saturation value
  //------------------------------------------------------
  double  Un;         // Controller output 
  double  Vn;         // Clamped output
  //---Methodds ------------------------------------------
public:
  CPIDbox(U_CHAR No,AutoPilot *ap);
 ~CPIDbox();
  //------------------------------------------------------
  void      Probe(CFuiCanva *cnv);
  //------------------------------------------------------
  int       Read(SStream *st,Tag tag);
  void      ReadFinished();
  void      SetMsgInp(SMessage *msg,Tag idn, Tag prm);
  double    Integrate(double kd);
  void      SetDTIME(double t);
  void      SetITIME(double t);
  void      SetATIME(double t);
  //-------------------------------------------------------
  void      Init();
	void      SetCoef(double kp,double ti,double td);
  void      TrakSaturation(double val,double minv, double maxv);
  double    Update(float Dt,double Y,double R);
  double    GetValue(Tag pm);
  double    Clamp(double v);
  void      SetValue(Tag pm,double val);
  void      SetClamp(CAeroControl *c);
  //-------------------------------------------------------
  inline int  GetPidNo()          {return nPid;}
  inline void Rate(double r)      {rate = r;}
	//-------------------------------------------------------
	inline float	GetVN()						{return Vn;}
  //-----Set values ---------------------------------------
  inline void SetSample(double s) {Yn = s;}
  inline void SetTarget(double t) {Rn = t;}
	inline void ClearKI()						{Ki = 0;}
  //-------------------------------------------------------
  inline void SetMaxi(double m)   {if (m < vmax) vmax = m;}
  inline void SetMini(double m)   {if (m > vmin) vmin = m;}
};
//=========================================================================================
//  CLASS CPIDQ
//        Class to collect list of PIDbox
//=========================================================================================
class CPIDQ : public CQueue {
  //---------------------------------------------------------------------
public:
  inline void Lock()                {pthread_mutex_lock (&mux);}
  inline void UnLock()              {pthread_mutex_unlock (&mux);}
  //----------------------------------------------------------------------
  //~CPIDQ();                        // Destructor
  //----------------------------------------------------------------------
  inline void PutEnd(CPIDbox *box)           {CQueue::PutEnd(box);}
  inline CPIDbox *Pop()                      {return (CPIDbox*)CQueue::Pop();}
  inline CPIDbox *GetFirst()                 {return (CPIDbox*)CQueue::GetFirst();}
  inline CPIDbox *GetNext(CPIDbox *box)      {return (CPIDbox*)CQueue::GetNext(box);}
};
//====================================================================================
//    GENERIC AUTOPILOT MODE
//====================================================================================
//====================================================================================
//  Activation flag
//====================================================================================
//====================================================================================
//    GENERIC AUTOPILOT PANEL
//  In computing the intercept point
//    A is the angle of aircraft heading
//    B is the angle of the OBS (ILS)
//    both angles are relative to geographical north
//  The rate turn coefficient correspond to a 2 minutes turn at 3° per sec
//    rK = 360 / (3 * PI).  If the rate turn is changed then this value must be changed
//    to rK = 360 / (k * PI) where k is the number of degre per second  
//====================================================================================
class AutoPilot : public CDependent {
  //---ATTRIBUTS ------------------------------------------------------
protected:
  U_CHAR     inUse;                         // Use autopilot
  U_CHAR     lStat;                         // Lateral state
	U_CHAR		 step;													// TGA step
  U_CHAR     vStat;                         // Vertical state
  U_CHAR     land;                          // landing option
  //-------------------------------------------------------------------
  char       Powr;                          // Power indicator
  char       uvsp;                          // use VSP
  char       aprm;                          // Approach mode
  char       ugaz;                          // Use autothrottle
	char       wgrd;													// Wheel on ground
	char			 redz;													// Red zone
	char       sect;													// Sector TO
  //-----------Lights--------------------------------------------------
  char       alta;                          // Altitude armed
  char       flsh;                          // Flash
  //-----------Flasher ------------------------------------------------
  U_CHAR      timFS;                            // Flasher timer
  U_CHAR      mskFS;                            // Flasher mask
	//---Subsystems ------------------------------------------------------
	CSubsystem	 *altS;												// Altimeter
	CSubsystem   *cmpS;												// Compass
  //---Surface control -------------------------------------------------
	CFlapControl *flpS;												// Flaps systems
  CAeroControl *ailS;                       // aileron surface
  CAeroControl *elvS;                       // Elevator surface
  CAeroControl *rudS;                       // Rudder surface
  CAeroControl *elvT;                       // Elevator trim
	//---------------------------------------------------------
	CThrottleControl *gazS;										// gas controller
  //---Tracking control ------------------------------------------------
  double     Turn;                          // Turn adjust coef
	double     gain;													// gain factor
  //---Angle of attack -------------------------------------------------
  double     wAOI;                          // Wing Angle of incidence (°)
  double     minA;                          // Minimuùm AOA
  double     maxA;                          // Maximum  AOA
  //---Limits -----------------------------------------------------------
  double     aLim;              // Altitude limit
  double     vLim;              // VSP limit
  double     aLND;              // Decision altitude for landing
  double     aMIS;              // Decision altitude for miss landing
	double     cMIS;							// Lateral missing coeff/ feet AGL
  double     hMIS;              // Lateral miss error
  double     vMIS;              // Vertical miss error
	double     rAGL;							// AGL reference
	double	   dSPD;							// Disengage speed
	//--- GO ARROUND ------------------------------------------------------
	double			TGA0;							// Distance for LEG 1
	double			TGA1;							// Distance for LEG 2
	//--- Flap control ----------------------------------------------------
	char			 tkoFP;							// Take off flap position
	double		 tkoFA;							// Altitude
	char			 lndFP;							// Landing flap position
	double		 lndFA;							// Distance for landing flap
	//--- Flare parameters ------------------------------------------------
	double     sTAN;						  // Flare slope (radian)
	double     nTDP;							// Touch down point
	double     dTDP;							// Distance to touch down point
	//--- Ground target ---------------------------------------------------
	SPosition	*gPOS;							// Ground position
	//--- Autothrottle parameters -----------------------------------------
	double     cRAT;							// Current rate to maintain
	double     xRAT;							// Cruise rate
	double		 fRAT;							// Final approach rate
	double		 aCUT;							// Altitude to cut throttle
	double     vROT;							// Rotate speed
	double		 aTGT;							// Target altitude
  //--- LEG2 mode control values --------------------------------------
  double     tCoef;                         // Turn coefficient
  double     dREF;                          // Distance to Reference
  double     sin3;                          // Sine(3°)
  double     glide;                         // Catching glide angle
	double     rDIS;													// Remaining distance
	//--- Lateral control values ----------------------------------------
  double     rHDG;                          // Target Heading
  double     aHDG;                          // Actual heading (yaw)
  double     xHDG;                          // cross heading
	double     xCOR;													// 45° correction
	double     nHDG;													// Next heading
  double     hERR;                          // Lateral error
  double     vTIM0;                         // Time for ARC AB
  double     vTIM1;                         // Time for P to D
  double     vHRZ;                          // Horizontal speed
  //----Vertical mode control values ----------------------------------
  double     Vref;                          // VSP Reference
  double     eVRT;                          // Vertical error (glide)
  double     vAMP;                          // Vertical amplifier
  double     rALT;                          // Reference altitude
  double     eVSP;                          // VSP error 
  double     rVSI;                          // Reference VSI
  //---Current parameters -------------------------------------------
	double      cFAC;												  // Current factor
  double      cALT;                         // Current altitude
  double      cAGL;                         // Current AGL
  double      afps;                         // Aircraft feet per second
	double      kSPD;													// Current speed KTS
  //-------------------------------------------------------------------
  double     vTime;                         // VSP timer
  //--------------------------------------------------------------------
  float      gTime;                         // ground timer
  CAirplane *plane;                   // Plane to control
  //---Lateral leg parameters ------------------------------------------
  U_CHAR     mode;                          // Autopilot mode
  U_CHAR     signal;                        // NAV Signal (VOR/ILS)
  U_CHAR     rfu1;                          // TO/ FROM
  U_CHAR     sEVN;                          // STate Event
  //--------------------------------------------------------------------
  char       abrt;                          // Abort land
  //---Delta time ------------------------------------------------------
  float      dTime;                         // Dt
  BUS_RADIO *Radio;                         // Radio Data 
  //--------------------------------------------------------------------
  CPIDQ       pidQ;                         // List of components
  CPIDbox    *pidL[PID_MAX];                // Stack of PID
  //-------------------------------------------------------------------
  SMessage    mTRN;           // Turn Coordinator
  SMessage    mHDG;           // Permanent heading
  SMessage    mBUG;           // bug tracking message
  SMessage    mNAV;           // NAV gauge message
  SMessage    mALT;           // Altitude message
  SMessage    mVSI;           // VSI message
	SMessage    mGAZ;						// Throttle message
	SMessage    mSPD;						// SPEED message
  //---METHODS---------------------------------------------------------
public:
  AutoPilot (void);
 ~AutoPilot();
  // CStreamObject methods --------------------------------------------
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  void  PrepareMsg(CVehicleObject *veh);
  bool  MsgForMe (SMessage *msg);
  virtual void  StateChanged(U_CHAR evs) {}
  virtual void  Alarm() {}
  //--- CSubsystem methods --------------------------------------------
  virtual const char* GetClassName (void) { return "AutoPilot"; }
	//-------------------------------------------------------------------
  void            TimeSlice(float dT,U_INT FrNo);
	//-------------------------------------------------------------------
	void		DisplayGroundDeviation(double p1);
	void		SetGroundMode(SPosition *p);
  //-------------------------------------------------------------------
  void            Probe(CFuiCanva *cnv);
  void            ReadPID(char *fn);
  void            AddPID(CPIDbox *pbx);
  EMessageResult  ReceiveMessage(SMessage *msg);
  void            GetAllSubsystems(std::vector<CPIDbox*> &pid);
  void            GetAllPID(CFuiPID *win);
  void            InitPID();
  int             BadSignal(char s);
  void            VSPerror();
  void            ALTalertSWP();
  void            ALTalertSET();
  bool            CheckAlert();
  //-------Options ----------------------------------------------------
	inline void SetACUT(double v)						{aCUT		= v;}
  inline void SetVREF(double v)           {Vref		= v;}
  inline void SetTrak(double t,double a)  {Turn		= t; gain = a;}
  inline void SetGLDopt(double g)         {glide	= g;}
  inline void SetDISopt(double a)         {aLND		= a;}
	void				SetLndFLP(char p,double a)	{lndFP = p; lndFA = a;}
	void				SetTkoFLP(char p,double a)  {tkoFP = p; tkoFA = a;}
	void				SetTKOopt(double s, double a);
  void        SetFLRopt(double a, double b,double d);
  void        SetMISopt(double a);
	inline void SetGroundPos(SPosition *p)	{gPOS		= p;}
	//--- External interface --------------------------------------------
	bool				Init();
	bool 				Engage();
	bool				EnterTakeOFF();
	void				SetNavMode();
  //-------------------------------------------------------------------
  double          RoundValue(double v,double p);
  double          GetAOS();
  void            LateralMode();
  void            VerticalMode();
  void            CatchVSP();
  //-------------------------------------------------------------------
  int             PowerLost();
  void            Disengage(char op);
	double					SelectSpeed();
	bool						MissLanding();
	void						LandingOption();
  bool            AbortLanding(char k);
	void						Rotate();
  //-------TRANSITION ROUTINE -----------------------------------------
  void            EnterINI();
  void            EnterROL();
  void            EnterHDG();
  void            EnterNAV();
  void            EnterAPR();
  void            EnterALT();
  void            EnterVSP();
  void            EnterFLR();
	void						EnterFIN();
  void            ExitLT2();
  void            ExitHDG();
  void            OnlyHDG();
  void            ExitNAV();
  void            ExitAPR();
  //-------------------------------------------------------------------
  void            SwapALT();
  //-------ACTION ROUTINES --------------------------------------------
  void            AltitudeHold();
  void            LateralHold();
  void            RudderHold();
  void            GlideHold();
	void						SpeedHold();
  void            IncVSP();
  void            DecVSP();
  void            IncALT();
  void            DecALT();
	void						ChangeALT(double a);
	void						HoldAOA(double v);
	void						SetLandingMode();
  //----Lateral modes --------------------------------------------------
	double					AdjustHDG();
  void            GetCrossHeading();
	void						ModeLT0();
  void            ModeLT1();
  void            ModeLT2();
	void						ModeTGA();
  void            ModeROL();
  void            ModeHDG();
	void						ModeGND();
  void            CrossDirection();
	//---- Vertical modes ------------------------------------------------
  void            ModeGSW();
  void            ModeGST();
  void            ModeALT();
  void            ModeVSP();
  void            ModeFLR();
	void						ModeFIN();
  void            ModeDIS();
  //-------EVENT PROCESSING -------------------------------------------
  void            StateDIS(int evn);
  void            StateROL(int evn);
  void            StateHDG(int evn);
  void            StateLAT(int evn);
	void						StateGND(int evn);
  void            NewEvent(int evn);
	void						SwapGasControl();
	void						SetGasControl(char s);
  //-------------------------------------------------------------------
  inline char     armALT()  {return alta;}
  //-------------------------------------------------------------------
  inline char GetPOW()                {return Powr;}
  inline char NavMode() {return (signal == SIGNAL_VOR);}
  inline char IlsMode() {return (signal == SIGNAL_ILS);}
  inline char Flash()   {return flsh;}
	//--- Virtual pilot interface ---------------------------------------
	inline bool BellowAGL(double a)	{return (cAGL < a);}
	inline bool IsDisengaged()		  {return (lStat == AP_DISENGD);}
  inline bool IsEngaged()					{return (lStat != AP_DISENGD);}
	inline bool ModeGround()				{return (vStat == AP_VRT_FIN);}
	inline bool HasGasControl()			{return (ugaz != 0);}
  //-------------------------------------------------------------------
  inline    bool        engLite()     {return (AP_STATE_DIS != lStat);}
};
//========================================================================================
//  DEFINE the K140 DISPLAY FIELDs
//========================================================================================
#define K140_FD_HD1 (1) // Heading mode field 1
#define K140_FD_HD2 (2) // Heading mode field 2
#define K140_FD_VT1 (3) // Vertical mode field 1
#define K140_FD_VT2 (4) // Vertical mode field 2
#define K140_FD_ALK (5) // ALtitude Armed
#define K140_FD_VSI (6) // VSI or ALT display
#define K140_FD_ALT (7) // Altitude
#define K140_FD_NBR (8)
//====================================================================================
// CKAP140Panel
//====================================================================================
class CKAP140Panel : public AutoPilot {
  friend class CBKAP140Gauge;
  //---ATTRIBUTES ---------------------------------------------------
  char  flsh;                       // flasher
  char  msDIR;                      // Mouse direction
  RADIO_FLD   fldTAB[K140_FD_NBR];  // Radio fields
  //--------Field to edit -------------------------------------------
  char      vsp[8];
  char      alt[16];
  //----Methods -----------------------------------------------------
public:
  CKAP140Panel (void);
  CSoundBUF *sbf;                   // Disconnect sound
  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CKAP140Panel"; }
  void      Alarm();
  //-----------------------------------------------------------------
  void      InitField(short No,char * data, short cf);
  void      Lite(char No,char *txt);
  void      ClearField(int No);
  void      CopyField(char No,RADIO_FLD *fld);
  void      ClearAll();
  void      PopField(int No);
  void      EditVSP();
  void      EditALT();
  int       Dispatch(int evn);
  void      TimeSlice(float dT,U_INT FrNo);
  //-----------------------------------------------------------------
  void      StateChanged(U_CHAR evn);
  int       PowerON();
  int       PowerOF();
  //----Return field address ----------------------------------------
  inline    RADIO_FLD  *GetField(U_CHAR No)   {return fldTAB + No;}
  inline    char LatARM() {return (fldTAB[K140_FD_HD2].data == 0)?(0):(1);}
  inline    char VrtARM() {return (fldTAB[K140_FD_VT2].data == 0)?(0):(1);}
  inline    char Blink()  {return flsh;}
	inline    char GasST()	{return ugaz;}
};
//=========================================================================================
//  CLASS CPIDdecoder
//        Class to decode PID file
//=========================================================================================
class CPIDdecoder: public CStreamObject {
  //------Attributes ---------------------------------------------
  AutoPilot *apil;                          // Autopilot
  //--------------------------------------------------------------
public:
  CPIDdecoder(const char *fn,AutoPilot *ap);
 ~CPIDdecoder();
  int     Read(SStream *st,Tag tag);
  void    DecodeLanding(SStream *st);
	void		DecodeTRAK(char *txt);
	void		DecodeTHRO(char *txt);
	void		DecodeVROT(char *txt);
	void		DecodeFlap(char *txt);
};

//====================================================================================
// CKAP150Panel
//====================================================================================
class CKAP150Panel : public AutoPilot {
public:
  CKAP150Panel (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CKAP150Panel"; }
};
//====================================================================================
// CKAP200Panel
//====================================================================================

class CKAP200Panel : public AutoPilot {
public:
  CKAP200Panel (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CKAP200Panel"; }
};
//====================================================================================
// CAFCS65Panel
//====================================================================================

class CAFCS65Panel : public AutoPilot {
public:
  CAFCS65Panel (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CAFCS65Panel"; }
};
//====================================================================================
// CAFCS85Panel
//====================================================================================

class CAFCS85Panel : public AutoPilot {
public:
  CAFCS85Panel (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CAFCS85Panel"; }
  //--------------------------------------------------------------
  int Read(SStream *st,Tag tag);
};
//====================END OF FILE ================================
#endif // AUTOPILOT_H
