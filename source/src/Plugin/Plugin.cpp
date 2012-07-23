// Plugin.cpp: implementation of the CPlugin class.
//
//////////////////////////////////////////////////////////////////////



#include "Plugin.h"
#include "dllintf.h"
#include "..\include\utility.h"
#include "..\include\globals.h"
#include "../Include/MagneticModel.h"
#include "../Include/GeoMath.h"
#include "../Include/DLL.h"

//====================================================================
#include <vector>
using namespace std;
using namespace ut;


///////// DEBUG STUFF TO REMOVE LATER ////////////////////////////////
#ifdef  _DEBUG
  //#define _DEBUG_dll       // print lc DDEBUG file     ... remove later
  //#define _DEBUG_dll2      // print lc DDEBUG file     ... remove later 
  //#define _DEBUG_DLLGAUGES // print lc DDEBUG file     ... remove later
  //#define _DEBUG_DLLlist   // print lc DDEBUG file     ... remove later 
#endif
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Global Vectors
//////////////////////////////////////////////////////////////////////
std::vector <HMODULE> m_dwa;
std::vector <HMODULE>::iterator it;

//std::vector <DLLFunctionRegistry *> m_dll;
//std::vector <DLLFunctionRegistry *>::iterator itdll;

typedef struct SDLLData {
  bool enabled;
  SDLLCopyright cpyrt;
  DLLFunctionRegistry *dll_registry;
  std::map <SDLLRegisterTypeList*, SDLLObject*> typ_obj;
  std::map <SDLLRegisterTypeList*, SDLLObject*>::iterator ityp_obj;
  SDLLData (void) {
    enabled = false;
    dll_registry = NULL;
    memset (&cpyrt, 0, sizeof (SDLLCopyright));
  }
} SDLLData;

std::vector <SDLLData> dll_data;
std::vector <SDLLData>::iterator idll_data;
std::vector<CDLLWindow*> wLst;

static bool flag_instantiate = false;
static int dll_gauges_init_counter = 0;
static int dll_windows_init_counter = 0;
static int dll_system_init_counter = 0;
static int dll_camera_init_counter = 0;
//////////////////////////////////////////////////////////////////////
// initialisation
//////////////////////////////////////////////////////////////////////
int ut::fake = 0;
// simple Registered Types list pointers
static SDLLRegisterTypeList *deb  = NULL;  // start
static SDLLRegisterTypeList *pac  = NULL;  // current
static SDLLRegisterTypeList *bkup = NULL;  // backup

//////////////////////////////////////////////////////////////////////
// General Declarations
//////////////////////////////////////////////////////////////////////
//typedef int (*PFUNC1)(void);        // test
//typedef int (*PFUNC2)(int &val);    // test
typedef int (*PFUNC3)(DLLFunctionRegistry *dll, SDLLCopyright *copyright, SDLLRegisterTypeList **types);
typedef int (*PFUNC4)(void);

//////////////////////////////////////////////////////////////////////
// Utility functions
//////////////////////////////////////////////////////////////////////
//void GetfnPlug1 (const std::vector <HMODULE>::iterator it_);
//void GetfnPlug2 (const std::vector <HMODULE>::iterator it_);
void GetDLLInit (const std::vector <HMODULE>::iterator it_);
void GetDLLKill (const std::vector <HMODULE>::iterator it_);


//////////////////////////////////////////////////////////////////////
// SDLLObject manager
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
void UtilityPrintDLLData (void)
{
  FILE *fp_debug;
  if(!(fp_debug = fopen("__DDEBUG_testDLLlist.txt", "a")) == NULL)
  {
    for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {

      for ((*idll_data).ityp_obj = (*idll_data).typ_obj.begin (); (*idll_data).ityp_obj != (*idll_data).typ_obj.end (); ++(*idll_data).ityp_obj) {
        char buf1 [8] = {0}, buf2 [8] = {0};
        TagToString (buf1, (*idll_data).ityp_obj->first->type); 
        TagToString (buf2, (*idll_data).ityp_obj->first->signature); 
        fprintf(fp_debug, "%s %s %p (%d)\n",
          buf1, buf2, (*idll_data).ityp_obj->second, (*idll_data).enabled);
      }
    }
    fprintf(fp_debug, "-----------------------------------------------------\n");
    fclose(fp_debug); 
  }
}
#endif

void UtilityAddSDLLObject (void)
{
  if ((plg_object = (SDLLObject **) realloc (plg_object, (dllobjects_counter + 1) * sizeof (SDLLObject *))) == NULL) exit (1);
  if (dllobjects_counter) {
    //MEMORY_LEAK_MARKER ("CDLLWindow 4");
    //if ((plg_object[dllobjects_counter] = (SDLLObject *) malloc (sizeof (SDLLObject))) == NULL) exit (1);
    //MEMORY_LEAK_MARKER ("CDLLWindow 4");
    plg_object[dllobjects_counter] = plg_object[0];
    plg_object[0] = NULL;
  }
  #ifdef _DEBUG_dll2	
    FILE *fp_debug;
    if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
    {
      fprintf(fp_debug, "CPluginMain::On_Inst Add plg_object %d %p %p %p\n",
        dllobjects_counter, plg_object, plg_object[0], plg_object[dllobjects_counter]);
      fclose(fp_debug);
    }
  #endif
  dllobjects_counter++;
}

#ifdef _DEBUG
void UtilityPrintDLLObject (void)
{
    FILE *fp_debug;
    if(!(fp_debug = fopen("__DDEBUG_testDLLlist.txt", "a")) == NULL)
    {
      fprintf(fp_debug, "plg_object [%d %p] ",
        dllobjects_counter, plg_object);
      for (int i = 0; i < dllobjects_counter; ++i)
        fprintf(fp_debug, "[%d]=%p ", i, plg_object[i]);
      fprintf(fp_debug, "\n");
      fclose(fp_debug);
    }
}
#endif

void FreeInitialDLLObjectList (void)
{ 
  int i = 0;
  for (i; i < dllobjects_counter; ++i) {
    #ifdef _DEBUG
      FILE *fp_debug;
      if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
      {
        fprintf(fp_debug, "%d %d %p\n", dllobjects_counter, i, plg_object[i]);
        fclose(fp_debug);
      }
    #endif
    SAFE_FREE (plg_object[i]);
  }
  dllobjects_counter -= i - 1;
  //SAFE_FREE (plg_object);
  #ifdef _DEBUG
    FILE *fp_debug;
    if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
    {
      fprintf(fp_debug, "FreeInitialDLLObjectList : left %p\n", plg_object);
      fclose(fp_debug);
    }
  #endif
}
///=====================================================================================
/// sdk: CFlyObjectListManager
///=====================================================================================
void CFlyObjectListManager::Init (void)
{
  rc = 0;

  tmp_fly_object.ref.objectPtr = NULL;
  tmp_fly_object.ref.classSig  = NULL;
  tmp_fly_object.ref.superSig  = NULL;
  tmp_fly_object.next          = NULL;
  tmp_fly_object.prev          = NULL;

  fo_list.clear ();

  // Allocate first struct
  fo_list.push_back (tmp_fly_object);
  dirty = false;
  rc++;
}

CFlyObjectListManager::~CFlyObjectListManager (void) 
{
  // dll are responsible for objects deletion in this list
  // except for the first one which is the user object;
/*/
  i_fo_list = fo_list.begin ();
  ++i_fo_list;                           // skip first user object
  for (i_fo_list; i_fo_list != fo_list.end (); ++i_fo_list) {
     SAFE_DELETE (i_fo_list->ref.objectPtr);          // 
  }
/*/
}

void CFlyObjectListManager::InsertUserInFirstPosition (const CVehicleObject *user)
{
  if (rc) {
    // (*it).ref.objectPtr
    std::list<SFlyObjectList>::iterator it = fo_list.begin ();
    (*it).ref.objectPtr = (CVehicleObject *) user;
    (*it).ref.classSig  = NULL;
    (*it).ref.superSig  = NULL;
  }
}

