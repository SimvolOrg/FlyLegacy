/*
 * Globals.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2006 Chris Wallace
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

/*! \file Globals.h
 *  \brief Defines data structures for global variable collection
 *
 *  To avoid proliferation of global variables throughout the application,
 *    all globals must be defined in the data structures within this
 *    header.  The single global variable data structure instance is
 *    defined in Main.cpp
 */


#ifndef GLOBALS_H
#define GLOBALS_H

//#if _MSC_VER > 1000
//#pragma once
//#endif // _MSC_VER > 1000

#include "../Include/FlyLegacy.h"
#include "../Include/Pod.h"
#include "../Include/Situation.h"
#include "../Include/DrawVehiclePosition.h"
#include "../Include/DrawVehicleSmoke.h"
#include "../Include\BlackBox.h"
#include "../Include/ElevationTracker.h"
#include "../Include/OSMobjects.h"
#include "../Include/LogFile.h"
#include "../Include/Ui.h"                 // sdk:
//=============================================================================================
class CFmtxMap;
class CBuilder;
class OSM_Object;
class CPluginMain;
//=============================================================================================
extern char *ReadTheFile(FILE *f, char *buf);
extern char *directoryTAB [];
extern float colorTABLE[];
extern char  GetOSMTextureDirectory(U_INT type);
extern void  InhibitOSM(char *type);
extern char *GetOSMType(U_INT t);
extern char  GetOSMUse(U_INT t);
extern char  SwapOSMuse(U_INT t);
//=============================================================================================
//	Directory number
//=============================================================================================
#define FOLDER_ART				(0)
#define FOLDER_OSM_TEXT		(1)
#define FOLDER_OSM_OBJS		(2)
#define FOLDER_OSM_TREE		(3)
#define FOLDER_OSM_LITE		(4)
#define FOLDER_OSM_BASE   (5)
#define FOLDER_MAX        (6)
//=============================================================================================
//	Define APPLICATION PROFILE
//	APPLICATION PROFILE is used by specifics windows like editors.
//	Option to prevent drawing and changing camera are set
//=============================================================================================
#define PROF_NO_TER			(0x00000001)				// No terrain drawing
#define PROF_NO_APT			(0x00000002)				// No airport drawing
#define PROF_NO_INT			(0x00000004)				// No internal plane drawing
#define PROF_NO_EXT			(0x00000008)				// No external plane drawing
//-----------------------------------------------------------------------
#define PROF_NO_OBJ			(0x00000010)				// No Object drawing
#define PROF_NO_MET			(0x00000020)				// No Meteo effect
#define PROF_NO_OSM     (0x00000040)				// No OSM objects
#define PROF_EDITOR			(0x00000080)				// This is an editor
//-----------------------------------------------------------------------
#define PROF_NO_PLN			(0x00000100)				// No plane
//-----------------------------------------------------------------------
#define PROF_DR_DET			(0x02000000)				// Draw detail tour	
#define PROF_DR_ELV			(0x04000000)				// Draw elevations
#define PROF_TRACKE			(0x08000000)				// Track elevation
#define PROF_DRAWRB			(0x10000000)				// Draw rabbit
#define PROF_NO_TEL			(0x20000000)				// No teleport
#define PROF_ACBUSY			(0x80000000)				// Aircraft busy
//-------------------------------------------------------------------------
//	Basic editor profile
//	no view, no meteo, use a rabbit camera, draw tile tour,no flight plan
//-------------------------------------------------------------------------
#define PROF_NO_PLANE   (PROF_NO_INT+PROF_NO_EXT+PROF_ACBUSY)
#define PROF_TOOL       (PROF_EDITOR | PROF_ACBUSY)
//=======================================================================================
//  Color index
//=======================================================================================
#define COLOR_BLACK_TRANS				(0)					// Black tranparent
#define COLOR_BLACK_OPAQUE			(1)					// black OPAQUE
#define COLOR_WHITE							(2)					// White color
#define COLOR_RED								(3)					// Red color
#define COLOR_PURE_GREEN				(4)					// Pure green
#define COLOR_PURE_BLUE					(5)					// Pure blue
#define COLOR_LIGHT_BLUE				(6)					// Light blue
#define COLOR_YELLOW						(7)					// Pure yelllow
#define COLOR_AMBER							(8)					// Orange
//=======================================================================================
//  Edit column for statistics
//=======================================================================================
#define STATS_NUM (20)
//=======================================================================================
//  AIRPORT OPTIONS
//=======================================================================================
#define APT_OPT_ALIT          (0x00000001)        // Alight airport
//=======================================================================================
//	GL Color
//=======================================================================================
inline void ColorGL(U_INT col)
	{	glColor4fv(colorTABLE + (col << 2));
	}
