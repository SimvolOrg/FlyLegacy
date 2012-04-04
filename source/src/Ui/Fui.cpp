/*
 * Fui.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
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
 */

/*! \file Fui.cpp
 *  \brief Implements Fly! UI widget classes descended from CFuiComponent
 */
//=================================================================================
#pragma warning(disable:4201)  // For file mmsystem.h
//---------------------------------------------------------------------------------
#include <stdarg.h>
#include "../Include/Ui.h"
#include "../Include/Fui.h"
#include "../Include/FuiUser.h"
#include "../Include/FuiParts.h"
#include "../Include/FuiOption.h"
#include "../Include/Utility.h"
#include "../Include/Globals.h"
#include "../Include/CursorManager.h"
#include "../Include/database.h"
#include "../Include/MagneticModel.h"
#include "../Include/WorldObjects.h"
#include "../Include/TerrainTexture.h"
//--------------------------------------------------------------------
using namespace std;

///////// DEBUG STUFF TO REMOVE LATER ////////////////////////////////
#ifdef  _DEBUG	
  //#define _DEBUG_ECW        // print lc DDEBUG file ... remove later
  //#define _DEBUG_CFuiDLL    //                      ... remove later 
#endif
//////////////////////////////////////////////////////////////////////
extern char asciiVAL[];
//--------------------------------------------------------------------------------
//  Compute free memory and trace
//--------------------------------------------------------------------------------
#ifdef HAVE_HEAPINFO
void heapdump( char *txt )
{ int mused = 0;
  int mfree = 0;
  TRACE("=========== %s =====================================",txt);
   _HEAPINFO hinfo;
   int heapstatus;
   hinfo._pentry = NULL;
   while( ( heapstatus = _heapwalk( &hinfo ) ) == _HEAPOK )
   { if (hinfo._useflag == _USEDENTRY)  mused += hinfo._size;
     if (hinfo._useflag != _USEDENTRY)  mfree += hinfo._size;
   }
  TRACE("USED = %6d       FREE = %6d",mused,mfree);
   switch( heapstatus )
   {
   case _HEAPEMPTY:
      TRACE( "OK - empty heap\n" );
      break;
   case _HEAPEND:
      TRACE( "OK - end of heap\n" );
      break;
   case _HEAPBADPTR:
      TRACE( "ERROR - bad pointer to heap\n" );
      break;
   case _HEAPBADBEGIN:
      TRACE( "ERROR - bad start of heap\n" );
      break;
   case _HEAPBADNODE:
      TRACE( "ERROR - bad node in heap\n" );
      break;
   }
}
#endif HAVE_HEAPINFO

//=====================================================================================
// Forward declaration of local helper functions
static CFuiComponent *CreateFuiComponent (EFuiComponentTypes type,CFuiComponent *win);
static bool           ValidFuiComponentType (Tag tag);


static SFont* FuiFont (Tag fontTag)
{
  SFont* rc = NULL;

  switch (fontTag) {
  case 'deff':
    // Default 10-point font
    return &globals->fonts.ftasci10;

  case 'tath':
    // Thin 24-point font
    return &globals->fonts.ftthin24;

  case 'butn':
    // Button uses default 10-point font
    return &globals->fonts.ftasci10;

  case 'micr':
    // Micro 5-point font
    gtfo("Use of micr font");
    break;

  case 'digi':
     gtfo("Use of digi font");
     break;

  case 'bold':
     return &globals->fonts.ftmono14;

  default:
    WARNINGLOG ("Unsupported FUI font 0x%08X", fontTag);
    rc = &globals->fonts.ftasci10;
  }

  return rc;
}


//===================================================================================
// CFuiComponent
//===================================================================================
CFuiComponent::CFuiComponent (int px, int py, int wd, int ht, CFuiComponent *win)
{ x = px;
  y = py;
  w = wd;
  h = ht;
  halfH   = h / 2;
  halfW   = w / 2;
  MoWind  = win;
  type = (EFuiComponentTypes)0;
  text[0]   = 0;
  id = 0;
  bind    = 0;
  prop    = 0;                // Default properties
  wName   = "";
  widgetTag = 'defa';
 *text = 0;
  xParent   = yParent = 0;
  SetProperty(FUI_IS_VISIBLE + FUI_IS_ENABLE);
  fontTag   = 'deff';
  font      = NULL;
  surface   = NULL;
  tw        = NULL;
  cFocus    = 0;
  clip      = 0;
  //---Set lower case by default -------------
  upper     = 0xFF;
}
//------------------------------------------------------------------------
//  Destroy the component
//  Delete all decoration items
//------------------------------------------------------------------------
CFuiComponent::~CFuiComponent (void)
{
  // Delete decoration components
  std::list<CFuiComponent*>::iterator i;
  for (i=decorationList.begin(); i!=decorationList.end(); i++) 
  { CFuiComponent *cp = (*i);
    delete (cp);
  }
  decorationList.clear();

  if (surface)  surface = FreeSurface (surface);
}
//------------------------------------------------------------------------
//  Read all tags
//------------------------------------------------------------------------
int CFuiComponent::Read (SStream *stream, Tag tag)
{ int rx,ry,rw,rh;
  int pm;
	Tag  wt;
  char rsz[4];
	char wn[64];
  switch (tag) {
  case 'ID  ':
  case 'id  ':
    ReadTag (&id, stream);
    return TAG_READ;
  case 'bind':
    { int i;
      ReadInt (&i, stream);
      bind = (EFuiBinding) i;
    }
    return TAG_READ;
  case 'widg':
    ReadString (wn, 64, stream);
    ReadTag (&wt, stream);					// JS: Force 'defa'
    return TAG_READ;
  case 'text':
    ReadString (text, 256, stream);
    return TAG_READ;
  case 'loc ':
    ReadInt (&rx, stream);
    ReadInt (&ry, stream);
    if (x == 0) x = rx;
    if (y == 0) y = ry;
    return TAG_READ;
  case 'rsiz':
    ReadString(rsz,4,stream);
    if (rsz[0] == 'R') SetProperty(FUI_VT_RELOCATE);
    if (rsz[0] == 'V') SetProperty(FUI_VT_RESIZING);
    return TAG_READ;
  case 'trns':
    SetTransparentMode();
    return TAG_READ;
  case 'size':
    ReadInt (&rw, stream);
    ReadInt (&rh, stream);
    if (w == 0) w = rw;
    if (h == 0) h = rh;
    halfW = (w >> 1);
    halfH = (h >> 1);
    return TAG_READ;
  case 'enab':
    int e;
    ReadInt (&e, stream);
    if (!e) RazProperty(FUI_IS_ENABLE);
    return TAG_READ;
  case 'modf':
		ReadInt (&pm,stream);
    SetEditMode(pm);
    return TAG_READ;
  case 'uper':
    UpperCase();
    return TAG_READ;
  case 'show':
    int s;
    ReadInt (&s, stream);
    Show((s == 1));
    return TAG_READ;
  case 'font':
    ReadTag (&fontTag, stream);
    return TAG_READ;
	case 'ok  ':
		ReadInt(&pm,stream);					// Ignore now
		return TAG_READ;
	case 'lsiz':
		ReadInt(&pm,stream);
		ReadInt(&pm,stream);
		return TAG_READ;
	case 'bsiz':
		ReadInt(&pm,stream);
		ReadInt(&pm,stream);
		return TAG_READ;
  }

  char s[8];
  WARNINGLOG ("%s : Unknown tag %s", wName, TagString (s, tag));
  return TAG_IGNORED;
}
//-------------------------------------------------------------------------
//  Clip the child window  
//-------------------------------------------------------------------------
void CFuiComponent::ClipChild()
{
}
//--------------------------------------------------------------------------
//  All tags read
//--------------------------------------------------------------------------
void CFuiComponent::ReadFinished (void)
{ font = FuiFont (fontTag);
  fnts = (CFont*)font->font;
  MakeSurface ();
	TagToString(desi,id);
  return;
}
//--------------------------------------------------------------------------
//  Find Window Theme
//	JS:  Force default theme for any windows
//--------------------------------------------------------------------------
void CFuiComponent::FindThemeWidget (void)
{ // Get FUI theme for this component
  tw = globals->fui->GetThemeWidget (widgetTag, wName);
  if (tw == NULL) {
    char s[8];
    TagToString (s, widgetTag);
    gtfo ("%s : Cannot get theme widget %s", wName, s);
  }
}
//-------------------------------------------------------------------------
//  Find given widget theme
//-------------------------------------------------------------------------
CFuiThemeWidget *CFuiComponent::FindThemeWidget (Tag wid, const char *name)
{
  // Get FUI theme for this component
  CFuiThemeWidget *tm = globals->fui->GetThemeWidget (wid, name);
  if (tm == NULL) {
    char s[8];
    TagToString (s, wid);
    gtfo ("%s : Cannot get theme widget %s", name, s);
  }
  return tm;
}
//-----------------------------------------------------------------------
//  Modify visibility
//----------------------------------------------------------------------
void CFuiComponent::Show(bool vs)
{ if (vs) SetProperty(FUI_IS_VISIBLE);
  else    RazProperty(FUI_IS_VISIBLE);
  return;
}

//--------------------------------------------------------------------------
//  Set font according to Tag
//--------------------------------------------------------------------------
void CFuiComponent::SetFont (Tag fontTag)
{
  this->fontTag = fontTag;
  this->font    = FuiFont (fontTag);
}

//--------------------------------------------------------------------------
// Create drawing surface for the component.  This may be called either from
//   ReadFinished() following the parsing of all tags from a .win file,
//   or in the constructor that takes the component size/location.
//--------------------------------------------------------------------------
void CFuiComponent::MakeSurface ()
{ surface = FreeSurface(surface);
  surface = CreateSurface (w, h);
	if (0 == surface)	return;
  surface->xScreen = xParent + x;
  surface->yScreen = yParent + y;
  EraseSurfaceRGBA (surface,0);
  //-----Remap pannel dimension if any --------------
 // if (0 == HasProperty(FUI_HAS_QUAD))     return;
  surface->xScreen &= 0xFFFFFFFE;         // Make it even
  surface->yScreen &= 0xFFFFFFFE;   
  return;
}
//-------------------------------------------------------------------------------
//  Init the QUAD
//-------------------------------------------------------------------------------
void CFuiComponent::InitQuad()
{ //----SW base corner ------------------------------------------------
  Pan[0].VT_S   = 0;
  Pan[0].VT_T   = 0;
  Pan[0].VT_X   = 0;
  Pan[0].VT_Y   = 0;
	Pan[0].VT_Z		= 0;
  //-----SE corner ----------------------------------------------------
  Pan[1].VT_S   = 1;
  Pan[1].VT_T   = 0;
  Pan[1].VT_X   = float(surface->xSize);
  Pan[1].VT_Y   = 0;
	Pan[0].VT_Z		= 0;
  //-----NE corner ----------------------------------------------------
  Pan[2].VT_S   = 1;
  Pan[2].VT_T   = 1;
  Pan[2].VT_X   = float(surface->xSize);
  Pan[2].VT_Y   = float(surface->ySize);
	Pan[0].VT_Z		= 0;
  //-----NW corner ----------------------------------------------------
  Pan[3].VT_S   = 0;
  Pan[3].VT_T   = 1;
  Pan[3].VT_X   = 0;
  Pan[3].VT_Y   = float(surface->ySize);
	Pan[0].VT_Z		= 0;
  return;
}
//-------------------------------------------------------------------------------
//  Init the QUAD
//-------------------------------------------------------------------------------
void CFuiComponent::SetQuad(int wd,int ht)
{ //----SW base corner ------------------------------------------------
  Pan[0].VT_S   = 0;
  Pan[0].VT_T   = 0;
  Pan[0].VT_X   = 0;
  Pan[0].VT_Y   = 0;
	Pan[0].VT_Z		= 0;
  //-----SE corner ----------------------------------------------------
  Pan[1].VT_S   = 1;
  Pan[1].VT_T   = 0;
  Pan[1].VT_X   = float(wd);
  Pan[1].VT_Y   = 0;
	Pan[1].VT_Z		= 0;
  //-----NE corner ----------------------------------------------------
  Pan[2].VT_S   = 1;
  Pan[2].VT_T   = 1;
  Pan[2].VT_X   = float(wd);
  Pan[2].VT_Y   = float(ht);
	Pan[2].VT_Z		= 0;
  //-----NW corner ----------------------------------------------------
  Pan[3].VT_S   = 0;
  Pan[3].VT_T   = 1;
  Pan[3].VT_X   = 0;
  Pan[3].VT_Y   = float(ht);
	Pan[3].VT_Z		= 0;
  return;
}
//-------------------------------------------------------------------------------
//  Set the text
//-------------------------------------------------------------------------------
void CFuiComponent::SetText (char *txt)
{ if (txt) strncpy (text, txt, 254); else *text = 0;
  return;
}
//-------------------------------------------------------------------------------
//  Change text color
//-------------------------------------------------------------------------------
void CFuiComponent::ColorText(U_INT c)
{	colText = c;
	return EditText();
}
//-------------------------------------------------------------------------------
//  simple text edition
//-------------------------------------------------------------------------------
void CFuiComponent::EditText()
{	EraseSurfaceRGBA (surface,0);
  //---- Draw the text --and bitmap ------------- 
  if (0 == surface)		return;
  fnts->DrawNText (surface,4, 0, colText, text);
	return;
}
//---------------------------------------------------------------------------------
// CreateHBoxPicture
//
// Load a bitmap defined in the theme to a new CFuiPicture
//
/// \param name       Name of the theme bitmap
//---------------------------------------------------------------------------------
CFuiPicture *CFuiComponent::CreateHBoxPicture (const char* name, int wd,int ht)
{ CFuiPicture *rc = NULL;
  CBitmap *bm = tw->GetBitmap (name);
  if (0 == bm) return 0;
  // Theme bitmap is defined, create CFuiPicture and bind the bitmap to it
  int bmw, bmh;
  bm->GetBitmapSize (&bmw, &bmh);
  //---Specify width either from request or from bitmap ------------
  int bwd = (wd)?(wd):(bmw);
	int bht = (ht)?(ht):(bmh);
 // rc = new CFuiPicture (0, 0, bwd, bmh);
	rc = new CFuiPicture (0, 0, bwd, bht);
  rc->SetBitmap (bm);
  return rc;
}

//---------------------------------------------------------------------------------
// CreateHBox
//
// Create a horizontal bitmap box consisting of LEFT, BACK and RIGHT bitmaps
//	NOTE:  The bitmap is of fixed size in Height. The button can
//				only vary in width
//---------------------------------------------------------------------------------
int CFuiComponent::CreateHBox (CFuiPicture *box[], int xw)
{
  CFuiPicture *pic;
  int lw = 0, rw = 0;

  // Create left-side box
  pic = CreateHBoxPicture ("LEFT",0,0);
  if (pic) {
    lw = pic->GetWidth ();
    pic->SetPosition (0, 0);
    pic->MoveParentTo (x, y);
    decorationList.push_back (pic);
    box[0] = pic;
  }

  // Create right-side box
  pic = CreateHBoxPicture ("RIGHT",0,0);
  if (pic) {
    rw = pic->GetWidth ();
    pic->SetPosition (w - rw, 0);
    pic->MoveParentTo (x, y);
    decorationList.push_back (pic);
    box[1] = pic;
  }

  // Create backdrop box (tiled)
  pic = CreateHBoxPicture ("BACK",(w - lw - rw),0);
  if (pic) {
    pic->SetPosition (lw, 0);
    pic->MoveParentTo (x, y);
    decorationList.push_back (pic);
    box[2] = pic;
  }

  return (box[2])?(box[2]->GetHeight()):(0);
}

//---------------------------------------------------------------------------------
//  Set Frame to the HBox 
//---------------------------------------------------------------------------------
void CFuiComponent::SetHBoxFrame(CFuiPicture *box[],int No)
{ if (box[0]) box[0]->SetFrame(No);
  if (box[1]) box[1]->SetFrame(No);
  if (box[2]) box[2]->SetFrame(No);
  return;
}

//---------------------------------------------------------------------------------
//  Draw the HBox
//---------------------------------------------------------------------------------
void CFuiComponent::DrawHBox(CFuiPicture *box[])
{ if (box[0]) box[0]->Draw();
  if (box[1]) box[1]->Draw();
  if (box[2]) box[2]->Draw();
  return;
}
//---------------------------------------------------------------------------------
//	Return bitmap according to the name
//---------------------------------------------------------------------------------
CBitmap* CFuiComponent::HaveBitmap(char *fn,char *own)
{	char fname[MAX_PATH];
	char *dot = strrchr(fn,'.');					// Look for dot
	*own	= 0;														// Default not owned
	//---No extention.  Try theme ------------------------
	if (0 == dot)	return tw->GetBitmap(fn);
	//--- Standard bitmap. Create and load ---------------
	_snprintf(fname,PATH_MAX,"ART/%s",fn);
	CBitmap *bmp	= new CBitmap(fname);
	*own	= 1;
	return bmp;
}
//---------------------------------------------------------------------------------
//  Create a side for full box
//  pwin  Parent window
//  wp    0=> Bitmap wide, W=>extend wide   to W
//  hp    0=> Bitmap wide, H=>extend height to H
//  wr    return wide
//  hr    return height
//----------------------------------------------------------------------------------
CFuiPicture *CFuiComponent::CreateFBoxSide(CFuiComponent *pwin,char * name,int wp, int hp, int *wr, int *hr)
{ char     own	= 0;												// Own indicator
	CBitmap *bmp	= HaveBitmap(name,&own);
  int wd  = 0;
  int ht  = 0;
  *wr     = 0;
  *hr     = 0;
  if (0 == bmp)      return 0;
  bmp->GetBitmapSize (&wd, &ht);
	if (0 == wd)	gtfo("No bitmap %s",name);
  if (wp) wd  = wp;
  if (hp) ht  = hp;
  CFuiPicture *pic = new CFuiPicture (0, 0, wd, ht,pwin);
  pic->SetBitmap (bmp);
	pic->SetOwn(own);
  pic->MoveParentTo(pwin->GetXPosition(),pwin->GetYPosition());
  decorationList.push_back (pic);
  *wr  = wd;
  *hr  = ht;
  return pic;
  }
//---------------------------------------------------------------------------------
//  Init a full box
//---------------------------------------------------------------------------------
void CFuiComponent::InitFBox(CFuiComponent *box[],int siz)
{ int inx = 0;
  for (inx = 0; inx < siz; inx++)  box[inx] = 0;
  return;
}
//---------------------------------------------------------------------------------
//  Create a  full box
//  x,y   window position
//  wd    window wide
//  ht    window height
//----------------------------------------------------------------------------------
void CFuiComponent::CreateFBox(CFuiComponent *pwin,CFuiComponent *box[],int x, int y,int wd, int ht)
{ tMG = 0;
  bMG = 0;
  lMG = 0;
  rMG = 0;
  int rwd = 0;
  int rht = 0;
  // Get optional top frame bitmap
	if (!HasProperty(FUI_NO_BORDER))
	{ box[TOPW]   = CreateFBoxSide(pwin,"TOP",wd,0,&rwd,&rht);
		if (box[TOPW])   box[TOPW]->MoveTo(0,-rht);
		tMG = rht;
		// Get optional bottom frame bitmap
		box[BOTW]   = CreateFBoxSide(pwin,"BOTTOM",wd,0,&rwd,&rht);
		if (box[BOTW])   box[BOTW]->MoveTo(0,ht);
		bMG = rht;
		// Get optional left frame bitmap
		box[LEFW]   = CreateFBoxSide(pwin,"LEFT",0,ht,&rwd,&rht);
		if (box[LEFW])   box[LEFW]->MoveTo(-rwd,0);
		lMG = rwd;
		// Get optional right frame bitmap
		box[RIGW]   = CreateFBoxSide(pwin,"RIGHT",0,ht,&rwd,&rht);
		if (box[RIGW])   box[RIGW]->MoveTo(wd,0);
		rMG = rwd;
		// Get optional top-left corner bitmap
		box[TLFW]   = CreateFBoxSide(pwin,"TOPLEFT",0,0,&rwd,&rht);
		if (box[TLFW])   box[TLFW]->MoveTo(-lMG,-tMG);
		// Get optional top-right corner bitmap
		box[TRGW]   = CreateFBoxSide(pwin,"TOPRIGHT",0,0,&rwd,&rht);
		if (box[TRGW])   box[TRGW]->MoveTo(w,-tMG);
		// Get optional bottom-left corner bitmap
		box[BLFW]   = CreateFBoxSide(pwin,"BOTLEFT",0,0,&rwd,&rht);
		if (box[BLFW])   box[BLFW]->MoveTo(-lMG,h);
		// Get optional bottom-right corner bitmap
		box[BRGW]   = CreateFBoxSide(pwin,"BOTRIGHT",0,0,&rwd,&rht);
		if (box[BRGW])   box[BRGW]->MoveTo(w,h);
		//--------Create Back plane if not transparent windows -----------------
	}
	if (HasProperty(FUI_TRANSPARENT)) return;
	box[BAKW]   = CreateFBoxSide(pwin,"BACK",w ,h,&rwd,&rht);
	if (box[BAKW])  box[BAKW]->MoveTo(0,0);
	return;
  }
//---------------------------------------------------------------------------------
//  Draw Decoration Full Box
//---------------------------------------------------------------------------------
void  CFuiComponent::DrawFBox(CFuiComponent *box[],int siz)
{ int inx = 0;
  for (inx = TOPW; inx < siz; inx++) if (box[inx])   box[inx]->Draw();
  return;
}
//---------------------------------------------------------------------------------
//  Notification to change dimension
//---------------------------------------------------------------------------------
void  CFuiComponent::RedimFBox(CFuiComponent *box[],short dx, short dy)
{ if (box[TOPW])   box[TOPW]->Format(WZ_SX00,dx,0);     // Top border
  if (box[TRGW])   box[TRGW]->Format(WZ_MX00,dx,0);     // Top Right corner
  if (box[RIGW])   box[RIGW]->Format(WZ_MXSY,dx,dy);    // Right border
  if (box[BRGW])   box[BRGW]->Format(WZ_MXMY,dx,dy);    // Bot Right corner
  if (box[BOTW])   box[BOTW]->Format(WZ_SXMY,dx,dy);    // Bot border
  if (box[BLFW])   box[BLFW]->Format(WZ_MY00,0,dy);     // Bot left corner
  if (box[LEFW])   box[LEFW]->Format(WZ_SY00,0,dy);     // Left border
  //-----Redim the background if present ------------------------
  CFuiComponent *cb = box[BAKW];
  if (cb)           cb->Redim(dx,dy);
  return;
}
//---------------------------------------------------------------------------
//  Set scroll box parameters
//---------------------------------------------------------------------------
void CFuiComponent::SetBox(SCBOX *box, char *bname)
{ int w,h;
  box->rx = 0;
  box->ry = 0;
  box->bmap = tw->GetBitmap(bname);
  box->bmap->GetBitmapSize(&w, &h);
  box->wd   = short(w);
  box->ht   = short(h);
  box->frm  = 0;
  return; }
//---------------------------------------------------------------------------------
//  Color the background of the box
//---------------------------------------------------------------------------------
void CFuiComponent::ColorInside(U_INT cr)
{ if (0 == surface)   return;
  U_INT *buf  = surface->drawBuffer;
  int end   = (surface->xSize * surface->ySize);
  int nbr   = 0;
  for (nbr = 0; nbr != end; nbr++) *buf++ = cr;
  return;
}
//---------------------------------------------------------------------------------
//  Tile a band bitmap accross surface
//  top and bot define the band position inside the surface
//---------------------------------------------------------------------------------
void CFuiComponent::TiledBand (CBitmap *bm,SSurface *sf,int y0,int y1)
{ if ((0 == bm) || (0 == sf)) return;
  int bx, by;
  bm->GetBitmapSize(&bx, &by);

  for (int iy=y0; iy<y1; iy+=by)
  {
    for (int ix=0; ix<(int)sf->xSize; ix+=bx) bm->DrawBitmap (sf, ix, iy, 0);
    
  }
  return;
}
//-----------------------------------------------------------------
//  Set new position
//-----------------------------------------------------------------
void CFuiComponent::SetPosition (int nx, int ny)
{ // Validate new position before applying
  this->x = CheckWD(nx);
  this->y = CheckHT(ny);
  if (0 == surface)             return;
  surface->xScreen = xParent + nx;
  surface->yScreen = yParent + ny;
  return;   
}
//-----------------------------------------------------------------
//  Set new X position
//-----------------------------------------------------------------
int CFuiComponent::CheckWD(int nx)
{ if ((nx < 5) && (nx < x))                                        return x;
  if ((nx > (globals->cScreen->Width  - this->w - 4)) && (nx > x)) return x;
  return nx;  }
//-----------------------------------------------------------------
//  Set new Y position
//-----------------------------------------------------------------
int CFuiComponent::CheckHT(int ny)
{ if ((ny <= 24) && (ny < y))                                      return y;
  if ((ny > (globals->cScreen->Height - this->h - 4)) && (ny > y)) return y;
  return ny;  }
//------------------------------------------------------------------
//  Move parent component to X,Y position
//------------------------------------------------------------------
void CFuiComponent::MoveParentTo (int xp, int yp)
{ this->xParent = xp;
  this->yParent = yp;
  surface->xScreen = xParent + x;
  surface->yScreen = yParent + y;
  return;
}
//------------------------------------------------------------------
//  Move the component to absolute X,Y position
//------------------------------------------------------------------
void CFuiComponent::MoveTo(int xs,int ys)
{ x = xs;
  y = ys;
  if (0 == surface) return;
  surface->xScreen = xParent + x;
  surface->yScreen = yParent + y;
  return;
}
//------------------------------------------------------------------
//  Move the component to relative X,Y position
//------------------------------------------------------------------
void CFuiComponent::MoveBy(int dx,int dy)
{ x += dx;
  y += dy;
  surface->xScreen = xParent + x;
  surface->yScreen = yParent + y;
  return;
}
//------------------------------------------------------------------------
//  Draw:  Use surface and texture
//------------------------------------------------------------------------
void CFuiComponent::Draw()
{ int opt; 
	glGetIntegerv(GL_BLEND,&opt);
	if (HasProperty(FUI_TRANSPARENT))  glEnable(GL_BLEND);
	Draw(surface);
	if (0 == opt)	glDisable(GL_BLEND);
  return;
}

//------------------------------------------------------------------------
//  Draw with a given surface
//------------------------------------------------------------------------
void CFuiComponent::Draw (SSurface *sf)
{
  // Only draw if surface has been created and the component is visible
  if ((sf) && HasProperty(FUI_IS_VISIBLE)) {
    glDrawBuffer (GL_BACK);
    // Starting raster position is bottom-left corner of the surface,
    //   with (0,0) at top-left of the screen
    glRasterPos2i (sf->xScreen, sf->yScreen + sf->ySize);
    glDrawPixels  (sf->xSize,   sf->ySize,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      sf->drawBuffer);
  }
}

//------------------------------------------------------------------------
//    Register the component that get focus
//    -Notify the previous for loosing focus 
//------------------------------------------------------------------------
bool CFuiComponent::RegisterFocus(CFuiComponent *comp)
{ bool  lf = (cFocus && (cFocus != comp));
	if (lf)			cFocus->FocusLost();
  cFocus  = comp;
  return lf;
}
//------------------------------------------------------------------------
//  Clear focus without notification
//------------------------------------------------------------------------
void CFuiComponent::ClearFocus(CFuiComponent *comp)
{ if (cFocus && (cFocus != comp)) cFocus->FocusLost();
  cFocus = 0;
  return;
}
//------------------------------------------------------------------------
//  Check for mouse Hit
//  sx and sy are screen coordinates
//------------------------------------------------------------------------
bool CFuiComponent::MouseHit (int sx, int sy)
{
  // Calculate component starting screen locations
  int cx = xParent + x;
  int cy = yParent + y;
  return (sx >= cx) && (sx < (cx + w)) && (sy >= cy) && (sy < (cy + h));
}

