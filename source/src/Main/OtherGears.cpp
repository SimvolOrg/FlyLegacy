/*
 * OtherGears.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
 * Copyright 2005 Laurent Claudet
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

#include "../Include/Globals.h"
#include "../Include/FlyLegacy.h"
#include "../Include/Gears.h"
using namespace std;
//====================================================================================
//===============================================================================
/*!
 * CGearJSBSim
 */
//===============================================================================

CGearJSBSim::CGearJSBSim (CVehicleObject *v,CSuspension *s) : CGear (v,s)
{

}
//------------------------------------------------------------------------
CGearJSBSim::~CGearJSBSim (void)
{ }
//------------------------------------------------------------------------
//* calculate the gear compression if the weight is acting on strut.
//* = the difference between the AGL and the wheel position
//* returns the WOW flag
//------------------------------------------------------------------------
char CGearJSBSim::GCompression  (char pp)
{
  return 0;
}
//------------------------------------------------------------------------
//* calculates the gear strut compression speed
//* vWhlVelVec = instantaneous velocity vector in body frame (m/s)
//* compression speed in body frame = Z-component vWhlVelVec.z
//------------------------------------------------------------------------
void CGearJSBSim::GComprV_Timeslice (void)
{}
//------------------------------------------------------------------------
//* compute the vertical force on the wheel using square-law damping
//* vLocalForce = local forces applied to the aircraft in body frame
//* body frame : in lbs
//------------------------------------------------------------------------
void CGearJSBSim::VtForce_Timeslice (float dT)
{}

//------------------------------------------------------------------------
//* compute the steering forces on the wheel with yaw
//* vLocalForce = local forces applied to the aircraft in body frame
//* body frame : in lbs
//  JS NOTE: Is that an additional force to the actual steering by the pilot
//           or the total steering direction???
//           For total force use:       susp->TurnWheelTo(xx)
//           For Additional force use:  susp->PushWheelTo(xx)
//           and set gearData->kframe to 0 before
//------------------------------------------------------------------------
void CGearJSBSim::DirectionForce_Timeslice (float dT)
{}

//------------------------------------------------------------------------
//* transform the forces back to the inertial frame : in lbs
//------------------------------------------------------------------------
void CGearJSBSim::GearL2B_Timeslice (void)
{}
//------------------------------------------------------------------------
//* compute moment in body coordinates : in lbs.ft
//------------------------------------------------------------------------
void CGearJSBSim::VtMoment_Timeslice (void)
{}
//------------------------------------------------------------------------
///< force in Newton
//------------------------------------------------------------------------
const SVector& CGearJSBSim::GetBodyGearForce_ISU  (void)
{
  return CGear::GetBodyGearForce_ISU ();
} 
//------------------------------------------------------------------------
//< moment in Kg.m
//------------------------------------------------------------------------
const SVector& CGearJSBSim::GetBodyGearMoment_ISU (void)
{
  double konst = LBFT_TO_KGM (1.0, 1.0);//0.1382566

  vLocalMoment_ISU.x = vLocalMoment.x * konst;
  vLocalMoment_ISU.y = vLocalMoment.y * konst * -1000.0; ///
  vLocalMoment_ISU.z = vLocalMoment.z * konst;
  return vLocalMoment_ISU;
} 


//=======END of FILE =================================================================
