/*
 * Menu.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
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

#include "../Include/FlyLegacy.h"
#include "../Include/Globals.h"
#include "../Include/Ui.h"
#include "../Include/Fui.h"
#include "../Include/Database.h"
#include "../Include/Utility.h"
#include "../Include/TimeManager.h"
#include "../Include/Situation.h"
#include "../Include/Menu.h" // sdk:  //  
#include "../Include/Joysticks.h"
#include "../Include/Sky.h"
#include "../Include/Export.h"
#include "../Include/Import.h"
//==================================================================================
extern Tag    snexttag (char* s, int maxLength, SStream *stream);

// GLUT callback for secondary view
extern void reshape2       (int w, int h);
extern void redraw2        (void);
extern void motion2        (int x, int y);
extern void passive_motion2(int x, int y);
extern void mouse2         (int button, int updown, int x, int y);
extern void keyboard2      (unsigned char key, int x, int y);
extern void special2       (int key, int x, int y);
extern void mouseEnter2    (int iState);

//=====================================================================
//  Menu texts
//=====================================================================
char *gmFile		= "File";						
char *gmOption	= "Options";
char *gmFplan		= "Flight Planner";
char *gmVeh			= "Vehicle";
char *gmMeteo		= "Weather";
char *gmWindow	= "Windows";
char *gmExport  = "Export Data";
char *gmDebugDB	= "Debug DB";
char *gmDebug   = "Debug";
char *gmTune		= "Tuning";
//=====================================================================
//  Menu bar
//=====================================================================
static puMenuBar *menu;
//----------------------------------------------------------------
//  Toggle a window
//-----------------------------------------------------------------
void toggle_window (Tag id, const char* winFilename)
{ 
  if (globals->fui->IsWindowCreated (id)) {
    // Destroy
    globals->fui->DestroyFuiWindow (id);

  } else {
    // Create
    globals->fui->CreateFuiWindow (id);
  }
}


//=========================================================================================
// File Menu
//=========================================================================================
//  Load Situation
//--------------------------------------------------------------------------
void file_load_situation_cb (puObject* obj)
{ globals->fui->CreateOneWindow(FUI_WINDOW_SITUATION_LOAD,0);
}
//--------------------------------------------------------------------------
//  File manage DLL
//--------------------------------------------------------------------------
void file_manage_dll_cb (puObject* obj)
{ globals->fui->CreateOneWindow(FUI_WINDOW_MANAGE_DLL, 0);
}
//--------------------------------------------------------------------------
//  File Quit
//--------------------------------------------------------------------------
void file_quit_cb (puObject* obj)
{ globals->fui->CreateOneWindow(FUI_WINDOW_QUIT, 0);
}
//==================================================================================
char *file_legends[] =
{
  "Quit",
  "--------------------",
  "Manage DLLs...",
  "--------------------",
  "Load Situation...",
  NULL
};

puCallback file_cb[] =
{
  file_quit_cb,
  NULL,
  file_manage_dll_cb,             // file_manage_dll_cb,
  NULL,
  NULL,                           //file_load_situation_cb,
  NULL
};
//==============================================================================
// Options menu
//==============================================================================
//  Video options
//------------------------------------------------------------------
void options_video_cb (puObject* obj)
{ globals->kbd->Stroke('menu','ogrp'); }
//------------------------------------------------------------------
//  Sound options
//------------------------------------------------------------------
void options_audio_cb (puObject* obj)
{ globals->kbd->Stroke('menu','osnd'); }
//------------------------------------------------------------------
//  Date time
//------------------------------------------------------------------
void options_date_time_cb (puObject* obj)
{ globals->kbd->Stroke('menu','date'); }
//------------------------------------------------------------------
// Key and buttons
//------------------------------------------------------------------
void options_keys_buttons_cb (puObject* obj)
{ globals->kbd->Stroke('menu','keys'); }
//------------------------------------------------------------------
// Set Axes
//------------------------------------------------------------------
void options_setup_axes_cb (puObject* obj)
{ globals->kbd->Stroke('menu','axes');  }
//------------------------------------------------------------------
// Start up
//------------------------------------------------------------------
void options_startup_cb (puObject* obj)
{ globals->kbd->Stroke('menu','gogo'); }
//------------------------------------------------------------------
// Scenery
//------------------------------------------------------------------
void options_scenery_cb (puObject* obj)
{ globals->kbd->Stroke('menu','tree'); }
//====================================================================
char *options_legends[] =
{
  "Scenery...",
  "--------------------",
  "Startup...",
  "--------------------",
  "Define Axes Parameters",
  "Keys & Buttons...",
  "--------------------",
  "Date & Time...",
  "Audio...",
  "Video...",
  NULL
};

puCallback options_cb[] =
{
  NULL,                         //options_scenery_cb,
  NULL,
  NULL,                         // options_startup_cb,
  NULL,
  options_setup_axes_cb,
  options_keys_buttons_cb,
  NULL,
  NULL,                         // options_date_time_cb,
  NULL,                         // options_audio_cb,
  NULL,                         // options_video_cb,
  NULL
};

//======================================================================
// Flight Plan menu
//======================================================================
//------------------------------------------------------------------
// Sectional charts
//------------------------------------------------------------------
void flightplan_sectional_cb (puObject* obj)
{ globals->kbd->Stroke('menu','chrt'); } 
//------------------------------------------------------------------
// Directory
//------------------------------------------------------------------
void flightplan_directory_cb (puObject* obj)
{ globals->kbd->Stroke('menu','vdir'); } 
//------------------------------------------------------------------
// Current flight plan
//------------------------------------------------------------------
void flightplan_log_cb (puObject* obj)
{ globals->kbd->Stroke('menu','cfpn'); }
//------------------------------------------------------------------
// List of plans
//------------------------------------------------------------------
void flightplan_list_cb (puObject* obj)
{ globals->kbd->Stroke('menu','lfpn'); }
//==================================================================
char *flightplan_legends[] =
{ "Sectional chart",
  "Current Flight Plan",
  "Flight Plans list...",
  "Directory...",
  NULL
};

puCallback flightplan_cb[] =
{ flightplan_sectional_cb,
  flightplan_log_cb,
  flightplan_list_cb,
  flightplan_directory_cb,
  NULL
};
//======================================================================
// Vehicle  menu
//======================================================================
//----------------------------------------------------------------------
//  Vehicle info:  Intercepted by by aircraft
//----------------------------------------------------------------------
void vehicle_option_cb (puObject* obj)
{ globals->kbd->Stroke('menu','adet'); }
//----------------------------------------------------------------------
//  Vehicle select
//----------------------------------------------------------------------
void vehicle_select_cb (puObject* obj)
{ globals->kbd->Stroke('menu','sair'); }
//----------------------------------------------------------------------
//  Vehicle check list
//----------------------------------------------------------------------
void vehicle_checklist_cb (puObject* obj)
{ globals->kbd->Stroke('menu','cklw'); }
//----------------------------------------------------------------------
//  Vehicle fuel load
//----------------------------------------------------------------------
void vehicle_fuel_loadout_cb (puObject* obj)
{ globals->kbd->Stroke('menu','sful'); }
//----------------------------------------------------------------------
//  Vehicle load and weight
//----------------------------------------------------------------------
void vehicle_load_station_cb (puObject* obj)
{ globals->kbd->Stroke('menu','sbag'); } 
//----------------------------------------------------------------------
//  Vehicle CoG
//----------------------------------------------------------------------
void vehicle_cg_cb (puObject* obj)
{ globals->kbd->Stroke('menu','cgin'); }
//----------------------------------------------------------------------
//  Vehicle Teleport
//----------------------------------------------------------------------
void vehicle_vpilot_cb (puObject* obj)
{ globals->pln->VirtualPilot(); }
//----------------------------------------------------------------------
//  Vehicle dammage report
//----------------------------------------------------------------------
void Vehicle_reset_cb (puObject* obj)
{ globals->kbd->Stroke('menu','rset');
}
//=======================================================================
char *vehicle_legends[] =
{ "Reset Damages",
  "--------------------",
	"Virtual pilot",
  "--------------------",
  "CG Indicator...",
  "--------------------",
  "Weight & Baggage",
  "Fuel Loadout...",
  "--------------------",
  "Checklist...",
  "Aircraft select",
  "Aircraft options",
  NULL
};

puCallback vehicle_cb[] =
{ Vehicle_reset_cb,
  NULL,
  vehicle_vpilot_cb,
  NULL,
  vehicle_cg_cb,
  NULL,
  vehicle_load_station_cb,
  vehicle_fuel_loadout_cb,
  NULL,
  vehicle_checklist_cb,
  vehicle_select_cb,
  vehicle_option_cb,           
  NULL
};

//=======================================================================
// Weather menu
//=======================================================================
//----------------------------------------------------------------------
//  Weather overview
//----------------------------------------------------------------------
void weather_overview_cb (puObject* obj)
{ globals->kbd->Stroke('menu','envo'); }
//----------------------------------------------------------------------
//  Weather clouds
//----------------------------------------------------------------------
void weather_clouds_cb (puObject* obj)
{ globals->kbd->Stroke('menu','envc'); }
//----------------------------------------------------------------------
//  Weather winds
//----------------------------------------------------------------------
void weather_winds_cb (puObject* obj)
{ globals->kbd->Stroke('menu','envw'); }
//----------------------------------------------------------------------
//  Weather sky tweaker
//----------------------------------------------------------------------
void weather_sky_tweaker_cb (puObject* obj)
{ globals->kbd->Stroke('menu','skyt'); }
//======================================================================
char *weather_legends[] =
{ "Sky Tweaker",
  "--------------------",
  "Winds & clouds",
  "--------------------",
  "Overview",
  NULL
};

puCallback weather_cb[] =
{
  NULL,                         // weather_sky_tweaker_cb,
  NULL,
  weather_winds_cb,
  NULL,
  weather_overview_cb,
  NULL
};

//======================================================================
// Windows menu
//======================================================================
//-------------------------------------------------------------------------
//  GPS window.  Created only if a GPS is present
//-------------------------------------------------------------------------
void windows_gps_cb (puObject* obj)
{ globals->kbd->Stroke('menu','gwin'); } 
//-------------------------------------------------------------------------
//  Vector map window.  
//-------------------------------------------------------------------------
void windows_vector_cb (puObject* obj)
{ globals->kbd->Stroke('menu','vwin'); }
//-------------------------------------------------------------------------
//  Global options.  
//-------------------------------------------------------------------------
void windows_glo_options_cb(puObject* obj)
{ globals->kbd->Stroke('glob','gopt'); }
//-------------------------------------------------------------------------
//  Secondary window.  TO BE DISPLACED SOMEWHERE ELSE
//-------------------------------------------------------------------------
// Create the secondary view or screen
// if it does not exist yet
// reuse if available the previous pos and size
void windows_newview_cb(puObject* obj)
{
  if (!globals->sScreen.ID)
  {
    glutInitWindowPosition (globals->sScreen.X, globals->sScreen.Y);
    glutInitWindowSize (globals->sScreen.Width,
                        globals->sScreen.Height);
    globals->sScreen.ID = glutCreateWindow("Fly! Legacy (2)");

    // declare specific callback fo this new window
    glutMouseFunc         (mouse2);
    glutMotionFunc        (motion2);
    glutPassiveMotionFunc (passive_motion2);
    glutDisplayFunc       (redraw2);
    glutReshapeFunc       (reshape2);
    glutKeyboardFunc      (keyboard2);
    glutSpecialFunc       (special2);

    // Disable GLUT cursor
    glutSetCursor (GLUT_CURSOR_NONE);
  }
}

void saveSecondViewPos()
{
  globals->sScreen.X      = glutGet(GLUT_WINDOW_X);
  globals->sScreen.Y      = glutGet(GLUT_WINDOW_Y);
  globals->sScreen.Height = glutGet(GLUT_WINDOW_HEIGHT);
  globals->sScreen.Width  = glutGet(GLUT_WINDOW_WIDTH);
}

void windows_closesec_cb(puObject* obj)
{
  if(globals->sScreen.ID)
  {
    // save the current win position
    // to reuse at the next opening request
    glutSetWindow(globals->sScreen.ID);
    saveSecondViewPos( );
    glutSetWindow(1);
    glutDestroyWindow(globals->sScreen.ID);
    globals->sScreen.ID = 0;
  }
}

void windows_toggleIlsView(puObject* obj)
{ globals->apm->SwapILSdraw();
}
//-------------------------------------------------------------------------
//  Swap Full screen.  
//-------------------------------------------------------------------------
void windows_togglefullscreen_cb(puObject* obj)
{

  if( glutGameModeGet(GLUT_GAME_MODE_ACTIVE)) return;
  int aWin = glutGetWindow();
  switch (globals->mScreen.full)  {
    case 0:
      { glutSetWindow(1);
        glutFullScreen();
        globals->mScreen.full = 1;
        return;
      }
    case 1:
      { glutSetWindow(1);
        glutReshapeWindow(globals->mScreen.Width,
                        globals->mScreen.Height);
        glutPositionWindow( 0, 0 );
        globals->mScreen.full = 0;
        return;
      }
  }
  return;
}
//------------------------------------------------------------------------
char *windows_legends[] =
{
  "Toggle full screen",
  "Toggle ILS View",
//  "--------------------",
//  "Close secondary view",
//  "Create secondary view",
  "--------------------",
//  "Globals options",
  "Vector Window",
  "GPS Window",
  NULL
};

puCallback windows_cb[] =
{
  windows_togglefullscreen_cb,
  windows_toggleIlsView,
 // NULL,                         // Separator
 // NULL,                         // windows_closesec_cb,
 //  NULL,                         // windows_newview_cb,
  NULL,
 // windows_glo_options_cb,
  windows_vector_cb,
  windows_gps_cb,
  NULL
};
//=========================================================================
//  Export menu
//=========================================================================
//-------------------------------------------------------------------------
//  Generic database.  
//-------------------------------------------------------------------------
void export_gen_cb(puObject* obj)
{ globals->exm->ExportGenericAsCVS();
  return; }
//-------------------------------------------------------------------------
//  Waypoint database.  
//-------------------------------------------------------------------------
void export_wpt_cb(puObject* obj)
{ globals->exm->ExportWptAsCVS();
  return; }
//-------------------------------------------------------------------------
//  Elevations database.  
//-------------------------------------------------------------------------
void export_elv_cb(puObject* obj)
{ globals->imp->ExportElevations();
  return; }
//-------------------------------------------------------------------------
//  coast contour database.  
//-------------------------------------------------------------------------
void export_cst_cb(puObject* obj)
{ globals->exm->ExportCoastInDB();
  return; }
//-------------------------------------------------------------------------
//  3D models  database.  
//-------------------------------------------------------------------------
void export_3dm_cb(puObject* obj)
{ globals->exm->Export3Dmodels();
  return; }
//-------------------------------------------------------------------------
//  Taxiways  database.  
//-------------------------------------------------------------------------
void export_twy_cb(puObject* obj)
{ globals->exm->ExportTaxiways();
  return; }
//-------------------------------------------------------------------------
//  Textures  database.  
//-------------------------------------------------------------------------
void export_gtx_cb(puObject* obj)
{ globals->exm->ExportGenTextures();
  return;
}
//-------------------------------------------------------------------------
//  World object  database.  
//-------------------------------------------------------------------------
void export_wob_cb(puObject* obj)
{ globals->exm->ExportSceneryOBJ();
  return;
}
//-------------------------------------------------------------------------
//  Export all sceneries  
//-------------------------------------------------------------------------
void export_trn_cb(puObject* obj)
{ globals->exm->ExportAllTRNs();
  return;
}
//-------------------------------------------------------------------------
// Check all sceneries  file in db
//-------------------------------------------------------------------------
void export_pod_in_db(puObject* obj)
{	 globals->exm->CheckSceneryFiles();
  return;
}
/*
void import_elv_cb(puObject* obj)
{ globals->imp->ImportElevations();
  return;
}
*/
//---Menu items ----------------------------------
char *export_legends[] =
{ "Check Database for POD files",
  "-------------------------",
	"Export TRN Sceneries",
  "Export World Objects",
  "Export Generic Textures",
  "Export Taxiwway",
  "Export 3D models",
  "Export Coasts",
  "Export Elevations",
  "Export Waypoints",
  "Export Generic(APT,..,COM)",
  NULL
};
//---Associated call back ------------------------
puCallback export_cb[] =
{ export_pod_in_db, 
	NULL,
	export_trn_cb,
  export_wob_cb,
  export_gtx_cb,
  export_twy_cb,
  export_3dm_cb,
  export_cst_cb,
  export_elv_cb,
  export_wpt_cb,
  export_gen_cb,
};
//--- Associated keyword in ini file ------------
char *keySQL[] = {
	"*",									// Check for pod
	"*",									// Separator
	"ExpTRN",							// Export sceneries
	"ExpOBJ",							// Export object
	"ExpTEX",							// Export generic texture
	"ExpTXY",							// Export taxiways
	"ExpM3D",							// Export model 3D
	"ExpSEA",							// Export coast data
	"ExpELV",							// Export elevation
	"EpxWPT",							// Export Waypoint
	"ExpGEN",							// Export generic
	0,
};
//----------------------------------------------------------
//	search for key word and check ini file
//----------------------------------------------------------
void CheckExportAccess(puCallback cb,puObject *itm)
{	int k=0;
  int	a=0;
	while (keySQL[k])
	{	if (cb != export_cb[k++])	continue;
		int m = k-1;
		if ('*' == *keySQL[m])		return;
		GetIniVar("SQL",keySQL[m],&a);
		if (0==a)		itm->greyOut();
		return;
	}
	itm->greyOut();
	return;
}
//----------------------------------------------------------
//	Check each export itmes
//----------------------------------------------------------
void CheckExportItems(puObject *itm)
{ puPopup  *exm = (puPopupMenu*)itm->getUserData();
  puObject *sub = exm->getFirstChild();
	int nbw       = exm->getNumChildren ();
	for (int k=0; k<nbw; k++)
	{	const char *name = sub->getLegend();
		puCallback  cb   = sub->getCallback();
		itm							 = sub;
		sub							 = sub->getNextObject();
		if (0 == cb)	continue;
		//--- Process according to item -----------
		CheckExportAccess(cb,itm);
	}
	return;
}
//----------------------------------------------------------
//	Create Export menu
//----------------------------------------------------------
void CheckExportMenu()
{	//-- Find the export sub menu ---------------
	puObject *sub = menu->getFirstChild();
	puObject *itm = 0;
	while (sub)
	{	const char *name = sub->getLegend();
	  itm		= sub;	
		sub		= sub->getNextObject();
		if (0 == name)	continue;
		if (strcmp(gmExport,name) == 0) return CheckExportItems(itm);
	}
	return;
}
//=========================================================================
// Debug Database items
//=========================================================================
//-------------------------------------------------------------------------
//  Dump waypoint
//-------------------------------------------------------------------------
void debug_dump_wptdb_cb (puObject* obj)
{
  FILE *f = fopen ("Debug/waypoint.txt", "w");
  if (f) {
    CDatabaseManager::Instance().GetWPTDatabase()->Dump (f);
    fclose (f);
  }

  DrawNoticeToUser ("Waypoint database dumped to \"waypoint.txt\"", 5);
}
//-------------------------------------------------------------------------
//  Dump obstruction
//-------------------------------------------------------------------------
void debug_dump_obsdb_cb (puObject* obj)
{
  FILE *f = fopen ("Debug/obstruct.txt", "w");
  if (f) {
    CDatabaseManager::Instance().GetOBSDatabase()->Dump (f);
    fclose (f);
  }

  DrawNoticeToUser ("Obstruction database dumped to \"obstruct.txt\"", 5);
}
//-------------------------------------------------------------------------
//  Dump ILS
//-------------------------------------------------------------------------
void debug_dump_ilsdb_cb (puObject* obj)
{
  FILE *f = fopen ("Debug/ils.txt", "w");
  if (f) {
    CDatabaseManager::Instance().GetILSDatabase()->Dump (f);
    fclose (f);
  }

  DrawNoticeToUser ("ILS database dumped to \"ils.txt\"", 5);
}
//-------------------------------------------------------------------------
//  Dump States
//-------------------------------------------------------------------------
void debug_dump_statdb_cb (puObject* obj)
{
  FILE *f = fopen ("Debug/state.txt", "w");
  if (f) {
    CDatabaseManager::Instance().GetSTADatabase()->Dump (f);
    fclose (f);
  }

  DrawNoticeToUser ("State database dumped to \"state.txt\"", 5);
}
//-------------------------------------------------------------------------
//  Dump country
//-------------------------------------------------------------------------
void debug_dump_ctrydb_cb (puObject* obj)
{
  FILE *f = fopen ("Debug/country.txt", "w");
  if (f) {
    CDatabaseManager::Instance().GetCTYDatabase()->Dump (f);
    fclose (f);
  }

  DrawNoticeToUser ("Country database dumped to \"country.txt\"", 5);
}
//-------------------------------------------------------------------------
//  Dump COM
//-------------------------------------------------------------------------
void debug_dump_commdb_cb (puObject* obj)
{
  FILE *f = fopen ("Debug/comm.txt", "w");
  if (f) {
    CDatabaseManager::Instance().GetCOMDatabase()->Dump (f);
    fclose (f);
  }

  DrawNoticeToUser ("Comm database dumped to \"comm.txt\"", 5);
}
//-------------------------------------------------------------------------
//  Dump ATS routes
//-------------------------------------------------------------------------
void debug_dump_atsdb_cb (puObject* obj)
{
  FILE *f = fopen ("Debug/atsroute.txt", "w");
  if (f) {
    CDatabaseManager::Instance().GetAtsRouteDatabase()->Dump (f);
    fclose (f);
  }

  DrawNoticeToUser ("ATS Route database dumped to \"ats.txt\"", 5);
}
//-------------------------------------------------------------------------
//  Dump NAVAID
//-------------------------------------------------------------------------
void debug_dump_navdb_cb (puObject* obj)
{
  FILE *f = fopen ("Debug/navaids.txt", "w");
  if (f) {
    CDatabaseManager::Instance().GetNAVDatabase()->Dump (f);
    fclose (f);
  }

  DrawNoticeToUser ("Navaid database dumped to \"navaids.txt\"", 5);
}
//-------------------------------------------------------------------------
//  Dump RUNWAY
//-------------------------------------------------------------------------
void debug_dump_rwydb_cb (puObject* obj)
{
  FILE *f = fopen ("Debug/runways.txt", "w");
  if (f) {
    CDatabaseManager::Instance().GetRWYDatabase()->Dump (f);
    fclose (f);
  }

  DrawNoticeToUser ("Runway database dumped to \"runways.txt\"", 5);
}
//-------------------------------------------------------------------------
//  Dump AIRPORTS
//-------------------------------------------------------------------------
void debug_dump_apdb_cb (puObject* obj)
{
  FILE *f = fopen ("Debug/airport.txt", "w");
  if (f) {
    CDatabaseManager::Instance().GetAPTDatabase()->Dump (f);
    fclose (f);
  }

  DrawNoticeToUser ("Airport database dumped to \"airport.txt\"", 5);
}
//=========================================================================
char *debugdb_legends[] =
{
  "Dump Waypoint Database",
  "Dump Obstruction Database",
  "Dump ILS Database",
  "Dump State Database",
  "Dump Country Database",
  "Dump Comm Database",
  "Dump ATS Route Database",
  "Dump Navaid Database",
  "Dump Runway Database",
  "Dump Airport Database",
  NULL
};

