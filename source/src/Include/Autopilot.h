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

#include "../Include/Subsystems.h"
#include "../Include/Radio.h"
#include "../Include/Queues.h"
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
class VPilot;
//====================================================================================
//	Class to linearize changing values
//====================================================================================
class VLinear {
protected:
	//--- ATTRIBUTES -----------------------------------
	float		Time;														// Lag time
	double  cVal;														// Current value
	double  tVal;														// Target value
	//--- Method ---------------------------------------
public:
	VLinear() {Time = 1; tVal = 0; cVal = 0;}
	//---------------------------------------------------
	double	Get()										{return cVal;}
	double  Tvl()										{return tVal;}
	void		Upd(double dT);																	// Update value
	void		Set(double v,float t)		{tVal  = v, Time = t;}		// Set target value
	void    Inc(double v,double t)	{tVal += v; Time = t;}
	void    Dec(double v,double lim){tVal -= v; if (tVal < lim) tVal = v;}
};
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
#define PID_SPD 8									// SPEED PID
#define PID_MAX 9                 // Number of PID
//====================================================================================
//  AUTOPILOT LATERAL STATES
//====================================================================================
#define AP_DISENGD  0           // Disengaged
#define AP_LAT_ROL  1           //  Roll mode
#define AP_LAT_HDG  2           // Heading mode
#define AP_LAT_LT0  3						// LATERAL LEG 0
#define AP_LAT_LT1  4           // LATERAL LEG 1
#define AP_LAT_LT2  5           // LATERAL LEG 2
#define AP_LAT_LND  6						// Landing
#define AP_LAT_GND  7						// Ground steering
#define AP_LAT_TGA  8						// GO ARROUND
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
	U_CHAR		 stga;													// TGA step
  U_CHAR     vStat;                         // Vertical state
  U_CHAR     land;                          // landing option
  //--------Indicators and options -------------------------------------
	char			 xCtl;													// Externally controlled				
	char       Powr;                          // Power indicator
  char       uvsp;                          // use VSP
  char       aprm;                          // Approach mode
	char       wgrd;													// Wheel on ground
	char			 redz;													// Red zone in lateral mode
	char       sect;													// current VOR/ILS Sector 
	//--- Throttle control ----------------------------------------------
	U_INT      ugaz;                          // Use autothrottle
  //-----------Lights--------------------------------------------------
  char       alta;                          // Altitude armed
  char       flsh;                          // Flash
  //-----------Flasher ------------------------------------------------
  U_CHAR      timFS;                        // Flasher timer
  U_CHAR      mskFS;                        // Flasher mask
	//---Subsystems ------------------------------------------------------
	CSubsystem	 *altS;												// Altimeter
	CSubsystem   *cmpS;												// Compass
  //---Surface control -------------------------------------------------
	CFlapControl *flpS;												// Flaps systems
  CAeroControl *ailS;                       // aileron surface
  CAeroControl *elvS;                       // Elevator surface
  CAeroControl *rudS;                       // Rudder surface
  CAeroControl *elvT;                       // Elevator trim
	//--------------------------------------------------------------------
	CBrakeControl *brak;											// Brake control
	//--------------------------------------------------------------------
	CSpeedRegulator  *sreg;										// Speed Regulator
  //---Tracking control ------------------------------------------------
  double     Turn;                          // Turn adjust coef
	double     gain;													// gain factor
  //---Angle of attack -------------------------------------------------
  double     minA;                          // Minimuùm AOA
  double     maxA;                          // Maximum  AOA
  //---Limits -----------------------------------------------------------
  double     aLim;              // Altitude limit
  double     vLim;              // VSP limit
  double     aLND;              // Decision altitude for landing
  double     aMIS;              // Decision altitude for miss landing
	double     cMIS;							// Lateral missing coeff/ per feet AGL
  double     hMIS;              // Lateral miss error
  double     vMIS;              // Vertical miss error
	double	   dSPD;							// Disengage speed
	//--- GO ARROUND ------------------------------------------------------
	int					flp0;							// Flap position Leg1
	double			ang0;							// Angle away from runway
	double			dga0;							// Distance for LEG 1
	//---------------------------------------------------------------------
	double			ang1;							// Angle away from runway
	double			dga1;							// Distance for LEG 2
	//---------------------------------------------------------------------
	double			aTGA;							// TGA altitude above Ground
	//--- Flap control ----------------------------------------------------
	char			 tkoFP;							// Take off flap position
	double		 tkoFA;							// Altitude for retracting flaps
	char			 lndFP;							// Landing flap position
	double		 lndFA;							// Altitude for setting flaps
	//--- Flare parameters ------------------------------------------------
	double     sTAN;						  // Flare slope (radian)
	double     nTDP;							// Touch down point
	double     dTDP;							// Distance to touch down point
	//--- Ground target ---------------------------------------------------
	LND_DATA  *rend;							// Runway end
	//--- Autothrottle parameters -----------------------------------------
	double		 mRAT;							// Maximum rate
	double     cRAT;							// Current rate to maintain
	double     xRAT;							// Cruise rate
	double     sAPR;							// Approach speed
	double		 fSPD;							// Final approach speed
	double		 aCUT;							// Altitude to cut throttle
	double     vROT;							// Rotate speed
	double		 aTGT;							// Target altitude
  //--- LEG2 mode control values --------------------------------------
  double     glide;                         // Catching glide angle
	double     rDIS;													// Remaining distance
	//--- Lateral control values ----------------------------------------
	double		 vDTA;													// Distnce Turning Anticipation
  double     rHDG;                          // Target Heading
  double     aHDG;                          // Actual heading (yaw)
  double     xHDG;                          // cross heading
	double     xCOR;													// 45° correction
	double     xAPW;													// Cross angle of approach
	double     nHDG;													// Next heading
  double     hERR;                          // Lateral error
  double     vHRZ;                          // Horizontal speed
	//--- Landing parameters --------------------------------------------
	CFmt1Map   linTB;													// Alignment table
  //----Vertical mode control values ----------------------------------
  double     eVRT;                          // Vertical error (glide)
  double     vAMP;                          // Vertical amplifier
  double     rVSI;                          // Reference VSI
	double		 vFPS;													// Vertical feet/sec
	double     aVSI;													// VSI adjustment
	double     xALT;													// Expected altitude
	double     fmax;													// FPM max
	double     fmin;													// FPM min
	VLinear    rALT;													// Reference altitude	
  //---Current parameters -------------------------------------------
	double      cFAC;												  // Current factor
  double      cALT;                         // Current altitude
  double      cAGL;                         // Current AGL
	double      aSPD;													// Current speed KTS
	double			xAGL;													// Expected AGL
  //--------------------------------------------------------------------
  float      gTime;                         // ground timer
  CAirplane *plane;													// Plane to control
  //---Lateral leg parameters ------------------------------------------
  U_CHAR     mode;                          // Autopilot mode
  U_CHAR     signal;                        // NAV Signal (VOR/ILS)
  U_CHAR     trace;                         // TO/ FROM
  U_CHAR     sEVN;                          // STate Event
  //--------------------------------------------------------------------
  char       abrt;                          // Abort land
  //---Delta time ------------------------------------------------------
  float      dTime;                         // Dt
  BUS_RADIO *busRD;                          // Radio bus 
  //--------------------------------------------------------------------
  CPIDQ       pidQ;                         // List of components
  CPIDbox    *pidL[PID_MAX];                // Stack of PID
  //-------------------------------------------------------------------
  SMessage    mHDG;           // Permanent heading
  SMessage    mBUG;           // bug tracking message
  SMessage    mNAV;           // NAV gauge message
  SMessage    mALT;           // Altitude message
  SMessage    mVSI;           // VSI message
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
	void						DisplayGroundDeviation(double p1);
  //-------------------------------------------------------------------
  void            Probe(CFuiCanva *cnv);
  void            ReadPID(char *fn);
  void            AddPID(CPIDbox *pbx);
  EMessageResult  ReceiveMessage(SMessage *msg);
  void            GetAllSubsystems(std::vector<CPIDbox*> &pid);
  void            GetAllPID(CFuiPID *win);
  void            InitPID();
	void						DecodeTHRO(SStream *st);
	void						DecodeLMIS(SStream *st);
	void						DecodeTKOF(SStream *st);
	void						DecodeTOGA(SStream *st);
	void						DecodeAPRO(SStream *st);
	void						DecodeLAND(SStream *st);
	//-------------------------------------------------------------------
	void						InitLINE(SStream *st);
  int             BadSignal(char s);
  void            VSPerror();
  void            ALTalertSWP();
  void            ALTalertSET();
  bool            CheckAlert();
  //-------Options ----------------------------------------------------
	inline void	SetVDTA(double d)						{vDTA		= d;}
	inline void SetAVSI(double v)						{aVSI   = v;}
  inline void SetGLDopt(double g)         {glide	= g;}
  inline void SetDISopt(double a)         {aLND		= a;}
	void				SetLndFLP(char p,double a)	{lndFP = p; lndFA = a;}
	void				SetTkoFLP(char p,double a)  {tkoFP = p; tkoFA = a;}
  void        SetFLRopt(double a, double b,double d);
  void        SetAMIS(double a,double b);
	//--- External interface --------------------------------------------
	bool				Init();
	bool 				Engage();
	int				  EnterTakeOFF(char x,LND_DATA *rdt);
	bool				EnterGPSMode();
	void				ReleaseControl()		{xCtl = 0;}
	void				EnterWPT(double alt);
  //-------------------------------------------------------------------
  double          RoundValue(double v,double p);
  double          GetAOS();
  void            LateralMode();
  void            VerticalMode();
  double          CatchVSP();
	float						GetVSPCoef();
  //-------------------------------------------------------------------
  int             PowerLost();
  void            Disengage(char op);
	double					SelectSpeed();
	bool						MissLanding();
	void						LandingOption();
  bool            AbortLanding(char k);
	void						Rotate();
	void						Brake();
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
  void            GlideHold();
	void						SpeedHold();
  void            IncVSP();
  void            DecVSP();
  void            IncALT();
  void            DecALT();
	void						ChangeALT(double a);
	void						SetRVSI(double v);
	void						SetLandingMode();
  //----Lateral modes --------------------------------------------------
	double					AdjustHDG();
  void            GetCrossHeading();
	void						ModeLT0();
  void            ModeLT1();
  void            ModeLT2();
	void						ModeLND();
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
	void						SetGasControl(U_INT m);
  //-------------------------------------------------------------------
  inline char     armALT()  {return alta;}
  //-------------------------------------------------------------------
  inline char GetPOW()                {return Powr;}
  inline char NavMode() {return (signal == SIGNAL_VOR);}
  inline char IlsMode() {return (signal == SIGNAL_ILS);}
  inline char Flash()   {return flsh;}
	//--- Virtual pilot interface ---------------------------------------
	inline bool			BellowAGL(double a)	{return (cAGL < a);}
	inline bool     AboveAGL(double a)  {return (cAGL > a);}
	inline bool			IsDisengaged()		  {return (lStat == AP_DISENGD);}
  inline bool			IsEngaged()					{return (lStat != AP_DISENGD);}
	inline bool			ModeGround()				{return (vStat == AP_VRT_FIN);}
	inline bool			HasGasControl()			{return (ugaz != 0);}
	//-------------------------------------------------------------------
	inline double   GetSpeed()					{return aSPD;}
	//-------------------------------------------------------------------
	inline LND_DATA *GetRunwayData()		{return rend;}
  //-------------------------------------------------------------------
  inline    bool   engLite()					{return (AP_STATE_DIS != lStat);}

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
	inline    char GasST()	{return (ugaz)?(1):(0);}
};
//=========================================================================================
//  CLASS CPIDdecoder
//        Class to decode PID file
//=========================================================================================
class CPIDdecoder: public CStreamObject {
  //------Attributes ---------------------------------------------
  AutoPilot *apil;                          // Autopilot
	VPilot    *vpil;													// Virtual pilot
  //--------------------------------------------------------------
public:
  CPIDdecoder(char *fn,AutoPilot *ap);
 ~CPIDdecoder();
  int     Read(SStream *st,Tag tag);
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
