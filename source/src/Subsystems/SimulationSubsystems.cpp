/*
 * SimulationSubsystems.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
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
#include "../Include/Globals.h"       // globals reference
#include "../Include/PistonEngine.h"
#include "../Include/Subsystems.h"
#include "../Include/Utility.h"       
#include "../Include/Atmosphere.h"
#include "../Include/Fui.h"
 
///////// DEBUG STUFF TO REMOVE LATER ////////////////////////////////
#ifdef  _DEBUG
#include "../Include/Fui.h"// ln 3489 
  //#define _DEBUG_PROP    //print lc DDEBUG file ... remove later
  //#define _DEBUG_MAP     //print lc DDEBUG file ... remove later
  //#define _DEBUG_EGT     //print lc DDEBUG file ... remove later
  //#define _DEBUG_OIL     //print lc DDEBUG file ... remove later
  //#define _DEBUG_ENG     //print lc DDEBUG file ... remove later
  //#define _DEBUG_FUEL    //print lc DDEBUG file ... remove later
  //#define MALIBU_TYPE    //                     ... remove later
#endif
//////////////////////////////////////////////////////////////////////

using namespace std;

//
// CAirModel
//
CAirModel::CAirModel (void)
{
  TypeIs (SUBSYSTEM_AIR_MODEL);
}


//
// CMixerModel
//
CMixerModel::CMixerModel (void)
{
  TypeIs (SUBSYSTEM_MIXER_MODEL);
}


//
// CWheelModel
//
CWheelModel::CWheelModel (void)
{
  TypeIs (SUBSYSTEM_WHEEL_MODEL);
}


//-------------------------------------------------------------------------------
// CEngineModel
//-------------------------------------------------------------------------------
CEngineModel::CEngineModel (void)
{
  TypeIs (SUBSYSTEM_ENGINE_MODEL);

  // engine common values
  vne   = 0;
  irpm  = 0.0f, srpm = 0.0f;
  rbhp  = 0.0f, rrpm = 0.0f;
  nGt_  = 0.0f;
  cowT  = 0.0f, cowD = 0.0f;
}

CEngineModel::~CEngineModel (void)
{
}
//-------------------------------------------------------------------------------
// Read all parameters
//-------------------------------------------------------------------------------
int CEngineModel::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'irpm':
    ReadFloat (&irpm, stream);
    rc = TAG_READ;
    break;
  case 'srpm':
    ReadFloat (&srpm, stream);
    rc = TAG_READ;
    break;
  case 'rbhp':
    ReadFloat (&rbhp, stream);
    rc = TAG_READ;
    break;
  case 'rrpm':
    ReadFloat (&rrpm, stream);
    rc = TAG_READ;
    break;
  case 'cowT':
    ReadFloat (&cowT, stream);
    rc = TAG_READ;
    break;
  case 'cowD':
    ReadFloat (&cowD, stream);
    rc = TAG_READ;
    break;
  case 'nGt_':
    ReadFloat (&nGt_, stream);
    rc = TAG_READ;
    break;
  case 'oilp':
    { CDataSearch map(stream);
      mpoilp = map.GetTable();
      return  TAG_READ;
    }
  case 'oilT':
    { CDataSearch map(stream);
      mpoilt = map.GetTable();
      return TAG_READ;
    }
  case 'dmge':
    SkipObject (stream);
    rc = TAG_READ;
    break;
  case 'fuel':
    SkipObject (stream);
    rc = TAG_READ;
    break;
  default:
    rc = TAG_IGNORED;
    break;
  }

  // If tag has not been processed, pass it to the parent
  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CSubsystem::Read (stream, tag);
  }

  return rc;
}


//======================================================================================
//
//  JSB SIM piston
//
//======================================================================================
/*!
 * Calculate the oil temperature.
 *
 * Inputs: Percentage_Power, running flag.
 *
 * Outputs: OilTemp_degK
 */


//
// CPistonJSBSim model
//
CPistonJSBSim::CPistonJSBSim (CVehicleObject *v,CPropellerModel *prop,CEngineData *ed)
: CPiston (v,prop)
{

}

CPistonJSBSim::~CPistonJSBSim (void)
{

}
//--------------------------------------------------------------------------------------
//  Compute engine data
//  JS NOTE:  Removing calls to DoEGT, DoCHT and doOilPressure because all planes include
//            table lookup depending on RPM for those data.  
//            Those tables are defined in NGN file and should still
//            be valid if RPM are OK.
//--------------------------------------------------------------------------------------
void CPistonJSBSim::eCalculate(float dT)
{ CEngine *meng = eData->meng;
  return;
}

/*!
 * Calculate the Current Boost Speed
 *
 * This function calculates the current turbo/supercharger boost speed
 * based on altitude and the (automatic) boost-speed control valve configuration.
 *
 * Inputs: p_amb, BoostSwitchPressure, BoostSwitchHysteresis
 *
 * Outputs: BoostSpeed
 */

void CPistonJSBSim::doBoostControl(void)
{
}

