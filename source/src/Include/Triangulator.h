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
#include "../Include/3dMath.h"//====================================================================================
//==== GEOMETRIC FUNCTIONS =================================================================
#define GEO_OPPOS_SIDE	(0)
#define GEO_INSIDE_PQ		(1)
#define GEO_OUTSIDE_PQ  (2)
#define GEO_SAME_SIDE		(3)
//--- Side of a point -----------------------------
#define GEO_ON_PQ			(4)
#define GEO_LEFT_PQ		(1)				// Left side
#define GEO_RITE_PQ		(2)				// right side
//-------------------------------------------------
#define GEO_OUTSIDE   (0)
#define GEO_ON_SIDE		(1)
#define GEO_INSIDE		(2)
//--- Type of POINT -------------------------------
#define GEO_CONVEX		(0)
#define GEO_REFLEX		(1)
//--- Type of FACE --------------------------------
#define GEO_FACE_NUL	(0)
#define GEO_X_FACE	  (0x00)
#define GEO_Y_FACE    (0x01)
#define GEO_N_FACE    (0x02)  // Negative face	
//--------------------------------------------------
#define GEO_FACE_XP   (0)
#define GEO_FACE_YP  (GEO_Y_FACE)
#define GEO_FACE_XM  (GEO_N_FACE)
#define GEO_FACE_YM  (GEO_Y_FACE + GEO_N_FACE)
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
//	Texturing mode
//====================================================================================
//--- Parameter surface type sType
#define TEXD2_XPLUS	 (0)												// Texture for X+
#define	TEXD2_YPLUS	 (1)												// Texture for Y+
#define	TEXD2_X_MINUS (2)												// Texture for X-
#define	TEXD2_Y_MINUS (3)												// Texture for Y-
//--- Floor mode  zMode -------------------------------------
#define TEXD2_WHOLE   (0)				// Texture for all floors
#define TEXD2_FLOOR1  (1)				// One texture for ground floor
#define TEXD2_FLOORM  (2)				// One texture for middle floors
#define TEXD2_FLOORZ	(3)				// One texture for last floor
//--- Roof Mode rMode ----------------------------------------
#define TEXD2_COLOR		(0)				// Mono color
#define TEXD2_RECT		(1)				// Texture rectangle specified 
//----  Texture matrix indices and dimension -----------------
#define TEXD2_MAT_DIM (16)
#define TEXD2_IND_XP  (0)
#define TEXD2_IND_YP  (4)
#define TEXD2_IND_XM  (8)
#define TEXD2_IND_YM	(12)
//---- Floor index --------------------------------------------
#define TEXD2_IND_GF  (1)
#define TEXD2_IND_MF  (2)
#define TEXD2_IND_ZF  (3)
//======================================================================================
//	Pixel definition
//======================================================================================
struct D2_PIXEL {
	double x;
	double y;
	//---Constructor ----------------------------
	D2_PIXEL::D2_PIXEL()	{x = y = 0;}
	};
