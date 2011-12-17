/*
 * TerrainTexture.cpp
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
#include "../Include/Globals.h"
#include "../Include/FlyLegacy.h"
#include "../Include/TerrainCache.h"
#include "../Include/TerrainElevation.h"
#include "../Include/TerrainTexture.h"
#include "../Include/Airport.h"
#include "../Include/Utility.h"
#include "../Include/FreeImage.h"
#include "../Include/FileParser.h"
#include "../Include/Fui.h"

//==========================================================================
U_CHAR Black[] = {0,0,0,0};
//===================================================================================
//  Ocean texture coordinates
//===================================================================================
TC_TCOORD coordSEA[] = {
  {  0,   0},
  {0.5f,  0},
  {0.5f,0.5f},
  {  0, 0.5f},
};
//--------------------------------------------------------------------------
TC_TCOORD normSEA[] = {
  {0,0},
  {1,0},
  {1,1},
  {0,1},
};
//===================================================================================
//  Plane reference for auto gen coordinates
//===================================================================================
GLfloat sPlan[] = {1,0,0,0};
GLfloat tPlan[] = {0,0,1,0};
//==========================================================================
//  Texture size = f(resolution)
//==========================================================================
U_INT SizeRES[] = {
   64 * 64,                               // 0 low
  128 * 128,                              // 1 medium
  256 * 256,                              // 2 hight
  128 * 128,                              // 3 EPD Medium
};
//==========================================================================
//  Texture side = f(resolution)
//==========================================================================
U_SHORT SideRES[] = {
  64,                               // 0 low
  128,                              // 1 medium
  256,                              // 2 hight
  128,                              // 3 EPD Medium
};
//==========================================================================
//  DTO factor:  Origin of real texture
//==========================================================================
float dtoRES[TC_MAX_TEX_RES] = {
  (float(8)/ 64),                 // 0 low 64
  (float(8)/128),                 // 1 medium (8/128)
  (float(8)/256),                 // 2 Hight
  (float(4)/128),                 // 3 EPD
};
//==========================================================================
//  DTS factor:  Size of real texture
//==========================================================================
float dtsRES[TC_MAX_TEX_RES] = {
  (float(48) / 64),               // 0 low 64
  (float(112)/128),               // 1 medium (112/128)
  (float(240)/256),               // 2 Hight
  (float(120)/128),               // 3 EDP
};
//=============================================================================
//  RUNWAY KEYS orderd by Ground type
//=============================================================================
U_INT RunwayKEY[] = {
   0,
  (GROUND_CONCRETE) | (TC_HIGHTR << 8) , // 1 Cement
  (GROUND_ASPHALT ) | (TC_HIGHTR << 8) , // 2 Asphalt
  (GROUND_TURF    ) | (TC_HIGHTR << 8) , // 3 turf
  (GROUND_DIRT    ) | (TC_HIGHTR << 8) , // 4 dirt
  (GROUND_GRAVEL  ) | (TC_HIGHTR << 8) , // 5 gravel
  (GROUND_METAL   ) | (TC_HIGHTR << 8) , // 6 metal
  (GROUND_SAND    ) | (TC_HIGHTR << 8) , // 7 sand
  (GROUND_WOOD    ) | (TC_HIGHTR << 8) , // 8 wood
  (GROUND_WATER   ) | (TC_HIGHTR << 8) , // 9 water
  (GROUND_MATS    ) | (TC_HIGHTR << 8) , // 10 misc
  (GROUND_SNOW    ) | (TC_HIGHTR << 8) , // 11 snow
  (GROUND_ICE     ) | (TC_HIGHTR << 8) , // 12 Ice
  (GROUND_GROOVED ) | (TC_HIGHTR << 8) , // 13 Groove
  (GROUND_TREATED ) | (TC_HIGHTR << 8) , // 14 Treated
};
//=============================================================================
//  RUNWAY KEYS orderd by Ground type
//=============================================================================
U_INT RwyKEY[] = {
  0,
  'RCMH',           // 1 Runway concrete
  'RASH',           // 2 Runway asphalt
  'R1TH',           // 3 turf: other
  'R1TH',           // 4 dirt
  'R1TH',           // 5 gravel
  'R2TH',           // 6 metal
  'R1TH',           // 7 sand
  'R1TH',           // 8 wood
  'R2TH',           // 9 water
  'R2TH',           // 10 misc
  'R2TH',           // 11 snow
  'R2TH',           // 12 ice
  'R2TH',           // 13 groove
  'R2TH',           // 14 traeted
};
//=============================================================================
//  Runway File name
//  Runway that have only one texture are named xxxxR0
//  Runway that have 3 textures are named xxxxR1 xxxxxR2 xxxxxR3 
//  See also MAIN.CPP for ground abreviation for GPS
//=============================================================================
char *RunwayNAM[] = {
  0,                              // Undefined
  "RWYCEM",                       // 1 Cement
  "RWYASF",                       // 2 Asphalt
  "RWYTRF",                       // 3 Turf
  "RWYDRT",                       // 4 Dirt
  "RWYGRV",                       // 5 Gravel
  "RWYMTL",                       // 6 Metal
  "RWYSND",                       // 7 Sand
  "RWYWOD",                       // 8 WOOD
  "RWYWAT",                       // 9 Water
  "RWYMAT",                       // 10 MAterial
  "RWYSNW",                       // 11 Snow
  "RWYICE",                       // 12 ICE
  "RWYGRV",                       // 13 Groved
  "RWYTRD",                       // 14 Traeted
};

//=============================================================================
//  Light Color file name
//=============================================================================
char *LiteNAM[] = {
  "FLARE",                        // 0 FLARE
  "WHTLITE",                      // 1 White
  "REDLITE",                      // 2 RED
  "GRNLITE",                      // 3 GREEN
  "BLULITE",                      // 4 BLUE
  "YLWLITE",                      // 5 YELLOW
  "GLOBE",                        // 6 GLOBE
  "HEAD",                         // 7 Head
};
//=============================================================================
//  File extension
//=============================================================================
char *FileEXT[] = {
  "PNG",          // TC_FILE_PNG
  "JPG",          // TC_FILE_JPG
  "BMP",          // TC_FILE_BMP
  "TGA",          
  "TIF",
};
//=============================================================================
//  File Format
//=============================================================================
FREE_IMAGE_FORMAT FileFFM[] = {
  FIF_PNG,
  FIF_JPEG,
  FIF_BMP,
  FIF_TARGA,
  FIF_TIFF,
};
//=============================================================================
//  File OPTIONS
//=============================================================================
int FileOPT[] = {
  PNG_IGNOREGAMMA,
  JPEG_ACCURATE,
  BMP_DEFAULT,
  TARGA_DEFAULT,
  TIFF_DEFAULT,
};
//=============================================================================
// Memory count
//=============================================================================
CMemCount::CMemCount()
{ Dcnt[0] = 0;              // std Day count
  Dcnt[1] = 0;              // std Day count
  Ncnt[0] = 0;              // std nigth count
  Ncnt[1] = 0;              // std nigth count
  dxcnt = 0;              // Day object count
  nxcnt = 0;              // Nit object count
}
//=============================================================================
//  CWaterTexture:  Parameters for Water Texture
//=============================================================================
CWaterTexture::CWaterTexture()
{ xOBJ  = 0;
  rgba  = 0;
}
//------------------------------------------------------------------------
//  Destructor
//------------------------------------------------------------------------
CWaterTexture::~CWaterTexture()
{ //TRACE ("CWaterTexture::~CWaterTexture %p", rgba);
  if (rgba) delete [] rgba;
  glDeleteTextures(1,&xOBJ);
}
//------------------------------------------------------------------------
//  Load water texture
//  NOTE: Alpha chanel is set to the ratio value
//  Ratio is the amount of blending for animated texture over the fixed one
//------------------------------------------------------------------------
void  CWaterTexture::LoadTexture(TEXT_INFO &txd,char rs,U_INT kaf)
{ CArtParser img(res);
  //--------Make RGBA texture for water --------------
  GLubyte *tex  = img.LoadTextureMT(txd);
  res     = rs;
  rgba    = (U_INT*)tex;
  //--------Change alpha chanel ----------------------
  int dim = SizeRES[rs];
  U_INT *src = rgba;
  for (int k=0; k<dim; k++) 
  { U_INT pix   = *src & TC_ONLY_RGB;
          pix  |=  kaf;
         *src++ = pix;
  }
  //--------Assign a texture object ------ -----------
	xOBJ = globals->txw->GetTerraOBJ(0,res,tex);
  //TRACE ("CWaterTexture::LoadTexture %p", tex);
  return;
}
//------------------------------------------------------------------------
//  Get texture copy
//------------------------------------------------------------------------
U_INT *CWaterTexture::GetWaterCopy()
{ int    dim = SizeRES[res];         // Pixel size
  U_INT *buf = new U_INT[dim];
  U_INT *src = rgba;
  U_INT *dst = buf;
  for (int k=0; k<dim; k++) *dst++ = *src++;
  return buf;
}
//=============================================================================
//  CBlendTexture:  Parameters for transition mask
//=============================================================================
CBlendTexture::CBlendTexture()
{ msk = 0;  
}
//---------------------------------------------------------
CBlendTexture::~CBlendTexture()
{ if (msk) delete [] msk;
}
//=============================================================================
//  CSharedTxnTex.  Constructor
//=============================================================================
CSharedTxnTex::CSharedTxnTex(char *name,U_CHAR res)
:CTextureDef()
{ strncpy(Name,name,TC_TEXNAMESIZE);
  Use     = 1;
  Reso[0] = res;
  Blend   = 0;
}
//-----------------------------------------------------------------------------
//  Delete the texture item
//-----------------------------------------------------------------------------
CSharedTxnTex::~CSharedTxnTex()
{ 
}
//=============================================================================
//  Shared 3D texture
//=============================================================================
CShared3DTex::CShared3DTex(char *tn,char tsp)
{ Use       = 1;
  x3d.azp   = tsp;
  x3d.bpp   = 4;
  x3d.xOBJ  = 0;
  x3d.mADR  = 0;
 // strncpy(x3d.path,tn,TC_LAST_INFO_BYTE);
  _snprintf(x3d.path,512,"ART/%s",tn);
  x3d.path[TC_LAST_INFO_BYTE]   = 0;
}
//-----------------------------------------------------------------------------
//  Free resources
//-----------------------------------------------------------------------------
CShared3DTex::~CShared3DTex()
{ if (x3d.xOBJ)  glDeleteTextures(1,&x3d.xOBJ); 
  if (x3d.mADR)  delete [] x3d.mADR;
}
//=============================================================================
//  External for getting the base Detail Tile coordinate in Super Tile
//=============================================================================
TC_INCREMENT SuperDT[];
//=============================================================================
//  CArtParser:  To read RAW texture files and provide all kind of mergers
//                and blending
//  NOTE:  No provision is made to check if memory is allocated for new operator
//         It is expected that further developpment will include exception
//         process
//=============================================================================
CArtParser::CArtParser(char rs)
{ ref     = 0;
  res     = rs;
  rgb     = 0;
  abt     = 1;                // Abort by default
  dim     = 0;
  act     = 0;
  raw     = 0;
  opa     = 0;
  fop     = 0;
  epd     = 0;
  afa     = 0xFF;             // Default alpha 
  lay     = 0;                // No sea layer
	rrv			= 1;								// Reverse row 
  //-----Transition file 1 --------------------------
  Tx1     = 0;
  Ms1     = 0;
  //-----Transition file 2 --------------------------
  Tx2     = 0;
  Ms2     = 0;
  //-----Transition file 3 --------------------------
  Tx3     = 0;
  Ms3     = 0;
}
//--------------------------------------------------------------------
//  Free the resources
//  DO NOT FREE WATER DATA (wtd & wac)
//--------------------------------------------------------------------
CArtParser::~CArtParser()
{ if (rgb)  delete [] rgb;
  if (raw)  delete [] raw;
  if (act)  delete [] act;
  if (opa)  delete [] opa;
  //---Delete transition data only (not the blending mask) --
  if (Tx3 == Tx1) Tx3 = 0;
  if (Tx3 == Tx2) Tx3 = 0;
  if (Tx2 == Tx1) Tx2 = 0;
  if (Tx1) delete [] Tx1;
  if (Tx2) delete [] Tx2;
  if (Tx3) delete [] Tx3;
  //-----------------------------------------------------
}
//--------------------------------------------------------------------
//  Set dimension parameters
//--------------------------------------------------------------------
int  CArtParser::SetSide(int s)
{ side  = s;
  dim   = s * s;
  return (s * s * 4);
}
//--------------------------------------------------------------------
//  Transfer rgb data
//--------------------------------------------------------------------
GLubyte *CArtParser::TransferRGB()
{ GLubyte *adr = (GLubyte*)rgb;
  rgb = 0;
  return adr;
}
//--------------------------------------------------------------------
//  Abort for error
//--------------------------------------------------------------------
void CArtParser::Abort(char *fn,char *er)
{ gtfo("FILE %s %s",fn,er);
  return;
}
//--------------------------------------------------------------------
//  Extract the bitmap for a RGB format
//  With opt = 1 black pixels receive alpha=0 for blending
//--------------------------------------------------------------------
int CArtParser::ConvertRGBA(U_CHAR alf)
{ //-----Extract the bitmap ----------------------------------
  RGBQUAD *pal  = FreeImage_GetPalette(ref);
  if (pal)              return IndxRGBA(alf);
  if (ffm == FIF_PNG)   return ByteRGBA(alf);
  if (ffm == FIF_TIFF)  return ByteTIFF(alf);
  if (ffm == FIF_BMP)   return PixlBGRO(alf);
  if (ffm == FIF_JPEG)  return ByteRGBA(alf);
  if (ffm == FIF_TARGA) return PixlRGBA(alf);
  Abort("ConvertRGBA","Bad format");
  return 1;
}
//--------------------------------------------------------------------
//  Pixel Transfert for TARGA
//	Just get the pixel as it is
//--------------------------------------------------------------------
int CArtParser::PixlRGBA(U_CHAR alf)
{ // rgb has to be freed before any new allocation
  FreeFFF (FIF_TARGA); 
  rgb = new U_INT[dim];
  U_INT *dst = rgb;
	U_INT  afa = (alf)?(0):(0xFF000000);
  for (U_INT z=0; z<htr; z++)
  { U_INT *src = (U_INT*)FreeImage_GetScanLine(ref,z);
    for (U_INT x=0; x<wid; x++)   *dst++ = *src++;
  }
  return 1;
}
//--------------------------------------------------------------------
//  Pixel RGB Only Transfert (BMP invert BGR)
//--------------------------------------------------------------------
int CArtParser::ByteTIFF(U_CHAR alf)
{ // rgb has to be freed before any new allocation
  if (bpp != 24)  gtfo("Unsupported format");
  FreeFFF (FIF_BMP); 
  //----------------------------------------
  U_INT *buf = new U_INT[dim];
  U_INT *dst = buf;
  rgb        = buf;
  U_INT  af1 = (U_INT)0xFF000000;
	U_INT  af2 = (alf << 24);
  for (U_INT z=0; z<(htr-1); z++)
  { BYTE *src = FreeImage_GetScanLine(ref,z);
    for (U_INT x=0; x<wid; x++)
    { char  R   = *src++;
      char  G   = *src++;
      char  B   = *src++;
      U_INT pix = MakeRGBA(R,G,B,0);
      if (pix ) pix |= af1;
			else			pix |= af2;
      *dst++ = pix;
    }
  }
  return 1;
}
//--------------------------------------------------------------------
//  Pixel RGB Only Transfert (BMP invert BGR)
//--------------------------------------------------------------------
int CArtParser::PixlBGRO(U_CHAR alf)
{ // rgb has to be freed before any new allocation
  if (bpp != 24)  gtfo("Unsupported format");
  FreeFFF (FIF_BMP); 
  //----------------------------------------
  U_INT *buf = new U_INT[dim];
  U_INT *dst = buf;
  rgb        = buf;
  U_INT af1 = (U_INT)(0xFF000000);
	U_INT	af2	=	(alf << 24);
  for (U_INT z=(htr-1); z>0; z--)
  { BYTE *src = FreeImage_GetScanLine(ref,z);
    for (U_INT x=0; x<wid; x++)
    { char  B   = *src++;
      char  G   = *src++;
      char  R   = *src++;
      U_INT pix = MakeRGBA(R,G,B,0);
      if (pix ) pix |= af1;
			else			pix |= af2;
      *dst++ = pix;
    }
  }
  return 1;
}
//--------------------------------------------------------------------
//  Byte Transfert
//--------------------------------------------------------------------
int CArtParser::ByteRGBA(U_CHAR alf)
{ U_INT af1 = ((U_INT)0xFF << 24);
  U_INT af2 = (alf << 24);
  rgb  = new U_INT[dim];
  U_INT *dst = rgb;
  for (U_INT z = 0; z < htr; z++)
      { U_CHAR *src = (U_CHAR*)FreeImage_GetScanLine(ref,z);
        for (U_INT x = 0; x < wid; x++) 
        { U_INT  pix  =  0;
             pix |=  *src++;
             pix |= (*src++ <<  8);
             pix |= (*src++ << 16);
						 if (pix)		pix |= af1;
						 else				pix |= af2;
            *dst++ = pix;
        }
      }
  return 1;
}
//--------------------------------------------------------------------
//  Byte Transfert color Indexed
//--------------------------------------------------------------------
int CArtParser::IndxRGBA(U_CHAR alf)
{ 
  RGBQUAD *pal  = FreeImage_GetPalette(ref);
  //----------------------------------------------------------
  U_INT af1		= (U_INT)(0xFF000000);
  U_INT af2   = (alf << 24);
  rgb  = new U_INT[dim];
  U_INT *dst = rgb;
  for (U_INT z = 0; z < htr; z++)
      { U_CHAR *src = (U_CHAR*)FreeImage_GetScanLine(ref,z);
        for (U_INT x = 0; x < wid; x++) 
        { U_INT    pix  = 0;
          U_CHAR   inx  = *src++;
          RGBQUAD *col  = pal + inx;
             pix |=  col->rgbRed;
             pix |= (col->rgbGreen <<  8);
             pix |= (col->rgbBlue  << 16);
             if (pix) pix |= af1;
             else     pix |= af2;
            *dst++ = pix;
        }
      }
  return 1;
}
//--------------------------------------------------------------------
//  No file
//--------------------------------------------------------------------
void CArtParser::NoFile(char *fn)
{ if (abt) gtfo("No such file: %s",fn);
  return;
}
//--------------------------------------------------------------------
// Load a file
//--------------------------------------------------------------------
void CArtParser::TryFILE(char *fnm,U_CHAR **buf,int &sz)
{ FILE *pf = 0;
	if (fopen_s(&pf,fnm,"rb"))		return NoFile(fnm);
	if (fseek(pf,0,SEEK_END))			return NoFile(fnm);
	long rdz = ftell(pf);
	if (fseek(pf,0,SEEK_SET))			return NoFile(fnm);
 *buf = new U_CHAR[rdz];
  sz	= fread(*buf,1,rdz,pf);
	if (sz != rdz)								return NoFile(fnm);
	fclose(pf);
	return;
}
//--------------------------------------------------------------------
// Load POD file
//--------------------------------------------------------------------
void CArtParser::TryaPOD(char *fnm,U_CHAR **buf,int &sz)
{	PODFILE* pod = popen(&globals->pfs, fnm);
  if (0 == pod)		return;
  //--- we have a pod file -------------------------------
 *buf = new U_CHAR[pod->size];
  int rdz = pread(*buf,1,pod->size,pod);
  if (rdz!= pod->size) gtfo("File error: %s",fnm);
  pclose(pod);
  sz = rdz;
	return;
}
//--------------------------------------------------------------------
//  Load a full file in memory
//--------------------------------------------------------------------
bool CArtParser::LoadFFF(char *rnm,char azp,FREE_IMAGE_FORMAT ff)
{	U_CHAR *buf = 0;
	int     rdz = 0;
	TryaPOD(rnm,&buf,rdz);
	if (0 == buf)	TryFILE(rnm,&buf,rdz);
  //----Wrap it with Freeimage --------------------
  FIMEMORY *hmem  = FreeImage_OpenMemory(buf,rdz);
  ffm  = ff;
  ref  = FreeImage_LoadFromMemory(ffm,hmem,fop);
  FreeImage_CloseMemory(hmem);
  delete [] buf;
  if ((0 ==  ref) && abt)  Abort(rnm,"no file");
  if ( 0 ==  ref)          return false;
  side = FreeImage_GetWidth(ref);
  htr  = FreeImage_GetHeight(ref);
  wid  = side;
  dim  = htr * side;
  bpp  = FreeImage_GetBPP(ref);
  type = FreeImage_GetImageType(ref);
  ConvertRGBA(azp);
  FreeImage_Unload(ref);
  ref = 0;
  return true;
}
//--------------------------------------------------------------------
//  Load a free file in the requested resolution
//  tsp:  transparent option 
//--------------------------------------------------------------------
bool CArtParser::LoadFIM(char *rnm,char tsp,FREE_IMAGE_FORMAT ff)
{ ffm  = ff;
  ref  = FreeImage_Load(ffm,rnm,fop);
  if ((0 ==  ref) && abt)  Abort(rnm,"no file");
  if ( 0 ==  ref)          return false;
  side = FreeImage_GetWidth(ref);
  htr  = FreeImage_GetHeight(ref);
  wid  = side;
  dim  = htr * side;
  bpp  = FreeImage_GetBPP(ref);
  type = FreeImage_GetImageType(ref);
  ConvertRGBA(tsp);
  FreeImage_Unload(ref);
  ref = 0;
  return true;
}
//--------------------------------------------------------------------
//  Load a pod file into memry, then request Free image
//  to decode it
//--------------------------------------------------------------------
bool CArtParser::LoadPOD(char *rnm,char tsp)
{ PODFILE *pod = popen (&globals->pfs, rnm);
  if (0 == pod)   return false;
  int   lgr = pod->size;
  char *buf = new char[lgr];
  int   rdl = pread(buf,1,lgr,pod);
  if (rdl != lgr)                               Abort(rnm,"Bad size");
  mem = FreeImage_OpenMemory((BYTE*)buf,lgr);
  ref = FreeImage_LoadFromMemory(ffm,mem,fop);
  if (0 == ref)                                 Abort(rnm,"no file");
  side = FreeImage_GetWidth(ref);
  dim       = side * side;
  bpp       = FreeImage_GetBPP(ref);
  type      = FreeImage_GetImageType(ref);
  ConvertRGBA(tsp);
  delete buf;
  FreeImage_Unload(ref);
  FreeImage_CloseMemory(mem);
  ref = 0;
  return true;
}
//--------------------------------------------------------------------
//  Load the RAW and ACT File
//  opt when 0 does not look for OPA file
//--------------------------------------------------------------------
GLubyte  *CArtParser::LoadRaw(TEXT_INFO &txd,char opt)
{ char *rnm = txd.path;
  //---Free resource when reusing parser -----------------------
  if (raw) {delete [] raw; raw = 0;}
  if (act) {delete [] act; act = 0;}
  if (opa) {delete [] opa; opa = 0;}
  //-----Open the raw pixels -----------------------------------
  dot   = strstr(rnm,".");
  strcpy(dot,".RAW");
  PODFILE *fraw = popen (&globals->pfs, rnm);
  if ( 0 == fraw)                               return 0;
  dim     = fraw->size;
  side    = GetSize(dim,rnm);
  txd.wd  = side;
  txd.ht  = side;
  raw     = new GLubyte[dim];
  if ( 1 != pread (raw, dim, 1, fraw))          Abort(rnm,"Can't read(raw)");
  pclose (fraw);
  //-----Open the color table ----------------------------------
  strcpy(dot,".ACT");
  PODFILE *fact = popen (&globals->pfs, rnm);
  act   = new GLubyte[TC_ACTSIZE];
  if ( 1 != pread (act, TC_ACTSIZE, 1, fact))   Abort(rnm,"Can't read(act)");
  pclose (fact);
  if (0== opt)                                  return RGBAInvert(0xFF);
  //----Check for OPA file -------------------------------------
  strcpy(dot,".OPA");
  if (!pexists (&globals->pfs, rnm))            return RGBAInvert(0xFF);
  PODFILE *fopa = popen (&globals->pfs, rnm);
  opa   = new GLubyte[dim];
  if ( 1 != pread (opa, dim, 1, fopa))          Abort(rnm,"Can't read(opa)");
  pclose (fopa);
  return RGBAInvert(0xFF);
}
//--------------------------------------------------------------------
//  Transform image into RGBA .  Invert rows
//  When index is 0 then alpha chanel is 'tsp' value
//  tsp = 0xFF      No transparency
//  tsp = 0x00      Transparency
//--------------------------------------------------------------------
GLubyte *CArtParser::RGBAInvert(U_CHAR tsp)
{ if (0 == rrv)   return 0;														// No inversion
  GLubyte *img  = new GLubyte[dim << 2];              // New Image in bytes
  GLubyte *src  = raw + dim + side;                   // Source image = (last+1)row
  GLuint  *dst  = (GLuint*)img;                       // Destination
  //-----------------------------------------------------------------
  GLuint   pix  = 0;                                  // RGBA pixel
  GLuint   inx  = 0;                                  // color index
	GLuint	 af1  = afa << 24;													// Alpha 1
	GLuint	 af2	= tsp << 24;													// alpha 2
  //------------------------------------------------------------------
  U_INT    row2 = (side << 1);                        // Twice side
  for (U_INT k = 0; k != side; k++)                   // row count
    { src -= row2;                                    // Back 2 rows on image
      for (U_INT j = 0; j != side; j++)               // column count
      { inx   = *src++;                               // palette index
				pix		= (inx)?(af1):(af2);
        inx   = (inx << 2) - inx;                     // Color entry
        pix  |= (act[inx++]);                         // R composite
        pix  |= (act[inx++] << 8);                    // G composite
        pix  |= (act[inx]   << 16);                   // B composite
       *dst++ = (pix);
       }
    }
  return img;
}
//--------------------------------------------------------------------
//  Check supported size
//--------------------------------------------------------------------
int CArtParser::GetSize(int tot,char *fn)
{ if (tot ==   1024)  return  32;
  if (tot ==   4096)  return  64;
  if (tot ==  16384)  return 128;
  if (tot ==  65536)  return 256;
  if (tot == 262144)  return 512;
  if (tot == 1048576) return 1024;
  Abort(fn,"Size not supported");
  return 0;
}
//--------------------------------------------------------------------
//  Transform image into RGBA with or without OPA
//  When index is 0 then alpha chanel is 'tsp' value
//  tsp = 0xFF      No transparency
//  tsp = 0x00      Transparency
//--------------------------------------------------------------------
GLubyte *CArtParser::RGBAMerger(U_CHAR tsp)
{ if (0 == raw) return 0;
  int lgb       = dim << 2;                   // 4 times size
  GLubyte *img  = new GLubyte [lgb];          // New RGBA image
  GLubyte *src  = raw;                        // Source byte = first row         
  GLubyte *dst  = img;                        // Destination
  U_SHORT  col  = 0;                          // Color index
  U_CHAR   alf  = 0xFF;                       // Alpha chanel
  U_CHAR   ind;                               // Pixel index
  for   (U_INT k = 0; k != side; k++)         // Row count
    { for (U_INT j = 0; j != side; j++)       // Line count
      {ind    = *src++;                       // Pixel index
       col    = (ind << 1) + ind;             // 3 times pix
       alf    = (ind)?(0xFF):(tsp);           // 0 if ind 0
      *dst++  = act[col++];                   // R composite
      *dst++  = act[col++];                   // G composite
      *dst++  = act[col];                     // B composite
      *dst++  = alf;                          // A composite
      }
  }
  return img;
}

//--------------------------------------------------------------------
//  Transform image into RGBA with or without OPA
//  When index is 0 then alpha chanel is 'tsp' value
//  afz = 0xFF      No transparency
//  afz = 0x00      Transparency
//--------------------------------------------------------------------
GLubyte *CArtParser::ModlMerger(U_CHAR afz)
{ if (0 == raw) return 0;
  int lgb       = dim << 2;                   // 4 times size
  GLubyte *img  = new GLubyte [lgb];          // New RGBA image
  GLubyte *src  = raw;                        // Source byte = first row         
  GLubyte *dst  = img;                        // Destination
  GLubyte *msk  = opa;                        // OPA file
  U_SHORT  col  = 0;                          // Color index
  U_CHAR   alf  = 0xFF;                       // Alpha chanel
  U_CHAR   ind;                               // Pixel index
  U_INT   *sva;                               // Save adress
  for   (U_INT k = 0; k != side; k++)         // Row count
    { for (U_INT j = 0; j != side; j++)       // Line count
      {ind    = *src++;                       // Pixel index
       col    = (ind << 1) + ind;             // 3 times pix
       sva    = (U_INT*)dst;                  // Save dest
       alf    = (ind)?(0xFF):(afz);           // 0 if ind 0
      *dst++  = act[col++];                   // R composite
      *dst++  = act[col++];                   // G composite
      *dst++  = act[col];                     // B composite
      *dst++  = alf;                          // A composite
      if (0 == msk)   continue;
      if (*msk++)     continue;
     *sva  = 0;
      }
  }
  return img;
}
//--------------------------------------------------------------------
//  Write a bitmap from buffer
//  
//--------------------------------------------------------------------
void CArtParser::WriteBitmap(FREE_IMAGE_FORMAT ff,char *fn,int wd,int ht,U_CHAR *buf)
{ ffm = ff;
  ref = FreeImage_AllocateT(FIT_BITMAP, wd, ht, 32);
  //----Fill the bitmap ---------------------------------
  RGBQUAD rgba;
  U_CHAR  *pix = buf;
  for   (U_INT j=0; j<(U_INT)ht; j++)
  { for (U_INT k=0; k<(U_INT)wd; k++)
    { rgba.rgbRed      = *buf++;
      rgba.rgbGreen    = *buf++;
      rgba.rgbBlue     = *buf++;
      rgba.rgbReserved = *buf++;
      if (!FreeImage_SetPixelColor(ref, k, j, &rgba))  gtfo("cannot write bitmap");
    }
  }
  FreeImage_Save(ff,ref, fn,fop);
  FreeImage_Unload(ref);
  return;
}
//-----------------------------------------------------------------------------
//  Merge Water texture 
//-----------------------------------------------------------------------------
GLubyte *CArtParser::MergeWater(GLubyte *tex)
{ //--- Merge water texture to texture where OPA is 0 -------------
  int      row  = dim + side;                         // Position to (last + 1) row
  GLubyte *opm  = opa + row;                          // OPA    image = (last+1)row
  GLuint  *wat  = (GLuint*)wtx;                       // Water  image = (last+1)row
  GLuint  *dst  = (GLuint*)tex;                       // Destination
  //-----------------------------------------------------------------
  U_INT    row2 = (side << 1);                        // Twice side
  for (U_INT k = 0; k != side; k++)                   // row count
    { opm -= row2;                                    // Back 2 rows on OPA
      for (U_INT j = 0; j != side; j++)               // column count
      { U_INT pix = *dst & 0x00FFFFFF;                // Only RGBA
        U_INT h2o = *wat++;                           // Only Water
        if (0 == *opm++) pix = h2o;                   // Set Water pixel
        *dst++ = pix;;                                // Next destination
       }
    }
  return tex;
}
//-----------------------------------------------------------------------------
//  Add alpha channel to the texture
//  Alpha chanel is the pixel luminance when pixel exist
//-----------------------------------------------------------------------------
GLubyte *CArtParser::MergeNight(GLubyte *tex)
{ GLuint  *dst  = (GLuint*)tex;                       // Destination
  U_INT    pix  = 0;                                  // Pixel entry
  U_CHAR   R    = 0;                                  // Red composite
  U_CHAR   G    = 0;                                  // Green composite
  U_CHAR   B    = 0;                                  // Blue composite
  U_CHAR   A    = 0;                                  // Alpha chanel
  for (U_INT k = 0; k != side; k++)                   // row count
    { for (U_INT j = 0; j != side; j++)               // column count
      { pix   = *dst;
        R     = pix;
        G     = pix >> 8;
        B     = pix >> 16;
        pix  &= 0x00FFFFFF;
        A     = ((R*38) + (G*74) + (B*16)) >> TC_BY128;
        if (A > 60) (A = 255);                        // Boost light
        pix  |= (A << 24);
       *dst++ = pix;                                  // Night pixel
       }
    }
  return tex;
}
//-----------------------------------------------------------------------------
//  Return a full day texture RGBA
//  opt = 1 =>  This texture is a dedicated texture that may be associated
//               with a OPA mask for water merging
//	TODO:  Do something when texture is missing
//-----------------------------------------------------------------------------
GLubyte *CArtParser::GetRawTexture(TEXT_INFO &txd,char opt)
{ GLubyte   *tex = 0;
  afa = 0;
  tex = LoadRaw(txd,opt);
  if (0 == opa) return tex;
  MergeWater(tex);
  return tex;
}

//-----------------------------------------------------------------------------
//  Return a texture RGBA for 3D model
//-----------------------------------------------------------------------------
GLubyte *CArtParser::GetModTexture(TEXT_INFO &txd)
{ NoReverse();
	LoadRaw(txd,1);                 // With optional OPA
  GLubyte *tex = ModlMerger(txd.azp);
  return tex;
}
//--------------------------------------------------------------------
//  Check if water texture is requested
//--------------------------------------------------------------------
GLubyte *CArtParser::TransitionTexture(TEXT_INFO &txd)
{ if (txd.type != 0x0C)   return LoadTextureFT(txd);
  //--- return copy of water texture -----------------------------
  GLubyte *buf = (GLubyte*)globals->txw->GetWaterCopy(txd.res);
  return buf;
}
//-----------------------------------------------------------------------------
//  Return a night texture RGBA 
//-----------------------------------------------------------------------------
GLubyte *CArtParser::GetNitTexture(TEXT_INFO &txd)
{ SqlTHREAD *sql = globals->sql;
  GLubyte   *tex = 0;
  DontAbort();
  if (sql->SQLtex()){ tex = sql->GetGenTexture(txd);
                      SetSide(txd.wd);
                    }
  else              { tex = LoadRaw(txd,0);    }
  return tex;
}

//--------------------------------------------------------------------
//  Load texture either from SQL or POD in file thread
//--------------------------------------------------------------------
GLubyte *CArtParser::LoadTextureFT(TEXT_INFO &txd)
{ afa            = 0;
	GLubyte   *tex = globals->sql->GetGenTexture(txd);
	if (0 == tex)  tex = LoadRaw(txd,0);			
  SetSide(txd.wd);
  if (0 == tex) gtfo("BAD TEXTURE NAME: %s",txd.path);
  txd.mADR = tex;
  return tex;
}
//--------------------------------------------------------------------
//  Load texture either from SQL or POD in main thread
//--------------------------------------------------------------------
GLubyte *CArtParser::LoadTextureMT(TEXT_INFO &txd)
{ afa		= 0;
  GLubyte *tex = globals->sqm->GetGenTexture(txd);
	if (0 == tex) tex = LoadRaw(txd,0);
  SetSide(txd.wd);
  if (0 == tex) gtfo("BAD TEXTURE NAME: %s",txd.path);
	txd.mADR = tex;
  return tex;
}
//--------------------------------------------------------------------
//  Load Transition 1 Texture
//--------------------------------------------------------------------
void  CArtParser::InitTransitionT1(TEXT_INFO &txd)
{ Tp1 = txd.type;
  Ms1 = txd.mADR;
  Tx1 = TransitionTexture(txd);
  return;
}
//--------------------------------------------------------------------
//  Load Transition 2 Texture
//--------------------------------------------------------------------
void  CArtParser::InitTransitionT2(TEXT_INFO &txd)
{ Tp2 = txd.type;
  Ms2 = txd.mADR;
	if (Tp2 == Tp1)		{Tx2 = Tx1; return;}
  Tx2 = TransitionTexture(txd);
  return;
}
//--------------------------------------------------------------------
//  Load Transition 3 Textures
//--------------------------------------------------------------------
void  CArtParser::InitTransitionT3(TEXT_INFO &txd)
{ Tp3 = txd.type;
  Ms3 = txd.mADR;
	if (Tp3 == Tp1)		{Tx3 = Tx1; return;}
	if (Tp3 == Tp2)		{Tx3 = Tx2;	return;}
  Tx3 = TransitionTexture(txd);
  return;
}
//--------------------------------------------------------------------
//  Transform image into RGBA with up to 3 Transitions 
//	This function uses the following parameters
//	st0 is the original texture to blend with the others
//	st1-ms1:	Texture T1 and associated mask
//	st2-ms2:	Texture T2 and associated mask
//	st3-ms3:	Texture T3 and associated mask
//	To blend colors for a given stage, the mask is used as an alpha chanel
//		Thus with the folowing values
//		Ci:		Input color for the current stage
//		Cn:		Color from texture stn to blend
//		Mn:		Mask associated with stn
//		An intermediate color Co is computed (equation 1):
//    Co= (Ci * Mn) + Ci * (255 - Mn)
//	Color Co is then used as input color to the next blending
//	stage.
//	NOTE on optimization:
//		Each color component must be in [0-255] range.  We used
//		integer multiply for equation 1 so the result should be divided
//		by 256.  This is achieved by a right shift to save computation 
//		time.
//--------------------------------------------------------------------
GLubyte *CArtParser::Mixer(TEXT_INFO &txd)
{ GLubyte *tex  = TransitionTexture(txd);
  GLubyte *st0  = tex;                        // first  source byte 
  GLubyte *st1  = Tx1;                        // Second source byte 
  GLubyte *st2  = Tx2;                        // Third source  byte 
  GLubyte *st3  = Tx3;                        // Forth source  byte 
  GLubyte *ms1  = Ms1;                        // Alpha  source byte 
  GLubyte *ms2  = Ms2;                        // Alpha source  byte 
  GLubyte *ms3  = Ms3;                        // Alpha source
  GLubyte *dst  = tex;                        // Destination
  GLubyte  A0   = 0;                          // Alpha channel
  //----Color components ---------------------------------------------------
  U_INT R0,G0,B0;
  U_INT R1=0,G1=0,B1=0;
  U_INT R2=0,G2=0,B2=0;
  U_INT R3=0,G3=0,B3=0;
  //------------------------------------------------------------------------
  for   (U_INT k = 0; k != dim; k++)      // Row count
      { U_INT A1 = (ms1)?(*ms1++):0;      // Alpha 1 
        U_INT A2 = (ms2)?(*ms2++):0;      // Alpha 2
        U_INT A3 = (ms3)?(*ms3++):0;      // alpha 3
        U_INT C1 = (255 - A1);            // 1 - A1
        U_INT C2 = (255 - A2);            // 1 - A2
        U_INT C3 = (255 - A3);            // 1 - A3
        //---Load composite from Original texture -------------------------
        R0 = *st0++;
        G0 = *st0++;
        B0 = *st0++;
        A0 = *st0++;
        //---Load composite for Transition 1 ------------------------------
        if (ms1)
        { R1 = *st1++;
          G1 = *st1++;
          B1 = *st1++;
        }
        //---Load composite for Transition 2 ------------------------------
        if (ms2)
        { R2 = *st2++;
          G2 = *st2++;
          B2 = *st2++;
        }
        //---Load composite for Transition 3 ------------------------------
        if (ms3)
        { R3 = *st3++;
          G3 = *st3++;
          B3 = *st3++;
        }
        //-----Compute the color result for first transition---------------
        if (A1)                              
        { R0 = ((R0*C1) + (R1*A1)) >> TC_BY256;
          G0 = ((G0*C1) + (G1*A1)) >> TC_BY256;
          B0 = ((B0*C1) + (B1*A1)) >> TC_BY256;
          A0 = *st1;
        }
        //-----Compute the color result for second transition--------------
        if (A2)                              
        { R0 = ((R0*C2) + (R2*A2)) >> TC_BY256;
          G0 = ((G0*C2) + (G2*A2)) >> TC_BY256;
          B0 = ((B0*C2) + (B2*A2)) >> TC_BY256;
          A0 = *st2;
        }
        //-----Compute the color result for third transition--------------
        if (A3)                              
        { R0 = ((R0*C3) + (R3*A3)) >> TC_BY256;
          G0 = ((G0*C3) + (G3*A3)) >> TC_BY256;
          B0 = ((B0*C3) + (B3*A3)) >> TC_BY256;
          A0 = *st3;
        }
        //-----Store color in texture -------------------------------------
        *dst++ = R0;
        *dst++ = G0;
        *dst++ = B0;
        //-----Store the ALPHA chanel -------------------------------------
        *dst++ = A0;
        //-----ALPHA bypass --(compute destination alpha) ---------------------
        if (st1)  st1++;
        if (st2)  st2++;
        if (st3)  st3++;
  }
  return tex;
}
//--------------------------------------------------------------------
//  Check file Type from Name  
//--------------------------------------------------------------------
int CArtParser::GetFileType(char *name)
{ char *dot = strrchr(name,'.');
  if (0 == dot) Abort(name,"incorrect type");
  if (stricmp(dot,".PNG") == 0)  return TC_FILE_PNG;
  if (stricmp(dot,".JPG") == 0)  return TC_FILE_JPG;
  if (stricmp(dot,".BMP") == 0)  return TC_FILE_BMP;
  if (stricmp(dot,".TIF") == 0)  return TC_FILE_TIF;
  if (stricmp(dot,".TGA") == 0)  return TC_FILE_TGA;
  Abort(name,"Type unsupported");
  return 0;
}
//--------------------------------------------------------------------
//  Return a  texture  
//--------------------------------------------------------------------
GLubyte *CArtParser::GetAnyTexture(TEXT_INFO &xdf)
{ int type = GetFileType(xdf.path);
  FREE_IMAGE_FORMAT fff = FileFFM[type];
  SetOption(FileOPT[type]);
  LoadFFF(xdf.path,xdf.azp,fff);
  GLubyte *rgba = (GLubyte*)rgb;
  xdf.ht        = htr;
  xdf.wd        = wid;
  xdf.mADR      = rgba;
  rgb           = 0;
  return rgba;
}
//--------------------------------------------------------------------
//  free a returned texture with GetAnyTexture
//--------------------------------------------------------------------
void CArtParser::FreeAnyTexture(GLubyte *text)
{ if (text) delete [] text;
}
//--------------------------------------------------------------------
//  free a returned texture with GetAnyTexture
//--------------------------------------------------------------------
void CArtParser::FreeFFF(FREE_IMAGE_FORMAT ff)
{ switch (ff) {
  case FIF_TARGA :
    if (rgb) {delete [] rgb; rgb = 0;}
    break;
  }
}
//=============================================================================
//  CTextureWard:  Storage for all textures in cache
//=============================================================================
CTextureWard::CTextureWard(TCacheMGR *mgr,U_INT t)
{ int   ind;
  tcm     = mgr;
  tr      = t;
  Night   = 'D';
  sqm     = globals->sqm;
  usq     = sqm->SQLtex();
  nPic    = 0;
  //-----------------------------------------------------------
  strcpy(xld.path,"SYSTEM/GLOBE/*");        // Texture path
  //------Register in globals ---------------------------------
  globals->txw = this;
  //------Clear Canvas ----------------------------------------
  memset(Cnv1,0,TC_TEXTMAXDIM);
  Canvas  = Cnv1;
  //------Init bounding box -----------------------------------
  iBox.xmin = 4096;
  iBox.zmin = 4096;
  iBox.xmax = 0;
  iBox.zmax = 0;
  //------Init Counters ---------------------------------------
  Nb3DT   = 0;
  NbSHD   = 0;
  NbCUT   = 0;
  //------Init working area -----------------------------------
  gx      = 0;
  gz      = 0;
  //-------------------------------------------------------------
  pthread_mutex_init (&txnMux,  NULL);
  pthread_mutex_init (&t3dMux,  NULL);
  pthread_mutex_init (&ctrMux,  NULL);
  //-------------------------------------------------------------
  alfa[0]   = 0;
  alfa[1]   = 255;
  //------Runway texture objects --------------------------------
  RwyTX[0]  = 0;
  RwyTX[1]  = 0;
  RwyTX[2]  = 0;
  RwyTX[3]  = 0;
  //--------Read terrain options ------------------------------------
  int i = 1;                                    // Default is set
  GetIniVar ("Terrain", "NightTextures", &i);
  NT    = (i == 1);
  //--------Initialize Library --------------------------------------
  FreeImage_Initialise(TRUE);
	anSEA	= 0;
  //----Check for animated water ------------------------------------
  //	int NoAW     = 0;
  //	GetIniVar("Sim", "NoAnimatedWater", &NoAW);
  //	if (NoAW) globals->noAWT++;
  //anSEA = new CWater3D(256);
  kaf   = int (double(1 - 0.62f) * 255);
  kaf   = kaf << 24;
  //--------Build the medium Water Texture --------------------------
  //    ONLY ONE WATER TEXTURE IS USED
  strcpy(xld.path + 13,"0C014D.");
  Tank[TC_MEDIUM].LoadTexture(xld,TC_MEDIUM,kaf);
  //--------Build the hight Water Texture --------------------------
  //    ONLY ONE WATER TEXTURE IS USED
  strcpy(xld.path + 13,"0C015D.");
  Tank[TC_HIGHTR].LoadTexture(xld,TC_HIGHTR,kaf);
  //--------Load right mask medium transition -------------------------
  ind       = TC_BLENDRT | TC_MEDIUM;
  LoadMaskTexture(ind,"RGHBLEND4",TC_MEDIUM_DIM);
  //-------Load right mask hight transition ---------------------------
  ind       = TC_BLENDRT | TC_HIGHTR;
  LoadMaskTexture(ind,"RGHBLEND5",TC_HIGHTR_DIM);
  //-------Load bottom mask medium transition -------------------------
  ind       = TC_BLENDBT | TC_MEDIUM;
  LoadMaskTexture(ind,"BOTBLEND4",TC_MEDIUM_DIM);
  //-------Load bottom mask hight transition -------------------------
  ind       = TC_BLENDBT | TC_HIGHTR;
  LoadMaskTexture(ind,"BOTBLEND5",TC_HIGHTR_DIM);
  //-------Load corner mask medium transition -------------------------
  ind       = TC_BLENDCN | TC_MEDIUM;
  LoadMaskTexture(ind,"CNRBLEND4",TC_MEDIUM_DIM);
  //-------Load corner mask hight transition -------------------------
  ind       = TC_BLENDCN | TC_HIGHTR;
  LoadMaskTexture(ind,"CNRBLEND5",TC_HIGHTR_DIM);
  //-------Load taxiways texture ------------------------------------
  LoadTaxiTexture("TAXICMNT",(U_CHAR)0xFF);
  //-------Load packed runway--------------------
  LoadRwyTexture('RCMH',"RWYCEMH",(U_CHAR)0xFF);
  LoadRwyTexture('RASH',"RWYASFH",(U_CHAR)0xFF);
  LoadRwyTexture('R1TH',"RWYOTRH",0);
  //-------Load light textures -------------------
  LoadLightTexture(TC_FLR_TEX);
  LoadLightTexture(TC_WHI_TEX);
  LoadLightTexture(TC_GLO_TEX);
  LoadLightTexture(TC_HDR_TEX);
}
//==============================================================================
//  Free all resources
//  TODO:  Free all reserved and shared bitmaps
//==============================================================================
CTextureWard::~CTextureWard()
{ FreeImage_DeInitialise();
  std::map<U_INT,CSharedTxnTex*>::iterator ita;
  for (ita=txnMAP.begin();ita!=txnMAP.end();ita++)
  { CSharedTxnTex *shx = (*ita).second;
    delete shx;
  }
  txnMAP.clear();
  std::map<std::string,CShared3DTex*>::iterator itb;
  for (itb=t3dMAP.begin();itb!=t3dMAP.end();itb++)
  { CShared3DTex *wdx = (*itb).second;
    delete wdx;
  }
  t3dMAP.clear();
  globals->txw = 0;
  if (anSEA)	delete anSEA;
}
//----------------------------------------------------------------------
//  ABORT ERROR
//----------------------------------------------------------------------
int  CTextureWard::Abort(char *msg,char *mse)
{ gtfo("TEXTURE WARD: %s %s",msg,mse);
  return 0;
}
//----------------------------------------------------------------------
//  WARNING ERROR
//----------------------------------------------------------------------
int  CTextureWard::Warn(char *msg,char *mse)
{ WARNINGLOG("TEXTURE WARD: %s %s",msg,mse);
  return 0;
}
//----------------------------------------------------------------------
//  Trace number of textures
//----------------------------------------------------------------------
void CTextureWard::TraceCTX()
{ pthread_mutex_lock (&ctrMux);
//  TRACE("tcm: TEXTURES: SHD=%04d RAW=%04d EPD=%04d GEN=%04d",NbSHD,NbRAW,NbEPD,NbGEN);
  pthread_mutex_unlock (&ctrMux);
  return;
}
//----------------------------------------------------------------------
//  Return color luminance
//----------------------------------------------------------------------
float CTextureWard::GetLuminance(float *col)
{ float R = col[0] * 0.38f;
  float G = col[1] * 0.74f;
  float B = col[2] * 0.16f;
  return (R + G + B);
}
//----------------------------------------------------------------------
//  Return texture parameters
//----------------------------------------------------------------------
void CTextureWard::GetTextParam(char res,float *to, float *tu)
{ *to = dtoRES[res];
  *tu = dtsRES[res];
  return;
}
//--------------------------------------------------------------------
//  Load the MSK file
//--------------------------------------------------------------------
GLubyte *CTextureWard::LoadMSK(char *msn,int side)
{ int dim = (side * side);
  if (!pexists (&globals->pfs, msn))    Abort(msn,"File Not found");
  PODFILE *fmsk = popen (&globals->pfs, msn);
  GLubyte *tex = new GLubyte[dim];
  if ( 1 != pread (tex, dim, 1, fmsk))  Abort(msn,"Can't read");
  pclose (fmsk);
  return tex;
}
//-----------------------------------------------------------------------------
//  Make a Terrain Key
//    char 0:   Letter 'T' for terrain
//    char 1:   Resolution
//    char 2:   Terrain type
//    char 3:   IJ indices compacted (I is X direction in super tile)
//-----------------------------------------------------------------------------
U_INT CTextureWard::KeyForTerrain(CTextureDef *txn,U_CHAR res)
{ return ('T' << 24) | (res << 16) | txn->Key; }
//-----------------------------------------------------------------------------
//  Make A Water Key
//-----------------------------------------------------------------------------
U_INT CTextureWard::KeyForWater(U_CHAR res)
{ return ('T' << 24) | (res << 16) | (0x0C00);}
//-----------------------------------------------------------------------------
//  Make A Runway Key
//-----------------------------------------------------------------------------
U_INT CTextureWard::KeyForRunway(U_CHAR grnd,U_CHAR seg)
{ return ('R' << 24) | (seg << 16) | RunwayKEY[grnd];}
//-----------------------------------------------------------------------------
//  Make a Taxiway Key
//-----------------------------------------------------------------------------
U_INT CTextureWard::KeyForTaxiway(U_CHAR res)
{ return ('P' << 24) | (res << 16) | 0;}
//-----------------------------------------------------------------------------
//  Assign a shared texture object
//  
//  After object is assigned, the memory image may be deleted if permitted
//-----------------------------------------------------------------------------
void CTextureWard::GetShdOBJ(CTextureDef *txn)
{ U_CHAR  res = txn->GetResolution();
  U_INT   key = KeyForTerrain(txn,res);

  pthread_mutex_lock (&txnMux);
  std::map<U_INT,CSharedTxnTex*>::iterator itx = txnMAP.find(key);
  CSharedTxnTex *shx = (itx == txnMAP.end())?(0):((*itx).second);
  //-----Assign the texture -------------------------------------
  pthread_mutex_unlock (&txnMux);
  if (0 == shx)             return;
  if (shx->AssignOBJ(txn))  return;
	U_INT obj = GetTerraOBJ(0,res,shx->dTEX[0]);
  shx->AssignDAY(obj);
  txn->dOBJ = obj;
  return;
}
//-----------------------------------------------------------------------------
//  Assign a Water texture object
//  
//-----------------------------------------------------------------------------
GLuint CTextureWard::GetWatOBJ(CTextureDef *txn)
{ U_CHAR  rs  = txn->GetResolution();
  GLuint obj  = Tank[rs].GetXOBJ(); 
  txn->dOBJ   = obj;
  return obj;
}
//-----------------------------------------------------------------------------
//  Load Default Taxiway texture as a shared one
//-----------------------------------------------------------------------------
void CTextureWard::LoadTaxiTexture(char *name,char tsp)
{ U_CHAR res = TC_HIGHTR;
  CArtParser img(res);
  //----PATH is ART --------------------------------
  strncpy(xds.name,name,TC_TEXTURE_NAME_NAM);
  _snprintf(xds.path,512,"ART/%s.PNG",name);
  xds.azp   = tsp;
  U_INT key = KeyForTaxiway(res);               
  CSharedTxnTex *shx = new CSharedTxnTex("TAXI",res);
  //--------Read the Day RAW and ACT texture file --
  GLubyte *rgb  = (usq)?(sqm->GetAnyTexture(xds)):(img.GetAnyTexture(xds));
  shx->Reso[0]  = res;
  //--------Assign a texture object ----------------
  U_INT obj     = GetRepeatOBJ(xds);
  shx->AssignDAY(obj);
  delete [] rgb;
  //-----Register this new texture -----------------
  pthread_mutex_lock (&txnMux);
  txnMAP[key] = shx;
  NbSHD++;
  pthread_mutex_unlock (&txnMux);
  return;
}
//-----------------------------------------------------------------------------
//  Load a Mask texture
//-----------------------------------------------------------------------------
void CTextureWard::LoadMaskTexture(int No,char *name,int dim)
{ char path[MAX_PATH];
  strcpy(xds.path,name);
  strcpy(xds.name,name);
  Blend[No].side = dim;
  _snprintf(path,(MAX_PATH-1),"SYSTEM/GLOBE/%s.RAW",name);
  //MEMORY_LEAK_MARKER ("LoadMaskTexture");
  Blend[No].msk = (usq)?(sqm->GetAnyTexture(xds)):LoadMSK(path,dim);
  //MEMORY_LEAK_MARKER ("LoadMaskTexture");
  return;
}
//-----------------------------------------------------------------------------
//  Load Runway texture as a shared one
//  gt is the ground type (i.e GROUND_ASPHALT)
//  fn is the texture file name
//  tsp is transparency option
//-----------------------------------------------------------------------------
void CTextureWard::LoadRwyTexture(U_INT key,char *fn, char tsp)
{ U_CHAR res = TC_HIGHTR;                         // Resolution
  U_CHAR bld = (tsp == 0);
  CArtParser img(res);
  //----PATH is ART --------------------------------
  xds.azp    = tsp;
  _snprintf(xds.path,512,"ART/%s.png",fn);
  strncpy(xds.name,fn,TC_TEXTURE_NAME_NAM);
  CSharedTxnTex *shx = new CSharedTxnTex(fn,res);
  //--------Read texture and assign texture object
  GLubyte *rgb  = 0;
  if (usq) rgb  = sqm->GetAnyTexture(xds);
  if (0 == rgb) rgb = img.GetAnyTexture(xds);
  shx->Reso[0]  = res;
  U_INT    obj  = GetRepeatOBJ(xds);
  shx->AssignDAY(obj);
  shx->SetBlend(bld);
  delete [] xds.mADR;
  xds.mADR = 0;
  //-----Register this new texture -----------------
  pthread_mutex_lock (&txnMux);
  txnMAP[key] = shx;
  NbSHD++;
  pthread_mutex_unlock (&txnMux);
  return;
}
//-----------------------------------------------------------------------------
//  Load A PNG Texture
//-----------------------------------------------------------------------------
bool CTextureWard::LoadImagePNG(char *fn, S_IMAGE &ref)
{ CArtParser img(0);
  img.SetOption(PNG_IGNOREGAMMA);
  img.DontAbort();
  img.LoadFFF(fn,0,FIF_PNG);
  ref.rgba  = (U_INT*)img.TransferRGB();
  ref.ht    = img.GetHeigth();
  ref.wd    = img.GetWidth();
  ref.dim   = img.GetDim();
  return (0 != ref.rgba);
}

//-----------------------------------------------------------------------------
//  Load A JPG Texture
//-----------------------------------------------------------------------------
bool CTextureWard::LoadImageJPG(char *fn, S_IMAGE &ref)
{ CArtParser img(0);
  img.SetOption(JPEG_ACCURATE);
  img.DontAbort();
  img.LoadFFF(fn,0,FIF_JPEG);
  ref.rgba  = (U_INT*)img.TransferRGB();
  ref.ht    = img.GetHeigth();
  ref.wd    = img.GetWidth();
  ref.dim   = img.GetDim();
  return (0 != ref.rgba);
}
//--------------------------------------------------------------
//  LOAD a  texture 
//---------------------------------------------------------------
void CTextureWard::LoadAnyTexture(char *pn,TEXT_DEFN &txd)
{	TEXT_INFO txf;                // Texture info;
  //--- Read the texture ----------------------
  CArtParser img(TC_HIGHTR);
  strncpy(txf.path,pn,TC_TEXTURE_NAME_DIM);
  img.GetAnyTexture(txf);
  txd.Copy(txf);
  //---- Adjust texture height ----------------
  int nf  = 1;
  txd.dm  = txd.wd * txd.ht;
  return;
}
//-----------------------------------------------------------------------------
//  Load Light Color texture into color table
//  Associate black pixel to a 0 Alpha channel
//-----------------------------------------------------------------------------
void CTextureWard::LoadLightTexture(U_CHAR No)
{ U_CHAR res = TC_LOWRES;
  CArtParser img(res);
  char *name  = LiteNAM[No];
  //----PATH is ART --------------------------------
  _snprintf(xds.path,512,"ART/%s.PNG",name);
  strncpy(xds.name,name,TC_TEXTURE_NAME_NAM);
  //----READ THE FILE ------------------------------
  GLubyte *rgb = (usq)?(sqm->GetAnyTexture(xds)):(img.GetAnyTexture(xds));
  int      dim = SideRES[res];          
  LiOBJ[No]    = GetLitOBJ(xds);
  if (rgb)  delete []  rgb;
  return;
}
//-----------------------------------------------------------------------------
//  Load an icon PNG file
//  Assign a texture object
//-----------------------------------------------------------------------------
GLuint CTextureWard::LoadIconPNG(char *name)
{ U_CHAR res = TC_LOWRES;
  CArtParser img(res);
  //----PATH is ART --------------------------------
  _snprintf(xds.path,512,"ART/%s.PNG",name);
  //----READ THE FILE ------------------------------
  GLubyte *rgb = img.GetAnyTexture(xds);
  GLuint   xob = GetLitOBJ(xds);
  delete []  rgb;
  return xob;
}
//-----------------------------------------------------------------------------
//  Return Texture object
//-----------------------------------------------------------------------------
GLuint CTextureWard::GetLiteTexture(U_CHAR No)   {return LiOBJ[No];}
//-----------------------------------------------------------------------------
//  Assign runway Texture to the tarmac
//-----------------------------------------------------------------------------
void CTextureWard::GetRwyTexture(CTarmac *tmac,U_CHAR gt)
{ U_INT key = RwyKEY[gt];          //RunwayKEY[gt] | ('R' << 8) | st;
  CSharedTxnTex *shx  = GetSharedTex(key);
  if (0 == shx)  gtfo("No standard runway texture");
  //----Allocate the shared texture -------------------------------
  U_CHAR  tsp = shx->GetBlend();
  tmac->SetOBJ(shx->dOBJ);         // Set texture object
  tmac->SetKEY(key);               // Set Key
  tmac->SetBLD(tsp);               // Set blending
  return;
}
//-----------------------------------------------------------------------------
//  Get Taxiway default texture object
//-----------------------------------------------------------------------------
GLuint CTextureWard::GetTaxiTexture()
{ U_INT key = KeyForTaxiway(TC_HIGHTR);         //TC_TAXI_TEXKEY | (TC_HIGHTR << TC_BYWORD) | 'TX';
  CSharedTxnTex *shx  = GetSharedTex(key);
  if (0 == shx) Abort("Taxiway Texture: ","No texture");
  return shx->dOBJ;
}
//-----------------------------------------------------------------------------
//  Get A 3D model Texture
//  Format may be either
//  1)RAW-ACT
//  2)TIF
//	Allocated a shared object for this texture name
//	Return the shared objet as a reference to this texture
//-----------------------------------------------------------------------------
void *CTextureWard::GetM3DPodTexture(char *fn,U_CHAR tsp)
{ _strupr(fn);
  void *ref = RefTo3DTexture(fn);
  if   (ref)  return ref;
  char *dot = strstr(fn,".");
  if (0 == dot) return 0;
  //---Add a new shared texture --------------------------
  CShared3DTex *shx = new CShared3DTex(fn,tsp);
  char         *idn = shx->GetIdent();
  TEXT_INFO    *inf = shx->GetInfo();
  if (strcmp(dot,".TIF") == 0)  Get3DTIF(inf);
  if (strcmp(dot,".RAW") == 0)  Get3DRAW(inf);
  if (0 == inf->mADR)  {delete shx; shx = 0;}
  //--Insert new shared object ---------------------------
  pthread_mutex_lock (&t3dMux);
  if (shx)  {t3dMAP[idn]  = shx; Nb3DT++;}
  pthread_mutex_unlock (&t3dMux);
  return shx;
}
//-----------------------------------------------------------------------------
//  Get A 3D model Texture
//  from SQL database
//-----------------------------------------------------------------------------
void *CTextureWard::GetM3DSqlTexture(char *fn,U_CHAR tsp)
{ void *ref = RefTo3DTexture(fn);
  if   (ref)  return ref;
  //---Add a new shared texture --------------------------
  CShared3DTex *shx = new CShared3DTex(fn,tsp);
  char         *idn = shx->GetIdent();
  TEXT_INFO    *inf = shx->GetInfo();
  globals->sql->GetM3DTexture(inf);
  //--Insert new shared object ---------------------------
  pthread_mutex_lock (&t3dMux);
  if (shx)  {t3dMAP[idn]  = shx; Nb3DT++;}
  pthread_mutex_unlock (&t3dMux);
  return shx;
}

//-----------------------------------------------------------------------------
//  Locate shared 3DW texture
//-----------------------------------------------------------------------------
void *CTextureWard::RefTo3DTexture(char *fn)
{ pthread_mutex_lock (&t3dMux);
  CShared3DTex *shx = 0;
  std::map<std::string,CShared3DTex*>::iterator itx = t3dMAP.find(fn);
  if (itx != t3dMAP.end())
  { shx = (*itx).second;
    shx->IncUser();
  }
  pthread_mutex_unlock (&t3dMux);
  return shx;
}
//-----------------------------------------------------------------------------
//  Return a TIF Texture for 3D object
//-----------------------------------------------------------------------------
void CTextureWard::Get3DTIF(TEXT_INFO *txd)
{ char tsp   = (txd->azp == 0)?(1):(0);
	CTIFFparser img(tsp);
  //----PATH is ART --------------------------------
  img.Decode(txd->path);
  txd->bpp  = 4;
  txd->mADR = img.TransferRGB();
  txd->wd   = img.GetWidth();
  txd->ht   = img.GetHeight();
  return;
}
//-----------------------------------------------------------------------------
//  Return a RAW-ACT Texture for 3D object
//-----------------------------------------------------------------------------
void CTextureWard::Get3DRAW(TEXT_INFO *txd)
{ CArtParser img(0);
  //----PATH is ART --------------------------------
  char *dot = strstr(txd->path,".") + 1;
  *dot = 0;
  img.DontAbort();
  txd->mADR = img.GetModTexture(*txd);
  txd->bpp  = 4;
//  txd->wd   = img.GetSide();
//  txd->ht   = img.GetSide();
  strcpy(dot,"RAW");                             // Reset name
  return;
}
//-----------------------------------------------------------------------------
//  Assign a 3D texture object
//-----------------------------------------------------------------------------
GLuint CTextureWard::Get3DObject(void *tref)
{ if (0 == tref)          return 0;
  CShared3DTex *shx = (CShared3DTex*)tref;
  TEXT_INFO    *inf = shx->GetInfo();
  if (0 != inf->xOBJ)     return inf->xOBJ;
  if (0 == inf->mADR)     return 0;
  return GetM3dOBJ(inf);
}
//-----------------------------------------------------------------------------
//  Free a 3D texture
//-----------------------------------------------------------------------------
void CTextureWard::Free3DTexture(void *sht)
{ CShared3DTex *shx = (CShared3DTex *)sht;
  if (0 == shx)   return;
  //---Decrement count and release if 0 user ----
  pthread_mutex_lock (&t3dMux);
  if (!shx->DecUser())
    { t3dMAP.erase(shx->GetIdent());
      delete shx;
      Nb3DT--;
    }
  pthread_mutex_unlock (&t3dMux);
  return;
}
//-----------------------------------------------------------------------------
//  Assign a texture object with mipmap level depending on resolution
//-----------------------------------------------------------------------------
GLuint CTextureWard::GetTexOBJ(GLuint obj,int x,int y,GLubyte *tex,U_INT type)
{ if (0 == obj) glGenTextures(1,&obj);
  glBindTexture(GL_TEXTURE_2D,obj);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,x,y,0,type,GL_UNSIGNED_BYTE,tex);
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  return obj;
}
//-----------------------------------------------------------------------------
//  Assign a texture object using texture info n
//-----------------------------------------------------------------------------
GLuint CTextureWard::GetTexOBJ(TEXT_INFO &inf, U_INT mip,U_INT type)
{ GLuint obj = inf.xOBJ;
  if (0 == obj) glGenTextures(1,&obj);
  glBindTexture(GL_TEXTURE_2D,obj);
  glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_TRUE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,mip);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,inf.wd,inf.ht,0,type,GL_UNSIGNED_BYTE,inf.mADR);
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  inf.xOBJ = obj;
  if (inf.mADR) delete [] inf.mADR;
  inf.mADR = 0;
  return obj;
}
//-----------------------------------------------------------------------------
//  Assign a texture object using texture info n
//-----------------------------------------------------------------------------
GLuint CTextureWard::GetGaugeOBJ(TEXT_DEFN &txd)
{ GLuint obj = txd.xo;
  if (0 == obj) glGenTextures(1,&obj);
  glBindTexture(GL_TEXTURE_2D,obj);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,txd.wd,txd.ht,0,GL_RGBA,GL_UNSIGNED_BYTE,txd.rgba);
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  txd.xo = obj;
  if (txd.rgba) delete [] txd.rgba;
  txd.rgba = 0;
  return obj;
}
//-----------------------------------------------------------------------------
//  Assign a texture object for an alpha mask
//  DONT delete the mask yet.
//-----------------------------------------------------------------------------
GLuint CTextureWard::GetMskOBJ(TEXT_INFO &inf,U_INT mip)
{ GLuint obj = inf.xOBJ;
  if (0 == obj) glGenTextures(1,&obj);
  glBindTexture(GL_TEXTURE_2D,obj);
  glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_TRUE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,mip);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
  glTexImage2D(GL_TEXTURE_2D,0,GL_ALPHA,inf.wd,inf.ht,0,GL_ALPHA,GL_UNSIGNED_BYTE,inf.mADR);

  inf.xOBJ = obj;
  return obj;
}
//-----------------------------------------------------------------------------
//  Assign a 3D texture object
//-----------------------------------------------------------------------------
GLuint CTextureWard::GetM3dOBJ(TEXT_INFO *inf)
{ GLuint obj  = 0;
  if (0 == inf->xOBJ) glGenTextures(1,&obj);
  glBindTexture(GL_TEXTURE_2D,obj);
  glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_TRUE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,6);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA,inf->wd,inf->ht,0,GL_RGBA,GL_UNSIGNED_BYTE,inf->mADR);
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  inf->xOBJ = obj;
  delete [] inf->mADR;
  inf->mADR = 0;
  return obj;
}

//-----------------------------------------------------------------------------
//  Assign a texture object with mipmap level depending on resolution
//  With repaet border
//-----------------------------------------------------------------------------
GLuint CTextureWard::GetRepeatOBJ(GLuint obj,U_CHAR res,GLubyte *tex)
{ U_INT  dim  = SideRES[res];
  if (0 == obj) glGenTextures(1,&obj);
  glBindTexture(GL_TEXTURE_2D,obj);
  glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_TRUE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,3);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,dim,dim,0,GL_RGBA,GL_UNSIGNED_BYTE,tex);
  glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_FALSE);
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  //----Check for error -----------------------------------------
	/*
  {GLenum e = glGetError ();
   if (e != GL_NO_ERROR) 
    WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  }
	*/
  return obj;
}
//-----------------------------------------------------------------------------
//  Assign a texture object with mipmap level depending on resolution
//  With repaet border
//-----------------------------------------------------------------------------
GLuint CTextureWard::GetRepeatOBJ(TEXT_INFO &xds)
{ U_INT obj = 0;
  glGenTextures(1,&obj);
  glBindTexture(GL_TEXTURE_2D,obj);
  glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_TRUE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,3);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);


  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,xds.wd,xds.ht,0,GL_RGBA,GL_UNSIGNED_BYTE,xds.mADR);
  glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_FALSE);
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  //----Check for error -----------------------------------------
	/*
  {GLenum e = glGetError ();
   if (e != GL_NO_ERROR) 
    WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  }
	*/
  return obj;
}
//-----------------------------------------------------------------------------
//  Assign a texture object with mipmap level depending on resolution
//  With repaet border
//-----------------------------------------------------------------------------
GLuint CTextureWard::GetTerraOBJ(GLuint obj,U_CHAR res,GLubyte *tex)
{ U_INT  dim  = SideRES[res];
  if (0 == obj) glGenTextures(1,&obj);
  glBindTexture(GL_TEXTURE_2D,obj);
  glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_TRUE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,3);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,dim,dim,0,GL_RGBA,GL_UNSIGNED_BYTE,tex);
  glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_FALSE);

  //----Check for error -----------------------------------------
	/*
  {GLenum e = glGetError ();
   if (e != GL_NO_ERROR) 
    WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  }
	*/
  return obj;
}

