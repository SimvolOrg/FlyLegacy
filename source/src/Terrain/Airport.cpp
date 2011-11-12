//=====================================================================================================
//	AIRPORT MANAGEMENT
//=====================================================================================================
/*
 * AIRPORT.cpp
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
///----------------------------------------------------------------------------
//  NOTES ON COORDINATE SYSTEM
//  All coordinates (pavement and runways) are expressed with the following rules:
//  In the files, for any vertex V(x,y,z) the coordinates x,y and z are all in feet, 
//  relatives to the AIRPORT origin O(x,y,z)
//  1) X and Y are transformed in arcsec from Airport origin.
//    For any vertex V(x,y,z) the new coordinates X,Y and Z are
//    X = ARCSEC_PER_FEET(x * cp);
//    Y = ARCSEC_PER_FEET(y);
//    Z = z;
//  2) Distances along X are inflated with a compensation factor (cp) that depends on the 
//    latitude. This is because when going toward the pole, the tile Y dimension
//    (in arcsec) is shrinking.  At rendering time, X and Y  dimensions are translated
//    in feet by different scaling factor to make a square tile of side Y.  In this
//    process, the X dimension is reduced to the Y dimension.  The inflate factor
//    just ensures that distances are correctly set. As all coordinate are relative 
//    to the airport origin, the x value of a vertex is the vertex distance to
//    airport origin.
//  3) At rendering time, a translation from aircraft to object is set.  Then
//     object vertices are rendered.
//=========================================================================================
//=========================================================================================
#include "../Include/Airport.h"
#include "../Include/Taxiway.h"
#include "../Include/Terraintexture.h"
#include "../Include/LightSystem.h"
#include "../Include/FileParser.h"
#include <math.h>
//=============================================================================
extern float *TexRES[];
//=============================================================================
//  Runway distance for PAPI
//=============================================================================
extern double valPAPI[];
extern TC_COLOR ColorTAB[];
//=============================================================================
//  MID POINT DESCRIPTOR
//=============================================================================
struct TC_RMP_DEF {
  U_CHAR segID;                             // Segment identifier
  U_CHAR segTX;                             // Segment texture
  U_INT  segLG;                             // Segment length
  double px;                                // X coordinate
  double py;                                // Y coordinate
  double pz;                                // Z coordinate
};
//=============================================================================
//  Runway marging above threshold
//  This table gives the minimum height in feet above runway threshold.
//  This height is respected by the langing systems (ILS , PAPI, etc) by
//  computing a landing point at D feet inside the runway, 
//  assuming a 3° glide slope
//  H/D = tan(3°)  thus D = H / tan(3°) = h * cot(3°)
//=============================================================================
#define TC_COT_3DEG double(19.081136687728)           // cotangent 3°
//-----------------------------------------------------------------------------
double rwyMARGE[] = 
{                 0,                        // Code 0 Not existing
  (15 * TC_COT_3DEG),                       // Code 1 runway <  800m
  (22 * TC_COT_3DEG),                       // Code 2 runway < 1200m
  (25 * TC_COT_3DEG),                       // Code 3 runway < 1800m
  (30 * TC_COT_3DEG),                       // Code 4 runway > 1800m
};
//=============================================================================
//  Runway altitude above threshold
//=============================================================================
double rwyATHR[] = {
  0,                  // 0 not existing
  15,                 // 1 runway < 800m
  22,                 // 2 runway < 1200m
  25,                 // 3 runway < 1800m
  30,                 // 4 runway > 1800m
};
//=============================================================================
//  Table of segment name
//=============================================================================
//=============================================================================
//  STACK OF RUNWAY SEGMENTS.
//  This is where the final segment s of the runway are build
//=============================================================================
RWY_EPF rwyRLP[32];
//=============================================================================
//  RUNWAY MODEL
//  This table guides the generation process of runway segment
//  It describes a complete PAVED RUNWAY
//  NOTE: If the number of item is changed, 
//      TC_MIDSEGTABLE   must be set to the MID segment index
//      TC_DISSEGTABLE   must be set to the LDP segment index
//=============================================================================
#define TC_HD1SEGINDEX  0
#define TC_HD2SEGINDEX  1
#define TC_MIDSEGINDEX 19
#define TC_LD2SEGINDEX 36
#define TC_LD1SEGINDEX 37
//-----------------------------------------------------------------------------
TC_RSEG_DESC PavedRWY_MODEL[] = {
  {TC_SG_HD1,TC_GEN_DSP,'1',0,  0},           //  0 Hi Displaced threshold (optional)
  {TC_SG_HD2,TC_GEN_DSP,'6',0,  0},           //  1 Hi Displaced Part 2
  {TC_SG_HTR,TC_GEN_ALW,'2',0,140},           //  2 Hi Threshold    (140 feet)
  {TC_SG_FIL,TC_GEN_ALW,'2',0, 60},           //  3 Filler          ( 60 feet)
  {TC_SG_HLT,TC_GEN_ALW,'2',0,100},           //  4 Hi Letter       (100 feet)
  {TC_SG_FIL,TC_GEN_ALW,'2',0, 50},           //  5 Filler          ( 50 feet)
  {TC_SG_HNB,TC_GEN_ALW,'2',0,100},           //  6 Hi Number       (100 feet)
  {TC_SG_LIN,TC_GEN_ALW,'3',0, 50},           //  7 Center marks    (100 feet)
  {TC_SG_MK5,TC_GEN_LIM,'4',0,100},           //  8  500 feet mark  (100 feet)
  {TC_SG_LIN,TC_GEN_LIM,'3',0,400},           //  9 Center marks    (400 feet)
  {TC_SG_MK0,TC_GEN_LIM,'5',0,100},           // 10 1000 feet mark  (100 feet)
  {TC_SG_LIN,TC_GEN_LIM,'3',0,400},           // 11 Center marks    (400 feet)
  {TC_SG_MK5,TC_GEN_LIM,'4',0,100},           // 12 1500 feet mark  (100 feet)
  {TC_SG_LIN,TC_GEN_LIM,'3',0,400},           // 13 Center marks    (400 feet)
  {TC_SG_MK0,TC_GEN_LIM,'5',0,100},           // 14 2000 feet mark  (100 feet)
  {TC_SG_LIN,TC_GEN_LIM,'3',0,400},           // 15 Center marks    (400 feet)
  {TC_SG_MK5,TC_GEN_LIM,'4',0,100},           // 16 2500 feet mark  (100 feet)
  {TC_SG_LIN,TC_GEN_LIM,'3',0,400},           // 17 Center marks    (400 feet)
  {TC_SG_MK0,TC_GEN_LIM,'5',0,100},           // 18 3000 feet mark  (100 feet)
  {TC_SG_MID,TC_GEN_MID,'3',0,  0},           // 19 Middle segment  (xxx feet)
  {TC_SG_MK0,TC_GEN_ALW,'5',0,100},           // 20 3000 feet mark  (100 feet)
  {TC_SG_LIN,TC_GEN_ALW,'3',0,400},           // 21 Center marks    (400 feet)
  {TC_SG_MK5,TC_GEN_ALW,'4',0,100},           // 22 2500 feet marks (100 feet)
  {TC_SG_LIN,TC_GEN_ALW,'3',0,400},           // 23 Center marks    (400 feet)
  {TC_SG_MK0,TC_GEN_ALW,'5',0,100},           // 24 2000 feet mark  (100 feet)
  {TC_SG_LIN,TC_GEN_ALW,'3',0,400},           // 25 Center marks    (400 feet)
  {TC_SG_MK5,TC_GEN_ALW,'4',0,100},           // 26 1500 feet mark  (100 feet)
  {TC_SG_LIN,TC_GEN_ALW,'3',0,400},           // 27 Center marks    (400 feet)
  {TC_SG_MK0,TC_GEN_ALW,'5',0,100},           // 28 1000 feet mark  (100 feet)
  {TC_SG_LIN,TC_GEN_ALW,'3',0,400},           // 29 Center marks    (400 feet)
  {TC_SG_MK5,TC_GEN_ALW,'4',0,100},           // 30  500 feet mark  (100 feet)
  {TC_SG_LIN,TC_GEN_ALW,'3',0,100},           // 31 Center marks    (100 feet)
  {TC_SG_LNB,TC_GEN_ALW,'2',0,100},           // 32 Lo Number       (100 feet)
  {TC_SG_LLT,TC_GEN_ALW,'2',0,100},           // 33 Lo Letter       (100 feet)
  {TC_SG_FIL,TC_GEN_ALW,'2',0, 60},           // 34 Filler          ( 60 feet)
  {TC_SG_LTR,TC_GEN_ALW,'2',0,140},           // 35 Lo Threshold    (140 feet)
  {TC_SG_LD2,TC_GEN_DSP,'7',0,  0},           // 36 Lo Displaced threshold (optional)
  {TC_SG_LD1,TC_GEN_DSP,'8',0,  0},           // 37 Lo Displaced threshold (optional)
  {TC_SG_END,TC_GEN_END, 0 ,0,  0},

};
//=============================================================================
//  Model for other Runway (non paved)
//  NOTE Displaced threshold are ignored until we encounter a case
//=============================================================================
TC_RSEG_DESC OtherRWY_MODEL[] = {
  {TC_SG_FIL,TC_GEN_ALW,'*',0, 100},           // 01 Filler          ( 100 feet)
  {TC_SG_MID,TC_GEN_MID,'*',0,   0},           // 02 Middle segment  (xxx feet)
  {TC_SG_FIL,TC_GEN_ALW,'*',0, 100},           // 03 Filler          ( 100 feet)
  {TC_SG_END,TC_GEN_END, 0 ,0,  0},
};
//=============================================================================
//  LEFT NUMBER POSITION for each side
//=============================================================================
SVector RwyNPOS[]= {
  {-0.50,+0.50},                    // Hi pos left
  {+0.15,-0.15},                    // Hi pos right
  {+0.50,-0.50},                    // Lo pos left
  {-0.15,+0.15},                    // Lo pos right
};
//=============================================================================
//  THRESHOLD BAND position
//=============================================================================
SVector RwyBPOS[] = {
  {-1.0,+1.0},                      // Hi pos left
  {+1.0,-1.0},                      // Hi pos right
  {+1.0,-1.0},                      // Lo pos left
  {-1.0,+1.0},                      // Lo pos right
};
//=============================================================================
// Mid point table for cuting runway into segments
//=============================================================================
TC_RMP_DEF RwyMID[128];

//=============================================================================
//  Runway Type
//=============================================================================
U_CHAR RwyTYP[] = {
  0,                            // 0 Unknown
  TC_RWY_PAVED,                            // 1 GROUND_CONCRETE
  TC_RWY_PAVED,                            // 2 GROUND_ASPHALT
  TC_RWY_OTHER,                            // 3 GROUND_TURF
  TC_RWY_OTHER,                            // 4 GROUND_DIRT
  TC_RWY_OTHER,                            // 5 GROUND_GRAVEL
  TC_RWY_OTHER,                            // 6 GROUND_METAL
  TC_RWY_OTHER,                            // 7 GROUND_SAND
  TC_RWY_OTHER,                            // 8 GROUND_WOOD
  TC_RWY_OTHER,                            // 9 GROUND_WATER
  TC_RWY_OTHER,                            // 10 GROUND_MATS
  TC_RWY_OTHER,                            // 11 GROUND_SNOW
  TC_RWY_OTHER,                            // 12 GROUND_ICE
  TC_RWY_OTHER,                            // 13 GROUND_GROOVED
  TC_RWY_OTHER,                            // 14 GROUND_TREATED
};
//=============================================================================
//  Texture relocation table for each individual segment of a non paved runway
//  The runway are grouped into one texture in the following order
//  (from SW corner)
//       
//=============================================================================
float RwyOTHR[] = {
  0,                            // 0 Unknown
  0,                            // 1 GROUND_CONCRETE
  0,                            // 2 GROUND_ASPHALT
  2,                            // 3 GROUND_TURF
  1,                            // 4 GROUND_DIRT
  4,                            // 5 GROUND_GRAVEL
  0,                            // 6 GROUND_METAL
  3,                            // 7 GROUND_SAND
  5,                            // 8 GROUND_WOOD
  0,                            // 9 GROUND_WATER
  0,                            // 10 GROUND_MATS
  0,                            // 11 GROUND_SNOW
  0,                            // 12 GROUND_ICE
  0,                            // 13 GROUND_GROOVED
  0,                            // 14 GROUND_TREATED
};

//=============================================================================
//  Beacon color 1
//=============================================================================
U_CHAR  LiteBC1[] = {
  TC_WHI_LITE,                  // 0 Unknown=> Flash WHITE
  0,                            // 1 No beacon
  TC_GRN_LITE,                  // 2 Flash-GREEN
  TC_YEL_LITE,                  // 3 Flash-YELLOW
  TC_GRN_LITE,                  // 4 Flash-GREEN-YELLOW
  TC_GRN_LITE,                  // 5 SPLIT-FLASH-GREEN
  TC_YEL_LITE,                  // 6 YELLOW
  TC_GRN_LITE,                  // 7 GREEN
};
//=============================================================================
//  Beacon color 2
//=============================================================================
U_CHAR  LiteBC2[] = {
  0,                            // 0 Unknown=> Flash WHITE
  0,                            // 1 No beacon
  0,                            // 2 Flash-GREEN
  0,                            // 3 Flash-YELLOW
  TC_YEL_LITE,                  // 4 Flash-GREEN-YELLOW
  0,                            // 5 SPLIT-FLASH-GREEN
  TC_YEL_LITE,                  // 6 YELLOW
  TC_GRN_LITE,                  // 7 GREEN
};
//=============================================================================
//  Center light model definition
//  Number of objects
//  space per light
//  nOFS
//  wOFS
//  type
//  color 1
//  color 2
//  color 3
// ============================================================================
//  Seg1:  300m ( 900 feet) of RED LIGHT = 9 Double dual light 50 feet apart
//  Seg2:  600m (1800 feet) of Alternate RED-WHITE = 18 double dual light
//  Seg3:  xxx m of omni lights
//=============================================================================
//---Center Light model descriptor ---(50 feet appart) -----------------------------
LITE_MODEL lmDUAL1  = { 9, 50, 0, 0, LS_1122_LITE,TC_RED_LITE,TC_WHI_LITE,TC_FLR_TEX};
LITE_MODEL lmDUAL2  = {18, 50, 0, 0, LS_2111_LITE,TC_WHI_LITE,TC_RED_LITE,TC_FLR_TEX};

LITE_MODEL lmOMNID  = { 0, 50, 0, 0, LS_OMNI_LITE,TC_WHI_LITE,TC_WHI_LITE,TC_FLR_TEX};
LITE_MODEL lmDUAL3  = {18, 50, 0, 0, LS_2111_LITE,TC_WHI_LITE,TC_RED_LITE,TC_FLR_TEX};
LITE_MODEL lmDUAL4  = { 9, 50, 0, 0, LS_1122_LITE,TC_RED_LITE,TC_WHI_LITE,TC_FLR_TEX};
//----Edge light model descriptor----(100 feet appart)------------------------------
LITE_MODEL lmEDGE1  = {18,100, 0, 0, LS_OMNI_LITE,TC_WHI_LITE,          0,TC_WHI_TEX};
LITE_MODEL lmEDGE2  = {18,100, 0, 0, LS_OMNI_LITE,TC_YEL_LITE,          0,TC_WHI_TEX};
//----Threshold light model descriptor -(10 feet appart)----------------------------
LITE_MODEL lmTBARS  = { 0,   5, 0, 0, LS_BAR2_LITE,           0,          0,TC_FLR_TEX};
LITE_MODEL lmTDZNE  = { 3,   5, 0, 0, LS_BAR2_LITE,           0,TC_WHI_LITE,TC_FLR_TEX};  
//----ODALS flash rail --------------------------------------------------------------
LITE_MODEL lmODALS  = { 5, -270, -270, 0, LS_STRB_LITE,TC_WHI_LITE,       0,TC_FLR_TEX};
LITE_MODEL lmODALN  = { 5, +270, +270, 0, LS_STRB_LITE,TC_WHI_LITE,       0,TC_FLR_TEX};
//----SARL  system  -----------------------------------------------------------------
LITE_MODEL lmSALR3  = { 3,    3,    0,  6, LS_BAR2_LITE,      0,TC_WHI_LITE,TC_FLR_TEX};
LITE_MODEL lmSALR5  = { 5,  4.5,    0, 30, LS_BAR2_LITE,      0,TC_WHI_LITE,TC_FLR_TEX};
LITE_MODEL lmSALRF  = { 5, -270,    0,  0, LS_STRB_LITE,TC_WHI_LITE,      0,TC_FLR_TEX};
//---ALSF1-2 system ---------------------------------------------------------------------
LITE_MODEL lmALSFa = { 3,    3,    0,  6, LS_BAR2_LITE,      0,TC_WHI_LITE,TC_FLR_TEX};
LITE_MODEL lmALSFb = { 8,  4.5,    0, 45, LS_BAR2_LITE,      0,TC_WHI_LITE,TC_FLR_TEX};
LITE_MODEL lmALSFc = {14, -100,    0,  0, LS_STRB_LITE,TC_WHI_LITE,      0,TC_FLR_TEX};
LITE_MODEL lmALSFd = { 3,  4.5,    0, 30, LS_BAR2_LITE,      0,TC_RED_LITE,TC_FLR_TEX};
LITE_MODEL lmALSFe = { 4,  4.5,    0, 26, LS_BAR2_LITE,      0,TC_WHI_LITE,TC_FLR_TEX};
LITE_MODEL lmALSFf = {14, -100,    0,  0, LS_STRB_LITE,TC_WHI_LITE,      0,TC_FLR_TEX};
//---REIL ------------------------------------------------------------------------------
LITE_MODEL lmREILa = { 2,    0,    0, 40, LS_FBAR_LITE,      0,TC_WHI_LITE,TC_FLR_TEX};
//---PAPI ------------------------------------------------------------------------------
LITE_MODEL lmPAPI4 = { 4,   27,  170,  0, LS_PAPI_LITE,      0,          0,TC_FLR_TEX};
LITE_MODEL lmPAPI2 = { 2,   18,  170,  0, LS_PAPI_LITE,      0,          0,TC_FLR_TEX};
//=============================================================================
//  Number of lights in threshold bar according to runway width for non precision runways
//  Same for Wing BAR
//=============================================================================
U_CHAR LiteNPR[] = {
  0,                                // Code 0 W < 10m ( 30 feet)
  2,                                // Code 1 W < 15m ( 45 feet)
  3,                                // Code 2 W < 24m ( 72 feet)
  4,                                // Code 3 W < 36m (108 feet)
  4,                                // Code 4 W < 52m (156 feet)
  5,                                // Code 5 W > 52m (xxxx    )
};
//=============================================================================
//  Light color 1 for threshold bar (inside light)
//=============================================================================
U_CHAR LiteTC1[] = {
  0,                                // 0 No start- no end
  0,                                // 1 Start only
  TC_RED_LITE,                      // 2 End only
  TC_RED_LITE,                      // 3 both end
  TC_RED_LITE,                      // 4 Inop
};
//=============================================================================
//  Light color 2 for threshold bar (outside light)
//=============================================================================
U_CHAR LiteTC2[] = {
  0,                                // 0 No start- no end
  TC_GRN_LITE,                      // 1 Start only
  0,                                // 2 End only
  TC_GRN_LITE,                      // 3 both end
  TC_RED_LITE,                      // 4 Inop
};
//=============================================================================
//  THRESHOLD MARK POLYGONS
//  Painted bands are designed as polygons.
//  -Coordinates are in feet
//  -Origin for the Hi END is the southwest corner of the HTR segment
//	The definition is organized for TRIANGLE_STRIP
//=============================================================================
//=============================================================================
//  VBO for Bands and letters
//=============================================================================
TC_WORLD vboBUF[] = {
	//--- White bands -------------------------------
	{10,20},{10,120},{ 16,20},{ 16,120},		// OFS 00
  {22,20},{22,120},{ 28,20},{ 28,120},		// OFS 04
  {34,20},{34,120},{ 40,20},{ 40,120},		// OFS 08
  {46,20},{46,120},{ 52,20},{ 52,120},		// OFS 12
  {58,20},{58,120},{ 64,20},{ 64,120},		// OFS 16
  {70,20},{70,120},{ 76,20},{ 76,120},		// OFS 20
  {82,20},{82,120},{ 88,20},{ 88,120},		// OFS 24
  {94,20},{94,120},{100,20},{100,120},		// OFS 28
	//--- Number 0 ----------------------------------
	{ 0,70},{10,80},{50,70},{40,80},				// OFS 32
  { 0,70},{10,70},{ 0,10},{10,10},				// OFS 36
  {40,70},{50,70},{40,10},{50,10},				// OFS 40
  { 0,10},{50,10},{10, 0},{40, 0},				// OFS 44
	//--- NUMBER 1 ----------------------------------
  {10,60},{20,80},{20,60},{30,80},				// OFS 48
  {20,80},{30,80},{20,10},{30,10},				// OFS 52
  {10,10},{40,10},{10, 0},{40, 0},				// OFS 56
	//--- NUMBER 2 ----------------------------------
  { 0,60},{ 0,70},{10,60},{10,70},				// OFS 60
  { 0,70},{10,80},{50,70},{40,80},				// OFS 64
  {40,70},{50,70},{40,50},{50,40},				// OFS 68
  {40,50},{50,40},{ 0,20},{10,10},				// OFS 72
  { 0,20},{10,10},{ 0, 0},{10, 0},				// OFS 76
  {10,10},{50,10},{10, 0},{50, 0},				// OFS 80
	//--- NUMBER 3 ----------------------------------
  { 0,60},{ 0,70},{10,60},{10,70},				// OFS 84
  { 0,70},{10,80},{50,70},{40,80},				// OFS 88
  {40,70},{50,70},{40,50},{50,50},				// OFS 92
  {40,50},{50,50},{30,40},{40,40},				// OFS 96
  {30,40},{40,40},{40,30},{50,30},				// OFS 100
  {40,30},{50,30},{40,10},{50,10},				// OFS 104
  { 0,10},{50,10},{10, 0},{40, 0},				// OFS 108
  { 0,20},{10,20},{ 0,10},{10,10},				// OFS 112
	//--- NUMBER 4 -----------------------------------
  { 0,30},{30,80},{ 0,20},{40,80},				// OFS 116
  { 0,30},{50,30},{ 0,20},{50,20},				// OFS 120
  {30,40},{40,40},{30, 0},{40, 0},				// OFS 124
	//--- NUMBER 5 -----------------------------------
  { 0,80},{40,80},{10,70},{40,70},				// OFS 128
  { 0,80},{10,70},{ 0,40},{10,50},				// OFS 132
  { 0,40},{10,50},{50,40},{40,50},				// OFS 136
  {40,40},{50,40},{40,10},{50,10},				// OFS 140
  { 0,10},{50,10},{10, 0},{40, 0},				// OFS 144
  { 0,20},{10,20},{ 0,10},{10,10},				// OFS 148
	//--- NUMBER 6 -----------------------------------
  {  0,70},{10,80},{50,70},{40,80},				// OFS 152
  {  0,70},{10,70},{ 0,10},{10,10},				// OFS 156
  { 40,70},{50,70},{40,60},{50,60},				// OFS 160
  {10,50},{40,50},{10,40},{50,40},				// OFS 164
  {40,40},{50,40},{40,10},{50,10},				// OFS 168
  { 0,10},{50,10},{10, 0},{40, 0},				// OFS 172
	//--- NUMBER 7 -----------------------------------
  { 0,70},{10,70},{ 0,60},{10,60},				// OFS 176
  { 0,70},{ 0,80},{40,70},{40,80},				// OFS 180
  {40,80},{50,80},{10, 0},{20, 0},				// OFS 184
  {10,40},{30,40},{10,20},{30,20},				// OFS 188
	//--- NUMBER 8 -----------------------------------
  { 0,70},{10,80},{50,70},{40,80},				// OFS 192
  { 0,70},{10,70},{ 0,60},{10,60},				// OFS 196
  {40,70},{50,70},{40,60},{50,60},				// OFS 200
  { 0,60},{10,60},{10,50},{20,50},				// OFS 204
  {40,60},{50,60},{30,50},{40,50},				// OFS 208
  {10,50},{40,50},{10,40},{40,40},				// OFS 212
  {10,40},{20,40},{ 0,30},{10,30},				// OFS 216
  {30,40},{40,40},{40,30},{50,30},				// OFS 220
  { 0,30},{10,30},{ 0,10},{10,10},				// OFS 224
  {40,30},{50,30},{40,10},{50,10},				// OFS 228
  { 0,10},{50,10},{10, 0},{40, 0},				// OFS 232
	//--- NUMBER 9 -----------------------------------
  { 0,70},{10,70},{ 0,50},{10,50},				// OFS 236
  { 0,70},{10,80},{50,70},{40,80},				// OFS 240
  {40,70},{50,70},{40,50},{50,50},				// OFS 244
  { 0,50},{50,50},{10,40},{40,40},				// OFS 248
  {40,40},{50,50},{40,10},{50,10},				// OFS 252
  { 0,10},{50,10},{10, 0},{40, 0},				// OFS 256
  { 0,20},{10,20},{ 0,10},{10,10},				// OFS 260
	//--- LETTER L (left)	----------------------------
  {-30,  0},{-20,  0},{-30,-80},{-20,-80},	// OFS 264
  {-20,-70},{ 20,-70},{-20,-80},{ 20,-80},	// OFS 268
	//--- LETTER C (center) ----------------------------
  {-20,  0},{ 10,  0},{-30,-10},{ 20,-10},	// OFS 272
  {-30,-10},{-20,-10},{-30,-70},{-20,-70},	// OFS 276
  {-30,-70},{ 20,-70},{-20,-80},{ 10,-80},	// OFS 280
  { 10,-10},{ 20,-10},{ 10,-20},{ 20,-20},	// OFS 284
  { 10,-60},{ 20,-60},{ 10,-70},{ 20,-70},	// OFS 288
	//--- LETTER R (right) -----------------------------
  {-30,  0},{ 10,  0},{-20,-10},{ 20,-10},	// OFS 292
  {-30,  0},{-20,-10},{-30,-80},{-20,-80},	// OFS 296
  {-20,-40},{ 10,-40},{-20,-50},{ 10,-50},	// OFS 300
  { 10,-10},{ 20,-10},{ 10,-30},{ 20,-30},	// OFS 304
  { 10,-30},{ 20,-30},{  0,-40},{ 10,-40},	// OFS 308
  {  0,-50},{ 10,-50},{ 10,-80},{ 20,-80},	// OFS 312
};
//=============================================================================
//  Offset for NUMBER 0 to 9 and letter L,C,R
//=============================================================================
GLint ofsBD[] = { 0, 4, 8,12,16,20,24,28};
GLint ofsN0[] = {32,36,40,44,};
GLint ofsN1[] = {48,52,56,};
GLint ofsN2[] = {60,64,68,72,76,80};
GLint ofsN3[] = {84,88,92,96,100,104,108,112};
GLint ofsN4[] = {116,120,124};
GLint ofsN5[] = {128,132,136,140,144,148};
GLint ofsN6[] = {152,156,160,164,168,172};
GLint ofsN7[] = {176,180,184,188};
GLint ofsN8[] = {192,196,200,204,208,212,216,220,224,228,232};
GLint ofsN9[] = {236,240,244,248,252,256,260};
GLint ofsLL[] = {264,268};
GLint ofsLC[] = {272,276,280,284,288};
GLint ofsLR[] = {292,296,300,304,308,312};
//----------------------------------------------------------------
GLint nbrPM[] = {  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4, 4, 4, 4};
//=============================================================================
//  Vector to offset list
//=============================================================================
GLint *ofsTAB[] = {
	ofsN0,ofsN1,ofsN2,ofsN3,ofsN4,ofsN5,ofsN6,ofsN7,ofsN8,ofsN9,
	ofsLL,ofsLC,ofsLR	};
//=============================================================================
//  Number of primitives for letters
//=============================================================================
GLint nbrVBO[]	= {
		sizeof(ofsN0) / sizeof(GLint),			// Number 0
		sizeof(ofsN1) / sizeof(GLint),			// Number 1
		sizeof(ofsN2) / sizeof(GLint),			// Number 2
		sizeof(ofsN3) / sizeof(GLint),			// Number 3
		sizeof(ofsN4) / sizeof(GLint),			// Number 4
		sizeof(ofsN5) / sizeof(GLint),			// Number 5
		sizeof(ofsN6) / sizeof(GLint),			// Number 6
		sizeof(ofsN7) / sizeof(GLint),			// NUmber 7
		sizeof(ofsN8) / sizeof(GLint),			// Number 8
		sizeof(ofsN9) / sizeof(GLint),			// Number 9
		sizeof(ofsLL) / sizeof(GLint),			// Letter L
		sizeof(ofsLC) / sizeof(GLint),			// Letter C
		sizeof(ofsLR) / sizeof(GLint),			// Letter R
};
//=========================================================================================
//  Vector table to center light model
//=========================================================================================
CAptObject::ctlFN      CAptObject::ctlVECTOR[] = {
  &CAptObject::CenterLightModel0,                   // 'none'
  &CAptObject::CenterLightModel1,                   // 'WWWW'                   
  &CAptObject::CenterLightModel2,                   // 'RWWW'
  &CAptObject::CenterLightModel3,                   // 'WWRW'
  &CAptObject::CenterLightModel4,                   // 'RWRW'
};
//=========================================================================================
//  Vector table to edge light model
//=========================================================================================
CAptObject::ctlFN      CAptObject::edgVECTOR[] = {
  &CAptObject::EdgeLightModel0,
  &CAptObject::EdgeLightModel1,
  &CAptObject::EdgeLightModel2,
  &CAptObject::EdgeLightModel3,
  &CAptObject::EdgeLightModel4,
};
//=========================================================================================
//=========================================================================================
//  END OF TABLES
//=========================================================================================
//  Destroy Pavement Q
//=========================================================================================
CPaveQ::~CPaveQ()
{ CPaveRWY *pav = Pop();
  while (pav) { delete pav; pav = Pop();}
}
//============================================================================
//
//  Class CPaveRWY to store pavement polygons
//
//============================================================================
CPaveRWY::CPaveRWY(U_INT nb)
{ ptab.SetPolygons(nb);
  Type = PAVE_TYPE_PAVE;
  aUse = PAVE_USE_TAXI;
}
//----------------------------------------------------------------------------
//  Destructor
//----------------------------------------------------------------------------
CPaveRWY::~CPaveRWY()
{ }
//----------------------------------------------------------------------------
//  Set elevation for  vertex k
//----------------------------------------------------------------------------
void CPaveRWY::SetElevation(float e,int k)
{ TC_VTAB *tab = ptab.GetVTAB();
  tab[k].VT_Z = e;
  return;
}
//----------------------------------------------------------------------------
//  Copy the vertices to the destination table
//----------------------------------------------------------------------------
int CPaveRWY::CopyTo(TC_VTAB *buf)
{	int nbc = GetNBVT() * sizeof(TC_VTAB);
  memcpy(buf,GetVTAB(),nbc);
	return GetNBVT();
}
//----------------------------------------------------------------------------
//  Append the source pavement and cut polygon into triangle
//----------------------------------------------------------------------------
void CPaveRWY::AppendAsTriangle(CPaveRWY *pav)
{ TC_VTAB *src = pav->GetVTAB();
  TC_VTAB *dst = ptab.GetVTAB() + ptab.GetNBVT();
  TC_VTAB *org = src++;               // Origin of fan
	if (0 == pav->GetNBVT())		return;
  int      nbt = pav->GetNBVT() - 2;  // Number of triangles
  for (int k = 0; k != nbt; k++)
  { *dst++ = *org;                    // First vertex of triangle
    *dst++ = *src++;              
    *dst++ = *src;
		//======SET TRAP HERE =====================================
     ptab.AddCount(3);                // 3 vertices added
  }
  return;
}

//=========================================================================================
//	CTarmac is a descriptor for drawing one runway
//=========================================================================================
CTarmac::CTarmac(CAptObject *a,CRunway *r)
{	apo		= a;
	rwy		= r;							// Save runway
	xKey	= 0;							// Texture key
	xOBJ	= 0;							// Texture object
	oVBO	= 0;							// VBO object
	flag	= 0;							// Flag
	blnd	= 0;							// Blending
	xIND	= 0;							// Current index
	nVRT	= 0;							// Number of vertices
	nPRM	= 0;							// Number of primitives
	glGenBuffers(1,&oVBO);
}
//-----------------------------------------------------------------------------
//	Destructor
//-----------------------------------------------------------------------------
CTarmac::~CTarmac()
{	if (oVBO)	glDeleteBuffers(1,&oVBO);
  globals->txw->FreeSharedKey(xKey);
}
//-----------------------------------------------------------------------------
//	return vertex slot and reserve room for n
//-----------------------------------------------------------------------------
TC_VTAB *CTarmac::Reserve(int n)
{	if (nVRT > TARMAC_VRT)	gtfo("Tarmac no more Vertex room");
	nPRM++;
	ind[xIND++]		=	nVRT;									// Start index
	TC_VTAB *avt	= tab + nVRT;
	nVRT	+= n;
	return avt;
}
//-----------------------------------------------------------------------------
//	Load VBO
//-----------------------------------------------------------------------------
void CTarmac::LoadVBO()
{	int	dim = nVRT * sizeof(TC_VTAB);
	glBindBuffer(GL_ARRAY_BUFFER,oVBO);
	glBufferData(GL_ARRAY_BUFFER,dim,tab,GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	return;
}
//-----------------------------------------------------------------------------
//	Draw tarmac
//-----------------------------------------------------------------------------
void CTarmac::Draw()
{	if (blnd)  glEnable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, xOBJ);
	glBindBuffer(GL_ARRAY_BUFFER,oVBO);
	glVertexPointer  (3,GL_FLOAT,sizeof(TC_VTAB),OFFSET_VBO(2*sizeof(float)));
	glTexCoordPointer(2,GL_FLOAT,sizeof(TC_VTAB), 0);
	for (int k=0; k < nPRM; k++)	glDrawArrays(GL_TRIANGLE_STRIP,ind[k],4);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	rwy->DrawDesignators(apo);
	if (blnd)  glDisable(GL_BLEND);
	return;
}
//=========================================================================================
//	The following methods pertain to the CRunway Object
//  Draw Threshold bands for Hi and Lo ends
//=========================================================================================
void CRunway::DrawThreshold(SVector &sl,U_CHAR rs)
{ SVector ct = {0};
  //--Draw left part ------------------------------
  GetLTR(ct,rs);                 // Letf part
  glPushMatrix();
  glTranslatef(ct.x,ct.y,ct.z);				//ct.z);
  glScaled(sl.x,sl.y,1);
  glRotated(GetROT(rs),0,0,1);
  DrawBand(GetNBT(rs));
  glPopMatrix();
  //--Draw right part -----------------------------
  GetRTR(ct,rs);                 // right part
  glPushMatrix();
  glTranslatef(ct.x,ct.y,ct.z);			//ct.z);
  glScaled(sl.x,sl.y,1);
  glRotated(GetROT(rs),0,0,1);
  DrawBand(GetNBT(rs));
  glPopMatrix();
  return;
}
//-----------------------------------------------------------------------------------------
//  Draw the designator of the runway
//  Runway Designator is made of 2 number with one optional lettre (example 10L or 03C)
//		Letters are painted as polygons
//		The process is
//  -Translate to the letter position
//  -Scale to runway dimension
//  -Rotate the letter
//-----------------------------------------------------------------------------------------
void CRunway::DrawRID(CAptObject *apo,int rs,char *rid)
{ SVector ct = {0};
  SVector sl;
  SVector sb;
  //----Compute scale factor ----------------
  sl.x = GetXscale();
  sl.y = GetYscale();
  //-----Draw Left Number -------------------
  GetLPS(ct,rs);
  glPushMatrix();
  glTranslatef(ct.x,ct.y,ct.z);
  glScaled(sl.x,sl.y,1);
  glRotated(GetROT(rs),0,0,1);
  DrawNumber(rid[0]);
  glPopMatrix();
  //-----Draw right Number -------------------
  GetRPS(ct,rs);
  glPushMatrix();
  glTranslatef(ct.x,ct.y,ct.z);
  glScaled(sl.x,sl.y,1);
  glRotated(GetROT(rs),0,0,1);
  DrawNumber(rid[1]);
  glPopMatrix();
  //-----Draw id letter --------------------------
  char No = GetLET(rs);
  if (No)
    { GetCPS(ct,rs);
      glPushMatrix();
      glTranslatef(ct.x,ct.y,ct.z);
      glScaled(sl.x,sl.y,1);
      glRotated(GetROT(rs),0,0,1);
      DrawLetter(No);
      glPopMatrix();
  }
  //----Draw Threshold bands ---------------------
  apo->GetSTH(sb);
  int nbb = GetNBT(rs);
  if (nbb)  DrawThreshold(sb,rs);
  return;
}
//----------------------------------------------------------------------------
//  Draw the runway letter designator
//----------------------------------------------------------------------------
void CRunway::DrawLetter(char No)
{ if (No == 0)    return;
	int		nbp = nbrVBO[No];
	GLint *pl = ofsTAB[No];
	GLint *cn = nbrPM;
	glMultiDrawArrays(GL_TRIANGLE_STRIP,pl,cn,nbp);
	return;
}
//----------------------------------------------------------------------------
//  Draw the runway designator
//----------------------------------------------------------------------------
void CRunway::DrawNumber(char cr)
{ if ((cr < ('0')) || (cr > '9')) return;
  int		No  = cr - '0';
	int		nbp = nbrVBO[No];
	GLint *pl = ofsTAB[No];
	GLint *cn = nbrPM;
	glMultiDrawArrays(GL_TRIANGLE_STRIP,pl,cn,nbp);
  return;
}
//----------------------------------------------------------------------------
//  Draw the runway THRESHOLD BAND
//----------------------------------------------------------------------------
void CRunway::DrawBand(int nbp)
{ if (nbp == 0)    return;
  GLint *cn = nbrPM;
  glMultiDrawArrays(GL_TRIANGLE_STRIP,ofsBD,cn,nbp);
  return;
}
//----------------------------------------------------------------------------
//  Draw both runway DESIGNATORS
//----------------------------------------------------------------------------
void CRunway::DrawDesignators(CAptObject *apo)
{ if (GetPaved() != TC_RWY_PAVED)  return;
  glDisable(GL_TEXTURE_2D);
	apo->apm->bindLETTERs();
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  DrawRID(apo,TC_HI,GetHiEnd());
  DrawRID(apo,TC_LO,GetLoEnd());
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER,0);
  glEnable(GL_TEXTURE_2D);
  return;
}
//---------------------------------------------------------------------------------
//  Set Letter position
//---------------------------------------------------------------------------------
void CRunway::SetLETTER(int k,U_CHAR rs)
{ SVector pos;
  //---- Letter position --------------
  pos.x  = RwyMID[k].px; 
  pos.y  = RwyMID[k].py; 
  pos.z  = RwyMID[k].pz;
  SetCPS(pos,rs);
  return;
}
//---------------------------------------------------------------------------------
//  Set Number position
//	nmx and nmy are factor to compute the perpendicular position corresponding
//	to a middle point along the runway
//---------------------------------------------------------------------------------
void CRunway::SetNUMBER(int k,U_CHAR rs,double nmx,double nmy)
{ SVector pos;
  int   inx = (rs << 1);                  // Index value
  double x0 = RwyMID[k].px;
  double y0 = RwyMID[k].py;
  double z0 = RwyMID[k].pz;
  //--- Left position ----------------
  pos.x  = x0 + (nmx * RwyNPOS[inx].x);		//apo->GetXnormal(RwyNPOS[inx].x);
  pos.y  = y0 + (nmy * RwyNPOS[inx].y);		//apo->GetYnormal(RwyNPOS[inx].y);
  pos.z  = z0;
  SetLPS(pos,rs);
  inx++;
  //--- Right position ----------------
  pos.x  = x0 + (nmx * RwyNPOS[inx].x);		//apo->GetXnormal(RwyNPOS[inx].x);
  pos.y  = y0 + (nmy * RwyNPOS[inx].y);		//apo->GetYnormal(RwyNPOS[inx].y);
  pos.z  = z0;
  SetRPS(pos,rs);
  return;
}

//=========================================================================================
//  Destroy Airport Q
//=========================================================================================
CAptQueue::~CAptQueue()
{ CAptObject *apo = Pop();
  while (apo) { delete apo; apo = Pop(); }
}
//=========================================================================================
//  Build Airport Object
//=========================================================================================
CAptObject::CAptObject(CAirportMgr *md, CAirport *apt)
{	apm     = md;
  pApt    = apt;
  Airp    = apt;
	nmiles  = apt->GetNmiles();
	//---- Clear vbo buffer to avoid drawing -----------------------
	pVBO	  = 0;
	eVBO		= 0;
	cVBO		= 0;
	//---- Locate QGT ----------------------------------------------
  Org     = Airp->GetPosition();
	//--------------------------------------------------------------
  apt->SetAPO(this);
  GetLatitudeFactor(Org.lat,rdf,xpf);
  ground  = Org.alt;
  oTAXI   = globals->txw->GetTaxiTexture();
  cutOF   = 1500;                         // Altitude cut-off
  //--------------------------------------------------------------
	bgr			= 1;
	txBGR   = 0;
  tcm     = globals->tcm;
  scale   = tcm->GetScale();
  //-----Colors --------------------------------------------------
  white  = (float*)&ColorTAB[TC_WHI_LITE];
  yellow = (float*)&ColorTAB[TC_YEL_LITE];
  //-----Drawing parameters --------------------------------------
  sta3D   = TC_AP_FADE_IN;
  alpha   = 0;
  //-----SQL option ----------------------------------------------
  txy     = globals->sqm->SQLtxy();
  //-----Compute scale factor for designator ---------------------
  scl.x = FN_ARCS_FROM_FEET(1.4) * xpf;
  scl.y = FN_ARCS_FROM_FEET(1.4);
  //-----Scale factor for threshold bands ------------------------
  sct.x = FN_ARCS_FROM_FEET(xpf);
  sct.y = FN_ARCS_FROM_FEET(1);
	//--- VBO Management -------------------------------------------
	nGVT	= 0;
	gBUF	= 0;
	glGenBuffers(1,&gVBO);
  //----Add profile to POD ---------------------------------------
  AddPOD();
  //-----Lighting control -----------------------------------------
  lTim    = 0;                              // Light timer
  lrwy    = 0;                              // Lighting runway
  swlt    = 0;                              // State off
  lreq    = 0;                              // No request
  //----Init Trace option ---------------------------------------
	int op  = 0;
  GetIniVar ("TRACE", "Airport", &op);
  tr			= op;
	if (tr) TRACE("===AIRPORT CONSTRUCTION: %s",apt->GetName());
	//--- Generate a sphere for test ------------------------------
	sphere = gluNewQuadric();
}
//----------------------------------------------------------------------------------
//  Constructor for export only
//----------------------------------------------------------------------------------
CAptObject::CAptObject(CAirport *apt)
{ apm     = 0;
  pApt    = 0;
  Airp    = apt;
  Org     = apt->GetPosition();
  ground  = Org.alt;
  GetLatitudeFactor(Org.lat,rdf,xpf);
  txy     = 0;
  tcm     = globals->tcm;
  scale   = tcm->GetScale();
}
//----------------------------------------------------------------------------------
//  Init position
//----------------------------------------------------------------------------------
bool CAptObject::InitBound()
{ GroundSpot spot(Org.lon,Org.lat);
  globals->tcm->SetGroundAt(spot);
	Airp->SetElevation(spot.alt);					// Set terrain altitude			
  glim.xmax = glim.xmin = AbsoluteTileKey(spot.qx,spot.tx);
  glim.zmax = glim.zmin = AbsoluteTileKey(spot.qz,spot.tz);
  return spot.HasQGT();
}
//----------------------------------------------------------------------------------
//  Add files for this airport
//  /RUNWAYS/XXX.RLP            For runway light profile
//  /DIAGRAM/XXX.PNG            For airport Diagram
//----------------------------------------------------------------------------------
void CAptObject::AddPOD()
{ char  fnm[PATH_MAX];
  char  key[PATH_MAX];
  _snprintf(key,(PATH_MAX-1),"%s.RLP",GetAptName());
  _snprintf(fnm,(PATH_MAX-1),"RUNWAYS/%s",key);
  pAddDisk(&globals->pfs,key,fnm);
  _snprintf(key,(PATH_MAX-1),"%s.PNG",GetAptName());
  _snprintf(fnm,(PATH_MAX-1),"DIAGRAMS/%s",key);
  pAddDisk(&globals->pfs,key,fnm);
  return;
}
//---------------------------------------------------------------------------------
//  Remove Profile from POD
//---------------------------------------------------------------------------------
void CAptObject::RemPOD()
{ char  fnm[PATH_MAX];
  char  key[PATH_MAX];
  _snprintf(key,(PATH_MAX-1),"%s.RLP",GetAptName());
  _snprintf(fnm,(PATH_MAX-1),"RUNWAYS/%s",key);
  pRemDisk(&globals->pfs,key,fnm);
  _snprintf(key,(PATH_MAX-1),"%s.PNG",GetAptName());
  _snprintf(fnm,(PATH_MAX-1),"DIAGRAMS/%s",key);
  pRemDisk(&globals->pfs,key,fnm);
  return;
}
//---------------------------------------------------------------------------------
//  destroy Airport Object
//  NOTE:  Before proceeding a new organisation for airports, we must avoid thz teleport
//         case where the QGT are destroyed first then the airport object (then the Quad
//          and the textureDef are no longer allocated)
//----------------------------------------------------------------------------------
CAptObject::~CAptObject()
{ if (apm) RemPOD();
  if (pVBO)	glDeleteBuffers(1,&pVBO);
	if (eVBO)	glDeleteBuffers(1,&eVBO);
	if (cVBO)	glDeleteBuffers(1,&cVBO);
	if (gVBO) glDeleteBuffers(1,&gVBO);
  //--- free tarmac segments ------------------
	std::vector<CTarmac *>::iterator it;
	for (it=tmcQ.begin(); it!=tmcQ.end(); it++)
	{	CTarmac *tmac = (*it);
		delete tmac;
	}
	//--- Free ground VBO -----------------------
	if (gBUF)		delete [] gBUF;
	//--- free BGR ------------------------------
	if (txBGR)	delete txBGR;
	//-------------------------------------------
	tmcQ.clear();
  UnmarkGround();
	gluDeleteQuadric(sphere);
}
//---------------------------------------------------------------------------------
//  Compute airport extension in term of Detail Tiles
//---------------------------------------------------------------------------------
void CAptObject::AptExtension(GroundSpot &gs)
{ U_INT xk = AbsoluteTileKey(gs.qx,gs.tx);
  if (TileIsLeft(xk,glim.xmin))   glim.xmin = xk;
  if (TileIsLeft(glim.xmax,xk))   glim.xmax = xk;
  U_INT zk = AbsoluteTileKey(gs.qz,gs.tz);
  if (zk < glim.zmin)             glim.zmin = zk;
  if (zk > glim.zmax)             glim.zmax = zk;
  return;
}
//---------------------------------------------------------------------------------
//  For each runway, build a set of segment polygons
//  NOTE: All pavements use the common ground from airport position
//        All coordinates are in arcsec, relative to airport origin
//---------------------------------------------------------------------------------
void CAptObject::SetRunway()
{ U_CHAR lit = 0;
  CAirport *apt = GetAirport();
  if (!apt->HasRunway())          return;
  if (!globals->tcm->MeshReady()) return;
  if (!InitBound())               return;
  lit = apm->SetRunwayProfile(apt);
  CRunway  *rwy = 0;
  //-----Build non paved runway first -----------------
  for (rwy = apt->GetNextRunway(rwy); rwy != 0;rwy = apt->GetNextRunway(rwy))
  { U_CHAR   type = RwyTYP[rwy->GroundIndex()];
    rwy->SetPaved(type);
    //----Now build runway segments -------------------
    if (type != TC_RWY_OTHER) continue;
    BuildOtherRunway(rwy);
    if (lit) BuildRunwayLight(rwy);
  }
  //----Build paved runway after ----------------------
  for (rwy = apt->GetNextRunway(rwy); rwy != 0;rwy = apt->GetNextRunway(rwy))
  { int   type = RwyTYP[rwy->GroundIndex()];
    //----Now build runway segments -------------------
    if (type != TC_RWY_PAVED) continue;
    BuildPavedRunway(rwy);
    if (lit) BuildRunwayLight(rwy);
  }
  //-----Read the pavement data -----------------------
  GetTaxiways();
	CompactRWY();
  //-----Locate airport ground    ---------------------
  LocateGround();                                   // Locate detail tiles
  return;
}
//---------------------------------------------------------------------------------
//  set Runway lights according to light profile
//---------------------------------------------------------------------------------
void CAptObject::BuildRunwayLight(CRunway *rwy)
{ int mod = 0;
  CRLP     *lpf = rwy->GetRLP();
  //---Build center light model -------------------
  mod = lpf->GetCenterLM();
  (this->*ctlVECTOR[mod])(rwy);
  //---Build edge light model ---------------------
  mod = lpf->GetEdgeLM();
  (this->*edgVECTOR[mod])(rwy);
  //---Other lights -------------------------------
  TouchDWLights(rwy);
  //---Build threshold bars -------------------------
  ThresholdBarLights(rwy);
  WingBarLights(rwy);
  ApproachLight(rwy);
  return;
}
//---------------------------------------------------------------------------------
//  Compute Runway Parameters
//  NOTE: All coordinates are in arcsec relative to airport origin 
//        All the computed parameters can be used to build various parts of
//        the current runway:
//        -Segment of tarmac
//        -Center lights
//        -Edge lights
//        -Threshold lights
//---------------------------------------------------------------------------------
void CAptObject::SetRunwayData(CRunway *rwy)
{ SPosition deb  = rwy->GetHiPos();
  SPosition end  = rwy->GetLoPos();
  //---Position relative to airport origin -------------
  Offset(deb,p0);               // Offset of runway deb
  Offset(end,p1);               // Offset of runway end
  dx   = (p1.x - p0.x);
  dy   = (p1.y - p0.y);
  dz   = (p1.z - p0.z);
  //--Store mid point -------------------------------------
  mid.x   = (p0.x + p1.x) * 0.5;
  mid.y   = (p0.y + p1.y) * 0.5;
  //--Store displacements ---------------------------------
  loDSP = rwy->GetLoDSP();
  hiDSP = rwy->GetHiDSP();
  tlgr  = rwy->GetLenghi() - hiDSP - loDSP;    // real length
  //---Store dimensions -----------------------------------
  width   = rwy->GetWidth() * 0.51; // * cpf;     
  rlgt    = rwy->GetLength();
  rhwd    = rwy->GetWidthi() >> 1;            // Runway half wide
  altF    = dz / rlgt;                        // alt per feet
  //---Compute runway Arcsec per feet per coordinate ------
  arcX    = dx / rlgt;
  arcY    = dy / rlgt;
  //---Compute edge normal component (longitude compensated)
  ppx = (width * arcY * xpf);
  ppy = (width * arcX * rdf);
  //---Compute (edge + 5 feet) normal component -----------
  double rw = width + 5;
  egx  = rw * arcY * xpf;
  egy  = rw * arcX * rdf;
	//--- Compute coefficients for vector director ----------
	//	Vector director is the line starting at Hi position and
	//	going throught lo position
	CVector ps0 = p0;
	CVector ps1 = p1;
	FeetCoordinates(ps0,rdf);
	FeetCoordinates(ps1,rdf);
	VECTOR_DIR *vdr = rwy->GetVDIR();
	double a  = -(ps1.y - ps0.y);
	double b  = +(ps1.x - ps0.x);
	vdr->rdf	= rdf;
	vdr->org  = Org;
	vdr->afa	= a;
	vdr->bta  = b;
	vdr->gma  = -((a * ps0.x) + (b * ps0.y));
	vdr->lgn  = sqrt((a * a) + (b * b));
	//double t1 = (a * ps1.x) + (b * ps1.y) + vdr->gma;
	//double t2 = (a * ps0.x) + (b * ps0.y) + vdr->gma;
  //-------------------------------------------------------
  //  We compute a point inside the runway
  //  some feet after the threshold.  This will be
  //  the ILS landing position.
  //  Then we compute an origin point (at some more distance)
  //  that is used in the glide slope computation
  //-------------------------------------------------------
  ILS_DATA   *ils   = rwy->GetIlsData(RWY_HI_END);
  ils->d1   = rwyMARGE[rwy->GetLgCode()] + hiDSP;
  ils->d2   = rlgt;
  ils->d3   = ils->d1 - 15000;
	ils->d4   = 0;							// Not used
  SetLandingPRM(ils,rwy->GetHmDir(),rwy->GetLmDir());
  //---- same for lo end ---------------------------------
  ils       = rwy->GetIlsData(RWY_LO_END);
  ils->d1   = (rlgt - loDSP) - rwyMARGE[rwy->GetLgCode()];
  ils->d2   = 0;
  ils->d3   = ils->d1 + 15000;
	ils->d4   = 0;							// Not used
  SetLandingPRM(ils,rwy->GetLmDir(),rwy->GetHmDir());
  //--- SetRunway texture offset in packed texture -------
  wTex  = (rwy->GetPaved() == TC_RWY_PAVED)?(float(1)/8):(float(1)/5);
  return;
}
//---------------------------------------------------------------------------------
//  We compute a point inside the runway
//  some feet after the threshold.  This will be
//  the ILS landing position. (lndP)
//  Then we compute an origin point (at opposite end)
//  that is used in the glide slope computation
//  all coordinates are in absolutes arcseconds
//	Then we compute a far point (farP) 15000 feet away for drawing ILS
//	Example:
//          mini above
//				(M)	Threshold 
//					  |        lndP (landing point)
//						|					|										Opposite End
//	RWY 12L  -------------------------------------                 (at 15000 feet)
//											L												|                    |
//																							|                    |
//																							|                    |
//																							refP(R)              |
//																																Far (F)
//	M, L , R and F are aligned at 3° slope
//--------------------------------------------------------------------------------
void CAptObject::SetLandingPRM(ILS_DATA *ils,float ln,float tk)
{ double d1 = ils->d1;
  SPosition  *land  = &ils->lndP;
  land->lon = p0.x + (arcX * d1);  // X coord
  land->lat = p0.y + (arcY * d1);  // Y coord
  ComputeElevation(*land);
  //---- Compute origin point ---------------------
  double d2 = ils->d2;
  SPosition  *ref  = &ils->refP;
  ref->lon  = p0.x + (arcX * d2) + Org.lon;  // X coord
  ref->lat  = p0.y + (arcY * d2) + Org.lat;  // Y coord
  ref->alt  = land->alt - (fabs(d2 - d1) * ils->gTan);
  //---- Compute far point at 15000 feet ----------
  SPosition  *fpn  = &ils->farP;
  double d3 = ils->d3;
  fpn->lon  = p0.x + (arcX * d3) + Org.lon;  // X coord
  fpn->lat  = p0.y + (arcY * d3) + Org.lat;  // Y coord
  fpn->alt  = land->alt + (fabs(d3 - d1) * ils->gTan);
  //--- Set landing direction for normal runway ---
	ils->tkDIR	= ln;											// Take off direction
	if (0 == ils->ils) ils->lnDIR = ln;		// Landing direction
  return;
}
//---------------------------------------------------------------------------------
//	Trace runway parameters 
//---------------------------------------------------------------------------------
void CAptObject::TraceRWY(CRunway *rwy)
{ RwyID *hi = rwy->GetEndDEF(RWY_HI_END);
	TRACE("****Rwy %4s-%4s head=%.4f magh=%.4f ori=%.4f lnd=%.2f",
				Airp->GetIdent(),rwy->GetHiEnd(),rwy->GetHiDir(),rwy->GetHmDir(),
				hi->aRot,rwy->GetLandDir(RWY_HI_END));
	RwyID *lo = rwy->GetEndDEF(RWY_LO_END);
	TRACE("****Rwy %4s-%4s head=%.4f magh=%.4f ori=%.4f lnd=%.2f",
				Airp->GetIdent(),rwy->GetLoEnd(),rwy->GetLoDir(),rwy->GetLmDir(),
				lo->aRot,rwy->GetLandDir(RWY_LO_END));
	return;
}
//---------------------------------------------------------------------------------
//  A runway model defines all segments in a given runway type.  This model is used
//	to generate the runway segment, adjusting the size of each segment, according to
//	the model.  The process generate mid point for all segments, then using those
//	points and the normal vector, the 4 points of a segment are generated
//---------------------------------------------------------------------------------
//  -All coordinates are relative to Airport origin in arcseconds
//  -Threshold are always generated
//  -Markers are generated up to 1/4 of each runway end
//    This process cut the runway into typed segment, using the model table
//    PavedRWY_MODEL.  Each segment is specialized into some particular category
//    -Displaced threshold
//    -Threshold
//    -Identifier
//    -Marker
//     etc.
//  Once the model table is filled, segment coordinates are generated
//----------------------------------------------------------------------------------
void CAptObject::BuildRunwayMidPoints(CRunway *rwy,TC_RSEG_DESC *model)
{ //---Eliminate runway if no parameters ------------------
  eInd  = 0;
  if ((dx == 0) && (dy == 0)) return;
  //---Compute the rotation parameters --------------------
  rot = atan2(-dx * rdf,dy);
  rot = RadToDeg(rot);
  rwy->SetROT(rot,TC_HI);
  rwy->SetROT((rot + 180),TC_LO);
	if (tr) TraceRWY(rwy);
  //---Compute runway scale (unit = width / 200)----------
  //  Scale is used for scaling the designator
  double ch = width / 200;
  rwy->SetXscale(ch * scl.x);
  rwy->SetYscale(ch * scl.y);
  //---Compute limit of marker generation --(real rwy lgth/4) - hi threshold)
  int lim = (tlgr >> TC_BY04) + hiDSP;           // marker limit                          
  total = 0;                                     // Total generated
  //--Init model table ------------------------------------
  int mhd = hiDSP % 100;
  int lhd = loDSP % 100;
  model[TC_HD1SEGINDEX].segLG = mhd;           // Hi displaced length
  model[TC_HD2SEGINDEX].segLG = hiDSP - mhd; 
  model[TC_LD1SEGINDEX].segLG = lhd;           // Lo displaced length
  model[TC_LD2SEGINDEX].segLG = loDSP - lhd;
  //--Start mid point generation --------------------------
  xsr = 0;
  xds = 0;
  int slg = 0;                                  // Segment length
  int tgt = 0;
  rInd    = TC_MIDSEGINDEX;                     // Default restart for long runway
  while (rwy)
  { slg =  model[xsr].segLG;
    switch(model[xsr].segGN)  {                // Generation code
      //--Displaced threshold: Generate if length not 0 -
      case TC_GEN_DSP:
        if (slg)  break;                        // Generate
        xsr++;                                  // By pass
        continue;
      //--Always generated : THRESHOLD and DESIGNATORS---
      case TC_GEN_ALW:
        break;
      //--Generate up to the limit ----------------------
      case TC_GEN_LIM:
        if ((total + slg) < lim)  break;        // Generate
        rInd  = (TC_MIDSEGINDEX << 1) - xsr;    // Restart index
        xsr    = (TC_MIDSEGINDEX);              // Go to mid segment
        continue;
      //--Mid segment generation ------------------------
      case TC_GEN_MID:
        tgt = tlgr + (hiDSP << 1) - total;      // New total
        BreakMidSegment(model,(tgt - total));
        continue;
      //--Generate last point ---------------------------
      case TC_GEN_END:
        RwyMID[xds].segID = TC_SG_END;
				RwyMID[xds].segTX = 0;
        RwyMID[xds].segLG = 0;
        RwyMID[xds].px    = p1.x;
        RwyMID[xds].py    = p1.y;
        eInd  = xds;
        return;
    }
    //----Come here to generate the mid point -------
    GenerateRwyPoint(model);
  }
  return;
}
//---------------------------------------------------------------------------------
//  Generate a new segment entry in RwyMID table
//---------------------------------------------------------------------------------
void CAptObject::GenerateRwyPoint(TC_RSEG_DESC *model)
{   RwyMID[xds].segID  = model[xsr].segID;
    RwyMID[xds].segTX  = model[xsr].segTX;
    RwyMID[xds].segLG  = model[xsr].segLG;
    RwyMID[xds].px     = p0.x + (arcX * total);  // X coord
    RwyMID[xds].py     = p0.y + (arcY * total);  // Y coord
    RwyMID[xds].pz     = p0.z + (altF * total);  // Z coord
    total += model[xsr].segLG;
    if (xsr == TC_MIDSEGINDEX) xsr = rInd;          // Restart index
    xsr++;
    xds++;
    return;
}
//---------------------------------------------------------------------------------
//  Cut the mid segment into parts of 400 feet length
//---------------------------------------------------------------------------------
void CAptObject::BreakMidSegment(TC_RSEG_DESC *model,int lg)
{ int src = xsr;
  int rst = lg;
  //---Generate intermediate segments ----------------------------------
  while (rst > 400)
  { model[xsr].segLG = 400;                       // Segment Size
    GenerateRwyPoint(model);                        // One segment
    xsr   = src;                                  // Stay on same model
    rst  -= 400;
  }
  //---Generate last segment --------------------------------------------
  model[xsr].segLG = rst;
  GenerateRwyPoint(model);
}
//---------------------------------------------------------------------------------
//  Set left and right positions for threshold bands
//  Each band is 12 feet spaced apart and start 10 feet from the runway border
//---------------------------------------------------------------------------------
void  CAptObject::SetHiThreshold(int k,CRunway *rwy)
{ CVector   pos;
  int       nbt = rwy->GetNBT(TC_HI);
  double    wid = rwy->GetWidth() * 0.51;
  if (0 == nbt)   return;
  //---Left position is given by corner ---------------
  pos.x   = bsw.VT_X;
  pos.y   = bsw.VT_Y;
  pos.z   = bsw.VT_Z;
  rwy->SetLTH(pos,TC_HI);
  //---Right position is computed from Mid point ------
  double dta = (wid - 16 - (nbt * 12)) / wid;
  pos.x = RwyMID[k].px +  GetXnormal(dta);
  pos.y = RwyMID[k].py -  GetYnormal(dta);
  rwy->SetRTH(pos,TC_HI);
  return;
}
//---------------------------------------------------------------------------------
//  Set left and right positions for threshold bands
//  Each band is 12 feet spaced apart and start 10 feet from the runway border
//---------------------------------------------------------------------------------
void  CAptObject::SetLoThreshold(int k,CRunway *rwy)
{ SVector   pos;
  int       nbt = rwy->GetNBT(TC_LO);
  double    wid = rwy->GetWidth() * 0.51;
  if (0 == nbt)   return;
  //---Left position is the NE corner -----------------
  pos.x   = vse.VT_X;
  pos.y   = vse.VT_Y;
  pos.z   = vse.VT_Z;
  rwy->SetLTH(pos,TC_LO);
  //---Right position is computed from Mid point ------
  double dta = (wid - 16 - (nbt * 12)) / wid;
  pos.x = RwyMID[k].px -  GetXnormal(dta);
  pos.y = RwyMID[k].py +  GetYnormal(dta);
  rwy->SetRTH(pos,TC_LO);
  return;
}
//---------------------------------------------------------------------------------
//  Build the Tarmac
//  For each segment generate a Quad with 4 points computed from the mid points
//      using orthogonal factors (ppx and ppy)
//      Coordinates are in arcseconds relative to the airport origin
//  
//	Vertices will be drawed CW in TRIANGLE_STRIPS in the following order
//		NW(1)			NE(3)
//
//		SW(0)			SE(2)
//
//---------------------------------------------------------------------------------
void CAptObject::BuildTarmacSegs(CRunway *rwy)
{ int dim = eInd;
	CTarmac *tmac = new CTarmac(this,rwy);								// Create Tarmac
	U_CHAR    gr = rwy->GroundIndex();										// Runway ground type
  globals->txw->GetRwyTexture(tmac,gr);									// Set runway texture
  SegmentBase(0);                                       // Compute base points
  AptExtension(psw);                                    // Set extension
  AptExtension(pse);                                    // Set Extension
	//if (tr) TRACE ("     Building segment (V2 map V1) (V3 map V2)");
  //-------------------------------------------------------------
	TC_VTAB *tab	= 0;
  for (U_CHAR k = 0; k != eInd; k++)
    { U_CHAR  u = k + 1;                                // Upper index
			tab  = tmac->Reserve(4);													// Get room for vertices
			//----Fill vertex table ---------------------------------
			tab[0]	= vsw;										// SW corner
			tab[2]  = vse;										// SE corner
      SegmentBase(u);										// Compute North coordinates
			tab[3]  = vse;										// NE corner
			tab[1]  = vsw;										// NW corner
			SetTxCoord(tab,k,gr);							// Assign texture coordinates
      //-------Dispatch specific process here -----------------
			switch (RwyMID[k].segID)
			{	case TC_SG_HTR:									// Hi threshold seg
          SetHiThreshold(k,rwy);
          break;
				case TC_SG_HLT:									// Hi letter seg
          rwy->SetLETTER(u,TC_HI);
          break;
				case TC_SG_HNB:									// Hi number seg
					rwy->SetNUMBER(k,TC_HI,ppx,ppy);
          break;
				case TC_SG_LNB:									// Lo number seg
					rwy->SetNUMBER(k,TC_LO,ppx,ppy);
          break;
				case TC_SG_LLT:									// Lo letter seg
          rwy->SetLETTER(k,TC_LO);
          break;
				case TC_SG_LTR:									// Lo threshold seg
          SetLoThreshold(u,rwy);
          break;

			}
		}
	//-----Check extension for last points ---------------------
  AptExtension(psw);
  AptExtension(pse);
	//--- Load the VBO -----------------------------------------
	tmac->LoadVBO();
	AddTarmac(tmac);
  return;
}
//---------------------------------------------------------------------------------
//  Set Hi coordinates 
//  Complex runways has 6 textures aligned side by side on horizontal axis.
//  Each texture is for 100 feet long and should be repeated along the t axis
//	x => Index of subtexture inside whole texture
//	w => Fractional witdh of a subtexture
//  xrl:  X relocation factor inside the subtexture
//	Vertices will be drawed CW in TRIANGLE_STRIPS in the following order
//		NW(1)			NE(3)
//
//		SW(0)			SE(2)
//---------------------------------------------------------------------------------
void CAptObject::SetTxCoord(TC_VTAB *tab,int No, char grnd)
{ char   tx     = RwyMID[No].segTX;
	char   nt     = (tx == '*')?(RwyOTHR[grnd]):(tx - '0');
  float  t      = float(RwyMID[No].segLG)  / 100;      // Height
  float  xrl    = wTex * (nt - 1);
  //--------SW corner -----------------------------------------
  tab[0].VT_S = 0 + xrl;
  tab[0].VT_T = 0;
  //--------NW corner------------------------------------------
  tab[1].VT_S = 0 + xrl;
  tab[1].VT_T = t;
  //------- NE corner -----------------------------------------
  tab[3].VT_S = wTex + xrl;
  tab[3].VT_T = t;
  //--------SE Corner -----------------------------------------
  tab[2].VT_S = wTex + xrl;
  tab[2].VT_T = 0;
  return;
}

//-------------------------------------------------------------------------------
//  Compute base segment at mid point k
//-------------------------------------------------------------------------------
void CAptObject::SegmentBase(int k)
{ double xl     = RwyMID[k].px;                     // mid X
  double yl     = RwyMID[k].py;                     // mid Y
  //--Compute SW base --------------------------------------
  bsw           = vsw;                              // Save previous
  vsw.VT_X      = xl - ppx;
  vsw.VT_Y      = yl + ppy;
  psw.lon       = vsw.VT_X + Org.lon;
  psw.lat       = vsw.VT_Y + Org.lat;
  globals->tcm->SetGroundAt(psw);
  vsw.VT_Z      = psw.alt - Org.alt;
  //--Compute SE base --------------------------------------
  bse           = vse;                              // Save previous
  vse.VT_X      = xl + ppx;
  vse.VT_Y      = yl - ppy;
  pse.lon       = vse.VT_X + Org.lon;
  pse.lat       = vse.VT_Y + Org.lat;
  globals->tcm->SetGroundAt(pse);
  vse.VT_Z      = pse.alt - Org.alt;
  //--Compute Mid point elevation --------------------------
  RwyMID[k].pz  = (vsw.VT_Z + vse.VT_Z) * 0.5;
  return;
}
//-------------------------------------------------------------------------------
//    Get elevation for paved vertex
//  TODO must check for world wrap arround the 0 meridian
//-------------------------------------------------------------------------------
void CAptObject::ComputeElevation(TC_VTAB *tab)
{ double lon = tab->VT_X + Org.lon;
  double lat = tab->VT_Y + Org.lat;
  GroundSpot spot(lon,lat);
  globals->tcm->SetGroundAt(spot);
  tab->VT_Z  = (spot.alt - Org.alt);
  return;
}
//--------------------------------------------------------------------------------
//  Compute elevation from location
//  Adjust longitude and latitude to absolute value
//--------------------------------------------------------------------------------
void CAptObject::ComputeElevation(SPosition &pos)
{ pos.lon += Org.lon;
  pos.lat += Org.lat;
  GroundSpot lnd(pos.lon,pos.lat);
  pos.alt  = globals->tcm->SetGroundAt(lnd);
  return;
}
//---------------------------------------------------------------------------------
//  Build a paved runway
//---------------------------------------------------------------------------------
void CAptObject::BuildPavedRunway(CRunway *rwy)
{ //if (tr)  TRACE("  PAVED RUNWAY %s-%s for %s (key=%s)",
	//							 rwy->GetHiEnd(),rwy->GetLoEnd(),Airp->GetName(),Airp->GetKey());
	rwy->SetNumberBand(12);
  SetRunwayData(rwy);
  if (tlgr <= 0) return;
  BuildRunwayMidPoints(rwy,PavedRWY_MODEL);
	BuildTarmacSegs(rwy);
  return;
}
//---------------------------------------------------------------------------------
//  Build other runways
//  Non paved runways are drawn as one piece
//
//---------------------------------------------------------------------------------
void CAptObject::BuildOtherRunway(CRunway *rwy)
{ //--Init model parameters -----------------------------------------
	//if (tr)  TRACE("  OTHER RUNWAY %s-%s for %s",
	//							rwy->GetHiEnd(),rwy->GetLoEnd(),Airp->GetName());
  SetRunwayData(rwy);
  if (tlgr <= 0) return;
  BuildRunwayMidPoints(rwy,OtherRWY_MODEL);
	BuildTarmacSegs(rwy);
  return;
}
//---------------------------------------------------------------------------------
//  Build one VBO from the Queue
//---------------------------------------------------------------------------------
void CAptObject::CompactRWY()
{	U_INT     tot = 0;
	TC_VTAB  *buf = 0;
	//--- Pavement VBO ---------------------------
	buf		= PutInVBO(pavQ,nPAV);
	if (buf)
	{	tot = nPAV * sizeof(TC_VTAB);
		glGenBuffers(1,&pVBO);
		glBindBuffer(GL_ARRAY_BUFFER,pVBO);
		glBufferData(GL_ARRAY_BUFFER,tot,buf,GL_STATIC_DRAW);
		delete [] buf;
	}
	//--- Edge VBO --------------------------------
	buf		= PutInVBO(edgQ,nEDG);
	if (buf)
	{	tot = nEDG * sizeof(TC_VTAB);
		glGenBuffers(1,&eVBO);
		glBindBuffer(GL_ARRAY_BUFFER,eVBO);
		glBufferData(GL_ARRAY_BUFFER,tot,buf,GL_STATIC_DRAW);
		delete [] buf;
	}
	//--- Center VBO ------------------------------
	buf		= PutInVBO(cntQ,nCTR);
	if (buf)
	{	tot = nCTR * sizeof(TC_VTAB);
		glGenBuffers(1,&cVBO);
		glBindBuffer(GL_ARRAY_BUFFER,cVBO);
		glBufferData(GL_ARRAY_BUFFER,tot,buf,GL_STATIC_DRAW);
		delete [] buf;
	}
  glBindBuffer(GL_ARRAY_BUFFER,0);
	return;
}
//---------------------------------------------------------------------------------
//  Build one VBO from the Queue
//---------------------------------------------------------------------------------
TC_VTAB *CAptObject::PutInVBO(CPaveQ &hq, U_INT n)
{ if (0 == n)	return 0;
	TC_VTAB   *vbo  = new TC_VTAB[n];
	TC_VTAB   *dst	= vbo;
	
	CPaveRWY	*pave = hq.Pop();
	while (pave)
	{	dst += pave->CopyTo(dst);
	  delete pave;
		pave = hq.Pop();
	}
	return vbo;	}
//---------------------------------------------------------------------------------
//  Build Taxiways from files
//---------------------------------------------------------------------------------
int CAptObject::BuildTaxiways()
{ char  *fn  = 0;
  char  fname[64];
  fn = Airp->GetIdentity();
  _snprintf(fname,64,"DATA/%s.TMS",fn);
  if (pexists (&globals->pfs, fname))               // Description
  {   CDataTMS tms(this);                           // TMS decoder
      tms.DecodeBinary(fname);                      // Decode file
  }
  return 1;
}
//---------------------------------------------------------------------------------
//  Check for SQL taxiways
//---------------------------------------------------------------------------------
int CAptObject::GetTaxiways()
{ nPAV	= 0;
	nEDG	= 0;
	nCTR	= 0;
	//--- Load yellow texture --------------------------
	if (0 == txy) return BuildTaxiways();
  //---Read taxiways from SQL database ---------------
  int np = globals->sqm->DecodePAVE(this);
  if (0 == np)  return BuildTaxiways();
  int nl = globals->sqm->DecodeLITE(this);
	//--- Compact all data into VBO --------------------
  return 1;
}
//---------------------------------------------------------------------------------
//  Add one pavement to Queue
//---------------------------------------------------------------------------------
void CAptObject::OnePavement(CPaveRWY *p,U_INT n)
{switch (p->GetType()) {
  case PAVE_TYPE_PAVE:
    pavQ.PutEnd(p);
		nPAV	+= n;
    return;
  case PAVE_TYPE_EDGE:
    edgQ.PutEnd(p);
		nEDG	+= n;
    return;
  case PAVE_TYPE_CENT:
    cntQ.PutEnd(p);
		nCTR	+= n;
    return;
  }
//---Unknown type ---------------------------
  delete p;
}
//---------------------------------------------------------------------------------
//  Airport beacon
//---------------------------------------------------------------------------------
void CAptObject::BeaconLight()
{ float ofs = float(FN_ARCS_FROM_FEET(100));
  U_CHAR inx = Airp->GetBeacon();
  if ((inx == 0) || (inx  > 7)) return;
  CFlshLITE *lit = new CFlshLITE();
  lit->TimeON (2.0);
  lit->TimeOFF(0.5);
  TC_VTAB  ent;
  lit->col1     = LiteBC1[inx];
  lit->col2     = LiteBC2[inx];
  lit->ntex     = TC_WHI_TEX;
  ent.VT_S = 0.5;
  ent.VT_T = 0.5;
  ent.VT_X = ofs;              // Offset beacon location
  ent.VT_Y = ofs;              // Offset beacon location
  ent.VT_Z = 30;
  lit->SpotAt(0,ent);
  becS.AddLight(lit);
  becS.SetMiniSize(16);
  becS.SetS1(1);
  return;
}
//---------------------------------------------------------------------------------
//  Build longitudinal bar of omni lights
//  off= starting offset
//  tgl= Length to build
//  total: total feet covered from x0,y0
//---------------------------------------------------------------------------------
void CAptObject::BuildOmniLBAR(LITE_MODEL &seg,CLitSYS &ls)
{ CBaseLITE *lit = new CBaseLITE(seg.type,seg.nbo);
  TC_VTAB  ent;
  //---Common texture coordinates -------------------
  ent.VT_S = 0.5;
  ent.VT_T = 0.5;
  //---Color and shape ------------------------------
  lit->col1     = seg.col1;
  lit->col2     = seg.col2;
  lit->ntex     = seg.ntex;
  //---Spot generation ------------------------------
  for (int k = 0; k != seg.nbo; k++)
  { ent.VT_X = p0.x + (arcX * total);
    ent.VT_Y = p0.y + (arcY * total);
    LsComputeElevation(ent,Org);
    lit->SpotAt(k,ent);
    total += seg.pace;
   }
  ls.AddLight(lit);
  return;
}
//---------------------------------------------------------------------------------
//  Fill decision parameters
//  
//---------------------------------------------------------------------------------
void  CAptObject::StoreDecision(CBaseLITE *lite)
{ lite->side.po = mid;                  // Set mid point as decision point
  lite->side.pa.x = lpt.x - egx;     //ppx;
  lite->side.pa.y = lpt.y + egy;     //ppy;
  lite->side.pb.x = lpt.x + egx;     //ppx;
  lite->side.pb.y = lpt.y - egy;     //ppy;
  //--Init pre computation ------------------------
  lite->PreCalculate();
  return;
}
//---------------------------------------------------------------------------------
//  Build Longitudinal ramp with 2 Dual Lights L1 and L2
//  Use the normal vector to L1-L2 axis as a decision line
//               |   
//        ---L1----L2----Runway center line
//               |
//              decision line
//---------------------------------------------------------------------------------
void CAptObject::BuildDualLLIN(LITE_MODEL &seg,CLitSYS &ls)
{ int nbo         = 2;
  CDualLITE *lit  = 0;                 
  TC_VTAB ent;
  //---Common texture coordinates ---------------------
  ent.VT_S = 0.5;
  ent.VT_T = 0.5;
  //---Spot generation --------------------------------
  lit       = new CDualLITE(seg.type,0);
  lit->col1       = seg.col1;
  lit->col2       = seg.col2;
  lit->ntex       = seg.ntex;
  //----compute light L1 coordinates ---------------
  ent.VT_X = p0.x + (arcX * total);
  ent.VT_Y = p0.y + (arcY * total);
  LsComputeElevation(ent,Org);
  lit->Lit1At(ent);
  total += seg.pace;
  //----compute light L2 coordinates ---------------
  ent.VT_X = p0.x + (arcX * total);
  ent.VT_Y = p0.y + (arcY * total);
  LsComputeElevation(ent,Org);
  lit->Lit2At(ent);
  total += seg.pace;
  lpt.x       = lit->LT[1].VT_X;
  lpt.y       = lit->LT[1].VT_Y;
  //-----Compute decision line --------------------
  StoreDecision(lit);
  //-----Add ligth to light system ----------------
  ls.AddLight(lit);
  return;
}
//---------------------------------------------------------------------------------
//  Build intro01 part center lights
//  off= starting offset
//  tgl= Length to build
//  total: total feet covered from x0,y0
//---------------------------------------------------------------------------------
void CAptObject::BuildCenterDual(LITE_MODEL &seg,CLitSYS &ls)
{ int nbo         = seg.nbo;
  //---Spot generation --------------------------------
  for (int k = 0; k < nbo; k++) BuildDualLLIN(seg,ls);
  return;
}
//---------------------------------------------------------------------------------
//  Build edge lights
//---------------------------------------------------------------------------------
void CAptObject::BuildEdgeOmni(LITE_MODEL &seg,CLitSYS &ls)
{ int nbl       = seg.nbo << 1;       // Twice (one light on each side)
  CBaseLITE *lit = new CBaseLITE(seg.type,nbl);
  TC_VTAB ent;
  //---Common texture coordinates -----------------------
  ent.VT_S = 0.5;
  ent.VT_T = 0.5;
  //---Colors and shape --------------------------------
  lit->col1     = seg.col1;
  lit->col2     = seg.col2;
  lit->ntex     = seg.ntex;
  //---Spot generation ----------------------------------
  for (int k = 0; k != nbl; k++)
  { double  xc  = p0.x + (arcX * total);    // X Center point
    double  yc  = p0.y + (arcY * total);    // Y Center point
    //----compute left light coordinates ---------------
    ent.VT_X = xc - egx;
    ent.VT_Y = yc + egy;
    LsComputeElevation(ent,Org);
    ent.VT_Z += 1;                          // One feet up
    lit->SpotAt(k,ent);
    //----Compute right light cordinates ----------------
    k++;
    ent.VT_X = xc + egx;
    ent.VT_Y = yc - egy;
    LsComputeElevation(ent,Org);
    ent.VT_Z += 1;                            // One feet up
    lit->SpotAt(k,ent);
    //----------------------------------------------------
    total      += seg.pace;
  }
  ls.AddLight(lit);
  return;
}
//---------------------------------------------------------------------------------
//  Build Threshold/end bar lights
//  ofs is set to the runway end position
//    hi end: ofs = hiDSP (hi displacement)
//    lo end: ofs = Lenth(rwy) - loDSP
//---------------------------------------------------------------------------------
void CAptObject::BuildDualTBAR(LITE_MODEL &seg,CLitSYS &ls)
{ int nbl   = seg.nbo << 1;               // twice object number
  CDualLITE *lit = new CDualLITE(seg.type,nbl);
  TC_VTAB ent;
  //---Common texture coordinates -----------------------
  ent.VT_S       = 0.5;
  ent.VT_T       = 0.5;
  //---Color and shape ----------------------------------
  lit->col1      = seg.col1;
  lit->col2      = seg.col2;
  lit->ntex      = seg.ntex;
  //--Compute line AB as transverse line ----------------
  lpt.x          = p0.x + (arcX * seg.nOfs);  // X Center point along runway
  lpt.y          = p0.y + (arcY * seg.nOfs);  // Y Center point along runway
  //---Decision line ------------------------------------
  StoreDecision(lit);
  //--Generate the light bar starting from edge light---------------
  int lgb = seg.wOfs;                         // Start at extremity
  for (int k = 0; k < nbl; k++)
  { double sx = (arcY * lgb * xpf);
    double sy = (arcX * lgb * rdf);
    //---Compute left light position -(at orthogonal direction) -------
    ent.VT_X = lpt.x - sx;
    ent.VT_Y = lpt.y + sy;
    LsComputeElevation(ent,Org);
    lit->SpotAt(k,ent);
    //---Compute right light position -(opposite from center) ---
    k++;
    ent.VT_X = lpt.x + sx;
    ent.VT_Y = lpt.y - sy;
    LsComputeElevation(ent,Org);
    lit->SpotAt(k,ent);
    //--Next inward position -------------------------
    lgb  -= seg.pace;
  }
  ls.AddLight(lit);
  return;
}
//---------------------------------------------------------------------------------
//  Build Longitudinal Flash BAR at runway end
//---------------------------------------------------------------------------------
void CAptObject::BuildFlashRAIL(LITE_MODEL &seg,CLitSYS &ls)
{ CStrobeLITE *lit = new CStrobeLITE(seg.type,seg.nbo);
  TC_VTAB  ent;
  //---Common texture coordinates -------------------
  ent.VT_S = 0.5;
  ent.VT_T = 0.5;
  //---Color and shape ------------------------------
  lit->col1       = seg.col1;
  lit->col2       = seg.col2;
  lit->ntex       = seg.ntex;
  //---Starting point -------------------------------
  int lgb       = seg.nOfs;
  //-------------------------------------------------
  lpt.x           = p0.x + (arcX * lgb);
  lpt.y           = p0.y + (arcY * lgb);
  //---Compute decision starting points -------------
  StoreDecision(lit);
  //---Spot generation ------------------------------
  for (int k = 0; k != seg.nbo; k++)
  { ent.VT_X = p0.x + (arcX * lgb);
    ent.VT_Y = p0.y + (arcY * lgb);
    LsComputeElevation(ent,Org);
    lit->SpotAt(k,ent);
    lgb += seg.pace;
  }
  //---Set decision increment ------------------------
  lit->ad.x = egx;
  lit->ad.y = egy;
  //---Set flash cycle duration ----------------------
  lit->SetT1(float(1)/15);
  lit->SetT2(float(4)/15);
  ls.AddLight(lit);
  return;
}
//---------------------------------------------------------------------------------
//  Build a dual flash on both sides of runway
//---------------------------------------------------------------------------------
void CAptObject::BuildFlashTBAR(LITE_MODEL &seg,CLitSYS &ls)
{ CFlashBarLITE *lit = new CFlashBarLITE(seg.type,seg.nbo);
  TC_VTAB  ent;
  //---Common texture coordinates -------------------
  ent.VT_S = 0.5;
  ent.VT_T = 0.5;
  //---Color and shape ------------------------------
  lit->col1       = seg.col1;
  lit->col2       = seg.col2;
  lit->ntex       = seg.ntex;
  //---Starting point -------------------------------
  lpt.x          = p0.x + (arcX * seg.nOfs);  // X Center point along runway
  lpt.y          = p0.y + (arcY * seg.nOfs);  // Y Center point along runway
  //---Decision line --------------------------------
  StoreDecision(lit);
  //---Spot generation ------------------------------
  int lgb = seg.wOfs;                         // Start at extremity
  for (int k = 0; k < seg.nbo; k++)
  { double sx = (arcY * lgb * xpf);
    double sy = (arcX * lgb * rdf);
    //---Compute left light position -(at orgonal direction) -------
    ent.VT_X = lpt.x - sx;
    ent.VT_Y = lpt.y + sy;
    LsComputeElevation(ent,Org);

    lit->SpotAt(k,ent);
    //---Compute right light position -(opposite from center) ---
    k++;
    ent.VT_X = lpt.x + sx;
    ent.VT_Y = lpt.y - sy;
    LsComputeElevation(ent,Org);

    lit->SpotAt(k,ent);
    //--Next inward position -------------------------
    lgb  -= seg.pace;
  }
  //---Set flash cycle duration ----------------------
  lit->SetT1(float(1)/20);
  lit->SetT2(float(0.8));
  ls.AddLight(lit);
  return;
}
//---------------------------------------------------------------------------------
//  Build a PAPI light
//---------------------------------------------------------------------------------
void CAptObject::BuildPAPIBAR(LITE_MODEL &seg,CLitSYS &ls)
{ CPapiLITE *lit = new CPapiLITE(seg.type,seg.nbo);
  TC_VTAB  ent;
  //---Common texture coordinates -------------------
  ent.VT_S = 0.5;
  ent.VT_T = 0.5;
  //---Color and shape ------------------------------
  lit->col1       = seg.col1;
  lit->col2       = seg.col2;
  lit->ntex       = seg.ntex;
  //---Starting point -------------------------------
  lpt.x          = p0.x + (arcX * seg.nOfs);  // X Center point along runway
  lpt.y          = p0.y + (arcY * seg.nOfs);  // Y Center point along runway
  //---Decision line --------------------------------
  StoreDecision(lit);
  //---Set the landing point ------------------------
  lit->lnd  = seg.lnd;
  //---Build light bar ------------------------------
  int lgb = seg.wOfs;                         // Start at extremity
  for (int k = 0; k < seg.nbo; k++)
  { double sx = (arcY * lgb * xpf);
    double sy = (arcX * lgb * rdf);
    //---Compute left light position -(at orgonal direction) -------
    ent.VT_X = lpt.x - sx;
    ent.VT_Y = lpt.y + sy;
    LsComputeElevation(ent,Org);
    lit->SpotAt(k,ent);
    lgb  -= seg.pace;
  }
  ls.AddLight(lit);
  return;
}
//---------------------------------------------------------------------------------
//  Compute maximum number of lights for threshold bar
//  nb = half width / (10 feet) + 1 edges
//---------------------------------------------------------------------------------
int CAptObject::MaxTBarLights(CRunway *rwy)
{ int hw = (rwy->GetWidthi() - lmTBARS.pace) >> 1;  // half width
  int nb = (hw / lmTBARS.pace) + 1;                 // Maximum at given feet interval
  //---Reduce number for non precision runway  ----------------
  if (rwy->NoPrecision()) nb = LiteNPR[rwy->GetWiCode()];
  return nb;
}
//---------------------------------------------------------------------------------
//  Hi Threshold bar model
//---------------------------------------------------------------------------------
int CAptObject::ThresholdBarLights(CRunway *rwy)
{ CRLP *lpf     = rwy->GetRLP();
  CLitSYS &lsys = lpf->GetBarSection();
  //---Compute number of light according to segment width -------
  U_CHAR hmd    = lpf->GetHiTBAR();
  lmTBARS.nbo   = MaxTBarLights(rwy);
  lmTBARS.nOfs  = 0;
  lmTBARS.type  = LS_BAR2_LITE;           // Dual 2 colors light
  //---Light color depends on the presence of runway end bar ----
  lmTBARS.col1  = LiteTC1[hmd];
  lmTBARS.col2  = LiteTC2[hmd];
  lmTBARS.wOfs  = rhwd + 5;               // Runway west extremity + 5 feets
  if (hmd) BuildDualTBAR(lmTBARS,lsys);
  //---Build Lo Threshold bar ------------------------------------
  U_CHAR lmd    = lpf->GetLoTBAR();
  lmTBARS.nOfs  = rwy->GetLenghi();
  lmTBARS.col1  = LiteTC1[lmd];
  lmTBARS.col2  = LiteTC2[lmd];
  if (lmd) BuildDualTBAR(lmTBARS,lsys);
  return 1;
}
//---------------------------------------------------------------------------------
//  Wing BAR lights
//---------------------------------------------------------------------------------
int CAptObject::WingBarLights(CRunway *rwy)
{ CRLP *lpf    = rwy->GetRLP();
  CLitSYS &lsys = lpf->GetBarSection();
  lmTBARS.nbo  = LiteNPR[rwy->GetWiCode()];
  lmTBARS.nOfs = hiDSP;                      // Start at displacement
  lmTBARS.type = LS_BAR2_LITE;               // Dual 2 colors light
  lmTBARS.col1 = 0;                          // Inside color
  lmTBARS.col2 = TC_GRN_LITE;                // Outside green
  lmTBARS.wOfs = rhwd + 5 + (lmTBARS.nbo * lmTBARS.pace);
  if (lpf->GetHiWBAR())  BuildDualTBAR(lmTBARS,lsys); 
  lmTBARS.nOfs = rwy->GetLenghi() - loDSP;   // Start at displacement
  if (lpf->GetLoWBAR())  BuildDualTBAR(lmTBARS,lsys);
  return 1;
}
//---------------------------------------------------------------------------------
//  Build 900 feet of TouchDown lights up to half runway
//---------------------------------------------------------------------------------
void CAptObject::BuildTDZbar(CRunway *rwy,LITE_MODEL &seg,int row)
{ CLitSYS &lsys = rwy->GetRLP()->GetTouchSection();
  for (int k = 0; k != row; k++)
  { BuildDualTBAR(seg,lsys);
    seg.nOfs += 100;            // Next double bars at 100 feet further
    total    += 100;
  }
  return;
}
//---------------------------------------------------------------------------------
//  Touch Down lights parameters
//  Build a bar every 100 feet after threshold
//  NOTE: Left extremity if from 1/4 of runway wide + 10 feets
//---------------------------------------------------------------------------------
int CAptObject::TouchDWLights(CRunway *rwy)
{ if (rwy->GetWiCode()  < 2)      return 0;
  CRLP    *lpf = rwy->GetRLP();
  lmTDZNE.nOfs = hiDSP + 100;                // Start at displacement
  lmTDZNE.wOfs = (rhwd >> 1) + (10);         // Left extremity
  int range   = (tlgr >> 1) - lmTDZNE.nOfs;  // Range available
  if (range > 900)  range = 900;             // Limit to 900 feet
  int row = range / 100;                     // Number of rows
  total   = 0;
  //---Build 900 feets or half lenght ----------------
  if (lpf->GetHiTDZN()) BuildTDZbar(rwy,lmTDZNE,row);   // Hi End
  total -= 100;                             // Last row
  lmTDZNE.nOfs = rwy->GetLenghi() - loDSP - total;
  if (lpf->GetLoTDZN()) BuildTDZbar(rwy,lmTDZNE,row);   // Lo End
  return 1;
}

//---------------------------------------------------------------------------------
//  Light Model 0: No center lights
//---------------------------------------------------------------------------------
int CAptObject::CenterLightModel0(CRunway *rwy)
{ return 0;
}
//---------------------------------------------------------------------------------
//  Ligth Model 1 (hi is CATx)
//  Omni lights up to 2700 feet from end
//  1800 feet of alternate RED WHITE
//   900 of red lights
//  Runway must be at least 2700 feet for this model to apply
//---------------------------------------------------------------------------------
int CAptObject::CenterLightModel1(CRunway *rwy)
{ if (tlgr < 2700) return CenterLightModel4(rwy);
  CLitSYS &lsys = rwy->GetRLP()->GetCenterSection();
  total         = hiDSP;
  lmOMNID.nbo   = (tlgr - 2700) / lmOMNID.pace;
  BuildOmniLBAR  (lmOMNID,lsys);
  BuildCenterDual(lmDUAL3,lsys);                // 1800 feet of alternate RED/WHITE
  BuildCenterDual(lmDUAL4,lsys);                //  900 feet of RED lights
  return 0;
}
//---------------------------------------------------------------------------------
//  Ligth Model 2 (lo is CATx)
//   900 feet of RED light
//  1800 feet of alternate RED WHITE
//  Rest  of runway as OMNI light
//  Runway must be at least 2700 feet for this model to apply
//---------------------------------------------------------------------------------
int CAptObject::CenterLightModel2(CRunway *rwy)
{ int rwl = rwy->GetLenghi() - hiDSP - loDSP;
  if (rwl < 2700) return CenterLightModel4(rwy);
  CLitSYS &lsys = rwy->GetRLP()->GetCenterSection();
  tlgr  = rwl;
  total = hiDSP;
  BuildCenterDual(lmDUAL1,lsys);                // Build part 1 (900 feet of red/white lights)
  BuildCenterDual(lmDUAL2,lsys);                // Build Part 2 (1800 feet of RED-WHITE/WHITE
  lmOMNID.nbo = (tlgr - 2700) / lmOMNID.pace;
  BuildOmniLBAR  (lmOMNID,lsys);                // Build Part 3 (rest of omni lights)
  return 0;
}

//---------------------------------------------------------------------------------
//  Ligth Model 3 (Lo and Hi are CATx)
//  Both side has red and alternate red white segments
//  Runway must be at least 5400 feet for this model to apply
//---------------------------------------------------------------------------------
int  CAptObject::CenterLightModel3(CRunway *rwy)
{ if (tlgr < 5400) return CenterLightModel4(rwy);
  CLitSYS &lsys = rwy->GetRLP()->GetCenterSection();
  total       = hiDSP;
  BuildCenterDual(lmDUAL1,lsys);                // Build part 1 (900 feet of red/white lights)
  BuildCenterDual(lmDUAL2,lsys);                // Build Part 2 (1800 feet of RED-WHITE/WHITE
  lmOMNID.nbo = (tlgr - 5400) / lmOMNID.pace;
  BuildOmniLBAR  (lmOMNID,lsys);                // Section of white light
  BuildCenterDual(lmDUAL3,lsys);                // 1800 feet of alternate RED/WHITE
  BuildCenterDual(lmDUAL4,lsys);                //  900 feet of RED lights
  return 0;
}
//---------------------------------------------------------------------------------
//  Ligth model 4
//  Row of omni lights all the way
//---------------------------------------------------------------------------------
int  CAptObject::CenterLightModel4(CRunway *rwy)
{ CLitSYS &lsys = rwy->GetRLP()->GetCenterSection();
  total         = hiDSP;
  lmOMNID.nbo   = tlgr  / lmOMNID.pace;
  BuildOmniLBAR (lmOMNID,lsys);
  return 0;
}

//---------------------------------------------------------------------------------
//  Light Model 0: No edge lights
//---------------------------------------------------------------------------------
int  CAptObject::EdgeLightModel0(CRunway *rwy)
{   return 0;
}
//---------------------------------------------------------------------------------
//  Light Model 1: Hi end = CATx
//  white light up to 1800 feet (600 m) from the end
//  then all yellow lights
//---------------------------------------------------------------------------------
int CAptObject::EdgeLightModel1(CRunway *rwy)
{ if (tlgr < 1800) return EdgeLightModel4(rwy);
  CLitSYS &lsys = rwy->GetRLP()->GetEdgeSection();
  total = hiDSP;
  lmEDGE1.nbo = (tlgr - 1800) / lmEDGE1.pace;
  BuildEdgeOmni(lmEDGE1,lsys);                // White section
  BuildEdgeOmni(lmEDGE2,lsys);                // Rest of runway
  return 0;
}
//---------------------------------------------------------------------------------
//  Light Model 2: Lo end = CATx
//  1800 feet (600m) of YELLOW lights
//  then all white light
//---------------------------------------------------------------------------------
int  CAptObject::EdgeLightModel2(CRunway *rwy)
{ if (tlgr < 1800) return EdgeLightModel4(rwy);
  CLitSYS &lsys = rwy->GetRLP()->GetEdgeSection();
  total = hiDSP;
  BuildEdgeOmni(lmEDGE2,lsys);                // Yellow section
  lmEDGE1.nbo = (tlgr - total) / lmEDGE1.pace;
  BuildEdgeOmni(lmEDGE1,lsys);                // Rest of runway
  return 0;
}
//---------------------------------------------------------------------------------
//  Light Model 3: both end are CATx
//  2 yellow sections of 1800ft at both ends and one  white mid section
//---------------------------------------------------------------------------------
int  CAptObject::EdgeLightModel3(CRunway *rwy)
{ if (tlgr < 1800) return EdgeLightModel4(rwy);
  CLitSYS &lsys = rwy->GetRLP()->GetEdgeSection();
  total = hiDSP;
  BuildEdgeOmni(lmEDGE2,lsys);                // Yellow section 1
  lmEDGE1.nbo = (tlgr - 3600) / lmEDGE1.pace;
  BuildEdgeOmni(lmEDGE1,lsys);                // White section
  BuildEdgeOmni(lmEDGE2,lsys);                // Yellow section 2
  return 0;
}
//---------------------------------------------------------------------------------
//  All white light
//---------------------------------------------------------------------------------
int  CAptObject::EdgeLightModel4(CRunway *rwy)
{ CLitSYS &lsys = rwy->GetRLP()->GetEdgeSection();
  total = hiDSP;
  lmEDGE1.nbo = tlgr  / lmEDGE1.pace;
  BuildEdgeOmni(lmEDGE1,lsys);                // Rest of runway
  return 0;
}
//---------------------------------------------------------------------------------
//  REIL system
//	Hi end => dis = 0
//	Lo end => dis = runway length
//---------------------------------------------------------------------------------
int CAptObject::REILSystem(CRunway *rwy,int dis)
{ CLitSYS &lsys = rwy->GetRLP()->GetApproachSection();
  lmREILa.nOfs = 0;
  lmREILa.wOfs = rhwd + 40;
  BuildFlashTBAR(lmREILa,lsys);
  return 1;
}

//---------------------------------------------------------------------------------
//  Hi ODAL system
//---------------------------------------------------------------------------------
int CAptObject::HiODALSystem(CRunway *rwy)
{ CLitSYS &lsys = rwy->GetRLP()->GetApproachSection();
  BuildFlashRAIL(lmODALS,lsys);
	REILSystem(rwy,0);
  return 0;
}
//---------------------------------------------------------------------------------
//  Lo ODAL system
//---------------------------------------------------------------------------------
int CAptObject::LoODALSystem(CRunway *rwy)
{ CLitSYS &lsys = rwy->GetRLP()->GetApproachSection();
  lmODALN.nOfs = 270 + rwy->GetLenghi();
  BuildFlashRAIL(lmODALN,lsys);
	REILSystem(rwy,rwy->GetLenghi());
  return 0;
}
//---------------------------------------------------------------------------------
//  Hi SSALR system
//  7  white barettes up to 1400 feet
//  5  flashers       up to 2400 feet
//---------------------------------------------------------------------------------
int CAptObject::HiSALRSystem(CRunway *rwy)
{ int lgb = -200;
  CLitSYS &lsys = rwy->GetRLP()->GetApproachSection();
  //---- 7 center barettes ------------------
  for (int k=0; k!=7; k++)  
  { lmSALR3.nOfs = lgb;
    BuildDualTBAR(lmSALR3,lsys);
    lgb -= 200;
  }
  //---Build the 1000 feet barette ---------
  lmSALR5.nOfs = -1000;
  BuildDualTBAR(lmSALR5,lsys);
  //---Build flasher up to 2400 feet -------
  lmSALRF.nOfs = -1400;
  lmSALRF.pace = -270;
  BuildFlashRAIL(lmSALRF,lsys);
  return 1;
}
//---------------------------------------------------------------------------------
//  Hi SSALR system
//  7  white barettes up to 1400 feet
//  5  flashers       up to 2400 feet
//---------------------------------------------------------------------------------
int CAptObject::LoSALRSystem(CRunway *rwy)
{ CLitSYS &lsys = rwy->GetRLP()->GetApproachSection();
  int lgb = rwy->GetLenghi() + 200;
  //---- 7 center barettes ------------------
  for (int k=0; k!=7; k++)  
  { lmSALR3.nOfs = lgb;
    BuildDualTBAR(lmSALR3,lsys);
    lgb += 200;
  }
  //---Build the 1000 feet barette ---------
  lmSALR5.nOfs = rwy->GetLenghi() + 1000;
  BuildDualTBAR(lmSALR5,lsys);
  //---Build flasher up to 2400 feet -------
  lmSALRF.nOfs = rwy->GetLenghi() + 1400;
  lmSALRF.pace = +270;
  BuildFlashRAIL(lmSALRF,lsys);
  return 1;
}
//---------------------------------------------------------------------------------
//  ALSF1 system
//---------------------------------------------------------------------------------
int CAptObject::HiALSF1System(CRunway *rwy)
{ //---Build 24 white barettes up to 2400 feet -----------------------
  CLitSYS &lsys = rwy->GetRLP()->GetApproachSection();
  int lgb = -100;
  //---- 24 center barettes ------------------
  for (int k=0; k!=24; k++)  
  { lmALSFa.nOfs = lgb;
    BuildDualTBAR(lmALSFa,lsys);
    lgb -= 100;
  }
  //---Build the 1000 feet barette ---------
  lmALSFb.nOfs = -1000;
  BuildDualTBAR(lmALSFb,lsys);
  //---Build flasher up to 2400 feet -------
  lmALSFc.nOfs = -1050;
  lmALSFc.pace =  -100;
  BuildFlashRAIL(lmALSFc,lsys);
  //---Build RED at 200 feets back ---------
  lmALSFd.nOfs = -200;
  lmALSFd.wOfs = (rhwd >> 1) + 10;
  BuildDualTBAR(lmALSFd,lsys);
    //---Build RED at 200 feets back ---------
  lmALSFd.nOfs = -100;
  BuildDualTBAR(lmALSFd,lsys);
  return 1;
}
//---------------------------------------------------------------------------------
//  ALSF1 system
//---------------------------------------------------------------------------------
int CAptObject::LoALSF1System(CRunway *rwy)
{ //---Build 24 white barettes up to 2400 feet -----------------------
  CLitSYS &lsys = rwy->GetRLP()->GetApproachSection();
  int lgr = rwy->GetLenghi();
  int lgb = lgr + 100;
  //---- 24 center barettes ------------------
  for (int k=0; k!=24; k++)  
  { lmALSFa.nOfs = lgb;
    BuildDualTBAR(lmALSFa,lsys);
    lgb += 100;
  }
  //---Build the 1000 feet barette ---------
  lmALSFb.nOfs = lgr + 1000;
  BuildDualTBAR(lmALSFb,lsys);
  //---Build flasher up to 2400 feet -------
  lmALSFc.nOfs = lgr + 1050;
  lmALSFc.pace =  +100;
  BuildFlashRAIL(lmALSFc,lsys);
  //---Build RED at 200 feets back ---------
  lmALSFd.nOfs = lgr + 200;
  lmALSFd.wOfs = (rhwd >> 1) + 10;
  BuildDualTBAR(lmALSFd,lsys);
    //---Build RED at 200 feets back ---------
  lmALSFd.nOfs = lgr + 100;
  BuildDualTBAR(lmALSFd,lsys);

  return 1;
}
//---------------------------------------------------------------------------------
//  ALSF2 system
//---------------------------------------------------------------------------------
int CAptObject::HiALSF2System(CRunway *rwy)
{ //---Build 24 white barettes up to 2400 feet -----------------------
  CLitSYS &lsys = rwy->GetRLP()->GetApproachSection();
  int lgb = -100;
  //---- 24 center barettes ------------------
  for (int k=0; k!=24; k++)  
  { lmALSFa.nOfs = lgb;
    BuildDualTBAR(lmALSFa,lsys);
    lgb -= 100;
  }
  //---Build the  500 feet barette ---------
  lmALSFe.nOfs =  -500;
  BuildDualTBAR(lmALSFe,lsys);
  //---Build the 1000 feet barette ---------
  lmALSFb.nOfs = -1000;
  BuildDualTBAR(lmALSFb,lsys);
  //---Build flasher up to 2400 feet -------
  lmALSFf.nOfs = -1050;
  lmALSFf.pace =  -100;
  BuildFlashRAIL(lmALSFf,lsys);
  //---Build 9 rows of red light -----------
  lmALSFd.nOfs = -100;
  lmALSFd.wOfs = (rhwd >> 1) + 10;
  for (int k=0; k!=9; k++)
  { BuildDualTBAR(lmALSFd,lsys);
    lmALSFd.nOfs -= 100;
  }
  return 1;
}

//---------------------------------------------------------------------------------
//  ALSF2 system
//---------------------------------------------------------------------------------
int CAptObject::LoALSF2System(CRunway *rwy)
{ //---Build 24 white barettes up to 2400 feet -----------------------
  CLitSYS &lsys = rwy->GetRLP()->GetApproachSection();
  int lgr = rwy->GetLenghi();
  int lgb = lgr + 100;
  //---- 24 center barettes ------------------
  for (int k=0; k!=24; k++)  
  { lmALSFa.nOfs = lgb;
    BuildDualTBAR(lmALSFa,lsys);
    lgb += 100;
  }
  //---Build the  500 feet barette ---------
  lmALSFe.nOfs = lgr + 500;
  BuildDualTBAR(lmALSFe,lsys);
  //---Build the 1000 feet barette ---------
  lmALSFb.nOfs = lgr + 1000;
  BuildDualTBAR(lmALSFb,lsys);
  //---Build flasher up to 2400 feet -------
  lmALSFf.nOfs = lgr + 1050;
  lmALSFf.pace =  +100;
  BuildFlashRAIL(lmALSFf,lsys);
  //---Build 9 rows of red light -----------
  lmALSFd.nOfs = lgr + 100;
  lmALSFd.wOfs = (rhwd >> 1) + 10;
  for (int k=0; k!=9; k++)
  { BuildDualTBAR(lmALSFd,lsys);
    lmALSFd.nOfs += 100;
  }
  return 1;
}
//---------------------------------------------------------------------------------
//  Build Hi end PAPI system to the left side of runway
//  The extrem left light is at 128 feet from runway side
//---------------------------------------------------------------------------------
int CAptObject::HiPAPI4System(CRunway *rwy)
{ CRLP *lpf     = rwy->GetRLP();
  lmPAPI4.nOfs  = rwyMARGE[rwy->GetLgCode()] + hiDSP;
  lmPAPI4.wOfs  = rhwd + 128;
  lmPAPI4.lnd   = rwy->GetLandPos(RWY_HI_END);
  BuildPAPIBAR(lmPAPI4,lpf->GetPapiSystem());
  return 1;
}
//---------------------------------------------------------------------------------
//  Build Lo end PAPI system to the left side of runway
//  The first left light is at 45 feet from runway side
//---------------------------------------------------------------------------------
int CAptObject::LoPAPI4System(CRunway *rwy)
{ CRLP *lpf     = rwy->GetRLP();
  lmPAPI4.nOfs  = rlgt - loDSP - rwyMARGE[rwy->GetLgCode()];
  lmPAPI4.wOfs  = - (rhwd + 45);
  lmPAPI4.lnd   = rwy->GetLandPos(RWY_LO_END);
  BuildPAPIBAR(lmPAPI4,lpf->GetPapiSystem());
  return 1;
}

//---------------------------------------------------------------------------------
//  Build Approach Lights
//---------------------------------------------------------------------------------
int CAptObject::ApproachLight(CRunway *rwy)
{ CRLP *lpf = rwy->GetRLP();
  int papi  = 0;
  switch (lpf->GetHiAPRL()) {
  case TC_APR_REIL:
		REILSystem(rwy,0);
    break;
  case TC_APR_ODAL:
    HiODALSystem(rwy);
    break;
  case TC_APR_ALSR:
    HiSALRSystem(rwy);
    break;
  case TC_APR_ALF1:
    HiALSF1System(rwy);
    break;
  case TC_APR_ALF2:
    HiALSF2System(rwy);
    break;
  }
  //---Check for lower end -----------------
  switch(lpf->GetLoAPRL()) {
  case TC_APR_REIL:
		REILSystem(rwy,rwy->GetLenghi());
    break;
  case TC_APR_ODAL:
    LoODALSystem(rwy);
    break;
  case TC_APR_ALSR:
    LoSALRSystem(rwy);
    break;
  case TC_APR_ALF1:
    LoALSF1System(rwy);
    break;
  case TC_APR_ALF2:
    LoALSF2System(rwy);
    break;
  }
  //---Check for PAPI light ---------------
  papi = lpf->GetHiPAPI();
  if (papi == TC_PAPI_4L) HiPAPI4System(rwy);
  papi = lpf->GetLoPAPI();
  if (papi == TC_PAPI_4L) LoPAPI4System(rwy);
  return 0;
}
//=================================================================================
//	AIRPORT GROUND MANAGEMENT
//	NOTE: Airport ground must be drawed first as tarmac, bands, letters, taxiway, 
//				etc, are set uppon the ground with no Z buffer test.  Mountains in near
//				range may overlay an airport drawing. If we where drawing tarmacs after
//			  drawin all the terrain, then airport features  would be placated uppon 
//				the near mountain.
//---------------------------------------------------------------------------------
//  Locate the set of ground tiles
//  Bounding coordinates are relatives to airport origin
//---------------------------------------------------------------------------------
void CAptObject::LocateGround()
{ CmQUAD *qdl = globals->tcm->GetTileQuad(glim.xmin,glim.zmin);
  CmQUAD *qdu = globals->tcm->GetTileQuad(glim.xmax,glim.zmax);
  MarkGround(glim);           // Mark ground tiles
  //-----Compute bounding box absolute coordinates------------
  CVertex *sw = qdl->GetCorner(TC_SWCORNER);
  CVertex *ne = qdu->GetCorner(TC_NECORNER);
  double   x0 = sw->GetAbsoluteLongitude();
  double   y0 = sw->GetWY();
  double   x1 = ne->GetAbsoluteLongitude();
  double   y1 = ne->GetWY();
  //-----Compute extension in feet ------------------------
  gBound.x = (x1 - x0) * FN_FEET_FROM_ARCS(1);
  gBound.y = (y1 - y0) * FN_FEET_FROM_ARCS(1);
  gBound.z = 0;
  //-----Adjust to airport origin -----------------------------
  x0       = LongitudeDifference(x0,Org.lon);
  y0       = y0 - Org.lat;
  x1       = LongitudeDifference(x1,Org.lon);
  y1       = y1 - Org.lat;
  //----Center coordinates -------------------------------------
  center.x = (x0 + x1) * 0.5;               // X relative airport
  center.y = (y0 + y1) * 0.5;               // Y relative airport
  center.z = 0;                             // Z absolute elevation
  return;
}
//---------------------------------------------------------------------------------
//  Draw airport grid (Used for test only)
//---------------------------------------------------------------------------------
void CAptObject::DrawGrid()
{ double x0 = llc.lon - Org.lon;
  double y0 = llc.lat - Org.lat;
  glColor3f(1,0,0);
  glBegin(GL_LINE_LOOP);
  glVertex3f(x0,y0,0);
  glVertex3f(x0,0,0);
  glVertex3f(0,0,0);
  glVertex3f(0,y0,0);
  glEnd();
}
//-----------------------------------------------------------------------
//  Mark the ground tiles
//	Number of vertice is collected for VBO allocation
//-----------------------------------------------------------------------
void CAptObject::MarkGround(TC_BOUND &bnd)
{ U_INT  txe = TC_NEXT_INDICE(bnd.xmax);
  U_INT  tze = TC_NEXT_INDICE(bnd.zmax);
  U_INT   lz = 0;
  U_INT   cx = 0;
  CTextureDef *txn = 0;
  TCacheMGR *tcm = globals->tcm;
  //-----Mark all tiles making airport ground ----
  for   (lz = bnd.zmin; lz != tze; lz = TC_NEXT_INDICE(lz))
  { for (cx = bnd.xmin; cx != txe; cx = TC_NEXT_INDICE(cx))
    { //---Add a ground tile to list ------------
      CGroundTile *gnd = new CGroundTile(cx,lz);
			tcm->FillGroundTile(gnd);
      grnd.push_back(gnd);
    }
  }
  return;
}
//-----------------------------------------------------------------------
//  Build ground VBO
//	We build a VBO for all ground tiles.  
//	NOTE: Vertices in Tile are allocated when the tile enters the inner ring
//				of visibility.  If only one tile is not yet ready, the VBO build
//				is deffered until the tile is ready.  
//				Without VBO, the airport is not drawed, but this is not important
//				as this is occurring for airport in the remote ring of visibility
//	
//-----------------------------------------------------------------------
void CAptObject::BuildGroundVBO()
{	int tot = 0;
	std::vector<CGroundTile*>::iterator it;
  for (it = grnd.begin();it != grnd.end(); it++)
	{	CGroundTile *gnd	= (*it); 
		int nbv	= gnd->GetNbrVTX();
		if (nbv == 0)	return;
		tot += nbv;
	}
	if (0 == tot)		return;
	//--- Now allocate the ground VBO --------------
	gRES  = 0;
	nGVT  = tot;
	gBUF	= new TC_GTAB[tot];
	FillGroundVBO();
	//--- Request handle from OpenGL ---------------
	int dim = tot * sizeof(TC_GTAB);
	glBindBuffer(GL_ARRAY_BUFFER,gVBO);
	glBufferData(GL_ARRAY_BUFFER,dim,gBUF,GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER,0);
	return;
}
//-----------------------------------------------------------------------
//	Fill the ground VBO
//-----------------------------------------------------------------------
void CAptObject::FillGroundVBO()
{	TC_GTAB   *dst = gBUF;
	int				 inx = 0;
	int        tot = 0;
//TRACE("FillVBO for %s  org.x= %.5f org.y = %.5f",Airp->GetAptName(),Org.lon,Org.lat);
	std::vector<CGroundTile*>::iterator it;
	for (it = grnd.begin();it != grnd.end(); it++)
	{	CGroundTile *gnd	= (*it);
		int nbv = gnd->TransposeTile(dst,inx,&Org);
		dst		 += nbv;
		inx    += nbv;
		tot    += nbv;
	}
	if (tot != nGVT)	gtfo("FillGroundVBO Count error");
	return;
}

//-----------------------------------------------------------------------
//  UnMark the ground tiles
//  NOTE:  Check for txn existence (teleport case)
//-----------------------------------------------------------------------
void CAptObject::UnmarkGround()
{ std::vector<CGroundTile*>::iterator it;
  for (it = grnd.begin();it != grnd.end(); it++)
    { CGroundTile *gnd = (*it);
	    gnd->Free();
      delete gnd;
    }
  grnd.clear();
  return;
}
//-----------------------------------------------------------------------
//  Swap light control
//  Defer order if a commande is already running
//-----------------------------------------------------------------------
void CAptObject::ChangeLights(char ls)
{ if (lrwy)     return;                // Defer
  CAirport *apt = (CAirport*)pApt.Pointer();
  lrwy  = apt->GetNextRunway(0);
  lTim  = 0;
  swlt  = ls;
  return;
}
//-----------------------------------------------------------------------
//  Update lights
//  
//-----------------------------------------------------------------------
void CAptObject::UpdateLights(float dT)
{//---Process lighting order ---------------------------
  lTim += 1;
  if (lTim & 0x01)        return;
  lTim  = 0;
  char ok = lrwy->ChangeLights(swlt);
  if (ok)                 return;
  lrwy    = lrwy->GetNext();
  return;
}
//-----------------------------------------------------------------------
//	Read Taxiway nodes
//-----------------------------------------------------------------------
void CAptObject::ReadTaxiNodes()
{ char  pn[PATH_MAX];
	char *fn = Airp->GetIdentity();
	//--- Check if already open ---------------
	bgr		= 0;
  _snprintf(pn,64,"DATA/%s.BGR",fn);
  SStream s;
  if (OpenRStream(pn,s) == 0)    return;
  CStreamFile *sf = (CStreamFile*)s.stream;
  txBGR = new CDataBGR(this);
	sf->ReadFrom (txBGR);
  CloseStream (&s);
  return;
}

//-----------------------------------------------------------------------
//  Time slice
//  
//-----------------------------------------------------------------------
void CAptObject::TimeSlice(float dT)
{ //-----------------------------------------------------	
	//--- Process BGR request -----------------------------
	if (bgr)	ReadTaxiNodes();
	//--- Process Ground resolution -----------------------
	if (0 == gBUF)	BuildGroundVBO();
	//--- Process lighting order --------------------------
	float lum = globals->tcm->GetLuminosity();
  if (lrwy)           return UpdateLights(dT) ;
  bool nowl     = (lum < 1.2145) || (lreq);       //
  bool oldl     = (swlt != 0);
  if (nowl == oldl)   return;
  //--- Start to change light --------------------------
  swlt          = nowl;
  CAirport *apt = (CAirport*)pApt.Pointer();
  lrwy  = apt->GetNextRunway(0);
  lTim  = 0;
  return;
}
//==================================================================================
//  For each runway, build a light profile (RLP) and a set of segment polygons
//  NOTE: All pavements use the common ground from airport position
//        All coordinates are in arcsec, relative to airport origin
//==================================================================================
int CAirportMgr::SetRunwayProfile(CAirport *apt)
{ U_CHAR mod  = 0;
  char  *fn   = 0;
  nProf       = 0;                            // Stack empty
  CRLParser prs(this,apt->GetAptName());      // Read user profiles
  CRunway  *rwy = 0;
  for (rwy = apt->GetNextRunway(rwy); rwy != 0;rwy = apt->GetNextRunway(rwy))
	{ if (rwy->HasProfile())  {CRLP *old = rwy->GetRLP(); delete old;} //return 0;
    //---Build the runway default profile -----------------------
    CRLP    *lpf = new CRLP(rwy);             // Build runway light profile
    //----Locate Hi and Lo user profile -------------------------
    RWY_EPF *hip = GetEndProfile(rwy->GetHiEnd());
    if (hip)  lpf->MergeHiUser(hip);
    RWY_EPF *lop = GetEndProfile(rwy->GetLoEnd());
    if (lop)  lpf->MergeLoUser(lop);
    rwy->SetRLP(lpf);                         // Store in runway
  }
  return 1;
}
//---------------------------------------------------------------------------------
//  Add profil to global table
//---------------------------------------------------------------------------------
void CAirportMgr::AddProfile(RWY_EPF &epf)
{ rwyRLP[nProf++] = epf;
  return;
}
//---------------------------------------------------------------------------------
//  Locate end profile
//---------------------------------------------------------------------------------
RWY_EPF *CAirportMgr::GetEndProfile(char *id)
{ for (int k=0; k<=nProf;k++) if (strcmp(rwyRLP[k].end,id) == 0)  return (rwyRLP + k);
  return 0;
}
//-----------------------------------------------------------------------------------------
//  Draw from a VBO  
//-----------------------------------------------------------------------------------------
void CAptObject::DrawVBO(U_INT vbo,U_INT n)
{	if (0 == vbo)	return;
  glBindBuffer(GL_ARRAY_BUFFER,vbo);
	glVertexPointer  (3,GL_FLOAT,sizeof(TC_VTAB),OFFSET_VBO(2*sizeof(float)));
  glTexCoordPointer(2,GL_FLOAT,sizeof(TC_VTAB),0);
	glDrawArrays(GL_TRIANGLES,0,n);
	return;
}
//---------------------------------------------------------------------------------
//  Draw the Airport  Objects
//  All coordinates are in arcseconds relative to airport origin
//  A translation is made from aircarft location to airport location, then
//  all objects are drawed relative to the airport origin
//        A)The ground tiles are first drawed, using the same transformations
//          as the terrain one.
//        B)Then taxiways and runway are placated on the tiles
//        C) Then other aspects of terrain are rendered
//         This order is important.  If not respected, taxiway and runway
//         would be see through mountains in certain situations
//        D) The last pass is dedicated to draw the lights
//
//  NOTES: During Step A and B above, the deepth buffer is not written.  This speeds
//         up rendering and avoid flickering effect when 3D airport draws on the
//         airport ground (pavement, taxiways, etc);
//         
//         Taxiway Pavement and runway should be rendered together as they are
//         Textured. The order is also important
//         Edges and center lines are rendered when altitude is under a certain value
//         to avoid flickering.
//---------------------------------------------------------------------------------
void CAptObject::Draw()
{ TCacheMGR *tcm = globals->tcm;
  //----Compute translation offset from aircraft to airport origin --------
  ofap.x  = LongitudeDifference(Org.lon,apos.lon);
  ofap.y  = Org.lat - apos.lat;
  ofap.z  = Org.alt - apos.alt;
  Alt     = -ofap.z;
  //----Cull airport based on ground tile corners -------------------------
  glPushMatrix();                                   // Mark T1
  glTranslated(ofap.x, ofap.y, ofap.z);             // Camera to airport
  tcm->ComputeClip();
  visible = tcm->BoxInView(center,gBound);
  glPopMatrix();                                    // Back to T1
  if (!visible)           return;
  //-------Draw the ground tiles first ------------------------------------
	glDisable(GL_BLEND);
  glEnable(GL_TEXTURE_2D);
  if (gBUF)	DrawGround();						// Draw the airport tiles first
  if (globals->noAPT)     return;
  //-----Prepare taxiway drawing ------------------------------------------
  SetAlphaColor(alpha);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(false);
  glPolygonMode (GL_FRONT, GL_FILL);          
  glPushMatrix();                                   // Mark T1
  glTranslated(ofap.x,ofap.y,ofap.z);               // Camera at airport origin
  //-----Draw all pavements -----------------------------------------------

  glFrontFace(GL_CW);
  glColor4fv(white);
  glBindTexture(GL_TEXTURE_2D,oTAXI);
	//if (tr) TRACE("TCM: --Draw pavement %s",Airp->GetName());
	//if (tr) TRACE("pVBO =%d , nPAV = %d",pVBO,nPAV);
	DrawVBO(pVBO,nPAV);
	//if (tr) TRACE("TCM: --Leave pavement");
  //-----Draw all edges ---------------------------------------------------
	if (nmiles < 2)
  { apm->BindYellow();
		DrawVBO(eVBO,nEDG);
  }
  //-----Draw runways -----------------------------------------------------
	glBindBuffer(GL_ARRAY_BUFFER,0);

  glColor4fv(white);
	std::vector<CTarmac*>::iterator tm;
	for(tm=tmcQ.begin(); tm!=tmcQ.end(); tm++) (*tm)->Draw();
  //-----Draw Center marks if distance < 2Nm  -----------------------------
	if (nmiles < 2)
  { apm->BindYellow();;
		DrawVBO(cVBO,nCTR);
  }
  glDepthMask(true);
  //-----------------------------------------------------------------------
  // Restore states
  //-----------------------------------------------------------------------
  glEnable(GL_DEPTH_TEST);
  glFrontFace(GL_CCW);
  //-----------------------------------------------------------------------
  glPopMatrix ();                           // Back to T1
  //-----------------------------------------------------------------------
  ResetAlphaColor();
  if (sta3D == TC_AP_FADE_IN) Update3Dstate();

  //----Check error -------------------------------------------------------
 /*
  {GLenum e = glGetError ();
   if (e != GL_NO_ERROR) 
    WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  }
 */
  return;
}
//-----------------------------------------------------------------------------------------
//  Pre Draw:  Called by camera  
//-----------------------------------------------------------------------------------------
void CAptObject::PreDraw(CCamera *cam)
{ glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
  glMaterialfv (GL_FRONT, GL_EMISSION, tcm->GetDeftEmission()); 
  glEnable(GL_TEXTURE_2D);
  glDepthMask(false);
  glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);          
  glShadeModel(GL_FLAT);
  return;
}
//-----------------------------------------------------------------------------------------
//  Draw for VectorMap:  Called by camera to render runways
//  NOTE: Camera is supposed to be at airport origin
//-----------------------------------------------------------------------------------------
void CAptObject::CamDraw(CCamera *ac)
{ SVector    trs;
  //----Compute translation to airport origin -----------------------------
  trs.x = LongitudeDifference(Org.lon,ac->GetTargetLon());
  trs.y = Org.lat - ac->GetTargetLat();
  trs.z = Org.alt;
	//--- Prepare client state ----------------------------------------------
  glPushClientAttrib (GL_CLIENT_ALL_ATTRIB_BITS);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  //-----Prepare taxiway drawing ------------------------------------------
  float *white = (float*)&ColorTAB[TC_WHI_LITE];
  glScaled(scale->x,scale->y, 1.0);                 // T1 Scale X,Y to feet coordinate
  glPushMatrix();
  glTranslated(trs.x,trs.y,trs.z);                  // Airport origin
  glFrontFace(GL_CW);
  
  //-----Draw all pavements -----------------------------------------------
  glBindTexture(GL_TEXTURE_2D,oTAXI);
  glColor4fv(white);
	DrawVBO(pVBO,nPAV);
  //-----Draw all edges ---------------------------------------------------
	apm->BindYellow();
  DrawVBO(eVBO,nEDG);
  //-----Draw runways -----------------------------------------------------
	glBindBuffer(GL_ARRAY_BUFFER,0);
	std::vector<CTarmac*>::iterator tm;
	for(tm=tmcQ.begin(); tm!=tmcQ.end(); tm++) (*tm)->Draw();
  //-----Draw Center marks -------------------------------------------------
  apm->BindYellow();
  DrawVBO(cVBO,nCTR);
  glBindBuffer(GL_ARRAY_BUFFER,0);
	//--- Restore state ----------------------------------------------------
  glPopClientAttrib();
  //-----Translate to aircraft . Put an icon on tarmac ground --------------
  float  elv = tcm->GetGroundAltitude();    // ground at aircraft position
  glFrontFace(GL_CCW);
  glColor4fv(yellow);
  trs.x  = LongitudeDifference(apos.lon,ac->GetTargetLon());
  trs.y  = apos.lat - ac->GetTargetLat();
  trs.z  = elv - ac->GetTargetAlt();
  //--- Get Aircraft heading  ------------------------------------------
  double rot = globals->dang.z;
  glPopMatrix();                            // Back to aircraft position
  glTranslated(trs.x,trs.y,trs.z);          // Plane origin
  glScaled(xpf,1.0, 1.0);                   // T1 Scale X,Y to feet coordinate
  glRotated(rot,0,0,1);
  //--- Draw pictogram --------------------------------------------------
  CPicQUAD *icp = apm->GetIcon();
  icp->SetElevation(elv);
  glEnable(GL_BLEND);
  icp->Draw();
  return;
}
//-----------------------------------------------------------------------------------------
//  End Draw by camera.  
//-----------------------------------------------------------------------------------------
void CAptObject::EndDraw(CCamera *cam)
{ glPopAttrib();
  glPopClientAttrib ();
  return;
}
//==========================================================================================
//	DRAW GROUND
//	Camera is set at airport position
//	NOTE:  Due to a precision problem, we must draw terrain elevation as a near airport
//				object, otherwise, ground and lights are flickering and this is a visible
//				artefact.  To do so, all tiles that belong to an airports are isolated and
//				relative coordinates (to airport center) are computed, and tiles are drawed
//				first. However, as second artifact is also present, at the junction of 
//				airport Tiles and normal terrain. So we must draw draw over hte texture agin
//				this time, using the normal terrain coordinates.
//				May be some better way is to be found to get better performances.
//==========================================================================================
void CAptObject::DrawGround()
{ glMaterialfv (GL_FRONT, GL_EMISSION, tcm->GetDeftEmission()); 
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
  glFrontFace(GL_CCW);
	glPushMatrix();
	glTranslated(ofap.x, ofap.y, ofap.z);			// Camera to airport
	//--- Activate ground VBO ------------------------------
	glBindBuffer(GL_ARRAY_BUFFER,gVBO);
  glTexCoordPointer(2,UNIT_OPENGL,sizeof(TC_GTAB),0);
	glVertexPointer  (3,UNIT_OPENGL,sizeof(TC_GTAB),OFFSET_VBO(2*UNIT_SIZE));
	//-------------------------------------------------------
	std::vector<CGroundTile*>::iterator it;
	for (it = grnd.begin();it != grnd.end(); it++)
	{	CGroundTile *gnd	= (*it);
	  gnd->DrawGround(0);
	}
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glPopMatrix();
	//-------------------------------------------------------
	glDisable(GL_DEPTH_TEST);
	DrawTextureGround();
	glEnable(GL_DEPTH_TEST);
	return;
}
//-----------------------------------------------------------------------------------------
//  update alpha chanel
//-----------------------------------------------------------------------------------------
void CAptObject::Update3Dstate()
{ alpha += 0.005f;
  if (alpha < 1)  return;
  alpha = 1;
  sta3D = TC_AP_IN_VIEW;
  return;
}
//-----------------------------------------------------------------------------------------
//  Compute offset of position to airport origin in arcsecs
//-----------------------------------------------------------------------------------------
void CAptObject::Offset(SPosition &p, SVector &v)
  {v.x = LongitudeDifference(p.lon,Org.lon);
   v.y = p.lat - Org.lat;
   v.z = p.alt - Org.alt;
  }
