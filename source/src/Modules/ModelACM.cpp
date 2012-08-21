/*
 * ModelACM.cpp
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
 */

/*! \file ModelACM.cpp
 *  \brief Implements CModelACM class for loading ACM format 3D models
 */

#include "../Include/Globals.h"
#include "../Include/FlyLegacy.h"
#include "../Include/Utility.h"
#include "../Include/Ui.h"
#include "../Include/FileParser.h"
#include "../Include/TerrainTexture.h"
#include "../Include/Cameras.h"
#include "../Include/AnimatedModel.h"
#include "../Include/collada.h"
#include "../Include/Weather.h"
#include "../Plugin/Plugin.h"
#include <algorithm> // used in SortChildTransparency ()
#include <float.h>
using namespace std;

///////// DEBUG STUFF TO REMOVE LATER ////////////////////////////////
#ifdef  _DEBUG
  //#define _DEBUG_PARTNAME   //print lc DDEBUG file ... remove later
  //#define _DEBUG_KEYFRAME   //print lc DDEBUG file ... remove later
#endif
//////////////////////////////////////////////////////////////////////
//============================================================================================
//  Key frame artificial
//============================================================================================
CKeyFrame kfrm0();
CKeyFrame kfrm1();
//====================================================================
//  Global parameters for ACM draw
//====================================================================
GLfloat black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat diffu[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
//====================================================================
//  Cross vertices for Gizmo
//====================================================================
F3_VERTEX crosTB[6] = {
  {-1, 0, 0},
  {+1, 0, 0},
  //-----------------
  { 0,-1, 0},
  { 0,+1, 0},
  //-----------------
  { 0, 0,-1},
  { 0, 0,+1},
};
//====================================================================
//  Cross vertices for tank
//    Front (Y=-1)   Back(Y=+1)
//        D---C      H---G
//        |   |      |   |
//        A---B      E---F
//
//====================================================================
F3_VERTEX vtxTK[] = {
  //---Face n°1 --------
  { -1 ,-1 ,-0.5},      // A 0
  { +1 ,-1 ,-0.5},      // B 1
  { +1 ,-1 ,+0.5},      // C 2
  { -1 ,-1 ,+0.5},      // D 3
  //---Face n°2 --------
  { -1 ,+1 ,-0.5},      // E 4
  { +1 ,+1 ,-0.5},      // F 5
  { +1 ,+1 ,+0.5},      // G 6
  { -1 ,+1 ,+0.5},      // H 7
  //--------------------
};
//--------------------------------------------------------------------
//  Front face indice
//--------------------------------------------------------------------
U_CHAR indTK[] = { 
  3,2,6,7,             // Top face
  0,1,2,3,             // Front face
};
//====================================================================
//  Key Frame. Clear all
//====================================================================
CKeyFrame::CKeyFrame()
{ frame   = 0;
  indP = indB = indH = 0;
  dx = dy = dz = 0;
  p  = b  = h  = 0;
}
//--------------------------------------------------------------------
//  Trace Keyframe
//--------------------------------------------------------------------
void CKeyFrame::Trace()
{ TRACE("KEYFRAME Key=%0.5f dx=%.4f dy=%.4f dz=%.4f p=%.4f b=%.4f h=%.4f",frame,dx,dy,dz,p,b,h);
  return;
}
//====================================================================
//  sort Child Parts Toward Transparency (used with SortChildTransparency)
//====================================================================
bool SortChildPartsTowardTransparency(CAcmPart * p1,CAcmPart * p2)
{ if (p1->IsTransparent() && !p2->IsTransparent ()) return true;
  return false;
}
//==========================================================================
//  Class Animator 
//==========================================================================
CAnimator::CAnimator()
{ used = 0;
  Reset();                     // Status
}
//-------------------------------------------------------------------
//  Destructor
//-------------------------------------------------------------------
CAnimator::~CAnimator()
{ obj.clear();
}
//-------------------------------------------------------------------
//  Add a part
//-------------------------------------------------------------------
void CAnimator::AddPart(CAniPart *p)
{ used = 1;
  obj.push_back(p);
  return;
}
//-------------------------------------------------------------------
//  Reset to state
//-------------------------------------------------------------------
void CAnimator::Reset()
{ tr    = 0;                    // No trace
  aStat = ITEM_KFR0;            // Status
  eStop = 0;                    // No stop event
  aMove = 0;                    // Move direction
  aTime = 0;                    // Current time
  aTmax = 0;                    // Mximum animation time
  aTarg = 0;                    // Keyframe target
  std::vector<CAniPart*>::iterator w;
  for (w = obj.begin(); w != obj.end(); w++)
  {(*w)->ResetKey();  }
  return;
}
//-------------------------------------------------------------------
//  Signal start move to parts
//-------------------------------------------------------------------
void CAnimator::StartMove()
{ std::vector<CAniPart*>::iterator w;
  for (w = obj.begin(); w != obj.end(); w++)
  {(*w)->StartMove();  }
  return;
}
//-------------------------------------------------------------------
//  Set forward moving state (toward keyframe 1)
//  tg  = target keyframe value in [0-1]
//  e   = Animator ending state
//-------------------------------------------------------------------
void CAnimator::Forward(float tg, char e)
{ eStat = e;
  aStat = ITEM_MOVE;
  aTime = 0;
  aTarg = tg;
  aMove = +1;
  //TRACE("GEAR: request to move forward");
  return;
}
//-------------------------------------------------------------------
//  Set Backward moving state (toward keyframe 0)
//  tg  = target keyframe in [0-1]
//  e   = Animator ending state
//-------------------------------------------------------------------
void CAnimator::Backward(float tg,char e)
{ eStat = e;
  aStat = ITEM_MOVE;
  aTime = aTmax;
  aMove = -1;
  aTarg = tg;
  //TRACE("GEAR: request to move backward");
  return;
}
//-------------------------------------------------------------------
//  Reverse direction and target
//-------------------------------------------------------------------
void CAnimator::Reverse()
{ aTarg  = (aMove == +1)?(0):(1);
  eStat  = (aMove == +1)?(ITEM_KFR0):(ITEM_KFR1);
  aMove *= -1;
  return;
}
//-------------------------------------------------------------------
//  Reverse direction and target
//-------------------------------------------------------------------
void CAnimator::Reverse(float tg,char end)
{ aTarg  = tg;
  eStat  = end;
  aMove *= -1;
  return;
}
//-------------------------------------------------------------------
// Change only target and end state
//-------------------------------------------------------------------
void CAnimator::ChangeTo(float tkf,char end)
{ aTarg = tkf;
  eStat = end;
}
//---------------------------------------------------------------------
//  Move parts according  to the scenario timer 
//---------------------------------------------------------------------
void CAnimator::Animate(float dT)
{ U_INT end  = 0;
  char  dir  = aMove;                // Direction
  float dtim = float(dir) * dT;
  aTime += dtim;
  std::vector<CAniPart*>::iterator w;
  for (w = obj.begin(); w != obj.end(); w++)
  { 
    //---Move until all parts has reached target ----
    end += (*w)->MovePart(aTarg,dir,dT,aTime);
  }
  //---Update state when all objects are in place --
  if (end != obj.size()) return;
  //---Enforce ending state ------------------------
  aStat = eStat;          // Ending state
  aTime = 0;              // Clear timer
  eStop = 1;              // Stop event       
  return;
}
//---------------------------------------------------------------------
//  Check for stop event 
//---------------------------------------------------------------------
bool CAnimator::Stops()
{ char evn = eStop;
  eStop = 0;
  return (evn != 0);
}
//-------------------------------------------------------------------
//  Set the scenario overall time
//  Send to every parts the FPS (keyframe per second) allowed
//-------------------------------------------------------------------
void CAnimator::SetTime(float time)
{ float fps = float(1) / time;
  aTmax     = time;
  std::vector<CAniPart*>::iterator w;
  for (w = obj.begin(); w != obj.end(); w++)
  { (*w)->SetFPS(fps); }
  return;
}
//-------------------------------------------------------------------
//  Return part ground status
//  Each part that is on ground return 1
//-------------------------------------------------------------------
char CAnimator::GroundState()
{ char grd = 0;
  std::vector<CAniPart*>::iterator w;
  for (w = obj.begin(); w != obj.end(); w++)
  { grd += (*w)->GroundState(); }
  return grd;
}
//====================================================================
// CAcmPart represents a single (possibly animated) part within the model.
//====================================================================
CAcmPart::CAcmPart (CModelACM *md,int n, float minValue, float maxValue)
{*name   = 0;
  mother = 0;
  model  = md;
  // Allocate storage for keyframe data table and init 'next entry' index
  nFrames = n;
  kFrame  = 0;
  if (n)  kFrame =   new CKeyFrame[nFrames];
  next  = 0;
  fps   = 0;
  //---Init spatial extension -------------------------
  CVector zer(0,0,0);
  minP  = zer;
  maxP  = zer;
	//--- Init geometry parameters ----------------------
	First	= 0;
	Count	= 0;
  //---------------------------------------------------
  SetDiffuse (false);
  SetSpecular(false, 0);
  Render  = GL_TRIANGLES;
  //---------------------------------------------------
  txRef = 0;                    // Texture reference
  // Initialize part transform
  transform.dx = transform.dy = transform.dz = 0;
  transform.p  = transform.b  = transform.h  = 0;
  //---------------------------------------------------
  alpha = 1;
  Type  = 0;
  uprm  = 0;
}
//-------------------------------------------------------------------------------------
//  Destroy the part
//	Do not destroy the child list parts.  They are deleted form top level
//-------------------------------------------------------------------------------------
CAcmPart::~CAcmPart (void)
{ globals->txw->Free3DTexture(txRef);
//  TRACE("DELETE PART %s",name);
	childList.clear();
  SAFE_DELETE_ARRAY (kFrame);
}
//-------------------------------------------------------------------------------------
//  Relocate at end of parent child list
//-------------------------------------------------------------------------------------
void CAcmPart::Relocate()
{ CAcmPart *mop = mother;               // Mother part
  if (0 == mop)   return;
  mop->Relocate(this);
  return;
}
//-------------------------------------------------------------------------------------
//  Relocate part at end of child list
//-------------------------------------------------------------------------------------
void CAcmPart::Relocate(CAcmPart *prt)
{ std::vector<CAcmPart*>::iterator it;
  for (it = childList.begin(); it != childList.end(); it++)
  { if (*it != prt)  continue;
    childList.erase(it);
    childList.push_back(prt);
    return;
  }
  return;
}
//-------------------------------------------------------------------------------------
//  Copy rotation parameters
//-------------------------------------------------------------------------------------
void CAcmPart::CopyRotation(int k, CKeyFrame &kf)
{ CKeyFrame *dst = kFrame + k;
  dst->p = kf.p;
  dst->b = kf.b;
  dst->h = kf.h;
}
//-------------------------------------------------------------------------------------
//  Save part extension
//-------------------------------------------------------------------------------------
void CAcmPart::SetExtension(GN_VTAB &v)
{ float x = v.VT_X;
  float y = v.VT_Y;
  float z = v.VT_Z;
  //---Save this part minimum extension ------------------------
  if (x < minP.x) minP.x = x;
  if (y < minP.y) minP.y = y;
  if (z < minP.z) minP.z = z;
  //---Save this part maximum extension ------------------------
  if (x > maxP.x)  maxP.x = x;
  if (y > maxP.y)  maxP.y = y;
  if (z > maxP.z)  maxP.z = z;
}
//-------------------------------------------------------------------------------------
//  Return the part radius: This is the maximum spatial extension
//-------------------------------------------------------------------------------------
double CAcmPart::GetRadius()
{ double rad = maxP.x - minP.x;
  double spy = maxP.y - minP.y;
  if (spy > rad)  rad = spy;
  double spz = maxP.z - minP.z;
  if (spz > rad)  rad = spz;
  return rad;
}
//-------------------------------------------------------------------------------------
//  Set part name and check for transparent glass
//-------------------------------------------------------------------------------------
void CAcmPart::setName (const char *s)
{ strncpy (name, s,63);
  return;
}
//-------------------------------------------------------------------------------------
//  Set part name and check for transparent glass
//-------------------------------------------------------------------------------------
void CAcmPart::setTransparent()
{ Trans = 1;
  return;
}
//-------------------------------------------------------------------------------------
//  Set diffuse color
//-------------------------------------------------------------------------------------
void CAcmPart::SetDiffuse(bool d)
{ vDIF = (d)?(white):(black);
  return;
}
//-------------------------------------------------------------------------------------
//  Add key frame
//-------------------------------------------------------------------------------------
CKeyFrame *CAcmPart::addKeyframe (CKeyFrame kf)
{ if (next >= nFrames) return 0;
  // Store keyframe values
  CKeyFrame *p = &kFrame[next];
  *p = kf;
  // Increment index of next keyframe
  next++;
  return p;
}
//-------------------------------------------------------------------------------------
//  Get a copy of keyframe
//-------------------------------------------------------------------------------------
CKeyFrame *CAcmPart::GetCopyKey()
{ CKeyFrame *k = new CKeyFrame;
  *k  = transform;
  return k;
}
//-------------------------------------------------------------------------------------
void CAcmPart::addChild (CAcmPart *child)
{
  childList.push_back (child);
}
//========================================================================================
#define PROP_5DEG_INCR (float(1.01) / 72)
#define PROP_SDEG_INCR (float(1.01) / 59)
#define PROP_HALF_INCR (36 * PROP_5DEG_INCR)
//========================================================================================
//  Sort all parts so that transparent parts are last
//========================================================================================
void CAcmPart::SortChildTransparency (void)
{
  std::stable_sort (childList.begin(), childList.end(), SortChildPartsTowardTransparency);

}
//--------------------------------------------------------------------------
//  Set Part Keyframe corresponding to the time
//--------------------------------------------------------------------------
float CAcmPart::TimeKey(float time)
{ float key = time * fps;
  setKeyframe(key);
  return transform.frame;
}
//--------------------------------------------------------------------------
//  Trace ending keyframe
//--------------------------------------------------------------------------
void CAcmPart::TracePart()
{ CKeyFrame &k = transform;
  TRACE("--Part %-20s: KF=%.05f P=%.05f B=%.05f H=%.05f",
        name,k.frame,k.p,k.b,k.h);
}
//--------------------------------------------------------------------------
//  Set Part Keyframe
//--------------------------------------------------------------------------
void CAcmPart::ResetKey(int k)
{transform.frame  = kFrame[k].frame;
 transform.dx     = kFrame[k].dx;
 transform.dy     = kFrame[k].dy;
 transform.dz     = kFrame[k].dz;
 transform.p      = kFrame[k].p;
 transform.b      = kFrame[k].b;
 transform.h      = kFrame[k].h;
 //TracePart();
 return;
}
//--------------------------------------------------------------------------
//  Allocate n keyframes
//--------------------------------------------------------------------------
void CAcmPart::AllocateKeyframe(int n)
{ nFrames = n;
  kFrame  = new CKeyFrame[n];
  return;
}
//--------------------------------------------------------------------------
//  Set Part Keyframe
//--------------------------------------------------------------------------
void CAcmPart::setKeyframe (float frame)
{
  // Search for keyframes before and after the frame value
  CKeyFrame *prv   = 0;
  CKeyFrame *aft   = 0;
  transform.frame     = frame;
  if (frame <= 0) return ResetKey(0);
  if (frame >= 1) return ResetKey(nFrames-1);
  for (int i=0; i<nFrames; i++)
  { CKeyFrame *f = &kFrame[i];
    if (f->frame == frame) return ResetKey(i);
    if (f->frame < frame)
    { // Replace previous' with this keyframe
      prv = f;
      continue;
    }
    return Interpolate(prv,f);
  }
  gtfo ("Could not find pre/post entries");
}
//--------------------------------------------------------------------------
//  Inc Part Keyframe
//--------------------------------------------------------------------------
void CAcmPart::IncKeyframe(float dF)
{ float fr = transform.frame;
  fr      += dF;
  if (fr > 1)  fr -= 1;
  setKeyframe(fr);
  return;
}
//--------------------------------------------------------------------------
//  Inc Part Keyframe
//--------------------------------------------------------------------------
void CAcmPart::ModifyKeyframe(float dF)
{ float fr = transform.frame;
  fr      += dF;
  if (fr > 1)  fr -= 1;
  if (fr < 0)  fr += 1;
  setKeyframe(fr);
  return;
}

//--------------------------------------------------------------------------
//  Rotate Keyframe
//--------------------------------------------------------------------------
void CAcmPart::RotateKeyframe(float dF)
{ float fr = transform.frame;
  fr      += dF;
  if (fr > 1)  fr -= 1;
  setKeyframe(fr);
  transform.dx = 0;
  transform.dy = 0;
  transform.dz = 0;
  return;
}

//--------------------------------------------------------------------------
//  Trace keyframes
//--------------------------------------------------------------------------
void CAcmPart::TraceKey()
{ TRACE("Part %s FPS %.02f",name,fps);
  CKeyFrame *kf = kFrame;
  for (int k = 0; k != nFrames; k++)
  { TRACE("    Frame : %0.04f",kFrame->frame);
    kf++;
  }
  return;
}
//--------------------------------------------------------------------------
//  check for keyframe destruction. Only for debbugging
//--------------------------------------------------------------------------
void CAcmPart::CheckKeys()
{CKeyFrame *kf = kFrame;
 for (int k = 0; k != nFrames; k++)
 { float fr = kf->frame;
   if ((fr < 0) || (fr > 1)) gtfo("Frame destroyed"); 
   kf++;
 }
 return;
}
//--------------------------------------------------------------------------
//  Set keyframe and trace
//--------------------------------------------------------------------------
void CAcmPart::SetAndTrace(float k)
{ setKeyframe(k);
  CKeyFrame *t = &transform;
  TRACE("Key %0.4f Part %s:",k,name);  
  TRACE("    Dx=%f Dy=%f Dz=%z Dp=%f Db=%f Dh=%f",
    t->dx,t->dy,t->dz,t->p,t->b,t->h);
    
  return;
}
//--------------------------------------------------------------------------
//  Add vertex array
//--------------------------------------------------------------------------
void CAcmPart::AddArray(int nb, GN_VTAB *tab)
{ //nFaceVerts = nb;
  //vxTAB = tab;
  return;
}
//--------------------------------------------------------------------------
//  Set Rotation
//--------------------------------------------------------------------------
void CAcmPart::CopyRotation(SVector &r)
{ transform.p = r.x;
  transform.b = r.y;
  transform.h = r.z;
  return;
}

//--------------------------------------------------------------------------
//  Set Translation
//--------------------------------------------------------------------------
void CAcmPart::CopyTranslation(SVector &t)
{ transform.dx = t.x;
  transform.dy = t.y;
  transform.dz = t.z;
  return;
}
//------------------------------------------------------------------------------
//  Set no transformation
//------------------------------------------------------------------------------
void  CAcmPart::Center()
{ transform.dx = 0;
  transform.dy = 0;
  transform.dz = 0;
  return;
}
//--------------------------------------------------------------------------
//  Interpolate between 2 frames
//  NOTE:  transform.frame is the requested frame, not necessarily an existing
//         frame. However this should be left as it is, beacuse it is returned
//         to the requestor as the reached frame.
//--------------------------------------------------------------------------
void CAcmPart::Interpolate(CKeyFrame *prv, CKeyFrame *aft)
{ if (0 == prv) return; 
  float frame = transform.frame;
  float scale = (frame - prv->frame) / (aft->frame - prv->frame);
  float dx    = aft->dx - prv->dx;
  float dy    = aft->dy - prv->dy;
  float dz    = aft->dz - prv->dz;
  float dp    = (prv->indP)?(0):(aft->p  - prv->p);
  float db    = (prv->indB)?(0):(aft->b  - prv->b);
  float dh    = (prv->indH)?(0):(aft->h  - prv->h);
  // Update current part transform values
  transform.dx  = prv->dx + (scale * dx);
  transform.dy  = prv->dy + (scale * dy);
  transform.dz  = prv->dz + (scale * dz);
  transform.p   = Wrap360(prv->p + (scale * dp));
  transform.b   = Wrap360(prv->b + (scale * db));
  transform.h   = Wrap360(prv->h + (scale * dh));
  //TracePart();
  return;
}
//------------------------------------------------------------------------------
//  Set specular effect
//------------------------------------------------------------------------------
void CAcmPart::SetSpecular (bool s, float pw)
{ vSPC = (s)?(white):(black);
  power = pw;
  return;
}

//------------------------------------------------------------------------------
//  Prepare shadow drawing
//  Draw only for the BODY_TRANSFORM mode
//  Shadow is drawed as a non textured object in almost black color
//------------------------------------------------------------------------------
void CAcmPart::DrawAsShadow(char mode)
{ if (mode != BODY_TRANSFORM) return;
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glDisable (GL_DEPTH_TEST);
  glDisable (GL_LIGHTING);
  glDisable (GL_TEXTURE_2D);
  glShadeModel (GL_FLAT);
  glDisable(GL_BLEND);
  float col, alf;
  globals->wtm->GetShadowParameters(&col,&alf);     // F(cloud cover)
  glColor4f (col,col,col,alf);            // Shadow's color
	DrawShadow(mode);
  //-------- Restore previous state ---------------------------
  glPopAttrib();
  return;
}
//------------------------------------------------------------------------------
//  DrawShadow
//  Mode 1: Normal mode
//  Mode 0: Fixed mode: No PHB tranformation 
//------------------------------------------------------------------------------
void CAcmPart::DrawShadow (char mode)
{ if (PART_SOBJ == Type) return;
  //
  // Create a shadow by rendering the object using the shadow matrix.
  //
  glPushMatrix ();
  {  
     // object's position & orientation 
     // (needs to use the same transformations used to render the actual object)
     //-----Render the part ------------------------------------------
     switch (Type) {

        case PART_BODY:
          { //---Parent node ---------------------------
            if (mode)
            {	glMultMatrixf(model->GetShadowMatrix());
              model->GetVEH()->Rotate();
              transform.h = 0.0f; transform.p = 0.0f; transform.b = 0.0f; // 
              break;
            }
            break;
          }

        case PART_CHILD:
          //----Child part ---------------------------------
          { glTranslatef (transform.dx, transform.dy, transform.dz);
						glRotatef (transform.h, 0, 0, 1);   // Heading (Z)
            glRotatef (transform.p, 1, 0, 0);   // Pitch   (X)
            glRotatef (transform.b, 0, 1, 0);   // Bank    (Y);
            break;
          }

     }
     //----Render geometry --------------------------------------
		 glDrawArrays (Render,First, Count);
     //---------- Draw child parts --------------------------------
     std::vector<CAcmPart*>::iterator i;
     for (i=childList.begin(); i!=childList.end(); i++) {
       CAcmPart *child = *i;
       child->DrawShadow (mode);
     }
  }

  glPopMatrix ();
  return;
}
//------------------------------------------------------------------------------
//  Draw part
//  Mode 1: Normal mode
//  Mode 0: Fixed mode: No PHB tranformation 
//  TODO: Propeller drawing.
//------------------------------------------------------------------------------
void CAcmPart::Draw (char mode)
{ // Apply current part transformation. The order in which the orientation angles
  //   are applied is important!!  Order should be heading, pitch, bank.
  //glMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  //-----Render the part ------------------------------------------
  switch (Type) {

      case PART_BODY:
        { //---Parent node ---------------------------
          if (mode)
          { model->GetVEH()->Rotate();
            transform.h = 0.0f; transform.p = 0.0f; transform.b = 0.0f; // 
            break;
          }
        }

      case PART_CHILD:
        //----Child part ---------------------------------
        { glTranslatef (transform.dx, transform.dy, transform.dz);
					glRotatef (transform.h, 0, 0, 1);   // Heading (Z)
          glRotatef (transform.p, 1, 0, 0);   // Pitch   (X)
          glRotatef (transform.b, 0, 1, 0);   // Bank    (Y);
          break;
        }

      case PART_SPINR:
        //----Child part ---------------------------------
        { DrawAsSpinr(mode,transform);
          return;
        }

      case PART_SOBJ:
        { glTranslated (transform.dx + globals->plugins->dVeh->sobj_offset.x,
                  transform.dy + globals->plugins->dVeh->sobj_offset.y,
                  transform.dz + globals->plugins->dVeh->sobj_offset.z);
          SVector  vect = globals->plugins->dVeh->GetOrientDegre ();
          glRotated ( vect.z, 0, 0, 1);      // Heading around Z
          glRotated ( vect.x, 1, 0, 0);      // Pitch   around X
          glRotated ( vect.y, 0, 1, 0);      // Bank    around Y
          transform.h = 0.0f; transform.p = 0.0f; transform.b = 0.0f;
        }
  }
  //---Set transparency factor ------------------------------
  white[3] = alpha;
  glEnable(GL_BLEND);
  //-- Set texturing parameters -----------------------------
	model->BindTxObject(txOBJ);
  //--- Set diffuse property --------------------------------
  glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, vDIF);
  //-- Set specular lighting parameters ---------------------
  glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, vSPC);
  //---Render geometry --------------------------------------
	glDrawArrays (Render,First, Count);
  //---- Restore opacity ----------
  white[3] = 1.0;
  //---------- Draw child parts -----------------------------
  std::vector<CAcmPart*>::iterator i;
  for (i=childList.begin(); i!=childList.end(); i++) {
    CAcmPart *child = *i;
    child->Draw (mode);
  }
  //-------- Restore previous state ---------------------------
  glPopMatrix ();
  //glMatrixMode (GL_MODELVIEW);
  return;
}
//-------------------------------------------------------------------------
//  Set current RPM (Rotation per minute)
//  A sector increment is computed when the speed is below 400 rpm
//-------------------------------------------------------------------------
void CAcmPart::SetRPM(float rpm)
{ float rat = rpm / 2500;
  uprm = rat * (float(1) / 63);
  return;
}
//------------------------------------------------------------------------------
//  Specific Draw for spinner part
//------------------------------------------------------------------------------
void CAcmPart::DrawInternalSpin(SVector &v)
{ glPushMatrix ();
  CKeyFrame kf;
  kf.dx = v.x;
  kf.dy = v.z;
  kf.dz = v.y;
  kf.h  = transform.h;
  kf.p  = transform.p;
  kf.b  = transform.b;
  glTranslated(v.x,v.z,v.y);
  DrawAsSpinr(BODY_TRANSFORM,kf);
  return;
}
//------------------------------------------------------------------------------
//  Specific Draw for spinner part
//------------------------------------------------------------------------------
void CAcmPart::DrawAsSpinr(char mode,CKeyFrame &kf)
{ //--- Rotate spinner and blades ----------------------
  glRotatef (transform.h, 0, 0, 1);   // Heading (Z)
  glRotatef (transform.p, 1, 0, 0);   // Pitch   (X)
  glRotatef (transform.b, 0, 1, 0);   // Bank    (Y)
  glDisable(GL_CULL_FACE);
  //---------- Draw child parts --------------------------------
  if (uprm > 0) DrawFastProp(kf);
  else          DrawFixeProp(kf);
  //---------- Draw spinner cap  -------------------------------
  //-- Set texturing parameters --------------------------------
	model->BindTxObject(txOBJ);
	//--- Set diffuse property -----------------------------------
  glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, vDIF);
  //-- Set specular lighting parameters ------------------------
  glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, vSPC);
  glColor4fv(diffu);
  glEnable(GL_CULL_FACE);
	//--- Render geometry ---------------------------------------
	glDrawArrays (Render,First, Count);
  //-------- Restore previous state ---------------------------
  glPopMatrix ();
  return;
}
//------------------------------------------------------------------------------
//  Draw the non moving propellor 
//------------------------------------------------------------------------------
void CAcmPart::DrawFixeProp(CKeyFrame &kf)
{ glPopMatrix();
  glPushMatrix();
  glTranslatef (kf.dx, kf.dy, kf.dz);
  glRotatef (transform.h, 0, 0, 1);   // Heading (Z)
  glRotatef (transform.p, 1, 0, 0);   // Pitch(X)
  glRotatef (transform.b, 0, 1, 0);   // Bank (Y);
  std::vector<CAcmPart*>::iterator i;
  for (i=childList.begin(); i!=childList.end(); i++) {
    CAcmPart *child = *i;
    child->DrawAsBlade ();
  }
  return;
}
//------------------------------------------------------------------------------
//  Draw as a slow moving part
//------------------------------------------------------------------------------
void CAcmPart::DrawSlowProp(CKeyFrame &kf)
{ white[3] = 0.5f;
  IncKeyframe(uprm);
  glPopMatrix();
  glPushMatrix();
  glTranslatef (kf.dx, kf.dy, kf.dz);
  glRotatef (transform.h, 0, 0, 1);   // Heading (Z)
  glRotatef (transform.p, 1, 0, 0);   // Pitch(X)
  glRotatef (transform.b, 0, 1, 0);   // Bank (Y);
  glDepthMask(GL_FALSE);
  std::vector<CAcmPart*>::iterator i;
  for (i=childList.begin(); i!=childList.end(); i++) {
    CAcmPart *child = *i;
    child->DrawAsBlade ();
  }
  glDepthMask(GL_TRUE);
  white[3] = 1;
  return;
}

