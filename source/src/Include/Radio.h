//===================================================================================
// Radio.h
//
// Part of Fly! Legacy project
//
// Copyright 2003 Chris Wallace
// Copyright 2007 Jean Sabatier
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
//=====================================================================================


#ifndef RADIO_H
#define RADIO_H


#include "../Include/FlyLegacy.h"
//===================================================================================
class CFPlan;
class CAirplane;
//===================================================================================
// External radio source
//===================================================================================
class CExtSource: public CmHead {
protected:
	//-------------------------------------------------
	CVehicleObject *mveh;										// Mother vehicle
	//--- ATTRIBUTE -----------------------------------
	char			qAct;													// Index of active queue;													// qType
	char			active;												// Active indicator
	U_CHAR    signal;												// Signal type
	U_CHAR		rfu1;
	//-------------------------------------------------
	char      sidn[5];                      // ident = first char from name
  char      snam[64];                     // source name
	//-------------------------------------------------
	ILS_DATA *ilsD;													// optional ILS DATA
	//-------------------------------------------------
	SPosition spos;                         // position lat long alti
	float     smag;                         // Magnetic dev
  float	    radial;                       // Aircraft radial in °
	float	    nmiles;                       // Distance to VOR/NDB
  float			dsfeet;                       // Distance in feet
	float			vdev;													// Vertical deviation
	double    refD;													// Reference direction
	//--- METHODS -------------------------------------
public:
	CExtSource(): CmHead(ANY,OTH) {active = 0;}
	//--------------------------------------------------
	void			SetSource(CmHead *src,ILS_DATA *ils,U_INT frm);
	void			Refresh(U_INT frm);
	//--------------------------------------------------
	inline  void			SetVEH(CVehicleObject *v) {mveh = v;} 
	//---------------------------------------------------
	inline	void			Stop()				{	active = 0;}
	inline	U_CHAR		SignalType()	{ return signal;}
	//--------------------------------------------------
	inline	void			SetRefDirection(float d) {refD = d;}
	inline	void			SetPosition(SPosition *p)	{spos = *p;}
	inline	ILS_DATA  *GetLandSpot()		{return ilsD;}
	//--------------------------------------------------
	inline  float			GetFeetDistance()	{ return dsfeet;}
	inline	float     GetNmiles()				{ return nmiles; }
	inline  float     GetMagDev()				{ return smag; }
	inline  float			GetRadial()				{ return radial;}
	inline  double    GetRefDirection()	{ return refD;}
	inline  double    Sensibility()     { return 10;}
	inline  float			GetVrtDeviation()	{ return vdev;}
	inline  float     GetMagDirection()	{ return (radial - smag);}
	//--------------------------------------------------
	inline  bool			IsActive()		{ return (active != 0);}
};
//=====================================================================
// *
// * Radio subsystems
// *
//=====================================================================
//  Field structure for  Radio
//=====================================================================
typedef struct  { short  sPos;                  // Screen position (CA#)
                  short  state;                 // Field state 
                  char  *data;                  // Field content
} RADIO_FLD;
//-----------FREQUENCY definition -------------------------------------
typedef struct  { short whole;                  // Whole part
                  short fract;                  // Fractional part
                  float   freq;                 // Float format
} RADIO_FRQ;
//-----------DRIVER SLOT ----------------------------------------------
typedef struct  { U_CHAR No;                    // Click area
                  U_CHAR Ev;                    // Event
} RADIO_HIT;
//-----------FIELD DISPLAY -----------------------------------------------
typedef struct  { U_CHAR fn;                    // Field Number
                  U_CHAR sp;                    // Screen position
} RADIO_DSP;
//=====================================================================
// CRadio
//
// This subsytem is the base for all radio subsystems
//=====================================================================
class CRadio : public CDependent {
protected:
  //------------Define Frequency format----------------------
  typedef struct {  short whole;              // Whole part
                    short fracp;              // Fractional part
                    char  wText[8];           // Whole part edit
                    char  fText[4];           // Fractional edit
  } CHFREQ;
  //------------COMMON PARTS -----------------------------------
	U_INT				Frame;
  U_CHAR      sPower;                           // Power state
  BUS_RADIO   Radio;                            // Computed values
  //------------COM part ---------------------------------------
  U_CHAR      cState;                           // COM state
  RADIO_FLD   comTAB[K155_DCOM_SZ];             // Control field
  RADIO_FRQ   ActCom;                           // Active COM
  RADIO_FRQ   SbyCom;                           // Standby COM
  //-------------------------------------------------------------------
  short       comINC[3];                        // Com fractional increment
  //---------COM data -------------------------------------------------
  char        cDat1[4];                         // ACOM WP
  char        cDat2[4];                         // ACOM FP
  char        cDat3[4];                         // SCOM WP
  char        cDat4[4];                         // SCOM FP
  char        cDat5[4];                         // CHAN OP
  char        cDat6[4];                         // CHAN NB
  //------------NAV PART ---------------------------------------
  U_CHAR      nState;                           // NAV state
  RADIO_FLD   navTAB[K155_DNAV_SZ];             // Control fields
  RADIO_FRQ   ActNav;                           // Active NAV
  RADIO_FRQ   SbyNav;                           // Standby NAV
  //---------NAV data -------------------------------------------------
  char        nDat1[4];                         // ANAV WP
  char        nDat2[4];                         // ANAV FP
  char        nDat3[4];                         // SNAV WP
  char        nDat4[4];                         // SNAV FP
  char        nDat5[32];                        // CDI
  char        nDat6[4];                         // OBS
  char        nDat7[4];                         // TIM MN
  char        nDat8[4];                         // TIM SEC
  char        nDat9[4];                         // IND
  RADIO_FRQ   tempf;                            // Temporary frequency
  //-----------Flasher --------------------------------------------
  U_CHAR      mskFS;                            // Flasher mask
  //--------------NAVAID objects ------ -----------------------------
	CExtSource  EXT;															// External source
	CNavaid    *VOR;                              // VOR selected
  CILS       *ILS;                              // ILS selected
  CCOM       *COM;                              // COM selected
  U_SHORT     OBS;                              // OBS value
  //-----Mouse management ---------------------------------------------
  short    mDir;                                // Mouse direction
  //--------------------------------------------------------------
  float     sinc;         // Seek increment
  bool      test;         // Test mode active
  //-----------Public methods --------------------------------
public:
  CRadio (void);
 ~CRadio();
  void   ReadFinished();
	void	 FreeRadios(char opt);
  //--- virtual methods --------------------------------------
  int    virtual Dispatcher(U_INT evn) {return 1;}
	int    virtual PowerON()             {sPower = 1; return 1;}
	void   virtual Update(float dT, U_INT fr, char exs) {;}
  //----------------------------------------------------------
  bool  MsgForMe (SMessage *msg);
  void  TimeSlice (float dT,U_INT FrNo);
	void	Synchronize();
  void  Probe(CFuiCanva *cnv);
  virtual float Frequency()  {return 0;}
	//--- Enter /leave waypoint mode ---------------------------
	void	ModeEXT(CmHead *src,ILS_DATA *ils = 0);	// Enter/leave external mode
	void	ChangeRefDirection(float d);
	void	ChangePosition(SPosition *p);
  //-----------------------------------------------------------
  void  MakeFrequency(RADIO_FRQ *loc);
  void  StoreFreq(RADIO_FRQ *loc, float fq);
  void  InitTable(RADIO_FLD *tab,short No,char *data, short cf);
  void  InhibitFields(RADIO_FLD *tab,short dim);
  void  SetField(RADIO_FLD *tab,short No,char *msk,U_SHORT val);
  void  SetField(RADIO_FLD *tab,short No,char *msk,char *txt);
  void  SetField(RADIO_FLD *tab,short No,char state,char k);
  void  SetField(RADIO_FLD *tab,short No,char state,char *s);
  void  SetField(RADIO_FLD *tab,short No,char k);
  void  RazField(RADIO_FLD *tab,short No);
  void  OffField(RADIO_FLD *tab,short No);
  //----------------------------------------------------------
  void  ModifyWholeNav(short No,RADIO_FRQ *loc,short inc);
  void  ModifyFractNav(short No,RADIO_FRQ *loc,short inc);
  void  SwapNav(U_CHAR opt);
  void  SetGlide(SMessage *msg);
  void  ModifyWholeCom(short No,RADIO_FRQ *loc,short inc);
  void  ModifyFractCom(short No,RADIO_FRQ *loc,short inc);
  void  SwapCom();
  void  TuneTo(float frq,U_CHAR opt);
  void  TuneNavTo(float freq,U_CHAR opt);
  void  TuneComTo(float freq,U_CHAR opt);
  int   IncXOBS(short inc);
  //-------------------------------------------------------------
  void  StoreFreq(short(wp),short(fp),CHFREQ &fq);
  //-------------------------------------------------------------
	inline U_CHAR GetPowerState()		{return sPower;}
  inline U_CHAR GetNavState()     {return nState;}
  inline U_CHAR GetComState()     {return cState;}
  inline U_CHAR GetFlashMask()    {return mskFS;}
  inline void   SetDirection(int d)   {mDir = d;}
  inline int    GetDirection()        {return mDir;}
	inline float  GetMilesTo()			{return Radio.mdis;}
	//-------------------------------------------------------------
	inline double GetDeviation()		{return Radio.hDEV;}
	//-------------------------------------------------------------
	inline BUS_RADIO *GetBUS()			{return &Radio;}
  //-------------------------------------------------------------
  // CStreamObject methods
  virtual int       Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CRadio"; }
  virtual EMessageResult  ReceiveMessage (SMessage *msg);
};