/*!
 * Calculate the manifold absolute pressure (MAP) in inches hg
 *
 * This function calculates manifold absolute pressure (MAP)
 * from the throttle position, turbo/supercharger boost control
 * system, engine speed and local ambient air density.
 *
 * TODO: changes in MP should not be instantaneous -- introduce
 * a lag between throttle changes and MP changes, to allow pressure
 * to build up or disperse.
 *
 * Inputs: minMAP, maxMAP, p_amb, Throttle
 *
 * Outputs: MAP, ManifoldPressure_inHg
 */

void CPistonJSBSim::doMAP(void)
{ }

/*!
 * Calculate the power produced by the engine.
 *
 * Currently, the JSBSim propellor model does not allow the
 * engine to produce enough RPMs to get up to a high horsepower.
 * When tested with sufficient RPM, it has no trouble reaching
 * 200HP.
 *
 * Inputs: ManifoldPressure_inHg, p_amb, p_amb_sea_level, RPM, T_amb,
 *   equivalence_ratio, Cycles, MaxHP
 *
 * Outputs: Percentage_Power, HP
 */

double CPistonJSBSim::GetEnginePower(void)
{ return 0;}

/*!
 * Calculate the exhaust gas temperature.
 *
 * Inputs: equivalence_ratio, m_dot_fuel, calorific_value_fuel,
 *   Cp_air, m_dot_air, Cp_fuel, m_dot_fuel, T_amb, Percentage_Power
 *
 * Outputs: combustion_efficiency, ExhaustGasTemp_degK
 */

void CPistonJSBSim::doEGT(void)
{ float  dT = eData->dT;

}

/*!
 * Calculate the cylinder head temperature.
 *
 * Inputs: T_amb, IAS, rho_air, m_dot_fuel, calorific_value_fuel,
 *   combustion_efficiency, RPM
 *
 * Outputs: CylinderHeadTemp_degK
 */

void CPistonJSBSim::doCHT(void)
{ float  dT = eData->dT;

}

/*!
 * Calculate the oil temperature.
 *
 * Inputs: Percentage_Power, running flag.
 *
 * Outputs: OilTemp_degK
 */

void CPistonJSBSim::doOilTemperature(void)
{ float dT = eData->dT;

}

/*!
 * Calculate the oil pressure.
 *
 * Inputs: RPM
 *
 * Outputs: OilPressure_psi
 */

void CPistonJSBSim::doOilPressure(void)
{

}


//============================================================================
// CTurbopropEngineModel
//============================================================================
CTurbopropEngineModel::CTurbopropEngineModel (CVehicleObject *v,int eNo)
{ mveh = v;
  eNum = eNo;
  prop = NULL;

  runupTimeK = rundownTimeK = starterTimeK = spoolTimeK = 0;
  betaRange = betaBrake = betaHigh = betaMid = betaLow = 0;
  designRPM = designInletTemp = designInletMach = designInletPressure = 0;
  conditionerLow = conditionerHigh = 0;
  lowIdleN1 = highIdleN1 = lowMaxN1 = highMaxN1 = 0;
  governorGain = governorLimit = 0;
  designCombustionTemp = designHeatingValue = designAirMassFlow = 0;
  designCompressorRatio = designDiffuserRatio = designBurnerRatio = 0;
  designNozzlePressureRatio = designHPTPressureRatio = designHPTTempRatio = 0;
  compSHR = turbSHR = 0;
  compSHRConstPressure = turbSHRConstPressure = 0;
  compEff = turbEff = mechEff = burnEff = 0;
}

CTurbopropEngineModel::~CTurbopropEngineModel (void)
{
}

