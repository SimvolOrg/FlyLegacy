/*
 * Gauges.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
 * Copyright 2005 Laurent Claudet
 * Copyright 2007 Jean Sabatier
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

/**
 * @file Gauges.cpp
 * @brief Implements CGauge and related classes for panel gauges
 *
 * Implementation of the many different varieties of panel gauge
 *   classes, including CGauge and all descendents.  Also includes
 *   the implementation of related classes such as CGaugeNeedle,
 *   CGaugeClickArea, etc.
 */


#include "../Include/Gauges.h"
#include "../Include/Ui.h"
#include "../Include/Globals.h"
#include "../Include/Subsystems.h"
#include "../Include/Fui.h"
#include "../Include/AudioManager.h"
#include "../Include/CursorManager.h"

///////// DEBUG STUFF TO REMOVE LATER ////////////////////////////////
#ifdef  _DEBUG	
  #define _DEBUG_sios	    //print lc DDEBUG file ... remove later
#endif
//////////////////////////////////////////////////////////////////////
void ReadUserTag(SMessage *msg,SStream *st);
//=========================================================================
//  This is a standard QUAD (SW-SE-NE-NW)
//=========================================================================
TC_VTAB StdQUAD[4] = {
	{0, 0, 0, 0, 0,},			// SW
	{1, 0, 0, 0, 0,},			// SE
	{0, 1, 0, 0, 0,},			// NW
	{1, 1, 0, 0, 0,},			// NE
};
//=========================================================================
//  Global table giving station (VOR or ILS sensibility) in Frame per degre
//=========================================================================
int SensTAB[] = {
  20,                               // VOR shows 20° deviation
  10,                               // ILS shows 10° deviation
};
//=========================================================================
//  Flag text
//=========================================================================
char *flagTAB[] = {
  "OFF",
  "TO",
  "FROM",
};
//====================================================================
// CGaugeNeedle:  DEPRECATED.  DO NOT USE
//=====================================================================
CGaugeNeedle::CGaugeNeedle (void)
{ nFrames = 0;
  sizx = sizy = 0;
  offx = offy = 0;
  mind = 0;
  maxd = 360;
  strcpy (base, "");
  bmap = AssignNullBitmap();
}
//--------------------------------------------------------------
//  Free the resources
//---------------------------------------------------------------
CGaugeNeedle::~CGaugeNeedle (void)
{
	SAFE_DELETE(bmap);
}
//---------------------------------------------------------------
//  Read parameters
//---------------------------------------------------------------
int CGaugeNeedle::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'file':
    // Refers to a NDL stream file containing the needle specification
    //  JS: Replace any suffixe like .PBG or other
    {
      char file[64];
      ReadString (file, 64, stream);
      strupper(file);
      char *dot = strchr(file,'.');
      if (dot && (strcmp(dot,".PBM") == 0))
        strcpy(dot,".NDL");
      if (dot && (strcmp(dot,".PBG") == 0))
        strcpy(dot,".NDL");
      SStream s;
      if (OpenRStream ("DATA",file,s)) {
        ReadFrom (this, &s);
        CloseStream (&s);
      }
      return TAG_READ;
    }
    break;

  case 'shar':
    // This needle is shared across multiple aircraft.  For now this has no effect
    return TAG_READ;

  case 'sizx':
    ReadShort (&sizx, stream);
    return TAG_READ;

  case 'sizy':
    ReadShort (&sizy, stream);
    return TAG_READ;

  case 'offx':
    ReadShort (&offx, stream);
    return TAG_READ;

  case 'offy':
    ReadShort (&offy, stream);
    return TAG_READ;

  case 'mind':
    ReadFloat(&mind, stream);
    return TAG_READ;

  case 'maxd':
    ReadFloat(&maxd, stream);
    return TAG_READ;

  case 'base':
    ReadString (base, 64, stream);
    return TAG_READ;
  }

  if (rc == TAG_IGNORED) {
    WARNINGLOG("CGaugeNeedle::Read : Unknown tag %s", TagToString(tag));
  }

  return rc;
}
//---------------------------------------------------------------------
//  Decode name
//----------------------------------------------------------------------
void CGaugeNeedle::DecodeName(char *nn)
{ char name[128];
  if (0 == *base)     return;
  _snprintf(name,59,"ART/%s",base);
	name[63] = 0;
  char *dot = strrchr(name,'.');
  if (0 == dot) strcat(name,".PBG");
  strncpy(nn,name,64);
  return;
 }
//---------------------------------------------------------------------
//  All parameters are read
//----------------------------------------------------------------------
void CGaugeNeedle::ReadFinished (void)
{ char txt[128];
  rang = maxd - mind;
  if (nFrames)    return;
  if (base[0])
	{
    DecodeName(txt);
    // Load bitmap
    bmap = new CBitmap(txt);
    nFrames = bmap->NumBitmapFrames ();

    // <offx> and <offy> represent offsets of the center of the needle;
    //   adjust for the top-left corner for use in Draw() if they were specified
    int w, h;
    bmap->GetBitmapSize (&w, &h);
    hw    = ((w+1) >> 1);
    hh    = ((h+1) >> 1);
    if (offx != 0)  offx -= hw;
    if (offy != 0)  offy -= hh;
  }
}
//------------------------------------------------------------------------
//  Draw the needle in [0,360°]
//-------------------------------------------------------------------------
void CGaugeNeedle::Draw (SSurface *surface, float degrees, int cx, int cy)
{ degrees = Wrap360(degrees);
  //--- Clamp to the min/max degrees ---------------------------
  if (degrees < mind) degrees = mind;
  if (degrees > maxd) degrees = maxd;
  //--- Convert degrees to frame number-------------------------
  int frame = int(nFrames * (degrees - mind) / rang);
  if (frame <    0) return;
  if (frame >  359) return;
  // cx, cy indicate offset to gauge center in the surface; calculate
  //   offset of top-left corner of needle bitmap within gauge
  /// \todo How do needle offx/offy and sizx/sizy factor into this?
  //------ Adjust by half-width and half-height, rounding up---------
  cx -= hw;
  cy -= hh;
  //-------Finally draw the animation frame on the supplied surface--
  bmap->DrawBitmap (surface, offx + cx, offy + cy, frame);
  return;
}
//-------------------------------------------------------------------------
void CGaugeNeedle::LoadExtBitmap (const char *bitmap_name)
{
	char name[64];
  if (bitmap_name[0])
  {
    _snprintf(name,63,"ART/%s", bitmap_name);
		name[63] = 0;
    bmap = new CBitmap(name);
  }
  strncpy(base, bitmap_name, 64); // only 64 bytes
  nFrames = bmap->NumBitmapFrames ();
}

//==============================================================================
// CGaugeRectangle
//==============================================================================
CGaugeRectangle::CGaugeRectangle (void)
{ Type = AREA_NONE;
  x1 = y1 = x2 = y2 = 0;
}
//--------------------------------------------------------------
//  Read parameters
//--------------------------------------------------------------
int CGaugeRectangle::Read (SStream *stream, Tag tag)
{switch (tag) {
  case 'rect':
    ReadShort (&x1, stream);
    ReadShort (&y1, stream);
    ReadShort (&x2, stream);
    ReadShort (&y2, stream);
    Type = AREA_BOTH;
    return TAG_READ;
  }
  WARNINGLOG("CGaugeRectangle::Read : Unknown tag %s", TagToString(tag));
  return TAG_IGNORED;
}



//==============================================================================
// CGaugeClickArea
//  Gauge area are of several types
//  default is AREA_CLIK
//==============================================================================
CGaugeClickArea::CGaugeClickArea (void)
{	mgg     = 0;
  quad    = 0;
  Type    = AREA_CLIK;
  cursTag = 0;
	*htxt   = 0;
}
//-------------------------------------------------------------
//	Destructor
//-------------------------------------------------------------
CGaugeClickArea::~CGaugeClickArea (void)
{ SAFE_DELETE(quad);
}

//-------------------------------------------------------------
//	Bind cursor
//-------------------------------------------------------------
void CGaugeClickArea::BindCursor(SStream *str)
{	char cart[64] = {0};
  ReadString (cart, 64, str);
	if (cart[0] == 0)	return;
  cursTag = globals->cum->BindCursor (cart);
	return;
}
//-------------------------------------------------------------
//	Decode digit position inside display
//-------------------------------------------------------------
void CGaugeClickArea::ReadPOSN(SStream *str)
{ char tex[128];
  int px,py;
  int wd,ht;
  ReadString(tex,128,str);
  int nf = sscanf(tex,"%d , %d , %d , %d ",&px, &py, &wd, &ht);
  if (nf != 4)  gtfo("Gauge: Invalide <posn> tag");
  x1  = px;
  y1  = py;
  x2  = px + wd;
  y2  = py + ht;
  return;
}

//-------------------------------------------------------------
//	Decode digit position inside display
//-------------------------------------------------------------
void CGaugeClickArea::ReadABSP(SStream *str)
{ char tex[128];
  int px,py;
  int wd,ht;
  ReadString(tex,128,str);
  int nf = sscanf(tex,"%d , %d , %d , %d ",&px, &py, &wd, &ht);
  if (nf != 4)  gtfo("Gauge: Invalide <absp> tag");
  //--- Adjust parameters to be relative to gauge origine ----
  x1  = px - mgg->GetXPos();
  y1  = py - mgg->GetYPos();
  x2  = x1 + wd;
  y2  = y1 + ht;
  return;
}
//-------------------------------------------------------------
//	Decode character position inside display
//  NOTE:  Position are given in character units relative to display
//-------------------------------------------------------------
void CGaugeClickArea::ReadCHRP(SStream *str)
{ char tex[128];
  int px,py;
  int wd,ht;
  ReadString(tex,128,str);
  int nf = sscanf(tex,"%d , %d , %d , %d",&px, &py, &wd, &ht);
  if (nf != 4)  gtfo("Gauge: Invalide <chrp> tag");
  //--- Adjust parameters to be relative to display origine ----
  px *= mgg->CharWD();
  py *= mgg->CharHT();
  wd *= mgg->CharWD();
  ht *= mgg->CharHT();
  x1  = px;
  y1  = py;
  x2  = x1 + wd;
  y2  = y1 + ht;
  return;
}
//-------------------------------------------------------------
//	Decode character origin inside display
//  NOTE:  Position are given in character units relative to display
//         and are adjusted to gauge origin
//-------------------------------------------------------------
void CGaugeClickArea::ReadGPOS(SStream *str)
{ char tex[128];
  int px,py;
  int wd,ht;
  ReadString(tex,128,str);
  int nf = sscanf(tex,"%d , %d , %d , %d",&px, &py, &wd, &ht);
  if (nf != 4)  gtfo("Gauge: Invalide <gpos> tag");
  //--- Adjust parameters to be relative to display origine ----
  px *= mgg->CharWD();
  px += mgg->GetDisXOrg();
  py *= mgg->CharHT();
  py += mgg->GetDisYOrg();
  wd *= mgg->CharWD();
  ht *= mgg->CharHT();
  x1  = px;
  y1  = py;
  x2  = x1 + wd;
  y2  = y1 + ht;
  return;
}
//-------------------------------------------------------------
//	Overwrite all parameters
//-------------------------------------------------------------
void CGaugeClickArea::Set(short px, short py, short wd, short ht, char type)
{ x1  = px;
  y1  = py;
  x2  = px + wd;
  y2  = py + ht;
  Type  = type;
  return;
}
//-------------------------------------------------------------
//	Init CA with mother gauge and type
//-------------------------------------------------------------
void CGaugeClickArea::Init(CGauge *mg, char tp)
{ mgg   = mg;
  Type  = tp;
  return;
}
//-------------------------------------------------------------
//	Read click area tags
//-------------------------------------------------------------
int CGaugeClickArea::Read (SStream *stream, Tag tag)
{
	switch (tag) {
    //--- Bitmap --------------------------
    case 'bmap':
      ReadStrip(stream,txd);
      return TAG_READ;
    //--- texture ------------------------
    case 'pict':
      ReadStrip(stream,txd);
      return TAG_READ;
    //--- Cursor --------------------------
	  case 'curs':
			BindCursor (stream);
			return TAG_READ;
    //--- Help text -----------------------
	  case 'help':
			ReadString (htxt, 64, stream);
			return TAG_READ;
    //---Define digit position ------------
    case 'posn':
      ReadPOSN(stream);
      return TAG_READ;
    //---Define absolute position ---------
    case 'absp':
      ReadABSP(stream);
      return TAG_READ;
    //---Define character position --------
    case 'chrp':
      ReadCHRP(stream);
      return TAG_READ;
    //---Define character position --------
    case 'gpos':
      ReadGPOS(stream);
      return TAG_READ;

    //---Define a click area --------------
    case 'rect':
      if (Type & AREA_DIGI) gtfo("<rect> is invalid in Digit, use <posn> instead");
      ReadShort (&x1, stream);
      ReadShort (&y1, stream);
      ReadShort (&x2, stream);
      ReadShort (&y2, stream);
      return TAG_READ;
    //---Can repeat click ----------------
    case 'rept':
      Type |= AREA_REPT;
      return TAG_READ;
    //-- Define a click only area --------
    case 'clko':
      Type    = AREA_CLIK;
      return TAG_READ;
	}
	//--- see upper level ------------------- 
  return TAG_IGNORED;
}
//--------------------------------------------------------------
//  Build the quad if defined for display
//  mgg is the mother gauge
//  NOTE: For digit, detection parameters (x1,y1,x2,y2) must be adjusted
//        by adding the offset of the display area as
//        we need relative coordinates to gauge origin for Hit
//        detection
//--------------------------------------------------------------
void CGaugeClickArea::BuildQuad()
{ //-----Build a quad for button and area --------------
  if (Type & AREA_QUAD) 
  { int wd = x2 - x1;       // Width
    int ht = y2 - y1;       // eight
    quad   = new TC_VTAB[4];
		vOfs   = mgg->FixRoom(4);
    int sx = mgg->GetXPos() + x1;
    int sy = mgg->GetYPos() + y1;
    mgg->BuildQuad(quad,sx,sy,wd,ht);
  }
  //---- Adjust detection if digit inside display --

  if (Type & AREA_DIGI)
  { x1 += mgg->GetDisXOrg();
    x2 += mgg->GetDisXOrg();
    y1 += mgg->GetDisYOrg();
    y2 += mgg->GetDisYOrg();
  }
  //----- Adjust y for bitmap -------------------
  return;
}
//--------------------------------------------------------------
//  CopyRectangle
// 
//--------------------------------------------------------------
void CGaugeClickArea::CopyRECT(CGaugeClickArea &s)
{ Type  = s.Type;
  x1    = s.x1;
  x2    = s.x2;
  y1    = s.y1;
  y2    = s.y2;
  return;
}
//--------------------------------------------------------------
//  Copy data from the ClickArea given in parameters
//	Never copy the texture
//--------------------------------------------------------------
void CGaugeClickArea::CopyFrom(CGaugeClickArea &s,CGauge *mg)
{ mgg       = mg;
  cursTag   = s.cursTag;
  quad      = 0;
  strncpy(htxt,s.htxt,64);
  CopyRECT(s);
  return;
}
//--------------------------------------------------------------
//	Collect the VBO Data
//--------------------------------------------------------------
void CGaugeClickArea::CollectVBO(TC_VTAB *qd)
{	if (quad)
	{	TC_VTAB *dst = qd + vOfs;
		for (U_SHORT k = 0; k<4; k++) *dst++ = quad[k];
	}
	return;
}
//--------------------------------------------------------------
//  Draw the appropiated texture
//--------------------------------------------------------------
void CGaugeClickArea::Draw(int nf)
{ if (0 == txd.rgba)    return;
  if (0 == quad)        return;
  int No  = txd.nf - 1 - nf;
  No      = txd.Clamp(No);
  U_INT *base = (U_INT*)txd.rgba + (No * txd.dm);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,txd.wd,txd.ht,0,GL_RGBA,GL_UNSIGNED_BYTE,base);
	glDrawArrays(GL_TRIANGLE_STRIP,vOfs,4);
  return;
}
//--------------------------------------------------------------
//  Draw texture on surface
//--------------------------------------------------------------
void CGaugeClickArea::DrawOnSurface (SSurface *s, int f)
{ if (0 == s)   return;
  DrawTextureOn(txd,*s,x1,y1,f);
}
//--------------------------------------------------------------
//  Check if the area is clicked
//  NOTE : Area of type 2 are display only
//--------------------------------------------------------------
bool CGaugeClickArea::IsHit (int x, int y)
{ if (NoClick()) return false;
  return ((x >= x1) && (x <= x2) && (y >= y1) && (y <= y2));
}
//--------------------------------------------------------------
//  Check if the mouse is in the area
//--------------------------------------------------------------
ECursorResult CGaugeClickArea::MouseMoved (int x, int y)
{ // Check that the position is within click area bounds
  if (!IsHit(x,y))  return CURSOR_NOT_CHANGED;
  if (mgg)  mgg->DisplayHelp(htxt);
  globals->cum->SetCursor (cursTag);
  return (*htxt)?(CURSOR_WAS_CHANGED):(CURSOR_NOT_CHANGED);
}
//=================================================================================
//		CGaugeKnob
//=================================================================================
CGaugeKnob::CGaugeKnob (char Type,float rev)
{
  knob_surf = NULL;
  bmKnob	= NULL;
  NbFrame	= 0;
  VaRev		= rev;
  LoVal		= 0;						// Low range
  HiVal		= 0;						// High range
  Actual	= 0;						// Actual value
  CState	= 2;						// Initial state
  CType		= Type;						// Type value
}
//--------------------------------------------------------------
CGaugeKnob::~CGaugeKnob (void)
{
	SAFE_DELETE(bmKnob);

  if (knob_surf) knob_surf = FreeSurface (knob_surf);
}

//--------------------------------------------------------------
//	Set the knob bitmap
//--------------------------------------------------------------
void CGaugeKnob::SetBitmap(SStream *str)
{	char name[64] = {0};
	char kart[64];
  ReadString (name, 64, str);
	if (name[0] == 0)	return;
	_snprintf(kart,63,"ART/%s", name);
	kart[63] = 0;
  bmKnob = new CBitmap(kart);
	NbFrame = bmKnob->NumBitmapFrames();
	return;
}
//---------------------------------------------------------------
//	Read knob tags
//---------------------------------------------------------------
int CGaugeKnob::Read (SStream *stream, Tag tag)
{	int rc = TAG_IGNORED;
	switch (tag) {
		case 'bmap':
			SetBitmap(stream);
			return TAG_READ;
	}
	// --- see upper level ---------------------
    rc = CGaugeClickArea::Read (stream, tag);
	if (rc == TAG_IGNORED) {
		WARNINGLOG("CGaugeClickArea::Read : Unknown tag %s", TagToString(tag));
	}
  return rc;
}
//-------------------------------------------------------------------
//	Read terminated
//-------------------------------------------------------------------
void CGaugeKnob::ReadFinished(void)
{	if (bmKnob == NULL)	return;
	knob_surf = CreateSurface ((x2 - x1 + 1), (y2 - y1 + 1));
	EraseSurfaceRGBA(knob_surf, 0);
	return;
}
//----------------------------------------------------------------------
void CGaugeKnob::SetOffset (int offset_x, int offset_y)
{
  this->offset_x = offset_x;
  this->offset_y = offset_y;
}
//----------------------------------------------------------------------
//	Set the rotation range
//----------------------------------------------------------------------
void CGaugeKnob::SetRange(float lo,float hi,float act)
{	Actual	= act;
	LoVal	  = lo;
	HiVal	  = hi;
	return;
}
//----------------------------------------------------------------------
//	Arm the knob for rotation
//----------------------------------------------------------------------
void CGaugeKnob::ArmRotation(float adj,int button)
{	CState	= 1;								// Active state
	maxTM	= 0.5f;								// Slow rate
	Timer	= maxTM;							// Fisrt tick
	Adjus	= 0;
	if (button & MOUSE_BUTTON_LEFT )	Adjus	= +adj;
	if (button & MOUSE_BUTTON_RIGHT)	Adjus	= -adj;
	return;
}
//-----------------------------------------------------------------------
//	Get the knob frame
//-----------------------------------------------------------------------
int	CGaugeKnob::GetFrame(void)
{	if (NbFrame == 0)	return 0;
	float	 val  = (Actual - LoVal);				// Relative value
	float	 rem  = fmodf(val,VaRev);				// Remaining rev
	int		 nf   = (NbFrame * rem) / VaRev;		// Frame 
	return nf;	}
//-----------------------------------------------------------------------
//	Check knob change if any
//-----------------------------------------------------------------------
bool CGaugeKnob::HasChanged()
{	if (CState == 2) {CState = 0;	return true;}		// First time
	if (CState == 0)				      return false;		// No change
	//--- Timer is active ----------------------------
	float dT	= globals->dST;
	Timer	+= dT;
	if (Timer < maxTM)				    return false;			// No change yet
	Timer	 = 0;
	if (maxTM > 0.05)	maxTM -= 0.05f;						    // Accelerate
	Actual	+= Adjus;										            // Change total
	switch (CType)	{
		case CLAMP:
			//-----Clamp value into range -----------------------
			if (Actual < LoVal)	Actual = LoVal;				// clamp to lower
			if (Actual > HiVal) Actual = HiVal;				// clamp to highest
			return	true;

		case DEGRE:
			//---- stay in modulus ------------------------------
			if (Actual <  LoVal) Actual += HiVal;			// Stay in modulo
			if (Actual >= HiVal) Actual  = LoVal;			// Stay in modulo
			return true;
	}
	return false;
}
//----------------------------------------------------------------------
//	Redraw the knob 
//----------------------------------------------------------------------
void CGaugeKnob::Draw (void)
{	if ((bmKnob == NULL) || (knob_surf == NULL))	return;
	int frame = GetFrame();
  // Draw the knob animation frame
  EraseSurfaceRGBA(knob_surf,0);
  knob_surf->xScreen = offset_x + x1;
  knob_surf->yScreen = offset_y + y1;
  bmKnob->DrawBitmap (knob_surf, 0, 0, frame);
}


//==========================================================================
//  CVariator:  Increment or Decrement in a contineous fashion and
//              send message on a timer basis
//==========================================================================
CVariator::CVariator()
{ state = 2;
  Tmax  = 0.5;
  Incr  = 0;
}
//---------------------------------------------------------------------
//  Init message
//---------------------------------------------------------------------
void CVariator::Init()
{ msg.sender    = 'Vari';
  msg.id		    = MSG_SETDATA;
  msg.dataType  = TYPE_REAL;
  return;
}
//---------------------------------------------------------------------
//  Check for change
//---------------------------------------------------------------------
void CVariator::SendValue(float inc)
{ msg.id		    = MSG_GETDATA;
  Send_Message(&msg);
  float val     = msg.realData + inc;
  if (val < vmin) val = vmin;
  msg.realData  = val;
  msg.id		    = MSG_SETDATA;
  Send_Message(&msg);
  return;
}
//---------------------------------------------------------------------
//  Arm the system
//---------------------------------------------------------------------
void CVariator::Arm(float inc)
{	state		= 1;						            // Armed
	Time		= 0;				                // For fist move
	Incr		= inc;
  SendValue(inc);
	return;
}
//---------------------------------------------------------------------
//  Check for change
//---------------------------------------------------------------------
bool CVariator::HasChanged()
{	if (state == 2) {state = 0;	return true;}		// First time
	if (state == 0)					    return false;		// No change
	//--- Timer is active ----------------------------
	float dT	= globals->dST;
	Time	+= dT;
	if (Time < Tmax)			      return false;	  // No change yet
	Time	 = 0;
  //---Change value     -------------------------------
  SendValue(Incr);
	return	true;												        // Return change
}
//==========================================================================
//  CKnob class with shared surface
//==========================================================================
CKnob::CKnob()
{ Init(0);
}
//---------------------------------------------------------------------
//  Constructor for knob with owned surface
//---------------------------------------------------------------------
CKnob::CKnob(CGauge *g)
{ Init(g);
}
//---------------------------------------------------------------------
//  Init knob area
//---------------------------------------------------------------------
void CKnob::Init(CGauge *g)
{ CType     = PN_DEGRE;
  ownS      = (g)?(1):(0);
  nFrame    = 0;
  cFrame    = 0;
  CState    = 2;
  help[0]   = 0;
  ksurf     = 0;
  mg        = g;
  bmap	    = AssignNullBitmap();
  //--- Assign mother gauge dimension --------
  x1 = x2 = y1 = y2 = 0;
  cursTag   = 0;
  Change    = 0;
}
//---------------------------------------------------------------------
//  Free the resources
//---------------------------------------------------------------------
CKnob::~CKnob()
{ if (ownS)  FreeSurface(ksurf);
  if (bmap)  delete bmap;
}
//---------------------------------------------------------------------
//  Read parameters (Read from)
//---------------------------------------------------------------------
int CKnob::Read(SStream *stream, Tag tag)
{ switch (tag) {
		case 'bmap':
      { char name[64] = {0};
	      char art[64];
        ReadString (name, 64, stream);
	      if (name[0] == 0)	TAG_READ;
	      _snprintf(art,63, "ART/%s", name);
				art[63] = 0;
        bmap = new CBitmap(art);
	      nFrame = bmap->NumBitmapFrames() - 1;
        return TAG_READ;  }
    case 'rect':
      { ReadShort (&x1, stream);
        ReadShort (&y1, stream);
        ReadShort (&x2, stream);
        ReadShort (&y2, stream);
        return TAG_READ;  }
    case 'curs':
      { char cart[64] = {0};
        ReadString (cart, 64, stream);
	      if (cart[0] == 0)	TAG_READ;
        cursTag = globals->cum->BindCursor (cart);
        return TAG_READ;  }
    case 'help':
      { ReadString (help, 64, stream);
        return TAG_READ;  }
  }
  return TAG_IGNORED;
}
//---------------------------------------------------------------------
//  All parameters are read
//---------------------------------------------------------------------
void CKnob::ReadFinished(void)
{	if (bmap == NULL)	return;
  int wd = (x2 - x1 + 1);
  int ht = (y2 - y1 + 1);
	ksurf = CreateSurface (wd, ht);
  ksurf->xScreen = mg->GetXPos() + x1;
  ksurf->yScreen = mg->GetYPos() + y1;
	EraseSurfaceRGBA(ksurf, 0);
  mg->BuildQuad(quad,ksurf->xScreen, ksurf->yScreen,wd,ht);
	return;
}
//---------------------------------------------------------------------
//	Read rotating knob parameters
//  mg is the mother gauge
//---------------------------------------------------------------------
void CKnob::ReadParameters(CGauge *mg,SStream *str)
{	char	name[64] = {0};
	char	art[128];
  this->mg = mg;
  ReadString (name, 64, str);
  ReadShort(&x1, str);
  ReadShort(&y1, str);
	ReadShort(&x2, str);
	ReadShort(&y2, str);
	if (name[0] == 0)	return;
  int wd = (x2 - x1) + 1;
  int ht = (y2 - y1) + 1;
  if (wd & 1) wd += 1;
  if (ht & 1) ht += 1;
	_snprintf(art,63,"ART/%s", name);
	art[63] = 0;
	CBitmap *map = new CBitmap(art);
  bmap = map;
  nFrame  = map->NumBitmapFrames() - 1;
	SSurface *sf = CreateSurface (wd,ht);
	if (sf == NULL)		return;
	ksurf	= sf;
	sf->xScreen = x1 + mg->GetXPos();
	sf->yScreen = y1 + mg->GetYPos();
	EraseSurfaceRGBA(ksurf, 0);
  mg->BuildQuad(quad,ksurf->xScreen,ksurf->yScreen,wd,ht);
	return;
}
//---------------------------------------------------------------------
//	Share the mother gauge surface
//---------------------------------------------------------------------
void CKnob::Share(CGauge *gage)
{ mg = gage;
  gage->GetSize(&x1,&y1,&x2,&y2);
  x1  = 0;
  y1  = 0;
  short px,py;
  gage->GetCenterOffset(&px,&py);
  cx += px;
  cy += py;
  ksurf = gage->GetSurface();
  ownS  = 0;
  return;
}
//---------------------------------------------------------------------
//	Assign a bitmap
//---------------------------------------------------------------------
void CKnob::AssignNeedle(char *bmn)
{ char art[128];
  _snprintf(art,63,"ART/%s",bmn);
	art[63] = 0;
  bmap = new CBitmap(art);
  nFrame = bmap->NumBitmapFrames() - 1;
  //---Compute needle position ---------
  int bw = 0;
  int bh = 0;
  bmap->GetBitmapSize(&bw,&bh);
  cx = (x2 - bw + 1) >> 1 ;         // Surface center
  cy = (y2 - bh + 1) >> 1 ;
  return;
}
//-----------------------------------------------------------------------
//  Arm floating value rotation
//-----------------------------------------------------------------------
void CKnob::ArmRotation(float adj,int button)
{	CState	= 1;								  // Active state
	maxTM	  = 0.5f;								// Slow rate
	Timer	  = maxTM;							// Fisrt tick
	Incr	  = 0;
  if (button & MOUSE_BUTTON_LEFT )	{Incr	= -adj; aFrame = -1;}
  if (button & MOUSE_BUTTON_RIGHT)	{Incr	= +adj; aFrame = +1;}
  Change  = 0;
	return;
}

//-----------------------------------------------------------------------
//  Check for a change
//-----------------------------------------------------------------------
bool CKnob::HasChanged()
{ if (CState == 2) {CState = 0; return true; }    // First time
  if (CState == 0)				      return false;		  // No change
  float dT	= globals->dST;
	Timer	+= dT;
	if (Timer < maxTM)				    return false;			// No change yet
  Timer	 = 0;
	if (maxTM > 0.05)	maxTM -= 0.05f;						    // Accelerate
  Change  = Incr;
  cFrame += aFrame;
  if (cFrame <       0) cFrame = nFrame;
  if (cFrame >  nFrame) cFrame = 0;
  return true;
}
//-----------------------------------------------------------------------
// Redraw rotating Knob at center
//-----------------------------------------------------------------------
int CKnob::DrawAsNeedle()
{	HasChanged();       // Update position
  if (ksurf == NULL)   return 0;
	if (ownS) EraseSurfaceRGBA(ksurf,0);
	bmap->DrawBitmap (ksurf,cx,cy, cFrame);
	return cFrame;
}
//-----------------------------------------------------------------------
// Redraw rotating Knob at current frame
//-----------------------------------------------------------------------
int CKnob::Redraw()
{	if (ksurf == NULL)   return 0;
	if (ownS) EraseSurfaceRGBA(ksurf,0);
	bmap->DrawBitmap (ksurf, 0, 0, cFrame);
	return cFrame;
}
//-----------------------------------------------------------------------
//	Redraw rotating Knob at given frame
//-----------------------------------------------------------------------
int CKnob::ReDraw(int frame)
{	cFrame  = frame;
  if (ksurf == NULL)   return 0;
	if (ownS) EraseSurfaceRGBA(ksurf,0);
	bmap->DrawBitmap (ksurf, 0, 0, cFrame);
	return cFrame;
}
//--------------------------------------------------------------
//  Check if the mouse is in the area
//--------------------------------------------------------------
ECursorResult CKnob::MouseMoved (int x, int y)
{ // Check that the position is within click area bounds
  if (IsHit (x, y)) return  globals->cum->SetCursor (cursTag);
  return CURSOR_NOT_CHANGED;
}
//==========================================================================
//  Animated bitmap
//==========================================================================
CAniMAP::CAniMAP()
{	x	= 0;
	y	= 0;
	NbFrame	= 1;
  hiFrame = 1;
  bmap		= AssignNullBitmap();
}
//----------------------------------------------------------------------
//  Free the bitmap
//----------------------------------------------------------------------
CAniMAP::~CAniMAP()
{ if (bmap) delete bmap;
  bmap  = 0;
}
//-------------------------------------------------------------
//	Decode the name
//-------------------------------------------------------------
int CAniMAP::DecodeBitmap(char *str,char *name)
{ int  n = 1;
  int nf = sscanf(str,"%d , %s", &n, str);
  if (2==nf) {strncpy(name,str,64); return n;}
  strncpy(name,str,64);
  return 1;
}
//----------------------------------------------------------------------
//  Read bitmap parameters
//----------------------------------------------------------------------
void  CAniMAP::Read(SStream *str, int nbp)
{	char	name[64] = {0};
	char	snm[128];
  char  art[128];
  ReadString (name, 64, str);
  if (nbp > 0) ReadShort (&x, str);
  if (nbp > 1) ReadShort (&y, str);
	if (name[0] == 0)	return;
	int nf = DecodeBitmap(name,snm);
  sprintf(art,"ART/%s",snm);
  CBitmap *map = new CBitmap(art);
  if (1 != nf)   map->SetFrameNb(nf);
  bmap = map;
  NbFrame = (U_SHORT)map->NumBitmapFrames();
  hiFrame = (U_SHORT)NbFrame - 1;
  return;
}
//----------------------------------------------------------------------
//  Init bitmap
//----------------------------------------------------------------------
void CAniMAP::Fill(char *name)
{ char	art[128];
  _snprintf(art,63,"ART/%s", name);
	art[63] = 0;
  CBitmap *map = new CBitmap(art);
  bmap = map;
  NbFrame = (U_SHORT)map->NumBitmapFrames();
  hiFrame = (U_SHORT)NbFrame - 1;
  return;
}
//----------------------------------------------------------------------
//  Clamp frame to value
//----------------------------------------------------------------------
int CAniMAP::ClampFrame(int fn)
{ if (fn < 0)          return 0;
  if (fn >= NbFrame)   return hiFrame;
  return fn;  }
//==========================================================================
// CGauge
//==========================================================================
CGauge::CGauge (void)
{ vOfs	= 0;
	Init();
}
//--------------------------------------------------------------------
//	JSDEV* Prepare the gauge message
//-------------------------------------------------------------------
void CGauge::Init()
{ type	= GAUGE_BASE;
  cursTag   = 0;
  unId			= 0;
	value			= 0;
	//--------------------------------------------
	w	 = h  = 0;
  cx = cy = 0;
	//---Default Hit detector -------------------
	vHit = &CGauge::RectHit;
  //---Gauge parameters -----------------------
  gmin = 0.0f;
  gmax = 100000.0f;
  gmap = 0;
  plit = 0;
  mask = 0;
  strcpy (help, "");
  strcpy (dfmt, "");
  sync = 0;
	subS = 0; 
  surf = 0;
  //--- Sound parameters -----------------------
  sbuf[0] = 0;
  sbuf[1] = 0;
  //--- Help buffer -----------------------------
  hbuf    = globals->fui->HelpBuffer();
}

//--------------------------------------------------------------------
//	JSDEV* Prepare the gauge message
//-------------------------------------------------------------------
CGauge::CGauge(CPanel *mp)
{	vOfs  = 0;
	panel	= mp;
  if (mp) vOfs	= mp->FixRoom(4);
	Init();
}
//--------------------------------------------------------------------
//	JSDEV* Prepare the gauge message
//-------------------------------------------------------------------
void CGauge::PrepareMsg(CVehicleObject *veh)
{	//---- For debbug, uncomment the next lines ----
	//if (unId == 'lful')
	//	int a = 0;											// Put break point here
	//---- Find subsystem --------------------------
  veh->FindReceiver(&mesg);
	subS	= (CSubsystem*)mesg.receiver;
	return;
}
//-------------------------------------------------------------------
//  Destroy gauge
//  Free all resources
//-------------------------------------------------------------------
CGauge::~CGauge (void)
{ if (mask) delete mask;
  if (surf) surf = FreeSurface (surf);
  if (gmap) gmap->DecUse();
  mask  = 0;
  surf  = 0;
  gmap  = 0;
  if (sbuf[0]) sbuf[0]->Release();
  if (sbuf[1]) sbuf[1]->Release();
}
//-----------------------------------------------------------
//  Set Quad coordinates from NW corner
//-----------------------------------------------------------
void CGauge::BuildQuad(TC_VTAB *qd,float px,float py,float wd,float ht)
{  //----Init base lines------------------------------------------------
  float ph = panel->GetHeight();    // Top OpenGL line
  float ls = ph - py - ht;          // South line
  float ln = ph - py;								// North line			
	//--- Compute SW corner ------------------------
	qd[Q_SW].VT_S = 0;
	qd[Q_SW].VT_T = 0;
	qd[Q_SW].VT_X = px;
	qd[Q_SW].VT_Y = ls;
	qd[Q_SW].VT_Z = 0;
	//--- Compute SE corner ------------------------
	qd[Q_SE].VT_S = 1;
	qd[Q_SE].VT_T = 0;
	qd[Q_SE].VT_X = px + wd;
	qd[Q_SE].VT_Y = ls;
	qd[Q_SE].VT_Z = 0;
	//--- Compute NE corner ------------------------
	qd[Q_NE].VT_S = 1;
	qd[Q_NE].VT_T = 1;
	qd[Q_NE].VT_X = px + wd;
	qd[Q_NE].VT_Y = ln;
	qd[Q_NE].VT_Z = 0;
	//--- Compute NW corner ------------------------
	qd[Q_NW].VT_S = 0;
	qd[Q_NW].VT_T = 1;
	qd[Q_NW].VT_X = px;
	qd[Q_NW].VT_Y = ln;
	qd[Q_NW].VT_Z = 0;
	return;
}