//-----------------------------------------------------------------------------
//  Assign a light texture object
//-----------------------------------------------------------------------------
GLuint CTextureWard::GetLitOBJ(GLuint obj,U_INT dim,GLubyte *tex)
{ if (0 == obj) glGenTextures(1,&obj);
  glBindTexture(GL_TEXTURE_2D,obj);
  glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_TRUE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,1);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
  glTexImage2D   (GL_TEXTURE_2D,0,GL_INTENSITY,dim,dim,0,GL_RGBA,GL_UNSIGNED_BYTE,tex);
  glTexEnvf (GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_BLEND);
  glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_FALSE);
  return obj;
}
//-----------------------------------------------------------------------------
//  Assign a light texture object
//-----------------------------------------------------------------------------
GLuint CTextureWard::GetLitOBJ(TEXT_INFO &xds)
{ U_INT obj = 0;
  if (0 == xds.mADR)  return 0;
  glGenTextures(1,&obj);
  glBindTexture(GL_TEXTURE_2D,obj);
  glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_TRUE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,1);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
  glTexImage2D   (GL_TEXTURE_2D,0,GL_INTENSITY,xds.wd,xds.ht,0,GL_RGBA,GL_UNSIGNED_BYTE,xds.mADR);
  glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_FALSE);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  return obj;
}
//-----------------------------------------------------------------------------
//  Assign a texture object to each detail tile in the Super Tile
//  For a shared texture, allocate a shared object
//  NOTE: After texture object is allocated memory RGBA may be deleted
//-----------------------------------------------------------------------------
void CTextureWard::GetSupOBJ(CSuperTile *sp)
{ CTextureDef *txn = 0;
  U_CHAR res = 0;
  U_INT  obj = 0;
  for (int k = 0; k!= TC_TEXSUPERNBR; k++)     
    { txn = &sp->Tex[k];
      res = txn->GetResolution();
      CmQUAD *qad = txn->quad;
			qad->RefreshVTAB(sp,res);            // Allocate Vertex table
      GLubyte *rgb = txn->dTEX[0];
      //--------assign day texture object ------------------
      
      switch (txn->TypTX) {

          case TC_TEXSHARD:
            GetShdOBJ(txn);
            break;
          case TC_TEXWATER:
            GetWatOBJ(txn);
            break;
          case TC_TEXGENER:
            if (0 == rgb) break;
            obj = GetTerraOBJ(txn->dOBJ,res,rgb);
            txn->AssignDAY(obj);
            break;
          case TC_TEXRAWTN:
            if (0 == rgb) break;
            obj = GetTerraOBJ(txn->dOBJ,res,rgb);
            txn->AssignDAY(obj);
            break;
          case TC_TEXCOAST:
            if (0 == rgb) break;
            obj = GetTerraOBJ(txn->dOBJ,res,rgb);
            txn->AssignDAY(obj);
            break;
          case TC_TEXRAWEP:
            if (0 == rgb) break;
            obj = GetTerraOBJ(txn->dOBJ,res,rgb);
            txn->AssignDAY(obj);
            break;
      }
      //--------Assign night texture if any ----------------
      rgb = txn->nTEX[0];
      if (0 == rgb)  continue;
			obj = GetTerraOBJ(txn->nOBJ,res,rgb);
      txn->AssignNIT(obj);
    }
  //----All component are ready ----------------------------
  sp->WantRDY();
  return;
}
//-----------------------------------------------------------------------------
//  Free all Textures from this SuperTile
//  Free the vertex table
//  NOTE: This routine is runing on the main THREAD
//        Dont use any field from the TextureWard object as it would conflict
///       with the texture load thread
//-----------------------------------------------------------------------------
int CTextureWard::FreeAllTextures(CSuperTile *sp)
{ CTextureDef *txn = 0;
  for (int Nd = 0; Nd != TC_TEXSUPERNBR; Nd++)
        { txn = &sp->Tex[Nd];
          if (txn->IsShare())     { FreeShared(txn);     continue;}
          if (txn->IsWater())     { FreeWater (txn);     continue;}
          if (txn->IsCoast())     { txn->FreeALL();      continue;}
          if (txn->IsSlice())     { txn->FreeALL();      continue;}
          if (txn->IsAnEPD())     { txn->FreeALL();      continue;}
          if (txn->IsGener())     { txn->FreeALL();      continue;}
        }
  return 0;
}
//-----------------------------------------------------------------------------
//  SWAP mode:
//  The current day texture and object are freed
//  The current night texture and object are freed
//  The current vertex table is freed
//  then the texture day and night are swapped and a new vertex table
//  is allocated for the new resolution
//  The next step will be 'object texture allocation'
//  NOTE: Dont use any variables from TextureWard as it would conflict
//        with the Texture load THREAD
///----------------------------------------------------------------------------
int CTextureWard::SwapTextures(CSuperTile *sp)
{ CmQUAD      *qad  = 0;
  U_CHAR       res = sp->aRes;                            // Alternate resolution
  CTextureDef *txn = 0;
  //------For each Detail swap the texture----------------------
  for (int Nd = 0; Nd != TC_TEXSUPERNBR; Nd++)
        { txn = &sp->Tex[Nd];
          qad = txn->quad;
          switch (txn->TypTX) {
            //----Water texture -------------------------------
            case TC_TEXWATER:
                FreeWaterSlot(txn);
                txn->PopTextures(0);
                continue;
            //----Shared texture -------------------------------
            case TC_TEXSHARD:
                FreeSharedSlot(txn);
                txn->PopTextures(0);
                continue;
            //----------------------------------------------------
            // Thoses textures will reuse the texture object
            //-----------------------------------------------------
            case TC_TEXCOAST:
            case TC_TEXGENER:
                txn->PopTextures(1);
                continue;
            //----------------------------------------------------
            //  Those textures will reuse the texture object
            //----------------------------------------------------
            case TC_TEXRAWTN:
                txn->PopTextures(1);
                continue;
            //----------------------------------------------------
            // FLY I EPD textures don't need swapping
            //----------------------------------------------------
          }
        }
  //---Change state ----------------------------------
  sp->WantOBJ();                                           // Next step
  return 1;
}
//-----------------------------------------------------------------------------
//  Get a reduced texture from the mipmap set
//-----------------------------------------------------------------------------
void CTextureWard::GetMediumTexture(CTextureDef *txn)
{ 
  return;
}
//-----------------------------------------------------------------------------
//  Free a shared texture
//-----------------------------------------------------------------------------
void CTextureWard::FreeSharedKey(U_INT key)
{ pthread_mutex_lock (&txnMux);
  std::map<U_INT,CSharedTxnTex*>::iterator itx = txnMAP.find(key);
  if (itx == txnMAP.end())  Abort("FreeShared","No Texture");
  CSharedTxnTex *shx = (*itx).second;
  shx->Use--;
  //------------------------------------------------------------- -----
  if (0 == shx->Use)
    { txnMAP.erase(key);
      delete shx;
      NbSHD--;
    }
  pthread_mutex_unlock (&txnMux);
return;
}
//-----------------------------------------------------------------------------
//  Free Shared texture for this descriptor
//  Day:    OBJ and TEXTURE
//  Night   OBJ and TEXTURE
//  ALternate textures are kept
//  NOTE: This routine runs on MAIN THREAD
//-----------------------------------------------------------------------------
int CTextureWard::FreeSharedSlot(CTextureDef *txn)
{ U_INT key = KeyForTerrain(txn,txn->GetResolution());             
  FreeSharedKey(key);
  //---------Clean descriptor --------------------------------
  txn->FreeDAY();
  txn->SetDOBJ(0);
  txn->FreeNTX();
  return 0;
}
//-----------------------------------------------------------------------------
//  Free Water texture for this descriptor
//  Day:    OBJ and TEXTURE
//  ALternate textures are kept
//  NOTE: This routine runs on MAIN THREAD
//-----------------------------------------------------------------------------
int CTextureWard::FreeWaterSlot(CTextureDef *txn)
{ txn->FreeDAY();
  txn->SetDOBJ(0);
  return 0;
}
//-----------------------------------------------------------------------------
//  Free Water
//-----------------------------------------------------------------------------
int CTextureWard::FreeWater(CTextureDef *txn)
{ txn->FreeDAY();
  txn->SetDOBJ(0);
  txn->FreeALT();
  return 0;
}
//-----------------------------------------------------------------------------
//  Free a Shared texture from memory
//  NOTE: This routine runs on MAIN THREAD
//-----------------------------------------------------------------------------
int CTextureWard::FreeShared(CTextureDef *txn)
{ FreeSharedSlot(txn);
  //------Free alternate Day texture ------------------------------------
  U_INT key  = KeyForTerrain(txn,txn->aRes);      //(txn->aRes << TC_BYWORD) | txn->Key;
  if (txn->HasADTX())  FreeSharedKey(key);
  //------Free the possible alternate  textures --------------------
  txn->FreeALT();
  return 0;
}
//=============================================================================
// Draw a continuous line segment starting at (and including) (x1, y1) and
//   ending at (and including) (x2, y2).  This is an implementation of
//   Bresenham's algorithm.
// Drawing values are
//    1 for a line going in the up direction
//    2 for a line going in the down direction
//=============================================================================
void  CTextureWard::DrawTLine(int x1, int y1, int x2, int y2)
{ int di;
  int p1,p2;
  U_CHAR  *buf = Canvas + (Dim * y1) + x1;
  // First set start point and calculate delta in x and y endpoints
  int dx = x2 - x1;
  int dy = y2 - y1;
  int ybuf  = 0;
  int ady   = 0;
  // Check for special cases
  if ((dx == 0) && (dy == 0))                         return; 
  if (dx == 0)                  {DrawVLine(x1,y1,y2); return; }
  if (dy == 0)                  {DrawHLine(x1,x2,y1); return; } 
   
    // First endpoint must be the leftmost
   if (x1 > x2)  {DrawTLine (x2, y2, x1, y1);                        return; }
   // Adjust y-increment for negatively sloped lines
   if (dy < 0)  {ady = -1; ybuf = -Dim; dy = -dy;} 
   else         {ady = +1; ybuf = +Dim;}
   // Determine whether to step in x-direction or y-direction
   if (abs(dx) >= abs(dy) ) {
      // Step in x direction
      // Calculate constants for Bresenham's algorithm
      p1  = (dy << 1);                          // Inc1 is 2*dy
      p2  = p1 - (dx << 1);                     // Inc2 is 2*dy - 2*dx
      di  = p1 - dx;                            // d1   is 2*dy - dx
      // Step through each x-coordinate, drawing the appropriate pixels
      //  NOTE that the extremity should be drawed
      while (x1 <= x2) 
        {  *buf |= Color;
            x1  += 1;
            buf += 1;
            if (di < 0)  {di += p1;} 
            else         {di += p2; buf += ybuf;}
        }
      return;
  } 
  // Step in y direction
   else {
      // Calculate constants for Bresenham's algorithm
      p1    = (dx << 1);                      // Inc1 is 2*dx
      p2    = p1 - (dy << 1);                 // Inc2 is 2*dx - 2*dy
      di    = p1 - dy;                        // d1 is   2*dx -dy
      // Step through each y-coordinate, drawing the appropriate pixels
      do { *buf |= Color;
            y1  += ady;
            buf += ybuf;
            if (di <= 0) {di += p1;} 
            else         {di += p2; buf += 1;}
          }
      while (y1 != y2);
      *buf |= Color;
   }
   return;
}
//-----------------------------------------------------------------------------
//  Draw Vertical line to mask.
//-----------------------------------------------------------------------------
void CTextureWard::DrawVLine(int x0,int y1,int y2)
{ if (y1 > y2)  {DrawVLine(x0,y2,y1);   return;}
  U_CHAR *buf  = Canvas + (y1 * Dim) + x0;
  while (y1 <= y2) {*buf |= Color; buf += Dim; y1++;} 
  return;
  }
