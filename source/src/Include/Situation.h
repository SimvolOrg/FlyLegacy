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

#include "FlyLegacy.h"
#include "../Include/database.h"
#include "UserVehicles.h"
#include "WorldObjects.h"
#include "Terrain.h"
#include "TimeManager.h"
#include "Cameras.h"
#include "FlyLegacy.h"
#include <vector>
#include <list> // sdk: CFlyObjectListManager SFlyObjectList


//=======================================================================
class CFuiRadioBand;
//=======================================================================
typedef enum  {

  NO_RND_EVENTS          = 0,
  RND_EVENTS_ENABLED     = (1 << 0),
  RAND_TURBULENCE        = (1 << 1)

} ERandomEvents;

/*! \class CRandomEvents CLASS manager
  
    This class maintains a serie of random events
    that occurrs during the timeslice situation
    and allows to trigger events wherever it 's needed
 */
class CRandomEvents
{
	bool                     dirty;                                       ///< oject state memo
	int                      rc;
  static CRandomEvents     instance;
  
  CRandomEvents            (void) {;}
	void Copy                (const CRandomEvents &aCopy);                ///< inactivated
	CRandomEvents            (const CRandomEvents &aCopy);                ///< {Copy(aCopy);} inactivated
	CRandomEvents& operator= (const CRandomEvents &aCopy);                ///< inactivated

public:
  CRandomizer random;
  
	///< construecteurs et destructeurs
  void     Init            (void);
  virtual ~CRandomEvents   (void);
  static   CRandomEvents& Instance (void) {return instance;}

	///< methods
  void Timeslice           (float dT,U_INT Frame);
  
  ///< set

	///< get
	
  ///< Clone
	CRandomEvents Clone(void);                                            ///< inactivated
};

/*!
  sdk: SFlyObjectList CLASS manager
  ----------------------------
  class used to maintain the list in CSituation
  and to provide a method that is linked to the SDK
  to retrive it
  #include <stdio.h>
  #include <string>
  #include <string.h>
 */
class CFlyObjectListManager
{
	bool dirty;	                                                          ///< oject state memo
	int  rc;
  void Init(void);
	
  void Copy(const CFlyObjectListManager &aCopy);                        ///< inactivated
	CFlyObjectListManager(const CFlyObjectListManager &aCopy);            ///< {Copy(aCopy);} inactivated
	CFlyObjectListManager& operator=(const CFlyObjectListManager &aCopy); ///< inactivated

public:
	///< construecteurs et destructeurs
  CFlyObjectListManager() {Init();}
  virtual ~CFlyObjectListManager(void);

  static CFlyObjectListManager* Instance (void);
  SFlyObjectList tmp_fly_object;                                        // temporary object
  std::list<SFlyObjectList> fo_list;
  std::list<SFlyObjectList>::iterator i_fo_list;
	///< set
	///< get
  void InsertUserInFirstPosition (const CVehicleObject *user);
  void InsertDLLObjInList (const SDLLObject *obj);

	///< Clone
	CFlyObjectListManager Clone(void);                                    // inactivated
};
//===========================================================================
//  Define SLEW mode
//===========================================================================
typedef enum {
  SLEW_STOP = 0,      // NO SLEW
  SLEW_MOVE = 1,      // Moving
  SLEW_LEVL = 2,      // Leveling
  SLEW_DAMP = 3,      // Damping slew
} SLEW_MODE;
//===========================================================================
//    class CSlewManager
//============================================================================
class CSlewManager {
public:
  // Constructor
  CSlewManager (void);
protected:
  void    SetLevel(CVehicleObject *user);
  bool    ZeroRate (void);
public:
  // CSlewManager methods
  void    Disable (void);
  void    Update (float dT);
  bool    Swap();
  bool    Reset();
  void    Level(char opt);
  void    StartSlew();
  void    StopSlew();
  void    SetAltitude(SPosition *p);
	void		NormalMove(float dT);
  //--------------------------------------------------------------
  void    BindKeys();
  //---------------------------------------------------------------
  inline void   IncAltRate(float d)   {aRate += d; grnd = 0;}
  inline void   DecAltRate(float d)   {aRate -= d; grnd = 0;}
  inline void   StateAs(int i)        {mode = (i == 0)?(SLEW_STOP):(SLEW_MOVE);}
  //---------------------------------------------------------------
  inline bool   IsEnabled()           {return (SLEW_STOP != mode);}
  inline bool   IsOn()                {return (SLEW_STOP != mode);}
  inline CVector *GetVelocity()       {return &velo;}
  //---------------------------------------------------------------
  bool   MoveForward();
  bool   MoveBackward();        
  bool   MoveLeft();            
  bool   MoveRight();
  bool   StopMove (); // stop any movement
	//----------------------------------------------------------------
protected:
  U_INT   vopt;     // Vehicle option
  char    mode;     // mode indicator
  char    call;     // Option call
  char    pmde;     // Previous mode
  char    grnd;     // ground option
  float   time;     // Stabilizator
  float   fRate;    // Forward motion
  float   lRate;    // Lateral motion
  float   hRate;    // heading motion
  float   aRate;    // Altitude rate in feet/sec
  CVehicleObject *veh;
  CVector velo;     // Incremental velocity (m/sec)
};

