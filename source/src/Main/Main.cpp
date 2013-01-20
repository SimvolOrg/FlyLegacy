/*
 * Main.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2007 Chris Wallace
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

/*! \file Main.cpp
 *  \brief Top-level application implementation including main() function
 *
 * Implementation of main() and WinMain() entry points for the application,
 *   application initialization and shutdown functions, and the
 *   GTFO method of immediate application abort due to fatal errors.
 */
//====================================================================================================
// Include Visual Leak Detector header
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
//===================================================================================================
#include "../Include/Globals.h"
#include "../Include/Situation.h"
#include "../Include/KeyMap.h"
#include "../Include/Ui.h"
#include "../Include/FuiParts.h"
#include "../Include/Database.h"
#include "../Include/TerrainCache.h"
#include "../Include/TerrainElevation.h"
#include "../Include/ScenerySet.h"
#include "../Include/Cloud.h"
#include "../Include/LightSystem.h"
#include "../Include/Utility.h"
#include "../Include/LogFile.h"
#include "../Include/Pod.h"
#include "../Include/MagneticModel.h"
#include "../Include/Weather.h"
#include "../Include/Atmosphere.h"
#include "../Include/Joysticks.h"
#include "../Include/Fui.h"
#include "../Include/AudioManager.h"
#include "../Include/CursorManager.h"
#include "../Include/Sky.h"
#include "../Include/GLSL.h"
#include "../Include/Subsystems.h"
#include "../Include/Export.h"
#include "../Include/Import.h"
#include "../Include/SqlMGR.h"
#include "../Include/BlackBox.h"
#include "../Include/TerrainTexture.h"
#include "../Include/Triangulator.h"
#include "../Include/OSMobjects.h"
#include "../Include/FuiOption.h"
#include "../Include/Collisions.h"
#include "../Include/Cameras.h"
#include "../Include/External.h"
#include "../Include/TestBed.h"
#include "../Plugin/Plugin.h"
//----Windows particular -------------------------
#include <math.h>
#include <pthread.h>
#include <direct.h>
using namespace std;
//=============================================================================================

//=============================================================================================
//  GLOBAL DATABANK
//  The following data are used at global level for consistency.
//  All module using those name refer to those table
//=============================================================================================
// General-purpose global settings
SGlobals *globals = NULL;
//=====================================================================
//	ASCII Valid text characters
//	0x00  => Invalid
//	0x01  => Space
//	0x02	=> Letters
//	0x04  => Number
//	0x08	=> Sign
//	0x10	=> Punctuation
//	0x20	=> Others
//=====================================================================
//=====================================================================
char asciiVAL[]	=	{
	//--00--01--02--03--04--05--06--07--08--09--0A--0B--0C--0D--0E--0F
			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	//--10--11--12--13--14--15--16--17--18--19--1A--1B--1C--1D--1E--1F
			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	//--SP--!---"---#---$---%---&---'---(---)---*---+---,-------.---/
			KSP,KPN,KKK,KKK,KKK,KKK,KKK,KKK,KKK,KKK,KKK,KSG,KPN,KSG,KDT,KKK,
	//--0---1---2---3---4---5---6---7---8---9---:---;---<---=--->---?--
			KNB,KNB,KNB,KNB,KNB,KNB,KNB,KNB,KNB,KNB,KPN,KPN,KKK,KKK,KKK,KPN,
	//--@---A---B---C---D---E---F---G---H---I---J---K---L---M---N---O--
			KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,
	//--P---Q---R---S---T---U---V---W---X---Y---Z---[---\---]---^---_---
			KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KKK,KKK,KKK,KKK,KLT,
	//--`---a---b---c---d---e---f---g---h---i---j---k---l---m---n---o---
			KPN,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,
	//--p---q---r---s---t---u---v---w---x---y---z---{---|---}---~--DEL-
			KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KLT,KKK,KKK,KKK,KKK,KKK,
};
//=====================================================================
//-----------Time zone designation --------------------------------
STIME_ZONE tmzTAB[]  =   {
  {"UTC",  0},              // 0 UTC
  {"GST", -3},              // 1 Greenland Standard Time
  {"GDT", -2},              // 2 Greenland Daylight Time
  {"ATS", -4},              // 3 ATlantic Standard Time
  {"ATD", -3},              // 4 Atlantic Daylight Time
  {"EST", -5},              // 5 Eastern Standard Time
  {"EDT", -4},              // 6 Eastern Daylight Time
  {"CST", -6},              // 7 Central Standard Time
  {"CDT", -5},              // 8 Central Daylight Time
  {"MST", -7},              // 9 Mountain Standard Time
  {"MDT", -6},              // 10 Mountain Daylight Time
  {"PST", -8},              // 11 Pacific Standard Time
  {"PDT", -7},              // 12 Pacific Daylight Time
  {"AKS", -9},              // 13 Alaska Standard Time
  {"AKD", -8},              // 14 Alaska Daylight Time
  {"HAS", -10},             // 15 Hawaii Standard Time
  {"HAD", - 9},             // 16 Hawaii Daylight Time
  {"SST", -11},             // 17 Samoa Standard Time
  {"SDT", -10},             // 18 Samoa Daylight Time
  {"LCL",  0},              // 19 Local Time user defined
};
//-----------Communication type table -----------------------------
char *comTAB[COMDIM] = {  "???",
                          " ATIS",
                          " TOWER",
                          " CLEARANCE",
                          " GROUND",
                          " APPROACH",
                          " DEPARTURE",
                          " SERVICE",
                          " CENTER",
                          " CTAF",
                          " MULTICOM",
                          " EMERGENCY",
};
//-----------Communication abreviate table --------Index-------------
char *cmaTAB[COMDIM] = {  "????",                 // 0
                          "ATIS",                 // 1
                          "TWR ",                 // 2
                          "CLR ",                 // 3
                          "GRND",                 // 4
                          "APR ",                 // 5
                          "DEP ",                 // 6
                          "AFIS",                 // 7
                          "CTR ",                 // 8
                          "CTAF",                 // 9
                          "MCOM",                 // 10
                          "SOS ",                 // 11
};
//---------Standard alphabet --------------------------------------
char *abcTAB[27]    = {   " ???",
                          " ALPHA",   " BRAVO",     " CHARLIE",   " DELTA",
                          " ECHO",    " FOX TROT",  " GOLF",      " HOTEL",
                          " INDIA",   " JULIETT",   " KILO",      " LIMA",
                          " MIKE",    " NOVEMBER",  " OSCAR",     " PAPA",
                          " QUEBEC",  " ROMEO",     " SIERRA",    " TANGO",
                          " UNIFORM", " VICTOR",    " WHISKY",    " X-RAY",
                          " YANKEE",  " ZULU",
};
//---------VOR NAME -----------------------------------------------
char  *vorTAB[] =     {   "???",      "VOR",        "TACAN",      "VORTAC",
                          "VOR-DME",  "NDB",        "NDB-DME",
};
//--------WAYPOINT NAME -------------------------------------------
char  *wptTAB[] =     {   "TILE",     "AIRPORT",    "VOR",        "NDB",
                          "ILS",      "COM",        "RUNWAY",     "WAYPOINT",
};
//--------GAZ TABLE ------------------------------------------------
char  *gasTAB[] =     {   "",
                          "80",       "100",        "JET 100LL",  "115",
                          "JET-A",    "JET-A1",     "JET-A1PLUS", "JET-B",
                          "JET-BPLUS",
};
//--------Month table ----------------------------------------------
char  *monTAB[] =     {   "",
                          "JANUARY",  "FEBRUARY",   "MARCH",      "APRIL",
                          "MAY",      "JUNE",       "JULY",       "AUGUST",
                          "SEPTEMBER", "OCTOBER",   "NOVEMBER",   "DECEMBER",
};
//-------Airport Ownership -----------------------------------------
char  *ownTAB[]  =   {    "UNKNOWN",
                          "PUBLIC",
                          "PRIVATE",
                          "AIRFORCE",
                          "NAVY",
                          "ARMY",
};
//-------Runway ground type ----------------------------------------
char *rwgTAB[]  =   {     "???",
                          "CEM",      "ASF",    "TRF",  "DRT",
                          "GRV",      "MAT",    "SND",  "WOD",
                          "WAT",      "MAT",    "SNW",  "ICE",
                          "GRV",      "TRD",
};
//=====================================================================================
//  TERRAIN TYPE
//=====================================================================================
TC_TERRA terraTAB[] = {
  {0,  0, "USER TEXTURE"},
  {12, 0, "WATER OCEAN"},
  {13, 0, "ICE CRACKED GLACIERS"},
  {50, 0, "SNOW"},
  {59, 0, "HARD SNOW"},
  {60, 0, "MOUNTAIN FOREST CALIFORNIA"},
  {61, 1, "TOWNS MIDEAST"},
  {101, 1, "CITY USA"},
  {102, 0, "GRASS SCRUB LIGHT (1)"},
  {104, 0, "FOREST MIXED SCRUB"},
  {105, 0, "FOREST MIXED"},
  {106, 0, "MOUNTAIN FOREST GREEN"},
  {107, 0, "GRASS SCRUB LIGHT (2)"},
  {108, 0, "DESERT BARREN"},
  {109, 0, "TUNDRA UPLAND"},
  {110, 0, "GRASS SCRUB LIGHT (3)"},
  {111, 0, "DESERT SCRUB LIGHT"},
  {112, 0, "SNOW HARD PACKED"},
  {113, 1, "EURO CITY ENGLAND HEAVY"},
  {116, 0, "SHRUB EVERGREEN"},
  {117, 0, "SHRUB DECIDUOUS"},
  {118, 1, "CITY HEAVY ASIAN"},
  {119, 0, "FOREST EVERGREEN W DARK FIELDS"},
  {120, 0, "RAINFOREST TROPICAL MIXED"},
  {121, 0, "CONIFER BOREAL FOREST"},
  {122, 0, "COOL CONIFER FOREST"},
  {123, 0, "EURO FOREST TOWNS"},
  {124, 0, "FOREST RIDGES MIXED"},
  {125, 0, "EURO FOREST DARK TOWNS"},
  {126, 1, "SUBURB USA BROWN"},
  {127, 0, "CONIFER FOREST"},
  {128, 0, "FOREST YELLOWED AFRICAN"},
  {129, 0, "SEASONAL RAINFOREST"},
  {130, 0, "COOL CROPS"},
  {131, 0, "CROPS TOWN"},
  {132, 0, "DRY TROPICAL WOODS"},
  {133, 0, "RAIN FOREST TROPICAL"},
  {134, 0, "RAIN FOREST DEGRADED"},
  {135, 0, "FARM USA GREEN"},
  {136, 0, "RICE PADDIES"},
  {137, 0, "HOT CROPS"},
  {138, 0, "FARM USA MIXED"},
  {139, 0, "EURO FARM FULL MIXED"},
  {140, 0, "COOL GRASSES W SHRUBS"},
  {141, 0, "HOT GRASSES W SHRUBS"},
  {142, 0, "COLD GRASSLAND"},
  {143, 0, "SAVANNA WOODS"},
  {144, 0, "SWAMP BOG"},
  {145, 0, "MARSH WETLAND"},
  {146, 0, "MEDITERRANEAN SCRUB"},
  {147, 0, "DRY WOODY SCRUB"},
  {148, 0, "DRY EVERGREEN WOODS"},
  {149, 1, "SUBURB USA"},
  {150, 0, "DESERT SAND DUNES"},
  {151, 0, "SEMI DESERT SHRUB"},
  {152, 0, "SEMI DESERT SAGE"},
  {153, 0, "MOUNTAIN ROCKY TUNDRA SNOW"},
  {154, 0, "MIXED RAINFOREST"},
  {155, 0, "COOL FIELD SANDWOODS"},
  {156, 0, "FOREST AND FIELD"},
  {157, 0, "COOL FOREST AND FIELD"},
  {158, 0, "FIELDS AND WOODY SAVANNA"},
  {159, 0, "THORN SCRUB"},
  {160, 0, "SMALL LEAF MIXED FOREST"},
  {161, 0, "MIXED BOREAL FOREST"},
  {162, 0, "NARROW CONIFERS"},
  {163, 0, "WOODED TUNDRA"},
  {164, 0, "HEATH SCRUB"},
  {165, 0, "EURO FULL GREEN (3)"},
  {166, 1, "AUSTRALIAN CITY"},
  {167, 1, "CITY HEAVY BRAZIL"},
  {168, 1, "CITY HEAVY USA"},
  {169, 0, "POLAR DESERT"},
  {170, 1, "EUROTOWNS"},
  {171, 1, "CITY MIDDLE EASTERN HEAVY"},
  {172, 0, "TUNDRA BOG"},
  {176, 0, "EURO FARM FULL MIXED (2)"},
  {177, 1, "TOWNS ASIAN"},
  {178, 0, "ICE CRACKED SNOW"},
  {179, 0, "EURO FARM FOREST"},
  {180, 0, "FARM USA BROWN"},
  {181, 0, "FARM MIDDLE EASTERN W FOREST"},
  {182, 0, "DESERT BRUSH REDDISH"},
  {183, 0, "FARM MIDDLE EASTERN"},
  {184, 0, "EURO FARM FULL MIXED (3)"},
  {185, 0, "EURO FARM FULL GREEN (2)"},
  {186, 0, "MOUNTAIN ROCKY SNOW"},
  {187, 0, "MOUNTAIN FOREST LIGHT"},
  {188, 0, "GRASS MEADOW"},
  {189, 0, "MOIST EUCALYPTUS"},
  {190, 0, "RAINFOREST HAWAIIAN"},
  {191, 0, "WOODY SAVANNA"},
  {192, 0, "BROAD LEAF CROPS"},
  {193, 0, "GRASS CROPS"},
  {194, 0, "CROPS GRASS SHRUBS"},
  {225, 0, "GRASSLAND"},
  {255, 0, "DESERT SAVANAH AFRICAN"},
  {-1,0,""},
};
//================================================================================
//    Dammage messages
//================================================================================
DAMAGE_MSG damMSG[] = {
  {},                                     // 0 Not a mesage
  {1,0,0,"Gear %s shocked"},              // 1 Gear shocked
  {2,0,'crgr',"Gear %s destroyed"},       // 2 Gear destroyed
  {3,0,'crby',"STRUCTURAL DAMAGE"},       // 3 Body destroyed
};
//==========================================================================================
//  OSM object type
//==========================================================================================
struct OSM_TP {
	char *type;										// Type
	char  Dir;										// Directory
	char  Use;										// Use it
	char	nite;										// May have nite texture
	char	rfu2;										// Reserved
};
//==========================================================================================
//  List of replacement directory per object type
//==========================================================================================
char *directoryTAB [] = {
	"ART",														// 0 => ART 
	"OpenStreet/Textures",						// 1 => User OSM session
	"OpenStreet/Models/Buildings",		// 2 => OSM Building
	"OpenStreet/Models/Trees",				// 3 => OSM trees
	"OpenStreet/Models/Lights",				// 4 => OSM Lights
	"OpenStreet/Databases",						// 5 => OSM Databases
	};
