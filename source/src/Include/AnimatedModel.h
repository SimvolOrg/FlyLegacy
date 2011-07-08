/*
 * AnimatedModel.h
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
#ifndef ANIMATEDMODEL_H_
#define ANIMATEDMODEL_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Include/FlyLegacy.h"
#include "../Include/Utility.h"
//===============================================================================
// CAnimatedModel represents an external model that has a number of varyingly-detailed
//   models for the same object.  High-detail models are typically displayed when
//   the object is close up, and lower-detail models when the object is farther away.
//  JS NOTE:  Supporting several level of detail requires additional work.  To be
//            efficient we must:
//    Set a current model depending on distance and time of day
//    Maintain a separate part state:  For dynamic parts, keyframe and other
//            dynamic attributes should be updated in real time and when model
//            is changed, the properties are thus immediatly available
//
//===============================================================================
class CAnimatedModel : public CStreamObject {
  //--- ATTRIBUTES ----------------------------------------
  Tag   Type;                               // Vehicle type
  CVehicleObject          *mveh;            // Parent vehicle
	char *mName;															// Model name
	//--- Model specifications ------------------------------
	char	Top;																// Top part type
	char  Dpn;																// Dependent part type
  //-------------------------------------------------------
  CVector                  extS;            // Day model spacial expension
  CVector                  extB;            // Body extension
  CVector                  minB;            // Body minimum
  CModelACM               *Model;           // Current model
  U_INT                    mDefn;           // Current definition
  std::vector<CModelACM*>  AcmDAY;          // Daytime models
  //---Reading flags  --------------------------------------
  char  dayTag;         // true when day models tag has been read
  char  nightTag ;      // true when night models tag has been read
  char  dayModels;      // true when day models sub-object active
  char  nightModels;    // true when night models sub-object active
  char  hasSpin;        // true when spinner part are present
  //---Wheel control ---------------------------------------
  CAnimator     aTire;              // Tire control
  //---Landing gear control --------------------------------
  CGearControl *cGear;              // Gear control
  CAnimator     aGear;              // Landing gear animator
  CSoundOBJ    *sGear;              // gear Sounds
  //---flap system    ----------------------------------------
  int        nPos;                  // Number of positions
  int        cPos;                  // Current position
  FLAP_POS  *aPos;                  // Array of positions
  CAnimator  aFlap;                 // Flap animator
  CSoundOBJ *s1Flap;                // Left  flap sounds
  CSoundOBJ *s2Flap;                // Right flap sounds
  //---Prop systems for 8 engines     ----------------------
  CAcmSpin *aSpin[ENGINE_MAX];      // Animated spinners
  //--- METHODS ----------------------------------------
public:
  // Constructors / Destructor
  CAnimatedModel (CVehicleObject *v,char* lodFilename,Tag type);
 ~CAnimatedModel (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  int   ReadModel(SStream *st);
  int   WarnModel(Tag t);
  void  ReadFinished();
	void	BuildModel();
	void	BuildFromACM();
	void	BuildFromDAE();
  //--- CAnimatedModel methods -------------------------------
  void  Reset();                   // Reset to keyframe
  void  SetPartKeyframe    (char *partname, float frame);
  void  SetPartTransparent (char *partname, bool ok);
  //---Manage rotating wheels --------------------------------
  void  AddTire(CAcmTire *w);
  //---Manage landing gear -----------------------------------
  void  AddGear(CAcmGears *w);
  void  SetLandingTo(int p);              // Change landing position
  char  WheelOnGround();                  // Check for wheel on ground
  char  WheelPosition(char w);            // Get wheel position
  //----------------------------------------------------------
  inline bool AreGearDown()       {return aGear.AtKFR0();}
  inline bool AreGearRetracted()  {return aGear.AtKFR1();}
  inline void SetGearControl(CGearControl *c) {cGear = c;}
  inline void Draw(char mode)     {Model->Draw(mode);}
  //----Flap interface  --------------------------------------
  void      InitFlap(int n,FLAP_POS *p,float time);
  int       SetFlapPosition(int d);       // Set flap to direction
  int       FlapForward (char p, char end);
  int       FlapBackward(char p, char end);
  int       FlapChange  (char p, char end);
  int       FlapReverse (char p, char end);
  //----Prop interface ---------------------------------------
  CAcmSpin *AddSpinner(char en,char *pn);
  void      DrawSpinner(CCameraCockpit *cam, TC_4DF &d);
  //--- Time slice the model ---------------------------------
  void    TimeSlice(float dT);
  //---Printing -----------------------------------------------
  void    Print (FILE *f);
  //---Helpers -----------------------------------------------
  void    CheckKeys()   {Model->CheckKeys();}
  //---MODEL INTERFACE ---------------------------------------
  inline  void       GetExtension(SVector &v)     {v = extS;}
  inline  void       GetBodyExtension(SVector &v) {v = extB;}
  inline  void       GetMiniExtension(SVector &v) {v = minB;}
  inline  CAcmPart  *GetPart(char *n)         {return Model->GetPart(n);}
  inline  CModelACM *GetDayModel()            {return Model;}
  //---inline for landing gear -------------------------------
  inline  void       SetLandTime(float t)     {aGear.MaxTime(t);}
  //----Inline for flaps --------------------------------------
  inline  void       AddFlap(CAcmFlap *f)      {aFlap.AddPart(f);}
};


//==========================END OF FILE ===========================================
#endif // ANIMATEDMODEL_H_
