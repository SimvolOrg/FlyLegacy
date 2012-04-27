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
#include "../Include/Cameras.h"
#include "../Include/Geomath.h"
//==========================================================================================
//	Triangle indices
//	Each index gives 2 informations about the vertex in the roof model
//	-Left part indicates the relative vertex position in the roof pan:
//		0->Lower part   (used for texture)
//		7->Top part			(used for texture)
//	-Right part is the vertex used by the triangle
//===========================================================================================
class C3DPart;
class OSM_Object;
class D2_Group;
class D2_Style;
class CRoofModel;
class	D2_TParam;
class CShared3DTex;
//------------------------------------------------
struct D2_BPM;
struct SQL_DB;
struct OSM_REP;
struct OSM_CONFP;
//==========================================================================================
#define TX_BH		(1)								// Full height texturing
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
//---------------------------------------------------
#define PAN_NONE			(0)
#define PAN_WALL_XP		(1)			// Wall X+
#define PAN_WALL_XM		(2)			// Wall X-
#define PAN_WALL_YP		(3)			// Wall Y+
#define PAN_WALL_YM		(4)			// WAll Y-
#define PAN_ROOF_XP		(5)			// ROOF X+
#define PAN_ROOF_XM		(6)			// ROOF X-
#define PAN_ROOF_YP		(7)			// ROOF Y+
#define PAN_ROOF_YM		(8)			// ROOF Y-
#define PAN_DORM_SW		(9)			// Dormer 
#define PAN_DORM_NE   (10)
#define PAN_DORM_XP		(11)
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
#define TEXD2_X_PLUS	(0)												// Texture for X+
#define	TEXD2_Y_PLUS	(1)												// Texture for Y+
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
//-------------------------------------------------------------
#define TEXD2_HMOD_WHOLE  (0)
#define TEXD2_HMOD_GFLOOR (1)
#define TEXD2_HMOD_MFLOOR (2)
#define TEXD2_HMOD_ZFLOOR (3)
//====================================================================================
//	Building parameter
//	
//====================================================================================
struct D2_BPM {
	//--- Localization parameters ------------------------
	SPosition			geop;										// Geo position
	U_INT		qgKey;												// QGT key
	U_SHORT	supNo;												// Super tile No
	U_SHORT	rfu1;													// Reserved
	//----------------------------------------------------
	OSM_Object   *obj;										// Related object
	//----------------------------------------------------
	double        flHtr;									// Floor height
	double				rdf;										// Reduction factor
	U_INT					ident;									// Object identity
	U_INT					stamp;									// Object number
	D2_Group		 *group;									// mother group
	D2_Style		 *style;									// Style
	CRoofModel   *roofM;									// Roof model
	D2_TParam    *roofP;									// Roof Texture
	//-----------------------------------------------------
	COption       opt;										// Options
	//-----------------------------------------------------
	char				  flNbr;									// floor number
	char					mans;										// Style mansart
	char					error;									// Error number
	char					selc;										// Selected
	//-----------------------------------------------------
	U_INT					side;										// Side number
	double        surf;										// Surface
	double        dlg;										// Longuest edge
	double				lgx;										// Width
	double				lgy;										// Lenght
	double				hgt;										// Height						
	//-----------------------------------------------------
public:
	D2_BPM() {Clear();}
~	D2_BPM()
	{}

	//-----------------------------------------------------
void Clear()
	{	mans	= 0;
		group	= 0;
		style	= 0;
		roofM	= 0;
		roofP	= 0;
		selc	= 0;
		flNbr = 0;
		geop.lat = geop.lon = geop.alt = 0;
		lgx = lgy = 0;
	};
};


