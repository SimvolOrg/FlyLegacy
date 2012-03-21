/*
 * Cameras.cpp
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

/*! \file Cameras.cpp
 *  \brief Implements CCamera and default camera views
 *
 *  Several default cameras are available for exterior and
 *    interior (cockpit) viewing of the user aircraft.  This file
 *    implements the common parent class CCamera as well as
 *    camera-specific descendent classes.
 */

#include "../Include/WorldObjects.h"
#include "../Include/Cameras.h"
#include "../Include/Utility.h"
#include "../Include/Ui.h"
#include "../Include/Globals.h"
#include "../Include/Fui.h"
#include "../Include/TerrainTexture.h"
#include "../Include/Airport.h"
using namespace std;
//==========================================================================
class CCameraCockpit;
//==========================================================================
// Group function for Camera manager 'cmra'
//==========================================================================
bool KeyCamGroup(CKeyDefinition *kdf, int code)
{ if (kdf->NoPCB())   return false;
  Tag kid   = kdf->GetTag();
  U_INT key = code & 0x0000FFFF;      // Key code
  U_INT mod = (code >> 16);           // Modifier
  return kdf->GetCallback() (kid,key,mod);
}
//==========================================================================
//  Global functions for Camera manager 'cmra'
//==========================================================================
//--------------------------------------------------------------------------
//  Camera cockpit 'cock'
//--------------------------------------------------------------------------
bool cKeyCOCK(int id, int code, int mod)
{ globals->ccm->KbEvent(id);
  return true; }
//--------------------------------------------------------------------------
//  Camera spot 'spot'
//--------------------------------------------------------------------------
bool cKeySPOT(int id, int code, int mod)
{ globals->ccm->KbEvent(id);
  return true; }
//--------------------------------------------------------------------------
//  Camera observer 'obsr'
//--------------------------------------------------------------------------
bool cKeyOBSR(int id, int code, int mod)
{ globals->ccm->KbEvent(id);
  return true; }
//--------------------------------------------------------------------------
//  Camera overhead 'over'
//--------------------------------------------------------------------------
bool cKeyOVER(int id, int code, int mod)
{ globals->ccm->KbEvent(id);
  return true; }
//--------------------------------------------------------------------------
//  Camera fly by 'flyb'
//--------------------------------------------------------------------------
bool cKeyFLYB(int id, int code, int mod)
{ globals->ccm->KbEvent(id);
  return true; }
//--------------------------------------------------------------------------
//  Camera tower 'towr'
//--------------------------------------------------------------------------
bool cKeyTOWR(int id, int code, int mod)
{ globals->ccm->KbEvent(id);
  return true; }
//--------------------------------------------------------------------------
//  zoom in ratio 'czri'
//--------------------------------------------------------------------------
bool cKeyCZRI(int id, int code, int mod)
{ globals->cam->ZoomRatioIn();
  return true; }
//--------------------------------------------------------------------------
//  zoom out ratio 'czro'
//--------------------------------------------------------------------------
bool cKeyCZRO(int id, int code, int mod)
{ globals->cam->ZoomRatioOut();
  return true; }
//--------------------------------------------------------------------------
//  zoom in normal 'czin'
//--------------------------------------------------------------------------
bool cKeyCZIN(int id, int code, int mod)
{ globals->cam->RangeIn();
  return true; }
//--------------------------------------------------------------------------
//  zoom in slow 'czis'
//--------------------------------------------------------------------------
bool cKeyCZIS(int id, int code, int mod)
{ globals->cam->RangeInSlow();
  return true; }
//--------------------------------------------------------------------------
//  zoom in fast 'czif'
//--------------------------------------------------------------------------
bool cKeyCZIF(int id, int code, int mod)
{ globals->cam->RangeInFast();
  return true; }
//--------------------------------------------------------------------------
//  zoom out normal 'czot'
//--------------------------------------------------------------------------
bool cKeyCZOT(int id, int code, int mod)
{ globals->cam->RangeOut();
  return true; }
//--------------------------------------------------------------------------
//  zoom out slow 'czos'
//--------------------------------------------------------------------------
bool cKeyCZOS(int id, int code, int mod)
{ globals->cam->RangeOutSlow();
  return true; }
//--------------------------------------------------------------------------
//  zoom out fast 'czof'
//--------------------------------------------------------------------------
bool cKeyCZOF(int id, int code, int mod)
{ globals->cam->RangeOutFast();
  return true; }
//--------------------------------------------------------------------------
//  zoom Reset 'czrs'
//--------------------------------------------------------------------------
bool cKeyCZRS(int id, int code, int mod)
{ globals->cam->RangeReset();
  return true; }
//--------------------------------------------------------------------------
//  Pitch up 'chpu'
//--------------------------------------------------------------------------
bool cKeyCHPU(int id, int code, int mod)
{ globals->cam->HeadPitchUp();
  return true; }
//--------------------------------------------------------------------------
//  Pitch Down 'chpd'
//--------------------------------------------------------------------------
bool cKeyCHPD(int id, int code, int mod)
{ globals->cam->HeadPitchDown();
  return true; }
//--------------------------------------------------------------------------
//  Pan left 'cplf'
//--------------------------------------------------------------------------
bool cKeyCPLF(int id, int code, int mod)
{ globals->cam->PanLeft();
  return true; }
//--------------------------------------------------------------------------
//  Pan Right 'cprt'
//--------------------------------------------------------------------------
bool cKeyCPRT(int id, int code, int mod)
{ globals->cam->PanRight();
  return true; }
//--------------------------------------------------------------------------
//  Pan up 'cpup'
//--------------------------------------------------------------------------
bool cKeyCPUP(int id, int code, int mod)
{ globals->cam->PanUp();
  return true; }
//--------------------------------------------------------------------------
//  Pan down 'cpdn'
//--------------------------------------------------------------------------
bool cKeyCPDN(int id, int code, int mod)
{ globals->cam->PanDown();
  return true; }
//--------------------------------------------------------------------------
//  User 1 'cs01'
//--------------------------------------------------------------------------
bool cKeyCS01(int id, int code, int mod)
{ globals->cam->User1();
  return true; }
//--------------------------------------------------------------------------
//  User 2 'cs02'
//--------------------------------------------------------------------------
bool cKeyCS02(int id, int code, int mod)
{ globals->cam->User2();
  return true; }
//--------------------------------------------------------------------------
//  User 3 'cs03'
//--------------------------------------------------------------------------
bool cKeyCS03(int id, int code, int mod)
{ globals->cam->User3();
  return true; }
//--------------------------------------------------------------------------
//  User 4 'cs04'
//--------------------------------------------------------------------------
bool cKeyCS04(int id, int code, int mod)
{ globals->cam->User4();
  return true; }
//--------------------------------------------------------------------------
//  User defined 1 'cd01'
//--------------------------------------------------------------------------
bool cKeyCD01(int id, int code, int mod)
{ globals->cam->DefineUser1();
  return true; }
//--------------------------------------------------------------------------
//  User defined 2 'cd02'
//--------------------------------------------------------------------------
bool cKeyCD02(int id, int code, int mod)
{ globals->cam->DefineUser2();
  return true; }
//--------------------------------------------------------------------------
//  User defined 3 'cd03'
//--------------------------------------------------------------------------
bool cKeyCD03(int id, int code, int mod)
{ globals->cam->DefineUser3();
  return true; }
//--------------------------------------------------------------------------
//  User defined 4 'cd04'
//--------------------------------------------------------------------------
bool cKeyCD04(int id, int code, int mod)
{ globals->cam->DefineUser4();
  return true; }
//--------------------------------------------------------------------------
//  Camera manual mode
//--------------------------------------------------------------------------
bool cKeyCMTG(int id, int code, int mod)
{ return globals->cam->ToggleMode(); }
//--------------------------------------------------------------------------
//  Camera control box
//--------------------------------------------------------------------------
bool cKeyCZAP(int id, int code, int mod)
{ globals->cam->ToggleBox();
  return true; }
//--------------------------------------------------------------------------
//  rfu 'cd07'
//--------------------------------------------------------------------------
bool cKeyCD07(int id, int code, int mod)
{ 
  return true; }
//--------------------------------------------------------------------------
//  rfu 'cd08'
//--------------------------------------------------------------------------
bool cKeyCD08(int id, int code, int mod)
{ 
  return true; }
//--------------------------------------------------------------------------
//  CAmera manager: Cockpit up 'ckup'
//--------------------------------------------------------------------------
bool cKeyCKUP(int id, int code, int mod)
{ globals->ccm->KeyCameraCockpitEvent(id);
  return true; }
//--------------------------------------------------------------------------
//  CAmera manager: Cockpit down 'ckdn'
//--------------------------------------------------------------------------
bool cKeyCKDN(int id, int code, int mod)
{ globals->ccm->KeyCameraCockpitEvent(id);
  return true; }
//--------------------------------------------------------------------------
//  CAmera manager: Cockpit left 'cklf'
//--------------------------------------------------------------------------
bool cKeyCKLF(int id, int code, int mod)
{ globals->ccm->KeyCameraCockpitEvent(id);
  return true; }
//--------------------------------------------------------------------------
//  CAmera manager: Cockpit right 'ckri'
//--------------------------------------------------------------------------
bool cKeyCKRI(int id, int code, int mod)
{ globals->ccm->KeyCameraCockpitEvent(id);
  return true; }
//--------------------------------------------------------------------------
//  CAmera manager: Cockpit home 'ckhm'
//--------------------------------------------------------------------------
bool cKeyCKHM(int id, int code, int mod)
{ globals->ccm->KeyCameraCockpitEvent(id);
  return true; }
//--------------------------------------------------------------------------
//  CAmera manager: Cockpit pan up 'ckpu'
//--------------------------------------------------------------------------
bool cKeyCKPU(int id, int code, int mod)
{ globals->ccm->KeyCameraCockpitEvent(id);
  return true; }
//--------------------------------------------------------------------------
//  CAmera manager: Cockpit pan down 'ckpd'
//--------------------------------------------------------------------------
bool cKeyCKPD(int id, int code, int mod)
{ globals->ccm->KeyCameraCockpitEvent(id);
  return true; }
