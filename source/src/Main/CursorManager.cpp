/*
 * CursorManager.cpp
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

/*! \file CursorManager.cpp
 *  \brief Implements CCursorManager to manage custom application cursors
 *
 *  Fly! Legacy disables normal OpenGL cursors and implements its own custom
 *   bitmap cursor manager in CCursorManager.  All cursors available to the
 *   application are defined in .CSR files; a number of default cursors
 *   are included in the standard installation, such as arrows, fingers,
 *   etc. Third-party content developers can also create custom cursors for
 *   use with their products.  The cursor manager is responsible for
 *   caching the bitmap cursors, keeping track of the currently active
 *   cursor and rendering it onscreen.
 */


#include "../Include/Globals.h"
#include "../Include/FlyLegacy.h"
#include "../Include/CursorManager.h"
#include "../Include/Utility.h"
#include "../Include/Ui.h"
#include "../Include/TerrainTexture.h"
using namespace std;


/*!
 * \todo For performance, cursors should be textured not bitmapped
 *       Use fixed-size 32x32 cursor area, construct texture image from bitmap
 *       data including transparency for index 0
 */
//=================================================================
//  Generic cursor from PBM and ACT files
//=================================================================
CCursor::CCursor()
{ curs[0] = 0;
  csid    = 0;
  imag[0] = 0;
  hots_x  = 16;
  hots_y  = 16;
	txd.pm  = 0;
}
//-----------------------------------------------------------
//  Destructor
//-----------------------------------------------------------
CCursor::~CCursor()
{ }
//-----------------------------------------------------------
//  Init Quad
//-----------------------------------------------------------
void CCursor::SetQuad(int wd)
{ //---Corner SW -----------------------
  qad[0].VT_S = 0;
  qad[0].VT_T = 0;
  qad[0].VT_X = 0;
  qad[0].VT_Y = 0;
  qad[0].VT_Z = 0;
  //---Corner SE -----------------------
  qad[1].VT_S = 1;
  qad[1].VT_T = 0;
  qad[1].VT_X = float(wd);
  qad[1].VT_Y = 0;
  qad[1].VT_Z = 0;
  //---Corner NE -----------------------
  qad[2].VT_S = 1;
  qad[2].VT_T = 1;
  qad[2].VT_X = float(wd);
  qad[2].VT_Y = float(wd);
  qad[2].VT_Z = 0;
  //---Corner NW -----------------------
  qad[3].VT_S = 0;
  qad[3].VT_T = 1;
  qad[3].VT_X = 0;
  qad[3].VT_Y = float(wd);
  qad[3].VT_Z = 0;

  return;
}
//-----------------------------------------------------------
//  Draw the quad
//-----------------------------------------------------------
void CCursor::Draw()
{ glBindTexture (GL_TEXTURE_2D, txd.xo);
  glEnable(GL_BLEND);
  // Draw a quad, textured with the current cursor texture id
  glInterleavedArrays(GL_T2F_V3F,0,qad);
  glDrawArrays(GL_QUADS,0,4);
  return;
}
//--------------------------------------------------------------
//  Load the bitmap according to extension and build a RGBA texture
//--------------------------------------------------------------
void CCursor::LoadCursor(char* name)
{ char *dot = strrchr (name, '.');
  _strupr(name);
  if (0 == dot)   gtfo("No such cursor %s",name);
  if (strcmp(dot,".BMP") != 0)  gtfo("Cursor %s format not supported");
  //---- Decode the BMP cursor -------------------------
  globals->txw->LoadAnyTexture(name,txd);
	SetQuad(txd.wd);
  return;
}
//-----------------------------------------------------------
//  Set Hot spot
//-----------------------------------------------------------
void CCursor::HotSpot()
{	hots_x = (txd.wd >> 1);
	hots_y = (txd.ht >> 1);
	return;
}
//--------------------------------------------------------------
//  Create OpenGL texture
//--------------------------------------------------------------
void CCursor::CreateTexture (void)
{ // Generate non-mipmapped OpenGL texture
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  glGenTextures (1, &txd.xo);
  glBindTexture (GL_TEXTURE_2D, txd.xo);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, txd.wd, txd.ht, 0, GL_RGBA, GL_UNSIGNED_BYTE, txd.rgba);
  // Clean up dynamically allocated data
  delete txd.rgba;
  txd.rgba  = 0;
  return;
}
//-----------------------------------------------------------
//  Free the texture
//-----------------------------------------------------------
void CCursor::FreeTexture (void)
{ glDeleteTextures (1, &txd.xo);
  txd.xo = 0;
}