void CFlyObjectListManager::InsertDLLObjInList (const SDLLObject *obj)
{
  if (rc) {
    tmp_fly_object.ref = obj->flyObject;
    tmp_fly_object.next = NULL;
    tmp_fly_object.prev = NULL;
    fo_list.push_back (tmp_fly_object);
    rc++;
  }
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPluginMain::CPluginMain()
{	g_plugin_allowed = false;
}

CPluginMain::~CPluginMain()
{	FreeDLLWindows();

}
//----------------------------------------------------------------
//	Plugin to load
//----------------------------------------------------------------
int CPluginMain::On_LoadPlugins (void) const
{			  CExecutable *exe = (CExecutable*)this;
        globals->Disp.Enter(exe, PRIO_DLL, DISP_EXCONT,  0);

        m_dwa.clear ();
        //m_dll.clear ();
        dll_data.clear ();


        static int level=-1;
        level++;
  
        const char *pc_path = "./Modules/PC";
        ulDir* dirp = ulOpenDir(pc_path);
        if ( dirp != NULL )
        {
          ulDirEnt* dp;
          while ( (dp = ulReadDir(dirp)) != NULL )
          {
            char *p = strrchr (dp->d_name, '.');
            if (p && (!stricmp (p, ".dll") || !stricmp (p, ".plx")))
            {
              char path[ UL_NAME_MAX+1 ] = {".\\modules\\PC\\\0"};
//              strcat (path, dirname);
//              strcat (path, "\\");
              strcat (path, dp->d_name);
              HMODULE hm = LoadLibrary (path);
              DEBUGLOG ("Load Library %s", path);
              if (hm) {
              //MEMORY_LEAK_MARKER ("m_dwa");
                m_dwa.push_back (hm);
              //MEMORY_LEAK_MARKER ("m_dwa");
                #ifdef _DEBUG_dll
	                FILE *fp_debug;
	                if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	                {
		                fprintf(fp_debug, "Loaded %s = %d\n", path, hm);
		                fclose(fp_debug); 
	                }
                    printf ("Loaded %s = %d\n", path, hm);
                #endif
              } else {
                #ifdef _DEBUG_dll	
	                FILE *fp_debug;
	                if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	                {
		                fprintf(fp_debug, "Couldn't load %s\n", path);
		                fclose(fp_debug); 
	                }
                    printf ("Couldn't load %s\n", path);
                #endif
              }
            }
          }
          ulCloseDir(dirp);
        }
        level--;

        #ifdef _DEBUG_dll	
	        FILE *fp_debug;
	        if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	        {
		        fprintf(fp_debug, "-- %d  ----------------------------------------------\n", m_dwa.size ());
		        fclose(fp_debug); 
	        }
            printf ("-- %d  ----------------------------------------------\n", m_dwa.size ());
        #endif
        return (m_dwa.size ());
}

void CPluginMain::On_InitPlugins (void) const
{
#ifdef _DEBUG
  DEBUGLOG ("CPluginMain::On_InitPlugins size %d", m_dwa.size ());
#endif

  for (it = m_dwa.begin (); it != m_dwa.end (); ++it) {
    //MEMORY_LEAK_MARKER ("On_InitPlugins");
    GetDLLInit (it);
    //MEMORY_LEAK_MARKER ("On_InitPlugins");
    #ifdef _DEBUG_dll	
	    FILE *fp_debug;
	    if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	    {
		    fprintf(fp_debug, "------------------------------------------------\n");
		    fclose(fp_debug); 
	    }
    #endif
  }

  // stop the linked list
  //types [1]->type = static_cast<EDLLObjectType> (NULL);
  //types [2]->next = NULL;
  //// free the last pointer created in the linked chain
  //if (types [1]->next) {
  //  #ifdef _DEBUG_dll2	
  //    FILE *fp_debug;
  //    if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
  //    {
  //      fprintf(fp_debug, "           Delete RegisteredType %p \n", types [1]);
  //      fclose(fp_debug);
  //    }
  //  #endif
  //  SAFE_DELETE (types[1]);
  //}

#ifdef _DEBUG_DLLlist	
  //UtilityPrintDLLData ();
#endif
}

void CPluginMain::On_KillPlugins (void) const
{
#ifdef _DEBUG
  DEBUGLOG ("CPluginMain::On_KillPlugins %d", globals->plugins_num);
#endif
#ifdef _DEBUG	
  FILE *fp_debug;
  if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
  {
    fprintf(fp_debug, "------------------------------------------------\n");
    fclose(fp_debug); 
  }
#endif
  //
  for (it = m_dwa.begin (), idll_data = dll_data.begin ();
       it != m_dwa.end (), idll_data != dll_data.end ();
       ++it, ++idll_data)
  {
    GetDLLKill (it);
    FreeLibrary ((HMODULE)(*it));
  }
  //for (itdll = m_dll.begin (); itdll != m_dll.end (); ++itdll) {
  //  FILE *fp_debug;
  //  if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
  //  {
  //    fprintf(fp_debug, "CPluginMain::On_KillPlugins FREE (m_dll) %p\n", *itdll);
  //    fclose(fp_debug);
  //  }
  //  SAFE_DELETE (*itdll);
  //}
  for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {
    FILE *fp_debug;
    if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
    {
      fprintf(fp_debug, "CPluginMain::On_KillPlugins FREE (dll_data) %p\n", (*idll_data).dll_registry);
      fclose(fp_debug);
    }
    SAFE_DELETE ((*idll_data).dll_registry);
    // lc 051510
    // force the liberation of the allocated structures
    (*idll_data).typ_obj.clear ();
    //
  }
  //
  globals->plugins_num = 0;
  // delete all pointers in dllW // 
  //
  pac = bkup = deb = NULL;  // 
  dllobjects_counter = 0;   // 
}

int CPluginMain::CountPlugin (void) const
{
  return (m_dwa.size ());
}

//************************************************************
//*                         Utilities                          *
//************************************************************
//*                                                          *
//* Created at  :  11/12/2006   16:22                        *
//* Author      :  Enter Your Name                           *
//* Description :                                            *
//* Parameters  :  dT           -                            *
//*                Unregistered - Limited Functionality.     *
//*                                                          *
//************************************************************

void CPluginMain::On_Idle (float dT) const
{
  //for (itdll = m_dll.begin (); itdll != m_dll.end (); ++itdll) {
  //  if ((*itdll)->DLLIdle)           (*itdll)->DLLIdle (dT);
  //}
  for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {
    if ((*idll_data).dll_registry->DLLIdle) (*idll_data).dll_registry->DLLIdle (dT);
  }
}

void CPluginMain::On_StartSituation (void) const
{
  for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {
    if ((*idll_data).dll_registry->DLLStartSituation){
      (*idll_data).dll_registry->DLLStartSituation ();
      //
      #ifdef _DEBUG_dll
      {	FILE *fp_debug;
	        if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
        {
	          fprintf(fp_debug, "StartSituation dll = %p\n", (*idll_data).dll_registry->DLLStartSituation);
	          fclose(fp_debug); 
      }   }
      #endif 
    }
  }
}

void CPluginMain::On_EndSituation (void) const
{
  for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {
    if ((*idll_data).dll_registry->DLLEndSituation) (*idll_data).dll_registry->DLLEndSituation ();
  }
}

/*! \brief This function is called when we switch aircraft
 *  to perform a complete deletion of spurious objects left behind
 *  On_DeleteAllObjects is basically used to relese memory when a dll
 *  can't be found for a GAUGE or any secondary dll object type
 *  (see CDLLGauge::ReadFinished)
 */
void CPluginMain::On_DeleteAllObjects (void) const
{ if (!flag_instantiate) return;
  std::map <SDLLRegisterTypeList*, SDLLObject*>::iterator it_tmp;
  for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {
    for ((*idll_data).ityp_obj = (*idll_data).typ_obj.begin (); (*idll_data).ityp_obj != (*idll_data).typ_obj.end (); ++(*idll_data).ityp_obj) {
        if ((*idll_data).ityp_obj->second) {
          it_tmp = (*idll_data).ityp_obj;
          #ifdef _DEBUG_DLLGAUGES
            char buf1 [8] = {0}, buf2 [8] = {0};
            TagToString (buf1, (*idll_data).ityp_obj->first->type); 
            TagToString (buf2, (*idll_data).ityp_obj->first->signature); 
            TRACE ("## ALL %s %s %p %p",
              buf1, buf2, (*idll_data).ityp_obj->second, (*idll_data).dll_registry);
          #endif
          On_DestroyObject ((*idll_data).ityp_obj->second, (*idll_data).dll_registry);
          (*idll_data).ityp_obj = it_tmp;
        }
    }
  }
}

void CPluginMain::On_DeleteObjects (void) const
{ if (!flag_instantiate) return;
  std::map <SDLLRegisterTypeList*, SDLLObject*>::iterator it_tmp;

  for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {
        //TRACE ("test 1");
    for ((*idll_data).ityp_obj = (*idll_data).typ_obj.begin (); (*idll_data).ityp_obj != (*idll_data).typ_obj.end (); ++(*idll_data).ityp_obj) {
        //TRACE ("test 2 %p", (*idll_data).ityp_obj->second);
      if ((*idll_data).ityp_obj->second && (*idll_data).ityp_obj->first->type == TYPE_DLL_GAUGE){
        it_tmp = (*idll_data).ityp_obj;
        //TRACE ("test 3 %p", (*idll_data).ityp_obj);
        #ifdef _DEBUG_DLLGAUGES
        char buf1 [8] = {0}, buf2 [8] = {0};
        TagToString (buf1, (*idll_data).ityp_obj->first->type); 
        TagToString (buf2, (*idll_data).ityp_obj->first->signature); 
        TRACE ("## %s %s %p", buf1, buf2, (*idll_data).ityp_obj->second);
        #endif
        On_DestroyObject ((*idll_data).ityp_obj->second, (*idll_data).dll_registry);
        (*idll_data).ityp_obj = it_tmp;
        //TRACE ("test 4 %p", (*idll_data).ityp_obj);
        //goto jump;
      }
    }
  }
//jump : 
  //TRACE ("test 5");
  //FreeInitialDLLObjectList ();

#ifdef _DEBUG_DLLGAUGES
  //UtilityPrintDLLData ();
#endif
}

void CPluginMain::On_DestroyObject (SDLLObject* obj, void *dll) const
{
  if (!flag_instantiate) return;
  //flag_instantiate = false; // 
  #ifdef _DEBUG_dll2	
    FILE *fp_debug;
    if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
    {
      char buff [8] = {0};
      TagToString (buff, static_cast<SDLLRegisterTypeList *> (obj->flyObject.superSig)->signature);
      fprintf(fp_debug, "CPluginMain::On_DestroyObject %p @ %s %p\n", obj, buff, dll);
      fclose(fp_debug);
    }
  #endif
#ifdef _DEBUG
  //DEBUGLOG ("CPluginMain::On_DestroyObject");
#endif
  // \todo create a SDLLObject list and call DLLDestroyObject with the right object signature
  //int k = 0;
  for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {
  //for (itdll = m_dll.begin (); itdll != m_dll.end (); ++itdll) {
//    if ((*itdll)->DLLDestroyObject) {
      for ((*idll_data).ityp_obj = (*idll_data).typ_obj.begin (); (*idll_data).ityp_obj != (*idll_data).typ_obj.end (); ++(*idll_data).ityp_obj) {
        //(*idll_data).ityp_obj->first->type); 
        //(*idll_data).ityp_obj->first->signature); 
      //for (types[1] = types[0]; types[1] != NULL; types[1] = types[1]->next) {
        //if (k < dllobjects_counter) 
          #ifdef _DEBUG_DLLGAUGES
            //char buff [8] = {0};
            //TagToString (buff, (*idll_data).ityp_obj->first->type);
            //TRACE ("FREE %s %p %p", buff, obj, (*idll_data).ityp_obj->second);
          #endif
          //if ('S__G' == types [1]->signature)
//          if (plg_object [k] && obj && plg_object [k] == obj && 
//            static_cast<SDLLRegisterTypeList *> (obj->flyObject.superSig)->signature == types [1]->signature) {
          //if (plg_object [k] && obj && plg_object [k] == obj) {
          if (obj == (*idll_data).ityp_obj->second /*|| NULL == obj*/) {//
//            (*itdll)->DLLDestroyObject (plg_object [k]); //
            //TRACE ("obj %p %p", obj, (*idll_data).ityp_obj->second);
            static_cast <DLLFunctionRegistry *> (dll)->DLLDestroyObject (obj); //
//            (*idll_data).dll_registry->DLLDestroyObject ((*idll_data).ityp_obj->second);//
            #ifdef _DEBUG_dll2	
              FILE *fp_debug;
//              char buff [8] = {0};
//              TagToString (buff, types [1]->signature);
              if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
              {
	              fprintf(fp_debug, "CPluginMain::On_DestroyObject plg_object[%d] %p %p\n",
                  0, (*idll_data).ityp_obj->second, obj);
	              fclose(fp_debug);
              }
            #endif
            //
            (*idll_data).enabled = false;
            //SDLLRegisterTypeList *test = static_cast<SDLLRegisterTypeList *> (obj->flyObject.superSig);
            if (TYPE_DLL_WINDOW == (*idll_data).ityp_obj->first->type) {
              dll_windows_init_counter--;
              #ifdef _DEBUG_DLLGAUGES
                  TRACE ("FREE DLL WINDOWS %d", dll_windows_init_counter);
              #endif
            } else
            if (TYPE_DLL_GAUGE == (*idll_data).ityp_obj->first->type) {
              dll_gauges_init_counter--;
              #ifdef _DEBUG_DLLGAUGES
                  TRACE ("FREE DLL GAUGES %d", dll_gauges_init_counter);
              #endif
            } else
            if (TYPE_DLL_SYSTEM == (*idll_data).ityp_obj->first->type) {
              dll_system_init_counter--;
              #ifdef _DEBUG_DLLGAUGES
                  TRACE ("FREE DLL SUBSYSTEM %d", dll_system_init_counter);
              #endif
            } else
            if (TYPE_DLL_VIEW == (*idll_data).ityp_obj->first->type) {
            } else
             if (TYPE_DLL_CAMERA == (*idll_data).ityp_obj->first->type) {
              dll_camera_init_counter--;
              #ifdef _DEBUG_DLLGAUGES
                  TRACE ("FREE DLL CAMERA %d", dll_camera_init_counter);
              #endif
            }
            //
            (*idll_data).ityp_obj->second = NULL;
            //SAFE_FREE ((*idll_data).ityp_obj->second);
            return;
          }//
        //} 
        //k++; 
      }
//    } // DLLDestroyObject
  }
/*/
  for (itdll = m_dll.begin (); itdll != m_dll.end (); ++itdll) {
    if (types [0]) {
    for (types[1] = types[0]; types[1] != NULL; types[1] = types[2]) {
      types[2] = types[1]->next;

        if (types[1]->type == TYPE_DLL_WINDOW){
          //if (dll_surf) {
            #ifdef _DEBUG_dll2
              {FILE *fp_debug;
              if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
              {
                fprintf(fp_debug, "           TYPE_DLL_WINDOW\n");
                fclose(fp_debug);
              }}
            #endif
          //  APIFreeSurface (dll_surf);
          //  dll_surf = NULL;
          //}
        } else
        if (types[1]->type == TYPE_DLL_GAUGE){
        } else
        if (types[1]->type == TYPE_DLL_SYSTEM){
        } else
        if (types[1]->type == TYPE_DLL_VIEW){
        }

        #ifdef _DEBUG_dll2	
          FILE *fp_debug;
          if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
          {
            fprintf(fp_debug, "           Delete RegisteredType %p %p %p\n",
              types [0], types [1], types [2]);
            fclose(fp_debug);
          }
        #endif
        SAFE_DELETE (types[1]);
//          }
      }
      types [0] = NULL; // deb = NULL
    } // TYPES

    if (flag_instantiate) {
      #ifdef _DEBUG_dll2
      if (plg_object) {
        FILE *fp_debug;
        if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
        {
          fprintf(fp_debug, "           FREE (plg_object) %p\n", plg_object);
          fclose(fp_debug);
        }
      }
      #endif
      SAFE_FREE (plg_object);
    } // plg_object
  }
/*/
} // OnDestroyObject;




void CPluginMain::On_ReceiveMessage  (SDLLObject*,SMessage*) const
{
}  		    // DLLReceiveMessage;

void CPluginMain::On_Instantiate (const long,const long,SDLLObject**) const
{
  //SDLLObject** plg_object = NULL;
  // \todo create a SDLLObject list and keep an index upon DLLDestroyObject with their object signature
  //for (itdll = m_dll.begin (); itdll != m_dll.end (); ++itdll) {
  for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {
    if ((*idll_data).dll_registry->DLLInstantiate) {
#ifdef _DEBUG
    //DEBUGLOG ("CPluginMain::On_Instantiate");
#endif
#ifdef _DEBUG_DLLGAUGES
    TRACE ("ON_INSTANTIATE");
#endif
      flag_instantiate = true;
      #ifdef _DEBUG_dll2	
	      //FILE *fp_debug;
	      //if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	      //{
		     // fprintf(fp_debug, "CPluginMain::On_Inst %p %p %p %p\n",
       //     types, types [0], types [1], types [2]);
		     // fclose(fp_debug);
	      //}
      #endif
      //for (types[1] = types[0]; types[1] != NULL; types[1] = types[1]->next) {
      for ((*idll_data).ityp_obj = (*idll_data).typ_obj.begin (); (*idll_data).ityp_obj != (*idll_data).typ_obj.end (); ++(*idll_data).ityp_obj) {
        //(*idll_data).ityp_obj->first->type); 
        //(*idll_data).ityp_obj->first->signature); 
//        if ((plg_object = (SDLLObject **) realloc (plg_object, (dllobjects_counter + 1) * sizeof (SDLLObject *))) == NULL) exit (1);
        UtilityAddSDLLObject ();
//        plg_object [dllobjects_counter] = NULL; // lc 110709
        //if (types[1]->type == TYPE_DLL_GAUGE){
        //  ///// traverse all the cockpits /////
        //  std::map<Tag,CPanel*>::const_iterator iter;
        //  Tag ckpt_tag = 0;
        //  int count_gauges = 0;
        //  for (iter = globals->pln->pit->GetMapCkpt ().begin();
        //       iter!= globals->pln->pit->GetMapCkpt ().end(); iter++) {
        //    ckpt_tag = (*iter).first;
        //    #ifdef _DEBUG_DLLGAUGES
        //    char pnl_buff [8] = {0};//'frnt'
        //    TagToString (pnl_buff, ckpt_tag);
        //    TRACE ("ckpt %s", pnl_buff);
        //    #endif
        //    count_gauges += globals->pln->pit->GetPanelByTag (ckpt_tag)->dll_gauge.size ();
        //  }
        //  if (count_gauges)
        //    (*itdll)->DLLInstantiate ( types[1]->type,
        //                               types[1]->signature,
        //                               plg_object); // workaround
        //} else
        #ifdef _DEBUG_dll2
          //TRACE ("type = %d", (*idll_data).ityp_obj->first->type);
        #endif
        if ((*idll_data).ityp_obj->first->type == TYPE_DLL_WINDOW){

        (*idll_data).dll_registry->DLLInstantiate ( (*idll_data).ityp_obj->first->type,
                                                    (*idll_data).ityp_obj->first->signature,
                                                    plg_object); // 

        #ifdef _DEBUG_dll2	
	        //FILE *fp_debug;
	        //if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	        //{
		       // fprintf(fp_debug, "CPluginMain plg_object %p\n", plg_object);
         //   for (int k = 0; k < dllobjects_counter; ++k) {
		       // fprintf(fp_debug, "CPluginMain plg_object %d       %p\n", k, plg_object[k]);
         //   }
		       // fclose(fp_debug);
	        //}
        #endif
        #ifdef _DEBUG_dll2
        //for (int i = 0; i < 3; ++i) {
        //  char buf1 [128] = {0};
        //  TagToString (buf1, types[i]->type);
        //  char buf2 [128] = {0}; 
        //  TagToString (buf2, types[i]->signature);
        //  {FILE *fp_debug;
	       // if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	       // {
		      //  fprintf(fp_debug, "CPluginMain::On_Inst %p %p %s %s\n", types, types[i], buf1, buf2);
		      //  fclose(fp_debug);
        //  }}
        //}
        #endif
        if (*plg_object) {
          //types_counter++;

          // set dll_data typ_obj SDLLObject
          (*idll_data).ityp_obj->second = plg_object [0];

          //if ((*idll_data).ityp_obj->first->type == TYPE_DLL_WINDOW){
            // create a new window
            //MEMORY_LEAK_MARKER ("windeb 1");
            CDLLWindow *wdl = new CDLLWindow();
            //MEMORY_LEAK_MARKER ("windeb 1");
            if (wdl) {
              plg_object [0]->flyObject.superSig = (*idll_data).ityp_obj->first;
              wdl->SetObject (plg_object [0]);
              wdl->SetSignature ((*idll_data).ityp_obj->first->signature);
              wdl->dll = (*idll_data).dll_registry;
              (*idll_data).enabled = true;
              wLst.push_back(wdl);
              dll_windows_init_counter++;
              #ifdef _DEBUG
                //TRACE ("DLL WINDOW INITIALIZED %d %p", dll_windows_init_counter, plg_object [0]);
              #endif
            }
  /*          
  */
            #ifdef _DEBUG_dll2
              {FILE *fp_debug;
              if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
              {
                char buff [8] = {0};
                TagToString (buff, (*idll_data).ityp_obj->first->signature);
	              fprintf(fp_debug, "  CPluginMain::On_Inst APICreateSurface %s\n", buff);
	              fclose(fp_debug);
              }}
            #endif
          }
        } else
          if ((*idll_data).ityp_obj->first->type == TYPE_DLL_GAUGE){
            plg_object [0] = 0; // 082711
            dll_gauges_init_counter++;
            #ifdef _DEBUG
              //TRACE ("DLL GAUGE INITIALIZED %d %p", dll_windows_init_counter, plg_object [0]);
            #endif
/*/ //122809
            ///// traverse all the cockpits /////
            std::map<Tag,CPanel*>::const_iterator iter;
            char pnl_buff [8] = {0};//'frnt'
            Tag ckpt_tag = 0;
            for (iter = globals->pln->pit->GetMapCkpt ().begin();
                 iter!= globals->pln->pit->GetMapCkpt ().end(); iter++) {
              ckpt_tag = (*iter).first;
              TagToString (pnl_buff, ckpt_tag);
              #ifdef _DEBUG
              //TRACE ("Try ckpt %s", pnl_buff);
              #endif
            //}
//
            /// search for a DLL gauge type to link with
            std::vector<CDLLGauge *>::iterator it =
              globals->pln->pit->GetPanelByTag (ckpt_tag)->dll_gauge.begin ();
            for (it; it != globals->pln->pit->GetPanelByTag (ckpt_tag)->dll_gauge.end (); ++it) {
              if ((*it)->GetSignature () == static_cast <const long> (types[1]->signature)) {
               break;
              }
            }
            char buff [8] = {0};
            TagToString (buff, types[1]->signature);
            if (it != globals->pln->pit->GetPanelByTag (ckpt_tag)->dll_gauge.end ()) {
              plg_object [0]->flyObject.superSig = types [1];
              (*it)->SetObject (plg_object [0]);
              (*it)->dll = *itdll;
              #ifdef _DEBUG_DLLGAUGES
              TRACE ("Found <%s> DLL GAUGE in '%s'", buff, pnl_buff);
              #endif
              #ifdef _DEBUG_dll2
                {FILE *fp_debug;
                if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
                {
                  char buff [8] = {0};
                  TagToString (buff, types[1]->signature);
                  fprintf(fp_debug, "  CPluginMain::On_Inst APICreateGauge %s in '%s'\n",
                    buff, pnl_buff);
	                fclose(fp_debug);
                }}
              #endif
            } else {
              #ifdef _DEBUG
              //TRACE ("Can't find <%s> DLL GAUGE in '%s'", buff, pnl_buff);
              #endif
            }
//
/*/
            //CDLLGauge *new_g = new CDLLGauge;
            //if (new_g) {
            //  plg_object [0]->flyObject.superSig = types [1];
            //  new_g->SetObject (plg_object [0]);
            //  new_g->SetSignature (types[1]->signature);
            //  new_g->dll = *itdll;
            //  globals->pln->pit->GetPanelByTag ('frnt')->dll_gauge.push_back (new_g);
            //}
/*/
            //#ifdef _DEBUG_dll2
            //  {FILE *fp_debug;
            //  if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
            //  {
            //    char buff [8] = {0};
            //    TagToString (buff, types[1]->signature);
            //    fprintf(fp_debug, "  CPluginMain::On_Inst APICreateGauge %s\n", buff);
	           //   fclose(fp_debug);
            //  }}
            //#endif
            } // end traverse ckpt
/*/ //122809
          } else
          if ((*idll_data).ityp_obj->first->type == TYPE_DLL_SYSTEM){
            plg_object [0] = 0; // 082711
            dll_system_init_counter++;
            #ifdef _DEBUG
              //TRACE ("DLL SUBSYSTEM INITIALIZED %d %p", dll_system_init_counter, plg_object [0]);
            #endif
          } else
          if ((*idll_data).ityp_obj->first->type == TYPE_DLL_VIEW){
            plg_object [0] = 0; // 082711
          } else
          if ((*idll_data).ityp_obj->first->type == TYPE_DLL_CAMERA){
            dll_camera_init_counter++;
            plg_object [0] = 0; // 082711
            #ifdef _DEBUG
              //TRACE ("DLL CAMERA INITIALIZED %d %p", dll_camera_init_counter, plg_object [0]);
            #endif
          }
//        }
//        dllobjects_counter++; // lc 110709
      }
    }
  }
//#ifdef _DEBUG_DLLGAUGES
//  UtilityPrintDLLData ();
//#endif
}  		        // DLLInstantiate;

/*! \brief This function is called in the initialisation process
 *  only AFTER the gauges have been created with the PNL file
 */
/*
void CPluginMain::On_Link_DLLGauges (const long,const long,SDLLObject**) const
{
  if (!flag_instantiate) return;
  //
#ifdef _DEBUG_DLLGAUGES
    TRACE ("DLL LINK GAUGES %d", dll_gauges_init_counter);
#endif
    int k = 1;
    for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {
//    for (itdll = m_dll.begin (); itdll != m_dll.end (); ++itdll) {
//    if ((*itdll)->DLLInstantiate) {
      //
      // k[0] is actually the last object so
      // we must correct the k value
      if (dllobjects_counter == k) k = 0;
      //
      for ((*idll_data).ityp_obj = (*idll_data).typ_obj.begin (); (*idll_data).ityp_obj != (*idll_data).typ_obj.end (); ++(*idll_data).ityp_obj) {
        //(*idll_data).ityp_obj->first->type); 
        //(*idll_data).ityp_obj->first->signature); 

//      for (types[1] = types[0]; types[1] != NULL; types[1] = types[1]->next) {
        //TRACE ("... %p %d %d", *plg_object, types[1]->type, TYPE_DLL_GAUGE);
        if ((*idll_data).ityp_obj->first->type == TYPE_DLL_GAUGE){
          //(*itdll)->DLLInstantiate ( types[1]->type,
          //                           types[1]->signature,
          //                           plg_object); // 
//          if (*plg_object) {

            ///// traverse all the cockpits /////
            std::map<Tag,CPanel*>::const_iterator iter;
            char pnl_buff [8] = {0};//'frnt'
            Tag ckpt_tag = 0;
            //int test = 0;
            for (iter = globals->pln->pit->GetMapCkpt ().begin();
                 iter!= globals->pln->pit->GetMapCkpt ().end(); iter++) {
              ckpt_tag = (*iter).first;
              TagToString (pnl_buff, ckpt_tag);
              #ifdef _DEBUG_DLLGAUGES
                TRACE ("Try ckpt %s", pnl_buff);
              #endif
            //}

              std::vector<CDLLGauge *>::iterator it =
                globals->pln->pit->GetPanelByTag (ckpt_tag)->dll_gauge.begin ();
              for (it; it != globals->pln->pit->GetPanelByTag (ckpt_tag)->dll_gauge.end (); ++it) {
                if ((*it)->GetSignature () == static_cast <const long> ((*idll_data).ityp_obj->first->signature)) {
                break;
                }
              }
              char buff [8] = {0};
              TagToString (buff, (*idll_data).ityp_obj->first->signature);
              if (it != globals->pln->pit->GetPanelByTag (ckpt_tag)->dll_gauge.end ()) {
//                *plg_object = NULL;
//                (*itdll)->DLLInstantiate ( types[1]->type,
//                                           types[1]->signature,
//                                           plg_object); // 
                if (*plg_object) {
                  //// k[0] is actually the last object so
                  //// we must correct the k value
                  //if (dllobjects_counter == k) k = 0;
                  ////
                  (*idll_data).enabled = true;
                  (*it)->dll = (*idll_data).dll_registry;
                  plg_object [k]->flyObject.superSig = (*idll_data).ityp_obj->first;
                  (*it)->SetObject (plg_object [k]);
                  //test++;
                  #ifdef _DEBUG_DLLGAUGES
                  TRACE ("Found <%s> DLL GAUGE in '%s' %d %p ",
                    buff, pnl_buff, dll_gauges_init_counter, (*idll_data).dll_registry);
                  #endif
                  #ifdef _DEBUG_dll2
                    {FILE *fp_debug;
                    if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
                    {
                      char buff [8] = {0};
                      TagToString (buff, (*idll_data).ityp_obj->first->signature);
                      fprintf(fp_debug, "  CPluginMain::On_Inst (+) APICreateGauge %s in '%s'\n",
                        buff, pnl_buff);
                      fclose(fp_debug);
                    }}
                  #endif 
                  //break;
                }
              } else {
                // no gauge found
                #ifdef _DEBUG
                //TRACE ("Can't find <%s> DLL GAUGE in '%s'", buff, pnl_buff);
                #endif
                //
                //On_DestroyObject (plg_object [k], (*idll_data).dll_registry);
              }
            } // end traverse ckpt
            //
//          }
            //if (test == 0) {
            //  #ifdef _DEBUG
            //  TRACE ("should remove gauge object");
            //  #endif
            //  if (NULL == (*itdll)->GetObject ())
            //    (*itdll)->DLLDestroyObject (plg_object [0]);
            //}
        }
      }
//    }
      ++k;
  }
  //FreeInitialDLLObjectList ();
  //
#ifdef _DEBUG_DLLGAUGES
  UtilityPrintDLLObject ();
  UtilityPrintDLLData ();
#endif
}  		        //
*/
/*! \brief This function is called in the initialisation process
 *  only AFTER the systems have been created with the AMP file
 */
void CPluginMain::On_Link_DLLSystems (const long,const long,SDLLObject**) const
{
  if (!flag_instantiate) return;
}

/*! \brief This function is called whenever we do switch between aircraft
 *  Used to initialise any dll gauge for the user aircraft
 */
void CPluginMain::On_Instantiate_DLLGauges (const long sig,const long,SDLLObject**) const
{
#ifdef _DEBUG_DLLGAUGES
  TRACE ("DLL INIT GAUGE AGAIN %d", dll_gauges_init_counter);
#endif
  for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {
    if ((*idll_data).dll_registry->DLLInstantiate) {
      for ((*idll_data).ityp_obj = (*idll_data).typ_obj.begin (); (*idll_data).ityp_obj != (*idll_data).typ_obj.end (); ++(*idll_data).ityp_obj) {
        if ((*idll_data).ityp_obj->first->type == TYPE_DLL_GAUGE){
          if (sig == static_cast <const long> ((*idll_data).ityp_obj->first->signature)) {
            dll_gauges_init_counter++;
            #ifdef _DEBUG_DLLGAUGES
            TRACE ("DLL GAUGE %d", dll_gauges_init_counter);
            #endif
            //
            *plg_object = NULL;
            (*idll_data).dll_registry->DLLInstantiate ( (*idll_data).ityp_obj->first->type,
                                                        (*idll_data).ityp_obj->first->signature,
                                                        plg_object); // 
            dll_gauges_init_counter++;
            if (*plg_object) {
              (*idll_data).ityp_obj->second = plg_object [0];
              //(*it)->dll = (*idll_data).dll_registry;
              plg_object [0]->flyObject.superSig = (*idll_data).ityp_obj->first;
              //(*it)->SetObject (plg_object [0]);
              (*idll_data).enabled = true;
              #ifdef _DEBUG_DLLGAUGES
              char buff [8] = {0};
              TagToString (buff, sig);
              TRACE ("Found <%s> DLL GAUGE", buff);
              #endif
            }
          } else {
            ;
          }
        }
      }
    }
  }
  //
#ifdef _DEBUG_DLLGAUGES
  UtilityPrintDLLObject ();
  UtilityPrintDLLData ();
#endif
}  //
/*
void On_Instantiate_DLLGauges (const long,const long,SDLLObject**) const
{
  if (!flag_instantiate) return;
  int counter = 0;
  //FreeInitialDLLObjectList ();
  //
#ifdef _DEBUG_DLLGAUGES
    TRACE ("DLL INIT GAUGE AGAIN %d", dll_gauges_init_counter);
#endif
    for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {
    //for (itdll = m_dll.begin (); itdll != m_dll.end (); ++itdll) {
    //if ((*itdll)->DLLInstantiate) {
      if ((*idll_data).dll_registry->DLLInstantiate) {
      //for (types[1] = types[0]; types[1] != NULL; types[1] = types[1]->next) {
      for ((*idll_data).ityp_obj = (*idll_data).typ_obj.begin (); (*idll_data).ityp_obj != (*idll_data).typ_obj.end (); ++(*idll_data).ityp_obj) {
       //(*idll_data).ityp_obj->first->type); 
       //(*idll_data).ityp_obj->first->signature); 
       //TRACE ("... %p %d %d", *plg_object, types[1]->type, TYPE_DLL_GAUGE);
        if ((*idll_data).ityp_obj->first->type == TYPE_DLL_GAUGE){
          //(*itdll)->DLLInstantiate ( types[1]->type,
          //                           types[1]->signature,
          //                           plg_object); // 
//          if (*plg_object) {
          #ifdef _DEBUG
            //TRACE ("... %p %d %d", *plg_object, types[1]->type, TYPE_DLL_GAUGE);
          #endif

            ///// traverse all the cockpits /////
            std::map<Tag,CPanel*>::const_iterator iter;
            char pnl_buff [8] = {0};//'frnt'
            Tag ckpt_tag = 0;
            for (iter = globals->pln->pit->GetMapCkpt ().begin();
                 iter!= globals->pln->pit->GetMapCkpt ().end(); iter++) {
              ckpt_tag = (*iter).first;
              TagToString (pnl_buff, ckpt_tag);
              #ifdef _DEBUG_DLLGAUGES
                TRACE ("Try ckpt %s", pnl_buff);
              #endif
            //}

              std::vector<CDLLGauge *>::iterator it =
                globals->pln->pit->GetPanelByTag (ckpt_tag)->dll_gauge.begin ();
              for (it; it != globals->pln->pit->GetPanelByTag (ckpt_tag)->dll_gauge.end (); ++it) {
                if ((*it)->GetSignature () == static_cast <const long> ((*idll_data).ityp_obj->first->signature)) {
                 break;
                }
              }
              char buff [8] = {0};
              TagToString (buff, (*idll_data).ityp_obj->first->signature);
              if (it != globals->pln->pit->GetPanelByTag (ckpt_tag)->dll_gauge.end ()) {

                // delete any former gauge
                #ifdef _DEBUG_DLLGAUGES
                  TRACE ("DLL GAUGE %d", dll_gauges_init_counter);
                #endif

                // search for any free instace
                
                //
                //if (NULL == (*it)->Get_Object ())
                //  if ((*itdll)->DLLDestroyObject) (*itdll)->DLLDestroyObject (plg_object [0]);
                // instatiate a new gauge
                //if (0 == dll_gauges_init_counter) {
                  *plg_object = NULL;
                  (*idll_data).dll_registry->DLLInstantiate ( (*idll_data).ityp_obj->first->type,
                                                              (*idll_data).ityp_obj->first->signature,
                                                              plg_object); // 
                  //dll_gauges_init_counter++;
                  counter++;
                //}
                if (*plg_object) {
                  (*idll_data).ityp_obj->second = plg_object [0];
                  //if ((*itdll)->DLLDestroyObject) (*itdll)->DLLDestroyObject (plg_object [0]);
                  (*it)->dll = (*idll_data).dll_registry;
                  plg_object [0]->flyObject.superSig = (*idll_data).ityp_obj->first;
                  (*it)->SetObject (plg_object [0]);
                  (*idll_data).enabled = true;
                  #ifdef _DEBUG_DLLGAUGES
                  TRACE ("Found <%s> DLL GAUGE in '%s' %d %p ",
                    buff, pnl_buff, dll_gauges_init_counter, (*idll_data).dll_registry);
                  #endif
                  #ifdef _DEBUG_dll2
                    {FILE *fp_debug;
                    if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
                    {
                      char buff [8] = {0};
                      TagToString (buff, (*idll_data).ityp_obj->first->signature);
                      fprintf(fp_debug, "  CPluginMain::On_Inst (+) APICreateGauge %s in '%s'\n",
                        buff, pnl_buff);
                      fclose(fp_debug);
                    }}
                  #endif 
                  //break;
                }
              } else {
                #ifdef _DEBUG
                //TRACE ("Can't find <%s> DLL GAUGE in '%s'", buff, pnl_buff);
                #endif
              }
            } // end traverse ckpt
            //
//          } 
        }
      }
    }
  }
  //
  if (counter) dll_gauges_init_counter++;
#ifdef _DEBUG_DLLGAUGES
  UtilityPrintDLLObject ();
  UtilityPrintDLLData ();
#endif
}  		        //
*/
/*! \brief This function is called whenever we do switch between aircraft
 *  Used to initialise any dll system for the user aircraft
 */
void CPluginMain::On_Instantiate_DLLSystems (const long sig,const long,SDLLObject**) const
{
#ifdef _DEBUG
  //TRACE ("DLL INSTANTIATE SUBSYSTEM %d", dll_system_init_counter);
#endif
  for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {
    if ((*idll_data).dll_registry->DLLInstantiate) {
      for ((*idll_data).ityp_obj = (*idll_data).typ_obj.begin (); (*idll_data).ityp_obj != (*idll_data).typ_obj.end (); ++(*idll_data).ityp_obj) {
        if ((*idll_data).ityp_obj->first->type == TYPE_DLL_SYSTEM){
          if (sig == static_cast <const long> ((*idll_data).ityp_obj->first->signature)) {
            dll_system_init_counter++;
            #ifdef _DEBUG_DLLGAUGES
            TRACE ("DLL SUBSYSTEM %d", dll_system_init_counter);
            #endif
            //
            *plg_object = NULL;
            (*idll_data).dll_registry->DLLInstantiate ( (*idll_data).ityp_obj->first->type,
                                                        (*idll_data).ityp_obj->first->signature,
                                                        plg_object); // 
            //dll_system_init_counter++;
            if (*plg_object) {
              (*idll_data).ityp_obj->second = plg_object [0];
              //(*it)->dll = (*idll_data).dll_registry;
              plg_object [0]->flyObject.superSig = (*idll_data).ityp_obj->first;
              //(*it)->SetObject (plg_object [0]);
              (*idll_data).enabled = true;
              #ifdef _DEBUG_DLLGAUGES
              char buff [8] = {0};
              TagToString (buff, sig);
              TRACE ("Found <%s> DLL SUBSYSTEM", buff);
              #endif
            }
          } else {
            ;
          }
        }
      }
    }
  }
}

void CPluginMain::On_Instantiate_DLLCamera (const long sig,const long,SDLLObject**) const
{
#ifdef _DEBUG
  //TRACE ("DLL INSTANTIATE CAMERA %d", dll_camera_init_counter);
#endif
  for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {
    if ((*idll_data).dll_registry->DLLInstantiate) {
      for ((*idll_data).ityp_obj = (*idll_data).typ_obj.begin (); (*idll_data).ityp_obj != (*idll_data).typ_obj.end (); ++(*idll_data).ityp_obj) {
        if ((*idll_data).ityp_obj->first->type == TYPE_DLL_CAMERA){
          if (sig == static_cast <const long> ((*idll_data).ityp_obj->first->signature)) {
            dll_camera_init_counter++;
            #ifdef _DEBUG_DLLGAUGES
            TRACE ("DLL CAMERA %d", dll_camera_init_counter);
            #endif
            //
            *plg_object = NULL;
            (*idll_data).dll_registry->DLLInstantiate ( (*idll_data).ityp_obj->first->type,
                                                        (*idll_data).ityp_obj->first->signature,
                                                        plg_object); // 
            //dll_camera_init_counter++;
            if (*plg_object) {
              (*idll_data).ityp_obj->second = plg_object [0];
              //(*it)->dll = (*idll_data).dll_registry;
              plg_object [0]->flyObject.superSig = (*idll_data).ityp_obj->first;
              //(*it)->SetObject (plg_object [0]);
              (*idll_data).enabled = true;
              #ifdef _DEBUG_DLLGAUGES
              char buff [8] = {0};
              TagToString (buff, sig);
              TRACE ("Found <%s> DLL CAMERA", buff);
              #endif
            }
          } else {
            ;
          }
        }
      }
    }
  }
#ifdef _DEBUG_DLLGAUGES
  UtilityPrintDLLObject ();
  UtilityPrintDLLData ();
#endif
}  //

void CPluginMain::On_InitObject      (SDLLObject*) const
{
}  		        // DLLInitObject;

void CPluginMain::On_Draw (SDLLObject *obj, SSurface *surf, void *dll) const
{
  if (!flag_instantiate) return;
  #ifdef _DEBUG_//dll2	
    FILE *fp_debug;
    if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
    {
      char buff [8] = {0};
      TagToString (buff, static_cast<SDLLRegisterTypeList *> (obj->flyObject.superSig)->signature);
      fprintf(fp_debug, "CPluginMain::On_Draw %p @ %s %p\n", obj, buff, dll);
      fclose(fp_debug);
    }
  #endif
  //int k = 0;
  for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {
  //for (itdll = m_dll.begin (); itdll != m_dll.end (); ++itdll) {
//    if ((*itdll)->DLLDraw) {
      for ((*idll_data).ityp_obj = (*idll_data).typ_obj.begin (); (*idll_data).ityp_obj != (*idll_data).typ_obj.end (); ++(*idll_data).ityp_obj) {
        //(*idll_data).ityp_obj->first->type); 
        //(*idll_data).ityp_obj->first->signature); 
      //for (types[1] = types[0]; types[1] != NULL; types[1] = types[1]->next) {
        //if (k < dllobjects_counter) {
          if (obj == (*idll_data).ityp_obj->second) {
          //if (plg_object [k] && obj && plg_object [k] == obj) {
//            (*itdll)->DLLDraw (obj, surf/*dll_surf*/);
            static_cast <DLLFunctionRegistry *> (dll)->DLLDraw (obj, surf); //
            #ifdef _DEBUG_//dll2	
              FILE *fp_debug;
              char buff [8] = {0};
              TagToString (buff, types [1]->signature);
              if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
              {
	              fprintf(fp_debug, "CPluginMain::On_Draw plg_object[%d] %p %p %s\n",
                  k, plg_object[k], obj, buff);
	              fclose(fp_debug);
              }
            #endif
            return;
          }
        //}
        //k++;
      }
//    }
  }
}  			        // DLLDraw;

/*
void CPluginMain::On_Draw            (SDLLObject *obj, SSurface *surf) const
{
  if (!flag_instantiate) return;
  for (itdll = m_dll.begin (); itdll != m_dll.end (); ++itdll) {
    for (int k = 0; k < dllobjects_counter; ++k) {
      if (plg_object [k] && obj && plg_object [k] == obj) {
        if ((*itdll)->DLLDraw) (*itdll)->DLLDraw (obj, surf);//dll_surf
        #ifdef _DEBUG_dll3	
          FILE *fp_debug;
          if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
          {
            fprintf(fp_debug, "CPluginMain::On_Draw %d plg_object[%d] %p %p\n",
              dllobjects_counter, k, plg_object [k], obj);
            fclose(fp_debug);
          }
        #endif
      }
    }
  }
}  			        // DLLDraw;
*/

int CPluginMain::On_Read (SDLLObject *obj, SStream *stream, unsigned int tag, void *dll) const
{ int ret = TAG_IGNORED;
  #ifdef _DEBUG
    //TRACE ("ON_READ %d", flag_instantiate);
  #endif 
  if (!flag_instantiate) return ret;
  for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {
    //TRACE ("test 1");
    for ((*idll_data).ityp_obj = (*idll_data).typ_obj.begin (); (*idll_data).ityp_obj != (*idll_data).typ_obj.end (); ++(*idll_data).ityp_obj) {
      //TRACE ("test 2 %p %p", static_cast <DLLFunctionRegistry *> (dll), (*idll_data).dll_registry);
      //if (obj == (*idll_data).ityp_obj->second) {
      if (static_cast <DLLFunctionRegistry *> (dll) == (*idll_data).dll_registry) {
      //if (sig == static_cast <const long> ((*idll_data).ityp_obj->first->signature)) {
        //TRACE ("test 3 %p", (*idll_data).ityp_obj->second);
        ret = static_cast <DLLFunctionRegistry *> (dll)->DLLRead ((*idll_data).ityp_obj->second, stream, tag); //
        #ifdef _DEBUG//dll2	
          FILE *fp_debug;
          char buf1 [8] = {0}, buf2 [8] = {0};
          //TagToString (buf1, (*idll_data).ityp_obj->second);
          TagToString (buf2, tag);
          //TRACE ("read  %p %p %p '%s'=%d %p",
          //  dll, (*idll_data).dll_registry, (*idll_data).ityp_obj->second, buf2, ret, stream);
          if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
          {
            fprintf(fp_debug, "CPluginMain::On_Read %p %s=%d %p\n",
              (*idll_data).ityp_obj->second, buf2, ret, stream);
            fclose(fp_debug);
          }
        #endif
        return ret;
      }
    }
  }
  return ret;
}  			        // DLLRead;

void CPluginMain::On_ReadFinished    (SDLLObject*) const
{
}  		    // DLLReadFinished;

void CPluginMain::On_Write           (SDLLObject*,SStream*) const
{
}  		        	// DLLWrite;

void CPluginMain::On_MouseClick      (SDLLObject*,SSurface*,int,int,int) const
{
}  		        // DLLMouseClick;

void CPluginMain::On_TrackClick      (SDLLObject*,SSurface*,int,int,int) const
{
}  		        // DLLTrackClick;

void CPluginMain::On_StopClick       (SDLLObject*,SSurface*,int,int,int) const
{
}  		        // DLLStopClick;

void CPluginMain::On_MouseMoved      (SDLLObject*,SSurface*,int,int) const
{
}  		        // DLLMouseMoved;

void CPluginMain::On_Prepare         (SDLLObject*) const
{
}  			        // DLLPrepare;

void CPluginMain::On_TimeSlice       (SDLLObject *obj, const float dT, void *dll) const
{
  if (!flag_instantiate) return;
  #ifdef _DEBUG_//dll2	
    FILE *fp_debug;
    if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
    {
      char buff [8] = {0};
      TagToString (buff, static_cast<SDLLRegisterTypeList *> (obj->flyObject.superSig)->signature);
      fprintf(fp_debug, "CPluginMain::On_TimeSlice %p @ %s %p\n", obj, buff, dll);
      fclose(fp_debug);
    }
  #endif
  //
  for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {
    for ((*idll_data).ityp_obj = (*idll_data).typ_obj.begin (); (*idll_data).ityp_obj != (*idll_data).typ_obj.end (); ++(*idll_data).ityp_obj) {
      if (obj == (*idll_data).ityp_obj->second) {
        static_cast <DLLFunctionRegistry *> (dll)->DLLTimeSlice (obj, dT); //
        #ifdef _DEBUG_//dll2	
          FILE *fp_debug;
          char buff [8] = {0};
          TagToString (buff, types [1]->signature);
          if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
          {
	          fprintf(fp_debug, "CPluginMain::On_TimeSlice plg_object[%d] %p %p %s\n",
                k, plg_object[k], obj, buff);
	              fclose(fp_debug);
          }
        #endif
        return;
      }
    }
  }
}  		        // DLLTimeSlice;

void CPluginMain::On_KeyPressed      (int,int) const
{
}  		        // DLLKeyPressed;

void CPluginMain::On_KeyIntercept    (int,int) const
{
}  		        // DLLKeyIntercept;

void CPluginMain::On_KeyCallback     (int,int,SDLLObject*) const
{
}  		        // DLLKeyCallback;

void CPluginMain::On_KeyCallback2     (int,int,int,SDLLObject*) const
{
}  		        // DLLKeyCallback2;

void CPluginMain::On_ProcessSoundEffects (SDLLObject*) const
{
}  	    // DLLProcessSoundEffects;

void CPluginMain::On_SuspendSoundEffects (SDLLObject*) const
{
}  	    // DLLSuspendSoundEffects;

void CPluginMain::On_ResumeSoundEffects (SDLLObject*) const
{
}  	    // DLLResumeSoundEffects;

void CPluginMain::On_InWindowRegion  (SDLLObject*,SSurface*,int,int) const
{
}  		    // DLLInWindowRegion;

void CPluginMain::On_GetPopupHelp    (SDLLObject*,char*,char*,char*) const
{
}  		    // DLLGetPopupHelp;

void CPluginMain::On_UpdateCamera    (SDLLObject *obj,
                                      SPosition  *targetPos, SVector *targetOri,
                                      SPosition  *eyePos, SVector *eyeOri,
                                      const float dT, void *dll) const
{
  if (!flag_instantiate) return;
  //
  for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {
    for ((*idll_data).ityp_obj = (*idll_data).typ_obj.begin (); (*idll_data).ityp_obj != (*idll_data).typ_obj.end (); ++(*idll_data).ityp_obj) {
      if (obj == (*idll_data).ityp_obj->second) {
        static_cast <DLLFunctionRegistry *> (dll)->DLLUpdateCamera (obj,
                                                                    targetPos, targetOri,
                                                                    eyePos, eyeOri,
                                                                    dT); //
        #ifdef _DEBUG_//dll2	
          FILE *fp_debug;
          char buff [8] = {0};
          TagToString (buff, types [1]->signature);
          if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
          {
	          fprintf(fp_debug, "CPluginMain::On_UpdateCamera  %p %s\n",
                obj, buff);
	              fclose(fp_debug);
          }
        #endif
        return;
      }
    }
  }
}  		    // DLLUpdateCamera;

void CPluginMain::On_IsInteriorCamera (SDLLObject*) const
{
}  		// DLLIsInteriorCamera;

void CPluginMain::On_GetCameraName   (SDLLObject*,char*,int) const
{
}  		    // DLLGetCameraName;

void CPluginMain::On_DrawOverlay     (SSurface*,SDLLObject*,int,int,int,int) const
{
}  		        // DLLDrawOverlay;

void CPluginMain::On_ReadIniFile     (void) const
{
}  		        // DLLReadIniFile;

void CPluginMain::On_WriteIniFile    (void) const
{
}  		    // DLLWriteIniFile;

void CPluginMain::On_ControlCanBeUsed (void) const
{
}  		// DLLControlCanBeUsed;

void CPluginMain::On_ControlButtonCount (void) const
{
}  	    // DLLControlButtonCount;

void CPluginMain::On_ControlButtonPressed (int) const
{
}  	// DLLControlButtonPressed;

void CPluginMain::On_ControlButtonName (int,char*,char*) const
{
}  	    // DLLControlButtonName;

void CPluginMain::On_ControlDebounceButton (int) const
{
}  	// DLLControlDebounceButton;
void CPluginMain::On_ControlHasAxis  (EJoystickAxis axis) const
{
}  		    // DLLControlHasAxis;

void CPluginMain::On_ControlGetAxis  (EJoystickAxis axis) const
{
}  		    // DLLControlGetAxis;

void CPluginMain::On_ControlGetIdentifier (void) const
{
}  	// DLLControlGetIdentifier;

void CPluginMain::On_RunScript       (const char *scriptFile, const char *refName, const char runOnce) const
{
}  		        // DLLRunScript;

void CPluginMain::On_CallScriptFunction (const char *scriptFile, const char *funcName) const
{
}  	    // DLLCallScriptFunction;

void CPluginMain::On_StopScript      (const char *scriptFile) const
{
}  		        // DLLStopScript;

void CPluginMain::On_StopScriptByRefName (const char *refName) const
{
}  	    // DLLStopScriptByRefName;

void CPluginMain::On_GetScriptInfo   (int *numScripts, SScriptInfo **array) const
{
}  		    // DLLGetScriptInfo;

void CPluginMain::On_GetPythonMethodTable (void) const
{
}  	// DLLGetPythonMethodTable;

void CPluginMain::On_BeginFrame      (void) const
{
}  		        // DLLBeginFrame;

void CPluginMain::On_EventNotice     (SDLLObject *notify,
                                      unsigned long windowID,
                                      unsigned long componentID,
                                      unsigned long event_,
                                      unsigned long subEvent) const
{
  //for (itdll = m_dll.begin (); itdll != m_dll.end (); ++itdll) {
  //  if ((*itdll)->DLLEventNotice) (*itdll)->DLLEventNotice (NULL, windowID, componentID, event_, subEvent);
  //}
  for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {
    if ((*idll_data).dll_registry->DLLEventNotice)
      (*idll_data).dll_registry->DLLEventNotice (NULL, windowID, componentID, event_, subEvent);
  }
}  		        // DLLEventNotice;

void CPluginMain::On_EpicEvent       (unsigned char, unsigned char[6]) const
{
}  		        // DLLEpicEvent;

void CPluginMain::On_GenerateTileScenery (SPosition*,SPosition*,int,int,int,int,int,int) const
{
}  	    // DLLGenerateTileScenery;
void CPluginMain::On_FilterMetar     (char *metarFilename) const
{
}  		        // DLLFilterMetar;

int CPluginMain::On_InitGlobalMenus (void) const
{
  int count = 0;
  for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {
    if ((*idll_data).dll_registry->DLLInitGlobalMenus) {
      (*idll_data).dll_registry->DLLInitGlobalMenus ();
      ++count;
    }
  }
  //TRACE ("On_InitGlobalMenus %d", count); // 082711
  return count;
}		    // DLLInitGlobalMenus;

void* CPluginMain::IsDLL (const long tag) const
{
  for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {
    for ((*idll_data).ityp_obj = (*idll_data).typ_obj.begin (); (*idll_data).ityp_obj != (*idll_data).typ_obj.end (); ++(*idll_data).ityp_obj) {
      if (tag == static_cast <const long> ((*idll_data).ityp_obj->first->signature)) {
        #ifdef _DEBUG_DLLGAUGES
        char buf1 [8] = {0}, buf2 [8] = {0};
        TagToString (buf1, tag); 
        TagToString (buf2, (*idll_data).ityp_obj->first->signature);         
        TRACE ("is dll ? %s %s", buf1, buf2);
        #endif
        return (*idll_data).dll_registry;
      }
    }
  } 
  return 0;
}

SDLLObject* CPluginMain::GetDLLObject (const long tag) const
{
  for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {
    for ((*idll_data).ityp_obj = (*idll_data).typ_obj.begin (); (*idll_data).ityp_obj != (*idll_data).typ_obj.end (); ++(*idll_data).ityp_obj) {
      if (tag == static_cast <const long> ((*idll_data).ityp_obj->first->signature)) {
        #ifdef _DEBUG_DLLGAUGES
        char buf1 [8] = {0};
        TagToString (buf1, tag); 
        TRACE ("GetDLLObject %s %p", buf1, (*idll_data).ityp_obj->second);
        #endif
        return (*idll_data).ityp_obj->second;
      }
    }
  } 
  return 0;
}

void CPluginMain::BuildExportList (void)
{
  dll_export_listing.clear ();
  for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data)
  {
    for ((*idll_data).ityp_obj = (*idll_data).typ_obj.begin (); (*idll_data).ityp_obj != (*idll_data).typ_obj.end (); ++(*idll_data).ityp_obj) {
      char buf [100] = {0};
      if ((*idll_data).ityp_obj->first->type)
      {
        char buf1 [8] = {0}, buf2 [8] = {0};
        TagToString (buf1, (*idll_data).ityp_obj->first->type); 
        TagToString (buf2, (*idll_data).ityp_obj->first->signature);
        sprintf(buf, " %05d <%4s> [%4s] (%1d) : %-64s",
            (*idll_data).cpyrt.internalVersion,
            buf1, buf2,
            (*idll_data).enabled, (*idll_data).cpyrt.product);
      } else {
        sprintf(buf, " %05d <%4s> [%4s] (%1d) : %-64s",
            (*idll_data).cpyrt.internalVersion,
            "----", "----",
            1, (*idll_data).cpyrt.product);
      }
      dll_export_listing.push_back (buf);
      //
      //#ifdef _DEBUG	
      //{	FILE *fp_debug;
	    //  if(!(fp_debug = fopen("__DDEBUG_plugin.txt", "a")) == NULL)
	    //  {
		  //    fprintf(fp_debug, "%s\n", buf);
		  //    fclose(fp_debug); 
      //}	}
      //#endif
    }
  }
}

