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
#include "../Include/Model3D.h"
#include "../Include/Triangulator.h"
//================================================================================================
extern	char		*ScriptCreateOSM[];
extern	char		*ScriptCreateAPO[];
extern	void		 GetOSMattributs(char *t ,char *v, U_INT *tp, U_INT *pp);
extern	char		*GetOSMdirectory(U_INT otype);
extern	char		 GetOSMfolder(U_INT otype);
extern	OSM_REP *GetOSMreplacement(char *T, char *V, char *obj);
//==========================================================================================
// Type of object
//==========================================================================================
struct OSM_VALUE {
	char *value;
	U_INT type;
	U_INT prop;
};
//==========================================================================================
// Type of object
//==========================================================================================
struct OSM_TAG {
	char      *tag;
	OSM_VALUE *table;					// Value table
};
//==========================================================================================
// replacement structure
//==========================================================================================
struct OSM_REP {
	char  dir;								// Replacement directory (No)
	char *obr;								// Replacing object
	U_INT type;								// Object type
	U_INT prop;								// Object property
	double sinA;
	double cosA;
	//------------------------------------------------------
	OSM_REP()	{	dir = 0; 	obr	= 0; type = 0; }
	//------------------------------------------------------
	~OSM_REP(){	Clear();	}
  //------------------------------------------------------
	void Clear()	{if (obr) delete [] obr; obr = 0;}
	//-------------------------------------------------------
	void Copy(OSM_REP &R)
	{	Clear(); 
		obr = Dupplicate(R.obr, FNAM_MAX);
		dir = R.dir;
		type = R.type;
		prop = R.prop;
	}
};
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

//====================================================================================
//	Object from OSM
//====================================================================================
//-------------------------------------------------------------------------
class OSM_Object {
	friend class CBuilder;
	//--- Attributes -------------------------------------------------
	U_INT					type;										// Type of object
	D2_BPM				bpm;										// Building parameters
	//--- Replacing  object ------------------------------------------
	OSM_REP			  repMD;									// Replacing model
	double				orien;									// Orientation (rad);
	//--- OSM properties ---------------------------------------------
	char	 *tag;													// From OSM
	char   *val;													// Value
	//--- List of base POINTS ---------------------------------------
	Queue<D2_POINT> base;									// Base Points
	//--- can be replaced --------------------------------------------
	C3DPart			 *part;										// Object component
	//--- Methods ----------------------------------------------------
public:
	OSM_Object(U_INT tp);
 ~OSM_Object();
  //----------------------------------------------------------------
  void    SetTag(char *am, char *v);
	void		EditPrm(char *txt);
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
	//-----------------------------------------------------------------
	void		SetPart(C3DPart *p)				{part = p;}
	void		Select();
	void		Deselect();
	void		SwapSelect();
	void		ChangePart(C3DPart *p);
	void		ReplaceBy(OSM_REP *rpp)			{repMD.Copy(*rpp); }    
	//----------------------------------------------------------------
	void     SetParameters(D2_BPM &p);
	GN_VTAB *StripToSupertile();
	//----------------------------------------------------------------
	D2_BPM &GetParameters()			        {return bpm;}	
	SPosition GetPosition()							{return bpm.geop;}
	double    GetRDF()									{return bpm.rdf;}
	//----------------------------------------------------------------
	U_INT		 GetStamp()									{return bpm.stamp;}
	//----------------------------------------------------------------
	C3DPart *GetPart()									{return part;}
	//----------------------------------------------------------------
	char		Selection()								  {return bpm.selc;}
	//----------------------------------------------------------------
	void		SetXY(double lx,double ly)	{bpm.lgx = lx; bpm.lgy = ly;}
	//--- draw as a single object ------------------------------------
	void		Draw();
	void		DrawLocal();
	//----------------------------------------------------------------
	void		Write(FILE *fp);
};
//======================= END OF FILE ==============================================================
#endif // OSM_OBJECTS_H