puCallback debugdb_cb[] =
{
  debug_dump_wptdb_cb,
  debug_dump_obsdb_cb,
  debug_dump_ilsdb_cb,
  debug_dump_statdb_cb,
  debug_dump_ctrydb_cb,
  debug_dump_commdb_cb,
  debug_dump_atsdb_cb,
  debug_dump_navdb_cb,
  debug_dump_rwydb_cb,
  debug_dump_apdb_cb,
  NULL
};

//======================================================================
// Debug menu
//======================================================================
//----------------------------------------------------------------------
void debug_dump_fui_cb (puObject* obj)
{
  const char *debugFilename = "Debug/Fui.txt";
  FILE *f = fopen (debugFilename, "w");
  if (f) {
    globals->fui->Print (f);
    fclose (f);
  }

  char debug[80];
  sprintf (debug, "FUI Manager dumped to %s", debugFilename);
  DrawNoticeToUser (debug, 5);
}
void debug_dump_system_pfs_cb (puObject* obj)
{
  FILE *f = fopen ("Debug/pfs.txt", "w");
  if (f) {
    pfsdump (&globals->pfs, f);
    fclose (f);
  }

  DrawNoticeToUser ("System POD filesystem dumped to \"pfs.txt\"", 5);
}

void debug_messaging_cb (puObject* obj)
{
  msg_debug_dlg_create ();
}

