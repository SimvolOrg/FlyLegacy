/*
 * TerrainTexture.h
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
#ifndef TERRAINTEXTURE_H
#define TERRAINTEXTURE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "../Include/Globals.h"
#include "../Include/Utility.h"
#include "../Include/FreeImage.h"
#include "../Include/TerrainCache.h"
#include "../Include/LightSystem.h"
//-----------------------------------------------------------------------------
#define TC_ACTSIZE 0x300
//=============================================================================
class CTarmac;
class CTextureWard;
extern U_CHAR    MaxiRES[];
//=============================================================================
//  Memory texture count
//=============================================================================
class CMemCount {
public:
  //-----------------------------------------
  int Dcnt[2];            // std Day count
  int Ncnt[2];            // std nigth count
  //-------------------------------------------
  CMemCount();
};
//=============================================================================
//  Contour state:
//  nState gives the next contour state (0, RED, BLUE or RED and BLUE)
//  swap   0=> No swap  1 = > swap
//=============================================================================
struct TC_PIX_STATE 
{ U_CHAR  nState;                               // Next state
  U_CHAR  swap;                                 // Inside swap value
  U_CHAR  In;                                   // Inside value
};
//=============================================================================
//  Type of shared texture
//=============================================================================
#define SHX_POD   (0)
#define SHX_SQL   (1)
//=============================================================================
//  Class CWaterTexture : to store fixed water file textures
//  NOTE:  The RGBA texture is managed by the shared texture object. 
//         Here this is just a pointer. So dont delete the texture here.
//=============================================================================
class CWaterTexture {
public:
  char     Name[TC_TEXNAMESIZE];        // Texture name
  U_INT   *rgba;                        // rgba texture
  int      res;                         // Resolution
  U_INT    xOBJ;                        // Texture object
  //---------Methodes ----------------------------------------
  CWaterTexture();
  //----------------------------------------------------------
 ~CWaterTexture();
 //-----------------------------------------------------------
  void    LoadTexture(TEXT_INFO &txd, char res,U_INT af);
  U_INT  *GetWaterCopy();
 //-----------------------------------------------------------
  inline   U_INT *GetRGBA()   {return rgba;} 
  inline   void   AssignOBJ(U_INT o) {xOBJ = o;}
  inline  GLuint  GetXOBJ()   {return xOBJ;}
};
//=============================================================================
//  Class CBlendTexture:  To store the transition mask
//=============================================================================
class CBlendTexture {
public:
  int      side;                         // Resolution
  GLubyte  *msk;                         // Texture
  //--------Methods ------------------------------------------
  CBlendTexture();
 ~CBlendTexture();
 //-----------------------------------------------------------
  inline GLubyte *GetMask()           {return msk;}
};
//=============================================================================
//  Class CSharedTxnTex:  Shared texture
//=============================================================================
class CSharedTxnTex: public CTextureDef {
public:
  U_INT   Use;                          // User count
  char  Blend;                          // Request blending alpha
  //---------Method ------------------------------------------
  CSharedTxnTex(char *name,U_CHAR res);
 ~CSharedTxnTex();
  //-----------------------------------------------------------------------------
  inline bool AssignOBJ(CTextureDef *txn) {txn->dOBJ = dOBJ; return (dOBJ != 0);}
  //-----------------------------------------------------------------------------
  inline void SetBlend(char b)    {Blend = b;}
  inline char GetBlend()          {return Blend;}
};
//=============================================================================
//	Texture State
//=============================================================================
#define SHX_INIT (0)
#define SHX_ISOK (1)
#define SHX_NULL (2)
#define SHX_SKIP (3)
//=============================================================================
//  Class CShared3DTex for 3D texture sharing
//  NOTE:  The texture name include path 
//=============================================================================
class CShared3DTex {
  //-----Attribute -----------------------------------------
	char          State;												// State
	char					rfu1;													// Reserved
  U_INT         Use;                          // User count
  TEXT_INFO     x3d;                          // Texture info
  //--------------------------------------------------------
public:
  CShared3DTex(char *tn,char tsp, char dir = 0);
	CShared3DTex(TEXT_INFO &txd);
 ~CShared3DTex();
 char                 *GetIdent()      {return  x3d.path;}
 //----------------------------------------------------------
 bool		BindTexture();
 void		GetDimension(TEXT_INFO &txd);
 bool		SameTexture(char dir, char *txn);
 char  *TextureData(char &dir);
 //----------------------------------------------------------
 inline void					 SetOK()								{State = SHX_ISOK;}
 inline TEXT_INFO     *GetInfo()              {return &x3d;}
 inline TEXT_INFO     *GetDescription()				{return & x3d;}
 inline U_INT          IncUser()              {Use++; return Use;}
 inline bool           DecUser()              {Use--; return (Use != 0);}
 inline U_INT          GetOBJ()               {return x3d.xOBJ;}
 inline void           SetXOBJ(GLuint r)      {x3d.xOBJ = r;}
 inline U_INT					 GetUser()							{return Use;}
 inline char          *GetPath()							{return x3d.path;}
 };
//=============================================================================
//  Class CArtParser to read texture files with several options
//
//   
//=============================================================================
class CArtParser {
  friend class CTextureWard;
  //-----------------------------------------------------------
  FREE_IMAGE_FORMAT    ffm;                     // File format
  int                  fop;                     // File option
  FIBITMAP            *ref;                     // File reference
  FIMEMORY            *mem;                     // Memory reference
  FREE_IMAGE_TYPE      type;                    // Image type
  U_INT     bpp;                                // Bit per plan
  U_INT     htr;                                // Heigth
  U_INT     wid;                                // Width
  U_INT     dim;                                // Full size
  U_INT     side;                               // Side in pixel
  char      *dot;                               // Dot place into name
  U_INT     *rgb;                               // RGBA data                         
  //-------Color for coast mask -------------------------------
  char      epd;																// EPD texture
  char      res;                                // Resolution
  char      abt;                                // abort option
	char      rrv;																// reverse row
  //-------Alpha channel --------------------------------------
  U_CHAR    afa;                                // Alpha channel
	//-----------------------------------------------------------
	U_CHAR		mac;																// Mac option
	char			rfu;																// Not used
  //-------Main tile data -------------------------------------
  GLubyte   *act;                               // ACT bytes
  GLubyte   *raw;                               // Raw bytes
  GLubyte   *opa;                               // OPA bytes
  GLubyte   *wtx;                               // Water RGBA
  //-------Transition file 1 -(used for right transition)--------
  char       Tp1;                               // Terrain type
  GLubyte   *Tx1;                               // Texture 1
  GLubyte   *Ms1;                               // Mask 1
  //-------Transition file 2 ------------------------------------
  char       Tp2;                               // Terrain type
  GLubyte   *Tx2;                               // Texture 2
  GLubyte   *Ms2;
  //-------Transition file 3 ------------------------------------
  char       Tp3;                               // Terrain Type
  GLubyte   *Tx3;                               // Texture 3
  GLubyte   *Ms3;
  //------------Methods -------------------------------------------------
public:
  CArtParser(char res);
  CArtParser()  {}
 ~CArtParser();
  void      Abort(char *fn,char *er);
  int       SetSide(int s);
  void      NoFile(char *fn);
  bool      LoadPOD(char *rnm,char tsp);            // Load from POD
  bool      LoadFIM(char *rnm,char tsp,FREE_IMAGE_FORMAT ff);
	GLubyte  *LoadRaw(TEXT_INFO &txd,char opt);       // Load texture
  //---------------------------------------------------------------------
  bool      LoadFFF(char *rnm,char tsp,FREE_IMAGE_FORMAT ff);
	void		  TryaPOD(char *fnm,U_CHAR **buf,int &sz);
	void			TryFILE(char *fnm,U_CHAR **buf,int &sz);
  //---------------------------------------------------------------------
  int       ConvertRGBA(U_CHAR alf);                // Convert in RGBA
  int       ByteRGBA(U_CHAR opt);
	int				ByteABGR(U_CHAR opt);
  int       IndxRGBA(U_CHAR opt);
  int       PixlRGBA(U_CHAR opt);
  int       PixlBGRO(U_CHAR opt);
  int       ByteTIFF(U_CHAR opt);
	int				RgbaTIFF(U_CHAR opt);
	//---------------------------------------------------------------------
  void      WriteBitmap(FREE_IMAGE_FORMAT ff,char *fn,int wd,int ht,U_CHAR *buf);
  //---------------------------------------------------------------------
  GLubyte  *TransferRGB();
  //----Normal textures -------------------------------------------------
  GLubyte  *ModlMerger(U_CHAR alf);
  GLubyte  *RGBAMerger(U_CHAR alf);
  GLubyte  *RGBAInvert(U_CHAR alf);
  //---------------------------------------------------------------------
  int       GetSize(int tot,char *fn);
  int       GetFileType(char *name);
  //---------------------------------------------------------------------
  inline void  SetWaterRGBA(U_INT *wt)      {wtx = (GLubyte*)wt;}
  inline U_INT GetBPP()                     {return bpp;}
  inline void  DontAbort()                  {abt = 0;}
  inline void  SetFormat(FREE_IMAGE_FORMAT f) {ffm = f;}
  inline void  SetOption(int p)               {fop = p;}
  inline void  SetEPD()                     {epd   = 1;}
	inline void	 NoReverse()									{rrv   = 0;}
	inline void  MacPic()											{mac   = 1;}
  //--- Transition textures ---------------------------------------------
  inline void  SetTrans1(GLubyte *t)        {Tx1 = t;}
  inline void  SetTrans2(GLubyte *t)        {Tx2 = t;}
  inline void  SetTrans3(GLubyte *t)        {Tx3 = t;}
	//--- Texture management ----------------------------------------------
	void					ReplaceRGBA(U_INT *buf);
  inline U_INT GetDim()                     {return dim;}
  inline U_INT GetSide()                    {return side;}
  inline U_INT GetHeigth()                  {return htr;}
  inline U_INT GetWidth()                   {return wid;}
	inline GLubyte *GetRAW()									{return raw;	}
	inline GLubyte *GetACT()									{return act;	}
	inline GLubyte *GetOPA()									{return opa;	}
  //---------------------------------------------------------------------
  void         InitTransitionT1(TEXT_INFO &txd);
  void         InitTransitionT2(TEXT_INFO &txd);
  void         InitTransitionT3(TEXT_INFO &txd);
  //--- FULL Textures functions -----------------------------------------
  GLubyte     *MergeWater(GLubyte *tex);
  GLubyte     *MergeNight(GLubyte *tex);
  //--------------------------------------------------------------------
  GLubyte     *Mixer(TEXT_INFO &txd);
  //-----VECTORED ROUTINES FOR TEXTURES ------------------------------------
  GLubyte     *GetRawTexture(TEXT_INFO &txd,char opt);
  GLubyte     *LoadTextureMT(TEXT_INFO &txd);
  GLubyte     *LoadTextureFT(TEXT_INFO &txd);
  GLubyte     *GetDayTexture(TEXT_INFO &txd,char opa);  // VF 1
  GLubyte     *GetNitTexture(TEXT_INFO &txd);           // VF 2
  GLubyte     *TransitionTexture(TEXT_INFO &txd);
  //-------------------------------------------------------------------
  GLubyte     *GetModTexture(TEXT_INFO &txd);
  GLubyte     *GetAnyTexture(TEXT_INFO &txd);
  void         FreeAnyTexture(GLubyte *text);
  void         FreeFFF(FREE_IMAGE_FORMAT ff);
};

//=============================================================================
//  Class CTextureWard
//  This class stores all current textures in cache
//
//=============================================================================
class CTextureWard  {
  //-------Data Member ----------------------------------------
  U_INT   tr;                         // Trace indicator
  TCacheMGR *tcm;                     // Cache manager
  SqlMGR    *sqm;                     // Main sql manager
  bool       usq;                     // Use sql for textures
	//--- MIP level for Objects ---------------------------------
  //-----Picture number ---------------------------------------
  int     nPic;                       // Picture number for output
  //-------Working --------------------------------------------
  C_SEA  *sea;                        // Associated sea file
  C_QGT  *qgt;                        // Working Quarter Global Tile
  U_INT   gx;                         // Global Tile X composite
  U_INT   gz;                         // Global Tile Z composite
  U_INT   qz;                         // QGT Z composite
  U_INT   tx;                         // Base Tile X index
  U_INT   tz;                         // Base Tile Z index
  //-------Texture allocation ---------------------------------
  TEXT_INFO  xmd;                     // Texture loading 3D models
  TEXT_INFO  xsp;                     // Texture loading specific
  TEXT_INFO  xld;                     // Texture loading terrain
  TEXT_INFO  xds;                     // Texture loading others
  GLubyte *dTEX;                      // Day texture
  GLubyte *nTEX;                      // Night texture
  U_CHAR   Resn;                      // Requested Resolution
  //-------Options --------------------------------------------
  U_CHAR NT;                          // Nigh textures enable
  //-------State management -----------------------------------
  U_CHAR    Inside;                   // Inside Flag
  U_CHAR    State;                    // Current color state
  //-------Terrain Texture cache ------------------------------
  pthread_mutex_t	txnMux;                   // State lock
  std::map<U_INT,CSharedTxnTex*>  txnMAP;   // Texture map
  //-------3D Object cache -----------------------------------
  pthread_mutex_t	t3dMux;                   // State lock
  std::map<std::string,CShared3DTex*> t3dMAP;   // Texture map
  //-------Generic files -------------------------------------
  U_CHAR  Night;                      // Night indicator
  CWaterTexture Tank[4];              // Water bitmaps
  CBlendTexture Blend[12];            // Blending masks
  //-------Runway texture Object ------------------------------
  U_INT     RwyTX[4];                 // 4 runway textures  
  //-------Accounting -----------------------------------------
  pthread_mutex_t	ctrMux;             // State lock
  CMemCount NbTXM;                    // Texture in memory
  int       NbCUT;                    // Cut number
	int				NbG3D;										// GL textures Objets for 3D
	int				NbDOB;										// Terrain day Textures object
	int				NbNOB;										// Terrian Nit texture object
  //-------Animated water ---------------------------------------
  U_INT     kaf;
  //-------Light textures ---------------------------------------
	GLuint  cTERRA;											// Compressed format
  GLuint  LiOBJ[8];                   // Ligth Texture objects
  //-------CANVAS for drawing coast polygons --------------------
  int     Dim;                        // Dimension for resolution
  U_CHAR  alfa[2];
  U_CHAR  Color;                      // Line color
  U_CHAR  Cnv1[TC_TEXTMAXDIM];        // Canvas 1
  U_CHAR *Canvas;                     // Current canvas
  U_INT  *Water;                      // Water Texture
  TC_BOUND pBox;                      // Bounding box for polygons
  TC_BOUND iBox;                      // Initial values
  //-------Methods --------------------------------------------
public:
  CTextureWard(TCacheMGR *mgr,U_INT tr);                      // Constructor
 ~CTextureWard();                                             // Destructor
  int     Abort(char *msg,char *mse);
  int     Warn (char *msg,char *mse);
  void    TraceCTX();
  void    GetTextParam(char res,float *dto,float *dtu);
  //----Shared Key building ----------------------------------
  U_INT   KeyForTerrain(CTextureDef *txn,U_CHAR res);
  U_INT   KeyForWater(U_CHAR res);
  U_INT   KeyForRunway(U_CHAR grnd,U_CHAR seg);
  U_INT   KeyForTaxiway(U_CHAR res);
  GLuint  GetLiteTexture(U_CHAR No);
  //-----Inlines ---------------------------------------------
  inline  void    SetResolution(U_CHAR r) {Resn = r;}
  inline  U_CHAR  GetMaxRes(U_CHAR t)   {return MaxiRES[t];}
  inline  void    SetNight(U_CHAR nit)  {Night = nit;}
  inline  bool    IsNight()             {return (Night == 'N');}
  inline  bool    IsDay()               {return (Night == 'D');}
  inline  GLubyte *GetMask(U_INT m,U_INT r) {return Blend[(m | r)].GetMask();}
  //-----Helpers ---------------------------------------------
  GLubyte *LoadMSK(char *msn,int side); 
  float   GetLuminance(float *col);
  int     GetMixTexture(CTextureDef *txn,U_CHAR opt);
  int     GetShdTexture(CTextureDef *txn,char *name);
  int     GetGenTexture(CTextureDef *txn);
  int     GetRawTexture(CTextureDef *txn);
  int     GetEPDTexture(CTextureDef *txn);
  int     GetSeaTexture(CTextureDef *txn);
	int 		GetTRNtextures(CTextureDef *txn, U_INT qx, U_INT qz);
	//----------------------------------------------------------
  void    GetTileName (char *base);
  void    BuildName(char *gen,char *root,char res);
  void    GetMediumTexture(CTextureDef *txn);
  //----------------------------------------------------------
  void    GetShdOBJ(CTextureDef *txn);
  GLuint  GetTaxiTexture();
	//------------------------------------------------------------
	CShared3DTex   *RefTo3DTexture (TEXT_INFO &txd);
	//------------------------------------------------------------
	CShared3DTex   *AddSHX(CShared3DTex *shx ,char type);
	CShared3DTex	 *GetM3DPodTexture(TEXT_INFO &txd);
	CShared3DTex	 *GetM3DSqlTexture(TEXT_INFO &txd);
	//------------------------------------------------------------
  void    Get3DTIF(TEXT_INFO *inf);
  void    Get3DRAW(TEXT_INFO *inf);
	void		GetAnyTexture(TEXT_INFO *inf);
  GLuint  Get3DObject(void *tref);
  void    Free3DTexture(void *sht);
	void		GetTextureParameters(void *ref,TEXT_INFO &inf);
	void		ReserveOne(void *ref);
	//------Night textures -------------------------------------
  int     NightGenTexture(CTextureDef *txd);
  int     NightRawTexture(CTextureDef *txn);
  int     DoubleNiTexture(CTextureDef *txn,U_INT *tex);
  //-----Texture Handling ------------------------------------
  int     FreeShared(CTextureDef *txn);
  int     FreeWater (CTextureDef *txn);
  int     FreeAllTextures(CSuperTile *sp);
  int     LoadTextures(U_CHAR lev,U_CHAR res,C_QGT *qgt,CSuperTile *sp);
  int     SwapTextures(CSuperTile *sp);
  void    LoadMaskTexture(int No,char *name,int dim);
  void    LoadTaxiTexture(char *name,char tsp);
  void    LoadRwyTexture(U_INT key,char *fn, char tsp);
  void    LoadLightTexture(U_CHAR No);
  //-----Helpers ----------------------------------------------
  void    WriteTexture(int wd,int ht,U_INT obj);
  void    WriteScreen();
  //-----------------------------------------------------------
	void		GetRwyTexture(CTarmac *tmac,U_CHAR gt);
  CSharedTxnTex *GetSharedTex(U_INT key);
  int     FreeSharedSlot(CTextureDef *txn);
  int     FreeWaterSlot(CTextureDef *txn);
  void    FreeSharedKey(U_INT key);
	void		FreeTerrainOBJ(U_INT day,U_INT nit);
  //----------------------------------------------------------
  GLuint  GetGaugeOBJ(TEXT_DEFN &txd);
  GLuint  GetMskOBJ(TEXT_INFO &inf,U_INT mip);
  GLuint  GetTexOBJ(TEXT_INFO &inf, U_INT mip,U_INT type);
  GLuint  GetTexOBJ(GLuint obj,int x,int y,GLubyte *tex,U_INT type);
  GLuint  GetRepeatOBJ(GLuint obj,U_CHAR res,GLubyte *tex);
  GLuint  GetTerraOBJ(GLuint obj,U_CHAR res,GLubyte *tex);
  GLuint  GetLitOBJ(GLuint obj,U_INT dim, GLubyte *tex);
  GLuint  GetWatOBJ(CTextureDef *txn);
  void    GetSupOBJ(CSuperTile *sp);
  //----------------------------------------------------------
  GLuint  GetRepeatOBJ(TEXT_INFO &xds);
  GLuint  GetLitOBJ(TEXT_INFO    &xds);
  //----------------------------------------------------------
  U_INT  *GetWaterRGBA(U_CHAR res)    {return Tank[res].rgba;}
  U_INT  *GetWaterCopy(U_CHAR res)    {return Tank[res].GetWaterCopy();}
  bool    LoadImagePNG(char *fn, S_IMAGE &ref);
  bool    LoadImageJPG(char *fn, S_IMAGE &ref);
  GLuint  LoadIconPNG(char *fn);
  void    LoadAnyTexture(char *fn,TEXT_DEFN &txd);
  //------------COAST ROUTINES       ----------------------
  void    DrawTLine (int x1,int y1,int x2, int y2);
  void    DrawVLine(int x0,int y1,int y2);
  void    DrawHLine(int x1,int x2,int y0);
  U_INT   AdjustCoordinate(U_INT val);
  COAST_VERTEX *DrawStencil(COAST_VERTEX *pol);
  void    ScanCoast();
  void    MakeStencil(char *lsp);
  void    AlphaCoastTexture(U_CHAR clr,GLubyte *land);
  void    BuildNightTexture(U_INT *tx);
  int     BuildCoastTexture(GLubyte *land);
  //--------------------------------------------------------
  //  Counters
  //--------------------------------------------------------
  //------Statistical data ---------------------------------
  void    GetStats(CFuiCanva *cnv);
};
//===============END OF THIS FILE =============================================
#endif  // TERRAINTEXTURE_H