//-----------------------------------------------------------------------------
//  Draw Horizontal line
//-----------------------------------------------------------------------------
void CTextureWard::DrawHLine(int x1,int x2, int y0)
{ if (x1 > x2)  {DrawHLine(x2,x1,y0); return;}
  U_CHAR *buf   = Canvas + (y0 * Dim) + x1;
  while (x1++ <= x2) 
  {*buf++ |= Color; }
  return;
}
//-----------------------------------------------------------------------------
//  Increment Day texture Object count
//-----------------------------------------------------------------------------
void CTextureWard::ModDTX(char k)
{ pthread_mutex_lock (&ctrMux);
  NbTXM.dxcnt += k;
  pthread_mutex_unlock (&ctrMux);
}
//-----------------------------------------------------------------------------
//  Increment Night texture Object count
//-----------------------------------------------------------------------------
void CTextureWard::ModNTX(char k)
{ pthread_mutex_lock (&ctrMux);
  NbTXM.nxcnt += k;
  pthread_mutex_unlock (&ctrMux);
}
//-----------------------------------------------------------------------------
//  Increment Day texture count
//-----------------------------------------------------------------------------
void CTextureWard::IncDAY(char k)
{ pthread_mutex_lock (&ctrMux);
  NbTXM.Dcnt[k]++;
  pthread_mutex_unlock (&ctrMux);
}
//-----------------------------------------------------------------------------
//  Decrement Day texture count
//-----------------------------------------------------------------------------
void CTextureWard::DecDAY(char k)
{ pthread_mutex_lock (&ctrMux);
  NbTXM.Dcnt[k]--;
  pthread_mutex_unlock (&ctrMux);
}
//-----------------------------------------------------------------------------
//  Increment Night texture count
//-----------------------------------------------------------------------------
void CTextureWard::IncNIT(char k)
{ pthread_mutex_lock (&ctrMux);
  NbTXM.Ncnt[k]++;
  pthread_mutex_unlock (&ctrMux);
}
//-----------------------------------------------------------------------------
//  Decrement Night texture count
//-----------------------------------------------------------------------------
void CTextureWard::DecNIT(char k)
{ pthread_mutex_lock (&ctrMux);
  NbTXM.Ncnt[k]--;
  pthread_mutex_unlock (&ctrMux);
}
//-----------------------------------------------------------------------------
//  Promote Day texture count
//-----------------------------------------------------------------------------
void CTextureWard::PopDAY()
{ pthread_mutex_lock (&ctrMux);
  NbTXM.Dcnt[0]++;
  NbTXM.Dcnt[1]--;
  pthread_mutex_unlock (&ctrMux);
}
//-----------------------------------------------------------------------------
//  Promote Nit texture count
//-----------------------------------------------------------------------------
void CTextureWard::PopNIT()
{ pthread_mutex_lock (&ctrMux);
  NbTXM.Ncnt[0]++;
  NbTXM.Ncnt[1]--;
  pthread_mutex_unlock (&ctrMux);
}

