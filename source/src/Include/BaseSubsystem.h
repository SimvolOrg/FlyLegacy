//==============================================================================================
// Version.h
//
// Part of Fly! Legacy project
//
// Copyright 2003 Chris Wallace
// Copyright 2005 Jean Sabatier
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
//
//================================================================================================
#include "../Include/Queues.h"
#include "../Include/3DMath.h"
//================================================================================================

#ifndef BASE_SUBSYSTEM_H
#define BASE_SUBSYSTEM_H
class CFuiCanva;
class CSoundBUF;
class CFuiPlot;
class CRobot;
class CAeroControl;
class CheckChapter;
//==================================================================================
#define DEP_MAIN_STATE (0x01)
#define DEP_AUXI_STATE (0x02)
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
#define SUBSYSTEM_SPEED_REGULATOR					('sREG')	// CSpeedRegulator

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
#define SUBSYSTEM_GPS_RADIO								('gpsr')  // CK89radio
#define SUBSYSTEM_GPS_BX_KLN89						('k89g')	// Bendix King KLN89
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
	INDN_SINUSOID			= 4,
	INDN_IDLE					= 5,
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
  MONITOR_BETW,
	MONITOR_AS,
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
//  WHEEL POSITION
//====================================================================================
typedef enum {
  GEAR_EXTENDED   = 0,
  GEAR_RETRACTED  = 1,
  GEAR_IN_TRANSIT = 2,
}EGearPosition;

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
//
// Top-level parent class for all vehicle subsystems.
//
// CSubsystem is the top-level abstract parent for all electrical subsystem
//   classes.  Every CSubsystem has an "indication" attribute stored in the 'indn'
//   member.  The interpretation of the indication is completely arbitrary based
//   on the specific CSubsystem subclass.  For example, the indn for a CVoltmeter
//   object would be the voltage level of the circuit it was connected to; the indn
//   value for a CAltimeter object would represent the vehicle barometric altitude.
// Every subsystem has a pointer to the mother vehicle
// Every CSubsystem instance MUST have a unique Tag identifier assigned to it in
//   the object specification.
// All CSubsystem objects can be configured to have the indication value lag behind
//   the current setting through the 'timK' and 'ratK' values.
//
//==========================================================================================
//  NOTE:  All subsystems has a pointer to the parent vehicle
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
	//----------------------------------------------------------------------------
	EMessageResult Send_Message (SMessage *msg);
	//--------------------------------------------------------------------------------------------
  virtual EMessageResult  ReceiveMessage (SMessage *msg);
  virtual void  SetGroup (int group) { gNum = group; }
  virtual	void  TimeSlice (float dT,U_INT FrNo);				    // JSDEV*   new TimeSlice
  virtual void  Print (FILE *f);
  virtual void  Draw()  {}                                  // For external subsystems
	//---------------------------------------------------------------------------
	virtual void	SetActive(char a)	{;}
	virtual void	SetState (char s)	{;}
	//--- GAUGE BUS -------------------------------------------------------------
	//	The GAUGE BUS is for gauge.  A Subsystem update its own value and
	//		when the gauge has a pointer to the subsystem, it can retrieve the value
	//		much faster than through the messaging system
	//---------------------------------------------------------------------------
	//	Each bus is dedicated to one value . For instance GaugeBusFT01() retrieve 
	//	the indn value of the subsystem.
	//	Subsystem must implement the virtual function returning specific
	//	value when the value is not the standard one.
	//	BUS of the name INxx must return integer value
	//	BUS of the name FTyy must return a float value
	//---------------------------------------------------------------------------
	virtual int   GaugeBusINNO(char no)	{return 0;}				// Get data bus number no
	virtual int   GaugeBusIN01()				{return 0;}				// State
	virtual int   GaugeBusIN02()				{return 0;}				// Activity
	virtual int   GaugeBusIN03()				{return 0;}				// Index
	//---------------------------------------------------------------------------
	virtual	int   GaugeBusIN04()				{return 0;}				// Specific to systems
	virtual int   GaugeBusIN05()				{return 0;}				// Specific to system
	virtual int   GaugeBusIN06()				{return 0;}				// Specific to system
	virtual int   GaugeBusIN07()				{return 0;}				// Specific to system
	virtual int   GaugeBusIN08()				{return 0;}				// Specific to system
	//---------------------------------------------------------------------------
	virtual float GaugeBusFT01()				{return indn;}		// Float p1 (indn)
	virtual float GaugeBusFT02()				{return 0;}				// Float p2
	//---------------------------------------------------------------------------
	virtual void	Target(float v)		{indnTarget = v;}
  //---------------------------------------------------------------------------
  void  TraceTimeSlice(U_INT FrNo);					        // JSDEV*	Trace activation
  void  SetIdent(Tag id);
  void  SetTimK(float t);
	//--- Synchro management ----------------------------------------------------
  virtual void  SetGauge(CGauge *g)	{;}
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
	inline float  IndnValue()		{return indn;}
	//-------------------------------------------------------------------------
	inline CVehicleObject *GetMVEH() {return mveh;}
	//----Probe management ----------------------------------------------------
  inline void   SetOption(char p) {popt = p;}
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
	//--- Hardware identier ---------------------------------------------------
  EMessageHWType  hwId;
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
	inline  bool		IsOff()				{return (on == 0);}
	inline  bool		IsOn()				{return (on != 0);}
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
  void  GetAllDependents(CFuiProbe *win);
	void	UpdateState(float dT,U_INT FrNo);	
  void  Poll_AND();
  void  Poll_OR_();
	void	SendAllPxy0(void);									
	void	SendAllPxy1(void);									
	void	TraceActivity(U_INT FrNo,SMessage *msg,int act);	// Trace activity
	//-------------------------------------------------------------------------------------------
	void	SetState(char s);
	void	SetActive(char a)							{active	= (a != 0);}
	//--- GAUGE BUS -----------------------------------------------------------------------------
	int		GaugeBusIN01()								{return state;}
	int   GaugeBusIN02()								{return active;}
	int		GaugeBusIN03()								{return indx;}
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
	char					oper;				// Operator
  //---indicators --------------------------------------------
  char          splay;      // Sound is playing
  char          nDPND;      // Number of dependents
  char          nPXY0;      // Number of proxy 0
  char          nPXY1;      // Number of proxy 1
  int           indx;       // Index value
  //------- Proxy messages for polling ----------------------
  SMessage        mpol; // Message used for polling
  float           rise; // Threshold for rising edge proxy
  float           fall; // Threshold for falling edge proxy
};
//====================================================================================
// PID controller for Autopilot
//  A PID controller monitors a value (aircraft heading for instance)against a
//  reference value and supplies an output value used to act on a control surface
//  Autopilot are built by cascading a set of PID
//  This particular PID controler is based on Flightgear PID controller
//  Thanks to Roy Ovesen for permission to use his formulea
//  It uses the following inputs ---------------------------------------------
//    Yn  is the sampled value to control at step n
//    Rn  is the target  value to reach at step n
//    Kp  is the proportional gain coefficient
//    Kb  is the proportional reference weighing factor
//    Kc  is the derivative reference weighing factor
//    Ti  integrator timing (sec)
//    Td  Derivator timing (sec)
//====================================================================================
class CPIDbox: public CDependent,public CqItem  {
  //---ATTRIBUTS -----------------------------------------
  U_SHORT  nPid;                      // Pid Index
  U_CHAR   drvt;                      // Use derivative term
  U_CHAR   intg;                      // Use integrator term
  U_CHAR   anti;                      // use anti saturation
  double   rate;                      // Output level
  AutoPilot *apil;              // Autopilot
  //-------------------------------------------------------
  static Tag PidIDENT[];
  //---Controller input values ------------------------------------
  double  Ts;                         // Time between sample
  double  Yn;                         // Sample of value
  double  Rn;                         // Target value
  double  Kp;                         // Proportional gain
  double  Ti;                         // Integrator Timing
  double  Td;                         // Derivator Timing
  double  Ta;                         // Antisaturator timer
  double  Ki;                         // Integrator term
  double  Ks;                         // Saturator term
  double  es;                         // Saturator error
  double  Kd;                         // Derivative term
  //---Saturation values ---------------------------------
  double  vmin;                       // Minimum output
  double  vmax;                       // Maximum output
  //------------------------------------------------------
  double  en;         // error step n
  double  en1;        // error step n-1
  double  en2;        // error step n-2
  double  eSum;       // error sum
  double  yPrv;       // Y at previous step
  double  iMax;       // Integrator limit
  double  vSat;       // Saturation value
  //------------------------------------------------------
  double  Un;         // Controller output 
  double  Vn;         // Clamped output
  //---Methodds ------------------------------------------
public:
  CPIDbox(U_CHAR No,AutoPilot *ap);
 ~CPIDbox();
  //------------------------------------------------------
  void      Probe(CFuiCanva *cnv);
  //------------------------------------------------------
  int       Read(SStream *st,Tag tag);
  void      ReadFinished();
  void      SetMsgInp(SMessage *msg,Tag idn, Tag prm);
  double    Integrate(double kd);
  void      SetDTIME(double t);
  void      SetITIME(double t);
  void      SetATIME(double t);
  //-------------------------------------------------------
  void      Init();
	void      SetCoef(double kp,double ti,double td);
  void      TrakSaturation(double val,double minv, double maxv);
  double    Update(float Dt,double Y,double R);
  double    GetValue(Tag pm);
  double    Clamp(double v);
  void      SetValue(Tag pm,double val);
  void      SetClamp(CAeroControl *c);
  //-------------------------------------------------------
  inline int  GetPidNo()          {return nPid;}
  inline void Rate(double r)      {rate = r;}
	//-------------------------------------------------------
	inline float	GetVN()						{return Vn;}
  //-----Set values ---------------------------------------
  inline void SetSample(double s) {Yn = s;}
  inline void SetTarget(double t) {Rn = t;}
	inline void ClearKI()						{Ki = 0;}
	inline void SetKP(double k)			{Kp = k;}
	inline void SetKI(double k)			{Ki = k;}
	inline void	SetKD(double k)			{Kd = k;}
  //-------------------------------------------------------
  inline void SetMaxi(double m)   {vmax = m;}
  inline void SetMini(double m)   {vmin = m;}
};
//=========================================================================================
//  CLASS CPIDQ
//        Class to collect list of PIDbox
//=========================================================================================
class CPIDQ : public CQueue {
  //---------------------------------------------------------------------
public:
  inline void Lock()                {pthread_mutex_lock (&mux);}
  inline void UnLock()              {pthread_mutex_unlock (&mux);}
  //----------------------------------------------------------------------
  //~CPIDQ();                        // Destructor
  //----------------------------------------------------------------------
  inline void PutEnd(CPIDbox *box)           {CQueue::PutEnd(box);}
  inline CPIDbox *Pop()                      {return (CPIDbox*)CQueue::Pop();}
  inline CPIDbox *GetFirst()                 {return (CPIDbox*)CQueue::GetFirst();}
  inline CPIDbox *GetNext(CPIDbox *box)      {return (CPIDbox*)CQueue::GetNext(box);}
};
//========================================================================================
// CDamageModel
//
// Damage model definition corresponding to <dmge> object in PRP or WNG files
//========================================================================================
class CDamageModel : public CStreamObject {
public:
  // Constructors / destructor
  CDamageModel (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

protected:
  float           period;         ///< Repair period (hours)
};

class CGroundSuspension;
//========================================================================================
//  struct used to share data between classes
//  
//  all the WHL file data
//========================================================================================
struct SGearData {
	CGroundSuspension  *mgsp;						// Mother suspension for shared data
	//------------------------------------------------------------------------
  char onGd;                          // On Ground
  char shok;                          // Shock number
  char Side;                          // Side of wheel (left or right)
	char sABS;													// ABS brake on landing
	char brak;                          // 0 not brake 1= brake
	char latK;													// Lateral coefficient
	//------------------------------------------------------------------------
  char susp_name[56];
  char long_[64];                    ///< -- Longitudinal Crash Part Name --
  CDamageModel oy_N;                 ///< -- Longitudinal Damage Entry --
  float stbl;                        ///< -- steering factor from data table
  float btbl;                        ///< -- braking factor from data table
  float powL;                        ///< -- Impact Power Limit (ft-lb/sec) (weight * velocity) --
	//-----------------------------------------------------------------------
  double imPW;                       // Impact power in ft-lb/sec
	double masR;											 // Mass repartition on this wheel
  double maxC;                       ///< -- max compression --
	//-----------------------------------------------------------------------
  CDamageModel oy_T;                 ///< -- Tire Damage Entry --
  // ntwt                            ///< -- Normal Tire Wear Rate vs Groundspeed (ktas) --
  // ltwt                            ///< -- Lateral Tire Wear Rate vs Lateral Groundspeed (ktas) --
  float boff;                        ///< -- Blowout Friction Factor --
  int   ster;                        ///< -- steerable wheel --
  float mStr;                        ///< -- max steer angle (deg) --
  float drag;                        ///< -- Drag coefficient
  float tirR;                        ///< -- Tire Radius (ft) --
  float rimR;                        ///< -- Rim Radius (ft) --
  float whrd;                         // Wheel radius (ft);
  CVector bPos;                       //< -- Tire Contact Point (model coodinates) --
  CVector gPos;                       //  Tire contact point (CG relative)
  std::vector<std::string> vfx_;      //< -- Visual Effects --
	//--- Wheel above ground level (feet) ------------------------------------
	double wagl;
	///-------Vertical damping for ground contact ----------------------------
  double damR;                        ///< -- damping ratio --
	double amor;												// Amortizing force
	double swing;												// Sinusoid value
	//---- Acceleration ------------------------------------------------------
	double angv;												// Angular velocity
  ///-------Brake parameters -----------------------------------------------
	double  xDist;											// Lateral distance to CG (meter)
	double	brakF;											// Brake force in (m/sec)
	double  ampBK;											// Brake amplifieer
	double  repBF;											// Brake repartition factor
  ///----- sterring wheel data ---------------------------------------------
  float deflect;
  float scaled;
  float kframe;												// Directional keyframe
	//------------------------------------------------------------------------
	float sframe;												// Shock absorber kframe
	///--- wheelBase (inter wheel lenght between axis in meters) -------------
  float wheel_base;

