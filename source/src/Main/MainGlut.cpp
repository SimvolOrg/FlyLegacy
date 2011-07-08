/*
 * MainGlut.cpp
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

/*! \file MainGlut.cpp
 *  \brief Main application code using GLUT for window management
 */

// Temporarily define USE_GLUT until all development environment
//   project files have been updated (GLUT is currently the only
//   supported windowing system
#define USE_GLUT

#ifdef USE_GLUT

#include "../Include/FlyLegacy.h"
#include "../Include/Globals.h"
#include "../Include/Fui.h"
#include "../Include/CursorManager.h"
#include "../Include/Panels.h"
#include "../Include/TestBed.h"
#include "../Include/Export.h"
#include <GL/glut.h>
#include <stdexcept>
//=====================================================================================
// GLUT callback functions for primary screen
#ifdef _WIN32
void idle (void);
#endif // _WIN32
void reshape (int w, int h);
void motion (int x, int y);
void passive_motion (int x, int y);
void mouse (int button, int updown, int x, int y);
void keyboard (unsigned char key, int x, int y);
void special (int key, int x, int y);
void redraw (void);

// GLUT callback functions for secondary screen
void reshape2       (int w, int h);
void redraw2        (void);
void motion2        (int x, int y);
void passive_motion2(int x, int y);
void mouse2         (int button, int updown, int x, int y);
void keyboard2      (unsigned char key, int x, int y);
void special2       (int key, int x, int y);

char          Clear   =  0;
int           Choice  =  0;                       // Initial choice;
Tag           Cursor = 0;
//===========================================================================
// Terminate function
//===========================================================================
void FatalExit()
{	TRACE("=====FATAL Error===================");
	globals->mBox.DumpAll();
  exit(-1);
}
//=====================================================================================
//  Data used by the mouse management
//=====================================================================================
bool PUuse = false;                               // Mouse used by PU
bool FUuse = false;                               // Mouse used by Fui window
bool VHuse = false;                               // Mouse used by Vehicle
//=====================================================================================
//
// GLUT mouse button event callback
//=====================================================================================
void mouse2 ( int button, int updown, int x, int y )
{
  globals->cScreen = &globals->sScreen;
  bool used = false;

  // Send mouse click events to FUI for processing
  EMouseButton b = MOUSE_BUTTON_LEFT;
  switch (updown) {
  case GLUT_DOWN:
    switch (button) {
    case GLUT_LEFT_BUTTON:
      b = MOUSE_BUTTON_LEFT;
      break;

    case GLUT_MIDDLE_BUTTON:
      b = MOUSE_BUTTON_MIDDLE;
      break;

    case GLUT_RIGHT_BUTTON:
      b = MOUSE_BUTTON_RIGHT;
      break;
    }
    used = globals->fui->MouseClick (x, y, b);
    break;

  case GLUT_UP:
    switch (button) {
    case GLUT_LEFT_BUTTON:
      b = MOUSE_BUTTON_LEFT;
      break;

    case GLUT_MIDDLE_BUTTON:
      b = MOUSE_BUTTON_MIDDLE;
      break;

    case GLUT_RIGHT_BUTTON:
      b = MOUSE_BUTTON_RIGHT;
      break;
    }
    used = globals->fui->MouseStopClick (x, y, b);
    break;
  }

  if (!used) {
    // Send mouse event to PUI for potential processing
    puMouse (button, updown, x, y);

    // Send mouse motion to cokpit manager for gauge interaction
    CCockpitManager *pit = globals->pit;
    if (pit) pit->MouseClick (button, updown, x, y);
  }

  // Force screen redraw
  glutPostRedisplay () ;
}
//===================================================================================
//  COMPUTE MOUSE EVENT
//====================================================================================
EMouseButton MouseEvent(int button)
{ switch (button) {
        case GLUT_LEFT_BUTTON:    return MOUSE_BUTTON_LEFT;
        case GLUT_MIDDLE_BUTTON:  return MOUSE_BUTTON_MIDDLE;
        case GLUT_RIGHT_BUTTON:   return MOUSE_BUTTON_RIGHT;
        }
  return MOUSE_BUTTON_LEFT;
}
//===================================================================================
//  MOUSE EVENT OVER SIMULATION
//====================================================================================
void MouseForSImulation(int x,int y,EMouseButton b,int u,int but)
{ CSituation *sit = globals->sit;
  //Send to PU first -----------------------------------------------
  PUuse = (0 != puMouse (but, u, x, y));
  // Send mouse click events to FUI for processing
  switch (u) {
  case GLUT_DOWN:
    //----Send to FUI manager -------------------------------------
    if (PUuse)            break;
    FUuse = globals->fui->MouseClick (x, y, b);
    //---Send mouse event to panel for potential processing--------------
    if (FUuse)            break;
    if (!sit )            break;
    if (globals->noINT)   break;
    VHuse = sit->uVeh->MouseClick(b,u,x,y);
    break;

  case GLUT_UP:
    //--Used by PU Ignore -----------------------------------------
    if (PUuse)            break;
    //--Used by FUI manager ---------------------------------------
    if (FUuse)  {globals->fui->MouseStopClick (x, y, b); break;}
    //--Used by vehicle -------------------------------------------
    if (!VHuse)           break;
    if (!sit->uVeh)       break;
    sit->uVeh->MouseClick(b,u,x,y);
    break;
   }
  //----------------------------------------------------------------
  // Force screen redraw
  glutPostRedisplay () ;
}
//===================================================================================
//  MOUSE EVENT
//====================================================================================
void mouse ( int button, int u, int x, int y )
{ globals->cScreen = &globals->mScreen;
  bool used = false;
  EMouseButton b = MouseEvent(button);
  // Send mouse click events to FUI for processing
  switch (globals->appState)  {
    case APP_LOADING_SCREEN:
      globals->cum->SetCursor(Cursor);
      Choice = 1;
      return;
    case APP_SIMULATION:
      MouseForSImulation(x,y,b,u,button);
      return;
  }
}

