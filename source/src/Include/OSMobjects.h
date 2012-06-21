//==============================================================================================
// OSMobjects.h
//
// Part of Fly! Legacy project
//
// Copyright 2003 Chris Wallace
// Copyright 2005 Jean Sabatier
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
//================================================================================================

/*! \file Version.h
 *  \brief Definitions for application version control
 */


#ifndef OSM_OBJECTS_H
#define OSM_OBJECTS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//====================================================================================
class C_QGT;
class COBJparser;
//====================================================================================

#include "../Include/Model3D.h"
#include "../Include/Triangulator.h"
//====================================================================================
//	Object properties
//====================================================================================
#define OSM_PROP_NONE	(0x0000)
#define OSM_PROP_MAJT (0x0001)								// Major Tag
#define OSM_PROP_REPL (0x0002)								// Object is replaced
#define OSM_PROP_MREP (0x0004)								// Object can be replaced
#define OSM_PROP_MSTY	(0x0008)								// Object can change style
#define OSM_PROP_SKIP (0x0010)								// Object is skipped
#define OSM_PROP_ZNED (0x0020)								// Need Z altitude
#define OSM_PROP_WALL	(0x0040)								// Object has walls
#define OSM_PROP_ROOF (0x0080)								// Object has roof
#define OSM_PROP_STOP (0x8000)								// Stop on debug
//------------------------------------------------------------------------
#define OSM_PROP_BOTH (OSM_PROP_WALL+OSM_PROP_ROOF)
//--- Building properties ------------------------------------------------
#define OSM_PROP_BLDG			(OSM_PROP_MAJT+OSM_PROP_MREP+OSM_PROP_MSTY+OSM_PROP_BOTH)
#define OSM_PROP_IGNR			(OSM_PROP_MREP+OSM_PROP_SKIP)	
#define OSM_PROP_TREE			(OSM_PROP_MAJT+OSM_PROP_ZNED)	
#define OSM_PROP_LITE			(OSM_PROP_MAJT+OSM_PROP_ZNED)
#define OSM_PROP_PSTREET	(OSM_PROP_MAJT+OSM_PROP_ZNED)	
#define OSM_PROP_PARK		  (OSM_PROP_MAJT+OSM_PROP_BOTH+OSM_PROP_ZNED)	
#define OSM_PROP_FORTIFS	(OSM_PROP_MAJT+OSM_PROP_WALL+OSM_PROP_ZNED)
#define OSM_PROP_FARMS	  (OSM_PROP_MAJT+OSM_PROP_WALL+OSM_PROP_ZNED)
#define OSM_PROP_DOCKS		(OSM_PROP_MAJT)					
//====================================================================================
//	Object build kind
//====================================================================================
#define OSM_BUILD_GRND	(0)									// Ground object
#define OSM_BUILD_BLDG	(1)									// Building
#define OSM_BUILD_LITE	(2)									// Light
#define OSM_BUILD_AMNY	(3)									// Amenity
#define OSM_BUILD_TREE	(4)									// Trees
#define OSM_BUILD_PSTR	(5)									// Street
#define OSM_BUILD_FORT	(6)									// Fortifications
#define OSM_BUILD_FLAT  (7)									// Parks and gardens
#define OSM_BUILD_FARM	(8)									// Farm contour
#define OSM_BUILD_VMAX	(9)
//====================================================================================
//	Object type
//====================================================================================
#define OSM_BUILDING  (1)					
#define OSM_CHURCH		(2)
#define OSM_POLICE    (3)
#define OSM_FIRE_STA	(4)
#define OSM_TOWNHALL	(5)
#define OSM_SCHOOL		(6)
#define OSM_COLLEGE		(7)
#define OSM_HOSPITAL	(8)
#define OSM_HOTEL			(9)
#define OSM_CHATODO   (10)									// WATER TOWER
#define OSM_PHARES		(11)
#define OSM_PSTREET		(12)									// CITY STREETS
#define OSM_FORTIFS		(13)									// CITY WALLS (HISTORICAL)
//---------------------------------
#define OSM_DOCK			(14)									// DOCK
#define OSM_FOREST    (15)									// FOREST TREES
#define OSM_LIGHT     (16)									// ROAD LIGHT
//---------------------------------
#define OSM_RPOINT		(17)									// ROUNDABOUT
#define OSM_GARDEN		(18)									// PARK and Garden
#define OSM_SPORT     (19)	
#define OSM_PARKING   (20)									// Parkings
#define OSM_HEDGE			(21)									// Hedge
#define OSM_FARM			(22)									// Farmyard

