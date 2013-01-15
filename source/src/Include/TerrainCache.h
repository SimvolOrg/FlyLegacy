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
#define QUAD_RESOLUTION (64)
//=============================================================================
#include "../Include/Globals.h"
#include "../Include/FileThread.h"
#include "../Include/TerrainUnits.h"
#include "../Include/TerrainData.h"
#include "../Include/GeoMath.h"
#include "../Include/3dMath.h"
#include "../Include/Model3D.h"
#include <map>
//=============================================================================
class CTerraFile;
class CMemCount;
class CVertex;
class CFuiTED;
class OSM_Object;
struct ELV_VTX;
struct ELV_PATCHE;
//=============================================================================
//  MASK
//=============================================================================
#define TC_ONLY_RGB (0x00FFFFFF)
#define TC_ONLY_ALF (0xFF000000)
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
  U_CHAR   cTyp;                            // compression type
  //----------------------------------------------------------
  U_CHAR   Reso[2];                         // Resolution
  //----------------------------------------------------------
  U_INT    sKey;                            // Shared key
	U_INT		 dSiz;														// Compressed day size
	U_INT    nSiz;														// Compressed night size
	//-----------------------------------------------------------
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
  virtual	~CTextureDef();
  //---------------------------------------------------------
	inline	void    SetDayTexture(U_CHAR n,GLubyte *t) {dTEX[n] = t;}
	inline	void    SetNitTexture(U_CHAR n,GLubyte *t) {nTEX[n] = t;}
  //---------------------------------------------------------
  int			AssignDAY(U_INT obj);
  int			AssignNIT(U_INT obj);
  //---------------------------------------------------------
  int     FreeALL();
  int     FreeNTX();
  void    FreeALT();
  void    FreeDAY();
  void    FreeNIT();
  int     FreeDOBJ();
  void    PopTextures(U_CHAR opt);
  void    GetTileIndices(U_INT &tx,U_INT &tz);
  //-----QUAD identifier for test purpose ------------------------
  bool		AreWe(U_INT tx,U_INT tz);
	//--- Name --------------------------------------------------
	inline  char *GetName()		{return Name;}
	//--- Set Type ----------------------------------------------
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
	inline  U_CHAR IsCmprs()	{return (TypTX == TC_TEXCMPRS);}
	//-------------------------------------------------------------
	inline void    ClearName() {*Name = 0; *Hexa = 0; TypTX = 0;}
	//-------------------------------------------------------------
	inline  bool   HasType(U_CHAR t)	{return (TypTX == t);}
  //-------------------------------------------------------------
  inline  bool   NotRDY()   {return (dOBJ == 0);}
  //-------------------------------------------------------------
  inline  U_CHAR UserTEX()  {return (xFlag & TC_USRTEX);}
  inline  U_CHAR SeaLayer() {return (xFlag & TC_SEALAY);}
  inline  U_CHAR IsNight()  {return (xFlag & TC_NITTEX)?(1):(0);}
	inline  void   Raz(U_CHAR F)			{xFlag &= (-1 - F);}
  //-------------------------------------------------------------
  inline  GLuint   GetDOBJ()  {return dOBJ;}
  inline  GLuint   GetNOBJ()  {return nOBJ;}
  inline  CmQUAD  *GetQUAD()  {return quad;}
  inline  void   SetDOBJ(GLuint obj){dOBJ = obj;}
  inline  void   SetNOBJ(GLuint obj){nOBJ = obj;}
  inline  void   SetCoast(char *d)  {coast = d;}
  inline  void   SetQUAD(CmQUAD *q) {quad  = q;}
  //--------------------------------------------------------------
  inline  void   SetResolution(U_CHAR n,U_CHAR r)   {Reso[n] = r;}
  inline  U_CHAR GetResolution()    {return Reso[0];}
  //--------------------------------------------------------------
  inline  U_INT  NextIndice(U_INT ind)  {return ((ind + 1) & TC_WORLDDETMOD);}
  //--------------------------------------------------------------
  inline  bool   HasADTX()    {return (dTEX[1] != 0);}
  inline  bool   HasDOBJ()    {return (dOBJ != 0);}
  inline  bool   NoCoast()    {return (0 == coast);}
	//--- copy name ------------------------------------------------
	inline  int		 CopyName(char *n)	
		{strncpy(Name,n,TEX_NAME_DIM); return TEX_NAME_DIM;}
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
    CSuperTile   *next;                   // Linkage
		//---------------------------------------------------------------
		C_QGT				*qgt;											// QGT 
		//---------------------------------------------------------------
    U_CHAR       State;                   // Request code
    U_CHAR       NoSP;                    // Super Tile number [0-63]
    U_CHAR       sta3D;                   // 3D state
    U_CHAR       obtr;										// Trace object
		//---Rendering vector -------------------------------------------
		VREND					Rend;										// Rending vector
    //---------------------------------------------------------------
    U_CHAR        levl;                   // Texture level
    U_CHAR        swap;                   // 0 = No swap permitted
    U_CHAR        Reso;                   // Resolution 1
    U_CHAR        LOD;                    // Level of Detail
		//--- Vertex buffer ---------------------------------------------
    U_SHORT      nbVTX;                   // Number of vertices
		TC_GTAB			*vBUF;										// Buffer
		U_INT				 aVBO;										// Vertex Buffer Object
    //----3D management ---------------------------------------------
    float        dEye;										// True eye distance
    float       alpha;										// Alpha chanel
    CObjQ         woQ;										// Super Tile 3D object Queue
    U_CHAR       visible;									// Visibility indicator
    float        white[4];                // Diffuse color for blending
		//--- OSM management --------------------------------------------
		U_CHAR        osmp;										// OSM objects
		U_CHAR				rfu1;
		U_CHAR				rfu2;
		U_CHAR				rfu3;										// 
		Queue<C3DPart> osmQ[OSM_LAYER_SIZE];	// osm Queue
    //---------------------------------------------------------------
    SPosition    mPos;                        // Center position
    CVector      sRad;												// Radius
    //---------------------------------------------------------------
    double       MiniH;                       // Minimum Height
    double       MaxiH;                       // Maximum Height
    CTextureDef  Tex[TC_TEXSUPERNBR];         // List of Tetxures
    D3_VERTEX    Tour[TC_SPTBORDNBR + 2];     // SP contour
    //------Methods ---------------------------------------------
    CSuperTile();
   ~CSuperTile();
	  bool					Update();	
    bool          NeedMedResolution(float rd);
    bool          NeedHigResolution(float rd);
		void					Refresh3D();
		//-----------------------------------------------------------
		void					LoadVBO();
		//-----------------------------------------------------------
		void          MakeOSMPart(CShared3DTex *ref, char L,int nv,GN_VTAB *S);
		void					AddToPack(OSM_Object *obj);
		//-----------------------------------------------------------
		void					Reallocate(char opt);
    void					AllocateVertices(char opt);
    void          RazNames();
    void          BuildBorder(C_QGT *qt,U_INT No);
		bool					IsTextured(char opt);
    //-----------------------------------------------------------
    void          GetLine(CListBox *box);
		void					ClearOSM(char lay);
		void					FlushOSM();
    //-----------------------------------------------------------
		void					DrawOuterSuperTile();
    void          DrawInnerSuperTile();
    void          DrawTour();
    int           Draw3D(U_CHAR tod);
    void          Add3DObject(CWobj *obj,char t); 
		void					BindVBO();
		//-----------------------------------------------------------
		void					TraceEnd();
		void					TraceRealloc(CmQUAD *qd);
    //-----------------------------------------------------------
    inline CTextureDef    *GetTexDesc(int nd) {return (Tex + nd);}
    //-----------------------------------------------------------
    inline void   SetVisibility(char v)   {visible = v;}
    inline char   Visibility()            {return visible;}
    inline bool   IsOutside()             {return (sta3D == SUP3D_OUTSIDE);}
    inline float  GetTrueDistance()       {return dEye;}
    inline float  GetAlpha()              {return alpha;}
		//--- Vertex buffer manageement -----------------------------
		inline TC_GTAB* GetVertexTable()			{return vBUF;}
		inline SPosition GeoPosition()				{return mPos;}
    //-----------------------------------------------------------
		inline int  GetNumber()	{return NoSP;}
    inline U_CHAR GetLOD()  {return LOD;}
    inline void zrSwap()  {swap = 0;}
    inline bool IsReady() {return (State == TC_TEX_RDY);}
    inline bool NeedLOD() {return (State == TC_TEX_LOD);}
    inline bool NeedOBJ() {return (State == TC_TEX_OBJ);}
    inline bool NeedSWP() {return (State == TC_TEX_POP);}
		inline bool InFarQ()	{return (State == TC_TEX_INQ);}
    //-----------------------------------------------------
    inline void WantRDY() {State  = TC_TEX_RDY; }
    inline void WantLOD() {State  = TC_TEX_LOD; }
    inline void WantOBJ() {State  = TC_TEX_OBJ; }
    inline void WantPOP() {State  = TC_TEX_POP; }
		inline void WantINQ() {State  = TC_TEX_INQ; }
    inline void SetState(U_CHAR s)  {State = s;}
    //-----------------------------------------------------
       //-----------------------------------------------------
		inline void RenderOUT()	{Rend = &CSuperTile::DrawOuterSuperTile;}
		inline void RenderINR()	{Rend = &CSuperTile::DrawInnerSuperTile;}
		inline void Draw()		{(this->*Rend)();}
		//-----------------------------------------------------
		inline CSuperTile *Next()				{return next;}
		inline void Next(CSuperTile *s)	{next = s;}
		inline C_QGT      *GetQGT()			{return qgt;}
		//------------------------------------------------------
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
    void        PutEnd(CSuperTile *sp);
    CSuperTile  *Pop();
    CSuperTile  *Detach(CSuperTile *sp);
    //-------in lines ------------------------------------
    CSuperTile  *GetFirst()               {Prev = 0;  return First;}
    CSuperTile  *GetNext(CSuperTile *sp)  {Prev = sp; return sp->Next();}
    bool        IsEmpty()                 {return (First == 0);}
    bool        NotEmpty()                {return (First != 0);}
    bool        NotFull()                 {return (NbSP != 64);}
    int         GetNumberItem()           {return NbSP;}
    void        Lock()                    {pthread_mutex_lock (&mux);}
    void        Unlock()                  {pthread_mutex_unlock (&mux);}
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
  void        SetArray(CmQUAD *cp,U_SHORT dim);
  void        SetParameters(CVertex *ct,U_CHAR l);
  int         CountVertices();
	int					NbrVerticesInTile();
	int					InitIndices(CSuperTile *s,char opt);
  void        GetTileIndices(U_INT &tx,U_INT &tz);
  CmQUAD     *Locate2D(CVector &p,C_QGT *qgt);
  U_INT       GetSuperNo();
	U_INT				GetTileNo();
	U_INT				WorldTileKey();
	//--- Vertex buffer management --------------------------
	void				Clean();
	int 				InitVTAB(TC_GTAB *vbo,int d, U_CHAR r);
	int					InitVertexCoord(TC_GTAB *vbo,float *txt);
	void				RefreshVTAB(CSuperTile *sp,U_CHAR res);
	void				RefreshVertexCoord(CSuperTile *sp,float *txt);
	//-------------------------------------------------------
	void				StoreIn(D3_VERTEX *d, CVertex *v);
	void				AssignGeoCOORD(TC_GTAB *tab, CVertex *v);
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
	void				DrawIND();			// Draw with indices
	void				DrawVBO();			// Draw with VBO
	void				CheckTour();
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
	inline U_INT    GetTileVX()			{return (Center.xKey &  TC_1024MOD);}
	inline U_INT    GetTileVZ()			{return (Center.zKey &  TC_1024MOD);}
	inline int      GetNbrVTX()     {return nvtx;}
  //--------------------------------------------------------------------
	inline void SetGTAB(TC_GTAB *t)				{vTab = t;}
  inline void SetGroundType(U_CHAR t)   {Center.gType = t;}
  inline void MarkAsGround()            {Flag |= QUAD_GND;}
  inline void ClearGround()             {Flag &= (-1 - QUAD_GND);}
  inline bool IsAptGround()             {return ((Flag & QUAD_GND) != 0);}
  //--------------------------------------------------------------------
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
//============================================================================
//  Ground tile: Define text descriptor for a detail tile that
//  if part of an airport ground
//============================================================================
class CGroundTile {
  //--- Attributes --------------------------------------------
  U_INT         ax;                       //  Tile ident X
  U_INT         az;                       //  Tile ident Z
	C_QGT       *qgt;												//  Tile QGT
  CTextureDef *txn;                       //  Tile descriptor
	CmQUAD      *quad;											// Quad
	CSuperTile  *sup;												// Super tile
	int          dim;												// Quad number
	//------------------------------------------------------------
	GLint        sIND[QUAD_RESOLUTION];			//   Start indices
	GLint				 nIND[QUAD_RESOLUTION];			//   Count of indices
  //--- Methods ------------------------------------------------
public:
  CGroundTile(U_INT x,U_INT z);
  //------------------------------------------------------------
	void		Free();
  int		  StoreData(CTextureDef *d);
	int			TransposeTile(TC_GTAB *d,int s, SPosition *o);
	void		RelocateVertices(TC_GTAB *vbo, int nbv, SPosition *org);
	//------------------------------------------------------------
	char		Visibility()	{return sup->Visibility();}
	//--- VBO management -----------------------------------------
	int 		GetNbrVTX();
	int 		DrawGround();
	//------------------------------------------------------------
	inline void					StoreSup(CSuperTile *s) {sup = s;}
  inline CmQUAD      *GetQUAD()			{return quad;}
  inline U_INT        GetAX()				{return ax;}
  inline U_INT        GetAZ()				{return az;}
	//------------------------------------------------------------
	inline	void	SetQGT(C_QGT *q)			{qgt = q;}
	inline	void	SetTXD(CTextureDef *t){txn = t;} 
	inline	void	SetQUAD(CmQUAD *q)		{quad = q;}
};

