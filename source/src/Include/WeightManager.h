/*
 * WeightManager.h
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

/*! \file WEIGHT_MANAGER.h
 *  \brief Defines CWeightManager base class and descendents
 */


#ifndef WEIGHT_MANAGER_H_
#define WEIGHT_MANAGER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "../Include/FlyLegacy.h"
#include "../Include/Utility.h"
#include "../Include/3Dmath.h"
#include <vector>
//======================================================================================
class CFuelCell;
//======================================================================================
// Loadout Unit
//
// This class represents a single loadout station
//======================================================================================
class CLoadCell : public CStreamObject {

public:
  CLoadCell (CVehicleObject	*veh);

  int   Read (SStream *stream, Tag tag);
  void  Write (SStream *stream);
  void  ContributionCG(SVector &mm,SVector &vn, double &sm);
  //----------------------------------------------------------
  inline char   *GetName()        {return name;}
  inline float   GetLoad()        {return load;}
  inline float   GetLimit()       {return  hiLm;}
  inline float   GetXloc()        {return float(bPos.x);}
  inline float   GetYloc()        {return float(bPos.y);}
  inline float   GetZloc()        {return float(bPos.z);}
  inline SVector GetPosition()    {return bPos;}
  inline bool    IsEmpty()        {return (load < 0.1);}
  //----------------------------------------------------------
  inline void    SetLoad(float w) {load = w;}
protected:
  //----- ATTRIBUTES -----------------------------------------
	CVehicleObject	*mveh;		// Mother vehicle
  char  name[64];           // Name of loadout position
  CVector bPos;             // Offset from default center of gravity
  float load;               // Default load value, in pounds
  float hiLm;               // Maximum load value, in pounds
  char  utyp[64];           // UI type
};
//==========================================================================
class CWeightBalance {

  SVector CGOffset;
  SVector moments_of_inertia;
  SVector CGOffset_ISU;
  SVector moments_of_inertia_ISU;

public:
  //
  CWeightBalance        (void) {Init ();}
  void Init             (void) {CGOffset.x               = CGOffset.y                = CGOffset.z                = 0.0;
                                CGOffset_ISU.x           = CGOffset_ISU.y            = CGOffset_ISU.z            = 0.0;
                                moments_of_inertia_ISU.x = moments_of_inertia_ISU.y  = moments_of_inertia_ISU.z  = 0.0;
                                moments_of_inertia.x     = moments_of_inertia.y      = moments_of_inertia.z      = 0.0;}
  void Copy             (const CWeightBalance &aCopy)
  {
    Init ();
    CGOffset               = aCopy.CGOffset;
    CGOffset_ISU           = aCopy.CGOffset_ISU;
    moments_of_inertia     = aCopy.moments_of_inertia;
    moments_of_inertia_ISU = aCopy.moments_of_inertia_ISU;
  }
  CWeightBalance        (const CWeightBalance &aCopy) {Copy(aCopy);}

  // getters and setters
  // ===================
  // gets the CG relative to 0.0.0 initial position in feet
  SVector* GetCGOffset           (void)             {return &CGOffset;}
  // gets the CG relative to 0.0.0 initial position in ISU meters
  const SVector* GetCGOffset_ISU (void);
  // sets the CG relative to 0.0.0 initial position in feet
  void           SetCGOffset     (const SVector &v) {CGOffset = v;}
  // gets moment of inertia in lbs.ft^2
  const SVector* GetMI           (void)             {return &moments_of_inertia;}
  // gets moment of inertia in ISU Kg.m^2
  SVector* GetMI_ISU       (void);
  // sets the moment of inertia in lbs.ft^2
  void           SetMI           (const SVector &v) {moments_of_inertia = v;}
  //-----------------------------------------------------------------------
  //
  // Clone
  CWeightBalance Clone  (void)
  {
	  CWeightBalance c;
	  c.Copy(*this);
	  return c;
  }
  // Operator =
  CWeightBalance& operator=(const CWeightBalance &aCopy)
  {
	  Copy(aCopy);
	  return *this;
  }

private:
  // Operator +
  friend CWeightBalance operator+(const CWeightBalance &aCopy, const CWeightBalance &bCopy);
};