//------------------------------------------------------------------------------
//  Draw the animated propellor 
//  -The props are drawed throught the spinner
//  -The Prop are drawed as several blended frame rotated by a 1° amount 
//   We made as much steps as needed to cover 180° 
//   depending on the number of blades. If there are 3 blades we only need to
//   make (180 / 3) = 60 drawing steps.
//------------------------------------------------------------------------------
void CAcmPart::DrawFastProp(CKeyFrame &kf)
{ white[3] = 0.025f;
  int nb     = childList.size();
  int end    = (180 / nb);                     // Sector to cover
  glDepthMask(GL_FALSE);
  for (int k = 0; k <= end; k++)
  { 
    glPopMatrix();
    glPushMatrix();
    glTranslatef (kf.dx, kf.dy, kf.dz);
    glRotatef (transform.h, 0, 0, 1);   // Heading (Z)
    glRotatef (transform.p, 1, 0, 0);   // Pitch(X)
    glRotatef (transform.b, 0, 1, 0);   // Bank (Y);
    std::vector<CAcmPart*>::iterator i;
    for (i=childList.begin(); i!=childList.end(); i++) {
      CAcmPart *child = *i;
      child->DrawAsBlade ();
    }
 //   IncKeyframe(PROP_SDEG_INCR);
    IncKeyframe(uprm);
  }
  glDepthMask(GL_TRUE);
  white[3]  = 1;
  return;
}