//===================================================================================
// CCamera base class methods
//===================================================================================
CCamera::CCamera (void)
{ fov   = globals->fovX;          // Degrees
	ffac	= 1;
  range = globals->camRange;      // xx feet initial
  Lock  = 0;
	pick	= 0;											// Picking indicator
	intcm	= 0;
	extcm = 1;
	Rate	= 0.05f;										// Default rotation rate
  //----Init default up vector -May be overwriten by specific camera -
  Up.x  = 0;
  Up.y  = 0;
  Up.z  = 1.0f;
  rmin  = globals->camRange;
  rmax  = 5000;
  dmin  = globals->nearP + 10;
  //----SW base corner ------------------------------------------------
  Pan[0].VT_S   = 0;
  Pan[0].VT_T   = 0;
  Pan[0].VT_X   = 0;
  Pan[0].VT_Y   = 0;
  Pan[0].VT_Z   = 0;
  //-----SE corner ----------------------------------------------------
  Pan[1].VT_S   = 1;
  Pan[1].VT_T   = 0;
  Pan[1].VT_X   = 0;
  Pan[1].VT_Y   = 0;
  Pan[1].VT_Z   = 0;
  //-----NE corner ----------------------------------------------------
  Pan[2].VT_S   = 1;
  Pan[2].VT_T   = 1;
  Pan[2].VT_X   = 0;
  Pan[2].VT_Y   = 0;
  Pan[2].VT_Z   = 0;
  //-----NW corner ----------------------------------------------------
  Pan[3].VT_S   = 0;
  Pan[3].VT_T   = 1;
  Pan[3].VT_X   = 0;
  Pan[3].VT_Y   = 0;
  Pan[3].VT_Z   = 0;
  //------------------------------------------------------------------
  // Initialize offset and orientation vectors
  offset.x = offset.y = offset.z = 0;
  orient.x = orient.y = orient.z = 0;
  nearP    = globals->nearP;
	farP		 = globals->afarP;
  SetMinAGL();
  // Define a clamp value just less than 90 degrees
  clamp = DegToRad (89.99999f);
	//--- Temporaire ---------------------------------------
	T1 = 0;
	T2 = 0;
}
//-------------------------------------------------------------------------
// Camera is destroyed
//-------------------------------------------------------------------------
CCamera::~CCamera()
{	if (globals->cam == this)	ReleaseIndicators();}
//-------------------------------------------------------------------------
// Camera change internal parameters
//	NOTE:  The tangent value is increased to accomodate for precision error
//	when computing a point position inside the frustum
//-------------------------------------------------------------------------
void CCamera::SetCameraParameters(double fv,double rt)
{	ratio = rt;
	fov   = fv;
	//--- Compute height and width of near section ---------
	tgf   = tan(DegToRad(fv * 0.5));
	htr		= nearP * tgf;
	wdt		= htr   * ratio;
	return;
}
//-------------------------------------------------------------------------
// Compute camera referential relative to target position tgt
//	The camera referential are the 3 vectors local to the camera
//	-Rx is the right direction
//	-Ry is the forward direction
//	-Rz is the up direction
//	Vector forward is the vector from eye position to target position
//-------------------------------------------------------------------------
void CCamera::SetReferential(SPosition &tgt)
{	//--- Compute components of forward vector --------------
	Ry = SubtractPositionInFeet(camPos, tgt);
	Ry.Normalize();
	//--- Compute X reference from Ry and given up vector ---
	Up.Normalize();
	Rx.CrossProduct(Ry,Up);
	Rx.Normalize();
	//--- Compute Up reference -(is normalized already) -----
	Rz.CrossProduct(Rx,Ry);
	return;
}
//=========================================================================
#define FTUM_INSIDE		(0)
#define FTUM_REAR			(1)
#define FTUM_LEFT			(2)
#define FTUM_RIGHT		(3)
#define FTUM_ABOVE		(4)
#define FTUM_BELOW		(5)
//-------------------------------------------------------------------------
// Check for point in frustum 
//	NOTE:  We use the reduction factor at aircraft position
//	This return an index to account for point position in the
//	frustum
//-------------------------------------------------------------------------
bool CCamera::GeoPosInFrustum(SPosition &P, CVector &R, char *T)
{	bool in = true;
	//--- Compute Vector from camera to Position -(in feet components)-----
	CVector fwp = PJ + R;								// Relocate to point position
	//--- Compute and test the forward coordinate -------------------------
	double  yp  = fwp.DotProduct(Ry);
	if (yp < nearP)			{	T[FTUM_REAR]++; in= false;}
	//--- Compute and test the Up direction --------------------------------
	double  zp		= fwp.DotProduct(Rz);
	double  zlim  = yp * tgf * ffac;						// Limit in X direction
	if (zp < -zlim)			{	T[FTUM_BELOW]++; in = false;}
	if (zp >  zlim)			{ T[FTUM_ABOVE]++; in = false;}
	//--- Compute and test the right direction ------------------------------
	double	xp		= fwp.DotProduct(Rx);
	double  xlim  = zlim * ratio;
	if (xp < -xlim)			{ T[FTUM_LEFT]++;		in = false; }
	if (xp >  xlim)			{ T[FTUM_RIGHT]++;	in = false; }
	return in;
}
//-------------------------------------------------------------------------
// Check for box in frustum 
//	ff is a flare factor to account for error when testing point in
//	frustum
//	NOTE:  Both P and B must have coordinates in arcsec
//	We check for the 8 corners points of the box.
//	If only one is in frustum, then the box is at least partially visible
//-------------------------------------------------------------------------
bool CCamera::BoxInFrustum(SPosition &P, CVector &B, double ff)
{	char  pos[8] = {0,0,0,0,0,0,0,0};
	CVector T = B;
	T.x *= TC_FEET_PER_ARCSEC * globals->rdf;			// In feet
	T.y *= TC_FEET_PER_ARCSEC;
	//--- Compute vector from camera to target --------------
	ffac	= ff;
	PJ = FeetComponents(camPos,P,globals->rdf);
	//--- Check corner 1 -------------------------
	CVector c1(-T.x,-T.y,-T.z);
	if (GeoPosInFrustum(P,c1,pos))	return true;
	//--- Check corner 2 --------------------------
	CVector c2(-T.x,-T.y,+T.z);
	if (GeoPosInFrustum(P,c2,pos))	return true;
	//--- Check corner 3 --------------------------
	CVector c3(-T.x,+T.y,-T.z);
	if (GeoPosInFrustum(P,c3,pos))	return true;
	//--- Check corner 4 --------------------------
	CVector c4(-T.x,+T.y,+T.z);
	if (GeoPosInFrustum(P,c4,pos))	return true;
	//--- Check corner 5 ---------------------------
	CVector c5(+T.x,-T.y,-T.z);
	if (GeoPosInFrustum(P,c5,pos))	return true;
	//--- Check corner 6 ---------------------------
	CVector c6(+T.x,-T.y,+T.z);
	if (GeoPosInFrustum(P,c6,pos))	return true;
	//--- Check corner 7 ---------------------------
	CVector c7(+T.x,+T.y,-T.z);
	if (GeoPosInFrustum(P,c7,pos))	return true;
	//--- Check corner 8 ---------------------------
	CVector c8(+T.x,+T.y,+T.z);
	if (GeoPosInFrustum(P,c8,pos))	return true;
	//--- Check if box is astride ------------------
	if (pos[FTUM_REAR]  == 8)		return false;
	if (pos[FTUM_LEFT]  == 8)		return false;
	if (pos[FTUM_RIGHT] == 8)		return false;
	if (pos[FTUM_ABOVE] == 8)		return false;
	if (pos[FTUM_BELOW] == 8)		return false;
	//--- Box may be visible ------------------------------
	//	NOTE: When we are here, then all parts are in front
	//				but some are spread on different sides of the
	//				frustum. Some objects may still be not
	//				visible. So some extra overhead is incurred
	//	Exemple:
	//            |                   *|
	//            |                    |*
	//      ------------------------------*---------------
	//            |   inside           |   * object is not
	//            | frustum            |     visible
	//            |                    |
	//-----------------------------------------------------
	return true;
}
//-------------------------------------------------------------------------
//	Standard camera referential use target position
//--------------------------------------------------------------------------
void	CCamera::CameraReferential()
{	SetReferential(tgtPos);
	return;
}
//-------------------------------------------------------------------------
// Camera save context
//-------------------------------------------------------------------------
void CCamera::Save(CAMERA_CTX &ctx)
{ TagToString(ctx.sidn,cIden);
	ctx.iden  = cIden;
  ctx.range = range;
  ctx.rmin  = rmin;
  ctx.rmax  = rmax;
  ctx.theta = theta;
  ctx.phi   = phi;
  ctx.fov   = fov;
	//--- save global position ----------
	ctx.ori   = globals->iang;
	ctx.pos		= globals->geop;
  return;
};
//-------------------------------------------------------------------------
// Camera restore parameters from context
//-------------------------------------------------------------------------
void CCamera::Restore(CAMERA_CTX &ctx)
{ range = ctx.range;
  rmin  = ctx.rmin;
  rmax  = ctx.rmax;
  theta = ctx.theta;
  phi   = ctx.phi;
  fov   = ctx.fov;
	//--- Restore vehicle position ----
  CVehicleObject *veh = globals->pln;
  if (veh) veh->SetObjectPosition(ctx.pos);
	if (veh) veh->SetObjectOrientation(ctx.ori);
	if (veh) veh->SetPhysicalOrientation(ctx.ori);
  return;
}
//-----------------------------------------------------------------
//  Release camera constraint
//-----------------------------------------------------------------
void	CCamera::ReleaseIndicators()
{	globals->noEXT -= GetINTMOD();
	globals->noINT -= GetEXTMOD();
	return;
}
//------------------------------------------------------------------------
//  Swap camera control window
//-------------------------------------------------------------------------
void CCamera::ToggleBox()
{	globals->fui->ToggleFuiWindow('ccam');
	return;
}
//-------------------------------------------------------------------------
//  Teleport to requested position
//-------------------------------------------------------------------------
void CCamera::GoToPosition(SPosition &dst)
{ globals->m3d->ReleaseVOR();
  globals->geop = dst;
  return;
}

//------------------------------------------------------------------------
//  Screen is resized
//-------------------------------------------------------------------------
void CCamera::ChangeResolution()
{ int  wd  = globals->cScreen->Width;
  int  ht  = globals->cScreen->Height;
  glViewport(0,0,wd,ht);    
  return;
}
//-------------------------------------------------------------------------
// Compute minimum above ground level
//-------------------------------------------------------------------------
void CCamera::SetMinAGL()
{ double rad  = DegToRad(double(fov) * 0.5);
  minAGL      = dmin * tan(rad);
  return;
}
//-------------------------------------------------------------------------
//  Update camera height
//  -offset are in feet from aircraft posiiton
//-------------------------------------------------------------------------
bool CCamera::GoodHeight (double aphi)
{ double rh =   range * sin (aphi);       // Relative height in feet
  double gh =   globals->geop.alt + rh;   // Absolute elevation
  return (gh >  minAGL);
}
//-------------------------------------------------------------------------
//  Adjust camera range if permitted
//-------------------------------------------------------------------------
void CCamera::RangeAdjust(double lg)
{ double nr = lg * 0.75;           // New range
  if (nr < dmin )   return;
  range = nr;
  return;
}

//-------------------------------------------------------------------------
// Set the camera zoom
//-------------------------------------------------------------------------
void  CCamera::SetZoom(float z)
{ fov = z;
  if (fov < 20.0f)   fov = 20.0f;
  else if (fov > 90.0f) fov = 90.0f;
  return;
}