//-----------------------------------------------------------------------------------------
//  Draw all lights when airport is visible.
//  All light systems are relative to airport origin
//  Lights must be drawed as last objects due to the transparency of halo
//-----------------------------------------------------------------------------------------
void CAptObject::DrawLights(CCamera *cc)
{ if (cam != cc) cam = cc;
  glPushMatrix();                                   // Mark Matrix
  glScaled(scale->x,scale->y, 1.0);                 // T1 Scale X,Y to feet coordinate
  glTranslated(ofap.x,ofap.y,ofap.z);               // Camera at Airport origin
  //-----------------------------------------------------------------------
  SetCameraPosition();
  glPushAttrib(GL_POINT_BIT);
	//---- Set Drawing conditions -------------------------------------------
	GLfloat p[] = {0.0f,0.01f,0.00001f};
  glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION,p);
  glPointParameterf (GL_POINT_SIZE_MIN,1);
  glPointParameterf (GL_POINT_SIZE_MAX,64);
  glPointSize(64);
  glEnable(GL_POINT_SPRITE);
  glTexEnvi(GL_POINT_SPRITE,GL_COORD_REPLACE,GL_TRUE);
  //-----Draw runways light -----------------------------------------------
  CRunway *rwy = 0;
  for (rwy = Airp->GetNextRunway(rwy); rwy != 0;rwy = Airp->GetNextRunway(rwy))
  { CRLP *lpf = rwy->GetRLP();
    if (0 == lpf)                 continue;
		lpf->DistanceNow(nmiles);
    lpf->DrawLITE(cpos);                      // Draw night lights
  }
  //----Draw Taxiway lights -----------------------------------------------
	if (swlt && (nmiles < 4))			taxS.DrawSystem(cpos);
  if (swlt)                     becS.DrawSystem(cpos);
  //-----------------------------------------------------------------------
  glPopAttrib();
  glPopMatrix();
  return;
}
//-----------------------------------------------------------------------------------------
//  Return camera position (X,Y) in arcsec relative to airport origin
//  Camera position is used to compute correct color of Dual colored light
//  that depends on the camera position
//-----------------------------------------------------------------------------------------
void CAptObject::SetCameraPosition()
{ cam->GetOffset(cpos);                          // In feet relative to aircraft
  cpos.x = AddLongitude(FN_ARCS_FROM_FEET(cpos.x) * xpf,apos.lon);
  cpos.y = FN_ARCS_FROM_FEET(cpos.y) + apos.lat;
  cpos.x = LongitudeDifference(cpos.x,Org.lon);
  cpos.y = cpos.y - Org.lat;
  return;
}
//-----------------------------------------------------------------------------------------
//  Rebuild the light system for the requested runway
//-----------------------------------------------------------------------------------------
void CAptObject::RebuildLight(CRunway *rwy)
{ CRLP *lpf = rwy->GetRLP();
  lpf->Flush();
  SetRunwayData(rwy);
  BuildRunwayLight(rwy);
  return;
}
//-----------------------------------------------------------------------------------------
//  Draw a sphere at airport center for test 
//-----------------------------------------------------------------------------------------
void CAptObject::DrawCenter()
{//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  SVector *v = globals->tcm->GetScale();
  glPushMatrix();
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  glFrontFace(GL_CCW);
  glScaled((1/v->x),(1/v->y),1);
  gluSphere(sphere,100,32,32);
  glPopAttrib();
  glPopMatrix();
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  return;
}
//---------------------------------------------------------------------------------
//  For debug purpose Draw current ILS Landing position on Tarmac
//  NOTE: Current transformation must be at airport origin
//---------------------------------------------------------------------------------
void CAptObject::DrawILS()
{ float col[4] = {1,0,0,0.2f};
  CILS *ils = globals->cILS;
  if (0 == ils)             return;
  if (!ils->SameAPT(Airp))  return;
  //--- Draw a  sphere at Landing point ---------
  SVector v1 = {0,0,0};
  SPosition *land = ils->GetLandingPoint();
  Offset(*land,v1);
  SVector v2 = {0,0,0};
  SPosition *farp = ils->GetFarPoint();
  Offset(*farp,v2);
  //SVector *s = globals->tcm->GetScale();
  glPushMatrix();
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glShadeModel(GL_FLAT);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  glDisable(GL_CULL_FACE);
  glScaled(scale->x,scale->y, 1.0);                 // T1 Scale X,Y to feet coordinate
  glTranslated(ofap.x,ofap.y,ofap.z);               // Camera at Airport origin

  //----------------------------------------------
  glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,col);
  glColor4fv(col);
  glBegin(GL_TRIANGLES);
  glVertex3f(v1.x,v1.y,v1.z);
  glVertex3f(v2.x,v2.y,v2.z);
  glVertex3f(v2.x,v2.y,v1.z);
  glEnd();
  //----------------------------------------------
  glPopAttrib();
  glPopMatrix();
  return;
}
//-----------------------------------------------------------------------
//  Draw texture Ground
//  Camera is set at aircraft position (trans vector)
//  NOTE: 1)	No night texture is drawn for airport ground
//				2) Client vertex array must be enabled before calling
//-----------------------------------------------------------------------
void CAptObject::DrawTextureGround()
{ std::vector<CGroundTile*>::iterator it;
  glPushMatrix();                                 // Mark T0;
	SVector *trs = tcm->GetTerrainTrans();
  glTranslated(trs->x, trs->y, trs->z);				// Camera to aircraft
	float *fog = tcm->GetFogColor();
  glColor4fv(fog);                                //
  glMaterialfv (GL_FRONT, GL_EMISSION, tcm->GetDeftEmission()); 
  for (it = grnd.begin(); it != grnd.end(); it++)
    { CGroundTile *gnd = (*it);
      //----------------------------------------------------------
      glPushMatrix();
      gnd->Draw();
      glPopMatrix();
    }
  glPopMatrix();                                // Back to T0
/*
  {GLenum e = glGetError ();
   if (e != GL_NO_ERROR) 
    WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  }
*/
  return;
}
//=========================================================================================
//  CAirportMgr constructor
//  Airport model collects all airports in a given radius and
//  call each airport to build the components for drawing
//=========================================================================================
CAirportMgr::CAirportMgr(TCacheMGR *tm)
{ clock   = 0;
  dbm     = globals->dbc;
  tcm     = tm;
  Dist    = 15.0;                     // Collect airport from 15 nmiles around
//  Limit   = Dist * Dist;              // Squared limit
	Limit   = Dist;
  //--- Taxi texture scale -------------------------
  PavSize = 50;
  PavArc  = 1 / FN_ARCS_FROM_FEET(PavSize);
	//--- Allocate VBO for band and letters ----------
	int tot	= sizeof(vboBUF);
	glGenBuffers(1,&bVBO);
	glBindBuffer(GL_ARRAY_BUFFER,bVBO);
	glBufferData(GL_ARRAY_BUFFER,tot,vboBUF,GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	//--- Load yellow texture -------------------------
	TEXT_INFO txf;
	CArtParser img(TC_HIGHTR);
  strcpy(txf.path,"ART/YLWMARK.BMP");
  img.GetAnyTexture(txf);
	xOBJ	= globals->txw->GetTexOBJ(txf,0,GL_RGBA);
  //--- Aicraft icon -------------------------------
  avion   = new CPicQUAD();
  avion->GetBitmap("ART/PICON.BMP");
  //----Register in globals ------------------------
  globals->apm = this;
	//--- Current location ---------------------------
	nApt				= 0;
	endp				= 0;
	rdep				= 0;
  //-----For test. ------------------------------------------------
  int op = 0;
  GetIniVar("TRACE","DrawILS",&op);
  dILS   = op;
}
//=========================================================================================
//  Destroy airports
//=========================================================================================
CAirportMgr::~CAirportMgr()
{   globals->apm = 0;
    if (avion)  delete avion;
		if (bVBO)		glDeleteBuffers(1,&bVBO);
		if (xOBJ)		glDeleteTextures(1,&xOBJ);
}
//=========================================================================================
//  Find all airports in the requested radius
//  NOTE: Due to the way the database cache is working, runways for airport are loaded
//        some frame latter after the airport is loaded.  Thus the update process
//        must scan for runway presence after airport are introduced in the aptQ
//=========================================================================================
void CAirportMgr::TimeSlice(float dT)
{	if (globals->noAPT)  return;
  CAptObject *apo = 0;
  CAptObject *prv = 0;
  CAirport   *apt = 0;
  char       *fn  = 0;
  //---Update the timers ---------------------------------------
  clock = ++clock & 0x03;
  if (clock)								return;
	if (tcm->MeshBusy())			return;
  //----scan airport queue for Airport leaving the radius ------
  for ( apo = aptQ.GetFirst(); apo != 0; apo = aptQ.GetNext(apo))
      { apt = apo->GetAirport();
				float dst = GetRealFlatDistance(apt);
				apo->SetMiles(dst);
        apo->TimeSlice(dT);
				SaveNearest(apo);																	// Save nearest airport
        if (dst <= Limit)   continue;
        //---------Remove entry --------------------------------
				if (apo == nApt)	nApt = 0;				// No more nearest
        apt->SetAPO(0);										// Remove pointer
				endp		= 0;
        prv = aptQ.Detach(apo);
        delete apo;
        apo = (prv)?(prv):(aptQ.GetFirst());
        break;
      }
  //----Scan database for new entry ----------------------------
  apt = 0;
  for (apt = dbm->FindAPTbyDistance(apt,Limit);apt != 0; apt = dbm->FindAPTbyDistance(apt,Limit))
    { if (apt->IsSelected())                continue;   // Already in Queue
      fn  = apt->GetIdentity();
      apo = new CAptObject(this,apt);                   // Create airport object
      apo->SetCamera(cam);                              // Current camera
      aptQ.PutEnd(apo);                                 // Enter new Airport in Queue
    }
  //----Update runway for current airports in queue -------------
  for   (apo = aptQ.GetFirst(); apo != 0; apo = aptQ.GetNext(apo))
  { if  (apo->HasRunway())      continue;
    apo->SetRunway();
    break;
  }
  return;
}
//----------------------------------------------------------------------------------
//	Save nearest airport
//----------------------------------------------------------------------------------
void CAirportMgr::SaveNearest(CAptObject *apo)
{	float dis = apo->GetNmiles();
	if (0 == nApt)	{nApt = apo;	return;}
	if (nApt->GetNmiles() < dis)	return;				// Still to far 
	nApt	= apo;																// New candidate
	return;
}
//----------------------------------------------------------------------------------
//	Check if we are at airport defined by key
//----------------------------------------------------------------------------------
bool CAirportMgr::AreWeAt(char *key)
{	if (0 == nApt)											return false;
	CAirport *apt = nApt->GetAirport();
	if (strcmp(apt->GetKey(),key) == 0)	return true;
	return false;
}
//----------------------------------------------------------------------------------
//	Position aircraft at the runway threshold
//----------------------------------------------------------------------------------
bool CAirportMgr::SetOnRunway(CAirport *apt,char *idn)
{	endp					= 0;
	CAirport *dep = (nApt)?(nApt->GetAirport()):(0);
	if (apt)	dep = apt;
	if (0 == dep)		return false;
	//------------------------------------------------
	float rot =     dep->GetTakeOffSpot(idn,&tko,&rdep);
  if (0 == tko)		return false;
	CAirplane *pln = globals->pln;
  if (0 == pln)		return false;
	globals->tcm->Teleport(*tko);
	CVector ori   = pln->GetOrientation();
	ori.z					= DegToRad(rot);
	ori.x					= 0;
	ori.y					= 0;
	pln->SetOrientation(ori);
	pln->SetPhysicalOrientation(ori);
	return true;
}
//----------------------------------------------------------------------------------
//	Return the nearest direction 
//----------------------------------------------------------------------------------
bool CAirportMgr::GetTakeOffDirection(SPosition **opp,SPosition *p)
{	CAirport *dep = (nApt)?(nApt->GetAirport()):(0);
	if (0 == dep)			return false;
	ILS_DATA *ils = dep->GetNearestRwyEnd(p,opp);
	if (0 == ils)			return false;
	return true;
}
//----------------------------------------------------------------------------------
//	Return the nearest airport Key
//----------------------------------------------------------------------------------
char *CAirportMgr::NearestIdent()
{	if (0 == nApt)	return "----";
	return nApt->GetAirport()->GetIdent(); }
//----------------------------------------------------------------------------------
//	Bind buffer
//----------------------------------------------------------------------------------
void CAirportMgr::bindLETTERs()
{	glBindBuffer(GL_ARRAY_BUFFER,bVBO);
	glVertexPointer  (3,GL_FLOAT,sizeof(TC_WORLD),0);
	return;
}
//----------------------------------------------------------------------------------
//  Draw Airports
//  pos is aircraft position
//	Camera is at T1:  Scaled for arsec conversion to ffet
//----------------------------------------------------------------------------------
void CAirportMgr::Draw(SPosition pos)
{	//--- Prepare client state ----------------------------------------------
  glPushClientAttrib (GL_CLIENT_ALL_ATTRIB_BITS);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	//--- Draw airports ----------------------------------------------------
	CAptObject *apo = 0;
  for (apo = aptQ.GetFirst(); apo != 0; apo = aptQ.GetNext(apo))
  { apo->SetAircraft(pos);
    apo->Draw();
  }
	//--- Restore state ----------------------------------------------------
  glPopClientAttrib();
  return;
}

//----------------------------------------------------------------------------------
//  Draw Airports Lights
//----------------------------------------------------------------------------------
void CAirportMgr::DrawLights()
{ if (globals->noAPT)       return;
  CAptObject *apo = 0;
  glEnable(GL_TEXTURE_2D);
  for (apo = aptQ.GetFirst(); apo != 0; apo = aptQ.GetNext(apo))
  { if (dILS) apo->DrawILS();
    if (apo->NotVisible())  continue;
    apo->DrawLights(cam);
  }
  glDisable(GL_TEXTURE_2D);
  return;
}
//==========================================================================
//  Ground Detail tile 
//==========================================================================
CGroundTile::CGroundTile(U_INT x,U_INT z)
{ ax  = x;
  az  = z;
  txn = 0;
}
//-------------------------------------------------------------
//	Free resources
//-------------------------------------------------------------
void CGroundTile::Free()
{	quad->ClearGround();
	qgt->DecUser();
	return;
}
//-------------------------------------------------------------
//	Set Ground parameters
//-------------------------------------------------------------
int CGroundTile::StoreData(CTextureDef *t)
{	txn		= t;
	quad	= t->GetQUAD();
	if (0 == quad)	gtfo("Timing error in Airport MGR");
	return quad->GetNbrVTX();
}
//-------------------------------------------------------------
//	Return number of vertices
//-------------------------------------------------------------
int CGroundTile::GetNbrVTX()
{ return quad->NbrVerticesInTile(); }

//-------------------------------------------------------------------------
//	Transpose vertices for each subquad of the detail tile
//-------------------------------------------------------------------------
int CGroundTile::TransposeTile(TC_GTAB *vbo,int dep,SPosition *ori)
{	CmQUAD  *qd		= quad->GetArray();
  TC_GTAB *dst	= vbo;
	int			 tot  = 0;
	int      inx  = dep;
//TRACE("TRANSPOSE TILE-------------------");
  dim  = quad->GetSize();
  for (int k = 0; k != dim; k++,qd++)
	{	sIND[k]  = inx;
		int nbv  = qd->InitVertexCoord(dst,TexRES[TC_HIGHTR]);
		qd->RelocateVertices(dst,ori);
		nIND[k]  = nbv;
		inx += nbv;
		tot += nbv;
		dst += nbv;
	}
	quad->MarkAsGround();
	return tot;
}

//-------------------------------------------------------------
//	Draw ground tiles (draw only elevations)
//	NOTE: Use quad with caution as teleport may have already
//				deleted the detail tile
//-------------------------------------------------------------
void CGroundTile::DrawGround(U_INT x)
{ glBindTexture(GL_TEXTURE_2D,txn->dOBJ);
	//--- Draw only elevations ---------------
	glMultiDrawArrays(GL_TRIANGLE_FAN,sIND,nIND,dim);
  //---- Draw contour if Terra Browser is active -----------
	//  NOTE : this is done in DrawTile();
  //if (0 == (globals->aPROF & PROF_DR_DET))	return;
  //if (!globals->tcm->PlaneQuad(quad))				return;
  //quad->Contour();
	return;
}
//-------------------------------------------------------------
//	Draw with band translation
//-------------------------------------------------------------
void CGroundTile::Draw()
{	U_INT qx = (ax >> 5);										// QGT X indice
	char hba = globals->tcm->GetHband();
	char hbq = FN_BAND_FROM_QGT(qx) << TC_BY08;
	glTranslated(GetXTRANS(hba,hbq),0,0);
	glBindTexture(GL_TEXTURE_2D,txn->dOBJ);
	CSuperTile *sp = quad->GetSuperTile();
	sp->BindVBO();
	quad->DrawTile();
	return;
}
//============================END OF FILE =================================================