//---------------------------------------------------------------------
//  Prepare the requested quad
//---------------------------------------------------------------------
void CGauge::PrepareQuad(TC_VTAB *pan)
{ for (int k=0; k<4; k++)	pan[k] = StdQUAD[k];
	return;
}
//---------------------------------------------------------------------
//  Init a QUAD polygon from dimension
//---------------------------------------------------------------------
void CGauge::BuildQuad(TC_VTAB *pan, S_PIXEL *sp)
{ //----Init base lines------------------------------------------------
  int ph = panel->GetHeight();                    // Top OpenGL line
	//-----SW corner -----------------------------------------------------
  pan[Q_SW].VT_S   = 0;
  pan[Q_SW].VT_T   = 0;
  pan[Q_SW].VT_X   = sp[SW_PIX].x;
  pan[Q_SW].VT_Y   = ph - sp[SW_PIX].y;
  pan[Q_SW].VT_Z   = 0;
  //-----SE corner ----------------------------------------------------
  pan[Q_SE].VT_S   = 1;
  pan[Q_SE].VT_T   = 0;
  pan[Q_SE].VT_X   = sp[SE_PIX].x;
  pan[Q_SE].VT_Y   = ph - sp[SE_PIX].y;
  pan[Q_SE].VT_Z   = 0;
  //-----NE corner ----------------------------------------------------
  pan[Q_NE].VT_S   = 1;
  pan[Q_NE].VT_T   = 1;
  pan[Q_NE].VT_X   = sp[NE_PIX].x;
  pan[Q_NE].VT_Y   = ph - sp[NE_PIX].y;
  pan[Q_NE].VT_Z   = 0;
  //-----NW corner ----------------------------------------------------
  pan[Q_NW].VT_S   = 0;
  pan[Q_NW].VT_T   = 1;
  pan[Q_NW].VT_X   = sp[NW_PIX].x;
  pan[Q_NW].VT_Y   = ph - sp[NW_PIX].y;
  pan[Q_NW].VT_Z   = 0;
  return;
}
//---------------------------------------------------------------------
//  Copy parameters from the gauge
//---------------------------------------------------------------------
void  CGauge::CopyFrom(CGauge &g)
{ dsw     = g.dsw;
  dsh     = g.dsh;
  xOrg    = g.xOrg;
  yOrg    = g.yOrg;
  yOfs    = g.yOfs;
  //--- Copy generic parameters ------------
	plit		= g.plit;
  mesg    = g.mesg;
  gmin    = g.gmin;
  gmax    = g.gmax;
  gmap    = g.gmap;
  if (gmap) gmap->IncUse();
  cursTag = g.cursTag;
  strncpy(help,g.help,64);
  strncpy(dfmt,g.dfmt,64);
  sync    = g.sync;
	subS		= g.subS;
	return;
}
//---------------------------------------------------------------------
//  Collect VBO data
//---------------------------------------------------------------------
void	CGauge::CollectVBO(TC_VTAB *vtb)
{	if (0 == vOfs)	return;
	TC_VTAB *dst	= vtb + vOfs;
	for (int k=0; k<4; k++)	*dst++ = quad[k];
	return;
}
//--------------------------------------------------------------
//  Read light mask
//--------------------------------------------------------------
void CGauge::ReadLightMask(SStream *str)
{ char	name[64] = {0};
	char	art[128];
  ReadString (name, 64, str);
  if (name[0] == 0)	return;
	_snprintf(art,63,"ART/%s", name);
	art[63] = 0;
  mask = new CMaskImage(w,h,art);
  mask = mask->Validate();
  return;
}
//--------------------------------------------------------------
//  Create sound tag
//--------------------------------------------------------------
int CGauge::GetSoundTag(int p,SStream *str)
{ char  sfx[64] = "";
  ReadString(sfx,64,str);
  sbuf[p] = globals->snd->ReserveSoundBUF(sfx);
  return 0;
}
//---------------------------------------------------------------
//    Bind cursor
//---------------------------------------------------------------
Tag CGauge::BindCursor(SStream *str)
{ char curs[64] = "";
  ReadString(curs,64,str);
  if (curs[0] == 0)   return 0;
  return globals->cum->BindCursor (curs);
  }
//---------------------------------------------------------------
//  Decode Display
//  The display area create a surface and a Quad  
//  The <disp> coordinates must be gauge relative (from
//            upper left corner defined by <size>
//  The surface has the same dimension as the quad.
//---------------------------------------------------------------
void CGauge::DecodeDisplay(SStream *str)
{ char txt[128];
  int px,py;
  int wd,ht;
  ReadString(txt,128,str);
  int nf = sscanf(txt,"%d , %d , %d , %d ",&px, &py, &wd, &ht);
  if (4 != nf)  gtfo("<disp> invalid parameter");
  xOrg  = px;
  yOrg  = py;
  dsw   = wd;
  dsh   = ht;
  //---------------------------------------------
	int sx = xOrg + tour[NW_PIX].x;
	int sy = yOrg + tour[NW_PIX].y;
  //---Create the quad --------------------------
  BuildQuad(quad, sx, sy, dsw, dsh);
  return;
}
//--------------------------------------------------------------
//  Allocate a surface 
//--------------------------------------------------------------
void CGauge::SetSurface(int wd,int ht)
{ FreeSurface(surf);
  surf = CreateSurface (wd, ht);
  EraseSurfaceRGBA(surf,0);
  return;
}
//--------------------------------------------------------------
//  Decode position parameters from <posn> tag
//--------------------------------------------------------------
void CGauge::DecodePOSN(SStream *str,TC_VTAB *qd,char opt)
{ char txt[128];
  float px = 0;
  float py = 0;
  float wx = 0;
  float hy = 0;
  ReadString(txt,128,str);
  int nf = sscanf(txt,"%f , %f , %f , %f",&px,&py,&wx,&hy);
  if (4 != nf)    gtfo("Invalid tag <posn>");
  //--- Always relative to gauge (NW corner) -------------------
	px += tour[NW_PIX].x;						// Add x origin
	py += tour[NW_PIX].y;						// Add y origin
  BuildQuad(qd,px,py,wx,hy);
  return;
}
//--------------------------------------------------------------
//  Decode quadrant parameters from <cadr> tag
//--------------------------------------------------------------
void CGauge::DecodeCADR(SStream *str,TC_VTAB *qd,char opt)
{ char txt[128];
  float px = 0;
  float py = 0;
  float wx = 0;
  float hy = 0;
  ReadString(txt,128,str);
  int nf = sscanf(txt,"%f , %f , %f , %f",&px,&py,&wx,&hy);
  if (4 != nf)    gtfo("Invalid tag <cadr>");
  //--- Check if relative to gauge ------------------------
	if (1 == opt) {px += tour[NW_PIX].x; py += tour[NW_PIX].y;}
  BuildQuad(qd,px,py,wx,hy);
  return;
}
//--------------------------------------------------------------
//  Decode needle center parameters from <nctr> tag
//  x,y = center position inside gauge
//  r   = radius
//--------------------------------------------------------------
void CGauge::DecodeNCTR(SStream *str,TC_VTAB *qd,char opt)
{ char txt[128];
  float x,y;
  float r;
  ReadString(txt,128,str);
  int nf = sscanf(txt,"%f , %f , R=%f",&x,&y,&r);
  if (3 != nf)    gtfo("Invalid tag <cntr>");

  //---- compute upper left corner inside gauge -------- 
  float px = x - r;
  float py = y - r;
  //---- compute absolute coordinates if requested -----
	if (1 == opt) {px += tour[NW_PIX].x; py += tour[NW_PIX].y;}
  float dim = 2*r;
  BuildQuad(qd,px,py,dim,dim);
  return;
}
//--------------------------------------------------------------
//  Decode the projector tag
//--------------------------------------------------------------
void CGauge::DecodePROJ(SStream *str, TC_VTAB *proj, char opt)
{ char txt[128];
  float px, py;
  int ns  = 0;
  float ph  = panel->GetHeight();
  PrepareQuad(proj);
  while (ns != 4)
  { ReadString(txt,128,str);
    //--- North East corner ---------------------------
    int n0 = sscanf(txt,"NE( %f , %f )",&px,&py);
		if (1 == opt) {px += tour[NW_PIX].x; py += tour[NW_PIX].y;}
    if (2 == n0)
    { proj[Q_NE].VT_X = (px); 
      proj[Q_NE].VT_Y = (ph - py - 1);
      ns++;
      continue; }
    //--- North West corner ---------------------------
    int n1 = sscanf(txt,"NW( %f , %f )",&px,&py);
		if (1 == opt) {px += tour[NW_PIX].x; py += tour[NW_PIX].y;}
    if (2 == n1)
    { proj[Q_NW].VT_X = (px); 
      proj[Q_NW].VT_Y = (ph - py - 1);
      ns++;
      continue; }
    //--- South West corner ---------------------------
    int n2 = sscanf(txt,"SW( %f , %f )",&px,&py);
		if (1 == opt) {px += tour[NW_PIX].x; py += tour[NW_PIX].y;}
    if (2 == n2)
    { proj[Q_SW].VT_X = (px); 
      proj[Q_SW].VT_Y = (ph - py - 1);
      ns++;
      continue; }
    //--- South East corner ---------------------------
    int n3 = sscanf(txt,"SE( %f , %f )",&px,&py);
		if (1 == opt) {px += tour[NW_PIX].x; py += tour[NW_PIX].y;}
    if (2 == n3)
    { proj[Q_SE].VT_X = (px); 
      proj[Q_SE].VT_Y = (ph - py - 1);
      ns++;
      continue; }
      //--- Errror --------------------------------------
    gtfo("Invalid tag <proj>");
  }
  return;
}
//--------------------------------------------------------------
//  Decode the shape tag
//	<shap> defines a quadrilater enclosing the gauge when
//	the <size> cannot approach the rectangular shape
//--------------------------------------------------------------
void CGauge::DecodeSHAP(SStream *str, S_PIXEL *sp, char opt)
{	char txt[128];
  int px, py;
  int ns  = 0;
  while (ns != 4)
  { ReadString(txt,128,str);
    //--- North East corner ---------------------------
    int n0 = sscanf(txt,"NE( %d , %d )",&px,&py);
    if (2 == n0)
    { sp[NE_PIX].x = short(px); 
      sp[NE_PIX].y = short(py);
      ns++;
      continue; }
    //--- North West corner ---------------------------
    int n1 = sscanf(txt,"NW( %d , %d )",&px,&py);
    if (2 == n1)
    { sp[NW_PIX].x = short(px); 
      sp[NW_PIX].y = short(py);
      ns++;
      continue; }
    //--- South West corner ---------------------------
    int n2 = sscanf(txt,"SW( %d , %d )",&px,&py);
    if (2 == n2)
    { sp[SW_PIX].x = short(px); 
      sp[SW_PIX].y = short(py);
      ns++;
      continue; }
    //--- South East corner ---------------------------
    int n3 = sscanf(txt,"SE( %d , %d )",&px,&py);
    if (2 == n3)
    { sp[SE_PIX].x = short(px); 
      sp[SE_PIX].y = short(py);
      ns++;
      continue; }
      //--- Errror --------------------------------------
    gtfo("Invalid tag <proj>");
  }
	vHit	= &CGauge::TourHit;
  return;
}
//-----------------------------------------------------------------------
//	Read gauge Tags
//-----------------------------------------------------------------------
int CGauge::Read (SStream *stream, Tag tag)
{   //  Uncomment for debugging 
    // if (unId == 'avi2')       // Change id
    //  int a  = 0;             // Set break point here
  Tag pm;
  switch (tag) {
  //--- Unique identity  ----------------
  case 'unid':
    ReadTag (&unId, stream);
    TagToString (unid_s, unId);
    return TAG_READ;
  //--- Overall size (include knob, etc) ---
  case 'size':
		ReadSIZE(stream,tour,&w,&h);
    BuildQuad(quad,tour[NW_PIX].x,tour[NW_PIX].y,w,h);
    return TAG_READ;
	//--- Shape ---------------------
	case 'shap':
		DecodeSHAP(stream,tour,0);
		BuildQuad(quad,tour);
		return TAG_READ;
  //--- Absolute projector --------
  case 'proj':
    DecodePROJ(stream,quad,0);
    return TAG_READ;
  //--- Gauge center --------------
  case 'cntr':
    ReadShort (&cx, stream);
    ReadShort (&cy, stream);
    return TAG_READ;
  //--- Message to subsystem ------
  case 'mesg':
    ReadMessage (&mesg, stream);
    return TAG_READ;
  //--- Clamping values ---------
  case 'clmp':
  case 'rang':
    ReadFloat (&gmin, stream);
    ReadFloat (&gmax, stream);
    return TAG_READ;
  //--- Minimum value -----------
  case 'gmin':
    ReadFloat (&gmin, stream);
    return TAG_READ;
  //--- Maximum value -----------
  case 'gmax':
    ReadFloat (&gmax, stream);
    return TAG_READ;
  //--- Table lookup ------------
  case 'gtbl':
    { CDataSearch map(stream);
      gmap = map.GetTable();
      gmap->IncUse();
      return TAG_READ;
    }
  case 'lite':
    ReadTag (&pm, stream);
    plit  = panel->GetLight(pm);
    return TAG_READ;

  case 'mask':
    ReadLightMask(stream);
    return TAG_READ;

  case 'fore':
    return TAG_READ;

  case 'curs':
  case 'crsr':
    cursTag = BindCursor(stream);
    return TAG_READ;

  case 'help':
    ReadString (help, 64, stream);
    return TAG_READ;

  case 'dtyp':
    return TAG_READ;

  case 'dfmt':
    ReadString (dfmt, 64, stream);
    return TAG_READ;

  case 'nomc':
    return TAG_READ;

  case 'igno':
    return TAG_READ;
  case 'sync':
    sync  = 1;
    return TAG_READ;
  case 'conn':
    // DEPRECATED -- ignore at this level ------------
    {
      Tag tag;
      ReadTag (&tag, stream);
    }
    return TAG_READ;
  case 'onfx':
  case 'onsf': // Might also be onfx
  case 'sfxu':
  case 'sfxo':
  case 'sfxr':
  case 'sfx_':
  case 'sfx1':
    GetSoundTag(GAUGE_ON__POS,stream);
    return TAG_READ;

  case 'ofsf':  // Might also be offx
  case 'offx':
  case 'sfxd':
  case 'sfxl':
  case 'sfxi':
  case 'sfx2':
    GetSoundTag(GAUGE_OFF_POS,stream);
    return TAG_READ;

  case 'user':
    // DEPRECATED // ex: oilP gauge in main flyhawk PNL
    {
      char user [64] = {0};
      char tmp_2[32] = {0};
      int  unit = 1;  
      ReadString (user, 64, stream);
      /// \todo  Parse user string into SMessage field update
      if (user[0] == 0) return TAG_READ;
      if (sscanf        (user, "HARDWARE,%s", tmp_2) == 1) {
            if (!strcmp (tmp_2, "GAUGE"))
            { mesg.user.u.hw = HW_GAUGE;
              return TAG_READ;
            }
            if (!strcmp (tmp_2, "SWITCH"))
            { mesg.user.u.hw = HW_SWITCH;
              return TAG_READ;
            }
          }
      if (sscanf (user, "ENGINE,%d", &unit) == 1)
          { mesg.user.u.engine = unit;     //user_engine; // should be tmp_val
            return TAG_READ;
          }
      if (sscanf (user, "UNIT,%d", &unit) == 1)
          { mesg.user.u.unit = unit;
            return TAG_READ;
          }
            ; // ...to be continued
 
      return TAG_READ;
    }

  case 'usr1':
    {
      // DEPRECATED used in FLYHWK01.PNL 'shet' Elevator Trim wheel but in <mesg>
      ReadUInt (&mesg.user.u.datatag, stream);
      return TAG_READ;
    }

  case 'unit':
    // DEPRECATED
    ReadUInt (&mesg.user.u.unit, stream);
    return TAG_READ;

  case 'dtag':
    // DEPRECATED
    ReadUInt (&mesg.user.u.datatag, stream);
    return TAG_READ;
  }

  // If this code is reached, the tag was not processed
  WARNINGLOG("%s::Read : Unknown tag %s", GetClassName(), TagToString(tag));
  return false;
}

//-----------------------------------------------------------------------
//    Read finished
//------------------------------------------------------------------------
void CGauge::ReadFinished (void)
{ subS = panel->GetMVEH()->GetNullSubsystem();
	// Compute center ------------------------------- 
  if (0 == cx)  cx = (w >> 1); 
  if (0 == cy)  cy = (h >> 1); 
  // Message default value 
  mesg.dataType = TYPE_REAL;
  mesg.sender   = 'gaug';
	mesg.id       = MSG_GETDATA;
  //----Assign a default light ---------------------
	if (0 == plit)	plit = panel->GetLight();
  //-- Create drawing surface for older gauges ----
  if (Prop.Not(NO_SURFACE))
  { surf = CreateSurface (w, h);
    if (0 == surf)  return;
    surf->xScreen = tour[NW_PIX].x;
    surf->yScreen = tour[NW_PIX].y;
    EraseSurfaceRGBA(surf,0);
    return; }
  return;
}
//------------------------------------------------------------------
//  Locate the gauge
//------------------------------------------------------------------
CGauge *CGauge::LocateGauge(SStream *s)
{ Tag pan = 0;
  Tag gag = 0;
  ReadTag(&pan,s);
  ReadTag(&gag,s);
  //---Locate panel and gage --------------
  CPanel *panl = globals->pit->GetPanelByTag(pan);
  if (0 == panl)    gtfo("Cant locate panel to copy");
  CGauge *g    = panl->GetGauge(gag);
  if (0 == g)       
		gtfo("Cant locate gauge to copy");
  return g;
}
//------------------------------------------------------------------------
//	Reserve room in VBO buffer
//------------------------------------------------------------------------
U_SHORT CGauge::FixRoom(U_SHORT n)
{	return panel->FixRoom(n);	}
//------------------------------------------------------------------------
//  trying feature
//  Light the gauge according to mask
//  When mask is 0 and a non black pixel exists, set alpha to 
//  brightness
//  NOTE this is working only with intel CPU because
//       a word is inverted from memory
//-------------------------------------------------------------------------
bool CGauge::SetLight()
{ if (Prop.Has(NO_SURFACE)) return false;
  U_INT af1 = 0xFF000000;
  U_CHAR  *msk = mask->rawdata;
  U_INT   *dst = surf->drawBuffer;
  for   (int k = 0; k!= surf->nPixel; k++)
  { U_CHAR  m = *msk++;
    U_INT  pix  = *dst & 0x00FFFFFF;
    if (m && pix) pix |= (m << 24);
    *dst++ = pix;
  }
  return true;
}
//-------------------------------------------------------------------------
//  Alight gauge with its dedicated light
//-------------------------------------------------------------------------
void CGauge::SetBrightness(float amb)
{	amlit	= amb;													// Save ambiant light
	return plit->Alight();}								// Set gauge light
//-------------------------------------------------------------------------
//  return position and size
//-------------------------------------------------------------------------
void CGauge::GetSize (short* x, short* y, short *w, short* h)
{	*x = tour[NW_PIX].x;
	*y = tour[NW_PIX].y;
  *w = this->w;
  *h = this->h;
	return;
}
//-------------------------------------------------------------------------
//  return position and size
//-------------------------------------------------------------------------
void CGauge::GetTour(S_PIXEL *t, short *w, short *h)
{	for (int k=0; k<4; k++) t[k] = tour[k];
	*w = this->w;
	*h = this->h;
	return;
}
//-------------------------------------------------------------------------
//  Check for a hit
//-------------------------------------------------------------------------
bool CGauge::RectHit(short x,short y,HIT_GAUGE &hg)
{	bool hit = (tour[NW_PIX].x <  x)	
						&(tour[NE_PIX].x >= x)
						&(tour[NW_PIX].y <  y)
						&(tour[SW_PIX].y >= y);
	hg.ghit = 0;
	if (hit)
	{	hg.ghit = this;
		hg.gx = tour[NW_PIX].x;
		hg.gy = tour[NW_PIX].y;
		hg.rx = x - hg.gx;
		hg.ry = y - hg.gy;
	}
	return hit;
}
//-------------------------------------------------------------------------
//  Check for a hit
//	Check if point(x,y) is in the gauge tour
//	This algorythm is based on the crossing count algorythm
//-------------------------------------------------------------------------
bool CGauge::TourHit(short x,short y,HIT_GAUGE &hg)
{	S_PIXEL cp;								// Current  point
	S_PIXEL pp = tour[3];			// Previous point
	int x1,x2;
	int y1,y2;
	char in = 0;
	hg.ghit = 0;
	//--- Loop on all edges ------------------------
	for (int k=0; k<4; k++)
	{	cp = tour[k];
		if (cp.x > pp.x) 
		{	x1 = pp.x;
			x2 = cp.x;
			y1 = pp.y;
			y2 = cp.y;
		}
		else
		{	x1 = cp.x;
			x2 = pp.x;
			y1 = cp.y;
			y2 = pp.y;
		}
		//---Check for intersection with vertical line ----
		if ((cp.x < x) == (x <= pp.x))
		{	long dt1 = (y  - y1) * (x2 - x1);
			long dt2 = (y2 - y1) * (x  - x1);
			if (dt1 < dt2)	in =!in;
		}
		pp	= cp;
	}
	//--- If inside, fill parameters -------------------
	if (in)
		{	hg.ghit = this;
			hg.gx = tour[NW_PIX].x;
			hg.gy = tour[NW_PIX].y;
			hg.rx = x - hg.gx;
			hg.ry = y - hg.gy;
		}
	return (in != 0);
}
//-------------------------------------------------------------------------
//  return Center offset
//-------------------------------------------------------------------------
void CGauge::GetCenterOffset(short* _cx, short* _cy)
{ *_cx = this->cx;
  *_cy = this->cy;
  return;
}
//-------------------------------------------------------------------------
//  Translate the gauge value
//-------------------------------------------------------------------------
void CGauge::Translate(float v)
{	value	= v;
  vReal = value;
  // First Clamp to gauge minimum and maximum values
  if (value < gmin) value = gmin;
  if (value > gmax) value = gmax;
  // Tranform data if we are in a lookout tbl 
  if (gmap)  value = gmap->Lookup(value);
  if (*dfmt != '*') vReal = value;
}
//----------------------------------------------------------------------
// Update the gauge's indication value
//  NOTE: Value is often used to compute an angle on a gauge display and
//        is not the real value read from the subsystem (like fuel QTY)
//        Thus when the sign * is the first sign in the dfmt mask, the real
//        value (rVal) is used rather than the final value.
//-----------------------------------------------------------------------
void CGauge::Update (void)
{ // Send the 'mesg' message to the supplier subsystem
  Send_Message (&mesg); //

  // All gauge values resolve to floating point values
  value = 0;
  switch (mesg.dataType) {
  case TYPE_INT:
    value = float(mesg.intData);
    break;

  case TYPE_INTPTR:
    if (mesg.intPtrData) value = float(*(mesg.intPtrData));
    break;

  case TYPE_REAL:
    value = mesg.realData;
    break;

  case TYPE_REALPTR:
    if (mesg.realPtrData) value = float(*(mesg.realPtrData));
    break;

  }
  vReal = value;
  // First Clamp to gauge minimum and maximum values
  if (value < gmin) value = gmin;
  if (value > gmax) value = gmax;
  // Tranform data if we are in a lookout tbl 
  if (gmap)  value = gmap->Lookup(value);
  if (*dfmt != '*') vReal = value;
	return;
}

//-------------------------------------------------------------------
//  Display Help below the gauge
//-------------------------------------------------------------------
void CGauge::DisplayHelp(char *txt)
{ if (globals->pln->NotOPT(VEH_PN_HELP)) return;
  char edt[64];
	int xn = tour[NW_PIX].x;
	int yn = tour[NW_PIX].y + h;
  if (*dfmt)
  { edt[0]  = ' ';
    char *msk = dfmt;
    if (*dfmt == '*') msk++;
    sprintf_s(hbuf,64,msk,vReal);
  }
  else if (txt)	strncpy(hbuf,txt,HELP_SIZE);
	else	return;
  globals->fui->DrawHelp(1,xn,yn);
  return;
}
//-------------------------------------------------------------------
//  Call Fui
//-------------------------------------------------------------------
void CGauge::FuiHelp()
{ if (globals->pln->NotOPT(VEH_PN_HELP)) return;
	int	xn	= tour[NW_PIX].x;			// Screen X coordinate
	int	yn	= tour[NW_PIX].y + h;	// Screen Y coordinate
  globals->fui->DrawHelp(1,xn,yn); 
}
//-----------------------------------------------------------
//  Arm repeat timer
//-----------------------------------------------------------
EClickResult CGauge::ArmRepeat(float t,int x, int y, int bt)
{ iTime = t;
  rTime = t;
  arm   = 1;
  mx    = x;
  my    = y;
  mbt   = bt;
  return MOUSE_TRACKING_ON;
}
//-----------------------------------------------------------
//  Repeat Mouse click if armed and timer out
//-----------------------------------------------------------
void CGauge::RepeatClick()
{ if (0 == arm)   return;
  rTime -= globals->dST;
  if (rTime > 0)  return;
  MouseClick(mx,my,mbt);
  return;
}
//-----------------------------------------------------------------------
//	Process mouse click
//-----------------------------------------------------------------------
EClickResult CGauge::MouseClick (int x, int y, int buttons)
{  return MOUSE_TRACKING_OFF;
}
//------------------------------------------------------------------------
EClickResult CGauge::TrackClick (int x, int y, int buttons)
{ return MOUSE_TRACKING_ON;
}
//---------------------------------------------------------------------------
//  Stop click intercept
//---------------------------------------------------------------------------
EClickResult CGauge::StopClick()
{ return MOUSE_TRACKING_OFF;
}
//---------------------------------------------------------------------------
//  MouseMove intercept
//---------------------------------------------------------------------------
ECursorResult CGauge::MouseMoved (int x, int y)
{ DisplayHelp(); 
  // Change to this gauge's cursor
  return globals->cum->SetCursor (cursTag);
}
//-------------------------------------------------------------------
//  Draw the gauge on panel
//-------------------------------------------------------------------
void CGauge::RenderSurface(TC_VTAB *tab,SSurface *sf)
{ if (0 == sf)  return;
  U_CHAR *tex = (U_CHAR*)sf->drawBuffer;
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,sf->xSize,sf->ySize,0,GL_RGBA,GL_UNSIGNED_BYTE,tex);
//----Only for debugging ----------------------------------------------
//glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);       // Uncomment to draw
//glColor3f(1,1,1);                               // Gauge outline on
//glDisable(GL_TEXTURE_2D);                       // The panel
//----------------------------------------------------------------------
	glDrawArrays(GL_TRIANGLE_STRIP,vOfs,4);
//glPolygonMode(GL_FRONT,GL_FILL);                // Dito
// CHECK_OPENGL_ERROR;
}
//-------------------------------------------------------------------
//  Outline the gauge on panel
//-------------------------------------------------------------------
void CGauge::OutLine()
{ TC_VTAB tab[4];
  tab[0]	= quad[0];
	tab[1]	= quad[1];
	tab[2]	= quad[3];
	tab[3]	= quad[2];
  glPushAttrib (GL_ALL_ATTRIB_BITS);
	glVertexPointer  (3,GL_FLOAT,sizeof(TC_VTAB),&tab[0].VT_X);
  glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);       // Draw lines
  glColor3f(1,1,1);                               // Gauge outline on
	glDrawArrays(GL_LINE_LOOP,0,4);
  glPopAttrib();
  return;
}
//-----------------------------------------------------------------------------
// Helper function to search for click area datatags.  These are tags
//  of the format <ca??> where ?? is a 2-digit value, i.e <ca01>, <ca15> etc.
//  Display only areas are of the format <da??>
// @returns int between 1 and 99 corresponding to <ca??> value, zero otherwise
//  CAxx is a displayable click area
//  DAxx is a display only area
//  CKxx is a clickable only area
//  DGxx is a digit inside the display surface (display and click)
//------------------------------------------------------------------------------
int CGauge::ClickAreaFromTag (Tag tag,int *cnb)
{ //--- Check if upper bytes are 'CA' or 'ca'
  U_INT upper = (tag & 0xFFFFF0F0);
  if ((upper == 'CA00') || (upper == 'ca00')) {
    U_INT d1 = ((tag & 0x0000FF00) >> 8) - '0';
    U_INT d0 =  (tag & 0x000000FF) - '0';
    *cnb = (d1 * 10) + d0;
    return (AREA_SHOW + AREA_CLIK);
  }
  //--- Check if upper bytes are 'DA' or 'da'------
  if ((upper == 'DA00') || (upper == 'da00')) {
    U_INT d1 = ((tag & 0x0000FF00) >> 8) - '0';
    U_INT d0 =  (tag & 0x000000FF) - '0';
    *cnb = (d1 * 10) + d0;
    return AREA_SHOW;
  }
  //--- Check if upper bytes are 'DG' or 'dg' -----
  if ((upper == 'DG00') || (upper == 'dg00')) {
    U_INT d1 = ((tag & 0x0000FF00) >> 8) - '0';
    U_INT d0 =  (tag & 0x000000FF) - '0';
    *cnb = (d1 * 10) + d0;
    return (AREA_DIGI + AREA_CLIK + AREA_SHOW);
  }
    //--- Check if upper bytes are 'BT' or 'bt' -----
  if ((upper == 'BT00') || (upper == 'bt00')) {
    U_INT d1 = ((tag & 0x0000FF00) >> 8) - '0';
    U_INT d0 =  (tag & 0x000000FF) - '0';
    *cnb = (d1 * 10) + d0;
    return (AREA_BUTN + AREA_CLIK + AREA_SHOW + AREA_QUAD);
  }

  //--- Check if upper bytes are 'CK' or 'ck' -----
  if ((upper == 'CK00') || (upper == 'ck00')) {
    U_INT d1 = ((tag & 0x0000FF00) >> 8) - '0';
    U_INT d0 =  (tag & 0x000000FF) - '0';
    *cnb = (d1 * 10) + d0;
    return AREA_CLIK;
  }
    //--- Check if upper bytes are 'BM' or 'bm' -----
  if ((upper == 'BM00') || (upper == 'bm00')) {
    U_INT d1 = ((tag & 0x0000FF00) >> 8) - '0';
    U_INT d0 =  (tag & 0x000000FF) - '0';
    *cnb = (d1 * 10) + d0;
    return (AREA_BMAP + AREA_SHOW + AREA_QUAD);
  }

  return 0;
}
//-----------------------------------------------------------------------------
// Helper function to search for message datatags.  These are tags
//  of the format <ms??> where ?? is a 2-digit value, i.e <MS01>, <ca15> etc.
//-----------------------------------------------------------------------------
int CGauge::MessageFromTag(Tag tag)
{ //--- Check if upper bytes are 'MS' or 'ms'
  U_INT upper = (tag & 0xFFFF0000) | 0x00003030;
  if ((upper == 'MS00') || (upper == 'ms00')) {
    U_INT d1 = ((tag & 0x0000FF00) >> 8) - '0';
    U_INT d0 =  (tag & 0x000000FF) - '0';
    return (d1 * 10) + d0; 
  }
  return 0;  }
//-----------------------------------------------------------------------------
// Helper function to search for On Action datatags.  These are tags
//  of the format <ON??> or <on??> where ?? is a 2-digit value, i.e <ON01>, <on15> etc.
//-----------------------------------------------------------------------------
int CGauge::ActionFromTag(Tag tag)
{ //--- Check if upper bytes are 'ON' or 'on'
  U_INT upper = (tag & 0xFFFF0000) | 0x00003030;
  if ((upper == 'ON00') || (upper == 'on00')) {
    U_INT d1 = ((tag & 0x0000FF00) >> 8) - '0';
    U_INT d0 =  (tag & 0x000000FF) - '0';
    return (d1 * 10) + d0; 
  }
  return 0;  }
//-----------------------------------------------------------------------------
//  Clear the gauge window
//-----------------------------------------------------------------------------
void CGauge::ClearDisplay()
{ if (0 == dsh) return;
  U_SHORT nl = dsh;
  U_INT *des = surf->drawBuffer + yOfs + xOrg;
  U_SHORT cl = 0;
  while (nl--)
  { while (cl++ != dsw) *des++ = 0;
    des += (surf->xSize - dsw);
    cl  = 0;
  }
  return;
}

//===============================================================
// CDLLGauge
//===============================================================
CDLLGauge::CDLLGauge (void)
{  
  #ifdef _DEBUG
    //TRACE ("--------------------------------------------------------");
    //TRACE ("DLL GAUGE ...");
  #endif
  obj = NULL;
  dll = NULL;
  dllgsurf = NULL;
  enabled = false;
  signature = 0;

}
//----------------------------------------------------------

CDLLGauge::~CDLLGauge()
{
  if (dllgsurf) dllgsurf = FreeSurface (dllgsurf);

#ifdef _DEBUG
  //TRACE ("DELETE DLL GAUGE %p %p", obj, dll);
  //TRACE ("--------------------------------------------------------");
#endif
  // sdk: cleanup objects = DLLDestroyObject // 
  globals->plugins.On_DestroyObject (obj, dll);
}

void CDLLGauge::Prepare (void)
{
  // sdk : test whether a dll is present
  if (globals->plugins_num) {
    void *test = globals->plugins.IsDLL (signature);
    if (NULL == test) {
      globals->plugins.On_DeleteAllObjects ();
      globals->plugins.On_KillPlugins ();
      char buf1 [8] = {0};
      TagToString (buf1, signature); 
      TRACE ("test dll '%s' = %d", buf1, test);
      gtfo ("failed to find a DLL for '%s'", buf1);
    }
    else {
      dll = test;
      globals->plugins.On_Instantiate_DLLGauges (signature,0,NULL);
      obj = globals->plugins.GetDLLObject (signature);
    }
  }
}

int CDLLGauge::Read (SStream *stream, Tag tag)
{
#ifdef _DEBUG
  TRACE ("READ DLL GAUGE %p %p %p", dll, obj, stream);
#endif

  switch (tag) {
  case 'unid':
    ReadTag (&unId, stream);
    TagToString (unid_s, unId);
    signature = unId;
    Prepare (); //
    return TAG_READ;
  }

  int ret = TAG_IGNORED;
  if (globals->plugins_num)
    ret = globals->plugins.On_Read (obj, stream, tag, dll);
  TRACE ("ret = %d", ret);
  if (ret == TAG_IGNORED) ret = CGauge::Read (stream, tag);
  return ret;
}

void CDLLGauge::ReadFinished (void)
{
  //Prepare ();
  //// sdk : test whether a dll is present
  //if (globals->plugins_num) {
  //  bool test = globals->plugins.IsDLL (signature);
  //  if (!test) {
  //    globals->plugins.On_DeleteAllObjects ();
  //    globals->plugins.On_KillPlugins ();
  //    char buf1 [8] = {0};
  //    TagToString (buf1, signature); 
  //    TRACE ("test dll '%s' = %d", buf1, test);
  //    gtfo ("failed to find a DLL for '%s'", buf1);
  //  }
  //}
  // x2  = x1 + w;
  // y2  = y1 - h;
  short x1 = 0,
      y1 = 0;
  short x2 = 0,
      y2 = 0; 
  GetSize (&x1, &y1, &x2, &y2);
#ifdef _DEBUG
  //TRACE ("CREATE DLL GAUGE SURF %d %d %d %d", x1, y1, x2, y2);
#endif  
  //CGauge::ReadFinished (); return;
  //
  if (dllgsurf) dllgsurf = FreeSurface (dllgsurf);
  dllgsurf = CreateSurface (x2, y2);
  if (0 == dllgsurf) GTFO ("CDLLGauge::ReadFinished surf pb");
  dllgsurf->xScreen = x1;
  dllgsurf->yScreen = y1;
}

void CDLLGauge::SetObject (SDLLObject *object)
{
  obj = object;
}

void CDLLGauge::SetSignature (const long &sign)
{
//#ifdef _DEBUG
//  TRACE ("SIGNATURE DLL GAUGE");
//#endif
  signature = sign;
}

void  CDLLGauge::Draw (void)
{ // Set projection matrix to 2D screen size
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D (0, globals->cScreen->Width, globals->cScreen->Height, 0);

  // Clear modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // Set up OpenGL drawing state
  glPushAttrib (GL_ALL_ATTRIB_BITS);
  //glDepthMask(GL_FALSE);                // Do not write in depth buffer
  glDisable (GL_DEPTH_TEST);
  glDisable (GL_COLOR_MATERIAL);
  glDisable(GL_TEXTURE_2D);
//*************************************************************************
  int y_diff = globals->cScreen->Height - panel->GetHeight ();
  glRasterPos2i (dllgsurf->xScreen - panel->GetXOffset (),
    dllgsurf->yScreen + dllgsurf->ySize - panel->GetYOffset () + y_diff);
  globals->plugins.On_Draw (obj, dllgsurf, dll); //  
//*************************************************************************
  // Restore original OpenGL state
  glPopAttrib();

  // Restore original modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  // Restore original projection matrix
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
}

//===============================================================
// CBitmapGauge
//===============================================================
CBitmapGauge::CBitmapGauge (CPanel *mp)
{ panel = mp;
	dsh   = 0; 
  return;
}
//----------------------------------------------------------

CBitmapGauge::~CBitmapGauge()
{
}

//-----------------------------------------------------------
//  Read gauge tags
//-----------------------------------------------------------
int CBitmapGauge::Read (SStream *stream, Tag tag)
{
  switch (tag) {
  case 'bmap':
    under.Read(stream,2);
    return TAG_READ;

  case 'over':
    overl.Read(stream,2);
    return TAG_READ;
  }
  return CGauge::Read (stream, tag);
}
//------------------------------------------------------------
//    Gauge tag are read
//-------------------------------------------------------------
void CBitmapGauge::ReadFinished (void)
{ CGauge::ReadFinished ();
  return;
}
//-------------------------------------------------------------
//  Draw underlay
//-------------------------------------------------------------
void CBitmapGauge::DrawUnderlay (void)
{ EraseSurfaceRGBA(surf,0);
  under.Draw(surf);
}
//-------------------------------------------------------------
//  Draw overlay
//-------------------------------------------------------------
void CBitmapGauge::DrawOverlay (void)
{ overl.Draw(surf);
}
//-------------------------------------------------------------
//    Draw both layers
//-------------------------------------------------------------
void CBitmapGauge::Draw (void)
{ EraseSurfaceRGBA(surf,0);
  under.Draw(surf);
  overl.Draw(surf);
}
//===========================================================================
// CNeedleGauge
//===========================================================================
CNeedleGauge::CNeedleGauge (CPanel *mp)
: CBitmapGauge(mp)
{ def  = 0;
  radi = 0;
  sang = 0.0f;
  dunt = 1.0f;
  rang_min = 0.0f;
  rang_max = 0.0f;
  noff_x = noff_y = 0;
}

//--------------------------------------------------------------------------
//  read parameters
//--------------------------------------------------------------------------
int CNeedleGauge::Read (SStream *stream, Tag tag)
{
  switch (tag) {
  case 'radi':
    ReadInt (&radi, stream);
    return TAG_READ;

  case 'sang':
    ReadFloat (&sang, stream);
    return TAG_READ;

  case 'dunt':
    ReadFloat (&dunt, stream);
    return TAG_READ;

  case 'rang':
    ReadFloat (&rang_min, stream);
    ReadFloat (&rang_max, stream);
    return TAG_READ;

  case 'noff':
    ReadInt (&noff_x, stream);
    ReadInt (&noff_y, stream);
    return TAG_READ;

  case 'noac':
    return TAG_READ;

  case 'nedl':
    ReadTag  (&nedl.tag, stream);
    ReadFrom (&nedl, stream);
    return TAG_READ;
  }

  return CBitmapGauge::Read (stream, tag);
}
//------------------------------------------------------------------
//  Draw the gauge
//------------------------------------------------------------------
void CNeedleGauge::Draw (void)
{ //  uncomment to test
  //    if (unId == 'fqtl')       // Change id
  //    int a  = 0;             // Set break point here
  // Get indication value, this updates the 'value' field
  CGauge::Update ();

  // Determine animation frame corresponding to gauge angle
  float degrees = value + sang;
  //---- Draw underlay ------------
  EraseSurfaceRGBA(surf,0);
  under.Draw(surf);
  // Draw needle into gauge surface
  nedl.Draw (surf, degrees, cx, cy);
  //---Draw overlay --------------
  overl.Draw(surf);  
  return;
}
//-----------------------------------------------------------------
//  Draw on a shared surface
//-----------------------------------------------------------------
void CNeedleGauge::DrawOnThisSurface(SSurface *sf)
{ //-----Update the value  --------------------------
  mesg.id = MSG_GETDATA;
  CGauge::Update ();
  // Determine animation frame corresponding to gauge angle
  float degrees = value + sang;
  // draw only if required
  nedl.Draw (sf, degrees, cx, cy);
  return;
}


