/*
 * Gps150.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2002-2007 Chris Wallace
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

#ifndef HAVE_GPS150_H
#define HAVE_GPS150_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Unique identifier tag for Garmin Gps150 DLL objects
#define GPS150_ID    (unsigned int)'g150'

/*
 * GPS150 RealTime Data
 *
 * This data structure represents real-time data which is periodically updated
 *   in the main sim loop, then utilized in various ways by the active page
 */

typedef struct {
  SDateTime     tod;            ///< Time of day
  SPosition     userPos;        ///< User position
	float         var;            // Magnetic variation
	float         hdg;            // Magnetic heading
	float         trk;            // Ground track
	float         gs;             // Ground speed (kts)
	float         alt;            // Altitude (feet)
	
	bool          nrstApOk;       // Nearest airport info valid
	SAirport      nrstAp;         // Nearest airport
	float         nrstApBrgTrue;  // True bearing
	float         nrstApDistNm;   // Distance in nm

	bool          nrstVorOk;      // Nearest VOR info valid
	SNavaid       nrstVor;        // Nearest VOR
	float         nrstVorBrgTrue; // True bearing
	float         nrstVorDistNm;  // Distance in nm

	bool          nrstNdbOk;      // Nearest NDB info valid
	SNavaid       nrstNdb;        // Nearest NDB
	float         nrstNdbBrgTrue; // True bearing
	float         nrstNdbDistNm;  // Distance in nm
} SGps150RealTimeData;

/*
 * GPS150 Shared Data Structure
 *
 * The following type, enum and struct definitions (up to SGps150SystemData)
 *   represent the entire internal state of the GPS150 simulation engine.
 *   This data is maintained by the platform-independent engine, based on
 *   the state of the simulation world at the time of the last call to
 *   Gps150Update() and any trigger events which have occurred.
 *
 */

typedef enum {
	AnySurface,
	SoftHardSurface,
	HardSurface,
	WaterSurface
} ERwySurface;

typedef enum {
	PosDms,
	PosDm
} EPosUnits;

typedef enum {
	AltFeet,
	AltMetres
} EAltUnits;

typedef enum {
	VsFpm,
	VsMpm,
	VsMps
} EVsUnits;

typedef enum {
	NavNmKt,
	NavMiMph,
	NavKmKph
} ENavUnits;

typedef enum {
	FuelUSGal,
	FuelImpGal,
	FuelKg,
	FuelLbs,
	FuelLitres
} EFuelUnits;

typedef enum {
	FuelAvGas,
	FuelJetA,
	FuelJetB
} EFuelTypeUnits;

typedef enum {
	PressHg,
	PressMb
} EPressureUnits;

typedef enum {
	TempC,
	TempF
} ETempUnits;

//
// Supported types of stored waypoint types
//
typedef enum {
	StoredWptEmpty = 0,
	StoredWptAirport,
	StoredWptVor,
	StoredWptNdb,
	StoredWptFix,
	StoredWptUser
} EStoredWaypointType;

typedef enum {
	WptAirport,
	WptVor,
	WptNdb,
	WptFix,
	WptUser,
	WptFss
} EWaypointType;

//
// Structure representing a proximity waypoint
//
typedef struct {
	char                   name[8];
	float                  distance;
} SProximityWaypoint;


//
// Structure representing a confirmation waypoint.  This structure is used to transfer
//   between various modes and the WptConfirm page for D->TO operation
//
typedef struct {
	bool            confirmed;  // Whether waypoint confirmation was successful
	bool            usePos;     // Use position as an initial filter

	EWaypointType   type;
	bool            direct;
	char            id[8];
	SPosition       pos;
} SRouteWaypoint;

//
// Structure representing a user waypoint. Since these waypoints are
//   not in the Fly! database, the full position must be stored
//
typedef struct {
	char            name[8];
	SPosition       pos;
	int             rte;
} SUserWaypoint;


//
// Structure representing a waypoint comment
//
typedef struct {
	EStoredWaypointType    type;
	char                   id[8];
	char                   line1[22];
	char                   line2[22];
} SWptComment;

#define NUM_WPT_COMMENTS   250

//
// Structure representing a waypoint, either a stored route waypoint
// or a waypoint representation on the WPT screen.  The system data
// structure contains an array of 100 user waypoints of this type
//
typedef struct {
	EStoredWaypointType type;
	char                name[8];
	SPosition           pos;
} SRteWaypoint;

//
// Structure representing a route leg.  See SRteActive definition
//
typedef struct {
	float        dtk;           // Degrees true
	float        legDistance;   // NM
	float        cumDistance;   // NM
} SRteLeg;

//
// Each stored route can contain up to 32 waypoints each
//
#define RTE_COMMENT_LENGTH  25
#define NUM_RTE_WAYPOINTS   32
typedef struct {
	// Stored/retrieved information
	int           nWaypoints;
	SRteWaypoint  waypoint[NUM_RTE_WAYPOINTS];
	char          comment[RTE_COMMENT_LENGTH];

	// Derived information
	float         legDistance[NUM_RTE_WAYPOINTS];
	float         routeLength;
} SRteStored;


