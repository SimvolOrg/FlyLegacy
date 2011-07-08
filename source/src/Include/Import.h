/*
 * IMPORT.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
 * Copyright 2008      Jean Sabatier
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
#ifndef IMPORT_H
#define IMPORT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "../Include/FlyLegacy.h"

//============================================================================
class C_QTR;
//============================================================================
typedef enum {
  SPOT_NBER = 1024,
} ElvEspot;
typedef enum {
  AREA_XEND = 1,
  AREA_ZEND = 2,
  AREA_INVD = 7,
  AREA_STAK = 256,
  MAT_END   = 32,
} AreaEval;
//============================================================================
//  Rectangle definition
//============================================================================
class CRectArea {
public:
  char    iden;                       // Number
  char    type;                       // Type
  U_CHAR  lgx;                        // X size
  U_CHAR  lgz;                        // Z size
  U_CHAR  col;                        // Column
  U_CHAR  row;                        // Row
  char    end;                        // Indicator
  int     elv;                        // Elevation
  //---Methods -------------------------------------
public:
  CRectArea();
  void  Clear();
  //--- Check Open to right ------------------------
  bool  OpenToX() {return (0 == (end & AREA_XEND));}
  bool  OpenToZ() {return (0 == (end & AREA_ZEND));}
  bool  NoValid() {return (type == AREA_INVD);}
};
//============================================================================
//  Elevation spot
//============================================================================
class CElvSpot {
  char  type;                         // Type of spot
  char  done;                         // Done indicator
  int   Dim;                          // Array X
  int   Val;                          // Unic elevation
  int  *elev;                         // Elevation array
  //-------------------------------------------------------------------
public:
  CElvSpot();
  void  Clear();
  void  Set(char tp,int dim,int *el);
  void  Set(char tp,int dim,int  vl);
  bool  SameType(CRectArea *zon);
  void  CopyTo(int *dst,int nbv);
  //-------------------------------------------------------------------
  inline char GetType()         { return type;}
  inline int  GetValue()        { return Val;}
  inline void MarkDone()        { done = 1;}
  inline bool IsDone()          { return (0 != done);}
  inline bool IsFree()          { return (0 == done);}
};
//============================================================================
//  Work area for importing files
//============================================================================
class CImport {
  //--- ATTRIBUTES ---------------------------------------
  int   Dim;                            // Array dimension
  int   unic;
  int   mono;
  int   nReg;                           // Region number
  int   nArt;                           // Article number
  char  elv;                            // Export indicator
  char  widx;                           // write index
  char  ridx;                           // Read index
  REGION_REC  area;                     // Area to write
  CRectArea   Stack[AREA_STAK];         // Stack of area
  CElvSpot    Spot[SPOT_NBER];          // Array of spot elevations
  //--- Methods ------------------------------------------
public:
  CImport();
  //---Helpers -------------------------------------------
  void  Reset();
  void  ElevationFromREG(REGION_REC &reg);
  void  SetUnicGrid(REGION_REC &reg);
  void  Overwrite(REGION_REC &reg);
  //------------------------------------------------------
  void  ImportElevations();
  void  ImportElevations(char *fn);
  //------------------------------------------------------
  void  GetBaseRow(CRectArea *zon);
  void  GetLastRow(CRectArea *zon);
  void  OpenArea(CRectArea *zon);
  void  CreateAreas();
  void  AreaDone(CRectArea *zon);
  bool  FreeSpot(CRectArea *zon);
  //---Write elevations ----------------------------------
  bool  RegionParameters(CRectArea *zon);
  void  UnicRegion(CRectArea *zon);
  void  MonoRegion(CRectArea *zon);
  void  MultRegion(CRectArea *zon);
  void  WriteRegions(char opt);
  //---Export to database --------------------------------
  void  ExportElevations();
  void  ExportElevationFile(int no);
};
//============================END OF FILE =================================================
#endif  // IMPORT_H
