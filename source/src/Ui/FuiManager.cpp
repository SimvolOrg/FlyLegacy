/*
 * FuiManager.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2008 Chris Wallace
 * Copyright 2007      Jean Sabatier
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
 */

/*! \file FuiManager.cpp
 *  \brief Implements CFuiManager class
 */
#include "../Include/Fui.h"
#include "../Include/FuiUser.h"
#include "../Include/FuiParts.h"
#include "../Include/WinAbout.h"
#include "../Include/WinScenery.h"
#include "../Include/FuiPlane.h"
#include "../Include/FuiDLLs.h"
#include "../Include/FuiProbe.h"
#include "../Include/FuiKeyMap.h"
#include "../Include/Globals.h"   // For globals->pfs
#include "../Include/Utility.h"   // For stream functions
#include "../Include/WinControlAxis.h"
#include "../Include/PlanDeVol.h"
#include "../Include/Ui.h"
#include "../Include/WinCity.h"
#include "../Include/WinTaxiway.h"
#include "../Include/FuiOption.h"
#include "../Plugin/Plugin.h"
#include <malloc.h>
#include <map>

using namespace std;
//============================================================================
// CFuiManager
//============================================================================
CFuiManager::CFuiManager(void)
{ globals->fui = this;
  xPos    = 32;
  yPos    = 32;
  widn    = 0;
  wTop    = 0;
	wCap		= 0;
	xOBJ		= 0;
	notex		= 0;
}
//------------------------------------------------------------------------
//    Close Fui Manager
//------------------------------------------------------------------------
CFuiManager::~CFuiManager()
{	if (xOBJ)	glDeleteTextures(1,&xOBJ);
  std::map<Tag,CFuiTheme*>::iterator ra;
	for (ra = themeMap.begin(); ra != themeMap.end(); ra++) delete (*ra).second;
	themeMap.clear();
}
//------------------------------------------------------------------------
//    Init the FuiManager
//------------------------------------------------------------------------
#define NOTE_OFFSET  (60)
void CFuiManager::Init (void)
{
  // When parsing master theme schema, theme identifiers and filenames
  //   are stored in the following map.  Only after the master schema
  //   file has been processed are the individual themes instantiated
  std::map<Tag,string> themename;
  
  // Load themes
  char* masterFilename = "UI/THEMES/Default.SCH";
  PODFILE *p = popen (&globals->pfs, masterFilename);
  if (p != NULL) {
    // Begin parsing lines
    char s[256];
    pgets (s, 256, p);
    const char *delimiters = " \t";
    while (!peof(p)) {
      // Trim trailing whitespace
      TrimTrailingWhitespace (s);

      if ((strncmp (s, "\r\n", 2) == 0)) {	// empty line
        pgets (s, 256, p);
        continue;
	    }

      // Check for comment or blank line
      if ((strcmp (s, "//") == 0)) {
        pgets (s, 256, p);
        continue;
      }

      // Check first token
      char *token = strtok (s, delimiters);
      if (stricmp (token, "THEMEID") == 0) {
        // THEMEID token just identifies unique ID for this theme set
      } else if (stricmp (token, "THEMEFILE") == 0) {
        // THEMENAME specifies a new theme; parse theme ID and filename
        token = strtok (NULL, delimiters);
        Tag tag = StringToTag (token);
        token = strtok (NULL, delimiters);
        themename[tag] = token;
      } else if (stricmp (token, "END") == 0) {
        // END token indicates end of the schema list
        break;
      } else {
        gtfo ("CFuiManager : Unknown token %s in master theme file %s",
          token, masterFilename);
      }        
      
      // Get next line
      pgets (s, 256, p);
    }  
    pclose (p);
  } else {
    gtfo ("CFuiManager : Cannot open master theme file %s", masterFilename);
  }

  // Load the individual themes
  std::map<Tag,string>::iterator i;
  for (i = themename.begin(); i != themename.end(); i++) {
    DEBUGLOG ("Instantiating FUI theme: %s", i->second.c_str());
    CFuiTheme *theme = new CFuiTheme (i->second.c_str());
    themeMap[i->first] = theme;
  }
  themename.clear();

  // Initialize text popups

  slew = new CFuiTextPopup;
  slew->SetText ("Slew");
  slew->SetPosition (globals->cScreen->Width - 60, globals->cScreen->Height - NOTE_OFFSET);

  notep = new CFuiTextPopup;
  notep->SetPosition (globals->cScreen->Width - 300,globals->cScreen->Height - NOTE_OFFSET);

  note1 = new CFuiTextPopup;
  note1->SetPosition (12, globals->cScreen->Height - NOTE_OFFSET);
  
  notec  = new CFuiTextPopup;
  help   = new CFuiTextPopup;
  //---------------------------------------------------
  notec->RedBack();
  notec->SetText(" *** AIRCRAFT DAMAGED ***");
  notec->SetPosition (600, 40);
	//---------------------------------------------------
  tClick  = 0;
  //--- Assign one texture object ---------------
	glGenTextures(1,&xOBJ);

}
//------------------------------------------------------------------------
//  Screen is redimensionned
//------------------------------------------------------------------------
void CFuiManager::ScreenResize()
{ slew->SetPosition   (globals->cScreen->Width - 60, globals->cScreen->Height - NOTE_OFFSET);
  notep->SetPosition  (globals->cScreen->Width - 200,globals->cScreen->Height - NOTE_OFFSET);
  note1->SetPosition ( 12, globals->cScreen->Height - NOTE_OFFSET);
  return;
}
//========================================================================
/// Delete all FUI themes loaded in the constructor.
/// Close any windows that are open.
//========================================================================
void CFuiManager::Cleanup (void)
{// Delete the nul component

  // Close any windows if necessary before closing
  map <Tag,CFuiWindow*>::iterator windowIter;
  for (windowIter=winMap.begin(); windowIter!=winMap.end(); windowIter++) {
    CFuiWindow *window = windowIter->second;
    window->Close();
    delete window;
  }
  winMap.clear();
  winList.clear();
  // Delete themes
  std::map<Tag,CFuiTheme*>::iterator i;
  for (i=themeMap.begin(); i!=themeMap.end(); i++) {
    SAFE_DELETE (i->second);
  }
  themeMap.clear();

  // Delete special widgets
  SAFE_DELETE (slew);
  SAFE_DELETE (notep);
  SAFE_DELETE (note1);
  SAFE_DELETE (notex);
  SAFE_DELETE (notec);
  SAFE_DELETE (help);
}
//---------------------------------------------------------------------------------
//  Generate a Tag for windows that are dupplicated
//  Compute a new screen position
//---------------------------------------------------------------------------------
Tag CFuiManager::GenTag()
{  U_INT x;
   return GetaTag(&x,&x);
}
//---------------------------------------------------------------------------------
//  Generate a Tag for windows that are dupplicated
//  Compute a new screen position
//---------------------------------------------------------------------------------
Tag CFuiManager::GetaTag(U_INT *x, U_INT *y)
{  char txt[8];
   sprintf(txt,"N%03u",int(widn++));
   Tag idn  = StringToTag(txt);
   xPos  += 32;
   yPos  += 32;
   if (xPos > 800)  xPos = 10;
   if (yPos > 780)  yPos = 40;
	 *x = xPos;
	 *y = yPos;
   return idn;
}
//---------------------------------------------------------------------------------
//	Check if tool allowed
//---------------------------------------------------------------------------------
bool	CFuiManager::ToolNotAllowed()
{	bool ok = globals->pln->AllEngStopped();
  if (!ok)	return true;
	return (globals->aPROF.Has(PROF_TOOL) != 0);
}
//---------------------------------------------------------------------------------
//  Close all windows
//---------------------------------------------------------------------------------
void CFuiManager::CloseAllWindows()
{	map <Tag,CFuiWindow*>::iterator rp;
  for (rp=winMap.begin(); rp!=winMap.end(); rp++) {
    CFuiWindow *window = rp->second;
    window->Close();
    delete window;
  }
	return;
}
///==========================================================================================
/// Create a new window based on a .WIN template file.  This is the primary means
///   for the application to open a new FUI window.
///
/// JSDEV* modifications:
///
/// param tag          Unique identification tag for the window
//  opt is used by certain windows that support short formats
//        1 Dont create the windows if it exists
///       2 New notification circuit
//  NOTE: Use ToggleWindow to alternatively set ON/OFF
///========================================================================================== 
CFuiWindow* CFuiManager::CreateFuiWindow (Tag windowId, int opt)
{ //----Dont create any window before simulation -------------------------------
  if (globals->appState != APP_SIMULATION)  return 0;
  //----Dont create any window if one is already active -------------------------
  CFuiWindow *old = GetFuiWindow (windowId);
  if (old)  return 0;
  // Instantiate a new CFuiWindow object and add to the window map
  CFuiWindow *window = 0;
  switch(windowId)  {
   //----------------------------------------------------------------------------
   case FUI_WINDOW_CHART:
      window  = new CFuiChart (windowId, "UI/TEMPLATES/SectionalChart.WIN");
      window->MoveTo(10,120);
      break;
    //---GPS WINDOW ----------------------------------------------
    case FUI_WINDOW_KLN89:
      window  = new CFuiKLN89 (windowId, "UI/TEMPLATES/GPS.WIN");
      break;
    //----VECTOR MAP ---------------------------------------------
    case FUI_WINDOW_VECTOR_MAP:
		{	TC_SPOINT pos = globals->vmapPos;
      window  = new CFuiVectorMap (windowId, "UI/TEMPLATES/VectorMap.WIN");
			window->MoveTo(pos.x,pos.y);
      break;
		}
    //----NAVIGATION DETAIL --------------------------------------
    case FUI_WINDOW_DETAILS_NAVAID:
      windowId  = GenTag();
      window    = new CFuiNavDetail (windowId,"UI/TEMPLATES/DETAILSNAVAID.WIN",opt);
      window->MoveTo(xPos,yPos);
      break;
    //---AIRPORT DETAIL ------------------------------------------
    case FUI_WINDOW_DETAILS_AIRPORT:
      windowId  = GenTag();
      window    = new CFuiAptDetail (windowId,"UI/TEMPLATES/DetailsAirport.WIN",opt);
      window->MoveTo(xPos,yPos);
      break;
    //---FPLAN AIRPORT DETAIL ------------------------------------------
    case FUI_WINDOW_FPDETAIL_AIRPORT:
      windowId  = GenTag();
      window    = new CFuiAptDetail (windowId,"UI/TEMPLATES/FPDetailsAirport.WIN",opt);
      window->MoveTo(xPos,yPos);
      break;
    //---LIST OF FLIGHT PLAN --------------------------------------------
    case FUI_WINDOW_FPLAN_LIST:
      window  = new CFuiListPlan (windowId,"UI/TEMPLATES/FlightPlanList.WIN");
      break;
    //---FLIGHT PLAN LOG     --------------------------------------------
    case FUI_WINDOW_FPLAN_LOG:
      window  = new CFuiFlightLog (windowId,"UI/TEMPLATES/FlightPlanLog.WIN");
      break;
    //---GENERAL DIRECTORY -----------------------------------------------
    case FUI_WINDOW_WAYPOINT_DIRECTORY:
      window  = new CFuiDirectory (windowId,"UI/TEMPLATES/FlightPlannerDirectory.WIN");
      break;
    //---AIRCRAFT LIST ------------------------------------------
    case FUI_WINDOW_VEHICLE_SELECT:
      window  = new CWinPlane (windowId,"UI/TEMPLATES/VehicleSelect.WIN");
      break;
    //---FUEL DETAIL --------------------------------------------
    case FUI_WINDOW_VEHICLE_FUEL:
      window  = new CFuiFuel (windowId,"UI/TEMPLATES/VehicleFuel.WIN");
      break;
    //---VARIABLE LOAD -------------------------------------------
    case FUI_WINDOW_VEHICLE_LOAD:
      window  = new CFuiLoad (windowId,"UI/TEMPLATES/VehicleLoad.WIN");
      window->MoveTo(150,130);
      break;
    //--- CENTER OF GRAVITY -------------------------------------
    case FUI_WINDOW_VEHICLE_CG:
      window  = new CFuiWinCG (windowId,"UI/TEMPLATES/VehicleCG.WIN");
      window->MoveTo(125,200);
      break;
    //---VEHICLE OPTIONS-----------------------------------------
    case FUI_WINDOW_VEHICLE_OPTIONS:
      window  = new CFuiVehOption(windowId,"UI/TEMPLATES/NOFILEYET.WIN");
      window->MoveTo(500,130);
      break;
    //---Globals OPTIONS-----------------------------------------
    case FUI_WINDOW_GLOBAL_OPTIONS:
//      window  = new CFuiOptions(windowId,"UI/TEMPLATES/GLOBALOPTIONS.WIN");
//      window->MoveTo(600,150);
      break;
    //---Globals OPTIONS-----------------------------------------
    case FUI_WINDOW_CAMERA_CONTROL:
      window  = new CFuiCamControl(windowId,"UI/TEMPLATES/CAMERACONTROL.WIN");
      window->MoveTo(1036,840);
      break;
		//---Terra editor -------------------------------------------
    case FUI_WINDOW_TEDITOR:
			if (ToolNotAllowed())		return 0;
      window  = new CFuiTED(windowId,"UI/TEMPLATES/TERRAEDITOR.WIN");
      window->MoveTo(580,50);
      break;
		//---City editor-----------------------------------------
    case FUI_WINDOW_CITY_EDIT:
			if (ToolNotAllowed())		return 0;
      window  = new CFuiSketch(windowId,"UI/TEMPLATES/CITY_EDITOR.WIN");
      window->MoveTo(1036,100);
      break;
		//---Taxiway editor-----------------------------------------
    case FUI_WINDOW_TAXIWAY:
			if (ToolNotAllowed())		return 0;
      window  = new CFuiTaxi(windowId,"UI/TEMPLATES/TAXY_EDITOR.WIN");
      window->MoveTo(900,100);
      break;
    //---- TERRA BROWSER --------------------------------------
    case FUI_WINDOW_TBROS:
      if (ToolNotAllowed())		return 0;
      window  = new CFuiTBROS (windowId, "UI/TEMPLATES/TBROS.WIN");
      window->MoveTo(20,100);
      break;
    //---- MODEL BROWSER --------------------------------------
    case FUI_WINDOW_MBROS:
      if (ToolNotAllowed())		return 0;
      window  = new CFuiMBROS (windowId, "UI/TEMPLATES/MBROS.WIN");
      window->MoveTo(20,200);
      break;
    //---SUBSYSTEM PROBE-----------------------------------------
    case FUI_WINDOW_PROBE:
      window  = new CFuiProbe (windowId,"UI/TEMPLATES/Probe.WIN");
      window->MoveTo(660,50);
      break;
    //---SUBSYSTEM PLOT-----------------------------------------
    case FUI_WINDOW_PLOT:
      window  = new CFuiPlot (windowId,"UI/TEMPLATES/NOFILEYET.WIN");
      window->MoveTo(30,30);
      break;
    //---- STATISTIC ---------------------------------------------
    case FUI_WINDOW_STATS:
      window  = new CFuiStat (windowId, "UI/TEMPLATES/STATS.WIN");
      window->MoveTo(20,100);
      break;
    //---KEYBOARD MAPPING ---------------------------------------
    case FUI_WINDOW_OPTIONS_KEYS_BUTTONS:
      window  = new CFuiKeyMap (windowId,"UI/TEMPLATES/OptionsKeysButtons.WIN");
      break;
    //---JOYSTICK MANAGEMENT ----------------------------------------
    case FUI_WINDOW_OPTIONS_SETUP_AXES:
      window  = new CFuiAxis (windowId,"UI/TEMPLATES/OptionsSetupAxes.WIN");
      break;
		//---OSM TUNING  ----------------------------------------
    case FUI_WINDOW_OSM_TUNE:
      window  = new CWinOSM (windowId,"UI/TEMPLATES/WinOSM.WIN");
			window->MoveTo(10,40);
      break;
    //---AIRCRAFT CHECKLIST -----------------------------------
    case FUI_WINDOW_CHECKLIST:
      window  = new CFuiCkList(windowId,"UI/TEMPLATES/VehicleChecklist.WIN");
      window->MoveTo(20,60);
      break;
    //---PID TUNING -------------------------------------------
    case FUI_WINDOW_PIDTUNE:
      window  = new CFuiPID(windowId,"UI/TEMPLATES/PIDTUNE.WIN");
      break;
    //--- WINDOW STRIP ----------------------------------------
    case FUI_WINDOW_STRIP:
      windowId  = GenTag();
      window    = new CFuiStrip(windowId,"Ui/TEMPLATES/WINSTRIP.WIN");
      window->MoveTo(xPos,yPos);
      break;
    //---SIM DEBUG = CAGING   ---------------------------------
    case FUI_WINDOW_CAGING:
      window  = new CCageDebug(windowId,"UI/TEMPLATES/DebugCaging.WIN", NULL);
      break;
    //---WIND MANAGEMENT  -------------------------------------
    case FUI_WINDOW_WEATHER_WINDS:
      window  = new CFuiWind (windowId, "UI/TEMPLATES/WeatherWinds.WIN");
      break;
    //---Quit CONFIRMATION-------------------------------------
    case FUI_WINDOW_QUIT:
      // Program quit confirmation dialog
      window = new CFuiConfirmQuit (windowId, "UI/TEMPLATES/FileQuit.win");
      break;
    //---Error message ----------------------------------------
			/*
    case FUI_WINDOW_ERMSG:
      windowId  = GenTag();
      window		= new CFuiErrorMSG (windowId, "UI/TEMPLATES/ERRORMSG.win");
			window->MoveTo(xPos,yPos);
      break;
			*/
    case FUI_WINDOW_SITUATION_LOAD:
      window  = new CFuiWindow (windowId, "UI/TEMPLATES/FileLoadSituation.WIN");
      break;
    case FUI_WINDOW_MANAGE_DLL:
      window  = new CFuiDLLsWindow (windowId, "Ui/TEMPLATES/FileManageDLL.win");
      break;
    case FUI_WINDOW_FRAME_RATE:
      window  = new CFuiWindow (windowId, "UI/TEMPLATES/FrameRate.WIN");
      break;
    case FUI_WINDOW_OPTIONS_VIDEO:
      window  = new CFuiWindow (windowId, "UI/TEMPLATES/OptionsVideo.WIN");
      break;
    case FUI_WINDOW_OPTIONS_AUDIO:
      window  = new CFuiWindow (windowId, "UI/TEMPLATES/OptionsAudio.WIN");
      break;
    case FUI_WINDOW_OPTIONS_DATE_TIME:
      window  = new CFuiWindow (windowId, "UI/TEMPLATES/OptionsDateTime.WIN");
      break;
    case FUI_WINDOW_OPTIONS_STARTUP:
      window  = new CFuiWindow (windowId, "UI/TEMPLATES/OptionsStartup.WIN");
      break;
    case FUI_WINDOW_OPTIONS_SCENERY:
      window  = new CFuiWindow (windowId, "UI/TEMPLATES/OptionsScenery.WIN");
      break;
    case FUI_WINDOW_TELEPORT:
      window  = new CFuiTeleport (windowId, "UI/TEMPLATES/VehicleTeleport.WIN");
      break;
    case FUI_WINDOW_VEHICLE_INFO:
      window  = new CFuiWindow (windowId, "UI/TEMPLATES/VehicleInfo.WIN");
      break;
    case FUI_WINDOW_WEATHER_ALMANAC:
      window  = new CFuiWindow (windowId, "UI/TEMPLATES/WeatherAlmanac.WIN");
      break;
    case FUI_WINDOW_WEATHER_CLOUDS:
      window  = new CFuiWindow (windowId, "UI/TEMPLATES/WeatherClouds.WIN");
      break;
    case FUI_WINDOW_WEATHER_SKY_TWEAKER:
      window  = new CFuiWindow (windowId, "UI/TEMPLATES/WeatherSkyTweaker.WIN");
      window->MoveTo(120,200);
      break;
    case FUI_WINDOW_WEATHER_OVERVIEW:
      window  = new CFuiWeatherView (windowId, "UI/TEMPLATES/WeatherOverview.WIN");
      break;

    case FUI_WINDOW_DEBUG_SIM:
      // Simulation debugging window
      window  = new CSimDebugManager (windowId,"UI/TEMPLATES/SimDebug.WIN");
      break;
    case FUI_WINDOW_DISPLAY_DBG:
      // Wing section debugging window
      window  = new CSimDebugWings (windowId, "UI/TEMPLATES/DebugDisplay.WIN");
      break;
    case FUI_WINDOW_ALERT_MSG:
      // API alert message -----------------------------------------
      window  = new CFuiWindow (windowId, "Ui/TEMPLATES/ALERT.WIN");
      break;
    case FUI_WINDOW_HELP_ABOUT:
      window  = new CFuiAbout(windowId, "UI/TEMPLATES/HelpAbout.WIN");
			window->MoveTo(350,200);
      return 0;

    default:
      return 0;
  }
  //----------------------------------------------------------------------
  // JS NOTE: Window will be activated when surface is created
  //  thus at  ReadFinished() time.  This to avoid a lot of problems
  //           while window is created (like at mouse click)
  //-----------------------------------------------------------------------
  return window;
}
//------------------------------------------------------------------------------
// Create a window for export message
//------------------------------------------------------------------------------
void CFuiManager::ExportMessage(char *fn)
{ char txt[210];
	_snprintf(txt,127,"YOU ARE CONFIGURED TO IMPORT IN %s",fn);
	txt[200] = 0;
	notex  = new CFuiTextPopup;
 	notex->RedBack();
	notex->SetText(txt);
	notex->SetPosition (200,60);
	return;
}
//------------------------------------------------------------------------------
// Activate the window
//  NOTE: The windows is just added to the windows lit with the state = INIT
//        Real activation will occurs in the Draw routine to avoid iterator
//        perturbation.
//------------------------------------------------------------------------------
void CFuiManager::ActivateWindow(CFuiWindow *win)
{ Tag wid = win->GetWinId(); 
	winMap[wid] = win;
  return;
}
//------------------------------------------------------------------------------
// Add fui Windows from SDK
//------------------------------------------------------------------------------
void CFuiManager::AddFuiWindow (Tag wid, CFuiWindow *win)
{ winMap[wid] = win;
  winList.push_back(win);
  return;
}
//------------------------------------------------------------------------------
// Hidde window: remove from active list
//------------------------------------------------------------------------------
void CFuiManager::HideWindow(CFuiWindow *win)
{ RemoveFromDisplay(win);
  return;
}
//------------------------------------------------------------------------------
// Show window: put in display list
//------------------------------------------------------------------------------
void CFuiManager::ShowWindow(CFuiWindow *win)
{ winList.push_back(win);
  return;
}
//------------------------------------------------------------------------------
/// Destroy the window identified by the unique tag.   Applications may call this
///   function to destroy a window that they created; it may also be called by the
///   FuiManager in response to the user clicking the Cancel, OK, or Close buttons
///
/// @param tag   Unique identification tag for the window
//------------------------------------------------------------------------------
bool CFuiManager::DestroyFuiWindow (Tag wid)
{ CFuiWindow *window = GetFuiWindow (wid);
  if (window == 0) return true;
  // Send event notification to window handler
  window->NotifyChildEvent('sysb','sysb',EVENT_CLOSEWINDOW);
	return true;
}

