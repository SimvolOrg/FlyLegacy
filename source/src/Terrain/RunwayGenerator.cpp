//=====================================================================================================
//	RUNWAY MANAGEMENT
//=====================================================================================================
/*
 * RunwayGenerator.cpp
 *
 * Part of Fly! Legacy project
 *
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
#ifndef RUNWAYGENERATOR_H
#define RUNWAYGENERATOR_H

#include "../Include/FlyLegacy.h"
#include "../Include/Globals.h"
#include "../Include/Terraintexture.h"
#include "../Include/RunwayGenerator.h"
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
// Mid point table for cuting runway into segments
//=============================================================================
TC_RMP_DEF RwyMID[128];
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
//  RUNWAY MODEL
//  This table guides the generation process of runway segment
//  It describes a complete PAVED RUNWAY
//  NOTE: If the number of item is changed, 
//      TC_MIDSEGTABLE   must be set to the MID segment index
//      TC_DISSEGTABLE   must be set to the LDP segment index
//=============================================================================
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
//=========================================================================================
//  Vector table to center light model
//=========================================================================================
CRwyGenerator::ctlFN      CRwyGenerator::ctlVECTOR[] = {
  &CRwyGenerator::CenterLightModel0,                   // 'none'
  &CRwyGenerator::CenterLightModel1,                   // 'WWWW'                   
  &CRwyGenerator::CenterLightModel2,                   // 'RWWW'
  &CRwyGenerator::CenterLightModel3,                   // 'WWRW'
  &CRwyGenerator::CenterLightModel4,                   // 'RWRW'
};
//=========================================================================================
//  Vector table to edge light model
//=========================================================================================
CRwyGenerator::ctlFN      CRwyGenerator::edgVECTOR[] = {
  &CRwyGenerator::EdgeLightModel0,
  &CRwyGenerator::EdgeLightModel1,
  &CRwyGenerator::EdgeLightModel2,
  &CRwyGenerator::EdgeLightModel3,
  &CRwyGenerator::EdgeLightModel4,
};

// ============================================================================
//	LIGHT MODELS PROTOTYPE
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
//=========================================================================================
//  END OF TABLES
//=========================================================================================

//==================================================================================================================
//	Runway GENERATOR
//==================================================================================================================
CRwyGenerator::CRwyGenerator(CRunway *rw,CAptObject *ap, char act)
{	apo		= ap;
	rwy		= rw;
	pave	= act;
	//--- Init airport parameters ----------------------------------
	Org		= apo->GetOrigin();				// Airport geoposition
	xpf   = apo->GetXPF();					// Expension factor
	rdf   = apo->GetRDF();					// Reduction factor
	//-----Compute scale factor for designator ---------------------
	apo-> GetSLT(scl);
	//--------------------------------------------------------------
	BuildRunway(act);
}
//---------------------------------------------------------------------------------
//  Build the runway
//---------------------------------------------------------------------------------
void CRwyGenerator::BuildRunway(char action)
{	if (1 == action)		return BuildPavedRunway();
	if (2 == action)		return BuildOtherRunway();
	if (3 == action)		return RebuildLights();
}
//---------------------------------------------------------------------------------
//  Build a paved runway
//---------------------------------------------------------------------------------
void CRwyGenerator::BuildPavedRunway()
{ //TRACE("  PAVED RUNWAY %s-%s for %s (key=%s)",
	//							 rwy->GetHiEnd(),rwy->GetLoEnd(),Airp->GetName(),Airp->GetKey());
	rwy->SetNumberBand(12);
  SetRunwayData();
  if (tlgr <= 0) return;
  BuildRunwayMidPoints(PavedRWY_MODEL);
	BuildTarmacSegs();
	BuildRunwayLight();
  return;
}
//---------------------------------------------------------------------------------
//  Build other runways
//  Non paved runways are drawn as one piece
//
//---------------------------------------------------------------------------------
void CRwyGenerator::BuildOtherRunway()
{ //--Init model parameters -----------------------------------------
	//if (tr)  TRACE("  OTHER RUNWAY %s-%s for %s",
	//							rwy->GetHiEnd(),rwy->GetLoEnd(),Airp->GetName());
  SetRunwayData();
  if (tlgr <= 0) return;
  BuildRunwayMidPoints(OtherRWY_MODEL);
	BuildTarmacSegs();
	BuildRunwayLight();
  return;
}
//---------------------------------------------------------------------------------
//  Rebuild the light systems for this runway
//---------------------------------------------------------------------------------
void CRwyGenerator::RebuildLights()
{ CRLP *lpf = rwy->GetRLP();
  lpf->Flush();
  SetRunwayData();
  BuildRunwayLight();
  return;
}

//--------------------------------------------------------------------------------
//  Compute elevation from location
//  Adjust longitude and latitude to absolute value
//--------------------------------------------------------------------------------
void CRwyGenerator::ComputeElevation(SPosition &pos)
{ pos.lon += Org.lon;
  pos.lat += Org.lat;
  GroundSpot lnd(pos.lon,pos.lat);
  pos.alt  = globals->tcm->SetGroundAt(lnd);
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
void CRwyGenerator::SetRunwayData()
{ SPosition deb  = rwy->GetHiPos();
  SPosition end  = rwy->GetLoPos();
  //---Position relative to airport origin -------------
  apo->Offset(deb,p0);               // Offset of runway deb
  apo->Offset(end,p1);               // Offset of runway end
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
void CRwyGenerator::SetLandingPRM(ILS_DATA *ils,float ln,float tk)
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
void CRwyGenerator::BuildRunwayMidPoints(TC_RSEG_DESC *model)
{ //---Eliminate runway if no parameters ------------------
  eInd  = 0;
  if ((dx == 0) && (dy == 0)) return;
  //---Compute the rotation parameters --------------------
  rot = atan2(-dx * rdf,dy);
  rot = RadToDeg(rot);
  rwy->SetROT(rot,TC_HI);
  rwy->SetROT((rot + 180),TC_LO);
//  apo->TraceRWY(rwy);
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
//  Cut the mid segment into parts of 400 feet length
//---------------------------------------------------------------------------------
void CRwyGenerator::BreakMidSegment(TC_RSEG_DESC *model,int lg)
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
//  Generate a new segment entry in RwyMID table
//---------------------------------------------------------------------------------
void CRwyGenerator::GenerateRwyPoint(TC_RSEG_DESC *model)
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
void CRwyGenerator::BuildTarmacSegs()
{ int dim = eInd;
	CTarmac *tmac = new CTarmac(apo,rwy);									// Create Tarmac
	U_CHAR    gr = rwy->GroundIndex();										// Runway ground type
  globals->txw->GetRwyTexture(tmac,gr);									// Set runway texture
  SegmentBase(0);                                       // Compute base points
  apo->AptExtension(psw);                               // Set extension
  apo->AptExtension(pse);                               // Set Extension
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
          SetHiThreshold(k);
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
          SetLoThreshold(u);
          break;

			}
		}
	//-----Check extension for last points ---------------------
  apo->AptExtension(psw);
  apo->AptExtension(pse);
	//--- Load the VBO -----------------------------------------
	tmac->LoadVBO();
	apo->AddTarmac(tmac);
  return;
}
//-------------------------------------------------------------------------------
//  Compute base segment at mid point k
//-------------------------------------------------------------------------------
void CRwyGenerator::SegmentBase(int k)
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
void CRwyGenerator::SetTxCoord(TC_VTAB *tab,int No, char grnd)
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
//---------------------------------------------------------------------------------
//  Set left and right positions for threshold bands
//  Each band is 12 feet spaced apart and start 10 feet from the runway border
//---------------------------------------------------------------------------------
void  CRwyGenerator::SetHiThreshold(int k)
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
  pos.x = RwyMID[k].px +  (ppx * dta);			// GetXnormal(dta);
  pos.y = RwyMID[k].py -  (ppy * dta);			// GetYnormal(dta);
  rwy->SetRTH(pos,TC_HI);
  return;
}
//---------------------------------------------------------------------------------
//  Set left and right positions for threshold bands
//  Each band is 12 feet spaced apart and start 10 feet from the runway border
//---------------------------------------------------------------------------------
void  CRwyGenerator::SetLoThreshold(int k)
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
  pos.x = RwyMID[k].px -  (ppx * dta);		// GetXnormal(dta);
  pos.y = RwyMID[k].py +  (ppy * dta);		//GetYnormal(dta);
  rwy->SetRTH(pos,TC_LO);
  return;
}
//=================================================================================
//  set Runway lights according to light profile
//=================================================================================
void CRwyGenerator::BuildRunwayLight()
{ int mod = 0;
  CRLP     *lpf = rwy->GetRLP();
  //---Build center light model -------------------
  mod = lpf->GetCenterLM();
  (this->*ctlVECTOR[mod])();
  //---Build edge light model ---------------------
  mod = lpf->GetEdgeLM();
  (this->*edgVECTOR[mod])();
  //---Other lights -------------------------------
  TouchDWLights();
  //---Build threshold bars -------------------------
  ThresholdBarLights();
  WingBarLights();
  ApproachLight();
  return;
}
//---------------------------------------------------------------------------------
//  Light Model 0: No center lights
//---------------------------------------------------------------------------------
int CRwyGenerator::CenterLightModel0()
{ return 0;
}
//---------------------------------------------------------------------------------
//  Ligth Model 1 (hi is CATx)
//  Omni lights up to 2700 feet from end
//  1800 feet of alternate RED WHITE
//   900 of red lights
//  Runway must be at least 2700 feet for this model to apply
//---------------------------------------------------------------------------------
int CRwyGenerator::CenterLightModel1()
{ if (tlgr < 2700) return CenterLightModel4();
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
int CRwyGenerator::CenterLightModel2()
{ int rwl = rwy->GetLenghi() - hiDSP - loDSP;
  if (rwl < 2700) return CenterLightModel4();
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
int  CRwyGenerator::CenterLightModel3()
{ if (tlgr < 5400) return CenterLightModel4();
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
int  CRwyGenerator::CenterLightModel4()
{ CLitSYS &lsys = rwy->GetRLP()->GetCenterSection();
  total         = hiDSP;
  lmOMNID.nbo   = tlgr  / lmOMNID.pace;
  BuildOmniLBAR (lmOMNID,lsys);
  return 0;
}
//---------------------------------------------------------------------------------
//  Light Model 0: No edge lights
//---------------------------------------------------------------------------------
int  CRwyGenerator::EdgeLightModel0()
{   return 0;
}
//---------------------------------------------------------------------------------
//  Light Model 1: Hi end = CATx
//  white light up to 1800 feet (600 m) from the end
//  then all yellow lights
//---------------------------------------------------------------------------------
int CRwyGenerator::EdgeLightModel1()
{ if (tlgr < 1800) return EdgeLightModel4();
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
int  CRwyGenerator::EdgeLightModel2()
{ if (tlgr < 1800) return EdgeLightModel4();
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
int  CRwyGenerator::EdgeLightModel3()
{ if (tlgr < 1800) return EdgeLightModel4();
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
int  CRwyGenerator::EdgeLightModel4()
{ CLitSYS &lsys = rwy->GetRLP()->GetEdgeSection();
  total = hiDSP;
  lmEDGE1.nbo = tlgr  / lmEDGE1.pace;
  BuildEdgeOmni(lmEDGE1,lsys);                // Rest of runway
  return 0;
}

//---------------------------------------------------------------------------------
//  Touch Down lights parameters
//  Build a bar every 100 feet after threshold
//  NOTE: Left extremity if from 1/4 of runway wide + 10 feets
//---------------------------------------------------------------------------------
int CRwyGenerator::TouchDWLights()
{ if (rwy->GetWiCode()  < 2)      return 0;
  CRLP    *lpf = rwy->GetRLP();
  lmTDZNE.nOfs = hiDSP + 100;                // Start at displacement
  lmTDZNE.wOfs = (rhwd >> 1) + (10);         // Left extremity
  int range   = (tlgr >> 1) - lmTDZNE.nOfs;  // Range available
  if (range > 900)  range = 900;             // Limit to 900 feet
  int row = range / 100;                     // Number of rows
  total   = 0;
  //---Build 900 feets or half lenght ----------------
  if (lpf->GetHiTDZN()) BuildTDZbar(lmTDZNE,row);   // Hi End
  total -= 100;                             // Last row
  lmTDZNE.nOfs = rwy->GetLenghi() - loDSP - total;
  if (lpf->GetLoTDZN()) BuildTDZbar(lmTDZNE,row);   // Lo End
  return 1;
}
//---------------------------------------------------------------------------------
//  Hi Threshold bar model
//---------------------------------------------------------------------------------
int CRwyGenerator::ThresholdBarLights()
{ CRLP *lpf     = rwy->GetRLP();
  CLitSYS &lsys = lpf->GetBarSection();
  //---Compute number of light according to segment width -------
  U_CHAR hmd    = lpf->GetHiTBAR();
  lmTBARS.nbo   = MaxTBarLights();
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
int CRwyGenerator::WingBarLights()
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
//  Build Approach Lights
//---------------------------------------------------------------------------------
int CRwyGenerator::ApproachLight()
{ CRLP *lpf = rwy->GetRLP();
  int papi  = 0;
  switch (lpf->GetHiAPRL()) {
  case TC_APR_REIL:
		REILSystem(0);
    break;
  case TC_APR_ODAL:
    HiODALSystem();
    break;
  case TC_APR_ALSR:
    HiSALRSystem();
    break;
  case TC_APR_ALF1:
    HiALSF1System();
    break;
  case TC_APR_ALF2:
    HiALSF2System();
    break;
  }
  //---Check for lower end -----------------
  switch(lpf->GetLoAPRL()) {
  case TC_APR_REIL:
		REILSystem(rwy->GetLenghi());
    break;
  case TC_APR_ODAL:
    LoODALSystem();
    break;
  case TC_APR_ALSR:
    LoSALRSystem();
    break;
  case TC_APR_ALF1:
    LoALSF1System();
    break;
  case TC_APR_ALF2:
    LoALSF2System();
    break;
  }
  //---Check for PAPI light ---------------
  papi = lpf->GetHiPAPI();
  if (papi == TC_PAPI_4L) HiPAPI4System();
  papi = lpf->GetLoPAPI();
  if (papi == TC_PAPI_4L) LoPAPI4System();
  return 0;
}
//---------------------------------------------------------------------------------
//  Build edge lights
//---------------------------------------------------------------------------------
void CRwyGenerator::BuildEdgeOmni(LITE_MODEL &seg,CLitSYS &ls)
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
//  Build longitudinal bar of omni lights
//  off= starting offset
//  tgl= Length to build
//  total: total feet covered from x0,y0
//---------------------------------------------------------------------------------
void CRwyGenerator::BuildOmniLBAR(LITE_MODEL &seg,CLitSYS &ls)
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
//  Build intro01 part center lights
//  off= starting offset
//  tgl= Length to build
//  total: total feet covered from x0,y0
//---------------------------------------------------------------------------------
void CRwyGenerator::BuildCenterDual(LITE_MODEL &seg,CLitSYS &ls)
{ int nbo         = seg.nbo;
  //---Spot generation --------------------------------
  for (int k = 0; k < nbo; k++) BuildDualLLIN(seg,ls);
  return;
}
//---------------------------------------------------------------------------------
//  Build 900 feet of TouchDown lights up to half runway
//---------------------------------------------------------------------------------
void CRwyGenerator::BuildTDZbar(LITE_MODEL &seg,int row)
{ CLitSYS &lsys = rwy->GetRLP()->GetTouchSection();
  for (int k = 0; k != row; k++)
  { BuildDualTBAR(seg,lsys);
    seg.nOfs += 100;            // Next double bars at 100 feet further
    total    += 100;
  }
  return;
}
//---------------------------------------------------------------------------------
//  Build Threshold/end bar lights
//  ofs is set to the runway end position
//    hi end: ofs = hiDSP (hi displacement)
//    lo end: ofs = Lenth(rwy) - loDSP
//---------------------------------------------------------------------------------
void CRwyGenerator::BuildDualTBAR(LITE_MODEL &seg,CLitSYS &ls)
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
//  Build a dual flash on both sides of runway
//---------------------------------------------------------------------------------
void CRwyGenerator::BuildFlashTBAR(LITE_MODEL &seg,CLitSYS &ls)
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
//  Build Longitudinal Flash BAR at runway end
//---------------------------------------------------------------------------------
void CRwyGenerator::BuildFlashRAIL(LITE_MODEL &seg,CLitSYS &ls)
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
//  Build a PAPI light
//---------------------------------------------------------------------------------
void CRwyGenerator::BuildPAPIBAR(LITE_MODEL &seg,CLitSYS &ls)
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
int CRwyGenerator::MaxTBarLights()
{ int hw = (rwy->GetWidthi() - lmTBARS.pace) >> 1;  // half width
  int nb = (hw / lmTBARS.pace) + 1;                 // Maximum at given feet interval
  //---Reduce number for non precision runway  ----------------
  if (rwy->NoPrecision()) nb = LiteNPR[rwy->GetWiCode()];
  return nb;
}
//---------------------------------------------------------------------------------
//  REIL system
//	Hi end => dis = 0
//	Lo end => dis = runway length
//---------------------------------------------------------------------------------
int CRwyGenerator::REILSystem(int dis)
{ CLitSYS &lsys = rwy->GetRLP()->GetApproachSection();
  lmREILa.nOfs = 0;
  lmREILa.wOfs = rhwd + 40;
  BuildFlashTBAR(lmREILa,lsys);
  return 1;
}
//---------------------------------------------------------------------------------
//  Hi ODAL system
//---------------------------------------------------------------------------------
int CRwyGenerator::HiODALSystem()
{ CLitSYS &lsys = rwy->GetRLP()->GetApproachSection();
  BuildFlashRAIL(lmODALS,lsys);
	REILSystem(0);
  return 0;
}
//---------------------------------------------------------------------------------
//  Lo ODAL system
//---------------------------------------------------------------------------------
int CRwyGenerator::LoODALSystem()
{ CLitSYS &lsys = rwy->GetRLP()->GetApproachSection();
  lmODALN.nOfs = 270 + rwy->GetLenghi();
  BuildFlashRAIL(lmODALN,lsys);
	REILSystem(rwy->GetLenghi());
  return 0;
}
//---------------------------------------------------------------------------------
//  Hi SSALR system
//  7  white barettes up to 1400 feet
//  5  flashers       up to 2400 feet
//---------------------------------------------------------------------------------
int CRwyGenerator::HiSALRSystem()
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
int CRwyGenerator::LoSALRSystem()
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
int CRwyGenerator::HiALSF1System()
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
int CRwyGenerator::LoALSF1System()
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
int CRwyGenerator::HiALSF2System()
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
int CRwyGenerator::LoALSF2System()
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
int CRwyGenerator::HiPAPI4System()
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
int CRwyGenerator::LoPAPI4System()
{ CRLP *lpf     = rwy->GetRLP();
  lmPAPI4.nOfs  = rlgt - loDSP - rwyMARGE[rwy->GetLgCode()];
  lmPAPI4.wOfs  = - (rhwd + 45);
  lmPAPI4.lnd   = rwy->GetLandPos(RWY_LO_END);
  BuildPAPIBAR(lmPAPI4,lpf->GetPapiSystem());
  return 1;
}
//---------------------------------------------------------------------------------
//  Build Longitudinal ramp with 2 Dual Lights L1 and L2
//  Use the normal vector to L1-L2 axis as a decision line
//               |   
//        ---L1----L2----Runway center line
//               |
//              decision line
//---------------------------------------------------------------------------------
void CRwyGenerator::BuildDualLLIN(LITE_MODEL &seg,CLitSYS &ls)
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
//  Fill decision parameters
//  
//---------------------------------------------------------------------------------
void  CRwyGenerator::StoreDecision(CBaseLITE *lite)
{ lite->side.po = mid;                  // Set mid point as decision point
  lite->side.pa.x = lpt.x - egx;     //ppx;
  lite->side.pa.y = lpt.y + egy;     //ppy;
  lite->side.pb.x = lpt.x + egx;     //ppx;
  lite->side.pb.y = lpt.y - egy;     //ppy;
  //--Init pre computation ------------------------
  lite->PreCalculate();
  return;
}

//============================ END OF FILE =========================================================================
#endif  RUNWAYGENERATOR_H