//===========================================================================================
//  Class CDLLWindow to display a window from a DLL plugin
//  NOTE: This class is instanciated in global situation, but the Draw method is called
//  in the context of CFuiManager as it is visible in every view
//===========================================================================================
class CDLLWindow {
  //---------------Internal state ------------------------------------------------
  //--------------Attribute ------------------------------------------------------
  long signature;
  SDLLObject *obj;
  SSurface   *surf;                                           ///< Surface
  short       wd;                                             ///< wide
  short       ht;                                             ///< Height
  U_INT       back;                                           ///< Back color
  U_INT       black;                                          ///< Black color
  //---------For time Management -------------------------------------------------
  //---------Editied fields ------------------------------------------------------
  //---------For editing  --------------------------------------------------------
  //-------------Method ----------------------------------------------------------
public:
  bool enabled;
  void *dll;
  CDLLWindow  ();
  ~CDLLWindow ();

  void  SetObject (SDLLObject *object);
  void  SetSignature (const long &sig);
  const SDLLObject* Get_Object (void) {return obj;}
  const long& GetSignature (void) {return signature;}
  //-----------------------Edit --------------------------------------------------
  //------------------------Size parameters --------------------------------------
  void  Resize    (void);
  //------------------------Drawing method ---------------------------------------
  void  Draw      (void);
  //-----------------------Time slice --------------------------------------------
  void  TimeSlice (float dT);
};
//==========================================================================================
//  Global situation
//==========================================================================================
class CSituation : public CStreamObject {
  //-----------------------------------------------------------
public:
  /// sdk: SFlyObjectList
  /// \todo link this list with wobjList
  CFlyObjectListManager sdk_flyobject_list;
  CVehicleObject      *uVeh;    ///< Reference to user vehicle object
  CSimulatedObject    *sVeh;    ///< Reference to simulated objects
  CDLLSimulatedObject *dVeh;    ///< Reference to Dll       objects
  //-----------------------------------------------------------
  std::vector     <CDLLWindow *> dllW;
  std::vector     <CDLLWindow *>::iterator idllW;
	//-----------------------------------------------------------
protected:
	U_INT           FrameNo;
  float           dTime;            // Delta time
  //---- Methods --------------------------------------
public:
   CSituation                          (void);
  ~CSituation                          (void);
   void FreeDLLWindows                 (void);

   
  //----- CStreamObject methods-----------------------
  virtual int   Read                   (SStream *stream, Tag tag);
  virtual void  ReadFinished           (void);

  //----- CSituation methods---------------------------
  void              OpenSitFile        (void);
  void              AdjustCameras      (void);
	void							ReloadAircraft();
  CAirplane  *GetAnAircraft            (void);
  CSimulatedObject *GetASimulated      (void);
  void              StoreVEH           (CVehicleObject *veh);
  void              SetAircraftFrom    (char *nfo);
  void              SetPosition        (SPosition &pos);
  void              PrintUserVehicle   (FILE *f);
  void              Prepare            (void);
	//----------------------------------------------------
  void              Timeslice          (float dT,U_INT Frame);
  void              Draw               (void);
  void              DrawExternal       (void);
  void              DrawVehicleFeatures(void);
  void              ClearUserVehicle   (void);
  void              ResetUserVehicle   (void);
  void              OpalGlobalsClean   (void);
  void              ChangeUserVehicle  (char * name, bool bPlane);
  U_INT             GetFrameNo         (void) {return FrameNo; }
  void              DrawDLLWindow      (void);
  //----------------------------------------------------
  inline CVehicleObject*   GetUserVehicle() {return uVeh;}
  //----------------------------------------------------

};


//=================================END OF FILE ==============================================
#endif // SITUATION_H