///
/// Determine whether the window with the specified tag has been created
///
/// \param tag   Unique identification tag for the window
/// \returns bool  true if window is created; false if not
///
bool CFuiManager::IsWindowCreated (Tag windowId)
{
  std::map<Tag,CFuiWindow*>::iterator it = winMap.find(windowId);
  return (it != winMap.end());
}
//------------------------------------------------------------------------------
// Create window if not already created
//------------------------------------------------------------------------------
CFuiWindow* CFuiManager::CreateOneWindow(Tag id,int lim)
{ if (IsWindowCreated(id))    return 0;
  return CreateFuiWindow(id,lim);
} 
//------------------------------------------------------------------------------
// Toggle the state of the given window
//------------------------------------------------------------------------------
void CFuiManager::ToggleFuiWindow (Tag windowId)
{ if (IsWindowCreated (windowId))   DestroyFuiWindow (windowId);
  CreateFuiWindow (windowId);
  return;
}
//-------------------------------------------------------------------------------
/// Prepare Fui Manager for Drawing
//-------------------------------------------------------------------------------
void CFuiManager::EnterDrawing()
{ //--- Set projection matrix to 2D screen size ----------------------
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D (0, globals->cScreen->Width, globals->cScreen->Height, 0);
  //--- Clear modelview matrix ----------------------------------------
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  //--- Set up OpenGL drawing state -----------------------------------
  glPushAttrib (GL_ALL_ATTRIB_BITS);
  glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
	//--- dont use Depth buffer -----------------------------------------
  glDisable(GL_LIGHTING);								// No lighting
  glDepthMask(GL_FALSE);                // Do not write in depth buffer
  glDisable (GL_DEPTH_TEST);            // Dont check Z depth
  //glDisable (GL_COLOR_MATERIAL);
	glEnable(GL_TEXTURE_2D);
  glPolygonMode(GL_FRONT,GL_FILL);
  glBindTexture(GL_TEXTURE_2D,xOBJ);
	//-------------------------------------------------------------------
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

  return;
}
//-------------------------------------------------------------------------------
/// Leave drawing state
//-------------------------------------------------------------------------------
void CFuiManager::ExitDrawing()
{   // Restore original OpenGL state
  glPopAttrib();
	glPopClientAttrib();
  // Restore original modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  // Restore original projection matrix
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  // Check for an OpenGL error
  GLenum e = glGetError ();
  if (e != GL_NO_ERROR) {
    WARNINGLOG ("CFuiManager::Draw - GL Error 0x%04X", e);
  }
  return;
}
//-------------------------------------------------------------------------------
/// Redraw all FUI components on the screen; this method is called on every
///   screen refresh.
//-------------------------------------------------------------------------------
void CFuiManager::Draw ()
{ if (tClick > 0)   tClick -= globals->dRT;
  std::map <Tag,CFuiWindow*>::iterator w;
  std::map <Tag,CFuiWindow*>::iterator e;
  for (w = winMap.begin(); w!= winMap.end();)
  { e = w++;
    CFuiWindow *win = e->second;
		win->CheckProfile(FUI_WINDOW_DRAW);
    switch (win->GetState()) {
      case FUI_WINDOW_CLOSED:
      {   RemoveFromDisplay(win);
          winMap.erase(e);
					if (win == wCap)	wCap = 0;
          delete win;
          continue;
      }
      case FUI_WINDOW_INIT:
        { win->SetState(FUI_WINDOW_OPEN);
          winList.push_back(win);
        }
    }
  }
  EnterDrawing();
  // sdk: Draw DLL windows plugins
  globals->plugins->DrawDLLWindow ();
  //------------------------------------------------------
  // Draw windows following priority deque
  //  NOTE: The top windows is the last displayed
  //------------------------------------------------------
  wTop  = 0;
  std::vector<CFuiWindow*>::iterator it;
  for (it = winList.begin(); it != winList.end(); it++)
  { CFuiWindow *win = (*it);
    wTop            = win;
    //---Dispatch according to state ------------------
    switch (win->GetState()) {
      case FUI_WINDOW_INIT:
         continue;

      case FUI_WINDOW_OPEN:
      case FUI_WINDOW_MOVE:
          // Window is open; draw it
				  if (win->NotModal())	win->TimeSlice();
          win->Draw ();
          continue;
    }
  }
 
  //------Check for additional process -------------------
  // Draw Slew / Pause indicator if applicable
  if (globals->slw->IsEnabled())       slew->DrawIt ();
  //--- Draw user messages -------------------------------
	if (notep->IsActive())		notep->Draw();
  if (note1->IsActive())    note1->Draw();
  if (notec->IsActive())    notec->DrawIt();
  if (help->IsActive())     help->Draw();
	//------------------------------------------------------
	if (notex)	notex->DrawIt();
  //--- Draw Radio communications ------------------------
  globals->rdb.Draw();
  ////------------------------------------------------------
  ExitDrawing();
  return;
}
//-------------------------------------------------------------------------------
//  Draw Notice only
//-------------------------------------------------------------------------------
void CFuiManager::DrawOnlyNotices()
{ EnterDrawing();
  if (note1->IsActive())  note1->Draw ();
  ExitDrawing();
  return;
}
//-------------------------------------------------------------------------------
/// Return a pointer to the CFuiTheme identified by the given tag.
///
/// \param tag  Unique identification tag for the desired theme
/// \return     Pointer to the theme or NULL if not found
//-------------------------------------------------------------------------------
CFuiTheme *CFuiManager::GetTheme (Tag tag)
{ CFuiTheme *rc = NULL;
  std::map<Tag,CFuiTheme*>::iterator i = themeMap.find(tag);
  if (i != themeMap.end()) {
    rc = i->second;
  }
  return rc;
}

