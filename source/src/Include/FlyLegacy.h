/*
 * FlyLegacy.h
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

/*! \file FlyLegacy.h
 *  \brief Top-level header file for application
 */

/*
 * Doxygen documentation style
 *
 * Code documentation is generated automatically by Doxygen.  Special comments
 *   should be provided for all classes and methods, standalone functions and
 *   all files.
 * 
 * Note on standardized documentation styles for this project:
 *
 * - All comment blocks should use Qt style notation
 * - All structural commands should use the backslash leading character
 * - All brief descriptions should be grouped into the same comment block as
 *     the detailed description, and use the \brief tag
 * - All detailed descriptions should be grouped into the same comment block as
 *     the brief description
 * - Documentation of class members that follow the member should use //!<
 */
//=================================================================================

//=================================================================================
#ifndef FLYLEGACY_H
#define FLYLEGACY_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

// Disable MS Visual C++ warnings
#ifdef _MSC_VER
#pragma warning(disable:4100)    // Formal parameter unused
#pragma warning(disable:4505)    // Unreferenced local function
#pragma warning(disable:4706)    // Assignment in conditional statement
#pragma warning(disable:4710)    // Function not inlined
#pragma warning(disable:4189)    // Local variable intialized but not used
#pragma warning(disable:4201)    // Nameless struct/union
#pragma warning(disable:4244)    // Data conversion, possible loss of data
#pragma warning(disable:4005)    // Formal parameter unused
#ifdef _DEBUG
#pragma warning(disable:4786)    // Browse file name truncation warnings
#endif // _DEBUG
#endif // _MSC_VER

// GLEW must be included before windows.h
#include <GL/glew.h>
#include <GL/glu.h>
#define FREEGLUT_STATIC
#include <GL/glut.h>
// --- JS: Already defined in freeglut.h
//#ifdef _WIN32
//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
//#endif // _WIN32

#include "Version.h"
#if defined(_DEBUG) && defined(HAVE_CRTDBG_H)
#include <crtdbg.h>
#endif

#ifdef HAVE_OPAL
// OPAL STUFF FOR WINDOWS AND VISUAL STUDIO 2008
  #define OPAL_USE_DOUBLE
  #include <opal.h>
  //extern opal::JointData gjointData;
#endif

//
// STL Includes
//
#include <string>
#include <pthread.h>
//
// Function portability
//
#ifndef HAVE_STRICMP
#ifdef HAVE_STRCASECMP
#define stricmp strcasecmp
#else
#error No case-independent string comparison (stricmp, strcasecmp) available.
#endif
#endif // HAVE_STRICMP
#ifdef NEED_GLUT_DISABLE_ATEXIT_HACK
#define GLUT_DISABLE_ATEXIT_HACK
#endif // NEED_GLUT_DISABLE_ATEXIT_HACK

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif
#ifndef SAFE_FREE
#define SAFE_FREE(p)         { if(p) { free (p);       (p)=NULL; } }
#endif

// Generate warning if OpenGL error has occurred
#define CHECK_OPENGL_ERROR \
  { \
    GLenum e = glGetError (); \
    if (e != GL_NO_ERROR)     \
        WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e)); \
  }

//
// Workarounds for MS Visual Studio 2005 deprecated functions
//
#if _MSC_VER >= 1300
#define stricmp _stricmp
#define itoa _itoa
#endif // MSC_VER >= 1300

//
// Absolute maximum path + filename + extension for a file name
//
#ifndef PATH_MAX
#ifdef _PATH_MAX
#define PATH_MAX _PATH_MAX
#else
#define PATH_MAX 260
#define FNAM_MAX (PATH_MAX-1)
#endif
#endif // PATH_MAX

#include "Pod.h"

//
// Maximum size of filename only (no path components)
//
#ifndef FILENAME_MAX
#define FILENAME_MAX 260
#endif

//
// Memory leak marker
//
#ifdef _DEBUG
#define MEMORY_LEAK_MARKER(s) { char *mark = new char[32]; strncpy (mark, s,31), mark[31] = 0; }
#else
#define MEMORY_LEAK_MARKER(s) {}
#endif

//
// Microsoft Windows headers include standard integer types in header basetsd.h
// If this header was not processed, include standard integer types from stdint.h
//
#ifndef _BASETSD_H_
#include "../Include/stdint.h"
typedef uint8_t    UINT8;
typedef int8_t     INT8;
typedef uint16_t   UINT16;
typedef int16_t    INT16;
typedef uint32_t   UINT32;
typedef int32_t    INT32;
#endif // _BASETSD_H
//==============================================================================
//
// Basic object classes
//==============================================================================
typedef unsigned long Tag;
typedef unsigned long   U_LONG;       // JSDEV*
typedef unsigned int	  U_INT;				// JSDEV*
typedef unsigned short	U_SHORT;			// JSDEV*
typedef unsigned char   U_CHAR;       // JSDEV*
//------------------------------------------------------------------------------
//	JSDEV* add macro to extract character from integer
//------------------------------------------------------------------------------
#define CHAR_N1(X)	((char)(X >> 24))
#define CHAR_N2(X)  ((char)(X >> 16))
#define CHAR_N3(X)  ((char)(X >>  8))
#define CHAR_N4(X)  ((char)(X      ))
//------------------------------------------------------------------------------
//  Number of entries in DbCache for objects on screen
//------------------------------------------------------------------------------
#define NBR_OBJ_DBC 200           
/////////////////////////////////////////////////////////////
//
//  Constants/enums
//
/////////////////////////////////////////////////////////////
//-------Globals terrain constants --------------------------
#define COVERED_RADIUS 140
#define ARCSEC_PER_GLOBE_TILE           (1.296E+6 / 256.0)
#define ARCSEC_PER_QUARTER_GLOBE_TILE   (ARCSEC_PER_GLOBE_TILE / 2.0)
#define ARCSEC_PER_SUPER_TILE           (ARCSEC_PER_QUARTER_GLOBE_TILE / 8.0)
#define ARCSEC_PER_DETAIL_TILE          (ARCSEC_PER_QUARTER_GLOBE_TILE /32.0)
//-------Globals physics constants --------------------------
const float ADJ_DRAG_COEFF = 1.000000f; /// adjustCoeffOfDrag
const float ADJ_LIFT_COEFF = 1.000000f; /// adjustCoeffOfLift
const float ADJ_MMNT_COEFF = 1.000000f; /// adjustCoeffOfMoment
const float ADJ_DHDL_COEFF = 500.0000f; /// adjustDihedralCoeff
const float ADJ_ENGN_THRST = 1.600000f; /// adjustEngineThrust
const float ADJ_PTCH_COEFF = 10.00000f; /// adjustPitchCoeff
const float ADJ_PTCH_MINE  = 1.000000f; /// adjustPitchMine
const float ADJ_ROLL_MINE  = 2.000000f; /// adjustRollMine
const float ADJ_STRG_CONST = 0.500000f; /// adjustSteeringConst
const float ADJ_BRAK_CONST = 1.000000f; /// adjustBrakingConst
const float ADJ_DIFF_CONST = 1.000000f; /// adjustDiffBrakingConst
const float ADJ_SINK_RATE  = 5.000000f; /// adjust crash sink rate
const float ADJ_CMPR_LNGT  = 2.000000f; /// adjust crash compression length 
const float ADJ_POWL_CNST  = 5.000000f; /// adjust crash WHL <powl>
const float ADJ_YAW_MINE   = 2.500000f; /// adjustYawMine
const float ADJ_AILR_FORCE = 4.750000f; /// jsAileronForceCoeff
const float ADJ_AILR_DRAG  = 0.250000f; /// jsAileronDragCoeff
const float ADJ_RUDR_FORCE = 1.650000f; /// jsRudderForceCoeff

const float ADJ_AERO_CENTR = 0.000000f; /// aero center for aeromodel
const float ADJ_ANGL_DAMPG = 0.450000f; /// OPAL angular damping
const float ADJ_GRND_FRCTN = 0.250000f; /// OPAL ground friction
const float ADJ_LINR_DAMPG = 0.017000f; /// OPAl linear damping
const float ADJ_STEP_SIZE  = 0.040000f; /// OPAl step size
const float ADJ_TOTL_DRAG  = 1.000000f; /// OPAL total drag
const int   INITIAL_SPEED  = 1;         /// initial speed y/n
const int   HAS_FAKE_ENG   = 0;         /// has fake engine
const int   WIND_EFFECT    = 1        ; /// wind effect
const int   GRND_EFFECT    = 1        ; /// ground effect
const float INIT_SPD_X     = 0.000000f; /// initial speed x
const float INIT_SPD_Y     = 0.000000f; /// initial speed y
const float INIT_SPD_Z     = 60.00000f; /// initial speed z

const float ADJ_IXX_COEFF  = 1.760000f; /// propeller inertia
const float ADJ_GEAR_RATIO = 0.150000f; /// propeller gear ratio
const float ADJ_GRND_BANK  = 10.00000f; /// fix for ground banking
const float ADJ_WIND_COEFF = 10.00000f; /// wind effect coeff   
const float ADJ_GEAR_DRAG  = 0.975000f; /// drag from gear

//
// Overall application state
//

typedef enum {
  APP_SPLASH_SCREEN,
  APP_TEST,
  APP_INIT,
  APP_GENERATE_SITUATION,
  APP_LOADING_SCREEN,
  APP_LOAD_SITUATION,
  APP_PREPARE,
	APP_TELEPORT,
  APP_SIMULATION,
  APP_EXIT_SCREEN,
  APP_EXIT,
  APP_EXPORT
} EAppState;

enum EMessageDataType
{
  TYPE_NONE = '    ',
  TYPE_CHAR = 'char',
  TYPE_CHARPTR = 'cptr',
  TYPE_INT = 'int ',
  TYPE_INTPTR = 'iptr',
  TYPE_REAL = 'real',
  TYPE_REALPTR = 'rptr',
  TYPE_VOID = 'void',
};

enum EMessageResult
{
  MSG_IGNORED   = 0,    // message was passed over
  MSG_PROCESSED = 1,    // message was processed (set/get) and should stop broadcast
  MSG_USED      = 2,    // message was used indirectly, but should continue to other objects
  MSG_ACK       = 3,    // Message is acknoledged
};

enum EMessageRequest
{
  MSG_UNKNOWN = 'none',
  MSG_GETDATA = 'getd',
  MSG_SETDATA = 'setd',
  MSG_DATACHANGED = 'chan',
  MSG_REFERENCE = 'refr',
  MSG_TRIGGER = 'trig',
  MSG_OBJECTDYING = 'die ',
  MSG_CAMERA = 'aCam'
};

enum EMessageHWType
{
  HW_UNKNOWN  = 0,
  HW_BUS    = 1,
  HW_SWITCH = 2,
  HW_LIGHT  = 3,
  HW_FUSE   = 4,
  HW_STATE  = 5,
  HW_GAUGE  = 6,
  HW_OTHER  = 7,
  HW_CIRCUIT  = 8,
  HW_RADIO  = 9,
  HW_FLAP   = 10,
  HW_HILIFT = 11,
  HW_GEAR   = 12,
  HW_BATTERY  = 13,
  HW_ALTERNATOR = 14,
  HW_ANNUNCIATOR  = 15,
  HW_GENERATOR  = 16,
  HW_CONTACTOR  = 17,
  HW_SOUNDFX  = 18,
  HW_FLASHER  = 19,
  HW_INVERTER = 20,
  HW_PID      = 21,
  HW_ENGINE   = 22,
  HW_WHEEL    = 23,
  HW_SPECIAL  = 24,
  HW_PROP     = 25,
  HW_PISTON   = 26,
  //---To increase, modify the following lines and 
  //  add one item to the hardware table hwTAB in FuiPart.cpp
  HW_MAX      = 27,
  HW_UNITLESS = 101,
  HW_UNBENT = 102,
  HW_SCALE  = 103
};

//
// The following enum lists all allowable object types that can be
//   instantiated by a third-party DLL module.
//

enum EDLLObjectType
{
  TYPE_DLL_GAUGE    = 'gage',
  TYPE_DLL_SYSTEM   = 'subs',
  TYPE_DLL_WORLDOBJECT  = 'wobj',
  TYPE_DLL_WINDOW   = 'wind',
  TYPE_DLL_CAMERA   = 'came',
  TYPE_DLL_CONTROL  = 'cont',
  TYPE_DLL_VIEW   = 'view'
};

//
// All possible types of <wobj> objects
//

enum EFlyObjectType
{
  TYPE_FLY_UNKNOWN        = 0,
  TYPE_FLY_WORLDOBJECT    = 'wobj',
  TYPE_FLY_MODELOBJECT    = 'mobj',
  TYPE_FLY_SIMULATEDOBJECT= 'sobj',
  TYPE_FLY_VEHICLE        = 'vehi',
  TYPE_FLY_GROUNDVEHICLE  = 'gveh',
  TYPE_USER_AIRCRAFT      = 'plan',
	TYPE_ANIM_AIRCRAFT		  = 'apln',
	TYPE_FLY_UFO						= '_ufo',
  TYPE_FLY_HELICOPTER     = 'heli'
};
enum EBitmapType
{ TYPE_DUMMY  = 'nnnn',
  TYPE_PBM    = 'pbm',
  TYPE_PBG    = 'pbg ',
  TYPE_BMP    = 'bmp ',
  TYPE_TIF    = 'tif ',
  TYPE_NULL   = 'null',
  TYPE_INVALID  = TYPE_NULL
};

enum EStreamTagResult 
{
  TAG_IGNORED = 0,
  TAG_READ = 1,
  TAG_EXIT = 2,
	TAG_STOP = 3
};

enum EClickResult
{
  MOUSE_TRACKING_OFF = 0,
  MOUSE_TRACKING_ON = 1
};

enum ECursorResult
{
  CURSOR_NOT_CHANGED = 0,
  CURSOR_WAS_CHANGED = 1
};

enum ECameraType
{
  CAMERA_INVALID    = 0,
  CAMERA_COCKPIT    = 'cock',
  CAMERA_SPOT       = 'spot',
  CAMERA_OBSERVER   = 'obsr',
  CAMERA_FLYBY      = 'flyb',
  CAMERA_TOWER      = 'towr',
  CAMERA_OVERHEAD   = 'over',
  CAMERA_ORBIT      = 'orbt',
	CAMERA_RABBIT		  = 'rabt'
};

//
// Type of pitot-static port, used in CPitotStaticPort (Subsystems)
//
typedef enum {
  PORT_PITOT,
  PORT_STATIC
} EPortType;

//
// Facing of pitot-static port, used in CPitotStaticPort (Subsystems)
//
typedef enum {
  PORT_LEFT,
  PORT_RIGHT
} EPortFace;


/*
 * Database enumerated types
 *
 * The following enumerated types are all related to the default
 *   databases of airports, runways, navigational aids, etc.
 *
 * Many have their origins in original DAFIF data from which the
 *   databases were constructed.
 */
enum EIlsTypes 
{ ILS_DME         = 0x00000010,   // 16
  ILS_LOCATOR     = 0x00000020,   // 32
  ILS_LOCALIZER   = 0x00000040,   // 64
  ILS_GLIDESLOPE  = 0x00000080,   // 128
  ILS_BACKCOURSE  = 0x00000080,   // 128
  ILS_INNERMARK   = 0x00000200,   // 512
  ILS_MIDDLMARK   = 0x00000400,   // 1024
  ILS_OUTERMARK   = 0x00000800,   // 2048
};
enum ENavaidTypes
{
  NAVAID_TYPE_UNKNOWN   = 0,
  NAVAID_TYPE_VOR     = (1 << 0),
  NAVAID_TYPE_TACAN   = (1 << 2),
  NAVAID_TYPE_NDB     = (1 << 3),
  NAVAID_TYPE_DME     = (1 << 4),
  NAVAID_TYPE_LOCATOR   = (1 << 5),
  NAVAID_TYPE_LOCALIZER   = (1 << 6),
  NAVAID_TYPE_GS      = (1 << 7),
  NAVAID_TYPE_BACKCOURSE    = (1 << 8),
  NAVAID_TYPE_INNERMARKER   = (1 << 9),
  NAVAID_TYPE_MIDDLEMARKER  = (1 << 10),
  NAVAID_TYPE_OUTERMARKER   = (1 << 11),
  NAVAID_TYPE_WAYPOINT    = (1 << 12),
  NAVAID_TYPE_VOT     = (1 << 13),
  NAVAID_TYPE_VORTAC    = (NAVAID_TYPE_VOR | NAVAID_TYPE_TACAN),
  NAVAID_TYPE_VORDME    = (NAVAID_TYPE_VOR | NAVAID_TYPE_DME),
  NAVAID_TYPE_NDBDME    = (NAVAID_TYPE_NDB | NAVAID_TYPE_DME),
  NAVAID_TYPE_ILSDME    = (NAVAID_TYPE_LOCALIZER | NAVAID_TYPE_DME)
};

enum ENavaidClasses
{
  NAVAID_CLASS_UNKNOWN    = 0,
  NAVAID_CLASS_HIGH       = 1,
  NAVAID_CLASS_LOW        = 2,
  NAVAID_CLASS_TERMINAL   = 3,
  NAVAID_CLASS_MEDIUMHOMING = 4,
  NAVAID_CLASS_HIGHHOMING   = 5,
  NAVAID_CLASS_ANTICIPATED  = 6
};

enum ENavaidUsage 
{
  NAVAID_USAGE_UNKNOWN  = 0,
  NAVAID_USAGE_HIGH = 1,
  NAVAID_USAGE_LOW  = 2,
  NAVAID_USAGE_BOTH = 3,
  NAVAID_USAGE_TERMINAL = 4,
  NAVAID_USAGE_RNAV = 5
};

