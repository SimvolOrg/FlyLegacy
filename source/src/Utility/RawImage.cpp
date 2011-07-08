/*
 * RawImage.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2007 Chris Wallace
 * Copyright 2007-2009 Jean Sabatier
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

/*! \file RawImage.cpp
 *  \brief Implements CImageRaw loader for RAW format images
 */

#include "../Include/Globals.h"
#include "../Include/Utility.h"
#include "../Include/TerrainTexture.h"
//
// Define the following macro to have all RAW images flipped left-right
//   when loaded.
//
// #define RAW_FLIP_LEFT_RIGHT
// #define MASK_FLIP_LEFT_RIGHT
//=================================================================================
//  2D Rectangle
//=================================================================================
C_2DRECT::C_2DRECT()
{ x0  = 0;
  y0  = 0;
  x1  = 0;
  y1  = 0;
}
//-----------------------------------------------------------------------------------
//  Set Rectangle
//-----------------------------------------------------------------------------------
void C_2DRECT::Init(int x0,int y0,int x1,int y1)
{ this->x0 = x0;
  this->y0 = y0;
  this->x1 = x1 + x0;
  this->y1 = y1 + y0;
  return;
}
//-----------------------------------------------------------------------------------
//  Check for Hit
//-----------------------------------------------------------------------------------
bool C_2DRECT::IsHit(int x,int y)
{ if ((x < x0) || (x > x1)) return false;
  if ((y < y0) || (y > y1)) return false;
  return true;
}
//=================================================================================
/*
 * CMaskImage
 *
 * A mask image is a special type of RAW file used to represent a bitmask for
 *   overlaying one texture on top of another.  There is no need for either the
 *   ACT or OPA files; the RAW image data represents an 8-bit grayscale mask.
 *
 */
//=================================================================================
//
// Constructor for creating a "blank" mask image
//
CMaskImage::CMaskImage (int width, int height)
{
  this->width   = width;
  this->height  = height;
  dim = width * height;
  this->rawdata = new GLubyte[dim];
  memset (this->rawdata, 0, dim);
}

//
// This constructor accepts a predefined buffer containing the mask data
//
CMaskImage::CMaskImage (int width, int height, const GLubyte *data)
{ this->width = width;
  this->height = height;
  dim = width * height;
  this->rawdata = new GLubyte[dim];
  memcpy (this->rawdata, data, dim);
}