//-------------------------------------------------------------------------------
/// Return a pointer to the named CFuiThemeWidget in the theme identified by
///   the given tag.
///
/// \param tag  Unique identification tag for the desired theme
/// \param name Name of the desired widget
/// \return     Pointer to the theme widget or NULL if not found
//-------------------------------------------------------------------------------
CFuiThemeWidget *CFuiManager::GetThemeWidget (Tag tag, string name)
{
  CFuiThemeWidget *rc = NULL;

  CFuiTheme *theme = GetTheme (tag);
  if (theme != NULL) {
    rc = theme->GetWidget (name);
  }
  return rc;
}
///============================================================================
/// Display an application-specified message to the user in a pop-up box for
///   the specified time
///
/// \param text     Text to be displayed, maximum 80 chars
/// \param timeout  Message timeout in seconds
///=============================================================================
void CFuiManager::DrawNoticeToUser (char *text, float time)
{ note1->SetText (text);
  note1->SetActive(time);
}
//------------------------------------------------------------------------------
//	Display pilot message
//------------------------------------------------------------------------------
void CFuiManager::PilotToUser()
{ notep->SetActive(10);
	notep->DrawTheText();
	return;
}
//------------------------------------------------------------------------------
/// Display Help for gauge
//------------------------------------------------------------------------------
void CFuiManager::DrawHelp (float time,int x, int y)
{ //--------------------------------------------------
	CVehicleObject *veh		= globals->pln;
  CCockpitManager *pit	= (veh)?(veh->GetPIT()):(0);
	if (0 == pit)		return;
	CPanel* pan =		pit->GetCurrentPanel();
  if (0 == pan)		return;
	help->DrawTheText();
  int     sh  = globals->cScreen->Height;
  int     ph  = pan->GetHeight();

  x -= pan->GetXOffset();
  y  = sh -  pan->GetYOffset() - ph + y ;
  help->MoveTo(x,y);
  help->SetActive(time);
  return;
}
//------------------------------------------------------------------------------
/// Display a error window
//------------------------------------------------------------------------------
void CFuiManager::DialogError(char *msg,char *from)
{	CFuiErrorMSG *win = new CFuiErrorMSG(FUI_WINDOW_ERMSG,0);
  char *ttl = "ERROR";
  if (from)	ttl = from;
	win->SetTitle(ttl);
	win->Display(msg);
	return;
}
///=============================================================================
/// Display an application-specified message to the user in a pop-up box for
///   the specified time
///
/// \param text     Text to be displayed, maximum 80 chars
/// \param timeout  Message timeout in seconds
///=============================================================================
void DrawNoticeToUser (char* text, float timeout)
{
  globals->fui->DrawNoticeToUser (text, timeout);
}

