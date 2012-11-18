/*
 * ImageDXT.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2007 Chris Wallace
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

/*! \file ImageDXT.cpp
 *  \brief Implements CImageDXT and child classes to support DXT compressed texture files
 *
 */

#include "../Include/Utility.h"
#include "../Include/Globals.h"


//
// To avoid having to include DirectX headers in the FlyLegacy development environment,
//   the DDSURFACEDESC2 struct definition is copied here from ddraw.h
//
#ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif //defined(MAKEFOURCC)

/*
 * FOURCC codes for DX compressed-texture pixel formats
 */
#define FOURCC_DXT1  (MAKEFOURCC('D','X','T','1'))
#define FOURCC_DXT2  (MAKEFOURCC('D','X','T','2'))
#define FOURCC_DXT3  (MAKEFOURCC('D','X','T','3'))
#define FOURCC_DXT4  (MAKEFOURCC('D','X','T','4'))
#define FOURCC_DXT5  (MAKEFOURCC('D','X','T','5')) 

typedef struct {
  DWORD dw1;
  DWORD dw2;
} DDCOLORKEY;

typedef struct _DDPIXELFORMAT {
  DWORD  dwSize;
  DWORD  dwFlags;
  DWORD  dwFourCC;
union {
  DWORD  dwRGBBitCount;
  DWORD  dwYUVBitCount;
  DWORD  dwZBufferBitDepth;
  DWORD  dwAlphaBitDepth;
  DWORD  dwLuminanceBitCount;
  DWORD  dwBumpBitCount;
  DWORD  dwPrivateFormatBitCount;
} ;
union {
  DWORD  dwRBitMask;
  DWORD  dwYBitMask;
  DWORD  dwStencilBitDepth;
  DWORD  dwLuminanceBitMask;
  DWORD  dwBumpDuBitMask;
  DWORD  dwOperations;
} ;
union {
  DWORD  dwGBitMask;
  DWORD  dwUBitMask;
  DWORD  dwZBitMask;
  DWORD  dwBumpDvBitMask;
  struct {
    WORD wFlipMSTypes;
    WORD wBltMSTypes;
  } MultiSampleCaps;
} ;
union {
  DWORD  dwBBitMask;
  DWORD  dwVBitMask;
  DWORD  dwStencilBitMask;
  DWORD  dwBumpLuminanceBitMask;
} ;
union {
  DWORD  dwRGBAlphaBitMask;
  DWORD  dwYUVAlphaBitMask;
  DWORD  dwLuminanceAlphaBitMask;
  DWORD  dwRGBZBitMask;
  DWORD  dwYUVZBitMask;
} ;
} DDPIXELFORMAT;

typedef struct _DDSCAPS2 {
  DWORD  dwCaps;
  DWORD  dwCaps2;
  DWORD  dwCaps3;
  DWORD  dwCaps4;
} DDSCAPS2, FAR* LPDDSCAPS2;

typedef struct _DDSURFACEDESC2 {
  DWORD  dwSize;
  DWORD  dwFlags;
  DWORD  dwHeight;
  DWORD  dwWidth; 
  union {
    LONG  lPitch;
    DWORD  dwLinearSize; 
  };
  DWORD  dwBackBufferCount; 
  union {
    DWORD  dwMipMapCount; 
    DWORD  dwRefreshRate;  
    DWORD  dwSrcVBHandle; 
  } ;
  DWORD  dwAlphaBitDepth;  
  DWORD  dwReserved; 
  LPVOID  lpSurface; 
  union {
    DDCOLORKEY  ddckCKDestOverlay;   
    DWORD  dwEmptyFaceColor;    
  };
  DDCOLORKEY  ddckCKDestBlt;     
  DDCOLORKEY  ddckCKSrcOverlay;    
  DDCOLORKEY  ddckCKSrcBlt;     
  union {
  DDPIXELFORMAT  ddpfPixelFormat;    
  DWORD  dwFVF;  
  } ;
  DDSCAPS2  ddsCaps;  
  DWORD  dwTextureStage;   
} DDSURFACEDESC2;

