
// .h : include file
//

#if !defined(__MY_MAIN_HEADER__)
#define __MY_MAIN_HEADER__

#include <stdio.h> // FILE
#include <string.h> // strcpy
#include <time.h> // time


// add lib from main application (exe)
#pragma message ("loading FlyLegacy.lib")
#pragma comment (lib,    "./../../sdk/FlyLegacy.lib")

#define	FOR_EXPORT
#include "./../../sdk/mydll.h"

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
//  import API functions from main application (exe)
//  sdkintf.h

    APIFunctionRegistry	gRegistry;

    extern "C" void __declspec (dllimport)  GetSDKAPI (APIFunctionRegistry &api);

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
//  Definitions

    #define DLL_NAME "BUDDY DLL"

    void DLLStartSituation (void);
    void DLLEndSituation   (void);
    void DLLIdle           (float deltaTimeSecs);


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/


/////////////////////////////////////////////////////////////
//
//	DLL Setup (Init & Kill)
//
//	These functions MUST be implemented, or the DLL
//	will totally fail!!!!
//
/////////////////////////////////////////////////////////////

DLL_INTERFACE int	DLLInit(DLLFunctionRegistry *dll, SDLLCopyright *copyright, SDLLRegisterTypeList **types)
{
	//
	//	populate copyright info
	//
    time_t dateTime;
    time (&dateTime);
    char buff [256];
    sprintf (buff, "1.00, %s 2009Z", ctime (&dateTime));
    buff[strlen (buff) - 1] = '\0';

	strcpy(copyright->product, DLL_NAME);
	strcpy(copyright->company, "ROTW (c)");
	strcpy(copyright->programmer, "laurentC");
	strcpy(copyright->dateTimeVersion, buff);
	strcpy(copyright->email, "no@email.com");
	copyright->internalVersion = 100;

	//
	//	return function pointers of available DLL functions
	//
	dll->DLLStartSituation  = DLLStartSituation;
  dll->DLLEndSituation    = DLLEndSituation;
  dll->DLLIdle            = DLLIdle;

	//
	//	result code
	//

	//
	//	get dsk api from application
	//
    GetSDKAPI (gRegistry);

	return(1);
}

void	DLLKill(void)
{
	//
	//	nothing mandatory to do here
	//	except internal cleanup
	//
  #ifdef _DEBUG	
  {	FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	  {
		  fprintf(fp_debug, "==DLLKill %s\n", DLL_NAME);
		  fclose(fp_debug); 
  }	}
  #endif
}




#endif // __MY_MAIN_HEADER__