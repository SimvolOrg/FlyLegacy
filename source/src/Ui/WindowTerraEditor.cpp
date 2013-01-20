/*
 * WindowTerraEditor.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2009 Chris Wallace
 * Copyright 2007 Jean Sabatier
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
#include "../Include/Fui.h"
#include "../Include/FuiUser.h"
#include "../Include/TerrainCache.h"
//========================================================================================
#define MASK_TILE_CORNER ((TC_032MODULO << TC_BY1024) + TC_1024MOD)
#define QGT_OTHER_SIDE    (TC_032MODULO << TC_BY1024)

//=================================================================================
//  Window profile
//=================================================================================
#define TED_PROF (PROF_NO_PLANE+PROF_NO_MET+PROF_DR_DET+PROF_TRACKE+PROF_NO_TEL+PROF_EDITOR)
//==========================================================================
//	Menu
//==========================================================================
char trakTED1[32];
char trakTED2[64];
char *trakMENU[] = {"Copy elevation",trakTED1,trakTED2,0,0};
//========================================================================================
//  Window for terrain editor
//========================================================================================
CFuiTED::CFuiTED(Tag idn, const char *filename)
:CFuiWindow(idn,filename,500,108,0)
{ mini  = 1;
	abtn	= 1;
	obtn	= 1;
	//-----------------------------------------------
	count	= 0;
	mode	= 0;
	palt	= 0;
	//-----------------------------------------------
	warn[0]	= MakeRGBA(0,255,0,255);		// Green color
	warn[1]	= MakeRGBA(255,0,0,255);		// Red color
	//-----------------------------------------------
	trak	= &globals->etrk;
	trak->Register(this);
	//-----------------------------------------------
	SetProperty(FUI_TRANSPARENT);
	//--- Init labels -------------------------------
	U_INT col = MakeRGBA(255,255,255,255);
	lab1	= new CFuiLabel (10,4,296,19,this);
	lab1->SetColour(col);
	AddChild('lab1',lab1,"");
	//----Init label 2 ------------------------------
	lab2	= new CFuiLabel (10,24,200,19,this);
	lab2->SetColour(col);
	AddChild('lab2',lab2,"Elevation at Vertex:");
	//---  Elevation edit ---------------------------
	lab3	= new CFuiTextField (45,45,47,17,this);
	lab3->RazProperty(FUI_NO_BORDER);
	AddChild('lab3',lab3,"",(FUI_EDIT_MODE + FUI_TRANSPARENT));
	lab3->SetColour(col);
	lab3->SetValidMask(KB_NUMBER_SIGN);
	//---  Add buttons ------------------------------
	elvm  = new CFuiButton( 6,44,36,20,this); 
  AddChild('elvm',elvm,"-",FUI_REPEAT_BT);
	elvm->SetRepeat(0.25);
	elvp  = new CFuiButton(95,44,36,20,this);
	AddChild('elvp',elvp,"+",FUI_REPEAT_BT);
	elvp->SetRepeat(0.25);
	//--- Add  Box ----------------------------------
	mBox	= new CFuiBox( 258,6,28,28,this);
	AddChild('mbox',mBox,"",0);
	//--- Add save button ---------------------------
	sBut	= new CFuiButton(  6, 80,128,20,this);
	AddChild('save',sBut,"Save elevations");
	//--- Set yes No --------------------------------
	yBut	= new CFuiButton(262, 80,100,20,this);
	AddChild('yes_',yBut,"YES");
	yBut->Hide();
	nBut  = new CFuiButton(378, 80,100,20,this);
	AddChild('no__',nBut,"NO");
	nBut->Hide();
	//--- Init Group Edit ---------------------------
	BuildGroupEdit(300,6);
	//--- Msg1 label --------------------------------
	wrn1 = new CFuiLabel(262,46,220,40,this);
	wrn1->SetColour(warn[0]);
	AddChild('wrn1',wrn1,"",(FUI_BLINK_TXT));
	wrn1->SetTransparentMode();
	//-----------------------------------------------	
	ReadFinished();
	//--- Request to capture mouse click ------------
	globals->fui->CaptureMouse(this);
	//-----------------------------------------------
	SetTitle("TERRAIN EDITOR");
  //--- Set application profile -------------------
	ctx.prof = TED_PROF;
	ctx.mode	= SLEW_RCAM;
	rcam	= globals->ccm->SetRabbitCamera(ctx,RABBIT_S_AND_C);
	rcam->SetTracker(trak, this);
}
//----------------------------------------------------------------------
//	Delete all resources
//----------------------------------------------------------------------
CFuiTED::~CFuiTED()
{	//-----------------------------------------------
	trak->Register(0);
	globals->noOBJ -= obtn;
  globals->noAPT -= abtn;
  globals->ccm->RestoreCamera(ctx);
}
//----------------------------------------------------------------------
//	Override check profile
//----------------------------------------------------------------------
bool CFuiTED::CheckProfile(char a)
{	return true;	}
//----------------------------------------------------------------------
//	Draw the window
//----------------------------------------------------------------------
void CFuiTED::Draw()
{	char txt[128];
	char mdif = (trak->GetModif())?(1):(0);
	spot = globals->tcm->GetSpot();			// Rabbit spot
	spot->Edit(txt);										// Edit tile coordinates
	lab1->SetText(txt);
	mBox->BackColor(warn[mdif]);				// Warning color
	CFuiWindow::Draw();
	return;
}
//-----------------------------------------------------------------------
//  Unselect mode
//-----------------------------------------------------------------------
void CFuiTED::Unselect()
{	trak->Unselect();
	mode		= 0;												// No selection
	lab3->SetText("");
	return;
}
//-----------------------------------------------------------------------
//  Click detected outside any window
//	-Start picking mode for the camera
//	return true only if a hit was detected. If not
//	return false so that the mouse event can be passed on to the next
//	object in scene
//-----------------------------------------------------------------------
bool CFuiTED::MouseCapture(int mx, int my, EMouseButton bt)
{ int keym = glutGetModifiers();
	RegisterFocus(0);										// Free textedit
	Unselect();
	if (!rcam->PickObject(mx, my))		return false;
	if ( MOUSE_BUTTON_RIGHT != bt)		return false;
	menu.Ident	= 'menu';
	menu.aText	= trakMENU;
	_snprintf(trakTED1,32,"Paste %d feet",palt);
	_snprintf(trakTED2,32,"Flatten tile to %d feet",palt);
	//--- Open a  pop menu ---------------
	OpenPopup(mx,my,&menu);
	return	true;
}
//-----------------------------------------------------------------------
//  One elevation selected from tracker
//-----------------------------------------------------------------------
void	CFuiTED::OneElevation(double e)
{	alti	= int(e);
	mode	= 1;
	EditElevation();
	return;
}
//-----------------------------------------------------------------------
//  Edit elevation
//-----------------------------------------------------------------------
void CFuiTED::EditElevation()
{	char txt[32];
	_snprintf(txt,31,"%d",alti);
	char *edt = (mode)?(txt):("");
	lab3->SetText(edt);
	return;
}
//-----------------------------------------------------------------------
//  Modify elevation
//-----------------------------------------------------------------------
void CFuiTED::IncElevation(int dte)
{	if (0 == mode)		return;
	alti	 += dte;
	alti		= trak->SetElevation(alti);
	return EditElevation();
}
//-----------------------------------------------------------------------
//  Convert text into elevation
//-----------------------------------------------------------------------
void CFuiTED::StoreElevation()
{	if (0 == mode)		return;
	double e	= 0;						
	char  *s	= lab3->GetText();
	double v	= 0;
	char *p;
	e = strtod (s,&p);
	alti			= int(e);
	//--- check if value is ok ----------
	if (*p)	{lab3->SetText("Error"); return;}
	trak->SetElevation(alti);
	return  EditElevation();
}
//-----------------------------------------------------------------------
//  Save all patches
//-----------------------------------------------------------------------
void CFuiTED::SaveAll()
{	trak->SavePatches();
	return;
}
//-----------------------------------------------------------------------
//	Warning
//-----------------------------------------------------------------------
void CFuiTED::Warn01()
{}
//-----------------------------------------------------------------------
//	Alert
//-----------------------------------------------------------------------
bool CFuiTED::NoAlert()
{ count	= trak->GetModif();
	if (0 == count)		return true;
	//--- Set alert message for closing ---------
	char txt[128];
	sprintf(txt,"%d Detail Tile(s) are modified.\n WANT TO SAVE??",count);
  wrn1->SetColour(warn[1]);
	wrn1->SetText(txt);
	yBut->Show();
	nBut->Show();
	return false;
}
//-----------------------------------------------------------------------
//	Check if modifications should be saved
//-----------------------------------------------------------------------
bool CFuiTED::WantToClose()
{	if (NoAlert())	Close();
	return true;
}
//-----------------------------------------------------------------------
//	Save and Quit
//-----------------------------------------------------------------------
void CFuiTED::SaveAndQuit(char opt)
{	if (opt)	SaveAll();
	Close();
	return;
}
//-----------------------------------------------------------------------
//  Notifications
//-----------------------------------------------------------------------
void  CFuiTED::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ switch (idm)  {
  case 'sysb':
		if (evn == EVENT_CLOSEWINDOW) WantToClose();
    else													SystemHandler(evn);
    return;
	//--- Hide/show object -----------------------------
  case 'obtn':
		obtn ^= 1;
		gBox->SetChildText('obtn',ObjBTN[obtn]);
    globals->noOBJ += btnLOK[obtn];
    return;
	//--- Hide/show airport ----------------------------
  case 'abtn':
		abtn ^= 1;
		gBox->SetChildText('abtn',AptBTN[abtn]);
    globals->noAPT += btnLOK[abtn];
    return;
	//--- Increase elevation ---------------------------
	case 'elvp':
		return IncElevation(+1);
		//--- Decrease elevation -------------------------
	case 'elvm':
		return IncElevation(-1);
		//--- Enter text ---------------------------------
	case 'lab3':
		if (evn == EVENT_TEXTENTER)	StoreElevation();
		return;
		//--- Request to save elevations ------------------
	case 'save':
		SaveAll();
		return;
		//--- YES to save button -------------------------
	case 'yes_':
		SaveAndQuit(1);
		return;
	case 'no__':
		SaveAndQuit(0);
		return;
  }
  return;
}
//------------------------------------------------------------------------------
//	NOTIFICATIONS from popup menu
//------------------------------------------------------------------------------
void CFuiTED::NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn)
{	if ((EVENT_POP_CLICK  != evn)	 || (id != 'menu'))		return;
	char  opt = *menu.aText[itm];
  switch (opt) {
			//--- Copy elevation ----------------
			case 'C':
				palt	= alti;
				break;
			//--- Paste alti --------------------
			case 'P':
				alti		= trak->SetElevation(palt);
				EditElevation();
				break;
			//--- Flatten tile ------------------
			case 'F':
				alti = trak->FlattenTile(palt);
				EditElevation();
				globals->apm->KillNearest();
				break;
	}
  ClosePopup();
}

//==========================END OF FILE ==========================================================