//-------------------------------------------------------------------------
// Decrease camera FOV by 1 degrees
//-------------------------------------------------------------------------
void CCamera::ZoomRatioIn (void)
{ if (Prof.Not(CAM_MAY_ZOOM))	return;
	char txt[256];
  fov -= 0.25f;
  if (fov < 25.0f) fov = 25.0f;
  int fv = int(fov);
  SetMinAGL();
  _snprintf(txt,255,"FOV: %02d °",fv);
  globals->fui->DrawNoticeToUser(txt,5);
  return;
}
//-------------------------------------------------------------------------
// Increase camera FOV by 1 degrees
//-------------------------------------------------------------------------
void CCamera::ZoomRatioOut (void)
{ if (Prof.Not(CAM_MAY_ZOOM))	return;
	char txt[256];
  fov += 0.25f;
  if (fov > 90.0f) fov = 90.0f;
  int fv = int(fov);
  SetMinAGL();
  _snprintf(txt,255,"FOV: %02d °",fv);
  globals->fui->DrawNoticeToUser(txt,5);
  return;
}
//------------------------------------------------------------------------
// Edit Range
//------------------------------------------------------------------------
void CCamera::ShowRange()
{ char txt[256];
  _snprintf(txt,255,"Range: %.0f feet",range);
  globals->fui->DrawNoticeToUser(txt,5);
  return;
}
//------------------------------------------------------------------------
// Zoom in by 2.5% of current range
//------------------------------------------------------------------------
void CCamera::RangeIn (void)
{ if (Prof.Not(CAM_MAY_MOVE))	return;
	range /= 1.025f;
  if (range < rmin) range = rmin;
  SetMinAGL();
  return ShowRange();
}
//------------------------------------------------------------------------
// Zoom in by 5 feet of current range
//------------------------------------------------------------------------
void CCamera::RangeInSlow (void)
{ if (Prof.Not(CAM_MAY_MOVE))	return;
	range -= 5;
  if (range < rmin) range = rmin;
  SetMinAGL();
  return ShowRange();
}
//------------------------------------------------------------------------
// Zoom in by 25% of current range
//------------------------------------------------------------------------
void CCamera::RangeInFast (void)
{ if (Prof.Not(CAM_MAY_MOVE))	return;
	range /= 1.20;
  if (range < rmin) range = rmin;
  SetMinAGL();
  return ShowRange();
}
//------------------------------------------------------------------------
// Zoom out by 2.5% of current range
//------------------------------------------------------------------------
void CCamera::RangeOut (void)
{ if (Prof.Not(CAM_MAY_MOVE))	return;
	range *= 1.025f;
  if (range > rmax) range = rmax;
  SetMinAGL();
  return ShowRange();
}
//------------------------------------------------------------------------
// Zoom out by 5 feet of current range
//------------------------------------------------------------------------
void CCamera::RangeOutSlow (void)
{ if (Prof.Not(CAM_MAY_MOVE))	return;
	range +=05;
  if (range > rmax) range = rmax;
  SetMinAGL();
  return ShowRange();
}
//------------------------------------------------------------------------
// Zoom out by 20% of current range
//------------------------------------------------------------------------
void CCamera::RangeOutFast (void)
{ if (Prof.Not(CAM_MAY_MOVE))	return;
	range *= 1.20;
  if (range > rmax) range = rmax;
  SetMinAGL();
  return ShowRange();
}
//------------------------------------------------------------------------
// Reset zoom range to default
//------------------------------------------------------------------------
void CCamera::RangeReset (void)
{ if (Prof.Not(CAM_MAY_MOVE))	return;
	range = 60;   // Feet
  SetMinAGL();
  return ShowRange();
}
//------------------------------------------------------------------------
// Return camera lookat point
//------------------------------------------------------------------------
void CCamera::GetLookatPoint (SVector &v)
{ // By default, look at target vehicle
  v.x = v.y = v.z = 0.0;
}
//------------------------------------------------------------------------
//   Compute vector offset from position to camera, in feet
//------------------------------------------------------------------------
void  CCamera::OffsetFrom(SPosition &pos,CVector &vt)
{ globals->tcm->RelativeFeetTo(pos,vt);
  vt.Add(offset);
  return;
}
//------------------------------------------------------------------------
//   Compute camera absolute feet position
//------------------------------------------------------------------------
void CCamera::AbsoluteFeetPosition(CVector &v)
{ v = *globals->tcm->PlaneFeetPos();
  v.Add(offset);
  return;
}
//-------------------------------------------------------------------------
//  Set Camera Range, return range ratio
//-------------------------------------------------------------------------
double CCamera::SetRange(double nr)
{ if (nr > rmax)  nr = rmax;
  if (nr < rmin)  nr = rmin;
  range = nr;
  double dr = rmax - rmin;
  return (nr / dr);
}
//-------------------------------------------------------------------------
//  Set Camera Angle
//-------------------------------------------------------------------------
void CCamera::SetAngle(double a, double b)
{ theta = DegToRad(a);
  phi   = DegToRad(b);
  return;
}
//-------------------------------------------------------------------------
//  Set  default
//-------------------------------------------------------------------------
void CCamera::Reset()
{ range   = globals->camRange;      // xx feet initial
  fov = globals->fovX;
  Rotate(0);
  return;
}
//-------------------------------------------------------------------------
//  Camera position is Aircraft position + offset
//NOTE:  Offset is supposed to be updated.  If not call Update first
//-------------------------------------------------------------------------
void CCamera::GetPosition(SPosition &pos)
{ pos = *globals->tcm->PlaneArcsPos();
  AddToPosition(pos,offset);
  return;
}
//-------------------------------------------------------------------------
// Prepare OpenGL to draw with this camera
//  This is standard drawing where camera look at model position stored
//  in global geop with orientation iang.
//  For other type of drawing, this may be overload
//-------------------------------------------------------------------------
void CCamera::StartShoot(float dT)
{ CVector   tgp;
  //----Initialize the model view matrix -------------------------------
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  //---Update the active camera -----------------------------------------
  UpdateCamera (globals->geop, globals->iang,dT);
  GetLookatPoint(tgp);
  // Initialize the modelview matrix to be the camera transform.  The camera is positioned
  //   at some offset from the world origin.  The world origin is coincident with the
  //   target vehicle position.
  glPushMatrix();
  gluLookAt (offset.x, offset.y, offset.z,
             tgp.x, tgp.y, tgp.z,
             Up.x,   Up.y,  Up.z);
  // Initialize aspect ratio and FOV
  //======================================================================
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
	StartPicking();
	SetCameraParameters(fov,globals->aspect);
  gluPerspective (fov, ratio, nearP,farP); 
  glMatrixMode (GL_MODELVIEW);
  glFrontFace(GL_CCW);
  return;
}
//-------------------------------------------------------------------------
// Prepare OpenGL to draw with this camera
//    The camera is set at Tgt position (depending on camera)
//    The object 'obj' is presumed to be at origin(0,0,-z)
//    VIEW_PORT specify a viewport on which the camera draw the object
//    The view port may be a Fui component for instance
//    The object should supply the 3 methods
//    -PreDraw()
//    -CamDraw();
//    -EndDraw();
//  NOTE:  This method may be used for Runways or objects that does not 
//    modify the depth buffer.  Otherwise, use the DrawObject Camera which
//    use a FBO for rendering
//-------------------------------------------------------------------------
void CCamera::DrawObject(float dT,VIEW_PORT &vp,CFuiWindow *win)
{ SPosition pos = globals->geop;
  CVector   ori(0,0,0);
  CVector   tgp(0,0,0); 
  //----Initialize the model view matrix -------------------------------
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity ();
  //---Update the active camera -----------------------------------------
  UpdateCamera (pos, ori,dT);
  GetLookatPoint(tgp);
  // Initialize the modelview matrix to be the camera transform.  The camera is positioned
  //   at some offset from the world origin.  The world origin is coincident with the
  //   target vehicle position.
  gluLookAt (offset.x, offset.y, offset.z,
             tgp.x, tgp.y, tgp.z,
             Up.x,   Up.y,  Up.z);
  //======================================================================
  double aspect = double(vp.wd) / vp.ht;
  glMatrixMode (GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity ();
	SetCameraParameters(fov,aspect);
  gluPerspective (fov, ratio, nearP,farP);
  //---------Save and set view port  -------------------------------------
  glGetIntegerv(GL_VIEWPORT,(GLint*)&vps);
  glViewport(vp.x0,vp.y0,vp.wd,vp.ht);
  glMatrixMode (GL_MODELVIEW);
  //----------------------------------------------------------------------
  win->DrawByCamera(this);                                // Drawing function
  //-----Exit -------------------------------------------------------------
  glViewport(vps.x0,vps.y0,vps.wd,vps.ht);
  glMatrixMode (GL_PROJECTION);
  glPopMatrix();
  glMatrixMode (GL_MODELVIEW);
  glPopMatrix();
  return;
}

//-------------------------------------------------------------------------
// Stop Drawing with this camera
//-------------------------------------------------------------------------
void CCamera::StopShoot()
{ glMatrixMode (GL_MODELVIEW);
  glPopMatrix();
	pick	= 0;
  //----------------------------------------------------------------
	/*
  {GLenum e = glGetError ();
  if (e != GL_NO_ERROR) 
    WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  }
	*/
  return;
}
//-------------------------------------------------------------------------
// Prepare OpenGL to project a 2D object on the viewport
//    The camera is set to a orthographic projection on the viewport
//    xOBJ must be a texture object binded to some texture
//-------------------------------------------------------------------------
void CCamera::Projection(VIEW_PORT &vp,U_INT xOBJ)
{ //-- Save and set   he viewport ----------------------------
  int wd  = vp.wd;
  int ht  = vp.ht;
  glGetIntegerv(GL_VIEWPORT,(GLint*)&vps);        // Save it
  //---Set projection on the QUAD ----------------------------
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D (0,wd,0, ht);
  //---Init QUAD coordinates ----------------------------------
  Pan[1].VT_X = wd;                       // SE corner
  Pan[2].VT_X = wd;                       // NE corner
  Pan[2].VT_Y = ht;
  Pan[3].VT_Y = ht;                       // NW corner
  //-----------------------------------------------------------
  // Initialize modelview matrix 
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  glLoadIdentity ();
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
  //----Set pre conditions ------------------------------------
  glViewport(vp.x0,vp.y0,wd,ht);
  glColor4f(1,1,1,1);
  glDisable(GL_LIGHTING);
  glEnable  (GL_TEXTURE_2D);
  glDisable (GL_DEPTH_TEST);
  glBindTexture(GL_TEXTURE_2D,xOBJ);
  glDisable(GL_BLEND);
  //----Draw the Quad at screen position ---------------------
  glPolygonMode(GL_FRONT,GL_FILL);
  glInterleavedArrays(GL_T2F_V3F,0,Pan);
  glDrawArrays(GL_QUADS,0,4);
  //----Restore everything -----------------------------------
  glViewport(vps.x0,vps.y0,vps.wd,vps.ht);
  glPopClientAttrib();
  glPopAttrib();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode (GL_MODELVIEW);
  glPopMatrix();
  return;
}
//-------------------------------------------------------------------------
// Prepare OpenGL to project a 2D object on the viewport
//    Same as above, but the texture is defined in TEXT_INFO
//    by the parameters
//    wd   width in pixels
//    ht   height
//    mADR :RGBA array of pixels
//-------------------------------------------------------------------------
void CCamera::Projection(VIEW_PORT &vp,TEXT_INFO &inf)
{ inf.xOBJ = 0;
  globals->txw->GetTexOBJ(inf,0,GL_RGBA);
  Projection(vp,inf.xOBJ);
  glDeleteTextures(1,&inf.xOBJ);
  return;
}
//-------------------------------------------------------------------------
// Rotate arround aircraft
//  -offset are in feet from aircraft position
//-------------------------------------------------------------------------
void CCamera::RockArround (SPosition tpos, SVector tori,float dT)
{ tgtPos = tpos;
	// Update offset to target position in world coordinates.  Note that spot camera
  // follows target vehicle orientation in heading only, not pitch or bank
  double atheta = WrapTwoPi(double(tori.z) + theta);
  double cosphi = cos(phi);
  offset.x =  range * sin (atheta) * cosphi;
  offset.y = -range * cos (atheta) * cosphi;
  offset.z =  range * sin (phi);

  // Update orientation
  orient.x = phi;
  orient.y = 0;
  orient.z = atheta;
  return;
}
//-------------------------------------------------------------------------
// Update camera world position
//  -offset are in feet from aircraft position
//-------------------------------------------------------------------------
void CCamera::UpdateCameraPosition(SPosition &wpos)
{	camPos			= AddToPositionInFeet(wpos,offset,globals->exf);
	double	alt = globals->tcm->GetGroundAltitude() + minAGL;
  //---Update phi if camera is too low -------------------------
  if (camPos.alt < alt)  phi += DegToRad(0.25f);
}
//-------------------------------------------------------------------------
// Pan Left - rotate clockwise
//-------------------------------------------------------------------------
void CCamera::RoundLeft (void)
{	theta = WrapTwoPi (theta - DegToRad (double(0.5)));	}
//-------------------------------------------------------------------------
// Pan Right - rotate counterclockwise
//-------------------------------------------------------------------------
void CCamera::RoundRight (void)
{	theta = WrapTwoPi (theta + DegToRad (double(0.5)));	}
//-------------------------------------------------------------------------
// Pan Up - rotate towards top of model, clamping at just less than 90 degrees
//-------------------------------------------------------------------------
void CCamera::RoundUp (void)
{ phi += DegToRad (double(0.25));
  if (phi > clamp) phi = clamp;
}
//-------------------------------------------------------------------------
// Pan Down - rotate towards bottom of model, clamping at just less than 90 degrees
//-------------------------------------------------------------------------
void CCamera::RoundDown (void)
{ double ang = phi - DegToRad (double(0.25));
  if (GoodHeight(ang)) phi = ang;
  return;
}

//------------------------------------------------------------------------
void CCamera::Print (FILE *f)
{
  fprintf (f, "CCamera : \n");
  fprintf (f, "  Horizontal FOV:   %f deg\n", fov);
  fprintf (f, "  Pitch (deg)   :   %f\n", RadToDeg(orient.x));
  fprintf (f, "  Bank (deg)    :   %f\n", RadToDeg(orient.y));
  fprintf (f, "  Heading (deg) :   %f\n", RadToDeg(orient.z));
  fprintf (f, "  Range         :   %f (%f/%f)\n", range, rmin, rmax);
  fprintf (f, "  Frustum :\n");
  for (int i=0; i<6; i++) {
  }
}

/*==================================================================================
 * CCockpitPanel class represents one node in the graph of interconnected
 *   cockpit panels encapsulated within the CCockpitCamera object.
 *
 * Each node is identified by the unique panel ID (which is passed to
 *   CCockpitManager to activate the current panel) and the IDs of
 *   the four panels to the left, right, up and down.
 */
//==================================================================================

typedef enum {
  PANEL_LEFT    = 0,
  PANEL_RIGHT   = 1,
  PANEL_UP      = 2,
  PANEL_DOWN    = 3
} EPanelScrollDirection;
//==========================================================================================
// This class represents a single instance of a cockpit panel within a cockpit
//   camera definition.  Members include the unique ID tag of the panel,
//   the orientation of the eye position when that panel is active, and
//   a list of which panel ID tags are situated to the left, right, up and down
//   from the panel.
//
// Example:
//
//    <panl> ---- panel ----
//    <bgno> ---- begin ----
//      <id__> ---- id ----
//      frnt
//      <main> ---- default ----
//      <hdg_> ---- heading ----
//      0.0
//      <ptch> ---- pitch ----
//      7.4
//      <pnls> ---- panels (L,R,U,D) ----
//      uplt
//      uprt
//      NONE
//      floo
//    <endo> ---- end ----
//  JS NOTE: Each panel has 2 vectors defining the Up position and the LookAt position (forward)
//
//==========================================================================================
class CCockpitPanel : public CStreamObject {
public:
  CCockpitPanel (CCameraCockpit *c);
  //--- CStreamObject methods -----------------------------------------
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  void  SetPanel(CPanel *p);
	//--------------------------------------------------------------------
  inline CPanel  *GetPanel()          {return panel;}
  inline void     GetXSRC(TC_4DF &r){r.x1 = xscr_x; r.y1 = xscr_y;}
  //--------------------------------------------------------------------
public:
  Tag     id;         ///< Panel ID, as specified in a PNL file
	char    idn[8];			// Tag for debug
  CPanel *panel;      // Panel 
  bool    main;       ///< Is this the main (default) panel?
  float   hdg;        ///< Viewpoint heading (y-axis rotation)
  float   pit;        ///< Viewpoint pitch (x-axis rotation)
  Tag     pnls[4];    ///< Links to IDs of other panels in cockpit
  float   xscr_x;     ///< Exterior scroll factor
  float   xscr_y;
  int     umdl;       ///< Use exterior model
  //---Camera cockpit-------------------------------------
  CCameraCockpit *cam;
  //---Matrix for local head orientation ----------------------------
  sgMat4  rotH;       // head rotation matrix
};
//============================================================================================
//	Camera used for cockpit display
//============================================================================================
CCockpitPanel::CCockpitPanel (CCameraCockpit *c) :
  id (0),
  main (false),
  hdg (0.0f),
  pit (0.0f)
{ panel   = 0;
  cam     = c;
  pnls[0] = pnls[1] = pnls[2] = pnls[3] = StringToTag ("NONE");
  xscr_x = xscr_y = 0;
  umdl = 0;
}
//----------------------------------------------------------------------
//  Read all parameters
//-----------------------------------------------------------------------
int CCockpitPanel::Read (SStream *stream, Tag tag)
{ float pm;
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'id__':
    ReadTag (&id, stream);
		TagToString(idn,id);
    return TAG_READ;

  case 'main':
    main = true;
    rc = TAG_READ;
    break;
  //--JS set positive direction for heading ---------------------
  case 'hdg_':
    ReadFloat (&hdg, stream);
    hdg = (360 - hdg);
    rc = TAG_READ;
    break;

  case 'ptch':
    ReadFloat (&pit, stream);
    rc = TAG_READ;
    break;

  case 'pnls':
    // Read four panel ID tags corresponding to the panels to the L, R, U and D
    ReadTag (&pnls[0], stream);
    ReadTag (&pnls[1], stream);
    ReadTag (&pnls[2], stream);
    ReadTag (&pnls[3], stream);
    rc = TAG_READ;
    break;

  case 'ofst':
    ReadFloat (&pm, stream);
    ReadFloat (&pm, stream);
    ReadFloat (&pm, stream);
    rc = TAG_READ;
    break;

  case 'xscr':
    ReadFloat (&xscr_x, stream);
    ReadFloat (&xscr_y, stream);
    rc = TAG_READ;
    break;

  case 'umdl':
    ReadInt (&umdl, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    WARNINGLOG ("CCockpitPanel::Read : Unknown tag %s", TagToString(tag));
  }

  return rc;
}
//----------------------------------------------------------------------
//  All parameter are read
//-----------------------------------------------------------------------
void CCockpitPanel::ReadFinished()
{ // Compute Head rotation according to pitch and heading

  return;
}
//----------------------------------------------------------------------
//  Set panel and panel attribute
//-----------------------------------------------------------------------
void CCockpitPanel::SetPanel(CPanel *p)
{	panel = p;
	p->SetMain(main);
	return;
}

//=====================================================================================
// Cockpit (vehicle interior) camera
//=====================================================================================
CCameraCockpit::CCameraCockpit (CVehicleObject *mv)
{ mveh	 = mv;
	Prof.Set(CAM_MAY_ZOOM);
	Rate	 = 2;						// Default rotation rate
	//-----------------------------------------------
	Seat.x = Seat.y = Seat.z = 0;
	intcm		= 1;						// Change to internal camera
	extcm		= 0;
  //--- Initialize active panel to none ----------
  ckPanel = 0;
  //--- Link to cameras -------------------------
  cIden = CAMERA_COCKPIT;
  cNext = CAMERA_SPOT;
  cPrev = CAMERA_ORBIT;
  //----Init Up vector  -------------------------
  Up.x  = 0;
  Up.y  = 0;
  Up.z  = 1;
  Head  = 0;
  //---------------------------------------------
  nearP = 5;
}
//--------------------------------------------------------------------------
//  Free camera 
//--------------------------------------------------------------------------
CCameraCockpit::~CCameraCockpit (void)
{  // Delete panels
  std::map<Tag,CCockpitPanel*>::iterator ra;
  for (ra=panl.begin(); ra!=panl.end(); ra++) delete ra->second;
  panl.clear();
}
//--------------------------------------------------------------------------
//  Read camera parameters
//--------------------------------------------------------------------------
int CCameraCockpit::Read (SStream *stream, Tag tag)
{
  switch (tag) {
  case 'seat':
    // Read seat orientation (in degrees relative to straight ahead)
    ReadDouble (&Seat.z, stream);
    ReadDouble (&Seat.x, stream);
    ReadDouble (&Seat.y, stream);

    // Read seat offset (in feet from model center)
    ReadDouble (&Ofs.x, stream);
    ReadDouble (&Ofs.z, stream);
    ReadDouble (&Ofs.y, stream);
    return TAG_READ;

  case 'panl':
    { //MEMORY_LEAK_MARKER ("cock_panel");
      CCockpitPanel *cock_panel = new CCockpitPanel(this);
      //MEMORY_LEAK_MARKER ("cock_panel");
      ReadFrom (cock_panel, stream);
      Tag tag   = cock_panel->id;
      panl[tag] = cock_panel;
    }
    return TAG_READ;
  }
  WARNINGLOG ("CCockpitPanel::Read : Unknown tag %s", TagToString(tag));
  return TAG_IGNORED;
}
//--------------------------------------------------------------------------
//  All parameters are read
//--------------------------------------------------------------------------
void CCameraCockpit::ReadFinished (void)
{ // Set the default panel ID to the first with the <main> tag present
  std::map<Tag,CCockpitPanel*>::iterator iter;
  for (iter=panl.begin(); iter!=panl.end(); iter++) {
    CCockpitPanel *p = iter->second;
    if (p->main) {
      if (ckPanel == NULL) {
        // This is the first <main> panel...assign it as the default
        ckPanel = p;
      } else {
        // There should be only one <main> panel, generate a warning
        WARNINGLOG ("CCameraCockpit : Multiple <main> panels");
      }
    }
  }
}
//----------------------------------------------------------------------
//  Cockpit camera uses its position translated by the Forward vector
//	as target
//-----------------------------------------------------------------------
void	CCameraCockpit::CameraReferential()
{	SPosition P;
	P.lon	= (camPos.lon + Fw.x);
	P.lat	= (camPos.lat + Fw.y);
	P.alt = (camPos.alt + Fw.z);
	SetReferential(P);
	return;
}

//------------------------------------------------------------------------------------
//	Compute camera position using openGL matrix operations
//	World position is the world position of aircraft
//------------------------------------------------------------------------------------
void CCameraCockpit::UpdateCamera (SPosition wpos, SVector ori ,float dT)
{	tgtPos = wpos;
	//----- Adjust orientation by fixed seat orientation ----
  CVector ors = Seat;
	ors.x += ckPanel->pit;
  ors.z += ckPanel->hdg;
  //--Adjust for pilot Head ----------------------
  ors.x += Head;
	//--- Rotate all -------------------------------
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glLoadMatrixd( mveh->GetROTM());				// Global aircraft rotation
	glRotated(+ors.z,0,0,1);				// Head   pilot eye (Z)
	glRotated(-ors.x,1,0,0);				// Pitch	pilot eye (X)
	//--- Save matrix ------------------------------
  glGetDoublev(GL_MODELVIEW_MATRIX,mSEAT);
  glPopMatrix();
	//--- Compute forward vector ----------------------
	CVector fw(0,1000,0);
	fw.MultMatGL(mSEAT,Fw);
	//--- Compute up vector ---------------------------
	CVector up(0,0,1);
	up.MultMatGL(mSEAT,Up);
	//--- Now compute offset --------------------------
  Ofs.MultMatGL(mSEAT,offset);
	//--- Update camera world position ----------------
	camPos  = AddToPositionInFeet(wpos,offset, globals->exf);
	return;
}

//=====================================================================================
// Return camera lookat point
//=====================================================================================
void CCameraCockpit::GetLookatPoint (SVector &v)
{ v = Fw;
}
//------------------------------------------------------------------------
//  Internal camera is activated
//-------------------------------------------------------------------------
void CCameraCockpit::ChangeResolution()
{ CPanel *panel = ckPanel->GetPanel();
  if (0 == panel) return;
  panel->SetViewPort();
  return;
}
//------------------------------------------------------------------------
//  Return Panel
//-------------------------------------------------------------------------
CPanel*	CCameraCockpit::GetPanel()
{	return ckPanel->GetPanel();	}
//------------------------------------------------------------------------
//  Draw panel
//-------------------------------------------------------------------------
void CCameraCockpit::DrawPanel()
{ CPanel *panel = ckPanel->GetPanel();
	if (panel) panel->Draw(this);
  return;
}
//------------------------------------------------------------------------
//  Find cockpit panel by tag
//------------------------------------------------------------------------
void  CCameraCockpit::ActivateCockpitPanel (Tag tag)
{ if (tag == 'NONE')    return;
  std::map<Tag,CCockpitPanel*>::iterator it = panl.find(tag);
  CCockpitPanel *cp = it->second;
  CPanel        *pn = cp->GetPanel();
	ckPanel = cp;
  if (pn)   pn->SetViewPort();
	cp->SetPanel(pn);
  return;
}
//------------------------------------------------------------------------
//  Set panel pointer
//------------------------------------------------------------------------
void CCameraCockpit::SetPanel(Tag id,CPanel *p)
{ std::map<Tag,CCockpitPanel*>::iterator it = panl.find(id);
  if (it == panl.end()) return;
  CCockpitPanel *cpn = (*it).second;
  cpn->SetPanel(p);
  return;
}
//------------------------------------------------------------------------
//  return current cockpit panel 
//------------------------------------------------------------------------
Tag  CCameraCockpit::GetCockpitPanel (void)
{ return (ckPanel)?(ckPanel->id):(0);
}
//------------------------------------------------------------------------
//  return current cockpit panel 
//------------------------------------------------------------------------
void CCameraCockpit::GetXSRC(TC_4DF &r)
{ if (0 == ckPanel) return;
  ckPanel->GetXSRC(r);
}
//------------------------------------------------------------------------
// Pan Left - switch panels to the one to the left
//------------------------------------------------------------------------
void CCameraCockpit::PanLeft (void)
{ if (ckPanel) {
    Tag tag = ckPanel->pnls[PANEL_LEFT];
    ActivateCockpitPanel (tag);
  }
}
//------------------------------------------------------------------------
// Pan Right - switch panels to the one to the right
//------------------------------------------------------------------------
void CCameraCockpit::PanRight (void)
{ if (ckPanel) {
    Tag tag = ckPanel->pnls[PANEL_RIGHT];
    ActivateCockpitPanel (tag);
  }
}
//------------------------------------------------------------------------
// Pan Up
//------------------------------------------------------------------------
void CCameraCockpit::PanUp (void)
{ if (ckPanel) {
    Tag tag = ckPanel->pnls[PANEL_UP];
    ActivateCockpitPanel (tag);
  }
}
//------------------------------------------------------------------------
// Pan Down
//------------------------------------------------------------------------
void CCameraCockpit::PanDown (void)
{ if (ckPanel) {
    Tag tag = ckPanel->pnls[PANEL_DOWN];
    ActivateCockpitPanel (tag);
  }
}
//------------------------------------------------------------------------
// User preset positions
//------------------------------------------------------------------------
void CCameraCockpit::User1 (void)
{  DrawNoticeToUser ("User Position 1", 5);}

void CCameraCockpit::DefineUser1 (void)
{ DrawNoticeToUser ("Defined User Position 1", 5);}

void CCameraCockpit::User2 (void)
{ DrawNoticeToUser ("User Position 2", 5);}

void CCameraCockpit::DefineUser2 (void)
{ DrawNoticeToUser ("Defined User Position 2", 5);}

void CCameraCockpit::User3 (void)
{ DrawNoticeToUser ("User Position 3", 5);}

void CCameraCockpit::DefineUser3 (void)
{ DrawNoticeToUser ("Defined User Position 3", 5);}

void CCameraCockpit::User4 (void)
{ DrawNoticeToUser ("User Position 4", 5);}

void CCameraCockpit::DefineUser4 (void)
{ DrawNoticeToUser ("Defined User Position 4", 5);}

//--------------------------------------------------------------------------
//  Adjust pilot head pitch around  X axis
//--------------------------------------------------------------------------
void CCameraCockpit::HeadPitchUp (void)
{ Head += 0.25;
  if (Head > 10)   Head = 10;
}
//--------------------------------------------------------------------------
//  Adjust pilot head pitch around  X axis
//--------------------------------------------------------------------------
void CCameraCockpit::HeadPitchDown (void)
{ Head -= 0.25;
  if  (Head < - 10)  Head = -10;
}
//============================================================================================
//	Camera used for editor purpose
//============================================================================================
CRabbitCamera::CRabbitCamera()
: CCamera()
{ Prof.Set(CAM_IS_SPOT);
  theta = DegToRad (0.0f);
  phi   = DegToRad (30.0f);
	//---------------------------------------------
	trak	  = 0;		// No tracker
	twin		= 0;		// No window
  //--- Link to cameras -------------------------
  cIden = CAMERA_RABBIT;
  cNext = 0;
  cPrev = 0;
	rmax	= 12000;
	range = 5000;
}
//--------------------------------------------------------------------------
//  Release camera and restore contexte
//--------------------------------------------------------------------------
CRabbitCamera::~CRabbitCamera()
{	}
//-------------------------------------------------------------------------
// Save tracker
//-------------------------------------------------------------------------
void CRabbitCamera::SetTracker(Tracker *t, CFuiWindow *w )
{	trak	= t;	
	twin	= w;}
//-------------------------------------------------------------------------
//  Start picking by loading picking matrix
//	This function should be called by the camera when Perspective matrix
//	mode is set and before the gluPerspective() is called
//-------------------------------------------------------------------------
void CRabbitCamera::StartPicking()
{ if (0 == pick)	return;
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT,vp);
	int   hy = vp[3] - py + vp[1];
	gluPickMatrix(double(px),double(hy),10,10,vp);
	return;
}
//-------------------------------------------------------------------------
//	Set Rabbit orientation
//-------------------------------------------------------------------------
void	CRabbitCamera::TurnRabbit(SVector v)
{ SVector d;
  d.x = RadToDeg(v.x);
  d.y = RadToDeg(v.y);
  d.z = RadToDeg(v.z);
  //----Save position at global level ----------------
  globals->iang = v;
  globals->dang = d;
	return;
}
//-------------------------------------------------------------------------
//  Rabbit camera intercept slew move to new position
//-------------------------------------------------------------------------
void CRabbitCamera::RabbitMoveTo(SPosition *pos)
{	return;		}
//-------------------------------------------------------------------------
//  Rabbit camera does not turn arround veh but rather turns the veh
//	so the slew always face the camera
//-------------------------------------------------------------------------
void CRabbitCamera::RabbitLeft()
{	SVector ori = globals->iang;
	ori.z       = WrapTwoPi (ori.z - DegToRad (double(0.5)));	
	TurnRabbit(ori);
	return;
}
//-------------------------------------------------------------------------
//  Rabbit camera does not turn arround veh but rather turns the veh
//	so the slew always face the camera
//-------------------------------------------------------------------------
void CRabbitCamera::RabbitRight()
{	SVector ori = globals->iang;
	ori.z       = WrapTwoPi (ori.z + DegToRad (double(0.5)));	
	TurnRabbit(ori);
	return;
}
//----------------------------------------------------------------------
//	Pick objects
//	-Draw all markers in pick mode in a small cube centered on mouse
//	 to detect if a hit occured.
//	When a hit is detected, the selction is changed to the new marker
//----------------------------------------------------------------------
bool CRabbitCamera::PickObject(U_INT mx, U_INT my)
{	//--- Init picking -------------------------------------
	pick	= 1;
	px		= mx;
	py		= my;
	glSelectBuffer(8,bHit);						// Supply buffer
	glRenderMode(GL_SELECT);					// Start select mode
	glInitNames();										// Init stack name
	glPushName(0);										// room for one name
	StartShoot(0);										// Init camera
	trak->DrawMarks();											// Redraw markers
	StopShoot();											// Stop Drawing
	int hit = glRenderMode(GL_RENDER);
	pick	= 0;
	if (0 == hit)					return false;
	//--- change selected vertex ---------------------------
	U_INT vno = bHit[3];							// Number hited
	trak->OneSelection(vno);
	twin->OnePicking(vno);
	return true;
}
//-------------------------------------------------------------------------
//  Update camera position
//  -offset are in feet from aircraft posiiton
//-------------------------------------------------------------------------
void CRabbitCamera::UpdateCamera (SPosition wpos, SVector tori,float dT)
{	RockArround(wpos,tori,dT);
	//--- Update camera world position ---------------------
	UpdateCameraPosition(wpos);
	return;
}
//=========================================================================
// CCameraSpot
//
// External spot camera, fixed orientation with user vehicle
//=========================================================================
CCameraSpot::CCameraSpot (void)
: CCamera()
{ Prof.Set(CAM_IS_SPOT);
  theta = DegToRad (30.0f);
  phi   = DegToRad (15.0f);
  //--- Link to cameras -------------------------
  cIden = CAMERA_SPOT;
  cNext = CAMERA_OBSERVER;
  cPrev = CAMERA_COCKPIT;
	rmax  = 15000;
}
//-------------------------------------------------------------------------
//  Update camera position
//  -offset are in feet from aircraft posiiton
//-------------------------------------------------------------------------
void CCameraSpot::UpdateCamera (SPosition wpos, SVector tori,float dT)
{	RockArround(wpos,tori,dT);
	//--- Update camera world position -------------------
	UpdateCameraPosition(wpos);
	return;
}
//-------------------------------------------------------------------------
// Set the camera position
//-------------------------------------------------------------------------
void  CCameraSpot::SetCameraPosition (const float &pitchInRads, const float &headingInRads, const float &distanceInFeet)
{ theta = -headingInRads;
  phi   =  pitchInRads;
  range =  distanceInFeet;
}
//-------------------------------------------------------------------------
// Pixel move
//-------------------------------------------------------------------------
void CCameraSpot::MoveBy(int px, int py)
{ double r1 = double(0.004) * px;
  double r2 = double(0.004) * py;
  theta = WrapTwoPi (theta - r1);
  phi   = WrapTwoPi (phi   - r2);
  return;
}
//-------------------------------------------------------------------------
//  Set Camera Range to see extension D at angle A
//-------------------------------------------------------------------------
void CCameraSpot::RangeFor(double D, double A)
{double ang = DegToRad(A);
 double hm  = D * 0.5;
 double dr = hm / tan(ang);
 SetRange(dr);
 return;
}