enum ECommTypes
{
  COMM_UNKNOWN= 0,
  COMM_TOWER = (1 << 0),
  COMM_CLEARANCEDELIVERY = (1 << 1),
  COMM_GROUNDCONTROL = (1 << 2),
  COMM_APPROACHCONTROL = (1 << 3),
  COMM_DEPARTURECONTROL = (1 << 4),
  COMM_FLIGHTSERVICESTATION = (1 << 5),
  COMM_CENTER = (1 << 6),
  COMM_ATIS = (1 << 7),
  COMM_CTAF = (1 << 8),
  COMM_MULTICOM = (1 << 9),
  COMM_EMERGENCY = (1 << 10),
  COMM_ALL = (-1),
};

enum ENavWaypointUsage 
{
  WAYPOINT_USAGE_UNKNOWN = 0,
  WAYPOINT_USAGE_HIGH = 1,
  WAYPOINT_USAGE_LOW = 2,
  WAYPOINT_USAGE_BOTH = 3,
  WAYPOINT_USAGE_TERMINAL = 4,
  WAYPOINT_USAGE_HIGHALTRNAV = 5
};

enum ENavWaypointTypes
{
  WAYPOINT_TYPE_UNNAMED = (1 << 0),
  WAYPOINT_TYPE_NDB = (1 << 1),
  WAYPOINT_TYPE_NAMED = (1 << 2),
  WAYPOINT_TYPE_OFFROUTE = (1 << 3)
};

enum ETunedTypes
{
  TUNED_NONE = 0,
  TUNED_NAVAID = (1 << 0),
  TUNED_ILS = (1 << 1),
  TUNED_COMM = (1 << 2),
};

enum EAirportTypes
{
  AIRPORT_TYPE_UNKNOWN = 0,
  AIRPORT_TYPE_AIRPORT = (1 << 1),
  AIRPORT_TYPE_HELIPORT = (1 << 2),
  AIRPORT_TYPE_SEAPLANEBASE = (1 << 3),
  AIRPORT_TYPE_BALLOONPORT = (1 << 4),
  AIRPORT_TYPE_GLIDERPORT = (1 << 5),
  AIRPORT_TYPE_STOLPORT = (1 << 6),
  AIRPORT_TYPE_ULTRALIGHT = (1 << 7)
};

enum EAirportOwnership
{
  AIRPORT_OWNERSHIP_UNKNOWN = 0,
  AIRPORT_OWNERSHIP_PUBLIC = 1,
  AIRPORT_OWNERSHIP_PRIVATE = 2,
  AIRPORT_OWNERSHIP_AIRFORCE = 3,
  AIRPORT_OWNERSHIP_NAVY = 4,
  AIRPORT_OWNERSHIP_ARMY = 5
};

enum EAirportUsage
{
  AIRPORT_USAGE_UNKNOWN = 0,
  AIRPORT_USAGE_PUBLIC = 1,
  AIRPORT_USAGE_PRIVATE = 2
};

enum EAirportFrameService
{
  AIRPORT_FRAMESERVICE_UNKNOWN = 0,
  AIRPORT_FRAMESERVICE_NONE = 1,
  AIRPORT_FRAMESERVICE_MINOR = 2,
  AIRPORT_FRAMESERVICE_MAJOR = 3
};

enum EAirportEngineService
{
  AIRPORT_ENGINESERVICE_UNKNOWN = 0,
  AIRPORT_ENGINESERVICE_NONE = 0,
  AIRPORT_ENGINESERVICE_MINOR = 1,
  AIRPORT_ENGINESERVICE_MAJOR = 2
};

enum EAirportOxygen
{
  AIRPORT_OXYGEN_UNKNOWN = 0,
  AIRPORT_OXYGEN_NONE = 1,
  AIRPORT_OXYGEN_LOW = 2,
  AIRPORT_OXYGEN_HIGH = 3,
  AIRPORT_OXYGEN_HIGHLOW = 4
};

enum EAirportLensColor
{
  AIRPORT_BEACON_UNKNOWN = 0,
  AIRPORT_BEACON_NONE = 1,
  AIRPORT_BEACON_CLEARGREEN = 2,
  AIRPORT_BEACON_CLEARYELLOW = 3,
  AIRPORT_BEACON_CLEARGREENYELLOW = 4,
  AIRPORT_BEACON_SPLITCLEARGREEN = 5,
  AIRPORT_BEACON_YELLOW = 6,
  AIRPORT_BEACON_GREEN = 7
};

enum EAirportRegions
{
  AIRPORT_REGION_UNKNOWN = 0,
  AIRPORT_REGION_FAAALASKA = 1,
  AIRPORT_REGION_FAACENTRAL = 2,
  AIRPORT_REGION_FAAEASTERN = 3,
  AIRPORT_REGION_FAAGREATLAKES = 4,
  AIRPORT_REGION_FAAINTERNATIONAL = 5,
  AIRPORT_REGION_FAANEWENGLAND = 6,
  AIRPORT_REGION_FAANORTHWESTMOUNTAIN = 7,
  AIRPORT_REGION_FAASOUTHERN = 8,
  AIRPORT_REGION_FAASOUTHWEST = 9,
  AIRPORT_REGION_FAAWESTERNPACIFIC = 10
};

enum EAirportFuelTypes
{
  AIRPORT_FUEL_80 = (1 << 1),
  AIRPORT_FUEL_100 = (1 << 2),
  AIRPORT_FUEL_100LL = (1 << 3),
  AIRPORT_FUEL_115 = (1 << 4),
  AIRPORT_FUEL_JETA = (1 << 5),
  AIRPORT_FUEL_JETA1 = (1 << 6),
  AIRPORT_FUEL_JETA1PLUS = (1 << 7),
  AIRPORT_FUEL_JETB = (1 << 8),
  AIRPORT_FUEL_JETBPLUS = (1 << 9),
  AIRPORT_FUEL_AUTOMOTIVE = (1 << 10)
};

enum EAirportBasedAircraft
{
  AIRPORT_AIRCRAFT_SINGLEENGINE = (1 << 1),
  AIRPORT_AIRCRAFT_MULTIENGINE = (1 << 2),
  AIRPORT_AIRCRAFT_JETENGINE = (1 << 3),
  AIRPORT_AIRCRAFT_HELICOPTER = (1 << 4),
  AIRPORT_AIRCRAFT_GLIDERS = (1 << 5),
  AIRPORT_AIRCRAFT_MILITARY = (1 << 6),
  AIRPORT_AIRCRAFT_ULTRALIGHT = (1 << 7)
};

enum EAirportScheduleMonth
{
  AIRPORT_MONTH_JANUARY = (1 << 1),
  AIRPORT_MONTH_FEBRUARY = ( 1 << 2),
  AIRPORT_MONTH_MARCH = (1 << 3),
  AIRPORT_MONTH_APRIL = (1 << 4),
  AIRPORT_MONTH_MAY = (1 << 5),
  AIRPORT_MONTH_JUNE = (1 << 6),
  AIRPORT_MONTH_JULY = (1 << 7),
  AIRPORT_MONTH_AUGUST = (1 << 8),
  AIRPORT_MONTH_SEPTEMBER = (1 << 9),
  AIRPORT_MONTH_OCTOBER = (1 << 10),
  AIRPORT_MONTH_NOVEMBER = (1 << 11),
  AIRPORT_MONTH_DECEMBER = (1 << 12)
};

enum EAirportScheduleDay
{
  AIRPORT_DAY_SUNDAY = (1 << 16),
  AIRPORT_DAY_MONDAY = (1 << 17),
  AIRPORT_DAY_TUESDAY = (1 << 18),
  AIRPORT_DAY_WEDNESDAY = (1 << 19),
  AIRPORT_DAY_THURSDAY = (1 << 20),
  AIRPORT_DAY_FRIDAY = (1 << 21),
  AIRPORT_DAY_SATURDAY = (1 << 22)
};

enum EAirportScheduleRange
{
  AIRPORT_SCHEDULE_ALLDAY = (1 << 28),
  AIRPORT_SCHEDULE_SPECIFICTIME = (1 << 29),
  AIRPORT_SCHEDULE_SUNRISE = (1 << 30),
  AIRPORT_SCHEDULE_SUNSET = (1 << 31)
};

enum EAirportLightingControl
{
  AIRPORT_LIGHTING_24HOURS = (1 << 1),
  AIRPORT_LIGHTING_ATCCONTROLLED = (1 << 2),
  AIRPORT_LIGHTING_RADIOCONTROLLED = (1 << 3),
  AIRPORT_LIGHTING_RADIOREQUIRED = (1 << 4),
  AIRPORT_LIGHTING_PHONEREQUIRED = (1 << 5)
};

enum EAirportAirspace
{
  AIRPORT_AIRSPACE_A = 1,
  AIRPORT_AIRSPACE_B = 2,
  AIRPORT_AIRSPACE_C = 3,
  AIRPORT_AIRSPACE_D = 4,
  AIRPORT_AIRSPACE_E = 5,
  AIRPORT_AIRSPACE_G = 6
};

//===================================================================================
//  GLOBAL INDICES FOR DATABANK
//===================================================================================
//  Radio com Type:  Associated with comTAB in main.cpp
//-----------------------------------------------------------------------------------
enum RD_COM { COM_ZR  = 0,                    // Unknown
              COM_AT  = 1,                    // ATIS
              COM_TW  = 2,                    // TOWER
              COM_CL  = 3,                    // CLEARANCE
              COM_GR  = 4,                    // GROUND
              COM_AP  = 5,                    // APPROACH
              COM_DP  = 6,                    // DEPARTURE
              COM_SV  = 7,                    // SERVICE
              COM_CR  = 8,                    // CENTER
              COM_CF  = 9,                    // CTAF
              COM_MU  = 10,                   // MULTICOM
              COM_EM  = 11,                   // EMERGENCY
              COMDIM  = 12,                   //-Dimension --------
};
//------Define dead zone for localizer arround 90° offset -------------
	#define DEADZONE 7.0f
//====================================================================================
//
// User Interface enumerated types
//
// ??? Will these still be appropriate for an implementation using
//       PLIB?
//====================================================================================

enum EMouseCursorStyle
{
  CURSOR_ARROW = 0,
  CURSOR_CROSS = 1,
  CURSOR_FINGER = 2,
  CURSOR_HAND = 3,
  CURSOR_MOVE = 4,
  CURSOR_SIZE_H = 5,
  CURSOR_SIZE_HV = 6,
  CURSOR_SIZE_V = 7,
  CURSOR_FLIP_DOWN = 8,
  CURSOR_FLIP_UP = 9,
  CURSOR_FLIP_LEFT = 10,
  CURSOR_FLIP_RIGHT = 11,
  CURSOR_TURN_LEFT = 12,
  CURSOR_TURN_RIGHT = 13,
  CURSOR_SLIDE = 14
};

enum EMouseButton
{
  MOUSE_BUTTON_LEFT   = (1 << 0),
  MOUSE_BUTTON_RIGHT  = (1 << 1),
  MOUSE_BUTTON_MIDDLE = (1 << 2)
};

enum EWindowRegion
{ 
  IN_WINDOW_CONTENT     = 1, 
  IN_WINDOW_DRAG        = 2, 
  IN_WINDOW_CLOSE       = 3, 
  IN_WINDOW_SIZE        = 4, 
  IN_WINDOW_SIZE_RIGHT  = 5, 
  IN_WINDOW_SIZE_LEFT   = 6, 
  IN_WINDOW_SIZE_BOTTOM = 7,
  IN_WINDOW_HIDE        = 8
};

enum EWindowFlags
{
  WINDOW_HAS_TITLEBAR = (1 << 0),
  WINDOW_HAS_CLOSEBOX = (1 << 1),
  WINDOW_HAS_SIZEBOX  = (1 << 2),
  WINDOW_HAS_BORDER = (1 << 3),
  WINDOW_IS_MOVEABLE  = (1 << 4),
  WINDOW_IS_RESIZEABLE  = (1 << 5)
};

enum EWindowType
{
  WINDOW_CAMERA   = 1,
  WINDOW_RASTER_MAP = 2,
  WINDOW_VECTOR_MAP = 3,
  WINDOW_CHECKLIST  = 4,
  WINDOW_GPS    = 5,
  WINDOW_MINIPANEL  = 6,
  WINDOW_AXIS   = 7
}; 

enum EFPEntryType
{
  ENTRY_AIRPORT = 1,
  ENTRY_NAVAID  = 2,
  ENTRY_WAYPOINT  = 3,
  ENTRY_USER  = 4
};

enum ETimeOfDay
{
  TIME_DAWN = 1,
  TIME_DAYTIME  = 2,
  TIME_DUSK = 3,
  TIME_NIGHTTIME  = 4
};

enum EJoystickAxis
{
  X_AXIS_INDEX    = 0,
  Y_AXIS_INDEX    = 1,
  RUDDER_AXIS_INDEX = 2,
  TRIM_AXIS_INDEX   = 3,
  RBRAKE_AXIS_INDEX = 4,
  LBRAKE_AXIS_INDEX = 5,
  THROTTLE_AXIS_INDEX = 6,
  THROTTLE1_AXIS_INDEX  = THROTTLE_AXIS_INDEX,
  THROTTLE2_AXIS_INDEX  = 7,
  THROTTLE3_AXIS_INDEX  = 8,
  THROTTLE4_AXIS_INDEX  = 9,
  MIXTURE_AXIS_INDEX  = 10,
  MIXTURE1_AXIS_INDEX = MIXTURE_AXIS_INDEX,
  MIXTURE2_AXIS_INDEX = 11,
  MIXTURE3_AXIS_INDEX = 12,
  MIXTURE4_AXIS_INDEX = 13,
  PROP_AXIS_INDEX   = 14,
  PROP1_AXIS_INDEX  = PROP_AXIS_INDEX,
  PROP2_AXIS_INDEX  = 15,
  PROP3_AXIS_INDEX  = 16,
  PROP4_AXIS_INDEX  = 17
};

enum ERadioType
{
  RADIO_COM = (1 << 0),
  RADIO_NAV = (1 << 1),
  RADIO_DME = (1 << 2),
  RADIO_XPDR  = (1 << 3),
  RADIO_AP  = (1 << 4),
  RADIO_GPS = (1 << 5),
  RADIO_ADF = (1 << 6),
  RADIO_AUDIO = (1 << 7),
  RADIO_HF  = (1 << 8)
};

enum EFreqType
{
  FREQ_ACTIVE = 0,
  FREQ_STANDBY  = 1
};

enum 
{ RAY_COLLIDE_GROUND  = 0x01, 
  RAY_COLLIDE_MODELS  = 0x02, 
  RAY_COLLIDE_CLOUDS  = 0x04,
  RAY_COLLIDE_ALL   = 0xFF, 
  RAY_COLLIDE_SOLID_OBJECTS = (RAY_COLLIDE_ALL & ~RAY_COLLIDE_CLOUDS)
};

enum EFileSearchLocation {

  SEARCH_PODS = (1 << 0),
  SEARCH_DISK = (1 << 1),
  SEARCH_ALL_ = (SEARCH_PODS | SEARCH_DISK)
};

enum EKeyboardKeys
{ KB_KEY_NONE   = 0,
  KB_KEY_1      = '1',
  KB_KEY_2      = '2',
  KB_KEY_3      = '3',
  KB_KEY_4      = '4',
  KB_KEY_5      = '5',
  KB_KEY_6      = '6',
  KB_KEY_7      = '7',
  KB_KEY_8      = '8',
  KB_KEY_9      = '9',
  KB_KEY_0      = '0',
  KB_KEY_MINUS  = '-',
  KB_KEY_EQUALS = '=',
	KB_KEY_PLUS		= '+',
  KB_KEY_Q      = 16,
  KB_KEY_W      = 17,
  KB_KEY_E      = 18,
  KB_KEY_R      = 19,
  KB_KEY_T      = 20,
  KB_KEY_Y      = 21,
  KB_KEY_U      = 22,
  KB_KEY_I      = 23,
  KB_KEY_O      = 24,
  KB_KEY_P      = 25,
  KB_KEY_FORWARD_BRACKET    = '[',
  KB_KEY_REVERSE_BRACKET    = ']',
  KB_KEY_A      = 30,
  KB_KEY_S      = 31,
  KB_KEY_D      = 32,
  KB_KEY_F      = 33,
  KB_KEY_G      = 34,
  KB_KEY_H      = 35,
  KB_KEY_J      = 36,
  KB_KEY_K      = 37,
  KB_KEY_L      = 38,
  KB_KEY_SEMI_COLON						= 39,
  KB_KEY_SINGLE_QUOTE					= '\'',
  KB_KEY_REVERSE_SINGLE_QUOTE = '`',
	KB_KEY_TILDE								= '~',
  KB_KEY_BACKSLASH						= '\\',
  KB_KEY_Z      = 44,
  KB_KEY_X      = 45,
  KB_KEY_C      = 46,
  KB_KEY_V      = 47,
  KB_KEY_B      = 48,
  KB_KEY_N      = 49,
  KB_KEY_M      = 50,
  KB_KEY_COMMA  = ',',
  KB_KEY_PERIOD = '.',
  KB_KEY_SLASH  = '/',
  KB_KEY_STAR   = '*',
  KB_KEY_SPACE  = ' ',
	KB_KEY_DBLEP  = 58,
	KB_KEY_LOWER  = 83,
	KB_KEY_GREAT	= 84,
	KB_KEY_EXCLM  = '!',
	KB_KEY_AROBS  = '@',
	KB_KEY_DIEZE	= '#',
	KB_KEY_DOLAR  = '$',
	KB_KEY_PERCN	= '%',
	KB_KEY_HATOV  = '^',
	KB_KEY_AMPER  = '&',
	KB_KEY_WHAT   = '?',
	KB_KEY_LEFTP  = '(',
	KB_KEY_RITEP  = ')',
	KB_KEY_UNDER  = '_',
	KB_KEY_LEFTB	= '{',
	KB_KEY_RITEB	= '}',
  KB_KEY_VBAR		= '|',
	KB_KEY_DOUBLE_QUOTE = '\"',