bool CFuiComponent::MouseMove (int x, int y)
{ if ( HasProperty(FUI_NO_MOUSE)) return false;
  return(MouseHit(x,y));
}

bool CFuiComponent::MouseClick (int x, int y, EMouseButton button)
{ if ( HasProperty(FUI_NO_MOUSE)) return false;
  return (MouseHit(x,y));
}
//------------------------------------------------------------------------
bool CFuiComponent::MouseStopClick (int x, int y, EMouseButton button)
{ return false;
}

//-----------------------------------------------------------------------
bool CFuiComponent::StopClickInside(int x, int y, EMouseButton button)
{ return false;
}

//------------------------------------------------------------------------
bool CFuiComponent::DoubleClick (int x, int y, EMouseButton button)
{ return true;
}

//-----------------------------------------------------------------------
//  Notify Menu Event.  Do nothing at this level
//-----------------------------------------------------------------------
void CFuiComponent::NotifyMenuEvent(Tag idm, Tag itm)
{ return;
}
//-----------------------------------------------------------------------------
//    End drawing when a Quad is used
//-----------------------------------------------------------------------------
void CFuiComponent::DrawAsQuad(U_INT obj)
{ int wd		= surface->xSize;
  int ht		= surface->ySize;
  double x0 = surface->xScreen;
  double y0 = surface->yScreen; //globals->mScreen.Height - surface->yScreen - ht;
	SetQuad(wd,ht);
  glBindTexture(GL_TEXTURE_2D,obj);
	//-----------------------------------------------------------------
  //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
  //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,wd,ht,0,GL_RGBA,GL_UNSIGNED_BYTE,surface->drawBuffer);
  //glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	//----------------------------------------------------------------
	glPushMatrix();
	glTranslated(x0,y0,0);
  //----Draw the panel as a textured square  -----------------------
  glInterleavedArrays(GL_T2F_V3F,0,Pan);
  glDrawArrays(GL_QUADS,0,4);
	glDrawArrays(GL_LINE_LOOP,0,4);
  //----------------------------------------------------------------
  glPopMatrix();
  // Check for an OpenGL error
    { GLenum e = glGetError ();
  if (e != GL_NO_ERROR) 
    WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  }

}

//=======================================================================
//
// CFuiWindowTitle
//
//=======================================================================
CFuiWindowTitle::CFuiWindowTitle (int px, int py, int w, int h, CFuiComponent *win)
: CFuiComponent (px, py, w, h, win)
{ widgetTag = 'defa';
  wName		= "WindowTitle";
  colText = MakeRGB (255, 255, 255);
  font    = FuiFont ('deff');
  fnts    = (CFont*)font->font;
  textw   = 0;
}
//---------------------------------------------------------------------
CFuiWindowTitle::~CFuiWindowTitle ()
{
 }
//---------------------------------------------------------------------
//  Initialize the title bar
//---------------------------------------------------------------------
void CFuiWindowTitle::InitTitle (char *txt)
{ FindThemeWidget ();
  backText  = MakeRGBA(137, 157, 178, 255);
  strncpy(text,txt,256);
  this->h   = CreateHBox(hBox,w);
  this->y  -= h;
  // Get text colour
  colText   = tw->GetColour ("TEXT");
  MakeSurface();
  EditTitle();
}

//---------------------------------------------------------------------
//  Change Title bar
//---------------------------------------------------------------------
void CFuiWindowTitle::ChangeTitle(char * txt)
{ strncpy (text, txt, 255);
  EditTitle();
}
//----------------------------------------------------------------
//  Change parent position
//----------------------------------------------------------------
void CFuiWindowTitle::MoveParentTo (int xp, int yp)
{ CFuiComponent::MoveParentTo (xp, yp);
  // Update all children
  std::list<CFuiComponent*>::iterator i;
  for (i=decorationList.begin(); i!=decorationList.end(); i++) {
    (*i)->MoveParentTo(surface->xScreen, surface->yScreen);
  }
  return;
}

//----------------------------------------------------------------
//  Edit the title text
//---------------------------------------------------------------
void CFuiWindowTitle::EditTitle()
{ int xT = (w >> 1) - (textw >> 1) - 4;
  int wT = textw + 4;
  // Draw a clear rect below title
  FillRect (surface, xT, 4, xT + wT, h-3, 0);
  int th = fnts->TextHeight ("H");
  fnts->DrawTextC (surface,(w >> 1) ,((h-th)>> 1) ,colText,text);
  textw  = fnts->TextWidth (text);
  return;
}
//----------------------------------------------------------------
//  Draw the title bar
//----------------------------------------------------------------
void CFuiWindowTitle::Draw (void)
{ DrawHBox(hBox);
  CFuiComponent::Draw ();
}
//----------------------------------------------------------------
//  Mouse click
//----------------------------------------------------------------
bool CFuiWindowTitle::MouseClick(int mx,int my,EMouseButton btn)
{ if (!MouseHit(mx,my))         return false;
  if (btn!= MOUSE_BUTTON_LEFT)  return true;
  MoWind->MoveWindow(mx,my);
  return true;
}
//----------------------------------------------------------------
//  Resize by delta x and delta y
//----------------------------------------------------------------
void CFuiWindowTitle::Redim(short dx,short dy)
{ if (0 == dx)  return;
  if (hBox[2])  hBox[2]->Redim(dx,0);
  if (hBox[1])  hBox[1]->MoveBy(dx,0);
  w += dx;
  
  MakeSurface();
  EditTitle();
  return; 
}
//===============================================================================
// CFuiCloseButton
//===============================================================================
CFuiCloseButton::CFuiCloseButton (int x, int y, int w, int h, CFuiComponent *win)
: CFuiComponent (x, y, w, h, win)
{
  type = COMPONENT_CLOSE_BUTTON;
  widgetTag = 'defa';
  wName		= "WindowCloseButton";
  id      = 'sysb';                         // System button
  bmBack  = 0;
  FindThemeWidget ();

  bmBack = tw->GetBitmap ("BACK");
  if (0 == bmBack) return;
  bmBack->GetBitmapSize (&w, &h);
  MakeSurface();
  SetFrame(0);
  return;
}
//------------------------------------------------------------------------
//  Set Frame and edit bitmap
//------------------------------------------------------------------------
void CFuiCloseButton::SetFrame(short fr)
{ Frame = fr;
  if (bmBack) bmBack->DrawBitmap(surface,0,0,fr);
  return;
}
//-----------------------------------------------------------------------
//  Mouse move over. 
//-----------------------------------------------------------------------
bool CFuiCloseButton::MouseMove (int mx, int my)
{ if (!MouseHit(mx,my))  return false;
  return true;
}
//-----------------------------------------------------------------------
//  Mouse Click
//-----------------------------------------------------------------------
bool CFuiCloseButton::MouseClick (int x, int y, EMouseButton button)
{   if (!MouseHit(x,y)) return false;
    SetFrame (1);
    return true;
}
//-----------------------------------------------------------------------
//  Stop click.  Close Mother Windows
//-----------------------------------------------------------------------
bool CFuiCloseButton::MouseStopClick (int x, int y, EMouseButton button)
{ SetFrame(0);
  MoWind->NotifyChildEvent(id,id,EVENT_CLOSEWINDOW);
  return true;
}
//===============================================================================
// CFuiMinimizeButton
//================================================================================
CFuiMinimizeButton::CFuiMinimizeButton (int x, int y, int w, int h, CFuiComponent *win)
: CFuiComponent (x, y, w, h, win)
{
  type = COMPONENT_MINIMIZE_BUTTON;
  widgetTag = 'defa';
  wName		= "WindowMinimizeButton";
  id      = 'sysb';                         // System button
  bmBack = NULL;
  FindThemeWidget ();
  bmBack = tw->GetBitmap ("BACK");
  if (0 == bmBack) return;
  bmBack->GetBitmapSize (&w, &h);
  MakeSurface();
  SetFrame(0);
  return;
}
//------------------------------------------------------------------------
//  Set Frame and edit bitmap
//------------------------------------------------------------------------
void CFuiMinimizeButton::SetFrame(short fr)
{ frame = fr;
  if (bmBack) bmBack->DrawBitmap(surface,0,0,fr);
  return;
}
//-------------------------------------------------------------------------
//  Mouse move over
//-------------------------------------------------------------------------
bool CFuiMinimizeButton::MouseMove (int mx, int my)
{ if (!MouseHit(mx,my))  return false;
  return true;
}
//-----------------------------------------------------------------------
//  Mouse Click
//-----------------------------------------------------------------------
bool CFuiMinimizeButton::MouseClick (int x, int y, EMouseButton button)
{   if (!MouseHit(x,y)) return false;
    SetFrame (1);
    MoWind->NotifyChildEvent(id,id,EVENT_MINIMIZE);
    return true;
}
//-----------------------------------------------------------------------
//  Stop click.  Close Mother Windows
//-----------------------------------------------------------------------
bool CFuiMinimizeButton::MouseStopClick (int x, int y, EMouseButton button)
{ SetFrame(0);
  return true;
}


//=======================================================================
// CFuiZoomButton
//=======================================================================
CFuiZoomButton::CFuiZoomButton (int x, int y, int w, int h, CFuiComponent *win)
: CFuiComponent (x, y, w, h, win)
{
  type = COMPONENT_ZOOM_BUTTON;
  widgetTag = 'defa';
  wName		= "WindowZoomButton";
  id      = 'sysb';                         // System button
  bmBack = NULL;
  FindThemeWidget ();
  bmBack = tw->GetBitmap ("BACK");
  if (0 == bmBack) return; 
  bmBack->GetBitmapSize (&w, &h);
  MakeSurface();
  SetFrame(0);
  return;
}
//------------------------------------------------------------------------
//  Set Frame and edit bitmap
//------------------------------------------------------------------------
void CFuiZoomButton::SetFrame(short fr)
{ frame = fr;
  if (bmBack) bmBack->DrawBitmap(surface,0,0,fr);
  return;
}
//-------------------------------------------------------------------------
//  Mouse move over
//-------------------------------------------------------------------------
bool CFuiZoomButton::MouseMove (int mx, int my)
{ if (!MouseHit(mx,my))  return false;
  return true;
}
//-----------------------------------------------------------------------
//  Mouse Click
//-----------------------------------------------------------------------
bool CFuiZoomButton::MouseClick (int x, int y, EMouseButton button)
{   if (!MouseHit(x,y)) return false;
    SetFrame (1);
    MoWind->NotifyChildEvent(id,id,EVENT_ZOOM);
    return true;
}
//-----------------------------------------------------------------------
//  Stop click.  Close Mother Windows
//-----------------------------------------------------------------------
bool CFuiZoomButton::MouseStopClick (int x, int y, EMouseButton button)
{ SetFrame(0);
  return true;
}


//==================================================================================
// CFuiWindow
//==================================================================================
CFuiWindow::CFuiWindow (Tag wId, const char* winFilename,int wd,int ht, short lim)
: CFuiComponent(0,0,wd,ht)
{ char s[8];
  char iniParm[64];
  this->state = FUI_WINDOW_INIT;
  this->windowId = wId;
  type = COMPONENT_WINDOW;
  widgetTag = 'defa';
  wName			= "Window";
  mini = zoom = close = 0;
  mPop      = 0;
  limit     = lim;
	modal			= 0;
  //------Init the decoration items---------------------------------
  InitFBox(fBox,MSIZ);
	edge			= 0;
	//----------------------------------------------------------------
	title			= 0;
  //---Init all bitmaps to null-------------------------------------
  mBar      = 0;
  tBar      = 0;
  btc       = 0;
  btz       = 0;
  btm       = 0;
  //----Init default size and position -----------------------------
  int nb = 0;
  TagToString (s, windowId);
  sprintf (iniParm, "WindowX_%s", s);
  GetIniVar ("Windows", iniParm, &nb);
  if (nb) x = short(nb);
  nb  = 0;
  sprintf (iniParm, "WindowY_%s", s);
  GetIniVar ("Windows", iniParm, &nb);
  if (nb) y = short(nb);
  nb  = 0;
  sprintf (iniParm, "WindowW_%s", s);
  GetIniVar ("Windows", iniParm, &nb);
  if (nb) w = short(nb);
  nb  = 0;
  sprintf (iniParm, "WindowH_%s", s);
  GetIniVar ("Windows", iniParm, &nb);
  if (nb) h = short(nb);
	//--- Find theme -------------------------------
	FindThemeWidget ();
  // Open stream ----------------------------------
  SStream stream;
  if (OpenRStream ((char*)winFilename, stream)) {
    ReadFrom (this, &stream);
    CloseStream (&stream);
  }
}
//-------------------------------------------------------------------
//  Destroy the window
//-------------------------------------------------------------------
CFuiWindow::~CFuiWindow (void)
{ if (modal)	modal->ModalClose();
	std::deque<CFuiComponent*>::iterator i;
  for (i=childList.begin(); i!=childList.end(); i++) 
  { CFuiComponent *cp = (*i);
    delete (cp);
  }
}
//-------------------------------------------------------------------
//  Close modal window
//-------------------------------------------------------------------
void CFuiWindow::CloseModal()
{	if (modal)	modal->ModalClose();
	modal = 0;
}
//-------------------------------------------------------------------
//  Read all tags
//-------------------------------------------------------------------
int CFuiWindow::Read (SStream *stream, Tag tag)
{
  int dm;
  switch (tag) {
  case 'hmax':
    ReadInt(&dm,stream);
    hMax = short(dm);
    if (limit) h = hMax;
    return TAG_READ;

  case 'wsiz':
    ReadInt (&dm, stream);
    ReadInt (&dm, stream);
    return TAG_READ;

  case 'titl':
    ReadInt (&dm, stream);
    title = (U_CHAR)dm;
    return TAG_READ;

  case 'clos':
    ReadInt (&dm, stream);
    close = (U_CHAR)dm;
    return TAG_READ;

  case 'mini':
    ReadInt (&dm, stream);
    mini  = (U_CHAR)dm;
    return TAG_READ;

  case 'zoom':
    ReadInt (&dm, stream);
    zoom  = (U_CHAR)dm;
    return TAG_READ;

  case 'rsiz':
    ReadInt (&dm, stream);
    if (dm) SetProperty(FUI_XY_RESIZING);
    return TAG_READ;

  case 'move':
    ReadInt (&dm, stream);
    move  = (U_CHAR)dm;
    return TAG_READ;

  case 'bord':
    ReadInt (&dm, stream);
    border = (U_CHAR)dm;
    return TAG_READ;

  case 'bsiz':
    ReadInt (&dm, stream);
    borderSize = (U_CHAR)dm;
    return TAG_READ;

  case 'layr':
    { int i;
      ReadInt (&i, stream);
    }
    return TAG_READ;

  case 'trns':
    ReadInt (&dm, stream);
    if (dm) SetProperty(FUI_TRANSPARENT);
    return TAG_READ;

  case 'save':
    ReadInt (&dm, stream);
    save = (U_CHAR)dm;
    return TAG_READ;

  case 'mbar':
    CreateMenuBar(stream);
    return TAG_READ;

  case 'quad':
    SetProperty(FUI_HAS_QUAD);
    return TAG_READ;
  }

  // Add subcomponent if applicable. Eliminate if position > hMax
  if (ValidFuiComponentType (tag)) {
      
      CFuiComponent *c = CreateFuiComponent ((EFuiComponentTypes)tag,this);
      if (c == NULL) 
      { char s[8];
        WARNINGLOG ("CFuiWindow : Skipping unsupported widget %s", TagString (s, tag));
        SkipObject (stream);
        return TAG_READ;
      }
      ReadFrom (c, stream);
      if (limit && (c->GetYPosition() > hMax))  
      { delete c;
        return TAG_READ;
      }
      c->MoveParentTo (x, y);
      childList.push_front (c); // in reverse order for drawing /focus reason
      return TAG_READ;
  }

  // Send tag to parent class for processing.
  return CFuiComponent::Read (stream, tag);
}
//----------------------------------------------------------------------
//  Set background picture
//----------------------------------------------------------------------
void CFuiWindow::SetBackPicture(char *fn)
{	if (fBox[BAKW])	gtfo("Windows has already a back bitmap");
	int wr,hr;
  fBox[BAKW] = CreateFBoxSide(this,fn,0,0, &wr, &hr);
	return;
}
//----------------------------------------------------------------------
//  Helper to add a component
//----------------------------------------------------------------------
void CFuiWindow::AddChild(Tag idn,CFuiComponent *cmp, char *txt, U_INT p, U_INT c)
{ cmp->SetId(idn);
	cmp->SetProperty(p);
  cmp->ReadFinished();
	if (c)		cmp->SetColour(c);
  if (txt)  cmp->SetText(txt);
  childList.push_back(cmp);
  return;
}

//--------------------------------------------------------------------------
//  Set Child property
//--------------------------------------------------------------------------
void CFuiWindow::SetChildProperty(Tag idn,U_INT p)
{ CFuiComponent *cp = GetComponent(idn);
  if (cp)  cp->SetProperty(p);
  return;
}
//--------------------------------------------------------------------------
//  Set Child Text
//--------------------------------------------------------------------------
void CFuiWindow::SetChildText(Tag idn,char *txt)
{ CFuiComponent *cp = GetComponent(idn);
  if (cp)  cp->SetText(txt);
  return;
}
//-------------------------------------------------------------------------------
//  Set Transparent mode (after creation)
//-------------------------------------------------------------------------------
void CFuiWindow::SetTransparentMode()
{ SetProperty(FUI_TRANSPARENT);
  CFuiComponent *cb = fBox[BAKW];
  if (0 == cb)        return;
  std::list<CFuiComponent*>::iterator i;
  std::list<CFuiComponent*>::iterator e;
  for (i=decorationList.begin(); i!=decorationList.end();)
  { e = i++;
    CFuiComponent *cp = (*e);
    if (cp != cb) continue;
    decorationList.erase(e);
    delete cb;
    fBox[BAKW] = 0;
    return;
  }
  return;
}
//-------------------------------------------------------------------------------
//  Create Menu Bar on title is any
//
//-------------------------------------------------------------------------------
void CFuiWindow::CreateMenuBar(SStream *str)
{ mBar = new CFuiMenuBar(200,this);
  ReadFrom (mBar, str);
  mBar->MoveParentTo(x, y);
  childList.push_front(mBar);     // in reverse order for drawing /focus reason
  return;
}
//-------------------------------------------------------------------------------
//  All parameters are read.  Create the real window now
//-------------------------------------------------------------------------------
void CFuiWindow::ReadFinished (void)
{
  // Update window size/position from INI settings if they are defined
  char s[8];
  TagToString (s, id);
  CFuiComponent::ReadFinished ();
  //--Create all decorations --------------------------------------
  CreateFBox(this,fBox,x,y,w,h);
  //--Process resize parameters is allowed -------------------- ----
	Tag xcs = globals->cum->BindFuiCursor("UI/CURSORS/CURSOR_RESIZEX.PBM",'sizr');
  Tag ycs = globals->cum->BindFuiCursor("UI/CURSORS/CURSOR_RESIZEY.PBM",'sizl');
  Tag bcs = globals->cum->BindFuiCursor("UI/CURSORS/CURSOR_RESIZE.PBM",'sizc');
  vStat = WINDOW_SIZE_NORM;
	CFuiComponent *cmp = 0;
	cmp		= fBox[RIGW];					// Right edge for X redim
	if (cmp && HasProperty(FUI_HZ_RESIZING))	cmp->SetResize(xcs,WINDOW_RESIZE_XDIM);
	cmp		= fBox[BOTW];					// bottom edge for Y redim
	if (cmp && HasProperty(FUI_VT_RESIZING))	cmp->SetResize(ycs,WINDOW_RESIZE_YDIM);
  cmp		= fBox[BRGW];					// corner edge for X-Y redim
	if (cmp && HasProperty(FUI_XY_RESIZING))	cmp->SetResize(bcs,WINDOW_RESIZE_BOTH);
  //-----------------------------------------------------------------
  // Locals for bitmap width/height
  int bmh;
  if (title) {
    // Create title bar
    /// \todo Use theme to determine window title type
    tBar = new CFuiWindowTitle (-lMG, -tMG, (w + lMG + rMG), 0,this);
    tBar->InitTitle(text);
    tBar->MoveParentTo (x, y);
    fBox[TITL]  = tBar;
    decorationList.push_back (tBar);
  }
  Top = (tBar)?(tBar->GetHeight()):(0);
  //-- Compute window overall rectangle --------------------
  Rect.x0 = -lMG;
  Rect.x1 = +rMG + w;
  Rect.y0 = -tMG - Top;
  Rect.y1 = +bMG + h;
  //-- Compute small button positions ----------------------
  int xpos = 0;
  int ypos = 0;
  if (tBar) tBar->GetPosition(&xpos,&ypos);
  xpos    += (w - 20);
  ypos    += 4;

  // Add optional window  components
  if (close) {
    /// \todo Use themed component type
    btc = new CFuiCloseButton (xpos ,ypos, 20, 20,this);
    btc->MoveParentTo (x, y);
    fBox[CLOS]  = btc;
    childList.push_back (btc);
    xpos -= 20;
  }
  if (zoom) {
    /// \todo Use themed component type
    /// \todo y-coordinate should be based on title bar height
    btz = new CFuiZoomButton (xpos,ypos, 20, 20, this);
    btz->MoveParentTo (x, y);
    fBox[ZOOM] = btz;
    childList.push_back (btz);
    xpos -= 20;
  }
  if (mini) {
    /// \todo Use themed component type
    /// \todo y-coordinate should be based on title bar height
    btm = new CFuiMinimizeButton (xpos, ypos, 20, 20,this);
    btm->MoveParentTo (x, y);
    fBox[MINI]  = btm;
    childList.push_back (btm);
  }
  //----Resize parameters ----------------------------------------------
  xMini = w;
  xMaxi = w;
  yMini = h;
  yMaxi = h;
  // Compute mid point drawing surface --------------------------------
  halfW = (w >> 1);          // Half wide
  halfH = (h >> 1);          // Half height
  //----Relocate Menu Bar ----------------------------------------------
  bmh = (tBar)?(tBar->GetHeight()):(0);
  if (mBar) mBar->MoveTo(0,-bmh);
  //----------- Set window state to open -------------------------------
  globals->fui->ActivateWindow(this);
}
//----------------------------------------------------------------------
//  Add a zoom button if not ppresent
//----------------------------------------------------------------------
void CFuiWindow::AddZoomButton()
{ if (btz)      return;
  if (0 == btc) return;
  int xpos = 0;
  int ypos = 0;
  if (btc) btc->GetPosition(&xpos,&ypos);
  btz = new CFuiZoomButton (xpos - 20,ypos, 20, 20,this);
  btz->MoveParentTo (x, y);
  fBox[ZOOM] = btz;
  childList.push_back (btz);
  return;
}
//----------------------------------------------------------------------
//  Add a Minimize button if not ppresent
//----------------------------------------------------------------------
void CFuiWindow::AddMiniButton()
{ if (btm)      return;
  if (0 == btc) return;
  int xpos = 0;
  int ypos = 0;
  if (btc) btc->GetPosition(&xpos,&ypos);
  if (btz) btz->GetPosition(&xpos,&ypos);
  btm = new CFuiMinimizeButton (xpos - 20,ypos, 20, 20,this);
  btm->MoveParentTo (x, y);
  fBox[ZOOM] = btm;
  childList.push_back (btm);
  return;
}
//----------------------------------------------------------------------
//  Database management.  Post a request tto the database and update state
//----------------------------------------------------------------------
void CFuiWindow::PostRequest(CDataBaseREQ *req)
{ req->SetState(1);
  globals->dbc->PostRequest(req);
  return;
}
//----------------------------------------------------------------------------------
//  Inside mouse click
//  Helper to start moving an image described by the S_IMAGE structure
//----------------------------------------------------------------------------------
bool CFuiWindow::ClickImage (int mx, int my, EMouseButton button,S_IMAGE &info)
{ RegisterFocus(0);
  if  (button != MOUSE_BUTTON_LEFT)   return true;
  info.state  = 1;
  info.mx     = mx;
  info.my     = my;
  return true;
}
//----------------------------------------------------------------------------------
//  Inside mouse move
//  Helper to maintain move state inside a picture described by the S_IMAGE
//  info structure
//  Controle ensure that the image is not going ouside the window
//----------------------------------------------------------------------------------
bool CFuiWindow::MoveImage(int mx,int my,S_IMAGE &info)
{ if (info.state != 1)  return true;
  short dx  = mx - info.mx;
  short dy  = my - info.my;
  info.mx   = mx;
  info.my   = my;
  info.x0  += dx;
  info.y0  += dy;
  return true;
}
//----------------------------------------------------------------------------------
//  Rest image position
//--------------------------------------------------------------------------------
bool CFuiWindow::ResetImage(S_IMAGE &info)
{ info.x0 = 0;
  info.y0 = 0;
  return true;
}
//--------------------------------------------------------------------------------
//  Return to previous size
//--------------------------------------------------------------------------------
void CFuiWindow::NormSize()
{ vStat = WINDOW_SIZE_NORM;
  ResizeTo((prevW - w),(prevH - h));
  return;
}
//--------------------------------------------------------------------------------
//  Maximize size
//--------------------------------------------------------------------------------
void CFuiWindow::MaxiSize()
{ prevW = w;
  prevH = h;
  vStat = WINDOW_SIZE_MAXI;
  ResizeTo((xMaxi - w),(yMaxi -h));
  return;
}
//--------------------------------------------------------------------------------
//  Minimize size
//--------------------------------------------------------------------------------
void CFuiWindow::MiniSize()
{ prevW = w;
  prevH = h;
  vStat = WINDOW_SIZE_MINI;
  ResizeTo((xMini - w),(yMini -h));
  return;

}
//--------------------------------------------------------------------------------
//  SYSTEM BUTTON HANDLER
//--------------------------------------------------------------------------------
void CFuiWindow::SystemHandler(EFuiEvents evn)
{ switch (evn)  {
  case EVENT_CLOSEWINDOW:
    Close();
    return;
  case EVENT_MINIMIZE:
    (WINDOW_SIZE_MINI == vStat)?(NormSize()):(MiniSize());
    return;
  case EVENT_ZOOM:
    (WINDOW_SIZE_MAXI == vStat)?(NormSize()):(MaxiSize());
    return;
}
  return;
}
//--------------------------------------------------------------------------------
//  Hide window
//--------------------------------------------------------------------------------
void CFuiWindow::HideMe()
{ globals->fui->HideWindow(this);
}
//--------------------------------------------------------------------------------
//  Show window
//--------------------------------------------------------------------------------
void CFuiWindow::ShowMe()
{ globals->fui->ShowWindow(this);
}
//--------------------------------------------------------------------------------
//  Window is closing
//--------------------------------------------------------------------------------
void CFuiWindow::Close()
{	state = FUI_WINDOW_CLOSED; }
//--------------------------------------------------------------------------------
// Close modal window
//--------------------------------------------------------------------------------
void CFuiWindow::ModalClose()
{	MoWind = 0;
	state = FUI_WINDOW_CLOSED;
}
//-------------------------------------------------------------------------------
//	Call to check profile
//	By default:  When editing, dont allow creation and drawing
//	Windows that must be present when editing will override this function
//-------------------------------------------------------------------------------
bool CFuiWindow::CheckProfile(char a)
{	char edt = globals->aPROF.Has(PROF_EDITOR);
	if (edt)	 state = FUI_WINDOW_CLOSED;
	return (edt == 0);	}