//==========================================================================================
//	List of OSM objects supported and some properties
//==========================================================================================
OSM_TP osmTYPE[OSM_MAX+1] = {
	{"NONE",				FOLDER_ART,				1,0},					// 0 Not a OSM object
	{"OSM_BUILDING",FOLDER_OSM_OBJS,	1,1},					// 1  OSM_BUILDING		(1)
	{"OSM_CHURCH",	FOLDER_OSM_OBJS,	1,0},					// 2  OSM_CHURCH			(2)
	{"OSM_POLICE",	FOLDER_OSM_OBJS,	1,1},					// 3  OSM_POLICE			(3)
	{"OSM_FIRE_STA",FOLDER_OSM_OBJS,	1,1},					// 4  OSM_FIRE_STA		(4)
	{"OSM_TOWNHALL",FOLDER_OSM_OBJS,	1,1},					// 5  OSM_TOWNHALL		(5)
	{"OSM_SCHOOL",	FOLDER_OSM_OBJS,	1,1},					// 6  OSM_SCHOOL			(6)
	{"OSM_COLLEGE",	FOLDER_OSM_OBJS,	1,1},					// 7  OSM_COLLEGE			(7)
	{"OSM_HOSPITAL",FOLDER_OSM_OBJS,	1,1},					// 8  OSM_HOSPITAL		(8)
	{"OSM_HOTEL",		FOLDER_OSM_OBJS,	1,1},					// 9  OSM_HOTEL				(9)
	//-----------------------------------------------------------------------
	{"OSM_CHATODO",	FOLDER_OSM_OBJS,	1,0},					// 10 OSM_CHATODO			(10)
	{"OSM_PHARES",	FOLDER_OSM_OBJS,	1,0},					// 11 OSM_PHARES		  (11)
	{"OSM_PSTREET", FOLDER_OSM_TREE,	1,0},					// 12 OSM_PSTREET			(12)
	{"OSM_FORTIF",	FOLDER_OSM_TEXT,	1,0},					// 13 OSM_FORTIFS     (13)
	{"OSM_DOCK",		FOLDER_OSM_OBJS,	1,0},					// 14 OSM_DOCK				(14)
	{"OSM_FOREST",	FOLDER_OSM_TREE,	1,0},					// 15 OSM_TREE				(15)
	{"OSM_LIGHT",		FOLDER_OSM_LITE,	1,0},					// 16 OSM_LIGHT				(16)
	{"OSM_RPOINT",	FOLDER_OSM_OBJS,	1,0},					// 17	OSM_RPOINT			(17) 
	{"OSM_GARDEN",	FOLDER_OSM_TEXT,	1,0},					// 18 OSM_GARDEN			(18)
	{"OSM_SPORT",		FOLDER_OSM_TEXT,	1,0},					// 19 OSM_SPORT 		  (19)
	//-----------------------------------------------------------------------
	{"OSM_PARKING",	FOLDER_OSM_TEXT,	1,0},					// 20 OSM_PARKING 		(20)
	{"OSM_HEDGE",		FOLDER_OSM_TREE,	1,0},					// 21 OSM_EDGE				(21)
	{"OSM_FARM",		FOLDER_OSM_TREE,	1,0},					// 22 OSM_FARM				(22)
	{0},

};
//================================================================================
//	Color Table
//================================================================================
float colorTABLE[] = {
	0,0,0,0,																	// 0 Black transparent
	0,0,0,1,																	// 1 Black opaque
	1,1,1,1,																	// 2 White
	1,0,0,1,																	// 3 Red color
	0,1,0,1,																	// 4 pure Green color
	0,0,1,0,																	// 5 pure Blue color
	0.11f,0.84f,0.81f,1,											// 6 Light blue
	1,1,0,1,																	// 7 Yellow
	float(0.84),float(0.35),0,1,							// 8 Amber
};
//================================================================================
//	yes no table
//================================================================================
char *yesno[] = {"not","is"};
//================================================================================
//	globals activity frame count
//================================================================================
int	globalsINIT = 200;
//================================================================================
//  Lookup the ICAO Spelling Alphabet word (ALPHA, BRAVO, etc.) for the supplied character.
//
//  \param  car
//    Character for which to get ICAO Spelling Alphabet word; must be in range a-z or A-Z
//  \return 
//    String containing ICAO Spelling Alphabet word for the character
//
//================================================================================
char *GetStandardAlphabet(char car)
{ car |= ' ';                   // set lower case
  car -= 'A';                   // index
  return (car > 26)?(abcTAB[0]):(abcTAB[car]);
}

