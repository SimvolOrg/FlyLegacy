//=====================================================================================
// CPistonEngine.cpp
//
// Part of Fly! Legacy project
//
// Copyright 2003-2005 Chris Wallace
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
//  You should have received a copy of the GNU General Public License
//   along with Fly! Legacy; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//===============================================================================
#include "../Include/Subsystems.h"
#include "../Include/Globals.h"
#include "../Include/PistonEngine.h"
#include "../Include/Atmosphere.h"
#include "../Include/Fui.h"
#include "../Include/FuiUser.h"

using namespace std;

//====================================================================================
//
// Piston engine simulation
//====================================================================================
//=============================================================================
// CPistonTRI1 model
//=============================================================================
CPistonTRI1::CPistonTRI1 (CPropellerModel *prop,CEngineData *ed)
: CPiston (prop)
{ //--- Forge id -----------------------------------
  uNum    = prop->GetUnum();
  Tag id  = 'Eps0' + uNum;
  SetIdent(id);
  hwId    = HW_PISTON;
  timK    = 0.05f;
  indn    = 0;
  //--- init parameters ----------------------------
  eData   = ed;
  Boosted = false;
  //boostAlt = boostPressure = 0.0;
  /// \todo remove later tmp
  MaxHP = double(eData->rbhp);//180;
  prop->SetMinRPM (eData->irpm);
  // 
  if (globals->uph) {
    Kegt = globals->uph->Kegt;
    DEBUGLOG ("PHY : Kegt=%f", Kegt);
  } else {
    Kegt = 1.0f;
  }
}
//-------------------------------------------------------------------------
//  Destroy object
//-------------------------------------------------------------------------
CPistonTRI1::~CPistonTRI1 (void)
{}

//--------------------------------------------------------------------------------------
//  Compute engine data
//  JS NOTE:  Removing calls to DoEGT, DoCHT and doOilPressure because all planes include
//            table lookup depending on RPM for those data.  
//            Those tables are defined in NGN file and should still
//            be valid if RPM are OK.
//--------------------------------------------------------------------------------------
void CPistonTRI1::eCalculate(float dT)
{ CSubsystem::TimeSlice(dT,0);            // Compute MAP
  CEngine *meng = eData->meng;
  //
  // Input values.
  //
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
  /// These values have to be adjusted when we get atmosphere data
  /// Pressure is provided in dynamic viscosity = slugs per foot second
  /// Computation are done in Pa.s Transformation id done multiplying by 47.88
  p_amb = globals->atm->GetPressureSlugsFtSec () * 47.88;  // (2116.2821) // convert from lbs/ft.s to Pa.s
//  p_amb = 101327.58;//29.90;//0.2991;
  p_amb_sea_level = globals->atm->GetPressureSL () * 47.88; // (2116.2821)
//  p_amb_sea_level = 101327.58;//29.92;//0.2991;
  T_amb = globals->atm->GetTemperatureR () * (5.0 / 9.0);  // convert from Rankine to Kelvin
//  T_amb = 535.6 * (5.0 / 9.0);

  /// These values are sent by the TimeSlice function
  RPM = p_prop->GetERPM();
  // 
 // doEngineRefresh();                      // OK !
  doMAP();                                // OK !
  doAirFlow();                            // OK !
  doFuelFlow();                           // OK !

  //Now that the fuel flow is done check if the mixture is too lean to run the engine
  //Assume lean limit at 22 AFR for now - thats a thi of 0.668
  //This might be a bit generous, but since there's currently no audiable warning of impending
  //cutout in the form of misfiring and/or rough running its probably reasonable for now.
	U_INT mix = (equivalence_ratio < 0.668)?(0):(ENGINE_MIXTURE);
	meng->SetMixture(mix);
  eData->e_HP = GetEnginePower();           // OK !
  doOilTemperature(); 
	//--- Now we can refresh engine state --------------------------------
	doEngineRefresh();
  double pwr      = p_prop->GetPowerRequired ()      /* * hptoftlbssec*/;
  eData->e_PWR    = HP_TO_FTLBS(eData->e_HP) - (pwr);
  eData->e_Thrust = p_prop->Calculate(eData->e_PWR) * ENGINE_THRUST_COEFF;
  eData->e_Torque = p_prop->GetTorque (); 
  eData->e_Pfac   = p_prop->GetThrustXDispl ();
  return;
}

//-------------------------------------------------------------------------
// Calculate the Current Boost Speed
//
// This function calculates the current turbo/supercharger boost speed
// based on altitude and the (automatic) boost-speed control valve configuration.
//
// Inputs: p_amb, BoostSwitchPressure, BoostSwitchHysteresis
//
// Outputs: BoostSpeed
//-------------------------------------------------------------------------
void CPistonTRI1::doBoostControl(void)
{}
//-------------------------------------------------------------------------
// Calculate the manifold absolute pressure (MAP) in inches hg
//
// This function calculates manifold absolute pressure (MAP)
// from the throttle position, turbo/supercharger boost control
// system, engine speed and local ambient air density.
//
// Inputs: minMAP, maxMAP, p_amb, Throttle
//
// Outputs: MAP, manifold in PASCAL
//-------------------------------------------------------------------------
void CPistonTRI1::doMAP(void)
{ // And set the value in American units as well
  MAP = minMAP + (eData->e_thro * ampMAP);
  MAP *= p_amb / p_amb_sea_level;
  indnTarget    = MAP;
  eData->e_Map  = indn; 
  eData->e_hMap = PAS_TO_INHG * indn;
  return;
}

