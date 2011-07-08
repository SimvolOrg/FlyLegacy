/*
 * Bitmaps.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
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
 */

/*! \file Bitmaps.cpp
 *  \brief Implements drawing surface and bitmap related functions and classes
 */

#include "../Include/Globals.h"
#include "../Include/FlyLegacy.h"
#include "../Include/Utility.h"
#include "../Include/FileParser.h"
#include <vector>
using namespace std;


/*
 * Drawing Primitives
 *
 */

//
// Create a new drawing surface.  In Fly! II, drawing surfaces were RGB
//   arrays where one specific pixel colour value represented transparency,
//   usually <0, 0, 0>.  The xSpan field ensured that the first pixel
//   of each row was longword aligned, by padding each row of RGB values
//   to the next longword boundary.
//
// Legacy surfaces are RGBA arrays, so each pixel is assigned 
//   its transparency level.  This provides more flexibility for
//   implementing alpha-blended surfaces.  The xSpan field is not
//   really needed, but is kept for backwards compatibility with
//   Fly! II DLLs.
//
// Surface image data is oriented starting at the lower-left corner,
//   with pixels increasing towards the right, then row-wise towards
//   the top.
//
//  NOTE BY JS:  Add the invert field to accelerate dot computation used everywhere
//               Add nbpix to accelerate erase used also everywhere
SSurface* CreateSurface(int w, int h)
{ if ((w > 0) && (h > 0)) {
    //MEMORY_LEAK_MARKER ("rc_surf")
    SSurface *rc = new SSurface;
    //MEMORY_LEAK_MARKER ("rc_surf")

    rc->xScreen = 0;
    rc->yScreen = 0;
    rc->xSize = w;
    rc->ySize = h;
    rc->drawBuffer = new unsigned int[w * h];
    rc->invert    = (h - 1) * w;
    rc->nPixel    = (w * h);
    return rc;
  }
  if ((w == 0) && (h == 0))  return 0;
//  WARNINGLOG ("CreateSurface : Invalid dimensions %d x %d", width, height);
  gtfo ("CreateSurface : Invalid dimensions %d x %d", w, h);
  return 0;
}
//---------------------------------------------------------------------------
//  Free surface
//---------------------------------------------------------------------------
SSurface *FreeSurface(SSurface *sf)
{ if (0 == sf) return 0;
	SAFE_DELETE_ARRAY(sf->drawBuffer);
	delete sf;
  return 0;
}
//---------------------------------------------------------------------------
//  Erase to a given color value
//---------------------------------------------------------------------------
void  EraseSurfaceRGB(SSurface *surface, U_INT rgb)
{
  EraseSurfaceRGBA (surface, rgb | (0xff << 24));
}
//-------------------------------------------------------------------------
//  Erase the number of pixels
//-------------------------------------------------------------------------
void  EraseSurfaceRGBA(SSurface *surface, U_INT rgba)
{ if (0 == surface) return;
  U_INT *buf  = surface->drawBuffer;
  for (U_INT i=0; i< surface->nPixel; i++) *buf++ = rgba;
  return;  
}
//-------------------------------------------------------------------------
//  Erase the number of pixels with marge
//-------------------------------------------------------------------------
void EraseSurfaceRGBA(SSurface *surface, U_INT rgba, int mrg)
{ int aj = (mrg << 1);                        // 2 * marge
  int nc = surface->xSize - aj;       // Number of columns
  int nl = surface->ySize - aj;       // Number of lines
  U_INT *dst = surface->drawBuffer + (mrg * surface->xSize) + mrg - aj;
  for (int ly=0; ly < nl; ly++)
  { dst += aj;
    for (int cx=0; cx < nc; cx++) *dst++ = rgba;
  }
  return;
}
//---------------------------------------------------------------------------
//  Left shift the surface by n pixels, starting at col s 
//---------------------------------------------------------------------------
void LeftShiftSurface(SSurface *sf,int n,int s,U_INT rgba)
{ U_INT *dst = sf->drawBuffer;
  U_INT *src = dst;
  U_INT  str = s   + n;         // Starting column
  if (str >= sf->xSize)   return ;
  //-- Start shift line by line ----------------------
  for (U_INT nl = 0; nl < sf->ySize; nl++)
  { //---for each line left shift nbs pixels ---------
    src   += str;             // Start position
    dst   += s;               // receiver 
    U_INT nc = str;
    while (nc++ < sf->xSize)  *dst++ = *src++;
    //---Fill end of line with rgba ------------------
    while (dst != src)  *dst++ = rgba;
  }
  return;
}
//-------------------------------------------------------------------------
//  MAke a texture from surface
//-------------------------------------------------------------------------
GLuint TextureFromSurface (SSurface *s, bool mipmap)
{
  GLuint texid = 0;

  // Check that surface is square and a power of two in size
  if ((s->xSize) == (s->ySize)) {
    // Create a GL texture
    glGenTextures (1, &texid);
    if (texid == 0) {
      GLenum err = glGetError();
      WARNINGLOG ("TextureFromSurface : Failed to generate texture, error = 0x%08X", err);
    } else {
      glBindTexture (GL_TEXTURE_2D, texid);
      glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

      GLenum format = GL_RGBA;
      int depth = 4;
      GLubyte *image = (unsigned char*)(s->drawBuffer);

      if (mipmap) {
        // Generate mipmapped texture
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        gluBuild2DMipmaps (GL_TEXTURE_2D, depth, s->xSize, s->ySize, format,
          GL_UNSIGNED_BYTE, image);
      } else {
        // Generate non-mipmapped texture
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexImage2D (GL_TEXTURE_2D, 0, format, s->xSize, s->ySize, 0,
          format, GL_UNSIGNED_BYTE, image);
      }
    }
  }

  return texid;
}

//==============================================================================
// Draw a single pixel in the drawing surface to the specified colour
//   (x,y) are relative to the top-left corner of the surface, increasing
//   to the right and down
//==============================================================================
void  DrawDot(SSurface *surface, int x, int y, unsigned int rgba)
{
  if (surface != NULL) {
    // Clip to surface boundaries
    if ((x >= 0) &&
        (y >= 0) &&
        (x < (int)surface->xSize) &&
        (y < (int)surface->ySize))
    {
      int offset  = surface->invert - (y * surface->xSize) + x;
      surface->drawBuffer[offset] = rgba;
    }
  }
}
//===========================================================================================
//  JSDEV* Draw Horizontal line to surface. Avoid Drawdot and multiply per pixel
//===========================================================================================
void DrawHLine(SSurface *sf, int x1,int x2, int y,U_INT rgba)
{ if (y < 0)                  return;                     // Clip above
  if (y >= int(sf->ySize))    return;                     // Clip below
  if (x1 > x2)  {DrawHLine(sf,x2,x1,y,rgba); return;}
  if (x1 < 0)                 x1 = 0;                     // Clip left
  if (x2 >= int(sf->xSize))   x2 = sf->xSize - 1;         // Clip right
  short yl  = sf->ySize - 1 - y;
  U_INT *buf = sf->drawBuffer + (yl * sf->xSize) + x1;
  while (x1++ <= x2)  *buf++ = rgba;
  return;
}
//===========================================================================================
//  JSDEV* Draw Vertical line to surface. Avoid DrawDot with multiply per pixel
//===========================================================================================
void DrawVLine(SSurface *sf,int xl,int y1,int y2,U_INT rgba)
{ if (xl < 0)                 return;                     // Clip left
  if (xl >= int(sf->xSize))   return;                     // Clip right
  if (y1 > y2)  {DrawVLine(sf,xl,y2,y1,rgba); return;}
  if (y1 < 0)                 y1 = 0;                     // Clip above
  if (y2 >= int(sf->ySize))   y2 = sf->ySize - 1;         // Clip below
  short yf  = sf->ySize - 1 - y1;
  short yd  = sf->ySize - 1 - y2;
  U_INT *buf = sf->drawBuffer + (yd * sf->xSize) + xl;
  while (yd++ <= yf)  {*buf = rgba; buf += sf->xSize;}
  return;
  }