//================================================================================
// CCameraObserver
//
// External spot camera, fixed orientation with ground
//================================================================================
CCameraObserver::CCameraObserver (void)
: CCamera()
{ Prof.Set(CAM_MAY_MOVE | CAM_MAY_ZOOM | CAM_VERT_ROT | CAM_SIDE_ROT);
  theta = DegToRad (-30.0f);
  phi   = DegToRad (15.0f);
  //--- Link to cameras -------------------------
  cIden = CAMERA_OBSERVER;
  cNext = CAMERA_FLYBY;
  cPrev = CAMERA_SPOT;

}
//-------------------------------------------------------------------------------
//  Compute Camera offset
//-------------------------------------------------------------------------------
void CCameraObserver::UpdateCamera (SPosition wpos, SVector tori,float dT)
{ RockArround(wpos,tori,dT);
	//--- Update camera world position -------------------
	UpdateCameraPosition(wpos);
  return;
}
//-------------------------------------------------------------------------------
// Pan Left - rotate clockwise
//-------------------------------------------------------------------------------
void CCameraObserver::PanLeft (void)
{ theta = WrapTwoPi (theta - ONE_DEGRE_RADIAN); 
	return;	}
//-------------------------------------------------------------------------------
// Pan Right - rotate counterclockwise
//-------------------------------------------------------------------------------
void CCameraObserver::PanRight (void)
{ theta = WrapTwoPi (theta + ONE_DEGRE_RADIAN); 
	return;	}
