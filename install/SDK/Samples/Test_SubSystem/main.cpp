// main.cpp : Defines the entry point for the DLL application.
//

#include "main.h" // FILE

/////////////////////////////////////////////////////////////
/*
	/////
	//
	//	Test
	//
	/////

	<subs> -- --
	dpnd
	<bgno>
		<unId> -  -
		tst1
	<endo>

	////////////////////////////////
	//
	//	
	//	TST1_DLL
	//
	////////////////////////////////

	<subs> -- 
	TST1
	<bgno> -- modify key in banner options
		<unId>
		TST1
    <test>
    2
	  <mesg> -- connect to Airspeed Subsystem --
	  <bgno>
		  <conn>
		  tst1
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
SMessage	*syst_message = NULL;
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
  if ((type == TYPE_DLL_SYSTEM) && (id == DLL_TYPE_ID))
  {
	  *object = gRegistry.APICreateDLLObject(NULL);
	  storage = (DLLStorage *) gRegistry.APIAllocMem (sizeof (DLLStorage));

	  (**object).dllObject = (void *) storage;
	  memset((**object).dllObject, 0, sizeof (DLLStorage));

    compt = 0;

    syst_message = new SMessage;
    memset (syst_message, 0, sizeof (SMessage)); 

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
        DLL_NAME, buff, *object, storage, syst_message);
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

  if (syst_message) {
    delete (syst_message);
    syst_message = NULL;
  }
}

void DLLTimeSlice (SDLLObject *refObject, const float deltaTime)
{
  // verify this function is the intended recipient
  //if (storage != (DLLStorage *) refObject->dllObject) return;

#ifdef _DEBUG
  if (compt) goto jump;
  compt++;
  {	FILE *fp_debug;
	if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	{
		fprintf(fp_debug, "==DLLTimeSlice %s\n", DLL_NAME);
		fclose(fp_debug); 
  }}
  jump:;
#endif

    syst_message->dataType = TYPE_INT;
    syst_message->intData = 0;
    gRegistry.APISendMessage (syst_message);
	  //char	buffer[64];
	  //sprintf(buffer, "%.0f ft", syst_message->realData);

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
			gRegistry.APIReadMessage (syst_message, stream);
			syst_message->id = MSG_SETDATA;
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
          result, stream, &syst_message, value);
		    fclose(fp_debug); 
     }}
  }
  #endif

	return (result);
}