//----------------------------------
#define OSM_MAX       (23)
//====================================================================================
#define OSM_PARTIAL   (1)
#define OSM_COMPLET   (2)
#define OSM_FAILED		(3)
//===================================================================================
//	UNIT CONVERTER 
//===================================================================================
#define FOOT_FROM_METERS(X)   (double(X) *  3.2808399)
#define SQRF_FROM_SQRMTR(X)   (double(X) * 10.7639104)
//==========================================================================================
// OSM database Request
//==========================================================================================
struct OSM_DBREQ {
	OSM_DBREQ *next;									// Next request
	C_QGT		*qgt;												// Requesting QGT
	int			 sNo;												// SUperTile number
	SQL_DB  *dbd;												// Database descriptor
	U_INT		 ident;											// Last object loaded
	//----------------------------------------------------------------
	OSM_DBREQ(C_QGT *qt, int sn, SQL_DB *db)
	{	next	= 0;
		qgt		= qt;
		sNo		= sn;
		dbd		= db;
		ident	= 0;
	}
	//----------------------------------------------------------------
	OSM_DBREQ *Next()							{return next;}
	void			 Next(OSM_DBREQ *n)	{next = n;}
};
//==========================================================================================
// Type of object
//==========================================================================================
struct OSM_CONFP {
	char *val;
	U_INT otype;
	U_INT prop;
	U_INT	bvec;						// Build vector
	char  layr;						// OSM layer
	//--- specific parameters ---------------------------
	double pm1;
	char  *pm2;
	//---------------------------------------------------
	char *tag;
	//---Set default to a building ---------------------
	void Reset()
	{ tag   = val = 0;
		layr  = OSM_LAYER_BLDG;
		otype = OSM_BUILDING;
		prop  = OSM_PROP_BLDG;
		bvec  = OSM_BUILD_BLDG;
	}
};
//==========================================================================================
extern	char		*ScriptCreateOSM[];
extern	U_INT		 GetOSMobjType(char *t ,char *v);
extern	void		 GetOSMconfig (char *t ,char *v, OSM_CONFP &V);
extern	void		 SetOSMproperty(char *t, char *v, U_INT P);
extern	char		*GetOSMdirectory(U_INT otype);
extern	char		 GetOSMfolder(U_INT otype);
extern  float    lightOSM[];
extern  float    alphaOSM;
extern  U_INT    lightDIM;
extern  float    lightDIS[];
extern	char    *layerNAME[];
extern	OSM_MDEF *GetOSMreplacement(char *T, char *V, char *obj);
//==========================================================================================
// Type of object
//==========================================================================================
struct OSM_TAG {
	char      *tag;
	OSM_CONFP *table;					// Value table
};
//==========================================================================================
// Model definition structure
//==========================================================================================
struct OSM_MDEF {
	char   dir;								// Replacement directory (No)
	char   num;								// Descriptor number
	char  *obj;								// Object name
	U_INT  otype;							// Object type
	double sinA;
	double cosA;
	//------------------------------------------------------
	COBJparser *fpar;					// Associated parser if cached
	//------------------------------------------------------
	OSM_MDEF()	{	dir = 0; 	obj	= 0; otype = 0; fpar = 0;}
	//------------------------------------------------------
	~OSM_MDEF(){	Clear();	}
  //------------------------------------------------------
	void Clear()	{if (obj) delete [] obj; obj = 0;}
	//-------------------------------------------------------
	void Copy(OSM_MDEF &R)
	{	Clear(); 
		obj = Dupplicate(R.obj, FNAM_MAX);
		dir = R.dir;
		otype = R.otype;
		sinA  = R.sinA;
		cosA  = R.cosA;
	}
};

