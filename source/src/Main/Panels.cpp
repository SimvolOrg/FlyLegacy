/*
 * Panels.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
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

/*! \file Panels.cpp
 *  \brief Implements CPanel cockpit panel class
 *
 *  Each interior cockpit view (static or interactive) is encapsulated
 *    in a CPanel instance.  CPanel is derived from CStreamObject and
 *    parses a PNL file (a mandatory parameter to the constructor) that
 *    specifies the backdrop image, gauges, lights, etc. included on the panel. 
 */


#include "../Include/Globals.h"
#include "../Include/FlyLegacy.h"
#include "../Include/Panels.h"
#include "../Include/RadioGauges.h"
#include "../Include/Utility.h"
#include "../Include/Ui.h"
#include "../Include/Fui.h"
#include "../Include/CursorManager.h"
#include "../Include/TerrainTexture.h"
using namespace std;
//=============================================================================
#define PANEL_VBO_DIM (20)			// Room for 5 * 4 vertices
#define PANEL_VBO_DYN	(20)			// Room for 5 dynamic quads
#define PANEL_VBO_TOT (PANEL_VBO_DIM + PANEL_VBO_DYN)
//=============================================================================
// CPanelLight
//=============================================================================
CPanelLight::CPanelLight (void)
{ id = 0;
  ambientOverride = false;
  bn = 0;
	col[0] = col[1] = col[2] = col[3] = 1;
}
//------------------------------------------------------------------
//	Build Light with ident
//------------------------------------------------------------------
CPanelLight::CPanelLight (Tag t)
{ id = t;
  ambientOverride = false;
  bn = 0;
}
//------------------------------------------------------------------
CPanelLight::~CPanelLight (void)
{
}
//------------------------------------------------------------------
int CPanelLight::Read (SStream *stream, Tag tag)
{
  switch (tag) {
  case 'id__':
    // Light unique identifier
    ReadTag (&id, stream);
    return TAG_READ;

  case 'ambi':
    // Ambient light override
    ambientOverride = true;
    return TAG_READ;
    
  case 'msg_':
    // Message connection to light subsystem
    ReadMessage (&msg, stream);
    msg.sender   = 'panl';
    msg.realData = 0;
    return TAG_READ;
  }

  WARNINGLOG ("CPanelLight::Read : Unrecognized tag <%s>", TagToString(tag));
  return TAG_IGNORED;
}
//----------------------------------------------------------------------
//  Prepare the message
//----------------------------------------------------------------------
void CPanelLight::PrepareMsg (void)
{ 
  Send_Message (&msg);
}
//----------------------------------------------------------------------
//  Panel light:  Update with panel brightness
//----------------------------------------------------------------------
void CPanelLight::Update (float b)
{ Send_Message (&msg);
  bn = msg.realData;
	float lum = bn + b;
	col[0] = col[1] = col[2] = lum;
  return;
}
//================================================================================
// CPanel
//  Create the panel
//	The panel is assigned the default light with id = 0
//	
//=================================================================================
CPanel::CPanel (CCockpitManager *pn,Tag id, const char* filename)
{ this->id  = id;
	pit				= pn;						// Panel manager
	mveh			= pn->GetMVEH();
	main			= 0;
  strncpy (this->filename, filename,63);
  txOBJ   = 0;
  ngOBJ   = 0;
	trn1		= 1;
	skip		= 0;
	//--- Assign a default light --------------------
	dlite		= pn->GetLight(0);
	plite		= dlite;
	brit[3] = 1;
  //--- initialize dll_gauge ----------------------
  dll_gauge.clear ();
  //---- Initialize to window dimensions ----------
  xScreen = globals->cScreen->Width;
  yScreen = globals->cScreen->Height;

  //---- Initialize panel viewing parameters ------
  y_3Dxy = x_3Dxy = 0;
	x_3Dsz = xScreen;
  y_3Dsz = yScreen;

  x_isiz = y_isiz = 0;
  xDep  = yDep = 0;
  //---Gauge in focus --------------------------------
  gFocus    = 0;
  ca        = 0;
	//--- Init vector offset ---------------------------
	gOfs		= 4;							// Room for panel
	gBUF		= 0;							// Gauge buffer
	pVBO		= 0;
	gVBO		= 0;
	xlok		= 0;
	ylok		= 0;
	//--- Allocate a VBO for dynamic parts ----------------
	int dim = PANEL_VBO_TOT;								// Want 5 * 4 vertices
	pBUF		= new TC_VTAB[dim];
	dOfs		= PANEL_VBO_DIM;								// Offset to VBO dynamic
  //--------------------------------------------------
  // Initialize keep flag to false
  //   when creating/deleting non-permanent views
  // Initialize panel scrolling offsets
  xOffset = yOffset = 0;

  //--- Initialize mouse handling data members
  track = MOUSE_TRACKING_OFF;
  gtk = 0;
  buttonsDown = 0;
  // If the filename has a .PNL extension, then this is a dynamic panel,
  //   otherwise it is static

  SStream s;
	_snprintf(s.filename,(PATH_MAX-1),"DATA/%s",filename);
  strncpy (s.mode, "r",3);

  const char *p = strrchr (filename, '.');
  if (p && (stricmp (p, ".PNL") != 0)) {

    // Filename suffix is the screen height in pixels
    // for LEGACY, get the maximum resolution
    int wrkrnd = 768;
    // end workaround
    char ext[16];
    _snprintf (ext,15,".%d", wrkrnd);
		ext[15] = 0;
    strncat (s.filename, ext,4);
  }
	TRACE("Read panel %s",filename);
  if (OpenStream (&s)) {
    ReadFrom (this, &s);
    CloseStream (&s);
  }
  // -----Add a pointer to camera entry --------------
  CCameraCockpit *cam = (CCameraCockpit *)globals->ccm->GetCamera('cock');
  cam->SetPanel(id,this);
}
//==============================================================================
//  Destroy the panel
//==============================================================================
CPanel::~CPanel (void)
{ // Delete gauges
  std::vector<CGauge*>::iterator it;
  for (it=dgag.begin(); it!=dgag.end(); it++)  delete (*it);
	for (it=sgag.begin(); it!=sgag.end(); it++)  delete (*it);
  //--- Clear all vectors ----------------------------------------
  dgag.clear();
  gage.clear();
	sgag.clear();
  //---Remove texture objets -------------------------------------
  glDeleteTextures(1,&txOBJ);
  glDeleteTextures(1,&ngOBJ);
	glDeleteBuffers (1,&gVBO);
	glDeleteBuffers (1,&pVBO);
	if (gBUF)		delete [] gBUF;
	if (pBUF)		delete [] pBUF;
	//-------------------------------------------------------------
	for (U_INT k=0; k< dll_gauge.size(); k++) delete dll_gauge[k];
	dll_gauge.clear ();
}

//-----------------------------------------------------------
//	JSDEV* Prepare message for all the panel gauges
//-----------------------------------------------------------
void CPanel::PrepareMsg(CVehicleObject *veh)
{ // Prepare messaging connections for all gauge objects
  std::map<Tag,CGauge*>::iterator it;
	for (it = gage.begin(); it != gage.end(); it++)
	{	CGauge *ga = it->second;
		ga->PrepareMsg(veh);
	}
	return;
}
//-----------------------------------------------------------------------
//  Add a new gauge to this panel
//-----------------------------------------------------------------------
int CPanel::ProcessGauge(CGauge *g,SStream *s)
{ g->SetPanel(this);
  ReadFrom (g, s);
	if (skip)	delete g;
	else {	gage[g->GetID()] = g;
					dgag.push_back(g);
	}
	skip	= 0;
  return TAG_READ;
}

