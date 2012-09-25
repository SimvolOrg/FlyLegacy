/*
 * Cameras.h
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

/*! \file Cameras.h
 *  \brief Defines CCamera user camera base class and default camera views
 *
 * There are four default pre-defined cameras, enumerated by ECameraType
 *   and implemented in Cameras.cpp.  Additional DLL camera objects may
 *   also be available.
 *
 */
 

#ifndef CAMERAS_H_
#define CAMERAS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlyLegacy.h"
#include "3dMath.h"
#include <map>
//===================================================================================
class CCanva;
//===================================================================================
struct RGBA_COLOR 
{ float R;
  float G;
  float B;
  float A;
};
//===================================================================================
//	Various definitions
//===================================================================================
#define CAM_MAN_MODE	(1)								// Manual mode
#define CAM_TRK_MODE	(2)								// Track mode
//===================================================================================
//	Camera profile
//===================================================================================
#define CAM_SIDE_ROT (0x0001)						// Camera can rotate left/right
#define CAM_VERT_ROT (0x0002)						// Camera may rotate up/down
#define CAM_MAY_MOVE (0x0004)						// Camera may move in/out from target
#define CAM_MAY_ZOOM (0x0008)						// Camera may zoom in/out
//---Combination ------------------------------------------------
#define CAM_IS_SPOT  (CAM_SIDE_ROT + CAM_VERT_ROT + CAM_MAY_MOVE + CAM_MAY_ZOOM)
//===================================================================================
//   Class Tracker for interractive windows.
//		This class is used in conjonction with camera to pick object on screen
//		from mouse click
//		Derived class must supply the Draw() and OneSeletion() functions
// 
//====================================================================================
class Tracker {
	//--- ATTRIBUTS ----------------------------------------------
	//--- METHODS ------------------------------------------------
public:
	Tracker() {;}
	virtual void		DrawMarks()							{;}
	virtual void    OneSelection(U_INT No)	{;}
};
//===================================================================================
// Abstract class common to all cameras.  All subclasses must implement the
//   mandatory UpdateCamera method
//===================================================================================
class CCamera : public CStreamObject {
public:
  // Constructors
  CCamera (void);
  virtual ~CCamera (void);
  virtual void    GetCameraName (char* name, int maxLength)
  {
    strncpy (name, "Invalid Camera", maxLength);
  }

  //--- CStreamObject methods ------------------------------------
  virtual int   Read (SStream *stream, Tag tag) { return TAG_IGNORED; }

  //--- Activate() method is called when the camera instance is activated by CCameraManager
  virtual void  Activate (void) {}

  // UpdateCamera is a pure virtual method and therefore must be implemented by
  //   all descended classes.  The position and orientation of the target
  //   object are supplied in the tgtPosition and tgtOrientation arguments;
  //   these should never be updated.
	virtual void    CameraReferential();
	virtual void    UpdateCamera (SPosition wPos, SVector tgtOrient,float dT = 0) = 0;
  virtual void    GetPosition(SPosition &pos);
  virtual void    StartShoot(float dT);
  virtual void    StopShoot();
  virtual void    GetLookatPoint (SVector &v);
  virtual Tag     GetCameraType (void) { return CAMERA_INVALID; }
  //--- Camera manipulation methods------------------------------------
	virtual void  MinRange(double m)	{rmin = m; range =m;}
  virtual void  ZoomRatioIn (void);
  virtual void  ZoomRatioOut (void);
  virtual void  RangeIn (void);
  virtual void  RangeInSlow (void);
  virtual void  RangeInFast (void);
  virtual void  RangeOut (void);
  virtual void  RangeOutSlow (void);
  virtual void  RangeOutFast (void);
  virtual void  RangeReset (void);
	virtual void  MoveTo (double inc, double tg) {;}
	void  StopMove() {move = 0;}
  // Camera position manipulation methods.  These do nothing by default, specific
  //   camera subclasses must implement them
  virtual void  PanLeft (void) {}
  virtual void  PanRight (void) {}
  virtual void  PanUp (void) {}
  virtual void  PanDown (void) {}
  virtual void  MoveBy(int px,int py) {}
  virtual void  Rotate(double r) {}
  virtual void  Stop()  {};