//-------------------------------------------------------------------------
// Calculate the air flow through the engine.
// Also calculates ambient air density
// (used in CHT calculation for air-cooled engines).
//
// Inputs: p_amb, R_air, T_amb, MAP,
//   
//
// TODO: Model inlet manifold air temperature.
//
// Outputs: rho_air, m_dot_air
// JS NOTE:  First the fuel flow in pph (part per hour) is calculated:
//          FuelFlow(pph) = idleFF(pph) + (MapRatio * ampFF(pph)) 
//          where idleFF(pph) = fuel flow at idle RPM in pph
//                 ampFF(pph) = maxFlow - IdleFF
//          Then it is converted in Air Flow in PPS (part per second)
//          The optimized burning ratio assumed here is  
//          14.47596 pph of air for one pph of fuel.
//          Thus the AIR_RATIO_PPS is 14.47596 / 3600 = 0.0040211
//          AirFlow(pps) = FuelFlow(pph) * AIR_RATIO_PPS
//-------------------------------------------------------------------------
#define BURNING_AIR_PPS (double (0.0040211))
//-------------------------------------------------------------------------
void CPistonTRI1::doAirFlow(void)
{ rho_air = p_amb / (R_air * T_amb);

  // there seems to be not relation between RPM
  // and the air flow in the Ctr+D Debug engine
  m_dot_air = (eData->idle + (((eData->e_Map - minMAP) / ampMAP) * (eData->ffsc - eData->idle))) * BURNING_AIR_PPS;
  eData->e_af = m_dot_air;                    // Air flow in pps
}
//-------------------------------------------------------------------------
// Calculate the fuel flow into the engine.
//
// Inputs: Mixture, thi_sea_level, p_amb_sea_level, p_amb, m_dot_air
//
// Outputs: equivalence_ratio, m_dot_fuel
// JS NOTES:  To compute Fuel flow in GPH (gallons per hour)
//            from the previous computation
//            AirFlow(pps) = FuelFlow(pph) * BURNING_AIR_PPS
//            we have
//            FuelFlow(pph)= AirFlow(pps)  / BURNING_AIR_PPS
//            FuelFlow(gph)= FuelFlow(pph) * GALLON_FROM_PPH;
//            There are 5.92 PPH per US GALLON
//  QUESTION:
//            Does m_dot_fuel is in pps (as it is assumed now)
//-------------------------------------------------------------------------
#define BURNING_AIR_PPH     (double (BURNING_AIR_PPS * 3600))
#define GALLON_FROM_PPH(X)  (double (X * 0.168918918))
//--------------------------------------------------------------------------
void CPistonTRI1::doFuelFlow(void)
{ double thi_sea_level = 1.3 * eData->e_mixt;
  equivalence_ratio = thi_sea_level * p_amb_sea_level / p_amb;
  double ffph = eData->e_af / (BURNING_AIR_PPS);
  double ffps = ffph / 3600;
  m_dot_fuel  = (EngStopped())?(0):(GALLON_FROM_PPH(ffps));
  // the const below is USG to pps  (JS: must be USG from pps)
  //  eData->e_gph = (float)(m_dot_fuel / 0.0016661); // pps
  eData->e_gph = GALLON_FROM_PPH(ffph);
  return;
}
//-------------------------------------------------------------------------
// Calculate the power produced by the engine.
// Inputs: ManifoldPressure_inHg, p_amb, p_amb_sea_level, RPM, T_amb,
//   equivalence_ratio, Cycles, MaxHP
//
// Outputs: Percentage_Power, HP
//-------------------------------------------------------------------------
double CPistonTRI1::GetEnginePower(void)
{ double HP = eData->e_hpaf - eData->e_fttb;
  Percentage_Power = HP * 100.0 / MaxHP;
  return HP;
}
//-------------------------------------------------------------------------
// Calculate the exhaust gas temperature.
//
// Inputs: equivalence_ratio, m_dot_fuel, calorific_value_fuel,
//   Cp_air, m_dot_air, Cp_fuel, m_dot_fuel, T_amb, Percentage_Power
//
// Outputs: combustion_efficiency, ExhaustGasTemp_degK
//-------------------------------------------------------------------------
void CPistonTRI1::doEGT(void)
{ double delta_T_exhaust;
  double enthalpy_exhaust;
  double heat_capacity_exhaust;
  double dEGTdt;
  float  dT = eData->dT;
  if ((EngPowered()) && (eData->e_af > 0.0)) {  // do the energy balance
    combustion_efficiency = GetCombustion_EfficiencyValue(equivalence_ratio);
    enthalpy_exhaust = m_dot_fuel * calorific_value_fuel *
                              combustion_efficiency * 0.33;
    heat_capacity_exhaust = (Cp_air * eData->e_af) + (Cp_fuel * m_dot_fuel);
    delta_T_exhaust = enthalpy_exhaust / heat_capacity_exhaust;
    ExhaustGasTemp_degK = T_amb + delta_T_exhaust;
    ExhaustGasTemp_degK *= 0.444 + ((0.544 - 0.444) * Percentage_Power / 100.0); // magic number 0.05
  } else {  // Drop towards ambient - guess an appropriate time constant for now
    dEGTdt = (298.0 - ExhaustGasTemp_degK) / 100.0;
    delta_T_exhaust = dEGTdt * dT;
    ExhaustGasTemp_degK += delta_T_exhaust;
  }

  eData->e_EGTr = KelvinToFahrenheit ((float)ExhaustGasTemp_degK) * Kegt;
}
//-------------------------------------------------------------------------
// Calculate the cylinder head temperature.
//
// Inputs: T_amb, IAS, rho_air, m_dot_fuel, calorific_value_fuel,
//   combustion_efficiency, RPM
//
// Outputs: CylinderHeadTemp_degK
//-------------------------------------------------------------------------
void CPistonTRI1::doCHT(void)
{ float dT = eData->dT;
  double h1 = -95.0;
  double h2 = -3.95;
  double h3 = -0.05;

  double arbitary_area = 1.0;
  double CpCylinderHead = 800.0;
  double MassCylinderHead = 8.0;

  double temperature_difference = CylinderHeadTemp_degK - T_amb;
  double v_apparent = eData->IAS * 0.5144444; // lc IAS or KIAS ?
  double v_dot_cooling_air = arbitary_area * v_apparent;
  double m_dot_cooling_air = v_dot_cooling_air * rho_air;
  double dqdt_from_combustion =
    m_dot_fuel * calorific_value_fuel * combustion_efficiency * 0.33;
  double dqdt_forced = (h2 * m_dot_cooling_air * temperature_difference) +
    (h3 * RPM * temperature_difference);
  double dqdt_free = h1 * temperature_difference;
  double dqdt_cylinder_head = dqdt_from_combustion + dqdt_forced + dqdt_free;

  double HeatCapacityCylinderHead = CpCylinderHead * MassCylinderHead;

  CylinderHeadTemp_degK += (dqdt_cylinder_head / HeatCapacityCylinderHead) * dT;

  eData->e_CHTr = DEGRE_FROM_KELVIN (float(CylinderHeadTemp_degK));
  return;
}
//-------------------------------------------------------------------------
//  Compute oil temperature
//  in KELVIN
//-------------------------------------------------------------------------
void CPistonTRI1::doOilTemperature(void)
{ float  dT = eData->dT;
  double idle_percentage_power = 2.3;        // approximately
  double target_oil_temp;                    // Steady state oil temp at the current engine conditions
  double time_constant;                      // The time constant for the differential equation

  if (EngPowered()) {
    target_oil_temp = 363;
    time_constant = 500;                     // Time constant for engine-on idling.
    if (Percentage_Power > idle_percentage_power) {
      time_constant /= ((Percentage_Power / idle_percentage_power) / 10.0); // adjust for power
    }
  } else {
    float atmf      = globals->atm->GetTemperatureF();
    target_oil_temp = KELVIN_FROM_FAHRENHEIT(atmf);
    time_constant = 1000;                    // Time constant for engine-off; reflects the fact
                                             // that oil is no longer getting circulated
  }

  double dOilTempdt = (target_oil_temp - OilTemp_degK) / time_constant;

  OilTemp_degK += (dOilTempdt * dT);

  eData->e_oilT = FAHRENHEIT_FROM_KELVIN(OilTemp_degK);
}
//-------------------------------------------------------------------------
// Calculate the oil pressure.
//  Oil Pressure is given by lookup table (from RPM);
//-------------------------------------------------------------------------
void CPistonTRI1::doOilPressure(void)
{   return;
}
//-------------------------------------------------------------------------
// Probe data
//-------------------------------------------------------------------------
void CPistonTRI1::Probe(CFuiCanva *cnv)
{ cnv->AddText(1,1,"MAP       =%.5f",  eData->e_hMap);
  cnv->AddText(1,1,"Air Flow   =%.5f", eData->e_af);
  cnv->AddText(1,1,"Fuel Flow  =%.5f", eData->e_gph);
  cnv->AddText(1,1,"Eng rawHP  =%.2f HP", eData->e_HP);
  cnv->AddText(1,1,"Eng remPWR =%.2f HP", FTLBS_TO_HP(eData->e_PWR));
  cnv->AddText(1,1,"Eng RPM    =%.0f", eData->e_rpm);
  cnv->AddText(1,1,"Eng Torque =%.5f", eData->e_Torque);
  cnv->AddText(1,1,"Eng Trust  =%.5f", eData->e_Thrust);
  return;
}
//==============================================================================
// CPiston model
//==============================================================================
CPiston::CPiston ( CPropellerModel *prop)
: R_air(287.3),
  rho_fuel(800),                 // estimate
  calorific_value_fuel(47.3e6),
  Cp_air(1005),
  Cp_fuel(1700)
{
  p_prop = prop;
  Percentage_Power = 0.0;

  ThrustPowerRequired = 0.0;  // 
  //
  OilTemp_degK = 298;
  MinManifoldPressure_inHg = 4.0; //6.5;//15.0;//6.5;   // 
  MaxManifoldPressure_inHg = 29.8;//42.0;//29.8;  // 
  MAP = globals->atm->GetPressureSlugsFtSec () * 47.88;  // psf to Pa.s
  //
  CylinderHeadTemp_degK = 0.0;
  Displacement = 360;
  MaxHP = 0;
  Cycles = 2;
  IdleRPM = 600;
  ExhaustGasTemp_degK = 0.0;
  EGT_degC = 0.0;

  /// \todo 
  minMAP = MinManifoldPressure_inHg * INHG_TO_PAS; // 3376.85;  // inHg to Pa.s
  maxMAP = MaxManifoldPressure_inHg * INHG_TO_PAS; // 3376.85;  // inHg to Pa.s
  ampMAP = maxMAP - minMAP;
  // Initialisation
  volumetric_efficiency = 0.8;  // Actually f(speed, load) but this will get us running

  // auto-start depending on INI file
  eng_auto_strt = 0;//0; 
  GetIniVar ("PHYSICS", "engineStarted", &eng_auto_strt);

  //
  ENGINE_THRUST_COEFF = 1.0;
  if (!globals->uph) { /// PHY file
    float tmp_eng_thrust_coeff = ADJ_ENGN_THRST; // 1.6f;
    GetIniFloat ("PHYSICS", "adjustEngineThrust", &tmp_eng_thrust_coeff);
    if (tmp_eng_thrust_coeff) ENGINE_THRUST_COEFF = static_cast<double> (tmp_eng_thrust_coeff);
    DEBUGLOG ("CPiston::CPiston thrust_coeff=%f", ENGINE_THRUST_COEFF);
  } else {
    ENGINE_THRUST_COEFF = static_cast<double> (globals->uph->Ktst);
    DEBUGLOG ("CPiston::CPiston PHY : thrust_coeff=%f", ENGINE_THRUST_COEFF);
  }
}
//--------------------------------------------------------------------------------
//  Destroy object
//--------------------------------------------------------------------------------
CPiston::~CPiston (void)
{}
//--------------------------------------------------------------------------------
//  Set Manifold presure limits
//--------------------------------------------------------------------------------
void CPiston::setMinMaxMAP (const double &min_inHg, const double &max_inHg)
{ minMAP = min_inHg * INHG_TO_PAS; // 3376.85;  // inHg to Pa.s
  maxMAP = max_inHg * INHG_TO_PAS; // 3376.85;  // inHg to Pa.s
  ampMAP = maxMAP - minMAP;
}
//-------------------------------------------------------------------------------
// Update RPM according to engine logical state.
//  NOTE: Only State transition are processed. RefreshState return 0, when
//       there is no state change
//-------------------------------------------------------------------------------
void CPiston::doEngineRefresh(void)
{ CEngine *meng = eData->meng;
  if (0 == meng->RefreshState())    return;
  //----Proceed according to new state -----
  char  state   = eData->e_state;
  switch (state) {
    //---Engine is just running -----------
    case ENGINE_CRANKING:
        RPM = 20;
        p_prop->SetRPM (RPM);
        return;

    case ENGINE_CATCHING:
        RPM = 200;
				eData->s_rpm = 190;
        p_prop->SetRPM (RPM);                     
        return;

    case ENGINE_RUNNING:
        RPM = 450;
				eData->s_rpm = 400;
        p_prop->SetRPM (RPM);                     
        return;

    case ENGINE_WINDMILL:
        RPM = 500; 
				eData->s_rpm = 400;
        p_prop->SetRPM (RPM);                     
        return;

    case ENGINE_STOPPING:
        RPM = 200;
				eData->s_rpm = 180;
        p_prop->SetRPM (RPM);                    
        return;

    case ENGINE_STOPPED:
      // Note for JS : I removed temporary this case
      // to fix the windmill case with no fuel
      // Yet it seems to cause no harm when stopping the engine
      // but I rather prefet to let you verify and adjust the 
      // switch sequence
			// JS: I put it back to synchronized the external view when
			//	propeller is stopping when the engine sound is dying.
			//	Otherwise RPM takes a long time to settle to 0
        RPM = 0;
        p_prop->SetRPM (RPM);     
        return;
  }
  return;
}
//-------------------------------------------------------------------------------
// Calculate the Current Boost Speed
//
// This function calculates the current turbo/supercharger boost speed
// based on altitude and the (automatic) boost-speed control valve configuration.
//
// Inputs: p_amb, BoostSwitchPressure, BoostSwitchHysteresis
//
// Outputs: BoostSpeed
//-------------------------------------------------------------------------------
void CPiston::doBoostControl(void)
{}
//-------------------------------------------------------------------------------
// Calculate the manifold absolute pressure (MAP) in inches hg
//
// This function calculates manifold absolute pressure (MAP)
// from the throttle position, turbo/supercharger boost control
// system, engine speed and local ambient air density.
//
// TODO: changes in MP should not be instantaneous -- introduce
// a lag between throttle changes and MP changes, to allow pressure
// to build up or disperse.
//
// Inputs: minMAP, maxMAP, p_amb, Throttle
//
// Outputs: MAP, ManifoldPressure_inHg
//-------------------------------------------------------------------------------
void CPiston::doMAP(void)
{}

