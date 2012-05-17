//==============================================================================================
// TerrainData.h
//
// Part of Fly! Legacy project
//
// Copyright 2003 Chris Wallace
//
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
#ifndef TERRAIN_DATA_H
#define TERRAIN_DATA_H
//================================================================================================
#include "../Include/FlyLegacy.h"
//================================================================================================
class CmQUAD;
class C_QGT;
class C_Vertex;
//================================================================================================
//	Structures data used for terrain
//================================================================================================
//===========================================================================
//  STRUCTURE TO ACCES CORNER VERTICES
//===========================================================================
typedef struct {
    short   dx;                             // QGT horizontal increment
    short   dz;                             // QGT vertical increment
    short   cn;                             // Corner index
  } QGT_DIR;
//-----------------------------------------------------------------------------
//  Texture indices
//-----------------------------------------------------------------------------
struct TC_ST_IND  {
  U_CHAR cs;
  U_CHAR ct;
};
//-----------------------------------------------------------------------------
//  Texture Coordinates
//-----------------------------------------------------------------------------
struct TC_ST_FL {
      GLfloat cs;
      GLfloat ct;
  };
//============================================================================
//  Increment Table format
//============================================================================
struct TC_INCREMENT {
    short dx;                              // cx increment
    short dz;                              // cz increment
  };

//============================================================================
//  Indices format
//============================================================================
struct TC_INDEX {
  float cx;
  float cz;
};
//============================================================================
//  COORDINATE definition
//============================================================================
struct TC_ST_COORD {
  float cx;
  float cz;
};
//============================================================================
//  WORLD COORDINATE definition (in feet)
//============================================================================
struct TC_WORLD {
  float wx;                           // Longitude
  float wy;                           // Latitude
  float wz;                           // Altitude
};
//============================================================================
//  STRUCTURE TO ACCESS SUPER TILE BORDERS
//============================================================================
struct TC_STBORD {
  U_SHORT cx;                         // DX
  U_SHORT cz;                         // DZ
  U_INT  msk;                         // CORNER INDICATOR
  U_INT  east;                        // EAST MARKER
};
//=============================================================================
//  Structure for terrain editor
//=============================================================================
#define VRT_MODULO (1023)
struct TVertex{
	U_INT			key;													// Vertex (X,Z) indice
	U_INT			vnum;													// Vertex number
	CmQUAD   *quad;													// Detail tile
	CVertex  *vrt;													// Vertex
	//--- contructor -------------------------------------
	TVertex()
	{ key		= 0;
		vnum	= 0;
		quad	= 0;
		vrt		= 0;
	}
	//----------------------------------------------------
	TVertex(CmQUAD *q)
	{	key		= 0;
		vnum	= 0;
		quad	= q;
		vrt		= 0;
	}
	//----------------------------------------------------
	void	SetQuad(CmQUAD *q)	{quad = q;}
	short	Row()	{ return (key & VRT_MODULO);}
	short Cln()	{ return (key >> 16) & VRT_MODULO;}
	//----------------------------------------------------
} ;
//=============================================================================
//  Structure for Elevation Tracker
//=============================================================================
struct TRACK_EDIT {
	U_INT	qKey;														// QGT Key
	U_INT	detNo;													// Detail tile Number in QGT
	CmQUAD *tile;													// Detail Tile
	char	rfu;														// reserved
	char	subq;														// subdivision levl
	U_INT		vNum;													// number of vertices
	TVertex  lvx[TC_MAX_ELV_DIM];					// List of vertices
	//------------------------------------------------------------
	TRACK_EDIT::TRACK_EDIT()
	{	subq	= 0;
		vNum	= 0;
	}
	//--- Free lists ---------------------------------------------
	void	TRACK_EDIT::Free()
	{	vNum	= 0;									// number of vertices
		return;
	}
	//--- Destructor ---------------------------------------------
	TRACK_EDIT::~TRACK_EDIT()
	{	Free();	}
	//------------------------------------------------------------
};