//------------------------------------------------------------------------------
//  Specific Draw for blades part
//  alf is the blending factor
//------------------------------------------------------------------------------
void CAcmPart::DrawAsBlade()
{ //---Put blade in place --------------------------
  glPushMatrix ();
  glTranslatef (transform.dx, transform.dy, transform.dz);
  //--- Rotate blades -------------------------------
  glRotatef (transform.h, 0, 0, 1);   // Heading (Z)
  glRotatef (transform.p, 1, 0, 0);   // Pitch(X)
  glRotatef (transform.b, 0, 1, 0);   // Bank (Y);
  //-- Set texturing parameters -----------------------------
	model->BindTxObject(txOBJ);
  //--- Set child properties ----------------------------------
  glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, vDIF);
  //-- Set specular lighting parameters ------------------------
  glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, vSPC);
  //----Render geometry --------------------------------------
	glDrawArrays (Render,First, Count);
  //-------- Restore previous state ---------------------------
  glPopMatrix ();
  return;
}

//-----------------------------------------------------------------------------
void CAcmPart::Print (FILE *f)
{ /*
  int index = 0;
  fprintf (f, "  Name  : %s\n", name);
  fprintf (f, "  Index : %d\n", index);
  fprintf (f, "  nFrames = %d\n", nFrames);
  fprintf (f, "  min/max = %f/%f\n", 0.0f, 1.0f);

  fprintf (f, "  Keyframes:\n");
  for (int i=0; i<nFrames; i++) {
    CKeyFrame *kf = &kFrame[i];
    fprintf (f, "    %2d :  %8.2f  %8.2f, %8.2f, %8.2f   %8.2f,%8.2f,%8.2f\n",
      i, kf->frame, kf->dx, kf->dy, kf->dz, kf->p, kf->b, kf->h);
  }
  fprintf (f, "\n");

  // Print child parts
  std::vector<CAcmPart*>::iterator it;
  for (it=childList.begin(); it!=childList.end(); it++) {
    CAcmPart *child = *it;
    child->Print (f);
  }
	*/
}
//-----------------------------------------------------------------------------
//  Create a new model ACM
//  t :     The top part type for drawing
//  d :     The child dependent part type
//-----------------------------------------------------------------------------
CModelACM::CModelACM(char t,char d, CVehicleObject *mv)
{ mveh    = mv;
  Top     = t;
  Dpn     = d;
  tr      = 0;
  int  op = 0;
  GetIniVar("TRACE","ACM",&op);
  tr      = op;
  topParent = 0;
	RenderVBO();
	//---Geometry parameters ------------------------
	int opt = 0;
	GetIniVar("Performances","UsePlaneVBO",&opt);
	vbu			= opt;
	nbVT		= 0;
	vBUF		= 0;
	oVBO		= 0;
	cOBJ		= 0;
	if (0==vbu)	RenderMEM();
}
//-----------------------------------------------------------------------------
//  Add a new Part
//-----------------------------------------------------------------------------
void CModelACM::AddPart(short No,int index,CAcmPart *part)
{ //--- Get a reference to the parent's CAcmPart
  CAcmPart *parent = 0;
  part->SetIndex(No);
  if (index == -1)
    //---Top parent part -----------
  { topParent  = part;
    part->SetType(Top);
  }
  //----Child part -----------------
  else
  { parent = mapParent[index];
    parent->addChild (part);
    part->SetType(Dpn);
  } 
  part->SetParent(parent);
  // Save a reference to this part in the parent table
  mapParent[No] = part;
  // Add the part to the map for lookup of part names for animation
  char *name    = part->GetPartName();
  partMap[name] = part;
  return;
}
//-----------------------------------------------------------------------------
//  Load a new model ACM from ACM file
//  t :     The top part type for drawing
//  d :     The child dependent part type
//-----------------------------------------------------------------------------
void  CModelACM::LoadFrom(char* acmFilename)
{ TEXT_INFO	txd;										// Texture descriptor
	//---List of vertex tables ----------------------------------
	std::vector<CAcmVPack*> pack;
	CAcmVPack *pak = 0;
	//MEMORY_LEAK_MARKER ("macm_load");
  PODFILE *p = popen (&globals->pfs, acmFilename);
  if (0 == p) gtfo ("CModelACM : Could not open ACM model %s", acmFilename);

  static const int LINE_LENGTH = 128;
  char s[LINE_LENGTH+1];

  // Object type, for aircraft this should be "CAircraftModel"
  pgets (s, LINE_LENGTH, p);
  if (strncmp (s, "CAircraftModel", strlen("CAircraftModel")) != 0) {
    WARNINGLOG ("CModelACM : Incorrect object type in %s", acmFilename);
  }

  // Version, always 1 (ignored)
  pgets (s, LINE_LENGTH, p);

  // Part count
  int nParts;
  pgets (s, LINE_LENGTH, p);
  if (sscanf (s, "%d", &nParts) != 1) {
    // Error
    gtfo ("CModelACM : Error parsing part count in %s", acmFilename);
  }

  // Part list
  for (int i=0; i<nParts; i++) {
    CAcmPart *part = new CAcmPart (this,31,0,1);

    //--- Part name ----------------------
    char name[LINE_LENGTH];
    pgets (s, LINE_LENGTH, p);
    TrimTrailingWhitespace (s);
    strncpy (name, s, 127);
    part->setName (name);
    //--- Part status (on/off) -----------
    pgets (s, LINE_LENGTH, p);
    //--- Parent part index --------------
    int p_index = -1;
    pgets (s, LINE_LENGTH, p);
    if (sscanf (s, "%d", &p_index) != 1) {
      // Error
      gtfo ("CModelACM : Error parsing parent index in %s", acmFilename);
    }
    AddPart(i,p_index,part);
    //---- Version, always 1 -------------------------------------------
    pgets (s, LINE_LENGTH, p);

    // Vertex count, frame count, face count, unused value
    int nVerts, nFrames, nFaces, dummy;
    pgets (s, LINE_LENGTH, p);
    if (sscanf (s, "%d,%d,%d,%d", &nVerts, &nFrames, &nFaces, &dummy) != 4) {
      // Error
      gtfo ("CModelACM : Error parsing vertex and face counts in %s", acmFilename);
    }

    // Optional line "vl" ???
    bool v1flag = false;
    pgets (s, LINE_LENGTH, p);
    if (strncmp (s, "v1", 2) == 0) {
      //  ?? What does this mean?
      v1flag = true;
      //  Pre-read the next line
      pgets (s, LINE_LENGTH, p);
    }

    // Diffuse value, specular value, specular power, use transparency,
    //   use environment mapping, texture name
    int diffuse, specular, specpower, transparent, envmap;
    if (sscanf (s, "%d,%d,%d,%d,%d,%s",
      &diffuse, &specular, &specpower, &transparent, &envmap, txd.name) != 6) {
      // Error
      gtfo ("CModelACM : Error parsing material parameters in %s", acmFilename) ;
    }
    part->SetDiffuse (diffuse != 0);
    part->SetSpecular (specular != 0, specpower);
    part->setTransparent();
    part->SetEnvMapped (envmap != 0);
    //-----Load texture from texture ward -------------------------------
		txd.apx  = 0xFF;
		txd.azp  = transparent;
		txd.Dir  = FOLDER_ART;
    tRef = globals->txw->GetM3DPodTexture(txd);
    U_INT obj = globals->txw->Get3DObject(tRef);
    part->SetTexREF(tRef,obj);
    //-------------------------------------------------------------------

    // If the line before last was "v1" then skip the next line, which is
    //   always a blank string ""
    if (v1flag) {
      pgets (s, LINE_LENGTH, p);
    }

    // Before reading vertex list, allocate table of (temporary) structs
    //   to hold the vertex table until the face data is read
    GN_VTAB *vt = new GN_VTAB[nVerts];

    //--- Vertex list --------------------------------------------------
    for (int j=0; j<nVerts; j++) {
      // x, y, z, nx, ny, nz, u, v
      float x, y, z, nx, ny, nz, u, v;
      pgets (s, LINE_LENGTH, p);
      if (sscanf (s, "%f,%f,%f,%f,%f,%f,%f,%f",
        &x, &y, &z, &nx, &ny, &nz, &u, &v) != 8) {
        // Error
        gtfo ("CModelACM : Error parsing vertex %d in %s", j, acmFilename) ;
      }

      //--- Assign values to (temporary) vertex table -----------------
      // z- and y-coordinates are swapped for LHS-RHS system conversion
      GN_VTAB *pNext = vt + j;         //&vt[j];
      pNext->VT_X  = x;
      pNext->VT_Y  = z;
      pNext->VT_Z  = y;
      pNext->VN_X  = nx;
      pNext->VN_Y  = nz;
      pNext->VN_Z  = ny;
      pNext->VT_S  = u;
      pNext->VT_T  = v;
    }

    // Allocate storage for vertex, normal and texcoord arrays for the part
		int nbvx	= nFaces * 3;
		pak = new CAcmVPack(nbvx,part);
		pack.push_back(pak);
		nbVT	+= nbvx;
    //---Process  Face list ---------------------------------------------
    for (int k=0; k<nFaces; k++) {
      // i1, i2, i3 (vertex indices, clockwise)
      int i1, i2, i3;
      pgets (s, LINE_LENGTH, p);
      if (sscanf (s, "%d,%d,%d",
        &i1, &i2, &i3) != 3)
      {
        // Error
        gtfo ("CModelACM : Error parsing face %d in %s", k, acmFilename) ;
      }

      // NOTE: ACM meshes use left-hand coordinate system.
      //- all triangle vertices are reversed order (3, 2, 1)
      //  for positive direction
			pak->AddFaceVertex  ((k * 3),    vt[i3]);
      SaveExtension(name,vt[i3]);
			part->SetExtension(vt[i3]);
			pak->AddFaceVertex ((k * 3) + 1, vt[i2]);
      SaveExtension(name,vt[i2]);
			part->SetExtension(vt[i2]);
			pak->AddFaceVertex  ((k * 3) + 2, vt[i1]);
      SaveExtension(name,vt[i1]);
			part->SetExtension(vt[i1]);
    }

    //--- Clean up ACM vertex list -------------------------------------
    delete vt;

    // 31 Animation frames, each interval is therefore 1/30
    static const float epsilon = float(1e-3);
    static const float kpi     = float(3.141);
    float kfStep = 1.0f / 30.0f;
    //--------Check for Trace ------------------------------------------
    if (tr) TRACE("ACM Part %-20s:",name);
    //------------------------------------------------------------------
    CKeyFrame *pf = 0;
    CKeyFrame  kf;          // working keyframe
    //----Process list keyframe for transformation --------------
    for (int m=0; m<31; m++) {
      // dx, dy, dz, p, b, h (from ACM file format reference)
      float dx, dy, dz, rx, ry, rz;
      pgets (s, LINE_LENGTH, p);
      if (sscanf (s, "%f,%f,%f,%f,%f,%f",
        &dx, &dy, &dz, &rx, &rz, &ry) != 6)
      {
        // Error
        gtfo ("CModelACM : Error parsing animation frame %d for part %d in %s",
          m, i, acmFilename) ;
      }
      
      // Eliminate small spurious translations and rotations
      if (fabs(dx) < epsilon) dx = 0.0f;
      if (fabs(dy) < epsilon) dy = 0.0f;
      if (fabs(dz) < epsilon) dz = 0.0f;
      if (fabs(rx) < epsilon) rx = 0.0f;
      if (fabs(ry) < epsilon) ry = 0.0f;
      if (fabs(rz) < epsilon) rz = 0.0f;
      //------------------------------------------------------------------
      // JS: In the original ACM file, parts are suddenly rotated by PI.
      //      For instance the wheel at the end of node chain is not
      //      continuously rotated from (0 to 2PI) along the 31 frames
      //      Thus, the interpolate function rotate the part, but this
      //      is wrong. This does not show on FLYII while it shows on LEGACY
      //  To prevent this to happen, we must avoid to interpolate between
      //  2 keyframes that has brutal transition from 0 to +/- PI
      //--- idem with PI -------------------------------------------------
      float  np = RadToDeg(-rx);
      float  nb = RadToDeg(-rz);
      float  nh = RadToDeg(-ry);
      //------------------------------------------------------------------
      // Convert keyframe from  LH to RH coordinate systems
      kf.indP  = 0;
      kf.indB  = 0;
      kf.indH  = 0;
      kf.frame = kfStep * (float)m;
      kf.dx    = dx;
      kf.dy    = dz;
      kf.dz    = dy;
      kf.p     = np;
      kf.b     = nb;
      kf.h     = nh;
      //---Check for gaps ------------------------------------------------
      CheckGap(pf,&kf);
      //---Check for Trace -----------------------------------------------
      if (tr) TRACE("Kf=%.05f dx=%.05f dy=%.05f dz=%.05f P=%.05f B=%.05f H=%.05f",
       kf.frame,dx,dy,dz,kf.p,kf.b,kf.h);
      //------------------------------------------------------------------
      pf = part->addKeyframe (kf);
    }

    //--- Initialize to first keyframe -----------------------------
    part->setKeyframe (0.0f);

  }
	//-----------------------------------------------------------------
	//  Pack all vertices into one buffer at model level
	//	Set indice in each part where the part begins
	//-----------------------------------------------------------------
	BuildVBO(nbVT,pack);
  //--- Close pod file ----------------------------------------------
  pclose (p);
  if (0 == topParent)     gtfo ("CModelACM::Draw : No top-level parent part in model");
  /// sort child parts for transparency
  /// workaround for proper model transparency 
  for (U_INT i = 0; i < mapParent.size (); ++i)  mapParent[i]->SortChildTransparency ();
  //MEMORY_LEAK_MARKER ("macm_load");
  return;
}
//---------------------------------------------------------------------------
//  Build a VBO from the list of packed parts
//---------------------------------------------------------------------------
void CModelACM::BuildVBO(int nbv,std::vector<CAcmVPack*> &pack)
{ CAcmVPack *pak = 0;												// Current pack
	nbVT			= nbv;
	vBUF			= new GN_VTAB[nbv];							// Allocate a big buffer
	char *dst = (char*)vBUF;									// Destination
	int   ofs = 0;														// Offset
	std::vector<CAcmVPack*>::iterator pk;
	for (pk=pack.begin(); pk!=pack.end(); pk++)
	{	pak			 = *pk;													// Current pack
		int nv   = pak->Copy(dst,ofs);					// Copy vertices
		ofs			+= nv;
		dst		  += (nv * sizeof(GN_VTAB));
		delete pak;															// delete temporary pack
	}
	//--- free the temporary pack -------------------------------------
	pack.clear();
	//--- Allocate the VBO --------------------------------------------
	glGenBuffers(1,&oVBO);
	glBindBuffer(GL_ARRAY_BUFFER,oVBO);
	glBufferData(GL_ARRAY_BUFFER,(nbv * sizeof(GN_VTAB)),vBUF,GL_STATIC_COPY);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	return;
}
//---------------------------------------------------------------------------
//  Destroy the model
//---------------------------------------------------------------------------
CModelACM::~CModelACM (void)
{ //--- Free GL resources ----------------------------
	if (vBUF)		delete [] vBUF;
	if (oVBO)		glDeleteBuffers(1,&oVBO);
	//--- Delete all parts -----------------------------
  std::map<string,CAcmPart*>::iterator i;
  for (i=partMap.begin(); i!=partMap.end(); i++)  delete (i->second);
  partMap.clear();
	mapParent.clear();
}
//---------------------------------------------------------------------------
#define FLT_DELTA float(45.0)
//---------------------------------------------------------------------------
//  Check for a fixed keyframe
//  Check for consecutive keyframes with brutal gap in values
//---------------------------------------------------------------------------
char CModelACM::CheckFixedFrame(float prv, float nxv)
{ float dta = fabs(nxv - prv);
  if (dta > FLT_DELTA)  return 1;
  return 0;
}
//---------------------------------------------------------------------------
//  Check for consecutive keyframes with brutal gap in values
//---------------------------------------------------------------------------
void CModelACM::CheckGap(CKeyFrame *prv, CKeyFrame *kfn)
{ if (0 == prv) return;
  prv->indP  = CheckFixedFrame(prv->p,kfn->p);
  prv->indB  = CheckFixedFrame(prv->b,kfn->b);
  prv->indH  = CheckFixedFrame(prv->h,kfn->h);
  return;
}
//-----------------------------------------------------------------------------
//  Save extension
//  Here we compute the greatest extension of all part to get
//  the aircraft global spatial extension
//-----------------------------------------------------------------------------
void CModelACM::SaveExtension(char *name,GN_VTAB &tab)
{ float x = tab.VT_X;
  float y = tab.VT_Y;
  float z = tab.VT_Z;
  //---Save minimum -------------------
  if (x < minS.x) minS.x = x;
  if (y < minS.y) minS.y = y;
  if (z < minS.z) minS.z = z;
  //---Save maximum -------------------
  if (x > maxS.x) maxS.x = x;
  if (y > maxS.y) maxS.y = y;
  if (z > maxS.z) maxS.z = z;
  //----Save body extension -----------
  if (strncmp(name,"body",4))   return;
  //-- Save minimums ------------------
  if (x < minB.x) minB.x = x;
  if (y < minB.y) minB.y = y;
  if (z < minB.z) minB.z = z;
  //--- Save maximum ------------------
  if (x > maxB.x) maxB.x = x;
  if (y > maxB.y) maxB.y = y;
  if (z > maxB.z) maxB.z = z;
  return;
}
//-----------------------------------------------------------------------------
//  Return spatial extension (all feet) for the airraft
//  x is the wing span
//  y is the aircarft length
//  z is the aircraft height 
//-----------------------------------------------------------------------------
void CModelACM::GetExtension(SVector &v)
{ v.x = maxS.x - minS.x;
  v.y = maxS.y - minS.y;
  v.z = maxS.z - minS.z;
  return;
}
//-----------------------------------------------------------------------------
//  Get body extension
//-----------------------------------------------------------------------------
void CModelACM::GetBodyExtension(SVector &v)
{ v.x = maxB.x - minB.x;
  v.y = maxB.y - minB.y;
  v.z = maxB.z - minB.z;
  return;
}
//-----------------------------------------------------------------------------
//  Get part by name
//-----------------------------------------------------------------------------
CAcmPart *CModelACM::GetPart(char *name)
{ std::map<string,CAcmPart*>::iterator i = partMap.find(string(name));
  return (i != partMap.end())?(i->second):(0);
}
//-----------------------------------------------------------------------------
//  Check all keys
//-----------------------------------------------------------------------------
void CModelACM::CheckKeys()
{ int k = 0;
  std::map<string,CAcmPart*>::iterator i;
  for (i = partMap.begin(); i != partMap.end(); i++)
  { CAcmPart *prt = i->second;
    prt->CheckKeys();
    k++;
  }
  return;
}
//-----------------------------------------------------------------------------
//  Set part key
//-----------------------------------------------------------------------------
void CModelACM::SetPartKeyframe (char* partname, float frame)
{
  std::map<string,CAcmPart*>::iterator i = partMap.find(partname);
  if (i != partMap.end()) {
    CAcmPart *part = i->second;
    part->setKeyframe (frame);
  }
}
//---------------------------------------------------------------------------
//  Compute shadow matrix
//---------------------------------------------------------------------------
bool CModelACM::BuildShadowMatrix()
{ float delta_alt =  mveh->GetAltitude() - globals->tcm->GetGroundAltitude();
  if (delta_alt > 300.0f) return false;
  CVector &sunP_ = *(globals->tcm->SunPosition());
  float lPos[]  = { sunP_.x, sunP_.y, sunP_.z, 1.0f }; // World position of light source
  //
  // Build a shadow matrix using the light's current position and the ground plane
  //
	globals->tcm->SetShadowMatrix (sMatrix, lPos);
	return true;
}
//------------------------------------------------------------------------------------
//	Configure for memory mode
//------------------------------------------------------------------------------------
void CModelACM::ConfMEM(char opt)
{	glTexCoordPointer(2,GL_FLOAT,sizeof(GN_VTAB),&vBUF[0].VT_S);
  glNormalPointer(    GL_FLOAT,sizeof(GN_VTAB),&vBUF[0].VN_X);
  glVertexPointer  (3,GL_FLOAT,sizeof(GN_VTAB),&vBUF[0].VT_X);
	return;	}
