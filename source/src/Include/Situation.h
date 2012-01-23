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
//=======================================================================
typedef enum  {

  NO_RND_EVENTS          = 0,
  RAND_TURBULENCE        = (1 << 1)

} ERandomEvents;
//=======================================================================
/*! LC: \class CRandomEvents CLASS manager
  
    This class maintains a serie of random events
    that occurrs during the timeslice situation
    and allows to trigger events wherever it 's needed
 */
 //=======================================================================
class CRandomEvents
{	char on;
	bool                     dirty;                                       ///< oject state memo
	int                      rc;
  CRandomizer random;
	//--- METHODS -------------------------------------------
	void Copy                (const CRandomEvents &aCopy);                ///< inactivated
	CRandomEvents            (const CRandomEvents &aCopy);                ///< {Copy(aCopy);} inactivated
	CRandomEvents& operator= (const CRandomEvents &aCopy);                ///< inactivated
	//-------------------------------------------------------
public:
  CRandomEvents(void);
	///< constructeurs et destructeurs
  virtual ~CRandomEvents   (void);

	///< methods
  void Timeslice           (float dT,U_INT Frame);
  
  ///< set

	///< get
	
  ///< Clone
	CRandomEvents Clone(void);                                            ///< inactivated
};
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
  CVehicleObject *veh;
};

//==========================================================================================
//  Global situation
//==========================================================================================
class CSituation : public CStreamObject {
  //-----------------------------------------------------------
public:
  /// sdk: SFlyObjectList
  /// \todo link this list with wobjList
  CVehicleObject      *uVeh;    ///< Reference to user vehicle object
  CSimulatedObject    *sVeh;    ///< Reference to simulated objects
	//-----------------------------------------------------------
protected:
	U_INT           FrameNo;
  float           dTime;            // Delta time
	CRandomEvents     rEVN;						// Event generator 
  //---- Methods --------------------------------------
public:
   CSituation                          (void);
  ~CSituation                          (void);
  
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
  void              OpalGlobalsClean   (void);
  void              ChangeUserVehicle  (char * name, bool bPlane);
  U_INT             GetFrameNo         (void) {return FrameNo; }
	void							WorldOrigin();
  //----------------------------------------------------
  inline CVehicleObject*   GetUserVehicle() {return uVeh;}
  //----------------------------------------------------

};


//=================================END OF FILE ==============================================
#endif // SITUATION_H
