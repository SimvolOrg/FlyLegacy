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

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlyLegacy.h"
#include "Pod.h"
#include "Situation.h"
#include "DrawVehiclePosition.h"
#include "DrawVehicleSmoke.h"
#include "..\Include\BlackBox.h"
#include "LogFile.h"
#include "..\Plugin\Plugin.h"   // 
#include "Ui.h"                 // sdk:
//=============================================================================================
class CFmtxMap;
//=============================================================================================
//	Define APPLICATION PROFILE
//	APPLICATION PROFILE is used by specifics windows like editors.
//	Option to prevent drawing and changing camera are set
//=============================================================================================
#define PROF_NO_TER			(0x00000001)				// No terrain drawing
#define PROF_NO_APT			(0x00000002)				// No airport drawing
#define PROF_NO_INT			(0x00000004)				// No internal plane drawing
#define PROF_NO_EXT			(0x00000008)				// No external plane drawing
#define PROF_NO_OBJ			(0x00000010)				// No Object drawing
#define PROF_NO_MET			(0x00000020)				// No Meteo effect
#define PROF_NO_FPL     (0x00000040)				// No flight plan
#define PROF_NO_PLN			(0x00000080)				// No aircraft
#define PROF_RABIT			(0x01000000)				// Rabbit camera
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
#define PROF_EDITOR (PROF_NO_INT+PROF_NO_EXT+PROF_NO_MET+PROF_DR_DET+PROF_RABIT+PROF_NO_FPL+PROF_NO_PLN)
//=======================================================================================
//  Edit column for statistics
//=======================================================================================
#define STATS_NUM (20)
//=======================================================================================
//  AIRPORT OPTIONS
//=======================================================================================
#define APT_OPT_ALIT          (0x00000001)        // Alight airport
//=======================================================================================

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
//  AIRCRAFT OPTION
//-----------------------------------------------------------------
typedef enum {
  VEH_AP_DISC   = 0x0001,           // Autopilot autodisconnect
  VEH_AP_LAND   = 0x0002,           // Autopilot autoland
  VEH_AP_OPTN   = 0x0003,           // Both option
  VEH_PN_HELP   = 0x0004,           // Panel Help
  VEH_DW_SMOK   = 0x0010,           // Draw vehicle smoke
  VEH_DW_SHAD   = 0x0020,           // Draw vehicle shadow
  VEH_DW_VPOS   = 0x0040,           // Draw vehicle position
  VEH_DW_AERO   = 0x0080,           // Draw vehicle aero vectors
  VEH_D_CRASH   = 0x0100,           // Crash detector 
} VEH_OPTION;
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
//==============================================================================
// Global variables
//
// All globals are contained within a master global data structure defined below
//==============================================================================
typedef struct {
	//--- Frame number ------------------------------------------------
	U_INT			Frame;
	char     *mdule;											// Current module
	BBM				mBox;												// Master black box
  //----World position ----------------------------------------------
	double		aMax;		// Altitude maximum
  SPosition geop;   // Position (lat/lon/alt) of the aircraft
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
  //-----------------------------------------------------------------
  char        MapName[SLOT_NAME_DIM];   /// Selected Map Name
  char        NulChar;                  /// Null Char
  char        FlyRoot[PATH_MAX];				/// JSDEV* remember Fly Root
  U_INT	      LoFrame;								  /// Lower frame
  U_INT	      HiFrame;								  /// Upper frame
  CBitmap    *nBitmap;                  /// Null Bitmap
  CBitmapNUL  nBmap;                    /// Bitmap NUL
  //--- Application state ---------------------------------------------
  EAppState appState;
  //--- Cockpit panel parameters --------------------------------------
  int         panelScrollStep, panelCreepStep;
  CPanel     *panel;                   /// Current panel in building for gauge
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
  U_CHAR            rfua;               // reserved futur use
  //---------Timer for everybody -------------------------------------
  float             dRT;                // Real time delta from last frame
  float             dST;                // Simu time delta from last Frame
  //---------Interface with master radio -----------------------------
  BUS_RADIO        *Radio;              // Radio block
  CILS             *cILS;               // Current ILS
  //---------Globals cameras parameters-------------------------------
  CCameraRunway         *cap;           // Camera airport
  CCameraSpot           *csp;           // spot camera
  CCamera               *cam;           // Current camera
  char                inside;           //  True when inside view
  char                  bTyp;           // Drawing type for Aircraft blue print
	//-------- Situation file ------------------------------------------
	char                  sitFilename[PATH_MAX];  /// Filename of .SIT file to load
  //-------- Various global objects ----------------------------------
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
  CPanel                *pan;           /// Active panel
  CFuelSystem           *gas;           /// Fuel system
  CCockpitManager       *pit;           /// Cockpit manager
  CFuiFuel              *wfl;           /// Fuel window
  CFuiLoad              *wld;           /// Load window
  CFuiRadioBand         *rdb;           // Radio band
  CFuiProbe             *wpb;           // Window probe
  //--- QGT indices fro aircraft actual position ---------------------
  U_INT             qgtX;               // X index
  U_INT             qgtZ;               // Z index
  //------- Limit parameters -----------------------------------------
  float             nmDET;              // 3D detect ring in miles
  float             ftDET;              // 3D detect ring in feet
  float             nmDRW;              // 3D Drawing limit in miles
  float             ftDRW;              // 3D Drawing limit in feet
  float             ftLD1;              // 3D limit for LOD1 (1/2)
  float             ftLD2;              // 3D limit for LOD2 (1/4)
  float             ftLD3;              // 3D limit for LOD3 (1/8)
  //------- Default fonts  -------------------------------------------
  SFonts            fonts;
  //-------- Default logs  -------------------------------------------
  CLogFile          *logDebug;
  CLogFile          *logWarning;
  CLogFile			    *logTrace;
  CLogFile          *logTerra;
	CLogFile					*logScene;
  //-------- Internal counters for tests -----------------------------
  U_INT              cnt1;
  U_INT              cnt2;
  //------------------------------------------------------------------
  // sdk: Plugin manager
  ut::CPluginMain   plugins;
  int               plugins_num;
  // sdk: Main menu additions if any  // 
  sdkmenu::CSDKMenu sdk_menu;
  //-----Screen parameters -------------------------------------------
  double         nearP;         // Near plane in feet
  double         afarP;         // Far  distance
  double         fovX;          // Field of view
  double         aspect;        // Aspect ratio
  double         pitch;         // pixel pitch
  double         camRange;      // Camera initial range
  //-----Terrain parameters -------------------------------------------
  double         maxView;       // Maximum view
  double         highRAT;       // Hight resolution ratio
  double         skyDist;       // Sky distance
  //-----Screen management --------------------------------------------
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

#ifdef HAVE_OPAL
  opal::Simulator *opal_sim;
#endif
  bool  simulation;
  bool  fps_limiter;
  char  sBar;
  float status_bar_limit;

  /// PHY file used

  /// AERODYMANICS CAGING
  bool  caging_fixed_sped;  float  caging_fixed_sped_fval;
  bool  caging_fixed_moi;    
  bool  caging_fixed_pitch; double caging_fixed_pitch_dval;
  bool  caging_fixed_roll;  double caging_fixed_roll_dval;
  bool  caging_fixed_yaw;   double caging_fixed_yaw_dval;
  bool  caging_fixed_alt;
  bool  caging_fixed_wings;

  USHORT random_flag;
  int   num_of_autogen;     // num_of_autogen objects
} SGlobals;

extern SGlobals *globals;   // Declared in Main.cpp
//==========================================================================
void		InitialProfile();
void		SpecialProfile(Tag wnd,U_INT p);
//================ END OF FILE =========================================================
#endif // GLOBALS_H