//=======================================================================================
//-------------------Font definition -------------------------------
typedef struct {
  SFont     ftasci10;
  SFont     ftsmal10;
  SFont     ftthin24;
  //----Mono font in BMP ------------------
  SFont     ftmono8;
  SFont     ftlucinda12;
  SFont     ftmono14;
  SFont     ftmono20;
  //----Vari font in BMP ------------------
  SFont     ftradi9;
} SFonts;
//-----------------------------------------------------------------
//  Scenery  OPTIONs
//-----------------------------------------------------------------
typedef enum {
  SCN_NO_AWTER  = 0x0001,             // No animated water
  SCN_NO_TRTEX  = 0x0002,             // No terrain texture
  SCN_NO_PLANE  = 0x0004,             // No aircraft
  SCN_NO_3DOBJ  = 0x0008,             // No 3D object
  SCN_NO_AIRPT  = 0x0010,             // No airport
  SCN_NO_METEO  = 0x0020,             // No Meteo
} SCN_OPTION;
//-----------------------------------------------------------------
//  Vector Map options
//-----------------------------------------------------------------
typedef enum {
  VM_DROS = 0x0001,                 // Draw compass rose
  VM_FROS = 0x0002,                 // Fade compass
  VM_DLAB = 0x0004,                 // Draw labels
  VM_DAPT = 0x0008,                 // Draw airports
  VM_DNDB = 0x0010,                 // Draw NDB
  VM_DVOR = 0x0020,                 // Draw VOR
  VM_DILS = 0x0040,                 // Draw ILS
  VM_DCST = 0x0080,                 // Draw coasts
  //------------------------------------------------
  VM_APTN = 0x0100,                 // Airport names
  VM_APTI = 0x0200,                 // Airport ident
  VM_VORN = 0x0400,                 // VOR names
  VM_VORI = 0x0800,                 // VOR ID
  VM_VORF = 0x1000,                 // VOR Frequencies
  VM_NDBN = 0x2000,                 // NDB names
  VM_NDBI = 0x4000,                 // NDB ID
	//--------------------------------------------------
	VM_PLAN = 0x8000,									// Mode plan
  //--------------------------------------------------
  VM_IDEN = (VM_APTI | VM_VORI | VM_NDBI),
  VM_NAME = (VM_APTN | VM_VORN | VM_NDBN),
  VM_FREQ = (VM_VORF),
} VMAP_OPTION;
//-----------------------------------------------------------------
typedef enum {
    //-------------Trace entries --------------------------------
  TRACE_TIMESLICE	=	(0x0001),				// Time slice
  TRACE_MSG_PREPA	=	(0x0002),				// Message preparation
  TRACE_DEPENDENT	=	(0x0004),				// Message Dependent
  TRACE_DBCACHEMGR=	(0x0008),				// DB cache Mgr
  TRACE_FRAME_RATE= (0x0010),       // FrameRate
  TRACE_TERRAIN_OP= (0x0020),       // Terrain Manager
  TRACE_SUBSYSTEM = (0x0040),       // Trace subsystem
  TRACE_LATITUDE  =   (TRACE_TERRAIN_OP | TRACE_DBCACHEMGR),
  //-----------------------------------------------------------
  TRACE_WHEEL     = (0x0080),       // Wheel animation

} ETrace;
//===========================================================================================
//  Class CFuiRadioBand to display radio communication
//  NOTE: This class is instanciated in global situation, but the Draw method is called
//  in the context of CFuiManager as it is visible in every view
//===========================================================================================
class CFuiRadioBand {
  //---------------Internal state --------------------------------------------------------
  enum  BANDST    { RB_STOP   = 0,                          // Inactive
                    RB_INIT   = 1,                          // Start emiting
                    RB_DSP1   = 2,                          // Display part 1
                    RB_DSP2   = 3,                          // Display part 2
                    RB_DEND   = 4,                          // Display end
                    RB_STBY   = 5,                          // Stand by
  };
  //---------------Entries for ATIS radio ----------------example ------------------------
  enum  COM_ATIS  { AT_IDEN   = 0,                          // KSFO
                    AT_TXT1   = 1,                          // Arrival info
                    AT_TIDN   = 2,                          // Romeo
                    AT_TIMH   = 3,                          // 13H00
                    AT_HIDN   = 4,                          // ZULU
                    AT_TXT2   = 5,                          // Automated weather observations. Wind
                    AT_WIND   = 6,                          // calm
                    AT_TXT3   = 7,                          // Visibility
                    AT_MVIS   = 8,                          // 50 
                    AT_TXT4   = 9,                          // miles. Temperature is
                    AT_TEMP   = 10,                         // 24
                    AT_TXT5   = 11,                         // degrees. Dewpoint is
                    AT_DEWP   = 12,                         // 20
                    AT_TXT6   = 13,                         // degres. Altimeter is 
                    AT_ALTI   = 14,                         // 29.92
                    AT_SMAL   = 15,                         //---Item number for small airports ---
                    //-----For control centers -----------------------------------
                    AT_TXT7   = 15,                         // Notice to airmem...
                    AT_CLDY   = 16,                         // 118.20 (clearance delivery)
                    AT_TXT8   = 17,                         // Advise on initial contact you have information 
                    AT_IDNT   = 18,                         // Romeo
                    AT_FULL   = 19,                         // Array dimension
  };
  //--------------Attribute ------------------------------------------------------
	CVehicleObject *mveh;									// User vehicle
	//------------------------------------------------------------------------------
	TXT_LIST  sTxt;                                           // Text description
  SFont    *font;                                           // Fonte
  CFont    *fnts;                                           // Real font
  SSurface *surf;                                           // Surface
  short     wd;                                             // wide
  short     ht;                                             // Height
  short     wCar;                                           // Character wide
  short     NbCar;                                          // Number of characters
  U_INT     back;                                           // Back color
  U_INT     black;
  BANDST    state;                                          // Active inactive state
  //---------For time Management -------------------------------------------------
  float     timTM;                                          // Minute timer
  SDateTime	lTime;                                          // Local time
  U_CHAR    tInfo;                                          // Time info edited
  U_CHAR    nInfo;                                          // timer info 
  //---------Editied fields ------------------------------------------------------
  char      eFrq[8];                                        // Clearance frequency
  char      hour[8];                                        // Time edited
  char      eVis[4];                                        // Visibility in nm
  char      eTmp[4];                                        // Temperature in deg
  //---------For editing radio band ----------------------------------------------
  short     xScr;                                           // Screen index
  short     cPos;                                           // new char position
  short     nCar;                                           // Number of characters to screen
  short     sEnd;                                           // screen right starting position
  short     sDeb;                                           // screen left limit
  //---------For tuned radio comm -------------------------------------------------
  CObjPtr   rCOM;                                           // Current radio com
	//--- Radio stack --------------------------------------------------------------
  Tag       rTAG[4];					// Radio TAG: NAV-COM-ADF
	//-----------------------ATHIS text --------------------------------------------
  char    *atis[AT_FULL];
  //-------------Method ----------------------------------------------------------
public:
   CFuiRadioBand();
  ~CFuiRadioBand();
	void	Init();
  //-----------------------Edit --------------------------------------------------
  void  SetATIStext();
  //------------------------Size parameters --------------------------------------
  void  Resize();
  //------------------------Drawing method ---------------------------------------
  void  Draw();
  //-----------------------Time slice --------------------------------------------
  void      TimeSlice(float dT);
  void      UpdateInfo();
  void      UpdateTime();
  bool      TuneTo(CVehicleObject *veh,CCOM *com);
  bool      ModeATIS(CCOM *com);
  COM_ATIS  EditATISdata(CAirport *apt);
	//--- Radio interface ----------------------------------------------------------
	void	Register(CVehicleObject *mv);	
	//------------------------------------------------------------------------------
  void  RegisterNAV(CVehicleObject *veh,Tag r);
  void  RegisterCOM(CVehicleObject *veh,Tag r);
  void  RegisterADF(CVehicleObject *veh,Tag r);
	//--- Return radio by tag ------------------------------------------------------	
  Tag   GetNAV()              {return  rTAG[NAV_INDEX];}  //rNAV;}
  Tag   GetCOM()              {return  rTAG[COM_INDEX];}  //rCOM;}
  Tag   GetADF()              {return  rTAG[ADF_INDEX];}  //rADF;}
  Tag   GetRadio(int k)       {return  rTAG[k];}
	//------------------------------------------------------------------------------
	