//=========================================================================
// CTwoNeedleGauge
//=========================================================================
CTwoNeedleGauge::CTwoNeedleGauge (CPanel *mp)
: CBitmapGauge(mp)
{	ndl1.SetPanel(mp);
	ndl2.SetPanel(mp);
}
//--------------------------------------------------------------
//	JSDEV* Prepare message
//--------------------------------------------------------------
void CTwoNeedleGauge::PrepareMsg(CVehicleObject *veh)
{	ndl1.PrepareMsg(veh);
	ndl2.PrepareMsg(veh);
	CGauge::PrepareMsg(veh);
	return;
}
//--------------------------------------------------------------
//  Read all parameters
//--------------------------------------------------------------
int CTwoNeedleGauge::Read (SStream *stream, Tag tag)
{ switch (tag) {
  case 'ndl1':
    ReadFrom (&ndl1, stream);
    ndl1.Define();
    return TAG_READ;

  case 'ndl2':
    ReadFrom (&ndl2, stream);
    ndl2.Define();
    return TAG_READ;
  }

  return CBitmapGauge::Read (stream, tag);
}
//------------------------------------------------------------------
//  Draw the gauge
//------------------------------------------------------------------
void CTwoNeedleGauge::Draw()
{ EraseSurfaceRGBA(surf,0);
  under.Draw(surf);
  if (ndl1.Defined()) ndl1.DrawOnThisSurface(surf);
  if (ndl2.Defined()) ndl2.DrawOnThisSurface(surf);
  overl.Draw(surf);
  return;
}
//==================================================================
// CThreeNeedleGauge
//==================================================================
CThreeNeedleGauge::CThreeNeedleGauge (CPanel *mp)
: CBitmapGauge(mp)
{	ndl1.SetPanel(mp);
	ndl2.SetPanel(mp);
	ndl3.SetPanel(mp);
}

//--------------------------------------------------------
//	Prepare Message
//----------------------------------------------------------
void CThreeNeedleGauge::PrepareMsg(CVehicleObject *veh)
{	ndl1.PrepareMsg(veh);
	ndl2.PrepareMsg(veh);
	ndl3.PrepareMsg(veh);
	CGauge::PrepareMsg(veh);
	return;	
}
int CThreeNeedleGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'ndl1':
    ReadFrom (&ndl1, stream);
    rc = TAG_READ;
    break;

  case 'ndl2':
    ReadFrom (&ndl2, stream);
    rc = TAG_READ;
    break;

  case 'ndl3':
    ReadFrom (&ndl3, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}


//=======================================================================
// CDigitalNeedleGauge
//=======================================================================
CDigitalNeedleGauge::CDigitalNeedleGauge (CPanel *mp)
: CNeedleGauge(mp)
{	
}
//------------------------------------------------------------------
int CDigitalNeedleGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'nbak':
    abak.Read(stream,2);
    return TAG_READ;

  case 'powr':
    ReadMessage (&powr, stream);
    return TAG_READ;
  }
  return CNeedleGauge::Read (stream, tag);
}

//=======================================================================
// JSDEV* simplified CAltimeterGauge
//=======================================================================
CAltimeterGauge::CAltimeterGauge (CPanel *mp)
: CBitmapGauge(mp)
{
  // Initialize members
  ndl1      = ndl2 = ndl3 = 0;
}
//--------------------------------------------------------------------
//  Free all resources
//--------------------------------------------------------------------
CAltimeterGauge::~CAltimeterGauge (void)
{
  if (ndl1) delete ndl1;
  if (ndl2) delete ndl2;
  if (ndl3) delete ndl3;
}
//--------------------------------------------------------------------
//  Read parameters
//---------------------------------------------------------------------
int CAltimeterGauge::Read (SStream *stream, Tag tag)
{ int pm;
  char txt[64];
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'psiz':
    ReadInt (&pm, stream);
    ReadInt (&pm, stream);
    rc = TAG_READ;
    break;

  case 'ndl1':
    ndl1 = new CNeedleGauge;
    ndl1->SetPanel(panel);
    ReadFrom (ndl1, stream);
    rc = TAG_READ;
    break;

  case 'ndl2':
    ndl2 = new CNeedleGauge;
    ndl2->SetPanel(panel);
    ReadFrom (ndl2, stream);
    rc = TAG_READ;
    break;

  case 'ndl3':
    ndl3 = new CNeedleGauge;
    ndl3->SetPanel(panel);
    ReadFrom (ndl3, stream);
    rc = TAG_READ;
    break;

  case 'csrl':
    ReadString (txt, 64, stream);
    rc = TAG_READ;
    break;

  case 'csrr':
    ReadString (txt, 64, stream);
    rc = TAG_READ;
    break;

  case 'koll':
    ReadFrom (&koll, stream);
    rc = TAG_READ;
    break;

  case 'kpxy':
    ReadInt (&pm, stream);
    ReadInt (&pm, stream);
    return TAG_READ;

  case 'knob':
    { alkn.SetGauge(this);
      ReadFrom(&alkn,stream);
      return TAG_READ;
    }
  case 'kmsg':
    ReadMessage (&kmsg, stream);
    kmsg.sender = 'kmsg';
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}
//-----------------------------------------------------------
//  JSDEV* Prepare Messages
//-----------------------------------------------------------
void CAltimeterGauge::PrepareMsg(CVehicleObject *veh)
{	kmsg.id       =	 MSG_SETDATA;
	kmsg.dataType = TYPE_REAL;
	veh->FindReceiver(&kmsg);
	CGauge::PrepareMsg(veh);
	return;	}
//-------------------------------------------------------------------
//	Draw note over knob
//-------------------------------------------------------------------
void CAltimeterGauge::DisplayHelp()
{ sprintf_s(hbuf,HELP_SIZE,"Baro %.2f",baro);
  FuiHelp();
  return;
}
//--------------------------------------------------------------------
//  Mouse mouves over
//--------------------------------------------------------------------
ECursorResult CAltimeterGauge::MouseMoved (int x, int y)
{ alkn.MouseMoved(x,y);
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//--------------------------------------------------------------------
//	Mouse click: Start knob rotation if hit
//--------------------------------------------------------------------
EClickResult CAltimeterGauge::MouseClick (int x, int y, int buttons)
{ if (!alkn.IsHit(x,y))             return MOUSE_TRACKING_OFF;    
  alkn.ArmRotation(0.01f,buttons);
	return MOUSE_TRACKING_ON;
}
//-----------------------------------------------------------------------
//	Mouse up:  stop rotation
//----------------------------------------------------------------------
EClickResult CAltimeterGauge::StopClick()
{	alkn.DisarmKnob();
	return	MOUSE_TRACKING_OFF;
}
//----------------------------------------------------------------------
//		Draw the altimeter
//-----------------------------------------------------------------------
void CAltimeterGauge::Draw(void)
{	// Get altitude value
	Send_Message(&mesg);
	float alt = mesg.realData;
  float f3  = alt * (float(36)/10000);
  alt       = fmod(alt,10000);
  float f2  = alt * (float(36)/1000);
  alt       = fmod(alt,1000);
  float f1  = alt * (float(36)/ 100);
//-----Erase and draw underlay --------------------------------------
	EraseSurfaceRGBA(surf,0);
  under.Draw(surf);
	if(ndl3) ndl3->DrawNDL(surf,int(f3));
	if(ndl2) ndl2->DrawNDL(surf,int(f2));
	if(ndl1)  ndl1->DrawNDL(surf,int(f1));
  //---- If koll value change: redraw and update barometer in subsystem
  if (alkn.HasChanged())
  { alkn.Redraw();
    kmsg.user.u.datatag = 'knob';
    kmsg.realData	      = alkn.GetChange();
    Send_Message (&kmsg);
    baro = float(kmsg.intData) * 0.01;
    DisplayHelp();
  }
  //-----Draw overlay ---------------------------------
	overl.Draw(surf);
	//--- Render gauge ----------------------------------------
	RenderSurface(GetVTAB(),surf);
  RenderSurface(alkn.GetVTAB(),alkn.GetSurface());
}
//=================================================================================
// CRollingAltimeterGauge
//=================================================================================
CRollingAltimeterGauge::CRollingAltimeterGauge (CPanel *mp)
: CBitmapGauge(mp)
{ // Initialize members
  psiz_x = psiz_y = 0;
  digx[0] = digx[1] = digx[2] = digx[3] = 0;
  digy = 0;
  dsiz_x = dsiz_y = 0;
  //----------------------------------------------
  d1  = 2;
  d2  = 9;
  d3  = 9;
  d4  = 2;
}
//-------------------------------------------------------------
//  Free all resources
//-------------------------------------------------------------
CRollingAltimeterGauge::~CRollingAltimeterGauge()
{ 
}
//-------------------------------------------------------------
//	Prepare Messages
//-------------------------------------------------------------
void CRollingAltimeterGauge::PrepareMsg(CVehicleObject *veh)
{	kmsg.id		  = MSG_SETDATA;
	veh->FindReceiver(&kmsg);
	CGauge::PrepareMsg(veh);
	return;
}
//--------------------------------------------------------------
		
int CRollingAltimeterGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'psiz':
    ReadInt (&psiz_x, stream);
    ReadInt (&psiz_y, stream);
    return TAG_READ;

  case 'ndl1':
    ReadFrom (&ndl1, stream);
    return TAG_READ;

  case 'zdig':
    zdig.Read(stream,0);
    return TAG_READ;

  case 'ndig':
    ndig.Read(stream,0);
    return TAG_READ;

  case 'tdig':
    tdig.Read(stream,0);
    return TAG_READ;

  case 'digx':
    ReadInt (&digx[0], stream);
    ReadInt (&digx[1], stream);
    ReadInt (&digx[2], stream);
    ReadInt (&digx[3], stream);
    ReadInt (&digx[4], stream);
    return TAG_READ;

  case 'digy':
    ReadInt (&digy, stream);
    return TAG_READ;

  case 'dsiz':
    ReadInt (&dsiz_x, stream);
    ReadInt (&dsiz_y, stream);
    return TAG_READ;

  case 'koll':
    ReadFrom (&koll, stream);
    return TAG_READ;

  case 'knob':
    aknb.SetGauge(this);
    ReadFrom(&aknb,stream);
    return TAG_READ;

  case 'kmsg':
    ReadMessage (&kmsg, stream);
    kmsg.sender = 'kmsg';
    return TAG_READ;
  }
    return CBitmapGauge::Read (stream, tag);
}
//-------------------------------------------------------------------
//  All parameters are read
//-------------------------------------------------------------------
void CRollingAltimeterGauge::ReadFinished()
{ int x;
  CBitmapGauge::ReadFinished();
  mesg.id = MSG_GETDATA;
  tdig.GetBitmapSize(&x,&lg1);
  ndig.GetBitmapSize(&x,&lg2);
  zdig.GetBitmapSize(&x,&lg3);
  wkol.Fill("SKYHHMSB.PBG");
  base  = koll.y1 - 6;
  return;
}
//-------------------------------------------------------------------
//  offset thousand digit position 
//  when going up
//-------------------------------------------------------------------
int CRollingAltimeterGauge::UpThousand(float hd)
{ if (hd < 7.0f)  return 0;
  return int ((7 - hd) * dsiz_y * 0.33f);
}
//-------------------------------------------------------------------
//  offset thousand digit position 
//  when going down
//-------------------------------------------------------------------
int CRollingAltimeterGauge::DnThousand(float hd)
{ if (hd > 3.0f)  return 0;
  return int ((3 - hd) * dsiz_y * 0.33f);
}
//-------------------------------------------------------------------
//  Draw digit
//  NOTE: Digit position into the bitmap are inverted
//        0 at end of the strip (pos 10 * dsiz_y) 
//  Future:  Implement parameters for preesure digits and placement
//-------------------------------------------------------------------
void CRollingAltimeterGauge::Draw()
{ //----Get altitude from subsystem -------------------------
  Send_Message(&mesg);
  float alt = mesg.realData;
  int   f1  = int(alt / 10000) + 1;
  alt       = fmod(alt,10000);
  int   f2  = int(alt / 1000)  + 1;
  alt       = fmod(alt,1000);
  float f3  = (alt / 100);
  //---------------------------------------------------------
  int   p1  = lg1 - int(f1 * dsiz_y);
  int   p2  = lg2 - int(f2 * dsiz_y);
  p2 += (mesg.realData > prev)?(UpThousand(f3)):(DnThousand(f3));
  int   p3  = lg3 - int((f3 + 1) * dsiz_y);
  //-----Erase and draw underlay    --------------------------
  EraseSurfaceRGBA(surf,0);
  under.Draw(surf);
  //-----Draw pressure in koll window ------------------------
  kmsg.id = MSG_GETDATA;
  kmsg.user.u.datatag = 'baro';
  Send_Message(&kmsg);
  U_INT pr  = (U_INT)kmsg.intData;
  d4        =   pr & 0xFF;
  d3        =  (pr >>  8) & 0xFF;
  d2        =  (pr >> 16) & 0xFF;
  d1        =  (pr >> 24) & 0xFF;
  wkol.Draw(surf,koll.x1    , base,d1);
  wkol.Draw(surf,koll.x1 + 6, base,d2);
  wkol.Draw(surf,koll.x1 +15, base,d3);
  wkol.Draw(surf,koll.x1 +21, base,d4);
  //-----Draw digits -----------------------------------------
  tdig.Draw(surf,p1,dsiz_y,digx[0],digy);
  ndig.Draw(surf,p2,dsiz_y,digx[1],digy);
  ndig.Draw(surf,p3,dsiz_y,digx[2],digy);
  zdig.Draw(surf,p3,dsiz_y,digx[3],digy);
  zdig.Draw(surf,p3,dsiz_y,digx[4],digy);
  prev  = mesg.realData;
  //-----Draw needle -----------------------------------------
  int deg = int (f3 * 36);           // 36° per 100 feet
  ndl1.DrawNDL(surf,deg);
  //---- If koll value change: redraw and update barometer in subsystem
  if (aknb.HasChanged())
  { kmsg.realData	      = aknb.GetChange();
    kmsg.user.u.datatag = 'knob';
    kmsg.id             = MSG_SETDATA;
    Send_Message (&kmsg);
    aknb.Redraw();
  }
  //-----Draw overlay ---------------------------------
	overl.Draw(surf);
	//--- Render -----------------------------------------
	RenderSurface(GetVTAB(),surf);
  RenderSurface(aknb.GetVTAB(),aknb.GetSurface());
  return;
}
//--------------------------------------------------------------------
//  Mouse moves over
//--------------------------------------------------------------------
ECursorResult CRollingAltimeterGauge::MouseMoved (int x, int y)
{ aknb.MouseMoved(x,y);
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//--------------------------------------------------------------------
//	Mouse click: Start knob rotation if hit
//--------------------------------------------------------------------
EClickResult CRollingAltimeterGauge::MouseClick (int x, int y, int buttons)
{ if (!aknb.IsHit(x,y))             return MOUSE_TRACKING_OFF;    
  aknb.ArmRotation(0.01f,buttons);
	return MOUSE_TRACKING_ON;
}
//-----------------------------------------------------------------------
//	Mouse up:  stop rotation
//----------------------------------------------------------------------
EClickResult CRollingAltimeterGauge::StopClick()
{	aknb.DisarmKnob();
	return	MOUSE_TRACKING_OFF;
}
//===========================================================================
// JSDEV* modified CHorizonGauge
//===========================================================================
CHorizonGauge::CHorizonGauge (CPanel *mp)
: CBitmapGauge(mp)
{	hoff                = 0;
	pixd                = 0;
	moff                = 0;
  hdeg                = 0;
	strcpy (flag_art,     "");
  // set messages default tag
	pich.user.u.datatag = 'pich';
  pich.sender = 'pich';

	roll.user.u.datatag = 'roll';
  roll.sender = 'roll';
}
//----------------------------------------------------------------------
CHorizonGauge::~CHorizonGauge (void)
{ 
}
//----------------------------------------------------------------------
//	Prepare Messages
//----------------------------------------------------------------------
void CHorizonGauge::PrepareMsg(CVehicleObject *veh)
{	veh->FindReceiver(&pich);
	veh->FindReceiver(&roll);
	CGauge::PrepareMsg(veh);
	return;	
}
//---------------------------------------------------------------------
//	Read all tags
//---------------------------------------------------------------------
int CHorizonGauge::Read (SStream *stream, Tag tag)
{ switch (tag) {
  case 'foot':
    Bfoot.Read(stream,2);
	  return TAG_READ;

  case 'horz':
	  Bhorz.Read(stream,2);
	  return TAG_READ;

  case 'irim':
    Birim.Read(stream,2);
	  return TAG_READ;

  case 'orim':
    Borim.Read(stream,2);
	  return TAG_READ;

  case 'fron':
    Bfron.Read(stream,2);
	  return TAG_READ;

  case 'hoff':
    ReadInt (&hoff, stream);
	  return TAG_READ;

  case 'pixd':
    ReadFloat (&pixd, stream);
	  return TAG_READ;

  case 'moff':
    ReadInt (&moff, stream);
	  return TAG_READ;

  case 'knob':
    { hknob.SetGauge(this);
      ReadFrom(&hknob,stream);
	    return TAG_READ;
    }

  case 'flag':
    ReadString (flag_art, 64, stream);
	  return TAG_READ;

  case 'pich':
    ReadMessage (&pich, stream);
	  return TAG_READ;

  case 'roll':
    ReadMessage (&roll, stream);
	  return TAG_READ;
  }

  return CBitmapGauge::Read (stream, tag);
}
//----------------------------------------------------------------------
//  All parameters are read. Init picth and roll destination
//----------------------------------------------------------------------
void CHorizonGauge::ReadFinished()
{ if (0 == pich.group) pich.group = mesg.group;
  if (0 == roll.group) roll.group = mesg.group;
  pich.id				      = MSG_GETDATA;
	pich.dataType	      = TYPE_REAL;
 	roll.id				      = MSG_GETDATA;
	roll.dataType		    = TYPE_REAL;
  mesg.user.u.datatag = 'knob';
  mesg.id             = MSG_SETDATA;
  CBitmapGauge::ReadFinished();
}
//----------------------------------------------------------------------
//  Mouse move over
//-----------------------------------------------------------------------
ECursorResult CHorizonGauge::MouseMoved (int x, int y)
{ // Send updated mouse position to all click areas
  hknob.MouseMoved(x,y);
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//----------------------------------------------------------------
//		Click on Knob, arm rotation
//----------------------------------------------------------------
EClickResult CHorizonGauge::MouseClick (int x, int y, int buttons)
{ if (!hknob.IsHit(x,y))    return 	MOUSE_TRACKING_OFF;
  hknob.ArmRotation(1,buttons);
	return 	MOUSE_TRACKING_ON;
}

//-----------------------------------------------------------------
//	When click stop disarm rotation
//-----------------------------------------------------------------
EClickResult CHorizonGauge::StopClick()
{ hknob.DisarmKnob();
  return MOUSE_TRACKING_OFF;
}
//-----------------------------------------------------------------------------
//	JSDEV* ATTITUDE subsystem now gives pitch and roll in degre
//-----------------------------------------------------------------------------
void CHorizonGauge::Draw(void)
{	EraseSurfaceRGBA(surf,0);
  under.Draw(surf);
	// Get Pitch value
	Send_Message (&pich);
  float pichD = pich.realData;
	// Get Roll value
  Send_Message (&roll);
  int   rollD = roll.intData;
  mesg.id      = MSG_GETDATA;
  Send_Message(&mesg);
  hdeg  = mesg.intData;
	// Draw foot
  int  yp	= Bfoot.GetY() - (int)(pichD * pixd);
  int  xp = Bfoot.GetX();
	Bfoot.Draw(surf,xp,yp,rollD);
	// Draw horz
  xp  = Bhorz.GetX();
  yp  = Bhorz.GetY() + hdeg;
	Bhorz.Draw(surf,xp,yp, 0);
	// Draw irim 
  Birim.Draw(surf,rollD);
	// Draw orim
	Borim.Draw(surf);
 	// Draw fron
	Bfron.Draw(surf);
	//--- Draw knob and signal change to subsystem ------
  if (hknob.HasChanged())
  { hknob.Redraw();
    mesg.id      = MSG_SETDATA;
    mesg.intData = hknob.GetChange();
    Send_Message(&mesg);
  }
	overl.Draw(surf);
	//--- Render -------------------------------------
	RenderSurface (GetVTAB(),surf);
  RenderSurface(hknob.GetVTAB(),hknob.GetSurface());
	return;
}
//===========================================================================
// CFDHorizonGauge
//===========================================================================
CFDHorizonGauge::CFDHorizonGauge (CPanel *mp)
: CHorizonGauge(mp)
{ 
}


int CFDHorizonGauge::Read (SStream *stream, Tag tag)
{
 switch (tag) {
  case 'bars':
    abar.Read(stream,2);
    return TAG_READ;
  }
  return  CHorizonGauge::Read (stream, tag);
}


//==========================================================================
// CHorizontalBallGauge
//==========================================================================
CHorizontalBallGauge::CHorizontalBallGauge (CPanel *mp)
: CBitmapGauge(mp)
{
}

CHorizontalBallGauge::~CHorizontalBallGauge (void)
{
}

int CHorizontalBallGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}


void CHorizontalBallGauge::ReadFinished (void)
{
}

ECursorResult CHorizontalBallGauge::MouseMoved (int x, int y)
{ return CURSOR_WAS_CHANGED; }

EClickResult CHorizontalBallGauge::MouseClick (int x, int y, int buttons)
{ return MOUSE_TRACKING_OFF;  }

void CHorizontalBallGauge::Draw(void)
{	Update (); }

void CHorizontalBallGauge::Update (void)
{
  // Send the 'mesg' message to the supplier subsystem
  mesg.id = MSG_GETDATA;
  Send_Message (&mesg);

  // we already know 'alti' is a float
  value = (float)mesg.realData;
}


//=================================================================
// CAirspeedGauge
//==================================================================
CAirspeedGauge::CAirspeedGauge (CPanel *mp)
: CNeedleGauge(mp)
{
}
//----------------------------------------------------------
//  Read all parameters
//----------------------------------------------------------
int CAirspeedGauge::Read (SStream *stream, Tag tag)
{ switch (tag) {
  case 'knob':
    ReadFrom (&knob, stream);
    return TAG_READ;

  case 'mmo_':
    ReadFrom (&mmo_, stream);
    return TAG_READ;
  }

  return CNeedleGauge::Read (stream, tag);
}
//-----------------------------------------------------------
//  Mouse move over
//-----------------------------------------------------------
ECursorResult CAirspeedGauge::MouseMoved (int x, int y)
{ knob.MouseMoved (x, y);
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//======================================================================
// CVerticalSpeedGauge
//======================================================================
CVerticalSpeedGauge::CVerticalSpeedGauge (CPanel *mp)
: CNeedleGauge(mp)
{ ;}
//---------------------------------------------------------------------
//	JSDEV* Prepare messages
//---------------------------------------------------------------------
void CVerticalSpeedGauge::PrepareMsg(CVehicleObject *veh)
{	vmsg.id			  = MSG_GETDATA;
  vmsg.dataType	= TYPE_REAL;
	veh->FindReceiver(&vmsg);
	CNeedleGauge::PrepareMsg(veh);
	return;
}
//---------------------------------------------------------------------
CVerticalSpeedGauge::~CVerticalSpeedGauge (void)
{
}
//---------------------------------------------------------------------
//	Read tags
//---------------------------------------------------------------------
int CVerticalSpeedGauge::Read (SStream *stream, Tag tag)
{
  switch (tag) {
  case 'vsbg':
    vsbg.Read(stream,2);
    return TAG_READ;

  case 'vskb':
    ReadFrom (&vskb, stream);
    return TAG_READ;

  case 'vmsg':
    ReadMessage (&vmsg, stream);
    return TAG_READ;
  }

  return CNeedleGauge::Read (stream, tag);
}
//-------------------------------------------------------------------
void CVerticalSpeedGauge::ReadFinished (void)
{  CNeedleGauge::ReadFinished ();
   vmsg.sender = unId;

}
//-------------------------------------------------------------------
//  Mouse moves over
//-------------------------------------------------------------------
ECursorResult CVerticalSpeedGauge::MouseMoved (int x, int y)
{ vskb.MouseMoved (x, y);
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//-------------------------------------------------------------------
//	Draw the gauge
//-------------------------------------------------------------------
void CVerticalSpeedGauge::Draw (void)
{ // Draw vertical speed needle
  CNeedleGauge::Draw ();
  // Get vertical speed bug value
  Send_Message (&vmsg);
  vsbg.Draw(surf);
  return;
}
//=====================================================================
// JSDEV* modified CDirectionalGyroGauge
//=====================================================================
CDirectionalGyroGauge::CDirectionalGyroGauge (CPanel *mp)
: CNeedleGauge(mp)
{	  dir = 0;}
//-------------------------------------------------------------
CDirectionalGyroGauge::~CDirectionalGyroGauge (void)
{;}
//-------------------------------------------------------------
//  Read GYRO tags
//-------------------------------------------------------------
int CDirectionalGyroGauge::Read (SStream *stream, Tag tag)
{ Tag  apb = 0;
  switch (tag) {
  case 'apbg':
		apbg.Read(stream, 2);
    ReadTag(&apb,stream);
    mbug.user.u.datatag = apb;
		return TAG_READ;

  case 'apkb':
    pikn.ReadParameters(this,stream);
		return TAG_READ;

  case 'knob':
    gykn.ReadParameters(this,stream);
		return TAG_READ;

  case 'bias':
    ReadTag (&bias, stream);
		return TAG_READ;
  }

  return  CNeedleGauge::Read (stream, tag);
}

//-------------------------------------------------------------------
//  End of tag read
//-------------------------------------------------------------------
void CDirectionalGyroGauge::ReadFinished ()
{ mbug.group			= mesg.group;
  mbug.sender     = unId;
	mbug.id         = MSG_SETDATA;
	mbug.dataType   = TYPE_REAL;
  //------------------------------------------
  mgyr.group			= mesg.group;
  mgyr.sender         = unId;
	mgyr.user.u.datatag = 'dgyr';
	mgyr.id             = MSG_SETDATA;
	mgyr.dataType       = TYPE_REAL;
	CNeedleGauge::ReadFinished ();
}
//-------------------------------------------------------------------
//	Prepare message
//------------------------------------------------------------------
void CDirectionalGyroGauge::PrepareMsg(CVehicleObject *veh)
{	veh->FindReceiver(&mbug);
  veh->FindReceiver(&mgyr);
	CNeedleGauge::PrepareMsg(veh);
	return;	
}
//----------------------------------------------------------------------
//	Check for knob click
//----------------------------------------------------------------------
EClickResult CDirectionalGyroGauge::MouseClick (int x, int y, int buttons)
{ // Initialize Ap knob hold-down values
  if (pikn.IsHit(x,y))
	{	pikn.ArmRotation(1,buttons);
    DisplayBUG();
		return MOUSE_TRACKING_ON;
	}
  if (gykn.IsHit(x,y))
	{	gykn.ArmRotation(1,buttons);
    DisplayHDG();
		return MOUSE_TRACKING_ON;
	}
  return MOUSE_TRACKING_OFF;
}
//---------------------------------------------------------------------
//	stop click
//---------------------------------------------------------------------
EClickResult CDirectionalGyroGauge::StopClick()
{ // Release OBS knob click if it is set
  pikn.DisarmKnob();
  gykn.DisarmKnob();
  return MOUSE_TRACKING_OFF;
}
//----------------------------------------------------------------------
//	Draw the gauge
//----------------------------------------------------------------------
void CDirectionalGyroGauge::Draw (void)
{ EraseSurfaceRGBA(surf,0);
  under.Draw(surf);
	// Draw parent needle
  Send_Message(&mesg);
  hdg = int(mesg.realData);
  bug = mesg.user.u.unit;
	DrawNDL(surf,hdg);
	//-----Update the gyro knob --------------------------------
  if (gykn.HasChanged())
	{	mgyr.id       = MSG_SETDATA;
    mgyr.realData = gykn.GetChange();
		Send_Message(&mgyr);
    gykn.Redraw();
    DisplayHDG();
	}
	//------Update the auto pilot knob ---and bug---------------
  mbug.realData = 0;
  if (pikn.HasChanged()) 
  { pikn.Redraw();
    mbug.id       = MSG_SETDATA;
    mbug.realData = pikn.GetChange();
    Send_Message(&mbug);
    bug           =  mbug.intData;
    dir           =  mbug.user.u.unit;
    DisplayBUG();
  }
	//-----Draw the bug over the plate --------------------------
	apbg.Draw(surf,bug);
  overl.Draw(surf);
	//--- Render ------------------------------------------------
	RenderSurface(GetVTAB(),surf);
  RenderSurface(pikn.GetVTAB(),pikn.GetSurface());
  RenderSurface(gykn.GetVTAB(),gykn.GetSurface());
	return;
}
//-------------------------------------------------------------------
//	Draw gyro heading
//-------------------------------------------------------------------
void CDirectionalGyroGauge::DisplayHDG()
{ sprintf_s(hbuf,HELP_SIZE,"DIR: %03d°",hdg);
  FuiHelp();
}
//------------------------------------------------------------------------
//  Display autopilot bug direction
//------------------------------------------------------------------------
void CDirectionalGyroGauge::DisplayBUG()
{ sprintf_s(hbuf,HELP_SIZE,"BUG: %03d°",dir);
  FuiHelp();
}
//-------------------------------------------------------------------
//	Mouse move over
//-------------------------------------------------------------------
ECursorResult CDirectionalGyroGauge::MouseMoved (int x, int y)
{	ECursorResult rc = CURSOR_WAS_CHANGED;
  if (gykn.MouseMoved(x,y)) {DisplayHDG();  return rc;}
  if (pikn.MouseMoved(x,y)) {DisplayBUG();  return rc;}
  DisplayHelp();
  return rc;
}
//==========================================================================
// CVacuumGauge
//==========================================================================
CVacuumGauge::CVacuumGauge (CPanel *mp)
: CNeedleGauge(mp)
{
}

int CVacuumGauge::Read (SStream *stream, Tag tag)
{ return CNeedleGauge::Read (stream, tag);
}

void CVacuumGauge::Draw (void)
{
  CNeedleGauge::Draw ();

//  #ifdef _DEBUG	
//	  FILE *fp_debug;
//	  if(!(fp_debug = fopen("__DDEBUG_Pump.txt", "a")) == NULL)
//	  {
//		  fprintf(fp_debug, "CVacuumGauge::Draw %.2f\n", value);
//		  fclose(fp_debug); 
//	  }
//  #endif
}

//======================================================================
// CSuctionGauge
//======================================================================
CSuctionGauge::CSuctionGauge (CPanel *mp)
: CNeedleGauge(mp)
{
}

int CSuctionGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc == TAG_IGNORED) {
    rc = CNeedleGauge::Read (stream, tag);
  }

  return rc;
}
//=========================================================================
// CFuelGauge
//=========================================================================
CFuelGauge::CFuelGauge (CPanel *mp)
: CNeedleGauge(mp)

{
}

int CFuelGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc == TAG_IGNORED) {
    rc = CNeedleGauge::Read (stream, tag);
  }

  return rc;
}

void CFuelGauge::Draw (void)
{
  CNeedleGauge::Draw ();
}


//======================================================================
// CFuelFlowGauge
//======================================================================
CFuelFlowGauge::CFuelFlowGauge (CPanel *mp)
: CNeedleGauge(mp)

{
}

int CFuelFlowGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc == TAG_IGNORED) {
    rc = CNeedleGauge::Read (stream, tag);
  }

  return rc;
}



//====================================================================
// COilPressureGauge
//=====================================================================
COilPressureGauge::COilPressureGauge (CPanel *mp)
: CNeedleGauge(mp)

{
}
//-----------------------------------------------------------
//  Read all parameters
//-----------------------------------------------------------
int COilPressureGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc == TAG_IGNORED) {
    rc = CNeedleGauge::Read (stream, tag);
  }

  return rc;
}

//====================================================================
// COilTemperatureGauge
//------------------------------------------------------------------------
COilTemperatureGauge::COilTemperatureGauge (CPanel *mp)
: CNeedleGauge(mp)

{
}

int COilTemperatureGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc == TAG_IGNORED) {
    rc = CNeedleGauge::Read (stream, tag);
  }

  return rc;
}
//==============================================================================
// CTachometerGauge
//===============================================================================
CTachometerGauge::CTachometerGauge (CPanel *mp)
: CNeedleGauge(mp)
{  hobb = false;
  hobs = NULL;
}

CTachometerGauge::~CTachometerGauge (void)
{
  SAFE_DELETE (hobs);
}
//------------------------------------------------------------------------------
//  read parameters
//------------------------------------------------------------------------------
int CTachometerGauge::Read (SStream *stream, Tag tag)
{switch (tag) {
  case 'hobb':
    hobs = new CHobbsMeterGauge(panel);
    hobs->SetPanel(panel);
    ReadFrom (hobs, stream);
    return TAG_READ;
  }
  return CNeedleGauge::Read (stream, tag);
}
//------------------------------------------------------------------------------
//  All parameters are read
//------------------------------------------------------------------------------
void CTachometerGauge::ReadFinished (void)
{
  CNeedleGauge::ReadFinished ();
}
//------------------------------------------------------------------------------
//  Draw this gauge
//------------------------------------------------------------------------------
void CTachometerGauge::Draw (void)
{  if (0 == surf) return;
   if (hobs) hobs->Draw ();
   CNeedleGauge::Draw ();
	 //--- Render -------------------------------------------
	 if (hobs) hobs->RenderGauge();
   RenderSurface (GetVTAB(),surf);
   return;
}
//================================================================================
// CDualTachometerGauge
//================================================================================
CDualTachometerGauge::CDualTachometerGauge (CPanel *mp)
: CTwoNeedleGauge(mp)
{
}
//------------------------------------------------------------------
//  Read all parameters
//------------------------------------------------------------------
int CDualTachometerGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc == TAG_IGNORED) {
    rc = CTwoNeedleGauge::Read (stream, tag);
  }

  return rc;
}

//------------------------------------------------------------------
//  Draw the gauge
//------------------------------------------------------------------
void CDualTachometerGauge::Draw()
{ CTwoNeedleGauge::Draw();
  return;
}
//================================================================================
// CN1TachometerGauge
//================================================================================
CN1TachometerGauge::CN1TachometerGauge (CPanel *mp)
: CTwoNeedleGauge(mp)
{
}

int CN1TachometerGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'knob':
    ReadFrom (&knob, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CTwoNeedleGauge::Read (stream, tag);
  }

  return rc;
}
//-----------------------------------------------------------------
//  Mouse move over
//------------------------------------------------------------------
ECursorResult CN1TachometerGauge::MouseMoved (int x, int y)
{ // Send updated mouse position to all click areas
  knob.MouseMoved (x, y);
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//======================================================================
// CExhaustGasTemperatureGauge
//======================================================================
CExhaustGasTemperatureGauge::CExhaustGasTemperatureGauge (CPanel *mp)
: CNeedleGauge(mp)
{ strcpy (egtn_art, "");
  redf = 125;
}
//---------------------------------------------------------------------
//  Destroy gauge
//---------------------------------------------------------------------
CExhaustGasTemperatureGauge::~CExhaustGasTemperatureGauge (void)
{

}
//-------------------------------------------------------------------------
//  Mouse move over 
//-------------------------------------------------------------------------
ECursorResult CExhaustGasTemperatureGauge::MouseMoved (int x, int y)
{ DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//-------------------------------------------------------------------------
//  Mouse click 
//-------------------------------------------------------------------------
EClickResult CExhaustGasTemperatureGauge::MouseClick (int x, int y, int buttons)
{ if (!knob.IsHit(x,y))             return MOUSE_TRACKING_OFF;    
  knob.ArmRotation(1,buttons);
	return MOUSE_TRACKING_ON;
}
//----------------------------------------------------------------------------
//  Stop click over
//----------------------------------------------------------------------------
EClickResult CExhaustGasTemperatureGauge::StopClick()
{ knob.DisarmKnob();
	return MOUSE_TRACKING_OFF;
}
//----------------------------------------------------------------------------
//  READ PARAMETERS
//----------------------------------------------------------------------------
int CExhaustGasTemperatureGauge::Read (SStream *stream, Tag tag)
{ char txt[128];
  switch (tag) {
    case 'egtr':
      ReadInt (&redf, stream);
      return TAG_READ;

    case 'egtn':
      ReadString (txt, 64, stream);
      knob.AssignNeedle(txt);
      return TAG_READ;
  }

  return CNeedleGauge::Read (stream, tag);
}
//----------------------------------------------------------------------------
//  All parameters are read
//----------------------------------------------------------------------------
void CExhaustGasTemperatureGauge::ReadFinished (void)
{ CNeedleGauge::ReadFinished ();
  knob.Share(this);
  knob.SetFrame(redf);
  return;
}
//----------------------------------------------------------------------------
//  Draw gauge
//----------------------------------------------------------------------------
void CExhaustGasTemperatureGauge::Draw (void)
{  // Draw white needle first (blits the red one otherwise)
  CNeedleGauge::Draw ();
  knob.DrawAsNeedle();
  return;
}

//====================================================================
//
// CAnalogClockGauge
//====================================================================
CAnalogClockGauge::CAnalogClockGauge (CPanel *mp)
: CTwoNeedleGauge(mp)
{
}

//---------------------------------------------------------------------
//  Read all parameters
//---------------------------------------------------------------------
int CAnalogClockGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc == TAG_IGNORED) {
    rc = CTwoNeedleGauge::Read (stream, tag);
  }
  return rc;
}
//---------------------------------------------------------------------
//  Draw Time according to global clock
//---------------------------------------------------------------------
void CAnalogClockGauge::Draw()
{ CClock *clk = globals->clk;
  U_SHORT dh = clk->GetHdeg();
  U_SHORT dm = clk->GetMdeg();
  EraseSurfaceRGBA(surf,0);
  DrawNDL1(surf,dh);
  DrawNDL2(surf,dm);
  return;
}
//=======================================================================
// CAmmeterGauge
//=======================================================================
CAmmeterGauge::CAmmeterGauge (CPanel *mp)
: CNeedleGauge(mp)

