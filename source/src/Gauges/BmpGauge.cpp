/*
 * BmpGauge.cpp
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
#include "../Include/NewGauge.h"
#include "../Include/WorldObjects.h"
#include "../Include/TerrainTexture.h"
#include "../Include/CursorManager.h"
//============================================================================================

//=========================================================================
//  Global table giving station (VOR or ILS sensibility) in degre
//=========================================================================
float navssyTAB[] = {
  20,                               // VOR shows 20° deviation
  10,                               // ILS shows 10° deviation
};
//=========================================================================
//  Flag text
//=========================================================================
char *navflgTAB[] = {
  "OFF",
  "TO",
  "FROM",
};
//=======================================================================
// JSDEV* CVertical Needle
//  This defines a textured quad, following a vertical trajectory
//========================================================================
CVertNeedle::CVertNeedle(CPanel *mp)
:CGauge(mp)
{	Prop.Set(NO_SURFACE);
	panel	= mp;	}
//---------------------------------------------------------
//	Read Vertical range
//---------------------------------------------------------
void CVertNeedle::ReadVRNG(SStream *stream)
{	char txt[128];
	ReadString(txt,128,stream);
	int nf = sscanf(txt," %f , %f",&p0,&p1);
	if (2 != nf)	gtfo("invalid <nvrt> parameter");
	return;
}
//---------------------------------------------------------
//	Collect VBO data
//---------------------------------------------------------
void CVertNeedle::CollectVBO(TC_VTAB *vtb)
{	nedl.CollectVBO(vtb);
	ovrl.CollectVBO(vtb);
	return;
}
//---------------------------------------------------------
//	Read Parameters
//---------------------------------------------------------
int CVertNeedle::Read(SStream *stream,Tag tag)
{	switch (tag)	{
	case 'quad':
		nedl.SetGauge(this);
		ReadFrom(&nedl,stream);
		return TAG_READ;
	case 'vrng':
		ReadVRNG(stream);
		return TAG_READ;
	//--- Define overlay ------------------
  case 'ovrl':
    ovrl.Init(stream,this);
    return TAG_READ;
	}
return CGauge::Read(stream,tag);
}
//---------------------------------------------------------
//	All parameters are read
//---------------------------------------------------------
void	CVertNeedle::ReadFinished()
{	CGauge::ReadFinished();	
	//--- compute pixel per value ratio -----
	ppv				= 0;
	float dv	= gmax - gmin;		// Value delta
	float dp  = p1 - p0;
	if (fabs(dv) > FLT_EPSILON) ppv = dp / dv;
	return;
}
//---------------------------------------------------------
//	Draw the quad
//---------------------------------------------------------
void CVertNeedle::Draw()
{	Update();
	float dy = p0 + ((value - gmin) * ppv);
	nedl.Draw(0,-dy);
	return;
}
//---------------------------------------------------------
//	Draw in ambient light
//---------------------------------------------------------
void CVertNeedle::DrawAmbient()
{	ovrl.Draw();
	return;	}
//=======================================================================
// JSDEV* CNavigationGauge
//  All computing done in the subsystem
//========================================================================
C_NavigationGauge::C_NavigationGauge (CPanel *mp)
: CTexturedGauge(mp)
{ Prop.Set(NO_SURFACE);
  // Subsystem message
  radi_tag  = 0;             // Default radio
  radi_unit = 1;             // Unit 1
  vobs      = 0;
  radio     = 0;
	return;
}
//--------------------------------------------------------
C_NavigationGauge::~C_NavigationGauge (void)
{}
//-------------------------------------------------------
//	Prepare message
//-------------------------------------------------------
void C_NavigationGauge::PrepareMsg(CVehicleObject *veh)
{ veh->FindReceiver(&mobs);
  veh->FindReceiver(&mrad);
	CGauge::PrepareMsg(veh);
	return;
}
//------------------------------------------------------
//	Decode flag texture
//-------------------------------------------------------
void C_NavigationGauge::DecodeFlag(SStream *str)
{ char *erm = "Texture %s does not contain integral frame number";
  TEXT_INFO txf;                // Texture info;
  ReadString(txf.name,TC_TEXTURE_NAME_NAM,str);
  //--- Read the texture ----------------------
  CArtParser img(TC_HIGHTR);
  txf.apx   = 0xFF;
	txf.azp   = 0;
  _snprintf(txf.path,TC_TEXTURE_NAME_DIM,"ART/%s",txf.name);
  img.GetAnyTexture(txf);
  Flag.Copy(txf);
	txf.mADR = 0;
  //---- Adjust texture height and size -------
  int nf  = 3;
  Flag.nf = nf;
  Flag.ht = txf.ht / nf;
  if (0  != (txf.ht % nf)) gtfo(erm,txf.name);
  Flag.dm = Flag.ht * Flag.wd;
  return;
}
//------------------------------------------------------
//	Decode TAB
//-------------------------------------------------------
void C_NavigationGauge::DecodeTab(SStream *str, VStrip &p)
{ char txt[128];
  int xp,yp;
  int wd,ht;
  ReadString(txt,128,str);
  int nf = sscanf(txt," %d, %d , %d , %d",&xp,&yp,&wd,&ht);
  if (4 != nf)  gtfo("<gtab> bad coordinates");
  p.LocalQuad(xp,yp,wd,ht);
  return;
}
//------------------------------------------------------
//	Read Tags
//-------------------------------------------------------
int C_NavigationGauge::Read (SStream *stream, Tag tag)
{ short u = 0;
  short v = 0;
  int rc = TAG_IGNORED;
  Tag nav = 0;
  switch (tag) {
    //--- Radio -------------------------------
    case 'radi':
		  ReadTag (&radi_tag, stream);
		  ReadInt (&radi_unit, stream);
		  return TAG_READ;
    //--- Compass plate -----------------------
    case 'comp':
      nedl.SetGauge(this);
		  ReadFrom (&nedl, stream);
		  return TAG_READ;
    //--- Localizer needle -------------------
    case 'lndl':
		  lndl.SetGauge(this);
      ReadFrom(&lndl,stream);
		  return TAG_READ;
    //--- glide needle -------------------
    case 'gndl':
      gndl.SetGauge(this);
		  ReadFrom(&gndl,stream);
      gldK = gndl.GetAMPV() / TANGENT_5DEG;
		  return TAG_READ;
    //--- OBS knob -----------------------
    case 'obs_':
      knob.SetGauge(this);
      ReadFrom(&knob,stream);
		  return TAG_READ;
    //--- flag texture --------------------
    case 'flag':
      DecodeFlag(stream);
		  return TAG_READ;
    //--- GLIDE TAB ---------------------
    case 'gtab':
      gldF.SetGauge(this);
      DecodeTab(stream,gldF);
		  return TAG_READ;
    //--- NAV TAB ---------------------
    case 'ntab':
      locF.SetGauge(this);
      DecodeTab(stream,locF);
		  return TAG_READ;
    //--- Copy from similar ------------
    case 'copy':
      CopyFrom(stream);
      return TAG_READ;

  }
  return CTexturedGauge::Read (stream, tag);
 }
//----------------------------------------------------------
//  All parameters are read
//----------------------------------------------------------
void C_NavigationGauge::ReadFinished ()
{ CTexturedGauge::ReadFinished ();
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
  //----Radio message ---------------
  mrad.sender         = unId;
  mrad.dataType       = TYPE_VOID;
  mrad.user.u.hw      = HW_RADIO;
  mrad.id             = MSG_GETDATA;
  mrad.user.u.datatag = 'getr';
  mrad.group          = radi_tag;
  mrad.user.u.unit    = radi_unit;
  return;
}
//----------------------------------------------------------
//  Dupplicate from similar
//  NOTE:  Flag tabs position are not dupplicated as 
//          position varies from gauge model
//----------------------------------------------------------
void C_NavigationGauge::CopyFrom(SStream *stream)
{ C_NavigationGauge &src = *(C_NavigationGauge*)LocateGauge(stream);
  CTexturedGauge::CopyFrom(src);
  //--- copy local parameters --------------------
  radi_tag  = src.radi_tag;
  radi_unit = src.radi_unit;
  mobs  = src.mobs;
  mrad  = src.mrad;
  lndK  = src.lndK;
  gldK  = src.gldK;
  lndl.CopyFrom(this,src.lndl);
  gndl.CopyFrom(this,src.gndl);
  Flag.Dupplicate(src.Flag);
  return;
}
//-------------------------------------------------------------------
//  Collect vbo data
//-------------------------------------------------------------------
void	C_NavigationGauge::CollectVBO(TC_VTAB *vtb)
{	CTexturedGauge::CollectVBO(vtb);
	lndl.CollectVBO(vtb);
	gndl.CollectVBO(vtb);
	knob.CollectVBO(vtb);
	gldF.CollectVBO(vtb);
	locF.CollectVBO(vtb);
}
//-------------------------------------------------------------------
//  Get radio block 
//-------------------------------------------------------------------
void C_NavigationGauge::GetRadio()
{ Send_Message(&mrad,mveh);
  radio     = (BUS_RADIO*)mrad.voidData;
  return;
}
//-------------------------------------------------------------------
//	Knob click => Arm rotation
//-------------------------------------------------------------------
EClickResult C_NavigationGauge::MouseClick (int x, int y, int btn)
{	return knob.ArmRotation(1,x,y,btn);	}
//----------------------------------------------------------------
//	Stop click => disarm rotation
//-----------------------------------------------------------------
EClickResult C_NavigationGauge::StopClick ()
{	return knob.DisarmKnob();
}
//-------------------------------------------------------------------
//	Mouse move over
//-------------------------------------------------------------------
ECursorResult C_NavigationGauge::MouseMoved (int x, int y)
{	if (knob.MouseMoved(x,y))  return ShowOBS(); 
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//-------------------------------------------------------------------
//	Draw note over knob
//-------------------------------------------------------------------
ECursorResult C_NavigationGauge::ShowOBS()
{ sprintf_s(hbuf,HELP_SIZE,"OBS (%s) %.0f°",navflgTAB[rflg],vobs);
  FuiHelp();
  return CURSOR_WAS_CHANGED;
}
//-----------------------------------------------------------------------------
//	JSDEV* Draw the gauge
//	NOTE:	  This gauge has 2 modes:  NAV and ILS
//          When in NAV mode, the obs is taken in account for the 
//          needle deviation.  When in ILS mode, the ILS direction is
//          driving the deviation.  The correct setting is maintained in 
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
//    4     For an ILS, the glide needle sensibility is 5° above or below ideal slope
//          As the glide error is given in tan unit , the value gERR should be clamped 
//          to the [tan(-5°), tan(+5°)] range.  
//          The needle is restricted in the [-amp,+amp] range which is the range of
//          needle tip, the corresponding deviation is given by the constant
//          K = amp / tan(5°) so ang = gERR * K
//----------------------------------------------------------------------------------
void C_NavigationGauge::Draw (void)
{	if (0 == radio) GetRadio();
  if (0 == radio) return;
	//---------Get OBS or ILS direction (depending on tuned station)  ----
  rflg          = radio->flag;
  //-------  Get NAV Type (ILS or VOR) ---------------------------------
  ils = (radio->ntyp == SIGNAL_ILS);                             
	//---- Draw Glide flags ----------------------------------------------
  locF.Draw(&Flag,rflg);                            // Draw texture
  if (0 == ils) rflg = VOR_SECTOR_OF;               // No ILS, no glide flag
  gldF.Draw(&Flag,rflg); 
  //-------Draw glide slope needle up to +- 5° around  ---------------------------
  float gERR    = ClampTo(float(TANGENT_5DEG),radio->gDEV);      // Glide error
  gndl.DrawNeedle(gERR * gldK);
	//----------- Draw localizer needle ----------------------------------
  float sbly    = navssyTAB[ils];
  lndK          = (lndl.GetAMPH() / sbly);         // Compute sensibility
  float dev     = ClampTo(sbly,radio->hDEV);       // Deviation in deg
	lndl.DrawNeedle(dev * lndK);
	//--------- Draw compass card according to knob rotation -------------
  vobs = float(radio->xOBS);
  nedl.DrawNeedle (-vobs);
	//----- Draw OBS knob if rotated ---------------------------------------
   if (knob.HasChanged())	
  { mobs.realData = knob.GetChange();             
	  Send_Message(&mobs,mveh);
    ShowOBS();
  }
  knob.Draw(); 
	
	return;
}
//==============================================================================
// CTachometerGauge
//===============================================================================
C_TachometerGauge::C_TachometerGauge (CPanel *mp)
: CTexturedGauge(mp)
{ Prop.Set(NO_SURFACE);
  hobs = 0;
}
//----------------------------------------------------------------------
//  Destructor
//	NOTE: Do not delete the hobb, it will be deleted by panel
//----------------------------------------------------------------------
C_TachometerGauge::~C_TachometerGauge (void)
{
}
//----------------------------------------------------------------------
//  read parameters
//----------------------------------------------------------------------
int C_TachometerGauge::Read (SStream *stream, Tag tag)
{switch (tag) {
  case 'nedl':
    nedl.SetGauge(this);
    ReadFrom(&nedl,stream);
    return TAG_READ;
	//--- hobb meter is defined --------------
  case 'hobb':
    hobs = new C_HobbsMeterGauge(panel);
		panel->AuxilliaryGauge(hobs,stream);
    return TAG_READ;
	//--- hobb position ----------------------
	//--- Dupplicate from similar ------------
  case 'copy':
    CopyFrom(stream);
    return TAG_READ;

  }
  return CTexturedGauge::Read (stream, tag);
}
//----------------------------------------------------------------------
//  All parameters are read
//----------------------------------------------------------------------
void C_TachometerGauge::ReadFinished (void)
{ CTexturedGauge::ReadFinished ();
}
//----------------------------------------------------------------------
//  Copy from similar gauge
//----------------------------------------------------------------------
void C_TachometerGauge::CopyFrom(SStream *stream)
{ C_TachometerGauge &src = *(C_TachometerGauge*)LocateGauge(stream);
  CTexturedGauge::CopyFrom(src);
	nedl.CopyFrom(this,src.nedl);
	hobs = 0;
	return;
}
//----------------------------------------------------------------------
//  Collect VBO data
//----------------------------------------------------------------------
void  C_TachometerGauge::CollectVBO(TC_VTAB *vtb)
{	CTexturedGauge::CollectVBO(vtb);
	nedl.CollectVBO(vtb);
	if (0 == hobs)	return;
	hobs->CollectVBO(vtb);
	return;
}
//----------------------------------------------------------------------
//  Draw the needle
//----------------------------------------------------------------------
void C_TachometerGauge::Draw (void)
{ DrawUnderlay(); 
	Update();
  if (hobs) hobs->Draw();
  nedl.DrawNeedle(value);
  return;
}
//=====================================================================
// JSDEV* modified CBasicADFGauge
//====================================================================
C_BasicADFGauge::C_BasicADFGauge (CPanel *mp)
: CTexturedGauge(mp)
{ Prop.Set(NO_SURFACE);
  cpas = 0;
}
//--------------------------------------------------------------------
C_BasicADFGauge::~C_BasicADFGauge (void)
{}
//--------------------------------------------------------------------
//  Read the tags
//--------------------------------------------------------------------
int C_BasicADFGauge::Read (SStream *stream, Tag tag)
{  switch (tag) {
    //--- Compas ring ------------------------
    case 'comp':
      comp.SetGauge(this);
      ReadFrom(&comp,stream);
      return TAG_READ;
    //--- Knob -------------------------------
    case 'knob':
      knob.SetGauge(this);
      ReadFrom(&knob,stream);
      return TAG_READ;
    //--- Needle ------------------------------
    case 'nedl':
      nedl.SetGauge(this);
      ReadFrom(&nedl,stream);
      return TAG_READ;
    //--- Dupplicate from similar ------------
    case 'copy':
      CopyFrom(stream);
      return TAG_READ;
  }
  return CTexturedGauge::Read (stream, tag);
  }
//---------------------------------------------------------------
//  All parameters are read
//---------------------------------------------------------------
void C_BasicADFGauge::ReadFinished (void)
{ CTexturedGauge::ReadFinished();
  knob.InitQuad();
  mesg.user.u.hw = HW_RADIO;
}
//---------------------------------------------------------------
//	Dupplicate from similar
//  Note: The knob is  not dupplicated as it uses
//        absolute coordinates
//---------------------------------------------------------------
void C_BasicADFGauge::CopyFrom(SStream *stream)
{ C_BasicADFGauge &src = *(C_BasicADFGauge*)LocateGauge(stream);
  CTexturedGauge::CopyFrom(src);
  //--- local parameters -------------------------
  comp.CopyFrom(this,src.comp);
  return;
}
//---------------------------------------------------------------
//	Collect VBO data
//---------------------------------------------------------------
void C_BasicADFGauge::CollectVBO(TC_VTAB *vtb)
{ CTexturedGauge::CollectVBO(vtb);
	knob.CollectVBO(vtb);
	comp.CollectVBO(vtb);
	return;
}
//---------------------------------------------------------------
//	Mouse moves over
//---------------------------------------------------------------
ECursorResult C_BasicADFGauge::MouseMoved (int x, int y)
{ // Send updated mouse position to all click areas
  if (knob.MouseMoved(x,y)) {ShowDIR(); return CURSOR_WAS_CHANGED;}
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//---------------------------------------------------------------
//	knob clicked => Start rotation
//---------------------------------------------------------------
EClickResult C_BasicADFGauge::MouseClick (int x, int y, int btn)
{ return knob.ArmRotation(1,x,y,btn); }
//----------------------------------------------------------------
//	Stop click:  Disarm rotation
//----------------------------------------------------------------
EClickResult C_BasicADFGauge::StopClick()
{	return knob.DisarmKnob();
}
//----------------------------------------------------------------
//	Draw the gauge
//------------------------------------------------------------------
void C_BasicADFGauge::Draw (void)
{ // Get navaid pointer from ADF radio
	mesg.id = MSG_GETDATA;
	mesg.user.u.datatag = 'navd';
	Send_Message (&mesg,mveh);
	navd = (float)mesg.realData;
  //-----Get compass --------------------------------
  mesg.user.u.datatag = 'comp';
	Send_Message (&mesg,mveh);
  cpas = mesg.intData;
	//------draw compass ------------------------------
  comp.DrawNeedle(float(-cpas));
  nedl.DrawNeedle(navd);
	//---------- Draw knob surface---------------------
  if (knob.HasChanged())
  { mesg.id             = MSG_SETDATA;
    mesg.user.u.datatag = 'comp';
    mesg.realData       = knob.GetChange();
    Send_Message(&mesg,mveh);
    ShowDIR();
  }
  knob.Draw();
  //--- and now draw overlay -----------------------
  DrawOverlay();
  return;
}
//-------------------------------------------------------------------
//	Draw note over knob
//-------------------------------------------------------------------
void C_BasicADFGauge::ShowDIR()
{ sprintf_s(hbuf,HELP_SIZE,"DIR %03d°",cpas);
  FuiHelp();
  return;
}
//=====================================================================
// JSDEV* modified CDirectionalGyroGauge
//=====================================================================
C_DirectionalGyroGauge::C_DirectionalGyroGauge (CPanel *mp)
: CTexturedGauge(mp)
{	Prop.Set(NO_SURFACE);
  dir = 0;
}
//-------------------------------------------------------------
C_DirectionalGyroGauge::~C_DirectionalGyroGauge (void)
{}
//-------------------------------------------------------------
//  Read GYRO tags
//-------------------------------------------------------------
int C_DirectionalGyroGauge::Read (SStream *stream, Tag tag)
{ Tag  apb = 0;
  switch (tag) {
    //--- autopilot bug ------------
    case 'apbg':
      apbg.SetGauge(this);
      ReadTag(&apb,stream);
      mbug.user.u.datatag = apb;
      ReadFrom(&apbg,stream);
      return TAG_READ;
    //--- gyro correction knob -----
    case 'knob':
      knob.SetGauge(this);
      ReadFrom(&knob,stream);
		  return TAG_READ;
    //--- bug knob -----------------
    case 'apkb':
      apkb.SetGauge(this);
      ReadFrom(&apkb,stream);
      return TAG_READ;
    //--- use bias on -------------
    case 'bias':
      ReadTag (&bias, stream);
		  return TAG_READ;
    //--- copy from similar -------
    case 'copy':
      CopyFrom(stream);
      return TAG_READ;
  }

  return  CTexturedGauge::Read (stream, tag);
}
//-------------------------------------------------------------------
//  End of tag read
//-------------------------------------------------------------------
void C_DirectionalGyroGauge::ReadFinished ()
{ CGauge::ReadFinished();
  //------------------------------------------
}
//----------------------------------------------------------------------
//	Dupplicate from similar gauge
//----------------------------------------------------------------------
void C_DirectionalGyroGauge::CopyFrom(SStream *stream)
{ C_DirectionalGyroGauge &src = *(C_DirectionalGyroGauge*)LocateGauge(stream);
  CTexturedGauge::CopyFrom(src);
  //-------------------------------------------
  apbg.CopyFrom(this,src.apbg);
  bias  = src.bias;
  mbug  = src.mbug;
  mgyr  = src.mgyr;
  return;
}
//----------------------------------------------------------------------
//	Collect VBO data
//----------------------------------------------------------------------
void C_DirectionalGyroGauge::CollectVBO(TC_VTAB *vtb)
{	CTexturedGauge::CollectVBO(vtb);
	apbg.CollectVBO(vtb);
	knob.CollectVBO(vtb);
	apkb.CollectVBO(vtb);
	return;
}
//-------------------------------------------------------------------
//	Prepare message
//------------------------------------------------------------------
void C_DirectionalGyroGauge::PrepareMsg(CVehicleObject *veh)
{ //--- Locate gyro subsystem ---------
	mesg.sender     = unId;
	veh->FindReceiver(&mesg);
	gyrS = (CSubsystem*)mesg.receiver;
	//--- Locate bug receiver -----------
	mbug.group			= mesg.group;
  mbug.sender     = unId;
	mbug.id         = MSG_SETDATA;
	mbug.dataType   = TYPE_REAL;
	veh->FindReceiver(&mbug);
	//--- Locate compss receiver -------
	mgyr.group			= mesg.group;
  mgyr.sender         = unId;
	mgyr.user.u.datatag = 'dgyr';
	mgyr.id             = MSG_SETDATA;
	mgyr.dataType       = TYPE_REAL;
  veh->FindReceiver(&mgyr);
	CGauge::PrepareMsg(veh);
	return;	
}
//----------------------------------------------------------------------
//	Check for knob click
//----------------------------------------------------------------------
EClickResult C_DirectionalGyroGauge::MouseClick (int x, int y, int btn)
{ // Initialize Ap knob hold-down values
  if (apkb.ArmRotation(1,x,y,btn))
  { DisplayBUG();
		return MOUSE_TRACKING_ON;
	}
  if (knob.ArmRotation(0.2f,x,y,btn))
  { DisplayHDG();
		return MOUSE_TRACKING_ON;
	}
  return MOUSE_TRACKING_OFF;
}
//---------------------------------------------------------------------
//	stop click
//---------------------------------------------------------------------
EClickResult C_DirectionalGyroGauge::StopClick ()
{ //Release OBS knob click if it is set
  knob.DisarmKnob();
  apkb.DisarmKnob();
  return MOUSE_TRACKING_OFF;
}
//----------------------------------------------------------------------
//	Draw the gauge
//	NOTE:  Is is expected from the gyro subsystem that
//				yaw is published through GaugeBusFT01()
//				bug is published through GaugeBusFT02()
//----------------------------------------------------------------------
void C_DirectionalGyroGauge::Draw (void)
{ DrawUnderlay();
  //--- Get hsi heading -------------------------------------
	hdg	= gyrS->GaugeBusFT01();
	bug	= gyrS->GaugeBusFT02();
  nedl.DrawNeedle(Wrap360(-hdg));

	//-----Update the gyro knob --------------------------------
  if (knob.HasChanged())
	{	mgyr.id       = MSG_SETDATA;
    mgyr.realData = knob.GetChange();
		Send_Message(&mgyr,mveh);
    DisplayHDG();
	}
  knob.Draw();

	//------Update the auto pilot knob ---and bug------------------------
  mbug.realData = 0;
  if (apkb.HasChanged()) 
  { mbug.id       = MSG_SETDATA;
    mbug.realData = apkb.GetChange();
    Send_Message(&mbug,mveh);
    bug           =  mbug.intData;
    dir           =  mbug.user.u.unit;
    DisplayBUG();
  }
  apkb.Draw();
	//-----Draw the bug over the plate ----------------------------------
	apbg.DrawNeedle(bug);
	DrawOverlay();
	return;
}

//-------------------------------------------------------------------
//	Draw gyro heading
//-------------------------------------------------------------------
ECursorResult C_DirectionalGyroGauge::DisplayHDG()
{ sprintf_s(hbuf,HELP_SIZE,"DIR: %.2f°",hdg);
  FuiHelp();
  return CURSOR_WAS_CHANGED;
}
//------------------------------------------------------------------------
//  Display autopilot bug direction
//------------------------------------------------------------------------
ECursorResult C_DirectionalGyroGauge::DisplayBUG()
{ sprintf_s(hbuf,HELP_SIZE,"BUG: %03d°",dir);
  FuiHelp();
  return CURSOR_WAS_CHANGED;
}
//-------------------------------------------------------------------
//	Mouse move over
//-------------------------------------------------------------------
ECursorResult C_DirectionalGyroGauge::MouseMoved (int x, int y)
{ if (knob.MouseMoved(x,y)) return DisplayHDG();  
  if (apkb.MouseMoved(x,y)) return DisplayBUG(); 
  return CURSOR_WAS_CHANGED;
}
//=======================================================================
//
// JSDEV*  CAltimeterGauge
//
//=======================================================================
C_AltimeterGauge::C_AltimeterGauge (CPanel *mp)
: CTexturedGauge(mp)
{ Prop.Set(NO_SURFACE);
  // Initialize members
  ndl1  = ndl2 = ndl3 = 0;
  baro  = 29.92f;
}
//--------------------------------------------------------------------
//  Free all resources
//--------------------------------------------------------------------
C_AltimeterGauge::~C_AltimeterGauge (void)
{
  if (ndl1) delete ndl1;
  if (ndl2) delete ndl2;
  if (ndl3) delete ndl3;
}
//--------------------------------------------------------------------
//  Read parameters
//---------------------------------------------------------------------
int C_AltimeterGauge::Read (SStream *stream, Tag tag)
{ int pm;
  char txt[64];

  switch (tag) {
  case 'psiz':
    ReadInt (&pm, stream);
    ReadInt (&pm, stream);
    return TAG_READ;
  case 'base':
    base.SetGauge(this);
    base.ReadStrip(stream);
    return TAG_READ;
  case 'ndl1':
    ndl1 = new CNeedle(this);
    ReadFrom (ndl1, stream);
    return TAG_READ;

  case 'ndl2':
    ndl2 = new CNeedle(this);
    ReadFrom (ndl2, stream);
    return TAG_READ;

  case 'ndl3':
    ndl3 = new CNeedle(this);
    ReadFrom (ndl3, stream);
    return TAG_READ;

  case 'csrl':
    ReadString (txt, 64, stream);
    return TAG_READ;

  case 'csrr':
    ReadString (txt, 64, stream);
    return TAG_READ;

  case 'koll':
    nedl.SetGauge(this);
    ReadFrom (&nedl, stream);
    return TAG_READ;

  case 'kpxy':
    ReadInt (&pm, stream);
    ReadInt (&pm, stream);
    return TAG_READ;

  case 'knob':
    { alkn.SetGauge(this);
      ReadFrom(&alkn,stream);
      return TAG_READ;

  case 'copy':
      CopyFrom(stream);
      return TAG_READ;
    }
  case 'kmsg':
    ReadMessage (&kmsg, stream);
    kmsg.sender = 'kmsg';
   return TAG_READ;
  }
  return CTexturedGauge::Read (stream, tag);
}
//--------------------------------------------------------------------
//  All parameters are read. Set the quad to all needles
//---------------------------------------------------------------------
void C_AltimeterGauge::ReadFinished()
{ CGauge::ReadFinished();
  alkn.InitQuad();
  return;
}
//--------------------------------------------------------------------
//  Copy from a similar gauge
//---------------------------------------------------------------------
void C_AltimeterGauge::CopyFrom(SStream *stream)
{ C_AltimeterGauge &src = *(C_AltimeterGauge*)LocateGauge(stream);
  CTexturedGauge::CopyFrom(src);
  //--- Copy local components ------------------
  base.CopyFrom(this,src.base);
  if (src.ndl1) { ndl1 = new CNeedle(this); ndl1->CopyFrom(this,*src.ndl1);}
  if (src.ndl2) { ndl2 = new CNeedle(this); ndl2->CopyFrom(this,*src.ndl2);}
  if (src.ndl3) { ndl3 = new CNeedle(this); ndl3->CopyFrom(this,*src.ndl3);}
  kmsg  = src.kmsg;
  baro  = src.baro;
}
//--------------------------------------------------------------------
//  Collect VBO data
//---------------------------------------------------------------------
void C_AltimeterGauge::CollectVBO(TC_VTAB *vtb)
{	CTexturedGauge::CollectVBO(vtb);
	base.CollectVBO(vtb);
	ndl1->CollectVBO(vtb);
	ndl2->CollectVBO(vtb);
	ndl3->CollectVBO(vtb);
	alkn.CollectVBO(vtb);
	return;
}
//-----------------------------------------------------------
//  JSDEV* Prepare Messages
//-----------------------------------------------------------
void C_AltimeterGauge::PrepareMsg(CVehicleObject *veh)
{	kmsg.id       =	 MSG_SETDATA;
	kmsg.dataType = TYPE_REAL;
	veh->FindReceiver(&kmsg);
	
	CGauge::PrepareMsg(veh);
	return;	}
//-------------------------------------------------------------------
//	Draw note over knob
//-------------------------------------------------------------------
void C_AltimeterGauge::DisplayHelp()
{ sprintf_s(hbuf,HELP_SIZE,"Baro %.2f",baro);
  FuiHelp();
  return;
}
//--------------------------------------------------------------------
//  Mouse mouves over
//--------------------------------------------------------------------
ECursorResult C_AltimeterGauge::MouseMoved (int x, int y)
{ alkn.MouseMoved(x,y);
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//--------------------------------------------------------------------
//	Mouse click: Start knob rotation if hit
//--------------------------------------------------------------------
EClickResult C_AltimeterGauge::MouseClick (int x, int y, int buttons)
{ return alkn.ArmRotation(0.01f,x,y,buttons);
}
//-----------------------------------------------------------------------
//	Mouse up:  stop rotation
//----------------------------------------------------------------------
EClickResult C_AltimeterGauge::StopClick ()
{	return alkn.DisarmKnob();
}
//----------------------------------------------------------------------
//		Draw the altimeter
//		NOTE: It is expected that the subsystem
//				a) Publishes altitude in GaugeBusFT01()
//				b) Publishes radar altitude in GaugeBusFT02();
//-----------------------------------------------------------------------
void C_AltimeterGauge::Draw(void)
{	//--- Draw underlay for light ----------------------------
	DrawUnderlay();
	//--- Draw the baro plate --------------------------------
  if (gmap)  value = gmap->Lookup(baro);
  nedl.DrawNeedle(value);
  //--- Draw the base --------------------------------------
  base.Draw(0);
  //--- Compute needle values ------------------------------
	float alt = subS->GaugeBusFT01();
  float f3  = alt * (float(36)/10000);
  alt       = fmod(alt,10000);
  float f2  = alt * (float(36)/1000);
  alt       = fmod(alt,1000);
  float f1  = alt * (float(36)/ 100);
//-----Draw the needles ------------------------------------
	if(ndl3) ndl3->DrawNeedle(f3);
	if(ndl2) ndl2->DrawNeedle(f2);
	if(ndl1) ndl1->DrawNeedle(f1);
  DrawOverlay();
  //---- If koll value change: redraw and update barometer in subsystem
  if (alkn.HasChanged())
  { kmsg.user.u.datatag = 'knob';
    kmsg.realData	      = alkn.GetChange();
    Send_Message (&kmsg,mveh);
    baro = float(kmsg.intData) * 0.01;
    DisplayHelp();
  }
  alkn.Draw();

	return;
}
//=================================================================
// CAirspeedGauge
// TODO: Implement the MMO needle and knob
//==================================================================
C_AirspeedGauge::C_AirspeedGauge (CPanel *mp)
: CTexturedGauge(mp)
{ Prop.Set(NO_SURFACE);
}
//----------------------------------------------------------
//  Read all parameters
//----------------------------------------------------------
int C_AirspeedGauge::Read (SStream *str, Tag tag)
{ switch (tag) {
  //--- Airspeed needle -------------------------
  //--- MMO knob --------------------------------
  case 'knob':
    knob.SetGauge(this);
    ReadFrom (&knob, str);
    knob.InitQuad();
    return TAG_READ;
  //--- MMO Needle -----------------------------
  case 'mmo_':
    nmmo.SetGauge(this);
    ReadFrom (&nmmo, str);
    return TAG_READ;
  //--- Copy from similar ----------------------
  case 'copy':
    CopyFrom(str);
    return TAG_READ;
  }

  return CTexturedGauge::Read (str, tag);
}
//-----------------------------------------------------------
//  All parameters are read
//-----------------------------------------------------------
void C_AirspeedGauge::ReadFinished()
{ CTexturedGauge::ReadFinished();
  return;
}
//-----------------------------------------------------------
//  Dupplicate from similar gauge
//-----------------------------------------------------------
void C_AirspeedGauge::CopyFrom(SStream *stream)
{ C_AirspeedGauge &src = *(C_AirspeedGauge *)LocateGauge(stream);
  CTexturedGauge::CopyFrom(src);
  //--- Dupplicate parameters ---------------
  nmmo.CopyFrom(this,src.nmmo);
  return;
}
//-----------------------------------------------------------
//  Collect VBO data
//-----------------------------------------------------------
void	C_AirspeedGauge::CollectVBO(TC_VTAB *vtb)
{	CTexturedGauge::CollectVBO(vtb);
	knob.CollectVBO(vtb);
  nmmo.CollectVBO(vtb);
	return;
}
//-----------------------------------------------------------
//	Check for knob click
//-----------------------------------------------------------
EClickResult C_AirspeedGauge::MouseClick (int x, int y, int btn)
{ // Initialize Ap knob hold-down values
  return knob.ArmRotation(1,x,y,btn);
}
//-----------------------------------------------------------------------
//	Mouse up:  stop rotation
//----------------------------------------------------------------------
EClickResult C_AirspeedGauge::StopClick ()
{	return knob.DisarmKnob();
}
//-----------------------------------------------------------
//  Mouse move over
//-----------------------------------------------------------
ECursorResult C_AirspeedGauge::MouseMoved (int x, int y)
{ knob.MouseMoved (x, y);
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//-----------------------------------------------------------
//  Display the gauge
//	NOTE:  It is expected that the subsystem
//			a) Publishes the airspeed in GaugeBusFT01();
//-----------------------------------------------------------
void C_AirspeedGauge::Draw()
{ DrawUnderlay();
	float spd = subS->GaugeBusFT01();
	Translate(spd);
	nedl.DrawNeedle(value);       // Draw the needle
  knob.HasChanged();
  knob.Draw();
  return;
}
//======================================================================
// CVerticalSpeedGauge
//  TODO:  Implement the mvsi knob and bug
//				redefine the draw to use published value.
//	NOTE: Actually this gauge dont work because it is not connected to the
//				subsystem
//======================================================================
C_VerticalSpeedGauge::C_VerticalSpeedGauge (CPanel *mp)
: CTexturedGauge(mp)
{ Prop.Set(NO_SURFACE);
}
//---------------------------------------------------------------------
//	JSDEV* Prepare messages
//---------------------------------------------------------------------
void C_VerticalSpeedGauge::PrepareMsg(CVehicleObject *veh)
{	mesg.id			  = MSG_GETDATA;
  mesg.dataType	= TYPE_REAL;
	CGauge::PrepareMsg(veh);
	return;
}
//---------------------------------------------------------------------
//  Destructor
//---------------------------------------------------------------------
C_VerticalSpeedGauge::~C_VerticalSpeedGauge (void)
{	return;
}
//---------------------------------------------------------------------
//  Collect VBO data
//---------------------------------------------------------------------
void C_VerticalSpeedGauge::CollectVBO(TC_VTAB *vtb)
{	CTexturedGauge::CollectVBO(vtb);
	knob.CollectVBO(vtb);
	return;
}
//---------------------------------------------------------------------
//	Read tags
//---------------------------------------------------------------------
int C_VerticalSpeedGauge::Read (SStream *str, Tag tag)
{ switch (tag) {
    case 'knob':
      knob.SetGauge(this);
      ReadFrom (&knob, str);
      return TAG_READ;

    case 'vmsg':
      ReadMessage (&mesg, str);
      return TAG_READ;
  }
  return CTexturedGauge::Read (str, tag);
}
//-------------------------------------------------------------------
//  Mouse moves over
//-------------------------------------------------------------------
ECursorResult C_VerticalSpeedGauge::MouseMoved (int x, int y)
{ //vskb.MouseMoved (x, y);
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//-------------------------------------------------------------------
//	Draw the gauge
//-------------------------------------------------------------------
void C_VerticalSpeedGauge::Draw (void)
{ DrawUnderlay();
	CTexturedGauge::Draw();
  // Get vertical speed bug value
  //  Send_Message (&vmsg);
  //  vsbg.Draw(surf);
  return;
}
//===========================================================================
// JSDEV* modified CHorizonGauge
//===========================================================================
C_HorizonGauge::C_HorizonGauge (CPanel *mp)
: CTexturedGauge(mp)
{	Prop.Set(NO_SURFACE);
  hoff    = 0;
	pixd    = 0;
	mdeg    = 0;
  // set messages default tag
	pich.user.u.datatag = 'pich';
  pich.sender = 'pich';

	roll.user.u.datatag = 'roll';
  roll.sender = 'roll';
}
//----------------------------------------------------------------------
C_HorizonGauge::~C_HorizonGauge (void)
{ 
}
//---------------------------------------------------------------------
//	Read all tags
//---------------------------------------------------------------------
int C_HorizonGauge::Read (SStream *str, Tag tag)
{ switch (tag) {
  case 'foot':
    Bfoot.SetGauge(this);
    ReadFrom(&Bfoot,str);
	  return TAG_READ;

  case 'mire':
    Bmire.SetGauge(this);
	  ReadFrom(&Bmire,str);
	  return TAG_READ;

  case 'irim':
    Birim.SetGauge(this);
	  ReadFrom(&Birim,str);
	  return TAG_READ;

  case 'orim':
    Borim.SetGauge(this);
	  ReadFrom(&Borim,str);
	  return TAG_READ;


  case 'hoff':
    ReadInt (&hoff, str);
	  return TAG_READ;

  case 'pixd':
    ReadDouble (&pixd, str);
	  return TAG_READ;

  case 'mdeg':
    ReadInt (&mdeg, str);
	  return TAG_READ;

  case 'knob':
    { knob.SetGauge(this);
      ReadFrom(&knob,str);
      knob.InitQuad();
	    return TAG_READ;
    }

  case 'flag':
	  return TAG_READ;

  case 'pich':
    ReadMessage (&pich, str);
	  return TAG_READ;

  case 'roll':
    ReadMessage (&roll, str);
	  return TAG_READ;

  case 'copy':
    CopyFrom(str);
    return TAG_READ;
  }

  return CTexturedGauge::Read (str, tag);
}
//----------------------------------------------------------------------
//  Dupplicate from similar gauge
//  NOTE: Knob is not dupplicated as it need absolute coordinates
//----------------------------------------------------------------------
void C_HorizonGauge::CopyFrom(SStream *stream)
{ C_HorizonGauge &src = *(C_HorizonGauge*)LocateGauge(stream);
  CTexturedGauge::CopyFrom(src);
  //--- Dupplicate local items ------------------
  Bfoot.CopyFrom(this,src.Bfoot);
  Bmire.CopyFrom(this,src.Bmire);
  Birim.CopyFrom(this,src.Birim);
  Borim.CopyFrom(this,src.Borim);
  //---------------------------------------------
  hoff  = src.hoff;
  pixd  = src.pixd;
  mdeg  = src.mdeg;
  pich  = src.pich;
  roll  = src.roll;
  return;
}
//----------------------------------------------------------------------
//  Collect VBO data
//----------------------------------------------------------------------
void C_HorizonGauge::CollectVBO(TC_VTAB *vtb)
{	CTexturedGauge::CollectVBO(vtb);
	Bfoot.CollectVBO(vtb);
	Bmire.CollectVBO(vtb);
	Birim.CollectVBO(vtb);
	Borim.CollectVBO(vtb);
	return;
}
//----------------------------------------------------------------------
//  All parameters are read. Init pitch and roll destination
//----------------------------------------------------------------------
void C_HorizonGauge::ReadFinished()
{ if (0 == pich.group) pich.group = mesg.group;
  if (0 == roll.group) roll.group = mesg.group;
  mesg.user.u.datatag = 'knob';
  mesg.id             = MSG_SETDATA;

  //---- Init the ball texture ----------
  Bfoot.SetPPD(pixd);
  //---- Init the mire texture ----------
  Bmire.SetPPD(pixd);
  //---- Init the knob texture ----------
  CTexturedGauge::ReadFinished();
}
//----------------------------------------------------------------------
//	Prepare Messages
//----------------------------------------------------------------------
void C_HorizonGauge::PrepareMsg(CVehicleObject *veh)
{	pich.id				      = MSG_GETDATA;
	pich.dataType	      = TYPE_REAL;
	veh->FindReceiver(&pich);
	pitS	= (CSubsystem*)pich.receiver;
 	roll.id				      = MSG_GETDATA;
	roll.dataType		    = TYPE_REAL;
	veh->FindReceiver(&roll);
	rolS	= (CSubsystem*)roll.receiver;
	CGauge::PrepareMsg(veh);
	return;	
}
//----------------------------------------------------------------------
//  Mouse move over
//-----------------------------------------------------------------------
ECursorResult C_HorizonGauge::MouseMoved (int x, int y)
{ // Send updated mouse position to all click areas
  knob.MouseMoved(x,y);
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//----------------------------------------------------------------
//		Click on Knob, arm rotation
//----------------------------------------------------------------
EClickResult C_HorizonGauge::MouseClick (int x, int y, int btn)
{ return 	knob.ArmRotation(1,x,y,btn);
}

//-----------------------------------------------------------------
//	When click stop disarm rotation
//-----------------------------------------------------------------
EClickResult C_HorizonGauge::StopClick ()
{ return knob.DisarmKnob();
}
//-----------------------------------------------------------------------------
//	JSDEV* ATTITUDE subsystem now gives pitch and roll in degre
//	NOTE:  It is expected from the subsystems that
//				subsystem pitS publishes pitch value to GaugeBusFT01()
//				subsystem rolS publishes roll  value to GaugeBusFT02()
//-----------------------------------------------------------------------------
void C_HorizonGauge::Draw(void)
{ DrawUnderlay();
	// Get Pitch value
	float	pichD = pitS->GaugeBusFT01();						// Should be pitch
	//---- Get Roll value ------------
	float	rollD = rolS->GaugeBusFT02();						// Should be roll
  //--- Draw foot -----------------
	Bfoot.DrawNeedle(rollD,pichD);
  //--- Draw ring  ----------------
  Birim.DrawNeedle(rollD);
  //--- Mire level ----------------
  mesg.id      = MSG_GETDATA;
  Send_Message(&mesg,mveh);
  float  levl  = mesg.intData;
  Bmire.DrawNeedle(0,levl);
  Borim.DrawNeedle(0);
  //--- Draw knob and signal change to subsystem ------
  if (knob.HasChanged())
  { mesg.id      = MSG_SETDATA;
    mesg.intData = knob.GetChange();
    Send_Message(&mesg,mveh);
  }
  //---- Finally draw the knob --------------------
  knob.Draw();
  return;
}
//-----------------------------------------------------------------------------
//	Draw in ambient light
//-----------------------------------------------------------------------------
void C_HorizonGauge::DrawAmbient()
{	return 	DrawOverlay(); }
//=======================================================================
// CPushPullKnobGauge
//  The gauge may operate in vertical or horizontal mode and may
//  have explicit value
//  When no value is defined, the gauge operate in vertical mode
//  and each frame produces a value in the range [0-1] with 0 at
//  bottom pixel
//=======================================================================
C_PushPullKnobGauge::C_PushPullKnobGauge (CPanel *mp)
: CTexturedGauge(mp)
{ Prop.Set(NO_SURFACE);
  lnk1.user.u.datatag = 'lnk1';
  polm.user.u.datatag = 'mpol';
  lowv = 0;
  high = 1;

  cVal    = 0;
  nVal    = 0;
  Area    = 0;
  invt    = 0;
}
//-----------------------------------------------------------
//  Destructor
//-----------------------------------------------------------
C_PushPullKnobGauge::~C_PushPullKnobGauge (void)
{ if (Area) delete [] Area;
}
//-----------------------------------------------------------
//	Prepare messages
//------------------------------------------------------------
void C_PushPullKnobGauge::PrepareMsg(CVehicleObject *veh)
{	mesg.id		  = MSG_SETDATA;
  mesg.sender = unId;
	veh->FindReceiver(&mesg);
	lnk1.id		  = MSG_SETDATA;
  lnk1.sender = unId;
	veh->FindReceiver(&lnk1);
	CGauge::PrepareMsg(veh);
	//---- Initial position ----------------------
  mesg.realData		= Area[cVal].valu;
	mesg.intData		= mesg.realData;
  Send_Message (&mesg,mveh);
	return;
}
//-----------------------------------------------------------
//	Collect VBO data
//------------------------------------------------------------
void C_PushPullKnobGauge::CollectVBO(TC_VTAB *vtb)
{	CTexturedGauge::CollectVBO(vtb);
	return;
}
//------------------------------------------------------------
//  Read parameters
//------------------------------------------------------------
int C_PushPullKnobGauge::Read (SStream *stream, Tag tag)
{ int pm;
  switch (tag) {
    //--- Texture definition ----------------------
    case 'bmap':
      bmap.SetGauge(this);
      bmap.ReadStrip(stream);
      return TAG_READ;
    //--- Upper pixel offset ----------------------
    case 'ytop':
      ReadInt (&pm, stream);
      return TAG_READ;
    //--- Lowar pixel offset     -----------------
    case 'ybtm':
      ReadInt (&pm, stream);
      return TAG_READ;
    //--- Table of values ------------------------
    case 'valu':
      CreateArea(stream);
      return TAG_READ;
    //--- Lower value ---------------------------
    case 'lowv':
      ReadFloat (&lowv, stream);
      return TAG_READ;
    //---- Upper value --------------------------
    case 'high':
      ReadFloat (&high, stream);
      return TAG_READ;
    //--- Obsolete tag -------------------------
    case 'poll':
      return TAG_READ;
		//--- Initial position ---------------------
		case 'ipos':
			ReadInt(&cVal,stream);
			return TAG_READ;
  }
  return CGauge::Read (stream, tag);
}
//---------------------------------------------------------------------
//  Abort
//---------------------------------------------------------------------
void C_PushPullKnobGauge::Err01()
{ gtfo ("CPushPullKnobGauge : <valu> maximum 32 values exceeded in PNL file unId=%s",
  GetUniqueID ());

}
//---------------------------------------------------------------------
//  Create click area
//----------------------------------------------------------------------
void C_PushPullKnobGauge::CreateArea(SStream *s)
{ ReadInt(&nVal, s);
  if (nVal > 32) Err01();
  Area  = new SKnobArea[nVal];
  for (int k=0; k<nVal; k++)
  { ReadInt    (&Area[k].ytop, s);
    ReadInt    (&Area[k].ybtm, s);
    ReadFloat  (&Area[k].valu, s);
    ReadString ( Area[k].help, 64, s);
    Area[k].fram = k;
   }
  return;
}
//---------------------------------------------------------------------
//  Check when bitmap at ratio r
//---------------------------------------------------------------------
bool C_PushPullKnobGauge::AtRatio(char k)
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
void C_PushPullKnobGauge::DefaultArea()
{ int th = bmap.FrameHeight();
  float htr = float(th);
  // define valu data
  nVal = bmap.NbFrames();
  if (nVal > 32) Err01();
  Area  = new SKnobArea[nVal];
  int   htf  = int(htr / nVal);
  for (int k = 0; k < nVal ; k++)
  { float top = (htr * k)  / nVal;
    Area[k].ytop = int(top);
    Area[k].ybtm = int(top) + htf;
    Area[k].valu = 1.0f - (float(k) / nVal);
    Area[k].fram = (nVal - 1 - k);
	 *Area[k].help = 0;
  }
  //---Start at Zero -----------------------
  cVal = nVal - 1;
  invt =  1;
  return;
}
//---------------------------------------------------------------------
//  All parameters are read
//----------------------------------------------------------------------
void C_PushPullKnobGauge::ReadFinished (void)
{ CGauge::ReadFinished ();
  //--- Create default area -------------------------
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
void C_PushPullKnobGauge::LookUpValue(float val)
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
void C_PushPullKnobGauge::DrawAmbient()
{	Send_Message(&polm,mveh);
	float	val;
	if (polm.user.u.datatag == 'indx')	val = polm.index;
	else																val	= polm.realData;
  LookUpValue(val);
  bmap.Draw(Area[cVal].fram);
	return;
}
//---------------------------------------------------------------------
// Increment knob value
//---------------------------------------------------------------------
void C_PushPullKnobGauge::IncValue (void)
{   cVal ++;
    if (cVal >= nVal) cVal = nVal - 1;
    //---- Send message ----------------------
    mesg.realData		= Area[cVal].valu;
		mesg.intData		= mesg.realData;
    Send_Message (&mesg,mveh);
    return;
}
//---------------------------------------------------------------------
// Decrement knob value
//---------------------------------------------------------------------
void C_PushPullKnobGauge::DecValue (void)
{   cVal --;
    if (cVal < 0) cVal = 0;
    //---- Send message ---------------------
    mesg.realData = Area[cVal].valu;
    Send_Message (&mesg,mveh);
    return;
}
//---------------------------------------------------------------------
//  Display help 
//---------------------------------------------------------------------
ECursorResult C_PushPullKnobGauge::DisplayRatio()
{ sprintf_s(hbuf,HELP_SIZE,"%.0f%%",mesg.realData * 100.0f);
  FuiHelp();
  return globals->cum->SetCursor(cursTag);
}
//---------------------------------------------------------------------
//  Mouse moves over
//---------------------------------------------------------------------
ECursorResult C_PushPullKnobGauge::MouseMoved (int mouseX, int mouseY)
{ return globals->cum->SetCursor(cursTag);
}
//---------------------------------------------------------------------
//  Mouse click
//---------------------------------------------------------------------
EClickResult C_PushPullKnobGauge::MouseClick (int mX, int mY, int but)
{ globals->cum->SetCursor(cursTag); 
  if (mY > Area[cVal].ybtm)
  { IncValue();
    return MOUSE_TRACKING_ON;
   } 
  if (mY < Area[cVal].ytop)
  { DecValue();
    return MOUSE_TRACKING_ON;
   }
  return MOUSE_TRACKING_OFF;
}
//---------------------------------------------------------------------
// Stop click
//---------------------------------------------------------------------
EClickResult  C_PushPullKnobGauge::StopClick()
{ return MOUSE_TRACKING_OFF;
}
//============================================================================
// CSimpleSwitch 'swit'
//  JS NOTES:  Some gauges uses  st8t for simple ON/OFF position
//             Multi position must use Indx as tag to change in swst
//============================================================================
C_SimpleSwitch::C_SimpleSwitch (CPanel *mp)
: CTexturedGauge(mp)
{ Prop.Set(NO_SURFACE);
  csru_tag = csrd_tag = 0;
  stat_n     = 0;
  stat       = 0;
  sstr_n     = 0;
  sstr       = 0;
  orie       = 0;
  ordr       = 0;
  cIndx      = 0;
  mmnt       = false;
  mntO       = false;
	cret			 = MOUSE_TRACKING_OFF;
}
//-----------------------------------------------------------------------
//  Free all resources
//-----------------------------------------------------------------------
C_SimpleSwitch::~C_SimpleSwitch (void)
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
void C_SimpleSwitch::PrepareMsg(CVehicleObject *veh)
{	CGauge::PrepareMsg (veh);
  //---Init the subsystem with current position ------
  mesg.intData	= stat[cIndx];
  Send_Message(&mesg,mveh);
	subS = (CSubsystem*)mesg.receiver;
	return;
}
//---------------------------------------------------------------------
//    Read gauge tags
//---------------------------------------------------------------------
int C_SimpleSwitch::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;
  switch (tag) {
    //--- read the texture ----------------------------------
    case 'bmap':
      swit.SetGauge(this);
      swit.ReadStrip(stream);
      return TAG_READ;
    // Cursor down values -----------------------------------
    case 'csrd':
    case 'csrl':
      csrd_tag = BindCursor(stream);
      return TAG_READ;
    // Cursor up -------------------------------------------
    case 'csru':
    case 'csrr':
      csru_tag = BindCursor(stream);
      return TAG_READ;
    //--- State table -------------------------------------
    case 'stat':
      ReadInt (&stat_n, stream);
      if (stat_n > 16) gtfo ("CSimpleSwitch : <stat> maximum 16 states exceeded");
      stat = new int[stat_n];
      for (int i=0; i<stat_n; i++) ReadInt (&stat[i], stream);
      return TAG_READ;
    //--- State Tables -------------------------------------
    case 'sstr':
      ReadInt (&sstr_n, stream);
      if (sstr_n > 16) gtfo ("CSimpleSwitch : <sstr> maximum 16 states exceeded");
      sstr = new char*[sstr_n];
      for (int i=0; i<sstr_n; i++) {sstr[i] = new char[64]; ReadString (sstr[i], 64, stream);}
      return TAG_READ;
    //--- Orientation --------------------------------------
    case 'orie':
      ReadInt (&orie, stream);
      return TAG_READ;
    //--- Order?? -----------------------------------------
    case 'ordr':
      ReadInt (&orie, stream);
      return TAG_READ;
    //--- Starting index ------------------------------------
    case 'indx':
      ReadInt (&cIndx, stream);
      return TAG_READ;
    //--- Momentary switch ----------------------------------
    case 'mmnt':
      mmnt = true;
			cret = MOUSE_TRACKING_ON;
      return TAG_READ;
    //---- ??? ----------------------------------------------
    case 'mntO':
      mntO = true;
      return TAG_READ;
  }
  return CTexturedGauge::Read (stream, tag);
}
//---------------------------------------------------------------------
//  Read terminated
//---------------------------------------------------------------------
void C_SimpleSwitch::ReadFinished (void)
{ CTexturedGauge::ReadFinished ();
  //---  Initialize default state ---------------------
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
	mesg.user.u.datatag = 'indx';
  //----------------------------------------------------
  if (cIndx > stat_n) cIndx = stat_n - 1;
}
//--------------------------------------------------------------------------
//  Collect VBO data
//--------------------------------------------------------------------------
void C_SimpleSwitch::CollectVBO(TC_VTAB *vtb)
{	CTexturedGauge::CollectVBO(vtb);
	swit.CollectVBO(vtb);
	return;
}
//--------------------------------------------------------------------------
//  Draw the switch
//	GAUGE_BUS_IN03 is used to get the switch state
//--------------------------------------------------------------------------
void C_SimpleSwitch::DrawAmbient()
{ cIndx	= subS->GaugeBusIN03();
	swit.Draw(cIndx);
  return;
}
//-----------------------------------------------------------------------
// Increment switch state
//------------------------------------------------------------------------
void C_SimpleSwitch::IncState (void)
{
  // Check upper bound and increment index
  if (cIndx < (stat_n - 1))
  { cIndx++;
    //---- Play sound effect -----------
    globals->snd->Play(sbuf[GAUGE_ON__POS]);
    //---- Send index value  -------------
    mesg.intData	= stat[cIndx];
    if (sstr) DisplayHelp(sstr[cIndx]);
    //---- Send message  ---------------
    return ChangeState();
  }
  return;
}
//----------------------------------------------------------------------
// Decrement switch state
//----------------------------------------------------------------------
void C_SimpleSwitch::DecState (void)
{ // Check upper bound and increment index
  if (cIndx > 0)
  { cIndx--;
    //---- Play sound effect ----------
    globals->snd->Play(sbuf[GAUGE_OFF_POS]);
    //---- Send index value- ----------
    mesg.intData	= stat[cIndx];
    if (sstr) DisplayHelp(sstr[cIndx]);
    //---- Send message ---------------
    return ChangeState(); 
  }
  return;
}
//----------------------------------------------------------------------
// Update switch state
//----------------------------------------------------------------------
void C_SimpleSwitch::ChangeState()
{ //--- advise subsystem --------------
	mesg.index		= cIndx;
	mesg.intData	= (cIndx != 0);
	Send_Message (&mesg,mveh); 
	return;
}
//----------------------------------------------------------------------
//  Mouse move
//----------------------------------------------------------------------
ECursorResult C_SimpleSwitch::MouseMoved (int mouseX, int mouseY)
{ DisplayHelp(help);
  switch (orie) {
  case 0:
  //------- Default, vertical orientation -------
  case 1:
	case 3:
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
EClickResult C_SimpleSwitch::MouseClick (int mouseX, int mouseY, int buttons)
{ switch (orie) {
  case 0:
  case 1:
    // Default, vertical orientation
    (mouseY < cy)?(IncState ()):(DecState ());
    return cret;

  case 2:
    // Horizontal orientation
    (mouseX < cx)?(DecState ()):(IncState ());
    return cret;
	case 3:
    // Inverted vertical orientation 
    (mouseY < cy)?(DecState ()):(IncState ());
    return cret;
  }
  return cret;
}
//---------------------------------------------------------------
//  Mouse Stop Click:  Called only if <mmnt> is defined
//--------------------------------------------------------------
EClickResult  C_SimpleSwitch::StopClick ()
{	//--- Check if last index -------------------------
	if (cIndx < (stat_n - 1))	return MOUSE_TRACKING_OFF;
	//--- Back to previous state ----------------------
	cIndx--;
  //---- Play sound effect ----------
  globals->snd->Play(sbuf[GAUGE_OFF_POS]);
  //---- Send index value- ----------
  mesg.intData	= stat[cIndx];
	ChangeState();
	return MOUSE_TRACKING_OFF;
}
//======================================================================
// CHobbsMeterGauge
//======================================================================
C_HobbsMeterGauge::C_HobbsMeterGauge(CPanel *mp)
: CTexturedGauge(mp)
{ Prop.Rep(0);
  val   = 0;
  wCar  = 8;
}
//-------------------------------------------------------------
//  Destructor
//-------------------------------------------------------------
C_HobbsMeterGauge::~C_HobbsMeterGauge()
{}
//-------------------------------------------------------------
//  Collect VBO data:
//	NOTE: This gauge
//-------------------------------------------------------------
void C_HobbsMeterGauge::CollectVBO(TC_VTAB *vtb)
{	CTexturedGauge::CollectVBO(vtb);
	digt.CollectVBO(vtb);
	return;	}
//-------------------------------------------------------------
//  Read a digit parameters
//-------------------------------------------------------------
void C_HobbsMeterGauge::ReadDIGK(SStream *str, CDigit *dg)
{ dg->SetGauge(this);
  ReadFrom(dg,str);
  if (dg->GetType() == 0) dg->SetTexture(&idig);
  if (dg->GetType() == 1) dg->SetTexture(&pdig);
}

//-------------------------------------------------------------
//  Read Hobbs tags
//-------------------------------------------------------------
int C_HobbsMeterGauge::Read (SStream *str, Tag tag)
{ switch (tag) {
    //--- Black Digit  ------------
    case 'idig':
      ReadStrip(str,idig);
      wCar  = idig.wd;
      return TAG_READ;
    //--- White digit  ------------
    case 'pdig':
      ReadStrip(str,pdig);
      pdig.pm = pdig.ht * 10;
		  return TAG_READ;
    //--- Digit 5 -----------------
    case 'part':
      ReadDIGK(str,&digt);
      return TAG_READ;
  }
  //------------------------------------------------------------
  return  CTexturedGauge::Read (str, tag);
}
//-------------------------------------------------------------
//  All parameters are read
//-------------------------------------------------------------
void C_HobbsMeterGauge::ReadFinished()
{ CTexturedGauge::ReadFinished();
  mesg.id = MSG_GETDATA;
  mesg.dataType = TYPE_REAL;
  //--- Allocate a surface for 4 full digits -----
  int sw = idig.wd * 4;
  int sh = idig.ht + 2;
  SetSurface(sw,sh);
  return;
}

//-------------------------------------------------------------
//  Draw
//-------------------------------------------------------------
void C_HobbsMeterGauge::Draw()
{ //  get the hobbs value here
  Send_Message (&mesg,mveh);
  float hobv = mesg.realData;
  int d0  = int(hobv * 0.001);
  int tp  = d0 * 1000; 
  int d1  = int((hobv - tp) * 0.01); 
      tp  += d1 * 100;  
  int d2  = int((hobv - tp) * 0.1); 
      tp  += d2 * 10; 
  int d3   = hobv - tp; // 7;
  //-- Compute partial part ---------------
      tp  += d3;
  float pr = hobv - float(tp);
  //--- Draw on Surface -------------------
  int cn = 0;
  DrawTextureOn(idig,*surf, cn,1,d0);
  cn  += wCar;
  DrawTextureOn(idig,*surf,cn,1,d1);
  cn  += wCar;
  DrawTextureOn(idig,*surf,cn,1,d2);
  cn  += wCar;
  DrawTextureOn(idig,*surf,cn,1,d3);
  RenderSurface(quad,surf);
  //--- Draw partial part -----------------
  digt.DrawPartial(pr);
  return;
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
C_BasicMagnetoSwitch::C_BasicMagnetoSwitch (CPanel *mp)
: C_SimpleSwitch(mp)
{ Prop.Set(NO_SURFACE);
}
//------------------------------------------------------------
//	Prepare messages
//------------------------------------------------------------
void C_BasicMagnetoSwitch::PrepareMsg(CVehicleObject *veh)
{	mesg.id		      = MSG_GETDATA;
  mesg.dataType		= TYPE_INT;
  mesg.sender     = unId;
  mesg.user.u.hw  = HW_SWITCH;
	veh->FindReceiver(&mesg);
	subS	= (CSubsystem*)mesg.receiver;
	mesg.id		      = MSG_SETDATA;
	C_SimpleSwitch::PrepareMsg(veh);
	return;
}
//-------------------------------------------------------------------------------
//  Collect VBO data
//-------------------------------------------------------------------------------
void C_BasicMagnetoSwitch::CollectVBO(TC_VTAB *vtb)
{	C_SimpleSwitch::CollectVBO(vtb);
	return;
}
//-------------------------------------------------------------------------------
//  Draw:  Update state only
//	Gauge Bus IN03 is used to gget the index value (see subsystem.h)
//-------------------------------------------------------------------------------
void C_BasicMagnetoSwitch::Draw()
{	cIndx	= subS->GaugeBusIN03();
	C_SimpleSwitch::Draw();
	return;
}
//-------------------------------------------------------------------------------
//  Mouse move over
//-------------------------------------------------------------------------------
ECursorResult C_BasicMagnetoSwitch::MouseMoved (int x, int y)
{ globals->cum->SetCursor(cursTag);
  return  CURSOR_WAS_CHANGED;}
//-------------------------------------------------------------------------------
//  Mouse Click
//  NOTE:  Prevent forward position after the last frame
//        Some start key uses only 4 positions.  The last one is the start position
//        
//---------------------------------------------------------------------------------
EClickResult C_BasicMagnetoSwitch::MouseClick (int mouseX, int mouseY, int buttons)
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
  if (cIndx == swit.GetHiFrame()) 
      { rc  = MOUSE_TRACKING_ON;
        cIndx = MAGNETO_SWITCH_START;
      }
  ChangeState();
  return rc;
}
//----------------------------------------------------------------------------
//  Stop Click
//-----------------------------------------------------------------------------
EClickResult C_BasicMagnetoSwitch::StopClick()
{
  // The only time this method should be called is when the switch is in the
  //   START state and the right mouse button is released.  Switch back
  //   to previous state
  cIndx -= 1;
  globals->snd->Play(sbuf[GAUGE_OFF_POS]);
  // Update magneto subsystem
	ChangeState();
  return MOUSE_TRACKING_OFF;
}
//================================================================================
// CBasicBatterySwitch
//================================================================================
C_BasicBatterySwitch::C_BasicBatterySwitch (CPanel *mp)
: C_SimpleSwitch(mp)
{ Prop.Set(NO_SURFACE);
  altt = batt = 0;
  sAlt = sBat = 0;
}
//------------------------------------------------------------
//	Prepare messages
//------------------------------------------------------------
void C_BasicBatterySwitch::PrepareMsg(CVehicleObject *veh)
{	//--- Locate subsysteem batterie -----
	mbat.id		      = MSG_GETDATA;
  mbat.sender     = unId;
  mbat.user.u.hw  = HW_SWITCH;
  mbat.dataType       = TYPE_INT;
  mbat.user.u.datatag = 'stat';
	veh->FindReceiver(&mbat);
	mbat.id		      = MSG_SETDATA;
	batS = (CSubsystem*)mbat.receiver;
	//--- Get alternator -------------------
	malt.id		      = MSG_GETDATA;
  malt.sender     = unId;
  malt.user.u.hw  = HW_SWITCH;
  malt.dataType       = TYPE_INT;
  malt.user.u.datatag = 'stat';
	veh->FindReceiver(&malt);
	malt.id		      = MSG_SETDATA;
	altS = (CSubsystem*)malt.receiver;
	//---------------------------------------
	C_SimpleSwitch::PrepareMsg(veh);
	return;
}
//------------------------------------------------------------
//	Collect VBO data
//------------------------------------------------------------
void C_BasicBatterySwitch::CollectVBO(TC_VTAB *vtb)
{	C_SimpleSwitch::CollectVBO(vtb);
	return;
}
//-----------------------------------------------------------
//  Read all tags
//-----------------------------------------------------------
int C_BasicBatterySwitch::Read (SStream *stream, Tag tag)
{
  switch (tag) {
  case 'altt':
    ReadTag (&altt, stream);
    malt.group = altt;
    return TAG_READ;

  case 'batt':
    ReadTag (&batt, stream);
    mbat.group = batt;
    return TAG_READ;
  }
  return C_SimpleSwitch::Read (stream, tag);
}
//---------------------------------------------------------------
//  All parameters are read. Init messages
//---------------------------------------------------------------
void  C_BasicBatterySwitch::ReadFinished()
{ C_SimpleSwitch::ReadFinished();
  return;
}
//---------------------------------------------------------------
//  Draw the gauge
//---------------------------------------------------------------
void C_BasicBatterySwitch::DrawAmbient()
{ sAlt	= altS->GaugeBusIN01();
	sBat	= batS->GaugeBusIN01();
	// Calculate frame number:
  //   0 = Both OFF
  //   1 = Alternator ON, Battery OFF
  //   2 = Alternator OFF, Battery ON
  //   3 = Both ON
  int frame = sAlt + sBat + sBat;
  // Draw appropriate bitmap frame
  swit.Draw(frame);
  return;
}
//------------------------------------------------------------------------------
//  Mouse click
//------------------------------------------------------------------------------
EClickResult C_BasicBatterySwitch::MouseClick (int mouseX, int mouseY, int buttons)
{  EClickResult rc = MOUSE_TRACKING_OFF;
  char mod = 0;
  char bat = sBat;
  char alt = sAlt;
	 // Left half of the switch controls the alternator
  if (mouseX < cx)  sAlt  = (mouseY < (cy))?(1):(0);
	// Right half of the switch controls the battery
  if (mouseX > cx)  sBat  = (mouseY < (cy))?(1):(0);
	//--- Alternator from off to on implies battery on ---
	sBat |= (sAlt==1) && (alt == 0);
	//--- Any change produce a sound ---------------------
  mod = ((sAlt ^ alt) | (sBat ^ bat));
    // Play sound effect if alternator state has changed
  if (mod) globals->snd->Play(sbuf[mod]);
	//--- Update alternator state -------------------------
  malt.intData  = int(sAlt);
  Send_Message (&malt,mveh);
	// Update battery and alternator subsystems
  mbat.intData  = int(sBat);
  Send_Message (&mbat,mveh);
  return rc;
}

//=========================================================================
//  CAmmeterGauge gauge
//=========================================================================
void C_AmmeterGauge::Draw()
{ C1NeedleGauge::Draw();           // Draw gauge
  return; }
//-------------------------------------------------------------
//	Draw in ambient light
//-------------------------------------------------------------
void C_AmmeterGauge::DrawAmbient()
{	return DrawOverlay(); }
//-------------------------------------------------------------
//	Collect VBO data
//-------------------------------------------------------------
void C_AmmeterGauge::CollectVBO(TC_VTAB *vtb)
{	C1NeedleGauge::CollectVBO(vtb);
	return; }
//=========================================================================
//  C_VacuumGauge gauge
//=========================================================================
void C_VacuumGauge::Draw()
{ C1NeedleGauge::Draw();           // Draw gauge
  return;
}
//-------------------------------------------------------------
//	Draw in ambient light
//-------------------------------------------------------------
void C_VacuumGauge::DrawAmbient()
{	return DrawOverlay(); }
//-------------------------------------------------------------
//	Collect VBO data
//-------------------------------------------------------------
void C_VacuumGauge::CollectVBO(TC_VTAB *vtb)
{	C1NeedleGauge::CollectVBO(vtb);
	return; }

//=========================================================================
// JSDEV* modified CTurnCoordinatorGauge
//  NOTE:  Tilt is the inclination ratio:  20° for a 3° per second turn
//          (a complete 360° turn in 2 minutes)
//=========================================================================
C_TurnCoordinatorGauge::C_TurnCoordinatorGauge (CPanel *mp)
: C1NeedleGauge(mp)
{	Prop.Set(NO_SURFACE);
  pcon	= bcon = 0;
}
//------------------------------------------------------------------------
C_TurnCoordinatorGauge::~C_TurnCoordinatorGauge (void)
{ }
//-------------------------------------------------------------------------
//	Read all tags
//-------------------------------------------------------------------------
int C_TurnCoordinatorGauge::Read (SStream *stream, Tag tag)
{ 
  switch (tag) {
    //--- Plane needle ------------------------------
    case 'plan':
      nedl.SetGauge(this);
		  ReadFrom(&nedl,stream);
		  return TAG_READ;
    //--- Ball strip ---------------------------------
    case 'ball':
      ball.SetGauge(this);
      ReadFrom(&ball,stream);
		  return TAG_READ;
    //--- subsystem tag -----------------------------
    case 'pcon':
		  ReadTag (&pcon, stream);
		  return TAG_READ;
    //--- Subsystem tag ----------------------------
    case 'bcon':
		  ReadTag (&bcon, stream);
		  return TAG_READ;
    //--- copy from similar gauge -------------------
    case 'copy':
      CopyFrom(stream);
      return TAG_READ;
  }
  return C1NeedleGauge::Read (stream, tag);
}
//-------------------------------------------------------------------------
//	Read all tags
//  The ball ratio is derived from the number of frames in the texture strip
//-------------------------------------------------------------------------
void  C_TurnCoordinatorGauge::ReadFinished()
{ C1NeedleGauge::ReadFinished();
  int nbf = ball.NbFrames();
  rbal    = (2 * float(TWO_PI) * 128) / nbf;
  mbal    = (nbf >> 1);                 // Middle frame
  tilt    = float(20) / 3;
  return;
}
//-------------------------------------------------------
//	Prepare message
//-------------------------------------------------------
void C_TurnCoordinatorGauge::PrepareMsg(CVehicleObject *veh)
{ //--- Locate indicator subsystem ---------
	mesg.sender     = unId;
	veh->FindReceiver(&mesg);
	trnS = (CSubsystem*)mesg.receiver;
	CGauge::PrepareMsg(veh);
	return;
}
//-------------------------------------------------------------------------
//	Copy from similar gauge
//-------------------------------------------------------------------------
void C_TurnCoordinatorGauge::CopyFrom(SStream *stream)
{ C_TurnCoordinatorGauge &src = *(C_TurnCoordinatorGauge*)LocateGauge(stream);
  CTexturedGauge::CopyFrom(src);
  //--- Copy local parameters ----------------
  nedl.CopyFrom(this,src.nedl);
  pcon  = src.pcon;
  bcon  = src.bcon;
  tilt  = src.tilt;
  rbal  = src.rbal;
  mbal  = src.mbal;
}
//-------------------------------------------------------------
//	Collect VBO data
//-------------------------------------------------------------
void C_TurnCoordinatorGauge::CollectVBO(TC_VTAB *vtb)
{	C1NeedleGauge::CollectVBO(vtb);
	ball.CollectVBO(vtb);
	return; }

//--------------------------------------------------------------------
//	JSDEV*  Read the subsystem and compute bitmap frame
//  NOTES:
//    rateD is the turn rate in degre per second.  A 2 minute turn
//          is a 3° per second turn at about 20° of inclination
//    ball  is the inertial angular acceleration in rad per sec
//          It is clamped in the [-2rad, +2rad] range
//    ball has been changed as part of body acceleration y / z in LH // LC
//	NOTE:   It is expected from the turn subsystem that:
//					ball value is ppublished from GaugeBusFT01();
//					needle is published from GaugeBusFT02()
//--------------------------------------------------------------------
void C_TurnCoordinatorGauge::Draw(void)
{ //--- Get ball value ----------------
	//mesg.user.u.datatag = bcon;
	//Send_Message (&mesg);
	//float acc =  float(mesg.realData);
	float acc = trnS->GaugeBusFT01();
  //--- Draw ball ---------------------
  //int frm  = mbal + int(rbal * acc);
  int frm  = mbal + int(float(mbal) * acc * 0.5f); // 0.16
  ball.Draw(frm);    
  //--- Get plane value ---------------
	//mesg.user.u.datatag = pcon;
	//Send_Message (&mesg);
	//float rateD = (float)mesg.realData;
	float rateD = trnS->GaugeBusFT02();
  //--- Clamp value to [-20°,+20°] ----
  float deg  = ClampTo(20,(rateD * tilt)) ;
  nedl.DrawNeedle(deg);
  DrawOverlay();
  return;
}
//=================================================================================
// C_FlapsSwitchGauge:  Just a C_PushPullKnobGauge
//
//=================================================================================
C_FlapsSwitchGauge::C_FlapsSwitchGauge (CPanel *mp)
: C_PushPullKnobGauge(mp)
{ Prop.Set(NO_SURFACE);
  mesg.user.u.datatag = 101; ///< send data to CAeroControl::ReceiveMessage
}
//------------------------------------------------------------------
//  Read the parameters
//------------------------------------------------------------------
int C_FlapsSwitchGauge::Read (SStream *stream, Tag tag)
{
  return C_PushPullKnobGauge::Read (stream, tag);
}
//-------------------------------------------------------------
//	Collect VBO data
//-------------------------------------------------------------
void C_FlapsSwitchGauge::CollectVBO(TC_VTAB *vtb)
{	C_PushPullKnobGauge::CollectVBO(vtb);
	return; }

//=================================================================================
//	CFlyhawkAnnunciatorTest
//	JSDEV*  This is a special switch for FlyHawk
//	pos 0	Set the test mode in the ann panel
//	pos 1	Set Bright mode
//	pos 2	Set Dim mode
//=================================================================================
C_FlyhawkAnnunciatorTest::C_FlyhawkAnnunciatorTest (CPanel *mp)
: CTexturedGauge(mp)
{ Prop.Set(NO_SURFACE);
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
void C_FlyhawkAnnunciatorTest::InitMsg(SMessage *msg,Tag tag)
{	msg->group			  = tag;
  msg->sender       = unId;
	msg->id				    = MSG_SETDATA;
	msg->dataType		  = TYPE_INT;
	msg->user.u.hw		= HW_SWITCH;
	msg->user.u.datatag	= 'stat';
	return;	
}
//---------------------------------------------------------------
C_FlyhawkAnnunciatorTest::~C_FlyhawkAnnunciatorTest (void)
{
}
//----------------------------------------------------------------
//	Read All Tags
//----------------------------------------------------------------
int C_FlyhawkAnnunciatorTest::Read (SStream *stream, Tag tag)
{
  switch (tag) {
  case 'bmap':
    butn.SetGauge(this);
    butn.ReadStrip(stream);
    return TAG_READ;

  case 'csru':
    csru_tag = BindCursor(stream);
    return TAG_READ;

  case 'csrd':
    csrd_tag = BindCursor(stream);
    return TAG_READ;
  }
  return CTexturedGauge::Read (stream, tag);
 }
//--------------------------------------------------------------
//  All parameters are read
//---------------------------------------------------------------
void C_FlyhawkAnnunciatorTest::ReadFinished (void)
{ CTexturedGauge::ReadFinished ();
  return;
}
//-------------------------------------------------------------
//	Collect VBO data
//-------------------------------------------------------------
void C_FlyhawkAnnunciatorTest::CollectVBO(TC_VTAB *vtb)
{	CTexturedGauge::CollectVBO(vtb);
	butn.CollectVBO(vtb);
	return; }
//------------------------------------------------------------
// JSDEV* Draw in ambient light
//------------------------------------------------------------
void C_FlyhawkAnnunciatorTest::DrawAmbient()
{	butn.Draw(cPos);
	return;	}
//------------------------------------------------------------
//	JSDEV* Update the components
//------------------------------------------------------------
void C_FlyhawkAnnunciatorTest::UpdateComponent(char old,char now)
{	SMessage	*msg = &msgT[old];
  globals->snd->Play(sbuf[GAUGE_ON__POS]);
	//---reset previous component -----------------
	msg->intData	= 0;
	Send_Message(msg,mveh);
	//---set the new component --------------------
	msg	= &msgT[now];
	msg->intData	= 1;
	Send_Message(msg,mveh);
	return;
}
//------------------------------------------------------------
//	JSDEV* Increment switch state
//------------------------------------------------------------
void C_FlyhawkAnnunciatorTest::IncState (void)
{	char old	= cPos;						// Current position
	cPos		= IncT[cPos];				// New position
	// Update subsystem 
	if (cPos == old)		return;			// No change
	UpdateComponent(old,cPos);
	return;
}

//------------------------------------------------------------
// JSDEV* Decrement switch state
//------------------------------------------------------------
void C_FlyhawkAnnunciatorTest::DecState (void)
{	char old	= cPos;						// Current position
	cPos		= DecT[cPos];				// new position
	if (cPos == old)		return;			// No change
	UpdateComponent(old,cPos);
	return;
  }
//-------------------------------------------------------------
//	Move over the button
//-------------------------------------------------------------
ECursorResult C_FlyhawkAnnunciatorTest::MouseMoved (int mouseX, int mouseY)
{ // Default, vertical orientation
  if (mouseY < cy) return globals->cum->SetCursor (csru_tag);
  if (mouseY > cy) return globals->cum->SetCursor (csrd_tag);
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//------------------------------------------------------------------------------------
//	Release the TEST button
//------------------------------------------------------------------------------------
EClickResult C_FlyhawkAnnunciatorTest::StopClick()
{	if (cPos == TEST)	IncState();
	return MOUSE_TRACKING_OFF;}
//------------------------------------------------------------------------------------
//	Click over the button
//------------------------------------------------------------------------------------
EClickResult C_FlyhawkAnnunciatorTest::MouseClick (int mouseX, int mouseY, int buttons)
{ // Default, vertical orientation
    if (mouseY < (cy)) {
      // Top half of the gauge increments the state
      DecState ();
    } else {
      // Bottom half of the gauge decrements the state
      IncState ();
    }
  return MOUSE_TRACKING_ON;
}
//==================================================================
// CFlyhawkElevatorTrimGauge
//==================================================================
CElevatorTrimGauge::CElevatorTrimGauge (CPanel *mp)
: CTexturedGauge(mp)
{ Prop.Set(NO_SURFACE);
  wRatio  = 0.1f;
}
//------------------------------------------------------------------
//  Destructor
//------------------------------------------------------------------
CElevatorTrimGauge::~CElevatorTrimGauge (void)
{
}
//------------------------------------------------------------------
//  Read the gauge tags
//------------------------------------------------------------------
int CElevatorTrimGauge::Read (SStream *stream, Tag tag)
{ int pm;
  switch (tag) {
    //-- Trim wheel bitmap -------------------
    case 'tour':
      ReadInt(&pm,stream);
      wRatio = float (pm) * 0.5;
      return TAG_READ;
    case 'trmw':
      wheel.SetGauge(this);
      ReadFrom(&wheel,stream);
      return TAG_READ;
    //--- Trim indicator --------------------
    case 'trmi':
      indwh.SetGauge(this);
      ReadFrom(&indwh,stream);
      return TAG_READ;
    //--- Cclick surfaces down -------------
    case 'down':
      ReadFrom (&down, stream);
      return TAG_READ;
    //--- Click surface up ----------------
    case 'up  ':
      ReadFrom (&caup, stream);
      return TAG_READ;
  }
  return CTexturedGauge::Read (stream, tag);
}
//-----------------------------------------------------------------
//  All tags read
//  NOTE:  ratio is the number of frame per deflection
//          There are K frames for deflection in [-1,+1]
//          Thus R = K / 2;
//-----------------------------------------------------------------
void CElevatorTrimGauge::ReadFinished (void)
{ CTexturedGauge::ReadFinished ();
  mesg.id = MSG_SETDATA;
  mesg.dataType = TYPE_REAL;
  //----Compute frame per deflection --------------
  iRatio = float(indwh.NbFrames()) / 2;
  mIndwh = (indwh.NbFrames() >> 1);
  return;
}
//-------------------------------------------------------------
//	Collect VBO data
//-------------------------------------------------------------
void CElevatorTrimGauge::CollectVBO(TC_VTAB *vtb)
{	CTexturedGauge::CollectVBO(vtb);
	wheel.CollectVBO(vtb);
	indwh.CollectVBO(vtb);
	down.CollectVBO(vtb);
	caup.CollectVBO(vtb);
	return; }

//---------------------------------------------------------------------
//  Mouse move over
//---------------------------------------------------------------------
ECursorResult CElevatorTrimGauge::MouseMoved (int x, int y)
{ // Send updated mouse position to all click areas
  down.MouseMoved (x, y);
  caup.MouseMoved   (x, y);
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//----------------------------------------------------------------------
//  Click gauge change trim value
//----------------------------------------------------------------------
EClickResult CElevatorTrimGauge::MouseClick (int x, int y, int bt)
{ // Only handle left/right mouse clicks
  if ((bt & (MOUSE_BUTTON_LEFT | MOUSE_BUTTON_RIGHT)) == 0)   return MOUSE_TRACKING_OFF;
  if (down.IsHit(x, y)) {
    mesg.id = MSG_SETDATA;
    mesg.user.u.datatag = 'decr';
    Send_Message (&mesg,mveh);
    ArmRepeat(0.1f,x,y,bt);
    return MOUSE_TRACKING_ON;
  }
  if (caup.IsHit(x, y)) {
    mesg.id = MSG_SETDATA;
    mesg.user.u.datatag = 'incr';
    Send_Message (&mesg,mveh);
    ArmRepeat(0.1f,x,y,bt);
    return MOUSE_TRACKING_ON;
  }
  return MOUSE_TRACKING_OFF;
}
//------------------------------------------------------------------------------
// Stop click gauge
//------------------------------------------------------------------------------
EClickResult CElevatorTrimGauge::StopClick()
{ Disarm();
  return MOUSE_TRACKING_OFF;
}
//------------------------------------------------------------------------------
//  Draw the gauge
//------------------------------------------------------------------------------
void CElevatorTrimGauge::DrawAmbient(void)
{ // Draw Gauge 
  mesg.id = MSG_GETDATA;
  mesg.user.u.datatag = 'rawv';
  Send_Message (&mesg,mveh);
  float df = float(mesg.realData);
  int   wf = Round(wRatio * (df + 1));
  int   nf = wf % wheel.NbFrames();
  nf       = wheel.NbFrames() - 1 -nf;
  wheel.Draw(nf);
  int   rf = mIndwh + Round(iRatio * df);
  indwh.Draw(rf);
	vReal		= df;
  RepeatClick();
	return;
}
//====================================================================
// JSDEV* modified CBasicCompassGauge
//====================================================================
CBasicCompassGauge::CBasicCompassGauge (CPanel *mp)
: CBitmapGauge(mp)
{
  wind	= 0;
  orgx	= 0;
  nton	= 0;
  rDeg	= 0;
  midP	= 0;
  mesg.id	= MSG_GETDATA;
}
//----------------------------------------------------------------
CBasicCompassGauge::~CBasicCompassGauge (void)
{
}
//-----------------------------------------------------------------
//  Read all tags
//------------------------------------------------------------------
int CBasicCompassGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
	case 'comp':
    comps.Read(stream,0);
		return TAG_READ;

	case 'wind':
		ReadInt (&wind, stream);
		return TAG_READ;

	case 'orgx':
		ReadInt (&orgx, stream);
		return TAG_READ;

  case 'nton':
		ReadInt (&nton, stream);
		rDeg = float(nton) / 360.0f;
		return TAG_READ;
  }

  return CBitmapGauge::Read (stream, tag);
  }
//---------------------------------------------------------------
void CBasicCompassGauge::ReadFinished (void)
{ midP	= orgx - (wind / 2);
  CBitmapGauge::ReadFinished ();
}
//-------------------------------------------------------------
//	Collect VBO data:
//	This old gauge is temporarily out of VBO
//-------------------------------------------------------------
void CBasicCompassGauge::CollectVBO(TC_VTAB *vtb)
{	return; }
//-----------------------------------------------------------------
//	JSDEV* Draw the strip
//	The strip bitmap is like this
//	60°-----N----------------------------N------330°
//	with nton pixels from N to N
//	return value from mcmp subsystem is in [0,360] in degres
//	Note that timK correction int mcmp can overshoot this range
//		   value is clamped inside this range
//-----------------------------------------------------------------
void CBasicCompassGauge::Draw(void)
{	//--- Get heading value ---------------------
	Send_Message (&mesg,mveh);
  cdir = int (mesg.realData);    //mesg.user.u.unit;
	//--- Draw Gauge ----------------------------
	int dev = int(rDeg * mesg.user.u.unit);
	int pix = dev + midP;
  // Draw only if required
	EraseSurfaceRGBA(surf,0);
  under.Draw(surf);
	comps.Draw(surf, -pix, 0, 0);
	overl.Draw(surf);
  RenderSurface(quad,surf);
  return;
}
//-------------------------------------------------------------------
//	Mouse move over
//-------------------------------------------------------------------
void CBasicCompassGauge::DisplayHelp()
{	if (0 == *help)     return;
  sprintf_s(hbuf,HELP_SIZE,"Compas %03d°",cdir);
  FuiHelp();
  return;
}
//===============================================================================
// CSimpleInOutStateSwitch
//===============================================================================
C_SimpleInOutStateSwitch::C_SimpleInOutStateSwitch (CPanel *mp)
:CGauge(mp)
{ Prop.Set(NO_SURFACE);
	panel	=		mp;
  type = GAUGE_SIMPLE_IN_OUT;
  vin[1] = 1;
  vin[0] = 0;
  gpos   = 0;
  mack   = 0;
 *onht	 = 0;
 *ofht	 = 0;
 *sstr_off = 0;
 *sstr_on	 = 0;
  mmnt  = 0;
  time  = 0;
}
//---------------------------------------------------------------------
//  Destroy it
//---------------------------------------------------------------------
C_SimpleInOutStateSwitch::~C_SimpleInOutStateSwitch (void)
{}
//---------------------------------------------------------------------
//  Read gauge tags
//---------------------------------------------------------------------
int C_SimpleInOutStateSwitch::Read (SStream *stream, Tag tag)
{ int nbr;
  int rc = TAG_IGNORED;
  switch (tag) {
  case 'sbmp':
  case 'bmap':
    stsw.SetGauge(this);
    stsw.ReadStrip(stream);
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
	case 'init':
    ReadInt (&nbr, stream);
		gpos	= nbr;
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
    //--- must be acknoledged --------------------
    mack = 1;
    return TAG_READ;
		//--- Cursor up -(0 position) ----------------
	case 'csru':
		csr[0] = BindCursor(stream);
    return TAG_READ;
		//--- Cursor dn -(1 position) ----------------
	case 'csrd':
		csr[1] = BindCursor(stream);
    return TAG_READ;

  }
  return CGauge::Read (stream, tag);
}
//-----------------------------------------------------------------------------
//  Read finished
//-----------------------------------------------------------------------------
void C_SimpleInOutStateSwitch::ReadFinished (void)
{ CGauge::ReadFinished ();
  //---Init message values -----------------------------
  //---- Send update message to dependent subsystem
  return;
}
//-----------------------------------------------------------------------------
//  Send message to init the subsystem
//-----------------------------------------------------------------------------
void  C_SimpleInOutStateSwitch::PrepareMsg(CVehicleObject *veh)
{ mesg.id       = MSG_SETDATA;
  mesg.dataType = TYPE_INT;
  mesg.intData  = vin[gpos];
  Send_Message (&mesg,mveh);
	subS = (CSubsystem*)mesg.receiver;
	CGauge::PrepareMsg(veh);
  return;
}
//-----------------------------------------------------------------------------
//  Collect VBO data
//-----------------------------------------------------------------------------
void C_SimpleInOutStateSwitch::CollectVBO(TC_VTAB *vtb)
{	CGauge::CollectVBO(vtb);
	stsw.CollectVBO(vtb);
	return;
}
//-----------------------------------------------------------------------------
//  State changed from subsystem
//-----------------------------------------------------------------------------
void C_SimpleInOutStateSwitch::SubsystemCall(CSubsystem *sys,int val)
{	gpos = (val == vin[0])?(0):(1);
	return;
}
//-----------------------------------------------------------------------------
//  Draw the gauge
//-----------------------------------------------------------------------------
void C_SimpleInOutStateSwitch::Draw(char val)
{	//---- Update state -----------------------
	gpos = (val == vin[0])?(0):(1);
	stsw.Draw(gpos);
  if (0 == gpos)      return;
  //---Check for time out -------------------
  if (0 == mmnt)      return;
  if (0 == time)      return;
  time--;
  if (time)           return;
  gpos = 0; 
  return;
  }
//-----------------------------------------------------------------------------
//  Draw the gauge
//-----------------------------------------------------------------------------
void C_SimpleInOutStateSwitch::DrawAmbient()
{	//---- Update state -----------------------
	char val	= subS->GaugeBusIN01();
	Draw(val);
  return;
  }

//-------------------------------------------------------------------------------------
//  Mouse click over
//-------------------------------------------------------------------------------------
EClickResult C_SimpleInOutStateSwitch::MouseClick (int mouseX, int mouseY, int buttons)
{ //--- Update subsystem state ----------------
  gpos ^= 1;
  mesg.intData  = vin[gpos];
  Send_Message (&mesg,mveh);
  //---- Play sound effect --------------------
  globals->snd->Play(sbuf[gpos]);
  //--- Arm timer for momentary contact -------
  if (gpos)  time = mmnt;
  return MOUSE_TRACKING_OFF;
}
//===============================================================================
// CGenericSwitch
//  This switch defines the following components
//  1) A multiframe bitmap
//  2) A list of one or more messages
//  3) A list of click areas inside the gauge. To each click area corresponds
//     an action.
//  4) Each action may specify one or more message to activate, the value to send
//     and the frame to display
//===============================================================================
CGenericSwitch::CGenericSwitch(CPanel *mp)
: CTexturedGauge(mp)
{ Prop.Set(NO_SURFACE);
  dfa = 0;
  nfr = 0;
}
//------------------------------------------------------------------
//  Destroy all resources
//------------------------------------------------------------------
CGenericSwitch::~CGenericSwitch()
{ std::map<int,CGaugeClickArea *>::iterator ita;
  for (ita=vkca.begin(); ita != vkca.end(); ita++)  delete (*ita).second;
  std::map<int,SMessage *>::iterator        itm;
  for (itm=vmsg.begin(); itm != vmsg.end(); itm++)  delete (*itm).second;
  std::map<int,CGaugeAction *>::iterator    itg;
  for (itg=vact.begin(); itg != vact.end(); itg++)  delete (*itg).second;
  //----------------------------------------------------------
  vkca.clear();
  vmsg.clear();
  vact.clear();
}
//------------------------------------------------------------------
//  Read all parameters
//------------------------------------------------------------------
int CGenericSwitch::Read(SStream *st, Tag tag)
{ int ica = 0;
  //--- Check for bitmap -----------------------
  if ('bmap' == tag)
  { bmap.SetGauge(this);
    bmap.ReadStrip(st);
    return TAG_READ;
  }
  //--- Decode click area ----<CAnn> ------------
  int typ = ClickAreaFromTag (tag,&ica);
  if (ica > 0)
  { // Valid click area---
    CGaugeClickArea *ca = new CGaugeClickArea();
    ca->Init(this,typ);
    ReadFrom (ca, st);
    vkca[ica]= ca;
    return TAG_READ;
  }
  //--- Decode list of messages -<MSnn> ---------
  int nms = MessageFromTag(tag);
  if (nms > 0)
  { //  Valid message ---
    SMessage *msg = new SMessage;
    ReadMessage(msg,st);
		msg->id		= MSG_SETDATA;
    vmsg[nms] = msg;
    return TAG_READ;
  }
  //--- Decode list of Actions ---<ONnn> -------
  int nac = ActionFromTag(tag);
  if (nac > 0)
  { //  Valid action --
    CGaugeAction *gac = new CGaugeAction();
    gac->ReadActions(st);
    vact[nac] = gac;
    return TAG_READ;
  }
	//--- Default action -------------------------
	if (tag =='dfac')
	{	ReadInt(&ica,st);
		dfa	= ica;
		return TAG_READ;
	}
  //--- Check for other tags -------------------
  return CTexturedGauge::Read(st,tag);
}
//------------------------------------------------------------------
//  All parameters are read
//------------------------------------------------------------------
void CGenericSwitch::ReadFinished()
{ CTexturedGauge::ReadFinished();
	nba = vact.size();
  nbm = vmsg.size();
  return;
}
//------------------------------------------------------------------
//  Initial call
//------------------------------------------------------------------
void CGenericSwitch::PrepareMsg(CVehicleObject *veh)
{ if (dfa)	AreaClicked(dfa);
	return;
}
//-----------------------------------------------------------------------------
//  Collect VBO data
//-----------------------------------------------------------------------------
void CGenericSwitch::CollectVBO(TC_VTAB *vtb)
{	CTexturedGauge::CollectVBO(vtb);
	bmap.CollectVBO(vtb);
	return;
}
//------------------------------------------------------------------
//  Draw the gauge
//------------------------------------------------------------------
void CGenericSwitch::DrawAmbient()
{	bmap.Draw(nfr);
  return;
}
//------------------------------------------------------------------
//  Execute requested action
//------------------------------------------------------------------
void CGenericSwitch::Execute(BASE_ACT *ba)
{ if (ba->Type == ACT_FRAME) {nfr = ba->pm1; return;}
  if (ba->Type == ACT_MSAGE)
  { //--- Send the corresponding message with value --
    std::map<int, SMessage*>::iterator itm = vmsg.find(ba->pm1);
    if (itm != vmsg.end())
    { SMessage *msg = (*itm).second;
      msg->realData = ba->Value;
			msg->intData	= ba->Value;
      Send_Message(msg,mveh);
    }
  }
  return;
}
//------------------------------------------------------------------
//  Clicked area.  Execute action list
//------------------------------------------------------------------
void CGenericSwitch::AreaClicked(int n)
{ //--- Check if one action is associated ------------
  std::map<int,CGaugeAction*>::iterator ita = vact.find(n);
  if (ita == vact.end())   return;
  //--- Execute action list --------------------------
  CGaugeAction *a = (*ita).second;
  BASE_ACT *act = 0;
  a->Start();
  for (act = a->GetAction(); act != 0; act = a->GetAction())
  { Execute(act); }
  return;
}
//------------------------------------------------------------------
//  Mouse click. Check for one area
//------------------------------------------------------------------
EClickResult  CGenericSwitch::MouseClick (int x, int y, int buttons)
{ //-- search area --------------------------------
  std::map<int,CGaugeClickArea*>::iterator ita;
  for (ita = vkca.begin(); ita != vkca.end(); ita++)
  { CGaugeClickArea *ca = (*ita).second;
    if (!ca->IsHit(x, y))  continue;
    AreaClicked((*ita).first);
    break;
  }
  return MOUSE_TRACKING_OFF;
}
//------------------------------------------------------------------
//  Mouse move over gauge. Check for one area
//------------------------------------------------------------------
ECursorResult CGenericSwitch::MouseMoved (int x, int y)
{ for (U_INT n=1; n <= vkca.size(); n++)
  { CGaugeClickArea *ca = vkca[n];
    if (CURSOR_NOT_CHANGED == ca->MouseMoved(x, y))  continue;
    return CURSOR_WAS_CHANGED;
  }
  return CURSOR_NOT_CHANGED;
}
//===============================================================================
//  PushPull with repeated click
//===============================================================================
EClickResult  CRepeatPushPull::MouseClick (int x, int y, int bt)
{ globals->cum->SetCursor(cursTag); 
  if (y > cy)
  { IncValue();
    ArmRepeat(0.1f,x,y,bt);
    return MOUSE_TRACKING_ON;
   } 
  if (y < cy)
  { DecValue();
    ArmRepeat(0.1f,x,y,bt);
    return MOUSE_TRACKING_ON;
   }
  return MOUSE_TRACKING_OFF;
}
//-----------------------------------------------------------------------------
//  Collect VBO data
//-----------------------------------------------------------------------------
void CRepeatPushPull::CollectVBO(TC_VTAB *vtb)
{	C_PushPullKnobGauge::CollectVBO(vtb);
	return;
}
//------------------------------------------------------------------
//  Draw the gauge and repeat any click
//------------------------------------------------------------------
void CRepeatPushPull::Draw()
{ C_PushPullKnobGauge::Draw();
  RepeatClick();
  return;   }
//------------------------------------------------------------------
//  Stop repeat  click
//------------------------------------------------------------------
EClickResult CRepeatPushPull::StopClick()
{ Disarm();
  return C_PushPullKnobGauge::StopClick();
}
//=======================================================================
// JSDEV* simplified C1NeedleGauge()
//=======================================================================
//--------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------
C1NeedleGauge::C1NeedleGauge(CPanel *mp)
: CTexturedGauge(mp)
{ Prop.Set(NO_SURFACE);
  mgg = this;
  nedl.SetGauge(this);
}
//--------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------
C1NeedleGauge::C1NeedleGauge()
: CTexturedGauge(0) 
{ Prop.Set(NO_SURFACE);
  mgg = 0;
  nedl.SetGauge(this);
}
//--------------------------------------------------------------------
//  Read parameters
//---------------------------------------------------------------------
int C1NeedleGauge::Read (SStream *stream, Tag tag)
{ switch (tag) {
    //--- neddle definition --------------
    case 'nedl':
      nedl.SetGauge(this);
      ReadFrom (&nedl, stream);
      return TAG_READ;
    //--- bitmap file ---------------------
    case 'bmap':
      nedl.ReadBMAP(stream);
      return TAG_READ;
    //--- rotation center -----------------
    case 'rotc':
      nedl.ReadROTC(stream);
      return TAG_READ;
    //--- copy the gauge from similar ----
    case 'copy':
      CopyFrom(stream);
      return TAG_READ;
  }
  return CTexturedGauge::Read (stream, tag);
}
//--------------------------------------------------------------------
//  All  parameters are read
//---------------------------------------------------------------------
void C1NeedleGauge::ReadFinished()
{ CTexturedGauge::ReadFinished();
	mesg.id = MSG_GETDATA;
}
//--------------------------------------------------------------------
//  Assign panel
//---------------------------------------------------------------------
void C1NeedleGauge::SetPanel(CPanel *mp)
{	panel = mp;
	vOfs  = mp->FixRoom(4);											// OK
	return;
}
//-----------------------------------------------------------------------------
//  Collect VBO data
//-----------------------------------------------------------------------------
void C1NeedleGauge::CollectVBO(TC_VTAB *vtb)
{	CTexturedGauge::CollectVBO(vtb);
	return;
}
//--------------------------------------------------------------------
//  Assign a mother gauge. Pass the mother quad to the needles
//---------------------------------------------------------------------
void C1NeedleGauge::SetMother(CGauge *mg)
{ mgg = mg;
  nedl.SetGauge(mg);
	mg->GetTour(tour,&w,&h);
  return;
}
//--------------------------------------------------------------------
//  Copy from a similar gauge
//---------------------------------------------------------------------
void C1NeedleGauge::CopyFrom(SStream *stream)
{ C1NeedleGauge &src = *(C1NeedleGauge*)LocateGauge(stream);
  CTexturedGauge::CopyFrom(src);
  return;
}
//--------------------------------------------------------------------
//  Partial Copy from a similar gauge
//  Used when the gauge is a component of another gauge
//---------------------------------------------------------------------
void C1NeedleGauge::CopyFrom(CGauge *mg,C1NeedleGauge &src)
{ CGauge::CopyFrom(src);
  nedl.CopyFrom(mg,src.nedl);
  mgg = mg;
  nedl.SetGauge(mg);
	mg->GetTour(tour,&w,&h);
  return;
}
//--------------------------------------------------------------------
//  Draw the gauge
//---------------------------------------------------------------------
void C1NeedleGauge::Draw()
{ DrawUnderlay();
	Update();
  nedl.DrawNeedle(value);
  return;
}
//--------------------------------------------------------------------
//  Draw in ambient light
//---------------------------------------------------------------------
void C1NeedleGauge::DrawAmbient()
{	return DrawOverlay();}
//--------------------------------------------------------------------
//  Mouse move over 
//---------------------------------------------------------------------
ECursorResult C1NeedleGauge::MouseMoved (int x, int y)
{ DisplayHelp();
  return globals->cum->SetCursor (cursTag);
}

//============================================================================
//  Gauge with 2 rotating needles 
//============================================================================
C2NeedleGauge::C2NeedleGauge(CPanel *mp)
: CTexturedGauge(mp)
{ CCockpitManager *pit	= mp->GetPIT();
	//--------------------------------------------------
	type	= 0;
	ndl1.SetPanel(mp);
	ndl2.SetPanel(mp);
	//--- Allocate holder --------------
	hold	= pit->GetHolder(unId);
}
//------------------------------------------------------------------
//  Read all parameters
//------------------------------------------------------------------
int C2NeedleGauge::Read(SStream *stream,Tag tag)
{ switch (tag)  {
  //---- first needle definition ---------
  case 'ndl1':
    ndl1.SetMother(this);          // Mother gauge
    ndl1.SetPanel(panel);
		ndl1.SetMVEH(mveh);
    ReadFrom(&ndl1,stream);
    return TAG_READ;
  //--- second needle definition -------
  case  'ndl2':
    ndl2.SetMother(this);
    ndl2.SetPanel(panel);
		ndl2.SetMVEH(mveh);
    ReadFrom(&ndl2,stream);
    return TAG_READ;
  //--- knob1 -------------------------
  case 'knb1':
    mrk1.ReadKNOB(stream);
    return TAG_READ;
  //--- knob2 -------------------------
  case 'knb2':
    mrk2.ReadKNOB(stream);
    return TAG_READ;
  //--- define vertical type ----------
  case  'vert':
    type = 1;
    return TAG_READ;
  //--- define first marker -----------
  case 'mrk1':
    mrk1.SetGauge(this);
		mrk1.SetHolder(hold,1);
    ReadFrom(&mrk1,stream);
    return TAG_READ;
  //--- define second marker -----------
  case 'mrk2':
    mrk2.SetGauge(this);
		mrk2.SetHolder(hold,2);
    ReadFrom(&mrk2,stream);
    return TAG_READ;

  //--- copy from ---------------------
  case 'copy':
    CopyFrom(stream);
    return TAG_READ;
  }
  return CTexturedGauge::Read(stream,tag);
}
//------------------------------------------------------------------
//  Copy from a similar gauge
//------------------------------------------------------------------
void C2NeedleGauge::CopyFrom(SStream *stream)
{ C2NeedleGauge &src = *(C2NeedleGauge *)LocateGauge(stream);
  CTexturedGauge::CopyFrom(src);
  ndl1.CopyFrom(this,src.ndl1);
  ndl2.CopyFrom(this,src.ndl2);
  mrk1.CopyFrom(this,src.mrk1);
  mrk2.CopyFrom(this,src.mrk2);
  type  = src.type;
  return;
}
//------------------------------------------------------------------
//  Prepare message for each needle
//------------------------------------------------------------------
void C2NeedleGauge::PrepareMsg(CVehicleObject *veh)
{ ndl1.PrepareMsg(veh);
	ndl2.PrepareMsg(veh);
	return;
}
//-----------------------------------------------------------------------------
//  Collect VBO data
//-----------------------------------------------------------------------------
void C2NeedleGauge::CollectVBO(TC_VTAB *vtb)
{	CTexturedGauge::CollectVBO(vtb);
	ndl1.CollectVBO(vtb);
	ndl2.CollectVBO(vtb);
	mrk1.CollectVBO(vtb);
	mrk2.CollectVBO(vtb);
	return;
}
//------------------------------------------------------------------
//  Draw the gauge
//------------------------------------------------------------------
void C2NeedleGauge::Draw()
{	under.Draw();
  mrk1.Draw();
  ndl1.Draw();
  mrk2.Draw();
  ndl2.Draw();
  return;
}
//------------------------------------------------------------------
//  Draw in the ambient light
//------------------------------------------------------------------
void C2NeedleGauge::DrawAmbient()
{ 
	return overl.Draw(); }
//------------------------------------------------------------------
//  Mouse click over the gauge
//------------------------------------------------------------------
EClickResult C2NeedleGauge::MouseClick (int x, int y, int btn)
{ if (mrk1.MouseClick(x,y,btn))		return MOUSE_TRACKING_ON;
  if (mrk2.MouseClick(x,y,btn))   return MOUSE_TRACKING_ON;
	return MOUSE_TRACKING_OFF;
}
//------------------------------------------------------------------
//  Mouse move over the gauge
//------------------------------------------------------------------
ECursorResult C2NeedleGauge::MouseMoved (int x, int y)
{ if (mrk1.MouseMoved(x,y))	return CURSOR_WAS_CHANGED;
  char side = (LfHit(x,y))?('L'):('R');
	switch(side)	{
		case 'L':
			{	Tag csr	= ndl1.Cursor();
				ndl1.DisplayHelp();
				return globals->cum->SetCursor (csr);
			}
		case 'R':
			{	Tag csr = ndl2.Cursor();
				ndl2.DisplayHelp();
				return globals->cum->SetCursor (csr);
			}
	}
  return CURSOR_NOT_CHANGED;
}
//----------------------------------------------------------------
//	Stop click:  Disarm rotation
//----------------------------------------------------------------
EClickResult C2NeedleGauge::StopClick()
{	mrk1.StopClick();
  mrk2.StopClick();
  return MOUSE_TRACKING_OFF;
}
//====================================================================
// JSDEV* modified C_BasicCompassGauge
//====================================================================
C_BasicCompassGauge::C_BasicCompassGauge (CPanel *mp)
: CTexturedGauge(mp)
{ Prop.Rep(NO_SURFACE);
  mesg.id	= MSG_GETDATA;
  A     = 0;
  B     = 0;
  M     = 0;
}
//----------------------------------------------------------------
//  Destructor
//----------------------------------------------------------------
C_BasicCompassGauge::~C_BasicCompassGauge (void)
{
}
//-----------------------------------------------------------------
//  Read all tags
//------------------------------------------------------------------
int C_BasicCompassGauge::Read (SStream *stream, Tag tag)
{ float pm;
  switch (tag) {
  //--- bitmap for compass strip --------------
	case 'comp':
    comps.SetGauge(this);
    comps.ReadStrip(stream);
		return TAG_READ;
  //--- windows width in pixels ---------------
	case 'wind':
		ReadFloat (&pm, stream);
    M  = (pm * 0.5);
		return TAG_READ;
  //--- distance to North origin in pixels ----
	case 'orgN':
		ReadFloat(&A, stream);
		return TAG_READ;
  //--- North to North distance in pixels -----
  case 'NtoN':
		ReadFloat(&pm, stream);
		B = pm / 360;
		return TAG_READ;
  //--- Quad definition -----------------------
  case 'quad':
    comps.SetGauge(this);
    comps.ReadPROJ(stream);
    return TAG_READ;
  }

  return CTexturedGauge::Read (stream, tag);
  }
//---------------------------------------------------------------
//  All parameters are read
//---------------------------------------------------------------
void C_BasicCompassGauge::ReadFinished (void)
{ CTexturedGauge::ReadFinished ();  
	//--- Allocate dynamic room for the gauge ----
  TC_VTAB *vbo = panel->GetDynVBO();
	U_SHORT  ofs = panel->DynRoom(4);
	comps.DynaQuad(vbo,ofs);
	ofs					 = panel->DynRoom(4);
	overl.DynaQuad(vbo,ofs);
	return;
}
//-----------------------------------------------------------------
//  Collect VBO data
//------------------------------------------------------------------
void C_BasicCompassGauge::CollectVBO(TC_VTAB *vtb)
{	CTexturedGauge::CollectVBO(vtb);
	return;
}

//-----------------------------------------------------------------
//	JSDEV* Draw the strip
//	The strip bitmap is like this
//	60°-----N----------------------------N------330°
//	with nton pixels from N to N
//	return value from mcmp subsystem is in [0,360] in degres
//	Note that timK correction int mcmp can overshoot this range
//		   value is clamped inside this range
//-----------------------------------------------------------------
void C_BasicCompassGauge::DrawDynamic(void)
{	//--- Get heading value ---------------------
	Send_Message (&mesg,mveh);
  float X   = mesg.user.u.unit;
  cdir      = mesg.realData;
	//--- Compute texture coordinates -----------
  float mp  = A + (B * X);        // mid window pixel
  float p0  = mp - M;             // left pixel
  float p1  = mp + M;             // Right pixel
  comps.DrawRange(p0,p1);         // Draw strip part
  overl.Draw();
  return;
}
//-------------------------------------------------------------------
//	Mouse move over
//-------------------------------------------------------------------
void C_BasicCompassGauge::DisplayHelp()
{	if (0 == *help)     return;
  sprintf_s(hbuf,HELP_SIZE,"Compas %03d°",cdir);
  FuiHelp();
  return;
}
//========================================================================
// strip Gauge:  Gauge using a vertical strip
//=========================================================================
CStripGauge::CStripGauge(CPanel *mp)
: CTexturedGauge(mp)
{}
//-------------------------------------------------------------------
//	Read All parameters
//-------------------------------------------------------------------
int  CStripGauge::Read (SStream *stream, Tag tag)
{	switch(tag)
	{	case 'bmap':
			bmap.SetGauge(this);
			bmap.ReadStrip(stream);
			return TAG_READ;
	}
	return CTexturedGauge::Read(stream,tag);	
}
//-----------------------------------------------------------------
//  Collect VBO data
//------------------------------------------------------------------
void CStripGauge::CollectVBO(TC_VTAB *vtb)
{	CTexturedGauge::CollectVBO(vtb);
	bmap.CollectVBO(vtb);
	return;
}
//-------------------------------------------------------------------
//	Draw the gauge
//-------------------------------------------------------------------
void CStripGauge::Draw()
{	Update();
	bmap.Draw(value);
}


//=========================END OF FILE =======================================================