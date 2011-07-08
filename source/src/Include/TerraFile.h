/*
 * TerraFile.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
 * Copyright 2007 Jean Sabatier
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


#ifndef TERRAFILE_H
#define TERRAFILE_H
#include "../Include/FlyLegacy.h"
//========================================================================================
//============================================================================
//  Class to manage the ImageTile file
//============================================================================
class CTerraFile {
  //---------------------------------------------------
  PODFILE*  Tiles;
  char      fname[PATH_MAX];
  //-----METHODS --------------------------------------
public:
  CTerraFile(char *fn);
 ~CTerraFile();
  void  GetBase(U_INT ax, U_INT az, char *base);   // Get Tile type in name
  bool  Write  (U_INT ax, U_INT az, char *type);   // Change tile type
  bool  OpenForWrite();
};
//==========END OF THIS FILE ==================================================
#endif // TERRAFILE_H
