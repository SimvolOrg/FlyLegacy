//============================================================================================
// WindowAbout.cpp
//
// Part of Fly! Legacy project
//
// Copyright 2005 Chris Wallace
// Copyright 2007 Jean Sabatier
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
//===========================================================================================

#include "../Include/FlyLegacy.h"
#include "../Include/WinAbout.h"
//===========================================================================================
struct AB_TXT {
	char *role;
	char *name;
};
//===========================================================================================
//	Change date and version here
//===========================================================================================
char *abT00	= "Version JS 1.23";
char *abT01 = "20 jan 2013";
char *abT02 = "FlyLegacy is hosted at www.simvol.org";
char *abt03 = "(Stéphane Maillard President)";
//===========================================================================================
//	Change contribution here
//===========================================================================================
AB_TXT  abROL[] = {
	{"Project Founder",	"Chris Wallace"},
	{"Contributor",			"Luc Lyon"},
	{"Cokpit designer", "Thomas Jullien"},
	{"Tests and tuning","Frank D'hont"},
	{"Documentation",		"Robert Morelli"},
	{"Specifications",	"Jean Sabatier"},
	{"Programmation",		"Laurent Claudet"},
	{"Programmation",		"Jean Sabatier"},
	{"END"},
};

//===========================================================================================
//	Display all credits 
//===========================================================================================
CFuiAbout::CFuiAbout(Tag idn, const char *filename)
:CFuiWindow(idn,filename,240,310,0)
{ //--- white color -------------------------------
	U_INT wit = MakeRGBA(255,255,255,255);
	SetTransparentMode();
  SetText("ABOUT FlyLegacy");
	//--- Add a canva ------------------------------------
  info  = new CFuiCanva(10,4,220,300,this);
	AddChild('info',info,"",FUI_TRANSPARENT,wit);
	CFuiWindow::ReadFinished();
}
//------------------------------------------------------------------------------------
//	Draw the window 
//------------------------------------------------------------------------------------
void CFuiAbout::Draw()
{	info->BeginPage();
	info->AddText(1,abT00,1);
	info->AddText(1,abT01,2);
	Credit(info);
	info->AddText(1,abT02,1);
	info->AddText(1,abt03,1);
  CFuiWindow::Draw();
}
//------------------------------------------------------------------------------------
//	Draw the Credits 
//------------------------------------------------------------------------------------
void CFuiAbout::Credit(CFuiCanva *cnv)
{	AB_TXT *tx = abROL;
	while (strcmp(tx->role, "END") != 0)
	{	
		cnv->AddText( 1,tx->role,0);
		cnv->AddText(18,tx->name,1);
		tx++;
	}
	cnv->AddText(1,"---------------------------",2);
	return;
}
//=====================END OF FILE ==========================================================
