/*
 * Subsystems.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2007 Chris Wallace
 * Copyright 2005 Laurent Claudet
 * Copyright 2007 Jean Sabatier
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

/*! \file Subsystems.h
 *  \brief Defines CSubsystem and derived aircraft system classes
 */


#ifndef SUBSYSTEMS_H
#define SUBSYSTEMS_H
#include "../Include/Engines.h"
#include "../Include/BaseSubsystem.h"
#include "../Include/Utility.h"
#include "../Include/database.h"
#include "../Include/WeightManager.h"	 
#include <vector>
#include <map>
//==================================================================================
class CSteeringControl;
//==============================================================================
// CAnnouncement
//==============================================================================
class CAnnouncement : public CSubsystem {
public:
  CAnnouncement (void);

  // CStreamObject methods
//  virtual int   Read (SStream *stream, Tag tag);
//  virtual void  ReadFinished (void);
//  virtual void  Write (SStream *stream);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CAnnouncement"; }
//  virtual void  ReceiveMessage (SMessage *msg);
//  virtual void  Update (float dT);
};

//====================================================================================
// CGenericMonitor
//====================================================================================
class CGenericMonitor : public CDependent {
public:
  CGenericMonitor (void);
  virtual ~CGenericMonitor (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
	const char* GetClassName (void) { return "CGenericMonitor"; }
	EMessageResult  ReceiveMessage (SMessage *msg);
	void	TimeSlice(float dT,U_INT FrNo);				// JSDEV*	
  void  Probe(CFuiCanva *cnv);
	void	PrepareMsg(CVehicleObject *veh);					    // JSDEV* Prepare Messages
	float	ExtractValue(SMessage *msg);						      // JSDEV*
  float CombineValues();
  char *ExtractName(SMessage *msg);
	//--- Published values on Gauge BUS ---------------------------------------
	int GaugeBusIN01()									{return compR;}	// Result of compare
	int GaugeBusIN03()									{return comp;}	// Compare reference
  //-----Attributes ---------------------------------------------------------
protected:
	EMonitorMode  mode;				          //< How to compare value to reference value
	float         comp;				          //< Reference value for comparison
	float					HiVal;				        // Hight value
	float					LoVal;				        // Low value
	SMessage				*msg1;				      // message for Val2
	SMessage				*msg2;				      // message for Val2
	SMessage				*msg3;				      // message for Val3
  float         refv;                 // Reference value
  float         val1;                 // Value 1
  float         val2;                 // Value 2
  float         val3;                 // Value 3
  char          oper;                 // Operator
	bool					compR;				        // Compare result

};
//====================================================================================
//	JSDEV* Class Polynome to store polynomial coefficients for convertion of value
//	Note: coefficient are ordered in increasing order of power
//	That is if X is the input value and coef = [A0,A1,A2,..,An]
/// then output value is A0 + A1*X + A2*X² + ... + AX^n
//====================================================================================
class CPolynome: public CStreamObject {
public:
	CPolynome(void);								//	Constructor
	virtual const char* GetClassName (void) { return "CPolynome"; }
	virtual ~CPolynome(void){};						//	Destructor
	virtual int	Read (SStream *stream, Tag tag);	//	Read coefficients
	float	Compute(float vx);						//	Compute output value
private:
	int		indx;									//	Number of coefficients
	float	coef[10];								//	List of up to 10 coefficients
};

//====================================================================================
//	CGeneric Indicator
//====================================================================================
class CGenericIndicator : public CDependent {
public:
  CGenericIndicator (void);
  virtual ~CGenericIndicator (void);

  //--- CStreamObject methods -----------------------------------------
  int       Read (SStream *stream, Tag tag);
  void      ReadFinished (void);
  void      Probe(CFuiCanva *cnv);
  //--- CSubsystem methods --------------------------------------------
  const char*		GetClassName (void) { return "CGenericIndicator"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  void			PrepareMsg(CVehicleObject *veh);					// JSDEV* Prepare Messages
  void			TimeSlice(float dT, U_INT FrNo);					// JSDEV*
	//--- Published values on gauge BUS ---------------------------------
	float			GaugeBusFT01()				{return indn;}					// computed value
	//--- ATTRIBUTES ----------------------------------------------------
protected:
  std::vector<SMessage*>	mVal;		///< Messages to get value for comparison
  Tag									alia;		///< Alias datatag
  ECombineMode				mode;		///< How to combine message values
  CPolynome				   *poly;		/// Polynome conversion
private:

};
//====================================================================================
//
// CTimeDelay
//
// This monitor subsystem can be configured to (do something) at periodic
//   time intervals
//====================================================================================
class CTimeDelay : public CDependent {
public:
  CTimeDelay (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTimeDelay"; }
};

//====================================================================================
// CKeyedSystem
//====================================================================================
class CKeyedSystem : public CSubsystem {
public:
  CKeyedSystem (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CKeyedSystem"; }
};
//======================================================================================
//  Subsystems are defiend from here
//  Terminal node means that no derived clss exists
//  When adding derived class, requalify as virtual all needed functions by the derived
//  class
//======================================================================================
//-----------------------------------------------------------------------
// JSDEV*  CContactor
//  Terminal node
//	Prepare Dependent messages
//-----------------------------------------------------------------------
class CContactor : public CDependent {

public:
  CContactor (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
	const char* GetClassName (void) { return "CContactor"; }
	// CContactor  methods
	void		TimeSlice(float dT,U_INT FrNo);
	void		PrepareMsg(CVehicleObject *veh);
  void    Probe(CFuiCanva *c);
  void    Reset();
protected:
	bool		NewTransition(SMessage *msg, int *actual);
	int			GetEvent(void);
	void		ClosedState(int event);
	void		OpeningState(float dT);
	void		OpenState(int event);
	void		ClosingState(float dT);
protected:
	//-----------Contactor internal state ------------------
	enum CState {	
          NONE	  = 0,
					CLOSED	= 1,
					OPENING = 2,
					OPEN	  = 3,
					CLOSING = 4, };
	//------------------------------------------------------
	enum CEvent {	RESET	= 1,
					      CLOSE	= 2,
					      EOPEN	= 3, };
	//------------------------------------------------------
	CState		Etat;				// Internal state
	int			Event;				// New event
	float		timer;				// transition timer
	float		dlyC;				  // Delay to close
	float		dlyO;				  // Delay to open
	int			Rstate;				// Reset last state
	SMessage    mReset;		// Contactor close message
	int			Cstate;				// Close last state
	SMessage    mClose;		// Contactor close message
	int			Ostate;				// Open  last state
	SMessage    mOpen;		// Contactor open message
};

//====================================================================================
// JSDEV* CFlasher
//  Terminal node
//====================================================================================
class CFlasher : public CDependent {
public:
  CFlasher (void);
  
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  const char*		  GetClassName (void) { return "CFlasher"; }
  void				    TimeSlice(float dT,U_INT FrNo);
  EMessageResult	ReceiveMessage(SMessage *msg);
  void    Probe(CFuiCanva *cnv);

protected:
  float     time;			  // Flasher cycle time
  float     delta;			// Current delta
  bool		  blnk;			  // Blink state
};


//======================================================================
// * Electrical Subsystems
//======================================================================

//======================================================================
// CBattery
//  Terminal node
//======================================================================
class CBattery : public CDependent {
public:
  CBattery (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  EMessageResult	ReceiveMessage (SMessage *msg);
  // CSubsystem methods
  const char* GetClassName (void) { return "CBattery"; }

protected:
  float   life;       ///< Capacity (amp-hours)
  float   lowLevel;   ///< Low battery level (volts)
};


//======================================================================
// JSDEV*	CAnnunciatorLight
//  Terminal node
//======================================================================
class CAnnunciatorLight : public CDependent {
public:
  CAnnunciatorLight (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  const char* GetClassName (void) { return "CAnnunciatorLight"; }
  EMessageResult	ReceiveMessage (SMessage *msg);
  // anlt method
  void		TimeSlice(float dT,U_INT FrNo);	// Update
protected:
	enum CState {OFF = 0, ON = 1,SBYE = 2};
	float		blnk;								// Blink period (sec)
	float		blpp;								// Blink rate (on duty cycle in sec)
	float		T1;									// Timer period
	float		T2;									// Timer blink
	int			bState;								// Blinking state
	int			Etat;								// Internal state
};
//===========================================================================
// CExteriorLight
//  Terminal node
//===========================================================================
class CExteriorLight : public CDependent {
public:
  CExteriorLight (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  // CSubsystem methods
  const char* GetClassName (void) { return "CExteriorLight"; }
  void        TimeSlice (float dT,U_INT FrNo = 0);			// JSDEV*
};

//===========================================================================
// CVoltmeter
//  Terminal node
//===========================================================================
class CVoltmeter : public CDependent {
public:
  CVoltmeter (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  // CSubsystem methods
  const     char* GetClassName (void) { return "CVoltmeter"; }
  void            TimeSlice(float dT,U_INT FrNo);
  EMessageResult  CVoltmeter::ReceiveMessage (SMessage *msg);
  void            Probe(CFuiCanva *cnv);
  //------- Attributes ------------------------
protected:
  float     lowV;               // Low voltage limit
  char      annV;               // Annunciator                
  SMessage  msg;                // Monitor message
};

//===========================================================================
// CAmmeter
//  Terminal node
//===========================================================================
class CAmmeter : public CDependent {
public:
  CAmmeter (void);