//==================================================================
// JSDEV* CNavRadio 
//==================================================================
class CNavRadio : public CRadio {
    //------RADIO ATTRIBUTES --------------------------------------
protected:
  float  stby;
  static RADIO_HIT  *RnavHIT[];               // NAV click table

public:
  CNavRadio (void);
  virtual ~CNavRadio (void);
  // CStreamObject methods
  virtual int       Read (SStream *stream, Tag tag);
  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CNavRadio"; }
  virtual EMessageResult  ReceiveMessage (SMessage *msg);
  virtual void            Update (float dT,U_INT FrNo,char exs);
  //---------RADIO METHODS ---------------------------------
  void  Dispatcher(U_CHAR evn);
  void  PowerStatus();
  //---------------------------------------------------------
  inline RADIO_HIT *GetHIT()              {return RnavHIT[nState];}
  inline RADIO_FLD *GetField(U_CHAR No)   {return &navTAB[No];}
};

//===================================================================
// CRnavRadio
//===================================================================
class CRnavRadio : public CRadio {
public:
  CRnavRadio (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CRnavRadio"; }
//  virtual EMessageResult  ReceiveMessage (SMessage *msg);

protected:
};
//==================================================================
//
// CComRadio
//==================================================================
class CComRadio : public CRadio {
  //-----ATTRIBUTE TABLE ----------------------------------------
  static RADIO_HIT  *RcomHIT[];               // NAV click table
  float  stby;
public:
  CComRadio (void);
 ~CComRadio (void);
 //-------------------------------------------------------------
  void    Dispatcher(U_CHAR evn);
  void    PowerStatus();
  void    Update (float dT,U_INT FrNo, char exs);
  //------------------------------------------------------------
  inline RADIO_HIT *GetHIT()              {return RcomHIT[cState];}
  inline RADIO_FLD *GetField(U_CHAR No)   {return &comTAB[No];}
  //------------------------------------------------------------
  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CComRadio"; }
  virtual EMessageResult  ReceiveMessage (SMessage *msg);

};
//=================================================================
// CHFCommRadio
//=================================================================
class CHFCommRadio : public CComRadio {
public:
  CHFCommRadio (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CHFCommRadio"; }
//  virtual EMessageResult  ReceiveMessage (SMessage *msg);

protected:
};

//
// CTransponderRadio
//
typedef enum {
  XPDR_OFF        = 0,
  XPDR_STARTUPA1  = 1,
  XPDR_STARTUPA2  = 2,
  XPDR_STANDBY    = 3,
  XPDR_TEST       = 4,
  XPDR_ON         = 5,
  XPDR_ALT        = 6,
  XPDR_DIM        = 7,
  XPDR_END        = 0x80,
} ETransponderMode;

typedef enum {
  XPDR_SWITCH_OFF     = 0,
  XPDR_SWITCH_STANDBY = 3,
  XPDR_SWITCH_TEST    = 4,
  XPDR_SWITCH_ON      = 5,
  XPDR_SWITCH_ALT     = 6
} ETransponderSwitchMode;
//===============================================================================
//  Generic transponder radio subsystem
//================================================================================
class CTransponderRadio : public CRadio {
  friend class CGenericTransponderGauge;
  //--- Attribute --------------------------------------------------------
  U_CHAR            State;      // Transponder state
  RADIO_FLD         Fields[9];  // Field 0 not used
  int               lspd;       // Minumum safe operating speed in kias
  int               uspd;       // Maximum safe operating speed in kias
  SMessage          mAlt;       // Altimeter message
  char              airc[16];   // Aircraft tail number
  static RADIO_HIT *XdtHIT[];   // Hit areas
  static U_CHAR     XpdFWD[];   // Forward mode
  static U_CHAR     XpdBKW[];   // Backward mode
  static U_CHAR     XpdBTM[];   // Button state
  char              aIDN[6];    // Active  ident
  char              sIDN[6];    // Standby ident
  //--------------------------------------------------------------------
  U_INT             aCode;      // Active code
  U_INT             sCode;      // Standby code
  //--------------------------------------------------------------------
public:
  CTransponderRadio (void);