  //------------------------------------------------------------------------------
	void	Stop()											{state = RB_STOP;}
};

//==============================================================================
// Time zone variables
//==============================================================================
  //-------------Time zone structure -------------------------
  typedef struct  { char *tzName;       // Time zone designation
                    short tzDelta;      // Delta from UTC
  } STIME_ZONE;
//-------------------------------------------------------------------
#define WINDOW_TBROS      (0x01)        // TERRA  BROWSER
#define WINDOW_MBROS      (0x02)        // OBJECT BROWSER
//=======================================================================
//	Dispatcher
//=======================================================================
#define DISP_EXCONT		(1)
#define DISP_EXSTOP		(0)
#define DISP_DR_YES   (1)
#define DISP_DR_NOT		(0)
//=======================================================================
//	JS: Dispatcher class
//	Used by Situation to dispatch TimeSlice and Draw
//=======================================================================
//	Executable slot
//=======================================================================
struct DSP_EXEC {
	char	lock;							  // TimeSlice lock
	char  exec;								// Time slice state
	char  draw;								// Draw state
	CExecutable *obj;					// Executable object
	};
//=======================================================================
//	Dispatcher
//=======================================================================
class CDispatcher {
	//--- ATTRIBUTES --------------------------------------
	DSP_EXEC	slot[PRIO_MAX];
	//----METHODS -----------------------------------------
public: 
CDispatcher::CDispatcher();
	void Enter(CExecutable *x, char p, char s,char d)	
		{	slot[p].obj	= x; slot[p].exec = s;	slot[p].draw = d;}