/***************************************************

  Fly! II Cursors

  Cursors are specified in a text file with extension .csr, stored in the
  \DATA folder.  The following example is \DATA\MPARROW.CSR in STARTUP.POD:

<bgno> ---- BEGIN CURSOR ----
<csid> ---- cursor id ----
arrw
<imag> ---- image file ----
mparrow.pbm
<hots> ---- cursor x,y hotspot ----
0
0
<endo> ---- END CURSOR ----

 ***************************************************/


//============================================================
// Constructor
//=============================================================
CFlyCursor::CFlyCursor (const char* csrfilename)
: CCursor()
{
  // Initialize fields
  strncpy (curs,csrfilename,64);
  // Open .CSR file stream and parse tags
  SStream s(this,"DATA",(char*)csrfilename);
}

//--------------------------------------------------------------
// Destructor
//--------------------------------------------------------------
CFlyCursor::~CFlyCursor (void)
{
  
}
//--------------------------------------------------------------
//  Read tag from CSR file
//--------------------------------------------------------------
int CFlyCursor::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;
  char curs[64];
  switch (tag) {
  case 'csid':
    // Read unique ID
    ReadTag (&csid, stream);
    return TAG_READ;

  case 'imag':
    // Read .BMP image filename
    ReadString (imag, 64, stream);
    return TAG_READ;

  case 'curs':
    // Read OS cursor resource name (not currently used)
    ReadString (curs, 64, stream);
    return TAG_READ;

  case 'hots':
    // Read hotspot offsets
    ReadInt (&hots_x, stream);
    ReadInt (&hots_y, stream);
		txd.pm	= 1;
    return TAG_READ;

  default:
    WARNINGLOG ("CCursor::Read : Unknown tag <%s>", TagToString(tag));
  }

  return rc;
}
//-------------------------------------------------------------------
//  Read finished.  Load the PBM-ACT file from the ART directory
//-------------------------------------------------------------------
void CFlyCursor::ReadFinished (void)
{ // Load image bitmap
  char fn[64];
  _snprintf(fn,63,"ART/%s",imag);
	fn[63] = 0;
  if (imag[0]) LoadCursor(fn);
  return;
}

//==================================================================
// CursorManager
//==================================================================
CCursorManager::CCursorManager (void)
{ globals->cum  = this;
  x = y = 0;
  cCur    = 0;
	state		= GetMenuState();					// Menu initial state
  Time    = 0;
  crsrArrow = BindCursor ("MPARROW.CSR");
}
//-----------------------------------------------------------------
//  Destroy resources
//-----------------------------------------------------------------
CCursorManager::~CCursorManager()
{ std::map<Tag,CCursor*>::iterator ra;
  for (ra=cache.begin(); ra!=cache.end(); ra++) delete ra->second;
  cache.clear();
}
//-----------------------------------------------------------------
//  Get Cursor arrow
//-----------------------------------------------------------------
Tag CCursorManager::CrsrArrow() {
  if (crsrArrow == 0) crsrArrow = BindCursor ("MPARROW.CSR");
  return crsrArrow;
}
//-----------------------------------------------------------------
// MouseMotion
//-----------------------------------------------------------------
void CCursorManager::MouseMotion (int mouse_x, int mouse_y)
{ if (0 == cCur)		return;
  x = mouse_x - cCur->GetHotx();
	y = globals->cScreen->Height - mouse_y - 1 + cCur->GetHoty();
//	SetMenuState(state);
	Time	= 30;
	return;
}