//-------------------------------------------------------------------------------
// Pan Up - rotate towards top of model, clamping at 90 degrees
//-------------------------------------------------------------------------------
void CCameraObserver::PanUp (void)
{ phi += DegToRad (0.25f);
  phi  = ClampTo(phi,-HALF_PI,+HALF_PI);
	return;	}
//-------------------------------------------------------------------------------
// Pan Down - rotate towards bottom of model, clamping at 90 degrees
//-------------------------------------------------------------------------------
void CCameraObserver::PanDown (void)
{ double ang = phi - DegToRad (0.25f);
  if (GoodHeight(ang)) phi = ang;
  return;	}
//-------------------------------------------------------------------------------
//  Set camera position
//-------------------------------------------------------------------------------
void  CCameraObserver::SetCameraPosition (const float &pitchInRads, const float &headingInRads, const float &distanceInFeet)
{ 
  theta = -headingInRads;
  phi   = -pitchInRads;
  range = distanceInFeet;
}
//===================================================================================
// Flyby Camera
//===================================================================================
CCameraFlyby::CCameraFlyby (void)
{ // This camera cannot be manually handled
  Prof.Set(CAM_MAY_ZOOM);
  // Default position to an arbitrary value to cause position recalc on first update
  camPos.lat = camPos.lon = camPos.alt = 0;
  // Temporarily set orientation based on position NE of the target
  orient.x = 0;
  orient.y = 0;
  orient.z = DegToRad (225.0);
  // Default range for flyby camera
  rng    = 1000.0;
  distance = 750.0;
  r0       = range;
  //---Link to camera --------------
  cIden = CAMERA_FLYBY;
  cNext = CAMERA_TOWER;
  cPrev = CAMERA_OBSERVER;
}
//-------------------------------------------------------------------------------
//  Update camera offset
//-------------------------------------------------------------------------------
void CCameraFlyby::UpdateCamera (SPosition wPos, SVector tori,float dT)
{ tgtPos = wPos;
	CVector v = SubtractPositionInFeet(wPos, camPos);
  if (v.Length() > rng || range != r0) {
    rng = range * 17.50;
    r0  = range;
    //TRACE ("%f", r);
    // Recalculate position
    CRotationMatrixHPB matx;                                            // LH
    CVector _angle (0.984, 0.0, 0.173);  // cos - sin 10°               // LH
    _angle.Times (range * 15.0 /*distance*/);
    SVector or_m = {0.0, 0.0, 0.0};                                     // LH
    or_m.y = TWO_PI - tori.z;     // + is right                    // RH to LH
    matx.Setup (or_m);                                                  // LH
    SVector dir, w_dir;                                                 // LH
    matx.ParentToChild (w_dir, _angle);                                 // LH
    dir.x = w_dir.z; dir.y = w_dir.x; dir.z = 0.0;
    // set camera to the NE of target position
    // camPos = AddToPositionInFeet(wPos, dir, globals->exf);
    v = dir;//	SubtractPositionInFeet (wPos, camPos);
  }
  offset.x = v.x;
  offset.y = v.y;
  offset.z = v.z;
  camPos = AddToPositionInFeet(wPos, offset, globals->exf);
  //---Update phi if camera is too low -------------------------
	double alt = globals->tcm->GetGroundAltitude() + minAGL;
  if (camPos.alt < alt)  camPos.alt += 4 ;
  return;
}
//==============================================================================
// External tower camera
//	Camera tower may operate in two modes:
//	Track mode:  The camera is set to the nearest airport tower and focus on the
//							aircraft
//	Manual mode:	The camera is operated manually.
//								
//==============================================================================
CCameraTower::CCameraTower (void)
: CCamera()
{ Prof.Set(CAM_SIDE_ROT + CAM_VERT_ROT + CAM_MAY_ZOOM); 
	Mode				= CAM_TRK_MODE;
  t_zoomRatio = 40.0f;
  //--- Default range for flyby camera
  distance		= 750.0;
  r0					= range;
  //--- Temporarily set camera a mile N and E of the vehicle, at same altitude
  offset.x		= NmToFeet (1.0f);
  offset.y		= NmToFeet (1.0f);
  offset.z		= 0;

  //--- Temporarily set orientation based on known position NE of the target
  orient.x	= 0;
  orient.y	= 0;
  orient.z	= 0;

  //---Link to camera ----------------
  cIden = CAMERA_TOWER;
  cNext = CAMERA_OVERHEAD;
  cPrev = CAMERA_FLYBY;

  //--- nearest airport --------------
  camPos.lat = 135428.40;
  camPos.lon = 855450.56;
  camPos.alt = 50.0;
  twr_timer = 15.0f;
}
//-------------------------------------------------------------------------------
//  Return look at point
//-------------------------------------------------------------------------------
void CCameraTower::GetLookatPoint(SVector &tgp)
{ tgp = tpos;
}
//-------------------------------------------------------------------------------
//  Compute camera angles from vector in degre for manual control
//-------------------------------------------------------------------------------
void	CCameraTower::ComputeAngle(CVector &v)
{	double ang1 = atan2(v.x,v.y);
  double hdis = v.GroundDistance();
	double ang2 = atan2(v.z,hdis);
	range	= 1000;
	orient.z	= RadToNorth(-ang1);
	orient.x  = 0;
	return;
}
//-------------------------------------------------------------------------------
//  Compute camera offset from aircraft
//-------------------------------------------------------------------------------
CVector CCameraTower::ComputeOffset(SPosition tgp)
{	CVector v = SubtractPositionInFeet (tgp, camPos);
	// Set camera offset from aircraft ------ 
	offset.x = v.x;
	offset.y = v.y;
	offset.z = v.z;
	return v;
}
//-------------------------------------------------------------------------------
//  Update camera offset
//-------------------------------------------------------------------------------
void CCameraTower::UpdateCamera (SPosition wPos, SVector tgtOrient,float Dt)
{ tgtPos = wPos;
	switch (Mode)	{
		case CAM_TRK_MODE:
			{	UpdatePosition(Dt);
				ComputeOffset(wPos);
				return;
			}
		case CAM_MAN_MODE:
			{ DrawNoticeToUser("Tower Camera in manual mode",2);
				CVector v = ComputeOffset(wPos);
				//--- compute target position -------------
       	double hprj = range * cos(orient.x);
				tpos.x			= hprj  * cos(orient.z);
				tpos.y			= hprj  * sin(orient.z);
				tpos.z			= range * sin(orient.x);
				//--- relative to camera ------------------
				tpos.Add(offset);
				return;
			}
	}
return;
}
//-------------------------------------------------------------------------------
//  Toggle camera mode
//  -In track mode, the target is origin,i.e where the aircraft is located at
//	rendering time.  The camera is set at the nearest airport location
//	-In Manual, the camera stay on top of airport tower and can be rotated lef-right
//	and up down.
//-------------------------------------------------------------------------------
bool CCameraTower::ToggleMode()
{	switch(Mode)	{
	//--- Set track mode ---------------------
	case CAM_TRK_MODE:
		{	Mode = CAM_MAN_MODE;
			//--- Compute theta and phi from positions -----
			CVector v = SubtractPositionInFeet (camPos,globals->geop);	// v is in Feet
			ComputeAngle(v);
			return true;
		}
  //--- Set Manual mode --------------------
	case CAM_MAN_MODE:
		Mode = CAM_TRK_MODE;
		tpos.Set(0,0,0);
		return true;
	}
	return true;
}
//------------------------------------------------------------------------
// Camera Pan left 
//------------------------------------------------------------------------
void CCameraTower::PanLeft()
{	orient.z += DegToRad(double(0.5));
	orient.z  = WrapTwoPi(orient.z);
}
//------------------------------------------------------------------------
// Camera Pan right 
//------------------------------------------------------------------------
void CCameraTower::PanRight()
{	orient.z -= DegToRad(double(0.5));
	orient.z  = WrapTwoPi(orient.z);
}
//------------------------------------------------------------------------
// Camera Pan Up 
//------------------------------------------------------------------------
void CCameraTower::PanUp()
{	orient.x += DegToRad(double(0.5));
	orient.x  = ClampTo(orient.x,-HALF_PI,+HALF_PI);
}
//------------------------------------------------------------------------
// Camera Pan Down 
//------------------------------------------------------------------------
void CCameraTower::PanDown()
{	orient.x -= DegToRad(double(0.5));
	orient.x  = WrapHalfPi(orient.x);
	return;
}
//-------------------------------------------------------------------------
// GetNearestAirport
//-------------------------------------------------------------------------
void CCameraTower::GetNearestAirport (void)
{ CObjPtr apt = 0;
  globals->dbc->GetNearestAirport(apt);
	if (apt.Assigned())	camPos = apt->GetPosition();
	else			          camPos = globals->geop;
	camPos.alt += 50;							
	return;
}
//-------------------------------------------------------------------------
// Update camera position in track mode
//-------------------------------------------------------------------------
void CCameraTower::UpdatePosition(float Dt)
{ // Periodically lookup nearest airport and set ground position
  twr_timer += Dt;
  if (twr_timer < 10.0f) return;
  twr_timer = twr_timer - 10.0f;
  //------ Get an airport and set position ------
  GetNearestAirport ();
  return;
}
//-------------------------------------------------------------------------
// ZoomIn
//-------------------------------------------------------------------------
void CCameraTower::ZoomRatioIn (void)
{ if (Prof.Not(CAM_MAY_ZOOM))	return;
	char txt[256];
  t_zoomRatio -= 0.25f;
  if (t_zoomRatio < 0.15f) t_zoomRatio = 0.15f;
  fov = t_zoomRatio;
  SetMinAGL();
  _snprintf(txt,255,"FOV: %02.0f °",fov);
  globals->fui->DrawNoticeToUser(txt,5);
  return;
}
//-------------------------------------------------------------------------
// ZoomOut
//-------------------------------------------------------------------------
void CCameraTower::ZoomRatioOut (void)
{ if (Prof.Not(CAM_MAY_ZOOM))	return;
	char txt[256];
  t_zoomRatio += 0.25f;
  if (t_zoomRatio > 90.0f) t_zoomRatio = 90.0f;
  fov = t_zoomRatio;
  SetMinAGL();
  _snprintf(txt,255,"FOV: %02.0f °",fov);
  globals->fui->DrawNoticeToUser(txt,5);
  return;
}

//==============================================================================
// Overhead camera
//==============================================================================
CCameraOverhead::CCameraOverhead (void)
: CCamera()
{ Prof.Set(CAM_MAY_MOVE + CAM_MAY_ZOOM);
	Up.x = 0;
  Up.z = 0;
  Up.y = 1.0f;
  // Increase rmax to allow very high zoom range
  rmax = NmToFeet (30.0f);
  
  // Initialize orientation to point straight down
  orient.x = DegToRad (90.0);
  orient.y = 0;
  orient.z = 0;
  //---Link to camera --------------
  cIden = CAMERA_OVERHEAD;
  cNext = CAMERA_ORBIT;
  cPrev = CAMERA_TOWER;
}
//-------------------------------------------------------------------------------
//  Update camera offset
//-------------------------------------------------------------------------------
void CCameraOverhead::UpdateCamera (SPosition wpos, SVector tgtOrient,float Dt)
{ tgtPos = wpos;
  //---------- Update position ---------------------------
  offset.x = 0;
  offset.y = 0;
  offset.z = range;
	//--- Update camera world position --------------------
	camPos			= wpos;
	camPos.alt	= range + wpos.alt;
}
//=====================================================================================
// Orbit camera
//=====================================================================================
CCameraOrbit::CCameraOrbit (void)
: CCamera()
{ Prof.Set(CAM_VERT_ROT + CAM_MAY_MOVE + CAM_MAY_ZOOM);
  // Initialize orbit rate
  orbitRate = 0;
  // Initialize theta and phi angles
  theta = DegToRad (-30.0f);
  phi   = DegToRad (15.0f);
  //---Link to camera --------------
  cIden = CAMERA_ORBIT;
  cNext = CAMERA_COCKPIT;
  cPrev = CAMERA_OVERHEAD;
}
//------------------------------------------------------------------------------------
//  Update camera offset and orientation
//------------------------------------------------------------------------------------
void CCameraOrbit::UpdateCamera (SPosition wpos, SVector tgtOrient,float dT)
{ tgtPos = wpos;
	theta += (dT * orbitRate);
  theta  = WrapTwoPi(theta);
  double cosphi = cos(phi);
  // Update heading rotation
  offset.x =  range * sin (theta) * cosphi;
  offset.y = -range * cos (theta) * cosphi;
  offset.z =  range * sin (phi);

  // Update orientation
  orient.x = -phi;
  orient.y = 0;
  orient.z = -theta;
	//--- Update Camera world position ---------------------------
	UpdateCameraPosition(wpos);
  return;
}
//------------------------------------------------------------------------------------
// Pan Left - Orbit clockwise looking from above
//------------------------------------------------------------------------------------
void CCameraOrbit::PanLeft (void)
{ orbitRate -= DegToRad (0.25f);
  if (orbitRate < -1) orbitRate = -1;
  return;
}