void debug_dump_keymap_cb (puObject* obj)
{
  FILE *f = fopen ("Debug/keymap.txt", "w");
  if (f) {
    globals->kbd->Print (f);
    fclose (f);
  }

  DrawNoticeToUser ("Key mappings dumped to \"keymap.txt\"", 5);
}

void debug_dump_time_cb (puObject* obj)
{
  FILE *f = fopen ("Debug/time.txt", "w");
  if (f) {
    globals->tim->Print (f);
    fclose (f);
  }

  DrawNoticeToUser ("Time manager dumped to \"time.txt\"", 5);
}

void debug_dump_camera_cb (puObject* obj)
{
  CCameraManager *camera = globals->pln->GetCameraManager ();
  if (camera != NULL) {
    FILE *f = fopen ("Debug/camera.txt", "w");
    if (f) {
      camera->Print (f);
      fclose (f);
    }
  }

  DrawNoticeToUser ("Camera manager dumped to \"camera.txt\"", 5);
}

void debug_dump_terrain_cb (puObject* obj)
{
  FILE *f = fopen ("Debug/terrain.txt", "w");
  if (f) {
    fclose (f);
  }

  DrawNoticeToUser ("Terrain root dumped to \"terrain.txt\"", 5);
}

void debug_dump_sky_cb (puObject* obj)
{
  FILE *f = fopen ("Debug/sky.txt", "w");
  if (f) {
    CSkyManager::Instance().Print (f);
    fclose (f);
    DrawNoticeToUser ("Sky root dumped to \"sky.txt\"", 5);
  }
}