  // User position methods.  These are traditionally implemented just for the
  //   cockpit camera, though there's no reason why they couldn't be implemented
  //   for external cameras as well.
  virtual void  User1 (void) {}
  virtual void  DefineUser1 (void) {}
  virtual void  User2 (void) {}
  virtual void  DefineUser2 (void) {}
  virtual void  User3 (void) {}
  virtual void  DefineUser3 (void) {}
  virtual void  User4 (void) {}
  virtual void  DefineUser4 (void) {}
	//-------Camera Manual Handling -------------------------------------
	virtual bool	NotManual()			{return true;}
	virtual bool	ToggleMode()		{return false;}	// Toggle mode
	virtual bool  NoHatSupport()	{return true;}
	//------------------------------------------------------------
	virtual void  ActivateView (float A) {}
  virtual void  HeadPitchUp (void) {}
  virtual void  HeadPitchDown (void) {}
  // sdk: User position method for external cameras position.
  virtual void  SetCameraPosition  (const float &pitchInRads,
                                    const float &headingInRads,
                                    const float &distanceInFeet) {}
  virtual double GetLevel()         {return 0;}
  virtual void  ChangeViewPort();
	//--- Rabbit methods -----------------------------------------
	virtual void	RabbitMoveTo(SPosition *pos)  {;}
	//--- Picking methods ----------------------------------------
	void		StartPicking();
	void		SetTracker(Tracker *t, CFuiWindow *w);
	void		StopPicking()		{;}
	//------------------------------------------------------------
  virtual void Print (FILE *f);
  //------------------------------------------------------------
	void		ReleaseIndicators();
	//--- Check camera type --------------------------------------
	bool    IsOf(Tag t) {return (GetCameraType() == t);}
  //------------------------------------------------------------
	void		SetAbove(SPosition pos, int mr, int cr);
	void		GoToPosition(SPosition &dst);
	void		ToggleBox();
  void    OffsetFrom(SPosition &pos,CVector &v);
  void    AbsoluteFeetPosition(CVector &p);
  void    DrawObject(float dT,VIEW_PORT  &vp,CFuiWindow *win);
  void    Projection(VIEW_PORT &vp,U_INT xOBJ);
  void    Projection(VIEW_PORT &vp,TEXT_INFO &inf);
  void    SetAngle(double a,double b);
  void    Reset();
  void    Save(CAMERA_CTX &ctx);
  void    Restore(CAMERA_CTX &ctx);
  void    SetMinAGL();
	void		NoseUp();
	void		NoseDown();
	double  SetRange(double d);
  void    SetZoom(float z);
  void    RangeAdjust(double lg);
  void    ShowRange();
	void		RockArround (SPosition tpos, SVector tori,float dT);
	void		UpdateCameraPosition(SPosition &wpos);
	//------------------------------------------------------------
	//--- Set perspective parameters -----------------------------
	void    SetCameraParameters(double fov, double ratio);
	void		SetReferential(SPosition &tg);
	bool		GeoPosInFrustum(SPosition &P, CVector &R, char *T);
	bool		BoxInFrustum(SPosition &P, CVector &B, double fc);
	//------------------------------------------------------------
	void		RoundLeft();
	void		RoundRight();
	void		RoundUp();
	void		RoundDown();
  //------------------------------------------------------------
  Tag   NextCamera()   {return cNext;}
  Tag   PrevCamera()   {return cPrev;}
  Tag   GetIdent()     {return cIden;}
  void  SetNext(Tag t) {cNext = t;}
  void  SetPrev(Tag t) {cPrev = t;}
  SVector *GetOffset() {return &offset;}
  void  SetOffset(SVector &v) {offset = v;}
  void  SetUpDir(SVector &u)  {Up = u;}
	//------------------------------------------------------------
	float GetRate()						{return Rate;}
	void  SetAngles(SVector &a)	{theta = a.x; phi = a.y;}
  //---Position parameters -------------------------------------
	void   FootPerPixel();
	double GetFPIX()			{return fpp;}
	double GetAzimuth()		{return orient.z;}
	double GetElevation()	{return orient.x;}
	double GetTheta()						{return theta;}
	double GetPhi()							{return phi;}
  float  GetXofs()						{return offset.x;}
  float  GetYofs()						{return offset.y;}
  float  GetZofs()						{return offset.z;}
  double GetRange()						{return range;}
  void   SetMaxRange(double m){rmax  = m;}
  void   GetUpVector (SVector &v) {v = Up;}
  double GetTargetLon()				{return tgtPos.lon;}
  double GetTargetLat()				{return tgtPos.lat;}
  double GetTargetAlt()				{return tgtPos.alt;}
  float  GetFOV ()						{return fov;}
  SVector   &CamOffset()			{return offset;}
	void   GetOffset (SVector &v)            {v = offset;}
  void   GetOrientation (SVector &v)       {v = orient;}
	//------------------------------------------------------------
	char	GetINTMOD()					{return intcm;}
	char  GetEXTMOD()					{return extcm;}
	//---Tracking functions --------------------------------------
	bool		PickObject(U_INT mx, U_INT my);