{
}

int CAmmeterGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc == TAG_IGNORED) {
    rc = CNeedleGauge::Read (stream, tag);
  }

  return rc;
}



//=====================================================================
// JSDEV* modified CBasicADFGauge
//====================================================================
CBasicADFGauge::CBasicADFGauge (CPanel *mp)
: CNeedleGauge(mp)
{
  comp_tag = 0;
  comp = NULL;
  hdng = NULL;
  cpas = 0;
}
//--------------------------------------------------------------------
CBasicADFGauge::~CBasicADFGauge (void)
{
  if (comp != NULL) delete comp;
  if (hdng != NULL) delete hdng;
}
//--------------------------------------------------------------------
//  Read the tags
//--------------------------------------------------------------------
int CBasicADFGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'comp':
    ReadTag (&comp_tag, stream);
    comp = new CGaugeNeedle;
    ReadFrom (comp, stream);
    return TAG_READ;

  case 'knob':
    aknb.SetGauge(this);
    ReadFrom(&aknb,stream);
    return TAG_READ;

  case 'hdng':
    hdng = new CGaugeNeedle;
    ReadFrom (hdng, stream);
    return TAG_READ;
  }
  return CNeedleGauge::Read (stream, tag);
  }
//---------------------------------------------------------------
void CBasicADFGauge::ReadFinished (void)
{ mesg.user.u.hw = HW_RADIO;
  CNeedleGauge::ReadFinished ();
}
//---------------------------------------------------------------
//	Mouse moves over
//---------------------------------------------------------------
ECursorResult CBasicADFGauge::MouseMoved (int x, int y)
{ // Send updated mouse position to all click areas
  if (aknb.MouseMoved(x,y)) {DisplayCMP(); return CURSOR_WAS_CHANGED;}
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//---------------------------------------------------------------
//	knob clicked => Start rotation
//---------------------------------------------------------------
EClickResult CBasicADFGauge::MouseClick (int x, int y, int buttons)
{	if (!aknb.IsHit(x,y))   return MOUSE_TRACKING_OFF;    
  aknb.ArmRotation(1,buttons);
  DisplayCMP();
	return MOUSE_TRACKING_ON;
}
//----------------------------------------------------------------
//	Stop click:  Disarm rotation
//----------------------------------------------------------------
EClickResult CBasicADFGauge::StopClick()
{	aknb.DisarmKnob();
	return MOUSE_TRACKING_OFF;
}
//----------------------------------------------------------------
//	Draw the gauge
//------------------------------------------------------------------
void CBasicADFGauge::Draw (void)
{	// Get navaid pointer from ADF radio
	mesg.id = MSG_GETDATA;
	mesg.user.u.datatag = 'navd';
	Send_Message (&mesg);
	navd = (float)mesg.realData;
  //-----Get compass ----------------------------------------
  mesg.user.u.datatag = 'comp';
	Send_Message (&mesg);
  cpas = mesg.intData;
	//------draw compass ---------------------------------
  EraseSurfaceRGBA(surf,0);
  under.Draw(surf);
  comp->Draw (surf, cpas, cx, cy);
  nedl.Draw  (surf, navd, cx, cy);
  if (hdng) hdng->Draw (surf, navd, cx, cy);
	//---------- Draw knob surface------------------------
  if (aknb.HasChanged())
  { aknb.Redraw();
    mesg.id             = MSG_SETDATA;
    mesg.user.u.datatag = 'comp';
    mesg.realData       = aknb.GetChange();
    Send_Message(&mesg);
    DisplayCMP();
  }
	overl.Draw(surf);
	//--- Render -----------------------------------------
	RenderSurface(GetVTAB(),surf);
  RenderSurface(aknb.GetVTAB(),aknb.GetSurface());
  return;
}
//----------------------------------------------------------------
//	Display Compass
//------------------------------------------------------------------
void CBasicADFGauge::DisplayCMP()
{ sprintf_s(hbuf,HELP_SIZE,"DIR %03d°",cpas);
  FuiHelp();
}
//========================================================================
// CDigitalReadoutGauge
//========================================================================
CDigitalReadoutGauge::CDigitalReadoutGauge (CPanel *mp)
: CBitmapGauge(mp)
{ maxd = 0;
  digY = 0;
  nDig = 0;
  whol = false;
  padz = false;
  for (int k =0; k != 8; k++) digX[k] = 0;

}
//-----------------------------------------------------------
//  Read parameters
//------------------------------------------------------------
int CDigitalReadoutGauge::Read (SStream *stream, Tag tag)
{ switch (tag) {
    case 'maxd':
      ReadInt (&maxd, stream);
      return TAG_READ;
    case 'wdig':
      wdig.Read(stream,0);
      return  TAG_READ;
    case 'fdig':
      fdig.Read(stream,0);
      return  TAG_READ;

    case 'digY':
      ReadInt (&digY, stream);
      return TAG_READ;

    case 'digX':
      { ReadInt (&nDig, stream);
        if (nDig > 8) nDig = 8;
        int end = nDig - 1;
        for (int i=0; i<nDig; i++) ReadInt (&digX[end - i], stream);
      }
      return TAG_READ;

    case 'whol':
      whol = true;
      return TAG_READ;

    case 'padz':
      padz = true;
      return TAG_READ;
    }
  return CGauge::Read (stream, tag);
}
//-----------------------------------------------------------
//  All parameters are read
//-----------------------------------------------------------
void CDigitalReadoutGauge::ReadFinished()
{ CBitmapGauge::ReadFinished();
  mesg.dataType = TYPE_REAL;
}
//-----------------------------------------------------------
//  Draw the Gauge
//------------------------------------------------------------
void CDigitalReadoutGauge::Draw()
{ EraseSurfaceRGBA(surf,0);
  under.Draw(surf);
  Update();
  int   val = int(value);
  char  dig[16];
  _snprintf(dig,16,"%0*u",nDig,val);
	dig[15] = 0;
  //--- Edit digit on surface ------
  int No = nDig - 1;
  while (No != -1)
  { int frm = dig[No] - '0';        // Frame number
    wdig.Draw(surf,digX[No],digY,frm);
    No--;
  }
  overl.Draw(surf);
}
//=====================================================================
// CDigitsReadoutGauge
//=====================================================================
CDigitsReadoutGauge::CDigitsReadoutGauge (CPanel *mp)
: CGauge(mp)
{ frgb_r  = frgb_g = frgb_b = 240;
  strcpy (just, "");
}
//--------------------------------------------------------------------
//  read All parameters
//--------------------------------------------------------------------
int CDigitsReadoutGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'frgb':
    ReadInt (&frgb_r, stream);
    ReadInt (&frgb_g, stream);
    ReadInt (&frgb_b, stream);
    return  TAG_READ;

  case 'just':
    ReadString (just, 64, stream);
    return  TAG_READ;
  }

  if (rc == TAG_IGNORED) {
    rc = CGauge::Read (stream, tag);
  }

  return rc;
}


//
// CCabinPressureRateKnobGauge
//
CCabinPressureRateKnobGauge::CCabinPressureRateKnobGauge (CPanel *mp)
: CBitmapGauge(mp)
{
  ofxy_x = ofxy_y = 0;
  alti_min = alti_max = 0;
  rate_min = rate_max = 0;
  strcpy (onsf, "");
  strcpy (ofsf, "");
}


int CCabinPressureRateKnobGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'ofxy':
    ReadInt (&ofxy_x, stream);
    ReadInt (&ofxy_y, stream);
    rc = TAG_READ;
    break;

  case 'alti':
    ReadFloat (&alti_min, stream);
    ReadFloat (&alti_max, stream);
    rc = TAG_READ;
    break;

  case 'rate':
    ReadFloat (&rate_min, stream);
    ReadFloat (&rate_max, stream);
    rc = TAG_READ;
    break;

  case 'cabn':
    ReadFrom (&cabn, stream);
    rc = TAG_READ;
    break;

  case 'knob':
    ReadFrom (&knob, stream);
    rc = TAG_READ;
    break;

  case 'onsf':
    ReadString (onsf, 64, stream);
    rc = TAG_READ;
    break;

  case 'ofsf':
    ReadString (ofsf, 64, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}
//------------------------------------------------------------------
//  Mouse move over
//------------------------------------------------------------------
ECursorResult CCabinPressureRateKnobGauge::MouseMoved (int x, int y)
{ if (cabn.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  if (knob.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//-------------------------------------------------------------------
// CCabinPressureGauge
//-------------------------------------------------------------------
CCabinPressureGauge::CCabinPressureGauge (CPanel *mp)
:CNeedleGauge(mp)
{}

int CCabinPressureGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'alti':
    ReadFrom (&alti, stream);
    rc = TAG_READ;
    break;

  case 'diff':
    ReadFrom (&diff, stream);
    rc = TAG_READ;
    break;

  case 'rate':
    ReadFrom (&rate, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CNeedleGauge::Read (stream, tag);
  }

  return rc;
}


//
// CBrakePressureGauge
//
CBrakePressureGauge::CBrakePressureGauge (CPanel *mp)
: CTwoNeedleGauge(mp)
{
}

int CBrakePressureGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'left':
    ReadFrom (&left, stream);
    rc = TAG_READ;
    break;

  case 'rght':
    ReadFrom (&rght, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CTwoNeedleGauge::Read (stream, tag);
  }

  return rc;
}
//------------------------------------------------------------------
//  Mouse move over
//------------------------------------------------------------------
ECursorResult CBrakePressureGauge::MouseMoved (int x, int y)
{ // Send updated mouse position to all click areas
  if (left.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  if (rght.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}

//-----------------------------------------------------------------
// CAOANeedleGauge
//
CAOANeedleGauge::CAOANeedleGauge (CPanel *mp)
: CTwoNeedleGauge(mp)
{
}

int CAOANeedleGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'knob':
    ReadFrom (&knob, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CTwoNeedleGauge::Read (stream, tag);
  }

  return rc;
}
//----------------------------------------------------------------
//  Mouse move over
//----------------------------------------------------------------
ECursorResult CAOANeedleGauge::MouseMoved (int x, int y)
{ // Send updated mouse position to all click areas
  if (knob.MouseMoved(x, y)) return CURSOR_WAS_CHANGED;
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//=========================================================================
// JSDEV* modified CTurnCoordinatorGauge
//=========================================================================
CTurnCoordinatorGauge::CTurnCoordinatorGauge (CPanel *mp)
: CBitmapGauge(mp)
{	pHead	= -400;								// Force first display
	ratio	= 0;
	pcon	= bcon = 0;
	Ltrn	= Rtrn = 0;
}
//------------------------------------------------------------------------
CTurnCoordinatorGauge::~CTurnCoordinatorGauge (void)
{
}
//-------------------------------------------------------------------------
//	Read all tags
//-------------------------------------------------------------------------
int CTurnCoordinatorGauge::Read (SStream *stream, Tag tag)
{ short u = 0;
  short v = 0;
  switch (tag) {
  case 'plan':
		plan.Read(stream,0);
		ratio	= plan.GetNbFrame() / 4;				// amplitude in [-2,+2]
		return TAG_READ;

  case 'poff':
		ReadShort (&u, stream);
		ReadShort (&v, stream);
    plan.SetX(u);
    plan.SetY(v);
		return TAG_READ;

  case 'ball':
		ball.Read(stream, 0);
		return TAG_READ;

  case 'boff':
		ReadShort (&u, stream);
		ReadShort (&v, stream);
    ball.SetX(u);
    ball.SetY(v);
		return TAG_READ;

  case 'pcon':
		ReadTag (&pcon, stream);
		return TAG_READ;

  case 'bcon':
		ReadTag (&bcon, stream);
		return TAG_READ;

  case 'Ltrn':
		ReadInt (&Ltrn, stream);
		return TAG_READ;

  case 'Rtrn':
		ReadInt (&Rtrn, stream);
		return TAG_READ;
  }
  return CBitmapGauge::Read (stream, tag);
}

//--------------------------------------------------------------------
//	JSDEV*  Read the subsystem and compute bitmap frame
//--------------------------------------------------------------------

void CTurnCoordinatorGauge::Draw(void)
{	EraseSurfaceRGBA(surf,0);
  under.Draw(surf);
	// Get plane value
	mesg.id = MSG_GETDATA;
	mesg.dataType = TYPE_REAL;
	mesg.user.u.datatag = pcon;
	Send_Message (&mesg);
	float heading = (float)mesg.realData;

  // Get ball value
	mesg.id = MSG_GETDATA;
	mesg.dataType = TYPE_REAL;
	mesg.user.u.datatag = bcon;
	Send_Message (&mesg);
	float acc = (float)mesg.realData;
 
	// Draw plane: nFrame is for the [-2,+2] interval
	int pframe = (plan.GetNbFrame() / 2) + (ratio * heading);
	plan.Draw(surf,pframe);
  pframe      = plan.ClampFrame(pframe);
//	pHead	= heading;

  // Draw ball todo compute acceleration
  int bframe  = (ball.GetNbFrame() / 2) + (ratio * acc);
  bframe      = ball.ClampFrame(bframe);
  ball.Draw(surf,bframe);    
  overl.Draw(surf);
}


//=================================================================
// CHSIGauge
//=================================================================
CHSIGauge::CHSIGauge (CPanel *mp)
: CBitmapGauge(mp)
{ navs     = 0;
  rang_min = 0;
  rang_max = 360;
  radi_tag = 0;
  radi_unit = 0;
  gsdf = 0;
  radio.flag = VOR_SECTOR_OF;
  radio.xOBS = 0;
  radio.ntyp = SIGNAL_OFF;
  radio.hDEV = 0;
  radio.gDEV = 0;
}
//----------------------------------------------------------------
CHSIGauge::~CHSIGauge(void)
{ 
}
//----------------------------------------------------------------
//	JSDEV* Prepare message
//------------------------------------------------------------------
void CHSIGauge::PrepareMsg(CVehicleObject *veh)
{	veh->FindReceiver(&obsm);
	veh->FindReceiver(&mbug);
	veh->FindReceiver(&fcs);
	CGauge::PrepareMsg(veh);
	return;
}
//-----------------------------------------------------------------
//  Read Gauge tags
//-----------------------------------------------------------------
int CHSIGauge::Read (SStream *stream, Tag tag)
{ Tag apb = 0;
  switch (tag) {
  case 'comp':
    acmp.Read(stream,2);
    return TAG_READ;

  case 'cour':
    acrs.Read(stream,2);
    return TAG_READ;

  case 'devi':
    adev.Read(stream,2);
    return TAG_READ;

  case 'devb':
    advb.Read(stream,2);
    return TAG_READ;

  case 'apbg':
    apbg.Read(stream,2);
    return TAG_READ;

  case 'to__':
    tofg.Read(stream,2);
    return TAG_READ;

  case 'from':
    fmfg.Read(stream,2);
    return TAG_READ;

  case 'glid':
    glid.Read(stream,2);
    return TAG_READ;

  case 'knob':
    obkn.SetGauge(this);
    ReadFrom(&obkn,stream);
    return TAG_READ;

  case 'apkb':
    apkb.SetGauge(this);
    ReadFrom (&apkb, stream);
    return TAG_READ;

  case 'obsm':
    ReadMessage (&obsm, stream);
    return TAG_READ;

  case 'bugm':
    ReadMessage (&mbug, stream);
    return TAG_READ;

  case 'fcs_':
    ReadMessage (&fcs, stream);
    return TAG_READ;

  case 'rang':
    ReadFloat (&rang_min, stream);
    ReadFloat (&rang_max, stream);
    return TAG_READ;

  case 'radi':
    ReadTag (&radi_tag, stream);
    ReadInt (&radi_unit, stream);
    return TAG_READ;

  case 'gsdf':
    ReadFloat (&gsdf, stream);
    return TAG_READ;
  }
  return CBitmapGauge::Read (stream, tag);
}
//---------------------------------------------------------------------
//  All parameters are read
//  TEMPO:  Autopilot bug are sent to GYRO subsystem until autopilot
//          AXIS is implemented
//---------------------------------------------------------------------
void CHSIGauge::ReadFinished()
{ int wd  = 0;
  int ht  = 0;
  CBitmapGauge::ReadFinished();
  //----Send autopilot bug message to the gyro ---------------------
 	mbug.dataType = TYPE_REAL;
  mbug.group    = mesg.group;
  mbug.sender   = unId;
  mbug.user.u.datatag = '_Bug';
  mbug.user.u.hw      = 0;
  mbug.id     = MSG_SETDATA;
  //----------------------------------------------------------------
  obsm.sender = unId;
  fcs.sender  = unId;
  //----Compute max deviation for localizer ------------------------
  adev.GetBitmapSize(&wd,&ht);
  PixDev  = wd * 0.3;                  // Half size as max deviation
  return;
}
//--------------------------------------------------------------------
//  Mouse mouves over
//--------------------------------------------------------------------
ECursorResult CHSIGauge::MouseMoved (int x, int y)
{ if (obkn.MouseMoved(x,y)) {DisplayOBS(odir); return CURSOR_WAS_CHANGED;}
  if (apkb.MouseMoved(x,y)) {DisplayAPB(adir); return CURSOR_WAS_CHANGED;}
  // Call parent method if cursor not changed
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//--------------------------------------------------------------------
//	Mouse click: Start knob rotation if hit
//--------------------------------------------------------------------
EClickResult CHSIGauge::MouseClick (int x, int y, int buttons)
{ if (obkn.IsHit(x,y))
  { obkn.ArmRotation(1,buttons);
    DisplayOBS(odir);
    return MOUSE_TRACKING_ON;
  }
  if (apkb.IsHit(x,y))
	{	apkb.ArmRotation(1,buttons);
		return MOUSE_TRACKING_ON;
	}

	return MOUSE_TRACKING_OFF;
}
//-----------------------------------------------------------------------
//	Mouse up:  stop rotation
//----------------------------------------------------------------------
EClickResult CHSIGauge::StopClick()
{	obkn.DisarmKnob();
  apkb.DisarmKnob();
	return	MOUSE_TRACKING_OFF;
}
//-------------------------------------------------------------------
//  Draw the gauge
//--------------------------------------------------------------------
void CHSIGauge::Draw()
{ EraseSurfaceRGBA(surf,0);
  under.Draw(surf);
  //---- Refresh values from radio through nav subsystem  ----------
  obsm.voidData = &radio;
  obsm.user.u.datatag = 'rfsh';
  obsm.id       = MSG_GETDATA;
  Send_Message(&obsm);
  float ang     = DegToRad(float(radio.xOBS));
  int ILS       = (radio.ntyp == SIGNAL_ILS);
  odir          = radio.xOBS;
  //-----Compute deviation parameters -----------------------------
  int sbly      = SensTAB[ILS];                       // Max degre
  float dev     = (PixDev * radio.hDEV) / sbly ;        // Compute sensibility
  if (dev > +PixDev) dev = +PixDev;
  if (dev < -PixDev) dev = -PixDev;
  //----Compute position parameters -------------------------
  int   px      = int (dev * cos(ang)) + adev.GetX();
  int   py      = int (dev * sin(ang)) + adev.GetY();
	//-----Draw compass ---------------------------------------
  Send_Message(&mesg);                       // Querry the GYRO
  comp = int(mesg.realData);
  acmp.Draw(surf,comp);
  //-----Draw the Course Needle ------------------------------
  acrs.Draw(surf,radio.xOBS);
  //-----Draw Deviation BAR ----------------------------------
  advb.Draw(surf,radio.xOBS);
  //-----Draw CDI --------------------------------------------
  adev.Draw(surf,px,py,radio.xOBS);
  //-----Draw flags ------------------------------------------
  if (radio.flag == VOR_SECTOR_TO) tofg.Draw(surf,radio.xOBS); 
  if (radio.flag == VOR_SECTOR_FR) fmfg.Draw(surf,radio.xOBS);
  //-----Draw OBS Knobs --------------------------------------
  if (obkn.HasChanged())
  { obkn.Redraw();
    obsm.user.u.datatag = 'knob';
    obsm.id             = MSG_SETDATA;
    obsm.realData	      = obkn.GetChange();
    Send_Message (&obsm);
    DisplayOBS(odir);
  }
  //---- Draw Autopilot knob --------------------------------
  mbug.realData   = 0;
  if (apkb.HasChanged())
  { apkb.Redraw();
    mbug.realData = apkb.GetChange();
    DisplayAPB(adir);
  }
  Send_Message(&mbug);
  adir  = mbug.intData;
  //---- Redraw pilot bug ----------------------------------
  apbg.Draw(surf,adir);
	//---Render ----------------------------------------------
	RenderSurface(GetVTAB(),surf);
  RenderSurface(obkn.GetVTAB(),obkn.GetSurface());
  RenderSurface(apkb.GetVTAB(),apkb.GetSurface());
  return;
}
//--------------------------------------------------------------------
//  Display OBS direction
//--------------------------------------------------------------------
void CHSIGauge::DisplayOBS(int d)
{ sprintf_s(hbuf,HELP_SIZE,"OBS: %03d°",d);
  FuiHelp();
  return;
}
//--------------------------------------------------------------------
//  Display autopilot bug
//--------------------------------------------------------------------
void CHSIGauge::DisplayAPB(int d)
{ int  val = Wrap360(d + comp);
  sprintf_s(hbuf,HELP_SIZE,"BUG: %03d°",val);
  FuiHelp();
  return;
}
//===================================================================
// CFlyhawkELTGauge
//===================================================================
CFlyhawkELTGauge::CFlyhawkELTGauge (CPanel *mp)
: CBitmapGauge(mp)
{
  //---- init bitmap ---------------------------------------------
  // Cursor initialization
  csru_tag = csrd_tag = 0;
  //----Init position -----------------------------------
  pos     = 0;                          // ON
  lit     = 1;                          // Alighted
  //---Init increment table -----------------------------
  incT[0] = 1;                          // ON->AUTO
  incT[1] = 2;                          // AUTO->RESET
  incT[2] = 2;                          // RESET->RESET
  //---Init Decrement table -----------------------------
  decT[0] = 0;                          // ON->ON
  decT[1] = 0;                          // AUTO->ON
  decT[2] = 1;                          // RESET->AUTO
  //-----------------------------------------------------
}
//----------------------------------------------------------------
CFlyhawkELTGauge::~CFlyhawkELTGauge (void)
{
}
//----------------------------------------------------------------
//  Read gauge tag
//----------------------------------------------------------------
int CFlyhawkELTGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'ulit':
    ulit.Read(stream,0);
    return TAG_READ;

  case 'lit ':
    litd.Read(stream,0);
    return TAG_READ;

  case 'csru':
    csru_tag = BindCursor(stream);
    return TAG_READ;

  case 'csrd':
    csrd_tag = BindCursor(stream);
    return TAG_READ;

  }
  return CBitmapGauge::Read (stream, tag);
}
//--------------------------------------------------------------------
//  Read gauge tags
//---------------------------------------------------------------------
void CFlyhawkELTGauge::ReadFinished (void)
{
  CBitmapGauge::ReadFinished ();
  mesg.user.u.datatag = 'swit';
}
//----------------------------------------------------------------
//  Draw the gauge
//----------------------------------------------------------------
void CFlyhawkELTGauge::Draw (void)
{ if (lit) litd.Draw(surf, 0, 0, pos);
  else     ulit.Draw(surf, 0, 0, pos);
  return;
}
//------------------------------------------------------------
//  Change position
//------------------------------------------------------------
void CFlyhawkELTGauge::ChangeState(U_CHAR npo)
{ if (npo == pos) return;
  pos = npo;
  globals->snd->Play(sbuf[GAUGE_ON__POS]);
  mesg.id       = MSG_SETDATA;
  mesg.intData  = pos;
  mesg.user.u.datatag = 'swit';
  Send_Message (&mesg);
  lit = mesg.intData;
}
//-------------------------------------------------------------------------
//  Mouse move over. See for cursor change
//--------------------------------------------------------------------------
ECursorResult CFlyhawkELTGauge::MouseMoved (int mouseX, int mouseY)
{ // Upper half of gauge increments the state 
  DisplayHelp();
  if (mouseY < cy) return globals->cum->SetCursor (csru_tag);
  // Lower half of gauge decrements the state  
  if (mouseY > cy) return globals->cum->SetCursor (csrd_tag);
  // Call parent method if necessary
  return CURSOR_WAS_CHANGED;
}
//------------------------------------------------------------------------------
//  Mouse click on gauge
//------------------------------------------------------------------------------
EClickResult CFlyhawkELTGauge::MouseClick (int mouseX, int mouseY, int buttons)
{ if (mouseY < cy) ChangeState(decT[pos]);
  else             ChangeState(incT[pos]);
  return (pos == 2)?(MOUSE_TRACKING_ON):(MOUSE_TRACKING_OFF);
}
//------------------------------------------------------------------------------
//  Mouse stop click: set position to AUTO-OFF
//------------------------------------------------------------------------------
EClickResult CFlyhawkELTGauge::StopClick()
{ globals->snd->Play(sbuf[GAUGE_ON__POS]);
  pos = GAUGE_ON__POS;
  lit = 0;
  return MOUSE_TRACKING_OFF;
}
//===============================================================================
// CSlipIndicatorGauge
//===============================================================================
CSlipIndicatorGauge::CSlipIndicatorGauge (CPanel *mp)
: CBitmapGauge(mp)
{
  strcpy (ball_art, "");
}


int CSlipIndicatorGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'ball':
    ReadString (ball_art, 64, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}



//=====================================================================
// CHobbsMeterGauge
//====================================================================
CHobbsMeterGauge::CHobbsMeterGauge (CPanel *mp)
: CGauge(mp)
{ blak_sizeX = 0, blak_sizeY = 0;
  whit_sizeX = 0, whit_sizeY = 0;
  tmp_hobbs_value = 0;
  n_hobbs_value   = 0;
  hobbs_value_1   = 0;
  hobbs_value_2   = 0;
  hobbs_value_3   = 0;
  hobbs_value_4   = 0;
  hobbs_value_5   = 0.0f;
  one_whit_sizeY  = 0;
}
//-------------------------------------------------------------
CHobbsMeterGauge::~CHobbsMeterGauge (void)
{
}
//-------------------------------------------------------------
//  Read gauge tags
//-------------------------------------------------------------
int CHobbsMeterGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'blak':
    black.Read(stream,0);
    return TAG_READ;

  case 'whit':
    white.Read(stream,0);
    return TAG_READ;
  }
  return CGauge::Read (stream, tag);

}
//---------------------------------------------------------------
//  All tags read
//---------------------------------------------------------------
void CHobbsMeterGauge::ReadFinished (void)
{ mesg.id = MSG_GETDATA;
  mesg.dataType = TYPE_REAL;
  // Get figures width
  black.GetBitmapSize (&blak_sizeX, &blak_sizeY);
  white.GetBitmapSize (&whit_sizeX, &whit_sizeY);
  one_whit_sizeY = whit_sizeY / 10;
  CGauge::ReadFinished ();
}
//-------------------------------------------------------------------
//  REDRAW THIS gauge
//-------------------------------------------------------------------
void CHobbsMeterGauge::Draw (void)
{
  ///  get the hobbs value here
  Send_Message (&mesg);
  //
  n_hobbs_value     = int(mesg.realData);
  hobbs_value_1     = int(n_hobbs_value / 1000); // 4
  tmp_hobbs_value   = hobbs_value_1 * 1000; // 4000
  hobbs_value_2     = int((n_hobbs_value - tmp_hobbs_value) / 100); // 5
  tmp_hobbs_value  += hobbs_value_2 * 100; // 4500 
  hobbs_value_3     = int((n_hobbs_value - tmp_hobbs_value) / 10); // 6;
  tmp_hobbs_value  += hobbs_value_3 * 10; // 4560 
  hobbs_value_4     = n_hobbs_value - tmp_hobbs_value; // 7;
  // inverted bmp position (10- x)
  hobbs_value_5     = 10.0f - ((float(mesg.realData) - float(n_hobbs_value)) * 10.0f); // 8;

  // Draw gauge
  CGauge::Draw ();
  if (surf == NULL)   return;
  black.Draw        (surf, 0             , 0, hobbs_value_1);
  black.Draw        (surf, 1 * blak_sizeX, 0, hobbs_value_2);
  black.Draw        (surf, 2 * blak_sizeX, 0, hobbs_value_3);
  black.Draw        (surf, 3 * blak_sizeX, 0, hobbs_value_4);
  int y1 = (int)(floorf((hobbs_value_5 - 1) * one_whit_sizeY));
  CBitmap *bmp = white.GetBitmap();
  bmp->DrawPartial  (surf, 4 * blak_sizeX, 0,
                                           0,
                                           y1,
                                           whit_sizeX,
                                           y1 + one_whit_sizeY,
                                           0);
  
}
//=======================================================================
// JSDEV* simplified CNavigationGauge
//  All computing done in the subsystem
//========================================================================
CNavigationGauge::CNavigationGauge (CPanel *mp)
: CBitmapGauge(mp)
{
  // Subsystem message
  radi_tag  = 'navi';             // Default radio
  radi_unit = 1;                  // Unit 1
  gflg      = 0;
  vobs      = 0;
  radio     = 0;
  // Compass card needle
  cmp = 0;
  obs = 0;
  // OBS -glide slope & localizer
  gspl_x = gspl_y = 0;
  Flags[0]  = &WarnF;
  Flags[1]  = &TlocF;
  Flags[2]  = &FlocF;
}
//--------------------------------------------------------
CNavigationGauge::~CNavigationGauge (void)
{}
//-------------------------------------------------------
//	Prepare message
//-------------------------------------------------------
void CNavigationGauge::PrepareMsg(CVehicleObject *veh)
{	fcs.id		= MSG_GETDATA;
	veh->FindReceiver(&fcs);
  veh->FindReceiver(&mobs);
  veh->FindReceiver(&mnav);
  veh->FindReceiver(&mrad);
	CGauge::PrepareMsg(veh);
	return;
}
//------------------------------------------------------
//	Read Tags
//-------------------------------------------------------
int CNavigationGauge::Read (SStream *stream, Tag tag)
{ short u = 0;
  short v = 0;
  int rc = TAG_IGNORED;
  Tag nav = 0;
  switch (tag) {
  case 'comp':
		ReadTag (&cmp, stream);
		ReadFrom (&comp_ndl, stream);
		return TAG_READ;

  case 'lndl':
		lndl.Read(stream,2);
		locFramesPerDeg = (lndl.GetNbFrame() / 20);
		return TAG_READ;

  case 'gndl':
		gndl.Read(stream,2);
    gldFramesPerDeg = (gndl.GetNbFrame() / 20);
		return TAG_READ;

  case 'obs_':
    obkn.ReadParameters(this,stream);
    ReadTag(&obs,stream);
		return TAG_READ;

  case 'wtab':
		WarnF.Read(stream,0);
		return TAG_READ;

  case 'ttab':
	  TlocF.Read(stream,0);
		return TAG_READ;

  case 'ftab':
	  FlocF.Read(stream,0);
		return TAG_READ;
  case 'ntpl':
		ReadShort (&u, stream);
		ReadShort (&v, stream);
    WarnF.SetX(u);
    WarnF.SetY(v);
		TlocF.SetX(u);
		TlocF.SetY(v);
		FlocF.SetX(u);
		FlocF.SetY(v);
		return TAG_READ;

  case 'gspl':
		ReadInt (&gspl_x, stream);
		ReadInt (&gspl_y, stream);
    gflg  = 1;
		return TAG_READ;

  case 'radi':
		ReadTag (&radi_tag, stream);
		ReadInt (&radi_unit, stream);
		return TAG_READ;

  case 'fcs_':
		ReadMessage (&fcs, stream);
		return TAG_READ;
  }
  return CBitmapGauge::Read (stream, tag);
 }
//----------------------------------------------------------
void CNavigationGauge::ReadFinished ()
{
  CBitmapGauge::ReadFinished ();

  // Initialize message attributes
  mesg.group		      = radi_tag;
  mesg.sender         = unId;
  mesg.user.u.unit	  = radi_unit;
  mesg.user.u.hw      = HW_RADIO;
 	mesg.id             = MSG_GETDATA;
  mesg.dataType       = TYPE_REAL;
  //----OBS message -----------------
  mobs.group          = radi_tag;
  mobs.sender         = unId;
  mobs.user.u.unit    = radi_unit;
  mobs.user.u.hw      = HW_RADIO;
  mobs.id             = MSG_SETDATA;
  mobs.user.u.datatag = 'obs_';                       
  mobs.dataType       = TYPE_REAL; 
  //---NAV message ------------------
  mnav.sender         = unId;
  mnav.dataType       = TYPE_VOID;
  mnav.user.u.hw      = HW_GAUGE;
  mnav.id             = MSG_SETDATA;
  mnav.user.u.datatag = 'setr';
  //----Radio message ---------------
  mrad.sender         = unId;
  mrad.dataType       = TYPE_VOID;
  mrad.user.u.hw      = HW_RADIO;
  mrad.id             = MSG_GETDATA;
  mrad.user.u.datatag = 'getr';
  mrad.group          = radi_tag;
  mrad.user.u.unit    = radi_unit;
  //---------------------------------
  fcs.sender          = unId;
}
//-------------------------------------------------------------------
//  Get radio block 
//-------------------------------------------------------------------
void CNavigationGauge::GetRadio()
{ Send_Message(&mrad);
  radio     = (BUS_RADIO*)mrad.voidData;
  if (0 == radio)   return;
  //--- Set radio block in nav gauge if OBS is specified ----
  if (0 == obs)     return;
  mnav.group    = obs;
  mnav.voidData = radio;
  Send_Message(&mnav);       // Set radio interface
  return;
}
//-------------------------------------------------------------------
//	Knob click => Arm rotation
//-------------------------------------------------------------------
EClickResult CNavigationGauge::MouseClick (int x, int y, int buttons)
{	if (obkn.IsHit(x,y))	{	obkn.ArmRotation(1,buttons);return MOUSE_TRACKING_ON;	}
	return MOUSE_TRACKING_OFF;
}
//----------------------------------------------------------------
//	Stop click => disarm rotation
//-----------------------------------------------------------------
EClickResult CNavigationGauge::StopClick()
{	obkn.DisarmKnob();
	return MOUSE_TRACKING_OFF;
}
//-------------------------------------------------------------------
//	Mouse move over
//-------------------------------------------------------------------
ECursorResult CNavigationGauge::MouseMoved (int x, int y)
{	if (obkn.IsHit(x,y))  return CURSOR_WAS_CHANGED;
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//-------------------------------------------------------------------
//	Draw note over knob
//-------------------------------------------------------------------
void CNavigationGauge::DisplayHelp()
{ sprintf_s(hbuf,HELP_SIZE,"OBS (%s) %03d°",flagTAB[xflg],vobs);
  FuiHelp();
  return;
}
//-----------------------------------------------------------------------------
//	JSDEV* Draw the gauge
//	NOTE:	  This gauge has 2 modes:  NAV and ILS
//          When in NAV mode, the obs is taken in account for the 
//          needle deviation.  When in ILS mode, the ILS direction is
//          driving the deviation.  The correct setting is maintain in 
//          the associated radio.
//
//	  1		  Vertical Dead zone is related to vertical aircraft position above the 
//			    it is a cone defined by a radius of 7° at the base of the station
//			    It should be computed in the Navaid station
//			    Horizontal dead zone occurs when aircraft radial and OBS are about +- 90°
//	  2		  if dta is = (navd - obs) mod(360) with navd = radial crossing the aircraft
//			    then dta in [90, 270[ => TO flag
//				  dta in [270, 90[ => FROM flag
//	  3		  if	dta is in [0, 180[	then the localizer needle is on left side
//				  negative frames are used.
//				  when dta is in [-180,0[	then the localizer needle is on right side
//				  positive frames are used.
//			    for dta in [90, 270[ dta is first adjusted.
//    
//----------------------------------------------------------------------------------
void CNavigationGauge::Draw (void)
{	if (0 == radio) GetRadio();
  EraseSurfaceRGBA(surf,0);
  under.Draw(surf);
  if (0 == radio) return;
	//---------Get OBS or ILS direction (depending on tuned station)  ------------
  float dev       = radio->hDEV;                              // Deviation in deg
  xflg            = radio->flag;
  CAniMAP *flag   = Flags[xflg];                              // Localizer flag
  //-------  Get ILS GLide slope if ILS tuned ---------------------------------
  float glid = radio->gDEV;                                   // Glide slope
  ils = (radio->ntyp == SIGNAL_ILS);                             
  //---------  Compute station (VOR or ILS) sensibility -----------------------
  int sbly            = SensTAB[ils];
  locFramesPerDeg     = (lndl.GetNbFrame() / sbly);         // Compute sensibility
	//--------- Draw compass card according to knob rotation -----------------------
  vobs = radio->xOBS;
  comp_ndl.Draw (surf, vobs, cx, cy);
	//---- Draw localizer flags ------------------------------------------------ ---
	flag->Draw(surf,0);
  //----- Draw Glide slope flag (if ILS then same flag as localizer flag) --------
  if (0 == ils) flag = &WarnF;                              // No ILS, no glide flag
  if (gflg)  flag->Draw(surf,gspl_x,gspl_y,0); 
	//----------- Draw localizer needle --------------------------------------------
	int frame = (lndl.GetNbFrame() >> 1);
	if (xflg) {
	    frame  += (int)(dev * locFramesPerDeg);
      frame   = lndl.ClampFrame(frame);
    }
	lndl.Draw(surf,frame);
  //-------Draw glide slope needle up to +- 5° around  -----------------
	frame = (gndl.GetNbFrame() >> 1);
  frame    -= (int)((frame * glid) / TANGENT_5DEG);
  frame     = gndl.ClampFrame(frame);
	gndl.Draw(surf,frame);
	//------Redraw  overlay -----------------------------------
	overl.Draw(surf);
	//----- Draw OBS knob if rotated, one rotation per 30 degrees---------
  if (obkn.HasChanged())	
  {   obkn.Redraw(); 
      mobs.realData = obkn.GetChange();             
	    Send_Message(&mobs);
      DisplayHelp();
  }
	//--- Render ---------------------------------------------
	RenderSurface(GetVTAB(),surf);
  RenderSurface(obkn.GetVTAB(),obkn.GetSurface());
	return;
}
//===================================================================
// CBKKFC200Gauge
//===================================================================
CBKKFC200Gauge::CBKKFC200Gauge (CPanel *mp)
: CBitmapGauge(mp)
{ 
}
//------------------------------------------------------------------
CBKKFC200Gauge::~CBKKFC200Gauge(void)
{ 
}
//------------------------------------------------------------------
//  Read gauge tags
//------------------------------------------------------------------
int CBKKFC200Gauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'ca01':
    ReadFrom (&ca01, stream);
    return TAG_READ;

  case 'ca02':
    ReadFrom (&ca02, stream);
    return TAG_READ;

  case 'ca03':
    ReadFrom (&ca03, stream);
    return TAG_READ;

  case 'ca04':
    ReadFrom (&ca04, stream);
    return TAG_READ;

  case 'ca05':
    ReadFrom (&ca05, stream);
    return TAG_READ;

  case 'ca06':
    ReadFrom (&ca06, stream);
    return TAG_READ;

  case 'ca07':
    ReadFrom (&ca07, stream);
    return TAG_READ;

  case 'ca08':
    ReadFrom (&ca08, stream);
    return TAG_READ;

  case 'ca09':
    ReadFrom (&ca09, stream);
    return TAG_READ;

  case 'ca10':
    ReadFrom (&ca10, stream);
    return TAG_READ;

  case 'ca11':
    ReadFrom (&ca11, stream);
    return TAG_READ;

  case 'fd__':
    arfd.Read(stream,2);
    return TAG_READ;

  case 'alt_':
    aalt.Read(stream,2);
    return TAG_READ;

  case 'hdg_':
    ahdg.Read(stream,2);
    return TAG_READ;

  case 'gs__':
    glid.Read(stream,2);
    return TAG_READ;

  case 'nav_':
    anav.Read(stream,2);
    return TAG_READ;

  case 'apr_':
    aapr.Read(stream,2);
    return TAG_READ;

  case 'bc__':
    arbc.Read(stream,2);
    return TAG_READ;

  case 'trim':
    trim.Read(stream,2);
    return TAG_READ;

  case 'ap__':
    arap.Read(stream,2);
    return TAG_READ;
  }
  return CBitmapGauge::Read (stream, tag);
}

