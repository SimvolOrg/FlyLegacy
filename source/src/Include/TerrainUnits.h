/*
 * TerrainUnits.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright (c) 2004 Chris Wallace
 * Copyright (c) 2008 Jean Sabatier
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
#ifndef TERRAINUNITS_H
#define TERRAINUINTS_H
//============================================================================
//  GENERAL DEFINITIONS
//============================================================================
//-----FILE FORMATS ---------------------------------------------
#define TC_FILE_PNG  0
#define TC_FILE_JPG  1
#define TC_FILE_BMP  2
#define TC_FILE_TGA  3
#define TC_FILE_TIF  4
//-----PI -------------------------------------------------------
#define PI       (double(SGD_PI))
#define HLF_PI   (double(TC_PI * 0.5))
#define PI_PI    (double(TC_PI * TC_PI))
#define HPI      (double(TC_PI) * 0.5)
#define SIN_T1   (double(4.0f / TC_PI))
#define SIN_T2   (double(4.0f / TC_PI_PI))
//----------------------------------------------------------------
#define SGD_PI                               3.14159265358979323846   /* From M_PI under Linux/X86 */
#define PI                                    double(SGD_PI)
#define TWO_PI                                double(PI * 2.0)
#define HALF_PI                               double(PI / 2.0)
#define QUARTER_PI                            double(PI / 4.0)
#define ONE_OVER_PI                           double(1.0 / PI)
#define ONE_OVER_TWO_PI                       double(1.0 / TWO_PI)
#define DEG2RAD                               double(PI / 180.0)
#define RAD2DEG                               double(180.0 / PI)
//------ILS SLOPE VALUES ---------------------------------------------
#define TANGENT_1DEG                          double(0.01745506492)
#define TANGENT_3DEG                          double(0.05240777928)
#define TANGENT_5DEG                          double(0.08748866352)
#define TANGENT_7DEG                          double(0.12278456090)
//------EDGES INDICES -------------------------------------------
#define TC_NORTH  0           
#define TC_SOUTH  1
#define TC_WEST   2
#define TC_EAST   3
//------DIRECTION VERTICES -----------------------------------------
#define TC_D_NORTH 1
#define TC_D_SOUTH 2
#define TC_D_WEST  4
#define TC_D_EAST  8
#define TC_D_SE (TC_D_SOUTH | TC_D_EAST)
#define TC_D_SW (TC_D_SOUTH | TC_D_WEST)
#define TC_D_NE (TC_D_NORTH | TC_D_EAST)
#define TC_D_NW (TC_D_NORTH | TC_D_WEST)
//------CORNER INDICES-------------------------------------------
#define TC_SWCORNER 0
#define TC_SECORNER 1
#define TC_NECORNER 2
#define TC_NWCORNER 3
//---------------------------------------------------------------
#define TC_HI 0
#define TC_LO 1
//------DETAIL NEIGBOURGS----------------------------------------
#define TC_TILE_A   0               // A -this Tile increment
#define TC_TILE_X   1               // X-Right tile increment
#define TC_TILE_B   2               // B-Bottom tile increment
#define TC_TILE_Y   3               // Y_Corner Tile increment
//-----------------------------------------------------------------
#define TC_ABSMODULO ((1 << 24) - 1)// 24 bits vertex absolute coordinate
#define TC_QGT_DET_MOD ((32 * 512) - 1)
#define TC_WORDMODULO (0x0000FFFF)
#define TC_WORLDDETMOD   (16383)
#define TC_4096MOD  (4095)
#define TC_1024MOD  (1023)          // 1024 vertex per Detail Tile 
#define TC_0512MOD  (511)           // 512 QGT in world
#define TC_128MODULO (127)
#define TC_064MODULO (63)
#define TC_032MODULO (31)
#define TC_016MODULO (15)
#define TC_008MODULO (7)
#define TC_004MODULO (3)
#define TC_001MODULO (1)
#define TC_DTSUBMOD  (32767)        // 32 DT * 1024 SUBDIVISIONS
#define TC_MAX_KEY   int((512 * 32 * 1024) - 1)
//---SHIFT POSITIONS -----------------------------------------------
#define TC_LAST_VTX    (512 << TCBY32768)
#define TC_TILE_MODULO (TC_FULL_WRD_TILE - 1)
//---SHIFT FACTOR ---------------------------------------------------
#define TC_BYWORD     16
#define TCBY32768     15
#define TCBY16384     14
#define TC_BY1024     10
#define TC_BY512       9
#define TC_BY256       8
#define TC_BY128       7
#define TC_BY64        6
#define TC_BY32        5
#define TC_BY16        4           
#define TC_BY08        3
#define TC_BY04        2
#define TC_BY02        1
//---TILE INCREMENT in ABSOLUTE COORNINATE -------------------------
#define TC_TILEINC   (1 << TC_BY1024)
//--QGT STEP ---------------------------------------------------------
#define TC_QT_RDY 0                 // No more action
#define TC_QT_INI 1                 // Init QGT
#define TC_QT_MSH 2                 // Build Tile mesh
#define TC_QT_ELV 3                 // Get Elevation
#define TC_QT_QTR 4                 // Default elevations
#define TC_QT_CNE 5                 // Corner elevation
#define TC_QT_TRN 6                 // TRN file
#define TC_QT_HDT 7                 // <hdtl> elevations
#define TC_QT_SUP 8                 // Finalize Super Tile
#define TC_QT_VRF 9                 // Check Quad integrity
#define TC_QT_WFF 10                // Waiting for file
#define TC_QT_TIL 11								// Detail elevation
#define TC_QT_PCH 12								// Elevation patche
#define TC_QT_SEA 13                // Coast data
#define TC_QT_3DO 14                // 3D objects
#define TC_QT_DEL 15               // Candidate to delete
#define TC_QT_END 16								// Ultimate step
//--------------------------------------------------------------------
#define TC_HH_PER_SEC        (double(1) / double(3600))
//--------------------------------------------------------------------
#define TC_MAX_COORDINATE ((512)<< TCBY32768)
#define TC_MAX_LONGITUDE (360 * 3600) // In ArcSeconde
#define TC_NEARLIMIT      (59 * 3600) // 59° in arcsec
//------GLOBALS PARAMETERS ----------------------------------------------
#define TC_DET_PER_QGT      (32)        // Detail Tiles per Quarter Global Tile
#define TC_SUBD_NUMBER      (1024)
#define TC_SUBD_PER_QGT     (TC_DET_PER_QGT * TC_SUBD_NUMBER)
#define TC_EARTH_RADIUS     double(20888146.31)  // in feet
#define TC_FULL_WRD_MILE    double(360 * 60)
#define TC_FULL_WRD_FEET    double(131244094)
#define TC_HALF_WRD_FEET    (0.5 * TC_FULL_WRD_FEET)
#define TC_FULL_WRD_ARCS    (360 * 3600)
#define TC_HALF_WRD_ARCS    (180 * 3600)
#define TC_FULL_WRD_TILE    (16384)
#define TC_FULL_WRD_SUBD    (TC_FULL_WRD_TILE * TC_SUBD_NUMBER)
#define TC_HALF_WRD_TILE    ( 8192)
//---------------------------------------------------------------------
#define TC_ARCS_PER_DET     (double(TC_FULL_WRD_ARCS) / TC_FULL_WRD_TILE)		// Arcsec per Detail Tile 
#define TC_ARCS_PER_SUP     (double(TC_FULL_WRD_ARCS) / 2048)								// Arcsec per Supertile
#define TC_ARCS_PER_HSP			(double(TC_FULL_WRD_ARCS) / 4096)								// Arcsec per half Supertile
#define TC_ARCS_PER_QGT     (double(TC_FULL_WRD_ARCS) / 512)								// Arcsec per QGT
#define TC_ARCS_PER_GBT     (double(TC_FULL_WRD_ARCS) / 256)								// Arcsec per Globe Tile
#define TC_ARCS_PER_BAND    (double(TC_FULL_WRD_ARCS) /   8)
#define TC_ARCS_PER_SUBD		(double(TC_FULL_WRD_ARCS) / (512 * TC_SUBD_PER_QGT))	// Arcsec per subdivision
//-----------------------------------------------------------------------
#define TC_INT_PER_ARCSEC   (double(1073741824) / TC_FULL_WRD_ARCS)
//-----------------------------------------------------------------------
#define TC_FEET_PER_DET     (double(TC_FULL_WRD_FEET) / TC_FULL_WRD_TILE)
#define TC_FEET_PER_ARCSEC  (double(TC_FULL_WRD_FEET) / TC_FULL_WRD_ARCS)
#define TC_FEET_PER_MILE    (double(TC_FULL_WRD_FEET) / TC_FULL_WRD_MILE)
#define TC_FEET_PER_METER    double(3.2808399)
#define TC_XB_ARCS          ( 45 * 3600)
//=====TRANSFORMATION / CONVERTIONS ====================================
//------PRESURE--------------------------------------------------------
#define FN_INHG_FROM_MBAR(X) (double(X) * 0.029529983071)   
//---------------------------------------------------------------------
#define FN_FEET_FROM_ARCS(X) ((double(X) * TC_FULL_WRD_FEET) / TC_FULL_WRD_ARCS)
#define FN_FEET_FROM_MILE(X) ((double(X) * TC_FULL_WRD_FEET) / TC_FULL_WRD_MILE)
#define FN_FEET_FROM_INCH(X) ((double(X) * 0.083333333333))
#define FN_FEET_FROM_METER(X)((double(X) * 3.2808399))
#define FN_MILE_FROM_FEET(X) ((double(X) * 0.000164))
#define FN_MILE_FROM_METER(X)((double(X) * 0.000539))
//------------------------------------------------------------------------------
#define FN_METRE_FROM_FEET(X)	(double(X) * 0.304799999)
//------------------------------------------------------------------------------
#define FN_SUB_FROM_INDX(X)  (U_INT (X) & (TC_SUBD_PER_QGT - 1))
//------------------------------------------------------------------------------
#define FN_ARCS_FROM_DEGRE(X)		((double (X) * TC_FULL_WRD_ARCS) / 360)
#define FN_ARCS_FROM_FEET(X) ((double(X) * TC_FULL_WRD_ARCS) / TC_FULL_WRD_FEET)
#define FN_ARCS_FROM_MILE(X) ((double(X) * 60))
#define FN_ARCS_FROM_QGT(X)  ((double(X) * TC_FULL_WRD_ARCS) / 512)
#define FN_ARCS_FROM_GBT(X)  ((double(X) * TC_FULL_WRD_ARCS) / 256)
#define FN_ARCS_FROM_SUB(X)  ((double(X) * TC_FULL_WRD_ARCS) / TC_FULL_WRD_SUBD)
//----------------------------------------------------------------------
#define FN_RAD_FROM_ARCS(X)  ( double(X) * (PI / 648000))      
//------------LONGITUDE BAND PARAMETERS --------------------------------
#define FN_ABS_QGT_DET(X)		 (X >> TC_BY1024)
#define FN_QGT_FROM_INDX(X)  ( U_INT(X) >> (TC_BY1024 + TC_BY32))       
#define FN_DET_FROM_INDX(X)  ((U_INT(X) >> TC_BY1024) & 31)
#define FN_DET_FROM_XZ(X,Z)  ((Z << TC_BY32) | X)
#define FN_BAND_FROM_QGT(X)  ( U_INT(X) >> TC_BY64)
#define FN_BAND_FROM_INX(X)  ( U_INT(X) >> (TC_BY1024 + TC_BY32 + TC_BY64))
//---------------------------------------------------------------------
#define TC_BANDMOD   ((64 * 32 * 1024)-1)
//------HORIZON PARAMETERS --------------------------------------------
#define TC_HORIZONMAX       double(32 * TC_FEET_PER_MILE)
//------Detail Tile parameters ----------------------------------------
#define TC_DT_ELV_PER_ST  (TC_DETAIL_PER_SPT + 1)
#define TC_DETAIL_NBR     (TC_DET_PER_QGT * TC_DET_PER_QGT)
#define TC_DETMASK  ((1 << 14) - 1) // Mask for DT absolute coordinate in QTR
#define TC_QGTMASK  (511 << (TC_BY32 + TC_BY1024))
//-------Super Tile Parameters ----------------------------------------
#define TC_SUPERT_PER_QGT  8        // SUPER Tiles per Quarter Global Tile
#define TC_SUPERT_NBR     (TC_SUPERT_PER_QGT * TC_SUPERT_PER_QGT)
#define TC_DETAIL_PER_SPT  4        // Detail Tiles per Super Tile
#define TC_SPTPOSITION     2        // Super Tile index position
#define TC_SPTBORDNBR     16        // Number of border vertices
#define TC_NBPOLARVERT     9        // Number of polar vertices
//-------ELEVATION PARAMETERS -------------------------------------------
#define TC_MAX_ELV_LEV   3          // Maximum elevation level
#define TC_MAX_ELV_SIDE  ((1 << TC_MAX_ELV_LEV) + 1)
#define TC_MAX_ELV_DIM   (TC_MAX_ELV_SIDE * TC_MAX_ELV_SIDE)
//-------TEXTURE PARAMETERS -------------------------------------------
#define TC_MAX_TEX_RES   4          // Texture level
#define TC_TEXTUREMAX   512         // Maximum texture size
#define TC_TEXTMAXDIM   (TC_TEXTUREMAX * TC_TEXTUREMAX)
#define TC_TEXNAMESIZE  12          // Name Size
#define TC_TEXSUPERNBR  16          // Number of Textures per Super Tile
#define TC_TEXMODULO    (15)        // Texture modulo
#define TC_MAXDIVIDE float(1/float(1024))  // Maximum subdivision
#define TC_MAX_PIX_DET    (2048)    // Maximum resolution in coast Tile
#define TC_MIN_PIX_DET    (2)
#define TC_END_PIX_DET    (TC_MAX_PIX_DET - 1)  // Last pixel allowed
#define TC_MAX_PIX_GT     (TC_MAX_PIX_DET * 64) // Maximum pixels in globe tiles
//-----------------------------------------------------
#define TC_INSIDE    (0x80)         // Inside pixel
#define TC_PIXCOL    (0x03)         // Pixel color
//------TEXTURE TYPE ----------------------------------
#define TC_TEXGENER     0x01          // Generic texture
#define TC_TEXRAWTN     0x02          // TRN RAW texture
#define TC_TEXCOAST     0x03          // Coast texture
#define TC_TEXRAWEP     0x04          // Raw from EPD
#define TC_TEXSHARD     0x05          // Shared texture
#define TC_TEXWATER     0x06          // Water texture
#define TC_TEX_END      0x07          // Number of textures
//------TEXTURE FLAGS----------------------------------
#define TC_USRTEX       0x04          // User texture
#define TC_SEALAY       0x10          // Sea  layer
#define TC_NITTEX       0x40          // Night texture
//------TRANSITION TYPE ------------------------------
#define TC_NONTRANSITION 0          // No transition
#define TC_BOTTRANSITION 1          // Bottom only
#define TC_CNRTRANSITION 2          // Corner only
#define TC_BCNTRANSITION 3          // Bottom and corner
#define TC_RGTTRANSITION 4          // Right transition
#define TC_RBTTRANSITION 5          // Right & bottom
#define TC_RCNTRANSITION 6          // Right and corner
#define TC_ALLTRANSITION 7          // All transitions
//------RENDER MODE ----------------------------------
#define TC_OUTER        0           // Far   super tile
#define TC_INNER        1           // Inner super tile
#define TC_POLAR        2           // Polar super tile
//------RESOLUTION PARAMETERS ------------------------
#define TC_LOWRES       0           // Low resolution
#define TC_MEDIUM       1           // Medium resolution
#define TC_HIGHTR       2           // Hight resolution
#define TC_EPDRES       3           // EPD resolution
//-----DIMENSION PARAMETERS ---------------------------
#define TC_MEDIUM_DIM (128)
#define TC_HIGHTR_DIM (256)
//-----BLENDING INDICE --------------------------------
#define TC_BLENDRT      0           // Right blending
#define TC_BLENDBT      4           // Bottom blending
#define TC_BLENDCN      8           // Corner blending
//-----FILE REQUESTS PARAMETERS ----------------------
#define TC_REQ_END    0             // Request terminated
#define TC_REQ_BSY    1             // Request is busy
#define TC_POD_QTR    1             // QTR file request
#define TC_REQ_TRN    2             // TRN File request
#define TC_REQ_SEA    3             // Coast file request
#define TC_SQL_ELV    4             // Elevation from Database
#define TC_SQL_CST    5             // Coast database
//-----TEXTURE STATE --------------------------------------
#define TC_TEX_RDY    0             // Texture ready
#define TC_TEX_LOD    1             // Request texture
#define TC_TEX_ALT    2             // Alternate texture
#define TC_TEX_POP    3             // Swap texture
#define TC_TEX_OBJ    4             // Request Object
#define TC_TEX_INQ    5             // In Queue
#define TC_TEX_NEW    9							// New supertile
//-----CONTOUR STATE parameters --------------------------
#define TC_PIX_ZERO   0             // Value 0
#define TC_PIX_RED    4             // RED value
#define TC_PIX_BLUE   8             // BLUE value
#define TC_PIX_BOTH   12            // Both RED and BLUE
//-----COORDINATE DEFINITION IN VERTEX -------------------
#define TC_COORD_X    0
#define TC_COORD_Y    1
#define TC_COORD_Z    2
#define TC_COORD_S    3
#define TC_COORD_T    4
//----MAKE ABSOLUTE TILE KEY ------------------------------
#define TC_ABSOLUTE_DET(Q,T)  ((Q << TC_BY32) | T)
//---------------------------------------------------------
#define TC_NEXT_INDICE(X)  ((X + 1) & TC_WORLDDETMOD)
#define TC_PREV_INDICE(X)  ((X - 1) & TC_WORLDDETMOD)
//----RUNWAY TEXTURES ------------------------------------
#define TC_TAXI_TEXKEY  (1 << 24)   // Taxiway texture key
//-------RUNWAY ENDS -------------------------------------
#define TC_HI 0
#define TC_LO 1
//-------BIN FILE NORME -----------------------------------
#define TC_BIN_SCALE(x) (float(x)  / 65535)
//---------------------------------------------------------------------
#define SIN_5DEG                             (0.08715574)
#define SIN_3DEG                             (0.05233595)
#define ONE_DEGRE_RADIAN		(double(PI / 180))
//==============================================================================
//  RUNWAY LIGHTS
//==============================================================================
//------PAPI SLOPE VALUES ---------------------------------------------
#define PAPI_LA           double(0.00374086610)   // Tan(3°30)²
#define PAPI_LB           double(0.00306086711)   // Tan(3°10)²
#define PAPI_LC           double(0.00244939314)   // Tan(2°50)²
#define PAPI_LD           double(0.00190627793)   // Tan(2°30)²
//------APAPI SLOPE VALUES ---------------------------------------------
#define APAPI_LA          double(0.00230721178)   // Tan(2°45)²
#define APAPI_LB          double(0.00322443573)   // Tan(3°15)²   
//==============================================================================
//  Temperature
//==============================================================================
#define DEGRE_FROM_FAHRENHEIT(X)  (float ((X - 32)* 5) /9)
#define FAHRENHEIT_FROM_DEGRE(X)  (float (float(9 * X) / 5) + 32)
#define KELVIN_FROM_DEGRE(X)      (float(X) + 273.15f)
#define DEGRE_FROM_KELVIN(X)      (float(X) - 273.15f)
#define FAHRENHEIT_FROM_KELVIN(X) (float((9 * (float(X) -273.15f))/ 5) + 32.0f)
#define KELVIN_FROM_FAHRENHEIT(X) (float((5 * (float(X) - 32.00f))/ 9) + 273.15f)
//=======END OF FILE ===========================================================
#endif  // TERRAINUNITS_H