  //---CAMERA ATTRIBUTES ---------------------------------------
public:
	COption   Prof;				// Camera profile
protected:
	float			Rate;				// Rotation rate (1/4 sec unit)
	//--- Camera type (inside or outside) ------------------------
	char			intcm;			// Type of camera (1= inside)
	char			extcm;			// Type of camera (1= outside
	//--------Camera parameters ----------------------------------
  double    range;      // Range
  double    rmin, rmax; // Min/max range
  double    clamp;      // Vertical rotation angle clamp
  double    minAGL;     // Minimum above ground levl
	double    farP;				// Far plan
	//------------------------------------------------------------
	SPosition tgtPos;			// Target position
	SPosition camPos;			// Camera position
  CVector   offset;     // Camera offset from target vehicle location (in feet)
  CVector   orient;     // Camera orientation with respect to world axes
	//---  Camera automove parameters ----------------------------
	double		moveInc;		// Zoom increment
	double    moveTgt;		// Zoom target
	//-----Camera orientation ------------------------------------
  CVector   Up;         //  Up vector (
	CVector   Fw;         //  Forward
	CVector   Lf;					// Left 
	//---- Camera referential ----------------------------------
	CVector   Rx;					// X  (left) vector reference
	CVector   Ry;					// Y (forward)	reference vector
	CVector   Rz;					// Z (Up)  reference vector
	CVector   PJ;					// Projection of forward
	//----Internal parameters -----------------------------------
  double    fov;        // Horizontal field of view in degrees
	double    tgf;				// Tangent of FOV
	double    ratio;			// Aspect ratio
  double    nearP;      // Near plan
	double    ffac;				// Flare factor
	double		fpp;				// Feet per pixel
	double		hpan;				// Height of near plan
	double    mphi;				// Minimum phi
  //----Locking ------------------------------------------------
	char			pick;				// Picking indicator
	char      move;				// move indicator
  //----Next and previous cammeras -----------------------------
  Tag       cIden;      // Camera identity
  Tag       cNext;      // Camera next
  Tag       cPrev;      // Camera previous
  //----Quad for texture projection ----------------------------
  TC_VTAB     Pan[4];   ///  A Quad
  //----View port save area ------------------------------------
  VIEW_PORT vps;        // View port save area
  //----Camera angles ------------------------------------------
  double     theta;     // Around Z
  double      phi;      // Around X
	//--- Picking parameters -------------------------------------
	GLuint				bHit[8];									// Hit buffer
	//------------------------------------------------------------
	CAMERA_CTX    *ctx;										  // Original context
	//--- Picking parameters -------------------------------------
	int		picx;															// Screen cursor
	int		picy;															// Screen cursor
	Tracker    *trak;												// Tracker
	CFuiWindow *twin;												// Associated Windows to warn

};
//====================================================================================
// Rabbit camera
//  Camera used for editor purpose.  No aircraft is displayed.
//	A virtual rabbit is set at the geographical position, and the camera
//	tracks this rabbit
//====================================================================================
class CRabbitCamera : public CCamera {
	//--- ATTRIBUTES ---------------------------------------------
	//--- METHODS ------------------------------------------------
public:
	CRabbitCamera();
 ~CRabbitCamera();
  //------------------------------------------------------------
	void		RabbitMoveTo(SPosition *pos);
	void		TurnRabbit(SVector v);
	//-------------------------------------------------------------
	void	UpdateCamera (SPosition tpos, SVector tori,float dT);
	//-------------------------------------------------------------
	void  MoveTo (double inc, double tg);
 	//-------------------------------------------------------------
	void  PanLeft();	
	void  PanRight();	
	void  PanUp();			
	void  PanDown();   
	//-------------------------------------------------------------
	inline void Store(CAMERA_CTX *t) {ctx = t;}
};
//====================================================================================
// Cockpit (vehicle interior) camera
//  JS:  Add up vector
//====================================================================================
class CCameraCockpit : public CCamera {
public:
  CCameraCockpit (CVehicleObject *mv);
  virtual ~CCameraCockpit (void);

