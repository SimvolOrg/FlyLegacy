/*
 * Subsystems.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2007 Chris Wallace
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

/*! \file Subsystems.h
 *  \brief Defines CSubsystem and derived aircraft system classes
 */


#ifndef SUBSYSTEMS_H
#define SUBSYSTEMS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#include "FlyLegacy.h"
#include "Utility.h"
#include "database.h"
#include "WeightManager.h"	// 
#include <vector>
#include <map>
//#include "WorldObjects.h"
//==================================================================================
class CFuiCanva;
class CSoundBUF;
class CFuiPlot;
class CRobot;
//==================================================================================
//
// Logic subsystems
//
#define SUBSYSTEM_BASE                    ('subs')  // CSubsystem
#define SUBSYSTEM_DEPENDENT               ('dpnd')  // CDependent
#define SUBSYSTEM_ANNOUNCEMENT            ('annc')  // CAnnouncement
#define SUBSYSTEM_GENERIC_MONITOR         ('genM')  // CGenericMonitor
#define SUBSYSTEM_GENERIC_INDICATOR       ('genI')  // CGenericIndicator
#define SUBSYSTEM_TIME_DELAY              ('tdla')  // CTimeDelay
#define SUBSYSTEM_KEYED_SYSTEM            ('keyd')  // CKeyedSystem
#define SUBSYSTEM_CONTACTOR               ('cntr')  // CContactor
#define SUBSYSTEM_FLASHER                 ('flsh')  // CFlasher

//
// Electrical subsystems
//
#define SUBSYSTEM_BATTERY                 ('batt')  // CBattery
#define SUBSYSTEM_ALTERNATOR              ('natr')  // CAlternator
#define SUBSYSTEM_ANNUNCIATOR_LIGHT       ('anlt')  // CAnnunciatorLight
#define SUBSYSTEM_EXTERIOR_LIGHT          ('bulb')  // CExteriorLight
#define SUBSYSTEM_VOLTMETER               ('volt')  // CVoltmeter
#define SUBSYSTEM_AMMETER                 ('amps')  // CAmmeter
#define SUBSYSTEM_GENERATOR               ('genr')  // CGenerator
#define SUBSYSTEM_FREQUENCY_METER         ('freq')  // CFrequencyMEter
#define SUBSYSTEM_INVERTER                ('nvrt')  // CInverter
#define SUBSYSTEM_PITOT_STATIC_SWITCH     ('pssw')  // CPitotStaticSwitch
#define SUBSYSTEM_PITOT_HEAT_SWITCH       ('phsw')  // CPitotHeatSwitch
#define SUBSYSTEM_STATIC_SOURCE_SWITCH    ('sssw')  // CStaticSourceSwitch
#define SUBSYSTEM_LIGHT_SETTING_STATE     ('lsst')  // CLightSettingState
#define SUBSYSTEM_SWITCH_SET              ('swst')  // CSwitchSet

//
// Gauge subsystems
//
#define SUBSYSTEM_PITOT_STATIC_SUBS       ('psss')  // CPitotStaticSubsystem
#define SUBSYSTEM_ALTIMETER               ('ALTI')  // CAltimeter
#define SUBSYSTEM_VERTICAL_SPEED          ('VSI_')  // CVerticalSpeedIndicator
#define SUBSYSTEM_AIRSPEED                ('SPED')  // CAirspeedIndicator
#define SUBSYSTEM_PNEUMATIC               ('pnus')  // CPneumaticSubsystem
#define SUBSYSTEM_PNEUMATIC_PUMP          ('pnup')  // CPneumaticPump
#define SUBSYSTEM_ATTITUDE                ('ATTI')  // CAttitudeIndicator
#define SUBSYSTEM_DIRECTIONAL_GYRO        ('GYRO')  // CDirectionalGyro
#define SUBSYSTEM_VACUUM_INDICATOR        ('vaci')  // CVacuumIndicator
#define SUBSYSTEM_TURN_COORDINATOR        ('TURN')  // CTurnCoordinator
#define SUBSYSTEM_MAGNETIC_COMPASS        ('mcmp')  // CMagneticCompass
#define SUBSYSTEM_NAVIGATION              ('navg')  // CNavigation
#define SUBSYSTEM_DIGITAL_CLOCK_OAT       ('doat')  // CDigitalClockOAT
#define SUBSYSTEM_KAP140_PANEL            ('K140')  // CBKKAP140Panel
#define SUBSYSTEM_MARKER_PANEL            ('mark')  // CMarkerPanel
#define SUBSYSTEM_INDICATOR_SET           ('inds')  // CIndicatorSet
#define SUBSYSTEM_ANNUNCIATOR             ('annr')  // CAnnunciatorSubsystem
#define SUBSYSTEM_SUCTION_GAUGE           ('suct')  // CSuctionGaugeSubsystem
#define SUBSYSTEM_FLAP_GAUGE              ('flpg')  // CFlapGauge
#define SUBSYSTEM_SYNCHROPHASER           ('sync')  // CSynchrophaser
#define SUBSYSTEM_EMERGENCY_LOCATOR       ('elts')  // CEmergencyLocator

//
// Avionics subsystems
//
#define SUBSYSTEM_DME_PANEL               ('dmep')  // CDMEPanel
#define SUBSYSTEM_DME_PANEL_1             ('dme2')  // CDMEPanel2
#define SUBSYSTEM_WEATHER_RADAR           ('wrdr')  // CWeatherRadar
#define SUBSYSTEM_COLLINS_PFD             ('cpfd')  // CCollinsPFD
#define SUBSYSTEM_COLLINS_ND              ('cnvd')  // CCollinsND
#define SUBSYSTEM_COLLINS_AD              ('cadd')  // CCollinsAD
#define SUBSYSTEM_COLLINS_MND             ('cmnd')  // CCollinsMND
#define SUBSYSTEM_COLLINS_RTU             ('crtu')  // CCollinsRTU
#define SUBSYSTEM_PILATUS_PFD             ('ppfd')  // CPilatusPFD
#define SUBSYSTEM_VIRTUAL_GPS             ('vgps')  // CVirtualGPS
#define SUBSYSTEM_TCAS_PANEL              ('tcas')  // CTCASPanel
#define SUBSYSTEM_UNIVERSAL_FMS           ('ufms')  // CFMSys

//
// Autopilot subsystems
//
#define SUBSYSTEM_AUTOPILOT_MASTER_PANEL  ('MrAp')  // CAutopilotMasterPanel
#define SUBSYSTEM_AUTOPILOT_PANEL         ('appl')  // AutoPilot
#define SUBSYSTEM_K150_PANEL              ('K150')  // CKAP150Panel
#define SUBSYSTEM_K200_PANEL              ('K200')  // CKAP200Panel
#define SUBSYSTEM_AFCS65_PANEL            ('C065')  // CAFCS65Panel
#define SUBSYSTEM_AFCS85_PANEL            ('C085')  // CAFCS85Panel
#define SUBSYSTEM_PIDBOX                  ('PIDB')  // PID BOX
//
// Engine subsystems
//
#define SUBSYSTEM_ENGINE_SUBSYSTEM        ('esub')  // CEngineSubsystem
#define SUBSYSTEM_STARTER_CONTROL         ('strt')  // CStarterControl
#define SUBSYSTEM_MAGNETO_CONTROL         ('neto')  // CMagnetoControl
#define SUBSYSTEM_IGNITER_CONTROL         ('ignc')  // CIgniterControl
#define SUBSYSTEM_TACHOMETER              ('TACH')  // CTachometer
#define SUBSYSTEM_OIL_TEMPERATURE         ('oilT')  // COilTemperature
#define SUBSYSTEM_OIL_PRESSURE            ('oilP')  // COilPressure
#define SUBSYSTEM_EXHAUST_GAS_TEMPERATURE ('eEGT')  // CExhaustGasTemperature
#define SUBSYSTEM_MANIFOLD_PRESSURE       ('eMAP')  // CManifoldPressure
#define SUBSYSTEM_HOBBS_METER             ('hobs')  // CHobbsMeter
#define SUBSYSTEM_TACHOMETER_TIMER        ('takt')  // CTachometerTimer
#define SUBSYSTEM_ENGINE_GAUGE            ('egag')  // CEngineGauge
#define SUBSYSTEM_ENGINE_STATUS           ('ests')  // CEngineStatus
#define SUBSYSTEM_FUEL_GAUGE              ('fgag')  // CFuelGaugeSubsystem
#define SUBSYSTEM_FUEL_FLOW_GAUGE         ('fflw')  // CFuelFlowSubsystem
#define SUBSYSTEM_FUEL_PRESSURE_GAUGE     ('fprs')  // CFuelPressureSubsystem
#define SUBSYSTEM_N1_TACHOMETER           ('_N1_')  // CN1Tachometer
#define SUBSYSTEM_N2_TACHOMETER           ('_N2_')  // CN2Tachometer
#define SUBSYSTEM_TORQUE_METER            ('torq')  // CTorqueMeter
#define SUBSYSTEM_CYLINDER_HEAD_TEMP      ('eCHT')  // CCylinderHeadTemp
#define SUBSYSTEM_INTERTURBINE_TEMP       ('eITT')  // CInterturbineTemp
#define SUBSYSTEM_ENGINE_MONITOR          ('eMnr')  // CEngineMonitor
#define SUBSYSTEM_STARTER_IGNITER         ('StIg')  // CStarterIgniter
#define SUBSYSTEM_FIRE_SYSTEM             ('fire')  // CFireSystem
#define SUBSYSTEM_FUEL_IMBALANCE          ('fimb')  // CFuelImbalance
#define SUBSYSTEM_FUEL_PUMP_CONTROL       ('fpsw')  // CFuelPumpControl
#define SUBSYSTEM_CROSSFEED_CONTROL       ('xfed')  // CCrossfeedControl
#define SUBSYSTEM_ROTARY_IGNITION_SWITCH  ('rign')  // CRotaryIgnitionSwitch  // moved from electrical section
#define SUBSYSTEM_ENGINE_PROXY            ('eprx')  // CEngineProxy
#define SUBSYSTEM_ENGINE_PROP             ('PROP')  // Propeller
//
// Control subsystems
//
#define SUBSYSTEM_AERO_CONTROL            ('AERO')  // CAeroControl
#define SUBSYSTEM_THROTTLE_CONTROL        ('THRO')  // CThrottleControl
#define SUBSYSTEM_MIXTURE_CONTROL         ('MIXT')  // CMixtureControl
#define SUBSYSTEM_STEERING_CONTROL        ('ster')  // CSteeringControl
#define SUBSYSTEM_FLAP_CONTROL            ('flap')  // CFlapControl
#define SUBSYSTEM_ELEVATOR_CONTROL        ('elvr')  // CElevatorControl
#define SUBSYSTEM_AILERON_CONTROL         ('ailr')  // CAileronControl
#define SUBSYSTEM_RUDDER_CONTROL          ('rudr')  // CRudderControl
#define SUBSYSTEM_ELEVATOR_TRIM_CONTROL   ('etrm')  // CElevatorTrimControl
#define SUBSYSTEM_AILERON_TRIM_CONTROL    ('atrm')  // CAileronTrimControl
#define SUBSYSTEM_RUDDER_TRIM_CONTROL     ('rtrm')  // CRudderTrimControl
#define SUBSYSTEM_IDLE_CONTROL            ('IDLE')  // CIdleControl
#define SUBSYSTEM_PROPELLER_CONTROL       ('BLAD')  // CPropellerControl
#define SUBSYSTEM_BRAKE_CONTROL           ('brak')  // CBrakeControl
#define SUBSYSTEM_GEAR_CONTROL            ('gear')  // CGearControl
#define SUBSYSTEM_PRIME_CONTROL           ('PRIM')  // CPrimeControl
#define SUBSYSTEM_HIGH_LIFT_CONTROL       ('hild')  // CHighLiftControl
#define SUBSYSTEM_COWL_FLAP_CONTROL       ('cowl')  // CCowlFlapControl
#define SUBSYSTEM_AIR_BRAKE_CONTROL       ('abrk')  // CAirBrakeControl
#define SUBSYSTEM_THRUST_REVERSE_CONTROL  ('trev')  // CThrustReverseControl
#define SUBSYSTEM_AUDIO_PANEL             ('AUDP')  // CAudioPanel
#define SUBSYSTEM_ENGINE_ANTI_ICE         ('CARB')  // CEngineAntiIce
#define SUBSYSTEM_SURFACE_DEICE           ('sdic')  // CSurfaceDeIce
#define SUBSYSTEM_PROPELLER_DEICE         ('pdic')  // CPropellerDeIce
#define SUBSYSTEM_WING_DEICE              ('wdic')  // CWingDeIce
#define SUBSYSTEM_TEMP_CONTROL            ('temp')  // CTempControl
#define SUBSYSTEM_WIPER_CONTROL           ('wipe')  // CWiperControl
#define SUBSYSTEM_PERFORMANCE_RESERVE     ('aprs')  // CPerformanceReserve
#define SUBSYSTEM_RUDDER_BIAS             ('rbia')  // CRudderBias
#define SUBSYSTEM_OXYGEN                  ('oxyg')  // COxygen

//
// Helicopter control subsystems
//
#define SUBSYSTEM_ROTOR                   ('rtrs')  // CRotorSubsystem
#define SUBSYSTEM_407_DIGITAL_INDICATORS  ('x407')  // C407DigitalIndicators
#define SUBSYSTEM_HOOK                    ('hook')  // CHookControl
#define SUBSYSTEM_THRUST_PITCH_CONTROL    ('tvpc')  // CThrustPitchControl
#define SUBSYSTEM_LON_CYCLIC_CONTROL      ('zCyc')  // CLonCyclicControl
#define SUBSYSTEM_LAT_CYCLIC_CONTROL      ('xCyc')  // CLatCyclicControl
#define SUBSYSTEM_TAIL_ROTOR_CONTROL      ('tRtr')  // CTailRotorControl
#define SUBSYSTEM_COLLECTIVE_CONTROL      ('coll')  // CCollectiveControl
#define SUBSYSTEM_LON_CYCLIC_TRIM_CONTROL ('zCyt')  // CLonCyclicTrimControl
#define SUBSYSTEM_LAT_CYCLIC_TRIM_CONTROL ('xCyt')  // CLatCyclicTrimControl
#define SUBSYSTEM_TAIL_ROTOR_TRIM_CONTROL ('tRtt')  // CTailRotorTrimControl
#define SUBSYSTEM_COLLECTIVE_TRIM_CONTROL ('tCol')  // CCollectiveTrimControl

//
// Radio subsystems
//
#define SUBSYSTEM_RADIO                   ('rado')  // CRadio
#define SUBSYSTEM_NAV_RADIO               ('navi')  // CNavRadio
#define SUBSYSTEM_RNAV_RADIO              ('rnav')  // CRnavRadio
#define SUBSYSTEM_COMM_RADIO              ('comm')  // CComRadio
#define SUBSYSTEM_HF_COMM_RADIO           ('hifr')  // CHFCommRadio
#define SUBSYSTEM_TRANSPONDER_RADIO       ('xpdr')  // CTransponderRadio
#define SUBSYSTEM_ADF_RADIO               ('adfr')  // CADFRadio
#define SUBSYSTEM_KLN89_GPS_RADIO         ('gpsr')  // CK89radio
#define SUBSYSTEM_KX155_RADIO             ('kx15')  // CBKKX155Radio
#define SUBSYSTEM_KT76_RADIO              ('kt76')  // CBKKT76Radio
#define SUBSYSTEM_AUDIO_PANEL_RADIO       ('adio')  // CAudioPanelRadio
#define SUBSYSTEM_KAP140_RADIO            ('ka14')  // CBKKAP140Radio
#define SUBSYSTEM_K89_GPS                 ('k89g')  // K89 GPS radio

//
// Vehicle state subsystems
//
#define SUBSYSTEM_HISTORY                 ('hist')  // CHistory
#define SUBSYSTEM_ON_GROUND_MONITOR       ('ognd')  // COnGroundMonitor
#define SUBSYSTEM_ALTITUDE_MONITOR        ('altm')  // CAltitudeMonitor
#define SUBSYSTEM_SPEED_MONITOR           ('spdm')  // CSpeedMonitor
#define SUBSYSTEM_STALL_WARNING           ('stal')  // CStallWarning
#define SUBSYSTEM_FAST_SLOW_METER         ('fosm')  // CFastSlowMEter
#define SUBSYSTEM_PRESSURIZATION          ('pzat')  // CPressurization
#define SUBSYSTEM_ACCELEROMETER           ('acel')  // CAccelerometer
#define SUBSYSTEM_GEAR_LIGHT              ('grlt')  // CGearLight
#define SUBSYSTEM_STALL_IDENT             ('stlI')  // CStallIdent
#define SUBSYSTEM_TEMP_AIRSPEED_DISPLAY   ('tasd')  // CTemperatureAirspeedDisplay
#define SUBSYSTEM_GEAR_WARNING            ('grwn')  // CGearWarning
//-----------------------------------------------------------------------
//    New Subsystems
//-----------------------------------------------------------------------
#define SUBSYSTEM_ROBOT										('robo')	// Internal robot
#define SUBSYSTEM_CIRCULAR_TIMER          ('crT2')  // CCircularTimer
#define SUBSYSTEM_CHECKLIST               ('chkl')  // Check list display
#define SUBSYSTEM_SMOKE                   ('smok')  // Smoke effect
//-----------------------------------------------------------------------
// Fly! V1 subsystems (deprecated)
//
#define SUBSYSTEM_THROTTLE_CONTROL_V1     ('thro')  // CThrottleV1
#define SUBSYSTEM_MIXTURE_CONTROL_V1      ('mixt')  // CMixtureControlV1
#define SUBSYSTEM_PROPELLER_CONTROL_V1    ('blad')  // CPropellerControlV1
#define SUBSYSTEM_PRIME_CONTROL_V1        ('prim')  // CPrimeControlV1
#define SUBSYSTEM_ENGINE_ANTI_ICE_V1      ('carb')  // CEngineAntiIceV1
#define SUBSYSTEM_IDLE_CONTROL_V1         ('idle')  // CIdleControlV1