//------------------------------------------------------------
ECursorResult CBKKFC200Gauge::MouseMoved (int x, int y)
{
  ECursorResult rc = CURSOR_NOT_CHANGED;

  // Send updated mouse position to all click areas
  rc = ca01.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca02.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca03.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca04.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca05.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca06.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca07.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca08.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca09.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca10.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca11.MouseMoved (x, y);

  // Call parent method if cursor not changed
  if (rc == CURSOR_NOT_CHANGED) rc = CBitmapGauge::MouseMoved (x, y);
  return rc;
}


//=============================================================
// CBKKFC150Gauge
//=============================================================
CBKKFC150Gauge::CBKKFC150Gauge (CPanel *mp)
: CBitmapGauge(mp)
{
}
CBKKFC150Gauge::~CBKKFC150Gauge(void)
{ 
}
//--------------------------------------------------------------
int CBKKFC150Gauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'ca01':
    ReadFrom (&ca01, stream);
    return TAG_READ;

  case 'ca02':
    ReadFrom (&ca02, stream);
    return TAG_READ;

  case 'ca03':
    ReadFrom (&ca03, stream);
    return TAG_READ;

  case 'ca04':
    ReadFrom (&ca04, stream);
    return TAG_READ;

  case 'ca05':
    ReadFrom (&ca05, stream);
    return TAG_READ;

  case 'ca06':
    ReadFrom (&ca06, stream);
    return TAG_READ;

  case 'ca07':
    ReadFrom (&ca07, stream);
    return TAG_READ;

  case 'ca08':
    ReadFrom (&ca08, stream);
    return TAG_READ;

  case 'ca09':
    ReadFrom (&ca09, stream);
    return TAG_READ;

  case 'ca10':
    ReadFrom (&ca10, stream);
    return TAG_READ;

  case 'ca11':
    ReadFrom (&ca11, stream);
    return TAG_READ;

  case 'fd__':
    arfd.Read(stream,2);
    return TAG_READ;

  case 'alt_':
    aalt.Read(stream,2);
    return TAG_READ;

  case 'hdg_':
    ahdg.Read(stream,2);
    return TAG_READ;

  case 'gs__':
    glid.Read(stream,2);
    return TAG_READ;

  case 'nav_':
    anav.Read(stream,2);
    return TAG_READ;

  case 'apr_':
    aapr.Read(stream,2);
    return TAG_READ;

  case 'bc__':
    arbc.Read(stream,2);
    return TAG_READ;

  case 'trim':
    trim.Read(stream,2);
    return TAG_READ;

  case 'ap__':
    arap.Read(stream,2);
    return TAG_READ;
  }
  return CBitmapGauge::Read (stream, tag);
}

ECursorResult CBKKFC150Gauge::MouseMoved (int x, int y)
{
  ECursorResult rc = CURSOR_NOT_CHANGED;

  // Send updated mouse position to all click areas
  rc = ca01.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca02.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca03.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca04.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca05.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca06.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca07.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca08.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca09.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca10.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca11.MouseMoved (x, y);

  // Call parent method if cursor not changed
  if (rc == CURSOR_NOT_CHANGED) rc = CBitmapGauge::MouseMoved (x, y);
  return rc;
}


//==========================================================================
// CBKAudioKMA26Gauge
//===========================================================================
CBKAudioKMA26Gauge::CBKAudioKMA26Gauge (CPanel *mp)
: CBitmapGauge(mp)
{
  radi = 0;
  mesg.id     = MSG_GETDATA;
  mesg.group  = 'mark';
}
//-----------------------------------------------------------------
//  read all parameters
//-----------------------------------------------------------------
int CBKAudioKMA26Gauge::Read (SStream *stream, Tag tag)
{ int ica = 0;
  int typ = ClickAreaFromTag (tag,&ica);
  if ((ica >= 1) && (ica < KMA26_CA_SZ)) {
    // Valid click area
    ReadFrom (&ca[ica], stream);
    ca[ica].Type = typ;
    return TAG_READ;
  }

  switch (tag) {
    case 'radi':
      ReadTag (&radi, stream);
      ReadInt (&unit, stream);
      return TAG_READ;
  }

  return CBitmapGauge::Read (stream, tag);
}
//-----------------------------------------------------------------
//  Final initialization
//-----------------------------------------------------------------
void CBKAudioKMA26Gauge::PrepareMsg(CVehicleObject *veh)
{ 
  return;
}
//-----------------------------------------------------------------
//  Draw the gauge
//-----------------------------------------------------------------
void CBKAudioKMA26Gauge::Draw()
{ //---Draw outter marker -----------------
  mesg.user.u.datatag = 'outr';
  Send_Message(&mesg);
 // ca[KMA_CA_OUTM].Draw(surf,mesg.intData);
  ca[KMA_CA_OUTM].Draw(mesg.intData);
  mesg.user.u.datatag = 'midl';
  Send_Message(&mesg);
//  ca[KMA_CA_MIDL].Draw(surf,mesg.intData);
  ca[KMA_CA_MIDL].Draw(mesg.intData);
  mesg.user.u.datatag = 'innr';
  Send_Message(&mesg);
//  ca[KMA_CA_INNR].Draw(surf,mesg.intData);
  ca[KMA_CA_INNR].Draw(mesg.intData);
  return;
}
//-----------------------------------------------------------------
//  Mouse move
//-----------------------------------------------------------------
ECursorResult CBKAudioKMA26Gauge::MouseMoved (int mx, int my)
{
  ECursorResult rc = CURSOR_NOT_CHANGED;
  for (int i = 1; (i< KMA26_CA_SZ); i++)
  { if ( ca[i].MouseMoved(mx,my))  return CURSOR_WAS_CHANGED;
  }
  return CURSOR_NOT_CHANGED;
}
//=====================================================================
// CGenericNavRadioGauge
//=====================================================================
CGenericNavRadioGauge::CGenericNavRadioGauge (CPanel *mp)
: CBitmapGauge(mp)
{ RAD       = 0;
  radi_tag  = 0;
  radi_unit = 0;
  sdim      = false;
  amber     = MakeRGB (215, 90, 0);
  radi9     = (CVariFontBMP*)globals->fonts.ftradi9.font;

}
//-------------------------------------------------------------
//  Read parameters
//-------------------------------------------------------------
int CGenericNavRadioGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'radi':
    ReadTag (&radi_tag, stream);
    ReadInt (&radi_unit, stream);
    rc = TAG_READ;
    break;

  case 'ca01':
    ReadFrom (&ck[1], stream);
    rc = TAG_READ;
    break;

  case 'ca02':
    ReadFrom (&ck[2], stream);
    rc = TAG_READ;
    break;

  case 'ca03':
    ReadFrom (&ck[3], stream);
    rc = TAG_READ;
    break;

  case 'ca04':
    ReadFrom (&ck[4], stream);
    rc = TAG_READ;
    break;

  case 'ca05':
    ReadFrom (&ck[5], stream);
    rc = TAG_READ;
    break;

  case 'ca06':
    ReadFrom (&ck[6], stream);
    rc = TAG_READ;
    break;

  case 'sdim':
    sdim = true;
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}
//-----------------------------------------------------------------------------
//  All parameters are read
//-----------------------------------------------------------------------------
void CGenericNavRadioGauge::ReadFinished()
{ CBitmapGauge::ReadFinished ();
  //----Define display window ------------------------------
  dsw   = surf->xSize;
  dsh   = surf->ySize;
  xOrg  = 0;
  yOrg  = 0;     // Bottom line
  yOfs  = yOrg * surf->xSize;
  return;
}
//-----------------------------------------------------------------------------
//  Get the subsystem radio
//-----------------------------------------------------------------------------
void CGenericNavRadioGauge::GetRadio()
{ //----Get Radio ----------------------------
  mesg.id             = MSG_GETDATA;
  mesg.group          = radi_tag;
  mesg.user.u.unit    = radi_unit;
  mesg.user.u.hw      = HW_RADIO;
  mesg.user.u.datatag = 'gets';
  Send_Message(&mesg);
  RAD = (CNavRadio*) mesg.voidData;
  return;
}
//------------------------------------------------------------------
//  Check for NAV event
//------------------------------------------------------------------
int CGenericNavRadioGauge::AnyEvent(int mx,int my)
{ RADIO_HIT *itm = RAD->GetHIT();
  while (itm->No) 
  { if (!ck[itm->No].IsHit(mx,my)) {itm++; continue; }
    RAD->Dispatcher(itm->Ev);
    return 1;
  }
  return 0;
}
//------------------------------------------------------------------
//  Draw Field
//------------------------------------------------------------------
void CGenericNavRadioGauge::DrawField(RADIO_FLD *fd)
{ U_CHAR mask = RAD->GetFlashMask();
  if (0 == fd->state) return;
  if ((RAD_ATT_FLASH == fd->state) && (0 == mask))  return;
  short No  = fd->sPos;
  short x0  = ck[No].x1;
  short y0  = ck[No].y1;
  radi9->DrawNText(surf,x0,y0,amber,fd->data);
  return;
}
//------------------------------------------------------------------
//  Draw the gauge
//------------------------------------------------------------------
void CGenericNavRadioGauge::Draw()
{ if (0 == RAD) GetRadio();
  if (0 == RAD) return;
  ClearDisplay();
  if (0 == RAD->GetNavState())  return;
  for (short No = RADIO_FD_ANAV_WP; No < RADIO_FD_NUMBER; No++)  DrawField(RAD->GetField(No));
  return;
}
//------------------------------------------------------------------
//  Mouse Click
//------------------------------------------------------------------
EClickResult CGenericNavRadioGauge::MouseClick (int x, int y, int buttons)
{ if (0 == RAD)           return MOUSE_TRACKING_OFF;
  K55_EVENT Event = K55EV_NULL;
  int dir   = (MOUSE_BUTTON_LEFT == buttons)?(-1):(+1);
  RAD->SetDirection(dir); 
  if (AnyEvent(x,y))      return MOUSE_TRACKING_ON;
  return MOUSE_TRACKING_OFF;
}
//------------------------------------------------------------------
//  Mouse move over
//------------------------------------------------------------------
ECursorResult CGenericNavRadioGauge::MouseMoved (int x, int y)
{ return CURSOR_WAS_CHANGED;
}
//===========================================================================
// CGenericComRadioGauge
//===========================================================================
CGenericComRadioGauge::CGenericComRadioGauge (CPanel *mp)
: CBitmapGauge(mp)
{ RAD       = 0;
  radi_tag  = 0;
  radi_unit = 0;
  sdim      = false;
  amber     = MakeRGB (215, 90, 0);
  radi9     = (CVariFontBMP*)globals->fonts.ftradi9.font;
}

//--------------------------------------------------------------
//  Read all parameters
//--------------------------------------------------------------
int CGenericComRadioGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;
  int ica = 0;
  int typ = ClickAreaFromTag (tag,&ica);
  if ((ica >= 1) && (ica < 7)) {
    // Valid click area
    ReadFrom (&ck[ica], stream);
    ck[ica].Type = typ;
    return TAG_READ;
  }
  //---Check for other tags -------------------
  switch (tag) {
  case 'radi':
    ReadTag (&radi_tag, stream);
    ReadInt (&radi_unit, stream);
    return TAG_READ;
  case 'sdim':
    sdim = true;
    return TAG_READ;
  }

  return CBitmapGauge::Read (stream, tag);
}
//-----------------------------------------------------------------------------
//  All parameters are read
//-----------------------------------------------------------------------------
void CGenericComRadioGauge::ReadFinished()
{ CBitmapGauge::ReadFinished ();
  //----Define display window ------------------------------
  dsw   = surf->xSize;
  dsh   = surf->ySize;
  xOrg  = 0;
  yOrg  = 0;     // Bottom line
  yOfs  = yOrg * surf->xSize;
  return;
}
//-----------------------------------------------------------------------------
//  Get the subsystem radio
//-----------------------------------------------------------------------------
void CGenericComRadioGauge::GetRadio()
{ //----Get Radio ----------------------------
  mesg.id             = MSG_GETDATA;
  mesg.group          = radi_tag;
  mesg.user.u.unit    = radi_unit;
  mesg.user.u.hw      = HW_RADIO;
  mesg.user.u.datatag = 'gets';
  Send_Message(&mesg);
  RAD = (CComRadio*) mesg.voidData;
  return;
}

//------------------------------------------------------------------
//  Check for COM event
//------------------------------------------------------------------
int CGenericComRadioGauge::AnyEvent(int mx,int my)
{ RADIO_HIT *itm = RAD->GetHIT();
  while (itm->No) 
  { if (!ck[itm->No].IsHit(mx,my)) {itm++; continue; }
    RAD->Dispatcher(itm->Ev);
    return 1;
  }
  return 0;
}
//------------------------------------------------------------------
//  Draw Field
//------------------------------------------------------------------
void CGenericComRadioGauge::DrawField(RADIO_FLD *fd)
{ U_CHAR mask = RAD->GetFlashMask();
  if (0 == fd->state) return;
  if ((RAD_ATT_FLASH == fd->state) && (0 == mask))  return;
  short No  = fd->sPos;
  short x0  = ck[No].x1;
  short y0  = ck[No].y1;
  radi9->DrawNText(surf,x0,y0,amber,fd->data);
  return;
}
//------------------------------------------------------------------
//  Draw the gauge
//------------------------------------------------------------------
void CGenericComRadioGauge::Draw()
{ if (0 == RAD) GetRadio();
  if (0 == RAD) return;
  ClearDisplay();
  if (0 == RAD->GetComState())  return;
  for (short nf = 1; nf <= RADIO_FD_NUMBER; nf++)  
  { if (ck[nf].NoShow())        continue;
    DrawField(RAD->GetField(nf));
  }
  return;
}
//------------------------------------------------------------------
//  Mouse Click
//------------------------------------------------------------------
EClickResult CGenericComRadioGauge::MouseClick (int x, int y, int buttons)
{ if (0 == RAD)           return MOUSE_TRACKING_OFF;
  K55_EVENT Event = K55EV_NULL;
  int dir   = (MOUSE_BUTTON_LEFT == buttons)?(-1):(+1);
  RAD->SetDirection(dir); 
  if (AnyEvent(x,y))      return MOUSE_TRACKING_ON;
  return MOUSE_TRACKING_OFF;
}
//----------------------------------------------------------------------------
//  Mouse move over
//----------------------------------------------------------------------------
ECursorResult CGenericComRadioGauge::MouseMoved (int x, int y)
{  return CURSOR_WAS_CHANGED;
}
//================================================================================
// CGenericTransponderGauge
//================================================================================
CGenericTransponderGauge::CGenericTransponderGauge (CPanel *mp)
: CBitmapGauge(mp)
{ RAD       = 0;
  radi_tag  = 0;
  radi_unit = 0;
  amber     = MakeRGB (215, 90, 0);
  radi9     = (CVariFontBMP*)globals->fonts.ftradi9.font;
  frame     = 0;
}
//------------------------------------------------------------------
//  Read all parameters
//------------------------------------------------------------------
int CGenericTransponderGauge::Read (SStream *stream, Tag tag)
{
  switch (tag) {
  case 'radi':
    ReadTag (&radi_tag, stream);
    ReadInt (&radi_unit, stream);
    return TAG_READ;
  //--- Read active ident ---------------------
  case 'ida1':
    ReadFrom (&ck[1], stream);
    return TAG_READ;
  case 'ida2':
    ReadFrom (&ck[2], stream);
    return TAG_READ;
  case 'ida3':
    ReadFrom (&ck[3], stream);
    return TAG_READ;
  case 'ida4':
    ReadFrom (&ck[4], stream);
    return TAG_READ;
  //----- Standby ident ----------------------
  case 'idb1':
    ReadFrom (&ck[5], stream);
    return TAG_READ;
  case 'idb2':
    ReadFrom (&ck[6], stream);
    return TAG_READ;
  case 'idb3':
    ReadFrom (&ck[7], stream);
    return TAG_READ;
  case 'idb4':
    ReadFrom (&ck[8], stream);
    return TAG_READ;
  //------------------------------------------
  case 'mode':
    ReadFrom (&ck[9], stream);
    return TAG_READ;

  case 'idnt':
    ReadFrom (&ck[10], stream);
    return TAG_READ;

  case 'tune':
    ReadFrom (&ck[11], stream);
    return TAG_READ;

  case 'test':
    ReadFrom (&ck[12], stream);
    return TAG_READ;
 }

    return CBitmapGauge::Read (stream, tag);

}

//-----------------------------------------------------------------------------
//  All parameters are read
//-----------------------------------------------------------------------------
void CGenericTransponderGauge::ReadFinished()
{ CBitmapGauge::ReadFinished ();
  //----Define display window ------------------------------
  dsw   = surf->xSize;
  dsh   = surf->ySize;
  xOrg  = 0;
  yOrg  = 0;     // Bottom line
  yOfs  = yOrg * surf->xSize;
  return;
}
//-----------------------------------------------------------------------------
//  Get the subsystem radio
//-----------------------------------------------------------------------------
void CGenericTransponderGauge::GetRadio()
{ //----Get Radio ----------------------------
  mesg.id             = MSG_GETDATA;
  mesg.group          = radi_tag;
  mesg.user.u.unit    = radi_unit;
  mesg.user.u.hw      = HW_RADIO;
  mesg.user.u.datatag = 'gets';
  Send_Message(&mesg);
  RAD                 = (CTransponderRadio*) mesg.voidData;
  return;
}
//------------------------------------------------------------------
//  Check for click event
//------------------------------------------------------------------
int CGenericTransponderGauge::AnyEvent(int mx,int my)
{ RADIO_HIT *itm = RAD->GetHIT();
  while (itm->No) 
  { if (!ck[itm->No].IsHit(mx,my)) {itm++; continue; }
    RAD->Dispatcher(itm->Ev);
    return 1;
  }
  return 0;
}
//------------------------------------------------------------------
//  Draw Field
//------------------------------------------------------------------
void CGenericTransponderGauge::DrawField(RADIO_FLD *fd)
{ if (0 == fd->state) return;
  if ((RAD_ATT_FLASH == fd->state) && (0 == mask))  return;
  short No  = fd->sPos;
  short x0  = ck[No].x1;
  short y0  = ck[No].y1;
  radi9->DrawChar(surf,x0,y0,*fd->data,amber);
  return;
}
//------------------------------------------------------------------
//  Draw the gauge
//  Display only active and standby Ident digits
//------------------------------------------------------------------
void CGenericTransponderGauge::Draw()
{ //---Draw button ------------------------
  ClearDisplay();
  ck[RADIO_CA09].DrawOnSurface (surf, frame);            // Mode
  if (0 == RAD) GetRadio();
  if (0 == RAD) return;
  frame         = RAD->GetFrame();
  U_CHAR state  = RAD->GetXpdState();
  if (XPDR_OFF == state)    return;
  for (short No = 1; No <= 8; No++)  
  { if (ck[No].NoShow())    continue;
    DrawField(RAD->GetField(No));
  }
  return;
}
//------------------------------------------------------------------
//  Mouse Click
//------------------------------------------------------------------
EClickResult CGenericTransponderGauge::MouseClick (int x, int y, int buttons)
{ if (0 == RAD)        return MOUSE_TRACKING_OFF;
  RAD->mDir = (MOUSE_BUTTON_LEFT == buttons)?(-1):(+1);
  if (AnyEvent(x,y))   return MOUSE_TRACKING_ON;
  return MOUSE_TRACKING_OFF;
}
//------------------------------------------------------------------
ECursorResult CGenericTransponderGauge::MouseMoved (int x, int y)
{ return CURSOR_WAS_CHANGED;  }
//========================================================================
//  INPUT DRIVER for GENERIC ADF GAUGE
//========================================================================
//  At power off
//-----------------------------------------------------------------------
RADIO_HIT KADF_p0[] = {
  {RADIO_CA14,K87EV_POWR},              // Power ON
  {         0,0},
};
//-----------------------------------------------------------------------
//  Frequency mode
//-----------------------------------------------------------------------
RADIO_HIT KADF_fq[] = {
  {RADIO_CA14,K87EV_POWR},              // Power OFF
  {RADIO_CA01,K87EV_ACT1},
  {RADIO_CA02,K87EV_ACT2},
  {RADIO_CA03,K87EV_ACT3},
  {RADIO_CA04,K87EV_ACT4},
  {RADIO_CA05,K87EV_ACT5},
  {RADIO_CA06,K87EV_SBY1},
  {RADIO_CA07,K87EV_SBY2},
  {RADIO_CA08,K87EV_SBY3},
  {RADIO_CA09,K87EV_SBY4},
  {RADIO_CA10,K87EV_SBY5},
  {RADIO_CA11,K87EV_INNR},
  {RADIO_CA12,K87EV_BFRQ},
  {0,0},
};
//-----------------------------------------------------------------------
RADIO_HIT *CGenericADFRadioGauge::RadfHIT[] = {
  KADF_p0,                              // Power off
  KADF_fq,                              // Frequency mode
};
//============================================================================
//  Fields to draw
//============================================================================
RADIO_DSP  CGenericADFRadioGauge::RadfFLD[] = {
  {KR87_ACTF_D1,RADIO_CA01},                          // Active frequency Digit 1
  {KR87_ACTF_D2,RADIO_CA02},                          // Active frequency Digit 2
  {KR87_ACTF_D3,RADIO_CA03},                          // Active frequency Digit 3
  {KR87_ACTF_D4,RADIO_CA04},                          // Active frequency Digit 4
  {KR87_ACTF_D5,RADIO_CA05},                          // Active frequency Digit 5
  {KR87_SBYF_D1,RADIO_CA06},                          // Standby frequency Digit 1
  {KR87_SBYF_D2,RADIO_CA07},                          // Standby frequency Digit 2
  {KR87_SBYF_D3,RADIO_CA08},                          // Standby frequency Digit 3
  {KR87_SBYF_D4,RADIO_CA09},                          // Standby frequency Digit 4
  {KR87_SBYF_D5,RADIO_CA10},                          // Standby frequency Digit 5
  {KR87_NULL_FD,},                         // Last filed
};

//========================================================================
// CGenericADFRadioGauge
//========================================================================
CGenericADFRadioGauge::CGenericADFRadioGauge (CPanel *mp)
: CBitmapGauge(mp)
{ sdim  = 0;
  RAD   = 0;
  PowST = 0;
  bPOW  = 0;
  //----------------------------------------------------------------
  amber = MakeRGB (215, 90, 0);
  radi9 = (CVariFontBMP*)globals->fonts.ftradi9.font;
  hCar  = radi9->CharHeight('H');
  wCar  = radi9->CharWidth (' ');
}
//----------------------------------------------------------------
//  Read Parameters 
//  There may be up to 14 click areas for this  radio.
//  Any area is optional. The tag <clko> indicate a 
//  click only, no display area
//  5 digits (dga1-dga5) for frequency A (active)
//  5 digits (dgb1-dgb5) for frequency B (standby)
//  1 button 'tune'
//  1 button 'store'
//  1 button 'test'
//  1 button 'mode'
//----------------------------------------------------------------
int CGenericADFRadioGauge::Read (SStream *stream, Tag tag)
{ Tag rad;
  int unit;
  switch (tag) {
  case 'radi':
    ReadTag (&rad, stream);
    ReadInt (&unit,stream);
    mesg.group          = rad;
    mesg.user.u.hw      = HW_RADIO;
    mesg.user.u.unit    = unit;
    return TAG_READ;

  case 'dga1':
    ReadFrom (&ck[1], stream);
    return TAG_READ;

  case 'dga2':
    ReadFrom (&ck[2], stream);
    return TAG_READ;

  case 'dga3':
    ReadFrom (&ck[3], stream);
    return TAG_READ;

  case 'dga4':
    ReadFrom (&ck[4], stream);
    return TAG_READ;

  case 'dga5':
    ReadFrom (&ck[5], stream);
    return TAG_READ;

  case 'dgb1':
    ReadFrom (&ck[6], stream);
    return TAG_READ;

  case 'dgb2':
    ReadFrom (&ck[7], stream);
    return TAG_READ;

  case 'dgb3':
    ReadFrom (&ck[8], stream);
    return TAG_READ;

  case 'dgb4':
    ReadFrom (&ck[9], stream);
    return TAG_READ;

  case 'dgb5':
    ReadFrom (&ck[10], stream);
    return TAG_READ;

  case 'tune':
    ReadFrom (&ck[11], stream);
    return TAG_READ;

  case 'stor':
    ReadFrom (&ck[12], stream);
    return TAG_READ;

  case 'test':
    ReadFrom (&ck[13], stream);
    return TAG_READ;

  case 'mode':
    bPOW  = 1;
    ReadFrom (&ck[14], stream);
    return TAG_READ;

  case 'sdim':
    sdim = 1;
    return TAG_READ;
  }

  return CBitmapGauge::Read (stream, tag);
}
//--------------------------------------------------------------------------
//  All parameters are read
//--------------------------------------------------------------------------
void CGenericADFRadioGauge::ReadFinished()
{ CBitmapGauge::ReadFinished ();
  mesg.id = MSG_GETDATA;
  //----Define display window ------------------------------
  dsw   = surf->xSize;
  dsh   = surf->ySize;
  xOrg  = 0;
  yOrg  = 0;     // Bottom line
  yOfs  = yOrg * surf->xSize;
  return;
}
//-----------------------------------------------------------------------------
//  Get the subsystem RADIO
//-----------------------------------------------------------------------------
void CGenericADFRadioGauge::GetRADIO()
{ //----Get ADF RADIO ----------------------------
  mesg.user.u.datatag = 'gets';
  Send_Message(&mesg);
  RAD = (CKR87radio*) mesg.voidData;
  //----Set power if no power button -------------
  if (0 == bPOW)  RAD->SetAutopower();
  return;
}
//--------------------------------------------------------------------------
//  Generate mouse events
//--------------------------------------------------------------------------
int CGenericADFRadioGauge::AnyEvent(int mx,int my)
{ char       sta  = RAD->GetRState();
  RADIO_HIT *itm  = RadfHIT[sta];
  while (itm->No) 
  { if (!ck[itm->No].IsHit(mx,my)) {itm++; continue; }
    RAD->Dispatcher((K87_EVENT)itm->Ev);
    return 1;
  }
  return 0;
}
//------------------------------------------------------------------
//  Mouse Click
//------------------------------------------------------------------
EClickResult CGenericADFRadioGauge::MouseClick (int x, int y, int buttons)
{ if (0 == RAD)           return MOUSE_TRACKING_OFF;
  K87_EVENT Event = K87EV_NULL;
  int dir  = (MOUSE_BUTTON_LEFT == buttons)?(-1):(+1);
  RAD->SetDirection(dir);
  if (AnyEvent(x,y))   return MOUSE_TRACKING_ON;
  return MOUSE_TRACKING_OFF;}

//--------------------------------------------------------------------------
//  Mouse moves over
//--------------------------------------------------------------------------
ECursorResult CGenericADFRadioGauge::MouseMoved (int mx, int my)
{  for (int i = 1; i< 15; i++) if (ck[i].MouseMoved(mx,my)) return CURSOR_WAS_CHANGED;
   return CURSOR_NOT_CHANGED;
}
//--------------------------------------------------------------------------
//  Draw One Field
//--------------------------------------------------------------------------
void CGenericADFRadioGauge::DrawField(RADIO_DSP *dsp)
{ RADIO_FLD *fd = RAD->GetField(dsp->fn);
  U_CHAR mask   = RAD->GetMask();
  if (0 == fd->state)       return;
  if ((RAD_ATT_FLASH == fd->state) && (0 == mask))  return;
  short No  = dsp->sp;
  if (ck[No].NoShow())      return;
  short x0  = ck[No].x1;
  short y0  = ck[No].y1;
  radi9->DrawNText(surf,x0,y0,amber,fd->data);
  return;
}

//--------------------------------------------------------------------------
//  Draw All fields
//--------------------------------------------------------------------------
void CGenericADFRadioGauge::DrawADF()
{ RADIO_DSP  *dsp = RadfFLD;
  while (dsp->fn) DrawField(dsp++);
  return;
}
//------------------------------------------------------------------
//  Draw the gauge
//------------------------------------------------------------------
void CGenericADFRadioGauge::Draw()
{ if (0 == RAD) GetRADIO();
  if (0 == RAD)   return;
  ClearDisplay();
  PowST = RAD->GetRState();
  if (0 == PowST) return;
  DrawADF();
  return;
}
//==============================================================================
//  CRMINeedleGauge
//==============================================================================
CRMINeedleGauge::CRMINeedleGauge (CPanel *mp)
: CBitmapGauge(mp)
{ comp_tag = 0;
  cpxy_x = cpxy_y = 0;
  anxy_x = anxy_y = 0;
  rnxy_x = rnxy_y = 0;
  adf1 = adf2 = nav1 = nav2 = 0;
  adf1_unit = adf2_unit = nav1_unit = nav2_unit = 0;
}
//------------------------------------------------------------------------------
CRMINeedleGauge::~CRMINeedleGauge(void)
{ 
}
//------------------------------------------------------------------------------
//  Read gauge tags
//------------------------------------------------------------------------------
int CRMINeedleGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'comp':
    ReadTag (&comp_tag, stream);
    ReadFrom (&comp, stream);
    return TAG_READ;

  case 'cpxy':
    ReadInt (&cpxy_x, stream);
    ReadInt (&cpxy_y, stream);
    return TAG_READ;

  case 'adfn':
    ReadFrom (&adfn, stream);
    return TAG_READ;

  case 'anxy':
    ReadInt (&anxy_x, stream);
    ReadInt (&anxy_y, stream);
    return TAG_READ;

  case 'rmin':
    ReadFrom (&rmin, stream);
    return TAG_READ;

  case 'rnxy':
    ReadInt (&rnxy_x, stream);
    ReadInt (&rnxy_y, stream);
    return TAG_READ;

  case 'adf1':
  case 'adfr':
    ReadTag (&adf1, stream);
    ReadInt (&adf1_unit, stream);
    return TAG_READ;

  case 'adf2':
    ReadTag (&adf2, stream);
    ReadInt (&adf2_unit, stream);
    return TAG_READ;

  case 'nav1':
    ReadTag (&nav1, stream);
    ReadInt (&nav1_unit, stream);
    return TAG_READ;

  case 'nav2':
    ReadTag (&nav2, stream);
    ReadInt (&nav2_unit, stream);
    return TAG_READ;

  case 'adfa':
    aadf.Read(stream,2);
    return TAG_READ;

  case 'vora':
    avor.Read(stream,2);
    return TAG_READ;

  case 'lbtn':
    ReadFrom (&lbtn, stream);
    return TAG_READ;

  case 'rbtn':
    ReadFrom (&rbtn, stream);
    return TAG_READ;
  }
  return CBitmapGauge::Read (stream, tag);
}
//-------------------------------------------------------------------
//  Mouse move over
//-------------------------------------------------------------------
ECursorResult CRMINeedleGauge::MouseMoved (int x, int y)
{ // Send updated mouse position to all click areas
  if (lbtn.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  if (rbtn.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}


//====================================================================
// CNavGaugeGroup
//====================================================================
CNavGaugeGroup::CNavGaugeGroup (CPanel *mp)
: CGauge(mp)
{  nav1 = nav2 = 0; }

int CNavGaugeGroup::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'nav1':
    ReadTag (&nav1, stream);
    rc = TAG_READ;
    break;

  case 'nav2':
    ReadTag (&nav2, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CGauge::Read (stream, tag);
  }

  return rc;
}


//
// TO BE DELETED CRadioGaugeGroup
//
CRadioGaugeGroup::CRadioGaugeGroup (CPanel *mp)
: CGauge(mp)
{
  rad1_tag = rad2_tag = key_tag = 0;
}

int CRadioGaugeGroup::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'rad1':
    ReadTag (&rad1_tag, stream);
    rc = TAG_READ;
    break;

  case 'rad2':
    ReadTag (&rad2_tag, stream);
    rc = TAG_READ;
    break;

  case 'key_':
    ReadTag (&key_tag, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CGauge::Read (stream, tag);
  }

  return rc;
}


//====================================================================
// CAnnunciator
//====================================================================
CAnnunciator::CAnnunciator (CPanel *mp)
: CBitmapGauge(mp)
{ //--- Init message default values -----------------
}
//--------------------------------------------------------------------
//  Read parameters
//--------------------------------------------------------------------
int CAnnunciator::Read (SStream *stream, Tag tag)
{ if (tag != 'bmap')  return CBitmapGauge::Read (stream, tag);
  under.Read(stream,0);
  return TAG_READ;
}
//--------------------------------------------------------------------
//  All parameters are read
//--------------------------------------------------------------------
void CAnnunciator::ReadFinished()
{ mesg.dataType       = TYPE_INT;
  CBitmapGauge::ReadFinished();
  return;
}
//--------------------------------------------------------------------
//  Draw annunciator
//--------------------------------------------------------------------
void CAnnunciator::Draw()
{ Send_Message(&mesg);
  int fr = (mesg.intData)?(1):(0);
  under.Draw(surf,fr);
  return;
}
//========================================================================
// JSDEV* Implement CFlyhawkAnnunciator
//  Default values are for Hawk panel
//========================================================================
CFlyhawkAnnunciator::CFlyhawkAnnunciator (CPanel *mp)
:CGauge(mp)
{	InitBlock(&lowl, 6, 6);				// Left tank
	InitBlock(&lowr,12, 6);				// Right tank
	InitBlock(&line,50,10);				// Line test
	InitBlock(&oilp, 6,14);				// Oil pres
	InitBlock(&vacl,40,14);				// vacuum left
	InitBlock(&vacr,46,14);				// vacuum right
	InitBlock(&volt,70,14);				// volt
}
//--------------------------------------------------------------------
//	Free resources
//--------------------------------------------------------------------
CFlyhawkAnnunciator::~CFlyhawkAnnunciator (void)
{	
}
//--------------------------------------------------------------------
//	Init Bloc ANN
//--------------------------------------------------------------------
void CFlyhawkAnnunciator::InitBlock(BlocANN *blc,int px,int py)
{	blc->x	= px;
	blc->y	= py;
  blc->amsg.sender          = unId;
	blc->amsg.id				      = MSG_GETDATA;
	blc->amsg.user.u.datatag	= 'actv';
	blc->amsg.user.u.hw			  = HW_LIGHT;
	blc->amsg.dataType			  = TYPE_INT;
	blc->blnk	= 0;							// Blink off
	return;
}
//--------------------------------------------------------------------
//	Fill  the requested bloc
//--------------------------------------------------------------------
void CFlyhawkAnnunciator::FillBlock(BlocANN *blc,SStream *st,Tag tag)
{	ReadMonoStrip(st,blc->txd,&blc->x, &blc->y);
	blc->amsg.group	= tag;
	blc->x2		= blc->x + blc->txd.wd;
	blc->y2		= blc->y + blc->txd.ht;
return;	}
//--------------------------------------------------------------------
//	Poll block state
//--------------------------------------------------------------------
void CFlyhawkAnnunciator::PollBlock(BlocANN *blc)
{	int old	= blc->blnk;								            // previous state
	int	nvs	= old;
	if (blc->amsg.group)	Send_Message(&blc->amsg);	// Poll component
	nvs	= blc->amsg.intData;							          // New state
	blc->blnk	= nvs;
	if (old == nvs)				return;					          // No change
	if (nvs == 0) FillRect(surf, blc->x, blc->y, blc->x2, blc->y2, 0);
	else	        DrawTextureOn(blc->txd,*surf,  blc->x,  blc->y,  0);
	return;	}

//--------------------------------------------------------------------
//	Read all tags
//--------------------------------------------------------------------
int CFlyhawkAnnunciator::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {

  case 'lowl':
    FillBlock(&lowl,stream,'LQty');				// low fuel left
    return TAG_READ;

  case 'lowr':									// low fuel right
    FillBlock(&lowr,stream,'RQty');
    return TAG_READ;

  case 'oilp':
    FillBlock(&oilp,stream,'loOP');				// Oil pressure
    return TAG_READ;

  case 'vacl':
    FillBlock(&vacl,stream,'vac1');				// Vaccum left
    return TAG_READ;

  case 'vacr':
    FillBlock(&vacr,stream,'vac2');				// Vacuum right
    return TAG_READ;

  case 'volt':
    FillBlock(&volt,stream,'lowV');				// Low volt
    return TAG_READ;

  case 'line':
    FillBlock(&line,stream,'line');				// light test
    return TAG_READ;
  }
  return CGauge::Read (stream, tag);
}

