/*
 * Atmosphere.cpp
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

/*! \file Atmosphere.cpp
 *  \brief Implements CAtmosphereModel and related atmosphere modelling classes
 *
 * Fly! Legacy has the potential to support a anumber of selectable
 *   atmosphere models.  All atmosphere models are derived from the
 *   abstract class CAtmosphereModel.  Given a set of initial conditions
 *   including:
 *     - surface temperature and altitude MSL
 *     - surface barometric pressure (adjusted to zero MSL)
 *     - surface relative humidity (or dewpoint)
 *   an atmosphere model must be able to provide the temperature,
 *   pressure and density at any given altitude MSL.
 *
 */


#include "../Include/Atmosphere.h"
#include "../Include/Globals.h"
//==============================================================================
//  JS:  Put atmosphere in a CmvalMap format
//  Pressure are in pound force per square foot  PSF
//==============================================================================
TUPPLE3 stdATPM[] = {
  //--Altitude (ft)--Slope-------Temper(Ra)--Pressure (psf) -Density (slug/ft3) ----
  {         0, -0.00356616f,    518.67f,   2116.22f},       // 0.00237767f
  {     36089,  0,              389.97f,    472.452f},      // 0.000706032f;
  {     65616,  0.00054864f,    389.97f,    114.636f},      // 0.000171306f;
  {    104986,  0.00153619f,    411.57f,      8.36364f},    // 1.18422e-05;
  {    154199,  0,              487.17f,      0.334882f},   // 4.00585e-7;
  {    170603, -0.00109728f,    487.17f,      0.683084f},   // 8.17102e-7;
  {    200131, -0.00219456f,    454.17f,      0.00684986f}, // 8.77702e-9;
  {    259186,  0,              325.17f,      0.000122276f},// 2.19541e-10;
  {-1,-1},          // Last non valid entry
};
//==============================================================================
//  Standard atmosphere look up table
//==============================================================================
CmvalMap stdATMOS;
//==============================================================================
// CAtmosphereModelFly2
//==============================================================================
CAtmosphereModelFly2::CAtmosphereModelFly2 (void)
{
}


//
// CAtmosphereModelJSBSim
//

CInertial::CInertial(void)
{
//  Name = "FGInertial";

  // Defaults
  RotationRate    = EARTH_SIDERAL_ROTATION_RATE_SI; ///< 0.00007272205217;
  GM              = 14.06252720E15;
  RadiusReference = EQUATORIAL_RADIUS_FEET;         ///< 20925650.00;
  gAccelReference = GM/(RadiusReference*RadiusReference);
  gAccel          = GM/(RadiusReference*RadiusReference);

//  Debug(0);
}

bool CInertial::Run(void)
{
//  // Fast return if we have nothing to do ...
////  if (FGModel::Run()) return true;
//
//  // Gravitation accel
//  double r = Propagate->GetRadius();
//  gAccel = GetGAccel(r);
//
  return false;
}


/*
 Models the standard atmosphere.

 Header:       FGAtmosphere.h
 Author:       Jon Berndt
               Implementation of 1959 Standard Atmosphere added by Tony Peden
 Date started: 11/24/98

 Modifications by :
    @author Tony Peden, Jon Berndt
    @version $Id: Atmosphere.cpp,v 1.20 2010/11/19 18:09:58 sabatier Exp $
    @see Anderson, John D. "Introduction to Flight, Third Edition", McGraw-Hill,
         1989, ISBN 0-07-001641-0
*/

// 
#define SHRatio   double(1.40)
#define Reng      double(1716.0)         /* specific gas const */

//==============================================================================
//  JSBSIM revisited by JS to unify temperature, presure and all between
//  the aircraft and the weather manager
//==============================================================================
CAtmosphereModelJSBSim::CAtmosphereModelJSBSim (void)
{ //MEMORY_LEAK_MARKER ("pInertial");
  pInertial = new CInertial;
  //MEMORY_LEAK_MARKER ("pInertial");
//  Name = "FGAtmosphere";
  psiw = 0.0;

  MagnitudedAccelDt = MagnitudeAccel = Magnitude = 0.0;
  turbType = ttStandard;
  TurbGain = 0.0;
  TurbRate = 1.0;

  //---Load table lookup for standard atmosphere --------
  stdATMOS.Load(stdATPM);
  //---Init base parameters ----------------------------
  InitModel();
  globals->atm  = this;
	//--- Enter in dispatcher --------------------------------------------
	globals->Disp.Enter(this,PRIO_ATMOSPHERE);

}
//------------------------------------------------------------------
//  Free all resourecs 
//-----------------------------------------------------------------
CAtmosphereModelJSBSim::~CAtmosphereModelJSBSim (void) {
  globals->atm = 0;
  SAFE_DELETE (pInertial);
}