int CTurbopropEngineModel::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'prop':
    // Propeller object
    prop = new CPropellerModel;
    ReadFrom (prop, stream);
    rc = TAG_READ;
    break;
  case 'ruTk':
    // Run-up time constant
    ReadFloat (&runupTimeK, stream);
    rc = TAG_READ;
    break;
  case 'rdTk':
    // Run-down time constant
    ReadFloat (&rundownTimeK, stream);
    rc = TAG_READ;
    break;
  case 'stTk':
    // Starter time constant
    ReadFloat (&starterTimeK, stream);
    rc = TAG_READ;
    break;
  case 'spTk':
    // Spool time constant
    ReadFloat (&spoolTimeK, stream);
    rc = TAG_READ;
    break;
  case 'beta':
    // Beta range
    ReadFloat (&betaRange, stream);
    rc = TAG_READ;
    break;
  case 'btaP':
    // Beta brake (hp)
    ReadFloat (&betaBrake, stream);
    rc = TAG_READ;
    break;
  case 'btHi':
    // Maximum blade angle (beta range forward) (degrees)
    ReadFloat (&betaHigh, stream);
    rc = TAG_READ;
    break;
  case 'btMd':
    // Mid blade angle (beta range zero) (degrees)
    ReadFloat (&betaMid, stream);
    rc = TAG_READ;
    break;
  case 'btLo':
    // Minimum blade angle (beta range reverse) (degrees)
    ReadFloat (&betaLow, stream);
    rc = TAG_READ;
    break;
  case 'dRPM':
    // Design RPM
    ReadFloat (&designRPM, stream);
    rc = TAG_READ;
    break;
  case '_T0D':
    // Design inlet temperature @ 20000'
    ReadFloat (&designInletTemp, stream);
    rc = TAG_READ;
    break;
  case '_M0D':
    // Design inlet Mach number @ 20000'
    ReadFloat (&designInletMach, stream);
    rc = TAG_READ;
    break;
  case '_P0D':
    // Design inlet pressure @ 20000'
    ReadFloat (&designInletPressure, stream);
    rc = TAG_READ;
    break;
  case 'pcLo':
    // Power conditioner (low)
    ReadFloat (&conditionerLow, stream);
    rc = TAG_READ;
    break;
  case 'pcHi':
    // Power conditioner (high)
    ReadFloat (&conditionerHigh, stream);
    rc = TAG_READ;
    break;
  case 'minL':
    // Low idle N1
    ReadFloat (&lowIdleN1, stream);
    rc = TAG_READ;
    break;
  case 'minH':
    // High idle N1
    ReadFloat (&highIdleN1, stream);
    rc = TAG_READ;
    break;
  case 'maxL':
    // Low max N1
    ReadFloat (&lowMaxN1, stream);
    rc = TAG_READ;
    break;
  case 'maxH':
    // High max N1
    ReadFloat (&highMaxN1, stream);
    rc = TAG_READ;
    break;
  case 'govK':
    // Governor gain
    ReadFloat (&governorGain, stream);
    rc = TAG_READ;
    break;
  case 'govL':
    // Governor limit
    ReadFloat (&governorLimit, stream);
    rc = TAG_READ;
    break;
  case 'Tt4D':
    // Design combustion temperature
    ReadFloat (&designCombustionTemp, stream);
    rc = TAG_READ;
    break;
  case 'HprD':
    // Design heating value of fuel
    ReadFloat (&designHeatingValue, stream);
    rc = TAG_READ;
    break;
  case 'amfD':
    // Design air mass flow
    ReadFloat (&designAirMassFlow, stream);
    rc = TAG_READ;
    break;
  case '_PcD':
    // Design compressor pressure ratio
    ReadFloat (&designCompressorRatio, stream);
    rc = TAG_READ;
    break;
  case '_PdD':
    // Design diffuser pressure ratio
    ReadFloat (&designDiffuserRatio, stream);
    rc = TAG_READ;
    break;
  case '_PbD':
    // Design burner pressure ratio
    ReadFloat (&designBurnerRatio, stream);
    rc = TAG_READ;
    break;
  case '_PnD':
    // Design nozzle pressure ratio
    ReadFloat (&designNozzlePressureRatio, stream);
    rc = TAG_READ;
    break;
  case 'PtHD':
    // Design high pressure turbine pressure ratio
    ReadFloat (&designHPTPressureRatio, stream);
    rc = TAG_READ;
    break;
  case 'TtHD':
    // Design high pressure turbine temperature ratio
    ReadFloat (&designHPTTempRatio, stream);
    rc = TAG_READ;
    break;
  case 'cSHR':
    // Compressor specific heat ratio
    ReadFloat (&compSHR, stream);
    rc = TAG_READ;
    break;
  case 'tSHR':
    // Turbine specific heat ratio
    ReadFloat (&turbSHR, stream);
    rc = TAG_READ;
    break;
  case 'cshp':
    // Compressor specific heat at constant pressure
    ReadFloat (&compSHRConstPressure, stream);
    rc = TAG_READ;
    break;
  case 'tshp':
    // Turbine specific heat ratio
    ReadFloat (&turbSHRConstPressure, stream);
    rc = TAG_READ;
    break;
  case 'cEff':
    // Compressor efficiency
    ReadFloat (&compEff, stream);
    rc = TAG_READ;
    break;
  case 'tEff':
    // Turbine efficiency
    ReadFloat (&turbEff, stream);
    rc = TAG_READ;
    break;
  case 'mEff':
    // Mechanical efficiency
    ReadFloat (&mechEff, stream);
    rc = TAG_READ;
    break;
  case 'bEff':
    // Burner efficiency
    ReadFloat (&burnEff, stream);
    rc = TAG_READ;
    break;
  }

  // If tag has not been processed, pass it to the parent
  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CEngineModel::Read (stream, tag);
  }

  return rc;
}

void CTurbopropEngineModel::ReadFinished (void)
{
}

void CTurbopropEngineModel::TimeSlice (float dT,U_INT FrNo)			// JSDEV*
{
  #ifdef _DEBUG_	
	  FILE *fp_debug;
	  if((fp_debug = fopen("__DDEBUG_.txt", "a")) != NULL)
	  {
		  int test = 0;
		  fprintf(fp_debug, "CTurbopropEngineModel::TimeSlice %f\n", irpm);
		  fclose(fp_debug); 
	  }
  #endif
}

//==============================================================================
// CTurbofanEngineModel
//==============================================================================
CTurbofanEngineModel::CTurbofanEngineModel (CVehicleObject *v,int eNo)
{ mveh  = v;
  eNum  = eNo;
  read_file_flag = true;
}