void CImageDXT::LoadDDS (const char* ddsFilename)
{
  // Open POD file
  PODFILE *p = popen (&globals->pfs, ddsFilename);
  if (p == NULL) {
    WARNINGLOG ("CImageDXT : Failed to open file %s", ddsFilename);
    return;
  }

  // Read magic code indicating this is a DDS file
  char  filecode[4];
  pread (filecode, 1, 4, p);
  if (strncmp (filecode, "DDS ", 4) != 0) {
    WARNINGLOG ("CImageDXT : Invalid file code for %s", ddsFilename);
    pclose (p);
    return;
  }

  // Read surface descriptor and set relevant class data members
  UINT8 factor = 0;
  DDSURFACEDESC2 ddsd;
  pread (&ddsd, sizeof(ddsd), 1, p);
  mipCount = ddsd.dwMipMapCount;
  width = ddsd.dwWidth;
  height = ddsd.dwHeight;
  switch (ddsd.ddpfPixelFormat.dwFourCC) {
    case FOURCC_DXT1:
      if (ddsd.dwAlphaBitDepth > 0) {
        format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
      } else {
        format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
      }
      factor = 2;
      break;
    case FOURCC_DXT3:
      format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
      factor = 4;
      break;
    case FOURCC_DXT5:
      format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
      factor = 4;
      break;
    default:
      WARNINGLOG ("LoadDXT : Unsupported pixel format 0x%04X", ddsd.ddpfPixelFormat.dwFourCC);
  }

  // Calculate size of the required image buffer including mipmaps
  UINT32 bufferSize;
  if (mipCount > 1) {
    bufferSize = ddsd.dwLinearSize * factor;
  } else {
    bufferSize = ddsd.dwLinearSize;
  }
  image = new GLubyte[bufferSize];
  pread (image, 1, bufferSize, p);
  pclose (p);
}

CImageDXT::CImageDXT (const char* ddsFilename)
{
  image = NULL;
  LoadDDS (ddsFilename);
}

CImageDXT::~CImageDXT (void)
{
  SAFE_DELETE (image);
}

void CImageDXT::BindTexture (GLuint texid)
{
  // Check for appropriate GL extension support
  bool supported = false;
  int nBlockSize = 0;
  switch (format) {
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
      if (globals->dxt1Supported) {
        supported = true;
        nBlockSize = 8;
      }
      break;
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
      if (globals->dxt1aSupported) {
        supported = true;
        nBlockSize = 8;
      }
      break;
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
      if (globals->dxt3Supported) {
        supported = true;
        nBlockSize = 16;
      }
      break;
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      if (globals->dxt5Supported) {
        supported = true;
        nBlockSize = 16;
      }
      break;
  }

  // If the appropriate driver support is available, generate the texture
  if (supported) {
    glBindTexture (GL_TEXTURE_2D, texid);
    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (mipCount > 1) {
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    } else {
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    UINT32 nWidth = width;
    UINT32 nHeight = height;
    UINT32 nOffset = 0;
    for (int i=0; i<mipCount; i++) {
      if (nWidth == 0) nWidth = 1;
      if (nHeight == 0) nHeight = 1;
      UINT32 nSize = ((nWidth+3)/4) * ((nHeight+3)/4) * nBlockSize;
      glCompressedTexImage2DARB (GL_TEXTURE_2D, i, format, width, height, 0,
        nSize, image + nOffset);
      CHECK_OPENGL_ERROR
      nOffset += nSize;

      // Halve the width/height for the next mipmap level
      nWidth /=2;
      nHeight /= 2;
    }
  } else {
    WARNINGLOG ("DXT format 0x%04X not supported by video card.", format);
  }
}

UINT32 CImageDXT::GetTextureMemorySize (void)
{
  UINT32 rc = 0;

  UINT32 nBlockSize = 0;
  switch (format) {
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
      nBlockSize = 8;
      break;
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      nBlockSize = 16;
      break;
  }

  // If the appropriate driver support is available, generate the texture
  UINT32 nWidth = width;
  UINT32 nHeight = height;
  for (int i=0; i<mipCount; i++) {
    if (nWidth == 0) nWidth = 1;
    if (nHeight == 0) nHeight = 1;
    UINT32 nSize = ((nWidth+3)/4) * ((nHeight+3)/4) * nBlockSize;
    rc += nSize;

    // Halve the width/height for the next mipmap level
    nWidth /=2;
    nHeight /= 2;
  }
  return rc;
}
//=== END OF FILE =============================================================================