//-------------------------------------------------------------------------------
/// Return a pointer to the window identified by the given tag.  The window must
///   be instantiated in the list of active FUI windows.
///
/// \param tag   Unique identification tag for the window
/// \return      Pointer to window or NULL if not found
//-------------------------------------------------------------------------------
CFuiWindow *CFuiManager::GetFuiWindow (Tag tag)
{ std::map<Tag,CFuiWindow*>::iterator i = winMap.find(tag);
  return (i != winMap.end())?(i->second):(0);
}

//-------------------------------------------------------------------------------
/// Inform FUI of mouse location
///
/// \param mx        X screen location of mouse (0=left)
/// \param my        Y screen location of mouse (0=top)
//-------------------------------------------------------------------------------
bool CFuiManager::MouseMove (int mx, int my)
{ std::vector<CFuiWindow*>::reverse_iterator w;
  for (w = winList.rbegin(); w != winList.rend(); w++)
  { CFuiWindow *win = (*w);
    if (win->MouseMove (mx, my))         return true;
  }
  return false;
}

//--------------------------------------------------------------------
// Inform TOP FUI of keyboard input key
// 
//  param key    ASCII code of key pressed
//  NOTE: Only the TOP Fui receive the keyboard
///-------------------------------------------------------------------
bool  CFuiManager::KeyboardInput(U_INT key)
{ if (0 == wTop)                          return false;
  CFuiWindow *win = wTop;
  return (win->KeyboardInput(key));
}
//--------------------------------------------------------------------
//	Mouse is captured by a window, send mouse to it
//--------------------------------------------------------------------
bool CFuiManager::MouseToWind(int mx, int my, EMouseButton bt)
{ if (0 == wCap)			return false;
	if (wCap != wTop)		MoveOnTop(wCap);
  return wCap->MouseCapture(mx, my, bt);
}
///-------------------------------------------------------------------
/// Inform FUI of mouse button click action and change focus if required
/// 
/// \param mx        X screen location of mouse (0=left)
/// \param my        Y screen location of mouse (0=top)
/// \param button    Mouse button clicked
///-------------------------------------------------------------------
bool CFuiManager::MouseClick (int mx, int my, EMouseButton bt)
{ if ((tClick > 0) && (bt == MOUSE_BUTTON_LEFT))  return DoubleClick(mx,my,bt);
  CFuiWindow *wck  = 0;
  CFuiWindow *win  = 0;
  std::vector<CFuiWindow *>::reverse_iterator  ip;
  for (ip = winList.rbegin(); ip != winList.rend(); ip++)
  { wck = (*ip);
    // Check for mouse hit on the window; click event will be sent
    //   to the window outside of this loop, to avoid invalidating
    //   the iterator if the mouse click results in a new window
    //   being created.
    if (!wck->WindowHit (mx, my))             continue;
    win = wck;
    // Put this window in front if mouse click handled
    // and window is not on top of list
    if (win != wTop) MoveOnTop(win);
    break;
    }
  //--- If a window was hit by the mouse click, send the click event to it
  if (win == 0)					return  MouseToWind(mx, my, bt);                            
	//---This is a simple click. Arm timer -------------------
	win->MouseClick (mx, my, bt);
	tClick = 0.25F;
	return true;
}
//-------------------------------------------------------------------
// A double click was detected.  Send to last clicked windows
//  which is the window on top
//-------------------------------------------------------------------
bool CFuiManager::DoubleClick(int mx,int my,EMouseButton bt)
{ if (0 == wTop)  return false;
  return  wTop->DoubleClick(mx,my,bt);
}
///-------------------------------------------------------------------
/// Inform FUI of mouse button release action
///
/// \param x         X screen location of mouse (0=left)
/// \param y         Y screen location of mouse (0=top)
/// \param button    Mouse button released
/// Send event to the last clicked window
///-------------------------------------------------------------------
bool CFuiManager::MouseStopClick (int mx, int my, EMouseButton bt)
{ if (0 == wTop)  return false;
  return wTop->MouseStopClick (mx,my,bt);
}
///--------------------------------------------------------------------------
/// Set the text of a FUI widget.
///
/// \param window    Unique identification tag for the window
/// \param component Unique identification tag for the component in the window
/// \param text      New text for the widget, maximum 64 chars
///--------------------------------------------------------------------------
void CFuiManager::SetComponentText (Tag window, Tag component, char* text)
{
  CFuiWindow *w = GetFuiWindow (window);
  if (w != NULL) {
    CFuiComponent *c = w->GetComponent (component);
    if (c != NULL) {
      c->SetText (text);
    }
  }
}
///--------------------------------------------------------------------------
//	Set big font
//---------------------------------------------------------------------------
void	CFuiManager::SetBigFont()
{	note1->ChangeFont(&globals->fonts.ftmono20); }
///--------------------------------------------------------------------------
//	Reset Normal font
//---------------------------------------------------------------------------
void	CFuiManager::ResetFont()
{	note1->ChangeFont(&globals->fonts.ftmono8); }

