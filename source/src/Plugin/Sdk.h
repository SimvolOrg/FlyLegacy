/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//
//	SDK.H
//
//
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

#ifndef __MAINSDK__
#define __MAINSDK__

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//	
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////


/// \todo all the enum and struct from flyLegacy should be
//        added here below and provided in the sdk.h for the
//        dll plugin compilation.
//
//  !!! Some data is not in FlyLegacy  // lc 10/8/2006
/*

        SPythonMethod
        SEpicEvent
        SDataBlockItem
        SFile
        SScriptInfo

*/

#include "..\Include\FlyLegacy.h"
//
extern "C" {

	//
	//	all functions defined with a straight C interface for
	//	easy implementation in a variety of languages
	//	(C, C++, Pascal, assembly, etc.)
	//

	/////////////////////////////////////////////////////////////
	//
	//	Constants/enums
	//
	/////////////////////////////////////////////////////////////
/*
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
		MSG_IGNORED = 0,	// message was passed over
		MSG_PROCESSED = 1,	// message was processed (set/get) and should stop broadcast
		MSG_USED = 2		// message was used indirectly, but should continue to other objects
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
		HW_UNKNOWN	= 0,
		HW_BUS		= 1,
		HW_SWITCH	= 2,
		HW_LIGHT	= 3,
		HW_FUSE		= 4,
		HW_STATE	= 5,
		HW_GAUGE	= 6,
		HW_OTHER	= 7,
		HW_CIRCUIT	= 8,
		HW_RADIO	= 9,
		HW_FLAP		= 10,
		HW_HILIFT	= 11,
		HW_GEAR		= 12,
		HW_BATTERY	= 13,
		HW_ALTERNATOR	= 14,
		HW_ANNUNCIATOR	= 15,
		HW_GENERATOR	= 16,
		HW_CONTACTOR	= 17,
		HW_SOUNDFX	= 18,
		HW_FLASHER	= 19,
		HW_INVERTER	= 20,
		HW_UNITLESS	= 101,
		HW_UNBENT	= 102,
		HW_SCALE	= 103
	};

	enum EDLLObjectType
	{
		TYPE_DLL_GAUGE		= 'gage',
		TYPE_DLL_SYSTEM		= 'subs',
		TYPE_DLL_WORLDOBJECT	= 'wobj',
		TYPE_DLL_WINDOW		= 'wind',
		TYPE_DLL_CAMERA		= 'came',
		TYPE_DLL_CONTROL	= 'cont',
		TYPE_DLL_VIEW		= 'view'
	};

	enum EFlyObjectType
	{
		TYPE_FLY_UNKNOWN		= 0,
		TYPE_FLY_WORLDOBJECT		= 'wobj',
		TYPE_FLY_MODELOBJECT		= 'mobj',
		TYPE_FLY_SIMULATEDOBJECT	= 'sobj',
		TYPE_FLY_VEHICLE		= 'vehi',
		TYPE_FLY_GROUNDVEHICLE		= 'gveh',
		TYPE_FLY_AIRPLANE		= 'plan',
		TYPE_FLY_HELICOPTER		= 'heli'
	};

	enum EBitmapType
	{
		TYPE_SINGLE	= 'pbm ',
		TYPE_FRAMES	= 'pbg ',
		TYPE_INVALID	= '    '
	};

	enum EStreamTagResult 
	{
		TAG_IGNORED = 0,
		TAG_READ = 1
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
		CAMERA_INVALID = 0,
		CAMERA_COCKPIT = 'cock',
		CAMERA_SPOT = 'spot',
		CAMERA_FLYBY = 'flyb',
		CAMERA_TOWER = 'towr'
	};

	enum ENavaidTypes
	{
		NAVAID_TYPE_UNKNOWN		= 0,
		NAVAID_TYPE_VOR			= (1 << 0),
		NAVAID_TYPE_TACAN		= (1 << 2),
		NAVAID_TYPE_NDB			= (1 << 3),
		NAVAID_TYPE_DME			= (1 << 4),
		NAVAID_TYPE_LOCATOR		= (1 << 5),
		NAVAID_TYPE_LOCALIZER		= (1 << 6),
		NAVAID_TYPE_GS			= (1 << 7),
		NAVAID_TYPE_BACKCOURSE		= (1 << 8),
		NAVAID_TYPE_INNERMARKER		= (1 << 9),
		NAVAID_TYPE_MIDDLEMARKER	= (1 << 10),
		NAVAID_TYPE_OUTERMARKER		= (1 << 11),
		NAVAID_TYPE_WAYPOINT		= (1 << 12),
		NAVAID_TYPE_VOT			= (1 << 13),
		NAVAID_TYPE_VORTAC		= (NAVAID_TYPE_VOR | NAVAID_TYPE_TACAN),
		NAVAID_TYPE_VORDME		= (NAVAID_TYPE_VOR | NAVAID_TYPE_DME),
		NAVAID_TYPE_NDBDME		= (NAVAID_TYPE_NDB | NAVAID_TYPE_DME),
		NAVAID_TYPE_ILSDME		= (NAVAID_TYPE_LOCALIZER | NAVAID_TYPE_DME)
	};

	enum ENavaidClasses
	{
		NAVAID_CLASS_UNKNOWN		= 0,
		NAVAID_CLASS_HIGH		= 1,
		NAVAID_CLASS_LOW		= 2,
		NAVAID_CLASS_TERMINAL		= 3,
		NAVAID_CLASS_MEDIUMHOMING	= 4,
		NAVAID_CLASS_HIGHHOMING		= 5,
		NAVAID_CLASS_ANTICIPATED	= 6
	};

	enum ENavaidUsage 
	{
		NAVAID_USAGE_UNKNOWN	= 0,
		NAVAID_USAGE_HIGH	= 1,
		NAVAID_USAGE_LOW	= 2,
		NAVAID_USAGE_BOTH	= 3,
		NAVAID_USAGE_TERMINAL	= 4,
		NAVAID_USAGE_RNAV	= 5
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
		COMM_EMERGENCY = (1 << 10)
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
		MOUSE_BUTTON_LEFT	= (1 << 0),
		MOUSE_BUTTON_RIGHT	= (1 << 1),
		MOUSE_BUTTON_MIDDLE	= (1 << 2)
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
		WINDOW_HAS_TITLEBAR	= (1 << 0),
		WINDOW_HAS_CLOSEBOX	= (1 << 1),
		WINDOW_HAS_SIZEBOX	= (1 << 2),
		WINDOW_HAS_BORDER	= (1 << 3),
		WINDOW_IS_MOVEABLE	= (1 << 4),
		WINDOW_IS_RESIZEABLE	= (1 << 5)
	};

	enum EWindowType
	{
		WINDOW_CAMERA		= 1,
		WINDOW_RASTER_MAP	= 2,
		WINDOW_VECTOR_MAP	= 3,
		WINDOW_CHECKLIST	= 4,
		WINDOW_GPS		= 5,
		WINDOW_MINIPANEL	= 6,
		WINDOW_AXIS		= 7
	}; 

	enum EFPEntryType
	{
		ENTRY_AIRPORT	= 1,
		ENTRY_NAVAID	= 2,
		ENTRY_WAYPOINT	= 3,
		ENTRY_USER	= 4
	};

	enum ETimeOfDay
	{
		TIME_DAWN	= 1,
		TIME_DAYTIME	= 2,
		TIME_DUSK	= 3,
		TIME_NIGHTTIME	= 4
	};

	enum EJoystickAxis
	{
		X_AXIS_INDEX		= 0,
		Y_AXIS_INDEX		= 1,
		RUDDER_AXIS_INDEX	= 2,
		TRIM_AXIS_INDEX		= 3,
		RBRAKE_AXIS_INDEX	= 4,
		LBRAKE_AXIS_INDEX	= 5,
		THROTTLE_AXIS_INDEX	= 6,
		THROTTLE1_AXIS_INDEX	= THROTTLE_AXIS_INDEX,
		THROTTLE2_AXIS_INDEX	= 7,
		THROTTLE3_AXIS_INDEX	= 8,
		THROTTLE4_AXIS_INDEX	= 9,
		MIXTURE_AXIS_INDEX	= 10,
		MIXTURE1_AXIS_INDEX	= MIXTURE_AXIS_INDEX,
		MIXTURE2_AXIS_INDEX	= 11,
		MIXTURE3_AXIS_INDEX	= 12,
		MIXTURE4_AXIS_INDEX	= 13,
		PROP_AXIS_INDEX		= 14,
		PROP1_AXIS_INDEX	= PROP_AXIS_INDEX,
		PROP2_AXIS_INDEX	= 15,
		PROP3_AXIS_INDEX	= 16,
		PROP4_AXIS_INDEX	= 17
	};

	enum ERadioType
	{
		RADIO_COM	=	(1 << 0),
		RADIO_NAV	=	(1 << 1),
		RADIO_DME	=	(1 << 2),
		RADIO_XPDR	=	(1 << 3),
		RADIO_AP	=	(1 << 4),
		RADIO_GPS	=	(1 << 5),
		RADIO_ADF	=	(1 << 6),
		RADIO_AUDIO	=	(1 << 7),
		RADIO_HF	=	(1 << 8)
	};

	enum EFreqType
	{
		FREQ_ACTIVE	=	0,
		FREQ_STANDBY	=	1
	};

	enum 
	{	RAY_COLLIDE_GROUND	= 0x01, 
		RAY_COLLIDE_MODELS	= 0x02, 
		RAY_COLLIDE_CLOUDS	= 0x04,
		RAY_COLLIDE_ALL		= 0xFF, 
		RAY_COLLIDE_SOLID_OBJECTS = (RAY_COLLIDE_ALL & ~RAY_COLLIDE_CLOUDS)
	};

	enum EFileSearchLocation
    {
		SEARCH_PODS	=	(1 << 0),
		SEARCH_DISK	=	(1 << 1),
		SEARCH_ALL	=	SEARCH_PODS | SEARCH_DISK
	};

	enum EKeyboardKeys
	{
		KB_KEY_ESC			= 1,
		KB_KEY_1			= 2,
		KB_KEY_2			= 3,
		KB_KEY_3			= 4,
		KB_KEY_4			= 5,
		KB_KEY_5			= 6,
		KB_KEY_6			= 7,
		KB_KEY_7			= 8,
		KB_KEY_8			= 9,
		KB_KEY_9			= 10,
		KB_KEY_0			= 11,
		KB_KEY_MINUS	= 12,
		KB_KEY_EQUALS	= 13,
		KB_KEY_BACK		= 14,
		KB_KEY_TAB		= 15,
		KB_KEY_Q			= 16,
		KB_KEY_W			= 17,
		KB_KEY_E			= 18,
		KB_KEY_R			= 19,
		KB_KEY_T			= 20,
		KB_KEY_Y			= 21,
		KB_KEY_U			= 22,
		KB_KEY_I			= 23,
		KB_KEY_O			= 24,
		KB_KEY_P			= 25,
		KB_KEY_FORWARD_BRACKET		= 26,
		KB_KEY_REVERSE_BRACKET		= 27,
		KB_KEY_ENTER			= 28,
		KB_KEY_LCTRL			= 29,
		KB_KEY_A			= 30,
		KB_KEY_S			= 31,
		KB_KEY_D			= 32,
		KB_KEY_F			= 33,
		KB_KEY_G			= 34,
		KB_KEY_H			= 35,
		KB_KEY_J			= 36,
		KB_KEY_K			= 37,
		KB_KEY_L			= 38,
		KB_KEY_SEMI_COLON		= 39,
		KB_KEY_SINGLE_QUOTE		= 40,
		KB_KEY_REVERSE_SINGLE_QUOTE	= 41,
		KB_KEY_LSHIFT			= 42,
		KB_KEY_BACKSLASH		= 43,
		KB_KEY_Z			= 44,
		KB_KEY_X			= 45,
		KB_KEY_C			= 46,
		KB_KEY_V			= 47,
		KB_KEY_B			= 48,
		KB_KEY_N			= 49,
		KB_KEY_M			= 50,
		KB_KEY_COMMA			= 51,
		KB_KEY_PERIOD			= 52,
		KB_KEY_SLASH			= 53,
		KB_KEY_RSHIFT			= 54,
		KB_KEY_STAR			= 55,
		KB_KEY_LALT			= 56,
		KB_KEY_SPACE			= 57,
		KB_KEY_CAPSLOCK			= 58,
		KB_KEY_F1			= 59,
		KB_KEY_F2			= 60,
		KB_KEY_F3			= 61,
		KB_KEY_F4			= 62,
		KB_KEY_F5			= 63,
		KB_KEY_F6			= 64,
		KB_KEY_F7			= 65,
		KB_KEY_F8			= 66,
		KB_KEY_F9			= 67,
		KB_KEY_F10			= 68,
		KB_KEY_NUMLOCK			= 69,
		KB_KEY_SCROLLLOCK		= 70,
		KB_KEY_HOME			= 71,
		KB_KEY_UP			= 72,
		KB_KEY_PGUP			= 73,
		KB_KEY_KEYPAD_MINUS		= 74,
		KB_KEY_LEFT			= 75,
		KB_KEY_CENTER			= 76,
		KB_KEY_RIGHT			= 77,
		KB_KEY_KEYPAD_PLUS		= 78,
		KB_KEY_END			= 79,
		KB_KEY_DOWN			= 80,
		KB_KEY_PGDN			= 81,
		KB_KEY_INSERT			= 82,
		KB_KEY_DEL			= 83,
		KB_KEY_F11			= 87,
		KB_KEY_F12			= 88,
		KB_KEY_KEYPAD_ENTER		= 284,
		KB_KEY_RCTRL			= 285,
		KB_KEY_KEYPAD_SLASH		= 309,
		KB_KEY_RALT			= 312,
		KB_KEY_EXT_NUMLOCK		= 325,
		KB_KEY_GRAY_HOME		= 327,
		KB_KEY_GRAY_UP			= 328,
		KB_KEY_GRAY_PGUP		= 329,
		KB_KEY_GRAY_LEFT		= 331,
		KB_KEY_GRAY_RIGHT		= 333,
		KB_KEY_GRAY_END			= 335,
		KB_KEY_GRAY_DOWN		= 336,
		KB_KEY_GRAY_PGDN		= 337,
		KB_KEY_GRAY_INS			= 338,
		KB_KEY_GRAY_DEL			= 339,
		KB_KEY_META			= 340
	};

	enum EKeyboardModifiers
	{
		KB_MODIFIER_NONE		= (0 << 0),
		KB_MODIFIER_CTRL		= (1 << 0),
		KB_MODIFIER_ALT			= (1 << 1),
		KB_MODIFIER_SHIFT		= (1 << 2),
		KB_MODIFIER_META		= (1 << 3)
	};

	enum EGaugeHilite
	{
		GAUGE_HILITE_NONE	= 0,
		GAUGE_HILITE_ARROW	= (1 << 0),
		GAUGE_HILITE_BOX	= (1 << 1),
		GAUGE_HILITE_CIRCLE	= (1 << 2),
		GAUGE_HILITE_BLINK	= (1 << 8)
	};

	enum EWeatherObstructionTypes
	{
		WEATHER_INTENSITY_VICINITY      	= 0x00000001,	// Weather intensity (bits 0-3)
		WEATHER_INTENSITY_LIGHT			= 0x00000002,
		WEATHER_INTENSITY_MODERATE		= 0x00000004,
		WEATHER_INTENSITY_HEAVY			= 0x00000008,
		WEATHER_DESCRIPTOR_SHALLOW		= 0x00000010,	// Weather description (4-11)
		WEATHER_DESCRIPTOR_PARTIAL		= 0x00000020,
		WEATHER_DESCRIPTOR_PATCHES		= 0x00000040,
		WEATHER_DESCRIPTOR_LOW_DRIFTING		= 0x00000080,
		WEATHER_DESCRIPTOR_BLOWING		= 0x00000100,
		WEATHER_DESCRIPTOR_SHOWERS		= 0x00000200,
		WEATHER_DESCRIPTOR_THUNDERSTORMS	= 0x00000400,
		WEATHER_DESCRIPTOR_FREEZING		= 0x00000800,
		WEATHER_PHENOMENA_DRIZZLE		= 0x00001000,	// Weather phenomena (bits 12-31)
		WEATHER_PHENOMENA_RAIN			= 0x00002000,
		WEATHER_PHENOMENA_SNOW			= 0x00004000,
		WEATHER_PHENOMENA_SNOW_GRAINS		= 0x00008000,
		WEATHER_PHENOMENA_ICE_CRYSTALS		= 0x00010000,
		WEATHER_PHENOMENA_ICE_PELLETS		= 0x00020000,
		WEATHER_PHENOMENA_HAIL			= 0x00040000,
		WEATHER_PHENOMENA_SMALL_HAIL_OR_SNOW	= 0x00080000,
		WEATHER_PHENOMENA_FOG			= 0x00100000,
		WEATHER_PHENOMENA_UNIDENTIFIED		= 0x00200000,
		WEATHER_PHENOMENA_VOLCANIC_ASH		= 0x00400000,
		WEATHER_PHENOMENA_SQUALL		= 0x00800000,
		WEATHER_PHENOMENA_WIDESPREAD_DUST	= 0x01000000,
		WEATHER_PHENOMENA_SPRAY			= 0x02000000,
		WEATHER_PHENOMENA_SAND			= 0x04000000,
		WEATHER_PHENOMENA_DUST_SAND_WHIRLS	= 0x08000000,
		WEATHER_PHENOMENA_FUNNEL_CLOUD		= 0x10000000,
		WEATHER_PHENOMENA_SAND_STORM		= 0x20000000,
		WEATHER_PHENOMENA_DUST_STORM		= 0x40000000
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
		GROUND_ASPHALT = 2,
		GROUND_TURF = 3,
		GROUND_DIRT = 4,
		GROUND_GRAVEL = 5,
		GROUND_METAL = 6,
		GROUND_SAND = 7,
		GROUND_WOOD = 8,
		GROUND_WATER = 9,
		GROUND_MATS = 10,
		GROUND_SNOW = 11,
		GROUND_ICE = 12,
		GROUND_GROOVED = 13,
		GROUND_TREATED = 14
	};

	enum EUIComponentTypes
	{
		COMPONENT_WINDOW = 'wind',
		COMPONENT_BUTTON = 'butn',
		COMPONENT_POPUPMENU = 'menu',
		COMPONENT_CHECKBOX = 'chek',
		COMPONENT_RADIOBUTTON = 'radi',
		COMPONENT_LABEL = 'labl',
		COMPONENT_LINE = 'line',
		COMPONENT_BOX = 'box_',
		COMPONENT_PICTURE = 'pict',
		COMPONENT_MENUBAR = 'mbar',
		COMPONENT_SCROLLBAR = 'scrl',
		COMPONENT_SLIDER = 'slid',
		COMPONENT_GROUPBOX = 'gbox',
		COMPONENT_GAUGE = 'gage',
		COMPONENT_LIST = 'list',
		COMPONENT_SCROLLAREA = 'scla',
		COMPONENT_TEXTFIELD = 'txtf',
		COMPONENT_TEXTAREA = 'txta',
		COMPONENT_PROGRESS = 'prog',
		COMPONENT_MAP = 'map_',
		COMPONENT_TABPAGE = 'tabp',
		COMPONENT_TABBUTTON = 'tabb',
		COMPONENT_TABCONTROL = 'tabc',
		COMPONENT_GRAPH = 'grph'
	};

	enum EUIComponentEvents
	{
		EVENT_IDCHANGED		='dtid',
		EVENT_HIDDEN		='hide',	
		EVENT_SHOWN		='show',
		EVENT_ENABLED		='enbl',	
		EVENT_DISABLED		='!enb',
		EVENT_RESIZED		='size',
		EVENT_MOVED		='move',
		EVENT_RENAMED		='name',
		EVENT_COMPONENTDIED	= 'dead',
		EVENT_CHECKED		= 'chek',	
		EVENT_UNCHECKED		= 'uchk',
		EVENT_SELECTITEM	= 'slct',
		EVENT_BUTTONPRESSED	= 'actn',
		EVENT_TEXTCHANGED	= 'delt',
		EVENT_TEXTSET		= 'tset',
		EVENT_VALUECHANGED	= 'valu',
		EVENT_OPENWINDOW	='open',
		EVENT_CLOSEWINDOW	='clos',
		EVENT_TITLECHANGED	='titl',
		EVENT_FOCUSLOST		= '!foc',
		EVENT_FOCUSGAINED	= 'foci',
		EVENT_WIDGETTYPECHANGED	= 'widg',
		EVENT_DIALOGDISMISSED	= 'dlog',
		EVENT_DIALOGDISMISSEDOK	= 'dlOK',
		EVENT_DIALOGDISMISSEDCANCEL	= 'dlCn',
		EVENT_ANYSUBEVENT	= 0,		
		EVENT_NOSUBEVENT	= 0
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

	enum EGestaltFlags
	{
		GESTALT_OBJECT_NEEDS_TO_DRAW	=	(1 << 0),
		GESTALT_OBJECT_NEEDS_TIME_SLICE	=	(1 << 1),
		GESTALT_OBJECT_HAS_MODEL	=	(1 << 2),
		GESTALT_OBJECT_HAS_CAMERA	=	(1 << 3),
		GESTALT_OBJECT_CAN_ACTIVATE	=	(1 << 4),
		GESTALT_OBJECT_HAS_CONTROLS	=	(1 << 5),
		GESTALT_OBJECT_UNDER_USER_CONTROL	=	(1 << 6),	// set by Fly!; don't modify!
		GESTALT_PRIMARY_USER_OBJECT	=	(1 << 7),	// set by Fly!; don't modify
		GESTALT_STATIC_SCENERY_OBJECT	=	(1 << 8),	// objects saved to scenery database
		GESTALT_DYNAMIC_SCENERY_OBJECT	=	(1 << 9),	// objects NOT saved to scenery database
		GESTALT_DRAW_OBJECT_NAME	=	(1 << 10),	// draw popup name when pointed at with mouse
		GESTALT_OBJECT_IS_CLOUD		=	(1 << 11),
		GESTALT_DO_NOT_AUTO_SNAP	=	(1 << 12),	// snap object to the ground automatically 
		GESTALT_OBJECT_IS_TOWER		=	(1 << 13),	// used to designate for tower camera
		GESTALT_OBJECT_ANIMATED		=	(1 << 14),	// animated scenery (cars, trucks, boats, etc.)
		GESTALT_OBJECT_COLLISIONS	=	(1 << 15),	// collision detection with this object (default ON)
		GESTALT_DISTANCE_HIDING		=	(1 << 16),	// object hidden when eye past a threshold distance
		GESTALT_SINK_UNDERGROUND	=	(1 << 17),	// sink model by 8-10 feet underground (for raised sidewalks)
		GESTALT_DRAW_NIGHT_LIGHTS	=	(1 << 18),	// force night lights on models
		GESTALT_DRAW_SHADOW		=	(1 << 19),	// draw shadows for this object??
		GESTALT_OBJECT_GENERATED	=	(1 << 20),	// object was generated by an automated external source (DLL)
		GESTALT_NO_Z_BUFFER		=	(1 << 21),	// object doesn't use Z-buffer, draws after taxiways and runways, but before models (ground striping)
		GESTALT_Z_SORTED		=	(1 << 22),	// object must sort in Z before drawing (back to front)
		GESTALT_NO_Z_BUFFER_UNDERLAY	=	(1 << 23)	// object doesn't use Z-buffer, draws before taxiways and runways (ground underlay)
	};

	/////////////////////////////////////////////////////////////
	//
	//	Structures/Typedefs
	//
	/////////////////////////////////////////////////////////////

	typedef struct SDLLCopyright
	{
		char	product[128];
		char	company[128];
		char	programmer[128];
		char	dateTimeVersion[128];
		char	email[128];
		int	internalVersion;
	} SDLLCopyright;

	typedef struct SDLLRegisterTypeList
	{
		EDLLObjectType	type;
		long		signature;
		SDLLRegisterTypeList	*next;
	} SDLLRegisterTypeList;

	typedef void* DLLObjectRef;
	typedef void* DLLFileRef;

	typedef struct SFlyObjectRef
	{
		void	*objectPtr;
		void	*superSig;
		void	*classSig;
	} SFlyObjectRef;

	typedef struct SFlyObjectList
	{
		SFlyObjectRef	ref;
		SFlyObjectList	*prev;
		SFlyObjectList	*next;
	} SFlyObjectList;

	typedef struct SDLLObject
	{
		//
		//	public; set this to any value you want to uniquely
		//	identify an "instance" of your object on the .DLL
		//	side.  this can be a gauge, system, or other 
		//	component.
		//

		DLLObjectRef	dllObject;	// set by .DLL; can be any value you want

		//
		//	private; do not use or modify the following members
		//

		DLLFileRef	dllFile;	// set by Fly!; do NOT modify this value!!!
		SFlyObjectRef	flyObject;	// set by Fly!; do NOT modify this value!!!
	} SDLLObject;

	typedef struct SMessage
	{
		unsigned int	id;				// message ID
		unsigned int	group;				// target group ID
		EMessageDataType	dataType;		// result data type
		unsigned int	result;				// message result code

		DLLObjectRef	sender;
		DLLObjectRef	receiver;

		union 
		{
			char	charData;
			char	*charPtrData;
			int	intData;
			int	*intPtrData;
			double	realData;
			double	*realPtrData;
			void	*voidData;
		};
				
		union
		{
			unsigned int	userData[8];
			struct 
			{
				unsigned int	unit;		// unit number
				unsigned int	hw;		// hardware type
				unsigned int	engine;		// engine number
				unsigned int	tank;		// tank number
				unsigned int	gear;		// gear number
				unsigned int	unused;		// unused
				unsigned int	datatag;	// data tag
				unsigned int	sw;		// switch position
			} u;
		} user;

		unsigned int	action;
	} SMessage;

	typedef struct SSurface
	{
		unsigned int	*drawBuffer;		// surface buffer
		unsigned int	xSize, ySize;		// surface dimensions
		unsigned int	xSpan;			// width in pixels of each line span (aligned)
		int		xScreen, yScreen;	// screen coordinates for upper-left corner
	} SSurface;

	typedef struct SFont
	{
		char	fontName[64];		// -> passed to APILoadFont; name of font family image file
		void	*font;			// <- returned from APILoadFont; do NOT alter!!!
	} SFont;

	typedef struct SBitmap
	{
		char		bitmapName[64];	// -> passed to APILoadBitmap
		void		*bitmap;	// <- returned from APILoadBitmap; do NOT alter!!!
		EBitmapType	type;		// <- returned from APILoadBitmap; do NOT alter!!!
	} SBitmap;

	typedef struct SStream
	{
		char	filename[64];
		char	mode[3];
		void	*stream;
	} SStream;

	typedef struct SVector
	{
		union {
			double	x;
			double	p;	// when used as a polar, pitch (in radians)
		};

		union {
			double	y;
			double	h;	// when used as a polar, heading (in radians)
		};

		union {
			double	z;
			double	r;	// when used as a polar, radius (in feet)
		};
	} SVector;

	typedef struct SPosition
	{
		double	lat;
		double	lon;
		double	alt;
	} SPosition;

	typedef struct SPositionList
	{
		SPosition	pos;		// points should be stored clockwise
		SPositionList	*next;
	} SPositionList;

	typedef struct SCollision
	{
		char		collided;	// 0 or 1
		SPosition	pos;		// collision position
		SFlyObjectRef	object;		// collision object
	} SCollision;

	typedef struct SMovie
	{
		void	*gc;
		void	*movie;
		SSurface	surface;
	} SMovie;

	typedef struct SNavaid
	{
		char		name[40];
		char		id[8];

		int		type;
		int		classification;
		int		usage;

		SPosition	pos;
		float		freq;
		float		range;
		float		magneticVariation;
		float		slavedVariation;

		SNavaid		*prev;	// double-linked list
		SNavaid		*next;	// double-linked list
	} SNavaid;

	typedef struct SILS
	{
		char		airportKey[10];
		char		runwayID[5];

		int		type;
		int		classification;
		SPosition	pos;
		float		range;
		float		magneticVariation;

		float		glideslopeAngle;
		float		approachBearing;
		char		backCourseAvailable;

		SILS		*prev;	// double-linked list
		SILS		*next;	// double-linked list

		//	version 2.0 additions

		float		freq;
	} SILS;

	typedef struct SComm
	{
		char		airportKey[10];

		char		name[20];
		int		type;
		float		freq[5];
		SPosition	pos;

		SComm		*prev;	// double-linked list
		SComm		*next;	// double-linked list
	} SComm;

	typedef struct SWaypoint
	{
		char		name[26];
		char		colocated;	// 0 or 1
		int		type;
		int		usage;
		float		bearingToNavaid;
		float		distanceToNavaid;
		SPosition	pos;
		char		navaid[10];

		SWaypoint	*prev;
		SWaypoint	*next;
	} SWaypoint;

	typedef struct SRunwayEnd
	{
		char		id[4];				// painted end marking
		float		trueHeading;			// in degrees
		float		magneticHeading;		// in degrees
		int		markings;			// numbers only, PIR, STOL, etc.
		SPosition	pos;				// lat/lon position of center of runway end
		int		thresholdCrossingHeight;	// in feet
		int		displacedThreshold;		// in feet
		int		rvr;				// touchdown, midfield, rollout
		char		rvv;				// 0..1
		char		edgeLights;			// 0..1
		char		endLights;			// 0..1
		char		centerlineLights;		// 0..1
		char		touchdownLights;		// 0..1
		char		alignLights;			// 0..1
		char		thresholdLights;		// 0..1
		char		sequenceFlashing;		// 0..1
		int		numLightSystems;		// 0..1
		int		lightSystems[8];		// 0..1
		char		edgeLightIntensity;		// 0..1
		int		glideslopeType;			// 0..1
		int		glideslopePlacement;		// 0..1
		int		glideslopeConfiguration;	// 0..1

		SILS		*glideslope;			// associated GS
		SILS		*localizer;			// associated LOC
	} SRunwayEnd;

	typedef struct SRunway
	{
		char		airportKey[10];

		float		length;			// in feet
		float		width;			// in feet
		int		surfaceType;		// surface material (asphalt, concrete, etc.)
		int		surfaceCondition;	// condition (good, fair, poor)
		char		pavementType;		// rigid or flexible
		int		pcn;			// pavement classification number (FAA)
		int		subgradeStrength;	// high, medium, low, very low
		int		tirePressure;		// high, medium, low, very low
		char		closed;			// 0..1
		char		pcl;			// 0..1
		char		pattern;		// left or right
		float		elevation;		// in feet

		SRunwayEnd	base;			// base end data
		SRunwayEnd	recip;			// reciprocal end data

		SRunway		*prev;			// double-linked list
		SRunway		*next;			// double-linked list
	} SRunway;

	typedef struct SAirport
	{
		char		airportKey[10];
		char		faaID[5];
		char		icaoID[5];
		int		type;
		char		name[40];
		char		country[3];
		char		state[3];
		char		county[40];
		char		city[40];
		float		elevation;
		int		ownership;
		int		usage;
		float		magneticVariation;
		float		trafficAltitude;
		char		fssAvailable;		// 0..1
		char		notamAvailable;		// 0..1
		char		atcAvailable;		// 0..1
		char		segmentedCircle;	// 0..1
		char		landingFeesCharged;	// 0..1
		char		jointUseAirport;	// 0..1
		char		militaryLandingRights;	// 0..1
		char		customsEntryPoint;	// 0..1
		int		fuelTypes;
		int		frameService;
		int		engineService;
		int		bottledOxygen;
		int		bulkOxygen;
		int		beaconLensColor;
		int		basedAircraft;
		int		annualCommercialOps;
		int		annualGeneralAvOps;
		int		annualMilitaryOps;
		int		attendanceFlags;
		int		lightingFlags;
		SPosition	pos;
				
		SRunway		*runways;		// list of runways
		SComm		*comms;			// list of radio comms
			
		SAirport	*prev;			// double-linked list
		SAirport	*next;			// double-linked list
	} SAirport;

	typedef struct SGeneric
	{
		void	*data;

		SGeneric	*prev;
		SGeneric	*next;
	} SGeneric;

	typedef struct SWeatherInfo
	{
		//
		//	weather data
		//

		double	visibility;		// in nautical miles (10.0 = 10 nautical miles)
		int	surfaceTemp;		// degrees F
		int	dewPointTemp;		// degrees F
		double	altimeter;		// inches Hg
		int	windSpeed;		// knots
		int	gustSpeed;		// knots
		int	windHeading;		// degrees
		char	windFluctuates;		// 0=no fluctuation, 1=fluctuates
		int	precipIntensity;	// valid values are:
						//	2=Light
						//	4=Medium
						//	8=Heavy (Thunderstorms if precipType is rain)
		int	precipType;		// valid values are:
						//	0x2000=Rain (8192 decimal)
						//	0x4000=Snow (16384 decimal)

		//
		//	cloud layers
		//

		int	layer1Active;		// 0 if no cloud layer, 1 if active
		int	layer1Altitude;		// in feet
		int	layer1Type;		// valid values are:
						//	2=Few
						//	3=Scattered
						//	4=Broken
						//	5=Overcast
						//	6=Fog
		int	layer1Height;		// in feet

		int	layer2Active;		// 0 if no cloud layer, 1 if active
		int	layer2Altitude;		// in feet
		int	layer2Type;		// valid values are:
						//	2=Few
						//	3=Scattered
						//	4=Broken
						//	5=Overcast
						//	6=Fog
		int	layer2Height;		// in feet

		int	layer3Active;		// 0 if no cloud layer, 1 if active
		int	layer3Altitude;		// in feet
		int	layer3Type;		// valid values are:
						//	2=Few
						//	3=Scattered
						//	4=Broken
						//	5=Overcast
						//	6=Fog
		int	layer3Height;		// in feet
	} SWeatherInfo;

	typedef struct SWindsAloft
	{
		int	windHeading;		// in degrees
		int	windSpeed;		// in knots
		int	gustSpeed;		// in knots
		char	windFluctuates;		// non-zero if TRUE, zero if FALSE
	} SWindsAloft;

	typedef struct SDate
	{
		unsigned int	month;
		unsigned int	day;
		unsigned int	year;
	} SDate;

	typedef struct STime
	{
		unsigned int	hour;
		unsigned int	minute;
		unsigned int	second;
		unsigned int	msecs;
	} STime;

	typedef struct SDateTime
	{
		SDate	date;
		STime	time;
	} SDateTime;

	typedef struct SDateTimeDelta
	{
		unsigned int	dYears;
		unsigned int	dMonths;
		unsigned int	dDays;
		unsigned int	dHours;
		unsigned int	dMinutes;
		unsigned int	dSeconds;
		unsigned int	dMillisecs;
	} SDateTimeDelta;

	typedef struct SFPAirport
	{
		char		landHere;	// 0 or 1
		SDateTimeDelta	layover;
		SAirport	data;
		char		depRunwayEnd[4];	// runway end ID
		char		arrRunwayEnd[4];	// runway end ID
	} SFPAirport;

	typedef struct SFPNavaid
	{
		SNavaid		data;
	} SFPNavaid;

	typedef struct SFPWaypoint
	{
		SWaypoint	data;
	} SFPWaypoint;

	typedef struct SFPUser
	{
		char	name[40];
	} SFPUser;

	typedef struct SFPEntry
	{
		EFPEntryType	type;
		SPosition	pos;
		SDateTime	arrival;
		SDateTime	departure;
		SDateTimeDelta	extend;
		float		distance;
		float		bearing;
		float		speed;
		float		altitude;
		float		effectiveSpeed;
		float		effectiveAltitude;
		float		magneticVariation;

		union		// 'type' determines which struct is populated
		{
			SFPAirport	a;
			SFPNavaid	n;
			SFPWaypoint	w;
			SFPUser		u;
		};
	} SFPEntry;

	typedef struct SFlightPlan
	{
		SFPEntry	entry;
		SFlightPlan	*prev;
		SFlightPlan	*next;
	} SFlightPlan;
*/
	typedef struct SScriptInfo
	{
		char	moduleName[128];
		char	version[128];
		double	frameSecs;
		double	totalSecs;
	} SScriptInfo;

	typedef struct SPythonMethod
	{
		char		*name;
		void		*method;
		int		flags;
		char		*doc;
	} SPythonMethod;

	#ifdef __MWERKS__
	#pragma options align = packed
	#else
	#pragma pack(1)
	#endif

	typedef struct SEpicEvent
	{
		unsigned char	next;			// idle link list
		unsigned char	cmd;			// EPIC command
		unsigned char	data[6];	
	} SEpicEvent;

	#ifdef __MWERKS__
	#pragma options align = reset
	#else
	#pragma pack()
	#endif

	typedef struct SDataBlockItem
	{
		unsigned int	label;
		union
		{
			float	floatData;
			int	intData;
		};
	} SDataBlockItem;

	typedef struct SFile
	{
		void	*file;
	} SFile;


	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////
	//
	//	SDK API
	//
	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////
    
    void APITestAPISdk (void);

	/////////////////////////////////////////////////////////////
	//
	//	Fly! function entry points
	//
	/////////////////////////////////////////////////////////////

	//
	//	object creation, type registration
	//

	SDLLObject*	APICreateDLLObject(void*);
	SDLLObject*	APIInstantiate(unsigned int, unsigned int);
	SDLLObject*	APICreateScriptObject(DLLFileRef dll, DLLObjectRef ref);
	void		APIDestroyScriptObject(SDLLObject *object);
	void		APIAddRegisteredType(SDLLRegisterTypeList **list, EDLLObjectType type, const long signature);
	void		APIRegisterType(EDLLObjectType type, const long signature);
	void		APIUnregisterType(EDLLObjectType type, const long signature);
	int		APIGetSDKVersion(void);
	void		APIGetFlyVersion(int *majorRevision, int *minorRevision, int *buildNumber);	// i.e. 2.0.218

	//
	//	messaging primitives
	//

	void	APISendMessage(SMessage *msg);

	//
	//	drawing primitives
	//

	SSurface*	APICreateSurface(int width, int height);
	void	APIFreeSurface(SSurface *surface);
	void	APIEraseSurface(SSurface *surface);
	void	APIEraseSurfaceRGB(SSurface *surface, unsigned int rgbColor);
	void	APIDrawDot(SSurface *surface, int x, int y, unsigned int rgbColor);
	void	APIDrawLine(SSurface *surface, int x1, int y1, int x2, int y2, unsigned int rgbColor);
	void	APIDrawRect(SSurface *surface, int x1, int y1, int x2, int y2, unsigned int rgbColor);
	void	APIFillRect(SSurface *surface, int x1, int y1, int x2, int y2, unsigned int rgbColor);
	void	APIDrawCircle(SSurface *surface, int xCenter, int yCenter, int radius, unsigned int rgbColor);
	unsigned int	APIMakeRGB(unsigned int r, unsigned int g, unsigned int b);
	void	APIBlit(SSurface *surface);
	void	APIBlitPartial(SSurface *surface, int x1, int y1, int x2, int y2);
	void	APIBlitTransparent(SSurface *surface, unsigned int rgbTransparentColor); 
	void	APIBlitPanelToSurface(SSurface *surface, SDLLObject *object);
	void	APICreateOverlay(SDLLObject *refObject, SSurface *surface, const int x, const int y);
	void	APIRemoveOverlay(SSurface *surface);

	//
	//	font primitives
	//

	int	APILoadFont(SFont *font);
	void	APIFreeFont(SFont *font);
	void	APIDrawText(SSurface *surface, SFont *font, int x, int y, unsigned int rgbColor, const char *text);
	void	APIDrawTextC(SSurface *surface, SFont *font, int x, int y, unsigned int rgbColor, const char *text);
	void	APIDrawTextR(SSurface *surface, SFont *font, int x, int y, unsigned int rgbColor, const char *text);
	int	APITextHeight(SFont *font, const char *text);
	int	APITextWidth(SFont *font, const char *text);
	int	APICharHeight(SFont *font, char c);
	int	APICharWidth(SFont *font, char c);
	int	APIMaxCharHeight(SFont *font);
	int	APIMaxCharWidth(SFont *font);

	//
	//	bitmap primitives
	//

	int	APILoadBitmap(SBitmap *bitmap);
	void	APIDrawBitmap(SSurface *surface, SBitmap *bitmap, int x, int y, int frame);
	void	APIDrawBitmapPartial(SSurface *surface, SBitmap *bitmap, int dx, int dy, int sx1, int sy1, int sx2, int sy2, int frame);
	void	APIGetBitmapSize(SBitmap *bitmap, int *xSize, int *ySize);
	int	APINumBitmapFrames(SBitmap *bitmap);
	void	APIFreeBitmap(SBitmap *bitmap);

	//
	//	stream primitives
	//

	int	APIOpenStream(SStream *stream);
	void	APICloseStream(SStream *stream);
	int	    APIDoesFileExist(const char *filename, EFileSearchLocation _where);
	int		APIDoesPodVolumeExist(const char *volumeName);

	void	APIWriteTo(SDLLObject *object, SStream *stream);
	void	APIReadFrom(SDLLObject *object, SStream *stream);
	void	APISkipObject(SStream *stream);
	void	APIAdvanceToTag(unsigned int tag, SStream *stream);

	void	APIReadInt(int *value, SStream *stream);
	void	APIReadUInt(unsigned int *value, SStream *stream);
	void	APIReadFloat(float *value, SStream *stream);
	void	APIReadDouble(double *value, SStream *stream);
	void	APIReadString(char *value, int maxLength, SStream *stream);
	void	APIReadVector(SVector *value, SStream *stream);
	void	APIReadPosition(SPosition *value, SStream *stream);
	void	APIReadTime(SDateTime *value, SStream *stream);
	void	APIReadTimeDelta(SDateTimeDelta *value, SStream *stream);
	void	APIReadMessage(SMessage *message, SStream *stream);

	void	APIWriteInt(int *value, SStream *stream);
	void	APIWriteUInt(unsigned int *value, SStream *stream);
	void	APIWriteFloat(float *value, SStream *stream);
	void	APIWriteDouble(double *value, SStream *stream);
	void	APIWriteString(char *value, SStream *stream);
	void	APIWriteVector(SVector *value, SStream *stream);
	void	APIWritePosition(SPosition *value, SStream *stream);
	void	APIWriteTime(SDateTime *value, SStream *stream);
	void	APIWriteTimeDelta(SDateTimeDelta *value, SStream *stream);
	void	APIWriteMessage(SMessage *message, SStream *stream);

	//
	//	file primitives
	//

	int	APIGetFileLength(const char *dir, const char *file);
	int	APIOpenFile(const char *dir, const char *file, SFile *fp);
	void	APICloseFile(SFile *fp);
	int	APIReadFromFile(SFile *fp, unsigned int numBytes, void *buffer);

	//
	//	memory primitives
	//

	void*	APIAllocMem(unsigned int bytes);
	void	APIFreeMem(void *address);

	//
	//	sound primitives
	//

	int	APIPlaySfx(char *wavfile);
	int	APIPlaySfxDelay(char *wavfile, float delay, float *length);
	void	APIStopSfx(int sfxID);				// parameter is value returned from APIPlaySfx
	void	APIStopSfxByUserData(int user1, int user2);
	int	APIIsValidSfx(int sfxID);
	void	APISetSfxVolume(int sfxID, float volume);	// volume is 0.0...1.0
	void	APISetSfxPosition(int sfxID, SPosition *p);
	void	APISetSfxDirection(int sfxID, SVector *v);
	void	APISetSfxVelocity(int sfxID, SVector *v);
	void	APISetSfxFrequency(int sfxID, float freqScalar);	// 1.0 is original rate; 2.0 is 2x rate, etc.
	void	APISetSfxUserData(int sfxID, int user1, int user2);
	void	APITextToSpeech(const char *text);
	int	APITextToSpeechAvailable(void);

	//
	//	mouse primitives
	//

	void	APISetMouseCursor(EMouseCursorStyle style);

	//
	//	keyboard primitives
	//

	void	APIAddKeyHandler(int keyCode, int modifiers, SDLLObject *object);
	void	APIRemoveKeyHandler(int keyCode, int modifiers, SDLLObject *object);
	void	APIDebounceKey(int keyCode, int modifiers);
	void	APIAddKeyHandler2(int keyID, SDLLObject *object);
	void	APIRemoveKeyHandler2(int keyID, SDLLObject *object);
	void	APIDebounceKey2(int keyID);
	void	APIMimicKeystroke(unsigned int keyID);
	void	APIDefineKey(int keyID, char *keyFunction, int keyCode, int modifiers);
	void	APIGetKeyCode(int keyID, int *keyCode, int *modifiers);
	int	APIGetKeyAssignment(int keyID);

	//
	//	OS-independent DLL primitives
	//

	void*	APILoadLibrary(char *name);
	void*	APIGetLibraryRoutine(void *library, char *funcName);
	void	APIFreeLibrary(void *library);

	//
	//	joystick functions
	//

	int	APIHasJoystickAxis(EJoystickAxis axis);
	float	APIGetJoystickAxis(EJoystickAxis axis);
	int	APIGetJoystickButtonCount(void);
	int	APIIsJoystickButtonPressed(int buttonIndex);

	//
	//	gauge functions
	//

	float	APIGetGaugeLightLevel(SDLLObject *object);
	float	APIGetGaugeAmbientLightLevel(SDLLObject *object);
	void	APIHiliteGauge(unsigned int gaugeID, unsigned int hiliteStyle, unsigned int hiliteColor, float timerInSecs);
	void	APIScrollToGauge(unsigned int gaugeID);

	//
	//	systems functions
	//

	void	APISetSystemHardwareType(SDLLObject *object, EMessageHWType hwType);
	EMessageHWType	APIGetSystemHardwareType(SDLLObject *object);
	void	APISetSystemIndication(SDLLObject *object, double indication);
	double	APIGetSystemIndication(SDLLObject *object);
	long	APIGetSystemUnitID(SDLLObject *object);
	int	APIGetSystemUnitNumber(SDLLObject *object);
	void	APISetSystemState(SDLLObject *object, int state);
	int	APIGetSystemState(SDLLObject *object);
	int	APIIsSystemActive(SDLLObject *object);
	void	APISetSystemLoad(SDLLObject *object, double amps);
	double	APIGetSystemLoad(SDLLObject *object);
	void	APISetSystemMaxVoltage(SDLLObject *object, double voltage);
	double	APIGetSystemMaxVoltage(SDLLObject *object);

	//
	//	database functions
	//

	void	APIGetLocalNavaids(SNavaid **navaids);	 	// caller must call APIFree... when done with list
	void	APIGetLocalILS(SILS **ils);			// caller must call APIFree... when done with list
	void	APIGetLocalComms(SComm **comms);		// caller must call APIFree... when done with list
	void	APIGetLocalCenters(SComm **comms);		// caller must call APIFree... when done with list
	void	APIGetLocalAirports(SAirport **airports);	// caller must call APIFree... when done with list
	void	APIGetLocalWaypoints(SWaypoint **waypoints);	// caller must call APIFree... when done with list

	int	APIGetNearestNavaid(SPosition *pos, int navType, SNavaid *navaid, float *distanceInFeet);
	int	APIGetNearestNavaidByFreq(SPosition *pos, int navType, float freq, SNavaid *navaid, float *distanceInFeet);
	int	APIGetNearestILS(SPosition *pos, float freq, SILS *ils, float *distanceInFeet);
	int	APIGetNearestComm(SPosition *pos, float freq, SComm *comm, float *distanceInFeet);
	int	APIGetNearestAirportComm(SPosition *pos, float freq, SComm *comm, float *distanceInFeet);
	int	APIGetNearestCenterComm(SPosition *pos, float freq, SComm *comm, float *distanceInFeet);
	int	APIGetNearestAirport(SPosition *pos, SAirport *airport, float *distanceInFeet);
	int	APIInAirportRegion(SPosition *pos, float *distanceInFeet);
	int	APIGetAirport(char *airportKey, SAirport *airport);

	int	APISearchNavaidsByID(char *id, int navType, SNavaid **navaids);
	int	APISearchNavaidsByName(char *name, int navType, SNavaid **navaids);
	int	APISearchWaypointsByName(char *name, SWaypoint **waypoints);
	int	APISearchAirportsByFAA(char *faaID, SAirport **airports);
	int	APISearchAirportsByICAO(char *icaoID, SAirport **airports);
	int	APISearchAirportsByName(char *name, SAirport **airports);
	int	APISearchILS(char *airportKey, char *runwayEndID, SILS **ils);
	int	APISearchDatabase(char *db, char *index, char *key, SGeneric **list);

	void	APIFreeNavaid(SNavaid *navaids);	// will free ALL linked navaids
	void	APIFreeILS(SILS *ils);			// will free ALL linked ils's
	void	APIFreeWaypoint(SWaypoint *waypoint);	// will free ALL linked waypoints
	void	APIFreeComm(SComm *comm);		// will free ALL linked comms
	void	APIFreeAirport(SAirport *airport);	// will free ALL linked airports
	void	APIFreeGeneric(SGeneric *list);		// will free ALL linked generic records

	//
	//	flight plan functions
	//

	void	APIGetFlightPlan(SFlightPlan **fp);
	void	APIFreeFlightPlan(SFlightPlan *fp);
	void	APIGetFPFirstEntry(SFPEntry *entry);
	void	APIGetFPLastEntry(SFPEntry *entry);
	void	APIGetFPToWaypoint(SFPEntry *entry);
	void	APIGetFPFromWaypoint(SFPEntry *entry);
	void	APIGetFPBeyondWaypoint(SFPEntry *entry);
	void	APIUpdateFPCurrentWaypoint(SPosition *pos);

	//
	//	object functions
	//
	//		note: most of these functions return an int error code (0=no error, 1=invalid object)
	//		because any of the objects returned to you from the Fly! sim can disappear at any
	//		moment due to the fact that model scenery and dynamic scenery are constantly being
	//		added and removed from the environment local to the user aircraft. You can call 
	//		APIIsValidObject() at any time to test the validity of the SFlyObject* you have 
	//		received from Fly!, but you should check the result codes from each of the following
	//		function calls before attempting to use the data returned if the object had gone
	//		out of scope
	//

	SDLLObject*	APICreateFlyObject(char *initFile);
	SDLLObject*	APICreateFlyObjectByType(unsigned int objectType);

	void	APIDestroyObject(SDLLObject *object);
	int	APIGetAircraft(SFlyObjectList **objects);		// aircraft only
	int	APIGetObjects(SFlyObjectList **objects);		// all objects (aircraft & models) in the sim
	void	APIFreeObjects(SFlyObjectList *objects);		// use to free allocations made in GetAircraft/GetObjects
	int	APIGetUserObject(SFlyObjectRef *object);
	int	APIGetNamedObject(char *name, SFlyObjectRef *object);
	int	APIIsValidObject(SFlyObjectRef *object);
	int	APIGetObjectType(SFlyObjectRef *object, EFlyObjectType *type);
	int	APIGetObjectPosition(SFlyObjectRef *object, SPosition *pos);
	int	APISetObjectPosition(SFlyObjectRef *object, SPosition *pos);
	int	APIGetObjectOrientation(SFlyObjectRef *object, SVector *orientation);
	int	APISetObjectOrientation(SFlyObjectRef *object, SVector *orientation);
	int	APIGetObjectBodyVelocity(SFlyObjectRef *object, SVector *velocity);
	int	APISetObjectBodyVelocity(SFlyObjectRef *object, SVector *velocity);
	int	APIGetObjectInertialVelocity(SFlyObjectRef *object, SVector *velocity);
	int	APISetObjectInertialVelocity(SFlyObjectRef *object, SVector *velocity);
	int	APIGetObjectAngularVelocity(SFlyObjectRef *object, SVector *velocity);
	int	APISetObjectAngularVelocity(SFlyObjectRef *object, SVector *velocity);
	int	APIGetObjectSpeed(SFlyObjectRef *object, float *speed);
	int	APISetObjectSpeed(SFlyObjectRef *object, float *speed);
	int	APIGetObjectAltitudeRate(SFlyObjectRef *object, float *speed);
	int	APIGetObjectInertialAcceleration(SFlyObjectRef *object, SVector *accel);
	int	APIGetObjectBodyAcceleration(SFlyObjectRef *object, SVector *accel);
	int	APIGetObjectAltitudeAcceleration(SFlyObjectRef *object, float *accel);
	int	APIGetObjectLateralAcceleration(SFlyObjectRef *object, float *accel);
	int	APIGetObjectTrueHeading(SFlyObjectRef *object, float *trueHeadingDegs);
	int	APIGetObjectMagneticHeading(SFlyObjectRef *object, float *magHeadingDegs);
	int	APIGetObjectMagneticVariation(SFlyObjectRef *object, float *magVarDeg);
	int	APIGetPartPosition(SFlyObjectRef *object, char *partName, SVector *pos);
	int	APISetPartPosition(SFlyObjectRef *object, char *partName, SVector *pos);
	int	APIGetPartOrientation(SFlyObjectRef *object, char *partName, SVector *orientation);
	int	APISetPartOrientation(SFlyObjectRef *object, char *partName, SVector *orientation);
	int	APIGetPartBodyOrientation(SFlyObjectRef *object, char *partName, SVector *bodyPos, SVector *bodyAngle);
	int	APISetPartBodyOrientation(SFlyObjectRef *object, char *partName, SVector *bodyPos, SVector *bodyAngle);
	int	APIRotatePart(SFlyObjectRef *object, char *partName, SVector *angle);
	int	APIShowPart(SFlyObjectRef *object, char *partName);
	int	APIHidePart(SFlyObjectRef *object, char *partName);
	int	APISetPart(SFlyObjectRef *object, char *partName, float status, float pos);
	int	APISetPartFrame(SFlyObjectRef *object, char *partName, int frameIndex);
	int	APIGetObjectName(SFlyObjectRef *object, char *name, int maxLength);
	int	APISetObjectName(SFlyObjectRef *object, char *name);
	int	APIGetTailNumber(SFlyObjectRef *object, char *name, int maxNameLength, char *abbrName, int maxAbbrLength);
	int	APISetTailNumber(SFlyObjectRef *object, char *name, char *abbreviatedName);
	int	APIGetObjectAGL(SFlyObjectRef *object, float *agl);
	int	APIGetObjectMSL(SFlyObjectRef *object, float *msl);
	int	APIGetObjectBodyGravity(SFlyObjectRef *object, SVector *gravity);
	int	APIGetObjectTotalWeight(SFlyObjectRef *object, float *weight);
	int	APIGetObjectPressureAltitude(SFlyObjectRef *object, float *altitude);
	int	APIGetObjectDensityAltitude(SFlyObjectRef *object, float *altitude);
	int	APIGetObjectInertialGroundSpeed(SFlyObjectRef *object, SVector *speed);
	int	APIGetObjectBodyGroundSpeed(SFlyObjectRef *object, SVector *speed);
	int	APIGetObjectGroundSpeed(SFlyObjectRef *object, float *speed);
	int	APIGetInertialWindVector(SFlyObjectRef *object, SVector *vector);
	int	APIGetBodyWindVector(SFlyObjectRef *object, SVector *vector);
	int	APIGetWindSpeed(SFlyObjectRef *object, float *speed);
	int	APIGetWindHeading(SFlyObjectRef *object, float *heading);
	int	APIGetHeadWind(SFlyObjectRef *object, float *headWind);
	int	APIGetTailWind(SFlyObjectRef *object, float *tailWind);
	int	APIGetOutsideAirTemperature(SFlyObjectRef *object, float *tempInF);
	int	APIGetOutsideAirPressure(SFlyObjectRef *object, float *inHg);
	int	APIGetTotalAirTemperature(SFlyObjectRef *object, float *tempInF);
	int	APIGetMachNumber(SFlyObjectRef *object, float *mach);
	int	APIGetModelRadius(SFlyObjectRef *object, float *radius);
	int	APIIsObjectNearGround(SFlyObjectRef *object, int *trueFalse);
	int	APIAreWheelsOnGround(SFlyObjectRef *object, int *wheelsOnGround);
	int	APIGetGroundTypeUnderObject(SFlyObjectRef *object, int *groundType);
	int	APIIsGearUp(SFlyObjectRef *object, int *gearUp);
	int	APIIsGearDown(SFlyObjectRef *object, int *gearDown);
	int	APIAreBrakesOn(SFlyObjectRef *object, int *brakesOn);
	int	APITuneRadio(SFlyObjectRef *object, ERadioType type, int radioUnit, float freq);
	int	APITuneRadio2(SFlyObjectRef *object, ERadioType type, int radioUnit, float freq, EFreqType whichFreq);
	float	APIGetTunedFreq(SFlyObjectRef *object, ERadioType type, int radioUnit, EFreqType whichFreq);
	int	APIGetTunedType(SFlyObjectRef *object, ERadioType type, int radioUnit);
	int	APIGetTunedNavaid(SFlyObjectRef *object, ERadioType type, int radioUnit, SNavaid *navaid);
	int	APIGetTunedILS(SFlyObjectRef *object, ERadioType type, int radioUnit, SILS *localizer, SILS *glideslope, SILS *outer, SILS *middle, SILS *inner, SILS *backcourse);
	int	APIGetTunedComm(SFlyObjectRef *object, ERadioType type, int radioUnit, SComm *comm);
	int	APIObjectInRegion(SFlyObjectRef *object, SPosition *ll, SPosition *ur, int *trueFalse);
	int	APIObjectInAirport(SFlyObjectRef *object, SAirport *airport, int *trueFalse);
	int	APISimplifyObject(SFlyObjectRef *object);
	int	APILoadModel(SFlyObjectRef *object, const char *modelName, const unsigned int modelID);
	int	APILoadNightModel(SFlyObjectRef *object, const char *modelName, const unsigned int modelID);
	int	APISelectModel(SFlyObjectRef *object, const unsigned int modelID);
	int	APISelectNightModel(SFlyObjectRef *object, const unsigned int modelID);
	int	APIRayWorldIntersection(SPosition *p1, SPosition *p2, SFlyObjectRef *avoid, SCollision *collision, int collideFlags);
	int	APIGetEngineState(SFlyObjectRef *object, int engineNum);
	int	APIAllEnginesRunning(SFlyObjectRef *object);
	void	APITurnOffAllAutopilots(SFlyObjectRef *object);
	void	APILockoutUserControl(SFlyObjectRef *object);

	SFlyObjectRef	APICreateVehicle(char *infoFile, char *pathFile, SPosition *pos);
	void	APIDisposeVehicle(SFlyObjectRef *object);
	int	APISetVehiclePath(SFlyObjectRef *object, char *pathFile);
	void	APIAddGeneratedScenery(SPosition *pos, SVector *orient, char *daySmfFile, char *nightSmfFile, int autoHide);
	
	//
	//	gestalt functions (object traits)
	//

	int	APIGestalt(SFlyObjectRef *object, int gestaltFlag);
	void	APIGestaltSet(SFlyObjectRef *object, int gestaltFlag);
	void	APIGestaltClear(SFlyObjectRef *object, int gestaltFlag);

	//
	//	position and reference utilities
	//

	int	APIPositionFromNamedObject(const char *objectName, SPosition *pos);
	int	APIDistanceToNamedObject(const char *objectName, SPosition *pos, float *distance);
	SPosition	APIMakePosition(const char *latString, const char *lonString, const float alt);
	int	APIPointInRegion(SPosition *pos, SPositionList *region);
	int	APIPointInCircle(SPosition *pos, SPosition *center, float radiusInFeet);
	int	APIPointInAirport(SPosition *pos, SAirport *airport);
	void	APICalculateRunwayRegions(SRunway *runway, SPositionList **upwind, SPositionList **crosswind, SPositionList **downwind, SPositionList **base, SPositionList **final);
	void	APIDisposePositionList(SPositionList *region);

	//
	//	ground functions
	//

	float	APIGetGroundHeight(SPosition *pos);
	SVector	APIGetGroundNormal(SPosition *pos);
	int	APIGetGroundType(SPosition *pos);
	int	APIDoesSceneryExist(SPosition *ll, SPosition *ur);

	//
	//	checklist functions
	//

	int	APIIsChecklistRunning(void);
	void	APIRunChecklist(const char *checklist);

	//
	//	weather functions
	//

	void	APILoadMetar(char *filePath);
	void	APIGetWeatherInfo(SWeatherInfo *info);
	void	APIGetWindsAloft(SWindsAloft winds[WEATHER_WINDS_ALOFT_MAX]);
	void	APISetWeatherInfo(SWeatherInfo *info);
	void	APISetWindsAloft(SWindsAloft winds[WEATHER_WINDS_ALOFT_MAX]);
	void	APIGetReportedWeather(SPosition *pos, SWeatherInfo *info);
	void	APISetFogColor(int r, int g, int b);
	void	APISetBrokenCloudTextures(char textures[3][64]);
	void	APISetOvercastCloudTexture(char texture[64]);
	void	APISetCloudPanelTextures(char texture[3][64]);
	void	APISetLightningTextures(char texture[3][64]);
	void	APISetRainTextures(char texture[3][64]);
	void	APISetSnowTexture(char texture[64]);
	void	APISetSkyBackdrop(char texture[64]);						// obsolete
	void	APIShowSkyBackdrop(char trueFalse);						// obsolete
	void	APISetDomeRGBSaturation(float redSat, float greenSat, float blueSat);		// obsolete
	void	APIGetDomeRGBSaturation(float *redSat, float *greenSat, float *blueSat);	// obsolete
	float	APIGetAmbientLightLevel(void);
	void	APISetAmbientLightMultiplier(float multiplier);
	float	APIGetAmbientLightMultiplier(void);
	int	APIVisibleMoisture(int *level);
	void	APIGetLightScalars(float *dawn, float *day, float *dusk, float *night);
	void	APISetLightScalars(float dawn, float day, float dusk, float night);

	//
	//	sky functions
	//

	void	APILoadSkyModels(const char *skyIniFilename);
	void	APISelectSkyModel(const char *skyEntryName);
	int	APIRandomlySwapSkies(int flag);	// flag=0 no swapping, flag=1 randomly swaps at dawn; returns previous setting

	//
	//	camera functions
	//

	void	APISetCamera(unsigned int camera);
	unsigned int	APIGetCamera(void);
	int	APIHasCamera(SFlyObjectRef *ref, unsigned int camera);
	void	APIAddCamera(SFlyObjectRef *ref, unsigned int camera, SStream *stream);
	void	APISetPanel(const int panelID);
	void	APISetToRightPanel(void);
	void	APISetToLeftPanel(void);
	void	APISetToUpperPanel(void);
	void	APISetToLowerPanel(void);
	void	APIHomePanel(void);
	void	APIShowPanel(void);
	void	APIHidePanel(void);
	int	APIIsInteriorCamera(void);
	int	APIIsExteriorCamera(void);
	void	APIPanCamera(float pitchInRads, float headingInRads, int autoCenter);
	void	APIPositionCamera(float pitchInRads, float headingInRads, float distanceInFeet);
	void	APIZoomCamera(float zoomRatio);
	void	APIUseVFRPanels(void);
	void	APIUseIFRPanels(void);
	void	APIGetCameraPosition(SPosition *pos, SVector *dir, SVector *vel);

	//
	//	window functions
	//

	SDLLObject*	APICreateWindow(int type, int x, int y, int xSize, int ySize, int windowFlags);
	void	APIWindowSetAutoSize(SDLLObject *object, int onOffFlag);
	void	APIWindowSetMoveable(SDLLObject *object, int onOffFlag);
	void	APIWindowSetResizeable(SDLLObject *object, int onOffFlag);
	void	APIWindowSetDrawTitleBar(SDLLObject *object, int onOffFlag);
	void	APIWindowSetDrawCloseBox(SDLLObject *object, int onOffFlag);
	void	APIWindowSetDrawSizeBox(SDLLObject *object, int onOffFlag);
	void	APIWindowSetDrawBorder(SDLLObject *object, int onOffFlag);
	void	APIWindowSetMinSize(SDLLObject *object, int minXSize, int minYSize);
	void	APIWindowSetMaxSize(SDLLObject *object, int maxXSize, int maxYSize);
	void	APIWindowSetPosition(SDLLObject *object, int x, int y);
	void	APIWindowSetSize(SDLLObject *object, int xSize, int ySize);
	void	APICloseWindow(SDLLObject *object);
	void	APICloseAllWindows(void);

	void	APICreateWindowByType(int type, int x, int y, int xSize, int ySize);
	void	APIMoveWindowByType(int type, int x, int y);
	void	APISizeWindowByType(int type, int x, int y);
	void	APICloseWindowByType(int type);
	int	APIWindowCount(int type);

	void	APICreateAirportWindow(SAirport* airport, int x, int y);
	void	APICloseAirportWindow(SAirport* airport);
	void	APICreateNavaidWindow(SNavaid* navaid, int x, int y);
	void	APICloseNavaidWindow(SNavaid* navaid);

	void	APIVectorMapDrawAirports(const int state);
	void	APIVectorMapDrawVOR(const int state);
	void	APIVectorMapDrawNDB(const int state);
	void	APIVectorMapDrawAirportName(const int state);
	void	APIVectorMapDrawAirportID(const int state);
	void	APIVectorMapDrawVORName(const int state);
	void	APIVectorMapDrawVORID(const int state);
	void	APIVectorMapDrawVORFrequency(const int state);
	void	APIVectorMapDrawNDBName(const int state);
	void	APIVectorMapDrawNDBID(const int state);
	void	APIVectorMapDrawLabels(const int state);
	void	APIVectorMapDrawCompassPlate(const int state);
	void	APIVectorMapDrawBackground(const int state);
	void	APIVectorMapZoom(const float zoomPercent);

	//
	//	damage functions
	//

	int	APIUserNeedsRepair(void);
	void	APIRepairUser(void);
	float	APIReportAllDamage(void);
	float	APIReportRotorDamage(void);
	float	APIReportPropellerDamage(void);
	float	APIReportEngineDamage(void);
	float	APIReportWingDamage(void);
	float	APIReportWheelDamage(void);

	//
	//	date and time functions
	//

	SDate	APIGetDate(void);
	void	APISetDate(SDate date);
	STime	APIGetTime(void);
	void	APISetTime(STime time);
	ETimeOfDay	APIGetTimeOfDay(void);
	void	APIAdvanceTime(STime *time, float secs);
	float	APIGetDeltaTime(void);
	unsigned int	APIGetSystemTimer(void);

	//
	//	movie functions
	//

	SMovie*	APIOpenMovie(const char *movFile);
	void	APIDrawMovie(SMovie *movie, int screenX, int screenY);
	void	APIPlayMovie(SMovie *movie);
	void	APIStopMovie(SMovie *movie);
	void	APIIdleMovie(SMovie *movie);
	int	APIMoviePlaying(SMovie *movie);
	void	APICloseMovie(SMovie *movie);
	void	APIPlayMovieInWindow(const char *movFile, int screenX, int screenY, int autoClose);
	void	APIBeginRecordMovie(char *movFile, const int xSize, const int ySize, const int fps);
	void	APIIdleRecordMovie(void);
	void	APIEndRecordMovie(void);
	int	APIIsRecordingMovie(void);
	int	APIMovieFramesPerSecond(void);

	//
	//	general info functions
	//

	int	APIIsSimulationRunning(void);
	int	APIIsSlewing(void);
	int	APIIsPaused(void);
	void	APISetSlewing(const int slewFlag);
	void	APISetPaused(const int pauseFlag);
	void	APIGetMainWindowCoords(int *x1, int *y1, int *x2, int *y2);
	void	APIGetScreenResolution(int *xSize, int *ySize);
	void	APIGetScreenDirect(
			unsigned char **rowTable[], 
			int *xSize, int *ySize, int *bpp, 
			int *redScale, int *redShift,
			int *greenScale, int *greenShift,
			int *blueScale, int *blueShift,
			int *byteOrder);

	//
	//	feedback functions
	//

	void	APIDrawNoticeToUser(const char *text, const int timeoutInSecs);
	void	APIMessageBox(const char *message);

	//
	//	utility functions
	//

	float	APIGreatCircleDistance(SPosition *from, SPosition *to);
	SPosition	APIGreatCirclePosition(SPosition *from, SVector *polar);
	SVector	APIGreatCirclePolar(SPosition *from, SPosition *to);
	float	APIWrap180(float value);
	float	APIWrap360(float value);
	float	APIWrapPi(float value);
	float	APIWrapTwoPi(float value);
	SPosition	APIAddVector(SPosition *from, SVector *v);
	SPosition	APISubtractVector(SPosition *from, SVector *v);
	SVector	APISubtractPosition(SPosition *from, SPosition *to);

	//
	//	ini functions
	//

	void	APIGetIniVar(const char *section, const char *varname, int *value);
	void	APIGetIniString(const char *section, const char *varname, char *strvar, int maxLength);
	void	APISetIniVar(const char *section, const char *varname, int *value);
	void	APISetIniString(const char *section, const char *varname, char *strvar);
	void	APISetDynamicScenery(char trueOrFalse);
	void	APISetNumDynamicScenery(int numDynamicObjects);
	void	APISetShadows(int shadowsValue);	// 0=off, 1=aircraft, 2=all
	void	APISetFiniteFuel(char trueOrFalse);
	void	APISetFiniteBattery(char trueOrFalse);
	void	APISetPFactor(char trueOrFalse);
	void	APISetPropTorque(char trueOrFalse);
	void	APISetAutoMixture(char trueOrFalse);
	void	APISetAutoPropeller(char trueOrFalse);
	void	APISetIcePropeller(char trueOrFalse);
	void	APISetIceEngine(char trueOrFalse);
	void	APISetIceWing(char trueOrFalse);
	void	APISetIcePitot(char trueOrFalse);
	void	APISetGyroDrift(char trueOrFalse);
	void	APISetCoupleRudderAilerons(char trueOrFalse);
	void	APISetCoupleAirBrake(char trueOrFalse);
	void	APISetCockpitMouseScrolling(char trueOrFalse);
	void	APISetATCServiceNames(char trueOrFalse);
	void	APISetATCScrollingText(char trueOrFalse);
	void	APISetObjectCollisions(int collisions);	// bit 1 = ground, bit 2 = static, bit 3 = dynamic

	//
	//	UI functions
	//

	void	APICreateWindow2(unsigned long windowID, char *winFile, SDLLObject *notify);
	int	APIIsWindowOpen(unsigned long windowID);
	void	APICloseWindow2(unsigned long windowID);
	int	APIIsComponentType(unsigned long windowID, unsigned long componentID, unsigned int componentType);
	void	APIShowWindow(unsigned long windowID);
	void	APIHideWindow(unsigned long windowID);

	void	APIGainFocus(unsigned long windowID, unsigned long componentID);
	void	APILoseFocus(unsigned long windowID, unsigned long componentID);

	char*	APIGetUIButtonLabel(unsigned long windowID, unsigned long buttonID);
	void	APISetUIButtonLabel(unsigned long windowID, unsigned long buttonID, char *label);

	int	APIGetUICheckboxState(unsigned long windowID, unsigned long checkID);
	void	APISetUICheckboxState(unsigned long windowID, unsigned long checkID, int state);
	char*	APIGetUICheckboxLabel(unsigned long windowID, unsigned long checkID);
	void	APISetUICheckboxLabel(unsigned long windowID, unsigned long checkID, char *label);

	int	APIGetUIRadioState(unsigned long windowID, unsigned long radioID);
	void	APISetUIRadioState(unsigned long windowID, unsigned long radioID, int state);
	char*	APIGetUIRadioLabel(unsigned long windowID, unsigned long radioID);
	void	APISetUIRadioLabel(unsigned long windowID, unsigned long radioID, char *label);

	void	APIAddUIMenu(unsigned long windowID, unsigned long menubarID, unsigned long menuID, const char *label);
	void	APIAddUIMenuItem(unsigned long windowID, unsigned long menubarID, unsigned long menuID, unsigned long itemID, const char *label);
	void	APIAddUIMenuSeperator(unsigned long windowID, unsigned long menubarID, unsigned long menuID);
	void	APIRemoveUIMenuItem(unsigned long windowID, unsigned long menubarID, unsigned long menuID, unsigned long itemID);
	void	APIClearUIMenuItems(unsigned long windowID, unsigned long menubarID, unsigned long menuID);
	void	APICheckUIMenuItem(unsigned long windowID, unsigned long menubarID, unsigned long menuID, unsigned long itemID, int checked);
	void	APIEnableUIMenuItem(unsigned long windowID, unsigned long menubarID, unsigned long menuID, unsigned long itemID, int enabled);

	void	APIAddUIPopupItem(unsigned long windowID, unsigned long popupID, unsigned long itemID, const char *label);
	void	APIAddUIPopupSeperator(unsigned long windowID, unsigned long popupID);
	void	APIRemoveUIPopupItem(unsigned long windowID, unsigned long popupID, unsigned long itemID);
	void	APIClearUIPopupItems(unsigned long windowID, unsigned long popupID);
	void	APISelectUIPopupItem(unsigned long windowID, unsigned long popupID, unsigned long itemID);
	unsigned long	APIGetUIPopupItemSelection(unsigned long windowID, unsigned long popupID, char *outText);
	char*	APIGetUIPopupLabel(unsigned long windowID, unsigned long popupID);
	void	APISetUIPopupLabel(unsigned long windowID, unsigned long popupID, char *label);

	void	APISetUIRadioGroupSelection(unsigned long windowID, unsigned long groupID, unsigned long radioID);
	unsigned long	APIGetUIRadioGroupSelection(unsigned long windowID, unsigned long groupID);

	char*	APIGetUILabelText(unsigned long windowID, unsigned long labelID);
	void	APISetUILabelText(unsigned long windowID, unsigned long labelID, char *label);

	void	APISetUIScrollbarMin(unsigned long windowID, unsigned long scrollID, float min);
	void	APISetUIScrollbarMax(unsigned long windowID, unsigned long scrollID, float max);
	void	APISetUIScrollbarValue(unsigned long windowID, unsigned long scrollID, float value);
	float	APIGetUIScrollbarMin(unsigned long windowID, unsigned long scrollID);
	float	APIGetUIScrollbarMax(unsigned long windowID, unsigned long scrollID);
	float	APIGetUIScrollbarValue(unsigned long windowID, unsigned long scrollID);
	void	APISetUIScrollbarPageSize(unsigned long windowID, unsigned long scrollID, float pageSize);
	float	APIGetUIScrollbarPageSize(unsigned long windowID, unsigned long scrollID);

	void	APISetUISliderMajorTickSpacing(unsigned long windowID, unsigned long sliderID, float spacing);
	void	APISetUISliderMinorTickSpacing(unsigned long windowID, unsigned long sliderID, float spacing);
	void	APISetUISliderSnapToTicks(unsigned long windowID, unsigned long sliderID, int snapFlag);
	void	APISetUISliderDrawMajorTicks(unsigned long windowID, unsigned long sliderID, int drawFlag);
	void	APISetUISliderDrawMinorTicks(unsigned long windowID, unsigned long sliderID, int drawFlag);
	float	APIGetUISliderMajorTickSpacing(unsigned long windowID, unsigned long sliderID);
	float	APIGetUISliderMinorTickSpacing(unsigned long windowID, unsigned long sliderID);
	int	APIGetUISliderSnapToTicks(unsigned long windowID, unsigned long sliderID);
	int	APIGetUISliderDrawMajorTicks(unsigned long windowID, unsigned long sliderID);
	int	APIGetUISliderDrawMinorTicks(unsigned long windowID, unsigned long sliderID);
	void	APISetUISliderLabel(unsigned long windowID, unsigned long sliderID, char *label);

	void	APISetUIGroupBoxLabel(unsigned long windowID, unsigned long groupID, char *label);

	void	APISetUITextMaxChars(unsigned long windowID, unsigned long textID, int maxChars);
	int	APIGetUITextMaxChars(unsigned long windowID, unsigned long textID);
	void	APIUseUITextPassword(unsigned long windowID, unsigned long textID, int passwordFlag);
	int	APIIsUITextPassword(unsigned long windowID, unsigned long textID);
	int	APIGetUITextLength(unsigned long windowID, unsigned long textID);
	void	APISetUITextText(unsigned long windowID, unsigned long textID, char *text);
	char*	APIGetUITextText(unsigned long windowID, unsigned long textID);
	void	APISetUITextSelection(unsigned long windowID, unsigned long textID, int firstChar, int lastChar);
	void	APIGetUITextSelection(unsigned long windowID, unsigned long textID, int *firstChar, int *lastChar);

	void	APISetUIProgressIndeterminate(unsigned long windowID, unsigned long progressID, int flag);
	int	APIGetUIProgressIndeterminate(unsigned long windowID, unsigned long progressID);
	void	APISetUIProgressMin(unsigned long windowID, unsigned long progressID, int min);
	int	APIGetUIProgressMin(unsigned long windowID, unsigned long progressID);
	void	APISetUIProgressMax(unsigned long windowID, unsigned long progressID, int max);
	int	APIGetUIProgressMax(unsigned long windowID, unsigned long progressID);
	void	APISetUIProgressValue(unsigned long windowID, unsigned long progressID, int value);
	int	APIGetUIProgressValue(unsigned long windowID, unsigned long progressID);

	void	APIScrollUIListToRow(unsigned long windowID, unsigned long listID, int row);
	void	APIScrollUIListToColumn(unsigned long windowID, unsigned long listID, int column);
	int	APIGetUIListRowCount(unsigned long windowID, unsigned long listID);
	int	APIGetUIListColumnCount(unsigned long windowID, unsigned long listID);
	void	APIAddUIListColumn(unsigned long windowID, unsigned long listID, int width, char *title);	// neg. width is "minimum" width
	void	APIDeleteUIListColumn(unsigned long windowID, unsigned long listID, int column);
	void	APIDeleteAllUIListColumns(unsigned long windowID, unsigned long listID);
	void	APISetUIListColumnWidth(unsigned long windowID, unsigned long listID, int column, int width);
	void	APIAddUIListRow(unsigned long windowID, unsigned long listID, int height);
	void	APIDeleteUIListRow(unsigned long windowID, unsigned long listID, int row);
	void	APIDeleteAllUIListRows(unsigned long windowID, unsigned long listID);
	int	APICountUIListSelectedRows(unsigned long windowID, unsigned long listID);
	int	APIGetUIListSelectedRow(unsigned long windowID, unsigned long listID, int index);
	void	APISelectUIListRow(unsigned long windowID, unsigned long listID, int row);
	void	APIClearUIListSelection(unsigned long windowID, unsigned long listID);
	void	APISetUIListCellText(unsigned long windowID, unsigned long listID, int row, int column, char *text);
	void	APISetUIListCellIcon(unsigned long windowID, unsigned long listID, int row, int column, char *filename);
	void	APIClearUIListCellText(unsigned long windowID, unsigned long listID, int row, int column);
	void	APIClearUIListCellIcon(unsigned long windowID, unsigned long listID, int row, int column);
	void	APIGetUIListCellText(unsigned long windowID, unsigned long listID, int row, int column, char *outtext);

	void	APIAddUIGraphTrace(unsigned long windowID, unsigned long graphID, unsigned long traceID, int traceType);
	void	APIRemoveUIGraphTrace(unsigned long windowID, unsigned long graphID, unsigned long traceID);
	void	APIRemoveAllUIGraphTraces(unsigned long windowID, unsigned long graphID);
	void	APIAddUIGraphTracePoint(unsigned long windowID, unsigned long graphID, unsigned long traceID, float x, float y);
	void	APIClearUIGraphTracePoints(unsigned long windowID, unsigned long graphID, unsigned long traceID);
	void	APISetUIGraphTraceRange(unsigned long windowID, unsigned long graphID, unsigned long traceID, float minX, float minY, float maxX, float maxY);
	void	APISetUIGraphTraceColor(unsigned long windowID, unsigned long graphID, unsigned long traceID, unsigned int color);
	void	APISetUIGraphUseGrid(unsigned long windowID, unsigned long graphID, int useGrid);
	void	APISetUIGraphGridRange(unsigned long windowID, unsigned long graphID, float minX, float minY, float maxX, float maxY);
	void	APISetUIGraphGridSpacing(unsigned long windowID, unsigned long graphID, float xStep, float yStep);
	void	APISetUIGridColor(unsigned long windowID, unsigned long graphID, unsigned int color);
	void	APISetUIGridBackColor(unsigned long windowID, unsigned long graphID, unsigned int color);
	void	APIGetUIGraphGridRange(unsigned long windowID, unsigned long graphID, float *minX, float *minY, float *maxX, float *maxY);
	void	APIGetUIGraphGridSpacing(unsigned long windowID, unsigned long graphID, float *xStep, float *yStep);

	void	APISetUIPictureImage(unsigned long windowID, unsigned long pictureID, char *pbmFile);

	//
	//	standard dialog boxes
	//

	void	APIOpenFileDialog(unsigned long windowID, char *title, char *extension, char *defaultName, char *defaultFolder, int createFolder, int allowLocalFiles, int allowPodFiles, int fileMustExist, SDLLObject *notify);
	void	APISaveFileDialog(unsigned long windowID, char *title, char *extension, char *defaultName, char *defaultFolder, int createFolder, int allowLocalFiles, int allowPodFiles, int fileMustExist, SDLLObject *notify);
	void	APIGetFilePath(unsigned long windowID, char *pathBuffer, int bufferLen);
	void	APIAlertBox(unsigned long windowID, char *title, char *message, int buttonStyle, int iconStyle, SDLLObject *notify);
	void	APIOKCancelBox(unsigned long windowID, char *message, char *okButton, char *cancelButton, SDLLObject *notify);
	void	APIDebugString(char *debug);

	//
	//	Python specific functions (internal use only)
	//
	
	int	APICountPythonMethodModules(void);
	void	APIGetPythonMethodModule(int index, char *moduleName, SPythonMethod **methods);

	//
	//	EPIC hardware support
	//

	int	APIEpicAvailable(void);
	void	APIEpicRead(unsigned char *buffer, int address, int numBytes);
	void	APIEpicWrite(unsigned char *buffer, int address, int numBytes);
	int	APIEpicSendEvent(SEpicEvent *event);
	int	APIEpicSendPH(int phNum, int data0, int data1, int data2, int data3);
	int	APIEpicSendQP(int qpNum);

	//
	//	Data Block support
	//

	int	APIOpenDataBlock(const char* blockName, const char* filename); // adds to list
	void	APICloseDataBlock(const char* blockName); // removes from list
	int	APIDataBlockSize(const char* blockName);
	int	APIReadDataBlock(const char* blockName, SDataBlockItem *array);
	int	APIWriteDataBlock(const char* blockName, SDataBlockItem *array);
	int	APIReadDataItem(const char *blockName, unsigned int label, SDataBlockItem *item);
	int	APIReadDataItemInt(const char *blockName, unsigned int label);
	float	APIReadDataItemFloat(const char *blockName, unsigned int label);
	int	APIWriteDataItem(const char *blockName, SDataBlockItem *item);
	int	APIWriteDataItemInt(const char *blockName, unsigned int label, int data);
	int	APIWriteDataItemFloat(const char *blockName, unsigned int label, float data);

}	// extern "C"


#endif	// __MAINSDK__