	void Clear(char p)
		{	slot[p].obj	= 0;
		}
	//-----------------------------------------------------
	void Store(CExecutable *ex, char p);
	void Remove (char p);
  //-----------------------------------------------------
	void Lock  (char p)						{slot[p].lock++;}
	void Unlock(char p)						{slot[p].lock--;}
	void Modlock(char p,char m)		{slot[p].lock += m;}
	void DrawON (char p)					{slot[p].draw = 1;}
	void DrawOFF(char p)					{slot[p].draw = 0;}
	void ExecON (char p)					{slot[p].exec = 1;}
	void ExecLOK(char p)					{slot[p].lock++;}
	void ExecULK(char p)					{slot[p].lock--;}
	void ExecOFF (char p)					{slot[p].exec = 0;}
	//---- Time slice -------------------------------------
	void	TimeSlice(float dT,U_INT frame);
	void	Draw(char p);
	void  DrawExternal();
	//-----------------------------------------------------
};
//==============================================================================
// Global variables
//
// All globals are contained within a master global data structure defined below
//==============================================================================
typedef struct {
	//--- Frame number ------------------------------------------------
	U_INT			Frame;
	BBM				mBox;												// Master black box
	//--- Dispatcher --------------------------------------------------
	CDispatcher Disp;
  //----World position ----------------------------------------------
	double		aMax;		// Altitude maximum
	double		exf;		// feet factor expension at aircraft position
	double		rdf;		// feet factor reduction at aircraft position
	SPosition geop;		// Position (lat/lon/alt) of the aircraft
  SVector   iang;   // Inertial angular orientation in radian
  SVector   dang;   // Same as above in degre
  double    magDEV; // Magnetic deviation at position
  int       NbVTX;  // Number of vertices
  //----APPLICATION PROFILE -----------------------------------------
	COption			aPROF;										// Application profile
	U_INT				iPROF;										// Initial profile
  U_CHAR      noAWT;                    // No animated water
  U_CHAR      noTER;                    // No terrain
  U_CHAR      noEXT;                    // No external aircraft
	U_CHAR			noINT;										// No internal aircarft
  U_CHAR      noOBJ;                    // No Objects
	U_CHAR      noOSM;										// No OSM objects
  U_CHAR      noAPT;                    // No Airport
  U_CHAR      noMET;                    // No Meteo
  CWobj       *wObj;										// Current object in focus
  //----SQL database detected ---------------------------------------
  char        genDB;                    // Generic database
  char        elvDB;                    // Elevation database
  char        seaDB;                    // Coast database
  char        txyDB;                    // Taxiway database
  char        m3dDB;                    // 3D model database
  char        texDB;                    // Terra texture database
  char        objDB;                    // World Object
	char				t2dDB;										// Texture 2D database
	//----VMAP Parameters --------------------------------------------
  char        MapName[SLOT_NAME_DIM];   /// Selected Map Name
  char        NulChar;                  /// Null Char
  char        FlyRoot[PATH_MAX];				/// JSDEV* remember Fly Root
  U_INT	      LoFrame;								  /// Lower frame
  U_INT	      HiFrame;								  /// Upper frame
  CBitmap    *nBitmap;                  /// Null Bitmap
  CBitmapNUL  nBmap;                    /// Bitmap NUL
  //--- Application state ---------------------------------------------
	char				stop;											// Request to stop
  EAppState appState;
  //--- Cockpit panel parameters --------------------------------------
  int         panelScrollStep, panelCreepStep;
  //---- POD filesystems for scenery and all other files -------------
  PFS               pfs;
	//--- Utility counters ---------------------------------------------
	U_INT							cnDOB;							// Deleted objects for one QGT
	U_INT							NbOBJ;							// Total Objects in scene
	U_INT							NbMOD;							// Total Models
	U_INT							NbPOL;							// Total polygons
	U_INT							NbCLN;							// Total coast lines
	U_INT							NbBMP;							// Bitmap number
  //--- Global indicators --------------------------------------------
  U_CHAR             ttr;               // Temporary trace
  U_CHAR            dBug;               // Subsystem debuging on
  U_CHAR             tod;               // Time of Day ('N' or 'D');
  U_CHAR            init;               // Initial time
	U_CHAR						zero;								// Clear Screen
	//---------Timer for everybody -------------------------------------
  float             dRT;                // Real time delta from last frame
  float             dST;                // Simu time delta from last Frame
  //---------Interface with master radio -----------------------------
  LND_DATA         *lnd;								// Landing data
	//--- OSM interface ------------------------------------------------
	U_INT							xobj;								// Last binded texture
	OSM_Object       *osmS;								// Selected object
  //---------Globals cameras parameters-------------------------------
  CCameraRunway         *cap;           // Camera airport
  CCameraSpot           *csp;           // spot camera
  CCamera               *cam;           // Current camera
  char                inside;           //  True when inside view
	//-------- Situation file ------------------------------------------
	char                  sitFilename[PATH_MAX];  /// Filename of .SIT file to load
  //-------- Various global objects ----------------------------------
	CBuilder					    *trn;
  CClock                *clk;           // Global clock
  CTimeManager          *tim;           // Time manager
  CCursorManager        *cum;           // Cursor manager           
  CTestBed              *tsb;           // Test bed
  CJoysticksManager     *jsm;           // Joystick handler
  CSituation            *sit;           // Current situation
  CDbCacheMgr		        *dbc;						// Cache database manager
  TCacheMGR             *tcm;           // Terrain cache manager
  CTextureWard          *txw;           // Texture Hangar
  CAirportMgr           *apm;           // Airport Manager
  C3DMgr                *m3d;           // 3D world manager
  CExport               *exm;           // Export manager
  CImport               *imp;           // Import manager
  CKeyMap               *kbd;           // Keyboard  interface
  CFuiManager           *fui;           // Windows manager
  CSkyManager           *skm;           // Sky manager
  CWeatherManager       *wtm;           // Weather manager
  CMagneticModel        *mag;           // Magnetic model
  CCloudSystem          *cld;           // Cloud System
  SqlMGR                *sqm;           // SQL Manager
  SqlTHREAD             *sql;           // SQL Thread
  CSlewManager          *slw;           // Slew manager
  CAtmosphereModelJSBSim *atm;          // Atmosphere
  CAudioManager         *snd;           // Sound manager
  CAirplane             *pln;           // Plane object
	CSceneryDBM						*scn;						// Scenery set
  //-------Aircraft items --------------------------------------------
  CCameraManager        *ccm;           /// Current camera manager
  CFuiFuel              *wfl;           /// Fuel window
  CFuiLoad              *wld;           /// Load window
  CFuiProbe             *wpb;           // Window probe
	//--- Included items -----------------------------------------------
  CFuiRadioBand          rdb;           // Radio band
  //--- QGT indices fro aircraft actual position ---------------------
  U_INT             qgtX;               // X index
  U_INT             qgtZ;               // Z index
  //------- Limit parameters -----------------------------------------
  float             inf3DO;             // 3D detect ring in feet
  float             dwf3DO;             // 3D Drawing limit in feet
  float             ftLD1;              // 3D limit for LOD1 (1/2)
  float             ftLD2;              // 3D limit for LOD2 (1/4)
  float             ftLD3;              // 3D limit for LOD3 (1/8)
	float							osmIN;							// OSM objects intro distance
	U_INT							osmLF;							// Object load factor from DB
	U_INT							osmMX;							// Maximum osm object loaded
  //------- Default fonts  -------------------------------------------
  SFonts            fonts;
  //-------- Default logs  -------------------------------------------
  CLogFile          *logDebug;
  CLogFile          *logWarning;
  CLogFile			    *logTrace;
  CLogFile          *logTerra;
	CLogFile					*logScene;
	CLogFile          *logStreet;
  //-------- Internal counters for tests -----------------------------
  U_INT              cnt1;
  U_INT              cnt2;
  //------------------------------------------------------------------
  // sdk: Plugin manager
  CPluginMain			*plugins;
  int              plugins_num;
  // sdk: Main menu additions if any  // 
  sdkmenu::CSDKMenu sdk_menu;
  //-----Screen parameters -------------------------------------------
  double         nearP;         // Near plane in feet
  double         afarP;         // Far  distance
  double         fovX;          // Field of view
  double         aspect;        // Aspect ratio
  double         pitch;         // pixel pitch
  double         camRange;      // Camera initial range
	//--- Elevation tracker ---------------------------------------------
	CElvTracker		 etrk;
  //-----Terrain parameters -------------------------------------------
  double         maxView;       // Maximum view
  double         highRAT;       // Hight resolution ratio
  double         skyDist;       // Sky distance
	U_INT						pakCAP;				// Super Tile buffer
	U_CHAR					mipOBJ;				// Mip Level for 3d object
	U_CHAR					mipTER;				// Mip level for terrain
	//----------------------------------------------------------------
  sScreenParams *cScreen;       ///< Current screen parameters
                                ///<  (pointer changed dynamically)
  sScreenParams  mScreen;       ///< Primary screen parameters
  sScreenParams  sScreen;       ///< Secondary screen parameters
  //-------------Probe parameters ------------------------------------
  CSubsystem    *psys;           /// Current probed subsystem
  //-------------Options ---------------------------------------------
	COption       Trace;					// Trace options
  COption       aptOpt;         // Airport options
  COption       scnOpt;         // Scenary options
  COption       vmpOpt;					// VMap    options
	COption       evnOpt;					// Event 
  //-------------Vector map parameter --------------------------------
  float         vmapZoom;   // Vector map zoom
  U_INT         vmapTrns;   // Transparency
  TC_SPOINT     vmapScrn;   // vmap screen
  //-------------Global databank addresses ---------------------------
  STIME_ZONE *tmzTAB;           // Time zones
  char      **comTAB;           // Com type
  char      **cmaTAB;           // Com abreviate table
  char      **abcTAB;           // Standard allphabet
  char      **vorTAB;           // Vor type name
  char      **wptTAB;           // WAYPOINT name
  char      **gazTAB;           // Gaz table
  char      **monTAB;           // Month table
  char      **ownTAB;           // Ownership table
  char      **rwgTAB;           // Runway ground
  TC_TERRA   *terTAB;           // Terra data
  // GL compressed texture support
  bool      dxt1Supported;      ///< Compressed texture format DXT1 supported
  bool      dxt1aSupported;     ///< Compressed texture format DXT1 with alpha supported
  bool      dxt3Supported;      ///< Compressed texture format DXT3 supported
  bool      dxt5Supported;      ///< Compressed texture format DXT5 supported
	//--- OPAL simulation --------------------------------------------
	float			TimLim;								// Time limit for simulation
  opal::Solid *Ground;                                              // Ground solid
  opal::Simulator *opal_sim;
  bool  fps_limiter;
  char  sBar;
  float status_bar_limit;

  /// PHY file used


} SGlobals;

//=========================================================================
extern SGlobals *globals;   // Declared in Main.cpp
//==========================================================================
void		InitialProfile();
void		SpecialProfile(Tag wnd,U_INT p);
//================ END OF FILE =========================================================
#endif // GLOBALS_H


