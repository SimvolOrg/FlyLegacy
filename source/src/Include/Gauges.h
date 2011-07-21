/*
 * Gauges.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
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

/*! \file Gauges.h
 *  \brief Defines CGauge and all derived panel gauge classes
 */


#ifndef GAUGES_H
#define GAUGES_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Include/FlyLegacy.h"
#include "../Include/Utility.h"
#include "../Include/TimeManager.h" // added on October the 24th, 2005 fot TimeManager
                         // in CHawkClockGauge
#include "../Include/database.h"
#include "../Include/Gps150.h"
#include "../Include/subsystems.h"
#include "../Include/AudioManager.h"
#include "../Include/BendixKing.h"
#include "../Include/GaugeComponents.h"
//====================================================================================
#define HELP_SIZE (128)
//====================================================================================
class CPanelLight;
//====================================================================================
//
// Macro definitions for all valid gauge types.  These correspond to the gauge
//   type in each <gage> object within a PNL file.
//====================================================================================
#define GAUGE_KEY                       'skey'    // Not a true gauge. Just to strike a key
#define GAUGE_GRIP                      'grip'  
#define GAUGE_BASE                      'gage'
#define GAUGE_BITMAP                    'bgge'
#define GAUGE_NEEDLE                    'need'
#define GAUGE_NEEDLE_DUAL               '2ned'
#define GAUGE_NEEDLE_TRIPLE             '3ned'
#define GAUGE_DIGITAL_NEEDLE            'dned'
#define GAUGE_ALTIMETER                 'alti'
#define GAUGE_ALTIMETER_ROLLING         'altr'
#define GAUGE_HORIZON                   'hori'
#define GAUGE_HORIZON_FD                'fdhz'
#define GAUGE_HORIZONTAL_BALL           'hbal'
#define GAUGE_AIRSPEED                  'aspd'
#define GAUGE_VERTICAL_SPEED            'vspd'
#define GAUGE_DIRECTIONAL_GYRO          'gyro'
#define GAUGE_VACUUM                    'vacu'
#define GAUGE_SUCTION                   'suct'
#define GAUGE_FUEL                      'fuel'
#define GAUGE_FUEL_FLOW                 'fflw'
#define GAUGE_OIL_PRESSURE              'oilp'
#define GAUGE_OIL_TEMPERATURE           'oilt'
#define GAUGE_TACHOMETER                'tach'
#define GAUGE_TACHOMETER_DUAL           'dtch'
#define GAUGE_N1_TACHOMETER             'n1ng'
#define GAUGE_EXHAUST_GAS_TEMP          'egtg'
#define GAUGE_BASIC_COMPASS             'comp'
#define GAUGE_AMMETER                   'amms'
#define GAUGE_ANALOG_CLOCK              'aclk'
#define GAUGE_DIGITAL_CLOCK_FLYHAWK     'cdcg'
#define GAUGE_DIGITAL_CLOCK_BASIC       'bdcg'
#define GAUGE_ADF_BASIC                 'adfg'
#define GAUGE_DIGITAL_READOUT           'dgrg'
#define GAUGE_DIGITS_READOUT            'drog'
#define GAUGE_CABIN_PRESSURE_RATE       'cprk'
#define GAUGE_CABIN_PRESSURE            'cprs'
#define GAUGE_BRAKE_PRESSURE             'brpg'
#define GAUGE_AOA                       'aoan'
#define GAUGE_TURN_COORDINATOR          'turn'
#define GAUGE_SLIP_INDICATOR            'slip'
#define GAUGE_HOBBS_METER               'hobb'
#define GAUGE_NAVIGATION                'navg'
#define GAUGE_AUTOPILOT                 'atop'
#define GAUGE_NAV_COMM_KX155            'k155'
#define GAUGE_GPS_KLN89                 'kl89'
#define GAUGE_XPDR_KT76                 'kt76'
#define GAUGE_ADF_KR87                  'kr87'
#define GAUGE_AUTOPILOT_KAP140          'ka14'
#define GAUGE_AUTOPILOT_KFC200          'kf20'
#define GAUGE_AUTOPILOT_KFC150          'kf15'
#define GAUGE_AUDIO_KMA26               'kma2'
#define GAUGE_HSI                       'hsi_'
#define GAUGE_FLYHAWK_ELT               'selt'
#define GAUGE_NAV_RADIO_GENERIC         'navr'
#define GAUGE_COMM_RADIO_GENERIC        'comr'
#define GAUGE_TRANSPONDER_GENERIC       'xpdr'
#define GAUGE_ADF_GENERIC               'adf_'
#define GAUGE_RMI_NEEDLE                'rmig'
#define GAUGE_NAV_RADIO_GROUP           'ngrp'
#define GAUGE_COM_RADIO_GROUP           'cgrp'
#define GAUGE_NAV_GAUGE_GROUP           'ogrp'
#define GAUGE_TRANSPONDER_RADIO_GROUP   'tgrp'
#define GAUGE_ADF_RADIO_GROUP           'agrp'
#define GAUGE_RADIO_GAUGE_GROUP         'radg'
#define GAUGE_ANNUNCIATOR               'annc'
#define GAUGE_FLYHAWK_ANNUNCIATOR       'shan'
#define GAUGE_FLYHAWK_ANNUNCIATOR_TEST  'shts'
#define GAUGE_FLYHAWK_NAV_GPS_PANEL     'sngp'
#define GAUGE_FLYHAWK_FUEL_SELECTOR     'shfs'
#define GAUGE_ELEVATOR_TRIM             'shet'
#define GAUGE_MALIBU_CHT_INDICATOR      'mcht'
#define GAUGE_MALIBU_CHT_BUTTON         'mchb'
#define GAUGE_MALIBU_NAV_GPS_PANEL      'mngp'
#define GAUGE_MALIBU_DIGITAL_CLOCK      'mdcg'
#define GAUGE_MALIBU_FUEL_SELECT        'mfss'
#define GAUGE_MALIBU_ENGINE_MONITOR     'pmem'
#define GAUGE_NAVAJO_FUEL_SELECTOR      'njfs'
#define GAUGE_NAVAJO_AMMETER            'namp'
#define GAUGE_NAVAJO_RADAR_ALTIMETER    'nral'
#define GAUGE_COLLINS_EFIS_SELECTION    'cesp'
#define GAUGE_COLLINS_VSPEED_PANEL      'cvsp'
#define GAUGE_COLLINS_HF_RADIO          'khfr'
#define GAUGE_COLLINS_PFD               'cpfd'
#define GAUGE_COLLINS_ND                'cnvd'
#define GAUGE_COLLINS_RTU               'crtu'
#define GAUGE_COLLINS_AD                'cadd'
#define GAUGE_COLLINS_MND               'cmnd'
#define GAUGE_UNIVERSAL_FMS             'ufms'
#define GAUGE_PILATUS_EFIS_PANEL        'pefs'
#define GAUGE_WALKER_TEMP_AIRSPEED      'hsat'
#define GAUGE_GYRO_SLAVING              'gslv'
#define GAUGE_VERTICAL_RATE             'vert'
#define GAUGE_PROP_SYNC                 'sync'
#define GAUGE_DME                       'dmei'
#define GAUGE_DME_2                     'dme2'
#define GAUGE_WEATHER_RADAR             'wrad'
#define GAUGE_ANALOG_ALT_PRESELECT      'apre'
#define GAUGE_TCAS                      'tcas'
#define GAUGE_GARMIN_GPS150             'g150'
#define GAUGE_GARMIN_GNS430             'g430'

#define GAUGE_HOTSPOT                   'hspt'
#define GAUGE_SIMPLE_IN_OUT             'sios'
#define GAUGE_LIT_IN_OUT                'lios'
#define GAUGE_SIMPLE_SWITCH             'swit'
#define GAUGE_BATTERY_SWITCH            'batt'
#define GAUGE_TWO_STATE_ELT             'elt_'
#define GAUGE_SWITCH_PAIR               'pair'
#define GAUGE_SIMPLE_FLOAT_SWITCH       'fswi'
#define GAUGE_DUAL_SWITCH               'dswi'
#define GAUGE_PUSH_PULL_KNOB            'knob'
#define GAUGE_PRIMER_KNOB               'prim'
#define GAUGE_FLAP                      'flap'
#define GAUGE_LIT_LANDING_GEAR_KNOB     'llgr'
#define GAUGE_MIXTURE_KNOB              'mixt'
#define GAUGE_REVERSIBLE_THROTTLE_KNOB  'revt'
#define GAUGE_DUAL_KNOB                 'dual'
#define GAUGE_DOUBLE_LIGHT_KNOB         'shlk'
#define GAUGE_TURN_KNOB                 'tknb'
#define GAUGE_MAGNETO_SWITCH            'magn'
#define GAUGE_COVERED_SWITCH            'cswi'
#define GAUGE_COVERED_LIGHT_BUTTON      'cbtn'
#define GAUGE_VERTICAL_SLIDER           'vsld'
#define GAUGE_TRIM_WHEEL                'twhl'
#define GAUGE_TRIM_WHEEL_INDICATOR      'twhi'
#define GAUGE_INDICATOR                 'indi'
#define GAUGE_FLAPS_INDICATOR           'find'
#define GAUGE_PUSHBUTTON                'push'
#define GAUGE_LIGHT_BUTTON              'lbut'
#define GAUGE_TURN_SWITCH               'tswi'
#define GAUGE_MASTER_CAUTION_BUTTON     'mcwb'
#define GAUGE_KINGAIR_FIRE_EXT          'kafe'
#define GAUGE_WALKER_AP_PANEL           'happ'
#define GAUGE_WALKER_FIRE_PANEL         'rhfp'
#define GAUGE_LIGHT_SWITCH_SET          'lssg'
#define GAUGE_ROCKER_SWITCH             'rock'
#define GAUGE_GENERIC_SWITCH            'sgen'
#define GAUGE_VERTICAL_NEEDLE						'vrtn'
#define GAUGE_STRIP											'strp'
//
// Attic.  The following gauge identifiers are listed in the Fly! II class
//   list documentation, but no known examples exist.  They are unimplemented
//   at the present time.
//
#define GAUGE_ATTITUDE_INDICATOR        'atti'
#define GAUGE_GHOST                     'ghst'
#define GAUGE_NAV_RADIO                 'nrdg'
#define GAUGE_NAV_RADIO_KN53            'kn53'
#define GAUGE_NAV_RADIO_KN80            'kn80'
#define GAUGE_COMM_RADIO                'comm'
#define GAUGE_COMM_RADIO_KY96           'ky96'
#define GAUGE_TRANSPONDER_KT79          'kt79'
#define GAUGE_AUTOPILOT                 'atop'



// For convenience in some gauge definitions, this struct represents
//   a click area within a knob.  The ytop and ybtm are the upper and
//   lower pixel row offsets within the gauge corresponding to the click
//   area.  The help value is the popup help string, and the value field
//   indicates the knob value when this click area is activated.
typedef struct {
  int     ytop;       // Top pixel
  int     ybtm;       // Bottom pixel
  float   valu;       // related value
  int     fram;       // related frame
  char    help[64];   // Help message
} SKnobArea;
//===============================================================================
// CGaugeNeedle
//
// Helper class for many types of gauges which use analog needles for data display
//================================================================================
class CGaugeNeedle : public CStreamObject {
public:
  CGaugeNeedle (void);
  virtual ~CGaugeNeedle (void);

  // CStream methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);
  void  DecodeName(char *nn);
  // CGaugeNeedle methods
  void  Draw (SSurface *surface, float degrees, int cx, int cy);
  void  LoadExtBitmap (const char *bitmap_name);
  void  EditName(char *edt);
public:
  Tag   tag;

protected:
  short sizx;   // X size
  short sizy;   // Y size
  short offx;   // X offset
  short offy;   // Y offset
  short hw;     // Half wide
  short hh;     // half height
  float mind;   // Minimum degrees
  float maxd;   // Maximum degrees
  float rang;   // Range in degre
  char  base[64]; // Base filename of PBG bitmap

  CBitmap *bmap;    // Needle animation bitmap
  int   nFrames;    // Number of frames in the bitmap
  // Temporary kluge to prevent ReadFinished() from being called twice

private:
};