//======================================================================================
//  2D point coordinates
//======================================================================================
struct D2_POINT {
	D2_POINT *next;															// Next point
	D2_POINT *prev;															// Previous
	//---------------------------------------------------------
	char   idn[4];															// Point ident
	//---------------------------------------------------------
  char   R;																		// Type of point
	char   F;																		// Type of face
	char   V;																		// Vertical position 
	char   rfu;																	// Reserved
	//----------------------------------------------------------
	U_INT  rng;																	// point order number
	//--- Original coordinates ---------------------------------
	double	dgx;																// Longitude in degre
	double	dgy;																// Latitude in degre
	//--- Space coordinates ------------------------------------
	double x;																		// X coordinate
  double y;																		// Y coordinate
	double z;																		// Z coordinate
	double a;																		// Ground altitude
	//--- Texture coordinate -----------------------------------
	double s;																		// along x
	double t;																		// along y
	//--- Local space -------------------------------------------
	double lx;																	// X coordinate
	double ly;																	// Y coordinate
	//-----------------------------------------------------------
	double elg;																	// Edge lenght
	//---- Methods ----------------------------------------------
	D2_POINT::D2_POINT() {x = y = z = s = t = 0; R= F = V = rng = 0; next = prev = 0; }
	//------------------------------------------------------------
	D2_POINT::D2_POINT(double x,double  y)
	 {	R	= F = V = 0;
			this->x = x;
			this->y = y;
			lx = ly = 0;
			z = s = t  = 0;
			next = prev = 0;
		}
	//-----------------------------------
	D2_POINT::D2_POINT(D2_POINT *p, char N)
	{	*this = *p;
	  *idn  =  N;
		 next	= 0;
		 prev	= 0;
	}
	//------------------------------------------
	void	Reset() {R = F = V = 0;}
	//------------------------------------------
	void	IncUser()		{;}
	//------------------------------------------
	void  Id(char *d)			{strncpy(d,idn,4); d[4] = 0;}
	void  SetID(char *d)	{strncpy(idn,d,4);}
	void  Stamp(char n)   {*idn = n;}
	//------------------------------------------
	void	D2_POINT::LocalShift(double tx,double ty);
	//------------------------------------------
	GN_VTAB *D2_POINT::SaveData(GN_VTAB *tab, CVector &N);
	//-----------------------------------------
	void	D2_POINT::Convex()			{R = 0;}
	void	D2_POINT::Reflex()			{R = 1;}
	void  D2_POINT::Type(char n)	{R = n;}
	bool  D2_POINT::IsReflex()	  {return (R == 1);}
	bool  D2_POINT::NotNulEdge()	{return (*idn != 'N');}
	//------------------------------------------
	char  D2_POINT::GetType()		{return *idn;}
	void	D2_POINT::SetEdge(char n)	{*idn = n;}
	//------------------------------------------
	void	D2_POINT::SetFace(char n)	{F	= n;}
	char  D2_POINT::GetFaceType() {return F;}
	//-------------------------------------------
	void  D2_POINT::Draw()
	{	glTexCoord2d(s,t);
		glVertex3d(x,y,z);
	}
	//-------------------------------------------
	void D2_POINT::SetST(double xs, double yt)
	{	s = xs; t = yt; }
	//------------------------------------------
	char FaceType()	{return F;}
	//-------------------------------------------
	char IsYFace()	{return (F & GEO_Y_FACE);}
	//--------------------------------------------
	char VertPos()	{return V;}
	//-------------------------------------------
	double	D2_POINT::LocalX() {	return lx;}
	double	D2_POINT::LocalY() {  return ly;}
};
//======================================================================================
//  2D triangles
//	Orientation is B->A->C->B
//======================================================================================
struct D2_TRIANGLE {
	char idn[4];
	char	type;
	D2_POINT *B;				// Previous point
	D2_POINT *A;				// current vertex
	D2_POINT *C;				// Next point
	CVector   N;				// Normal
	//----------------------------------------------
	D2_TRIANGLE::D2_TRIANGLE() { type = 0; A=B=C=0; strncpy(idn,"D2TR",4);}
	//---- Allocate D2_POINTS ----------------------
	D2_TRIANGLE::D2_TRIANGLE(char a)
	{	type = 1; A=B=C=0; strncpy(idn,"D2TR",4);}
	//----- Destruction ----------------------------
	D2_TRIANGLE::~D2_TRIANGLE()
		{}
	//----- Methods --------------------
	void	D2_TRIANGLE::Set(D2_POINT *P, D2_POINT *S, D2_POINT *N)
	 {	A = S; 
			B = P;
			C = N;
		}
	//------------------------------------------------------
	//U_INT	StoreVertices(C3DPart *p, U_INT n);
	U_INT StoreData(C3DPart *p, U_INT n);
};

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
	CVector PlanNorme(D2_POINT &A, D2_POINT &B, D2_POINT &C);
	//-------------------------------------------------------------
	inline void		SetPrecision(double p)	{precision = p;}
	inline double GetPrecision()					{return precision;}
};
//====================================================================================
//	Slot to a 2D_INDICE
//	Used for defining a roof model
//====================================================================================
struct D2_INDICE {
	char ind1;										// Indice to B
	char ind2;										// Indice to A
	char ind3;										// Indice to C
	//---Type of pan -------------------------------------------
	char pt;											// Pan type
	char bh;											// Full batiment height
};
//======================================================================================
//	Class object with generation controlled by ratio
//	The turn is given when the frequency (Number of Instances / Total instances) is
//	lower than the limit (ratio)
//======================================================================================
class D2_Ratio {
protected:
	//--- Attributes ------------------------------------------------
	U_INT	  quota;								// Local weight of this member
	U_INT		objNB;								// Object number in this class
	//--- Method -----------------------------------------------------
public:
	D2_Ratio()	
	{	quota = 1; objNB = 0;}
	//-----------------------------------------------------------
	void	SetQuota(U_INT q)	{quota = q;}
	//--------------------------------------------------------------
	bool  ReachQuota()
	{	objNB++;
		U_INT  rst = objNB % quota;
		return (rst)?(false):(true);
	}
};
//====================================================================================
//	Slot to a 2D_POINT
//	Used for triagulation of the polygon
//====================================================================================
struct D2_SLOT {
	char idn[4];
	D2_SLOT  *prev;
	D2_SLOT  *next;
	D2_POINT *vrtx;
	//-----------------------------------------------
	char     R;							// Reflex indicator
	//-----------------------------------------------
	D2_SLOT::D2_SLOT(D2_POINT *p)
	{	prev	= 0;	next	= 0;	vrtx	= p; R = p->R;	strncpy(idn,"D2SL",4);}
	//-----------------------------------------------
	D2_POINT *D2_SLOT::GetVRTX()	{return vrtx;}
	//-----------------------------------------------
	bool D2_SLOT::IsReflex()			{return (R != 0);}
	//------------------------------------------------
	void D2_SLOT::SetType(char t)	{R = t;}
	//--------------------------------------------------
	void Clear() { if (vrtx) delete vrtx;}
	};