//-------------------------------------------------------------------------------
//  Add screen coordinate from surface to (X,Y)
//-------------------------------------------------------------------------------
void CFuiWindow::SurfaceOrigin(int &x,int &y)
{ if (0 == surface)   return;
  x += surface->xScreen;
  y += surface->yScreen;
  return;
}
//-------------------------------------------------------------------------------
//  Returen window Rectangle in pixels
//-------------------------------------------------------------------------------
void CFuiWindow::WindowRect(C_2DRECT &r)
{ if (0 == surface) return;
  r.x0  = surface->xScreen;
  r.y0  = surface->yScreen + surface->ySize;
  r.x1  = r.x0 + surface->xSize;
  r.y1  = surface->yScreen;
  return;
}
//-------------------------------------------------------------------------------
//  Return window ViewPort in pixels
//-------------------------------------------------------------------------------
void CFuiWindow::WindowViewPort(VIEW_PORT &v)
{ if (0 == surface) return;
  v.ht  = surface->ySize;
  v.wd  = surface->xSize;
  v.x0  = surface->xScreen;
  v.y0 = globals->mScreen.Height - surface->yScreen - v.ht;
  return;
}
//-------------------------------------------------------------------------------
//  Start drawing primitive inside the windows
//  Units must be pixels inside the window x in [0,w] and y in [0,h]
//  (0,0) is the lower corner
//  All primitives are clipped inside the surface rectangle
//-------------------------------------------------------------------------------
void CFuiWindow::DebDrawInside()
{ VIEW_PORT v;
  WindowViewPort(v);                          // Set window viewport
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();                             // Save model view
  glMatrixMode(GL_PROJECTION);                // Mode projection               
  glPushMatrix();                             // Save projection
  glLoadIdentity();
  glGetIntegerv(GL_VIEWPORT,(GLint*)&vps);    // Save actual viewport
  gluOrtho2D(0, v.wd, 0, v.ht);               // Limit world to viewport
  glViewport(v.x0,v.y0,v.wd,v.ht);            // Set the viewport
  return;

}
//-------------------------------------------------------------------------------
//  Stop drawing primitive inside the windows
//--------------------------------------------------------------------------------
void CFuiWindow::EndDrawInside()
{ glViewport(vps.x0,vps.y0,vps.wd,vps.ht);    // Restore previous viewport
  glPopMatrix();                              // Restore projection
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();                              // Restore model view
  glPopAttrib();                              // Restore attributes

  return;
}
//-------------------------------------------------------------------------------
//  Move the windows
//-------------------------------------------------------------------------------
void CFuiWindow::MoveWindow(int mx,int my)
{ state = FUI_WINDOW_MOVE;
  lastX = mx;
  lastY = my;
  return;
}
//-------------------------------------------------------------------------------
//  Notified event Obsolete
//-------------------------------------------------------------------------------
void CFuiWindow::EventNotify(Tag wtg,Tag cpn,EFuiEvents evn,EFuiEvents sub)
{  return;
}
//-----------------------------------------------------------------------------
//  Draw the window decoration and child
//-----------------------------------------------------------------------------
void CFuiWindow::Draw (void)
{ 
  //----Drawing  windows c omponents  ---------
  DrawFBoxBack(fBox[BAKW]);
  DrawFBox(fBox,MSIZ);
  //---- Draw child components ----------------
  std::deque<CFuiComponent*>::reverse_iterator ir;
  for (ir=childList.rbegin(); ir!=childList.rend(); ir++) 
  {   (*ir)->Draw();
  }
  if (mPop) mPop->Draw();
}
//-------------------------------------------------------------------------------
//  Modify visibility of the child component
//-------------------------------------------------------------------------------
void CFuiWindow::ModifyShow(Tag idn, bool vs)
{ //----Find the child -----------------------------
  std::deque<CFuiComponent*>::iterator ide;
  for (ide=childList.begin(); ide!=childList.end(); ide++) {
    if ((*ide)->GetId() != idn) continue;
    (*ide)->Show(vs);
    return;
  }
  return;
}
//-------------------------------------------------------------------------------
// Set the position of the window relative to the top-left corner of the screen
//-------------------------------------------------------------------------------
void CFuiWindow::MoveBy (int dx, int dy)
{
  CFuiComponent::SetPosition (dx + x, dy + y);
  int nx = surface->xScreen;
  int ny = surface->yScreen;
  // Update parent position of window decorations
  std::list<CFuiComponent*>::iterator i;
  for (i=decorationList.begin(); i!=decorationList.end(); i++) {
    (*i)->MoveParentTo (nx, ny);
  }

  // Update parent position of child widgets
  std::deque<CFuiComponent*>::iterator ide;
  for (ide=childList.begin(); ide!=childList.end(); ide++) {
    (*ide)->MoveParentTo(nx, ny);
  }
}
//-------------------------------------------------------------------------------
// Set the  absolute position of the window in the screen
//-------------------------------------------------------------------------------
void CFuiWindow::MoveTo(int sx, int sy)
{ sx  = CheckWD(sx);
  sy  = CheckHT(sy);
  if (sy < 24)  sy = 24;
  CFuiComponent::SetPosition (sx,sy);
  int nx = surface->xScreen;
  int ny = surface->yScreen;
  // Update parent position of window decorations
  std::list<CFuiComponent*>::iterator i;
  for (i=decorationList.begin(); i!=decorationList.end(); i++) {
    (*i)->MoveParentTo (nx, ny);
  }

  // Update parent position of child widgets
  std::deque<CFuiComponent*>::iterator ide;
  for (ide=childList.begin(); ide!=childList.end(); ide++) {
    (*ide)->MoveParentTo(nx, ny);
  }
  return;
}
//-----------------------------------------------------------------------------
//  Keyboard input
//  Send ky to the focused component if any
//-----------------------------------------------------------------------------
bool CFuiWindow::KeyboardInput(U_INT key)
{ if (0 == cFocus)    return false;
  return cFocus->KeyboardInput(key);
}
//------------------------------------------------------------------------------
//  Register a popup menu
//------------------------------------------------------------------------------
void CFuiWindow::RegisterPopup(CFuiPage *pop)
{ mPop   = pop;
  return;
}
//------------------------------------------------------------------------------
//  Build groupbox with Airport and Object Hide button
//------------------------------------------------------------------------------
CFuiGroupBox *CFuiWindow::BuildGroupEdit(int x,int y)
{	CFuiGroupBox *box = new CFuiGroupBox(x,y,176,28,this);
	gBox	= box;
  //----Create Object button ----------------------
  CFuiButton *bto  =  new CFuiButton( 4,2, 80,20,this);
	box->AddChild('obtn',bto,ObjBTN[obtn]);
	globals->noOBJ += obtn;
	//--- Create Airport button ---------------------
	CFuiButton *bta		= new CFuiButton(90,2,80,20,this);
	box->AddChild('abtn',bta,AptBTN[abtn]);
	globals->noAPT += abtn;
	//--- Add the group ------------------------------
	AddChild('gped',box,"");
	return box;}
//------------------------------------------------------------------------------
//  Swap edit button
//------------------------------------------------------------------------------
int CFuiWindow::SwapGroupButton(Tag btn, char *zbt)
{ *zbt ^= 1;
   gBox->SetChildText(btn,ObjBTN[*zbt]);
   return btnLOK[*zbt];
}
//------------------------------------------------------------------------------
//	Create a modal search file box
//------------------------------------------------------------------------------
void CFuiWindow::CreateFileBox(FILE_SEARCH *fpm)
{	if (modal)					return;
	CFuiFileBox *fbox = new CFuiFileBox(this,fpm);
	modal							= fbox;
	fbox->MoveTo(200,200);
	return;
}
//------------------------------------------------------------------------------
//	Create a modal Dialog box
//------------------------------------------------------------------------------
void CFuiWindow::CreateDialogBox(char *ttl, char *msg, char nb)
{	if (modal)					return;
	CFuiErrorMSG *win = new CFuiErrorMSG(this,nb);
	win->SetTitle(ttl);
	modal							= win;
	win->Display(msg);
	return;
}
//------------------------------------------------------------------------------
/// Override check for mouse hit to account for window decorations
//  JS TODO: There may be mouse click while the window has not yet created 
//          the surface.  Thus the window should be active only when
//          surface is created, after ReadFinished()
//------------------------------------------------------------------------------
bool CFuiWindow::WindowHit (int sx, int sy)
{ if (mPop && (mPop->MouseHit(sx,sy)))        return true;
  int rx = sx - surface->xScreen;
  int ry = sy - surface->yScreen;
  if ((rx < Rect.x0) || (sx > (x + Rect.x1))) return false;
  if ((ry < Rect.y0) || (sy > (y + Rect.y1))) return false;
  return true;
}
///-----------------------------------------------------------------------------
/// Mouse movement handler
///
///-----------------------------------------------------------------------------
bool CFuiWindow::MouseMove (int mx, int my)
{ if (state == FUI_WINDOW_MOVE) {
    int dx = mx - lastX;
    int dy = my - lastY;

    MoveBy (dx, dy);
    lastX = mx;
    lastY = my;
    return true;
  }
  //---------Try popup ---------------------------------------------
  if (mPop && (mPop->MouseMove(mx,my)))     return true;
  //-------- Otherwise send to child widget ------------------------
  std::deque<CFuiComponent*>::iterator i;
  for (i=childList.begin(); i!=childList.end(); i++) if ((*i)->MouseMove(mx,my))	return true;
	//---------See for resize on window edges ------------------------
  if (CheckResize(mx,my))										return true;
  if (0 == edge)	globals->cum->SetArrow();
  return (InsideMove(mx,my));
}
//-------------------------------------------------------------------------------
//  Check for Resize
//-------------------------------------------------------------------------------
bool CFuiWindow::CheckResize(int mx, int my)
{	if (!HasProperty(FUI_XY_RESIZING))		return false;
  edge	= (CFuiPicture *)fBox[RIGW];
  if ((edge) && edge->MoveOver(mx,my))	return true;
  edge	= (CFuiPicture *)fBox[BOTW];
  if ((edge) && edge->MoveOver(mx,my))	return true;
  edge	= (CFuiPicture *)fBox[BRGW];
  if ((edge) && edge->MoveOver(mx,my))  return true;
	edge	= 0;
  return false;
}
//-------------------------------------------------------------------------------
//  Click inside
//-------------------------------------------------------------------------------
bool CFuiWindow::InsideClick (int mx,int my, EMouseButton button)
{ ClearFocus(0);
  return((MouseHit(mx,my) && (!HasProperty(FUI_TRANSPARENT))));
}
//-------------------------------------------------------------------------------
//  Focus Management.  Where is the mouse? 
//  The MouseClick event follows a priority scheme that must not be altered
//-------------------------------------------------------------------------------
bool CFuiWindow::MouseClick (int mx, int my, EMouseButton button)
{ if ((mPop) && (mPop->MouseClick(mx, my, button)))  return RegisterFocus(mPop);
  if ((mBar) && (mBar->MouseClick(mx, my, button)))  return RegisterFocus(mBar);
  // If mouse click has not been handled yet, send to child widgets
  CFuiComponent * comp = NULL;
  std::deque<CFuiComponent*>::iterator i;
  for (i=childList.begin(); i!=childList.end(); i++) 
  { if ((*i)->NoMouse())                  continue;
    if ((*i)->MouseClick(mx, my, button)) return RegisterFocus(*i);}
  //------------Check for title move --------------------------------
  if ((tBar) && (tBar->MouseClick(mx, my, button))) return RegisterFocus(tBar);
	//--- Try resize first -------------------------------------------
	if (edge) {edge->ClickOver(mx,my);			return RegisterFocus(edge);}
  //----Check if clicked inside ------------------------------------
  if (InsideClick(mx,my,button))          return true;
  if (0 == mPop)  ClearFocus(0);
	return false;
	}
//---------------------------------------------------------------------------------
//  Mouse stop click.  Check for which component
//----------------------------------------------------------------------------------
bool CFuiWindow::MouseStopClick (int mx, int my, EMouseButton button)
{
  if ((state == FUI_WINDOW_MOVE) && (button == MOUSE_BUTTON_LEFT)) {
    state = FUI_WINDOW_OPEN;
    return true;
  }
  globals->cum->SetArrow();    
  // If mouse click has not been handled yet, send to focused component
  if (cFocus && (cFocus != this) && (cFocus->MouseStopClick (mx, my, button)))   return true;
  StopClickInside(mx, my, button);
  return MouseHit(mx,my);
}
//---------------------------------------------------------------------------------
//  Mouse double click.  Send to last register component
//----------------------------------------------------------------------------------
bool CFuiWindow::DoubleClick(int mx,int my,EMouseButton bt)
{ if ((cFocus) && (cFocus != this)) return cFocus->DoubleClick(mx,my,bt);
  return false;
}
//---------------------------------------------------------------------------------
//  Notification to resize
//  1-Check new dimension
//  2-Set the new parameters 
//  3-Make a new surface
//  4-Relocate some child when change in Y direction
//  5-Redraw window decorations with new dimensions
//  6-Notify the derived class
//---------------------------------------------------------------------------------
void CFuiWindow::ResizeTo(short dx,short dy)
{ int nw = w + dx;
  if ((nw < xMini) || (nw > xMaxi))  return;
  int nh = h + dy;
  if ((nh < yMini) || (nh > yMaxi))  return;
  if (WINDOW_SIZE_NORM == vStat) {prevW = w; prevH = h;}
  h = nh;
  w = nw;
  halfW = (w >> 1);
  halfH = (h >> 1);
  MakeSurface();
  ResizeChild(dx,dy);
  RedimFBox(fBox,dx,dy);
  if (tBar)         tBar->Redim(dx,dy);
  if (btc)          btc->MoveBy(dx,0);
  if (btz)          btz->MoveBy(dx,0);
  if (btm)          btm->MoveBy(dx,0);
  NotifyResize(dx,dy);                      // Notify derived class
  //----Recompute end point ---------------------------------------
  Rect.x0 = -lMG;
  Rect.x1 = +rMG + w;
  Rect.y0 = -tMG - Top;
  Rect.y1 = +bMG + h;
  return;
}
//-------------------------------------------------------------------------------
//  Resize the window to the requested dimensnsion
//-------------------------------------------------------------------------------
void CFuiWindow::Resize(short wd,short ht)
{ int dx = wd - w;
  int dy = ht - h;
  ResizeTo(dx,dy);
  return;
}
//-------------------------------------------------------------------------------
//  Rearrange child windows when the mother is resized
//-------------------------------------------------------------------------------
void CFuiWindow::ResizeChild(short dx,short dy)
{ std::deque<CFuiComponent*>::iterator i;
  int x = 0;
  int y = 0;
  for (i=childList.begin(); i!=childList.end(); i++)
  { CFuiComponent *cp = (*i);
    if (dy && cp->HasProperty(FUI_VT_RELOCATE)) cp->MoveBy(0,dy); 
    if (dy && cp->HasProperty(FUI_VT_RESIZING)) cp->ResizeVT(dy);
  }

  return;
}
//-------------------------------------------------------------------------------
//  Change title
//-------------------------------------------------------------------------------
void CFuiWindow::SetTitle (char* title)
{ if (tBar)  tBar->ChangeTitle(title);
  return;
}
//------------------------------------------------------------------------
//  Get a given component
//------------------------------------------------------------------------
CFuiComponent *CFuiWindow::GetComponent (Tag idc)
{ std::deque<CFuiComponent*>::iterator i;
  for (i=childList.begin(); i!=childList.end(); i++)
  { CFuiComponent *cp = (*i);
    if (cp->GetId() == idc) return cp;
    cp = cp->GetComponent(idc);
    if (cp) return cp;
  }

  return 0;
}
//-------------------------------------------------------------------------
//  Default notification
//-------------------------------------------------------------------------
void  CFuiWindow::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ if (idm == 'sysb') SystemHandler(evn);
  if (idm == 'okok') Close();
  return;
}

//================================================================================
// CFuiButton
//=================================================================================
CFuiButton::CFuiButton (int x, int y, int w, int h, CFuiComponent *wind)
: CFuiComponent (x, y, w, h, wind)
{
  type = COMPONENT_BUTTON;
  widgetTag = 'defa';
  wName	= "Button";
  ok    = 0;
  canc  = 0;
  rrpt  = 0;
  rate  = 1;
  hText = 0;
  pres  = 0;
  stop  = 0;
}
//---------------------------------------------------------------------------------
//  Delete the Hbox
//---------------------------------------------------------------------------------
CFuiButton::~CFuiButton(void)
{ 
}
//---------------------------------------------------------------------------------
//  Readall tags
//---------------------------------------------------------------------------------
int CFuiButton::Read (SStream *stream, Tag tag)
{
  int   nb;
  float ft;
  switch (tag) {
  case 'ok  ':
    ReadInt (&nb, stream);
    ok = (U_CHAR)(nb != 0);
    if (ok) id = 'okok';
    return TAG_READ;

  case 'canc':
    ReadInt (&nb, stream);
    canc  = (U_CHAR)(nb != 0);
    if (canc) id = 'canc';
    return TAG_READ;

  case 'rrpt':
    ReadInt (&nb, stream);
    if (nb)	SetProperty(FUI_REPEAT_BT);
    return TAG_READ;

  case 'rate':
    ReadFloat (&ft, stream);
    rate  = (U_CHAR)(int(ft * 25));
    return TAG_READ;
  
  case 'lsiz':
    ReadInt(&nb,stream);
    ReadInt(&nb,stream);
    return TAG_READ;

  case 'bsiz':
    ReadInt(&nb,stream);
    ReadInt(&nb,stream);
    return TAG_READ;

  }

  return CFuiComponent::Read (stream, tag);
}
//-------------------------------------------------------------------------
//  End of Tags
//-------------------------------------------------------------------------
void CFuiButton::ReadFinished (void)
{
  FindThemeWidget ();
  CreateHBox(hBox, w);
  // Get theme components
  colText = tw->GetColour ("TEXT");
  CFuiComponent::ReadFinished ();
  MoveParentTo(x,y);
  hText = fnts->TextHeight ("H");
  // Draw text centered within button
  fnts->DrawTextC (surface, (w>>1), (h-hText)>>1, colText, text);
}
//-----------------------------------------------------------------------
//  Set repeat parameters
//-----------------------------------------------------------------------
void CFuiButton::SetRepeat(float r)
{ rate  = (U_CHAR)(int(r * 25));
	SetProperty(FUI_REPEAT_BT);
  return;
}
//-------------------------------------------------------------------------
//  Change text
//-------------------------------------------------------------------------
void CFuiButton::SetText(char *txt)
{ strncpy(text,txt,254);
  text[255] = 0;
  EraseSurfaceRGBA(surface,0);
  // Draw text centered within button
  fnts->DrawTextC (surface, (w>>1), (h-hText)>>1, colText, text);
  return;
}

//-------------------------------------------------------------------------
//  Parent move : Relocate decoration
//-------------------------------------------------------------------------
void CFuiButton::MoveParentTo(int xp, int yp)
{ CFuiComponent::MoveParentTo (xp, yp);
  // Update all children
  std::list<CFuiComponent*>::iterator i;
  for (i=decorationList.begin(); i!=decorationList.end(); i++) {
    (*i)->MoveParentTo(surface->xScreen, surface->yScreen);
  }
  return;
}
//-------------------------------------------------------------------------
//  Repeat notification
//-------------------------------------------------------------------------
void CFuiButton::Repeat()
{ if (!HasProperty(FUI_REPEAT_BT))	return;
	rrpt += 1;
  if (rrpt < rate)									return;
  MoWind->NotifyChildEvent(id,id,EVENT_BUTTONPRESSED);
  rrpt = 0;
  return;
}
//--------------------------------------------------------------------
//  Relocate Button with all children
//--------------------------------------------------------------------
void CFuiButton::MoveBy(int dx, int dy)
{ CFuiComponent::MoveBy(dx, dy);
  std::list<CFuiComponent*>::iterator i;
  for (i=decorationList.begin(); i!=decorationList.end(); i++) {
    (*i)->MoveParentTo(surface->xScreen, surface->yScreen);
  }

 return;
}

//-------------------------------------------------------------------------
//    Draw the button
//-------------------------------------------------------------------------
void CFuiButton::Draw (void)
{ // Draw button components
  if (!HasProperty(FUI_IS_VISIBLE))	return;
  if (pres)													Repeat();
  DrawHBox(hBox);
  CFuiComponent::Draw ();
}
//-------------------------------------------------------------------------
//  Mouse move over the button
//-------------------------------------------------------------------------
bool CFuiButton::MouseMove (int x, int y)
{ if (!HasProperty(FUI_IS_VISIBLE)) return false;
  // If button is currently pressed and mouse is moved outside of button
  //   hit area, then set state to unpressed but don't send BUTTONPRESS event
  bool hit = MouseHit (x, y);
  if (! hit) SetHBoxFrame(hBox,0);
  return false;
}
//--------------------------------------------------------------------------------
//  Button is clicked.  Notify Mother Windows
//--------------------------------------------------------------------------------
bool CFuiButton::MouseClick (int x, int y, EMouseButton button)
{ // Mouse clicked inside button area, set state to pressed.
  if (!HasProperty(FUI_IS_VISIBLE)) return false;
  if (!MouseHit(x,y))               return false;
  rrpt  = 0;
  pres  = 1;
  SetHBoxFrame(hBox,1);
  MoWind->NotifyChildEvent(id,id,EVENT_BUTTONPRESSED);
  return true;
}
//-------------------------------------------------------------------------------
//  Mouse stop click.  Return to frame 0
//-------------------------------------------------------------------------------
bool CFuiButton::MouseStopClick (int x, int y, EMouseButton button)
{   SetHBoxFrame(hBox,0);
    pres = 0;
    if (stop) MoWind->NotifyChildEvent(id,id,EVENT_BUTTONRELEASE);
    return true;
}
//-------------------------------------------------------------------------------
//  Lost focus: stop repeat
//-------------------------------------------------------------------------------
void CFuiButton::FocusLost()
{	pres = 0;
	return;
}

//==================================================================================
// CFuiPopupMenu
//==================================================================================
CFuiPopupMenu::CFuiPopupMenu (int x, int y, int w, int h, CFuiComponent *win)
: CFuiComponent (x, y, w, h, win)
{ type = COMPONENT_POPUPMENU;
  widgetTag = 'defa';
  wName		= "PopupMenu";
  colHili = MakeRGB(255,255,255);
  colText = MakeRGB(0,0,0);
  just1 = just2 = just3 = 0;
  hCar          = 0;
  selection     = 0;
  xText         = 0;
  yText         = 0;
  Page          = 0;
  bText[0]      = 0;
}
//-----------------------------------------------------------
//  destroy this window
//-----------------------------------------------------------
CFuiPopupMenu::~CFuiPopupMenu()
{ 
  SAFE_DELETE(Page);
}
//-----------------------------------------------------------------------
//  Read parameters from tags
//-----------------------------------------------------------------------
int CFuiPopupMenu::Read (SStream *stream, Tag tag)
{ int i;
  char txt[128];
  switch (tag) {
  case 'just':
    ReadInt (&i, stream);
    just1 = (EFuiJustify) i;
    ReadInt (&i, stream);
    just2 = (EFuiJustify) i;
    ReadInt (&i, stream);
    just3 = (EFuiJustify) i;
    return TAG_READ;
  case 'item':
    ReadInt(&i,stream);
    ReadString(txt,120,stream);
    return TAG_READ;
  }

  // Send tag to parent class for processing
  return CFuiComponent::Read (stream, tag);
}

//------------------------------------------------------------------------
//  Create the popup page
//  slin is the selected line when different from 0
//------------------------------------------------------------------------
void CFuiPopupMenu::CreatePage(FL_MENU *sm,char **txt,short slin)
{ sm->Ident = id;
  sm->NbLin = 0;
  sm->aText = txt;
  int x0  = x + wCar; 
  int y0  = y + hCar;           // fix page position   + w;
  if (Page) delete Page;
  Page    = new CFuiPage(x0,y0,sm,MoWind,slin);
  Page->SetID(id);
  Page->MoveParentTo(xParent,yParent);
  return;
}  

//-----------------------------------------------------------------------
//  Clear the pane windows
//------------------------------------------------------------------------
void CFuiPopupMenu::ClearPage()
{ Page->DrawPane();
  return;
}
//------------------------------------------------------------------------
//  All tags are read
//------------------------------------------------------------------------
void CFuiPopupMenu::ReadFinished (void)
{ FindThemeWidget ();
  CFuiComponent::ReadFinished ();
  CreateHBox (hBox, w);
  // Get text colour
  colText = tw->GetColour ("TEXT");
  colHili = tw->GetColour ("SELECTION");
  //---Fix surface dimension -----------
  hCar    = fnts->TextHeight("H");
  wCar    = fnts->TextWidth ("H");
  yText   = hCar + 4;
  h       = yText + hCar;
  MakeSurface();
  //---- Update all children --------------------
  std::list<CFuiComponent*>::iterator i;
  for (i=decorationList.begin(); i!=decorationList.end(); i++) {
    (*i)->MoveBy(0, hCar+2);
  }
  fnts->DrawNText (surface,0, 0, colText, text);
}

//----------------------------------------------------------------
//  Change parent position
//----------------------------------------------------------------
void CFuiPopupMenu::MoveParentTo (int xp, int yp)
{ CFuiComponent::MoveParentTo (xp, yp);
  // Update all children
  std::list<CFuiComponent*>::iterator i;
  for (i=decorationList.begin(); i!=decorationList.end(); i++) {
    (*i)->MoveParentTo(surface->xScreen, surface->yScreen);
  }
  if (Page) Page->MoveParentTo(xParent,yParent);
  return;
}
//---------------------------------------------------------------------
//  Relocate popup with the page
//---------------------------------------------------------------------
void CFuiPopupMenu::MoveBy(int dx,int dy)
{ CFuiComponent::MoveBy(dx,dy);
  // Update all children
  std::list<CFuiComponent*>::iterator i;
  for (i=decorationList.begin(); i!=decorationList.end(); i++) {
    (*i)->MoveParentTo(surface->xScreen, surface->yScreen);
  }
  if (Page) Page->MoveBy(dx,dy);
  return;
}
//-------------------------------------------------------------------------------
//  Mouse click inside Popup
//  NOTE: The CFuiPopupMenu get the focus. It should  never give the focus to the 
//        CFuiPage.  It manages the CFuiPage state according to its own focus
//        status.
//-------------------------------------------------------------------------------
bool CFuiPopupMenu::MouseClick (int mx, int my, EMouseButton button)
{ if (!MouseHit(mx,my))           return false;
  if (HasProperty(FUI_IS_LOCKED)) return true;
  SetHBoxFrame(hBox,1);
  if (0 == Page)                  return true;
  Page->SwapState();
  return true;
}
//--------------------------------------------------------------------------------
//  Mouse stop click
//--------------------------------------------------------------------------------
bool CFuiPopupMenu::MouseStopClick (int mx, int my, EMouseButton button)
{   SetHBoxFrame(hBox,0);
    return true;
}
//-------------Notifications   -----------------------------------
void CFuiPopupMenu::NotifyChildEvent (Tag idm, Tag itm, EFuiEvents evn)
{ MoWind->NotifyChildEvent (id, itm, evn);
  CFuiComponent::NotifyChildEvent (idm, itm, evn);
}
void CFuiPopupMenu::NotifyMenuEvent (Tag idm, Tag itm)
{ CFuiComponent::NotifyMenuEvent (idm, itm);
}