//----------------------------------------------------------------
//  Find cursor in cache by name
//----------------------------------------------------------------
CCursor *CCursorManager::FindCursor (const char *csrfilename)
{ std::map<Tag,CCursor*>::iterator iter;
  for (iter=cache.begin(); iter!=cache.end(); iter++) {
    CCursor *crsr = iter->second;
    if (crsr->IsThisOne(csrfilename)) return crsr;
  }
  return 0;
}
//---------------------------------------------------------------
//  Find cursor by Tag ID
//---------------------------------------------------------------
CCursor *CCursorManager::FindCursor (Tag tag)
{ CCursor *rc = NULL;
  std::map<Tag,CCursor*>::iterator iter = cache.find(tag);
  if (iter != cache.end()) {
    rc = iter->second;
  }
  return rc;
}
//---------------------------------------------------------------
// BindCursor for FLYII cursor defined in CSR file
//
// Applications call this method to load the specified cursor bitmap into cache.
//   The method returns a handle to the cursor that is used in SetCursor().
//  NOTE:  This is for FLYII cursor.  For Fui see BindFuiCursor
//---------------------------------------------------------------
Tag CCursorManager::BindCursor (const char *csrfile)
{ CCursor *crsr = FindCursor (csrfile);
  if (crsr)   return crsr->GetKey();
  // Try to load it
  //MEMORY_LEAK_MARKER ("crsr1");
  crsr = new CFlyCursor (csrfile);
  //MEMORY_LEAK_MARKER ("crsr1");
  cache[crsr->GetKey()] = crsr;
  return crsr->GetKey();
}
//---------------------------------------------------------------
//  BindCursor for other cursor
//---------------------------------------------------------------
Tag CCursorManager::BindFuiCursor(char *name,Tag key)
{ char fui[64];
  CCursor *crsr = FindCursor (name);
  if (crsr) return crsr->GetKey();
  // Load Cursor from FUI directory
  _snprintf(fui,63,"UI/CURSORS/%s",name);
	fui[63] = 0;
  // MEMORY_LEAK_MARKER ("crsr2");
  crsr = new CCursor();
  // MEMORY_LEAK_MARKER ("crsr2");
  crsr->SetName(name);
  crsr->LoadCursor(name);
	crsr->HotSpot();
  crsr->SetKey(key);
  cache[key] = crsr;
  return crsr->GetKey();
}
//--------------------------------------------------------------------
//   Change current cursor
//   arm timer to 30 seconds
//--------------------------------------------------------------------
ECursorResult CCursorManager::SetCursor (Tag tag)
{ CCursor *ncur = FindCursor(tag);
  if (0 == ncur)              return CURSOR_NOT_CHANGED;
  cCur = ncur;
  if (0 == cCur->GetTextId()) cCur->CreateTexture();
//	SetMenuState(state);
	Time	= 30;
  return CURSOR_WAS_CHANGED;
}
//--------------------------------------------------------------------
//  Return current cursor tag
//--------------------------------------------------------------------
Tag CCursorManager::GetCursor (void)
{ return (cCur)?(cCur->GetKey()):(0);
}
//--------------------------------------------------------------------
// Hide cursor and menu
//--------------------------------------------------------------------
void CCursorManager::HideAll()
{	state = GetMenuState();
	SetMenuState(0);
	return;
}
//--------------------------------------------------------------------
//  Draw the cursor
//--------------------------------------------------------------------
void CCursorManager::Draw (float dT)
{ if (Time < 0)		return;
  if (0 == cCur)  return;
	Time -= dT;
	if (Time < 0)		SetMenuState(0);
  glColor3f (1, 1, 1);
  // Set projection matrix to 2D screen size
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity ();
  gluOrtho2D (0, globals->cScreen->Width, 0, globals->cScreen->Height);

  // Position cursor onscreen
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  glLoadIdentity ();
  glTranslatef (float(x), float(y), 0);
  glPushAttrib (GL_ENABLE_BIT);
  glDisable (GL_DEPTH_TEST);
  glEnable (GL_TEXTURE_2D);
  cCur->Draw();
    // Restore original settings
  glPopAttrib ();
  glMatrixMode (GL_PROJECTION);
  glPopMatrix ();
	glMatrixMode (GL_MODELVIEW);
  glPopMatrix ();

 // CHECK_OPENGL_ERROR
  return;
}
//=======END of FILE =================================================================