//========================================================================
// Window manager Idle loop: process extra key from keyboard
//========================================================================
#ifdef _WIN32

// PROTOTYPE : Windows keyboard handling.  GLUT keyboard events do not support
//  all numeric keypad keys (e.g. 5, Enter, +, etc.) that are required.


typedef struct {
  BYTE          vk;
  EKeyboardKeys key;
} SNonGlutKey;

SNonGlutKey nonGlutKeys[] =
{
  { VK_CAPITAL, KB_KEY_CAPSLOCK },
  { VK_NUMLOCK, KB_KEY_NUMLOCK },
  { VK_SCROLL,  KB_KEY_SCROLLLOCK },
  { VK_SUBTRACT,KB_KEY_KEYPAD_MINUS },
  { VK_CLEAR,   KB_KEY_CENTER },
  { VK_ADD,     KB_KEY_KEYPAD_PLUS },
  { VK_RETURN,  KB_KEY_KEYPAD_ENTER },
  { VK_DIVIDE,  KB_KEY_KEYPAD_SLASH },
};

int nKeys = sizeof(nonGlutKeys) / sizeof(SNonGlutKey);
//========================================================================

void idle (void)
{
  BYTE keys[256];

  // Update the key modifier state
  GetKeyboardState(keys);
  int flymod = KB_MODIFIER_NONE;
  bool shift = (keys[VK_SHIFT] & 0x80) != 0;
  if (shift) flymod |= KB_MODIFIER_SHIFT;
  bool ctrl  = (keys[VK_CONTROL] & 0x80) != 0;
  if (ctrl)  flymod |= KB_MODIFIER_CTRL;
  bool alt   = (keys[VK_MENU] & 0x80) != 0;
  if (alt)   flymod |= KB_MODIFIER_ALT;
  //--- Check for keys that are not supported by GLUT, and pass them to the sim
  //   on keydown events.
  for (int i=0; i<nKeys; i++) {
    BYTE vk = nonGlutKeys[i].vk;
    //----Check for keystroke ---------------------------
    if (0== (keys[vk] & 0x80))  continue;
    globals->kbd->KeyPress (nonGlutKeys[i].key, (EKeyboardModifiers)flymod);
    break;
  }
}
#endif // _WIN32


//=====================================================================================
// GLUT window reshape's
//=====================================================================================
void reshape2 ( int w, int h ) // secondary view
{ glViewport ( 0, 0, w, h ) ;
  globals->sScreen.Width  = w;
  globals->sScreen.Height = h;
  globals->aspect = double(w) / h;
}
//=====================================================================================
//  Windows is rescaled 
//=====================================================================================
void reshape ( int w, int h )
{ glViewport ( 0, 0, w, h ) ;
  globals->mScreen.Width  = w;
  globals->mScreen.Height = h;
  globals->aspect = double(w) / h;
  switch (globals->appState)  {
    case APP_SIMULATION:
      if (globals->pit) globals->pit->ScreenResize();
      if (globals->fui) globals->fui->ScreenResize();
      return;
    default:
      return;
  }
  return;
}