//------------------------------------------------------------------------------------
//	Configure for VBO mode
//------------------------------------------------------------------------------------
void CModelACM::ConfVBO(char opt)
{	glBindBuffer(GL_ARRAY_BUFFER,oVBO);
	glTexCoordPointer(2,GL_FLOAT,sizeof(GN_VTAB),0);
  glNormalPointer(    GL_FLOAT,sizeof(GN_VTAB),OFFSET_VBO(2*sizeof(float)));
  glVertexPointer  (3,GL_FLOAT,sizeof(GN_VTAB),OFFSET_VBO(5*sizeof(float)));
	return;	}
//------------------------------------------------------------------------------------
//  Draw the model
//	NOTE on optimization:
//	Childreen part must request model to bind texture
//	Texture is binded only when it is different from the current one
//------------------------------------------------------------------------------------
void CModelACM::Draw (char mode)
{ //---- Assign GL state --------------------------------------------------
  glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
  glPushAttrib (GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT);
  //-----Set all common parameters -----------------------------------------
  glPolygonMode (GL_FRONT, GL_FILL);
  glMaterialfv  (GL_FRONT, GL_AMBIENT, white);
	//------------------------------------------------------------------------
  glShadeModel (GL_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable (GL_TEXTURE_2D);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER,0);
	//------------------------------------------------------------------------
	glEnableClientState(GL_VERTEX_ARRAY);
	Configure(vbu);									// Configure mode memory or VBO
  //-- Draw only the top-level parent...children will be drawn recursively --
	cOBJ	= 0;
  if (mveh->HasOPT(VEH_DW_SHAD))
	{	if (BuildShadowMatrix())	topParent->DrawAsShadow (mode);	}
	//------------------------------------------------------------------------
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glMatrixMode (GL_MODELVIEW);
  topParent->Draw (mode);
  glPopAttrib ();
  glPopClientAttrib ();
  glDisable(GL_ALPHA_TEST);
	glBindBuffer(GL_ARRAY_BUFFER,0);
  return;
}
//---------------------------------------------------------------------------
//  Predrawing by camera object
//---------------------------------------------------------------------------
void CModelACM::PreDraw()
{ glPushAttrib(GL_LIGHTING_BIT);
  glDisable(GL_LIGHTING);
  return;
}
//---------------------------------------------------------------------------
//  Drawing by camera object
//---------------------------------------------------------------------------
void CModelACM::CamDraw(CCamera *cam)
{ topParent->Draw(0);
  return;
}
//---------------------------------------------------------------------------
//  End drawing by camera object
//---------------------------------------------------------------------------
void CModelACM::EndDraw()
{ glPopAttrib();
  return;
}
//---------------------------------------------------------------------------
void CModelACM::Print (FILE* f)
{
  // Draw only the top-level parent...children will be drawn recursively
  if (topParent) {
    topParent->Print (f);
  }
}
//====================================================================
//  CGizmo to display a mark over a picture
//====================================================================
CGizmo::CGizmo()
{ obj = gluNewQuadric();
  gluQuadricDrawStyle(obj,GLU_FILL);

}
//--------------------------------------------------------------------
//  Delete the object
//--------------------------------------------------------------------
CGizmo::~CGizmo()
{ gluDeleteQuadric(obj);
}
//--------------------------------------------------------------------
//  Set drawing state
//--------------------------------------------------------------------
void CGizmo::PreDraw()
{ U_INT msk = (GL_COLOR_BUFFER_BIT + GL_LIGHTING_BIT 
               + GL_DEPTH_BUFFER_BIT + GL_ENABLE_BIT
               + GL_TEXTURE_BIT);
  glPushAttrib(msk);
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  glDisable(GL_TEXTURE_2D);
  glLineWidth(2);
  return;
}
//--------------------------------------------------------------------
//  Prepare to draw
//--------------------------------------------------------------------
void CGizmo::PreCros()
{ PreDraw();
  //glColor4f(1,0,0,1);
	ColorGL(COLOR_RED);
  return;
}
//--------------------------------------------------------------------
//  Draw Gizmo as a cross
//  Current matrix stack is assumed to be MODEL_VIEW 
//--------------------------------------------------------------------
void CGizmo::DrawCros()
{ glPushMatrix();
  glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
  glTranslated(pos.x,pos.y,pos.z);
  glInterleavedArrays (GL_V3F, sizeof(F3_VERTEX), crosTB);
  glDrawArrays (GL_LINES, 0, 6);
  glPopClientAttrib();
  glPopMatrix();
  return;
}
//--------------------------------------------------------------------
//  End  draw
//--------------------------------------------------------------------
void CGizmo::EndCros()
{ glPopAttrib();
  return;
}