//=============================================================================
//	Structure for Detail Tile in QGT
//=============================================================================
struct QUAD_QGT {
	U_INT		 key;				// SuperTile Key
	C_QGT		*qgt;				// QGT
	CmQUAD	*quad;			// Detail Tile
	//--- constructor ------------------------------------------
	QUAD_QGT::QUAD_QGT()
	{	qgt		= 0;
		quad	= 0;
	}
	//----------------------------------------------------------
};
//=============================================================================
//  COLOR TABLE
//=============================================================================
struct TC_COLOR {
  GLfloat Red;
  GLfloat Green;
  GLfloat Blue;
  GLfloat alpha;
};
//============================================================================
//  HDTL CLASSE for DETAIL TILE 
//	NOTE: IncArray is only used for import elevation. TODO check if
//				delete is OK
//============================================================================
class TRN_HDTL {
	friend class CQ_HDTL;
    TRN_HDTL   *Next;               // Next Structure
		//------------------------------------------------------------------
		U_CHAR			org;								// Origin (POD or SQL)
    U_SHORT     tx;                 // Detail Tile x relative coordinate
    U_SHORT     tz;                 // Detail Tile z relative coordinate
		U_SHORT     st;									// Super tile number
    U_SHORT     aDim;               // Array dimension
    U_SHORT     aRes;               // Array resolution
    int        *elev;               // Elevation array
public:
		TRN_HDTL();
		TRN_HDTL(int r,char src);
	 ~TRN_HDTL();
		void					SetTile(U_INT n);
		void					SetElevation(C_QGT *qgt);
		void					SetTile(U_SHORT x,U_SHORT z);
		//----------------------------------------------------------------------
	  inline void		SetArray(int *a)		{elev = a;}
		inline void		SetSup(U_SHORT n)		{st		= n;}
		inline void		SetDIM(int d)				{aDim = d;}
		inline void   SetRES(int r)				{aRes = r;}
		inline void   IncArray(int d)			{elev += d;}
		inline double Elevation(int m)		{return double(elev[m]);}
		inline void		UpdTile(U_SHORT bx,U_SHORT bz)	{tx |= bx; tz |= bz;}
		//----------------------------------------------------------------------
		inline int    GetTileX()					{return tx;}
		inline int		GetTileZ()					{return tz;}
		inline int    GetRes()						{return aRes;}
		inline int		GetDim()						{return aDim;}
		inline U_INT	GetSupNo()					{return st;}
		inline U_INT  GetTile()						{return (tx << 16) | tz;}
		inline int		GetArrayDim()				{return (aDim * aDim);}
		inline int	 *GetElvArray()				{return elev;}
		inline U_SHORT	GetLevel()				{return (aRes >> 2);}
		//----------------------------------------------------------------------
		inline U_CHAR	GetORG()						{return org;}
};
//============================================================================
//  Class CVertex
//  This class is the base class to build terrain mesh
//============================================================================
#define VTX_DYNAM (0x01)
//----------------------------------------------------
class CVertex {
  friend class TCacheMGR;
  friend class C_QGT;
  friend class CmQUAD;
  //--- Absolute Vertex coordinates ------------------
  U_INT			xKey;										// X coordinate in world grid
  U_INT			zKey;										// Z coordinate in world grid
  U_CHAR		Fixe;										// Fixed
  U_CHAR		Use;										// Quad user count
  U_CHAR		nElev;									// Number of elevations
  U_CHAR		gType;									// Ground type when vertex is center tile
	//--- Texture indices -------------------------------
	U_CHAR		indS;										// S Texture indice
	U_CHAR		indT;										// T Texture indice
	U_CHAR		inES;										// S border  indice
	U_CHAR		inNT;										// N border  indice
  //--- Total elevation -------------------------------
  double    Ground;
  //--- Edge/Corner pointers --------------------------
  CVertex *Edge[4];               // 4 Adjacent vertices
  //--- Relatives coordinates in QGT ------------------
  double	rx;
	double  ry;
	double  wdz;
  //------------Methods ---------------------------------
public:
  CVertex(U_INT xk, U_INT zk);
  CVertex();
 ~CVertex();
  //----- Relative vector --------------------------------
  CVector RelativeFrom(CVertex &a);
  bool    IsAbove(double y);
  bool    ToRight(double x);
	bool    AreWe(U_INT ax,U_INT az);
  //------Border vertices ----------- --------------------
  CVertex *VertexNB()  {return Edge[TC_NORTH];}
  CVertex *VertexSB()  {return Edge[TC_SOUTH];}
  CVertex *VertexEB()  {return Edge[TC_EAST];}
  CVertex *VertexWB()  {return Edge[TC_WEST];}
  //------Corner vertices from center --------------------
  CVertex *VertexSW()   {return Edge[TC_SWCORNER];}
  CVertex *VertexSE()   {return Edge[TC_SECORNER];}
  CVertex *VertexNE()   {return Edge[TC_NECORNER];}
  CVertex *VertexNW()   {return Edge[TC_NWCORNER];}
  //------------------------------------------------------
  bool    IsSWcorner(CVertex *vt) {return (vt == Edge[TC_SWCORNER]);}
  bool    IsSEcorner(CVertex *vt) {return (vt == Edge[TC_SECORNER]);}
  bool    IsNEcorner(CVertex *vt) {return (vt == Edge[TC_NECORNER]);}
  bool    IsNWcorner(CVertex *vt) {return (vt == Edge[TC_NWCORNER]);}
  //------------------------------------------------------
  void    Init(U_INT vx,U_INT vz);
  //------------------------------------------------------
  void    CopyEdge(CVertex *vt);
  //-------------------------------------------------------
	SVector GeoCoordinates(C_QGT *qgt);
	//-------------------------------------------------------
  void    InsertNorth(CVertex *vn);
  void    InsertEast(CVertex *vn);
  void    EastLink(CVertex *v2);
  void    NorthLink(CVertex *v2);
  //------------in line ----------------------------------------------
  inline void IncUse()                {Use++;}
  inline void DecUse()                {Use--;}
  inline void ClearUse()              {Use = 0;}
  inline void SetFullQuad()           {Use = 8;}
  inline bool IsFull()                {return (8 == Use);}
  inline bool NoMoreUsed()  {Use--;    return (0 == Use);}
  inline bool IsUsed()                {return (0 != Use);}
	//-----XBAND is QGT index divided by 64 and multiplied by 8 --------
	inline U_INT  keyX()								{return xKey;}
	inline U_INT  keyZ()								{return zKey;}
	//---- Check if there is subdivision in index -----------------------
	inline bool HasSubdivision(U_INT k) { return (FN_SUB_FROM_INDX(k) != 0);}
	//------------------------------------------------------------------
	inline void   SetCornerHeight()							{SetWZ(Ground / nElev);}
  inline void   SetEdge(U_CHAR e,CVertex *v)  {Edge[e] = v;}
  inline void   SetCorner(U_CHAR c,CVertex *v){Edge[c] = v; if (v) v->IncUse();}
	//--- For debug ---------------------------------------------------
	inline bool   Is(U_INT x,U_INT z)	{return (xKey == x) && (zKey == z);}
	//----Make a detail key only---------------------------------------
	inline U_INT  VertexKey() 
	{ return ((xKey & TC_DTSUBMOD) << 16) | (zKey & TC_DTSUBMOD);}
	inline short xCln()								{return (xKey & TC_1024MOD);}
	inline short zRow()								{return (zKey & TC_1024MOD);}
  //---------------------------------------------------
  inline double GetAbsoluteLongitude()  {return FN_ARCS_FROM_SUB(xKey);}
  //---------------------------------------------------
  inline CVertex *GetEdge(U_SHORT cnr)  {return Edge[cnr];}
  inline CVertex *GetCorner(U_SHORT cnr){return Edge[cnr];}
	//--- Compute texture indices -----------------------
	inline U_LONG	SCoord(char sh)				{return (xKey & TC_1024MOD) >> sh;}
	inline U_LONG TCoord(char sh)				{return (zKey & TC_1024MOD) >> sh;}
	//--- Set S Coordinate ------------------------------
	inline U_INT S_Coord(char sh)
	{	indS	= (xKey & TC_1024MOD) >> sh;
		return indS;		}
	//--- Set T Coordinate ------------------------------
	inline U_INT T_Coord(char sh)
	{	indT	=	(zKey & TC_1024MOD) >> sh;
		return indT;	}
	//--- Set East border S coordinate ------------------
	inline U_INT ES_Coord(char sh,char fx)
	{	int n = (xKey & TC_1024MOD) >> sh;
		inES	= (n)?(n):(fx);
		return inES;	}
	//--- Set North border T coordinate -----------------
	inline U_INT NT_Coord(char sh,char fx)
	{	int n = (zKey & TC_1024MOD) >> sh;
		inNT	= (n)?(n):(fx);	
		return inNT;	}
	//----Compute texture index on East Side-------------  
	inline U_LONG ESCoord(char sh,char fx)
		{	int n = (xKey & TC_1024MOD) >> sh;
			return (n == 0)?(fx):(n);	}
	//----Compute texture index on North Side -----------
	inline U_LONG NTCoord(char sh,char fx)
		{	int n = (zKey & TC_1024MOD) >> sh;
			return (n == 0)?(fx):(n);	}
	//--- Copy World coordinates ------------------------
	inline void CopyCOORD(CVertex &v)
		{	this->rx = v.rx;
			this->ry = v.ry;
			SetWZ(v.wdz);
		}
	//--- Check for same QGT in X direction --------------
  bool SameXQGT(U_INT x)
	{	U_INT cmp = (xKey ^ x) & (TC_QGTMASK);
		return (cmp == 0);	}
	//--- Check for same X-QGT as vertex V -----------------
	bool SameXQGT(CVertex &v)
	{	U_INT cmp = (xKey ^ v.keyX())	& (TC_QGTMASK);
		return (cmp == 0);	}
	//--- Check for same QGT in Z direction --------------
	bool SameZQGT(U_INT z)
	{	U_INT cmp = (zKey ^ z) & (TC_QGTMASK);
		return (cmp == 0);	}
	//--- Check for same Z QGT as vertex v --------------
	bool SameZQGT(CVertex &v)
	{	U_INT cmp = (zKey ^ v.keyZ()) & (TC_QGTMASK);
		return (cmp == 0);	}
	//---------------------------------------------------
	//--- Return relative coordinates -------------------
	double GetRX()					{return rx;}
	double GetRY()					{return ry;}
	double GetRZ()					{return wdz;}
	//--- Relatives coordinate for NE corner -----------
	double GetTX()					{return (rx > 0 )?(rx):(TC_ARCS_PER_QGT);}
	double GetTY(double dl)	{return (ry > 0 )?(ry):(dl);}
	//--- Return altitude ------------------------------
	double GetWZ()		{return wdz;}
	//--- Set relative coordinates ---------------------
	void	SetWZ(double a)	
	{	wdz = a;	}
 };
//======================= END OF FILE ==============================================================
#endif // TERRAIN_DATA_H