//
// The active route (route 0 in the database) contains additional data
//   beyond that in the stored routes
//
typedef struct{
	bool          valid;                     // Whether a route is active

	SRteStored    stored;                    // Stored route waypoints

	int           iLeg;                      // Index of active leg
	int           nLegs;                     // Total number of legs
	SRteLeg       leg[NUM_RTE_WAYPOINTS-1];  // Realtime data for legs

	// Search route parameters
	bool          search;
	float         searchInitial;
	float         searchOffset;

	// Parallel track parameters
	bool          parallelTrackLeft;
	float         parallelTrackDistance;
} SRteActive;


typedef enum {
	AltitudeInputFail,	
} EStatusMsg;


//
// Checklists.  GPS150 has 9 checklists of up to 31 items each (20 chars per item)
//
#define NUM_CHECKLISTS           9
#define NUM_CHECKLIST_ITEMS     31
#define CHECKLIST_ITEM_LENGTH   16

typedef struct {
	char      name[CHECKLIST_ITEM_LENGTH+1];
	int       nItems;
	bool      done[NUM_CHECKLIST_ITEMS];
	char      item[NUM_CHECKLIST_ITEMS][CHECKLIST_ITEM_LENGTH+1];
} SChecklist;


//
// Scheduled messages.  Up to 9 scheduled events can be programmed, each
//   event has a countdown timer and causes a system MSG to be displayed
//   when the timer has expired.
//
#define NUM_SCHEDULED_MSGS      9
#define SCHEDULED_MSG_LENGTH   12

typedef struct {
	bool      active;
	char      name[SCHEDULED_MSG_LENGTH+1];
	SDateTime time;
} SScheduledMsg;


//
// GPS150 System Data
//
typedef struct {
	// Common real-time user data
	float          pwrOnTime;

//	Fsm         fsm;
//	FsmStateKey keyOff, keyPowerUp, keyConfirm, keyWptConfirm, keyNrst;
//	FsmStateKey keySet, keyRte, keyWpt, keyNav, keyStat;

	// RealTime data
	float               realTimeTimer;
	SGps150RealTimeData *pRealTime;

	// Settings data
	float          cdiScale;
	float          arrivalAlarm;
	bool           autoLegSeq;
	bool           autoLegSelect;
	EPosUnits      posUnits;
	EAltUnits      altUnits;
	EVsUnits       vsUnits;
	ENavUnits      navUnits;
	EFuelUnits     fuelUnits;
	EFuelTypeUnits fuelTypeUnits;
	EPressureUnits pressureUnits;
	ETempUnits     tempUnits;
	bool           magVarAuto;
	int            magVar;
	int            minRwyLen;
	ERwySurface    rwySurface;
	int            battSaver;
	bool           tripTimerPwrOn;
	int            tripTimerSpeed;

	// UTC/Local time display
	bool          useUTC;
	double        lclOffset;
	SDateTime     utc;

	// Trip timer
	bool          departed;
  SDateTime     utcDepart;
	float         tripTimer;

	// Approach timer
	float         apprTimerPreset;
	float         apprTimer;
	bool          apprTimerCountDown;
	bool          apprTimerActive;

	// User waypoint database, 100 waypoints
	SUserWaypoint    userWpDatabase[100];
	int              nUserWaypoints;

	// Proximity waypoint database, 9 waypoints
	SProximityWaypoint proxWpDatabase[9];
	int                iProxWpt;
	int                nProximityWaypoints;

	// Waypoint comments database, 250 entries
	SWptComment        wptCommentDatabase[250];
	int                iWptComment;
	int                nWptComments;

	// Route database
	SRteStored   rteDatabase[19];
	SRteActive   rteActive;

	// Checklists, 9 checklists of up to 31 items each
	SChecklist   checklist[NUM_CHECKLISTS];
	int          nChecklists;

	// Scheduled messages, up to 9 items
	SScheduledMsg   scheduledMsg[NUM_SCHEDULED_MSGS];

	// Active waypoint data, used to transfer between modes
	SRouteWaypoint  wpActive;
//	FsmTrigger      replyTrigger;

	// Status message status
	bool            msgPending;
	EStatusMsg      msg;
} SGps150SystemData;


//
// Shared data structure for GPS150
//
// This data structure contains all information needed for an application
//   to visually display the GPS state.  Applications which retrieve this
//   data by calling Gps150GetSharedData() must realize that this is just
//   a snapshot of the GPS state at the time the function was called.
//   The application cannot change the GPS state by updating the contents
//   of the shared data structure; it must send appropriate triggers to the
//   Gps150Trigger() function in order to effect changes to the GPS internal
//   state.
//
// Members:
//   keyLock      Indicates whether keyboard focus lock is active
//   onOff        Indicates whether the GPS power is on or off
//   line1, 2, 3  Contents of the 3-line GPS text display
//   liteXXXX     Whether each particular indicator light is on/off
//