//================================================================================
//	Get Texture directory
//================================================================================
char GetOSMTextureDirectory(U_INT t)
{	if (t >= OSM_MAX)		return 0;
	return osmTYPE[t].Dir;
}
//================================================================================
//	Get OSM Parameters
//================================================================================
char *GetOSMType(U_INT t)
{	if (t >= OSM_MAX)		return 0;
	return  osmTYPE[t].type;
}
//================================================================================
//	Get OSM Parameters
//================================================================================
char GetOSMUse(U_INT t)
{	if (t >= OSM_MAX)		return 0;
	return  osmTYPE[t].Use;
}
//================================================================================
//	Swap OSM use
//================================================================================
char SwapOSMuse(U_INT t)
{	if (t >= OSM_MAX)		return 0;
	osmTYPE[t].Use ^= 1;
	return osmTYPE[t].Use;
}
//================================================================================
//	Inhibit OSM
//================================================================================
void InhibitOSM(char *type)
{	OSM_TP *osm = osmTYPE;
	while (osm->type)
	{	if (0 != strcmp(osm->type,type)) {osm++; continue;}
		osm->Use = 0;
		return;
	}
	return;
}
//================================================================================
//	Init  OSM exclude
//================================================================================
void InitOSMusage()
{	OSM_TP *ost = osmTYPE;
	while (ost->type)
	{	if (HasIniKey("OSMexclude",ost->type)) ost->Use = 0;
		ost++;
	}
	return;
}
//================================================================================
//	Check if OSM allowed
//================================================================================
bool OSMnotUsed(char type)
{	if (type >= OSM_MAX)	return true;
	return (osmTYPE[type].Use == 0);
}
//================================================================================
//	Check if OSM allowed
//================================================================================
void CheckPerformances(float fps)
{	int lim = 0;
	GetIniVar("Performances", "OSMAutoRegulation",&lim);
	if (lim == 0)			return;
	if (fps > lim)		return;
	//--- Inihibit forest ---------------------------------
	osmTYPE[OSM_FOREST].Use = 0;
	//globals->tcm->FlushOSM();
	return;
}
//=============================================================================================
#ifdef _WIN32
//
// Array for storage of gamma ramp data
//
static WORD savedGammaRamp[3][0x100];
static HDC  hdc = 0;
#endif // _WIN32
//=========================================================================================
// Trace object size
//	Frequently allocated object may be put here
//	We may try to identify them by size when memory leak occurs
//=========================================================================================
void TraceObjectSize()
{	TRACE("----POD---------------------------------");
	TRACE("%30s size = %05d","PFSPOD",						sizeof(PFSPOD));
	TRACE("%30s size = %05d","PFSPODFILE",				sizeof(PFSPODFILE));
	TRACE("---NAV DATABASE ------------------------");
	TRACE("%30s size = %05d","CTileCache",				sizeof(CTileCache));
	TRACE("---TEXTURES ----------------------------");
	TRACE("%30s size = %05d","CSharedTxnTex",			sizeof(CSharedTxnTex));
	TRACE("%30s size = %05d","CShared3DTex",			sizeof(CShared3DTex));
	TRACE("%30s size = %05d","CTextureDef",				sizeof(CTextureDef));
	TRACE("---TERRAIN -----------------------------");
	TRACE("%30s size = %05d","CSuperTile",				sizeof(CSuperTile));
	TRACE("%30s size = %05d","CVertex",				    sizeof(CVertex));
	TRACE("%30s size = %05d","CmQUAD",				    sizeof(CmQUAD));
	TRACE("%30s size = %05d","C_QGT",							sizeof(C_QGT));
	TRACE("--- COAST DATA -------------------------");
	TRACE("%30s size = %05d","C_SEA",							sizeof(C_SEA));
	TRACE("%30s size = %05d","C_CDT",							sizeof(C_CDT));
	TRACE("--- AIRPORTS ---------------------------");
	TRACE("%30s size = %05d","CTarmac",						sizeof(CTarmac));
	TRACE("%30s size = %05d","CAptObject",				sizeof(CAptObject));
	TRACE("%30s size = %05d","CPaveRWY",					sizeof(CPaveRWY));
	TRACE("--- LIGHTS -----------------------------");
	TRACE("%30s size = %05d","CDualLITE",					sizeof(CDualLITE));
	TRACE("%30s size = %05d","CFlshLITE",					sizeof(CFlshLITE));
	TRACE("%30s size = %05d","CStrobeLITE",				sizeof(CStrobeLITE));
	TRACE("%30s size = %05d","CFlashBarLITE",			sizeof(CFlashBarLITE));
	TRACE("%30s size = %05d","CPapiLITE",					sizeof(CPapiLITE));
	TRACE("%30s size = %05d","CRLP",							sizeof(CRLP));
	TRACE("--- CITY EDITOR ------------------------");
	TRACE("%30s size = %05d","OSM_Object",				sizeof(OSM_Object));
	TRACE("%30s size = %05d","D2_FACE",					  sizeof(D2_FACE));
	TRACE("%30s size = %05d","D2_FLOOR",					sizeof(D2_FLOOR));
	TRACE("%30s size = %05d","D2_Group",					sizeof(D2_Group));
	TRACE("%30s size = %05d","D2_POINT",					sizeof(D2_POINT));
	TRACE("%30s size = %05d","D2_Session",				sizeof(D2_Session));
	TRACE("%30s size = %05d","D2_Style",					sizeof(D2_Style));
	TRACE("%30s size = %05d","D2_TParam",					sizeof(D2_TParam));
	TRACE("%30s size = %05d","D2_Session",				sizeof(D2_Session));
	TRACE("--- CITY 3D Objects---------------------");
	TRACE("%30s size = %05d","C3Dmodel",					sizeof(C3Dmodel));
	TRACE("%30s size = %05d","C3DPart",						sizeof(C3DPart));
	TRACE("%30s size = %05d","CWobj",							sizeof(CWobj));
	TRACE("--- LINES EDITOR------------------------");
	TRACE("%30s size = %05d","CCtyLine",					sizeof(CCtyLine));
	TRACE("%30s size = %05d","CStaLine",					sizeof(CStaLine));
	TRACE("%30s size = %05d","CAptLine",					sizeof(CAptLine));
	TRACE("----FUI---------------------------------");
	TRACE("%30s size = %05d","CFuiManager",				sizeof(CFuiManager));
	TRACE("%30s size = %05d","CSlot",							sizeof(CSlot));
	TRACE("%30s size = %05d","CFuiBox",						sizeof(CFuiBox));
	TRACE("%30s size = %05d","CFuiButton",				sizeof(CFuiButton));
	TRACE("%30s size = %05d","CFuiCanva",					sizeof(CFuiCanva));
	TRACE("%30s size = %05d","CFuiCheckbox",		  sizeof(CFuiCheckbox));
	TRACE("%30s size = %05d","CFuiCheckBox",			sizeof(CFuiCheckBox));
	TRACE("%30s size = %05d","CFuiCloseButton",		sizeof(CFuiCloseButton));
	TRACE("%30s size = %05d","CFuiComponent",		  sizeof(CFuiComponent));
	TRACE("%30s size = %05d","CFuiErrorMSG",		  sizeof(CFuiErrorMSG));
	TRACE("%30s size = %05d","CFuiFileBox",				sizeof(CFuiFileBox));
	TRACE("%30s size = %05d","CFuiGroupBox",		  sizeof(CFuiGroupBox));
	TRACE("%30s size = %05d","CFuiLabel",					sizeof(CFuiLabel));
	TRACE("%30s size = %05d","CFuiList",					sizeof(CFuiList));
	TRACE("%30s size = %05d","CFuiMenu",					sizeof(CFuiMenu));
	TRACE("%30s size = %05d","CFuiMenuBar",		    sizeof(CFuiMenuBar));
	TRACE("%30s size = %05d","CFuiMinimizeButton",sizeof(CFuiMinimizeButton));
	TRACE("%30s size = %05d","CFuiPage",					sizeof(CFuiPage));
	TRACE("%30s size = %05d","CFuiPicture",				sizeof(CFuiPicture));
	TRACE("%30s size = %05d","CFuiPopupMenu",			sizeof(CFuiPopupMenu));
	TRACE("%30s size = %05d","CFuiScrollBar",		  sizeof(CFuiScrollBar));
	TRACE("%30s size = %05d","CFuiScrollBTN",		  sizeof(CFuiScrollBTN));
	TRACE("%30s size = %05d","CFuiSlider",				sizeof(CFuiSlider));
	TRACE("%30s size = %05d","CFuiTextField",		  sizeof(CFuiTextField));
	TRACE("%30s size = %05d","CFuiTextPopup",		  sizeof(CFuiTextPopup));
	TRACE("%30s size = %05d","CFuiThemeWidget",		sizeof(CFuiThemeWidget));
	TRACE("%30s size = %05d","CFuiWindow",				sizeof(CFuiWindow));
	TRACE("%30s size = %05d","CFuiWindowMenuBar",	sizeof(CFuiWindowMenuBar));
	TRACE("%30s size = %05d","CFuiWindowTitle",		sizeof(CFuiWindowTitle));
	TRACE("%30s size = %05d","CFuiZoomButton",		sizeof(CFuiZoomButton));
	TRACE("----Cameras------------------------------------");
	TRACE("%30s size = %05d","CRabbitCamera",			sizeof(CRabbitCamera));
	TRACE("%30s size = %05d","CCameraCockpit",		sizeof(CCameraCockpit));
	TRACE("----Components---------------------------------");
	TRACE("%30s size = %05d","CListBox",					sizeof(CListBox));
}
//=====================================================================================
//	Helper: String dupplication 
//=====================================================================================
char *DupplicateString(char *s, int lgm)
{	int lgr = strlen(s) + 1;
	if (lgr > lgm)	lgr = lgm;
	char *d = new char[lgr];
	strncpy(d,s,lgr);
	d[lgr-1] = 0;
	return d;
}

