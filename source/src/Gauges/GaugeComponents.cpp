/*
 * New Gauges
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005 Chris Wallace
  * Copyright 2007 Jean Sabatier
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
#include "../Include/WorldObjects.h"
#include "../Include/TerrainTexture.h"
#include "../Include/CursorManager.h"
#include "../Include/GaugeComponents.h"
//====================================================================
// GripGauge
//=====================================================================
CGripGauge::CGripGauge(CPanel *mp)
:CGauge(mp)
{ Prop.Set(NO_SURFACE);
	panel = mp;
	grip = 0;
}
//-------------------------------------------------------------------
//	Knob click => Arm rotation
//-------------------------------------------------------------------
EClickResult CGripGauge::MouseClick (int x, int y, int buttons)
{	grip  = 1;
  panel->MouseScreen(px,py);
  return MOUSE_TRACKING_ON;
}
//----------------------------------------------------------------
//	Stop click => disarm rotation
//-----------------------------------------------------------------
EClickResult CGripGauge::StopClick()
{	grip  = 0;
  return MOUSE_TRACKING_OFF;
}
//-------------------------------------------------------------------
//	Mouse move over
//-------------------------------------------------------------------
EClickResult CGripGauge::TrackClick (int x, int y, int bDown)
{	if (0 == grip)  return MOUSE_TRACKING_OFF;
  int mx,my;
  panel->MouseScreen(mx,my);
  int dx = px - mx;
  int dy = py - my;
  panel->ScrollPanel(dx,dy);
  px  = mx;
  py  = my;
  return MOUSE_TRACKING_ON;
}
//==========================================================================
//  CGauge Action
//==========================================================================
CGaugeAction::CGaugeAction()
{ }
//--------------------------------------------------------------
//  Destroy resources
//--------------------------------------------------------------
CGaugeAction::~CGaugeAction()
{ std::vector<BASE_ACT*>::iterator ita;
  for (ita=vact.begin(); ita != vact.end(); ita++)  delete (*ita);
  vact.clear();
}
//--------------------------------------------------------------
//  Read action frame
//---------------------------------------------------------------
bool CGaugeAction::DecodeFrame(char *txt, BASE_ACT *a)
{ int nf = sscanf(txt,"frame = %d",&a->pm1);
  if (1 != nf)  return false;
  a->Type = ACT_FRAME;
  return true;
}
//--------------------------------------------------------------
//  Read action message
//---------------------------------------------------------------
bool CGaugeAction::DecodeMessage(char *txt, BASE_ACT *a)
{ int nf = sscanf(txt,"MS%d = %f",&a->pm1, &a->Value);
  if (2 != nf)  return false;
  a->Type = ACT_MSAGE;
  return true;
}
//--------------------------------------------------------------
//  Read list of actions
//---------------------------------------------------------------
void CGaugeAction::ReadActions(SStream *str)
{ char txt[128];
  int na = 0;
  ReadInt(&na,str);
  while (na--)
  { ReadString(txt,128,str);
    BASE_ACT *a = new BASE_ACT();
    vact.push_back(a);
    if (DecodeFrame  (txt,a))   continue;
    if (DecodeMessage(txt,a))   continue;
    gtfo("Bad Gauge action");
  }
  return;
}
//====================================================================
// Part managing a vertical strip of frames
//  This part inherit the mother gauge quad unless it is locally defined
//=====================================================================
VStrip::VStrip()
{ Init(0);
}
//--------------------------------------------------------------
VStrip::VStrip(CGauge *mg)
{ Init(mg);
}
//----------------------------------------------------------------
//  Initialize Part
//----------------------------------------------------------------
void VStrip::Init(CGauge *mg)
{ mgg     = mg;
  vtab    = 0;
  quad    = 0;
	vOfs		= 0;															// OK
  return;
}
//----------------------------------------------------------------
//  Destructor
//----------------------------------------------------------------
VStrip::~VStrip()
{ if (vtab) delete [] vtab;
}
//----------------------------------------------------------------
//  Init quad to the requested dimension
//----------------------------------------------------------------
void VStrip::LocalQuad(int xp, int yp, int wd, int ht)
{ if (!mgg)      return;
  vtab   = new TC_VTAB[4];
	vOfs	 = mgg->FixRoom(4);									// OK
  int sx = mgg->GetXPos() + xp;             // x position in panel
  int sy = mgg->GetYPos() + yp;             // y position in panel
  mgg->BuildQuad(vtab,sx,sy,wd,ht);         // Init texture quad
  quad   = vtab;
  return;
}

//----------------------------------------------------------------
//  Read a strip of vertical texture
//  By default, the quad is copied from the mother gauge
//----------------------------------------------------------------
void VStrip::ReadStrip(SStream *s)
{ CStreamObject::ReadStrip(s,txd);
  hiFrame = txd.nf - 1;
  if (quad)   return;
  //---- Allocate a quad and dupplicate the mother gauge --
  if (0 == mgg) gtfo("VStrip need mother gauge");
  quad  = mgg->GetVTAB();
  vOfs	= mgg->ofsVBO();										// OK    
  return;
}
//----------------------------------------------------------------
//  Init quad to the requested dimension
//----------------------------------------------------------------
void VStrip::ReadPROJ(SStream *str)
{ if (0 == mgg) gtfo("VStrip need mother gauge");
  vtab  = new TC_VTAB[4];
	vOfs  = mgg->FixRoom(4);									// OK
  mgg->DecodePROJ(str,vtab,0);
  quad  = vtab;
  return;
}
//----------------------------------------------------------------
//  Init quad to the requested dimension
//----------------------------------------------------------------
void VStrip::ReadPOSN(SStream *str)
{ if (0 == mgg) gtfo("VStrip need mother gauge");
  vtab  = new TC_VTAB[4];
	vOfs  = mgg->FixRoom(4);								// OK
  mgg->DecodePOSN(str,vtab,1);
  quad  = vtab;
  return;
}
//----------------------------------------------------------------
//  Read all parameters
//----------------------------------------------------------------
int VStrip::Read(SStream *str,Tag tag)
{ char txt[16];
  switch (tag)  {
    //---- File name ----------------------
    case 'file':
    case 'bmap':
      ReadStrip(str);
      return TAG_READ;
    //---- POSN position ------------------
    case 'posn':
      ReadPOSN(str);
      return TAG_READ;
    //--- PROJ projector ------------------
    case 'proj':
      ReadPROJ(str);
      return TAG_READ;
  }
  TagToString(txt,tag);
  WARNINGLOG("VStrip: <%s> tag ignored");
  return TAG_IGNORED;
}
//--------------------------------------------------------------
//  Collect VBO data 
//---------------------------------------------------------------
void VStrip::CollectVBO(TC_VTAB *vtb)
{	if (0 == vOfs)	return;									
	TC_VTAB *dst = vtb + vOfs;
	for (int k=0; k<4; k++) *dst++ = quad[k];
	return;
}
//--------------------------------------------------------------
//  Copy from a similar gauge
//---------------------------------------------------------------
void VStrip::CopyFrom(CGauge *mg,VStrip &src)
{ if (0 == src.vOfs)	return;
	mgg   = mg;                     // Mother gauge
  quad  = mg->GetVTAB();					// Use mother gauge
	vOfs  = mgg->FixRoom(4);				// Reserve in panel VBO
  //--- dupplicate texture items ------------------------
  txd.Dupplicate(src.txd);
  hiFrame = src.hiFrame;
  return;
}
//--------------------------------------------------------------
//  Store quad in dynamic area
//---------------------------------------------------------------
void	VStrip::DynaQuad(TC_VTAB *vbo,U_SHORT ofs)
{ vOfs         = ofs;
  TC_VTAB *dst = vbo + ofs;
	for (int k=0; k<4; k++)	dst[k] = quad[k];
	quad				 = dst;
	return;
}
//--------------------------------------------------------------
//  Draw a frame
//---------------------------------------------------------------
bool VStrip::Draw(int fr)
{ if (0 == vOfs)    return false;
  int No  = txd.nf - 1 - fr;
  No      = txd.Clamp(No);
  U_INT *base = (U_INT*)txd.rgba + (No * txd.dm);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,txd.wd,txd.ht,0,GL_RGBA,GL_UNSIGNED_BYTE,base);
	glDrawArrays(GL_TRIANGLE_STRIP,vOfs,4);
  return true;
}
//--------------------------------------------------------------
//  Draw a frame
//---------------------------------------------------------------
void VStrip::Draw(TEXT_DEFN *tdf,int fr)
{ if (0 == tdf->rgba)     return;
  if (0 == quad)          return;
  int No  = tdf->nf - 1 - fr; 
  U_INT *base = (U_INT*)tdf->rgba + (No * tdf->dm);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,tdf->wd,tdf->ht,0,GL_RGBA,GL_UNSIGNED_BYTE,base);
	glDrawArrays(GL_TRIANGLE_STRIP,vOfs,4);
  return;
}
//--------------------------------------------------------------
//  Draw a range in texture
//	NOTE:  Correct image is diplayed one frame later
//				 because dynamic VBO is updated at cycle end
//---------------------------------------------------------------
void VStrip::DrawRange(float p0, float p1)
{ if (0 == txd.wd)    return;
  float s0 = p0 / txd.wd;
  float s1 = p1 / txd.wd;
	quad[Q_NW].VT_S = s0;              // NW corner
	quad[Q_SW].VT_S = s0;              // SW corner
  quad[Q_NE].VT_S = s1;              // NE corner
  quad[Q_SE].VT_S = s1;              // SE corner
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,txd.wd,txd.ht,0,GL_RGBA,GL_UNSIGNED_BYTE,txd.rgba);
	glDrawArrays(GL_TRIANGLE_STRIP,vOfs,4);
  return;
}

//====================================================================
// CRotNeedle
//=====================================================================
CRotNeedle::CRotNeedle (void)
{ offx  = offy = 0;
  mind  = 0;
  maxd  = 360;
  quad  = 0;
  incd  = 0;
  vadj  = 0;
	vOfs	= 0;
  //---Init pitch translation ----
  ptrs.x  = 0.5;
  ptrs.y  = 0.5;
  ptrs.z  = 0;
  //--- Init rotation center -----
  rotc.x  = 0.5;
  rotc.y  = 0.5;
  rotc.z  = 0;
	//------------------------------
}
//--------------------------------------------------------------
//  Initialize needle texture
//---------------------------------------------------------------
void CRotNeedle::GetTexture(SStream *str)
{ ReadBMAP(str,txd);
	return;
}

//--------------------------------------------------------------
//  Set pixel per degre
//  1)  We have ppd = pixel per degre.
//      1/ppd = degre per pixel
//      Thus for a texture with ht pixels we can represent
//      amp = ht * 1/ppd degre of amplitude
//      The texture coefficient inc = 1 / amp per degre
//      inc = ppd * 1/ht = ppd  / ht
// 
//---------------------------------------------------------------
void CRotNeedle::SetPPD(double ppd)
{ if (txd.ht == 0)  return;
  ampl  = (txd.ht / ppd) * 0.5;
  incd  = (1 / ampl);           // texture increment per degre
  return;
}
//--------------------------------------------------------------
//  set Quad from gauge
//---------------------------------------------------------------
void CRotNeedle::SetQuad(CGauge *mg)
{	quad	= mg->GetVTAB();
	vOfs	= mg->ofsVBO();
	return;
}
//--------------------------------------------------------------
//  Adjust the texture rotation center
//  Texture wd and ht must be present
//---------------------------------------------------------------
void CRotNeedle::SetCenter(double dx, double dy)
{ if (0 == quad)      gtfo("<rotc> Gauge without dimension");
  rotc.x  = double(dx) / double(txd.wd);
  rotc.y  = double(1) - (double(dy) / double(txd.ht));
  return;
}
//--------------------------------------------------------------
//  Free the resources
//  Note: This component does not onw the quad and 
//        should not delete it
//---------------------------------------------------------------

CRotNeedle::~CRotNeedle (void)
{ }
//--------------------------------------------------------------
//	Check completude
//--------------------------------------------------------------
void CRotNeedle::ReadFinished()
{	if (0 == txd.rgba)	vOfs = 0;
	return;	}
//--------------------------------------------------------------
//  Copy from a similar component
//  NOTE:  This component share the quad pointer and should not
//         delete it
//---------------------------------------------------------------
void  CRotNeedle::CopyFrom(CGauge *mg,CRotNeedle &src)
{ if (0 == src.vOfs)	return;
	quad    = mg->GetVTAB();
  vOfs    = mg->FixRoom(4);		// Reserve in panel VBO
  txd.Dupplicate(src.txd);
  //--- copy others parameters ---------------
  offx  = src.offx;
  offy  = src.offy;
  mind  = src.mind;
  maxd  = src.maxd;
  vadj  = src.vadj;
  ampl  = src.ampl;
  incd  = src.incd;
  ptrs  = src.ptrs;
  rotc  = src.rotc;
  return;
}
//--------------------------------------------------------------
//	Collect VBO data
//--------------------------------------------------------------
void CRotNeedle::CollectVBO(TC_VTAB *vtb)
{	if (0 == vOfs)	return;
	TC_VTAB *dst = vtb + vOfs;
	for (int k=0; k<4; k++)	*dst++ = quad[k];
	return;
}
//------------------------------------------------------------------------
//  Draw the needle in [0,360°]
//  The OpenGL context must be:
//  Perspective is Ortho
//  The gauge texture object must be binded
//  Blend mode must be on.
//-------------------------------------------------------------------------
void CRotNeedle::Draw (float deg)
{ if (0 == vOfs)			return;
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,txd.wd,txd.ht,0,GL_RGBA,GL_UNSIGNED_BYTE,txd.rgba);
  //--- Clamp to the min/max degrees ---------------------------
  if (deg < mind) deg = mind;
  if (deg > maxd) deg = maxd;
  //--- Rotate the texture  ------------------------------------
  glMatrixMode(GL_TEXTURE);
  glPushMatrix();
  glTranslated(+rotc.x,+rotc.y,0);        // Go to center
  glRotated(deg,0,0,1);                   // Rotate texture
  glTranslated(-rotc.x,-rotc.y,0);        // Back to origin
	glDrawArrays(GL_TRIANGLE_STRIP,vOfs,4);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  return;
}
//------------------------------------------------------------------------
//  Draw as a fixed image
//------------------------------------------------------------------------
void CRotNeedle::DrawFixe()
{ if (0 == vOfs)			return;
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,txd.wd,txd.ht,0,GL_RGBA,GL_UNSIGNED_BYTE,txd.rgba);
	glDrawArrays(GL_TRIANGLE_STRIP,vOfs,4);
  return;
}
//------------------------------------------------------------------------
//  Draw as a rotated and translated texture
//  -Roll will rotatte the texture
//  -Pitch will translate the rotated picture verticaly
//------------------------------------------------------------------------
void CRotNeedle::Draw (float rol, float pit)
{	if (0 == vOfs)			return;
	if (pit > +ampl) pit = +ampl;
  if (pit < -ampl) pit = -ampl;
  double off = (pit * incd);
  double bak = -ptrs.y - off;
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,txd.wd,txd.ht,0,GL_RGBA,GL_UNSIGNED_BYTE,txd.rgba);
  //--- Clamp to the min/max degrees ---------------------------
  if (rol < mind) rol = mind;
  if (rol > maxd) rol = maxd;
  //--- Rotate the texture  ------------------------------------
  glMatrixMode(GL_TEXTURE);
  glPushMatrix();
  glTranslated(+ptrs.x,ptrs.y,0);
  glRotated(rol,0,0,1);
  glTranslated(-ptrs.x,bak,0);
	glDrawArrays(GL_TRIANGLE_STRIP,vOfs,4);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  return;
}

//===========================================================================
//
// C_Cover for overlay and underlay
//
//===========================================================================
C_Cover::C_Cover()
{ mgg   = 0;
  vtab  = 0;
  quad  = 0;
	vOfs	= 0;
}
//----------------------------------------------------------------------
//  Destroy
//----------------------------------------------------------------------
C_Cover::~C_Cover() 
{ if (vtab) delete vtab;
	txd.Free();	}
//----------------------------------------------------------------------
//  Set mother Gauge
//----------------------------------------------------------------------
void C_Cover::SetGauge(CGauge *mg)
{ mgg   = mg; 
  quad  = mg->GetVTAB();
	vOfs  = mg->ofsVBO();
  return;
}

//-----------------------------------------------------------
//  Init cover from mother gauge
//-----------------------------------------------------------
void C_Cover::Init(SStream *str,CGauge *mg)
{ mgg   = mg;
  quad  = mgg->GetVTAB();
	vOfs  = mgg->ofsVBO();
  ReadBMAP(str,txd);
  return;
}
//-----------------------------------------------------------
//  Read texture position
//-----------------------------------------------------------
void C_Cover::ReadSize(SStream *str)
{ if (0 == mgg)  gtfo("Mother Gauge needed");
  int px = 0;
  int py = 0;
  int wd = 0;
  int ht = 0;
  ReadSIZE(str,&px,&py,&wd,&ht);
  px += mgg->GetXPos();
  py += mgg->GetYPos();
  //---- Build the needle quad ------------------------
  vtab  = new TC_VTAB[4];
	vOfs  = mgg->FixRoom(4);
  mgg->BuildQuad(vtab,px,py,wd,ht);
  quad  = vtab;
}
//----------------------------------------------------------------------
//  Read parameters of  <covr>
//	NOTE: All local definition result in using a local quad, not
//				the mother gauge quad
//----------------------------------------------------------------------
int C_Cover::Read(SStream *str, Tag tag)
{ switch (tag)  {
    //--- texture file ----------------
    case 'bmap':
    case 'file':
      ReadBMAP(str,txd);
      return TAG_READ;
    //--- Texture position ------------
    case 'size':
      ReadSize(str);
      return TAG_READ;
    //--- texture position ------------
    case 'cadr':
			vtab  = new TC_VTAB[4];
			vOfs  = mgg->FixRoom(4);
			quad  = vtab;
			ReadCADR(str,mgg,quad);
      return TAG_READ;
  }
  return TAG_IGNORED;
}
//----------------------------------------------------------------------
//  Dupplicate from a similar component
//----------------------------------------------------------------------
void C_Cover::CopyFrom(CGauge *mg,C_Cover &src)
{ if (0 == src.vOfs)	return;
	mgg   = mg;
  txd.Dupplicate(src.txd);
  quad  = mg->GetVTAB();
	vOfs  = mg->FixRoom(4);	// Reserve in panel VBO
  return;
}
//----------------------------------------------------------------------
//  Collect VBO data
//----------------------------------------------------------------------
void C_Cover::CollectVBO(TC_VTAB *vtb)
{ if (0 == vOfs)		return;
	TC_VTAB *dst = vtb + vOfs;
	for (int k=0; k<4; k++) *dst++ = quad[k];
	return;
}
//--------------------------------------------------------------
//  Store quad in dynamic area
//---------------------------------------------------------------
void	C_Cover::DynaQuad(TC_VTAB *vbo,U_SHORT ofs)
{ if (0 == quad)	return;
	vOfs         = ofs;
  TC_VTAB *dst = vbo + ofs;
	for (int k=0; k<4; k++)	dst[k] = quad[k];
	quad				 = dst;
	return;
}
//----------------------------------------------------------------------
//  Draw the cover
//----------------------------------------------------------------------
void C_Cover::Draw()
{ if (0 == vOfs)      return;
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,txd.wd,txd.ht,0,GL_RGBA,GL_UNSIGNED_BYTE,txd.rgba);
	glDrawArrays(GL_TRIANGLE_STRIP,vOfs,4);
  return;
}
//==========================================================================
//
//  CKnob class with shared surface
//
//==========================================================================
C_Knob::C_Knob()
{ Init(0);
}
//---------------------------------------------------------------------
//  Constructor for knob with owned surface
//---------------------------------------------------------------------
C_Knob::C_Knob(CGauge *mg)
{ Init(mg);
}
//---------------------------------------------------------------------
//  Init knob area
//---------------------------------------------------------------------
void C_Knob::Init(CGauge *g)
{ mgg       = g;
  CType     = PN_DEGRE;
  CState    = 2; 
  help[0]   = 0;
  vRot      = 0;
  fixe      = 0;
	vOfs			= 0;
  //--- Assign mother gauge dimension --------
  x1 = x2 = y1 = y2 = 0;
  cTag    = 0;
  Change  = 0;
	return;
}
//---------------------------------------------------------------------
//  Free the resources
//---------------------------------------------------------------------
C_Knob::~C_Knob()
{ 
}
//---------------------------------------------------------------------
//  SetMother Gauge
//---------------------------------------------------------------------
void C_Knob::SetGauge(CGauge *mg)
{	mgg		= mg;	
	return;
}
//---------------------------------------------------------------------
//  Read the knob position
//---------------------------------------------------------------------
void  C_Knob::ReadPOSN(SStream *str)
{ if (0 == mgg) gtfo("Knob: need mother gauge");
  char txt[128];
  ReadString(txt,128,str);
  int px,py;
  int wx,hy;
  int nf = sscanf(txt,"%d , %d , %d , %d",&px,&py,&wx,&hy);
  if (4 != nf) gtfo("Invalide <posn>");
  x1  = short(px);
  y1  = short(py);
  wd  = short(wx);
  ht  = short(hy);
  InitQuad();
  return;
}
//---------------------------------------------------------------------
//  Read the projector
//---------------------------------------------------------------------
void C_Knob::ReadPROJ(SStream *str)
{ if (0 == mgg) gtfo("Knob: need mother gauge");
  mgg->DecodePROJ(str,quad,1);
  //--- compute top left corner ------------------
  x1  = GetLefPos(quad);
  y1  = GetTopPos(quad);
  wd  = GetWIDTH(quad);
  ht  = GetHEIGHT(quad);
  x2  = x1 + wd;
  y2  = y1 + ht;
	vOfs	= mgg->FixRoom(4);
  rotn.SetVTAB(quad);
	rotn.SetVBO(vOfs);
  return;
}
//---------------------------------------------------------------------
//  Read parameters (Read from)
//---------------------------------------------------------------------
int C_Knob::Read(SStream *stream, Tag tag)
{ switch (tag) {
    case 'fixe':
        fixe = 1;
        return TAG_READ;
    case 'file':
		case 'bmap':
        rotn.GetTexture(stream);
        return TAG_READ;

    case 'rect':
    case 'size':
      { ReadSIZE(stream,&x1,&y1,&wd,&ht);
        InitQuad();
        return TAG_READ;  }

    case 'posn':
        ReadPOSN(stream);
        return TAG_READ;

    case 'proj':
        ReadPROJ(stream);
        return TAG_READ;

    case 'curs':
      { char cart[64] = {0};
        ReadString (cart, 64, stream);
	      if (cart[0] == 0)	TAG_READ;
        cTag = globals->cum->BindCursor (cart);
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
void C_Knob::ReadFinished(void)
{	rotn.ReadFinished();
	return;
}
//---------------------------------------------------------------------
//  Init quad from mother gauge
//---------------------------------------------------------------------
void C_Knob::InitQuad()
{ if (0 == mgg) gtfo("Knob: need mother gauge");
	if (vOfs)			return;
  //--- Compute rectangle ---------------------------
  x2  = x1 + wd;
  y2  = y1 + ht;
  //--- Init quad -----------------------------------
  int sx = mgg->GetXPos() + x1;       // x position in panel
  int sy = mgg->GetYPos() + y1;       // y position in panel
  mgg->BuildQuad(quad,sx,sy,wd,ht);    // Init knob quad
	vOfs	 = mgg->FixRoom(4);
  rotn.SetVTAB(quad);
  rotn.SetVBO(vOfs);
  return;
}
//---------------------------------------------------------------------
//  Collect VBO data
//---------------------------------------------------------------------
void C_Knob::CollectVBO(TC_VTAB *vtb)
{	return rotn.CollectVBO(vtb);	}
//-----------------------------------------------------------------------
//  Arm floating value rotation
//-----------------------------------------------------------------------
EClickResult C_Knob::ArmRotation(float adj,int x,int y,int btn)
{ if (!mgg)        return MOUSE_TRACKING_OFF;
  bool hit = ((x >= x1) && (x <= x2) && (y >= y1) && (y <= y2));
  if (!hit)        return MOUSE_TRACKING_OFF;
  CState	= 1;								  // Active state
	maxTM	  = 0.5f;								// Slow rate
	Timer	  = maxTM;							// Fisrt tick
	Incr	  = 0;
  if (btn & MOUSE_BUTTON_LEFT )	{Incr	= -adj; iRot = -4;}
  if (btn & MOUSE_BUTTON_RIGHT)	{Incr	= +adj; iRot = +4;}
  Change  = 0;
	return MOUSE_TRACKING_ON;
}
//-----------------------------------------------------------------------
//  Check for a change
//-----------------------------------------------------------------------
bool C_Knob::HasChanged()
{ if (CState == 2) {CState = 0; return true; }    // First time
  if (CState == 0)				      return false;		  // No change
  float dT	= globals->dST;
	Timer	+= dT;
	if (Timer < maxTM)				    return false;			// No change yet
  Timer	 = 0;
	if (maxTM > 0.05)	maxTM -= 0.05f;						    // Accelerate
  Change  = Incr;
  vRot    = Wrap360(vRot + iRot);
  return true;
}
//-----------------------------------------------------------------------
// Redraw rotating Knob at current frame
//-----------------------------------------------------------------------
void C_Knob::Draw()
{	if (!fixe)  rotn.Draw(vRot);
  else        rotn.DrawFixe();
  return;
}
//--------------------------------------------------------------
//  Check if the mouse is in the area
//--------------------------------------------------------------
ECursorResult C_Knob::MouseMoved (int x, int y)
{ // Check that the position is within click area bounds
  if (!mgg) return CURSOR_NOT_CHANGED; 
  if (IsHit (x, y)) return  globals->cum->SetCursor(cTag);
  return CURSOR_NOT_CHANGED;
}
//===========================================================================
// CNeedleGauge
//  This gauge has only one needle
//===========================================================================
CNeedle::CNeedle (CGauge *mg)
{ Init(mg);
  rotn.SetQuad(mg);
}
//----------------------------------------------------------------------
//  Embbeded constructor
//----------------------------------------------------------------------
CNeedle::CNeedle ()
{ Init(0);
}
//----------------------------------------------------------------------
//  Set mother gauge
//  By default, the needle inherit of the mother gauge QUAD
//----------------------------------------------------------------------
void CNeedle::SetGauge(CGauge *mg)
{ mgg = mg;
  rotn.SetQuad(mg);
}
//----------------------------------------------------------------------
//  Init parameters
//----------------------------------------------------------------------
void CNeedle::Init(CGauge *mg)
{ mgg   = mg;
  cTag  = 0;
  vtab  = 0;
  ampv  = 0;
  amph  = 0;
  sang  = 0.0f;
  dunt  = 1.0f;
  gmin  = 0;
  gmax  = 1000000;
  gmap  = 0;
	vOfs	= 0;
 *help  = 0;
  return;
}
//----------------------------------------------------------------------
//  If the position is specified, the needle uses its own Quad
//  whatever the position is
//  NOTE:  The coordinates are relative to the top left corner of the Gauge
//----------------------------------------------------------------------
void  CNeedle::ReadPOSN(SStream *str)
{ if (0 == mgg)  gtfo("<posn>: Mother Gauge needed");
  //--- Allocate the needle quad --------------------------
  vtab = new TC_VTAB[4];
	vOfs = mgg->FixRoom(4);
  mgg->DecodePOSN(str,vtab,1);
  //---- Set the needle quad ------------------------
  rotn.SetVTAB(vtab);
	rotn.SetVBO(vOfs);
  return;
  }
//----------------------------------------------------------------------
//  If the projection is specified, the needle uses its own Quad
//  whatever the position is
//  NOTE:  All coordinates are relative to the top left corner of the gauge
//----------------------------------------------------------------------
void CNeedle::ReadPROJ(SStream *str)
{ if (0 == mgg)  gtfo("<proj>: Mother Gauge needed");
  vtab = new TC_VTAB[4];
	vOfs = mgg->FixRoom(4);
  //--- Decode projector corners -----------
  mgg->DecodePROJ(str,vtab,1);
  //---- Set the needle quad ---------------
  rotn.SetVTAB(vtab);
	rotn.SetVBO(vOfs);
  return;
}
//----------------------------------------------------------------------
//  Decode rotation center and dimension
//  <cntr> -------
//  x,y,r
//----------------------------------------------------------------------
void CNeedle::ReadNCTR(SStream *str)
{ if (0 == mgg)  gtfo("<nctrj>: Mother Gauge needed");
  vtab  = new TC_VTAB[4];
	vOfs = mgg->FixRoom(4);
  mgg->DecodeNCTR(str,vtab,1);
  //---- set the needle quad --------------
  rotn.SetVTAB(vtab);
	rotn.SetVBO(vOfs);
  return;
}
//----------------------------------------------------------------------
//  Decode rotation center
//----------------------------------------------------------------------
void CNeedle::ReadROTC(SStream *str)
{ char txt[128];
  float  x,y;
  ReadString(txt,128,str);
  if (2 != sscanf(txt," %f , %f \t",&x,&y))  gtfo("Invalid <rotc> tag");
  rotn.SetCenter(x,y);
  return;
}
//----------------------------------------------------------------------
//  Decode bitmap
//----------------------------------------------------------------------
void CNeedle::ReadBMAP(SStream *str)
{ rotn.GetTexture(str);
  return;
}
//----------------------------------------------------------------------
//  read parameters
// TODO check why there is a tag <rect>
//----------------------------------------------------------------------
int CNeedle::Read (SStream *stream, Tag tag)
{ int pm;
  double dm;
  char name[128];
  switch (tag) {
    //--- Vertical amplitude-----------
    case 'ampv':
      ReadFloat(&ampv, stream);
      return TAG_READ;
    //--- Horizontal amplitude----------
    case 'amph':
      ReadFloat (&amph, stream);
      return TAG_READ;
    //--- Starting angle --------------
    case 'sang':
      ReadFloat (&sang, stream);
      return TAG_READ;
    //--- ???? -----------------------
    case 'dunt':
      ReadFloat (&dunt, stream);
      return TAG_READ;
    //--- Range ----------------------
    case 'clmp':
      ReadFloat(&gmin, stream);
      ReadFloat(&gmax, stream);
      return TAG_READ;
    //--- Offset ?? ------------------
    case 'noff':
      ReadInt (&pm, stream);
      ReadInt (&pm, stream);
      return TAG_READ;
    //--- Vertical adjustment ---------
    case 'vadj':
      ReadDouble(&dm, stream);
      rotn.SetVADJ(dm);
      return TAG_READ;
    //--- No what ??? ----------------
    case 'noac':
      return TAG_READ;
    //--- needle position in gauge ---
    case 'posn':
      ReadPOSN(stream);
      return TAG_READ;
    //--- needle projector in gauge ---
    case 'proj':
      ReadPROJ(stream);
      return TAG_READ;

    //--- Rotation center ------------
    case 'rotc':
      ReadROTC(stream);
      return TAG_READ;
    //--- Needle center ------------
    case 'nctr':
      ReadNCTR(stream);
      return TAG_READ;
      
    //--- Texture file ---------------
    case 'file':
    case 'bmap':
			rotn.GetTexture(stream);
      return TAG_READ;
    //----Table look up -----------------
    case 'gtbl':
    { CDataSearch map(stream);
      gmap = map.GetTable();
      gmap->IncUse();
      return TAG_READ;
    }
    //---- Message for update -----------
    case 'mesg':
      ReadMessage (&mesg, stream);
      return TAG_READ;
    //--- Cursor -----------------------
    case 'curs':
    case 'crsr':
      cTag = mgg->BindCursor(stream);
      return TAG_READ;
    //--- Help message ---------------
    case 'help':
      ReadString (help, 64, stream);
      return TAG_READ;
		//--- Rectangle ----------------
		case 'rect':
			break;

  }
  TagToString(name,tag);
  WARNINGLOG("CNeedle: %s Tag ignored",name);
  return TAG_IGNORED;
}
//----------------------------------------------------------------------
//  All parameters are read
//----------------------------------------------------------------------
void CNeedle::ReadFinished()
{  rotn.ReadFinished();
}
//----------------------------------------------------------------------
//  Destroy needle
//----------------------------------------------------------------------
CNeedle::~CNeedle()
{ if (vtab) delete [] vtab;
  if (gmap) gmap->DecUse();
}
//----------------------------------------------------------------------
//  Dupplicate from similar gauge
//----------------------------------------------------------------------
void  CNeedle::CopyFrom(CGauge *mg,CNeedle &src)
{ mgg   = mg;
  cTag  = src.cTag;
  mesg  = src.mesg;
  ampv  = src.ampv;
  amph  = src.amph;
  sang  = src.sang;
  dunt  = src.dunt;
  gmin  = src.gmin;
  gmax  = src.gmax;
  gmap  = src.gmap;
  if (gmap) gmap->IncUse();
  rotn.CopyFrom(mg,src.rotn);
  strncmp(help,src.help,64);
  return;
}
//----------------------------------------------------------------------
//  Collect VBO data
//----------------------------------------------------------------------
void CNeedle::CollectVBO(TC_VTAB *vtb)
{		return rotn.CollectVBO(vtb); }
//----------------------------------------------------------------------
//  Clamp value in interval
//----------------------------------------------------------------------
int CNeedle::Clamp(float v)
{ if (v < gmin) return gmin;
  if (v > gmax) return gmax;
  return v;
}

//----------------------------------------------------------------------
//  Draw only the needle
//----------------------------------------------------------------------
void CNeedle::DrawNeedle(float deg)
{ float ang = Wrap360(deg + sang);
  rotn.Draw(ang);
  return;
}
//----------------------------------------------------------------------
//  Draw the gauge
//  -Update with message and draw needle
//----------------------------------------------------------------------
void CNeedle::Draw (void)
{ //--- get value from message ------------------
  Send_Message(&mesg,mgg->GetMVEH());
  switch (mesg.dataType) {
  case TYPE_INT:
    value = float(mesg.intData);
    break;

  case TYPE_REAL:
    value = mesg.realData;
    break;
  }
  //----- check for --- table look up -----------
  if (gmap) value   = gmap->Lookup(value);
  else      value   = Clamp(value);
  float     deg     = sang + value;
  // Draw needle into gauge surface
  rotn.Draw (deg);
  return;
}

//=======================================================================
// Textured gauge
//=======================================================================
CTexturedGauge::CTexturedGauge(CPanel *mp)
:CGauge(mp)
{ Prop.Set(ROT_TEXTURE + NO_SURFACE);
  arm = 0;  }
//-----------------------------------------------------------
CTexturedGauge::~CTexturedGauge()
{ }
//-----------------------------------------------------------
//  Read texture parameters
//-----------------------------------------------------------
void CTexturedGauge::ReadLayer(SStream *str,TEXT_DEFN &df)
{ char fn[128];
  ReadString(fn,128,str);
  TEXT_INFO txf;  // Texture info;
	txf.apx = 0;
	txf.azp = 0;
  CArtParser img(TC_HIGHTR);
  strncpy(txf.name,fn,TC_TEXTURE_NAME_NAM);
  _snprintf(txf.path,TC_TEXTURE_NAME_DIM,"ART/%s",fn);
  img.GetAnyTexture(txf);
  df.Copy(txf);
}
//-----------------------------------------------------------
//  Read gauge tags
//-----------------------------------------------------------
int CTexturedGauge::Read(SStream *str, Tag tag)
{ int  ph = panel->GetHeight();
  switch (tag) {
    //--- Define projector -----------------
    case 'cadr':
      DecodeCADR(str,quad,1);
      return TAG_READ;
    case 'proj':
      DecodePROJ(str,quad,0);
      return TAG_READ;
    //--- Define the needle ----------------
    case 'nedl':
      nedl.SetGauge(this);
      ReadFrom(&nedl,str);
      return TAG_READ;
    //--- Define underlay -----------------
    case 'back':
      under.SetGauge(this);
      ReadFrom(&under,str);
      return TAG_READ;
    case 'undr':
      under.Init(str,this);
      return TAG_READ;
    //--- Define overlay ------------------
    case 'ovrl':
      overl.Init(str,this);
      return TAG_READ;
    //--- Define a cover ------------------
    case 'covr':
      overl.SetGauge(this);
      ReadFrom(&overl,str);
      return TAG_READ;
  }
  return CGauge::Read (str, tag);
}
//-----------------------------------------------------------
//  Dupplicate from a similar component
//-----------------------------------------------------------
void CTexturedGauge::CopyFrom(CTexturedGauge &src)
{ CGauge::CopyFrom(src);
  under.CopyFrom(this,src.under);
  overl.CopyFrom(this,src.overl);
  nedl.CopyFrom(this,src.nedl);
  return;
}
//-----------------------------------------------------------
//  Collecte VBO data
//-----------------------------------------------------------
void CTexturedGauge::CollectVBO(TC_VTAB *vtb)
{	CGauge::CollectVBO(vtb);
	under.CollectVBO(vtb);
	overl.CollectVBO(vtb);
	nedl.CollectVBO(vtb);
	return;
}
//-----------------------------------------------------------
//  Draw the needle
//-----------------------------------------------------------
void CTexturedGauge::Draw()
{ // Draw vertical speed needle
  Update();                     // Update value
  nedl.DrawNeedle(value);       // Draw the needle
  return;
}
//-----------------------------------------------------------
//  Helper for Drawing help
//-----------------------------------------------------------
void  CTexturedGauge::ShowHelp(char *fmt, ...)
{ va_list argp;
  va_start(argp, fmt);
  vsprintf(hbuf,fmt,argp);
  FuiHelp();
  return;
}

//==========================================================================
//
//  CDIGIT class
//
//==========================================================================
CDigit::CDigit()
{ mgg   = 0;
  txd   = 0;
  type  = 0;
	vOfs	= 0;

}
//---------------------------------------------------------------------
//  Set Gauge
//---------------------------------------------------------------------
void CDigit::SetGauge(CGauge *mg)
{ mgg		= mg;
  vOfs	= mg->FixRoom(4);
	return;
}

//---------------------------------------------------------------------
//  Read 
//---------------------------------------------------------------------
int CDigit::Read(SStream *str, Tag tag)
{ if (0 == mgg) gtfo("CDigit: Mother gauge needed");
  switch (tag)  {
    //--- Get position ------------------
    case 'posn':
      mgg->DecodeCADR(str,quad,1);
      return TAG_READ;
    //--- GetProjector ------------------
    case 'proj':
      mgg->DecodePROJ(str,quad,1);
      return TAG_READ;
    //--- Integer digit -----------------
    case 'idig':
      type  = 0;
      return TAG_READ;
    //--- Partial digit -----------------
    case 'pdig':
      type  = 1;
      return TAG_READ;
  }
  return TAG_IGNORED;
}
//---------------------------------------------------------------------
//  Collect VBO data
//---------------------------------------------------------------------
void CDigit::CollectVBO(TC_VTAB *vtb)
{	if (0 == vOfs)		return;
	TC_VTAB *dst = vtb + vOfs;
	for (int k=0; k<4; k++) *dst++ = quad[k];
	return;
}
	
//---------------------------------------------------------------------
//  Draw a full digit 
//---------------------------------------------------------------------
void CDigit::Draw(int fr)
{ if (0 == txd)    return;
  int No  = txd->nf - 1 - fr; 
  U_INT *base = (U_INT*)txd->rgba + (No * txd->dm);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,txd->wd,txd->ht,0,GL_RGBA,GL_UNSIGNED_BYTE,base);
	glDrawArrays(GL_TRIANGLE_STRIP,vOfs,4);
  return;

}
//---------------------------------------------------------------------
//  Draw a partial digit with slow rotation
//---------------------------------------------------------------------
void CDigit::DrawPartial(float pr)
{ if (0 == txd)    return;
  //--- compute base line in digit font ---------------------------
  int   ln = (1 - pr) * txd->pm;
  GLubyte *bs = (ln * txd->wd * 4) + txd->rgba;
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,txd->wd,txd->ht,0,GL_RGBA,GL_UNSIGNED_BYTE,bs);
	glDrawArrays(GL_TRIANGLE_STRIP,vOfs,4);
  return;
}
//==========================================================================
//
//  CNeedleMark:  helper to provide manual needle
//
//==========================================================================
CNeedleMark::CNeedleMark()
{ mgg     = 0;
  dmin    = 0;
  dmax    = 360;
  dval    = 0;
	hold		= 0;
}
//---------------------------------------------------------------------
//  Read all parameters
//---------------------------------------------------------------------
int CNeedleMark::Read(SStream *stream, Tag tag)
{ if (0 == mgg) gtfo("<CNeedleMark: need mother gauge");
  switch (tag)  {
    case 'bmap':
      nedl.SetGauge(mgg);
      nedl.ReadBMAP(stream);
      return TAG_READ;
    //--- rotation center --------------
    case 'rotc':
      nedl.ReadROTC(stream);
      return TAG_READ;
    //--- minimum value ----------------
    case 'dmin':
      ReadFloat(&dmin, stream);
      return TAG_READ;
    //--- maximum value ----------------
    case 'dmax':
      ReadFloat(&dmax, stream);
      return TAG_READ;
    //--- Clamp values -----------------
    case 'clmp':
      ReadFloat(&dmin,stream);
      ReadFloat(&dmax,stream);
      return TAG_READ;
    //--- Current value ----------------
		case 'sang':
    case 'dval':
      ReadFloat(&dval,stream);
			SaveValue(dval);
      return TAG_READ;
    //--- Knob -------------------------
    case 'knob':
      ReadKNOB(stream);
      return TAG_READ;

  }
  WARNINGLOG("CNeedleMark: tag %s unknowned",TagToString(tag));
  return TAG_IGNORED;
}

//---------------------------------------------------------------------
//  Read the knob parameters 
//---------------------------------------------------------------------
int CNeedleMark::ReadKNOB(SStream *stream)
{ knob.SetGauge(mgg);
  ReadFrom(&knob,stream);
  return TAG_READ;
}
//---------------------------------------------------------------------
//  Copy from a similar gauge
//---------------------------------------------------------------------
void  CNeedleMark::CopyFrom(CGauge *mg,CNeedleMark &src)
{ mgg   = mg;
  nedl.CopyFrom(mgg,src.nedl);
  dmin  = src.dmin;
  dmax  = src.dmax;
  dval  = src.dval;
	hold	= src.hold;
	pos		= src.pos;
  return;
}
//---------------------------------------------------------------------
//  COllect VBO data
//---------------------------------------------------------------------
void CNeedleMark::CollectVBO(TC_VTAB *vtb)
{	nedl.CollectVBO(vtb);
  knob.CollectVBO(vtb);
	return;
}
//---------------------------------------------------------------------
//  Set mother gauge and quad
//---------------------------------------------------------------------
void CNeedleMark::SetGauge(CGauge *mg)
{ mgg   = mg;
  nedl.SetGauge(mgg);
  return;
}
//---------------------------------------------------------------------
//  Set knob holder 
//---------------------------------------------------------------------
void CNeedleMark::SetHolder(CgHolder *s,char p)
{	pos	  = p;
	hold	= s;
	return;
}
//---------------------------------------------------------------------
//  Save value in holder 
//---------------------------------------------------------------------
void CNeedleMark::SaveValue(float v)
{	if (pos == 1)	hold->fm1	= v;
	if (pos == 2) hold->fm2 = v;
	return;
}
//---------------------------------------------------------------------
//  Draw the gauge
//---------------------------------------------------------------------
void CNeedleMark::Draw()
{ if (pos == 1)	dval = hold->fm1;
	if (pos == 2) dval = hold->fm2;
	if (knob.HasChanged())
  { dval += knob.GetChange();
    if (dval < dmin)  dval = dmin;
    if (dval > dmax)  dval = dmax;
		SaveValue(dval);
  }
  nedl.DrawNeedle(dval);
  return;
}
//--------------------------------------------------------------------
//  Mouse mouves over
//--------------------------------------------------------------------
ECursorResult CNeedleMark::MouseMoved (int x, int y)
{ return knob.MouseMoved(x,y);
}
//---------------------------------------------------------------
//	Gauge clicked => Start rotation
//---------------------------------------------------------------
EClickResult CNeedleMark::MouseClick (int x, int y, int btn)
{ return knob.ArmRotation(1,x,y,btn); }
//----------------------------------------------------------------
//	Stop click:  Disarm rotation
//----------------------------------------------------------------
EClickResult CNeedleMark::StopClick()
{	return knob.DisarmKnob();
}
//==========================================================================
//	CgQUAD a basic textured quad for gauges
//  
//==========================================================================
CgQUAD::CgQUAD()
{	mgg = 0;	}
//-----------------------------------------------------------------
//	Set Mother gauge
//-----------------------------------------------------------------
void CgQUAD::SetGauge(CGauge *mg)
{	mgg		= mg;
  vOfs	= mg->FixRoom(4);
	return;
}
//-----------------------------------------------------------------
//	Decode dimension and build quad 
//-----------------------------------------------------------------
void CgQUAD::DecodeRDIM(char *txt)
{	float w,h;
	int nf = sscanf(txt," %f , %f ", &w, &h);
	if (2 != nf)	gtfo("Invalid <rdim> parameters");
	float sx  = mgg->GetXPos();
  float sy  = mgg->GetYPos();
	mgg->BuildQuad(quad,sx,sy,w,h);
	return;
}
//-----------------------------------------------------------------
//	Read Parameters 
//-----------------------------------------------------------------
int CgQUAD::Read(SStream *stream, Tag tag)
{ if (0 == mgg)	gtfo("<rdim> needs mother gauge");
	float pm;
	char txt[128];
	switch(tag)	{
	case 'bmap':
		ReadStrip(stream,txdf);
		return TAG_READ;
	case 'rdim':
		ReadString(txt,128,stream);
		DecodeRDIM(txt);
		return TAG_READ;
	//--- rigth skew ------------
	case 'rskw':
		ReadFloat(&pm,stream);
		quad[0].VT_Y += pm;
		quad[3].VT_Y += pm;
		return TAG_READ;
			//--- left skew ------------
	case 'lskw':
		ReadFloat(&pm,stream);
		quad[1].VT_Y += pm;
		quad[2].VT_Y += pm;
		return TAG_READ;

	}
  WARNINGLOG("Unknown Tag %s",TagToString(tag));
	return TAG_IGNORED;
}
//-----------------------------------------------------------------
//	Collect VBO data
//-----------------------------------------------------------------
void CgQUAD::CollectVBO(TC_VTAB *vtb)
{	if (0 == vOfs)		return;
	TC_VTAB *dst = vtb + vOfs;
	for (int k=0; k<4; k++)	*dst++ = quad[k];
	return;
}
//-----------------------------------------------------------------
//	Draw the quad
//-----------------------------------------------------------------
void CgQUAD::Draw(float dx, float dy)
{ if (0 == txdf.rgba)    return;
  if (0 == quad)         return;
	glPushMatrix();
	glTranslated(dx,dy,0);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,txdf.wd,txdf.ht,0,GL_RGBA,GL_UNSIGNED_BYTE,txdf.rgba);
	glDrawArrays(GL_TRIANGLE_STRIP,vOfs,4);
	glPopMatrix();
	return;
}
//==========================================================================
//
//  CgHolder:  helper to provide memory for gauges which must keep values
//							that cannot be stored in subsystem.
//						Examplee:  The value of a needle marker when the same gauge
//							appears in different panels.
//==========================================================================
CgHolder::CgHolder(Tag t)
{	idn	= t;
	pm1	= 0;
	pm2	= 0;
	fm1	= 0;
	fm2	= 0;
}
CgHolder::~CgHolder()
{}
//============ END OF FILE ==========================================================
