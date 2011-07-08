/*
 * Atmosphere.h
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

/*! \file Atmosphere.h
 *  \brief Defines CAtmosphereModel base class and descendents
 */


#ifndef ATMOSPHERE_H_
#define ATMOSPHERE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlyLegacy.h"
#include "Utility.h"


//
// CAtmosphereModel
//
// Abstract base class that all atmosphere model implementations must support
//
// 
//
class CAtmosphereModel {
public:
  // Constructors/destructor
  CAtmosphereModel (void);
  virtual ~CAtmosphereModel (void);

  // CAtmosphereModel methods
  // Set baseline conditions
  virtual void  SetTemperatureC (float m_msl, float t) = 0;
  virtual void  SetPressureKgM2 (float m_msl, float p) = 0;

  // Get temperature at specified position in various units
  virtual float GetTemperatureC (void);
  virtual float GetTemperatureK (void);
  virtual float GetTemperatureF (void);
  virtual float GetTemperatureR (void);

  // Get dewpoint in various units
  virtual float GetDewpointC (void);
  virtual float GetDewpointK (void);
  virtual float GetDewpointF (void);
  virtual float GetDewpointR (void);

  // Get relative humidity in percentage
  virtual float GetRelativeHumidity (float m_msl);

  // Get atmospheric pressure in various units
  virtual float GetPressureKgM2 (void);
  virtual float GetPressureSlugsFtSec (void);
  virtual float GetPressureInchesHg (void); 

  // Get air pressure in various units
  virtual float GetDensityKgM3 (void);
  virtual float GetDensitySlugsFt3 (void);

protected:
};



//
// CAtmosphereModelFly2
//
// This concrete CAtmosphereModel subclass implements the standard Fly! II
//   atmosphere model.  This is similar to the International Standard
//   Atmosphere with some important differences.  See Docs/Atmosphere.doc
//   for details.
//
class CAtmosphereModelFly2 {
public:
  // Constructors/destructor
  CAtmosphereModelFly2 (void);

  // CAtmosphereModel methods
  // Set baseline conditions
  void  SetTemperatureC (float m_msl, float t);
  void  SetPressureKgM2 (float m_msl, float p);

  // Get temperature at specified position in various units
  float GetTemperatureC (float m_msl);
  float GetTemperatureK (float m_msl);
  float GetTemperatureF (float ft_msl);
  float GetTemperatureR (float ft_msl);

  // Get dewpoint in various units
  float GetDewpointC (float m_msl);
  float GetDewpointK (float m_msl);
  float GetDewpointF (float ft_msl);
  float GetDewpointR (float ft_msl);

  // Get relative humidity in percentage
  // Fly! II uses a dry air model, no humidity
  float GetRelativeHumidity (float m_msl) { return 0.0f; };

  // Get atmospheric pressure in various units
  float GetPressureKgM2 (float m_msl);
  float GetPressureSlugsFtSec (float ft_msl);
  float GetPressureInchesHg (float ft_msl); 

  // Get air pressure in various units
  float GetDensityKgM3 (float m_msl);
  float GetDensitySlugsFt3 (float ft_msl);

protected:
  float   tempOffset;       // Sea Level temperature offset from +15 C (in deg C)
  float   pressureOffset;   // Sea Level pressure offset from 1.013E+5 Pa (in Pa)
};


//
// CAtmosphereModelJSBSim
//
// Developped from JSBSim model, see below :

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

 Header:       FGInertial.h
 Author:       Jon S. Berndt
 Date started: 09/13/00

 ------------- Copyright (C) 1999  Jon S. Berndt (jsb@hal-pc.org) -------------

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free Software
 Foundation; either version 2 of the License, or (at your option) any later
 version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 details.

 You should have received a copy of the GNU General Public License along with
 this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 Place - Suite 330, Boston, MA  02111-1307, USA.

 Further information about the GNU General Public License can also be found on
 the world wide web at http://www.gnu.org.

HISTORY
--------------------------------------------------------------------------------
09/13/00   JSB   Created

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/


//
// CLASS CInertial
// Models inertial forces (e.g. centripetal and coriolis accelerations).
//
class CInertial {

public:
  CInertial(void);
//  ~CInertial(void);

  bool Run(void);
//  bool LoadInertial(FGConfigFile* AC_cfg);
  double SLgravity(void)      const {return gAccelReference;}
  double gravity(void)        const {return gAccel;}
  double omega(void)          const {return RotationRate;}
  double GetGAccel(double r)  const {return GM/(r*r);}
  double RefRadius(void)      const {return RadiusReference;}

private:
  double gAccel;
  double gAccelReference;
  double RadiusReference;
  double RotationRate;
  double GM;
//  void Debug(int from);
};