//================================================================================
//	JS:  Put time slice here
//================================================================================
int CPluginMain::TimeSlice(float dT,U_INT frame)
{	//---- sdk: Update DLL FlyObjects -----------------------
  sdk_flyobject_list.i_fo_list = sdk_flyobject_list.fo_list.begin ();
  ++sdk_flyobject_list.i_fo_list;                                // skip first user object
  for (sdk_flyobject_list.i_fo_list; sdk_flyobject_list.i_fo_list != sdk_flyobject_list.fo_list.end ();
       ++sdk_flyobject_list.i_fo_list) {
       ((CSimulatedObject *)(sdk_flyobject_list.i_fo_list)->ref.objectPtr)->Timeslice (dT,frame); // 
    }
  //---- sdk: Update plugin DLLIdle data -------------------
  On_Idle (dT);
	//---- sdk: Update DLL windows ---------------------------
	std::vector<CDLLWindow*>::iterator wk;
  for (wk = wLst.begin (); wk != wLst.end (); ++wk)
	{CDLLWindow *wdl =   (*wk);
	 wdl->TimeSlice (dT);
	 }
	 return 1;
}

//================================================================================
//	JS:  Put Draw here
//================================================================================
void CPluginMain::DrawExternal()
{	sdk_flyobject_list.i_fo_list = sdk_flyobject_list.fo_list.begin ();
    ++sdk_flyobject_list.i_fo_list;                                // skip first user object
    for (sdk_flyobject_list.i_fo_list;  sdk_flyobject_list.i_fo_list != sdk_flyobject_list.fo_list.end ();
       ++sdk_flyobject_list.i_fo_list) {
        dVeh = (CDLLSimulatedObject *)((sdk_flyobject_list.i_fo_list)->ref.objectPtr); // 
        if (dVeh)  dVeh->DrawExternal();
    }
	return;
}
//==============================================================================
// sdk: Draw the DLL windows plugins
//  Called from CFUIManager::Draw method
//===============================================================================
void CPluginMain::DrawDLLWindow (void)
{ std::vector<CDLLWindow*>::iterator wk;
  for (wk = wLst.begin (); wk != wLst.end (); ++wk) (*wk)->Draw ();
}
//-------------------------------------------------------------------------
//  Free DLLWindows resources
//-------------------------------------------------------------------------
void CPluginMain::FreeDLLWindows (void)
{ 
#ifdef _DEBUG
  DEBUGLOG ("CSituation::FreeDLLWindows");
#endif  
  // delete all pointers in dllW
	std::vector<CDLLWindow*>::iterator wk;
 for (wk = wLst.begin (); wk != wLst.end (); ++wk)  SAFE_DELETE (*wk);
 wLst.clear (); // 
}