void debug_dump_user_cb (puObject* obj)
{
  CVehicleObject *veh = globals->pln;
  if (veh) {
    FILE *f = fopen ("Debug/user.txt", "w");
    if (f) {
      veh->Print (f);
      fclose (f);
      DrawNoticeToUser ("Vehicle dumped to \"user.txt\"", 5);
    }
  }
}

void debug_dump_aeromodel_cb (puObject* obj)
{
  CVehicleObject *veh = globals->pln;
  if (veh) {
    FILE *f = fopen ("Debug/DumpedAeromodel.txt", "w");
    if (f) {
      veh->Print (f);
      fclose (f);
    }
  }
}

void debug_sim_cb (puObject *obj)
{
  globals->fui->CreateFuiWindow (FUI_WINDOW_DEBUG_SIM,0);
  return;
}

#ifdef _DEBUG
void debug_breakpoint_cb (puObject* obj)
{
#ifdef HAVE__ASM
  _asm { int 3 }
#endif // HAVE__ASM
}
#endif // _DEBUG

void debug_dump_electrical_cb (puObject* obj)
{
  CAirplane *pln = globals->pln;
  if (pln != NULL) {
    if (pln->GetType () == TYPE_FLY_AIRPLANE) 
		{	CElectricalSystem *amp = pln->amp;
      if (amp != NULL) 
			{	FILE *f = fopen ("Debug/electrical.txt", "w");
        if (f) 
				{	amp->Print (f);
          fclose (f);
          
        }
      }
    }
  }

  DrawNoticeToUser ("Aircraft electrical system dumped to \"electrical.txt\"", 5);
}

