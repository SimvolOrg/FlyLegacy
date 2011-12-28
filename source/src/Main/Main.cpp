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

// Include Visual Leak Detector header
#ifdef MEMORY_LEAK_VLD
#include <vld.h>
#endif // MEMORY_LEAK_VLD
#include "../Include/FlyLegacy.h"
#include "../Include/Globals.h"
#include "../Include/Situation.h"
#include "../Include/KeyMap.h"
#include "../Include/Ui.h"
#include "../Include/FuiParts.h"
#include "../Include/Database.h"
#include "../Include/TerrainCache.h"
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
//----Windows particular -------------------------
#include <math.h>
#include <pthread.h>
#include <direct.h>
using namespace std;
//=============================================================================================
//  GLOBAL DATABANK
//  The following data are used at global level for consistency.
//  All module using those name refer to those table
//=============================================================================================
// General-purpose global settings
SGlobals *globals = NULL;
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
/**
 *  Lookup the ICAO Spelling Alphabet word (ALPHA, BRAVO, etc.) for the supplied character.
 *
 *  \param  car
 *    Character for which to get ICAO Spelling Alphabet word; must be in range a-z or A-Z
 *  \return 
 *    String containing ICAO Spelling Alphabet word for the character
 *
 */
char *GetStandardAlphabet(char car)
{ car |= ' ';                   // set lower case
  car -= 'A';                   // index
  return (car > 26)?(abcTAB[0]):(abcTAB[car]);
}


//=============================================================================================
#ifdef _WIN32
//
// Array for storage of gamma ramp data
//
static WORD savedGammaRamp[3][0x100];
static HDC  hdc = 0;
#endif // _WIN32
//========================================================================================
//  Restaure gamma ramp from file
//========================================================================================
void RestoreGamma()
{ WORD ramp[3][0x100];
  HDC hdc = wglGetCurrentDC();
  char *fn = "System/Gamma.cof";
  FILE* f = fopen (fn, "r");
  if (!f) return;
  //----------------------------------------------------------
  int k = -1;
  int r,g,b;
  int q = 0;
  char go = 1;
  while (go)
  { int n = fscanf(f,"%d R%d G%d B%d \n",&k,&r,&g,&b);
    if (4 != n) break;
    ramp[0][k] = r;
    ramp[1][k] = g;
    ramp[2][k] = b;
    q++;
  }
  fclose(f);
  if (q != 256)  return;
  // Set the new gamma ramp
  BOOL ok = SetDeviceGammaRamp (hdc, ramp);
  return;
}
//============================================================================
//  Set Initial profile from ini file 
//	Lock, unlock individual feature 
//	NOTE: globals->aPROF is used to lock various items when some
//				windows are activated.  For example using TERRA browser prevent
//				aircraft display and other features.
//============================================================================
void InitialProfile()
{ globals->noTER = 0;
	globals->noAPT = 0;
	globals->noOBJ = 0;
	globals->noEXT = 0;
	globals->noINT = 0;
	globals->noMET = 0;
	//-----------------------------------------------------
	int nt   = 0;
	//--- Check for terrain rendition ---------------------
  GetIniVar("Sim", "NoTerrain", &nt);
  if (nt) globals->noTER = 1;
  if (nt) globals->noAPT = 1;
  if (nt) globals->noOBJ = 1;
  if (nt) globals->noMET = 1;
  if (nt) globals->noAWT = 1;
	//--- Check for airport rendition ---------------------
	int na = 0;
  GetIniVar("Sim", "NoAirport", &na);
  if (na) globals->noAPT = 1;
	//--- Check for Object rendition ----------------------
	int no = 0;
  GetIniVar("Sim", "NoModel", &no);        // Skip 3D objects
  if (no) globals->noOBJ = 1;
	//--- Check for plane rendition ------------------------
	int np = 0;
  GetIniVar("Sim", "NoAircraft", &np);
  if (np)	globals->noEXT = 1;									// No external aircraft
	if (np)	globals->noINT = 1;									// No internal aircraft
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
	globals->aPROF.Rep(pf);
	if (p & PROF_NO_TER)	globals->noTER += dta;
	if (p & PROF_NO_APT)	globals->noAPT += dta;
	if (p & PROF_NO_INT)	globals->noINT += dta;
	if (p & PROF_NO_EXT)	globals->noEXT += dta;
	if (p & PROF_NO_OBJ)	globals->noOBJ += dta;
	if (p & PROF_NO_MET)	globals->noMET += dta;
	return;
}
//========================================================================================
//  Generic initialization of the graphics engine.
//
//  This code must be free of any window--system specific calls unless wrapped in preprocessor macro checks.
//  Code in this function may be dependent on INI settings and globals
//========================================================================================
void InitGraphics (void)
{
  // Initialize GLEW
  GLenum e = glewInit ();
  if (e != GLEW_OK)  WARNINGLOG ("GLEW Initialization error : 0x%04X", e);

  // Check the availability of various GL extensions on the host machine, and set
  //   global flags accordingly

  // Check for compressed texture formats
  globals->dxt1Supported = false;
  globals->dxt1aSupported = false;
  globals->dxt3Supported = false;
  globals->dxt5Supported = false;
  GLint nFormats;
  glGetIntegerv (GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB, &nFormats);
  GLint *format = new GLint[nFormats];
  glGetIntegerv (GL_COMPRESSED_TEXTURE_FORMATS_ARB, format);
  for (int i=0; i<nFormats; i++) {
    switch (format[i]) {
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
      globals->dxt1Supported = true;
      break;
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
      globals->dxt1aSupported = true;
      break;
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
      globals->dxt3Supported = true;
      break;
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      globals->dxt5Supported = true;
      break;
    }
  }
  SAFE_DELETE_ARRAY (format);

  // Establish baseline OpenGL state that differs from default.
  // The following states are not explicitly set, as the baseline uses the
  //   defaults per OpenGL standards:
  //    GL_LIGHTING             Disabled
  //    GL_ALPHA_TEST           Disabled
  //    GL_STENCIL_TEST         Disabled
  //    GL_FOG                  Disabled
  //
  glClearColor (0, 0, 0, 0);
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable (GL_CULL_FACE);
  glEnable (GL_DEPTH_TEST);

  DumpOpenGLDriver ("Debug/OpenGL_Driver.txt");
  DumpOpenGLState ("Debug/OpenGL_Init.txt");
}