  // CStreamObject methods
  virtual int       Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTransponderRadio"; }
  virtual EMessageResult  ReceiveMessage (SMessage *msg);
  virtual void            Update (float dT,U_INT FrNo,char exs);				// JSDEV*
  //---------------------------------------------------------------------
  void        Dispatcher(U_CHAR evn);
  void        ModActiveDigit(int xd);
  void        ModStandbyDigit(int xd);
  void        ChangeMode();
  //---------------------------------------------------------------------
  inline U_CHAR     GetXpdState()         {return State;}
  inline RADIO_HIT *GetHIT()              {return XdtHIT[State];}
  inline RADIO_FLD *GetField(U_CHAR No)   {return &Fields[No];}
  inline float      GetFrame()            {return XpdBTM[State];}
  //---------------------------------------------------------------------
protected:
};

//=====================================================================
//
// CBKKT76Radio
//=====================================================================
class CBKKT76Radio : public CTransponderRadio {
public:
  CBKKT76Radio (void);

  // CStreamObject methods
  virtual int       Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CBKKT76Radio"; }
  virtual EMessageResult  ReceiveMessage (SMessage *msg);
  virtual void            TimeSlice (float dT,U_INT FrNo = 0);		//JSDEV*

protected:
  char              tempCode[5];
  int               entryDigit;
  bool              entryMode;
  float             entryTimer;
};
//==========================================================================
//	Tracking state 
//==========================================================================
#define GPSR_PWOF		(0)						// Power OFF
#define GPSR_STBY   (1)						// Standby
#define GPSR_TRAK		(2)						// Tracking mode
#define GPSR_LAND		(3)						// Landing
#define GPSR_NONE   (4)						// NO MODE
//==========================================================================
//	Tracking Event 
//==========================================================================
#define GPSR_EV_PWR	(0)						// Power Event
#define GPSR_EV_NAV	(1)						// Nav Switch 
#define GPSR_EV_APR	(2)						// APR Switch 

//==========================================================================
//   GPS Radio
//		GaugeBus:
//			IN01 => State
//			IN02 => Active
//			IN03 => NOT USED
//			IN04 => NAV/GPS 
//	NOTE:  	gpsON means that the GPS is driving the Autopilot
//==========================================================================
class GPSRadio :  public CDependent {
	//--- ATTRIBUTES -----------------------------------------
protected:
	//--- Switch ----------------------------------------------
	char							gpsTK;							// GPS tracking state
	char							navON;							// Switch position
	char							aprON;							// Switch position
	//---------------------------------------------------------
	CRadio            *RAD;								// Radio n°1
	AutoPilot         *APL;								// Auto pilote
	BUS_RADIO         *BUS;               // Master radio
	//--- Tracking parameters ---------------------------------
	CWPoint					  *wTRK;							// Tracked WayPoint
	//---------------------------------------------------------
	SMessage					mSpd;
	SMessage					mFMS;
	//--- Flight plan management ------------------------------
	CAirplane        *pln;								// Airplane
  CFPlan           *FPL;                // Flight plan
	CWPoint					 *basWP;							// Base waypoint
	CWPoint          *actWP;							// Active waypoint
	//--- METHODS --------------------------------------------
public:
  GPSRadio ();
 ~GPSRadio();
	//--- CSubsystem methods ---------------------------------
  const char* GetClassName (void) { return "GPSRadio"; }
	//--- CStreamObject methods ------------------------------
  int   Read (SStream *stream, Tag tag);
	//--------------------------------------------------------
  virtual void  ReadFinished (void);
	virtual void  TimeSlice(float dT,U_INT FrNo)	{;}
	//--- Interface with real GPS ----------------------------
	//	Real GPS must supplies those functions ---------------
	virtual void			ModifiedPlan()	{;}
	virtual void			TrackWaypoint(CWPoint *wpt,bool e) {;}
	virtual void			UpdNavigationData(CWPoint *w) {;}
	virtual CWPoint  *StartingNode()		{return 0;}
	virtual void			NavIsActive()	{;}
	//--- Tracking management --------------------------------
	void	TrackGPSEvent(U_CHAR evn, char prm);
	void	PowerEVN (char parm);
	void	SwitchNAV(char parm);
	void	SwitchAPR(char parm);
	//--------------------------------------------------------
	float	SelectDirection();
	void	UpdateTracking(float dT,U_INT frm);
	void	Refresh();
	//--------------------------------------------------------
	void	PowerON();
	void	EnterTRK();
	void	NextNODE();
	void	EnterAPR();
	void	EnterSBY();
	//--------------------------------------------------------
	void	GPSRadio::Probe(CFuiCanva *cnv);
	//--- Message interface ----------------------------------
  EMessageResult  ReceiveMessage (SMessage *msg);
	//--- Mode state -----------------------------------------
	inline int  GaugeBusIN04()	{return navON;}
	inline int	GaugeBusIN05()	{return aprON;}

};
//==========END OF THIS FILE ==================================================
#endif // RADIO_H