//------------------------------------------------------------------------------------
// Pan Right - Orbit counterclockwise looking from above
//------------------------------------------------------------------------------------
void CCameraOrbit::PanRight (void)
{ orbitRate += DegToRad (0.25f);
  if (orbitRate > 1) orbitRate = 1;
  return;
}
//------------------------------------------------------------------------------------
// Pan Up - rotate towards top of model
//------------------------------------------------------------------------------------
void CCameraOrbit::PanUp (void)
{ phi += ONE_DEGRE_RADIAN;
  if (phi > clamp) phi = clamp;
  return;
}
//------------------------------------------------------------------------------------
// Rotate - Orbit counterclockwise looking from above
//------------------------------------------------------------------------------------
void CCameraOrbit::Rotate(double deg)
{ orbitRate = DegToRad(deg);
  if (orbitRate < -1.0f) orbitRate = -1.0f;
  if (orbitRate >  1.0f) orbitRate =  1.0f;
  return;
}
//-----------------------------------------------------------------------
// Pan Down - rotate towards bottom of model
//  check if camera is lower than ground + 25 at zoom = 1
//  if so => Do not move
//
//-----------------------------------------------------------------------
void CCameraOrbit::PanDown (void)
{ double ang = phi - ONE_DEGRE_RADIAN;
  if (GoodHeight(ang))  phi = ang;
  return;
}
//==============================================================================
// Runway camera: set over an airport to draw the runways
//  NOTE:  The camera position must be in arcsecs for X,Y and feet for Z
//==============================================================================
CCameraRunway::CCameraRunway (void)
: CCamera()
{ Prof.Set(CAM_MAY_MOVE);
  // Increase rmax to allow very high zoom range
  rmax = NmToFeet (10.0f);
  rmin = 100;
  // Initialize orientation to point straight down
  orient.x = DegToRad (90.0);
  orient.y = 0;
  orient.z = 0;
  //------------------------------------------------
  offset.x = 0;
  offset.y = 0;
  offset.z = 0;
  //----Default target -----------------------------
  Tgt.lon = 0;
  Tgt.lat = 0;
  Tgt.alt = 0;
  //----UP is toward north direction ---------------
  Up.x  = 0;
  Up.y  = 1;
  Up.z  = 0;
  //----FOV is 40°----------------------------------
  fov = 40.0f;
	tgf = tan(DegToRad(fov * 0.5));
}
//-------------------------------------------------------------------------------
//  Update camera position relative to 0rigin
//-------------------------------------------------------------------------------
void CCameraRunway::UpdateCamera (SPosition wpos, SVector tgtOrient,float Dt)
{	camPos			= wpos;
	camPos.alt	= offset.z;
}
//-------------------------------------------------------------------------------
//  Set camera to airport origin
//  Camera is placed 3.2 miles above ground
//	return the horizontal extend in miles
//-------------------------------------------------------------------------------
double CCameraRunway::SetOrigin(SPosition *org)
{ double magl = 3.2;				// 3.2 miles above ground
	//----------------------------------------------
	Tgt.lon = org->lon;
  Tgt.lat = org->lat;
  Tgt.alt = org->alt;
  //---Compute zoom range ------------------------
  rmin      = org->alt + 100;
  rmax      = org->alt + NmToFeet (10.0f);
  //---Set offset --------------------------------
  offset.x  = 0;
  offset.y  = 0;
  offset.z  = org->alt + FN_FEET_FROM_MILE(magl);
  return (2 * magl * tgf);
}
//-------------------------------------------------------------------------------
//  Set camera target
//-------------------------------------------------------------------------------
void CCameraRunway::GetLookatPoint(SVector &v)
{ v.x = 0.0;
  v.y = 0.0;
  v.z = -offset.z;
  return;
}
//-------------------------------------------------------------------------------
//  Activate the camera.  The camara position is an airport world coordinates
//  Return camera altitude in feet
//-------------------------------------------------------------------------------
void CCameraRunway::SetView()
{gluLookAt (0, 0,  offset.z,             // eye position
            0, 0, -offset.z,             // Target position 
            Up.x, Up.y, Up.z);

  return;
}
//-------------------------------------------------------------------------------
//  Move up or down by inc (feet)
//-------------------------------------------------------------------------------
double CCameraRunway::MoveUp(int df)
{ offset.z += 100 * df;
  if (offset.z < rmin)  offset.z = rmin;
  if (offset.z > rmax)  offset.z = rmax;
	double magl = FN_MILE_FROM_FEET(offset.z);
  return (2 * magl * tgf);
}
//-------------------------------------------------------------------------------
//  Dont process
//-------------------------------------------------------------------------------
void CCameraRunway::Zoom(int zf)
{ //float df  = float(zf) * 0.5;
  //fov += df;
  //if (fov < 20)      fov = 20;
  //else if (fov > 90) fov = 90;
}

//-------------------------------------------------------------------------------
//  Move by (dx,dy) in pixels
//  convert dx,dy in arcsecs
//  Move about 20 feet per pixel at 4nm up
//-------------------------------------------------------------------------------
void CCameraRunway::MoveBy(float dx,float dy)
{ double mx = (20 * dx) * (offset.z / FN_FEET_FROM_MILE(4));
  double my = (20 * dy) * (offset.z / FN_FEET_FROM_MILE(4));;
  Tgt.lon  -= FN_ARCS_FROM_FEET(mx);
  Tgt.lat  += FN_ARCS_FROM_FEET(my);
  GroundSpot spot(Tgt.lon,Tgt.lat);
  Tgt.alt   = globals->tcm->GetGroundAt(spot);
  return;
}
//==============================================================================
// Object camera:  This camera can draw an object in a window and
//  render to texture.  It can adapt the distance to view the object entierly
//  and can be positionned in front or Top of it
//==============================================================================
CCameraObject::CCameraObject (void)
: CCamera()
{ bCOL.R = 0.8f;
  bCOL.G = 0.8f;
  bCOL.B = 0.8f;
  bCOL.A = 1;
  // Increase rmax to allow very high zoom range
  rmax = NmToFeet (1.0f);
  rmin = 30;
  range  = rmin;
  // Initialize orientation to point arround
  theta = DegToRad (30.0f);
  phi   = DegToRad (15.0f);
  //------------------------------------------------
  offset.x = 0;
  offset.y = 0;
  offset.z = 0;
  //------------------------------------------------
  Tgt.lon = 0;
  Tgt.lat = 0;
  Tgt.alt = 0;
  //----FOV is 40°----------------------------------
  fov = 40.0f;
  //------------------------------------------------
  wd   = 0;
  ht   = 0;
  mv   = 0;       // Allow mouse move
  Refresh();
}
//----------------------------------------------------------------------------
//  Ignore target position and orientation
//----------------------------------------------------------------------------
void CCameraObject::UpdateCamera(SPosition tgt,SVector ori,float dT )
{ 
  return;
}
//----------------------------------------------------------------------------
//  Set Background color
//----------------------------------------------------------------------------
void CCameraObject::SetColor(RGBA_COLOR &rgba)
{ bCOL = rgba;
  return;
}
//----------------------------------------------------------------------------
//  Set Top distance to view object of dim ht
//  Return the pixel ratio (nber of pixel per foot)
//----------------------------------------------------------------------------
void CCameraObject::TopOffsetFor(double ht)
{ //---Set Up vector ---------------------
  Up.x  = 0;
  Up.y  = 1;
  Up.z  = 0;
  //---Compute z value -------------------
  offset.x  = 0;
  offset.y  = 0;
  //---Compute tg fov --------------------
  double tg = tan(DegToRad(fov * 0.5));
  double cz = ht / tg;
  offset.z  = (cz < rmin)?(rmin):(cz);
  return;
}
//----------------------------------------------------------------------------
//  Set front distance to view object of dim ht
//----------------------------------------------------------------------------
void CCameraObject::FrontOffsetFor(double ht)
{ //---Set Up vector ---------------------
  Up.x  = 0;
  Up.y  = 0;
  Up.z  = 1;
  //---Compute z value -------------------
  offset.x  = 0;
  offset.z  = 0;
  //---Compute tg fov --------------------
  double tg = tan(DegToRad(fov * 0.5));
  double cy = ht / tg;
  offset.y  = (cy < rmin)?(rmin):(cy);
  //---Change fov to accomodate the distance ----
  return;
}
//----------------------------------------------------------------------------
//  Set field of view to accomodate the height to visualize
//  Input: rd =>  Object radius
//         fv =>  Field of view
//         w,h    Viewport size in pixels
//----------------------------------------------------------------------------
void CCameraObject::SetParameters(double rd, double fv,int w, int h)
{ double tg = tan(DegToRad(fv * 0.5));
  fov     = fv;
  wd      = w;
  ht      = h;
  //---Set Up vector -----------------------------
  Up.x  = 0;
  Up.y  = 0;
  Up.z  = 1;
  //---Compute camera distance to object ---------
  offset.x  = 0;
  offset.z  = 0;
  offset.y  = 2 * globals->nearP;
  //--- Compute near plan half height in feet ----
  double hp = globals->nearP * tg;
  scale     =  (2 * hp) / (rd);
  return;
}
//----------------------------------------------------------------------------
//  Swap front and top view
//----------------------------------------------------------------------------
void CCameraObject::SwapView()
{ //---Change UP vector ------------------
  double cz = Up.z;
  Up.z  = Up.y;
  Up.y  = cz;
  //---Change offset vector --------------
  cz        = offset.z;
  offset.z  = offset.y;
  offset.y  = cz;
  return;
}
//----------------------------------------------------------------------------
//  Move arround origin
//  Ignore target position and orientation
//  Compute camera offset arround position (0,0,0);
//----------------------------------------------------------------------------
void CCameraObject::Refresh()
{ // Update offset to target position in world coordinates.
  double sp =  sin(phi);
  double cp =  cos(phi);
  double st =  sin(theta);
  double ct =  cos(theta);
  offset.x  =  range * st * cp;
  offset.y  = -range * ct * cp;
  offset.z  =  range * sp;
  //---Update UP vector --------------------------
  Up.z      =  cp;
  scale     = 1;
  return;
}
//-------------------------------------------------------------------------
//  Destroy resources
//-------------------------------------------------------------------------
CCameraObject::~CCameraObject()
{ glDeleteRenderbuffersEXT(1, &DEP);
  glDeleteFramebuffersEXT (1, &FBO);
  if (IMG) glDeleteTextures(1, &IMG);
}
//-------------------------------------------------------------------------
//  Set a FBO buffer for rendering
//-------------------------------------------------------------------------
void CCameraObject::SetFBO(int w,int h)
{ wd  = w;
  ht  = h;
  //-----ALLOCATE THE FBO OBJECT ---------------------------------------------
  FBO = 0;
  IMG = 0;
  DEP = 0;
  glGenFramebuffersEXT(1,&FBO);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);
  //-----ALLOCATE A RENDER BUFFER for DEPTH --------------------------
  glGenRenderbuffersEXT(1,&DEP);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,DEP);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT,wd,ht);
  glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT,
                                GL_DEPTH_ATTACHMENT_EXT,
                                GL_RENDERBUFFER_EXT,
                                DEP);
  //-----ALLOCATE A TEXTURE OBJECT and LINK to FBO---------------------

  glGenTextures(1, &IMG);
  glBindTexture(GL_TEXTURE_2D, IMG);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,  wd, ht, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                            GL_COLOR_ATTACHMENT0_EXT,
                            GL_TEXTURE_2D,
                            IMG, 0);
  //---Check for completness -----------------------------------------
  GLenum st = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  if (st != GL_FRAMEBUFFER_COMPLETE_EXT)  gtfo("FBO problem");
  //---Reset normal rendering to frame buffer --------------------------------
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);

  return;
}
//-------------------------------------------------------------------------
// Return look at Point
//-------------------------------------------------------------------------
void CCameraObject::GetLookatPoint(SVector &v)
{ v.x = 0.0;
  v.y = 0.0;
  v.z = 0.0;
  return;
}
//-------------------------------------------------------------------------
// Pan Left - rotate clockwise
//-------------------------------------------------------------------------
void CCameraObject::PanLeft ()
{ theta = WrapTwoPi (theta - DegToRad (double(0.5)));
  Refresh();
  return; }
//-------------------------------------------------------------------------
// Pan Right - rotate counterclockwise
//-------------------------------------------------------------------------
void CCameraObject::PanRight ()
{ theta = WrapTwoPi (theta + DegToRad (double(0.5)));
  Refresh();
  return; }
//-------------------------------------------------------------------------
// Pan Up - rotate towards top of model,
//-------------------------------------------------------------------------
void CCameraObject::PanUp ()
{ phi += DegToRad (double(0.25));
  Refresh();
  return; }
//-------------------------------------------------------------------------
// Pan Down - rotate towards bottom of model,
//-------------------------------------------------------------------------
void CCameraObject::PanDown ()
{ phi -= DegToRad (double(0.25));
  Refresh();
  return; }