//-------------------------------------------------------------------------------------------
//  Draw a pixel in the surface if not clipped
//-------------------------------------------------------------------------------------------
void DrawPixel(SSurface *sf,int x, int y, U_INT rgba,U_INT *buf)
{ if (x < 0)                return;           // Clip left
  if (x >= int(sf->xSize))  return;           // Clip right
  if (y < 0)                return;           // Clip above
  if (y >= int(sf->ySize))  return;           // Clip below
  *buf = rgba;
  return;
}
//=============================================================================
// Draw a continuous line segment starting at (and including) (x1, y1) and
//   ending at (and including) (x2, y2).  This is an implementation of
//   Bresenham's algorithm.
//=============================================================================
void  DrawFastLine(SSurface *surface, int x1, int y1, int x2, int y2, unsigned int rgba)
{ int cx, cy;
  int di;
  int p1,p2;
  int slope;
  int     wd  = surface->xSize;
  U_INT  *buf = surface->drawBuffer + (surface->ySize - 1 - y1) * wd + x1;
  // First set start point and calculate delta in x and y endpoints
  int dx = x2 - x1;
  int dy = y2 - y1;

  // Check for special cases
  if ((dx == 0) && (dy == 0))   {DrawDot  (surface,x1,y1,rgba);     return; }
  if (dx == 0)                  {DrawVLine(surface,x1,y1,y2,rgba);  return; }
  if (dy == 0)                  {DrawHLine(surface,x1,x2,y1,rgba);  return; }
    // Determine whether to step in x-direction or y-direction
  if (abs(dx) >= abs(dy) ) {
      // Step in x direction
      // First endpoint must be the leftmost
      if (x1 > x2)  {DrawFastLine (surface, x2, y2, x1, y1, rgba);  return; }

      // Adjust y-increment for negatively sloped lines
      if (dy < 0) {slope = -1; dy = -dy; } else {slope = 1; wd = - wd;}
      // Calculate constants for Bresenham's algorithm
      p1  = (dy << 1);
      p2  = p1 - (dx << 1);
      di  = p1 - dx;
      cy  = y1;
      // Step through each x-coordinate, drawing the appropriate pixels
      while (x1 != x2) 
      { DrawPixel (surface, x1, cy, rgba, buf);
        x1++;
        buf += 1;
        if (di <= 0) {di += p1;} else {di += p2; cy += slope; buf += wd;}
      }
      return;
  } 
  // Step in y direction
  // First endpoint must be the topmost
  if (y1 > y2) { DrawFastLine(surface,x2,y2,x1,y1,rgba); return;}

  // Adjust x-increment for negatively sloped lines
  if (dx < 0) {slope = -1; dx = - dx;} else {slope = 1;}

  // Calculate constants for Bresenham's algorithm
  p1    = (dx << 1);
  p2    = p1 - (dy << 1);
  di    = p1 - dy;
  cx    = x1;
 // Step through each y-coordinate, drawing the appropriate pixels
      while (y1 != y2)
      { DrawPixel (surface, cx, y1, rgba,buf);
        y1++;
        buf -= wd;
        if (di <= 0) {di += p1;} else {di += p2; cx += slope; buf += slope;}
      }
}
//===========================================================================================
//    Draw an RGBA Texture on a Surface
//===========================================================================================
void DrawImage(SSurface *sf, S_IMAGE &ref)
{ int    x  = ref.x0;
  int    y  = ref.y0;
  short xs  = (x < 0)?(0):(x);
  if (xs >= short(sf->xSize))   return;         // Clip: bitmap in the right side of rectangle
  short ys  = sf->ySize - y - ref.ht;
  if (ys >= short(sf->ySize))   return;         // Clip: bitmap above rectangle
  //----Compute start position in bitmap ------------------
  short xb  = (x < 0)?(-x):(0);
  if (xb >= ref.wd)             return;         // clip: bitmap in the left side of rectangle
  short yb  = (ys < 0)?(-ys):(0);
  if (yb >= ref.ht)             return;         // clip: bitmap below the rectangle
  if (ys < 0) ys = 0;
  //----Compute number of line and column ----------------
  short dx1 = (sf->xSize - xs);
  short dx2 = (ref.wd - xb);
  short nc  = (dx1 < dx2)?(dx1):(dx2);
  short dy1 = (sf->ySize - ys);
  short dy2 = (ref.ht -yb);
  short nl  = (dy1 < dy2)?(dy1):(dy2);
  //----Compute buffer correction factor -----------------
  short cs  = (sf->xSize - nc);
  short cb  = (ref.wd    - nc);
  //----Compute buffer  starting pointers ----------------
  U_INT *surf  = sf->drawBuffer  + (ys * sf->xSize) + xs;
  U_INT *bmap  = ref.rgba        + (yb * ref.wd)    + xb;

  //----Draw to surface ----------------------------------
  while (nl--) 
  { short cn = nc;
    while (cn--)  {*surf++ = *bmap++;}
    bmap  += cb;
    surf  += cs;
  }
  return;
}

//===========================================================================================
// Draw an unfilled rectangle with corners (x1, y1) and (x2, y2).
//===========================================================================================
void  DrawRect(SSurface *surface, int x1, int y1, int x2, int y2, unsigned int rgba)
{
  DrawHLine (surface, x1, x2, y1, rgba);        // Top
  DrawVLine (surface, x2, y1, y2, rgba);        // Right
  DrawHLine (surface, x2, x1, y2, rgba);        // Bottom
  DrawVLine (surface, x1, y2, y1, rgba);        // Left
}

//-------------------------------------------------------------------------------------------
// Draw a filled rectangle with corners (x1, y1) and (x2, y2).  The sides of
//   the rectangle must be parallel to the axes (sides) of the surface
//-------------------------------------------------------------------------------------------
void  FillRect(SSurface *surface, int x1, int y1, int x2, int y2, unsigned int rgba)
{
  // x1, y1 must be the top coordinate
  int height = (y2 - y1);
  if (height < 0) {FillRect (surface, x2, y2, x1, y1, rgba); return;}
  for (int row=0; row<height; row++) DrawHLine(surface,x1,x2,y1+row,rgba);
  return;
}

