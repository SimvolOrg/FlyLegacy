/*
 * FileParser.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
 * CopyRight 2008 Jean Sabatier
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
#ifndef FILEPARSER_H
#define FILEPARSER_H

#include "../Include/FlyLegacy.h"
#include "../Include/Model3d.h"
#include "../Include/LightSystem.h"
//======================================================================================
class CAirportMgr;
class C3Dmodel;
class C3DPart;
class GroundSpot;
struct METAR_INFO;
//======================================================================================
#define SIZE_INT1   (sizeof(int))
#define SIZE_INT2   (SIZE_INT1 + sizeof(int))
#define SIZE_INT3   (SIZE_INT2 + sizeof(int))
#define SIZE_INT4   (SIZE_INT3 + sizeof(int))
#define SIZE_INT5   (SIZE_INT4 + sizeof(int))
//=============================================================================
//  SMF decoder to parse a SMF file
//  Parts are classified as
//  model 0:  Day   - Big size
//  model 1:  Night - Big size
//-----------------------------------------------------------
//  model 2:  Day   - Tin size
//  model 3:  Night - Tin  size
//-----------------------------------------------------------
//  Only part of hi resolution are kept
//=============================================================================
//=============================================================================
//  BIN STRUCTURES
//=============================================================================
struct BIN_HEADER {
  int scale;
  int unkn1;
  int unkn2;
  int nvert;                    // vertices numbers
};
struct B19_HEADER {
  int nvert;                    // Vertice count
  int nmx;                      // Normal x
  int nmy;                      // Normal y
  int nmz;                      // Normal z
  int magic;      
};

//----- a triangle -------------------------------
struct OBJ_TRIANGLE {
	GN_VTAB vtx[3];				
};
//--- Structure to define OBJ material -----------
struct OBJ_MATERIAL {
	char *name;
	std::vector<OBJ_TRIANGLE*> triQ;		// List of triangles
	//--- constructor ------------------------------
	OBJ_MATERIAL() {name = 0;}
	//--- destructor -------------------------------
 ~OBJ_MATERIAL() 
 {	if (name) delete name;
		for (U_INT k=0; k<triQ.size(); k++) delete triQ[k];
		triQ.clear();	
 }
};
//========================================================================
//  TIF STRUCTURES
//========================================================================
typedef struct {
  unsigned short    endian;     // 'II' (Little Endian) or 'MM' (Big-Endian)
  unsigned short    signature;  // 0x0042 in appropriate endian-ness
  long              ifd;        // Image File Directory offset
} TIF_HDR;
//----Directory entry ----------------------------------------------------
typedef struct {
  unsigned short    tag;
  unsigned short    type;
  unsigned long   count;
  unsigned long   offset;
} TIF_DIR;
//------STRIP PARAMETERS -------------------------------------------------
typedef struct {
  unsigned long    ofs;                 // Strip offset
  unsigned long    cnt;                 // strip byte count
}TIF_STRIP;
//------COLOR MAP --------------------------------------------------------
typedef struct {
    GLubyte r, g, b;
  } TIF_RGB;
//------TAG VALUES -------------------------------------------------------
#define TIF_TAG_IMAGEWIDTH          (0x0100)
#define TIF_TAG_IMAGELENGTH         (0x0101)
#define TIF_TAG_BITSPERSAMPLE       (0x0102)
#define TIF_TAG_COMPRESSION         (0x0103)
#define TIF_TAG_INTERPRETATION      (0x0106)
#define TIF_TAG_STRIPOFFSETS        (0x0111)
#define TIF_TAG_SAMPLESPERPIXEL     (0x0115)
#define TIF_TAG_ROWSPERSTRIP        (0x0116)
#define TIF_TAG_STRIPBYTECOUNTS     (0x0117)
#define TIF_TAG_PLANARCONFIGURATION (0x011C)
#define TIF_TAG_COLORMAP            (0x0140)
#define TIF_TAG_XRESOLUTION					(0x011A)
#define TIF_TAG_YRESOLUTION					(0x011B)
#define TIF_TAG_RESOLUTION_UNIT			(0x0128)
#define TIF_TAG_SOFTWARE						(0x0131)
#define TIF_TAG_DATE								(0x0132)
#define TIF_TAG_ExtraSamples				(0x0152)
#define TIF_TAG_NEWSUBFILETYPE      (0x00FE)
//------------------------------------------------------------------------
#define PTIF_LITTLE_ENDIAN  (0x4949)
#define PTIF_BIG_ENDIAN     (0x4D4D)
#define PTIF_SIGNATURE      (42)
//------------------------------------------------------------------------
#define TIF_RGBA            (0x01)
#define TIF_CMAP            (0x02)
//-----DATA TYPE ---------------------------------------------------------
#define TIF_TYPE_BYTE    (1)
#define TIF_TYPE_ASCII   (2)
#define TIF_TYPE_SHORT   (3)
#define TIF_TYPE_LONG    (4)
#define TIF_TYPE_RATIONAL  (5)
//========================================================================
//	Common file parser
//========================================================================
class CParser : public CStreamObject {

	//--- Attributes ---------------------------------------
protected:
	C3DPart   *part;                  // Current part
	char      *fname;                 // File name
  int        nFace;                 // Number of faces
	OBJ_TRIANGLE tri;									// A triangle
	//------------------------------------------------------
	PODFILE   *pod;										// Pod file
	char      retcode;								// return code
	char      trace;									// Trace request
	char			type;										// Type of file
	U_CHAR     Tsp;										// Transparent mode
	U_CHAR     Dir;										// Directory
	//--- Texture description ------------------------------
	TEXT_INFO  txd;
	//----Model extension ----------------------------------
  CVector    vmax;                  // Maximum coordinates
  CVector    vmin;                  // Minimum coordinates
	//--- Part Queue ---------------------------------------
	Queue<C3DPart> partQ;							// List of parts
	//------------------------------------------------------
	CShared3DTex   *tREF;									// Texture reference
	char  txname[TC_TEXTURE_NAME_DIM];    // Texture name
	//--- METHODS ------------------------------------------
public:
	CParser::CParser(char t);
	int		StopParse (PODFILE *p,char *msg);
	void	SaveExtension(F3_VERTEX &vt);
	void	SaveExtension(TC_VTAB &vt);
	void	SaveExtension(GN_VTAB &vt);
	int   GetStatement(char *s);
	//------------------------------------------------------
	int		LoadModel(C3Dmodel *mod);
	//------------------------------------------------------
	C3DPart *GetOnlyOnePart();
	};
//========================================================================
//
// CTIF parser
//
//=========================================================================
class CTIFFparser {
  char         *Name;
  unsigned long Wd, Ht, Dp;
  U_CHAR        Tsp;              // Transparent option
  U_CHAR        Big;              // Endian option
  U_CHAR        Bps;              // Bit per sample
  U_CHAR        Spp;              // Sample per pixel          
  U_CHAR        Fmt;              // Format
  U_INT         Rps;              // Number of row per strip
  U_INT         Nst;              // Number of Strip
  //----Color MAP parameter---------------------------------
  U_INT         CMdim;            // Dimension(byte)
  U_INT         CMofs;            // Offset
  U_INT        *cMap;             // Color MAP
  //----Final Image ----------------------------------------
  U_INT         nPix;             // Number of pixels
  U_INT        *rgba;             // Final image
  U_INT        *bdst;             // Current destination
	U_INT					cnbp;							// Current pixel number
  //----Decoding structure ---------------------------------
  TIF_HDR       Hdr;              // TIF haeder
  TIF_DIR       Dir;              // Directory header
  TIF_STRIP    *Strip;            // Strip definition
  //--------------------------------------------------------
public:
  // Constructor/Destructor
   CTIFFparser (U_CHAR tsp);
  ~CTIFFparser ();
  //-------------------------------------------------------
   U_SHORT Value(short s);
  //-------------------------------------------------------
  GLubyte  *TransferRGB();
  U_INT    *TransferPIX();
  //--------------------------------------------------------
  void      CopyLine(U_INT *src,U_INT *dst);
  void      InvertPIX();
  int   Decode(char *fn);
  int   Warning(PODFILE *p,char *msg);
  short Read16 (PODFILE *p);
  long  Read32 (PODFILE *p);
  int   GetFormat(PODFILE *p,int f);
  int   GetStripOffset(PODFILE *p);
  int   GetStripCount(PODFILE *p);
  int   GetImage(PODFILE *p);
  void  GetColorMAP(PODFILE *p);
  void  GetStripFromCMAP(PODFILE *p,int k,U_INT *dst);
  void  GetStripFromRGBA(PODFILE *p,int k,U_INT *dst);
  //-------------------------------------------------------
  U_INT GetSide()       {return Wd; }
  int   GetWidth()      {return Wd; }
  int   GetHeight()     {return Ht; }
};
//=============================================================================
//  SMF decoder to parse a SMF file
//  This version will use vertex tables for parts
//  Parts are classified as
//  model 0:  Day   - Big size
//  model 1:  Night - Big size
//-----------------------------------------------------------
//  model 2:  Day   - Tin size
//  model 3:  Night - Tin  size
//-----------------------------------------------------------
//  Only part of hi resolution are kept
//=============================================================================
class CSMFparser: public CParser {
  //---ATTRIBUTES ----------------------------------------
  U_CHAR     rfu;                   // Transparency
  U_CHAR     Res;                   // Resolution
  int        version;               // File version
  //----Methods ------------------------------------------
public:
  CSMFparser(char t);
  int     Decode(char *fn,char t);
  int     ReadPart(PODFILE *p,char *fn);
  int     ReadType(char *name);
  //------------------------------------------------------
 };
//=============================================================================
//  BIN decoder to parse a BIN file
//=============================================================================
class CBINparser: public CParser {
  //---ATTRIBUTES ----------------------------------------
  F3_VERTEX   *nVTX;              // Vertex list
  U_CHAR    shiny;                // Shiny texture
  U_CHAR    Res;                  // Resolution
  U_INT     xOBJ;                 // Texture OBJECT
  char      Texn[20];             // Texture name
  F3_VERTEX norm;									// Normal vertex
	F2_COORD  ctex;									// Texture coord
	//------------------------------------------------------
	int				Nbv;									// Number of vertices
  //------------------------------------------------------
  int       Tof;                  // Total faces
  int       Tov;                  // Total vertices
  //------------------------------------------------------
  BIN_HEADER hdr;
  B19_HEADER b19;
  //------------------------------------------------------
public:
  CBINparser(char t);
 ~CBINparser();
  //-------------------------------------------------------
  float GetSValue(int vs);
  float GetTValue(int vs);
  int   Decode(char *fn,char t);
  int   ReadHeader(PODFILE *p);         // File header
  int   ReadVertex(PODFILE *p);         // Vertex list
  int   ReadBlock(PODFILE *p);          // Block type
  int   ReadNormal(PODFILE *p);         // Normal vectors
  int   ReadColor(PODFILE *p);
  void  AddToModel(C3DPart *prt);
	//---New interface  -------------------------------------
	int		ReadTexture(PODFILE *p);
	int		ReadTFaces(PODFILE *p);
	int		ReadQFaces(PODFILE *p);
  //-------------------------------------------------------
  inline void Trace()     {trace = 1;}
};
//=======================================================================================
//  CLASS CMETARparser to decode metar bulletins
//=======================================================================================
class METARparser {
  //---ATTRIBUTES -----------------------------------------------
  FILE       *filr;               // File reference
  METAR_INFO *info;
  char        fld[32];
  //-------------------------------------------------------------
public:
  METARparser(FILE *f,METAR_INFO *inf,char *fn);
  int   Parse();
  int   Warn(char *msg);
  int   ParseWind(FILE *f);
  int   ParseWdP2(FILE *f);
  int   ParseUSvis(FILE *f);
  int   ParseEUvis(FILE *f);
  int   ParseRWvis(FILE *f);
  int   ParseRWend(FILE *f);
  int   ParseWeather(FILE *f);
  int   WeatherP1(FILE *f,char c);
  int   WeatherP2(FILE *f);
  int   ParseCover(FILE *f);
  int   CoverP1(FILE *f);
  int   CoverP2(FILE *f);
  int   ParseTemp(FILE *f);
  int   ParseAlti(FILE *f);
  int   ParseQNH(FILE *f);
  int   ParseBARO(FILE *f);
  //---------------------------------------------------------------
  int   ParseBody(FILE *f);
  int   SkipField(FILE *f);
};
//=======================================================================================
//  Bt header
//=======================================================================================
struct BtHead {
  char    ident[10];              // File header
  int     ncol;                   // Number of column
  int     nrow;                   // Number of rows
  short   dsiz;                   // Data size
  short   dtyp;                   // Data type
  short   unit;                   // Data unit
  short   zUTM;                   // UTM if relevent
  short   datum;                  // Data source
  double  xlef;                   // Left extend
  double  xrig;                   // Right extend
  double  xbot;                   // Bottom extend
  double  xtop;                   // Top extend
  short   proj;                   // projection
  float   scale;                  // Scale factor
};
//=======================================================================================
//  Base structure
//=======================================================================================
struct Corner {
  U_INT ax;
  U_INT ay;
};
//=======================================================================================
//  QGT region
//=======================================================================================
struct QRegion {
  //----QGT ---------------------------------
  U_INT qx0;            // Left side
  U_INT qx1;            // Right side
  U_INT qz0;            // bottom side
  U_INT qz1;            // Top side
  //-----------------------------------------
  U_INT qtx;            // Current qgt
  U_INT qtz;            // Current qgt
  //-----------------------------------------
  U_INT dtx;            // Depart X tile
  U_INT dtz;            // Depart Z tile
  U_INT ftx;            // End X tile
  U_INT ftz;            // End Z tile
  //-----------------------------------------
  U_INT nqx;
  U_INT nqz;
  //-----------------------------------------
  U_INT tx0;
  U_INT tx1;
  U_INT tz0;
  U_INT tz1;
};
//=======================================================================================
//  CLASS CBtParser to decode binary elevation BT files
//=======================================================================================
class CBtParser {
  //---- ATTRIBUTES -----------------------------------------
  FILE   *fp;                       // Disk file
  char   *fname;                    // File name
  BtHead  Head;                     // Header 
  int     fsiz;
  float  *data;
  //---- Limits in arcseconds -------------------------------
  float   lefS;                     // Left side   (including)
  float   rigS;                     // Right side  (excluding)
  float   topS;                     // Top side    (excluding)
  float   botS;                     // Bottom side (including)
  //---------------------------------------------------------
  bool    nxt;
  //----Grid spacing ----------------------------------------
  double  horz;                     // Horizontal in arcsec
  double  vert;                     // Vertical   in arcsec
  //----Target tile array -----------------------------------
  int     tDim;                     // Target array size
  U_INT   step;                     // Vertex step
  //----Limits in detail tiles keys -------------------------
  TC_BOUND mREG;                    // Region Limit
  //-----Working area ---------------------------------------
  QRegion  qReg;                    // QGT region
  int     *elPT;
  //---------------------------------------------------------
  CVector  dot;                     // Current dot
  CVector  SW;                      // SouthWest elevation
  CVector  NW;                      // NorthWest
  CVector  NE;                      // NorthEast
  CVector  SE;                      // SouthEast
  //---- Methods --------------------------------------------
public:
  CBtParser(char *fn);
 ~CBtParser();
 //--- Helpers ----------------------------------------------
 bool   GetQgtKey(U_INT &xk,U_INT &zk);
 bool   NextQgtKey();
 U_INT  NextVertexKey(U_INT vk);
 U_INT  NextTileKey(U_INT vk);
 U_INT  IncTileKey (U_INT vk,U_INT inc);
 //----------------------------------------------------------
 float  GetElevation(int c,int r);
 void   ComputeTileLimits();
 bool   VertexElevation(U_INT vx,U_INT vz);
 //----------------------------------------------------------
 bool   LowerTriangle();
 bool   UpperTriangle();
 //---------------------------------------------------------
 int    ProcessTile(U_INT tx,U_INT tz);
 bool   GetRegionElevation(REGION_REC &reg);
 //---------------------------------------------------------
 inline int Resolution()  {return tDim;}
};
//=======================================================================================
//  CLASS OBJParser to decode ASCII 3D OBJ file format
//=======================================================================================
class COBJparser: public CParser {
	//---- ATTRIBUTES -----------------------------------------
	U_INT			pm;							// Parameter
	OBJ_MATERIAL *mat;				// Current material
	C3Dmodel *model;
	C3DPart  *part;
	char			trfm;						// Transform indicator
	char      rfu1;						// Reserved
	CVector   T;							// Translation vector
	double    S;							// Sinus
	double    C;							// Cosinus
	double    E;							// Scale
	//---- List of space vertices -----------------------------
	std::vector<GN_VTAB *>			vpos;	
	std::vector<GN_VTAB *>			vtex;
	std::vector<GN_VTAB *>			vnor;
	std::vector<OBJ_MATERIAL*>  matQ;
	//---- METHODS --------------------------------------------
public:
	COBJparser(char t);
 ~COBJparser();
	void	Free(char opt);
	//---------------------------------------------------------
	void	SetDirectory(char *d);
	void	SetTransform(CVector T,double C,double S, double e=1);
	//---------------------------------------------------------
	OBJ_MATERIAL *GetMaterial(char *mn);
	void		 ExtendParts(C3DPart *P);
	//---------------------------------------------------------
	void	   BuildW3DPart();
	C3DPart *BuildOSMPart(char dir);
	C3DPart *BuildMATPart(char dir);
	int			 TransformVerticeStrip(GN_VTAB **dst);
	//--- Free all except material queues ---------------------
	void		 Reduce()  {Free(0);	}
	//---------------------------------------------------------
	int		Decode(char *fn, char t);
	bool	ParseLibrary(char *s);
	bool	ParseMaterial(char *s);
	bool	ParseName(char *s);
	bool	Parse3Vertex(char *s);
	bool	Parse3TCoord(char *s);
	bool	Parse3Normes(char *s);
	bool	Parse3NFaces(char *s);
	bool	Parse4Faces(char *s);
	bool	Parse3Faces(char *s);
	//----------------------------------------------------------
	bool	BuildTriangleVertex(int dst, U_INT nv, U_INT nt);
	bool	BuildTriangleVertex(int dst, U_INT vt, U_INT nt,U_INT nm);
	//----------------------------------------------------------
	char *TextureName()	{return txd.name;}
};
//=======END OF FILE ============================================================================
#endif FILEPARSER_H