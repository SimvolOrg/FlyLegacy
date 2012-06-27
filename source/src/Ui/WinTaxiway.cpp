//============================================================================================
// WinTaxiway.cpp
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
#include "../Include/WinTaxiway.h"
//===========================================================================================
//	Menu for editing 
//===========================================================================================
char *taxiMENU[] = {
	"Insert a Node",
	"Delete Selected",
	"Link to",
	""};
//===========================================================================================
//	Taxiway tracker
//===========================================================================================
TaxiTracker::TaxiTracker()
{	Seq			= 1;
	selN		= 0;
	disk		= gluNewQuadric();
	gluQuadricDrawStyle(disk,GLU_FILL);
	globals->Disp.Enter(this, PRIO_ABSOLUTE, DISP_EXSTOP, 1);
}
//-------------------------------------------------------------------------------------------
//	Exit from tracker
//-------------------------------------------------------------------------------------------
TaxiTracker::~TaxiTracker()
{	Clear();
	globals->Disp.Clear(PRIO_ABSOLUTE);
	gluDeleteQuadric(disk);
}
//-------------------------------------------------------------------------------------------
//	Clear all resources
//-------------------------------------------------------------------------------------------
void TaxiTracker::Clear()
{	for (rn = nodQ.begin(); rn != nodQ.end(); rn++) delete (*rn).second;
	for (re = edgQ.begin(); re != edgQ.end(); re++)	delete (*re).second;
	nodQ.clear();
	edgQ.clear();
	return;
}
//-------------------------------------------------------------------------------------------
//	Add a node
//-------------------------------------------------------------------------------------------
void TaxiTracker::AddNode(SPosition &P)
{	TaxNODE *node = new TaxNODE(Seq,P);
	nodQ[Seq]	= node;
	Seq++;
	selN			= node;
	return;
}
//-------------------------------------------------------------------------------------------
//	Delete selected node
//-------------------------------------------------------------------------------------------
void TaxiTracker::DelNode()
{	if (0 == selN)			return;
	DelEdge(selN->idn);
	rn = nodQ.find(selN->idn);
	delete selN;
	nodQ.erase(rn);
	selN = 0;
	return;
}
//-------------------------------------------------------------------------------------------
//	Delete any edge with reference to this node
//-------------------------------------------------------------------------------------------
void TaxiTracker::DelEdge(Tag tn)
{	re = edgQ.begin();
	while (re != edgQ.end())
	{	TaxEDGE *edg = (*re).second;
		if (!edg->ReferTo(tn))	re++;
		else 	{	delete edg;		edgQ.erase(re++);}
	}
	return;
}
//-------------------------------------------------------------------------------------------
//	Add a edge between A and B
//-------------------------------------------------------------------------------------------
void TaxiTracker::AddEdge(Tag A, Tag B)
{	//--- Check for existence of AB --------
	Tag t1 = (A << 16) | (B & 0x0000FFFF);
	re = edgQ.find(t1);
	if (re != edgQ.end())		return;
	//--- Check for existence of BA --------
	Tag t2 = (B << 16) | (A & 0x0000FFFF);
	if (t1 == t2)						return;
	re = edgQ.find(t2);
	if (re != edgQ.end())		return;
	//--- Create an AB edge ----------------
	TaxEDGE *edg = new TaxEDGE();
	edg->idn	= t1;
	edgQ[t1]	= edg;
	//--- Reselect A node -----------------
	selN	= nodQ[A];
}
//-------------------------------------------------------------------------------------------
//	Selected node receive a new position
//-------------------------------------------------------------------------------------------
bool TaxiTracker::MoveNode(SPosition &P)
{	if (selN)			selN->pos	= P;
	return true;
}
//-------------------------------------------------------------------------------------------
//	Draw all node in picking mode
//-------------------------------------------------------------------------------------------
void TaxiTracker::DrawMarks()
{	for (rn = nodQ.begin(); rn != nodQ.end(); rn++)
		{	TaxNODE *node = (*rn).second;
			(node == selN)?glColor4f(1,0,0,1):glColor4f(0,1,0,1);	
			DrawNode(node);
		}
	return;
}
//-------------------------------------------------------------------------------------------
//	Draw a line between 2 positions
//-------------------------------------------------------------------------------------------
void TaxiTracker::DrawLine(SPosition &P, SPosition &Q)
{	SPosition R = globals->geop;
	double  rdf = globals->rdf;
	CVector A		= FeetComponents(R,P,rdf);
	CVector B   = FeetComponents(R,Q,rdf);
	//--- Draw a line ----------------------------
	glColor4f(1,1,1,1);
	glBegin(GL_LINES);
	glVertex3d(A.x,A.y,A.z+1);
	glVertex3d(B.x,B.y,B.z+1);
	glEnd();
}
//-------------------------------------------------------------------------------------------
//	Draw the floating link
//-------------------------------------------------------------------------------------------
void TaxiTracker::DrawLink()
{	if (0 == selN)		return;
	if (0 == lpos)		return;
	DrawLine(selN->pos,*lpos);
}
//-------------------------------------------------------------------------------------------
//	Draw one node in picking mode
//-------------------------------------------------------------------------------------------
void TaxiTracker::DrawNode(TaxNODE *N)
{	SVector T;
  globals->tcm->RelativeFeetTo(N->pos,T);
	glLoadName(N->idn);
	glPushMatrix();
	glTranslated(T.x, T.y, T.z);
	gluSphere(disk,10,12,12);	
	glPopMatrix();
}
//-------------------------------------------------------------------------------------------
//	Draw one edge
//-------------------------------------------------------------------------------------------
void TaxiTracker::DrawEdge(TaxEDGE *edg)
{	//--- Extract A node -------------------------------
	Tag ta	= edg->idn >> 16;									// A node
	rn			= nodQ.find(ta);
	if (rn == nodQ.end())		return;
	TaxNODE *na = (*rn).second;
	//--- Extract B node ------------------------------
	Tag tb	= edg->idn & 0x0000FFFF;
	rn			= nodQ.find(tb);
	if (rn == nodQ.end())		return;
	TaxNODE *nb = (*rn).second;
	DrawLine(na->pos,nb->pos);
}
//-------------------------------------------------------------------------------------------
//	Normal draw:
//-------------------------------------------------------------------------------------------
void TaxiTracker::Draw()
{ glPushAttrib(GL_ALL_ATTRIB_BITS);
	DrawMarks();
	DrawLink();
	for (re = edgQ.begin(); re != edgQ.end(); re++)	DrawEdge((*re).second);
	glPopAttrib();
}
//-------------------------------------------------------------------------------------------
//	One node is selected
//-------------------------------------------------------------------------------------------
void TaxiTracker::SetSelection(Tag No)
{	rn = nodQ.find(No);
	if (rn == nodQ.end())	selN = 0;
	else									selN	= (*rn).second;
	return;
}
//-------------------------------------------------------------------------------------------
//	Return selection ident
//-------------------------------------------------------------------------------------------
Tag TaxiTracker::GetSelected()
{	if (0 == selN)			return 0;
	return selN->idn;
}
//===========================================================================================
//	Taxiway editor
//===========================================================================================
CFuiTaxi::CFuiTaxi(Tag idn, const char *filename)
	:CFuiWindow(idn,filename,300,300,0)
{	//--- Initial modes -------------------------------------
	mode	= MODE_PICKING;
	//---BuildComponents ------------------------------------
	CFuiButton	*butn = 0;
	CFuiGroupBox *box  = 0;
	CFuiCheckBox *chk  = 0;
	U_INT wit = MakeRGBA(255,255,255,255);	// White color
	//--- Label group ----------------------------------------
	box  = new CFuiGroupBox (  4 , 8, 290, 50, this);
  AddChild('gbox',box,"");
	//--- Geo position  --------------------------------------
  gplb  = new CFuiLabel   (  4,  4, 270, 20, this);
  box->AddChild('gplb',gplb,"Geo   :",wit);
	//--- Cursor position ------------------------------------
  cplb  = new CFuiLabel   (  4, 24, 270, 20, this);
  box->AddChild('cplb',cplb,"Cursor:",wit);
	//--------------------------------------------------------
	//--------------------------------------------------------
	butn		= new CFuiButton( 10, 124, 60, 20, this);
	AddChild('cent',butn,"Center");
	//--------------------------------------------------------
	SetTransparentMode();
	//--- Set Camera type and parameters ---------------------
	ctx.prof	= PROF_TAXI;
	ctx.mode	= 0;
  rcam			= globals->ccm->SetRabbitCamera(ctx,RABBIT_CONTROL);
	rcam->Prof.Rep(CAM_MAY_MOVE);			// Restrict camera movement
	
	Validate();
	ReadFinished();
	//---------------------------------------------------------
	rcam->SetTracker(&trak, this);
	globals->fui->CaptureMouse(this);
	//--------------------------------------------------------------------
	globals->Disp.ExecON  (PRIO_ABSOLUTE);	// Allow terrain TimeSlice
	globals->Disp.ExecOFF (PRIO_TERRAIN);		// Nothing exe after terrain
	globals->Disp.DrawON  (PRIO_ABSOLUTE);	// Drawing
	globals->Disp.ExecLOK (PRIO_WEATHER);		// No weather
	//---- Marker 1 -----------------------------------
	char *ds = new char[32];
	strcpy(ds,"*START TaxiEDIT*");

}
//------------------------------------------------------------------------------
//	Destroy and free resources
//------------------------------------------------------------------------------
CFuiTaxi::~CFuiTaxi()
{	//--- Restore State ---------------------------------
	globals->Disp.DrawON (PRIO_TERRAIN);		// Allow Terrain TimeSlice
	globals->Disp.ExecON (PRIO_TERRAIN);		// Allow Terrain Draww
	globals->Disp.ExecULK(PRIO_WEATHER);
	//---- Marker 2 -------------------------------------
	char *ds = Dupplicate("*END taxiEDIT*",32);
	globals->ccm->RestoreCamera(ctx);
}
//------------------------------------------------------------------------------
//	Validation:  We must have a nearest airport
//------------------------------------------------------------------------------
void	CFuiTaxi::Validate()
{	apo	= globals->apm->GetNearestAPT();
	if (0 == apo)	Close();
	//--- Go to airport center --------------------------
	apt	= apo->GetAirport();
	org	= apo->GetOrigin();
	NewPosition(org);											// Start at airport center
	rcam->SetAbove(globals->geop);
	//--- Edit title ------------------------------------
	char edt[128];
	_snprintf(edt,128,"TAXIWAY EDITOR for %s", apt->GetName());
	SetText(edt);
	return;
}
//------------------------------------------------------------------------------
//	Set a new position
//------------------------------------------------------------------------------
void CFuiTaxi::NewPosition(SPosition P)
{	//--- Get terrain altitude -------------------------
	GroundSpot lnd(P.lon,P.lat);
  P.alt = globals->tcm->GetGroundAt(lnd);
	globals->geop = P;
	geop	= P;
	PositionEdit(P,gplb,"GEOP:");
}
//------------------------------------------------------------------------------
//	Edit geo position
//------------------------------------------------------------------------------
void CFuiTaxi::PositionEdit(SPosition P, CFuiLabel *lab,char *T)
{	char tlat[128];
	char tlon[128];
	char edt[128];
	EditLat2DMS(P.lat,tlat);
  EditLon2DMS(P.lon,tlon);
	sprintf(edt,"%s %-20s %-20s",T,tlon,tlat);
	lab->SetText(edt);
}
//------------------------------------------------------------------------------
//	Compute geo position under cursor
//	We set the origin of pixel at glut window center
//	NOTE: mx and my are normally relative to the glut window, 
//				not to the full screen
//------------------------------------------------------------------------------
void	CFuiTaxi::CursorGeop(int mx, int my)
{	int xorg = globals->cScreen->Width  >> 1;					// pixel origins in screen
	int yorg = globals->cScreen->Height >> 1;
	//------ Compute pixel deltas --------------------------
	int dx	= +(mx - xorg);
	int dy  = -(my - yorg);
	double fpp = rcam->GetFPIX();
	CVector dta((fpp * dx),(fpp * dy),0);
	cpos	= AddToPositionInFeet(geop,dta);	//,globals->exf);
	return;
}
//------------------------------------------------------------------------------
//	A node is selected
//------------------------------------------------------------------------------
void CFuiTaxi::OnePicking(U_INT No)
{	selN	= No; }
//------------------------------------------------------------------------------
//	Center world on spot
//------------------------------------------------------------------------------
bool CFuiTaxi::CenterOnSpot(int mx, int my)
{	CursorGeop(mx,my);
	NewPosition(cpos);
	return true;
}
//------------------------------------------------------------------------------
//	Enter link mode
//------------------------------------------------------------------------------
void CFuiTaxi::LinkMode()
{	edgA	= trak.GetSelected();
	if (0 == edgA)	return;
	mode = MODE_LINKING;
	trak.SetLink(&cpos);
	return;
}
//------------------------------------------------------------------------------
//	Enter Pick mode with a selected node (or 0)
//------------------------------------------------------------------------------
bool CFuiTaxi::PickMode(U_INT No)
{	selN	= No;
	mode	= MODE_PICKING;
	trak.SetLink(0);
	return true;
}