//-------------------------------------------------------------------------------------------
//  Draw a circle. Bresenham's algorithm and Michener
//-------------------------------------------------------------------------------------------
void  DrawCircle(SSurface *surface, int xCenter, int yCenter, int radius, unsigned int rgba)
{ int dx  = 0;
  int dy  = radius;
  int dv  = 3 - (2 * radius);
  if (radius == 0)   return;
  while (dx < dy) 
  { DrawDot(surface, xCenter + dx, yCenter + dy, rgba);
    DrawDot(surface, xCenter + dy, yCenter + dx, rgba);
    DrawDot(surface, xCenter + dy, yCenter - dx, rgba);
    DrawDot(surface, xCenter + dx, yCenter - dy, rgba);
    DrawDot(surface, xCenter - dx, yCenter - dy, rgba);
    DrawDot(surface, xCenter - dy, yCenter - dx, rgba);
    DrawDot(surface, xCenter - dy, yCenter + dx, rgba);
    DrawDot(surface, xCenter - dx, yCenter + dy, rgba);
    if (dv < 0) {dv +=  (dx << 2) +  6; }
    else { dv += ((dx - dy) << 2) + 10; dy--;}
    dx++; }
  return;
}
//=============================================================================
//   Draw a circle using  Bresenham's algorithm. Canter at Cx,Cy
//    Predict buffer placement to avoid DrawDot
//  NOTE Not working yet. DO NOT USE
//=============================================================================
void  DrawFastCircle(SSurface *sf, int Cx, int Cy, int radius, U_INT rgba)
{ int   dx  = 0;
  int   dy  = radius;
  int   yc  = Cy;         //sf->ySize - 1 - Cy;
  //----------decision variable = 3 - 2*radius ---------
  int   dv  = 3 - (radius << 1);
  if  (radius == 0) return;
  //-------Start at D0(Cx,yc+radius) --------------------
  U_INT *buf  = sf->drawBuffer + ((yc + radius)* sf->xSize);
  U_INT *bd0  = 0;
  int     M2  = (radius << 1) * sf->xSize;          // M2 initial value
  int     M4  = 0;                                  // M4 initial value
  //-------Generate the first octant only. Then use various symetries ---
  while (dx < dy)
  { bd0 = buf;                                      // Save D0 starting point
    DrawPixel(sf,Cx + dx, yc + dy, rgba, buf);      // DO
    buf -= (dx << 1);                               // D7 symetry about OY
    DrawPixel(sf,Cx - dx, yc + dy, rgba, buf);      // D7 draw
    buf += M2;                                      // D4 symetry about OX
    DrawPixel(sf,Cx - dx, yc - dy, rgba, buf);      // D4 draw
    buf += (dx << 1);                               // D3 Symetry about OY
    DrawPixel(sf,Cx + dx, yc - dy, rgba, buf);      // D3 Draw
  //------Generate D5 at 90° offset of D3 -------------------
    buf += (dx*(sf->xSize - 1)) - (dy*(sf->xSize + 1));
    DrawPixel(sf,Cx - dy, yc - dx, rgba, buf);      // D5 draw
    buf -= M4;                                      // D6 symetry about OX
    DrawPixel(sf,Cx - dy, yc + dx, rgba, buf);      // D6 draw
    buf += (dy << 1);                               // D1 symetry about OY
    DrawPixel(sf,Cx + dy, yc + dx, rgba, buf);      // D1 draw
    buf += M4;                                      // D2  symetry about OX
    DrawPixel(sf,Cx + dy, yc - dx, rgba, buf);      // D draw
    buf  = bd0;                                     // Back to D0
    //-------step on  X and decide for Y backstep ------------
    dx++;
    M4  += (sf->xSize << 1);                        // Adjust M4
    buf += 1;                                       // Adjust buf
    if (dv < 0) {dv +=  (dx << 2) +  6; continue; }
    dv += ((dx - dy) << 2) + 10; 
    dy--;
    M2  -= (sf->xSize << 2);
    buf -= (sf->xSize);
  }
  return;
}

//-------------------------------------------------------------------------------------------
unsigned int  MakeRGB(unsigned int r, unsigned int g, unsigned int b)
{
  return (r << 0) + (g << 8) + (b << 16) + (0xff << 24);
}

unsigned int  MakeRGBA(unsigned int r, unsigned int g, unsigned int b, unsigned int a)
{
  return (r << 0) + (g << 8) + (b << 16) + (a << 24);
}

void UnmakeRGB (unsigned int rgb, unsigned int *r, unsigned int *g, unsigned int *b)
{
  *r =  rgb & 0x000000FF;
  *g = (rgb & 0x0000FF00) >> 8;
  *b = (rgb & 0x00FF0000) >> 16;
}

//==========================================================================
//  BLIT for FUI
//==========================================================================
void  Blit(SSurface *surface)
{
  glDrawBuffer (GL_BACK);
  glDrawPixels (surface->xSize, surface->ySize,
    GL_RGBA,
    GL_UNSIGNED_BYTE,
    surface->drawBuffer);
}

void  BlitTransparent(SSurface *surface, unsigned int rgbTransparentColor)
{
  glDrawBuffer (GL_BACK);
  glDrawPixels (surface->xSize, surface->ySize,
    GL_RGBA,
    GL_UNSIGNED_BYTE,
    surface->drawBuffer);
}


//=========================================================================
// PBM (Single-frame) Bitmap
//=========================================================================
class CBitmapPBM: public CBaseBitmap {
public:
  // Constructors
   CBitmapPBM (char* pbmFilename = NULL);
  ~CBitmapPBM (void);
  //--------------------------------------------------------------------
  void  DrawBitmap  (SSurface *sf,int x,int y,int fr = 0);
  void  FillTheRect (SSurface *sf,int x,int y,int wd, int ht,int fr = 0);
  void  DrawPartial (SSurface *sf,int x,int y,int x1,int y1,int x2,int y2,int fr=0);
  void  DrawFrom    (SSurface *sf,int pos,int nbl, int x, int y);
  //--------------------------------------------------------------------
  void  GetBitmapSize(int *x, int *y) {*x = width; *y = height;}
  int   NumBitmapFrames() {return 1;}
  //--------------------------------------------------------------------
  void  Load (PODFILE *pbm, PODFILE *act);
  void  DrawFast    (SSurface *surface, int x, int y);
  void  ScanLine(short nc);
  U_CHAR *GetRGBA() {return bmap;}
  //--------------------------------------------------------------------
protected:
//  char     pbmFilename[64];
  RGB      *palette;
  GLubyte  *image;
  int      width, height;
  GLubyte  *bmap;
  U_INT    *surf;
};
//-------------------------------------------------------------------------
//  Constructor
//-------------------------------------------------------------------------
CBitmapPBM::CBitmapPBM (char *pbmName)
{
  // Initialize
  image = NULL;
  width = height = 0;

  if (pbmName == 0) return;

    // Construct full names for PBM and ACT files
    char pbmFilename[256];
    char actFilename[256];
    strcpy (pbmFilename, pbmName);
    strcpy (actFilename, pbmName);
    char *pExt = strrchr (actFilename, '.');
    if (pExt != NULL) {
      pExt++;
      strcpy (pExt, "ACT");
    } else {
      gtfo ("CBitmapPBM : Invalid PBM filename %s", pbmFilename);
    }

    // Open PBM and ACT files in default pod filesystem
    PODFILE* pbm = popen (&globals->pfs, pbmFilename);
    PODFILE* act = popen (&globals->pfs, actFilename);
    if ((pbm != NULL) && (act != NULL)) {
      Load (pbm, act);
      pclose (pbm);
      pclose (act);
    } else {
      gtfo ("CBitmapPBM : Could not open PBM or ACT file for %s", pbmName);
    }
    loaded = 1;
}
//-------------------------------------------------------------------------
//  Destructor
//-------------------------------------------------------------------------
CBitmapPBM::~CBitmapPBM (void)
{
  SAFE_DELETE(palette);
  SAFE_DELETE(image);
}
//-------------------------------------------------------------------------
//  Load the Pod file
//-------------------------------------------------------------------------
void CBitmapPBM::Load (PODFILE *pbm, PODFILE *act)
{
  int i;

  if ((pbm == NULL) || (act == NULL))   return;

  // Load colormap from the ACT file
  palette = new RGB[0x100];
  pread (palette, 3, 0x100, act);

  // Read PBM header
  SPBMHeader h;
  pread (&h, sizeof(SPBMHeader), 1, pbm);
  if (h.magic != 0x1A4D4250) {    // 'PBM<Esc>' in Little-Endian format
    gtfo ("CBitmapPBM : Invalid magic value, %s", pbm->filename);
  }
  width = h.width;
  height = h.height;

  // Read PBM row offset table. This is a list of (height+1) offsets to the
  //   start of each row's data.  The last element is the size of the entire
  //   data area, which allows for simple calculation of the last row's size
  unsigned long *rowOffset = new unsigned long[height+1];
  pread (rowOffset, sizeof(unsigned long), height+1, pbm);

  unsigned long *rowLength = new unsigned long[height];
  for (i=0; i<height; i++) {
    // Calculate data length for this row
    rowLength[i] = rowOffset[i+1] - rowOffset[i];
  }

  // Pre-load all image data into memory for faster processing
  int rowDataSize = rowOffset[height];
  unsigned char *rowData = new unsigned char[rowDataSize];
  pread (rowData, 1, rowDataSize, pbm);

  // Allocate storage for the 32bpp image
  image = new GLubyte[width*height];

  // Initialize entire image to palette index 0 (transparent)
  memset (image, 0, width*height);

  for (i=0; i<height; i++) {

    unsigned char *p = &rowData[rowOffset[i]];

    // Each row consists of a series of data chunks, each chunk representing
    //   a non-transparent series of pixels.

    // Continue reading data chunks until finished this row
    unsigned long iRow = 0;
    while (iRow < rowLength[i]) {
      // Read pixel colum
      short column;
      column = *((short*)p);
      p += sizeof(short);
      iRow += sizeof(short);

      // Read pixel count
      short count;
      count = *((short*)p);
      p += sizeof(short);
      iRow += sizeof(short);

      // Read data (palette index) for each pixel in the chunk
      for (short j=0; j<count; j++) {
        unsigned char iCmap;
        iCmap = *p;
        p++;
        iRow++;

        // Flip image vertically
        unsigned int offset = (((height - i - 1) * width) + column + j);

        // Set pixel colour, and transparency to fully opaque
        image[offset] = iCmap;
      }

      // Pad to next long boundary
      while ((iRow % 4) != 0) {
        p++;
        iRow++;
      }
    }
  }

  // Clean up allocated data
  SAFE_DELETE( rowOffset);
  SAFE_DELETE( rowLength);
  SAFE_DELETE( rowData);
}