/**
 *  Initialize fonts for use by the application.
 *
 *  The application uses a set of bitmap fonts availalble in the global data structure.
 */
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
  strcpy (globals->fonts.ftsmal10.fontName, "ART/FTSMAL10.RAW");
  LoadFont (&globals->fonts.ftsmal10);
  strcpy (globals->fonts.ftthin24.fontName, "ART/FTTHIN24.RAW");
  LoadFont (&globals->fonts.ftthin24);
  strcpy (globals->fonts.ftasci10.fontName, "ART/FTASCI10.RAW");
  LoadFont (&globals->fonts.ftasci10);
  
  /*!
   * \todo Debug fonts:
   *         FTDOS12.RAW and FTTINY7W.RAW do not comply with required
   *           palette index scheme for fonts; required bitmap patch
   *         FTNORM14.RAW
   *         FTNORM28.RAW
   */
   
}

/**
 *  Clean up fonts used by the application.
 */
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

//======================================================================================
//  Initialize global variables that are not dependent upon files in the POD Filesystem.
//
//  Code in this function may be dependent on INI settings, but not on any contents of the
//    POD filesystem.
//======================================================================================
void InitGlobalsNoPodFilesystem (char *root)
{ //MEMORY_LEAK_MARKER ("globalinit");
  // Allocate NULL bitmap
	globals->dMap				= new CFmtxMap();
  globals->nBitmap		= new CBitmap;
  globals->dBug       = 0;
  globals->logDebug   = new CLogFile ("Logs/Debug.log", "w");
  globals->logWarning = new CLogFile ("Logs/Warning.log", "w");

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
    if (32 == dm.dmBitsPerPel) {
      int num, denom;
      AspectRatio (dm.dmPelsWidth, dm.dmPelsHeight, num, denom);
#ifdef _DEBUG
      DEBUGLOG ("Display setting %d (%d:%d) : %dx%d:%d@%d", iModeNum, num, denom,
        dm.dmPelsWidth, dm.dmPelsHeight, dm.dmBitsPerPel, dm.dmDisplayFrequency);
#endif
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
/*
  // Validate screen width/height
  bool screen640 = (w == 640) && (h == 480);
  bool screen800 = (w == 800) && (h == 600);
  bool screen1024 = (w == 1024) && (h == 768);
  if (!(screen640 || screen800 || screen1024)) {
    // Default to 1024 x 768
    WARNINGLOG ("Invalid screen resolution in INI settings, default to 1024x768");
    w = 1024;
    h = 768;
    SetIniVar ("Graphics", "gamePIXX", w);
    SetIniVar ("Graphics", "gamePIXY", h);
  }
*/
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

  // Init main view params
  globals->mScreen.ID     = 1; // constant for main view
  globals->mScreen.X      = x;
  globals->mScreen.Y      = y;
  globals->mScreen.Width  = w;
  globals->mScreen.Height = h;
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
  //----vector map zoom parameters -------------------
  U_INT dop = VM_DROS | VM_FROS | VM_DLAB | VM_DAPT | VM_DVOR | VM_DILS | VM_DCST;
  U_INT vop = VM_APTI | VM_VORI | VM_VORF | VM_NDBI;
  globals->vmpOpt.Set(dop);
  globals->vmpOpt.Set(vop);  
  globals->vmapZoom   = 50.0f;                // 50 nautical miles for 800 pixels
  globals->vmapTrns   = MakeRGBA (0,0,0,128); // Transparency
  globals->vmapScrn.x = 800;
  globals->vmapScrn.y = 600;
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
  int plugin_allowed = 0;
  GetIniVar ("Sim", "allowDLLFiles", &plugin_allowed);
  globals->plugins.g_plugin_allowed = (plugin_allowed != 0);
/*
  // sdk: load and initialize the dll plugins
  if (globals->plugins.g_plugin_allowed) {
    if (globals->plugins_num = globals->plugins.On_LoadPlugins ()) {
      globals->plugins.On_InitPlugins ();
    } else
      globals->plugins_num = 0;
  }
  else
    //! plugins_num is used as a plugin flag along the code
    globals->plugins_num = 0;
*/
   globals->opal_sim = NULL;
   //MEMORY_LEAK_MARKER ("opal::createSimulator start")
   globals->opal_sim = opal::createSimulator ();
   globals->opal_sim->setMaxContacts(2);
   //MEMORY_LEAK_MARKER ("opal::createSimulator end")
   opal::Vec3r g (0.0f, 0.0f, -(GRAVITY_MTS));
   globals->opal_sim->setGravity (g);
   float step_size = ADJ_STEP_SIZE;          // 0.04f;
   GetIniFloat ("PHYSICS", "adjustStepSize", &step_size);
   globals->opal_sim->setStepSize (step_size); // was 0.02
   DEBUGLOG ("InitGlobalsNoPod step_size=%f", step_size);
   globals->simulation = false;
   char buffer_ [128] = {0};
   GetIniString ("PHYSICS", "aircraftPhysics", buffer_, 128);
   if (!strcmp (buffer_, "ufo")) globals->simulation = true;
   globals->sBar = 0;
   globals->status_bar_limit = 0.5f;
   GetIniFloat ("Sim", "statusBarDeltaSec", &globals->status_bar_limit);
   globals->fps_limiter = true;
   char buff_ [8] = {0};
   GetIniString ("Sim", "fpsLimiter", buff_, 8);
   if (!strcmp (buff_, "false")) globals->fps_limiter = false;

   // CAGING
   globals->caging_fixed_sped = false;
   globals->caging_fixed_sped_fval = 0.0f;
   globals->caging_fixed_moi = false;    
   globals->caging_fixed_pitch = false;
   globals->caging_fixed_pitch_dval = 0.0;
   globals->caging_fixed_roll = false;
   globals->caging_fixed_roll_dval = 0.0;
   globals->caging_fixed_yaw = false;
   globals->caging_fixed_yaw_dval = 0.0;
   globals->caging_fixed_alt = false;
   globals->caging_fixed_wings = true;
   if (IsSectionHere ("CAGING")) {
      int tmp_val = 0;
      float tmp_fval = 0.0f; 
      GetIniVar ("CAGING", "fixedSpeed", &tmp_val);
      globals->caging_fixed_sped = (tmp_val != 0);
      if (globals->caging_fixed_sped)
        GetIniFloat ("CAGING", "fixedSpeedVal", &globals->caging_fixed_sped_fval);
      //
      GetIniVar ("CAGING", "fixedMOI", &tmp_val);
      globals->caging_fixed_moi = (tmp_val != 0);
      //
      GetIniVar ("CAGING", "fixedAlt", &tmp_val);
      globals->caging_fixed_alt = (tmp_val != 0);
      //
      GetIniVar ("CAGING", "disconnectWings", &tmp_val);
      globals->caging_fixed_wings = (!tmp_val);
      //
      GetIniVar ("CAGING", "fixedPitch", &tmp_val);
      globals->caging_fixed_pitch = (tmp_val != 0);
      if (globals->caging_fixed_pitch) {
        GetIniFloat ("CAGING", "fixedPitchVal", &tmp_fval);
        globals->caging_fixed_pitch_dval = static_cast<double> (tmp_fval);
      }
      //
      GetIniVar ("CAGING", "fixedRoll", &tmp_val);
      globals->caging_fixed_roll = (tmp_val != 0);
      if (globals->caging_fixed_roll) {
        GetIniFloat ("CAGING", "fixedRollVal", &tmp_fval);
        globals->caging_fixed_roll_dval = static_cast<double> (tmp_fval);
      }
      //
      GetIniVar ("CAGING", "fixedYaw", &tmp_val);
      globals->caging_fixed_yaw = (tmp_val != 0);
      if (globals->caging_fixed_yaw) {
        GetIniFloat ("CAGING", "fixedYawVal", &tmp_fval);
        globals->caging_fixed_yaw_dval = static_cast<double> (tmp_fval);
      }
    }

    globals->random_flag = NO_RND_EVENTS;
    
    int randomEvents = 0;
    GetIniVar ("Sim", "randomEvents", &randomEvents);
    if (randomEvents) globals->random_flag |= RND_EVENTS_ENABLED;

    int num_of_autogen = 0;
    GetIniVar ("Graphics", "numOfAutogen", &num_of_autogen);
    globals->num_of_autogen = num_of_autogen;

    globals->iang.x = 0.0; 
    globals->iang.y = 0.0; 
    globals->iang.z = 0.0; 
    globals->dang.x = 0.0; 
    globals->dang.y = 0.0; 
    globals->dang.z = 0.0; 
    //MEMORY_LEAK_MARKER ("globalinit");
}


//======================================================================================
//  Cleanup settings in the Global data structure
//======================================================================================
void CleanupGlobals (void)
{
#ifdef _DEBUG
  DEBUGLOG ("CleanupGlobals start");
#endif
  /// \todo
  // sdk: cleanup situation call DLLEndSituation
  if (globals->plugins_num) globals->plugins.On_EndSituation ();

  // sdk: cleanup objects = DLLDestroyObject // 
  //if (globals->plugins_num) globals->plugins.On_DestroyObject (NULL);

  // sdk: clean-up dll plugins : call DLLKill ()
  //if (globals->plugins_num) globals->plugins.On_KillPlugins ();

  cleanup_ui ();   
  CleanupFonts();
  SAFE_DELETE (globals->fui);
  SAFE_DELETE (globals->cld);         // Delete cloud system
  SAFE_DELETE (globals->sit);
  SAFE_DELETE (globals->slw);         // Delete slew manager
  SAFE_DELETE (globals->kbd);         // Delete keymap manager;
//  ----Must be last ----------------------
  SAFE_DELETE (globals->tcm);         // Delete terrain before DBcache
  SAFE_DELETE (globals->dbc);         // Delete DB cache
  SAFE_DELETE (globals->wtm);         // Delete weather manager,
  SAFE_DELETE (globals->atm);         // Delete atmosphere, 
  SAFE_DELETE (globals->sqm);         // Delete SQL manager
  globals->nBitmap->ChangeType();
  SAFE_DELETE (globals->nBitmap);
	SAFE_DELETE (globals->dMap);
  SAFE_DELETE (globals->csp);
  SAFE_DELETE (globals->cap);
  SAFE_DELETE (globals->snd);
  SAFE_DELETE (globals->logWarning);
  SAFE_DELETE (globals->logTrace);
  SAFE_DELETE (globals->logTerra);
	SAFE_DELETE (globals->logScene);
//  ---------------------------------------
//   Clean up POD filesystems
  pshutdown (&globals->pfs);


#ifdef _DEBUG
  DEBUGLOG ("CleanupGlobals end");
#endif
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
#ifdef _DEBUG
  DEBUGLOG ("ShutdownAll start");
#endif

#ifdef _WIN32
  // Restore saved gamma ramp
  // if (!SetDeviceGammaRamp (hdc, savedGammaRamp))  WARNINGLOG ("Failed to set Win32 gamma ramp");
#endif // _WIN32

  // Clean up singletons
  SAFE_DELETE(globals->cum);
  CSkyManager::Instance().Cleanup ();

  // Clean up FUI manager
  globals->fui->Cleanup ();
  CTextureManager::Instance().Cleanup ();
  CDatabaseManager::Instance().Cleanup();

  // Save and clean up INI settings database
  SaveIniSettings ();
  UnloadIniSettings ();

  // Clean up global variables
  CleanupGlobals ();

  // sdk: clean-up dll plugins : call DLLKill ()
  if (globals->plugins_num) globals->plugins.On_KillPlugins ();

#ifdef _DEBUG
  DEBUGLOG ("ShutdownAll end");
#endif
  SAFE_DELETE (globals->logDebug);      // JS Must be the last if log is used
  SAFE_DELETE (globals);                // JS Must be the last if log is used

  // try to release as much heap allocated memory as possible
  // lc 051610
  // If successful, _heapmin returns 0; otherwise, the function returns 1
  // The _heapmin function minimizes the heap by releasing unused heap memory
  int heapmin_success = _heapmin ();
  #ifdef _DEBUG	
   // Check heap status
   int heapstatus = _heapchk ();
   {FILE *fp_debug;
    if (!(fp_debug = fopen ("__DDEBUG_heap.txt", "a")) == NULL)
    {
      fprintf (fp_debug, "_heapmin %s\n",
        heapstatus ? "hasn't worked" : "has succeeded");
      switch (heapstatus)
      {
      case _HEAPOK:
         fprintf (fp_debug, "OK - heap is fine\n");
         break;
      case _HEAPEMPTY:
         fprintf (fp_debug, "OK - heap is empty\n");
         break;
      case _HEAPBADBEGIN:
         fprintf (fp_debug, "ERROR - bad start of heap\n");
         break;
      case _HEAPBADNODE:
         fprintf (fp_debug, "ERROR - bad node in heap\n");
         break;
      }	    
	    fclose(fp_debug); 
   }}
  #endif
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
  glDisable (GL_COLOR_MATERIAL);
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
{ InitialProfile();
	globals->Frame	= 0;
	globals->aMax		= 1.0E+5;
  globals->magDEV = 0;
  globals->NbVTX  = 0;
  globals->snd = new CAudioManager();
  globals->sqm = new SqlMGR();
  globals->dbc = new CDbCacheMgr();
  // Initialize various subsystems.  These initialization functions may be
  //   dependent on INI settings and/or POD filesystem
  globals->kbd = new CKeyMap();
  globals->slw = new CSlewManager();
	globals->exm = new CExport();
  //---Latitude tables for Globe tiles and QGTs --------
  InitGlobeTileTable ();
  //-- Initialize singletons----------------------------
  CTextureManager::Instance().Init ();
  CDatabaseManager::Instance().Init();
  //----Initialize sky and weather ----------------------
  CSkyManager::Instance().Init();
  globals->atm = new CAtmosphereModelJSBSim();
  globals->wtm = new CWeatherManager();
  globals->fui->Init();
  //------Start terrain ---------------------------------
  globals->tcm = new TCacheMGR();
  globals->cld = new CCloudSystem();
  globals->wtm->Init();
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
{ TRACE("Start InitSimulation");
  //---- Set default area to Marseilles airport --------------------
  SPosition area = {18802.79, 156335.87, 78};
  globals->geop  = area;
  CVector   orie (0, 0, DegToRad(180));
  globals->iang  = orie;
  CVector   orid (0,0,180);
  globals->dang  = orid;
	//----Joystick ---------------------------------------------------
	CJoysticksManager::Instance().Init( );
  //------Load situation -------------------------------------------
  globals->sit = new CSituation ();
  TRACE("End of InitSimulation");
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
unsigned int   nFrames = 0;      // Sim frame rate counter
float          tFrames = 0.0f;   // Sim frame rate timer
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
  // Accumulate frame rate statistics every second
  nFrames++;
  tFrames += dRealT;
  if (tFrames > 1.0f) {
      frameRate = (float)nFrames / tFrames;
      tFrames = 0.0f;
      nFrames = 0;
  }
  //------------Update global clock ---------------------
  Frame++;
	globals->Frame = Frame;
  globals->clk->Update(dRealT);
  //-------- Update situation----------------------------
//
////#ifdef _DEBUG	
//  {	FILE *fp_debug;
//	  if(!(fp_debug = fopen("__DDEBUG_frame.txt", "a")) == NULL)
//	  {
//		  fprintf(fp_debug, "%f %u %f [R=%f S=%f]\n", 
//        dSimT, Frame, frameRate, 
//        dRealT, tmp_timerS);
//		  fclose(fp_debug); 
//  }	}
////#endif

  globals->sit->Timeslice (dSimT,Frame);

  // The global CSituation object contains all informations about the current
  //   simulation state, user vehicle, camera mode, etc.
  globals->sit->Draw ();

  // Draw UI only on main screen at this time
  // Draw UI components
  globals->fui->Draw ();
	//--- Draw master menu --------------------------------
	puDisplay();
  // draw cursor only on the window where the mouse is really on
  globals->cum->Draw ();
  return APP_SIMULATION;
}

/**
 *  Initialize splash screen
 *
 *  Initialize static splash screen image displayed during initial application
 *  startup.
 */
CTexture *tSplash = NULL;

void InitSplashScreen (void)
{
  tSplash = new CTexture();
  tSplash->LoadFromDisk ("Splash1.JPG", 0, 0, 0);
}

/**
 *  Redraw splash screen
 *
 *  This function is called by the window manager redraw() callback while the application is initializing.
 */
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

/**
 *  Cleanup exit screen.
 *
 *  Free resources used to display the application exit screen.
 */
void CleanupSplashScreen (void)
{
	SAFE_DELETE(tSplash);
}

/**
 *  Initialize exit screen
 *
 *  When realtime simulation ends and the application exits, it displays
 *  a static exit screen while cleaning up.  This function initializes
 *  the exit image and renders it to a drawing surface.
 */
static CTexture *tExit = NULL;

void InitExitScreen (void)
{
  // Load exit screen
  tExit = new CTexture();
  tExit->LoadFromDisk ("Legacy_off_2.jpg", 0, 0, 0);
}

/**
 *  Redraw exit screen.
 *
 *  This function is called by the window manager redraw() callback while the application is exiting.
 */
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

/**
 *  Cleanup exit screen.
 *
 *  Free resources used to display the application exit screen.
 */
void CleanupExitScreen (void)
{
	SAFE_DELETE(tExit);
}

/**
 *  Primary entry point for graceful application shutdown.
 */
void ExitApplication (void)
{ globals->mBox.DumpAll();
  // Gracefully shut down and clean up before exiting
  ShutdownAll ();
  CleanupWindowManager ();
  CleanupExitScreen ();

#ifdef MEMORY_LEAK_CRT
#if defined(_DEBUG) && defined(HAVE_CRTDBG_H)
  // Check for memory leaks
  HANDLE hfile = CreateFile ("Logs/MemoryLeaks.log",
                             GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL, NULL);
  _CrtSetReportMode (_CRT_WARN, _CRTDBG_MODE_FILE);
  _CrtSetReportFile (_CRT_WARN, hfile);
  _CrtSetReportMode (_CRT_ERROR, _CRTDBG_MODE_FILE);
  _CrtSetReportFile (_CRT_ERROR, hfile);
  _CrtMemDumpAllObjectsSince(&memoryState);
  CloseHandle (hfile);
#endif
#endif // MEMORY_LEAK_CRT

//  fprintf (stdout, "Press Enter key to exit...");
//  char s[80];
//  gets(s);

#if defined(_MSC_VER) && defined(_DEBUG)
  // Avoid MSVC6 crash on exit() when debugging
  TerminateProcess (GetCurrentProcess(), 0);
#else
  //exit (0);
  // had to change exit(0) with Terminateprocess to avoid
  // crash on exit in release mode
  TerminateProcess (GetCurrentProcess(), 0);
#endif // _WIN32 && _DEBUG
}

//
// pthread mutexes for warning, debug and gtfo logging
//
static pthread_mutex_t	mutexWarn, 
												mutexTrace,
												mutexScene,
												mutexDebug, 
												mutexGtfo;  // JSDEV* add trace

/**
 *  Initialize tracing options.
 */
void InitTraces(void)
{	bool trc	= IsSectionHere ("TRACE");
	int op	= 0;
  globals->ttr      = 0;
  globals->logTrace	= (trc)?(new CLogFile ("logs/Trace.log", "w")):(0);
  if (0 == trc) return;
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
{ double pxr = double(1) / 72;                // Pixel radius in inch
  double hpx = FN_FEET_FROM_INCH(pxr);        // Pixel radius in feet
  double nrp = 20;                            // Near clip plane
  globals = new SGlobals;	                    // Create global structure


  //------------- Initialize mutexes----------------------------------
  pthread_mutex_init (&mutexWarn,  NULL);
  pthread_mutex_init (&mutexDebug, NULL);
  pthread_mutex_init (&mutexGtfo,  NULL);
  pthread_mutex_init (&mutexTrace, NULL);		  // JSDEV* add trace
	pthread_mutex_init (&mutexScene, NULL);
  //--------- Load application settings from INI file -----------------
  LoadIniSettings ();

  // Initialize simulation situation (.SIT) to load
  if (argc == 2) {
    // First argument is .SIT filename
    strcpy (globals->sitFilename, argv[1]);
  } else {
    // No .SIT filename provided on command line; use INI settings
    GetIniString ("UI", "startupSituation", globals->sitFilename, (PATH_MAX-1));
    if (strlen (globals->sitFilename) == 0) {
      // No default startup situation specified in INI settings
      strcpy (globals->sitFilename, "Saved Simulations/Default.sit");
    }
  }

  //---Put find root foldeer here to open log file in final directory
  const char *flyRootFolder = GetIniValue("UI","flyRootFolder");
  //---If a test folder set it as current directory to allow debugging
  if (flyRootFolder == 0) {gtfo ("Cannot determine Fly! root folder");}
  //=========Init the global structure==================================
  strcpy(globals->FlyRoot,flyRootFolder);			// Root folder
  InitTraces();
  globals->logTerra	= new CLogFile ("logs/ChangedTiles.log", "a+");
  TRACE("TRACE FILE CREATED"); 
	globals->logScene = new CLogFile ("logs/Scenery.log", "w");
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
  globals->cum      = 0;
  globals->cap      = new CCameraRunway();
  globals->csp      = new CCameraSpot();
  globals->cam      = globals->csp;
  globals->ccm      = 0;
  globals->bTyp     = 0;                      // Blue print type
  //---Time of Day ---------------------------------------------------
  globals->tod      = 0;
  //---Chart map -----------------------------------------------------
  globals->MapName[0] = 0;
  globals->NulChar    = 0;
  //---Profile init --------------------------------------------------
  globals->noAWT    = 1;                      // No animated water
  globals->noTER    = 0;                      // Allow Terrain
  globals->noEXT    = 0;                      // Allow aircraft external drawing
	globals->noINT		= 0;											// Allow aircraft internal drawing
  globals->noAPT    = 0;                      // Allow Airport
  globals->noOBJ    = 0;                      // Allow Object
  globals->noMET    = 0;                      // Allow Meteo
  //---Terrain parameters --------------------------------------------
  globals->maxView  = 40;                     // Default Maximum view (miles)
  globals->highRAT  = 0.32;                   // Hight resolution (ratio of maxView)
  globals->skyDist  = FN_FEET_FROM_MILE(60);  // Sky distance
  //---SQL databases -------------------------------------------------
  globals->genDB = 0;
  globals->elvDB = 0;
  globals->seaDB = 0;
  globals->txyDB = 0;
  globals->m3dDB = 0;
  globals->texDB = 0;
  globals->objDB = 0;
  //---Master radio interface ----------------------------------------
  globals->cILS  =  0;
  //---Object pointer ------------------------------------------------
  globals->tim  = new CTimeManager();         // Time manager
  globals->clk  = new CClock();               // Global clock
  globals->tsb  = 0;                          // Test bed 
  globals->sit  = 0;                          // Situation
  globals->dbc	= 0;								          // Database cache
  globals->sqm  = 0;                          // SQL Manager
  globals->sql  = 0;                          // SQL thread
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
  globals->imp  = new CImport();              // Import manager
  globals->atm  = 0;                          // Atmosphere
  //---Aircraft objects -----------------------------------------------
  globals->pln  = 0;
  globals->ccm  = 0;                          // Camera manager 
  globals->pit  = 0;                          // Cockpit manager
  globals->gas  = 0;                          // Fuel system
  globals->pan  = 0;                          // Active pannel
  globals->wfl  = 0;                          // Fuel loadout
  globals->wld  = 0;                          // Load window
  globals->wpb  = 0;                          // Window probe
  globals->rdb  = 0;                          // Radio band
  //-------------------------------------------------------------------
  globals->wObj = 0;                          // Current object
  TRACE("Globals INITIALIZED");
  //-------------------------------------------------------------------
#ifdef MEMORY_LEAK_CRT
#if defined(_DEBUG) && defined(HAVE_CRTDBG_H)
  /// \todo Remove memory checkpoint and eliminate memory leaks prior to main()
  _CrtMemCheckpoint (&memoryState);
#endif
#endif // MEMORY_LEAK_CRT
  //--------------------------------------------------------------------
  // Initialize globals variables
  //-------------------------------------------------------------------
  InitGlobalsNoPodFilesystem ((char*)flyRootFolder);
  TRACE("InitGlobalsNoPodFilesystem OK");
  globals->appState = APP_SPLASH_SCREEN;
  //MEMORY_LEAK_MARKER ("CFuiManager")
  globals->fui = new CFuiManager();
  //MEMORY_LEAK_MARKER ("CFuiManager")
  TRACE("CFuiManager CREATED");
  // Initialize graphics engine and window manager
  InitWindowManager (argc, argv);
  TRACE("InitWindowManager OK");
  InitGraphics ();
  TRACE("InitGraphics OK");
  //---------------------------------------------------------------------------
  // Initialize the system pod filesystem.  The top-level reference to the POD
  //   filesystem is stored in the global variable struct, so this must
  //   be called after globals have been instantiated.
  //---------------------------------------------------------------------------
  pinit (&globals->pfs, "logs/systempod.log");		// JSDEV* add file name
  //----ADD POD FROM FLYII FOLDERS -------------------------------------------
  PFS *pfs = &globals->pfs;
  // Mount folders from Fly! II filesystem
  char folder[PATH_MAX];
  strcpy (folder, flyRootFolder);
  strcat (folder, "/SYSTEM");
  paddpodfolder (pfs, folder);
  //------------------------------------------
  strcpy (folder, flyRootFolder);
  strcat (folder, "/AIRCRAFT");
  paddpodfolder (pfs, folder, true);      ///< Add subfolders too
  //------------------------------------------
  strcpy (folder, flyRootFolder);
  strcat (folder, "/TAXIWAYS");
  paddpodfolder (pfs, folder);
  //------------------------------------------
  strcpy (folder, flyRootFolder);
  strcat (folder, "/SCENERY/SHARED");			// Must be Uppercases
  paddpodfolder (pfs, folder);

  //------ Add any disk files except pod -----
  padddiskfolder (pfs, flyRootFolder, "Art");
  padddiskfolder (pfs, flyRootFolder, "Aircraft");
  padddiskfolder (pfs, flyRootFolder, "Data");
  padddiskfolder (pfs, flyRootFolder, "Models");
  padddiskfolder (pfs, flyRootFolder, "Saved Simulations");
  padddiskfolder (pfs, flyRootFolder, "System");
  padddiskfolder (pfs, flyRootFolder, "Ui");
  padddiskfolder (pfs, flyRootFolder, "World");
  padddiskfolder (pfs, flyRootFolder, "Sound");

  //------------------------------------------------------------------------------
	//	JS: I suppress this because it is confusing
	//	We now must have only one root folder with legacy and we dont need
	//	to override files from somewhere else
	//------------------------------------------------------------------------------
  // Add disk files from Fly! Legacy folders; these files will override any in the
  //  Fly! II folders with the same name
	/*
  paddpodfolder (pfs, "./Aircraft");
  paddpodfolder (pfs, "./System");

  // Add aircraft subfolders
  strcpy (folder, "./Aircraft");
  ulDir* dirp = ulOpenDir (folder);
  if (dirp != NULL) {
    ulDirEnt* dp;
    while ( (dp = ulReadDir(dirp)) != NULL ) {
      if (dp->d_isdir) {
        // This is a sub-folder, recursively add it
        if ((strcmp (dp->d_name, ".") != 0) && (strcmp (dp->d_name, "..") != 0)) {
          char subfolder[PATH_MAX];
          strcpy (subfolder, folder);
          strcat (subfolder, "/");
          strcat (subfolder, dp->d_name);
          padddiskfolder (pfs, subfolder, "");
        }
      }
    }
    ulCloseDir(dirp);
  }

  // Add other Fly! Legacy disk folders
  padddiskfolder (pfs, ".", "Art");
  padddiskfolder (pfs, ".", "Charts");
  padddiskfolder (pfs, ".", "Data");
  padddiskfolder (pfs, ".", "Saved Simulations");
  padddiskfolder (pfs, ".", "System");
  padddiskfolder (pfs, ".", "UI");
  padddiskfolder (pfs, ".", "World");
	padddiskfolder (pfs, ".", "Updates");
  */
  // Initialize subsystems so that mouse and keyboard callbacks can be handled
 
  // sdk: load and initialize the dll plugins
  if (globals->plugins.g_plugin_allowed) {
    if (globals->plugins_num = globals->plugins.On_LoadPlugins ()) {
      globals->plugins.On_InitPlugins ();
    } else
      globals->plugins_num = 0;
  }
  else
    //! plugins_num is used as a plugin flag along the code
    globals->plugins_num = 0; 

  // sdk: init menu with added items if any 
  if (globals->plugins_num && !globals->plugins.On_InitGlobalMenus ()) {
    init_user_ui (); // was init_ui (); 082711
  } else {
    init_user_ui ();
  }
  //--- Initialize fonts and cursor manager -------------------
  InitFonts ();
  globals->cum = new CCursorManager();
  EnterWindowManagerMainLoop ();
  return 0;
}

#ifdef _WINDOWS

#include <shellapi.h>

/**
 *  Initial Windows application entry point.
 */
int APIENTRY WinMain (HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPSTR lpCmdLine,
                      int nShowCmd)
{
  main(__argc, __argv);
  return 0;
}
#endif // _WINDOWS

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
    vsprintf(usermsg, fmt, argp);
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
  log->Write ("Abnormal exit\n  File: %s  Line: %d\n  %s\n", file, m_line, usermsg);

  delete log;

#if defined(WIN32) && defined(_DEBUG)
  TerminateProcess (GetCurrentProcess(), 0);
#endif
  exit (0);
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
      vsprintf(usermsg, fmt, argp);
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
      vsprintf(usermsg, fmt, argp);
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