//================================================================================
//  Class CKnob for rotating knob
//================================================================================
#define PN_DEGRE 0x00     // Integer rotation 0-360
#define PN_CLAMP 0x01     // Clamp type
#define PN_ROTAT 0x02     // Rotate
//--------------------------------------------------------------------------------
class CKnob : public CStreamObject {
private:
  //---Attributes ----------------------------------------
	U_CHAR	CState;		      // Internal state
  U_CHAR  CType;          // Clamp or degre
  U_CHAR  ownS;           // owned surface
  U_CHAR  rfu1;           // Reserved
  //-- Position ------------------------------------------
  short	  x1;			        // X offset
	short   y1;			        // Y offset
	short	  x2;			        // X2
	short	  y2;			        // Y2
  short   cx;             // Center for needle
  short   cy;             // Center for needle
  //---Mother gauge ---------------------------------------
  CGauge *mg;             // Mother gauge
  //-------------------------------------------------------
  Tag   cursTag;          // Unique tag for custom cursor
  char  help[64];         // Custom popup help
  //-------------------------------------------------------
	short			nFrame;	        // Number of frame
  short     cFrame;         // Current frame
  short     aFrame;         // Frame increment
	CBitmap	   *bmap;		    // Bitmap
	SSurface   *ksurf;		  // Knob surface
	float		Timer;		      // timer
	float		maxTM;		      // max time before accelaration
  //-----Floating value knob -------------------------------
  float   Incr;           // Increment
  float   Change;         // Changing increment
  TC_VTAB  quad[4];
  //-----Methods -------------------------------------------
public:
  CKnob();                // default Constructor
  CKnob(CGauge *mg);      // For new operator
  virtual  ~CKnob();      // Destructor
  virtual int		Read(SStream *stream, Tag tag);
  virtual void  ReadFinished(void);
  //---------------------------------------------------------
  void  Init(CGauge *g);
  void  Share(CGauge *gage);
  void  AssignNeedle(char *fn);
  void  ReadParameters(CGauge *mg,SStream *st);
  bool  HasChanged();
  int   Redraw();
  int   ReDraw(int frame);
  int   DrawAsNeedle();
  //-----Floating values management -------------------------
  void  SetRange(float v1,float v2,float vi);
  void  ArmRotation(float adj,int button);
  //------------------------------------------------------
  inline bool   IsHit(int x,int y)  {return ((x >= x1) && (x <= x2) && (y >= y1) && (y <= y2)); }
  inline bool   NoKnob()            {return ((x1 + x2) == 0);}
  inline float  GetChange()         {return Change;}
  inline  int   GetFrame()          {return cFrame;}
  inline  int   GetModulo()         {return nFrame;}
  inline bool   Update()            {return HasChanged();}
  inline void   SetFrame(short f)   {cFrame = f;}
  //--------------------------------------------------------
  inline void       DisarmKnob()      {CState = 0; Change = 0;}
  inline TC_VTAB   *GetVTAB()         {return quad;}
  inline SSurface  *GetSurface()      {return ksurf;}
  //--------------------------------------------------------
  inline void SetGauge(CGauge *g)       {mg = g;}
  //-------Mouse management -------------------------------
  ECursorResult     MouseMoved (int x, int y);
};
//=================================================================================
//  CVariator
//  Provides a continious update in [0, 1] based on timer
//=================================================================================
class CVariator {
  //---Attribute --------------------------------------------
public:
  SMessage msg;                             // Message to send
protected:
  U_CHAR state;                             // Internal state
  float Tmax;                               // TIC value
  float Time;                               // Timer
  float Incr;                               // Increment
  float vmin;                               // Minimum value
  //---------------------------------------------------------
public:
  CVariator();
  //---------------------------------------------------------
  void  Init();
  void  Arm(float inc);                     // Arm the system
  bool  HasChanged();                       // Check change
  void  SendValue(float inc);               // send value to system
  //----------------------------------------------------------
  inline void     Disarm()          {state = 0;}
  inline void     SetTime(float t)  {Tmax = t;}
  inline void     SetVMIN(float m)  {vmin = m;}
};
//=================================================================================
// CGaugeKnob
//=================================================================================

class CGaugeKnob : public CGaugeClickArea {
public:
	//---Type of value -----------------------------------
	#define CLAMP 0x00				// Range value
	#define DEGRE 0x01				// Modulo
	//----------------------------------------------------
  CGaugeKnob (char type = DEGRE,float rev = 1);
  virtual ~CGaugeKnob (void);

  // CStreamObject methods
  virtual	int		Read (SStream *stream, Tag tag);
  virtual	void	ReadFinished(void);
			void	SetBitmap(SStream *str);
  // CGaugeKnob methods
  virtual	void	Draw (void);
  virtual	void	SetOffset (int offset_x, int offset_y);
			void	SetRange(float lo,float hi,float act);
			void	ArmRotation(float adj,int button);
			bool	HasChanged(void);
			int		GetFrame(void);
	//----Disarm rotation --------------------------------
	inline void		Disarm(void) {CState = 0;}
	inline float	GetActual()  {return Actual; }
	inline int		GetNbFrame(void) {return bmKnob->NumBitmapFrames();}
	//----------------------------------------------------
public:
	CBitmap *bmKnob;
	int offset_x, offset_y;   // Offset within owner gauge
	SSurface *knob_surf;
	//----Rotation parameters ----------------------------
	char	CState;					// Internal state
	char	CType;					// Type of value
	int		NbFrame;				// Number f frames
	float	Timer;					// Timer
	float	maxTM;					// Maximum time before acceleration
	float	Adjus;					// Adjustment
	float	Actual;					// Actual value
	float	LoVal;					// clamp Lower value
	float	HiVal;					// clamp highest value
	float   VaRev;					// increment value for one knob revolution
};

//================================================================================
//  Class for animated bitmap
//================================================================================
class CAniMAP {
  //---------Attributes ---------------------------------
  short   x;			    // X offset
	short   y;			    // Y offset
	U_SHORT	NbFrame;	  // Number of frames
  U_SHORT hiFrame;    //  Highest frame
	CBitmap *bmap;			// Bitmap pointeur
  //-----------------------------------------------------
public:
  CAniMAP();          // Constructor
 ~CAniMAP();          // Destructor
  void	Read(SStream *str,int nbp = 0);
  void  Fill(char *name);
  int   ClampFrame(int fn);
  int   DecodeBitmap(char *str,char *name);
  //-----------------------------------------------------
  CBitmap *GetBitmap()              {return bmap;}
  void  Draw(SSurface *sf)                              {bmap->DrawBitmap(sf,x,y,0);}
  void  Draw(SSurface *sf,int fr)                       {bmap->DrawBitmap(sf,x,y,fr);}
  void  Draw(SSurface *sf,int u,int v,int fr)           {bmap->DrawBitmap(sf,u,v,fr);}
  void  Draw(SSurface *sf,int pos,int nbl,int u,int v)  {bmap->DrawFrom(sf,pos,nbl,u,v); }
  void  GetBitmapSize(int *x,int *y)  {bmap->GetBitmapSize(x,y);}
  int   GetX()                      {return x;}
  int   GetY()                      {return y;}
  int   GetNbFrame()                {return NbFrame;}
  int   GetHiFrame()                {return hiFrame;}
  void  SetX(int v)                 {x = v;}
  void  SetY(int v)                 {y = v;}
};
//=================================================================================
// CGauge is the abstract parent class for all gauges.
//
//================================================================================
/*! \todo Currently this class is not abstract, but the Draw() method should
 *   be changed to pure virtual once all of the subclasses have their own
 *   Draw() methods defined.
 */
//------------------------------------------------------------------------------
class CVehicleObject;								// JSDEV* forward declaration
typedef enum {
  GAUGE_OFF_POS = 0,
  GAUGE_ON__POS = 1,
} EGaugePos;

//==================================================================
// CDLLGauge
//==================================================================
class CDLLGauge : public CGauge {
public:
  bool enabled;
  void *dll;
  CDLLGauge (void);
  virtual ~CDLLGauge (void);
  // CStreamObject methods
	virtual int		Read (SStream *stream, Tag tag);
	virtual void	ReadFinished (void);
	//--------------------------------------------------------------
  // CGauge methods
  //--------------------------------------------------------------
  virtual void Draw (void);
  //-----------funct members -------------------------------------
  void SetObject (SDLLObject *object);
  void SetSignature (const long &sign);
  void Prepare (void);
  const SDLLObject* Get_Object (void) {return obj;}
  const long& GetSignature (void) {return signature;}
  //-----------attributes ----------------------------------------
private:
  SSurface *dllgsurf;
  SDLLObject *obj;
  long signature;
};

//==================================================================
// CBitmapGauge
//==================================================================
class CBitmapGauge : public CGauge {
public:
  CBitmapGauge (CPanel *mp);
  virtual ~CBitmapGauge (void);

  // CStreamObject methods
  virtual const char* GetClassName (void) { return "CBitmapGauge"; }
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);

  // CGauge methods
  virtual void  Draw (void);

  // CBitmapGauge methods
  virtual void  DrawUnderlay (void);
  virtual void  DrawOverlay (void);
  //-----------------------------------------------------------
protected:
  //----------bitmap management --------------------------------
  CAniMAP under;            // Bitmap underlay
  CAniMAP overl;            // Bipmap Overlay
};


//==========================================================================
// CNeedleGauge
//==========================================================================
class CNeedleGauge : public CBitmapGauge {
public:
  CNeedleGauge                        (CPanel *mp);
	CNeedleGauge(): CBitmapGauge(0) {CNeedleGauge(0);}
  virtual ~CNeedleGauge               (void) {}

  // CStreamObject methods
  virtual const char* GetClassName    (void) { return "CNeedleGauge"; }
  virtual int         Read            (SStream *stream, Tag tag);
  virtual void        Draw            (void);
  virtual void        DrawNDL(SSurface *sf,int deg) {nedl.Draw(sf,float(deg),cx,cy);}
  //--------------------------------------------------------------------
  void                DrawOnThisSurface(SSurface *sf);             // Draw on shared surface
  //--------------------------------------------------------------------
  inline bool   Defined()   {return (def == 1);}
  inline void   Define()    {def = 1;}
  //--------------------------------------------------------------------
  // CNeedleGauge methods
  CGaugeNeedle*       GetGaugeNeedle() {return &nedl;}   // for some gauges with needles ex : altimeter};
  //----------METHODS -------------------------------------------------
protected:
  bool          def;                                                   // Needle is defined
  int           radi;                                                  // Radius in pixels
  float         sang;                                                  // Starting angle (degrees)
  float         dunt;                                                  // Degrees per unit
  float         rang_min;                                              // Minimum range (degrees)
  float         rang_max;                                              // Maximum range (degrees)
  int           noff_x;                                                // Needle center offset
  int           noff_y;
  CGaugeNeedle  nedl;
};


//=======================================================================
// CTwoNeedleGauge
//========================================================================
class CTwoNeedleGauge : public CBitmapGauge {
public:
  CTwoNeedleGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CTwoNeedleGauge"; }
  virtual void PrepareMsg(CVehicleObject *veh);
  virtual void Draw();
  //---------------------------------------------------------
  void DrawNDL1(SSurface *sf,int deg){ndl1.DrawNDL(sf,deg);}
  void DrawNDL2(SSurface *sf,int deg){ndl2.DrawNDL(sf,deg);}
  //------------ATTRIBUTES ----------------------------------
protected:
  CNeedleGauge  ndl1;
  CNeedleGauge  ndl2;
};


//=======================================================================
// CThreeNeedleGauge
//=======================================================================
class CThreeNeedleGauge : public CBitmapGauge {
public:
  CThreeNeedleGauge (CPanel *mp);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void	PrepareMsg(CVehicleObject *veh);
  // CGauge methods
  virtual const char* GetClassName (void) { return "CThreeNeedleGauge"; }

protected:
  CNeedleGauge  ndl1;
  CNeedleGauge  ndl2;
  CNeedleGauge  ndl3;
};


//========================================================================
// CDigitalNeedleGauge
//=========================================================================
class CDigitalNeedleGauge : public CNeedleGauge {
public:
  CDigitalNeedleGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CDigitalNeedleGauge"; }

protected:
  CGaugeNeedle  nndl;
  CAniMAP       abak;
  SMessage      powr;
};