//---------------------------------------------------------------------
//  DrawBitmap
//---------------------------------------------------------------------
void CBitmapPBM::DrawBitmap (SSurface *sf, int x, int y,int fr)
{ DrawFast(sf,x,y);
  return;
}
//=====================================================================
//  JSDEV* Faster Draw bitmap by scanline method 
//         Avoid the multiplication per pixel in DrawDot()
//=====================================================================
void CBitmapPBM::DrawFast(SSurface *sf, int x, int y)
{ if (0 == image)   return;
  //----compute start position in rectangle ------------------
  short xs  = (x < 0)?(0):(x);
  if (xs >= short(sf->xSize))   return;         // Clip: bitmap in the right side of rectangle
  short ys  = sf->ySize - y - height;
  if (ys >= short(sf->ySize))   return;         // Clip: bitmap above rectangle
  //----Compute start position in bitmap ------------------
  short xb  = (x < 0)?(-x):(0);
  if (xb >= width)              return;         // clip: bitmap in the left side of rectangle
  short yb  = (ys < 0)?(-ys):(0);
  if (yb >= height)             return;         // clip: bitmap below the rectangle
  if (ys < 0) ys = 0;
  //----Compute number of line and column ----------------
  short dx1 = (sf->xSize - xs);
  short dx2 = (width - xb);
  short nc  = (dx1 < dx2)?(dx1):(dx2);
  short dy1 = (sf->ySize - ys);
  short dy2 = (height -yb);
  short nl  = (dy1 < dy2)?(dy1):(dy2);
  //----Compute buffer correction factor -----------------
  short cs  = (sf->xSize - nc);
  short cb  = (width     - nc);
  //----Compute buffer  starting pointers ----------------
  surf      = sf->drawBuffer  + (ys * sf->xSize) + xs;
  bmap      = image           + (yb * width)  + xb;
  //----Draw to surface ----------------------------------
  while (nl--) 
  { ScanLine(nc);
    bmap += cb;
    surf += cs;
  }
  return;
}
//--------------------------------------------------------------------
//  Scan line
//--------------------------------------------------------------------
void  CBitmapPBM::ScanLine(short nc)
{ GLubyte  pix;
  U_INT  alf = 0xFF000000;
  while (nc--)
  { pix = *bmap++;
    if (0 == pix)  { surf++; continue;}
    *surf++ = alf | (palette[pix].b << 16) | (palette[pix].g << 8) | palette[pix].r;
  }
  return;
}
//---------------------------------------------------------------------
//  Draw fast from line position in bitmap
//  used for digits bitmap
//---------------------------------------------------------------------
void CBitmapPBM::DrawFrom(SSurface *sf,int pos,int nbl,int x, int y)
{ if (0 == image)   return;
  //----compute start position in rectangle ------------------
  short xs  = (x < 0)?(0):(x);
  if (xs >= short(sf->xSize))   return;         // Clip: bitmap in the right side of rectangle
  short ys  = sf->ySize - y - nbl;
  if (ys >= short(sf->ySize))   return;         // Clip: bitmap above rectangle
  //----Compute start position in bitmap ------------------
  short xb  = (x < 0)?(-x):(0);
  if (xb >= width)              return;         // clip: bitmap in the left side of rectangle
  short yb  = (ys < 0)?(-ys):(0);
  yb  += pos;
  if (yb >= height)             return;         // clip: bitmap below the rectangle
  if (ys < 0) ys = 0;
  //----Compute number of line and column ----------------
  short dx1 = (sf->xSize - xs);
  short dx2 = (width - xb);
  short nc  = (dx1 < dx2)?(dx1):(dx2);
  short dy1 = (sf->ySize - ys);
  short nl  = (dy1 < nbl)?(dy1):(nbl);
  //----Compute buffer correction factor -----------------
  short cs  = (sf->xSize - nc);
  short cb  = (width     - nc);
  //----Compute buffer  starting pointers ----------------
  surf      = sf->drawBuffer  + (ys * sf->xSize) + xs;
  bmap      = image           + (yb * width)  + xb;
  //----Draw to surface ----------------------------------
  while (nl--) 
  { ScanLine(nc);
    bmap += cb;
    surf += cs;
  }
  return;
}
//---------------------------------------------------------------------
//  Fill a given rectangle in the surface
//---------------------------------------------------------------------
void CBitmapPBM::FillTheRect(SSurface *sf,int px,int py, int wd, int ht,int fr)
{ int bx, by;
  GetBitmapSize(&bx, &by);
  for (int iy=py; iy<(py + ht); iy+=by)
  { for (int ix=px; ix<(px + wd); ix+=bx) DrawFast(sf, ix, iy); }
  return;
}
//---------------------------------------------------------------------
//  Draw partial (not used)
//---------------------------------------------------------------------
void CBitmapPBM::DrawPartial(SSurface *sf,int x,int y,int x1,int y1,int x2,int y2,int fr)
{
  // Check that rendered bitmap will fit in the surface
  int wDraw = x2 - x1;
  int hDraw = y2 - y1;

  /// \todo Support clipping of the drawn bitmap by the surface edges
  // Copy bitmap pixels to surface image buffer
  int sybase = sf->ySize - y - hDraw;
  long          bmOffset;
  int           iCmap;
  unsigned int  rgba;
  int           sx;
  int           sy;
  int           surfOffset;

  for (int i=0; i<hDraw; i++)
  {
    for (int j=0; j<wDraw; j++)
    {
      bmOffset = (((y1 + i) * width) + (x1 + j));
      iCmap = image[bmOffset];

      // Copy only non-transparent pixels to the surface
      if (iCmap != 0) {
        GLubyte r = palette[iCmap].r;
        GLubyte g = palette[iCmap].g;
        GLubyte b = palette[iCmap].b;
        GLubyte a = 0xFF;
        rgba = MakeRGBA (r, g, b, a);

        // Copy pixel value to surface buffer, clipping against edges
        sx = x+j;
        sy = sybase+i;
        if ((sx >= 0) && (sx < (int)sf->xSize) &&
            (sy >= 0) && (sy < (int)sf->ySize))
        {
          surfOffset = (sy * sf->xSize) + sx;
          sf->drawBuffer[surfOffset] = rgba;
        }
      }
    }
  }
}