//======================================================================================
//  2D point coordinates
//======================================================================================
struct D2_POINT {
	D2_POINT *next;															// Next point
	D2_POINT *prev;															// Previous
	char   idn[4];															// Point ident
	//---------------------------------------------------------
  char   R;																		// Type of point
	char   F;																		// Type of face
	char   rfu1;																// Not used 
	char   rfu2;																// Not used
	//--- Space coordinates ------------------------------------
	double x;																		// X coordinate
  double y;																		// Y coordinate
	double z;																		// Z coordinate
	//--- Texture coordinate -----------------------------------
	double s;																		// along x
	double t;																		// along y
	//--- Local space -------------------------------------------
	double lx;																	// X coordinate
	double ly;																	// Y coordinate
	//-----------------------------------------------------------
	double elg;																	// Edge lenght
	double l2D;																	// 2D lenght
	//---- Methods -----------------------
	D2_POINT::D2_POINT() {x = y = z =0; R= 0; next = prev = 0;}
	D2_POINT::D2_POINT(double x,double  y)
	 {	R	= F = 0;	
			this->x = x;
			this->y = y;
			z       = 0;
			next = prev = 0;
		}
	//-----------------------------------
	D2_POINT::D2_POINT(D2_POINT &p)	{*this = p; elg = 0;}
	//-----------------------------------
	void  Id(char *d)			{strncpy(d,idn,4); d[4] = 0;}
	void  SetID(char *d)	{strncpy(idn,d,4);}
	//-----------------------------------
	void	D2_POINT::Convex()			{R = 0;}
	void	D2_POINT::Reflex()			{R = 1;}
	void  D2_POINT::Type(char n)	{R = n;}
	bool  D2_POINT::IsReflex()	  {return (R == 1);}
	bool  D2_POINT::NotNulEdge()	{return (*idn != 'N');}
	//-----------------------------------
	char  D2_POINT::GetType()		{return *idn;}
	void	D2_POINT::SetEdge(char n)	{*idn = n;}
	//-----------------------------------
	void	D2_POINT::SetFace(char n)	{F	= n;}
	char  D2_POINT::GetFaceType() {return F;}
	//----------------------------------------
	void  D2_POINT::Draw()				{	glVertex3f(x,y,z);}
	//----------------------------------------
	void  D2_POINT::Add(D2_POINT &p, SVector &v)
	{	x = p.x + v.x;
		y = p.y + v.y;
		z = p.z + v.z;
	}
	//----------------------------------------
	void D2_POINT::Translate(double tx,double ty)
	{	x += tx;
		y += ty;
	}
	//------------------------------------------
};
//======================================================================================
//  2D triangles
//	Orientation is B->A->C->B
//======================================================================================
struct D2_TRIANGLE {
	char	type;
	D2_POINT *B;				// Previous point
	D2_POINT *A;				// current vertex
	D2_POINT *C;				// Next point
	//----------------------------------------------
	D2_TRIANGLE::D2_TRIANGLE() { type = 0; A=B=C=0;}
	//---- Allocate D2_POINTS ----------------------
	D2_TRIANGLE::D2_TRIANGLE(char a)
	{	type = 1; A=B=C=0;}
	//----- Destruction ----------------------------
	D2_TRIANGLE::~D2_TRIANGLE()
		{}
	//----- Methods --------------------
	void	D2_TRIANGLE::Set(D2_POINT *P, D2_POINT *S, D2_POINT *N)
	 {	A = S; 
			B = P;
			C = N;
		}
	//----------------------------------------------
	void  D2_TRIANGLE::Draw()
	{	B->Draw();
		A->Draw();
		C->Draw();
	}
};
//=====================================================================
//	Queue Header. Used for queuing D2_POINT object
//=====================================================================
template <class T> class Queue {
protected:
	//--------------Attributes ------------------------------------
	U_SHORT			NbOb;									// Item number
	T	 *First;					              // First object in queue
	T	 *Last;													// Last  object in queue
	T  *Prev;
public:
  Queue();
 ~Queue();
  //---------inline ---------------------------------------------
  inline  T*  GetFirst()         {return First;}
  inline  T*  GetLast()          {return Last; }
  inline  T*  NextFrom(T *p)     {return (p)?(p->next):(0);}
  inline  T*  PrevFrom(T *p)     {return (p)?(p->prev):(0);}
  inline  U_SHORT     GetNbObj() {return NbOb;}
  inline  bool        NotEmpty()  {return (0 != NbrOb);}
	//--- Circular links -----------------------------------------
	inline  T*  CyNext(T *p)	{T* n = p->next; if (0 == n) n = First; return n;}
	inline  T*  CyPrev(T *p)	{T* n = p->prev; if (0 == n) n = Last;  return n;}
	//------------------------------------------------------------
	inline  void  Update(int k,T *p, T *q) {NbOb += k; if (p == Last) Last = q;}
	//------------------------------------------------------------
  void        Clear();
  void        PutLast (T *obj);
  void        PutBefore(T *ne,T *obj);
  T          *Detach(T *obj);
  T          *Pop();
	void	      Transfer(T **h, T **e);
	T          *Remove(T *obj);

};
//==========================================================================
//  GENERIC QUEUE MANAGEMENT
//==========================================================================
template <class T> Queue<T>::Queue()
{ NbOb    = 0;
  First   = 0;
  Last    = 0;
  Prev    = 0;
}
//----------------------------------------------------------
//	Destory all elements
//----------------------------------------------------------
template <class T> Queue<T>::~Queue()
{	Clear();	}
//----------------------------------------------------------
//	Insert element Obj at end of queue
//----------------------------------------------------------
template <class T> void	Queue<T>::PutLast (T *obj)
{	NbOb++;								                    // Increment count
	obj->next	 = 0;							              // No next
	T *lo = Last;															// Get the last object
	obj->prev	 = lo;							            // Link to previous
	Last	     = obj;							            // This is the last
	if (lo == NULL)	  First    = obj;					// Queue was empty
	if (lo != NULL)		lo->next = obj;					// Link previous to new
	return ;	
}
//----------------------------------------------------------
//	Insert new element ne after object obj
//----------------------------------------------------------
template <class T> void Queue<T>::PutBefore(T *ne,T *obj)
{ T *prv = (obj)?(obj->prev):(Last);
  if (prv)  prv->next = ne;
  ne->next  = obj;
  if (obj)  obj->prev = ne;
  ne->prev  = prv;
  if (0 ==     obj)   Last  = ne;
  if (First == obj)   First = ne;
  NbOb++;
  return;  
}
//----------------------------------------------------------
//	Detach element  obj from Queue return the previous
//----------------------------------------------------------
template <class T> T* Queue<T>::Detach(T *obj)
{	T	*nx = obj->next;					      // next object
	T	*pv = obj->prev;					      // Previous
	NbOb--;														// Decrease count
	if (nx)		nx->prev = pv;					// Relink next
	if (pv)		pv->next = nx;					// Relink previous
	if (First == obj)	    First = nx;	// New first
	if (Last  == obj)	    Last  = pv;	// New last
	obj->next	= 0;								    // Clear detached next
	obj->prev = 0;								    // Clear detached previous
	return pv;	}