  SGearData (void) {
    onGd              = 0;          // On ground
    shok              = 0;          // Number of shocks
		sABS							= 0;					// No ABS
    strcpy (susp_name,  "");
    strcpy (long_,      "");         // -- Longitudinal Crash Part Name --
    // oy_N'                         // -- Longitudinal Damage Entry --
    stbl              = 1.0f;        // -- steering factor from data table
    btbl              = 1.0f;        // -- braking factor from data table
    powL              = 0.0f;        // -- Impact Power Limit (ft-lb/sec) (weight * velocity) --
    // oy_T                          // -- Tire Damage Entry --
    // ntwt                          // -- Normal Tire Wear Rate vs Groundspeed (ktas) --
    // ltwt                          // -- Lateral Tire Wear Rate vs Lateral Groundspeed (ktas) --
    boff              = 0.0f;        // -- Blowout Friction Factor --
    ster              = 0;           // -- steerable wheel --
    mStr              = 0.0f;        // -- max steer angle (deg) --
    maxC              = 0.0f;        // -- max compression --
		masR							= 0.33f;
    damR              = 0.0f;        // -- damping ratio --
		amor							= 0;
    tirR              = 0.0f;        // -- Tire Radius (ft) --
    rimR              = 0.0f;        // -- Rim Radius (ft) --
    drag              = 0.0f;
    // vfx_;                         // -- Visual Effects --
    // endf;                         //
    brak              = 0;            // -- has brake
    vfx_.clear ();
    deflect           = 0.0f;        // -- direction wheel deflection
    scaled            = 0.0f;        // -- direction wheel deflection scale
    kframe            = 0.5f;
    wheel_base        = 0.0f;        // -- distance from main gear and nose gear (metres)
		angv							= 0;					 // Angular velocity
		swing							= 0;
		xDist							= 0;
  }
};

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
  PlaneCheckList();
 ~PlaneCheckList();
	//------------------------------------------------------------
	
  void    Init(CVehicleObject *v,char *tail);
  int     Read(SStream *st,Tag tag);
  char  **GetChapters();
  void    GetLines(CListBox &box,U_INT ch);
  void    Close();
	void    RegisterWindow(CFuiCkList *w);
	bool    Execute(void *a);
  //------------------------------------------------------------
  EMessageResult ReceiveMessage (SMessage *msg);
  //------------------------------------------------------------
  inline  bool HasChapter()		{return (vCHAP.size() != 0);}
	//------------------------------------------------------------
};

//======================= END OF FILE ==============================================================
#endif // BASE_SUBSYSTEM_H