//========================================================================
// PBG (Multi-frame) Bitmap
//========================================================================
class CBitmapPBG: public CBaseBitmap  {
public:
  // Constructor
   CBitmapPBG (char* pbgFilename);
  ~CBitmapPBG (void);
  //--------------------------------------------------------------------
  void  DrawBitmap (SSurface *sf,int x,int y,int fr);
  void  FillTheRect(SSurface *sf,int x,int y,int wd, int ht,int fr);
  void  DrawPartial(SSurface *sf,int x,int y,int x1,int y1,int x2,int y2,int fr);
  void  DrawFrom    (SSurface *sf,int pos,int nbl, int x, int y) {}
  //---------------------------------------------------------------------
  void  GetBitmapSize(int *x, int *y);
  int   NumBitmapFrames() {return nFrames;}
  //---------------------------------------------------------------------
  // CBitmapPBG methods
  void  Load (PODFILE *pbm, PODFILE *act);
protected:
  int                 nFrames;  ///< Number of frames
  std::deque<CBitmapPBM*> pbm;      ///< List of CBitmapPBM*, one for each frame
};
//-------------------------------------------------------------------------
//  Constructor
//-------------------------------------------------------------------------
CBitmapPBG::CBitmapPBG (char *pbgName)
{
  // Initialize
  nFrames = 0;

  if (pbgName != NULL) {
    // Construct full names for PBG and ACT files
    char pbgFilename[256];
    char actFilename[256];
    strcpy (pbgFilename, pbgName);

    strcpy (actFilename, pbgFilename);
    char *pExt = strrchr (actFilename, '.');
    if (pExt != NULL) {
      pExt++;
      strcpy (pExt, "ACT");
    } else {
      gtfo ("CBitmapPBG : Invalid PBG filename %s", pbgFilename);
    }

    // Open PBM and ACT files in default pod filesystem
    PODFILE* pbg = popen (&globals->pfs, pbgFilename);
    PODFILE* act = popen (&globals->pfs, actFilename);
    Load (pbg, act);
    pclose (pbg);
    pclose (act);
  }   
}
//-------------------------------------------------------------------------
//  Destructor
//-------------------------------------------------------------------------
CBitmapPBG::~CBitmapPBG (void)
{
  std::deque<CBitmapPBM*>::iterator i;
  for (i=pbm.begin(); i!=pbm.end(); i++) {
    CBitmapPBM *bm = *i;
    SAFE_DELETE( bm);
  }
}
//-------------------------------------------------------------------------
typedef struct {
  unsigned long magic;
  unsigned long nFrames;
} SPBGHeader;
//-------------------------------------------------------------------------
//  Load from Pod file
//-------------------------------------------------------------------------
void CBitmapPBG::Load (PODFILE *pbg, PODFILE *act)
{
  if ((pbg == NULL) || (act == NULL)) {
    return;
  }

  // Read PBG header
  SPBGHeader h;
  pread (&h, sizeof(SPBGHeader), 1, pbg);
  if (h.magic != 0x1A474250) {    // 'PBG<Esc>' in Little-Endian format
    gtfo ("CBitmapPBG : Invalid magic value, %s", pbg->filename);
  }
  nFrames = h.nFrames;

  for (int i=0; i<nFrames; i++) {
    CBitmapPBM *bm = new CBitmapPBM;
    prewind (act);
    bm->Load (pbg, act);
    pbm.push_back (bm);
  }
  loaded = 1;
}
//-------------------------------------------------------------------------
//  Return the sizes
//-------------------------------------------------------------------------
void CBitmapPBG::GetBitmapSize(int *x, int *y)
{ if (pbm.size() != 0) {
    // Get size of first PBM entry (assume they are all the same size!)
    pbm[0]->GetBitmapSize (x, y);
  } else {
    // Zero frames ?
    *x = 0;
    *y = 0;
  }
}
//---------------------------------------------------------------------
//  Draw the bitmap
//---------------------------------------------------------------------
void CBitmapPBG::DrawBitmap(SSurface *surface, int x, int y, int fr)
{ if (fr >=  nFrames) return;
  CBitmapPBM *bm = pbm[fr];
  bm->DrawFast (surface, x, y);                   // JSDEV*
  return;
}
//---------------------------------------------------------------------
//  Fill a given rectangle in the surface
//---------------------------------------------------------------------
void CBitmapPBG::FillTheRect(SSurface *sf,int px,int py, int wd, int ht,int fr)
{ if (fr >=  nFrames) return;
  CBitmapPBM *bm = pbm[fr];
  bm->FillTheRect(sf,px,py,wd,ht);  
  return;
}
//-------------------------------------------------------------------------
//  Draw partial (not used)
//-------------------------------------------------------------------------
void CBitmapPBG::DrawPartial(SSurface *sf,int x,int y,int x1,int y1,int x2,int y2,int fr)
{ if (fr >= nFrames) return;
  CBitmapPBM *bm = pbm[fr];
  bm->DrawPartial (sf, x, y, x1, y1, x2, y2);
  return;
}
//=======================================================================
// BMP (Single-frame Windows Bitmap)
//=======================================================================
class CBitmapBMP: public CBaseBitmap {
public:
  // Constructors
   CBitmapBMP (char* bmpFilename = NULL);
  ~CBitmapBMP (void);
  //--------------------------------------------------------------
  void DrawBitmap(SSurface *sf,int x,int y, int fr = 0);
  void GetBitmapSize(int *x, int *y) {*x = wd, *y = ht;}
  void DrawPartial(SSurface *sf,int x,int y,int x1,int y1,int x2,int y2,int fr = 0);
  void DrawFrom    (SSurface *sf,int pos,int nbl, int x, int y) {}
  void SetFrameNb(int k);
  int  NumBitmapFrames() {return NbFrame;}
  //--------------------------------------------------------------
  U_CHAR *GetRGBA()     {return bmap;}
  //--------------------------------------------------------------
  void Load (PODFILE *bmp);
  //---------------------------------------------------------------
  void DrawPalBMP (SSurface *surface, int x, int y, int fr);
  void ScanPalBMP (short nc,short cb,short cs);
  void DrawRgbBMP (SSurface *surface, int x, int y, int fr);
  void ScanRgbBMP (short nc,short cb,short cs);
  //---------------------------------------------------------------
protected:
  char            filename[PATH_MAX];
  SBmpHeader      h;
  SBmpInfoHeader  ih;
  GLubyte*        cmap;
  GLubyte*        image;
  int             wd, ht;   // pixels, rows
  int             rowSize;  // Row size (bytes)
  U_INT           trns;     // Transparent color
  //---------Frame parameters -------------------
  int             NbFrame;
  int             frSize;   // Frame size (bytes)
  //----------------------------------------------
  U_INT           *surf;
  GLubyte         *bmap;
  GLubyte         *base;
  short            pDim;    // Pixel dim (bytes)
  short            bDim;    // Byte dimension for line
  int              nbr;
};
//---------------------------------------------------------------------
//  Constructor with Loading included
//----------------------------------------------------------------------
CBitmapBMP::CBitmapBMP (char *bmpFilename)
{
  // Initialize
  image = 0;
  cmap  = 0;
  wd    = ht = 0;
  strcpy (filename, bmpFilename);

  // Try to load from BMP file in POD filesystem
  PODFILE* p = popen (&globals->pfs, filename);
  if (p != NULL) {
    Load (p);
    pclose (p);
  }
}
//---------------------------------------------------------------
//  Destructor
//----------------------------------------------------------------
CBitmapBMP::~CBitmapBMP (void)
{
  SAFE_DELETE (cmap);
  SAFE_DELETE (image);
}
//-----------------------------------------------------------------
//  Load the bitmap
//-----------------------------------------------------------------
void CBitmapBMP::Load (PODFILE *p)
{ int pm;     // For test
  // Read header
  pread (&h, sizeof(SBmpHeader), 1, p);

  // Read info header
  pread (&ih, sizeof(SBmpInfoHeader), 1, p);

  // Set width/height
  wd = ih.w;
  ht = ih.h;
  pDim  = ih.bpp / 8;     // Bytes per pixel
  //---- Read colormap data -----
  if (ih.nColours != 0) {
    int cmapSize = ih.nColours * 4;
    cmap = new unsigned char[cmapSize];
    pm   = pread (cmap, 1, cmapSize, p);
  }
  // Read image data
  if (0 != ih.compression)  gtfo("Bitmap BMP %s:Unsupported compression mode",filename);
  // calculate row size (always an even longword boundary)
  rowSize = wd * (pDim);
  if ((rowSize % 4) != 0) rowSize += 4 - (rowSize % 4);
  int dim = rowSize * ht;
  frSize  = dim;
  NbFrame = 1;
  image   = new GLubyte[dim];
  pseek (p,h.offset,0);
  pm = pread (image, 1,(rowSize * ht), p);
  loaded  = 1;
  return;
}
//-----------------------------------------------------------------
//  Reconfigure parameters for the indicated frame number
//-----------------------------------------------------------------
void CBitmapBMP::SetFrameNb(int k)
{ NbFrame = k;
  int dim = frSize / k;
  int rst = frSize % k;
  if (rst)  
    gtfo("Bitmap size can't be divided by the frame number");
  frSize  = dim;
  ht      = ht / k;
  return;
}
//=========================================================================
//  Draw a BMP bitmap
//=========================================================================
void CBitmapBMP::DrawBitmap (SSurface *sf, int x, int y,int fr)
{ if ( 0 == image)  return;
  base = image + ((NbFrame - fr - 1) * frSize);
  if  (1 == pDim) {DrawPalBMP(sf,x,y,fr); return;}
  if  (3 == pDim) {DrawRgbBMP(sf,x,y,fr); return;}
  return;
}
//=========================================================================
//  JSDEV*:  Faster Draw RGB BMP by scan line. 
//          Avoid the multiplication for each pixel in DrawDot()
//=========================================================================
void CBitmapBMP::DrawRgbBMP(SSurface *sf, int x, int y, int fr)
 { short xs  = (x < 0)?(0):(x);
  if (xs >= short(sf->xSize))   return;         // Clip: bitmap in the right side of rectangle
  short ys  = sf->ySize - y - ht;
  if (ys >= short(sf->ySize))   return;         // Clip: bitmap below rectangle
  //----Compute start position in bitmap ------------------
  short xb  = (x < 0)?(-x):(0);
  if (xb >= wd)                 return;         // clip: bitmap in the left side of rectangle
  short yb  = (ys < 0)?(-ys):(0);
  if (yb >= ht)                 return;         // clip: bitmap above the rectangle
  if (ys < 0) ys = 0;  
  //----Compute number of line and column ----------------
  short dx1 = (sf->xSize - xs);
  short dx2 = (wd - xb);
  short nc  = (dx1 < dx2)?(dx1):(dx2);
  short dy1 = (sf->ySize - ys);
  short dy2 = (ht - yb);
  short nl  = (dy1 < dy2)?(dy1):(dy2);
  //----Compute buffer correction factor -----------------
  short cs  = (sf->xSize - nc);
  short cb  = (rowSize   - (nc * pDim));
  //----Compute buffer  starting pointers ----------------
  surf      = sf->drawBuffer  + (ys * sf->xSize) + xs;
  bmap      = base            + (yb * rowSize)   + xb;
  //----Draw to surface ----------------------------------
  nbr       = 0;
  while (nl--) ScanRgbBMP(nc,cb,cs);
  return;
 }