//---------------------------------------------------------------
//	Detach element  obj from Queue without destroying the pointer
//---------------------------------------------------------------
template <class T> T* Queue<T>::Remove(T *obj)
{	T	*nx = obj->next;					      // next object
	T	*pv = obj->prev;					      // Previous
	NbOb--;														// Decrease count
	if (nx)		nx->prev = pv;					// Relink next
	if (pv)		pv->next = nx;					// Relink previous
	if (First == obj)	    First = nx;	// New first
	if (Last  == obj)	    Last  = pv;	// New last
	return obj;	}

//---------------------------------------------------------------------
//	Pop the first object from Queue
//---------------------------------------------------------------------
template <class T> T*	Queue<T>::Pop()	
{	T	*obj	  = First;															// Pop the first
	if (obj)	{First  = obj->next; NbOb--; }		    // Update header
	if (First)	First->prev = 0;										// First has no previous
	if (Last == obj)	  Last = 0;					          // Queue is now empty
	if (0 == obj)	return obj;				
	obj->next	= 0;
	obj->prev	= 0;
	return obj;	}
//-----------------------------------------------------------------------
//	Delete all objects in a queue
//-------------------------------------------------------------------------
template <class T> void Queue<T>::Clear()
{	T *obj = Pop();
	while (obj)
	{	delete obj;
		obj	= Pop();
	}
	return;
}
//-----------------------------------------------------------------------
//	Transfer Queue
//-------------------------------------------------------------------------
template <class T> void Queue<T>::Transfer(T **h, T **e)
{ *h = First;
	*e = Last;
	First = Last = 0;
	NbOb	= 0;
}
//=========================================================================================
//	Geometric tester
//=========================================================================================
class GeoTest {
	//--- ATTRIBUTES ----------------------------------------
	double precision;													// Precision limit
	//--- Methods -------------------------------------------
public:
	GeoTest()		{precision	= DBL_EPSILON;}
	//--- Check side ----------------------------------------------
	int		SameSide(D2_POINT &P, D2_POINT &Q, D2_POINT &A, D2_POINT &B);
	bool	VisibilityTest(D2_POINT &P, D2_POINT &Q, D2_POINT &A, D2_POINT &B);
	int 	OnLeft(D2_POINT &A, D2_POINT &P, D2_POINT &Q);
	int 	InTriangle(D2_POINT &A, D2_POINT &P, D2_POINT &Q, D2_POINT &R);
	int   InTriangle(D2_POINT &A, D2_TRIANGLE &T);
	int	  Positive(D2_TRIANGLE &T);
	//-------------------------------------------------------------
	inline void		SetPrecision(double p)	{precision = p;}
	inline double GetPrecision()					{return precision;}
};
//====================================================================================
//	Slot to a 2D_POINT
//	Used for triagulation of the polygon
//====================================================================================
struct D2_SLOT {
	D2_SLOT  *prev;
	D2_SLOT  *next;
	D2_POINT *vrtx;
	//-----------------------------------------------
	char     R;							// Reflex indicator
	//-----------------------------------------------
	D2_SLOT::D2_SLOT(D2_POINT *p)
	{	prev	= 0;	next	= 0;	vrtx	= p; R = p->R;	}
	//-----------------------------------------------
	D2_POINT *D2_SLOT::GetVRTX()	{return vrtx;}
	//-----------------------------------------------
	bool D2_SLOT::IsReflex()			{return (R != 0);}
	//------------------------------------------------
	void D2_SLOT::SetType(char t)	{R = t;}
	};


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
//	data for Texture definition
//====================================================================================
class D2_TParam {
friend class D2_Style;
	//--- ATTRIBUTES ----------------------------------------------
	char code;														// Code texture
	char rept;														// Repeat number
	char user;														// User count
	char rfu2;														// Reserved 2
	//--------------------------------------------------------------
	int		x0;															// SW corner
	int		y0;															// 
	int		x1;															// NE corne
	int   y1;
	//--------------------------------------------------------------
	U_INT *mem;														// Memory image
	//--- METHODS --------------------------------------------------
public:
	D2_TParam()	{user = 0;}
 ~D2_TParam();
  //--- ---------------------------------------------------
	void IncUser()	{user++;}
	void DecUser()  {user--; if (user <= 0) delete this;}
};
//====================================================================================
//	data for Style definition 
//====================================================================================
class D2_Group;
//------------------------------------------------------------------------------------
class D2_Style {
	//--- ATTRIBUTES -----------------------------------------------
	char			 name[63];									// Style name
	char       filn[63];									// Texture file name					
	D2_Group  *group;											// Parent group
	short			 weight;										// relative weight in group
	char			 minF;											// minimum floor number
	char			 maxF;											// maximum floor number
	//--------------------------------------------------------------							
	D2_TParam *param[TEXD2_MAT_DIM];			// Texture array
	//--- METHODS -------------------------------------------------
public:
	D2_Style(char *sn, D2_Group *gp);
 ~D2_Style();
  void			Clear(D2_TParam *p);
  //-------------------------------------------------------------
	bool			Error1 (char *s);
	bool			Error2 (char *s);
	bool			Decode(char *buf);
	bool			AddFace(D2_TParam &p);
	bool			AddFloor(char *fl, int n, D2_TParam &p);
	//-------------------------------------------------------------
	bool			IsOK();
};
//====================================================================================
//	data for group definition 
//====================================================================================
class D2_Session;
class D2_Group {
	//--- ATTRIBUTES ----------------------------------------------
	char  name[64];													// Group name
	double	sfMin;													// min surface
	double  sfMax;													// Max surface
	int			sdMin;													// side number
	int			sdMax;													// side max
	std::vector<D2_Style*> styles;					// All styles in group
	//--- METHODS --------------------------------------------------
public:
	D2_Group(char *gn);
 ~D2_Group();
  //--------------------------------------------------------------
	bool Parse(FILE *f, D2_Session *sn);
	bool DecodeParam(char *buf);
	//--------------------------------------------------------------
	void		AddStyle(D2_Style *sy)		{styles.push_back(sy);}

	};