//-------------------------------------------------------------------------------
// Calculate the air flow through the engine.
// Also calculates ambient air density
// (used in CHT calculation for air-cooled engines).
//
// Inputs: p_amb, R_air, T_amb, MAP, Displacement,
//   RPM, volumetric_efficiency
//
// TODO: Model inlet manifold air temperature.
//
// Outputs: rho_air, m_dot_air
//-------------------------------------------------------------------------------
void CPiston::doAirFlow(void)
{ rho_air = p_amb / (R_air * T_amb);
  double rho_air_manifold = MAP / (R_air * T_amb);
  double displacement_SI = Displacement * IN3_TO_M3;
  double swept_volume = (displacement_SI * (RPM/60)) / 2;
  double v_dot_air = swept_volume * volumetric_efficiency;
  m_dot_air = v_dot_air * rho_air_manifold;
  eData->e_af = m_dot_air;
  return;
}
//-------------------------------------------------------------------------------
// Calculate the fuel flow into the engine.
//
// Inputs: Mixture, thi_sea_level, p_amb_sea_level, p_amb, m_dot_air
//
// Outputs: equivalence_ratio, m_dot_fuel
//-------------------------------------------------------------------------------
void CPiston::doFuelFlow(void)
{
  double thi_sea_level = 1.3 * eData->e_mixt;
  equivalence_ratio = thi_sea_level * p_amb_sea_level / p_amb;
  double rf = eData->e_af / 14.7 * equivalence_ratio;
  m_dot_fuel = (EngStopped())?(0):(rf);
  eData->e_gph = (float)m_dot_fuel
    * 3600.0f			    // seconds to hours
    * 2.2046f   			// kg to lb
    / 6.6f;  			    // lb to gal_us of kerosene
  return;
}
//-------------------------------------------------------------------------------
// Calculate the power produced by the engine.
//
// Currently, the JSBSim propellor model does not allow the
// engine to produce enough RPMs to get up to a high horsepower.
// When tested with sufficient RPM, it has no trouble reaching
// 200HP.
//
// Inputs: ManifoldPressure_inHg, p_amb, p_amb_sea_level, RPM, T_amb,
//   equivalence_ratio, Cycles, MaxHP
//
// Outputs: Percentage_Power, HP
//-------------------------------------------------------------------------------
double CPiston::GetEnginePower(void)
{ return 0;}
//-------------------------------------------------------------------------------
// Calculate the exhaust gas temperature.
//-------------------------------------------------------------------------------
void CPiston::doEGT(void)
{}
//-------------------------------------------------------------------------------
// Calculate the cylinder head temperature.
//-------------------------------------------------------------------------------
void CPiston::doCHT(void)
{}
//-------------------------------------------------------------------------------
// Calculate the oil temperature.
//-------------------------------------------------------------------------------
void CPiston::doOilTemperature(void)
{}
//-------------------------------------------------------------------------------
// Calculate the oil pressure.
//-------------------------------------------------------------------------------
void CPiston::doOilPressure(void)
{ }
//-------------------------------------------------------------------------------
//  Mixture values
//-------------------------------------------------------------------------------
double  CPiston::GetPower_Mixture_CorrelationValue(const double &ratio)
{
  float x1 = 0.0f, y1 = 0.0f;
  float x2 = 0.0f, y2 = 0.0f;
  /* 14.7/1.6 */
  if        (ratio < 09.1875) {x1 = 09.1875f; x2 = 00.0000f; y1 = 78.00f; y2 = 00.00f;        
  } else if (ratio < 10.0000) {x1 = 10.0000f; x2 = 09.1875f; y1 = 86.00f; y2 = 78.00f;
  } else if (ratio < 11.0000) {x1 = 11.0000f; x2 = 10.0000f; y1 = 93.50f; y2 = 86.00f;
  } else if (ratio < 12.0000) {x1 = 12.0000f; x2 = 11.0000f; y1 = 98.00f; y2 = 93.50f;
  } else if (ratio < 13.0000) {x1 = 13.0000f; x2 = 12.0000f; y1 = 100.0f; y2 = 98.00f;
  } else if (ratio < 14.0000) {x1 = 14.0000f; x2 = 13.0000f; y1 = 99.00f; y2 = 100.0f;
  } else if (ratio < 15.0000) {x1 = 15.0000f; x2 = 14.0000f; y1 = 96.40f; y2 = 99.00f;
  } else if (ratio < 16.0000) {x1 = 16.0000f; x2 = 15.0000f; y1 = 92.50f; y2 = 96.40f;
  } else if (ratio < 17.0000) {x1 = 17.0000f; x2 = 16.0000f; y1 = 88.00f; y2 = 92.50f;
  } else if (ratio < 18.0000) {x1 = 18.0000f; x2 = 17.0000f; y1 = 83.00f; y2 = 88.00f;
  } else if (ratio < 19.0000) {x1 = 19.0000f; x2 = 18.0000f; y1 = 78.50f; y2 = 83.00f;
  } else if (ratio < 20.0000) {x1 = 20.0000f; x2 = 19.0000f; y1 = 74.00f; y2 = 78.50f;
  } else if (ratio < 24.5000) {x1 = 24.5000f; x2 = 20.0000f; y1 = 58.00f; y2 = 74.00f;
  }
  /* 14.7/0.6 */
  float P = (y1 - y2) / (x1 - x2);
  float K = y1 - (x1 * P);

  return (ratio * P + K);
}

