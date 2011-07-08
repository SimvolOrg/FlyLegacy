// AIPlane.h: interface for the CAIPlane class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AIPLANE_H__433CBB29_F3FA_444E_9D93_A860876CFE0C__INCLUDED_)
#define AFX_AIPLANE_H__433CBB29_F3FA_444E_9D93_A860876CFE0C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
//#include <math.h>

#include "utilities\writefile.h"
#include "utilities\fileelementaryop.h"

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

#endif // !defined(AFX_AIPLANE_H__433CBB29_F3FA_444E_9D93_A860876CFE0C__INCLUDED_)