//=======================================================================
// CAltimeterGauge
//=======================================================================
class CAltimeterGauge : public CBitmapGauge {
public:
  CAltimeterGauge (CPanel *mp);
  virtual ~CAltimeterGauge (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  // CGauge methods-------------------------------------------------
  virtual const char*   GetClassName (void) { return "CAltimeterGauge"; }
  virtual      void		PrepareMsg(CVehicleObject *veh);
  ECursorResult         MouseMoved (int x, int y);
  EClickResult          MouseClick (int x, int y, int buttons);
  EClickResult          StopClick();
  //----------------------------------------------------------------
  void			Draw (void);
  void      DisplayHelp();
  //----Attributes -------------------------------------------------
protected: 
  CNeedleGauge    *ndl1;
  CNeedleGauge    *ndl2;
  CNeedleGauge    *ndl3;
  CGaugeClickArea koll;
  CKnob           alkn;
  SMessage        kmsg;               // Baro message
  float           baro;
  //-----------------------------------------------------------------
};


//==================================================================
// CRollingAltimeterGauge
//==================================================================
class CRollingAltimeterGauge : public CBitmapGauge {
public:
  CRollingAltimeterGauge (CPanel *mp);
 ~CRollingAltimeterGauge();
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CGauge methods
  virtual const char* GetClassName (void) { return "CRollingAltimeterGauge"; }
  void          PrepareMsg(CVehicleObject *veh);
  ECursorResult MouseMoved (int x, int y);
  EClickResult  MouseClick (int x, int y, int buttons);
  EClickResult  StopClick();
  //----------------------------------------------------------
  void  Draw();
  int   UpThousand(float hd);
  int   DnThousand(float hd);
  //----------Attributes -------------------------------------
protected:
  CAniMAP     zdig;                 // Zero digit
  CAniMAP     ndig;                 // Numbers
  CAniMAP     tdig;                 // 10 thousand digits
  CAniMAP     wkol;                 // Kollman window
  CKnob       aknb;                 // Kollsman knob
  //----------Digits    --------------------------------------
  int   lg1;                        // Ten thousands
  int   lg2;                        // Thousands
  int   lg3;                        // Hundred
  //----------------------------------------------------------
  int       psiz_x, psiz_y;
  int       digx[5];
  int       digy;
  int       dsiz_x, dsiz_y;
  float     prev;                   // previous altitude
  float     press;                  // Pressure
  //------Pressure digits -------------------------------------
  int       base;                   // Base line for windows
  int       d1;
  int       d2;
  int       d3;
  int       d4;
  CNeedleGauge  ndl1;
  CGaugeClickArea koll;
  SMessage    kmsg;                 // Baro message
};


//----------------------------------------------------------------------
// JSDEV* modified CHorizonGauge
//----------------------------------------------------------------------
class CHorizonGauge : public CBitmapGauge {
public:
  CHorizonGauge                       (CPanel *mp);
  virtual ~CHorizonGauge              (void);

  // CStreamObject methods
  int                   Read          (SStream *stream, Tag tag);
  void                  ReadFinished();
//  virtual void          ReadFinished  (void);

  // CGauge methods
  virtual const char*   GetClassName  (void) { return "CHorizonGauge"; }
  void			PrepareMsg	  (CVehicleObject *veh);
  ECursorResult         MouseMoved    (int x, int y);
  EClickResult          MouseClick    (int x, int y, int buttons);
  EClickResult          StopClick     ();
  void			Draw(void);
protected:
	//-------------------------------------------------------------
  CAniMAP	Bfoot;
	CAniMAP	Bhorz;
	CAniMAP	Birim;
	CAniMAP	Borim;
	CAniMAP	Bfron;
	//-------------------------------------------------------------
protected:
	char        flag_art[64];
	int         hoff;
	float       pixd;
	int         moff;
	//--------------------------------------------------------------
  int         hdeg;                             // level of horizon
	CKnob       hknob;
	SMessage    pich;
	SMessage    roll;

};


//==========================================================================
// CFDHorizonGauge
//=========================================================================
class CFDHorizonGauge : public CHorizonGauge {
public:
  CFDHorizonGauge (CPanel *mp);

  // CStreamObject methods
  virtual const char* GetClassName (void) { return "CFDHorizonGauge"; }
  int   Read (SStream *stream, Tag tag);

protected:
  CAniMAP     abar;
};


//=========================================================================
// CHorizontalBallGauge
//=========================================================================
class CHorizontalBallGauge : public CBitmapGauge {
public:
  CHorizontalBallGauge (CPanel *mp);
  virtual ~CHorizontalBallGauge (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);

  // CGauge methods
  virtual const char*   GetClassName (void) { return "CHorizontalBallGauge"; }
  ECursorResult MouseMoved (int x, int y);
  EClickResult  MouseClick (int x, int y, int buttons);
  void			    Draw (void);
  void		      Update (void);

protected:
};


//======================================================================
// CAirspeedGauge
//======================================================================
class CAirspeedGauge : public CNeedleGauge {
public:
  CAirspeedGauge (CPanel *mp);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CAirspeedGauge"; }
  virtual ECursorResult MouseMoved (int x, int y);

protected:
  CGaugeKnob    knob;   // Kollsman knob
  CNeedleGauge  mmo_;   // MMO indicator
};


//-------------------------------------------------------------------------
// CVerticalSpeedGauge
//-------------------------------------------------------------------------
class CVerticalSpeedGauge : public CNeedleGauge {
public:
  CVerticalSpeedGauge (CPanel *mp);
  virtual ~CVerticalSpeedGauge (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CVerticalSpeedGauge"; }
  virtual void PrepareMsg(CVehicleObject *veh);
  virtual ECursorResult MouseMoved (int x, int y);

  // CGauge methods
  virtual void Draw (void);

protected:
  CAniMAP     vsbg;
	CGaugeKnob	vskb;
	SMessage		vmsg;

};


//---------------------------------------------------------------------------
// JSDEV*  Simplified CDirectionalGyroGauge
//---------------------------------------------------------------------------
class CDirectionalGyroGauge : public CNeedleGauge {
public:
  CDirectionalGyroGauge (CPanel *mp);
  virtual ~CDirectionalGyroGauge (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished ();

  // CGauge methods
  virtual const char* GetClassName (void) { return "CDirectionalGyroGauge"; }
  void		            PrepareMsg(CVehicleObject *veh);
  EClickResult        MouseClick (int x, int y, int buttons);
  EClickResult        StopClick ();
  ECursorResult       MouseMoved (int x, int y);
  void                Draw (void);
  //--------------------------------------------------------------
  void    DisplayBUG();
  void    DisplayHDG();
  void    DisplayHelp() {}
  //--------------------------------------------------------------
protected:
  CAniMAP   apbg;           // Bug Bitmpa
	CKnob			gykn;						// Gyro rotating knob
	CKnob			pikn;						// Auto pilot rotating knob
	Tag				bias;
	//--- message for GYRO gauge plate -----------------------------
	SMessage		mbug;						// bug message
  SMessage    mgyr;           // gyro message
  //----Values for display----------------------------------------
  int       hdg;            // Gyro direction
  int       bug;            // Autopilot bug
  int       dev;            // Gyro deviation
  int       dir;            // Autopilot direction
};


//=======================================================================
// CVacuumGauge
//=======================================================================
class CVacuumGauge : public CNeedleGauge {
public:
  CVacuumGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CVacuumGauge"; }
  virtual void        Draw (void);

protected:
};


//
// CSuctionGauge
//
class CSuctionGauge : public CNeedleGauge {
public:
  CSuctionGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CSuctionGauge"; }

protected:
};


//
// CFuelGauge
//
class CFuelGauge : public CNeedleGauge {
public:
  CFuelGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CFuelGauge"; }
  virtual void        Draw (void);
protected:
};


//
// CFuelFlowGauge
//
class CFuelFlowGauge : public CNeedleGauge {
public:
  CFuelFlowGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CFuelFlowGauge"; }

protected:
};


//============================================================================
// COilPressureGauge
//============================================================================
class COilPressureGauge : public CNeedleGauge {
public:
  COilPressureGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  // CGauge methods
  const char* GetClassName (void) { return "COilPressureGauge"; }

protected:
};


//===========================================================================
// COilTemperatureGauge
//===========================================================================
class COilTemperatureGauge : public CNeedleGauge {
public:
  COilTemperatureGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "COilTemperatureGauge"; }
//  virtual void Draw (void);

protected:
};


//===============================================================================
// CDualTachometerGauge
//===============================================================================
class CDualTachometerGauge : public CTwoNeedleGauge {
public:
  CDualTachometerGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  Draw();
  // CGauge methods
  const char* GetClassName (void) { return "CDualTachometerGauge"; }

protected:
};


//===============================================================================
// CN1TachometerGauge
//
class CN1TachometerGauge : public CTwoNeedleGauge {
public:
  CN1TachometerGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual ECursorResult MouseMoved (int x, int y);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CN1TachometerGauge"; }

protected:
  CGaugeKnob  knob;
};


//=============================================================================
// CExhaustGasTemperatureGauge
//=============================================================================
class CExhaustGasTemperatureGauge : public CNeedleGauge {
public:
  CExhaustGasTemperatureGauge (CPanel *mp);
  virtual ~CExhaustGasTemperatureGauge (void);
  virtual  const char* GetClassName (void) { return "CExhaustGasTemperatureGauge"; }

  // CStreamObject methods
  int     Read (SStream *stream, Tag tag);
  void    ReadFinished (void);
  // CGauge methods
  ECursorResult MouseMoved	(int x, int y);
  EClickResult  MouseClick (int x, int y, int buttons);
  EClickResult  StopClick();
  void	Draw ();
  //--- ATTRIBUTES ---------------------------------------------
protected:
  CKnob knob;
  char  egtn_art[64];
  CGaugeNeedle  pnedl;
  int redf;
};


//======================================================================
// CAnalogClockGauge
//======================================================================
class CAnalogClockGauge : public CTwoNeedleGauge {
public:
  CAnalogClockGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CAnalogClockGauge"; }
  //-------------------------------------------------------------------
  void Draw();
protected:
};


//======================================================================
// CAmmeterGauge
//======================================================================
class CAmmeterGauge : public CNeedleGauge {
public:
  CAmmeterGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CAmmeterGauge"; }

protected:
};
//==============================================================================
//  Structure for DOAT CLOCK in HAWK
//==============================================================================
struct  DOAT_TIME {
  int x0,x1,y;
  //--------------------------------------------------
  DOAT_TIME::DOAT_TIME()
  { x0 = x1 = y = 0; }
  inline void DOAT_TIME::Draw(SSurface *s, U_INT col)
  { DrawHLine(s, x0, x1, y, col); }
  inline void DOAT_TIME::Set(int xa, int xb, int ln)
  { x0  = xa;
    x1  = xb;
    y   = ln;
  }
  inline void DOAT_TIME::CopyFrom(DOAT_TIME &src)
  { x0 = src.x0;
    x1 = src.x1;
    y  = src.y;
  }
};



//----------------------------------------------------------------------
// JSDEV* Simplified CBasicADFGauge
//----------------------------------------------------------------------
class CBasicADFGauge : public CNeedleGauge {
public:
  CBasicADFGauge (CPanel *mp);
  virtual ~CBasicADFGauge ();

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CBasicAdfGauge"; }
  ECursorResult MouseMoved (int x, int y);
  EClickResult  MouseClick (int x, int y, int buttons);
  EClickResult  StopClick();
  void          Draw (void);
  //--------------------------------------------------
  void DisplayCMP();
protected:
  Tag           comp_tag;
  CGaugeNeedle  *comp;
  CKnob          aknb;
  CGaugeNeedle  *hdng;
  int            cpas;      // Compass card setting
  float          navd;

};
//===========================================================================
// CDigitalReadoutGauge
//============================================================================
class CDigitalReadoutGauge : public CBitmapGauge {
  //---ATTRIBUTES -------------------------------------
protected:
  int   maxd;
  int   digY;
  int   nDig;
  int   digX[8];
  bool  whol;     // Whole numbers only
  bool  padz;     // Pad with leading zeroes
  //-----------------------------------------------------------------------
  CAniMAP wdig;                     // Whole digit bitmap
  CAniMAP fdig;                     // Fractional digit bitmap
  //-----------METHODS-----------------------------------------------------
public:
  CDigitalReadoutGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CGauge methods
  virtual const char* GetClassName (void) { return "CDigitalReadoutGauge"; }
  //------------------------------------------------------------------------
  void  Draw();
};


//=================================================================================
// CDigitsReadoutGauge
//=================================================================================
class CDigitsReadoutGauge : public CGauge {
  //----ATTRIBUTES ----------------------------------
  protected:
  int   frgb_r, frgb_g, frgb_b;
  char  just[64];
public:
  CDigitsReadoutGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CDigitsReadoutGauge"; }
  virtual void  Draw (void) {}

};


//
// CCabinPressureRateKnobGauge
//
class CCabinPressureRateKnobGauge : public CBitmapGauge {
public:
  CCabinPressureRateKnobGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CCabinPressureRateKnobGauge"; }
  virtual ECursorResult MouseMoved (int x, int y);

protected:
  int     ofxy_x, ofxy_y;
  float   alti_min, alti_max;   // Altitude range
  float   rate_min, rate_max;   // Rate range
  CGaugeKnob  knob;         // Rate knob
  CGaugeKnob  cabn;         // Cabin Pressure knob
  char    onsf[64];
  char    ofsf[64];
};


//
// CCabinPressureGauge
//
class CCabinPressureGauge : public CNeedleGauge {
public:
  CCabinPressureGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CCabinPressureGauge"; }

protected:
  CNeedleGauge  alti;
  CNeedleGauge  diff;
  CNeedleGauge  rate;
};


//
// CBrakePressureGauge
//
class CBrakePressureGauge : public CTwoNeedleGauge {
public:
  CBrakePressureGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual ECursorResult MouseMoved (int x, int y);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CBrakePressureGauge"; }

protected:
  CGaugeClickArea   left;   // Left help area
  CGaugeClickArea   rght;   // Right help area
};


//
// CAOANeedleGauge
//
class CAOANeedleGauge : public CTwoNeedleGauge {
public:
  CAOANeedleGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual ECursorResult MouseMoved (int x, int y);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CAOANeedleGauge"; }

protected:
  CGaugeKnob      knob;
};


//----------------------------------------------------------
// JSDEV*	CTurnCoordinatorGauge
//--------------------------------------------------------
class CTurnCoordinatorGauge : public CBitmapGauge {
public:
			CTurnCoordinatorGauge (CPanel *mp);
  virtual ~CTurnCoordinatorGauge (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  // CGauge methods
  virtual const char* GetClassName (void) { return "CTurnCoordinatorGauge"; }
  virtual void		  Draw (void);
protected:
	Tag     pcon;
	Tag     bcon;
	int     Ltrn;
	int     Rtrn;
	float   pHead;
	//----Block for plane and ball ----------
	float	ratio;
  CAniMAP	plan;
	CAniMAP	ball;
	//---------------------------------------
};


//=============================================================
// CHSIGauge
//=============================================================
class CHSIGauge : public CBitmapGauge {
protected:
  CNavigation    *navs;       // subsystem connected
  CAniMAP         acmp;       // Compass art
  CAniMAP         acrs;       // Course art
  CAniMAP         adev;       // Deviation art
  CAniMAP         advb;       // Dev B art
  CAniMAP         apbg;       // Autopilot heading bug
  CAniMAP         tofg;       // TO flag
  CAniMAP         fmfg;       // FROM flag
  CAniMAP         glid;       // Glide art
  CKnob	          obkn;				// OBS KNOB
  CKnob           apkb;       // Autopilot heading knob
  SMessage        obsm;       // OBS message
  SMessage        mbug;       // Autopilot heading message
  SMessage        fcs;        // FCS message
  //-----------------------------------------------------------
  int             comp;       // Compass direction
  int             adir;       // Autopilot direction
  int             odir;       // OBS direction
  //-----------------------------------------------------------
  Tag         radi_tag;
  int         radi_unit;
  float       rang_min, rang_max;
  float       gsdf;       // Glideslope deflection
  //-----------------------------------------------------------
  BUS_RADIO       radio;      // Radio values
  float PixDev;               // Half Bar wide in pixel
  //-----------------------------------------------------------
public:
  CHSIGauge(CPanel *mp);
 ~CHSIGauge(void);
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();