  // CStreamObject methods
  int     Read (SStream *stream, Tag tag);
  void    ReadFinished (void);
	void		Init(CVehicleObject *mv);

  // CCamera methods
  void  UpdateCamera (SPosition targetPosition, SVector targetOrientation,float dT);
  void  GetLookatPoint (SVector &v);
  Tag   GetCameraType (void) { return CAMERA_COCKPIT; }
  void  GetCameraName (char* name, int maxLength)
  {
    strncpy (name, "Cockpit Camera", maxLength);
  }
	//-----------------------------------------------
	void	MinRange(double m)	{;}		// Ignore 
	//-----------------------------------------------
  void  PanLeft (void);
  void  PanRight (void);
  void  PanUp (void);
  void  PanDown (void);
	//-----------------------------------------------
  void  User1 (void);
  void  User2 (void);
  void  User3 (void);
  void  User4 (void);
	//-----------------------------------------------
  void  DefineUser1 (void);
  void  DefineUser2 (void);
  void  DefineUser3 (void);
  void  DefineUser4 (void);
  void  HeadPitchUp (void);
  void  HeadPitchDown (void);

  //---- CCameraCockpit methods -------------------
	void			CameraReferential();
  void      SetUpVector(SVector &ori);
  void      ChangeViewPort();
	//-----------------------------------------------
	void			SetMatrix(SVector ori);
  //-----------------------------------------------
	void      GetOFS(SVector &v)    {v = Ofs;}
  SVector  &GetSeat()             {return Seat;}
	bool			NoHatSupport()	{return false;}
  //-----------------------------------------------
protected:
	CVehicleObject							 *mveh;						//  Mother Vehicle
	CCockpitManager							 *pit;						//  Cockpit manager
  SVector                       Seat;           //  Orientation angles of <seat> tag
  CVector                       Ofs;            //  True offset
  double                        Head;           //  Pilot head direction
	//--- transformation matrix ----------------------
	double												mSEAT[16];			// matrice seat
};

//========================================================================================
// External spot camera
//========================================================================================
class CCameraSpot : public CCamera {
public:
  // Constructors/Destructors
  CCameraSpot (void);
  // CCamera methods
  virtual void  UpdateCamera (SPosition tgtPos, SVector tgtOrient,float dT);
  virtual Tag   GetCameraType (void) { return CAMERA_SPOT; }
  virtual void  GetCameraName (char* name, int maxLength)
  {
    strncpy (name, "Spot Camera", maxLength);
  }
	//-----------------------------------------------------------------
	void  PanLeft()		{RoundLeft();}
	void  PanRight()	{RoundRight();}
	void  PanUp()			{RoundUp();}
	void  PanDown()   {RoundDown();}
  //-----------------------------------------------------------------
  // sdk:
  void  SetCameraPosition  (const float &pitchInRads,
                                    const float &headingInRads,
                                    const float &distanceInFeet);
  //-----------------------------------------------------------------
  void  MoveBy(int px,int py);
	void  RangeFor(double D, double A);
};

//===========================================================================
// External observer camera
//===========================================================================
class CCameraObserver : public CCamera {
public:
  // Constructors/Destructors
  CCameraObserver (void);