//=====================================================================================
// GLUT mouse motion event
//=====================================================================================
void motion2 ( int x, int y )
{ globals->sScreen.bMouseOn  = true;
  globals->mScreen.bMouseOn = false;
  globals->cScreen = &globals->sScreen;

  // Send mouse motion to cockpit manager for panel scrolling
  CCockpitManager *pit = globals->pit;
  if (pit) pit->MouseMove (x, y);

  // Send mouse motion to cursor manager
  globals->cum->MouseMotion (x, y);

  // Force screen redraw
  glutPostRedisplay () ;
}
//============================================================================
//  MOUSE MOTION
//===========================================================================
void motion ( int x, int y )
{ globals->cum->SetCursor(Cursor);
  globals->sScreen.bMouseOn = false;
  globals->mScreen.bMouseOn = true;
  globals->cScreen = &globals->mScreen;
  // Send mouse motion data to FUI
  bool used = globals->fui->MouseMove (x, y);
  if (!used) {
    // Send mouse motion data to PUI for potential processing
    puMouse (x, y);

    // Send mouse motion to veh interior panel
    CCockpitManager *pit = globals->pit;
    if (pit)  used = pit->MouseMove(x,y);
  }
  // Send mouse motion to cursor manager
  globals->cum->MouseMotion (x, y);
  // Force screen redraw
  glutPostRedisplay () ;
}

//============================================================================
// GLUT passive mouse motion event
//============================================================================
void passive_motion2 ( int x, int y )
{ bool used = false;
  globals->sScreen.bMouseOn = true;
  globals->mScreen.bMouseOn = false;
  globals->cScreen = &globals->sScreen;

  // Send mouse motion for cockpit manager for panel scrolling
  CCockpitManager *pit = globals->pit;
  if (pit)  used = pit->MouseMove(x,y);

  // Send mouse motion to cursor manager
  globals->cum->MouseMotion (x, y);

  // Force screen redraw
  glutPostRedisplay () ;
}
//============================================================================
//  PASSIVE MOTION (No click)
//============================================================================
void passive_motion ( int x, int y )
{ globals->cum->SetCursor(Cursor);
  globals->sScreen.bMouseOn = false;
  globals->mScreen.bMouseOn = true;
  globals->cScreen = &globals->mScreen;

  // Send mouse motion to PU
  bool used = (0 != puMouse (x, y));
  //--Not used then try Fui ----------------
  if (!used)  used = globals->fui->MouseMove (x, y);
  //-- Not used.  Try panel ----------------
  if (!used)
  {CCockpitManager *pit = globals->pit;
    if (pit)        pit->MouseMove(x,y);
  }
  // Send mouse motion to cursor manager
  globals->cum->MouseMotion (x, y);
  // Force screen redraw
  glutPostRedisplay () ;
}

//============================================================================
// GLUT keyboard event
//============================================================================
void keyboard2(unsigned char key, int x, int y)
{ CKeyMap *kbd = globals->kbd;
  globals->cScreen = &globals->sScreen;

  // Pass keystroke to PUI keyboard handler
  if (!puKeyboard (key, PU_DOWN)) {

    // Ignore keypresses if there is no keymap definition
    //   if (kbd == 0) return;

    // Convert GLUT modifiers to FlyLegacy modifiers
    int glutmod = glutGetModifiers ();
    EKeyboardModifiers flymod = glutModifiersToFlyLegacyModifiers (glutmod);

    // Get FlyLegacy key code
    EKeyboardKeys flykey;
    if (glutKeyToFlyLegacyKey (key, &flykey)) {
      kbd->KeyPress (flykey, flymod);
      return;
    }
    WARNINGLOG ("Unmapped GLUT key %d", key);
  }
}
//============================================================================
//  KEYBOARD MANAGEMENT
//  Ignore during all states except Simulation
//  During initial selection, go to default choice
//============================================================================
void keyboard (unsigned char key, int x, int y)
{ CKeyMap  *kbd = globals->kbd;
  globals->cScreen    = &globals->mScreen;
  switch(globals->appState) {
    case APP_LOADING_SCREEN:
      Choice = 1;
      return;
    case APP_SIMULATION:
      break;
    case APP_TEST:
      { int mod = glutGetModifiers ();
        EKeyboardModifiers mdf = glutModifiersToFlyLegacyModifiers (mod);
        globals->tsb->Keyboard(key,mod);
        return;
      }
    case APP_EXPORT:
      { int mod = glutGetModifiers ();
        EKeyboardModifiers mdf = glutModifiersToFlyLegacyModifiers (mod);     
        globals->exm->Keyboard(key,mdf);
        return;
      }
    default:
      return;
  }
  //---- Pass keystroke to PUI keyboard handler--
  if (puKeyboard (key, PU_DOWN))                            return; 
  //---- Pass Keystroke to FUI handler -----------
  int glutmod = glutGetModifiers ();
  if (globals->fui->KeyboardInput((glutmod << 16) | key))   return;
  //---- Ignore keypresses if there is no keymap definition
 
  EKeyboardModifiers flymod = glutModifiersToFlyLegacyModifiers (glutmod);
  // Get FlyLegacy key code
  EKeyboardKeys flykey;
  if (glutKeyToFlyLegacyKey (key, &flykey)) kbd->KeyPress (flykey, flymod);
  else                                      WARNINGLOG ("Unmapped GLUT key %d", key);
  return;
}