//============================================================================
// This constructor accepts a RAW filename from which the mask data is loaded
//===========================================================================
CMaskImage::CMaskImage (int width, int height, const char* rawFilename)
{ rawdata = 0;
  // Initialize data members
  this->width = width;
  this->height = height;
  dim = width * height;
  // Open RAW pod file
  PODFILE *pRaw = popen (&globals->pfs, rawFilename);
  if (!pRaw) {
    WARNINGLOG ("CMaskImage : Cannot open file %s", rawFilename);
    return;
  }

  // Load image data
  rawdata = new GLubyte[dim];
  pread (rawdata, (dim), 1, pRaw);
  Invert();
  pclose (pRaw);
}
//------------------------------------------------------------------------
CMaskImage::CMaskImage (const CMaskImage &src)
{
  width = src.width;
  height = src.height;
  dim = width * height;
  // Copy data
  rawdata = new GLubyte[dim];
  memcpy (rawdata, src.rawdata, dim);
}
//------------------------------------------------------------------------
//  Validate the mask
//------------------------------------------------------------------------
CMaskImage *CMaskImage::Validate()
{ if (rawdata)    return this;
  //----------------------------------
  delete this;
  return 0;
}
//------------------------------------------------------------------------
//  Invert the mask values.
//  For lighting gauge, the mask is used as a luminance texture
//------------------------------------------------------------------------
void CMaskImage::Invert()
{ GLubyte *buf = new GLubyte[dim];
  GLubyte *src = rawdata;
  GLubyte *dst = buf;
  for (int k=0; k<dim; k++) *dst++ = (GLubyte)(0xFF)  - *src++;
  delete [] rawdata;
  rawdata = buf;
  return;
}
//------------------------------------------------------------------------
//  Destroy the mask
//------------------------------------------------------------------------
CMaskImage::~CMaskImage (void)
{ if (rawdata) delete rawdata; }
//========================================================================
//  CTexture
//  Support for RGBA array of pixel that may be used for any purpose
//========================================================================
//  Create the texture
//------------------------------------------------------------------------
CTexture::CTexture()
{ rect.x1 = rect.y1 = 0;
  rgba = 0;
}
//------------------------------------------------------------------------
//  Load texture from ART directory (non pod file)
//  tsp is the transparent option. When  set, all pure black pixels
//      are made transparent via the alpha chanel
//------------------------------------------------------------------------
void CTexture::LoadFromDisk(char *name,char tsp,int xs,int ys)
{ CArtParser img(0);
  TEXT_INFO xds;
  if (rgba) delete [] rgba;
  rgba = 0;
  sprintf(xds.path,"ART/%s",name);
	xds.azp		= tsp;
  rgba = img.GetAnyTexture(xds);
  rect.x0   = xs;
  rect.y0   = ys;
  rect.x1   = img.GetWidth();
  rect.y1   = img.GetHeigth();
  SetQUAD();
  return;
}
//------------------------------------------------------------------------
//  Load from Terrain Raw file
//------------------------------------------------------------------------
void CTexture::LoadTerrain(char *name)
{
  return;
}
//------------------------------------------------------------------------
//    Set QUAD to texture size
//------------------------------------------------------------------------
void CTexture::SetQUAD()
{ float x0 = float(rect.x0);
  float y0 = float(rect.y0);
  float wd = float(rect.x1);
  float ht = float(rect.y1);
  //----SW base corner ------------------------------------------------
  Pan[0].VT_S   = 0;
  Pan[0].VT_T   = 0;
  Pan[0].VT_X   = x0;
  Pan[0].VT_Y   = y0;
  Pan[0].VT_Z   = 0;
  //-----SE corner ----------------------------------------------------
  Pan[1].VT_S   = 1;
  Pan[1].VT_T   = 0;
  Pan[1].VT_X   = x0 + wd;
  Pan[1].VT_Y   = y0;
  Pan[1].VT_Z   = 0;
  //-----NE corner ----------------------------------------------------
  Pan[2].VT_S   = 1;
  Pan[2].VT_T   = 1;
  Pan[2].VT_X   = x0 + wd;
  Pan[2].VT_Y   = y0 + ht;
  Pan[2].VT_Z   = 0;
  //-----NW corner ----------------------------------------------------
  Pan[3].VT_S   = 0;
  Pan[3].VT_T   = 1;
  Pan[3].VT_X   = x0;
  Pan[3].VT_Y   = y0 + ht;
  Pan[3].VT_Z   = 0;
  return;
}
//------------------------------------------------------------------------
//  Draw textured QUAD
//------------------------------------------------------------------------
void CTexture::DrawQUAD(int sw, int sh)
{ int vp[4]; 
  int x0 = rect.x0;
  int y0 = rect.y0;
  int wd = rect.x1;
  int ht = rect.y1;
  SetQUAD();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D (0, wd, 0, ht);
  // Initialize modelview matrix to ensure panel drawing is not affected by
  //   any junk left at the top of the stack by previous rendering
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  glLoadIdentity ();
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  //---Save viewport -----------------------------------------------
  glGetIntegerv(GL_VIEWPORT,vp);
  glViewport(x0,y0,sw,sh);
  xOBJ = globals->txw->GetTexOBJ(xOBJ,wd,ht,rgba,GL_BGRA);
  glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
  glEnable  (GL_TEXTURE_2D);
  glDisable (GL_DEPTH_TEST);
  glBindTexture(GL_TEXTURE_2D,xOBJ);
  //----Draw the panel as a textured square  -----------------------
  glEnable(GL_BLEND);
  glPolygonMode(GL_FRONT,GL_FILL);
  glInterleavedArrays(GL_T2F_V3F,0,Pan);
  glDrawArrays(GL_QUADS,0,4);
  //----------------------------------------------------------------
  glViewport(vp[0],vp[1],vp[2],vp[3]);
  glPopClientAttrib ();
  glPopAttrib();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
	/*
    // Check for an OpenGL error
    { GLenum e = glGetError ();
  if (e != GL_NO_ERROR) 
    WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  }
 */
  return;
}
//------------------------------------------------------------------------
//  Draw texture
//------------------------------------------------------------------------
void CTexture::Draw()
{ xOBJ = globals->txw->GetTexOBJ(xOBJ,rect.x1,rect.y1,rgba,GL_BGRA);
  glBindTexture(GL_TEXTURE_2D,xOBJ);
  glPolygonMode(GL_FRONT,GL_FILL);
  glInterleavedArrays(GL_T2F_V3F,0,Pan);
  glDrawArrays(GL_QUADS,0,4);
  return;
}
//------------------------------------------------------------------------
//  Set Texture parameters
//------------------------------------------------------------------------
void CTexture::SetDimension(C_2DRECT &r)
{ rect.x0 = r.x0;
  rect.y0 = r.y0;
  return;
}
//========================================================================
// CRawImage
//=======================================================================
CRawImage::CRawImage (void)
{
  Init ();
}

