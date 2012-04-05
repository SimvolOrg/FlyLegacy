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
//===============================================================================
//	Script to create an OSM datatbase from scratch
//===============================================================================
char *ScriptCreateOSM[] = {
	//--- Create QGT table ---------------------------------
	"CREATE TABLE QGT ( key INTEGER UNIQUE);*",
	"CREATE INDEX skey ON QGT(key)",
	//--- Create object table ------------------------------
	"CREATE TABLE OSMbuilding ("
		"Ident   INTEGER PRIMARY KEY,"		// P01 Object identity
		"QGT		 INTEGER,"								// P02 QGT key
		"Type    INTEGER,"								// P03 Object type
		"SupNo	 INTEGER,"								// P04 Super Tile number
		"nDir    INTEGER,"								// P05 Texture Directory nO
		"nTex    CHAR(64),"								// P06 Texture name
		"nVtx		 INTEGER,"								// P07 Number of vertices
		"dim     INTEGER,"                // P08 Size of blob
		"Vrtx    BLOB );*",								// P09 blob of vertices
	"CREATE INDEX qkey ON OSMbuilding(QGT);*",
	//--- Last enry (not a SQL Statement) ------------------
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
//  List of AMENITY VALUES Tags
//==========================================================================================
OSM_VALUE amenityVAL[] = {
	{"PLACE_OF_WORSHIP",	OSM_CHURCH,			0},
	{"POLICE",						OSM_POLICE,			0},
	{"FIRE_STATION",			OSM_FIRE_STA,		0},
	{"TOWNHALL",					OSM_TOWNHALL,		0},
	{"SCHOOL",						OSM_SCHOOL,			0},
	{"COLLEGE",						OSM_COLLEGE,		0},
	{"HOSPITAL",					OSM_HOSPITAL,		0},
	{EndOSM,					0},									  // End of table
};
//==========================================================================================
//  List of BUILDING VALUES Tags
//==========================================================================================
OSM_VALUE  buildingVAL[] = {
		{"SCHOOL",					OSM_SCHOOL,			0},
		{EndOSM,						0},									// End of table
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
//	Check for legible value
//==========================================================================================
void CheckValue(OSM_VALUE *tab,char *v,U_INT *tp, U_INT *pp)
{	while (strcmp(tab->value,EndOSM) != 0)
	{ if  (strcmp(tab->value,v) != 0)	{tab++; continue; }
		*tp = tab->type;
		*pp = tab->prop;
		return;
	}
  //--- Unsupported value --------------------
	*tp = 0;
	*pp	= 0;
	return;
}
//==========================================================================================
//	Check for legible tag
//==========================================================================================
void  GetOSMattributs(char *t ,char *v, U_INT *tp, U_INT *pp)
{	OSM_TAG *tab = TagLIST;
	while (strcmp(tab->tag,EndOSM) != 0)
	{	if  (strcmp(tab->tag,t)   == 0)	return CheckValue(tab->table,v,tp,pp);
		tab++;
	}
	//--- tag not supported --------------------
	*tp = 0;
	*pp = 0;
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
OSM_REP *GetOSMreplacement(char *T, char *V, char *obj)
{	U_INT otype, oprop;
	GetOSMattributs(T, V, &otype, &oprop);
	if (0 == otype)			return 0;
	//--- fill replacement parameters ------------------
	OSM_REP *rep = new OSM_REP();
	rep->type = otype;
	rep->prop = oprop;
	rep->dir  = GetOSMfolder(otype);
	rep->obr  = Dupplicate(obj,FNAM_MAX);
	return rep;
}
//==========================================================================
//  OSM Object
//==========================================================================
OSM_Object::OSM_Object(U_INT tp)
{	type				= tp;
	bpm.stamp		= 0;
	bpm.group		= 0;
	tag					= 0;
	val					= 0;
	part				= 0;
	orien       = 0;
}
//-----------------------------------------------------------------
//	Destroy resources
//	Queue will be deleted when object is deleted and it will delete
//	all queued objects
//-----------------------------------------------------------------
OSM_Object::~OSM_Object()
{	int a = 0;
	if (tag)			delete [] tag;
	if (val)			delete [] val;
	if (part)			delete part;
	//TRACE("DELETE BUILDING %d",bpm.stamp);
}
//-----------------------------------------------------------------
//	Transfer Queue and Clear items
//-----------------------------------------------------------------
void OSM_Object::Swap(Queue<D2_POINT> &H)
{	//--- Clear the receiver ---------------------------
	H.Clear();
	//--- Transfert base in foot print -----------------
	base.TransferQ(H);
	if (part)		delete part;
	part = 0;
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
	if (part)		delete part;
	part = 0;
	return;
}

//-----------------------------------------------------------------
//	Transfer queue
//-----------------------------------------------------------------
void OSM_Object::ReceiveQ(Queue<D2_POINT> &H)
{	D2_POINT *pp;
  D2_POINT *np;
	for (pp = H.GetFirst(); pp != 0; pp = pp->next)
	{	np = new D2_POINT(pp,'R');
		base.PutLast(np);
	}
	return;
}
//-----------------------------------------------------------------
//	Rotation to align the building
//-----------------------------------------------------------------
U_CHAR OSM_Object::Rotate()
{	if (bpm.opt.Not(D2B_REPLACED))	  return 0;
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
//	Save amenity
//-----------------------------------------------------------------
void OSM_Object::SetTag(char *am, char *vl)
{	if (0 ==  *am)	return;
  tag = Dupplicate(am,64);
	if (0 == *vl)		return;
	val	= Dupplicate(vl,64);
	return;
}
//-----------------------------------------------------------------
//	Edit Object
//-----------------------------------------------------------------
void OSM_Object::EditPrm(char *txt)
{	U_INT nb  = bpm.stamp;
	double lx = FN_METRE_FROM_FEET(bpm.lgx);
	double ly = FN_METRE_FROM_FEET(bpm.lgy);
	_snprintf(txt,127,"BUILDING %05d lg:%.1lf wd:%.1lf",nb,lx,ly);
	return;
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
//	Save building in mother group
//-----------------------------------------------------------------
void OSM_Object::SetParameters(D2_BPM &p)
{	//--- check for group change ----------------------------
	bool chge = (bpm.group != 0) && (bpm.group != p.group);
	if (chge) bpm.group->RemBuilding(this);
	bpm				= p;
	D2_Group *grp = bpm.group;
	grp->AddBuilding(this);
	//--- Set part parameters ----------------------
	C3DPart      *prt  = new C3DPart();
	CShared3DTex *ref  = grp->GetTREF();
	prt->Reserve(ref);
	if (part)	delete part;
	part	= prt;
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
  U_INT nbv				= part->GetNBVTX();
	if (0 == nbv)		return 0;
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
//	Swap part
//-----------------------------------------------------------------
void OSM_Object::ChangePart(C3DPart *prt)
{	orien	= atan2(repMD.sinA,repMD.cosA);
	bpm.opt.Set(D2B_REPLACED);
	if (part)	delete part;
	part = prt;
	Rotate();
	return;
}
//-----------------------------------------------------------------
//	Draw as a terrain object
//-----------------------------------------------------------------
void OSM_Object::Draw()
{	glLoadName(bpm.stamp);
	glPushMatrix();
	SVector T = FeetComponents(globals->geop, this->bpm.geop, bpm.rdf);
	glTranslated(T.x, T.y, T.z);  //T.z);
	part->Draw();	
	glPopMatrix();
}
//-----------------------------------------------------------------
//	Draw as a single local object
//	We dont translate camera at building center
//-----------------------------------------------------------------
void OSM_Object::DrawLocal()
{	part->Draw();	 }
//-----------------------------------------------------------------
//	Write the building
//-----------------------------------------------------------------
void OSM_Object::Write(FILE *fp)
{	char txt[128];
	_snprintf(txt,127,"Start %05d id=%d\n", bpm.stamp, bpm.ident);
	fputs(txt,fp);
	//--- Write style ----------------------------------
	char *nsty = bpm.style->GetSlotName();
	int   rmno = bpm.roofM->GetRoofModNumber();
	int   rofn = bpm.roofP->GetRoofTexNumber();
	_snprintf(txt,127,"Style %s rofm=%d rftx=%d\n",nsty,rmno,rofn);
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
	if (0 == repMD.obr)			return;
	//--- Write the replace directive ------------------
	_snprintf(txt,127,"Replace (Z=%.7lf) with %s\n",orien,repMD.obr);
	fputs(txt,fp);
	//--- all ok ---------------------------------------
	return;
}
//====================== END OF FILE ============================================