//-----------------------------------------------------------------------------
//  GetStatistical data
//-----------------------------------------------------------------------------
void CTextureWard::GetStats(CFuiCanva *cnv)
{ char txt[128];
  cnv->AddText(1,"Shared Terrain:");
  sprintf_s(txt,128,"% 8d",NbSHD);
  cnv->AddText(STATS_NUM,txt,1);
  //------------------------------------
  cnv->AddText(1,"Shared 3D texture:");
  sprintf_s(txt,128,"% 8d",Nb3DT);
  cnv->AddText(STATS_NUM,txt,1);
  //------------------------------------
  cnv->AddText(1,"Memory Day TEX:");
  sprintf_s(txt,128,"% 8d",NbTXM.Dcnt[0]);
  cnv->AddText(STATS_NUM,txt,1);
  //------------------------------------
  cnv->AddText(1,"Memory Day ALT:");
  sprintf_s(txt,128,"% 8d",NbTXM.Dcnt[1]);
  cnv->AddText(STATS_NUM,txt,1);
  //------------------------------------
  cnv->AddText(1,"Memory Nit TEX:");
  sprintf_s(txt,128,"% 8d",NbTXM.Ncnt[0]);
  cnv->AddText(STATS_NUM,txt,1);
  //------------------------------------
  cnv->AddText(1,"Memory Nit ALT:");
  sprintf_s(txt,128,"% 8d",NbTXM.Ncnt[1]);
  cnv->AddText(STATS_NUM,txt,1);
  //------------------------------------
  cnv->AddText(1,"Day Texture OBJ:");
  sprintf_s(txt,128,"% 8d",NbTXM.dxcnt);
  cnv->AddText(STATS_NUM,txt,1);
  //------------------------------------
  cnv->AddText(1,"Nite Texture OBJ:");
  sprintf_s(txt,128,"% 8d",NbTXM.nxcnt);
  cnv->AddText(STATS_NUM,txt,1);
}
//----------------------------------------------------------------------------
//  Write the texture:  This is used for test only
//----------------------------------------------------------------------------
void CTextureWard::WriteTexture(int wd,int ht,U_INT obj)
{ int     nbp = wd * ht;
  int     dim = nbp* 4;
  U_CHAR *buf = new U_CHAR[dim];
  glBindTexture(GL_TEXTURE_2D,obj);
  glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,buf);
    //----------------------------------------------------------------
  {GLenum e = glGetError ();
   if (e != GL_NO_ERROR) 
    WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  }
  U_INT     nzr = 0;
  U_INT  *pix = (U_INT*)buf;
  //---Compute a non zero image indicator --------------------
  for (int k=0; k<nbp; k++)     nzr |= (*pix++ != 0x00FFFFFF);
  //----------------------------------------------------------
  char fn[PATH_MAX];
  _snprintf(fn,(PATH_MAX-1),"TMP\\TEXTURE%04d.BMP",nPic++);
  CArtParser img(0);
  if (nzr) img.WriteBitmap(FIF_BMP,fn,wd,ht,buf);
  delete [] buf;
  return;
}
//----------------------------------------------------------------------------
//  Write the texture:  This is used for test only
//----------------------------------------------------------------------------
void CTextureWard::WriteScreen()
{ int     wd  = globals->mScreen.Width;
  int     ht  = globals->mScreen.Height;
  int     nbp = wd * ht;
  int     dim = nbp* 4;
  U_CHAR *buf = new U_CHAR[dim];
  glReadPixels(0,0,wd,ht,GL_RGBA,GL_UNSIGNED_BYTE,buf);
    //----------------------------------------------------------------
  {GLenum e = glGetError ();
   if (e != GL_NO_ERROR) 
    WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  }
  U_INT     nzr = 0;
  U_INT  *pix = (U_INT*)buf;
  //---Compute a non zero image indicator --------------------
  for (int k=0; k<nbp; k++)     nzr |= (*pix++ != 0x00FFFFFF);
  //----------------------------------------------------------
  char fn[PATH_MAX];
  _snprintf(fn,(PATH_MAX-1),"TMP\\SCREEN%04d.BMP",nPic++);
  CArtParser img(0);
  if (nzr) img.WriteBitmap(FIF_BMP,fn,wd,ht,buf);
  delete [] buf;
  return;
}