CRawImage::CRawImage (const char* rawFilename,
                      const char* actFilename,
                      const char* opaFilename)
{
  Init ();

  // Open RAW file and get the file size
  PODFILE *pRaw = popen (&globals->pfs, rawFilename);
  if (!pRaw) {
    gtfo ("CRawImage : Cannot open file %s", rawFilename);
  }
  int rawSize = pRaw->size;
  pclose (pRaw);

  // Calculate side length and verify that the image is square
  int size = (int)(sqrt((double)rawSize));
  if ((size * size) != rawSize) {
    gtfo ("CRawImage : Image %s is not square; cannot auto-determine size", rawFilename);
    return;
  }

  // Initialize data members
  strcpy (name, rawFilename);
  width = height = size;

  // Load image data
  Load (rawFilename, actFilename, opaFilename);
}


CRawImage::CRawImage (int width,
                      int height,
                      const char* rawFilename,
                      const char* actFilename,
                      const char* opaFilename)
{
  Init ();

  // Initialize data members
  strcpy (name, rawFilename);
  this->width = width;
  this->height = height;
  
  // Load image data
  Load (rawFilename, actFilename, opaFilename);
}


void CRawImage::Init (void)
{
  actdata   = rawdata = opadata = NULL;
  width     = height = 0;
  rawLoaded = false;
  opaLoaded = false;
}

void CRawImage::Allocate (UINT32 w, UINT32 h)
{
  if (actdata == NULL) actdata = new GLubyte[0x300];
  if (rawdata == NULL) rawdata = new GLubyte[w * h];
  if (opadata == NULL) opadata = new GLubyte[w * h];
  rawLoaded = false;
  opaLoaded = false;
  width     = w;
  height    = h;
}

//
// Load the image data from the specified pod files.
//
void CRawImage::Load (const char *rawFilename,
                      const char *actFilename,
                      const char *opaFilename)
{
  // Open RAW file from default POD filesystem
  PODFILE *raw = popen (&globals->pfs, rawFilename);

  // Open ACT pod file
  PODFILE *act = NULL;
  if (actFilename == NULL) {
    // Derive ACT filename from RAW file
    char actName[PATH_MAX];
    strcpy (actName, rawFilename);
    char *p = strrchr (actName, '.');
    if (p) {
      strcpy (p, ".ACT");
      act = popen (&globals->pfs, actName);
    } else {
      WARNINGLOG ("CRawImage : Could not derive ACT filename for %s", rawFilename);
    }
  } else {
    // ACT filename was supplied
    act = popen (&globals->pfs, actFilename);
  }
  if (act == NULL) {
    WARNINGLOG ("CRawImage : Cannot open file %s", actFilename);
    pclose (raw);
    return;
  }

  // Open OPA data file
  PODFILE *opa = NULL;
  if (opaFilename == NULL) {
    // Derive OPA filename from RAW file
    char opaName[PATH_MAX];
    strcpy (opaName, rawFilename);
    char *p = strrchr (opaName, '.');
    if (p) {
      strcpy (p, ".OPA");
      opa = popen (&globals->pfs, opaName);
    }
  } else {
    // OPA filename was supplied
    opa = popen (&globals->pfs, opaFilename);
    if (!opa) {
      WARNINGLOG ("CRawImage : Cannot open supplied file %s", opaFilename);
    }
  }

  // Load image from open pod files
  Load (raw, act, opa);

  pclose (raw);
  pclose (act);
  if (opa != NULL) pclose (opa);
}

void CRawImage::Load (PODFILE *raw, PODFILE *act, PODFILE *opa)
{
  // Initialize data buffer pointers
  actdata = NULL;
  rawdata = NULL;
  opadata = NULL;

  if ((width == 0) || (height == 0)) {
    // Attempt to guess at image size assuming it is square
    int root = (int)(sqrt((float)raw->size));
    if ((root * root) == raw->size) {
      // Set width and height to the assumed size
      width = height = root;
    } else {
      // Could not guess image size
      WARNINGLOG ("CRawImage::Load cannot determine size for %s", raw->filename);
      return;
    }
  }

  // Validate that at least RAW and ACT are valid
  if (raw == NULL) {
    WARNINGLOG ("CRawImage::Load called with NULL RAW podfile");
    return;
  }
  if (act == NULL) {
    WARNINGLOG ("CRawImage::Load called with NULL ACT podfile");
    return;
  }

  // Ensure memory is allocated
  Allocate (width, height);

  // Load colour map from ACT file
  pread (actdata, 0x300, 1, act);

  // Load image data from RAW file
  pread (rawdata, (width * height), 1, raw);
  rawLoaded = true;

  // Open OPA data file if the filename has been specified
  if (opa != NULL) {
    pread (opadata, (width * height), 1, opa);
    opaLoaded = true;
  }
}