void debug_stream_test_cb (puObject* obj)
{
  SStream* s = new SStream;
  strncpy (s->filename, "teststream.txt",(PATH_MAX-1));
  strncpy (s->mode, "w",3);
  OpenStream (s);

  WriteComment ("Comment...testing testing testing", s);
  WriteComment ("", s);
  WriteTag ('bgno', "---- object ----", s);
  WriteTag ('bgno', "---- nested Object ----", s);
  WriteTag ('int_', "---- int ----", s);
  int i = 500;
  WriteInt (&i, s);
  WriteTag ('uint', "---- unsigned int ----", s);
  unsigned int ui = 12345678;
  WriteUInt (&ui, s);
  float f = 12345.67f;
  WriteTag ('flot', "---- float ----", s);
  WriteFloat (&f, s);
  double d = 987654.3210;
  WriteTag ('dubl', "---- double ----", s);
  WriteDouble (&d, s);
  WriteTag ('stng', "---- string ----", s);
  WriteString ("This a string", s);
  SVector v;
  v.x = 1.0;
  v.y = 2.0;
  v.z = 3.0;
  WriteTag ('vect', "--- vector ----", s);
  WriteVector (&v, s);
  SPosition pos;
  pos.lat = 1000.0;
  pos.lon = 2000.0;
  pos.alt = 3000.0;
  WriteTag ('posn', "---- position ----", s);
  WritePosition (&pos, s);
  SMessage mesg;
  WriteTag ('mesg', "---- message ----", s);
  WriteMessage (&mesg, s);
  WriteTag ('endo', s);
  WriteTag ('endo', s);

  CloseStream (s);
  delete s;
}

