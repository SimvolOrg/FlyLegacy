/*
 * WindowOptions.cpp
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
#include "../Include/FuiOption.h"
//================================================================================================
WORD Ramp[3][256];                                    // Current ramp
HDC  hdc = 0;                                           // Current HDC
//==================================================================================
//	Global function to add a file
//==================================================================================
int fileBoxCB(char *fn,void *upm)
{	CFuiFileBox *win = (	CFuiFileBox *)upm;
  //TRACE("FP add:%s",fn);
	return win->AddFileName(fn);
}

//========================================================================================
//  Window for list of files. This is a modal window
//========================================================================================
CFuiFileBox::CFuiFileBox(CFuiWindow *mwin,FILE_SEARCH *pm)
	:CFuiWindow(FUI_WINDOW_FILE_BOX,0,180,210,0)
{	MoWind	= mwin;
	FPM   = pm;
	close = FPM->close;
  //--- white color -------------------------------
	U_INT wit = MakeRGBA(255,255,255,255);
	//--- Set title ---------------------------------
	strcpy(text,"FILE SELECTION");
	//--- Create list box ---------------------------
	wFIL  = new CFuiList    ( 4, 30,170,140,this);
  wFIL->SetVScroll();
  AddChild('file',wFIL,"Files",FUI_TRANSPARENT,wit);
	//--- Add a label for directory -----------------
  wDIR  = new CFuiLabel   ( 4,  4, 170, 20, this);
  AddChild('labl',wDIR,"Directory:",FUI_BLINK_TXT,wit);
	wDIR->SetText(FPM->text);
	//--- Add a Button for selection -----------------
  sBUT  = new CFuiButton  ( 44,180, 90, 20, this);
  AddChild('_yes',sBUT,"OK");
	//--- Set transparent mode ----------------------
	SetTransparentMode();
	ReadFinished();
	//-----------------------------------------------
	FPM->sfil	= "";
	FPM->sdir = "";
	fBOX.SetParameters(this,'file',0);
	SearchFiles(FPM->dir);
	fBOX.SortAndDisplay();
}
//---------------------------------------------------------------------------
//	Close everything
//---------------------------------------------------------------------------
CFuiFileBox::~CFuiFileBox()
{ if (MoWind)	MoWind->ClearModal();
}
//---------------------------------------------------------------------------
//	Load file from directory
//---------------------------------------------------------------------------
void CFuiFileBox::LoadFiles(char *dir)
{ char fpat[PATH_MAX];
	_snprintf(fpat,FNAM_MAX,"%s/%s",dir, FPM->pat);
	path	= dir;
	ApplyToFiles(fpat,fileBoxCB,this);
}
//---------------------------------------------------------------------------
//	Search In directory
//---------------------------------------------------------------------------
void CFuiFileBox::SearchDirectory(ulDirEnt *de, char *dir)
{	char *nm = de->d_name;
	if (*nm == '.')									return;
	//--- Search here --------------------------
	char fdir[PATH_MAX];
	_snprintf(fdir,FNAM_MAX,"%s/%s",dir,nm);
	SearchFiles(fdir);
	return;
}
//---------------------------------------------------------------------------
//	Search all files
//---------------------------------------------------------------------------
void CFuiFileBox::SearchFiles(char *dir)
{ ulDir* dirp = ulOpenDir (dir);
  if (0 == dirp)				return;
	//--- Get all files in this directory --------
	LoadFiles(dir);
	if (FPM->sbdir)	
	{	//--- Scan all entries for subdirectory -----
		ulDirEnt* de;
		while ( (de = ulReadDir(dirp)) != NULL )
		{	if (!de->d_isdir)		continue; 
			SearchDirectory(de,dir);
		}
	}
  ulCloseDir(dirp);
}
//---------------------------------------------------------------------------
//	Add a file name to the list
//---------------------------------------------------------------------------
int CFuiFileBox::AddFileName(char *fn)
{	CFnmLine *lin = new CFnmLine();
	lin->SetSlotName(fn);
	lin->SetPath(path);
	fBOX.AddSlot(lin);
	return 1;
}
//---------------------------------------------------------------------------
//	File is selected
//---------------------------------------------------------------------------
void CFuiFileBox::OneFile(U_INT nb)
{ CFnmLine *lin = (CFnmLine *)fBOX.GetSelectedSlot();
	if (lin)
	{	FPM->sfil			= lin->GetSlotName();
		FPM->sdir			= lin->GetPath();
	}
	if (MoWind) MoWind->FileSelected(FPM);
	if (MoWind) MoWind->NotifyChildEvent('file','clos',EVENT_DIR_CLOSE);
	Close();
}
//---------------------------------------------------------------------------
//	Draw the box
//---------------------------------------------------------------------------
void CFuiFileBox::Draw()
{	CFuiWindow::Draw();
	//--------------------------------------------------
	return;
}
//---------------------------------------------------------------------
//  Intercept events
//---------------------------------------------------------------------
void CFuiFileBox::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{	switch (idm)  {
    case 'sysb':
      SystemHandler(evn);
      return;
		//--- Model collection ------------------
		case 'file':
			if (evn == EVENT_DBLE_CLICK)	OneFile(itm);
      else	fBOX.VScrollHandler((U_INT)itm,evn);
      return;
		case '_yes':
			OneFile(itm);
			return;
	}
	return;
}

//==========================END OF FILE ==========================================================
