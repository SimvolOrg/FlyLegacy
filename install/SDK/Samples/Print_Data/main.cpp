// main.cpp : Defines the entry point for the DLL application.
//

#include "utility.h" // FILE

/////////////////////////////////////////////////////////////
//
//	GetUserData
//
/////////////////////////////////////////////////////////////

// test
const unsigned int	DESKTOP				      =  0;
const unsigned int	GLOBAL_MENUBAR		  = 'MBar';
const unsigned int	PLUGINS_MENU_ID_1	  = 'rotw';
#define				      PLUGINS_MENU_NAME_1	  "Plugins"
const unsigned int	MY_MENU_ITEM_3    	= 'DAT1'; // to be modified
const unsigned int	MY_MENU_ITEM_4    	= 'DAT2'; // to be modified

//////////////////////////////////////////////////////////////
typedef struct DLLStorage
{
	SSurface *scoreSurface;

} DLLStorage;

static DLLStorage *storage = NULL;
static SSurface *surf = NULL;

bool SHOW_DATA = false;

//////////////////////////////////////////////////////////////

//************************************************************
//*                    DLLInstantiate                        *
//************************************************************
//*                                                          *
//* Created at  :                                            *
//* Author      :                                            *
//* Description :                                            *
//* Parameters  :  None         -                            *
//*                Unregistered - Limited Functionality.     *
//*                                                          *
//************************************************************

void DLLInstantiate( const long type, const long id, SDLLObject **object)
{
  #ifdef _DEBUG
	  FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	  {
      char buf [128] = {0};
      TagToString (buf, type);
      fprintf(fp_debug, "==DLLInstantiate @ %s %s (%d) %d\n",
        buf, DLL_NAME, DLL_TYPE_ID, id);
		  fclose(fp_debug); 
	  }
  #endif

  if ((type == TYPE_DLL_WINDOW) && (id == DLL_TYPE_ID))
  {
	  *object = gRegistry.APICreateDLLObject(NULL);
	  //DLLStorage	*storage = new DLLStorage;
	  storage = (DLLStorage *) gRegistry.APIAllocMem (sizeof (DLLStorage));

	  (**object).dllObject = (void *) storage;
	  memset((**object).dllObject, 0, sizeof (DLLStorage));

    #ifdef _DEBUG
	    FILE *fp_debug;
	    if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	    {
        char buf [128] = {0};
        TagToString (buf, type);
        fprintf(fp_debug, "==DLLInstantiate @ object (%p) (%p)\n", *object, storage);
		    fclose(fp_debug); 
	    }
    #endif

  }
}

//
// this gets called whenever one of our
// objects is being destroyed; perform
// whatever cleanup is necessary
//
void DLLDestroyObject(SDLLObject *object)
{
    #ifdef _DEBUG
	    FILE *fp_debug;
	    if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	    {
		    fprintf(fp_debug, "==DLLDestroyObject  %s (%p-%p)\n", DLL_NAME,
          storage, (DLLStorage *) object->dllObject);
		    fclose(fp_debug); 
	    }
    #endif

//  DLLStorage	*storage = (DLLStorage *) object->dllObject;
//	if (storage) delete storage;
  if (storage) {
    gRegistry.APIFreeMem (storage);
    storage = NULL;
  }

  if (surf) gRegistry.APIFreeSurface (surf);
}