void CFuiPopupMenu::EventNotify (Tag win, Tag cpn, EFuiEvents evn, EFuiEvents sub)
{  CFuiComponent::EventNotify (win, cpn, evn, sub);
}
//------------------------------------------------------------------------
//  Focus is lost
//------------------------------------------------------------------------
void CFuiPopupMenu::FocusLost()
{ SetHBoxFrame(hBox,0);
  if (Page) Page->SetState(0);
  return;
}
//---------------------------------------------------------------------
//  Draw the Popup
//---------------------------------------------------------------------
void CFuiPopupMenu::Draw (void)
{ if (0 == surface)  return;
  DrawHBox(hBox);
  CFuiComponent::Draw ();
}
//----------------------------------------------------------------------
//  Set the text button
//----------------------------------------------------------------------
void CFuiPopupMenu::SetButtonText(char *txt)
{ strncpy(bText,txt,32);
  bText[31] = 0;
  int y0  = yText;          // Fix text offset
  int yf  = y0   + hCar;
  int x0  = w >> 1;
  FillRect(surface,0,y0,w,yf,0);
  fnts->DrawTextC(surface,x0,y0, colText,bText);
  return;
}
//----------------------------------------------------------------------
//  Set the popup text
//----------------------------------------------------------------------
void CFuiPopupMenu::SetText(char *bla)
{ strncpy(text,bla,256);
  if (0 == surface)  return;
  EraseSurfaceRGBA(surface,0);
  fnts->DrawNText (surface,0 ,0 ,colText ,text);
  return;
}
//-----------------------------------------------------------------------
//  Set Selection
//-----------------------------------------------------------------------
void CFuiPopupMenu::Select(U_INT No)
{ FL_MENU *men = (Page)?(Page->GetMenu()):(0);
  if (0 == men)   return;
  SetButtonText((char*)men->aText[No]);
  Page->Select(No);
  return;
}
//-----------------------------------------------------------------------
//  Select by text
//-----------------------------------------------------------------------
void CFuiPopupMenu::SelectText(char *art)
{	FL_MENU *men = (Page)?(Page->GetMenu()):(0);
  if (0 == men)   return;
	char **atx = men->aText;
	U_INT	 No=0;
	for (int k=0; k >= 0; k++)
	{	char *itm = men->aText[k];
		if (0 == itm)							break;
		if (strcmp(itm,art) != 0) continue;
		No	= k;
		break;
	}
	Select(No);
	return;
}
//-----------------------------------------------------------------------
//  Return menu
//-----------------------------------------------------------------------
FL_MENU *CFuiPopupMenu::GetMenu()
{return (Page)?(Page->GetMenu()):(0);}
//==============================================================================
// CFuiCheckbox
//==============================================================================
CFuiCheckbox::CFuiCheckbox (int x, int y, int w, int h, CFuiComponent *win)
: CFuiComponent (x, y, w, h, win)
{
  type = COMPONENT_CHECKBOX;
  widgetTag = 'defa';
  wName			= "CheckBox";

  bmBack = NULL;
  state = 0;
  pressed = false;
  Frame = 0;
  Event[0]  = EVENT_UNCHECKED;
  Event[1]  = EVENT_CHECKED;
}
//--------------------------------------------------------------------------
//  Read all tags
//--------------------------------------------------------------------------
int CFuiCheckbox::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'stat':
    ReadInt (&state, stream);
    rc = TAG_READ;
    break;
  }

  // Send tag to parent class for processing
  if (rc == TAG_IGNORED) {
    rc = CFuiComponent::Read (stream, tag);
  }

  return rc;
}
//--------------------------------------------------------------------------
//  All tags are read
//--------------------------------------------------------------------------
void CFuiCheckbox::ReadFinished (void)
{ int dx,dy;
  CFuiComponent::ReadFinished ();
  FindThemeWidget ();
  bmBack = tw->GetBitmap ("BACK");
  txPos  = 0;
  if (0 == bmBack)    return;
  bmBack->GetBitmapSize (&dx, &dy);
  //------set text attributes -----------------------
  txPos = dx + fnts->TextWidth ("m");
  black = MakeRGB (0, 0, 0);
  fnts->DrawNText (surface ,txPos, 0, black, text);
  SetState(0);
  return;
}
//---------------------------------------------------------------------
//    Return checkbox state
//---------------------------------------------------------------------
int CFuiCheckbox::GetState (void)
{
  return state;
}
//---------------------------------------------------------------------
//  Change check box state
// Draw bitmap
// Checkbox bitmaps are 9-frame PBGs with the following frame layout:
//   0 - Unchecked normal
//   1 - Checked normal
//   2 - N/A normal
//   3 - Unchecked depressed
//   4 - Checked depressed
//   5 - N/A depressed
//   6 - Unchecked grayed-out
//   7 - Checked grayed-out
//   8 - N/A grayed-out
//---------------------------------------------------------------------
void CFuiCheckbox::SetState (int state)
{ this->state = (state)?(1):(0);
  Frame  = state;
}
//---------------------------------------------------------------------
//  Set the text
//---------------------------------------------------------------------
void CFuiCheckbox::SetText(char *bla)
{ int dx,dy;
  bmBack->GetBitmapSize (&dx, &dy);
  strncpy(text,bla,256);
  if (0 == surface)  return;
  EraseSurfaceRGBA(surface,0);
  int txPos = dx + fnts->TextWidth ("m");
  fnts->DrawNText (surface ,txPos, 0, black, text);
  return;
}
//---------------------------------------------------------------------
//  Mouse Hit.  Notify mother windows
//---------------------------------------------------------------------
bool CFuiCheckbox::MouseClick (int mx, int my, EMouseButton button)
{ if (!MouseHit (mx, my)) return false;
  state ^= 1;
  Frame = (state == 0)?(3):(4);
  bmBack->DrawBitmap(surface,  0, 0, Frame);
  MoWind->NotifyChildEvent(id,state,Event[state]);
  return true;
}
//---------------------------------------------------------------------
//    Mouse stop click
//---------------------------------------------------------------------
bool CFuiCheckbox::MouseStopClick (int mx, int my, EMouseButton button)
{ if (Frame == 3) Frame = 0;
  if (Frame == 4) Frame = 1;
  bmBack->DrawBitmap(surface,  0, 0, Frame);
  return true;
}

//---------------------------------------------------------------------
//    Draw
//---------------------------------------------------------------------
void CFuiCheckbox::Draw()
{ if (!HasProperty(FUI_IS_VISIBLE))  return;
	bmBack->DrawBitmap(surface,  0, 0, Frame);
  CFuiComponent::Draw();
	return;
}
//======================================================================
// CFuiCheckBox
//  This radio button is managed with 3 states
//  0 = unchecked in normal color
//  1 = checked normal color
//  2 = Not available
//======================================================================
CFuiCheckBox::CFuiCheckBox (int x, int y, int w, int h, CFuiComponent *win)
: CFuiComponent (x, y, w, h, win)
{
  type = COMPONENT_RADIOBUTTON;
  widgetTag = 'defa';
  wName			= "CheckBox";
  bmBT    = 0;
  state   = 0;
}
//-----------------------------------------------------------
//    Read parameters 
//-----------------------------------------------------------
int CFuiCheckBox::Read (SStream *stream, Tag tag)
{  switch (tag) {
  case 'stat':
    ReadInt (&state, stream);
    SetState (state);
    return  TAG_READ;
  }

  // Send tag to parent class for processing
    return CFuiComponent::Read (stream, tag);
}
//-----------------------------------------------------------
//    All parameters  are read
//-----------------------------------------------------------
void CFuiCheckBox::ReadFinished (void)
{ CFuiComponent::ReadFinished ();
  if (0 == surface) gtfo("CheckBox: no surface");
  FindThemeWidget();
  if (0 == bmBT) bmBT = tw->GetBitmap("BACK");
  bmBT->GetBitmapSize(&bwd,&bht);
  colText = tw->GetColour ("TEXT");
  wCar    = fnts->CharWidth('m');
  hCar    = fnts->CharHeight('H');
  //---Compute bitmap limits ----------------------
  bx0     = 0;                      // Left
  bx1     = bx0 + bwd;              // right
  by0     = h - bht;                // Top line
  by1     = h;                      // bottom line
  //-- Text position -------------------------------
  xTXT    = bwd + wCar;             // Start of text
  yTXT    = h   - hCar;             // Text line
  if (*text) EditText();
  return;
}
//--------------------------------------------------------------------------------
//  Change text
//--------------------------------------------------------------------------------
void CFuiCheckBox::SetText(char * bla)
{ strncpy(text,bla,256);
  EditText();
  return;
}
//---------------------------------------------------------------------------
//  Edit Text
//---------------------------------------------------------------------------
void CFuiCheckBox::EditText()
{ EraseSurfaceRGBA (surface,0);
  //---- Draw the text --and bitmap ------------- 
  fnts->DrawNText (surface,xTXT, yTXT, colText, text);
  bmBT->DrawBitmap(surface,0,by0,state);
  return;
}
//---------------------------------------------------------------------------
//  Set new state
//---------------------------------------------------------------------------
void CFuiCheckBox::SetState(char s)
{ if (s > 2)  return;
  state = s;
  EditText();
  return;
}
//---------------------------------------------------------------------------
// Mouse Click:  Change state if allowed 
//---------------------------------------------------------------------------
bool CFuiCheckBox::MouseClick (int mx, int my, EMouseButton button)
{ // Calculate component starting screen locations
  int cx  = xParent + x + bx0;
  int cy  = yParent + y + by0;
  bool ok = ((mx >= cx) && (mx < (cx + bwd)) && (my >= cy) && (my < (cy + bht)));
  if (!ok)  return false;
  if (state != 2) SetState(state ^ 1);         // Swap state
  MoWind->NotifyChildEvent(id,state,EVENT_BUTTONPRESSED);
  return true;
}
//---------------------------------------------------------------------------
// Mouse stop Click:  always catched 
//---------------------------------------------------------------------------
bool CFuiCheckBox::MouseStopClick (int x, int y, EMouseButton button)
{
  return true;
}
//======================================================================
// CFuiLabel
//======================================================================
CFuiLabel::CFuiLabel (int x, int y, int w, int h, CFuiComponent *win)
: CFuiComponent (x, y, w, h, win)
{ type = COMPONENT_LABEL;
  widgetTag = 'defa';
  wName	=	"Label";
  just  = JUSTIFY_H_LEFT;
  wrap  = 0;
  yText = 0;
  colText   = MakeRGB (255, 255, 255);
  redText   = MakeRGB (255, 0, 0);
  colShadow = MakeRGB (0, 0, 0);
  SetProperty(FUI_NO_MOUSE | FUI_NO_BORDER);
  FindThemeWidget ();
	// Get theme components
  colText = tw->GetColour ("TEXT");
  colShadow = tw->GetColour ("SHADOW");

}
//--------------------------------------------------------------
int CFuiLabel::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'just':
    ReadInt (&just, stream);
    rc = TAG_READ;
    break;
  case 'wrap':
    ReadInt (&wrap, stream);
    rc = TAG_READ;
    break;
  }

  // Send tag to parent class for processing
  if (rc == TAG_IGNORED) {
    rc = CFuiComponent::Read (stream, tag);
  }
  return rc;
}

//---------------------------------------------------------------------------
void CFuiLabel::ReadFinished (void)
{ CFuiComponent::ReadFinished ();
  if (just & JUSTIFY_V_TOP) yText = 0;
  else if (just & JUSTIFY_V_CENTER)  yText = (int(surface->ySize) - fnts->TextHeight("H")) >> 1;
  else if (just & JUSTIFY_V_BOTTOM)  yText =  int(surface->ySize) - fnts->TextHeight("H");
  Edit(colText);
}
//---------------------------------------------------------------------------
//  Edit label  Text
//---------------------------------------------------------------------------
void CFuiLabel::SetText(char *bla)
{ strncpy(text,bla,256);
  Edit(colText);
  return;
}
//---------------------------------------------------------------------------
//  Edit label  Text
//---------------------------------------------------------------------------
void CFuiLabel::EditText(char *fmt, ...)
{ va_list argp;
  va_start(argp, fmt);
  vsprintf(text,fmt,argp);
  Edit(colText);
  return;
}
//---------------------------------------------------------------------------
//  Edit Red  Text
//---------------------------------------------------------------------------
void CFuiLabel::RedText(char *bla)
{ strncpy(text,bla,256);
  Edit(redText);
  return;
}

//---------------------------------------------------------------------------
//  Edit Text
//---------------------------------------------------------------------------
void CFuiLabel::Edit(U_INT col)
{ if (0 == surface)     return;
  EraseSurfaceRGBA (surface,0);
  // Draw justified text
  if (just & JUSTIFY_H_LEFT)          fnts->DrawNText (surface,        0, yText, col, text);
  else if (just & JUSTIFY_H_CENTER)   fnts->DrawTextC (surface, (w >> 1), yText, col, text);
  else if (just & JUSTIFY_H_RIGHT)    fnts->DrawTextR (surface,        w, yText, col, text);
	//--- Draw border if requested ---------------------------------------------
	if (HasProperty(FUI_NO_BORDER))	return;
	DrawRect(surface,0,0,w-1,h-1,colShadow);
  return;
}
//---------------------------------------------------------------------------
//  Draw Text
//---------------------------------------------------------------------------
void CFuiLabel::Draw()
{ bool ok = (!HasProperty(FUI_BLINK_TXT)); 
	if (ok) return CFuiComponent::Draw();
  if (!globals->clk->GetON())	return;
	CFuiComponent::Draw();
	return;
}
//==========================================================================
// CFuiTextField
//===========================================================================
CFuiTextField::CFuiTextField (int x, int y, int w, int h, CFuiComponent *win)
: CFuiComponent (x, y, w, h, win)
{ type = COMPONENT_TEXTFIELD;
  widgetTag = 'defa';
  wName			= "TextField";
  //------Init the decoration items---------------------------------
  InitFBox(fBox,RSIZ);
  input   = 0;
  oInx    = 0;
  xEnd    = 0;
  //----------------------------------------------------------------
  colText = MakeRGB (0, 0, 0);
  colHighlight = MakeRGB (128, 0, 128);
  colTextHighlight = MakeRGB (255, 255, 255);
  colLine = MakeRGB (0, 0, 0);
  colFill = MakeRGB (0, 0, 0);
  colFillDisabled = MakeRGB (128, 0, 128);
  colCaret = MakeRGB (0, 0, 0);
  //----------------------------------------------------------------
  pass    = false;
  kMax    = 255;
  cpos    = 0;
  numb    = 0;
  caret   = 0xFF000000;
  time    = 0;
  mask    = 0x00FFFFFF;
	//--- Set signed number as valid characters input --------------
	vcar		= KB_NUMBER_SIGN;

}

//-------------------------------------------------------------------------
//  Read all parameters
//-------------------------------------------------------------------------
int CFuiTextField::Read (SStream *stream, Tag tag)
{ int rc = TAG_IGNORED;
  int n;
  switch (tag) {
  case 'pass':
    ReadInt (&n, stream);
    pass  = (char)n;
    return TAG_READ;

  case 'chrs':
    ReadInt (&n, stream);
    return TAG_READ;

  case 'numb':
    ReadInt (&n, stream);
    numb = (char)n;
    return TAG_READ;
  //--- Horizontal scroll --------------
  case 'hscr':
    ReadInt (&n, stream);
    return TAG_READ;

  }

  // Send tag to parent class for processing.
  if (rc == TAG_IGNORED) {
    rc = CFuiComponent::Read (stream, tag);
  }

  return rc;
}
//--------------------------------------------------------------------
//  All Tags are read
//--------------------------------------------------------------------
void CFuiTextField::ReadFinished (void)
{  CFuiComponent::ReadFinished ();
  FindThemeWidget ();
  //-------- Compute definitive drawing surface ----------------
  halfW = (w >> 1);          // Half wide
  halfH = (h >> 1);          // Half height
  CreateFBox(this,fBox,x,y,w,h);
  fmod  = 0;
  // Get text colours
  colText = tw->GetColour ("TEXT");
  colHighlight = tw->GetColour ("HILITE");
  colTextHighlight = tw->GetColour ("TEXTHILITE");
  colLine = tw->GetColour ("LINE");
  colFill = tw->GetColour ("FILL");
  colFillDisabled = tw->GetColour ("FILLDISABLED");
  colCaret = tw->GetColour ("CARET");
  bCol[0]  = colFillDisabled;
  bCol[1]  = colFill;
	if (HasProperty(FUI_TRANSPARENT)) {bCol[0] = bCol[1] = 0;}
  //---------------------------------------------------------------
  fnts    = (CFont*) font->font;
  //------------Character size ------------------------------------
  hCar    = fnts->TextHeight("H");
  wCar    = fnts->TextWidth ("h");
  kMax    = w / wCar;
  text[kMax]  = 0;
  nCar    = strlen(text);
  EditText();
}
//-------------------------------------------------------------------
//  Delete resources
//-------------------------------------------------------------------
CFuiTextField::~CFuiTextField()
{ 
}
//-------------------------------------------------------------------
//  Move to new position
//-------------------------------------------------------------------
void CFuiTextField::MoveBy(int dx,int dy)
{ CFuiComponent::MoveTo(dx + x,dy + y);
  int nx = surface->xScreen;
  int ny = surface->yScreen;
  // Update parent position of window decorations
  std::list<CFuiComponent*>::iterator i;
  for (i=decorationList.begin(); i!=decorationList.end(); i++) {
    (*i)->MoveParentTo(nx, ny);
  }

  return;
}
//-------------------------------------------------------------------
//  Set Parent position to all components
//-------------------------------------------------------------------
void CFuiTextField::MoveParentTo (int xp, int yp)
{
  CFuiComponent::MoveParentTo (xp, yp);
  // Update all decorations
  std::list<CFuiComponent*>::iterator i;
  for (i=decorationList.begin(); i!=decorationList.end(); i++) {
    (*i)->MoveParentTo (surface->xScreen, surface->yScreen);
  }
}
//------------------------------------------------------------------
//  Change edit mode
//------------------------------------------------------------------
void CFuiTextField::SetEditMode(U_CHAR md)
{ if (md) SetProperty(FUI_EDIT_MODE);
  else    RazProperty(FUI_EDIT_MODE);  
  return;
}
//------------------------------------------------------------------
//  TEXT EDITING: Shift text to right one position
//-------------------------------------------------------------------
void CFuiTextField::ShiftRight()
{ short  cx  = nCar;
  while (cx >= cpos)   text[cx+1]  = text[cx--];
  return;
}
//-------------------------------------------------------------------
//  TEXT EDITING: Shitf to upper if needed
//-------------------------------------------------------------------
char CFuiTextField::UpperChar(U_CHAR car)
{ if ((car < 'a') || (car > 'z')) return car;
  return (car & upper);
}
//-------------------------------------------------------------------
//  TEXT EDITING: Add the character and redraw text
//-------------------------------------------------------------------
int CFuiTextField::AddChar(U_CHAR kar)
{ char  car = UpperChar(kar);
  if ((nCar == kMax) || (cpos >= kMax))   return 0;
  if (text[cpos] != 0)  ShiftRight();
  text[cpos++]  = car;
  nCar++;
  text[nCar]    = 0;
  NewCaret(cpos);
  EditText();
  return 1;
}
//-------------------------------------------------------------------
//  TEXT EDITING: Shift text to left one position
//-------------------------------------------------------------------
int CFuiTextField::ShiftLeft()
{ short cx = cpos + 1;
  while (cx <= nCar) text[cx - 1] = text[cx++];
  if (nCar) nCar--;
  EditText();
  return 1;
}
//-------------------------------------------------------------------
//  TEXT EDITING: Backspace
//-------------------------------------------------------------------
int CFuiTextField::BackSpace()
{ if (0 == cpos)       return 0;
  cpos--;
  NewCaret(cpos);
  ShiftLeft();
  return 1;
}
//-------------------------------------------------------------------
//  TEXT EDITING: Set Text
//-------------------------------------------------------------------
void CFuiTextField::SetText (char *tx)
{ strncpy(text,tx,255);
  text[255] = 0;
  nCar  = strlen(text);
  cpos  = nCar;
  NewCaret(cpos);
  EditText();
	return;
}
//---------------------------------------------------------------------------
//  Edit label  Text
//---------------------------------------------------------------------------
void CFuiTextField::EditText(char *fmt, ...)
{ va_list argp;
  va_start(argp, fmt);
  vsprintf(text,fmt,argp);
  nCar  = strlen(text);
  cpos  = nCar;
  NewCaret(cpos);
  EditText();
  return;
}
//-------------------------------------------------------------------
//  TEXT EDITING: SCroll right
//-------------------------------------------------------------------
void CFuiTextField::ScrollRight()
{ if (xEnd < int(surface->xSize))   return;
  oInx++;
  EditText();
  return;
}
//-------------------------------------------------------------------
//  TEXT EDITING: left position
//-------------------------------------------------------------------
void CFuiTextField::LeftPos()
{ if (0 == cpos) return;
  cpos--;
  NewCaret(cpos);
  EditText();
  return;
}
//-------------------------------------------------------------------
//  TEXT EDITING: Right position
//-------------------------------------------------------------------
void CFuiTextField::RightPos()
{ if (cpos == nCar) return;
  cpos++;
  NewCaret(cpos);
  EditText();
  return;
}
//-------------------------------------------------------------------
//  TEXT EDITING: SCroll right
//-------------------------------------------------------------------
void CFuiTextField::ScrollLeft()
{ if (0 == oInx)      return;
  oInx--;
  EditText();
  return;
}
//-------------------------------------------------------------------
//  Keyboard input
//  if no focus, the input is not for this field
//  Edit the text into the text field
//  Note:  Text is edited if allowed
//-------------------------------------------------------------------
bool CFuiTextField::KeyboardInput(U_INT key)
{ if (0 == input)   return false;
  switch (key)  {
    case 13:          // ENTER (do not lost focus. focus is managed elsewhere)
			MoWind->NotifyChildEvent(id,id,EVENT_TEXTENTER);
      return true;
    case KB_KEY_BACK:							// BACKSPACE
      fmod |= BackSpace();
      break;
    case KB_KEY_DEL:							// DEL
      fmod |= ShiftLeft();
      break;

    case KB_KEY_LEFT:							// Left arrow
      LeftPos();
      return true;

		case KB_KEY_ENTER:
			MoWind->NotifyChildEvent(id,id,EVENT_TEXTENTER);
			return true;

		case KB_KEY_RIGHT:						// Up
      RightPos();    // Right arrow
      return true;
    default:
			//--- Check for character validity --------------
			if (key & 0x80)				return true;
			key &= 0x7F;	
			U_CHAR val = asciiVAL[key] & vcar;
			if (0 == val)					return true;
      fmod |= AddChar(key);
      break;
  }
  //---Notify if modification -------------------------
  if (fmod) MoWind->NotifyChildEvent(id,id,EVENT_TEXTCHANGED);
  fmod = 0;
  return true;
}
//--------------------------------------------------------------------------------
//  Edit the text using focus color
//  Due to variable letter size, the caret should be managed on a per letter
//  basis.
//--------------------------------------------------------------------------------
void CFuiTextField::EditText()
{ DrawFBoxBack(fBox[BAKW]);
  DrawFBox(fBox,RSIZ);
  ColorInside(bCol[input]);
  char *src = text + oInx;
  xEnd = (wCar >> 1);
  int    kp = 0;
  while (*src)
  { if (kp == cpos)  cpix = xEnd;
    xEnd += fnts->DrawChar(surface,xEnd,0,*src++,colText);
    kp++;
  }
  if  (kp == cpos)   cpix = xEnd;
  return;
}
//--------------------------------------------------------------------------------
//  Mouse Click inside the box. Get focus
//  Compute character position inside the text
//  TODO Set the caret
//--------------------------------------------------------------------------------
bool CFuiTextField::MouseClick(int mx, int my, EMouseButton button)
{ if (!MouseHit(mx,my))  return false;
  input = (HasProperty(FUI_EDIT_MODE))?(1):(0);
  cpix  = short(mx - xParent - x);
  cpos  = cpix / wCar;
  if (cpos >= nCar) cpos = nCar;
  NewCaret(cpos);
  EditText();
  return true;
}
//--------------------------------------------------------------------------------
//  Focus is lost
//  TODO remove caret
//--------------------------------------------------------------------------------
void CFuiTextField::FocusLost()
{ input = 0;
  EditText();
  return;
}
//-------------------------------------------------------------------------
//  Draw Caret
//-------------------------------------------------------------------------
void CFuiTextField::DrawCaret()
{ if (0 == input) return;
  int x0 = cpix;
  int x1 = x0   + wCar - 1;
  int y0 = hCar - 1;
  DrawFastLine(surface,x0,y0,x1,y0,caret);
  return;
}
//-------------------------------------------------------------------------
//  Set new caret position
//-------------------------------------------------------------------------
void CFuiTextField::NewCaret(short cp)
{ int x0 = cpix;
  int x1 = x0   + wCar - 1;
  int y0 = hCar - 1;
  DrawFastLine(surface,x0,y0,x1,y0,0xFFFFFFFF);
  return;
}
//-------------------------------------------------------------------------
//  Draw the text box 
//-------------------------------------------------------------------------
void CFuiTextField::Draw (void)
{ time++;
  if (time == 32) {time = 0; caret ^= mask; }
  DrawFBox(fBox,RSIZ);
  DrawCaret();
  CFuiComponent::Draw ();
}
//-------------------------------------------------------------------------
void CFuiTextField::UsePassword (int passwordFlag)
{
  this->pass = passwordFlag;
}

int CFuiTextField::IsPassword (void)
{
  return pass;
}

int CFuiTextField::GetLength (void)
{
  // Return actual number of used characters in data buffer
 //  return data.size();
  return nCar;
}

//----------------------------------------------------------------------------
void  CFuiTextField::SetMaxChars(int mc)
{
}



void CFuiTextField::SetSelection(int firstChar, int lastChar)
{
}

void CFuiTextField::GetSelection(int *firstChar, int *lastChar)
{
}

//===========================================================================
// CFuiLine
//===========================================================================
CFuiLine::CFuiLine (int x, int y, int w, int h, CFuiComponent *win)
: CFuiComponent (x, y, w, h, win)
{
  type = COMPONENT_LINE;
  widgetTag = 'defa';
  wName			= "Line";

  direction = FUI_LINE_HORIZONTAL;
  thickness = 1;
  SetProperty(FUI_NO_MOUSE);
}

int CFuiLine::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'dirc':
    ReadInt (&direction, stream);
    rc = TAG_READ;
    break;

  case 'thck':
    ReadInt (&thickness, stream);
    rc = TAG_READ;
    break;
  }

  // Send tag to parent class for processing
  if (rc == TAG_IGNORED) {
    rc = CFuiComponent::Read (stream, tag);
  }
  
  return rc;
}
//------------------------------------------------------------------------
//  All tags read
//------------------------------------------------------------------------
void CFuiLine::ReadFinished (void)
{ CFuiComponent::ReadFinished ();
  FindThemeWidget ();
  // Get theme components
  colLine = tw->GetColour ("LINE");
  if (surface) DrawFastLine (surface, x, y, x+w, y+h, colLine);
  return;
}

//=========================================================================
// CFuiBox
//=========================================================================
CFuiBox::CFuiBox (int x, int y, int w, int h, CFuiComponent *win)
: CFuiComponent (x, y, w, h, win)
{
  type = COMPONENT_BOX;
  widgetTag = 'defa';
  wName			= "Box";

  thickness = 0;
  colLine = MakeRGB (0, 0, 0);
  colFill = MakeRGB (0, 0, 0);
  SetProperty(FUI_NO_MOUSE);
}
//---------------------------------------------------------------------
//  Read all tags
//---------------------------------------------------------------------
int CFuiBox::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'thck':
    ReadInt (&thickness, stream);
    rc = TAG_READ;
    break;
  }

  // Send tag to parent class for processing
  if (rc == TAG_IGNORED) {
    rc = CFuiComponent::Read (stream, tag);
  }

  return rc;
}
//---------------------------------------------------------------------
//  All tags are read
//---------------------------------------------------------------------
void CFuiBox::ReadFinished (void)
{ FindThemeWidget ();
  Init(xParent,yParent);
  return;
}
//---------------------------------------------------------------------
//  Init the window
//----------------------------------------------------------------------
void CFuiBox::Init(int x, int y)
{ CFuiComponent::ReadFinished ();
  MoveParentTo(x,y);
	InitFBox(cBox,RSIZ);
  CreateFBox(this,cBox,x,y,w,h);
  // Get theme components
  colLine = tw->GetColour ("LINE");
  colFill = tw->GetColour ("FILL");
  return;
}
//---------------------------------------------------------------------
//  Change background color
//---------------------------------------------------------------------
void CFuiBox::BackColor(U_INT col)
{	CFuiComponent *cmp = cBox[BAKW];
	if (cmp) EraseSurfaceRGBA(cmp->GetSurface(),col);
	return;
}
//-----------------------------------------------------------------------
//  Change parent position in all components
//-----------------------------------------------------------------------
void CFuiBox::MoveParentTo (int xp, int yp)
{ CFuiComponent::MoveParentTo (xp, yp);
  // Update all decorations
  std::list<CFuiComponent*>::iterator i;
  for (i=decorationList.begin(); i!=decorationList.end(); i++) {
    (*i)->MoveParentTo (surface->xScreen, surface->yScreen);
}
  return;
}
//---------------------------------------------------------------------
//  Draw the window
//---------------------------------------------------------------------
void CFuiBox::Draw (void)
{ DrawFBoxBack(cBox[BAKW]);
  DrawFBox(cBox,RSIZ);
  CFuiComponent::Draw ();
}

