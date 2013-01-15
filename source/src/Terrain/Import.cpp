/*
 * IMPORT QGT elevations
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
 * Copyright 2007      Jean Sabatier
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
#include "../include/Import.h"
#include "../Include/TerrainElevation.h"
#include "../Include/FileParser.h"
#include "../Include/SqlMGR.h"
#include "../Include/GeoMath.h"
//=============================================================================
//  Elevation spot
//=============================================================================
CElvSpot::CElvSpot()
{ elev  = 0;
  Clear();
}
//-----------------------------------------------------------------
//  Clear the spot
//-----------------------------------------------------------------
void  CElvSpot::Clear()
{ type  = AREA_INVD;
  done  = 0;
  Dim   = 0;
  elev  = 0;
  Val   = 0;
  return;
}
//-----------------------------------------------------------------
//  Init the spot
//-----------------------------------------------------------------
void CElvSpot::Set(char tp,int dm,int *el)
{ type  = tp;
  Dim   = dm;
  elev  = el;
  return;
}
//-----------------------------------------------------------------
//  Init the spot
//-----------------------------------------------------------------
void CElvSpot::Set(char tp,int dm,int vl)
{ type  = tp;
  Dim   = dm;
  Val   = vl;
  elev  = 0;
  return;
}

//-----------------------------------------------------------------
//  Copy elevations
//-----------------------------------------------------------------
void CElvSpot::CopyTo(int *dst,int nbv)
{ int  mvt = Dim * Dim;
  int *src = elev;
  if (nbv != mvt) gtfo("Area error");
  for(int k=0; k<nbv; k++) *dst++ = *src++;
  return;
}
//-----------------------------------------------------------------
//  Check for type of area
//-----------------------------------------------------------------
bool CElvSpot::SameType(CRectArea *zon)
{ if ( 1 == done)                               return false;
  if ((type != EL_UNIC) && (type == zon->type)) return true;
  if ((type != EL_UNIC) && (type != zon->type)) return false;
  if ((type == EL_UNIC) && (type != zon->type)) return false;
  return (Val  == zon->elv);
}
//=============================================================================
//  Rectangle area
//=============================================================================
CRectArea::CRectArea()
{ Clear();
}
//------------------------------------------------------------------
//  Clear area
//------------------------------------------------------------------
void  CRectArea::Clear()
{ type  = 0;
  lgx   = 0;
  lgz   = 0;
  col   = 0;
  row   = 0;
  end   = 0;
  return;
}
//=============================================================================
//  Class to Import several data types
//=============================================================================
CImport::CImport()
{ //------------------------------------------------------------
  int opt = 0;
  GetIniVar("SQL","ExpELV",&opt);
  elv = opt;
  Reset();
  nArt  = 0;
}
//-----------------------------------------------------------------
//  Reset array
//-----------------------------------------------------------------
void CImport::Reset()
{ mono  = 0;
  unic  = 0;
  nReg  = 0;
  for (int k=0; k < SPOT_NBER; k++) Spot [k].Clear();
  for (int k=0; k < 32;        k++) Stack[k].Clear();
  widx  = 0;
  ridx  = 0;
  return;
}
//-----------------------------------------------------------------
//  Import elevation file
//  TODO:  Check for existing database
//-----------------------------------------------------------------
void CImport::ImportElevations()
{ //if (0 == globals->elvDB)    return;
  if (0 == globals->sqm->UseElvDB())	return;
  if (0 == elv)               return;
  ImportElevations("Import/sanfrancisco.bt");
}

//-----------------------------------------------------------------
//  Import elevation file
//  Elevation are processed QGT per QGT
//  The array Spot is used to store for each detail tile the
//  spot elevation:
//  Type 0:   This is a unic elevation shared between sevral tiles
//  Type 1:   This is the unic center elevation for the detail tile
//  Type 2:   Elevation is an array of integers of dimension dim 
//-----------------------------------------------------------------
//  1) The elevations found in the database are loaded
//  2) The elevations from the BT file for this QGT are loaded
//     by => GetRegionElevation
//  3) New elevation overwrite those from the database =>Overwrite
//  4) Rectabgular region of same type are computed => CreateAreas
//-----------------------------------------------------------------
void CImport::ImportElevations(char *fn)
{ REGION_REC reg;
  reg.qgt = 0;
  CBtParser btp(fn);
  Dim = btp.Resolution();
  TRACE("=====BUILDING REGIONS ==========================");
  while (  btp.GetQgtKey(reg.qtx,reg.qtz))
  { reg.key = QGTKEY(reg.qtx,reg.qtz);
    TRACE("---QGT key=%010d",reg.key);
   //--- Load existing elevations from database ----
    globals->sqm->GetQgtElevation(reg,ELVtoSlots);
    btp.GetRegionElevation(reg);
    Overwrite(reg);
    //--- Create areas ----------------------------
    CreateAreas();
    area.qgt = 0;
    area.qtx = reg.qtx;
    area.qtz = reg.qtz;
    globals->sqm->DeleteElevation(reg.key);
    WriteRegions(1);
    //----Reset all resources ---------------------
    Reset();
    delete [] reg.data;
    reg.data = 0;
    btp.NextQgtKey();
  }
  return;
}
//---------------------------------------------------------------------------
//  Set Region Elevation
//  from the bt file
//---------------------------------------------------------------------------
void CImport::Overwrite(REGION_REC &reg)
{ int *src  = reg.data;
  int  inc  = reg.sub * reg.sub;
  int   No  = 0;
  U_INT ftx = reg.dtx + reg.lgx;
  U_INT ftz = reg.dtz + reg.lgz;
  for   (U_INT z = reg.dtz; z < ftz; z++)
  { for (U_INT x = reg.dtx; x < ftx; x++)
    { No = (z << TC_BY32) + x;
      Spot[No].Set(EL_MULT,reg.sub,src);
      src += inc;
    }
  }
  return;
}
//---------------------------------------------------------------------------
//  Set Region Elevation
//  Region is a square whose base det is the SW corner
//---------------------------------------------------------------------------
void CImport::SetUnicGrid(REGION_REC &reg)
{ int   No = 0;
  int   tx = reg.dtx;
  int   tz = reg.dtz - 1;
  int   el = reg.val;
  for   (U_INT z = 0; z < reg.lgz; z++)
  { tz++;
    tx   = reg.dtx;
    for (U_INT x = 0; x < reg.lgx; x++)
    { No = (tz << TC_BY32) + tx;
      Spot[No].Set(EL_UNIC,1,el);
      tx++;
      unic++;
    }
  }
  return;
}
//---------------------------------------------------------------------------
//  Set Region Elevation
//  Region is a square whose base det is the SW corner
//---------------------------------------------------------------------------
void CImport::ElevationFromREG(REGION_REC &reg)
{ nReg++;
  if (0 == reg.type)  return SetUnicGrid(reg);
  int     No = 0;
  int   *src = (int*)reg.data;
  int     tx = reg.dtx;
  int     tz = reg.dtz - 1;
  for   (U_INT z = 0; z < reg.lgz; z++)
  { tz++;
    tx   = reg.dtx;
    for (U_INT x = 0; x < reg.lgx; x++)
    { int el = *src++;
      No = (tz << TC_BY32) + tx;
      Spot[No].Set(EL_MONO,1,el);
      tx++;
      mono++;
    }
  }
  return;
}
//--------------------------------------------------------------------------
//  Search for area X size
//  The base row of a region is made of all horizontal tiles
//  of the same type inside the QGT. 
//  A slot pertaining to a previous region stops the row scan
//--------------------------------------------------------------------------
void  CImport::GetBaseRow(CRectArea *zon)
{ int row = zon->row;
  int No  = 0;
  for (int x = zon->col + 1; x < MAT_END; x++)
  { No = (row << TC_BY32) + x;
    if (!Spot[No].SameType(zon))  return;
    zon->lgx++;
  }
  //----close X direction---------------------
  zon->end |= AREA_XEND;
  return;
}
//--------------------------------------------------------------------------
//  Search for area Z size
//  The rectagle is extended upward by one row when all following conditions
//  are true:
//  -The new row is made of slot of the same type as the base row.
//  -The row count is at least the same size of the base row.
//--------------------------------------------------------------------------
void  CImport::GetLastRow(CRectArea *zon)
{ char  typ = zon->type;
  int   lgx = 0;
  int    No = 0;
  for   (int z = zon->row + 1; z < MAT_END; z++)
  { lgx   = 0;
    for (int x = zon->col;     x < MAT_END; x++)
    { No = (z << TC_BY32) + x;
      if (!Spot[No].SameType(zon))  return;
      lgx++;
      if (lgx < zon->lgx)           continue;
      //---One more row ---------------------
      zon->lgz++;
      break;
    }
  }
  //---Close top direction -------------------
  zon->end |= AREA_ZEND;
  return;
}
//--------------------------------------------------------------------------
//  Invalidate any area that is enclosed with this one
//  All slot in the rectangular area are marked as "done"
//--------------------------------------------------------------------------
void CImport::AreaDone(CRectArea *zon)
{ int fz = zon->row + zon->lgz;
  int fx = zon->col + zon->lgx;
  int No = 0;
  for   (int z = zon->row; z < fz; z++)
  { for (int x = zon->col; x < fx; x++)
    { No = (z << TC_BY32) + x;
      Spot[No].MarkDone();
    }
  }
  return;
}
//--------------------------------------------------------------------------
//  Create a new area
//  1)  A rectangular area is located staring from the row and column of the region
//  2)  Two new candidate areas are pushed down to the stack:  One from
//      the horizontal end and one from the top row, provide the slots are not
//      already done.
//  
//--------------------------------------------------------------------------
void  CImport::OpenArea(CRectArea *zon)
{ char row  = zon->row;
  char col  = zon->col;
  zon->lgx  = 1;
  zon->lgz  = 1;
  zon->iden = ridx;
  zon->end  = 0;
  int No    = (row << TC_BY32) + col;
  zon->type = Spot[No].GetType();
  zon->elv  = Spot[No].GetValue();
  GetBaseRow(zon);
  GetLastRow(zon);
  AreaDone(zon);
  //---Add right and top area ----------------
  if (zon->OpenToX())
  { CRectArea *rzn = Stack + widx;
    rzn->col  = zon->col + zon->lgx;
    rzn->row  = zon->row;
    if (FreeSpot(rzn)) widx++;
    if (widx == AREA_STAK) gtfo ("too much areas");
  }
  if (zon->OpenToZ())
  { CRectArea *tzn = Stack + widx;
    tzn->col  = zon->col;
    tzn->row  = zon->row + zon->lgz;
    if (FreeSpot(tzn)) widx++;
    if (widx == AREA_STAK) gtfo ("too much areas");
  }
}
//--------------------------------------------------------------------------
//  Check if the base slot of an area is Ok for processing
//--------------------------------------------------------------------------
bool CImport::FreeSpot(CRectArea *zon)
{ int z = zon->row;
  int x = zon->col;
  int No = (z << TC_BY32) + x;
  if (Spot[No].IsDone())  return false;
  //---Check for other starting area -----------
  for (int k=ridx; k<widx; k++)
  { CRectArea *zt = Stack + k;
    int col = zt->col;
    int row = zt->row;
    if (col != zon->col)  continue;
    if (row != zon->row)  continue;
    return false;
  }
  return true;
}

//--------------------------------------------------------------------------
//  Create all areas
//  Cut the QGT into rectangular area of the same type.
//  Area are pushed down the Stack.
//--------------------------------------------------------------------------
void CImport::CreateAreas()
{ CRectArea *zon = Stack;
  zon->row  = 0;
  zon->col  = 0;
  widx      = 1;
  while (ridx != widx)
  { zon     = Stack + ridx;
    int x   = zon->col;
    int z   = zon->row;
    int No  = (z << TC_BY32) + x;
    if (Spot[No].IsFree())  OpenArea(zon);
    else    zon->type = AREA_INVD;
    ridx++;
  }
  return;
}
//--------------------------------------------------------------------------
//  Prepare a unique region parameters
//--------------------------------------------------------------------------
void CImport::UnicRegion(CRectArea *zon)
{ area.sub  = 0;
  area.val  = zon->elv;
  area.nbv  = 0;
  area.data = 0;
  return;
}
//--------------------------------------------------------------------------
//  Prepare a mono region parameters
//--------------------------------------------------------------------------
void CImport::MonoRegion(CRectArea *zon)
{ area.sub  = 0;
  area.val  = 0;
  area.nbv  = area.lgx * area.lgz;
  area.data = new int[area.nbv];
  //--Copy elevations from spots ------------
  U_INT row = area.dtz;
  U_INT col = area.dtx;
  U_INT ftx = col + area.lgx;
  U_INT ftz = row + area.lgz;
  int  *dst = (int*)area.data;
  int   No  = 0;
  for   (U_INT z=row; z<ftz; z++)
  { for (U_INT x=col; x<ftx; x++)
    {  No     = (z << TC_BY32) + x;
      *dst++  = Spot[No].GetValue();
    }
  }
  return;
}
//--------------------------------------------------------------------------
//  Prepare a multi region parameters
//--------------------------------------------------------------------------
void CImport::MultRegion(CRectArea *zon)
{ int  vol  = Dim * Dim;
  area.sub  = Dim;            // Dimension
  area.val  = 0;
  area.nbv  = area.lgx * area.lgz * vol;
  area.data = new int[area.nbv];
  nArt++;
  //--Copy elevations from spots ------------
  U_INT row = area.dtz;
  U_INT col = area.dtx;
  U_INT ftx = col + area.lgx;
  U_INT ftz = row + area.lgz;
  int  *dst = (int*)area.data;
  int   No  = 0;
  for   (U_INT z=row; z<ftz; z++)
  { for (U_INT x=col; x<ftx; x++)
    { No  = (z << TC_BY32) + x;
      Spot[No].CopyTo(dst,vol);
      dst += vol;
    }
  }
  return;
}
//--------------------------------------------------------------------------
//  Prepare the region parameters
//--------------------------------------------------------------------------
bool CImport::RegionParameters(CRectArea *zon)
{ //---Init common parameters ---------------
  area.dtx  = zon->col;
  area.dtz  = zon->row;
  area.lgx  = zon->lgx;
  area.lgz  = zon->lgz;
  area.type = zon->type;
  //---Individal types ----------------------
  switch (zon->type)  {
    case EL_UNIC:
      UnicRegion(zon);
      if (area.val == 0)  return false;
      break;
    case EL_MONO:
      MonoRegion(zon);
      break;
    case EL_MULT:
      MultRegion(zon);
      break;
    case AREA_INVD:
      return false;
  }
  //---Write to SQL database ----------------
  TRACE("------REGION N°%02d type %d at %03d-%03d lgx=%02d lgz=%02d NBV=%05d",
    zon->iden,area.type,area.dtx,area.dtz,area.lgx,area.lgz,area.nbv);
  return true;
}
//--------------------------------------------------------------------------
//  Write Region parameters
//--------------------------------------------------------------------------
void CImport::WriteRegions(char opt)
{ SqlMGR *sqm = globals->sqm;
  int inx = 0;
  while (inx != widx)
  { CRectArea *zon = Stack + inx;
    if (RegionParameters(zon) && opt)  sqm->WriteElevationRecord(area);
    inx++;
  }
  return;
}
//==========================================================================
//  Export elevations to Database
//==========================================================================
//--------------------------------------------------------------------------
//  Export this elevation file
//--------------------------------------------------------------------------
void CImport::ExportElevations()
{ if (0 == elv)   return;
  char name[MAX_PATH];
  int qNo = 0;
  //---Get a list of all QTR file -------------------------
  for (qNo = 0; qNo != 1024; qNo++)
  { sprintf(name,"Data/%03X.QTR",qNo);
    if (pexists (&globals->pfs, name))   ExportElevationFile(qNo);
  }
  return;
}

//--------------------------------------------------------------------------
//  Export this elevation file
//--------------------------------------------------------------------------
void CImport::ExportElevationFile(int no)
{ SqlMGR *sqm = globals->sqm;
  REGION_REC reg;
  C_QTR  *qtr = new C_QTR(no,0);
//  sqm->ELVtransaction();
  //----extract all QGT regions ---------------------------------
  for   (int rz = 0; rz < 16; rz++)
  { for (int rx = 0; rx < 16; rx++)
    { //---Process one QGT -----------------
      Reset();
      while (qtr->PopRegionInfo(rx,rz,reg))  ElevationFromREG(reg);
      if (0 == nReg)  continue;
      TRACE("...QGT(%03d-%03d)",reg.qtx,reg.qtz); 
      CreateAreas();
      area.qgt = 0;
      area.qtx = reg.qtx;
      area.qtz = reg.qtz;
      WriteRegions(1);
    }
  }
//  sqm->ELVcommit();
  delete qtr;
  return;
}

//===================================END OF FILE ==========================================