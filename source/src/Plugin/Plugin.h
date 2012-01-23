/*
 * Plugin.h
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

/*! \file Plugin.h
 *  \brief Enable dll plugin architecture
 *
 *      In addition to the headers Sdk.h, dllintf.h and sdkintf.h you need to 
 *      provide a proper sdklib.lib buit in a separated project.
 *      Example of usage :

	  #include "Plugin.h"

        ut::CPluginMain plgn;

        void main(void)
        {
        //  ut::InitSdkIntf ();

          ut::fake = 1;
            if (plgn.On_LoadPlugins ()) {

              plgn.On_InitPlugins ();
              plgn.On_StartSituation ();
              //////////////////////
            int counter = 3;
            while (counter) {
              --counter;
                  plgn.On_Idle ();
                }
              ///////////////////
              plgn.On_KillPlugins ();
            }

          system("PAUSE");
}
 */

#if !defined(__PLUGIN_HEADER__)
#define __PLUGIN_HEADER__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// TODO: reference additional headers your program requires here
#include "..\Include\DLL.h"
#include <plib\ul.h>
#include "sdkintf.h"
#include <vector>
#include <list>

static SDLLCopyright copyright;
static SDLLRegisterTypeList *types [3];
static SDLLObject** plg_object = NULL;
static int dllobjects_counter = 0;
//static int types_counter = 0;



namespace ut {


  extern int fake;
	}

	//=======================================================================