//------------------------------------------------------------------------------
//	Moving a node
//------------------------------------------------------------------------------
bool CFuiTaxi::MovingNode(int mx, int my)
{	if (0 == selN)									return false;
	int mod = glutGetModifiers();
	if (GLUT_ACTIVE_CTRL != mod)		mmov = false;
	//--- Move the node ---------------------------------
	if (mmov)	trak.MoveNode(cpos);
	return true;
}
//------------------------------------------------------------------------------
//	Mouse move
//	NOTE: When a node is selected and the CTRL button is held, then
//				we move the node
//------------------------------------------------------------------------------
bool CFuiTaxi::InsideMove(int mx,int my)
{	if (MouseHit(mx,my))						return true;
	//--- Change position -------------------------------
  CursorGeop(mx,my);
	PositionEdit(cpos,cplb,"CURS:");
	//--- Check for moving selected node ----------------
	switch (mode)	{
			case MODE_PICKING:
				return MovingNode(mx, my);
			case MODE_LINKING:
				trak.SetLink(&cpos);
				return false;
	}
	return false;
}
//------------------------------------------------------------------------------
//	Mouse Picking
//------------------------------------------------------------------------------
bool CFuiTaxi::MousePicking(int mx, int my)
{	sx			= mx;
	sy			= my;
	selN		= 0;
	RegisterFocus(0);										// Free textedit
	bool		hit = rcam->PickObject(mx, my);
	trak.SetSelection(selN);
	return true;		
}
//------------------------------------------------------------------------------
//	Mouse Linking
//------------------------------------------------------------------------------
bool CFuiTaxi::MouseLinking(int mx, int my)
{	MousePicking(mx,my);
	if (0 == selN)			return PickMode(edgA);
	trak.AddEdge(edgA,selN);
	selN		= trak.GetSelected();
	return true;
}
//------------------------------------------------------------------------------
//	Mouse click are captured here
//------------------------------------------------------------------------------
bool CFuiTaxi::MouseCapture(int mx, int my, EMouseButton bt)
{	if (MouseClick (mx, my, bt))		return true;
  if (MOUSE_BUTTON_RIGHT == bt)		return OpenMenu(mx,my);
	//--- Check for centering on the clicked spot ------------
	int mod = glutGetModifiers();
	if (GLUT_ACTIVE_ALT == mod)			return CenterOnSpot(mx,my);
	mmov		= (GLUT_ACTIVE_CTRL == mod);
	switch (mode) {
			case MODE_PICKING:
				return MousePicking(mx,my);
			case MODE_LINKING:
				return MouseLinking(mx,my);
	}
	return true;		
}
//------------------------------------------------------------------------------
//	Stop any move here
//------------------------------------------------------------------------------
bool	CFuiTaxi::StopClickInside(int mx, int my, EMouseButton but)
{	mmov = 0;
	return true;
}
//------------------------------------------------------------------------------
//	Time slice: nothing to do
//------------------------------------------------------------------------------
void	CFuiTaxi::TimeSlice()
{	

}
//------------------------------------------------------------------------------
//	Open the floating menu Menu
//------------------------------------------------------------------------------
bool CFuiTaxi::OpenMenu(int mx,int my)
{	menu.Ident = 'edmn';
	menu.aText = taxiMENU;
	//--- Open a  pop menu ---------------
	OpenPopup(mx,my,&menu);
	PickMode(0);
	return true;
}
//------------------------------------------------------------------------------
//	Click on edit Menu
//------------------------------------------------------------------------------
int CFuiTaxi::ClickEditMENU(short itm)
{ char  opt = *menu.aText[itm];
  switch (opt) {
			//--- Insert a node -----------------
			case 'I':
				trak.AddNode(cpos);
				break;
			//--- Delete a node -----------------
			case 'D':
				trak.DelNode();
				break;
			//---  Link to other ----------------
			case 'L':
				LinkMode();
				break;
	}
  ClosePopup();
	return 1;
}
//------------------------------------------------------------------------------
//	NOTIFICATION from buttons
//------------------------------------------------------------------------------
void CFuiTaxi::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{	switch (idm)  {
    case 'sysb':
      SystemHandler(evn);
      return;
		//--- center drawing uon airport -----------
		case 'cent':
			NewPosition(org);
			rcam->SetAbove(org);
			return;

	}
return;
}
//------------------------------------------------------------------------------
//	NOTIFICATIONS from popup menu
//------------------------------------------------------------------------------
void CFuiTaxi::NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn)
{	if (EVENT_POP_CLICK  != evn)			return;
	switch(id)	{
			//--- click on map ------------------
			case 'edmn':
				ClickEditMENU(itm);
				return;
	}
	return;
}
//=======================EDN OF FILE ========================================================