///--------------------------------------------------------------------------
/// Dump FuiManager details to a file for debugging
///
/// \param f   stdio file to write debug output to
///--------------------------------------------------------------------------
void CFuiManager::Print (FILE *f)
{
  std::map<Tag,CFuiTheme*>::iterator i;
  for (i=themeMap.begin(); i!=themeMap.end(); i++) {
    i->second->Print (f);
    fprintf (f, "\n");
  }
}

///--------------------------------------------------------------------------
/// Translation entry of FUI widget name to EFuiComponentTypes enumerated value
///--------------------------------------------------------------------------
typedef struct {
  char                  *name;
  EFuiComponentTypes    type;
} SFuiNameLookup;

///--------------------------------------------------------------------------
/// Translation table between FUI widget names and EFuiComponentTypes enumerated values
///--------------------------------------------------------------------------
static SFuiNameLookup nameLookupTable[] =
{
  {"Window",               COMPONENT_WINDOW},
  {"Button",               COMPONENT_BUTTON},
  {"DoubleButton",         COMPONENT_DOUBLE_BUTTON},
  {"DefaultButton",        COMPONENT_DEFAULT_BUTTON},
  {"PopupMenu",            COMPONENT_POPUPMENU},
  {"MenuBar",              COMPONENT_MENUBAR},
  {"Menu",                 COMPONENT_MENU},
  {"CheckBox",             COMPONENT_CHECKBOX},
  {"RadioButton",          COMPONENT_RADIOBUTTON},
  {"Label",                COMPONENT_LABEL},
  {"Line",                 COMPONENT_LINE},
  {"Box",                  COMPONENT_BOX},
  {"Picture",              COMPONENT_PICTURE},
  {"Scrollbar",            COMPONENT_SCROLLBAR},
  {"Slider",               COMPONENT_SLIDER},
  {"Groupbox",             COMPONENT_GROUPBOX},
  {"Gauge",                COMPONENT_GAUGE},
  {"List",                 COMPONENT_LIST},
  {"ScrollArea",           COMPONENT_SCROLLAREA},
  {"TextField",            COMPONENT_TEXTFIELD},
  {"TextArea",             COMPONENT_TEXTAREA},
  {"Progress",             COMPONENT_PROGRESS},
  {"Map",                  COMPONENT_MAP},
  {"TabPage",              COMPONENT_TABPAGE},
  {"TabButton",            COMPONENT_TABBUTTON},
  {"TabControl",           COMPONENT_TABCONTROL},
  {"Graph",                COMPONENT_GRAPH},
  {"DLLView",              COMPONENT_DLLVIEW},
  {"Disclosure",           COMPONENT_DISCLOSURE},
  {"Canvas",               COMPONENT_CANVAS},
  {"RunwayCanvas",         COMPONENT_RUNWAY_CANVAS},
  {"Marquee",              COMPONENT_MARQUEE},
  {"HorizMarquee",         COMPONENT_HORIZ_MARQUEE},
  {"WindowNoResize",       COMPONENT_WINDOW_NORESIZE},
  {"WindowTitle",          COMPONENT_TITLE},
  {"PaletteWindow",        COMPONENT_PALETTE_WINDOW},
  {"PaletteWindowTitle",   COMPONENT_PALETTE_WINDOW_TITLE},
  {"WindowCloseButton",    COMPONENT_CLOSE_BUTTON},
  {"WindowMinimizeButton", COMPONENT_MINIMIZE_BUTTON},
  {"WindowZoomButton",     COMPONENT_ZOOM_BUTTON}
};

///--------------------------------------------------------------------------
// Protected methods
///--------------------------------------------------------------------------

CFuiComponent* CFuiManager::FindComponent (Tag windowID, Tag id)
{
  CFuiComponent *rc = NULL;

  CFuiWindow *window = GetFuiWindow (windowID);
  if (window != NULL) {
    rc = window->GetComponent (id);
  }

  return rc;
}

///--------------------------------------------------------------------------
/// Switch the specified window to highest drawing priority (in front of all)
///
///--------------------------------------------------------------------------
void CFuiManager::MoveOnTop(CFuiWindow *win)
{ // First remove it
  RemoveFromDisplay(win);
  // Then push in front
  winList.push_back(win);
	wTop	= win;
}
///--------------------------------------------------------------------------
/// Delete window from drawing list
///
/// \param windowID   Unique identification tag for the window
///--------------------------------------------------------------------------
void CFuiManager::RemoveFromDisplay(CFuiWindow *win)
{ std::vector<CFuiWindow *>::iterator w = winList.begin();
  while (w != winList.end())
  {
    if(*w == win)
    { winList.erase(w);
      return;
    }
    w++;
  }
  if (win == wTop)  wTop = 0;
  return;
}

//======================END OF FILE ======================================================================

