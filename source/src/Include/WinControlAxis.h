//==============================================================================
// WinControlAxis.h
//
// Part of Fly! Legacy project
//
// Copyright 2011 jean Sabatier
//
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
//=================================================================================
#ifndef WINCONTROLAXIS_H
#define WINCONTROLAXIS_H
#include "../Include/FlyLegacy.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//================================================================================
//=============================================================================
//  Class CFuiAxis to assign Joystick axe
//=============================================================================
class CFuiAxis: public CFuiWindow
{ //-------ATTRIBUTES --------------------------------------
  CJoysticksManager *jsm;
  //-------VEHICLE component selection ---------------------
  U_SHORT            vehNo;                 // current vehicle
  CFuiPopupMenu     *vehPOP;                // Vehicle popup
  FL_MENU            vehMEN;                // Vehicle menu
  //------ LABEL For errors ---------------------------------
  CFuiButton         *asgWIN;               // Default
  CFuiLabel          *labWIN;               // Label windows
  CFuiCheckbox       *chkWIN;               // Full checkbox
	//------ Group box for axe --------------------------------
  CFuiGroupBox			 *gp1AXE;								// Group box 1
	CFuiLabel          *devAXE;               // Label  device
	CFuiLabel          *labAXE;               // Label  axe
	CFuiSlider         *valAXE;								// slider value 
	CFuiCheckbox			 *invAXE;								// Invert box
	//------ Group Box tune -----------------------------------
  CFuiGroupBox			 *grpFOR;								// Group box force
	CFuiButton         *minFOR;               // minus button
	CFuiButton         *plsFOR;               // plus button
	//------ Neutral area -------------------------------------
	CFuiLabel          *labNEU;               // Label  neutral
	CFuiSlider         *valNEU;								// slider value 
	//-------Button control -----------------------------------
	CFuiPopupMenu     *devPOP;                // Vehicle popup
	FL_MENU            devMEN;                // Vehicle menu
	CListBox           butBOX;								// Button list
	CFuiGroupBox		  *grpHAT;								// Hat group
	CFuiCheckbox      *chkHAT;                // Hat checkbox
  //-------Detected axe -------------------------------------
  int                axeNo;                 // Selected axe
  SJoyDEF           *jsd;                   // Joystick descriptor
  //-------AXE LIST -----------------------------------------
  U_CHAR              all;                  // All option
  U_CHAR              rfu;									// Modify indicator
  CListBox            axeBOX;               // List of axes
	//--- Current selected -----------------------------------
	CSimAxe						 *axe;									// Current axe
  //----Current selected joystick --------------------------
	SJoyDEF						 *jsp;									// Selected
	//--------------------------------------------------------
  static char *vehMENU[];
	static char *devMENU[];										// List of device
  //-------METHODS -----------------------------------------
public:
  CFuiAxis(Tag idn, const char *filename);
 ~CFuiAxis();
  void      SelectVehicle(int nv);
  void      NewVehicleType(U_INT No);
  void      FillAxes(int tp);
  void      AxeDetected(CSimAxe *nax);
	//-------ACTIONS -----------------------------------------
  void      AxeAssign();
  void      AxeClear();
	void			AxeSelect(int No);
	void			AxeInvert(Tag st);
	void			AxeForce(float inc);
	void			AxeNeutral();
	void			ButtonList(int No);
	void			ButtonClick(SJoyDEF *jsd, int nbut);
	void			HatControl();
	//-------DRAWING -----------------------------------------
	void			ShowNeutral();
	void			ShowForce();
	void			Draw();
  //-------NOTIFICATIONS------------------------------------
  void      NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn);
  void      NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
};


//=== END OF FILE ================================================================
#endif // WINCONTROLAXIS_H
