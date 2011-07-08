/*
 * WindowSimDebug.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005-2006 Chris Wallace
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

/*! \file WindowSimDebug
 *  \brief Implements FUI callback for Sim Debug window
 *
 *  Implements the FUI callback for the Sim Debug window which displays debugging values for
 *    engine, wings, gear ...
 */

#include "../Include/FlyLegacy.h"
//#include "../Include/FuiUser.h"
#include "../Include/Globals.h"
#include "../Include/FuiPlane.h"


/// \todo make this function available in a header
extern void close_window (Tag id);


//=======================================================================
//  MENU FOR RUNWAY MAP
//=======================================================================
char *WingMENU[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//=======================================================================
//  LCDEV* CSimDebugManager
//=======================================================================
CSimDebugManager::CSimDebugManager (Tag windowId, const char* winFilename)
: CFuiWindow( windowId, winFilename)
{
  type = COMPONENT_WINDOW;
  widgetTag = 'defa';
  strcpy (widgetName, "Sim Debug");
  //
  slct_wing_list = NULL;
  dfui_wing = (CFuiPopupMenu *) GetComponent ('wing');
  if (0 == dfui_wing) gtfo ("CSimDebugManager::CSimDebugManager error getting 'wing' component");
  //
  debug_cage = NULL;
  dfui_cage = (CFuiButton*)   GetComponent('cage');
  if (0 == dfui_cage) gtfo ("CSimDebugManager::CSimDebugManager error getting 'cage' component");

  Init ();
  //
  #ifdef _DEBUG	
  {	FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_debug_.txt", "a")) == NULL)
	  {
		  int test = 0;
		  fprintf(fp_debug, "CSimDebugManager::CSimDebugManager %d\n", test);
		  fclose(fp_debug); 
  }	}
  #endif  
}

CSimDebugManager::~CSimDebugManager (void)
{
  if (slct_wing_list) {
    if (globals->fui->IsWindowCreated (FUI_WINDOW_DEFAULT))
      globals->fui->DestroyFuiWindow (FUI_WINDOW_DEFAULT);
    slct_wing_list = NULL;
  }
  if (debug_cage) {
    if (globals->fui->IsWindowCreated (FUI_WINDOW_CAGING))
      globals->fui->DestroyFuiWindow (FUI_WINDOW_CAGING);
    debug_cage = NULL;
  }
  //
  #ifdef _DEBUG	
  {	FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_debug_.txt", "a")) == NULL)
	  {
		  int test = 0;
		  fprintf(fp_debug, "CSimDebugManager::~CSimDebugManager %d\n", test);
		  fclose(fp_debug); 
  }	}
  #endif  
}

void CSimDebugManager::Close (void)
{
  //
  #ifdef _DEBUG	
  {	FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_debug_.txt", "a")) == NULL)
	  {
		  int test = 0;
		  fprintf(fp_debug, "CSimDebugManager::Close %d\n", test);
		  fclose(fp_debug); 
  }	}
  #endif 
/// \todo verify this  virtual function is computed all along the global close process
  CFuiWindow::Close ();
}

int CSimDebugManager::Init (void)
{
  GetWingList (NULL, NULL);
  SetWingList ();
  return 0;
}

void CSimDebugManager::Draw (void)
{
  /// \todo maybe a timer here
  // don't draw anything if pointer is not ready
  if (!surface) return;

  //
  //EraseSurfaceRGB   (surface, MakeRGB (0,0,0));
  //DrawFastLine      (surface, 10, 5, 10, 10, MakeRGB (255,255,255));
  //BlitOpaqueSurface (surface, 0, 0, 0);

  //---Draw window decoration -------------------------------
  CFuiWindow::Draw(); ///< fui_map->Draw ();
}

bool CSimDebugManager::InsideClick (int mx, int my, EMouseButton button)
{
  return CFuiWindow::InsideClick (mx, my, button);
}

void CSimDebugManager::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{
  switch (idm) {
    case FUI_WINDOW_CAGING:
      if (evn == EVENT_BUTTONPRESSED) {
        OpenCageDialogWindow ();
      }
      return;
    case '____':
      if (evn == EVENT_SELECTITEM) {
      }
      return;
  }
  return;
}

void CSimDebugManager::NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn)
{ 
  switch (id) {
    case 'wing':
      if (evn == EVENT_SELECTITEM) {
        dfui_wing->SetButtonText (static_cast<char *> (wMenu.aText[(U_INT)itm]));
        OpenListDialogWindow ();
      }
      return;
    case '____':
      if (evn == EVENT_SELECTITEM) {
      }
      return;
  }
  return;
}