/*!
  LC: sdk: SFlyObjectList CLASS manager
  ----------------------------
  class used to maintain the list in CSituation
  and to provide a method that is linked to the SDK
  to retrive it
  #include <stdio.h>
  #include <string>
  #include <string.h>
 */
 //=======================================================================

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
//======================================================================
//	JS:  Put all plugin code here and remove it from CSituation
//=====================================================================
//=====================================================================

  class CPluginMain: public CExecutable
  {	//--- ATTRIBUTES -------------------------------------------
	public:
		//-----------------------------------------------------------
		CFlyObjectListManager sdk_flyobject_list;
		CDLLSimulatedObject *dVeh;
		//--- METHODS ----------------------------------------------
  public:
    bool g_plugin_allowed;          ///< plugin flag from ini file
    std::vector <std::string> dll_export_listing;

    CPluginMain             (void);
    virtual ~CPluginMain    (void);

    // manadatory functions
    int  On_LoadPlugins     (void) const;
    void On_InitPlugins     (void) const;
    void On_KillPlugins     (void) const;

    // service
    int  CountPlugin        (void) const;
    void* IsDLL             (const long) const;
    SDLLObject* GetDLLObject(const long) const;

    // exported list of dls
    void BuildExportList   (void);

    // utility functions
    void On_StartSituation  (void) const;
    void On_EndSituation    (void) const;

	void On_DestroyObject   (SDLLObject*, void *dll = 0) const;		    // DLLDestroyObject;
	void On_ReceiveMessage  (SDLLObject*,SMessage*) const;		    // DLLReceiveMessage;
	void On_Instantiate     (const long,const long,SDLLObject**) const;		        // DLLInstantiate;
	//void On_Link_DLLGauges  (const long,const long,SDLLObject**) const;		        // DLLInstantiate;
	void On_Link_DLLSystems (const long,const long,SDLLObject**) const;		        // DLLInstantiate;
  void On_DeleteObjects   (void) const;
  void On_DeleteAllObjects(void) const;
  void On_Instantiate_DLLGauges (const long,const long,SDLLObject**) const;		        // DLLInstantiate;
  void On_Instantiate_DLLSystems (const long,const long,SDLLObject**) const;		        // DLLInstantiate;
  void On_Instantiate_DLLCamera (const long,const long,SDLLObject**) const;		        // DLLInstantiate;
	void On_InitObject      (SDLLObject*) const;		        // DLLInitObject;
  void On_Idle            (float dT) const;
	void On_Draw            (SDLLObject*,SSurface*, void *dll = 0) const;			        // DLLDraw;
	int  On_Read            (SDLLObject*,SStream*,unsigned int, void *dll = 0) const;			        // DLLRead;
	void On_ReadFinished    (SDLLObject*) const;		    // DLLReadFinished;
	void On_Write           (SDLLObject*,SStream*) const;		        	// DLLWrite;
	void On_MouseClick      (SDLLObject*,SSurface*,int,int,int) const;		        // DLLMouseClick;
	void On_TrackClick      (SDLLObject*,SSurface*,int,int,int) const;		        // DLLTrackClick;
	void On_StopClick       (SDLLObject*,SSurface*,int,int,int) const;		        // DLLStopClick;
	void On_MouseMoved      (SDLLObject*,SSurface*,int,int) const;		        // DLLMouseMoved;
	void On_Prepare         (SDLLObject*) const;			        // DLLPrepare;
	void On_TimeSlice       (SDLLObject*,const float,void *dll = 0) const;		        // DLLTimeSlice;
	void On_KeyPressed      (int,int) const;		        // DLLKeyPressed;
    void On_KeyIntercept    (int,int) const;		        // DLLKeyIntercept;
	void On_KeyCallback     (int,int,SDLLObject*) const;		        // DLLKeyCallback;
	void On_KeyCallback2     (int,int,int,SDLLObject*) const;		        // DLLKeyCallback2;
    void On_ProcessSoundEffects (SDLLObject*) const;	    // DLLProcessSoundEffects;
	void On_SuspendSoundEffects (SDLLObject*) const;	    // DLLSuspendSoundEffects;
	void On_ResumeSoundEffects (SDLLObject*) const;	    // DLLResumeSoundEffects;
	void On_InWindowRegion  (SDLLObject*,SSurface*,int,int) const;		    // DLLInWindowRegion;
	void On_GetPopupHelp    (SDLLObject*,char*,char*,char*) const;		    // DLLGetPopupHelp;
	void On_UpdateCamera    (SDLLObject*,SPosition*,SVector*,SPosition*,SVector*,const float, void *dll = 0) const;		    // DLLUpdateCamera;
	void On_IsInteriorCamera (SDLLObject*) const;		// DLLIsInteriorCamera;
	void On_GetCameraName   (SDLLObject*,char*,int) const;		    // DLLGetCameraName;
	void On_DrawOverlay     (SSurface*,SDLLObject*,int,int,int,int) const;		        // DLLDrawOverlay;
	void On_ReadIniFile     (void) const;		        // DLLReadIniFile;
	void On_WriteIniFile    (void) const;		    // DLLWriteIniFile;
    void On_ControlCanBeUsed (void) const;		// DLLControlCanBeUsed;
    void On_ControlButtonCount (void) const;	    // DLLControlButtonCount;
    void On_ControlButtonPressed (int) const;	// DLLControlButtonPressed;
    void On_ControlButtonName (int,char*,char*) const;	    // DLLControlButtonName;
    void On_ControlDebounceButton (int) const;	// DLLControlDebounceButton;
    void On_ControlHasAxis  (EJoystickAxis axis) const;		    // DLLControlHasAxis;
    void On_ControlGetAxis  (EJoystickAxis axis) const;		    // DLLControlGetAxis;
    void On_ControlGetIdentifier (void) const;	// DLLControlGetIdentifier;
    void On_RunScript       (const char *scriptFile, const char *refName, const char runOnce) const;		        // DLLRunScript;
    void On_CallScriptFunction (const char *scriptFile, const char *funcName) const;	    // DLLCallScriptFunction;
    void On_StopScript      (const char *scriptFile) const;		        // DLLStopScript;
    void On_StopScriptByRefName (const char *refName) const;	    // DLLStopScriptByRefName;
    void On_GetScriptInfo   (int *numScripts, SScriptInfo **array) const;		    // DLLGetScriptInfo;
    void On_GetPythonMethodTable (void) const;	// DLLGetPythonMethodTable;
    void On_BeginFrame      (void) const;		        // DLLBeginFrame;
    void On_EventNotice     (SDLLObject*,unsigned long,unsigned long,unsigned long,unsigned long) const;		        // DLLEventNotice;
    void On_EpicEvent       (unsigned char, unsigned char[6]) const;		        // DLLEpicEvent;
    void On_GenerateTileScenery (SPosition*,SPosition*,int,int,int,int,int,int) const;	    // DLLGenerateTileScenery;
    void On_FilterMetar     (char *metarFilename) const;		        // DLLFilterMetar;
    int  On_InitGlobalMenus (void) const;		    // DLLInitGlobalMenus;
		//-----------------------------------------------------------------------------------
		int		TimeSlice(float dT,U_INT frame);
		void	DrawExternal();
		void	DrawDLLWindow ();
		void	FreeDLLWindows();
  //  void InitSdkIntf (void);
  };



#endif // __PLUGIN_HEADER__