//--------------------------------------------------------------------
//  Pre fuel drawing
//--------------------------------------------------------------------
void CGizmo::PreFuel()
{ PreDraw();
  glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
  //glColor4f(0,0,1,1);
	ColorGL(COLOR_PURE_BLUE);
  return;
}
//--------------------------------------------------------------------
//  Draw Gizmo as a Fuel box
//  Current matrix stack is assumed to be MODEL_VIEW 
//  No  0=> Top view
//      1=> Front view
//--------------------------------------------------------------------
void CGizmo::DrawTank(U_CHAR No)
{ U_CHAR nf = (No << 2);        // 4 vertices
  glPushMatrix();
  glTranslated(pos.x,pos.y,pos.z);
  glInterleavedArrays (GL_V3F, sizeof(F3_VERTEX), vtxTK);
  glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE,indTK+nf);
  glPopMatrix();
  return;
}
//--------------------------------------------------------------------
//  End fuel drawing
//--------------------------------------------------------------------
void CGizmo::EndFuel()
{ glPopAttrib();
  glPopClientAttrib();
  return;
}
//--------------------------------------------------------------------
//  Pre Load drawing
//--------------------------------------------------------------------
void CGizmo::PreLoad()
{ PreDraw();
  //glColor4f(1,0,0,1);
	ColorGL(COLOR_RED);
  return;
}
//--------------------------------------------------------------------
//  Draw Gizmo as a ball
//  Current matrix stack is assumed to be MODEL_VIEW 
//--------------------------------------------------------------------
void CGizmo::DrawLoad()
{ glPushMatrix();
  glTranslated(pos.x,pos.y,pos.z);
  gluSphere(obj,0.8,6,6);
  glPopMatrix();
  return;
}
//--------------------------------------------------------------------
//  End fuel drawing
//--------------------------------------------------------------------
void CGizmo::EndLoad()
{ glPopAttrib();
  return;
}
//==========================================================================
//  CAniPart : mother class of animated parts
//==========================================================================
CAniPart::CAniPart()
{ vlod  = 0;
  prop = 0;
  *name = 0;
}
//==========================================================================
//  CAcmGears supports the landing gears and steering wheels
//==========================================================================
CAcmGears::CAcmGears(CVehicleObject *v,char *n)
{ if (0 == v)  gtfo("No Vehicle for Part Group");
  vlod = v->GetLOD();
  gnam  = n;
  vlod->AddGear(this);
  time  = 0;
  cDIR  = 0;
  wAGL  = 0;
  wGRN  = 0;
  wKFS  = 0;
  sTire = 0;
  wPos  = GEAR_EXTENDED;
}
//--------------------------------------------------------------------
//  Destroy this object
//	Dont delete ani AcmPart as they will be deleted at top level
//--------------------------------------------------------------------
CAcmGears::~CAcmGears()
{ if (sTire)  delete sTire;
  gears.clear();
  shok.clear();
  turn.clear();
}
//--------------------------------------------------------------------
//  Clamp value
//--------------------------------------------------------------------
inline float CAcmGears::ClampVal(float v,float a, float b)
{ if (v < a)  return a;
  if (v > b)  return b;
  return v;
}
//--------------------------------------------------------------------
//  Add a gear part
//--------------------------------------------------------------------
void  CAcmGears::AddGear(char *name)
{ if (0 == vlod)      return;
  CAcmPart *wp = vlod->GetPart(name);
  if (wp) gears.push_back(wp);
  return;
}
//--------------------------------------------------------------------
//  Add a shock part
//--------------------------------------------------------------------
void  CAcmGears::AddShok(char *name)
{ if (0 == vlod)      return;
  CAcmPart *wp = vlod->GetPart(name);
  if (wp) shok.push_back(wp);
  return;
}
//--------------------------------------------------------------------
//  Add a tire sound
//--------------------------------------------------------------------
void CAcmGears::AddSound(SVector &pos)
{float lat = fabs(pos.x);
 if (1 >= lat)      return;     // Probably a front or rear tire
 if (sTire)         return;
 //---- Create a new tire sound ---------------------
 Tag  idn = 'tire';
 sTire = new CSoundOBJ(idn,pos);
 Tag  id0 = 'tir0';
 sTire->AddSound(0,id0);
 Tag  id1 = 'tir1';
 sTire->AddSound(1,id1);
 Tag  id2 = 'tir2';
 sTire->AddSound(2,id2);
 sTire->SetGain(0.1f);
 return;
}
//--------------------------------------------------------------------
//  Play a tire sound
//--------------------------------------------------------------------
void CAcmGears::PlayTire(int No)
{ if (0 == sTire) return; 
  sTire->Play(No);
  return;
}
//--------------------------------------------------------------------
//  Add the turn
//--------------------------------------------------------------------
void  CAcmGears::AddTurn(char *name)
{ if (0 == vlod)      return;
  CAcmPart *wp = vlod->GetPart(name);
  if (wp) turn.push_back(wp);
  return;
}
//--------------------------------------------------------------------
//  Add the timer
//--------------------------------------------------------------------
void CAcmGears::AddTime(float t)
{ time = t;
  float fps = float(1) / t;
  if (t <= 0.01)   return;
  //---Compute all animated frame per sec coefficient -------
  std::vector<CAcmPart*>::iterator p;
  for (p = gears.begin(); p != gears.end(); p++)
  { CAcmPart *prt = (*p);
    prt->SetFPS(fps);
  }
  vlod->SetLandTime(t);
  return;
}
//--------------------------------------------------------------------
//  Reset to frame key k
//--------------------------------------------------------------------
void CAcmGears::ResetKey()
{ std::vector<CAcmPart*>::iterator p;
  //---- Reset gear ------------------------------------
  for (p = gears.begin(); p != gears.end(); p++)
  { CAcmPart *prt = (*p);
    prt->ResetKey(0);
  }
  //---- Reset shok ------------------------------------
  for (p = shok.begin(); p != shok.end(); p++)
  { CAcmPart *prt = (*p);
    prt->ResetKey(0);
  }
  return;
}
//--------------------------------------------------------------------
//  Reset shock position
//--------------------------------------------------------------------
void CAcmGears::StartMove()
{  std::vector<CAcmPart*>::iterator p;
  //---- reset socks -------------------------------
  for (p = shok.begin(); p != shok.end(); p++)
  { CAcmPart *prt = (*p);
    prt->ResetKey(0);
  }
  return;
}
//--------------------------------------------------------------------
//  Move Landing parts to direction according to overall time
//  in the wheel scenario
//  NOTE: The time factor is converted into a frame index by
//        each part.
//        Function return 1 when all moving parts in gear list
//        has reached target position tkf.
//  tgf:  The target keyframe
//  dir:  The direction (0 extending, 1 retracting)
//  time: The global time in the scenario from T0
//--------------------------------------------------------------------
int CAcmGears::MovePart(float tkf,char dir,float dT,float time)
{ U_INT   end = 0;
  wPos        = GEAR_IN_TRANSIT;
  char pos    = (dir > 0)?(GEAR_RETRACTED):(GEAR_EXTENDED);
  //TRACE("Wheel %-20s move part: TargK=%.05f dT= %.04f Tim=%.04f",name,tkf,dT,time);
  std::vector<CAcmPart*>::iterator p;
  //---- reset socks -------------------------------
  for (p = shok.begin(); p != shok.end(); p++)
  { CAcmPart *prt = (*p);
    prt->ResetKey(0);
  }
  //--- Set keyframe -------------------------------
  for (p = gears.begin(); p != gears.end(); p++)
  { CAcmPart *prt = (*p);
    float     ckf = prt->TimeKey(time);
    end +=  (fabs(tkf - ckf) < 0.001)?(1):(0);
  }
  //--- Update gear position for grlt --------------
  bool trm = (end == gears.size());       // Check for end moving
  if (trm) wPos = pos;                    // if stopped ensure position
  return (trm)?(1):(0);                   // return moving state
}
//--------------------------------------------------------------------
//  Set the altitude above ground.  Detect contact
//  Actually plane are rather floating above ground
//--------------------------------------------------------------------
void CAcmGears::SetAGL(float f)
{ wAGL = f;
  wGRN = (f < 4)?(1):(0);
  return;
}
//--------------------------------------------------------------------
//  Steering interface
//  NOTE:  The steering works like this
//        0   => Left
//        0.5 => neutral
//        1   => Right
//  But the invert tag inverse the process
//--------------------------------------------------------------------
void CAcmGears::TurnTo(float d)
{ float dir  = (Inverted())?(1.0f - d):(d);
  cDIR  = dir;
  std::vector<CAcmPart*>::iterator p;
  for (p = turn.begin(); p != turn.end(); p++)
  { CAcmPart *prt = (*p);
    prt->setKeyframe(dir);
  }
  return;
}
//--------------------------------------------------------------------
//  Steering additional force
//--------------------------------------------------------------------
void CAcmGears::PushTo(float d)
{ float str = (Inverted())?(1.0f - d):(d);
  float dir = ClampVal(float(cDIR + str),0,1);
  cDIR      = dir;
  std::vector<CAcmPart*>::iterator p;
  for (p = turn.begin(); p != turn.end(); p++)
  { CAcmPart *prt = (*p);
    prt->setKeyframe(dir);
  }
  return;
}
//--------------------------------------------------------------------
//  Set shock force
//  PB: Must retract shock when retracting the gears
//--------------------------------------------------------------------
void CAcmGears::SetShockTo(float f)
{ float key = fabs(f);
  //------Animate the suspension ----------------------------
  std::vector<CAcmPart*>::iterator p;
  for (p = shok.begin(); p != shok.end(); p++)
  { CAcmPart *prt = (*p);
    prt->setKeyframe(key);
  }
 
  return;
}
//==========================================================================
//  CAcmTire supports the rotating wheels
//==========================================================================
CAcmTire::CAcmTire(CVehicleObject *v,char *n)
{ if (0 == v)  gtfo("No Vehicle for Part Group");
  vlod = v->GetLOD();
  tnam  = n;
  vlod->AddTire(this);
  wKFS  = 0;
  wVEL  = 0;
}
//--------------------------------------------------------------------
//  Destructor
//--------------------------------------------------------------------
CAcmTire::~CAcmTire()
{ tire.clear();
} 
//--------------------------------------------------------------------
//  Add a Tire part
//--------------------------------------------------------------------
void  CAcmTire::AddTire(char *name)
{ if (0 == vlod)      return;
  CAcmPart *wp = vlod->GetPart(name);
  if (wp) tire.push_back(wp);
  return;
}
//--------------------------------------------------------------------
//  Set tire radius:  
//  For wheel animation we need to know
//  -The wheel perimeter Wp  in feet
//  -The linear speed V in feet/sec (or so I hope).
//  For a speed of Wp/sec, the keyframe move from 0 to 1
//  Thus  for a speed of 1 feet/sec the keyframe move by dF = 1/Wp
//        for a speed of v feet/sec the keyframe move by dF = v/Wp
//        during time dT = dF = v.dT / Wp
//  Knowing the radius R we compute 1/Wp (Keyframe increment for 1 Wp/sec)
//--------------------------------------------------------------------
void CAcmTire::SetRadius(float r)
{ float wp = 2 * PI * r;
  wKFS = 1 / wp;            // wp = 2PI*Radius
  return;
}
//--------------------------------------------------------------------
//  Wheel rotation:  
//  Add to keyframe the corresponding distance d = v * dT
//--------------------------------------------------------------------
void CAcmTire::SetWheelVelocity(float v,float dT)
{ float vf = MetresToFeet(v);
  wVEL += (wKFS * vf * dT);
  if (wVEL > 1) wVEL -= 1;              // stay in [0,1]
  return;
}
//--------------------------------------------------------------------
//  Move rotating wheel according to time and speed
//  tgf:  The target keyframe
//  dir:  The direction (0 extending, 1 retracting)
//  time: The global time in the scenario from T0
//--------------------------------------------------------------------
int CAcmTire::MovePart(float tkf,char dir,float dT,float time)
{ float key = wVEL;
  std::vector<CAcmPart*>::iterator p;
  for (p = tire.begin(); p != tire.end(); p++)
  { CAcmPart *prt = (*p);
    prt->setKeyframe(key);
  }
  return 1;
}
//==========================================================================
//  CAcmFlap supports the animated flaps 
//==========================================================================
CAcmFlap::CAcmFlap(CVehicleObject *mveh)
{ vlod = &mveh->lod;
  vlod->AddFlap(this);
}
//----------------------------------------------------------------
//  Delete this object
//----------------------------------------------------------------
CAcmFlap::~CAcmFlap()
{ flaps.clear();
}
//----------------------------------------------------------------
//  Add a part
//----------------------------------------------------------------
void CAcmFlap::AddFlap(char *name)
{ CAcmPart *prt = vlod->GetPart(name);
  flaps.push_back(prt);
  return;
}
//--------------------------------------------------------------------
//  Move Flaps parts to direction according to target
//  
//  NOTE: The time factor is converted into a frame index by
//        each part.
//        Function return 1 when all moving parts in gear list
//        has reached target position tkf.
//  tgf is the target keyframe
//  time is the global time in the scenario from T0
//--------------------------------------------------------------------
int CAcmFlap::MovePart(float tkf,char dir,float dT,float time)
{ U_INT   end = 0;
  float   dfk = dT * cFPS * dir;          // Delta kframe
  std::vector<CAcmPart*>::iterator p;
  //--- Set keyframe --------------------------------
  for (p = flaps.begin(); p != flaps.end(); p++)
  { CAcmPart *prt = (*p);
    float     ack = prt->ActualFrame();
    //---Enough room to additional keyframe
    if (fabs(tkf - ack) > fabs(dfk))
    { ack += dfk;
      prt->setKeyframe(ack);
    }
    //---This part is on target ---------------------
    else
    { prt->setKeyframe(tkf);
      end += 1;
    }

    //TRACE("Dir %d Key %.02f Part: %s",d,time,prt->GetPartName());
  }
  return (end == flaps.size())?(1):(0);
}
//=================================================================
//  New spin part
//=================================================================
CAcmSpin::CAcmSpin(CAcmPart *p)
{ part = p; 
  draw = 0;
	if (0 == p)		return;
	p->Relocate();
	p->SetType(PART_SPINR);
	return;
}
//----------------------------------------------------------------
//  Spin part
//  X => left side
//  Y => Upward
//  Z => Forward
//  The directions will be corrected into DrawInternalSpin()
//----------------------------------------------------------------
void CAcmSpin::SetPOS(SVector &v)
{ pos.x =  v.x;
  pos.y =  v.y;
  pos.z =  v.z;
  draw  =  1;
  return;
}
//----------------------------------------------------------------
//  Draw Spin part
//----------------------------------------------------------------
void CAcmSpin::Draw(CCameraCockpit *cam, TC_4DF &d)
{ if (0 == part)  return;
  if (0 == draw)  return;
  CVector trs = pos;
  //---Compensate the scroll displacement ------
  trs.x -= (d.x0 *d.x1);
  trs.y += (d.y0 * d.y1);
  part->DrawInternalSpin(trs);
  return;
}
//======================================================================
//  Class to pack vertex for ACM model
//======================================================================
CAcmVPack::CAcmVPack(int nbv,CAcmPart *p)
{	Part	= p;
	nbVT	= nbv;
	vtab	= new GN_VTAB[nbv];
}
//----------------------------------------------------------------
//	Constructor for collada
//----------------------------------------------------------------
CAcmVPack::CAcmVPack(CAcmPart *p,Xgeom *nod)
{	Part	= p;
	nbVT	= nod->nbv;
	vtab	= nod->vtab;
	nod->vtab	= 0;
}
//----------------------------------------------------------------
//	Destroy this object
//----------------------------------------------------------------
CAcmVPack::~CAcmVPack()
{	Part = 0;
	delete [] vtab;
}
//----------------------------------------------------------------
//  Add a new vertice
//----------------------------------------------------------------
void CAcmVPack::AddFaceVertex (int k, GN_VTAB &v)
{ if (k > nbVT)  return;
  memcpy (&vtab[k], &v, sizeof(GN_VTAB));
	return;
}
//----------------------------------------------------------------
//  Copy vertices and init part with geometry parameters
//----------------------------------------------------------------
int CAcmVPack::Copy(char *dst,int ofs)
{	char *src = (char *)vtab;
	memcpy(dst,src,(nbVT * sizeof(GN_VTAB)));
	Part->SetFirst(ofs);
	Part->SetCount(nbVT);
	return nbVT;
}
//=================END OF FILE ====================================================================