  // CGauge methods
  virtual const char*	GetClassName (void) { return "CHSIGauge"; }
  virtual void			PrepareMsg(CVehicleObject *veh);
  virtual ECursorResult MouseMoved (int x, int y);
  virtual EClickResult  MouseClick (int x, int y, int buttons);
  virtual EClickResult  StopClick  ();
  //-------------------------------------------------------
  void    DisplayOBS(int d);
  void    DisplayAPB(int d);
  void    Draw();
  //-------------------------------------------------------
};

//=================================================================
// CFlyhawkELTGauge
//==================================================================
class CFlyhawkELTGauge : public CBitmapGauge {
public:
  CFlyhawkELTGauge (CPanel *mp);
  virtual ~CFlyhawkELTGauge (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);

  // CGauge methods
  virtual const char*   GetClassName (void) { return "CFlyhawkELTGauge"; }
  virtual void          Draw (void);
  virtual ECursorResult MouseMoved (int x, int y);
  virtual EClickResult  MouseClick (int x, int y, int buttons);
  virtual EClickResult  StopClick();
  //---------------------------------------------------------
protected:
  void  ChangeState(U_CHAR npo);
protected:
  //------Bitmap management --------------------------------
  CAniMAP     ulit;         // Unlighted
  CAniMAP     litd;         // Lighted

  //---cursor tag ------------------------------------------
  Tag         csru_tag;     // Unique tag for switch up cursor
  Tag         csrd_tag;     // Unique tag for switch down cursor

  // States-------------------------------------------------
  U_CHAR     incT[3];       // Increment value
  U_CHAR     decT[3];       // Decrement value
  U_CHAR     pos;           // Switch position
  U_CHAR     lit;           // ON-OFF status
};


//======================================================================
// CSlipIndicatorGauge
//======================================================================
class CSlipIndicatorGauge : public CBitmapGauge {
public:
  CSlipIndicatorGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CSlipIndicatorGauge"; }

protected:
  char    ball_art[64];
};


//======================================================================
// CHobbsMeterGauge
//======================================================================
class CHobbsMeterGauge : public CGauge {
public:
  CHobbsMeterGauge (CPanel *mp);
  virtual ~CHobbsMeterGauge (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CHobbsMeterGauge"; }
  virtual void		  Draw (void);

protected:
  //-------Bitmap management ---------------------------------
  CAniMAP   black;                            // Black art
  CAniMAP   white;                            // White art
  //----------------------------------------------------------

private:
  int blak_sizeX, blak_sizeY;
  int whit_sizeX, whit_sizeY;
  int one_whit_sizeY;

  int tmp_hobbs_value;
  int n_hobbs_value;
  int hobbs_value_1;
  int hobbs_value_2;
  int hobbs_value_3;
  int hobbs_value_4;
  float hobbs_value_5;
};

//==========================================================================
// CTachometerGauge
//===========================================================================
class CTachometerGauge : public CNeedleGauge {
public:
  CTachometerGauge (CPanel *mp);
  virtual ~CTachometerGauge (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CTachometerGauge"; }
  void		  Draw (void);
  //--------------------------------------------------------------------
protected:
  bool    hobb;   // Integrated Hobbs meter
  CHobbsMeterGauge *hobs;
};


//--------------------------------------------------------------
// JSDEV* simplified CNavigationGauge
//--------------------------------------------------------------
class CNavigationGauge : public CBitmapGauge {
	//---------------------------------------------------------
public:
  CNavigationGauge (CPanel *mp);
  virtual ~CNavigationGauge (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished ();

  // CGauge methods
	virtual const char* GetClassName (void) { return "CNavigationGauge"; }
	virtual void  PrepareMsg(CVehicleObject *veh);
  void          GetRadio();
  void          DisplayHelp();
  //------------------------------------------------------------
	EClickResult  MouseClick (int x, int y, int buttons);
	EClickResult  StopClick();
  ECursorResult MouseMoved (int x, int y);
  //------------------------------------------------------------
	void          Draw (void);
protected:
  Tag       cmp;
  Tag       obs;
  CGaugeNeedle  comp_ndl;
  Tag       radi_tag;
  int       radi_unit;
  SMessage  fcs;
  SMessage  mobs;             // OBS message set
  SMessage  mnav;             // Nav message
  SMessage  mrad;             // Radio Message
  BUS_RADIO *radio;           // Radio data
	//--------Localizer needle-- -------------------------------
  int     vobs;                       // OBS from radio
  int     locFramesPerDeg;            // Localizer sensibility
  int     gldFramesPerDeg;            // Glide sensibility
  CAniMAP	lndl;
	//--------Glide slope needle -------------------------------
  int     gspl_x, gspl_y;     // Flag
  U_CHAR  ils;
  U_CHAR  gflg;
  U_CHAR  xflg;
	CAniMAP	gndl;
	//----------OBS management --------------------------------
	CKnob	obkn;						        // OBS KNOB
  CAniMAP *Flags[3];            // Flag pointers
	//----------FLAG bitmap ---------------------------------
  CAniMAP	WarnF;				  // Warning flag
	CAniMAP	TlocF;				  // To localizer
	CAniMAP	FlocF;				  // From localizer

};
//-------------------------------------------------------------

/*
 * Avionics gauges
 */






//=================================================================
// CBKKFC200Gauge
//=================================================================
class CBKKFC200Gauge : public CBitmapGauge {
public:
  CBKKFC200Gauge(CPanel *mp);
 ~CBKKFC200Gauge(void);
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CBKKFC200Gauge"; }
  virtual ECursorResult MouseMoved (int x, int y);

protected:
  CGaugeClickArea   ca01;
  CGaugeClickArea   ca02;
  CGaugeClickArea   ca03;
  CGaugeClickArea   ca04;
  CGaugeClickArea   ca05;
  CGaugeClickArea   ca06;
  CGaugeClickArea   ca07;
  CGaugeClickArea   ca08;
  CGaugeClickArea   ca09;
  CGaugeClickArea   ca10;
  CGaugeClickArea   ca11;
  CAniMAP           arfd;
  CAniMAP           aalt;
  CAniMAP           ahdg;
  CAniMAP           glid;
  CAniMAP           anav;
  CAniMAP           aapr;
  CAniMAP           arbc;
  CAniMAP           trim;
  CAniMAP           arap;

};


//===================================================================
// CBKKFC150Gauge
//===================================================================
class CBKKFC150Gauge : public CBitmapGauge {
public:
   CBKKFC150Gauge(CPanel *mp);
  ~CBKKFC150Gauge(void);
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CBKKFC150Gauge"; }
  virtual ECursorResult MouseMoved (int x, int y);

protected:
  CGaugeClickArea   ca01;
  CGaugeClickArea   ca02;
  CGaugeClickArea   ca03;
  CGaugeClickArea   ca04;
  CGaugeClickArea   ca05;
  CGaugeClickArea   ca06;
  CGaugeClickArea   ca07;
  CGaugeClickArea   ca08;
  CGaugeClickArea   ca09;
  CGaugeClickArea   ca10;
  CGaugeClickArea   ca11;
  CAniMAP           arfd;
  CAniMAP           aalt;
  CAniMAP           ahdg;
  CAniMAP           glid;
  CAniMAP           anav;
  CAniMAP           aapr;
  CAniMAP           arbc;
  CAniMAP           trim;
  CAniMAP           arap;

};

//===========================================================================
#define KMA_CA_OUTM (11)
#define KMA_CA_MIDL (12)
#define KMA_CA_INNR (13)
#define KMA26_CA_SZ (16)
//===========================================================================
// CBKAudioKMA26Gauge
//===========================================================================
class CBKAudioKMA26Gauge : public CBitmapGauge {
public:
  CBKAudioKMA26Gauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  const char* GetClassName (void) { return "CBKAudioKMA26Gauge"; }
  ECursorResult MouseMoved (int x, int y);
  void          PrepareMsg(CVehicleObject *veh);
  void          Draw();
protected:
  Tag         radi;
  int         unit;
  CGaugeClickArea ca[KMA26_CA_SZ];          // Click area
};


//=========================================================================
// CGenericNavRadioGauge
//==========================================================================
class CGenericNavRadioGauge : public CBitmapGauge {
  //------ATTRIBUTES -------------------------------------------------
  CGaugeClickArea ck[7];
  CNavRadio  *RAD;                           // Radio component
  Tag         radi_tag;
  int         radi_unit;
  bool        sdim;
  //-------FONTS and colors -----------------------------------
  CVariFontBMP  *radi9;                       // Radio font
  U_INT          amber;                       // amber color
  //------METHODS -----------------------------------------------------
public:
  CGenericNavRadioGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CGauge methods
  virtual const char* GetClassName (void) { return "CGenericNavRadioGauge"; }
  virtual ECursorResult MouseMoved (int x, int y);
  virtual EClickResult  MouseClick (int x, int y, int buttons);
  //-------------------------------------------------------------------
  int     AnyEvent(int mx,int my);
  void    Draw();
  void    GetRadio();
  void    DrawField(RADIO_FLD *fd);
  //-------------------------------------------------------------------
protected:

};

//===========================================================================
//
// CGenericComRadioGauge
//============================================================================
class CGenericComRadioGauge : public CBitmapGauge {
//------ATTRIBUTES -------------------------------------------------
  CGaugeClickArea ck[7];
  CComRadio  *RAD;                           // Radio component
  Tag         radi_tag;
  int         radi_unit;
  bool        sdim;
  //-------FONTS and colors -----------------------------------
  CVariFontBMP  *radi9;                       // Radio font
  U_INT          amber;                       // amber color
  //-----METHODS -------------------------------------------
public:
  CGenericComRadioGauge (CPanel *mp);
  void    GetRadio();
  int     AnyEvent(int mx,int my);
  void    DrawField(RADIO_FLD *fd);
  void    Draw();
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CGauge methods
  virtual const char* GetClassName (void) { return "CGenericComRadioGauge"; }
  virtual ECursorResult MouseMoved (int x, int y);
  virtual EClickResult  MouseClick (int x, int y, int buttons);
};

//===========================================================================
// CGenericTransponderGauge
//  There are up to 12 click areas
//  4 digits for active ident
//  4 digits for standby ident
//  one 'tune' button
//  one ''idnt' button
//  one 'mode'  button
//  one ' test' 
//============================================================================
class CGenericTransponderGauge : public CBitmapGauge {
  //---- ATTRIBUTES ---------------------------------------
  CGaugeClickArea ck[13];
  Tag         radi_tag;
  int         radi_unit;
  CTransponderRadio *RAD;                     // Radio component
  //-------FONTS and colors -----------------------------------
  CVariFontBMP  *radi9;                       // Radio font
  U_INT          amber;                       // amber color
    //--------------------------------------------------------------------
  U_CHAR         frame;
public:
  CGenericTransponderGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CGauge methods
  virtual const char* GetClassName (void) { return "CGenericTransponderGauge"; }
  virtual ECursorResult MouseMoved (int x, int y);
  //-----------------------------------------------------------------
  EClickResult          MouseClick (int x, int y, int buttons);
  //-----------------------------------------------------------------
  void    GetRadio();
  int     AnyEvent(int mx,int my);
  void    DrawField(RADIO_FLD *fd);
  void    Draw();
};


//=============================================================================
// CGenericADFRadioGauge
//=============================================================================
class CGenericADFRadioGauge : public CBitmapGauge {
protected:
  //---Attributes ----------------------------------------------------------
  CGaugeClickArea   ck[15];             // Click area
  CKR87radio       *RAD;                // Radio system
  U_CHAR            PowST;              // Power status
  U_CHAR            sdim;               // dimmed
  U_CHAR            bPOW;               // Power button
  //--------Font parameters -----------------------------------
  U_CHAR         hCar;                        // Char height
  U_CHAR         wCar;                        // Char wide
  U_INT          amber;                       // Amber color
  CVariFontBMP  *radi9;                       // Radio font
  //------------------------------------------------------------------------
  static RADIO_HIT *RadfHIT[];          // Input driver table
  static RADIO_DSP  RadfFLD[];
  //------------------------------------------------------------------------
public:
  CGenericADFRadioGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CGauge methods
  virtual const char* GetClassName (void) { return "CGenericADFRadioGauge"; }
  virtual ECursorResult MouseMoved (int x, int y);
  virtual EClickResult  MouseClick (int x, int y, int buttons);
  //-----------------------------------------------------------------------
  void  GetRADIO();
  int   AnyEvent(int mx,int my);
  void  DrawField(RADIO_DSP *dsp);
  void  DrawADF();
  void  Draw();
  //-----------------------------------------------------------------------
protected:
};


//==============================================================================
// CRMINeedleGauge
//==============================================================================
class CRMINeedleGauge : public CBitmapGauge {
public:
   CRMINeedleGauge (CPanel *mp);
  ~CRMINeedleGauge (void);
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CRMINeedleGauge"; }
  virtual ECursorResult MouseMoved (int x, int y);

protected:
  Tag       comp_tag;
  CGaugeNeedle  comp;
  int       cpxy_x, cpxy_y;
  CGaugeNeedle  adfn;
  int       anxy_x, anxy_y;
  CGaugeNeedle  rmin;
  int       rnxy_x, rnxy_y;
  Tag       adf1;
  int       adf1_unit;
  Tag       adf2;
  int       adf2_unit;
  Tag       nav1;
  int       nav1_unit;
  Tag       nav2;
  int       nav2_unit;
  CAniMAP   aadf;
  CAniMAP   avor;