	//---Special keys --------------------------------------------
  KB_KEY_BACK   = 128,
	KB_KEY_TAB		= 129,
  KB_KEY_ENTER	= 130,
  KB_KEY_ESC    = 131,
	//--- GLUT KEYS ---------------------------------------------
  KB_KEY_F1     = 141,			//141,		// WAS 59
  KB_KEY_F2     = 142,			//142,		// 60
  KB_KEY_F3     = 143,			//131,		// 61
  KB_KEY_F4     = 144,			//132,		// 62
  KB_KEY_F5     = 145,			//133,		// 63
  KB_KEY_F6     = 146,			//134,		// 64
  KB_KEY_F7     = 147,			//135,		// 65
  KB_KEY_F8     = 148,			//136,		// 66
  KB_KEY_F9     = 149,			//137,		// 67
  KB_KEY_F10    = 150,			//138,		// 68
  KB_KEY_F11    = 151,			//139,		// 87
  KB_KEY_F12    = 152,			//140,		// 88
	//-----------------------------------------------------------
	KB_KEY_HOME   = 153,			//141,		// 71
  KB_KEY_UP     = 154,			//142,		// 72
	KB_KEY_PGUP		= 155,			//143,		// 73
  KB_KEY_LEFT   = 156,			//144,		// 75
  KB_KEY_RIGHT  = 157,			//145,		// 77
  KB_KEY_END    = 158,			//146,		// 79
  KB_KEY_DOWN   = 159,			//147,		// 80
  KB_KEY_PGDN   = 160,			//148,		// 81
  KB_KEY_INSERT = 161,			//149,		// 82
  KB_KEY_LCTRL	= 162,
  KB_KEY_LSHIFT	= 163,
  KB_KEY_RSHIFT = 164,
  KB_KEY_LALT   = 165,
	//--- NON GLUT KEYS -----------------------------------------
  KB_KEY_CAPSLOCK     = 180,		// 58
  KB_KEY_NUMLOCK			= 181,		// 69
  KB_KEY_SCROLLLOCK		= 182,		// 70
  KB_KEY_KEYPAD_MINUS = 183,		// 74
  KB_KEY_CENTER				= 184,		// 76
  KB_KEY_KEYPAD_PLUS  = 185,		// 78
  KB_KEY_DEL					= 186,		// 83
	//--- Key PAD Keys ------------------------------------------
  KB_KEY_KEYPAD_ENTER   = 200,	//187,	// 284
  KB_KEY_RCTRL					= 201,	//188,	// 285
  KB_KEY_KEYPAD_SLASH   = 202,	//189,	// 309
  KB_KEY_RALT						= 203,	//190,	// 312
  KB_KEY_EXT_NUMLOCK    = 204,	//191,	// 325
  KB_KEY_GRAY_HOME			= 205,	//192,	// 327
  KB_KEY_GRAY_UP				= 206,	//193,	// 328
  KB_KEY_GRAY_PGUP			= 207,	//194,	// 329
  KB_KEY_GRAY_LEFT			= 208,	//195,	// 331
  KB_KEY_GRAY_RIGHT			= 209,	//196,	// 333
  KB_KEY_GRAY_END				= 210,	//197,	// 335
  KB_KEY_GRAY_DOWN			= 211,	//198,	// 336
  KB_KEY_GRAY_PGDN			= 212,	//199,	// 337
  KB_KEY_GRAY_INS				= 213,	//200,	// 338
  KB_KEY_GRAY_DEL				= 214,	//201,	// 339

	//------------------------------------------------------------
  KB_KEY_META						= 340
};
//----------------------------------------------------------------------
//	Key class
//----------------------------------------------------------------------
#define KSP (0x01)				// Space
#define KLT (0x02)				// Letter
#define KNB (0x04)				// Number
#define KSG (0x08)				// Sign + -
#define KPN (0x10)				// punctuation
#define KDT (0x20)				// Dot
#define KKK (0x40)				// other
//----------------------------------------------------------------------
#define KB_WORD_ONLY			(KLT+KNB)
#define KB_NUMBER_SIGN		(KNB+KSG+KDT)
#define KB_ANYTHING		    (KSP+KLT+KNB+KPN+KDT+KKK) 
//----------------------------------------------------------------------
enum EKeyboardModifiers
{
  KB_MODIFIER_NONE    = (0 << 0),
  KB_MODIFIER_CTRL    = (1 << 0),
  KB_MODIFIER_ALT     = (1 << 1),
  KB_MODIFIER_SHIFT   = (1 << 2),
  KB_MODIFIER_META    = (1 << 3)
};

enum EGaugeHilite
{
  GAUGE_HILITE_NONE = 0,
  GAUGE_HILITE_ARROW  = (1 << 0),
  GAUGE_HILITE_BOX  = (1 << 1),
  GAUGE_HILITE_CIRCLE = (1 << 2),
  GAUGE_HILITE_BLINK  = (1 << 8)
};

enum EWeatherObstructionTypes
{
  WEATHER_INTENSITY_VICINITY        = 0x00000001, // Weather intensity (bits 0-3)
  WEATHER_INTENSITY_LIGHT     = 0x00000002,
  WEATHER_INTENSITY_MODERATE    = 0x00000004,
  WEATHER_INTENSITY_HEAVY     = 0x00000008,
  WEATHER_DESCRIPTOR_SHALLOW    = 0x00000010, // Weather description (4-11)
  WEATHER_DESCRIPTOR_PARTIAL    = 0x00000020,
  WEATHER_DESCRIPTOR_PATCHES    = 0x00000040,
  WEATHER_DESCRIPTOR_LOW_DRIFTING   = 0x00000080,
  WEATHER_DESCRIPTOR_BLOWING    = 0x00000100,
  WEATHER_DESCRIPTOR_SHOWERS    = 0x00000200,
  WEATHER_DESCRIPTOR_THUNDERSTORMS  = 0x00000400,
  WEATHER_DESCRIPTOR_FREEZING   = 0x00000800,
  WEATHER_PHENOMENA_DRIZZLE   = 0x00001000, // Weather phenomena (bits 12-31)
  WEATHER_PHENOMENA_RAIN      = 0x00002000,
  WEATHER_PHENOMENA_SNOW      = 0x00004000,
  WEATHER_PHENOMENA_SNOW_GRAINS   = 0x00008000,
  WEATHER_PHENOMENA_ICE_CRYSTALS    = 0x00010000,
  WEATHER_PHENOMENA_ICE_PELLETS   = 0x00020000,
  WEATHER_PHENOMENA_HAIL      = 0x00040000,
  WEATHER_PHENOMENA_SMALL_HAIL_OR_SNOW  = 0x00080000,
  WEATHER_PHENOMENA_FOG     = 0x00100000,
  WEATHER_PHENOMENA_UNIDENTIFIED    = 0x00200000,
  WEATHER_PHENOMENA_VOLCANIC_ASH    = 0x00400000,
  WEATHER_PHENOMENA_SQUALL    = 0x00800000,
  WEATHER_PHENOMENA_WIDESPREAD_DUST = 0x01000000,
  WEATHER_PHENOMENA_SPRAY     = 0x02000000,
  WEATHER_PHENOMENA_SAND      = 0x04000000,
  WEATHER_PHENOMENA_DUST_SAND_WHIRLS  = 0x08000000,
  WEATHER_PHENOMENA_FUNNEL_CLOUD    = 0x10000000,
  WEATHER_PHENOMENA_SAND_STORM    = 0x20000000,
  WEATHER_PHENOMENA_DUST_STORM    = 0x40000000
};

enum EWindsAloftLayer
{
  WEATHER_WINDS_ALOFT_3000 = 0,
  WEATHER_WINDS_ALOFT_6000 = 1,
  WEATHER_WINDS_ALOFT_9000 = 2,
  WEATHER_WINDS_ALOFT_12000 = 3,
  WEATHER_WINDS_ALOFT_15000 = 4,
  WEATHER_WINDS_ALOFT_18000 = 5,
  WEATHER_WINDS_ALOFT_FL18 = WEATHER_WINDS_ALOFT_18000,
  WEATHER_WINDS_ALOFT_21000 = 6,
  WEATHER_WINDS_ALOFT_FL21 = WEATHER_WINDS_ALOFT_21000,
  WEATHER_WINDS_ALOFT_24000 = 7,
  WEATHER_WINDS_ALOFT_FL24 = WEATHER_WINDS_ALOFT_24000,
  WEATHER_WINDS_ALOFT_27000 = 8,
  WEATHER_WINDS_ALOFT_FL27 = WEATHER_WINDS_ALOFT_27000,
  WEATHER_WINDS_ALOFT_30000 = 9,
  WEATHER_WINDS_ALOFT_FL30 = WEATHER_WINDS_ALOFT_30000,
  WEATHER_WINDS_ALOFT_33000 = 10,
  WEATHER_WINDS_ALOFT_FL33 = WEATHER_WINDS_ALOFT_33000,
  WEATHER_WINDS_ALOFT_36000 = 11,
  WEATHER_WINDS_ALOFT_FL36 = WEATHER_WINDS_ALOFT_36000,
  WEATHER_WINDS_ALOFT_39000 = 12,
  WEATHER_WINDS_ALOFT_FL39 = WEATHER_WINDS_ALOFT_39000,
  WEATHER_WINDS_ALOFT_42000 = 13,
  WEATHER_WINDS_ALOFT_FL42 = WEATHER_WINDS_ALOFT_42000,
  WEATHER_WINDS_ALOFT_60000 = 14,
  WEATHER_WINDS_ALOFT_FL60 = WEATHER_WINDS_ALOFT_60000,
  WEATHER_WINDS_ALOFT_MAX
};

enum EWeatherSkyTypes
{
  WEATHER_SKY_UNKNOWN = 0,
  WEATHER_SKY_CLEAR = 1,
  WEATHER_SKY_FEW = 2,
  WEATHER_SKY_SCATTERED = 3,
  WEATHER_SKY_BROKEN = 4,
  WEATHER_SKY_OVERCAST = 5,
  WEATHER_SKY_FOG = 6
};

enum EWheelStatus
{
  WHEELS_NOTONGROUND = 0,
  WHEELS_SOMEONGROUND = 1,
  WHEELS_ALLONGROUND = 2,
  WHEELS_ONEONGROUND = 3,
  WHEELS_INWATER = 4
};

enum EGroundTypes 
{
  GROUND_CONCRETE = 1,
  GROUND_ASPHALT  = 2,
  GROUND_TURF     = 3,
  GROUND_DIRT     = 4,
  GROUND_GRAVEL   = 5,
  GROUND_METAL    = 6,
  GROUND_SAND     = 7,
  GROUND_WOOD     = 8,
  GROUND_WATER    = 9,
  GROUND_MATS     = 10,
  GROUND_SNOW     = 11,
  GROUND_ICE      = 12,
  GROUND_GROOVED  = 13,
  GROUND_TREATED  = 14
};

enum ETerrainType
{
  TERRAIN_WATER_OCEAN = 12,
  TERRAIN_ICE_CRACKED_GLACIERS = 13,
  TERRAIN_SNOW = 59,
  TERRAIN_MOUNTAIN_FOREST_CALIFORNIA = 60,
  TERRAIN_TOWNS_MIDEAST = 61,
  TERRAIN_CITY_USA = 101,
  TERRAIN_GRASS_SCRUB_LIGHT = 102,
  TERRAIN_FOREST_MIXED_SCRUB = 104,
  TERRAIN_FOREST_MIXED = 105,
  TERRAIN_MOUNTAIN_FOREST_GREEN = 106,
  TERRAIN_GRASS_SCRUB_LIGHT_2 = 107,
  TERRAIN_DESERT_BARREN = 108,
  TERRAIN_TUNDRA_UPLAND = 109,
  TERRAIN_GRASS_SCRUB_LIGHT_3 = 110,
  TERRAIN_DESERT_SCRUB_LIGHT = 111,
  TERRAIN_SNOW_HARD_PACKED = 112,
  TERRAIN_EURO_CITY_ENGLAND_HEAVY = 113,
  TERRAIN_SHRUB_EVERGREEN = 116,
  TERRAIN_SHRUB_DECIDUOUS = 117,
  TERRAIN_CITY_HEAVY_ASIAN = 118,
  TERRAIN_FOREST_EVERGREEN_W_DARK_FIELDS = 119,
  TERRAIN_RAINFOREST_TROPICAL_MIXED = 120,
  TERRAIN_CONIFER_BOREAL_FOREST = 121,
  TERRAIN_COOL_CONIFER_FOREST = 122,
  TERRAIN_EURO_FOREST_TOWNS = 123,
  TERRAIN_FOREST_RIDGES_MIXED = 124,
  TERRAIN_EURO_FOREST_DARK_TOWNS = 125,
  TERRAIN_SUBURB_USA_BROWN = 126,
  TERRAIN_CONIFER_FOREST = 127,
  TERRAIN_FOREST_YELLOWED_AFRICAN = 128,
  TERRAIN_SEASONAL_RAINFOREST = 129,
  TERRAIN_COOL_CROPS = 130,
  TERRAIN_CROPS_TOWN = 131,
  TERRAIN_DRY_TROPICAL_WOODS = 132,
  TERRAIN_RAINFOREST_TROPICAL = 133,
  TERRAIN_RAINFOREST_DEGRADED = 134,
  TERRAIN_FARM_USA_GREEN = 135,
  TERRAIN_RICE_PADDIES = 136,
  TERRAIN_HOT_CROPS = 137,
  TERRAIN_FARM_USA_MIXED = 138,
  TERRAIN_EURO_FARM_FULL_MIXED = 139,
  TERRAIN_COOL_GRASSES_W_SHRUBS = 140,
  TERRAIN_HOT_GRASSES_W_SHRUBS = 141,
  TERRAIN_COLD_GRASSLAND = 142,
  TERRAIN_SAVANNA_WOODS = 143,
  TERRAIN_SWAMP_BOG = 144,
  TERRAIN_MARSH_WETLAND = 145,
  TERRAIN_MEDITERRANEAN_SCRUB = 146,
  TERRAIN_DRY_WOODY_SCRUB = 147,
  TERRAIN_DRY_EVERGREEN_WOODS = 148,
  TERRAIN_SUBURB_USA = 149,
  TERRAIN_DESERT_SAND_DUNES = 150,
  TERRAIN_SEMI_DESERT_SHRUB = 151,
  TERRAIN_SEMI_DESERT_SAGE = 152,
  TERRAIN_MOUNTAIN_ROCKY_TUNDRA_SNOW = 153,
  TERRAIN_MIXED_RAINFOREST = 154,
  TERRAIN_COOL_FIELDS_AND_WOODS = 155,
  TERRAIN_FOREST_AND_FIELD = 156,
  TERRAIN_COOL_FOREST_AND_FIELD = 157,
  TERRAIN_FIELDS_AND_WOODY_SAVANNA = 158,
  TERRAIN_THORN_SCRUB = 159,
  TERRAIN_SMALL_LEAF_MIXED_FOREST = 160,
  TERRAIN_MIXED_BOREAL_FOREST = 161,
  TERRAIN_NARROW_CONIFERS = 162,
  TERRAIN_WOODED_TUNDRA = 163,
  TERRAIN_HEATH_SCRUB = 164,
  TERRAIN_EURO_FULL_GREEN_3 = 165,
  TERRAIN_AUSTRALIAN_CITY = 166,
  TERRAIN_CITY_HEAVY_BRAZIL = 167,
  TERRAIN_CITY_HEAVY_USA = 168,
  TERRAIN_POLAR_DESERT = 169,
  TERRAIN_EURO_TOWNS = 170,
  TERRAIN_CITY_MIDDLE_EASTERN_HEAVY = 171,
  TERRAIN_TUNDRA_BOG = 172,
  TERRAIN_EURO_FARM_FULL_MIXED_2 = 176,
  TERRAIN_TOWNS_ASIAN = 177,
  TERRAIN_ICE_CRACKED_SNOW = 178,
  TERRAIN_EURO_FARM_FOREST = 179,
  TERRAIN_FARM_USA_BROWN = 180,
  TERRAIN_FARM_MIDDLE_EASTERN_W_FOREST = 181,
  TERRAIN_DESERT_BRUSH_REDDISH = 182,
  TERRAIN_FARM_MIDDLE_EASTERN = 183,
  TERRAIN_EURO_FARM_FULL_MIXED_3 = 184,
  TERRAIN_EURO_FARM_FULL_GREEN_2 = 185,
  TERRAIN_MOUNTAIN_ROCKY_SNOW = 186,
  TERRAIN_MOUNTAIN_FOREST_LIGHT = 187,
  TERRAIN_GRASS_MEADOW = 188,
  TERRAIN_MOIST_EUCALYPTUS = 189,
  TERRAIN_RAINFOREST_HAWAIIAN = 190,
  TERRAIN_WOODY_SAVANNA = 191,
  TERRAIN_BROADLEAF_CROPS = 192,
  TERRAIN_GRASS_CROPS = 193,
  TERRAIN_CROPS_GRASS_SHRUBS = 194,
  TERRAIN_GRASSLAND = 225,
  TERRAIN_DESERT_SAVANAH_AFRICAN = 255
};

enum ETerrainSubdivision
{
  TERRAIN_SUBDIVISION_WORLD               =  0,
  TERRAIN_SUBDIVISION_QTR                 =  5,
  TERRAIN_SUBDIVISION_GLOBE_TILE          =  8,
  TERRAIN_SUBDIVISION_QUARTER_GLOBE_TILE  =  9,
  TERRAIN_SUBDIVISION_SUPER_TILE          = 12,
  TERRAIN_SUBDIVISION_DETAIL_TILE         = 14
};

