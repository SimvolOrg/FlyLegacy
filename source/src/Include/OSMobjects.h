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
//====================================================================================

#include "../Include/Model3D.h"
#include "../Include/Triangulator.h"
//====================================================================================
//	Object properties
//====================================================================================
#define OSM_PROP_NONE	(0x00)
#define OSM_PROP_REPL (0x01)								// Object is replaced
#define OSM_PROP_MREP (0x02)								// Object can be replaced
#define OSM_PROP_MSTY	(0x04)								// Object can change style
#define OSM_PROP_SKIP (0x08)								// Object is skipped
//--- Building properties ------------------------------------------------
#define OSM_PROP_BLDG (OSM_PROP_MREP+OSM_PROP_MSTY)
#define OSM_PROP_IGNR (OSM_PROP_MREP+OSM_PROP_SKIP)							
//====================================================================================
//	Object build kind
//====================================================================================
 #define OSM_BUILD_BLDG	(1)
 #define OSM_BUILD_LITE (2)
 #define OSM_BUILD_AMNY (3)
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
#define OSM_TREE      (9)
#define OSM_LIGHT     (10)
//---------------------------------
#define OSM_MAX       (11)
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
	//--------------------------------------------------
	char  zned;						// Z val is needed
	char  layr;						// OSM layer
	U_INT	build;					// Build category
	char *tag;
	//---Set default to a building ---------------------
	void Reset()
	{ tag   = val = 0;
		zned	= 0;
		layr  = OSM_LAYER_BLDG;
		otype = OSM_BUILDING;
		prop  = OSM_PROP_BLDG;
		build = OSM_BUILD_BLDG;
	}
};
//==========================================================================================
extern	char		*ScriptCreateOSM[];
extern	U_INT		 GetOSMobjType(char *t ,char *v);
extern	void		 GetOSMconfig (char *t ,char *v, OSM_CONFP &V);
extern	char		*GetOSMdirectory(U_INT otype);
extern	char		 GetOSMfolder(U_INT otype);
extern  float    lightOSM[];
extern  float    alphaOSM;
extern  U_INT    lightDIM;
extern  float    lightDIS[];
extern	OSM_REP *GetOSMreplacement(char *T, char *V, char *obj);
//==========================================================================================
// Type of object
//==========================================================================================
struct OSM_TAG {
	char      *tag;
	OSM_CONFP *table;					// Value table
	U_INT			 build;					// Build category
	char			 zned;					// Z value is needed
	char			 layr;					// OSM layer
};
//==========================================================================================
// replacement structure
//==========================================================================================
struct OSM_REP {
	char   dir;								// Replacement directory (No)
	char  *obr;								// Replacing object
	U_INT  otype;								// Object type
	double sinA;
	double cosA;
	//------------------------------------------------------
	OSM_REP()	{	dir = 0; 	obr	= 0; otype = 0; }
	//------------------------------------------------------
	~OSM_REP(){	Clear();	}
  //------------------------------------------------------
	void Clear()	{if (obr) delete [] obr; obr = 0;}
	//-------------------------------------------------------
	void Copy(OSM_REP &R)
	{	Clear(); 
		obr = Dupplicate(R.obr, FNAM_MAX);
		dir = R.dir;
		otype = R.otype;
	}
};

