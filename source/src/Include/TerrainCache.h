/*
 * TerrainCache.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright  2003 Chris Wallace
 *            2008 Jean Sabatier
 *
 * Fly! Legacy is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 * Fly! Legacy is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 *   along with Fly! Legacy; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef TERRAINCACHE_H
#define TERRAINCACHE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//=============================================================================
// THE FOLLOWING PARAMETER DEFINE the MAXIMUM RESOLUTION OF THE TERRAIN CACHE
//	in term of number of QUADs in a DETAIL TILE
//	Level 0=>  1 QUAD					ARRAY of   5 elevations
//	Level 1=>  4 QUADs				ARRAY of  25 (5 * 5) elevations
//	Level 2=> 16 QUADs				ARRAY of  81 (9 * 9) elevations
//	Level 3=> 64 QUADs				ARRAY of 289 (17 * 17) elevations
//	Level k=>  4 Power(k)
//	For each QUAD there are 10 vertices to render in a TRIANGLE_FAN mode
//=============================================================================
#define QUAD_RESOLUTION (16)
//=============================================================================
#include "../Include/FlyLegacy.h"
#include "../Include/Model3D.h"
#include "../Include/FileThread.h"
#include "../Include/TerrainUnits.h"
#include "../Include/GeoMath.h"
#include "../Include/3dMath.h"
#include "../Include/BlackBox.h"
#include <map>
//=============================================================================
class CTerraFile;
class CMemCount;
class CWater3D;
class CVertex;
class CFuiTED;
struct ELV_VTX;
struct ELV_PATCHE;
//=============================================================================
//  MASK
//=============================================================================
#define TC_ONLY_RGB (0x00FFFFFF)
#define TC_ONLY_ALF (0xFF000000)
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
//============================================================================
//  Texture Descriptor
//============================================================================
#define TEX_NAME_DIM (8)
class CTextureDef {
public:
  CmQUAD  *quad;                            // Quad pointer
  U_CHAR   xFlag;                           // Flag indicator
  U_CHAR   Tmask;                           // Transition mask
  U_CHAR   TypTX;                           // Texture Type
  U_CHAR   aRes;                            // Alternate resolution
  //----------------------------------------------------------
  U_CHAR   Reso[2];                         // Resolution
  //----------------------------------------------------------
  U_INT    Key;                             // Shared key
  GLuint   dOBJ;                            // Day texture Object
  GLuint   nOBJ;                            // Nigth texture Object
  GLubyte *dTEX[2];                         // Day texture data
  GLubyte *nTEX[2];                         // Night texture data
  //---------------------------------------------------------
  char    *coast;                           // Coast Data
  char     Name[TC_TEXNAMESIZE];            // Name in ASCII
  char     Hexa[4];                         // Name in Hexa
  //----Constructor -----------------------------------------
  CTextureDef();
 ~CTextureDef();
  //---------------------------------------------------------
  void    SetDayTexture(U_CHAR n,GLubyte *t);
  void    SetNitTexture(U_CHAR n,GLubyte *t);
  //---------------------------------------------------------
  void    AssignDAY(U_INT obj);
  void    AssignNIT(U_INT obj);
  //---------------------------------------------------------
  int     FreeALL();
  int     FreeNTX();
  void    FreeALT();
  void    FreeDAY();
  void    FreeNIT();
  int     FreeDOBJ();
  void    PopTextures(U_CHAR opt);
  void    GetTileIndices(U_INT &tx,U_INT &tz);
	//--- Name --------------------------------------------------
	inline  char *GetName()		{return Name;}
	//--- Set Type ----------------------------------------------
	inline  void   SetType(U_CHAR t)	{TypTX |= t;}
	inline  void   SetFlag(U_CHAR t)	{xFlag |= t;}
  //-------Check texture type ---------------------------------
  inline  void   RazName()  {TypTX = 0;}
  inline  bool   HasName()  {return (TypTX != 0);}
  inline  U_CHAR UnNamed()  {return (TypTX == 0);}
  inline  U_CHAR IsWater()  {return (TypTX == TC_TEXWATER);}
  inline  U_CHAR IsSlice()  {return (TypTX == TC_TEXRAWTN);}
  inline  U_CHAR IsGener()  {return (TypTX == TC_TEXGENER);}
  inline  U_CHAR IsCoast()  {return (TypTX == TC_TEXCOAST);}
  inline  U_CHAR IsAnEPD()  {return (TypTX == TC_TEXRAWEP);}
  inline  U_CHAR IsShare()  {return (TypTX == TC_TEXSHARD);}
	//-------------------------------------------------------------
	inline  bool   HasType(U_CHAR t)	{return (TypTX == t);}
  //-------------------------------------------------------------
  inline  bool   NotRDY()   {return (dOBJ == 0);}
  //-------------------------------------------------------------
  inline  U_CHAR UserTEX()  {return (xFlag & TC_USRTEX);}
  inline  U_CHAR SeaLayer() {return (xFlag & TC_SEALAY);}
  inline  U_CHAR IsNight()  {return (xFlag & TC_NITTEX)?(1):(0);}
  //-------------------------------------------------------------
  inline  GLuint   GetDOBJ()  {return dOBJ;}
  inline  GLuint   GetNOBJ()  {return nOBJ;}
  inline  CmQUAD  *GetQUAD()  {return quad;}
  inline  void   SetDOBJ(GLuint obj){dOBJ = obj;}
  inline  void   SetNOBJ(GLuint obj){nOBJ = obj;}
  inline  void   SetCoast(char *d)  {coast = d;}
  inline  void   SetQUAD(CmQUAD *q) {quad  = q;}
  //--------------------------------------------------------------
  inline  void   AddFlag(U_CHAR f)  {xFlag |= f;}
  inline  void   SetResolution(U_CHAR n,U_CHAR r)   {Reso[n] = r;}
  inline  U_CHAR GetResolution()    {return Reso[0];}
  //--------------------------------------------------------------
  inline  U_INT  NextIndice(U_INT ind)  {return ((ind + 1) & TC_WORLDDETMOD);}
  //--------------------------------------------------------------
  inline  bool   HasADTX()    {return (dTEX[1] != 0);}
  inline  bool   HasDOBJ()    {return (dOBJ != 0);}
  inline  bool   NoCoast()    {return (0 == coast);}
  //-----QUAD identifier for test purpose ------------------------
  inline  bool   AreWe(U_INT tx,U_INT tz);
	//--- copy name ------------------------------------------------
	inline  int		 CopyName(char *n)	
		{strncpy(Name,n,TEX_NAME_DIM); return TEX_NAME_DIM;}
};
//============================================================================
//  Ground tile: Define text descriptor for a detail tile that
//  if part of an airport ground
//============================================================================
class CGroundTile {
  //--- Attributes --------------------------------------------
  U_INT         ax;                       //  Tile ident X
  U_INT         az;                       //  Tile ident Z
	C_QGT       *qgt;												// Tile QGT
  CTextureDef *txn;                       //  Tile descriptor
	CmQUAD      *quad;
	int          dim;												// Quad number
	GLint        sIND[QUAD_RESOLUTION];			//   Start indices
	GLint				 nIND[QUAD_RESOLUTION];			//   Count of indices
  //--- Methods ------------------------------------------------
public:
  CGroundTile(U_INT x,U_INT z);
  //------------------------------------------------------------
	void		Free();
  int		  StoreData(CTextureDef *d);
	int			TransposeTile(TC_GTAB *d,int s, SPosition *o);
	//--- VBO management -----------------------------------------
	int 		GetNbrVTX();
	void		Draw(U_INT obj);
	void		DrawGround(U_INT xo);
	//------------------------------------------------------------
  inline CmQUAD      *GetQUAD()		{return quad;}
  inline U_INT        GetAX()			{return ax;}
  inline U_INT        GetAZ()     {return az;}
	//------------------------------------------------------------
	inline void	SetQGT(C_QGT *q)			{qgt = q;}
	inline void SetTXD(CTextureDef *t){txn = t;} 
	inline void SetQUAD(CmQUAD *q)		{quad = q;}
};
//============================================================================
//  SUPERTILE  DESCRIPTOR
//	Super held data for 16 Detail Tile
//	1) The VBO buffer is an array of all vertices for the 16 Detail Tiles
//	2) The buffer is refreshed each time the resolution is changed.  All
//		 Details Tile will contribute to give the new textures coordinates
//	3) When the Super Tile is at far distance, only the outline is drawn
//		 using the SuperTile contour.
//	4) The SuperTile hold a list of 3D objects that are drawn if the supertile
//		is near enough and is visible.
//============================================================================
//-------------------------------------------------------------
class CSuperTile  {
  friend class CSupQueue;
	//--- Define render vector -----------------------------
	typedef void (CSuperTile::*VREND)(void);	// Rending vector
  //---Member data ---------------------------------------
public:
    CSuperTile   *Next;                   // Linkage
		//---------------------------------------------------------------
		C_QGT				*qgt;											// QGT 
    U_CHAR       State;                   // Request code
    U_CHAR       NoSP;                    // Super Tile number [0-63]
    U_CHAR       sta3D;                   // 3D state
    U_CHAR       rfu1;										// reserved futur use
		//---Rendering vector -------------------------------------------
		VREND					Rend;										// Rending vector
    //---------------------------------------------------------------
    U_CHAR        aRes;                   // Alternate resolution
    U_CHAR        swap;                   // 0 = No swap permitted
    U_CHAR        Reso;                   // Resolution 1
    U_CHAR        LOD;                    // Level of Detail
		//--- Vertex buffer ---------------------------------------------
    U_SHORT      nbVTX;                   // Number of vertices
		TC_GTAB			*vBUF;										// Buffer
		U_INT				 aVBO;										// Vertex Buffer Object
    //----3D management ---------------------------------------------
    float        dEye;            // True eye distance
    float       alpha;            // Alpha chanel
		CObjQ					zbQ;						// Queue for nozb objects
    CObjQ         woQ;            // Super Tile 3D object Queue
    bool      visible;            // Visibility indicator
    float        white[4];                // Diffuse color for blending
    //---------------------------------------------------------------
    SPosition    cPos;                        // Center position
    //---------------------------------------------------------------
    CVector      Center;                      // Center Coordinates
    float        MiniH;                       // Minimum Height
    float        MaxiH;                       // Maximum Height
    float        dimz;                        // Z size
    CTextureDef  Tex[TC_TEXSUPERNBR];         // List of Tetxures
    F3_VERTEX    Tour[TC_SPTBORDNBR + 2];     // SP contour
    //------Methods ---------------------------------------------
    CSuperTile();
   ~CSuperTile();   
    bool          NeedMedResolution(float rd);
    bool          NeedHigResolution(float rd);
    int           Inside3DRing();
    int           Outside3DRing();
    int           Update3Dstate();
		//-----------------------------------------------------------
		void					LoadVBO();
		//-----------------------------------------------------------
		void					Reallocate(char opt);
    void					AllocateVertices(char opt);
    void          RazNames();
    void          BuildBorder(C_QGT *qt,U_INT No);
    //-----------------------------------------------------------
    void          GetLine(CListBox *box);
    //-----------------------------------------------------------
		void					DrawOuterSuperTile();
    void          DrawInnerSuperTile();
    void          DrawTour();
    int           Draw3D(U_CHAR tod);
    void          Add3DObject(CWobj *obj); 
		void					BindVBO();
    //-----------------------------------------------------------
    inline CTextureDef *GetTexDesc(int nd)  {return (Tex + nd);}
    //-----------------------------------------------------------
    inline void   SetVisibility(bool v)   {visible = v;}
    inline bool   IsVisible()             {return visible;}
    inline bool   IsOutside()             {return (sta3D == TC_3D_OUTSIDE);}
    inline float  GetTrueDistance()       {return dEye;}
    inline float  GetAlpha()              {return alpha;}
		//--- Vertex buffer manageement -----------------------------
		inline TC_GTAB* GetVertexTable()			{return vBUF;}
    //-----------------------------------------------------------
    inline U_CHAR GetLOD()  {return LOD;}
    inline void zrSwap()  {swap = 0;}
    inline bool IsReady() {return (State == TC_TEX_RDY);}
    inline bool NeedLOD() {return (State == TC_TEX_LOD);}
    inline bool NeedOBJ() {return (State == TC_TEX_OBJ);}
    inline bool NeedSWP() {return (State == TC_TEX_POP);}
    inline bool NeedALT() {return (State == TC_TEX_ALT);}
    //-----------------------------------------------------
    inline void WantRDY() {State  = TC_TEX_RDY; }
    inline void WantLOD() {State  = TC_TEX_LOD; }
    inline void WantOBJ() {State  = TC_TEX_OBJ; }
    inline void WantALT() {State  = TC_TEX_ALT; }
    inline void WantSWP() {State  = TC_TEX_POP; }
    inline void SetState(U_CHAR s)  {State = s;}
    //-----------------------------------------------------
    inline bool NeedHigres() {return((Reso != TC_HIGHTR) && (aRes != TC_HIGHTR));}
    //-----------------------------------------------------
		inline void RenderOUT()	{Rend = &CSuperTile::DrawOuterSuperTile;}
		inline void RenderINR()	{Rend = &CSuperTile::DrawInnerSuperTile;}
		inline void Draw()		{(this->*Rend)();}
  };
//============================================================================
//  QUEUE of SUPERTILES
//============================================================================
class CSupQueue {
  	pthread_mutex_t		mux;					      // Mutex for lock
    CSuperTile *First;
    CSuperTile *Last;
    CSuperTile *Prev;
    int        NbSP;
    //--------Methods ------------------------------------
public:
    CSupQueue();
    void        PutLast(CSuperTile *sp);
    CSuperTile  *Pop();
    CSuperTile  *Detach(CSuperTile *sp);
    //-------in lines ------------------------------------
    CSuperTile  *GetFirst()                {Prev = 0;  return First;}
    CSuperTile  *GetNext(CSuperTile *sp)    {Prev = sp; return sp->Next;}
    bool        IsEmpty()                 {return (First == 0);}
    bool        NotEmpty()                {return (First != 0);}
    bool        NotFull()                 {return (NbSP != 64);}
    int         GetNumberItem()           {return NbSP;}
    void        Lock()                    {pthread_mutex_lock (&mux);}
    void        UnLock()                  {pthread_mutex_unlock (&mux);}
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
  U_LONG   xKey;                  // X coordinate in world grid
  U_LONG   zKey;                  // Z coordinate in world grid
  U_CHAR   Fixe;                  // Fixed
  U_CHAR   Use;                   // Quad user count
  U_CHAR   nElev;                 // Number of elevations
  U_CHAR   gType;                 // Ground type when vertex is center tile
	//--- Texture indices -------------------------------
	U_CHAR   indS;									// S Texture indice
	U_CHAR	 indT;									// T Texture indice
	U_CHAR	 inES;									// S border  indice
	U_CHAR   inNT;									// N border  indice
  //--- Total elevation -------------------------------
  float    Ground;
  //--- Edge/Corner pointers --------------------------
  CVertex *Edge[4];               // 4 Adjacent vertices
  //--- World coordinates -----------------------------
  WCoord   Coord;                   // World coordinates
  //------------Methods ---------------------------------
public:
  CVertex(U_LONG xk,U_LONG zk);
  CVertex();
 ~CVertex();
  //----- Relative vector --------------------------------
  CVector RelativeFrom(CVertex &a);
	CVector DistanceFrom(CVertex &a);
  bool    IsAbove(float y);
  bool    ToRight(float x);
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
  inline void Assign(double *ft)        {Coord.Assign(ft);}
  inline void Tour(F3_VERTEX *t,char v) {Coord.SetTour(t,v);}
  //------------------------------------------------------
  inline void AssignNE(TC_GTAB *tab){Coord.AssignNE(tab);}
  inline void AssignNB(TC_GTAB *tab){Coord.AssignNB(tab);}
  inline void AssignNW(TC_GTAB *tab){Coord.AssignNW(tab);}
  inline void AssignWB(TC_GTAB *tab){Coord.AssignWB(tab);}
  inline void AssignSW(TC_GTAB *tab){Coord.AssignSW(tab);}
  inline void AssignSB(TC_GTAB *tab){Coord.AssignSB(tab);}
  inline void AssignSE(TC_GTAB *tab){Coord.AssignSE(tab);}
  inline void AssignEB(TC_GTAB *tab){Coord.AssignEB(tab);}
	inline void AssignCT(TC_GTAB *tab){Coord.AssignCT(tab);}
	//-------------------------------------------------------
	inline void AssignHT(TC_GTAB *tab){Coord.AssignHT(tab);}
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
	inline U_CHAR GetXBand()						{return (xKey >> 21) << 3;}
	inline U_INT  keyX()								{return xKey;}
	inline U_INT  keyZ()								{return zKey;}
  //--------Maximum Band coordinate ----------------------------------
  inline double GetMX()                       {return Coord.GetMX();}
  inline double GetMY()                       {return Coord.GetMY();}
  //--------World    coordinates -------------------------------------
  inline void   SetWZ(float elv)       {Coord.SetWZ(elv);}
	inline double  GetWX()               {return Coord.GetWX();}
  inline double  GetWY()               {return Coord.GetWY();}
  inline double  GetWZ()               {return Coord.GetWZ();}
	//------------------------------------------------------------------
  inline void   SetEdge(U_CHAR e,CVertex *v)  {Edge[e] = v;}
  inline void   SetCornerHeight()             {Coord.SetWZ(Ground / nElev);}
  inline void   SetCorner(U_CHAR c,CVertex *v){Edge[c] = v; if (v) v->IncUse();}
	//--- For debug ---------------------------------------------------
	inline bool   Is(U_INT x,U_INT z)	{return (xKey == x) && (zKey == z);}
	//----Make a detail key only---------------------------------------
	inline U_INT  VertexKey() 
	{ return ((xKey & TC_DTSUBMOD) << 16) | (zKey & TC_DTSUBMOD);}
	inline short xCln()								{return (xKey & TC_1024MOD);}
	inline short zRow()								{return (zKey & TC_1024MOD);}
  //---------------------------------------------------
  inline double GetAbsoluteLongitude()        {return FN_ARCS_FROM_SUB(xKey);}
	inline double LongitudeOffset()   {return TC_ARCS_PER_BAND * FN_BAND_FROM_INX(xKey);}
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
	//----------------------------------------------------
 };

//============================================================================
//  Class CmQUAD.  This class describe a Detail Tile
//	NOTE: 1)	the vTAB pointer is a pointer in the vertex table held by the
//						SuperTile.  Il should not be deleted from here. This is where
//						the vertex coordinates for this Quad are stored.  There are
//						nvtx vertices
//				2)  qARR is a pointer to this quad when the detail tile mesh is not
//						subdived further (There is only one QUAD).  When the Detail Tile
//						is subdivided in subQuad, qARR points to an array of the subquads
//						resulting from the subdivision.  qDim gives the side of this array
//				3)  iBUF is used as a list of start indices and counters to the Vertex
//						Table held in the SuperTile.
//				
//============================================================================
#define QUAD_GND  0x01                // Airport ground indicator
//============================================================================
class CmQUAD {
	friend class TCacheMGR;
	//--- Define render vector -----------------------------
	typedef void (CmQUAD::*VREND)();	// Rendering vector
	//--- ATTRIBUTES ---------------------------------------------------
  U_CHAR      subL;                   // Subdivision level
  U_CHAR      Flag;                   // Indicators
  U_SHORT     qDim;                   // Array dimension (0 for a Quad pointer)
  U_SHORT     nvtx;                   // Number of vertices
	//------------------------------------------------------------------
  CmQUAD     *qARR;										// Array of CmQUAD
  CVertex     Center;                 // Quad Center
	CSuperTile *msp;										// Mother super tile
  TC_GTAB    *vTab;                   // Pointer in Vertex table
	GLint      *iBUF;										// VBO indices
	VREND       Rend;										// Rendering vector
  //---- METHODS  -----------------------------------------------------
public:
  CmQUAD();
 ~CmQUAD();
  //--------------------------------------------------------
  double      LongitudeOffset();
  void        SetArray(CmQUAD *cp,U_SHORT dim);
  void        SetParameters(CVertex *ct,U_CHAR l);
  int         CountVertices();
	int					NbrVerticesInTile();
	int					InitIndices(CSuperTile *s,char opt);
  void        GetTileIndices(U_INT &tx,U_INT &tz);
  CmQUAD     *Locate2D(CVector &p);
  U_INT       GetSuperNo();
	U_INT				GetTileNo();
	U_INT				WorldTileKey();
	//--- Vertex buffer management --------------------------
	void				Clean();
	int 				InitVTAB(TC_GTAB *vbo,int d, U_CHAR r);
	int					InitVertexCoord(TC_GTAB *vbo,float *txt);
	void				RefreshVTAB(CSuperTile *sp,U_CHAR res);
	void				RefreshVertexCoord(CSuperTile *sp,float *txt);
	int         TransposeVertices(TC_GTAB *vbo,SPosition *org);
	int					RelocateVertices(TC_GTAB *vbo,SPosition *org);
  //-------------------------------------------------------
  bool        AreWe(U_INT qx,U_INT tx,U_INT qz,U_INT tz);
	bool        AreWe(U_INT ax,U_INT az);
  //-------------------------------------------------------
  bool        PointHeight(CVector &p,CVector &nm);
  //-------------------------------------------------------
  bool        PointInSW(CVector &p, CVector nm);
  bool        PointInSE(CVector &p, CVector nm);
  bool        PointInNE(CVector &p, CVector nm);
  bool        PointInNW(CVector &p, CVector nm);
  //-------------------------------------------------------
  void        Contour();
  void        DrawNML();			// Draw normal mode
	void				DrawIND();			// Draw with indices
	void				DrawVBO();			// Draw with VBO
	//--- For terrain editor interface ----------------------
	void				GetVertices(TRACK_EDIT &w);
	void				PutVertices(TRACK_EDIT &w, CmQUAD  *qd);
	void				StoreVertex(TRACK_EDIT &w, CVertex *vt);
	void				PatchVertices(ELV_PATCHE &p);
	void				ProcessPatche(ELV_PATCHE &p);
	CVertex    *Patche(CVertex *vt,ELV_PATCHE &p,int dir);
  //-------------------------------------------------------
  inline CmQUAD  *GetArray()                {return qARR;}
  inline TC_GTAB *GetVTAB()                 {return vTab;}
  inline U_INT    GetSize()                 {return qDim;}
  inline float    CenterElevation()         {return Center.GetWZ();}
  inline void     SetCorner(U_CHAR c,CVertex *v) {Center.SetCorner(c,v);}
	inline void			SetLevel(U_CHAR sub)			{subL = sub;}
	inline U_CHAR   GetLevel()								{return subL;}
  //-----------Get inline ---------------------------------
  inline CVertex *GetCorner(U_SHORT cn) {return Center.Edge[cn];}
  inline CVertex *GetCenter()			{return &Center;}
  inline bool     IsaQuad()       {return (this == qARR);}
  inline bool     IsArray()       {return (this != qARR);}
  //-------------------------------------------------------------------
  inline U_CHAR   GetGroundType() {return  Center.gType;}
  inline U_INT    GetTileAX()     {return (Center.xKey >> TC_BY1024);}
  inline U_INT    GetTileAZ()     {return (Center.zKey >> TC_BY1024);}
  inline U_INT    GetTileTX()     {return (Center.xKey >> TC_BY1024) & TC_032MODULO;}
  inline U_INT    GetTileTZ()     {return (Center.zKey >> TC_BY1024) & TC_032MODULO;}
  inline int      GetNbrVTX()     {return nvtx;}
  //--------------------------------------------------------------------
	inline void SetGTAB(TC_GTAB *t)				{vTab = t;}
  inline void SetGroundType(U_CHAR t)   {Center.gType = t;}
  inline void MarkAsGround()            {Flag |= QUAD_GND;}
  inline void ClearGround()             {Flag &= (-1 - QUAD_GND);}
  inline bool IsAptGround()             {return ((Flag & QUAD_GND) != 0);}
  //--------------------------------------------------------------------
	inline void			RenderNML()			{Rend = &CmQUAD::DrawNML;}
	inline void			RenderIND()			{Rend = &CmQUAD::DrawIND;}
	inline void			RenderVBO()			{Rend = &CmQUAD::DrawVBO;}
	inline void			DrawTile(){(this->*Rend)();}
	//--------------------------------------------------------------------
	inline CSuperTile *GetSuperTile()	{return msp;}
};
//==============================================================================
//  CONTEXTE FOR QUAD SUBDIVISION
//==============================================================================
//---------Contexte structure for recursive routine ----------
struct CTX_QUAD {
    U_SHORT Target;                       // Target level
    U_SHORT Actual;                       // Actual level
    U_INT      vSub;                      // Vertex subdivision
    U_SHORT    ebx;                       // Elevation  X base
    U_SHORT    ebz;                       // Elevation  Z base
    U_SHORT Inc[3];                       // Elevation increment
    U_SHORT     qx;                       // Quad X coordinate
    U_SHORT     qz;                       // Quad Z coordinate
    U_SHORT    qDim;                      // Quad array Dimension
		CSuperTile *sup;												// Super tile
    CmQUAD    *qARR;                      // Quad Array
    CVertex   *sw;                        // south west corner
    U_INT      tx;                        // Tile key
    U_INT      tz;                        // Tile Key
  };
//=============================================================================
//  CLASS Quarter Tile descriptor
//  C_QGT
//=============================================================================
//--------------------------------------------------------------
class C_QGT: public CShared {
  friend class TCacheMGR;
  friend class C_QTR;
  friend class C_STile;
  friend class C_TRN;
  friend class qgtQ;
  friend class C3DMgr;
  friend class SqlMGR;
  //---------Render vector -------------------------------------
public:
  typedef void (C_QGT::*RenFN)(U_INT No);
private:
  //---------Attributes ----------------------------------------
  C_QGT        *Next;                     // Next in Action Queue
  U_SHORT       xKey;                     // X coordinate in world grid
  U_SHORT       zKey;                     // Z coordinate in world grid
  U_INT         qKey;                     // QGT key (X,Z)
  U_CHAR        rCode;                    // Request code
  U_INT         rKey;                     // Request parameter
  SPosition     Scene;                    // Mid position for scenery
  //--------Band parameters--------------------------------------
  U_CHAR        xBand;                    // Tile X band
  U_CHAR        yBand;                    // Tile Y band
  U_CHAR        visb;                     // is visible
  //--------Mux for Step protection -----------------------------
  pthread_mutex_t	stMux;                  // State lock
  U_CHAR        qSTAT;                    // Quad available when 0
  U_CHAR        Step;                     // QGT Major steps
  U_CHAR        nStp;                     // Next Step
  U_CHAR        dead;                     // Dead mark
  //-------------------------------------------------------------
  double        aLon;                     // West absolute longitude
  double        wLon;                     // West  (band) longitude of SW corner
  double        sLat;                     // South Latitude of SW corner
  double        nLat;                     // North Latitude
  double        dLat;                     // Latitude delta for Detail Tile
  double        dLon;                     // Longitude delta for detail tile
  SPosition     mPoint;                   // Middle point
  TCacheMGR    *tcm;                      // Cache manager
  CTextureWard *txw;                      // Texture Manager
  //----Working data for coast line ----------------------------
  U_INT         seaKEY[4];                // Key to read sea file
  U_INT         seaREQ[4];                // Key to load sea file
  //----Divide process working area ----------------------------
  int           eDim;                     // Elevation array size
  int          *eLev;                     // Array elevation
  //----Resources related to this QGT --------------------------
  C_QTR        *qtr;                      // Related QTR entry
  C_TRN        *trn;                      // Related TRN object
  //---------Cloud ownership -----------------------------------
  Tag           Metar;                    // Metar Key       
  U_CHAR        cloud[4];                 // counter for cloud types
  //---------3D objects ----------------------------------------
  C3Dworld      w3D;                      // Related world objects
  //---------Texture Parameters --------------------------------
  float         dto;                      // Texture origin
  float         dtu;                      // Texture unit
  //---------Center coordinates --------------------------------
  CVector       Center;                   // Center in feet
  CVector       Bound;                    // Bounding
  TRANS2D       qTran;                    // QGT translation
  //---------Option indicator ----------------------------------
  U_CHAR          tr;                     // Trace indicator
  U_CHAR          vbu;                    // use VBO
	U_CHAR					elv;										// Has base elevation
  //---------Demux counter--------------------------------------
  U_CHAR          demux;
  //------------------------------------------------------------
  CVertex    *Corner[4];                  // Corner vertices
  //---------Array for 1024 initial Detail Tile Quads ----------
  CmQUAD qTAB[32*32];                     // Quad pointers
  CSuperTile Super[TC_SUPERT_NBR];        // Super Tile context
  CSupQueue FarsQ;                        // Far  Super Tiles
  CSupQueue NearQ;                        // Near Super Tiles
  CSupQueue LoadQ;                        // Loading Queue
  //----------Methods ------------------------------------------
public:
  C_QGT(U_INT xk, U_INT zk,TCacheMGR *tm);
  C_QGT() {}                              // Just for reservation
 ~C_QGT();                                // Destroy this QGT
  void    TraceDelete();
  //-------------------------------------------------------------
  void TimeSlice(float dT);
  //---------Helpers  ---------------------------------
  inline  U_CHAR      GetReqCode()        {return rCode;}
  inline  U_INT       GetReqKey()         {return rKey;}
  inline  void        SetQTR(C_QTR *qtf)  {qtr  = qtf;}
  inline  double      GetDlon() {return  dLon;}
  inline  double      GetDlat() {return  dLat;}
  inline  U_INT       GetXkey() {return xKey;}
  inline  U_INT       GetZkey() {return zKey;}
  inline  U_INT       FullKey() {return qKey;}
  inline  bool  NotVisible()    {return (visb == 0);}
  inline  bool        NoQuad()  {return (1 == qSTAT);}
  inline  bool        HasQuad() {return (0 == qSTAT);}
	//---- Band translation -------------------------------
	inline	U_CHAR			GetHband(){return xBand;}
	//-----------------------------------------------------
	inline  void				IndElevation()	{elv = 1;}
	inline  bool				HasElevation()	{return elv != 0;}
  //--------3D Object -----------------------------------
  inline  int         IncNOBJ() {return w3D.IncNOBJ();}
  inline  int         DecNOBJ() {return w3D.DecNOBJ();}
  inline  int         GetNOBJ() {return w3D.GetNOBJ();}
  inline  C3Dworld   *Get3DW()  {return &w3D;}
  //-------Create vertex in line ------------------------------
  inline  CVertex    *CreateVertex(U_LONG vx,U_LONG vz)
                      { CVertex *vt = new CVertex(vx,vz);
                        globals->NbVTX++;
                        return vt;
                      }
  //---------File management -----------------------------------
  inline  void        AssignQTR(C_QTR *qt) {qtr = qt;}
  inline  CmQUAD     *GetQUAD(U_INT No) {return &qTAB[No];}
	//--- Step routines ------------------------------------------
	int					StepINI();					// Initialize
  int					StepMSH();					// Mesh divide
	int					StepELV();					// Load Elevations
	int					StepQTR();					// Set Elevations from QTR
	int					StepTRN();					// Set Elevations from TRN
	int					StepHDT();					// Set detail elevations from TRN
	int					StepTIL();					// Detail elevations
	int					StepPCH();					// Elevation patche
	int					StepSEA();					// Set coast data
	int					StepSUP();					// Finalize Super Tile
  //---------State management ----------------------------------
  void				LockState()  {pthread_mutex_lock   (&stMux);}
  void				UnLockState(){pthread_mutex_unlock (&stMux);}
  bool				NotReady()   {return (Step != TC_QT_RDY);}
  bool				IsReady()    {return (Step == TC_QT_RDY);}
  bool				NotAlive()   {return (dead == 1);}
  void				SetStep(U_CHAR s);
  void				PostIO();
  //---------Queing management ---------------------------------
  CSuperTile *PopLoad();
  CSuperTile *NextLoad(CSuperTile *sp)  {return sp->Next;}
  void        EnterNearQ(CSuperTile *sp);
  //---------Position routines ---------------------------------
  bool        GetTileIndices(SPosition &pos,short &tx, short &tz);
  bool        GetTileIndices(GroundSpot &gns);
  //----------Helper -------------------------------------------
  CmQUAD     *GetQuad(U_INT No,U_SHORT rx,U_SHORT rz);
	CmQUAD		 *GetQuad(U_INT ax,U_INT az);
  void        Abort(char *msg);
  void        Abortv();
  void        SetAXBYkey();
  int         CheckSeaPOD(int k);
  int         CheckSeaSQL(int k);
	bool				AreWe(U_INT qx,U_INT qz);
	int					HasTRN();
	void				Reallocate(CmQUAD *qd);
  //----------Mesh Management ------------------------------------
  int         CenterTile(CVertex *sw,CVertex *nw,CVertex *ne,CVertex *se);
  int         DivideTile(U_INT div,CVertex *sw,CVertex *nw,CVertex *ne,CVertex *se);
  CVertex    *GetEastVertex (CVertex *n,U_LONG vx,U_LONG vz,int op);
  CVertex    *GetNorthVertex(CVertex *n,U_LONG vx,U_LONG vz,int op);
  CVertex    *CreateCenter(CVertex *sw,CVertex *se,CVertex *ne,CVertex *nw,CVertex *op);
  void        DetailElevation(U_INT tx,U_INT tz,float elev);
  void        SetMiniMax(float el,CVertex *vt,CSuperTile *sp);
  //---------TRN Management  --------------------------------------
  void        SetElvFromTRN(U_INT tx,U_INT tz,float *el);
  void        DivideHDTL(TRN_HDTL *hd);
  CVertex *   DuplicateCenter(CVertex *ct,TRN_HDTL *hd);
  float       GetHDTLelevation(U_SHORT dx,U_SHORT dz,CTX_QUAD &ctx);
  int         DivideQuad(CVertex *ct,CTX_QUAD &ctx);
  int         TerminalQuad(CVertex *ct,CTX_QUAD &ctx);
  //--------Super Tile Management --------------------------------
  void        InitSuperTiles();
  int         PutOutside();
  int         UpdateInnerCircle();
  void        FlushTextures();
  CTextureDef *GetTexDescriptor(U_INT tx,U_INT tz);
  CTextureDef *GetTexList(U_INT No)    {return Super[No].Tex;}
  CSuperTile   *GetSuperTile(U_INT No)   {return &Super[No];}
  CSuperTile   *GetSuperTile(int tx,int tz);
//--------Delete resources --------------------------------------
  int         FreeQuad(CmQUAD *cp);
  int         FreeVertices(CmQUAD *cp);
  int         DeleteQTR();
  void        FreeHMidVertex(CVertex *vt);
  void        FreeVMidVertex(CVertex *vt);
  void        FreeAllVertices();
  void        UnlinkVertex(CVertex *vt);
  //--------Drawing ----------------------------------------------
  void        DrawSuperMesh(U_INT wat);
  void        DrawWiresSuperTile(CSuperTile *sp);
  //--------Coast management -------------------------------------
  inline      U_INT       GetSeaKEY(int k)        {return seaKEY[k];}
  inline      U_INT       GetSeaREQ(int k)        {return seaREQ[k];}
  inline      void        SetSeaREQ(int k,U_INT r){seaKEY[k] = r;}
  //--------3D object management ---------------------------------
  inline      SPosition  *GetMidPoint()       {return &mPoint;}
  inline      double      GetMidLat()         {return  mPoint.lat;}
  inline      double      GetMidLon()         {return  mPoint.lon;}
  inline      double      GetMidAlt()         {return  mPoint.alt;}
  inline      double      GetWesLon()         {return  aLon;}
  inline      double      GetNorLat()         {return  nLat;}
  inline      double      GetSudLat()         {return  sLat;}
  //--------Meteo Management  -------------------------------------
  inline      void        SetMETAR(Tag m)     {Metar = m;}
  inline      Tag         GetMETAR()          {return Metar;}
  inline      bool        DifMETAR(Tag m)     {return (m != Metar);}
  //---------------------------------------------------------------
  inline      void        IncCloud(U_CHAR t)  {cloud[t]++;}
  inline      void        DecCloud(U_CHAR t)  {cloud[t]--;}
  inline      U_INT       NbCloud(U_CHAR t)   {return cloud[t];}
  //---------------------------------------------------------------
  inline      void        CheckW3D()          {w3D.Check();}
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
//	Structure to save modified elevation into blob
//=============================================================================
struct ELV_VTX {
		U_INT key;										// Vertex local key
		float elv;										// elevation
};
//=============================================================================
//		Structure to process ELEVATION PATCHE
//=============================================================================
struct ELV_PATCHE {
		char			dir;											// Patche direction
		char			subL;											// Subdivision level
		short			nbe;											// Number of entries
		U_INT			qKey;											// QGT key
		U_INT			dtNo;											// Tile number
		ELV_VTX	  mat[TC_MAX_ELV_DIM];			// Matrice of key-elevation
};
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
//============================================================================
//	Classe for elevation tracker
//============================================================================
class CElvTracker {
	//--- ATTRIBUTES ------------------------------------
	char						 upd;									// Update indicator
	TCacheMGR       *tcm;									// Cache manager
	GroundSpot			*spot;								// Ground spot
	CVector					 tran;								// Translation vector
	CmQUAD          *tile;								// Current tile
	//--- Hit buffer -------------------------------------
	GLuint					 bHit[8];						  // Hit buffer
	//--- WIndow tracker ---------------------------------
	CFuiTED         *wind;								// Window editor
	//--- Working area for QUAD --------------------------
	TRACK_EDIT			 wrk;									// For QUAD
	QUAD_QGT				 stak[4];							// Stock of QGT
	int							 nbs;									// Number of super Tile
	//--- Selected vertex --------------------------------
	TVertex         *svrt;
	//--- METHODS ----------------------------------------
public:
	CElvTracker();
 ~CElvTracker();
	//----------------------------------------------------
	void	TimeSlice(float dT);
	//----------------------------------------------------
	void	FixElevation();
	void	InitStack();
	void	FillStack(U_INT ax,U_INT az);
	bool	PickOne(CRabbitCamera *cam);
	void  HaveElevation();
	float	IncElevation(float dte);
	float	SetElevation(float elv);
	void  Register(CFuiTED *w);	
	//----------------------------------------------------
	void	Draw();
	void	DrawAll();
	void	DrawMark(TVertex *vdf);
	//----------------------------------------------------
	inline void SetTCM(TCacheMGR *t)	{tcm = t;}
	inline CmQUAD *GetTile()	{return tile;}							
};
//=============================================================================
//  Terrain Cache manager
//=============================================================================
class TCacheMGR {
  //---------Attributes ----------------------------------------
  float       dTime;
  float       eTime;                        // Elapse time     
  U_INT       rKEY;                         // Reference QGT (aircraft position)
  U_INT       nKEY;                         // New Key
  U_SHORT     qRDY;                         // Number qgt waiting ready
  //------Terrain parameters ------------------------------------
  U_CHAR      xBand;                        // Aircraft xBand
  U_CHAR      yBand;                        // Aircraft yBand
  U_CHAR      wire;                         // QGT step
  U_CHAR      Terrain;                      // Terrian indicator
  //------Teleport ----------------------------------------------
  U_CHAR      Tele;                         // Teleport in action
  //-------Vector map:  pixels per mile -------------------------
  SVector     ppM;                          // Pixels per mile
  TC_SPOINT  sDIM;                          // Screen dimension
  //--------Current camera and transform ------------------------
  CCamera    *cCam;                         // Current camera
  //--------Time of day -----------------------------------------
  U_CHAR      cTod;                         // Day/Night indicator
  U_CHAR      sInd;                         // Sun indicator
  SDateTime   Hours;
  //-------Type tile file ---------------------------------------
  CTerraFile *tFIL;                         // Tile image
  C_QGT      *cQGT;                         // Current QGT
  U_INT       tx;                           // Current Tile X index in QGT
  U_INT       tz;                           // Current Tile Z index in QGT
  U_INT       ax;                           // Absolute X index
  U_INT       az;                           // Absolute Z index
  //--------Aircraft position ------------------------------------
  SPosition   aPos;                         // Aircraft position
  GroundSpot  Spot;                         // Aircraft spot
	double      fAGL;													// Altitude AGL (in feet)
  CVector     geow;                         // Coordinates in feet
  U_SHORT     xKey;                         // QGT X number
  U_SHORT     zKey;                         // QGT Z number
  //------North magnetic management ------------------------------
  U_CHAR      magRF;                        // Time to refresh
  float       magDV;                        // Magnetic deviation
  float       magFD;                        // Field strength
  //------Triangle where P reside --------------------------------
  CVector     gNM;                          // Normal vector
  //--------TRANSFORMATIONS --------------------------------------
  SVector     scale;                        // Scaling parameters
  SVector     cTran;                        // Terrain translation vector
  TRANS2D     aTran;                        // Airport translation
  //--------Horizon Parameters -----------------------------------
  double      hLine;                        // Distance to horizon
  float       fDens;                        // Fog Density to use
  float       fogC[4];                      // Fog color
  float       skyc[4];                      // Sky color
  float       EmsColor[4];                  // Emissive color
  float       NitColor[4];                  // Night emission
  float       TgtColor;                     // Night target color
  float       lumn;                         // Current luminosity
	//-------Terrain parameters ------------------------------------
	float       gplan[4];											// Floor plane
  //--------Latitude correction factors --------------------------
  double      rFactor;                      // X longitude Reduction factor
  double      cFactor;                      // X compensation factor
  //-------Resolution parameters -----------------------------
  float       higRAT;                       // Hight def ratio
  float       medRAD;                       // Drawing limit
  float       higRAD;                       // Radius max for high res
  float       hDist;                        // Horizon line
  //--------Shadow parameters---------------------------------
  double      sunT;                         // Sun apparent radius
  double      sunR;                         // Sun radius
  CVector     sunP;                         // Sun position
  float       sunC[4];                      // Sun color
  float     lp[16];                         // Light projection
  float     lv[16];                         // Light view
  //--------Draw Parameter -----------------------------------
  GLUquadricObj *ground;                    // Flat ground
  GLUquadricObj *sphere;                    // A Sphere
  double    orient;                         // Camera orientation
  GLint       mask;                         // Attribute mask
  //-------TEXTURE COORDINATE  TABLES ---------------------------
  U_CHAR      Disp;                         // Display accounting
  U_CHAR      HiRes;                        // Hires permited
  U_CHAR      FactEV;                       // Shift factor
  U_CHAR      LastEV;                       // Last indice in elevation
  //--------MAP for QGT and QTR ---------------------------------
  std::map<U_INT,C_QGT*>  qgtMAP;           // Map of QGT
  std::map<U_INT,C_QGT*>::iterator iqt;
  //--------Shared file access ---------------------------------
  pthread_mutex_t	qtrMux;                   // protection Mutex
  std::map<U_INT,C_QTR*>      qtrMAP;       // Map of QTR files
  pthread_mutex_t	seaMux;                   // protection Mutex
  std::map<U_INT,C_SEA*>      seaMAP;       // Map of SEA files
  std::map<U_INT,C_CDT*>      cstMAP;       // MAP of coast entry
  std::map<U_INT,char *>      ctyMAP;       // MAP of country names
  CListBox  *terBOX;                        // MAP of terrain type
  //--------Action Queue ---------------------------------------
  qgtQ     ActQ;                            // Action Queue
  qgtQ     LodQ;                            // Load Queue
  qgtQ     FilQ;                            // File Queue
  //--------Accounting -----------------------------------------
  pthread_mutex_t	ctrMux;                   // protection Mutex
  U_SHORT     clock;                        // internal clock
  U_SHORT     clock1;                       // Clock N1
  U_INT       NbQTR;                        // Number of QTR files
  U_INT       NbQGT;                        // Number of Quarter Tiles
  U_INT       NbSEA;                        // Number of Coast Files
  U_INT       NbREG;                        // Number of regions
  U_INT       NbTEX;                        // Number of textures
  U_INT       tr;                           // Trace option
  U_INT       td;                           // Thread debug
  float       aTime;                        // Accounting timer
  //---------Airport management ---------------------------------
  CAirportMgr  *aptMGR;                     // Airport   Manager
  C3DMgr       *objMGR;                     // 3D object Manager
  CTextureWard *txw;                        // Texture   Manager
	//---------Track management -----------------------------------
	CElvTracker     eTrack;										// Elevation Tracker
  //---------Thread management-----------------------------------
  pthread_t       thIden;                   // Thread identity
  pthread_cond_t  thCond;                   // Condition variable
  pthread_mutex_t	thMux;                    // Condition Mutex
  U_INT           thSIG;                    // Number of signal posted
  U_INT           thRCV;                    // Number of Signal received
  U_INT           thRUN;                    // Runing signal
  //---------TRN File name -------------------------------------
  char        trnName[64];                  // TRN file name
  //---------CULLING  --------------------------------------
  C_QGT       *vqt;                         // QGT in test
  double      pj[16];                       // Projection Matrix
  double      mv[16];                       // Model view
  double      rs[16];                       // Resulting
  float       pn[6][4];                     // Clip planes
	//------Black box ---------------------------------------------
	BBcache	   *bbox;													// TCache black box
  //---------Methods -------------------------------------------
public:
  TCacheMGR();
 ~TCacheMGR();
  //---- Trace in black box -------------------------------
	void				Enter(Tag a,void *p1, U_INT p2) {bbox->Enter(a,p1,p2);}
  //---------Init -----------------------------------------
  float      *InitTextureCoord(int dim,int res);
  void        LoadPodTerra();
  void        LoadSqlTerra();
  void        OneCountry(CCtyLine *cty);
  void        OneTerraLine(CTgxLine *lin);
  char       *GetCountry(char *key);
  //---------Helper ---------------------------------------
  void        Abort(char *msg);
  U_SHORT     GetTablePos(short vp) {return (vp < 256)?(255 - vp):(vp - 256);}
  int         LastAction();
  void        GetTileName (U_CHAR *base);
  U_CHAR      BuildName(CTextureDef *txn,U_SHORT nx,U_SHORT nz);
  void        FormatName(C_QGT *qt,CSuperTile *sp);
  int         GetTransitionMask(C_QGT *qt,CSuperTile *sp);
  U_CHAR      GetLandTransition(char *hex);
  void        SetTransitionMask(C_QGT *qgt,CTextureDef *txn);
  char        GetTileType(char tp,int dir);
  int         GetCoastMark(int inc,U_INT ax,U_INT az);
  double      PlaneLongitudeBand() {return Spot.BandLongitude();}
  float       AircraftFeetDistance(SPosition &pos);
	void				SetShadowMatrix( float mat[16],float lp[4]);
  //----------Terrain management -------------------------------
  void        GetTerrainInfo(TC_GRND_INFO &inf, SPosition &pos);
  double      SetGroundAt(GroundSpot &gns);
  void        UpdateAGL(SPosition &pos);
	void				UpdateGroundPlane();
	GroundSpot *GetSpot()		{return &Spot;}
	void				GetPlaneSpot(SPosition &p);
	//----------TIME management ----------------------------------
  void        UpdateTOD();
  U_CHAR      SetLuminosity();
  //---------Diag and Tests ------------------------------------
  bool        PlaneInTile(U_INT ax,U_INT az);
  //---------AIRPORT MANAGEMENT --------------------------------
  CmQUAD      *GetTileQuad(SPosition pos);
  CmQUAD      *GetTileQuad(U_INT ax,U_INT az);
  CTextureDef *GetTexDescriptor();
 // CTextureDef *GetTexDescriptor(U_INT tx,U_INT tz);
  CTextureDef *GetTexDescriptor(C_QGT *qtg,U_INT ax,U_INT az);
	void				FillGroundTile(CGroundTile *gnt);
  //--------METAR MANAGEMENT -------------------------------------
  void         AssignMetar(CMeteoArea *ma);
  //---------Helper --------------------------------------------
  void        MeshError(U_LONG xv,U_LONG zv);
  U_INT       GetQTRindex(U_INT cx,U_INT cz);         // Name from QGT indices
  //---------Cache update routines -----------------------------
  void        TimeSlice(float dT,U_INT FrNo);		      // Cache update
  bool        IsaNewKey(U_INT key);                   // Check for new key
  bool        IsaActKey(U_INT key);                   // Check for old key
  void        GetRange(U_INT cz,U_SHORT &up,U_SHORT &dn);
  C_QGT      *LocateQGT(QGT_DIR *itm,U_SHORT cx,U_SHORT cz);
  C_QGT      *GetQGT(U_SHORT cx, U_SHORT cz);
  C_QGT      *GetQGT(SPosition &pos);
  bool        SetQGT(GroundSpot &gns);
  CVertex    *GetQgtCorner(C_QGT *qgt,QGT_DIR *tab,U_SHORT cn);
  int         FreeTheQGT(C_QGT *qt);
  int         InitMesh();                             // Initial mesh
  int         OneAction();                            // One Action per QGT
  int         RefreshCache();
  int         UpdateQGTs(float dT);
  void        GetObjLines(CListBox *box);             // 3D object lines
  //---------STEP ROUTINES --------------------------------------
  void        CreateQGT(U_SHORT cx, U_SHORT cz);      // STEP 0
  //---------Inline ---------------------------------------------
	inline CElvTracker  *GetETrack()	{return &eTrack;}
  inline CTextureWard *GetTexWard() {return txw;}
  inline void   InActQ(C_QGT *qgt)  {ActQ.PutLast(qgt);}
  //-------------------------------------------------------------
	inline void IncRDY(short n)				{qRDY += n;}
  inline bool MeshReady()           {return (qRDY == 0);}
	inline bool MeshBusy()						{return (qRDY != 0);}
  inline bool TelePorting()         {return (Tele == 1);}
  //---------Time of day management -----------------------------
  inline bool   IsDay()             {return (cTod == 'D');}
  inline bool   IsNight()           {return (cTod == 'N');}
  inline bool   IsSun()             {return (sInd == 'S');}
  inline bool   IsDawn()            {return (lumn > 0.10);}
  inline float  Elapse()            {return eTime;}
  inline float  GetLuminosity()     {return lumn;}
  inline CVector *SunPosition()     {return &sunP;}
  //--------Return scale parameters ------------------------------
  inline SVector *GetScale()        {return &scale;}
  inline SVector *GetTerrainTrans() {return &cTran;}
  //--------Terrain Parameters -----------------------------------
  inline bool     Teleporting()       {return (Tele != 0);}
  inline double   GetHorizon()        {return hLine;}
  inline float    GetMagnecticNorth() {return magDV;}
  inline float    GetGroundAltitude() {return Spot.alt;}
	inline double		GetFeetAGL()				{return fAGL;}
  inline void     GetGroundNormal(CVector &v) {v = Spot.gNM;}
  inline U_CHAR   GetGroundType()     {return Spot.Type;}
  inline double   GetInflation()      {return cFactor;}
  inline double   GetReduction()      {return rFactor;}
  inline double*  GetProjection()     {return pj;}
  inline TRANS2D  GetTranslation()    {return aTran;}
	inline float*   GetGroundPlane()		{return gplan;}
	//---- Band translation -------------------------------
	inline U_CHAR		GetHband()					{return xBand;}
  //--------Aircraft Parameters -----------------------------------
	inline bool     QGTplane(C_QGT *q)	{return Spot.qgt == q;}
  inline SPosition  *PlaneArcsPos()      {return &globals->geop;}
  inline CVector    *PlaneFeetPos()      {return &geow;}
  inline double      GetPlaneAltitude()  {return globals->geop.alt;}
  inline double      GetPlaneLongitude() {return globals->geop.lon;}
  inline double      GetPlaneLatitude()  {return globals->geop.lat;}
  inline double      GetPlaneAGL()       {return aPos.alt - Spot.alt;}
  inline C_QGT      *GetPlaneQGT()       {return Spot.qgt;}
  inline CmQUAD     *GetPlaneQuad()      {return Spot.Quad;}
  inline bool        PlaneQuad(CmQUAD *q){return (q == Spot.Quad);}
  //--------------------------------------------------------------
  inline void   SetHires(U_CHAR r)  {HiRes = r;}
  inline bool   HiResPermited()     {return (HiRes!= 0);}
  inline char   GetTrace()          {return (tr)?(1):(0);}
  inline char   GetDebug()          {return (td)?(1):(0);}
	inline char   GetFactEV()					{return FactEV;}
  inline float  Time()              {return dTime;}
  //--------Drawing parameters -----------------------------------
  inline float  GetMedDist()        {return medRAD;}
  inline float  GetHigDist()        {return higRAD;}
  inline float *GetFogColor()       {return fogC;}
  inline float *GetFarColor()       {return fogC;}
  inline U_CHAR GetLastEVindex()    {return LastEV;}
  inline float *GetNightEmission()  {return NitColor;}
  inline float *GetDeftEmission()   {return EmsColor;}
  //---------------------------------------------------------------
  inline CListBox *GetTerraBox()    {return terBOX;}
  //---------Aircraft position ------------------------------------
  void        GetAbsoluteIndices(U_INT &ax,U_INT &az);
  void        GetTileIndices(int &tx,int &tz);
  void        Probe(CFuiCanva *cnv);
  void        EditGround(char *txt);
  void        RelativeFeetTo(SPosition &pos,SVector &v);
  SVector    *TerrainScale()        {return &scale;}
  //---------QTR routines ----------------------------------------
  void        AddRegion(U_INT nb) {NbREG += nb;}
  C_QTR      *GetQTR(U_INT key);
  bool        DecQTR (C_QTR *qtr);
  void        AddQTR(C_QTR *qtf,U_INT key);
  //--------COAST data management --------------------------------
  C_SEA      *GetSEA(U_INT key);
  void        DecSEA (U_INT key);
  void        FreeSEA(C_QGT *qgt);
  void        AddSea(C_SEA *sea,U_INT key);
  char       *GetSEAdata(U_INT ax, U_INT az);
  void        CheckCoastFile(U_INT key);
  void        LoadCoastPOD(U_INT ind);
  void        LoadCoastSQL(U_INT kxz);
  void        AllSeaPOD(C_QGT *qgt);
  void        AllSeaSQL(C_QGT *qgt);
  //-------------------------------------------------------------
  void        LockSEA()   {pthread_mutex_lock   (&seaMux);}
  void        UnLockSEA() {pthread_mutex_unlock (&seaMux);}
  //---------COAST DATA -----------------------------------------
  C_CDT      *GetCST(U_INT key);
  void        AddCST(U_INT key,C_CDT *cst);
  char       *GetCDTdata(U_INT key);
  void        FreeCST(C_QGT *qgt);
  void        DecCST(U_INT key);
  //---------TRN FILE -------------------------------------------
  int         TryEPD(C_QGT *qgt,char *name);
  //---------Drawing primitives ----------------------------------
  void        Draw(CCamera *cam);
  void        DrawHorizon();
  void        ScanCoast(SSurface *sf);
  void        ScanCoastSQL(SSurface *sf);
  void        SetSkyFog(float dens);
  //-------------------------------------------------------------
  void        ComputeClip();
  void        ExtractPlanes();
  void        Normalize(int No);
  float       Distance(CVector &vt,int No);
  float       ToPlan(U_INT No,CVector &vt,float dx, float dy, float dz);
  bool        BoxInView(CVector &ct,CVector &bd);
  int         SphereInView(CVector &vt,float rd);
  bool        PointInView(CVector &pt);
  //----------Misc items  ---------------------------------------
  void        DrawSun();
  void        GetSunPosition (void);
  void        BuildShadowMap (void);
  void        SetPPM(double rat,short wd,short ht);
  inline double GetXPPM()   {return ppM.x;}
  inline double GetYPPM()   {return ppM.y;}
  inline short  GetVmapWD() {return sDIM.x;}
  inline short  GetVmapHT() {return sDIM.y;}
  //----------File access --------------------------------------
  CTerraFile *GetTerraFile(){return tFIL;}
  int         StartIO(C_QGT *qgt, U_CHAR ns);
  void        LockQTR()   {pthread_mutex_lock   (&qtrMux);}
  void        UnLockQTR() {pthread_mutex_unlock (&qtrMux);}
  void        GoThread()  {pthread_cond_signal(&thCond); }
  void        MarkDelete(C_QGT *qgt);
  void        RequestTextureLoad(C_QGT *qgt);   // Request Textures
  void        GetQTRfile(C_QGT *qgt,TCacheMGR *tcm);
  int         RequestELV(C_QGT *qgt);
  C_QGT      *PopFileREQ()      {return FilQ.Pop();}
    //----------Thread parameters --------------------------------
  inline void ThreadRuns()      {thRUN = 1;}
  inline void ThreadStop()      {thRUN = 0;}
  //------------------------------------------------------------
  C_QGT      *PopLoadTEX()      {return LodQ.Pop();}
  pthread_cond_t  *GetTHcond()  {return &thCond;}
  pthread_mutex_t *GetTHmux()   {return &thMux;}
  //----------Test and debug -----------------------------------
  GLUquadricObj  *GetSphere()   {return sphere;}
  void        CheckW3D();
  //----------Teleport -----------------------------------------
  void        CheckTeleport();
  void        Teleport(SPosition &dst);
  void        NoteTeleport();
  void        DrawCount(float dT);
  //----------Statistics ---------------------------------------
  void        GetStats(CFuiCanva *cnv);
};

//===============END OF THIS FILE =============================================
#endif  // TERRAINCACHE_H
