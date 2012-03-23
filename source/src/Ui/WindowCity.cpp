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
//	Primary data used for building are comming from http://www.openstreetmap.org/ with
//	copyright "© les contributeurs d’OpenStreetMap, CC BY-SA "
//
//	"osm2xp par Benjamin Blanchet" pour la generation FlyLegacy
//
//==========================================================================================
#include "../include/WinCity.h"
#include "../Include/Triangulator.h"
#include "../Include/RoofModels.h"
#include "../Include/Model3D.h"
#include "../Include/TerrainTexture.h"
#include "../Include/FuiOption.h"
#include <vector>
char *EndOSM = "***";
//==========================================================================================
//  List of AMENITY VALUES Tags
//==========================================================================================
OSM_VALUE amenityVAL[] = {
	{"PLACE_OF_WORSHIP",	OSM_CHURCH},
	{"POLICE",						OSM_POLICE},
	{"FIRE_STATION",			OSM_FIRE},
	{"TOWNHALL",					OSM_TOWNHALL},
	{"SCHOOL",						OSM_SCHOOL},
	{"COLLEGE",						OSM_COLLEGE},
	{"HOSPITAL",					OSM_HOSPITAL},
	{EndOSM,					0},									  // End of table
};
//==========================================================================================
//  List of BUILDING VALUES Tags
//==========================================================================================
OSM_VALUE  buildingVAL[] = {
		{"SCHOOL",						OSM_SCHOOL},
		{EndOSM,					0},									// End of table
};
//==========================================================================================
//  List of admitted Tags
//==========================================================================================
OSM_TAG TagLIST[] = {
	{"AMENITY",			amenityVAL},
	{"BUILDING",		buildingVAL},
	{EndOSM,					0},									// End of table
};
//==========================================================================================
//  Window to display building sketch
//==========================================================================================
CFuiSketch::CFuiSketch(Tag idn, const char *filename)
:CFuiWindow(idn,filename,220,470,0)
{ title = 1;
  close = 1;
  zoom  = 0;
  mini  = 0;
	//--- white color -------------------------------
	U_INT wit = MakeRGBA(255,255,255,255);
	//--- Set title ---------------------------------
	strcpy(text,"CITY EDITOR");
	gBOX  = new CFuiGroupBox(10,8,200,50,this);
  AddChild('gbox',gBOX,"");
	//--- Label --------------------------------------
  aLAB  = new CFuiLabel   ( 4, 4,56, 20, this);
  gBOX->AddChild('labl',aLAB,"Options:",wit);
	//---Fill option ---------------------------------
  fOPT  = new CFuiCheckBox(60, 4,160,20,this);
  gBOX->AddChild('fopt',fOPT,"Fill mode",wit);
	//--- Other 3D objects ---------------------------
  vOPT  = new CFuiCheckBox(60,24,160,20,this);
  gBOX->AddChild('vopt',vOPT,"View scenery objects",wit);
	vOPT->SetState(1);

	//--- Second group box ------------------------
	Box2 = new CFuiGroupBox(10,66,200,100,this);
	AddChild('box2',Box2,"");
	//--- Terrain button---------------------------
	vTer	= "Hide Terrain";
	vTER = new CFuiButton( 106, 46,  90, 20,this);
	Box2->AddChild('vter', vTER,"View Terrain");
	//--- NEXT button---------------------------
	nBUT	= new CFuiButton(  4, 72,  90, 20,this);
	Box2->AddChild('nbut', nBUT,"Next");
	//--- View All button --------------------------
	vALL	= new CFuiButton(106, 72,  90, 20,this);
	Box2->AddChild('vall', vALL,"Load all");
	//-----------------------------------------------
	nBAT	= new CFuiLabel ( 4,   4, 190, 20, this);
	Box2->AddChild('nbat', nBAT, "Building",wit);
	//-----------------------------------------------
	nTAG  = new CFuiLabel ( 4,  30, 190, 20, this);
	Box2->AddChild('ntag', nTAG, "Tag",wit);

	//----Create the style List box -----------------
  sWIN  = new CFuiList  (10,174, 200,140,this);
  sWIN->SetVScroll();
  AddChild('styl',sWIN,"Styles",FUI_TRANSPARENT,wit);

	//--- Create a group box 3 -------------------
	Box3	= new CFuiGroupBox(10, 330, 200, 56, this);
	AddChild('box3',Box3,"", FUI_TRANSPARENT,wit);
	//--- Change style ---------------------------
	nSTY	= new CFuiButton(  4, 4, 90, 20,this);
	Box3->AddChild('nsty', nSTY,"Change Style");
	//--- Delete object ---------------------------
	dOBJ	= new CFuiButton(106, 4, 90, 20,this);
	Box3->AddChild('delt', dOBJ,"Remove building");
	//--- Restore object ---------------------------
	gOBJ	= new CFuiButton(106, 30, 90, 20,this);
	Box3->AddChild('gobj', gOBJ,"Restore building");

	//--- Create a group box 4 -------------------
	Box4	= new CFuiGroupBox(10, 400, 200, 36, this);
	AddChild('box4',Box4,"", FUI_TRANSPARENT,wit);
	//----Create Replace button ---------------------	
	rOBJ	= new CFuiButton(4,  4,  90, 20,this);
	Box4->AddChild('robj', rOBJ,"Replace building");
	//----Create Rotation button --------------------	
	lROT	= new CFuiButton(106,4,  42, 20,this);
	Box4->AddChild('lrot', lROT,"left");
	lROT->SetRepeat(0.1F);
	//----Create Rotation button --------------------	
	rROT	= new CFuiButton(152,4,  42, 20,this);
	Box4->AddChild('rrot', rROT,"right");
	rROT->SetRepeat(0.1F);
	//-----------------------------------------------
	bTRY  = new CFuiButton(10, 446, 200, 20, this);
	AddChild('btry',bTRY,"FLY IT",0);

	//-----------------------------------------------	
	ReadFinished();
	//--- Open triangulation ------------------------
	trn	= new Triangulator(&ses);
	globals->trn = trn;
	//-----------------------------------------------
	ctx.prof	= PROF_SKETCH;
	ctx.mode	= 0;
  rcam			= globals->ccm->SetRabbitCamera(ctx,RABBIT_S_AND_C);
	rcam->SetTracker(trn, this);
	//--- Set transparent mode ----------------------
	SetTransparentMode();
	//-----------------------------------------------
	oneD = DegToRad(double(1));
	//------------------------------------------------
	globals->cam->SetRange(100);
	SetOptions(TRITOR_ALL);
	ses.SetTrace(1);
	modif = 0;
	tera	= 0;
	scny	= 1;
	wait  = 1;
	edit	= 0;
	wfil	= 1;
	FP		= 0;
	bpm		= 0;
	//--- delete aircraft for more memory ------------
	//SAFE_DELETE( globals->pln);
	//--- Set Initial state --------------------------
	globals->Disp.ExecOFF (PRIO_ABSOLUTE);	// Only TRN
	globals->Disp.DrawON  (PRIO_ABSOLUTE);	// Drawing
	globals->Disp.DrawOFF (PRIO_TERRAIN);		// No Terrain
	globals->Disp.ExecLOK (PRIO_WEATHER);		// No weather
	//-------------------------------------------------
	globals->fui->SetBigFont();
	DrawNoticeToUser("INITIALIZATION",1);
	//---- Marker 1 -------------------------------------
	char *ds = new char[32];
	strcpy(ds,"*START CityEDIT*");
	//--------------------------------------------------
	FPM.close = 0;
	FPM.sbdir = 1;
	FPM.userp = 0;
	FPM.text  = "SELECT A FILE";
	FPM.dir		= "OpenStreet";
	FPM.pat		= "*.ofe";
	State = SKETCH_FILE;
	STREETLOG("START CITY EDITOR");
};
//-----------------------------------------------------------------------
//	destroy this
//-----------------------------------------------------------------------
CFuiSketch::~CFuiSketch()
{	//--- Close File ------------------------------------
	if (FP)	fclose(FP);
	//--- Restore State ---------------------------------
	globals->Disp.DrawON (PRIO_TERRAIN);		// No Terrain
	globals->Disp.ExecON (PRIO_TERRAIN);		// No Terrain
	globals->Disp.ExecULK(PRIO_WEATHER);
	//--- Back to position ------------------------------
	SAFE_DELETE(globals->trn);

//	globals->stop = 1;
	//---- Marker 2 -------------------------------------
	char *ds = new char[32];
	strcpy(ds,"*END CityEDIT*");
	STREETLOG("END CITY EDITOR");
	globals->ccm->RestoreCamera(ctx);

}
//----------------------------------------------------------------------
//	Override check profile
//----------------------------------------------------------------------
bool CFuiSketch::CheckProfile(char a)
{	return true;	}