//--------------------------------------------------------
//  Scan one line
//  skip pixel if  transparent color
//--------------------------------------------------------
void CBitmapBMP::ScanRgbBMP (short nc,short cb,short cs)
{ U_INT pix;
  while (nc--)
  { GLubyte b = *bmap++;
    GLubyte g = *bmap++;
    GLubyte r = *bmap++;
    pix = MakeRGBA(r,g,b,0xFF);
    if (pix == 0xFF000000) {surf++; continue;}
    if (pix == 0xFFFF00FF) {surf++; continue;}
    *surf++ = pix;
  }
  bmap  += cb;
  surf  += cs;
  return;
}
//=========================================================================
//  JSDEV*:  Draw palettized BMP by scan line. Avoid mulpiply for each pixel
//=========================================================================
void CBitmapBMP::DrawPalBMP(SSurface *sf, int x, int y, int fr)
 { short xs  = (x < 0)?(0):(x);
  if (xs >= short(sf->xSize))   return;         // Clip: bitmap in the right side of rectangle
  short ys  = sf->ySize - y - ht;
  if (ys >= short(sf->ySize))   return;         // Clip: bitmap below rectangle
  //----Compute start position in bitmap ------------------
  short xb  = (x < 0)?(-x):(0);
  if (xb >= wd)                 return;         // clip: bitmap in the left side of rectangle
  short yb  = (ys < 0)?(-ys):(0);
  if (yb >= ht)                 return;         // clip: bitmap above the rectangle
  if (ys < 0) ys = 0;  
  //----Compute number of line and column ----------------
  short dx1 = (sf->xSize - xs);
  short dx2 = (wd - xb);
  short nc  = (dx1 < dx2)?(dx1):(dx2);
  short dy1 = (sf->ySize - ys);
  short dy2 = (ht - yb);
  short nl  = (dy1 < dy2)?(dy1):(dy2);
  //----Compute buffer correction factor -----------------
  short cs  = (sf->xSize - nc);
  short cb  = (rowSize   - nc);
  //----Compute buffer  starting pointers ----------------
  surf      = sf->drawBuffer  + (ys * sf->xSize) + xs;
  bmap      = base            + (yb * rowSize)   + xb;
  //----Draw to surface ----------------------------------
  nbr       = 0;
  while (nl--) ScanPalBMP(nc,cb,cs);
  return;
 }
//--------------------------------------------------------
//  Scan one line 
//--------------------------------------------------------
void CBitmapBMP::ScanPalBMP (short nc,short cb,short cs)
{ U_INT  pix;
  while (nc--)
  { pix = (*bmap++) << 2;
    GLubyte b = cmap[pix];
    GLubyte g = cmap[pix+1];
    GLubyte r = cmap[pix+2];
    pix       = MakeRGBA (r, g, b, 0xFF);
    if (pix != 0xFF000000)  *surf = pix;
    surf++;
  }
  bmap  += cb;
  surf  += cs;
  return;
}

//=========================================================================
//  Not used
//=========================================================================
void CBitmapBMP::DrawPartial (SSurface *surface, int x, int y,
                              int x1, int y1, int x2, int y2,int fr)
{
}
//=======================================================================
// TIF (Single-frame Windows Bitmap)
//=======================================================================
class CBitmapTIF: public CBaseBitmap {
public:
  // Constructors
  CBitmapTIF (char* Filename = NULL);
 ~CBitmapTIF (void);
  //--------------------------------------------------------------
  void  Load(char *fn);
  //--------------------------------------------------------------
  void DrawBitmap(SSurface *sf,int x,int y, int fr = 0);
  void GetBitmapSize(int *x, int *y) {*x = wd, *y = ht;}
  void DrawPartial(SSurface *sf,int x,int y,int x1,int y1,int x2,int y2,int fr = 0);
  void DrawFrom    (SSurface *sf,int pos,int nbl, int x, int y) {}
  int  NumBitmapFrames() {return 1;}
  //--------------------------------------------------------------
  U_CHAR *GetRGBA()       {return (U_CHAR*)rgba;}
  //--------------------------------------------------------------
  void Load (PODFILE *tif);
  void ScanLine (short nc,short cb,short cs);
  //----ATTRIBUTES -----------------------------------------------
protected:
  int     wd;                             // Width
  int     ht;                             // height
  U_INT  *surf;                           // Surface
  U_INT  *rgba;                           // RGBA array
  U_INT  *bmap;                           // Current pixel
};
//----------------------------------------------------------------
//  Constructor
//----------------------------------------------------------------
CBitmapTIF::CBitmapTIF(char *fn)
{ wd = ht = 0;
  rgba    = 0;
  Load(fn);
}
//----------------------------------------------------------------
//  Load the bitmap
//----------------------------------------------------------------
void CBitmapTIF::Load(char *fn)
{ CTIFFparser img(0);
  //----PATH is ART --------------------------------
  loaded = img.Decode(fn);
  if (0 == loaded)  return;
  img.InvertPIX();
  rgba = img.TransferPIX();
  wd   = img.GetWidth();
  ht   = img.GetHeight();
  return;
}
//----------------------------------------------------------------
//  Destructor
//----------------------------------------------------------------
CBitmapTIF::~CBitmapTIF()
{ if (rgba) delete [] rgba;
}
//----------------------------------------------------------------
//  Draw bitmap
//----------------------------------------------------------------
void CBitmapTIF::DrawBitmap(SSurface *sf,int x,int y, int fr)
{ short xs  = (x < 0)?(0):(x);
  if (xs >= short(sf->xSize))   return;         // Clip: bitmap in the right side of rectangle
  short ys  = sf->ySize - y - ht;
  if (ys >= short(sf->ySize))   return;         // Clip: bitmap above rectangle
  //----Compute start position in bitmap ------------------
  short xb  = (x < 0)?(-x):(0);
  if (xb >= wd)                 return;         // clip: bitmap in the left side of rectangle
  short yb  = (ys < 0)?(-ys):(0);
  if (yb >= ht)                 return;         // clip: bitmap below the rectangle
  if (ys < 0) ys = 0;
  //----Compute number of line and column ----------------
  short dx1 = (sf->xSize - xs);
  short dx2 = (wd - xb);
  short nc  = (dx1 < dx2)?(dx1):(dx2);
  short dy1 = (sf->ySize - ys);
  short dy2 = (ht -yb);
  short nl  = (dy1 < dy2)?(dy1):(dy2);
  //----Compute buffer correction factor -----------------
  short cs  = (sf->xSize - nc);
  short cb  = (wd        - nc);
  //----Compute buffer  starting pointers ----------------
  surf      = sf->drawBuffer  + (ys * sf->xSize) + xs;
  bmap      = rgba            + (yb * wd)        + xb;
  //----Draw to surface ----------------------------------
  while (nl--) ScanLine(nc,cb,cs);
  return;
}
//--------------------------------------------------------
//  Scan one line
//  PURE BLACK IS TRANSPARENT
//--------------------------------------------------------
void CBitmapTIF::ScanLine (short nc,short cb,short cs)
{ U_INT pix;
  while (nc--)
  { pix = *bmap++;
    if (0 == pix) {surf++; continue;}
   *surf++ = pix;
  }
  bmap  += cb;
  surf  += cs;
  return;
}

