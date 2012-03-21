/*
 * TextureManager.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005-2007 Chris Wallace
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

/*! \file TextureManager.cpp
 *  \brief Implements CTextureManager
 *
 * The CTextureManager class provides centralized management and control of
 *   OpenGL texture resources for the entire application.  The manager maintains
 *   dynamically-sized pools of texture resources.  Methods to bind various types
 *   of bitmap image data to a texture are provided.
 */

#include "../Include/Utility.h"
using namespace std;

//
// CTextureInfo encapsulates all relevant data about a texture cached by the
//   texture manager.
//
typedef enum {
  TEXTURE_TYPE_UNKNOWN = 0,
  TEXTURE_TYPE_TIF,
  TEXTURE_TYPE_RAW,
  TEXTURE_TYPE_DXT
} ETextureType;

class CTextureInfo {
public:
  CTextureInfo ();

public:
  ETextureType  type;           ///< File type for texture source
  GLuint        texid;          ///< OpenGL Texture ID
  UINT32        width, height;  ///< Width and height in texels
  UINT32        texMemorySize;  ///< Estimated texture memory in octets
};

CTextureInfo::CTextureInfo()
{
  type = TEXTURE_TYPE_UNKNOWN;
  texid = 0;
  width = height = 0;
  texMemorySize = 0;
}


// Declare static class members
CTextureManager         CTextureManager::instance;

CTextureManager::CTextureManager (void)
{
  // Initialize data members
  log = NULL;
  nFindSuccess = nFindFailure = nBind = 0;
  for (int i=0; i<TEXTURE_SIZE_SENTINEL; i++) {
    tifCount[i] = rawCount[i] = dxtCount[i] = 0;
  }
}

void CTextureManager::Init (void)
{
 }

void CTextureManager::Cleanup (void)
{
  // TEMP: Dump to log file
  FILE *f = fopen ("Debug/TextureManager.txt", "w");
  if (f) {
    print (f);
    fclose (f);
  }

  // Clean up textures
  std::map<string, CTextureInfo*>::iterator i;
  for (i=mapTexture.begin(); i!=mapTexture.end(); i++) {
    glDeleteTextures (1, &i->second->texid);
    SAFE_DELETE (i->second);
  }
  mapTexture.clear();

  // Clean up log file
  SAFE_DELETE (log);
}

GLuint CTextureManager::FindTexture (string key)
{
  GLuint texid = 0;
  std::map<string,CTextureInfo*>::iterator i = mapTexture.find(key);
  if (i != mapTexture.end()) texid = i->second->texid;

  // Generate log when lookup is successful
  if (texid != 0) {
    Log ("FindTexture : %s", key.c_str());
    nFindSuccess++;
  } else {
    nFindFailure++;
  }

  return texid;
}

ETextureSize CTextureManager::TextureSize (UINT32 w, UINT32 h)
{
  ETextureSize rc = TEXTURE_SIZE_OTHER;

  switch (w) {
    case 64:
      if (h == 64) rc = TEXTURE_SIZE_64;
      break;
    case 128:
      if (h == 128) rc = TEXTURE_SIZE_128;
      break;
    case 256:
      if (h == 256) rc = TEXTURE_SIZE_256;
      break;
    case 512:
      if (h == 512) rc = TEXTURE_SIZE_512;
      break;
    case 1024:
      if (h == 1024) rc = TEXTURE_SIZE_1024;
      break;
  }
  return rc;
}