CRawImage::CRawImage (const CRawImage &src)
{
  Init ();
  Copy (src);
}

CRawImage::~CRawImage (void)
{
  SAFE_DELETE (actdata);
  SAFE_DELETE (rawdata);
  SAFE_DELETE (opadata);
}

int CRawImage::GetWidth (void)
{
  return width;
}

int CRawImage::GetHeight (void)
{
  return height;
}

void CRawImage::SetName (const char* name)
{
  strcpy (this->name, name);
}

void CRawImage::Copy (const CRawImage &src)
{
  width = src.width;
  height = src.height;
  strcpy (name, src.name);
  actdata = NULL;
  rawdata = NULL;
  opadata = NULL;

  // Copy all data
  if (src.actdata != NULL) {
    actdata = new GLubyte[0x300];
    memcpy (actdata, src.actdata, 0x300);
  }

  if (src.rawdata != NULL) {
    rawdata = new GLubyte[width * height];
    memcpy (rawdata, src.rawdata, (width * height));
  }

  if (src.opadata != NULL) {
    opadata = new GLubyte[width * height];
    memcpy (opadata, src.opadata, (width * height));
  }
}


void CRawImage::Merge (const CRawImage *raw, const CMaskImage *mask, unsigned char actOffset)
{
  if ((raw != NULL) && (mask != NULL)) {
    // Copy palette
    memcpy (&actdata[actOffset * 3], &raw->actdata[0], 0x40 * 3);

    // Overwrite current image data when corresponding mask bit is non-zero
    int offset = 0;
    for (int i=0; i<height; i++) {
      for (int j=0; j<width; j++, offset++) {
        if (mask->rawdata[offset] != 0) {
          this->rawdata[offset] = raw->rawdata[offset] + actOffset;
        }
      }
    }
  }
}


void CRawImage::CopyMasked (const CRawImage *src, const CMaskImage *mask)
{
  /// \todo Check for mismatched sizes
/*
  for (int i=0; i<height; i++) {
    for (int j=0; j<width; j++) {
      // Calculate offsets
      int image_offset = ((i * width) + j) * depth;
      int mask_offset = (i * width) + j;

      float weight = (float)(mask->image[mask_offset] / 255);
      for (int k=0; k<3; k++) {
        char this_c = image[image_offset + k];
        char src_c = src->image[image_offset + k];
        char new_c = (char)((weight * (float)src_c) +
                          ((1.0 - weight) * (float)this_c));
        image[image_offset + k] = new_c;
      }
    }
  }
*/
}
//----------------------------------------------------------------
//  Make a RBG image from the raw color indexed file
//----------------------------------------------------------------
GLubyte *CRawImage::GetRGBImageData (void)
{
  int depth = 3;
  GLubyte *image = new GLubyte [width * height * depth];

  for (int i=0; i<height; i++) {
    for (int j=0; j<width; j++) {

      long iSrc = (i * width) + j;
      long iImg = ((i * width) + j) * depth;

      // Read colormap index from the raw image data
      GLubyte iCmap = rawdata[iSrc];

      image[iImg + 0] = actdata[iCmap * 3 + 0];
      image[iImg + 1] = actdata[iCmap * 3 + 1];
      image[iImg + 2] = actdata[iCmap * 3 + 2];
    }
  }

  return image;
}
//----------------------------------------------------------------
//  Make a RBGA image from the raw color indexed file
//----------------------------------------------------------------
GLubyte *CRawImage::GetRGBAImageData (void)
{
  int depth = 4;
  GLubyte *image = new GLubyte [width * height * depth];

  for (int i=0; i<height; i++) {
    for (int j=0; j<width; j++) {

      long iSrc = (i * width) + j;
      long iImg = ((i * width) + j) * depth;

      // Read colormap index from the raw image data
      GLubyte iCmap = rawdata[iSrc];

      image[iImg + 0] = actdata[iCmap * 3 + 0];
      image[iImg + 1] = actdata[iCmap * 3 + 1];
      image[iImg + 2] = actdata[iCmap * 3 + 2];

      // Read opacity from OPA file, or default to 0xFF (opaque)
      GLubyte opa = (GLubyte)0xFF;
      if (opadata != NULL) {
        opa = opadata [(i * width) + j];
      }
      image[iImg + 3] = opa;
    }
  }

  return image;
}

