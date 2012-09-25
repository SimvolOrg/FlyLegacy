/*
 * AnimatedModel.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
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

#include "../Include/Globals.h"
#include "../Include/FlyLegacy.h"
#include "../Include/Utility.h"
#include "../Include/UserVehicles.h"
#include "../Include/Cameras.h"
#include "../Include/Collada.h"

using namespace std;

//=====================================================================================
// CAnimatedModel
//  This object manages the 3D models and all complexe animation (gear, flap, etc)
//======================================================================================
CAnimatedModel::CAnimatedModel ()
{ //----Model parameters ------------------------------
	mName					= 0;				// Model name
  mDefn         = 0;        // Current definition
  Model         = 0;        // Model ACM
  //---- Set Read tags --------------------------------
  dayTag        = 0;        // true when day models tag has been read
  nightTag      = 0;        // true when night models tag has been read
  dayModels     = 0;        // true when day models sub-object active
  nightModels   = 0;        // true when night models sub-object active
  hasSpin       = 0;
  //-------Init Flap parameters -----------------------
  nPos          = 0;
  aPos          = 0;
  s1Flap        = 0;              // Left sound
  s2Flap        = 0;              // Right sound
  //-------Init Gear parameters ------------------------
  cGear         = 0;
  sGear         = 0;              // gear sound 
  //-------Init spinners -------------------------------
  for (int k=0; k<ENGINE_MAX; k++) aSpin[k] = 0;
}
//----------------------------------------------------------------------
//  Read parameters
//----------------------------------------------------------------------
void	CAnimatedModel::Init(char *lodFilename,Tag t)
{	Type = t;
	//-------Read the parameters -------------------------
  SStream s(this,"WORLD",lodFilename) ;
	return;
}
//----------------------------------------------------------------------
//  Free resources
//----------------------------------------------------------------------
CAnimatedModel::~CAnimatedModel (void)
{ if (mName) delete [] mName;
	//------------------------------------------------------
  SAFE_DELETE (s1Flap);
  SAFE_DELETE (s2Flap);
  //-------------------------------------------------------
  SAFE_DELETE (Model);                                          // lc 052310 +
  SAFE_DELETE (sGear);                                          // lc 052310 +
  for (int k = 0; k < ENGINE_MAX; k++) SAFE_DELETE (aSpin [k]); // lc 052310 +
}
//----------------------------------------------------------------------
//  Read a comp model
//  Keep only the higghest definition in day model
//----------------------------------------------------------------------
int CAnimatedModel::ReadModel(SStream *st)
{ 
	//---------------------------------------------------------------------
	//	We isolate the file name, the type and the minimum pixel
	//	We are only keeping the day model in the highest resolution
	//---------------------------------------------------------------------
  char edt[8];
  //----Type comp of smf ------------
  ReadString (edt, 8, st);
  Tag type = StringToTag (edt);
  //--- File name -------------------
  char filename[64];
  ReadString (filename, 64, st);
	filename[63] = 0;
  char fullname[MAX_PATH];
	_snprintf(fullname,(MAX_PATH-1),"MODELS/%s",filename);
  //--- level of detail -------------
  unsigned int lod;
  ReadUInt (&lod, st);
	//---------------------------------------------------------------------
  if (0     == dayModels) return TAG_READ;   // Not a day model
  if (mDefn  > lod)       return TAG_READ;   // Lower Resolution
	//--- Keep the new candidate name -------------------------------------
	if (mName)	delete mName;
	mName  = Dupplicate(fullname,MAX_PATH);
	mDefn	 = lod;
  return TAG_READ;

}
//---------------------------------------------------------------------
//	Build an ACM model
//---------------------------------------------------------------------
void CAnimatedModel::BuildFromACM()
{	Model = new CModelACM (Top,Dpn,mveh);
  Model->LoadFrom(mName);
	return;
}
//---------------------------------------------------------------------
//	Build an ACM model
//---------------------------------------------------------------------
void CAnimatedModel::BuildFromDAE()
{	ColladaParser psr(mName,mveh);
	Model = psr.GetModel();
	return;
}
//---------------------------------------------------------------------
//	the model is now build from the model file
//---------------------------------------------------------------------
void CAnimatedModel::BuildModel()
{ // Model specification -----------------
  Top = 0;
  Dpn = 0;
	//---------------------------------------
	if (Type != 'sobj')
	{Top = PART_BODY;
   Dpn = PART_CHILD;
	}
		//--------------------------------------
	else 
	{ Top = PART_SOBJ;
    Dpn = PART_CHILD; 
  }

	//----Decode the format of 3D model ----
	strupper(mName);
	char *dot = strrchr(mName,'.');
	if (0 == dot)	gtfo("%s: not a valid model format",mName);
	dot++;
	if (strncmp(dot,"ACM",3) == 0)	BuildFromACM();
	if (strncmp(dot,"DAE",3) == 0)	BuildFromDAE();
  if (0 == Model) gtfo("No 3D model");
  Model->GetExtension(extS);
  Model->GetBodyExtension(extB);
  Model->GetMiniExtension(minB);
  return ;
}
//----------------------------------------------------------------------
//  Read a comp model
//  Keep only the higghest definition in day model
//----------------------------------------------------------------------
int  CAnimatedModel::WarnModel(Tag tag)
{ WARNINGLOG ("file LOD: Unrecognized tag <%s>", TagToString(tag));
  return TAG_IGNORED;
}

//----------------------------------------------------------------------
//  Read parameters 
//----------------------------------------------------------------------
int CAnimatedModel::Read (SStream *stream, Tag tag)
{ switch (tag) {
    case 'dlod':
      // Daytime models.  This tag introduces a sub-object containing the
      //   various level of detail models for the daytime exterior
      dayTag = 1;
      return TAG_READ;

    case 'nlod':
      // Nighttime models.  This tag introduces a sub-object containing the
      //   various level of detail models for the nighttime exterior
      nightTag = 1;
      return TAG_READ;

    case 'bgno':
      if (dayTag) {
        // We are expecting this tag to open the day models sub-object
        dayTag    = 0;
        dayModels = 1;
        return TAG_READ;
      } 
      if (nightTag) {
        // We are expecting this <bgno> to open the night models sub-object
        nightTag    = 0;
        nightModels = 1;
        return TAG_READ;
      }
      return WarnModel(tag);

    case 'endo':
      // Signals the end of either the day or night models subobject
      if (dayModels) {
        // This <endo> closes the day models sub-object
        dayModels = 0;
        return TAG_READ;
      }
      if (nightModels) {
        // This <endo> closes the night models sub-object
        nightModels = 0;
        return TAG_READ;
      }
      return WarnModel(tag);

    // mod2 : 
    //  modelTag (4-chars)
    //  modelName (filename.smf)
    //  pixelHeight (in Y screen pixels) to swap models
    case 'mod2':
      return ReadModel(stream);

    // modl : 
    //  modelTag (4-chars)
    //  modelName (filename.smf)
    //  no swap intended
    case 'modl':
      return TAG_IGNORED;

    // mdst : 
    //  modelTag (4-chars)
    //  modelName (filename.smf)
    //  minDist (minimum distance in feet from camera eye) to swap models
    //  maxDist (maximum distance in feet from camera eye) to swap models
    case 'mdst':
      return TAG_IGNORED;

    case 'shad':
      {
      // Low-detail shadow model Skip it --------
      char type[64];
      ReadString (type, 64, stream);
      Tag typeTag = StringToTag (type);

      char filename[64];
      ReadString (filename, 64, stream);
      return TAG_READ;
    }
  }

  return WarnModel(tag);
}
//---------------------------------------------------------------------
//  All parameters are read. Set type of parts
//  NOTE:  For now, only one model is supported.  More work
//        is needed to support several LODs:
//      -Set current model with distance and time of day
//      -Update model with all keyframe from the previous one
//  
//---------------------------------------------------------------------
void CAnimatedModel::ReadFinished()
{ BuildModel();
  return;
}
//---------------------------------------------------------------------
//  Add a Tire and init sound
//  JS NOTE:  Tires are created and entered in chronological order from 
//            the WHL file
//---------------------------------------------------------------------
void CAnimatedModel::AddTire(CAcmTire *w)
{ aTire.AddPart(w);
  return;
}
//---------------------------------------------------------------------
//  Reset Landing gear
//---------------------------------------------------------------------
void CAnimatedModel::Reset()
{ aGear.Reset();
  aFlap.Reset();
  return;
}
//---------------------------------------------------------------------
//  Activate landing gear
//  - activate landing gear according to current status
//  - return position
//---------------------------------------------------------------------
void  CAnimatedModel::SetLandingTo(int p)
{ char  lock;
  //-- init global temporary trace -----------------
  switch (aGear.GetState()) {
  //---Gear are down -------------------------------
  case ITEM_KFR0:
    if (0 == p)                   return ;    // Already down
    lock = cGear->Lock();
    if (WheelOnGround() && lock)  return;     // Ground lock
    //--Configure for moving up ---------------------
    aGear.StartMove();
    aGear.Forward(1,ITEM_KFR1);
    sGear->Play(1);
    return;

  case ITEM_KFR1:
    if (1 == p)                   return ;    // Already up
    //--configure to move down ---------------------
    aGear.Backward(0,ITEM_KFR0);
    sGear->Play(0);
    return;
    //---Juste change direction  if we are moving ---
  case ITEM_MOVE:
    aGear.Reverse();
    return ;
  }
  return;
}
//---------------------------------------------------------------------
//  Add a wheel and init sound
//  JS NOTE:  Wheels are created and entered in chronological order from 
//            the WHL file
//            The grlt susbsystem (gear light) references gears in that
//            same order
//---------------------------------------------------------------------
void CAnimatedModel::AddGear(CAcmGears *w)
{ aGear.AddPart(w);
  if (sGear)    return;
  //---Init landing gear sound --------------
  CVector pos(0,0,0);
  Tag ids = 'gear';
  sGear = new CSoundOBJ(ids,pos);
  sGear->AddSound(0,'gerd');
  sGear->AddSound(1,'geru');
  return;
}
//---------------------------------------------------------------------
//  Check for wheel on ground (at least one)
//---------------------------------------------------------------------
char CAnimatedModel::WheelOnGround()
{ return aGear.GroundState();}
//---------------------------------------------------------------------
//  Check for wheel on ground (at least one)
//---------------------------------------------------------------------
char CAnimatedModel::WheelPosition(char w)
{ CAcmGears *whl = (CAcmGears *)aGear.GetPart(w);
  return (whl)?(whl->GetPosition()):(0);
}
//---------------------------------------------------------------------
//  Store Flap positions
//  NOTE:  for each position there is a corresponding keyframe
//  and  the full range (0, 1) needs T seconds.
//---------------------------------------------------------------------
void CAnimatedModel::InitFlap(int n,FLAP_POS *p,float time)
{ nPos  = n;
  aPos  = p;
  cPos  = 0;
  int   dg0 = p[0].degre;
  int   dtv = p[n-1].degre - dg0;
  float dkf = float(1) / dtv;
  //----Compute positional keyframe in table ------------
  for (int k=0; k != n; k++)
  {float kf     = float(p[k].degre - dg0) * dkf;
   p[k].kFrame  = kf;
  }
  //---Propagate the time constant to all flaps ---------
  aFlap.SetTime(time);
  //---Init sounds --------------------------------------
  CAudioManager *snd = globals->snd;
  Tag     id1 = '1flp';
  CVector lef(-10,0,0);
  s1Flap = new CSoundOBJ(id1,lef);
  s1Flap->SetGain(0.4f);
  Tag     id2 = '2flp';
  CVector rit(+10,0,0);
  //MEMORY_LEAK_MARKER ("s2Flap");
  s2Flap = new CSoundOBJ(id2,rit);
  //MEMORY_LEAK_MARKER ("s2Flap");
  s2Flap->SetGain(0.4f);
  //---Init flap sounds --------------------------------
  for (int k=1; k < n; k++)
  { Tag snn = 'Flp0' + k;
    CSoundBUF *sb1 = snd->GetSoundBUF(snn);
    if (sb1) s1Flap->SetSBUF(k,sb1);
    CSoundBUF *sb2 = snd->GetSoundBUF(snn);
    if (sb2) s2Flap->SetSBUF(k,sb2);
  }
  //---Dupplicate sound up to max -----------------------
  s1Flap->FillFlaps();
  s2Flap->FillFlaps();
  return;
}
//---------------------------------------------------------------------
//  Move Forward
//  p = flap position
//  e = Ending state
//---------------------------------------------------------------------
int CAnimatedModel::FlapForward(char p,char e)
{ float tkf = aPos[p].kFrame;     // Target keyframe
  aFlap.Forward(tkf,e);           // Forward to target
  cPos  = p;                      // Current position
  s1Flap->Play(cPos);
  s2Flap->Play(cPos);
  return p;
}
//---------------------------------------------------------------------
//  Move Bacward
//---------------------------------------------------------------------
int CAnimatedModel::FlapBackward(char p,char e)
{ float tkf = aPos[p].kFrame;     // Target keyframe
  aFlap.Backward(tkf,e);          // Backward to target
  aFlap.EndState(e);              // Set ending state (must be after backward)
  s1Flap->Play(cPos);
  s2Flap->Play(cPos);
  cPos  = p;                      // Current position
  return p;
}
//---------------------------------------------------------------------
//  Change target and end state only. Let flap move
//---------------------------------------------------------------------
int CAnimatedModel::FlapChange(char p,char end)
{ float tkf = aPos[p].kFrame;
  aFlap.ChangeTo(tkf,end);
  cPos = p;
  return p;
}
//---------------------------------------------------------------------
//  Invert motion to new target and new end
//---------------------------------------------------------------------
int CAnimatedModel::FlapReverse(char p,char end)
{ float tkf = aPos[p].kFrame;
  aFlap.Reverse(tkf,end);
  cPos = p;
  return p;
}
//---------------------------------------------------------------------
//  Start toward flap position
//  return the accepted position
//---------------------------------------------------------------------
int CAnimatedModel::SetFlapPosition(int d)
{ char last = nPos - 1;
  char end  = ITEM_BETW;
  //---Normalize flap position ------------------
  if (0 == aPos)              return cPos;   // Ignore no table
  if (d <= 0)      {d = 0;    end = ITEM_KFR0;}
  if (d >= last)   {d = last; end = ITEM_KFR1;}
  if (cPos == d)              return cPos;
  //---Process according to flap state ----------
  switch (aFlap.GetState()) {
    //---Flaps are retracted --------------------
    case ITEM_KFR0:
       return FlapForward(d,end);
    //---Flaps are all down ---------------------
    case ITEM_KFR1:
       return FlapBackward(d,end);
    //---Flaps are moving -----------------------
    case ITEM_MOVE:
       if ((d > cPos) && (aFlap.Motion() > 0)) return FlapChange(d,end);
       if ((d < cPos) && (aFlap.Motion() < 0)) return FlapChange(d,end);
       return  FlapReverse(d,end);
    //---Flaps are in between -------------------
    case ITEM_BETW:
      if (d > cPos)   return FlapForward (d,end);
      if (d < cPos)   return FlapBackward(d,end);
      return cPos;
}
  return cPos;
}
//---------------------------------------------------------------------
//  Add a spinner for engine en
//---------------------------------------------------------------------
CAcmSpin *CAnimatedModel::AddSpinner(char en,char *pn)
{ CAcmPart *part = Model->GetPart(pn);
  CAcmSpin *spin = new CAcmSpin(part);
  char iNum   = en - 1;
  aSpin[iNum] = spin;
  return spin;
}
//---------------------------------------------------------------------
//  Draw all spinners
//---------------------------------------------------------------------
void  CAnimatedModel::DrawSpinner(CCameraCockpit *cam, TC_4DF &d)
{ if (0 == aSpin[0])     return;
  
  glMatrixMode (GL_MODELVIEW);
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
  glEnable(GL_TEXTURE_2D);
  //----A voir blend ----------------------------------
  glEnable(GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //---------------------------------------------------
  glDepthMask(GL_FALSE);
  glPushMatrix();
  SVector ofs;
  cam->GetOffset(ofs);
  glTranslated(ofs.x,ofs.y,0);     //ofs.z);
  Model->GetVEH()->Rotate();
  //---------------------------------------------
  for (int k= 0; k < ENGINE_MAX; k++)
  { CAcmSpin *spin = aSpin[k];
    if (0 == spin)  break;
    //----Draw spinner --------------------------
    spin->Draw(cam,d);
  }
  //---------------------------------------------
  glPopMatrix();
  glPopAttrib();
  glPopClientAttrib();
  return;
}
//---------------------------------------------------------------------
//  Set Keyframe to each part
//---------------------------------------------------------------------
void CAnimatedModel::SetPartKeyframe (char *partname, float frame)
{ Model->SetPartKeyframe (partname, frame);
}
//----------------------------------------------------------------------
//  Time slice the current model
//----------------------------------------------------------------------
void CAnimatedModel::TimeSlice(float dT)
{ if (WheelOnGround())                    aTire.Animate(dT);
  if (aGear.Moving() && cGear->Active())  aGear.Animate(dT);
  if (aFlap.Moving())                     aFlap.Animate(dT);
  //---Stop any flap move --------------------------------
  if (aFlap.Stops()) 
  { s1Flap->StopSound();
    s2Flap->StopSound();
  }
  return;
}
//------------------------------------------------------------------------
void CAnimatedModel::Print (FILE *f)
{ /*
  fprintf (f, "Day Models\n");
  fprintf (f, "==========\n");
  Model->Print (f);
	*/
}

//======================END OF FILE ==============================================