//----------------------------------------------------------------
//  Draw Partial (not used until now)
//----------------------------------------------------------------
void CBitmapTIF::DrawPartial(SSurface *sf,int x,int y,int x1,int y1,int x2,int y2,int fr)
{
  return;
}
//=======================================================================================
//  GLOBAL FUNCTIONS
// Draw a PBM/PBG bitmap onto the specified surface.  The 'frame' parameter is
//  only relevant for multi-frame PBG bitmaps.  The 'x' and 'y' parameters
//  indicate the location of the top-left corner of the bitmap relative to
//  the top-left corner of the surface
//========================================================================================
void  DrawBitmap(SSurface *sf, SBitmap * bmap, int x, int y, int fr)
{ if (0 == bmap)  return;
  CBaseBitmap *bm = (CBaseBitmap*)bmap->bitmap;
  bm->DrawBitmap(sf,x,y,fr);
  return;
}
//----------------------------------------------------------------
//  Draw partial
//----------------------------------------------------------------
void  DrawBitmapPartial(SSurface *sf, SBitmap * bmap,int x,int y,int x1,int y1,int x2,int y2,int fr)
{ if (0 == bmap)  return;
  CBaseBitmap *bm = (CBaseBitmap*)bmap->bitmap;
  bm->DrawPartial(sf,x,y,x1,y1,x2,y2,fr);
  return;
}
//----------------------------------------------------------------
//  return size
//----------------------------------------------------------------
void  GetBitmapSize(SBitmap * bmap, int *x, int *y)
{ if (0 == bmap)  return;
  CBaseBitmap *bm = (CBaseBitmap*)bmap->bitmap;
  bm->GetBitmapSize(x,y);
  return;
}
//----------------------------------------------------------------
//  return number of frames
//----------------------------------------------------------------
int NumBitmapFrames(SBitmap * bmap )
{ if (0 == bmap)  return 0;
  CBaseBitmap *bm = (CBaseBitmap*)bmap->bitmap;
  return bm->NumBitmapFrames();
}

//----------------------------------------------------------------
// Bitmap primitives
//----------------------------------------------------------------
int Load_Bitmap(SBitmap *bm)
{
  int rc = false;

  bm->type = TYPE_INVALID;
  bm->bitmap = NULL;

  char *p = strrchr (bm->bitmapName, '.');
  if (0 == p)   return false;
  // File has an extension, now check for valid bitmaps
  if (_stricmp (p, ".PBM") == 0)
  {   bm->type = TYPE_PBM;
      bm->bitmap = new CBitmapPBM (bm->bitmapName);
      return true;
  }
  if (_stricmp (p, ".PBG") == 0)
  {   bm->type = TYPE_PBG;
      bm->bitmap = new CBitmapPBG (bm->bitmapName);
      return true;
  }
  if (_stricmp (p, ".BMP") == 0)
  {   bm->type = TYPE_BMP;
      bm->bitmap = new CBitmapBMP (bm->bitmapName);
      return true;
  }
  if (_stricmp (p, ".TIF") == 0)
  {   bm->type = TYPE_TIF;
      bm->bitmap = new CBitmapTIF (bm->bitmapName);
      return true;
  }

  gtfo ("Load_Bitmap : Invalid bitmap name %s", bm->bitmapName);
  return rc;
}
//----------------------------------------------------------------
// Free Bitmap
//----------------------------------------------------------------
void FreeBitmap(SBitmap *bm)
{ if(bm && bm->bitmap)delete (CBaseBitmap*)(bm->bitmap);
  bm->bitmap = NULL;
  bm->type = TYPE_INVALID;
}
//===================================================================================
//  ASSIGN THE NUL BITMAP
//===================================================================================
CBitmap *AssignNullBitmap()
{ globals->nBmap.IncUser();
  return globals->nBitmap;
}
//====================================================================================
// Class CBitmap
//=====================================================================================
CBitmap::CBitmap()
{ AssignNUL();
}
//===================================================================================
//  Create a bitmap: assign NULL type by default
//===================================================================================
CBitmap::CBitmap(char * bmapname)
{ AssignNUL();
  if(bmapname) 
  { strncpy(m_bm.bitmapName, bmapname,63);
    m_bm.bitmapName[63] = 0;
    Load();
  }
}
//--------------------------------------------------------------------------
//  Assign the NUL bitmap
//--------------------------------------------------------------------------
void CBitmap::AssignNUL()
{ m_bm.type   = TYPE_NULL;
  strcpy(m_bm.bitmapName, "NULL");
  CBitmapNUL *bm  = &globals->nBmap;
  m_bm.bitmap     = bm;
  bm->IncUser();
  return;
}
//=================================================================================
// Operator new for CBitmap is a simple memory allocator, and is defined only to
//   ensure compatibility with the overridden delete operator
//=================================================================================
void *CBitmap::operator new (size_t size)
{
  return malloc (sizeof(CBitmap));
}