  // CCamera methods
  void  UpdateCamera (SPosition tgtPosition, SVector tgtOrient,float dT);
  Tag   GetCameraType (void) { return CAMERA_OBSERVER; }
  void  GetCameraName (char* name, int maxLength)
  {
    strncpy (name, "Observer Camera", maxLength);
  }
  void  PanLeft (void);
  void  PanRight (void);
  void  PanUp (void);
  void  PanDown (void);
  //-----------------------------------------------------------------
  void  SetCameraPosition  (const float &pitchInRads,
                            const float &headingInRads,
                            const float &distanceInFeet);
  //-----------------------------------------------------------------

};

//==================================================================================
// External fly-by camera
//==================================================================================
class CCameraFlyby : public CCamera {
public:
  // Constructors/Destructors
  CCameraFlyby (void);

  // CCamera methods
  void  UpdateCamera (SPosition tgtPos, SVector tgtOrient,float dT);
  Tag   GetCameraType (void) { return CAMERA_FLYBY;}
  void  GetCameraName (char* name, int maxLength)
  {
    strncpy (name, "Fly-by Camera", maxLength);
  }
  //-----------------------------------------------------------------
protected:
  double rng;
  double distance;
  double r0;
};

//===================================================================================
// External tower camera
//
// When selected, the eyepoint for this camera is placed a the nearest
//   airport tower location.
//====================================================================================
class CCameraTower : public CCamera {
public:
  // Constructors/Destructors
  CCameraTower (void);
  //---  CCamera methods --------------------------------------------------
  Tag   GetCameraType (void) { return CAMERA_TOWER; }
	//-----------------------------------------------------------------------
  void  GetCameraName (char* name, int maxLength)
  {
    strncpy (name, "Tower Camera", maxLength);
  }
	//----------------------------------------------------------------------
	void	MinRange(double m)	{;}			// Ignore
	//----------------------------------------------------------------------
	CVector		ComputeOffset(SPosition tgt);
  void  ZoomRatioIn (void);
  void  ZoomRatioOut (void);
  void			UpdateCamera (SPosition tgtPos, SVector tgtOrient,float dT);
	void			UpdatePosition(float Dt);
	void			ComputeAngle(CVector &v);
	void			PanLeft();
	void			PanRight();
	void			PanUp();
	void			PanDown();
	bool	ToggleMode();
	void	GetLookatPoint(SVector &tgp);
	//----------------------------------------------------------------------
	inline		bool NotManual()	{return (Mode != CAM_MAN_MODE); }
  //----------------------------------------------------------------------
protected:
	char			Mode;
	//---- Azimut and elevation ----------------------
	CVector   tpos;
	//------------------------------------------------
  float     t_zoomRatio;
  float     twr_timer;
  double    distance;
  double    r0;
  void GetNearestAirport (void);
};

//=======================================================================================
// Overhead camera
//
// When selected, the eyepoint for this camera is placed above the aircraft
//   looking down towards the ground.
//========================================================================================
class CCameraOverhead : public CCamera {
public:
  // Constructors/Destructors
  CCameraOverhead (void);

  void  UpdateCamera (SPosition tgtPos, SVector tgtOrient,float dT);
  Tag   GetCameraType (void) { return CAMERA_OVERHEAD;}
  void  GetCameraName (char* name, int maxLength)
  {
    strncpy (name, "Overhead Camera", maxLength);
  }
  //-------------------------------------------------------------------------------
};

//=======================================================================================
// Orbit camera
//
// This camera orbits around the target vehicle.  The orbit path is always parallel
//   to the ground, and can be elevated up/down and zoomed in/out
//=======================================================================================
class CCameraOrbit : public CCamera {
public:
  // Constructors/Destructors
  CCameraOrbit (void);

