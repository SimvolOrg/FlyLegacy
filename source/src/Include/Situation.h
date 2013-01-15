/*
 * Situation.h
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

/*! \file Situation.h
 *  \brief Defines CSituation encapsulation of overall simulator state
 *
 * The CSituation object represents the "top-level" of the simulation state.
 *   It contains references to the current user aircraft, camera, etc.
 */


#ifndef SITUATION_H
#define SITUATION_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#include <plib/pu.h>
#include "../Include/Globals.h"
#include "../Include/WorldObjects.h"
#include "../Include/database.h"
#include "../Include/Queues.h"
#include "../Include/3DMath.h"
#include "../Include/UserVehicles.h"
#include "../Include/Terrain.h"
#include "../Include/TimeManager.h"
#include "../Include/Cameras.h"
#include <vector>
#include <list> // sdk: CFlyObjectListManager SFlyObjectList


//=======================================================================
class CFuiRadioBand;
class CSimulatedObject;
//=======================================================================
//=======================================================================
typedef enum  {

  NO_RND_EVENTS          = 0,
  EVN_RAND_TURBULENCE        = (1 << 1)

} ERandomEvents;
//===========================================================================
//    class CSlewManager
//============================================================================
class CSlewManager: public CExecutable {
public:
  // Constructor
  CSlewManager (void);
 ~CSlewManager();
protected:
  void    SetLevel(CVehicleObject *user);
  bool    ZeroRate (void);
public:
  // CSlewManager methods
  void    Disable (void);
  int			TimeSlice(float dT,U_INT f);
  bool    Swap();
  bool    Reset();
  void    Level(char opt);
  void    StartMode(CAMERA_CTX *ctx);
	void		StartSlew();
	void		SetSlew();
  void    StopSlew();
  void    SetAltitude(SPosition *p);
	//--- Moving ---------------------------------------------------
	void		NormalMove(float dT);
	void		RabbitMove(float dT);
	void		RabbitMove(double x,double y, double z);
  //--------------------------------------------------------------
  void    BindKeys();
  //---------------------------------------------------------------
  inline void   StateAs(int i)        {mode = (i == 0)?(SLEW_STOP):(SLEW_MOVE);}
  //---------------------------------------------------------------
  inline bool   IsEnabled()           {return (SLEW_STOP != mode);}
  inline bool   IsOn()                {return (SLEW_STOP != mode);}
  //---Moving management  -----------------------------------------
  void   MoveOnZ(float d);
  bool   MoveOnY(float d);
  bool   MoveOnX(float d);            
  bool   StopMove (); // stop any movement
	//----------------------------------------------------------------
protected:
	CFPlan *flpn;			// Flight plan
  U_INT   vopt;     // Vehicle option
  char    mode;     // mode indicator
  char    call;     // Option call
  char    pmde;     // Previous mode
  char    grnd;     // ground option
  float   time;     // Stabilizator
  float   fRate;    // Forward motion
  float   lRate;    // Lateral motion
  float   aRate;    // Altitude rate in feet/sec
	//--- Aircraft ---------------------------------------------------
  CAirplane *mveh;
};
//==========================================================================================
//	SItuation States
//==========================================================================================
#define SIT_NORMAL			(0)
#define SIT_TELEP01			(1)
#define SIT_TELEP02			(2)
#define SIT_TELEP03			(3)
//==========================================================================================
//  Animated aircrafts:  
//==========================================================================================
class CAnimatedPlane: public CExecutable {
protected:
	//--- Attributes ------------------------------
	std::vector<CAirplane*> plnQ;				
public:
	//--- Methods ---------------------------------
	CAnimatedPlane();
  virtual ~CAnimatedPlane();
	//----------------------------------------------
	void  AddPlane(CAirplane* p);
	void	Clear();
	void	BackToSimulation(char grn);
	void	UpdatePosition(SPosition pos);
	//----------------------------------------------
	int	 TimeSlice(float dT,U_INT frame);
	//----------------------------------------------
	void  DrawExternal();
};
//==========================================================================================
//  Global situation
//==========================================================================================
class CSituation : public CStreamObject {
friend class CSlewManager;
	//-----------------------------------------------------------
protected:
	U_INT						State;						// Current state
	U_INT           FrameNo;					// Current frame
  float           dTime;            // Delta time
	//--- Animated planes -------------------------------
	CAnimatedPlane	planes;						// List of animated planes				
	//--- Teleport control ------------------------------
	double          ang;
	int 						wait;							// Wait timer
	CAMERA_CTX			contx;						// Camera context
	CRabbitCamera   *rcam;						// Rabbit camera
	//---------------------------------------------------
	CSimulatedObject *sVeh;						// Object simulated
  //---- Methods --------------------------------------
public:
   CSituation                          (void);
  ~CSituation                          (void);
  
  //----- CStreamObject methods-----------------------
  int   Read                   (SStream *stream, Tag tag);
  void  ReadFinished           (void);
  //----- CSituation methods---------------------------
  void              OpenSitFile        (void);
  void              AdjustCameras      (void);
	void							Reload();
  CAirplane        *GetAnAircraft      ();
  CSimulatedObject *GetASimulated      ();
  void              SetAircraftFrom    (char *nfo);
  void              PrintUserVehicle   (FILE *f);
  void              Prepare            (void);
	//----------------------------------------------------
	void							WriteFile();
	//----------------------------------------------------
  void              Timeslice (float dT,U_INT Frame);
	//----------------------------------------------------
	void							DrawNormal();
  void              Draw();
  void              DrawExternal();
  void              DrawVehicleFeatures();
	//-----------------------------------------------------
	void							TeleportS1();
	void							TeleportS2();
	void							TeleportS3();
	//-----------------------------------------------------
  void              ClearUserVehicle();
  void              OpalGlobalsClean();
  void              ChangeUserVehicle  (char * name, bool bPlane);
  U_INT             GetFrameNo         (void) {return FrameNo; }
	void							WorldOrigin();
  //----------------------------------------------------
	void							EnterTeleport(SPosition *P, SVector *O);
	void							ShortTeleport(SPosition *P, SVector *O);
	//-------------------------------------------------------
	void							AddPlane(CAirplane *p); 
	void							BackToSimulation(char g) {planes.BackToSimulation(g);}
};

//=================================END OF FILE ==============================================
#endif // SITUATION_H