//==================================================================================
// Operator delete for CBitmap operates differently depending on whether bitmap
//   is of type NULL or not; if not NULL, then the bitmap resources are freed
//   and the memory is deallocated.  If NULL, then the memory is not freed.
//   Due to the way NULL CBitmaps are used, it may be possible for the same 
//   NULL bitmap to be deleted multiple times; this overridden delete operator
//   ensures that the app does not crash if this occurs.  The current implementation
//   results in a memory leak for a NULL bitmap, future implementation may implement
//   a reference counter to clean up the NULL bitmap when the last copy is deleted
//==================================================================================
void CBitmap::operator delete(void *obj)
{ CBitmap *map = (CBitmap*)obj;
  if (map->m_bm.type != TYPE_NULL) free (obj);
  return;
}
//=================================================================================
CBitmap::~CBitmap()
{ if (m_bm.type == TYPE_NULL) return;
  CBaseBitmap *bm = (CBaseBitmap*)m_bm.bitmap;
  if (bm) delete bm;
  m_bm.bitmap = 0;
}
//=================================================================================
//  Change type to allow the NULL bitmap deletion
//=================================================================================
void CBitmap::ChangeType()
{ if (m_bm.type != TYPE_NULL) return;
  m_bm.type   = TYPE_DUMMY;
  m_bm.bitmap = 0;
}
//=================================================================================
bool CBitmap::Load_Bitmap(char * bname)
{	if( bname ) strcpy(m_bm.bitmapName, bname);
	return Load();
}
//---------------------------------------------------------------------------------
//  Load the bitmap
//---------------------------------------------------------------------------------
bool CBitmap::Load( )
{
	bool rc = false;
	if(m_bm.bitmap)		FreeBitmap();

  if(m_bm.bitmapName[0] == 0) return false;
  char *p = strrchr (m_bm.bitmapName, '.');
  if (0 == p)       return false;
  // File has an extension, now check for valid bitmaps
  if (_stricmp (p, ".PBM") == 0)
      { m_bm.type   = TYPE_PBM;
        m_bm.bitmap = new CBitmapPBM (m_bm.bitmapName);
        return true;
      }
  if (stricmp (p, ".PBG") == 0)
      { m_bm.type   = TYPE_PBG;
        m_bm.bitmap = new CBitmapPBG (m_bm.bitmapName);
        return true;
      }
  if (stricmp (p, ".BMP") == 0)
      { m_bm.type   = TYPE_BMP;
        m_bm.bitmap = new CBitmapBMP (m_bm.bitmapName);
        return true;
      }
  if (_stricmp (p, ".TIF") == 0)
      { m_bm.type  = TYPE_TIF;
        m_bm.bitmap = new CBitmapTIF (m_bm.bitmapName);
        return true;
      }

  gtfo ("Load_Bitmap : Invalid bitmap name %s", m_bm.bitmapName);
  return rc;
}
//-----------------------------------------------------------------------
//  Free the bitmap except the nul bitmap
//-----------------------------------------------------------------------
void CBitmap::FreeBitmap( )
{ switch (m_bm.type)
    {
    case TYPE_PBM:
      if(m_bm.bitmap) delete (CBaseBitmap*)(m_bm.bitmap);   ///(CBitmapPBM *)(m_bm.bitmap);
      return;

    case TYPE_PBG:
      if(m_bm.bitmap) delete (CBaseBitmap*)(m_bm.bitmap);
      return;

    case TYPE_BMP:
      if(m_bm.bitmap) delete (CBaseBitmap*)(m_bm.bitmap);
      return;

    case TYPE_NULL:
      { CBitmapNUL *bm = (CBitmapNUL *)(m_bm.bitmap);
        bm->DecUser();
        return;
      }
    default:
      // Invalid bitmap type
      WARNINGLOG ("FreeBitmap : Invalid bitmap type %d", m_bm.type);
  }

  m_bm.bitmap = 0;
  m_bm.type = TYPE_INVALID;
  strcpy (m_bm.bitmapName, "Deleted");
}

//===============================================================================
// Draw a PBM/PBG bitmap onto the specified surface.  The 'frame' parameter is
//  only relevant for multi-frame PBG bitmaps.  The 'x' and 'y' parameters
//  indicate the location of the top-left corner of the bitmap relative to
//  the top-left corner of the surface
//===============================================================================
void  CBitmap::DrawBitmap (SSurface *sf, int x, int y, int fr)
{ CBaseBitmap *bm = (CBaseBitmap*)m_bm.bitmap;
  bm->DrawBitmap(sf,x,y,fr);
  return;
}
//===============================================================================
//  Draw a subimage of the bitmap
//===============================================================================
void  CBitmap::DrawFrom (SSurface *sf,int pos,int nbl, int x, int y)
{ CBaseBitmap *bm = (CBaseBitmap*)m_bm.bitmap;
  bm->DrawFrom(sf,pos,nbl,x,y);
  return;
}
//===============================================================================
// Fill a given rectangle in the surface with the bitmap
//===============================================================================
void CBitmap::FillTheRect(SSurface *sf,int x,int y,int wd,int ht,int fr)
{ CBaseBitmap *bm = (CBaseBitmap*)m_bm.bitmap;
  bm->FillTheRect(sf,x,y,wd,ht,fr);
  return;
}
//=================================================================================
void  CBitmap::DrawPartial (SSurface *sf,int x,int y,int x1,int y1,int x2,int y2,int fr)
{ CBaseBitmap *bm = (CBaseBitmap*)m_bm.bitmap;
  bm->DrawPartial(sf,x,y,x1,y1,x2,y2,fr);
  return;
} 
//==========================================================================
void  CBitmap::GetBitmapSize(int *x, int *y)
{ CBaseBitmap *bm = (CBaseBitmap*)m_bm.bitmap;
  bm->GetBitmapSize(x,y);
  return;
}

//============================================================================
int CBitmap::NumBitmapFrames( )
{ CBaseBitmap *bm = (CBaseBitmap*)m_bm.bitmap;
  return bm->NumBitmapFrames();
}
//===================================================================================
// Helper function to blit a SSurface to the screen, taking into account
//   OpenGL row order and DrawPixels trick
//===================================================================================
void BlitTransparentSurface (SSurface *surface, int xOffset, int yOffset, int y_isiz)
{   glRasterPos2i (xOffset, -yOffset);
    int xMove = -xOffset + surface->xScreen;
    int yMove =  yOffset + y_isiz - surface->yScreen - surface->ySize;
    glBitmap (0, 0, 0, 0, (GLfloat)xMove, (GLfloat)yMove, NULL);
    glDrawBuffer (GL_BACK);
    glDrawPixels (surface->xSize, surface->ySize, GL_RGBA,GL_UNSIGNED_BYTE, surface->drawBuffer);
}

//==================================================================================
// Helper function to blit a SSurface to the screen, taking into account
//   OpenGL row order and DrawPixels trick
//==================================================================================
void BlitOpaqueSurface (SSurface *surface, int xOffset, int yOffset, int y_isiz)
{
  if(surface) {
    glRasterPos2i (xOffset, -yOffset);
    int xMove = -xOffset + surface->xScreen;
    int yMove = yOffset + y_isiz - surface->yScreen - surface->ySize;
    glBitmap (0, 0, 0, 0, (GLfloat)xMove, (GLfloat)yMove, NULL);

    Blit (surface);
  }
}
//==================================================================================
//  Draw texture (txd) on surface sf, staring on line y, column x
//  
//==================================================================================
void DrawTextureOn (TEXT_DEFN &txd,SSurface &sf,int x, int y, int No)
{ if (txd.rgba == 0)  return;
  short wd  = txd.wd;
  short ht  = txd.ht;
  short xs  = (x < 0)?(0):(x);
  if (xs >= short(sf.xSize))    return;         // Clip: bitmap in the right side of surf
  short ys  = sf.ySize - y - ht;
  if (ys >= short(sf.ySize))   return;         // Clip: bitmap below surface
  //----Compute start position in bitmap ------------------
  short xb  = (x < 0)?(-x):(0);
  if (xb >= wd)                 return;         // clip: bitmap in the left side of surf
  short yb  = (ys < 0)?(-ys):(0);
  if (yb >= ht)                 return;         // clip: bitmap above the surface
  if (ys < 0) ys = 0;  
  //----Compute number of line and column ----------------
  short dx1 = (sf.xSize - xs);
  short dx2 = (wd - xb);
  short nc  = (dx1 < dx2)?(dx1):(dx2);
  short dy1 = (sf.ySize - ys);
  short dy2 = (ht - yb);
  short nl  = (dy1 < dy2)?(dy1):(dy2);
  //----Compute buffer correction factor -----------------
  short cd  = (sf.xSize - nc);    // To complete line in surface
  short cs  = (wd   -     nc);    // To get next line in texture
  //----Compute buffer  starting pointers ----------------
  int    nf   = txd.nf - No - 1;
  U_INT *dst  = sf.drawBuffer  + (ys * sf.xSize) + xs;
  U_INT *src  = (U_INT*)txd.rgba + (nf * txd.dm);           
         src += (yb * wd)   + xb;
  while (nl--)                      // For each line
  { int k = nc;
    while (k--) *dst++ = *src++;
    src  += cs;
    dst  += cd;
  }
  return;
}
//=====================END OF FILE ===================================================
