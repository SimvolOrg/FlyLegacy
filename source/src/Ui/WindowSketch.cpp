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
//  Roof model N°1
//										E---------------F			height = 1 A= origin (0,0,0)
//								D-------A      C--------B
//==========================================================================================
//----- Vertices for model 1 --------------------------------
SVector    vM1[] =
{	{0,0,0},													// 0 Point A
	{1,0,0},													// 1 Point B
	{1,2,0},													// 2 point C
	{0,2,0},													// 3 Point D
	{0,1,1},													// 4 point E
	{1,1,1},													// 5 point F
};
//-------Indices for model 1-------------------------------
short xM1[]=
	{	0,4,3,				// T(AED)	
	  2,5,1,				// T(CFB)
	  4,0,1,				// T(EAB)
	  4,1,5,				// T(EBF)
	  5,2,4,				// T(FCE)
	  4,2,3,				// T(ECD)
	};
//------Instance for model 1 -----------------------------
CRoofModel roofM1(6,vM1,18,xM1);
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
	ProcessFile("Objects/Obj05.txt");
	//-----------------------------------------------
	ctx.prof	= PROF_SKETCH;
	ctx.mode	= SLEW_RCAM;
  rcam			= globals->ccm->SetRabbitCamera(ctx,this);
	//-----------------------------------------------
	globals->cam->SetRange(40);
	SetOptions(TRITOR_ALL);
};
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
void CFuiSketch::ProcessFile(char *fn)
{	//---- Open a file ---------------
	if (!trn->Load(fn))						return;
	int np = trn->NbPoints();
	if (!trn->QualifyPoints())		return;
	if (!trn->Triangulation())		return;
	trn->QualifyFaces();
	int etg = (np != 4)?(5):(2);
	trn->Extrude(etg,2.5);
	if (np == 4) trn->ChangeRoof(roofM1);
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
