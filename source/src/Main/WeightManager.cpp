/*
 * WeightManager.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright (c) 2004 Chris Wallace
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

/*! \file WeightManager.cpp
 *  \brief Implements CWeightManager 
 *
 */


#include "../Include/WeightManager.h"
#include "../Include/Subsystems.h"
#include "../Include/3Dmath.h"
#include "../Include/Globals.h"
#include <math.h>
#include <string>

//=======================================================================
// Loadout Unit
//=======================================================================
CLoadCell::CLoadCell (CVehicleObject	*veh)
{ mveh	= veh;
	*name = 0;
  bPos.x = 0;   bPos.y = 0;   bPos.z = 0;
  load = 0;
  hiLm = 0;
 *utyp = 0;
}
//----------------------------------------------------------------------
//  Read parameters
//----------------------------------------------------------------------
int CLoadCell::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'name':
    // Name of loadout position
    ReadString (name, 64, stream);
    name[63]  = 0;
    rc = TAG_READ;
    break;

  case 'bPos':
    // Offset from default center of gravity
    ReadVector (&bPos, stream);
    bPos = bPos + mveh->wgh->svh_cofg;
    bPos.InvertXY();         
    rc = TAG_READ;
    break;

  case 'load':
    // Default load value in pounds
    ReadFloat (&load, stream);
    rc = TAG_READ;
    break;

  case 'hiLm':
    // Maximum load value in pounds
    ReadFloat (&hiLm, stream);
    rc = TAG_READ;
    break;

  case 'utyp':
    // UI type
    ReadString (utyp, 64, stream);
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // Tag was not processed by this object, it is unrecognized
    WARNINGLOG ("CLoadCell::Read : Unrecognized tag <%s>", TagToString(tag));
  }

  return rc;
}

//-------------------------------------------------------------------------
//  Compute contribution to CG
//  -mm = sum(FuelMassi * bPosi²)   (inertia)
//  -vn = sum(FuelMassi * bPosi)
//  -sm = sum(FuelMassi)
//-------------------------------------------------------------------------
void CLoadCell::ContributionCG(SVector &mm,SVector &vn, double &sm)
{ double sx = load * bPos.x;
  double sy = load * bPos.y;
  double sz = load * bPos.z;
  vn.x += sx;
  vn.y += sy;
  vn.z += sz;
  //---Inertia contribution  -------
  mm.x += sx * bPos.x;
  mm.y += sy * bPos.y;
  mm.z += sz * bPos.z;
  //---Mass Contribution      ------
  sm   += load;
  return;
}
//-------------------------------------------------------------------------
//  Write to history
//-------------------------------------------------------------------------
void CLoadCell::Write (SStream *stream)
{

}

//======================================================================================

const SVector* CWeightBalance::GetCGOffset_ISU (void) 
{
  CGOffset_ISU.x = FN_METRE_FROM_FEET (CGOffset.x); // 
  CGOffset_ISU.y = FN_METRE_FROM_FEET (CGOffset.y); // 
  CGOffset_ISU.z = FN_METRE_FROM_FEET (CGOffset.z); // 
  return &CGOffset_ISU;
}

const SVector* CWeightBalance::GetMI_ISU (void) 
{
  moments_of_inertia_ISU.x = moments_of_inertia.x * LBFT2_TO_KGM2; // 
  moments_of_inertia_ISU.y = moments_of_inertia.y * LBFT2_TO_KGM2; // 
  moments_of_inertia_ISU.z = moments_of_inertia.z * LBFT2_TO_KGM2; // 
//
  // temporary removed any computation for the function needs to be fixed : see
  // right now we use instead a force at CG pos. in OPAL COPALObject
  moments_of_inertia_ISU.x = 0.0;
  moments_of_inertia_ISU.y = 0.0;
  moments_of_inertia_ISU.z = 0.0;
	
  return &moments_of_inertia_ISU;
}

