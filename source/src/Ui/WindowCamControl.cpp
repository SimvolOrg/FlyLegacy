/*
 * WindowCamControl.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005 Chris Wallace
 * Copyright 2007 Jean Sabatier
 *
 * Fly! Legacy is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 * Fly! Legacy is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 *   along with Fly! Legacy; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include "../Include/FlyLegacy.h"
#include "../Include/Utility.h"
#include "../Include/Globals.h"
#include "../Include/Fui.h"
#include "../Include/FuiUser.h"
#include <vector>
#include <stdio.h>
//=====================================================================================
//  Window to control camera
//=====================================================================================
CFuiCamControl::CFuiCamControl(Tag idn, const char *filename)
:CFuiWindow(idn,filename,240,180,0)
{	SetProperty(FUI_TRANSPARENT);
	SetProperty(FUI_NO_BORDER);
	//---- Set the back bitmap ----------------------
	SetBackPicture("CamControl.bmp");
	CFuiWindow::ReadFinished();
	//----Create all buttons ------------------------
	U_INT col = MakeRGBA(255,255,255,255);
	lrot	= new CFuiLabel (43,48,47,19,this);
	lrot->RazProperty(FUI_NO_BORDER);
	lrot->SetColour(col);
	AddChild('lrot',lrot,"");
	rtlf  = new CFuiButton( 6,48,36,20,this); 
  AddChild('rtlf',rtlf,"-",FUI_REPEAT_BT);
	rtrt  = new CFuiButton(91,48,36,20,this);
	AddChild('rtrt',rtrt,"+",FUI_REPEAT_BT);
	rtup  = new CFuiButton(40,26,52,20,this);
	AddChild('rtup',rtup,"+",FUI_REPEAT_BT);
	rtdn  = new CFuiButton(40,68,52,20,this);
	AddChild('rtdn',rtdn,"-",FUI_REPEAT_BT);
	//----Create range items ------------------------
  lrng  = new CFuiLabel (168,48,36,19,this);
	lrng->RazProperty(FUI_NO_BORDER);
	lrng->SetColour(col);
	AddChild('lrng',lrng,"");

	rnut  = new CFuiButton(168,26,36,20,this);  
  AddChild('rnut',rnut,"+",FUI_REPEAT_BT);
	rnin  = new CFuiButton(168,68,36,20,this);  
  AddChild('rnin',rnin,"-",FUI_REPEAT_BT);
	//----Create zoom items -------------------------
	lzom  = new CFuiLabel (101,130,38,19,this);
	lzom->RazProperty(FUI_NO_BORDER);
	lzom->SetColour(col);
	AddChild('lzom',lzom,"");
	zmin  = new CFuiButton( 64,129,36,20,this); 
	AddChild('zmin',zmin,"-",FUI_REPEAT_BT);
	zmup  = new CFuiButton(140,129,36,20,this); 
	AddChild('zmup',zmup,"+",FUI_REPEAT_BT);

}
//---------------------------------------------------------------------
//	Edit values 
//---------------------------------------------------------------------
void CFuiCamControl::EditValues()
{	char edt[128];
	float a1 = cam->GetAzimuth();
	float e1 = cam->GetElevation();
	sprintf(edt," %.0f %.0f",RadToDeg(a1),RadToDeg(e1));
	lrot->SetText(edt);

	//----Range ---------------------------------------
	float rg = cam->GetRange();
	sprintf(edt," %.0fft",rg);
	lrng->SetText(edt);

	//----Field of view -------------------------------
	float zm = cam->GetFOV();
	sprintf(edt," %.0fd",zm);
	lzom->SetText(edt);
	return;
}
//---------------------------------------------------------------------
//	Click inside 
//---------------------------------------------------------------------
bool CFuiCamControl::InsideClick (int mx, int my, EMouseButton buttons)
{	if (!(buttons & MOUSE_BUTTON_LEFT))		return true;
	//--- save coordinates for moving ------------------------
	MoveWindow(mx,my);
	return true;
}
//---------------------------------------------------------------------
//	Mouse move: Process according to state
//---------------------------------------------------------------------
bool	CFuiCamControl::MouseMove (int x, int y)
{	CFuiWindow::MouseMove(x,y);
	return true;
}
//---------------------------------------------------------------------
//	Draw the window
//	Get current camera and set rotation rate
//---------------------------------------------------------------------
void CFuiCamControl::Draw()
{	cam		= globals->cam;
  float  rate = cam->GetRate();
	rtlf->SetRepeat(rate);
	rtrt->SetRepeat(rate);
	EditValues();
	CFuiWindow::Draw();
}
//---------------------------------------------------------------------
//  Intercept events
//---------------------------------------------------------------------
void CFuiCamControl::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ switch (idm)  {
    case 'sysb':
      return SystemHandler(evn);
		//--- Camera pan/rotate  left ------
		case 'rtlf':
			return cam->PanLeft();
		//--- Camera pan/rotate right ------
		case 'rtrt':
			return cam->PanRight();
		//--- Camera pan/rotate up ---------
		case 'rtup':
			return cam->PanUp();
	  //---- Camera pan/rotate down ------
		case 'rtdn':
			return cam->PanDown();
		//--- Camera zoom in ---------------
		case 'zmin':
			return cam->ZoomRatioIn();
		//--- Camera zoom out --------------
		case 'zmup':
			return cam->ZoomRatioOut();
		//--- Range in --------------------
		case 'rnin':
			return cam->RangeIn();
		//--- Range out -------------------
		case 'rnut':
			return cam->RangeOut();
	}
return;
}

//=======================END OF FILE ==================================================