//------------------------------------------------------------------------------
//  Return Combustion value
//------------------------------------------------------------------------------
double  CPiston::GetCombustion_EfficiencyValue(const double &ratio)
{
  float x1 = 0.0f, y1 = 0.0f;
  float x2 = 0.0f, y2 = 0.0f;
  //
  if        (ratio < 0.00) {x1 = 0.00f; x2 = 0.00f; y1 = 0.980f; y2 = 00.00f;        
  } else if (ratio < 0.90) {x1 = 0.90f; x2 = 0.00f; y1 = 0.980f; y2 = 0.980f;
  } else if (ratio < 1.00) {x1 = 1.00f; x2 = 0.90f; y1 = 0.970f; y2 = 0.980f;
  } else if (ratio < 1.05) {x1 = 1.05f; x2 = 1.00f; y1 = 0.950f; y2 = 0.970f;
  } else if (ratio < 1.10) {x1 = 1.10f; x2 = 1.05f; y1 = 0.900f; y2 = 0.950f;
  } else if (ratio < 1.15) {x1 = 1.15f; x2 = 1.10f; y1 = 0.850f; y2 = 0.900f;
  } else if (ratio < 1.20) {x1 = 1.20f; x2 = 1.15f; y1 = 0.790f; y2 = 0.850f;
  } else if (ratio < 1.30) {x1 = 1.30f; x2 = 1.20f; y1 = 0.700f; y2 = 0.790f;
  } else if (ratio < 1.40) {x1 = 1.40f; x2 = 1.30f; y1 = 0.630f; y2 = 0.700f;
  } else if (ratio < 1.50) {x1 = 1.50f; x2 = 1.40f; y1 = 0.570f; y2 = 0.630f;
  } else if (ratio < 1.60) {x1 = 1.60f; x2 = 1.50f; y1 = 0.525f; y2 = 0.570f;
  } else if (ratio < 2.00) {x1 = 2.00f; x2 = 1.60f; y1 = 0.345f; y2 = 0.525f;
  }
  //
  float P;
  if (x1 - x2)   P = (y1 - y2) / (x1 - x2);
  else  P = 0.0f;
  float K = y1 - (x1 * P);
  return (ratio * P + K);
}
//================================================================================
// CPistonEngineModel
//================================================================================
CPistonEngineModel::CPistonEngineModel (CVehicleObject *v,int eNo)
{ mveh  = v;                // Save parent vehicle
  uNum  = eNo;
  eNum  = eNo;
  //---Set ident ------------------------------------------------------
  Tag id = 'Eng0' | eNo;
  SetIdent(id);
  //--------------------------------------------------------------------
  boosted = false;
  boostAltitude = 0;
  boostPressure = 0;
  egts = 0.0f, chts = 0.0f, ffsc = 0.0f, idle = 0.0f;
  //---- propellor data -----------------------------------------------
  e_prop = NULL;
  //--- init table lookup ---------------------------------------------
  mphpaf  = 0;
  mpfttb  = 0;
  mpegta  = 0;
  mpchta  = 0;
  mpegtr  = 0;
  mpchtr  = 0;
  mpoilp  = 0;
  //----set default propeller model -----------------------------------
  /// models : TRI1 ... JSBSim (default TRI1)
  type   = PROP_TRI;
  piston = NULL;
}
//---------------------------------------------------------------------
//  destroy this object
//---------------------------------------------------------------------
CPistonEngineModel::~CPistonEngineModel (void)
{ SAFE_DELETE (e_prop);
  SAFE_DELETE (piston);
  SAFE_DELETE (mphpaf);
  SAFE_DELETE (mpfttb);
  SAFE_DELETE (mpegta);
  SAFE_DELETE (mpchta);
  SAFE_DELETE (mpegtr);
  SAFE_DELETE (mpchtr);
  SAFE_DELETE (mpoilp);
}
//---------------------------------------------------------------------
//  Read parameters
//  Note that a propeller is always created.
//---------------------------------------------------------------------
int CPistonEngineModel::Read (SStream *stream, Tag tag)
{ char model[16];
  int rc = TAG_IGNORED;

  switch (tag) {

  case 'modl':
    // default is TRI
    ReadString (model, 16, stream);
    if (0 == strcmp(model,"JSBSim")) type = PROP_JSB;
    return TAG_READ;
  
  case 'prop':
    if (type == PROP_JSB) e_prop = new CPropellerJSBSimModel(mveh,uNum);
    else                  e_prop = new CPropellerTRIModel(mveh,uNum);
    e_prop->SetEngineData(eData);
    ReadFrom (e_prop, stream);
    return TAG_READ;

  case 'bost':
    // Boost altitude/pressure
    boosted = true;
    ReadFloat (&boostAltitude, stream);
    ReadFloat (&boostPressure, stream);
    return TAG_READ;

  case 'egts':
    ReadFloat (&egts, stream);
    return TAG_READ;

  case 'chts':
    ReadFloat (&chts, stream);
    return TAG_READ;

  case 'ffsc':
    ReadFloat (&ffsc, stream);
    return TAG_READ;

  case 'idle':
    ReadFloat (&idle, stream);
    return TAG_READ;

  case 'hpaf':
    { CDataSearch map(stream);
      mphpaf = map.GetTable();
      return TAG_READ;
    }

  case 'fttb':
    { CDataSearch map(stream);
      mpfttb = map.GetTable();
      return TAG_READ;
    }

  case 'egta':
    { CDataSearch map(stream);
      mpegta = map.GetTable();
      return TAG_READ;
    }

  case 'chta':
    { CDataSearch map(stream);
      mpchta = map.GetTable();
      return TAG_READ;
    }

  case 'egtr':
    { CDataSearch map(stream);
      mpegtr = map.GetTable();
      return TAG_READ;
    }

  case 'chtr':
    { CDataSearch map(stream);
      mpchtr = map.GetTable();
      return TAG_READ;
    }

  case 'dRpm':
    SkipObject (stream);
    return TAG_READ;

  }
  
  // See if the tag can be processed by the parent class type
  return CEngineModel::Read (stream, tag);
}
//-----------------------------------------------------------------------
//  Read Finished
//  JS NOTE:  We dont need a Constant mixture parameter
//            By default mixt value in engine is 1.
//            Any mixture knob will overide the default value
//-----------------------------------------------------------------------
void CPistonEngineModel::ReadFinished (void)
{   //----Init windmill default (that may be overriden) --------
    eData->s_wml = 450;             // Start engine at this rpm
    eData->s_rpm = 400;             // Stalling rpm
    //----Create a propellor if none ---------------------------
    if (0 == e_prop)    e_prop = new CPropellerTRIModel(mveh,uNum);
    //----------------------------------------------------------
    eData->ffsc = ffsc;
    eData->idle = idle;
    eData->irpm = irpm;
    eData->rbhp = rbhp;
    eData->bost_alt = double(boostAltitude);
    eData->bost_pr  = double(boostPressure);
    if (type == PROP_JSB) piston  = new CPistonJSBSim (e_prop,eData);
    else                  piston  = new CPistonTRI1   (e_prop,eData);
    if (boosted) {
          piston->SetBoosted ();
          /// if boosted save the boost values as min and max MAP
          /// the min being 1/5 of the edited NGN max in <bost>
          piston->setMinMaxMAP (double(boostPressure) / 5.0, // 08.4
                                double(boostPressure)) ;     // 42.0
        }
    return;
}
//-----------------------------------------------------------------------
//  Return component corresponding to number
//-----------------------------------------------------------------------
CDependent *CPistonEngineModel::GetPart(char k)
{ if (k == 1)   return e_prop->GetPart();
  if (k == 2)   return piston->GetPart();
  return 0;
}
//-----------------------------------------------------------------------
//  Set constant mixture from phy file
//  if a mixt knob exists, it will override this feature
//-----------------------------------------------------------------------
void CPistonEngineModel::SetMixture()
{ eData->c_mixt = 0;
  if (!globals->uph)    return;
  eData->c_mixt = globals->uph->mixC;
  eData->e_mixt = eData->c_mixt;
  return;
}
//-----------------------------------------------------------------------
//  Get  RPM either from engine or from windmill
//  JS:  There should be a parameters to decide when engine must start
//       becuase it must depend on the engine type.
//-----------------------------------------------------------------------
void CPistonEngineModel::SetTrueRPM()
{ if (EngPowered())         return;  
  float wrpm = 0.0f;
  float vel = MetresToFeet (mveh->GetAirspeed ()->z);
  eData->e_vel = vel;
  //---Check for windmill ----------------------------------
  if (fabs(irpm) < FLT_EPSILON) irpm = GetIRPM ();
  if (fabs(vne)  < FLT_EPSILON) vne  = KtToFps (mveh->svh->GetVNE ());
  // rpm from windmill (basic physics)
  // to develop later if necessary
  //const float vne  = KtToFps (mveh->svh->GetVNE ());         ///< knts
  wrpm = (vne > 0.00001)?((irpm * vel) / vne):(0);
  /// simulate engine braking effect < 20 knts
  if (wrpm < 33.76f) wrpm = 0.0f;
  // Type is CPistonEngineModel
  float erpm   = eData->e_rpm;
  eData->e_wml = wrpm;                         // Set windmill rpm
  eData->e_rpm = max(wrpm,erpm);
  return;
 }