GLuint CTextureManager::BindTexture (string key, CImageDXT *dxt)
{
  Log ("BindTexture : %s", key.c_str());
  nBind++;

  // Verify that there is not already a texture bound to this key value
  GLuint texid = FindTexture (key);
  if (texid == 0) {
    // Generate GL texture object
    glGenTextures (1, &texid);

    // Copy texture data to GL texture object
    dxt->BindTexture (texid);

    // Store texture ID in lookup map
    CTextureInfo *info = new CTextureInfo;
    info->type = TEXTURE_TYPE_DXT;
    info->texid = texid;
    info->width = dxt->GetWidth();
    info->height = dxt->GetHeight();
    info->texMemorySize = dxt->GetTextureMemorySize();
    mapTexture[key] = info;

    // Increment texture usage count for this type and size
    dxtCount[TextureSize(info->width, info->height)] += 1;
  } else {
    // Generate a warning that BindTexture was called when a texture was already bound
    WARNINGLOG ("Texture already bound to %s", key.c_str());
  }
  return texid;
}
void CTextureManager::DiscardTexture (string key)
{
  std::map<string, CTextureInfo*>::iterator i = mapTexture.find(key);
  if (i != mapTexture.end()) {
    CTextureInfo *info = i->second;

    // Delete texture
    glDeleteTextures (1, &info->texid);

    // Decrement metric count for this texture type and size
    switch (info->type) {
      case TEXTURE_TYPE_TIF:
        tifCount[TextureSize(info->width, info->height)] -= 1;
        break;
      case TEXTURE_TYPE_RAW:
        rawCount[TextureSize(info->width, info->height)] -= 1;
        break;
      case TEXTURE_TYPE_DXT:
        dxtCount[TextureSize(info->width, info->height)] -= 1;
        break;
    }
  }
}

void CTextureManager::print (FILE *f)
{
  fprintf (f, "CTextureManager\n\n");

  // Dump stats
  fprintf (f, "Statistics:\n");
  fprintf (f, "  numTextures:    %d\n", mapTexture.size());
  fprintf (f, "\n");

  // Dump overall metrics
  fprintf (f, "Metrics:\n");
  fprintf (f, "  nFindSuccess:   %d\n", nFindSuccess);
  fprintf (f, "  nFindFailure:   %d\n", nFindFailure);
  fprintf (f, "  nBind:          %d\n", nBind);
  fprintf (f, "\n");

  // Dump TIF metrics
  fprintf (f, "                 TIF  RAW  DXT\n");
  fprintf (f, "   64 x   64 : %4d  %4d  %4d\n",
    tifCount[TEXTURE_SIZE_64], rawCount[TEXTURE_SIZE_64], dxtCount[TEXTURE_SIZE_64]);
  fprintf (f, "  128 x  128 : %4d  %4d  %4d\n",
    tifCount[TEXTURE_SIZE_128], rawCount[TEXTURE_SIZE_128], dxtCount[TEXTURE_SIZE_128]);
  fprintf (f, "  256 x  256 : %4d  %4d  %4d\n",
    tifCount[TEXTURE_SIZE_256], rawCount[TEXTURE_SIZE_256], dxtCount[TEXTURE_SIZE_256]);
  fprintf (f, "  512 x  512 : %4d  %4d  %4d\n",
    tifCount[TEXTURE_SIZE_512], rawCount[TEXTURE_SIZE_512], dxtCount[TEXTURE_SIZE_512]);
  fprintf (f, " 1024 x 1024 : %4d  %4d  %4d\n",
    tifCount[TEXTURE_SIZE_1024], rawCount[TEXTURE_SIZE_1024], dxtCount[TEXTURE_SIZE_1024]);
  fprintf (f, "       Other : %4d  %4d  %4d\n",
    tifCount[TEXTURE_SIZE_OTHER], rawCount[TEXTURE_SIZE_OTHER], dxtCount[TEXTURE_SIZE_OTHER]);
  fprintf (f, "\n");

  // Dump texture manager objects
  UINT32 texSizeTotal = 0;
  std::map<string, CTextureInfo*>::iterator i;
  for (i=mapTexture.begin(); i!=mapTexture.end(); i++) {
    CTextureInfo *info = i->second;
    fprintf (f, "%4d  %-40s  %4d x %4d (%d KB)\n",
             info->texid, i->first.c_str(),
             info->width, info->height, info->texMemorySize / 1024);
    texSizeTotal += info->texMemorySize;
  }
  fprintf (f, "\n");
  fprintf (f, "Texture total : %d MB\n", texSizeTotal / (1024 * 1024));
}

void CTextureManager::Log (const char *fmt, ...)
{
  if (log) {
		va_list argp;
		va_start(argp, fmt);
    log->Write (fmt, argp);
		va_end(argp);
	}
}
