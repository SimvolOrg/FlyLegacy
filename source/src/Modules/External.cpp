//===============================================================================
// Externa.cpp  External interface to other systems
//
//
// Part of Fly! Legacy project
//
// Copyright 2005 Chris Wallace
// CopyRight 2007 Jean Sabatier
// Fly! Legacy is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
// Fly! Legacy is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
//   along with Fly! Legacy; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//===============================================================================
#include "../Include/Globals.h"
#include "../Include/External.h"
#include <stdio.h>
//===============================================================================
//	Connector to Navigation Application
//===============================================================================
//-----------------------------------------------------------
//	Constructor
//-----------------------------------------------------------
CNavPlug::CNavPlug()
{	state      = 0;													// Closed
	TCHAR name[]=TEXT("Local\\FlyLegacyToNavigation");
	Map = CreateFileMapping(
                 INVALID_HANDLE_VALUE,    // use paging file
                 NULL,                    // default security
                 PAGE_READWRITE,          // read/write access
                 0,                       // maximum object size (high-order DWORD)
                 sizeof(zex),             // maximum object size (low-order DWORD)
                 name);										// name of mapping object
	//--- Check state ------------------------------------------
	if (0 == Map)		Error("CreateFileMapping");
	else	Init();

}
//-----------------------------------------------------------
//	Error
//-----------------------------------------------------------
void CNavPlug::Error(char *t)
{	DWORD er = GetLastError();
	char *ter;
	FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        er,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &ter,
        0, NULL );
	TRACE("SHARED MEMORY operation %s: %s",t,ter);							 
}
//-----------------------------------------------------------
//	Init : create a view of the shared memory
//-----------------------------------------------------------
void CNavPlug::Init()
{	if (NULL == Map)	return;
	vue	= (LPTSTR) MapViewOfFile(Map,   // handle to map object
                        FILE_MAP_ALL_ACCESS, // read/write permission
                        0,
                        0,
                        sizeof(zex));
	if (NULL == vue)	return Error("MapViewOfFile");
	//----------------------------------------------
	TRACE("NAVIGATION: View  Memory OK");
	state	= 1;														// Open connexion
	memset(&zex,0,sizeof(zex));
	CopyMemory((PVOID)vue, &zex, sizeof(zex));
	time	= 0;
	//--- Enter in Dispatcher --(no drawing)--------------
	globals->Disp.Enter(this, PRIO_EXTERNAL, DISP_EXCONT, DISP_DR_NOT);
	//--- For test ----------------------------------
	/*
	char *name = "Local\\FlyLegacyToNavigation";
	rmp = OpenFileMapping(
                   FILE_MAP_ALL_ACCESS,   // read/write access
                   FALSE,                 // do not inherit the name
                   name);               // name of mapping object
	if (0 == rmp)	Error("OpenFileMapping");
	rvu = (LPTSTR) MapViewOfFile(rmp, // handle to map object
               FILE_MAP_ALL_ACCESS,  // read/write permission
               0,
               0,
               sizeof(zex));
	if (0 == rvu)	Error("MapViewOfFile"); 
	*/								
	return;
}
//-----------------------------------------------------------
//	Close connexion
//-----------------------------------------------------------
void CNavPlug::Close()
{	state	= 0;
	TRACE("NAVIGATION: Disconnect");
	//------------------------------------------
	if (vue)	UnmapViewOfFile(vue);
	if (Map ) CloseHandle(Map);
	return;
}
//-----------------------------------------------------------
//	Destructor
//-----------------------------------------------------------
CNavPlug::~CNavPlug()
{	Close();	}	
//-----------------------------------------------------------
//	TimeSlice every dT
//-----------------------------------------------------------
int CNavPlug::TimeSlice(float dT, U_INT frame)
{	zex.OK	= 1;
	time -= dT;
	if (time > 0)		return 1;
	//--- Refresh shared data -----------------------
	time	= 1;					// One second elapse
	SDateTime prm = globals->tim->GetLocalDateTime();
	zex.date	= prm.date;
	zex.date.year += 1900;
	//--- Set time ---------------------------------
	zex.time    = prm.time;
	//--- Geo position -----------------------------
	SPosition P = globals->geop;
	zex.lon			= P.lon;
	zex.lat			= P.lat;
	zex.alt			= P.alt;
	//--- Aircraft indicated airspeed --------------
	zex.speed		=  globals->pln->GetIAS();
	zex.orien		= -globals->dang.z;
	zex.head		=  globals->pln->GetMagneticDirection();
	//--- Refresh shared memory --------------------
	CopyMemory((PVOID)vue, &zex, sizeof(zex));
	return 1;
}
//====================== END OF FILE ============================================
