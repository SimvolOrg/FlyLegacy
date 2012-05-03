//===============================================================================
// OSMobjects.cpp
//
//
// Part of Fly! Legacy project
//
// Copyright 2005 Chris Wallace
// CopyRight 2007 Jean Sabatier
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
//===============================================================================
#include "../Include/FlyLegacy.h"
#include "../Include/OSMobjects.h"
#include "../Include/RoofModels.h"
#include "../Include/Terraintexture.h"
#include "../Include/fileparser.h"
//===============================================================================
//	Script to create an OSM datatbase from scratch
//===============================================================================
char *ScriptCreateOSM[] = {
	//--- Create QGT table ---------------------------------
	"CREATE TABLE QGT ( key INTEGER UNIQUE);*",
	"CREATE INDEX skey ON QGT(key)",
	//--- Create object table ------------------------------
	"CREATE TABLE OSM_OBJ ("
		"Ident   INTEGER PRIMARY KEY,"		// P01 Object identity
		"QGT		 INTEGER,"								// P02 QGT key
		"Type    INTEGER,"								// P03 Object type
		"Layer   INTEGER,"								// P04 OSM Layer
		"SupNo	 INTEGER,"								// P05 Super Tile number
		"nDir    INTEGER,"								// P06 Texture Directory nO
		"nTex    CHAR(64),"								// P07 Texture name
		"nVtx		 INTEGER,"								// P08 Number of vertices
		"dim     INTEGER,"                // P09 Size of blob
		"Vrtx    BLOB );*",								// P10 blob of vertices
	"CREATE INDEX qkey ON OSM_OBJ(QGT);*",
	//--- Last entry (not a SQL Statement) ------------------
	"***",
};
//===============================================================================
//	Script to create an Airport datatbase from scratch
//===============================================================================
char *ScriptCreateAPO[] = {
	//--- Create QGT table ----------------------------------
	"CREATE TABLE QGT ( key INTEGER UNIQUE);*",
	"CREATE INDEX skey ON QGT(key)",
	//--- Create TEXTURE table ------------------------------
	"CREATE TABLE TEX ("
			"Name CHAR(64),"									// P01 Texture name
			"wd   INTEGER,"										// P02 width
			"ht   INTEGER,"										// P03 height
			"rgba BLOB);*",										// P04 pixel blob
	"CREATE INDEX tkey ON TEX (name);*",
	//--- Create object table ------------------------------
	"CREATE TABLE OBJ ("
			"QGT    INTEGER,"									// P01 QGT key
			"SupNo  INTEGER,"									// P02 SuperTile
			"xlon   INTEGER,"									// P03 decimalized longitude
			"ylat   INTEGER,"									// P04 decimalized latitude
			"Lod    INTEGER,"									// P07 Level of Detail
			"nTex   CHAR(64),"								// P05 Texture name
			"Type   INTEGER,"									// P06 Type of object
			"nVtx   INTEGER,"									// P08 Number of vertices
			"Vrtx   BLOB);*",									// P09 blob of vertices
	"CREATE INDEX okey ON OBJ (QGT, SupNo);*",
	"CREATE UNIQUE INDEX ukey ON OBJ(xlon,ylat);*",
	//--- Last entry (not a SQL Statement) ------------------
	"***",
};
//==========================================================================================
char *EndOSM = "***";
//==========================================================================================
//  Layer name
//==========================================================================================
char *layerNAME[] = {
		"Buildings",
		"Forests",
		"Lights",
		"",
};
//==========================================================================================
//  List of amenity VALUES Tags
//==========================================================================================
OSM_CONFP amenityVAL[] = {
	//--- TAG VALUE ----OTYPE ----------OPROP ---------BVEC ----------LAYER-------------------
	{"PLACE_OF_WORSHIP",OSM_CHURCH,			OSM_PROP_BLDG, OSM_BUILD_BLDG, OSM_LAYER_BLDG},
	{"POLICE",					OSM_POLICE,			OSM_PROP_IGNR, OSM_BUILD_BLDG, OSM_LAYER_BLDG},
	{"FIRE_STATION",		OSM_FIRE_STA,		OSM_PROP_IGNR, OSM_BUILD_BLDG, OSM_LAYER_BLDG},
	{"TOWNHALL",				OSM_TOWNHALL,		OSM_PROP_IGNR, OSM_BUILD_BLDG, OSM_LAYER_BLDG},
	{"SCHOOL",					OSM_SCHOOL,			OSM_PROP_IGNR, OSM_BUILD_BLDG, OSM_LAYER_BLDG},
	{"COLLEGE",					OSM_COLLEGE,		OSM_PROP_IGNR, OSM_BUILD_BLDG, OSM_LAYER_BLDG},
	{"HOSPITAL",				OSM_HOSPITAL,		OSM_PROP_IGNR, OSM_BUILD_BLDG, OSM_LAYER_BLDG},
	{EndOSM,					0},									  // End of table
};
//==========================================================================================
//  List of BUILDING VALUES Tags
//==========================================================================================
OSM_CONFP  buildingVAL[] = {
	//--- TAG VALUE --OTYPE ----------OPROP ---------BVEC -----------LAYER -----------
	{"SCHOOL",				OSM_SCHOOL,			OSM_PROP_BLDG, OSM_BUILD_BLDG, OSM_LAYER_BLDG},
	{EndOSM,						0},									// End of table
};
//==========================================================================================
//  List of LIGHT VALUES Tags
//==========================================================================================
OSM_CONFP  liteVAL[] = {
	//--- TAG VALUE --OTYPE ----------OPROP ---------BVEC -----------LAYER ------------
	{"YES",						OSM_LIGHT,			OSM_PROP_LITE, OSM_BUILD_LITE, OSM_LAYER_LITE},
	{"RESIDENTIAL",		OSM_LIGHT,			OSM_PROP_LITE, OSM_BUILD_LITE, OSM_LAYER_LITE},
	{EndOSM,					0},									// End of table
};
//==========================================================================================
//  List of LAND VALUES Tags
//==========================================================================================
OSM_CONFP	landVAL[] = {
	//--- TAG VALUE --OTYPE ----------OPROP ---------BVEC ------------
	{"FOREST",				OSM_TREE,				OSM_PROP_TREE, OSM_BUILD_TREE, OSM_LAYER_TREE},
};
//==========================================================================================
//  List of admitted Tags
//==========================================================================================
OSM_TAG TagLIST[] = {
	//--- TAG -----Value Table ---- Layer ---
	{"AMENITY",			amenityVAL,	},
	{"BUILDING",		buildingVAL,},
	{"LIT",					liteVAL,		},
	{"HIGHWAY",			liteVAL,		},
	{"LANDUSE",     landVAL,    },
	{EndOSM,					0},									// End of table
};
//==========================================================================================
//  Local rendering vector depending on Layer
//==========================================================================================
OSM_Object::drawCB renderOSM[] = {
	&OSM_Object::DrawAsBLDG,							// 0 => OSM_LAYER_BLDG
	&OSM_Object::DrawAsTREE,							// 1 => OSM_LAYER_TREE
	&OSM_Object::DrawAsLITE,							// 2 => OSM_LAYER_LITE
};
//==========================================================================================
//  Vector to write object depending on build vector
//==========================================================================================
OSM_Object::writeCB writeOSM[] = {
	0,																		// 0 => Not OSM
	&OSM_Object::WriteAsBLDG,							// 1 => OSM_BUILD_BLDG
	&OSM_Object::WriteAsGOSM,							// 2 => OSM_BUILD_LITE
	0,																		// 3 => OSM_BUILD_AMNY
	&OSM_Object::WriteAsGOSM,							// 4 => OSM_BUILD_TREE
};
//==========================================================================================
//  Street light parameters
//==========================================================================================
float lightOSM[4] = {1,1,float(0.8),1};
U_INT lightDIM    = 64;
float alphaOSM    = float(0.25);
float lightDIS[]  = {0.0f,0.01f,0.00001f};
//==========================================================================================
//	Locate OSM value slot
//==========================================================================================
OSM_CONFP *LocateOSMvalue(char *t,char *v)
{	OSM_TAG *tab = TagLIST;
	while (strcmp(tab->tag,EndOSM) != 0)
	{	if  (strcmp(tab->tag,t)   != 0) {tab++; continue;}
	OSM_CONFP *cnf = tab->table;	
	  while (strcmp(cnf->val,EndOSM) != 0)
		{	if  (strcmp(cnf->val,v) != 0)	{cnf++; continue; }
			return cnf;
		}
		//--- unknown value --------------------------
		return 0;
	}
	//--- Unknown tag ------------------------------
	return 0;
}
//==========================================================================================
//	Locate OSM Tag
//==========================================================================================
OSM_TAG *LocateOSMtag(char *t)
{	OSM_TAG *tab = TagLIST;
	while (strcmp(tab->tag,EndOSM) != 0)
	{	if  (strcmp(tab->tag,t)   != 0) {tab++; continue;}
		return tab;
	}
	return 0;
}
//==========================================================================================
//	Check for legible tag
//==========================================================================================
U_INT  GetOSMobjType(char *t ,char *v)
{	OSM_CONFP *cnf = LocateOSMvalue(t,v);
	return (cnf)?(cnf->otype):(0);	}

