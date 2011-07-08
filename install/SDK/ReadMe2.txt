Those files are provided to test the FlyLegacy plugin architecture
==================================================================

The FlyLegacy.lib, that is used with the dll compilation, is the lib produced during the main  FlyLegacy compilation (obtained in conjunction with the FlyLegacy exe)

The yielded dll has to be placed in a special "modules" folder in the FlyLegacy main root.

...\FlyLegacy
      \system
      \modules
      \...


A typical dll plugin workspace contains :
- Sdk.h
- Sdkintf.h
- DLLINTF.H
- FlyLegacy.lib
- MyDll.h


The MAIN.CPP has to contain those lines at the beginning :
*****************************************************************************
// add lib from main application (exe)
#pragma message ("loading FlyLegacy.lib")
#pragma comment (lib,    "FlyLegacy.lib")

#define	FOR_EXPORT
#include "mydll.h"

//  import API functions from main application (exe)
APIFunctionRegistry	gRegistry;
extern "C" void __declspec (dllimport)  GetSDKAPI (APIFunctionRegistry &api);
*****************************************************************************

And the DLL_INTERFACE int DLLInit(...) need a special line :
*****************************************************************************
	//
	//	get dsk api from application
	//
      GetSDKAPI (gRegistry);
*****************************************************************************

Please notice also that each API call is typed as "api.APISomeFunction(...)"