//-----------------------------------------------------------------------
//  Add an auxilliary gauge to this panel
//-----------------------------------------------------------------------
int CPanel::AuxilliaryGauge(CGauge *g,SStream *s)
{ g->SetPanel(this);
  ReadFrom (g, s);
	gage[g->GetID()] = g;
  sgag.push_back(g);
  return TAG_READ;
}
//-----------------------------------------------------------------------
//  Read a Gauge
//-----------------------------------------------------------------------
int CPanel::ReadGauge(SStream *stream, Tag tag)
    { Tag type;
      ReadTag (&type, stream);
      switch (type) {
      case GAUGE_BASE:
        // This is the unique tag of the abstract gauge base.  There should
        //   be no instances of this in any panel
        WARNINGLOG ("CPanel : Read gauge type 'gage'");
        return TAG_READ;

      case GAUGE_BITMAP:
        { CBitmapGauge *g = new CBitmapGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_NEEDLE:
        { CNeedleGauge *g = new CNeedleGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_NEEDLE_DUAL:
        { CTwoNeedleGauge *g = new CTwoNeedleGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_NEEDLE_TRIPLE:
        { CThreeNeedleGauge *g = new CThreeNeedleGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_DIGITAL_NEEDLE:
        { CDigitalNeedleGauge *g = new CDigitalNeedleGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_ALTIMETER:
        { CAltimeterGauge *g = new CAltimeterGauge(this);
          return ProcessGauge(g,stream); }


      case GAUGE_ALTIMETER_ROLLING:
        { CRollingAltimeterGauge *g = new CRollingAltimeterGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_HORIZON:
        { CHorizonGauge *g = new CHorizonGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_HORIZON_FD:
        { CFDHorizonGauge *g = new CFDHorizonGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_HORIZONTAL_BALL:
        { CHorizontalBallGauge *g = new CHorizontalBallGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_AIRSPEED:
        { CAirspeedGauge *g = new CAirspeedGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_VERTICAL_SPEED:
        { CVerticalSpeedGauge *g = new CVerticalSpeedGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_DIRECTIONAL_GYRO:
        { CDirectionalGyroGauge *g = new CDirectionalGyroGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_VACUUM:
        { CVacuumGauge *g = new CVacuumGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_SUCTION:
        { CSuctionGauge *g = new CSuctionGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_FUEL:
        { CFuelGauge *g = new CFuelGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_FUEL_FLOW:
        { CFuelFlowGauge *g = new CFuelFlowGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_OIL_PRESSURE:
        { COilPressureGauge *g = new COilPressureGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_OIL_TEMPERATURE:
        { COilTemperatureGauge *g = new COilTemperatureGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_TACHOMETER:
        { CTachometerGauge *g = new CTachometerGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_TACHOMETER_DUAL:
        { CDualTachometerGauge *g = new CDualTachometerGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_N1_TACHOMETER:
        { CN1TachometerGauge *g = new CN1TachometerGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_EXHAUST_GAS_TEMP:
        { CExhaustGasTemperatureGauge *g = new CExhaustGasTemperatureGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_BASIC_COMPASS:
        { CBasicCompassGauge *g = new CBasicCompassGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_AMMETER:
        { CAmmeterGauge *g = new CAmmeterGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_ANALOG_CLOCK:
        { CAnalogClockGauge *g = new CAnalogClockGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_DIGITAL_CLOCK_FLYHAWK:
        { CHawkClockGauge *g = new CHawkClockGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_DIGITAL_CLOCK_BASIC:
        { CBasicDigitalClockGauge *g = new CBasicDigitalClockGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_ADF_BASIC:
        { CBasicADFGauge *g = new CBasicADFGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_DIGITAL_READOUT:
        { CDigitalReadoutGauge *g = new CDigitalReadoutGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_DIGITS_READOUT:
        { CDigitsReadoutGauge *g = new CDigitsReadoutGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_CABIN_PRESSURE_RATE:
        { CCabinPressureRateKnobGauge *g = new CCabinPressureRateKnobGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_CABIN_PRESSURE:
        { CCabinPressureGauge *g = new CCabinPressureGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_BRAKE_PRESSURE:
        { CBrakePressureGauge *g = new CBrakePressureGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_AOA:
        { CAOANeedleGauge *g = new CAOANeedleGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_TURN_COORDINATOR:
        { CTurnCoordinatorGauge *g = new CTurnCoordinatorGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_SLIP_INDICATOR:
        { CSlipIndicatorGauge *g = new CSlipIndicatorGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_HOBBS_METER:
        { CHobbsMeterGauge *g = new CHobbsMeterGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_NAVIGATION:
        { CNavigationGauge *g = new CNavigationGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_NAV_COMM_KX155:
        { CK155gauge *g = new CK155gauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_GPS_KLN89:
        { CK89gauge *g = new CK89gauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_XPDR_KT76:
        { CKT76gauge *g = new CKT76gauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_ADF_KR87:
        { CKR87gauge *g = new CKR87gauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_AUTOPILOT_KAP140:
        { CBKAP140Gauge *g = new CBKAP140Gauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_AUTOPILOT_KFC200:
        { CBKKFC200Gauge *g = new CBKKFC200Gauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_AUTOPILOT_KFC150:
        { CBKKFC150Gauge *g = new CBKKFC150Gauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_AUDIO_KMA26:
        { CBKAudioKMA26Gauge *g = new CBKAudioKMA26Gauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_HSI:
        { CHSIGauge *g = new CHSIGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_FLYHAWK_ELT:
        { CFlyhawkELTGauge *g = new CFlyhawkELTGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_NAV_RADIO_GENERIC:
        { CGenericNavRadioGauge *g = new CGenericNavRadioGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_COMM_RADIO_GENERIC:
        { CGenericComRadioGauge *g = new CGenericComRadioGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_TRANSPONDER_GENERIC:
        { CGenericTransponderGauge *g = new CGenericTransponderGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_ADF_GENERIC:
        { CGenericADFRadioGauge *g = new CGenericADFRadioGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_RMI_NEEDLE:
        { CRMINeedleGauge *g = new CRMINeedleGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_RADIO_GAUGE_GROUP:
        { CRadioGaugeGroup *g = new CRadioGaugeGroup(this);
          return ProcessGauge(g,stream); }

      case GAUGE_NAV_GAUGE_GROUP:
        { CNavGaugeGroup *g = new CNavGaugeGroup(this);
          return ProcessGauge(g,stream); }

      case GAUGE_ANNUNCIATOR:
        { CAnnunciator *g = new CAnnunciator(this);
          return ProcessGauge(g,stream); }

      case GAUGE_FLYHAWK_ANNUNCIATOR:
        { CFlyhawkAnnunciator *g = new CFlyhawkAnnunciator(this);
          return ProcessGauge(g,stream); }

      case GAUGE_FLYHAWK_ANNUNCIATOR_TEST:
        { CFlyhawkAnnunciatorTest *g = new CFlyhawkAnnunciatorTest(this);
          return ProcessGauge(g,stream); }

 //     case GAUGE_FLYHAWK_NAV_GPS_PANEL:
 //       { CFlyhawkNavGpsPanelGauge *g = new CFlyhawkNavGpsPanelGauge(this);
 //         return ProcessGauge(g,stream); }

      case GAUGE_FLYHAWK_FUEL_SELECTOR:
        { CFlyhawkFuelSelectorGauge *g = new CFlyhawkFuelSelectorGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_ELEVATOR_TRIM:
        { CFlyhawkElevatorTrimGauge *g = new CFlyhawkElevatorTrimGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_MALIBU_CHT_INDICATOR:
        { CMalybooCHTIndicatorGauge *g = new CMalybooCHTIndicatorGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_MALIBU_CHT_BUTTON:
        { CMalybooCHTButtonGauge *g = new CMalybooCHTButtonGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_MALIBU_NAV_GPS_PANEL:
        { CMalybooNavGpsPanelGauge *g = new CMalybooNavGpsPanelGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_MALIBU_DIGITAL_CLOCK:
        { CMalybooDigitalClockGauge *g = new CMalybooDigitalClockGauge(this);
          return ProcessGauge(g,stream); }

//      case GAUGE_MALIBU_FUEL_SELECT:
//        {
//          CMalybooFuelSelectGauge *g = new CMalybooFuelSelectGauge;
//          return ProcessGauge(g,stream); }

      case GAUGE_MALIBU_ENGINE_MONITOR:
        { CMalybooEngineMonitorGauge *g = new CMalybooEngineMonitorGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_NAVAJO_FUEL_SELECTOR:
        { CNavajoFuelSelectorGauge *g = new CNavajoFuelSelectorGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_NAVAJO_AMMETER:
        { CNavajoAmmeterGauge *g = new CNavajoAmmeterGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_NAVAJO_RADAR_ALTIMETER:
        { CNavajoRadarAltimeterGauge *g = new CNavajoRadarAltimeterGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_COLLINS_EFIS_SELECTION:
        { CCollinsEFISSelectionGauge *g = new CCollinsEFISSelectionGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_COLLINS_VSPEED_PANEL:
        { CCollinsVspeedPanel *g = new CCollinsVspeedPanel(this);
          return ProcessGauge(g,stream); }

      case GAUGE_COLLINS_HF_RADIO:
        { CCollinsHFRadioGauge *g = new CCollinsHFRadioGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_COLLINS_PFD:
        { CCollinsPFDGauge *g = new CCollinsPFDGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_COLLINS_ND:
        { CCollinsNDGauge *g = new CCollinsNDGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_COLLINS_RTU:
        { CCollinsRTUGauge *g = new CCollinsRTUGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_COLLINS_AD:
        { CCollinsAirDataGauge *g = new CCollinsAirDataGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_COLLINS_MND:
        { CCollinsMNDGauge *g = new CCollinsMNDGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_UNIVERSAL_FMS:
        { CUniversalFMSGauge *g = new CUniversalFMSGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_PILATUS_EFIS_PANEL:
        { CPilatusEFISPanel *g = new CPilatusEFISPanel(this);
          return ProcessGauge(g,stream); }

      case GAUGE_WALKER_TEMP_AIRSPEED:
        { CWalkerTempAirspeedGauge *g = new CWalkerTempAirspeedGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_GYRO_SLAVING:
        { CGyroSlavingGauge *g = new CGyroSlavingGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_VERTICAL_RATE:
        { CVerticalRateGauge *g = new CVerticalRateGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_PROP_SYNC:
        { CPropSyncGauge *g = new CPropSyncGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_DME:
        { CDMEGauge *g = new CDMEGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_DME_2:
        { CDME2Gauge *g = new CDME2Gauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_WEATHER_RADAR:
        { CWeatherRadarGauge *g = new CWeatherRadarGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_ANALOG_ALT_PRESELECT:
        { CAnalogAltPreselectGauge *g = new CAnalogAltPreselectGauge(this);
          return ProcessGauge(g,stream); }

//      case GAUGE_TCAS:
//        {
//          CTCASGauge *g = new CTCASGauge;
//          return ProcessGauge(g,stream); }

      case GAUGE_HOTSPOT:
        { CMomentaryHotSpotGauge *g = new CMomentaryHotSpotGauge(this);
          return ProcessGauge(g,stream); }
     
      case GAUGE_SIMPLE_IN_OUT:
        { CSimpleInOutStateSwitch *g = new CSimpleInOutStateSwitch(this);
          return ProcessGauge(g,stream); }

      case GAUGE_LIT_IN_OUT:
        { CLitInOutSwitch *g = new CLitInOutSwitch(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_SIMPLE_SWITCH:
        { CSimpleSwitch *g = new CSimpleSwitch(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_BATTERY_SWITCH:
        { CBasicBatterySwitch *g = new CBasicBatterySwitch(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_TWO_STATE_ELT:
        { CTwoStateELTGauge *g = new CTwoStateELTGauge(this);
          return ProcessGauge(g,stream); }

      case GAUGE_SWITCH_PAIR:
        { CSwitchPairGauge *g = new CSwitchPairGauge(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_SIMPLE_FLOAT_SWITCH:
        { CSimpleFloatSwitch *g = new CSimpleFloatSwitch(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_DUAL_SWITCH:
        { CDualSwitch *g = new CDualSwitch(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_PUSH_PULL_KNOB:
        { CPushPullKnobGauge *g = new CPushPullKnobGauge(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_PRIMER_KNOB:
        { CPrimerKnobGauge *g = new CPrimerKnobGauge(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_FLAP:
        { CFlapsSwitchGauge *g = new CFlapsSwitchGauge(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_LIT_LANDING_GEAR_KNOB:
        { CLitLandingGearKnobGauge *g = new CLitLandingGearKnobGauge(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_MIXTURE_KNOB:
        { CPushPullKnobGauge *g = new CPushPullKnobGauge(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_REVERSIBLE_THROTTLE_KNOB:
        { CReversibleThrottleKnobGauge *g = new CReversibleThrottleKnobGauge(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_DUAL_KNOB:
      case GAUGE_DOUBLE_LIGHT_KNOB:
        { CDualKnobGauge *g = new CDualKnobGauge(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_TURN_KNOB:
        { CTurnKnobGauge *g = new CTurnKnobGauge(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_MAGNETO_SWITCH:
        { CBasicMagnetoSwitch *g = new CBasicMagnetoSwitch(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_COVERED_SWITCH:
        { CCoveredSwitch *g = new CCoveredSwitch(this);
          return ProcessGauge(g,stream); }

      case GAUGE_COVERED_LIGHT_BUTTON:
        { CCoveredLightButtonGauge *g = new CCoveredLightButtonGauge(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_VERTICAL_SLIDER:
        { CVerticalSliderGauge *g = new CVerticalSliderGauge(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_TRIM_WHEEL:
        { CTrimWheelGauge *g = new CTrimWheelGauge(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_TRIM_WHEEL_INDICATOR:
        { CTrimWheelIndicatorGauge *g = new CTrimWheelIndicatorGauge(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_INDICATOR:
        { CIndicatorGauge *g = new CIndicatorGauge(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_FLAPS_INDICATOR:
        { CFlapsIndicatorGauge *g = new CFlapsIndicatorGauge(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_PUSHBUTTON:
        { CPushButtonGauge *g = new CPushButtonGauge(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_LIGHT_BUTTON:
        { CLightButtonGauge *g = new CLightButtonGauge(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_TURN_SWITCH:
        { CTurnSwitchGauge *g = new CTurnSwitchGauge(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_MASTER_CAUTION_BUTTON:
        { CMasterCautionWarningButtonGauge *g = new CMasterCautionWarningButtonGauge(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_KINGAIR_FIRE_EXT:
        { CKAFireExtinguisherButton *g = new CKAFireExtinguisherButton(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_WALKER_AP_PANEL:
        { CWalkerAPPanel *g = new CWalkerAPPanel(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_WALKER_FIRE_PANEL:
        { CWalkerFirePanel *g = new CWalkerFirePanel(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_LIGHT_SWITCH_SET:
        { CLightSwitchSetGauge *g = new CLightSwitchSetGauge(this);
          return ProcessGauge(g,stream); }
      
      case GAUGE_ROCKER_SWITCH:
        { CRockerSwitch *g = new CRockerSwitch(this);
          return ProcessGauge(g,stream); }
      //case 'S__G':
      //  { CDLLGauge *g = new CDLLGauge;
      //    return ProcessGauge(g,stream); }

      default:
        { // try for DLL gauges
          char s[8];
          TagToString (s, type);
          CDLLGauge *dg = new CDLLGauge;
          dg->SetSignature (static_cast <const long> (type));
          dg->SetPanel(this);
          ReadFrom (dg, stream);
          dll_gauge.push_back (dg);
#ifdef _DEBUG
          char buff [8] = {0};
          TagToString (buff, this->id);
          TRACE ("DLL GAUGE (panel) %s <%s>", buff, s);
#endif
          break;
          // early version before 21-12-09
          //WARNINGLOG ("CPanel : Skipping gauge type <%s>", s);
          //SkipObject (stream);
          //rc = TAG_READ;
        }
      }
      return TAG_READ;
      }
//-----------------------------------------------------------------------
//  Read panel parameters
//-----------------------------------------------------------------------
int CPanel::ReadNewGauge(SStream *str, Tag tag)
{ Tag type;
      ReadTag (&type, str);
      switch (type) {
        case GAUGE_GRIP:
            { CGripGauge *g = new CGripGauge(this);
              return ProcessGauge(g,str); }

        case GAUGE_ALTIMETER:
            { C_AltimeterGauge *g = new C_AltimeterGauge(this);
              return ProcessGauge(g,str); }

        case GAUGE_DIRECTIONAL_GYRO:
            { C_DirectionalGyroGauge *g = new C_DirectionalGyroGauge(this);
              return ProcessGauge(g,str); }

        case GAUGE_VERTICAL_SPEED:
          { C_VerticalSpeedGauge *g = new C_VerticalSpeedGauge(this);
            return ProcessGauge(g,str); }

        case GAUGE_HORIZON:
          { C_HorizonGauge *g = new C_HorizonGauge(this);
            return ProcessGauge(g,str); }

        case GAUGE_AIRSPEED:
          { C_AirspeedGauge *g = new C_AirspeedGauge(this);
            return ProcessGauge(g,str); }

        //--------------------------------------------------------
				case GAUGE_VERTICAL_NEEDLE:
          { CVertNeedle *g = new CVertNeedle(this);
            return ProcessGauge(g,str); }

        case GAUGE_NEEDLE:
          { C1NeedleGauge *g = new C1NeedleGauge(this);
            return ProcessGauge(g,str); }

        case GAUGE_BASIC_COMPASS:
        { C_BasicCompassGauge *g = new C_BasicCompassGauge(this);
          return AuxilliaryGauge(g,str); }

        //-------------------------------------------------------- 
        case GAUGE_SIMPLE_SWITCH:
          { C_SimpleSwitch *g = new C_SimpleSwitch(this);
            return ProcessGauge(g,str); }

        case GAUGE_MAGNETO_SWITCH:
          { C_BasicMagnetoSwitch *g = new C_BasicMagnetoSwitch(this);
            return ProcessGauge(g,str); }

        case GAUGE_BATTERY_SWITCH:
          { C_BasicBatterySwitch *g = new C_BasicBatterySwitch(this);
            return ProcessGauge(g,str); }

        case GAUGE_FLYHAWK_ANNUNCIATOR_TEST:
          { C_FlyhawkAnnunciatorTest *g = new C_FlyhawkAnnunciatorTest(this);
            return ProcessGauge(g,str); }

        case GAUGE_ELEVATOR_TRIM:
          { CElevatorTrimGauge *g = new CElevatorTrimGauge(this);
            return ProcessGauge(g,str); }

        //--- SWITCHES and KNOBS ------------------------------
        case GAUGE_MIXTURE_KNOB:
          { CRepeatPushPull *g = new CRepeatPushPull(this);
            return ProcessGauge(g,str); }

        case GAUGE_PUSH_PULL_KNOB:
          { C_PushPullKnobGauge *g = new C_PushPullKnobGauge(this);
            return ProcessGauge(g,str); }

        case GAUGE_FLAP:
          { C_FlapsSwitchGauge *g = new C_FlapsSwitchGauge(this);
          return ProcessGauge(g,str); }

        case GAUGE_SIMPLE_IN_OUT:
          { C_SimpleInOutStateSwitch *g = new C_SimpleInOutStateSwitch(this);
          return ProcessGauge(g,str); }

        case GAUGE_GENERIC_SWITCH:
          { CGenericSwitch *g = new CGenericSwitch(this);
          return ProcessGauge(g,str); }
     
        //---- GAUGES WITH NEEDLES -------------------------

        case GAUGE_NAVIGATION:
          { C_NavigationGauge *g = new C_NavigationGauge(this);
          return ProcessGauge(g,str); }

        case GAUGE_TACHOMETER:
          { C_TachometerGauge *g = new C_TachometerGauge(this);
            return ProcessGauge(g,str); }

        case GAUGE_ADF_BASIC:
          { C_BasicADFGauge *g = new C_BasicADFGauge(this);
            return ProcessGauge(g,str); }
        //--- two NEEDLE GAUGES ---------------------------
        case GAUGE_NEEDLE_DUAL:
          { C2NeedleGauge *g = new C2NeedleGauge(this);
            return ProcessGauge(g,str); }
        //--- ONE NEEDLE GAUGES ---------------------------

        case GAUGE_VACUUM:
          { C_VacuumGauge *g = new C_VacuumGauge(this);
            return ProcessGauge(g,str); }

        case GAUGE_AMMETER:
          { C_AmmeterGauge *g = new C_AmmeterGauge(this);
            return ProcessGauge(g,str); }

        //---- PARTICULAR ONE NEEDLE GAUGES ---------------
        case GAUGE_TURN_COORDINATOR:
          { C_TurnCoordinatorGauge *g = new C_TurnCoordinatorGauge(this);
            return ProcessGauge(g,str); }
        //--- DIGITAL GAUGES ------------------------------
        case GAUGE_HOBBS_METER:
          { C_HobbsMeterGauge *g = new C_HobbsMeterGauge(this);
            return ProcessGauge(g,str); }

        //--- RADIO GAUGES ------------------------------------
        case GAUGE_NAV_COMM_KX155:
          { CK155gauge *g = new CK155gauge(this);
            return ProcessGauge(g,str); }

        case GAUGE_ADF_KR87:
          { CKR87gauge *g = new CKR87gauge(this);
            return ProcessGauge(g,str); }

        case GAUGE_XPDR_KT76:
          { CKT76gauge *g = new CKT76gauge(this);
            return ProcessGauge(g,str); }

				case GAUGE_STRIP:
          { CStripGauge *g = new CStripGauge(this);
            return ProcessGauge(g,str); }

				case GAUGE_NAV_GPS_TYPE0:
        { CNavGpsSwitchT0 *g = new CNavGpsSwitchT0(this);
          return ProcessGauge(g,str); }

      }
			gtfo("Unknown gauge: %s",TagToString(type));
      return TAG_IGNORED;
}
//-----------------------------------------------------------------------
//  Assign a light to panel
//-----------------------------------------------------------------------
void CPanel::AssignLite(Tag t)
{ plite = pit->GetLight(t);
	if (0 == plite)	gtfo("lite %s undefined in %s",TagToString(t),filename);
	return;
}
//-----------------------------------------------------------------------
//  Assign main indicator to panel
//-----------------------------------------------------------------------
void CPanel::SetMain(char m)
{	main = m;
}
//-----------------------------------------------------------------------
//  Load panel texture
//-----------------------------------------------------------------------
void CPanel::LoadTexture(char *fn)
{ TEXT_INFO txf;  // Texture info;
  CArtParser img(TC_HIGHTR);
 // txf.tsp   = 1;
  strncpy(txf.name,fn,TC_TEXTURE_NAME_NAM);
  _snprintf(txf.path,TC_TEXTURE_NAME_DIM,"ART/%s",fn);
  img.GetAnyTexture(txf);
  txPanl.Copy(txf);
  return;
}
//-----------------------------------------------------------------------
//  Load Light  Map
//-----------------------------------------------------------------------
void CPanel::LoadLightMap()
{  return;
}
//-----------------------------------------------------------------------
//  Read panel parameters
//-----------------------------------------------------------------------
int CPanel::Read (SStream *stream, Tag tag)
{ char fnam[128];
	Tag		pm;
  switch (tag) {
		case 'skip':
			skip	= 1;
			return TAG_READ;
    case 'panl':
      // Panel base bitmap
      ReadString (fnam,128, stream);
      LoadTexture(fnam);
      return TAG_READ;

    case '3Dxy':
      // XY offset of panel in 3D space
      ReadInt (&x_3Dxy, stream);
      ReadInt (&y_3Dxy, stream);
      return TAG_READ;

    case '3Dsz':
      // Size of panel in 3D space
      ReadInt (&x_3Dsz, stream);
      ReadInt (&y_3Dsz, stream);
      return TAG_READ;

    case 'isiz':
      // Base bitmap image size
      ReadInt (&x_isiz, stream);
      ReadInt (&y_isiz, stream);
      return TAG_READ;

    case 'hoff':
    // Horizontal offset for various screen resolutions
    { int width, x;
      ReadInt (&width, stream);
      ReadInt (&x, stream);
    }
    return TAG_READ;

    case 'home':
    // Home (x,y) offset for various screen resolutions
    { int width;
      SPanelOffset offset;
      ReadInt (&width, stream);
      ReadInt (&offset.x, stream);
      ReadInt (&offset.y, stream);
    }
    return TAG_READ;

    case 'keep':
    // Keep panel resident in memory when not visible
      return TAG_READ;

    case 'scrl':
      // This is a scrolling panel
      return TAG_READ;

    case 'lite':
      // Panel light
      pit->AddLight(stream);
      return TAG_READ;

    case 'mlsw':
      // Master light switch message
      SkipObject (stream);
      return TAG_READ;

    case 'clao':
      // Cockpit lights are always on
      return TAG_READ;

    case 'dlid':
      // Default light ID
      ReadTag (&pm, stream);
			AssignLite(pm);
      return TAG_READ;

    case 'gage':
      // Read gauge type
      return ReadGauge(stream,tag);

    case 'ngag':
      // Read gauge type
      return ReadNewGauge(stream,tag);
    //-- Strike a key (ex: set parkin braque at start-up
    case 'skey':
      ReadTag(&cmde,stream);
      return TAG_READ;
    }


  // Tag was not processed by this object, it is unrecognized
  WARNINGLOG ("CPanel::Read : Unrecognized tag <%s>", TagToString(tag));
  return TAG_IGNORED;
}
//-----------------------------------------------------------------------------
//  Check parameters
//-----------------------------------------------------------------------------
bool CPanel::ParametersOK()
{ if ((x_isiz > 0) && (x_isiz <= 2048) && (y_isiz > 0) && (y_isiz <= 2048)) return true;
  gtfo ("CPanel::Activate : Could not create %dx%d surface for panel", x_isiz, y_isiz);
  return false;
}
//-----------------------------------------------------------------------------
//  End of parameters
//-----------------------------------------------------------------------------
void CPanel::ReadFinished (void)
{ // Adjust panel dimension to be a multiple of 2 for OpenGL ----------
  if (x_isiz & 1) x_isiz +=1;
  if (y_isiz & 1) y_isiz +=1;
  //---Check parameters and load the bitmap ---------------------------
  if (!ParametersOK())  return;
  //--- Build the VBO buffer for gauge -------------------------------
	BuildGaugeVBO();
	//------------------------------------------------------------------
  Activate ();														// Activate panel texture
  ScreenResize();
	glGenBuffers(1,&pVBO);
	SetViewPort();													// Adjust to screen dimension
	return;
}
//---------------------------------------------------------------------------------
//  Compute panel viewport after change in size
//---------------------------------------------------------------------------------
void CPanel::SetViewPort()
{ int   swd  = globals->cScreen->Width;
  int   sht  = globals->cScreen->Height;
  xMrg   = float((swd - x_3Dsz) >> 1);      // X margin
  yMrg   = float((sht - y_3Dsz) >> 1);      // Y margin
  xlok   = (xMrg > 0);                  // Horizontal lock
  ylok   = (yMrg > 0);                  // Vertical lock
  if (xlok) xOffset = -xMrg;
  else      xMrg    = 0;
  if (ylok) yOffset = +yMrg;
  else      yMrg    = 0;
	pan				= pBUF;
	SetPanelQuad(pan);
	//--- Rebuild the panel VBO -------------------------
	sid				= pan + 4;									// Side vertices
	if (xlok)	{SetCacheLEF(sid); SetCacheRIT(sid);}
	top				= sid + 8;
	if (ylok) {SetCacheTOP(top); SetCacheBOT(top);}
  return;
}
//---------------------------------------------------------------------------------
//  Compute Panel Quad
//---------------------------------------------------------------------------------
void CPanel::SetPanelQuad(TC_VTAB *pan)
{ //-------------------------------------------------------
  //  Allocate the vertex table for the panel 
  //  The panel is drawed as a textured QUAD 
  //----SW base corner ------------------------------------
  pan[Q_SW].VT_S   = 0;
  pan[Q_SW].VT_T   = 0;
  pan[Q_SW].VT_X   = 0;
  pan[Q_SW].VT_Y   = 0;
  pan[Q_SW].VT_Z   = 0;
  //-----SE corner ----------------------------------------
  pan[Q_SE].VT_S   = 1;
  pan[Q_SE].VT_T   = 0;
  pan[Q_SE].VT_X   = float(x_isiz);
  pan[Q_SE].VT_Y   = 0;
  pan[Q_SE].VT_Z   = 0;
  //-----NW corner ----------------------------------------
  pan[Q_NW].VT_S   = 0;
  pan[Q_NW].VT_T   = 1;
  pan[Q_NW].VT_X   = 0;
  pan[Q_NW].VT_Y   = float(y_isiz);
  pan[Q_NW].VT_Z   = 0;
  //-----NE corner ----------------------------------------
  pan[Q_NE].VT_S   = 1;
  pan[Q_NE].VT_T   = 1;
  pan[Q_NE].VT_X   = float(x_isiz);
  pan[Q_NE].VT_Y   = float(y_isiz);
  pan[Q_NE].VT_Z   = 0;
	return;
}
//---------------------------------------------------------------------------------
//  Compute Left Cache
//---------------------------------------------------------------------------------
void CPanel::SetCacheLEF(TC_VTAB *dst)
{ dst[0].VT_X   = -(xMrg);
  dst[0].VT_Y   = -(yMrg);
	dst[0].VT_Z   = 0;
  //-----SE corner ----------------------------------------------------
  dst[1].VT_X   = pan[Q_SW].VT_X;
  dst[1].VT_Y   = pan[Q_SW].VT_Y;
	dst[1].VT_Z   = 0;
  //-----NW corner ----------------------------------------------------
  dst[2].VT_X   = pan[Q_NW].VT_X  - (xMrg);
  dst[2].VT_Y   = pan[Q_NW].VT_Y  + (yMrg);
	dst[2].VT_Z   = 0;
  //-----NE corner ----------------------------------------------------
  dst[3].VT_X   = pan[Q_NW].VT_X;
  dst[3].VT_Y   = pan[Q_NW].VT_Y;
	dst[3].VT_Z   = 0;
  return;
}
//---------------------------------------------------------------------------------
//  Compute Rigth Cache
//---------------------------------------------------------------------------------
void CPanel::SetCacheRIT(TC_VTAB *dst)
{ dst[4].VT_X   = pan[Q_SE].VT_X;
  dst[4].VT_Y   = pan[Q_SE].VT_Y;
	dst[4].VT_Z   = 0;
  //-----SE corner ----------------------------------------------------
  dst[5].VT_X   = pan[Q_SE].VT_X + (xMrg);
  dst[5].VT_Y   = pan[Q_SE].VT_Y - (yMrg);
	dst[5].VT_Z   = 0;
  //-----NW corner ----------------------------------------------------
  dst[6].VT_X   = pan[Q_NE].VT_X;
  dst[6].VT_Y   = pan[Q_NE].VT_Y;
	dst[6].VT_Z   = 0;
  //-----NE corner ----------------------------------------------------
  dst[7].VT_X   = pan[Q_NE].VT_X + (xMrg);
  dst[7].VT_Y   = pan[Q_NE].VT_Y + (yMrg);
	dst[7].VT_Z   = 0;
  return;
}
//---------------------------------------------------------------------------------
//  Compute top Cache
//---------------------------------------------------------------------------------
void CPanel::SetCacheTOP(TC_VTAB *dst)
{ dst[0].VT_X   = pan[Q_NW].VT_X;
  dst[0].VT_Y   = pan[Q_NW].VT_Y;
	dst[0].VT_Z   = 0;
  //-----SE corner ----------------------------------------------------
  dst[1].VT_X   = pan[Q_NE].VT_X;
  dst[1].VT_Y   = pan[Q_NE].VT_Y;
	dst[1].VT_Z   = 0;
  //-----NW corner ----------------------------------------------------
  dst[2].VT_X   = pan[Q_NW].VT_X - (xMrg);
  dst[2].VT_Y   = pan[Q_NW].VT_Y + (yMrg);
	dst[2].VT_Z   = 0;
  //-----NE corner ----------------------------------------------------
  dst[3].VT_X   = pan[Q_NE].VT_X + (xMrg);
  dst[3].VT_Y   = pan[Q_NE].VT_Y + (yMrg);
	dst[3].VT_Z   = 0;
  return;
}
//---------------------------------------------------------------------------------
//  Compute bottom Cache
//---------------------------------------------------------------------------------
void CPanel::SetCacheBOT(TC_VTAB *dst)
{ dst[4].VT_X   = pan[Q_SW].VT_X - (xMrg);
  dst[4].VT_Y   = pan[Q_SW].VT_Y - (yMrg);
	dst[4].VT_Z   = 0;
  //-----SE corner ----------------------------------------------------
  dst[5].VT_X   = pan[Q_SE].VT_X + (xMrg);
  dst[5].VT_Y   = pan[Q_SE].VT_Y - (yMrg);
	dst[5].VT_Z   = 0;
  //-----NW corner ----------------------------------------------------
  dst[6].VT_X   = pan[Q_SW].VT_X;
  dst[6].VT_Y   = pan[Q_SW].VT_Y;
	dst[6].VT_Z   = 0;
  //-----NE corner ----------------------------------------------------
  dst[7].VT_X   = pan[Q_SE].VT_X;
  dst[7].VT_Y   = pan[Q_SE].VT_Y;
	dst[7].VT_Z   = 0;
  return;
}
//---------------------------------------------------------------------------------
//  Reserve room in VBO gauge
//---------------------------------------------------------------------------------
U_SHORT CPanel::FixRoom(U_SHORT n)
{ U_SHORT p = gOfs; 
	gOfs += n; 
	return p;}
//---------------------------------------------------------------------------------
//  Reserve room in dynamic VBO
//---------------------------------------------------------------------------------
U_SHORT CPanel::DynRoom(U_SHORT n)
{ U_SHORT p = dOfs; 
	dOfs += n;
	if (p > PANEL_VBO_TOT) gtfo("No more room in Dynamic VBO");
	return p;
}
//---------------------------------------------------------------------------
//   Collect VBO data
//	NOTE:  The first 4 slots (offset 0) in the VBO are dedicated to the 
//					panel quad.
//				All other gauges have a non zero offset into the VBO buffer
//---------------------------------------------------------------------------
void CPanel::BuildGaugeVBO()
{	int dim = gOfs;
	gBUF		= new TC_VTAB[dim];									// Allocate a buffer
	//--- Call each gauge to collect VBO data -----------------------
	std::vector<CGauge*>::iterator ig;
	for (ig=dgag.begin(); ig!=dgag.end(); ig++)
  { CGauge* g = (*ig);
		g->CollectVBO(gBUF);
	}
	//--- Build the panel Quad -------------------------------------
	SetPanelQuad(gBUF);															// Build panel quad and VBO
	//--- Transform to VBO -----------------------------------------
	int tot = dim * sizeof(TC_VTAB);
  glGenBuffers(1,&gVBO);
	glBindBuffer(GL_ARRAY_BUFFER,gVBO);
	glBufferData(GL_ARRAY_BUFFER,tot,gBUF,GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	//--------------------------------------------------------------
	delete [] gBUF;
	gBUF	= 0;
	return;
}
//---------------------------------------------------------------------------------
//  Compute scroll parameters
//---------------------------------------------------------------------------------
void CPanel::ScreenResize()
{ int temp;
  int swd   = globals->cScreen->Width;
  int sht   = globals->cScreen->Height;
  //---------------------------------------------
  xMinLimit = 0;
  xMaxLimit = x_3Dsz - swd;
  if (xMinLimit > xMaxLimit) {
    temp = xMinLimit;
    xMinLimit = xMaxLimit;
    xMaxLimit = temp;
  }
  
  if (y_3Dxy <= 0) {
    yMinLimit = 0;
    yMaxLimit = -(y_3Dsz - sht - y_3Dxy);
  } else {
    yMinLimit = y_3Dxy;
    yMaxLimit = sht - y_isiz;
  }

  if (yMinLimit > yMaxLimit) {
    temp = yMinLimit;
    yMinLimit = yMaxLimit;
    yMaxLimit = temp;
  }

  // Clamp Offsets to limits ---------------------
  if (yOffset < yMinLimit) yOffset = yMinLimit;
  if (yOffset > yMaxLimit) yOffset = yMaxLimit;
  if (xOffset < xMinLimit) xOffset = xMinLimit;
  if (xOffset > xMaxLimit) xOffset = xMaxLimit;
  return;
}
//---------------------------------------------------------------------------------
//  Activate the panel
//  Create surface
//  Create texture from surface
//---------------------------------------------------------------------------------
void CPanel::Activate (void)
{	int dx = txPanl.wd;
  int dy = txPanl.ht;
  //---Create the texture ---------------------------------------------
  glGenTextures(1,&txOBJ);
  if (0 == txOBJ) gtfo("Panel: No Texture Object");
  GLubyte *tex = txPanl.rgba;
  glBindTexture(GL_TEXTURE_2D,txOBJ);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,dx,dy,0,GL_RGBA,GL_UNSIGNED_BYTE,tex);
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	delete [] tex;
	txPanl.rgba = 0;
  //---Allocate one object for gauges --------------------------
  glGenTextures(1,&ngOBJ);
  glBindTexture(GL_TEXTURE_2D,ngOBJ);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  return;
}
//---------------------------------------------------------------------------------
//  Deactivate the panel
//  Keep bitmap
//---------------------------------------------------------------------------------
void CPanel::Deactivate (void)
{   return;
}
//---------------------------------------------------------------------------------
//  Move verticaly
//---------------------------------------------------------------------------------
void CPanel::MoveVT()
{ yOffset += yDep;
  if (yOffset < yMinLimit) yOffset = yMinLimit;
  if (yOffset > yMaxLimit) yOffset = yMaxLimit;
  return;
}
//---------------------------------------------------------------------------------
//  Move Horizontaly
//---------------------------------------------------------------------------------
void CPanel::MoveHZ()
 { xOffset += xDep;
  if (xOffset < xMinLimit) xOffset = xMinLimit;
  if (xOffset > xMaxLimit) xOffset = xMaxLimit;
  return;
}
//---------------------------------------------------------------------------------
//  Scan list of gauge for a hit
//	(px,py)	are the mouse coordinates relative to panel
//---------------------------------------------------------------------------------
CGauge* CPanel::GaugeHit (int px, int py)
{ //-- Compute coordinates relative to panel ----
  std::vector<CGauge*>::iterator it;
  for (it=dgag.begin(); it!=dgag.end(); it++) 
	{	CGauge *g = (*it);
		// --- check for a hit -------------------------------
		if (g->IsHit(px,py,hit)) return g;
  }
  return 0;
}
//------------------------------------------------------------------------
//  Check if a gauge is clicked
//------------------------------------------------------------------------
bool CPanel::GaugeMouseClick (int mouseX, int mouseY)
{ int px = mouseX + xOffset;
	int py = mouseY + yOffset - (globals->cScreen->Height - y_isiz);
	CGauge *g = GaugeHit (px, py);
  
	if (g) {
		hit.sx	= mouseX;								// X Screen coordinates
		hit.sy  = mouseY;								// Y screen coordinates
    // Send mouse click to the gauge
		track = g->MouseClick (hit.rx, hit.ry, buttonsDown);
    gtk   = (track == MOUSE_TRACKING_ON)?(g):(0);
  }
  return (g != 0);
}
//---------------------------------------------------------------------------
//  Track if mouse is captured
//	(px,py)	are mouse coordinates relative to panel (not to screen)
//---------------------------------------------------------------------------
void CPanel::GaugeTrackClick ()
{ track = gtk->TrackClick (hit.rx, hit.ry, buttonsDown);
  if (track == MOUSE_TRACKING_OFF) gtk = 0; 
  return;
}
//---------------------------------------------------------------------------
//  Click stop:  Send stop to tracking gauge
//---------------------------------------------------------------------------
void CPanel::GaugeStopClick (int x, int y)
{ // Send mouse click to the capturing gauge 
  gtk->StopClick ();
  gtk = 0;
  track = MOUSE_TRACKING_OFF;
  buttonsDown = 0;
	return;
 }
//---------------------------------------------------------------------------
//   Return vertical displacement
//---------------------------------------------------------------------------
int CPanel::VertDisp(int y)
{ if (ylok)                                 return 0;
  int vd = globals->panelCreepStep;
  if (y <= 8)                               return -vd;
  if (y >= globals->cScreen->Height-20)     return +vd;
  return 0;
}
//---------------------------------------------------------------------------
//   Return Horizontal displacement
//---------------------------------------------------------------------------
int CPanel::HorzDisp(int x)
{ if (xlok)                               return 0;
  int vd = globals->panelCreepStep;
  if (x <= 4)                             return -vd;
  if (x >= globals->cScreen->Width - 4)   return +vd;
  return 0;
}
//---------------------------------------------------------------------------
//    Panel mouse motion
//---------------------------------------------------------------------------
bool CPanel::MouseMotion (int mx, int my)
{ //----Compute mouse offset in panel ----------------------------------
	int px = mx + xOffset;
	int py = my + yOffset - (globals->cScreen->Height - y_isiz);
  //---- Scroll when mouse is on margin -------------
  yDep  = VertDisp(my);
  xDep  = HorzDisp(mx);
  //---  Check if a gauge is concerned --------------
	CGauge *g = GaugeHit(px,py);
  if (g) {
    if (buttonsDown != 0) {
      // At least one button is pressed; call gauge's TrackClick method
      //   only if the tracking flag is set
      if ( gtk)  GaugeTrackClick ();
      if (!gtk)  buttonsDown = 0;
    // No buttons are pressed; always call gauge's MouseMoved method
    } 
		else	g->MouseMoved ((px - hit.gx), (py - hit.gy));
  } 
  return (g != 0);
}
//---------------------------------------------------------------------------
//    Panel mouse click
//---------------------------------------------------------------------------
bool CPanel::MouseClick (int button, int updown, int mx, int my)
{ bool OK = false;
  // Update current set of pressed buttons
  /// todo Handle multiple button presses
  switch (updown) {
  case GLUT_DOWN:
    buttonsDown |= button;
    // Button down events are always sent to the underlying gauge
    return GaugeMouseClick (mx, my);

  case GLUT_UP:
    buttonsDown &= ~button;
    // Button up events are only sent to the underlying gauge if tracking is ON
    if (gtk)  GaugeStopClick (mx, my);
    return true;
  }
  return true;
}
//---------------------------------------------------------------------------------
//  Move Horizontaly
//  mx  = mouse position
//---------------------------------------------------------------------------------
void CPanel::ScrollPanel(int mx,int my)
{ xDep  = (xlok)?(0):(mx);
  yDep  = (ylok)?(0):(my);
  return;
}
//---------------------------------------------------------------------------
//    Panel Time slice: Update scrolling
//---------------------------------------------------------------------------
void CPanel::TimeSlice (float dT)
{}
//---------------------------------------------------------------------------
//    Panel scroll
//---------------------------------------------------------------------------
void CPanel::ScrollUP (void)
{ if (!ylok) {
    yOffset -= globals->panelScrollStep;
    if (yOffset < yMinLimit) yOffset = yMinLimit;
  }
}
//---------------------------------------------------------------------------
void CPanel::ScrollDN (void)
{ if (!ylok) {
    yOffset += globals->panelScrollStep;
    if (yOffset > yMaxLimit) yOffset = yMaxLimit;
  }
}
//---------------------------------------------------------------------------
void CPanel::ScrollLF (void)
{ if (!xlok) {
    xOffset -= globals->panelScrollStep;
    if (xOffset < xMinLimit) xOffset = xMinLimit;
  }
}
//---------------------------------------------------------------------------
void CPanel::ScrollRT (void)
{
  if (!xlok) {
    xOffset += globals->panelScrollStep;
    if (xOffset > xMaxLimit) xOffset = xMaxLimit;
  }
}
//---------------------------------------------------------------------------
// Set the panel y-offset to the maximum "up" scrolled position
//---------------------------------------------------------------------------
void CPanel::PageUp (void) {}
//---------------------------------------------------------------------------
// Set the panel y-offset to the maximum "down" scrolled position
//---------------------------------------------------------------------------
void CPanel::PageDown (void) {}
//----------------------------------------------------------------------------
// Return the unique ID tag value for this panel instance
//----------------------------------------------------------------------------
Tag CPanel::GetId (void)
{  return id;}
//---------------------------------------------------------------------------
//   Locate Gauge by Tag and store as gauge to focus on
//---------------------------------------------------------------------------
void CPanel::FocusOnGauge(Tag id)
{ short w,h;
  if (0 == id)            return;
  std::map<Tag,CGauge*>::iterator it = gage.find(id);
  if (it == gage.end())   return;
  CGauge *gg  = it->second;
  gFocus = gg;
  gg->GetSize(&Fx,&Fy,&w,&h);
  return;
}
//---------------------------------------------------------------------------
//   Locate Gauge by Tag and store as gauge to focus on
//---------------------------------------------------------------------------
void CPanel::FocusOnGauge(CGauge *g)
{ short w,h;
  gFocus = g;
  //--- Set gauge origin -----------------------
  g->GetSize(&Fx,&Fy,&w,&h);
  return;
}
//---------------------------------------------------------------------------
//   Focus on Click Area
//---------------------------------------------------------------------------
void CPanel::FocusOnClick(CGauge *g, char No)
{ if ((0 == g) || (0 == No)) return;
  ca  = g->GetClickArea(No); 
  return;
}
//---------------------------------------------------------------------------
//   Get Gauge by Tag
//---------------------------------------------------------------------------
CGauge *CPanel::GetGauge(Tag id)
{ std::map<Tag,CGauge*>::iterator it=gage.find(id);
  if (it == gage.end())   return 0;
  return it->second;}

//---------------------------------------------------------------------------
//   Draw ClickArea tour
//  NOTE:  Y line are inverted on the bitmap
//---------------------------------------------------------------------------
void CPanel::DrawCaTour()
{ //--- Compute Gauge Screen origin ---------------------
  short sx,sy;
  short w, h;
  gFocus->GetSize (&sx, &sy, &w, &h);
  sy  = y_isiz - sy;          // Gauge base line
  //--- Compute CA tour ---------------------------------
  float x0 = sx + ca->x1;
  float y0 = sy - ca->y1;
  float x1 = sx + ca->x2;
  float y1 = sy - ca->y2;
  //--- Draw tour ---------------------------------------
  glPushMatrix();
  glLoadIdentity();
  glPushAttrib (GL_ALL_ATTRIB_BITS);
  glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);       // Draw lines
  glDisable(GL_TEXTURE_2D); 
  glBegin(GL_LINE_LOOP);
  glVertex2f(x0,y0);
  glVertex2f(x0,y1);
  glVertex2f(x1,y1);
  glVertex2f(x1,y0);
  glEnd();
  glPopAttrib();
  glPopMatrix();
	glPolygonMode(GL_FRONT,GL_FILL);
}
//===========================================================================
// Draw the panel, including all contained gauges
//===========================================================================
void CPanel::Draw (CCameraCockpit *cam)
{ //--- Creep the panel if any flags are set
  if (xDep) MoveHZ();
  if (yDep) MoveVT();
  //--- Set projection matrix to 2D screen size
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D ((GLdouble) xOffset, (GLdouble)( xOffset + globals->cScreen->Width),
              (GLdouble)-yOffset, (GLdouble)(-yOffset + globals->cScreen->Height));

  // Initialize modelview matrix to ensure panel drawing is not affected by
  //   any junk left at the top of the stack by previous rendering
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  glLoadIdentity ();

  // Set up OpenGL drawing state
  glPushAttrib (GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_COLOR_BUFFER_BIT | GL_POLYGON_BIT);
  //------Compute Panel Britness ------------------------------------------------
  GLfloat S[4];
	glGetFloatv (GL_LIGHT_MODEL_AMBIENT, S);
	float R = S[0] * 0.38f;
  float G = S[1] * 0.74f;
  float B = S[2] * 0.16f;
  S[0]		= R + G + B;
  brit[0]	= max(globals->tcm->GetLuminosity(),S[0]);
  dlite->SetBrightness(brit[0]);
  //--- Override default light with panel lighting
  float brightness = plite->GetBrightness();
  brit[0] = max(brightness, brit[0]);
  brit[1] = brit[0];
  brit[2] = brit[0];
  pit->SetBrightness(brit[0]);
 //-------------------------------------------------------------------------------
  glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
  glEnable(GL_TEXTURE_2D);
  glDisable (GL_DEPTH_TEST);            // No Z test
  glDepthMask(GL_FALSE);                // Do not write in depth buffer
  glBindTexture(GL_TEXTURE_2D,txOBJ);
  //----Draw the panel as a textured square using VBO ---------------
  glMaterialfv(GL_FRONT,GL_EMISSION,brit);
  glEnable(GL_BLEND);
  glPolygonMode(GL_FRONT,GL_FILL);
	//---- Activate panel VBO -----------------------------------------
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
  //  
  // Draw gauges which are even partially visible on screen
	//	NOTE on first tour, gauge are registered into the TRACE file
  //
	glBindBuffer(GL_ARRAY_BUFFER,gVBO);
	glTexCoordPointer(2,GL_FLOAT,sizeof(TC_VTAB),0);
	glVertexPointer  (3,GL_FLOAT,sizeof(TC_VTAB),OFFSET_VBO(2*sizeof(float)));
	//----Draw main panel --------------------------------------------
  glDrawArrays(GL_TRIANGLE_STRIP,0,4);
  std::vector<CGauge*>::iterator iter;
  //-------Set common parameters -------------------------------------
	glBindTexture(GL_TEXTURE_2D,ngOBJ);
	if (1 == trn1) TRACE("=========== PANEL %s ===============",filename);
  for (iter=dgag.begin(); iter!=dgag.end(); iter++)
  { CGauge* g = (*iter);
    //---Render gauge with gauge light --------------
	  g->SetBrightness(*brit);
    g->Draw();
	  glMaterialfv(GL_FRONT,GL_EMISSION,brit);
		g->DrawAmbient();
		//--- Trace the gauge ---------------------------
		if (0 == trn1)	continue;
		TRACE("..Gauge %s",g->GetUniqueID());
  }
	trn1 = 0;
	glBindBuffer(GL_ARRAY_BUFFER,0);
  //-- SDK:Draw DLL Gauge -------------------------------------------
  if (globals->plugins_num) {
    std::vector<CDLLGauge *>::iterator dll_iter;
    for (dll_iter = dll_gauge.begin(); dll_iter!=dll_gauge.end(); dll_iter++) {
      CDLLGauge *dg = *dll_iter;
      dg->Draw ();
    }
  }
	//--- Draw bands and dynamics gauges ------------------------------
	int	tot = PANEL_VBO_TOT * sizeof(TC_VTAB);
	glBindBuffer(GL_ARRAY_BUFFER,pVBO);
	glBufferData(GL_ARRAY_BUFFER,tot,pBUF,GL_STATIC_DRAW);
	glTexCoordPointer(2,GL_FLOAT,sizeof(TC_VTAB),0);
	glVertexPointer  (3,GL_FLOAT,sizeof(TC_VTAB),OFFSET_VBO(2*sizeof(float)));
	
	//--- Draw dynamic gauges -----------------------------------------
	  for (iter=sgag.begin(); iter!=sgag.end(); iter++)
  { CGauge* g = (*iter);
    //---Render gauge with gauge light --------------
	  g->SetBrightness(*brit);
    g->DrawDynamic();
	  glMaterialfv(GL_FRONT,GL_EMISSION,brit);
		g->DrawAmbient();
  }
	
	//--- Draw all bands -----------------------------------------------
	glColor4f(0,0,0,1);
  glDisable(GL_LIGHTING);
  glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
  if (xlok)
  { glDrawArrays(GL_TRIANGLE_STRIP, 4,4);
    glDrawArrays(GL_TRIANGLE_STRIP, 8,4);
  }
  if (ylok)
  { glDrawArrays(GL_TRIANGLE_STRIP,12,4);
    glDrawArrays(GL_TRIANGLE_STRIP,16,4);
  }
	glBindBuffer(GL_ARRAY_BUFFER,0);
  //--Focus on Gauge if requested ------------------------------------
  char on = globals->clk->GetON();
  if (on && gFocus) gFocus->OutLine();
  if (on && ca    ) DrawCaTour();
  //------------------------------------------------------------------
  // Restore original OpenGL state
  glPopAttrib();
  glPopClientAttrib ();
  //------------------------------------------------------------------
  // Restore original projection and modelview matrices
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  //------------------------------------------------------------------
  // Check for an OpenGL error
	/*
  { GLenum e = glGetError ();
    if (e != GL_NO_ERROR) 
    WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  }
	*/
	return;
}

//---------------------------------------------------------------------------
//	Return a global light
//---------------------------------------------------------------------------
CPanelLight*	CPanel::GetLight(Tag id)
{	return pit->GetLight(id);	}
//============END OF FILE =========================================================