//==================================================================
//	Read a file:  
//	Read a line and trim space, tabulation, line feed, etc
//==================================================================
char *ReadTheFile(FILE *f, char *buf)
{	bool go = true;
	while (go)
	{*buf  = 0;
		char *txt = fgets(buf,128,f);
		buf[127] = 0;
		if (0 == txt)		return buf;
		//--- Load a character and skip all unwanted --------
		char car = *txt;
		if (0 == car)		continue;
		while (car)
		{	bool sk = (car == 0x09) || (car == ' ') || (car == 0x0D) || (car == 0x0A);
			if (!sk)		break;
			else	{ txt++; car = *txt;}
		}
		if (0			== car)								continue;
		if (strncmp(txt,"//", 2) == 0)	continue;
		TrimTrailingWhitespace(txt);
		return txt;
	}
	return 0;
}
//============================================================================
//  Set Initial profile from ini file 
//	Lock, unlock individual feature 
//	NOTE: globals->aPROF is used to lock various items when some
//				windows are activated.  For example using TERRA browser prevent
//				aircraft display and other features.
//============================================================================
void InitialProfile()
{ 
	globals->noTER = 0;
	globals->noAPT = 0;
	globals->noOBJ = 0;
	globals->noOSM = 0;
	globals->noEXT = 0;
	globals->noINT = 0;
	globals->noMET = 0;
	globals->noINP = 1;
	//-----------------------------------------------------
	int nt   = 0;
	//--- Check for terrain rendition ---------------------
  GetIniVar("Sim", "NoTerrain", &nt);
  if (nt) 
		{	globals->noTER = 1;
			globals->noAPT = 1;
			globals->noOBJ = 1;
			globals->noTER = 1;
      globals->noMET = 1;
      globals->noAWT = 1;
	 }
	//--- Check for airport rendition ---------------------
	int na = 0;
  GetIniVar("Sim", "NoAirport", &na);
  if (na) globals->noAPT = 1;
	//--- Check for Object rendition ----------------------
	int no = 0;
  GetIniVar("Sim", "NoModel", &no);        // Skip 3D objects
  if (no) globals->noOBJ = 1;
	//--- Check for OSM rendition ----------------------
	int ns = 0;
  GetIniVar("Sim", "NoOSM", &ns);        // Skip OSM objects
  if (ns) globals->noOSM = 1;

	//--- Check for plane rendition ------------------------
	int np = 0;
  GetIniVar("Sim", "NoAircraft", &np);
  if (np)	globals->noEXT = 1;									// No external aircraft
	if (np)	globals->noINT = 1;									// No internal aircraft
	if (np) globals->Disp.Lock(PRIO_UPLANE);
  //----Check for meteo rendition  -----------------------
  int nm = 0;
  GetIniVar("Sim","NoMeteo",&nm);
  if (nm) globals->noMET = 1;
	//--- Reset global profile lock -------------------------
	globals->aPROF.Rep(0);
	return;
}
//============================================================================
//  Set special application profile
//============================================================================
void SpecialProfile(Tag set,U_INT p)
{ int	dta		= (set)?(+1):(-1);
	U_INT pf	= (set)?(p):(0);
	int exe   = (set)?(0):(1);
	globals->aPROF.Rep(pf);
	if (p & PROF_NO_TER)	globals->noTER += dta;
	if (p & PROF_NO_APT)	globals->noAPT += dta;
	if (p & PROF_NO_INT)	globals->noINT += dta;
	if (p & PROF_NO_EXT)	globals->noEXT += dta;
	if (p & PROF_NO_OBJ)	globals->noOBJ += dta;
	if (p & PROF_NO_OSM)	globals->noOSM += dta;
	if (p & PROF_NO_MET)	globals->noMET += dta;
	//--- Check for NO plane at all -----------
	if (p & PROF_NO_PLN)  
	{ //--- NO dispatching after ATMOSPHERE -----
		globals->Disp.ExecSET(PRIO_SLEWMGR,exe);
		globals->noINT += dta;
		globals->noEXT += dta;
		}
	return;
}
//======================================================================================
//  Read VectorMap config file
//======================================================================================
class VMAP : public CStreamObject 
{	public:
	VMAP();
	int	Read(SStream * st, Tag tag);
};
//=====================================================================================
//	Read VMAP configuration file
//=====================================================================================
VMAP::VMAP()
{	SStream s(this,"System/vmap.cfg");
}
//=====================================================================================
//	Decode configuration file
//=====================================================================================
int VMAP::Read(SStream * st, Tag tag)
{	int nf;
	U_INT p1,p3;
	float	p2;
	TC_SPOINT p,d;
  char txt[128];
	switch (tag)	{
		//--- Window position ----------------
		case 'vmps':
			ReadString(txt,128,st);
			nf = sscanf(txt,"%d , %d , %d , %d",&p.x,&p.y,&d.x,&d.y);
			if (4 != nf)	return TAG_EXIT;
			//--- Init position -----------------------------------
			globals->vmapPos	= p;
			globals->vmapScrn = d;
			return TAG_READ;
		//--- Window options ------------------
		case 'vmop':
			ReadString(txt,128,st);
			nf	= sscanf(txt,"%X , %f , %X",&p1,&p2,&p3);
			if (3 != nf)	return TAG_EXIT;
			//--- Init options -------------------
			globals->vmpOpt.Rep(p1);
			globals->vmapZoom = p2;
			globals->vmapTrns = p3;
			return TAG_READ;
		case 'endm':
			return TAG_EXIT;
	}
	return TAG_EXIT;
}
//========================================================================================
//  Generic initialization of the graphics engine.
//
//  This code must be free of any window--system specific calls unless wrapped in preprocessor macro checks.
//  Code in this function may be dependent on INI settings and globals
//========================================================================================
void InitGraphics (void)
{	// Initialize GLEW
  GLenum e = glewInit ();
  if (e != GLEW_OK)  WARNINGLOG ("GLEW Initialization error : 0x%04X", e);

  // Check the availability of various GL extensions on the host machine, and set
  //   global flags accordingly

  // Check for compressed texture formats
  globals->dxt1Supported	= 0;
  globals->dxt1aSupported = 0;
  globals->dxt3Supported	= 0;
  globals->dxt5Supported	= 0;
  GLint nFormats;
  glGetIntegerv (GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB, &nFormats);
  GLint *format = new GLint[nFormats];
  glGetIntegerv (GL_COMPRESSED_TEXTURE_FORMATS_ARB, format);
  for (int i=0; i<nFormats; i++) {
    switch (format[i]) {
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
      globals->dxt1Supported = 1;
      break;
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
      globals->dxt1aSupported = 1;
      break;
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
      globals->dxt3Supported = 1;
      break;
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      globals->dxt5Supported = 1;
      break;
    }
  }
  SAFE_DELETE_ARRAY (format);
	//-------------------------------------------------------------------------
	TRACE("Compression DTX5 %s supported",yesno[globals->dxt5Supported]);
	//-------------------------------------------------------------------------
  // Establish baseline OpenGL state that differs from default.
  // The following states are not explicitly set, as the baseline uses the
  //   defaults per OpenGL standards:
  //    GL_LIGHTING             Disabled
  //    GL_ALPHA_TEST           Disabled
  //    GL_STENCIL_TEST         Disabled
  //    GL_FOG                  Disabled
  //-------------------------------------------------------------------------
  glClearColor (0, 0, 0, 0);
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable (GL_CULL_FACE);
  glEnable (GL_DEPTH_TEST);

  DumpOpenGLDriver ("Debug/OpenGL_Driver.txt");
  DumpOpenGLState ("Debug/OpenGL_Init.txt");
}

//========================================================================================
//  Initialize fonts for use by the application.
//
//  The application uses a set of bitmap fonts availalble in the global data structure.
//========================================================================================
void InitFonts (void)
{
  //----------------------------------------------------------
  LoadMonoFont(0,&globals->fonts.ftmono8);
  LoadMonoFont(1,&globals->fonts.ftmono14);
  LoadMonoFont(2,&globals->fonts.ftmono20);
  LoadMonoFont(3,&globals->fonts.ftlucinda12);
  //-----------------------------------------------------------
  LoadVariFont(0,&globals->fonts.ftradi9);
  //-----------------------------------------------------------
  strncpy (globals->fonts.ftsmal10.fontName, "ART/FTSMAL10.RAW",63);
  LoadFont (&globals->fonts.ftsmal10);
  strncpy (globals->fonts.ftthin24.fontName, "ART/FTTHIN24.RAW",63);
  LoadFont (&globals->fonts.ftthin24);
  strncpy (globals->fonts.ftasci10.fontName, "ART/FTASCI10.RAW",63);
  LoadFont (&globals->fonts.ftasci10);
  
  /*!
   * \todo Debug fonts:
   *         FTDOS12.RAW and FTTINY7W.RAW do not comply with required
   *           palette index scheme for fonts; required bitmap patch
   *         FTNORM14.RAW
   *         FTNORM28.RAW
   */
   
}