typedef struct {
	bool           keyLock;
	bool           onOff;
	char           line1[25], line2[25], line3[25];
	bool           liteNrst, liteSet, liteRte, liteWpt, liteNav, liteMsg;
} SGps150SharedData;

//
// GPS150 triggers.  Each of these triggers can be activated by an external event
//   in the application (i.e. mouse click, keypress, etc.)
//

#define G150_TRIGGER_SELF_TEST_EXPIRY   'stex'
#define G150_TRIGGER_SAT_ACQUISITION    'acqr'
#define G150_TRIGGER_CONFIRM_WPT        'cfwp'
#define G150_TRIGGER_WPT_CONFIRMED      'wpcf'
#define G150_TRIGGER_PWR                'pwr_'
#define G150_TRIGGER_D_TO               'd_to'
#define G150_TRIGGER_NRST               'nrst'
#define G150_TRIGGER_SET                'set_'
#define G150_TRIGGER_RTE                'rte_'
#define G150_TRIGGER_WPT                'wpt_'
#define G150_TRIGGER_NAV                'nav_'
#define G150_TRIGGER_CRSR               'crsr'
#define G150_TRIGGER_STAT               'stat'
#define G150_TRIGGER_CLR                'clr_'
#define G150_TRIGGER_ENT                'ent_'
#define G150_TRIGGER_OUTER_RIGHT        'outr'
#define G150_TRIGGER_OUTER_LEFT         'outl'
#define G150_TRIGGER_INNER_RIGHT        'innr'
#define G150_TRIGGER_INNER_LEFT         'innl'
#define G150_TRIGGER_KEYLOCK            'kLok'
#define G150_TRIGGER_KEY_A              'keyA'
#define G150_TRIGGER_KEY_B              'keyB'
#define G150_TRIGGER_KEY_C              'keyC'
#define G150_TRIGGER_KEY_D              'keyD'
#define G150_TRIGGER_KEY_E              'keyE'
#define G150_TRIGGER_KEY_F              'keyF'
#define G150_TRIGGER_KEY_G              'keyG'
#define G150_TRIGGER_KEY_H              'keyH'
#define G150_TRIGGER_KEY_I              'keyI'
#define G150_TRIGGER_KEY_J              'keyJ'
#define G150_TRIGGER_KEY_K              'keyK'
#define G150_TRIGGER_KEY_L              'keyL'
#define G150_TRIGGER_KEY_M              'keyM'
#define G150_TRIGGER_KEY_N              'keyN'
#define G150_TRIGGER_KEY_O              'keyO'
#define G150_TRIGGER_KEY_P              'keyP'
#define G150_TRIGGER_KEY_Q              'keyQ'
#define G150_TRIGGER_KEY_R              'keyR'
#define G150_TRIGGER_KEY_S              'keyS'
#define G150_TRIGGER_KEY_T              'keyT'
#define G150_TRIGGER_KEY_U              'keyU'
#define G150_TRIGGER_KEY_V              'keyV'
#define G150_TRIGGER_KEY_W              'keyW'
#define G150_TRIGGER_KEY_X              'keyX'
#define G150_TRIGGER_KEY_Y              'keyY'
#define G150_TRIGGER_KEY_Z              'keyZ'
#define G150_TRIGGER_KEY_1              'key1'
#define G150_TRIGGER_KEY_2              'key2'
#define G150_TRIGGER_KEY_3              'key3'
#define G150_TRIGGER_KEY_4              'key4'
#define G150_TRIGGER_KEY_5              'key5'
#define G150_TRIGGER_KEY_6              'key6'
#define G150_TRIGGER_KEY_7              'key7'
#define G150_TRIGGER_KEY_8              'key8'
#define G150_TRIGGER_KEY_9              'key9'
#define G150_TRIGGER_KEY_0              'key0'
#define G150_TRIGGER_KEY_SLASH          'key/'
#define G150_TRIGGER_KEY_SPACE          'key_'

/*
 * Define specific FsmState subclasses for GPS150 display states
 */