//==========================================================================================
//	Get tag value configuration
//==========================================================================================
void  GetOSMconfig(char *t ,char *v, OSM_CONFP &V)
{	OSM_CONFP *cnf = LocateOSMvalue(t,v);
	OSM_TAG   *tab = LocateOSMtag(t);
	V.otype = 0;
	if (0 == cnf) {STREETLOG("Tag (%s,%s) Skipped",t,v); return;}
	V	= *cnf;
	V.tag		= tab->tag;
	return;
}
//==========================================================================================
//	Set Property
//==========================================================================================
void SetOSMproperty(char *t, char *v, U_INT P)
{	OSM_CONFP *cnf = LocateOSMvalue(t,v);
	if (0 == cnf)			return;
	cnf->prop |= P;
	return;
}
//==========================================================================================
//	Return OSM directory
//==========================================================================================
char *GetOSMdirectory(U_INT otype)
{	if (otype >= OSM_MAX)	return "";
	int num  = replOBJ[otype];
	return     directoryTAB[num];
}
//==========================================================================================
//	Return OSM directory
//==========================================================================================
char GetOSMfolder(U_INT otype)
{	if (otype >= OSM_MAX) return 0;
	return replOBJ[otype];
}
//==========================================================================================
//	Return a replacement struct for the tag-value parameters 
//==========================================================================================
OSM_MDEF *GetOSMreplacement(char *T, char *V, char *obj)
{	U_INT otype =	GetOSMobjType(T, V);
	if (0 == otype)			return 0;
	//--- fill replacement parameters ------------------
	OSM_MDEF *rep = new OSM_MDEF();
	rep->otype	= otype;
	rep->dir		= GetOSMfolder(otype);
	rep->obj		= Dupplicate(obj,FNAM_MAX);
	return rep;
}
//==========================================================================
//  OSM Object
//	NOTE:		The first PART is for the main building
//					Additional parts may come from accessory like climbox, etc
//==========================================================================
OSM_Object::OSM_Object(CBuilder *B,OSM_CONFP *CF, D2_Style *sty)
{	bld					= B;
	type				= CF->otype;
	bvec				= CF->bvec;
	Layer				= CF->layr;
	State				= 1;
	bpm.stamp		= 0;
	bpm.obj			= this;
	part				= 0;
	orien       = 0;
	style				= 0;
	tag	= val		= 0;
	if (CF->tag)	tag = Dupplicate(CF->tag,64);
	if (CF->val)	val = Dupplicate(CF->val,64);
	if (sty)	ForceStyle(sty);
}
//-----------------------------------------------------------------
//	Destroy resources
//	Queue will be deleted when object is deleted and it will delete
//	all queued objects
//-----------------------------------------------------------------
OSM_Object::~OSM_Object()
{	int a = 0;
	if (tag)	delete tag;
	if (val)	delete val;
	RazPart();
}
//-----------------------------------------------------------------
//	Clear part
//-----------------------------------------------------------------
void OSM_Object::RazPart()
{	if (part)	 delete part;	
	part	= 0;
	return;
}
//-----------------------------------------------------------------
//	Transfer Queue and Clear items
//-----------------------------------------------------------------
void OSM_Object::Swap(Queue<D2_POINT> &H)
{	//--- Clear the receiver ---------------------------
	H.Clear();
	//--- Transfert base in foot print -----------------
	base.TransferQ(H);
	return;
}
//-----------------------------------------------------------------
//	Transfer Queue and invert items
//-----------------------------------------------------------------
void OSM_Object::Invert(Queue<D2_POINT> &H)
{	//--- Clear the receiver ---------------------------
	H.Clear();
	//--- Transfert base in foot print -----------------
	D2_POINT *pp;
	for (pp = base.Pop(); pp != 0; pp = base.Pop())
	{	H.PutHead(pp);	}
	RazPart();
	return;
}
//-----------------------------------------------------------------
//	Adjust translation vector with altitude
//-----------------------------------------------------------------
void OSM_Object::AdjustZ(CVector *T)
{	if (NeedZ())	T->z += alti;
	return;
}
//-----------------------------------------------------------------
//	Transfer queue
//-----------------------------------------------------------------
void OSM_Object::ReceiveBase(D2_POINT *p0)
{	D2_POINT *pp;
  D2_POINT *np;
	for (pp = p0; pp != 0; pp = pp->next)
	{	np = new D2_POINT(pp,'R');
		base.PutLast(np);
	}
	alti		= -bpm.geop.alt;
	return;
}
//-----------------------------------------------------------------
//	Rotation to align the building
//-----------------------------------------------------------------
U_CHAR OSM_Object::Rotate()
{	if (!CanBeRotated())	  return 0;
	double sn = sin(orien);
	double cn = cos(orien);
	part->ZRotation(sn,cn);
	return 1;
}
//-----------------------------------------------------------------
//	Change rotation
//-----------------------------------------------------------------
U_CHAR OSM_Object::IncOrientation(double rad)
{	orien = WrapTwoPi(orien + rad);
	double sn = sin(rad);
	double cn = cos(rad);
	part->ZRotation(sn,cn);
	return 1;
}
//-----------------------------------------------------------------
//	Edit Object
//-----------------------------------------------------------------
int OSM_Object::EditPrm(char *txt)
{	U_INT nb  = bpm.stamp;
	int   er  = bpm.error;
	double lx = FN_METRE_FROM_FEET(bpm.lgx);
	double ly = FN_METRE_FROM_FEET(bpm.lgy);
	switch (type)	{
		case OSM_LIGHT:
			_snprintf(txt,127,"Street Light (%d spots)",bpm.side);
			return er;
		case OSM_TREE:
			return er;
		default:
			_snprintf(txt,127,"BUILDING %05d lg:%.1lf wd:%.1lf",nb,lx,ly);
			}
	return er;
}
//-----------------------------------------------------------------
//	Edit Tag
//-----------------------------------------------------------------
void OSM_Object::EditTag(char *txt)
{	if (0 == tag)		return;
	_snprintf(txt,127,"%s : %s",tag,val);
	return;
}
//-----------------------------------------------------------------
//	Force Style parameters
//-----------------------------------------------------------------
void OSM_Object::ForceStyle(D2_Style *sty)
{	bpm.style = sty;
	if (0 == sty)		return;
	sty->AssignBPM(&bpm);
	//--- search roof model and texture --------
	bpm.roofP			= sty->SelectedTexture();
	D2_Group *grp = sty->GetGroup();
	bpm.roofM     = grp->SelectedRoof();
	bpm.flNbr     = grp->GenFloorNbr();
}
//-----------------------------------------------------------------
//	Make a building
//-----------------------------------------------------------------
int OSM_Object::BuildBLDG()
{	D2_Session *ses = bld->GetSession();
	bld->QualifyPoints(&bpm);
	//--- Set generation parameters ---------
	ses->GetaStyle(&bpm);
	AssignStyle(bpm.style,bld);
	//--------------------------------------
	return OSM_COMPLET;
}
//-----------------------------------------------------------------
//	Make a light row
//-----------------------------------------------------------------
int OSM_Object::BuildLITE()
{	D2_Session *ses = bld->GetSession();
	ReceiveBase(bld->FirstNode());
	BuildLightRow(6);						// 6 meters above ground
	ses->AddLight(this);
	bld->ClearNode();
	return OSM_COMPLET;
}
//-----------------------------------------------------------------
//	Make a forest 
//-----------------------------------------------------------------
int OSM_Object::BuildFRST()
{	D2_Session *ses = bld->GetSession();
	bld->QualifyPoints(&bpm);
	bld->Triangulation();
	ReceiveBase(bld->FirstNode());
	BuildForestTour();
	ses->AddForest(this);
	return OSM_PARTIAL;
}
//-----------------------------------------------------------------
//	Assign style to object
//-----------------------------------------------------------------
void OSM_Object::AssignStyle(D2_Style *sty, CBuilder *B)
{ D2_Group *grp = sty->GetGroup();
	if (0 == bpm.flNbr)	grp->GenFloorNbr();
	//--- Get everything in bpm --------------------
	bpm.group	= grp;
	bpm.flNbr	= grp->GetFloorNbr();
	bpm.flHtr	= grp->GetFloorHtr();
	bpm.mans	= sty->IsMansart();
	if (0 == bpm.roofP)	bpm.roofP	= grp->Session()->GetRoofTexture(sty);
  if (0 == bpm.roofM)	grp->SelectOneRoof(&bpm);
	if (0 == bpm.roofM)	bpm.roofM	= grp->GetRoofModByNumber(bpm.mans);
	//--- Add the building -------------------------
	grp->AddBuilding(this);
	bld->SetBDP(bpm);
	//--- Set part parameters ----------------------
	C3DPart      *prt  = new C3DPart();
	CShared3DTex *ref  = grp->GetTREF();
	prt->Reserve(ref);
	RazPart();
	part	= prt;
	B->RiseBuilding(&bpm);
	B->SaveBuildingData(prt);
	return;
}
//-----------------------------------------------------------------
//	Change style to object
//-----------------------------------------------------------------
void OSM_Object::ChangeStyle(D2_Style *sty, CBuilder *B)
{ //--- set block parameter to me----
	sty->AssignBPM(&bpm);
	bpm.style = sty;
  bpm.opt.Raz(OSM_PROP_REPL);
	bpm.roofM	= 0;
	bpm.roofP	= 0;
	bpm.flNbr = 0;
	//--- Check for group change ------------
	D2_Group *grp = sty->GetGroup();
	if (bpm.group == grp)   return AssignStyle(sty,B);
	//--- Change group ----------------------
	D2_Group *pgp = bpm.group;
	pgp->RemBuilding(this);
	//grp->AddBuilding(this);
	AssignStyle(sty,B);
	return;
}
//-----------------------------------------------------------------
//	Translate all vertices to SuperTile center
//-----------------------------------------------------------------
GN_VTAB *OSM_Object::StripToSupertile()
{ C_QGT *qgt			= globals->tcm->GetQGT(bpm.qgKey);
	CSuperTile *sup = qgt->GetSuperTile(bpm.supNo);
	SPosition   p0  = sup->GeoPosition();
	double rad      = FN_RAD_FROM_ARCS(p0.lat);
	double rdf      = cos(rad);
	SPosition   p1	= GetPosition();
	CVector T				= FeetComponents(p0, p1,rdf);
	if (NeedZ())    T.z+= alti;
	//----------------------------------------------     
  U_INT nbv				= part->GetNBVTX();
	if (0 == nbv)		return 0;
	//--- Copy vertices from main part -------------
	GN_VTAB *src	= part->GetGTAB();
	GN_VTAB *vtx  = new GN_VTAB[nbv];
	GN_VTAB *dst  = vtx;
	for (U_INT k=0; k<nbv; k++)
	{	*dst	= *src++;
		 dst->Add(T);
		 dst++;
	}
	return vtx;
}
//-----------------------------------------------------------------
//	Return group texture reference
//-----------------------------------------------------------------
void *OSM_Object::GetGroupTREF()
{	if (0 == bpm.group)		return 0;
	return bpm.group->GetTREF();
}
//-----------------------------------------------------------------
//	Return Part texture reference
//-----------------------------------------------------------------
CShared3DTex *OSM_Object::GetPartTREF()
{return (part)?(part->GetTREF()):(0);}
//-----------------------------------------------------------------
//	Select this building
//-----------------------------------------------------------------
void OSM_Object::Select()
{	globals->osmS = this;
	bpm.selc	= 1;
	return;
}
//-----------------------------------------------------------------
//	Return Texture parameters
//-----------------------------------------------------------------
char *OSM_Object::TextureData(char &d)
{	CShared3DTex *ref = GetPartTREF();
	if (0 == ref)				return 0;
	return ref->TextureData(d);
}
//-----------------------------------------------------------------
//	Change selection on building
//-----------------------------------------------------------------
void OSM_Object::SwapSelect()
{	if (bpm.selc)	Deselect();
	else		Select();
	return;
}
//-----------------------------------------------------------------
//	Deselect this building
//-----------------------------------------------------------------
void OSM_Object::Deselect()
{	globals->osmS = 0;
	bpm.selc	= 0;
	return;
}
//-----------------------------------------------------------------
//	Replace  this object
//-----------------------------------------------------------------
void	OSM_Object::ReplaceBy(OSM_MDEF *rpp)
{	char *dir = directoryTAB[rpp->dir];
	repMD.Copy(*rpp);
  COBJparser fpar(OSM_OBJECT);
	fpar.SetDirectory(dir);
  fpar.Decode(repMD.obj,OSM_OBJECT);
	C3DPart *prt = fpar.BuildOSMPart(repMD.dir);
	if (0 == prt)							return;
	//--- Change model parameters --------------
	ReplacePart(prt);
	return; 
} 