//========================================================================================
//  Clean up fonts used by the application.
//========================================================================================
void CleanupFonts (void)
{ FreeMonoFont(&globals->fonts.ftmono8);
  FreeMonoFont(&globals->fonts.ftmono14);
  FreeMonoFont(&globals->fonts.ftmono20);
  FreeMonoFont(&globals->fonts.ftlucinda12);
  FreeVariFont(&globals->fonts.ftradi9);
  FreeFont (&globals->fonts.ftsmal10);
  FreeFont (&globals->fonts.ftthin24);
  FreeFont (&globals->fonts.ftasci10);
}
//===================================================================================
/// AspectRatio
///
/// \brief Calculate the w:h aspect ratio of a screen resolution
///
/// \param w      Screen width
/// \param h      Screen height
/// \param num    Updated with numerator of the aspect ratio
//===================================================================================
void AspectRatio (int w, int h, int &num, int &denom)
{
  // Find greatest common divisor (GCD) of w and h using Euclid's algorithm
  int gcd = GreatestCommonDivisor (w, h);

  // Calculate numerator and denominator by dividing GCD into w and h
  num = w / gcd;
  denom = h / gcd;
}
//======================================================================================
//	Create a physical ground for simulation
//	NOTE:  Ground is set at world origin as aircraft is also set at world origin, but at
//				 some altitude Z over the ground.
//======================================================================================
opal::Solid *CreateGround()
{	//----Create ground --a big square below plane body -----------
  opal::BoxShapeData boxData;
  boxData.contactGroup				= 1;
  boxData.material.hardness   = opal::real(0.95f);    // was 1.0 
  boxData.material.bounciness = opal::real(0.0f);
  boxData.material.density    = 1;
  boxData.setUserData(0,SHAPE_GROUND);
  float ground_friction       = ADJ_GRND_FRCTN;                    // was 0.25f;
  GetIniFloat ("PHYSICS", "adjustGroundFriction", &ground_friction);
  boxData.material.friction   = static_cast<opal::real> (ground_friction);
  DEBUGLOG ("ground_friction=%f", ground_friction);
	
  boxData.dimensions.set (opal::real(100000.0),
                          opal::real(100000.0),
                          opal::real(0.01)     ); // 0m up to AGL

  opal::Solid *ground = globals->opal_sim->createSolid();
  ground->setName("Ground");
  ground->addShape(boxData);
  ground->setStatic(true);
	ground->isMoving();
	return ground;
}
//======================================================================================
//  Initialize global variables that are not dependent upon files in the POD Filesystem.
//
//  Code in this function may be dependent on INI settings, but not on any contents of the
//    POD filesystem.
//======================================================================================
void InitGlobalsNoPodFilesystem (char *root)
{ //--- Globals counters -----------------------------------
	globals->NbOBJ	= 0;										// Total Objects
	globals->NbMOD	= 0;										// Total Models
	globals->NbPOL	= 0;										// Total polygons
	globals->NbCLN  = 0;										// Coast lines
	globals->NbBMP	= 0;										// Number of bitmap
  //----  Allocate NULL bitmap and logs --------------------
  globals->nBitmap		= new CBitmap;
  globals->dBug       = 0;
  globals->logDebug   = new CLogFile ("Logs/Debug.log", "w");
  globals->logWarning = new CLogFile ("Logs/Warning.log", "w");
	globals->logAeros   = new CLogFile ("Logs/AeroData.log","w");
	globals->logWings   = new CLogFile ("Logs/WingData.log","w");
	//---------------------------------------------------------
  //
  // Screen resolution/colour depth
  //

  // Determine supported video display modes
#ifdef _WIN32
  // Currently only supported for Windows via GDI function call EnumDisplaySettings
  DWORD iModeNum = 0;
  DEVMODE dm;
  BOOL rc = EnumDisplaySettings (NULL, iModeNum++, &dm);
  while (rc != 0) {
    // Add display setting to global list and get next
    if (32 == dm.dmBitsPerPel) 
		{ int num, denom;
      AspectRatio (dm.dmPelsWidth, dm.dmPelsHeight, num, denom);
    }
    rc = EnumDisplaySettings (NULL, iModeNum++, &dm);
  }
#endif //_WIN32

  // Get screen resolution/depth
  int w = 1024;
  GetIniVar ("Graphics", "gamePIXX", &w);
  int h = 768;
  GetIniVar ("Graphics", "gamePIXY", &h);
  int x = 0;
  GetIniVar ("Graphics", "gameWindowX", &x);
  int y = 0;
  GetIniVar ("Graphics", "gameWindowY", &y);
  // Get colour depth from globals and validate
  int bpp = 32;
  GetIniVar ("Graphics", "gameBitsPerPixel", &bpp);
  if ((bpp != 16) && (bpp != 32)) {
    // Default to 32
    WARNINGLOG ("Invalid screen colour depth in INI settings, default to 32bpp");
    bpp = 32;
    SetIniVar ("Graphics", "gameBitsPerPixel", bpp);
  }

  // Get refresh rate
  int refresh = 0;
  GetIniVar ("Graphics", "gameRefreshRate", &refresh);

  //-- Init main view params -------------------------
  globals->mScreen.ID     = 1; // constant for main view
  globals->mScreen.X      = x;
  globals->mScreen.Y      = y;
  globals->mScreen.Width  = w;
  globals->mScreen.Height = h;
	globals->mScreen.xRes		= w;
	globals->mScreen.yRes		= h;
  globals->mScreen.Depth  = bpp;
  globals->mScreen.Refresh  = refresh;
  globals->mScreen.bMouseOn = true;

  // Init second view params --------------------------
  globals->sScreen.ID = 0;
  globals->sScreen.X = 20;
  globals->sScreen.Y = 20;
  globals->sScreen.Height = globals->mScreen.Height/2;
  globals->sScreen.Width  = globals->mScreen.Width/2;
  globals->sScreen.bMouseOn = false;

  globals->cScreen = &globals->mScreen;
  //----vector map parameters -------------------
  U_INT dop = VM_DROS | VM_FROS | VM_DLAB | VM_DAPT | VM_DVOR | VM_DILS | VM_DCST;
  U_INT vop = VM_APTI | VM_VORI | VM_VORF | VM_NDBI;
  globals->vmpOpt.Set(dop);
  globals->vmpOpt.Set(vop);  
  globals->vmapZoom   = 50.0f;                // 50 nautical miles for 800 pixels
  globals->vmapTrns   = MakeRGBA (0,0,0,128); // Transparency
  globals->vmapScrn.x = 800;
  globals->vmapScrn.y = 600;
	globals->vmapPos.x  = 200;
	globals->vmapPos.y	= 200;
  //--------------------------------------------------
  //
  // Panel scrolling parameters
  //---------------------------------------------------
  // Panel scroll step
  int i = 10;
  GetIniVar ("Cockpit", "panelScrollStep", &i);
  globals->panelScrollStep = i;

  // Panel creep (mouse scroll) step
  i = 6;
  globals->panelCreepStep = i;

  // Time Manager
  globals->tim->Prepare ();
  globals->tim->SetTimeScale (1.0);
  
  // Initialize various application object pointers
  // sdk: toggle plugin feature usage from ini file
	globals->plugins = new CPluginMain();
  int plugin_allowed = 0;
  GetIniVar ("Sim", "allowDLLFiles", &plugin_allowed);
  globals->plugins->g_plugin_allowed = (plugin_allowed != 0);
	//--- Create OPAL simulation ----------------------------
  globals->opal_sim = opal::createSimulator ();
  globals->opal_sim->setMaxContacts(2);
	globals->Ground   = CreateGround();
  opal::Vec3r g (0.0f, 0.0f, -(GRAVITY_MTS));
  globals->opal_sim->setGravity (g);
  float step_size = ADJ_STEP_SIZE;          // 0.04f;
  GetIniFloat ("PHYSICS", "adjustStepSize", &step_size);
  globals->opal_sim->setStepSize (step_size); // was 0.02
  DEBUGLOG ("InitGlobalsNoPod step_size=%f", step_size);
  char buffer_ [128] = {0};
  globals->sBar = 0;
  globals->bar_timer = 0.5f;
  GetIniFloat ("Sim", "statusBarDeltaSec", &globals->bar_timer);
  globals->fps_limiter = true;
  char buff_ [8] = {0};
  GetIniString ("Sim", "fpsLimiter", buff_, 8);
  if (!strcmp (buff_, "false")) globals->fps_limiter = false;
	//--- Set initial position and orientation -----------------
  globals->iang.x = 0.0; 
  globals->iang.y = 0.0; 
  globals->iang.z = 0.0; 
  globals->dang.x = 0.0; 
  globals->dang.y = 0.0; 
  globals->dang.z = 0.0;
	//--------------------------------------------------------
	return; 
}
//======================================================================================
//  Cleanup settings in the Global data structure
//======================================================================================
void CleanupGlobals (void)
{ CTestBed *tsb = globals->tsb;
	if (tsb)		delete tsb;
	if (globals->plugins_num) globals->plugins->On_EndSituation ();
	delete(globals->plugins);
  CloseUserMenu();   
  CleanupFonts();
	TRACE("Delete Aircraft");
	SAFE_DELETE(globals->pln);
	TRACE("Delete FuiManager");
  SAFE_DELETE (globals->fui);
	TRACE("Delete CloudSystem");
  SAFE_DELETE (globals->cld);         // Delete cloud system
	TRACE("Delete Situation");
  SAFE_DELETE (globals->sit);
	TRACE("Delete SlewManager");
  SAFE_DELETE (globals->slw);         // Delete slew manager
	TRACE("Delete KeyMap");
  SAFE_DELETE (globals->kbd);         // Delete keymap manager;
	TRACE("Delete TerrainCache");
	TCacheMGR *tcm = globals->tcm;
  SAFE_DELETE (globals->tcm);         // Delete terrain before DBcache
	TRACE("Delete SCeneryDBM");
	SAFE_DELETE (globals->scn);
	TRACE("Delete DatabaseCache");
  SAFE_DELETE (globals->dbc);         // Delete DB cache
	TRACE("Delete WeatherManager");
  SAFE_DELETE (globals->wtm);         // Delete weather manager,
	TRACE("Delete Atmosphere");
  SAFE_DELETE (globals->atm);         // Delete atmosphere, 
	TRACE("Delete SQLManager");
  SAFE_DELETE (globals->sqm);         // Delete SQL manager
	TRACE("Delete joystick MGR");
	SAFE_DELETE (globals->jsm);
  globals->nBitmap->ChangeType();
  SAFE_DELETE (globals->nBitmap);
  SAFE_DELETE (globals->csp);
  SAFE_DELETE (globals->cap);
  SAFE_DELETE (globals->snd);
	SAFE_DELETE (globals->ccm);					// Delete camera manager
	//----------------------------------------
	globals->opal_sim->destroy();
	//---------------------------------------
  pshutdown (&globals->pfs);
	SAFE_DELETE (globals->clk);
	SAFE_DELETE (globals->tim);
	//----------------------------------------
	SAFE_DELETE (globals->logAeros);
	SAFE_DELETE (globals->logWings);
  SAFE_DELETE (globals->logWarning);
  SAFE_DELETE (globals->logTerra);
	SAFE_DELETE (globals->logScene);
	SAFE_DELETE (globals->logStreet);
  SAFE_DELETE (globals->logDebug);      // JS Must be the last if log is used
//  SAFE_DELETE (globals);                // JS Must be the last if log is used
//  ---------------------------------------
	TRACE("END CLEAN_UP");
}


#ifdef MEMORY_LEAK_CRT
#if defined(_DEBUG) && defined(HAVE_CRTDBG_H)
//
// Define global for memory state debugging
//
_CrtMemState memoryState;
#endif
#endif // MEMORY_LEAK_CRT

/**
 *  Primary entry point for application shutdown
 *
 *  This function should be called prior to program exit in order to gracefully shut down, deallocate heap memory,
 *  close open files, etc.
 */
void ShutdownAll (void)
{ 
  UnloadIniSettings ();
	//--------------------------------------------
	TRACE("Delete CursorManager");
  // Clean up singletons
  SAFE_DELETE(globals->cum);
	//--------------------------------------------
	TRACE("Clean SkyManager");
  CSkyManager::Instance().Cleanup ();
  // Clean up FUI manager
	TRACE("Clean CFuiManager");
  globals->fui->Cleanup ();
	//-------------------------------------------
	TRACE("Clean TextureManager");
  CTextureManager::Instance().Cleanup ();
	//--------------------------------------------
	TRACE("Clean DatabaseManager");
  CDatabaseManager::Instance().Cleanup();
	//----------------------------------------------
  // sdk: clean-up dll plugins : call DLLKill ()
	TRACE("Kill plugins");
  if (globals->plugins_num) globals->plugins->On_KillPlugins ();
	globals->Disp.Destroy(PRIO_EXTERNAL);
	//---------------------------------------------
  // Clean up global variables
	TRACE("Clean Globals");
  CleanupGlobals ();

}

//======================================================================================
//  Initialize OpenGL camera for 2D rendering of an image surface.
//
//  When the application is not in realtime simulation mode, it is generally
//  displaying a static image (e.g. splash screen or exit screen).  This
//  function is called to initialize the OpenGL camera and blit the provided
//  image surface to the 2D scene.
//
//  @param surf
//    Drawing surface to blit onto the 2D screen
//======================================================================================
void Draw2D (SSurface *surf)
{ // Set projection matrix to 2D screen size
  if (0 == surf)  return;
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D (0, globals->cScreen->Width, 0, globals->cScreen->Height);

  // Set up OpenGL drawing state
  glPushAttrib (GL_ENABLE_BIT);
  glDisable (GL_DEPTH_TEST);
  glDisable (GL_LIGHTING);
  //glDisable (GL_COLOR_MATERIAL);
  glDisable (GL_TEXTURE_2D);

  // Blit the surface
  glRasterPos2i (0, 0);
  Blit (surf);

  // Restore original OpenGL state
  glPopAttrib();

  // Restore original projection matrix
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  return;
}

//======================================================================================
//  Primary entry point for application initialization
//======================================================================================
void InitApplication (void)
{ globals->TimLim = (float(1) / 18);				// Time limit for simulation
	TRACE("Init PROFILES");
	InitialProfile();
	globals->Frame	= 0;
	globals->aMax		= 1.0E+5;
  globals->magDEV = 0;
  globals->NbVTX  = 0;
DupplicateString("*** AUDIO DEB ***",32);
	TRACE("Create Audio Manager");
  globals->snd = new CAudioManager();

DupplicateString("*** SQLMGR DEB ***",32);
	TRACE("Create SQL Manager");
  globals->sqm = new SqlMGR();

DupplicateString("*** DBcache DEB ***",32);
	TRACE("Create Database Manager");
  globals->dbc = new CDbCacheMgr();
  // Initialize various subsystems.  These initialization functions may be
  //   dependent on INI settings and/or POD filesystem
DupplicateString("*** KEYMAP DEB ***",32);
	TRACE("Create Key Map");
  globals->kbd = new CKeyMap();

DupplicateString("*** CAMERA DEB ***",32);
	TRACE("Create Camera Manager");
	globals->ccm   = new CCameraManager(0,0);

DupplicateString("*** SLEW DEB ***",32);
	TRACE("Create Slew Manager");
  globals->slw = new CSlewManager();

DupplicateString("*** IMPORT DEB ***",32);
	TRACE("Create Import Manager");
	globals->exm = new CExport();

  //---Latitude tables for Globe tiles and QGTs --------
DupplicateString("*** GLOBE DEB ***",32);
	TRACE("Init Globe Tile Table");
  InitGlobeTileTable ();
  //-- Initialize singletons----------------------------
  CTextureManager::Instance().Init ();
	TRACE("Init Database Manager");

  CDatabaseManager::Instance().Init();
  //----Initialize sky and weather ----------------------
  CSkyManager::Instance().Init();

DupplicateString("*** ATMOSPHERE DEB ***",32);
	TRACE("Create Atmosphere Manager");
  globals->atm = new CAtmosphereModelJSBSim();

DupplicateString("*** WEATHER DEB ***",32);
	TRACE("Create Weather Manager");
  globals->wtm = new CWeatherManager();
	TRACE("Create Fui Manager");

DupplicateString("*** FUI MGR DEB ***",32);
  globals->fui->Init();
DupplicateString("*** SCENARY DEB ***",32);
	TRACE("START SCENERY MGR");
	globals->scn = new CSceneryDBM();
  //------Start terrain ---------------------------------
DupplicateString("*** TCACHE DEB ***",32);
	TRACE("Start TERRAIN CACHE");
  globals->tcm = new TCacheMGR();
	//-----------------------------------------------------
	TRACE("START CLOUD SYSTEM");
  globals->cld = new CCloudSystem();
  globals->wtm->Init();
	//-----------------------------------------------------
	globals->rdb.Init();
	//--- Check menu items --------------------------------
	CheckTuningMenu();
}
//======================================================================================
//  Initialization of the realtime simulation engine.
//
//  - Create checklist manager
//
//  @param    sit
//    Filename of situation file to load relative to POD Filesystem, or NULL to use default situation
//=======================================================================================
void InitSimulation (void)
{ TRACE("===OBJECT DIMENSION (bytes) ==============");
	//TraceObjectSize();
  TRACE("Start InitSimulation");
  //---- Set default area to Marseilles airport --------------------
  SPosition area = {18802.79, 156335.87, 78};
  globals->geop  = area;
  CVector   orie (0, 0, DegToRad(180));
  globals->iang  = orie;
  CVector   orid (0,0,180);
  globals->dang  = orid;
	//----Joystick ---------------------------------------------------
	globals->jsm		= new CJoysticksManager();
  //------Load situation -------------------------------------------
  globals->sit		= new CSituation ();
	//--- Load external plugs ----------------------------------------
	int p0 = 0;
	GetIniVar("Sim","NavigationPlug",&p0);
	//if (p0)	CNavPlug *nav = new CNavPlug();	
	CNavPlug *nav = new CNavPlug();
	//--- Init OSM usage ---------------------------------------------
	InitOSMusage();
	//----------------------------------------------------------------
  TRACE("End of InitSimulation");
DupplicateString("***END INITSIM ***",32);
  //----Set some options ------------------------------------------
  return;
}
//=======================================================================
//  Class to set options
//=======================================================================
void COption::Toggle(U_INT p)
{ U_INT q = prop & p;             // Get property p
  prop   &= (-1 - q);             // Clear it
  q      ^= p;                    // Invert p
  prop   |= q;                    // Set in
}
//=======================================================================================
// Redraw function when application is in simulation state
//========================================================================================
float          frameRate = 0.0f; // Sim frame rate
//===========================================================================
//  Global frame count used to synchronize all subsystems
//===========================================================================
U_INT Frame = 0xFFFFFFFF;
//===========================================================================
//    Redraw the simulation
//    return the next global state
//===========================================================================