//=============================================================================
// Debug menu items
//=============================================================================
char *debug_legends[] =
{
  "Dump FUI Manager",
  "Dump Electrical Systems",
  "Dump System POD Filesystem",
  "Messaging",
  "Dump Key Mappings",
  "Dump Time Manager",
  "Dump Camera",
  "Dump Terrain",
  "Dump Sky",
  "Dump User Vehicle",
  "Dump Aeromodel",
  "Stream test",
  "Simulation Debug",
#ifdef _DEBUG
  "Breakpoint",
#endif
  NULL
};

puCallback debug_cb[] =
{
  debug_dump_fui_cb,
  debug_dump_electrical_cb,
  debug_dump_system_pfs_cb,
  debug_messaging_cb,
  debug_dump_keymap_cb,
  debug_dump_time_cb,
  debug_dump_camera_cb,
  debug_dump_terrain_cb,
  debug_dump_sky_cb,
  debug_dump_user_cb,
  debug_dump_aeromodel_cb,
  debug_stream_test_cb,
  debug_sim_cb,
#ifdef _DEBUG
  debug_breakpoint_cb,
#endif
  NULL
};
//============================================================================
//	Locate tuning ITEM
//============================================================================
void CheckTuningMenu(puObject *itm,char *iden,char st)
{ puPopup  *exm = (puPopupMenu*)itm->getUserData();
  puObject *sub = exm->getFirstChild();
	int nbw       = exm->getNumChildren ();
	for (int k=0; k<nbw; k++)
	{	const char *name = sub->getLegend();
		itm							 = sub;
		sub							 = sub->getNextObject();
		if (strcmp(name,iden) != 0)	continue;
		//--- Process according to item -----------
		if (0==st)		itm->greyOut();
		return;
	}
	return;
}
//============================================================================
//	Locate tuning MENU
//============================================================================
void CrossTuningMenu(char *iden,char st)
{	//-- Find the  sub menu -----------------
	puObject *sub = menu->getFirstChild();
	puObject *itm = 0;
	while (sub)
	{	const char *name = sub->getLegend();
	  itm		= sub;	
		sub		= sub->getNextObject();
		if (0 == name)							  continue;
		if (strcmp(gmTune,name) != 0) continue;
		//--- Check itm according to state ---
		CheckTuningMenu(itm,iden,st);
	}
	return;
}
//============================================================================
//	Inhibit some menus
//============================================================================
void CheckTuningMenu()
{ char elv = globals->elvDB;
  CrossTuningMenu("TERRA Editor",elv);
	return;
}