  CGaugeClickArea lbtn;
  CGaugeClickArea rbtn;
};


//
// CNavGaugeGroup
//
class CNavGaugeGroup : public CGauge {
public:
  CNavGaugeGroup (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CNavGaugeGroup"; }

protected:
  Tag   nav1;
  Tag   nav2;
};


//
// CRadioGaugeGroup
//
class CRadioGaugeGroup : public CGauge {
public:
  CRadioGaugeGroup (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CRadioGaugeGroup"; }

protected:
  Tag   rad1_tag;   // 'rad1' conflicts with Windows #define
  Tag   rad2_tag;   // 'rad2' conflicts with Windows #define
  Tag   key_tag;
};


//=======================================================================
// CAnnunciator
//=======================================================================
class CAnnunciator : public CBitmapGauge {
public:
  CAnnunciator (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  void  Draw();
  // CGauge methods
  virtual const char* GetClassName (void) { return "CAnnunciator"; }

protected:
};


//======================================================================
// CFlyhawkAnnunciator
//======================================================================
class CFlyhawkAnnunciator : public CGauge {
protected:
	typedef struct {
    TEXT_DEFN txd;            // Texture definition
	  SMessage  amsg;						// system to poll
	  int		x;									// Panel position
	  int		y;									// panel position
	  int		x2;									// right position
	  int		y2;									// left position
	  int		blnk;								// Blink state
	} BlocANN;
public:
	//----------------------------------------------------------------
  CFlyhawkAnnunciator (CPanel *mp);
 virtual ~CFlyhawkAnnunciator (void);
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  // CGauge methods
  virtual const char*	GetClassName (void) { return "CFlyhawkAnnunciator"; }
  virtual void			Draw(void);
  //---------------------------------------------------------------
  // Annunciator method
	void		InitBlock(BlocANN *blc,int x,int y);
	void		FillBlock(BlocANN *blc,SStream *st, Tag tag);
	void		PollBlock(BlocANN *blc);
protected:
	BlocANN	lowl;
	BlocANN	lowr;
	BlocANN	oilp;
	BlocANN	vacl;
	BlocANN	vacr;
	BlocANN	volt;
	BlocANN line;
};
//------------------------------------------------------------------------
// JSDEV specific switch related to FlyHawk annunciator panel
//
// CFlyhawkAnnunciatorTest
//-------------------------------------------------------------------------
class CFlyhawkAnnunciatorTest : public CBitmapGauge {
public:
  CFlyhawkAnnunciatorTest (CPanel *mp);
  virtual ~CFlyhawkAnnunciatorTest (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CFlyhawkAnnunciatorTest"; }
  virtual void          Draw (void);
  virtual ECursorResult MouseMoved (int x, int y);
  virtual EClickResult  MouseClick (int x, int y, int buttons);
  virtual EClickResult  StopClick();
	// This gauge method
  void					InitMsg(SMessage *msg,Tag tag);
  void					UpdateComponent(char old,char now);
protected:
  void DecState (void);
  void IncState (void);

protected:
  CAniMAP butn;             // Bitmap switch
  // Cursors
  Tag     csru_tag;         // Unique tag for switch up cursor
  Tag     csrd_tag;         // Unique tag for switch down cursor
  //--------Define positions ----------------------------
  enum BPOS {	TEST	= 0x00,
				BRIT	= 0x01,
				DIMR	= 0x02,	};
  //------------Message array ---------------------------
  SMessage msgT[3];				// Array of Messages
  //-----------------------------------------------------
  char IncT[4];					// Increment table
  char DecT[4];					// Decrement table
  char	  cPos;					// Current position
  char    oPos;					// Old position
};


//
// CFlyhawkNavGpsPanelGauge
//
class CFlyhawkNavGpsPanelGauge : public CBitmapGauge {
public:
  CFlyhawkNavGpsPanelGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CFlyhawkNavGpsPanelGauge"; }
  virtual ECursorResult MouseMoved (int x, int y);

protected:
  CGaugeKnob    cnav;
  CGaugeKnob    capr;
  int       xnav_x, xnav_y;
  int       xgps_x, xgps_y;
  int       xapr_x, xapr_y;
  int       xmsg_x, xmsg_y;
  int       xwpt_x, xwpt_y;
};


//===================================================================
// CFlyhawkFuelSelectorGauge
//===================================================================
class CFlyhawkFuelSelectorGauge : public CBitmapGauge {
public:
  CFlyhawkFuelSelectorGauge (CPanel *mp);
  virtual ~CFlyhawkFuelSelectorGauge (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);

  // CGauge methods
  virtual const char*   GetClassName (void) { return "CFlyhawkFuelSelectorGauge"; }
  virtual void			    PrepareMsg(CVehicleObject *veh);
  virtual ECursorResult MouseMoved (int x, int y);
  virtual EClickResult  MouseClick (int x, int y, int buttons);
  virtual void			Draw (void);

protected:
  //--------Bitmap management --------------------------------
  CAniMAP     swit;                       // Switch bitmap
  int         switch_frames;
  //------Click area -----------------------------------------
  CGaugeClickArea   left;
  CGaugeClickArea   both;
  CGaugeClickArea   righ;
  SMessage      Lmsg;
  SMessage      Rmsg;
};


//====================================================================
// CFlyhawkElevatorTrimGauge
//====================================================================
class CFlyhawkElevatorTrimGauge : public CBitmapGauge {
public:
  CFlyhawkElevatorTrimGauge (CPanel *mp);
  virtual ~CFlyhawkElevatorTrimGauge (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);

  // CGauge methods
  const char*   GetClassName (void) { return "CFlyhawkElevatorTrimGauge"; }
  ECursorResult MouseMoved (int x, int y);
  EClickResult  MouseClick (int x, int y, int buttons);
  EClickResult  TrackClick (int x, int y, int buttons);
  EClickResult  StopClick  ();
  void			    Draw (void);

protected:
  CAniMAP           wheel;                   // wheel bitmap

  CAniMAP           indwh;                    // Indicator bitmap

  CGaugeClickArea   down;
  CGaugeClickArea   up;