//--------------------------------------------------------------------
//	Draw the annunciators
//	NOTE:  both FUEL and VACUUM are not directly connected to
//		a subsystem.  Those status are computed from left and right
//		warning status.
//--------------------------------------------------------------------
void CFlyhawkAnnunciator::Draw (void)
{	PollBlock(&lowl);					// Left Fuel
	PollBlock(&lowr);					// Fuel right
	PollBlock(&oilp);
	PollBlock(&vacl);
	PollBlock(&vacr);
	PollBlock(&volt);
	PollBlock(&line);
  RenderSurface(quad,surf);
return;	}
//======================================================================
//	CFlyhawkAnnunciatorTest
//	JSDEV*  This is a special switch for FlyHawk
//	pos 0	Set the test mode in the ann panel
//	pos 1	Set Bright mode
//	pos 2	Set Dim mode
//======================================================================
CFlyhawkAnnunciatorTest::CFlyhawkAnnunciatorTest (CPanel *mp)
: CBitmapGauge(mp)
{
  // Cursor initialization
  csru_tag = csrd_tag = 0;

  //----Set Increment Table ----------
  IncT[0]	= BRIT;
  IncT[1]	= DIMR;
  IncT[2]	= DIMR;
  IncT[3]	= DIMR;
  //--- Set Decrement Table ---------
  DecT[0]	= TEST;
  DecT[1]	= TEST;
  DecT[2]	= BRIT;
  DecT[3]	= BRIT;
  //--- Set Current position --------
  cPos		= BRIT;
  //--- Init Message array -----------
  InitMsg(&msgT[TEST],'test');
  InitMsg(&msgT[BRIT],'abrt');
  InitMsg(&msgT[DIMR],'adim');
}
//---------------------------------------------------------------
//	Init a message
//---------------------------------------------------------------
void CFlyhawkAnnunciatorTest::InitMsg(SMessage *msg,Tag tag)
{	msg->group			  = tag;
  msg->sender       = unId;
	msg->id				    = MSG_SETDATA;
	msg->dataType		  = TYPE_INT;
	msg->user.u.hw		= HW_SWITCH;
	msg->user.u.datatag	= 'stat';
	return;	
}
//---------------------------------------------------------------
CFlyhawkAnnunciatorTest::~CFlyhawkAnnunciatorTest (void)
{
}
//----------------------------------------------------------------
//	Read All Tags
//----------------------------------------------------------------
int CFlyhawkAnnunciatorTest::Read (SStream *stream, Tag tag)
{
  switch (tag) {
  case 'swit':
    butn.Read(stream,0);
    return TAG_READ;

  case 'csru':
    csru_tag = BindCursor(stream);
    return TAG_READ;

  case 'csrd':
    csrd_tag = BindCursor(stream);
    return TAG_READ;
  }
  return CBitmapGauge::Read (stream, tag);
 }
//--------------------------------------------------------------
//
//---------------------------------------------------------------
void CFlyhawkAnnunciatorTest::ReadFinished (void)
{
  CBitmapGauge::ReadFinished ();
  return;
}

//------------------------------------------------------------
// JSDEV* Draw according to position
//------------------------------------------------------------
void CFlyhawkAnnunciatorTest::Draw (void)
{// Draw appropriate bitmap frame
  butn.Draw(surf, 0, 0, cPos);
  return;
}
//------------------------------------------------------------
//	JSDEV* Update the components
//------------------------------------------------------------
void CFlyhawkAnnunciatorTest::UpdateComponent(char old,char now)
{	SMessage	*msg = &msgT[old];
  globals->snd->Play(sbuf[GAUGE_ON__POS]);
	//---reset previous component -----------------
	msg->intData	= 0;
	Send_Message(msg);
	//---set the new component --------------------
	msg	= &msgT[now];
	msg->intData	= 1;
	Send_Message(msg);
	return;
}
//------------------------------------------------------------
//	JSDEV* Increment switch state
//------------------------------------------------------------
void CFlyhawkAnnunciatorTest::IncState (void)
{
	char old	= cPos;						// Current position
	cPos		= IncT[cPos];				// New position
	// Update subsystem 
	if (cPos == old)		return;			// No change
	UpdateComponent(old,cPos);
	return;
}

//------------------------------------------------------------
// JSDEV* Decrement switch state
//------------------------------------------------------------
void CFlyhawkAnnunciatorTest::DecState (void)
{
	char old	= cPos;						// Current position
	cPos		= DecT[cPos];				// new position
	if (cPos == old)		return;			// No change
	UpdateComponent(old,cPos);
	return;
  }
//-------------------------------------------------------------
//	Move over the button
//-------------------------------------------------------------
ECursorResult CFlyhawkAnnunciatorTest::MouseMoved (int mouseX, int mouseY)
{ // Default, vertical orientation
  if (mouseY < cy) return globals->cum->SetCursor (csru_tag);
  if (mouseY > cy) return globals->cum->SetCursor (csrd_tag);
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//------------------------------------------------------------------------------------
//	Release the TEST button
//------------------------------------------------------------------------------------
EClickResult CFlyhawkAnnunciatorTest::StopClick()
{	if (cPos == TEST)	IncState();
	return MOUSE_TRACKING_OFF;}
//------------------------------------------------------------------------------------
//	Click over the button
//------------------------------------------------------------------------------------
EClickResult CFlyhawkAnnunciatorTest::MouseClick (int mouseX, int mouseY, int buttons)
{
  // Default, vertical orientation
    if (mouseY < (cy)) {
      // Top half of the gauge increments the state
      DecState ();
    } else {
      // Bottom half of the gauge decrements the state
      IncState ();
    }
  return MOUSE_TRACKING_ON;
}

//====================================================================
// CFlyhawkFuelSelectorGauge
//====================================================================
CFlyhawkFuelSelectorGauge::CFlyhawkFuelSelectorGauge (CPanel *mp)
: CBitmapGauge(mp)
{  switch_frames = 1;}
//---------------------------------------------------------------
CFlyhawkFuelSelectorGauge::~CFlyhawkFuelSelectorGauge (void)
{}
//---------------------------------------------------------------
//	JSDEV* Prepare Message
//--------------------------------------------------------------
void CFlyhawkFuelSelectorGauge::PrepareMsg(CVehicleObject *veh)
{	Lmsg.sender   = unId;
  Lmsg.id		    = MSG_SETDATA;
	Lmsg.dataType = TYPE_INT;
	veh->FindReceiver(&Lmsg);
  Rmsg.sender   = unId;
	Rmsg.id		    = MSG_SETDATA;
  Rmsg.dataType = TYPE_INT;
	veh->FindReceiver(&Rmsg);

	CGauge::PrepareMsg(veh);
	return;
}
//-----------------------------------------------------------------
//  Read GAUGE Tags
//-----------------------------------------------------------------
int CFlyhawkFuelSelectorGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'swit':
    swit.Read(stream,0);
    return TAG_READ;

  case 'left':
    ReadFrom (&left, stream);
    return TAG_READ;

  case 'both':
    ReadFrom (&both, stream);
    return TAG_READ;

  case 'righ':
    ReadFrom (&righ, stream);
    return TAG_READ;

  case 'Lmsg':
    ReadMessage (&Lmsg, stream);
    return TAG_READ;

  case 'Rmsg':
    ReadMessage (&Rmsg, stream);
    return TAG_READ;
  }
  return CBitmapGauge::Read (stream, tag);
}
//-------------------------------------------------------------------
//  All parameters are read
//-------------------------------------------------------------------
void CFlyhawkFuelSelectorGauge::ReadFinished (void)
{
  CBitmapGauge::ReadFinished ();
}
//--------------------------------------------------------------------
//  Mouse moves over
//--------------------------------------------------------------------
ECursorResult CFlyhawkFuelSelectorGauge::MouseMoved (int x, int y)
{ // Send updated mouse position to all click areas
  if (left.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  if (righ.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  if (both.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//-------------------------------------------------------------------------

EClickResult CFlyhawkFuelSelectorGauge::MouseClick (int x, int y, int buttons)
{
  EClickResult rc = MOUSE_TRACKING_OFF;

  // Only handle left/right mouse clicks
  if ((buttons & (MOUSE_BUTTON_LEFT | MOUSE_BUTTON_RIGHT)) == 0) {
    return rc;
  }

  if (left.IsHit(x, y)) {
    switch_frames = 0;
	// Set fuel tank
  Lmsg.intData = 1;
	Send_Message (&Lmsg);
  Rmsg.intData = 0;
	Send_Message (&Rmsg);
  }
  else if (both.IsHit(x, y)) {
  switch_frames = swit.GetHiFrame() >> 1;
	// Set fuel tank
  Lmsg.intData = 1;
	Send_Message (&Lmsg);
  Rmsg.intData = 1;
	Send_Message (&Rmsg);
  }
  else if (righ.IsHit(x, y)) {
  switch_frames = swit.GetHiFrame();
	// Set fuel tank
  Lmsg.intData = 0;
	Send_Message (&Lmsg);
  Rmsg.intData = 1;
	Send_Message (&Rmsg);
  }

  return rc;
}
//------------------------------------------------------------
//    Draw the gauge
//------------------------------------------------------------
void CFlyhawkFuelSelectorGauge::Draw(void)
{
  // Draw Gauge
  CBitmapGauge::Draw();
  swit.Draw(surf, 0, 0, switch_frames);
}


//=================================================================
// CFlyhawkElevatorTrimGauge
//==================================================================
CFlyhawkElevatorTrimGauge::CFlyhawkElevatorTrimGauge (CPanel *mp)
: CBitmapGauge(mp)
{ 
  deflect = 0.0f;
}
//------------------------------------------------------------------
//  Destructor
//------------------------------------------------------------------
CFlyhawkElevatorTrimGauge::~CFlyhawkElevatorTrimGauge (void)
{
}

//------------------------------------------------------------------
//  Read the gauge tags
//------------------------------------------------------------------
int CFlyhawkElevatorTrimGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'trmw':
    wheel.Read(stream,2);
    return TAG_READ;

  case 'trmi':
    indwh.Read(stream,2);
    return TAG_READ;

  case 'down':
    ReadFrom (&down, stream);
    return TAG_READ;

  case 'up  ':
    ReadFrom (&up, stream);
    return TAG_READ;
  }
  return CBitmapGauge::Read (stream, tag);
}
//-----------------------------------------------------------------
//  All tags read
//  NOTE:  ratio is the number of frame per deflection
//          There are K frames for deflection in [-1,+1]
//          Thus R = K / 2;
//-----------------------------------------------------------------
void CFlyhawkElevatorTrimGauge::ReadFinished (void)
{ CBitmapGauge::ReadFinished ();
  mesg.id = MSG_SETDATA;
  mesg.dataType = TYPE_REAL;
  //----Compute frame per deflection --------------
  wRatio = float(wheel.GetNbFrame()) / 2;
  iRatio = float(indwh.GetNbFrame()) / 2;
  mWheel = (wheel.GetNbFrame() >> 1);
  mIndwh = (indwh.GetNbFrame() >> 1);
  return;
}
//---------------------------------------------------------------------
//  Mouse move over
//---------------------------------------------------------------------
ECursorResult CFlyhawkElevatorTrimGauge::MouseMoved (int x, int y)
{ // Send updated mouse position to all click areas
  if (down.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  if (  up.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//----------------------------------------------------------------------
//  Click gauge change trim value
//----------------------------------------------------------------------
EClickResult CFlyhawkElevatorTrimGauge::MouseClick (int x, int y, int buttons)
{ // Only handle left/right mouse clicks
  if ((buttons & (MOUSE_BUTTON_LEFT | MOUSE_BUTTON_RIGHT)) == 0)   return MOUSE_TRACKING_OFF;

  if (down.IsHit(x, y)) {
    mesg.id = MSG_SETDATA;
    mesg.user.u.datatag = 'decr';
    Send_Message (&mesg);
    deflect = float(mesg.realData);
    return MOUSE_TRACKING_ON;
  }
  if (up.IsHit(x, y)) {
    mesg.id = MSG_SETDATA;
    mesg.user.u.datatag = 'incr';
    Send_Message (&mesg);
    deflect = float(mesg.realData);

    return MOUSE_TRACKING_ON;
  }
  return MOUSE_TRACKING_OFF;
}
//------------------------------------------------------------------------------
//  Send another click
//------------------------------------------------------------------------------
EClickResult CFlyhawkElevatorTrimGauge::TrackClick (int x, int y, int buttons)
{   return MouseClick (x, y, buttons); }
//----------------------------------------------------------------------
// Stop click gauge
//----------------------------------------------------------------------
EClickResult CFlyhawkElevatorTrimGauge::StopClick()
{ return MOUSE_TRACKING_OFF;
}
//-----------------------------------------------------------------------------
void CFlyhawkElevatorTrimGauge::Draw(void)
{
  // Draw Gauge
  CBitmapGauge::Draw();
  mesg.id = MSG_GETDATA;
  mesg.user.u.datatag = 'rawv';
  Send_Message (&mesg);
  deflect = float(mesg.realData);
  int wf = Round(wRatio * deflect) + mWheel;
  wheel.Draw(surf,wf);
  int rf = Round(iRatio * deflect) + mIndwh;
  indwh.Draw(surf,rf);
}


//
// CMalybooCHTIndicatorGauge
//
CMalybooCHTIndicatorGauge::CMalybooCHTIndicatorGauge (CPanel *mp)
: CBitmapGauge(mp)
{
  for (int i=0; i<6; i++) xoff[i] = 0;
  yoff = 0;
}

int CMalybooCHTIndicatorGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'xoff':
    ReadInt (&xoff[0], stream);
    ReadInt (&xoff[1], stream);
    ReadInt (&xoff[2], stream);
    ReadInt (&xoff[3], stream);
    ReadInt (&xoff[4], stream);
    ReadInt (&xoff[5], stream);
    rc = TAG_READ;
    break;

  case 'yoff':
    ReadInt (&yoff, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}


//
// CMalybooCHTButtonGauge
//
CMalybooCHTButtonGauge::CMalybooCHTButtonGauge (CPanel *mp)
: CBitmapGauge(mp)
{
}

int CMalybooCHTButtonGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}


//
// CMalybooNavGpsPanelGauge
//

CMalybooNavGpsPanelGauge::CMalybooNavGpsPanelGauge (CPanel *mp)
: CBitmapGauge(mp)
{
  xnav_x = xnav_y = 0;
  xgps_x = xgps_y = 0;
  xapr_x = xapr_y = 0;
  xarm_x = xarm_y = 0;
  xact_x = xact_y = 0;
  xcrs_x = xcrs_y = 0;
  xobs_x = xobs_y = 0;
  xleg_x = xleg_y = 0;
  xmsg_x = xmsg_y = 0;
  xwpt_x = xwpt_y = 0;
  xyaw_x = xyaw_y = 0;
}

int CMalybooNavGpsPanelGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'cnav':
    ReadFrom (&cnav, stream);
    rc = TAG_READ;
    break;

  case 'xnav':
    ReadInt (&xnav_x, stream);
    ReadInt (&xnav_y, stream);
    rc = TAG_READ;
    break;

  case 'xgps':
    ReadInt (&xgps_x, stream);
    ReadInt (&xgps_y, stream);
    rc = TAG_READ;
    break;

  case 'capr':
    ReadFrom (&cnav, stream);
    rc = TAG_READ;
    break;

  case 'xapr':
    ReadInt (&xapr_x, stream);
    ReadInt (&xapr_y, stream);
    rc = TAG_READ;
    break;

  case 'xarm':
    ReadInt (&xarm_x, stream);
    ReadInt (&xarm_y, stream);
    rc = TAG_READ;
    break;

  case 'xact':
    ReadInt (&xact_x, stream);
    ReadInt (&xact_y, stream);
    rc = TAG_READ;
    break;

  case 'ccrs':
    ReadFrom (&cnav, stream);
    rc = TAG_READ;
    break;

  case 'xcrs':
    ReadInt (&xcrs_x, stream);
    ReadInt (&xcrs_y, stream);
    rc = TAG_READ;
    break;

  case 'xobs':
    ReadInt (&xobs_x, stream);
    ReadInt (&xobs_y, stream);
    rc = TAG_READ;
    break;

  case 'xleg':
    ReadInt (&xleg_x, stream);
    ReadInt (&xleg_y, stream);
    rc = TAG_READ;
    break;

  case 'cmsg':
    ReadFrom (&cnav, stream);
    rc = TAG_READ;
    break;

  case 'xmsg':
    ReadInt (&xmsg_x, stream);
    ReadInt (&xmsg_y, stream);
    rc = TAG_READ;
    break;

  case 'xwpt':
    ReadInt (&xwpt_x, stream);
    ReadInt (&xwpt_y, stream);
    rc = TAG_READ;
    break;

  case 'cyaw':
    ReadFrom (&cnav, stream);
    rc = TAG_READ;
    break;

  case 'xyaw':
    ReadInt (&xyaw_x, stream);
    ReadInt (&xyaw_y, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}



//
// CMalybooDigitalClockGauge
//

CMalybooDigitalClockGauge::CMalybooDigitalClockGauge (CPanel *mp)
: CBitmapGauge(mp)
{
  clkd_x1 = clkd_y1 = clkd_x2 = clkd_y2 = 0;
}

int CMalybooDigitalClockGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'clkd':
    ReadInt (&clkd_x1, stream);
    ReadInt (&clkd_y1, stream);
    ReadInt (&clkd_x2, stream);
    ReadInt (&clkd_y2, stream);
    rc = TAG_READ;
    break;

  case 'mode':
    ReadFrom (&mode, stream);
    rc = TAG_READ;
    break;

  case 'powr':
    ReadFrom (&powr, stream);
    rc = TAG_READ;
    break;

  case 'play':
    ReadFrom (&play, stream);
    rc = TAG_READ;
    break;

  case 'rec_':
    ReadFrom (&rec, stream);
    rc = TAG_READ;
    break;

  case 'set_':
    ReadFrom (&set, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}


//
// CMalybooEngineMonitorGauge
//
CMalybooEngineMonitorGauge::CMalybooEngineMonitorGauge (CPanel *mp)
: CBitmapGauge(mp)
{
  strcpy (rlit_art, "");
  strcpy (glit_art, "");
  strcpy (ylit_art, "");
}

int CMalybooEngineMonitorGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'fuel':
    ReadFrom (&fuel, stream);
    rc = TAG_READ;
    break;

  case 'inst':
    ReadFrom (&inst, stream);
    rc = TAG_READ;
    break;

  case 'blk_':
    ReadFrom (&blk_, stream);
    rc = TAG_READ;
    break;

  case 'fill':
    ReadFrom (&fill, stream);
    rc = TAG_READ;
    break;

  case 'pwr_':
    ReadFrom (&pwr_, stream);
    rc = TAG_READ;
    break;

  case 'set_':
    ReadFrom (&set_, stream);
    rc = TAG_READ;
    break;

  case 'MAP_':
    ReadFrom (&MAP_, stream);
    rc = TAG_READ;
    break;

  case 'RPM_':
    ReadFrom (&RPM_, stream);
    rc = TAG_READ;
    break;

  case 'TIT_':
    ReadFrom (&TIT, stream);
    rc = TAG_READ;
    break;

  case 'FF__':
    ReadFrom (&FF__, stream);
    rc = TAG_READ;
    break;

  case 'OT__':
    ReadFrom (&OT__, stream);
    rc = TAG_READ;
    break;

  case 'OP__':
    ReadFrom (&OP__, stream);
    rc = TAG_READ;
    break;

  case 'CHT_':
    ReadFrom (&CHT_, stream);
    rc = TAG_READ;
    break;

  case 'VAC_':
    ReadFrom (&VAC_, stream);
    rc = TAG_READ;
    break;

  case 'LQTY':
    ReadFrom (&LQTY, stream);
    rc = TAG_READ;
    break;

  case 'RQTY':
    ReadFrom (&RQTY, stream);
    rc = TAG_READ;
    break;

  case 'rlit':
    ReadString (rlit_art, 64, stream);
    rc = TAG_READ;
    break;

  case 'glit':
    ReadString (glit_art, 64, stream);
    rc = TAG_READ;
    break;

  case 'ylit':
    ReadString (ylit_art, 64, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}


//==============================================================================
// CNavajoFuelSelectorGauge
//==============================================================================
CNavajoFuelSelectorGauge::CNavajoFuelSelectorGauge (CPanel *mp)
: CBitmapGauge(mp)
{  cIndx = 1; oIndx = -10;
}
//-------------------------------------------------------------------
//	Pepare messages
//-------------------------------------------------------------------
void CNavajoFuelSelectorGauge::PrepareMsg(CVehicleObject *veh)
{	obms.sender   = unId;
  obms.id		    = MSG_SETDATA;
  obms.dataType = TYPE_INT;
	veh->FindReceiver(&obms);
  ibms.sender   = unId;
	ibms.id		    = MSG_SETDATA;
  ibms.dataType = TYPE_INT;
	veh->FindReceiver(&ibms);
	CGauge::PrepareMsg(veh);
	return;
}
//------------------------------------------------------------------
int CNavajoFuelSelectorGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'swit':
    swit.Read(stream,0);
    return TAG_READ;
    break;

  case 'obca':
    ReadFrom (&obca, stream);
    rc = TAG_READ;
    break;

  case 'ofca':
    ReadFrom (&ofca, stream);
    rc = TAG_READ;
    break;

  case 'ibca':
    ReadFrom (&ibca, stream);
    rc = TAG_READ;
    break;

  case 'obms':
    ReadMessage (&obms, stream);
    rc = TAG_READ;
    break;

  case 'ibms':
    ReadMessage (&ibms, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}
//---------------------------------------------------------------
//  Mouse Moves over
//---------------------------------------------------------------
ECursorResult CNavajoFuelSelectorGauge::MouseMoved (int x, int y)
{ // Send updated mouse position to all click areas
  if (obca.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  if (ibca.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  if (ofca.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//---------------------------------------------------------------
//  Mouse Click
//---------------------------------------------------------------
EClickResult CNavajoFuelSelectorGauge::MouseClick (int mouseX, int mouseY, int buttons)
{
  if (obca.IsHit (mouseX, mouseY)) {
    cIndx = 0;
    obms.intData	= 1;
    Send_Message (&obms);
    ibms.intData	= 0;
    Send_Message (&ibms);
    return MOUSE_TRACKING_OFF;
  }

  if (ofca.IsHit (mouseX, mouseY)) {
      cIndx = 1; 
      obms.intData	= 0;
      Send_Message (&obms);
      ibms.intData	= 0;
      Send_Message (&ibms);
      return MOUSE_TRACKING_OFF;
  }
  if (ibca.IsHit (mouseX, mouseY)) {
        cIndx = 2; 
        obms.intData	= 0;
        Send_Message (&obms);
        ibms.intData	= 1;
        Send_Message (&ibms);
        return MOUSE_TRACKING_OFF;
      }

  return MOUSE_TRACKING_OFF;
}
//--------------------------------------------------------------------------
//  Draw the switch
//--------------------------------------------------------------------------
void CNavajoFuelSelectorGauge::Draw (void)
{
  // Draw appropriate bitmap frame
  // if situation has changed
  if (cIndx != oIndx) {
    oIndx = cIndx;
    EraseSurfaceRGBA (surf, 0);
    swit.Draw(surf, 0, 0, cIndx);
  }
}

//=========================================================================
// CNavajoFuelAmmeterGauge
//=========================================================================
CNavajoAmmeterGauge::CNavajoAmmeterGauge (CPanel *mp)
:CNeedleGauge(mp)
{}
//--------------------------------------------------------------------
//	JSDEV* Prepare messages
//----------------------------------------------------------------------
void CNavajoAmmeterGauge::PrepareMsg(CVehicleObject *veh)
{	mChg.id		  = MSG_GETDATA;
  mChg.sender = unId;
	veh->FindReceiver(&mChg);
	mLtM.id		= MSG_GETDATA;
  mLtM.sender = unId;
	veh->FindReceiver(&mLtM);
	mRtM.id		= MSG_GETDATA;
  mRtM.sender = unId;
	veh->FindReceiver(&mRtM);
	mLtS.id		= MSG_GETDATA;
  mLtS.sender = unId;
	veh->FindReceiver(&mLtS);
	mRtS.id		= MSG_GETDATA;
  mRtS.sender = unId;
	veh->FindReceiver(&mRtS);

	CNeedleGauge::PrepareMsg(veh);
	return;
}
int CNavajoAmmeterGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'nedl':
    ReadFrom (&nedl, stream);
    rc = TAG_READ;
    break;

  case 'mChg':
    ReadMessage (&mChg, stream);
    rc = TAG_READ;
    break;

  case 'mLtM':
    ReadMessage (&mLtM, stream);
    rc = TAG_READ;
    break;

  case 'mRtM':
    ReadMessage (&mRtM, stream);
    rc = TAG_READ;
    break;

  case 'mLtS':
    ReadMessage (&mLtS, stream);
    rc = TAG_READ;
    break;

  case 'mRtS':
    ReadMessage (&mRtS, stream);
    rc = TAG_READ;
    break;

  case 'ca01':
    ReadFrom (&ca01, stream);
    rc = TAG_READ;
    break;

  case 'ca02':
    ReadFrom (&ca02, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CNeedleGauge::Read (stream, tag);
  }

  return rc;
}


//==============================================================================
// CNavajoRadarAltimeterGauge
//==============================================================================
CNavajoRadarAltimeterGauge::CNavajoRadarAltimeterGauge (CPanel *mp)
:CNeedleGauge(mp)
{ }
//--------------------------------------------------------------
//	JSDEV* Prepare message
//--------------------------------------------------------------
void CNavajoRadarAltimeterGauge::PrepareMsg(CVehicleObject *veh)
{	bMsg.id		  = MSG_GETDATA;
  bMsg.sender = unId;
	veh->FindReceiver(&bMsg);
	dhms.id		  = MSG_GETDATA;
  dhms.sender = unId;
	veh->FindReceiver(&dhms);
	CNeedleGauge::PrepareMsg(veh);
	return;
}
//-----------------------------------------------------------------------------
CNavajoRadarAltimeterGauge::~CNavajoRadarAltimeterGauge(void)
{}
//-----------------------------------------------------------------------------
//  Read gauge tag
//-----------------------------------------------------------------------------
int CNavajoRadarAltimeterGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'bMsg':
    ReadMessage (&bMsg, stream);
    return TAG_READ;

  case 'dhbt':
    SkipObject (stream);
//    ReadTable (&dhbt, stream);
    return TAG_READ;

  case 'bug ':
    abug.Read(stream,2);
    return TAG_READ;

  case 'plit':
    plit.Read(stream,2);
    return TAG_READ;

  case 'dhms':
    ReadMessage (&dhms, stream);
    return TAG_READ;

  case 'knob':
    ReadFrom (&knob, stream);
    return TAG_READ;
  }
  return CNeedleGauge::Read (stream, tag);
}

//
// CCollinsEFISSelectionGauge
//
CCollinsEFISSelectionGauge::CCollinsEFISSelectionGauge (CPanel *mp)
: CBitmapGauge(mp)
{}
//---------------------------------------------------------------
//	JSDEV* Prepare message
//---------------------------------------------------------------
void CCollinsEFISSelectionGauge::PrepareMsg(CVehicleObject *veh)
{	pfdm.id		  = MSG_GETDATA;
  pfdm.sender = unId;
	veh->FindReceiver(&pfdm);
	ehim.id		= MSG_GETDATA;
  ehim.sender = unId;
	veh->FindReceiver(&ehim);
	mndm.id		= MSG_GETDATA;
  mndm.sender = unId;
	veh->FindReceiver(&mndm);
	adm_.id		= MSG_GETDATA;
  adm_.sender = unId;
	veh->FindReceiver(&adm_);
	aplt.id		= MSG_GETDATA;
  aplt.sender = unId;
	veh->FindReceiver(&aplt);

	CGauge::PrepareMsg(veh);
	return;	
}
int CCollinsEFISSelectionGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'brg_':
    ReadFrom (&brg_, stream);
    rc = TAG_READ;
    break;

  case 'brgm':
    ReadFrom (&brgm, stream);
    rc = TAG_READ;
    break;

  case 'brgk':
    ReadFrom (&brgk, stream);
    rc = TAG_READ;
    break;

  case 'dh__':
    ReadFrom (&dh__, stream);
    rc = TAG_READ;
    break;

  case 'form':
    ReadFrom (&form, stream);
    rc = TAG_READ;
    break;

  case 'crs_':
    ReadFrom (&crs_, stream);
    rc = TAG_READ;
    break;

  case 'crsm':
    ReadFrom (&crsm, stream);
    rc = TAG_READ;
    break;

  case 'crsk':
    ReadFrom (&crsk, stream);
    rc = TAG_READ;
    break;

  case 'wx__':
    ReadFrom (&wx__, stream);
    rc = TAG_READ;
    break;

  case 'nav_':
    ReadFrom (&nav_, stream);
    rc = TAG_READ;
    break;

  case 'pfdm':
    ReadMessage (&pfdm, stream);
    rc = TAG_READ;
    break;

  case 'ehim':
    ReadMessage (&ehim, stream);
    rc = TAG_READ;
    break;

  case 'mndm':
    ReadMessage (&mndm, stream);
    rc = TAG_READ;
    break;

  case 'adm_':
    ReadMessage (&adm_, stream);
    rc = TAG_READ;
    break;

  case 'aplt':
    ReadMessage (&aplt, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}


//
// CCollinsVspeedPanel
//
CCollinsVspeedPanel::CCollinsVspeedPanel (CPanel *mp)
: CBitmapGauge(mp)
{
}

int CCollinsVspeedPanel::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'sped':
    ReadFrom (&sped, stream);
    rc = TAG_READ;
    break;

  case 'vsel':
    ReadFrom (&vsel, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}



//
// CCollinsHFRadioGauge
//
CCollinsHFRadioGauge::CCollinsHFRadioGauge (CPanel *mp)
: CBitmapGauge(mp)
{
  ft13 = false;
}

int CCollinsHFRadioGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'tel_':
    ReadFrom (&tel, stream);
    rc = TAG_READ;
    break;

  case 'am__':
    ReadFrom (&am, stream);
    rc = TAG_READ;
    break;

  case 'usb_':
    ReadFrom (&usb, stream);
    rc = TAG_READ;
    break;

  case 'lsb_':
    ReadFrom (&lsb, stream);
    rc = TAG_READ;
    break;

  case 'mhz_':
    ReadFrom (&mhz, stream);
    rc = TAG_READ;
    break;

  case 'khz_':
    ReadFrom (&khz, stream);
    rc = TAG_READ;
    break;

  case 'stor':
    ReadFrom (&stor, stream);
    rc = TAG_READ;
    break;

  case 'tune':
    ReadFrom (&tune, stream);
    rc = TAG_READ;
    break;

  case 'volu':
    ReadFrom (&volu, stream);
    rc = TAG_READ;
    break;

  case 'dash':
    ReadFrom (&dash, stream);
    rc = TAG_READ;
    break;

  case 'tx__':
    ReadFrom (&tx, stream);
    rc = TAG_READ;
    break;

  case 'chan':
    ReadFrom (&chan, stream);
    rc = TAG_READ;
    break;

  case 'ft13':
    ft13 = true;
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}


//
// CCollinsPFDGauge
//
CCollinsPFDGauge::CCollinsPFDGauge (CPanel *mp)
: CBitmapGauge(mp)
{
}

int CCollinsPFDGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}


//
// CCollinsNDGauge
//
CCollinsNDGauge::CCollinsNDGauge (CPanel *mp)
: CBitmapGauge(mp)
{
}

int CCollinsNDGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}


//=============================================================================
// CCollinsRTUGauge
//=============================================================================
CCollinsRTUGauge::CCollinsRTUGauge (CPanel *mp)
: CBitmapGauge(mp)
{   side = 0;
}
//--------------------------------------------------------------
//	JSDEV* Prepare Messages
//---------------------------------------------------------------
void CCollinsRTUGauge::PrepareMsg(CVehicleObject *veh)
{	rtu_.id		= MSG_GETDATA;
  rtu_.sender = unId;
	veh->FindReceiver(&rtu_);
	dme1.id		= MSG_GETDATA;
  dme1.sender = unId;
	veh->FindReceiver(&dme1);
	dme2.id		= MSG_GETDATA;
  dme2.sender = unId;
	veh->FindReceiver(&dme2);

	CGauge::PrepareMsg(veh);
	return;
}
//----------------------------------------------------------------
int CCollinsRTUGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'lin1':
    ReadFrom (&lin1, stream);
    return TAG_READ;

  case 'lin2':
    ReadFrom (&lin2, stream);
    return TAG_READ;

  case 'lin3':
    ReadFrom (&lin3, stream);
    return TAG_READ;

  case 'lin4':
    ReadFrom (&lin4, stream);
    return TAG_READ;

  case 'lin5':
    ReadFrom (&lin4, stream);
    return TAG_READ;

  case 'btn1':
    ReadFrom (&btn1, stream);
    return TAG_READ;

  case 'btn2':
    ReadFrom (&btn2, stream);
    return TAG_READ;

  case 'btn3':
    ReadFrom (&btn3, stream);
    return TAG_READ;

  case 'btn4':
    ReadFrom (&btn4, stream);
    return TAG_READ;

  case 'btn5':
    ReadFrom (&btn5, stream);
    return TAG_READ;

  case 'cmaw':
    ReadFrom (&cmaw, stream);
    return TAG_READ;

  case 'cmaf':
    ReadFrom (&cmaf, stream);
    return TAG_READ;

  case 'cmsw':
    ReadFrom (&cmsw, stream);
    return TAG_READ;

  case 'cmsf':
    ReadFrom (&cmsf, stream);
    return TAG_READ;

  case 'nvaw':
    ReadFrom (&nvaw, stream);
    return TAG_READ;

  case 'nvaf':
    ReadFrom (&nvaf, stream);
    return TAG_READ;

  case 'adf1':
    ReadFrom (&adf1, stream);
    return TAG_READ;

  case 'adf2':
    ReadFrom (&adf2, stream);
    return TAG_READ;

  case 'adf3':
    ReadFrom (&adf3, stream);
    return TAG_READ;

  case 'adf4':
    ReadFrom (&adf4, stream);
    return TAG_READ;

  case 'adf5':
    ReadFrom (&adf5, stream);
    return TAG_READ;

  case 'xpd1':
    ReadFrom (&xpd1, stream);
    return TAG_READ;

  case 'xpd2':
    ReadFrom (&xpd2, stream);
    return TAG_READ;

  case 'xpd3':
    ReadFrom (&xpd3, stream);
    return TAG_READ;

  case 'xpd4':
    ReadFrom (&xpd4, stream);
    return TAG_READ;

  case 'atci':
    ReadFrom (&atci, stream);
    return TAG_READ;

  case 'dmeh':
    ReadFrom (&dmeh, stream);
    return TAG_READ;

  case 'coms':
    ReadFrom (&coms, stream);
    return TAG_READ;

  case 'atcs':
    ReadFrom (&atcs, stream);
    return TAG_READ;

  case '1/2_':
    ReadFrom (&b1_2, stream);
    return TAG_READ;

  case 'adft':
    ReadFrom (&xpd1, stream);
    return TAG_READ;

  case 'rtu_':
    ReadMessage (&rtu_, stream);
    return TAG_READ;

  case 'dme1':
    ReadMessage (&dme1, stream);
    return TAG_READ;

  case 'dme2':
    ReadMessage (&dme2, stream);
    return TAG_READ;

  case 'dmeH':
    adme.Read(stream,2);
    return TAG_READ;

  case 'repl':
    arep.Read(stream,2);
    return TAG_READ;

  case 'side':
    ReadInt (&side, stream);
    return TAG_READ;
  }
  return CBitmapGauge::Read (stream, tag);
}


//
// CCollinsAirDataGauge
//
CCollinsAirDataGauge::CCollinsAirDataGauge (CPanel *mp)
: CBitmapGauge(mp)
{
}

int CCollinsAirDataGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'scrn':
    ReadFrom (&scrn, stream);
    rc = TAG_READ;
    break;

  case 'baro':
    ReadFrom (&baro, stream);
    rc = TAG_READ;
    break;

  case 'vs__':
    ReadFrom (&vs__, stream);
    rc = TAG_READ;
    break;

  case 'alt_':
    ReadFrom (&alt_, stream);
    rc = TAG_READ;
    break;

  case 'in__':
    ReadFrom (&in__, stream);
    rc = TAG_READ;
    break;

  case 'mb__':
    ReadFrom (&mb__, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}


//
// CCollinsMNDGauge
//
CCollinsMNDGauge::CCollinsMNDGauge (CPanel *mp)
: CBitmapGauge(mp)
{}
//----------------------------------------------------
//	JSDEV* prepare messages
//-----------------------------------------------------
void CCollinsMNDGauge::PrepareMsg(CVehicleObject *veh)
{	obs_.id		  = MSG_GETDATA;
  obs_.sender = unId;
	veh->FindReceiver(&obs_);
	ap__.id		= MSG_GETDATA;
  ap__.sender = unId;
	veh->FindReceiver(&ap__);
	CGauge::PrepareMsg(veh);
	return;	
}
//----------------------------------------------------
int CCollinsMNDGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'obs_':
    ReadMessage (&obs_, stream);
    rc = TAG_READ;
    break;

  case 'ap__':
    ReadMessage (&ap__, stream);
    rc = TAG_READ;
    break;

  case 'frmt':
    ReadFrom (&frmt, stream);
    rc = TAG_READ;
    break;

  case 'hdg_':
    ReadFrom (&hdg_, stream);
    rc = TAG_READ;
    break;

  case 'crs_':
    ReadFrom (&crs_, stream);
    rc = TAG_READ;
    break;

  case 'brgt':
    ReadFrom (&brgt, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}


//
// CUniversalFMSGauge
//
CUniversalFMSGauge::CUniversalFMSGauge (CPanel *mp)
: CBitmapGauge(mp)
{
}

int CUniversalFMSGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'scrn':
    ReadFrom (&scrn, stream);
    rc = TAG_READ;
    break;

  case 'l1__':
    ReadFrom (&l1, stream);
    rc = TAG_READ;
    break;

  case 'l2__':
    ReadFrom (&l2, stream);
    rc = TAG_READ;
    break;

  case 'l3__':
    ReadFrom (&l3, stream);
    rc = TAG_READ;
    break;

  case 'l4__':
    ReadFrom (&l4, stream);
    rc = TAG_READ;
    break;

  case 'l5__':
    ReadFrom (&l5, stream);
    rc = TAG_READ;
    break;

  case 'r1__':
    ReadFrom (&r1, stream);
    rc = TAG_READ;
    break;

  case 'r2__':
    ReadFrom (&r2, stream);
    rc = TAG_READ;
    break;

  case 'r3__':
    ReadFrom (&r3, stream);
    rc = TAG_READ;
    break;

  case 'r4__':
    ReadFrom (&r4, stream);
    rc = TAG_READ;
    break;

  case 'r5__':
    ReadFrom (&r5, stream);
    rc = TAG_READ;
    break;

  case 'data':
    ReadFrom (&data, stream);
    rc = TAG_READ;
    break;

  case 'nav_':
    ReadFrom (&nav, stream);
    rc = TAG_READ;
    break;

  case 'vnav':
    ReadFrom (&vnav, stream);
    rc = TAG_READ;
    break;

  case 'dto_':
    ReadFrom (&dto, stream);
    rc = TAG_READ;
    break;

  case 'list':
    ReadFrom (&list, stream);
    rc = TAG_READ;
    break;

  case 'prev':
    ReadFrom (&prev, stream);
    rc = TAG_READ;
    break;

  case 'fuel':
    ReadFrom (&fuel, stream);
    rc = TAG_READ;
    break;

  case 'fpl_':
    ReadFrom (&fpl, stream);
    rc = TAG_READ;
    break;

  case 'perf':
    ReadFrom (&perf, stream);
    rc = TAG_READ;
    break;

  case 'tune':
    ReadFrom (&tune, stream);
    rc = TAG_READ;
    break;

  case 'menu':
    ReadFrom (&menu, stream);
    rc = TAG_READ;
    break;

  case 'next':
    ReadFrom (&next, stream);
    rc = TAG_READ;
    break;

  case '1___':
    ReadFrom (&key1, stream);
    rc = TAG_READ;
    break;

  case '2___':
    ReadFrom (&key2, stream);
    rc = TAG_READ;
    break;

  case '3___':
    ReadFrom (&key3, stream);
    rc = TAG_READ;
    break;

  case '4___':
    ReadFrom (&key4, stream);
    rc = TAG_READ;
    break;

  case '5___':
    ReadFrom (&key5, stream);
    rc = TAG_READ;
    break;

  case '6___':
    ReadFrom (&key6, stream);
    rc = TAG_READ;
    break;

  case '7___':
    ReadFrom (&key7, stream);
    rc = TAG_READ;
    break;

  case '8___':
    ReadFrom (&key8, stream);
    rc = TAG_READ;
    break;

  case '9___':
    ReadFrom (&key9, stream);
    rc = TAG_READ;
    break;

  case 'back':
    ReadFrom (&back, stream);
    rc = TAG_READ;
    break;

  case '0___':
    ReadFrom (&key0, stream);
    rc = TAG_READ;
    break;

  case 'msg_':
    ReadFrom (&msg, stream);
    rc = TAG_READ;
    break;

  case 'dim_':
    ReadFrom (&dim, stream);
    rc = TAG_READ;
    break;

  case '+-__':
    ReadFrom (&plus, stream);
    rc = TAG_READ;
    break;

  case 'entr':
    ReadFrom (&entr, stream);
    rc = TAG_READ;
    break;

  case 'A___':
    ReadFrom (&keyA, stream);
    rc = TAG_READ;
    break;

  case 'B___':
    ReadFrom (&keyB, stream);
    rc = TAG_READ;
    break;

  case 'C___':
    ReadFrom (&keyC, stream);
    rc = TAG_READ;
    break;

  case 'D___':
    ReadFrom (&keyD, stream);
    rc = TAG_READ;
    break;

  case 'E___':
    ReadFrom (&keyE, stream);
    rc = TAG_READ;
    break;

  case 'F___':
    ReadFrom (&keyF, stream);
    rc = TAG_READ;
    break;

  case 'G___':
    ReadFrom (&keyG, stream);
    rc = TAG_READ;
    break;

  case 'H___':
    ReadFrom (&keyH, stream);
    rc = TAG_READ;
    break;

  case 'I___':
    ReadFrom (&keyI, stream);
    rc = TAG_READ;
    break;

  case 'J___':
    ReadFrom (&keyJ, stream);
    rc = TAG_READ;
    break;

  case 'K___':
    ReadFrom (&keyK, stream);
    rc = TAG_READ;
    break;

  case 'L___':
    ReadFrom (&keyL, stream);
    rc = TAG_READ;
    break;

  case 'M___':
    ReadFrom (&keyM, stream);
    rc = TAG_READ;
    break;

  case 'N___':
    ReadFrom (&keyN, stream);
    rc = TAG_READ;
    break;

  case 'O___':
    ReadFrom (&keyO, stream);
    rc = TAG_READ;
    break;

  case 'P___':
    ReadFrom (&keyP, stream);
    rc = TAG_READ;
    break;

  case 'Q___':
    ReadFrom (&keyQ, stream);
    rc = TAG_READ;
    break;

  case 'R___':
    ReadFrom (&keyR, stream);
    rc = TAG_READ;
    break;

  case 'S___':
    ReadFrom (&keyS, stream);
    rc = TAG_READ;
    break;

  case 'T___':
    ReadFrom (&keyT, stream);
    rc = TAG_READ;
    break;

  case 'U___':
    ReadFrom (&keyU, stream);
    rc = TAG_READ;
    break;

  case 'V___':
    ReadFrom (&keyV, stream);
    rc = TAG_READ;
    break;

  case 'W___':
    ReadFrom (&keyW, stream);
    rc = TAG_READ;
    break;

  case 'X___':
    ReadFrom (&keyX, stream);
    rc = TAG_READ;
    break;

  case 'Y___':
    ReadFrom (&keyY, stream);
    rc = TAG_READ;
    break;

  case 'Z___':
    ReadFrom (&keyZ, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}


//
// CPilatusEFISPanel
//
CPilatusEFISPanel::CPilatusEFISPanel (CPanel *mp)
: CBitmapGauge(mp)
{}
//-----------------------------------------------------------------
//	Prepare Messages
//-----------------------------------------------------------------

void CPilatusEFISPanel::PrepareMsg(CVehicleObject *veh)
{	pfdm.id		= MSG_GETDATA;
  pfdm.sender = unId;
	veh->FindReceiver(&pfdm);
	navm.id		= MSG_GETDATA;
  navm.sender = unId;
	veh->FindReceiver(&navm);
	CGauge::PrepareMsg(veh);
	return;
}
//--------------------------------------------------------------
int CPilatusEFISPanel::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'pfdm':
    ReadMessage (&pfdm, stream);
    rc = TAG_READ;
    break;

  case 'navm':
    ReadMessage (&navm, stream);
    rc = TAG_READ;
    break;

  case 'dh__':
    ReadFrom (&dh, stream);
    rc = TAG_READ;
    break;

  case 'brt_':
    ReadFrom (&brt, stream);
    rc = TAG_READ;
    break;

  case 'crs_':
    ReadFrom (&crs, stream);
    rc = TAG_READ;
    break;

  case 'hdg_':
    ReadFrom (&hdg, stream);
    rc = TAG_READ;
    break;

  case 'hsi_':
    ReadFrom (&hsi, stream);
    rc = TAG_READ;
    break;

  case 'arc_':
    ReadFrom (&arc, stream);
    rc = TAG_READ;
    break;

  case 'nav_':
    ReadFrom (&nav, stream);
    rc = TAG_READ;
    break;

  case 'rgup':
    ReadFrom (&rgup, stream);
    rc = TAG_READ;
    break;

  case 'rgdn':
    ReadFrom (&rgdn, stream);
    rc = TAG_READ;
    break;

  case 'nav1':
    ReadFrom (&nav1, stream);
    rc = TAG_READ;
    break;

  case 'nav2':
    ReadFrom (&nav2, stream);
    rc = TAG_READ;
    break;

  case 'k1_2':
    ReadFrom (&k1_2, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}

//
// CWalkerTempAirspeedGauge
//
CWalkerTempAirspeedGauge::CWalkerTempAirspeedGauge (CPanel *mp)
: CBitmapGauge(mp)
{
}

int CWalkerTempAirspeedGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'tat_':
    ReadFrom (&tat, stream);
    rc = TAG_READ;
    break;

  case 'sat_':
    ReadFrom (&sat, stream);
    rc = TAG_READ;
    break;

  case 'tas_':
    ReadFrom (&tas, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}

//=======================================================================
// CGyroSlavingGauge
//=======================================================================
CGyroSlavingGauge::CGyroSlavingGauge (CPanel *mp)
: CBitmapGauge(mp)
{  lrgb_r = lrgb_g = lrgb_b = 0;
}
//-------------------------------------------------------------
CGyroSlavingGauge::~CGyroSlavingGauge (void)
{}
//-------------------------------------------------------------
//  Read the gauge tags
//-------------------------------------------------------------
int CGyroSlavingGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'nedl':
    andl.Read(stream,2);
    return TAG_READ;

  case 'slvb':
    aslv.Read(stream,2);
    return TAG_READ;

  case 'ccwb':
    acwb.Read(stream,2);
    return TAG_READ;

  case 'err_':
    ReadFrom (&err, stream);
    return TAG_READ;

  case 'slav':
    ReadFrom (&slav, stream);
    return TAG_READ;

  case 'free':
    ReadFrom (&free, stream);
    return TAG_READ;
  
  case 'mode':
    ReadFrom (&mode, stream);
    return TAG_READ;

  case 'ccw_':
    ReadFrom (&ccw, stream);
    return TAG_READ;

  case 'cw__':
    ReadFrom (&cw, stream);
    return TAG_READ;

  case 'lrgb':
    ReadInt (&lrgb_r, stream);
    ReadInt (&lrgb_g, stream);
    ReadInt (&lrgb_b, stream);
    return TAG_READ;

  case 'sfx_':
    GetSoundTag(GAUGE_ON__POS,stream);
    GetSoundTag(GAUGE_OFF_POS,stream);
    return TAG_READ;
  }
  return CBitmapGauge::Read (stream, tag);
}
//----------------------------------------------------------------------------
//  End of READ
//-----------------------------------------------------------------------------
void CGyroSlavingGauge::ReadFinished (void)
{
  CBitmapGauge::ReadFinished ();
  return;
}


//============================================================================
// CVerticalRateGauge
//============================================================================
CVerticalRateGauge::CVerticalRateGauge (CPanel *mp)
: CBitmapGauge(mp)
{
  avil = 0;
}

int CVerticalRateGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'eng_':
    ReadFrom (&eng, stream);
    rc = TAG_READ;
    break;

  case 'arm_':
    ReadFrom (&arm, stream);
    rc = TAG_READ;
    break;

  case 'knob':
    ReadFrom (&knob, stream);
    rc = TAG_READ;
    break;

  case 'led_':
    ReadFrom (&led, stream);
    rc = TAG_READ;
    break;

  case 'avil':
    ReadTag (&avil, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}

//
// CPropSyncGauge
//
CPropSyncGauge::CPropSyncGauge (CPanel *mp)
: CBitmapGauge(mp)
{
}

int CPropSyncGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}

//
// CDMEGauge
//
CDMEGauge::CDMEGauge (CPanel *mp)
: CBitmapGauge(mp)
{
  hold = false;
  dist_x = dist_y = 0;
  sped_x = sped_y = 0;
  time_x = time_y = 0;
  tiny = false;
}

int CDMEGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'hold':
    hold = true;
    rc = TAG_READ;
    break;

  case 'dist':
    ReadInt (&dist_x, stream);
    ReadInt (&dist_y, stream);
    rc = TAG_READ;
    break;

  case 'sped':
    ReadInt (&sped_x, stream);
    ReadInt (&sped_y, stream);
    rc = TAG_READ;
    break;

  case 'time':
    ReadInt (&time_x, stream);
    ReadInt (&time_y, stream);
    rc = TAG_READ;
    break;

  case 'knob':
    ReadFrom (&knob, stream);
    rc = TAG_READ;
    break;

  case 'tiny':
    tiny = true;
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}
//---------------------------------------------------------------
//  Mouse moves over
//---------------------------------------------------------------
ECursorResult CDMEGauge::MouseMoved (int x, int y)
{ // Send updated mouse position to all click areas
  if (knob.MouseMoved(x, y))  return CURSOR_WAS_CHANGED;
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//==================================================================
// CDME2Gauge
//==================================================================
CDME2Gauge::CDME2Gauge (CPanel *mp)
: CBitmapGauge(mp)
{
  hold = false;
  left_x = left_y = 0;
  rght_x = rght_y = 0;
}

int CDME2Gauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'hold':
    hold = true;
    rc = TAG_READ;
    break;

  case 'left':
    ReadInt (&left_x, stream);
    ReadInt (&left_y, stream);
    rc = TAG_READ;
    break;

  case 'rght':
    ReadInt (&rght_x, stream);
    ReadInt (&rght_y, stream);
    rc = TAG_READ;
    break;

  case 'ch__':
    ReadFrom (&ch, stream);
    rc = TAG_READ;
    break;

  case 'sel_':
    ReadFrom (&sel, stream);
    rc = TAG_READ;
    break;

  case 'pwr_':
    ReadFrom (&pwr, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}

//------------------------------------------------------------
//  Mouse moves over
//------------------------------------------------------------
ECursorResult CDME2Gauge::MouseMoved (int x, int y)
{  // Send updated mouse position to all click areas
  if ( ch.MouseMoved(x, y)) return CURSOR_WAS_CHANGED;
  if (sel.MouseMoved(x, y)) return CURSOR_WAS_CHANGED;
  if (pwr.MouseMoved(x, y)) return CURSOR_WAS_CHANGED;
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//=================================================================
// CWeatherRadarGauge
//==================================================================
CWeatherRadarGauge::CWeatherRadarGauge (CPanel *mp)
: CBitmapGauge(mp)
{;}

int CWeatherRadarGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'scrn':
    ReadFrom (&scrn, stream);
    rc = TAG_READ;
    break;

  case 'wx__':
    ReadFrom (&wx, stream);
    rc = TAG_READ;
    break;

  case 'vp__':
    ReadFrom (&vp, stream);
    rc = TAG_READ;
    break;

  case 'map_':
    ReadFrom (&map, stream);
    rc = TAG_READ;
    break;

  case 'nav_':
    ReadFrom (&nav, stream);
    rc = TAG_READ;
    break;

  case 'rng+':
    ReadFrom (&rngp, stream);
    rc = TAG_READ;
    break;

  case 'rng-':
    ReadFrom (&rngm, stream);
    rc = TAG_READ;
    break;

  case 'trk+':
    ReadFrom (&trkp, stream);
    rc = TAG_READ;
    break;

  case 'trk-':
    ReadFrom (&trkm, stream);
    rc = TAG_READ;
    break;

  case 'mode':
    ReadFrom (&mode, stream);
    rc = TAG_READ;
    break;

  case 'tilt':
    ReadFrom (&tilt, stream);
    rc = TAG_READ;
    break;

  case 'gain':
    ReadFrom (&gain, stream);
    rc = TAG_READ;
    break;

  case 'brit':
    ReadFrom (&brit, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}


//
// CAnalogAltPreselectGauge
//
CAnalogAltPreselectGauge::CAnalogAltPreselectGauge (CPanel *mp)
: CBitmapGauge(mp)
{
  strcpy (fdig_art, "");
  strcpy (digi_art, "");
  strcpy (zero_art, "");
  thou_x = thou_y = 0;
  hund_x = hund_y = 0;
  ones_x = ones_y = 0;
  dspc = 0;
  zspc = 0;
}

int CAnalogAltPreselectGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'knob':
    ReadFrom (&knob, stream);
    rc = TAG_READ;
    break;

  case 'fdig':
    ReadString (fdig_art, 64, stream);
    rc = TAG_READ;
    break;

  case 'digi':
    ReadString (digi_art, 64, stream);
    rc = TAG_READ;
    break;

  case 'zero':
    ReadString (zero_art, 64, stream);
    rc = TAG_READ;
    break;

  case 'thou':
    ReadInt (&thou_x, stream);
    ReadInt (&thou_y, stream);
    rc = TAG_READ;
    break;

  case 'hund':
    ReadInt (&hund_x, stream);
    ReadInt (&hund_y, stream);
    rc = TAG_READ;
    break;

  case 'ones':
    ReadInt (&ones_x, stream);
    ReadInt (&ones_y, stream);
    rc = TAG_READ;
    break;

  case 'dspc':
    ReadInt (&dspc, stream);
    rc = TAG_READ;
    break;

  case 'zspc':
    ReadInt (&zspc, stream);
    rc = TAG_READ;
    break;

  case 'aler':
    ReadFrom (&aler, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}


//======================================================================
// CMomentaryHotSpotGauge
//======================================================================

CMomentaryHotSpotGauge::CMomentaryHotSpotGauge (CPanel *mp)
: CBitmapGauge(mp)
{ Prop.Set(NO_SURFACE);
}
//----------------------------------------------------------------------
//  Read parameters 
//-----------------------------------------------------------------------
int CMomentaryHotSpotGauge::Read (SStream *stream, Tag tag)
{ return CBitmapGauge::Read (stream, tag);
}
//-----------------------------------------------------------------------
//    Read finished
//------------------------------------------------------------------------
void CMomentaryHotSpotGauge::ReadFinished (void)
{ CBitmapGauge::ReadFinished();
	//Init overall data
	mesg.id = MSG_SETDATA;
  // Dont Create drawing surface for the gauge
  return;
}
//-----------------------------------------------------------------------
//  Mouse click ??  Send message
//-----------------------------------------------------------------------
EClickResult CMomentaryHotSpotGauge::MouseClick (int x, int y, int buttons)
{ Send_Message(&mesg);
  return MOUSE_TRACKING_OFF;
}
//=============================================================================
// CLitInOutSwitch
//=============================================================================

CLitInOutSwitch::CLitInOutSwitch (CPanel *mp)
: CSimpleInOutStateSwitch(mp)
{}
//----------------------------------------------------------------
int CLitInOutSwitch::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'powr':
    ReadMessage (&powr, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CSimpleInOutStateSwitch::Read (stream, tag);
  }

  return rc;
}


//============================================================================
// CSimpleSwitch 'swit'
//  JS NOTES:  Some gauges uses  st8t for simple ON/OFF position
//             Multi position must use Indx as tag to change in swst
//============================================================================
CSimpleSwitch::CSimpleSwitch (CPanel *mp)
:	CGauge(mp)
{ // Cursor initialization
  csru_tag = csrd_tag = 0;

  stat_n     = 0;
  stat       = NULL;
  sstr_n     = 0;
  sstr       = NULL;
  orie       = 0;
  ordr       = 0;
  cIndx      = 0;
  oIndx      = -5;
  mmnt       = false;
  mntO       = false;
}
//-----------------------------------------------------------------------
//  Free all resources
//-----------------------------------------------------------------------
CSimpleSwitch::~CSimpleSwitch (void)
{
  // Delete bitmap
	SAFE_DELETE_ARRAY (stat);
  // Delete state string array
  if (sstr != NULL) {
    for (int i=0; i < sstr_n; i++) {
      SAFE_DELETE_ARRAY (sstr[i]);
    }
    SAFE_DELETE_ARRAY (sstr);
  }
}
//-----------------------------------------------------------------------
void CSimpleSwitch::PrepareMsg(CVehicleObject *veh)
{	CGauge::PrepareMsg (veh);
  //---Init the subsystem with current position ------
  mesg.intData	= stat[cIndx];
  Send_Message(&mesg);
}
//---------------------------------------------------------------------
//    Read gauge tags
//---------------------------------------------------------------------
int CSimpleSwitch::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'bmap':
    swit.Read(stream,0);
    return TAG_READ;

  case 'csrd':
  case 'csrl':
    csrd_tag = BindCursor(stream);
    return TAG_READ;

  case 'csru':
  case 'csrr':
    csru_tag = BindCursor(stream);
    return TAG_READ;

  case 'stat':
    {
      ReadInt (&stat_n, stream);
      if (stat_n > 16) {
        gtfo ("CSimpleSwitch : <stat> maximum 16 states exceeded");
      }
      stat = new int[stat_n];
      for (int i=0; i<stat_n; i++) {
        ReadInt (&stat[i], stream);
      }
    }
    return TAG_READ;

  case 'sstr':
    {
      ReadInt (&sstr_n, stream);
      if (sstr_n > 16) {
        gtfo ("CSimpleSwitch : <sstr> maximum 16 states exceeded");
      }
      sstr = new char*[sstr_n];
      for (int i=0; i<sstr_n; i++) {
        sstr[i] = new char[64];
        ReadString (sstr[i], 64, stream);
      }
    }
    return TAG_READ;

  case 'orie':
    ReadInt (&orie, stream);
    return TAG_READ;

  case 'ordr':
    ReadInt (&orie, stream);
    return TAG_READ;

  case 'indx':
    ReadInt (&cIndx, stream);
    return TAG_READ;

  case 'mmnt':
    mmnt = true;
    return TAG_READ;

  case 'mntO':
    mntO = true;
    return TAG_READ;
  }
  return CGauge::Read (stream, tag);
}
//---------------------------------------------------------------------
//  Read terminated
//---------------------------------------------------------------------
void CSimpleSwitch::ReadFinished (void)
{
  CGauge::ReadFinished ();
  lastP = -1;
  // Initialize default state
  if (stat_n == 0)
  { // No <stat> tag was specified, default to two states, 0 (off) and 1 (on)
    stat_n = 2;
    stat = new int[2];
    stat[0] =  0;
    stat[1] =  1;
  }
  //--Set message default values ---------------------
  //  JSNOTE: Gauge must define datatag  
  //---------------------------------------------------
  mesg.id			  = MSG_SETDATA;
  mesg.dataType = TYPE_INT;
  //----------------------------------------------------
  if (cIndx > stat_n) cIndx = stat_n - 1;
}
//--------------------------------------------------------------------------
//  Draw the switch
//--------------------------------------------------------------------------
void CSimpleSwitch::Draw (void)
{ //---Locate index by value ------------------------------
  if (lastP == cIndx) return;
  lastP = cIndx;
  // Draw appropriate bitmap frame
  EraseSurfaceRGBA (surf, 0);
  swit.Draw(surf, 0, 0, cIndx);
  return;
}

//-----------------------------------------------------------------------
// Increment switch state
//------------------------------------------------------------------------
void CSimpleSwitch::IncState (void)
{
  // Check upper bound and increment index
  if (cIndx < (stat_n - 1))
  { cIndx++;
    //---- Play sound effect -----------
    globals->snd->Play(sbuf[GAUGE_ON__POS]);
    //---- Set index value  -------------
    mesg.intData	= stat[cIndx];
    if (sstr) DisplayHelp(sstr[cIndx]);
    //---- Send message  ---------------
    Send_Message (&mesg); 
  }
  return;
}

//----------------------------------------------------------------------
// Decrement switch state
//----------------------------------------------------------------------
void CSimpleSwitch::DecState (void)
{ // Check upper bound and increment index
  if (cIndx > 0)
  { cIndx--;
    //---- Play sound effect ----------
    globals->snd->Play(sbuf[GAUGE_OFF_POS]);
    //---- Send index value- ----------
    mesg.intData	= stat[cIndx];
    if (sstr) DisplayHelp(sstr[cIndx]);
    //---- Send message ---------------
    Send_Message (&mesg); 
  }
  return;
}

//---------------------------------------------------------------
//  Mouse move
//--------------------------------------------------------------
ECursorResult CSimpleSwitch::MouseMoved (int mouseX, int mouseY)
{ DisplayHelp(help);
  switch (orie) {
  case 0:
  //------- Default, vertical orientation -------
  case 1:
    // Upper half of gauge increments the state
    if (mouseY < cy) return globals->cum->SetCursor (csru_tag);
    // Bottom half of the gauge decrements the state
    if (mouseY > cy) return globals->cum->SetCursor (csrd_tag);
    return CURSOR_WAS_CHANGED;
  //------ Horizontal orientation ----------------
  case 2:
    // Left half of the gauge decrements the state
    if (mouseX < cx) return globals->cum->SetCursor (csrd_tag);
    // Right half of the gauge increments the state
    if (mouseX > cx) return globals->cum->SetCursor (csru_tag);
    return CURSOR_WAS_CHANGED;
  }
  //-----------------------------------------------
  return CURSOR_WAS_CHANGED;
}
//---------------------------------------------------------------
//  Mouse Click
//--------------------------------------------------------------
EClickResult CSimpleSwitch::MouseClick (int mouseX, int mouseY, int buttons)
{ switch (orie) {
  case 0:
  case 1:
    // Default, vertical orientation
    (mouseY < cy)?(IncState ()):(DecState ());
    return MOUSE_TRACKING_OFF;

  case 2:
    // Horizontal orientation
    (mouseX < cx)?(IncState ()):(DecState ());
    return MOUSE_TRACKING_OFF;
  }
  return MOUSE_TRACKING_ON;
}

//===================================================================
// CSimpleInOutStateSwitch
//===================================================================

CSimpleInOutStateSwitch::CSimpleInOutStateSwitch (CPanel *mp)
: CBitmapGauge(mp)
{ type = GAUGE_SIMPLE_IN_OUT;
  vin[1] = 1;
  vin[0] = 0;
  stat   = 0;
  mack   = 0;
  strcpy (onht, "");
  strcpy (ofht, "");
  strcpy (sstr_off, "");
  strcpy (sstr_on, "");
  mmnt  = 0;
  time  = 0;
  chng  = 1;
}

CSimpleInOutStateSwitch::~CSimpleInOutStateSwitch (void)
{}
//-----------------------------------------------------------------------------
//  Read gauge tags
//-----------------------------------------------------------------------------

int CSimpleInOutStateSwitch::Read (SStream *stream, Tag tag)
{ int nbr;
  int rc = TAG_IGNORED;
  switch (tag) {
  case 'sbmp':
    stsw.Read(stream,0);
    return TAG_READ;

  case 'onvl':
  case 'in__':
  case 'on__':
    ReadInt (&nbr, stream);
    vin[1] = nbr;
    return TAG_READ;

  case 'ofvl':
  case 'off_':
  case 'out_':
    ReadInt (&nbr, stream);
    vin[0] = nbr;
    return TAG_READ;

  case 'stat':
  case 'st8t':
    ReadInt ((int*)&stat, stream);
    return TAG_READ;

  case 'onht':
    ReadString (onht, 64, stream);
    return TAG_READ;

  case 'ofht':
    ReadString (ofht, 64, stream);
    return TAG_READ;

  case 'sstr':
    ReadString (sstr_off, 64, stream);
    ReadString (sstr_on, 64, stream);
    return TAG_READ;

  case 'mmnt':
    //--- This value may be override by the susbsystem -------------
    mmnt = 20;
    return TAG_READ;

  case 'mack':
    //--- must be acknoledged -----------
    mack = 1;
    return TAG_READ;
  }
  return CBitmapGauge::Read (stream, tag);
}
//-----------------------------------------------------------------------------
//  Read finished
//-----------------------------------------------------------------------------
void CSimpleInOutStateSwitch::ReadFinished (void)
{ CGauge::ReadFinished ();
  //---Init message values -----------------------------
  mesg.id       = MSG_SETDATA;
  mesg.dataType = TYPE_INT;
  //---- Send update message to dependent subsystem
  return;
}
//-----------------------------------------------------------------------------
//  Send message to init the subsystem
//-----------------------------------------------------------------------------
void  CSimpleInOutStateSwitch::PrepareMsg(CVehicleObject *veh)
{ Tag tag = mesg.user.u.datatag;
  mesg.intData  = vin[stat];
  Send_Message (&mesg);
  if (0 == sync)      return;
  mesg.user.u.datatag = 'gets';
  Send_Message(&mesg);
	subS	= (CSubsystem*) mesg.voidData;
  mesg.user.u.datatag = tag;
  return;
}
//-----------------------------------------------------------------------------
//  Draw the change
//-----------------------------------------------------------------------------
void CSimpleInOutStateSwitch::DrawChange()
{ chng = 0;
  EraseSurfaceRGBA(surf,0);
  stsw.Draw(surf, 0, 0, stat);
  mesg.intData  = vin[stat];
  Send_Message (&mesg);
  return;
}
//-----------------------------------------------------------------------------
//  Check for persistence
//-----------------------------------------------------------------------------
void CSimpleInOutStateSwitch::CheckHold()
{ //---Check state if acknoledged ------------
  mesg.id = MSG_GETDATA;
  Send_Message(&mesg);
  mesg.id = MSG_SETDATA;
  int old = stat;
  stat    = mesg.intData;
  chng    = (old != stat);
  return;
}
//-----------------------------------------------------------------------------
//  Draw the gauge
//-----------------------------------------------------------------------------
void CSimpleInOutStateSwitch::Draw (void)
{ subS->SetGauge(this);
	if (chng)           DrawChange();
  if (mack)           CheckHold();
  if (0 == stat)      return;
  //---Check for time out -------------------
  if (0 == mmnt)      return;
  if (0 == time)      return;
  time--;
  if (time)           return;
  stat = 0; 
  chng = 1;
  return;
  }
//-------------------------------------------------------------------------------------
//  Check for change
//-------------------------------------------------------------------------------------
void CSimpleInOutStateSwitch::SetChange()
{ if (mack && (mesg.result != MSG_ACK))  return;
  chng = 1;
  stat = !stat;
  //---- Play sound effect --------------------
  globals->snd->Play(sbuf[stat]);
  //--- Arm timer for momentary contact -------
  if (stat)  time = mmnt;
  return;
}
//-------------------------------------------------------------------------------------
//  Mouse click over
//  JS: We must stop to implement Hack for every plane.
//                       Change the plane instead!!!!!
//-------------------------------------------------------------------------------------
EClickResult CSimpleInOutStateSwitch::MouseClick (int mouseX, int mouseY, int buttons)
{ //---- Send update message to dependent subsystem -------------
  //--- Check for acknowledge -----------------------------------
  SetChange(); 
  return MOUSE_TRACKING_OFF;
}
//================================================================================
// CBasicBatterySwitch
//================================================================================
CBasicBatterySwitch::CBasicBatterySwitch (CPanel *mp)
: CSimpleSwitch(mp)
{
  altt = batt = 0;
  sAlt = sBat = 0;
}
//------------------------------------------------------------
//	Prepare messages
//------------------------------------------------------------
void CBasicBatterySwitch::PrepareMsg(CVehicleObject *veh)
{	mbat.id		      = MSG_GETDATA;
  mbat.sender     = unId;
  mbat.user.u.hw  = HW_SWITCH;
	veh->FindReceiver(&mbat);
	malt.id		      = MSG_GETDATA;
  malt.sender     = unId;
  malt.user.u.hw  = HW_SWITCH;
	veh->FindReceiver(&malt);
	CSimpleSwitch::PrepareMsg(veh);
	return;
}
//-----------------------------------------------------------
//  Read all tags
//-----------------------------------------------------------
int CBasicBatterySwitch::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;
  Tag t;

  switch (tag) {
  case 'altt':
    ReadTag (&t, stream);
    malt.group = (unsigned long) t;
    return TAG_READ;

  case 'batt':
    ReadTag (&t, stream);
    mbat.group = (unsigned long) t;
    return TAG_READ;
  }
  return CSimpleSwitch::Read (stream, tag);
}
//---------------------------------------------------------------
//  All parameters are read. Init messages
//---------------------------------------------------------------
void  CBasicBatterySwitch::ReadFinished()
{ CSimpleSwitch::ReadFinished();
 //--- Message to Battery -----------------
  mbat.id             = MSG_SETDATA;
  mbat.sender         = unId;
  mbat.dataType       = TYPE_INT;
  mbat.user.u.datatag = 'stat';
  //--- Message to Alternator -------------
  malt.id             = MSG_SETDATA;
  malt.sender         = unId;
  malt.dataType       = TYPE_INT;
  malt.user.u.datatag = 'stat';
  return;
}
//---------------------------------------------------------------
//  Draw the gauge
//---------------------------------------------------------------
void CBasicBatterySwitch::Draw (void)
{
  // Get battery and alternator states
  Send_Message (&mbat);
  sBat      = mbat.intData & 0x01;

  Send_Message (&malt);
  sAlt      = malt.intData & 0x01;

  // Calculate frame number:
  //   0 = Both OFF
  //   1 = Alternator ON, Battery OFF
  //   2 = Alternator OFF, Battery ON
  //   3 = Both ON
  int frame = sAlt + sBat + sBat;
  // Draw appropriate bitmap frame
  swit.Draw(surf, 0, 0, frame);
  return;
}
//------------------------------------------------------------------------------
//  Mouse click
//------------------------------------------------------------------------------
EClickResult CBasicBatterySwitch::MouseClick (int mouseX, int mouseY, int buttons)
{
  EClickResult rc = MOUSE_TRACKING_OFF;
  char mod = 0;
  char bat = sBat;
  char alt = sAlt;
  if (mouseX < cx) {
    // Left half of the switch controls the alternator
    sAlt  = (mouseY < (cy))?(1):(0);
    sBat |= sAlt;
  }

  if (mouseX > cx) {
    // Right half of the switch controls the battery
    sBat = (mouseY < (cy))?(1):(0);
    }
  mod = ((sAlt ^ alt) | (sBat ^ bat));
    // Play sound effect if alternator state has changed
  if (mod) globals->snd->Play(sbuf[mod]);

  // Update battery and alternator subsystems
  mbat.intData  = int(sBat);
  malt.intData  = int(sAlt);

  return rc;
}
//==============================================================================
// CBasicMagnetoSwitch
//
// This gauge type implements a basic turn-switch for magnetos.  The switch
//   has five positions:
//   0 = off 
//   1 = Right
//   2 = Left
//   3 = Both
//   4 = Start (momentary contact)
//
// Clicking the right mouse button increments the gauge state, and clicking the
//   left button decrements the state.
//
// This gauge is intended to be connected to a rotary ignition subsystem ('rign')
//

//  typedef enum {
//    MAGNETO_SWITCH_OFF    = 0,
//    MAGNETO_SWITCH_RIGHT  = 1,
//    MAGNETO_SWITCH_LEFT   = 2,
//    MAGNETO_SWITCH_BOTH   = 3,
//    MAGNETO_SWITCH_START  = 4
//  } EMagnetoSwitch;
//=========================================================================
CBasicMagnetoSwitch::CBasicMagnetoSwitch (CPanel *mp)
: CSimpleSwitch(mp)
{
}
//-----------------------------------------------------------------------
//  Draw the gauge
//  cIndx is the current state
//-----------------------------------------------------------------------
void CBasicMagnetoSwitch::Draw (void)
{
  // draw only if required
  if (oIndx == cIndx) return; 

  // Draw appropriate bitmap frame
  EraseSurfaceRGBA(surf,0);
  swit.Draw(surf, 0, 0, cIndx);
  oIndx = cIndx;
  return; 
}
//-------------------------------------------------------------------------------
//  Mouse Click
//  NOTE:  Prevent forward position after the last frame
//        Some start key uses only 4 positions.  The last one is the start position
//        
//---------------------------------------------------------------------------------
EClickResult CBasicMagnetoSwitch::MouseClick (int mouseX, int mouseY, int buttons)
{
  EClickResult rc = MOUSE_TRACKING_OFF;
  if ((buttons & MOUSE_BUTTON_LEFT) &&  (cIndx > MAGNETO_SWITCH_OFF))
  {// Decrement state 
    cIndx--;
    globals->snd->Play(sbuf[GAUGE_OFF_POS]);
  }
  if ((buttons & MOUSE_BUTTON_RIGHT) && (cIndx < swit.GetHiFrame()))
  {// Increment state 
    cIndx++;
    globals->snd->Play(sbuf[GAUGE_ON__POS]);
  }

  // Set position to start if last frame is reached ----
  int pos = cIndx;
  if (cIndx == swit.GetHiFrame()) 
      { rc  = MOUSE_TRACKING_ON;
        pos = MAGNETO_SWITCH_START;
      }
  // Update magneto subsystem
  mesg.id       = MSG_SETDATA;
  mesg.dataType = TYPE_INT;
  mesg.intData  = pos;
  Send_Message (&mesg);
  return rc;
}
//-------------------------------------------------------------------------------
//  Mouse move over
//-------------------------------------------------------------------------------
ECursorResult CBasicMagnetoSwitch::MouseMoved (int x, int y)
{ globals->cum->SetCursor(cursTag);
  return  CURSOR_WAS_CHANGED;}
//----------------------------------------------------------------------------
//  Stop Click
//-----------------------------------------------------------------------------
EClickResult CBasicMagnetoSwitch::StopClick()
{
  // The only time this method should be called is when the switch is in the
  //   START state and the right mouse button is released.  Switch back
  //   to previous state
  cIndx -= 1;
  globals->snd->Play(sbuf[GAUGE_OFF_POS]);
  // Update magneto subsystem
  mesg.id = MSG_SETDATA;
  mesg.dataType = TYPE_INT;
  mesg.intData  = cIndx;
  Send_Message (&mesg);
  return MOUSE_TRACKING_OFF;
}


//
// CTwoStateELTGauge
//

CTwoStateELTGauge::CTwoStateELTGauge (CPanel *mp)
: CSimpleSwitch(mp)
{
}

int CTwoStateELTGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc == TAG_IGNORED) {
    rc = CSimpleSwitch::Read (stream, tag);
  }

  return rc;
}


//
// CSwitchPairGauge
//