//================================================================================
// GLUT special keys event
//================================================================================
void special2(int key, int x, int y)
{ CKeyMap *kbd = globals->kbd;
  globals->cScreen = &globals->sScreen;

  // Pass keystroke to PUI keyboard handler 
  if (!puKeyboard (key + PU_KEY_GLUT_SPECIAL_OFFSET, PU_DOWN)) {


    // Get GLUT modifiers and convert to FlyLegacy modifiers
    int glutmod = glutGetModifiers ();
    EKeyboardModifiers flymod = glutModifiersToFlyLegacyModifiers (glutmod);

    // Get FlyLegacy key code
    EKeyboardKeys flykey;
    if (glutSpecialToFlyLegacyKey (key, &flykey)) 
    { kbd->KeyPress (flykey, flymod);
      return;
    }
  WARNINGLOG ("Unmapped GLUT key %d", key);
  }
}
//================================================================================
// GLUT special keys event
//================================================================================
void special (int key, int x, int y)
{ CKeyMap *kbd = globals->kbd;
  globals->cScreen = &globals->mScreen;
  //--- MODE TEST ------------------------------------------------------
	  if (globals->appState == APP_TEST)
  { int mod = glutGetModifiers ();
    EKeyboardModifiers mdf = glutModifiersToFlyLegacyModifiers (mod);
    globals->tsb->Special(key,mdf);
    return;
  }
  //--- MODE SIMULATION ------------------------------------------------
  if (globals->appState != APP_SIMULATION)                    return;
  //---- Pass Keystroke to PUI handler ---------------------------------
  if (puKeyboard (key + PU_KEY_GLUT_SPECIAL_OFFSET, PU_DOWN)) return;
  //---- Pass Keystroke to FUI Handler ---------------------------------
  int glutmod = glutGetModifiers ();
  if (globals->fui->KeyboardInput((glutmod << 16) | key))     return;
  //---- Simulator key event handling ----------------------------------
  // Ignore keypresses if there is no keymap definition
  // if (0 == kbd)                               return;
  // Get GLUT modifiers and convert to FlyLegacy modifiers
  EKeyboardModifiers flymod = glutModifiersToFlyLegacyModifiers (glutmod);
  // Get FlyLegacy key code
  EKeyboardKeys flykey;
  if (glutSpecialToFlyLegacyKey (key, &flykey)) kbd->KeyPress (flykey, flymod);
  else                                          WARNINGLOG ("Unmapped GLUT key %d", key);
  return;
}