//====================================================================================
//	Object from OSM
//====================================================================================
//-------------------------------------------------------------------------
class OSM_Object {
	friend class CBuilder;
public:
	//--- Callback vectors ----------------------------------------
	typedef int  (OSM_Object::*buildCB)(OSM_CONFP *C);	// Build vector
	typedef void (OSM_Object::*drawCB)();								// Draw vector
	typedef void (OSM_Object::*writeCB)(FILE *fp);			// Write vector

protected:
	//--- Attributes -------------------------------------------------
	CBuilder     *bld;										// Builder
	U_INT					type;										// Type of object
	U_INT         bvec;										// Build vector
	D2_BPM				bpm;										// Building parameters
	//----------------------------------------------------------------
	char				 *tag;
	char				 *val;
	//--- States -----------------------------------------------------
	U_CHAR        State;									// Existing
	U_CHAR				Layer;									// OSM layer
	U_CHAR				style;									// Style is forced
	U_CHAR				rfu;										// reserved
	//--- Replacing  object ------------------------------------------
	OSM_MDEF			 repMD;									// Replacing model
	double				orien;									// Orientation (rad);
	//--- OSM properties --------------------------------------------
	double        minLat;									// minimum Latitude
	double				maxLat;									// Maximum latitude
	double				minLon;									// Minimum longitude
	double				maxLon;									// Maximum longitude
	//--- internal parameters ---------------------------------------
	double				pm1;										// Parameter 1
	double				pm2;										// Parameter 2					
	//---------------------------------------------------------------
	double        alti;										// Z correction
	//--- List of base POINTS ---------------------------------------
	Queue<D2_POINT> base;									// Base Points
	//--- can be replaced --------------------------------------------
	C3DPart			 *part;										// Object component
	//--- Methods ----------------------------------------------------
public:
	OSM_Object(CBuilder *b,OSM_CONFP *CF,D2_Style *sty);
 ~OSM_Object();
  //---  build functions -----------------------------------------
	void		AssignBase();
	int			BuildFAIL();
	int			BuildBLDG(OSM_CONFP *CF);			// Make  building
	int			BuildLITE(OSM_CONFP *CF);			// Make a light row
	int			BuildFRST(OSM_CONFP *CF);	  	// Make a forest
	int			BuildSTRT(OSM_CONFP *CF);			// Build a street
	int			BuildGRND(OSM_CONFP *CF);			// Build ground object
	int			BuildWALL(OSM_CONFP *CF);			// Build a wall
	int			BuildFLAT(OSM_CONFP *CF);			// Build a park, garden etc
	int			BuildFARM(OSM_CONFP *CF);			// Build a farmyard
	//---------------------------------------------------------------
	int			BuildROWF(OSM_CONFP *CF);			// Make a row of forest
	//---------------------------------------------------------------
	void		ForceStyle(D2_Style *sty);
  //----------------------------------------------------------------
	void		RazPart();
	int 		EditPrm(char *txt);
	void		EditTag(char *txt);
	void		Swap(Queue<D2_POINT> &Q);
	void		Invert(Queue<D2_POINT> &H);
	//----------------------------------------------------------------
	void		ReceiveBase(D2_POINT *p0);
	//----------------------------------------------------------------
	U_CHAR	Rotate();
	U_CHAR  IncOrientation(double rad);
	//----------------------------------------------------------------
	U_INT					GetIdent()			{return bpm.ident;}
	U_INT					GetKey()				{return bpm.qgKey;}
	U_INT					GetSupNo()			{return bpm.supNo;}
	U_INT					GetType()				{return  type;}
	//----------------------------------------------------------------
	CShared3DTex *GetPartTREF();
	char   *TextureData(char &d);
	//----------------------------------------------------------------
	void   *GetGroupTREF();
	//----------------------------------------------------------------
	void		BuildLightRow(double H);
	void		BuildForestTour(OSM_MDEF *mdf);
	int		  NextForestRow();
	void		StoreTree(D2_POINT *pp, OSM_MDEF *mdf, double e);
	void		ScanLine(double y);
	void		SeedLine(D2_POINT *p0, OSM_MDEF *mdf);
	//----------------------------------------------------------------
	void		SetPart(C3DPart *p)				{part = p;}
	void		Select();
	void		Deselect();
	void		SwapSelect();
	void		ReplacePart(C3DPart *p);
	void		ReplaceBy(OSM_MDEF *rpp);
	void		AdjustPart();
	//----------------------------------------------------------------
	C3DPart *LocatePart(char d,char *txn);
	//----------------------------------------------------------------
	GN_VTAB *StripToSupertile();
	//----------------------------------------------------------------
	void		 AssignStyle(D2_Style *sty);
	void		 ChangeStyle(D2_Style *sty);
	//----------------------------------------------------------------
	D2_BPM &GetParameters()			        {return bpm;}	
	D2_BPM *Parameters()								{return &bpm;}
	SPosition GetPosition()							{return bpm.geop;}
	double    GetRDF()									{return bpm.rdf;}
	//----------------------------------------------------------------
	U_INT			GetStamp()								{return bpm.stamp;}
	D2_Style *GetStyle()								{return bpm.style;}
	U_INT			GetError()								{return bpm.error;}
	char      GetFocus()								{return bpm.selc;}
	//----------------------------------------------------------------
	C3DPart *GetPart()									{return part;}
	char    *GetTag()										{return tag;	}
	char    *GetVal()										{return val;	}
	//----------------------------------------------------------------
	char		Selection()								  {return bpm.selc;}
	char    GetLayer()									{return Layer;}
	char    GetBuildVector()						{return bvec;}
	//----------------------------------------------------------------
	void		SetXY(double lx,double ly)	{bpm.lgx = lx; bpm.lgy = ly;}
	void    Copy(D2_BPM &p)							{bpm = p;}
	void		SetIdent(U_INT n)						{bpm.ident = n;}
	//----------------------------------------------------------------
	void		Remove()							{State = 0;}
	void		Restore()							{State = 1;}
	bool	  IsValid()							{return (1 == State);}
	bool    IsaLight()						{return (type == OSM_LIGHT);}
	bool		IsSelected()					{return (bpm.selc != 0);}
	bool    SameStamp(U_INT n)		{return (bpm.stamp == n);}
	//----------------------------------------------------------------
	bool    AreYou(char *T)				{return (strcmp(T,tag) == 0);}
	char    NeedZ()								{return (bpm.opt.Has(OSM_PROP_ZNED));}
	//----------------------------------------------------------------
	char	CanBeModified()					{return bpm.opt.Has(OSM_PROP_MSTY);}
	char  CanBeRotated()					{return bpm.opt.Has(OSM_PROP_REPL);}
	char	CanBeReplaced()					{return bpm.opt.Has(OSM_PROP_MREP);}
	//--- draw as a single object ------------------------------------
	void		Draw();
	void		DrawAsBLDG();
	void		DrawAsLITE();
	void		DrawAsTREE();
	void		DrawAsGRND();
	void		DrawAsDBLE();
	void		DrawLocal();
	void		DrawBase();									
	//----------------------------------------------------------------
	void		SkipWrite(FILE *fp);
	void		Write(FILE *fp);							
	void		WriteAsBLDG(FILE *fp);
	void    WriteAsGOSM(FILE *fp);
	//----------------------------------------------------------------
	void		ObjGeoPos(SPosition &P)	{P = bpm.geop;}

};
//===================================================================================
//	Environment OSM Building
//===================================================================================
inline void DebDrawOSMbuilding()
{ glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_TEXTURE_2D);
	glColor4f(1,1,1,1);
	//--- Set client state  ------------------------------------------
	glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glFrontFace(GL_CCW);
	}
	//----------------------------------------------------------------
