//==============================================================================================
// External.h
//
// Part of Fly! Legacy project
//
// Copyright 2003 Chris Wallace
// Copyright 2005 Jean Sabatier
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
//================================================================================================

/*! \file Version.h
 *  \brief Definitions for application version control
 */


#ifndef CNAVPLUG__H
#define CNAVPLUG__H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//===============================================================================
// Exchange area
//===============================================================================
struct CNAV_PLUG {
	U_INT   OK;										// 4 bytes
	//--- Date ---------------------------
	SDate   date;									// 12 Bytes
	//--- Time ---------------------------
	STime   time;									// 16 bytes
	//--- Position -----------------------
	double	lat;									// Latitutde (degrés)
	double	lon;									// Longitude (degrés)
	double	alt;									// Altitude
	//--- Speed (knots ) -----------------
	double	speed;								// Speed
	//--- Heading -------------------------
	double  orien;								// Orientation (degré)
	double  head;									// Magnetic heading
};
//===============================================================================
//	Class to connect to the external application: Navigation
//===============================================================================
class CNavPlug: public CExecutable, public CStreamObject {
	//--- ATTRIBUTES -------------------------------------------
	U_CHAR		state;							// Logical state
	float 		time;								// Timer
	HANDLE		Map;								// Shared memory file
	LPCTSTR   vue;								// View handle
	CNAV_PLUG	zex;								// Exchange buffer
	//--- For rtest ---------------------------------------------
	HANDLE		rmp;								// Shared memory file
	LPCTSTR   rvu;								// View handle
protected:
	//--- METHODS ----------------------------------------------
public:
	CNavPlug();
 ~CNavPlug();
	void		Error(char *t);
	void		Init();
	void		Close();
	int			TimeSlice(float dT, U_INT frame);
	//-----------------------------------------------------
	bool		IsConnected()				{return (0 != state);}
};
//======================= END OF FILE ==============================================================
#endif // CNAVPLUG__H