  void  UpdateCamera (SPosition tgtPos, SVector tgtOrient,float dT);
  Tag   GetCameraType (void) { return CAMERA_ORBIT; }
  void  GetCameraName (char* name, int maxLength)
  {
    strncpy (name, "Orbit Camera", maxLength);
  }
  void  PanLeft (void);
  void  PanRight (void);
  void  PanUp (void);
  void  PanDown (void);
  //---------------------------------------------------------------
  void    Rotate(double deg);
  //---------------------------------------------------------------
  inline void Stop()       {orbitRate = 0;}
  //---------------------------------------------------------------
protected:
  SVector   pod;            /// Camera pod location
  float     orbitRate;      ///< Rate of orbiting in rad/s
};
//=======================================================================================
// Runway camera
//
// When selected, the eyepoint for this camera is placed above a selected airport
//   looking down towards the ground.
//========================================================================================
class CCameraRunway : public CCamera {
  //----ATTRIBUTES -------------------------------------------------------
public:
  // Constructors/Destructors
  CCameraRunway (void);
  void    UpdateCamera (SPosition tgtPos, SVector tgtOrient,float dT);
  void    SetView();
  void    MoveBy(float nx,float ny);
  void    Zoom(int zf);
  void    GetLookatPoint(SVector &v);
	//-----------------------------------------------------------------
  double  SetOrigin(SPosition *org);
  double  MoveUp(int df);
  //-----------------------------------------------------------------
  inline  double GetUPF()         {return offset.z;}
};
//=======================================================================================
// Object camera
//  Object camera is placed at -1000 feet to draw object to avoid
//        any interference between object and real scenery
//  NOTE:   Object must translate to -100 feet in the PreDraw() routine
//======================================================================================
class CCameraObject: public CCamera {
  //---Additional attributes ---------------------------------------
  int     wd;
  int     ht;
  char    mv;
  double  scale;
  double  radius;
  //----------------------------------------------------------------
  RGBA_COLOR   bCOL;             // BackGround color
  //-----FBO object ------------------------------------------------
  GLuint    FBO;
  GLuint    IMG;
  GLuint    DEP;
  //---Methods -----------------------------------------------------
public:
  CCameraObject();
 ~CCameraObject();
  void  UpdateCamera(SPosition tgt,SVector ori,float dT );
  void  Refresh();
  void  GetLookatPoint(SVector &v);
  void  TopOffsetFor(double ht);
  void  FrontOffsetFor(double ht);
  void  SetParameters(double rd, double fv,int w, int h);
  void  SwapView();
  //---Camera move --------------------------------------------------
  void    PanLeft ();
  void    PanRight();
  void    PanUp();
  void    PanDown();
  void    MoveBy(int px, int py);
  double  ZoomBy(double rat);
  void    SetColor(RGBA_COLOR &rgba);
  //----Drawing routines --------------------------------------------
  void    DebDrawFBOinPerspective(int w, int h);
  void    DebDrawFBOinOrthoMode(int w, int h);
  void    EndDrawFBO();
  //-----------------------------------------------------------------
  void  SetFBO(int wd,int ht);
  void  DrawOnWin(VIEW_PORT &vp,CFuiWindow *win);
  //-----------------------------------------------------------------
  U_INT   PassTextureObject();
  //-----------------------------------------------------------------
  inline  void    MoveMouseIs(char m) {mv = m;}
  inline  GLuint  TextureObject() {return IMG;}
};
//===================================================================================
// Abstract class to draw on a FBO
//
//===================================================================================
class CCanva {
protected:
  int wd;                       // Texture dimension
  int ht;                       // Texture height
  U_INT xOBJ;                   // Texture Object
  CCameraObject *cam;
  U_INT bak;                    // Background color
  //----ATTRIBUTES ---------------------------------------------
public:
  CCanva(int w,int h);
 ~CCanva();
  //------------------------------------------------------------
 void   SetParameters(double rd, double fv);
 void   FrontView(double rad);
 void   EndDraw();
 void   PopTexture();
 void   WriteTexture();
 U_INT  GetTextureObject();
 //-------------------------------------------------------------
 inline void  PreDraw() {if (cam) cam->DebDrawFBOinPerspective(wd,ht);}
};
//===================================================================================
// DLL camera
//
// 
// 
//====================================================================================
class CCameraDLL : public CCamera {
public:
  bool enabled;
  void *dll;
  // Constructors/Destructors
  CCameraDLL (Tag&, const char*, int&);
  virtual ~CCameraDLL (void);