void CSimDebugManager::NotifyMenuEvent(Tag idm, Tag itm)
{
  switch (itm)  {
    case 'slct':
      {
        GetWingList ();
        OpenListDialogWindow ();
      }
      break;

      // don't CheckSelectedPart so return
      return;
  }
}

void CSimDebugManager::GetWingList (const char* root, const char* folder)
{
  md_.clear ();
  md_.push_back ("close");
  md_.push_back ("All");
  md_.push_back ("left aileron");
  md_.push_back ("right aileron");
  md_.push_back ("left flap");
  md_.push_back ("right flap");
  md_.push_back ("rudder");
  md_.push_back ("elevator");
}

void CSimDebugManager::OpenCageDialogWindow (void)
{
  debug_cage = (CCageDebug *) globals->fui->CreateFuiWindow (FUI_WINDOW_CAGING);
  if (!debug_cage) gtfo ("CSimDebugManager::OpenCageDialogWindow CreateFuiWindow error");
}

void CSimDebugManager::OpenListDialogWindow (void)
{
  slct_wing_list = (CSimDebugWings *) globals->fui->CreateFuiWindow (FUI_WINDOW_DISPLAY_DBG);
  if (!slct_wing_list) gtfo ("CSimDebugManager::OpenListDialogWindow CreateFuiWindow error");
}

void CSimDebugManager::SetWingList (const bool &newpage, const int &start, const int &end)
{
  if (dfui_wing) {

//    std::vector<std::string>::iterator i_md;
    if (newpage) dfui_wing->ClearPage ();
    dfui_wing->SetButtonText ("All");

/*
    U_INT inx = 0;
    for (i_md = md_.begin (); i_md != md_.end (); ++i_md) {
      char buff[256] = {"\0"};
      strncpy (buff, i_md->c_str (), 256);
      WingMENU[inx] = buff;
      inx++;
    }
*/

    WingMENU[0] = "close";
    WingMENU[1] = "All";
    WingMENU[2] = "left aileron";
    WingMENU[3] = "right aileron";
    WingMENU[4] = "left flap";
    WingMENU[5] = "right flap";
    WingMENU[6] = "rudder";
    WingMENU[7] = "elevator";

//    WingMENU[inx] = "";
    dfui_wing->CreatePage (&wMenu,WingMENU);
  }
}

//=======================================================================
//  LCDEV* CSimDebugManager
//=======================================================================
CSimDebugWings::CSimDebugWings (Tag windowId, const char* winFilename,char *wpart)
: CFuiWindow( windowId, winFilename)
{
  type = COMPONENT_WINDOW;
  widgetTag = 'defa';
  strcpy (widgetName, "Wing");
  if (wpart) wing_part = wpart;
  else wing_part = "wing Left w/Aileron"; //"wing Tail w/rudder";// "wing Stabilizer w/elevator";
  this->SetProperty (FUI_XY_RESIZING);
  //this->SetWidth  (100);
  //this->SetHeight (200);
  //this->Resize    (100, 200);
  w = 108; // width
  h = 172; // height
  //this->ReadFinished();


  this->GetPosition (&posWx, &posWy);
  //
  label_wing = (CFuiLabel *) GetComponent ('dbgL');
  if (0 == label_wing) gtfo ("CSimDebugWings::CSimDebugWings error getting 'dbgL' component");
  //label_wing->SetHeight (100);
  //label_wing->SetWidth  (200);
  label_wing->SetText ("\0");
  label_wing->SetProperty (FUI_TRANSPARENT);

  //
  #ifdef _DEBUG	
  {	FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_debug_.txt", "a")) == NULL)
	  {
		  int test = 0;
		  fprintf(fp_debug, "CSimDebugWings::CSimDebugWings %d\n", test);
		  fclose(fp_debug); 
  }	}
  #endif
  
  cl_wing     = NULL;
  cd_wing     = NULL;
  cm_wing     = NULL;
  LD_wing     = NULL;
  aoa_wing    = NULL;
  damage_wing = NULL;
  dCL_wing    = NULL;
  dCD_wing    = NULL;
  dCM_wing    = NULL;
  dPitch_wing = NULL;
  
  Init ();
}

CSimDebugWings::~CSimDebugWings (void)
{
  SAFE_DELETE (cl_wing);
  SAFE_DELETE (cd_wing);
  SAFE_DELETE (cm_wing);
  SAFE_DELETE (LD_wing);
  SAFE_DELETE (aoa_wing);
  SAFE_DELETE (damage_wing);
  SAFE_DELETE (dCL_wing);
  SAFE_DELETE (dCD_wing);
  SAFE_DELETE (dCM_wing);
  SAFE_DELETE (dPitch_wing);
  //SAFE_DELETE (label_wing);

  #ifdef _DEBUG	
  {	FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_debug_.txt", "a")) == NULL)
	  {
		  int test = 0;
		  fprintf(fp_debug, "CSimDebugWings::~CSimDebugWings\n");
		  fclose(fp_debug); 
  }	}
  #endif
}