//================================================================================
//  Class CWater3D to produce animated textures
//  sob => Handle to background sea texture
//  w3d => Handle to texture 3D holding water animation
//================================================================================
CWater3D::CWater3D(int sz)
{ dim = sz;
  CArtParser bim(TC_HIGHTR);
  cds   = cdt = 0;
  vit   = 0;
  once  = 1;
  //---Build the 3D animation --------------------------------
  //if (0 == globals->noAWT)  LoadWater3D();
  //---Create a camera object --------------------------------
  CVector  ofs(0,0,0);
  RGBA_COLOR blk = {0,0,0,0};
  cam      = new CCameraObject();
  cam->FrontOffsetFor(256);
  cam->SetOffset(ofs);
  cam->SetFBO(dim,dim);
  cam->SetColor(blk);
  //----------------------------------------------------------
  globals->txw->GetTextParam(TC_HIGHTR,&org,&exp);
  exp = float(1) / (exp);
  //---Init the quad coordinates -----------------------------
  InitQuad();
}
//--------------------------------------------------------------------
//  Init Quad coordinates
//--------------------------------------------------------------------
void CWater3D::InitQuad()
{ float sid   = 128;
  //---SW corner -------------
  qad[0].VT_X = -sid;
  qad[0].VT_Y =    0;
  qad[0].VT_Z = -sid;
  //---SE corner -------------
  qad[1].VT_X = +sid;
  qad[1].VT_Y =    0;
  qad[1].VT_Z = -sid;
  //---NE corner -------------
  qad[2].VT_X = +sid;
  qad[2].VT_Y =    0;
  qad[2].VT_Z = +sid;
  //---NW corner ------------
  qad[3].VT_X = -sid;
  qad[3].VT_Y =    0;
  qad[3].VT_Z = +sid;
  return;
}
//--------------------------------------------------------------------
//  Delete all resources
//--------------------------------------------------------------------
CWater3D::~CWater3D()
{ delete    cam;
  glDeleteTextures(1,&sob);
  glDeleteTextures(1,&w3d);
} 
//--------------------------------------------------------------------
//  Build a sea texture into the FBO
//  The texture represent the animated part extracted from the 3D texture
//  The third parameter u gives the texture number to extract
//  This function is called once per frame as all water tiles share this
//  texture.
//--------------------------------------------------------------------
/*
void CWater3D::BuildAnimation()
{ cam->DebDrawFBOinOrthoMode(dim,dim);
  glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
  //--- Set direction - ------------------------------------
  glFrontFace(GL_CW);
  //--------------------------------------------------------
  COORD_UNIT   *vtx   = &qad[0].VT_X;
  COORD_UNIT    col[] = {1,1,1,1};
  //--------------------------------------------------------
  glMatrixMode(GL_TEXTURE);
  glPushMatrix();
  glScalef(exp,exp,1);
  //--------------------------------------------------------
  glBindTexture(GL_TEXTURE_3D, w3d);
  glEnable(GL_TEXTURE_3D);
  glTexCoordPointer(3,GL_FLOAT,0,s3d);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glVertexPointer(3,GL_FLOAT,sizeof(TC_VTAB),vtx);
  glEnableClientState(GL_VERTEX_ARRAY);
  //--------------------------------------------------------
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  //---------------------------------------------------------
  glDrawArrays(GL_QUADS,0,4);
  //---------------------------------------------------------
  glDisable(GL_TEXTURE_3D);
  //----Inhibit pointer usage -------------------------------
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopClientAttrib();
  cam->EndDrawFBO();
  return;
}
*/
//--------------------------------------------------------------------
//  Load Water textures
//  The 16 images are loaded and are used to make the 3D texture
//--------------------------------------------------------------------
void CWater3D::LoadWater3D()
{ int         sid = 256;
  int         dim = sid * sid;
  int         htr =  16;
  int         tot = htr * dim;
  U_CHAR     *buf = new U_CHAR[tot];
  CArtParser  img(TC_HIGHTR);
  U_CHAR     *tex = 0;
  int         nf  = 0;
  TEXT_INFO   xds;
  xds.azp     = 0xFF;
  w3d         = Get3DtexOBJ();
  glBindTexture(GL_TEXTURE_3D,w3d);
  for (int k=0; k<16; k++)
  { nf    = k + 1;
    _snprintf(xds.path,512,"ART/stw%03d.TIF",nf);
    tex = img.GetAnyTexture(xds);
    tex = PickAlphaChanel(tex, sid);
    Append(buf,tex,dim,k);
  }
  glTexImage3D(GL_TEXTURE_3D,0,GL_LUMINANCE8,sid,sid,htr-1,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,buf);
  glTexParameteri(GL_TEXTURE_3D,GL_GENERATE_MIPMAP,GL_FALSE);
  glDisable(GL_TEXTURE_3D);
  delete [] buf;
  return;
}
//--------------------------------------------------------------------
//  Append a plane image to buffer to aggregate the 3D texture
//--------------------------------------------------------------------
void CWater3D::Append(U_CHAR *buf,U_CHAR *tex,int dim,int k)
{ U_CHAR *src = tex;
  U_CHAR *dst = buf + (k * dim);
  for (int k=0; k<dim; k++) *dst++ = *src++;
  delete [] tex;
  return;
}
//--------------------------------------------------------------------
//  Get a 8 bit grey texture
//  Pick only the blue chanel as they are all the same
//--------------------------------------------------------------------
U_CHAR *CWater3D::PickAlphaChanel(U_CHAR *rgba, int side)
{ int   dim     = side * side;
  U_CHAR *buf   = new U_CHAR[dim];
  U_INT  *src   = (U_INT*)rgba;
  U_CHAR *dst   = buf;
  for (int k=0; k<dim; k++)
  { U_INT pix = *src++;
   *dst++    = (pix & 0x000000FF);
  }
  delete [] rgba;
  return buf;
}