//-----------------------------------------------------------------------
//	A file is selected 
//-----------------------------------------------------------------------
void CFuiSketch::FileSelected(FILE_SEARCH *fpm)
{ switch (fpm->userp)	{
		//--- Openstreet file selected ---------
		case 0:
			State = SKETCH_WSEL;
			strncpy(spath,fpm->sdir,FNAM_MAX);
			strncpy(sname,fpm->sfil,FNAM_MAX);
			_snprintf(fnam,FNAM_MAX,"%s/%s",spath,sname);
			break;
		case 1:
			strncpy(smodl,fpm->sfil,FNAM_MAX);
			State = SKETCH_ROBJ;
			break;
	}

	return;
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
	return;
}
//-------------------------------------------------------------------
//	Parse a new building 
//-------------------------------------------------------------------
bool CFuiSketch::ParseBuilding()
{	char txt[128];
  fsetpos( FP, &fpos); 
	char *ch = ReadTheFile(FP,txt);
	_strupr(txt);
	skip	= 0;
	otype = OSM_BUILDING;
	*tagn = *valn = *smodl = 0;
	//--- Check for a building number -------------------------
	int nf = sscanf(ch,"START %d ID=%d ",&seqNo, &ident);
	if (nf != 2)		return false;
	trn->SetIdent(seqNo,ident); 
	return ParseFile();
}
//-------------------------------------------------------------------
//	Parse vertice list 
//-------------------------------------------------------------------
bool CFuiSketch::ParseFile()
{	char txt[256];
	bool go = true;
	trn->StartOBJ();
	while (go)
	{	fgetpos (FP,&fpos);
		char *ch = ReadTheFile(FP,txt);
		if (ParseStyle(txt))							continue;
		if (ParseReplace(txt))						continue;
		//--- continue with upper cases ------------
		_strupr(ch);
		if (ParseTAG(txt))								continue;				// Tag directive
		if (ParseVTX(txt))								continue;				// Vertices
		if (ParseHOL(txt))								continue;				// Hole directive
		//--- End of vertices --------------------------
		return true;
	}
	return false;
}
//-------------------------------------------------------------------
//	Parse Style
//-------------------------------------------------------------------
bool CFuiSketch::ParseStyle(char *txt)
{ char      nsty[64];
	U_INT rofm = 0;
	U_INT rftx = 0;
	int nf = sscanf_s(txt," Style %63s rofm = %d rftx = %d",nsty,63, &rofm, &rftx);
	
	if (nf == 3)  trn->ForceStyle(nsty,rofm,rftx);
	return (nf == 3);
}
//-------------------------------------------------------------------
//	Parse vertex list
//-------------------------------------------------------------------
bool CFuiSketch::ParseVTX(char *txt)
{	int nv = 0;
  int rd = 0;
	double x;
	double y;
	bool go		= true;
	char *src = txt;
	while (go)
	{	int nf = sscanf(src," V ( %lf , %lf ) %n",&y,&x,&rd);
		if (nf != 2)	return (nv != 0);
		if (skip == 0) trn->AddVertex(x,y);
		src += rd;
		nv++;
	}
	return true;
}
//-------------------------------------------------------------------
//	Parse Hole directive
//-------------------------------------------------------------------
bool CFuiSketch::ParseHOL(char *txt)
{	if (strncmp(txt,"HOLE",4) != 0)			return false;
	if (skip == 0)	trn->NewHole();
	return true;
}
//-------------------------------------------------------------------
//	Check for legible value
//-------------------------------------------------------------------
void CFuiSketch::CheckValue(OSM_VALUE *tab)
{	while (strcmp(tab->value,EndOSM) != 0)
	{ otype	= tab->type;
		if  (strcmp(tab->value,valn) == 0)	return;
		tab++;
	}
  //--- unsupported object --------------------
	skip	= 1;
}
//-------------------------------------------------------------------
//	Check for legible tag
//-------------------------------------------------------------------
void CFuiSketch::CheckTag()
{	OSM_TAG *tab = TagLIST;
	while (strcmp(tab->tag,EndOSM) != 0)
	{	if  (strcmp(tab->tag,tagn)   == 0)	return CheckValue(tab->table);
		tab++;
	}
	//--- tag not supported --------------------
	skip	= 1;
}
//-------------------------------------------------------------------
//	Parse Tag directive
//-------------------------------------------------------------------
bool CFuiSketch::ParseTAG(char *txt)
{	int nf = sscanf(txt,"TAG ( %32[^ =)] = %32[^ )] ) ",tagn, valn);
	if (nf != 2)		return false;
	CheckTag();
	return true;
}
//-------------------------------------------------------------------
//	Parse replace directive
//-------------------------------------------------------------------
bool CFuiSketch::ParseReplace(char *txt)
{ int nf = sscanf(txt,"Replace ( Z = %lf ) with %s",&orien,smodl);
	return (nf == 2);
}
//-------------------------------------------------------------------
//	Abort
//-------------------------------------------------------------------
U_INT CFuiSketch::Abort(char *erm, char *fn)
{	char txt[128];
	_snprintf(txt,127,"%s %s", erm, fn);
	STREETLOG("%s %s",erm,fn);
	CreateDialogBox("ERROR",txt);
	return SKETCH_ABORT;
}
//-------------------------------------------------------------------
//	Parse Area
//-------------------------------------------------------------------
bool CFuiSketch::ParseArea()
{	char txt[128];
	char *ed = " AREA SW [ %lf , %lf ] NE [ %lf , %lf ] ";
	char *ch = ReadTheFile(FP,txt);
	_strupr(ch);
	double y1;
	double x1;
	double y2;
	double x2;
	int nf = sscanf(ch, ed , &SW.lat, &SW.lon, &NE.lat , &NE.lon);
	if (nf != 4)	return false;
	//--- Compute mid position ----------------------------
	fgetpos( FP, &fpos);
	y1 = FN_ARCS_FROM_DEGRE(SW.lat);
	x1 = FN_ARCS_FROM_DEGRE(SW.lon);
	y2 = FN_ARCS_FROM_DEGRE(NE.lat);
	x2 = FN_ARCS_FROM_DEGRE(NE.lon);
	rpos.lon = AddLongitude(x1,x2) * 0.5;
	rpos.lat = (y1 + y2) * 0.5;
	return true;
}
//-------------------------------------------------------------------
//	Compute reference position
//-------------------------------------------------------------------
U_INT CFuiSketch::GotoReferencePosition()
{	char *er1 = "Cant open file ";
	char *er2 = "Invalid file ";
	rpos.lon	= 0;
	rpos.lat	= 0;
	rpos.alt	= 0;
	count			= 0;
	FP  = fopen(fnam,"rb");
  if (0 == FP)			return Abort(er1,fnam);
	fpos	= 0;
	if (!ParseArea())	return Abort(er2,fnam);
	//--- Go to reference position --------
	wfil	= 0;
	cntw	= 0;
	nBLDG	= 0;
	trn->SetReference(rpos);
	rcam->GoToPosition(rpos);			// Teleport
	globals->Disp.ExecON (PRIO_ABSOLUTE);		// Allow Terrain
	globals->Disp.ExecOFF(PRIO_TERRAIN);		// Stop after terrain
	nStat = SKETCH_OPEN;
	return SKETCH_WAIT;
}
//-------------------------------------------------------------------
//	Edit building
//-------------------------------------------------------------------
void CFuiSketch::EditBuilding()
{	char txt[128];
	U_INT nb  = bpm->stamp;
	double lx = FN_METRE_FROM_FEET(bpm->lgx);
	double ly = FN_METRE_FROM_FEET(bpm->lgy);
	_snprintf(txt,127,"BUILDING %05d lg:%.1lf wd:%.1lf",nb,lx,ly);
	nBAT->SetText(txt);
	//--------------------------------------------
	trn->EditTag(txt);
	nTAG->SetText(txt);
	return;
}
//-------------------------------------------------------------------
//	Build Object
//-------------------------------------------------------------------
void CFuiSketch::BuildObject()
{	if (skip)	return;
	bpm = trn->BuildOBJ(otype);
	trn->SetTag(tagn,valn);
	if (*smodl)	trn->ReplaceBy(smodl,"OpenStreet/Models", orien);
	nBLDG++;
	EditBuilding();
	geop  = bpm->geop;
	rcam->GoToPosition(geop);			// Teleport
	if (bpm->error == 0)	rpos = geop;
	return ;
}
//-----------------------------------------------------------------------
//	View terrain action
//-----------------------------------------------------------------------
U_INT CFuiSketch::TerrainView()
{	trn->DrawGroups();
	rcam->GoToPosition(geop);			// Teleport
	globals->Disp.ExecON (PRIO_ABSOLUTE);		// Allow Terrain
	globals->Disp.ExecOFF(PRIO_TERRAIN);		// Stop after terrain
	globals->Disp.DrawON (PRIO_TERRAIN);		// Draw Terrain
	tera  = 1;
	globals->fui->CaptureMouse(this);
	vTER->SetText(vTer);
	nStat = SKETCH_PAUSE;
	return SKETCH_WAIT;
}
//-----------------------------------------------------------------------
//	Hide terrain action
//-----------------------------------------------------------------------
U_INT CFuiSketch::TerrainHide()
{	char ret = SKETCH_PAUSE;
	if (0 == bpm)					return ret;				// No selected building
	trn->DrawSingle();
	globals->Disp.ExecOFF (PRIO_ABSOLUTE);						// No Terrain
	globals->Disp.DrawOFF (PRIO_TERRAIN);							// No Terrain
	tera  = 0;
	vTER->SetText("View Terrain");										// View or Edit
	globals->fui->CaptureMouse(0);										// Stop Capture
	return ret;													// just pause till next event
}
//-----------------------------------------------------------------------
//	Wait terrain for ready to set the terrain altitude
//-----------------------------------------------------------------------
U_INT CFuiSketch::TerrainWait()
{	char txt[128];
	//--- warning in first step ---------------------------
	if (wait)
	{	_snprintf(txt,128,"TELEPORTING TO DESTINATION (%06u).  PLEASE WAIT",cntw++);
		DrawNoticeToUser(txt,1);
	}
	if (!globals->tcm->TerrainStable())		return SKETCH_WAIT;
	//--- get terrain elevation ------------------------
	wait	= 0;
	return nStat;
}
//-----------------------------------------------------------------------
//	Terrain at reference position
//-----------------------------------------------------------------------
U_INT CFuiSketch::HereWeAre()
{	rcam->SetAngle(30,15);
	globals->Disp.ExecOFF (PRIO_ABSOLUTE);		// No Terrain
	globals->Disp.DrawOFF (PRIO_TERRAIN);		  // No Terrain
	trn->DrawSingle();
	eofl	= 0;
	return SKETCH_NEXT;
}
//-----------------------------------------------------------------------
//	Get next building from OFE file
//-----------------------------------------------------------------------
U_INT	CFuiSketch::OneBuilding()
{	if (!ParseBuilding())	return SKETCH_ENDL;
	//--- build object -----------------
	BuildObject();
	//--- show style on list box -------
	U_INT ns = bpm->style->GetSlotSeq();
	sBOX.GoToItem(ns);
	return SKETCH_PAUSE;
}
//-----------------------------------------------------------------------
//	Start Load
//-----------------------------------------------------------------------
U_INT CFuiSketch::StartLoad()
{	if (eofl)			return SKETCH_PAUSE;
	if (tera)			TerrainHide();
	rcam->SetAngle(30,15);
	rcam->SetRange(150);
	return SKETCH_LOAD;
}
//-----------------------------------------------------------------------
//	Read next building
//-----------------------------------------------------------------------
U_INT CFuiSketch::ReadNext()
{ char txt[128];
	if (eofl)							return SKETCH_PAUSE;
	if (!ParseBuilding())	return SKETCH_ENDL;
	//--- build object ---------------------
	BuildObject();
	time	= 1;
	//---------------------------------------
	_snprintf(txt,127,"Load Building %05d",bpm->stamp);
	DrawNoticeToUser(txt,1);
	return SKETCH_SHOW;
}
//-----------------------------------------------------------------------
//	Show building before next
//-----------------------------------------------------------------------
U_INT	CFuiSketch::ShowBuilding()
{	time--;
	if (time >= 0)	return SKETCH_SHOW;
	return SKETCH_LOAD;
}
//-----------------------------------------------------------------------
//	End of file is reached
//	Enter in edit mode
//-----------------------------------------------------------------------
U_INT CFuiSketch::EndLoad()
{	char txt[128];
	_snprintf(txt,127,"%05d buildings",nBLDG);
	if (0 == nBLDG)	return Abort("Invalid file ",fnam);
	nBAT->SetText(txt);
	trn->EndOBJ();
	fclose(FP);
	FP		= 0;
	eofl	= 1;
	edit  = 1;
	TerrainView();
	rcam->GoToPosition(rpos);
	rcam->SetRange(500);
	globals->fui->CaptureMouse(this);
	vTer	= "Zoom Building";
	vTER->SetText(vTer);
	bpm		= 0;
	//--- change ident for 'load all' button -----------------------------
	vALL->SetId('null');					// Becomes a null button
	vALL->SetText("End of File");
	//--- inhibit next button --------------------------------------
	nBUT->SetId('null');
	return SKETCH_PAUSE;
}
//-----------------------------------------------------------------------
//	Edit error code
//-----------------------------------------------------------------------
bool CFuiSketch::EditError()
{ char txt[128];
	if (0 == bpm->error)		return false;
	_snprintf(txt,128,"Build %d Error %d",bpm->stamp,bpm->error);
	nBAT->SetText(txt);
	bpm->error = 0;
	return true;
}
//-----------------------------------------------------------------------
//	Change the current style
//-----------------------------------------------------------------------
void CFuiSketch::ChangeStyle()
{	if (0 == bpm)										return;
	if ((tera) && (bpm->selc == 0))	return;
  EditError();
	D2_Style *sty = (D2_Style*)sBOX.GetSelectedSlot();
	D2_Group *grp = sty->GetGroup();
	grp->GetOneRoof(*bpm);
	sty->AssignBPM(bpm);
	bpm->style		= sty;
	bpm->group    = sty->GetGroup();
	bpm->mans			= sty->IsMansart();
	bpm->flNbr		= grp->GetFloorNbr();
	bpm->flHtr		= grp->GetFloorHtr();
	bpm->roofP		= ses.GetRoofTexture(sty);
	bpm->roofM		= grp->GetRoofModByNumber(*bpm);
	modif	= trn->ModifyStyle();
	EditError();
	return;
}
//-----------------------------------------------------------------------
//	Remove current building
//	NOTE:  We must be in edit mode
//-----------------------------------------------------------------------
void	CFuiSketch::RemoveBuilding()
{	if (NoSelection())	return;
	int rm = trn->RemoveBuilding();
	modif	|= rm;
	nBLDG -= rm;
	if (rm)	bpm = 0;
	return;
}
//-----------------------------------------------------------------------
//	Replace current building
//	NOTE:  We must be in edit mode
//-----------------------------------------------------------------------
void CFuiSketch::ReplaceBuilding()
{	if (NoSelection())	return;
  double rad = atan2(bpm->sinA,bpm->cosA);
	modif = trn->ReplaceBy(smodl,"OpenStreet/Models", rad);
	State = SKETCH_PAUSE;
	return;
}
//-----------------------------------------------------------------------
//	Restore last deleted building
//	NOTE:  We must be in edit mode
//-----------------------------------------------------------------------
void	CFuiSketch::ResetBuilding()
{	bpm = trn->RestoreBuilding(&nBLDG);
	return;
}
//-----------------------------------------------------------------------
//	Rotate the building
//-----------------------------------------------------------------------
void CFuiSketch::RotateBuilding(double rad)
{	if (NoSelection())	return;
	modif	= trn->RotateObject(rad);
	return;
}
//-----------------------------------------------------------------------
//  Click detected outside any window
//	-Start picking mode for the camera
//	return true only if a hit was detected. If not
//	return false so that the mouse event can be passed on to the next
//	object in scene
//-----------------------------------------------------------------------
bool CFuiSketch::MouseCapture(int mx, int my, EMouseButton bt)
{	RegisterFocus(0);										// Free textedit
	bool		hit = rcam->PickObject(mx, my);						
	return	hit;		
}
//-----------------------------------------------------------------------
//	One object selected
//-----------------------------------------------------------------------
void CFuiSketch::OnePicking(U_INT No)
{	bpm = trn->SelectBuilding(No);
  if (0 == bpm)		return;
	EditBuilding();
	geop	= trn->GetPosition();
	U_INT ns = bpm->style->GetSlotSeq();
	sBOX.GoToItem(ns);
	return;
}
//-----------------------------------------------------------------------
//	Check if selected
//-----------------------------------------------------------------------
bool CFuiSketch::NoSelection()
{	if (0 == bpm)													return true;
	if ((0 == bpm->selc) && (1 == tera))	return true;
	return false;
}
//-----------------------------------------------------------------------
//	Fly Over the city
//-----------------------------------------------------------------------
void CFuiSketch::FlyOver()
 { Write(); 
	 globals->Disp.DrawOFF (PRIO_TERRAIN);		// No Terrain
	  ses.UpdateCache();
		Close();
		return;
}
//-----------------------------------------------------------------------
//	Time slice
//-----------------------------------------------------------------------
void CFuiSketch::TimeSlice()
{	char *erm = "No file to edit";
	switch (State)	{
		//--- Create file selection ------------
		case SKETCH_FILE:
				CreateFileBox(&FPM);
				return;
		//--- Wait file selection --------------
		case SKETCH_WSEL:
			  if (0 == *FPM.sfil)	State = Abort(erm," Editor will close");
			  else								State = BuildStyList();
				return;
		//--- Create reference position --------
		case SKETCH_REFER:
				State = GotoReferencePosition();
				return;
		//--- Open again the file ---------------
		case SKETCH_OPEN:
				State = HereWeAre();
				return;
		//--- Read next building ----------------
		case SKETCH_NEXT:
				State = OneBuilding();
				return;
		//--- Pausing --------------------------
		case SKETCH_PAUSE:
				return;
		//--- Waiting on terrain ---------------
		case SKETCH_WAIT:
				State = TerrainWait();
				return;
		//---LOAD State ------------------------
		case SKETCH_LOAD:
				State = ReadNext();
				return;
		//--- SHOW state -----------------------
		case SKETCH_SHOW:
				State = ShowBuilding();
				return;
		//--- Replace model --------------------
		case SKETCH_ROBJ:
				ReplaceBuilding();
				return;
		//--- END of file ----------------------
		case SKETCH_ENDL:
				State = EndLoad();
				return;
		//--- END of seession ----------------------
		case SKETCH_ABORT:
				Close();
				return;

	}
	return;
}
//-----------------------------------------------------------------------
//	Collect all models
//-----------------------------------------------------------------------
void CFuiSketch::CollectModels()
{	if (NoSelection())	return;
	FPM.userp	= 1;
	FPM.close = 1;
	FPM.sbdir = 0;
	FPM.text  = "Select a Model";
	FPM.dir   = "OpenStreet/Models";
	FPM.pat		= "*.OBJ";
	CreateFileBox(&FPM);
	return;
}
//-----------------------------------------------------------------------
//	No File session
//-----------------------------------------------------------------------
int CFuiSketch::NoSession()
{	char txt[256];
	_snprintf(txt,255,"No Session parameter file in %s", spath);
	STREETLOG("%s",txt);
  CreateDialogBox("SESSION",txt);
	return SKETCH_ABORT;
}
//-----------------------------------------------------------------------
//	Collect all Style
//-----------------------------------------------------------------------
int CFuiSketch::BuildStyList()
{	sBOX.SetParameters(this,'styl',(LIST_DONT_FREE+LIST_HAS_TITLE));
  styTT.SetSlotName("Style collection");
	//--- Look for session parameters --------
	bool ok = ses.ReadParameters(spath);
	if (!ok)		return NoSession();
	//----Warning ---------------------------
	ses.SetTRN(trn);
	sBOX.AddSlot(&styTT);
	ses.FillStyles(&sBOX);
	sBOX.Display();
	return SKETCH_REFER;
}
//-----------------------------------------------------------------------
//	Swap scenery mode
//-----------------------------------------------------------------------
void CFuiSketch::Swap3D()
{	scny ^= 1;
	int md = (scny)?(-1):(+1);
	globals->noOBJ += md;
	globals->noAPT += md;
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
//  Write the file
//---------------------------------------------------------------------
void CFuiSketch::Write()
{	char txt[128];
	char *ed = "Area SW[%.2lf, %.2lf] NE[%.2lf, %.2lf] \n";
	if (0 == edit)	return;
	if (0 == modif)	return;
	FILE *fp = fopen(fnam,"w");
	if (0 == fp)		return;
	//--- Edit area --------------------------------
	_snprintf(txt,127,ed,SW.lat, SW.lon, NE.lat, NE.lon);
	fputs(txt,fp);
	ses.Write(fp,nBLDG);
	fclose(fp);
	return;
}
//---------------------------------------------------------------------
//  Intercept events
//---------------------------------------------------------------------
void CFuiSketch::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{	switch (idm)  {
    case 'sysb':
			if (evn == EVENT_CLOSEWINDOW)	Write();
      SystemHandler(evn);
      return;
		//--- Model collection ------------------
		case 'modl':
      SystemHandler(evn);
      return;
		//--- Fill mode option -----------------
		case 'fopt':
			SetOptions(TRITOR_DRAW_FILL);
			return;
		//--- Scenery object -------------------
		case 'vopt':
			Swap3D();
			return;
		//--- View terrain ---------------------
		case 'vter':
			if (wfil)	return;
			if (tera)	State = TerrainHide();
			else			State = TerrainView();
			return;
		//--- Next Object ----------------------
		case 'nbut':
			if (wfil)	return;
			State = (State == SKETCH_WAIT)?(SKETCH_OPEN): (SKETCH_NEXT);
			return;
		//--- Load all ------------------------
		case	'vall':
			if (wfil)	return;
			State = StartLoad();
			return;
		//--- Style list -----------------------
		case 'styl':
			sBOX.VScrollHandler((U_INT)itm,evn);
			return;
		//--- New style ------------------------
		case 'nsty':
			if (wfil)	return;
			ChangeStyle();
			return;
		//--- Delete building ------------------
		case 'delt':
			if (!edit)	return;
			RemoveBuilding();
			return;
		//--- Restore building ------------------
		case 'gobj':
			if (!edit)	return;
			ResetBuilding();
			return;
		//--- Replace building ------------------
		case 'robj':
			CollectModels();
			return;
		//--- Rotate left ------------------------
		case 'lrot':
			RotateBuilding(-oneD);
			return;
		//--- Rotate right -----------------------
		case 'rrot':
			RotateBuilding(+oneD);
			return;
		//--- Update cache -----------------------
		case 'btry':
			FlyOver();
			return;
	}
	return;

}
//===================================================================================
//
//	Start OpenStreet session
//
//===================================================================================
D2_Session::D2_Session()
{	tr			= 0;
	trn			= 0;
	Stamp		= 0;
	roof = new CRoofFLAT('fixe');
	//--- Assign a default roof texture ------------------
	rtex = new D2_TParam();

}
//-----------------------------------------------------------------
//	Destroy resources
//-----------------------------------------------------------------
D2_Session::~D2_Session()
{	grpM.clear();
	delete roof;
	delete rtex;
}
//-----------------------------------------------------------------
//	Stop Session 
//-----------------------------------------------------------------
bool D2_Session::Stop01(char *ln)
{	STREETLOG("Invalid statement %s",ln);
	if (ln) exit(-1);
	return false;
}
//-----------------------------------------------------------------
//	Read session parameters 
//-----------------------------------------------------------------
bool	D2_Session::ReadParameters(char *dir)
{	char path[128];
	_snprintf(path,128,"%s/session.txt",dir);
	FILE  *f  = fopen(path,"r");
  if (0 == f)  return false;
	bool ok = ParseSession(f);
	fclose(f);
	return ok;
}
//-----------------------------------------------------------------
//	Read file 
//-----------------------------------------------------------------
char *D2_Session::ReadFile(FILE *f, char *buf)
{	bool go = true;
	while (go)
	{	fpos = ftell(f);
	 *buf  = 0;
		char *ch = fgets(buf,128,f);
		buf[127] = 0;
		if (0 == ch)		return buf;
		while ((*ch == 0x09) || (*ch == ' '))	ch++;
		if (strncmp(ch,"//", 2) == 0)		continue;
		if (*ch == 0x0A)								continue;
		return ch;
	}
	return 0;
}
//-----------------------------------------------------------------
//	Read session Name
//	NOTE: When all style parameters are OK we may compute the
//				style ratio inside of each group. 
//-----------------------------------------------------------------
bool D2_Session::ParseSession(FILE *f)
{	ReadFile(f,buf);
	char *nm = 0;
	int nf = sscanf_s(buf,"Session %63s", name,63);
	name[63] = 0;
	if (nf!= 1)				return false;
	//--- Parse Groups ----------------------
	ParseGroups(f);
	fseek(f,fpos,SEEK_SET);
	ParseStyles(f);
	//--- Normally we got the end statement here
	ReadFile(f,buf);
	_strupr(buf);
	char *ok = strstr(buf,"FIN");
	if (ok)		return true;
	//--------------------------------------
	STREETLOG("Error arround %s in Session file",buf);
	gtfo("Error in Session file");
	return false;
}
//-----------------------------------------------------------------
//	Read a group 
//-----------------------------------------------------------------
bool D2_Session::ParseGroups(FILE *f)
{	char gnm[64];
	bool go = true;
	while (go)
	{	fpos = ftell(f);
		ReadFile(f,buf);
		int nf =	sscanf_s(buf," GROUP %63s",gnm,63);
		gnm[63]  = 0;
		if (nf != 1)		return false;
		//--- Allocate a new group -----------------
		D2_Group *gp = new D2_Group(gnm,this);
		grpM[gnm] = gp;
		gp->Parse(f,this);
		groupQ.PutLast(gp);
	}
	return false;
}
//-----------------------------------------------------------------
//	Read a Style 
//-----------------------------------------------------------------
bool D2_Session::ParseStyles(FILE *f)
{ char buf[128];
  char snm[64];
	char gnm[64];
	bool go = true;
	while (go)
	{	int pos = ftell(f);
		ReadFile(f,buf);
		int nf	= sscanf_s(buf," STYLE %63[^ (] ( %64[^)]s )",snm, 63, gnm, 63);
		snm[63] = 0;
		gnm[63]	= 0;
		if (nf == 2)	{ AddStyle(f,snm,gnm);	continue; }
		//--- Back up one line --------------------
		fseek(f,pos,SEEK_SET);
		return false;
	}
	return false;
}

//-----------------------------------------------------------------
//	Add a style  
//-----------------------------------------------------------------
bool D2_Session::AddStyle(FILE *f,char *sn,char *gn)
{	//--- Find group ----------------------------
	std::map<std::string,D2_Group*>::iterator rg = grpM.find(gn);
	if (rg == grpM.end())	return Stop01(sn);
	D2_Group *gp = (*rg).second;
	D2_Style *sy = new D2_Style(sn,gp);
	//--- Decode Style parameters ---------------
	bool go = true;
	char buf[128];
	while (go)	
	{	fpos = ftell(f);	
		char * ch = ReadFile(f,buf);
		if (sy->DecodeStyle(ch))	continue;
		//-- have a new style for the group -----
		gp->AddStyle(sy);
		//--- Exit now --------------------------
		fseek(f,fpos,SEEK_SET);
		if (sy->IsOK())				return true;
		gtfo("STYLE error %s, see OpenStreet.log file",sn);
	}
	return false;
}
//------------------------------------------------------------------
//	Abort for bad parameters
//------------------------------------------------------------------
void D2_Session::Abort(char *msgr)
{	STREETLOG("%s",msgr);
	gtfo(msgr);
}
//-----------------------------------------------------------------
//	Force a Style 
//-----------------------------------------------------------------
D2_Style *D2_Session::GetStyle(char *nsty)
{	std::map<std::string,D2_Group*>::iterator rg;
	for (rg = grpM.begin(); rg != grpM.end(); rg++)
	{	D2_Group *grp		= (*rg).second;
		D2_Style *sty   = grp->GetStyle(nsty);
		if (0 == sty)	continue;
		return sty;
	}
	return 0;
}
//------------------------------------------------------------------
//	Select a roof texture from style
//------------------------------------------------------------------
D2_TParam *D2_Session::GetRoofTexture(D2_Style *sty)
{	D2_TParam *rft = sty->GetRoofTexture();
	if (0 == rft)	rft = rtex;					// Assign default
	rft->SetStyle(sty);
	return rft;
}
//------------------------------------------------------------------
//	Select all parameters
//------------------------------------------------------------------
void D2_Session::GetBuildParameters(D2_BPM *p)
{	int val = -1;
	grp			= 0;
	bpm			= p;
	if (p->stamp > Stamp)	Stamp = p->stamp;
	if (p->style)	return;
	std::map<std::string,D2_Group*>::iterator rg;
	for (rg = grpM.begin(); rg != grpM.end(); rg++)
	{	D2_Group *gp		= (*rg).second;
		int vg = gp->ValueGroup(p);
		if (vg <= val)		continue;
		grp = gp;				// Save group
		val	= vg;				// Save value
	}
	//----We have one group -----------------
	if (0 == grp) Abort("No Group found");
	if (grp->ReachQuota()) groupQ.SwitchToLast(grp);
	sty = grp->GetOneStyle();
	grp->GetOneRoof(*p);
	//--- check parameters -----------------
	if (0 == sty) Abort("No Style found");
	//--- Save parameters -------------------
	Stamp++;
	bpm->stamp  = Stamp;
	bpm->group	= grp;
	bpm->style	= sty;
	bpm->flNbr		= grp->GetFloorNbr();
	bpm->flHtr		= grp->GetFloorHtr();
	bpm->mans			= sty->IsMansart();
	return;
}
//------------------------------------------------------------------
//	Return building 
//------------------------------------------------------------------
OSM_Object *D2_Session::GetBuilding(U_INT No)
{	std::map<std::string,D2_Group*>::iterator rg;
	for (rg = grpM.begin(); rg != grpM.end(); rg++)
	{	D2_Group   *grp = (*rg).second;
		OSM_Object *bld = (*rg).second->FindBuilding(No);
		if (bld)	return bld;
	}
	return 0;
}
//------------------------------------------------------------------
//	Fill box with styles
//------------------------------------------------------------------
void D2_Session::FillStyles(CListBox *box)
{	Queue <D2_Style > *Q;
	D2_Style *sty = 0;
	std::map<std::string,D2_Group*>::iterator rg;
	for (rg = grpM.begin(); rg != grpM.end(); rg++)
	{	D2_Group *grp = (*rg).second;
		Q	= grp->GetStyleQ();
		for (sty = Q->GetFirst(); sty != 0; sty = sty->GetNext())
		{	box->AddSlot(sty);	}
	}
	return;
}
//------------------------------------------------------------------
//	Return Group number k
//------------------------------------------------------------------
std::map<std::string,D2_Group*> &D2_Session::GetGroups()
{	return grpM;	}
//------------------------------------------------------------------
//	Write all buildings
//------------------------------------------------------------------
void D2_Session::Write(FILE *fp, U_INT cnt)
{	U_INT bno = 1;
	U_INT wrt = 0;
	OSM_Object *bld = 0;

	for (bno = 1; bno <= Stamp; bno++)
	{	bld = GetBuilding(bno);
		if (0 == bld)	continue;
		bld->Write(fp);
		wrt++;
	}
	fputs("END\n",fp);
	STREETLOG("Imported %05d Buildings", cnt);
	STREETLOG("Updated  %05d Buildings", wrt);
	return;
}
//------------------------------------------------------------------
//	Store all buildings
//------------------------------------------------------------------
void D2_Session::UpdateCache()
{	OSM_Object *obj;
	for (U_INT k=1; k <= Stamp; k++)
	{	obj = GetBuilding(k);
		if (obj) globals->tcm->AddToPack(obj);
	}
	return;
}
//============================END OF FILE ================================================================================