//========================================================================
//    CDLLSimulated Object
//========================================================================
CDLLSimulatedObject::CDLLSimulatedObject (void)
: CSimulatedObject ()
{ SetType(TYPE_FLY_SIMULATEDOBJECT);
  nfo = NULL;
  lod = NULL;
  draw_flag = false;
  sim_objects_active = false;
 *nfoFilename = 0;
}
//------------------------------------------------------------------------
//	Destroy object
//------------------------------------------------------------------------
CDLLSimulatedObject::~CDLLSimulatedObject (void)
{*nfoFilename = 0;
  SAFE_DELETE (nfo);
  SAFE_DELETE (lod);
}
//------------------------------------------------------------------ 
//    Set Orientation
//------------------------------------------------------------------
void  CDLLSimulatedObject::SetOrientation (SVector orientation)
{ // adjust orientation with magnetic declination
  CVector iang_ = orientation;
  float decl_degrees_ = 0.0f, hor_field_ = 0.0f;
  CMagneticModel::Instance().GetElements (this->GetPosition (), decl_degrees_, hor_field_);
  iang_.z += DegToRad (static_cast<double> (decl_degrees_)); 
  // if object is from simulated situation set global ori 
//  if (globals->pln) CWorldObject::SetOrientation (orientation);
//  else
  CWorldObject::SetObjectOrientation (iang_);
}