//=========================================================================
// CFuiPicture
//=========================================================================
CFuiPicture::CFuiPicture (int x, int y, int w, int h, CFuiComponent *win)
: CFuiComponent (x, y, w, h, win)
{ type = COMPONENT_PICTURE;
  widgetTag = 'defa';
  wName			= "Picture";
	own	= 0;
  bm	= NULL;
  FindThemeWidget ();
  // Initialize frame index, ensuring that first call to Draw() will force
  //   the bitmap to be drawn on the component surface
  pFrame	= -1;
  cFrame	= 0;
  rStat		= WINDOW_RESIZE_NONE;
	rCurs		= 0;
	rType		= 0;
  MoWind	= win;
  SetProperty(FUI_NO_MOUSE);
}
//------------------------------------------------------------------------
CFuiPicture::~CFuiPicture (void)
{ if (own)	SAFE_DELETE(bm);	}
//-------------------------------------------------------------------------
//  Read parameters
//-------------------------------------------------------------------------
int CFuiPicture::Read (SStream *stream, Tag tag)
{ char fn[PATH_MAX];
  switch (tag) {
  case 'back':
		if (bm && own)	SAFE_DELETE(bm);
    ReadString (fn, 64, stream);
		own	= 1;
    bm	= new CBitmap(fn);
    SetBitmap (bm);
    return TAG_READ;
  }

  // Send tag to parent class for processing
  return CFuiComponent::Read (stream, tag);
}
//---------------------------------------------------------------------
//	All parameters are read
//---------------------------------------------------------------------
void CFuiPicture::ReadFinished (void)
{ if (bm == NULL) {
    // Get theme components
    bm = tw->GetBitmap ("BACK");
    if (bm == NULL) {
      bm = tw->GetBitmap ("ALTBACK");
    }
  }
  CFuiComponent::ReadFinished();
}
//----------------------------------------------------------------------
//  Set resize attributes
//----------------------------------------------------------------------
void	CFuiPicture::SetResize(Tag c,char t)
{	RazProperty(FUI_NO_MOUSE);
	rCurs	= c;
	rType	= t;
	return;	
}

//----------------------------------------------------------------------
//  Init the bitmap and surface.  Edit the bitmap
//----------------------------------------------------------------------
void CFuiPicture::SetBitmap (CBitmap *bm)
{ int wd = 0;
  int ht = 0;
  if (bm == NULL)   return;
  this->bm = bm;
  // If no width/height has been specified, use the bitmap width/height
  if ((this->w == 0) || (this->h == 0))  
  { bm->GetBitmapSize (&wd, &ht);
    w = short(wd);
    h = short(ht);
  }
  MakeSurface ();
  Edit();
}

//-------------------------------------------------------------------
//   Fill the given rectangle with color
//-------------------------------------------------------------------
void CFuiPicture::FillTheRect(short x,short y,short wd,short ht,U_INT col)
{ U_INT *des = surface->drawBuffer + (y * surface->xSize) + x;
  U_SHORT nl = ht;
  U_SHORT cl = 0;
  while (nl--)
  { while (cl++ != wd) *des++ = col;
    des += (surface->xSize - wd);
    cl  = 0;
  }
  return;
}
//-------------------------------------------------------------------
//   Edit the bitmap
//-------------------------------------------------------------------
void CFuiPicture::Edit()
{ 
  if ((0 == surface) || (0 == bm))  return;
  int bx, by;
  bm->GetBitmapSize (&bx, &by);
  for (int iy=0; iy<(int)surface->ySize; iy+=by)
    {	for (int ix=0; ix<(int)surface->xSize; ix+=bx)	 
					bm->DrawBitmap (surface, ix, iy, cFrame);
    }
  pFrame = cFrame;  
  return;
}

//---------------------------------------------------------------------
//  Change frame and reedit
//---------------------------------------------------------------------
void CFuiPicture::SetFrame (int i)
{ if (cFrame == i)  return;
  cFrame  = i;
  Edit();
  return;
}
//--------------------------------------------------------------------
//  Moving state
//--------------------------------------------------------------------
bool CFuiPicture::Moving(int xs, int ys)
{ short dx = xs - mx;
  short dy = ys - my;
  mx = xs;
  my = ys;

  if (rStat == WINDOW_RESIZE_XDIM)  dy = 0;
  if (rStat == WINDOW_RESIZE_YDIM)  dx = 0;
  MoWind->ResizeTo(dx,dy);
  return true;
}
//--------------------------------------------------------------------
//  Check for move over the bord represented by this picture
//--------------------------------------------------------------------
bool CFuiPicture::MoveOver(int mx,int my)
{ if (rStat)            return Moving(mx,my);
  if (!MouseHit(mx,my)) return false;
	if (0 == rCurs)				return false; 
  globals->cum->SetCursor(rCurs);
  return true;
}
//--------------------------------------------------------------------
//  Check for click in
//--------------------------------------------------------------------
void CFuiPicture::ClickOver(int mx, int my)
{ rStat			= rType;
  this->mx  = mx;
  this->my  = my;
  return;
}
//--------------------------------------------------------------------
//  Stopt click. Stop rezise
//--------------------------------------------------------------------
bool CFuiPicture::MouseStopClick (int mx, int my, EMouseButton button)
{ rStat = WINDOW_RESIZE_NONE;
  return true;
}
//--------------------------------------------------------------------
//  Change dimension by delta x and delta y.
//--------------------------------------------------------------------
void CFuiPicture::Redim(short dx, short dy)
{ w += dx;
  h += dy;
  MakeSurface();
  pFrame = -1;
  Edit();
  return;
}
//--------------------------------------------------------------------
//  Reformat the window
//--------------------------------------------------------------------
void CFuiPicture::Format(WZ_SIZE code, short dx, short dy)
{ switch (code) {
  case WZ_NONE:
    return;
  case WZ_MX00:
    MoveBy(dx,0);
    return;
  case WZ_SX00:
    if (dx) Redim(dx,0);
    return;
  case WZ_MY00:
    MoveBy(0,dy);
    return;
  case WZ_SY00:
    if (dy) Redim(0,dy);
    return;
  case WZ_MXSY:
    MoveBy(dx,0);
    if (dy) Redim(0,dy);
    return;
  case WZ_SXMY:
    MoveBy(0,dy);
    if (dx) Redim(dx,0);
    return;
  case WZ_MXMY:
    MoveBy(dx,dy);
    return;
  }
return;
}
//========================================================================================
// CFuiScrollBTN
//========================================================================================
CFuiScrollBTN::CFuiScrollBTN(int x, int y, int w, int h, CFuiComponent *win,char *btn)
: CFuiComponent(x,y,w,h,win)
{ type  = COMPONENT_SCROLLBTN;
  wName	= "Scrollbtn";
  FindThemeWidget();
  SetBox(&sBOX,btn);
  //--------------------------------------------------
  rate  = 10;
  pres  = 0;
}
//-----------------------------------------------------------------------
//  Read Parameters
//-----------------------------------------------------------------------
int CFuiScrollBTN::Read (SStream *stream, Tag tag)
{ int   nb;
  float ft;
  switch (tag) {
    case 'rrpt':
      ReadInt (&nb, stream);
      if (nb) SetProperty(FUI_REPEAT_BT);
      return TAG_READ;
    case 'rate':
      ReadFloat (&ft, stream);
      rate  = (U_CHAR)(int(ft * 25));
      return TAG_READ;
  }
  return CFuiComponent::Read(stream,tag);
}
//-----------------------------------------------------------------------
//  Set repeat parameters
//-----------------------------------------------------------------------
void CFuiScrollBTN::SetRepeat(float r)
{ rate  = (U_CHAR)(int(r * 25));
	SetProperty(FUI_REPEAT_BT);
  return;
}
//-----------------------------------------------------------------------
//  Read finished, init parameters
//-----------------------------------------------------------------------
void CFuiScrollBTN::ReadFinished()
{ 
  //-----------Font and surface ----------------------------------
  CFuiComponent::ReadFinished ();
  return;
}
//-------------------------------------------------------------------------
//  Repeat notification
//-------------------------------------------------------------------------
void CFuiScrollBTN::Repeat()
{ if (!HasProperty(FUI_REPEAT_BT))	return;
	rrpt += 1;
  if (rrpt < rate)  return;
  MoWind->NotifyChildEvent(id,id,EVENT_BUTTONPRESSED);
  rrpt = 0;
  return;
}
//--------------------------------------------------------------------------------
//  Button is clicked.  Notify Mother Windows
//--------------------------------------------------------------------------------
bool CFuiScrollBTN::MouseClick (int x, int y, EMouseButton button)
{ // Mouse clicked inside button area, set state to pressed.  
  if (!MouseHit(x,y))   return false;
  rrpt  = 0;
  pres  = 1;
  sBOX.frm = 1;
  MoWind->NotifyChildEvent(id,id,EVENT_BUTTONPRESSED);
  return true;
}
//-------------------------------------------------------------------------------
//  Mouse stop click.  Return to frame 0
//-------------------------------------------------------------------------------
bool CFuiScrollBTN::MouseStopClick (int x, int y, EMouseButton button)
{ sBOX.frm  = 0;
  pres      = 0;
  MoWind->NotifyChildEvent(id,id,EVENT_BUTTONRELEASE);
  return true;
}
//-----------------------------------------------------------------------
//  Draw the button
//-----------------------------------------------------------------------
void CFuiScrollBTN::Draw()
{ sBOX.bmap->DrawBitmap(surface,sBOX.rx,sBOX.ry,sBOX.frm);
  CFuiComponent::Draw();
  if (pres) Repeat();
  return;
}

//========================================================================================
// CFuiScrollBar
//========================================================================================
CFuiScrollBar::CFuiScrollBar(int x, int y, int w, int h, CFuiComponent *win, bool bVertical)
: CFuiComponent (x, y, w, h, win)
{ type = COMPONENT_SCROLLBAR;
  id = 0; // unused
  wName			= "Scrollbar";
  bFocus    = 0;
  m_bVertical = bVertical;
  //------------------------------
  ReadFinished();
}
//----------------------------------------------------------------------
CFuiScrollBar::~CFuiScrollBar()
{
}
//---------------------------------------------------------------------------
//  Compute vertical positions for item
//---------------------------------------------------------------------------
void CFuiScrollBar::SetVTposition()
{   SetBox(&UpBOX,"TOP");
    x     -= UpBOX.wd;
    w      = UpBOX.wd;
    //-------Init lower arrow -------------------
    SetBox(&DoBOX,"BOTTOM");
    DoBOX.ry  = h - DoBOX.ht; 
    //------ Init Middle ------------------------
    SetBox(&VmBOX,"ALTBACK");
    VmBOX.ry  = UpBOX.ht;
    //------ Init Thumb -------------------------
    SetBox(&VtBOX,"VTHUMB");
    VtBOX.ry  = UpBOX.ht;
    VtBOX.rx  = (VmBOX.wd - VtBOX.wd) >> 1;
    //-------Set Vertical thumb limits ----------
    VtTop     = UpBOX.ht;
    VtAmp     = DoBOX.ry - VtTop - VtBOX.ht;
    VtLow     = VtTop + VtAmp;
    VtBOX.ry  = UpBOX.ht;
    return;
}
//---------------------------------------------------------------------------
//  Vertical Resize 
//---------------------------------------------------------------------------
void CFuiScrollBar::ResizeVT(short dy)
{ h += dy;                         // New height
  DoBOX.ry  = h - DoBOX.ht;
  VtAmp     = DoBOX.ry - VtTop - VtBOX.ht;
  VtLow     = VtTop + VtAmp;
  MakeSurface();
  EditVSB();
  return;
}
//---------------------------------------------------------------------------
//  Compute Horizontal positions for item
//---------------------------------------------------------------------------
void CFuiScrollBar::SetHZposition()
{ //------Init left Arrow ---------------------
    SetBox(&LfBOX,"LEFT");
    y     -= LfBOX.ht;
    h      = LfBOX.ht;
    //----Init right arrow -----------------------
    SetBox(&RtBOX,"RIGHT");
    RtBOX.rx  = w - (RtBOX.wd << 1);
    //----Init Midle -----------------------------
    SetBox(&HmBOX,"BACK");
    HmBOX.rx  = LfBOX.wd;
    //----Init  Thumb ----------------------------
    SetBox(&HtBOX,"HTHUMB");
    HtBOX.rx  = LfBOX.wd;
    HtBOX.ry  = (HmBOX.ht - HtBOX.ht) >> 1;
    //----Set thumb limits -----------------------
    HzLef     = LfBOX.wd;
    HzAmp     = RtBOX.rx - HzLef - HtBOX.wd;
    HzRit     = HzLef + HzAmp;
    HtBOX.rx  = HzLef;
    return;
}
//---------------------------------------------------------------------------
//  All tags are read
//---------------------------------------------------------------------------
void CFuiScrollBar::ReadFinished(void)
{
  FindThemeWidget ();
  if(m_bVertical) SetVTposition();
  else            SetHZposition();
  //-----------Font and surface ----------------------------------
  CFuiComponent::ReadFinished ();
  if (m_bVertical) EditVSB();
}

//---------------------------------------------------------------------
//  Fill surface with bitmap
//---------------------------------------------------------------------
void CFuiScrollBar::FillBox (SCBOX *box)
{  // Frame number has changed, redraw bitmap tiled across surface
    int bx, by;
    CBitmap *bm = box->bmap;
    bm->GetBitmapSize (&bx, &by);
    for (int iy=0; iy<(int)surface->ySize; iy+=by)
    {
      for (int ix=0; ix<(int)surface->xSize; ix+=bx) bm->DrawBitmap (surface, ix, iy, 0);
    }
    return;
}

//------------------------------------------------------------------------------
//  MouseStop
//------------------------------------------------------------------------------
bool CFuiScrollBar::MouseStopClick (int mx, int my, EMouseButton button)
{ if (bFocus) {bFocus->frm = 0; EditButton(bFocus);}
  bFocus  = 0;
  return true;
}
//------------------------------------------------------------------------------
//    Vertical event. Notify the parent with this event
//------------------------------------------------------------------------------
bool CFuiScrollBar::SendVTNotification(EFuiEvents evn, SCBOX *box)
{ MoWind->NotifyChildEvent(id,'sbar',evn);
  bFocus    = box;
  if (0 == box)   return true;  
  box->frm = 1;
  EditButton(box);
  return true;  }
//------------------------------------------------------------------------------
//    Horizontal event Notify the parent with this event
//------------------------------------------------------------------------------
bool CFuiScrollBar::SendHZNotification(EFuiEvents evn, SCBOX *box)
{ MoWind->NotifyChildEvent(id,'sbar',evn);
  bFocus    = box;
  if (0 == box)   return true;  
  box->frm = 1;
  EditButton(box);
  return true;  }

//------------------------------------------------------------------------------
//  Mouse Click. Send event to parent window anyway
//------------------------------------------------------------------------------
bool CFuiScrollBar::VtMouseClick (int mx, int my, EMouseButton button)
{ if (!MouseHit(mx,my))           return false;
  int ry = my - yParent - y;

  if (ry < UpBOX.ht)              return SendVTNotification(EVENT_PREVLINE,&UpBOX);
  if (ry > DoBOX.ry)              return SendVTNotification(EVENT_NEXTLINE,&DoBOX);
  if (ry < VtBOX.ry)              return SendVTNotification(EVENT_PREVPAGE);
  if (ry > (VtBOX.ry + VtBOX.ht)) return SendVTNotification(EVENT_NEXTPAGE);
  //----Must track thumb with mouse ------------------------------------
  bFocus  = &VtBOX;                
  return true;
}
//------------------------------------------------------------------------------
//  Mouse Click. Send event to parent window anyway
//------------------------------------------------------------------------------
bool CFuiScrollBar::HzMouseClick (int mx, int my, EMouseButton button)
{ if (!MouseHit(mx,my))           return false;
  int rx = mx - xParent - x;
  if (rx < LfBOX.wd)              return SendHZNotification(EVENT_PREVCOLN,&LfBOX);
  if (rx > RtBOX.rx)              return SendHZNotification(EVENT_NEXTCOLN,&RtBOX);
  if (rx < HtBOX.rx)              return SendHZNotification(EVENT_PREVPCOL);
  if (rx > (HtBOX.rx + HtBOX.wd)) return SendHZNotification(EVENT_NEXTPCOL);
  //----Must track thumb with mouse ------------------------------------
  bFocus  = &HtBOX;
  return true;
}
//---------------------------------------------------------------------------
//  Edit button
//---------------------------------------------------------------------------
void CFuiScrollBar::EditButton(SCBOX *box)
{ box->bmap->DrawBitmap(surface, box->rx, box->ry, box->frm);
  return;
}
//---------------------------------------------------------------------------
//  Edit horizontal scroll bar
//---------------------------------------------------------------------------
void CFuiScrollBar::EditHSB()
{ EraseSurfaceRGBA(surface,0);
  FillBox(&HmBOX);
  LfBOX.bmap->DrawBitmap(surface, LfBOX.rx, LfBOX.ry, LfBOX.frm);
  RtBOX.bmap->DrawBitmap(surface, RtBOX.rx, RtBOX.ry, RtBOX.frm);
  HtBOX.bmap->DrawBitmap(surface, HtBOX.rx, HtBOX.ry, 0);
  return;
}
//---------------------------------------------------------------------------
//  Draw horizontal scroll bar
//---------------------------------------------------------------------------
void CFuiScrollBar::DrawHSB()
{ CFuiComponent::Draw();
  return;
}
//---------------------------------------------------------------------------
//  Edit vertical scroll bar
//---------------------------------------------------------------------------
void CFuiScrollBar::EditVSB()
{ EraseSurfaceRGBA(surface,0);
  FillBox(&VmBOX);
  UpBOX.bmap->DrawBitmap(surface, UpBOX.rx, UpBOX.ry, UpBOX.frm);
  DoBOX.bmap->DrawBitmap(surface, DoBOX.rx, DoBOX.ry, DoBOX.frm);
  VtBOX.bmap->DrawBitmap(surface, VtBOX.rx, VtBOX.ry, 0);
  return;
}
//---------------------------------------------------------------------------
//  Draw vertical scroll bar
//---------------------------------------------------------------------------
void CFuiScrollBar::DrawVSB()
{ CFuiComponent::Draw();
  return;
}
//-------------------------------------------------------------------------------
//  Drag Vertical thumb
//-------------------------------------------------------------------------------
bool CFuiScrollBar::DragVThumb(int mx, int my)
{ int dy  = my - yParent - y;
  if (dy < VtTop)  dy  = VtTop;
  if (dy > VtLow)  dy  = VtLow;
  int dis  = dy - VtTop;
  VtBOX.ry = dy;
  //-------Compute % placement as integer value [0,100] ------------
  int pc = (dis * 100) / VtAmp;
  MoWind->NotifyChildEvent(id,(Tag)pc,EVENT_THBRATIO);
  EditVSB();
  return true;
}
//-------------------------------------------------------------------------------
//  Drag Horizontal thumb
//-------------------------------------------------------------------------------
bool CFuiScrollBar::DragHThumb(int mx, int my)
{ int dx  = mx - xParent;
  if (dx < HzLef)  dx  = HzLef;
  if (dx > HzRit)  dx  = HzRit;
  int dis  = dx - HzLef;
  HtBOX.rx = dx;
  //-------Compute % placement as integer value [0,100] ------------
  int pc = (dis * 100) / HzAmp;
  MoWind->NotifyChildEvent(id,(Tag)pc,EVENT_THBRATIO);
  EditHSB();
  return true;
}

//-------------------------------------------------------------------------------
//  Set Vertical ratio
//-------------------------------------------------------------------------------
void CFuiScrollBar::SetVSRatio(float rat)
{ VtBOX.ry  = VtTop + int(VtAmp * rat);
  EditVSB();
  return;
}
//-------------------------------------------------------------------------------
//  Set Horizontal ratio
//-------------------------------------------------------------------------------
void CFuiScrollBar::SetHSRatio(float rat)
{ HtBOX.rx  = HzLef + int(HzAmp *rat);
  EditHSB();
  return;
}
//-------------------------------------------------------------------------------
//  Mouse Mouve.  Check for draging thumb
//-------------------------------------------------------------------------------
bool CFuiScrollBar::MouseMove (int x, int y)
{ if (!MouseHit(x,y))     return false;
  if (0      == bFocus)   return true;
  if (&VtBOX == bFocus)   return DragVThumb(x,y);
  if (&HtBOX == bFocus)   return DragHThumb(x,y);
  return true;
}
//================================================================================
// CFuiSlider
//================================================================================
CFuiSlider::CFuiSlider (int x, int y, int w, int h, CFuiComponent *win)
: CFuiComponent (x, y, w, h, win)
{
  type = COMPONENT_SLIDER;
  widgetTag = 'defa';
  wName		= "Slider";
	bCol		= MakeRGBA(129,162,184,255);
  bmBack	= NULL;
  bmThumb = NULL;
  bmLTick = NULL;
  bmSTick = NULL;

  minv  = 0.0f;
  maxv  = 100.0f;
  curv  = 0.0f;
  majt  = 10.0f;
  dmjt  = true;
  mint  =  5.0f;
  link  = 0;
  curPixPos = 0;
  mstat = 0;
  vert  = (h > w)?(true):(false);
}
//------------------------------------------------------------------------------
//  Read all parameters
//------------------------------------------------------------------------------
int CFuiSlider::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'maxv':
    ReadFloat (&maxv, stream);
    return TAG_READ;

  case 'minv':
    ReadFloat (&minv, stream);
    return TAG_READ;

  case 'curv':
    ReadFloat (&curv, stream);
    return TAG_READ;

  case 'majt':
    ReadFloat (&majt, stream);
    return TAG_READ;

  case 'dmjt':
    {
      int i;
      ReadInt (&i, stream);
      dmjt = (i != 0);
    }
    return TAG_READ;

  case 'mint':
    ReadFloat (&mint, stream);
    return TAG_READ;

  case 'dmnt':
    {
      int i;
      ReadInt (&i, stream);
    }
    return TAG_READ;
  case 'snap':
    {
      int i;
      ReadInt (&i, stream);
    }
    return TAG_READ;
  case 'link':
    ReadTag (&link, stream);
    return TAG_READ;
  }
  
  // Send tag to parent class for processing.
  if (rc == TAG_IGNORED) {
    rc = CFuiComponent::Read (stream, tag);
  }

  return rc;
}
//------------------------------------------------------------------------------
//  All parameters read
//------------------------------------------------------------------------------

void CFuiSlider::ReadFinished (void)
{  FindThemeWidget ();

  // Get backdrop bitmap
  bmBack = tw->GetBitmap ("BACK");
  if (bmBack) {
    int bmw, bmh;
    bmBack->GetBitmapSize (&bmw, &bmh);
    hrul = short(bmh);
    wrul = short(bmw);
  }

  // Get  thumb bitmap
  bmThumb = (vert)?(tw->GetBitmap ("VTHUMB")):(tw->GetBitmap ("HTHUMB"));

  // Get  major tick bitmap
  bmLTick = (vert)?(tw->GetBitmap ("VLTICK")):(tw->GetBitmap ("HLTICK"));

  // Get  minor tick bitmap
  bmSTick = (vert)?(tw->GetBitmap ("VSTICK")):(tw->GetBitmap ("HSTICK"));

  // Get thumb bitmap size in order to offset any tick marks
  bmwI = 0;
  bmhI = 0;
  if (bmThumb) {
    bmThumb->GetBitmapSize (&bmwI, &bmhI);
    bmI2  = (vert)?(short(bmhI>>1)):(short(bmwI>>1));
    curPixPos = bmI2;
  }
  font = FuiFont (fontTag);
  //----Increase height to include thumb ----------------
  MakeSurface ();
  EditThick();
  SetValue(curv);
}
//-------------------------------------------------------------------------
//  Edit the horizontal thick marks
//-------------------------------------------------------------------------
void CFuiSlider::EditThick()
{ // Draw minor ticks
  if (bmSTick && (mint > 0))
  { for (float f=minv; f<=maxv; f+=mint) 
    { int x0 = (vert)?(bmwI):(int((float)(w-1) * ((f - minv) / (maxv - minv))));
      int y0 = (vert)?(int((float)(h-1) * ((f - minv) / (maxv - minv)))):(bmhI);
      bmSTick->DrawBitmap (surface, x0, y0, 0);
    }
  }
  if (0 == dmjt)  return;
  // Draw major ticks
  if (bmLTick && (majt > 0))
  { for (float f=minv; f<=maxv; f+=majt)
    { int x0 = (vert)?(bmwI):(int((float)(w-1) * ((f - minv) / (maxv - minv))));
      int y0 = (vert)?(int((float)(h-1) * ((f - minv) / (maxv - minv)))):(bmhI);
      bmLTick->DrawBitmap (surface, x0, bmhI, 0);
    }
  }
  return;
}
//-------------------------------------------------------------------------
//  Edit horizontal cursor value
//-------------------------------------------------------------------------
void  CFuiSlider::EditThumb()
{ FillRect(surface,0,0,w,bmhI,bCol);
//  bmBack->FillTheRect(surface,0,0,w,hrul,0);
//	bmBack->DrawBitmap(surface,0,0,0);
  if (0 == bmThumb)  return;
  int px    = (vert)?(0):(curPixPos - bmI2);
  int py    = (vert)?(curPixPos - bmI2):(0);
  bmThumb->DrawBitmap (surface, px, py, 0);
  return;
}
//-------------------------------------------------------------------------
//  Force value and reedit cursor
//-------------------------------------------------------------------------
void CFuiSlider::SetValue(float fValue)
{ curv = fValue;
  if (0 == surface)  return;
  int cx    = int(float(w) * (curv - minv) / (maxv - minv));
  int cy    = int(float(h) * (curv - minv) / (maxv - minv));
  curPixPos = (vert)?(cy):(cx);
  EditThumb();
  return;
}
//-------------------------------------------------------------------------
//  Change range value
//-------------------------------------------------------------------------
void CFuiSlider::SetRange(float a,float b)
{ minv = a;
  maxv = b;
  EraseSurfaceRGBA(surface,0);
  EditThick();
  SetValue(curv);
}
//-------------------------------------------------------------------------
//  Check for horizontal click
//-------------------------------------------------------------------------
bool CFuiSlider::InThumb(int ms,int sc)
{ int rm = ms - sc;
  if (rm < (curPixPos-bmI2)) return false;
  if (rm > (curPixPos+bmI2)) return false;
  mstat = 1;
  prevM = ms;
  return true;
}

//-------------------------------------------------------------------------
//  Mouse click in thumb
//-------------------------------------------------------------------------
bool CFuiSlider::MouseClick(int mx,int my,EMouseButton btn)
{ if (!MouseHit(mx,my))           return false;
  if ( HasProperty(FUI_NO_MOUSE)) return true;
  return (vert)?(InThumb(my,surface->yScreen)):(InThumb(mx,surface->xScreen));
}
//-------------------------------------------------------------------------
//  Moving the thumb
//  Compute the ratio
//-------------------------------------------------------------------------
bool CFuiSlider::Moving(int nm,int dim)
{ int   dm      = nm - prevM;
  int   nc      = curPixPos + dm;
  prevM         = nm;
  //----Clamp to cursor limits ----------------------
  if (nc < 0)    nc = 0;
  if (nc > dim)  nc = dim;
  curPixPos   = nc;
  //----Compute percentage ratio --------------------
  curv  = (float(nc) * (maxv - minv) / float(dim)) + minv;
  EditThumb();
  MoWind->NotifyChildEvent(id,0,EVENT_THBRATIO);
  return true;
}
//-------------------------------------------------------------------------
//  Mouse Move the thumb
//-------------------------------------------------------------------------
bool CFuiSlider::MouseMove(int mx,int my)
{ if (mstat) return (vert)?(Moving(my,h)):(Moving(mx,w));
  return MouseHit(mx,my);
}
//-------------------------------------------------------------------------
//  Mouse Stp click. Stop Moving
//-------------------------------------------------------------------------
bool CFuiSlider::MouseStopClick(int mx,int my,EMouseButton btn)
{ mstat = 0;
  return true;
}
//-------------------------------------------------------------------------
//  Focus lost. Stop moving
//-------------------------------------------------------------------------
void CFuiSlider::FocusLost()
{ mstat = 0;
  return;
}