enum EGestaltFlags
{
  GESTALT_OBJECT_NEEDS_TO_DRAW  = (1 << 0),
  GESTALT_OBJECT_NEEDS_TIME_SLICE = (1 << 1),
  GESTALT_OBJECT_HAS_MODEL  = (1 << 2),
  GESTALT_OBJECT_HAS_CAMERA = (1 << 3),
  GESTALT_OBJECT_CAN_ACTIVATE = (1 << 4),
  GESTALT_OBJECT_HAS_CONTROLS = (1 << 5),
  GESTALT_OBJECT_UNDER_USER_CONTROL = (1 << 6), // set by Fly!; don't modify!
  GESTALT_PRIMARY_USER_OBJECT = (1 << 7), // set by Fly!; don't modify
  GESTALT_STATIC_SCENERY_OBJECT = (1 << 8), // objects saved to scenery database
  GESTALT_DYNAMIC_SCENERY_OBJECT  = (1 << 9), // objects NOT saved to scenery database
  GESTALT_DRAW_OBJECT_NAME  = (1 << 10),  // draw popup name when pointed at with mouse
  GESTALT_OBJECT_IS_CLOUD   = (1 << 11),
  GESTALT_DO_NOT_AUTO_SNAP  = (1 << 12),  // snap object to the ground automatically 
  GESTALT_OBJECT_IS_TOWER   = (1 << 13),  // used to designate for tower camera
  GESTALT_OBJECT_ANIMATED   = (1 << 14),  // animated scenery (cars, trucks, boats, etc.)
  GESTALT_OBJECT_COLLISIONS = (1 << 15),  // collision detection with this object (default ON)
  GESTALT_DISTANCE_HIDING   = (1 << 16),  // object hidden when eye past a threshold distance
  GESTALT_SINK_UNDERGROUND  = (1 << 17),  // sink model by 8-10 feet underground (for raised sidewalks)
  GESTALT_DRAW_NIGHT_LIGHTS = (1 << 18),  // force night lights on models
  GESTALT_DRAW_SHADOW   = (1 << 19),  // draw shadows for this object??
  GESTALT_OBJECT_GENERATED  = (1 << 20),  // object was generated by an automated external source (DLL)
  GESTALT_NO_Z_BUFFER   = (1 << 21),  // object doesn't use Z-buffer, draws after taxiways and runways, but before models (ground striping)
  GESTALT_Z_SORTED    = (1 << 22),  // object must sort in Z before drawing (back to front)
  GESTALT_NO_Z_BUFFER_UNDERLAY  = (1 << 23) // object doesn't use Z-buffer, draws before taxiways and runways (ground underlay)
};
enum eWheelType {
  TRICYCLE      = 1,
  TAIL_DRAGGER  = 2,
  SNOW          = 3,
  SKIDS         = 4,
};
//=====================================================================================
//	Sound definitions
//=====================================================================================
typedef enum {
  ENGINE_STOPPED    = 0,
  ENGINE_CRANKING   = 1,
  ENGINE_RUNNING    = 2,
  ENGINE_STOPPING   = 3,
  ENGINE_FAILING    = 4,
  ENGINE_WINDMILL   = 5,
  ENGINE_CATCHING   = 6,
	ENGINE_MISSFIRE   = 7,
	ENGINE_MAX_SOUND  = 8,
} EEngineStatus;

/////////////////////////////////////////////////////////////
//
//  Structures/Typedefs
//
/////////////////////////////////////////////////////////////

typedef struct SDLLCopyright
{
  char  product[128];
  char  company[128];
  char  programmer[128];
  char  dateTimeVersion[128];
  char  email[128];
  int internalVersion;
} SDLLCopyright;

typedef struct SDLLRegisterTypeList
{
  EDLLObjectType  type;
  long    signature;
  SDLLRegisterTypeList  *next;
} SDLLRegisterTypeList;

typedef void* DLLObjectRef;
typedef void* DLLFileRef;

typedef struct SFlyObjectRef
{
  void  *objectPtr;
  void  *superSig;
  void  *classSig;
} SFlyObjectRef;

typedef struct SFlyObjectList
{
  SFlyObjectRef ref;
  SFlyObjectList  *prev;
  SFlyObjectList  *next;
} SFlyObjectList;

typedef struct SDLLObject
{
  //
  //  public; set this to any value you want to uniquely
  //  identify an "instance" of your object on the .DLL
  //  side.  this can be a gauge, system, or other 
  //  component.
  //

  DLLObjectRef  dllObject;  // set by .DLL; can be any value you want

  //
  //  private; do not use or modify the following members
  //

  DLLFileRef  dllFile;  // set by Fly!; do NOT modify this value!!!
  SFlyObjectRef flyObject;  // set by Fly!; do NOT modify this value!!!
} SDLLObject;
//---------------------------------------------------------------------------
//  JS NOTE:  replace Sender by a Tag because the field was not used
//            coherently.  Gauges and subsytems where stored in this field
//            without any mean to know which one
//	Add a first tag used only to identify this structure when memory leak occurs
//---------------------------------------------------------------------------
typedef struct SMessage
{ Tag								mem;						// Memory type
  unsigned int      id;				      // message ID
  unsigned int      group;			    // target group ID
  EMessageDataType  dataType;       // result data type
  unsigned int      result;			    // message result code
	unsigned int			index;					// Index value
  Tag               sender;         // Sender tag
  DLLObjectRef      receiver;
  char              dst[8];         // string destination
  float             volts;
  union 
  { char    charData;
    char   *charPtrData;
    int     intData;
    int    *intPtrData;
    double  realData;
    double *realPtrData;
    void   *voidData;
  };
      
  union
  {
    unsigned int  userData[8];
    struct 
    {
      unsigned int  unit;				// unit number
      unsigned int  hw;					// hardware type
      unsigned int  engine;			// engine number
      unsigned int  tank;				// tank number
      unsigned int  gear;				// gear number
      unsigned int  invert;			// Used to invert value
      unsigned int  datatag;		// data tag
      unsigned int  sw;					// switch position
    } u;
  } user;
  //--- Constructor -------------------------------
  SMessage()
  { memset(this,0,sizeof(SMessage)); 
		mem = 'SMSG'; }
	//--- Tarce message -----------------------------
	void	Trace();
} SMessage;
//==============================================================================
typedef struct SSurface
{ unsigned int   nPixel;
  unsigned int   invert;          // (ySize -1)* xSpan to reverse line order
  unsigned int  *drawBuffer;      // surface buffer
  unsigned int   xSize, ySize;    // surface dimensions
  int   xScreen, yScreen;         // screen coordinates for upper-left corner
} SSurface;

typedef struct SFont
{
  char  fontName[64];   // -> passed to APILoadFont; name of font family image file
  void  *font;      // <- returned from APILoadFont; do NOT alter!!!
} SFont;

//-------------------------------------------------------------------------
struct SBitmap {
  char    bitmapName[64]; // -> passed to APILoadBitmap
  void    *bitmap;				// <- returned from APILoadBitmap; do NOT alter!!!
  EBitmapType type;				// <- returned from APILoadBitmap; do NOT alter!!!
	//----------------------------------------------------------------------
	SBitmap() {bitmap = 0;}
	//----Set abort when bitmap is not free ----------------------
 ~SBitmap() 
	{if (bitmap)  {int *a = 0; *a = 1;} }
};

//===========================================================================
typedef struct {
  unsigned long   magic;    // Magic code of 'PBM '
  unsigned long   width;
  unsigned long   height;
  unsigned long   x1, y1, x2, y2;
} SPBMHeader;


typedef struct {
  GLubyte r, g, b;
} RGB;


//===============================================================================
// SStream represents a stream text file, see Stream.h and Stream.cpp for details
//===============================================================================
class CStreamObject;
//-----------------------------------------------------
typedef struct SStream
{ char  filename[PATH_MAX];
  char  mode[3];
	bool  ok;
  void  *stream;
	//----------------------------------------------------
	SStream() {;}									// Standard constructor
	SStream(CStreamObject *object, char *fn);						// Open file for read 
	SStream(CStreamObject *object, char *pn, char *fn);	// Open file in path for read
} SStream;

//===============================================================================
// SVector represents a vector in either Cartesian (x,y,z) or Polar (p,h,r)
//   coordinates
//===============================================================================

typedef struct SVector
{
  union {
    double  x;
    double  p;  // when used as a polar, pitch (in radians)
  };

  union {
    double  y;
    double  h;  // when used as a polar, heading (in radians)
  };

  union {
    double  z;
    double  r;  // when used as a polar, radius (in feet)
  };
} SVector;
;
//===============================================================================
// SPosition represents a geographical position on the earth.  The "zero" reference
//   point is 0 deg latitude, 0 deg longitude, 0 feet MSL.
//
// 'lat' is the latitude, in arcseconds north (+) or south (-) of the equator
// 'lon' is the longitude, in arcseconds west of the Prime Meridian
// 'alt' is the altitude in feed above MSL
//=================================================================================

typedef struct SPosition
{
  double  lat;
  double  lon;
  double  alt;
} SPosition;


//----------------------------------------
	//SPosition::SPosition() {lat = lon = alt = 0;}
//------------------------------------------------
typedef struct SPositionList
{
  SPosition pos;    // points should be stored clockwise
  SPositionList *next;
} SPositionList;

typedef struct SCollision
{
  char    collided; // 0 or 1
  SPosition pos;    // collision position
  SFlyObjectRef object;   // collision object
} SCollision;

typedef struct SMovie
{
  void  *gc;
  void  *movie;
  SSurface  surface;
} SMovie;

typedef struct SNavaid
{
  char    name[40];
  char    id[8];

  int   type;
  int   classification;
  int   usage;

  SPosition pos;
  float   freq;
  float   range;
  float   magneticVariation;
  float   slavedVariation;

  SNavaid   *prev;  // double-linked list
  SNavaid   *next;  // double-linked list
} SNavaid;

typedef struct SILS
{
  char    airportKey[10];
  char    runwayID[5];

  int   type;
  int   classification;
  SPosition pos;
  float   range;
  float   magneticVariation;

  float   glideslopeAngle;
  float   approachBearing;
  char    backCourseAvailable;

  SILS    *prev;  // double-linked list
  SILS    *next;  // double-linked list

  //  version 2.0 additions

  float   freq;
} SILS;

typedef struct SComm
{
  char    airportKey[10];

  char    name[20];
  int   type;
  float   freq[5];
  SPosition pos;

  SComm   *prev;  // double-linked list
  SComm   *next;  // double-linked list
} SComm;

typedef struct SWaypoint
{
  char    name[26];
  char    colocated;  // 0 or 1
  int   type;
  int   usage;
  float   bearingToNavaid;
  float   distanceToNavaid;
  SPosition pos;
  char    navaid[10];

  SWaypoint *prev;
  SWaypoint *next;
} SWaypoint;

typedef struct SRunwayEnd
{
  char    id[4];        // painted end marking
  float   trueHeading;      // in degrees
  float   magneticHeading;    // in degrees
  int   markings;     // numbers only, PIR, STOL, etc.
  SPosition pos;        // lat/lon position of center of runway end
  int   thresholdCrossingHeight;  // in feet
  int   displacedThreshold;   // in feet
  int   rvr;        // touchdown, midfield, rollout
  char    rvv;        // 0..1
  char    edgeLights;     // 0..1
  char    endLights;      // 0..1
  char    centerlineLights;   // 0..1
  char    touchdownLights;    // 0..1
  char    alignLights;      // 0..1
  char    thresholdLights;    // 0..1
  char    sequenceFlashing;   // 0..1
  int   numLightSystems;    // 0..1
  int   lightSystems[8];    // 0..1
  char    edgeLightIntensity;   // 0..1
  int   glideslopeType;     // 0..1
  int   glideslopePlacement;    // 0..1
  int   glideslopeConfiguration;  // 0..1

  SILS    *glideslope;      // associated GS
  SILS    *localizer;     // associated LOC
} SRunwayEnd;


typedef struct SRunway
{
  char    airportKey[10];

  float   length;     // in feet
  float   width;      // in feet
  int   surfaceType;    // surface material (asphalt, concrete, etc.)
  int   surfaceCondition; // condition (good, fair, poor)
  char    pavementType;   // rigid or flexible
  int   pcn;      // pavement classification number (FAA)
  int   subgradeStrength; // high, medium, low, very low
  int   tirePressure;   // high, medium, low, very low
  char    closed;     // 0..1
  char    pcl;      // 0..1
  char    pattern;    // left or right
  float   elevation;    // in feet

  SRunwayEnd  base;     // base end data
  SRunwayEnd  recip;      // reciprocal end data

  SRunway   *prev;      // double-linked list
  SRunway   *next;      // double-linked list
} SRunway;


typedef struct SAirport
{
  char    airportKey[10];
  char    faaID[5];
  char    icaoID[5];
  int   type;
  char    name[40];
  char    country[3];
  char    state[3];
  char    county[40];
  char    city[40];
  float   elevation;
  int   ownership;
  int   usage;
  float   magneticVariation;
  float   trafficAltitude;
  char    fssAvailable;   // 0..1
  char    notamAvailable;   // 0..1
  char    atcAvailable;   // 0..1
  char    segmentedCircle;  // 0..1
  char    landingFeesCharged; // 0..1
  char    jointUseAirport;  // 0..1
  char    militaryLandingRights;  // 0..1
  char    customsEntryPoint;  // 0..1
  int   fuelTypes;
  int   frameService;
  int   engineService;
  int   bottledOxygen;
  int   bulkOxygen;
  int   beaconLensColor;
  int   basedAircraft;
  int   annualCommercialOps;
  int   annualGeneralAvOps;
  int   annualMilitaryOps;
  int   attendanceFlags;
  int   lightingFlags;
  SPosition pos;
      
  SRunway   *runways;   // list of runways
  SComm   *comms;     // list of radio comms
    
  SAirport  *prev;      // double-linked list
  SAirport  *next;      // double-linked list
} SAirport;

typedef struct SGeneric
{
  void  *data;

  SGeneric  *prev;
  SGeneric  *next;
} SGeneric;
//=============================================================================
//  WEATHER INFO
//=============================================================================
typedef struct SWeatherInfo
{
  //
  //  weather data
  //

  double  visibility;     // in nautical miles (10.0 = 10 nautical miles)
  int surfaceTemp;        // degrees F
  int dewPointTemp;       // degrees F
  double  altimeter;      // inches Hg
  int windSpeed;          // knots
  int gustSpeed;          // knots
  int windHeading;        // degrees
  char  windFluctuates;   // 0=no fluctuation, 1=fluctuates
  int precipIntensity;    // valid values are:
          //  2=Light
          //  4=Medium
          //  8=Heavy (Thunderstorms if precipType is rain)
  int precipType;   // valid values are:
          //  0x2000=Rain (8192 decimal)
          //  0x4000=Snow (16384 decimal)

  //
  //  cloud layers
  //

  int layer1Active;   // 0 if no cloud layer, 1 if active
  int layer1Altitude;   // in feet
  int layer1Type;   // valid values are:
          //  2=Few
          //  3=Scattered
          //  4=Broken
          //  5=Overcast
          //  6=Fog
  int layer1Height;   // in feet

  int layer2Active;   // 0 if no cloud layer, 1 if active
  int layer2Altitude;   // in feet
  int layer2Type;   // valid values are:
          //  2=Few
          //  3=Scattered
          //  4=Broken
          //  5=Overcast
          //  6=Fog
  int layer2Height;   // in feet

  int layer3Active;   // 0 if no cloud layer, 1 if active
  int layer3Altitude;   // in feet
  int layer3Type;   // valid values are:
          //  2=Few
          //  3=Scattered
          //  4=Broken
          //  5=Overcast
          //  6=Fog
  int layer3Height;   // in feet
} SWeatherInfo;

typedef struct SWindsAloft
{
  int windHeading;    // in degrees
  int windSpeed;      // in knots
  int gustSpeed;      // in knots
  char  windFluctuates;   // non-zero if TRUE, zero if FALSE
} SWindsAloft;

//
// This structure represents a calendar date, with field definitions as follows:
//  month = 1..12, for January..December
//  day   = 1..31, for the calendar date in each month
//  year  = Gregorian year (positive is AD)
//
typedef struct SDate
{
  unsigned int  month;
  unsigned int  day;
  unsigned int  year;
} SDate;

//
// This structure represents a time of day, with field definitions as follows:
//  hour  = number of whole hours since midnight
//  minute  = number of whole minutes since the start of the last hour
//  second  = number of seconds since the start of the last minute
//  msecs = number of milliseconds since the start of the last second
//
typedef struct STime
{
  unsigned int  hour;
  unsigned int  minute;
  unsigned int  second;
  unsigned int  msecs;
} STime;

//
// This structure simply combines the SDate and STime structs into a single
//   entity for convenience
//
typedef struct SDateTime
{
  SDate date;
  STime time;
} SDateTime;

//
// The following structure represents a date/time difference.  Note that since
//   all fields are unsigned, this can only represent a positive difference
//   in time.
//
typedef struct SDateTimeDelta
{
  unsigned int  dYears;
  unsigned int  dMonths;
  unsigned int  dDays;
  unsigned int  dHours;
  unsigned int  dMinutes;
  unsigned int  dSeconds;
  unsigned int  dMillisecs;
} SDateTimeDelta;


typedef struct SFPAirport
{
  char    landHere; // 0 or 1
  SDateTimeDelta  layover;
  SAirport  data;
  char    depRunwayEnd[4];  // runway end ID
  char    arrRunwayEnd[4];  // runway end ID
} SFPAirport;

typedef struct SFPNavaid
{
  SNavaid   data;
} SFPNavaid;

typedef struct SFPWaypoint
{
  SWaypoint data;
} SFPWaypoint;

typedef struct SFPUser
{
  char  name[40];
} SFPUser;

