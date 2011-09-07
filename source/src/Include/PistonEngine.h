//===================================================================================
// PistonEngine.h
//
// Part of Fly! Legacy project
//
// Copyright 2003 Chris Wallace
//
// Fly! Legacy is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
// Fly! Legacy is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
//   along with Fly! Legacy; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//====================================================================================
#ifndef PISTONENGINE_H
#define PISTONENGINE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//==============================================================================
#include "../Include/Subsystems.h"
//==============================================================================
class CFuiPlot;
//=============================================================================
//  CPiston 
//
//=============================================================================
class CPiston : public CEngineModel {
public:
  CPiston (CVehicleObject *v, CPropellerModel *prop);
  virtual ~CPiston (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPiston"; }

  // class methods
  virtual void   eCalculate (float dT) {;}
  virtual double GetPowerAvailable (void) const {return eData->e_PWR;}

  virtual double GetEGT (void) const { return EGT_degC; }

  virtual double getExhaustGasTemp_degF (void) const {return KelvinToFahrenheit(ExhaustGasTemp_degK);}
  virtual double getOilPressure_psi (void) const {return OilPressure_psi;}
  virtual double getOilTemp_degF (void) const {return KelvinToFahrenheit(OilTemp_degK);}
  virtual double getRPM (void) const {return RPM;}
  virtual void   setMinMaxMAP (const double &min_inHg, const double &max_inHg);
  virtual CDependent *GetPart() {return this;}
 //----------------------------------------------------------------------------------
protected:
  double BrakeHorsePower;
  double SpeedSlope;
  double SpeedIntercept;
  double AltitudeSlope;

  CPropellerModel *p_prop;

  virtual void    doEngineRefresh(void);
  virtual void    doBoostControl(void);
  virtual void    doMAP(void);
  virtual void    doAirFlow(void);
  virtual void    doFuelFlow(void);
  virtual void    doEGT(void);
  virtual void    doCHT(void);
  virtual void    doOilPressure(void);
  virtual void    doOilTemperature(void);
  virtual double  GetEnginePower(void);
  virtual double  GetPower_Mixture_CorrelationValue(const double &ratio);
  virtual double  GetCombustion_EfficiencyValue(const double &ratio);

  //
  // constants
  //

  double R_air;
  double rho_fuel;             ///< kg/m^3
  double calorific_value_fuel; ///< W/Kg (approximate)
  double Cp_air;               ///< J/KgK
  double Cp_fuel;              ///< J/KgK
  double ENGINE_THRUST_COEFF;  ///< used to adjust the overall computation

  //
  // Configuration
  //
  double MinManifoldPressure_inHg;   ///< Inches Hg
  double MaxManifoldPressure_inHg;   ///< Inches Hg
  double Displacement;               ///< cubic inches
  double MaxHP;                      ///< horsepower
  double Cycles;                     ///< cycles/power stroke
  double IdleRPM;                    ///< revolutions per minute

  double minMAP;                     ///< Pa
  double maxMAP;                     ///< Pa
  double ampMAP;                      // MAP amplitude   
  double MAP;                        ///< Pa

  //
  // Inputs
  //
  double  p_amb;                     ///< Pascals
  double  p_amb_sea_level;           ///< Pascals
  double  T_amb;                     ///< degrees Kelvin
  double  RPM;                       ///< revolutions per minute
  double  ThrustPowerRequired;       ///< 

  //
  // Outputs
  //
  double rho_air;
  double volumetric_efficiency;
  double m_dot_air;
  double equivalence_ratio;
  double m_dot_fuel;
  double Percentage_Power;
  double combustion_efficiency;
  double ExhaustGasTemp_degK;
  double EGT_degC;
  double CylinderHeadTemp_degK;
  double OilPressure_psi;
  double OilTemp_degK;

  //
  // extra
  //
  int eng_auto_strt;
};
//=================================================================================
//
//  CPistonJSBSim
//
//  Currently not used
//=================================================================================
const int FG_MAX_BOOST_SPEEDS = 3;          // tmp = 0 (no boost) fix later

class CPistonJSBSim : public CPiston {
public:
  CPistonJSBSim (CVehicleObject *v, CPropellerModel *prop,CEngineData *ed);
  virtual ~CPistonJSBSim (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPistonJSBSim"; }

  // class methods
  virtual void   eCalculate (float dT);
  virtual double GetPowerAvailable () {return eData->e_PWR;}

private:
  virtual void   doBoostControl(void);
  virtual void   doMAP(void);
  virtual void   doOilPressure(void);
  virtual void   doOilTemperature(void);
  virtual void   doCHT(void);
  virtual void   doEGT(void);
  virtual double GetEnginePower(void);
  //-------------------------------------------------------------------
  int BoostSpeeds;	                          // Number of super/turbocharger boost speeds - zero implies no turbo/supercharging.
  int BoostSpeed;	                          // The current boost-speed (zero-based).
  bool Boosted;		                          // Set true for boosted engine.
  int BoostOverride;	                      // The raw value read in from the config file - should be 1 or 0 - see description below.
  bool bBoostOverride;	                      // Set true if pilot override of the boost regulator was fitted.
                                              // (Typically called 'war emergency power').
  bool bTakeoffBoost;	                      // Set true if extra takeoff / emergency boost above rated boost could be attained.
                                              // (Typically by extra throttle movement past a mechanical 'gate').
  double TakeoffBoost;	                      // Sea-level takeoff boost in psi. (if fitted).
  double RatedBoost[FG_MAX_BOOST_SPEEDS];	  // Sea-level rated boost in psi.
  double RatedAltitude[FG_MAX_BOOST_SPEEDS];  // Altitude at which full boost is reached (boost regulation ends)
                                              // and at which power starts to fall with altitude [ft].
  double RatedRPM[FG_MAX_BOOST_SPEEDS];       // Engine speed at which the rated power for each boost speed is delivered [rpm].
  double RatedPower[FG_MAX_BOOST_SPEEDS];	  // Power at rated throttle position at rated altitude [HP].
  double BoostSwitchAltitude[FG_MAX_BOOST_SPEEDS - 1];	// Altitude at which switchover (currently assumed automatic)
                                              // from one boost speed to next occurs [ft].
  double BoostSwitchPressure[FG_MAX_BOOST_SPEEDS - 1];  // Pressure at which boost speed switchover occurs [Pa]
  double BoostMul[FG_MAX_BOOST_SPEEDS];	      // Pressure multipier of unregulated supercharger
  double RatedMAP[FG_MAX_BOOST_SPEEDS];	      // Rated manifold absolute pressure [Pa] (BCV clamp)
  double TakeoffMAP[FG_MAX_BOOST_SPEEDS];	  // Takeoff setting manifold absolute pressure [Pa] (BCV clamp)
  double BoostSwitchHysteresis;	              // Pa.
};
//=========================================================================
//
//  CPistonTRI1
//
//  This class acts as a CEngineModel specialisation for piston engines
//=========================================================================
class CPistonTRI1 : public CPiston {
public:
  CPistonTRI1 (CVehicleObject *v, CPropellerModel *prop,CEngineData *ed);
  virtual ~CPistonTRI1 (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPistonTRI1"; }

  // class methods
  virtual void   eCalculate (float dT);
  virtual double GetPowerAvailable (void) {return eData->e_PWR;}

  virtual void   SetBoosted (void) {Boosted = true;}
  //-------------------------------------------------------------------
  void  Probe(CFuiCanva *cnv);
  //-------------------------------------------------------------------
  CDependent *GetPart() {return this;}
  //-------------------------------------------------------------------
private:
  virtual void   doBoostControl(void);
  virtual void   doMAP(void);
  virtual void   doOilPressure(void);
  virtual void   doOilTemperature(void);
  virtual void   doCHT(void);
  virtual void   doEGT(void);
  virtual void   doAirFlow(void);
  virtual void   doFuelFlow(void);
  virtual double GetEnginePower(void);
  //--------------------------------------------------------------------
  bool Boosted;		                          // Set true for boosted engine.
  float Kegt;                               // EGT coeff
};
//==============================================================================
typedef enum {
  PROP_TRI  = 0,
  PROP_JSB  = 1,
} EPistonType;
//==============================================================================
//
//
//  CPistonEngineModel
//
//  This class acts as a CEngineModel specialisation for piston engines
//
//==============================================================================
class CPistonEngineModel : public CEngineModel {
public:
  CPistonEngineModel (CVehicleObject *v,int eNo);
 ~CPistonEngineModel (void);

