//=========================================================================================
/*
 * GeoMath.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2007 Chris Wallace
 * Copyright 2007 Jean Sabatier
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
//=========================================================================================
#include "../Include/FlyLegacy.h"
#include "../Include/Globals.h"
#include "../Include/GeoMath.h"
#include "../Include/TerrainUnits.h"
#include "../Include/TerrainCache.h"
#include "../Include/TerrainTexture.h"
#include <math.h>
//================================================================================
//================================================================================
//  JS:  Define the global tile table
//        Each entry contains the folowing fields:
//        1)lat   : Base latitude for this row of tiles
//        2)side  : Side dimension of tiles in nm at this latitude
//        3)hrz   : Number of horizontal tiles needed to cover the    COVERED_RADIUS
//        4)vrp   : Number of vertical tiles toward the pole to cover COVERED_RADIUS
//        5)vrq   : Number of vertical tiles toward equator  to cover COVERED_RADIUS
//        COVER_RADIUS define the radius around the aircraft 
//  NOTE:  globe_tile_lat should be merged in qtg_lattitude later
//        and the search functions modified to use new table.
//================================================================================
typedef struct {
  double lat;                           // Base latitude in �
  double lats;                          // Base latitude in arcsec
  double dta;                           // Latitude delta for QGT in arcsec
  double det;                           // Latitude delta per Detail Tile
	double sub;														// Latitude delta per subdivision
  double mid;                           // Mid point in arcsec (GBT only) ---
  double rdf;                           // Longitude reduction factor
  double cpf;                           // Longitude compensation factor (inverse of rdf)
  //-----------Visibility parameters (in QGT only) --------------------------
  double fmax;                          // Max visibility in feet
  float vmax;                           // Visibility in miles
  float dfog;                           // Fog density
	//---- Dimensions --------------------------------------------------------
	double mSide;													// Side in miles
	double vArcs;													// Side in arcsec at mid QGT (Vertical)
  //------------------------------------------------------------------------
  short hzr;                            // horizontal range
  short vpr;                            // Vertical range to pole
  short vqr;                            // Vertical range to equator
} GLOBE_TILE;
//================================================================================
//  GLOBE TILE AND QGT TABLES
//================================================================================
GLOBE_TILE globe_tile_lat [132];
GLOBE_TILE qgt_latitude[260];
//==========================================================================
//  SUPER TILE Base Detail Tile coordinates
//==========================================================================
TC_INCREMENT SuperDT[]  = {
  { 0, 0},{ 4, 0},{ 8, 0},{12, 0},{16, 0},{20, 0},{24, 0},{28, 0},
  { 0, 4},{ 4, 4},{ 8, 4},{12, 4},{16, 4},{20, 4},{24, 4},{28, 4},
  { 0, 8},{ 4, 8},{ 8, 8},{12, 8},{16, 8},{20, 8},{24, 8},{28, 8},
  { 0,12},{ 4,12},{ 8,12},{12,12},{16,12},{20,12},{24,12},{28,12},
  { 0,16},{ 4,16},{ 8,16},{12,16},{16,16},{20,16},{24,16},{28,16},
  { 0,20},{ 4,20},{ 8,20},{12,20},{16,20},{20,20},{24,20},{28,20},
  { 0,24},{ 4,24},{ 8,24},{12,24},{16,24},{20,24},{24,24},{28,24},
  { 0,28},{ 4,28},{ 8,28},{12,28},{16,28},{20,28},{24,28},{28,28},
};

//================================================================================
//  Compute vertical range toward pole for Globe Tile
//================================================================================
short GetVerticalPoleRange(short no)
{ float dis = COVERED_RADIUS;
  short rg  = 0;
  while (no != 127)
  { dis -= globe_tile_lat[no++].mSide;
    rg++;
    if (dis <= 0) return rg;
  }
  return rg;
}
//---------------------------------------------------------------------------------
//  Compute vertical range toward equator for Globe Tile
//---------------------------------------------------------------------------------
short GetVerticalEquatorRange(short no)
{ float dis = COVERED_RADIUS;
  short rg  = 0;
  while (no >= 0)
  { dis -= globe_tile_lat[no--].mSide;
    rg++;
    if (dis <= 0) return rg;
  }
  no = 0;
  while (no != 127)
  { dis -= globe_tile_lat[no++].mSide;
    rg++;
    if (dis <= 0) return rg;
  } 
  return rg;
}

//--------------------------------------------------------------------------------
// @brief Initialize globe tile latitude lookup table
//
// The globe tile latitude lookup table allows determination of N and S latitude
//   boundaries for all 256 possible globe tile z-indices.  Entry zero is
//   latitude 0 (the equator) and successive entries are the latitudes of each
//   globe tile boundary approaching the poles.
//
// The formula for globe tile latitude boundaries is:
//   k = (360 / 256)
//   L(0) = 0
//  L(n) = L(n-1) + k * cos[L(n-1)] 
//-------------------------------------------------------------------------------
void InitGlobeTileTable ()
{ U_INT tr = globals->Trace.Has(TRACE_LATITUDE);
  if (tr) TRACE("====INIT GLOBAL TILE PARAMETERS ======================");
  int i       = 0;
  double side = (double(MILE_CIRCUMFERENCE) / 256);
  int    hrz  = int(COVERED_RADIUS / side) + 1;
  double k    = 360.0 / 256.0;
  double prev = 0;
  double lat  = 0;
  double lats = 0;
  globe_tile_lat[0].lat   = 0.0;
	globe_tile_lat[0].lats  = 0;
  globe_tile_lat[0].mSide = side;
  globe_tile_lat[0].dta   = 0;
  globe_tile_lat[0].vArcs = 0;
  globe_tile_lat[0].hzr   = hrz;
  globe_tile_lat[0].dfog  = 0.000016f;
  for (i=1; i<128; i++) 
	{	int pp = i - 1;
    double prevRad          = DegToRad (lat);
    lat											= lat + k * cos(prevRad);
		//--- Set latitude in degre and arcsec ----------------------------
    globe_tile_lat[i].lat   = lat;                   // Latitude in �
    lats                    = lat * 3600;            // Latitude in arcsec
    globe_tile_lat[i].lats  = lats;
		//--- Compute Latitude Delta for this indice ----------------------
    globe_tile_lat[pp].dta  = (lats - globe_tile_lat[pp].lats);
		globe_tile_lat[pp].sub  = 0;
    globe_tile_lat[pp].mid  = (lats + globe_tile_lat[pp].lats) * 0.5;
		//--- Compute side -------------------------------------------------
		double latrad   = DegToRad(lat);
		double cosn			= cos(latrad);
    side            = (MILE_CIRCUMFERENCE * cosn) / 256;
    hrz             = int(COVERED_RADIUS / side) + 1;
    globe_tile_lat[i].mSide = side;
		globe_tile_lat[i].vArcs = 0;
    globe_tile_lat[i].hzr   = hrz;
    globe_tile_lat[i].dfog  = 0;
  }
    //---------Compute the vertical ranges ---------------
  short  vpr = 0;
  short  vqr = 0;
  double dta = 0;
  double mid = 0;
  double las = 0;
  for (i = 0; i<128; i++)
  { double lat	= globe_tile_lat[i].lat;
    las					= globe_tile_lat[i].lats;
		double dim	= globe_tile_lat[i].mSide;
    dta					= globe_tile_lat[i].dta;
    mid					= globe_tile_lat[i].mid;
    hrz					= globe_tile_lat[i].hzr;
    vpr = GetVerticalPoleRange(i);
    vqr = GetVerticalEquatorRange(i);
    globe_tile_lat[i].vpr   = vpr;
    globe_tile_lat[i].vqr   = vqr;
    if (tr) TRACE("i=%03u LAT=%0.6lf Side=%8.4lf (Mid=%8.4lfnm) HR=%2u VP=%2u VQ=%2u",i,lat,dim,mid,hrz,vpr,vqr);
  }
  //-----------Last entry for init ----------------------
  globe_tile_lat[128].mSide = 0.0;
  globe_tile_lat[128].hzr   = 0;
  globe_tile_lat[128].vpr   = 0;
  globe_tile_lat[128].vqr   = 0;
	globe_tile_lat[127].vpr   = 1;
	globe_tile_lat[126].vpr   = 2;
  return;
}
//-----------------------------------------------------------------------------
//  JS:  Find the global tile number by dichotomy
//-----------------------------------------------------------------------------
int GetVerticalGlobeTileNumber(float lat)
{ int num   = 64;                         // Start in middle
  int inc   = 32;                         // First inc
  // Convert latitude absolute value to degrees
  double val = fabs(lat / 3600.0);
  while (inc)
  { if (val == globe_tile_lat[num].lat) return num;
    if (val  > globe_tile_lat[num].lat) num += inc;
    if (val  < globe_tile_lat[num].lat) num -= inc;
    inc = (inc >> 1);
  }
  if (val  < globe_tile_lat[num].lat) num--;
  return (lat >+ 0)? (128 + num):(127 - num);
}
//-----------------------------------------------------------------------------
//  return number of tiles needed to reach COVERED_RADIUS in horizontal direction
//  vt is the vertical tile number
//-----------------------------------------------------------------------------
short GetGbtHorzRange(short vt)
{ int No = (vt >= 128)?(vt - 128):(127 - vt);
  return globe_tile_lat[No].hzr; }
//-----------------------------------------------------------------------------
//  return number of tiles needed to reach COVERED_RADIUS in pole direction
//  vt is the vertical tile number
//-----------------------------------------------------------------------------
short GetGbtVertPoleRange(short vt)
{ int No = (vt >= 128)?(vt - 128):(127 - vt);
  return globe_tile_lat[No].vpr; }
//-----------------------------------------------------------------------------
//  return number of tiles needed to reach COVERED_RADIUS in equator direction
//  vt is the vertical tile number
//-----------------------------------------------------------------------------
short GetGbtVertEquaRange(short vt)
{ int No = (vt >= 128)?(vt - 128):(127 - vt);
  return globe_tile_lat[No].vqr; }
//-----------------------------------------------------------------------------
//  Return South latitude of Global in arcsec
//-----------------------------------------------------------------------------
double GetGbtSouthLatitude(short cz)
{ int     No = (cz >= 128)?(cz - 128):(128 - cz);
  double lat = globe_tile_lat[No].lats;
  return (cz >= 128)?(lat):(-lat);  }

//-----------------------------------------------------------------------------
//  Return North latitude of Global in arcsec
//-----------------------------------------------------------------------------
double GetGbtNorthLatitude(short cz)
{ int     No = (cz >= 128)?(cz - 127):(127 - cz);
  double lat = globe_tile_lat[No].lats;
  return (cz >= 128)?(lat):(-lat);  }
//-----------------------------------------------------------------------------
//  return last latitude
//-----------------------------------------------------------------------------
double LastLatitude()
{ return  globe_tile_lat[127].lats;}
//=================================================================================
//  QGT table management
//=================================================================================
//---------------------------------------------------------------------------------
//  Compute vertical range toward pole for QGT
//---------------------------------------------------------------------------------
short GetQgtVerticalPoleRange(short no,float vmax)
{ float dis = vmax;
  short rg  = 0;
  while (no != 255)
  { dis -= qgt_latitude[no++].mSide;
    rg++;
    if (dis <= 0) return rg;
  }
  return rg;
}
//---------------------------------------------------------------------------------
//  Compute vertical range toward equator for QGT
//---------------------------------------------------------------------------------
short GetQgtVerticalEquatorRange(short no,float vmax)
{ float dis = vmax;
  short rg  = 0;
  while (no >= 0)
  { dis -= qgt_latitude[no--].mSide;
    rg++;
    if (dis <= 0) return rg;
  }
  no = 0;
  while (no != 255)
  { dis -= qgt_latitude[no++].mSide;
    rg++;
    if (dis <= 0) return rg;
  } 
  return rg;
}
//----------------------------------------------------------------------------------
//  Compute VMAX at the given latitude
//----------------------------------------------------------------------------------
float GetQgtVMAX(int tz,float vmax)
{ if (tz <= 105)  return vmax;
  if (tz >= 205)  return 8;
  float K = float(8 - vmax) / float(100);
  return vmax + float(tz - 105) * K ;
}
//----------------------------------------------------------------------------------
//  Compute Fog density at the given latitude
//----------------------------------------------------------------------------------
float GetQgtFog(int tz,double vmax,double vmx)
{ float f0  = 0.000008f;
  float f1  = 0.000045f;
  if (tz <= 105)  return f0;
  if (tz >= 205)  return f1;
  float K = float(vmx / vmax);
  return f1 - (f1 - f0) * K;
}
//==================================================================================
// @brief Initialize QGT tile latitude lookup table
//
// The QTG globe tile latitude lookup table allows determination of N and S latitude
//   boundaries for all 512 possible QTG globe tile z-indices.  Entry zero is
//   latitude 0 (the equator) and successive entries are the latitudes of each
//   QTG globe tile boundary approaching the poles.
//
// The formula for globe tile latitude boundaries is:
//   k = (360 / 512)
//   L(0) = 0
//   L(n) = L(n-1) + k * cos[L(n-1)] 
//  NOTE: 1)  To keep this table in synchronization with the Globe tile table
//            each south base of lower QGT is taken from the Globe tile table
//        2)  The vmax (inner circle) parameter defines the radius of textured terrain.
//            vmax is modulated by the latitude.  As we go toward
//            the pole, tiles get smaller and more are needed to cover a given
//            radius.  Thus we need more and more textures which take too much memory.
//            By limiting vmax, we keep visible tiles in a reasonable limit 
//==================================================================================
void InitQgtTable (float vmax)
{ U_INT tr = globals->Trace.Has(TRACE_LATITUDE);
  if (tr) TRACE("====INIT QGT GLOBAL TILE PARAMETERS ======================");
  int nz       = 0;
  double side = (float(MILE_CIRCUMFERENCE) / 512);
  double feet = (TC_FULL_WRD_FEET / 512);
  double vmx  = vmax;
  double det;
  int    hrz  = int(vmax / side) + 1;
  double k    = 360.0 / 512.0;
  double lat  = 0;
  double lats = 0;
  double spr  = 0;
  double rmn  = 0;
  short vpr = 0;
  short vqr = 0;
	//--- cosinus latitude --------------------------------------------------------
	double cosn = 1;
  //-----------------------------------------------------------------------------
  qgt_latitude[0].lat   = 0.0;
  qgt_latitude[0].mSide = side;
  qgt_latitude[0].rdf   = 1;
  qgt_latitude[0].cpf   = 1;
  qgt_latitude[0].vmax = vmax;
  qgt_latitude[0].fmax = vmax * TC_FEET_PER_MILE;
  qgt_latitude[0].dfog  = GetQgtFog(0,vmax,vmax);
  qgt_latitude[0].hzr   = 2;
  for (nz =1; nz < 257; nz++) {
		int pp = nz-1;
    double prevRad         = DegToRad (lat);
    if (nz & 0x01)   lat   = lat + k * cos(prevRad);
    else             lat   = globe_tile_lat[(nz >> 1)].lat;
    lats                   = lat * 3600;
    qgt_latitude[nz].lat   = lat;                               // Lat in �
    qgt_latitude[nz].lats  = lats;                              // Lat in arcsec
		//--- Compute previous QGT mid point -----------------------
		qgt_latitude[pp].mid   = (lats + qgt_latitude[pp].lats) * 0.5;
		//--- latitude delta for previous QGT -----------------------
    det                    = lats  - qgt_latitude[pp].lats;    // Delta in arcsec
		//--- Compute all subdivisions in previous QGT --------------
    qgt_latitude[pp].dta = det;												// Arcsec per QGT
    qgt_latitude[pp].det = det / TC_DET_PER_QGT;			// Arcsec per Detail Tile
		qgt_latitude[pp].sub = det / TC_SUBD_PER_QGT;			// Arcsec per smallest vertice
		qgt_latitude[pp].vArcs	= det * 0.5;							// Arcsec at mid latitude
		//--- Parameters for this QGT ------------------------------
    double latrad   = DegToRad(lat);
		cosn	= cos(latrad);
    qgt_latitude[nz].rdf   = cosn;												// Reduction factor
    qgt_latitude[nz].cpf   = (1 / cosn);									// Compensation factor 
    side            = (MILE_CIRCUMFERENCE * cosn) / 512;	// QGT side in mile
    feet            = (TC_FULL_WRD_FEET   * cosn) / 512;	// QGT side in feet
    vmx             =  GetQgtVMAX(nz,vmax);                     // Maximum visibility
    qgt_latitude[nz].vmax  = vmx; 
    qgt_latitude[nz].fmax  = vmx * TC_FEET_PER_MILE;
    hrz                    = ceil(vmx / side);
    if (hrz == 1)    hrz   = 2;
    qgt_latitude[nz].mSide = side;
    qgt_latitude[nz].hzr   = hrz;
    qgt_latitude[nz].dfog  = GetQgtFog(nz,vmax,vmx);
  }
    //---------Compute the  ranges --------------------------------
  for (int i = 0; i < 257; i++)
  { double latd = qgt_latitude[i].lat;
    double lats = qgt_latitude[i].lats;
    double vmx  = qgt_latitude[i].vmax;
    double dta  = qgt_latitude[i].dta;
    double side = qgt_latitude[i].mSide;
    double rdf  = qgt_latitude[i].rdf;
    float fog   = qgt_latitude[i].dfog;
    float rst   = (vmx / side) * 8;               // Radius in super tile unit
    float nsp   = floor(rst * rst * PI);					// Number of inner super tiles
    //----------------------------------------------------
    hrz         = qgt_latitude[i].hzr;
    vpr = GetQgtVerticalPoleRange(i,vmx);
    vqr = GetQgtVerticalEquatorRange(i,vmx);
    if (vpr == 1) vpr = 2;
    if (vqr == 1) vqr = 2;
    qgt_latitude[i].vpr   = vpr;
    qgt_latitude[i].vqr   = vqr;
    //---Adjust fmax to let one SP as border in QGT -----------
    spr = side / 8;                                             // SuperTile side in miles
    rmn = (side * hrz) - vmx;
    bool adj  = (rmn < spr);
    rmn = (side * vpr) - vmx;
    adj      |= (rmn < spr);
    rmn = (side * vqr) - vmx;
    adj      |= (rmn < spr);
    if (adj)  qgt_latitude[i].fmax -= spr * TC_FEET_PER_MILE;

    if (tr) TRACE("Z=%03u LAT=%.6lf dta= %.8lf arcs VMAX=%2.4lf RDF=%.5lf FOG=%.8lf HR=%2u VP=%2u VQ=%2u",
                i,lats,dta,vmx,rdf,fog,hrz,vpr,vqr);
  }
  //-----------Last entry for init ----------------------
  qgt_latitude[256].hzr   = 0;
  qgt_latitude[256].vpr   = 0;
  qgt_latitude[256].vqr   = 0;
	//----- Limit pole extension ---------------------------
	qgt_latitude[255].vpr     = 0;
	qgt_latitude[254].vpr     = 1;
  return;
}
//-----------------------------------------------------------------------------
//  JS:  Find the global tile number by dichotomy
//-----------------------------------------------------------------------------
int GetVerticalQgtNumber(double lat)
{ int num   = 128;                         // Start in middle
  int inc   =  64;                         // First inc
  // Convert latitude absolute value to degrees
  double val = (lat < 0)?(-lat):(+lat);
  while (inc)
  { if (val == qgt_latitude[num].lats) break;
    if (val  > qgt_latitude[num].lats) 
    { num += inc;
      inc  = (inc >> 1);
      continue;}
    if (val  < qgt_latitude[num].lats) 
    { num -= inc;
      inc  = (inc >> 1);
      continue;}
    //---Invalid value -------------
    gtfo("QNAN detected");
  }
  if (val  < qgt_latitude[num].lats) num--;
  return (lat >= 0)? (256 + num):(255 - num);
}
//-----------------------------------------------------------------------------
//  Request the QGT indices
//-----------------------------------------------------------------------------
void IndicesInQGT (SPosition pos, U_INT &x, U_INT &z)
{ // X-index is simple calculate of longitude by arcseconds per QGTe
  x = int(pos.lon / ARCSEC_PER_QUARTER_GLOBE_TILE) & (511);
  z = GetVerticalQgtNumber(pos.lat);
  return;
}
//-----------------------------------------------------------------------------
//  Request the QGT indices
//-----------------------------------------------------------------------------
void IndicesInQGT (GroundSpot &gsp)
{ // X-index is simple calculate of longitude by arcseconds per QGTe
  gsp.qx = int(gsp.lon / ARCSEC_PER_QUARTER_GLOBE_TILE) & (511);
  gsp.qz = GetVerticalQgtNumber(gsp.lat);
  return;
}
//-----------------------------------------------------------------------------
//	Return QGT mid point radius
//-----------------------------------------------------------------------------
void MidRadius(SVector &v, U_INT qz)
{	U_INT No	= (qz >= 256)?(qz - 256):(255 - qz);
	v.x				= TC_ARCS_PER_QGT * 0.5;
	v.y				= qgt_latitude[No].vArcs;
	v.z				= 0;
	return;
}
//-----------------------------------------------------------------------------
//	Return QGT top latitude increment
//-----------------------------------------------------------------------------
double LatitudeIncrement(U_INT qz)
{	U_INT   No = (qz >= 256)?(qz - 256):(255 - qz);
	return  qgt_latitude[No].dta;
}
//-----------------------------------------------------------------------------
//  return number of tiles needed to reach vMax in horizontal direction
//  vt is the vertical tile coordinate
//-----------------------------------------------------------------------------
short GetQgtHorizonRange(short vt)
{ int No = (vt >= 256)?(vt - 256):(255 - vt);
  return qgt_latitude[No].hzr; }
//-----------------------------------------------------------------------------
//  return number of tiles needed to reach COVERED_RADIUS in pole direction
//  vt is the vertical tile number
//-----------------------------------------------------------------------------
short GetQgtVertPoleRange(short vt)
{ int No = (vt >= 256)?(vt - 256):(255 - vt);
  return qgt_latitude[No].vpr; }
//-----------------------------------------------------------------------------
//  return number of tiles needed to reach COVERED_RADIUS in equator direction
//  vt is the vertical tile number
//-----------------------------------------------------------------------------
short GetQgtVertEquaRange(short vt)
{ int No = (vt >= 256)?(vt - 256):(255 - vt);
  return qgt_latitude[No].vqr; }
//-----------------------------------------------------------------------------
//  Return South latitude of QGT in arcsec
//-----------------------------------------------------------------------------
double GetQgtSouthLatitude(short cz)
{ int No = (cz >= 256)?(cz - 256):(256 - cz);
  double lat = qgt_latitude[No].lats;
  return (cz >= 256)?(lat):(-lat);  }
//-----------------------------------------------------------------------------
//  Return North latitude in arcsec
//-----------------------------------------------------------------------------
double GetQgtNorthLatitude(short cz)
{ int No = (cz >= 256)?(cz - 255):(255 - cz);
  double lat = qgt_latitude[No].lats;
  return (cz >= 256)?(lat):(-lat);  }
//-----------------------------------------------------------------------------
//  Return QGT midPoint
//-----------------------------------------------------------------------------
void GetQgtMidPoint(int gx,int gz,SPosition &p)
{ double lat  = GetQgtNorthLatitude(gz);
         lat += GetQgtSouthLatitude(gz);
  double lon  = FN_ARCS_FROM_QGT(gx);
         lon += FN_ARCS_FROM_QGT(gx + 1);
  p.lat = 0.5 * lat;
  p.lon = 0.5 * lon;
	p.alt	= 0;
  return;
}
//-----------------------------------------------------------------------------
//	Return QGT key and super tile No
//-----------------------------------------------------------------------------
void GetSuperTileNo(SPosition *P, U_INT *K, U_SHORT *S)
{ U_INT qx, qz;
	IndicesInQGT (*P, qx, qz);
	*K = QGTKEY(qx,qz);
	//--- compute supertile Number ----------------------
	double slat = GetQgtSouthLatitude(qz);
	double dlat = GetLatitudeDelta(qz);
	U_INT tz    = U_INT((P->lat - slat) / dlat);
	//--- Get longitude delta ---------------------------
	double wlon = FN_ARCS_FROM_QGT(qx);
	U_INT tx    = (P->lon - wlon) / TC_ARCS_PER_DET;
	//--- divide each number by 4 -----------------------
	U_INT sx	= tx >> 2;
	U_INT sz  = tz >> 2;
	*S = U_SHORT((sz << 3) | sx);
	return;
}
//-----------------------------------------------------------------------------
//  Return SW corner coordinates for Detail-Tile (ax,az)
//-----------------------------------------------------------------------------
double GetTileSWcorner(U_INT ax,U_INT az,SVector &v)
{ U_INT   tz = az & 31;                             // Detail tile Z-index in QGT
  U_INT   gz = az >> TC_BY32;                       // QGT z index
  double  sl = GetQgtSouthLatitude(gz);             // Base latitude for QGT
  double  dt = GetLatitudeDelta(gz);                // Latitude delta
  v.x = ax * TC_ARCS_PER_DET;                       // Base longitude
  v.y = sl + (tz * dt);                             // Base latitude
  return dt;                                        // return latitude delta
}
//-----------------------------------------------------------------------------
//  Return coordinates for vertex (vx,vz)
//-----------------------------------------------------------------------------
void GetVertexCoordinates(U_INT vx,U_INT vz,SVector &v)
{ v.x = FN_ARCS_FROM_SUB(vx);
  v.y = GetLatitudeArcs (vz);
  return;
}
//-----------------------------------------------------------------------------
//  Compute next vertex key
//-----------------------------------------------------------------------------
U_INT NextVertexKey(U_INT vk,U_INT inc)
{ U_INT nk = (vk + inc);
  return (nk & TC_MAX_KEY); 
}
//-----------------------------------------------------------------------------
//  Return Tile number from latitude
//-----------------------------------------------------------------------------
U_INT GetTileFromLatitude(double lat)
{ int     gz = GetVerticalQgtNumber(lat);           // QGT z index
  double  sl = GetQgtSouthLatitude(gz);             // Base latitude for QGT
  double  dt = GetLatitudeDelta(gz);                // Latitude delta per DET
  int     tz = int(lat - sl) / dt;                  // Tile number
  return (gz << TC_BY32) + tz;
}
//-----------------------------------------------------------------------------
//  return the compensation factor
//-----------------------------------------------------------------------------
double GetCompensationFactor(short cz)
{ int No = (cz >= 256)?(cz - 256):(256 - cz);
  return qgt_latitude[No].cpf;}
//-----------------------------------------------------------------------------
//  return reduction factor
//-----------------------------------------------------------------------------
double GetReductionFactor(U_INT cz)
{ int No = (cz >= 256)?(cz - 256):(256 - cz);
  return qgt_latitude[No].rdf;}

//-----------------------------------------------------------------------------
//  Return compensation factor for the latitude
//-----------------------------------------------------------------------------
void GetLatitudeFactor(double lat,double &rf,double &cp)
{ double rad = FN_RAD_FROM_ARCS(lat);			// DegToRad(lat / 3600);
  rf = cos(rad);
  cp = 1/ rf;
  return;
}
//-----------------------------------------------------------------------------
//  return the maximum inner circle in feet
//-----------------------------------------------------------------------------
float GetMediumCircle(int tz)
{ int No = (tz >= 256)?(tz - 256):(255 - tz);
  return qgt_latitude[No].fmax; }
//-----------------------------------------------------------------------------
//  return the fog density
//-----------------------------------------------------------------------------
float GetFogDensity(int tz)
{ int No = (tz >= 256)?(tz - 256):(255 - tz);
  return qgt_latitude[No].dfog;
}
//-----------------------------------------------------------------------------
//  Compute longitude difference accross 0 meridian in arcsec
//  Both f1 and f2 must be absolute longitudes in arcsec coordinates
//  NOTE: This is working because longitudes laying accross 0 meridien
//        are separated by less than 180 �
//-----------------------------------------------------------------------------
double LongitudeDifference(double f1,double f2)
{ if ((f1 > TC_HALF_WRD_ARCS) && (f2 < TC_HALF_WRD_ARCS))
    return (f1 - f2 - TC_FULL_WRD_ARCS);
  if ((f1 < TC_HALF_WRD_ARCS) && (f2 > TC_HALF_WRD_ARCS))
    return (f1 - f2 + TC_FULL_WRD_ARCS);
  return   (f1 - f2);
}
//-----------------------------------------------------------------------------
//  Compute QGT difference accross 0 meridian in QGT number
//-----------------------------------------------------------------------------
U_INT QgtDifference(U_INT q1,U_INT q2)
{ if ((q1 > 256) && (q2 < 256))
    return (q1 - q2 - 512);
  if ((q1 < 256) && (q2 > 256))
    return (q1 - q2 + 512);
  return   (q1 - q2);
}
//-----------------------------------------------------------------------------
//  Compute longitude addition accross 0 meridian
//  f1 must be an absolute longitude in arcsec coordinate
//  f2 must be an increment in arcsec
//-----------------------------------------------------------------------------
double AddLongitude(double f1,double f2)
{ double lon = f1 + f2;
  if (lon >= TC_FULL_WRD_ARCS) lon -= TC_FULL_WRD_ARCS;
  return lon;
}
//-----------------------------------------------------------------------------
//  Transform right hand radian in left hand degre
//-----------------------------------------------------------------------------
void GetRRtoLDOrientation(SVector &ld)
{	ld.x	= - globals->dang.x;
	ld.y	= - globals->dang.z;
	ld.z	= - globals->dang.y;
	return;
}
//-----------------------------------------------------------------------------
//  Add to position the feet increment in vector
//  pos contains a geop in absolute arcsec
//  v   contains a vector increment in feet
//  - Longitude feet is converted into arcsec with expansion factor depending
//    on latitude, because a feet is more arcsec toward the pole
//-----------------------------------------------------------------------------
void AddToPosition(SPosition &pos,SVector &v)
{ double exf = 1 / GetReductionFactor(pos.lat);  // expansion
  double acx = FN_ARCS_FROM_FEET(v.x) * exf;
  pos.lon    = AddLongitude(pos.lon,acx);
  pos.lat   += FN_ARCS_FROM_FEET(v.y);
  pos.alt   += v.z;
  return;
}

//-----------------------------------------------------------------------------
//  return latitude of vertex Z index
//  TZ is the concatenation of
//  QGT-DET-SUBD
//-----------------------------------------------------------------------------
double GetLatitudeArcs(U_INT vz)
{ U_INT qz    = FN_QGT_FROM_INDX(vz);               // Isolate QGT Z number
  U_INT nz    = (qz >= 256)?(qz - 256):(256 - qz);  // Table entry        
  double bl   =  qgt_latitude[nz].lats;             // Base south latitude arcsec
  double sb   = (vz & TC_DTSUBMOD);                 // Number of subdivisions
  double dt   = (qgt_latitude[nz].dta * sb) / TC_SUBD_PER_QGT;
  double lat  = (qz >= 256)?(dt + bl):(dt - bl);    // Latitude of tz
  return lat;
}
//---------------------------------------------------------------------------------
//  Return in vector the absolute feet coordinates of position, 
//  taking in account the correction factor for X longitude
//  Position must be in absolute arcsec
//---------------------------------------------------------------------------------
void FeetCoordinates(SPosition &pos,SVector &v)
{ double rdf  = GetReductionFactor(pos.lat);
  //------ tx and ty in feet ------------------------------
  v.x = FN_FEET_FROM_ARCS(pos.lon) * rdf;
  v.y = FN_FEET_FROM_ARCS(pos.lat);
  v.z = pos.alt;
  return;
}
//---------------------------------------------------------------------------------
//  Convert vector from arcsecs into feet, 
//  taking in account the correction factor for X longitude
//---------------------------------------------------------------------------------
void FeetCoordinates(SVector &v, double rdf)
{ //------ tx and ty in feet ------------------------------
  v.x = FN_FEET_FROM_ARCS(v.x) * rdf;
  v.y = FN_FEET_FROM_ARCS(v.y);
  return;
}
//---------------------------------------------------------------------------------
//	Compute feet components of vector (to - from) usinf the rdf factor
//---------------------------------------------------------------------------------
SVector		FeetComponents(SPosition &from, SPosition &to, double rdf)
{	SVector R;
	R.x = LongitudeDifference(to.lon,from.lon) * TC_FEET_PER_ARCSEC * rdf;
	R.y = (to.lat - from.lat) * TC_FEET_PER_ARCSEC;
	R.z = (to.alt - from.alt);
	return R;
}
//---------------------------------------------------------------------------------
//	Compute vector director between 2 positions and return spot at ratio R
//---------------------------------------------------------------------------------
SPosition GetAlignedSpot(SPosition &org, SPosition ext, double R)
{	CVector D(LongitudeDifference(ext.lon,org.lon),(ext.lat - org.lat), (ext.alt - org.alt));
	D.Times(R);
	SPosition P;
	P.lon = AddLongitude(org.lon,D.x);
	P.lat = org.lat + D.y;
	P.alt = org.alt + D.z;
	return P;
}
//-----------------------------------------------------------------------------
//  Return Latitude increment per detail tile for this QGT
//-----------------------------------------------------------------------------
double GetLatitudeDelta(U_INT qz)
{ U_INT nz    = (qz >= 256)?(qz - 256):(255 - qz);  // Table entry
  double dta  = qgt_latitude[nz].det;
  return dta;
}
//-----------------------------------------------------------------------------
//	Given geo positions P1 and P2 return angle between
//	P1 and P2 taking P1 as origin
//	Compute angle relative to geographical north
//-----------------------------------------------------------------------------
double GetAngleFromGeoPosition(SPosition &p1,SPosition &p2, double *dist)
{	double dlon = LongitudeDifference(p2.lon,p1.lon);
	double dlat	= p2.lat - p1.lat;
	double rad  = FN_RAD_FROM_ARCS(p2.lat);
	double f1   = FN_FEET_FROM_ARCS(dlat);							// Vertical feet
	double f2   = FN_FEET_FROM_ARCS(dlon) * cos(rad);		// Horizontal feet
	//-- Compute distance in feet --------------------------------------
  *dist = sqrt((f1 * f1) + (f2 * f2));
	//-- Compute angle to target ---------------------------------------
	double alf  = atan2(f2,f1);												// atan2(Y,X)
	double deg  = RadToDeg(alf);
	return deg;
}
//========================================================================
//  Add two positions in arcsec
//========================================================================
void Add2dPosition(SPosition &p1,SPosition &p2, SPosition &r)
{ r.lon = p1.lon + p2.lon;
  if (r.lon >= TC_FULL_WRD_ARCS) r.lon -= TC_FULL_WRD_ARCS;
  if (r.lon <  0)                r.lon += TC_FULL_WRD_ARCS;
  r.lat = p1.lat + p2.lat;
  r.alt = p1.alt + p2.alt;
  return;
}

//========================================================================
//  Add to position the feet increment in vector
//  pos contains a geop in absolute arcsec
//  v   contains a vector increment in feet
//	Use exf as expension factor
//========================================================================
SPosition AddToPositionInFeet(SPosition &pos,SVector &v, double exf)
{ SPosition p;
	double acx = FN_ARCS_FROM_FEET(v.x) * exf;
  p.lon   = AddLongitude(pos.lon,acx);
  p.lat   = pos.lat + FN_ARCS_FROM_FEET(v.y);
  p.alt		= pos.alt + v.z;
  return p;
}
//========================================================================
//  Add to position the feet increment in vector
//  pos contains a geop in absolute arcsec
//  v   contains a vector increment in feet
//	Get expension factor from latitude
//========================================================================
SPosition AddToPositionInFeet(SPosition &pos,SVector &v)
{ SPosition p;
  double rad = FN_RAD_FROM_ARCS(pos.lat);			// DegToRad(lat / 3600);
  double exf = 1/ cos(rad);
	double acx = FN_ARCS_FROM_FEET(v.x) * exf;
  p.lon   = AddLongitude(pos.lon,acx);
  p.lat   = pos.lat + FN_ARCS_FROM_FEET(v.y);
  p.alt		= pos.alt + v.z;
  return p;
}

//========================================================================
// SubtractPositionInFeet computes the vector offset between two globe positions.
//   the result is in feet
//	NOTE: We use the feet expension factor at aircraft position (globals->exf)
//=======================================================================
SVector SubtractPositionInFeet(SPosition &from, SPosition &to)
{ SVector v;
  // Calculate number of arcseconds difference in latitude
  double arcsecLat = to.lat - from.lat;
  v.y = FN_FEET_FROM_ARCS (arcsecLat);
  // Calculate arcsecond difference in longitude
  double arcsecLon = LongitudeDifference(to.lon,from.lon);
  v.x = FN_FEET_FROM_ARCS(arcsecLon) * globals->rdf;				
  // Altitude is already in feet, simply subtract
  v.z = to.alt - from.alt;
  return v;
}
//========================================================================
// Compute distance position in feet, based on reduction factor of 
//   the from position
//
//=======================================================================
double DistancePositionInFeet(SPosition &from, SPosition &to)
{ CVector v;
	//--- Get Expension factor -----------------------
	double rad = FN_RAD_FROM_ARCS(from.lat);			// DegToRad(lat / 3600);
  double rdf = cos(rad);
  // Calculate number of arcseconds difference in latitude
  double arcsecLat = to.lat - from.lat;
  v.y = FN_FEET_FROM_ARCS (arcsecLat);
  // Calculate arcsecond difference in longitude
  double arcsecLon = LongitudeDifference(to.lon,from.lon);
  v.x = FN_FEET_FROM_ARCS(arcsecLon) * rdf;				
  // Altitude is already in feet, simply subtract
  v.z = to.alt - from.alt;
  return v.Length();
}
//========================================================================
// Return sector of position D relative to positon S
//=======================================================================
U_SHORT GetSectorNumber(SPosition &S,SPosition &D)
{	return U_SHORT(DistancePositionInFeet(S,D)) / 100; }

//========================================================================
// SubtractPositionInArcs computes the vector offset between two globe positions.
//   the result is in arcsec
//=======================================================================
SVector SubtractPositionInArcs(SPosition &from, SPosition &to)
{ SVector v;
  // Calculate number of arcseconds difference in latitude
  v.y = to.lat - from.lat;
  // Calculate arcsecond difference in longitude
  v.x = LongitudeDifference(to.lon,from.lon);
  // Altitude is  in feet, simply subtract -----
  v.z = to.alt - from.alt;
  return v;
}
//========================================================================
// SubtractPositionInArcs computes the vector offset between two globe positions.
//   the result is in arcsec
//=======================================================================
SVector   SubtractFromPositionInArcs(SPosition &from, CVector &to)
{	SVector v;
	v.y = to.y - from.lat;				// Latitude difference
	v.x = LongitudeDifference(to.x,from.lon);
	// Altitude is  in feet, simply subtract -----
  v.z = to.z - from.alt;
  return v;
}
//-----------------------------------------------------------------------------
//  Add miles to position
//  Position should be in absolute world coordinates
//  Horizontal arcsec are inflated by a latitude coefficent when 
//  transformed from miles
//-----------------------------------------------------------------------------
void AddMilesTo(SPosition &pos,double mx,double my)
{ pos.lat   += FN_ARCS_FROM_MILE(my);
  double rad = FN_RAD_FROM_ARCS(pos.lat);				// Latitude in radian
  double cpf = 1 / cos(rad);
  pos.lon   += FN_ARCS_FROM_MILE(mx) * cpf;
  return;
}
//-----------------------------------------------------------------------------
//  Add feet to position
//  Position should be in absolute world coordinates
//  Horizontal arcsec are inflated by a latitude coefficent when 
//  transformed from feet
//-----------------------------------------------------------------------------
void AddFeetTo(SPosition &pos,SVector &v)
{ pos.lat   += FN_ARCS_FROM_FEET(v.y);
  double rad = FN_RAD_FROM_ARCS(pos.lat);				// Latitude in radian
  double cpf = 1 / cos(rad);
  pos.lon   += FN_ARCS_FROM_FEET(v.x) * cpf;
  return;
}
//-----------------------------------------------------------------------------
//  Make an absolute tile key with QGT index[0-512] and Detail index [0-32]
//-----------------------------------------------------------------------------
U_INT AbsoluteTileKey(int qx, int dx)
{ return ((qx << TC_BY32) | dx); }
//-----------------------------------------------------------------------------
//  Compare 2 detail tile horizontal positions given the earth rotondity
//-----------------------------------------------------------------------------
bool TileIsLeft(U_INT k1,U_INT k2)
{ int  dif = k2 - k1;
  if (dif > +TC_HALF_WRD_TILE)  dif -= TC_FULL_WRD_TILE;
  if (dif < -TC_HALF_WRD_TILE)  dif += TC_FULL_WRD_TILE;
  return (dif > 0);
}
//-----------------------------------------------------------------------
//  Check for Point P in triangle ABC
//  Use P = A + U*(C-A) + V*(B-A)
//  p, b and c must already be relatives to A 
//  p = P-A
//  b = B-A
//  c = C-A
//  n will receive the cross product (norm) of b X c
//-----------------------------------------------------------------------
bool PointInTriangle(CVector &p,CVector &a,CVector &b,CVector &c,CVector &n)
{ double dot00 = c.DotProd2D(c);
  double dot01 = c.DotProd2D(b);
  double dot02 = c.DotProd2D(p);
  double dot11 = b.DotProd2D(b);
  double dot12 = b.DotProd2D(p);
  //-----------------------------------------------------
  double inv  = 1 / ((dot00 * dot11) -(dot01 * dot01));
  double u    = ((dot11 * dot02) - (dot01 * dot12)) * inv;
  double v    = ((dot00 * dot12) - (dot01 * dot02)) * inv;
  //-----------------------------------------------------
  p.z = a.z;
  if ((u < 0) || (v < 0) || ((u + v) > 1))  return false;
  //---Extrapole elevation from A,B,C using U,V ---------
  p.z = a.z + (u * c.z) + v * (b.z);
  //---Compute normal at this point ---------------------
  n.CrossProduct(b,c);                  // Get z positive
  n.Normalize();
  return true;
}
//-----------------------------------------------------------------------------
//  Compute flat distance in nautical miles from aircraft 
//  position (pos) to destination position(to). 
//  1 N mile => 1 minute of arc
//  Vertical and horizontal distances are stored as integer and scaled by a 
//  factor 128 for better precision in drawing the vactor map.
//  The short int allows for a +/-256 miles capacity with this factor
//-----------------------------------------------------------------------------
float GetRealFlatDistance(CmHead *obj)
{   SPosition pos = globals->geop;														// Aircraft position
	  SPosition *To = obj->ObjPosition();
    double disLat = (To->lat - pos.lat) / 60.0;								// Lattitude Distance in nm
    double difLon = LongitudeDifference(To->lon,pos.lon);			// Longitude difference in arcsec
    double disLon = obj->GetNmFactor() * difLon;              // Compute x component
    obj->SetDistLon(short(disLon * 128));                     // Store longitude component scaled by 128
    obj->SetDistLat(short(disLat * 128));                     // Store latitude component scaled by 128
		float  sq     = float((disLon * disLon) + (disLat * disLat));
    float  ds     = SquareRootFloat(sq);                      // Return real distance
		obj->SetNmiles(ds);
		obj->SetFeet(FN_FEET_FROM_MILE(ds));
		return ds;
}
//-----------------------------------------------------------------------------
//  Compute flat distance in nautical miles from aircraft 
//  position (pos) to destination position(to). Store distance in obj
//  1 N mile => 1 minute of arc
//  Vertical and horizontal distances are stored as integer and scaled by a 
//  factor 128 for better precision in drawing the vactor map.
//  The short int allows for a +/-256 miles capacity with this factor
//	NOTE: The NmFactor is divided by 60 to convert arsec in nmiles
//-----------------------------------------------------------------------------
float GetFlatDistance(SPosition *To)
{   SPosition pos = globals->geop;															// Aircraft position  
		double	disLat	= (To->lat - pos.lat) / 60.0;								// Lattitude Distance in nm
    double	difLon	= LongitudeDifference(To->lon,pos.lon);			// Longitude difference in arcsec
		double  lr			= FN_RAD_FROM_ARCS(pos.lat);								// Latitude in radian
		double  factor	= cos(lr) / 60;															// 1 nm at latitude lr
    double	disLon	= factor * difLon;													// Reduce x component
		double	sq			= ((disLon * disLon) + (disLat * disLat));  // squarred distance
		return   SquareRootFloat(sq);
}
//-----------------------------------------------------------------------------
//  Compute flat distance in nautical miles from aircraft 
//  position (pos) to destination position(to). Store distance in obj
//  1 N mile => 1 minute of arc
//  Vertical and horizontal distances are stored as integer and scaled by a 
//  factor 128 for better precision in drawing the vactor map.
//  The short int allows for a +/-256 miles capacity with this factor
//	NOTE: rdf is the reduction factor to use for longitude component
//-----------------------------------------------------------------------------
double GetFlatDistanceInMiles(CmHead *obj)
{   SPosition *F = &globals->geop;
	  SPosition *T =  obj->ObjPosition();
    double disLat = (T->lat - F->lat) / 60.0;									// Lattitude Distance in nm
    double difLon = LongitudeDifference(T->lon,F->lon);				// Longitude difference in arcsec
		double  lr			= FN_RAD_FROM_ARCS(F->lat);								// Latitude in radian
		double  rdf			= cos(lr) / 60;														// 1 nm at latitude lr
    double disLon   = rdf * difLon;														// Compute x component
    obj->SetDistLon(short(disLon * 128));                     // Store longitude component scaled by 128
    obj->SetDistLat(short(disLat * 128));                     // Store latitude  component scaled by 128
    return ((disLon * disLon) + (disLat * disLat));           // Return squarred distance
}

//-----------------------------------------------------------------------------
//	Return the maximum longitude in degre
//-----------------------------------------------------------------------------
double MaxDegLongitude(double l1, double l2)
{	double dta = Wrap180(l2 - l1);
	return (dta >= 0)?(l2):(l1);
}
//-----------------------------------------------------------------------------
//	Return the maximum longitude in degre
//-----------------------------------------------------------------------------
double MinDegLongitude(double l1, double l2)
{	double dta = Wrap180(l2 - l1);
	return (dta >= 0)?(l1):(l2);
}
//-----------------------------------------------------------------------------
//	Return the arcseconds from degres
//-----------------------------------------------------------------------------
double LongitudeFromDegre(double d)
{	double a = FN_ARCS_FROM_DEGRE(d);
	if (a < 0)	a += TC_FULL_WRD_ARCS;
	return a;
}
//=========================================================================================
//  Class GroundSpot:   Info on a ground spot
//=========================================================================================
GroundSpot::GroundSpot()
{ qgt = 0;
  pgt = 0;
  Rdy = 0;
  lon = 0;
  lat = 0;
  alt = 0;
}
//------------------------------------------------------------------------
//  Ground Spot with latitude and longitude
//------------------------------------------------------------------------
GroundSpot::GroundSpot(double x,double y)
{ qgt = 0;
  Rdy = 0;
  lon = x;
  lat = y;
  alt = 0;
}
//-------------------------------------------------------------------------
//  Set QGT into ground spot
//-------------------------------------------------------------------------
void GroundSpot::SetQGT(C_QGT *qgt)
{ this->qgt = qgt;
  this->qx  = qgt->GetXkey();
  this->qz  = qgt->GetZkey();
  return;
}
//-------------------------------------------------------------------------
//  Check for Valid QGT
//-------------------------------------------------------------------------
bool GroundSpot::ValidQGT()
{ if (0 == qgt) return false;
  if (tx > 32)  return false;
  if (tz > 32)  return false;
  return true;
}
//-------------------------------------------------------------------------
//  Check for Valid QGT
//	NOTE:  Due to precision error, it may happen that the point is not
//	localized, mostly because it is almost on the frontier between two
//	tiles. So we just let it pass and use the previous altitude value
//	NOTE: To locate position, we need to express it as a relative offset
//			  from the SW corner of the QGT
//-------------------------------------------------------------------------
char  GroundSpot::GetTerrain()
{ Rdy	= 0;
	if (0 == qgt)           return 0;
  if ( qgt->NoQuad())     return 0;
  if (!qgt->GetTileIndices(*this))			gtfo("Position error");
  U_INT No    = FN_DET_FROM_XZ(tx,tz);    
  CmQUAD  *dt = qgt->GetQUAD(No);             // Detail tile QUAD
  Type        = dt->GetGroundType();          // Ground Type
  Quad        = dt;
	ax					= dt->GetTileAX();
	az					= dt->GetTileAZ();
  //------Compute precise elevation at position ---------------
  CVector  p(lon,lat,0);
	qgt->RelativeToBase(p);											// Relative to SW corner
  CmQUAD  *qd = dt->Locate2D(p,qgt);          // Smaller QUAD in Detail Tile
  //----Locate the triangle where p reside --------------------
  if (!qd->PointHeight(p,gNM)) 	p.z = qd->CenterElevation();
  alt         = p.z;
	Rdy	= 1;
  return 1;
}
//-------------------------------------------------------------------------
//  Return terrain altitude from position
//	Works only if the Terrain cache has data about the position
//-------------------------------------------------------------------------
double GroundSpot::GetAltitude(SPosition &p)
{ lat = p.lat;
	lon = p.lon;
	qgt = globals->tcm->GetQGT(p);
	IndicesInQGT(*this);
	GetTerrain();
	return alt;
}
//-------------------------------------------------------------------------
//	Return Texture number in supertile for current detail tile
//-------------------------------------------------------------------------
U_INT	GroundSpot::GetTextureInSUP()
{	if (0 == Quad)	gtfo("Invalid call to GroundSpot");
	U_INT No	= Quad->GetSuperNo();
	U_INT rx  = tx - SuperDT[No].dx;
	U_INT rz  = tz - SuperDT[No].dz;
	return (rz * 4) + rx;
}
//-------------------------------------------------------------------------
//  Check QUAD validity
//-------------------------------------------------------------------------
bool GroundSpot::InvalideQuad()
{	if (0 == qgt)					return true;
	if (qgt->NotReady())	return true;
	return false;
}
//-------------------------------------------------------------------------
//  Check for spot validity
//-------------------------------------------------------------------------
bool GroundSpot::Valid()
{	if (0 == qgt)				return false;
	if (qgt->NoQuad())	return false;
	if (0 == Rdy)				return false;
	return true;
}
//-------------------------------------------------------------------------
//  Compute feet distance to vertex
//-------------------------------------------------------------------------
void GroundSpot::FeetCoordinatesTo(CVertex *vtx,CVector &v)
{	SVector w = vtx->GeoCoordinates(qgt);
	v.x				= TC_FEET_PER_ARCSEC * rdf * LongitudeDifference(w.x,lon);
	v.y				= TC_FEET_PER_ARCSEC * (w.y - lat);
	v.z				= w.z - alt;
	return;
}
//-------------------------------------------------------------------------
//  Return relative TILE indices
//-------------------------------------------------------------------------
CSuperTile *GroundSpot::GetSuperTile()
{ return (qgt)?(qgt->GetSuperTile(tx,tz)):(0);
}
//-------------------------------------------------------------------------
//  Update Altitude above ground
//-------------------------------------------------------------------------
void GroundSpot::UpdateAGL(SPosition &P,double R)
{	lon		= P.lon;
  lat		= P.lat;
  alt		= 0;
	rdf		= R;
  GetTerrain();
	//--- Update AGL and ground plane ---------------------
	agl		=		P.alt - alt;
	return;
}
//-------------------------------------------------------------------------
//  Get Ground info at position
//-------------------------------------------------------------------------
void GroundSpot::GetGroundAt(SPosition &pos)
{	if (0 == qgt) qgt =  globals->tcm->GetQGT(pos);
  if (0 == qgt)         return;
  if (qgt->NoQuad())    return;
  qgt->GetTileIndices(pos,tx,tz);
  //----Access the Super Tile ----------------
  sup     = qgt->GetSuperTile(tx,tz);
  //----Compute location elevation -----------
  U_INT No    = FN_DET_FROM_XZ(tx,tz);           
  CmQUAD *dt  = qgt->GetQUAD(No);
  CVector  p(pos.lon,pos.lat,0);
	qgt->RelativeToBase(p);
  CmQUAD *qd  = dt->Locate2D(p,qgt);
  //----Locate the triangle where p reside ------------
  qd->PointHeight(p,gNM);
  alt     = p.z;
	return;
}
//-------------------------------------------------------------------------
//	Check for same spot
//-------------------------------------------------------------------------
bool GroundSpot::SameSpot(CmQUAD *Q)
{return (Q->GetTileAX() == ax) && (Q->GetTileAZ() == az);}
//=========================================================================================
//	Get a lateral spot 
//	dis = distance from far point
//	dir	= (+1)		Right
//			= (-1)		Left
//=========================================================================================
SPosition	LND_DATA::GetLateralSpot(double dis, double dir)
{ double  R = (dis / RWY_FWD_POINT);
	CVector D(LongitudeDifference(refP.lon,fwdP.lon),(refP.lat - fwdP.lat), (refP.alt - fwdP.alt));
	SPosition Q;
	Q.lon = AddLongitude(fwdP.lon, (D.y * dir));
	Q.lat = fwdP.lat - D.x;
	Q.alt	= fwdP.alt + D.z;
	return Q;
}
//=========================================================================================
//  return rounded value as integer
//=========================================================================================
int GetRounded(float nb)
{ int round = int(nb * 4);
  int value = (round >> 2);
  if (round & 0x02) value++;
  return value;
}
//--------------------------------------------------------------------------
//  Return rounded altitude to 100 feet
//--------------------------------------------------------------------------
double RoundAltitude(double a)
{	int    alt = int(a / 100);           // Integer part
  double rst = fmod(a, 100);           // Fract part
  double rnd = (rst > 50)?(100):(0);
  return (double(alt) * 100) + rnd;	
}
//-----------------------------------------------------------
//	Round altitude to 100 feet
//-----------------------------------------------------------
int  RoundAltitude(int a)
{	int rst = a % 100;
	int ent = a / 100;
	if (rst) ent++;
	return (ent * 100);
}
//=========================================================================================
//  Compute the flag
//  0 Warning flag (no detection)
//  1 To flag
//  2 From flag
//=========================================================================================
int GetFlag(int dta, char pwr)
{ if (0 == pwr)  return VOR_SECTOR_OF;
  if ((dta >= (90 + DEADZONE)) && (dta <= (270 - DEADZONE))) return VOR_SECTOR_FR;
	if ((dta <= (90 - DEADZONE)) || (dta >= (270 + DEADZONE))) return VOR_SECTOR_TO;
	return VOR_SECTOR_OF;
}

//=========================================================================================
//  Normalize the deviation
//  Signe of Deviation give plane position relative to reference direction Ref
//	Positive:		Plane is on left side 
//	Negative:		Plane is on right side 
//=========================================================================================
float GetSector(float dta)
{	if (dta >= 270)		return (360 - dta);			    // sector 4
	if (dta <   90)		return (-dta);				      // Sector 1
	return (dta - 180);	}							            // Sector 2 and 3

//=========================================================================================
//  Compute Waypoint deviation
//  Deviation is the angle between the reference direction (ref)
//  and actual heading (rad).
//  Reference Direction is :
//  -OBS direction when radio is a NAV
//  -ILS direction when radio is a ILS
//  -WPT direction (radial)
//  The flag is either F (from) or T (to) depending on the sector
//  of the NAV where the aircraft is flying. 0 is when no signal is
//  available or aircraft is in the dead angle of the radio
//------------------------------------------------------------------------
//  ref   = reference direction
//  rad   = actual radial where aircraft is lying
//  flag  = receive the flag result
//  pwr   = power indicator (0 = OFF)
//=========================================================================================
float ComputeDeviation(float ref,float rad,U_CHAR *flag, U_CHAR pwr)
{ float dta   = Wrap360(rad - ref);
  float dev   = GetSector(dta);
  int   dtf   = GetRounded(dta);
  *flag       = GetFlag(dtf,pwr);
  return dev;
}
//=========================================================================================
//	check if value A  is in the circular range R
//=========================================================================================
bool InCircularRange(float A, float M, float R)
{ float dta = Wrap180(A - M);
	if (dta < -R)		return false;
	if (dta >	+R)		return false;
	return true;
}

//=========================================================================================
//  Return COAST  index from Quarter Globe Tile indices
//=========================================================================================
U_INT GetSEAindex(U_INT cx,U_INT cz)        
{ U_INT gx = cx >> 1;                                 // X Global TILE
  U_INT gz = cz >> 1;                                 // Z Global TILE
  U_INT indx  = (gx << 16) + gz;                      // Final index
  return indx;
}
//-----------------------------------------------------------------------------
//	Return Detail tile indices in QGT
//	sp = SuperTile No in QGT
//	dn = No of Detail Tile in SuperTile
//	NOTE: Order is from SW to SE then upward
//-----------------------------------------------------------------------------
U_INT  DetailTileIndices(U_INT sp, U_INT dn, U_INT *tx, U_INT *tz)
{	U_INT sz	= sp >> 3;						// SuperTile Z index in QGT
	U_INT sx  = sp & 0x07;					// SuperTile X index in QGT
	U_INT px  = dn >> 2;						// Detail Tile Z index in SuperTile
	U_INT pz  = dn & 0x03;					// Detail Tile X index in SuperTile
	U_INT nz  = (sz << 2) | pz;			// Detail Tile Z index in QGT
	U_INT nx  = (sx << 2) | px;			// Detail Tile X index in QGT
	*tx	= nx;
	*tz = nz;
	return (nx << 16) | nz;
}
//=========================================================================================
//  Vertex maths
//=========================================================================================
//	Return relative longitude in QGT of vertex indice vx
//-----------------------------------------------------------------------------------
double RelativeLongitudeInQGT(U_INT vx)
{	U_INT sub			= FN_SUB_FROM_INDX(vx);							// Number of subdivision in QGT
	return sub * TC_ARCS_PER_SUBD;
}
//-----------------------------------------------------------------------------------
//	Return Absolute longitude of vertex indice vx
//-----------------------------------------------------------------------------------
double AbsoluteLongitude(CVertex &v)
{	U_INT No		= FN_QGT_FROM_INDX(v.keyX());				// QGT No
	return (No * TC_ARCS_PER_QGT) + (v.GetRX());
}
//-----------------------------------------------------------------------------------
//	Return relative latitude in QGT of vertex indice vz
//-----------------------------------------------------------------------------------
double RelativeLatitudeInQGT(U_INT vz)
{	U_INT  No		= FN_QGT_FROM_INDX(vz);										// QGT row number
	U_INT  nz   = (No >= 256)?(No - 256):(255 - No);			// Table entry        
  U_INT sub   = FN_SUB_FROM_INDX(vz);										// Number of subdivisions
  double dt   = qgt_latitude[nz].sub * sub;							// Delta from south border
	return dt;
}
//-----------------------------------------------------------------------------------
//	Return Absolute latitude of vertex indice vz
//-----------------------------------------------------------------------------------
double AsoluteLatitude(CVertex &v)
{	U_INT  No		= FN_QGT_FROM_INDX(v.keyZ());							// QGT row number
	U_INT  nz   = (No >= 256)?(No - 256):(255 - No);			// Table entry        
  double sb   = qgt_latitude[nz].lats;									// South border latitude
	if (No < 256)	sb = -sb;																// Negative in south
	return (qgt_latitude[nz].lats + sb);									// Absolute latitude
}
//==============================================================================
//	Random number about H in the interval [a,b]
//==============================================================================
double RandomCentered(double H, int a, int b)
{	int n = RandomNumber(b - a);
	int m = (a + b) >> 1;								// Interval middle
	return H + m - n;
}
//==============================================================================
// CRandomizer:
//  Produce a random number N in the requested range R every tim T.  The 
//  value N is reached in T secondes from the previous one
//  pd:     the lower bound
//  Ap:     Amplitude
//  pT:     Time constant to reach the next random value
//==============================================================================
CRandomizer::CRandomizer()
{ aval   = 0;
  tval   = 0;
  timr   = 0;
  cTim   = 0;
}
//------------------------------------------------------------------
//  Start a new serie
//------------------------------------------------------------------
void CRandomizer::Set(float db,int mp,float t)
{ cTim  = t;                   // Time constant
  //------ Generate a new target value --------------
  amp     = mp;
  vdeb    = db;
  tval    = db + float(RandomNumber(amp));    // New target
  aval    = db;                               // Both values equal
  timr    = t;                                // arm timer
  dval    = (tval - aval) / cTim;             // Delta per sec
}
//------------------------------------------------------------------
//  Start a new range
//  db = start value
//  mp = amplitude
//------------------------------------------------------------------
void CRandomizer::Range(float db, int mp)
{ amp   = mp;
  vdeb  = db;
  return;
}
//------------------------------------------------------------------
//  Time slice: compute actual value or generate a new number
//------------------------------------------------------------------
float CRandomizer::TimeSlice(float dT)
{ timr += dT;
  if (timr < cTim)  { aval += dval * dT;    return aval;}
  //-- timer elapses: generate a new target --------------
  aval    = tval;                             // Target is reached
  tval    = float(RandomNumber(amp) + vdeb);  // New target
  dval    = (tval - aval) / cTim;             // Delta per second
  timr    = 0;
  return aval;
}
//==============================================================================
// ValGenerator:
//  Produce a value y=f(t) where t is time.
//	1)-One shot Generator is configured by a call to Conf(char M,float T);
//			M = INDN_LINEAR or INDN_EXPONENTIAL to indicate which mode the current
//			value is computed into the time interval T.
//		-Then the valuateur is armed by a call to Set(float TV).  Starting from actual
//			value whatever it is, the target value TV will then be reached at time T. 
//		-Intermediate values during simulation are retrieved by
//			float v = Get() or through TimeSLice() call
//		-After reaching the target value, subsequent calls return TV, until the 
//		generator is rearmed with a different target value.
//		This feature may be used to smooth a given signal  TV by delaying the output
//		by the time constant T.
//	2)- Generator may be used to produce a damped sinusoidal value according to
//			formulea x(t) = A * power(e,-t) cos (2PI*t)  where t is time.
//		- Arming:  StartSin(float A,int T) where A is signal amplitude and T the
//			signal duration
//		- Intermediate values are retrieve by
//			float v = Get() or through TimeSlice() call
//			When value reaches near 0, the Generator stay quiet during a random time 
//			in intervalle T.  Then is it rearmed to fire again
//==============================================================================
ValGenerator::ValGenerator(char md, float tm)
{ Targ  = 0;
  cVal  = 0;
  mode  = md;
  timK  = tm;
}
//-------------------------------------------------------------------
//  Default constructor
//-------------------------------------------------------------------
ValGenerator::ValGenerator()
{ Targ  = 0;
  cVal  = 0;
  mode  = INDN_LINEAR;
  timK  = 1;
}
//-------------------------------------------------------------------
//  Start sinusoid movement
//-------------------------------------------------------------------
void ValGenerator::StartSin(float a,int R)
{	Targ	= a;
	time	= 0;
	mode	= INDN_SINUSOID;
	cVal	= a;
	rand	= 1000 * R;
}
//-------------------------------------------------------------------
//  Time Slice: Compute value
//-------------------------------------------------------------------
float ValGenerator::TimeSlice(float dT)
{ if (INDN_LINEAR == mode)	
  { // Use timK (in seconds) as linear coefficient
	  //	Do  not overshoot target value
		float dta = (dT / timK);
		if (dta > 1)	dta = 1;
		cVal += (Targ - cVal) * dta;
		return cVal;	}

  if (INDN_EXPONENTIAL == mode)
  { // Use timK (in seconds) as exponential coeffiecient
		cVal += (Targ - cVal) * (1.0f - (float)exp(-dT / timK));
		return cVal;
  }
	if (INDN_SINUSOID == mode)
	{	time += dT;
		cVal	= Targ * exp(-time) * cos(TWO_PI * time);
		if (fabs(cVal) > 0.00001F)	return cVal;
		mode	= INDN_IDLE;
		U_INT r = RandomNumber(rand);
		time		= float(r) * 0.001F;
		return cVal;
	}
	if (INDN_IDLE == mode)
	{	time -= dT;
		if (time > 0)		return 0;
		time	= 0;
		mode	= INDN_SINUSOID;
		cVal	= Targ;
		return cVal;
	}
  return Targ;
}
//============================================================================
//  Return left position in Quad
//  Quad is ordered as NE-NW-SW-SE
//============================================================================
float GetLefPos(TC_VTAB *qd)
{ float nw = qd[1].VT_X;
  float sw = qd[2].VT_X;
  return (nw < sw)?(nw):(sw);
}
//============================================================================
//  Return top position in Quad
//  Quad is ordered as NE-NW-SW-SE
//============================================================================
float GetTopPos(TC_VTAB *qd)
{ float ne = qd[0].VT_Y;
  float nw = qd[1].VT_Y;
  return (ne > nw)?(ne):(nw);
}
//============================================================================
//  Return average width
//  Quad is ordered as NE-NW-SW-SE
//============================================================================
float GetWIDTH(TC_VTAB *qd)
{ float w1 = qd[0].VT_X + qd[1].VT_X;
  float w2 = qd[2].VT_X + qd[3].VT_X;
  return (w1 + w2) * 0.5;
}
//============================================================================
//  Return average height
//  Quad is ordered as NE-NW-SW-SE
//============================================================================
float GetHEIGHT(TC_VTAB *qd)
{ float h1 = qd[0].VT_Y + qd[3].VT_Y;
  float h2 = qd[1].VT_Y + qd[2].VT_Y;
  return (h1 + h2) * 0.5;
}
//============================================================================
//	Rotate vertex arround Z axis
//============================================================================
void ZRotate(GN_VTAB &v, double sn, double cn)
{	double x = v.VT_X;
	double y = v.VT_Y;
	double M0	= +cn; 
	double M1 = +sn;
	double M2 = -sn; 
	double M3 = +cn;
  v.VT_X  = ((x * M0) + (y * M2));
	v.VT_Y  = ((x * M1) + (y * M3));
	//--------------------------------
	x	= v.VN_X;
	y = v.VN_Y;
	v.VN_X  = ((x * M0) + (y * M2));
	v.VN_Y  = ((x * M1) + (y * M3));
	return;
}
//============================================================================
//	Horizontal Transformer
//============================================================================
HTransformer::HTransformer(double c,double s,SVector &t, double e)
{	cn	= c;							// Cosinus
	sn	= s;							// Sinus
	T		= A = t;					// Translation
	ex	= ey = e;					// Scale
	//--- Init matrix ----------------
	M0	= +cn;						// L1-C1
	M1	= +sn;						// L1-C2
	M2	= -sn;						// L2-C1
	M3	= +cn;						// L2-C2
}
//---------------------------------------------------------------------
//	Set rotation parameters (a in deg)
//---------------------------------------------------------------------
void HTransformer::SetROT(double a)
{	double R	= DegToRad(a);
	cn				= cos(R);
	sn				= sin(R);
	//--- Init matrix ----------------
	M0	= +cn;						// L1-C1
	M1	= +sn;						// L1-C2
	M2	= -sn;						// L2-C1
	M3	= +cn;						// L2-C2
}
//---------------------------------------------------------------------
//	Transform vertex (Rotate then translate)
//---------------------------------------------------------------------
void HTransformer::ComputeRT(GN_VTAB &src,GN_VTAB *dst)
{ *dst = src;
	//--- Rotate vertex ------------------
	rx = (src.VT_X * M0) + (src.VT_Y * M2);
	ry = (src.VT_X * M1) + (src.VT_Y * M3);
	//--- Translate now -------------------
	dst->VT_X		= (rx + T.x) * ex;
	dst->VT_Y		= (ry + T.y) * ex;
	dst->VT_Z  += T.z;
	dst->VT_Z  *= ex;
	//-------------------------------------
	dst->VN_X = dst->VN_Y = dst->VN_Z = float(0.01);
	return;
}
//---------------------------------------------------------------------
//	Transform vertex (Scale, rotate and Translate)
//---------------------------------------------------------------------
F3_VERTEX *HTransformer::TransformSRT(U_INT n,F3_VERTEX *src,F3_VERTEX *dst)
{	while (n--)
	{	
		//--- Rotate -------------------------------------
		rx = (src->VT_X * M0) + (src->VT_Y * M2);
		ry = (src->VT_X * M1) + (src->VT_Y * M3);
		//--- Scale in x,y ------------------------------
		double sx	= rx * ex;
		double sy	= ry * ey;
		//------------------------------------------------
		dst->VT_X = (sx + T.x);
		dst->VT_Y = (sy + T.y);
		dst->VT_Z = (T.z);

		src++;
		dst++;
	}
	return dst;
}
//==========================================================================
//  Edit Latitude in deg min sec
//==========================================================================
void EditLat2DMS(float lat, char *edt, char opt)
{ const char *pole  = (lat < 0)?("S"):("N");
  long  val   = (lat < 0)?(long(-lat * 100)):(long(lat *100));
  long  deg   =  0;
  long  min   =  0;
  deg   = (val / 360000);
  val   = (val % 360000);
  min   = (val / 6000);
  val   = (val % 6000);
  if (opt)  sprintf_s(edt,31,"Lat: %3u %2u' %2.2f\" %s",int(deg),int(min),(float(val) / 100), pole);
	else			sprintf_s(edt,31,     "%3u %2u' %2.2f\" %s",int(deg),int(min),(float(val) / 100), pole);
	edt[31] = 0;
  return;
}
//==========================================================================
//  Edit Longitude in deg min sec
//==========================================================================
void EditLon2DMS(float lon, char *edt, char opt)
{ if (lon > (180 * 3600)) lon -= (360 * 3600);
  const char *meri  = (lon < 0)?("W"):("E");
  long  val   = (lon < 0)?(long(-lon * 100)):(long(lon * 100));
  long  deg   =  0;
  long  min   =  0;
  deg   = (val / 360000);
  val   = (val % 360000);
  min   = (val / 6000);
  val   = (val % 6000);
  if (opt)	sprintf_s(edt,31,"Lon: %3u %2u' %3.2f\" %s",int(deg),int(min),(float(val) / 100), meri);
	else			sprintf_s(edt,31,     "%3u %2u' %3.2f\" %s",int(deg),int(min),(float(val) / 100), meri);
	edt[31] = 0;
  return;
}

//=======================================================================
//	Write a texture from texture object
//=======================================================================
void WriteTexture(U_INT xob,char *dir, char *name)
{ int			wd,ht = 0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH, &wd);
	glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&ht);
	int     nbp = wd * ht;
  int     dim = nbp* 4;
  U_CHAR *buf = new U_CHAR[dim];
  glBindTexture(GL_TEXTURE_2D,xob);
  glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,buf);
  //----------------------------------------------------------------
  {GLenum e = glGetError ();
   if (e != GL_NO_ERROR) 
    WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  }
  //----------------------------------------------------------
  char	fn[1024];
  _snprintf(fn,1023,"%s/%s.TIF",dir,name);
  CArtParser img(0);
  img.WriteBitmap(FIF_TIFF,fn,wd,ht,buf);
  delete [] buf;
  return;
}
//=======================================================================
//  Gamma transfer
//=======================================================================
void GammaCorrection(U_CHAR *img, U_INT nbp, U_INT *lut)
{	U_CHAR *src = img;
	U_CHAR *dst = src;
	U_INT   dim = nbp;
	for (U_INT k=0; k< dim; k++)
	{	*dst++ = lut[*src++];		// Red chanel
		*dst++ = lut[*src++];		// Green
		*dst++ = lut[*src++];		// Blue
		*dst++ = *src++;				// Alpha
	}
}
//=======================================================================
//	Check for file existence
//=======================================================================
bool DirectoryExists(char* filePath)
{	//This will get the file attributes bitlist of the file
	DWORD fileAtt = GetFileAttributesA(filePath);
	//If an error occurred it will equal to INVALID_FILE_ATTRIBUTES
	if(fileAtt == INVALID_FILE_ATTRIBUTES) return false;
	//--- check for directory ------------------------------
	return ( ( fileAtt & FILE_ATTRIBUTE_DIRECTORY ) != 0 ); 
}
//=======================END OF FILE ======================================================