//
// Power-Off state
//
/*
class PowerOff : public FsmState
{
public:
	// Constructors
	PowerOff (void *p) : FsmState (p) {}

	// Member functions
	void Activate (void);
	void Deactivate (void);
};


//
// Power-Up (self-test) pages
//

class PowerUp : public FsmState
{
public:
	// Constructors
	PowerUp (void *p) : FsmState (p) {}

	// Member functions
	void Activate (void);
	void Deactivate (void);
	void TimeSlice (float dT);

	void UpdateDisplay (void);
	void UpdateDisplay1 (void);
	void UpdateDisplay2 (void);
	void UpdateDisplay3 (void);

	// Member variables
	bool  crsrFlash;
	float crsrElapsed;
	int   page;
	float elapsed;

	static const float SelfTestInterval_m;
};


//
// Database confirmation page
//

class Confirm : public FsmState
{
public:
	// Constructors
	Confirm (void *p) : FsmState (p), crsrFlash(true) {}

	// Member functions
	void Activate (void);
	void Deactivate (void);
	void TimeSlice (float dT);

	// Member variables
	float crsrElapsed;
	bool  crsrFlash;

private:
	void updateDisplay (void);
};


//
// WPT Confirm page
//

class WptConfirm : public FsmState
{
public:
	// Constructors
	WptConfirm (void *p) : FsmState (p) {}

	// Member functions
	void Activate (void);
	void Deactivate (void);
	bool HandleTrigger (FsmTrigger);
	void TimeSlice (float dT);

	// Private member functions
private:
	void UpdateWptConfirmDisplay (void);
	void UpdateWptConfirmAirportDisplay (void);
	void UpdateWptConfirmVorDisplay (void);
	void UpdateWptConfirmNdbDisplay (void);

	// Private member variables
private:
	float   crsrElapsed;
	bool    crsrFlash;

	// There may be many possible values to be confirmed, the following variables allow
	//   the linked-list capabilities of the Fly! database search routines to be used
	int          nOptions;
	SFsimAirport *pApList, *pAp;
	SFsimNavaid  *pNavaidList, *pNavaid;

	// Common display values
	char sId[8];
	char sAlt[10];
	char SFsimPosition[24];

	// Airport display values
	char sFuel[8];
};


//
// NRST Pages
//

typedef enum {
	NrstAirport,
	NrstVor,
	NrstNdb,
	NrstFix,
	NrstFss
} ENrstPage;

//typedef struct {
//	char  id[8];
//	float length;
//} SNrstRunway;


typedef struct {
	float			distance;         // In nm
	SFsimAirport	airport;
	int				iRunway;          // Index to currently displayed runway
	int				iComm;            // Index to currently display comm facility
} SNrstAirport;

typedef struct {
	float           distance;			// In nm
	SFsimNavaid     navaid;
} SNrstNavaid;

typedef struct {
	float				distance;		// In nm
	SFsimIntersection	intersection;
} SNrstIntersection;

typedef struct {
	float			distance;			// In nm
	SFsimComm		comm;
} SNrstComm;

class Nearest : public FsmState
{
public:
	// Constructors
	Nearest (void *p) : FsmState (p), wpConfirmPending (false) {}

	// Member functions
	void Activate (void);
	void Deactivate (void);
	bool HandleTrigger (FsmTrigger t);
	void TimeSlice (float dT);

	// Member variables

	// Summarized lists of airports, VOR, NDB, Fixes and FSS
	int nAirports;
	int iAirport;
	SNrstAirport  airportList[9];
	int           airportSort[9];

	int nVor;
	int iVor;
	SNrstNavaid   vorList[9];
	int           vorSort[9];

	int nNdb;
	int iNdb;
	SNrstNavaid   ndbList[9];
	int           ndbSort[9];

	int nFix;
	int iFix;
	SNrstIntersection   fixList[9];
	int                 fixSort[9];

	int nFss;
	int iFss;
	SNrstComm     fssList[5];

	ENrstPage   page;

	// Waypoint confirmation pending, used when D->TO is pressed from the WPT page
	bool  wpConfirmPending;

	bool  crsr;
	float crsrElapsed;
	bool  crsrFlash;
	int   crsrMode;

private:
	void UpdateNrstDisplay (void);
	void UpdateNrstAirportDisplay (void);
	void UpdateNrstVorDisplay (void);
	void UpdateNrstNdbDisplay (void);
	void UpdateNrstFixDisplay (void);
	void UpdateNrstFssDisplay (void);
	void UpdateAirportList (void);
	void UpdateNavaidList (void);
	void UpdateFixList (void);
	void UpdateFssList (void);
};


//
// SET pages
//

typedef enum {
	CDIArrival,
	RouteSettings,
	MagVariation,
	NrstApSearch,
	BatterySaver,
	TripTimer,
	MeasurementUnits,
	SUASettings,
	MapDatum,
	IOChannels,
	ARINC429,
	DataTransfer
} ESetPage;

typedef enum {
	CDIScale,
	ArrivalAlarm,
	AutoLegSeq,
	AutoLegSelect,
	MagneticVar,
	MagVarMode,
	MagVarSet1,
	MagVarSet2,
	MagVarSet3,
	MagVarSet4,
	MinRwyLengthSet1,
	MinRwyLengthSet2,
	MinRwyLengthSet3,
	MinRwyLengthSet4,
	MinRwySetSurface,
	BattSaverInterval,
	TripTimerMode,
	TripTimerSetSpeed1,
	TripTimerSetSpeed2,
	TripTimerSetSpeed3,
	UnitsPosition,
	UnitsAlt,
	UnitsVs,
	UnitsNav,
	UnitsFuel,
	UnitsFuelType,
	UnitsPressure,
	UnitsTemperature
} ESetCrsrMode;


class Set : public FsmState
{
public:
	// Constructors
	Set (void *p) : FsmState (p), page(CDIArrival) {}

	// Member functions
	void Activate (void);
	void Deactivate (void);
	bool HandleTrigger (FsmTrigger);
	void TimeSlice (float dT);

	// Member variables
	bool          crsr;
	bool          crsrFlash;
	float         crsrElapsed;
	ESetPage      page;
	ESetCrsrMode  crsrMode;

private:
	void updateDisplay (void);
	void updateCDIArrivalDisplay (void);
	void updateRouteSettingsDisplay (void);
	void updateMagVariationDisplay (void);
	void updateNrstApSearchDisplay (void);
	void updateBatterySaverDisplay (void);
	void updateTripTimerDisplay (void);
	void updateMeasurementUnitsDisplay (void);
	void updateSUASettingsDisplay (void);
	void updateMapDatumDisplay (void);
	void updateIOChannelsDisplay (void);
	void updateARINC429Display (void);
	void updateDataTransferDisplay (void);
};


//
// RTE (Stored routes) pages
//

typedef enum {
	RteActive,
	RteCatalog,
	RteCopy,
	RteSearch,
	RteCpa
} ERtePage;

typedef enum {
	RteDistCum,
	RteDistLeg
} ERteDistMode;

typedef enum {
	RteTimeEte,
	RteTimeEta,
	RteTimeDtk
} ERteTimeMode;

class Route : public FsmState
{
public:
	// Constructors
	Route (void *p);

	// Member functions
	void Activate (void);
	void Deactivate (void);
	bool HandleTrigger (FsmTrigger);
	void TimeSlice (float dT);

	void UpdateRteActiveDisplay (void);
	void UpdateRteCatalogDisplay (void);
	void UpdateRteDisplay (void);
	void UpdateRteCopyDisplay (void);
	void UpdateRteSearchDisplay (void);
	void UpdateRteCpaDisplay (void);

	void UpdateRteFromFlightPlan (void);
	void CopyRte (int rte);
	void ReverseRte (int rte);
	void FormatRteWpt (int wpt, char *buffer);

	void Route::HandleKey (char c);

	// Temporary text entry data
	char          textValue[80];
	int           iText;          // Index of current char being edited
	int           iTextBound;     // Upper char bound for this field

	// Member variables
	bool      crsr;
	float     crsrElapsed;
	bool      crsrFlash;
	int       crsrMode;
	int       iRte;
	ERtePage  page;

	// Variables for Catalog Copy page
	int       copyFromRte, copyToRte;

	// Variables for RTE Active page
	int          iLeg;
	int          wptActiveFrom, wptActiveTo;
	ERteDistMode distMode;
	ERteTimeMode timeMode;

	// Variables for CPA page
	bool           cpaWptValid;
	SRouteWaypoint cpaWpt;
	SFsimPosition  cpaPos;
	int            cpaFrom;
	float          cpaBrg, cpaDis;
};


//
// WPT (Database) pages
//

typedef enum {
	WptMenu,
	WptProximity,
	WptUserCatalog,
	WptComments,

	WptAptIdent,
	WptAptPosition,
	WptAptComm,
	WptAptRwy,
	WptAptComments,

	WptVorIdent,
	WptVorPosition,
	WptVorComments,

	WptNdbIdent,
	WptNdbPosition,
	WptNdbComments,

	WptIntersectionIdent,

	WptUserIdent,
	WptUserNew,
	WptUserEntry,
} EWptPage;

typedef enum {
	WptRwyApprIls,
	WptRwyApprLoc
} EWptRwyApprType;

typedef struct {
	EWptRwyApprType  type;
	char             rwyId[8];
	float            freq;
} SWptRwyApproach;

typedef struct {
	char            id[12];
	float           length;
	ERwySurface     surface;
	int             nApproach;
	SWptRwyApproach approach[4];
} SWptRunway;

typedef struct {
	char            name[20];
	EFsimCommType   type;
	SFsimPosition   pos;
	float           freq;
} SWptComm;

typedef struct {
	bool					valid;
	char					id[8];
	char					name[40];
	char					city[40];
	SFsimPosition			pos;
	EFsimAirportFuelTypes	fuel;
	EFsimAirportUsage		usage;
	EFsimAirportAirspace	airspace;
	bool					radar;
	int						nRunways;
	int						iRunway;
	SWptRunway				rwy[16];
	int						nComms;
	int						iComm;
	SWptComm				comm[16];
	SWptComment				comment;
} SWptAirport;

typedef struct {
	bool             valid;
	char             id[8];
	char             name[40];
	char             city[40];
	float            freq;
	SFsimPosition    pos;
	SWptComment      comment;
} SWptVor;

typedef struct {
	bool             valid;
	char             id[8];
	char             name[40];
	char             city[40];
	float            freq;
	SFsimPosition        pos;
	SWptComment      comment;
} SWptNdb;

typedef struct {
	bool             valid;
	char             id[8];
	SFsimPosition        pos;
	char             vorId[8];
	float            vorBearing;
	float            vorDistance;
} SWptFix;

typedef struct {
	bool             valid;
	char             id[8];
	SFsimPosition        pos;
} SWptUser;
 

class Waypoint : public FsmState
{
public:
	// Constructors
	Waypoint (void *p);

	// Member functions
	void Activate (void);
	void Deactivate (void);
	bool HandleTrigger (FsmTrigger);
	void TimeSlice (float dT);

	// Member variables
	EWptPage      page;
	EWaypointType type;
	bool          crsr;
	float         crsrElapsed;
	bool          crsrFlash;
	int           crsrMode;
	float         triggerElapsed;

	// Key data for currently selected object
	char sAirportKey[8];
	char sFixKey[8];
	char sNdbKey[8];
	char sVorKey[8];
	char sUserKey[8];

	// Currently selected objects
	SWptAirport   wptAirport;
	SWptFix       wptFix;
	SWptNdb       wptNdb;
	SWptVor       wptVor;
	SWptUser      wptUser;

	// Temporary text entry data
	char          textValue[80];
	int           iText;          // Index of current char being edited
	int           iTextBound;     // Upper char bound for this field

	// Waypoint ID entry variables
//	char          sIdEntry[8];
//	char          sCountryEntry[8];
//	char          sCityEntry[24];
//	char          sStateEntry[8];
//	char          sNameEntry[24];
//	int           iId, iName;

	// Airport display variables
	int  iAptComm;
	int  iAptRwy;
	int  iAptRwyApproach;

	// User waypoint display variables
	int  iUserWpt;

	// Waypoint comment database display variables
	int  iWptCommentBase;

	// Flag for pending D->TO waypoint confirmation
	bool wpConfirmPending;

private:
	//
	// Copy functions populate the SWpt... structs from source Fly! data
	//
	void CopyWptAirport (SFsimAirport *pAirport, SWptAirport *pWpt);
	void CopyWptNdb (SFsimNavaid *pAirport, SWptNdb *pWpt);
	void CopyWptVor (SFsimNavaid *pAirport, SWptVor *pWpt);
	void CopyWptFix (SFsimWaypoint *pAirport, SWptFix *pWpt);
	void CopyWptUser (SUserWaypoint  *pUser, SWptUser *pWpt);

	//
	// Search functions 
	//

	void SearchAirportId (char *, bool force);
	void SearchAirportName (char *);
	void UpdateWptDisplay (void);
	void SearchNdb (char *);
	void SearchVor (char *);
	void SearchFix (char *);
	void SearchUser (char* id);

	//
	// Miscellaneous helper functions
	//
	void IncAptRwy ();
	void DecAptRwy ();
	void RescanEntry (void);
	void HandleKey (char c);
	void FormatProximityWaypoint (int iWpt, char *buffer);
//	void RefreshAirport (SGeneric *);

	//
	// Display update functions
	//
	void UpdateWptMenuDisplay (void);
	void UpdateWptProximityDisplay (void);
	void UpdateWptUserCatalogDisplay (void);
	void UpdateWptCommentDisplay (void);
	void UpdateAptId (char *);
	void UpdateAptName (char *);
	void UpdateAptCommentLine1 (char *);
	void UpdateAptCommentLine2 (char *);
	void UpdateWptAptIdentDisplay (void);
	void UpdateWptAptPositionDisplay (void);
	void UpdateWptAptCommDisplay (void);
	void UpdateWptAptRwyDisplay (void);
	void UpdateWptAptCommentsDisplay (void);
	void UpdateVorId (char *);
	void UpdateVorCommentLine1 (char *);
	void UpdateVorCommentLine2 (char *);
	void UpdateWptVorIdentDisplay (void);
	void UpdateWptVorPositionDisplay (void);
	void UpdateWptVorCommentsDisplay (void);
	void UpdateNdbId (char *);
	void UpdateNdbCommentLine1 (char *);
	void UpdateNdbCommentLine2 (char *);
	void UpdateWptNdbIdentDisplay (void);
	void UpdateWptNdbPositionDisplay (void);
	void UpdateWptNdbCommentsDisplay (void);
	void UpdateIntersectionId (char *s);
	void UpdateWptIntersectionIdentDisplay (void);
	void UpdateUserWptId (char *);
	void UpdateWptUserIdentDisplay (void);
	void UpdateWptUserNewDisplay (void);
};


//
// NAV page
//

typedef enum {
	NavCdi,
	NavPosition,
	NavMenu1,
	NavMenu2,
	NavAutoStore,
	NavTripPlan,
	NavDaltTas,
	NavFuelPlan,
	NavWinds,
	NavVnavPlan,
	NavChklistMenu,
	NavApprTime,
	NavClock,
	NavTripTime,
	NavGpsPlan,
	NavScheduler,
	NavSunrise,
	NavChklistDetail,
} ENavPage;

typedef enum {
	NAV_DIS_DISTANCE,
	NAV_DIS_STEER
} ENavDistanceMode;

typedef enum {
	NAV_TRK_TRACK,
	NAV_TRK_BEARING,
	NAV_TRK_COURSE,
	NAV_TRK_DESIRED_TRACK,
	NAV_TRK_OBS,
	NAV_TRK_TURN
} ENavTrackMode;

typedef enum {
	NAV_ETE_ETA,
	NAV_ETE_ETE,
	NAV_ETE_TRK,
	NAV_ETE_VN
} ENavEteMode;

typedef enum {
	NAV_PROX_APT,
	NAV_PROX_VOR,
	NAV_PROX_NDB,
	NAV_PROX_INT,
	NAV_PROX_WPT,
	NAV_PROX_USR
} ENavProximity;

class Nav : public FsmState
{
public:
	// Constructors
	Nav (void *p);

	// Member functions
	void Activate (void);
	void Deactivate (void);
	bool HandleTrigger (FsmTrigger);
	void TimeSlice (float dT);

private:
	void updateNavState (void);
	void updateNavCdiDisplay (void);
	void updateNavPositionDisplay (void);
	void updateNavMenu1Display (void);
	void updateNavMenu2Display (void);
	void updateNavAutoStoreDisplay (void);
	void updateNavTripPlanDisplay (void);
	void updateNavDaltTasDisplay (void);
	void updateNavFuelPlanDisplay (void);
	void updateNavWindsDisplay (void);
	void updateNavVnavPlanDisplay (void);
	void updateNavChklistMenuDisplay (void);
	void updateNavApprTimeDisplay (void);
	void updateNavClockDisplay (void);
	void updateNavTripTimeDisplay (void);
	void updateNavGpsPlanDisplay (void);
	void updateNavSchedulerDisplay (void);
	void updateNavSunriseDisplay (void);
	void updateNavChklistDetailDisplay (void);
	void updateNavDisplay (void);

	// Private member variables
private:
	ENavPage  page;

	// Current aircraft state variables
	float     elapsed;

	// CRSR variables
	bool      crsr;
	int       crsrMode;
	float     crsrElapsed;
	bool      crsrFlash;

	// Temporary text entry data
	char          textValue[80];
	int           iText;          // Index of current char being edited
	int           iTextBound;     // Upper char bound for this field

	// NAV proximity page variables
	ENavProximity      proximityMode;

	// CDI page variables
	ENavDistanceMode   disMode;
	ENavTrackMode      trkMode;
	ENavEteMode        eteMode;

	// User waypoint auto-save parameters
	char               idAutoStore[8];
	SFsimPosition          posAutoStore;
	int                rteAutoStore;

	// Checklist menu
	int iChecklist, iChecklistSelect;
	int iChecklistItem, iChecklistItemSelect;

	// Scheduler menu
	int             iScheduler, iSchedulerSelect;

	// Approach timer page
	int    apprTimeHour, apprTimeMin, apprTimeSec;

	// Sunrise/sunset page
	bool            wpSunrisePending;
	SRouteWaypoint  wptSunrise;
	SFsimDate       dateSunrise;
};


//
// STAT (Status/Message) page
//

typedef enum {
	SearchSky,
	Acquiring,
	Nav2D,
	Nav3D,
	Simulator,
	PoorCvrge,
	NeedAlt,
	NotUsable
} EReceiverStatus;

typedef enum {
	StatusReceiver,
	StatusMsg
} EStatusPage;

typedef enum {
	PrecisionDop,
	PrecisionEpe
} EPrecisionMode;


class Status : public FsmState
{
public:
	// Constructors
	Status (void *p) :
	  FsmState (p), page(StatusReceiver), rcvrStatus(SearchSky),
	  precisionMode (PrecisionDop), nSatellites(0)
	  {}

	// Member variables
	void Activate (void);
	void Deactivate (void);
	bool HandleTrigger (FsmTrigger);
	void TimeSlice (float dT);

private:
	void UpdateStatusReceiverDisplay (void);
	void UpdateStatusMsgDisplay (void);
	void UpdateStatusDisplay (void);

	// Private member variables
private:
	EStatusPage     page;
	EReceiverStatus rcvrStatus;
	EPrecisionMode  precisionMode;
	float           precision;
	int             nSatellites;
	int             satAcquired[8];
	int             satSignal[8];
};

//
// Special font characters, defined in Utilities.cpp
//
extern char CharFeet;
extern char CharMetres;
extern char CharNm;
extern char CharMi;
extern char CharKm;
extern char CharKts;
extern char CharMph;
extern char CharKph;

extern char CharUSGal;
extern char CharImpGal;
extern char CharKg;
extern char CharLbs;
extern char CharLitres;

extern char CharHg;
extern char CharMb;
extern char CharDegC;
extern char CharDegF;

extern char CharFrom;

extern char CharSmall0;
extern char CharSmallDot0;
extern char CharSmallQuote;

//
// Utility functions implemented in Utilities.cpp
//
void formatPosition (SFsimPosition pos, EPosUnits units, char *fmt);
void formatAltitude (float altFeet, EAltUnits units, char *fmt);
void formatVerticalSpeed (float vsFpm, EVsUnits units, char *fmt);
void formatEmptyDistance (ENavUnits units, char *fmt);
void formatDistance (float distanceNm, ENavUnits units, char *fmt);
void formatDistance (float distanceNm, ENavUnits units, char *fmt);
void formatSpeed (float speedKts, ENavUnits units, char *fmt);
void formatSpeedInt (int speedKts, ENavUnits units, char *fmt);
void formatFrequency (float freq, char *fmt);
void formatAirportId (const SFsimAirport *ap, char *fmt);
void formatWaypointName (char* name, char* fmt);
void formatCommType (int type, char *fmt);
void formatTemperature (float temp, ETempUnits units, char *fmt);
/// Format integer month 0-11 into month name abbreviation, e.g. "Jan"
void formatMonth (int month, char *fmt);
/// Format date in form dd-Mon-yy
void formatDate (SFsimDate date, char *fmt);
void formatDop (float dop, char *fmt);
void formatEpe (float epe, char *fmt);
void formatSatellite (int sat, char *fmt);
void formatSignal (int sig, char *fmt);
void formatRwySurface (ERwySurface &rwy, char *fmt);
void formatBearing (float brg, char *s);
void formatCdi (float cdi, char *s);
void toggleCdiScale (float &cdiScale);
void incRwySurface (ERwySurface &rwy);
void decRwySurface (ERwySurface &rwy);
void formatRwySurface (ERwySurface &rwy, char *fmt);
void formatTemperature (float temp, ETempUnits units, char *fmt);
void incPosUnits (EPosUnits &units);
void decPosUnits (EPosUnits &units);
void incAltUnits (EAltUnits &units);
void decAltUnits (EAltUnits &units);
void incVsUnits (EVsUnits &units);
void decVsUnits (EVsUnits &units);
void incNavUnits (ENavUnits &units);
void decNavUnits (ENavUnits &units);
void incFuelUnits (EFuelUnits &units);
void decFuelUnits (EFuelUnits &units);
void incFuelTypeUnits (EFuelTypeUnits &units);
void decFuelTypeUnits (EFuelTypeUnits &units);
void incPressureUnits (EPressureUnits &units);
void decPressureUnits (EPressureUnits &units);
void incTempUnits (ETempUnits &units);
void decTempUnits (ETempUnits &units);

//
// Format Degree of Precision value on STAT page
//
void formatDop (float dop, char *fmt);

//
// Format Estimated Position Error value on STAT page
//
void formatEpe (float feet, char *fmt);

//
// Format Satellite number
//
void formatSatellite (int sat, char *fmt);

//
// Format Satellite signal strength
//
void formatSignal (int sig, char *fmt);
void formatTime (float time, char *s);
void incEntry (char &c);
void decEntry (char &c);
void SaveGPSConfiguration (SGps150SystemData *pData);
void LoadGPSConfiguration (SGps150SystemData *pData);

//
// Stored route (flight plan) utility functions
//
void clearRteStored (SRteStored &rte);
void copyRteStored (SGps150SystemData *pData, int from, int to);
void insertRteStored (SRteStored *pStored, SRteWaypoint *pWpt, int after);

void clearRteActive (SRteActive &rte);
void activateRteStored (SRteActive *pActive, SFsimRoute *pStored, SFsimPosition startPos, bool autoLeg);
void reverseRteActive (SGps150SystemData *pData);
void directToRte (SFsimPosition &from, SRouteWaypoint &to, SRteActive *pActive, bool autoLeg);
void formatRteEndpoints (SGps150SystemData *pData, int iRte, char *s1, char *s2);

//
// Waypoint Comments database
//
void updateWptComment (SGps150SystemData *pData, SWptComment comment);
int searchWptComment (SGps150SystemData *pData, EStoredWaypointType type, char *id);

//
//
//
void calcCpaRoute (SRteStored *pRte, SFsimPosition x, int *iLeg, SFsimPosition *pos);
bool searchWaypointId (SGps150SystemData *pData, const char *id, SRouteWaypoint *pWp);
void calcBearingDistance (SFsimPosition from, SFsimPosition to, float *brg, float *dis);

//
// Real-Time Data
//
void updateRealTimeData (SGps150SystemData *pData);

*/

#endif // HAVE_GPS150_H