//=============================================================================
//  CLASS Quarter Tile descriptor
//  C_QGT
//=============================================================================
//--------------------------------------------------------------
class C_QGT: public QGTHead {
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
  U_INT         rKey;                     // Request parameter
  SPosition     Scene;                    // Mid position for scenery
	//--- OSM interface -------------------------------------------
	U_INT					sqlNO;										// Current Database
	SQL_DB       *sqlDB;										// Current Descriptor
	std::vector<SQL_DB*> osmDB;							// Database Queue
	//--- Compressed texture database ----------------------------
	SQL_DB			 *ctxDB;										// Pointer to descriptor
  //--------Option parameters--------------------------------------
  U_CHAR        rCode;                    // Request code
  U_CHAR        visb;                     // is visible
	U_CHAR        strn;											// Skip trn
	U_CHAR				rfux;											// reserved futur use
  //--------Mux for Step protection -----------------------------
  pthread_mutex_t	stMux;                  // State lock
  U_CHAR        qSTAT;                    // Quad available when 0
  U_CHAR        Step;                     // QGT Major steps
  U_CHAR        nStp;                     // Next Step
  U_CHAR        dead;                     // Dead mark
  //-------------------------------------------------------------
  double        wLon;                     // West absolute longitude
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
	int						nbO;											// Object remainging in Supertiles
  C3Dworld      w3D;                      // Related world objects
  //---------Texture Parameters --------------------------------
  float         dto;                      // Texture origin
  float         dtu;                      // Texture unit
  //---------Center coordinates --------------------------------
  CVector       Bound;                    // Bounding
  //---------Option indicator ----------------------------------
  U_CHAR          tr;                     // Trace indicator
  U_CHAR          vbu;                    // use VBO
	U_CHAR					elv;										// Has base elevation
  //---------Demux counter--------------------------------------
  U_CHAR        demux;
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
	void		GetTrace(int *t,int *x,int *z);
  //-------------------------------------------------------------
  void TimeSlice(float dT);
  //---------Helpers  ---------------------------------
  U_CHAR      GetReqCode()        {return rCode;}
  U_INT       GetReqKey()         {return rKey;}
  void        SetQTR(C_QTR *qtf)  {qtr  = qtf;}
  double      GetDlon() {return  dLon;}
  double      GetDlat() {return  (nLat - sLat);}
  U_INT       GetXkey() {return xKey;}
  U_INT       GetZkey() {return zKey;}
  U_INT       FullKey() {return qKey;}
  bool     NotVisible() {return (visb == 0);}
  bool        NoQuad()  {return (1 == qSTAT);}
  bool        HasQuad() {return (0 == qSTAT);}
	bool				InLoad()	{return LoadQ.NotEmpty();}
	void				StoreCompressedTexDB(SQL_DB *p) {ctxDB = p;}
	//-----------------------------------------------------
	void				IndElevation()	{elv = 1;}
	bool				HasElevation()	{return elv != 0;}
  //--------3D Object -----------------------------------
  int         GetNOBJ() {return w3D.GetNOBJ();}
  C3Dworld   *Get3DW()  {return &w3D;}
  //-------Create vertex in line ------------------------
  CVertex    *CreateVertex(U_INT vx,U_INT vz)
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
	int					Step3DO();					// Load 3D objects
	//---------State management ----------------------------------
  void				LockState()  {pthread_mutex_lock   (&stMux);}
  void				UnLockState(){pthread_mutex_unlock (&stMux);}
  bool				NotReady()   {return (Step != TC_QT_RDY);}
  bool				IsReady()    {return (Step == TC_QT_RDY);}
  bool				NotAlive()   {return (dead == 1);}
  void				SetStep(U_CHAR s);
  void				PostIO();
  //---------Queuing management --------------------------------
  CSuperTile *PopLoad();
  void        EnterNearQ(CSuperTile *sp);
  //---------Position routines ---------------------------------
  bool        GetTileIndices(SPosition &pos,short &tx, short &tz);
  bool        GetTileIndices(GroundSpot &gns);
	//-------------------------------------------------------------
	void				RelativeToBase(CVector &v)
		{	v.x -= wLon;													// Remove west longitude
			v.y -= sLat;													// Remove south latitude
		}
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
	//--- OSM management ----------------------------------------
	void        ExtendOSMPart(char No,char dir, char *ntx, char layer, int nv, GN_VTAB  *src);
  void				AddOsmBase(SQL_DB *db)	{ osmDB.push_back(db); }
	void        StartOSMload(int sNo);
	void				FlushOSM();
  //----------Mesh Management ------------------------------------
	SPosition		GetBase();
	int         CenterTile(CVertex *sw,CVertex *nw,CVertex *ne,CVertex *se);
  int         DivideTile(U_INT div,CVertex *sw,CVertex *nw,CVertex *ne,CVertex *se);
  CVertex    *GetEastVertex (CVertex *n,U_INT vx, U_INT vz,int op);
  CVertex    *GetNorthVertex(CVertex *n,U_INT vx, U_INT vz,int op);
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
	void						InitSuperTiles();
  int							PutOutside();
  int							UpdateInnerCircle();
  void						FlushTextures();
  CTextureDef		 *GetTexDescriptor(U_INT tx,U_INT tz);
  CTextureDef    *GetTexList(U_INT No)    {return Super[No].Tex;}
	CSuperTile     *GetSuperTile(int tx,int tz);
	bool						AllTextured(char opt);
	//-----------------------------------------------------------
  CSuperTile     *GetSuperTile(U_INT No) { return (No > 63)?(0):(&Super[No]); }
	//--- Delete resources --------------------------------------
  int         FreeQuad(CmQUAD *cp);
  int         FreeVertices(CmQUAD *cp);
  int         DeleteQTR();
  void        FreeHMidVertex(CVertex *vt);
  void        FreeVMidVertex(CVertex *vt);
  void        FreeAllVertices();
  void        UnlinkVertex(CVertex *vt);
  //--- Drawing ----------------------------------------------
  void        DrawSuperMesh(CCamera *cam, char d);
  void        DrawWiresSuperTile(CSuperTile *sp);
	void				DrawContour();
  //--- Coast management -------------------------------------
  U_INT       GetSeaKEY(int k)        {return seaKEY[k];}
  U_INT       GetSeaREQ(int k)        {return seaREQ[k];}
  void        SetSeaREQ(int k,U_INT r){seaKEY[k] = r;}
  //--- 3D object management ---------------------------------
  SPosition  *GetMidPoint()       {return &mPoint;}
  double      GetMidLat()         {return  mPoint.lat;}
  double      GetMidLon()         {return  mPoint.lon;}
  double      GetMidAlt()         {return  mPoint.alt;}
  double      GetWesLon()         {return  wLon;}
  double      GetNorLat()         {return  nLat;}
  double      GetSudLat()         {return  sLat;}
  //---		Management  -------------------------------------
  void        SetMETAR(Tag m)     {Metar = m;}
  Tag         GetMETAR()          {return Metar;}
  bool        DifMETAR(Tag m)     {return (m != Metar);}
  //---------------------------------------------------------------
  void        IncCloud(U_CHAR t)  {cloud[t]++;}
  void        DecCloud(U_CHAR t)  {cloud[t]--;}
  inline      U_INT       NbCloud(U_CHAR t)   {return cloud[t];}
  //---------------------------------------------------------------
  void        CheckW3D()          {w3D.Check();}
	//--- Texture management ----------------------------------------
	char				GetCompTextureInd()	{return (ctxDB != 0);}
	SQL_DB     *GetCompTextureDBE()	{return ctxDB;}
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
//  Terrain Cache manager
//=============================================================================
class TCacheMGR: public CExecutable {
  //---------Attributes ----------------------------------------
  float       dTime;
  float       eTime;                        // Elapse time     
  U_INT       rKEY;                         // Reference QGT (aircraft position)
  U_INT       nKEY;                         // New Key
  U_SHORT     qRDY;                         // Number qgt waiting ready
  //------Terrain parameters ------------------------------------
  U_CHAR      stop;                         // stop sql
  U_CHAR      wire;                         // QGT step
  U_CHAR      trfu;													// Terrian indicator
	U_CHAR        strn;											  // Skip trn
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
  double      xFactor;                      // X Expension factor
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
  U_CHAR      rfu2;                         // Display accounting
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
  U_INT       NbSEA;                        // Number of Coast Files
  U_INT       NbREG;                        // Number of regions
  U_INT       NbTEX;                        // Number of textures
	U_INT				DrSUP;												// Numer of SUP drawed
	U_INT				DrQGT;												// Number of QGT drawed
  U_INT       tr;                           // Trace option
  U_INT       td;                           // Thread debug
  float       aTime;                        // Accounting timer
  //---------Airport management ---------------------------------
  CAirportMgr  *aptMGR;                     // Airport   Manager
  C3DMgr       *objMGR;                     // 3D object Manager
  CTextureWard *txw;                        // Texture   Manager
  //---------Thread management-----------------------------------
	U_CHAR					tnbr;											// Thread number
  pthread_t       t1id;											// Thread identity
	pthread_t       t2id;											// Thread identity
  pthread_cond_t  thCond;                   // Condition variable
  pthread_mutex_t	thMux[2];									// Condition Mutex
  U_INT           thSIG;                    // Number of signal posted
  U_INT           thRCV;                    // Number of Signal received
  U_INT           thRUN;                    // Runing signal
	char            t1OK;
	char						t2OK;
  //---------TRN File name -------------------------------------
  char        trnName[64];                  // TRN file name
	//--- Activity management ------------------------------------
	U_CHAR				ldevn;
	U_CHAR				activity;
	short					aframe;
	//---------Methods -------------------------------------------
public:
  TCacheMGR();
 ~TCacheMGR();
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
  U_CHAR      BuildName(CTextureDef *txn,U_SHORT nx,U_SHORT nz);
  void        FormatName(C_QGT *qt,CSuperTile *sp);
  int         GetTransitionMask(C_QGT *qt,CSuperTile *sp);
  U_CHAR      GetLandTransition(char *hex);
  void        SetTransitionMask(C_QGT *qgt,CTextureDef *txn);
  char        GetTileType(char tp,int dir);
  int         GetCoastMark(int inc,U_INT ax,U_INT az);
  float       AircraftFeetDistance(SPosition &pos);
	void				SetShadowMatrix( float mat[16],float lp[4]);
	char        GetThreadNumber()		{return tnbr++;}
	void				ArmActivity(char evn);
	bool				StillLoading(char evn);
  //----------Terrain management -------------------------------
  double      GetGroundAt(GroundSpot &gns);
	void  			GetGroundAt(GroundSpot &gns,SPosition &pos);
	void				UpdateGroundPlane();
	GroundSpot *GetSpot()		{return &Spot;}
	bool				SPotReady()	{return Spot.Valid();}
	bool				TerrainStable(char opt);
	//----------TIME management ----------------------------------
  void        UpdateTOD();
  U_CHAR      SetLuminosity();
  //---------Diag and Tests ------------------------------------
  bool        PlaneInTile(U_INT ax,U_INT az);
  //---------AIRPORT MANAGEMENT --------------------------------
  CmQUAD      *GetTileQuad(SPosition pos);
  CmQUAD      *GetTileQuad(U_INT ax,U_INT az);
  CTextureDef *GetTexDescriptor();
  CTextureDef *GetTexDescriptor(C_QGT *qtg,U_INT ax,U_INT az);
	void				FillGroundTile(CGroundTile *gnt);
  //--------METAR MANAGEMENT -------------------------------------
  void         AssignMetar(CMeteoArea *ma);
  //---------Helper --------------------------------------------
  void        MeshError(U_LONG xv,U_LONG zv);
  U_INT       GetQTRindex(U_INT cx,U_INT cz);         // Name from QGT indices
  //---------Cache update routines -----------------------------
  int         TimeSlice(float dT,U_INT FrNo);		      // Cache update
  bool        IsaNewKey(U_INT key);                   // Check for new key
  bool        IsaActKey(U_INT key);                   // Check for old key
  void        GetRange(U_INT cz,U_SHORT &up,U_SHORT &dn);
  C_QGT      *LocateQGT(QGT_DIR *itm,U_SHORT cx,U_SHORT cz);
  C_QGT      *GetQGT(U_SHORT cx, U_SHORT cz);
  C_QGT      *GetQGT(SPosition &pos);
	C_QGT      *GetQGT(U_INT key);
  bool        SetQGT(GroundSpot &gns);
  CVertex    *GetQgtCorner(C_QGT *qgt,QGT_DIR *tab,U_SHORT cn);
  int         FreeTheQGT(C_QGT *qt);
	int					EndOfQGT(C_QGT *qgt);
  int         OneAction();                            // One Action per QGT
  int         RefreshCache();
  int         UpdateQGTs(float dT);
  void        GetObjLines(CListBox *box);             // 3D object lines
  //---------STEP ROUTINES --------------------------------------
  void        CreateQGT(U_SHORT cx, U_SHORT cz);      // STEP 0
  //---------Inline ---------------------------------------------
	void	ProbeBB(CFuiCanva *c,int n) {;}
  CTextureWard *GetTexWard() {return txw;}
  void   InActQ (C_QGT *qgt) {ActQ.PutLast(qgt);}
	void	 HeadofQ(C_QGT *qgt) {ActQ.PutHead(qgt);}
  //-------------------------------------------------------------
	void IncDSP()							{DrSUP++;}
	void IncRDY(short n)			{qRDY += n;}
  bool MeshReady()          {return (qRDY == 0);}
	bool MeshBusy()						{return (qRDY != 0);}
  bool TelePorting()        {return (Tele == 1);}
  //---------Time of day management -----------------------------
  bool   IsDay()            {return (cTod == 'D');}
  bool   IsNight()          {return (cTod == 'N');}
  bool   IsSun()            {return (sInd == 'S');}
  bool   IsDawn()           {return (lumn > 0.10);}
  float  Elapse()           {return eTime;}
  float  GetLuminosity()    {return lumn;}
  CVector *SunPosition()    {return &sunP;}
	bool    RunThread()				{return (stop == 0);}	
  //--------Return scale parameters ------------------------------
  SVector *GetScale()       {return &scale;}
  //--------Terrain Parameters -----------------------------------
	CSuperTile *GetSpotSuperTile()	{return Spot.GetSuperTile();}
	//--------------------------------------------------------------
	int      Locate3DO(C_QGT *q)	{return objMGR->LocateObjects(q);}
	bool     Teleporting()				{return (Tele != 0);}
  double   GetHorizon()					{return hLine;}
  float    GetMagnecticNorth()	{return magDV;}
  float    GetGroundAltitude()	{return Spot.alt;}
	double	 GetFeetAGL()					{return Spot.agl;}
  U_CHAR   GetGroundType()			{return Spot.Type;}
  double   GetInflation()				{return xFactor;}
  double   GetReduction()				{return rFactor;}
  TRANS2D  GetTranslation()			{return aTran;}
	float*   GetGroundPlane()			{return gplan;}
	U_CHAR	 GetTRNoption()			  {return strn;}
	void     GetGroundNormal(CVector &v) {v = Spot.gNM;}
  //--------Aircraft Parameters -----------------------------------
	bool     QGTplane(C_QGT *q)	{return Spot.qgt == q;}
  SPosition  *PlaneArcsPos()      {return &globals->geop;}
  CVector    *PlaneFeetPos()      {return &geow;}
  double      GetCenterLongitude(){return globals->geop.lon;}
  double      GetCenterLatitude() {return globals->geop.lat;}
  C_QGT      *GetSpotQGT()				{return Spot.qgt;}
  bool        SameQuad(CmQUAD *q)	{return (Spot.SameSpot(q));}
  //--------------------------------------------------------------
  bool   HiResPermited()     {return (HiRes!= 0);}
  char   GetTrace()          {return (tr)?(1):(0);}
  char   GetDebug()          {return (td)?(1):(0);}
	char   GetFactEV()				 {return FactEV;}
  float  Time()              {return dTime;}
  //--------Drawing parameters -----------------------------------
  float  GetMedDist()        {return medRAD;}
  float  GetHigDist()        {return higRAD;}
  float *GetFogColor()       {return fogC;}
  float *GetFarColor()       {return fogC;}
  U_CHAR GetLastEVindex()    {return LastEV;}
  float *GetNightEmission()  {return NitColor;}
  float *GetDeftEmission()   {return EmsColor;}
  //---------------------------------------------------------------
  CListBox *GetTerraBox()    {return terBOX;}
  //---------Aircraft position ------------------------------------
  void        GetAbsoluteIndices(U_INT &ax,U_INT &az);
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
  //---------Drawing primitives ----------------------------------
  void        Draw();
	void				Draw3DObjects();
	void        DrawHorizon();
  void        ScanCoast(SSurface *sf);
  void        ScanCoastSQL(SSurface *sf);
  void        SetSkyFog(float dens);
  //---OSM Management -------------------------------------------
	void				AddToPack(OSM_Object *obj);
	std::map<U_INT,C_QGT*> &GetQgtList()	{return qgtMAP;}
  //----------Misc items  ---------------------------------------
  void        DrawSun();
  void        GetSunPosition (void);
  void        BuildShadowMap (void);
  void        SetPPM(double rat,short wd,short ht);
  double GetXPPM()   {return ppM.x;}
  double GetYPPM()   {return ppM.y;}
  short  GetVmapWD() {return sDIM.x;}
  short  GetVmapHT() {return sDIM.y;}
  //----------File access --------------------------------------
  CTerraFile *GetTerraFile(){return tFIL;}
  int         StartIO(C_QGT *qgt, U_CHAR ns);
  void        LockQTR()   {pthread_mutex_lock   (&qtrMux);}
  void        UnLockQTR() {pthread_mutex_unlock (&qtrMux);}
  void        GoThread()  {pthread_cond_signal(&thCond); }
  void        RequestTextureLoad(C_QGT *qgt);   // Request Textures
  void        GetQTRfile(C_QGT *qgt,TCacheMGR *tcm);
  int         RequestELV(C_QGT *qgt);
  C_QGT      *PopFileREQ()      {return FilQ.Pop();}
    //----------Thread parameters --------------------------------
  void ThreadRuns()      {thRUN = 1;}
  void ThreadStop()      {thRUN = 0;}
	//------------------------------------------------------------
  C_QGT      *PopLoadTEX()						{return LodQ.Pop();}
  pthread_cond_t  *GetTHcond()			  {return &thCond;}
  pthread_mutex_t *GetaMux(char n)    {return &thMux[n];}
  //----------Test and debug -----------------------------------
  GLUquadricObj  *GetSphere()   {return sphere;}
  void        CheckW3D();
  //----------Teleport -----------------------------------------
	void				MoveRabbit(SPosition &dst);
  void        CheckTeleport();
  void        Teleport(SPosition *P, SVector *O);
	void				FlushOSM();
  //----------Statistics ---------------------------------------
  void        GetStats(CFuiCanva *cnv);
};

//===============END OF THIS FILE =============================================
#endif  // TERRAINCACHE_H