void DLLDraw (SDLLObject *refObject, SSurface *surface)
{
  // DLLStorage	*storage = (DLLStorage *) refObject->dllObject;
  
  // verify whether to show data or not
  if (0 == SHOW_DATA) return;

  // verify this function is the intended recipient
//  if (storage == (DLLStorage *) refObject->dllObject) {

  #ifdef _DEBUG
    if (compt) goto jump;
    compt++;

    {	FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	  {
		  fprintf(fp_debug, "==DLLDraw %8s %p %p %p\n",
        DLL_NAME, refObject, surface, surf);
		  fclose(fp_debug); 
    }}
    jump:;
  #endif

    GetUserAircraftData ();
    if (surface) {
      //surface->xScreen = (100);
      //surface->yScreen = (600);

	    //* transparent surface */
      gRegistry.APIEraseSurfaceRGB (surface, 0);
      //////////////////////////////////////////////////////////////////////////////
      sprintf (buff, "ori    %06.2f %06.2f %06.2f\n", userOri.x, userOri.y, userOri.z);
      gRegistry.APIDrawText (surface, &drawFont, 10, 10,  yellow, buff);
      sprintf (buff, "ang.v. %06.2f %06.2f %06.2f\n", userAng.x, userAng.y, userAng.z);
      gRegistry.APIDrawText (surface, &drawFont, 10, 30,  yellow, buff);
      sprintf (buff, "Alt.   %06.2f\n", userPos.alt);
      gRegistry.APIDrawText (surface, &drawFont, 10, 50,  yellow, buff);
      //////////////////////////////////////////////////////////////////////////////
	    // BLIT 
      gRegistry.APIBlit (surface);
    }
//  }
}

//************************************************************
//*                    DLLStartSituation                     *
//************************************************************
//*                                                          *
//* Created at  :  11/2/2006   16:57                         *
//* Author      :  Enter Your Name                           *
//* Description :                                            *
//* Parameters  :  None         -                            *
//*                Unregistered - Limited Functionality.     *
//*                                                          *
//************************************************************
void DLLStartSituation (void)
{
  red    = gRegistry.APIMakeRGB (255, 20 , 100);
  green  = gRegistry.APIMakeRGB (100, 255,  20);
  white  = gRegistry.APIMakeRGB (255, 255, 255);
  yellow = gRegistry.APIMakeRGB (255, 255, 125);

//#ifdef _DEBUG
  time_t dateTime;
  time (&dateTime);
  char buff [256];
  sprintf (buff, "1.00, %s", ctime (&dateTime));
  buff[strlen (buff) - 1] = '\0';
	FILE *fp_debug;
	if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	{
		fprintf(fp_debug, "DLLStartSituation %s %s\n", DLL_NAME, buff);
		fclose(fp_debug); 
	}
//#endif
}

void DLLInitGlobalMenus(void)
{
//	#
//	# add a Plugins menu (Fly! will ignore this request if a Plugins menu already exists)
//	# APIAddUIMenu(DESKTOP, GLOBAL_MENUBAR, PLUGINS_MENU_ID, PLUGINS_MENU_NAME)

	gRegistry.APIAddUIMenu(DESKTOP, GLOBAL_MENUBAR, PLUGINS_MENU_ID_1, PLUGINS_MENU_NAME_1);

//	# 
//	# add a menu item to the Plugins menu to open our custom window
//	# APIAddUIMenuItem(DESKTOP, GLOBAL_MENUBAR, PLUGINS_MENU_ID, SIMPLE_VIEW_WINDOW, "SimpleView Sample")

	gRegistry.APIAddUIMenuItem(DESKTOP, GLOBAL_MENUBAR, PLUGINS_MENU_ID_1, MY_MENU_ITEM_3, "PRINT DATA ON/OFF");
	//gRegistry.APIAddUIMenuItem(DESKTOP, GLOBAL_MENUBAR, PLUGINS_MENU_ID_1, MY_MENU_ITEM_4, "YAGET 4");

}

//#################################################
//#
//#	EventNotice
//#
//#	Implement this routine to get
//#	notified on menu and window events
//#
//#################################################

void DLLEventNotice(SDLLObject *notify,	unsigned long windowID, unsigned long componentID, unsigned long event, unsigned long subEvent)
{
//	#
//	# handle menu item selection events
//	#---------------------------------------------

	if (event == EVENT_SELECTITEM)
	{
		if (componentID == PLUGINS_MENU_ID_1)
		{
			if (subEvent == MY_MENU_ITEM_3)
			{
         SHOW_DATA = (!SHOW_DATA);
         if (SHOW_DATA) gRegistry.APIDrawNoticeToUser ("DATA ON", 5);
         else           gRegistry.APIDrawNoticeToUser ("DATA OFF", 5);
			}
//      else
//			if (subEvent == MY_MENU_ITEM_4)
//			{
//         gRegistry.APIDrawNoticeToUser ("MY_MENU_ITEM_4", 5);
//			}
		}
	}
}