//
// Window manager redraw callback
//
void redraw2 ()
{
  globals->cScreen = &globals->sScreen;

  // Clear the back buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );  

  switch (globals->appState) {
  case APP_EXIT_SCREEN:
    glutDestroyWindow(globals->sScreen.ID);
    globals->sScreen.ID = 0;
    break;

  case APP_SIMULATION:
    // Run simulation
    RedrawSimulation ();
    break;

    default:
      // Do nothing
    {}
  }
  glutPostRedisplay ();
  glutSwapBuffers ();
}
//===========================================================================
//  Check for test mode
//===========================================================================
bool CheckForTest()
{ int tst = 0;
  GetIniVar ("Sim", "TestMode", &tst);
  if (0 == tst) return false;
  //---- Create the test bed ------------------------
  globals->tsb = new CTestBed();
  return true;
}
//===========================================================================
//  REDRAW LOOP
//===========================================================================
void redraw ()
{ globals->cScreen = &globals->mScreen;

  // Clear the back buffer
  if (Clear) glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 

  switch (globals->appState) {
  case APP_SPLASH_SCREEN:
    // Display splash screen
    InitSplashScreen ();
    RedrawSplashScreen ();
    globals->appState = APP_INIT;
    break;

  case APP_INIT:
    // Perform application initialization
    InitApplication();
    Cursor = globals->cum->CrsrArrow();
    globals->appState = APP_GENERATE_SITUATION;
    break;

  case APP_GENERATE_SITUATION:
    // Currently nothing to do.

    /*!
     * \todo If use has selected 'Generate Situation' as the startup default,
     *       then display the dialog and wait for the user to create the
     *       situationbefore transitioning to the next state
     */
    globals->appState = APP_LOADING_SCREEN;
    break;

  case APP_LOADING_SCREEN:
    // Display selection screen.  Currently only one choice supported
    globals->appState = (CheckForTest())?(APP_TEST):(APP_LOAD_SITUATION); 
    break;

  case APP_LOAD_SITUATION:
    // Load situation
    InitSimulation ();
    globals->appState = APP_PREPARE;
    break;

  case APP_PREPARE:
    // Prepare situation for simulation
    globals->sit->Prepare ();
    globals->appState = APP_SIMULATION;
    Clear = 1;
    globals->tim->Update ();
    CleanupSplashScreen ();
    break;

  case APP_SIMULATION:
    // Run simulation
		globals->appState = (EAppState)RedrawSimulation ();
		/*
    __try {globals->appState = (EAppState)RedrawSimulation ();}
    __except(EXCEPTION_EXECUTE_HANDLER)										//(std::exception &e)
    { FatalExit();
			exit(-1);	} 
			*/
			break; 

  case APP_EXIT_SCREEN:
    // Display exit screen
    InitExitScreen ();
    RedrawExitScreen ();
    globals->appState = APP_EXIT;
    break;

  case APP_EXIT:
    // Exit application...program does not return from this function
    CleanupExitScreen ();
    ExitApplication ();
    break;

  case APP_TEST:
    Clear = 1;
    CleanupSplashScreen ();
    globals->tsb->TimeSlice();
    globals->tsb->Draw();
    break;

  case APP_EXPORT:
    Clear = globals->exm->TimeSlice(0.01f);
    break;
  }
  // Force screen redraw
  glutPostRedisplay ();
  glutSwapBuffers ();
}

//===========================================================================
// Initialize window manager
//===========================================================================
void InitWindowManager (int argc, char **argv)
{
  // Get screen resolution from globals
  int x = globals->mScreen.X;
  int y = globals->mScreen.Y;
  int w = globals->mScreen.Width;
  int h = globals->mScreen.Height;
  int bpp = globals->mScreen.Depth;
  int refresh = globals->mScreen.Refresh;

  // Get fullscreen mode from INI settings
  int autoFullScreen = 0;
  GetIniVar ("Graphics", "autoFullScreen", &autoFullScreen);
  globals->mScreen.full = autoFullScreen;
  // Initialise GLUT
  //MEMORY_LEAK_MARKER (">glutInit");
  glutInit (&argc, argv);
  //MEMORY_LEAK_MARKER ("<glutInit");
  glutInitDisplayMode (GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ALPHA);

  //MEMORY_LEAK_MARKER (">glutInitWindow");
  glutInitWindowPosition (x, y) ;
  glutInitWindowSize (w, h) ;
  glutCreateWindow ("Fly! Legacy") ;
  //MEMORY_LEAK_MARKER ("<glutInitWindow");
  if (autoFullScreen) glutFullScreen();  
  // Bind GLUT callback functions
#ifdef _WIN32
  glutIdleFunc (idle);
#endif // _WIN32
  glutMouseFunc (mouse);
  glutMotionFunc (motion);
  glutPassiveMotionFunc (passive_motion);
  glutDisplayFunc (redraw);
  glutReshapeFunc (reshape);
  glutKeyboardFunc (keyboard);
  glutSpecialFunc (special);
  
  // Disable GLUT cursor
  glutSetCursor (GLUT_CURSOR_NONE);
}

//===========================================================================
// Enter window manager main event loop
//===========================================================================
void EnterWindowManagerMainLoop (void)
{
  // Enter GLUT main loop.
  glutMainLoop ();
}

//===========================================================================
// Cleanup GLUT
//===========================================================================
void CleanupWindowManager (void)
{
  // Exit fullscreen mode
  glutLeaveGameMode ();
}
#endif // USE_GLUT
//================End of File ===============================================