//-----------------------------------------------------------------------------
//  Assign a texture object with mipmap level depending on resolution
//-----------------------------------------------------------------------------
GLuint CWater3D::Get3DtexOBJ()
{ U_INT obj = 0;
  glGenTextures(1,&obj);
  glEnable(GL_TEXTURE_3D);
  glBindTexture(GL_TEXTURE_3D,obj);
  glTexParameteri(GL_TEXTURE_3D,GL_GENERATE_MIPMAP,GL_TRUE);
  glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAX_LEVEL,3);
  glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
  //----Check for error -----------------------------------------
	/*
  {GLenum e = glGetError ();
   if (e != GL_NO_ERROR) 
    WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  }
	*/
  return obj;
}
//---------------------------------------------------------
//  Move texture by wind direction
//  Origin is defined by (cds,cdt) in [0,1]
//  The end point (S or T) defines the texture scale on the tile
//---------------------------------------------------------
U_INT CWater3D::MoveWater()
{ if (globals->noAWT)  return 0;
//---TODO move according to wind direction -----------
  cds += 0.00005f;
  if (cds > 1) cds = 0;
  cdt += 0.00006f;
  if (cdt > 1) cdt = 0;
  //--------------------------------------------------
  float ds    = cds;
  float es    = ds + 2.2;
  float dt    = cdt;
  float et    = dt + 2.2;
  //---Init quad with new texture coordinates --------
  s3d[0].VT_X = ds;
  s3d[0].VT_Y = dt;
  s3d[0].VT_Z = vit;
  s3d[1].VT_X = es;
  s3d[1].VT_Y = dt;
  s3d[1].VT_Z = vit;
  s3d[2].VT_X = es;
  s3d[2].VT_Y = et;
  s3d[2].VT_Z = vit;
  s3d[3].VT_X = ds;
  s3d[3].VT_Y = et;
  s3d[3].VT_Z = vit;
  //-------------------------------------------------
  vit += 0.005f;
  if (vit > 1)  vit = 0;
//  BuildAnimation();
  return cam->TextureObject();
}


//===============END OF THIS FILE =============================================