  // CStreamObject methods
  int         Read (SStream *stream, Tag tag);
  void        ReadFinished (void);

  // CSubsystem methods
  const char* GetClassName (void) { return "CPistonEngineModel"; }
  void        TimeSlice (float dT,U_INT FrNo = 0);					// JSDEV*
  void        SetMixture();
  void        SetTrueRPM();
  CDependent *GetPart(char k);
  //-----Plotting interface -----------------------------------------
  int         AddToPlotMenu(char **menu, PLOT_PM *pm, int k);
  bool        PlotParameters(PLOT_PP *pp, Tag id, Tag type);
  void        Plot(PLOT_PP &pp);
  //------------------------------------------------------------------------
  inline CPropellerModel *GetProp()     {return e_prop;}
  inline  void		Abort()       {e_prop->SetRPM(400);}
  inline	void		Idle()        {e_prop->SetRPM(600);}
	//--- Reset --------------------------------------------------------
	inline	void		Reset()	{e_prop->Reset();}
  //--  Attributes ---------------------------------------------------------
protected:
  CPropellerModel *e_prop;

  bool  boosted;
  float boostAltitude;            ///< Altitude (ft MSL) at which boost kicks in
  float boostPressure;            ///< Fuel pressure (in. Hg) at boost
  float egts, chts, ffsc, idle;

  //--- table look up for engine ---------------------------
  CFmtxMap * mphpaf;
  CFmtxMap * mpfttb;
  CFmtxMap * mpegta;
  CFmtxMap * mpchta;
  CFmtxMap * mpegtr;
  CFmtxMap * mpchtr;

private:
  float pTime;                    // Plotting time
  char type;
  CPiston  *piston;
};

//=========================END OF FILE ========================================
#endif // PISTONENGINE_H