void CSimDebugWings::Draw (void)
{ CVehicleObject *veh = globals->sit->GetUserVehicle();
  
  // don't draw anything if pointer is not ready
  if (!surface) return;

  //
  //EraseSurfaceRGB   (surface, MakeRGB (0,0,0));
  //DrawFastLine      (surface, 10, 5, 10, 10, MakeRGB (255,255,255));
  //BlitOpaqueSurface (surface, 0, 0, 0);

  //---Draw window decoration -------------------------------
  //CFuiWindow::Resize (100, 200);
  CFuiWindow::Draw(); ///< fui_map->Draw ();

  int a = -2, b = -2;
  this->GetPosition         (&posWx, &posWy);
  cl_wing->SetPosition      (posWx + a + 10, posWy + b + 3);
  cd_wing->SetPosition      (posWx + a + 10, posWy + b + 18);
  cm_wing->SetPosition      (posWx + a + 10, posWy + b + 33);
  LD_wing->SetPosition      (posWx + a + 10, posWy + b + 48);
  aoa_wing->SetPosition     (posWx + a + 10, posWy + b + 63);
  damage_wing->SetPosition  (posWx + a + 10, posWy + b + 78);
  dCL_wing->SetPosition     (posWx + a + 10, posWy + b + 93);
  dCD_wing->SetPosition     (posWx + a + 10, posWy + b + 108);
  dCM_wing->SetPosition     (posWx + a + 10, posWy + b + 123);
  dPitch_wing->SetPosition  (posWx + a + 10, posWy + b + 138);

  if (veh->wng) {
    char buffer [128] = {0};
    double cl = (veh->wng->GetWingSection (wing_part))->GetCL ();
    sprintf (buffer,       " cl = %06.2f", cl);
    cl_wing->SetText      (buffer);
    double cdi = (veh->wng->GetWingSection (wing_part))->GetCDI ();
    sprintf (buffer,       "cdi = %06.2f", cdi);
    cd_wing->SetText      (buffer);
    sprintf (buffer,       " cm = %06.2f", (veh->wng->GetWingSection (wing_part))->GetCM ());
    cm_wing->SetText      (buffer);
    if (cdi) sprintf (buffer,       "L/D = %06.2f", cl / cdi);
    else     sprintf (buffer,       "L/D = error");
    LD_wing->SetText      (buffer);
    sprintf (buffer,       "aoa = %06.2f", (veh->wng->GetWingSection (wing_part))->GetAoA () * RAD2DEG);
    aoa_wing->SetText     (buffer);
    CVector tmp_ = (veh->wng->GetWingSection (wing_part))->GetSpeedVector ();
    sprintf (buffer,       "SVx = %06.2f", tmp_.x);
    damage_wing->SetText  (buffer);
    sprintf (buffer,       "SVy = %06.2f", tmp_.y);
    dCL_wing->SetText     (buffer);
    sprintf (buffer,       "SVz = %06.2f", tmp_.z * 3.6 / (double) KM_PER_NM);
    dCD_wing->SetText     (buffer);
    sprintf (buffer,       "ASL = %06.0f", (globals->geop.alt));
    dCM_wing->SetText     (buffer);
    sprintf (buffer,       "pch = %06.0f", 0.0);
    dPitch_wing->SetText  (buffer);

    cl_wing->Draw      ();
    cd_wing->Draw      ();
    cm_wing->Draw      ();
    LD_wing->Draw      ();
    aoa_wing->Draw     ();
    damage_wing->Draw  ();
    dCL_wing->Draw     ();
    dCD_wing->Draw     ();
    dCM_wing->Draw     ();
    dPitch_wing->Draw  ();
  }
}