typedef struct SFPEntry
{
  EFPEntryType  type;
  SPosition pos;
  SDateTime arrival;
  SDateTime departure;
  SDateTimeDelta  extend;
  float   distance;
  float   bearing;
  float   speed;
  float   altitude;
  float   effectiveSpeed;
  float   effectiveAltitude;
  float   magneticVariation;

  union   // 'type' determines which struct is populated
  {
    SFPAirport  a;
    SFPNavaid n;
    SFPWaypoint w;
    SFPUser   u;
  };
} SFPEntry;

typedef struct SFlightPlan
{
  SFPEntry  entry;
  SFlightPlan *prev;
  SFlightPlan *next;
} SFlightPlan;

// added for multiple screen management
typedef struct {
		int xRes, yRes;
    int Width, Height, Depth, Refresh, X, Y, ID;
    bool bMouseOn;
    char full;
} sScreenParams;
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
	//-------------------------------------------------------------
  VEH_D_CRASH   = 0x0100,           // Crash detector
	VEH_OP_TEST   = 0x0800,						// Test mode
	//-------------------------------------------------------------
	VEH_IS_FLY		= 0x1000,						// Flying object
  VEH_IS_UFO		= 0x2000,						// UFO Kind
} VEH_OPTION;

//-----------------------------------------------------------------
//  IMAGE structure
//-------------------------------------------------------------------------------
typedef struct {
  U_CHAR state;                   // Drag State
  short ht;                       // Image Height (pixel)
  short wd;                       // Image width  (pixel)
  int   dim;                      // Total pixels
  short  x0;                      // Window position
  short  y0;                      // Window position
  short  mx;                      // Mouse position
  short  my;                      // Mouse position
  U_INT *rgba;                    // Pixel map
} S_IMAGE;
//-----------------------------------------------------------------
//  Damage message
//-----------------------------------------------------------------
typedef struct {
  char    Severity;               // 0 None
  char    rfu;                    // 1 Warning (orange)
  Tag     snd;                    // 2 Sound tag
  char   *msg;                    // 3 Message
} DAMAGE_MSG;
//=====================================================================
//  Common fuel weight per gallon
//=====================================================================
#define FUEL_LBS_PER_GAL (float(6.02))
#define FUEL_EMPTY_QTY   (float(0.00001))
//-------------------------------------------------------------------------------
//  Request code to database access
//-------------------------------------------------------------------------------
  enum DBCODE { NO_REQUEST      = 0,
                COM_BY_AIRPORT  = 1,
                RWY_BY_AIRPORT  = 2,
                CTY_BY_ALLLOT   = 3,
                CTY_BY_CTYKEY   = 4,
                APT_BY_FILTER   = 5,
                NAV_BY_FILTER   = 6,
                WPT_BY_FILTER   = 7,
                STA_BY_COUNTRY  = 8,
                APT_BY_OFFSET   = 9,
                NAV_BY_OFFSET   =10,
                NDB_BY_OFFSET   =11,
                ILS_BY_AIRPORT  =12,
                APT_NEARESTONE  =13,              // The first nearest airport
                GPS_NR_AIRPORT  =14,              // Nearest Airports from cache
                GPS_GT_AIRPORT  =15,              // Airport by Ident/Name
                GPS_GT_RUNWAY   =16,              // Runway for airport
                GPS_GT_COMM     =17,              // Comm for airport
                GPS_GT_ILS      =18,              // ILS for airport
                GPS_GT_VOR      =19,              // VOR by Ident/Name
                GPS_NR_VOR      =20,              // Nearest VOR from cache
                GPS_GT_NDB      =21,              // NDB by Ident/Name
                GPS_NR_NDB      =22,              // Nearest NDB from cache
                GPS_GT_WPT      =23,              // WPT by Ident/name
                GPS_NR_WPT      =24,              // Neareast WPT from cache
                GPS_ANY_WPT     =25,              // Any waypoint by Ident
                REQUEST_END     =255,
      };
  //-----------Define GPS search mode ---------------------------------
  enum GPS_MODE { GPS_NO_MATCH = 0,         // Match meaningless
                  GPS_FS_MATCH = 1,         // First match
                  GPS_NX_MATCH = 2,         // Next match
                  GPS_PV_MATCH = 3,         // Previous match
  };
  //-----------RADIO TAG INDEX -----------------------------------------
  enum TAG_RADIO {  NAV_INDEX = 1,
                    COM_INDEX = 2,
                    ADF_INDEX = 3,
  };
//==============================================================================
class CNullSubsystem;
//==============================================================================
//
// CObject is the most basic object type and is the root ancestor of all
//   object classes
//	Send message is implemented at this level for intra messaging in the
//	aircraft
//==============================================================================
class CVehicleObject;
//------------------------------------------------------------------------------
class CObject {
protected:
public:
	CObject();
	EMessageResult CObject::Send_Message (SMessage *msg, CObject *obj);
	//-------------------------------------------------------------------------
	 virtual EMessageResult    ReceiveMessage(SMessage *msg) {return MSG_IGNORED;}
protected:
};

//===============================================================================
//  QGTHead is a basic class for sharing an object
//  It provide a user count and a delete mecanism when user count is 0
//===============================================================================
class QGTHead {
protected:
  pthread_mutex_t		mux;					// Mutex for lock
  unsigned int Users;
  //-----------------------------------------------------
public:
           QGTHead();
  virtual ~QGTHead() {}
  //----Count management --------------------------------
  void  IncUser();
  bool  DecUser();
  //-----------------------------------------------------
	virtual void GetTrace(int *t,int *x,int *z)		{;}
  virtual void TraceDelete() {};
};
//==============================================================================
//  CPTR is a smart pointer to a shared object
//  Methods are implemented in TerrainCache.cpp
//==============================================================================
class CPTR {
  //----Attribut is a pointer to a shared object --------
protected:
  QGTHead *obj;
  //----Methods -----------------------------------------
public:
  CPTR() {obj = 0;}
  //--------Define assignement and access operators ------
  void operator=(QGTHead *p);
  void operator=(QGTHead &n);
  void operator=(CPTR    &q);
  //--------Define Access operator ----------------------
  inline QGTHead* operator->() {return obj;}
  inline bool     operator==(QGTHead *itm) {return (obj == itm);}
  inline bool     Assigned()  {return (obj != 0);}
  inline bool     IsNull()    {return (0 == obj);}
  inline QGTHead *Obj()       {return obj;}
};


//============================================================================
//  Class CBaseBitmap:  Provide only virtual functions
//  This is base class for
//    CBitmapPBG
//    CBitmapPBM
//    CBitmapNUL
//============================================================================
class CBaseBitmap {
  //---This is the only attribute --------------------------------
protected:
  char    loaded;                     // Load indicator
  //--------------------------------------------------------------
public:
  virtual ~CBaseBitmap()  {loaded = 0;}
  //--------------------------------------------------------------
  virtual void DrawBitmap (SSurface *sf,int x,int y,int fr) {}
  virtual void FillTheRect(SSurface *sf,int x,int y,int wd,int ht,int fr){}
  virtual void DrawPartial(SSurface *sf,int x,int y,int x1,int y1,int x2,int y2,int fr){}
  virtual void DrawFrom   (SSurface *sf,int pos,int nbl, int x, int y){}
  virtual void GetBitmapSize(int *x, int *y) {*x = 0; *y = 0;}
  virtual int  NumBitmapFrames()  {return 0;}
  virtual void SetFrameNb(int k)  {};
  virtual U_CHAR *GetRGBA()       {return 0;}
  //--------------------------------------------------------------
  bool NotLoaded()  {return (loaded == 0);}

};
//============================================================================
//  Class  CBitmapNUL:  Substitute for CBitmapPBG or CBitmapPBM
//  Used when no bitmap is supplied or bitmap has error
//  This bitmap does nothing except intercepting call and avoid testing
//  if a bitmap exists or not
//============================================================================
class CBitmapNUL: public CBaseBitmap {
  int use;                            // Number of users
  //-------------------------------------------------------------
public:
  CBitmapNUL()  {use = 0;}
  void          IncUser()   {use++;}
  int           DecUser()   {use--; return use; }
  //--------No other supplied method ----------------------------
};
//============================================================================
//  Class CBITMAP
//============================================================================
class CBitmap
{
public:
  CBitmap(void);
  CBitmap(char * bmapname);
 ~CBitmap(void);

  // Override operators new and delete
  void  operator delete (void*);
  void *operator new (size_t size);

  const char * name(void) {return m_bm.bitmapName;};
  EBitmapType type(void) {return m_bm.type;};
  bool  Load(void);
  bool  Load_Bitmap(char * bname);

  void * Bitmap(void) {return m_bm.bitmap;};

  void  DrawBitmap (SSurface *sf,int x,int y,int fr);
  void  FillTheRect(SSurface *sf,int x,int y,int wd,int ht,int fr);
  void  DrawPartial(SSurface *sf,int x,int y,int x1,int y1,int x2,int y2,int fr);
  void  DrawFrom   (SSurface *sf,int pos,int nbl, int x, int y);
  void  GetBitmapSize(int *xSize, int *ySize);
  int   NumBitmapFrames( );
  void  FreeBitmap( );
  //------------------------------------------------------------
  void  AssignNUL();
  void  ChangeType();
  bool  NotLoaded()         {return ((CBaseBitmap*)m_bm.bitmap)->NotLoaded();   }
  void  SetFrameNb(int k)   {       ((CBaseBitmap*)m_bm.bitmap)->SetFrameNb(k); }
  U_CHAR *GetRGBA()         {return ((CBaseBitmap*)m_bm.bitmap)->GetRGBA();}
  //------------------------------------------------------------
protected:
  SBitmap m_bm;
};

//
//  INI functions
//

void LoadIniSettings (void);
void UnloadIniSettings (void);
void SaveIniSettings (void);
bool HasIniKey(char *section,char *key);
bool GetIniKey(char *section,char *key);
void GetIniVar (const char *section, const char *varname, int *value);
void GetIniFloat (const char *section, const char *varname, float *value);
void GetIniString (const char *section, const char *varname, char *strvar, int maxLength);
void SetIniVar (const char *section, const char *varname, int value);
void SetIniFloat (const char *section, const char *varname, float value);
void SetIniString (const char *section, const char *varname, char *strvar);
const char *GetIniValue(const char *section, const char *key);
bool IsSectionHere(const char *section);
//===================================================================================
//	FORWARD DECLARATION FOR ALL CLASSES WITH POINTERS
//===================================================================================
//-------Forward declaration for all .H files ----------------------
//----OBJECTS ----------------------------------------------------
class CCamera;
class CCameraObject;
class CCameraRunway;
class CCameraSpot;
class ClQueue;
class CSubsystem;
class CTileCache;
class CAirport;
class CNavaid;
class CRunway;
class CPaveRWY;
class CAptObject;
class CVehicleObject;             // Vehicle object
class CAudioManager;              // Audio sustem
class CExternalLightManager;      // Light manager
class CElectricalSystem;          // electrical system
class AutoPilot;									// Autopilot system
class CMagneticModel;             // Magnetic model
class CFuelSystem;                // Fuel system
class CKeyMap;                    // Keyboard interface
class CBitmapNUL;                 // Null bitmap
class CKeyDefinition;             // Key definition
class CKeyFile;                   // Default keys
class PlaneCheckList;                    // Checklist
//---NAVIGATION -------------------------------------------------
class CILS;
//---Line box ---------------------------------------------------
class CTgxLine;
//----CONTROL ---------------------------------------------------
class CAnimatedModel;             // Level of Detail
class CSlewManager;               // Slew manager
class CCursorManager;             // Cursor manager
class CTestBed;                   // Test bed
class SqlMGR;                     // SQL MAnager
class SqlTHREAD;                  // SQL Thread
class CJoysticksManager;          // Joystick handler
//class C3DMgr;                     // 3D world manager
class CSkyManager;                // Sky manager
class CWeatherManager;            // Weather Manager
class CCameraManager;             // Camera manager
class CCockpitManager;            // Cockpit manager
class CFuiManager;                // Windows manager
class CAtmosphereModelJSBSim;     // Atmosphere
class CEngine;                    // Engine instance
class CSuspension;                // Wheel suspension
class CSoundOBJ;                  // Sound object
class CSceneryDBM;								// Scenery set
//----PANEL -----------------------------------------------------
class CGauge;
class CKAP140Panel;
class CCockpitPanel;
class CPanel;                     // Current panel
//----MODEL 3D --------------------------------------------------
class CWobj;
class C3Dmodel;
class C3Dworld;
//----METEO -----------------------------------------------------
class CMeteoArea;
class CCloud;
class CCloudModel;
class CCloudSystem;                         // Cloud system
//----TERRAIN CACHE ---------------------------------------------
class CDbCacheMgr;								          // Nav cache
class TCacheMGR;                            // Terrain cache
class C_QGT;                                // Quarter Global Tile
class C_QTR;                                // QTR object
class C_SEA;                                // COAST object
class CAirportMgr;                          // Airport  Model
class CVertex;                              // Vertex
class CmQUAD;                               // QUAD
class C3DMgr;                               // 3D models
class CSuperTile;                            // Super Tile
class C_CDT;                                // Coast data
class CTextureWard;                         // Texture Hangar
//----FUI INTERFACE ----------------------------------------------
class CFuiComponent;
class CFuiPicture;
class CFuiButton;
class CFuiCloseButton;
class CFuiMiniButton;
class CFuiZoomButton;
class CFuiWindowTitle;
class CFuiWindow;
class CFuiTheme;
class CFuiThemeWidget;
class CFuiPage;
class CFuiCanva;
class CChkLine;
class CListBox;
class CFPlan;
//----WINDOWS --------------------------------------------------
class CFuiCkList;                 // Window check list
class CFuiFuel;                   // Fuel loadout
class CFuiLoad;                   // Variable loads
class CFuiChart;                  // Sectional chart
class CFuiList;
class CFuiProbe;
//----TOOLS -----------------------------------------------------
class CExport;
class CImport;
//===========================================================================
// External Lights
//===========================================================================
typedef enum {
  EXTERNAL_LIGHT_NONE     = 0,
  EXTERNAL_LIGHT_TAXI     = 1,
  EXTERNAL_LIGHT_LAND     = 2,
  EXTERNAL_LIGHT_STROBE   = 3,
  EXTERNAL_LIGHT_NAV      = 4,
  EXTERNAL_LIGHT_BEACON   = 5
} EExternalLightPurpose;

//=============================================================================
//  OSM Layer
//=============================================================================
#define OSM_LAYER_BLDG	(0)
#define OSM_LAYER_LITE  (1)
#define OSM_LAYER_DBLE  (2)
#define OSM_LAYER_GRND	(3)
#define OSM_LAYER_SIZE  (4)
//============================================================================
//  Define GAUGE BUS VALUES
//============================================================================
#define GAUGE_BUS_INT00 (0)					// must always return 0
#define GAUGE_BUS_INT01	(1)					// Must always return subsystem 'state'  value
#define GAUGE_BUS_INT02	(2)					// Must always return subsystem 'active' value
#define GAUGE_BUS_INT03	(3)					// Must always return subsystem 'indx'   value
//-----------------------------------------------------------------------------
#define GAUGE_BUS_INT04	(4)					// Specific to subsystem
#define GAUGE_BUS_INT05	(5)					// Specific to subsystem
#define GAUGE_BUS_INT06	(6)					// Specific to subsystem
#define GAUGE_BUS_INT07	(7)					// Specific to subsystem
#define GAUGE_BUS_INT08	(8)					// Specific to subsystem
//--------------------------------------
#define GAUGE_BUS_FLT01	(1)					// Specific to subsystem
#define GAUGE_BUS_FLT02	(2)					// Specific to subsystem
#define GAUGE_BUS_FLT03	(3)					// Specific to subsystem
#define GAUGE_BUS_FLT04	(4)					// Specific to subsystem
#define GAUGE_BUS_FLT05	(5)					// Specific to subsystem
#define GAUGE_BUS_FLT06	(6)					// Specific to subsystem
#define GAUGE_BUS_FLT07	(7)					// Specific to subsystem
#define GAUGE_BUS_FLT08	(8)					// Specific to subsystem

//=============================================================================
//  Define VBO offset
//=============================================================================
#define OFFSET_VBO(x) ((void *)(x))
//=============================================================================
//  Structure View PORT
//=============================================================================
struct VIEW_PORT {
  int x0;
  int y0;
  int wd;
  int ht;
};
//=============================================================================
//  Structure for terrain type
//=============================================================================
struct TC_TERRA {
  int   type;                                     // Terrain type
  char  nite;                                     // Night indicator
  char *desc;                                     // Terrain description
};
//=============================================================================
// Function type declaration for key map callbacks
//
typedef bool(*KeyCallbackPtr) (int keyid, int code, int modifiers);
typedef bool(*KeyGroupCB)(CKeyDefinition *kdf, int modifiers);
//=============================================================================
//  Structure to describe a rectangle in arcsec coordinates
//=============================================================================
struct TC_RECT {
  float SW_LON;                                  // SW longitude
  float SW_LAT;                                  // SW latitude
  float NE_LON;                                  // NW longitude
  float NE_LAT;                                  // NW latitude
  float alt;                                     // Altitude      
};
//=============================================================================
//  SPECIFIC POINT DEFINITION
//=============================================================================
struct TC_SPOINT {
  int x;
  int y;
  int type;
};
//=============================================================================
//  Structure to describe coefficient for vector director 
//=============================================================================
struct VECTOR_DIR {
	double		afa;											// Alpha coefficient (normal x)
	double		bta;											// Beta coefficient (norma y)
	double		gma;											// Gamma coefficient
	double		lgn;											// Lenght of normal
	SPosition org;											// Origin
	double    rdf;											// Reduction factor
};
//============================================================================
//	TEXTURE DIRECTORY
//============================================================================
#define TEXDIR_ART			(0)
#define TEXDIR_OSM_MD		(1)
#define TEXDIR_OSM_TX		(2)
//============================================================================
extern char *DupplicateString(char *s, int lgm);
//============================================================================
//  Structure texture name for 3D Part
//============================================================================
struct TEXT_NAME {
	int		dir;
	char *name;
	//-----------------------------------------------------------
	 TEXT_NAME()  {dir = TEXDIR_ART; name = 0;}
	//--- Destructor --------------------------------------------
	~TEXT_NAME() { if (name)  delete name; }
	//-----------------------------------------------------------
	void	SetTexture(char d, char *n) 
	{ dir = d;
		if (name)	delete [] name;
		name = DupplicateString(n,128);
	}
	//-----------------------------------------------------------
	char	GetDirectory()	{return dir;}
	char *GetName()				{return name;}
	//-------------------------------------------------------------
	void	Clear()	{if (name) delete name; name = 0;}
};