inline void EndDrawOSM()
{	glPopClientAttrib();
	glPopAttrib();
}
//===================================================================================
//	Environment OSM Light
//===================================================================================
inline void DebDrawOSMlight(GLfloat *col, float a)
{	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
  glAlphaFunc(GL_GREATER,a);
  glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION,lightDIS);
  glPointParameterf (GL_POINT_SIZE_MIN,1);
  glPointParameterf (GL_POINT_SIZE_MAX,40);
  glPointSize(lightDIM);
  glEnable(GL_POINT_SPRITE);
  glTexEnvi(GL_POINT_SPRITE,GL_COORD_REPLACE,GL_TRUE);
	glMaterialfv (GL_FRONT, GL_EMISSION, col);
	glColor4fv(col);
	glFrontFace(GL_CCW);
}
//===================================================================================
//	Environment OSM Building
//===================================================================================
inline void DebDrawOSMforest()
{ DebDrawOSMbuilding();
	//----------------------------------------------------------------
	glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER,float(0.6));
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glDisable(GL_CULL_FACE);
	}
//===================================================================================
//	Environment OSM Building
//===================================================================================
inline void DebDrawOSMtrees()
{ //----------------------------------------------------------------
	glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER,float(0.6));
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glDisable(GL_CULL_FACE);
	}

//===================================================================================
//	Environment Ground object
//===================================================================================
inline void DebDrawOSMground()
{ DebDrawOSMbuilding();
	//----------------------------------------------------------------
	glDisable(GL_ALPHA_TEST);
	glPolygonMode(GL_FRONT,GL_FILL);
	glEnable(GL_CULL_FACE);
	}
//======================= END OF FILE ==============================================================
#endif // OSM_OBJECTS_H