//-------------------------------------------------------------------------------
//  Time slice the model
//  JS:  Remove all logic to poll the starter, ingnitions and magneto
//       Now, Magneto sends messages to update startKit with magneto
//       states and starter state.  This save a  lot of polling message
//       and simplify the design because engine don't has to
//       know what type of starter is used.
//-------------------------------------------------------------------------------
void CPistonEngineModel::TimeSlice (float dT,U_INT FrNo)			// JSDEV*
{
  float blade_aoa = 0.0f;
  eData->e_rpm    = e_prop->GetERPM();
  blade_aoa       = float(e_prop->GetBladeAoA ());
  e_prop->SetBladePitch (eData->e_blad);

  // 2) prepare data to be sent to CPiston
  // process data if we are in a lookout tbl 
  if (mphpaf) eData->e_hpaf = mphpaf->Lookup (eData->e_af);
  // process data if we are in a lookout tbl 
  if (mpfttb) eData->e_fttb = mpfttb->Lookup (eData->e_rpm);
  // process data if we are in a lookout tbl 
  if (mpegta) eData->e_EGTa = mpegta->Lookup (blade_aoa);

  // process data if we are in a lookout tbl 
  if (mpegtr) eData->e_EGTr = mpegtr->Lookup (eData->e_rpm);

  // process data if we are in a lookout tbl 
  if (mpchta) eData->e_CHTa = mpchta->Lookup (blade_aoa);

  // process data if we are in a lookout tbl 
  if (mpchtr) eData->e_CHTr = mpchtr->Lookup (eData->e_rpm);

  // process data if we are in a lookout tbl 
  if (mpoilp) eData->e_oilP = mpoilp->Lookup (eData->e_rpm);

  // Fill data  ---------------------------
  eData->IAS    = mveh->GetPreCalculedKIAS (); // 100.0f // IAS in forward ft/s
  eData->dT     = dT;

  e_prop->TimeSlice (dT);
  piston->eCalculate(dT);

  // 2) compute all the values if engine is running

  if (EngStopped())  { 
    eData->e_gph   = 0.0f;                                             // fuel flow
    eData->e_rpm   = 0.0f;                                             // RPM
    eData->e_oilP  = 0.0f;                                             // oilP
    eData->e_EGTr  = 0.0f;                                             // EGT
    eData->e_CHTr  = 0.0f;                                             // CHT
  }
}
//-------------------------------------------------------------------------
// Add item to PLOT menu
//-------------------------------------------------------------------------
int CPistonEngineModel::AddToPlotMenu(char **menu, PLOT_PM *pm, int k)
{ if (k == PLOT_MENU_SIZE)  return k;
  char *item  = pm[k].menu;
  //--- Add air flow -------------------------------------
  _snprintf(item,PLOT_MENU_WIDTH,"%s air flow",GetIdString());
  menu[k]     = item;
  pm[k].iden  = unId;
  pm[k].type  = 'airf';
  k++;
  if (k == PLOT_MENU_SIZE)  return k;
   //--- Add MAP   --------------------------------------
  item  = pm[k].menu;
  _snprintf(item,PLOT_MENU_WIDTH,"%s MAP",GetIdString());
  menu[k]     = item;
  pm[k].iden  = unId;
  pm[k].type  = 'eMAP';
  k++;
  if (k == PLOT_MENU_SIZE)  return k;
  //--- Add HP ---------------------------------------------
  item  = pm[k].menu;
  _snprintf(item,PLOT_MENU_WIDTH,"%s raw HP",GetIdString());
  menu[k]     = item;
  pm[k].iden  = unId;
  pm[k].type  = 'hpwr';
  k++;
  if (k == PLOT_MENU_SIZE)  return k;
  //--- Add TORQUE   --------------------------------------
  item  = pm[k].menu;
  _snprintf(item,PLOT_MENU_WIDTH,"%s Torque",GetIdString());
  menu[k]     = item;
  pm[k].iden  = unId;
  pm[k].type  = 'torq';
  k++;
  if (k == PLOT_MENU_SIZE)  return k;
  //--- Add TRUST -----------------------------------------
  item  = pm[k].menu;
  _snprintf(item,PLOT_MENU_WIDTH,"%s Trust",GetIdString());
  menu[k]     = item;
  pm[k].iden  = unId;
  pm[k].type  = 'trus';
  k++;
  //--- Add Propeller lift --------------------------------
  item  = pm[k].menu;
  _snprintf(item,PLOT_MENU_WIDTH,"%s p-lift",GetIdString());
  menu[k]     = item;
  pm[k].iden  = unId;
  pm[k].type  = 'plft';
  k++;
  if (k == PLOT_MENU_SIZE)  return k;
  //--- Add Propeller drag --------------------------------
  item  = pm[k].menu;
  _snprintf(item,PLOT_MENU_WIDTH,"%s p-drag",GetIdString());
  menu[k]     = item;
  pm[k].iden  = unId;
  pm[k].type  = 'pdrg';
  k++;
  return k;}