//=============================================================================
//  PIXEL DEFINITION
//=============================================================================
#define NE_PIX (0)
#define NW_PIX (1)
#define SW_PIX (2)
#define SE_PIX (3)
//---------------------------------------------------------
struct S_PIXEL {
	short	x;																	// X coordinate
	short	y;																	// Y Coordinate
	//---- Constructor --------------------------------------
	S_PIXEL::S_PIXEL()
	{	x = y = 0;}
};
//=============================================================================
//  COLOR DEFINITION
//=============================================================================
struct TC_COLORS {
  U_CHAR VT_R;                               // RED COEFFICIENT
  U_CHAR VT_G;                               // GRREN COEFFICIENT
  U_CHAR VT_B;                               // BLUE COEFFICIENT
  U_CHAR VT_A;                               // ALPHA COEFFICIENT
};
//=============================================================================
//  Vector 2D float
//=============================================================================
struct TC_4DF {
  float x0;
  float y0;
  float x1;
  float y1;

};

//=============================================================================
//  SCreen parameters
//=============================================================================
struct TC_SCREEN {
  char  in;                     // Inside indicator
  //-----------------------------------------------
  int   sx;                     // X width
  int   sy;                     // Y Height
  //-----------------------------------------------
  int   mx;                     // X max
  int   my;                     // Y max
};
//=============================================================================
//	UNIT SIZE
//=============================================================================
#define UNIT_OPENGL GL_DOUBLE
#define UNIT_GTAB   (GL_FLOAT)
//=============================================================================
//  VERTEX TABLE
//=============================================================================
struct TC_VTAB {
  float VT_S;                                     // S coordinate
  float VT_T;                                     // T coordinate
  float VT_X;                                     // X corrdinate
  float VT_Y;                                     // Y coordinate
  float VT_Z;                                     // Z coordinate
  //----Copy from another table -------------------------
  void  TC_VTAB::Dupplicate(TC_VTAB *s, int n)
  { TC_VTAB *dst = this;
  for (int k = 0; k<n; k++)  *dst++  = *s++;
  return; }
  //------------------------------------------------------
	void Copy(TC_VTAB &v)
	{	*this = v;	}
	//------------------------------------------------------
	void InvertXZ(TC_VTAB &v)
	{ VT_X = v.VT_Z;
		VT_Z = v.VT_X;
		VT_Y = v.VT_Y;
	}
	//------------------------------------------------------
	void InvertYZ(TC_VTAB &v)
	{ VT_X = v.VT_X;
		VT_Z = v.VT_Y;
		VT_Y = v.VT_Z;
	}
	//------------------------------------------------------
	void InvertXY(TC_VTAB &v)
	{ VT_X = v.VT_Y;
		VT_Z = v.VT_Z;
		VT_Y = v.VT_X;
	}
	//------------------------------------------------------
	void InvertHD1(TC_VTAB &v)
	{ VT_X =  v.VT_X;
		VT_Z =  v.VT_Y;
		VT_Y = -v.VT_Z;
	}
		//------------------------------------------------------
	void InvertHD2(TC_VTAB &v)
	{ VT_X =  v.VT_X;
		VT_Z = -v.VT_Y;
		VT_Y =  v.VT_Z;
	}
		//-----------------------------------------------------
	void Add(SVector &T)
	{	VT_X += T.x;
		VT_Y += T.y;
		VT_Z += T.z;
	}

};
//=============================================================================
//  VERTEX TABLE for ground description
//=============================================================================
struct TC_GTAB {
  //double GT_S;                                     // S coordinate
  //double GT_T;                                     // T coordinate
  //double GT_X;                                     // X corrdinate
  //double GT_Y;                                     // Y coordinate
  //double GT_Z;                                     // Z coordinate
	  
	float GT_S;                                     // S coordinate
  float GT_T;                                     // T coordinate
  float GT_X;                                     // X corrdinate
  float GT_Y;                                     // Y coordinate
  float GT_Z;                                     // Z coordinate

  //----Copy from another table -------------------------
  void  TC_GTAB::Dupplicate(TC_GTAB *s, int n)
  { TC_GTAB *dst = this;
  for (int k = 0; k<n; k++)  *dst++  = *s++;
  return; }
  //------------------------------------------------------
};
//=============================================================================
//  VERTEX TABLE FOR PANEL
//=============================================================================
struct VTP_2D {
  float VT_S;                                     // S coordinate
  float VT_T;                                     // T coordinate
  float VT_X;                                     // X corrdinate
  float VT_Y;                                     // Y coordinate
  //----Copy from another table -------------------------
  void  VTP_2D::Dupplicate(VTP_2D *s, int n)
  { VTP_2D *dst = this;
  for (int k = 0; k<n; k++)  *dst++  = *s++;
  return; }
  //------------------------------------------------------
};
//=============================================================================
//  Vector 2D float
//=============================================================================
struct F2_VERTEX {
  float VT_X;
  float VT_Y;
};
//=============================================================================
//  3D POINT DEFINITION
//=============================================================================
struct F3_VERTEX {
  float VT_X;
  float VT_Y;
  float VT_Z;
	//-------------------------------------------------
	void F3_VERTEX::Clear()
	{	VT_X = VT_Y = VT_Z = 0;	}
};
//=============================================================================
//  2D TEXTURE COORDINATES
//=============================================================================
struct F2_COORD {
  float VT_S;
  float VT_T;
};
//=============================================================================
//  VERTEX TABLE with NORMAL VECTOR and TEXTURE COORDINATES
//=============================================================================
struct GN_VTAB {
  float VT_S;                                     // S coordinate
  float VT_T;                                     // T coordinate
  float VN_X;                                     // X Normal
  float VN_Y;                                     // Y Normal
  float VN_Z;                                     // Z Normal
  float VT_X;                                     // X coordinate
  float VT_Y;                                     // Y coordinate
  float VT_Z;                                     // Z coordinate
	//--------------------------------------------------------------
	GN_VTAB::GN_VTAB() {VT_S = VT_T = VN_X = VN_Y = VN_Z = VT_X = VT_Y = VT_Z = 0;}
	//--------------------------------------------------------------
	void Add(SVector &T)
	{	VT_X += T.x;
		VT_Y += T.y;
		VT_Z += T.z;
	}
	//--------------------------------------------------------------
	void InvertHD1(GN_VTAB &v)
	{ VT_X =  v.VT_X;
		VT_Z =  v.VT_Y;
		VT_Y = -v.VT_Z;
	}
  //--------------------------------------------------------------
	void Copy(GN_VTAB &v)
	{	*this = v;	}
	//--------------------------------------------------------------
  void ROT2D(double *M)
	{	double X = VT_X;
		double Y = VT_Y;
		VT_X = (X * M[0]) + (Y * M[3]) + M[6];
		VT_Y = (X * M[1]) + (Y * M[4]) + M[7];
		X = VN_X;
		Y = VN_Y;
		VN_X = (X * M[0]) + (Y * M[3]) + M[6];
		VN_Y = (X * M[1]) + (Y * M[4]) + M[7];
		return;
	}
	//---------------------------------------------------------------
	void DupVTX(F3_VERTEX *V)
	{	VT_X	= V->VT_X;
		VT_Y  = V->VT_Y;
		VT_Z  = V->VT_Z;
	}
	//---------------------------------------------------------------
	void DupVNX(F3_VERTEX *V)
	{	VN_X	= V->VT_X;
		VN_Y  = V->VT_Y;
		VN_Z  = V->VT_Z;
	}
	//---------------------------------------------------------------
	void DupTVX(F2_COORD *V)
	{	VT_S	= V->VT_S;
		VT_T  = V->VT_T;
	}
	//---------------------------------------------------------------
	};
//=============================================================================
//  VERTEX TABLE with 2 TEXTURE COORDINATES and color values
//=============================================================================
struct C3_VTAB {
  float VT_S;                                     // S coordinate
  float VT_T;                                     // T coordinate
	float VT_R;																			// Red value
	float	VT_G;																			// Green value
	float VT_B;																			// Blu value
  float VT_X;                                     // X coordinate
  float VT_Y;                                     // Y coordinate
  float VT_Z;                                     // Z coordinate
	//--------------------------------------------------------------
};

//=============================================================================
//  VERTEX TABLE with NORMAL VECTOR
//=============================================================================
struct VT_VTAB {
  float VN_X;                                     // X Normal
  float VN_Y;                                     // Y Normal
  float VN_Z;                                     // Z Normal
  float VT_X;                                     // X coordinate
  float VT_Y;                                     // Y coordinate
  float VT_Z;                                     // Z coordinate
	//--------------------------------------------------------------
	VT_VTAB::VT_VTAB()
	{	VN_X	= VN_Y = VN_Z = 0;
		VT_X	= VT_Y = VT_Z = 0;
	}
};
//===========================================================================
//  Define SLEW mode
//===========================================================================
typedef enum {
  SLEW_STOP = 0,      // NO SLEW
  SLEW_MOVE = 1,      // Moving
  SLEW_LEVL = 2,      // Leveling
  SLEW_RCAM = 3,      // Rabbit cam slew
	SLEW_FPLM = 4,			// Flight plan node
} SLEW_MODE;
//=============================================================================
//  CAMERA CONTEXTE
//=============================================================================
struct CAMERA_CTX {
	char    sidn[12];																// Camera tag
  Tag     iden;                                   // Camera ident
  double  range;                                  // Range
  double  rmin;                                   // Min range
  double  rmax;                                   // max range
  double  theta;                                  // Angle theta (horizontal)
  double  phi;                                    // Angle phi   (vertical)
  float   fov;                                    // Field of view
	char		mode;
	//---------------------------------------------------------------
	CFPlan *fpln;																		// Flight plan
	U_INT		prof;																		// Profile
	//---------------------------------------------------------------
	SVector		ori;																	// World orientation
	SPosition pos;																	// World position
};
//=====================================================================
//  Tuple for loading table lookup
//=====================================================================
struct TUPPLE {
  float inp;                    // Input value
  float out;                    // Output value
  float dtv;                    // delta (y) / delta (x)
};
//=====================================================================
//  Tuple for loading table lookup with 2 output value
//=====================================================================
struct TUPPLE2 {
  float inp;                    // Input value
  float out1;                   // Output value 1
  float out2;                   // output value 2
};
//=====================================================================
//  Tuple for loading table lookup with 2 output value
//=====================================================================
struct PAIRF {
  float inp;                    // Input value
  float out;                   // Output value 1
};

//=====================================================================
//  Tuple for loading table lookup with 3 output value
//=====================================================================
struct TUPPLE3 {
  float inp;                    // Input value
  float out1;                   // Output value 1
  float out2;                   // output value 2
  float out3;                   // Output value 3
};

//============================================================================
//  Struct vertex 2D
//============================================================================
struct TC_BOUND {
  U_INT xmin;
  U_INT zmin;
  U_INT xmax;
  U_INT zmax;
};
//==============================================================================
//  PLOT PARAMETERS
//==============================================================================
#define PLOT_MENU_SIZE  (16)
#define PLOT_MENU_WIDTH (64)
class CDependent;
//-----------------------------------------------------------------------------
typedef struct {
   Tag  iden;                     // Destination
   Tag  type;                     // Data type
   char menu[PLOT_MENU_WIDTH];    // Menu type
} PLOT_PM;
//-----------------------------------------------------------------------------
typedef struct {
  CDependent *dpnd;               // Plotted component
  float *pvl;                     // pointer to value
  float yUnit;                    // number of unit in Y axis
  float val;                      // Value to plot
  float sign;                     // Sign of value
  char  mask[PLOT_MENU_WIDTH];    // Edit mask
} PLOT_PP;                        // Plot parameters

//=============================================================================
//  A simple 2D rectangle
//=============================================================================
class C_2DRECT {
  //-----public ---------------------
public:
  int x0;
  int y0;
  int x1;
  int y1;
  //---------------------------------
  C_2DRECT();
  void  Init(int x0,int y0, int x1,int y1);
  bool  IsHit(int x,int y);
};
//=============================================================================
//  POLYGON TABLE
//=============================================================================
class CPolygon {
  //-----Attributes ------------------------------------------------------
  short    nbv;                                      // Number of vertices
  short    rfu;                                      // Reserved
  U_INT    mode;
  TC_VTAB *vtb;                                      // Vertex table
  //----------------------------------------------------------------------
public:
  CPolygon();
 ~CPolygon();
  //------------------------------------------------------
  void      Draw();
  void      SetPolygons(int n);
  //------------------------------------------------------
  TC_VTAB  *GetVTAB()               {return vtb;}
  int       GetNBVT()               {return nbv;}
  //------------------------------------------------------
  inline    void  ClearCount()      {nbv = 0;}
  inline    void  AddCount(int k)   {nbv += k;}
  inline    void  SetMode(U_INT m)  {mode = m;}
  inline    void  IncCount()        {nbv++;}
};

//=============================================================================
//  Rectangular POLYGON
//=============================================================================
class CPolyREC {
  //----Attributes ------------------------------------------------------
  short nbv;                                        // Number of vertices
  short rfu;                                        // Reserved
  TC_VTAB vtab[4];                                  // 4 vertices
  //------------------------------------------------------------------
public:
  CPolyREC();
  //------------------------------------------------------------------
  void  InitQuad(CPanel *panel, SSurface *sf);
  void  Draw(bool bld);
  void  DrawStrip();
  //------------------------------------------------------------------
  TC_VTAB  *GetVTAB()               {return vtab;}
  float     GetX(int k)             {return vtab[k].VT_X;}
  float     GetY(int k)             {return vtab[k].VT_Y;}
};
//=============================================================================
//  POLYGON TABLE with normals
//=============================================================================
class CPoly3D {
  //----Attributes ------------------------------------------
  short    nbv;                                       // Number of vertices
  short    rf1;                                       // RFU
  GLenum   pmd;                                       // Polygon mode
  GN_VTAB *vtb;                                       // Vertice table
  //-----------------------------------------------------------------
public:
  CPoly3D();
  CPoly3D(int nv);
 ~CPoly3D();
  void      Allocate(int nv);
  void      Draw(U_INT txo);
  GN_VTAB  *GetVTAB(int k);
  GN_VTAB  *SetVTAB(int k,GN_VTAB *v);      
 //------------------------------------------------------------------
 inline GN_VTAB *GetVTAB()                    {return vtb;}
 inline void     SetMode(GLenum m)            {pmd = m;}
};
//=============================================================================
//  Texture Info
//=============================================================================
#define TC_TEXTURE_NAME_DIM (PATH_MAX)
#define TC_TEXTURE_NAME_NAM (64)
#define TC_LAST_INFO_BYTE   (TC_TEXTURE_NAME_DIM - 1)
//-------------------------------------------------------------------
struct  TEXT_INFO {
	U_INT			key;
  short     wd;                           // Texture width
  short     ht;                           // Texture height
	U_INT			dim;													// Texture dimension
  U_CHAR    apx;                          // alpha when pixel
	U_CHAR		azp;													// alpha when 0 pixle
	char			Dir;													// Directory
	char      bpp;                          // Byte per plan
  char      res;                          // Resolution     (if needed)
  char      type;                         // Terrain type   (if needed)
  U_INT     xOBJ;                         // Texture object (if needed)
  GLubyte  *mADR;                         // RGBA Memory addresse
	GLubyte  *nite;													// Night texture if needed
  char      name[TC_TEXTURE_NAME_NAM];    // Texture name
  char      path[TC_TEXTURE_NAME_DIM];    // Texture full name
	//--- Constructor --------------------------------------------------
  TEXT_INFO::TEXT_INFO()
	{	Dir			= 0;
		key			= 0;
		wd = ht = 0;
		apx			= 0xFF;
		azp			= 0x00;
		xOBJ		= 0;
		name[0]	= 0;
		mADR    = 0;
		nite    = 0;
	}
	};
	//===================================================================
struct TEXT_DEFN  {
  short wd;                               // Texture width
  short ht;                               // Texture height
  short nf;                               // Number of frames
  short pm;                               // gauge parameter
  U_INT xo;                               // Texture object
	U_INT bo;																// Buffer Object
  int   dm;                               // Dimension in pixel
  GLubyte *rgba;                          // RGBA pixels
  //-------------------------------------------------------------------
  TEXT_DEFN::TEXT_DEFN()
  { xo    = 0;
		bo		= 0;
    rgba  = 0;
    nf    = 1;
    dm    = 0;
  }
  //-------------------------------------------------------------------
  TEXT_DEFN::~TEXT_DEFN()
  { Free(); }
  //-------------------------------------------------------------------
  void  TEXT_DEFN::Copy(TEXT_INFO &t)
  { wd    = t.wd;
    ht    = t.ht;
    xo    = t.xOBJ;
    dm    = wd * ht;
    rgba  = t.mADR;
  }
  //-------------------------------------------------------------------
  void  TEXT_DEFN::Copy(TEXT_DEFN &t)
  { wd    = t.wd;
    ht    = t.ht;
    xo    = t.xo;
		bo		= t.bo;
    dm    = t.dm;
    nf    = t.nf;
    rgba  = t.rgba;
  }
  //-------------------------------------------------------------------
  void  TEXT_DEFN::Dupplicate(TEXT_DEFN &t)
  { Copy(t);
    if (0 == t.rgba)  return;
    int     nb  = dm * nf * 4;
    U_CHAR *bf  = new U_CHAR[nb];
    U_CHAR *src = t.rgba;
    U_CHAR *dst = bf;
    for (int k=0; k<nb; k++) *dst++ = *src++;
    rgba  = bf;
    return;
  }