// There are no instances of these deprecated subsystem types in any of
//   the available Fly! II aircraft...they are omitted from implementation
//
//#define SUBSYSTEM_ALTIMETER_V1            ('alti')  // CAltimeterV1
//#define SUBSYSTEM_VERTICAL_SPEED_V1       ('vsi_')  // CVerticalSpeedIndicatorV1
//#define SUBSYSTEM_AIRSPEED_V1             ('sped')  // CAirspeedIndicatorV1
//#define SUBSYSTEM_ATTITUDE_V1             ('atti')  // CAttitudeIndicatorV1
//#define SUBSYSTEM_DIRECTIONAL_GYRO_V1     ('gyro')  // CDirectionalGyroV1
//#define SUBSYSTEM_TURN_COORDINATOR_V1     ('turn')  // CTurnCoordinatorV1
//#define SUBSYSTEM_AUDIO_PANEL_V1          ('audp')  // CAudioPanelV1
//#define SUBSYSTEM_TACHOMETER_V1           ('tach')  // CTachometerV1

//
// Fuel subsystems specified in the GAS file
//
#define SUBSYSTEM_FUEL_SUBSYSTEM          ('fSub')  // CFuelSubsystem
#define SUBSYSTEM_FUEL_TAP                ('fTap')  // CFuelTap
#define SUBSYSTEM_FUEL_CELL               ('fCel')  // CFuelCell
#define SUBSYSTEM_FUEL_COLR               ('fCol')  // CFuel collector
#define SUBSYSTEM_FUEL_HEATER             ('fHtr')  // CFuelHeater
#define SUBSYSTEM_FUEL_MATH               ('fMat')  // CFuelMath
#define SUBSYSTEM_FUEL_PUMP               ('fPmp')  // CFuelPump
#define SUBSYSTEM_FUEL_SOURCE             ('fsrc')  // CFuelSource
#define SUBSYSTEM_FUEL_TANK               ('ftnk')  // CFuelTank
#define SUBSYSTEM_ENGINE_FUEL_PUMP        ('edfp')  // CEngineFuelPump
#define SUBSYSTEM_FUEL_MANAGEMENT         ('fmnt')  // CFuelManagement
#define SUBSYSTEM_APU_FUEL_SOURCE         ('fAPU')  // CAPUFuelSource

//
// Simulation model subsystems
//
#define SUBSYSTEM_AIR_MODEL               ('airS')  // CAirModel
#define SUBSYSTEM_MIXER_MODEL             ('mixr')  // CMixerModel
#define SUBSYSTEM_WHEEL_MODEL             ('whls')  // CWheelModel
#define SUBSYSTEM_ENGINE_MODEL            ('ngns')  // CEngineModel
#define SUBSYSTEM_WING_MODEL              ('wngs')  // CWingModel
//================================================================================
//
// Dependent function determines whether individual dependent states are combined
//  using an AND or OR logical function to determine the CDependent state.
//================================================================================
typedef enum {
  DEPENDENCY_AND,
  DEPENDENCY_OR
} EDependentFunction;

/*!
 * Enumerated value for CDependency polling behaviour
 */
typedef enum {
  POLL_NEVER,
  POLL_CHANGED,
  POLL_ALWAYS,
} EDependentPolling;
/*
 * JSDEV  *Enumarate type of values
 */
typedef enum {
	TYPE_WHAT = 0,
	TYPE_MESG = 1,
	TYPE_COMP = 2
}
	EValueType;

//================================================================================
// The indication mode of a CSubsystem determines how the actual indication
//   value tracks to the target indication value.  The target indication
//   always reflects the "true" value of the subsystem on any given update.
//
// INDN_NONE: Indication tracks exactly to target with no time lag
// INDN_LINEAR: Indication increases or decreases linearly towards the
//   target value.  Linear rate is given by the <ratK> attribute
// INDN_EXPONENTIAL:Indication grows or decays exponentially towards 
//   the target value.  Time constant for the exponential function is given
//   by the <timK> attribute.
//================================================================================
typedef enum {
  INDN_NONE         = 0,
  INDN_LINEAR       = 1,
  INDN_EXPONENTIAL  = 2,
  INDN_MOD360       = 3,
} EIndicationMode;

//------------------------------------------------------------------------------ 
// How to compare values in CGenericMonitor
//------------------------------------------------------------------------------
typedef enum {
  MONITOR_LT,
  MONITOR_GT,
  MONITOR_LE,
  MONITOR_GE,
  MONITOR_EQ,
  MONITOR_NE,
  MONITOR_XEED,
  MONITOR_BETW
} EMonitorMode;
//------------------------------------------------------------------------------
// How to compute monitored values in CGenericMonitor
//------------------------------------------------------------------------------
typedef enum {
  MODE_NONE       = 0,
  MODE_SUM        = 1,
  MODE_DIFFERENCE = 2,
  MODE_MULTIPLY	  = 3
} ECombineMode;
//====================================================================================
//   Brake side
//====================================================================================
typedef enum {
	BRAKE_NONE  = 0,
  BRAKE_LEFT  = 1,
  BRAKE_RITE  = 2,
  BRAKE_BOTH  = 3,
} EBrakeSide;
#define BRAKE_MINI (0.00001)
//====================================================================================
// *
// * Engine subsystems
// *
//====================================================================================
typedef enum {
  MAGNETO_SWITCH_OFF    = 0,
  MAGNETO_SWITCH_RIGHT  = 1,
  MAGNETO_SWITCH_LEFT   = 2,
  MAGNETO_SWITCH_BOTH   = 3,
  MAGNETO_SWITCH_START  = 4,
  MAGNETO_SWITCH_BACK   = 5
} EMagnetoSwitch;
typedef enum {
  ENGINE_MAGN_01 = 0x01,
  ENGINE_MAGN_02 = 0x02,
  ENGINE_MAGN_12 = 0x03,
  ENGINE_STARTER = 0x04,
  ENGINE_STR_ALL = 0x07,
  ENGINE_IGNITER = 0x08,
  ENGINE_PRIMER  = 0x10,
  ENGINE_FUELED  = 0x20,
	ENGINE_MIXTURE = 0x40,
} START_ITEM;
typedef enum {
  ENGINE_MAX = 8,
}EEngineProperty;
//====================================================================================
//  WHEEL POSITION
//====================================================================================
typedef enum {
  GEAR_EXTENDED   = 0,
  GEAR_RETRACTED  = 1,
  GEAR_IN_TRANSIT = 2,
}EGearPosition;
//===================================================================================
//  Structure Engine data is shared between the engine representation
//            and the physical model like CPiston
//	NOTE:	startkit integrates all running conditions for the engine such as
//				-Magneto 1 and 2
//				-Starter on/off
//				-Mixture OK
//				-Engine fueled
//				etc...
//===================================================================================
class CEngineData {
public:
  //---Mother engine ------------------------------------------
  CEngine *meng;                        // Mother engine
  U_CHAR  rfu1;                         // reserved
  U_CHAR  powered;                      // Powered engine
	//--- Running conditions --------------------
	U_INT   startKit;											// running conditions
  //----Overall state -------------------------
  U_CHAR  e_stop;                       // Cause of stopping
  U_CHAR  e_state;                      // Engine state
  //--- Control value -------------------------
  float   c_mixt;       // used with engines without mixture knob
  float   e_thro;                       // Throttle control value
  float   e_revr;                       // Reverse control value
  float   e_mixt;                       // Mixture control value
  float   e_blad;                       // Blad from controler
  //----Output values -------------------------
  float   e_hob;                        // Hobb value
  float   e_Map;                        // Manifold presure (P)
  float   e_hMap;                       // Manifold presure (Hg)
  float   e_gph;                        // fuel requested (galon per hour)
  float   e_rpm;                        // Engine RPM
  float   e_vel;                        // Aircraft velocity
  float   e_oilT;                       // Oil temperature
  float   e_oilP;                       // Oil Presure
  float   e_EGTa;                       // Exhaust Gas Temp versus blad
  float   e_EGTr;                       // Exhaust Gas temp versus rpm
  float   e_CHTa;                       // Cylinder Head Temp versus blad
  float   e_CHTr;                       // Cylinder Head Temp versus rmp
  float   e_Thrust;                     // Engine trust
  float   e_Torque;                     // Engine torque
  float   e_HP;                         // Raw Power
  float   e_PWR;                        // Power available
  float   e_af;                         // Air flow
  float   e_Pfac;
  //----Propeller parameters -----------------------------------
  float   e_pLift;                      // Lift
  float   e_pDrag;                      // Drag
	float   e_minRv;										  // Minimum RPM
	float   e_maxRv;											// Maximum RPM
	float		e_govrn;											// Governor gain
  //----Sound ratio --------------------------------------------
  float   e_Srat;                       // Sound ratio
  //----Piston engine parameters -------------------------------
  float   e_wml;                        // Engine windmill as computed
  float   s_wml;                        // Starting value
  float   s_rpm;                        // stalling rpm
  //------------------------------------------------------------
  float   dT;                           // Time step
  //------------------------------------------------------------
  double  e_hpaf;                       // Horse power
  double  e_fttb;
  double  IAS;                          // IAS in knots
  //-----TRI data ----------------------------------------------
  float   ffsc;
  float   idle;
  float   rbhp;
  double  irpm;
  double  bost_alt;
  double  bost_pr;
  int place_holder;
  //---METHODS -------------------------------------------------
  CEngineData(CEngine *m);
  //------------------------------------------------------------
  inline bool EngRunning()  {return (e_state == ENGINE_RUNNING);}
  inline bool EngStopped()  {return (e_state == ENGINE_STOPPED);}
  inline bool EngCranking() {return (e_state == ENGINE_CRANKING);}
  inline bool EngWindmill() {return (e_state == ENGINE_WINDMILL);}
  inline bool EngFueled()   {return (startKit & ENGINE_FUELED) != 0;}
  inline bool EngPowered()  {return (powered != 0);}
  //----For ests subsystem -------------------------------------
public:
  inline char   EngState()  {return e_state;}
  inline float  EngRPM()    {return e_rpm;}
  //-------------------------------------------------------------
  float Pitch();      
  //------------------------------------------------------------
};
//=====================================================================
//  DEFINE RADIO CLICK AREA
//=====================================================================
typedef enum {
 RADIO_CA01 = 1,
 RADIO_CA02 = 2,
 RADIO_CA03 = 3,
 RADIO_CA04 = 4,
 RADIO_CA05 = 5,
 RADIO_CA06 = 6,
 RADIO_CA07 = 7,
 RADIO_CA08 = 8,
 RADIO_CA09 = 9,
 RADIO_CA10 =10,
 RADIO_CA11 =11,
 RADIO_CA12 =12,
 RADIO_CA13 =13,
 RADIO_CA14 =14,
 RADIO_CA15 =15,
 RADIO_CA16 =16,
 RADIO_CA17 =17,
 RADIO_CA18 =18,
 RADIO_CA19 =19,
 RADIO_CA20 =20,
 RADIO_CA21 =21,
 RADIO_CA22 =22,
 RADIO_CA23 =23,
 RADIO_CA24 =24,
 RADIO_CA25 =25,
} ERadioClick;
//=====================================================================
//  DEFINE RADIO BUTTON AREA
//=====================================================================
typedef enum {
 RADIO_BT01 = 1,
 RADIO_BT02 = 2,
 RADIO_BT03 = 3,
 RADIO_BT04 = 4,
 RADIO_BT05 = 5,
 RADIO_BT06 = 6,
 RADIO_BT07 = 7,
 RADIO_BT08 = 8,
 RADIO_BT09 = 9,
 RADIO_BT10 =10,
 RADIO_BT11 =11,
 RADIO_BT12 =12,
 RADIO_BT13 =13,
 RADIO_BT14 =14,
 RADIO_BT15 =15,
 RADIO_BT16 =16,
 RADIO_BT17 =17,
 RADIO_BT18 =18,
 RADIO_BT19 =19,
 RADIO_BT20 =20,
 RADIO_BT21 =21,
 RADIO_BT22 =22,
 RADIO_BT23 =23,
 RADIO_BT24 =24,
 RADIO_BT25 =25,
 RADIO_BT26 =26,
 RADIO_BT27 =27,
} ERadioBtn;
//====================================================================
//  Define KX15 RADIO field number
//=====================================================================
#define K155_ACOM_WP 0                          // Active COM whole part
#define K155_ACOM_FP 1                          // Active COM fract part
#define K155_SCOM_WP 2                          // Standby COM whole part
#define K155_SCOM_FP 3                          // Standby COM fract part
#define K155_CHAN_OP 4                          // Chanel operation
#define K155_CHAN_NB 5                          // Chanel number
#define K155_DCOM_SZ 6                          // Control table size
//------NAV Fields ident ----------------------------------------------
#define K155_FCDI_GR 4                          // CDI Gradation
#define K155_FOBS_DG 5                          // OBS digits
#define K155_TIMR_MN 6                          // TIMER minute
#define K155_TIMR_SC 7                          // Timer second
#define K155_INDI_FD 8                          // Indicator position
#define K155_DNAV_SZ 9                          // Control table size
//=====================================================================
//  Define KX15 RADIO EVENTS (following gauge description panel)
//=====================================================================
enum K55_EVENT { 
  K55EV_NULL    = 0,                          // NUL EVENT
  K55EV_ACOM_WP = 1,                          // ACOM WP
  K55EV_ACOM_FP = 2,                          // ACOM FP
  K55EV_SCOM_WP = 3,                          // SCOM WP
  K55EV_SCOM_FP = 4,                          // SCOM FP
  K55EV_ANAV_WP = 5,                          // ANAV WP
  K55EV_ANAV_FP = 6,                          // ANAV FP
  K55EV_SNAV_WP = 7,                          // SNAV WP
  K55EV_SNAV_FP = 8,                          // SNAV FP
  K55EV_TUNE_CM = 9,                          // COM TUNE OUT
  K55EV_TUNE_NV = 10,                         // NAV TUNE OUT
  K55EV_CHAN_BT = 11,                         // Chanel button
  K55EV_MODE_NV = 12,                         // NAV mode
  K55EV_TRSF_CM = 13,                         // COM transfer
  K55EV_TRSF_NV = 14,                         // NAV Transfer
  K55EV_POWR_SW = 15,                         // COM volume
  K55EV_VOLM_NV = 16,                         // NAV volume
  K55EV_OBSD_D1 = 17,                         // OBS digit 1
  K55EV_OBSD_D2 = 18,                         // OBS digit 2
  K55EV_OBSD_D3 = 19,                         // OBS digit 3
  K55EV_TIMR_MN = 20,                         // TIM mn
  K55EV_TIMR_SC = 21,                         // TIM sec
  //----Additional events (additional click area)--------------
  K55EV_CHAN_DG = 22,                         // Chanel digit
  //---Additional event (no click area) -----------------------
  K55EV_CLOCK   = 27,                         // Clock event
  K55EV_TOPSC   = 28,                         // Second top
  K55EV_TUNE_CF = 30,                         // COM TUNE Fractional
  K55EV_TUNE_NF = 31,                         // NAV TUNE Fractional
};
//=====================================================================
//  Define COM-NAV STATE
//=====================================================================
#define K55_RAD_POF 0                           // Power off
#define K55_COM_NOR 1                           // Normal mode
#define K55_COM_PG  2                           // Programing mode
#define K55_COM_CH  3                           // Chanel mode
#define K55_NAV_NOR 4                           // Normal mode
#define K55_NAV_CDI 5                           // CDI mode
#define K55_NAV_BRG 6                           // Bearing  mode (to)
#define K55_NAV_RAD 7                           // Radial mode (from)
#define K55_NAV_TIM 8                           // Timer mode
#define K55_STA_SIZ 9
//=====================================================================
//  Define state radio field attribute
//=====================================================================
#define RAD_ATT_INACT 0
#define RAD_ATT_ACTIV 1
#define RAD_ATT_FLASH 2
//=====================================================================
//  DEFINE GENERIC NAV RADIO EVENTS
//=====================================================================
#define RADIO_EV_ANAV_WP 1       // CA1 active freq whole part
#define RADIO_EV_ANAV_FP 2       // CA2 active freq frac part
#define RADIO_EV_SNAV_WP 3       // Standby freq whole part
#define RADIO_EV_SNAV_FP 4       // Standby freq fract part
#define RADIO_EV_TUNE_NV 5       // Tune frequency
#define RADIO_EV_TRSF_NV 6       // Freq exchange (transfer)
//=====================================================================
//  DEFINE GENERIC COM RADIO EVENTS
//=====================================================================
#define RADIO_EV_ACOM_WP 1       // CA1 active freq whole part
#define RADIO_EV_ACOM_FP 2       // CA2 active freq frac part
#define RADIO_EV_SCOM_WP 3       // Standby freq whole part
#define RADIO_EV_SCOM_FP 4       // Standby freq fract part
#define RADIO_EV_TUNE_CM 5       // Tune frequency
#define RADIO_EV_TRSF_CM 6       // Freq exchange (transfer)
//=====================================================================
//  DEFINE GENERIC XPDR RADIO EVENTS
//=====================================================================
#define XPDR_ADG1       1       // Change active digit 1
#define XPDR_ADG2       2       // Change active digit 2
#define XPDR_ADG3       3       // Change active digit 3
#define XPDR_ADG4       4       // Change active digit 4
//--------------------------------------------------------
#define XPDR_SDG1       5       // Change standby digit 1
#define XPDR_SDG2       6       // Change standby digit 2
#define XPDR_SDG3       7       // Change standby digit 3
#define XPDR_SDG4       8       // Change standby digit 4
//--------------------------------------------------------
#define XPDR_MODE       9       // Tune mode 
#define XPDR_QWAK      10       // Sqwak ident
//=====================================================================
//  Define GENERIC NAV RADIO FIELDS
//=====================================================================
#define RADIO_FD_ANAV_WP  0
#define RADIO_FD_ANAV_FP  1
#define RADIO_FD_SNAV_WP  2
#define RADIO_FD_SNAV_FP  3
//=====================================================================
//  Define GENERIC COM RADIO FIELDS
//=====================================================================
#define RADIO_FD_ACOM_WP  0
#define RADIO_FD_ACOM_FP  1
#define RADIO_FD_SCOM_WP  2
#define RADIO_FD_SCOM_FP  3
#define RADIO_FD_NUMBER   4
//---------SECTOR VOR -------------------------------------------------
#define VOR_SECTOR_OF 0
#define VOR_SECTOR_TO 1
#define VOR_SECTOR_FR 2
//========================================================================
// *
// * Logic Subsystems
// *

