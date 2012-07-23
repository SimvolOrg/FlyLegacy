//=====================================================================================================
//	RUNWAY MANAGEMENT
//=====================================================================================================
/*
 * RunwayGenerator.h
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
//======================================================================================================
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//-------------------------------------------------------------------------------------------------------
#include "../Include/FlyLegacy.h"
#include "../Include/Airport.h"
//=========================================================================================
//	Class RUNWAY GENERATOR
//	This class build runway drawing components
//=========================================================================================
class CRwyGenerator
{	//--- ATTRIBUTES ---------------------------------------------------
	CAptObject	*apo;													// Airport object
	CRunway		  *rwy;												  // Runway to generate
	char         pave;												// Paved or not
  //------Airport parameters  ----------------------------------------
  SPosition Org;                                // Airport origin
	double  xpf;																	// Expension factor
  double  rdf;																	// Reduction factor
	//----Runway parameters --------------------------------------------
  double width;                                 // Runway half wide (feet)
  double rlgt;                                  // Runway length (feet)
  SVector p0;                                   // Runway Hi position
  SVector p1;                                   // Runway Lo position
  //--------------------------------------------------------------------
  P2_POINT  mid;                                // Runway mid point
  P2_POINT  lpt;                                // Light point
  //---Metric parameters -----------------------------------------------
  double  dx;                                   // dx in arcsec
  double  dy;                                   // dy in arcsec
  double  dz;                                   // dz in arcsec
  double altF;                                  // Altitude factor
  double arcX;                                  // Arcs / feet along X
  double arcY;                                  // Arcs / feet along Y
  short  rhwd;                                  // Runway half width
  //---Runway segment generator ----------------------------------------
  float   wTex;                                 // SubTexture width
  int     xsr;                                  // source index
  int     xds;                                  // destination index
  int     total;                                // Total generated
  int     loDSP;                                // Low displacement
  int     hiDSP;                                // hig Displacement
  int     tlgr;                                 // true runway lenght
  //----Generator parameters -------------------------------------------
  U_CHAR    eInd;                               // Ending index (number of textured seg)
  U_CHAR    rInd;                               // Restart index
  double    ppx;                                // X norme generator
  double    ppy;                                // Y norme generator
  double    egx;                                // X edge generator
  double    egy;                                // Y edge generator
  double    rot;                                // Angle of rotation for runway
	//--- Landing date work area ---------------------------------------
	double		d1;
	double		d2;
	double		d3;
	double		d4;
	//------------------------------------------------------------------
  SVector   scl;                                // Scale factor for letter
  SVector   sct;                                // Scale factor for threshold bands
  //---Segment base points ---------------------------------------------
  TC_VTAB  vsw;                                 // SW vertex (current)
  TC_VTAB  vse;                                 // SE vertex (current)
  TC_VTAB  bsw;                                 // NW vertex (base)
  TC_VTAB  bse;                                 // NE vertex (base)
  GroundSpot psw;                               // SW corner
  GroundSpot pse;                               // SE corner
  //--------------Vector table -------------------------------------------
  typedef int (CRwyGenerator::*ctlFN)();
  static ctlFN      ctlVECTOR[];              // Center light process
  static ctlFN      edgVECTOR[];              // Center light process
	//--- METHODS ---------------------------------------------------
public:
	CRwyGenerator(CRunway *rwy,CAptObject *ap, char type);
	//---------------------------------------------------------------
	void		BuildRunway(char type);
	void		BuildPavedRunway();
	void		BuildOtherRunway();
	void		RebuildLights();
	//---------------------------------------------------------------
	void		ComputeElevation(SPosition &pos);
	void		SetRunwayData();
	void		SetLandingPRM(LND_DATA *ils,float ln,float tk);
	//---------------------------------------------------------------
	void		BuildRunwayMidPoints(TC_RSEG_DESC *model);
	void		BreakMidSegment(TC_RSEG_DESC *model,int lg);
	void		GenerateRwyPoint(TC_RSEG_DESC *model);
	void		BuildTarmacSegs();
	void		SegmentBase(int k);
	//--- Texture generation ----------------------------------------
	void		SetTxCoord(TC_VTAB *tab,int No, char grnd);
	void		SetHiThreshold(int k);
	void		SetLoThreshold(int k);
	//--- Light system building -------------------------------------
	void		BuildRunwayLight();
	//--- Center light models ---------------------------------------
	int			CenterLightModel0();
	int			CenterLightModel1();
	int			CenterLightModel2();
	int			CenterLightModel3();
	int			CenterLightModel4();
	//--- Edge light models -----------------------------------------
	int			EdgeLightModel0();
	int			EdgeLightModel1();
	int			EdgeLightModel2();
	int			EdgeLightModel3();
	int			EdgeLightModel4();
	//--- Various lights systems ------------------------------------
	int			TouchDWLights();
	int			ThresholdBarLights();
	int			WingBarLights();
	int			ApproachLight();
	//--- Approach systems ------------------------------------------
	int			REILSystem(int dis);
	int			HiODALSystem();
	int			LoODALSystem();
	int			HiSALRSystem();
	int			LoSALRSystem();
	int			HiALSF1System();
	int			LoALSF1System();
	int			HiALSF2System();
	int			LoALSF2System();
	int			HiPAPI4System();
	int			LoPAPI4System();
	//--- Various light components ----------------------------------
	void		BuildEdgeOmni(LITE_MODEL &seg,CLitSYS &ls);
	void		BuildOmniLBAR(LITE_MODEL &seg,CLitSYS &ls);
	void		BuildCenterDual(LITE_MODEL &seg,CLitSYS &ls);
	void		BuildTDZbar   (LITE_MODEL &seg,int row);
	void		BuildDualTBAR (LITE_MODEL &seg,CLitSYS &ls);
	void		BuildDualLLIN (LITE_MODEL &seg,CLitSYS &ls);
	void		BuildFlashTBAR(LITE_MODEL &seg,CLitSYS &ls);
	void		BuildFlashRAIL(LITE_MODEL &seg,CLitSYS &ls);
	void		BuildPAPIBAR  (LITE_MODEL &seg,CLitSYS &ls);
	//--- Helpers ---------------------------------------------------
	int			MaxTBarLights();
	void		StoreDecision(CBaseLITE *lite);
};

//======================================END OF FILE =================================================================