  //------------------------------------------------------------------
  void TEXT_DEFN::Free()
  { if (xo) glDeleteTextures(1,&xo);
		if (bo) glDeleteBuffers(1,&bo);
    if (rgba) delete [] rgba;
    xo    = 0;
    rgba  = 0;
  }
  //------------------------------------------------------------------
  int Clamp(int fr)
  { if (fr <   0)     return 0;
    if (fr >= nf)     return (nf - 1);
    return fr;
  }
  //-------------------------------------------------------------------
};
//==============================================================================
// A CStreamObject is one which can be loaded and saved from/to a stream
//   file.  See Stream\Stream.h for more details and the API for stream
//   files
//==============================================================================
class CStreamFile;
//==============================================================================
class CStreamObject : public CObject {
public:
  virtual ~CStreamObject (void) {}
  virtual const char *GetClassName(void) {return "UnKnown";}	// JSDEV*
  virtual       char *GetIdString(void)  {return "????";}		// JSDEV*
  virtual int   Read (SStream *stream, Tag tag) {return TAG_IGNORED;}
  virtual void  ReadFinished (void) {}
  virtual void  Write (SStream *stream) {}
  virtual int   Read  (CStreamFile *sf, Tag tag){return TAG_IGNORED;}
  //-----------------------------------------------------------------
  void    ReadMonoStrip(SStream *str,TEXT_DEFN &txd,int *px, int *py);
  void    DecodeMonoName(SStream *str,char *fn,int *px,int *py);
  void    DecodeStripName(SStream *str, char *fn, TEXT_DEFN &txd);
  void    ReadStrip(SStream *str,TEXT_DEFN &txd);
	void		ReadBMAP(SStream *str,TEXT_DEFN &txd);
  void    ReadCADR(SStream *str,CGauge *mgg,TC_VTAB *qd);
  void    ReadSIZE(SStream *str,int *px,int *py, int *wd, int *ht);
  void    ReadSIZE(SStream *str,short *px,short *py, short *wd, short *ht);
	void		ReadSIZE(SStream *str,S_PIXEL *t, short *wd, short *ht);
  //-------Helper ----------------------------------------------------
  float Clamp(float lim, float val)
  { if (val < -lim)  return -lim;
    if (val > +lim)  return +lim;
    return val;
  }
  //------------------------------------------------------------------

};
//===================================================================================
//	CDrawByCamera:
//  Is a virtual class for object that may be drawed into small windows by a camera
//===================================================================================
class CDrawByCamera {
public:
  CDrawByCamera() {}
  virtual void PreDraw(CCamera *cam){}
  virtual void CamDraw(CCamera *cam){}
  virtual void EndDraw(CCamera *cam){}
};
//===================================================================================
//	Executable object
//	Must supply 3 methodes:  TimeSlice and Draw, DrawExternal
//===================================================================================
class CExecutable {
	//--- Attribute is next executable of same type -------
	CExecutable  *Next;
	//--- Methods -----------------------------------------
public:
	CExecutable() {Next = 0;}
	virtual ~CExecutable() {;}
	//-----------------------------------------------------
	CExecutable *NextExec()	{return Next;}
	//-----------------------------------------------------
	virtual int	  TimeSlice(float dT, U_INT frame)	{return 0;}
	virtual void	Draw() {;}
	virtual void  DrawExternal() {;}
	//------------------------------------------------------
	void SetNext(CExecutable *n)	{Next = n;}
	bool HasNext()								{return (Next != 0);}
};
//===================================================================================
//	Dispatching priority
//===================================================================================
#define PRIO_ABSOLUTE		(0)
#define PRIO_WEATHER		(1)
#define PRIO_TERRAIN		(2)
#define PRIO_DBCACHE		(3)
#define PRIO_SLEWMGR		(4)
#define PRIO_ATMOSPHERE	(5)
#define PRIO_UPLANE			(6)
#define PRIO_APLANE			(7)
#define PRIO_SIMVH			(8)
#define PRIO_SDK				(9)
#define PRIO_DLL				(10)
#define PRIO_EXTERNAL		(11)
#define PRIO_OTHERS			(12)
#define PRIO_MAX				(16)
//==============================================================================
//  RANDOM GENERATOR (SYSTEM DEPENDENT)
//==============================================================================
inline int RandomNumber(int mod) {return (rand() % mod);}
//==============================================================================
// JS: CRandomizer:
//  Produce a random number N in the requested range R every time T.  The 
//  value N is reached in T secondes from the previous one
//  vdeb:     the lower bound
//  amp:			Range amplitude
//	aval:		  Actual random value until next time T
//	cTim:			Time interval T
//------------------------------------------------------------------------------
//  T:     Time constant to reach the next random value
//==============================================================================
class CRandomizer {
protected:
  //--- ATTRIBUTES -----------------------------------------
  int   amp;                  // Amplitutde
  float vdeb;                 // Deb value
  float aval;                 // Actual value
  float tval;                 // Target value
  float dval;                 // delta value
  float cTim;                 // Time constant
  float timr;                 // Internal timer
  //--- Methods --------------------------------------------
public:
  CRandomizer();
  //--------------------------------------------------------
  void  Set(float db,int amp, float T);     // Start new serie
  void  Range(float  db, int amp);
  float TimeSlice(float dT);                // Time slice
  //---------------------------------------------------------
  inline float GetValue() {return aval;}
};
//===================================================================================
//  ValGenerator:  Flatten a value according to time and target
//===================================================================================
class ValGenerator {
protected:
  //--- ATTRIBUTE ------------------------------------------
  char		mode;															// Mode computing
  float		Targ;															// Target value
  float		cVal;															// Current value
  float		timK;															// Time constant
	float		time;															// Timer
	int			rand;															// Random interval									
  //--- METHODS --------------------------------------------
public:
  ValGenerator(char md, float tm);
  ValGenerator();
	void		StartSin(float a,int R);
  float   TimeSlice(float dT);
  //--------------------------------------------------------
  inline void   Conf(char md,float tm) {mode = md; timK = tm;}
  inline void   Set(float v)    {Targ = v;}
  inline float  Get()           {return cVal;}
};
//===================================================================================
//	CmHead is the base class for all managed objects in the data
//  base Cache manager 
//===================================================================================
//-----------------------------------------------------------------
//		Define Queue type
//		Add more when needed
//-----------------------------------------------------------------
enum QTYPE {	TIL = 0,					// Tile cache
				      APT = 1,					// Airport
				      VOR = 2,					// VOR
				      NDB	= 3,					// NDB
				      ILS	= 4,					// ILS
				      COM = 5,					// Commm radio
				      RWY	= 6,					// RUNWAYS
				      WPT	= 7,					// Waypoints
              OBN = 8,          // Obstruction
				      QDIM =9,					// Queue size
              //----OTHER OBJECT ------------
              WOB = 20,         // 3D Object       
              OTH = 99,         // Other object
				};
//-----------------------------------------------------------------
//  Define object type
//-----------------------------------------------------------------
enum OTYPE {  ANY = 0x00,       // Any
              SHR = 0x01,       // Shared
              DBM = 0x03,       // DBM
};
//----------SIGNAL TYPE -------------------------------------------
#define SIGNAL_OFF 0
#define SIGNAL_VOR 1
#define SIGNAL_ILS 2
#define SIGNAL_COM 4
#define SIGNAL_WPT 8
//====================================================================
//  Data to control ILS
//====================================================================
struct LND_DATA {
		CRunway   *rwy;														// Related runway
    CILS      *ils;                           // ILS object
		LND_DATA  *opp;														// Opposite 
    SPosition  lndP;                          // Landing point (on tarmac)
    SPosition  refP;                          // reference point
    SPosition  fwdP;                          // Forward point
		SPosition  midP;													// Mid point
		SPosition  tkoP;													// Take off point
		//-----------------------------------------------------------
		double     orie;													// Runway true orientation
		//-----------------------------------------------------------
    float      disF;                          // Distance in feet
    float      errG;                          // Glide error (in tan unit)
    float      gTan;                          // Tan of glide slope
    float      altT;                          // altitude above threshold
		//--- End (0 == hi, 1 == low) ------------
		char			 rEnd;													// Runway end
		char			 rfu1;													// Reserved
		U_SHORT    sect;													// Runway sector
		//--- LAnding direction ------------------------
		float      lnDIR;												  // Landing  with mag dev
		//--- Runway ident -----------------------------
		char       ridn[4];												// Runway end identifier
    //--- Airport definition -----------------------
    CAptObject *apo;
		//------------------------------------------------
};
//===================================================================================
//  NOTE:  dLon and dLat are working area used for several purposes 
//===================================================================================
class CmHead {
protected:
  friend class ClQueue;
	//------------define state ---------------------------------
#define FREE 0						// Free state
#define ACTV 1						// Active
#define TDEL 2						// To delete
	//------------Attributes -----------------------------------
	char          idn[4];						// Identity
	pthread_mutex_t		mux;					// Mutex for lock
	U_CHAR				State;					  // State
	U_CHAR				uCount;					  // User count
  U_CHAR        oTyp;             // Object type
  U_CHAR        qAct;             // Index of active queue
	//----------------------------------------------------------
	U_INT				NoFrame;				    // Frame stamp
	//--- User pointer -----------------------------------------
	void         *uptr;							// User data
  //------------Distance -------------------------------------
  short         dLon;             // Longitudinal component
  short         dLat;             // Lattitude component
  //------------Cosinus latitude------------------------------
  float         nmFactor;         // 1nm at latitude xx
  //----------------------------------------------------------
	CmHead			*Cnext;					  // queue Q1 next item
	CmHead			*Cprev;					  // queue Q1 previous item
	CmHead			*Tnext;					  // queue Q2 next item
	CmHead			*Tprev;					  // queue Q2 previous item
	//------------------------------------------------------------
public:
	CmHead(OTYPE qo,QTYPE qa, char * idn);  // New object
  CmHead();                               // Default constructor
  virtual ~CmHead() {}                    // Virtual destructor
  virtual char      *GetName()        {return 0;}  // Get Name
	virtual char      *GetKey()					{return "NONE";} // Database key
  virtual char      *GetIdent()       {return "";} // ICAO identification
  virtual char      *GetCountry()     {return "";}
  virtual char      *GetEFreq()       {return "";}
	virtual int	       GetType()	      {return 0; }// Object type
  virtual float      GetMagDev()      {return 0; }
  virtual float      GetRadial()      {return 0; }
	virtual float			 GetTrueRadial()	{return 0;} 
  virtual float      GetNmiles()      {return 0; }
  virtual float      GetElevation()   {return 0;}
  virtual U_INT      GetRecNo()       {return 0;}
	virtual double     GetRefDirection(){return 0;}
	virtual float      GetVrtDeviation(){return 0;}
	virtual double		 Sensibility()		{return 0;}
	virtual U_CHAR		 SignalType()     {return SIGNAL_OFF;}
	virtual float			 GetFeetDistance(){return 0;}
	virtual float			 GetGlide()       {return 0;}
	virtual char			 GetTrackMode()		{return 'T';}
	//------------------------------------------------------------
	virtual bool			 MayMove()				{return false;}
	//------------------------------------------------------------
  virtual void       RefreshStation(U_INT FrNo) {return;}
  virtual float      GetLatitude()    {return 0;}
  virtual float      GetLongitude()   {return 0;}
  virtual float      GetFrequency()   {return 0;}
  virtual SPosition  GetPosition();
  virtual SPosition *ObjPosition()    {return 0;}
	//------------------------------------------------------------
	virtual LND_DATA  *GetLandSpot()		{return 0;}
	//------------------------------------------------------------
	virtual void			 SetMagneticOBS(float d)   {;}
	virtual void       SetRefDirection(float d)	{;}
	virtual void			 SetNmiles(float m) {;}
	virtual void			 SetFeet(float f)   {;}
	//--- Refresh signal -----------------------------------------
	virtual	CmHead    *Select(U_INT frame,float freq) {return 0;}
	//------------------------------------------------------------
	virtual	void  DecUser();				          // Decrement user count
	        void  IncUser(void);					    // Increment user count
	        bool  NeedUpdate(U_INT FrNo);		  // Need update
          int   GetRadioIndex();
          void  SetState(U_CHAR sta);
	virtual	void  Trace(char *op,U_INT FrNo= 0,U_INT key= 0){};
  //----inline method ------------------------------------------
  inline int    GetDistLat(void)        {return dLat; }
  inline void   SetDistLat(short lat)   {dLat = lat;  }  
  inline int    GetDistLon(void)        {return dLon; }
  inline void   SetDistLon(short lon)   {dLon = lon;  }
	//------------------------------------------------------------
	inline void	 *GetUPTR()								{return uptr;}
	inline void   SetUPTR(void *p)				{uptr = p;}
	inline bool   HasUPTR()								{return (0 != uptr);}
	inline bool   NoUPTR()								{return (0 == uptr);}
	//------------------------------------------------------------
  inline float  GetNmFactor()           {return nmFactor; }
  inline  CmHead*  NextInQ1()						{return Cnext; }
  inline  CmHead*  PrevInQ1()						{return Cprev; }
  inline  CmHead*  NextInQ2()						{return Tnext; }
  inline QTYPE  GetActiveQ(void)        {return (QTYPE)(qAct); }
  inline OTYPE  GetObjType(void)        {return (OTYPE)(oTyp); }
	//------------------------------------------------------------
  inline void   Lock()                  {pthread_mutex_lock  (&mux);}
  inline void   Unlock()                {pthread_mutex_unlock(&mux);}
  inline bool   IsUsed()                {return (0 != uCount);}
	//-------------------------------------------------------------
	inline bool   IsNot(U_CHAR t)					{return (qAct != t);}
	inline bool   Isa(char t)							{return (qAct == t);}
  //-------------------------------------------------------------
  inline bool   IsShared()              {return (oTyp & SHR);}
};
//=====================================================================
//--------Structure Runway end --------------------------------
struct RWEND { 
    SPosition pos;                  // END Position
    char  rwid[4];                  // Runway ID
		float	ifrq;											// ILS frequency
		char  ilsD[8];									// Frequency edited
    int   dx;                       // Distance to origin
    int   dy;                       // (dito)
    int   cx;                       // Corner coordinate
    int   cy;                       // (dito)
		//----------------------------------------------------
		RWEND::RWEND()
		{	ifrq = 0; ilsD[0] = 0;}
};

//====================================================================
#define RWY_HI_END 0
#define RWY_LO_END 1
//====================================================================
#define SLOT_NAME_DIM (64)
#define SLOT_NO_MVUP  (0x01)        // No move up
#define SLOT_NO_MVDN  (0x02)        // No move down
#define SLOT_NO_INSR  (0x04)        // No insert
#define SLOT_NO_DELT  (0x08)        // No delete
#define SLOT_NO_OPER  (0x0F)        // No operation allowed
//====================================================================
//  CSLot is used for individual line in a list box
//  This is the base class.
//  Each list box should supply a derived class of CSlot
//====================================================================
class CSlot {
  //============SLOT ATTRIBUTES ===================================
 protected:
    //------Sequence number ---------------------------------------
    U_INT        Seq;                             // Slot number    
    //-------------------------------------------------------------
    U_CHAR       Rfu1;                            // Reserved
    U_CHAR       Fixed;                           // 0 slot is dynamic, 1 is fixed
    //-------------------------------------------------------------
    U_CHAR       nLIN;                            // Total line
    U_CHAR       cLIN;                            // Current line
    //-------------------------------------------------------------
    U_LONG       Offset;                          // Record offset
    char         Name[SLOT_NAME_DIM];             // Name of object;
 //--------------Vector to line edition ---------------------------
 public:
   CSlot();
	 CSlot(char d);
	 void		Init();
   int    GetTotLines() {return nLIN;}
   int    GetCurLine()  {return cLIN;}
 //-----------Virtual functions --------------------------------
   virtual ~CSlot() {}
   virtual  void    GetPosition(SPosition &p)   {}
   virtual  char *  GetSlotKey()  {return "";}
   virtual  void    CleanSlot() {}
   virtual  bool    Match(void *key)  {return false;}
   virtual  void    Print(CFuiList *w,U_CHAR ln);
   virtual  void    Title(CFuiList *w) {Print(w,0);}
   //-----------------------------------------------------------
   virtual  void    SetSlotKey(char *k)   {}
 //-----------Generic access -----------------------------------
  inline   void    SetOFS(U_LONG of)            {Offset = of;}
  inline   U_LONG  GetOFS()                     {return Offset;}
  inline   void    FixeIt()                     {Fixed = 1;} 
  inline   bool    IsNotFixed()                 {return  (Fixed == 0);}
  inline   bool    IsFixed()                    {return  (Fixed != 0);}
  inline   void    SetCurLine(char nl)          {cLIN = nl;}
  inline   void    SetTotLine(char nl)          {nLIN = nl;}
  inline   void    SetSlotSeq(U_INT n)          {Seq  = n;}
  //--------------------------------------------------------------
  inline   void    SetSlotName(char *nm)        {strncpy(Name,nm,SLOT_NAME_DIM); Name[SLOT_NAME_DIM-1] = 0;}
  inline   char   *GetSlotName()                {return Name;}
  inline   int     GetSlotSeq()                 {return Seq;}
};