  // CSubsystem methods
  int   Read (SStream *stream, Tag tag);
  void  TimeSlice(float dT,U_INT FrNo);
  void  Probe(CFuiCanva *cnv);
  const char* GetClassName (void) { return "CAmmeter"; }
  //-------------------------------------------------------------------
  EMessageResult CAmmeter::ReceiveMessage (SMessage *msg);
	//--- published values on gauge bus ---------------------------------
	float GaugeBusFT01()				{return indn;}	// AMP/H
  //----ATTRIBUTES ----------------------------------------------------
protected:
  bool        chrg;         // Charge meter
  SMessage    msg;          // Monitor message
  bool        loadMeter;    // Load meter
};
//===========================================================================
// CGenerator
//  Terminal node
//===========================================================================
class CGenerator : public CDependent {
public:
  CGenerator (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CSubsystem methods
  const char*     GetClassName (void) { return "CGenerator"; }
  void            TimeSlice(float dT,U_INT FrNo);
  EMessageResult  ReceiveMessage (SMessage *msg);
  //----ATTRIBUTES ----------------------------------------------------
protected:
  float     mxld;           // Maximum load (amps)
  float     hiRG;           // Higher RPM
  float     loRG;           // Lower RPM
  //---Engine message ------------------------------------------------
  SMessage  emsg;
};

//===========================================================================
// CFrequencyMeter
//  Terminal node
//===========================================================================
class CFrequencyMeter : public CDependent {
public:
  CFrequencyMeter (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  const char* GetClassName (void) { return "CFrequencyMeter"; }
  //----ATTRIBUTES ----------------------------------------------------
protected:
  SMessage    mMon;         ///< Monitor message
};

//===========================================================================
// CInverter
//  Terminal node
//===========================================================================
class CInverter : public CDependent {
public:
  CInverter (void);

  // CSubsystem methods
  const char* GetClassName (void) { return "CInverter"; }
};
//===========================================================================
// CPitotStaticSwitch
//===========================================================================
class CPitotStaticSwitch : public CDependent {
public:
  CPitotStaticSwitch (void);
  virtual ~CPitotStaticSwitch (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPitotStaticSwitch"; }
  //----ATTRIBUTES ----------------------------------------------------
protected:
  int   nSystems;       // Number of pitot/static systems
  int   *system;        // Array of pitot/static system group numbers
};

//===========================================================================
// CPitotHeatSwitch
//  Terminal node
//===========================================================================
class CPitotHeatSwitch : public CPitotStaticSwitch {
public:
  CPitotHeatSwitch (void);

  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CPitotHeatSwitch"; }
};

//===========================================================================
// CStaticSourceSwitch
//  Terminal node
//===========================================================================
class CStaticSourceSwitch : public CPitotStaticSwitch {
public:
  CStaticSourceSwitch (void);

  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CStaticSourceSwitch"; }
};
//=============================================================================
// CLightSettingState
//  Terminal node
//=============================================================================
class CLightSettingState : public CDependent {
public:
  CLightSettingState (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  const char* GetClassName (void) { return "CLightSettingState"; }
  EMessageResult      ReceiveMessage (SMessage *msg);
  void    Probe(CFuiCanva *cnv);
  //---ATTRIBUTES ----------------------------------------
protected:
  float   levl;       // Light level
};
//=============================================================================
// CSwitchSet
//  Terminal node
//=============================================================================
class CSwitchSet : public CDependent {
public:
  CSwitchSet (void);
  virtual ~CSwitchSet (void);

  // CStreamObject methods
  int             Read (SStream *stream, Tag tag);
  void            ReadFinished();
  //---Switch state -----------------------------------------
  void            SynchroAll();
  void            ChangeState();
  void            ChangePosition(int inx);
  void            Probe(CFuiCanva *cnv);
  EMessageResult  ReceiveMessage (SMessage *msg);
  // CSubsystem methods
  const char* GetClassName (void) { return "CSwitchSet"; }
	//--- Gauge BUS --------------------------------------------
	inline int			GaugeBusIN03()  {return Indx;}
  //--- Attributes -------------------------------------------
protected:
  int     Indx;                      // Current position
  std::vector<SMessage >    smsg;   // Switch messages
  std::vector<SMessage *>   msgs;   // List of messages
  bool    sync;         ///< Sync all switches
  int     zBase;        ///< States start at zero (instead of one)
  int     onState;      ///< Switch state in On position
  int     offState;     ///< Switch state in Off position
};

// *
// * Gauge subsystems
// *

//=============================================================================
// CPitotStaticSubsystem
//
// This subystem is an abstract base class for any subsystems which have a 
//   dependency on a pitot/static subsystem.
//=============================================================================
class CPitotStaticSubsystem : public CDependent {
public:
  CPitotStaticSubsystem (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
//  virtual void  ReadFinished (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPitotStaticSubsystem"; }
//  virtual EMessageResult  ReceiveMessage (SMessage *msg);

protected:
  int       gNum;   // Port group number
};

//=============================================================================
// JSDEV* revisited CAltimeter
//  Terminal node
//=============================================================================
class CAltimeter : public CPitotStaticSubsystem {
public:

