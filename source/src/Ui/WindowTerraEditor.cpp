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
#define TED_PROF (PROF_NO_PLN+PROF_NO_MET+PROF_DR_DET+PROF_TRACKE+PROF_NO_TEL+PROF_EDITOR)
//========================================================================================
//  Window for terrain editor
//========================================================================================
CFuiTED::CFuiTED(Tag idn, const char *filename)
:CFuiWindow(idn,filename,500,108,0)
{ mini  = 1;
	abtn	= 1;
	obtn	= 1;
	//-----------------------------------------------
	mdif	= 0;
	count	= 0;
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
	elvm->SetRepeat(0.5);
	elvp  = new CFuiButton(95,44,36,20,this);
	AddChild('elvp',elvp,"+",FUI_REPEAT_BT);
	elvp->SetRepeat(0.5);
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
	ldet.clear();
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
	spot = globals->tcm->GetSpot();			// Rabbit spot
	spot->Edit(txt);										// Edit tile coordinates
	lab1->SetText(txt);
	mBox->BackColor(warn[mdif]);				// Warning color
	CFuiWindow::Draw();
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
{	RegisterFocus(0);										// Free textedit
	bool		hit = rcam->PickObject(mx, my);						
	return	hit;
}
//-----------------------------------------------------------------------
//  Edit elevation
//-----------------------------------------------------------------------
void CFuiTED::EditElevation(float e)
{	char txt[32];
	sprintf(txt,"%0.2f",e);
	lab3->SetText(txt);
	return;
}
//-----------------------------------------------------------------------
//  Modify elevation
//-----------------------------------------------------------------------
void CFuiTED::IncElevation(float dte)
{	float e = trak->IncElevation(dte);
	mdif		= 1;
	count++;
	WarnM1();
	return EditElevation(e);
}
//-----------------------------------------------------------------------
//  Convert text into elevation
//-----------------------------------------------------------------------
void CFuiTED::GetElevation()
{	float  e	= 0;						
	char  *s	= lab3->GetText();
	double v	= 0;
	char *p;
	v = strtod (s,&p);
  e = float(v);
	//--- check if value is ok ----------
	if (*p)	{lab3->SetText("Error"); return;}
	trak->SetElevation(e);
	mdif		= 1;
	count++;
	WarnM1();
	return EditElevation(e);
}
//-----------------------------------------------------------------------
//  One modification is done
//	Locate all QGT where the vertex is modified
//-----------------------------------------------------------------------
void	CFuiTED::TileNotify(CmQUAD *qd,CVertex *vt)
{	U_INT ax = qd->GetTileAX();
	U_INT	az = qd->GetTileAZ();
	EnterKey(ax,az);
	//--- Check for a left tile (DET-VTX= (00-00)----
	U_INT tx =  ax & TC_032MODULO;  // Isolate DET-VRT
	//--- Tile is left side of QGT------------------- 
	if (0 == tx)
	{	U_INT x = (ax - 1) & TC_QGT_DET_MOD;	// Left tile index
		EnterKey(x,az);
	}
	//--- Tile is right side of QGT -----------------
	if (TC_032MODULO == tx)
	{	U_INT x = (ax + 1) & TC_QGT_DET_MOD;	// right tile index
		EnterKey(x,az);
	}
	//--- Check for a south border tile ------------
	U_INT tz = az & TC_032MODULO;  // Isolate DET-VRT
	//--- Tile is south border ---------------------
	if (0 == tz)
	{	U_INT z = (az - 1) & TC_QGT_DET_MOD;	// Lower tile border
		EnterKey(ax,z);
	}
	//--- Tile is north border ---------------------
	if (QGT_OTHER_SIDE == tz)
	{	U_INT z = (az + 1) & TC_QGT_DET_MOD;	// Upper tile border
		EnterKey(ax,z);
	}
	return;
}
//-----------------------------------------------------------------------
//  Enter modification if it is not in table
//-----------------------------------------------------------------------
void CFuiTED::EnterKey(U_INT tx,U_INT tz)
{	U_INT key = (tx << 16) | tz;
	std::map<U_INT,U_INT>::iterator iq = ldet.find(key);
	if (iq == ldet.end())	ldet[key] = key;
	return;
}
//-----------------------------------------------------------------------
//  Save all patches
//-----------------------------------------------------------------------
void CFuiTED::SaveAll()
{	if (0 == mdif)	return;
	pbuf.dir = 0;												// Out direction
	std::map<U_INT,U_INT>::iterator im;
	for (im=ldet.begin(); im!=ldet.end(); im++) SaveOne((*im).first);
	ldet.clear();
	mdif	= 0;
	count	= 0;
	WarnM1();
	return;
}
//-----------------------------------------------------------------------
//	Save one QUAD.  Check if still in cache before
//-----------------------------------------------------------------------
void CFuiTED::SaveOne(U_INT key)
{	U_INT		ax	= (key >> 16);
	U_INT		az  = (key & 0x0000FFFF);
	U_SHORT qx	= (ax >> TC_BY32);
	U_SHORT qz  = (az >> TC_BY32);
	C_QGT	*qgt	= globals->tcm->GetQGT(qx,qz);
	if (0 == qgt)				return Warn01();
	if (qgt->NotAlive())return Warn01();
	if (qgt->NoQuad())	return Warn01();
	//--- Locate QGT -------------------
	CmQUAD *qd  = qgt->GetQuad((ax & TC_032MODULO),(az & TC_032MODULO));
	pbuf.subL		= qd->GetLevel();
	pbuf.nbe		= 0;												// Init counter
	qd->ProcessPatche(pbuf);
	//--- Write in database ------------
	pbuf.qKey	= qgt->FullKey();
	pbuf.dtNo	= qd->GetTileNo();
	globals->sqm->DeletePatche(pbuf);
	globals->sqm->WritePatche(pbuf);
	return;
}
//-----------------------------------------------------------------------
//	Warning
//-----------------------------------------------------------------------
void CFuiTED::WarnM1()
{	char txt[128];
  if (count > 20)
	{	sprintf(txt,"You have made about %d modifications.\nTIME TO SAVE??",count);
	  wrn1->SetColour(warn[0]);
	  wrn1->SetText(txt);
	}
	else
	{ wrn1->SetText("");
	}
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
{ if (0 == mdif)		return true;
	//--- Set alert message for closing ---------
	char txt[128];
	sprintf(txt,"you have %d modifications.\n WANT TO SAVE??",count);
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
		return IncElevation(+0.5);
		//--- Decrease elevation -------------------------
	case 'elvm':
		return IncElevation(-0.5);
		//--- Enter text ---------------------------------
	case 'lab3':
		if (evn == EVENT_TEXTENTER)	GetElevation();
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
//==========================END OF FILE ==========================================================