//------------------------------------------------------------------ 
//    TimeSlice: Update everything
//------------------------------------------------------------------
int CDLLSimulatedObject::Timeslice (float dT,U_INT FrNo)
{
  if (sim_objects_active) {
    sobj_offset = SubtractPositionInFeet (globals->geop, geop);
    // verify whether to draw object or not
    draw_flag = true;
    if (fabs (sobj_offset.x) > FEET_PER_NM) draw_flag = false;
    else
    if (fabs (sobj_offset.y) > FEET_PER_NM) draw_flag = false;
    else
    if (fabs (sobj_offset.z) > FEET_PER_NM) draw_flag = false;
  }
  Simulate (dT,FrNo);
	return 1;
}

void CDLLSimulatedObject::Simulate (float dT,U_INT FrNo)
{
  return;
}

void CDLLSimulatedObject::DrawExternal (void)
{
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  // Draw all externally visible objects associated with this object
  if (lod) {
    //if (globals->pln) { // 
    //if (globals->sit->sdk_flyobject_list.fo_list.size () > 1) {
    if (sim_objects_active) {
      if (draw_flag) {
        draw_flag    = false;
        globals->pln = (CAirplane*)this;
        lod->Draw (BODY_TRANSFORM);       // 1 = simulated DLL object
        globals->plugins->dVeh = NULL;
      }
    }
  }
  glMatrixMode (GL_MODELVIEW);
  glPopMatrix ();
  return;
}