//-------------------------------------------------------------------------
// Plot command
//-------------------------------------------------------------------------
bool CPistonEngineModel::PlotParameters(PLOT_PP *pp, Tag id, Tag type)
{ if (id != unId)   return false;
  switch(type)  {
    //--- engine power in HP ------------------------------
    case 'hpwr':
      { float hp   = rbhp * 2;               // Rated brake horse power
        pp->pvl    = &eData->e_HP;
        pp->dpnd   = this;
        pp->yUnit  = hp;
        pp->sign   = +1;
        strcpy(pp->mask,"raw HP(%.0f)");
        return true;
      }
    //--- Engine airflow ----------------------------------
    case 'airf':
      { float af   = 0.5f;
        pp->pvl    = &eData->e_af;
        pp->dpnd   = this;
        pp->yUnit  = af;
        pp->sign   = +1;
        strcpy(pp->mask,"air Flow(%.4f)");
        return true;
      }
    //--- Manifold pressure in inHg ----------------------
    case 'eMAP':
      { float map  = 30;
        pp->pvl    = &eData->e_hMap;
        pp->dpnd   = this;
        pp->yUnit  = map;
        pp->sign   = +1;
        strcpy(pp->mask,"MAP(%.1f) Hg");
        return true;
      }
    //--- TORQUE VALUE ---------------------------------
    case 'torq':
      { float trq  = 400;
        pp->pvl    = &eData->e_Torque;
        pp->dpnd   = this;
        pp->yUnit  = trq;
        pp->sign   = +1;
        strcpy(pp->mask,"TORQUE(%.0f)");
        return true;
      }
    //--- TRUST VALUE -----------------------------------
    case 'trus':
      { float trs  = 1200;
        pp->pvl    = &eData->e_Thrust;
        pp->dpnd   = this;
        pp->yUnit  = trs;
        pp->sign   = 1;
        strcpy(pp->mask,"TRUST(%.0f)"); 
        return true;
      }
    //--- Propeller LIFT -----------------------------------
    case 'plft':
      { float val  = 2;
        pp->pvl    = &eData->e_pLift;
        pp->dpnd   = this;
        pp->yUnit  = val;
        pp->sign   = +1;
        strcpy(pp->mask,"P-LIFT(%.1f)");
        return true;
      }
    //--- Propeller DRAG -----------------------------------
    case 'pdrg':
      { float val  = 1.5;
        pp->pvl    = &eData->e_pDrag;
        pp->dpnd   = this;
        pp->yUnit  = val;
        pp->sign   = +1;
        strcpy(pp->mask,"P-DRAG(%.1f)");
        return true;
      }

    }
  return false;
}
//-------------------------------------------------------------------------
// Plot HP versus airflow (static plot)
//-------------------------------------------------------------------------
void CPistonEngineModel::Plot(PLOT_PP &pp)
{ pp.val = (*pp.pvl) * pp.sign;
  return;
}
//============END OF FILE ======================================================