//====================================================================================
//	Object from OSM
//====================================================================================
//-------------------------------------------------------------------------
class OSM_Object {
	friend class CBuilder;
public:
	typedef void (OSM_Object::*drawCB)();							// Draw vector
	typedef void (OSM_Object::*writeCB)(FILE *fp);		// Write vector
protected:
	//--- Attributes -------------------------------------------------
	U_INT					type;										// Type of object
	U_INT         build;									// Kind of object
	D2_BPM				bpm;										// Building parameters
	//--- States -----------------------------------------------------
	U_CHAR        State;									// Existing
	U_CHAR				Layer;									// OSM layer
	U_CHAR				style;									// Style is forced
	U_CHAR				zned;										// Need correction
	//--- Drawing vector ---------------------------------------------
	drawCB				drawFN;									// local Drawing vector	
	writeCB				writFN;									// Write function			
	//--- Replacing  object ------------------------------------------
	OSM_REP			  repMD;									// Replacing model
	double				orien;									// Orientation (rad);
	//--- OSM properties ---------------------------------------------
	char				 *tag;										// From OSM
	char				 *val;										// Value
	//---------------------------------------------------------------
	double        alti;										// Z correction
	//--- List of base POINTS ---------------------------------------
	Queue<D2_POINT> base;									// Base Points
	//--- can be replaced --------------------------------------------
	C3DPart			 *part;										// Object component
	//--- Methods ----------------------------------------------------
public:
	OSM_Object(OSM_CONFP *CF);
 ~OSM_Object();
  //---  Virtual functions -----------------------------------------
	virtual void Build()   {;}
  //----------------------------------------------------------------
	int 		EditPrm(char *txt);
	void		EditTag(char *txt);
	void		Swap(Queue<D2_POINT> &Q);
	void		Invert(Queue<D2_POINT> &H);
	//----------------------------------------------------------------
	void		ReceiveQ(Queue<D2_POINT> &Q);
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
	//----------------------------------------------------------------
	void		SetPart(C3DPart *p)				{part = p;}
	void		Select();
	void		Deselect();
	void		SwapSelect();
	void		ReplacePart(C3DPart *p);
	void		ReplaceBy(OSM_REP *rpp)			{repMD.Copy(*rpp); } 
	//----------------------------------------------------------------
	void		AdjustZ(CVector *V);
	//----------------------------------------------------------------
	//void     SetParameters(D2_BPM *p);
	GN_VTAB *StripToSupertile();
	//----------------------------------------------------------------
	void		 AssignStyle(D2_Style *sty, CBuilder *B);
	void		 ChangeStyle(D2_Style *sty, CBuilder *B);
	//----------------------------------------------------------------
	D2_BPM &GetParameters()			        {return bpm;}	
	SPosition GetPosition()							{return bpm.geop;}
	double    GetRDF()									{return bpm.rdf;}
	//----------------------------------------------------------------
	U_INT			GetStamp()								{return bpm.stamp;}
	D2_Style *GetStyle()								{return bpm.style;}
	U_INT			GetError()								{return bpm.error;}
	char      GetFocus()								{return bpm.selc;}
	//----------------------------------------------------------------
	C3DPart *GetPart()									{return part;}
	//----------------------------------------------------------------
	char		Selection()								  {return bpm.selc;}
	char    GetLayer()									{return Layer;}
	//----------------------------------------------------------------
	void		SetXY(double lx,double ly)	{bpm.lgx = lx; bpm.lgy = ly;}
	void    Copy(D2_BPM &p)							{bpm = p;}
	//----------------------------------------------------------------
	void		Remove()										{State = 0;}
	void		Restore()										{State = 1;}
	bool	  IsValid()									  {return (1 == State);}
	bool    IsaLight()									{return (type == OSM_LIGHT);}
	bool		IsSelected()								{return (bpm.selc != 0);}
	bool    SameStamp(U_INT n)					{return (bpm.stamp == n);}
	//----------------------------------------------------------------
	bool    AreYou(char *T)							{return (strcmp(T,tag) == 0);}
	//----------------------------------------------------------------
	char	CanBeModified()								{return bpm.opt.Has(OSM_PROP_MSTY);}
	char  CanBeRotated()								{return bpm.opt.Has(OSM_PROP_REPL);}
	char	CanBeReplaced()								{return bpm.opt.Has(OSM_PROP_MREP);}
	//--- draw as a single object ------------------------------------
	void		Draw();
	void		DrawAsBLDG();
	void		DrawAsLITE();
	void		DrawLocal();									
	//----------------------------------------------------------------
	void		Write(FILE *fp);							
	void		WriteAsBLDG(FILE *fp);
	void    WriteAsLITE(FILE *fp);
	//----------------------------------------------------------------
	void		ObjGeoPos(SPosition &P)	{P = bpm.geop;}

};
//===================================================================================
//	Environment OSM Building
//===================================================================================
inline void DebDrawOSMbuilding()
{ glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_TEXTURE_2D);
	glColor3f(255,255,255);
	//--- Set client state  ------------------------------------------
	glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glFrontFace(GL_CCW);
	}
	//----------------------------------------------------------------
inline void EndDrawOSMbuilding()
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
  glAlphaFunc(GL_GREATER,a);
  glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION,lightDIS);
  glPointParameterf (GL_POINT_SIZE_MIN,1);
  glPointParameterf (GL_POINT_SIZE_MAX,64);
  glPointSize(lightDIM);
  glEnable(GL_POINT_SPRITE);
  glTexEnvi(GL_POINT_SPRITE,GL_COORD_REPLACE,GL_TRUE);
	glMaterialfv (GL_FRONT, GL_EMISSION, col);
	glColor4fv(col);
}
//-------------------------------------------------------------------
inline void EndDrawOSMlight()
{	glPopClientAttrib();	
	glPopAttrib();
}
//======================= END OF FILE ==============================================================
#endif // OSM_OBJECTS_H