//====================================================================
// JSDEV* Modified CFuiGroupBox
//====================================================================
CFuiGroupBox::CFuiGroupBox (int x, int y, int w, int h, CFuiComponent *win)
: CFuiComponent (x, y, w, h, win)
{ type = COMPONENT_GROUPBOX;
  widgetTag = 'defa';
  wName			= "Groupbox";
  InitFBox(fBox,RSIZ);
  SetProperty(FUI_TRANSPARENT);
  colText = MakeRGB (0, 0, 0);
  xText   = 0;
}
//----------------------------------------------------------------------
//  Decoration is destroyed by CFuiComponent level
//----------------------------------------------------------------------
CFuiGroupBox::~CFuiGroupBox (void)
{
  std::list<CFuiComponent*>::iterator i;
  for (i=childList.begin(); i!=childList.end(); i++) {
    SAFE_DELETE(*i);
  }
}
//-------------------------------------------------------------------------
//  Read All Tags
//-------------------------------------------------------------------------
int CFuiGroupBox::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  // Add subcomponent if applicable
  if (ValidFuiComponentType (tag)) {
    CFuiComponent *c = CreateFuiComponent ((EFuiComponentTypes)tag,this);
    if (c != NULL) {
      ReadFrom (c, stream);
      c->MoveParentTo (x, y);
      childList.push_back (c);
      rc = TAG_READ;
    } else {
      char s[8];
      WARNINGLOG ("CFuiGroupBox : Skipping unsupported widget %s", TagString (s, tag));
      SkipObject (stream);
    }
    rc = TAG_READ;
  }

  // Send tag to parent class for processing.
  if (rc == TAG_IGNORED) {
    rc = CFuiComponent::Read (stream, tag);
  }

  return rc;
}
//-----------------------------------------------------------------------------
//  Build the bitmap for this Box
//-----------------------------------------------------------------------------
void CFuiGroupBox::ReadFinished (void)
{ CFuiComponent::ReadFinished ();
  FindThemeWidget ();
  CreateFBox(this,fBox,x,y,w,h);
  // Get text colour
  colText = tw->GetColour ("TEXT");
  fnts->DrawNText(surface,4,0,colText,text);
}
//--------------------------------------------------------------------------
//  Change text
//--------------------------------------------------------------------------
void CFuiGroupBox::SetText(char *t)
{	strncpy(text,t,128);
	fnts->DrawNText(surface,4,0,colText,text);
	return;
}
//--------------------------------------------------------------------------
//  Move the box to the indicated absolute position
//--------------------------------------------------------------------------
void CFuiGroupBox::MoveTo(int xs,int ys)
{ CFuiComponent::MoveTo(xs,ys);
  // Update parent position of window decorations
  std::list<CFuiComponent*>::iterator i;
  for (i=decorationList.begin(); i!=decorationList.end(); i++) {
    (*i)->MoveParentTo(surface->xScreen, surface->yScreen);
  }

  // Update parent position of child widgets
  std::list<CFuiComponent*>::iterator ide;
  for (ide=childList.begin(); ide!=childList.end(); ide++) {
    (*ide)->MoveParentTo(surface->xScreen, surface->yScreen);
  }
  return;
}
//--------------------------------------------------------------------------
//  Move the box to the indicated relative position
//--------------------------------------------------------------------------
void CFuiGroupBox::MoveBy(int dx,int dy)
{ CFuiComponent::MoveTo(dx + x,dy + y);
  int nx = surface->xScreen;
  int ny = surface->yScreen;
  // Update parent position of window decorations
  std::list<CFuiComponent*>::iterator i;
  for (i=decorationList.begin(); i!=decorationList.end(); i++) {
    (*i)->MoveParentTo(nx, ny);
  }

  // Update parent position of child widgets
  std::list<CFuiComponent*>::iterator ide;
  for (ide=childList.begin(); ide!=childList.end(); ide++) {
    (*ide)->MoveParentTo(nx, ny);
  }
  return;
}
//--------------------------------------------------------------------------
//  Change parent position for all component of this box
//--------------------------------------------------------------------------
void CFuiGroupBox::MoveParentTo (int xp, int yp)
{
  CFuiComponent::MoveParentTo (xp, yp);

  // Update all decorations
  std::list<CFuiComponent*>::iterator i;
  for (i=decorationList.begin(); i!=decorationList.end(); i++) {
    (*i)->MoveParentTo (surface->xScreen, surface->yScreen);
  }

  // Update all children
  for (i=childList.begin(); i!=childList.end(); i++) {
    (*i)->MoveParentTo(surface->xScreen, surface->yScreen);
  }
  return;
}
//----------------------------------------------------------------------
//  Helper to add a component
//----------------------------------------------------------------------
void CFuiGroupBox::AddChild(Tag idn,CFuiComponent *cmp,char *txt, U_INT c)
{ cmp->SetId(idn);
  cmp->ReadFinished();
	if (c) cmp->SetColour(c);
  cmp->SetText(txt);
  childList.push_back(cmp);
  return;
}
//---------------------------------------------------------------------------
//  Draw the GroupBox
//---------------------------------------------------------------------------
void CFuiGroupBox::Draw (void)
{ std::list<CFuiComponent*>::iterator i;
  if ((!HasProperty(FUI_IS_VISIBLE)) || (!surface))  return;
  // Draw groupbox decoration components
  DrawFBoxBack(fBox[BAKW]);
  DrawFBox(fBox,RSIZ);
  // Draw child components
  for (i=childList.begin(); i!=childList.end(); i++) {
    (*i)->Draw();
  }
  CFuiComponent::Draw ();
}
//----------------------------------------------------------------------------
//  Mouse Mouve Send to components
//----------------------------------------------------------------------------
bool CFuiGroupBox::MouseMove (int mx, int my)
{ if (!MouseHit(mx,my))               return false;
  if (!HasProperty(FUI_IS_VISIBLE))   return true;
  // Send directly to child widgets
  std::list<CFuiComponent*>::iterator i;
  for (i=childList.begin(); i!=childList.end(); i++) if ((*i)->MouseMove (mx, my)) return true;
  return true;
}
//-----------------------------------------------------------------------------
//  A child send notification.  Send it to mother windows
//-----------------------------------------------------------------------------
void CFuiGroupBox::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ MoWind->NotifyChildEvent(idm,itm,evn);
  return;
}
//-----------------------------------------------------------------------------
//  A child popup send notification.  Send it to mother windows
//-----------------------------------------------------------------------------
void CFuiGroupBox::NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn)
{ MoWind->NotifyFromPopup(id,itm,evn);
  return;
}
//-----------------------------------------------------------------------------
//  Send Mouse click to childreen if visible
//  Register component with focus
//-----------------------------------------------------------------------------
bool CFuiGroupBox::MouseClick (int mx, int my, EMouseButton button)
{ if (!MouseHit(mx,my))             return false;
  if (!HasProperty(FUI_IS_VISIBLE)) return true;
  // Send directly to child widgets
  std::list<CFuiComponent*>::iterator i;
  for (i=childList.begin(); i!=childList.end(); i++) {
    CFuiComponent *cp = (*i);
    if (!cp->MouseHit(mx,my))     continue;
    RegisterFocus(cp);
    cp->MouseClick (mx, my, button);
    return true;
  }
  return true;
}
//-----------------------------------------------------------------------------
//   Send notifcation to the focused component if any
//-----------------------------------------------------------------------------
bool CFuiGroupBox::MouseStopClick (int mx, int my, EMouseButton button)
{ if (cFocus) cFocus->MouseStopClick(mx,my,button);
  return true;
}
//-----------------------------------------------------------------------------
//  Focus is lost
//  Notifify the registered component
//-----------------------------------------------------------------------------
void CFuiGroupBox::FocusLost(void)
{ ClearFocus(this);
  return;
}
//-----------------------------------------------------------------------------
//  Set text in component
//-----------------------------------------------------------------------------
void CFuiGroupBox::SetChildText(Tag idn, char *txt)
{ CFuiComponent *lab = GetComponent(idn);
  if (0 == lab)   return;
  lab->SetText(txt);
  return;
}
//-----------------------------------------------------------------------------
//  Set Edit mode
//-----------------------------------------------------------------------------
void CFuiGroupBox::SetEditMode(Tag idn,U_CHAR md)
{ CFuiComponent *cmp = GetComponent(idn);
  if (0 == cmp)   return;
  cmp->SetEditMode(md);
  return;
}
//-----------------------------------------------------------------------------
//  Return the requested component
//-----------------------------------------------------------------------------
CFuiComponent *CFuiGroupBox::GetComponent (Tag component)
{ std::list<CFuiComponent*>::iterator i;
  for (i=childList.begin(); i!=childList.end(); i++) {
    CFuiComponent *cmp = (*i);
    if (cmp->GetId() == component) return (*i);
  }
  return 0;
}
//-----------------------------------------------------------------------------
//  Keyboard input
//-----------------------------------------------------------------------------
bool CFuiGroupBox::KeyboardInput(U_INT key)
{
  // If Keyboard has not been handled yet, send to child widgets
  std::list<CFuiComponent*>::iterator i;
  for (i=childList.begin(); i!=childList.end(); i++)
  { CFuiComponent *cp = (*i);
    
    if (cp->KeyboardInput (key)) return true;
  }

  return false;
}
//===============================================================================
// CFuiList
//===============================================================================
CFuiList::CFuiList (int x, int y, int w, int h, CFuiComponent *win)
: CFuiComponent (x, y, w, h, win)
{
  type = COMPONENT_LIST;
  widgetTag = 'defa';
  wName= "List";
  usrr = 0;
  msel = 0;
  vscr = 0;
  hscr = 0;
  input       = 0;
  autowidth   = 0;
  rowSelected = 0;
	cMark = 0;
  vsBOX = NULL;
  hzBOX = NULL;
  //-- init decorations ---------------
  InitFBox(fBox,RSIZ);
  //-----------------------------------
  colText     = MakeRGB (0,    0, 0);
  cTxtHLight  = MakeRGB (0,    0, 255);
  cBakHLight  = MakeRGB (200,  0, 200);
  cBackTitle  = MakeRGB (105,243, 232);
  hCar    = 0;
  hLine   = 0;
  wCar    = 0;
  NbCol   = 0;
}
//-------------------------------------------------------------------------
//  Destroy the window list 
//-------------------------------------------------------------------------
CFuiList::~CFuiList()
{ SAFE_DELETE(cMark);
  SAFE_DELETE(hzBOX);
  SAFE_DELETE(vsBOX);
}

//-------------------------------------------------------------------------
//  Read all tags
//-------------------------------------------------------------------------
int CFuiList::Read (SStream *stream, Tag tag)
{ int pm;
  int nb;
  switch (tag) {
  case 'utit':
    ReadInt (&nb, stream);
    return TAG_READ;
  case 'usrr':
    ReadInt (&pm, stream);
    usrr = pm;
    return TAG_READ;
  case 'msel':
    ReadInt (&pm, stream);
    msel = pm;
    return TAG_READ;
  case 'vscr':
    ReadInt (&pm, stream);
    vscr = pm;
    return TAG_READ;
  case 'hscr':
    ReadInt (&pm, stream);
    hscr = pm;
    return TAG_READ;
  case 'dwid':
    ReadInt (&nb, stream);
    return TAG_READ;
  case 'dhgt':
    ReadInt (&nb, stream);
    return TAG_READ;
  case 'autw':
    ReadInt (&pm, stream);
    autowidth = pm;
    return TAG_READ;
  }

  // Send tag to parent class for processing
  return CFuiComponent::Read (stream, tag);
}
//-------------------------------------------------------------------------
//  All parameters are read.  Create the real window now
//-------------------------------------------------------------------------
void CFuiList::ReadFinished (void)
{
  CFuiComponent::ReadFinished ();

  FindThemeWidget ();
  //-------- Compute definitive drawing surface ----------------
  halfW = (w >> 1);          // Half wide
  halfH = (h >> 1);          // Half height
  CreateFBox(this,fBox,x,y,w,h);
  //--------- Get text colour ----------------------------------
  colText     = tw->GetColour ("TEXT");
  cTxtHLight  = tw->GetColour ("TEXTHILITE");
  cBakHLight  = tw->GetColour ("HILITE");

  // Create Scrollbar object if required
  if(hscr) hzBOX = new CFuiScrollBar(0, h, w, h, this, false);
  if(vscr) vsBOX = new CFuiScrollBar(w, 0, w, h, this, true);
  //------------------------------------------------------------
  int wt  = fnts->TextWidth ("W");
  hCar    = fnts->TextHeight("H");
  hLine   = hCar;
  NbCol   = (w) / wt;
  wCar    = wt;
  //--------------------------------------------
  cMark = new SBitmap;
  strncpy (cMark->bitmapName,"ART/CHECKMARK.BMP",63);
  Load_Bitmap (cMark);
  NbLin   = h / hLine;
  //--------------------------------------------
	//ChangeFont(&globals->fonts.ftradi9);
	//fnts = (CFont*)font->font;
  return;
}
//-------------------------------------------------------------------------
//  Move verticaly
//-------------------------------------------------------------------------
void CFuiList::MoveBy(int dx,int dy)
{ CFuiComponent::MoveTo(dx + x,dy + y);
  int xp = surface->xScreen;
  int yp = surface->yScreen;
  // Update parent position of window decorations
  std::list<CFuiComponent*>::iterator i;
  for (i=decorationList.begin(); i!=decorationList.end(); i++) {
    (*i)->MoveParentTo(xp, yp);
  }
  if (hzBOX) hzBOX->MoveParentTo(xp,yp);
  if (vsBOX) vsBOX->MoveParentTo(xp,yp);
  return;
}
//-------------------------------------------------------------------------
//  Resize verticaly
//-------------------------------------------------------------------------
void CFuiList::ResizeVT(short dy)
{ h += dy;
  NbLin   = h / hLine;
  MakeSurface();
  RedimFBox(fBox,0,dy);
  if (vsBOX)  vsBOX->ResizeVT(dy);
  return;
}
//-------------------------------------------------------------------------
//  Change font to Bold Text
//-------------------------------------------------------------------------
void  CFuiList::TextPolicy(Tag pol)
{ font  = FuiFont(pol);
  return;
}
//-------------------------------------------------------------------------
//  Change Line Height
//-------------------------------------------------------------------------
short CFuiList::ChangeLineHeight(short ht)
{ hLine = ht;
  NbLin = h / hLine;
  return NbLin;
}
//-------------------------------------------------------------------------
//  Modify Line Height
//-------------------------------------------------------------------------
short CFuiList::IncLineHeight(short ht)
{ hLine += ht;
  NbLin  = h / hLine;
  return NbLin;
}
//-------------------------------------------------------------------------
//  Set Transparent mode
//-------------------------------------------------------------------------
void	CFuiList::SetTransparentMode()
{	prop |= FUI_TRANSPARENT;
	CFuiComponent *cmp = fBox[BAKW];
	if (cmp) delete cmp;
	fBox[BAKW] = 0;
	return;
}
//--------------------------------------------------------------------------
//  Change parent position for all component of this box
//--------------------------------------------------------------------------
void CFuiList::MoveParentTo (int xp, int yp)
{ CFuiComponent::MoveParentTo (xp, yp);
  int nx  = surface->xScreen;
  int ny  = surface->yScreen;
  // Update all decorations
  std::list<CFuiComponent*>::iterator i;
  for (i=decorationList.begin(); i!=decorationList.end(); i++) {
    (*i)->MoveParentTo (nx, ny);
  }
  if(hzBOX) hzBOX->MoveParentTo(nx,ny);
  if(vsBOX) vsBOX->MoveParentTo(nx,ny);
  return;
}
//--------------------------------------------------------------------------
//  Clear Page
//--------------------------------------------------------------------------
void  CFuiList::ClearPage()
{ EraseSurfaceRGBA(surface,0);
  DrawFBoxBack(fBox[BAKW]);
  return;
}
//--------------------------------------------------------------------------
//  Draw background for title
//--------------------------------------------------------------------------
void  CFuiList::SetBackTitle()
{ int cw  = (vsBOX)?(vsBOX->GetVSwide()):(0);
  int yd  = 0;
  int yf  = yd + hLine;
  FillRect(surface, 0, yd, (w - cw), yf, cBackTitle);
  return;
}
//----------------------------------------------------------------------------------
//  Clear Selected line
//----------------------------------------------------------------------------------
U_INT CFuiList::ClearLine()
{ int cw  = (vsBOX)?(vsBOX->GetVSwide()):(0);
  int yd  = rowSelected * hLine;
  int yf  = yd + hLine;
  FillRect(surface, 0, yd, (w - cw), yf, 0);
  return rowSelected;
}
//----------------------------------------------------------------------------------
//  Clear Rectangle defined by base line ln and number of lines nbl
//----------------------------------------------------------------------------------
void CFuiList::ClearBand(int ln,int nbl)
{ int cw  = (vsBOX)?(vsBOX->GetVSwide()):(0);
  int yd  = ln * hLine;
  int yf  = yd + (nbl * hLine);
  FillRect(surface, 0, yd, (w - cw), yf, 0);
  bROW = 0;
  return;
}
//----------------------------------------------------------------------------------
//  New line
//  Set selection if needed and text color
//----------------------------------------------------------------------------------
void CFuiList::NewLine(short ln)
{ cText   = colText;
  bool sl = (ln >= aROW) && (ln < bROW);
  if (!sl)    return;  
  DrawSelection(ln,cBakHLight);
  cText  = cTxtHLight;
  return;
}
//----------------------------------------------------------------------------------
//  Draw selected background
//----------------------------------------------------------------------------------
void CFuiList::DrawSelection(short ln,U_INT col)
{ int cw  = (vsBOX)?(vsBOX->GetVSwide()):(0);
  int yd  = ln * hLine;
  int yf  = yd + hLine;
  FillRect(surface, 0, yd, (w - cw), yf, col);
  return;
}
//--------------------------------------------------------------------------
//  Center Y according to line height
//--------------------------------------------------------------------------
short CFuiList::GetTextBase(short lin)
{ short yt = lin * hLine;
  if (hLine == hCar)  return yt;
  yt += (hLine - hCar) >> 1;
  return yt;
}
//--------------------------------------------------------------------------
//  Add text to line
//  lin = line number
//  col = starting column (in character wide)
//  nc  = number of character
//  txt = Text string
//--------------------------------------------------------------------------
void  CFuiList::AddText(short lin,short col,short nc, char *txt)
{ int xc  = col * wCar;
  int yd  = GetTextBase(lin);
  fnts->DrawLimChar(surface,xc, yd, nc,cText,txt);
  return;
}
//----------------------------------------------------------------------------------
//  Draw a bitmap at line number
//----------------------------------------------------------------------------------
void  CFuiList::AddBitmap(short lin,short col,SBitmap *bmap)
{ int y0  = lin * hLine;
  int x0  = col * wCar;
  if (bmap->bitmap) DrawBitmap (surface, bmap, x0, y0, 0);
  return;
}
//----------------------------------------------------------------------------------
//  Draw a Separator at Line Number
//----------------------------------------------------------------------------------
void CFuiList::AddSeparator(short ln)
{ int cw  = (vsBOX)?(vsBOX->GetVSwide()):(0);
  int yd  = ln * hLine;
  int yf  = yd + hLine - 1;
  DrawFastLine(surface,0,yf,(w - cw),yf,cText);
  return;
}
//----------------------------------------------------------------------------------
//  Draw a checkMark at line number
//----------------------------------------------------------------------------------
void CFuiList::AddMark(short lin,short col,char mrk)
{ if (0 == mrk)     return;
  int y0  = lin * hLine;
  int x0  = col * wCar;
  DrawBitmap (surface, cMark, x0, y0, 0);
  return;
}
//----------------------------------------------------------------------------------
//  Remove horizontal scroll
//----------------------------------------------------------------------------------
void CFuiList::NoHscroll()
{ if (hzBOX) delete (hzBOX);
  hzBOX = 0;
  return;
}
//----------------------------------------------------------------------------------
//  Draw the list box
//----------------------------------------------------------------------------------
void CFuiList::Draw (void)
{ DrawFBoxBack(fBox[BAKW]);
  DrawFBox(fBox,RSIZ);
  CFuiComponent::Draw ();
  if(hzBOX) hzBOX->DrawHSB();
  if(vsBOX) vsBOX->DrawVSB();
  return;
}
//----------------------------------------------------------------------------------
//  Keyboard input: process only scroll arrows
//----------------------------------------------------------------------------------
bool CFuiList::KeyboardInput(U_INT key)
{ return false;
}
//----------------------------------------------------------------------------------
//  Mouse click in the list
//----------------------------------------------------------------------------------
bool CFuiList::MouseClick (int x, int y, EMouseButton button)
{ if (!MouseHit (x, y)) return false;
  input = 1;
  scrol = 1;
  if (vsBOX && vsBOX->VtMouseClick(x,y,button)) {RegisterFocus(vsBOX); return true; }
  if (hzBOX && hzBOX->HzMouseClick(x,y,button)) {RegisterFocus(hzBOX); return true; }
  scrol = 0;
  U_SHORT line  = (y - (yParent + this->y)) / hLine;
  if (InvalidLine(line)) return true;
  MoWind->NotifyChildEvent(id, line, EVENT_SELECTITEM);
  return true;
}
//----------------------------------------------------------------------------------
//  Mouse Double click in the list
//-If the last click was on scroll bar, reroute as a normal click
//----------------------------------------------------------------------------------
bool CFuiList::DoubleClick(int x,int y,EMouseButton bt)
{ U_SHORT line  = (y - (yParent + this->y)) / hLine;
  if (InvalidLine(line))  return true;
  //---Ignore if the scroll bar is double clicked ---------
  if (scrol)              return MouseClick(x,y,bt);
  MoWind->NotifyChildEvent(id, line, EVENT_DBLE_CLICK);
  return true;
}
//----------------------------------------------------------------------------------
//  Lost focus. Stop input
//----------------------------------------------------------------------------------
void CFuiList::FocusLost()
{ input = 0;
  RegisterFocus(0);
  return;
}
//----------------------------------------------------------------------------------
//  Mouse Stop click. Notify component if any
//----------------------------------------------------------------------------------
bool CFuiList::MouseStopClick (int x, int y, EMouseButton button)
{ if (cFocus) return cFocus->MouseStopClick(x,y,button);
  return false;
}
//----------------------------------------------------------------------------------
//  Mouse move.  Notify the components
//----------------------------------------------------------------------------------
bool CFuiList::MouseMove (int mx, int my)
{ if (!MouseHit(mx,my))                 return false;
  if (vsBOX && vsBOX->MouseMove(mx,my)) return true;
  if (hzBOX && hzBOX->MouseMove(mx,my)) return true;
  return true;
}
//----------------------------------------------------------------------------------
//  Send Child notification to parent
//----------------------------------------------------------------------------------
void  CFuiList::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ MoWind->NotifyChildEvent(id,itm,evn);
  return;
}
//----------------------------------------------------------------------------------
//  Set Vertical scroll ratio
//----------------------------------------------------------------------------------
void CFuiList::SetVSRatio(float rat)
 {  if (vsBOX)  vsBOX->SetVSRatio(rat);
    return;
}
//----------------------------------------------------------------------------------
//  Set Horizontal scroll ratio
//----------------------------------------------------------------------------------
void CFuiList::SetHSRatio(float rat)
 {  if (hzBOX)  hzBOX->SetHSRatio(rat);
    return;
}
//---------------------------------------------------------------------------------
//  Check if line is valid
//---------------------------------------------------------------------------------
bool CFuiList::InvalidLine(U_INT row)
{ int end = ((row + 1) * hLine) - 1;
  return (end > h)?(true):(false);
}
//---------------------------------------------------------------------------------
//  Set Selected row
//---------------------------------------------------------------------------------
void CFuiList::SetSelectionTo(U_SHORT nl)
{ rowSelected = nl;
  return;
}
//---------------------------------------------------------------------------------
//  Underline this line
//---------------------------------------------------------------------------------
void CFuiList::Underline(short lin)
{ short yb = (lin * hLine) + (hLine - 1);
  DrawFastLine(surface,0,yb,(w-1),yb,cText);

  return;
}
//====================================================================================
// CFuiTextPopup
//====================================================================================
CFuiTextPopup::CFuiTextPopup()
{ //---  Use default font ------------------
  font = &globals->fonts.ftmono8;
  fnts = (CFont*)font->font;
  // Define colours

  black       = MakeRGB (0, 0, 0);
  darkgrey    = MakeRGB (100, 100, 100);
  mediumgrey  = MakeRGBA(164, 164, 164, 255);
  lightgrey   = MakeRGB (212, 212, 212);
  orange      = MakeRGB (212, 116,  38);
  white       = MakeRGB (255, 255, 255);
  red         = MakeRGB (255,   0,   0);
  back        = mediumgrey;
  actv        = false;
}
//-------------------------------------------------------------------------------
//  Change font
//-------------------------------------------------------------------------------
void CFuiTextPopup::ChangeFont(SFont *f)
{ font = f;
  fnts = (CFont*)font->font;
  return;
}
//-------------------------------------------------------------------------------
//  Build the text
//-------------------------------------------------------------------------------
void CFuiTextPopup::DrawTheText()
{ // Create surface for current text
	text[255] = 0;
	if (surface)	FreeSurface(surface);
  int xSpace = 12;
  int ySpace =  4;
  w = fnts->TextWidth  (this->text) + (2 * xSpace);
  h = fnts->TextHeight (this->text) + (2 * ySpace);
  surface = CreateSurface (w, h);
  surface->xScreen = x;
  surface->yScreen = y;
  EraseSurfaceRGBA (surface, back);
  // Draw borders
  DrawRect (surface, 2, 2, w-1, h-1, lightgrey);
  DrawRect (surface, 1, 1, w-2, h-2, darkgrey);

  // Draw shadowed text
  fnts->DrawNText (surface,xSpace+1 ,ySpace+1 ,black, this->text);
  fnts->DrawNText (surface,xSpace   ,ySpace   ,white, this->text);
  return;
}
//-------------------------------------------------------------------------------
//  Build the text
//-------------------------------------------------------------------------------
void CFuiTextPopup::SetText (char* text = NULL)
{ if (0 == text) return;
  // Call parent class method
  CFuiComponent::SetText (text);
  return DrawTheText();
}
//----------------------------------------------------------------------------
//  Draw if Active
//----------------------------------------------------------------------------
void CFuiTextPopup::Draw()
{ time -= globals->dST;
  if (time < 0) {actv = false;  return;}
  if (0 == *text)               return;
  CFuiComponent::Draw();
  return;
}
//----------------------------------------------------------------------------
//  Draw on request
//----------------------------------------------------------------------------
void CFuiTextPopup::DrawIt()
{ CFuiComponent::Draw();
  return;
}

//============================================================================
// CFuiGraphTrace
//============================================================================
CFuiGraphTrace::CFuiGraphTrace (Tag id, int type)
{
  this->id = id;
  this->type = type;
  colour = MakeRGB (255, 255, 255);
  minX = minY = 0.0f;
  maxX = maxY = 100.0f;
}

void CFuiGraphTrace::AddPoint (float x, float y)
{
  SFuiGraphTracePoint p;
  p.x = x;
  p.y = y;
  points.push_back (p);
}

