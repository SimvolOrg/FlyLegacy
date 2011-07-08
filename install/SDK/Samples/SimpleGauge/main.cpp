// main.cpp : Defines the entry point for the DLL application.
//

#include "main.h" // FILE

/////////////////////////////////////////////////////////////
/*
<gage> ==== GAUGE ENTRY ====
S__G
<bgno> ==== BEGIN GAUGE ENTRY ====
	<unid> ---- unique id ----
	S__G
	<size> ---- x,y,xsize,ysize ----
	197
	170
	90
	90
	<test>
	15
	<mesg> -- connect to Airspeed Subsystem --
	<bgno>
		<conn>
		alti
		<user>
		HARDWARE,GAUGE
		<user>
		UNIT,1
	<endo>
<endo>
*/
/////////////////////////////////////////////////////////////

typedef struct DLLStorage
{
	SSurface *scoreSurface;

} DLLStorage;

static DLLStorage *storage = NULL;
static int compt = 0;
unsigned int green = 0;
unsigned int yellow = 0;
SMessage	*message = NULL;
int value = 0;
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
  if ((type == TYPE_DLL_GAUGE) && (id == DLL_TYPE_ID))
  {

 
	  *object = gRegistry.APICreateDLLObject(NULL);
	  //DLLStorage	*storage = new DLLStorage;
	  storage = (DLLStorage *) gRegistry.APIAllocMem (sizeof (DLLStorage));

	  (**object).dllObject = (void *) storage;
	  memset((**object).dllObject, 0, sizeof (DLLStorage));

    green = gRegistry.APIMakeRGB (0,255,0);
    yellow = gRegistry.APIMakeRGB (255,255,0);
    compt = 0;

    message = new SMessage;
    memset (message, 0, sizeof (SMessage)); 

//#ifdef _DEBUG
    time_t dateTime;
    time (&dateTime);
    char buff [256];
    sprintf (buff, "1.00, %s", ctime (&dateTime));
    buff[strlen (buff) - 1] = '\0';
	  FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	  {
		  fprintf(fp_debug, "DLLInstantiate %s %s (%p)(%p) [%p]\n",
        DLL_NAME, buff, *object, storage, message);
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

  if (message) {
    delete (message);
    message = NULL;
  }
}

void DLLDraw (SDLLObject *refObject, SSurface *surface)
{
  // verify this function is the intended recipient
  //if (storage != (DLLStorage *) refObject->dllObject) return;

  //gRegistry.APISendMessage (&message);

#ifdef _DEBUG
  if (compt) goto jump;
  compt++;
  {	FILE *fp_debug;
	if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	{
		fprintf(fp_debug, "==DLLDraw %s %p\n", DLL_NAME, surface);
		fprintf(fp_debug, "==DLLDraw %d %d %d %d\n", 
      surface->xScreen, surface->yScreen,
      surface->xSize, surface->ySize);
		fclose(fp_debug); 
  }}
  jump:;
#endif

    gRegistry.APISendMessage (message);
	  char	buffer[64];
	  sprintf(buffer, "%.0f ft", message->realData);

    if (surface) {
      gRegistry.APIEraseSurface (surface);
      //////////////////////////////////////////////////////////////////////////////
      gRegistry.APIDrawBitmap (surface, &bitmap, 0, 0, 0);
	    gRegistry.APIDrawLine   (surface, 00, 50, 90, 50, green);
	    gRegistry.APIDrawLine   (surface, 00, 00, 90, 90, green);
      gRegistry.APIDrawText   (surface, &drawFont, 62, 60, yellow, buffer);
      //////////////////////////////////////////////////////////////////////////////
	    // BLIT 
      gRegistry.APIBlit (surface);
    }
}

int	DLLRead (SDLLObject *object, SStream *stream, unsigned int tag)
{
	int		result = TAG_READ;

	//
	//	The only custom piece of data we are reading 
	//	from the gauge stream (in the .PNL file)
	//	is the message block. The message block 
	//	identifies the unique source of the data
	//	we want access to. We'll use this message
	//	block along with the APISendMessage call
	//	to poll the data for updates, then draw
	//	it during our draw callback.
	//

	switch (tag)
	{
		case 'mesg':
			gRegistry.APIReadMessage (message, stream);
			message->id = MSG_GETDATA;
      result = TAG_READ;
			break;

		case 'test':
			gRegistry.APIReadInt (&value, stream);
      result = TAG_READ;
			break;

		default:
			result = TAG_IGNORED;
	}

  #ifdef _DEBUG
  if (result) {
    {FILE *fp_debug;
	    if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	    {
		    fprintf(fp_debug, "==DLLRead %s %p ", DLL_NAME, *object);
		    fprintf(fp_debug, "(%d) %p %p %d\n", 
          result, stream, &message, value);
		    fclose(fp_debug); 
     }}
  }
  #endif

	return (result);
}