//================================================================================
//! \class CWeightManager
//  \brief This class is an interface for all object related to the user weight 
//
//========================================================================================
class CWeightManager {
  //----ATTRIBUTES ----------------------------------------
  CVehicleObject *mveh;                 // Parent Vehicle
  //----METHODS--------------------------------------------
public:
                 CWeightManager();
  virtual       ~CWeightManager(void);

protected:
  //! Constructors/destructor
  void           Debug                   (void);
  //-------------------------------------------------------
  void          FuelCGcontribution(SVector &vn,double &sm);
  void          LoadCGcontribution(SVector &vn,double &sm);
  void          ComputeVisualCG(CVector &cg);
  void          ComputeOffsetCG(CVector &cg);
  //------------Cell positions   -------------------------
public:
  void          GetFuelCell(std::vector<CFuelCell*> &vf);
  void          GetLoadCell(std::vector<CLoadCell*> &vl);
  //------CWeightManager methods --------------------------
  virtual void   Timeslice               (float dT);
  void           Init                    (void);

  // getters
  // =======
  //! gets instantaneous mass
  float          GetTotalMassInLbs       (void);
  //! gets instantaneous mass
  float          GetTotalMassInKgs       (void);
  //! sets <rmas> from WHL file
  inline void           Set_rmas                (float &mass) {whl_rmas = mass;}
  inline float  GetEmptyMassInLbs()       {return wDRY;}
  inline float  GetEmptyMassInLKgs()      {return LbsToKg (wDRY);}
  inline float  GetPayloadInLKgs()        {return LbsToKg(wGAS + wLOD);}
  inline float  GetUtilityGasInLbs()      {return wGAS;}
  //----------------------------------------------------------------------------
  inline void   AddLoad(CLoadCell *u)     {vld_unit.push_back(u);}
  inline void   GetVisualCG(SVector &v)   {v = vCG;}
  inline float  GetDryWeight()            {return float(wDRY);}
  inline float  GetGasWeight()            {return float(wGAS);}
  inline float  GetLodWeight()            {return float(wLOD);} 
  //------------Inertia vector ------------------------------------------------
  inline SVector  GetGazInertia()         {return mGAS;}
  inline SVector  GetLodInertia()         {return mLOD;}
  inline SVector  GetTotInertia()         {return (svh_mine + mGAS + mLOD);}
	//---------------------------------------------------------------------------
	inline double   GetCGHeight()					  {return svh_cofg.y;}
	inline void     SetVEH(CVehicleObject *v) {mveh = v;}
  //--------------Attributes --------------------------------------------------
  // members
  std::vector<CLoadCell*>               vld_unit;    ///< List of loadout stations
  std::vector<CFuelCell*>               gas_cell;     // List of gas cells
  float                                 svh_emas;
  CVector                               svh_mine;
  CVector                               svh_cofg;
  float                                 whl_rmas;
  CWeightBalance                        wb;
  //----------------------------------------------------------------------------
  CVector vCG;                            // visual CG vector
  CVector oCG;                            // Offset CG
  //----Masses in LBS ----------------------------------------------------------
  double   wDRY;                          // Dry  weight (emas)
  double   wGAS;                          // Fuel weight
  double   wLOD;                          // Load weight
  //-----Inertia ---------------------------------------------------------------
  CVector  mGAS;                          // MI gaz
  CVector  mLOD;                          // MI Load
  CVector  mDRY;                          // mime
  //----------------------------------------------------------------------------
protected:
  CWeightBalance                         wbVld;
  CWeightBalance                         wbGas;
};


#endif // WEIGHT_MANAGER_H_

