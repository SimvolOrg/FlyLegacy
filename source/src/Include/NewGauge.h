/*
 * New Gauges.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005 Chris Wallace
 * Copyright since 2007 Jean Sabatier
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



#ifndef NEWGAUGE_H
#define NEWGAUGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//================================================================================
#include "../Include/Gauges.h"
#include "../Include/GaugeComponents.h"
//================================================================================
class C_HobbsMeterGauge;
//=======================================================================
//	Vertical needle
//=======================================================================
class CVertNeedle: public CGauge {
protected:
	//--- ATTRIBUTES ------------------------------
	CgQUAD		nedl;								// Quad and texture
	C_Cover   ovrl;								// Overlay
	float			p0;									// Minimum vertical value
	float			p1;									// Maximum vertical value
	float			ppv;								// pixel per value ratio for translation
	//---------------------------------------------
public:
	CVertNeedle(CPanel *mp);
	//---------------------------------------------
	void		CollectVBO(TC_VTAB *vtb);
	//---------------------------------------------
	int			Read(SStream *stream, Tag tag);
	void		ReadVRNG(SStream *stream);
	void		ReadFinished();
	//---------------------------------------------
	void		Draw();
	void		DrawAmbient();
};
//=======================================================================
// C1NeedleGauge
//  This is the simplest gauge with one message and one needle
//  It use the needle of the upper CTexturedGauge
//=======================================================================
class C1NeedleGauge: public CTexturedGauge {
  //--- ATTRIBUTES -----------------------------
  CGauge       *mgg;    // Optional mother gauge
  //--- METHODS --------------------------------
public:
  C1NeedleGauge(CPanel *mp);
	C1NeedleGauge();
	//---------------------------------------------
	void		CollectVBO(TC_VTAB *vtb);
  //--------------------------------------------
  int   Read(SStream *str,Tag tag);
  void  ReadFinished();
  void  SetMother(CGauge *mg);
  void  CopyFrom(SStream *s);
  void  CopyFrom(CGauge *mg,C1NeedleGauge &src);
	void	SetPanel(CPanel *mp);
	//---------------------------------------------
	void	SetMVEH(CVehicleObject *v)	{mveh = v;}
  //---------------------------------------------
  ECursorResult MouseMoved (int x, int y);
  //---------------------------------------------
  void  Draw();
	void	DrawAmbient();
};
//==============================================================================
// CHawkClockGauge
//
//==============================================================================
class CHawkClockGauge : public CGauge {
public:
  CHawkClockGauge (CPanel *mp);
  virtual ~CHawkClockGauge (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadTIME(SStream *stream);
  void  ReadUNDR(SStream *stream, int No);
  void  ReadFONT(SStream *stream);
  void  ReadFinished (void);
  void  CopyFrom(SStream *stream);
	//------------------------------------------------------
	void	SaveValue();
	void	LoadValue();
  //------------------------------------------------------
  // CGauge methods
  const char*   GetClassName (void) { return "CHawkClockGauge"; }
  void          Draw (void);
  ECursorResult MouseMoved (int x, int y);
  EClickResult  MouseClick (int x, int y, int buttons);
	void			    IntgFromDoat(U_INT tag);
  void          DisplayUPR();
  //-------------------------------------------------------------------
protected:
	//-------------------------------------------------------------------
	CgHolder				 *hold;								// Value holder
  //-------------------------------------------------------------------
  CGaugeClickArea   bupp;               // Upper button
  CGaugeClickArea   bsel;               // Select button
  CGaugeClickArea   bcon;               // Control button
  CGaugeClickArea   bbot;               // Bottom (both SEL and CON) buttons
  //-------------------------------------------------------------------
  CFont             *cFont;       // Clock font
  //-------------------------------------------------------------------
  char        wCar;               // Character width
  char        hCar;               // Character height
  char        tLn;                // Time line
  char        tCn;                // Time column
	// ------ String to edit time  --------------------------------------
  char				edNb[8];			// JSDEV*  edit value here
  char        btup;         // Upper button
  char        btsl;         // Selector
  char        btst;         // Timer start
  //-------------------------------------------------------------------
private :
  TEXT_DEFN           txd;      //  Texture
  U_INT               col;      //  color
  DOAT_TIME           und[4];   //  DOAT  underline
  //-------------------------------------------------------------------
};
//============================================================================
// CBasicDigitalClockGauge
//============================================================================
class CBasicDigitalClockGauge : public CHawkClockGauge {
public:
  CBasicDigitalClockGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CBasicDigitalClockGauge"; }

protected:
  int         tick_x, tick_y;
  int         tkut;
  int         tklt;
  int         tkft;
  int         tket;
};

//=======================================================================
// CAltimeterGauge
//=======================================================================
class C_AltimeterGauge : public CTexturedGauge {
public:
  C_AltimeterGauge (CPanel *mp);
  virtual ~C_AltimeterGauge (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished();
  //----------------------------------------------------------------
  void    CopyFrom(SStream *s);
	void		CollectVBO(TC_VTAB *vtb);
  // CGauge methods-------------------------------------------------
  virtual const char*   GetClassName (void) { return "C_AltimeterGauge"; }
  virtual      void		  PrepareMsg(CVehicleObject *veh);
  ECursorResult         MouseMoved (int x, int y);
  EClickResult          MouseClick (int x, int y, int buttons);
  EClickResult          StopClick();
  //----------------------------------------------------------------
  void			Draw (void);
  void      DisplayHelp();
  //----Attributes -------------------------------------------------
protected:
  VStrip          base;               // Underlay over baro      
  CNeedle        *ndl1;
  CNeedle        *ndl2;
  CNeedle        *ndl3;
  C_Knob          alkn;
  SMessage        kmsg;               // Baro message
  float           baro;
  //-----------------------------------------------------------------
};
//======================================================================
// JSDEV*  Simplified CDirectionalGyroGauge
//======================================================================
class C_DirectionalGyroGauge : public CTexturedGauge {
    //--------------------------------------------------------------
protected:
  CNeedle     apbg;   // Bug image
	C_Knob			knob;	  // Gyro rotating knob
	C_Knob			apkb;	  // Auto pilot rotating knob
	Tag				  bias;
	//--- message for GYRO gauge plate -----------------------------
	SMessage		mbug;						// bug message
  SMessage    mgyr;           // gyro message
	//--- Related subsystems ---------------------------------------
	CSubsystem *gyrS;
  //----Values for display----------------------------------------
  float       hdg;              // Gyro direction
  float       bug;              // Autopilot bug
  int         dir;              // Autopilot direction

public:
  virtual const char* GetClassName (void) { return "CDirectionalGyroGauge"; }
  C_DirectionalGyroGauge (CPanel *mp);
  virtual ~C_DirectionalGyroGauge (void);

  //---- CStreamObject methods -----------------------------------
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished ();
  // CGauge methods
  void		            PrepareMsg(CVehicleObject *veh);
  EClickResult        MouseClick (int x, int y, int buttons);
  EClickResult        StopClick ();
  ECursorResult       MouseMoved (int x, int y);
  //---- ---------------------------------------------------------
  void                CopyFrom(SStream *stream);
	void								CollectVBO(TC_VTAB *vtb);
  //--------------------------------------------------------------
  void                Draw ();
  //--------------------------------------------------------------
  ECursorResult       DisplayBUG();
  ECursorResult       DisplayHDG();
  void                DisplayHelp() {}
};
//======================================================================
// CAirspeedGauge
//======================================================================
class C_AirspeedGauge : public CTexturedGauge {
public:
  C_AirspeedGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
	//------------------------------------------------------
	void	CollectVBO(TC_VTAB *vtb);
  void  CopyFrom(SStream *s);
  //--- CGauge methods -----------------------------------
  virtual const char* GetClassName (void) { return "CAirspeedGauge"; }
  ECursorResult MouseMoved (int x, int y);
  EClickResult  MouseClick (int x, int y, int buttons);
  EClickResult  StopClick();
  //------------------------------------------------------
  void  Draw();
  //------------------------------------------------------
protected:
  C_Knob        knob;       // MMO knob
  CNeedle       nmmo;       // MMO needle
};
//======================================================================
// C_VerticalSpeedGauge
//======================================================================
class C_VerticalSpeedGauge : public CTexturedGauge {
public:
  C_VerticalSpeedGauge (CPanel *mp);
  virtual ~C_VerticalSpeedGauge (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  // CGauge methods
  virtual const char* GetClassName (void) { return "CVerticalSpeedGauge"; }
  virtual void PrepareMsg(CVehicleObject *veh);
	//-------------------------------------------------
	void	CollectVBO(TC_VTAB *vtb);
  //-------------------------------------------------
  ECursorResult MouseMoved (int x, int y);
  // CGauge methods
  void	Draw (void);
  //-------------------------------------------------
protected:
	C_Knob	        knob;
};
//======================================================================
// CHorizonGauge
//======================================================================
class C_HorizonGauge : public CTexturedGauge {
public:
  C_HorizonGauge                       (CPanel *mp);
  virtual ~C_HorizonGauge              (void);

  //---- CStreamObject methods --------------------------
  int                   Read          (SStream *stream, Tag tag);
  void                  ReadFinished();
	//-----------------------------------------------------
  void                  CopyFrom(SStream *stream);
	void									CollectVBO(TC_VTAB *vtb);
  //--- CGauge methods ----------------------------------
  virtual const char*   GetClassName  (void) { return "CHorizonGauge"; }
  void			PrepareMsg	  (CVehicleObject *veh);
  //----Mouse metheods ---------------------------------
  ECursorResult         MouseMoved    (int x, int y);
  EClickResult          MouseClick    (int x, int y, int buttons);
  EClickResult          StopClick     ();
	//-------------------------------------------------------------
  void									Draw();
	void									DrawAmbient();
protected:
	//-------------------------------------------------------------
  CNeedle	Bfoot;
	CNeedle	Bmire;
	CNeedle	Birim;
	CNeedle	Borim;
	//-------------------------------------------------------------
protected:
	int         hoff;
	double      pixd;
	int         mdeg;
	//--------------------------------------------------------------
	C_Knob      knob;
	SMessage    pich;
	SMessage    roll;
	//--- Related subsystems ---------------------------------------
	CSubsystem *pitS;
	CSubsystem *rolS;

};
//=======================================================================
// CSimpleSwitch
//	Derived class must supply the Draw function
//=======================================================================
class C_SimpleSwitch : public CTexturedGauge {
public:
  C_SimpleSwitch (CPanel *mp);
  virtual ~C_SimpleSwitch (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);
  virtual void  CollectVBO(TC_VTAB *vtb);
  // CGauge methods
  virtual void          PrepareMsg(CVehicleObject *veh);				// JSDEV* prepare gauge  messages
  virtual const char*   GetClassName (void) { return "CSimpleSwitch"; }
	virtual void          Draw() {;}
	virtual void					DrawAmbient();
  virtual ECursorResult MouseMoved (int x, int y);
  virtual EClickResult  MouseClick (int x, int y, int buttons);
	virtual EClickResult	StopClick();
  //-----------------------------------------------------
protected:
  void  DecState ();
  void  IncState ();
  void  ChangeState();
  //--- ATTRIBUTES -------------------------------------
protected:
  VStrip  swit;           // Bitmap sitch
  int     cIndx;          // Current index
  //-------- Cursor Tags -------------------------------
  Tag     csru_tag;     // Unique tag for switch up cursor
  Tag     csrd_tag;     // Unique tag for switch down cursor
  // States---------------------------------------------
  int     stat_n;     // Number of states
  int     *stat;      // Array of state values
  int     sstr_n;     // Number of state strings
  char    **sstr;     // Array of stat strings
  int     orie;       // Orientation
  int     ordr;       // Order
  bool    mmnt;       // Momentary contact
  bool    mntO;       // Momentary open
};
//=======================================================================
// CGenericSwitch
//=======================================================================
class CGenericSwitch: public CTexturedGauge {
  //--- Attributes ---------------------------------
  U_CHAR  nba;                    // Number of actions
  U_CHAR  nbm;                    // Number of messages
  short   nfr;                    // Frame number
	short   dfa;										// Default action
  //------------------------------------------------
  VStrip  bmap;                   // Bitmap art
  std::map<int,CGaugeClickArea *>  vkca;
  std::map<int,SMessage *>         vmsg;
  std::map<int,CGaugeAction*>      vact;
  //----METHODS ------------------------------------
public:
  CGenericSwitch(CPanel *mp);
 ~CGenericSwitch();
 //----- Initial call -------------------------------
 void	PrepareMsg(CVehicleObject *veh);
 //--------------------------------------------------  
 virtual void  CollectVBO(TC_VTAB *vtb);
 //--------------------------------------------------
 int    Read(SStream *s, Tag t);
 void   ReadFinished();
 void   AreaClicked(int n);
 void   Execute(BASE_ACT *ba);
 void   Draw() {;}
 void		DrawAmbient();
 //---Mouse management ------------------------------
 EClickResult  MouseClick (int x, int y, int buttons);
 ECursorResult MouseMoved (int x, int y);
};
//===========================================================================
// CPushPullKnobGauge
//===========================================================================
class C_PushPullKnobGauge : public CTexturedGauge {
public:
  C_PushPullKnobGauge (CPanel *mp);
 ~C_PushPullKnobGauge (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);

  // CGauge methods
  virtual const char*   GetClassName (void) { return "CPushPullKnob"; }
  virtual void PrepareMsg(CVehicleObject *veh);
	//----------------------------------------------------------------
	virtual void CollectVBO(TC_VTAB *vtb);
  //-----------------------------------------------------------------
  virtual ECursorResult MouseMoved (int x, int y);
  virtual ECursorResult DisplayRatio();
  virtual EClickResult  MouseClick (int x, int y, int bt);
  virtual EClickResult  StopClick  ();
  //----------------------------------------------------------------
  void    Err01();
  //----------------------------------------------------------------
  void    CreateArea(SStream *s);
  void    DefaultArea();
  //----------------------------------------------------------------
  bool    AtRatio(char r);
  //----------------------------------------------------------------
  void    LookUpValue(float v);
  //----------------------------------------------------------------
	virtual void  Draw (void) {;}
	virtual void	DrawAmbient();
	//----------------------------------------------------------------
	inline U_CHAR State()	{return cVal;}
  //----------------------------------------------------------------
protected:
  void DecValue (void);
  void IncValue (void);

protected:
  //------ ATTRIBUTES --------------------------------------
  VStrip      bmap;                   // Bitmap art
  SKnobArea  *Area;                   // Stack of values
  //--------------------------------------------------------
  SMessage    lnk1;
  SMessage    polm;
  //------ values ------------------------------------------
  int         nVal;       // Number of values
  int         cVal;       // Current value
  //--------------------------------------------------------
  char        invt;        // sort order
  float       lowv;
  float       high;
};
//===========================================================================
// JSDEV* simplified CNavigationGauge
//===========================================================================
class C_NavigationGauge : public CTexturedGauge {
	//---------------------------------------------------------
public:
  C_NavigationGauge (CPanel *mp);
  virtual ~C_NavigationGauge (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished ();
  void  DecodeFlag(SStream *str);
  void  DecodeTab(SStream *str, VStrip &p);
	//------------------------------------------------------------
  void  CopyFrom(SStream *s);
	void	CollectVBO(TC_VTAB *vtb);
  //--- CGauge methods -----------------------------------------
	virtual const char* GetClassName (void) { return "CNavigationGauge"; }
	virtual void  PrepareMsg(CVehicleObject *veh);
  void          GetRadio();
  ECursorResult ShowOBS();
  //------------------------------------------------------------
	EClickResult  MouseClick (int x, int y, int buttons);
	EClickResult  StopClick  ();
  ECursorResult MouseMoved (int x, int y);
  //------------------------------------------------------------
	void          Draw (void);
protected:
  //--- Needle is defined at CTextured gauge level -------------
  Tag       radi_tag;
  int       radi_unit;
  SMessage  mobs;             // OBS message set
  SMessage  mrad;             // Radio Message
  BUS_RADIO *radio;           // Radio data
	//--------Localizer needle-- -------------------------------
  float     vobs;             // OBS from radio
  float     lndK;             // Horizontal coefficient
  CNeedle	  lndl;             // Vertical needle
 	CNeedle	  gndl;             // Horizontal needle
	//--------Glide slope working area -------------------------
  U_CHAR    ils;
  U_CHAR    rflg;             // Radio Flag
  float     gldK;             // Glide coefficient
  //----------OBS management --------------------------------
	C_Knob	knob;						    // OBS KNOB
  TEXT_DEFN  Flag;            // Flag texture
	//----------FLAG bitmap ---------------------------------
  VStrip	gldF;				        // glide flag
	VStrip	locF;				        // Localizer flag
};
//==========================================================================
// CTachometerGauge
//===========================================================================
class C_TachometerGauge : public CTexturedGauge {
public:
  C_TachometerGauge (CPanel *mp);
  virtual ~C_TachometerGauge (void);
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);
	//--------------------------------------------------------------------
	void	CopyFrom(SStream *stream);
	void	CollectVBO(TC_VTAB *vtb);
  //---- CGauge methods ------------------------------------------------
  virtual const char* GetClassName (void) { return "CTachometerGauge"; }
  void		  Draw (void);
  //--------------------------------------------------------------------
protected:
  CNeedle            nedl;
  C_HobbsMeterGauge *hobs;
};
//===========================================================================
// JSDEV* Simplified CBasicADFGauge
//===========================================================================
class C_BasicADFGauge : public CTexturedGauge {
public:
  C_BasicADFGauge (CPanel *mp);
  virtual ~C_BasicADFGauge (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);
  //-------------------------------------------------
	void  CollectVBO(TC_VTAB *vtb);
  void  CopyFrom(SStream *s);
  //-------------------------------------------------
  // CGauge methods
  virtual const char* GetClassName (void) { return "CBasicAdfGauge"; }
  //-------------------------------------------------
  ECursorResult MouseMoved (int x, int y);
  EClickResult  MouseClick (int x, int y, int buttons);
  EClickResult  StopClick  ();
  void          Draw (void);
  void          ShowDIR();
  //--------------------------------------------------
protected:
  C_Knob         knob;
  CNeedle        comp;
  int            cpas;      // Compass card setting
  float          navd;

};
//======================================================================
// CHobbsMeterGauge
//======================================================================
class C_HobbsMeterGauge : public CTexturedGauge {
  //--- METHODS ---------------------------------------------
public:
  C_HobbsMeterGauge (CPanel *mp);
  virtual ~C_HobbsMeterGauge (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadDIGK(SStream *str, CDigit *dg);
  void  ReadFinished (void);
	//---  Collect for VBO --------------------------------------
	void  CollectVBO(TC_VTAB *vtb);
  //---  CGauge methods----------------------------------------
  virtual const char* GetClassName (void) { return "CHobbsMeterGauge"; }
  void		  Draw (void);
  //--- ATTRIBUTES -------------------------------------------
protected:
  float     val;
  char      wCar;                 // Char wide
  //-------Bitmap management ---------------------------------
  TEXT_DEFN idig;                 // integer texture
  TEXT_DEFN pdig;                 // partial texture
  CDigit    digt;                 // last digit
  //----------------------------------------------------------
  };
//=======================================================================
//  CBasicMagnetoSwitch
//=======================================================================
class C_BasicMagnetoSwitch : public C_SimpleSwitch {
public:
  C_BasicMagnetoSwitch (CPanel *mp);
  // CGauge methods
  virtual const char*   GetClassName (void) { return "CBasicMagnetoSwitch"; }
	//---------------------------------------------------------------
	void CollectVBO(TC_VTAB *vtb);
  //---------------------------------------------------------------
	void	PrepareMsg(CVehicleObject *veh);
	void	Draw();
  ECursorResult MouseMoved (int x, int y);
  EClickResult  MouseClick (int x, int y, int buttons);
  EClickResult  StopClick  ();
  //----Attributes ------------------------------------------------
protected:
};
//==================================================================
// CBasicBatterySwitch
//==================================================================
class C_BasicBatterySwitch : public C_SimpleSwitch {
public:
  C_BasicBatterySwitch (CPanel *mp);
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CGauge methods
  const char* GetClassName (void) { return "CBasicBatterySwitch"; }
	//-----------------------------------------------------
	void	CollectVBO(TC_VTAB *vtb);
  //-----------------------------------------------------
  void    PrepareMsg(CVehicleObject *veh);
	void    Draw (void) {;}
	void		DrawAmbient();
  EClickResult  MouseClick (int x, int y, int buttons);

protected:
  Tag				altt;
  Tag				batt;
  SMessage  mbat;
  SMessage  malt;
  char			sAlt;
  char			sBat;
	CSubsystem *batS;							// Battery 
	CSubsystem *altS;							// Alternator
};
//=======================================================================
// C2NeedleGauge
//========================================================================
class C2NeedleGauge : public CTexturedGauge {
protected:
  //--- Attributes ------------------------------------------
  char     type;                // Horizontal or vertical
  C1NeedleGauge ndl1;           // Needle 1
  C1NeedleGauge ndl2;           // Needle 2
  //--- Gauge marker ----------------------------------------
  CNeedleMark   mrk1;           // First marker
  CNeedleMark   mrk2;           // second marker
	//---------------------------------------------------------
	CgHolder		 *hold;
public:
  C2NeedleGauge (CPanel *mp);
  //--- CStreamObject methods -------------------------------
  int   Read (SStream *stream, Tag tag);
  void  CopyFrom(SStream *s);
	void	PrepareMsg(CVehicleObject *veh);
	//-----------------------------------------------------
	void	CollectVBO(TC_VTAB *vtb);
  //-----------------------------------------------------
  //---- CGauge methods -------------------------------------
  virtual const char* GetClassName (void) { return "C2NeedleGauge"; }
  //--- mouse management -------------------------------------
  ECursorResult MouseMoved(int x, int y);
  EClickResult  MouseClick(int x, int y, int btn);
  EClickResult  StopClick ();
  //----------------------------------------------------------
  void  Draw();
	void	DrawAmbient();
};
//======================================================================
// CAmmeterGauge
//======================================================================
class C_AmmeterGauge : public C1NeedleGauge {
  //--- METHODS ---------------------------------
public:
	C_AmmeterGauge (CPanel *mp): C1NeedleGauge(mp)  {;}
  // CStreamObject methods
  // CGauge methods
  virtual const char* GetClassName (void) { return "CAmmeterGauge"; }
	//----------------------------------------------
	void CollectVBO(TC_VTAB *vtb);
	//----------------------------------------------
  void  Draw();
	void	DrawAmbient();
  //--- No specific attributes ------------------
protected:
};
//=======================================================================
// CVacuumGauge
//=======================================================================
class C_VacuumGauge : public C1NeedleGauge {
  //--- METHODS ---------------------------------
public:
	C_VacuumGauge(CPanel *mp): C1NeedleGauge(mp) {;}
  // CGauge methods
  virtual const char* GetClassName (void) { return "CVacuumGauge"; }
	//---------------------------------------------
	void	CollectVBO(TC_VTAB *vtb);
	//---------------------------------------------
  void  Draw();
	void	DrawAmbient();
  //--- No specific attrbutes -------------------
protected:
};
//=====================================================================
// JSDEV*	CTurnCoordinatorGauge
//=====================================================================
class C_TurnCoordinatorGauge : public C1NeedleGauge {
public:
  C_TurnCoordinatorGauge (CPanel *mp);
  virtual ~C_TurnCoordinatorGauge (void);

  //--- CStreamObject methods --------------------
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  void  CopyFrom(SStream *s);
  // CGauge methods
  virtual const char* GetClassName (void) { return "CTurnCoordinatorGauge"; }
	//---------------------------------------------
	void	CollectVBO(TC_VTAB *vtb);
	//---------------------------------------------
	void			PrepareMsg(CVehicleObject *veh);
  void		  Draw (void);
  //----ATTRIBUTES ----------------------------
protected:
	Tag       pcon;
	Tag       bcon;
  float     tilt;             // Inclination ratio
  float     rbal;             // Ball ratio
  int       mbal;             // Middle frame
	//----Block for plane and ball -------------
	VStrip	ball;
	//---- Related subsystem -------------------
	CSubsystem *trnS;
};
//======================================================================
//  Gauge Push pull with repeat feature
//======================================================================
class CRepeatPushPull: public C_PushPullKnobGauge {
  //--- METHODS ---------------------------------------
public:
	CRepeatPushPull(CPanel *mp):C_PushPullKnobGauge(mp) {;}
	//---------------------------------------------
	void	CollectVBO(TC_VTAB *vtb);
  //--- Specific methods ------------------------------
  EClickResult  MouseClick (int x, int y, int bt);
  EClickResult  StopClick  ();
	void          Draw();
};
//======================================================================
//  Gauge FLAP
//======================================================================
class C_FlapsSwitchGauge : public C_PushPullKnobGauge {
public:
  C_FlapsSwitchGauge (CPanel *mp);

  //---- CStreamObject methods ------------------
  int   Read (SStream *stream, Tag tag);
	//---------------------------------------------
	void	CollectVBO(TC_VTAB *vtb);
	//---------------------------------------------
  // CGauge methods
  virtual const char* GetClassName (void) { return "CFlapsSwitchGauge"; }
};
//======================================================================
//--------Define positions ----------------------------
enum B_POS {	
  TEST	= 0,
	BRIT	= 1,
	DIMR	= 2,	};

//======================================================================
// JSDEV specific switch related to FlyHawk annunciator panel
//
// CFlyhawkAnnunciatorTest
//======================================================================
class C_FlyhawkAnnunciatorTest : public CTexturedGauge {
public:
  C_FlyhawkAnnunciatorTest (CPanel *mp);
  virtual ~C_FlyhawkAnnunciatorTest (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CFlyhawkAnnunciatorTest"; }
	//---------------------------------------------
	void	CollectVBO(TC_VTAB *vtb);
  //----------------------------------------------------------
	void          Draw (){;}
	void					DrawAmbient();
  ECursorResult MouseMoved (int x, int y);
  EClickResult  MouseClick (int x, int y, int buttons);
  EClickResult  StopClick  ();
	//--- This gauge method ------------------------------------
  void					InitMsg(SMessage *msg,Tag tag);
  void					UpdateComponent(char old,char now);
protected:
  void          DecState (void);
  void          IncState (void);
  //----ATTRIBUTES ------------------------------------------
protected:
  VStrip        butn;             // Bitmap switch
  //----- Cursors -------------------------------------------
  Tag     csru_tag;               // Unique tag for switch up cursor
  Tag     csrd_tag;               // Unique tag for switch down cursor
  //------------Message array ---------------------------
  SMessage msgT[3];				// Array of Messages
  //-----------------------------------------------------
  char IncT[4];					// Increment table
  char DecT[4];					// Decrement table
  char	  cPos;					// Current position
  char    oPos;					// Old position
};
//====================================================================
// CFlyhawkElevatorTrimGauge
//====================================================================
class CElevatorTrimGauge : public CTexturedGauge {
public:
  CElevatorTrimGauge (CPanel *mp);
  virtual ~CElevatorTrimGauge (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);
  //----------------------------------------------------------
	void	CollectVBO(TC_VTAB *vtb);
  //---  CGauge methods ---------------------------------------------
  const char*   GetClassName (void) { return "CFlyhawkElevatorTrimGauge"; }
  ECursorResult MouseMoved (int x, int y);
  EClickResult  MouseClick (int x, int y, int buttons);
  EClickResult  StopClick  ();
	void			    Draw () {;}
	void					DrawAmbient();
  //---  ATTRIBUTES -------------------------------------------------
protected:
  //--- Animated textures -------------------------------------------
  VStrip        wheel;                // wheel bitmap
  VStrip        indwh;                // Indicator bitmap
  //--- Click components --------------------------------------------
  CGaugeClickArea   down;
  CGaugeClickArea   caup;
// Repeat timer      
  //-----------------------------------------------------------------
  float             wRatio;
  float             iRatio;
  int               mIndwh;
};
//====================================================================
// JSDEV* CBasicCompassGauge
//====================================================================
class CBasicCompassGauge : public CBitmapGauge{
public:
  CBasicCompassGauge (CPanel *mp);
  virtual ~CBasicCompassGauge (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);
  // CGauge methods
  virtual const char*	GetClassName (void) { return "CBasicCompassGauge"; }
	//----------------------------------------------------------
	void	CollectVBO(TC_VTAB *vtb);
  //--------------------------------------------------
  void    Draw (void);
  //--------------------------------------------------
  void    DisplayHelp();
  //--------------------------------------------------
protected:
  int   cdir;                    // Direction
  int   wind;
  int   orgx;
  int   nton;
  float rDeg;								      // Ratio per degre
  int	  midP;								      // MidPoint offset
  CAniMAP comps;                  // Compass bitmap
};
//====================================================================
// JSDEV* CBasicCompassGauge
//====================================================================
class C_BasicCompassGauge : public CTexturedGauge{
public:
  C_BasicCompassGauge (CPanel *mp);
  virtual ~C_BasicCompassGauge (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);
  // CGauge methods
  virtual const char*	GetClassName (void) { return "CBasicCompassGauge"; }
	//----------------------------------------------------------
	void	CollectVBO(TC_VTAB *vtb);
  //--------------------------------------------------
  void    DrawDynamic(void);
  //--------------------------------------------------
  void    DisplayHelp();
  //--------------------------------------------------
protected:
  int cdir;                     // Direction
  //--- pixel coefficients ---------------------------
  float M;								        // MidPoint offset
  float A;                        // Pixels to North origin
  float B;                        // Pixel per degre
  VStrip comps;                   // Compass bitmap
};
//========================================================================
// CSimpleInOutStateSwitch 'sios'
//=========================================================================
class C_SimpleInOutStateSwitch : public CGauge {
  // --- METHODS -------------------------------------------
public:
  C_SimpleInOutStateSwitch (CPanel *mp);
  virtual ~C_SimpleInOutStateSwitch (void);
  // CStreamObject methods
  int     Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);
  virtual void  PrepareMsg(CVehicleObject *veh);
	//----------------------------------------------------------
	void	CollectVBO(TC_VTAB *vtb);
  //--------------------------------------------------
  // CGauge methods
  virtual const char* GetClassName (void) { return "CSimpleInOutStateSwitch"; }
	virtual void  Draw () {;}
	virtual void  DrawAmbient();
  virtual EClickResult  MouseClick (int x, int y, int buttons);
  //------------------------------------------------------------------
	void		SubsystemCall(CSubsystem *sys,int val);
	void		Draw(char p);
	//--- ATTRIBUTES ---------------------------------------------------
protected:
  VStrip    stsw;                           // Bitmap switch
  int       vin[2];                         // On/off intt
  //-------- Cursor Tags ---------------------------------------------
  Tag     csr[2];														// Tags for cursor up/dn
  //------------------------------------------------------------------
  char     gpos;                            // On/Off position
  char     rfu1;                            // reserved
  char     mmnt;                            // Momentary contact
  char     mack;                            // Must acknoledge
  //------------------------------------------------------------------
  char     onht[64];                       ///< ON help text
  char     ofht[64];                       ///< OFF help text
  int      time;
  // Sound effects
  char     sstr_off[64];                   ///< ON popup string value
  char     sstr_on[64];                    ///< OFF popup string value
};
//=============================================================================
// CNavGpsSwitchT0:  Switch Nav/GPS type T0
//=============================================================================
class CNavGpsSwitchT0 : public C_SimpleInOutStateSwitch {
	//--- ATTRIBUTES ----------------------------------------------
protected:
	Tag	usas;					// Gauge bus for state
	//-------------------------------------------------------------
public:
  CNavGpsSwitchT0 (CPanel *mp);

  // CStreamObject methods
	void ReadFinished();
  // CGauge methods
  virtual const char* GetClassName (void) { return "CNavGpsSwitchT0"; }
	//--------------------------------------------------------------------
	void	DrawAmbient();
protected:
};

//========================================================================
// strip Gauge:  Gauge using a vertical strip
//=========================================================================
class CStripGauge : public CTexturedGauge
{	
protected:
	//--- Attributes --------------------------------
	VStrip	bmap;									// Vertical bitmap
	//--- Methods -----------------------------------
public:
	CStripGauge(CPanel *mp);
	//-----------------------------------------------
	void	CollectVBO(TC_VTAB *vtb);
	//-----------------------------------------------
	int     Read (SStream *stream, Tag tag);
	void		Draw();
};
//=========================== END OF FILE ======================================
#endif NEWGAUGE_H