//============================================================================
//  TUNING MENU ITEMS
//  NOTE JS->Laurent:  Don't put vehicle code into the menu callback
//                      When there is no vehicle, this code should not be 
//                      executed
// Normally, all vehicle functions should be redirected to the key map
// because there is a feature that prevent vehicle code when no vehicle is 
// present
//============================================================================
//----------------------------------------------------------------------------
//  Model terra editor: mted
//----------------------------------------------------------------------------
void model_teditor_cb(puObject *obj)
{ globals->kbd->Stroke('glob','wted');
  return; }
//----------------------------------------------------------------------------
//  Model browser: mbro
//----------------------------------------------------------------------------
void model_browser_cb(puObject *obj)
{ globals->kbd->Stroke('menu','mbro');
  return; }
//----------------------------------------------------------------------------
//  Terra browser: tbro
//----------------------------------------------------------------------------
void terra_browser_cb(puObject *obj)
{ globals->kbd->Stroke('menu','tbro');
  return; }
//----------------------------------------------------------------------------
//  Tune PID: tpid
//----------------------------------------------------------------------------
void tune_pid_cb(puObject *obj)
{ globals->kbd->Stroke('menu','tpid');
  return; }
//----------------------------------------------------------------------------
//  Statistics:  cntr
//----------------------------------------------------------------------------
void tune_statistic_cb(puObject *obj)
{ globals->kbd->Stroke('menu','cntr');
  return; }
