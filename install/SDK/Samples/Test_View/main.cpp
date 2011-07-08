// main.cpp : Defines the entry point for the DLL application.
//

#include "main.h" // FILE

/////////////////////////////////////////////////////////////
/*
7
spot,Spot Camera,1
orbt,Orbit Camera,0
obsr,Stationary Camera,0
flyb,Fly-By Camera,0
towr,Tower Camera,0
VIEW,Test DLL Camera,1
over,Over Camera,0
*/
/////////////////////////////////////////////////////////////

typedef struct DLLStorage
{
	SSurface *scoreSurface;

} DLLStorage;

static DLLStorage *storage = NULL;
static int compt = 0;

////////////////////////////////////////////////////////////

//************************************************************
//*                    DLLInstantiate                     *
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
  if ((type == TYPE_DLL_CAMERA) && (id == DLL_TYPE_ID))
  {
	  *object = gRegistry.APICreateDLLObject(NULL);
	  storage = (DLLStorage *) gRegistry.APIAllocMem (sizeof (DLLStorage));

	  (**object).dllObject = (void *) storage;
	  memset((**object).dllObject, 0, sizeof (DLLStorage));

    compt = 0;

//#ifdef _DEBUG
    time_t dateTime;
    time (&dateTime);
    char buff [256];
    sprintf (buff, "1.00, %s", ctime (&dateTime));
    buff[strlen (buff) - 1] = '\0';
	  FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	  {
		  fprintf(fp_debug, "DLLInstantiate %s %s (%p)(%p)\n",
        DLL_NAME, buff, *object, storage);
		  fclose(fp_debug); 
	  }
//#endif
  }
}

//
// this gets called whenever one of our
// objects is being destroyed; perform
// whatever cleanup is necessary
//
void DLLDestroyObject(SDLLObject *object)
{
  // verify this function is the intended recipient
  //if (storage != (DLLStorage *) refObject->dllObject) return;

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
}

void DLLUpdateCamera (SDLLObject *refObject, 
                      SPosition  *tgtPos, SVector *tgtOri,
                      SPosition  *camPos, SVector *camOri,
                      const float deltaTime)
{
  // verify this function is the intended recipient
  //if (storage != (DLLStorage *) refObject->dllObject) return;

#ifdef _DEBUG
  if (compt) goto jump;
  compt++;
  {	FILE *fp_debug;
	if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	{
		fprintf(fp_debug, "==DLLUpdateCamera %s\n", DLL_NAME);
		fclose(fp_debug); 
  }}
  jump:;
#endif

  camPos->lat = tgtPos->lat + 5.0;
  camPos->lon = tgtPos->lon + -2.0;
  camPos->alt = tgtPos->alt + 40.0;
  // RH convention
  camOri->x = 0.00;                // pitch
  camOri->y = 0.00;                // roll
  camOri->z = 0.05;                // head
  //
  if (1) {
    gRegistry.APIZoomCamera (15.0);
  }
}