//void ut::InitSdkIntf (void)
//{
//  gRegistry.APISetMouseCursor = APISetMouseCursor;
//}

//============================================================================================
//  Class CDLLWindow to display a DLL window plugin
//============================================================================================
CDLLWindow::CDLLWindow (void)
{ surf  = 0;
  obj   = 0;
  wd    = static_cast<short> (globals->mScreen.Width);
  ht    = static_cast<short> (globals->mScreen.Height);
  signature = 0;
  dll = 0;
  enabled = false;
  //back  = MakeRGB (212, 212,   0);
  back  = MakeRGB (  0,   0,   0);
  black = MakeRGB (  0,   0,   0);
  Resize();  
}
//---------------------------------------------------------------------------------
//  Destroy 
//---------------------------------------------------------------------------------
CDLLWindow::~CDLLWindow (void)
{ 
#ifdef _DEBUG
  //TRACE ("DELETE DLL SURF %p %p %p", surf, obj, dll);
#endif
  if (surf) surf = FreeSurface (surf);
  // sdk: cleanup objects = DLLDestroyObject // 
  globals->plugins->On_DestroyObject (obj, dll);
}
//---------------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------------
void CDLLWindow::SetObject (SDLLObject *object)
{
  obj = object;
}
//---------------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------------
void CDLLWindow::SetSignature (const long &sig)
{
  signature = sig;
}
//---------------------------------------------------------------------------------
//  Resize parameters
//---------------------------------------------------------------------------------
void CDLLWindow::Resize (void)
{ if (surf) FreeSurface (surf);
  surf  = CreateSurface (400, 400);
#ifdef _DEBUG
  //TRACE ("CREATE DLL SURF");
#endif
  return;
}
//---------------------------------------------------------------------------------
//  Time slice called from CSituation
//---------------------------------------------------------------------------------
void  CDLLWindow::TimeSlice (float dT)
{ if (0 == surf) return;
  // sdk: Draw the DLLDraw
  //--------------------------------------------------------------------
  //if (globals->plugins_num) globals->plugins->On_Draw (obj, surf);
  //--- e.g. Draw a line in a windows ---------------------------
  //EraseSurfaceRGB (surf, back);
  //DrawFastLine (surf, 0, 0, 500, 5, black);
  return;
}
//---------------------------------------------------------------------------------
//  Draw the surface
//  Should be called  from CFuiManager contex
//---------------------------------------------------------------------------------
void  CDLLWindow::Draw (void)
{ if (0 == surf) return;
  // Starting raster position is bottom-left corner of the surface,
  //   with (0,0) at top-left of the screen
  // sdk :
  if (globals->plugins_num) {
    glRasterPos2i (surf->xScreen, surf->yScreen + surf->ySize);
    //
    globals->plugins->On_Draw (obj, surf, dll); // 
    // Blit is performed in the dll
    //glDrawBuffer (GL_BACK);
    //glDrawPixels  (surf->xSize,   surf->ySize,
    //    GL_RGBA,
    //    GL_UNSIGNED_BYTE,
    //    surf->drawBuffer);
  }
  return;
}

