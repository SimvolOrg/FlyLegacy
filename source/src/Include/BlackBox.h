//===============================================================================
// BlackBox.cpp
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

#ifndef BLACKBOX_H
#define BLACKBOX_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Include/FlyLegacy.h"
//==============================================================================
#define BB_SIZE				(32)
#define BB_MASK				(BB_SIZE - 1)
//==============================================================================
//	Structur to save data
//==============================================================================
struct BB_SLOT {
	U_INT	frame;													// Frame number
	Tag		action;													// Action type
	void *item;														// Item parameter
	U_INT pm1;														// Parameter
};
//==============================================================================
//	BlackBox to register trace of subsystems for debug purpose
//==============================================================================
class BlackBox {
protected:
	//--- attributes -----------------------------------------------
	char     *name;												// Name
	U_CHAR    dim;												// Dimension
	U_CHAR		ind;												// Current indice
	//--------------------------------------------------------------
	BB_SLOT   buf[BB_SIZE];								// Register
	//--- METHODS --------------------------------------------------
public:
	BlackBox();
	//--------------------------------------------------------------
	void	Enter(Tag a,void *p1, U_INT p2);
	void	Dump();
	//--------------------------------------------------------------
	virtual void Edit()	{;}
};
//==============================================================================
//	BlackBox for terrain cache
//==============================================================================
class BBcache: public BlackBox {
	//---Edit level 2-----------------------
public:
	BBcache(char *n): BlackBox() {name = n;}
  void Edit();
};
//==============================================================================
//	BlackBox for airport cache
//==============================================================================
class BBairport: public BlackBox {
	//---Edit level 2 -----------------------
public:
	BBairport(char *n): BlackBox() {name = n;}
	void	Edit();
};
//==============================================================================
//	MASTER BLACK BOX
//==============================================================================
class BBM : public BlackBox 
{	// Attributes --------------------------------------------------
	std::vector<BlackBox*> boxes;				// List of boxes
	//--- METHODS --------------------------------------------------
public:
	BBM()	{name = "MASTER";}
	//--- Register a new black box ---------------------------------
	void			DumpAll();
	//--------------------------------------------------------------
	BBcache			*GetTcmBox(char *name);
	BBairport		*GetAptBox(char *name);
};
//======================= END OF FILE ==========================================
#endif BLACKBOX_H