/*!
 Models the standard atmosphere.

 Header:       FGAtmosphere.h
 Author:       Jon Berndt
               Implementation of 1959 Standard Atmosphere added by Tony Peden
 Date started: 11/24/98

 Modifications by :
    @author Tony Peden, Jon Berndt
    @version $Id: Atmosphere.h,v 1.23 2010/11/19 18:09:21 sabatier Exp $
    @see Anderson, John D. "Introduction to Flight, Third Edition", McGraw-Hill,
         1989, ISBN 0-07-001641-0
*/

//! Used with CSituation::Timeslice
//! and InitGlobalsNoPodFilesystem
//============================================================================
//  Revisited by JS to unify temperature and pressure for weather
//  manager
//============================================================================
class CAtmosphereModelJSBSim {
  // Constructors/destructor
public:
  CAtmosphereModelJSBSim (void);

public:
  virtual ~CAtmosphereModelJSBSim (void);
  void    TimeSlice(float dT,double alt);
  void    InitModel(void);
  void    LocalTempC(float t);
  void    LocalPressureHPA(float p);
  //----------------------------------------------------------------
  // CAtmosphereModel methods
  /// Returns the speed of sound in ft/sec.
  inline float GetSoundSpeed     (void) const {return soundspeed;}
  inline float GetSoundSpeed_ISU (void) const {return FeetToMetres (soundspeed);}
  /// Returns the sea level temperature in degrees Rankine.
  inline float GetTemperatureSL  (void) const { return SLtemp; }
  /// Returns the sea level density in slugs/ft^3
  inline float GetDensitySL      (void)  const { return SLdens; }
  /// Returns the sea level pressure in psf.
  /// 1 (pound second) per foot = 1.48816394 kg s / m = 1 Pa.s
  /// = 47.88 Newton second per square meter = 1 Pa.s
  inline float GetPressureSL()      {return SLpres; }
  inline float GetPressureSLinHG()  {return SLpresHG;}
  /// Returns the sea level speed of sound in ft/sec.
  inline float GetSoundSpeedSL   (void) const { return SLsoundspeed; }
  
  // Get temperature at specified position in various units
  float GetTemperatureC() {return tempC;}
  float GetTemperatureF() {return tempF;}
  float GetTemperatureR() {return tempR;}

  // Get dewpoint in various units
  /// \todo this dew functions
  float GetDewpointC            (void);
  float GetDewpointK            (void);
  float GetDewpointF            (void);
  float GetDewpointR            (void);

  // Get relative humidity in percentage
  // Fly! II uses a dry air model, no humidity
  float GetRelativeHumidity  (float m_msl) { return 0.0f;}

  // Get atmospheric pressure in various units
  float  GetPressureSlugsFtSec(){return presS;} /// in psf
  float  GetPressureInHG()      {return presH;} 
  float  GetPressureHPA()       {return presB;} 
  float  GetPressurePSF()       {return presS;}
  // Get air pressure in various units
  double GetDensityKgM3         (void) {return (SLUGS_FT3_TO_KGM3 * densD);}
  double GetDensitySlugsFt3     (void) {return densD;}

protected:
  //---- Attributes --------------------------------------
  CInertial *pInertial;
  double rho;

  enum tType {ttStandard, ttBerndt, ttNone} turbType;
  //---Sea level parameters ------------------------------
  float SLtemp,SLdens,SLpres,SLsoundspeed;
  float SLpresHG;
  float rSLtemp,rSLdens,rSLpres,rSLsoundspeed; //reciprocals
  float soundspeed;
  //--- Valuators ---------------------------------------
  CValuator tVAL;                 // Temperature
  CValuator pVAL;                 // Pressure
  CValuator dVAL;                 // Density
  //---Temperatures data --------------------------------
  float tempR;                    // Temp in rankine scale
  float tempF;                    // Temp in Fahrenheit
  float tempC;                    // Temp in Celsius
  //--- Local temperature delta from metar ---------------
  float dtaTC;                    // In Celsius
  //---Density data -------------------------------------
  float densD;                    // Pressure in slug per cubic foot
  //---Pressure data ------------------------------------
  float presS;                    // Pressure in Slug/ft²
  float presH;                    // Pressure in inHG
  float presB;                    // Pressure in millibar (hPa)
  //--- Local Pressure delta from metar ------------------
  float dtaPB;                    // In mbar
  float dtaPS;                    // In slug/ft²
  //------------------------------------------------------
  float density_altitude;
  
  double MagnitudedAccelDt, MagnitudeAccel, Magnitude;
  double TurbGain;
  double TurbRate;
  double psiw;

};


#endif // ATMOSPHERE_H_