/*!
 * Top-level parent class for all vehicle subsystems.
 *
 * CSubsystem is the top-level abstract parent for all electrical subsystem
 *   classes.  Every CSubsystem has an "indication" attribute stored in the 'indn'
 *   member.  The interpretation of the indication is completely arbitrary based
 *   on the specific CSubsystem subclass.  For example, the indn for a CVoltmeter
 *   object would be the voltage level of the circuit it was connected to; the indn
 *   value for a CAltimeter object would represent the vehicle barometric altitude.
 * Every subsystem has a pointer to the mother vehicle
 * Every CSubsystem instance MUST have a unique Tag identifier assigned to it in
 *   the object specification.
 * All CSubsystem objects can be configured to have the indication value lag behind
 *   the current setting through the 'timK' and 'ratK' values.
 */
//==========================================================================================
//  JS NOTE:  All subsystems has a pointer to the parent vehicle
//==========================================================================================
class CSubsystem : public CStreamObject {
  //----ATTRIBUTES ---------------------------------------------------------------
protected:
  CVehicleObject *mveh;
  //----METHODS-------------------------------------------------------------------
public:
  CSubsystem (void);
  virtual ~CSubsystem (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);
  virtual void  Write (SStream *stream);
  // CSubsystem methods -----------------------------------------------------------
  void          TypeIs (Tag t);                
  //------------------------------------------------------------------------------
  virtual const    char* GetClassName (void) { return "CSubsystem"; }
  virtual void     GetAllDependents(CFuiProbe *win) {return;}
  virtual char **  GetProbeOptions()         {return 0;}
  virtual Tag      GetMode()                 {return 0;}
  //-------------------------------------------------------------------------------
  bool          NeedUpdate(U_INT FrNo);							        // Do it needs update
  virtual bool  MsgForMe ( SMessage *msg);						      // Make virtual for special case (rign)
  //-----PROBE INTERFACE ----------------------------------------------------------
  virtual void  Plot(PLOT_PP &pp)  {}
  virtual void  Probe(CFuiCanva *cnv);
  //----- JSDEV* Message preparation -----------------------------------------------------------
  virtual	void  PrepareMsg(CVehicleObject *veh) {return; }	// Prepare all messages
			    bool  IsReceiver(SMessage *msg);						      // Is this the receiver
 //--------------------------------------------------------------------------------------------
  virtual EMessageResult  ReceiveMessage (SMessage *msg);
  virtual void  SetGroup (int group) { gNum = group; }
  virtual	void  TimeSlice (float dT,U_INT FrNo);				    // JSDEV*   new TimeSlice
			    void  TraceTimeSlice(U_INT FrNo);					        // JSDEV*	Trace activation
  virtual void  Print (FILE *f);
  virtual void  Draw()  {}                                  // For external subsystems
  //---------------------------------------------------------------------------
  void  SetIdent(Tag id);
  void  SetTimK(float t);
  //---------------------------------------------------------------------------
  inline bool   IsType (Tag t){return (type == t);}
  inline Tag    GetUnId()     {return unId;}				        // 
  inline Tag    GetType()     {return type;}
  inline char  *GetIdString() {return unId_string;}
  inline char  *GetTyString() {return type_string;}
  inline bool   IsNot(Tag t)  {return (t != unId);}
  inline bool   IsTag(Tag t)  {return (t == unId);}
  inline void   SetParent(CVehicleObject *v)  {mveh = v;}
  inline int    GetHWID()     {return hwId;}
  inline int    GetUnum()     {return uNum;}
  //----Probe management ----------------------------------------------------
  inline void   SetOption(char p) {popt = p;}
	inline void		Target(float v)		{indnTarget = v;}
protected:
  //-------------Attributes --------------------------------------------------
  // Frame counter indicates if subsystem is up to date for current cycle
	U_INT FrameNo;												    // Frame counter
  //----------- Subsystem type ----------------------------------------------
  Tag   type;
  char  type_string[8];
  // Unique ID.  Mandatory for all subsystem instances
  Tag   unId;
  char  unId_string[8];
  //-------- Probe option ----------------------------------------------------
  char      popt;                           // Probe option
  char      rfu1;                           // RFU
  //-------- TRACE option ----------------------------------------------------
  #define TRACE_ACTV	(0x01)			          // Trace activity
  U_INT			Trace;
  //----Parameters or Computing mode of indn ---------------------------------
  float     timK;                           // Time constant
  float     ratK;                           // Rate constant
  float     indn;                           // Current indication value
  float     indnTarget;                     // Target indication value
  EIndicationMode indnMode;                 // Indication mode
  //--- Associated gauge if needed ---------------------------
  CGauge    *gage;                          // Gauge to click from keyboard
  //---Sound associated to object --------------------------------------------
  CSoundBUF *sound;
  //----- unit used in messaging fields --------------------------------------
  U_SHORT    uNum;
  U_SHORT    gNum;
  EMessageHWType  hwId;
};
//============================================================================
//	Class CNullSubsystem
//		This object is generated in the CVehicle object.
//		Its purpose is to receive messages without identified receiver
//		This object should never be time sliced
//============================================================================
class CNullSubsystem: public CSubsystem {
public:
  CNullSubsystem (void);
  virtual ~CNullSubsystem (void) {};
  virtual const char* GetClassName (void) { return "CNullSubsystem"; }
  inline  char *GetUniqueID()	{return "Null";}

// Unique method return message ignored -----------------------
  EMessageResult  ReceiveMessage (SMessage *msg);
};
//=================================================================
//  CTimer.  Small class for subsytem Timer
//=================================================================
class CTimer
{ //------------Attributes ------------------------------
  char  state;            // Timer state
  float click;            // Second timer
  short min;              // Minute
  short sec;              // Second
  char  timText[8];       // Edited minute
  //-----------------------------------------------------
public:
  CTimer(void);
  void  Update(float dT);               // One timer click
  void  Change(char sta);               // Change state
  int   Toggle();                       // Toggle state
  void  SetTime(int time);              // Set initial time
  //--------inline --------------------------------------
  inline  char   *GetTime(void)  {return timText; }
  inline  int     GetTimer(void) {return ((60 * min) + sec);}
};
//=================================================================
//  Class Clock:  Maintains the local time during simulation.
//  dh, dm, ds return the deviation in degre for hh mm ss
//  This for needle clock
//=================================================================
class CClock : public CSubsystem {
  //--------Attributes --------------------------------------
  SDateTime loc;                          // Local time at start
  float     dta;                          // Total
  U_SHORT   am;                           // 0 AM 1 PM
  U_SHORT   hh;                           // Hours
  U_SHORT   mm;                           // Minute
  U_SHORT   ss;                           // Second
  U_SHORT   dh;                           // H°
  U_SHORT   dm;                           // M°
  U_SHORT   ds;                           // S°
  U_CHAR    ch;                           // Change indicator
  U_CHAR    on;                           // Flasher
  U_INT     ts;                           // Total second
  //----------------------------------------------------------
public:
  CClock();
  virtual void  Probe(CFuiCanva *cnv);
  //----------------------------------------------------------
  void  Update(float dT);
  void  Set();
  //----------------------------------------------------------
  inline  char    GetON()       {return on;}
  //----------------------------------------------------------
  inline  U_SHORT GetYear()     {return loc.date.year + 1900;}
  inline  U_SHORT GetMonth()    {return loc.date.month;}
  inline  U_SHORT GetDay()      {return loc.date.day;}
  inline  U_SHORT GetHour()     {return hh;}
  inline  U_SHORT GetMinute()   {return mm;}
  inline  U_SHORT GetSecond()   {return ss;}
  inline  U_SHORT GetHdeg()     {return dh;}
  inline  U_SHORT GetMdeg()     {return dm;}
  inline  U_SHORT GetSdeg()     {return ds;}
  inline  U_INT   GetActual()   {return ts;}
  inline  bool    NoChange()    {return (ch == 0);}
};
//====================================================================================
// CDependent introduces the concept of an on/off activity state to the CSubsystem.
//   The on/off state of a CDependency is derived from the on/off states of other
//   CDependency instances, in a hierarchical tree that can represent various
//   electrical circuits in a user vehicle.
// On each simulation cycle, the CDependent polls the 'stat' datatag of each of
//   its dependencies, combines them with either an AND or OR function, and
//   sets its own 'stat' member accordingly.  (Note, 'actv' and 'st8t' are
//   synonym datatags for 'stat').
// Since CDependent objects are typically used to represent components of
//   electrical circuits such as switches, fuses, busses, etc., additional
//   members are present to represent circuit voltage and current load
//   as well as AC frequency (if the object is dependent upon an AC current source)
//   and maximum voltage.
// Proxy messaging is a feature whereby the CDependent can send additional
//   messages to other subsystems when the state of the CDependent changes.
//
//====================================================================================
class CDependent: public CSubsystem {
public:
  CDependent (void);
  virtual ~CDependent (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void	ReadComponent(EMessageHWType Hw, SStream *stream);			// Read inverted component
  virtual void  ReadFinished (void);
  virtual void  Write (SStream *stream);
  virtual void  Probe(CFuiCanva *cnv,Tag tag = 0);              // probe with source
  virtual void  Probe(CFuiCanva *cnv)     {Probe(cnv,0);}
  virtual void  TimeSlice  (float dT,U_INT FrNo);					      // New TimeSlice
  //------ CSubsystem methods ----------------------------------------------------------------
  virtual const char* GetClassName (void) { return "CDependent"; }
  virtual EMessageResult  ReceiveMessage (SMessage *msg);
  virtual void            Print (FILE *f);
  //------------------------------------------------------------------------------------------
  void  SetState(int s);
  void  GetAllDependents(CFuiProbe *win);
	void	UpdateState(float dT,U_INT FrNo);	
  void  Poll_AND();
  void  Poll_OR_();
	void	SendAllPxy0(void);									
	void	SendAllPxy1(void);									
	void	TraceActivity(U_INT FrNo,SMessage *msg,int act);	// Trace activity
  //-------------------------------------------------------------------------------------------
  // CDependent methods
  inline int    NumDependencies (void){return dpnd.size();}
  inline int    State()               {return state;}
  inline bool   NeedPoll()            {return (mpol.group != 0);}
  inline Tag    GetMode()             {return mode;}
  //----- JSDEV* Message preparation -----------------------------------------------------------
  void PrepareMsg(CVehicleObject *veh);		// Prepare all dependent messages
 //--------------------------------------------------------------------------------------------
public:

  // Whether the dependency activity status has been evaluated for this timeslice
  
  // State dependencies
  std::vector<SMessage *> dpnd;		  // Messages used to determine our state
  std::vector<SMessage *>	pxy0m;		// List of Pxy0 messages			JSDEV*
  std::vector<SMessage *> pxy1m;		// List of Pxy1 messages			JSDEV*
  Tag					mode;		// How to combine multiple dependent states
  // Electrical ciruit parameters (read datatags)
  bool          stat; // Default activity state
  int           st8t; // Default state controller state
  float         volt; // Subsystem voltage
  float         offV; // Voltage when state is off
  float         mVlt; // Maximum voltage
  float         load; // Subsystem load (amps)
  float         freQ; // AC frequency
  unsigned int  eNum; // Dependent engine number
  //--- Real-time state---------------------------------------
  bool			    dflact;			// Initial aggregate (Or => 0, AND => 1)
  bool          active;			// Activity state (true=active, false=inactive)
  char          state;			// State controller value
  //---indicators --------------------------------------------
  char          splay;      // Sound is playing
  char          nDPND;      // Number of dependents
  char          nPXY0;      // Number of proxy 0
  char          nPXY1;      // Number of proxy 1
  char          rfu1;       // Reserved
  //------- Proxy messages for polling ----------------------
  SMessage        mpol; // Message used for polling
  float           rise; // Threshold for rising edge proxy
  float           fall; // Threshold for falling edge proxy
};

//==============================================================================
// CAnnouncement
//==============================================================================
class CAnnouncement : public CSubsystem {
public:
  CAnnouncement (void);

  // CStreamObject methods
//  virtual int   Read (SStream *stream, Tag tag);
//  virtual void  ReadFinished (void);
//  virtual void  Write (SStream *stream);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CAnnouncement"; }
//  virtual void  ReceiveMessage (SMessage *msg);
//  virtual void  Update (float dT);
};

//====================================================================================
// CGenericMonitor
//====================================================================================
class CGenericMonitor : public CDependent {
public:
  CGenericMonitor (void);
  virtual ~CGenericMonitor (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
	virtual const char* GetClassName (void) { return "CGenericMonitor"; }
	virtual EMessageResult  ReceiveMessage (SMessage *msg);
	virtual void	TimeSlice(float dT,U_INT FrNo);				// JSDEV*	
  virtual void  Probe(CFuiCanva *cnv);
	void	PrepareMsg(CVehicleObject *veh);					    // JSDEV* Prepare Messages
	float	ExtractValue(SMessage *msg);						      // JSDEV*
  float CombineValues();
  char *ExtractName(SMessage *msg);
  //-----Attributes ---------------------------------------------------------
protected:
	EMonitorMode  mode;				          //< How to compare value to reference value
	float         comp;				          //< Reference value for comparison
	float					HiVal;				        // Hight value
	float					LoVal;				        // Low value
	SMessage				*msg1;				      // message for Val2
	SMessage				*msg2;				      // message for Val2
	SMessage				*msg3;				      // message for Val3
  float         refv;                 // Reference value
  float         val1;                 // Value 1
  float         val2;                 // Value 2
  float         val3;                 // Value 3
  char          oper;                 // Operator
	bool					compR;				        // Compare result

};
//====================================================================================
//	JSDEV* Class Polynome to store polynomial coefficients for convertion of value
//	Note: coefficient are ordered in increasing order of power
//	That is if X is the input value and coef = [A0,A1,A2,..,An]
/// then output value is A0 + A1*X + A2*X² + ... + AX^n
//====================================================================================
class CPolynome: public CStreamObject {
public:
	CPolynome(void);								//	Constructor
	virtual const char* GetClassName (void) { return "CPolynome"; }
	virtual ~CPolynome(void){};						//	Destructor
	virtual int	Read (SStream *stream, Tag tag);	//	Read coefficients
	float	Compute(float vx);						//	Compute output value
private:
	int		indx;									//	Number of coefficients
	float	coef[10];								//	List of up to 10 coefficients
};

//====================================================================================
//	CGeneric Indicator
//====================================================================================
class CGenericIndicator : public CDependent {
public:
  CGenericIndicator (void);
  virtual ~CGenericIndicator (void);