CTurbofanEngineModel::~CTurbofanEngineModel (void)
{
}

int CTurbofanEngineModel::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  // If tag has not been processed, pass it to the parent
  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CEngineModel::Read (stream, tag);
  }

  return rc;
}

void CTurbofanEngineModel::ReadFinished (void)
{

}

void CTurbofanEngineModel::TimeSlice (float dT,U_INT FrNo)		// JSDEV*
{
  #ifdef _DEBUG_	
	  FILE *fp_debug;
	  if((fp_debug = fopen("__DDEBUG_.txt", "a")) != NULL)
	  {
		  int test = 0;
		  fprintf(fp_debug, "CTurbofanEngineModel::TimeSlice %f\n", irpm);
		  fclose(fp_debug); 
	  }
  #endif

}

//===========================================================================
// CTurbineEngineModel
//============================================================================
CTurbineEngineModel::CTurbineEngineModel (CVehicleObject *v,int eNo)
{ mveh = v;
  eNum = eNo;
  read_file_flag = true;
}

CTurbineEngineModel::~CTurbineEngineModel (void)
{
}

int CTurbineEngineModel::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  // If tag has not been processed, pass it to the parent
  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CEngineModel::Read (stream, tag);
  }

  return rc;
}

void CTurbineEngineModel::ReadFinished (void)
{

}

void CTurbineEngineModel::TimeSlice (float dT,U_INT FrNo)		// JSDEV*
{
  #ifdef _DEBUG_
	  FILE *fp_debug;
	  if((fp_debug = fopen("__DDEBUG_.txt", "a")) != NULL)
	  {
		  int test = 0;
		  fprintf(fp_debug, "CTurbineEngineModel::TimeSlice %f\n", irpm);
		  fclose(fp_debug); 
	  }
  #endif
}

//========================================================================
// CTurboshaftAPUEngineModel
//========================================================================
CTurboshaftAPUEngineModel::CTurboshaftAPUEngineModel (CVehicleObject *v,int eNo)
{ mveh = v;
  eNum = eNo;
  read_file_flag = true;
}

CTurboshaftAPUEngineModel::~CTurboshaftAPUEngineModel (void)
{
}

int CTurboshaftAPUEngineModel::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  // If tag has not been processed, pass it to the parent
  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CEngineModel::Read (stream, tag);
  }

  return rc;
}

void CTurboshaftAPUEngineModel::ReadFinished (void)
{

}

void CTurboshaftAPUEngineModel::TimeSlice (float dT,U_INT FrNo)		// JSDEV*
{
  #ifdef _DEBUG_	
	  FILE *fp_debug;
	  if((fp_debug = fopen("__DDEBUG_.txt", "a")) != NULL)
	  {
		  int test = 0;
		  fprintf(fp_debug, "CTurboshaftAPUEngineModel::TimeSlice %f\n", irpm);
		  fclose(fp_debug); 
	  }
  #endif
}

//===================================================================
// CSimplisticJetEngineModel
//====================================================================
CSimplisticJetEngineModel::CSimplisticJetEngineModel (CVehicleObject *v, int eNo)
{ mveh = v;
  eNum = eNo;
  read_file_flag = true;
}

CSimplisticJetEngineModel::~CSimplisticJetEngineModel (void)
{
}

int CSimplisticJetEngineModel::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  // If tag has not been processed, pass it to the parent
  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CEngineModel::Read (stream, tag);
  }

  return rc;
}

void CSimplisticJetEngineModel::ReadFinished (void)
{

}

void CSimplisticJetEngineModel::TimeSlice (float dT,U_INT FrNo)		// JSDEV*
{
  #ifdef _DEBUG_	
	  FILE *fp_debug;
	  if((fp_debug = fopen("__DDEBUG_.txt", "a")) != NULL)
	  {
		  int test = 0;
		  fprintf(fp_debug, "CSimplisticJetEngineModel::TimeSlice %f\n", irpm);
		  fclose(fp_debug); 
	  }
  #endif
}


//
// CWingModel
//
CWingModel::CWingModel (void)
{
  TypeIs (SUBSYSTEM_WING_MODEL);
}

//==================================================================
//
//
// CPropellerModel
//
//===================================================================

CPropellerModel::CPropellerModel (void)
{*propeller_filename = 0;
  read_PRP_file_flag = false;
  //
  mode = 0,    blad = 2;
  cord = 0.0f, diam = 0.0f;
  mnbd = 0.0f, mxbd = 0.0f;
  facT = 1.0f, facQ = 1.0f; 
  iceR = 0.0f, idcf = 0.0f;
  pFac = 1;
  eRPM = 0;
  //

  RPM = 0.0;
  Thrust = PowerRequired = Torque = 0.0;
  blade_AoA = 0.0;
  minRPM = 0.0;
  thrust_displ = 0.0f;
  is_fixed_speed_prop = false;
	pdmge = 0;
}