//-----------------------------------------------------------------
//	Change part. Object is replaced 
//-----------------------------------------------------------------
void OSM_Object::ReplacePart(C3DPart *prt)
{	orien	= atan2(repMD.sinA,repMD.cosA);
	bpm.opt.Set(OSM_PROP_REPL);
	if (part)	delete part;
	part = prt;
	Rotate();
	return;
}
//-----------------------------------------------------------------
//	Build a light row at the given height
//-----------------------------------------------------------------
void OSM_Object::BuildLightRow(double ht)
{	double H = FN_FEET_FROM_METER(ht);
	part		 = new C3DPart();
	part->AllocateOsmLIT(bpm.side);
	TEXT_INFO txd;
	strncpy(txd.name,"GLOBE.PNG",FNAM_MAX);
	txd.Dir = FOLDER_OSM_USER;
	CShared3DTex *ref = globals->txw->GetM3DPodTexture(txd);
	part->SetTREF(ref);
	//--- Init all vertices ----------------
	GN_VTAB *dst = part->GetGTAB();
	D2_POINT  *pps = 0;
	for (pps = base.GetFirst(); pps != 0; pps = pps->next)
	{	pps->z		= pps->a;						// Set real altitude
		dst->VT_X = pps->x;
		dst->VT_Y	= pps->y;
		dst->VT_Z = pps->z + H;
		dst++;
	}
	return;
}

