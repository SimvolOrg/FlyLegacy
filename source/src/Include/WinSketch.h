//=========================================================================================
// WinSketch.h
//
// Part of Fly! Legacy project
//
// Copyright 2003-2005 Chris Wallace
// copyright 2007-2012 Jean Sabatier
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
//=========================================================================================

#ifndef WINSKETCH_H
#define WINSKETCH_H
//=========================================================================================
#include "../Include/FlyLegacy.h"
#include "../Include/FlyLegacy.h"
#include "../Include/Fui.h"
#include "../include/FuiUser.h"
#include "../include/3DMath.h"
#include "../include/globals.h"
//==========================================================================================
#define PROF_SKETCH   (PROF_EDITOR+PROF_NO_TEL+PROF_NO_PLANE+PROF_NO_SIT)
//==========================================================================================
//  Window to display building sketch
//==========================================================================================
class CFuiSketch : public CFuiWindow
{	//--- ATTRIBUTES ----------------------------------------------
	CFuiGroupBox	*gBOX;					// Option group box
	CFuiLabel			*aLAB;
  CFuiCheckBox  *fOPT;					// fill Option box
  CFuiCheckBox  *wOPT;					// wall Option box
	//-------------------------------------------------------------
	COption        optD;					// Option box
	//-------------------------------------------------------------
	GLUquadricObj *sphere;
	Triangulator    *trn;
	//---Original context -----------------------------------------
  CAMERA_CTX       ctx;         // Original camera and situation
	//--- Rabbit camera -----------------------------------------
	CRabbitCamera   *rcam;
	//-------------------------------------------------------------
public:
	CFuiSketch(Tag idn, const char *filename);
 ~CFuiSketch();
  //--- File handling -----------------------------------------
	void	ProcessFile(char *fn);
	void	SetOptions(U_INT p);
	//--- Drawing() ----------------------------------------------
	void	Draw();
	void	NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
	//--- Dont close tthe windows ---------------------------------
	void CheckProfile()		{;}
};

//============================END OF FILE =================================================
#endif // WINSKETCH_H