  // CCamera methods
  virtual void  UpdateCamera (SPosition tgtPos, SVector tgtOrient,float dT);
  virtual Tag   GetCameraType (void);
  virtual void  GetCameraName (char* name, int maxLength);
  virtual void  GetLookatPoint (SVector &v);
  //-----------funct members ---------------------------------------------
  void SetObject (SDLLObject *object);
  void SetSignature (const long &sign);
  void Prepare (void);
  const SDLLObject* Get_Object (void) {return obj;}
  const long& GetSignature (void) {return signature;}

protected:
  int is_interior_camera;
  char camera_name [FILENAME_MAX];
  Tag camera_type;

private:
  SDLLObject *obj;
  long signature;
  SPosition eyePos_;
  SVector   eyeOri_;
};
//==============================================================================
// Camera Views list
//
// CCameraViewsList gets a list of the cameras that are listed in DATA/CAMERAS.TXT
// SCameraType is a structure used with the data in CAMERAS.TXT  
//===============================================================================
struct SCameraType {
  char camera_tag [4+1];
  char camera_name[64+1];
  int val;
  SCameraType (void) {
    *camera_tag = 0;
    *camera_name = 0;
    val = 0;
  }
};
//==============================================================================
// Camera Views list
//==============================================================================
class CCameraViewsList {
public:
  SCameraType *cam_type;
  CCameraViewsList (void);
  ~CCameraViewsList (void);

  // methods
  int   ReadCamerasFile (void);
  inline const int& GetNumItems (void) const {return num_lines;}
  inline const char* GetTag (const int& i) const
  { if (type) return type[i].camera_tag;
    else return 0;
  }
  inline const char* GetCameraName (const int& i) const
  { if (type) return type[i].camera_name;
    else return 0;
  }
  inline const int GetVal (const int& i) const
  { if (type) return type[i].val;
    else return 0;
  }
private:
  int num_lines;
  SCameraType *type;
};

//==============================================================================
//	Camera option
//==============================================================================
#define RABBIT_CONTROL    (0x01)
#define RABBIT_IN_SLEW    (0x02)
//-----------------------------------------------------------------------
#define RABBIT_S_AND_C		(RABBIT_CONTROL | RABBIT_IN_SLEW)
//==============================================================================
// Camera Manager
//
// CCameraManager links together all of the various static and interactive
//   panel views defined in the CCockpitManager (.PIT file).
//===============================================================================
class CCameraManager : public CStreamObject {
public:
   CCameraManager (CVehicleObject *veh,char* fn);
  ~CCameraManager (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);
	//---------------------------------------------------------------------
	void			ReadPanelCamera(CVehicleObject *veh,char * fn);
  //----- CCameraManager ------------------------------------------------
  void      BindKeys();
  void      ZeroRate();
  void      DefaultCameras();
  void      ExplicitCameras(int nb);
  void      Link(CCamera *cam,int k,int last);
  void      UpdateCamera (SPosition tgtPos, SVector tgtOrient,float dT);
  void      AdjustRange(double lg);
  //----------------------------------------------------------------------------
  void      NextCamera (void);
  void      PrevCamera (void);
  CCamera  *SelectCamera (Tag id);
  CCamera  *GetCamera(Tag id);
  CCameraCockpit *GetCockpitCamera();
	void			RestoreCamera(CAMERA_CTX &ctx);
	void			SetMinimumRange(double m);
	//----------------------------------------------------------------------------
	CRabbitCamera *SetRabbitCamera(CAMERA_CTX &ctx,U_CHAR opt);
  //----------------------------------------------------------------------------
  void      KbEvent(Tag id);                // Keyboard command
  bool      KeyCameraCockpitEvent(int id);
  //----------------------------------------------------------------------------
  void      Print (FILE *f);
  //----------------------------------------------------------------------------
  CCamera*  GetActiveCamera ()     {return aCam;}
	void			NoCamera() 						 {aCam = 0;}
  //----------------------------------------------------------------------------
protected:
  ///----------Attributes ------------------------------------------------------
	CVehicleObject *mveh;												// Mother vehicle
  CCamera  *aCam;                             // Active camera
  std::map<Tag,CCamera*>   came;              // List of standard cameras
	Tag				fcam;															// First camera
	Tag				lcam;															// Last camera 
	Tag       tCam;                             // Tag of current camera
  /// Current camera parameters
  SPosition tgtPos;
  SVector   tgtOrient;
  char      Internal;                         // Camera is internal if (1)
  ///--------Cameras list from DATA\CAMERAS_LEGACY.TXT --------------------------
  CCameraViewsList cam_list;
};

//==============END OF FILE ================================================================
#endif // CAMERAS_H_