CPropellerModel::~CPropellerModel (void)
{ SAFE_DELETE (mlift);
  SAFE_DELETE (mdrag);
  SAFE_DELETE (mpirt);
  SAFE_DELETE (mrift);
  SAFE_DELETE (mfacP);

  SAFE_DELETE (pdmge);
}
//---------------------------------------------------------------------------
//  Reset computed values
//---------------------------------------------------------------------------
void CPropellerModel::Reset()
{ lift = 0.0f, drag = 0.0f, pirt = 0.0f, rift = 0.0f, dmge = 0.0f;
  pdmge = NULL;
	return;
}
//---------------------------------------------------------------------------
//  Read Propeller parameters
//---------------------------------------------------------------------------
int CPropellerModel::Read (SStream *stream, Tag tag)
{ float pm;
  int rc = TAG_IGNORED;

  switch (tag) {

  case 'prpf':
    ReadString (propeller_filename, PATH_MAX, stream);
    if (strlen (propeller_filename) != 0) read_PRP_file_flag = true;
    return TAG_READ;

  case 'mode':
    ReadInt (&mode, stream);
    return TAG_READ;

  case 'blad':
    ReadInt (&blad, stream);
    return TAG_READ;

  case 'cord':
    ReadFloat (&cord, stream);
    return TAG_READ;

  case 'diam':
    ReadFloat (&diam, stream);
    return TAG_READ;

  case 'mnbd':
    ReadFloat (&mnbd, stream);
    return TAG_READ;

  case 'mxbd':
    ReadFloat (&mxbd, stream);
    return TAG_READ;

  case 'facT':
    ReadFloat (&facT, stream);
    return TAG_READ;

  case 'facQ':
    ReadFloat (&facQ, stream);
    return TAG_READ;

  case 'pFac':
    ReadFloat (&pm, stream);
    return TAG_READ;

  case 'qFac':
    ReadFloat (&pFac, stream);
    return TAG_READ;

  case 'gRAT':
    ReadDouble(&GearRatio,stream);
    return TAG_READ;

  case 'pINR':
    ReadDouble(&Ixx,stream);
    return TAG_READ;

  case 'iceR':
    ReadFloat (&iceR, stream);
    return TAG_READ;

  case 'idcf':
    ReadFloat (&idcf, stream);
    return TAG_READ;

  // lift, drag, pirt, rift, dmge
  case 'lift':
    { CDataSearch map(stream);
      mlift = map.GetTable();
      return TAG_READ;
    }
  case 'drag':
    { CDataSearch map(stream);
      mdrag = map.GetTable();
      return TAG_READ;
    }

  case 'pirt':
    { CDataSearch map(stream);
      mpirt = map.GetTable();
      return TAG_READ;
    }

  case 'rift':
    { CDataSearch map(stream);
      mrift = map.GetTable();
      return TAG_READ;
    }

  case 'dmge':
    pdmge = new CDamageModel;
    ReadFrom (pdmge, stream);
    return TAG_READ;
  }

  // If tag has not been processed, pass it to the parent
  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CEngineSubsystem::Read (stream, tag);
  }

  return rc;
}

void CPropellerModel::ReadFinished (void)
{
  if (read_PRP_file_flag) {
    read_PRP_file_flag = false;
    SStream s(this,"WORLD",propeller_filename);
    return;
  }
  //
  // create a P fact table
  // independent variabe = blade AoA
  // dependent variable  = thrust offset X position
 
  mfacP = new CFmt1Map();
  mfacP->Add (-3.14f, 00.0f);
  mfacP->Add ( 0.10f, 00.0f);
  mfacP->Add ( 3.14f, 20.0f);

  CEngineSubsystem::ReadFinished ();
}
//----------------------------------------------------------------------
//  Time slice propeller
//----------------------------------------------------------------------
void CPropellerModel::TimeSlice (float dT)
{ deltaT = dT;
  // Get current user vehicle orientation
  // compute propeller lift//AoA
  // process data if we are in a lookout tbl 
  // calculate blade AoA
/*
  blade_AoA = DegToRad (Pitch) - Advance;
  if (mlift) lift = mlift->Lookup (blade_AoA);
  eData->e_pLift  = lift;
  // compute propeller drag//AoA
  if (mdrag) drag = mdrag->Lookup (blade_AoA);
  eData->e_pDrag  = drag;
*/
  return;
}

//==============================================================================
//
//
//  CPropellerJSBSimModel
//
//
//===============================================================================
CPropellerJSBSimModel::CPropellerJSBSimModel (CVehicleObject *v, int eNo)
{

}

CPropellerJSBSimModel::~CPropellerJSBSimModel (void)
{

}

double CPropellerJSBSimModel::GetPowerRequired(void)
{
  return PowerRequired;
}


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//
// We must be getting the aerodynamic velocity here, NOT the inertial velocity.
// We need the velocity with respect to the wind.
//
// Note that PowerAvailable is the excess power available after the drag of the
// propeller has been subtracted. At equilibrium, PowerAvailable will be zero -
// indicating that the propeller will not accelerate or decelerate.
// Remembering that Torque * omega = Power, we can derive the torque on the
// propeller and its acceleration to give a new RPM. The current RPM will be
// used to calculate thrust.
//
// Because RPM could be zero, we need to be creative about what RPM is stated as.

