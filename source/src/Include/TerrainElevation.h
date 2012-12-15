/*
 * Utility.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
 *                2008 Jean Sabatier
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
#ifndef TERRAINELEVATION_H
#define TERRAINELEVATION_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//========================================================================================
#include "../Include/Globals.h"
#include "../Include/FlyLegacy.h"
#include "../Include/TerrainCache.h"
//========================================================================================
//=========================================================================
//  QTR FILE HEADER
//=========================================================================
struct QTR_HEADER{
  U_SHORT  nNodes;                                  // Number of nodes
  U_LONG   magic; 
  U_LONG   width;                                   // x Number of Detail Tiles
  U_LONG  height;                                   // z Number of Detail Tiles
  U_LONG  nBytes;                                   // Raster size
  short   elev[4];
};
//=========================================================================
//  QTR NODE DESCRIPTOR
//=========================================================================
struct QTR_NODE_HD  {
  U_CHAR  Type;                                       // Node type
  short   cElv;                                       // Center elevation
  };
//=========================================================================
//  QTR BRANCH DESCRIPTOR
//=========================================================================
struct QTR_BRANCH {
  short data[4];
};
//=========================================================================
//  QTR ABSOLUTE/RELATIVE RASTER NODE DESCRIPTOR
//=========================================================================
struct QTR_RASTER {
  U_LONG  index;
  U_LONG  rfu;
};
//=========================================================================
//  REGION DESCRIPTOR
//=========================================================================
struct QTR_REGION{
    QTR_REGION *Next;                               // Next Region
    U_SHORT     node;                               // Node number
    U_SHORT     Type;                               // Node type
    U_SHORT     cx;                                 // Base Detail Tile cx
    U_SHORT     cz;                                 // Base Detail Tile cz
    U_SHORT     dim;                                // Region Size
    long       elev;                                // elevation or index
  } ;
//-----------------------------------------------------------------
#define TC_TYPE_BR 0x00             // Branch type
#define TC_TYPE_LF 0x01             // Leaf node
#define TC_TYPE_AR 0x03             // Absolute raster
#define TC_TYPE_RR 0x85             // Relative raster
//----------------------------------------------------------------
//========================================================================================
// Class CQregion to hold list of QTR_REGION 
//========================================================================================
class CQregion {
  //---------Data members ---------------------------------------
  QTR_REGION  *First;
  QTR_REGION  *Last;
  //--------Methods ---------------------------------------------
public:
  CQregion();                               // Constructor
 ~CQregion();                               // Destructor
  void        PutLast(QTR_REGION *reg);
  QTR_REGION *Pop();
  void        Insert(QTR_REGION *reg);
  //--------Access to item --------------------------------------
  inline QTR_REGION *GetFirst() {return First;}
  };
//========================================================================================
// Class CQ_HDTL to hold list of TRN_HDTL 
//========================================================================================
class CQ_HDTL  {
  //--------Data Members ----------------------------------------
  U_INT      Nbre;                        // Number of items
  TRN_HDTL  *First;     
  TRN_HDTL  *Last;
  //--------Methods ---------------------------------------------
public:
  CQ_HDTL();
 ~CQ_HDTL();                              // Destructor
  void      PutLast(TRN_HDTL *itm);       // Insert at end
  TRN_HDTL *Pop();                        // Pop an item
};
//========================================================================================
//  CLASS C_QTR for default elevation
//  
//========================================================================================
class C_QTR {
  PODFILE *pod;
  //----ATTRIBUTES --------------------------------------------------
  U_INT       key;                                  // File identity
  U_SHORT     use;                                  // Number of QGT using this file
  CQregion    qREG;                                 // List of regions in formation
  CQregion    qQGT[16][16];                         // Array of region per QGT
  //---Files data --------------------------------------------------
  QTR_HEADER  head;                                 // File header
  QTR_NODE_HD nHD;                                  // Node Header
  QTR_BRANCH  nBR;                                  // Node Branch
  QTR_RASTER  aRS;                                  // Raster node
  char *raster;                                     // Raster data    
  char  name[64];                                   // File name
  //-----------------------------------------------------------------
  char    tNode;                                    // Node Type
  U_SHORT cNode;                                    // Current node
  U_SHORT rNode;                                    // Region node
  U_INT   btx;                                      // Base detail tile index  
  U_INT   btz;                                      // Base Detail Tile index
  //-----------------------------------------------------------------
  TCacheMGR *tcm;                                   // Terrain Cache manager
  U_INT   tr;                                       // Trace option
  //----------Accounting --------------------------------------------
  U_INT NbReg;                                      // Number of regions
  //---------Elevations routines ------------------------------------
  C_QGT   *qgt;                                     // Current QGT object
  //----Methods -----------------------------------------------------
public:
  C_QTR(U_INT idn,TCacheMGR *tm);
 ~C_QTR();                                          // Destructor
  bool    NoMoreUsed();
  void    Abort(char *reason);
  void    LoadFile(U_INT idn);
  void    InitRegion(U_INT idn,U_SHORT dim);
  void    DecodeNode(PODFILE *p);
  void    CutHeadRegion();
  void    StoreElevation();
  void    ClipRegion(QTR_REGION *reg);
  void    ClipRegionCZ(QTR_REGION *reg,U_SHORT dim);
  void    EnterMatrix(QTR_REGION *reg);
  void    RasterRegion(U_SHORT type);
  QTR_REGION *NewRegion(U_SHORT cx,U_SHORT cz);
  //-----------------------------------------------------------------
  inline void   IncUser() {use++;}
  inline U_INT  GetKey()  {return key;}
  //-------For QGT --------------------------------------------------
  int     SetQTRElevations(C_QGT *qgt);
  void    RegionElevation(QTR_REGION *reg);
  void    RasterElevation(QTR_REGION *reg);
  void    RelativeElevation(QTR_REGION *reg);
  void    GetQGTindices(int xk,int zk,U_SHORT *dx,U_SHORT *dz);
  //------For export -------------------------------------------------
  int     PopRegionInfo(int rx,int rz,REGION_REC &inf);
  void    RegionELinfo(QTR_REGION *reg,REGION_REC &inf);
  void    RegionABinfo(QTR_REGION *reg,REGION_REC &inf);
  void    RegionRRinfo(QTR_REGION *reg,REGION_REC &inf);  
  //------------------------------------------------------------------
};
//===================================================================================
//  CLASS C_STILE to decode a SUPER TILE FROM TRN FILE
//  NOTE: As the Supertile subdivision in 4*4 Detail Tiles is enforced by Legacy
//  Then All reservations are of this fixed size.
//===================================================================================
class C_STile: public CStreamObject {
  friend class C_TRN;
public:
  //---------ATTRIBUTES -------------------------------------------------
	U_INT					ax;													// Absolute X indice of SW Det
	U_INT					az;													// Absolute Z indice of SW Det
	//----------------------------------------------------------------------
	U_INT					cx;													// Current DET x index
	U_INT					cz;													// Current DET z index
	//----------------------------------------------------------------------	
	U_INT					qKey;												// QGT key
	//---------------------------------------------------------------------
  char         tr;                          // Trace indicator
	U_CHAR			 comp;												// Texture type
	//--- Parameters -------------------------------------------------------
	short         side;												// elevation side
  U_INT         No;                         // Super Tile Number
  short         Type;                       // Type
  short         Dim;                        // Dimension
  C_QGT        *qgt;                        // Mother QGT
	CTextureDef  *xtmp;                       // Texture list for export
  CTextureDef  *DayList;                    // List of Day textures
  float         elev[5][5];                 // Default elevation matrix
  CQ_HDTL      qHDTL;                       // HDTL queue
  CTextureDef *qList;                       // Texture list from QGT
  C_TRN        *trn;                        // Parent TRN for statistic
  //---------METHODS ----------------------------------------------------
public:
  C_STile();                                // Constructor
 ~C_STile();                                // Destructor
  void    Abort(char *msg);                 // TRN error
	int     Read (CStreamFile *stf, Tag tag);  // Read STILE parameters
  void    SetSupElevation(U_INT sx, U_INT sz, C_QGT *qgt);
  void    GetHdltElevations(U_INT sx,U_INT sz,C_QGT *qgt);
  void    FlagDayTexture(CTextureDef *txn);
	void		PackTexName(char *tn);
	//---Helper to pack 16 flags for SQL ---------------------------------
	U_INT		GetTextureType(U_CHAR t);
	U_INT		GetTextureOrg();
	U_INT   GetNite();
  //--------inline -----------------------------------------------------
  void    SetParam(C_TRN *trn,U_INT t)			{tr = t; this->trn = trn;}
  void    SetTList(CTextureDef *l,U_CHAR c) {qList = l; comp = c; }
	void		SetKey(U_INT k)										{qKey	= k;}
	//--------------------------------------------------------------------
	inline void				SetAX(U_INT x)				{ax = x;}
	inline void				SetAZ(U_INT z)				{az = z;}
	//--------------------------------------------------------------------
	inline int				GetKey()							{return qKey;}
	inline int				GetNo()								{return No;}
	inline int				GetDim()							{return Dim;}
	inline int				GetSide()							{return side;}
	inline float     *GetElvArray()					{return *elev;}
	inline TRN_HDTL  *PopDetail()						{return qHDTL.Pop();}
	//--------------------------------------------------------------------
	inline CTextureDef *GetTexDef()					{return xtmp;}
	//--- return a local texture list ------------------------------------
	CTextureDef* GetTexList() {xtmp = new CTextureDef[TC_TEXSUPERNBR]; return xtmp;}
};
//===================================================================================
//  CLASS CTxtDecoder  Temprorary class to decode <txtl> tag
//===================================================================================
class CTxtDecoder: public CStreamObject {
   friend class C_STile;
  //---------ATTRIBUTES ------------------------------------------------
	U_CHAR type;																			// Comprssion indicator
  U_INT nbx;                                        // Number of textures
  CTextureDef *txd;                                 // List of texture name
  //---------METHODS ---------------------------------------------------
public:
  CTxtDecoder(char t);
	int     Read (CStreamFile *stf, Tag tag);  // Read <txtl> parameters
  U_INT   NormeName(char *txt,CTextureDef *txd);
};
//===================================================================================
//  CLASS ChdtlDecoder  Temprorary class to decode <hdtl> tag
//===================================================================================
class ChdtlDecoder : public CStreamObject {
  friend class C_STile;
  //--------Data Members ----------------------------------------------
	short			 sup;
  TRN_HDTL  *hd;
  short      tr;                              // Trace indicator
  C_TRN     *trn;                             // Parent TRN file for report
  //--------Methods ---------------------------------------------------
public:
  ChdtlDecoder(C_TRN *trn,short No,U_INT tr); // Constructor
  void      Abort(char *msg);                 // Abort process
	int       Read (CStreamFile *stf, Tag tag);  // Read <hdtl> parameters
  void      GetHDTL(int dim);                 // Allocate a TRN_HDTL struct
	//--------------------------------------------------------------------
};
//===================================================================================
//  CLASS C_TRN to DECODE a TRN FILE
//===================================================================================
class  C_TRN: public CStreamObject {
  friend class TCacheMGR;
	//---------------------------------------------------------------------
	U_INT		ax;																					// SW corner of QGT
	U_INT		az;																					// SW corner of QGT
	//---------------------------------------------------------------------
	U_INT		cx;																					// Current DT
	U_INT		cz;																					// Current dt
  //---------ATTRIBUTES -------------------------------------------------
  U_CHAR   tr;                                        // Trace indicator
	U_CHAR	type;																				// Compression indicator
  C_QGT  *qgt;                                        // Quarter Global Tile
  C_STile aST[TC_SUPERT_PER_QGT][TC_SUPERT_PER_QGT];  // Array of Supertiles
  //---------Accounting -------------------------------------------------
  U_INT   nHDTL;                                      // Number of <hdtl> tags
  U_INT   nDETS;                                      // Number of processed tags
	//---------------------------------------------------------------------
	char		impr;																				// Real or export
  //---------------------------------------------------------------------
public:
  C_TRN(char *fn,C_QGT *qgt,U_INT tr);                         // TRN File 
 ~C_TRN();                                            // TRN destructor
	void		ReadSUPR(CStreamFile *stf);	 								// Read 'supr' tag
  void    Abort(char *msg);                           // ABORT for errors
	int     Read (CStreamFile *stf, Tag tag);           // Read Parameters
  void    ReadFinished ();                            // Read finished
  void    SetTRNdefault(C_QGT *qgt);                  // Default elevations
  void    GetHdtlElevations(C_QGT *qgt);              // Super Tile elevations
  //---------------------------------------------------------------------
  inline void IncHDTL()   {nHDTL++;}                  // Increment HDTL statement
  inline void IncDETS()   {nDETS++;}                  // Increment number of processed tags
	inline void ForExport()	{impr = 1;}									// Export mode
	inline char GetMode()		{return impr;}
	inline C_QGT	*GetQGT()	{return qgt;}
	//---------------------------------------------------------------------
	C_STile *GetSupTile(short x,short z)	{return &aST[x][z];}	// Return array of SupTile
};
//==================================================================================
//  CLASS CTEXto read and decode FLYI scenery files TEX file
//==================================================================================
class C_CTEX: public CStreamObject {
  friend class TCacheMGR;
  //--------ATTRIBUTES -------------------------------------------------
  U_INT   tr;                                         // Trace indicator
  C_QGT  *qgt;                                        // Current QGT
  U_INT   nbt;                                        // Number of textures
  char   text[1024][16];                              // Texture names
  //--------------------------------------------------------------------
public:
  C_CTEX(C_QGT *qgt,U_INT tr);                        // Constructor
  void  Abort(char *fn, char *em);
  void  Assign(char *tn,U_SHORT tx,U_SHORT tz);       // Assign texture
  void  NormeName(char *txt);
};
//==================================================================================
//  DETAIL TILE HEADER
//===================================================================================
struct SEA_HEADER {
  U_SHORT Index;
  U_LONG  Offset;
};
struct SEA_VERTEX {
  float cx;
  float cz;
};
//------------------------------------------------------------------------
//  Intermediate structure to decode coast polygon
//-----------------------------------------------------------------------
struct SEA_COAST {
    U_LONG        Key;                                  // Detail tile Key
    U_SHORT      nVRT;                                  // Number of Vertices
    U_SHORT      nPOL;                                  // Number of Polygon
    SEA_VERTEX  *vTAB;                                  // Vertice Table
    void        *pTAB;                                  // Polygon Table
};
//-----------------------------------------------------------------------
#define SEA_HEAD_SIZE (sizeof(U_SHORT) + sizeof(U_LONG))
//----------------------------------------------------------------------------------
//==================================================================================
//  Class Coast Line:  Common parts between POD and SQL
//==================================================================================
class CoastLine {
protected:
  //--- Attributes ------------------------------------------------------
  U_INT       Key;                                     // QGT key
  U_INT       Use;                                     // Reference count
  TCacheMGR  *tcm;                                     // Cache manager
  //--------Drawing color -----------------------------------------------
  U_INT     color;
  //---------------------------------------------------------------------
  pthread_mutex_t	Mux;                                // protection Mutex
  std::map<U_INT,char*>  coastMAP;                    // Map of polygon lines
  //--------Drawing parameters ------------------------------------------
  SSurface  *surf;                                  // Surface
  TC_SPOINT  pPix;                                  // Previous pixel
  TC_SPOINT  cPix;                                  // Coast pixel
  double     dLat;                                  // Delta per Detail tile
  double     dLon;                                  // Delta per Detail Tile
  //-----METHODS --------------------------------------------------------
protected:
  void          CoastVertexDistance(short px,short py,SVector &sw);
  COAST_VERTEX *CoastPolygon(COAST_VERTEX *pol,SVector &v);
  //---------------------------------------------------------------------
public:
	virtual			~CoastLine();
  void         DrawCoastLine(SSurface *sf);
};
//==================================================================================
//  Class C_SEA:   To read and Decode coast files
//==================================================================================
class C_SEA: public CoastLine  {
  //--------Data member -------------------------------------------------
  U_INT       gx;                                           // Global X
  U_INT       gz;                                           // Global Z
  U_INT       qgx;                                          // Base QGT X indice
  U_INT       qgz;                                          // Base QGT Z indice
  U_INT       tr;                                           // Trace indicator
  PODFILE    *pod;                                          // Pod File
  U_SHORT     lgr;                                          // Number of tiles
  //--------QGT coordinates ----------------------------------------------
  float       gbtHT;                                        // Global Tile heigth
  float       gbtSL;                                        // South latitude
  float       gbtNL;                                        // North latitude
  //--------Detail tile working area -------------------------------------
  int         xpDET;                                // Base pixel X
  int         zpDET;                                // Base pixel Z
  //------Delete option --------------------------------------------------
  char        opDEL;                                // Delete option
  char        nCN;                                  // Number of detected corners
  //--------MAP of DETAIL TILE COAST -------------------------------------
  U_INT       NbDET;                                // Number of detail tiles
  U_INT       NbDEL;                                // Deleted number
  //--------WORKING AREA -------------------------------------------------
  char      *head;                                  // header data
  SEA_VERTEX ord[4];                                // Ordered vertices
  SEA_COAST  coast;                                 // Current coast data
  char      *poly;                                  // Current polygon
  double      wLon;                                 // Base Longitude
  double      sLat;                                 // Base latitude
  U_SHORT    NbVRT;                                 // Total vertice for polygons
  char       Name[256];                             // File name
  //--------Public methods ----------------------------------------------
public:
  C_SEA(U_INT key,TCacheMGR *tcm);
 ~C_SEA();
 //-----------------------------------------------------------------------
  void         Init(U_INT key);
  void         Abort(char *reason);
  bool         NoMoreUsed();
  bool         StillUsed();
  void         Load();
  void         LoadDetail(char *head);
  bool         LoadVertices(U_LONG offset);
  void         LoadPolygon();
  void         CheckCorner(U_INT px,U_INT pz);
  //---------------------------------------------------------------------
  char         *BuildCoastLine();
  COAST_VERTEX *Finalize(COAST_VERTEX *pol, char *pid,SEA_VERTEX *vtb);
  U_INT         GetPixXcoord(float cx);
  U_INT         GetPixZcoord(float cz);
  //---------------------------------------------------------------------
  //------HELPER --------------------------------------------------------
  U_LONG  MakeTileKey(U_SHORT ind);                          // Get Tile key
  char   *GetCoast(U_INT ax,U_INT az);
  //---------------------------------------------------------------------
  inline  void   IncUser()    {Use++;}
  inline  U_INT  GetKey()     {return Key;}
  inline  U_INT  GetUse()     {return Use;}
  //-----DRAWING ROUTINES -----------------------------------------------
};
//========================================================================
//  C_CDT to describe QGT coast data
//========================================================================
class C_CDT: public CoastLine  {
  //---Attributes -------------------------------------------------
  int item;
  //---------------------------------------------------------------
public:
  C_CDT(U_INT key,TCacheMGR *tcm);
 ~C_CDT();
 //----------------------------------------------------------------
 void   AddCoast(U_INT key,char *data);
 char  *GetCoast(U_INT key);
 //----------------------------------------------------------------
 bool   NoMoreUsed();
 void   IncUser();
};
//===========END OF THIS FILE ============================================================
#endif // TERRAINELEVATION_H
