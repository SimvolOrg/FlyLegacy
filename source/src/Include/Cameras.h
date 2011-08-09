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
  virtual void    UpdateCamera (SPosition tgtPos, SVector tgtOrient,float dT = 0) = 0;
  virtual void    GetPosition(SPosition &pos);
  virtual void    StartShoot(float dT);
  virtual void    StopShoot();
  virtual void    GetLookatPoint (SVector &v);
  virtual Tag     GetCameraType (void) { return CAMERA_INVALID; }
  // Camera FOV manipulation methods.
  virtual void  ZoomRatioIn (void);
  virtual void  ZoomRatioOut (void);
  virtual void  RangeIn (void);
  virtual void  RangeInSlow (void);
  virtual void  RangeInFast (void);
  virtual void  RangeOut (void);
  virtual void  RangeOutSlow (void);
  virtual void  RangeOutFast (void);
  virtual void  RangeReset (void);

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
	//------------------------------------------------------------
  virtual void  HeadPitchUp (void) {}
  virtual void  HeadPitchDown (void) {}
  // sdk: User position method for external cameras position.
  virtual void  SetCameraPosition  (const float &pitchInRads,
                                    const float &headingInRads,
                                    const float &distanceInFeet) {}
  virtual double GetLevel()         {return 0;}
  virtual void  ChangeResolution();
	//--- Picking methods ----------------------------------------
	virtual void  StartPicking()	{;}
	virtual void	StopPicking()		{;}
	//------------------------------------------------------------
  virtual void Print (FILE *f);
  //------------------------------------------------------------
	void		ReleaseIndicators();
	//--- Check camera type --------------------------------------
	bool IsOf(Tag t) {return (GetCameraType() == t);}
  //------------------------------------------------------------
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
  bool    GoodHeight (double aphi);
  double  SetRange(double d);
  void    SetZoom(float z);
  void    RangeAdjust(double lg);
  void    ShowRange();
	void		RockArround (SPosition tpos, SVector tori,float dT);
	//------------------------------------------------------------
	void		RoundLeft();
	void		RoundRight();
	void		RoundUp();
	void		RoundDown();
  //------------------------------------------------------------
  inline Tag   NextCamera()   {return cNext;}
  inline Tag   PrevCamera()   {return cPrev;}
  inline Tag   GetIdent()     {return cIden;}
  inline void  SetNext(Tag t) {cNext = t;}
  inline void  SetPrev(Tag t) {cPrev = t;}
  inline SVector *GetOffset() {return &offset;}
  inline void  SetOffset(SVector &v) {offset = v;}
  inline void  SetUpDir(SVector &u)  {Up = u;}
  //------------------------------------------------------------
  inline void  SetLock()            {Lock = 1;}
  inline void  SetLock(char k)      {Lock = k;}
	//------------------------------------------------------------
  inline bool  IsLocked()           {return (Lock != 0);}
	inline float GetRate()						{return Rate;}
  //------------------------------------------------------------
	inline double GetAzimuth()		{return orient.z;}
	inline double GetElevation()	{return orient.x;}
	inline double	GetTheta()					{return theta;}
	inline double GetPhi()						{return phi;}
  inline float  GetXofs()           {return offset.x;}
  inline float  GetYofs()           {return offset.y;}
  inline float  GetZofs()           {return offset.z;}
  inline double GetRange()          {return range;}
  inline void   SetMaxRange(double m){rmax  = m;}
  inline void   GetUpVector (SVector &v) {v = Up;}
  inline double GetTargetLon()      {return Tgt.lon;}
  inline double GetTargetLat()      {return Tgt.lat;}
  inline double GetTargetAlt()      {return Tgt.alt;}
  inline float  GetFOV ()           {return fov;}
  inline SVector   &CamOffset()     {return offset;}
	inline void   GetOffset (SVector &v)            {v = offset;}
  inline void   GetOrientation (SVector &v)       {v = orient;}
	//------------------------------------------------------------
	inline	char	GetINTMOD()					{return intcm;}
	inline  char  GetEXTMOD()					{return extcm;}
  //------------------------------------------------------------
protected:
  char      Prop;       // properties
	float			Rate;				// Rotation rate (1/4 sec unit)
	COption   Prof;				// Camera profile
	//--- Camera type (inside or outside) ------------------------
	char			intcm;			// Type of camera (1= inside)
	char			extcm;			// Type of camera (1= outside
	//--------Camera parameters ----------------------------------
  SPosition Tgt;        // Camera target world position 
  float     fov;        // Horizontal field of view in degrees
  double    range;      // Range
  double    dmin;       // Minimum distance
  double    rmin, rmax; // Min/max range
  double    clamp;      // Vertical rotation angle clamp
  double    minAGL;     // Minimum above ground levl
  double    nearP;      // Near plan
	double    farP;				// Far plan
	SPosition camPos;
  SVector   offset;     // Camera offset from target vehicle location (in feet)
  SVector   orient;     // Camera orientation with respect to world axes
  SVector   Up;         // Up vector
  //----Locking ------------------------------------------------
  char      Lock;       // Lock indicator
	//----Picking ------------------------------------------------
	char			pick;				// Picking indicator
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

};
//====================================================================================
// Rabbit camera
//  Camera used for editor purpose.  No aircraft is displayed.
//	A virtual rabbit is set at the geographical position, and the camera
//	tracks this rabbit
//====================================================================================
class CRabbitCamera : public CCamera {
	//--- ATTRIBUTES ---------------------------------------------
	CAMERA_CTX       ctx;										// Original context
	//--- Picking parameters -------------------------------------
	int		px;																// Screen cursor
	int		py;																// Screen cursor
	//--- METHODS ------------------------------------------------
public:
	CRabbitCamera();
 ~CRabbitCamera();
  //-------------------------------------------------------------
	void		SetPicking(int x,int y);
	void		StartPicking();
	void		StopPicking()        {pick = 0;}
	//-------------------------------------------------------------
	void	UpdateCamera (SPosition tpos, SVector tori,float dT);
 	//-------------------------------------------------------------
	void  PanLeft()		{RoundLeft();}
	void  PanRight()	{RoundRight();}
	void  PanUp()			{RoundUp();}
	void  PanDown()   {RoundDown();}
	//-------------------------------------------------------------
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
  virtual int     Read (SStream *stream, Tag tag);
  virtual void    ReadFinished (void);

