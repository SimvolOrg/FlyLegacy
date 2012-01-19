/*
 * TerraFile.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
 * Copyright 2007-2009 Jean Sabatier
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
#include "../Include/TerraFile.h"
#include "../Include/Globals.h"
#include "../Include/TerrainUnits.h"
using namespace std;
//========================================================================
//  Constructor
//========================================================================
CTerraFile::CTerraFile(char *fn)
{ Tiles   = popen (&globals->pfs, fn);
  if (0 == Tiles) gtfo ("TCache: Cannot open %s",fn);
  strncpy(fname,fn,(MAX_PATH-1));
}
//-------------------------------------------------------------------
//  Close tile image
//-------------------------------------------------------------------
CTerraFile::~CTerraFile()
{ if (Tiles) pclose(Tiles);
}
//-------------------------------------------------------------------
//  Read the Tile Name as 4 characters AXBY
//-------------------------------------------------------------------
void CTerraFile::GetBase(U_INT ax, U_INT az, char *base)
{ // Calculate offset in tiletype.img for this tile.
  // The top-left entry in the image is x=0, z=16383
  
  U_LONG offset = ((16383 - az) << TCBY16384) + ax;
  pseek (Tiles, offset, SEEK_SET);
  pread (base, 2, 1, Tiles);
  //----Get the types immediately below -------------
  offset  += TC_FULL_WRD_TILE;
  if (offset > (U_INT)Tiles->size) offset = (Tiles->size - TC_FULL_WRD_TILE);
  pseek (Tiles, offset, SEEK_SET);
  pread (base+2, 2, 1, Tiles);
  //--check for zero meridian crossing ---------------
  if (ax != TC_TILE_MODULO) return;
  base[3] = base[1];                      // X is inf fact Y
  offset  = ((16383 - az) << TCBY16384);
  pseek (Tiles, offset, SEEK_SET);
  pread (base + 1, 1, 1, Tiles);
  return;
}
//-------------------------------------------------------------------
//  Close and Reopen for Write
//-------------------------------------------------------------------
bool CTerraFile::OpenForWrite()
{ pclose(Tiles);        // Close file in read mode
  //---- try for write mode --------------------------------
  Tiles   = popen (&globals->pfs, fname,"rb+");
  if (Tiles)      return true;
  //---- Reopen in read only mode --------------------------
  Tiles   = popen (&globals->pfs, fname);
  if (0 == Tiles) gtfo ("TCache: Cannot open %s",fname);
  return false;
}
//-------------------------------------------------------------------
//  Write the new type and reopen in read mode
//-------------------------------------------------------------------
bool CTerraFile::Write(U_INT ax, U_INT az, char *tp)
{ if (!OpenForWrite()) return false;
  U_INT offset = ((16383 - az) << TCBY16384) + ax;
  pseek (Tiles, offset, SEEK_SET);
  pwrite(tp, sizeof(char),1,Tiles);
  pclose(Tiles);
  Tiles   = popen (&globals->pfs, fname);
  if (0 == Tiles) gtfo ("TCache: Cannot open %s",fname);
  return true;
}

//=========================END 0F FILE ====================================================