//=========================================================================
//  Update Temperature, pressure and density from altitude
//  NOTE:  Temperature is adjusted with the local conditions
//         if local condition say 20°C, then temperature is adusted 
//         by the delta (20 - 15) because 15°C is the standard temperature
//         at sea level;  We just make a (certainly) wrong supposition by 
//         saying that temperature is 5° higgher at all altitude
//=========================================================================
int CAtmosphereModelJSBSim::TimeSlice(float dt,U_INT frame)
{ double altitude = globals->geop.alt;
	//---Compute temperature and pressure -----------------------
  C3valSlot *slot = stdATMOS.Getfloor(altitude);
  float slp  = slot->GetU();
  float rfT  = slot->GetV();
  float rfP  = slot->GetW();
  float da   = altitude - slot->GetX();
  float tp   = 0;
  float pr   = 0;
  float dn   = 0;
  //---Compute new targets ---------------------
  if (0 == slp)
  { tp = rfT;
    pr = rfP * exp(-pInertial->SLgravity()/(rfT*Reng)* da);
    dn = pr/ (Reng*tp);
  }
  else 
  { tp = rfT + (slp * da);
    pr = rfP * pow(float(tp/rfT),float(-pInertial->SLgravity()/(slp*Reng)));
    dn = pr/(Reng*tp);
  }
  //--- Get final values -----------------------------------------------
  tempR = tVAL.TimeSlice(dt);
  presS = pVAL.TimeSlice(dt);
  densD = dVAL.TimeSlice(dt);
  //--- Set Target value -----------------------------------------------
  tVAL.Set(tp);
  pVAL.Set(pr);
  dVAL.Set(dn);
  //---Update temperature to various units -----------------------------
  tempC   = RankineToCelsius(tempR) + dtaTC;
  tempF   = CelsiusToFahrenheit(tempC);
  //---Update pressure to various units --------------------------------
  presS  += dtaPS;                    // Add local deviation
  presH   = presS * PSF_TO_INHG;
  presB   = presS * PFS_TO_HPA;
  //--- Update sound speed ---------------------------------------------
  soundspeed = sqrt(SHRatio*Reng*(tempR));
	return 1;
}
//--------------------------------------------------------------------
//  Compute sea level parameters 
//---------------------------------------------------------------------
void CAtmosphereModelJSBSim::InitModel(void)
{
//  FGModel::InitModel();
  //--- Assume no temperature deviation from standard
  dtaTC = 0;
  //--- Assume no pressure deviation ----------------
  dtaPS = dtaPB = 0;
  //--- Assume standard density in slugper cubic foot
  densD   = 0.00237767f;
  tempR   = 32;
  //---Init valuators -----------------------------------
  tVAL.Conf(INDN_LINEAR,1);
  pVAL.Conf(INDN_LINEAR,1);
  dVAL.Conf(INDN_LINEAR,1);
	globals->geop.alt = 0;
  TimeSlice(0,0);         //  Set Target
  TimeSlice(1,0);         //  Get Values
  //---Set sea level parameters --------------
  SLtemp    = tempR;        // Rankine
  SLpres    = presS;        // .
  SLdens    = densD;        // .
  SLpresHG  = presS * PSF_TO_INHG; 
  SLsoundspeed    = sqrt(SHRatio*Reng*tempR);
  rSLtemp         = 1.0/tempR;
  rSLpres         = 1.0/presS;
  rSLdens         = 1.0/densD;
  rSLsoundspeed   = 1.0/SLsoundspeed;
  return;
}
//--------------------------------------------------------------------
//  Set local temperature 
//---------------------------------------------------------------------
void CAtmosphereModelJSBSim::LocalTempC(float t)
{ dtaTC = t - 15;           // Deviation from standard
  return;
}
//--------------------------------------------------------------------
//  Set loacl pressure in hPa
//---------------------------------------------------------------------
void CAtmosphereModelJSBSim::LocalPressureHPA(float p)
{ dtaPB = p - 1013;           // Deviation from standard
  dtaPS = HPA_TO_PSF * dtaPB;
  return;
}

//--------------------------------------------------------------------
//  Compute Dew point
//---------------------------------------------------------------------
float CAtmosphereModelJSBSim::GetDewpointF (void)
{
  return (GetTemperatureF() - 10.0f);
}

float CAtmosphereModelJSBSim::GetDewpointC (void)
{
  return (FahrenheitToCelsius (GetDewpointF ()));
}

//================END OF FILE ======================================================================