int CSimDebugWings::Init (CFuiComponent *cmp)
{
  int a = -2, b = -2;
  cl_wing = new CFuiLabel (posWx + a + 10, posWy + b + 3, 90, 15, cmp/*this*/);
  cl_wing->SetProperty (FUI_IS_VISIBLE + FUI_IS_ENABLE);
  cl_wing->ReadFinished();

  cd_wing = new CFuiLabel (posWx + a + 10, posWy + b + 18, 90, 15, cmp/*this*/);
  cd_wing->SetProperty (FUI_IS_VISIBLE + FUI_IS_ENABLE);
  cd_wing->ReadFinished();

  cm_wing = new CFuiLabel (posWx + a + 10, posWy + b + 33, 90, 15, cmp/*this*/);
  cm_wing->SetProperty (FUI_IS_VISIBLE + FUI_IS_ENABLE);
  cm_wing->ReadFinished();

  LD_wing = new CFuiLabel (posWx + a + 10, posWy + b + 48, 90, 15, cmp/*this*/);
  LD_wing->SetProperty (FUI_IS_VISIBLE + FUI_IS_ENABLE);
  LD_wing->ReadFinished();

  aoa_wing = new CFuiLabel (posWx + a + 10, posWy + b + 63, 90, 15, cmp/*this*/);
  aoa_wing->SetProperty (FUI_IS_VISIBLE + FUI_IS_ENABLE);
  aoa_wing->ReadFinished();

  damage_wing = new CFuiLabel (posWx + a + 10, posWy + b + 78, 90, 15, cmp/*this*/);
  damage_wing->SetProperty (FUI_IS_VISIBLE + FUI_IS_ENABLE);
  damage_wing->ReadFinished();

  dCL_wing = new CFuiLabel (posWx + a + 10, posWy + b + 93, 90, 15, cmp/*this*/);
  dCL_wing->SetProperty (FUI_IS_VISIBLE + FUI_IS_ENABLE);
  dCL_wing->ReadFinished();

  dCD_wing = new CFuiLabel (posWx + a + 10, posWy + b + 108, 90, 15, cmp/*this*/);
  dCD_wing->SetProperty (FUI_IS_VISIBLE + FUI_IS_ENABLE);
  dCD_wing->ReadFinished();

  dCM_wing = new CFuiLabel (posWx + a + 10, posWy + b + 123, 90, 15, cmp/*this*/);
  dCM_wing->SetProperty (FUI_IS_VISIBLE + FUI_IS_ENABLE);
  dCM_wing->ReadFinished();

  dPitch_wing = new CFuiLabel (posWx + a + 10, posWy + b + 138, 90, 15, cmp/*this*/);
  dPitch_wing->SetProperty (FUI_IS_VISIBLE + FUI_IS_ENABLE);
  dPitch_wing->ReadFinished();

  return 0;
}

void CSimDebugWings::Close (void)
{
  #ifdef _DEBUG	
  {	FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_debug_.txt", "a")) == NULL)
	  {
		  int test = 0;
		  fprintf(fp_debug, "CSimDebugWings::Close\n");
		  fclose(fp_debug); 
  }	}
  #endif
/// \todo verify this  virtual function is computed all along the global close process
  CFuiWindow::Close ();
}

//=======================================================================
//  LCDEV* CCageDebug
//=======================================================================
CCageDebug::CCageDebug (Tag windowId, const char* winFilename, char *wpart)
: CFuiWindow( windowId, winFilename)
{
  type = COMPONENT_WINDOW;
  widgetTag = 'defa';
  strcpy (widgetName, "cage");

  this->GetPosition (&posWx, &posWy);
   //
  #ifdef _DEBUG	
  {	FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_debug_.txt", "a")) == NULL)
	  {
		  int test = 0;
		  fprintf(fp_debug, "CCageDebug::CCageDebug %d\n", test);
		  fclose(fp_debug); 
  }	}
  #endif

  Init ();
}

CCageDebug::~CCageDebug (void)
{
  #ifdef _DEBUG	
  {	FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_debug_.txt", "a")) == NULL)
	  {
		  int test = 0;
		  fprintf(fp_debug, "CCageDebug::~CCageDebug\n");
		  fclose(fp_debug); 
  }	}
  #endif
}

void CCageDebug::Draw (void)
{
  
  // don't draw anything if pointer is not ready
  if (!surface) return;

  //
  //EraseSurfaceRGB   (surface, MakeRGB (0,0,0));
  //DrawFastLine      (surface, 10, 5, 10, 10, MakeRGB (255,255,255));
  //BlitOpaqueSurface (surface, 0, 0, 0);

  //---Draw window decoration -------------------------------
  //CFuiWindow::Resize (100, 200);
  CFuiWindow::Draw(); ///< fui_map->Draw ();


}

int CCageDebug::Init (CFuiComponent *cmp)
{
  return 0;
}

void CCageDebug::Close (void)
{
  #ifdef _DEBUG	
  {	FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_debug_.txt", "a")) == NULL)
	  {
		  int test = 0;
		  fprintf(fp_debug, "CCageDebug::Close\n");
		  fclose(fp_debug); 
  }	}
  #endif
/// \todo verify this  virtual function is computed all along the global close process
  CFuiWindow::Close ();
}