	//-----------------------------------------------------
  CAltimeter (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  const char* GetClassName (void) { return "CAltimeter"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  void TimeSlice (float dT,U_INT FrNo = 0);					
  //--------------------------------------------------------------
	U_INT   ComputeCorrection(float kpr);
  U_INT   UpdatePressure(float inc);
  void    Probe(CFuiCanva *cnv);
  //---------Attributes ------------------------------------------
protected:
  float   kollValue;            // In inches of Hg
  float	  kollVariation;		    // Correction in feet
  float   decisionHeight;       // In feet AGL
  float   radarAlt;             // Radar altitude
  float   hPres;                // presure in inHg
	//--- Published values ---------------------------------------
	float	  GaugeBusFT01()					{return indn;}		// altitude
	float		GaugeBusFT02()					{return radarAlt;}
  //-----Kolman digits -----------------------------------------
  U_CHAR d1;
  U_CHAR d2;
  U_CHAR d3;
  U_CHAR d4;
};
//=============================================================================
// CVerticalSpeedIndicator
//  Terminal node
//=============================================================================
class CVerticalSpeedIndicator : public CPitotStaticSubsystem {
  //--- ATTRIBUTES -----------------------------------------
protected:
  double old;
  double u100;
  double vfs;                     /// Vertical feet /sec
public:
  CVerticalSpeedIndicator (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CSubsystem methods
  const char*     GetClassName (void) { return "CVerticalSpeedIndicator"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  void            TimeSlice (float dT,U_INT FrNo = 0);		// JSDEV*

};
//=============================================================================
// CAirspeedIndicator
//  Terminal node
//=============================================================================
class CAirspeedIndicator : public CPitotStaticSubsystem {
public:
  CAirspeedIndicator (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  // CSubsystem methods
  const char* GetClassName (void) { return "CAirspeedIndicator"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  void            TimeSlice (float dT,U_INT FrNo = 0);		// JSDEV*
	//--- Published values --------------------------------------------
	float		GaugeBusFT01()		{return indn;}	// Airspeed
  //----ATTRIBUTES---------------------------------------------------
protected:
  float   maxOperatingMach;       ///< Max operating Mach number
  float   maxOperatingSpeed;      ///< Max operating speed in kts
};

//=============================================================================
//	CTurnCoordinator implementation
//  Terminal node
//=============================================================================
class CTurnCoordinator : public CDependent {
public:
  CTurnCoordinator (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  // CSubsystem methods
  const char* GetClassName (void) { return "CTurnCoordinator"; }
  EMessageResult      ReceiveMessage (SMessage *msg);
  void                TimeSlice (float dT,U_INT FrNo = 0);			// JSDEV*
  void                Probe(CFuiCanva *cnv);
	//--------------------------------------------------------------------
	float		GaugeBusFT01()	{return indn;}
	float   GaugeBusFT02()	{return rateD;}
  //---Attributes ------------------------------------------------------
protected:
  float Head;                 // Aircraft banking in °
	float rateT;							  // computed turning rate
  float rateD;                // Turning rate in degre
	float tilt;									// Head rate adjust
};
//=============================================================================
// JSDEV* CMagneticCompass 
//  Terminal node
//=============================================================================
class CMagneticCompass : public CDependent {
public:
  CMagneticCompass (void);

  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CMagneticCompass"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  void TimeSlice (float dT,U_INT FrNo = 0);					// JSDEV*
};

//=============================================================================
// JSEDV* complete CDigitalClockOAT
//  Terminal node
//	NOTE:  Several tag are new but are used only beteween the 
//  displaying gauge and this subsystem.  Also internal encoding is used 
//  to avoid sprintf call for CPU optimization
//	Tag 'oatC'	gives C temperature. Format (SDDD)in intData (S=sign, ddd = 3 digits)
//		'oatF'  gives F temperature (format dito)
//		'volt'  gives Voltage in intData (format VV.V)
//		'uTim'	gives U time HHMM in intData and 00SS in engine
//		'lTim'  gives local time (format dito)
//		'fTim'	gives flight time in intData (format MMSS)
//		'eTim'	gives elapse timer in intData(format MMSS)
//
//=============================================================================
class CDigitalClockOAT : public CDependent {
public:
  CDigitalClockOAT (void);

  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CDigitalClockOAT"; }
  void TimeSlice(float dT,U_INT FrNo);
  EMessageResult  ReceiveMessage (SMessage *msg);
protected:
  // DOAT methos --------------------------------
  bool	GetRequestedValue(SMessage *msg);
  bool	SetRequestedValue(SMessage *msg);
  void  EditHour(void);
  char *EditVal(int val);
  //--------In line method ---------------------
protected:
	inline void	StoreRealData(float val,SMessage *msg)
	{	msg->realData	= val;
	}
	inline void	StoreIntgData(int val,SMessage *msg)
	{	msg->intData	= val;
	}
  //---------------Attributes ----------------------------
protected:
	SDateTime		uTime;						// Universal Date time
	SDateTime		lTime;						// Local time
  CTimer      eTimer;           // Elapse timer
  CTimer      fTimer;           // Flight timer
  char        lHour[8];         // Local Hour
  char        uHour[8];         // Universal hour
  char        dVal[8];          // Other values
	float	delta;								  // Delta for hour update
	float	Volt;								    // Battery voltage
  //----temperatures ------------------------------------
  float       cTemp;            // Celsius
  float       fTemp;            // Farenheit
};
//==========================================================================
typedef enum { 
  OUTR_MRK = 0,
  MIDL_MRK = 1,
  INNR_MRK = 2,
} EMarkType;
//==========================================================================
// CMarkerPanel
//  Terminal node
//==========================================================================
class CMarkerPanel : public CDependent {
protected:
  SMessage    rMSG;             // Radio message
  BUS_RADIO  *Radio;            // primary radio
  CBeaconMark outm;             // Outter
  CBeaconMark medm;             // medium
  CBeaconMark inrm;             // inner
public:
  CMarkerPanel (void);
 ~CMarkerPanel (void);
  // CStreamObject methods
  const char* GetClassName (void) { return "CMarkerPanel"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  // CSubsystem methods ------------------------------------
  void  PrepareMsg(CVehicleObject *veh);
  //---Panel method ----------------------------------------
  void  TimeSlice(float dT,U_INT FrNo);
  void  Probe(CFuiCanva *cnv);
};
//==========================================================================
// CIndicatorSet
//  Terminal node
//==========================================================================
class CIndicatorSet : public CDependent {
public:
  CIndicatorSet (void);
 ~CIndicatorSet (void);

  //-----------------------------------------------------------------
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);
  char  ClampPos(char p);
  void  SetPosition(char p);
  void  Probe(CFuiCanva *cnv);
  EMessageResult  ReceiveMessage (SMessage *msg);
  // CSubsystem methods
  const char* GetClassName (void) { return "CIndicatorSet"; }
  //---- ATTRIBUTES--------------------------------------------------
protected:
  SMessage               *cMsg;         // Current message
  char                    sPos;         // Start position 
  char                    ePos;         // Last position
  char                    cPos;         // Current switch position
  char                    nMsg;         // Number of messages
  float                   scale;
  std::vector<SMessage*>  smsg;         ///< Indicator source messages
};

//==========================================================================
// CAnnunciatorSubsystem
// Terminal node
//==========================================================================
class CAnnunciatorSubsystem : public CDependent {
public:
  CAnnunciatorSubsystem (void);

  // CSubsystem methods
  const char* GetClassName (void) { return "CAnnunciatorSubsystem"; }
};
//==========================================================================
// CSuctionGaugeSubsystem
// Terminal node
//==========================================================================
class CSuctionGaugeSubsystem : public CDependent {
public:
  CSuctionGaugeSubsystem (void);

  // CSubsystem methods
  const char* GetClassName (void) { return "CSuctionGaugeSubsystem"; }
};
//==========================================================================
// CFlapGauge
// Terminal node
//==========================================================================
class CFlapGauge : public CDependent {
public:
  CFlapGauge (void);
  EMessageResult  ReceiveMessage (SMessage *msg);
  // CSubsystem methods
  const char* GetClassName (void) { return "CFlapGauge"; }

protected:
};
//==========================================================================
// CSynchrophaser
// Terminal node
//==========================================================================
class CSynchrophaser : public CDependent {
public:
  CSynchrophaser (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  const char* GetClassName (void) { return "CSynchrophaser"; }

protected:
  int       leftEngine;         ///< Left engine
  int       rightEngine;        ///< Right engine
};
//==========================================================================
// CEmergencyLocator
// Terminal node
//==========================================================================
class CEmergencyLocator : public CDependent {
public:
  CEmergencyLocator (void);
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  const char     *GetClassName (void) { return "CEmergencyLocator"; }
  EMessageResult  ReceiveMessage(SMessage *msg);
  //----------------------------------------------------------------
  void  NewState(SMessage *msg);
  //-----------Attributes ------------------------------------------
protected:
  float   gLim;                 // G-load activation limit
};
//=========================================================================
// * Avionics subsystems
//=========================================================================
//==========================================================================
//  DME
//==========================================================================
class CDMEPanel : public CDependent {
public:
  CDMEPanel (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CDMEPanel"; }

protected:
  bool      hold;         ///< Hold -- ???
};
//==========================================================================
//  DME2
// Terminal node
//==========================================================================
class CDMEPanel2 : public CDMEPanel {
public:
  CDMEPanel2 (void);

  // CSubsystem methods
  const char* GetClassName (void) { return "CDMEPanel2"; }

protected:
};
//==========================================================================
//  Weather Radar
//==========================================================================
class CWeatherRadar : public CDependent {
public:
  CWeatherRadar (void);

  // CSubsystem methods
  const char* GetClassName (void) { return "CWeatherRadar"; }

protected:
};

class CCollinsPFD : public CDependent {
public:
  CCollinsPFD (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CCollinsPFD"; }

protected:
};

class CCollinsND : public CDependent {
public:
  CCollinsND (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CCollinsND"; }

protected:
};

class CCollinsAD : public CDependent {
public:
  CCollinsAD (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CCollinsAD"; }

protected:
};

class CCollinsMND : public CDependent {
public:
  CCollinsMND (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CCollinsMND"; }

protected:
};

class CCollinsRTU : public CDependent {
public:
  CCollinsRTU (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CCollinsRTU"; }

protected:
};

class CPilatusPFD : public CDependent {
public:
  CPilatusPFD (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPilatusPFD"; }

protected:
};

class CVirtualGPS : public CDependent {
public:
  CVirtualGPS (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CVirtualGPS"; }

protected:
};

class CTCASPanel : public CDependent {
public:
  CTCASPanel (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTCASPanel"; }

protected:
};

class CFMSys : public CDependent {
public:
  CFMSys (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFMSys"; }

protected:
};

//================================================================================
// CEngineSubsystem
//
// This subsystem is an abstract base class for any subsystems which have a 
//   dependency on an engine
//===================================================================================
class CEngineSubsystem : public CDependent {
  //---- ATTRIBUTES -----------------------------------------------------------------
protected:
  CVehicleObject *mveh;                 // Mother vehicle
  CEngine        *meng;                 // Mother engine  
  //---- METHODS --------------------------------------------------------------------
public:
  CEngineSubsystem (void);

  // CSubsystem methods
  virtual const char*     GetClassName      (void) { return "CEngineSubsystem"; }
  virtual void            TimeSlice         (float dT, U_INT FrNo);		// JSDEV*
  //--------------------------------------------------------------------------------
public:
  CEngineData   *eData;             // Engine data
  //--------------------------------------------------------------------------------
  inline void   SetEngineData(CEngineData *d) {eData = d;}
};

//
// CEngineGauge
//
class CEngineGauge : public CEngineSubsystem {
public:
  CEngineGauge (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CEngineGauge"; }
};



//
// CFuelGaugeSubsystem
//
class CFuelGaugeSubsystem : public CEngineSubsystem {
public:
  CFuelGaugeSubsystem (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFuelGaugeSubsystem"; }
};

//
// CFuelFlowSubsystem
//
class CFuelFlowSubsystem : public CEngineSubsystem {
public:
  CFuelFlowSubsystem (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFuelFlowSubsystem"; }
};

//
// CFuelPressureSubsystem
//
class CFuelPressureSubsystem : public CEngineSubsystem {
public:
  CFuelPressureSubsystem (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFuelPressureSubsystem"; }
};

//
// CN1Tachometer
//
class CN1Tachometer : public CEngineSubsystem {
public:
  CN1Tachometer (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CN1Tachometer"; }
};

//
// CN2Tachometer
//
class CN2Tachometer : public CEngineSubsystem {
public:
  CN2Tachometer (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CN2Tachometer"; }
};

//
// CTorqueMeter
//
class CTorqueMeter : public CEngineSubsystem {
public:
  CTorqueMeter (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTorqueMeter"; }
};


//
// CInterTurbineTemp
//
class CInterTurbineTemp : public CEngineSubsystem {
public:
  CInterTurbineTemp (void);

  // CStreamObject methods
  virtual int Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CInterTurbineTemp"; }

protected:
  bool        degC;         ///< Report temperature in degrees C
};

//=======================================================================================
//
// CStarterIgniter
//
class CStarterIgniter : public CEngineSubsystem {
public:
  CStarterIgniter (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CStarterIgniter"; }
};

//
// CFireSystem
//
class CFireSystem : public CEngineSubsystem {
public:
  CFireSystem (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFireSystem"; }
};

//
// CFuelImbalance
//
class CFuelImbalance : public CEngineSubsystem {
public:
  CFuelImbalance (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFuelImbalance"; }
};

//
// CFuelPumpControl
//
class CFuelPumpControl : public CEngineSubsystem {
public:
  CFuelPumpControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFuelPumpControl"; }
};

//
// CCrossfeedControl
//
class CCrossfeedControl : public CEngineSubsystem {
public:
  CCrossfeedControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CCrossfeedControl"; }
};


//---------------------------------------------------------------------------------------------------------
// *
// * CPneumaticSubsystem
// *
// NB : this section *must* be situated after the CEnginesubsystem section
//      for the CEngineSubsystem *pEng engine pointer 
// 
// This subystem is an abstract base class for any subsystems which have a 
//   dependency on a pneumatic pump subsystem
//
//---------------------------------------------------------------------------------------------------------
class CPneumaticSubsystem : public CDependent {
public:
  CPneumaticSubsystem();
 ~CPneumaticSubsystem();
  // CStreamObject methods
  virtual int   Read               (SStream *stream, Tag tag);
//  virtual void  ReadFinished     (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPneumaticSubsystem"; }
//  virtual EMessageResult  ReceiveMessage (SMessage *msg);
  void	PrepareMsg(CVehicleObject *veh);				// JSDEV* Prepare Messages
  virtual void TimeSlice           (float dT,U_INT FrNo);			// JSDEV*

protected:
  std::vector<SMessage>            mPmp;				//< Dependent pump messages
  bool                             regulated;			//< System is pressure regulated
  float							   timer;				// Poll timer	
};

//---------------------------------------------------------------------------
// CAttitudeIndicator
//---------------------------------------------------------------------------
class CAttitudeIndicator : public CPneumaticSubsystem {
public:
  CAttitudeIndicator                     (void);

  // CStreamObject methods
	virtual int             Read           (SStream *stream, Tag tag);
  // CSubsystem methods
	virtual const char*		GetClassName   (void) { return "CAttitudeIndicator"; }
	virtual EMessageResult	ReceiveMessage (SMessage *msg);
	virtual void			TimeSlice      (float dT,U_INT FrNo);
  //--------------------------------------------------------------------
  void    Probe(CFuiCanva *cnv);
	bool	  TrackTarget(float dT);
  int     UpdateLevel(int inc);
	float		GaugeBusFT01();
	float		GaugeBusFT02();
	//--------------------------------------------------------------------
	inline	float			Clamp180(float deg)
							{	if (deg >  180) return +180;
								if (deg < -180) return -180;
								return deg;	}
  //---------ATTRIBUTES ------------------------------------------------
protected:
  float     prat;                         // Precession rate
  bool      regulated_flag;               // true = maintain op. suction when deice is in use
  int       hdeg;                         // Horizon level [-15°,+15°]
private: // precession stuff
  double    P_LIMIT;                      // pitch lower limit in precession
  double    R_LIMIT;                      // roll  lower limit in precession
  //-----Internal state --------------------------------------------------
  float     delay;                       // timer for precession
  //-----------------------------------------------------------------------
  enum DState {	STOPPED = 0,			// Stopped
				INTROPC = 1,			// Enter precession
				PRECESS	= 2,			// Precession
				TO_USER	= 3,			// Target to user
				TOLIMIT = 4,			// To limit
				RUNING	= 5,
				TOLEFT	= 6,
				TORIGH	= 7,	};
  //----------------------------------------------------------------------
  float		operP;						// Operational pressure (need adjustement)
  float		Timer;						// Timer for precess
  float		incp;						// increment pitch
  float		incr;						// increment roll
  float		ampP;
  float		ampR;
  DState	Etat;						// Internal state
  SVector	Uatt;						// User vehicle attitude
  SVector	Tatt;						// Target attitude tracked by Uatt
};
//-------------------------------------------------------------------------------
//	JSDEV* Modified for error computing and local coordinate
// CDirectionalGyro
//-------------------------------------------------------------------------------
class CDirectionalGyro : public CPneumaticSubsystem {
public:
  CDirectionalGyro (void);

  // CStreamObject methods
  virtual int             Read (SStream *stream, Tag tag);
//  virtual void          ReadFinished (void);

  // CSubsystem methods
  virtual const char*     GetClassName (void) { return "CDirectionalGyro"; }
  virtual EMessageResult  ReceiveMessage (SMessage *msg);
  virtual void            TimeSlice (float dT,U_INT FrNo = 0);		// JSDEV*
	//----------------------------------------------
protected:
  float     step;         ///< Heading bug increment/decrement step size
  bool      autoAlign;    ///< Gyro is auto aligned
  void      UpdateGyro(float inc);
  void      UpdatePbug(float inc);
	//--- Published values on Gauge BUS --------------
	float     GaugeBusFT01()		{return aYaw;}
	float			GaugeBusFT02()		{return abug;}
	int			  GaugeBusIN03()		{return abug;}
  //---------ATTRIBUTES ---------------------------
private:
  float						aYaw;		// Actual yaw
  float						tYaw;		// Target yaw
  float						aRat;		// Align rate
  float						eRate;		// error rate
  float						Error;
  //-------------------------------------------------
  float           gyro;     // GYRO plate
  float           rbug;     // Auto pilot bug relative position
  int             abug;     // bug absolute position
};

//=====================================================================
// CVacuumIndicator
//=====================================================================
class CVacuumIndicator : public CPneumaticSubsystem {
public:
  CVacuumIndicator (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  // CSubsystem methods
  const char*     GetClassName (void) { return "CVacuumIndicator"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
	//--- Publish values on gauge bus -------------------
	float GaugeBusFT01()			{return indn;}	// suction Pressure
};


//=====================================================================
// * Control subsystems
//=====================================================================
struct MIXER_DATA {
  char  chn[8];         // Chanel
  float raw;            // Raw control value in range [0,1] or [-1,+1]
  float step;           // Step size
  float bend;           // Exponential value
  float posn;           // Default position
  float deflect;        // Deflection value in normalized range [0,1] or [-1,+1]
  float scaled;         // scaled data
  float scal;           // Scaling factor
  float minClamp;       // Minimum value of the control subsystem
  float maxClamp;       // Maximum value of the control subsystem
  float ampli;          // Amplitude

};
//=======================================================================
// CAeroControl
//  JSNOTE to LC: 1) remove coding related to  engine identification as those 
//          objects are not related to engine.  
//          Objects that need engine addrssing (like throttle for instance)
//          must be derived from CEngineControl
//          2) Added a data structure for reading in one message all data
//             internally.
//========================================================================
class CAeroControl : public CDependent {
public:
  CAeroControl (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished();
  // CSubsystem methods
  virtual const char*     GetClassName (void) { return "CAeroControl"; }
  virtual EMessageResult  ReceiveMessage (SMessage *msg);
  virtual void            TimeSlice (float dT,U_INT FrNo = 0);		// JSDEV*

  // CAeroControl SET methods
  virtual void    Incr          (void);
  virtual void    Decr          (void);
  virtual void    SetValue      (float fv);
  virtual void    Zero()        {data.raw = 0;}
	virtual void    ModBias(float v)	{;}
	//------------------------------------------------------------
  void						Modify(float dt);
	void						Transfer();
  //------------------------------------------------------------
  void            Probe(CFuiCanva *cnv);
  //--- For all controls ---------------------------------------
  float   Clamp(float v);
  //-------------------------------------------------------------
  void    GetClamp(float &m1,float &m2) {m1 = data.minClamp,m2= data.maxClamp;}
  //---- for value ---------------------------------------------
  inline  float    Val()   {return data.raw;}
  inline  float Deflect()  {return data.deflect;};
	inline  float UnBias()	 {return data.deflect - Bias;}
  //--- For autopilot ------------------------------------------
	inline  void	   Neutral()	{vPID = 0; data.raw = 0;} 
  inline  void     PidValue(double v)   {vPID = v;}
  inline  void     SetMainControl(CAeroControl *c) {Cont = c;}
  //---ATTRIBUTS -----------------------------------------------
protected:
  CAeroControl    *Cont;        // Associated control for trim
  MIXER_DATA       data;				// mixer data
  double  vPID;									///< FCS value (autopilot)
  double  Bias;									// Trim deviation
	float		timer;								// available for controls
public:
};
//=====================================================================
// CAileronControl
//=====================================================================
class CAileronControl : public CAeroControl {
public:
  CAileronControl (void);

  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CAileronControl"; }
  void                TimeSlice (float dT,U_INT FrNo = 0);					// JSDEV*

protected:
};

//=====================================================================
// CElevatorControl
//=====================================================================
class CElevatorControl : public CAeroControl {
public:
  CElevatorControl (void);

  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CElevatorControl"; }
  void                TimeSlice (float dT,U_INT FrNo = 0);

protected:
};
//======================================================================
// CRudderControl
//  NOTE: This control is attached to CAeroControl
//=======================================================================
class CRudderControl : public CAeroControl {
protected:
	//--- ATTRIBUTES ------------------------------------------
	double	pidK;						// PID Direct coefficient
	double	pidD;						// PID derived coefficient
	double	pidI;						// PID integral coefficient
	//----Raw adjustement --------------------------------
  CFmtxMap      *macrd;			// Adjustement table
  float          oADJ;			//Specific Opal adjustement
	SGearData     *steer;			//  Steering wheel
  CSteeringControl *sCTLR;	// Controller
	//----------------------------------------------------------
public:
  CRudderControl (void);
  void    Adjust();
  // CSubsystem methods
  const char* GetClassName (void) { return "CRudderControl"; }
	//--- Read Parameters -------------------------------------
  int       Read(SStream *st,Tag tag);
  //---Rudder methods ---------------------------------------
	void			SetCoef(CPIDbox *B); 
	void			ModBias(float v);
  void      TimeSlice (float dT,U_INT FrNo = 0);					
  //---------------------------------------------------------
	inline void InitSteer(SGearData *s) {steer = s;}
	inline void InitCTLR (CSteeringControl *s) {sCTLR = s;}
  inline void SetOpalCoef(float c)    {oADJ = c;}
  inline void SetBankMap(CFmtxMap *m) {macrd = m;}
protected:
};
//====================================================================
//  Flap data structure
//====================================================================
struct FLAP_POS{
  int   degre;          // FLAP degre for the position
  float speed;          // Speed limit before dammage
  float kFrame;         // Related keyframe
};
//====================================================================
// CFlapControl
//====================================================================
class CFlapControl : public CAeroControl {
public:
  CFlapControl (void);
 ~CFlapControl (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CAeroControl SET methods
  void    Incr (void);
  void    Decr (void);
  void    TimeSlice (float dT,U_INT FrNo = 0);
  void    NewPosition(int pos);
  void    SetPosition(SMessage *msg);
	void		SetPosition(int pos);
	float   GetMaxSpeed(int pos);
  // CSubsystem methods
  const char* GetClassName (void) { return "CFlapControl"; }
  EMessageResult  ReceiveMessage (SMessage *msg);

protected:
  CAnimatedModel *vlod;       // Level of detail
  float     fTim;             // Time for full flap        
  float     flod;             //< Motor current load when operating
  int       dump;             //< Dump position in pos array
  int       nPos;             //< Number of flap positions
  FLAP_POS *aPos;             //< Array of flap positions
  int       actualPos;
  //
};
//======================================================================
// CAlternator
//  Terminal node
//======================================================================
class CAlternator : public CDependent {
public:
  CAlternator (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CSubsystem methods
  const char* GetClassName (void) { return "CAlternator"; }
  EMessageResult ReceiveMessage (SMessage *msg);
  void        TimeSlice(float dT,U_INT FrNo);
  //---ATTRIBUTES ---------------------------------
protected:
  SMessage  emsg;     // Engine Message
  float     mxld;     // Maximum load (amps)
  float     mvlt;     // maximum volt
  float     loRg;     // Minimum RPM regulatable
  float     hiRg;     // maximum RPM regulated
  float     mxRg;     // maximum voltage RPM
  float     vFac;     // Volt coeff
};



//======================================================================
// CSteeringControl
//======================================================================
class CSteeringControl : public CDependent {
public:
  CSteeringControl (void);
  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CSteeringControl"; }

protected:
};

//======================================================================
// CElevatorTrimControl
//======================================================================
class CElevatorTrimControl : public CAeroControl {
public:
  CElevatorTrimControl (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CElevatorTrimControl"; }
	void								ReadFinished();
  void                TimeSlice (float dT,U_INT FrNo = 0);					// SDEV*
	//--- Overloaded ---------------------------------------
	void Incr();
	void Decr();
	//--- Attribute ----------------------------------------
protected:
	char ok;
};

//======================================================================
// CAileronTrimControl
//======================================================================
class CAileronTrimControl : public CAeroControl {
public:
  CAileronTrimControl (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CAileronTrimControl"; }

protected:
};

//
// CRudderTrimControl
//
class CRudderTrimControl : public CAeroControl {
public:
  CRudderTrimControl (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CRudderTrimControl"; }

protected:
};
//==============================================================================
// CBrakeControl
//==============================================================================
// JS NOTE:  Brake control receive messages from
//    Keyboard  for left brake
//              for right brake
//              for both brakes
//    Gauge for parking brake
//  Any change is propagated to CGroundBrake Object
//===========================================================================
class CBrakeControl : public CDependent { 
public:
  CBrakeControl (void);

  // CStreamObject methods
  void    ReadFinished();
  // CAeroControl SET methods
  void    Incr (char pos, float rt);
  int     Decr (char pos, float rt);
  void    PressBrake(char pos);
  void    ReleaseBrakes();
  void    SwapPark(U_CHAR opt);
	void		Probe(CFuiCanva *cnv);
  //--------------------------------------------------------------------
  inline void   HoldBrake(char p)     {Hold = p;}
	inline float  GetBrakeForce(char p) {return Force[p];}
  inline void   SetParking()          {Park = 1;}
	//--- Published values on Gauge Bus ----------------------------------
	inline int		GaugeBusIN01()				{return Park;}
  //--------------------------------------------------------------------
  // CSubsystem methods
  const char* GetClassName (void) { return "CBrakeControl"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  void            TimeSlice (float dT,U_INT FrNo = 0);		// JSDEV*

protected:
  char  Park;
  char  Hold;         // Brake on
	char  bKey;					// Brake key
  float rate;
  float Brake[3];     // Brake force
  float Force[3];     // Real force
  char  turn;         // set which timeslice do the brake release
  float br_timer;
  float rf_timer;     // reference
};

//===========================================================================
// CGearControl
//===========================================================================
class CGearControl : public CDependent{
public:
  CGearControl (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CSubsystem methods
  const char* GetClassName (void) { return "CGearControl"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  //---Control methods ---------------------------------------
  void  Swap();
  void  RotateGear();
  //----------------------------------------------------------
  char  Lock()        {return lock;}
  bool  Active()      {return active;}
  //----ATTRIBUTES -------------------------------------------
protected:
  char    hydr;               ///< Gear is hydraulically actuated
  char    lock;               ///< Gear lever is locked when on ground
  int     Pos;
};
//===========================================================================
// CHighLiftControl
//===========================================================================
class CHighLiftControl : public CAeroControl {
public:
  CHighLiftControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CHighLiftControl"; }

protected:
};

//
// CCowlFlap
//
class CCowlFlapControl : public CAeroControl {
public:
  CCowlFlapControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CCowlFlap"; }

protected:
};

//==========================================================================
// CAirBrakeControl
//==========================================================================
class CAirBrakeControl : public CAeroControl {
public:
  virtual const char* GetClassName (void) { return "CAirBrakeControl"; }
  CAirBrakeControl (void);
  int                 Read (SStream *stream, Tag tag);
//----Attributes ------------------------------------------------
protected:
  SMessage msg;                           // Flap signal                
};


//
// CAudioPanel
//
class CAudioPanel : public CDependent {
public:
  CAudioPanel (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CAudioControl"; }

protected:
};

//
// CEngineAntiIce
//
class CEngineAntiIce : public CDependent {
public:
  CEngineAntiIce (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CEngineAntiIce"; }

protected:
};

//
// CSurfaceDeIce
//
class CSurfaceDeIce : public CDependent {
public:
  CSurfaceDeIce (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CSurfaceDeIce"; }

protected:
};

//
// CPropellerDeIce
//
class CPropellerDeIce : public CDependent {
public:
  CPropellerDeIce (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPropellerDeIce"; }

protected:
  float             cycleTime;        ///< Deice cycle time
  std::vector<int>  propHeater;       ///< Array of propeller heaters
  float             heaterLoad;       ///< Heater load (amps)
};

//=====================================================================================
// CWingDeIce
//=====================================================================================
class CWingDeIce : public CDependent {
public:
   CWingDeIce (void);
  ~CWingDeIce (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CWingDeIce"; }

protected:
  float                   cycleTime;  ///< Deice cycle time in seconds
  std::vector<SMessage*>  msgPump;    ///< List of pump connection messages
};

//=====================================================================
// CTempControl
//=====================================================================
class CTempControl : public CDependent {
public:
  CTempControl (void);
  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTempControl"; }
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  Probe(CFuiCanva *cnv);
  virtual void  TimeSlice (float dT,U_INT FrNo);
  virtual EMessageResult ReceiveMessage (SMessage *msg);
protected:
  //--------------------------------------------------------
  float base;                             // Base tic
  float rate;                             // Rate of change
  float temp;                             // Temperature
  float sign;                             // Sign of increment
  int   mint;                             // mini
  int   maxt;                             // maxt
};

//====================================================================
// CWiperControl
//====================================================================
class CWiperControl : public CDependent {
public:
  CWiperControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CWiperControl"; }

protected:
};

//
// CPerformanceReserve
//
class CPerformanceReserve : public CDependent {
public:
  CPerformanceReserve (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPerformanceReserve"; }

protected:
};

//
// CRudderBias
//
class CRudderBias : public CDependent {
public:
  CRudderBias (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CRudderBias"; }

protected:
  int       rpmOption;        ///< RPM option (0=RPM, 1=N1, 2=N2)
  float     rpmLimit;         ///< RPM limit
  int       leftEngine;       ///< Left engine index
  int       rightEngine;      ///< Right engine index
};

//
// COxygen
//
class COxygen : public CDependent {
public:
  COxygen (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "COxygen"; }

protected:
  float     full;
  float     fill;
  float     rate;
  float     pressure;
};


// *
// * Helicopter subsystems
// * 

//
// CRotorSubsystem
//
class CRotorSubsystem : public CDependent {
public:
  CRotorSubsystem (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CRotorSubsystem"; }

protected:
};

//
// C407DigitalIndicators
//
class C407DigitalIndicators : public CDependent {
public:
  C407DigitalIndicators (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "C407DigitalIndicators"; }

protected:
};

//
// CHookSubsystem
//
class CHookSubsystem : public CDependent {
public:
  CHookSubsystem (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CHookSubsystem"; }

protected:
};

//
// CThrustPitchControl
//
class CThrustPitchControl : public CDependent {
public:
  CThrustPitchControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CThrustPitchControl"; }

protected:
};

//
// CLonCyclicControl
//
class CLonCyclicControl : public CDependent {
public:
  CLonCyclicControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CLonCyclicControl"; }

protected:
};

//
// CLatCyclicControl
//
class CLatCyclicControl : public CDependent {
public:
  CLatCyclicControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CLatCyclicControl"; }

protected:
};

//
// CTailRotorControl
//
class CTailRotorControl : public CDependent {
public:
  CTailRotorControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTailRotorControl"; }

protected:
};

//
// CCollectiveControl
//
class CCollectiveControl : public CDependent {
public:
  CCollectiveControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CCollectiveControl"; }

protected:
};

//
// CLonCyclicTrimControl
//
class CLonCyclicTrimControl : public CDependent {
public:
  CLonCyclicTrimControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CLonCyclicTrimControl"; }

protected:
};

//
// CLatCyclicTrimControl
//
class CLatCyclicTrimControl : public CDependent {
public:
  CLatCyclicTrimControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CLatCyclicTrimControl"; }

protected:
};

//
// CTailRotorTrimControl
//
class CTailRotorTrimControl : public CDependent {
public:
  CTailRotorTrimControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTailRotorTrimControl"; }

protected:
};

//
// CCollectiveTrimControl
//
class CCollectiveTrimControl : public CDependent {
public:
  CCollectiveTrimControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CCollectiveTrimControl"; }

protected:
};




//==================================================================================
// CNavigation
//==================================================================================
class CNavigation : public CDependent {
  //----Attributes ----------------------------------------------------
  BUS_RADIO *radio;
  SMessage   msg;
  //-------------------------------------------------------------------
public:
  CNavigation (void);
  int   SetXOBS(short inc);
  //------------------------------------------------------------------
  // CStreamObject methods

  // CSubsystem methods
  virtual const     char* GetClassName (void) { return "CNavigation"; }
  virtual EMessageResult  ReceiveMessage (SMessage *msg);
  //------------------------------------------------------------------
};

//==========================================================================
// CAudioPanelRadio
//
class CAudioPanelRadio : public CDependent {
public:
  CAudioPanelRadio (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CAudioPanelRadio"; }
//  virtual EMessageResult  ReceiveMessage (SMessage *msg);

protected:
};

//
// CBKKAP140Radio
//
class CBKKAP140Radio : public CDependent {
public:
  CBKKAP140Radio (void);

  // CStreamObject methods
//  virtual int   Read (SStream *stream, Tag tag);
//  virtual void  ReadFinished (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CBKKAP140Radio"; }
//  virtual EMessageResult  ReceiveMessage (SMessage *msg);

protected:
};


// *
// * Vehicle state subsystems
// *

//
// CHistory
//
class CHistory : public CDependent {
public:
  CHistory (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CHistory"; }
};

//
// COnGroundMonitor
//
class COnGroundMonitor : public CDependent {
public:
  COnGroundMonitor (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "COnGroundMonitor"; }
};

//
// CAltitudeMonitor
//
// This subsystem monitors the user vehicle altitude and compares it to a preset
//   altitude threshold.  The subsystem state reflects whether the user vehicle
//   is above or below the threshold altitude.
//
class CAltitudeMonitor : public CDependent {
public:
  CAltitudeMonitor (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CAltitudeMonitor"; }
};

//
// CSpeedMonitor
//
// This subsystem monitors the user vehicle airspeed and compares it to a preset
//   threshold value.  The subsystem state reflects whether the user vehicle
//   is travelling faster or slower than the threshold airspeed.
//
class CSpeedMonitor : public CDependent {
public:
  CSpeedMonitor (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CSpeedMonitor"; }
};

//
// CStallWarning
//
class CStallWarning : public CDependent {
public:
  CStallWarning (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CStallWarning"; }

protected:
  char    wing[64];              ///< Wing section name
  bool    enableStateAnnouncer;
};

//
// CFastSlowMeter
//
class CFastSlowMeter : public CDependent {
public:
  CFastSlowMeter (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFastSlowMeter"; }
};

//
// CPressurization
//
class CPressurization : public CDependent {
public:
  CPressurization (void);

  // CStreamObject methods
  int Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPressurization"; }

protected:
  bool      switchCabinPressure;      ///< Cabin pressure switch state
  bool      switchPressureControl;    ///< Pressure control switch state
  float     diffPressure;             ///< Maximum differential pressure
  float     warningAlt;               ///< Warning altitude
};

//
// CAccelerometer
//
class CAccelerometer : public CDependent {
public:
  CAccelerometer (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CAccelerometer"; }
};

//=============================================================================
// CGearLight
//=============================================================================
class CGearLight : public CDependent {
  //------METHODS ---------------------------------------------
public:
  CGearLight (void);

  // CStreamObject methods
  int     Read (SStream *stream, Tag tag);
  void    ReadFinished();
  char    ClampMode(char m);
  // CSubsystem methods
  const char* GetClassName (void) { return "CGearLight"; }
  //-----------------------------------------------------------
  void    TimeSlice(float dT,U_INT FrNo);
  //-------ATTRIBUTES -----------------------------------------
protected:
  CAnimatedModel *vlod;
  char    mode;               // Monitor mode (0=Extended, 1=Retracted, 2=Transit)
  char    gNum;               // Gear number
};

//=============================================================================
// CStallIdent
//
class CStallIdent : public CDependent {
public:
  CStallIdent (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CStallIdent"; }
};

//
// CTempAirspeedDisplay
//
class CTempAirspeedDisplay : public CDependent {
public:
  CTempAirspeedDisplay (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTempAirspeedDisplay"; }
};

//
// CGearWarning
//
class CGearWarning : public CDependent {
public:
  CGearWarning (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CGearWarning"; }

protected:
  float   throttleLimit;      ///< Minimum throttle to inhibit warning
  float   flapLimit;          ///< Minimum flaps to inhibit warning
  bool    odd;                ///< Gear position and lever agreement
};


//
// CTachometerBase
//
class CTachometerBase : public CEngineSubsystem {
public:
  CTachometerBase (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTachometerBase"; }
};

//
// CTachometerV1
//
class CTachometerV1 : public CTachometerBase {
public:
  CTachometerV1 (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTachometerV1"; }
};


/*!
 *
 *  CFuelSubsystem class
 *
 *  This class is the base for all the subsystems found in the GAS (fuel system)
 *  stream file.
 *
 *  An overall_upstream_state is maintained throughtout the derived class in order
 *  to enable or disable the fuel FLOW from a cell or tank. The BURNING is enabled with
 *  a fuel_burning_flag.
 * 
 *  The pipes objects yield a list of recipient message (pipe_msg).
 *  A list of related cells is also created and maintained (related_cell) :
 *   CFuelSystem::ReadFinished
 *   A pointer of a cell fuel is created and saved as a valve object in order
 *   to enable or disable fuel consumption.
 *  
 *  TimeSlice :
 *  It sends the overall_upstream_state to all the derived classes.
 *  It enables fuel_burning_flag of each cell.
 *
 */

//
// 
//

//====================================================================================
//  Fuel subsystem
//===================================================================================
class CFuelSubsystem : public CDependent {
public:
  // Constructor
  CFuelSubsystem (void);
  virtual ~CFuelSubsystem (void);

  // CStreamObject methods
  virtual int                   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char*           GetClassName (void) { return "CFuelSubsystem"; }
  virtual	EMessageResult        ReceiveMessage (SMessage *msg);
  void		PrepareMsg(CVehicleObject *veh);							// JSDEV* Prepare Messages
  virtual void                  TimeSlice (float dT,U_INT FrNo = 0);	// JSDEV*	
  //-------------------------------------------------------------------
  virtual bool   IsOFF()           {return (0 == state);}
  virtual char   ByPass()          {return 'N';}
  //-------------------------------------------------------------------
  void    LinkCell(CFuelSystem *fsys);
  void    TraceLink(CFuelSubsystem *fs);
  void    TraceFS(CFuelSubsystem *fs, float rq);
  void    TraceT1(CFuelCell *fc,float frq);
  void    GetContributingTanks(CFuelSystem *fsys,float rqt);
  void    TraceT2(CFuelCell *fc, CFuelCell *tk,  float frq);
  void    GetRefillingTanks(CFuelSystem *fsys,CFuelCell *cel,float rqf);
  void    Poll();
  //-------------------------------------------------------------------
  inline const char*            GetName (void) const {return name;}
  //-------------------------------------------------------------------
  inline bool   IsaTank()           {return (1 == Fsrc);}
  inline bool   NotTank()           {return (0 == Fsrc);}
  inline void   SetName(char *n)    {strncpy(name,n,64); name[63] = 0;}
  inline U_INT  EngineNo()          {return eNum;}
  //-------------------------------------------------------------------
  // Data members
public:
  float                         f_gph;            ///< Fuel flow through tap (gallons per hour)
  float                         gals;             ///< Fuel quantity in gallons

//protected:
  char                          name[64];         ///< UI readable name
  //-------------------------------------------------------------------
  std::map<Tag,CFuelSubsystem*> piped;            // Piped subsystems
  //-----ATTRIBUTES ---------------------------------------------------
public:
  bool                          force_fuel;
  char                          Fsrc;             // Tank indicator
  char                          Tr;               // Trace request
};

//===========================================================================
// CFuelTap
//===========================================================================
class CFuelTap : public CFuelSubsystem {
public:
  // Constructor
  CFuelTap (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);

  // CSubsystem methods
  const char* GetClassName (void) { return "CFuelTap"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  void TimeSlice (float dT,U_INT FrNo);
  //-------------------------------------------------------------
  inline void SetFuelFlow(float ff) {f_gph = ff;}
  //---- Data members ---------------------------------------------
protected:
  float     stff;       // Startup fuel flow
};
//======================================================================
// CFuelCell
//======================================================================
#define FUEL_FEED_STACK (16)
//======================================================================
//  NOTE:  All data from CFuelUnit are moved here
//======================================================================
class CFuelCell : public CFuelSubsystem {
public:
  // Constructor
  CFuelCell (U_CHAR ind);
  CFuelCell ();
 ~CFuelCell();
  void Create();
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);
  //-------------------------------------------------------------
  virtual float   GetLimitQty()       {return qty;}
  //---------- CSubsystem methods -------------------------------
  const char*     GetClassName (void) { return "CFuelCell"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  void            TimeSlice (float dT,U_INT FrNo);

  //--------------------------------------------------------------
  void                    BurnFuel(float bnf);
  void                    SetCellQty(float qty);
  void                    Probe(CFuiCanva *cnv);
  bool                    CanFill(CFuelCell *cel,float rqf);
  void                    GiveFuelTo(CFuelCell *tk, float rqf);
  void                    AddFuel(float fqt);
  void                    RemFuel(float fqt);
  void                    ClearFeed();
  //--------------------------------------------------------------
  void                    ContributionCG(SVector &mm, SVector &vn, double &sm);
  //--------------------------------------------------------------
  inline  char            GetCellIndex()      {return xInd;}
  inline  void            SetCellWgh(float w) {wgh = w;};
  inline  void            SetCellCap(float c) {cap = c;}
  inline  SVector         GetCellPos () {return bPos;}
  inline  float           GetCellQty () {return gals;}
  inline  float           GetCellCap () {return cap;}
  inline  float           GetCellWgh () {return wgh;}
  inline  float           GetCellPCT () {return (gals * 100) / cap;}
  inline  float           GetXXRate()   {return xrFF;}
  inline  float           GetRemainingCap()   {return (cap - qty);}
  inline  float           GetXloc()           {return float(bPos.x);}
  inline  float           GetYloc()           {return float(bPos.y);}
  inline  float           GetZloc()           {return float(bPos.z);}
  inline  SVector         GetPosition()       {return bPos;}
  //--------------------------------------------------------------
  inline bool   HasFuelFor(float q) {return (gals >= q);}
  inline void   DumpFuel()    {gals = qty = 0;}
  //--------------------------------------------------------------
  // Data members
protected:
  float   cap;              //< Capacity (gallons)
  float   qty;              //< Default quantity (gallons)
  float   wgh;              //< Weight
  CVector bPos;             //< Fuel cell position relative to aircraft CoG
  float   basP;             //< Base pressure (psi)
  //float   Dqty;           //< Fuel burned negative
  bool    xfer;             //< Forced fuel transfer
  float   xrFF;             //< Forced fuel transfer flow rate (gal/sec)
  //---Stack index for this cell -----------------------------
  U_CHAR  xInd;            // Cell index into Feed
  //---Stack for 16 fuel cells -------------------------------
  Tag     Feed[FUEL_FEED_STACK];         // Room for 16 associations
  //----------------------------------------------------------
private:
  float   fuel_burned;      // Total burned for info

};
//========================================================================
//  CFuelCollector is a particulaer cell used as a fuel buffer
//========================================================================
class CFuelCollector: public CFuelCell {
  //----No specific attribute --------------------------------
public:
  CFuelCollector(U_CHAR ind);
  //----------------------------------------------------------
  inline float GetLimitQty()  {return 0;}
};
//========================================================================
// CFuelHeater
//========================================================================
class CFuelHeater : public CFuelSubsystem {
public:
  // Constructor
  CFuelHeater (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFuelHeater"; }

// Data members
protected:
};


//========================================================================
// CFuelMath
//========================================================================
class CFuelMath : public CFuelSubsystem {
public:
  // Constructor
  CFuelMath (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFuelMath"; }

// Data members
protected:
};


//=========================================================================
// CFuelPump
//=========================================================================
class CFuelPump : public CFuelSubsystem {
public:
  // Constructor
  CFuelPump (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  //------------------------------------------------------------
  bool  IsOFF();
  //------------------------------------------------------------
  inline char ByPass() {return (pass)?('Y'):('N');}
  //------------------------------------------------------------
  // CSubsystem methods
  const char* GetClassName (void) { return "CFuelPump"; }
  void TimeSlice (float dT,U_INT FrNo = 0);			// JSDEV*

protected:
  float setP;    ///< Pump pressure
  bool  pass;    ///< Allow fuel passthrough when system is off
  bool  enabled; ///< Enabled by default
};


//========================================================================
// CFuelSource
//========================================================================
class CFuelSource : public CFuelSubsystem {
public:
  // Constructor
  CFuelSource (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFuelSource"; }

protected:
};


//===========================================================================
// CFuelTank
//===========================================================================
class CFuelTank : public CFuelCell {
public:
  // Constructor
  CFuelTank (U_CHAR ind);

  virtual const char* GetClassName (void) { return "CFuelTank"; }

protected:
};

//========================================================================
// CEngineFuelPump
//========================================================================
class CEngineFuelPump : public CFuelSubsystem {
public:
  // Constructor
  CEngineFuelPump (void);
  bool  IsOFF();
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  //----------------------------------------------------------
  inline char ByPass()    {return (pass)?('Y'):('N');}
  // CSubsystem methods
  const char* GetClassName (void) { return "CEngineFuelPump"; }

protected:
  bool  pass;
};

//========================================================================
// CFuelManagement
//========================================================================
class CFuelManagement : public CFuelSubsystem {
public:
  // Constructor
  CFuelManagement (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFuelManagement"; }

protected:
};

//========================================================================
// CAPUFuelSource
//========================================================================
class CAPUFuelSource : public CFuelSubsystem {
public:
  // Constructor
  CAPUFuelSource (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CAPUFuelSource"; }

protected:
};

//========================================================================
// * Simulation model subsystems
//========================================================================

//
// CAirModel
//
class CAirModel : public CSubsystem {
public:
  CAirModel (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CAirModel"; }
};

//
// CMixerModel
//
class CMixerModel : public CSubsystem {
public:
  CMixerModel (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CMixerModel"; }
};

//
// CWheelModel
//
class CWheelModel : public CSubsystem {
public:
  CWheelModel (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CWheelModel"; }
};


/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

 Author:       Jon S. Berndt
 Date started: 08/24/00

 ------------- Copyright (C) 2000  Jon S. Berndt (jsb@hal-pc.org) -------------

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

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
class CPropellerModel : public CEngineSubsystem {
  //--- ATTRIBUTES ------------------------------------------
protected:
  CVehicleObject *mveh;             // Parent vehicle
  //--- METHODS ---------------------------------------------
public:
  CPropellerModel (void);
  virtual ~CPropellerModel (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);
  virtual CDependent *GetPart() {return 0;}
  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPropellerModel"; }
	virtual void        Reset();
  void                TimeSlice (float dT);
	
  /** Sets the Revolutions Per Minute for the propeller. Normally the propeller
      instance will calculate its own rotational velocity, given the Torque
      produced by the engine and integrating over time using the standard
      equation for rotational acceleration "a": a = Q/I , where Q is Torque and
      I is moment of inertia for the propeller.
      @param rpm the rotational velocity of the propeller */
  void SetRPM                     (const double &rpm)  {RPM = rpm;}
  void SetMinRPM                  (const double &mrpm) {minRPM = mrpm;}
	void SetBladePitch(float p)			{blad_pitch = eData->e_blad; }
  //---Functions that are specific to propeller instance -------------------
  virtual double GetPowerRequired (void)                    {return 0;}
  virtual double Calculate        (double PowerAvailable)   {return 0;}
  //---return all values ---------------------------------------------------
  int& GetBladeNum                       (void) {return blad;} 
  virtual float   GetAngularMomentum     (void) {return 0.0f;}
  virtual double  GetTorque              (void) {return 0.0f;}
  virtual const double& GetBladeAoA      (void) {return blade_AoA;}
  virtual const float&  GetThrustXDispl  (void) {return thrust_displ;}
  //------------------------------------------------------------------------
  inline float GetERPM()            {return eRPM;}
  //------------------------------------------------------------------------
protected:
  bool read_PRP_file_flag;
  char propeller_filename[PATH_MAX];
  //
  int mode, blad;
  float cord, diam, mnbd, mxbd, facT, facQ, iceR, idcf, pFac;

  // lift, drag, pirt, rift, dmge
  float lift, drag, pirt, rift, dmge;
  CFmtxMap *mlift;            // Lift table look up (fmt3)
  CFmtxMap *mdrag;            // drag table look up (fmt3)
  CFmtxMap *mpirt;            // prop table look up (fmt3)
  CFmtxMap *mrift;            // prop table look up (fmt3)
  CFmt1Map *mfacP;            // prop table look up (fmt3)
  CDamageModel  *pdmge;       // Damage model
  //----------------------------------------------------------------
  double Ixx;                 // Propeller inertia
  double GearRatio;           // Gear ratio
  double Pitch;               // Blade pitch
  double Advance;
  float  deltaT;
  double Thrust;
  double Torque;
  double PowerRequired;
  double RPM;
  double blade_AoA;
	double blad_pitch;
  double minRPM;
  bool   is_fixed_speed_prop;
  float  thrust_displ;        // displacement of thrust along the X vect (P fact)
  float  eRPM;
};

//=======================================================================
// CPropellerJSBSimModel // added
//
// Modified from JSBSim
//======================================================================

class CPropellerJSBSimModel : public CPropellerModel {
public:
  CPropellerJSBSimModel (CVehicleObject *v, int eNo);
  virtual ~CPropellerJSBSimModel (void);
  
  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPropellerJSBSimModel"; }
  /** Retrieves the power required (or "absorbed") by the propeller -
      i.e. the power required to keep spinning the propeller at the current
      velocity, air density,  and rotational rate. */
  virtual double GetPowerRequired(void);
  /** Calculates and returns the thrust produced by this propeller.
      Given the excess power available from the engine (in foot-pounds), the thrust is
      calculated, as well as the current RPM. The RPM is calculated by integrating
      the torque provided by the engine over what the propeller "absorbs"
      (essentially the "drag" of the propeller).
      @param PowerAvailable this is the excess power provided by the engine to
      accelerate the prop. It could be negative, dictating that the propeller
      would be slowed.
      @return the thrust in pounds */
  virtual double Calculate(double PowerAvailable);

  // getters
  virtual float  GetAngularMomentum     (void) {return 0.0f;}
  virtual double GetTorque              (void) {return 0;}

private:
  int    numBlades;
  double Ixx;
  double Diameter;
  double MaxPitch;
  double MinPitch;
  double MinRPM;
  double MaxRPM;
  double P_Factor;
  double Sense;
  double Pitch;
  double Advance;
  double ExcessTorque;
  double GearRatio;
  double ThrustCoeff;
  
  //  FGColumnVector3 vTorque;
  //  FGTable *cThrust;
  //  FGTable *cPower;
  //  void Debug(int from);
};
//==========================================================================
/*!
 * CPropellerTRIModel //  added
 * defined in the .PRP file
 * Modified from JSBSim
 */
//==========================================================================
class CPropellerTRIModel : public CPropellerModel {
public:
  CPropellerTRIModel                 (CVehicleObject *v,int eNo);
  virtual ~CPropellerTRIModel        (void);
  
  virtual void  ReadFinished         (void);
 
  virtual const char* GetClassName   (void) { return "CPropellerTRIModel"; }
  virtual double GetPowerRequired    (void);
  virtual double Calculate           (double PowerAvailable);

  // getters
  virtual float  GetAngularMomentum  (void);                      // pound ft2/s
  virtual double GetTorque           (void) {return Torque;}      // pound-force foot
  float          GetAngMomentumKgM2s (void);                      // Kg m2/s
	//--------------------------------------------------------------------
	void	 Reset();
  //--------------------------------------------------------------------
  void   Probe(CFuiCanva *cnv);
  //--------------------------------------------------------------------
  inline CDependent *GetPart() {return this;}
  //--------------------------------------------------------------------
private:
  double PwrAva;                    // Power available to display
  double MinRPM;
  double MaxRPM;
  double Sense;
  double ExcessTorque;
  double ThrustCoeff;
  double blade_speed;
  double TotalDrag;
  double TotalLift;
  double PfcK;
  double CQ;
  double CT;
  double magic_number;
  float  tmp_lift;
  float  tmp_drag;
};


/*! The piston class is adapted from Jon S. Berndt work:
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

 Author:       Jon S. Berndt
 Date started: 09/12/2000

 ------------- Copyright (C) 2000  Jon S. Berndt (jsb@hal-pc.org) --------------

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
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable:4512 )
#endif


#ifdef _MSC_VER
#pragma warning( pop )
#endif




//
// CWingModel
//
class CWingModel : public CSubsystem {
public:
  CWingModel (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CWingModel"; }
};
//===========================================================================
// *
// * Pitot-Static system objects defined in the .PSS file
// *

//
// Pitot-Static Port
//
//===========================================================================
class CPitotStaticPort : public CDependent {
  //--- ATTRIBUTEs ---------------------------------------------
protected:
  CVehicleObject *mveh;
  //--- METHODs ------------------------------------------------
public:
  CPitotStaticPort (CVehicleObject *v);

  // CStreamObject members
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPitotStaticPort"; }
  virtual EMessageResult  ReceiveMessage (SMessage *msg);
  virtual void            TimeSlice (float dT,U_INT FrNo = 0);		// JSDEV*

  const double& ComputePitotPort (const double &rho, const double &pr, const SVector &vt);

protected:
  EPortType type;   // PITOT or STATIC port type
  SVector   bPos;   // Location relative to aircraft center of gravity
  int     grup;     // Group number
  EPortFace face;   // LEFT or RIGHT facing
  bool    inie;     // Interior port?
  bool    wont;     // Port won't ice up
  int     stat;     // Port Enabled? (1=Yes|0=No) default=yes

  double val;
};


//=================================================================
//  Circular timer used for animating bitmap
//=================================================================
class CCircularTimer: public CDependent {
  //---Attribute ----------------------------------------
  U_INT modulo;
  float maxT;                           // Period
  float Time;                           // Current time
  U_INT beat;                           // Beat per second
  //------------------------------------------------------
public:
  CCircularTimer();
  virtual const   char* GetClassName (void) { return "CCircularTimer"; }
  virtual int     Read (SStream *stream, Tag tag);
  virtual void    TimeSlice(float dT,U_INT FrNo);
  EMessageResult  ReceiveMessage (SMessage *msg);
  //-------------------------------------------------------
  void  Probe(CFuiCanva *cnv);
};

//===================================================================
class CheckChapter;
struct D2R2_ACTION;

//==================================================================
// CDLLSubsystem
//==================================================================
class CDLLSubsystem : public CSubsystem {
public:
  bool enabled;
  void *dll;
  CDLLSubsystem (void);
  virtual ~CDLLSubsystem (void);
  // CStreamObject methods
	virtual int		Read (SStream *stream, Tag tag);
	virtual void	ReadFinished (void);
	//--------------------------------------------------------------
  // CSubsystem methods
  //--------------------------------------------------------------
  virtual	void  TimeSlice (float dT,U_INT FrNo);
  //-----------funct members -------------------------------------
  void SetObject (SDLLObject *object);
  void SetSignature (const long &sign);
  void Prepare (void);
  const SDLLObject* Get_Object (void) {return obj;}
  const long& GetSignature (void) {return signature;}
  //-----------attributes ----------------------------------------
private:
  SDLLObject *obj;
  long signature;
};
//====================END OF FILE ================================
#endif // SUBSYSTEMS_H
