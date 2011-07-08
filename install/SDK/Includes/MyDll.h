
/*
 * MyDll.h
 * 
 * Header file for DLL plug-in
 *
 *
 */

#ifndef __MYDLL__
#define __MYDLL__

#ifndef __SDKINTF__
#include "sdkintf.h"
#endif

#ifndef __DLLINTF__
#include "dllintf.h"
#endif

#define _WINDOWS_
#ifndef _WINDOWS_
#include <windows.h>
#endif

//
//	Define proper export interface
// 

#ifdef FOR_EXPORT
	#define	DLL_INTERFACE	__declspec(dllexport)
#else
	#define DLL_INTERFACE
#endif	// FOR_EXPORT

extern "C" {

//DLL_INTERFACE int   fnPlug1(void);
//DLL_INTERFACE int   fnPlug2(int &val);

DLL_INTERFACE int	DLLInit(DLLFunctionRegistry *dll, SDLLCopyright *copyright, SDLLRegisterTypeList **types);
DLL_INTERFACE void	DLLKill(void);

};	// extern "C"

#endif