  // CCamera methods
  virtual void  UpdateCamera (SPosition targetPosition, SVector targetOrientation,float dT);
  virtual void  GetLookatPoint (SVector &v);
  virtual Tag   GetCameraType (void) { return CAMERA_COCKPIT; }
  virtual void  GetCameraName (char* name, int maxLength)
  {
    strncpy (name, "Cockpit Camera", maxLength);
  }

  virtual void  PanLeft (void);
  virtual void  PanRight (void);
  virtual void  PanUp (void);
  virtual void  PanDown (void);
	//-----------------------------------------------
  virtual void  User1 (void);
  virtual void  User2 (void);
  virtual void  User3 (void);
  virtual void  User4 (void);
	//-----------------------------------------------
  virtual void  DefineUser1 (void);
  virtual void  DefineUser2 (void);
  virtual void  DefineUser3 (void);
  virtual void  DefineUser4 (void);
  virtual void  HeadPitchUp (void);
  virtual void  HeadPitchDown (void);

  //---- CCameraCockpit methods -------------------
  Tag       GetCockpitPanel (void);
  void      SetUpVector(SVector &ori);
  void      ActivateCockpitPanel (Tag tag);
  void      SetPanel(Tag id,CPanel *p);
  void      ChangeResolution();
  void      DrawPanel();
  void      GetXSRC(TC_4DF &r);
	//-----------------------------------------------
	void			SetMatrix(SVector ori);
	//-----------------------------------------------
	CPanel*   GetPanel();
  //-----------------------------------------------
  inline void     GetOFS(SVector &v)    {v = Ofs;}
  inline SVector &GetSeat()             {return Seat;}
  //-----------------------------------------------
protected:
	CVehicleObject							 *mveh;						//  Mother Vehicle
  std::map<Tag,CCockpitPanel*>  panl;           //  Map unique IDs to cockpit panels
  SVector                       Seat;           //  Orientation angles of <seat> tag
  CCockpitPanel*                ckPanel;        //  Pointer to currently active panel
  CVector                       Fw;             //  Forward
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
};

//===========================================================================
// External observer camera
//===========================================================================
class CCameraObserver : public CCamera {
public:
  // Constructors/Destructors
  CCameraObserver (void);

  // CCamera methods
  virtual void  UpdateCamera (SPosition tgtPosition, SVector tgtOrient,float dT);
  virtual Tag   GetCameraType (void) { return CAMERA_OBSERVER; }
  virtual void  GetCameraName (char* name, int maxLength)
  {
    strncpy (name, "Observer Camera", maxLength);
  }
  virtual void  PanLeft (void);
  virtual void  PanRight (void);
  virtual void  PanUp (void);
  virtual void  PanDown (void);
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
  virtual void  UpdateCamera (SPosition tgtPos, SVector tgtOrient,float dT);
  virtual Tag   GetCameraType (void) { return CAMERA_FLYBY;}
  virtual void  GetCameraName (char* name, int maxLength)
  {
    strncpy (name, "Fly-by Camera", maxLength);
  }
  //-----------------------------------------------------------------
protected:
  SPosition   cameraPos;      ///< Geographical position of camera
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
  virtual Tag   GetCameraType (void) { return CAMERA_TOWER; }
	//-----------------------------------------------------------------------
  virtual void  GetCameraName (char* name, int maxLength)
  {
    strncpy (name, "Tower Camera", maxLength);
  }
	//----------------------------------------------------------------------
	CVector		ComputeOffset(SPosition tgt);
  virtual void  ZoomRatioIn (void);
  virtual void  ZoomRatioOut (void);
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

  virtual void  UpdateCamera (SPosition tgtPos, SVector tgtOrient,float dT);
  virtual Tag   GetCameraType (void) { return CAMERA_OVERHEAD;}
  virtual void  GetCameraName (char* name, int maxLength)
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

  virtual void  UpdateCamera (SPosition tgtPos, SVector tgtOrient,float dT);
  virtual Tag   GetCameraType (void) { return CAMERA_ORBIT; }
  virtual void  GetCameraName (char* name, int maxLength)
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
  void    SetOrigin(SPosition *org);
  void    SetView();
  void    MoveUp(int df);
  void    MoveBy(float nx,float ny);
  void    Zoom(int zf);
  void    GetLookatPoint(SVector &v);
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

//==============END OF FILE ================================================================
#endif // CAMERAS_H_