  float             deflect;
  float             wRatio;                   // Frame per deflect ratio
  float             iRatio;
  int               mWheel;
  int               mIndwh;
};


//
// CMalybooCHTIndicatorGauge
//
class CMalybooCHTIndicatorGauge : public CBitmapGauge {
public:
  CMalybooCHTIndicatorGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CMalybooCHTIndicatorGauge"; }

protected:
  int   xoff[6];
  int   yoff;
};


//
// CMalybooCHTButtonGauge
//
class CMalybooCHTButtonGauge : public CBitmapGauge {
public:
  CMalybooCHTButtonGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CMalybooCHTButtonGauge"; }

protected:
};


//
// CMalybooNavGpsPanelGauge
//
class CMalybooNavGpsPanelGauge : public CBitmapGauge {
public:
  CMalybooNavGpsPanelGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CMalaybooNavGpsPanelGauge"; }

protected:
  CGaugeClickArea cnav;
  int       xnav_x, xnav_y;
  int       xgps_x, xgps_y;
  CGaugeClickArea capr;
  int       xapr_x, xapr_y;
  int       xarm_x, xarm_y;
  int       xact_x, xact_y;
  CGaugeClickArea ccrs;
  int       xcrs_x, xcrs_y;
  int       xobs_x, xobs_y;
  int       xleg_x, xleg_y;
  CGaugeClickArea cmsg;
  int       xmsg_x, xmsg_y;
  int       xwpt_x, xwpt_y;
  CGaugeClickArea cyaw;
  int       xyaw_x, xyaw_y;
};


//
// CMalybooDigitalClockGauge
//
class CMalybooDigitalClockGauge : public CBitmapGauge {
public:
  CMalybooDigitalClockGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CMalybooDigitalClockGauge"; }

protected:
  int       clkd_x1, clkd_y1, clkd_x2, clkd_y2;   // Clock display area
  CGaugeClickArea mode;   // Mode button
  CGaugeClickArea powr;   // Power button
  CGaugeClickArea play;   // Play button
  CGaugeClickArea rec;    // Rec button
  CGaugeClickArea set;    // Set button
};

//
// CMalybooEngineMonitorGauge
//
class CMalybooEngineMonitorGauge : public CBitmapGauge {
public:
  CMalybooEngineMonitorGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CMalybooEngineMonitorGauge"; }

protected:
  CGaugeClickArea   fuel;
  CGaugeClickArea   inst;
  CGaugeClickArea   blk_;
  CGaugeClickArea   fill;
  CGaugeClickArea   pwr_;
  CGaugeClickArea   set_;
  CGaugeClickArea   MAP_;
  CGaugeClickArea   RPM_;
  CGaugeClickArea   TIT;
  CGaugeClickArea   FF__;
  CGaugeClickArea   OT__;
  CGaugeClickArea   OP__;
  CGaugeClickArea   CHT_;
  CGaugeClickArea   VAC_;
  CGaugeClickArea   LQTY;
  CGaugeClickArea   RQTY;
  char        rlit_art[64];
  char        glit_art[64];
  char        ylit_art[64];
};


//
// CNavajoFuelSelectorGauge
//
class CNavajoFuelSelectorGauge : public CBitmapGauge {
public:
  CNavajoFuelSelectorGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char*   GetClassName (void) { return "CNavajoFuelSelectorGauge"; }
  virtual void		      PrepareMsg   (CVehicleObject *veh);
  virtual EClickResult  MouseClick   (int x, int y, int buttons);
  virtual ECursorResult MouseMoved   (int x, int y);
  virtual void          Draw         (void);

protected:
  int               cIndx, oIndx;
  //char            swit[64]; // Switch PBG artwork
  CAniMAP           swit;     // switch
  CGaugeClickArea   obca;     // Outboard
  CGaugeClickArea   ofca;     // Off
  CGaugeClickArea   ibca;     // Inboard
  SMessage          obms;     // Outboard tank message
  SMessage          ibms;     // Inboard tank message
};


//
// CNavajoAmmeterGauge
//
class CNavajoAmmeterGauge : public CNeedleGauge {
public:
  CNavajoAmmeterGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CNavajoAmmeterGauge"; }
  virtual void	PrepareMsg(CVehicleObject *veh);
protected:
  SMessage      mChg;     // Charge message
  SMessage      mLtM;     // Left ammeter message
  SMessage      mRtM;     // Right ammeter message
  SMessage      mLtS;     // Left switch message
  SMessage      mRtS;     // Right switch message
  CGaugeClickArea   ca01;     // Left switch
  CGaugeClickArea   ca02;     // Right switch
};


//==========================================================================
// CNavajoRadarAltimeterGauge
//==========================================================================
class CNavajoRadarAltimeterGauge : public CNeedleGauge {
public:
   CNavajoRadarAltimeterGauge(CPanel *mp);
  ~CNavajoRadarAltimeterGauge(void);
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CNavajoRadarAltimeterGauge"; }
  virtual void PrepareMsg(CVehicleObject *veh);
protected:
  SMessage        bMsg;     // DH Bug message
  CAniMAP         abug;
  CAniMAP         plit;

  SMessage        dhms;     // DH message
  CGaugeKnob      knob;     // DH Knob
};


//
// CCollinsEFISSelectionGauge
//
class CCollinsEFISSelectionGauge : public CBitmapGauge {
public:
  CCollinsEFISSelectionGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CCollinsEFISSelectionGauge"; }
  virtual void PrepareMsg(CVehicleObject *veh);
protected:
  CGaugeClickArea   brg_;     // Bearing
  CGaugeClickArea   brgm;     // Bearing mode
  CGaugeKnob      brgk;     // Bearing knob
  CGaugeKnob      dh__;     // DH knob
  CGaugeClickArea   form;     // Display format
  CGaugeClickArea   crs_;     // Course
  CGaugeClickArea   crsm;     // Course mode
  CGaugeKnob      crsk;     // Course knob
  CGaugeClickArea   wx__;     // Weather display
  CGaugeClickArea   nav_;     // Nav Data Toggle
  SMessage      pfdm;     // PFD message
  SMessage      ehim;     // EHSI message
  SMessage      mndm;     // Multi Nav Display message
  SMessage      adm_;     // Air Data message
  SMessage      aplt;     // Autopilot message
};


//
// CCollinsVspeedPanel
//
class CCollinsVspeedPanel : public CBitmapGauge {
public:
  CCollinsVspeedPanel (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CCollinsVspeedPanel"; }

protected:
  CGaugeClickArea   sped;
  CGaugeClickArea   vsel;
};


//
// CCollinsHFRadioGauge
//
class CCollinsHFRadioGauge : public CBitmapGauge {
public:
  CCollinsHFRadioGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CCollinsHFRadioGauge"; }

protected:
  CGaugeClickArea     tel;    //
  CGaugeClickArea     am;     // Amplitude Modulation button
  CGaugeClickArea     usb;    // Upper Sideband button
  CGaugeClickArea     lsb;    // Lower Sideband button
  CGaugeClickArea     mhz;    // MHz button
  CGaugeClickArea     khz;    // KHz button
  CGaugeClickArea     stor;   // Store button
  CGaugeClickArea     tune;   // Tune button
  CGaugeClickArea     volu;   // Volume button
  CGaugeClickArea     dash;   // 
  CGaugeClickArea     tx;     // Tx button
  CGaugeClickArea     chan;   // Channel button
  bool          ft13;   // Use 13-point font
};


//
// CCollinsPFDGauge
//
class CCollinsPFDGauge : public CBitmapGauge {
public:
  CCollinsPFDGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CCollinsPFDGauge"; }

protected:
};


//
// CCollinsNDGauge
//
class CCollinsNDGauge : public CBitmapGauge {
public:
  CCollinsNDGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CCollinsNDGauge"; }

protected:
};


//========================================================================
// CCollinsRTUGauge
//========================================================================
class CCollinsRTUGauge : public CBitmapGauge {
public:
  CCollinsRTUGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CCollinsRTUGauge"; }
  virtual       void PrepareMsg(CVehicleObject *veh);
protected:
  CGaugeRectangle   lin1;     // Line 1
  CGaugeRectangle   lin2;     // Line 2
  CGaugeRectangle   lin3;     // Line 3
  CGaugeRectangle   lin4;     // Line 4
  CGaugeRectangle   lin5;     // Line 5
  CGaugeClickArea   btn1;     // Button 1
  CGaugeClickArea   btn2;     // Button 2
  CGaugeClickArea   btn3;     // Button 3
  CGaugeClickArea   btn4;     // Button 4
  CGaugeClickArea   btn5;     // Button 5
  CGaugeClickArea   cmaw;     // COM Active Whole MHz
  CGaugeClickArea   cmaf;     // COM Active Fract MHz
  CGaugeClickArea   cmsw;     // COM Stby Whole MHz
  CGaugeClickArea   cmsf;     // COM Stby Fract MHz
  CGaugeClickArea   nvaw;     // NAV Active Whole MHz
  CGaugeClickArea   nvaf;     // NAV Active Fract MHz
  CGaugeClickArea   adf1;     // ADF Digit 1
  CGaugeClickArea   adf2;     // ADF Digit 2
  CGaugeClickArea   adf3;     // ADF Digit 3
  CGaugeClickArea   adf4;     // ADF Digit 4
  CGaugeClickArea   adf5;     // ADF Digit 5
  CGaugeClickArea   xpd1;     // Transponder Digit 1
  CGaugeClickArea   xpd2;     // Transponder Digit 2
  CGaugeClickArea   xpd3;     // Transponder Digit 3
  CGaugeClickArea   xpd4;     // Transponder Digit 4
  SMessage      rtu_;		  // RTU subsystem message
  SMessage      dme1;		  // DME 1 message
  SMessage      dme2;		  // DME 2 message
  CAniMAP       adme;
  CAniMAP       arep;