//====================================================================================
class D2_Style;
class CRoofModel;
//====================================================================================
//	Class face for stair extrusion
//====================================================================================
class D2_FACE {
	// ATTRIBUTES ------------------------------------------------
public:
	char  idn[4];
	//--- Floor code ------------------------------------
	char	fType;										// Face type
	char	fIndx;										// Floor index
	//--- Corner definitions ----------------------------
	D2_POINT sw;										// SW corner
	D2_POINT se;										// SE corner
	D2_POINT ne;										// NE corner
	D2_POINT nw;										// NW corner
	CVector  N;											// Normal
	//--- public Methods -------------------------------
public:
	D2_FACE()	{strncpy(idn,"D2FA",4);}
	D2_FACE(char t, char n);
 ~D2_FACE();
	//--------------------------------------------------
	void D2_FACE::Copy(D2_POINT *A, D2_POINT *B, D2_POINT *C);
	//--------------------------------------------------
	void		Extrude(double f,double c,D2_POINT *sw,D2_POINT *se);
	void		SetNorme(GeoTest *geo);
	void		TextureFaceByPoint(D2_Style *s);
	void		TextureFaceByFaces(D2_Style *sty);
	void		MoveToSW();
	void		CRotation();
	//--------------------------------------------------
	void    BissectorBC(double dy);
	void		RotationMC(D2_POINT *p);
	//-------------------------------------------------
	U_INT		StoreVertices(C3DPart *prt, U_INT n);
	U_INT	  StoreData(C3DPart *prt, U_INT n);
	//--------------------------------------------------
	char		SwFaceType()							{ return sw.F;}

};
//====================================================================================
//	Class floor for floor extrusion
//====================================================================================
class D2_FLOOR {
	//--- ATTRIBUTES ------------------------------------------
public:
	char      idn[4];
	char			sNo;										// Stair number
	char      type;										// Floor type
	double		floor;									// Height
	double		ceil;
	//---------------------------------------------------------
	std::vector<D2_FACE*> faces;			// Polygon faces
	//--- METHODS ---------------------------------------------
public:
	D2_FLOOR() {strncpy(idn,"D2FL",4);}
	D2_FLOOR(char e, char t, double f, double c);
 ~D2_FLOOR();
 //----------------------------------------------------------
  void	TextureFloor(D2_Style *s);
	U_INT	StoreVertice(C3DPart *p,U_INT x);
	U_INT	StoreData(C3DPart *p,U_INT x);
	U_INT	VerticesNumber();
};
//====================================================================================
//	data for Texture definition
//====================================================================================
class D2_TParam: public D2_Ratio {
friend class D2_Style;
friend class Queue<D2_TParam>;
	//--- ATTRIBUTES -----------------------------------------------
	char idn[4];
	//---------------------------------------------------------------
	D2_TParam *next;
	D2_TParam *prev;
	//---------------------------------------------------------------
	D2_Style *sty;
	D2_BPM	 *bpm;												// Batiment parameters
  //--------------------------------------------------------------
	char face[4];													// Code face
	//--------------------------------------------------------------
	char		code;													// Code texture
	char		mans;													// Mansart
	char		hmod;													// Texture mode in vertical
	U_CHAR	rfnb;													// Roof number
	//--------------------------------------------------------------
	int		x0;															// SW corner
	int		y0;															// 
	//--- Texture dimension ----------------------------------------- 
	double  Tw;
	double  Th;
	//--- Rectangle extensions --------------------------------------
	double Rx;
	double Ry;
	//--- ARRAY of texture coordinates for X roofs ------------------
	double rofB;														// Roof base
	double rofH;														// Roof height
	double rofW;														// Roof Y width
	//--- METHODS --------------------------------------------------
public:
	D2_TParam();	
 ~D2_TParam();
  //--------------------------------------------------------------
	void		DefaultParameters(TEXT_INFO &inf);	
	void		AdjustFrom(D2_TParam &p, D2_Style *s, U_INT w, U_INT h);
	void		TextureSideRoof(D2_TRIANGLE &T);
	void		TextureRoofTriangle(D2_TRIANGLE &T);
	void		TextureDormerTriangle(D2_TRIANGLE &T, char cn);
	void		TextureFrontTriangle(D2_TRIANGLE &T);
	//--------------------------------------------------------------
	void		TextureLocalPoint(D2_POINT *p);
	void		TextureXRoofPoint(D2_POINT *p);
	void		TextureYRoofPoint(D2_POINT *p);
  void		TextureDRoofPoint(D2_POINT *p);
	void		TextureFRoofPoint(D2_POINT *p);
  //--------------------------------------------------------------
	void		SetStyle(D2_Style *st);
	//--------------------------------------------------------------
	bool  IsWhole()								{ return (hmod == 0);	}
	void  SetFace(char *t)				{ strncpy(face,t,4);	}
	void  SetRoofTexNumber(U_INT n)	{	rfnb = n;}
	U_INT GetRoofTexNumber()			{ return rfnb;}
	//--- roof base ------------------------------------------------
	void	SetRoofBase(double b, double H, double W)		{rofB = b; rofH = H; rofW = W;}
	//--------------------------------------------------------------
	double GetTextureWD()			{	return Tw; }
	double GetTextureHT()			{	return Th; }
	double GetRectangWD()			{	return Rx; }
	double GetRectangHT()			{ return Ry; }
};
//====================================================================================
//	data for Group of Style definition 
//====================================================================================
class D2_Group;
//------------------------------------------------------------------------------------
class D2_Style: public D2_Ratio, public CSlot {
	friend class Queue<D2_Style>;
	//--- ATTRIBUTES -----------------------------------------------
	D2_Style  *next;
	D2_Style  *prev;
	//--------------------------------------------------------------
	char       filn[64];									// Texture file name					
	D2_Group  *group;											// Parent group
	char			 tr;												// Trace
	char       mans;											// Style mansart on last floor
	char       texf;											// Texture by face
	//--- Texture size ---------------------------------------------
	U_INT			 Tw;												// Texture width
	U_INT			 Th;												// Texture height
	//--------------------------------------------------------------
	D2_BPM    *bpm;												// Building parameters
	//--------------------------------------------------------------
	double     cover;											// Texture coverage (feet)
	double     Wz;												// wall height
	//----- Queue for selector -------------------------------------
	Queue<D2_TParam> rtexQ;
	//--------------------------------------------------------------							
	D2_TParam *param[TEXD2_MAT_DIM];			// Texture array for face
	std::vector<D2_TParam *> roofT;				// Roof textures
	//-------------------------------------------------------------
	D2_TParam *dormer;										// Dormer texture
	D2_TParam *sText;											// Selected Roof texture
	//--- METHODS -------------------------------------------------
public:
	D2_Style(char *sn, D2_Group *gp);
 ~D2_Style();
  void			Clear(D2_TParam *p);
  //-------------------------------------------------------------
	bool			Error1 (char *s);
	bool			Error2 (char *s);
	bool			Error3 (D2_TParam *p);
	bool			DecodeStyle(char *buf);
	bool			AddFace(D2_TParam &p);
	bool			AddFloor(char *fl, D2_TParam &p);
	bool			AddRoof(D2_TParam &p);
	bool			AddDormer(D2_TParam &p);
	//-------------------------------------------------------------
	char      TextureMode();
	double    TextureCover()	{return cover;}
	//-------------------------------------------------------------
	void			AddGroundFXP(D2_TParam &p);
	//-------------------------------------------------------------
	void					SelectRoofTexture();
	D2_TParam		 *GetRoofTexture();
	D2_TParam    *SelectRoofNum(U_INT rfno);
	//-------------------------------------------------------------
	void      TexturePoint(D2_POINT *p, char tp, char fx,char hb = 0);			// Compute x texture coordinate
	void			TextureByFace(D2_POINT *pp, char ft, char fx, char hb);
	void			TextureSideWall(D2_TRIANGLE &T, char ft, char hb);
	//-------------------------------------------------------------
	bool			IsOK();
	bool			IsMansart()					{return (mans != 0);}
	D2_Style *GetNext()						{return next;}
	char      GetMansart()				{return mans;}
	//-------------------------------------------------------------
	D2_BPM   *GetBPM()						{return bpm;}
	char      HasTrace()					{return tr;}
	bool			SameName(char *n)		{return (strcmp(Name,n) == 0);}
	//------ Save building extensions ------------------------------
	void			SetWz(double r)			{Wz = r;}
	void			AssignBPM(D2_BPM *p){bpm = p;}
	//--------------------------------------------------------------
	D2_Group  *GetGroup()					{return group; }
	D2_TParam *GetDormer()				{return dormer;}
};
//====================================================================================
//	Group options 
//====================================================================================
#define D2_GRP_SURF				(0x0001)				// Has surface
#define D2_GRP_SIDE				(0x0002)				// Has sides
#define D2_GRP_GEOP				(0x0004)				// Has geo position
#define D2_GRP_LNGT				(0x0008)				// Has length filter
//====================================================================================
//	data for group definition 
//====================================================================================
class D2_Session;
//------------------------------------------------------------------------
class D2_Group: public  D2_Ratio {
	friend class Queue<D2_Group>;
	//--- ATTRIBUTES ----------------------------------------------
	D2_Group   *next;
	D2_Group	 *prev;
	//--------------------------------------------------------------
	char  name[64];													// Group name
	D2_Session *ssn;												// Parent session
	//--- filter indicators ---------------------------------------
	COption  opt;														// filters
	//--- filters -------------------------------------------------
	double  lgMin;													// X length
	double  lgMax;
	double	sfMin;													// min surface
	double  sfMax;													// Max surface
	U_INT		sdMin;													// side number
	U_INT		sdMax;													// side max
	SPosition geop;													// geo center
	double   radius;												// Radius
	//--- Trace indicator -----------------------------------------
	char		tr;															// Trace
	char    mans;														// Current style is mansard
	char 		rmno;														// Selected roof model
	char		rfu1;														// reserved value
	//--- Renforced notation --------------------------------------
	char    rlgr;														// Renforced lenght
	char    rsuf;														// Renforced surface
	char    rsid;														// Renforced side
	char		rfu2;
	//--- TAG-VALUE -----------------------------------------------
	char		pTag[64];												// Tag parameter
	char    pVal[64];												// Value parameter
	//--- Texture parameters --------------------------------------
	TEXT_INFO txd;													// Texture definition
	CShared3DTex *tREF;											// Texture reference
	//---- Turn index ---------------------------------------------
	short    indx;													// Turn index
	//---- Floor parameters ---------------------------------------
	short    flNbr;													// Number of floors
	short    flMin;													// Minimum floors
	short    flMax;													// Floor maxi
	double   flHtr;													// Floor height
	//--- Number of generated objects in this group ----------------
	U_INT    nItem;													// Instance number
	//--- Default roof texture -------------------------------------
	D2_TParam tRoof;												// Texture roof
	//--- Building list --------------------------------------------
	std::map<U_INT,OSM_Object*>building;		// List of building in group
	//--- Queues for selector --------------------------------------
	Queue<D2_Style>			styleQ;
	Queue<CRoofModel>   roofsQ;
	//--------------------------------------------------------------
	std::vector<CRoofModel*>  roofM;				// Roof models to use
	//--- METHODS --------------------------------------------------
public:
	D2_Group(char *gn, D2_Session *s);
 ~D2_Group();
  //--------------------------------------------------------------
	bool		Parse(FILE *f, D2_Session *sn);
	bool		DecodeParam(char *buf);
	//--------------------------------------------------------------
	void		LoadTexture();
	//--------------------------------------------------------------
	bool		AddRoof(char *name, double a, int w);
	void		AddStyle(D2_Style *sy);		
	//--------------------------------------------------------------
	void    GetTexDim(U_INT &x,U_INT &y) {x = txd.wd; y = txd.ht;}
	//--------------------------------------------------------------
	D2_Style    *GetStyle(char *nsty);
	D2_Style    *GetOneStyle();
	U_INT        SelectOneRoof(D2_BPM *pm);
	CRoofModel  *GetRoofModByNumber(char mans);
	U_INT				 GetRoofNumber()	{return rmno;}
	//---------------------------------------------------------------
	int						ValueGroup(D2_BPM *pm);
	int						ValueTag(char *T, char *V);
	int						ValuePosition(SPosition &p);
	int						ValueSurface(double sf);
	int						ValueSide(U_INT sd);
	int						ValueLength(double lg);
	int						GenFloorNbr();
	//--------------------------------------------------------------
	void				 AddBuilding(OSM_Object *b);
	void				 RemBuilding(OSM_Object *b);
	OSM_Object *FindBuilding(U_INT No);
	//--- Draw all buildings ---------------------------------------
	void			DrawBuilding();
	//--------------------------------------------------------------
	void			SetTexName(char *n)	{strncpy(txd.name,n,64);}
	//--------------------------------------------------------------
	D2_Group *Next()							{return next;}								
	void			SetMansar(char m)		{mans = m;	}
	//--------------------------------------------------------------
	void			SetRoofModelNumber(U_INT n)		{rmno		= n;}
	//--------------------------------------------------------------
	U_INT			GetFloorNbr()				{ return	flNbr;}
	double    GetFloorHtr()				{ return flHtr;}
	void			SetFloorNbr(U_INT n){ flNbr = n;}
	//--------------------------------------------------------------
	CShared3DTex *GetTREF()				{return tREF;}
	//--------------------------------------------------------------
	char			HasTrace()					{return tr;}
	//--------------------------------------------------------------
	D2_TParam  *GetRoofTexDefault(){return &tRoof;}
	D2_Session *Session()					{return ssn;}
	//--------------------------------------------------------------
	char     *GetName()						{return name;}
	char     *TextureName()				{return txd.name;}
	//--------------------------------------------------------------
	std::vector<CRoofModel*>		&GetRoofList()		{return roofM;}
	Queue<D2_Style> *GetStyleQ()	{return &styleQ;}
  //--------------------------------------------------------------
	};

