//===============================================================================================
// TRAINGULATOR.h
//
// Part of Fly! Legacy project
//
// Copyright 2003 Chris Wallace
// Copyright 2012	Jean Sabatier
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
//===============================================================================================

#ifndef TRIANGULATOR_H
#define TRIANGULATOR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "../Include/3dMath.h"
//====================================================================================
//	Class face for stair extrusion
//====================================================================================
class D2_FACE {
	// ATTRIBUTES ------------------------------------------------
protected:
	//--- Corner definitions ----------------------------
	D2_POINT sw;										// SW corner
	D2_POINT se;										// SE corner
	D2_POINT ne;										// NE corner
	D2_POINT nw;										// NW corner
	//--- public Methods -------------------------------
public:
	D2_FACE();
 ~D2_FACE();
	//--------------------------------------------------
	void	Extrude(double f,double c,D2_POINT *sw,D2_POINT *se);
	void	Draw();
};
//====================================================================================
//	Class Stair for stair extrusion
//====================================================================================
class D2_FLOOR {
	//--- ATTRIBUTES ------------------------------------------
public:
	int			sNo;											// Stair number
	double	floor;										// Height
	double  ceil;
	std::vector<D2_FACE*> faces;			// Polygon faces
	//--- METHODS ---------------------------------------------
public:
	D2_FLOOR() {;}
	D2_FLOOR(int e, double f, double c);
 ~D2_FLOOR();
 //----------------------------------------------------------
 void	Draw();
};
//====================================================================================
//	Roof Model
//====================================================================================
class CRoofModel {
	//--- ATTRIBUTES -----------------------------------------------
	short			nbv;												// Number of vertices
	short			nbx;												// Number of index
	SVector	 *aVEC;												// Vectors
	short    *aIND;												// Indices
	D2_POINT *aOUT;												// Output vectors
	//---METHODS --------------------------------------------------
public:
	CRoofModel(int n,SVector *v,int q, short *x);
 ~CRoofModel();
  //--- clone model ---------------------------------------------
	void	CloneModel()	{aOUT = new D2_POINT[nbv];}
	int 	FillTriangle(D2_TRIANGLE &T, short k);
	//---Default is a 2 slopes roof -------------------------------
	virtual void BuildRoof(Queue <D2_POINT> &inp, std::vector<D2_TRIANGLE*> &out);
};
//====================================================================================
//	TRIANGULATOR Drawing options
//====================================================================================
#define TRITOR_DRAW_ROOF	(0x00000001)
#define TRITOR_DRAW_WALL	(0x00000002)
//--------------------------------------------
#define TRITOR_DRAW_LINE  (0x00000100)
#define TRITOR_DRAW_FILL  (0x00000200)
//---------------------------------------------
#define TRITOR_ALL (TRITOR_DRAW_ROOF+TRITOR_DRAW_WALL+TRITOR_DRAW_LINE+TRITOR_DRAW_FILL)
//====================================================================================
//	TRIANGULATOR for triangulation of polygones
//====================================================================================
class Triangulator: public CExecutable {
	//--- ATTRIBUTES ----------------------------------------------
	COption		dop;																// Drawing option
	GeoTest	geo;
	Queue <D2_POINT> extp;												// External contour
	Queue <D2_POINT> hole;												// Hole contour
	Queue <D2_SLOT>  slot;												// Slot list
	D2_POINT *P0;																	// First point
	D2_POINT *Pn;																	//  Last Point
	std::vector<D2_TRIANGLE *> roof;						  // roof output
	std::vector<D2_FLOOR*>     walls;							// Walls
	//--------------------------------------------------------------
	D2_TRIANGLE tri;															// Internal triangle
	D2_TRIANGLE qtr;															// Qualify triangle
	//--------------------------------------------------------------
	D2_POINT   *TO;																// Texture origin
	double      dlg;															// Larger diagonal
	//---------------------------------------------------------------
	double      minx;															// Mini x
	D2_POINT   *pminx;														// Point with minx
	double      maxx;															// Max  x
	D2_POINT   *pmaxx;														// Point with maxx
	double      miny;															// mini y
	D2_POINT   *pminy;														// Point with miny
	double      maxy;															// Max  y
	D2_POINT   *pmaxy;														// Point with maxy
	//----------------------------------------------------
	double	surf;																	// Ground surface
	//----------------------------------------------------
	double  rotM[4];															// Rotation matrix
	//----------------------------------------------------
	char    trace;																// Trace indicator
	char    vRFX;																	// Reflex indicator
	char    hIND;																	// Hole indicator
	char    face;																	// Type of face	
	//----------------------------------------------------
	char    stair;																// Stair numbers		
	//----------------------------------------------------
	U_INT   num;																	// Null number
	U_INT		seq;																	// Sequence number
	//--- METHODS ----------------------------------------
public:
	Triangulator();
 ~Triangulator();
  //-----------------------------------------------------
	void		AddVertex(double x, double y);
	void		NewHole();
	void		Load(char *fn);
	bool	  QualifyPoints();
	void		Triangulation();
	void		QualifyFaces();
	void		Extrude(char e,double h);
	void		Reorder();
	void		ChangeRoof(CRoofModel *m);
	void		Start();
	void		Draw();					// Drawing interface
	//-----------------------------------------------------
	inline double	GetSurface()		{return surf;}
protected:
  //-----------------------------------------------------
	void		Clean();
	void		ParseOBJ(FILE *fp);
	bool		ParseVTX(char *txt);
	bool		ParseHOL(char *txt);
	//------------------------------------------------------
	bool		NotAnEar(D2_SLOT  *sa);
	void		GetAnEar();
	void		Requalify(D2_SLOT *sp);
	//----Hole processing  -------------------------------
	void			Merge();
	D2_POINT *MatchHole(D2_POINT *xp);
	bool			CheckInternal(D2_POINT *xp, D2_POINT *hp);
	bool			CheckExternal(D2_POINT *xp, D2_POINT *hp);
	void			Splice(D2_POINT *xp, D2_POINT *hp);
	//---- Face processing --------------------------------
	void		QualifyEdge(D2_POINT *pa);
	void		BuildFloor(int no,double f, double c);
	//------------------------------------------------------
	void		TraceOut();
	void		TraceInp();
	void		TraceFace();
	//--- Drawing routines ---------------------------------
	int 		LineMode();					// Drawing interface
	int 		FillMode();
	void		DrawRoof();						// Draw roof
	void		DrawWall();					  // Draw floors
	//--- TimeSlice must return 0 to inhibit further processing --
	int			TimeSlice(float dT,U_INT frame);
	//--- Option  management -------------------------------
public:
	void		repD(U_INT p)			{dop.Rep(p);}
	char    hasR(U_INT p)			{return dop.Has(p);}
};

//============================= END OF FILE ======================================================
#endif // TRIANGULATOR_H