//-------------------------------------------------------------------------
// Pixel move
//  TODO: Rotate the UP vector too
//-------------------------------------------------------------------------
void CCameraObject::MoveBy(int px, int py)
{ if (0 == mv)  return;
  double r1 = double(0.01) * px;
  double r2 = double(0.01) * py;
  theta = WrapTwoPi (theta - r1);
  phi   = WrapTwoPi (phi   + r2);
  Refresh();
  return;
}
//-------------------------------------------------------------------------
// Zoom by some amount
//-------------------------------------------------------------------------
double CCameraObject::ZoomBy(double rat)
{ double dta = (rmax - rmin) * rat;
  range = rmin + dta;
  Refresh();
  return range;
}
//-------------------------------------------------------------------------
// Prepare OpenGL to draw with this camera on the indicated window
//    The camera is set to look at Tgt position i.e origin(0,0,0))
//    The object to draw should be at origin(0,0,0)
// Camera is set at such offset so to cover the object extension.
//    VIEW_PORT specify a viewport dimension on which the camera draw the object
//    The view port may be a Fui component for instance a Canva
//    The window is called with DrawByCamera(cam) and should supply the drawing
//    Then a quad is drawed with the texture coming from FBO
//
//  NOTE: This routine was hard to implement so modify it with care
//-------------------------------------------------------------------------
void CCameraObject::DrawOnWin(VIEW_PORT &vp,CFuiWindow *win)
{ CVector   tgp(0,0,0); 
  //----Initialize the model view matrix -------------------------------
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity ();
  // Initialize the modelview matrix to be the camera transform.  The camera is positioned
  //   at some offset from the world origin.  The world origin is coincident with the
  //   target vehicle position.
  gluLookAt (offset.x, offset.y, offset.z,
             tgp.x, tgp.y, tgp.z,
             Up.x,   Up.y,  Up.z);
  //======================================================================
  double aspect = double(vp.wd) / vp.ht;
  glMatrixMode (GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity ();
	SetCameraParameters(fov,aspect);
  gluPerspective (fov, ratio, nearP,farP);
  //---------Save and set view port  -------------------------------------
  glGetIntegerv(GL_VIEWPORT,(GLint*)&vps);
  glMatrixMode (GL_MODELVIEW);
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  //----------------------------------------------------------------------
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);    // Render to FBO
  glDepthMask(GL_TRUE);                
  glEnable(GL_DEPTH_TEST);
  glViewport(0,0,wd,ht);
  glClearColor(bCOL.R, bCOL.G, bCOL.B, bCOL.A);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );  
  glColor4f(1,1,1,1);
  //---Call draw function ------------------------------------------------
  win->DrawByCamera(this); 
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);      // Normal rendering
  //----------------------------------------------------------------------
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D (0,wd,0, ht);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  //---Init QUAD coordinates ---------------------------------
  Pan[1].VT_X = wd;                       // SE corner
  Pan[2].VT_X = wd;                       // NE corner
  Pan[2].VT_Y = ht;
  Pan[3].VT_Y = ht;                       // NW corner
  glBindTexture(GL_TEXTURE_2D,IMG);
  glEnable(GL_TEXTURE_2D);
  glDisable (GL_DEPTH_TEST);
  glViewport(vp.x0,vp.y0,vp.wd,vp.ht);
  //----Draw the Quad at screen position ---------------------
  glColor4f(1,1,1,1);
  glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
  glPolygonMode(GL_FRONT,GL_FILL);
  glInterleavedArrays(GL_T2F_V3F,0,Pan);
  glDrawArrays(GL_QUADS,0,4);
  glPopClientAttrib();
  
  //-----Exit ------------------------------------------------
  glPopAttrib();
  glViewport(vps.x0,vps.y0,vps.wd,vps.ht);
  glMatrixMode (GL_PROJECTION);
  glPopMatrix();
  glMatrixMode (GL_MODELVIEW);
  glPopMatrix();
  //---------- Check for an OpenGL error ---------------------
	/*
  { GLenum e = glGetError ();
  if (e != GL_NO_ERROR) 
    WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  }
	*/
  return;
}
//-------------------------------------------------------------------------
//  Transfert texture responsibility
//-------------------------------------------------------------------------
U_INT CCameraObject::PassTextureObject()
{ U_INT obj = IMG;
  IMG = 0;
  return obj;
}
//-------------------------------------------------------------------------
// Prepare OpenGL to draw with this camera on an internal FBO
//    The camera is set to look at  origin(0,0,0))
//    The object to draw should be at origin(0,0,0)
//    The camera use perspective drawing.
// Camera is set at some offset so to cover the object extension.
//    VIEW_PORT specify a viewport dimension on which the camera draw the object
//    The view port may be a Fui component for instance a Canva
//    The CCanva is called with DrawByCamera(cam) and should supply the drawing
//    Then a quad is drawed with the texture coming from FBO
//
//  NOTE: This routine was hard to implement so modify it with care
//-------------------------------------------------------------------------
void CCameraObject::DebDrawFBOinPerspective(int w, int h)
{ wd  = w;
  ht  = h;
  //----Initialize the model view matrix -------------------------------
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity ();
  //----Scale object if needed -----------------------------------------
  // Initialize the modelview matrix to be the camera transform.  The camera is positioned
  //   at some offset from the world origin.  The camera looks at world origin (0,0,0)
  gluLookAt (offset.x, offset.y, offset.z,
             0, 0, 0,
             Up.x,   Up.y,  Up.z);
  //======================================================================
  double aspect = double(wd) / ht;
  glMatrixMode (GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity ();
	SetCameraParameters(fov,aspect);
  gluPerspective (fov, ratio, nearP,farP);
  //---------Save and set view port  -------------------------------------
  glGetIntegerv(GL_VIEWPORT,(GLint*)&vps);
  glMatrixMode (GL_MODELVIEW);
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  if (scale != 1) glScaled(scale,scale,scale);
  //----------------------------------------------------------------------
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);    // Render to FBO
  glDepthMask(GL_TRUE);                
  glEnable(GL_DEPTH_TEST);
  glViewport(0,0,wd,ht);
  glClearColor(bCOL.R, bCOL.G, bCOL.B, bCOL.A);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );  
  return;
}
//-------------------------------------------------------------------------
// Prepare OpenGL to draw with this camera on an internal FBO
//    The camera is set to look at  origin(0,0,0))
//    The object to draw should be at origin(0,0,0)
//    The camera use orthoprojection drawing.
// Camera is set at Y offset to look at X,Z plane (camera up is along Z plane)
//    VIEW_PORT specify the texture dimension on which the camera draw the object
//
//  NOTE: This routine was hard to implement so modify it with care
//-------------------------------------------------------------------------
void CCameraObject::DebDrawFBOinOrthoMode(int w, int h)
{ wd  = w;
  ht  = h;
  double dw  = double (w >> 1);                     // Half width
  double dh  = double (h >> 1);                     // Half height
  //----Initialize the model view matrix -------------------------------
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity ();
  //---------------------------------------------------------------------
  // Initialize the modelview matrix to be the camera transform.  
  //  The camera looks at world origin (0,0,0)
  //  Up vector is along Z (0,0,1)
  gluLookAt (0,  10,  0,                        // position above plane XZ
             0,   0,  0,                        // Target origin 
             0,   0,  1);                       // Up vector
  //======================================================================
  glMatrixMode (GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity ();
  glOrtho (-dw,+dw,-dh,+dh,-dw,+dw);
  //---------Save and set view port  -------------------------------------
  glGetIntegerv(GL_VIEWPORT,(GLint*)&vps);
  glMatrixMode (GL_MODELVIEW);
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  //----------------------------------------------------------------------
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);    // Render to FBO
  glDepthMask(GL_TRUE);                
  glEnable(GL_DEPTH_TEST);
  glViewport(0,0,wd,ht);
  glClearColor(bCOL.R, bCOL.G, bCOL.B, bCOL.A);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );  
  return;
}
//-------------------------------------------------------------------------
//  End drawing to FBO.  The original viewport is restaured (if one was saved)
//  NOT used actually
//-------------------------------------------------------------------------
void CCameraObject::EndDrawFBO()
{ glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);      // Normal rendering
  //-----Exit ------------------------------------------------
  glPopAttrib();
  glViewport(vps.x0,vps.y0,vps.wd,vps.ht);
  glMatrixMode (GL_PROJECTION);
  glPopMatrix();
  glMatrixMode (GL_MODELVIEW);
  glPopMatrix();
  //---------- Check for an OpenGL error ---------------------
	/*
  { GLenum e = glGetError ();
  if (e != GL_NO_ERROR) 
    WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  }
	*/
  return;
}
//==============================================================================
// Helper CCanva
//==============================================================================
CCanva::CCanva(int w,int h)
{ RGBA_COLOR col = {1,1,1,1};
  xOBJ  = 0;
  wd    = w;
  ht    = h;
  bak   = 0;                // Default to black transparent
  cam   = new CCameraObject();
  cam->SetFBO(w,h);
  cam->MoveMouseIs(0);
  cam->SetColor(col);         // Black transparent
}
//-------------------------------------------------------------
//  Destructor
//-------------------------------------------------------------
CCanva::~CCanva()
{ if (xOBJ)  glDeleteTextures(1, &xOBJ);
  if (cam)   delete cam;
}
//-------------------------------------------------------------
//  Set object radius
//-------------------------------------------------------------
void CCanva::FrontView(double rad)
{ if (cam) cam->FrontOffsetFor(rad);
  return;
}
//-------------------------------------------------------------
//  Set camera position
//-------------------------------------------------------------
void CCanva::SetParameters(double rd, double fv)
{ if (cam) cam->SetParameters(rd, fv,wd, ht);
  return;
}
//-------------------------------------------------------------
//  End of Drawing.  Preserve texture Object
//  NOTE: Texture object responsibility is now to the canva
//        object.
//-------------------------------------------------------------
void CCanva::EndDraw()
{ if (0 == cam) return;
  if (0 == xOBJ)  xOBJ  = cam->PassTextureObject();
  cam->EndDrawFBO();
  return;
}
//-------------------------------------------------------------
//  Transfer texture object responsibility
//-------------------------------------------------------------
void  CCanva::PopTexture()
{ if (0 == cam) return;
  if (0 == xOBJ)  xOBJ  = cam->PassTextureObject();
  return;
}
//-------------------------------------------------------------
//  Transfer texture object responsibility
//-------------------------------------------------------------
U_INT CCanva::GetTextureObject()
{ U_INT obj = xOBJ;
  xOBJ  = 0;
  return obj;
}
//----------------------------------------------------------------------------
//  Write the texture:  This is used for test only
//----------------------------------------------------------------------------
void CCanva::WriteTexture()
{ int     nbp = wd * ht;
  int     dim = nbp* 4;
  U_CHAR *buf = new U_CHAR[dim];
  glBindTexture(GL_TEXTURE_2D,xOBJ);
  glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,buf);
  //----------------------------------------------------------------
  {GLenum e = glGetError ();
   if (e != GL_NO_ERROR) 
    WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  }
  U_INT     nzr = 0;
  U_INT  *pix = (U_INT*)buf;
  //---Compute a non zero image indicator --------------------
  for (int k=0; k<nbp; k++)     nzr |= (*pix++ != 0x00FFFFFF);
  //----------------------------------------------------------
  char fn[1024];
  _snprintf(fn,1023,"TMP\\PROP.BMP");
  CArtParser img(0);
  if (nzr) img.WriteBitmap(FIF_BMP,fn,wd,ht,buf);
  delete [] buf;
  return;
}

//================================================================================
// CCameraDLL
//
// Additional cameras with DLL plugins
//================================================================================
CCameraDLL::CCameraDLL (Tag &camera_type_, const char *camera_name_, int &is_interior_camera_)
: CCamera ()
{
  #ifdef _DEBUG
    //TRACE ("--------------------------------------------------------");
    //TRACE ("DLL CAMERA ...");
  #endif
  obj = NULL;
  dll = NULL;
  enabled = false;
  //
  camera_type = camera_type_;
  strncpy (camera_name, camera_name_, FILENAME_MAX);
  is_interior_camera = is_interior_camera_;
  //
  signature = camera_type;
  Prepare ();
  //
  #ifdef _DEBUG
    //TRACE ("CREATE DLL CAMERA");
  #endif 
  eyePos_.lat = eyePos_.lon = eyePos_.alt = 0.0;
  eyeOri_.x = eyeOri_.y = eyeOri_.z = 0.0;
  eyeOri_.p = eyeOri_.h = eyeOri_.r = 0.0;
}

CCameraDLL::~CCameraDLL (void)
{
#ifdef _DEBUG
  //TRACE ("DELETE DLL CAMERA %p %p", obj, dll);
  //TRACE ("--------------------------------------------------------");
#endif
  // sdk: cleanup objects = DLLDestroyObject // 
  globals->plugins.On_DestroyObject (obj, dll); 
}

void CCameraDLL::Prepare (void)
{
  // sdk : test whether a dll is present
  if (globals->plugins_num) {
    void *test = globals->plugins.IsDLL (signature);
    if (NULL == test) {
      globals->plugins.On_DeleteAllObjects ();
      globals->plugins.On_KillPlugins ();
      char buf1 [8] = {0};
      TagToString (buf1, signature); 
      //TRACE ("test dll '%s' = %d", buf1, test);
      gtfo ("failed to find a DLL for '%s'", buf1);
    }
    else {
    dll = test;
      globals->plugins.On_Instantiate_DLLCamera (signature,0,NULL);
      obj = globals->plugins.GetDLLObject (signature);
    }
  }
}


void CCameraDLL::GetLookatPoint (SVector &v)
{
  v.x = v.y = v.z = 0.0;
  glRotated (RadToDeg (eyeOri_.y), 0.0, 0.0, 1.0); // roll
  glRotated (RadToDeg (eyeOri_.z), 0.0, 1.0, 0.0); // head
  glRotated (RadToDeg (eyeOri_.x), 1.0, 0.0, 0.0); // pitch
}

void CCameraDLL::UpdateCamera (SPosition wPos, SVector tgtOri, float dT)
{ tgtPos = wPos;
  // only eyePos and eyeOri should be modified
  globals->plugins.On_UpdateCamera (obj, &tgtPos, &tgtOri, &eyePos_, &eyeOri_, dT, dll);
  //
  offset = SubtractPositionInFeet (tgtPos, eyePos_);
}

Tag CCameraDLL::GetCameraType (void)
{
  return camera_type;
}

void CCameraDLL::GetCameraName (char* name, int maxLength)
{
  strncpy (name, camera_name, maxLength);
}


void CCameraDLL::SetObject (SDLLObject *object)
{
  obj = object;
}

void CCameraDLL::SetSignature (const long &sign)
{
  signature = sign;
}

//===================================================================================
//  Bind camera keys
//===================================================================================
void CCameraManager::BindKeys()
{ CKeyMap *km = globals->kbd;
  km->BindGroup('cmra',KeyCamGroup);
  //---------------------------------------
  km->Bind ('cock', cKeyCOCK,KEY_SET_ON);
  km->Bind ('spot', cKeySPOT,KEY_SET_ON);
  km->Bind ('obsr', cKeyOBSR,KEY_SET_ON);
  km->Bind ('over', cKeyOVER,KEY_SET_ON);
  km->Bind ('flyb', cKeyFLYB,KEY_SET_ON);
  km->Bind ('towr', cKeyTOWR,KEY_SET_ON);
  //---------------------------------------
  km->Bind('czri',cKeyCZRI,KEY_SET_ON);
  km->Bind('czro',cKeyCZRO,KEY_SET_ON);
  km->Bind('czin',cKeyCZIN,KEY_SET_ON);
  km->Bind('czis',cKeyCZIS,KEY_SET_ON);
  km->Bind('czif',cKeyCZIF,KEY_SET_ON);
  km->Bind('czot',cKeyCZOT,KEY_SET_ON);
  km->Bind('czos',cKeyCZOS,KEY_SET_ON);
  km->Bind('czof',cKeyCZOF,KEY_SET_ON);
  km->Bind('czrs',cKeyCZRS,KEY_SET_ON);
  km->Bind('chpu',cKeyCHPU,KEY_REPEAT);
  km->Bind('chpd',cKeyCHPD,KEY_REPEAT);
  km->Bind('cplf',cKeyCPLF,KEY_SET_ON);
  km->Bind('cprt',cKeyCPRT,KEY_SET_ON);
  km->Bind('cpup',cKeyCPUP,KEY_SET_ON);
  km->Bind('cpdn',cKeyCPDN,KEY_SET_ON);
  //---------------------------------------
  km->Bind('cs01',cKeyCS01,KEY_SET_ON);
  km->Bind('cs02',cKeyCS02,KEY_SET_ON);
  km->Bind('cs03',cKeyCS03,KEY_SET_ON);
  km->Bind('cs04',cKeyCS04,KEY_SET_ON);
  //---------------------------------------
  km->Bind('cd01',cKeyCD01,KEY_SET_ON);
  km->Bind('cd02',cKeyCD02,KEY_SET_ON);
  km->Bind('cd03',cKeyCD03,KEY_SET_ON);
  km->Bind('cd04',cKeyCD04,KEY_SET_ON);
  km->Bind('cmtg',cKeyCMTG,KEY_SET_ON);
  km->Bind('czap',cKeyCZAP,KEY_SET_ON);
  km->Bind('cd07',cKeyCD07,KEY_SET_ON);
  km->Bind('cd08',cKeyCD08,KEY_SET_ON);
  //---------------------------------------
  km->Bind ('ckup', cKeyCKUP,KEY_SET_ON);
  km->Bind ('ckdn', cKeyCKDN,KEY_SET_ON);
  km->Bind ('cklf', cKeyCKLF,KEY_SET_ON);
  km->Bind ('ckri', cKeyCKRI,KEY_SET_ON);
  km->Bind ('ckhm', cKeyCKHM,KEY_SET_ON);
  km->Bind ('ckpu', cKeyCKPU,KEY_SET_ON);
  km->Bind ('ckpd', cKeyCKPD,KEY_SET_ON);
  return;
}
//===================================================================================
// Camera Manager
//===================================================================================
CCameraManager::CCameraManager (CVehicleObject *veh,char* fn)
{ // Default camera is external spot, unless overridden in a Read() call
	mveh		= veh;
  tCam    = CAMERA_SPOT;
  aCam    = 0;
  globals->ccm = this;
  SStream s;
  if (OpenRStream ("WORLD",fn,s)) {
    ReadFrom (this, &s);
    CloseStream (&s);
  } else {
    WARNINGLOG ("CCameraManager : Cannot open .CAM file %s", fn);
  }
}
//-----------------------------------------------------------------
//  Free camera list
//-----------------------------------------------------------------
CCameraManager::~CCameraManager (void)
{ std::map<Tag,CCamera*>::iterator i;
  for (i=came.begin(); i!=came.end(); i++) delete (*i).second;
	came.clear();
  globals->ccm = 0;
}