void CRawImage::BindTexture (GLuint texid, bool mipmap)
{
  // Create a GL texture
  glBindTexture (GL_TEXTURE_2D, texid);
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // Default is RBG (non-alpha) texture
  GLenum format = GL_RGB;
  int depth = 3;
  GLubyte *image = NULL;
  if (opaLoaded) {
    // Alpha channel exists
    format = GL_RGBA;
    depth = 4;
    image = GetRGBAImageData ();
  } else {
    format = GL_RGB;
    depth = 3;
    image = GetRGBImageData ();
  }

  if (mipmap) {
    // Generate mipmapped texture
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    gluBuild2DMipmaps (GL_TEXTURE_2D, depth, width, height, format,
      GL_UNSIGNED_BYTE, image);
  } else {
    // Generate non-mipmapped texture
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D (GL_TEXTURE_2D, 0, format, width, height, 0,
      format, GL_UNSIGNED_BYTE, image);
  }

  // Free image data
  SAFE_DELETE_ARRAY (image);
}
/*
void CRawImage::BindTexture (GLuint texid, bool mipmap)
{
  CreateTransferMaps ();

  glPushAttrib (GL_PIXEL_MODE_BIT);

  // Create a GL texture
  glBindTexture (GL_TEXTURE_2D, texid);
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

  GLenum format = GL_COLOR_INDEX;
  GLint depth = 3;

  // Set up pixel transfer functions for RGB colours
  glPixelMapfv (GL_PIXEL_MAP_I_TO_R, 0x100, xferRed);
  glPixelMapfv (GL_PIXEL_MAP_I_TO_G, 0x100, xferGreen);
  glPixelMapfv (GL_PIXEL_MAP_I_TO_B, 0x100, xferBlue);
  if (xferAlpha != NULL) {
    glPixelMapfv (GL_PIXEL_MAP_I_TO_A, 0x100, xferAlpha);
    depth = 4;
  }
  glPixelTransferi (GL_MAP_COLOR, true);

  if (mipmap) {
    // Generate mipmapped texture
    gluBuild2DMipmaps (GL_TEXTURE_2D, depth, width, height, format, GL_UNSIGNED_BYTE, rawdata);
  } else {
    // Generate non-mipmapped texture
    glTexImage2D (GL_TEXTURE_2D, 0, depth, width, height, 0, format, GL_UNSIGNED_BYTE, rawdata);
  }

  glPopAttrib ();

  CHECK_OPENGL_ERROR
}
*/
UINT32 CRawImage::GetTextureMemorySize()
{
  UINT32 rc = 0;
  if (opaLoaded) {
    rc = width * height * 4;
  } else {
    rc = width * height * 3;
  }
  return rc;
}

//
// CSunRawImage
//
// RAW textures used to display the sun are special in that the transparency
//   is implied by the RAW image itself.  That is, there are no OPA files yet
//   the texture is supposed to be transparent.
//
// Try using the pixel colour index as the transparency value
//
CSunRawImage::CSunRawImage (int width,
              int height,
              const char* rawFilename,
              const char* actFilename)
              : CRawImage (width, height, rawFilename, actFilename)
{
/*
  // Initialize data members
  strcpy (name, rawFilename);
  this->width = width;
  this->height = height;
  
  actdata = NULL;
  rawdata = NULL;
  opadata = NULL;

  // Open ACT pod file
  PODFILE *pAct = popen (&globals->pfs, actFilename);
  if (!pAct) {
    gtfo ("CSunRawImage : Cannot open file %s", actFilename);
    return;
  }

  // Load colour map from ACT file
  actdata = new GLubyte[0x300];
  pread (actdata, 0x300, 1, pAct);
  pclose (pAct);

  // Open RAW pod file
  PODFILE *pRaw = popen (&globals->pfs, rawFilename);
  if (!pRaw) {
    WARNINGLOG ("CRawImage : Cannot open file %s", rawFilename);
    return;
  }

  // Load image data
  rawdata = new GLubyte[width * height];
  pread (rawdata, (width * height), 1, pRaw);
  pclose (pRaw);
*/
  opadata = new GLubyte [width * height];
  memcpy (opadata, rawdata, (width * height));
}