//----------------------------------------------------------------------------
//  plot data window: 
//----------------------------------------------------------------------------
void tune_plot_cb(puObject *obj)
{ globals->kbd->Stroke('menu','plot');
  return; }
//----------------------------------------------------------------------------
//  Probe window: prob
//----------------------------------------------------------------------------
void tune_probe_cb(puObject *obj)
{ globals->kbd->Stroke('menu','prob');
  return; }
//--------------------------------------------------------
char *tune_legends[] =
{ "TERRA Editor",
	"----------------",
	"OBJECT Browser",
  "TERRA Browser",
  "Statistics",
  "Plot data",
  "PID tuning",
  "Subsystem probe",
  NULL
};

//------------------------------------------------------------------------
puCallback tune_cb[] =
{ model_teditor_cb,
	NULL,
	model_browser_cb,
  terra_browser_cb,
  tune_statistic_cb,
  tune_plot_cb,
  tune_pid_cb,
  tune_probe_cb,
  NULL
};

//============================================================================
// Help menu
//============================================================================
void helpAbout_cb (puObject* obj)
{
  toggle_window (FUI_WINDOW_HELP_ABOUT,0);
}

char *help_legends[] =
{
  "About...",
  NULL
};

puCallback help_cb[] =
{
  NULL,                 // helpAbout_cb,
  NULL
};

//=============================================================================
void cleanup_menu (void)
{
  menu->empty();
  delete menu;
}

void hide_menu (void)
{
  menu->hide ();
}

void show_menu (void)
{
  menu->reveal ();
}

void toggle_menu (void)
{
  if (menu->isVisible()) {
    menu->hide ();
  } else {
    menu->reveal ();
  }
}

//============================================================================
// sdk: Adds sub menus from APIAddUIMenu //
//      Send data to :
//      DLLEventNotice(SDLLObject*,unsigned long,unsigned long,unsigned long,unsigned long);
//      ==> On_EventNotice (NULL, windowID, componentID, event, subEvent)
//============================================================================
char			 *nul_Legend[] = {0};
puCallback  nul_cb[] = {0};
//============================================================================
// Menu initialization
//============================================================================
void init_user_menu (void)
{ std::vector <sdkmenu::SMenuData>::const_iterator i_md;

  menu = new puMenuBar(-1);
	menu->setStyle(PUSTYLE_SHADED | PUSTYLE_BEVELLED);
  menu->add_submenu (gmFile,				file_legends, file_cb);
  menu->add_submenu (gmOption,			options_legends, options_cb);
  menu->add_submenu (gmFplan,				flightplan_legends, flightplan_cb);
  menu->add_submenu (gmVeh,					vehicle_legends, vehicle_cb);
  menu->add_submenu (gmMeteo,				weather_legends, weather_cb);
  menu->add_submenu (gmWindow,			windows_legends, windows_cb);

  // Only add debugging menus if [UI] setting debugMenus is true (=1)
  int i = 0;
  GetIniVar ("UI", "debugMenus", &i);
  if (i == 1) {
		menu->add_submenu (gmExport,	export_legends, export_cb);
    menu->add_submenu (gmDebugDB, debugdb_legends, debugdb_cb);
    menu->add_submenu (gmDebug,		debug_legends, debug_cb);
    menu->add_submenu (gmTune,		tune_legends, tune_cb);
		CheckExportMenu();
  }
  menu->add_submenu ("Help", help_legends, help_cb);

  //----Change colors ----------------------------------------------------
  //----------------------------------------------------------------------
  int addin_menus_counter = 0; 
  for (i_md = globals->sdk_menu.md_.begin (); i_md != globals->sdk_menu.md_.end (); ++i_md) {
    //
    std::vector <std::string>::const_iterator i_item_str;
    std::vector <unsigned long>::const_iterator i_item_ID = i_md->itemID.begin ();
    int counter = 0;
    for (i_item_str = i_md->items.begin (); i_item_str != i_md->items.end (); ++i_item_str, ++i_item_ID)
    {
      if (counter < sdkmenu::MAX_SDK_SUBMENU_NUM) {
        sdk_menu_legends[addin_menus_counter][counter]             = (char *)i_item_str->c_str ();
        sdk_en          [addin_menus_counter][counter].windowID    = 0;
        sdk_en          [addin_menus_counter][counter].componentID = i_md->menuID;
        sdk_en          [addin_menus_counter][counter].event       = EVENT_SELECTITEM;
        sdk_en          [addin_menus_counter][counter].subEvent    = (*i_item_ID);
      }

      counter++;
    }

    // callback are yet statically done
//    sdk_menu_cb[0]      = _0_cb;
//    sdk_menu_cb[1]      = _1_cb;

    menu->add_submenu ( i_md->str.c_str (),
                        sdk_menu_legends[addin_menus_counter],
                        sdk_menu_cb[addin_menus_counter]
                      );

    addin_menus_counter++;
  }

  menu->close ();
  // Hide menu by default
  hide_menu ();
}