//==============================================================================
// CWeightManager
//==============================================================================
CWeightManager::CWeightManager (CVehicleObject *v)
{ mveh  = v;        // Save parent vehicle
  CVector vnul      (0,0,0);
  svh_emas          = 0.0f;
  whl_rmas          = 0.0f;
  svh_mine          = vnul;
  svh_cofg          = vnul;
  wDRY              = 0.0;
  wGAS              = 0.0;
  wLOD              = 0.0;
}
//-----------------------------------------------------------------------
//  Destructor
//-----------------------------------------------------------------------
CWeightManager::~CWeightManager (void)
{ std::vector<CLoadCell*>::iterator vl;
  for (vl=vld_unit.begin(); vl!=vld_unit.end(); vl++) delete (*vl);
  vld_unit.clear();
}
//-----------------------------------------------------------------------
//  Real time Update 
//-----------------------------------------------------------------------
void CWeightManager::Timeslice (float dT)
{ vCG.x =  svh_cofg.x;
  vCG.y =  svh_cofg.z;
  vCG.z =  svh_cofg.y;
  ComputeVisualCG(vCG);
}
//-----------------------------------------------------------------------
// Compute initial values
//  NOTE: mDRY is not in the same hand coordinate as svh_mine
//  svh_mine is original from fly (LH)
//  mDRY is RH (as OpenGL is)
//-----------------------------------------------------------------------
void CWeightManager::Init (void)
{
  wDRY    = svh_emas * float(GRAVITY_FTS);
  mDRY    = svh_mine;
  mDRY.InvertXY();
}
//-----------------------------------------------------------------------
//  Compute the Fuel contribution to CG
//  -cg = sum(FuelMassi * bPosi)
//  -sm = sum(FuelMassi)
//-----------------------------------------------------------------------
void  CWeightManager::FuelCGcontribution(SVector &cg,double &sm)
{ wGAS = 0;
  mGAS.Zero();
  std::vector<CFuelCell*>::iterator it;
  for (it = gas_cell.begin(); it != gas_cell.end(); it++)
  { (*it)->ContributionCG(mGAS,cg,wGAS);
  }
  //-- Update sm = SUM(FuelMass(i)) -----
  sm += wGAS;
  return;
}
//-----------------------------------------------------------------------
//  Compute the Load contribution to CG
//-----------------------------------------------------------------------
void  CWeightManager::LoadCGcontribution(SVector &cg,double &sm)
{ wLOD = 0;
  mLOD.Zero();
  std::vector<CLoadCell*>::iterator it;
  for (it = vld_unit.begin(); it != vld_unit.end(); it++)
  { (*it)->ContributionCG(mLOD,cg,wLOD);
  }
  sm += wLOD;
  return;
}
//-----------------------------------------------------------------------
//  Compute CG from visual center (based on bPos coordinates)
//  Visual center is the origin (0,0,0) of aircraft
//-----------------------------------------------------------------------
void CWeightManager::ComputeVisualCG(CVector &cg)
{ double tm = wDRY;      // Sum of masses = dry weight
  cg.Times(tm);
  //-----Compute fuel contribution to CG ------------
  FuelCGcontribution(cg,tm);
  //-----Compute load contribution to CG ------------
  LoadCGcontribution(cg,tm);
  double dn = (tm > DBL_EPSILON)?(1 / tm):(1);
  cg.Times(dn);
  return;
}
//-----------------------------------------------------------------------
//  Compute CG offset from initial CofG and visual CG (vCG)
//  VCG<------------o
//         CofG<----o
//  As both are computed from the visual origin (o) we have
//  offsetCG = VCG - CofG
//-----------------------------------------------------------------------
void CWeightManager::ComputeOffsetCG(CVector &cg)
{ oCG = vCG - svh_cofg;
  cg  = oCG;
  return;
}
//-----------------------------------------------------------------------
//  return a copy of the fuel cell list
//-----------------------------------------------------------------------
void  CWeightManager::GetFuelCell(std::vector<CFuelCell*> &vf)
{ vf = gas_cell;}
//-----------------------------------------------------------------------
//  return a copy of the load cell list
//-----------------------------------------------------------------------
void  CWeightManager::GetLoadCell(std::vector<CLoadCell*> &vl)
{ vl = vld_unit;}
//-----------------------------------------------------------------------
void CWeightManager::Debug (void)
{
	  FILE *fp_debug = fopen("__DDEBUG_CWeightManager.txt", "a");
	  if(fp_debug != NULL)
	  {   std::vector<CLoadCell*>::iterator  vl;
        for (vl = vld_unit.begin(); vl != vld_unit.end(); vl++) {
          std::string s = (*vl)->GetName();
          float test = (*vl)->GetLoad();
		      fprintf(fp_debug, " %s %f\n", s.c_str (), test);
        }
        std::vector<CFuelCell*>::iterator it;
        for (it = gas_cell.begin(); it != gas_cell.end(); it++) {
          std::string s = (*it)->name;
          double test = (*it)->GetXloc();
		      fprintf(fp_debug, " %s %f %f \n", s.c_str (), test, (*it)->GetCellQty());
        }

        fprintf(fp_debug, " emas rmas %f %f\n", svh_emas, whl_rmas);
        //
        fprintf(fp_debug, " VLD = %f %f %f --- GAS = %f %f %f\n",
                                             wbVld.GetCGOffset ()->x,
                                             wbVld.GetCGOffset ()->y,
                                             wbVld.GetCGOffset ()->z,
                                             wbGas.GetCGOffset ()->x,
                                             wbGas.GetCGOffset ()->y,
                                             wbGas.GetCGOffset ()->z);
//        Calc ();
        fprintf(fp_debug, " CG = %f %f %f\n",wb.GetCGOffset ()->x,
                                             wb.GetCGOffset ()->y,
                                             wb.GetCGOffset ()->z     );
        fprintf(fp_debug, " MI  = %f %f %f\n",
                                             wb.GetMI ()->x,
                                             wb.GetMI ()->y,
                                             wb.GetMI ()->z           );
        fprintf(fp_debug, "------------------------------------\n" );
        //
		fclose(fp_debug); 
	  }
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

//------------------------------------------------------------------------
//  Get total mass in lbs
//------------------------------------------------------------------------
float CWeightManager::GetTotalMassInLbs (void)
{ return (wDRY + wGAS + wLOD);
}
//------------------------------------------------------------------------
//  Get total mass in lbs
//------------------------------------------------------------------------
float CWeightManager::GetTotalMassInKgs (void)
{
  float rt = GetTotalMassInLbs () * float(LBS_TO_KGS);
  return (rt);
}


//----------------------------------------------------------------------------
CWeightBalance operator+(const CWeightBalance &aCopy, const CWeightBalance &bCopy)
{
  CWeightBalance wb;
  //
//  wb.newCG.x               = aCopy.newCG.x              + bCopy.newCG.x;
//  wb.newCG.y               = aCopy.newCG.y              + bCopy.newCG.y;
//  wb.newCG.z               = aCopy.newCG.z              + bCopy.newCG.z;
  //
  wb.CGOffset.x            = aCopy.CGOffset.x           + bCopy.CGOffset.x;
  wb.CGOffset.y            = aCopy.CGOffset.y           + bCopy.CGOffset.y;
  wb.CGOffset.z            = aCopy.CGOffset.z           + bCopy.CGOffset.z;
  //
  wb.moments_of_inertia.x  = aCopy.moments_of_inertia.x + bCopy.moments_of_inertia.x;
  wb.moments_of_inertia.y  = aCopy.moments_of_inertia.y + bCopy.moments_of_inertia.y;
  wb.moments_of_inertia.z  = aCopy.moments_of_inertia.z + bCopy.moments_of_inertia.z;
  //
  return wb;
}
//======================END OF FILE ==============================================