void CFuiGraphTrace::ClearPoints (void)
{
  points.clear();
}

void CFuiGraphTrace::SetRange (float minX, float minY, float maxX, float maxY)
{
  this->minX = minX;
  this->minY = minY;
  this->maxX = maxX;
  this->maxY = maxY;
}

void CFuiGraphTrace::SetColour (unsigned int colour)
{
  this->colour = colour;
}

void CFuiGraphTrace::Draw (SSurface *surface)
{
  static int prevX = 0;
  static int prevY = 0;

  std::deque<SFuiGraphTracePoint>::iterator i;
  for (i=points.begin(); i!=points.end(); i++) {
    float x = i->x;
    float y = i->y;

    float dx = (x - minX) / (maxX - minX);
    float dy = (maxY - y) / (maxY - minY);
    if (dx < 0) dx = 0;
    if (dx > 1.0f) dx = 1.0f;
    if (dy < 0) dy = 0;
    if (dy > 1.0f) dy = 1.0f;

    int px = (int)((float)surface->xSize * dx);
    int py = (int)((float)surface->ySize * dy);

    if (i == points.begin()) {
      // Initialize previous x and y offets for line trace
      prevX = px;
      prevY = py;
    }

    switch (type) {
    case FUI_GRAPH_TRACE_POINT:
      DrawDot (surface, px, py, colour);
      break;

    case FUI_GRAPH_TRACE_LINE:
      DrawFastLine (surface, prevX, prevY, px, py, colour);
      prevX = px;
      prevY = py;
      break;
    }
  }
}


//================================================================================
// CFuiGraph
//=================================================================================
CFuiGraph::CFuiGraph (void)
{
  type = COMPONENT_GRAPH;
  widgetTag = 'defa';
  wName			= "Graph";

  useGrid = false;
  minX = minY = 0;
  maxX = maxY = 100.0f;
  stepX = stepY = 10.0f;

  // Initialize background and grid colours
  bgColour = MakeRGB (1, 1, 1);
  gridColour = MakeRGB (64, 64, 64);
}

int CFuiGraph::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;
  switch (tag) {
  case 'grid':
    {
      int i = 0;
      ReadInt (&i, stream);
      useGrid = (i != 0);
    }
    rc = TAG_READ;
    break;
  case 'grdr':
    ReadFloat (&minX, stream);
    ReadFloat (&minY, stream);
    ReadFloat (&maxX, stream);
    ReadFloat (&maxY, stream);
    ReadFloat (&stepX, stream);
    ReadFloat (&stepY, stream);
    rc = TAG_READ;
    break;
  }

  // Send tag to parent class for processing
  if (rc == TAG_IGNORED) {
    rc = CFuiComponent::Read (stream, tag);
  }

  return rc;
}

void CFuiGraph::Draw (void)
{
  if (surface != NULL) {

    // Fill surface with background colour
    EraseSurfaceRGB (surface, bgColour);

    // Draw grid if necessary
    if (useGrid) {
      // Horizontal grid lines
      float y = stepY;
      while (y <= surface->ySize) {
        float dy = (maxY - y) / (maxY - minY);
        if (dy < 0) dy = 0;
        if (dy > 1.0f) dy = 1.0f;
        int py = (int)((float)surface->ySize * dy);
        DrawFastLine (surface, 0, py, surface->xSize, py, gridColour);
        y += stepY;
      }

      // Vertical grid lines
      float x = stepX;
      while (x <= surface->xSize) {
        float dx = (x - minX) / (maxX - minX);
        if (dx < 0) dx = 0;
        if (dx > 1.0f) dx = 1.0f;
        int px = (int)((float)surface->xSize * dx);
        DrawFastLine (surface, px, 0, px, surface->ySize, gridColour);
        x += stepX;
      }
    }

    // Notify application to update trace data

    // Draw traces
    std::map<Tag,CFuiGraphTrace*>::iterator i;
    for (i=traces.begin(); i!=traces.end(); i++) {
      CFuiGraphTrace *t = i->second;
      t->Draw (surface);
    }
  }

  CFuiComponent::Draw ();
}

void CFuiGraph::AddTrace (Tag traceID, int traceType)
{
  std::map<Tag,CFuiGraphTrace*>::iterator i = traces.find(traceID);
  if (i == traces.end()) {
    traces[traceID] = new CFuiGraphTrace (traceID, traceType);
  } else {
    WARNINGLOG ("CFuiGraph::AddTrace : Duplicate trace");
  }
}

void CFuiGraph::RemoveTrace (Tag traceID)
{
  std::map<Tag,CFuiGraphTrace*>::iterator i = traces.find(traceID);
  if (i != traces.end()) {
    traces.erase (i);
  } else {
    WARNINGLOG ("CFuiGraph::RemoveTrace : Non-existent trace");
  }
}

void CFuiGraph::RemoveAllTraces (void)
{
  traces.clear();
}

void CFuiGraph::AddTracePoint(Tag traceID, float x, float y)
{
  std::map<Tag,CFuiGraphTrace*>::iterator i = traces.find(traceID);
  if (i != traces.end()) {
    CFuiGraphTrace* t = i->second;
    t->AddPoint (x, y);
  } else {
    WARNINGLOG ("CFuiGraph::AddTracePoint : Non-existent trace");
  }
}

void CFuiGraph::ClearTracePoints (Tag traceID)
{
  std::map<Tag,CFuiGraphTrace*>::iterator i = traces.find(traceID);
  if (i != traces.end()) {
    CFuiGraphTrace* t = i->second;
    t->ClearPoints ();
  } else {
    WARNINGLOG ("CFuiGraph::ClearTracePoints : Non-existent trace");
  }
}

void CFuiGraph::SetTraceRange (Tag traceID, float minX, float minY, float maxX, float maxY)
{
  std::map<Tag,CFuiGraphTrace*>::iterator i = traces.find(traceID);
  if (i != traces.end()) {
    CFuiGraphTrace* t = i->second;
    t->SetRange (minX, minY, maxX, maxY);
  } else {
    WARNINGLOG ("CFuiGraph::SetTraceRange : Non-existent trace");
  }
}

void CFuiGraph::SetTraceColour (Tag traceID, unsigned int colour)
{
  std::map<Tag,CFuiGraphTrace*>::iterator i = traces.find(traceID);
  if (i != traces.end()) {
    CFuiGraphTrace* t = i->second;
    t->SetColour (colour);
  } else {
    WARNINGLOG ("CFuiGraph::SetTraceColour : Non-existent trace");
  }
}

void CFuiGraph::SetUseGrid (int useGrid)
{
  this->useGrid = (useGrid != 0);
}

void CFuiGraph::SetGridRange (float minX, float minY, float maxX, float maxY)
{
  this->minX = minX;
  this->minY = minY;
  this->maxX = maxX;
  this->maxY = maxY;
}

void CFuiGraph::SetGridSpacing (float stepX, float stepY)
{
  this->stepX = stepX;
  this->stepY = stepY;
}

void CFuiGraph::SetGridColour (unsigned int colour)
{
  this->gridColour = colour;
}

void CFuiGraph::SetGridBackColour (unsigned int colour)
{
  this->bgColour = colour;
}

void CFuiGraph::GetGridRange(float *minX, float *minY, float *maxX, float *maxY)
{
  *minX = this->minX;
  *minY = this->minY;
  *maxX = this->maxX;
  *maxY = this->maxY;
}

void CFuiGraph::GetGridSpacing(float *stepX, float *stepY)
{
  *stepX = this->stepX;
  *stepY = this->stepY;
}
//=============================================================================
// CFuiMenu
//==============================================================================
CFuiMenu::CFuiMenu (Tag id, int pos, const char* label, CFuiComponent *mwin)
{
  type = COMPONENT_MENU;
  widgetTag = 'defa';
  wName			= "Menu";
  MoWind        = mwin;
  strncpy (text,label,255);
  this->id = id;
  this->fontTag = 0;
  this->font    = &globals->fonts.ftasci10;
  fnts          = (CFont*)font->font;
  colLine = 0;
  tColor[0]     = MakeRGB(255,255,255);
  tColor[1]     = MakeRGB(255,255,255);
  tColor[2]     = 0;
  cBackPane     = MakeRGB (130,153,196);
  cBakHLight    = MakeRGB (  0,  0,255);
  mState        = 0;
  mPart         = 0;
  mPane         = 0;
  //--Compute menu parameters ---------------------------------------------
  w     = fnts->TextWidth ((char*)label) + 6;
  h     = fnts->TextHeight("H");
  x     = pos;
  y     = 0;
  vDim  = 0;
  hText = h + 4;
  hLine = 6;
  //----Compute pane Parameters ------------------------------------------
  xPane = pos;
  yPane = h + 4;
  pWd   = 0;
  pHt   = 0;
  bmCK        = 0;
  ckw = ckh   = 0;
  MakeSurface ();
}
//------------------------------------------------------------------------------
//  Destroy all components
//------------------------------------------------------------------------------
CFuiMenu::~CFuiMenu (void)
{ if (mPane) mPane->pSurf = FreeSurface(mPane->pSurf);
  SAFE_DELETE(mPane);
  Cleanup ();
}
//------------------------------------------------------------------------------
//  All Tags read
//  Set definitive properties
//------------------------------------------------------------------------------
void CFuiMenu::ReadFinished (void)
{ CFuiThemeWidget *chtm = FindThemeWidget('defa',"CheckBox");
  bmCK        = chtm->GetBitmap("BACK");
  if (bmCK) bmCK->GetBitmapSize(&ckw,&ckh);
  FindThemeWidget ();
  colLine     = tw->GetColour ("LINE");
  tColor[0]   = tw->GetColour ("TEXT");
  tColor[2]   = tw->GetColour ("TEXTGREY");
  Back[NORM]  = cBackPane;
  Back[HILI]  = cBakHLight;
  //-------Create Menu Pane ---------------------------------------
  pWd        += ckw + 6;                      // Adjust wide for check mark
  pHt         = vDim;                         // Total Height
  mPane       = new SMenuPane;
  mPane->xp   = x;
  mPane->yp   = 0;
  mPane->wp   = pWd;
  mPane->hp   = vDim;
  mPane->pSurf  = CreateSurface (mPane->wp, mPane->hp);
  if (0 == mPane->pSurf)     return;
  mPane->pSurf->xScreen = xParent + xPane;
  mPane->pSurf->yScreen = yParent + yPane;
  EraseSurfaceRGBA (mPane->pSurf,0);
  SetState(0);
  Initialize();
  return;
}

//------------------------------------------------------------------------------
//  Relocate 
//------------------------------------------------------------------------------
void CFuiMenu::MoveParentTo (int xp, int yp)
{  CFuiComponent::MoveParentTo (xp, yp);
  //-----Relocate Menu Pane ---------------------------------
  if (0 == mPane)       return;
  SSurface *sp = mPane->pSurf;
  if (0 == sp)          return;
  sp->xScreen = xp + xPane;
  sp->yScreen = yp + yPane;
  return;
}
//-----------------------------------------------------------------------
//  Clean all Menu parts
//-----------------------------------------------------------------------
void CFuiMenu::Cleanup (void)
{ for (U_INT k= 0; k < Parts.size(); k++) delete Parts[k];
  Parts.clear();
}
//----------------------------------------------------------------------
//    Add text menu part
//----------------------------------------------------------------------
void CFuiMenu::AddItem (Tag idn, const char* label)
{ SMenuPart *mp = new SMenuPart;
  mp->Type    = (char)(TEXT);
  mp->State   = (char)(NORM);
  mp->Check   = 0;
  mp->yPos    = vDim;
  mp->hDim    = hText;
  mp->Iden    = idn;
  strncpy(mp->Text,label,63);
  //-----------------------------------------------------------
  vDim        +=  hText;
  Parts.push_back(mp);
  int txw      = fnts->TextWidth((char*)label);
  if (txw > pWd) pWd = txw;
  return;
}
//-----------------------------------------------------------------------------------
//  Add a separator
//-----------------------------------------------------------------------------------
void CFuiMenu::AddSeparator (void)
{SMenuPart *mp = new SMenuPart;
  mp->Type    = (char)(LINE);
  mp->State   = (char)(NORM);
  mp->yPos    = vDim;
  mp->hDim    = 6;
  mp->Text[0] = 0;
  mp->Iden    = 0;
    //-----------------------------------------------------------
  vDim        +=  hLine;
  Parts.push_back(mp);
  return;
}
//---------------------------------------------------------------------------------
void CFuiMenu::ClearItems (void)
{
  Cleanup();
}
//---------------------------------------------------------------------------------
//  Initialize Menu
//---------------------------------------------------------------------------------
void CFuiMenu::Initialize()
{ std::vector<SMenuPart*>::iterator ip;
  for (ip = Parts.begin(); ip != Parts.end(); ip++)
  { SMenuPart *mp = (*ip);
    SetPartState(mp,(char)NORM);
  }
  return;
}
//---------------------------------------------------------------------------------
//  Return Part 
//---------------------------------------------------------------------------------
SMenuPart *CFuiMenu::GetMenuPart(Tag itm)
{ std::vector<SMenuPart*>::iterator ip;
  for (ip=Parts.begin(); ip!=Parts.end(); ip++) 
  { SMenuPart *mp = (*ip);
     if (mp->Iden != itm) continue;
     return mp;
  }
  return 0;
}
//---------------------------------------------------------------------------------
//  Check Selected Part
//---------------------------------------------------------------------------------
void CFuiMenu::CheckSelectedPart(Tag idn)
{ if (0 == mPart)         return;
  if (mPart->Iden != idn) return;
  mPart->Check ^= 1;
  return; 
}
//---------------------------------------------------------------------------------
//  Set Selected Part
//---------------------------------------------------------------------------------
void CFuiMenu::ChangeState(Tag idn,char st)
{ SMenuPart *mp = GetMenuPart(idn);
	if (0 == mp)         return;
  mPart->Check = st;
  return; 
}
//---------------------------------------------------------------------------------
//  Swap Menu Part
//---------------------------------------------------------------------------------
void  CFuiMenu::SwapPart(SMenuPart *mp)
{ if( mPart) SetPartState(mPart,(char)(NORM));
  mPart     = mp;
  SetPartState(mp,(char)(HILI));
  return;
}
//---------------------------------------------------------------------------------
//  Change Menu State
//---------------------------------------------------------------------------------
void  CFuiMenu::SetState(int sta)
{ mState = (char)sta;
  fnts->DrawNText(surface,0,0,tColor[mState],text);
  return;
}
//---------------------------------------------------------------------------------
//  Check for Active menu click
//----------------------------------------------------------------------------------
bool CFuiMenu::PaneHit(int x,int y)
{ int x0  = xParent + xPane;
  int y0  = yParent + yPane;
  if ((x < x0) || (y < y0))   return false;
  if (x > (x0 + pWd))         return false;
  if (y > (y0 + pHt))         return false;
  return true;
}
//---------------------------------------------------------------------------------
//  Check if mouse move over the active menu
//----------------------------------------------------------------------------------
bool CFuiMenu::MouseMove(int x,int y)
{ int yp  = 0;
  int y0  = yParent + yPane;
  if (false == PaneHit(x,y))  return false;
  //---Locate part ------------------------------------------------
  std::vector<SMenuPart*>::iterator ip;
  for (ip=Parts.begin(); ip!=Parts.end(); ip++) 
  { SMenuPart *mp = (*ip);
    yp  = y0 + mp->yPos;
    SetPartState(mp,(char)(NORM));
    if ((y < yp) || (y > (yp + mp->hDim))) continue;
    if (TEXT != mp->Type)               return true;
    SwapPart(mp);
    return true;
  }
  return true;
}
//---------------------------------------------------------------------------------
//  Click inside active pane
//  Normaly the item selected is pointed by mPart set during mouse move
//---------------------------------------------------------------------------------
bool  CFuiMenu::PaneClick(int x,int y, EMouseButton btn)
{ if (0 == mPart)       return false;
  //--------Notify Mother windows of this event ------------------------
  if (0 == MoWind)      return true;
  MoWind->NotifyMenuEvent(0, mPart->Iden);
  return true; }
//---------------------------------------------------------------------------------
//  Focus is Lost
//---------------------------------------------------------------------------------
void CFuiMenu::FocusLost(void)
{ SetState(0);
  mPart  = 0;
  return;
}
//---------------------------------------------------------------------------------
//  Change state of menu part
//---------------------------------------------------------------------------------
void CFuiMenu::SetPartState(SMenuPart *mp,char st)
{ if (TEXT == mp->Type) SetTextState(mp,st);
  if (LINE == mp->Type) SetLineState(mp,st);
  return;
}
//---------------------------------------------------------------------------------
//  Change state of menu part
//---------------------------------------------------------------------------------
void CFuiMenu::SetTextState(SMenuPart *mp,char st)
{ mp->State     = st;
  SSurface  *sf = (mPane)?(mPane->pSurf):(0);
  int        y0 = mp->yPos;
  int        y1 = y0 +hText;
  FillRect(sf,0,y0,pWd,y1,Back[mp->State]);
  if (bmCK) bmCK->DrawBitmap(sf,2,y0+2,mp->Check);
  fnts->DrawNText(sf,(ckw + 4),y0,tColor[mp->State],mp->Text);
  return;
}
//---------------------------------------------------------------------------------
//  Draw a menu separator
//---------------------------------------------------------------------------------
void CFuiMenu::SetLineState(SMenuPart *mp,char st)
{ SSurface  *sf = (mPane)?(mPane->pSurf):(0);
  int        y0 = mp->yPos;
  int        y1 = y0 + hLine;
  FillRect(sf,0,y0,pWd,y1,Back[mp->State]);
  DrawFastLine(sf,0,y0,pWd,y0,colLine);
  return;
}
//---------------------------------------------------------------------------------
//  Draw Menu Pane
//---------------------------------------------------------------------------------
void CFuiMenu::DrawPane()
{ SSurface  *sf = (mPane)?(mPane->pSurf):(0);
  CFuiComponent::Draw(sf);
  return;
}
//---------------------------------------------------------------------------------
//  DRaw menu color according to state
//---------------------------------------------------------------------------------
void CFuiMenu::Draw (void)
{ CFuiComponent::Draw();
  if (mState != 1)  return;
  DrawPane();
  return;
}

//============================================================================================
// CFuiMenuBar
//
// Main application menu
//
// Theme components:
//   - BITMAPs BACK, LEFT, RIGHT
//   - COLOR TEXT        -  Normal text colour
//   - COLOR HILITE      - Highlight background colour
//   - COLOR TEXTHILITE  - Highlight text colour
//===========================================================================================
CFuiMenuBar::CFuiMenuBar (int wd,CFuiComponent *mwin)
{
  type = COMPONENT_MENUBAR;
  widgetTag = 'defa';
  wName		= "MenuBar";
  mPos    = 8;
  mSel    = 0;
  width   = 8;
  MoWind  = mwin;                 
  w       = wd;
  state   = 1;
}
//------------------------------------------------------------------------------------
CFuiMenuBar::~CFuiMenuBar (void)
{ std::map<Tag, CFuiMenu*>::iterator mi;
  for (mi = menu.begin();mi != menu.end();mi++) delete mi->second;
	menu.clear();
  return;
}
//------------------------------------------------------------------------------------
//  Read Tags
//------------------------------------------------------------------------------------
int CFuiMenuBar::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;
  Tag t;
  char s[80];
  int i;

  static Tag menuTag = 0;

  switch (tag) {
  case 'menu':
    {
      // Start of the next menu specification
      ReadTag (&t, stream);
      ReadString (s, 80, stream);
      AddMenu (t, s, fontTag);
      
      // Save menu tag ID for any subsequent <item> or <sepi> objects
      menuTag = t;
      rc = TAG_READ;
    }
    break;

  case 'item':
    // Start of the next menu specification
    ReadTag (&t, stream);
    ReadString (s, 80, stream);
    if (menuTag != 0)  AddMenuItem (menuTag, t, s);
    rc = TAG_READ;
    break;

  case 'sepi':
    // Separator
    ReadInt (&i, stream);
    if (menuTag != 0)  AddMenuSeparator (menuTag);
    rc = TAG_READ;
    break;
  }

  // Send tag to parent class for processing
  if (rc == TAG_IGNORED) {
    rc = CFuiComponent::Read (stream, tag);
  }

  return rc;
}
//------------------------------------------------------------------------------------
//  All tags read
//------------------------------------------------------------------------------------
void CFuiMenuBar::ReadFinished (void)
{ w = (mPos + 4);                                 // Definitive width
  CFuiComponent::ReadFinished ();
  FindThemeWidget ();
  CreateHBox(hBox,w);
 
  colText = tw->GetColour ("TEXT");
  colHighlight = tw->GetColour ("HILITE");
  colHighlightText = tw->GetColour ("HILITETEXT");

  // Call ReadFinished() for all submenus
  std::map<Tag,CFuiMenu*>::iterator i;
  for (i=menu.begin(); i!=menu.end(); i++) {
    CFuiMenu* m = i->second;
    m->ReadFinished ();
  }
}
//--------------------------------------------------------------------
//  Relocate Menubar with all children
//--------------------------------------------------------------------
void CFuiMenuBar::MoveTo(int xs, int ys)
{
  CFuiComponent::MoveTo (xs, ys);
  int nx = surface->xScreen;
  int ny = surface->yScreen;
  // Update all children ----------------------------------
  std::list<CFuiComponent*>::iterator ic;
  for (ic=decorationList.begin(); ic!=decorationList.end(); ic++) {
    (*ic)->MoveParentTo(nx,ny);
  }
  // Update parent position of child widgets
  std::map<Tag,CFuiMenu*>::iterator ide;
  for (ide=menu.begin(); ide!=menu.end(); ide++) {
    CFuiMenu* m = ide->second;
    m->MoveParentTo(nx, ny);
  }
 return;
}

//------------------------------------------------------------------------
//  Parent move: Relocate all items
//------------------------------------------------------------------------
void CFuiMenuBar::MoveParentTo (int xp, int yp)
{ CFuiComponent::MoveParentTo(xp, yp);
  int nx = surface->xScreen;
  int ny = surface->yScreen;
  // Update all children ----------------------------------
  std::list<CFuiComponent*>::iterator ic;
  for (ic=decorationList.begin(); ic!=decorationList.end(); ic++) {
    (*ic)->MoveParentTo(nx,ny);
  }
  // Update all menu item
  std::map<Tag,CFuiMenu*>::iterator im;
  for (im=menu.begin(); im!=menu.end(); im++) {
    CFuiMenu* m = im->second;
    m->MoveParentTo(nx,ny);
  }
  return;
}
//-----------------------------------------------------------------------
//  Add a Menu
//-----------------------------------------------------------------------
void CFuiMenuBar::AddMenu (Tag menuID, const char *label, Tag fontTag)
{
  CFuiMenu* m = new CFuiMenu (menuID,mPos,label,this);
  menu[menuID] = m;
  mPos  += m->GetWide() + 4;
}
//------------------------------------------------------------------------
//  Remove a Menu
//------------------------------------------------------------------------
void CFuiMenuBar::RemoveMenu (Tag menuID)
{
  std::map<Tag, CFuiMenu*>::iterator i = menu.find(menuID);
  if (i != menu.end()) {
    delete i->second;
    menu.erase (i);
  }
}
//-----------------------------------------------------------------------------------
//  Return a Menu Part
//-----------------------------------------------------------------------------------
SMenuPart *CFuiMenuBar::GetMenuPart(Tag idm, Tag itm)
{ CFuiMenu *men = GetMenu(idm);
  if (0 == men)   return 0;
  return men->GetMenuPart(itm);
}
//-----------------------------------------------------------------------------------
//  Locate Menu by Tag
//-----------------------------------------------------------------------------------
CFuiMenu* CFuiMenuBar::GetMenu (Tag menuID)
{ std::map<Tag, CFuiMenu*>::iterator i = menu.find(menuID);
  return (i != menu.end())?(i->second):(0);
}
//----------------------------------------------------------------------------------
//  Add a menu item
//----------------------------------------------------------------------------------
void CFuiMenuBar::AddMenuItem (Tag menuID, Tag itemID, const char *label)
{
  CFuiMenu* men = GetMenu (menuID);
  if (men == 0)       return;
  men->AddItem (itemID, label);
  return;
}
//---------------------------------------------------------------------------------
//  Add separator
//---------------------------------------------------------------------------------
void CFuiMenuBar::AddMenuSeparator (Tag menuID)
{ CFuiMenu* m = GetMenu (menuID);
  if (m)    m->AddSeparator ();
}

//---------------------------------------------------------------------------
//  Check current menu Item
//---------------------------------------------------------------------------
void CFuiMenuBar::CheckSelectedPart(Tag idm,Tag itm)
{ if ((0 == mSel) || (mSel->GetId() != idm))  return;
  mSel->CheckSelectedPart(itm);
  return;
}
//---------------------------------------------------------------------------
//  Check current menu Item
//---------------------------------------------------------------------------
void CFuiMenuBar::ChangePartState(Tag idm,Tag itm,char st)
{ CFuiMenu *mn = GetMenu(idm);
	if (0 == mn)		return;
  mn->ChangeState(itm,st);
  return;
}
//---------------------------------------------------------------------------
//  Mouse move over menu Bar
//---------------------------------------------------------------------------
bool CFuiMenuBar::MouseMove(int x, int y)
{ if (0 == state)         return CFuiComponent::MouseHit(x,y);
  if (0 == mSel)          return MouseHit(x,y);
  if (SelectMenu(x,y))    return true;
  return (mSel)?(mSel->MouseMove(x,y)):(false);
}
//---------------------------------------------------------------------------
//  MenuBar lost focus
//---------------------------------------------------------------------------
void CFuiMenuBar::FocusLost()
{ if (mSel) mSel->SetState(0);
  mSel  = 0;
  return;
}
//---------------------------------------------------------------------------
//  Swap selected menu
//---------------------------------------------------------------------------
void  CFuiMenuBar::SwapMenu(CFuiMenu *men)
{ if (mSel) mSel->SetState(0);
  mSel  = men;
  mSel->SetState(1);
  return;
}
//---------------------------------------------------------------------------
//  Select menu by x, y
//---------------------------------------------------------------------------
bool CFuiMenuBar::SelectMenu(int x, int y)
{ std::map<Tag, CFuiMenu*>::iterator i;
  for (i=menu.begin(); i!=menu.end(); i++) {
    CFuiMenu *m = i->second;
    if (!m->MouseHit(x,y))  continue;
    if ( m->IsGrayed())     return true;
    SwapMenu(m);
    return true;
  }
  return false;
}
//---------------------------------------------------------------------------
//  Check if mouse hit one of the menu
//---------------------------------------------------------------------------
/*
bool CFuiMenuBar::MouseHit(int x,int y)
{ if (CFuiComponent::MouseHit(x,y)) return true;
  return (mSel)?(mSel->PaneHit(x,y)):(false);
}
*/
//---------------------------------------------------------------------------
//  Click on Menu BAR.  Search the involved menu
//---------------------------------------------------------------------------
bool CFuiMenuBar::MouseClick (int x, int y, EMouseButton button)
{ if (mSel && mSel->PaneHit(x,y))   return mSel->PaneClick(x,y,button); 
  if (!MouseHit(x,y))               return false;
  if  (0 == state)                  return false;
  return SelectMenu(x,y);
  }
//----------------------------------------------------------------------
//  Notify Parent window for menu Event
//----------------------------------------------------------------------
void CFuiMenuBar::NotifyMenuEvent(Tag idm, Tag itm)
{ if ((0 == MoWind) || (0 == mSel))  return;
  MoWind->NotifyMenuEvent(mSel->GetId(),itm);
  return;
}

//----------------------------------------------------------------------
//  Draw the bar
//----------------------------------------------------------------------
void CFuiMenuBar::Draw (void)
{ // Draw menus Bar----------
  DrawHBox(hBox);
  // Draw menus -------------
  std::map<Tag, CFuiMenu*>::iterator i;
  for (i=menu.begin(); i!=menu.end(); i++) {
    CFuiMenu *m = i->second;
    m->Draw ();
  }
}