  // CStreamObject methods
  virtual int       Read (SStream *stream, Tag tag);
  virtual void      ReadFinished (void);
  virtual void      Probe(CFuiCanva *cnv);
  // CSubsystem methods
  virtual const char*		GetClassName (void) { return "CGenericIndicator"; }
  virtual	EMessageResult  ReceiveMessage (SMessage *msg);
  void		PrepareMsg(CVehicleObject *veh);							// JSDEV* Prepare Messages
  virtual void				TimeSlice(float dT, U_INT FrNo);			// JSDEV*
//virtual void				OldTimeSlice (float dT, U_INT FrNo = 0);	// JSDEV*
protected:
  std::vector<SMessage*>	mVal;		///< Messages to get value for comparison
  Tag						alia;		///< Alias datatag
  ECombineMode				mode;		///< How to combine message values
  CPolynome				   *poly;		/// Polynome conversion
private:

};
//====================================================================================
//
// CTimeDelay
//
// This monitor subsystem can be configured to (do something) at periodic
//   time intervals
//====================================================================================
class CTimeDelay : public CDependent {
public:
  CTimeDelay (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTimeDelay"; }
};

//====================================================================================
// CKeyedSystem
//====================================================================================
class CKeyedSystem : public CSubsystem {
public:
  CKeyedSystem (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CKeyedSystem"; }
};
//======================================================================================
//  Subsystems are defiend from here
//  Terminal node means that no derived clss exists
//  When adding derived class, requalify as virtual all needed functions by the derived
//  class
//======================================================================================
//-----------------------------------------------------------------------
// JSDEV*  CContactor
//  Terminal node
//	Prepare Dependent messages
//-----------------------------------------------------------------------
class CContactor : public CDependent {

public:
  CContactor (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
	const char* GetClassName (void) { return "CContactor"; }
	// CContactor  methods
	void		TimeSlice(float dT,U_INT FrNo);
	void		PrepareMsg(CVehicleObject *veh);
  void    Probe(CFuiCanva *c);
  void    Reset();
protected:
	bool		NewTransition(SMessage *msg, int *actual);
	int			GetEvent(void);
	void		ClosedState(int event);
	void		OpeningState(float dT);
	void		OpenState(int event);
	void		ClosingState(float dT);
protected:
	//-----------Contactor internal state ------------------
	enum CState {	
          NONE	  = 0,
					CLOSED	= 1,
					OPENING = 2,
					OPEN	  = 3,
					CLOSING = 4, };
	//------------------------------------------------------
	enum CEvent {	RESET	= 1,
					      CLOSE	= 2,
					      EOPEN	= 3, };
	//------------------------------------------------------
	CState		Etat;				// Internal state
	int			Event;				// New event
	float		timer;				// transition timer
	float		dlyC;				  // Delay to close
	float		dlyO;				  // Delay to open
	int			Rstate;				// Reset last state
	SMessage    mReset;		// Contactor close message
	int			Cstate;				// Close last state
	SMessage    mClose;		// Contactor close message
	int			Ostate;				// Open  last state
	SMessage    mOpen;		// Contactor open message
};

//====================================================================================
// JSDEV* CFlasher
//  Terminal node
//====================================================================================
class CFlasher : public CDependent {
public:
  CFlasher (void);
  
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  const char*		  GetClassName (void) { return "CFlasher"; }
  void				    TimeSlice(float dT,U_INT FrNo);
  EMessageResult	ReceiveMessage(SMessage *msg);
  void    Probe(CFuiCanva *cnv);

protected:
  float     time;			  // Flasher cycle time
  float     delta;			// Current delta
  bool		  blnk;			  // Blink state
};


//======================================================================
// * Electrical Subsystems
//======================================================================

//======================================================================
// CBattery
//  Terminal node
//======================================================================
class CBattery : public CDependent {
public:
  CBattery (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  EMessageResult	ReceiveMessage (SMessage *msg);
  // CSubsystem methods
  const char* GetClassName (void) { return "CBattery"; }

protected:
  float   life;       ///< Capacity (amp-hours)
  float   lowLevel;   ///< Low battery level (volts)
};


//======================================================================
// JSDEV*	CAnnunciatorLight
//  Terminal node
//======================================================================
class CAnnunciatorLight : public CDependent {
public:
  CAnnunciatorLight (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  const char* GetClassName (void) { return "CAnnunciatorLight"; }
  EMessageResult	ReceiveMessage (SMessage *msg);
  // anlt method
  void		TimeSlice(float dT,U_INT FrNo);	// Update
protected:
	enum CState {OFF = 0, ON = 1,SBYE = 2};
	float		blnk;								// Blink period (sec)
	float		blpp;								// Blink rate (on duty cycle in sec)
	float		T1;									// Timer period
	float		T2;									// Timer blink
	int			bState;								// Blinking state
	int			Etat;								// Internal state
};
//===========================================================================
// CExteriorLight
//  Terminal node
//===========================================================================
class CExteriorLight : public CDependent {
public:
  CExteriorLight (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  // CSubsystem methods
  const char* GetClassName (void) { return "CExteriorLight"; }
  void        TimeSlice (float dT,U_INT FrNo = 0);			// JSDEV*
};

//===========================================================================
// CVoltmeter
//  Terminal node
//===========================================================================
class CVoltmeter : public CDependent {
public:
  CVoltmeter (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  // CSubsystem methods
  const     char* GetClassName (void) { return "CVoltmeter"; }
  void            TimeSlice(float dT,U_INT FrNo);
  EMessageResult  CVoltmeter::ReceiveMessage (SMessage *msg);
  void            Probe(CFuiCanva *cnv);
  //------- Attributes ------------------------
protected:
  float     lowV;               // Low voltage limit
  char      annV;               // Annunciator                
  SMessage  msg;                // Monitor message
};

//===========================================================================
// CAmmeter
//  Terminal node
//===========================================================================
class CAmmeter : public CDependent {
public:
  CAmmeter (void);

  // CSubsystem methods
  int   Read (SStream *stream, Tag tag);
  void  TimeSlice(float dT,U_INT FrNo);
  void  Probe(CFuiCanva *cnv);
  const char* GetClassName (void) { return "CAmmeter"; }
  //-------------------------------------------------------------------
  EMessageResult CAmmeter::ReceiveMessage (SMessage *msg);
  //----ATTRIBUTES ----------------------------------------------------
protected:
  bool        chrg;         // Charge meter
  SMessage    msg;          // Monitor message
  bool        loadMeter;    // Load meter
};
//===========================================================================
// CGenerator
//  Terminal node
//===========================================================================
class CGenerator : public CDependent {
public:
  CGenerator (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CSubsystem methods
  const char*     GetClassName (void) { return "CGenerator"; }
  void            TimeSlice(float dT,U_INT FrNo);
  EMessageResult  ReceiveMessage (SMessage *msg);
  //----ATTRIBUTES ----------------------------------------------------
protected:
  float     mxld;           // Maximum load (amps)
  float     hiRG;           // Higher RPM
  float     loRG;           // Lower RPM
  //---Engine message ------------------------------------------------
  SMessage  emsg;
};

//===========================================================================
// CFrequencyMeter
//  Terminal node
//===========================================================================
class CFrequencyMeter : public CDependent {
public:
  CFrequencyMeter (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  const char* GetClassName (void) { return "CFrequencyMeter"; }
  //----ATTRIBUTES ----------------------------------------------------
protected:
  SMessage    mMon;         ///< Monitor message
};

//===========================================================================
// CInverter
//  Terminal node
//===========================================================================
class CInverter : public CDependent {
public:
  CInverter (void);

  // CSubsystem methods
  const char* GetClassName (void) { return "CInverter"; }
};
//===========================================================================
// CPitotStaticSwitch
//===========================================================================
class CPitotStaticSwitch : public CDependent {
public:
  CPitotStaticSwitch (void);
  virtual ~CPitotStaticSwitch (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPitotStaticSwitch"; }
  //----ATTRIBUTES ----------------------------------------------------
protected:
  int   nSystems;       // Number of pitot/static systems
  int   *system;        // Array of pitot/static system group numbers
};

//===========================================================================
// CPitotHeatSwitch
//  Terminal node
//===========================================================================
class CPitotHeatSwitch : public CPitotStaticSwitch {
public:
  CPitotHeatSwitch (void);

  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CPitotHeatSwitch"; }
};

//===========================================================================
// CStaticSourceSwitch
//  Terminal node
//===========================================================================
class CStaticSourceSwitch : public CPitotStaticSwitch {
public:
  CStaticSourceSwitch (void);

  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CStaticSourceSwitch"; }
};
//=============================================================================
// CLightSettingState
//  Terminal node
//=============================================================================
class CLightSettingState : public CDependent {
public:
  CLightSettingState (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  const char* GetClassName (void) { return "CLightSettingState"; }
  EMessageResult      ReceiveMessage (SMessage *msg);
  void    Probe(CFuiCanva *cnv);
  //---ATTRIBUTES ----------------------------------------
protected:
  float   levl;       // Light level
};
//=============================================================================
// CSwitchSet
//  Terminal node
//=============================================================================
class CSwitchSet : public CDependent {
public:
  CSwitchSet (void);
  virtual ~CSwitchSet (void);

  // CStreamObject methods
  int             Read (SStream *stream, Tag tag);
  void            ReadFinished();
  //---Switch state -----------------------------------------
  void            SynchroAll();
  void            ChangeState();
  void            ChangePosition(int inx);
  void            Probe(CFuiCanva *cnv);
  EMessageResult  ReceiveMessage (SMessage *msg);
  // CSubsystem methods
  const char* GetClassName (void) { return "CSwitchSet"; }
  //--- Attributes -------------------------------------------
protected:
  int     Indx;                      // Current position
  std::vector<SMessage >    smsg;   // Switch messages
  std::vector<SMessage *>   msgs;   // List of messages
  bool    sync;         ///< Sync all switches
  int     zBase;        ///< States start at zero (instead of one)
  int     onState;      ///< Switch state in On position
  int     offState;     ///< Switch state in Off position
};

// *
// * Gauge subsystems
// *

//=============================================================================
// CPitotStaticSubsystem
//
// This subystem is an abstract base class for any subsystems which have a 
//   dependency on a pitot/static subsystem.
//=============================================================================
class CPitotStaticSubsystem : public CDependent {
public:
  CPitotStaticSubsystem (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
//  virtual void  ReadFinished (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPitotStaticSubsystem"; }
//  virtual EMessageResult  ReceiveMessage (SMessage *msg);

protected:
  int       gNum;   // Port group number
};

//=============================================================================
// JSDEV* revisited CAltimeter
//  Terminal node
//=============================================================================
class CAltimeter : public CPitotStaticSubsystem {
public:

	//-----------------------------------------------------
  CAltimeter (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  const char* GetClassName (void) { return "CAltimeter"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  void TimeSlice (float dT,U_INT FrNo = 0);					
  //--------------------------------------------------------------
	U_INT   ComputeCorrection(float kpr);
  U_INT   UpdatePressure(float inc);
  void    Probe(CFuiCanva *cnv);
  //---------Attributes ------------------------------------------
protected:
  float   kollValue;            // In inches of Hg
  float	  kollVariation;		    // Correction in feet
  float   decisionHeight;       // In feet AGL
  float   radarAlt;             // Radar altitude
  float   hPres;                // presure in inHg
  //-----Kolman digits -----------------------------------------
  U_CHAR d1;
  U_CHAR d2;
  U_CHAR d3;
  U_CHAR d4;
};
//=============================================================================
// CVerticalSpeedIndicator
//  Terminal node
//=============================================================================
class CVerticalSpeedIndicator : public CPitotStaticSubsystem {
  //--- ATTRIBUTES -----------------------------------------
protected:
  double old;
  double u100;
  double vfs;                     /// Vertical feet /sec
public:
  CVerticalSpeedIndicator (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CSubsystem methods
  const char*     GetClassName (void) { return "CVerticalSpeedIndicator"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  void            TimeSlice (float dT,U_INT FrNo = 0);		// JSDEV*

};
//=============================================================================
// CAirspeedIndicator
//  Terminal node
//=============================================================================
class CAirspeedIndicator : public CPitotStaticSubsystem {
public:
  CAirspeedIndicator (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  // CSubsystem methods
  const char* GetClassName (void) { return "CAirspeedIndicator"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  void            TimeSlice (float dT,U_INT FrNo = 0);		// JSDEV*
  //----ATTRIBUTES---------------------------------------------------
protected:
  float   maxOperatingMach;       ///< Max operating Mach number
  float   maxOperatingSpeed;      ///< Max operating speed in kts
};

//=============================================================================
//	CTurnCoordinator implementation
//  Terminal node
//=============================================================================
class CTurnCoordinator : public CDependent {
public:
  CTurnCoordinator (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  // CSubsystem methods
  const char* GetClassName (void) { return "CTurnCoordinator"; }
  EMessageResult      ReceiveMessage (SMessage *msg);
  void                TimeSlice (float dT,U_INT FrNo = 0);			// JSDEV*
  void                Probe(CFuiCanva *cnv);
  //---Attributes ------------------------------------------------------
protected:
  bool  get_data;             // get calculation only when asked
  bool  get_data_ball;        // get calculation only when asked
  float Head;                 // Aircraft banking in °
	float rateT;							  // computed turning rate
  float rateD;                // Turning rate in degre
	float tilt;									// Head rate adjust
};
//=============================================================================
// JSDEV* CMagneticCompass 
//  Terminal node
//=============================================================================
class CMagneticCompass : public CDependent {
public:
  CMagneticCompass (void);

  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CMagneticCompass"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  void TimeSlice (float dT,U_INT FrNo = 0);					// JSDEV*
};

//=============================================================================
// JSEDV* complete CDigitalClockOAT
//  Terminal node
//	NOTE:  Several tag are new but are used only beteween the 
//  displaying gauge and this subsystem.  Also internal encoding is used 
//  to avoid sprintf call for CPU optimization
//	Tag 'oatC'	gives C temperature. Format (SDDD)in intData (S=sign, ddd = 3 digits)
//		'oatF'  gives F temperature (format dito)
//		'volt'  gives Voltage in intData (format VV.V)
//		'uTim'	gives U time HHMM in intData and 00SS in engine
//		'lTim'  gives local time (format dito)
//		'fTim'	gives flight time in intData (format MMSS)
//		'eTim'	gives elapse timer in intData(format MMSS)
//
//=============================================================================
class CDigitalClockOAT : public CDependent {
public:
  CDigitalClockOAT (void);

  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CDigitalClockOAT"; }
  void TimeSlice(float dT,U_INT FrNo);
  EMessageResult  ReceiveMessage (SMessage *msg);
protected:
  // DOAT methos --------------------------------
  bool	GetRequestedValue(SMessage *msg);
  bool	SetRequestedValue(SMessage *msg);
  void  EditHour(void);
  char *EditVal(int val);
  //--------In line method ---------------------
protected:
	inline void	StoreRealData(float val,SMessage *msg)
	{	msg->realData	= val;
	}
	inline void	StoreIntgData(int val,SMessage *msg)
	{	msg->intData	= val;
	}
  //---------------Attributes ----------------------------
protected:
	SDateTime		uTime;						// Universal Date time
	SDateTime		lTime;						// Local time
  CTimer      eTimer;           // Elapse timer
  CTimer      fTimer;           // Flight timer
  char        lHour[8];         // Local Hour
  char        uHour[8];         // Universal hour
  char        dVal[8];          // Other values
	float	delta;								  // Delta for hour update
	float	Volt;								    // Battery voltage
  //----temperatures ------------------------------------
  float       cTemp;            // Celsius
  float       fTemp;            // Farenheit
};
//==========================================================================
typedef enum { 
  OUTR_MRK = 0,
  MIDL_MRK = 1,
  INNR_MRK = 2,
} EMarkType;
//==========================================================================
// CMarkerPanel
//  Terminal node
//==========================================================================
class CMarkerPanel : public CDependent {
protected:
  SMessage    rMSG;             // Radio message
  BUS_RADIO  *Radio;            // primary radio
  CBeaconMark outm;             // Outter
  CBeaconMark medm;             // medium
  CBeaconMark inrm;             // inner
public:
  CMarkerPanel (void);
 ~CMarkerPanel (void);
  // CStreamObject methods
  const char* GetClassName (void) { return "CMarkerPanel"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  // CSubsystem methods ------------------------------------
  void  PrepareMsg(CVehicleObject *veh);
  //---Panel method ----------------------------------------
  void  TimeSlice(float dT,U_INT FrNo);
  void  Probe(CFuiCanva *cnv);
};
//==========================================================================
// CIndicatorSet
//  Terminal node
//==========================================================================
class CIndicatorSet : public CDependent {
public:
  CIndicatorSet (void);
 ~CIndicatorSet (void);

  //-----------------------------------------------------------------
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);
  char  ClampPos(char p);
  void  SetPosition(char p);
  void  Probe(CFuiCanva *cnv);
  EMessageResult  ReceiveMessage (SMessage *msg);
  // CSubsystem methods
  const char* GetClassName (void) { return "CIndicatorSet"; }
  //---- ATTRIBUTES--------------------------------------------------
protected:
  SMessage               *cMsg;         // Current message
  char                    sPos;         // Start position 
  char                    ePos;         // Last position
  char                    cPos;         // Current switch position
  char                    nMsg;         // Number of messages
  float                   scale;
  std::vector<SMessage*>  smsg;         ///< Indicator source messages
};

//==========================================================================
// CAnnunciatorSubsystem
// Terminal node
//==========================================================================
class CAnnunciatorSubsystem : public CDependent {
public:
  CAnnunciatorSubsystem (void);

  // CSubsystem methods
  const char* GetClassName (void) { return "CAnnunciatorSubsystem"; }
};
//==========================================================================
// CSuctionGaugeSubsystem
// Terminal node
//==========================================================================
class CSuctionGaugeSubsystem : public CDependent {
public:
  CSuctionGaugeSubsystem (void);

  // CSubsystem methods
  const char* GetClassName (void) { return "CSuctionGaugeSubsystem"; }
};
//==========================================================================
// CFlapGauge
// Terminal node
//==========================================================================
class CFlapGauge : public CDependent {
public:
  CFlapGauge (void);
  EMessageResult  ReceiveMessage (SMessage *msg);
  // CSubsystem methods
  const char* GetClassName (void) { return "CFlapGauge"; }

protected:
};
//==========================================================================
// CSynchrophaser
// Terminal node
//==========================================================================
class CSynchrophaser : public CDependent {
public:
  CSynchrophaser (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  const char* GetClassName (void) { return "CSynchrophaser"; }

protected:
  int       leftEngine;         ///< Left engine
  int       rightEngine;        ///< Right engine
};
//==========================================================================
// CEmergencyLocator
// Terminal node
//==========================================================================
class CEmergencyLocator : public CDependent {
public:
  CEmergencyLocator (void);
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  const char     *GetClassName (void) { return "CEmergencyLocator"; }
  EMessageResult  ReceiveMessage(SMessage *msg);
  //----------------------------------------------------------------
  void  NewState(SMessage *msg);
  //-----------Attributes ------------------------------------------
protected:
  float   gLim;                 // G-load activation limit
};
//=========================================================================
// * Avionics subsystems
//=========================================================================
//==========================================================================
//  DME
//==========================================================================
class CDMEPanel : public CDependent {
public:
  CDMEPanel (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CDMEPanel"; }

protected:
  bool      hold;         ///< Hold -- ???
};
//==========================================================================
//  DME2
// Terminal node
//==========================================================================
class CDMEPanel2 : public CDMEPanel {
public:
  CDMEPanel2 (void);

  // CSubsystem methods
  const char* GetClassName (void) { return "CDMEPanel2"; }

protected:
};
//==========================================================================
//  Weather Radar
//==========================================================================
class CWeatherRadar : public CDependent {
public:
  CWeatherRadar (void);

  // CSubsystem methods
  const char* GetClassName (void) { return "CWeatherRadar"; }

protected:
};

class CCollinsPFD : public CDependent {
public:
  CCollinsPFD (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CCollinsPFD"; }

protected:
};

class CCollinsND : public CDependent {
public:
  CCollinsND (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CCollinsND"; }

protected:
};

class CCollinsAD : public CDependent {
public:
  CCollinsAD (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CCollinsAD"; }

protected:
};

class CCollinsMND : public CDependent {
public:
  CCollinsMND (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CCollinsMND"; }

protected:
};

class CCollinsRTU : public CDependent {
public:
  CCollinsRTU (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CCollinsRTU"; }

protected:
};

class CPilatusPFD : public CDependent {
public:
  CPilatusPFD (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPilatusPFD"; }

protected:
};

class CVirtualGPS : public CDependent {
public:
  CVirtualGPS (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CVirtualGPS"; }

protected:
};

class CTCASPanel : public CDependent {
public:
  CTCASPanel (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTCASPanel"; }

protected:
};

class CFMSys : public CDependent {
public:
  CFMSys (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFMSys"; }

protected:
};

//================================================================================
// CEngineSubsystem
//
// This subsystem is an abstract base class for any subsystems which have a 
//   dependency on an engine
//===================================================================================
class CEngineSubsystem : public CDependent {
  //---- ATTRIBUTES -----------------------------------------------------------------
protected:
  CVehicleObject *mveh;                 // Mother vehicle
  CEngine        *meng;                 // Mother engine  
  //---- METHODS --------------------------------------------------------------------
public:
  CEngineSubsystem (void);

  // CSubsystem methods
  virtual const char*     GetClassName      (void) { return "CEngineSubsystem"; }
  virtual void            TimeSlice         (float dT, U_INT FrNo);		// JSDEV*
  //--------------------------------------------------------------------------------
public:
  CEngineData   *eData;             // Engine data
  //--------------------------------------------------------------------------------
  inline void   SetEngineData(CEngineData *d) {eData = d;}
};

//
// CEngineGauge
//
class CEngineGauge : public CEngineSubsystem {
public:
  CEngineGauge (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CEngineGauge"; }
};



//
// CFuelGaugeSubsystem
//
class CFuelGaugeSubsystem : public CEngineSubsystem {
public:
  CFuelGaugeSubsystem (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFuelGaugeSubsystem"; }
};

//
// CFuelFlowSubsystem
//
class CFuelFlowSubsystem : public CEngineSubsystem {
public:
  CFuelFlowSubsystem (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFuelFlowSubsystem"; }
};

//
// CFuelPressureSubsystem
//
class CFuelPressureSubsystem : public CEngineSubsystem {
public:
  CFuelPressureSubsystem (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFuelPressureSubsystem"; }
};

//
// CN1Tachometer
//
class CN1Tachometer : public CEngineSubsystem {
public:
  CN1Tachometer (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CN1Tachometer"; }
};

//
// CN2Tachometer
//
class CN2Tachometer : public CEngineSubsystem {
public:
  CN2Tachometer (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CN2Tachometer"; }
};

//
// CTorqueMeter
//
class CTorqueMeter : public CEngineSubsystem {
public:
  CTorqueMeter (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTorqueMeter"; }
};


//
// CInterTurbineTemp
//
class CInterTurbineTemp : public CEngineSubsystem {
public:
  CInterTurbineTemp (void);

  // CStreamObject methods
  virtual int Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CInterTurbineTemp"; }

protected:
  bool        degC;         ///< Report temperature in degrees C
};

//=======================================================================================
//
// CStarterIgniter
//
class CStarterIgniter : public CEngineSubsystem {
public:
  CStarterIgniter (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CStarterIgniter"; }
};

//
// CFireSystem
//
class CFireSystem : public CEngineSubsystem {
public:
  CFireSystem (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFireSystem"; }
};

//
// CFuelImbalance
//
class CFuelImbalance : public CEngineSubsystem {
public:
  CFuelImbalance (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFuelImbalance"; }
};

//
// CFuelPumpControl
//
class CFuelPumpControl : public CEngineSubsystem {
public:
  CFuelPumpControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFuelPumpControl"; }
};

//
// CCrossfeedControl
//
class CCrossfeedControl : public CEngineSubsystem {
public:
  CCrossfeedControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CCrossfeedControl"; }
};


//---------------------------------------------------------------------------------------------------------
// *
// * CPneumaticSubsystem
// *
// NB : this section *must* be situated after the CEnginesubsystem section
//      for the CEngineSubsystem *pEng engine pointer 
// 
// This subystem is an abstract base class for any subsystems which have a 
//   dependency on a pneumatic pump subsystem
//
//---------------------------------------------------------------------------------------------------------
class CPneumaticSubsystem : public CDependent {
public:
  CPneumaticSubsystem              (void);

  // CStreamObject methods
  virtual int   Read               (SStream *stream, Tag tag);
//  virtual void  ReadFinished     (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPneumaticSubsystem"; }
//  virtual EMessageResult  ReceiveMessage (SMessage *msg);
  void	PrepareMsg(CVehicleObject *veh);				// JSDEV* Prepare Messages
  virtual void TimeSlice           (float dT,U_INT FrNo);			// JSDEV*

protected:
  std::vector<SMessage>            mPmp;				//< Dependent pump messages
  bool                             regulated;			//< System is pressure regulated
  float							   timer;				// Poll timer	
};

//---------------------------------------------------------------------------
// CAttitudeIndicator
//---------------------------------------------------------------------------
class CAttitudeIndicator : public CPneumaticSubsystem {
public:
  CAttitudeIndicator                     (void);

  // CStreamObject methods
	virtual int             Read           (SStream *stream, Tag tag);
  // CSubsystem methods
	virtual const char*		GetClassName   (void) { return "CAttitudeIndicator"; }
	virtual EMessageResult	ReceiveMessage (SMessage *msg);
	virtual void			TimeSlice      (float dT,U_INT FrNo);
  //--------------------------------------------------------------------
  void      Probe(CFuiCanva *cnv);
	bool	  TrackTarget(float dT);
  int     UpdateLevel(int inc);
	//--------------------------------------------------------------------
	inline	float			Clamp180(float deg)
							{	if (deg >  180) return +180;
								if (deg < -180) return -180;
								return deg;	}
  //---------ATTRIBUTES ------------------------------------------------
protected:
  float     prat;                         // Precession rate
  bool      regulated_flag;               // true = maintain op. suction when deice is in use
  int       hdeg;                         // Horizon level [-15°,+15°]
private: // precession stuff
  double    P_LIMIT;                      // pitch lower limit in precession
  double    R_LIMIT;                      // roll  lower limit in precession
  //-----Internal state --------------------------------------------------
  float     delay;                       // timer for precession
  //-----------------------------------------------------------------------
  enum DState {	STOPPED = 0,			// Stopped
				INTROPC = 1,			// Enter precession
				PRECESS	= 2,			// Precession
				TO_USER	= 3,			// Target to user
				TOLIMIT = 4,			// To limit
				RUNING	= 5,
				TOLEFT	= 6,
				TORIGH	= 7,	};
  //----------------------------------------------------------------------
  float		operP;						// Operational pressure (need adjustement)
  float		Timer;						// Timer for precess
  float		incp;						// increment pitch
  float		incr;						// increment roll
  float		ampP;
  float		ampR;
  DState	Etat;						// Internal state
  SVector	Uatt;						// User vehicle attitude
  SVector	Tatt;						// Target attitude tracked by Uatt
};
//-------------------------------------------------------------------------------
//	JSDEV* Modified for error computing and local coordinate
// CDirectionalGyro
//-------------------------------------------------------------------------------
class CDirectionalGyro : public CPneumaticSubsystem {
public:
  CDirectionalGyro (void);

  // CStreamObject methods
  virtual int             Read (SStream *stream, Tag tag);
//  virtual void          ReadFinished (void);

  // CSubsystem methods
  virtual const char*     GetClassName (void) { return "CDirectionalGyro"; }
  virtual EMessageResult  ReceiveMessage (SMessage *msg);
  virtual void            TimeSlice (float dT,U_INT FrNo = 0);		// JSDEV*
	//----------------------------------------------
protected:
  float     step;         ///< Heading bug increment/decrement step size
  bool      autoAlign;    ///< Gyro is auto aligned
  void      UpdateGyro(float inc);
  void      UpdatePbug(float inc);
  //---------ATTRIBUTES ---------------------------
private:
  float						aYaw;		// Actual yaw
  float						tYaw;		// Target yaw
  float						aRat;		// Align rate
  float						eRate;		// error rate
  float						Error;
  //-------------------------------------------------
  float           gyro;     // GYRO plate
  float           rbug;     // Auto pilot bug relative position
  int             abug;     // bug absolute position
};

//=====================================================================
// CVacuumIndicator
//=====================================================================
class CVacuumIndicator : public CPneumaticSubsystem {
public:
  CVacuumIndicator (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
//  virtual void  ReadFinished (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CVacuumIndicator"; }
  virtual EMessageResult  ReceiveMessage (SMessage *msg);
//  virtual void TimeSlice (float dT);

};


// *
// * Control subsystems
// *
struct MIXER_DATA {
  char  chn[8];         // Chanel
  float raw;            // Raw control value in range [0,1] or [-1,+1]
  float step;           // Step size
  float bend;           // Exponential value
  float posn;           // Default position
  float deflect;        // Deflection value in normalized range [0,1] or [-1,+1]
  float scaled;         // scaled data
  float scal;           // Scaling factor
  float minClamp;       // Minimum value of the control subsystem
  float maxClamp;       // Maximum value of the control subsystem
  float ampli;          // Amplitude

};
//=======================================================================
// CAeroControl
//  JSNOTE to LC: 1) remove coding related to  engine identification as those 
//          objects are not related to engine.  
//          Objects that need engine addrssing (like throttle for instance)
//          must be derived from CEngineControl
//          2) Added a data structure for reading in one message all data
//             internally.
//========================================================================
class CAeroControl : public CDependent {
public:
  CAeroControl (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished();
  // CSubsystem methods
  virtual const char*     GetClassName (void) { return "CAeroControl"; }
  virtual EMessageResult  ReceiveMessage (SMessage *msg);
  virtual void            TimeSlice (float dT,U_INT FrNo = 0);		// JSDEV*

  // CAeroControl SET methods
  virtual void    Incr          (void);
  virtual void    Decr          (void);
  virtual void    SetValue      (float fv);
  virtual void    Zero()        {data.raw = 0;}
	virtual void    ModBias(float v)	{;}
	//------------------------------------------------------------
  void						Modify(float dt);
  //------------------------------------------------------------
  void            Probe(CFuiCanva *cnv);
  //--- For all controls ---------------------------------------
  float   Clamp(float v);
  //-------------------------------------------------------------
  void    GetClamp(float &m1,float &m2) {m1 = data.minClamp,m2= data.maxClamp;}
  //---- for value ---------------------------------------------
  inline  float    Val()   {return data.raw;}
  inline  float Deflect()  {return data.deflect;};
  //--- For autopilot ------------------------------------------
	inline  void	   Neutral()	{vPID = 0; data.raw = 0;} 
  inline  void     PidValue(double v)   {vPID = v;}
  inline  void     SetMainControl(CAeroControl *c) {Cont = c;}
  //---ATTRIBUTS -----------------------------------------------
protected:
  CAeroControl    *Cont;        // Associated control for trim
  MIXER_DATA       data;
  double  vPID;       ///< FCS value (autopilot)
  double  Bias;       // Trim deviation
public:
};
//=====================================================================
// CAileronControl
//=====================================================================
class CAileronControl : public CAeroControl {
public:
  CAileronControl (void);

  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CAileronControl"; }
  void                TimeSlice (float dT,U_INT FrNo = 0);					// JSDEV*

protected:
};

//=====================================================================
// CElevatorControl
//=====================================================================
class CElevatorControl : public CAeroControl {
public:
  CElevatorControl (void);

  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CElevatorControl"; }
  void                TimeSlice (float dT,U_INT FrNo = 0);

protected:
};
//======================================================================
// CRudderControl
//  NOTE: This control is attached to CAeroControl
//=======================================================================
class CRudderControl : public CAeroControl {
public:
  CRudderControl (void);
  void    Adjust();
  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CRudderControl"; }
  //---Rudder methods ---------------------------------------
	void								ModBias(float v);
  void                TimeSlice (float dT,U_INT FrNo = 0);					// JSDEV*
  //---------------------------------------------------------
  inline void SetOpalCoef(float c)         {oADJ = c;}
  inline void SetBankMap(CFmtxMap *m) {macrd = m;}
protected:
  //----Raw adjustement --------------------------------
  CFmtxMap      *macrd;     // Adjustement table
  float          oADJ;      //Specific Opal adjustement
};
//====================================================================
//  Flap data structure
//====================================================================
struct FLAP_POS{
  int   degre;          // FLAP degre for the position
  int   speed;          // Speed limit before dammage
  float kFrame;         // Related keyframe
};
//====================================================================
// CFlapControl
//====================================================================
class CFlapControl : public CAeroControl {
public:
  CFlapControl (void);
 ~CFlapControl (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CAeroControl SET methods
  void    Incr (void);
  void    Decr (void);
  void    TimeSlice (float dT,U_INT FrNo = 0);
  void    NewPosition(int pos);
  void    SetPosition(SMessage *msg);
	void		SetPosition(int pos);
  // CSubsystem methods
  const char* GetClassName (void) { return "CFlapControl"; }
  EMessageResult  ReceiveMessage (SMessage *msg);

protected:
  CAnimatedModel *vlod;       // Level of detail
  float     fTim;             // Time for full flap        
  float     flod;             //< Motor current load when operating
  int       dump;             //< Dump position in pos array
  int       nPos;             //< Number of flap positions
  FLAP_POS *aPos;             //< Array of flap positions
  int       actualPos;
  //
};
//========================================================================
// CEngine control:
//  BASE for all susbsystem related to engine control
//========================================================================
class CEngineControl: public CDependent {
protected:
  //---ATTRIBUTES ------------------------------------
  SMessage  engm;          // Engine message
  float     step;
  U_CHAR    conv;          // how to display values, whether F° or C°
  //---METHODS ---------------------------------------
  int   Read (SStream *stream, Tag tag);
  EMessageResult ReceiveMessage(SMessage *msgr);
  //--------------------------------------------------
  void  ReadFinished();
  void  PollEngine(U_CHAR c);
  void  Monitor(Tag tag);
public:
  CEngineControl();
  virtual bool  MsgForMe (SMessage *msg);
  //--------------------------------------------------
  inline float Clamp(float v)
  { if (v < 0) return 0;
    if (v > 1) return 1;
    return v;
  }
  //--------------------------------------------------
  inline void Incr() {indnTarget = Clamp(indnTarget + step);}
  //--------------------------------------------------
  inline void Decr() {indnTarget = Clamp(indnTarget - step);}
};
//==========================================================================
// CEngineStatus
//
// This subsystem monitors the status of a single engine, and sets its
//   indication value based on whether the engine is stopped, starting or
//   running
//==========================================================================
class CEngineStatus : public CEngineControl {
public:
  CEngineStatus (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  char  DecodeMode(char *md);
  void  GetEngineState();
  EMessageResult ReceiveMessage(SMessage *msgr);
  // CSubsystem methods
  const char* GetClassName (void) { return "CEngineStatus"; }

protected:
  SMessage  emsg;         // Engine message
  char mode;              ///< Monitoring mode
};
//======================================================================
// CAlternator
//  Terminal node
//======================================================================
class CAlternator : public CDependent {
public:
  CAlternator (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CSubsystem methods
  const char* GetClassName (void) { return "CAlternator"; }
  EMessageResult ReceiveMessage (SMessage *msg);
  void        TimeSlice(float dT,U_INT FrNo);
  //---ATTRIBUTES ---------------------------------
protected:
  SMessage  emsg;     // Engine Message
  float     mxld;     // Maximum load (amps)
  float     mvlt;     // maximum volt
  float     loRg;     // Minimum RPM regulatable
  float     hiRg;     // maximum RPM regulated
  float     mxRg;     // maximum voltage RPM
  float     vFac;     // Volt coeff
};

//===========================================================================
// CEngineProxy:  This is for the amp subsystem 'ngn' object
//    This object is supposed to be a representative of the associated
//    engine number.  Any message adressed to it is relayed to the
//    real engine. Thus the proxy name.
//===========================================================================
class CEngineProxy: public CEngineControl {
  //----Methods ------------------------------
public:
  CEngineProxy();
  int   Read (SStream *stream, Tag tag);
  // CSubsystem methods
  const char* GetClassName (void) { return "CEngineProxy"; }
  //----------------------------------------------------------
  EMessageResult  ReceiveMessage (SMessage *msg);
};
//===========================================================================
// CPrimeControl
//===========================================================================
class CPrimeControl : public CEngineControl {
public:
  CPrimeControl (void);
  void            ReadFinished();
  EMessageResult  ReceiveMessage (SMessage *msg);
  // CSubsystem methods
  const char* GetClassName (void) { return "CPrimeControl"; }

protected:
  int mask[2];          // State mask
};
//===========================================================================
// CRotaryIgnitionSwitch
//===========================================================================
class CRotaryIgnitionSwitch : public CEngineControl {   
public:
  CRotaryIgnitionSwitch (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);

  // CSubsystem methods
  const char* GetClassName (void) { return "CRotaryIgnitionSwitch"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  void            TimeSlice (float dT,U_INT FrNo);			// JSDEV*

protected:
  SMessage      meng;   // Engine state
  float         sAmp;   // magnitude of amperage required to operate the starter
  int           rot_pos;

};
//===========================================================================
// CMagnetoControl
//===========================================================================
class CMagnetoControl : public CEngineControl {
public:
  CMagnetoControl (void);

  // CStreamObject methods
  void  ReadFinished();
  // CSubsystem methods
  const char* GetClassName (void) { return "CMagnetoControl"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  //---- Attributes ---------------------------------------------
protected:
  int       mask[2];            // Mask value
  Tag       NoMag;              // Magneto number
};
//=========================================================================
// CStarterControl
//=========================================================================
class CStarterControl : public CEngineControl {
public:
  CStarterControl (void);
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CSubsystem methods
  const char* GetClassName (void) { return "CStarterControl"; }
  EMessageResult  ReceiveMessage(SMessage *msg);
  void            ArmTimer (SMessage *msg);
  void            TimeSlice(float dT, U_INT FrNo);
protected:
  U_INT     time;           // Timer
  bool      mTerm;          ///< Manual starter termination
  int       mask[2];
};
//===========================================================================
// CIgniterControl
//===========================================================================
class CIgniterControl : public CEngineControl {
public:
  CIgniterControl (void);
  void  ReadFinished();
  // CSubsystem methods
  const char*       GetClassName (void) { return "CIgniterControl"; }
  EMessageResult    ReceiveMessage (SMessage *msg);
  //----Attributes ----------------------------------------
  int       mask[2];
};
//=======================================================================
// CThrottleControl
// * The throttle position is retrieved by the eMap subsystem
// in the skyhawk system
//=======================================================================
class CThrottleControl : public CEngineControl {
	//--- Attribute ------------------------------------------
	CEngineData  *data;
public:
  CThrottleControl (void);
  void  ReadFinished();
	void	PrepareMsg(CVehicleObject *veh);
	void	Target(float v);
  // CSubsystem methods
  const char* GetClassName (void) { return "CThrottleControl"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
};
//==========================================================================
// CThrustReverseControl
//==========================================================================
class CThrustReverseControl : public CEngineControl {
public:
  CThrustReverseControl (void);
  void  ReadFinished();
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  EMessageResult  ReceiveMessage (SMessage *msg);
  // CSubsystem methods
  const char* GetClassName (void) { return "CThrustReverseControl"; }

protected:
  float   dTime;           ///< Deployment time (sec)
};
//=======================================================================
// CMixtureControl
//=======================================================================
class CMixtureControl : public CEngineControl {
public:
  CMixtureControl (void);
  // CSubsystem methods
  const char* GetClassName (void) { return "CMixtureControl"; }
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
	void	PrepareMsg(CVehicleObject *veh);
  EMessageResult  ReceiveMessage (SMessage *msg);
	int		SetControl(float val);
	//--------------------------------------------------------
protected:
	CEngineData	*data;
  bool  autoControl;    // Under automatic or user control?
  bool  injd;       // Fuel injected system (vs carbeurated)
  float prmr;       // Priming rate
  float prff;       // Priming fuel flow
  float plim;       // Priming pump limit
};
//=======================================================================
// CPropellerControl
//=======================================================================
class CPropellerControl : public CEngineControl {
public:
  CPropellerControl (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CSubsystem methods
  const char* GetClassName (void) { return "CPropellerControl"; }
	void	PrepareMsg(CVehicleObject *veh);
  EMessageResult  ReceiveMessage  (SMessage *msg);
  //---ATTRIBUTES ----------------------------------------------
protected:
	CEngineData *data;				// Shared engine data
  float   minRPM;           ///< Minimum governable engine RPM
  float   maxRPM;           ///< Maximum governable engine RPM
  float   governorGain;     ///< Governor gain
  bool    autoFeather;      ///< Auto-feather enable
};
//===============================================================================
// CIdleControl:  TO BE IMPLEMENTED WHEN NEEDED
//===============================================================================
class CIdleControl : public CEngineControl {
public:
  CIdleControl (void);

  // CSubsystem methods
  const char* GetClassName (void) { return "CIdleControl"; }

protected:
};
//===================================================================================
// CTachometer
//===================================================================================
class CTachometer : public CEngineControl{
//---ATTRIBUTES -----------------------------------------------
  SMessage emsg[ENGINE_MAX];
//---METHODS --------------------------------------------------
public:
  CTachometer (void);
  void      ReadFinished();
  bool      GetRPM(SMessage *msg);
  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CTachometer"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
};

//===================================================================================
// CTachometerTimer
//===================================================================================
class CTachometerTimer : public CEngineControl {
public:
  CTachometerTimer (void);
  // CStreamObject methods
  void    ReadFinished();
  // CSubsystem methods
  const char* GetClassName (void) { return "CTachometerTimer"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
protected:
};
//===================================================================================
// COilTemperature
//===================================================================================
class COilTemperature : public CEngineControl{
public:
  COilTemperature (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CSubsystem methods
  const char*		  GetClassName (void) { return "COilTemperature"; }
  EMessageResult	ReceiveMessage (SMessage *msg);
  // 
protected:
};
//===================================================================================
// COilPressure
//===================================================================================
class COilPressure : public CEngineControl {
public:
  COilPressure (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CSubsystem methods
  const char*		GetClassName (void) { return "COilPressure"; }
  EMessageResult	ReceiveMessage (SMessage *msg);		// JSDEV*

protected:
	float	lowP;						// Low pressure threshold
	bool	plow;						// Low indicator
};
//====================================================================
// CManifoldPressure
//=====================================================================
class CManifoldPressure : public CEngineControl {
public:
  CManifoldPressure (void);
  void  ReadFinished();
  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CManifoldPressure"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
};
//======================================================================
// CCylinderHeadTemp
//======================================================================
class CCylinderHeadTemp : public CEngineControl {
public:
  CCylinderHeadTemp (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CSubsystem methods
  const char* GetClassName (void) { return "CCylinderHeadTemp"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  // 
protected:
  U_CHAR  conv;                 // how to display values, whether F° or C°
};
//=======================================================================
// CExhaustGasTemperature
//=======================================================================
class CExhaustGasTemperature : public CEngineControl {
public:
  CExhaustGasTemperature (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CSubsystem methods
  const char*		GetClassName (void) { return "CExhaustGasTemperature"; }
  EMessageResult	ReceiveMessage (SMessage *msg);
  // 
protected:
  U_CHAR conv;                 // how to display values, whether F° or C°
};
//============================================================================
// CHobbsMeter
//============================================================================
class CHobbsMeter : public CEngineControl {
public:
  CHobbsMeter (void);
  void    ReadFinished();
  // CStreamObject methods
  // CSubsystem methods
  const char* GetClassName (void) { return "CHobbsMeter"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  // CHobbsMeter methods
protected:
};
//==================================================================================
// CEngineMonitor
//==================================================================================
class CEngineMonitor : public CEngineControl{
public:
  CEngineMonitor (void);

  // CStreamObject methods
  int Read (SStream *stream, Tag tag);

  // CSubsystem methods
  const char* GetClassName (void) { return "CEngineMonitor"; }

protected:
  SMessage    mMAP;       ///< Message connection to manifold pressure
  SMessage    mRPM;       ///< Message connection to tachometer
  SMessage    mTIT;       ///< Message connection to turbine interstage temp
  SMessage    mFMS;       ///< Message connection to fuel management system
  SMessage    mOT;        ///< Message connection to oil temperature
  SMessage    mOP;        ///< Message connection to oil pressure
  SMessage    mCHT;       ///< Message connection to cylinder head temp
  SMessage    mVAC;       ///< Message connection to vacuum gauge
  SMessage    mLQTY;      ///< Message connection to left fuel quantity
  SMessage    mRQTY;      ///< Message connection to right fuel quantity
};

//=======================================================================
// CPneumaticPump
//=======================================================================
class CPneumaticPump : public CEngineControl {
public:
  CPneumaticPump (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished   (void);
  // CSubsystem methods
  const char* GetClassName (void) { return "CPneumaticPump"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  void TimeSlice (float dT,U_INT FrNo = 0);			// JSDEV*
  void Probe(CFuiCanva *cnv);

protected:
  float             rpm;
  float             suct;				      //< Pump suction (psi) at Max RPM
//  unsigned int pump_eNum;				    //< specifies the index of the engine that this pump depends
  int               dIce;				      //< the pressure required to operate the de-ice inflatable boots
  int               Lrpm;				      //< the least RPM required to create sufficient succion
  int               Hrpm;				      //< the greatest RPM tolerated to create sufficient succion
  float             mSct;				      //< the suction generated at the Least Operative RPM
  float				     Coef;				      //< proportional coef to RPM
};
//=====================================================================
// CEngineAntiIceV1
//  NOT YET IMPLEMENTED
//=====================================================================
class CEngineAntiIceV1 : public CEngineControl{
public:
  CEngineAntiIceV1 (void);

  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CEngineAntiIceV1"; }
};
//============================================================================
// CThrottleControlV1
//  NOT YET IMPLEMENTED
//============================================================================
class CThrottleControlV1 : public CEngineControl {
public:
  CThrottleControlV1 (void);

  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CThrottleControlV1"; }
};
//=============================================================================
// CMixtureControlV1
//  NOT YET IMPLEMENTED
//=============================================================================
class CMixtureControlV1 : public CEngineControl {
public:
  CMixtureControlV1 (void);

  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CMixtureControlV1"; }
};
//=====================================================================
// CPropellerControlV1
//  NOT YET IMPLEMENTED
//=====================================================================
class CPropellerControlV1 : public CEngineControl {
public:
  CPropellerControlV1 (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPropellerControlV1"; }
};
//====================================================================
// CPrimeControlV1
//  NOT YET IMPLEMENTED
//=====================================================================
class CPrimeControlV1 : public CEngineControl {
public:
  CPrimeControlV1 (void);

  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CPrimeControlV1"; }
};
//=================================================================================
// CIdleControlV1
//  NOT YET IMPLEMENTED
//=================================================================================
class CIdleControlV1 : public CEngineControl {
public:
  CIdleControlV1 (void);

  // CStreamObject methods

  // CSubsystem methods
  const char* GetClassName (void) { return "CIdleControlV1"; }
};
//===================================================================================
// CSteeringControl
//
class CSteeringControl : public CDependent {
public:
  CSteeringControl (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CSteeringControl"; }

protected:
};

//======================================================================
// CElevatorTrimControl
//======================================================================
class CElevatorTrimControl : public CAeroControl {
public:
  CElevatorTrimControl (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CElevatorTrimControl"; }
	void								ReadFinished();
  void                TimeSlice (float dT,U_INT FrNo = 0);					// SDEV*

protected:
};

//
// CAileronTrimControl
//
class CAileronTrimControl : public CAeroControl {
public:
  CAileronTrimControl (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CAileronTrimControl"; }

protected:
};

//
// CRudderTrimControl
//
class CRudderTrimControl : public CAeroControl {
public:
  CRudderTrimControl (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CRudderTrimControl"; }

protected:
};
//==============================================================================
// CBrakeControl
//==============================================================================
// JS NOTE:  Brake control receive messages from
//    Keyboard  for left brake
//              for right brake
//              for both brakes
//    Gauge for parking brake
//  Any change is propagated to CGroundBrake Object
//===========================================================================
class CBrakeControl : public CDependent { 
public:
  CBrakeControl (void);

  // CStreamObject methods
  void    ReadFinished();
  // CAeroControl SET methods
  void    Incr (char pos);
  int     Decr (char pos);
  void    PressBrake(char pos);
  void    ReleaseBrakes();
  void    SwapPark();
	void		Probe(CFuiCanva *cnv);
  //--------------------------------------------------------------------
  inline void   HoldBrake(char p)     {Hold = p;}
	inline float  GetBrakeForce(char p) {return (p)?( Force[p]):(0);}
  inline void   SetParking()          {Park = 1;}
  //--------------------------------------------------------------------
  // CSubsystem methods
  const char* GetClassName (void) { return "CBrakeControl"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  void            TimeSlice (float dT,U_INT FrNo = 0);		// JSDEV*

protected:
  char  Park;
  char  Hold;         // Brake on
  float rate;
  float Brake[3];     // Brake force
  float Force[3];     // Real force
  char  turn;         // set which timeslice do the brake release
  float br_timer;
  float rf_timer;     // reference
};

//===========================================================================
// CGearControl
//===========================================================================
class CGearControl : public CDependent{
public:
  CGearControl (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CSubsystem methods
  const char* GetClassName (void) { return "CGearControl"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  //---Control methods ---------------------------------------
  void  Swap();
  void  ChangePosition();
  //----------------------------------------------------------
  char  Lock()        {return lock;}
  bool  Active()      {return active;}
  //----ATTRIBUTES -------------------------------------------
protected:
  char    hydr;               ///< Gear is hydraulically actuated
  char    lock;               ///< Gear lever is locked when on ground
  int     Pos;
};
//===========================================================================
// CHighLiftControl
//===========================================================================
class CHighLiftControl : public CAeroControl {
public:
  CHighLiftControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CHighLiftControl"; }

protected:
};

//
// CCowlFlap
//
class CCowlFlapControl : public CAeroControl {
public:
  CCowlFlapControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CCowlFlap"; }

protected:
};

//==========================================================================
// CAirBrakeControl
//==========================================================================
class CAirBrakeControl : public CAeroControl {
public:
  virtual const char* GetClassName (void) { return "CAirBrakeControl"; }
  CAirBrakeControl (void);
  int                 Read (SStream *stream, Tag tag);
//----Attributes ------------------------------------------------
protected:
  SMessage msg;                           // Flap signal                
};


//
// CAudioPanel
//
class CAudioPanel : public CDependent {
public:
  CAudioPanel (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CAudioControl"; }

protected:
};

//
// CEngineAntiIce
//
class CEngineAntiIce : public CDependent {
public:
  CEngineAntiIce (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CEngineAntiIce"; }

protected:
};

//
// CSurfaceDeIce
//
class CSurfaceDeIce : public CDependent {
public:
  CSurfaceDeIce (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CSurfaceDeIce"; }

protected:
};

//
// CPropellerDeIce
//
class CPropellerDeIce : public CDependent {
public:
  CPropellerDeIce (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPropellerDeIce"; }

protected:
  float             cycleTime;        ///< Deice cycle time
  std::vector<int>  propHeater;       ///< Array of propeller heaters
  float             heaterLoad;       ///< Heater load (amps)
};

//=====================================================================================
// CWingDeIce
//=====================================================================================
class CWingDeIce : public CDependent {
public:
   CWingDeIce (void);
  ~CWingDeIce (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CWingDeIce"; }

protected:
  float                   cycleTime;  ///< Deice cycle time in seconds
  std::vector<SMessage*>  msgPump;    ///< List of pump connection messages
};

//=====================================================================
// CTempControl
//=====================================================================
class CTempControl : public CDependent {
public:
  CTempControl (void);
  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTempControl"; }
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  Probe(CFuiCanva *cnv);
  virtual void  TimeSlice (float dT,U_INT FrNo);
  virtual EMessageResult ReceiveMessage (SMessage *msg);
protected:
  //--------------------------------------------------------
  float base;                             // Base tic
  float rate;                             // Rate of change
  float temp;                             // Temperature
  float sign;                             // Sign of increment
  int   mint;                             // mini
  int   maxt;                             // maxt
};

//====================================================================
// CWiperControl
//====================================================================
class CWiperControl : public CDependent {
public:
  CWiperControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CWiperControl"; }

protected:
};

//
// CPerformanceReserve
//
class CPerformanceReserve : public CDependent {
public:
  CPerformanceReserve (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPerformanceReserve"; }

protected:
};

//
// CRudderBias
//
class CRudderBias : public CDependent {
public:
  CRudderBias (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CRudderBias"; }

protected:
  int       rpmOption;        ///< RPM option (0=RPM, 1=N1, 2=N2)
  float     rpmLimit;         ///< RPM limit
  int       leftEngine;       ///< Left engine index
  int       rightEngine;      ///< Right engine index
};

//
// COxygen
//
class COxygen : public CDependent {
public:
  COxygen (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "COxygen"; }

protected:
  float     full;
  float     fill;
  float     rate;
  float     pressure;
};


// *
// * Helicopter subsystems
// * 

//
// CRotorSubsystem
//
class CRotorSubsystem : public CDependent {
public:
  CRotorSubsystem (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CRotorSubsystem"; }

protected:
};

//
// C407DigitalIndicators
//
class C407DigitalIndicators : public CDependent {
public:
  C407DigitalIndicators (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "C407DigitalIndicators"; }

protected:
};

//
// CHookSubsystem
//
class CHookSubsystem : public CDependent {
public:
  CHookSubsystem (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CHookSubsystem"; }

protected:
};

//
// CThrustPitchControl
//
class CThrustPitchControl : public CDependent {
public:
  CThrustPitchControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CThrustPitchControl"; }

protected:
};

//
// CLonCyclicControl
//
class CLonCyclicControl : public CDependent {
public:
  CLonCyclicControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CLonCyclicControl"; }

protected:
};

//
// CLatCyclicControl
//
class CLatCyclicControl : public CDependent {
public:
  CLatCyclicControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CLatCyclicControl"; }

protected:
};

//
// CTailRotorControl
//
class CTailRotorControl : public CDependent {
public:
  CTailRotorControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTailRotorControl"; }

protected:
};

//
// CCollectiveControl
//
class CCollectiveControl : public CDependent {
public:
  CCollectiveControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CCollectiveControl"; }

protected:
};

//
// CLonCyclicTrimControl
//
class CLonCyclicTrimControl : public CDependent {
public:
  CLonCyclicTrimControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CLonCyclicTrimControl"; }

protected:
};

//
// CLatCyclicTrimControl
//
class CLatCyclicTrimControl : public CDependent {
public:
  CLatCyclicTrimControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CLatCyclicTrimControl"; }

protected:
};

//
// CTailRotorTrimControl
//
class CTailRotorTrimControl : public CDependent {
public:
  CTailRotorTrimControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTailRotorTrimControl"; }

protected:
};

//
// CCollectiveTrimControl
//
class CCollectiveTrimControl : public CDependent {
public:
  CCollectiveTrimControl (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CCollectiveTrimControl"; }

protected:
};




//==================================================================================
// CNavigation
//==================================================================================
class CNavigation : public CDependent {
  //----Attributes ----------------------------------------------------
  BUS_RADIO *radio;
  SMessage   msg;
  //-------------------------------------------------------------------
public:
  CNavigation (void);
  int   SetXOBS(short inc);
  //------------------------------------------------------------------
  // CStreamObject methods

  // CSubsystem methods
  virtual const     char* GetClassName (void) { return "CNavigation"; }
  virtual EMessageResult  ReceiveMessage (SMessage *msg);
  //------------------------------------------------------------------
};

//==========================================================================
// CAudioPanelRadio
//
class CAudioPanelRadio : public CDependent {
public:
  CAudioPanelRadio (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CAudioPanelRadio"; }
//  virtual EMessageResult  ReceiveMessage (SMessage *msg);

protected:
};

//
// CBKKAP140Radio
//
class CBKKAP140Radio : public CDependent {
public:
  CBKKAP140Radio (void);

  // CStreamObject methods
//  virtual int   Read (SStream *stream, Tag tag);
//  virtual void  ReadFinished (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CBKKAP140Radio"; }
//  virtual EMessageResult  ReceiveMessage (SMessage *msg);

protected:
};


// *
// * Vehicle state subsystems
// *

//
// CHistory
//
class CHistory : public CDependent {
public:
  CHistory (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CHistory"; }
};

//
// COnGroundMonitor
//
class COnGroundMonitor : public CDependent {
public:
  COnGroundMonitor (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "COnGroundMonitor"; }
};

//
// CAltitudeMonitor
//
// This subsystem monitors the user vehicle altitude and compares it to a preset
//   altitude threshold.  The subsystem state reflects whether the user vehicle
//   is above or below the threshold altitude.
//
class CAltitudeMonitor : public CDependent {
public:
  CAltitudeMonitor (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CAltitudeMonitor"; }
};

//
// CSpeedMonitor
//
// This subsystem monitors the user vehicle airspeed and compares it to a preset
//   threshold value.  The subsystem state reflects whether the user vehicle
//   is travelling faster or slower than the threshold airspeed.
//
class CSpeedMonitor : public CDependent {
public:
  CSpeedMonitor (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CSpeedMonitor"; }
};

//
// CStallWarning
//
class CStallWarning : public CDependent {
public:
  CStallWarning (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CStallWarning"; }

protected:
  char    wing[64];              ///< Wing section name
  bool    enableStateAnnouncer;
};

//
// CFastSlowMeter
//
class CFastSlowMeter : public CDependent {
public:
  CFastSlowMeter (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFastSlowMeter"; }
};

//
// CPressurization
//
class CPressurization : public CDependent {
public:
  CPressurization (void);

  // CStreamObject methods
  int Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPressurization"; }

protected:
  bool      switchCabinPressure;      ///< Cabin pressure switch state
  bool      switchPressureControl;    ///< Pressure control switch state
  float     diffPressure;             ///< Maximum differential pressure
  float     warningAlt;               ///< Warning altitude
};

//
// CAccelerometer
//
class CAccelerometer : public CDependent {
public:
  CAccelerometer (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CAccelerometer"; }
};

//=============================================================================
// CGearLight
//=============================================================================
class CGearLight : public CDependent {
  //------METHODS ---------------------------------------------
public:
  CGearLight (void);

  // CStreamObject methods
  int     Read (SStream *stream, Tag tag);
  void    ReadFinished();
  char    ClampMode(char m);
  // CSubsystem methods
  const char* GetClassName (void) { return "CGearLight"; }
  //-----------------------------------------------------------
  void    TimeSlice(float dT,U_INT FrNo);
  //-------ATTRIBUTES -----------------------------------------
protected:
  CAnimatedModel *vlod;
  char    mode;               // Monitor mode (0=Extended, 1=Retracted, 2=Transit)
  char    gNum;               // Gear number
};

//=============================================================================
// CStallIdent
//
class CStallIdent : public CDependent {
public:
  CStallIdent (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CStallIdent"; }
};

//
// CTempAirspeedDisplay
//
class CTempAirspeedDisplay : public CDependent {
public:
  CTempAirspeedDisplay (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTempAirspeedDisplay"; }
};

//
// CGearWarning
//
class CGearWarning : public CDependent {
public:
  CGearWarning (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CGearWarning"; }

protected:
  float   throttleLimit;      ///< Minimum throttle to inhibit warning
  float   flapLimit;          ///< Minimum flaps to inhibit warning
  bool    odd;                ///< Gear position and lever agreement
};


//
// CTachometerBase
//
class CTachometerBase : public CEngineSubsystem {
public:
  CTachometerBase (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTachometerBase"; }
};

//
// CTachometerV1
//
class CTachometerV1 : public CTachometerBase {
public:
  CTachometerV1 (void);

  // CStreamObject methods

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTachometerV1"; }
};


/*!
 *
 *  CFuelSubsystem class
 *
 *  This class is the base for all the subsystems found in the GAS (fuel system)
 *  stream file.
 *
 *  An overall_upstream_state is maintained throughtout the derived class in order
 *  to enable or disable the fuel FLOW from a cell or tank. The BURNING is enabled with
 *  a fuel_burning_flag.
 * 
 *  The pipes objects yield a list of recipient message (pipe_msg).
 *  A list of related cells is also created and maintained (related_cell) :
 *   CFuelSystem::ReadFinished
 *   A pointer of a cell fuel is created and saved as a valve object in order
 *   to enable or disable fuel consumption.
 *  
 *  TimeSlice :
 *  It sends the overall_upstream_state to all the derived classes.
 *  It enables fuel_burning_flag of each cell.
 *
 */

//
// 
//

//====================================================================================
//  Fuel subsystem
//===================================================================================
class CFuelSubsystem : public CDependent {
public:
  // Constructor
  CFuelSubsystem (void);
  virtual ~CFuelSubsystem (void);

  // CStreamObject methods
  virtual int                   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char*           GetClassName (void) { return "CFuelSubsystem"; }
  virtual	EMessageResult        ReceiveMessage (SMessage *msg);
  void		PrepareMsg(CVehicleObject *veh);							// JSDEV* Prepare Messages
  virtual void                  TimeSlice (float dT,U_INT FrNo = 0);	// JSDEV*	
  //-------------------------------------------------------------------
  virtual bool   IsOFF()           {return (0 == state);}
  virtual char   ByPass()          {return 'N';}
  //-------------------------------------------------------------------
  void    LinkCell(CFuelSystem *fsys);
  void    TraceLink(CFuelSubsystem *fs);
  void    TraceFS(CFuelSubsystem *fs, float rq);
  void    TraceT1(CFuelCell *fc,float frq);
  void    GetContributingTanks(CFuelSystem *fsys,float rqt);
  void    TraceT2(CFuelCell *fc, CFuelCell *tk,  float frq);
  void    GetRefillingTanks(CFuelSystem *fsys,CFuelCell *cel,float rqf);
  void    Poll();
  //-------------------------------------------------------------------
  inline const char*            GetName (void) const {return name;}
  //-------------------------------------------------------------------
  inline bool   IsaTank()           {return (1 == Fsrc);}
  inline bool   NotTank()           {return (0 == Fsrc);}
  inline void   SetName(char *n)    {strncpy(name,n,64); name[63] = 0;}
  inline U_INT  EngineNo()          {return eNum;}
  //-------------------------------------------------------------------
  // Data members
public:
  float                         f_gph;            ///< Fuel flow through tap (gallons per hour)
  float                         gals;             ///< Fuel quantity in gallons

//protected:
  char                          name[64];         ///< UI readable name
  //-------------------------------------------------------------------
  std::map<Tag,CFuelSubsystem*> piped;            // Piped subsystems
  //-----ATTRIBUTES ---------------------------------------------------
public:
  bool                          force_fuel;
  char                          Fsrc;             // Tank indicator
  char                          Tr;               // Trace request
};

//===========================================================================
// CFuelTap
//===========================================================================
class CFuelTap : public CFuelSubsystem {
public:
  // Constructor
  CFuelTap (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);

  // CSubsystem methods
  const char* GetClassName (void) { return "CFuelTap"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  void TimeSlice (float dT,U_INT FrNo);
  //-------------------------------------------------------------
  inline void SetFuelFlow(float ff) {f_gph = ff;}
  //---- Data members ---------------------------------------------
protected:
  float     stff;       // Startup fuel flow
};
//======================================================================
// CFuelCell
//======================================================================
#define FUEL_FEED_STACK (16)
//======================================================================
//  NOTE:  All data from CFuelUnit are moved here
//======================================================================
class CFuelCell : public CFuelSubsystem {
public:
  // Constructor
  CFuelCell (U_CHAR ind);
  CFuelCell ();
 ~CFuelCell();
  void Create();
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);
  //-------------------------------------------------------------
  virtual float   GetLimitQty()       {return qty;}
  //---------- CSubsystem methods -------------------------------
  const char*     GetClassName (void) { return "CFuelCell"; }
  EMessageResult  ReceiveMessage (SMessage *msg);
  void            TimeSlice (float dT,U_INT FrNo);

  //--------------------------------------------------------------
  void                    BurnFuel(float bnf);
  void                    SetCellQty(float qty);
  void                    Probe(CFuiCanva *cnv);
  bool                    CanFill(CFuelCell *cel,float rqf);
  void                    GiveFuelTo(CFuelCell *tk, float rqf);
  void                    AddFuel(float fqt);
  void                    RemFuel(float fqt);
  void                    ClearFeed();
  //--------------------------------------------------------------
  void                    ContributionCG(SVector &mm, SVector &vn, double &sm);
  //--------------------------------------------------------------
  inline  char            GetCellIndex()      {return xInd;}
  inline  void            SetCellWgh(float w) {wgh = w;};
  inline  void            SetCellCap(float c) {cap = c;}
  inline  SVector         GetCellPos () {return bPos;}
  inline  float           GetCellQty () {return gals;}
  inline  float           GetCellCap () {return cap;}
  inline  float           GetCellWgh () {return wgh;}
  inline  float           GetCellPCT () {return (gals * 100) / cap;}
  inline  float           GetXXRate()   {return xrFF;}
  inline  float           GetRemainingCap()   {return (cap - qty);}
  inline  float           GetXloc()           {return float(bPos.x);}
  inline  float           GetYloc()           {return float(bPos.y);}
  inline  float           GetZloc()           {return float(bPos.z);}
  inline  SVector         GetPosition()       {return bPos;}
  //--------------------------------------------------------------
  inline bool   HasFuelFor(float q) {return (gals >= q);}
  inline void   DumpFuel()    {gals = qty = 0;}
  //--------------------------------------------------------------
  // Data members
protected:
  float   cap;              //< Capacity (gallons)
  float   qty;              //< Default quantity (gallons)
  float   wgh;              //< Weight
  CVector bPos;             //< Fuel cell position relative to aircraft CoG
  float   basP;             //< Base pressure (psi)
  //float   Dqty;           //< Fuel burned negative
  bool    xfer;             //< Forced fuel transfer
  float   xrFF;             //< Forced fuel transfer flow rate (gal/sec)
  //---Stack index for this cell -----------------------------
  U_CHAR  xInd;            // Cell index into Feed
  //---Stack for 16 fuel cells -------------------------------
  Tag     Feed[FUEL_FEED_STACK];         // Room for 16 associations
  //----------------------------------------------------------
private:
  float   fuel_burned;      // Total burned for info

};
//========================================================================
//  CFuelCollector is a particulaer cell used as a fuel buffer
//========================================================================
class CFuelCollector: public CFuelCell {
  //----No specific attribute --------------------------------
public:
  CFuelCollector(U_CHAR ind);
  //----------------------------------------------------------
  inline float GetLimitQty()  {return 0;}
};
//========================================================================
// CFuelHeater
//========================================================================
class CFuelHeater : public CFuelSubsystem {
public:
  // Constructor
  CFuelHeater (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFuelHeater"; }

// Data members
protected:
};


//========================================================================
// CFuelMath
//========================================================================
class CFuelMath : public CFuelSubsystem {
public:
  // Constructor
  CFuelMath (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFuelMath"; }

// Data members
protected:
};


//=========================================================================
// CFuelPump
//=========================================================================
class CFuelPump : public CFuelSubsystem {
public:
  // Constructor
  CFuelPump (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  //------------------------------------------------------------
  bool  IsOFF();
  //------------------------------------------------------------
  inline char ByPass() {return (pass)?('Y'):('N');}
  //------------------------------------------------------------
  // CSubsystem methods
  const char* GetClassName (void) { return "CFuelPump"; }
  void TimeSlice (float dT,U_INT FrNo = 0);			// JSDEV*

protected:
  float setP;    ///< Pump pressure
  bool  pass;    ///< Allow fuel passthrough when system is off
  bool  enabled; ///< Enabled by default
};


//========================================================================
// CFuelSource
//========================================================================
class CFuelSource : public CFuelSubsystem {
public:
  // Constructor
  CFuelSource (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFuelSource"; }

protected:
};


//===========================================================================
// CFuelTank
//===========================================================================
class CFuelTank : public CFuelCell {
public:
  // Constructor
  CFuelTank (U_CHAR ind);

  virtual const char* GetClassName (void) { return "CFuelTank"; }

protected:
};

//========================================================================
// CEngineFuelPump
//========================================================================
class CEngineFuelPump : public CFuelSubsystem {
public:
  // Constructor
  CEngineFuelPump (void);
  bool  IsOFF();
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  //----------------------------------------------------------
  inline char ByPass()    {return (pass)?('Y'):('N');}
  // CSubsystem methods
  const char* GetClassName (void) { return "CEngineFuelPump"; }

protected:
  bool  pass;
};

//========================================================================
// CFuelManagement
//========================================================================
class CFuelManagement : public CFuelSubsystem {
public:
  // Constructor
  CFuelManagement (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CFuelManagement"; }

protected:
};

//========================================================================
// CAPUFuelSource
//========================================================================
class CAPUFuelSource : public CFuelSubsystem {
public:
  // Constructor
  CAPUFuelSource (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CAPUFuelSource"; }

protected:
};

//========================================================================
// * Simulation model subsystems
//========================================================================

//
// CAirModel
//
class CAirModel : public CSubsystem {
public:
  CAirModel (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CAirModel"; }
};

//
// CMixerModel
//
class CMixerModel : public CSubsystem {
public:
  CMixerModel (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CMixerModel"; }
};

//
// CWheelModel
//
class CWheelModel : public CSubsystem {
public:
  CWheelModel (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CWheelModel"; }
};
//===================================================================================
//  ENGINE SUBSYSTEM
//===================================================================================
class CEngineModel : public CDependent {
public:
  CEngineModel (void);
  virtual ~CEngineModel (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName    (void) { return "CEngineModel"; }

  // virtual functions for engine models
  virtual void        eTimeslice      () {;}
  virtual void        eCalculate      (float dT) {;}
  virtual void        SetBoosted      (void) {;}
  virtual void        setMinMaxMAP    (const double &min_inHg, const double &max_inHg) {;}
  virtual void        SetTrueRPM()    {return;}
  virtual void        Abort()         {;}
	virtual void				Reset()					{;}
  virtual void        Idle()          {;}
  //-----Plotting interface -----------------------------------------
  virtual int  AddToPlotMenu(char **menu, PLOT_PM *pm, int k){return 0;}
  virtual bool PlotParameters(PLOT_PP *pp,Tag id, Tag type) {return false;}
  //------------------------------------------------------------------
  virtual CDependent *GetPart(char k) {return this;}
  //------------------------------------------------------------------
  inline const float&        GetRRPM         (void) const {return rrpm;}
  inline const float&        GetIRPM         (void) const {return irpm;}
  //------------------------------------------------------------------
  inline bool   EngStopped()    {return eData->EngStopped();}
  inline bool   EngPowered()    {return eData->EngPowered();}
  inline bool   EngRunning()    {return eData->EngRunning();}
  inline bool   EngCranking()   {return eData->EngCranking();}
  inline bool   EngFueled()     {return eData->EngFueled();}
  inline float  GetERPM()       {return eData->e_rpm;}
  inline void   SetEngineData(CEngineData *d) {eData = d;}
  //------------------------------------------------------------------
protected:
  // NGN engine common values
  CEngineData   *eData;             // Engine data
  float vne;
  float irpm, srpm;
  float rbhp, rrpm;
  float nGt_;
  float cowT, cowD;
  CFmtxMap *mpoilp;
  CFmtxMap *mpoilt;

private:
  // external values
};


/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

 Author:       Jon S. Berndt
 Date started: 08/24/00

 ------------- Copyright (C) 2000  Jon S. Berndt (jsb@hal-pc.org) -------------

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

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
class CPropellerModel : public CEngineSubsystem {
  //--- ATTRIBUTES ------------------------------------------
protected:
  CVehicleObject *mveh;             // Parent vehicle
  //--- METHODS ---------------------------------------------
public:
  CPropellerModel (void);
  virtual ~CPropellerModel (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);
  virtual CDependent *GetPart() {return 0;}
  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPropellerModel"; }
	virtual void        Reset() {;}
  void                TimeSlice (float dT);
	
  /** Sets the Revolutions Per Minute for the propeller. Normally the propeller
      instance will calculate its own rotational velocity, given the Torque
      produced by the engine and integrating over time using the standard
      equation for rotational acceleration "a": a = Q/I , where Q is Torque and
      I is moment of inertia for the propeller.
      @param rpm the rotational velocity of the propeller */
  void SetRPM                     (const double &rpm)  {RPM = rpm;}
  void SetMinRPM                  (const double &mrpm) {minRPM = mrpm;}
	void SetBladePitch(float p)			{blad_pitch = eData->e_blad; }
  //---Functions that are specific to propeller instance -------------------
  virtual double GetPowerRequired (void)                    {return 0;}
  virtual double Calculate        (double PowerAvailable)   {return 0;}
  //---return all values ---------------------------------------------------
  int& GetBladeNum                       (void) {return blad;} 
  virtual float   GetAngularMomentum     (void) {return 0.0f;}
  virtual double  GetTorque              (void) {return 0.0f;}
  virtual const double& GetBladeAoA      (void) {return blade_AoA;}
  virtual const float&  GetThrustXDispl  (void) {return thrust_displ;}
  //------------------------------------------------------------------------
  inline float GetERPM()            {return eRPM;}
  //------------------------------------------------------------------------
protected:
  bool read_PRP_file_flag;
  char propeller_filename[PATH_MAX];
  //
  int mode, blad;
  float cord, diam, mnbd, mxbd, facT, facQ, iceR, idcf, pFac;

  // lift, drag, pirt, rift, dmge
  float lift, drag, pirt, rift, dmge;
  CFmtxMap *mlift;            // Lift table look up (fmt3)
  CFmtxMap *mdrag;            // drag table look up (fmt3)
  CFmtxMap *mpirt;            // prop table look up (fmt3)
  CFmtxMap *mrift;            // prop table look up (fmt3)
  CFmt1Map *mfacP;            // prop table look up (fmt3)
  CDamageModel  *pdmge;       // Damage model
  //----------------------------------------------------------------
  double Ixx;                 // Propeller inertia
  double GearRatio;           // Gear ratio
  double Pitch;               // Blade pitch
  double Advance;
  float  deltaT;
  double Thrust;
  double Torque;
  double PowerRequired;
  double RPM;
  double blade_AoA;
	double blad_pitch;
  double minRPM;
  bool   is_fixed_speed_prop;
  float  thrust_displ;        // displacement of thrust along the X vect (P fact)
  float  eRPM;
};

//=======================================================================
// CPropellerJSBSimModel // added
//
// Modified from JSBSim
//======================================================================

class CPropellerJSBSimModel : public CPropellerModel {
public:
  CPropellerJSBSimModel (CVehicleObject *v, int eNo);
  virtual ~CPropellerJSBSimModel (void);
  
  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPropellerJSBSimModel"; }
  /** Retrieves the power required (or "absorbed") by the propeller -
      i.e. the power required to keep spinning the propeller at the current
      velocity, air density,  and rotational rate. */
  virtual double GetPowerRequired(void);
  /** Calculates and returns the thrust produced by this propeller.
      Given the excess power available from the engine (in foot-pounds), the thrust is
      calculated, as well as the current RPM. The RPM is calculated by integrating
      the torque provided by the engine over what the propeller "absorbs"
      (essentially the "drag" of the propeller).
      @param PowerAvailable this is the excess power provided by the engine to
      accelerate the prop. It could be negative, dictating that the propeller
      would be slowed.
      @return the thrust in pounds */
  virtual double Calculate(double PowerAvailable);

  // getters
  virtual float  GetAngularMomentum     (void) {return 0.0f;}
  virtual double GetTorque              (void) {return 0;}

private:
  int    numBlades;
  double Ixx;
  double Diameter;
  double MaxPitch;
  double MinPitch;
  double MinRPM;
  double MaxRPM;
  double P_Factor;
  double Sense;
  double Pitch;
  double Advance;
  double ExcessTorque;
  double GearRatio;
  double ThrustCoeff;
  
  //  FGColumnVector3 vTorque;
  //  FGTable *cThrust;
  //  FGTable *cPower;
  //  void Debug(int from);
};
//==========================================================================
/*!
 * CPropellerTRIModel //  added
 * defined in the .PRP file
 * Modified from JSBSim
 */
//==========================================================================
class CPropellerTRIModel : public CPropellerModel {
public:
  CPropellerTRIModel                 (CVehicleObject *v,int eNo);
  virtual ~CPropellerTRIModel        (void);
  
  virtual void  ReadFinished         (void);
 
  virtual const char* GetClassName   (void) { return "CPropellerTRIModel"; }
  virtual double GetPowerRequired    (void);
  virtual double Calculate           (double PowerAvailable);

  // getters
  virtual float  GetAngularMomentum  (void);                      // pound ft2/s
  virtual double GetTorque           (void) {return Torque;}      // pound-force foot
  float          GetAngMomentumKgM2s (void);                      // Kg m2/s
	//--------------------------------------------------------------------
	void	 Reset();
  //--------------------------------------------------------------------
  void   Probe(CFuiCanva *cnv);
  //--------------------------------------------------------------------
  inline CDependent *GetPart() {return this;}
  //--------------------------------------------------------------------
private:
  double PwrAva;                    // Power available to display
  double MinRPM;
  double MaxRPM;
  double Sense;
  double ExcessTorque;
  double ThrustCoeff;
  double blade_speed;
  double TotalDrag;
  double TotalLift;
  double PfcK;
  double CQ;
  double CT;
  double magic_number;
  float  tmp_lift;
  float  tmp_drag;
};


/*! The piston class is adapted from Jon S. Berndt work:
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

 Author:       Jon S. Berndt
 Date started: 09/12/2000

 ------------- Copyright (C) 2000  Jon S. Berndt (jsb@hal-pc.org) --------------

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
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable:4512 )
#endif


#ifdef _MSC_VER
#pragma warning( pop )
#endif


/*!
 *
 *  CTurbopropEngineModel
 *
 *  This class acts as a CEngineModel specialisation for turboprop engines
 */
class CTurbopropEngineModel : public CEngineModel {
public:
  CTurbopropEngineModel (CVehicleObject *v,int eno);
  virtual ~CTurbopropEngineModel (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTurbopropEngineModel"; }
  virtual void        TimeSlice (float dT,U_INT FrNo = 0);			// JSDEV*

protected:
  bool read_file_flag; // avoid multiple reading of NGN file

  CPropellerModel   *prop;      ///< Propeller model
  
  float runupTimeK;             ///< Runup time constant
  float rundownTimeK;           ///< Rundown time constant
  float starterTimeK;           ///< Starter time constant
  float spoolTimeK;             ///< Spool time constant
  float betaRange;              ///< Beta range
  float betaBrake;              ///< Beta brake (hp)
  float betaHigh;               ///< Maximum blade angle (beta range forward) (degrees)
  float betaMid;                ///< Mid blade angle (beta range zero) (degrees)
  float betaLow;                ///< Minimum blade angle (beta range reverse) (degrees)
  float designRPM;              ///< Design RPM
  float designInletTemp;        ///< Design inlet temperature
  float designInletMach;        ///< Design inlet Mach number
  float designInletPressure;    ///< Design inlet pressure
  float conditionerLow;         ///< Power conditioner (low)
  float conditionerHigh;        ///< Power conditioner (high)
  float lowIdleN1;              ///< Low idle N1
  float highIdleN1;             ///< High idle N1
  float lowMaxN1;               ///< Low max N1
  float highMaxN1;              ///< High max N1
  float governorGain;           ///< Governor gain
  float governorLimit;          ///< Governor limit
  float designCombustionTemp;       ///< Design combustion temperature
  float designHeatingValue;         ///< Design heating value for fuel
  float designAirMassFlow;          ///< Design air mass flow
  float designCompressorRatio;      ///< Design compressor pressure ratio
  float designDiffuserRatio;        ///< Design diffuser pressure ratio
  float designBurnerRatio;          ///< Design burner pressure ratio
  float designNozzlePressureRatio;  ///< Design nozzle pressure ratio
  float designHPTPressureRatio;     ///< Design high pressure turbine pressure ratio
  float designHPTTempRatio;         ///< Design high pressure turbine temperature ratio
  float compSHR;                    ///< Compressor specific heat ratio
  float turbSHR;                    ///< Turbine specific heat ratio
  float compSHRConstPressure;       ///< Compressor specific heat ratio constant pressure
  float turbSHRConstPressure;       ///< Turbine specific heat ratio constant pressure
  float compEff;                    ///< Compressor efficiency
  float turbEff;                    ///< Turbine efficiency
  float mechEff;                    ///< Mechanical efficiency
  float burnEff;                    ///< Burner efficiency
};

/*!
 *
 *  CTurbofanEngineModel
 *
 *  This class acts as a CEngineModel specialisation for turbofan engines
 */
class CTurbofanEngineModel : public CEngineModel {
public:
  CTurbofanEngineModel (CVehicleObject *v,int eNo);
  virtual ~CTurbofanEngineModel (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTurbofanEngineModel"; }
  virtual void        TimeSlice (float dT,U_INT FrNo = 0);			// JSDEV*

protected:
  bool read_file_flag; // avoid multiple reading of NGN file
};
//=====================================================================================
/*!
 *
 *  CTurbineEngineModel
 *
 *  This class acts as a CEngineModel specialisation for turbine engines
 */
//=====================================================================================
class CTurbineEngineModel : public CEngineModel {
public:
  CTurbineEngineModel (CVehicleObject *v,int eNo);
  virtual ~CTurbineEngineModel (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTurbineEngineModel"; }
  virtual void        TimeSlice (float dT,U_INT FrNo = 0);		// JSDEV*

protected:
  bool read_file_flag; // avoid multiple reading of NGN file
};

/*!
 *
 *  CTurboshaftAPUEngineModel
 *
 *  This class acts as a CEngineModel specialisation for APU
 */
class CTurboshaftAPUEngineModel : public CEngineModel {
public:
  CTurboshaftAPUEngineModel (CVehicleObject *v, int eNo);
  virtual ~CTurboshaftAPUEngineModel (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CTurboshaftAPUEngineModel"; }
  virtual void        TimeSlice (float dT, U_INT FrNo = 0);		// JSDEV*

protected:
  bool read_file_flag; // avoid multiple reading of NGN file
};

/*!
 *
 *  CSimplisticJetEngineModel
 *
 *  This class acts as a CEngineModel specialisation for turbine engines
 */
class CSimplisticJetEngineModel : public CEngineModel {
public:
  CSimplisticJetEngineModel (CVehicleObject *v, int eNo);
  virtual ~CSimplisticJetEngineModel (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CSimplisticJetEngineModel"; }
  virtual void        TimeSlice (float dT,U_INT FrNo = 0);		// JSDEV*

protected:
  bool read_file_flag; // avoid multiple reading of NGN file
};



//
// CWingModel
//
class CWingModel : public CSubsystem {
public:
  CWingModel (void);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CWingModel"; }
};
//===========================================================================
// *
// * Pitot-Static system objects defined in the .PSS file
// *

//
// Pitot-Static Port
//
//===========================================================================
class CPitotStaticPort : public CDependent {
  //--- ATTRIBUTEs ---------------------------------------------
protected:
  CVehicleObject *mveh;
  //--- METHODs ------------------------------------------------
public:
  CPitotStaticPort (CVehicleObject *v);

  // CStreamObject members
  virtual int   Read (SStream *stream, Tag tag);

  // CSubsystem methods
  virtual const char* GetClassName (void) { return "CPitotStaticPort"; }
  virtual EMessageResult  ReceiveMessage (SMessage *msg);
  virtual void            TimeSlice (float dT,U_INT FrNo = 0);		// JSDEV*

  const double& ComputePitotPort (const double &rho, const double &pr, const SVector &vt);

protected:
  EPortType type;   // PITOT or STATIC port type
  SVector   bPos;   // Location relative to aircraft center of gravity
  int     grup;     // Group number
  EPortFace face;   // LEFT or RIGHT facing
  bool    inie;     // Interior port?
  bool    wont;     // Port won't ice up
  int     stat;     // Port Enabled? (1=Yes|0=No) default=yes

  double val;
};


//=================================================================
//  Circular timer used for animating bitmap
//=================================================================
class CCircularTimer: public CDependent {
  //---Attribute ----------------------------------------
  U_INT modulo;
  float maxT;                           // Period
  float Time;                           // Current time
  U_INT beat;                           // Beat per second
  //------------------------------------------------------
public:
  CCircularTimer();
  virtual const   char* GetClassName (void) { return "CCircularTimer"; }
  virtual int     Read (SStream *stream, Tag tag);
  virtual void    TimeSlice(float dT,U_INT FrNo);
  EMessageResult  ReceiveMessage (SMessage *msg);
  //-------------------------------------------------------
  void  Probe(CFuiCanva *cnv);
};

//===================================================================
class CheckChapter;
struct D2R2_ACTION;
//===================================================================
//  CheckList Subsystem
//===================================================================
class PlaneCheckList: public CSubsystem {
  //---ATTRIBUTES ----------------------------------------------
protected:
	//--- Registered window --------------------------------------
  CFuiCkList *cWIN;
	//---  Autorobot ---------------------------------------------
  CRobot *d2r2;																	// Robot location
  std::vector<CheckChapter*>   vCHAP;						// Table of Chapters
  //---METHODS--------------------------------------------------
public:
  PlaneCheckList(CVehicleObject *v);
 ~PlaneCheckList();
  //------------------------------------------------------------
  void    OpenList(char *tail);
  int     Read(SStream *st,Tag tag);
  char  **GetChapters();
  void    GetLines(CListBox &box,U_INT ch);
  void    Close();
	void    RegisterWindow(CFuiCkList *w);
	bool    Execute(D2R2_ACTION &a);
	//------------------------------------------------------------
	void		AutoStart();
  //------------------------------------------------------------
  EMessageResult ReceiveMessage (SMessage *msg);
  //------------------------------------------------------------
  inline  bool HasChapter()		{return (vCHAP.size() != 0);}
	//------------------------------------------------------------

};

//==================================================================
// CDLLSubsystem
//==================================================================
class CDLLSubsystem : public CSubsystem {
public:
  bool enabled;
  void *dll;
  CDLLSubsystem (void);
  virtual ~CDLLSubsystem (void);
  // CStreamObject methods
	virtual int		Read (SStream *stream, Tag tag);
	virtual void	ReadFinished (void);
	//--------------------------------------------------------------
  // CSubsystem methods
  //--------------------------------------------------------------
  virtual	void  TimeSlice (float dT,U_INT FrNo);
  //-----------funct members -------------------------------------
  void SetObject (SDLLObject *object);
  void SetSignature (const long &sign);
  void Prepare (void);
  const SDLLObject* Get_Object (void) {return obj;}
  const long& GetSignature (void) {return signature;}
  //-----------attributes ----------------------------------------
private:
  SDLLObject *obj;
  long signature;
};
//====================END OF FILE ================================
#endif // SUBSYSTEMS_H