  CGaugeClickArea   atci;     // ATC ID
  CGaugeClickArea   dmeh;     // DME Hold button
  CGaugeClickArea   coms;     // COM Squelch button
  CGaugeClickArea   atcs;     // ATC Standby button
  CGaugeClickArea   b1_2;     // 1/2 button
  CGaugeClickArea   adft;     // ADF Test button
  int         side;     // Side
};


//
// CCollinsAirDataGauge
//
class CCollinsAirDataGauge : public CBitmapGauge {
public:
  CCollinsAirDataGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CCollinsAirDataGauge"; }

protected:
  CGaugeRectangle   scrn;     // Display screen
  CGaugeKnob      baro;     // Barometric Setting knob
  CGaugeClickArea   vs__;     // VS reference knob
  CGaugeClickArea   alt_;     // Altitude Preselect knob
  CGaugeClickArea   in__;     // Inches button
  CGaugeClickArea   mb__;     // Millibars button
};


//
// CCollinsMNDGauge
//
class CCollinsMNDGauge : public CBitmapGauge {
public:
  CCollinsMNDGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CCollinsMNDGauge"; }
  virtual       void  PrepareMsg(CVehicleObject *veh);
protected:
  SMessage      obs_;     // OBS message
  SMessage      ap__;     // AP message
  CGaugeClickArea   frmt;     // Display Format
  CGaugeClickArea   hdg_;     // Heading
  CGaugeClickArea   crs_;     // Course
  CGaugeClickArea   brgt;     // Brightness
};


//
// CUniversalFMSGauge
//
class CUniversalFMSGauge : public CBitmapGauge {
public:
  CUniversalFMSGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CUniversalFMSGauge"; }

protected:
  CGaugeClickArea   scrn;     // Display screen
  CGaugeClickArea   l1;       // Left Select Key 1
  CGaugeClickArea   l2;       // Left Select Key 2
  CGaugeClickArea   l3;       // Left Select Key 3
  CGaugeClickArea   l4;       // Left Select Key 4
  CGaugeClickArea   l5;       // Left Select Key 5
  CGaugeClickArea   r1;       // Right Select Key 1
  CGaugeClickArea   r2;       // Right Select Key 2
  CGaugeClickArea   r3;       // Right Select Key 3
  CGaugeClickArea   r4;       // Right Select Key 4
  CGaugeClickArea   r5;       // Right Select Key 5
  CGaugeClickArea   data;     // DATA key
  CGaugeClickArea   nav;      // NAV key
  CGaugeClickArea   vnav;     // VNAV key
  CGaugeClickArea   dto;      // DTO key
  CGaugeClickArea   list;     // LIST key
  CGaugeClickArea   prev;     // PREV key
  CGaugeClickArea   fuel;     // FUEL key
  CGaugeClickArea   fpl;      // FPL key
  CGaugeClickArea   perf;     // PERF key
  CGaugeClickArea   tune;     // TUNE key
  CGaugeClickArea   menu;     // MENU key
  CGaugeClickArea   next;     // NEXT key
  CGaugeClickArea   key1;     // 1 key
  CGaugeClickArea   key2;     // 2 key
  CGaugeClickArea   key3;     // 3 key
  CGaugeClickArea   key4;     // 4 key
  CGaugeClickArea   key5;     // 5 key
  CGaugeClickArea   key6;     // 6 key
  CGaugeClickArea   key7;     // 7 key
  CGaugeClickArea   key8;     // 8 key
  CGaugeClickArea   key9;     // 9 key
  CGaugeClickArea   back;     // BACK key
  CGaugeClickArea   key0;     // 0 key
  CGaugeClickArea   msg;      // MSG key
  CGaugeClickArea   dim;      // DIM key
  CGaugeClickArea   plus;     // +/- key
  CGaugeClickArea   entr;     // ENT key
  CGaugeClickArea   keyA;     // A key
  CGaugeClickArea   keyB;     // B key
  CGaugeClickArea   keyC;     // C key
  CGaugeClickArea   keyD;     // D key
  CGaugeClickArea   keyE;     // E key
  CGaugeClickArea   keyF;     // F key
  CGaugeClickArea   keyG;     // G key
  CGaugeClickArea   keyH;     // H key
  CGaugeClickArea   keyI;     // I key
  CGaugeClickArea   keyJ;     // J key
  CGaugeClickArea   keyK;     // K key
  CGaugeClickArea   keyL;     // L key
  CGaugeClickArea   keyM;     // M key
  CGaugeClickArea   keyN;     // N key
  CGaugeClickArea   keyO;     // O key
  CGaugeClickArea   keyP;     // P key
  CGaugeClickArea   keyQ;     // Q key
  CGaugeClickArea   keyR;     // R key
  CGaugeClickArea   keyS;     // S key
  CGaugeClickArea   keyT;     // T key
  CGaugeClickArea   keyU;     // U key
  CGaugeClickArea   keyV;     // V key
  CGaugeClickArea   keyW;     // W key
  CGaugeClickArea   keyX;     // X key
  CGaugeClickArea   keyY;     // Y key
  CGaugeClickArea   keyZ;     // Z key
};


//
// CPilatusEFISPanel
//
class CPilatusEFISPanel : public CBitmapGauge {
public:
  CPilatusEFISPanel (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CPilatusEFISPanel"; }
  virtual void	PrepareMsg(CVehicleObject *veh);
protected:
  SMessage      pfdm;     // PFD message
  SMessage      navm;     // ND message
  CGaugeKnob      dh;       // DH knob
  CGaugeKnob      brt;      // BRT knob
  CGaugeKnob      crs;      // CRS knob
  CGaugeKnob      hdg;      // HDG knob
  CGaugeKnob      hsi;      // HSI knob
  CGaugeKnob      arc;      // ARC knob
  CGaugeKnob      nav;      // NAV knob
  CGaugeKnob      rgup;     // Range Up knob
  CGaugeKnob      rgdn;     // Range Down knob
  CGaugeKnob      nav1;     // NAV1 knob
  CGaugeKnob      nav2;     // NAV2 knob
  CGaugeKnob      k1_2;     // 1/2 knob
};


//
// CWalkerTempAirspeedGauge
//
class CWalkerTempAirspeedGauge : public CBitmapGauge {
public:
  CWalkerTempAirspeedGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CWalkerTempAirspeedGauge"; }

protected:
  CGaugeClickArea     tat;      // True Air Temp button
  CGaugeClickArea     sat;      // Static Air Temp button
  CGaugeClickArea     tas;      // True Airspeed button
};


//==========================================================================
// CGyroSlavingGauge
//===========================================================================
class CGyroSlavingGauge : public CBitmapGauge {
public:
   CGyroSlavingGauge (CPanel *mp);
  ~CGyroSlavingGauge (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CGyroSlavingGauge"; }

protected:
  CAniMAP           andl;             // Needle
  CAniMAP           aslv;             // Slave button
  CAniMAP           acwb;             // Mode switch

  CGaugeRectangle     err;            // Error window
  CGaugeClickArea     slav;           // Slave button
  CGaugeClickArea     free;           // Free button
  CGaugeClickArea     mode;           // Slave/Free Toggle button
  CGaugeClickArea     ccw;            // CCW switch
  CGaugeClickArea     cw;             // CW switch
  int           lrgb_r, lrgb_g, lrgb_b;   // Line RGB colour
  //--------sound tags ----------------------------------------
};


//========================================================================
// CVerticalRateGauge
//========================================================================
class CVerticalRateGauge : public CBitmapGauge {
public:
  CVerticalRateGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CVerticalRateGauge"; }

protected:
  CGaugeKnob        eng;      // Engage button
  CGaugeKnob        arm;      // Arm button
  CGaugeKnob        knob;     // VS knob
  CGaugeRectangle     led;      // LED Display area
  Tag           avil;     // Avionics light subsytem
};


//
// CPropSyncGauge
//
class CPropSyncGauge : public CBitmapGauge {
public:
  CPropSyncGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CPropSyncGauge"; }

protected:
};


//
// CDMEGauge
//
class CDMEGauge : public CBitmapGauge {
public:
  CDMEGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CDMEGauge"; }
  virtual ECursorResult MouseMoved (int x, int y);

protected:
  bool        hold;       // Hold mode
  int         dist_x, dist_y;   // Distance text location
  int         sped_x, sped_y;   // Speed text location
  int         time_x, time_y;   // Time text location
  CGaugeKnob      knob;       // Distance text location
  bool        tiny;       // Use tiny fonts
};


//
// CDME2Gauge
//
class CDME2Gauge : public CBitmapGauge {
public:
  CDME2Gauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CDME2Gauge"; }
  virtual ECursorResult MouseMoved (int x, int y);

protected:
  bool        hold;       // Hold mode
  int         left_x, left_y;   // DME text location
  int         rght_x, rght_y;   // SEL text location
  CGaugeClickArea   ch;         // Channel button
  CGaugeClickArea   sel;        // Select button
  CGaugeClickArea   pwr;        // Power button
};


//
// CWeatherRadarGauge
//
class CWeatherRadarGauge : public CBitmapGauge {
public:
  CWeatherRadarGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CWeatherRadarGauge"; }

protected:
  CGaugeRectangle   scrn;   // Display screen
  CGaugeClickArea   wx;     // WX button
  CGaugeClickArea   vp;     // VP button
  CGaugeClickArea   map;    // MAP button
  CGaugeClickArea   nav;    // NAV button
  CGaugeClickArea   rngp;   // RNG+ button
  CGaugeClickArea   rngm;   // RNG- button
  CGaugeClickArea   trkp;   // TRK+ button
  CGaugeClickArea   trkm;   // TRK- button
  CGaugeKnob      mode;   // Mode knob
  CGaugeKnob      tilt;   // Tile knob
  CGaugeKnob      gain;   // Gain knob
  CGaugeKnob      brit;   // Brightness knob
};


//
// CAnalogAltPreselectGauge
//
class CAnalogAltPreselectGauge : public CBitmapGauge {
public:
  CAnalogAltPreselectGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CAnalogAltPreselectGauge"; }

protected:
  CGaugeKnob      knob;   // Altitude Select knob
  char        fdig_art[64]; // FL Digits strip PBM filename
  char        digi_art[64]; // Digits strip PBM filename
  char        zero_art[64]; // Zero digits strip PBM filename
  int         thou_x, thou_y; // Thousands digit offset
  int         hund_x, hund_y; // Hundreds digit offset
  int         ones_x, ones_y; // Ones digit offset
  int         dspc;     // Digits spacing
  int         zspc;     // Zero spacing
  CGaugeClickArea   aler;     // Altitude alert
};

//
// Garmin GPS150
//
typedef enum {
  ACTION_NONE,
  ACTION_PWR_LEFT,
  ACTION_PWR_RIGHT,
  ACTION_DTO,
  ACTION_NRST,
  ACTION_SET,
  ACTION_RTE,
  ACTION_WPT,
  ACTION_NAV,
  ACTION_CRSR,
  ACTION_STAT,
  ACTION_CLR,
  ACTION_ENT,
  ACTION_OUTER_LEFT,
  ACTION_OUTER_RIGHT,
  ACTION_INNER_LEFT,
  ACTION_INNER_RIGHT,
  ACTION_KEYLOCK
} EG150Action;

class CGarminGPS150Gauge : public CBitmapGauge {
public:
  CGarminGPS150Gauge (CPanel *mp);
	CGarminGPS150Gauge();
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  const char*   GetClassName (void) { return "CGarminGPS150Gauge"; }
  EClickResult  MouseClick (int x, int y, int buttons);
  EClickResult  TrackClick (int x, int y, int buttons);
  EClickResult  StopClick ();
  ECursorResult MouseMoved (int x, int y);
  void          Draw (void);
  void          ClickAction (EG150Action action);

protected:
  bool              pwrOn;
  SMessage          msg;
  int               brightness;
  CGaugeClickArea   ca01;
  CGaugeClickArea   ca02;
  CGaugeClickArea   ca03;
  CGaugeClickArea   ca04;
  CGaugeClickArea   ca05;
  CGaugeClickArea   ca06;
  CGaugeClickArea   ca07;
  CGaugeClickArea   ca08;
  CGaugeClickArea   ca09;
  CGaugeClickArea   ca10;
  CGaugeClickArea   ca11;
  CGaugeClickArea   ca12;
  CGaugeClickArea   ca13;
  CGaugeClickArea   ca14;
  CGaugeClickArea   ca15;
  CGaugeClickArea   ca16;
  CGaugeClickArea   ca17;
  CGaugeClickArea   ca18;
  CGaugeClickArea   ca19;
  CGaugeClickArea   ca20;
  float             caClickStart;   ///< Start of current click area delay
  EG150Action       caAction;       ///< Action associated with current click area
  SFont             font;           ///< Gps150 custom font
};


/*
 * Switch gauge types
 *
 * The following gauges implement user-interactive objects such as switches, knobs,
 *   wheels, etc.
 */

//=========================================================================
// CMomentaryHotSpotGauge
//=========================================================================
class CMomentaryHotSpotGauge : public CBitmapGauge {
public:
  CMomentaryHotSpotGauge (CPanel *mp);
  CGaugeClickArea ca;  
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CGauge methods
  virtual const char* GetClassName (void) { return "CMomentaryHotSpotGauge"; }
  //---Mouse interface -----------------------------------------
  EClickResult  MouseClick (int x, int y, int buttons);
  //---Dont draw anything for this gauge -----------------------
  void  Draw()  {}

protected:
};

//========================================================================
// CSimpleInOutStateSwitch 'sios'
//=========================================================================
class CSimpleInOutStateSwitch : public CBitmapGauge {
public:
  CSimpleInOutStateSwitch (CPanel *mp);
  virtual ~CSimpleInOutStateSwitch (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);
  virtual void  PrepareMsg(CVehicleObject *veh);
  // CGauge methods
  virtual const char* GetClassName (void) { return "CSimpleInOutStateSwitch"; }
  virtual void  Draw (void);
  virtual EClickResult  MouseClick (int x, int y, int buttons);
  void    SetChange();
  void    DrawChange();
  void    CheckHold();
protected:
  CAniMAP  stsw;                           // Bitmap switch
  int      vin[2];                         // On/off intt

  //------------------------------------------------------------------
  char     stat;                            // On/Off state
  char     chng;                            // changing state
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


//=============================================================
// CLitInOutSwitch
//==============================================================
class CLitInOutSwitch : public CSimpleInOutStateSwitch {
public:
  CLitInOutSwitch (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  virtual const char* GetClassName (void) { return "CLitInOutSwitch"; }

protected:
  SMessage  powr;
};


//=======================================================================
// CSimpleSwitch
//=======================================================================
class CSimpleSwitch : public CGauge {
public:
  CSimpleSwitch (CPanel *mp);
  virtual ~CSimpleSwitch (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);
//  virtual void  Update (void);

  // CGauge methods
  virtual void          PrepareMsg(CVehicleObject *veh);				// JSDEV* prepare gauge  messages
  virtual const char*   GetClassName (void) { return "CSimpleSwitch"; }
  virtual void          Draw (void);
  virtual ECursorResult MouseMoved (int x, int y);
  virtual EClickResult  MouseClick (int x, int y, int buttons);

protected:
  void DecState (void);
  void IncState (void);

protected:
  CAniMAP swit;           // Bitmap sitch
  int     lastP;          // Previous position
  int     cIndx;          // Current index
  int     oIndx;          // previous index
  //-------- Cursor Tags -------------------------
  Tag     csru_tag;     // Unique tag for switch up cursor
  Tag     csrd_tag;     // Unique tag for switch down cursor

  // States---------------------------------------
  int     stat_n;     // Number of states
  int     *stat;      // Array of state values
  int     sstr_n;     // Number of state strings
  char    **sstr;     // Array of stat strings
  int     orie;     // Orientation
  int     ordr;     // Order
  bool    mmnt;     // Momentary contact
  bool    mntO;     // Momentary open
};


//==================================================================
// CBasicBatterySwitch
//==================================================================
class CBasicBatterySwitch : public CSimpleSwitch {
public:
  CBasicBatterySwitch (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CGauge methods
  virtual const char* GetClassName (void) { return "CBasicBatterySwitch"; }
  virtual void PrepareMessage(CVehicleObject *veh);
  virtual void  Draw (void);
  virtual EClickResult  MouseClick (int x, int y, int buttons);

protected:
  Tag     altt;
  Tag     batt;
  SMessage  mbat;
  SMessage  malt;
  char    sAlt;
  char    sBat;
};


//===================================================================
// CTwoStateELTGauge
//====================================================================
class CTwoStateELTGauge : public CSimpleSwitch {
public:
  CTwoStateELTGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CTwoStateELTGauge"; }

protected:
};


//
// CSwitchPairGauge
//
class CSwitchPairGauge : public CSimpleSwitch {
public:
  CSwitchPairGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CSwitchPairGauge"; }
  virtual void PrepareMsg(CVehicleObject *veh);
  virtual ECursorResult MouseMoved (int x, int y);

protected:
  SMessage    s1ms;
  SMessage    s2ms;
  CGaugeClickArea s1of;
  CGaugeClickArea s1on;
  CGaugeClickArea s2of;
  CGaugeClickArea s2on;
};


//=======================================================================
// CSimpleFloatSwitch
//=======================================================================
class CSimpleFloatSwitch : public CGauge {
public:
  CSimpleFloatSwitch (CPanel *mp);
  virtual ~CSimpleFloatSwitch (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CSimpleFloatSwitch"; }

protected:
  char    bmap_art[64];

  //---------- Cursor tgas --------------------------
  int     csru_tag, csrd_tag;
  int     stat_n;
  float   *stat;
  int     sstr_n;
  char    **sstr;
  bool    mmnt;
};


//=================================================================
// CDualSwitch
//=================================================================
class CDualSwitch : public CSimpleSwitch {
public:
  CDualSwitch (CPanel *mp);
  virtual ~CDualSwitch (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);
//  virtual void  Update (void);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CDualSwitch"; }
  virtual void          Draw (void);
  virtual ECursorResult MouseMoved (int x, int y);
  virtual EClickResult  MouseClick (int x, int y, int buttons);
  virtual void PrepareMsg(CVehicleObject *veh);

protected:
  void DecState (void);
  void IncState (void);

protected:
  int     nums;
  int     orie;
  SMessage  usrl;
  SMessage  usrr;
  bool    mmnt;
  int     dswi_timer;
};


//===========================================================================
// CPushPullKnobGauge
//===========================================================================
class CPushPullKnobGauge : public CGauge {
public:
  CPushPullKnobGauge (CPanel *mp);
  virtual ~CPushPullKnobGauge (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);