/**
 *  Redraw function for use in realtime simulation application state.
 *
 *  This function is called by the window manager redraw() callback while the application is in realtime simulation mode.
 *
 *  @param    bMain
 *    ???
 *  @return
 *    Enumeration value representing the application state following the completion of the redraw cycle.
 */
float tmp_timerS = 0.0f,
      tmp_timerR = 0.0f;
//===========================================================================
//	Rdraw the simulation
//===========================================================================
int RedrawSimulation ()
{ float dSimT, dRealT;
  const float FPS_LIMIT = globals->opal_sim->getStepSize ();

  // Call the time manager to indicate that another cycle is occurring.
  //   This represents the redraw cycle, not necessarily the simulation
  //   cycle, though at present they are coupled.
  // 
  bool test_fr = frameRate < 1.0f / FPS_LIMIT - (frameRate * 0.1f);
  if (!globals->fps_limiter || test_fr)
  {
    globals->tim->Update ();
    dSimT  = globals->tim->GetDeltaSimTime();
    dRealT = globals->tim->GetDeltaRealTime();
    globals->dST = dSimT;
    globals->dRT = dRealT;
  } else {
    while (tmp_timerS < FPS_LIMIT) { // start basic fps limiter
      globals->tim->Update ();
      dSimT  = globals->tim->GetDeltaSimTime();
      dRealT = globals->tim->GetDeltaRealTime();
      tmp_timerS += dSimT;
      tmp_timerR += dRealT; 
    } // end basic fps limiter
    dSimT = tmp_timerS;
    globals->tim->SetDeltaSimTime (dSimT);
    globals->dST = dSimT;
    tmp_timerS -= FPS_LIMIT;
    dRealT = tmp_timerR;
    globals->tim->SetDeltaRealTime (dRealT);
    globals->dRT = dRealT;
    tmp_timerR = tmp_timerS;
  } 
	//--- Check performance OSM ---------------------------
	//if (Frame == 600)	CheckPerformances(frameRate);
  //------------Update global clock ---------------------
  Frame++;
	frameRate	= float(Frame) / dSimT;
	globals->Frame = Frame;
  globals->clk->Update(dRealT);
  //-------- Update situation----------------------------
  globals->sit->Timeslice (dSimT,Frame);
	if (globals->tcm->StillLoading(GLOBAL_EVN_INITLOAD))		return APP_SIMULATION;
  // The global CSituation object contains all informations about the current
  //   simulation state, user vehicle, camera mode, etc.
  globals->sit->Draw ();
  // Draw UI only on main screen at this time
  // Draw UI components
  globals->fui->Draw ();
	//--- Draw master menu --------------------------------
	puDisplay();
  // draw cursor only on the window where the mouse is really on
  globals->cum->Draw (dSimT);
  return APP_SIMULATION;
}

//==============================================================================
//  Initialize splash screen
//
//  Initialize static splash screen image displayed during initial application
//  startup.
//==============================================================================
CTexture *tSplash = NULL;

void InitSplashScreen (void)
{
  tSplash = new CTexture();
  tSplash->LoadFromDisk ("Splash1.JPG", 0, 0, 0);
}