//-----------------------------------------------------------------
//  Read parameters
//-----------------------------------------------------------------
int CCameraManager::Read (SStream *stream, Tag tag)
{ switch (tag) {
  case 'came':
    // Camera definition
    {
      Tag type;
      ReadTag (&type, stream);
      switch (type) {
      case CAMERA_COCKPIT:
        // Cockpit camera
        {
          CCameraCockpit *cock = new CCameraCockpit(mveh);
          ReadFrom (cock, stream);
          came[CAMERA_COCKPIT] = cock;
          // If a cockpit camera is defined, make it the default
          tCam = CAMERA_COCKPIT;
					return TAG_READ;
        }

      default:
        WARNINGLOG ("CCameraManager::Read : Unknown camera type %s", TagToString(tag));
				return TAG_IGNORED;
			}
		}
	}

  // Tag was not processed by this object, it is unrecognized
  WARNINGLOG ("CCameraManager::Read : Unrecognized tag <%s>", TagToString(tag));
  return TAG_IGNORED;
}
//-------------------------------------------------------------------------
//  Assign default cameras
//-------------------------------------------------------------------------
void CCameraManager::DefaultCameras()
{ CCameraSpot *spot = new CCameraSpot;
  came[CAMERA_SPOT] = spot;

  CCameraObserver *cobs = new CCameraObserver;
  came[CAMERA_OBSERVER] = cobs;

  CCameraTower *ctwr  = new CCameraTower;
  came[CAMERA_TOWER]  = ctwr;

  CCameraOverhead *over = new CCameraOverhead;
  came[CAMERA_OVERHEAD] = over;

  CCameraOrbit *orbit = new CCameraOrbit;
  came[CAMERA_ORBIT]  = orbit;

  CCameraFlyby *cfly = new CCameraFlyby;
  came[CAMERA_FLYBY]  = cfly;

  return;
}
//-------------------------------------------------------------------------
//  Link  Explicit cameras
//-------------------------------------------------------------------------
void CCameraManager::Link(CCamera *cam,int k,int last)
{ int idn = (k == last)?(   0):(k+1);       // Next index
  int idp = (k ==    0)?(last):(k-1);       // Previous index
  char tnx[8];
  char tpv[8];
  Tag nxt = StringToTag (cam_list.GetTag (idn));
  Tag prv = StringToTag (cam_list.GetTag (idp));
  TagToString(tnx,nxt);
  TagToString(tpv,prv);
  cam->SetNext(nxt);
  cam->SetPrev(prv);
  return;
}
//-------------------------------------------------------------------------
//  Assign Explicit cameras
//-------------------------------------------------------------------------
void CCameraManager::ExplicitCameras(int nb)
{ int last = nb - 1;
  for (int i = 0; i < nb; ++i) {
    Tag cam = StringToTag (cam_list.GetTag (i));
    if (cam_list.GetVal (i)) {
      //TRACE ("camera %s", cam_list.GetTag (i));
      switch (cam) {
        case CAMERA_ORBIT:    {
          CCamera *cam_ = new CCameraOrbit;
          came[CAMERA_ORBIT]  = cam_;
          Link(cam_,i,last);
          break;  }

        case CAMERA_TOWER:    {
          CCamera *cam_  = new CCameraTower;
          came[CAMERA_TOWER]  = cam_;
          Link(cam_,i,last);
          break;  }

        case CAMERA_OVERHEAD: {
          CCamera *cam_ = new CCameraOverhead;
          came[CAMERA_OVERHEAD] = cam_;
          Link(cam_,i,last);
          break;  }

        case CAMERA_FLYBY : {
          CCamera *cam_  = new CCameraFlyby;
          came[CAMERA_FLYBY]  = cam_;
          Link(cam_,i,last);
          break;  }

        case CAMERA_OBSERVER : {
          CCamera *cam_ = new CCameraObserver;
          came[CAMERA_OBSERVER] = cam_;
          Link(cam_,i,last);
          break;  }

        case CAMERA_SPOT : {
          CCamera *cam_ = new CCameraSpot;
          came[CAMERA_SPOT] = cam_;
          Link(cam_,i,last);
          break;  }

        default : {
            // LC NOTE : in CVehicleObject::ReadFinished
            // there are two cam : cam[0] and cam[1]
            // so we've to stop double reading of cam_list.ReadCamerasFile ()
            //TRACE ("CCameraManager::ReadFinished");
            // JS:  Pourquoi creer des cameras partout ailleurs et pas dans ce module???
            //  C'est confus. A eclaircir
            int interior = 0;
            CCamera *cam_ = new CCameraDLL (cam, cam_list.GetCameraName (i), interior);
            came[cam] = cam_;
            Link(cam_,i,last);
          break;}
      }
    }
  }
  //---Add the cockpit camera if present -------------------
  if (CAMERA_COCKPIT != tCam)   return;
  CCamera *ck = GetCamera(CAMERA_COCKPIT);
  Tag      t1 = StringToTag (cam_list.GetTag (0));
  CCamera *c1 = GetCamera(t1);
  Tag      td = StringToTag (cam_list.GetTag (last));
  CCamera *cd = GetCamera(td);
  cd->SetNext(CAMERA_COCKPIT);
  ck->SetPrev(td);
  c1->SetPrev(CAMERA_COCKPIT);
  ck->SetNext(t1);
  return;
}
//-------------------------------------------------------------------------
//  All parameters are read
//-------------------------------------------------------------------------
void CCameraManager::ReadFinished (void)
{ BindKeys();
  // Add other default cameras
  int nb = cam_list.ReadCamerasFile (); /// reads DATA\CAMERAS.TXT
  if (nb)      ExplicitCameras(nb);
  else         DefaultCameras(); 
  //---  Set default camera ----------
  SelectCamera (tCam);
	if (0 == aCam)	gtfo("Can't find camera %s",TagToString(tCam));
  return;
}
//-----------------------------------------------------------------
//  Adjust all range for adjustable camera
//-----------------------------------------------------------------
void CCameraManager::AdjustRange(double lg)
{ std::map<Tag,CCamera*>::iterator i;
  for (i=came.begin(); i!=came.end(); i++)
  { CCamera *cam = (*i).second;
    cam->RangeAdjust(lg);
  }
  return;
}
//-------------------------------------------------------------------------
//  Stop any camera move
//-------------------------------------------------------------------------
void CCameraManager::ZeroRate()
{ aCam->Stop();
  return;
}
//---------------------------------------------------------------
//  Update position for Active  camera
//---------------------------------------------------------------
void CCameraManager::UpdateCamera (SPosition tgtPos, SVector tgtOrient,float dT)
{ // Get updated eye position from the active camera
  aCam->UpdateCamera (tgtPos, tgtOrient);
  // Store target position for Print() method
  this->tgtPos    = tgtPos;
  this->tgtOrient = tgtOrient;
  return;
}
//---------------------------------------------------------------
//  Change camera
//---------------------------------------------------------------
CCamera *CCameraManager::SelectCamera (Tag id)
{ //-- Search camera list for the given tag --------
	if (0 == id)					return aCam;
  std::map<Tag,CCamera*>::iterator ic = came.find(id);
  if (ic == came.end()) return aCam;
  // Notify user of new camera name
  char name[64];
	//--- relaxe drawing indicators ---------
	if (aCam)	globals->noEXT -= aCam->GetINTMOD();
	if (aCam) globals->noINT -= aCam->GetEXTMOD();
	//--- Change to new camera -------------
  aCam = (*ic).second;
  aCam->GetCameraName (name, 64);
  Internal = aCam->GetINTMOD();
	globals->inside = Internal;
	//--- Set drawing constraints ----------
	globals->noEXT += aCam->GetINTMOD();
	globals->noINT += aCam->GetEXTMOD();
  DrawNoticeToUser (name, 8);
  //---Change resolution ------------------
  aCam->ChangeResolution();
	globals->cam = aCam;
  return aCam;
}

//---------------------------------------------------------------
//  Save current camera context
//  Allocate a new spot camera during editing
//---------------------------------------------------------------
CRabbitCamera *CCameraManager::SetRabbitCamera(CAMERA_CTX &ctx, U_CHAR opt)
{ aCam->Save(ctx);			// Save actual context
	//--- Relax drawing from current camera -
	globals->noEXT -= aCam->GetINTMOD();
	globals->noINT -= aCam->GetEXTMOD();
	//---Change to rabit camera -------------
	CRabbitCamera *rcam = new CRabbitCamera();
  aCam					= rcam;
	globals->cam	= rcam;
	rcam->Store(&ctx);
	//--- Set drawing constraints -----------
	globals->noEXT += rcam->GetINTMOD();
	globals->noINT += rcam->GetEXTMOD();
	//--- Lock in slew mode -----------------
	if (opt & RABBIT_IN_SLEW) globals->slw->StartMode(&ctx);							// Slew mode
	//--- Set profile -----------------------
	ctx.prof |= PROF_ACBUSY;
	SpecialProfile('busy',ctx.prof);
	//--- Create camera window --------------
	if (opt & RABBIT_CONTROL) globals->fui->CreateOneWindow('ccam',0);
  return rcam;
}
//---------------------------------------------------------------
//  Restore camera from context
//---------------------------------------------------------------
void CCameraManager::RestoreCamera(CAMERA_CTX &ctx)
{ //--- Relax drawing from active camera ------
	delete aCam;
	aCam	= 0;
	//--- Set new camera and relax constraints --
  SelectCamera(ctx.iden);
  aCam->Restore(ctx);
	//--- Restore profile -----------------------
	SpecialProfile(0,ctx.prof);					// Clear profile
	//--- Now stop slew mode --------------------
	if (ctx.mode == SLEW_RCAM) globals->slw->StopSlew();
  return;
}
//---------------------------------------------------------------
//  Return camera by idn
//---------------------------------------------------------------
CCamera *CCameraManager::GetCamera(Tag id)
{ std::map<Tag,CCamera *>::iterator cm = came.find(id);
  return (cm != came.end())?((*cm).second):(0);}
//---------------------------------------------------------------
//  Return cockpit camera
//---------------------------------------------------------------
CCameraCockpit *CCameraManager::GetCockpitCamera()
{ std::map<Tag,CCamera *>::iterator cm = came.find('cock');
  return (cm != came.end())?((CCameraCockpit*)(*cm).second):(0);
}
//---------------------------------------------------------------
//  Change to previous camera
//---------------------------------------------------------------
void CCameraManager::PrevCamera (void)
{ SelectCamera(aCam->PrevCamera());
  return;
}
//---------------------------------------------------------------
//  Change to next camera
//---------------------------------------------------------------
void CCameraManager::NextCamera (void)
{ SelectCamera(aCam->NextCamera());
  return;
}

//---------------------------------------------------------------
//  Keyboard command
//---------------------------------------------------------------
void CCameraManager::KbEvent(Tag id)
{ if (aCam->IsLocked()) return;
  SelectCamera (id);
  return;
}
//-------------------------------------------------------------------------
//  Key for Cockpit manager
//-------------------------------------------------------------------------
bool CCameraManager::KeyCameraCockpitEvent(int id)
{ CCockpitManager *pit = globals->pit;
  if (pit)  pit->KbEvent(id);
  return true;
}
//---------------------------------------------------------------
//  Print parameter from camera
//---------------------------------------------------------------
void CCameraManager::Print (FILE *f)
{
  char debug[256];

  fprintf (f, "Camera Manager:\n\n");

  // Current camera attributes
  CCamera* camera = GetActiveCamera();
  Tag tag = camera->GetCameraType ();
  char sTag[8];
  TagToString (sTag, tag);
  char name[80];
  camera->GetCameraName (name, 80);
  SVector offset;
  camera->GetOffset (offset);

  fprintf (f, "Current Camera : '%4s' %s\n", sTag, name);
  fprintf (f, "  Offset : x=%6.3f  y=%6.3f  z=%6.3f\n",
    offset.x, offset.y, offset.z);
  fprintf (f, "\n");

  camera->Print (f);

  // Target position
  fprintf (f, "Target:\n");
  FormatPosition (tgtPos, debug);
  fprintf (f, "  Pos  %-40s\n", debug);
  fprintf (f, "\n");

  // Eye orientation
  fprintf (f, "Orientation :\n");
  fprintf (f, "       Heading       Pitch       Roll\n");
  fprintf (f, "  Rad   %6.3f       %6.3f       %6.3f\n",
    tgtOrient.h, tgtOrient.p, tgtOrient.r);
  fprintf (f, "  Deg   %6.3f       %6.3f       %6.3f\n",
    RadToDeg (tgtOrient.h), RadToDeg (tgtOrient.p), RadToDeg (tgtOrient.r));
  fprintf (f, "\n");
}
//=====================END OF FILE ===========================================================