//====================================================================================
//	data for session 
//====================================================================================
class D2_Session {
	//--- ATTRIBUTES ------------------------------------------------
	char  name[64];													// Session name
	std::map<std::string,D2_Group*> group;	// Groups
	U_INT  fpos;														// File position
	//---METHODS ----------------------------------------------------
public:
	D2_Session();
 ~D2_Session();
	//----Session decoding ------------------------------------
	bool	Stop01(char *ln);
	void	ReadFile(FILE *f, char *buf);
	bool	ReadParameters(char *fn);
	bool  ParseSession(FILE *f);
	bool	ParseGroups(FILE *f);
	bool  ParseStyles(FILE *f);
	bool	AddStyle(FILE *f,char *sn,char *gn);
	//---------------------------------------------------------------
	inline void BackTo(int p)	{fpos = p;}
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
	void	CloneModel();
	int 	FillTriangle(D2_TRIANGLE &T, short k);
	//---Default is a 2 slopes roof -------------------------------
	virtual D2_POINT *BuildRoof(Queue <D2_POINT> &inp, std::vector<D2_TRIANGLE*> &out);
};
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
	D2_POINT *roofArray;													// Sloped roof
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
	//---Parameters blocs --------------------------------
	//--- METHODS ----------------------------------------
public:
	Triangulator();
 ~Triangulator();
	//-----------------------------------------------------
  //-----------------------------------------------------
	void		AddVertex(double x, double y);
	void		NewHole();
	bool		Load(char *fn);
	bool	  QualifyPoints();
	bool		Triangulation();
	void		QualifyFaces();
	void		Extrude(char e,double h);
	void		Reorder();
	void		ChangeRoof(CRoofModel &m);
	void		Start();
	void		Draw();					// Drawing interface
	//-----------------------------------------------------
	inline double	GetSurface()		{return surf;}
	inline int    NbPoints()			{return extp.GetNbObj();}
protected:
  //-----------------------------------------------------
	void		Clean();
	void		ParseOBJ(FILE *fp);
	bool		ParseVTX(char *txt);
	bool		ParseHOL(char *txt);
	//------------------------------------------------------
	bool		NotAnEar(D2_SLOT  *sa);
	bool		GetAnEar();
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