CSwitchPairGauge::CSwitchPairGauge (CPanel *mp)
: CSimpleSwitch(mp)
{}
//------------------------------------------------------
//	Prepare messages
//------------------------------------------------------
void CSwitchPairGauge::PrepareMsg(CVehicleObject *veh)
{	s1ms.id		  = MSG_GETDATA;
  s1ms.sender = unId;
	veh->FindReceiver(&s1ms);
	s2ms.id		  = MSG_GETDATA;
  s2ms.sender = unId;
	veh->FindReceiver(&s2ms);
	CSimpleSwitch::PrepareMsg(veh);
	return;
}
int CSwitchPairGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 's1ms':
    ReadMessage (&s1ms, stream);
    rc = TAG_READ;
    break;

  case 's2ms':
    ReadMessage (&s2ms, stream);
    rc = TAG_READ;
    break;

  case 's1of':
    ReadFrom (&s1of, stream);
    rc = TAG_READ;
    break;

  case 's1on':
    ReadFrom (&s1on, stream);
    rc = TAG_READ;
    break;

  case 's2on':
    ReadFrom (&s2on, stream);
    rc = TAG_READ;
    break;

  case 's2of':
    ReadFrom (&s2of, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CSimpleSwitch::Read (stream, tag);
  }

  return rc;
}
//---------------------------------------------------------------
//  Mouse moves over
//---------------------------------------------------------------
ECursorResult CSwitchPairGauge::MouseMoved (int x, int y)
{ // Send updated mouse position to all click areas
  if (s1of.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  if (s1on.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  if (s2of.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  if (s2on.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//=======================================================================
// CSimpleFloatSwitch
//=======================================================================

CSimpleFloatSwitch::CSimpleFloatSwitch (CPanel *mp)
: CGauge(mp)
{
  strcpy (bmap_art, "");
  csru_tag = csrd_tag = 0;
  stat_n = 0;
  stat = NULL;
  sstr_n = 0;
  sstr = NULL;
  mmnt = false;
}

CSimpleFloatSwitch::~CSimpleFloatSwitch (void)
{
  if (stat != NULL) delete[] stat;
  if (sstr != NULL) {
    for (int i=0; i<sstr_n; i++) {
      delete[] sstr[i];
    }
    delete[] sstr;
  }
}
//--------------------------------------------------------------------------------
//  Read gauge tags
//--------------------------------------------------------------------------------
int CSimpleFloatSwitch::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'bmap':
    ReadString (bmap_art, 64, stream);
    return TAG_READ;

  case 'csru':
    csru_tag = BindCursor(stream);
    return TAG_READ;

  case 'csrd':
    csrd_tag = BindCursor(stream);
    return TAG_READ;

  case 'stat':
    {
      ReadInt (&stat_n, stream);
      if (stat_n > 16) {
        gtfo ("CSimpleFloatSwitch : <stat> maximum 16 states exceeded");
      }
      stat = new float[stat_n];
      for (int i=0; i<stat_n; i++) {
        ReadFloat (&stat[i], stream);
      }
    }
    return TAG_READ;

  case 'sstr':
    {
      ReadInt (&sstr_n, stream);
      if (sstr_n > 16) {
        gtfo ("CSimpleFloatSwitchGauge : <sstr> maximum 16 states exceeded");
      }
      sstr = new char*[sstr_n];
      for (int i=0; i<sstr_n; i++) {
        sstr[i] = new char[64];
        ReadString (sstr[i], 64, stream);
      }
    }
    return TAG_READ;

  case 'mmnt':
    {
      int i;
      ReadInt (&i, stream);
      mmnt = (i != 0);
    }
    return TAG_READ;
  }
  return CGauge::Read (stream, tag);
}
//--------------------------------------------------------------------------------
//  Read finished
//--------------------------------------------------------------------------------
void CSimpleFloatSwitch::ReadFinished (void)
{
  CGauge::ReadFinished ();
  return;
}

//=================================================================================
// CDualSwitch
//==================================================================================
CDualSwitch::CDualSwitch (CPanel *mp)
: CSimpleSwitch(mp)
{
  nums = 0;
  orie = 0;
  mmnt = false;
  dswi_timer = 0;
}

CDualSwitch::~CDualSwitch (void)
{
//  CSimpleSwitch::~CSimpleSwitch ();
}
//--------------------------------------------------------------------------
//	JSDEV* prepare messages
//---------------------------------------------------------------------------
void CDualSwitch::PrepareMsg(CVehicleObject *veh)
{	
  usrl.id		  = MSG_GETDATA;
  usrl.sender = unId;
	veh->FindReceiver(&usrl);
	usrr.id		  = MSG_GETDATA;
  usrr.sender = unId;
	veh->FindReceiver(&usrr);
	CSimpleSwitch::PrepareMsg(veh);
}

int CDualSwitch::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {

  case 'nums':
    ReadInt (&nums, stream);
    if (nums != 3) {
      WARNINGLOG ("CDualSwitch::Read nums not 3");
      /// \todo : we assume it's always 3
      nums = 3;
    }
    rc = TAG_READ;
    break;

  case 'orie':
    ReadInt (&orie, stream);
    DEBUGLOG ("dwsi = %d", orie);
    rc = TAG_READ;

    break;

  case 'usrl':
    ReadMessage (&usrl, stream);
    rc = TAG_READ;
    break;

  case 'usrr':
    ReadMessage (&usrr, stream);
    rc = TAG_READ;
    break;

  case 'mmnt': /// ! NO VALUE
    //{
    //  int i;
    //  ReadInt (&i, stream);
    //  mmnt = (i != 0);
    //}
    mmnt = true;
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CSimpleSwitch::Read (stream, tag);
  }

  return rc;
}

//---------------------------------------------------------------------
//  Read terminated
//---------------------------------------------------------------------
void CDualSwitch::ReadFinished (void)
{
  CSimpleSwitch::ReadFinished ();
  stat_n = nums;
  cIndx =  1;
  oIndx = -1;
  //----Init message -------------------------
  usrr.id			  = MSG_SETDATA;
  usrr.dataType = TYPE_INT;
  usrr.user.u.datatag = 'st8t';
  return;
}
//--------------------------------------------------------------------------
//  Draw the switch
//--------------------------------------------------------------------------
void CDualSwitch::Draw (void)
{
  // CSimpleSwitch::Draw ();

  // Draw appropriate bitmap frame
  // if situation has changed
  if (cIndx != oIndx) {
    oIndx = cIndx;
    EraseSurfaceRGBA (surf, 0);
    swit.Draw(surf, 0, 0, cIndx);
  }

  if (mmnt &&  dswi_timer) {
    if (cIndx == 0) {
      DecState ();
       dswi_timer--;
       if (0 == dswi_timer)
         IncState ();
    } else 
      if (cIndx == 2) {
        IncState ();
        dswi_timer--;
        if (0 == dswi_timer)
          DecState ();
      }
  }
}

//-----------------------------------------------------------------------
// Increment switch state
//------------------------------------------------------------------------
void CDualSwitch::IncState (void)
{
  // CSimpleSwitch::IncState ();
  switch(cIndx) {
  // Check upper bound and increment index
    case 1:
    { cIndx = 2;
      oIndx = 1; 
      // Play sound effect
      globals->snd->Play(sbuf[GAUGE_ON__POS]);
      // Send message
      usrr.intData	= 1;
      Send_Message (&usrr);
      return;
    }
    case 0:
    { cIndx = 1;
      oIndx = 0;
      // Play sound effect
      globals->snd->Play(sbuf[GAUGE_OFF_POS]);
      // Send message
      usrl.intData	= 0;
      Send_Message (&usrl);
      return;
    }
    case 2:
    { cIndx = 2;
      oIndx = 2;
      // Send message
      usrr.intData	= 1;
      Send_Message (&usrr);
      return;
    }
  }
  return;
}

//----------------------------------------------------------------------
// Decrement switch state
//----------------------------------------------------------------------
void CDualSwitch::DecState (void)
{ 
  // CSimpleSwitch::DecState ();
  switch (cIndx)  {
    case 1:
    // Check upper bound and increment index
    { cIndx = 0;
      oIndx = 1;
      // Play sound effect
      globals->snd->Play(sbuf[GAUGE_ON__POS]);
      // Send message
      usrl.intData	= 1;
      Send_Message (&usrl);
      return;
    } 
    case 2:
    { cIndx = 1;
      oIndx = 2;
      // Play sound effect
      globals->snd->Play(sbuf[GAUGE_OFF_POS]);
      // Send message
      usrr.intData	= 0;
      Send_Message (&usrr);
      return;
    }
    case 0:
    { cIndx = 0;
      oIndx = 0;
      // Send message
      usrl.intData	= 1;
      Send_Message (&usrl);
      return;
    } 
  }
  return;
}

//---------------------------------------------------------------
//  Mouse move
//--------------------------------------------------------------
ECursorResult CDualSwitch::MouseMoved (int mouseX, int mouseY)
{
  DisplayHelp(help);

  switch (orie) {
  case 0:
  // Default, vertical orientation
  case 1:
    // Upper part of gauge
    if (mouseY < cy) return globals->cum->SetCursor(csru_tag);
    // Bottom part of the gauge
    if (mouseY > cy) return globals->cum->SetCursor(csrd_tag);
    return CURSOR_WAS_CHANGED;
  // Horizontal orientation
  case 2:
    // Left part of the gauge
    if (mouseX < cx) return globals->cum->SetCursor(csrd_tag);
    // Right part of the gauge
    if (mouseX > cx) return globals->cum->SetCursor(csru_tag);
    return CURSOR_WAS_CHANGED;
  }
  //---------------------------------------------------
  return CURSOR_WAS_CHANGED;
}
//---------------------------------------------------------------
//  Mouse Click
//--------------------------------------------------------------
EClickResult CDualSwitch::MouseClick (int mouseX, int mouseY, int buttons)
{ switch (orie) {
  case 0:
  case 1:
    // Default, vertical orientation
    if (mouseY < cy)   {IncState (); dswi_timer = 200;} // see crank delay 50
    if (mouseY > cy)   {DecState (); dswi_timer = 200;} // see crank delay 50
    DisplayHelp(sstr[cIndx]);
    return MOUSE_TRACKING_OFF;

  case 2:
    // Horizontal orientation
    if (mouseX < cx)    {DecState (); dswi_timer = 200;} // see crank delay 50
    if (mouseX > cx)    {IncState (); dswi_timer = 200;} // see crank delay 50
    DisplayHelp(sstr[cIndx]);
    return MOUSE_TRACKING_OFF;
  }
  return MOUSE_TRACKING_ON;
}
//=======================================================================
// CPushPullKnobGauge
//=======================================================================
CPushPullKnobGauge::CPushPullKnobGauge (CPanel *mp)
: CGauge(mp)
{ lnk1.user.u.datatag = 'lnk1';
  polm.user.u.datatag = 'mpol';
  ytop = ybtm = 0;
  lowv = 0;
  high = 1;

  cVal = 0;
  nframe = 0;
  nVal = 0;
  Area = 0;
  invt = 0;
}
//-----------------------------------------------------------
//  Destructor
//-----------------------------------------------------------
CPushPullKnobGauge::~CPushPullKnobGauge (void)
{ if (Area) delete [] Area;
}
//-----------------------------------------------------------
//	Prepare messages
//------------------------------------------------------------
void CPushPullKnobGauge::PrepareMsg(CVehicleObject *veh)
{	mesg.id		  = MSG_SETDATA;
  mesg.sender = unId;
	veh->FindReceiver(&mesg);
	lnk1.id		  = MSG_SETDATA;
  lnk1.sender = unId;
	veh->FindReceiver(&lnk1);
	CGauge::PrepareMsg(veh);
	return;
}
//------------------------------------------------------------
//  Read parameters
//------------------------------------------------------------
int CPushPullKnobGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'bmap':
    pknob.Read(stream,0);
    return TAG_READ;

  case 'lnk1':
    ReadMessage (&lnk1, stream);
    return TAG_READ;

  case 'ytop':
    ReadInt (&ytop, stream);
    return TAG_READ;

  case 'ybtm':
    ReadInt (&ybtm, stream);
    return TAG_READ;

  case 'valu':
    CreateArea(stream);
    return TAG_READ;

  case 'lowv':
    ReadFloat (&lowv, stream);
    return TAG_READ;

  case 'high':
    ReadFloat (&high, stream);
    return TAG_READ;

  case 'poll':
    return TAG_READ;
  }
  return CGauge::Read (stream, tag);
}
//---------------------------------------------------------------------
//  Abort
//---------------------------------------------------------------------
void CPushPullKnobGauge::Err01()
{ gtfo ("CPushPullKnobGauge : <valu> maximum 32 values exceeded in PNL file unId=%s",
  GetUniqueID ());

}
//---------------------------------------------------------------------
//  Create click area
//----------------------------------------------------------------------
void CPushPullKnobGauge::CreateArea(SStream *s)
{ ReadInt (&nVal, s);
  if (nVal > 32) Err01();
  Area  = new SKnobArea[nVal];
  for (int k=0; k<nVal; k++)
  { ReadInt    (&Area[k].ytop, s);
    ReadInt    (&Area[k].ybtm, s);
    ReadFloat  (&Area[k].valu, s);
    ReadString ( Area[k].help, 64, s);
    Area[k].fram = k;
   }
  cVal  = 0;
  wait  = 60;
  return;
}
//---------------------------------------------------------------------
//  Check when bitmap at ratio r
//---------------------------------------------------------------------
bool CPushPullKnobGauge::AtRatio(char k)
{ float tg = float(k) * 0.01;
  float ac = float(cVal) / nVal;
  float df = (tg - ac);
  float mn = float(1)    / nVal;
  return (abs(df) <= mn);
}
//---------------------------------------------------------------------
// Create default area
//  From bottom to top of gauge (in pixel)
//  Thus the value are sorted in reverse order
//---------------------------------------------------------------------
void CPushPullKnobGauge::DefaultArea()
{ int xs = 0, ys = 0;
  pknob.GetBitmapSize(&xs, &ys);
  float htr = float(ys);
  // define valu data
  nVal = pknob.GetNbFrame();
  if (nVal > 32) Err01();
  Area  = new SKnobArea[nVal];
  int   htf  = int(htr / nVal);
  for (int k = 0; k < nVal ; k++)
  { float top = (htr * k)  / nVal;
    Area[k].ytop = int(top);
    Area[k].ybtm = int(top) + htf;
    Area[k].valu = 1.0f - (float(k) / nVal);
    Area[k].fram = (nVal - 1 - k);
    strcpy (Area[k].help, "");
  }
  //---Start at Zero -----------------------
  cVal = nVal - 1;
  wait =  1;
  invt =  1;
  return;
}
//---------------------------------------------------------------------
//  All parameters are read
//----------------------------------------------------------------------
void CPushPullKnobGauge::ReadFinished (void)
{ CGauge::ReadFinished ();
  if (0 == nVal)  DefaultArea();
  //---Init message ---------------------------------
  mesg.id       = MSG_SETDATA;
  mesg.dataType = TYPE_REAL;
  //-------------------------------------------------
  polm.group    = mesg.group;
  polm.sender   = unId;
  polm.id       = MSG_GETDATA;
  polm.dataType = TYPE_REAL;
  polm.user.u.datatag = mesg.user.u.datatag;
  return;
}
//---------------------------------------------------------------------
// Look up for value to get the position
//---------------------------------------------------------------------
void CPushPullKnobGauge::LookUpValue(float val)
{ for (cVal=0; cVal != nVal; cVal++)
  { float vtb = Area[cVal].valu;
    bool cont = (invt)?(val < vtb):(val > vtb);
    if (cont)  continue;
    return;
  }
  cVal--;
  return;
}
//---------------------------------------------------------------------
//  Draw the gauge
//  NOTE:  get the value from the subsystem in case joystick change
//         the control
//---------------------------------------------------------------------
void CPushPullKnobGauge::Draw (void)
{ Send_Message(&polm);
  float val = polm.realData;
  LookUpValue(val);
  EraseSurfaceRGBA(surf,0);
  pknob.Draw(surf, 0, 0, Area[cVal].fram);
}
//---------------------------------------------------------------------
// Increment knob value
//---------------------------------------------------------------------
void CPushPullKnobGauge::IncValue (void)
{   cVal ++;
    if (cVal >= nVal) cVal = nVal - 1;
    //---- Send message ----------------------
    mesg.realData = Area[cVal].valu;
    Send_Message (&mesg);
    return;
}

//---------------------------------------------------------------------
// Decrement knob value
//---------------------------------------------------------------------
void CPushPullKnobGauge::DecValue (void)
{   cVal --;
    if (cVal < 0) cVal = 0;
    //---- Send message ---------------------
    mesg.realData = Area[cVal].valu;
    Send_Message (&mesg);
    return;
}
//---------------------------------------------------------------------
//  Display help 
//---------------------------------------------------------------------
ECursorResult CPushPullKnobGauge::DisplayRatio()
{ sprintf_s(hbuf,HELP_SIZE,"%.0f%%",mesg.realData * 100.0f);
  FuiHelp();
  return globals->cum->SetCursor(cursTag);
}
//---------------------------------------------------------------------
//  Mouse moves over
//---------------------------------------------------------------------
ECursorResult CPushPullKnobGauge::MouseMoved (int mouseX, int mouseY)
{ return globals->cum->SetCursor(cursTag);
}
//---------------------------------------------------------------------
//  Mouse click
//---------------------------------------------------------------------
EClickResult CPushPullKnobGauge::MouseClick (int mX, int mY, int but)
{ globals->cum->SetCursor(cursTag); 
  if (mY > Area[cVal].ybtm)
  { IncValue();
    time = wait;
    return MOUSE_TRACKING_ON;
   } 
  if (mY < Area[cVal].ytop)
  { DecValue();
    time = wait;
    return MOUSE_TRACKING_ON;
   }
  return MOUSE_TRACKING_OFF;
}
//---------------------------------------------------------------------
// Track click:  wait before repeating click
//---------------------------------------------------------------------
EClickResult CPushPullKnobGauge::TrackClick (int mX, int mY, int but)
{ if (0 == time)  return MouseClick(mX, mY, but);
  time--;
  return MOUSE_TRACKING_ON;
}
//---------------------------------------------------------------------
// Stop click
//---------------------------------------------------------------------
EClickResult  CPushPullKnobGauge::StopClick()
{ time = 0;
  return MOUSE_TRACKING_OFF;
}

//========================================================================
// CPrimerKnobGauge
//========================================================================

CPrimerKnobGauge::CPrimerKnobGauge (CPanel *mp)
: CPushPullKnobGauge(mp)
{
}

int CPrimerKnobGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc == TAG_IGNORED) {
    rc = CPushPullKnobGauge::Read (stream, tag);
  }

  return rc;

}

//=================================================================================
// CFlapsSwitchGauge:  Just a CPushPullKnobGauge
//
//==================================================================================
CFlapsSwitchGauge::CFlapsSwitchGauge (CPanel *mp)
:	CPushPullKnobGauge(mp)
{
  //deflect = step = 0.0f;

  mesg.user.u.datatag = 101; ///< send data to CAeroControl::ReceiveMessage

}

int CFlapsSwitchGauge::Read (SStream *stream, Tag tag)
{
  return CPushPullKnobGauge::Read (stream, tag);
}

//==================================================================================
// CLitLandingGearKnobGauge
//==================================================================================

CLitLandingGearKnobGauge::CLitLandingGearKnobGauge (CPanel *mp)
:	CPushPullKnobGauge(mp)
{
}

int CLitLandingGearKnobGauge::Read (SStream *stream, Tag tag)
{ int rc = TAG_IGNORED;
  if (rc == TAG_IGNORED) {
    rc = CPushPullKnobGauge::Read (stream, tag);
  }
  return rc;
}
//============================================================================
// CReversibleThrottleKnobGauge
//============================================================================

CReversibleThrottleKnobGauge::CReversibleThrottleKnobGauge (CPanel *mp)
:	CPushPullKnobGauge(mp)
{ strcpy (rbmp_art, "");
  rtop = rbtm = 0;
}

int CReversibleThrottleKnobGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'rvsr':
    ReadMessage (&rvsr, stream);
    rc = TAG_READ;
    break;

  case 'rbmp':
    ReadString (rbmp_art, 64, stream);
    rc = TAG_READ;
    break;

  case 'rtop':
    ReadInt (&rtop, stream);
    rc = TAG_READ;
    break;

  case 'rbtm':
    ReadInt (&rbtm, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CPushPullKnobGauge::Read (stream, tag);
  }

  return rc;
}


//=======================================================================
// CDualKnobGauge
//=======================================================================
CDualKnobGauge::CDualKnobGauge (CPanel *mp)
: CGauge(mp)
{ iinc = +0.01f;
  idec = -0.01f;
  oinc = +0.01f;
  odec = -0.01f;
	inca.SetNoClick();
	ouca.SetNoClick();
}
//-----------------------------------------------------------------------
//	Prepare messages
//-----------------------------------------------------------------------
void CDualKnobGauge::PrepareMsg(CVehicleObject *veh)
{	ivar.msg.id		  = MSG_SETDATA;
	veh->FindReceiver(&ivar.msg);
	ovar.msg.id		  = MSG_SETDATA;
	veh->FindReceiver(&ovar.msg);
	CGauge::PrepareMsg(veh);
	return;
}
//-----------------------------------------------------------------------
//	Prepare messages
//-----------------------------------------------------------------------
int CDualKnobGauge::Warn01(Tag tag)
{ WARNINGLOG("Tag <%> not used anymore. Change to <mesg>",TagToString(tag));
  return TAG_IGNORED;
}
//------------------------------------------------------------------------
//  Read parameters
//------------------------------------------------------------------------
int CDualKnobGauge::Read (SStream *stream, Tag tag)
{ float p1;
  switch (tag) {
  case 'imsg':
    ReadMessage(&ivar.msg,stream);
    return TAG_READ;

  case 'usri':
    // DEPRECATED
    return Warn01(tag);

  case 'igrp':
    // DEPRECATED
    return Warn01(tag);

  case 'omsg':
    ReadMessage (&ovar.msg, stream);
    return TAG_READ;

  case 'usro':
    // DEPRECATED
    return Warn01(tag);

  case 'ogrp':
    // DEPRECATED
    return Warn01(tag);

  case 'iinc':
    ReadFloat (&iinc, stream);
    return TAG_READ;

  case 'idec':
    ReadFloat (&idec, stream);
    return TAG_READ;

  case 'oinc':
    ReadFloat (&oinc, stream);
    return TAG_READ;

  case 'odec':
    ReadFloat (&odec, stream);
    return TAG_READ;

  case 'inca':
    ReadFrom (&inca, stream);
		inca.YesToClick();
    return TAG_READ;

  case 'ouca':
    ReadFrom (&ouca, stream);
		ouca.YesToClick();
    return TAG_READ;
  //----Repeat Timer -----------------------
  case 'rept':
    ReadFloat(&p1,stream);
    ivar.SetTime(p1);
    ovar.SetTime(p1);
    return TAG_READ;
  //--- inner minimum ---------------------
  case 'imin':
    ReadFloat(&p1,stream);
    ivar.SetVMIN(p1);
    return TAG_READ;
  //--- outter minimum --------------------
  case 'omin':
    ReadFloat(&p1,stream);
    ovar.SetVMIN(p1);
    return TAG_READ;
  }

  return CGauge::Read (stream, tag);
}
//----------------------------------------------------------------------
//  Mouse moves over
//-----------------------------------------------------------------------
ECursorResult CDualKnobGauge::MouseMoved (int x, int y)
{ // Send updated mouse position to all click areas
  if (inca.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  if (ouca.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//-------------------------------------------------------------------------------
//  Mouse click:  Keep focus until stop
//-------------------------------------------------------------------------------
EClickResult CDualKnobGauge::MouseClick (int mouseX, int mouseY, int buttons)
{// Check inner click area
  if (inca.IsHit (mouseX, mouseY))
  { float val = (buttons & MOUSE_BUTTON_LEFT)?(-idec):(iinc);
    ivar.Arm(val);
    return MOUSE_TRACKING_ON;
  }
 // check outer click area
  if (ouca.IsHit (mouseX, mouseY))
  { float val = (buttons & MOUSE_BUTTON_LEFT)?(-odec):(oinc);
    ovar.Arm(val);
    return MOUSE_TRACKING_ON;
  }
  return MOUSE_TRACKING_OFF;
}
//--------------------------------------------------------------------------
//  Stop click : disarm
//--------------------------------------------------------------------------
EClickResult CDualKnobGauge::StopClick()
{ ivar.Disarm();
  ovar.Disarm();
  return MOUSE_TRACKING_OFF;
}
//--------------------------------------------------------------------------
//  Draw:  just to change variator value
//--------------------------------------------------------------------------
void CDualKnobGauge::Draw()
{ ivar.HasChanged();
  ovar.HasChanged();
  return;
}
//==========================================================================
// CFlyhawkLightKnobGauge
//
/*
CFlyhawkLightKnobGauge::CFlyhawkLightKnobGauge (void)
{
}

int CFlyhawkLightKnobGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc == TAG_IGNORED) {
    rc = CDualKnobGauge::Read (stream, tag);
  }

  return rc;
}
*/

//=============================================================================
// CTurnKnobGauge
//
//=============================================================================
CTurnKnobGauge::CTurnKnobGauge (CPanel *mp)
: CGauge(mp)
{ incr = +0.1f;
  decr = -0.1f;
  *bmap_art = '\0';
}
//-----------------------------------------------------------------------------
//  Read parameters
//-----------------------------------------------------------------------------
int CTurnKnobGauge::Read (SStream *stream, Tag tag)
{ switch (tag) {
  case 'bmap':
    ReadString (bmap_art, 64, stream);
    return TAG_READ;
  case 'mesg':
    ReadMessage(&var.msg,stream);
    var.Init();
    return TAG_READ;
  case 'incr':
  case 'rght':
    ReadFloat (&incr, stream);
    return TAG_READ;

  case 'decr':
  case 'left':
    ReadFloat (&decr, stream);
    return TAG_READ;
  }
  return CGauge::Read (stream, tag);
}
//-----------------------------------------------------------------------------
//  All parameters are read
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//  Mouse click:  Keep focus until stop
//-------------------------------------------------------------------------------
EClickResult CTurnKnobGauge::MouseClick (int mouseX, int mouseY, int buttons)
{ float val = (buttons & MOUSE_BUTTON_LEFT)?(decr):(incr);
  var.Arm(val);
  return MOUSE_TRACKING_ON;
}
//--------------------------------------------------------------------------
//  Stop click : disarm
//--------------------------------------------------------------------------
EClickResult CTurnKnobGauge::StopClick()
{ var.Disarm();
  return MOUSE_TRACKING_OFF;
}
//--------------------------------------------------------------------------
//  Draw:  just to change variator value
//--------------------------------------------------------------------------
void CTurnKnobGauge::Draw()
{ var.HasChanged();
  return;
}
//===============================================================
// CCoveredSwitch
//===============================================================

CCoveredSwitch::CCoveredSwitch (CPanel *mp)
: CSimpleSwitch(mp)
{
}

int CCoveredSwitch::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc == TAG_IGNORED) {
    rc = CSimpleSwitch::Read (stream, tag);
  }

  return rc;
}


//=======================================================================
// CCoveredLightButtonGauge
//=======================================================================
CCoveredLightButtonGauge::CCoveredLightButtonGauge (CPanel *mp)
: CCoveredSwitch(mp)
{ }
//-------------------------------------------------------------
//  Read the tags
//-------------------------------------------------------------
int CCoveredLightButtonGauge::Read (SStream *stream, Tag tag)
{ switch (tag) {
  case 'lbmp':
    lbut.Read(stream,0);
    return TAG_READ;

  case 'lmsg':
    ReadMessage (&lmsg, stream);
    return TAG_READ;
  }
  return CCoveredSwitch::Read (stream, tag);
}


//=========================================================================
// CVerticalSliderGauge
//=========================================================================
CVerticalSliderGauge::CVerticalSliderGauge (CPanel *mp)
: CGauge(mp)
{
}

int CVerticalSliderGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc == TAG_IGNORED) {
    rc = CGauge::Read (stream, tag);
  }

  return rc;
}


//
// CTrimWheelGauge
//

CTrimWheelGauge::CTrimWheelGauge (CPanel *mp)
: CGauge(mp)
{
  strcpy (bmap_art, "");
}

int CTrimWheelGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'bmap':
    ReadString (bmap_art, 64, stream);
    rc = TAG_READ;
    break;

  case 'down':
  case 'left':
    ReadFrom (&down, stream);
    rc = TAG_READ;
    break;

  case 'up  ':
  case 'rght':
    ReadFrom (&up, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CGauge::Read (stream, tag);
  }

  return rc;
}
//-------------------------------------------------------------
//  Mouse moves over
//-------------------------------------------------------------
ECursorResult CTrimWheelGauge::MouseMoved (int x, int y)
{ // Send updated mouse position to all click areas
  if (down.MouseMoved (x, y))   return CURSOR_WAS_CHANGED;
  if (  up.MouseMoved (x, y))   return CURSOR_WAS_CHANGED;
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//================================================================
// CTrimWheelIndicatorGauge
//================================================================
CTrimWheelIndicatorGauge::CTrimWheelIndicatorGauge (CPanel *mp)
: CTrimWheelGauge(mp)
{
}

int CTrimWheelIndicatorGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc == TAG_IGNORED) {
    rc = CTrimWheelGauge::Read (stream, tag);
  }

  return rc;
}


//====================================================================
// CIndicatorGauge
//====================================================================
CIndicatorGauge::CIndicatorGauge (CPanel *mp)
:CGauge(mp)
{ //---Default value ------------------------
  frpv = 0;
  NbFr = 0;
  lval = 0;
  hval = 1;
}
//--------------------------------------------------------------
//	Set the bitmap
//--------------------------------------------------------------
void CIndicatorGauge::SetBitmap(SStream *str)
{	char name[64] = {0};
	char kart[128];
  ReadString (name, 64, str);
	if (name[0] == 0)	return;
	_snprintf(kart,63, "ART/%s", name);
	kart[63] = 0;
  bmap = new CBitmap(kart);
	NbFr = bmap->NumBitmapFrames();
	return;
}

//-----------------------------------------------------------
//  Read all parameters
//-----------------------------------------------------------
int CIndicatorGauge::Read (SStream *stream, Tag tag)
{
  switch (tag) {
  case 'bmap':
    SetBitmap(stream);
    return TAG_READ;

  case 'lval':
    ReadFloat (&lval, stream);
    return TAG_READ;

  case 'hval':
    ReadFloat (&hval, stream);
    return TAG_READ;
  }

  return CGauge::Read (stream, tag);
}
//-----------------------------------------------------------
//  All parameters are read
//-----------------------------------------------------------
void CIndicatorGauge::ReadFinished()
{ CGauge::ReadFinished();
  //----Compute the frame per value coefficient -------
  float ampli = (hval - lval) + 1;
  if (NbFr) frpv = (ampli / NbFr);
  return;
}
//-----------------------------------------------------------
//  Clamp value
//-----------------------------------------------------------
float CIndicatorGauge::NormValue(float val)
{ if (val < lval)   val = lval;
  if (val > hval)   val = hval;
  float norm = val - lval;
  return norm;
}
//-----------------------------------------------------------
//  Draw the gauge
//-----------------------------------------------------------
void CIndicatorGauge::Draw()
{ EraseSurfaceRGBA(surf,0);
  Send_Message(&mesg);
  //---Draw According to value ---------------
  float val = NormValue(mesg.realData);
  int   frm = int(val * frpv);
  bmap->DrawBitmap(surf, 0, 0, frm);
}
//=========================================================================
//
// CFlapsIndicatorGauge
//
//=========================================================================
CFlapsIndicatorGauge::CFlapsIndicatorGauge (CPanel *mp)
:CIndicatorGauge(mp)
{;}

int CFlapsIndicatorGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc == TAG_IGNORED) {
    rc = CIndicatorGauge::Read (stream, tag);
  }

  return rc;
}


//=================================================================
// CPushButtonGauge
//=================================================================
CPushButtonGauge::CPushButtonGauge (CPanel *mp)
:CGauge(mp)
{ strcpy (bmap_art, "");
  strcpy (curs_csr, "");
  mmnt = false;
}

int CPushButtonGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'bmap':
    ReadString (bmap_art, 64, stream);
    rc = TAG_READ;
    break;

  case 'curs':
    ReadString (curs_csr, 64, stream);
    rc = TAG_READ;
    break;

  case 'mmnt':
    mmnt = true;
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CGauge::Read (stream, tag);
  }

  return rc;
}


//
// CLightButtonGauge
//

CLightButtonGauge::CLightButtonGauge (CPanel *mp)
: CPushButtonGauge(mp)
{
  stat_off = stat_on = 0;
  strcpy (sstr_off, "");
  strcpy (sstr_on, "");
}

int CLightButtonGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'lmsg':
    ReadMessage (&lmsg, stream);
    rc = TAG_READ;
    break;

  case 'stat':
    ReadInt (&stat_off, stream);
    ReadInt (&stat_on, stream);
    rc = TAG_READ;
    break;

  case 'sstr':
    ReadString (sstr_off, 64, stream);
    ReadString (sstr_on, 64, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CPushButtonGauge::Read (stream, tag);
  }

  return rc;
}


//
// CTurnSwitchGauge
//

CTurnSwitchGauge::CTurnSwitchGauge (CPanel *mp)
: CSimpleSwitch(mp)
{
}

int CTurnSwitchGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  if (rc == TAG_IGNORED) {
    rc = CSimpleSwitch::Read (stream, tag);
  }

  return rc;
}


//========================================================================
// CMasterCautionWarningButtonGauge
//========================================================================

CMasterCautionWarningButtonGauge::CMasterCautionWarningButtonGauge (CPanel *mp)
:CGauge(mp)
{ strcpy (bmap_art, "");}
//------------------------------------------------------
//	JSDEV* prepare message
//------------------------------------------------------
void CMasterCautionWarningButtonGauge::PrepareMsg(CVehicleObject *veh)
{	smsg.id		    = MSG_GETDATA;
  smsg.sender   = unId;
	veh->FindReceiver(&smsg);
	lmsg.id		    = MSG_GETDATA;
  lmsg.sender   = unId;
	veh->FindReceiver(&lmsg);
	CGauge::PrepareMsg(veh);
	return;
}
//-------------------------------------------------------------------------------
//  Read gauge tags
//-------------------------------------------------------------------------------
int CMasterCautionWarningButtonGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'bmap':
    ReadString (bmap_art, 64, stream);
    return TAG_READ;

  case 'smsg':
  case 'swch':
    ReadMessage (&smsg, stream);
    return TAG_READ;

  case 'lmsg':
    ReadMessage (&lmsg, stream);
    return TAG_READ;
  }
  return CGauge::Read (stream, tag);
}
//-------------------------------------------------------------------------------
//  Read finished
//-------------------------------------------------------------------------------
void CMasterCautionWarningButtonGauge::ReadFinished (void)
{
  CGauge::ReadFinished ();
  return;
}


//===============================================================================
// CKAFireExtinguisherButton
//===============================================================================
CKAFireExtinguisherButton::CKAFireExtinguisherButton (CPanel *mp)
:CGauge(mp)
{  curs_tag = 0;
}
//--------------------------------------------------------------------
CKAFireExtinguisherButton::~CKAFireExtinguisherButton (void)
{}
//--------------------------------------------------------------------
//  Read Gauge tags
//--------------------------------------------------------------------
int CKAFireExtinguisherButton::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'd___':
    adsc.Read(stream,2);
    return TAG_READ;

  case 'ok__':
    arok.Read(stream,2);
    return TAG_READ;

  case 'fire':
    afir.Read(stream,2);
    return TAG_READ;

  case 'curs':
    curs_tag = BindCursor(stream);
    return TAG_READ;
  }
  return CGauge::Read (stream, tag);
}
//--------------------------------------------------------------------
//  Read finished
//--------------------------------------------------------------------
void CKAFireExtinguisherButton::ReadFinished (void)
{
  CGauge::ReadFinished ();
  return;
}


//===========================================================================
// CWalkerAPPanel
//===========================================================================
CWalkerAPPanel::CWalkerAPPanel (CPanel *mp)
:CGauge(mp)
{
  strcpy (enga_yd_art, "");
  strcpy (enga_ap_art, "");
  strcpy (knob_bnk_art, "");
  strcpy (knob_trm_art, "");
}

int CWalkerAPPanel::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'left':
    ReadFrom (&left, stream);
    rc = TAG_READ;
    break;

  case 'rght':
    ReadFrom (&rght, stream);
    rc = TAG_READ;
    break;

  case 'up__':
    ReadFrom (&up, stream);
    rc = TAG_READ;
    break;

  case 'down':
    ReadFrom (&down, stream);
    rc = TAG_READ;
    break;

  case 'xfer':
    ReadFrom (&xfer, stream);
    rc = TAG_READ;
    break;

  case 'turb':
    ReadFrom (&turb, stream);
    rc = TAG_READ;
    break;

  case 'ydon':
    ReadFrom (&ydon, stream);
    rc = TAG_READ;
    break;

  case 'ydof':
    ReadFrom (&ydof, stream);
    rc = TAG_READ;
    break;

  case 'apon':
    ReadFrom (&apon, stream);
    rc = TAG_READ;
    break;

  case 'apof':
    ReadFrom (&apof, stream);
    rc = TAG_READ;
    break;

  case 'enga':
    ReadString (enga_yd_art, 64, stream);
    ReadString (enga_ap_art, 64, stream);
    rc = TAG_READ;
    break;

  case 'knob':
    ReadString (knob_bnk_art, 64, stream);
    ReadString (knob_trm_art, 64, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CGauge::Read (stream, tag);
  }

  return rc;
}


//
// CWalkerFirePanel
//

CWalkerFirePanel::CWalkerFirePanel (CPanel *mp)
:CGauge(mp)
{
}

int CWalkerFirePanel::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'fir1':
    ReadMessage (&fir1, stream);
    rc = TAG_READ;
    break;

  case 'fir2':
    ReadMessage (&fir2, stream);
    rc = TAG_READ;
    break;

  case 'ext1':
    ReadFrom (&ext1, stream);
    rc = TAG_READ;
    break;

  case 'ext2':
    ReadFrom (&ext2, stream);
    rc = TAG_READ;
    break;

  case 'bel1':
    ReadFrom (&bel1, stream);
    rc = TAG_READ;
    break;

  case 'bel2':
    ReadFrom (&bel2, stream);
    rc = TAG_READ;
    break;

  case 'ann1':
//    ReadFrom (&ann1, stream);
    rc = TAG_READ;
    break;

  case 'ann2':
//    ReadFrom (&ann2, stream);
    rc = TAG_READ;
    break;

  case 'ef1_':
    ReadFrom (&ef1, stream);
    rc = TAG_READ;
    break;

  case 'ef2_':
    ReadFrom (&ef2, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CGauge::Read (stream, tag);
  }

  return rc;
}


//
// CLightSwitchSetGauge
//

CLightSwitchSetGauge::CLightSwitchSetGauge (CPanel *mp)
: CBitmapGauge(mp)
{;}
//------------------------------------------------------------
//	Prepare Message
//------------------------------------------------------------
void CLightSwitchSetGauge::PrepareMsg(CVehicleObject *veh)
{	lmsg.id		= MSG_GETDATA;
	veh->FindReceiver(&lmsg);
	CGauge::PrepareMsg(veh);
	return;
}
int CLightSwitchSetGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'lmsg':
    ReadMessage (&lmsg, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CBitmapGauge::Read (stream, tag);
  }

  return rc;
}


//==============================================================================
// CRockerSwitch
//==============================================================================

CRockerSwitch::CRockerSwitch (CPanel *mp)
: CSimpleSwitch(mp)
{
}
//----------------------------------------------------------------
//	Prepare messages
//----------------------------------------------------------------
void CRockerSwitch::PrepareMsg(CVehicleObject *veh)
{	msg1.id		    = MSG_GETDATA;
  msg1.sender   = unId;
	veh->FindReceiver(&msg1);
	msg2.id		    = MSG_GETDATA;
  msg2.sender   = unId;
	veh->FindReceiver(&msg2);
	CSimpleSwitch::PrepareMsg(veh);
	return;
}
//----------------------------------------------------------------
//	Read gauge tags
//----------------------------------------------------------------
int CRockerSwitch::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'one_':
    ReadFrom (&one, stream);
    return TAG_READ;

  case 'two_':
    ReadFrom (&two, stream);
    return TAG_READ;

  case 'msg1':
    ReadMessage (&msg1, stream);
    return TAG_READ;

  case 'msg2':
    ReadMessage (&msg2, stream);
    return TAG_READ;

  }
  return CSimpleSwitch::Read (stream, tag);
}
//-----------------------------------------------------------------
//  All parameters read
//------------------------------------------------------------------
void CRockerSwitch::ReadFinished (void)
{ CGauge::ReadFinished ();
  return;
}
//------------------------------------------------------------------
//  Mouse moves over
//------------------------------------------------------------------
ECursorResult CRockerSwitch::MouseMoved (int x, int y)
{ // Send updated mouse position to all click areas
  if (one.MouseMoved (x, y))    return CURSOR_WAS_CHANGED;
  if (two.MouseMoved (x, y))    return CURSOR_WAS_CHANGED;
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//=================END OF FILE =====================================================================