  // CGauge methods
  virtual const char*   GetClassName (void) { return "CPushPullKnob"; }
  virtual void PrepareMsg(CVehicleObject *veh);
  virtual void          Draw (void);
  virtual ECursorResult MouseMoved (int x, int y);
  virtual ECursorResult DisplayRatio();
  virtual EClickResult  MouseClick (int x, int y, int buttons);
  virtual EClickResult  TrackClick (int x, int y, int buttons);
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
protected:
  void DecValue (void);
  void IncValue (void);
  int  nframe;

protected:
  //------ ATTRIBUTES --------------------------------------
  CAniMAP     pknob;                  // Bitmap art
  SKnobArea  *Area;                   // Stack of values
  //
  SMessage    lnk1;
  SMessage    polm;
  int         ytop;
  int         ybtm;
  
  //------ values ------------------------------------------
  int         fram;       // Current frame
  int         nVal;       // Number of values
  int         cVal;       // Current value
  int         wait;       // Initial value
  int         time;       // Time tampo
  //--------------------------------------------------------
  char        invt;        // sort order
  float       lowv;
  float       high;
};


//
// CPrimerKnobGauge
//
class CPrimerKnobGauge : public CPushPullKnobGauge {
public:
  CPrimerKnobGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CPrimerKnobGauge"; }

protected:
};

//======================================================================
//  Gauge FLAP
//======================================================================
class CFlapsSwitchGauge : public CPushPullKnobGauge {
public:
  CFlapsSwitchGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
//  virtual void			Draw (void);
//  virtual EClickResult  MouseClick (int x, int y, int buttons);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CFlapsSwitchGauge"; }

protected:
  //SMessage          flapmsg;
  //float             deflect;
//  float             step;
};


class CLitLandingGearKnobGauge : public CPushPullKnobGauge {
public:
  CLitLandingGearKnobGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CLitLandingGearKnobGauge"; }

protected:
};

//--------------------------------------------------------------------------
class CReversibleThrottleKnobGauge : public CPushPullKnobGauge {
public:
  CReversibleThrottleKnobGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CReversibleThrottleKnobGauge"; }

protected:
  SMessage    rvsr;
  char      rbmp_art[64];
  int       rtop;
  int       rbtm;
};
//========================================================================
//  Dual KNOB GAUGE
//  TODO:  Implement animated bitmap through CBitmap
//========================================================================
class CDualKnobGauge : public CGauge {
public:
  CDualKnobGauge (CPanel *mp);
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  int   Warn01(Tag tag);
  void  Draw();
  //--- CGauge methods ----------------------------------
  virtual const char* GetClassName (void) { return "CDualKnobGauge"; }
  void          PrepareMsg(CVehicleObject *veh);
  ECursorResult MouseMoved (int x, int y);
  EClickResult  MouseClick (int mouseX, int mouseY, int buttons);
  EClickResult  StopClick  ();

  //-------Attributes -----------------------------------
protected:
  CVariator       ivar;
  CVariator       ovar;
  float           iinc;
  float           idec;
  float           oinc;
  float           odec;
  //-----------------------------------------------------
  CGaugeClickArea inca;
  CGaugeClickArea ouca;
};
//=========================================================================
class CFlyhawkLightKnobGauge : public CDualKnobGauge {
public:
  CFlyhawkLightKnobGauge (void);

  // CStreamObject methods
//  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CFlyhawkLightKnobGauge"; }

protected:
};
//=========================================================================
//  Simple turn knob
//  TODO: Implement animated bitmap through CBitmap
//==========================================================================
class CTurnKnobGauge : public CGauge {
public:
  CTurnKnobGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CTurnKnobGauge"; }
  //------------------------------------------------------------------
  void         Draw();
  EClickResult MouseClick (int mouseX, int mouseY, int buttons);
  EClickResult StopClick  ();
  //------------------------------------------------------------------
protected:
  char bmap_art[64];
  CVariator var;
  float     incr;
  float     decr;
};

//=======================================================================
//  CBasicMagnetoSwitch
//=======================================================================
class CBasicMagnetoSwitch : public CSimpleSwitch {
public:
  CBasicMagnetoSwitch (CPanel *mp);

  // CStreamObject methods
  virtual void  Draw (void);

  // CGauge methods
  virtual const char*   GetClassName (void) { return "CBasicMagnetoSwitch"; }
  EClickResult  MouseClick (int x, int y, int buttons);
  EClickResult  StopClick  ();
  ECursorResult MouseMoved (int x, int y);
  //----Attributes ------------------------------------------------
protected:
};
//=======================================================================
//  CBasicMagnetoSwitch
//=======================================================================
class CCoveredSwitch : public CSimpleSwitch {
public:
  CCoveredSwitch (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CCoveredSwitch"; }

protected:
};
//=======================================================================
//  CCoveredLightButtonGauge
//=======================================================================
class CCoveredLightButtonGauge : public CCoveredSwitch {
public:
  CCoveredLightButtonGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CCoveredLightButtonGauge"; }

protected:
  CAniMAP   lbut;           // Lighted button
  SMessage  lmsg;         // Light message
};


class CVerticalSliderGauge : public CGauge {
public:
  CVerticalSliderGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CVerticalSliderGauge"; }

protected:
};

class CTrimWheelGauge : public CGauge {
public:
  CTrimWheelGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CTrimWheelGauge"; }
  virtual ECursorResult MouseMoved (int x, int y);

protected:
  char      bmap_art[64];
  CGaugeClickArea down;
  CGaugeClickArea up;
};


class CTrimWheelIndicatorGauge : public CTrimWheelGauge {
public:
  CTrimWheelIndicatorGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CTrimWheelIndicatorGauge"; }

protected:
};

//============================================================================
//  CIndicator gauge is connected to a subsystem giving a value.
//                  The value gives the farme to draw
//============================================================================
class CIndicatorGauge : public CGauge {
public:
  CIndicatorGauge (CPanel *mp);

  // CStreamObject methods
  int     Read (SStream *stream, Tag tag);
  void    ReadFinished();
  void    SetBitmap(SStream *str);
  void    Draw();
  float   NormValue(float val);
  // CGauge methods
  const char* GetClassName (void) { return "CIndicatorGauge"; }

protected:
  int      NbFr;
  CBitmap *bmap;
  float    frpv;               // Frame per value
  float    lval;
  float    hval;
};
//============================================================================
//  Flap indicator gauge
//============================================================================
class CFlapsIndicatorGauge : public CIndicatorGauge {
public:
  CFlapsIndicatorGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CFlapsIndicatorGauge"; }

protected:
};

//=================================================================
//  Push Button
//==================================================================
class CPushButtonGauge : public CGauge {
public:
  CPushButtonGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CPushButtonGauge"; }

protected:
  char    bmap_art[64];
  char    curs_csr[64];
  bool    mmnt;
};


class CLightButtonGauge : public CPushButtonGauge {
public:
  CLightButtonGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CLightButtonGauge"; }

protected:
  SMessage  lmsg;
  int     stat_off, stat_on;
  char    sstr_off[64];
  char    sstr_on[64];
};


class CTurnSwitchGauge : public CSimpleSwitch {
public:
  CTurnSwitchGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CTurnSwitchGauge"; }

protected:
};
//=====================================================================
//  CMasterCautionWarningButtonGauge
//=====================================================================
class CMasterCautionWarningButtonGauge : public CGauge {
public:
  CMasterCautionWarningButtonGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CMasterCautionWarningButtonGauge"; }
  virtual void PrepareMsg(CVehicleObject *veh);
protected:
  char    bmap_art[64];
  //------Messages --------------------------
  SMessage  smsg;
  SMessage  lmsg;
};
//=====================================================================
//  CKAFireExtinguisherButton
//=====================================================================
class CKAFireExtinguisherButton : public CGauge {
public:
   CKAFireExtinguisherButton (CPanel *mp);
  ~CKAFireExtinguisherButton (void);
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CKAFireExtinguisherButton"; }

protected:
  CAniMAP         afir;
  CAniMAP         arok;
  CAniMAP         adsc;

  //------ Cursor tag -------------------------
  int             curs_tag;
};

//=====================================================================
//  CWalkerAPPanel
//=====================================================================

class CWalkerAPPanel : public CGauge {
public:
  CWalkerAPPanel (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CWalkerAPPanel"; }

protected:
  CGaugeClickArea   left;
  CGaugeClickArea   rght;
  CGaugeClickArea   up;
  CGaugeClickArea   down;
  CGaugeClickArea   xfer;
  CGaugeClickArea   turb;
  CGaugeClickArea   ydon;
  CGaugeClickArea   ydof;
  CGaugeClickArea   apon;
  CGaugeClickArea   apof;
  char        enga_yd_art[64];
  char        enga_ap_art[64];
  char        knob_bnk_art[64];
  char        knob_trm_art[64];
};


class CWalkerFirePanel : public CGauge {
public:
  CWalkerFirePanel (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CWalkerFirePanel"; }

protected:
  SMessage      fir1;
  SMessage      fir2;
  CGaugeKnob      ext1;
  CGaugeKnob      ext2;
  CGaugeClickArea   bel1;
  CGaugeClickArea   bel2;
  CGaugeRectangle   ef1;
  CGaugeRectangle   ef2;
};


class CLightSwitchSetGauge : public CBitmapGauge {
public:
  CLightSwitchSetGauge (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CLightSwitchSetGauge"; }
  virtual void		  PrepareMsg(CVehicleObject *veh);
protected:
  SMessage      lmsg;
};

//===============================================================================
//  CRockerSwitch
//===============================================================================
class CRockerSwitch : public CSimpleSwitch {
public:
  CRockerSwitch (CPanel *mp);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);

  // CGauge methods
  virtual const char* GetClassName (void) { return "CRockerSwitch"; }
  virtual void PrepareMsg(CVehicleObject *veh);
  virtual ECursorResult MouseMoved (int x, int y);

protected:
  CGaugeClickArea   one;
  CGaugeClickArea   two;
  SMessage      msg1;
  SMessage      msg2;
};
//=================================================================================

/*

//
// CNavRadioGroup
//
class CNavRadioGroup : public CGauge {
public:
  CNavRadioGroup (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

protected:
};


//
// CComRadioGroup
//
class CComRadioGroup : public CGauge {
public:
  CComRadioGroup (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

protected:
};


//
// CTransponderRadioGroup
//
class CTransponderRadioGroup : public CGauge {
public:
  CTransponderRadioGroup (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

protected:
};


//
// CAdfRadioGroup
//
class CAdfRadioGroup : public CGauge {
public:
  CAdfRadioGroup (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

protected:
};

//
// CMalybooFuelSelectGauge
//
class CMalybooFuelSelectGauge : public CBitmapGauge {
public:
  CMalybooFuelSelectGauge (void);

  // CStreamObject methods
//  int   Read (SStream *stream, Tag tag);

protected:
};

//
// CTCASGauge
//
class CTCASGauge : public CBitmapGauge {
public:
  CTCASGauge (void);

  // CStreamObject methods
//  int   Read (SStream *stream, Tag tag);

protected:
};


*/
//============END OF FILE ================================================================

#endif    // GAUGES_H