//====================================================================================
//	Structure for bevel parameters
//====================================================================================
struct D2_BEVEL {
	double      tang;															// Slope tangent
	double      pah;															// Point absolute height
	double			H;																// floor height
};
//====================================================================================
//
//====================================================================================

//====================================================================================
//	TRIANGULATOR for triangulation of polygones
//====================================================================================
class CBuilder: public CExecutable, public Tracker {
	//--- Callback vectors ----------------------------------------
public:
	typedef void (CBuilder::*buildCB)(U_INT);		// Build vector
protected:
	//--- ATTRIBUTES ----------------------------------------------
	COption		dop;																// Drawing option
	GeoTest	  geo;																// Geo tester
	D2_Session *session;													// Current session
	//--------------------------------------------------------------
	Queue <D2_POINT> extp;												// External contour
	Queue <D2_POINT> hole;												// Hole contour
	Queue <D2_SLOT>  slot;												// Slot list
	std::vector<D2_FLOOR*>    walls;							// Walls
	std::vector<D2_TRIANGLE*> roof;								// Data
	//--- Spot to get terrain elevation----------------------------
	GroundSpot  spot;
	//--- Roof points ----------------------------------------------
	D2_POINT   *bevel;														// Bevel array
	double			t70;															// tan(60°)
	//--- Transformation matix -------------------------------------
	double      mat[9];														// Global matrix
	double			gx;																// Working
	double			gy;																// Working
	//--- Rotation parameters --------------------------------------
	double			rx;																// x result
	double      ry;																// y result
	//--------------------------------------------------------------
	D2_TRIANGLE tri;															// Internal triangle
	D2_TRIANGLE qtr;															// Qualify triangle
	//--------------------------------------------------------------
	D2_POINT   *TO;																// Texture origin
	double      dlg;															// Larger diagonal
	//---Translation parameters ------------------------------------
	double      minx;															// Mini x
	double      miny;															// mini y
	double      maxx;															// Max  x
	double      maxy;															// Max  y
	//-------------------------------------------------------------
	double      Xp;																// building X extend
	double		  Yp;																// building Y extent
	double      Zp;																// Building Z extend
	//-------------------------------------------------------------
	double      Wz;																// Wall height
	//-------------------------------------------------------------
	double	surf;																	// Ground surface
	//----------------------------------------------------
	double sinA, cosA;														// Rotation angle
	double  rotM[4];															// Rotation matrix
	//----------------------------------------------------
	char    trace;																// Trace indicator
	char    vRFX;																	// Reflex indicator
	char    hIND;																	// Hole indicator
	char    rfu;																	// reserved
	//----------------------------------------------------
	U_INT   num;																	// Null number
	U_INT		seq;																	// Sequence number
	//--- Current building  ------------------------------
	D2_BPM  BDP;																	// Building parameters
	OSM_Object *osmB;															// Current building
	OSM_Object *remB;															// Removed building
	U_INT   xOBJ;																	// Current texture
	//---  Texture descriptor ----------------------------
	TEXT_INFO txd;																// Texture desc
	//--- Drawing mode -----------------------------------
	char		dMOD;																	// Drawing mode													
	//--- METHODS ----------------------------------------
public:
	CBuilder(D2_Session *s);
 ~CBuilder();
	//-----------------------------------------------------
  GeoTest  *GetGeotester()	{return &geo;}
	//-----------------------------------------------------
  void			CheckAll();
	void			ClearRoof();
	D2_POINT *AllocateBevel(int nb);
	void			TranslatePoint(D2_POINT &p, CVector &T);
	void			LocalCoordinates(D2_POINT &p);
	//----------------------------------------------------
	void    	GetBevelVector(D2_POINT *pa, double dy,D2_POINT *dst);
	int				SetBevelArray(D2_BEVEL &pm);
  //-----------------------------------------------------
	void		AddVertex(double x, double y);
	void		NewHole();
	void		ForceStyle(char *nsty,U_INT rfmo, U_INT rftx, U_INT flnb);
	char		ConvertInFeet();
	int	    QualifyPoints(D2_BPM *bmp);
	char		Triangulation();
	void		MakeSlot();
	void		ReleaseSlot();
	void		OrientFaces(D2_BPM *bpm);
	void		BuildWalls(D2_BPM *bpm);
	void		Reorder();
	void		SelectRoof(D2_BPM *bpm);
	void		Texturing(D2_BPM *bpm);
	//---------------------------------------------------
	void		Draw();					// Drawing interface
	void		DrawMarks();
	//--- Building management ----------------------------
	U_INT		CountVertices();
	void		StartOBJ();
	U_CHAR  ReplaceOBJ(OSM_REP *rp,char or);
	void    RemoveOBJ();
	void    RestoreOBJ();
	U_CHAR	RotateOBJ(double rad);
	bool    SelectOBJ(U_INT No);
	void		SetOrientationOBJ();
	//-----------------------------------------------------
	inline double			GetSurface()			{return surf;}
	inline U_INT			GetSideNbr()			{return extp.GetNbObj();}
	inline SPosition	GetPosition()			{return BDP.geop;}
	//-----------------------------------------------------
protected:
  //-----------------------------------------------------
	void		Clean();
	//-----------------------------------------------------
	void		NegativeROT(double sn, double cn);
	void		PositiveROT(double sn, double cn);
	void		Rotate(double x, double y);
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
	bool		SetPointInside(D2_POINT *p, D2_POINT *s, double H);
	void		QualifyEdge(D2_POINT *pa);
	void		BuildFloor(int no,double f, D2_BPM *bpm);
	int			BuildBevelFloor(int No, int inx, double afh, D2_BPM *bpm);
	int 		BuildNormaFloor(int No, int inx, double afh, D2_BPM *bpm);
	//------------------------------------------------------
	void		TraceOut();
	void		TraceInp();
	void		TraceFace();
	void		TraceBevel(D2_POINT *p);
	//--- Drawing routines ---------------------------------
	int 		LineMode();					// Drawing interface
	int 		FillMode();
	//--- TimeSlice must return 0 to inhibit further processing --
	int			TimeSlice(float dT,U_INT frame);
	//--- Option  management -------------------------------
public:
	void		repD(U_INT p)							{dop.Rep(p);}
	char    hasR(U_INT p)							{return dop.Has(p);}
	D2_Style  *GetStyle()							{return BDP.style;}
	void		SetIdent(U_INT id)				{BDP.ident = id;}
	//--- Drawing interface --------------------------------
	void		ModeSingle();
	void		ModeGroups();
	//--- OSM objects --------------------------------------
	void    MakeBLDG(U_INT type);
	void    MakeLITE(U_INT type);
	//------------------------------------------------------
	bool		RiseBuilding(D2_BPM *bpm);
	void		SaveBuildingData(C3DPart *P);
	void		EndOBJ();
	//void		SaveOSMParam(D2_BPM *bpm);
	//--- Helper -------------------------------------------
	void			EditTag(char *txt);
	int 			EditPrm(char *txt);
	void			UpdatePosition(SPosition &P);
	//------------------------------------------------------
	void			ClearError()			{BDP.error = 0;}
	void			ActualPosition(SPosition &P);
	U_INT     ActualStamp();
	D2_Style *ActualStyle();
	int       ActualError();
	char      ActualFocus();
	//------------------------------------------------------
	U_CHAR	ModifyStyle(D2_Style *sty);
	int     ReOrientation(D2_BPM *bpm);
	D2_BPM *GetBuildingParameters()			{return &BDP;}
	//--- Call back functions --------------------------------
	void    BuildOBJ(OSM_CONFP *CF);	
	//--------------------------------------------------------
};

