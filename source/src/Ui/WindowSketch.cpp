//==========================================================================================
//
// Part of Fly! Legacy project
//
// Copyright 2005 Chris Wallace
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
//==========================================================================================
#include "../include/WinSketch.h"
#include "../Include/Triangulator.h"
#include <vector>

//==========================================================================================
//  Window to display building sketch
//==========================================================================================
CFuiSketch::CFuiSketch(Tag idn, const char *filename)
:CFuiWindow(idn,filename,220,200,0)
{ title = 1;
  close = 1;
  zoom  = 0;
  mini  = 0;
	//--- Set title ---------------------------------
	strcpy(text,"SKETCH EDITOR");
	TRACE("SKETCH EDITOR on");
	gBOX  = new CFuiGroupBox(10,8,200,80,this);
  AddChild('gbox',gBOX,"");
	//--- Label --------------------------------------
  aLAB  = new CFuiLabel   ( 4, 4,76, 20, this);
  gBOX->AddChild('labl',aLAB,"Options:");
	//---Fill option --------------------
  fOPT  = new CFuiCheckBox(80,4,160,20,this);
  gBOX->AddChild('fopt',fOPT,"Fill mode");
	//---wall option --------------------
  wOPT  = new CFuiCheckBox(80,24,160,20,this);
  gBOX->AddChild('wopt',wOPT,"draw walls");
	//-----------------------------------------------	
	ReadFinished();
	//--- Open triangulation ------------------------
	trn	= new Triangulator();
	globals->trn = trn;
	//-----------------------------------------------
	ctx.prof	= PROF_SKETCH;
	ctx.mode	= SLEW_RCAM;
  rcam			= globals->ccm->SetRabbitCamera(ctx,this);
	//-----------------------------------------------
	globals->cam->SetRange(100);
	SetOptions(TRITOR_ALL);
	ses.SetTrace(1);
	//--- Temporary ---------------------------------
	OpenSession("Session.txt");
	//OpenFile("OpenStreet/Obj05.txt");
	OpenFile("OpenStreet/House.txt");
};
//-----------------------------------------------------------------------
//	OpenSession parameters
//-----------------------------------------------------------------------
void CFuiSketch::OpenSession(char *fn)
{	bool ok = ses.ReadParameters(fn);
	if (ok)		return;
	//----Warning ---------------------------
	STREETLOG("Problem with parameter file %s",fn);
	return;
}
//-----------------------------------------------------------------------
//	destroy this
//-----------------------------------------------------------------------
CFuiSketch::~CFuiSketch()
{	globals->ccm->RestoreCamera(ctx);
	SAFE_DELETE(globals->trn);
}
//-----------------------------------------------------------------------
//	Set Option
//-----------------------------------------------------------------------
void CFuiSketch::SetOptions(U_INT q)
{	char p = 0;
	optD.Toggle(q);	
	trn->repD(optD.GetAll());
	p = optD.Has(TRITOR_DRAW_FILL);
	fOPT->SetState(p);
	p = optD.Has(TRITOR_DRAW_WALL);
	wOPT->SetState(p);
	return;
}
//-----------------------------------------------------------------------
//	Process file
//	Building contruction from contour
//-----------------------------------------------------------------------
void CFuiSketch::OpenFile(char *fn)
{	//---- Open a file -----------------
	if (!trn->Load(fn))						return;
	//--- Step 2: Process points --------
	if (!trn->QualifyPoints())		return;			
	//--- Step 2 Find a style -----------
	double sf = trn->GetSurface();
	int    sd = trn->GetSideNbr();
	D2_Style *sty = ses.GetStyle(sf,sd);
	trn->SetStyle(sty);
	//--- Step 3: Compute triangle ------
	if (!trn->Triangulation())		return;
	//--- Step 4: Face orientation ----- 
	trn->OrientFaces();
	//--- Step 5: Build walls -----------
	trn->BuildWalls();
	//--- Step 6: Select roof model -----
	trn->SelectRoof();
	//--- Step 7: Texturing -------------
	trn->Texturing();
	return;
}
//-----------------------------------------------------------------------
//	Draw the sketch
//-----------------------------------------------------------------------
void	CFuiSketch::Draw()
{	CFuiWindow::Draw();
	//--------------------------------------------------
	return;
}
//---------------------------------------------------------------------
//  Intercept events
//---------------------------------------------------------------------
void CFuiSketch::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{	switch (idm)  {
    case 'sysb':
      SystemHandler(evn);
      return;
		case 'fopt':
			SetOptions(TRITOR_DRAW_FILL);
			return;
		case 'wopt':
			SetOptions(TRITOR_DRAW_WALL);
			return;
		case 'ropt':
			SetOptions(TRITOR_DRAW_ROOF);
			return;
			}
	return;

}

//============================END OF FILE ================================================================================