//-----------------------------------------------------------------
//	Set a tree at the spot
//	NOTE:  All trees must share the same texture ARBRES.tif
//-----------------------------------------------------------------
void OSM_Object::StoreTree(D2_POINT *pp)
{	D2_Session *ses = bld->GetSession();	// call session
	OSM_MDEF   *mdf = ses->GetTree();							// Get a tree model
	char *dir				= directoryTAB[FOLDER_OSM_TREE];
	COBJparser fpar(OSM_OBJECT);
	fpar.SetDirectory(dir);
  fpar.Decode(mdf->obj,OSM_OBJECT);
	//--- Compute translation to the spot ------------
	double alfa = RandomNumber(180);
	double  rad = DegToRad(alfa);
	double   cs = cos(rad);
	double   sn = sin(rad);
	CVector T(pp->x,pp->y,pp->a);									// Translation to pp
	fpar.Transform(cs,sn,T);											// Translate vertices
	//--- Extend actual part with new vertices -------
	GN_VTAB *buf;
	int nbv = fpar.GetVerticeStrip(&buf);
	part->ExtendOSM(nbv,buf);
	delete buf;
	//TRACE("TREE at x=%lf y=%lf z%lf",pp->x,pp->y,pp->a);
	return;
}
//-----------------------------------------------------------------
//	Build a scan line about the forest width
//-----------------------------------------------------------------
void OSM_Object::ScanLine(double y)
{	double   x = minLon;
	D2_POINT pp;

	while (x < maxLon)
	{	pp.x  = RandomCentered(x,0,70);
		pp.y  = RandomCentered(y,0,70);
		x	   += bld->GetSession()->GetSeed();
		bool in = bld->PointInBase(&pp);
		if (!in)	continue;
		//--- Set a tree ----------------------
		CVector feet(pp.x,pp.y,0);
		SPosition pos    = bpm.geop;
		AddFeetTo(pos,feet);
		GroundSpot lnd(pos.lon,pos.lat);
		pp.a = globals->tcm->GetGroundAt(lnd);
		StoreTree(&pp);
	}
}
//-----------------------------------------------------------------
//	first step: seed a tree at each point
//	NOTE: Each tree is a 4 triangles figure, thus 
//				Each tree has 12 vertices
//-----------------------------------------------------------------
void OSM_Object::BuildForestTour()
{	minLat = minLon = 0;
	maxLat = maxLon	= 0;
	part		 = new C3DPart();
	TEXT_INFO txd;
	strncpy(txd.name,"ARBRES.TIF",FNAM_MAX);
	txd.Dir = FOLDER_OSM_TREE;
	CShared3DTex *ref = globals->txw->GetM3DPodTexture(txd);
	part->SetTREF(ref);
	//-----------------------------------------------------
	for (D2_POINT *pp=base.GetFirst(); pp != 0; pp = pp->next)
	{	StoreTree(pp);
		if (pp->y < minLat)	minLat = pp->y;
		if (pp->y > maxLat)	maxLat = pp->y;
		if (pp->x < minLon) minLon = pp->x;
		if (pp->x > maxLon)	maxLon = pp->x;
	}
	pm1	= minLat;
	return;
	}
