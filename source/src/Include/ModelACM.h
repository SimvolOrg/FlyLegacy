/*
 * ModelACM.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2000-2004 Chris Wallace
 * Copyright 2007-.... Jean Sabatier
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


#ifndef MODELACM_H
#define MODELACM_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdarg.h>
#include <stdio.h>
//=============================================================================
typedef enum {
  PART_BODY     = 0,
  PART_CHILD    = 1,
  PART_SPINR    = 2,
  PART_SOBJ     = 3,
} EPartType;
//=============================================================================
//=============================================================================
typedef enum {
  BODY_NOMOVE    = 0,
  BODY_TRANSFORM = 1,
}EDrawMode;
//=============================================================================

class CAnimatedModel;
class CModelACM;
class Xgeom;
class CShared3DTex;
//=============================================================================
//  CGizmo:  Display a showing symbol
//=============================================================================
class CGizmo: public CDrawByCamera {
  //----Attribute ----------------------------------------
  U_CHAR  face;               // Tank face to draw
  CVector pos;                // World position
  GLUquadricObj *obj;         // Quadric
  //----Methods ------------------------------------------
public:
  CGizmo();
 ~CGizmo();
  //-------------------------------------------------------
  inline void SetPosition(SVector &v)  {pos  = v;}
  inline void SetFace(U_CHAR f)        {face = f;}
  //----Drawing by object camera ----------------------------------------------
  void  PreDraw();
  //-------------------------------------------------------
  void  PreCros();
  void  DrawCros();
  void  EndCros();
  //-------------------------------------------------------
  void  PreFuel();
  void  DrawTank(U_CHAR No);
  void  EndFuel();
  //-------------------------------------------------------
  void  PreLoad();
  void  DrawLoad();
  void  EndLoad();
};
//============================================================================================
// CModelACM
//
// Implemented in Utility/ModelACM.cpp
//============================================================================================
class CKeyFrame {
public:
  float     frame;
  char      indP;               // Interpolation indicator for P (1 for fixed frame)
  char      indB;               // Interpolation indicator for B (1 for fixed frame)
  char      indH;               // Interpolation indicator for H (1 for fixed frame)
  char      render;             // Rendering
  float     dx, dy, dz, p, b, h;
  //----------------------------------------
  CKeyFrame();
  void  Trace();
} ;
//============================================================================================
class CAcmPart {
	friend class CModelACM;
public:
  CAcmPart (CModelACM *md, int n, float min = 0, float max = 1);
  virtual ~CAcmPart (void);

  // CModelAcmPart methods
  void          setName (const char* s);
  void          setTransparent();
  CKeyFrame    *addKeyframe (CKeyFrame kf);
  CKeyFrame    *GetCopyKey();
  void          setKeyframe (float frame);
  void          ModifyKeyframe(float dF);
  void          AllocateKeyframe(int n);
  void          IncKeyframe(float dF);
  void          RotateKeyframe(float dF);
  float         TimeKey(float time);
  void          TracePart();
  void          ResetKey(int k);
  void          Interpolate(CKeyFrame *prv,CKeyFrame *aft);
  void          addChild (CAcmPart* child);
  void          SetExtension(GN_VTAB &v);
  void          CopyRotation(int k,CKeyFrame &kf);
  void          Print (FILE *f);
  void          SortChildTransparency (void);
  //-----------------------------------------------------------------
  char*         GetPartName (void) {return name;}
  void          Relocate();
  void          Relocate(CAcmPart *p);
  //-----------------------------------------------------------------
  void          SetDiffuse(bool d);
  void          SetSpecular (bool sp, float pwr);
  void          Center();
  //-----------------------------------------------------------------
  void          Draw (char mode);
  void          DrawAsShadow(char mode);
  void          DrawShadow (char mode); // test
  void          DrawInternalSpin(SVector &v);
  void          DrawAsSpinr(char mode,CKeyFrame &k);
  void          DrawFastProp(CKeyFrame &kf);
  void          DrawFixeProp(CKeyFrame &kf);
  void          DrawSlowProp(CKeyFrame &kf);
  void          DrawAsBlade();
  double        GetRadius();
  void          SetRPM(float rpm);
  //---Store texture Object -----------------------------------------
  inline  CKeyFrame *GetKeyF0()          {return kFrame;}
  inline        void SetTexREF(void *r,U_INT o)    {txRef = r;txOBJ = o;}
  inline        void SetAlpha(float a)      {alpha  = a;}
  inline        void SetEnvMapped (bool m)  {envmap = m;}
  inline        void SetTransparent(char t) {Trans  = t;}
  //------------------------------------------------------------------
  inline        void PushKF(CKeyFrame &t){t = transform;}
  inline        void PopKF (CKeyFrame &t){transform = t;}
  inline        void SetType(char t)        {Type = t;}
  inline        void SetFPS(float f)        {fps  = f;}
  inline        float ActualFrame()         {return transform.frame;}
  //-----------------------------------------------------------------
  inline        void      RenderAs(U_INT m)		  {Render = m;}
  inline        void      SetURPM(char p)       {uprm   = p;}
  inline        void      SetParent(CAcmPart *m){mother = m;}
  inline        void      SetIndex(int No)      {index  = No;}
  inline        CAcmPart *GetParent()           {return mother;}
  inline        bool      IsGlasse()            {return (0 != glasse);}
  inline        bool      IsTransparent()       {return (0 != Trans);}
  //-----------------------------------------------------------------
  inline    SVector &minEXT()               {return minP;}
  inline    SVector &maxEXT()               {return maxP;}
  //--Helper --------------------------------------------------------
  void          TraceKey();
  void          CheckKeys();
  void          SetAndTrace(float k);
  void          CopyTranslation(SVector &t);
  void          CopyRotation(SVector &r);
  void          AddArray(int nb, GN_VTAB *tab);
	//--- Store geometry parameters -----------------------------------
	inline void		SetFirst(int n)		{First = n;}
	inline void   SetCount(int n)		{Count = n;}
  //-----------------------------------------------------------------
  inline void   SetPitch(float p) {transform.p = p;}
  inline void   SetBank (float b) {transform.b = b;}
  inline void   SetHead (float h) {transform.h = h;}
  //-----------------------------------------------------------------
protected:
	//--- Geometry ----------------------------------------------------
	int						First;					// Indice of first vertice
	int						Count;					// Number of primitives
  //-----------------------------------------------------------------
  char          Type;           // Top,child, other
  char          Trans;          // Transparency
  char          envmap;         // Use environment mapping for this part
  char          glasse;         // Glasse part
	//-----------------------------------------------------------------
	U_INT					Render;					// Render mode
  //-----------------------------------------------------------------
  short         index;          // Child number
  //-----------------------------------------------------------------
  CModelACM    *model;          // Mother model
  //-----------------------------------------------------------------
  float         uprm;           // User parameter
  char          name[64];       // Part name
  CAcmPart     *mother;         // Parent part
  int           nFrames;        // Number of keyframe animation frames
  int           next;           // Next keyframe to be added
  CKeyFrame    *kFrame;         // Keyframe animation frames
  void         *txRef;          // Texture reference
  U_INT         txOBJ;          // Texture object
  //----  Spatial properties ----------------------------------------
  SVector       minP;           // Minimum coordinates
  SVector       maxP;           // Maximum corrdinates
  //----  color properties ------------------------------------------
  float        *vDIF;           // Diffuse vector
  float        *vSPC;           // Specular vector
  float         power;          // Specular lighting power
  float         alpha;          // For transparency
  //------------------------------------------------------------------
  float         fps;            // Frame per second for animated frame
  CKeyFrame  transform;      // Current part animation transform data
  std::vector<CAcmPart*> childList;    // List of child parts
};
//=========================================================================================
//  PROPERTIES
//=========================================================================================
typedef enum {
  GEAR_INVERTED  = 0x01,
  PROP_INVERTED  = 0x01,
  BLAD_INVERTED  = 0x02,
  PART_DEFINED   = 0x80,
} EGearProp;
typedef enum {
  GEAR_FRONT = 0x00,
  GEAR_LEFT  = 0x01,
  GEAR_RIGHT = 0x02,
} EGearType;
//=========================================================================================
//  Mother class for animated part
//=========================================================================================
class CAniPart {
protected:
  //---Attributes ---------------------------------------
  CAnimatedModel *vlod;           // Level of detail
  U_INT prop;                     // Property (turn, retract, etc)
  char       prm1;                // user parameter
  char       name[64];            // Part name
  //---Methods ------------------------------------------
public:
  CAniPart();
  //-----------------------------------------------------
  void  SetName(char *n) {strncpy(name,n,63); name[63] = 0;}
  //---Move parts ---------------------------------------
  virtual void  StartMove()     {}
  virtual int   MovePart(float tg,char dir,float dT,float tm) {return 0;}
  virtual void  SetFPS(float k) {}
  virtual void  ResetKey()      {}
  virtual char  GetState()      {return 0;} 
  virtual char  GroundState()   {return 0;}
  //-----------------------------------------------------
  inline void InvertSteering()  {prop |= GEAR_INVERTED;}
  inline void InvertProp()      {prop |= PROP_INVERTED;}
  inline void InvertBlad()      {prop |= BLAD_INVERTED;}
};
//=========================================================================================
//  Class Animated gear
//  Animated gear is a group of animated parts that need to be synchronized for
//  the landing gear.
//  NOTE:   fAGL should be updated from external module computing whatever wheel physic
//=========================================================================================
class CAcmGears: public CAniPart {
protected:
  //----ATTRIBUTES ---------------------------------------------------
  char                   wPos;                // Wheel position
  char                   type;                // Type of wheel 
  char                  *gnam;                // gear name
  std::vector<CAcmPart*> gears;               // List of gear parts
  std::vector<CAcmPart*> shok;                // List of shock parts 
  std::vector<CAcmPart*> turn;                // steering part
  float                  time;                // Overall timer
  float                  wKFS;                // wheel Frame per sec
  float                  wAGL;                // Altitude above ground 
  char                   wGRN;                // On ground indicator
  char                   WoW;                 // Weight indicator 
  //--- Tire Sound -----------------------------------------------------
  CSoundOBJ             *sTire;               // Tire sound
  //--- For steering wheel ---------------------------------------------
  float                  cDIR;                // Current direction (steer wheel)
  //----METHODS ------------------------------------------------------
public:
  CAcmGears(CVehicleObject *v,char *n);
 ~CAcmGears();
  void      AddGear(char *name);              // Add a gear  part
  void      AddShok(char *name);              // Add a shock part
  void      AddTurn(char *name);              // Add the turn part
  void      AddTime(float t); 
  void      AddSound(SVector &pos);           // Add tire sound
  void      ResetKey();                       // Reset Frame key
  void      PlayTire(int p);                  // Play tire sound
  //------------------------------------------------------------------
  void      StartMove();
  //------------------------------------------------------------------
  float     ClampVal(float v,float a, float b);
  //------------------------------------------------------------------
  inline void SetFPS(float k)           {wKFS = k;}
  inline void SetProperty(U_INT p)      {prop |= p;}
  inline bool Inverted()                {return (prop & GEAR_INVERTED);}
  inline char GroundState()             {return wGRN;}
  inline char GetPosition()             {return wPos;}
  //------Animator interface    --------------------------------------
  int       MovePart(float tg,char dir,float dt,float tm);   // Move landing gear
  //------Steering interface -----------------------------------------
  void      TurnTo(float f);                  // Wheel interface
  void      PushTo(float s);                  // additional force
  //------Shock interface --------------------------------------------
  void      SetShockTo(float f);              // Set shock damper
  //------------------------------------------------------------------
  void      SetAGL(float f);
};
//=========================================================================================
//  Class Animated tire
//  Animated tire is for the rolling wheel on ground
//=========================================================================================
class CAcmTire: public CAniPart {
  //---ATTRIBUTES ---------------------------------------------------
  std::vector<CAcmPart*> tire;                // Tire part
  char                  *tnam;                // Tire name
  float                  wKFS;                // wheel Frame per sec
  float                  wVEL;                // Wheel velocity
  //----METHODS ------------------------------------------------------
public:
  CAcmTire(CVehicleObject *v,char *n);
 ~CAcmTire();
  //------------------------------------------------------------------
  void      AddTire(char *name);              // Add a Tire
  void      SetRadius(float r);
  void      SetWheelVelocity(float v, float dT);
  //------------------------------------------------------------------
  int       MovePart(float tkf,char dir,float dT,float time);
};
//=========================================================================================
//  This class define animated 3D FLAP model
//=========================================================================================
class CAcmFlap: public CAniPart  {
  //---- ATTRIBUTES --------------------------------------------------
  std::vector<CAcmPart*> flaps;               // List of parts
  float  cFPS;                                // Frame per second
  //-----METHODS -----------------------------------------------------
public:
  CAcmFlap(CVehicleObject *v);
 ~CAcmFlap();
 //-------------------------------------------------------------------
 void AddFlap(char *name);
 //------------------------------------------------------------------
 inline void SetFPS(float fps)   {cFPS = fps;}
 //----Animator interface  ------------------------------------------
 int  MovePart(float tkf,char dir,float dt,float time);
};
//=========================================================================================
//  This class define specific spinner part
//=========================================================================================
class CAcmSpin {
  //--- Attributes --------------------------------------------------
  CAcmPart *part;                             // Spinner
  CVector   pos;                              // Position
  TC_2DF    ofs;                              // Offset
  char      draw;                             // Draw indicator
  //-----------------------------------------------------------------
public:
  CAcmSpin(CAcmPart *p);
  void      SetPOS(SVector &v);
  void      Draw(CCameraCockpit *cam, TC_4DF &d);
  //-----------------------------------------------------------------
  inline void SetRPM(float rpm)     {if (part) part->SetRPM(rpm);}
};
//===============================================================================
//  Class to help vertex packaging in ACM model
//===============================================================================
class CAcmVPack {
protected:
	//--- ATTRIBUTES ----------------------------------
	CAcmPart	*Part;										// Part pointer
	GN_VTAB	  *vtab;										// Vertex table
	int				 nbVT;										// Number of vertex
	//--- METHODS -------------------------------------
public:
	CAcmVPack(int nbv, CAcmPart *p);
	CAcmVPack(CAcmPart *p,Xgeom *nod);
 ~CAcmVPack();
	//-------------------------------------------------
	void	AddFaceVertex (int k, GN_VTAB &v);
	//-------------------------------------------------
	int   Copy(char *dst,int ofs);			// Copy vertices
	//-------------------------------------------------
	inline CAcmPart *GetPart()	{return Part;}
};
//=========================================================================================
//  This class define animated 3D models parsed from ACM file
//=========================================================================================
class CModelACM {
	//--- Define configuration vector -----------------------------
	typedef void (CModelACM::*VCONF)(char n);	// Config vector
public:
	//--- ATTRIBUTES ----------------------------------------------
  CModelACM(char t, char d, CVehicleObject *mv);
 ~CModelACM ();
  void  LoadFrom(char *fn);
  void  AddPart(short No,int index,CAcmPart *part);
  //-- CModelACM methods ----------------------------------------
  void  Draw (char mode);
  void  Print (FILE *f);
  void  SetPartKeyframe    (char* partname, float frame);
  void  SetPartTransparent (char* partname, bool ok);
  void  SaveExtension(char *name,GN_VTAB &tab);
  void  GetExtension(SVector &v);
  void  GetMiniExtension(SVector &v)  {v = minB;}
  void  GetBodyExtension(SVector &v);
  //U_INT GetTextureRef(char *fn,char opt);
	//--- Configure rendering -------------------------------------
	void  ConfMEM(char opt);			// Memory configurartion
	void	ConfVBO(char opt);			// VBO configuration
	//--- Helpers -------------------------------------------------
  void  CheckKeys();
  char  CheckFixedFrame(float prv, float nxv);
  void  CheckGap(CKeyFrame *prv, CKeyFrame *kfn);
	//--- Shadow management ---------------------------------------
	bool  BuildShadowMatrix();
	void	BuildVBO(int nbv,std::vector<CAcmVPack*> &pack);
  //-------------------------------------------------------------
  CAcmPart *GetPart(char *name);
  //----Drawing by object camera --------------------------------
  void  PreDraw();
  void  CamDraw(CCamera *cam);
  void  EndDraw();
  //------------------------------------------------------------
  inline   CAcmPart *GetBody() {return topParent;}
	inline void			RenderMEM()				{Rend = &CModelACM::ConfMEM;}
	inline void			RenderVBO()				{Rend = &CModelACM::ConfVBO;}
	inline void			Configure(char n)	{(this->*Rend)(n);}
	//--- Bind requested texture if needed -----------------------
	inline void BindTxObject(U_INT xob)
	{	if (cOBJ == xob)	return;
		glBindTexture (GL_TEXTURE_2D, xob);
		cOBJ	= xob;
		return;	}
  //---------------------------------------------------------------------------
protected:
	//--- ATTRIBUTES -------------------------------------------------------------
  Tag   Type;                                     // Type of model
  std::map<std::string,CAcmPart*> partMap;        // Map part names to CAcmPart
  std::map<int, CAcmPart*>        mapParent;      // Map part indices to CAcmPart
  CAcmPart                       *topParent;      // Top-level 'body' part
	//--- VBO parameters ---------------------------------------------------------
	char     vbu;																		// Use VBO
	int			 nbVT;																	// Total vertices
	GN_VTAB *vBUF;																	// Vertex Buffer
	U_INT		 oVBO;																	// Buffer object
	U_INT		 cOBJ;																	// Current Texture object
	VCONF		 Rend;																	// Configuration vector
	//--- Texture parameters -----------------------------------------------------
	CShared3DTex *tRef;															// Texture reference
	//--- Shadows parameters -----------------------------------------------------
  float	sMatrix[16];														  // Shadow matrix
  //---Mother vehicule ---------------------------------------------------------
  CVehicleObject *mveh;
  //---Model spacial extension--------------------------------------------------
  CVector minS;       // Minimum coordinate
  CVector maxS;       // Maximum coordinate
  //----Body spatial extension -------------------------------------------------
  CVector minB;       // Minimum coordinate
  CVector maxB;       // Maximum coordinate
  //---Type of parts -----------------------------------------------------------
  char    tr;                                     // Trace indicator
  char    Top;                                    // head part
  char    Dpn;                                    // Dependent
  char    rfu1;                                   // reserved
  //----------------------------------------------------------------------------
public:
  inline void			SetType(char t)     {topParent->SetType(t);}
  inline CVehicleObject *GetVEH()			{return mveh;}
	inline float*	  GetShadowMatrix()		{return sMatrix;}
};
//===============================================================================
//  Landing gear part moving status
//===============================================================================
typedef enum {
  ITEM_NONE  = 4,                  // Not used      
  ITEM_KFR0  = 1,                  // At Keyframe 0
  ITEM_MOVE  = 2,                  // Moving
  ITEM_KFR1  = 3,                  // At keyframe 1
  ITEM_BETW  = 4,                  // Anywhere in between
} EPartState;
//===============================================================================
//  Animation stucture
//  Helper for complex animating scenario such as the gear landing, flaps, etc
//  involving several parts that must move in synchro.
//===============================================================================
class CAnimator {
  char  tr;                               // Trace indicator
  char  used;                             // Used or not
  char  uprm1;                            // User parameter 1
  char  aStat;                            // Status
  char  eStat;                            // Ending state
  char  eStop;                            // Stop Event
  char  aMove;                            // Move direction
  float aTime;                            // Current time
  float aTmax;                            // Mximum animation time
  float aTarg;                            // Keyframe target
  //--- A list of animated objects -------------------------------
  std::vector<CAniPart*> obj;             // Animated object
  //----methods --------------------------------------------------
public:
  CAnimator();
 ~CAnimator();
  void    Reset();
  void    StartMove();
  void    Forward(float tg, char end);
  void    Backward(float tg,char end);
  void    Reverse();
  void    Reverse(float tg,char end);
  void    ChangeTo(float tg,char end);
  void    Animate(float dT);
  bool    Stops();
  void    SetTime(float time);
  char    GroundState();
  void    AddPart(CAniPart *p);
  //---------------------------------------------------------------
  inline  bool   Used()               {return (used != 0);}
  inline  void   SetUserP1(char p)    {uprm1 = p;}
  inline  char   GetUserP1()          {return uprm1;}
  //---------------------------------------------------------------
  inline  bool   AtKFR0()    {return (aStat == ITEM_KFR0);}
  inline  bool   Moving()    {return (aStat == ITEM_MOVE);}
  inline  bool   AtKFR1()    {return (aStat == ITEM_KFR1);}
  inline  bool   Stable()    {return (aStat != ITEM_MOVE);}
  //---------------------------------------------------------------
  inline  void   SetDirection(char d){aMove = d;}
  //---------------------------------------------------------------
  inline void    MaxTime(float t) {if (t > aTmax) aTmax = t;}
  //---------------------------------------------------------------
  inline  void   SetTrace()       {tr = 1;}
  inline  void   SetKFR0()        {aStat = ITEM_KFR0;}
  inline  void   SetKFR1()        {aStat = ITEM_KFR1;}
  inline  void   SetIDLE()        {aStat = ITEM_BETW;}
  inline  void   SetRUNS(){aStat = eStat = ITEM_MOVE;}
  inline  void   SetSTOP(){aStat = eStat = ITEM_BETW;}
  //---------------------------------------------------------------
  inline  float  Target()         {return aTarg;}
  inline  char   GetDirection()   {return aMove;}
  inline  char   GetState()       {return aStat;}
  inline  void   EndState(char s) {eStat = s;}
  inline  char   Motion()         {return aMove;}
  inline  CAniPart *GetPart(int k){return obj[k];}
};

//=======================END OF FILE ===============================================
#endif // MODELACM_H

