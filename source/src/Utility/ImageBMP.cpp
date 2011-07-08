/*
 * ImageBMP.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2006 Chris Wallace
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

/*! \file ImageBMP.cpp
 *  \brief Implements CImageBMP class, a rudimentary Windows BMP image file loader
 *
 * Implements the CImageBMP class.
 */

#include "../Include/FlyLegacy.h"
#include "../Include/Utility.h"
#include "../Include/Globals.h"
#include "../Include/Endian.h"

CImageBMP::CImageBMP (const char* bmpFilename)
{
  strcpy (filename, bmpFilename);

  image = NULL;
  PODFILE* p = popen (&globals->pfs, filename);
  if (p != NULL) {
    // Read header
    pread (&h, sizeof(SBmpHeader), 1, p);

    // Read info header
    pread (&ih, sizeof(SBmpInfoHeader), 1, p);

    // Read colormap data
    if (ih.nPlanes == 3) {
      int cmapSize = ih.nColours * 3;
      cmap = new unsigned char[cmapSize];
      pread (cmap, cmapSize, 1, p);
    }

    // Read image data
    switch (ih.compression) {
    case 0:
      // Uncompressed
      image = new unsigned char[ih.imageSize];
      pread (image, ih.imageSize, 1, p);
      break;

    default:
      // Only uncompressed images are currently supported
      WARNINGLOG ("CImageBMP : Unsupported compression mode %d in %s",
        ih.compression, filename);
    }
    pclose (p);
  }
}


CImageBMP::~CImageBMP (void)
{
}

unsigned long CImageBMP::GetWidth (void)
{
  return ih.w;
}

unsigned long CImageBMP::GetHeight (void)
{
  return ih.h;
}

void CImageBMP::DrawToSurface (SSurface* surface, int x, int y)
{
  // Check that valid image data exists
  if (image == NULL) {
    WARNINGLOG ("CImageBMP::DrawToSurface : NULL image data for %s", filename);
    return;
  }

  int sybase = surface->ySize - y - ih.h;

  // Copy pixel by pixel
  unsigned long i, j;
  unsigned long imageOffset, sx, sy;
  for (i=0; i<ih.h; i++) {
    for (j=0; j<ih.w; j++) {
      switch (ih.nPlanes) {
      case 1:
        // 24-bit RGB data
        {
          imageOffset = ((i * ih.w) + j) * 3;
          GLubyte r = image[imageOffset];
          GLubyte g = image[imageOffset+1];
          GLubyte b = image[imageOffset+2];
          GLubyte a = 0xFF;
          unsigned int rgba = MakeRGBA (r, g, b, a);

          sx = x+j;
          sy = sybase+i;
          if ((sx >= 0) && (sx < (int)surface->xSize) &&
              (sy >= 0) && (sy < (int)surface->ySize))
          {
            unsigned long surfOffset = (sy * surface->xSize) + sx;
            surface->drawBuffer[surfOffset] = rgba;
          }
        }
        break;

      case 3:
        // Pallettized data
        {
          imageOffset = (i * ih.w) + j;
          GLubyte cmapIndex = image[imageOffset];
          GLubyte b = cmap[cmapIndex];
          GLubyte g = cmap[cmapIndex+1];
          GLubyte r = cmap[cmapIndex+2];
          GLubyte a = 0xFF;
          unsigned int rgba = MakeRGBA (r, g, b, a);

          sx = x+j;
          sy = sybase+i;
          if ((sx >= 0) && (sx < (int)surface->xSize) &&
              (sy >= 0) && (sy < (int)surface->ySize))
          {
            unsigned long surfOffset = (sy * surface->xSize) + sx;
            surface->drawBuffer[surfOffset] = rgba;
          }
        }
        break;
      }
    }
  }
}

void CImageBMP::DrawTransparentToSurface (SSurface* s, int x, int y, unsigned int rgb)
{
}