//======================================================================
// CFuiWindowMenuBar
//
// Sub-window menu
//
// Theme components:
//   - BITMAPs BACK, LEFT, RIGHT
//   - COLOR TEXT      - Normal text colour
//   - COLOR HILITE      - Highlight background colour (default black)
//   - COLOR TEXTHILITE  - Highlight text colour (default white)
//
CFuiWindowMenuBar::CFuiWindowMenuBar ()
:CFuiMenuBar(0)
{
  type = COMPONENT_WINDOW_MENUBAR;
  widgetTag = 'defa';
  wName     = "WindowMenuBar";
  x = 50;
  y = 50;
}

CFuiWindowMenuBar::~CFuiWindowMenuBar (void)
{
}
//=============================================================================
//  CFuiCanva to draw runway for detail airport
//=============================================================================
CFuiCanva::CFuiCanva (int x, int y, int w, int h, CFuiComponent *win)
: CFuiComponent (x, y, w, h, win)
{
  type = COMPONENT_RUNWAY_GRAPH;
  widgetTag = 'defa';
  wName		= "Graph";
  vscr    = 0;
  hscr    = 0;
  row     = 0;
  InitFBox(fBox,RSIZ);
  white   = MakeRGB(255,255,255);
  black   = MakeRGB(0,0,0);
  cTab[0] = MakeRGB(0,0,0);             // Normal drawing black
  cTab[1] = MakeRGB(255,0,0);           // Selected drawing red
  //-----------------------------------------------------------------
  move    = 0;
  Cam     = 0;
}
//-----------------------------------------------------------------------------
//  Destroy the window
//-----------------------------------------------------------------------------
CFuiCanva::~CFuiCanva()
{
}
//-----------------------------------------------------------------------------
//  Read All tags
//-----------------------------------------------------------------------------
int CFuiCanva::Read (SStream *stream, Tag tag)
{ int nb;
  switch (tag) {
  case 'vscr':
    ReadInt (&nb, stream);
    vscr = (U_CHAR)nb;
    return  TAG_READ;

  case 'hscr':
    ReadInt (&nb, stream);
    hscr = (U_CHAR)nb;
    return  TAG_READ;
  }

  // Send tag to parent class for processing
  return CFuiComponent::Read (stream, tag);
}
//-------------------------------------------------------------------------
//  All parameters are read.  Create the real window now
//-------------------------------------------------------------------------
void CFuiCanva::ReadFinished (void)
{
  CFuiComponent::ReadFinished ();
  InitQuad();
  FindThemeWidget ();
  //-------- Compute definitive drawing surface ----------------
  halfW = (w >> 1);          // Half wide
  halfH = (h >> 1);          // Half height
  CreateFBox(this,fBox,x,y,w,h);
  //--------- Get back colour ----------------------------------
  if (HasProperty(FUI_TRANSPARENT)) white = 0;
  ColorInside (white);
  hLin  = fnts->TextHeight("H");
  wCar  = fnts->TextWidth("H");
  return;
}

//-------------------------------------------------------------------
//  Set Parent position to all components
//-------------------------------------------------------------------
void CFuiCanva::MoveParentTo (int xp, int yp)
{
  CFuiComponent::MoveParentTo (xp, yp);

  // Update all decorations
  std::list<CFuiComponent*>::iterator i;
  for (i=decorationList.begin(); i!=decorationList.end(); i++) {
    (*i)->MoveParentTo (xParent + x, yParent + y);
  }
  return;
}
//-------------------------------------------------------------------
//  Draw a Character 
//-------------------------------------------------------------------
void CFuiCanva::DrawChar(int x0,int y0,char c,U_INT col)
{	fnts->DrawChar(surface,x0,y0,c,cTab[col]);
}
//-------------------------------------------------------------------
//  Draw a text 
//-------------------------------------------------------------------
void CFuiCanva::DrawNText(int x0,int y0,char *t,U_INT c)
{	fnts->DrawNText(surface,x0,y0,cTab[c],t);
}
//-------------------------------------------------------------------
//  Draw a line 
//-------------------------------------------------------------------
void CFuiCanva::DrawSegment(int x0,int y0,int x1,int y1,int xc)
{ DrawFastLine(surface,x0,y0,x1,y1,cTab[xc]);
  return;
}
//-------------------------------------------------------------------
//  Erase the canvas
//-------------------------------------------------------------------
void CFuiCanva::EraseCanvas()
{ ColorInside (white);
  return;
}
//-------------------------------------------------------------------
//  Begin a new page
//-------------------------------------------------------------------
void CFuiCanva::BeginPage()
{ row = 0;
  ColorInside(white);
  return;
}
//-------------------------------------------------------------------
//  Add a line of text
//-------------------------------------------------------------------
void  CFuiCanva::AddText(short col, char *txt,char nl)
{ U_SHORT xc  = col * wCar;
  U_SHORT yd  = row * hLin;
  if (yd < surface->ySize) fnts->DrawNText(surface,xc ,yd ,black ,txt);
  row += nl;
  return;
}
//-------------------------------------------------------------------
//  Add a line of text with edit parameters
//-------------------------------------------------------------------
void  CFuiCanva::AddText(short col, char nl,char *fmt, ...)
{ U_SHORT xc  = col * wCar;
  U_SHORT yd  = row * hLin;
  if (yd < surface->ySize)
  { va_list argp;
    va_start(argp, fmt);
    _vsnprintf(text,128,fmt,argp);
    fnts->DrawNText(surface,xc ,yd ,black ,text);
  }
  row += nl;
  return;
}
//-------------------------------------------------------------------
//  Mouse Click.  Store coordinates
//-------------------------------------------------------------------
bool CFuiCanva::MouseClick (int mx, int my, EMouseButton button)
{ if (MOUSE_BUTTON_LEFT != button)  return false;
  if (!MouseHit(mx,my))             return false;
  move  = 1;
  sx    = mx;
  sy    = my;
  return true;
}
//-------------------------------------------------------------------
//  Mouse move.  Advise registered Camera
//-------------------------------------------------------------------
bool CFuiCanva::MouseMove(int mx,int my)
{ if (0 == Cam)       return false;
  if (0 == move)      return false;
  int dx = mx - sx;
  int dy = my - sy;
  Cam->MoveBy(dx,dy);
  sx  = mx;
  sy  = my;
  return true;
}
//-------------------------------------------------------------------
//  Mouse stop move.  
//-------------------------------------------------------------------
bool CFuiCanva::MouseStopClick(int x,int y,EMouseButton but)
{ move = 0;
  return true;
}
//-------------------------------------------------------------------
//  Return view port data
//-------------------------------------------------------------------
void CFuiCanva::GetViewPort(VIEW_PORT &vp)
{ vp.wd = surface->xSize;
  vp.ht = surface->ySize;
  vp.x0 = surface->xScreen;
  vp.y0 = globals->mScreen.Height - surface->yScreen - vp.ht;
  return;
}
//-------------------------------------------------------------------
//  Draw the canvas
//-------------------------------------------------------------------
void CFuiCanva::Draw()
{ if (Cam)   return;
  DrawFBoxBack(fBox[BAKW]);
  DrawFBox(fBox,RSIZ);
  CFuiComponent::Draw();
  return;
}

//=============================================================================
//  CFuiPage to draw text on List and popup
//  sl is the selected line when different from 0
//=============================================================================
CFuiPage::CFuiPage (int x, int y, FL_MENU *sm, CFuiComponent *win,short sl)
: CFuiComponent (x, y, 0, 0, win)
{ menu    = sm;
  type = COMPONENT_PAGE;
  widgetTag = 'defa';
  wName		= "List";
  font    = FuiFont (fontTag);
  fnts    = (CFont*)font->font;
  hCar    = fnts->TextHeight("H");
  wCar    = fnts->TextWidth ("H");
  State   = 0;
  InitMenu(sm);
  w       = sm->NbCar + (wCar << 1);              // **MARK * wCar;
  h       = sm->NbLin * hCar + 2;
  halfW   = (w >> 1);          // Half wide
  halfH   = (h >> 1);          // Half height
  //---------------------------------------
  cBackPane   = MakeRGB (130,153,196);
  colText     = MakeRGB (0, 0, 0);
  cTxtHLight  = MakeRGB (0, 0, 255);
  cBakHLight  = MakeRGB (200, 0, 200);
  grey        = MakeRGB ( 30, 30, 30);
  //---------------------------------------
  sLin    = sl;
  Initialize();
}
//-----------------------------------------------------------------------------
//  Destroy the window
//-----------------------------------------------------------------------------
CFuiPage::~CFuiPage()
{
}
//------------------------------------------------------------------------
//  Init the menu part
//------------------------------------------------------------------------
void CFuiPage::InitMenu(FL_MENU *sm)
{ int stl   = 0;
  sm->NbCar = 0;
  sm->NbLin = 0;
  int k     = 0;
  char *itm = sm->aText[k];
  while ((itm) && (*itm))
  { sm->NbLin++;
    stl = fnts->TextWidth(itm);
    if (stl > sm->NbCar)  sm->NbCar = stl;
    k++;
    itm = sm->aText[k];
  }
  return;
}
//-------------------------------------------------------------------------
//  All parameters are read.  Create the real window now
//-------------------------------------------------------------------------
void CFuiPage::Initialize ()
{ MakeSurface ();
  tw = globals->fui->GetThemeWidget (widgetTag, wName);
  //---------Text color ---------------------------------------
  colText     = tw->GetColour ("TEXT");
  cTxtHLight  = tw->GetColour ("TEXTHILITE");
  cBakHLight  = tw->GetColour ("HILITE");
  cText[0]    = colText;
  cText[1]    = cTxtHLight;
  cBack[0]    = cBackPane;
  cBack[1]    = cBakHLight;
  //--------------Edit the menu text ---------------------------
  DrawPane();                   // Clear panel
  for (short ln = 0; ln != menu->NbLin; ln++) AddPopupText(ln, (char*)menu->aText[ln]);
  return;
}
//--------------------------------------------------------------------------
//  Set page state
//--------------------------------------------------------------------------
void CFuiPage::SetState(U_CHAR st)
{ if (State == st)    return;
  State   = st;
  if (1 == State) MoWind->RegisterPopup(this);
  if (0 == State) MoWind->RegisterPopup(0);
  return;
}
//--------------------------------------------------------------------------
//  Swap the activity state
//--------------------------------------------------------------------------
void CFuiPage::SwapState()
{ U_CHAR nst = State ^ 1;
  SetState(nst);
  return;
}
//--------------------------------------------------------------------------
//  Draw the back Pane
//--------------------------------------------------------------------------
void CFuiPage::DrawPane()
{ ColorInside(cBackPane);
  DrawFastLine(surface,0,0,0,h,grey);
  FillRect(surface,0,h-2,w,h,grey);
  return;
}
//--------------------------------------------------------------------------
//  Add a line to popup panel
//--------------------------------------------------------------------------
void CFuiPage::AddPopupText(short nl,char *txt)
{ int indx = (nl == sLin)?(1):(0);
  int yd  = nl * hCar;
  int yf  = yd + hCar;
  FillRect(surface, 1, yd, w, yf,  cBack[indx]);
  fnts->DrawLimChar(surface,wCar,yd,0,cText[indx],txt);
  if (nl == sLin) MoWind->NotifyFromPopup(menu->Ident,(Tag)nl,EVENT_SELECTITEM);
  return;
}
//---------------------------------------------------------------------------------
//  Clear a line with back color
//---------------------------------------------------------------------------------
void  CFuiPage::ClearLine(short nl)
{ int yd  = nl * hCar;
  int yf  = yd + hCar;
  FillRect(surface,1, yd, w, yf,  cBack[0]);
  return;
}
//--------------------------------------------------------------------------
//  Focus is lost 
//--------------------------------------------------------------------------
void CFuiPage::FocusLost()
{ SetState(0);
  MoWind->NotifyFromPopup(menu->Ident,0,EVENT_FOCUSLOST);
  return;
}
//--------------------------------------------------------------------------
//  Mouse click inside
//--------------------------------------------------------------------------
bool CFuiPage::MouseClick (int mx, int my, EMouseButton button)
{ if  (0 == State)                                        return false;
  if  (!MouseHit (mx, my))                                return false;
  MoWind->ClearFocus(this);
  MoWind->NotifyFromPopup(menu->Ident,sLin,EVENT_POP_CLICK);
  return true;
}
//--------------------------------------------------------------------------
//  Mouse Move over
//  When mouse move over the line selected, the notification is sent to
//  the mother windows
//--------------------------------------------------------------------------
bool  CFuiPage::MouseMove(int mx, int my)
{ if  (!MouseHit (mx, my))        return false;
  int lin = (my - yParent - y) / hCar;
  if  (sLin == lin)               return true;
  if  (lin  >= menu->NbLin)       return true;
  short old = sLin;
  sLin      = lin;
  ClearLine(old);
  AddPopupText(old, (char*)menu->aText[old]);
  AddPopupText(lin, (char*)menu->aText[lin]);
  return true;
}
//===========================================================================================
//	CFuiRwyEXT:  Runway extension
//===========================================================================================
CFuiRwyEXT::CFuiRwyEXT()
{	Init();
  //-------------------------------------------------
	ptko	= 0;
	plnd	= 0;
	grh		= 0;
}
//------------------------------------------------------------------
//	Init Drawing parameters
//------------------------------------------------------------------
void CFuiRwyEXT::Init()
{	//-- init extremities for all runways  ---------------------
  lExt  = 0;                // Left   side
  rExt  = 0;                // Right  side
  uExt  = 0;                // Upper  side
  bExt  = 0;                // Bottom side
  mx    = 0;                // Point of view coordinate X
  my    = 0;                // (dito)        coordinate Y 
}
//-------------------------------------------------------------------------
//  Store extremities
//    Extremities are stored to compute the distance span of all runway ends
//    The point of view (POV) is the barycenter of all ends
//-------------------------------------------------------------------------
void CFuiRwyEXT::StoreExtremities(short dx, short dy)
{ if (dx < lExt)   lExt  = dx;
  if (dx > rExt)   rExt  = dx;
  if (dy < uExt)   uExt  = dy;
  if (dy > bExt)   bExt  = dy;
  mx  += dx;
  my  += dy;
  return;
}
//-------------------------------------------------------------------------
//  Return ruways span
//  -Runway span is the largest extend (in feet) in either direction
//   The end coordinates are first translated to the POV
//-------------------------------------------------------------------------
int CFuiRwyEXT::GetRWYspan()
{ int Nb  = rwyBOX.GetSize();
	if (0 == Nb)  return 0;
  //-------compute barycenter of all ends ---------------
  mx  = mx / (Nb << 1);
  my  = my / (Nb << 1);
  //------ Translate extentds to POV --------------------
  lExt  -= mx;
  rExt  -= mx;
  uExt  -= my;
  bExt  -= my;
  //----- return largest extend ------------------------
  int   dtx = (rExt < -lExt)?(rExt):(-lExt);
  int   dty = (bExt > -uExt)?(bExt):(-uExt);
  return (dtx > dty)?(dtx):(dty);
}
//----------------------------------------------------------------------------------
//  Compute drawing scale for canvas (nbr of pixels per feet)
//----------------------------------------------------------------------------------
void CFuiRwyEXT::ComputeScale()
{ int md  = 2 * GetRWYspan();
  int wd  = grh->Height();
  int ht  = grh->Width();
  int np  = (wd > ht)?(wd):(ht);
  wx      = (wd >> 1);
  wy      = (ht >> 1);
  scale   = 0;
  //-------Compute Scale ratio -----------------------------------
  if (0 == md)  return;
  scale   =  float(np * 0.8) / md;
  return;
}
//----------------------------------------------------------------------------------
//  Scale and translate all runway coordinates
//----------------------------------------------------------------------------------
void CFuiRwyEXT::ScaleAllRWY()
{ U_INT No	= 0;
	CRwyLine *slot = (CRwyLine*)rwyBOX.GetSlot(No++);
  while (slot)
  { slot->AdjustEnd(scale,mx,my);
    slot->ComputeCorner(wx,wy);
    slot  = (CRwyLine*)rwyBOX.GetSlot(No++);
  }
  return;
}
//----------------------------------------------------------------------------------
//    Note:  
//      To draw the runways, the following method is applied
//      The first runway end is taken as origin of real coordinates (in feet).
//      For all others runway ends, the distance to the origin is computed.
//      The point of view (POV) is computed as the barycenter of all ends
//      Then a translation to the POV and a scaling will be applied  to
//      all ends to give pixel coordinate. 
//      Runway corners (to have a 2D drawing) are computed using the normal
//      to the ruway direction.
//      Largest Extremities in all directions are stored to compute the 
//      distance span and then the scale ratio (feet to pixel).
//      A last translation occurs to position the POV in the mid point of
//      the window canva.
//
//----------------------------------------------------------------------------------
//	Return text to draw
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//  Draw all runway
//----------------------------------------------------------------------------------
void CFuiRwyEXT::DrawRunways()
{ //----------------------------------
	int x0; 
	int y0;
  int x1;
  int y1;
	//---------------------------------
	char tk	= 0;				// Take off detected
	char ld = 0;				// Land detected
	char eq = (strcmp(tkoID,lndID)==0)?(1):(0);
	//--- Take off coordinates --------
	tEND	= 0;
	//--- Landing coordinates ---------
	lEND  = 0;
	//---------------------------------
	U_INT No  = 0;
  U_INT sel = rwyBOX.GetSelectedNo();
  int   xc  = 0;
  CRwyLine *slot = 0;
  grh->EraseCanvas();
  for (No = 0; No != rwyBOX.GetSize();No++)
  { slot  = (CRwyLine*)rwyBOX.GetSlot(No);
		tk	 += slot->CheckEnd(tkoID,&tEND);		//&xt,&yt,0);
		ld	 += slot->CheckEnd(lndID,&lEND);		//&xl,&yl,&ilsFQ);
		//--- todo: save ILS data if landing runway is CRwyLine*)
    xc    = (No == sel)?(1):(0);
		if (ptko)	xc = 0;
    if (0 == slot->GetLeng()) continue;
    slot->GetEnd01(&x0,&y0);
    slot->GetEnd02(&x1,&y1);
    grh->DrawSegment(x0,y0,x1,y1,xc);
    x0  = x1;
    y0  = y1;
    slot->GetEnd03(&x1,&y1);
    grh->DrawSegment(x0,y0,x1,y1,xc);
    x0  = x1;
    y0  = y1;
    slot->GetEnd04(&x1,&y1);
    grh->DrawSegment(x0,y0,x1,y1,xc);
    x0  = x1;
    y0  = y1;
    slot->GetEnd01(&x1,&y1);
    grh->DrawSegment(x0,y0,x1,y1,xc);
  }
	//--- Edit ILS data -----------------------
	char *ilsd = (lEND)?(lEND->ilsD):("None");
	_snprintf(ilsTXT,20,"Landing ILS %s",ilsd);
	ilsFQ			 = (lEND)?(lEND->ifrq):(0);
	//--- Draw the label ----------------------
	if (eq && (tk) && (ld))
	{	grh->DrawNText(tEND->dx,tEND->dy,"T+L",1);	return;}
	if (tk)	grh->DrawNText(tEND->dx,tEND->dy,"Tko",1);
	if (ld)	grh->DrawNText(lEND->dx,lEND->dy,"Lnd",1);
  return;
}
//----------------------------------------------------------------------
//  Init runway ends popup from runway list
//----------------------------------------------------------------------
void CFuiRwyEXT::InitRunwayEnds()
{	//--- Slot 0 is for NONE entry -------------------
	cMENU[0]		= "NONE";
	pMENU[0]		= 0;
	//--- Scan list of runways ----------------------
	int				No	 = 0;
	int       k    = 1;
	CRwyLine *slot = (CRwyLine*)rwyBOX.GetSlot(No++);
 	//--- Note that first numbers are for HI ends ----
  while (slot)
  { char *hi	= slot->GetHiEndID();
		cMENU[k]	= hi;
		pMENU[k]	= slot;
		k++;
		char *lo  = slot->GetLoEndID();
		cMENU[k]	= lo;
		pMENU[k]	= slot;
		k++;
		slot  = (CRwyLine*)rwyBOX.GetSlot(No++);
  }
	cMENU[k]	= 0;
	//--- Create take off pop menu ------------------
	ptko->CreatePage(&men1,cMENU);
	ptko->SelectText(tkoID);
	//--- Create landing pop menu -------------------
	plnd->CreatePage(&men2,cMENU);
	plnd->SelectText(lndID);
	return;
}
//===========================================================================================
//
//  DLL fui window
//
//===========================================================================================
CFuiDLL::CFuiDLL (Tag idn, const char *filename)
:CFuiWindow(idn,filename)
{
  #ifdef _DEBUG_CFuiDLL	
	  FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_CFuiDLL.txt", "a")) == NULL)
	  {
      char buf1[128] = {0};
      TagToString (buf1, idn);
		  fprintf(fp_debug, "CFuiDLL::CFuiDLL %s\n", buf1);
		  fclose(fp_debug); 
	  }
  #endif
}
//--------------------------------------------------------------------------------
//  Free all resources
//--------------------------------------------------------------------------------
CFuiDLL::~CFuiDLL()
{
  #ifdef _DEBUG_CFuiDLL	
	  FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_CFuiDLL.txt", "a")) == NULL)
	  {
      char buf1[128] = {0};
      TagToString (buf1, windowId);
		  fprintf(fp_debug, "CFuiDLL::~CFuiDLL %s\n", buf1);
		  fclose(fp_debug); 
	  }
  #endif
}
//-----------------------------------------------------------------------
//  Notifications
//-----------------------------------------------------------------------
void CFuiDLL::EventNotify(Tag win,Tag cpn,EFuiEvents evn,EFuiEvents sub )
{ 
  #ifdef _DEBUG_CFuiDLL	
	  FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_CFuiDLL.txt", "a")) == NULL)
	  {
      char buf1[128] = {0};
      char buf2[128] = {0};
      char buf3[128] = {0};
      char buf4[128] = {0};
      TagToString (buf1, win);
      TagToString (buf2, cpn);
      TagToString (buf3, evn);
      TagToString (buf4, sub);
		  fprintf(fp_debug, "CFuiDLL::EventNotify 1=%s 2=%s 3=%s 4=%s\n",
        buf1, buf2, buf3, buf4);
		  fclose(fp_debug); 
	  }
  #endif

  //globals->plugins.On_EventNotice (0, win, cpn, evn, sub);
}

void CFuiDLL::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ 
  #ifdef _DEBUG_CFuiDLL	
	  FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_CFuiDLL.txt", "a")) == NULL)
	  {
      char buf1[128] = {0};
      char buf2[128] = {0};
      char buf3[128] = {0};
      char buf4[128] = {0};
      TagToString (buf1, windowId);
      TagToString (buf2, idm);
      TagToString (buf3, itm);
      TagToString (buf4, evn);
		  fprintf(fp_debug, "CFuiDLL::NotifyChildEvent 1=%s 2=%s 3=%s 4=%s\n",
        buf1, buf2, buf3, buf4);
		  fclose(fp_debug); 
	  }
  #endif

  globals->plugins.On_EventNotice (0, windowId, idm, evn, EVENT_NOSUBEVENT);
  return;
}

//=============================================================================
// Local helper functions
//

//
// Validate a tag value suspected to be a FUI component type
//
static bool ValidFuiComponentType (Tag tag)
{
  bool rc = false;
  switch (tag) {
  case COMPONENT_WINDOW:
  case COMPONENT_BUTTON:
  case COMPONENT_DOUBLE_BUTTON:
  case COMPONENT_DEFAULT_BUTTON:
  case COMPONENT_POPUPMENU:
  case COMPONENT_MENUBAR:
  case COMPONENT_WINDOW_MENUBAR:
  case COMPONENT_MENU:
  case COMPONENT_CHECKBOX:
  case COMPONENT_RADIOBUTTON:
  case COMPONENT_LABEL:
  case COMPONENT_LINE:
  case COMPONENT_BOX:
  case COMPONENT_PICTURE:
  case COMPONENT_SCROLLBAR:
  case COMPONENT_SLIDER:
  case COMPONENT_GROUPBOX:
  case COMPONENT_GAUGE:
  case COMPONENT_LIST:
  case COMPONENT_SCROLLAREA:
  case COMPONENT_TEXTFIELD:
  case COMPONENT_TEXTAREA:
  case COMPONENT_PROGRESS:
  case COMPONENT_MAP:
  case COMPONENT_TABPAGE:
  case COMPONENT_TABBUTTON:
  case COMPONENT_TABCONTROL:
  case COMPONENT_GRAPH:
  case COMPONENT_DLLVIEW:
  case COMPONENT_DISCLOSURE:
  case COMPONENT_CANVAS:
  case COMPONENT_RUNWAY_CANVAS:
  case COMPONENT_MARQUEE:
  case COMPONENT_HORIZ_MARQUEE:
  case COMPONENT_WINDOW_NORESIZE:
  case COMPONENT_TITLE:
  case COMPONENT_PALETTE_WINDOW:
  case COMPONENT_PALETTE_WINDOW_TITLE:
  case COMPONENT_CLOSE_BUTTON:
  case COMPONENT_MINIMIZE_BUTTON:
  case COMPONENT_ZOOM_BUTTON:
  case COMPONENT_RUNWAY_GRAPH:
    rc = true;
  }
  return rc;
}

//
// Helper function instantiates a FUI component given its component type
//
static CFuiComponent *CreateFuiComponent (EFuiComponentTypes type, CFuiComponent *wind)
{
  switch (type) {
  case COMPONENT_TITLE:
    return new CFuiWindowTitle(0,0,0,0,wind);

  case COMPONENT_CLOSE_BUTTON:
    return new CFuiCloseButton(0,0,0,0,wind);

  case COMPONENT_MINIMIZE_BUTTON:
    return new CFuiMinimizeButton(0,0,0,0,wind);

  case COMPONENT_ZOOM_BUTTON:
    return new CFuiZoomButton(0,0,0,0,wind);

  case COMPONENT_BUTTON:
    return new CFuiButton(0,0,0,0,wind);

  case COMPONENT_LABEL:
    return new CFuiLabel(0,0,0,0,wind);

  case COMPONENT_TEXTFIELD:
    return new CFuiTextField(0,0,0,0,wind);

  case COMPONENT_PICTURE:
    return new CFuiPicture;

  case COMPONENT_CHECKBOX:
    return new CFuiCheckbox(0,0,0,0,wind);

  case COMPONENT_RADIOBUTTON:
    return new CFuiCheckBox;

  case COMPONENT_LINE:
    return new CFuiLine;

  case COMPONENT_BOX:
    return new CFuiBox(0,0,0,0,wind);

  case COMPONENT_GROUPBOX:
    return new CFuiGroupBox(0,0,0,0,wind);

  case COMPONENT_SLIDER:
    return new CFuiSlider(0,0,0,0,wind);
    break;

  case COMPONENT_LIST:
    return new CFuiList(0,0,0,0,wind);

  case COMPONENT_POPUPMENU:
    return new CFuiPopupMenu(0,0,0,0,wind);

  case COMPONENT_GRAPH:
    return new CFuiGraph;

  case COMPONENT_SCROLLBAR:
//    return new CFuiScrollbar;
    break;

  case COMPONENT_MENUBAR:
    return new CFuiMenuBar(0);

  case COMPONENT_WINDOW_MENUBAR:
    return new CFuiWindowMenuBar;

  case COMPONENT_MENU:
    return new CFuiMenu;

  case COMPONENT_TEXTAREA:
    //return new CFuiTextField (0,0,0,0,wind); // 
    break;

  case COMPONENT_PROGRESS:
    break;

  case COMPONENT_MAP:
//    return new CFuiMap;
		break;

  case COMPONENT_RUNWAY_GRAPH:
    return new CFuiCanva(0,0,0,0,wind);

  default:
    WARNINGLOG ("Unknown FUI Component : %d", type);
  }

  return NULL;
}

//================END OF FILE ====================================================