//////////////////////////////////////////////////////////////////////
// utility definitions
//////////////////////////////////////////////////////////////////////
/*
void GetfnPlug1 (const std::vector <HMODULE>::iterator it_)
{
  int counter = 0;
    printf ("Trying to get ProcAddress %d ...\n", *it);
    PFUNC1 pFunc1 = (PFUNC1)GetProcAddress ((HINSTANCE)(*it), "fnPlug1");
    if (pFunc1 != NULL) {
      ++counter;
      int n1 = pFunc1 ();
      printf ("The answer #%d is %d\n", counter, n1);
    } else {
      printf ("pFunc is %d\n", pFunc1);
    }
}

void GetfnPlug2 (const std::vector <HMODULE>::iterator it_)
{
  int counter = 0;
    PFUNC2 pFunc2 = (PFUNC2)GetProcAddress ((HINSTANCE)(*it), "fnPlug2");
    if (pFunc2 != NULL) {
      ++counter;
      int val = 0;
      int n2 = pFunc2 (val);
      printf ("The answer #%d is %d val = %d\n", counter, n2, val);
    } else {
      printf ("pFunc is %d\n", pFunc2);
    }
}
*/

//DLL_INTERFACE int	DLLInit(DLLFunctionRegistry *dll, SDLLCopyright *copyright, SDLLRegisterTypeList **types)
void GetDLLInit (const std::vector <HMODULE>::iterator it_)
{
#ifdef _DEBUG
  //DEBUGLOG ("             GetDLLInit");
#endif
  int counter = 0;
  PFUNC3 pFunc3 = (PFUNC3)GetProcAddress ((HINSTANCE)(*it), "DLLInit");
  if (pFunc3 != NULL) {
    ++counter;
    //MEMORY_LEAK_MARKER ("dll 1"); 
    DLLFunctionRegistry *dll = new DLLFunctionRegistry;
    //MEMORY_LEAK_MARKER ("dll 2");
    /////////////////////////////////////
    dll->DLLTest = NULL;
    dll->DLLStartSituation = NULL;
    dll->DLLEndSituation = NULL;

	  dll->DLLDestroyObject = NULL;		    // DLLDestroyObject;
	  dll->DLLReceiveMessage = NULL;		    // DLLReceiveMessage;
	  dll->DLLInstantiate = NULL;		    // DLLInstantiate;
	  dll->DLLInitObject = NULL;		        // DLLInitObject;
    dll->DLLIdle = NULL;
	  dll->DLLDraw = NULL;			        // DLLDraw;
	  dll->DLLRead = NULL;			        // DLLRead;
	  dll->DLLReadFinished = NULL;		    // DLLReadFinished;
	  dll->DLLWrite = NULL;		        	// DLLWrite;
	  dll->DLLMouseClick = NULL;		        // DLLMouseClick;
	  dll->DLLTrackClick = NULL;		        // DLLTrackClick;
	  dll->DLLStopClick = NULL;		        // DLLStopClick;
	  dll->DLLMouseMoved = NULL;		        // DLLMouseMoved;
	  dll->DLLPrepare = NULL;			    // DLLPrepare;
	  dll->DLLTimeSlice = NULL;		        // DLLTimeSlice;
	  dll->DLLKeyPressed = NULL;		        // DLLKeyPressed;
	  dll->DLLKeyCallback = NULL;		    // DLLKeyCallback;
	  dll->DLLProcessSoundEffects = NULL;	// DLLProcessSoundEffects;
	  dll->DLLSuspendSoundEffects = NULL;	// DLLSuspendSoundEffects;
	  dll->DLLResumeSoundEffects = NULL;	    // DLLResumeSoundEffects;
	  dll->DLLInWindowRegion = NULL;		    // DLLInWindowRegion;
	  dll->DLLGetPopupHelp = NULL;		    // DLLGetPopupHelp;
	  dll->DLLUpdateCamera = NULL;		    // DLLUpdateCamera;
	  dll->DLLIsInteriorCamera = NULL;		// DLLIsInteriorCamera;
	  dll->DLLGetCameraName = NULL;		    // DLLGetCameraName;
	  dll->DLLDrawOverlay = NULL;		    // DLLDrawOverlay;
	  dll->DLLReadIniFile = NULL;		    // DLLReadIniFile;
	  dll->DLLWriteIniFile = NULL;		    // DLLWriteIniFile;
    dll->DLLControlCanBeUsed = NULL;		// DLLControlCanBeUsed;
    dll->DLLControlButtonCount = NULL;	    // DLLControlButtonCount;
    dll->DLLControlButtonPressed = NULL;	// DLLControlButtonPressed;
    dll->DLLControlButtonName = NULL;	    // DLLControlButtonName;
    dll->DLLControlDebounceButton = NULL;	// DLLControlDebounceButton;
    dll->DLLControlHasAxis = NULL;		    // DLLControlHasAxis;
    dll->DLLControlGetAxis = NULL;		    // DLLControlGetAxis;
    dll->DLLControlGetIdentifier = NULL;	// DLLControlGetIdentifier;
    dll->DLLRunScript = NULL;		        // DLLRunScript;
    dll->DLLCallScriptFunction = NULL;	    // DLLCallScriptFunction;
    dll->DLLStopScript = NULL;		        // DLLStopScript;
    dll->DLLStopScriptByRefName = NULL;	// DLLStopScriptByRefName;
    dll->DLLGetScriptInfo = NULL;		    // DLLGetScriptInfo;
    dll->DLLBeginFrame = NULL;		        // DLLBeginFrame;
    dll->DLLKeyCallback2 = NULL;		    // DLLKeyCallback2;
    dll->DLLGetPythonMethodTable = NULL;	// DLLGetPythonMethodTable;
    dll->DLLEventNotice = NULL;		    // DLLEventNotice;
    dll->DLLEpicEvent = NULL;		        // DLLEpicEvent;
    dll->DLLGenerateTileScenery = NULL;	// DLLGenerateTileScenery;
    dll->DLLFilterMetar = NULL;		    // DLLFilterMetar;
    dll->DLLKeyIntercept = NULL;		    // DLLKeyIntercept;
    dll->DLLInitGlobalMenus = NULL;		// DLLInitGlobalMenus;

    ///////////////////////////////////// lc 110208
    //SDLLCopyright copyright;
    //SDLLRegisterTypeList *types = NULL;

//    if (NULL == deb) {
      //MEMORY_LEAK_MARKER ("deb 1");
      deb = new SDLLRegisterTypeList;
      //MEMORY_LEAK_MARKER ("deb 2");
      if (deb) {
        pac = bkup = deb;
        types [0] = deb;
        types [1] = pac;
        types [2] = bkup;
        //types_counter = 0;
      }
//    }
    //
    #ifdef _DEBUG_dll2	
    {	FILE *fp_debug;
        char buff [8] = {0};
	    if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	    {
        fprintf(fp_debug, "GetDLLInit SDLLRegisterTypeList @ %p %p %p %p\n",
          types, types [0], types [1], types [2]);
		    fclose(fp_debug); 
    }	}
    #endif
    //MEMORY_LEAK_MARKER ("pFunc3");
    pFunc3 (dll, &copyright, types);
    //MEMORY_LEAK_MARKER ("pFunc3");

    #ifdef _DEBUG_dll	
      FILE *fp_debug;
      if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
      {
	      fprintf(fp_debug, "DLLInit #%d (%d) is %s %s [%s] %d %d %d\n",  
                                counter,
                                pFunc3,
                                copyright.company,
                                copyright.programmer,
                                copyright.dateTimeVersion,
                                dll->DLLStartSituation,
                                dll->DLLIdle,
                                dll->DLLTest);
	      fclose(fp_debug); 
      }
        printf ("DLLInit #%d (%d) is %s %s %d %d %d\n",  
                                counter,
                                pFunc3,
                                copyright.company,
                                copyright.programmer,
                                dll->DLLStartSituation,
                                dll->DLLIdle,
                                dll->DLLTest);
    #endif

    //m_dll.push_back (dll);
//************************************************************************************//
    // stop the linked list
    types [1]->type = static_cast<EDLLObjectType> (NULL);
    types [1]->signature = 0;
    types [2]->next = NULL;
    // free the last pointer created in the linked chain
    if (types [1]->next) {
      #ifdef _DEBUG_dll2	
        FILE *fp_debug;
        if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
        {
          fprintf(fp_debug, "           Delete RegisteredType %p \n", types [1]);
          fclose(fp_debug);
        }
      #endif
      SAFE_DELETE (types[1]);
    }
    // end of "stop the linked list"

    SDLLData tmp_dll_data;
    tmp_dll_data.cpyrt = copyright;
    tmp_dll_data.dll_registry = dll;

    for (types[1] = types[0]; types[1] != NULL; types[1] = types[1]->next) {
      //<SDLLRegisterTypeList*, SDLLObject*>
      tmp_dll_data.typ_obj [types[1]] = NULL;
    }
    //MEMORY_LEAK_MARKER ("dll_data");
    dll_data.push_back (tmp_dll_data);
   // MEMORY_LEAK_MARKER ("dll_data");
//************************************************************************************//

  } else {
      #ifdef _DEBUG_dll	
	      FILE *fp_debug;
	      if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	      {
		      fprintf(fp_debug, "DLLInit not found (%d)\n", pFunc3);
		      fclose(fp_debug); 
	      }
          printf ("DLLInit not found (%d)\n", pFunc3);
      #endif
  }
}