double CPropellerJSBSimModel::Calculate(double PowerAvailable)
{
  return Thrust; // return thrust in pounds
}
//==============================================================================
/*!
 *
 *  CPropellerTRIModel
 *
 */
//==============================================================================
CPropellerTRIModel::CPropellerTRIModel (CVehicleObject *v,int eNo)
{ mveh = v; // Save parent vehicle
  //-- compute propeller Idd -----------
  TypeIs(SUBSYSTEM_ENGINE_PROP);
  Tag id      = 'Epp0' | eNo;
  hwId        = HW_PROP;
  SetIdent(id);
  uNum        = eNo;
  Sense       = 1.0;
  Pitch       = Advance = 0.0;
  MaxRPM      = 2700.0;
  MinRPM      = 700.0;
  GearRatio   = ADJ_GEAR_RATIO; // 0.15;
  Advance     = RadToDeg (mnbd);
  RPM         = 0.0;
  Ixx         = ADJ_IXX_COEFF; // 1.76;//5.0
  Thrust      = ThrustCoeff = PowerRequired = 0.0;
  blade_speed = TotalDrag = 0.0;
  PfcK        = 1.0;
  tmp_lift    = 0.0f;
  tmp_drag    = 0.0f;
  magic_number= 100.0;
  //--- Init PHY coefficients ---------------------
	CPhysicModelAdj *phy = mveh->GetPHY();
  if (phy) {
    magic_number = phy->Kpmn;
#ifdef _DEBUG
    DEBUGLOG ("PHY : Kpmn=%f", magic_number);
#endif
  } else {
    magic_number = 100.0f;
#ifdef _DEBUG
    DEBUGLOG ("    : Kpmn=%f", magic_number);
#endif
  }
  //---Read values from PHY file ---------------------------------------
  if (phy) PfcK = double(phy->KfcP); /// PHY file
    
#ifdef _DEBUG
  DEBUGLOG ("CPropellerTRIModel::CPropellerTRIModel PfacK=%.2f", PfcK);
#endif
}
//---------------------------------------------------------------------
//  Destroy model
//---------------------------------------------------------------------
CPropellerTRIModel::~CPropellerTRIModel (void)
{
}
//---------------------------------------------------------------------
//  All parameters are read
//  JS NOTE:  As all prop plane must have a gear Ratio and a propeller 
//  inertia, I have add two tags to each prp files
//  tag <gRAT>  for defining gear ratio
//  tag <pINR>  for defining prop inertia.
//  Optionnal values found in the PHY file are now coefficients rather than
//  the final value
//---------------------------------------------------------------------
void  CPropellerTRIModel::ReadFinished (void)
{ CPhysicModelAdj *phy = mveh->GetPHY();
	if (0 == phy)    return;
  Ixx       *= phy->Kixx;
  GearRatio *= phy->KgrR; 
  #ifdef _DEBUG
    DEBUGLOG ("PHY : prop inertia = %f gear ratio = %f", Ixx, GearRatio);
  #endif
  CPropellerModel::ReadFinished ();
}
//---------------------------------------------------------------------------
// Retrieves the power required (or "absorbed") by the propeller -
//    i.e. the power required to keep spinning the propeller at the current
//    velocity, air density,  and rotational rate. 
//----------------------------------------------------------------------------
double CPropellerTRIModel::GetPowerRequired(void)
{ double RPS  = RPM / 60.0;
  blade_speed = double(diam * PI) * RPS;
  if (eRPM <= 0.80) return 0.0; // If the prop ain't turnin', the fuel ain't burnin'.
	double minR		= eData->e_minRv;				// Minimum Reev
	double maxR		= eData->e_maxRv;				// Maximum Reev
	double bladP	= eData->e_blad;				// Blad Picth
  //double Vel  = MetresToFeet ((mveh->GetRelativeBodyAirspeed ())->z); //GetAirspeed ())->z /*m/s LH*/);
  double Vel  = MetresToFeet ((mveh->GetAirspeed ())->z /*m/s LH*/);

  // calculate AAR rift and pirt
  // here a potential bug with RPS :: 
  double AAR = (RPS != 0)?(Vel / (diam * RPS)):(0); // Advance ratio angle
  if (mrift) rift = mrift->Lookup (float(AAR));
  if (mpirt) pirt = mpirt->Lookup (float(AAR));
  // had to use a magic number to stay close to Fly!II
  double magic_number = DegToRad (1.5);
	//-- JS: Avoid Float error when blad_speed is zero --------
	double beta =(blade_speed > 0)?(atan (Vel / blade_speed)):(0);
	Advance = beta + magic_number; // in rad 
 // Advance = atan (Vel / blade_speed) + magic_number; // in rad 
  
  // JS NOTE:  Blade pitch must be in [mnbd,mxbd] and just depend on the blade lever
  if (is_fixed_speed_prop) { // Fixed pitch prop
    Pitch = double(mnbd);
  } else {                                           // Variable pitch prop
    if (maxR != minR) {        // fixed-speed prop
      double rpmReq = minR +  ((maxR - minR) * bladP);
      double dRPM = rpmReq - RPM;
      Pitch -= dRPM / 10000.0; // 25.0;
      if (Pitch < double(mnbd))	Pitch = double(mnbd);
      if (Pitch > double(mxbd)) Pitch = double(mxbd);
    } else {
      Pitch = ((mxbd - mnbd) * bladP) + mnbd;   
    }
  }
	//--- Temporary ---------------------------------------
	if (0 == mlift)	
		gtfo("mlift");
  //--- Calculate blade AoA
  blade_AoA = DegToRad (Pitch) - Advance;
  if (mlift) lift = mlift->Lookup (float(blade_AoA));// 
  if (mdrag) drag = mdrag->Lookup (float(blade_AoA));// 
  eData->e_pLift  = lift;
  eData->e_pDrag  = drag;
  // PowerRequired = TotalDrag; // 
  // Remembering that Torque * omega = Power, we can derive the torque on the
  // propeller and its acceleration to give a new RPM. The current RPM will be
  // used to calculate thrust.
  PowerRequired = Torque * RPS * static_cast<double> (TWO_PI);

  //--- JS: Sometime the PowerRequired is negative. Is that allowed? 
	if (PowerRequired < 0) 	PowerRequired = 0;			//JS TRY this
  return PowerRequired; //0.0;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//
// We must be getting the aerodynamic velocity here, NOT the inertial velocity.
// We need the velocity with respect to the wind.
//
// Note that PowerAvailable is the excess power available after the drag of the
// propeller has been subtracted. At equilibrium, PowerAvailable will be zero -
// indicating that the propeller will not accelerate or decelerate.
// Remembering that Torque * omega = Power, we can derive the torque on the
// propeller and its acceleration to give a new RPM. The current RPM will be
// used to calculate thrust.
//
// Because RPM could be zero, we need to be creative about what RPM is stated as.
//===============================================================================
double CPropellerTRIModel::Calculate(double PowerAvailable)
{
  PwrAva = PowerAvailable;
  double omega = 0.0;
  const SVector *v = mveh->GetRelativeBodyAirspeed (); //GetAirspeed();                   ///< m/s LH
  //const SVector *v = mveh->GetAirspeed();                   ///< m/s LH
  double Vel = MetresToFeet (v->z);
  double rho = globals->atm->GetDensitySlugsFt3 () ;        //0.003277; // slug/cu.f
  double dynamic_pressure = rho * ((blade_speed * blade_speed) + (Vel * Vel)) * 0.5;
  double solidity = cord * diam * 0.5 * blad;
  double dyso = dynamic_pressure * solidity;
  TotalLift = dyso * lift;                        // JS: was: dynamic_pressure * solidity * lift;
  TotalDrag = dyso * drag;                        // JS: was: dynamic_pressure * solidity * drag;

  // adjust thrust according to Aoa only in air
  //*/
  if (0 == mveh->whl.GetNbWheelOnGround ()) {
  // Rotation Interference Factor (rift)
  // and Axial Interference Factor
  // Axial Interference = pirt * rift
    float _lift = lift;
    float _drag = drag;
  //
  //  JS: This code produces great vibration into torque and trust
  //  that generates instability at certain regime. Better inhibit it
  //  until we understand how to use the rift and pirt values
  //  From FlyII doc, rift is used to decrease drag while the
  //  product rift * pirt should decrease lift value.
    if (0 == mode && Torque > 0.0) { 
      // added the Torque test to let the RPM decrease when thottle is reduced
      if (fabs (rift) > FLT_EPSILON) { // > 0.0f
        //double magic_number = 100.0;
        if (TotalLift > magic_number) {// was 500 magic number
          _lift = (TotalLift * pirt / (100.0 * rift))   / dyso;
          _drag = (TotalDrag + (100.0 - (pirt / rift))) / dyso;
          //tmp_lift = min (_lift, tmp_lift);
          //tmp_drag = max (_drag, tmp_drag);
          tmp_lift = (_lift + tmp_lift) / 2.0f;
          tmp_drag = (_drag + tmp_drag) / 2.0f;
          _lift = tmp_lift;
          _drag = tmp_drag;
        }
      } 
    } else {
      ; /// \todo : find out what is tri1 mode = 1
    }
    /*
    #ifdef _DEBUG	
    {	FILE *fp_debug;
	    if(!(fp_debug = fopen("__DDEBUG_pirt.txt", "a")) == NULL)
	    {
		    fprintf(fp_debug, "[%f %f %f] %f (%f) %f (%f)\n", 
          rift, pirt, TotalLift, lift, _lift, drag, _drag);
		    fclose(fp_debug); 
    }	}
    #endif*/
    lift = _lift;
    drag = _drag;
	}

  //*/
  //--------------------------------------------------
  CT = (lift) * cos (Advance) - (drag) * sin (Advance);
  CQ = (lift) * sin (Advance) + (drag) * cos (Advance);

  //
  Thrust = facT * CT * dyso;                        // JS: was: dynamic_pressure * solidity; // 
  Torque = facQ * CQ * dyso * diam * 0.5;           // JS: was: dynamic_pressure * solidity * diam * 0.5; //

//	if  ((Torque > DBL_MAX) ||  (Torque < -DBL_MAX))
//	int a = 0;

  // adjust P fact // 
  if (mfacP) thrust_displ = mfacP->Lookup (float(blade_AoA)) * pFac * PfcK;
  //
  double RPS = RPM / 60.0; // 
  omega = RPS * TWO_PI; // rate of rotation rad per sec
  if (omega <= 1.0) omega = 1.0;

  ///////////////////////////////////////////////////////
  /// RPM changes due to increased torque in higher AoA
  /// Math :
  /// acceleration = Torque / MomentOfInertia (Ixx)
  ///////////////////////////////////////////////////////

  // torque x omega = power
  // PowerAvailable = HP
  ExcessTorque = PowerAvailable / omega * GearRatio;
  RPM = (RPS + ((ExcessTorque  / Ixx) / TWO_PI) * deltaT) * 60.0;

  // remove later
  #ifdef _DEBUG_PROP	
  {	FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_pr.txt", "a")) == NULL)
	  {
		  fprintf(fp_debug, "RPM=%f ExT%f=%f//%f*%f %f\n", 
        RPM, 
        ExcessTorque, PowerAvailable, omega, GearRatio,
        ((ExcessTorque  / Ixx) / TWO_PI) * deltaT);
		  fclose(fp_debug); 
  }	}
  #endif

  #ifdef  _DEBUG
      if (!globals->sBar) {
      char debug[256]; // 
      sprintf (debug, "3@{%8.4f %8.4f} [%8.4f %8.4f %8.4f %8.4f] (%8.4f %8.4f %5.2f (%4.2f) %8.4f) %8.4f (%2d %d)",
        RadToDeg (blade_AoA), RadToDeg (Advance),
        lift, drag, rift, pirt,
        Thrust, Torque, thrust_displ, pFac, RPM,
        FpsToKt (Vel),
        mveh->whl.GetNbWheelOnGround (),
        mode);
      //DrawNoticeToUser (debug, 1);
      }
  #endif
  
  // The friction from the engine should
  // stop it somewhere; I chose an
  // arbitrary point.
  if (RPM < 5.0)  RPM = 0;
  eRPM  = float(RPM);
if  (Thrust < 0)											// ENGINE MARK
int a = 0;
  return Thrust; // return thrust in pounds
}

float CPropellerTRIModel::GetAngularMomentum (void)
{
  // \todo Ixx = 1.76 is arbitrary moment of inertia I
  // it should be changed
  return (float)(Ixx * (RPM/60.0)*2.0*PI);
}

float CPropellerTRIModel::GetAngMomentumKgM2s(void)
{
  // \todo Ixx = 1.76 is arbitrary moment of inertia I
  // it should be changed
  // kg.m^2 = 4.214011 e-2 * lb.ft^2
  return (float)(LBFT2_TO_KGM2 * Ixx * (RPM / 60.0) * 2.0 * PI);
}
//----------------------------------------------------------------------------
//  Reset values
//----------------------------------------------------------------------------
void CPropellerTRIModel::Reset()
{	PwrAva			= 0;
	Pitch       = Advance = 0;
	ExcessTorque	= 0;
  RPM         = 0;
  Thrust      = ThrustCoeff = PowerRequired = 0;
  blade_speed = TotalDrag = 0;
	TotalLift		= 0;
  tmp_lift    = 0;
  tmp_drag    = 0;
	CT	= CQ = 0;
	CPropellerModel::Reset();
	return;
}
//----------------------------------------------------------------------------
//  Probe the prop
//----------------------------------------------------------------------------
void CPropellerTRIModel::Probe(CFuiCanva *cnv)
{ cnv->AddText(1,1,"PwrAva: %.2f HP",FTLBS_TO_HP(PwrAva));
  cnv->AddText(1,1,"PwrReq: %.2f HP",FTLBS_TO_HP(PowerRequired));
  cnv->AddText(1,1,"ExTorq: %.5f",FTLBS_TO_HP(ExcessTorque));
  //cnv->AddText(1,1,"Ixx   : %.5f",Ixx);
  cnv->AddText(1,1,"Advance %.2f",RadToDeg (Advance));
  cnv->AddText(1,1,"CT    : %.5f",CT);
  cnv->AddText(1,1,"Thrust: %.5f",Thrust);
  cnv->AddText(1,1,"CQ    : %.5f",CQ);
  cnv->AddText(1,1,"Torque: %.5f",Torque);
  cnv->AddText(1,1,"Lift  : %.5f",lift);
  cnv->AddText(1,1,"Drag  : %.5f",drag);
  cnv->AddText(1,1,"Rift  : %.5f",rift);
  cnv->AddText(1,1,"Pirt  : %.5f",pirt);
  cnv->AddText(1,1,"AoA   : %.2f",RadToDeg (blade_AoA));
  return;
}
//============END OF FILE ====================================================