///=========================================================================
//  Redraw splash screen
//
//  This function is called by the window manager redraw() 
//	callback while the application is initializing.
///=========================================================================
void RedrawSplashScreen (void)
{
  int vp[4]; 
  int wd = tSplash->GetWD();
  int ht = tSplash->GetHT();
  int sw = globals->mScreen.Width;
  int sh = globals->mScreen.Height;

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D (0, wd, 0, ht);
  // Initialize modelview matrix to ensure panel drawing is not affected by
  //   any junk left at the top of the stack by previous rendering
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  glLoadIdentity ();
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  //---Save viewport -----------------------------------------------
  glGetIntegerv(GL_VIEWPORT,vp);
  glViewport(0,0,sw,sh);
  glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
  glEnable  (GL_TEXTURE_2D);
  glDisable (GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  //----Draw the panel as a textured square  -----------------------
  tSplash->Draw();
  //----------------------------------------------------------------
  glViewport(vp[0],vp[1],vp[2],vp[3]);
  glPopClientAttrib ();
  glPopAttrib();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  CHECK_OPENGL_ERROR

  return;
}

///=========================================================================
//  Cleanup exit screen.
//
//  Free resources used to display the application exit screen.
///=========================================================================
void CleanupSplashScreen (void)
{
	SAFE_DELETE(tSplash);
}

///=========================================================================
//  Initialize exit screen
//
//  When realtime simulation ends and the application exits, it displays
//  a static exit screen while cleaning up.  This function initializes
//  the exit image and renders it to a drawing surface.
//===========================================================================
static CTexture *tExit = NULL;

void InitExitScreen (void)
{
  // Load exit screen
  tExit = new CTexture();
  tExit->LoadFromDisk ("Legacy_off_2.jpg", 0, 0, 0);
}

///=========================================================================
//  Redraw exit screen.
//
//  This function is called by the window manager redraw() 
//	callback while the application is exiting.
///=========================================================================
void RedrawExitScreen (void)
{ int vp[4]; 
  int wd = tExit->GetWD();
  int ht = tExit->GetHT();
  int sw = globals->mScreen.Width;
  int sh = globals->mScreen.Height;

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D (0, wd, 0, ht);
  // Initialize modelview matrix to ensure panel drawing is not affected by
  //   any junk left at the top of the stack by previous rendering
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  glLoadIdentity ();
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  //---Save viewport -----------------------------------------------
  glGetIntegerv(GL_VIEWPORT,vp);
  glViewport(0,0,sw,sh);
  glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
  glEnable  (GL_TEXTURE_2D);
  glDisable (GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  //----Draw the panel as a textured square  -----------------------
  tExit->Draw();
  //----------------------------------------------------------------
  glViewport(vp[0],vp[1],vp[2],vp[3]);
  glPopClientAttrib ();
  glPopAttrib();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  CHECK_OPENGL_ERROR

  return;
}


//=====================================================================================
//  Primary entry point for graceful application shutdown.
 //====================================================================================
void ExitApplication (void)
{ globals->mBox.DumpAll();
  // Gracefully shut down and clean up before exiting
  ShutdownAll ();
  CleanupWindowManager ();
	TRACE("DELETE EXIT TEXTURE");
  SAFE_DELETE(tExit);
	TRACE("END of PROCESS");
	SAFE_DELETE (globals->logTrace);		 // Trace is needed  to the end
  SAFE_DELETE (globals);               // JS Must be the last if log is used
#if defined(_DEBUG)
 _CrtDumpMemoryLeaks();
#endif
  TerminateProcess (GetCurrentProcess(), 0);
	exit(0);
}

//==================================================================================
// pthread mutexes for warning, debug and gtfo logging
//==================================================================================
static pthread_mutex_t	mutexWarn, 
												mutexTrace,
												mutexOSM,
												mutexAeros,
												mutexScene,
												mutexDebug, 
												mutexGtfo;  // JSDEV* add trace

/**
 *  Initialize tracing options.
 */
void InitTraces(void)
{	int op	= 0;
	globals->ttr      = 0;
  GetIniVar ("TRACE", "Step", &op);
  globals->ttr  = op;
	GetIniVar ("TRACE", "TimeSlice", &op);
	if (op) globals->Trace.Set(TRACE_TIMESLICE);
	op	= 0;
	GetIniVar ("TRACE", "PrepareMsg",&op);
	if (op) globals->Trace.Set(TRACE_MSG_PREPA);
	op	= 0;
	GetIniVar ("TRACE", "Dependency",&op);
	if (op) globals->Trace.Set(TRACE_DEPENDENT);
	op	= 0;
	GetIniVar ("TRACE", "LoFrame", &op);
	globals->LoFrame	= (U_INT)op;
	op	= 0;
	GetIniVar ("TRACE", "NbFrame", &op);
	globals->HiFrame	= (U_INT)(globals->LoFrame + op);
	op	= 0;
	GetIniVar ("TRACE", "DbCacheMgr", &op);
	if (op) globals->Trace.Set(TRACE_DBCACHEMGR);
  op  = 0;
	GetIniVar ("TRACE", "FrameRate", &op);
  if (op) globals->Trace.Set(TRACE_FRAME_RATE);
  op  = 0;
  GetIniVar ("TRACE", "TerrainCache", &op);
  if (op) globals->Trace.Set(TRACE_TERRAIN_OP);
  op  = 0;
  GetIniVar("TRACE","Subsystem",&op);
  if (op) globals->Trace.Set(TRACE_SUBSYSTEM);
  op  = 0;
  GetIniVar("TRACE","Gear", &op);
  if (op) globals->Trace.Set(TRACE_WHEEL);
	return;	}

/**
 *  Prepare situation for realtime simulation.
 */
void PrepareSimulation (void)
{
  globals->sit->Prepare ();
}
//==================================================================================
//	This is a patch for X64 system where user exception handler are not called
//	by windows
//==================================================================================
void EnableCrashingOnCrashes()
{
    typedef BOOL (WINAPI *tGetPolicy)(LPDWORD lpFlags);
    typedef BOOL (WINAPI *tSetPolicy)(DWORD dwFlags);
    const DWORD EXCEPTION_SWALLOWING = 0x0001;
		const DWORD MaskException = (~EXCEPTION_SWALLOWING);
    HMODULE kernel32 = LoadLibraryA("kernel32.dll");
    tGetPolicy pGetPolicy = (tGetPolicy)GetProcAddress(kernel32,
                "GetProcessUserModeExceptionPolicy");
    tSetPolicy pSetPolicy = (tSetPolicy)GetProcAddress(kernel32,
                "SetProcessUserModeExceptionPolicy");
    if (pGetPolicy && pSetPolicy)
    {
        DWORD dwFlags;
        if (pGetPolicy(&dwFlags))
        {
            // Turn off the filter
            pSetPolicy(dwFlags & MaskException);	//~EXCEPTION_SWALLOWING);
        }
    }
}
//=======================================================================================
//	Qualify some standards exceptions
//=======================================================================================
extern int FatalError(int code);
//=======================================
char* GetExDescription( int code )
   {
      switch( code ) {
         case EXCEPTION_ACCESS_VIOLATION:         return "ACCESS_VIOLATION"         ;
         case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "ARRAY_BOUNDS_EXCEEDED"    ;
         case EXCEPTION_BREAKPOINT:               return "BREAKPOINT"               ;
         case EXCEPTION_DATATYPE_MISALIGNMENT:    return "DATATYPE_MISALIGNMENT"    ;
         case EXCEPTION_FLT_DENORMAL_OPERAND:     return "FLT_DENORMAL_OPERAND"     ;
         case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "FLT_DIVIDE_BY_ZERO"       ;
         case EXCEPTION_FLT_INEXACT_RESULT:       return "FLT_INEXACT_RESULT"       ;
         case EXCEPTION_FLT_INVALID_OPERATION:    return "FLT_INVALID_OPERATION"    ;
         case EXCEPTION_FLT_OVERFLOW:             return "FLT_OVERFLOW"             ;
         case EXCEPTION_FLT_STACK_CHECK:          return "FLT_STACK_CHECK"          ;
         case EXCEPTION_FLT_UNDERFLOW:            return "FLT_UNDERFLOW"            ;
         case EXCEPTION_ILLEGAL_INSTRUCTION:      return "ILLEGAL_INSTRUCTION"      ;
         case EXCEPTION_IN_PAGE_ERROR:            return "IN_PAGE_ERROR"            ;
         case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "INT_DIVIDE_BY_ZERO"       ;
         case EXCEPTION_INT_OVERFLOW:             return "INT_OVERFLOW"             ;
         case EXCEPTION_INVALID_DISPOSITION:      return "INVALID_DISPOSITION"      ;
         case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "NONCONTINUABLE_EXCEPTION" ;
         case EXCEPTION_PRIV_INSTRUCTION:         return "PRIV_INSTRUCTION"         ;
         case EXCEPTION_SINGLE_STEP:              return "SINGLE_STEP"              ;
         case EXCEPTION_STACK_OVERFLOW:           return "STACK_OVERFLOW"           ;
         default: return "OTHER EXCEPTION" ;
      }
   }
//==================================================================================
//	Application exception handler to replace Windows provided
//==================================================================================
LONG WINAPI FlyExceptionFilter(PEXCEPTION_POINTERS p)
{ TRACE("===================EXCEPTION CALLED ============================\n");
	PEXCEPTION_RECORD px = p->ExceptionRecord;
	DWORD code = px->ExceptionCode;
	char *text = GetExDescription(code);
	TRACE("Module %s, ThREAD0 in %s, Thread1 in %s",
				globals->module,globals->thread[0],globals->thread[1]);
	TRACE("Error code = %8x: %s",code,text);
  return EXCEPTION_EXECUTE_HANDLER;
}
//==================================================================================
//==================================================================================
//  Initial application entry point.
//
//  @param argc
//    Number of command-line arguments passed to the application.
//  @param argv
//    List of command-line arguments passed to the application
//  @return
//    Return code provided back to operating system on application termination
//==================================================================================
//==================================================================================
int main (int argc, char **argv)
{ 
	//--- Microsoft bug for exception handler ---------------------------
	EnableCrashingOnCrashes();
	//--- Replace standard unhandled exception handler ------------------
	SetUnhandledExceptionFilter(FlyExceptionFilter);
	//-------------------------------------------------------------------
	double pxr = double(1) / 72;                // Pixel radius in inch
  double hpx = FN_FEET_FROM_INCH(pxr);        // Pixel radius in feet
  double nrp = 20;                            // Near clip plane
  globals = new SGlobals;	                    // Create global structure
	//------------- Initialize mutexes----------------------------------
  pthread_mutex_init (&mutexWarn,  NULL);
  pthread_mutex_init (&mutexDebug, NULL);
  pthread_mutex_init (&mutexGtfo,  NULL);
	pthread_mutex_init (&mutexOSM ,  NULL);
  pthread_mutex_init (&mutexTrace, NULL);		  // JSDEV* add trace
	pthread_mutex_init (&mutexAeros, NULL);		  // JSDEV* add trace
	pthread_mutex_init (&mutexScene, NULL);
	//--- Create trace file now -----------------------------------------
  globals->logTrace	= new CLogFile ("logs/Trace.log", "w");
  //--------- Load application settings from INI file -----------------
  LoadIniSettings ();

  // Initialize simulation situation (.SIT) to load
  if (argc == 2) {
    // First argument is .SIT filename
    strncpy (globals->sitFilename, argv[1],FNAM_MAX);
  } else {
    // No .SIT filename provided on command line; use INI settings
    GetIniString ("UI", "startupSituation", globals->sitFilename, FNAM_MAX);
    if (strlen (globals->sitFilename) == 0) {
      // No default startup situation specified in INI settings
      strncpy (globals->sitFilename, "Saved Simulations/Default.sit",FNAM_MAX);
    }
  }

  //---Put find root foldeer here to open log file in final directory
	char *flyRootFolder = ".";
  //=========Init the global structure==================================
  strncpy(globals->FlyRoot,flyRootFolder,FNAM_MAX);			// Root folder
  InitTraces();
  TRACE("TRACE FILE CREATED"); 
  globals->logTerra		= new CLogFile ("logs/ChangedTiles.log", "a+");
	globals->logScene		= new CLogFile ("logs/Scenery.log", "w");
	globals->logStreet	= new CLogFile ("logs/OpenStreet.log","w");
	//----------------------------------------------------------------------
	globals->stop	 = 0;
  //----------- Init global databank -------------------------------------
  globals->tmzTAB = tmzTAB;
  globals->comTAB = comTAB;
  globals->cmaTAB = cmaTAB;
  globals->abcTAB = abcTAB;
  globals->vorTAB = vorTAB;
  globals->wptTAB = wptTAB;
  globals->gazTAB = gasTAB;
  globals->monTAB = monTAB;
  globals->ownTAB = ownTAB;
  globals->rwgTAB = rwgTAB;
  globals->terTAB = terraTAB;
  //---View parameters -----------------------------------------------
  globals->nearP = nrp;                       // Near plane at 30 feet
  globals->afarP = 1.0e+10;                   // Far plane
  globals->fovX  = 50.0;                      // Field of view
  globals->pitch = (hpx / nrp);               // one pixel fov
  globals->camRange = nrp + 20;               // Initial range 50 feets
  //---Default global simulation data --------------------------------
	globals->trn			= 0;
  globals->cum      = 0;
  globals->cap      = new CCameraRunway();
  globals->csp      = new CCameraSpot();
  globals->cam      = globals->csp;
  globals->ccm      = 0;
  //---Time of Day ---------------------------------------------------
  globals->tod      = 0;
  //---Chart map -----------------------------------------------------
  globals->MapName[0] = 0;
  globals->NulChar    = 0;
  //---Profile init --------------------------------------------------
  globals->noAWT    = 1;                      // No animated water
  //---Terrain parameters --------------------------------------------
	globals->pakCAP		= 1000;										// 1000 vertices
  globals->maxView  = 40;                     // Default Maximum view (miles)
  globals->highRAT  = 0.32;                   // Hight resolution (ratio of maxView)
  globals->skyDist  = FN_FEET_FROM_MILE(60);  // Sky distance
	globals->mipOBJ		= 4;											// Mip level for Objects
	globals->mipTER		= 3;											// Mip level for terrain
  //---Master radio interface ----------------------------------------
  globals->lnd  =  0;
  //---Object pointer ------------------------------------------------
  globals->tim  = new CTimeManager();         // Time manager
  globals->clk  = new CClock();               // Global clock
  globals->tsb  = 0;                          // Test bed 
  globals->sit  = 0;                          // Situation
  globals->dbc	= 0;								          // Database cache
  globals->sqm  = 0;                          // SQL Manager
  globals->sql[0] = 0;                        // SQL thread 0
	globals->sql[1]	= 0;												// SQL thread 1
  globals->tcm  = 0;                          // Terrain cache
  globals->txw  = 0;                          // Texture cache
  globals->apm  = 0;                          // Airport models
  globals->m3d  = 0;                          // 3D world manager
  globals->exm  = 0;                          // Export
  globals->imp  = 0;                          // Import
  globals->kbd  = 0;                          // Keyboard interface
  globals->fui  = 0;                          // Windows manager
  globals->jsm  = 0;                          // Joystick manager
  globals->skm  = 0;                          // Sky manager
  globals->wtm  = 0;                          // Weather manager
  globals->mag  = 0;                          // Magnetic Model
  globals->cld  = 0;                          // Cloud system
  globals->slw  = 0;                          // Slew manager
  globals->exm  = 0;													// Export manager
  globals->imp  = 0;		//new CImport();      // Import manager
  globals->atm  = 0;                          // Atmosphere
  //---Aircraft objects -----------------------------------------------
  globals->pln  = 0;
  globals->ccm  = 0;                          // Camera manager 
  globals->wpb  = 0;                          // Window probe
	//--- Module and thread state ---------------------------------------
	globals->module = "Init";
	globals->thread[0]	= "OFF";
	globals->thread[1]	= "OFF";
	pthread_mutex_init(&globals->mux,NULL);
  //-------------------------------------------------------------------
  globals->wObj = 0;                          // Current object
  TRACE("Globals INITIALIZED");
  //-------------------------------------------------------------------
	//--- Loose memory marker 1 -----------------------
DupplicateString("*** GLOBAL DEB ***",32);
  //--------------------------------------------------------------------
  // Initialize globals variables
  //-------------------------------------------------------------------
  InitGlobalsNoPodFilesystem ((char*)flyRootFolder);
  TRACE("InitGlobalsNoPodFilesystem OK");
  globals->appState = APP_SPLASH_SCREEN;
DupplicateString("****FUI MGR ****",32);
  globals->fui = new CFuiManager();
  TRACE("CFuiManager CREATED");
  // Initialize graphics engine and window manager
DupplicateString("****FUI INI ****",32);
  InitWindowManager (argc, argv);
  TRACE("InitWindowManager OK");
DupplicateString("****GRAFIC ****",32);
  InitGraphics ();
  TRACE("InitGraphics OK");
  //---------------------------------------------------------------------------
  // Initialize the system pod filesystem.  The top-level reference to the POD
  //   filesystem is stored in the global variable struct, so this must
  //   be called after globals have been instantiated.
  //---------------------------------------------------------------------------
DupplicateString("****POD INI ****",32);

  pinit (&globals->pfs, "logs/systempod.log");		// JSDEV* add file name
  //----ADD POD FROM FLYII FOLDERS -------------------------------------------
  PFS *pfs = &globals->pfs;
	int lgr  = FNAM_MAX;
  // Mount folders from Fly! II filesystem
	TRACE("Mounting Pods in /SYSTEM");
  char folder[PATH_MAX];
	_snprintf(folder,lgr,"%s/SYSTEM",flyRootFolder);
  paddpodfolder (pfs, folder);
	
  //------------------------------------------
	TRACE("Mounting Pods in /AIRCRAFT");
	_snprintf(folder,lgr,"%s/AIRCRAFT",flyRootFolder);
  paddpodfolder (pfs, folder, true);      ///< Add subfolders too
  //------------------------------------------
	TRACE("Mounting Pods in /TAXIWAYS");
	_snprintf(folder,lgr,"%s/TAXIWAYS",flyRootFolder);
  paddpodfolder (pfs, folder);
  //-----shared sceneries are mounted by ScenerySet------
  //------ Add any disk files except pod -----
	TRACE("Mounting Disk file in /ART");
  padddiskfolder (pfs, flyRootFolder, "ART");
	TRACE("Mounting Disk file in /AIRCRAFT");
  padddiskfolder (pfs, flyRootFolder, "AIRCRAFT");
	TRACE("Mounting Disk file in /DATA");
  padddiskfolder (pfs, flyRootFolder, "DATA");
	TRACE("Mounting Disk file in /MODELS");
  padddiskfolder (pfs, flyRootFolder, "MODELS");
	TRACE("Mounting Disk file in /SAVE SIMULATION");
  padddiskfolder (pfs, flyRootFolder, "SAVED SIMULATION");
	TRACE("Mounting Disk file in /SYSTEM");
  padddiskfolder (pfs, flyRootFolder, "SYSTEM");
	TRACE("Mounting Disk file in /UI");
  padddiskfolder (pfs, flyRootFolder, "UI");
	TRACE("Mounting Disk file in /WORLD");
  padddiskfolder (pfs, flyRootFolder, "WORLD");
	TRACE("Mounting Disk file in /SOUND");
  padddiskfolder (pfs, flyRootFolder, "SOUND");
DupplicateString("****POD END ****",32);

  // Initialize subsystems so that mouse and keyboard callbacks can be handled
 
  // sdk: load and initialize the dll plugins
  if (globals->plugins->g_plugin_allowed) {
    if (globals->plugins_num = globals->plugins->On_LoadPlugins ()) {
      globals->plugins->On_InitPlugins ();
    } else
      globals->plugins_num = 0;
  }
  else
    //! plugins_num is used as a plugin flag along the code
    globals->plugins_num = 0; 
	//--- Open VMAP configuration -------------------------------
	VMAP();	
	//--- Open Master menu --------------------------------------
	TRACE("Open Master MENU");
	OpenMasterMenu();
  //--- Initialize fonts and cursor manager -------------------
	TRACE("Initialize FONTS");
  InitFonts ();
  globals->cum = new CCursorManager();
	TRACE("Enter GLUT LOOP");
  EnterWindowManagerMainLoop ();
  return 0;
}

#ifdef _WINDOWS

#include <shellapi.h>

//=====================================================================================
//  Initial Windows application entry point.
//=====================================================================================
int APIENTRY WinMain (HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPSTR lpCmdLine,
                      int nShowCmd)
{
  main(__argc, __argv);
  return 0;
}
#endif // _WINDOWS
//=====================================================================================
//	ABORT OPERATOR
//=================================================================================
void GTFO::operator() (const char* fmt, ...)
{
  // Make thread-safe
  pthread_mutex_lock (&mutexGtfo);

  glutLeaveGameMode ();

  CLogFile *log = new CLogFile ("Logs/gtfo.log", "a");

  char usermsg[1024];
  // Format user message from varargs
  if (fmt != NULL) {
    va_list argp;
    va_start(argp, fmt);
    vsnprintf (usermsg,1023, fmt, argp);
		usermsg[1023];
    va_end(argp);
  } else  strcpy (usermsg, "");

  // Trim all but last two path components from filename
#ifdef _WIN32
  char separator = '\\';
#else
  char separator = '/';
#endif // _WIN32

  int nSeparators = 0;
  int len = strlen(m_file);

  // Initialize file buffer with entire filename
  char file[1024];
  char *c;
  for (c = (m_file + len); c >= m_file; c--) {
    if (*c == separator) {
      nSeparators++;
      if (nSeparators == 2) {
        // Found two separators...extract remnant to buffer
        c++;
        break;
      }
    }
  }
  strcpy (file, c);

  // Output log message
  log->Write ("Abnormal exit\n  File: %s  Line: %d\n  %s\n", file, m_line, usermsg);

  delete log;

#if defined(WIN32) && defined(_DEBUG)
  TerminateProcess (GetCurrentProcess(), 0);
#endif
	exit(-1);
}
//=================================================================================
void WARN::operator() (const char* fmt, ...)
{
  // Make thread-safe
  pthread_mutex_lock (&mutexWarn);

  if (globals->logWarning != NULL) {
    char usermsg[2048];
    // Format user message from varargs
    if (fmt != NULL) {
      va_list argp;
      va_start(argp, fmt);
      vsnprintf(usermsg,2047, fmt, argp);
			usermsg[2047] = 0;
      va_end(argp);
    } else {
      // NULL user message format string
      strcpy (usermsg, "");
    }

    // Trim all but last two path components from filename
#ifdef _WIN32
    char separator = '\\';
#else
    char separator = '/';
#endif // _WIN32

    int nSeparators = 0;
    int len = strlen(m_file);

    // Initialize file buffer with entire filename
    char file[2048];
    char *c;
    for (c = (m_file + len); c >= m_file; c--) {
      if (*c == separator) {
        nSeparators++;
        if (nSeparators == 2) {
          // Found two separators...extract remnant to buffer
          c++;
          break;
        }
      }
    }
    strcpy (file, c);

    // Output log message
    globals->logWarning->Write ("Warning\n  File: %s  Line: %d\n  %s\n",
      file, m_line, usermsg);
  }

  pthread_mutex_unlock (&mutexWarn);
}
//-------------------------------------------------------------------------
//	JSDEV* Implement TRACE log
//-------------------------------------------------------------------------
TRACE::TRACE(const char *fmt,...)
{	pthread_mutex_lock (&mutexTrace);
	if (globals->logTrace != NULL) {
		va_list argp;
		va_start(argp, fmt);
		globals->logTrace->Write (fmt, argp);
		va_end(argp);	}
	pthread_mutex_unlock (&mutexTrace);
}
//-------------------------------------------------------------------------
//	JSDEV* Implement AERO log
//-------------------------------------------------------------------------
AERO::AERO(const char *fmt,...)
{	pthread_mutex_lock (&mutexAeros);
	if (globals->logAeros != NULL) {
		va_list argp;
		va_start(argp, fmt);
		globals->logAeros->Write (fmt, argp);
		va_end(argp);	}
	pthread_mutex_unlock (&mutexAeros);
}
//-------------------------------------------------------------------------
//	JSDEV* Implement WING log
//-------------------------------------------------------------------------
WING::WING(const char *fmt,...)
{	pthread_mutex_lock (&mutexAeros);
	if (globals->logWings != NULL) {
		va_list argp;
		va_start(argp, fmt);
		globals->logWings->Write (fmt, argp);
		va_end(argp);	}
	pthread_mutex_unlock (&mutexAeros);
}

//-------------------------------------------------------------------------
//	JSDEV* Implement Scenery log
//-------------------------------------------------------------------------
SCENE::SCENE(const char *fmt,...)
{	pthread_mutex_lock (&mutexScene);
	if (globals->logScene != NULL) {
		va_list argp;
		va_start(argp, fmt);
		globals->logScene->Write (fmt, argp);
		va_end(argp);	}
	pthread_mutex_unlock (&mutexScene);
}

//-------------------------------------------------------------------------
//	JSDEV* Implement TERRA log
//-------------------------------------------------------------------------
TERRA::TERRA(const char *fmt,...)
{	pthread_mutex_lock (&mutexTrace);
	if (globals->logTerra != 0) {
    globals->logTerra->Date();
		va_list argp;
		va_start(argp, fmt);
		globals->logTerra->Write (fmt, argp);
		va_end(argp);	}
	pthread_mutex_unlock (&mutexTrace);
}
//-------------------------------------------------------------------------
//	JSDEV* Implement OpenStreet log
//-------------------------------------------------------------------------
STREETLOG::STREETLOG(const char *fmt,...)
{	pthread_mutex_lock (&mutexOSM);
	if (globals->logStreet != 0) {
   	va_list argp;
		va_start(argp, fmt);
		globals->logStreet->Write (fmt, argp);
		va_end(argp);	}
	pthread_mutex_unlock (&mutexOSM);
}

//---------------------------------------------------------------------
void DEBUG::operator() (const char* fmt, ...)
{
  // Make thread-safe
  pthread_mutex_lock (&mutexDebug);

  if (globals->logDebug != NULL) {
    char usermsg[1024];
    // Format user message from varargs
    if (fmt != NULL) {
      va_list argp;
      va_start(argp, fmt);
      vsnprintf(usermsg,1023, fmt, argp);
			usermsg[1023] = 0;
      va_end(argp);
    } else {
      // NULL user message format string
      strcpy (usermsg, "");
    }

    // Trim all but last two path components from filename
#ifdef _WIN32
    char separator = '\\';
#else
    char separator = '/';
#endif // _WIN32

    int nSeparators = 0;
    int len = strlen(m_file);

    // Initialize file buffer with entire filename
    char file[1024];
    char *c;
    for (c = (m_file + len); c >= m_file; c--) {
      if (*c == separator) {
        nSeparators++;
        if (nSeparators == 2) {
          // Found two separators...extract remnant to buffer
          c++;
          break;
        }
      }
    }
    strcpy (file, c);

    // Output log message
    globals->logDebug->Write ("Debug\n  File: %s  Line: %d\n  %s\n",
      file, m_line, usermsg);
  }

  pthread_mutex_unlock (&mutexDebug);
}

//=================END OF FILE ===========================================================