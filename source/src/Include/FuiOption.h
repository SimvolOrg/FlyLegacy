//==============================================================================================
// CFuiOptin.h
//
// Part of Fly! Legacy project
//
// Copyright 2003 Chris Wallace
// Copyright 2012 Jean Sabatier
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


#ifndef CFUI_OPTION_H
#define CFUI_OPTION_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//==================================================================================
#include "../Include/Fui.h"
//==================================================================================
//  CFuiCamControl.  A window to control a camera
//==================================================================================
class CFuiCamControl: public CFuiWindow 
{	//--- ATTRIBUTES --------------------------------------------
	CCamera     *cam;											// Cockpit camera
	//--- Rotation control	-------------------------------------
	CFuiLabel   *lrot;										// Label rotation
	CFuiButton	*rtlf;										// Rotate left
	CFuiButton  *rtrt;										// Rotate right
	CFuiButton  *rtup;										// Rotate up
	CFuiButton  *rtdn;										// Rotate down
	//--- Range buttons -----------------------------------------
	CFuiLabel   *lrng;										// Label range
	CFuiButton  *rnut;										// Outward
	CFuiButton  *rnin;										// In ward
	//----Zoom items --------------------------------------------
	CFuiLabel   *lzom;										// Zoom label
	CFuiButton  *zmup;										// Zoom out
	CFuiButton  *zmin;										// Zoom in
	//----METHODS -----------------------------------------------
public:
	CFuiCamControl(Tag idn, const char *filename);
  virtual ~CFuiCamControl();
	//-----------------------------------------------------------
	void	EditValues();
  //-----------------------------------------------------------
	bool	InsideClick (int x, int y, EMouseButton buttons);
	bool	MouseMove (int x, int y);
	void	NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
	//-----------------------------------------------------------
	void	Draw();
	//--- Dont close this window ----------------------------------------
	bool    CheckProfile(char a) {return true;}
};
//=============================================================================================
// FUI Error message
//=============================================================================================
class CFuiErrorMSG : public CFuiWindow
{	//------------------------------------------------------------
	CFuiLabel  *mLAB;			// Message label
	CFuiButton *oBUT;			// Yes button
	CFuiButton *nBUT;			// No  button
public :
  CFuiErrorMSG (Tag id, char nb);
	CFuiErrorMSG (CFuiWindow *mwin, char nb);
	//--------------------------------------------------------------
	void	Init(char nb);
  void	NotifyChildEvent (Tag id, Tag component, EFuiEvents event);
	//--------------------------------------------------------------
	bool	CheckProfile(char a)	{return true;}
	//--------------------------------------------------------------
	inline void Display(char *m)	{mLAB->SetText(m);}
};

//==================================================================================
//  CFuiFileLine.  A slot for one file
//==================================================================================
class CFnmLine : public CSlot {
	char dir[PATH_MAX];							// Path name
	//-------------------------------------------------------------------
public:
	CFnmLine(): CSlot(1) {*dir = 0;}
	//-------------------------------------------------------------------
	void	SetPath(char *p)		{	strncpy(dir,p,FNAM_MAX);}
	char *GetPath()						{	return dir;}
};
//==================================================================================
//  CFuiFileList.  A window to control a list of files
//==================================================================================
class CFuiFileBox : public CFuiWindow {
	friend class CFuiManager;
	//--- ATTRIBUTES -------------------------------------------------------
	FILE_SEARCH *FPM;														// File parameters
	CFuiList   *wFIL;														// List of files
	CFuiLabel  *wDIR;														// Directory
	CFuiButton *sBUT;														// Select button
	char       *path;														// Current path
	//-----------------------------------------------------------------------
	CListBox   fBOX;
	//----------------------------------------------------------------------
private:
	bool	CheckProfile(char a)  {return true;}  // Let it pass
public:
	CFuiFileBox(CFuiWindow *mwin, FILE_SEARCH *fpm);
  virtual ~CFuiFileBox();
	//----------------------------------------------------------------------
	void			LoadFiles(char *dir);
	void			SearchDirectory(ulDirEnt *de, char *dir);
	void			SearchFiles(char *dir);
	void			OneFile(U_INT nb);
	int 			AddFileName(char *fn);
	bool			IsEmpty()				{return fBOX.IsEmpty();}
	//----------------------------------------------------------------------
	void			NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
	void			Draw();
	//----------------------------------------------------------------------
};
//======================= END OF FILE ==============================================================
#endif // CFUI_OPTION_H