//-----------------------------------------------------------------
//	Make a forest row
//-----------------------------------------------------------------
int OSM_Object::BuildROWF()
{ //--- Check for surface -------------------------------
	if (bpm.surf < SQRF_FROM_SQRMTR(100))	return OSM_COMPLET;	
  ScanLine(pm1);
	pm1 += bld->GetSession()->GetSeed();
	return (pm1 < maxLat)?(OSM_PARTIAL):(OSM_COMPLET);
}
//-----------------------------------------------------------------
//	Draw next row
//-----------------------------------------------------------------
int OSM_Object::NextForestRow()
{	//--- Check for surface -------------------------------
	if (bpm.surf < SQRF_FROM_SQRMTR(100))	return OSM_COMPLET;
	ScanLine(pm1);												// Build a row
	pm1 += 90;
	return (pm1 > maxLat)?(OSM_COMPLET):(OSM_PARTIAL);
}
//-----------------------------------------------------------------
//	Draw as a terrain object
//-----------------------------------------------------------------
void OSM_Object::Draw()
{	//--- check for selected object -----------------
  bool ok		= (this != globals->osmS) || globals->clk->GetON();
			 ok  &= (State == 1);
	if (!ok)		return;
	//--- Get line mode -----------------------------
	U_INT mode = bld->DrawMode();
	bool  tour = (mode == 0) && (bpm.selc);
	//--- Draw my parts -----------------------------
	glLoadName(bpm.stamp);
	glPushMatrix();
	SVector T = FeetComponents(globals->geop, this->bpm.geop, bpm.rdf);
	if (NeedZ())	T.z	+= alti;
	glTranslated(T.x, T.y, T.z);  //T.z);
	part->Draw();
	if (tour)	DrawBase();	
	glPopMatrix();
	return;
}
//-----------------------------------------------------------------
//	Draw as a Building object
//	We dont translate camera at building center
//-----------------------------------------------------------------
void OSM_Object::DrawAsBLDG()
{	DebDrawOSMbuilding();	
	part->Draw();
	EndDrawOSM();
}
//-----------------------------------------------------------------
//	Draw Tour
//-----------------------------------------------------------------
void OSM_Object::DrawBase()
{	glColor4f(1,1,1,1);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINE_LOOP);
	for (D2_POINT *pp=base.GetFirst(); pp != 0; pp = pp->next)
	{	glVertex3d(pp->x,pp->y,pp->a);	}
	glEnd();
	glEnable(GL_TEXTURE_2D);
	return;
}
//-----------------------------------------------------------------
//	Draw as a Tree object
//	We dont translate camera at building center
//-----------------------------------------------------------------
void OSM_Object::DrawAsTREE()
{	U_INT mode = bld->DrawMode();
  glPushMatrix();	
  glTranslated(0,0, -bpm.geop.alt);
	DebDrawOSMforest();
	part->Draw();
	glPopMatrix();
	if (0 == mode) bld->DrawGround(1);
	EndDrawOSM();
	return;
}
//-----------------------------------------------------------------
//	Draw as a light row
//-----------------------------------------------------------------
void OSM_Object::DrawAsLITE()
{	glPushMatrix();
  glTranslated(0,0, -bpm.geop.alt);
	DebDrawOSMlight(lightOSM, alphaOSM);
	part->Draw();
  EndDrawOSM();
	glPopMatrix();
}
//-----------------------------------------------------------------
//	Write the building
//-----------------------------------------------------------------
void OSM_Object::WriteAsBLDG(FILE *fp)
{	char txt[128];
	_snprintf(txt,127,"Start %05d id=%d\n", bpm.stamp, bpm.ident);
	fputs(txt,fp);
	//--- Write style ----------------------------------
	char *nsty = bpm.style->GetSlotName();
	int   rmno = bpm.roofM->GetRoofModNumber();
	int   rofn = bpm.roofP->GetRoofTexNumber();
	int   flnb = bpm.flNbr;
	_snprintf(txt,127,"Style %s rofm=%d rftx=%d flNbr=%d \n",nsty,rmno,rofn,flnb);
	fputs(txt,fp);
	//--- Write tag ------------------------------------
	char *vl = (tag)?(val):("---");
	if (tag)
		{	_snprintf(txt,127,"Tag (%s = %s)\n",tag,vl);
			fputs(txt,fp);
		}
	//--- Write all points -----------------------------
	D2_POINT *pp;
	for (pp = base.GetFirst(); pp != 0; pp = pp->next)
	{	_snprintf(txt,127,"V(%.7lf, %.7lf)\n", pp->dgy, pp->dgx);
		fputs(txt,fp);
	}
	//--- Write replace if any -------------------------
	if (0 == repMD.obj)			return;
	//--- Write the replace directive ------------------
	_snprintf(txt,127,"Replace (Z=%.7lf) with %s\n",orien,repMD.obj);
	fputs(txt,fp);
	//--- all ok ---------------------------------------
	return;
}
//-----------------------------------------------------------------
//	Write as a generic object
//-----------------------------------------------------------------
void OSM_Object::WriteAsGOSM(FILE *fp)
{	char txt[128];
	_snprintf(txt,127,"Start %05d id=%d\n", bpm.stamp, bpm.ident);
	fputs(txt,fp);
	//--- Write tag ------------------------------------
	char *vl = (tag)?(val):("---");
	if (tag)
		{	_snprintf(txt,127,"Tag (%s = %s)\n",tag,vl);
			fputs(txt,fp);
		}
	//--- Write all points -----------------------------
	D2_POINT *pp;
	for (pp = base.GetFirst(); pp != 0; pp = pp->next)
	{	_snprintf(txt,127,"V(%.7lf, %.7lf)\n", pp->dgy, pp->dgx);
		fputs(txt,fp);
	}
	//--- all ok ---------------------------------------
	return;
}
//==================================================================
//	Draw as a local object
//==================================================================
void OSM_Object::DrawLocal()
{	if (0 == State)		return;
	(this->*renderOSM[Layer])();
	return;
}
//==================================================================
//	Write object
//==================================================================
void OSM_Object::Write(FILE *p)
{	if (0 == State)		return;
	(this->*writeOSM[bvec])(p);
	return;
}

//====================== END OF FILE ============================================