//=====================================================================
//  SMart pointer to CmHead
//  All components should use this class pointer to access
//  a CILS object allocated  by the DataBase cache Mgr
//=====================================================================
class CObjPtr {
  //----Attributes ---------------------------------------
private:
  CmHead *ptr;
public:
  CObjPtr(CmHead *p);
  CObjPtr();
  CObjPtr(CmHead &n);
 ~CObjPtr();
  //--------Define assignement and access operators ------
  void operator=(CmHead *p);
  void operator=(CmHead &n);
  void operator=(CObjPtr   &q);
  //--------Define Access operator ----------------------
  inline CmHead* operator->() {return ptr;}
  inline bool       operator==(CmHead *obj) {return (ptr == obj);}
  inline bool       Assigned()  {return (ptr != 0);}
  inline bool       IsNull()    {return (0 == ptr);}
  inline CmHead *Pointer()      {return ptr;}
};

//==============================================================================
//  Option 
//==============================================================================
class COption {
protected:
  //---Attribute is the set of options ---------------------------
  U_INT   prop;
public:
  COption::COption() {prop = 0;}
  U_INT Get(U_INT p)    {return (prop & p);}    // Return property
  void  Rep(U_INT p)    {prop  =  p;}           // Replace property
  void  Set(U_INT p)    {prop |= p;}            // Set property
  void  Raz(U_INT p)    {prop &= (-1 - p);}     // Clear property
  void  Toggle(U_INT p);                        // Swap property
	//-------------------------------------------------------------
	U_INT GetAll()				{return prop;}
  //-------------------------------------------------------------
  char  Has(U_INT p)    {return (prop & p)?(1):(0);}
  char  Not(U_INT p)    {return (prop & p)?(0):(1);}
};
//============================================================================
//  STRUCTURE FOR COAST LINE HEADER
//============================================================================
struct COAST_HEADER {
	Tag			mem;													// Memory identifier
	U_SHORT nbp;													// Number of polygon
	U_SHORT rfu;													// Not used
	};
//============================================================================
//  STRUCTURE FOR COAST DETAIL TILE
//============================================================================
struct COAST_VERTEX {
  U_SHORT Nbv;                        // Number of vertices
  U_CHAR  Out;                        // Outside indicator
  U_CHAR  Num;                        // Vertex number
  short   xPix;                       // Pixel X coordinate
  short   zPix;                       // Pixel Z coordinate
};
//=========================================================================
//  Region RECORD
//=========================================================================
typedef struct {
    C_QGT *qgt;
    U_INT key;
    U_INT qtx;                                      // QGT X index
    U_INT qtz;                                      // QGT Z index
    U_INT dtx;                                      // Base X key
    U_INT dtz;                                      // Base Z key
    U_INT lgx;                                      // X-Dimension
    U_INT lgz;                                      // Z-Dimension
    U_INT sub;                                      // Sub division level
    U_INT type;                                     // Region type
    U_INT val;                                      // Value
    int   nbv;                                      // Number of values
    int *data;                                     // Array of elevation
} REGION_REC ;
//-------------------------------------------------------------------------
typedef enum {
  EL_UNIC = 0,
  EL_MONO = 1,
  EL_MULT = 2,
} ElevationEtype;
//=========================================================================
//  COAST RECORD IN DATA BASE
//=========================================================================
typedef struct {
    U_INT qtk;                                      // QGT key
    U_INT dtk;                                      // QGT detail
    short nbv;                                      // Number of vertices
    U_INT dim;                                      // Dimension
    char *data;                                     // Polygon definitions
} COAST_REC ;
//=====================================================================
//	Queue Header. Used for queuing D2_POINT object
//=====================================================================
template <class T> class Queue {
protected:
	//--------------Attributes ------------------------------------
	pthread_mutex_t		mux;					  // Mutex for lock
	U_SHORT			NbOb;									// Item number
	T	 *First;					              // First object in queue
	T	 *Last;													// Last  object in queue
	T  *Prev;
public:
  Queue();
 ~Queue();
  //---------inline ---------------------------------------------
  inline  T*  GetFirst()         {return First;}
  inline  T*  GetLast()          {return Last; }
  inline  T*  NextFrom(T *p)     {return (p)?(p->next):(0);}
  inline  T*  PrevFrom(T *p)     {return (p)?(p->prev):(0);}
  inline  U_SHORT     GetNbObj() {return NbOb;}
  inline  bool        NotEmpty() {return (0 != NbOb);}
	inline  bool				IsEmpty()	 {return (0 == NbOb);}
	//--- Circular links -----------------------------------------
	inline  T*  CyNext(T *p)	{T* n = p->next; if (0 == n) n = First; return n;}
	inline  T*  CyPrev(T *p)	{T* n = p->prev; if (0 == n) n = Last;  return n;}
	//------------------------------------------------------------
	inline  void  Update(int k,T *p, T *q) {NbOb += k; if (p == Last) Last = q;}
	//------------------------------------------------------------
  inline void Lock()                {pthread_mutex_lock (&mux);}
  inline void UnLock()              {pthread_mutex_unlock (&mux);}
	//------------------------------------------------------------
  void        Clear();
  void        PutLast (T *obj);
  void        PutBefore(T *ne,T *obj);
	void        PutHead(T *obj);
  T          *Detach(T *obj);
  T          *Pop();
	void				SwitchToLast(T *obj);
	void        TransferQ(Queue<T> &h);
	void				ReceiveQ(Queue<T>  &h);
	T          *Remove(T *obj);
	//--------------------------------------------------------------------
	T					 *SwapFirst()		{T *h = First; First = 0;					return h; }
	T					 *SwapLast()		{T *e = Last; Last = 0; NbOb = 0; return e; }
};
//==========================================================================
//  GENERIC QUEUE MANAGEMENT
//==========================================================================
template <class T> Queue<T>::Queue()
{ pthread_mutex_init (&mux,0);
	NbOb    = 0;
  First   = 0;
  Last    = 0;
  Prev    = 0;
}
//----------------------------------------------------------
//	Destory all elements
//----------------------------------------------------------
template <class T> Queue<T>::~Queue()
{	Clear();	}
//----------------------------------------------------------
//	Insert element Obj at end of queue
//----------------------------------------------------------
template <class T> void	Queue<T>::PutLast (T *obj)
{	NbOb++;								                    // Increment count
	obj->next	 = 0;							              // No next
	T *lo = Last;															// Get the last object
	obj->prev	 = lo;							            // Link to previous
	Last	     = obj;							            // This is the last
	if (lo == NULL)	  First    = obj;					// Queue was empty
	if (lo != NULL)		lo->next = obj;					// Link previous to new
	return ;	
}
//----------------------------------------------------------
//	Insert element Obj at Head of queue
//----------------------------------------------------------
template <class T> void Queue<T>::PutHead(T *obj)
{	NbOb++;
  T *nxt = First;
	obj->next = nxt;
	obj->prev = 0;
	First			= obj;
	if (nxt)	nxt->prev = obj;
	else			Last			= obj;
	return;
}
//---------------------------------------------------------
//	Insert new element ne after object obj
//----------------------------------------------------------
template <class T> void Queue<T>::PutBefore(T *ne,T *obj)
{ T *prv = (obj)?(obj->prev):(Last);
  if (prv)  prv->next = ne;
  ne->next  = obj;
  if (obj)  obj->prev = ne;
  ne->prev  = prv;
  if (0 ==     obj)   Last  = ne;
  if (First == obj)   First = ne;
  NbOb++;
  return;  
}
//----------------------------------------------------------
//	Detach element  obj from Queue return the previous
//----------------------------------------------------------
template <class T> T* Queue<T>::Detach(T *obj)
{	T	*nx = obj->next;					      // next object
	T	*pv = obj->prev;					      // Previous
	NbOb--;														// Decrease count
	if (nx)		nx->prev = pv;					// Relink next
	if (pv)		pv->next = nx;					// Relink previous
	if (First == obj)	    First = nx;	// New first
	if (Last  == obj)	    Last  = pv;	// New last
	obj->next	= 0;								    // Clear detached next
	obj->prev = 0;								    // Clear detached previous
	return pv;	}
//---------------------------------------------------------------
//	Detach element  obj from Queue without destroying the pointer
//---------------------------------------------------------------
template <class T> T* Queue<T>::Remove(T *obj)
{	T	*nx = obj->next;					      // next object
	T	*pv = obj->prev;					      // Previous
	NbOb--;														// Decrease count
	if (nx)		nx->prev = pv;					// Relink next
	if (pv)		pv->next = nx;					// Relink previous
	if (First == obj)	    First = nx;	// New first
	if (Last  == obj)	    Last  = pv;	// New last
	return obj;	}
//---------------------------------------------------------------
//	Detach element  obj from Queue and put at end of Queue
//---------------------------------------------------------------
template <class T> void Queue<T>::SwitchToLast(T *obj)
{	Detach(obj);
	PutLast(obj);
}
//---------------------------------------------------------------------
//	Pop the first object from Queue
//---------------------------------------------------------------------
template <class T> T*	Queue<T>::Pop()	
{	T	*obj	  = First;															// Pop the first
	if (obj)	{First  = obj->next; NbOb--; }		    // Update header
	if (First)	First->prev = 0;										// First has no previous
	if (Last == obj)	  Last = 0;					          // Queue is now empty
	if (0 == obj)	return obj;				
	obj->next	= 0;
	obj->prev	= 0;
	return obj;	}
//-----------------------------------------------------------------------
//	Delete all objects in a queue
//-------------------------------------------------------------------------
template <class T> void Queue<T>::Clear()
{	T *obj = Pop();
	while (obj)
	{	delete obj;
		obj	= Pop();
	}
	return;
}
//-----------------------------------------------------------------------
//	Transfer Queue
//-------------------------------------------------------------------------
template <class T> void Queue<T>::TransferQ(Queue<T> &Q)
{ Q.First = First;
	Q.Last  = Last;
	Q.NbOb	= NbOb;
	Q.Prev  = 0;
	First = Last = Prev = 0;
	NbOb	= 0;
	return;
}
//-----------------------------------------------------------------------
//	Receive Queue
//-------------------------------------------------------------------------
template <class T> void Queue<T>::ReceiveQ(Queue<T> &Q)
{	NbOb	= Q.NbOb;
  First = Q.First;
	Last	= Q.Last;
	Prev  = 0;
	Q.First = Q.Last = Q.Prev = 0;
	return;
}
//===================================================================================
//    Structure used for makink continuous text from a list of pointers
//===================================================================================
class TXT_LIST {
  public:
    SFont    *font;                     // Font
    U_INT     color;                    // Text color
    U_SHORT   NbPtr;                    // Number of pointers
    U_SHORT   CuPtr;                    // Current pointer
    U_SHORT   wkPtr;                    // Working index
    char     *data;                     // Current data
    char    **List;                     // Pointer to a list of pointers
    //----------Methods -----------------------------------------------------------
    void    Init(U_SHORT nb, char **lst);
    void    Restart();
    void    Reset();
    char   *GetNextAddr(char *txt);
    void    Increment();
    //-----------------------------------------------------------------------------
    inline  bool  HasMore()       {return (*data != 0);}
};
//=============================================================================
//  Radio Interface
//=============================================================================
#define RADIO_MODE_TRACK	('T')
#define RADIO_MODE_DIRECT ('D')
//=============================================================================
//  Radio Interface
//=============================================================================
//-----------COMPUTED VALUE FOR EXTERNAL GAUGES -----------------------
struct BUS_RADIO {U_CHAR    rnum;       // Radio num
                  U_CHAR    actv;       // Activity
                  U_CHAR    flag;       // Flag type
                  U_CHAR    ntyp;       // Nav Type
									//----------------------------------------
									U_CHAR    mode;				// Radio mode
									U_CHAR    rfu1;				// Future use
									//----------------------------------------
                  short     xOBS;       // External OBS
                  double    hREF;       // Lateral reference
                  double    radi;       // ILS/VOR radial
									double		rDEV;				// Relative deviation
                  double    hDEV;       // Lateral Deviation
                  double    gDEV;       // Glide     Deviation
                  double    mdis;       // Distance in miles
                  double    fdis;       // Distance in feet
                  double    mdev;       // Magnetic deviation
                  double    aDir;       // Aircraft direction (relative to VOR/ILS)
                  double    iAng;       // Intercept angle plane to OBS/ILS
                  double    sens;       // Horizontal sensibility
                  CmHead   *rSRC;       // Radio Source
	//----------------------------------------------------------
} ;
//=============================================================
//  Structure to describe a waypoint for GPS Bendix King
//=============================================================
typedef struct {
  short    wpNO;                // Number when wp is from a flight plan
  CObjPtr  wpAD;                // Waypoint description
  float    wpRT;                // Desired Route
  float    wpDR;                // Direction To waypoint
} GPSpoint;
//=====================================================================================
//  Define callback functions for Database Cache
//=====================================================================================
typedef void (HdrFunCB(CmHead   *obj,CTileCache *tc));          // Call back for object
typedef void (RwyFunCB(CRunway  *apt,CTileCache *tc));          // Call back for runways
typedef void (LinFunCB(CmHead   *obj,void *pm));                // Call back for LinSlot
typedef void (AptFunCB(CmHead   *obj));                         // Call back for airports
typedef void (PavFunCB(CPaveRWY *pav,CAptObject *apo));         // Call back for Pavement
typedef void (tgxFunCB(CTgxLine *tgx));                         // Call back for generic texture
typedef void (ElvFunCB(REGION_REC &));                          // Call back for elevation
typedef void (IntFunCB(U_INT k, void *obj));		  							// Call back for integer 
//======================================================================================
/*! \brief "Go To Failure Output" :-) Exit application due to fatal error.
 *
 * Exit the application ("Go To Failure Output"...definitely not "Get the &#^$ out"
 *   as some people might have you believe :-)  This simple class definition allows
 *   varargs to be used to generate the failure message.  The macro below
 *   will automatically fill in the source code filename and line number
 *
 * Thanks to Paul Mclachlan's article at:
 * http://www.codeproject.com/debug/location_trace.asp
 */
//--------------------------------------------------------------------------------------

class GTFO {
private:
  char      m_file[1024];
  int       m_line;

public:
    GTFO (const char* file = NULL, int line = 0) : m_line(line) { strncpy (m_file, file,1023); }
  void operator() (const char *fmt = NULL, ...);
};

#define gtfo (GTFO(__FILE__, __LINE__))

//--------------------------------------------------------------------------
/*! \brief Log a warning about unexpected (but not fatal) situations
 *q
 * If logging is enabled, then referencing the \a WARNINGLOG macro will
 *   result in the appropriate message being written to the log.  Warnings
 *   should be used for situations which are unexpected but not fatal.  It
 *   is expected that end-users may need to refer to the warning log to
 *   troubleshoot why a particular add-on is not working on their system.
 */
//--------------------------------------------------------------------------
class WARN {
private:
  char      m_file[1024];
  int       m_line;

public:
  WARN (const char* file = NULL, int line = 0) : m_line(line) { strncpy (m_file, file,1023); }
  void operator() (const char *fmt = NULL, ...);
};

#define WARNINGLOG (WARN(__FILE__, __LINE__))
//-------------------------------------------------------------------------
//	JSDEV* TERRA log.   This log serve to trace changed tiles
//
//--------------------------------------------------------------------------
class TERRA {
private:
	char msg[1024];
public:
	TERRA(const char *fmt = NULL, ...);
};
//-------------------------------------------------------------------------
//	JSDEV* TERRA log.   This log serve to trace changed tiles
//
//--------------------------------------------------------------------------
class STREETLOG {
private:
	char msg[1024];
public:
	STREETLOG(const char *fmt = NULL, ...);
};

//-------------------------------------------------------------------------
//	JSDEV* Trace log.   This log serve to trace various process
//
//--------------------------------------------------------------------------
class TRACE {
private:
	char msg[1024];
public:
	TRACE(const char *fmt = NULL, ...);
};
//-------------------------------------------------------------------------
//	JSDEV* AERO log.   This log serve to trace aero data
//
//--------------------------------------------------------------------------
class AERO {
private:
	char msg[1024];
public:
	AERO(const char *fmt = NULL, ...);
};
//-------------------------------------------------------------------------
//	JSDEV* WING log.   This log serve to trace wing data
//
//--------------------------------------------------------------------------
class WING {
private:
	char msg[1024];
public:
	WING(const char *fmt = NULL, ...);
};
//-------------------------------------------------------------------------
//	JSDEV* Scenery log.   This log serve to trace scenery process
//
//--------------------------------------------------------------------------
class SCENE {
private:
	char msg[1024];
public:
	SCENE(const char *fmt = NULL, ...);
};
#define SCENERYLOG (SCENE(__FILE__, __LINE__))
//-------------------------------------------------------------------------
/*! \brief Log a debugging message
 *
 * If debug logging is enabled, then referencing the \a DEBUGLOG macro will
 *   result in the message being written to the debug log.  Debug messages
 *   are only intended for use by the development team and would not typically
 *   be used by end-users.
 */
class DEBUG {
private:
  char      m_file[1024];
  int       m_line;

public:
  DEBUG (const char* file = NULL, int line = 0) : m_line(line) { strncpy (m_file, file,1023); }
  void operator() (const char *fmt = NULL, ...);
};

#define DEBUGLOG (DEBUG(__FILE__, __LINE__))



#if defined(_DEBUG) && defined(HAVE_CRTDBG_H)
//
// Declare global for memory state debugging, defined in Main.cpp
//
extern _CrtMemState memoryState;
#endif // _DEBUG

//===============================================================================================
// Application API
//===============================================================================================
void InitApplication (void);
void ExitApplication (void);

void InitSimulation (void);
void CleanupSimulation (void);
int  RedrawSimulation ();

void InitSplashScreen (void);
void RedrawSplashScreen (void);
void CleanupSplashScreen (void);

void InitExitScreen (void);
void RedrawExitScreen (void);

//
// Window Manager API
//
void InitWindowManager (int argc, char **argv);
void EnterWindowManagerMainLoop (void);
void CleanupWindowManager (void);

#endif  // FLYLEGACY_H