//DLL_INTERFACE void DLLKill(void)
void GetDLLKill (const std::vector <HMODULE>::iterator it_)
{
    int counter = 0;
    PFUNC4 pFunc4 = (PFUNC4)GetProcAddress ((HINSTANCE)(*it), "DLLKill");
    if (pFunc4 != NULL) {
      ++counter;
//    pFunc4 ();
      if (flag_instantiate) {
        //if (globals->plugins_num) globals->plugins->On_DeleteObjects ();
        #ifdef _DEBUG_dll2
        if (plg_object) {
          FILE *fp_debug;
          if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
          {
            fprintf(fp_debug, "GetDLLKill FREE (plg_object) %p (%d)\n", plg_object, dllobjects_counter);
            fclose(fp_debug);
          }
        }
        #endif
        FreeInitialDLLObjectList ();
        //if (globals->plugins_num) globals->plugins->On_DeleteObjects ();
        SAFE_FREE (plg_object);
        flag_instantiate = false;
      }
//    for (idll_data = dll_data.begin (); idll_data != dll_data.end (); ++idll_data) {

      for ((*idll_data).ityp_obj = (*idll_data).typ_obj.begin (); 
           (*idll_data).ityp_obj != (*idll_data).typ_obj.end ();
         ++(*idll_data).ityp_obj) {
        #ifdef _DEBUG	
          FILE *fp_debug;
          if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
          {
            char buf1 [8] = {0}, buf2 [8] = {0};
            TagToString (buf1, (*idll_data).ityp_obj->first->type); 
            TagToString (buf2, (*idll_data).ityp_obj->first->signature); 
            fprintf(fp_debug, "GetDLLKill Delete RegisteredType %s %s\n", buf1, buf2);
            fclose(fp_debug);
          }
        #endif
        SDLLRegisterTypeList *free_ = (*idll_data).ityp_obj->first;
        if (free_->type == TYPE_DLL_WINDOW){
        } else
        if (free_->type == TYPE_DLL_GAUGE){
        } else
        if (free_->type == TYPE_DLL_SYSTEM){
        } else
        if (free_->type == TYPE_DLL_VIEW){
        } else
        if (free_->type == TYPE_DLL_CAMERA){
        }
        SAFE_DELETE (free_);
      }

//    }
//*/
//    if (flag_instantiate) {
//      flag_instantiate = false;
/*/
    if (types [0]) {

      for (types[1] = types[0]; types[1] != NULL; types[1] = types[2]) {
        types[2] = types[1]->next;
//        for (types[1] = types[0]; types[1] != NULL; types[1] = types[1]->next) {
//          if (types[1]) {

        if (types[1]->type == TYPE_DLL_WINDOW){
          //if (dll_surf) {
            #ifdef _DEBUG_dll2
              {FILE *fp_debug;
              if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
              {
                fprintf(fp_debug, "GetDLLKill TYPE_DLL_WINDOW\n");
                fclose(fp_debug);
              }}
            #endif
          //  APIFreeSurface (dll_surf);
          //  dll_surf = NULL;
          //}
        } else
        if (types[1]->type == TYPE_DLL_GAUGE){
        } else
        if (types[1]->type == TYPE_DLL_SYSTEM){
        } else
        if (types[1]->type == TYPE_DLL_VIEW){
        }

        #ifdef _DEBUG_dll2	
          FILE *fp_debug;
          if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
          {
            fprintf(fp_debug, "GetDLLKill Delete RegisteredType %p %p %p\n",
              types [0], types [1], types [2]);
            fclose(fp_debug);
          }
        #endif
        SAFE_DELETE (types[1]);
//          }
      }
      types [0] = NULL; // deb = NULL
    }
/*/

//    }
//*/
      pFunc4 ();

      //pac = bkup = deb = NULL;  // lc 112109

      #ifdef _DEBUG_dll	
	      FILE *fp_debug;
	      if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	      {
		      fprintf(fp_debug, "DLLKill #%d (%d)\n", counter, pFunc4);
		      fclose(fp_debug); 
	      }
          printf ("DLLKill #%d (%d)\n", counter, pFunc4);
      #endif
    } else {
      #ifdef _DEBUG_dll	
	      FILE *fp_debug;
	      if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	      {
		      fprintf(fp_debug, "DLLKill not found (%d)\n", pFunc4);
		      fclose(fp_debug); 
	      }
          printf ("DLLKill not found (%d)\n", pFunc4);
      #endif
    }
}