//====================================================================================
//	data for session 
//====================================================================================
class D2_Session {
	//--- ATTRIBUTES ------------------------------------------------
	char  tr;																	// Trace indicator
	char  name[64];														// Session name
	//--- Street lights queue ---------------------------------------
	std::map<U_INT,OSM_Object*> litQ;					// Light queue
	std::map<U_INT,OSM_Object*>::iterator rl;	
	//--- Groups parameters -----------------------------------------
	Queue<D2_Group> groupQ;										// Group queue for selection
	std::map<std::string,D2_Group*> grpQ;			// Groups
	std::map<std::string,D2_Group*>::iterator rg;
	//--- Replacement MAP -------------------------------------------
	std::multimap<U_INT,OSM_REP*> repQ;				// Replacement list
  std::multimap<U_INT,OSM_REP*>::iterator rp;
	//---------------------------------------------------------------
	U_INT			fpos;														// File position
	CBuilder *trn;														// CBuilder
	char			buf[256];												// Read buffer
	//--- Default roof ----------------------------------------------
	CRoofModel *roof;
	D2_TParam  *rtex;
	//--- Number of instances in group ------------------------------
	U_INT	gpn;															// Starting group number
	U_INT	Stamp;														// Number of generated objects
	//---------------------------------------------------------------
	D2_Group   *grp;												// Selected group
	D2_Style   *sty;												// Selected style
	//---METHODS ----------------------------------------------------
public:
	D2_Session();
 ~D2_Session();
	//----Session decoding ------------------------------------------
  void	Abort(char *msg);
	bool	Stop01(char *ln);
	bool	ParseTheSession(FILE *f);
	bool	ReadParameters(char *dir);
	bool	ParseReplace(FILE *f, char *ln);
	bool	ParseGroups(FILE *f, char *ln);
	bool  ParseStyles(FILE *f, char *ln);
	//--- return a replacing object ---------------------------------
	bool  GetReplacement(OSM_REP &rpm);
	//--- Draw everything -------------------------------------------
	void  Draw();
	//---------------------------------------------------------------
	void	Write(FILE *fp);
	//---------------------------------------------------------------
	void         GetaStyle(D2_BPM *p);
	OSM_Object  *GetObjectOSM(U_INT k);
	D2_Style    *GetStyle(char *nsty);
	D2_TParam   *GetRoofTexture(D2_Style *sty);
	//---------------------------------------------------------------
	void		FillStyles(CListBox *box);
	void		UpdateCache();
	//---------------------------------------------------------------
	bool		AddStyle(FILE *f,char *sn,char *gn);
	void		AddLight(OSM_Object *L);
	//---------------------------------------------------------------
	inline void SetTrace(char t)	{tr = t;}
	inline void BackTo(int p)			{fpos = p;}
	inline char HasTrace()				{return tr;}
	inline void SetTRN(CBuilder *t)	{trn = t;}
	//---------------------------------------------------------------
	bool				IsEmpty()					{ return (1 == Stamp);}
	//---------------------------------------------------------------
	CRoofModel *GetDefaultRoof()					{return roof;}
	D2_TParam  *GetDefaultRoofTexture() 	{return rtex;}
	U_INT       GetStamp()								{return Stamp;}
	U_INT				GetNextStamp()						{return Stamp++;}
	U_INT				GetNbObject()							{return (Stamp-1);}
 };

//============================= END OF FILE ======================================================
#endif // TRIANGULATOR_H
