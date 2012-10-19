/*
 * AutopilotSubsystems.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
 * Copyright 2007-etc  Jean Sabatier
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

#include "../Include/Subsystems.h"
#include "../Include/Autopilot.h"
#include "../Include/RadioGauges.h"
#include "../Include/Globals.h"
#include "../Include/FuiParts.h"
#include "../Include/FuiUser.h"
#include "../Include/Joysticks.h"
using namespace std;
//============================================================================
#define VSI_SAMPLE (double(0.05))
//============================================================================
//  K140 Fields to draw
//============================================================================
RADIO_DSP  CBKAP140Gauge::fieldTAB[] = {
  {K140_FD_HD1,RADIO_CA11},                          // Heading mode field 1
  {K140_FD_HD2,RADIO_CA12},                          // Heading mode field 2
  {K140_FD_VT1,RADIO_CA13},                          // Vertical mode field 1
  {K140_FD_VT2,RADIO_CA14},                          // Vertical mode field 2
  {K140_FD_VSI,RADIO_CA18},                          // VSI
  {K140_FD_ALT,RADIO_CA19},                          // Altitude
  {0,},                                              // Last field
};
//============================================================================
//  K140 Fields to draw
//============================================================================
RADIO_FLD  alertK140[] = {
  {0,RAD_ATT_INACT,0},                      // 0 state
  {0,RAD_ATT_ACTIV,"ALT"},                  // 1 State
  };
//============================================================================
//  Autopilot PID name
//============================================================================
Tag CPIDbox::PidIDENT[] = {
  'roll',                   
  'head',
  'bank',
  'glid',
  'alth',
  'vsih',
  'aoa_',
  'rudr',
	'thro',
};
//===============================================================================================
//  Autopilot edit table
//===============================================================================================
//---Lateral mode -----------------------------------------
char *autoTB1[] = {
  "OFF",					// OFF
  "ROL",          // Roll control
  "HDG",          // Heading
	"LT0",					// APR up TO
  "LG1",          // NAV intercept
  "LG2",          // NAV tracking
	"GND",					// Ground steering
	"TGA",					// Go arround
};
//--- Vertical mode ---------------------------------------
char *autoTB2[] = {
  "OFF",					// 0
	"TKO",					// 1-Take off
  "ALT",          // 2-Altitude
  "VSP",          // 3-VSP Tracking
  "GSW",          // 4-Glide slope intercept
  "GST",          // 5-Glide slope tracking
  "FLR",          // 6-Flare mode
	"FIN",					// 7-Final
};
//===============================================================================================
// PID controlleLinear boxr
//===============================================================================================
void VLinear::Upd(double dT)
{	float dta = (dT / Time);
	if (dta > 1)	dta = 1;
	cVal += (tVal - cVal) * dta;
	return;
}

//===============================================================================================
// PID controller
//  A PID controller is a single unit used to monitor a single value against a reference value
//===============================================================================================
//  Init default values
//--------------------------------------------------------------------------------------
CPIDbox::CPIDbox(U_CHAR No,AutoPilot *ap)
{ TypeIs (SUBSYSTEM_PIDBOX);
  hwId  = HW_PID;
  SetIdent(PidIDENT[No]);
  nPid  = No;
  apil  = ap;  //---------------------------------------------
  Yn  = 0;              // Current value to 0
  Rn  = 0;              // Reference value to 0
  //--------Default values-----------------------
  Rn    = 0;
  Kp    = 0.05;
  Ts    = 0;
  SetITIME(0);
  SetDTIME(0);
  SetATIME(0);
  iMax  = 0;
  vmin  = -1;
  vmax  = +1;
  drvt  = 0;            // Dont use derivative
  intg  = 0;            // Dont use integrator
  anti  = 0;            // Dont use anti saturation
  //---------------------------------------------
  Init();
}
//------------------------------------------------------------------------------
//  Free the resources
//------------------------------------------------------------------------------
CPIDbox::~CPIDbox()
{ 
}
//------------------------------------------------------------------------------
//  Init Controller
//------------------------------------------------------------------------------
void CPIDbox::Init()
{ en    = 0;
  en1   = 0;
  en2   = 0;
  Un    = 0;
  Vn    = 0;
  es    = 0;
  Rn    = 0;
  eSum  = 0;
  yPrv  = 0;
  Ki    = 0;
  Kd    = 0;
  Ks    = 0;
  Yn    = 0;
  vSat  = 0;
  return;
}
//------------------------------------------------------------------------------
//	Set coefficients
//------------------------------------------------------------------------------
void CPIDbox::SetCoef(double kp,double ti, double td)
{	Kp	= kp;
	Ti	= ti;
	Td	= td;
	return;
}
//------------------------------------------------------------------------------
//  Read controller parameters
//------------------------------------------------------------------------------
int CPIDbox::Read(SStream *st,Tag tag)
{ double val;
  switch(tag) {
  //---Kp: Proportional coefficient ----------
  case 'Kp__':
    ReadDouble(&Kp,st);
    return TAG_READ;
	//--- TimK ---------------------------------
	case 'timK':
		ReadFloat(&timK,st);
	  return TAG_READ;
  //---Integrator time -----------------------
  case 'iTim':
    ReadDouble(&val,st);
    SetITIME(val);
    return TAG_READ;
  //---Saturator time -----------------------
  case 'aTim':
    ReadDouble(&val,st);
    SetATIME(val);
    return TAG_READ;
  //---Integrator limit -----------------------
  case 'iMax':
    ReadDouble(&iMax,st);
    return TAG_READ;
  //---Derivator time -----------------------
  case 'dTim':
    ReadDouble(&val,st);
    SetDTIME(val);
    return TAG_READ;
  //---Minimum value ----------------------
  case 'vmin':
    ReadDouble(&vmin,st);
    return TAG_READ;
  //---Maximum value ----------------------
  case 'vmax':
    ReadDouble(&vmax,st);
    return TAG_READ;
  }
  return TAG_IGNORED;
}
//------------------------------------------------------------------------------
//  All parameters are read
//------------------------------------------------------------------------------
void CPIDbox::ReadFinished()
{ 
  return;
}
//------------------------------------------------------------------------------
//  Set derivator time
//------------------------------------------------------------------------------
void CPIDbox::SetDTIME(double t)
{ Td    = t;
  drvt  = (fabs(t) > 0.00001)?(1):(0);
  return;
}
//------------------------------------------------------------------------------
//  Set integrator time
//------------------------------------------------------------------------------
void CPIDbox::SetITIME(double t)
{ Ti    = t;
  intg  = (fabs(t) > 0.00001)?(1):(0);
  eSum  = 0;
  Ki    = 0;
  return;
}
//------------------------------------------------------------------------------
//  Set anti saturation time
//------------------------------------------------------------------------------
void CPIDbox::SetATIME(double t)
{ Ta    = t;
  anti  = (fabs(t) > 0.00001)?(1):(0);
  return;
}

//------------------------------------------------------------------------------
//  clamp value
//------------------------------------------------------------------------------
double CPIDbox::Clamp(double v)
{ if (v < vmin) return vmin;
  if (v > vmax) return vmax;
  return v;
}
//------------------------------------------------------------------------------
//  check clamp values
//------------------------------------------------------------------------------
void CPIDbox::SetClamp(CAeroControl *sys)
{ float m1;
  float m2;
  sys->GetClamp(m1,m2);
  if (m1 > vmin)  vmin = m1;
  if (m2 < vmax)  vmax = m2;
  return;
}
//------------------------------------------------------------------------------
//  Set message for reading
//------------------------------------------------------------------------------
void CPIDbox::SetMsgInp(SMessage *msg,Tag dst, Tag prm)
{ msg->group          = dst;
  msg->sender         = unId;
  msg->id             = MSG_GETDATA;
  msg->dataType       = TYPE_REAL;
  msg->user.u.datatag = prm;
  return;
}

//------------------------------------------------------------------------------
//  Return a parameter value
//------------------------------------------------------------------------------
double CPIDbox::GetValue(Tag pm)
{ switch (pm) {
  case 'Kp__':
    return Kp;
  case 'iTim':
    return Ti;
  case 'dTim':
    return Td;
  case 'aTim':
    return Ta;
  case 'iMax':
    return iMax;
  case 'vmin':
    return vmin;
  case 'vmax':
    return vmax;

  }
  return 0;
}
//------------------------------------------------------------------------------
//  Set a parameter value
//------------------------------------------------------------------------------
void CPIDbox::SetValue(Tag pm,double val)
{ switch (pm) {
  case 'Kp__':
    Kp  = val;
    return;
  case 'iTim':
    SetITIME(val);
    return;
  case 'dTim':
    SetDTIME(val);
    return;
  case 'aTim':
    SetATIME(val);
    return;
  case 'iMax':
    iMax  = val;
    return;
  case 'vmin':
    vmin = val;
    return;
  case 'vmax':
    vmax = val;
    return;

  }
  return;
}
//------------------------------------------------------------------------------
//  Compute integral term
//------------------------------------------------------------------------------
double CPIDbox::Integrate(double ki)
{ double    vi   = ki * eSum;
  if (vi > +iMax)  return +iMax;
  if (vi < -iMax)  return -iMax;
  return vi;
}
//------------------------------------------------------------------------------
//  Compute saturation value
//------------------------------------------------------------------------------
void CPIDbox::TrakSaturation(double val,double minv, double maxv)
{ vSat = 0;
  if ((val > minv) && (val < maxv))   return;
  double ers = val - Un;
  vSat = (ers * Ta) / Ts;
  return;
}
//------------------------------------------------------------------------------
//  Update the controller and return the output value
//    tm is delta time
//    Y  is the actual controlled value
//    R is the reference (target) value
//   return the new action value
//------------------------------------------------------------------------------
double CPIDbox::Update(float tm,double Y,double R)
{ double dtm = 0;
  double itm = 0;
  //--Update sum and history -----------
  eSum -= en2;
  en2   = en1;
  en1   = en;
  //------------------------------------
  Ts    = tm;
  yPrv  = Yn;
  Yn    = Y;
  Rn    = R;
  //----Compute Ki ---------------------
  if (intg) Ki = (Kp * tm) / Ti;
  //----Compute Kd ---------------------
  if (drvt) Kd = (Kp * Td) / tm;
  //----Compute current error ----------
  en    = Rn - Yn;
  eSum += en;
  //-----Compute proportional term -----
  Un    =  (Kp * en);
  //---- Get Integral term -------------
  if (intg) {Ki = Integrate(Ki);    Un += Ki;}
  //---- Get Derivative gain -----------
  if (drvt) {Kd = Kd * (Yn - yPrv); Un += Kd;}
  Vn = Clamp(Un);
  return Vn;
}
//--------------------------------------------------------------------
//  Edit controller data for Probe
//--------------------------------------------------------------------
void CPIDbox::Probe(CFuiCanva *cnv)
{ //---Input signal and reference
  cnv->AddText( 1,0,"Inp(Yn) %.05f",Yn);
  cnv->AddText(20,1,"Ref(Rn) %.05f",Rn);
  //---Ti and Td -----------------------------------
  cnv->AddText( 1,0,"Ki      %.05f",Ki);
  cnv->AddText(20,1,"Kd      %.05f",Kd);
  //------------------------------------------------
  cnv->AddText( 1,1,"sat      %.05f",vSat);
  //------------------------------------------------
  cnv->AddText(20,1,"Out(Vn) %.05f",Vn);
  return;
}
//===============================================================================================
//  CLASS CPIDdecoder
//        Class to decode all PID controllers
//===============================================================================================
CPIDdecoder::CPIDdecoder(char *fn,AutoPilot *ap)
{ apil	= ap;
  vpil	= ap->GetMVEH()->amp.GetVirtualPilot();
	SStream s(this,"WORLD",fn);
}

//------------------------------------------------------------------------------------
//  Delete all resources
//------------------------------------------------------------------------------------
CPIDdecoder::~CPIDdecoder()
{ 
}
//------------------------------------------------------------------------------------
//  Read list of PID components
//------------------------------------------------------------------------------------
int CPIDdecoder::Read(SStream *st,Tag tag)
{ CPIDbox *pbx = 0;
  double   prm = 0;
  float    fnb = 10;
  switch (tag)  {
		//--- Rotation speed for take-off ------------------------
		case 'Vrot':
			apil->DecodeVROT(st);
			return TAG_READ;
		//--- Cut off altitude -----------------------------------
		case 'THRO':
			apil->DecodeTHRO(st);
			return TAG_READ;
		//--- Angle of approach to runway -----------------------
		case 'aprw':
			ReadDouble(&prm,st);
			apil->SetAPRW(prm);
			return TAG_READ;
		//---Disengage altitude ---------------------------------
		case 'land':
			apil->DecodeLAND(st);
			return TAG_READ;
		//--- Flaps parameters ----------------------------------
		case 'flap':
			apil->DecodeFLAP(st);
			return TAG_READ;
		//---MISS LANDING PARAMETERS ---------------
		case 'miss':
			apil->DecodeLMIS(st);
			return TAG_READ;
		//---Heading coefficient --------------------------------
		case 'bias':              // Head adjust
			ReadDouble(&prm,st);
			return TAG_READ;
		//---Turn anticipation ----------------------------------
		case 'vDTA':
			ReadDouble(&prm,st);
			apil->SetVDTA(prm);
			return TAG_READ;
		//----Glide catching angle ------------------------------
		case 'catg':
			ReadDouble(&prm,st);
			apil->SetGLDopt(prm);
			return TAG_READ;
		//---Vertical trim reference -----------------------------
		case 'vref':
			ReadDouble(&prm,st);
			apil->SetVREF(prm);
			return TAG_READ;
		//----ROLL controller ------------------------------------
		case 'roll':              
			pbx = new CPIDbox(PID_ROL,apil);
			ReadFrom(pbx,st);
			apil->AddPID(pbx);
			return TAG_READ;
		//---HEAD controller ------------------------------------
		case 'head':
			pbx = new CPIDbox(PID_HDG,apil);
			ReadFrom(pbx,st);
			apil->AddPID(pbx);
			return TAG_READ;
		//---BANK CONTROLLER ------------------------------------
		case 'bank':
			pbx = new CPIDbox(PID_BNK,apil);
			ReadFrom(pbx,st);
			apil->AddPID(pbx);
			return TAG_READ;
		//---ALTITUDE hold controller ---------------------------
		case 'alth':
			pbx = new CPIDbox(PID_ALT,apil);
			ReadFrom(pbx,st);
			apil->AddPID(pbx);
			return TAG_READ;
		//---Glide Slope controller ---------------------------
		case 'glid':
			pbx = new CPIDbox(PID_GLS,apil);
			ReadFrom(pbx,st);
			apil->AddPID(pbx);
			return TAG_READ;
		//---Vertical speed hold controller -----------------------
		case 'vsih':
			pbx = new CPIDbox(PID_VSP,apil);
			ReadFrom(pbx,st);
			apil->AddPID(pbx);
			return TAG_READ;
		//---Angle of attack controller -----------------------
		case 'aoa_':
			pbx = new CPIDbox(PID_AOA,apil);
			ReadFrom(pbx,st);
			apil->AddPID(pbx);
			return TAG_READ;
		//---Rudder controller -----------------------
		case 'rudr':
			pbx = new CPIDbox(PID_RUD,apil);
			ReadFrom(pbx,st);
			apil->AddPID(pbx);
			return TAG_READ;
		//--- Line table -----------------------------
		case 'line':
    { apil->InitLINE(st);
      return TAG_READ;
    }
		//--- Auto start list ------------------------
		case 'strt':
			return	vpil->StrtProcedure(st);
		//--- Auto stop procedure --------------------
		case 'stop':
			return	vpil->StopProcedure(st);
  }

  return TAG_IGNORED;
}
//===============================================================================================
// CAutopilotMasterPanel
//===============================================================================================
CAutopilotMasterPanel::CAutopilotMasterPanel (void)
{ TypeIs (SUBSYSTEM_AUTOPILOT_MASTER_PANEL); }
//-----------------------------------------------------------------------------------
//  Read parameters 
//-----------------------------------------------------------------------------------
int CAutopilotMasterPanel::Read (SStream *stream, Tag tag)
{switch (tag) {
  case 'mAP1':
    ReadMessage(&ap1,stream);
    return TAG_READ;
  case 'mAP2':
    ReadMessage(&ap2,stream);
    return TAG_READ;
    }
return CDependent::Read(stream,tag);
}
//===============================================================================================
//
// AutoPilot:  Autopilot generic subsystem
//  This subsystem is used as the core autopilot for every other panels
//===============================================================================================
AutoPilot::AutoPilot (void)
{ TypeIs (SUBSYSTEM_AUTOPILOT_PANEL);
  hwId    = HW_OTHER;
  lStat   = AP_DISENGD;
  vStat   = AP_DISENGD;
  glide   = 0.5;                     // Default catching glide angle
  Radio   = 0;
  Powr    = 0;
  land    = LAND_DISCT;			// Landing option
	rend		= 0;							// No runway end
	xCtl		= 0;							// No external control
	trace   = 0;
	//--- Final leg parameters -------------------------
	Turn		= 1;							// Turning anticipation factor
	gain		= 2.5;
	//--- TGA parameters -----------------------------------
	TGA0		= 2.5;						// 2.5 miles for Leg0
	TGA1		= 4.0;						// 6.0 miles for Leg1
	//---Flap parameters -----------------------------------
	tkoFP		= 0;
	tkoFA		= -100;
	lndFP		= 0;
	lndFA		= -100;
  //---Init flasher --------------------------------------
  timFS   = 0;              // Flasher timer
  mskFS   = 0xFF;           // Character on
  //---Lateral mode --------------------------------------
	rDIS		= 0;
	aHDG		= 0;
  rHDG    = 0;
  hERR    = 0;
  vHRZ    = 0;
  Vref    = 0;            // Default Vertical Trim reference
	//------------------------------------------------------
	aTGT		= 2500;					// Target altitude at take off
	vROT		= 0;						// Rotation speed at TKO
	xRAT		= 0;						// Cruise rate
	aFSP    = 2000;					// Altitude to enter final speed		
	fSPD		= 0;						// Final speed
	cRAT		= 0;						// Crrent speed 
	aCUT		= 0;						// Altitude for cut throttle
	xAPW		= 45;						// Angle of Approach to runway
  //---Vertical parameters -------------------------------
  eVRT    = 0;
  rALT.Set(400,1);
  rVSP    = 0;
	vrub		= +0.1;
	vrlb    = -0.1;
  //---Options and controls ------------------------------
  uvsp    = 0;
  aprm    = 0;
	ugaz		= 0;
	//--Lateral error is 2.5° whatever the distance -----------
  SetAMIS(400,1500);						// Misslanding altitude check
  hMIS    = 2;
	cMIS    = 2 / aMIS;
  //--Vertical error is tangent(1) * 1000 units -------------
  vMIS  = 2.5;
  //---Default limits ------------------------------------
  aLim    = 10000;
  vLim    = 2000;
  abrt    = 0;
  //-------------------------------------------------------
  for (int k=0; k<PID_MAX; k++) pidL[k] = 0;
	//-------------------------------------------------------
	
  //-------------------------------------------------------
  mALT.group  = 'alti';               // altimeter
  mALT.sender = unId;
  mALT.user.u.datatag = 'alti';       // baro altitude tag
  mALT.id = MSG_GETDATA;              // Get data
  mALT.dataType = TYPE_REAL;          // Double format
  //-------------------------------------------------------
  mVSI.group  = 'vsi_';               // VSI
  mVSI.sender = unId;
  mVSI.user.u.datatag = 'vsi_';       // vertical speed
  mVSI.id = MSG_GETDATA;              // Get data
  mVSI.dataType = TYPE_REAL;          // Double format
  //-------------------------------------------------------
  double d3 = 3.0;
  double r3 = DegToRad(d3);           // 3° in radian
  sin3      = sin(r3);                // sine(3°)
  //-------------------------------------------------------
  plane = 0;
  //---Set Time K ------------------------------------------
  timK  = float(VSI_SAMPLE);
  eVSP  = 0;
  aLND  = 200;                        // Disengage altitude
 
  //----Init lights ----------------------------------------
  alta    = 0;
  flsh    = 0;
}

//------------------------------------------------------------------------------------
//  Decode PID controllers
//------------------------------------------------------------------------------------
void AutoPilot::ReadPID(char *fn)
{ char fname[MAX_PATH];
  if (0 == *fn)   return;
  strncpy(fname,fn,256);
  char *dot = strchr(fname,'.');
  if (0 == dot)   return;
  strcpy(dot,".PID");
  CPIDdecoder pdc(fname,this);
  //---- Check that all PID exist ------------------
  inUse = 1;
  if (0 == pidL[PID_ROL]) inUse = 0;
  if (0 == pidL[PID_HDG]) inUse = 0;
  if (0 == pidL[PID_BNK]) inUse = 0;
  if (0 == pidL[PID_GLS]) inUse = 0;
  if (0 == pidL[PID_ALT]) inUse = 0;
  if (0 == pidL[PID_AOA]) inUse = 0;
  if (0 == pidL[PID_VSP]) uvsp  = 0;
  //---- Store plane to control ---------------------
  plane = (CAirplane*) mveh;
  return;
}
//------------------------------------------------------------------------------------
//  Get direct access to most used subsystems
//------------------------------------------------------------------------------------
void AutoPilot::PrepareMsg(CVehicleObject *veh)
{ //-- init PID -------------------------------
  mveh->Register(this);
  char *fcs = mveh->nfo.GetFCS();
  ReadPID(fcs);
  InitPID();
	//--- Locate altimeter ----------------------
	mveh->FindReceiver(&mALT);
	altS = (CSubsystem*)mALT.receiver;
	if (0 == altS)	inUse = 0;
	//--- Locate compass ------------------------
  mveh->FindReceiver(&mHDG);
	cmpS = (CSubsystem*)mHDG.receiver;
	if (0 == cmpS)	inUse = 0;
	//--- Get Speed regulator -------------------
	sreg = mveh->GetSREG();
	//--- Get radio bus -------------------------
	Radio = mveh->GetRadioBUS();
	//--- end of init ---------------------------

  return;
}
//------------------------------------------------------------------------------------
//  Init Aircraft parameters
//------------------------------------------------------------------------------------
void AutoPilot::InitPID()
{ if (0 == inUse) return;
  //---Get control surfaces ------------------------------
	flpS = mveh->amp.GetFlaps();
  ailS = mveh->amp.GetAilerons();
  elvS = mveh->amp.GetElevators();
  rudS = mveh->amp.GetRudders();
  elvT = mveh->amp.GetElevatorTrim();
	brak = mveh->amp.GetBrakeControl();
  //---Aircraft paramaters --------------------------------
  minA    = mveh->GetWingAoAMinRad();
  maxA    = mveh->GetWingAoAMaxRad();
  minA    = RadToDeg(minA) * 0.5f;
  maxA    = RadToDeg(maxA) * 0.6f;
  //------------------------------------------------------
  pidL[PID_GLS]->SetMaxi(maxA);
  pidL[PID_GLS]->SetMini(minA);
  //------------------------------------------------------
  pidL[PID_ALT]->SetMaxi(maxA);
  pidL[PID_ALT]->SetMini(minA);
  //------------------------------------------------------
  pidL[PID_BNK]->SetClamp(ailS);
  pidL[PID_AOA]->SetClamp(elvS);
  pidL[PID_AOA]->SetClamp(elvT);
  //------------------------------------------------------
  if (0 == uvsp)  return;
  pidL[PID_VSP]->SetMaxi(maxA);
  pidL[PID_VSP]->SetMini(minA);
  return;
}
//------------------------------------------------------------------------------------
//  Decode miss landing
//------------------------------------------------------------------------------------
void AutoPilot::DecodeLMIS(SStream *st)
{ char txt[128];
	double pm1;
  double pm2;
	ReadString(txt,128,st);
	int nf = sscanf(txt,"Check %lf ft , TGO %lf ft",&pm1, &pm2);
	if (nf != 2)		return;
	SetAMIS(pm1,pm2);
	return;
}
//------------------------------------------------------------------------------------
//  Decode Rotation speed
//------------------------------------------------------------------------------------
void AutoPilot::DecodeVROT(SStream *st)
{	char txt[128];	
	double pm1;
	double pm2;
	ReadString(txt,128,st);
  int nf = sscanf(txt,"%lf kts , %lf ft",&pm1,&pm2);
	if (nf != 2)	return;
	vROT		= pm1;
	aTGT		= pm2;
	return;
}

//------------------------------------------------------------------------------------
//  Decode throttle control
//------------------------------------------------------------------------------------
void AutoPilot::DecodeTHRO(SStream *st)
{ char txt[128];
	double pm1,pm2;
	double pm3,pm4;
	ReadString(txt,128,st);
	int nf = sscanf(txt,"Final %lf ft , Cut %lf ft , More %lf , Less %lf",&pm1, &pm2, &pm3, &pm4);
	if (nf != 4)	return;
	aFSP	= pm1;
	aCUT	= pm2;
	vrlb  = pm3;
	vrub  = pm4;
	return;
}
//------------------------------------------------------------------------------------
//  Decode Landing option
//------------------------------------------------------------------------------------
void AutoPilot::DecodeLAND(SStream *st)
{ double pm1;
  double pm2;
	double pm3;
  int nf;
  char str[128];
  ReadString(str,100,st);
  nf = sscanf(str," disengage , %lf ft", &pm1);
  if (1 == nf)    {aLND = pm1; return; }
  nf = sscanf(str," Flare , %lf ft, %lf deg, %lf kts",&pm1,&pm2,&pm3);
  if (3 == nf)     SetFLRopt(pm1,pm2,pm3);
  return;
}
//------------------------------------------------------------------------------------
//  Decode Flaps parameters
//------------------------------------------------------------------------------------
void AutoPilot::DecodeFLAP(SStream *st)
{	char txt[128];
	int		 pm1,pm3;
	double pm2,pm4;
	ReadString(txt,128,st);
	int nf = sscanf(txt,"tko ( %d , %lf ft ) , lnd ( %d , %lf ft)", &pm1,&pm2,&pm3,&pm4);
	if (nf != 4)	return;
	SetTkoFLP(pm1,pm2);
	SetLndFLP(pm3,pm4);
	return;
}

//------------------------------------------------------------------------------------
//  Fill alignment table
//------------------------------------------------------------------------------------
void AutoPilot::InitLINE(SStream *st)
{	 linTB.DecodeFMT1(st);
}
//------------------------------------------------------------------------------------
//  Round value
//------------------------------------------------------------------------------------
double AutoPilot::RoundValue(double v,double p)
{ int v1 = int(v  * p);
  return double(v1) / p;
}
//------------------------------------------------------------------------------------
//  Compute Aircraft Angle of slope
//------------------------------------------------------------------------------------
double AutoPilot::GetAOS()
{ double p = mveh->GetPitch();					
  return -RoundValue(p,100);
}
//------------------------------------------------------------------------------------
//  Compute vertical error
//  Given the VSI and the vehicle speed, we can compute the slope of ascent/descent.
//      target slope = asin( (speed in feet per minute) / (vsi feet per minute))
//  The error is the difference between actual slope and target one (in radian)
//------------------------------------------------------------------------------------
void AutoPilot::VSPerror()
{ double fpm  = FN_FEET_FROM_MILE(aSPD) / 60;  // In feet per minute 
  if (aSPD < 10) return;                       // No more speed 
  double val  = rVSP / fpm;                    // Sine of angle of target slope
  double phi  = asin(val);
  phi         = RadToDeg(phi);
  //---Get actual angle ------------------
  double aos  = -GetAOS();
  eVSP        = (phi - aos);
	bool in			= CheckAlert();
  //---Check for Altitude Alert ----------
  if (!in)		return;
  //---Change to ALTITUDE HOLD ------------
  EnterALT();
  return;
}
//------------------------------------------------------------------------------------
//  Check altitude Alert
//  The flasher is set if
//  a)  Altitude alert (alta) is set, and
//  b)  The difference between target (rALT) and actual (cALT) altitude
//      is in [1000,200] feet
//  The function return true if altitude alert is effective and coming within 50 feet
//------------------------------------------------------------------------------------
bool AutoPilot::CheckAlert()
{ eVRT			= (rALT.Get() - cALT);   // Vertical error
	float lim = fabs(float(eVRT));
	if ((alta) && (lim > 200) && (lim < 1000))  flsh = 1;
  return (lim < 50);
}
//------------------------------------------------------------------------------------
//  Add PID controller 
//------------------------------------------------------------------------------------
void AutoPilot::AddPID(CPIDbox *pbx)
{ pidQ.PutEnd(pbx);
  int No    = pbx->GetPidNo();
  pidL[No]  = pbx;
  return;
}
//------------------------------------------------------------------------------------
//  Delete all resources
//------------------------------------------------------------------------------------
AutoPilot::~AutoPilot()
{	CPIDbox *pbx  = pidQ.Pop();
  while (pbx) {delete pbx; pbx = pidQ.Pop();}
  mveh->Register((AutoPilot *)0);
}
//-----------------------------------------------------------------------------------
//  Read parameters 
//-----------------------------------------------------------------------------------
int AutoPilot::Read (SStream *stream, Tag tag)
{ SMessage msg;
	float pm;
  switch (tag) {
  case 'atop':
    // Autopilot specification
    SkipObject (stream);
    return TAG_READ;
  //----Get heading (gyro or GPS) ----------------
  case 'mDG_':
    // Directional gyro (heading) message
    ReadMessage (&mBUG, stream);
    mBUG.user.u.datatag = 'dBug';         // Get Bug deviation
    mBUG.id = MSG_GETDATA;
    mHDG    = mBUG;
    mHDG.user.u.datatag = 'yaw_';         // Actual heading
    return TAG_READ;
  //----Altimeter -----------------------------
  case 'mAlt':
    // Altimeter message
    ReadMessage (&mALT, stream);
    return TAG_READ;
  //----VSI gauge ----------------------------
  case 'vsi_':
    ReadMessage (&mVSI, stream);
    return TAG_READ;
  //----Speed system --------------------------
	case 'sped':
		ReadMessage(&msg, stream);
		return TAG_READ;
  //----Navigation radio ---------------------
  case 'nav1':
    // NAV radio message
    ReadMessage (&mNAV, stream);
    return TAG_READ;
	//--- Throttle control --------------------
	case 'thro':
		ReadMessage(&msg, stream);
		return TAG_READ;
  //---Altitude limit ------------------------
  case 'aLim':
    ReadFloat(&pm,stream);
    aLim  = pm;
    return TAG_READ;
  //---VSP limit -----------------------------
  case 'vLim':
    ReadFloat(&pm,stream);
    vLim  = pm;
    return TAG_READ;
  }

  return CDependent::Read(stream,tag);
}
//-----------------------------------------------------------------------------------
//  All parameters are read
//-----------------------------------------------------------------------------------
void AutoPilot::ReadFinished()
{ CDependent::ReadFinished();
  CSimulatedVehicle *svh = &mveh->svh;
  fSPD	= svh->GetApproachSpeed();
	xRAT  = svh->GetCruiseSpeed() * 0.98;
  return;
}
//-----------------------------------------------------------------------
//  Get all PID subsystems for probe
//-----------------------------------------------------------------------
void AutoPilot::GetAllSubsystems(std::vector<CPIDbox*> &pid)
{ CPIDbox *pbx;
  for (pbx = pidQ.GetFirst(); pbx!=0; pbx = pidQ.GetNext(pbx))
  { pid.push_back(pbx); }
  return;
}
//-----------------------------------------------------------------------
//  Get all PID subsystems for PID tuner
//-----------------------------------------------------------------------
void AutoPilot::GetAllPID(CFuiPID *win)
{ CPIDbox *pbx;
  for (pbx = pidQ.GetFirst(); pbx!=0; pbx = pidQ.GetNext(pbx))
  { win->AddPID(pbx); }
  return;
}
//-----------------------------------------------------------------------
//  Set flare option
//  Compute the leg length that is the lenght before touching ground during
//  the flared segment
//  Note that eLEG is computed feet
//  a = decision altitude
//  s = slope in degres
//	Compute ground distance as h/d = tan(s)
//	so d = h / tan(alpha)
//-----------------------------------------------------------------------
void AutoPilot::SetFLRopt(double a, double s, double d)
{ double r = DegToRad(s);
  aLND    = a;
	dSPD		= d;
	sTAN		= tan(r);
  land    = LAND_FLARE;
  return;
}
//-----------------------------------------------------------------------
//  Set miss landing option
//  a is the decision altitude
//  r is the cone radius to catch the signal
//-----------------------------------------------------------------------
void AutoPilot::SetAMIS(double a,double b)
{ aMIS	= RoundValue(a,100);
  cMIS  = 2 / a;
	aTGA = RoundValue(b,100);
  return;
}
//---------------------------------------------------------------------
//  This is to intercept AXIS destination as well
//---------------------------------------------------------------------
bool AutoPilot::MsgForMe (SMessage *msg)
{ bool matchGroup = ((msg->group == unId) || (msg->group == 'AXIS'));
  bool hwNull     = (msg->user.u.hw == HW_UNKNOWN);
  bool hwMatch    = (msg->user.u.hw == (unsigned int) hwId);
  bool unitNull   = (msg->user.u.unit == 0);
  bool unitMatch  = (msg->user.u.unit == uNum);
  return matchGroup && (hwNull || hwMatch) && (unitNull || unitMatch);
}
//--------------------------------------------------------------------
//  Receive a message 
//--------------------------------------------------------------------
EMessageResult AutoPilot::ReceiveMessage (SMessage *msg)
{ EMessageResult rs = (active)?(MSG_ACK):(MSG_IGNORED);
  if (msg->id == MSG_GETDATA) {
    switch (msg->user.u.datatag) {
      //--- Request a pointer to this subsystem --------
      case 'gets':
        msg->voidData = this;
        return MSG_PROCESSED;
      //--- Auto pilot engaged -------------------------
      case 'apOn':
				msg->intData = (IsEngaged())?(1):(0);
        return MSG_PROCESSED;
      //--- Heading mode ON ----------------------------
      case '_Hdg':
        msg->intData = (lStat == AP_LAT_HDG)?(1):(0);
        return MSG_PROCESSED;
      //--- Altitude maintained ------------------------
      case '_Alt':
        msg->intData = (vStat == AP_VRT_ALT)?(1):(0);
        return MSG_PROCESSED;
      //--- Navigation mode ----------------------------
      case '_Nav':
        msg->intData = NavMode();
        return MSG_PROCESSED;
      //--- Approach mode ------------------------------
      case '_Apr':
        msg->intData = IlsMode();
        return MSG_PROCESSED;
      //--- Glide slope tracking -----------------------
      case '__GS':
        msg->intData = (vStat == AP_VRT_GST);
        return MSG_PROCESSED;

      }
    return  MSG_IGNORED;
  }
  if (msg->id == MSG_SETDATA) {
    switch (msg->user.u.datatag) {
      //---Autopilot engage ----------------------------
      case 'apOn':
        NewEvent(AP_EVN_ENG);
        msg->result = rs;
        return MSG_PROCESSED;
      //---Heading mode --------------------------------
      case '_Hdg':
        NewEvent(AP_EVN_HDG);
        msg->result = rs;
        return MSG_PROCESSED;
      //--- Altitude hold ------------------------------
      case '_Alt':
        NewEvent(AP_EVN_ALT);
        msg->result = rs;
        return MSG_PROCESSED;
      //--- Navigation mode ----------------------------
      case '_Nav':
        NewEvent(AP_EVN_NAV);
        msg->result = rs;
        return MSG_PROCESSED;
      //--- Approach mode ------------------------------
      case '_Apr':
        NewEvent(AP_EVN_APR);
        msg->result = rs;
        return MSG_PROCESSED;
    }
  }
      //=======================================
  return  MSG_IGNORED;
}
//-----------------------------------------------------------------------
//  Manage Lateral mode (aircraft behavior in horizontal
//-----------------------------------------------------------------------
void AutoPilot::LateralMode()
{ 
  switch (lStat)  {
    case AP_DISENGD:
      return ModeDIS();
    case AP_LAT_ROL:
      return ModeROL();
    case AP_LAT_HDG:
      return ModeHDG();
		case AP_LAT_LT0:
			return ModeLT0();
    case AP_LAT_LT1:
      return ModeLT1();
    case AP_LAT_LT2:
      return ModeLT2();
		case AP_LAT_TGA:
			return ModeTGA();
		case AP_LAT_GND:
			return ModeGND();
  }
	gtfo("AUTOPILOT error");
	return;
}
//-----------------------------------------------------------------------
//  Manage Vertical mode
//-----------------------------------------------------------------------
void AutoPilot::VerticalMode()
{ flsh  = 0;  
  switch(vStat) {
      //--- Disengaged ---------
      case AP_DISENGD:          
        return;
			//--- Take off -----------
			case AP_VRT_TKO:
				return;
      //--- Maintain altitude --
      case AP_VRT_ALT:
        ModeALT();
        return;
      //--- Glide intercept ----
      case AP_VRT_GSW:
        ModeGSW();
        return;
      //--- Glide tracking ----
      case AP_VRT_GST:
        ModeGST();
        return;
      //--- Vertical speed ----
      case AP_VRT_VSP:
        ModeVSP();
        return;
      //---Flare leg ---------
      case AP_VRT_FLR:
        ModeFLR();
				return;
			//---Final leg --------
			case AP_VRT_FIN:
				ModeFIN();
        return;
   }
	gtfo("AUTOPILOT error");
  return;
}
//-----------------------------------------------------------------------
//  Time slice Autopilot
//-----------------------------------------------------------------------
void AutoPilot::TimeSlice(float dT,U_INT FrNo)
{ CDependent::TimeSlice(dT,FrNo);
	rALT.Upd(dT);															// Update reference altitude
	if (0 == Radio)												return;
  if (0 == active) {PowerLost();        return;}                        
  if (globals->dbc->NotSynch(FrameNo))  return;
  dTime = dT;
  if (AP_DISENGD == lStat)              return;
  //----Get current parameters (altitude, heading, etc)------
	sect	= (Radio->flag == VOR_SECTOR_TO)?(1):(0);
	wgrd  = mveh->WheelsAreOnGround();
	cALT	= altS->GaugeBusFT01();					// Current altitude
	cAGL	= mveh->GetAltitudeAGL();
	aSPD	= mveh->GetPreCalculedKIAS();		// Current speed
	aHDG	= cmpS->GaugeBusFT01();					// Current mag heading
	//--- Update both modes -----------------------------------
  LateralMode();
  VerticalMode();
	SpeedHold();
  return;
}

//-----------------------------------------------------------------------
//  AUTO PILOT DISENGAGE
//	NOTE: 
//			Elevator trim computed by autopilot is transfered to the aero control
//			so there is no brutal change when autopilot is disengaged in flight
//-----------------------------------------------------------------------
void AutoPilot::Disengage(char gr)
{ lStat   = AP_DISENGD;
  vStat   = AP_DISENGD;
  signal  = SIGNAL_OFF;
  StateChanged(AP_STATE_DIS);
	mveh->SetABS(0);
  ailS->Neutral();							// Aileron to O
  rudS->Neutral();
	flpS->SetPosition(0);
	sreg->SetOFF();
	ugaz		= 0;
	rend		= 0;
	//---- Pull up if altitude is lower than 500 ---------
	if (cAGL < 500)	elvT->SetValue(-0.5);
  //----Reset trim if aircraft on ground ---------------
  if ((gr) || (wgrd)) elvT->SetValue(0);
  Alarm();
  return;
}
//-----------------------------------------------------------------------
//  Check radio Signal
//-----------------------------------------------------------------------
int AutoPilot::BadSignal(char s)
{ if (0 == Radio)           {Alarm(); return 1;}
  char  msk = Radio->ntyp & s; 
  if (0 == msk)             {Alarm(); return 1;}
  signal  = s;
  return 0;
}
//-----------------------------------------------------------------------
//	Track Speed
//	NOTE: Adjust throttle to target speed
//-----------------------------------------------------------------------
void AutoPilot::SpeedHold()
{	if (0 == ugaz)				return;
	cRAT		= SelectSpeed();
	ugaz = (sreg->SetSpeed(cRAT))?(1):(0);
}
//-----------------------------------------------------------------------
//  Manage Lateral mode with ailerons
//  Maintain the target heading rHDG
//-----------------------------------------------------------------------
void AutoPilot::LateralHold()
{ float turn    = mveh->GetBanking();								// Actual banking (deg)
  CPIDbox *hbox = pidL[PID_HDG];                    // Heading controller
  CPIDbox *bbox = pidL[PID_BNK];                    // Banking controller
  double   err  = Norme180(aHDG - rHDG);						// Error in [-180,+180]
  double   trn = hbox->Update(dTime,err,0);					// Input to Head controller
  double   avl = bbox->Update(dTime,turn,trn);      // Banking to controller
  ailS->PidValue(avl);                              // result to ailerons
	if (trace)  TRACE("HOLD: aHDG=%.2f rHDG=%.2f",aHDG,rHDG);
  return;
}
//-----------------------------------------------------------------------
//  Manage Vertical mode
//  Maintain the target vertical slope. As very small value is used
//  for glide slope error, so it is multiplied by 1000 because it is
//  easier to control by the PID.
//-----------------------------------------------------------------------
void AutoPilot::GlideHold()
{ eVRT *= vAMP;                                       // Amplify glide error
  double gld = pidL[PID_GLS]->Update(dTime,eVRT,0);   // error to glide controller
  double glr = RoundValue(gld,100);
  double aoa = GetAOS();                              // Current AOA inverted
  double etr = pidL[PID_AOA]->Update(dTime,aoa,glr);  // Feed AOA controler
  elvT->SetValue(-etr);                               // Vertical to elevator
  return;
}
//-----------------------------------------------------------------------
//  Manage Altitude mode
//  Maintain the target altitude
//  NOTE:  The VSP controller is maintained in synchro with the altitude
//         controller.
//-----------------------------------------------------------------------
void AutoPilot::AltitudeHold()
{ double nul = pidL[PID_VSP]->Update(dTime,eVRT,0); // Maintain VSP controller
  double taa = pidL[PID_ALT]->Update(dTime,eVRT,0); // erro to altitude controller
  double alt = RoundValue(taa,10);                  // round value
  double aoa = GetAOS();
  double val = pidL[PID_AOA]->Update(dTime,aoa,alt);// Feed AOA controler
  elvT->SetValue(-val);                             // To elevator TRIM
  return;
}
//-----------------------------------------------------------------------
//  Manage Roll mode
//  1°Read rate turn and send sample to ROLL CONTROLLER
//  2°Activate ROL Controller
//-----------------------------------------------------------------------
void AutoPilot::ModeROL()
{ rHDG	= cmpS->GaugeBusFT01();					// Current mag heading
	double bnk = mveh->GetRRtoLDBank();               // Get Bank angle
  double cor = pidL[PID_ROL]->Update(dTime,bnk,0);  // Feet to controller
  double val = pidL[PID_BNK]->Update(dTime,cor,0);  // Turn rate
  ailS->PidValue(val);                              // Send to aileron 
  return;
}
//--------------------------------------------------------------------------------
//	Mode LT0: We are in approach mode in sector FR, going in Sector TO
//						until we are somes miles aways to reenter LT1 mode
//--------------------------------------------------------------------------------
void	AutoPilot::ModeLT0()
{ //--- Check for new approach conditions -------
	bool go = (sect) && (Radio->mdis > TGA1);
	if (go)		return GetCrossHeading();
	LateralHold();
	return;
}
//--------------------------------------------------------------------------------
//  Manage Lateral Leg 1
//  Leg1 is the part where aircraft is heading in direction D, trying to
//  intercept the radial R of the OBS or ILS. D may be any direction, even one
//  that is opposite the NAV/ILS signal.
//---------------------------------------------------------------------------------
//  The arc AB start with point A where the aircraft should start to turn
//  toward direction  R.  A is the decision point to leave Leg1 to enter
//  Leg2 which is the R tracking part.
//--------------------------------------------------------------------------------
//  Decision point is based on time for the aircraft to complete ARC AB
//  Assuming a standard turning speed of 2° sec (A complete 360° turn in 3 minutes)
//  the time T0 is given by
//  T0 = A / 2   where A = (R - D)mod360, the angle from direction D
//  to direction R (the reference direction)
//  At anytime we know the distance from the aircraft P to the R direction
//  at point H.
//  h = horizontal distance from P to D = d sin(Q) where d is the distance
//  from P to the ILS/OBS spot and Q° is the angle between R and the radial
//  where P reside.  h = distance PH.
//  We can compute the velocity of P along PH with v = V sin(Q) where V is
//  the aircraft velocity along D and Q is defined above.
//  T1 = h / v.  Is the time for P to reach D.
//  Now when T1 <= T0, its time to turn.
//--------------------------------------------------------------------------------
void AutoPilot::ModeLT1()
{ if (BadSignal(signal))		return EnterROL();
  //----Compute distance to desired track/runway direction ----------
  double rd   = DegToRad(Radio->rDEV);
  rDIS        = Radio->mdis * sin(rd);    // Distance to R (nm)
	//--- Remaining distance to ils plane ----------------------------------
	if (fabs(rDIS) > vDTA)	return CrossDirection();
  //----Enter second leg ------------------------------------------------
  if (trace)	TRACE("Enter LT2");
  lStat = AP_LAT_LT2;
  StateChanged(sEVN);
	return;
}
//-----------------------------------------------------------------------
// Get the runway crossing direction
//  The precomputed cross direction xHDG is used.
//  xHDG was computed to cross the NAV/ILS signal at right angle
//-----------------------------------------------------------------------
void AutoPilot::CrossDirection()
{ rHDG	= xHDG;
	if (trace) TRACE("rHDG to %.1f", rHDG);
  return LateralHold();
}
//-----------------------------------------------------------------------
//	Adjust correction factor according to different types of legs
//	Red azone is a cone of 20° around the ils line
//	When landing in red zone, the heading correction is boosted to allow
//	more sensibility
//	era is error in horizontal plan between actual radial and target one.
//	cFAC	is a correction factor for heading
//-----------------------------------------------------------------------
double AutoPilot::AdjustHDG()
{ //-- Approach=> 45° toward ILS---------------------
	double era  = fabs(Radio->hDEV);
	redz				= (era < 20)?(1):(0);			// Red sector
	cFAC				= 0;
	//--- Remaining distance to runway axis -----------
	double rd   = DegToRad(Radio->rDEV);
  rDIS        = Radio->mdis * sin(rd);    // Distance to R (nm)
	//--- Approach leg and not in red sector ----------
	if (!redz &&  aprm)		return xCOR;
	//--- Other tracking mode --------------
	double coef  = linTB.Lookup(era);
  cFAC  =  (coef * Radio->hDEV);
	if (!aprm)	cFAC *= 0.4;
	double dir  = Wrap360(Radio->radi - cFAC);     // New direction;
	//if (aprm) TRACE("L2-%.4lf:hDEV=%.4lf rDIS=%.4lf coef=%.4lf cFAC=%.4lf aHDG=%.4lf dir=%.4lf",
	//							Radio->hREF,Radio->hDEV,rDIS,coef,cFAC,aHDG,dir);  
	return dir;
}
//-----------------------------------------------------------------------
//  Manage Lateral Leg 2
//  Track reference direction(hREF)
//  During leg 2, the ILS signal is tracked laterally by feeding corrective
//  amount to the reference heading. The Lateral PID is then used
//------------------------------------------------------------------------
//  hERR =  Error between Current Radial (crossed by plane) and 
//          hREF.  
//  This value is used to adjust the reference heading (rHDG) used by
//  the heading PID. 
//		hERR negatif => aircraft is on left side of Reference direction
//		hERR positif => aircraft is on right side
//-----------------------------------------------------------------------
void AutoPilot::ModeLT2()
{ if (BadSignal(signal))  return ExitLT2();
  //--Compute heading factor ---------------
	rHDG	= AdjustHDG();     // New direction;
	//	TRACE("LT2: aHDG=%.4f RADI=%.4f hERR=%.4f, cFAC=%.4f rHDG=%.4f",
	//	aHDG,Radio->radi,hERR,cFAC,rHDG);
	//-- check for final leg ----------------
	return LateralHold();
}
//-----------------------------------------------------------------------
//	Mode GO ARROUND: A standard procedure is applied 
//	TODO:  Build a file for specific GO ARROUND per airport
//	LEG 0:  Climb up to 500 feet
//	LEG 1:  Turn 90° to the runway direction and make some distance
//	LEG 2:  Climb up to 1500 and turn 180° to the runway
//					Make some distance and then enter approach
//-----------------------------------------------------------------------
void	AutoPilot::ModeTGA()
{	double amin = aMIS + 100;
	double agrn = mveh->GetGroundAltitude();			//globals->tcm->GetGroundAltitude();
	double mref = RoundAltitude(agrn + aTGA);
	rALT.Set(mref,1);
	//TRACE("LEG=%d cAGL=%.2lf rALT.tval=%.2lf rALT.cval=%.2lf",stga,cAGL,rALT.Tvl(),rALT.Get());
	switch (stga)	{
		//--- climb to (aMISS + 100) AGL -----------
		case AP_TGA_UP5:
			if (cAGL < amin)			 return LateralHold();
			flpS->SetPosition(0);
			//--- Set direction to 90° left of runway --
			rHDG	= Wrap360(Radio->hREF - 90);
			stga  = AP_TGA_HD1;
			return  LateralHold();
		//--- Go back to heading mode --------------
		case AP_TGA_HD1:
			if (Radio->mdis < TGA0)	return LateralHold();
			stga	= AP_TGA_HD2;
			return LateralHold();
		//--- wait for some distance --------------
		case AP_TGA_HD2:
			//--- Set Direction along the runway ----
			rHDG	= Wrap360(Radio->hREF - 180);
			stga  = AP_TGA_HD3;
			return  LateralHold();
		case AP_TGA_HD3:
			if (Radio->mdis < TGA1)		return LateralHold();
			return EnterAPR();
	}
	return;
}
//-----------------------------------------------------------------------
//	Brake until half stop speed is reached
//-----------------------------------------------------------------------
void AutoPilot::Brake()
{	if (0 == brak)		return;
	if (aSPD < dSPD)	return;
	brak->HoldBrake(BRAKE_BOTH);
	return;
}
//-----------------------------------------------------------------------
//	Ground steering
//	Compute deviation error between target and current direction
//	Use rudder to maintain target heading
//	
//-----------------------------------------------------------------------
void AutoPilot::ModeGND()
{	//--- Steer to target --------------------------
	sreg->SteerTo(rend->refP);					// Steer to direction
	SPosition &S	= rend->lndP;					// runway origin
	rend->sect		= GetSectorNumber(S,*mveh->GetAdPosition());
	//TRACE("rHDG=%.5f DIR=%.5f hERR=%.5f val=%.5f",rHDG,gHDG,hERR,val);
	//---- hold level ---------------------------
	ModeROL();
	//---- Proceed according to vertical mode ---
	switch (vStat)	{
		//--- take off mode ----------------------
		case AP_VRT_TKO:
			if (aSPD < vROT)							return;
			Rotate();
			return;
		//--- in final disengage below cut speed--
		case AP_VRT_FIN:
			Brake();
			if (aSPD > dSPD)							return;
			Disengage(1);
			return;
	}
	return;
}
//-----------------------------------------------------------------------
//		Rotate for take off
//		Set altitude alert to requested altitude
//-----------------------------------------------------------------------
void AutoPilot::Rotate()
{	alta  = 1;                        // ALT armed
  StateChanged(AP_STATE_AAA);       // State is changed
	//--- Set reference altitude ------------------------
	rALT.Set(RoundAltitude(aTGT + mveh->GetGroundAltitude()),0.5);
	StateChanged(AP_STATE_ACH);				// Altitude changed
	//--- Enter altitude Hold ---------------------------
	StateChanged(AP_STATE_ALT);       // Warn Panel
  vStat = AP_VRT_ALT;               // Lock on altitude
	rudS->Neutral();									// Reset rudder
	ailS->Neutral();									// Reset ailerons
  pidL[PID_ALT]->Init();            // init PID
  pidL[PID_AOA]->Init();            // init PID
	elvT->PidValue(0.5);							// Boost up								
	EnterROL();
	sreg->SteerOFF();									// Stop steering
	return;
}
//-----------------------------------------------------------------------
//	Check for miss landing conditions
//-----------------------------------------------------------------------
bool AutoPilot::MissLanding()
{ if (cAGL > aMIS)							return false;
  if (cAGL <   50)							return false;
	hMIS = cMIS * cAGL;
  //---Check for lateral miss landing ------------------
  if (fabs(hERR) > hMIS)        return AbortLanding(1);
	//---Check for vertical miss landing -----------------
  if (fabs(eVRT) > vMIS)				return AbortLanding(2);
	return false;
}
//-----------------------------------------------------------------------
//	Process landing option
//	This function is called only from vertical glide tracking mode
//-----------------------------------------------------------------------
void AutoPilot::LandingOption()
{ hERR	= Radio->hDEV;
	if (MissLanding())					return;
	//--- Update options ---------------------------
	U_INT opt = mveh->GetOPT(VEH_AP_OPTN) | land;  // Land option
	//--- Check for flap control -------------------
	float mspd = flpS->GetMaxSpeed(lndFP);
	if ((cAGL < lndFA) && (aSPD < mspd)) flpS->SetPosition(lndFP);
	//---Check flare control -----------------------
	bool fc = (opt == LAND_FLARE) && (cAGL < aLND);
	if (fc)		return EnterFLR();
	if (cAGL > aLND)						return;
	//--- Disconnect autopilot ---------------------
	Disengage(0);
  return;
}
//-----------------------------------------------------------------------
//  Get VSP coefficient 
//-----------------------------------------------------------------------
float AutoPilot::GetVSPCoef()
{	float lim = fabs(float(eVRT));
	if (lim > 200)		return 1;
	return (lim / 200);
}
//-----------------------------------------------------------------------
//  Track Altitude 
//-----------------------------------------------------------------------
void AutoPilot::ModeALT()
{ bool in = CheckAlert();
  AltitudeHold();
	if (!in)				return;
  alta  = 0;                        // ALT arm off
  StateChanged(AP_STATE_AAA);       // State is changed
  return;
}
//-----------------------------------------------------------------------
//  Track glide slope uses an amplifier of 1000 for better control
//-----------------------------------------------------------------------
//  NOTE:  The vertical amplifier acts as a damping feature to avoid
//         a brutal change of attitude when the glide slope is tracked
//         at the begining.
//         vAMP is set to 1000 at maximum value
//-----------------------------------------------------------------------
void AutoPilot::ModeGST()
{ eVRT = -Radio->gDEV;          // Glide error
  if (vAMP < 1000) vAMP += 5;   // Increase amplifier
  GlideHold();                  // Hold slope
	LandingOption();
  return;
}
//-----------------------------------------------------------------------
//  Wait for glide slope intercept
//  Maintain altitude until the glide slope of tuned ILS is intercepted
//  -glide is the catching error threshold
//-----------------------------------------------------------------------
void AutoPilot::ModeGSW()
{ eVRT  = (rALT.Get() - cALT);   // Vertical error
	if (lStat == AP_LAT_LT0)			return AltitudeHold();
  if (Radio->gDEV < 0.0005)			return AltitudeHold();
	pidL[PID_GLS]->Init();
  vStat = AP_VRT_GST;
  StateChanged(AP_STATE_VTK);
  vAMP  = 0;               // Vertical amplifier
  return;
}
//-----------------------------------------------------------------------
//  Abort landing
//	Enter lateral TGA mode and vertical ALTITUDE HOLD
//-----------------------------------------------------------------------
bool AutoPilot::AbortLanding(char r)
{ abrt  = r;
  Alarm();
  EnterALT();
  EnterROL();
	//rALT.Set(RoundAltitude(globals->tcm->GetGroundAltitude() + aTGA),0.5);
	rALT.Set(RoundAltitude(mveh->GetGroundAltitude() + aTGA),0.5);
	StateChanged(AP_STATE_ALT);
	lStat	= AP_LAT_TGA;
	stga	= AP_TGA_UP5;
  return true;
}
//-----------------------------------------------------------------------
//  Track vertical speed. 
//  Get VSP error in eVSP
//  Maintain ALT controller in synchro
//  Use VSP controller to get correction . Vref is the trim position
//  at null VSI (depend on aircraft)
//-----------------------------------------------------------------------
void AutoPilot::ModeVSP()
{ VSPerror();
  double nul = pidL[PID_ALT]->Update(dTime,eVSP,0);       // Maintain ALT controller
  double trm = pidL[PID_VSP]->Update(dTime,eVSP,Vref);    // Feed VSI controler
  elvT->SetValue(-trm);                                   // value to TRIM elevator
  return;
}
//-----------------------------------------------------------------------
//  Flare segment
//	Compute expected altitude at touch dow distance
//  NOTE: Vertical error eVRT is computed in Tan unit for
//        compatibility with GlideHold()
//-----------------------------------------------------------------------
void AutoPilot::ModeFLR()
{ afps = aSPD * FEET_PER_NM * HOUR_PER_SEC;          // In feet per sec
	//--- compute expected altitude ------------------------------
	double dis	= Radio->fdis - nTDP;				// Touch down distance
	double xagl	=	dis * sTAN;								// Expected AGL
	dTDP				= dis;											// Touch down point
	eVRT				= (xagl - cAGL) / dis;			// Expressed in tan unit
  vAMP = 2000;														// Amplifier
  GlideHold();
	//	TRACE("FLR: kts=%.2f agl=%.4f AoA=%.4f xagl=%.4f",spd,cAGL,-GetAOS(),xagl);
	//--- wait for touch down ------------------------------------
  if (wgrd)  EnterFIN();
	rAGL	= cAGL;
  return;
}
//-----------------------------------------------------------------------
//	Final groung segment:  
//			Compute runway segment:
///-----------------------------------------------------------------------
void AutoPilot::ModeFIN()
{	
	return;	}

//-----------------------------------------------------------------------
//  Manage Head mode
//  -Read autopilot deviation from heading to BUG
//  -This is the input for HEAD PID
//  -Reference is 0. The HEAD PID will send turning order until
//                   deviation is 0
//  -Send Turning rate to BANK PID to limit rate turn;
//-----------------------------------------------------------------------
void AutoPilot::ModeHDG()
{ //---Activate heading controller ------------------------------
  Send_Message(&mBUG);                            // Get BUG deviation
  rHDG  = mBUG.realData;													// Target = bug direction
  LateralHold();																	// Reduce deviation
  return;
}
//-----------------------------------------------------------------------
//  In disengage mode, we maintain the VSP controller in standby
//-----------------------------------------------------------------------
void AutoPilot::ModeDIS()
{ rVSP = CatchVSP();
  VSPerror();
  pidL[PID_VSP]->Update(dTime,eVSP,0);
	rALT.Set(RoundAltitude(cALT),1);		// Current altitude As reference
  return;
}
//-----------------------------------------------------------------------
//  Lost of Power:  Enter inactive state
//-----------------------------------------------------------------------
int AutoPilot::PowerLost()
{ lStat = AP_DISENGD;
  vStat = AP_DISENGD;
  alta = 0;
  return 0;
}
//-----------------------------------------------------------------------
//  Exit mode LT2
//	Enter altitude Hold at 1500 minimum
//-----------------------------------------------------------------------
void AutoPilot::ExitLT2()
{ EnterROL();
  EnterALT();
  return;
}
//-----------------------------------------------------------------------
//  Exit Heading mode
//-----------------------------------------------------------------------
void AutoPilot::ExitHDG()
{ StateChanged(AP_STATE_HDF);
  EnterROL();
  return;
}

//-----------------------------------------------------------------------
//  Enter INI Mode
//-----------------------------------------------------------------------
void AutoPilot::EnterINI()
{ pidL[PID_AOA]->Init();
	ailS->Neutral();
	elvT->Neutral();
	EnterROL();
  EnterVSP();
	stga	= 0;
  return;
}
//-----------------------------------------------------------------------
//  Leave NAV or APR leg1 with NAV event
//  1° Transition NAV => No NAV :   Enter ROL mode only
//  2° Transition APR => NAV    :   Enter NAV mode
//-----------------------------------------------------------------------
void AutoPilot::ExitNAV()
{ if (0 == aprm)  return EnterROL();
  //---Changing from APR to NAV -------------------
  EnterALT();                   // Enter Altitude mode
  EnterNAV();                   // Enter NAV mode
  return;   
}
//-----------------------------------------------------------------------
//  Leave APR leg2  NAV event
//  1° Transition NAV => No NAV :   Enter ROL mode only
//-----------------------------------------------------------------------
void AutoPilot::ExitAPR()
{ if (0 == aprm)  return EnterAPR();
  //--- Leave APR mode ----------------------------
	aprm	= 0;
  EnterALT();
  EnterROL();
  return;
}
//-----------------------------------------------------------------------
//  Enter ALT Mode
//  NOTE:  Transition from APR=>ALT implies the end of APR mode
//         In this case, new mode is ROL + ALT
//-----------------------------------------------------------------------
void AutoPilot::EnterALT()
{ if (vStat == AP_VRT_ALT)	return;
	aprm	= 0;
  StateChanged(AP_STATE_ALT);       // Warn Panel
  vStat = AP_VRT_ALT;               // Lock on altitude
  pidL[PID_ALT]->Init();            // init PID
  return;
}
//-----------------------------------------------------------------------
//  Enter ROLL Mode
//-----------------------------------------------------------------------
void AutoPilot::EnterROL()
{ pidL[PID_ROL]->Init();
  pidL[PID_BNK]->Init();
  lStat   = AP_LAT_ROL;
  signal  = SIGNAL_OFF;
  StateChanged(AP_STATE_ROL);
  return;
}
//-----------------------------------------------------------------------
//  Enter HEADING Mode
//-----------------------------------------------------------------------
void AutoPilot::EnterHDG()
{ aprm  = 0;
  pidL[PID_HDG]->Init();
  pidL[PID_BNK]->Init();
  pidL[PID_HDG]->SetTarget(0);
  signal  = SIGNAL_OFF;
  lStat   = AP_LAT_HDG;
  StateChanged(AP_STATE_HDG);
  return;
}
//-----------------------------------------------------------------------
//  Enter HEADING Mode only
//-----------------------------------------------------------------------
void AutoPilot::OnlyHDG()
{ EnterHDG();
  vStat   = AP_DISENGD;
  StateChanged(AP_STATE_VOF);
  EnterVSP();
  return;
}

//-----------------------------------------------------------------------
//  Compute a direction according to the current state
//  Non approach mode (aprm == 0)
//		Compte a direction that will cross the expected radial given
//		by Radio->hREF
//	Approach Mode normal (aprm = 1) 
//		a)	We are in sector TO
//				Compute a direction that cross the runway direction at 90°
//		b)  We are in sector FR
//				Compute direction opposite to the runway direction to catch
//				the TO sector
//
//-----------------------------------------------------------------------
void AutoPilot::GetCrossHeading()
{ //--- normal approach ------------------------
  if ((aprm) && (sect))
	{	lStat = AP_LAT_LT1;
	  double cor	= (Radio->rDEV > 0)?(+xAPW):(-xAPW);
		double qrt  = cor * 0.25;
		xHDG				= Wrap360(Radio->hREF + cor);
		xCOR				= Wrap360(Radio->hREF + qrt);
	  //TRACE("CROSS (aprm) hREF=%.4f rDEV=%.4f xHDG=%.4f",Radio->hREF,Radio->rDEV,xHDG);
	  //	trace = 1;
		return;
	}
	//--- Approach in sector FR -----------------------
	//--- Must do a go arround to catch direction -----
	if (aprm)
	{ lStat				= AP_LAT_LT0;
		xHDG				= Wrap360(Radio->hREF + 180);
		rHDG				= xHDG;
		return;
	}
	//--- NAV mode: Direction to cross Radial ---------
	lStat = AP_LAT_LT1;
	double dev    = Wrap180(aHDG - Radio->hREF);
	//TRACE("CROSS hREF=%.4f aHDG=%.4f dev=%.4f rHDG=%.4f",Radio->hREF,aHDG,dev,rHDG);
  xHDG				= rHDG;							// Set cross heading as actual
	if ((dev > -90) || (dev < +90))	return;
	double cor	= (Radio->hDEV > 0)?(-90):(+90);
	xHDG				= Wrap360(Radio->hREF + cor);
	rHDG				= xHDG;
	//TRACE("CHANGE rHDG=%.4f",rHDG);
  return;
}
//-----------------------------------------------------------------------
//  Enter Lateral Mode Leg 1 for tracking VOR
//-----------------------------------------------------------------------
void AutoPilot::EnterNAV()
{ if (BadSignal(SIGNAL_VOR))  return;
  aprm = 0;
  pidL[PID_HDG]->Init();
  pidL[PID_BNK]->Init();
  StateChanged(AP_STATE_NAV);
  sEVN  = AP_STATE_NTK;										// Next state
  //---Compute direction to catch the radial -----
  GetCrossHeading();
	return;
}
//-----------------------------------------------------------------------
//  Swap ALTITUDE HOLD Mode
//-----------------------------------------------------------------------
void AutoPilot::SwapALT()
{ if (vStat == AP_VRT_ALT)  return EnterVSP();
  //--- Enter Altitude mode -------------
  EnterALT();
  //--- Leave approach mode if needed ---
  if (aprm)   EnterROL();
	aprm	= 0;
  return;
}
//-----------------------------------------------------------------------
//  Enter Lateral Mode Leg 1 for tracking ILS
//-----------------------------------------------------------------------
void AutoPilot::EnterAPR()
{ rend	= 0;
	StateChanged(AP_STATE_ACH);
	if (BadSignal(SIGNAL_ILS))    return;
  aprm  = 1;
  alta  = 0;                        // ALT armed off
  pidL[PID_HDG]->Init();
  pidL[PID_BNK]->Init();
	rHDG	= aHDG;											// Target heading = actual
  StateChanged(AP_STATE_APR);
  sEVN  = AP_STATE_ATK;             // next state
	//--- Get landing data  ---------------------------------
	rend		= Radio->rSRC->GetLandSpot();
	//TRACE("EnterAPR lndDIR=%.4f",rend->lnDIR);
  //---Compute a direction perpendicular to the radial ----
  GetCrossHeading();
  //--Init vertical mode -----
  pidL[PID_GLS]->Init();
  vStat = AP_VRT_GSW;
	//	TRACE("EnterAPR elvT=%.4f eVRT=%.4f AOA=%.4f",elvT->Val(),eVRT,pidL[PID_AOA]->GetVN());
  return;
}
//-----------------------------------------------------------------------
//  Catch vertical speed and normalize to multiple of 100 feet 
//-----------------------------------------------------------------------
double AutoPilot::CatchVSP()
{ if (0 == uvsp) return 0;
  Send_Message(&mVSI);											// Get actual VSI
  int    vsi = int(mVSI.realData / 100);    // integer part
  double rst = fmod(mVSI.realData,100 );    // fract part
  double rnd = (rst > 50)?(100):(0);
  double rvs = (double(vsi) * 100) + rnd;
  if (rvs < -2000) rvs = -2000;
  if (rvs > +2000) rvs = +2000;
  return rvs;
}
//-----------------------------------------------------------------------
//  Enter vertical speed
//-----------------------------------------------------------------------
void AutoPilot::EnterVSP()
{ eVSP  = 0;
  aprm  = 0;
  vStat = AP_DISENGD;						// VSP holder
  StateChanged(AP_STATE_VOF);
  if (0 == uvsp) return;
  pidL[PID_VSP]->Init();
  pidL[PID_AOA]->Init();
  elvT->Neutral();
  rVSP	= CatchVSP();           // Get VSI
  vStat = AP_VRT_VSP;           // VSP holder
  StateChanged(AP_STATE_VSP);   // state change
  return;
}
//-----------------------------------------------------------------------
//  Enter flare mode
//  Fix the airspeed and change state
//	Touch down offset is the new traget point due to change of slope
//	in flare mode
//-----------------------------------------------------------------------
void AutoPilot::EnterFLR()
{ aprm	= 0;
	EnterROL();
  afps	= aSPD * FEET_PER_NM * HOUR_PER_SEC;         // In feet per sec
  vStat = AP_VRT_FLR;                               // Vertical state
	//  TRACE("ENTER FLARE");
	//--- Compute touch down offset from ILS ----------
	double tdp = aLND / sTAN;
	nTDP	= Radio->fdis - tdp ;
	//	globals->Trace.Set(TRACE_WHEEL);
	mveh->SetABS(1);
  return;
}
//-----------------------------------------------------------------------
//	Enter final leg
//	-Set final state
//	-Locate ILS far point and set as ground target
//
//-----------------------------------------------------------------------
void AutoPilot::EnterFIN()
{	//--- Disengage and set aircraft level --------------- 
	lStat   = AP_LAT_ROL;
  vStat   = AP_VRT_FIN;
  StateChanged(AP_VRT_FIN);
  ailS->PidValue(0);							// Set Level
  rudS->PidValue(0);							// Rudder to 0
	mveh->SetABS(0);
	//--- push plane on ground ---------------------------------
	ailS->Neutral();
	elvT->SetValue(0);
	elvS->SetValue(0.1f);
	rudS->Neutral();
	lStat		= AP_LAT_GND;
	if (rend)		return;
	Disengage(1);
	return;
}
//-----------------------------------------------------------------------
//  Inc ALT
//-----------------------------------------------------------------------
void AutoPilot::IncALT()
{ if (aprm)   return;
  rALT.Inc(100,4);
  StateChanged(AP_STATE_ACH);
  return;
}
//-----------------------------------------------------------------------
//  Change Altitude
//-----------------------------------------------------------------------
void AutoPilot::ChangeALT(double a)
{ if (aprm)									return;
  if (vStat != AP_VRT_ALT)	return;
  rALT.Set(RoundAltitude(a),4);
  StateChanged(AP_STATE_ACH);       // Warn Panel
	alta  = 1;
  return;
}
//-----------------------------------------------------------------------
//  Set Landing
//-----------------------------------------------------------------------
void AutoPilot::SetLandingMode()
{	NewEvent(AP_EVN_APR);
	return;
}
//-----------------------------------------------------------------------
//  Inc ALT
//-----------------------------------------------------------------------
void AutoPilot::DecALT()
{ if (aprm)   return;
	rALT.Dec(100, 500);	
  StateChanged(AP_STATE_ACH);
  return;
}
//-----------------------------------------------------------------------
//  Button UP
//-----------------------------------------------------------------------
void AutoPilot::IncVSP()
{ if (aprm)                 return;
  if (rVSP >= +vLim)        return;
  rVSP += 100;
  StateChanged(AP_STATE_VCH);       // VSP change
  return;
}
//-----------------------------------------------------------------------
//  Button DN
//-----------------------------------------------------------------------
void AutoPilot::DecVSP()
{ if (aprm)                 return;
  if (rVSP <= -vLim)        return;
  rVSP -= 100;
  StateChanged(AP_STATE_VCH);       // VSP change
  return;
}
//-----------------------------------------------------------------------
//  PROCES EVENT in DISENGAGED STATE
//-----------------------------------------------------------------------
void AutoPilot::StateDIS(int evn)
{ //--- Process only autopilot engage ------------------
  if (AP_EVN_ENG != evn)  return;
	cALT	= altS->GaugeBusFT01();					// Current altitude
	rHDG	= cmpS->GaugeBusFT01();					// Current mag heading
	aSPD  = mveh->GetPreCalculedKIAS();		// Current speed
  EnterINI(); 
  return;
}
//-----------------------------------------------------------------------
//  PROCES EVENT in ROLL STATE
//-----------------------------------------------------------------------
void AutoPilot::StateROL(int evn)
{ switch (evn)  {
  //----ENG button --------------------
  case AP_EVN_ENG:
    return Disengage(0);
  //--- HDG button --------------------
  case AP_EVN_HDG:
    return EnterHDG();
  //--- NAV button --------------------
  case AP_EVN_NAV:
    return EnterNAV();
  //--- APR  button--------------------
  case AP_EVN_APR:
    return EnterAPR();
  //--- REV button --------------------
  case AP_EVN_REV:
    return;
  //--- ALT button --------------------
  case AP_EVN_ALT:
    return SwapALT();
  }
  return;
}
//-----------------------------------------------------------------------
//  PROCES EVENT in HEADING STATE
//-----------------------------------------------------------------------
void AutoPilot::StateHDG(int evn)
{ switch (evn)  {
  //----ENG button ----------------
  case AP_EVN_ENG:
    return Disengage(0);
  //--- HDG button ----------------
  case AP_EVN_HDG:
    return ExitHDG();
  //--- NAV button ----------------
  case AP_EVN_NAV:
    return EnterNAV();
  //--- APR button ----------------
  case AP_EVN_APR:
    return EnterAPR();
  //--- REV button ----------------
  case AP_EVN_REV:
    return;
    //--- ALT button --------------
  case AP_EVN_ALT:
    return SwapALT();
  }
  return;
}
//-----------------------------------------------------------------------
//  PROCES EVENT in LATERAL LEG 1
//-----------------------------------------------------------------------
void AutoPilot::StateLAT(int evn)
{ switch (evn)  {
  //----ENG button ------------------------
  case AP_EVN_ENG:
    return Disengage(0);
  //--- HDG button ------------------------
  case AP_EVN_HDG:
    return OnlyHDG();
  //--- NAV button ------------------------
  case AP_EVN_NAV:
    return ExitNAV();
  //--- APR button ------------------------
  case AP_EVN_APR:
    return ExitAPR();
  //--- REV button ------------------------
  case AP_EVN_REV:
    return EnterINI();
  //--- ALT button --------------
  case AP_EVN_ALT:
    return SwapALT();

  }
  return;
}
//-----------------------------------------------------------------------
//  PROCES EVENT in Ground steering
//-----------------------------------------------------------------------
void AutoPilot::StateGND(int evn)
{	if (AP_EVN_ENG == evn)	return Disengage(1);
	rudS->Neutral();
	ailS->Neutral();
	lStat = AP_LAT_ROL;
	StateROL(evn);
	return;
}
//-----------------------------------------------------------------------
//  Dispatch event to current state
//-----------------------------------------------------------------------
void AutoPilot::NewEvent(int evn)
{ if (0 == inUse)         return;
  if (!active)            return;
  if (AP_EVN_BUP == evn)  return IncVSP();
  if (AP_EVN_BDN == evn)  return DecVSP();
  if (AP_EVN_AUP == evn)  return IncALT();
  if (AP_EVN_ADN == evn)  return DecALT();
  switch (lStat)  {
    case AP_DISENGD:
      return StateDIS(evn);
    case AP_LAT_ROL:
      return StateROL(evn);
    case AP_LAT_HDG:
      return StateHDG(evn);
		case AP_LAT_LT0:
			return StateLAT(evn);
    case AP_LAT_LT1:
      return StateLAT(evn);
    case AP_LAT_LT2:
      return StateLAT(evn);
		case AP_LAT_TGA:
			return StateLAT(evn);
		case AP_LAT_GND:
			return StateGND(evn);
  }
  return;
}
//-----------------------------------------------------------------------
//  AUTO PILOT INIT
//-----------------------------------------------------------------------
bool AutoPilot::Init()
{ elvT->Neutral();									// Reset elevator
	rudS->Neutral();									// Reset rudder
	ailS->Neutral();									// Reset ailerons
	EnterALT();
  EnterROL();
	StateChanged(AP_STATE_ALT);
	return true;
}
//-----------------------------------------------------------------------
//  AUTO PILOT ENGAGE
//-----------------------------------------------------------------------
bool AutoPilot::Engage()
{ if (0 == Powr)						return false;
	if (lStat != AP_DISENGD)	return true;
  NewEvent(AP_EVN_ENG);
  return true;
}
//-----------------------------------------------------------------------
//	External interface to enter take-off mode
//	Check all pre-conditions
//-----------------------------------------------------------------------
int AutoPilot::EnterTakeOFF(char x, LND_DATA *rwd)
{	wgrd  = mveh->WheelsAreOnGround();
	rend	= rwd;
	if (vROT <= 0)						return 1;					// Vrot present
	if (0 == rend)						return 2;					// Runway end position
	if (!wgrd)								return 3;					// Wheels on ground
	if (0 == sreg)						return 4;					// No speed regulator
	if (AP_LAT_ROL != lStat)	return 6;					// Initial lateral mode
	if (AP_VRT_VSP != vStat)	return 7;					// Initial vertical mode
	if (!mveh->AllEngineOn())	return 8;					// All engine running
	//----- Engage Throttle control ----
	ugaz = SetGasControl(1);
	//--- Engage steering mode ---------
	sreg->SteerTo(rend->refP);
	//--- Trim all surfaces ------------
	ailS->Neutral();
	rudS->Neutral();
	elvT->Neutral();
	//---- Lateral state to steer mode --
	lStat	= AP_LAT_GND;
	vStat = AP_VRT_TKO;
	//--- Externally controlled ---------
	xCtl	= x;
	return 0;
}
//-----------------------------------------------------------------------
//	GPS interface to enter automatic mode
//	Check all pre-conditions
//-----------------------------------------------------------------------
bool AutoPilot::EnterGPSMode()
{	if (0 != xCtl)						return false;
	if (0 == Radio)						return false;
	if (cAGL < 500)						return false;
	if (AP_DISENGD == vStat)	return false;
	if (AP_VRT_TKO == vStat)	return false;
	if (0 == sreg)						return false;
	return true;
}
//-----------------------------------------------------------------------
//	Get More speed
//-----------------------------------------------------------------------
void AutoPilot::MoreSpeed()
{	double more = cRAT *1.01;
	double lim  = (cAGL <  aFSP)?(fSPD):(xRAT);
	lim *= 1.5;
	sAPR = (more > lim)?(lim):(more);
}
//-----------------------------------------------------------------------
//	Get More speed
//-----------------------------------------------------------------------
void AutoPilot::LessSpeed()
{	double less = cRAT * 0.99;
	double lim  = (cAGL <  aFSP)?(fSPD):(xRAT);
	lim *= 0.80;
	sAPR = (less < lim)?(lim):(less);
}

//-----------------------------------------------------------------------
//	Select  Speed to hold
//-----------------------------------------------------------------------
double AutoPilot::SelectSpeed()
{	double more = cRAT * 1.01;
  double evrt = -eVRT;
	
	switch (vStat)	{
	  //--- ground  final --------------------------
		case AP_VRT_FLR:
			return 0;
		case AP_VRT_FIN:
			return 0;
		//--- Tracking glide in final ----------------
		case AP_VRT_GST:
			if (sect == 0)			return xRAT;
			if (cAGL <= aCUT)		return fSPD;
			if (evrt >  vrub)		return fSPD;
			if (evrt <  vrlb)		return more;			// Was 0.8
			if (cAGL <  aFSP)		return fSPD;
			return  xRAT;
		//--- take-off -------------------------------
		case AP_VRT_TKO:
			return (cRAT>=20)?(1000):(20);
		//--- climbing -------------------------------
		case AP_VRT_VSP:
			if (rVSP > 0)			return 1000;
			return xRAT;
		//--- Altitude mode --------------------------
		case AP_VRT_ALT:
			if (eVRT > 100)	  return 1000;			// Going up
			//-- when going down, set limit to VSI -----
			//double vsi = CatchVSP();
			//if (vsi < -800)		return more;			
			return xRAT;
	}

	//--- return cruise speed --------------------
	return xRAT;
}
//-----------------------------------------------------------------------
//  External Request to enter lateral NAV mode
//-----------------------------------------------------------------------
void AutoPilot::SetWPTmode(double alt)
{	elvT->PidValue(0);					// Inhibit boost
	EnterALT();
	EnterNAV();
	ChangeALT(alt);
	return;
}
//-----------------------------------------------------------------------
//  Swap GAS control
//-----------------------------------------------------------------------
void AutoPilot::SwapGasControl()
{	ugaz ^= 1;
	ugaz  = SetGasControl(ugaz);
	return;
}
//-----------------------------------------------------------------------
//  GAS control
//	NOTE: When throttle is controlled by autopilot, ailerons
//	and elevators are disconnected from pilot control.
//	To recover control, throttle must be moved
//-----------------------------------------------------------------------
char AutoPilot::SetGasControl(char s)
{	if (0 == sreg)						return 0;
	if (AP_DISENGD == lStat)	return 0;
	//--- Set/ Stop gas control ------------------
	if (0 == s)	
	{	sreg->SetOFF();	ailS->Neutral();}											
  else				
	s = (sreg->SetON(JS_SURF_APL))?(1):(0);			
	return s;
}
//-----------------------------------------------------------------------
//  Dispatch event altitude alert
//-----------------------------------------------------------------------
void AutoPilot::ALTalertSWP()
{ if (aprm)       return;           // Ignore in approach mode
  alta ^= 1;                        // Toggle ALT arm
  StateChanged(AP_STATE_AAA);       // State is changed
  return;
}
//-----------------------------------------------------------------------
//  Set altitude alert
//-----------------------------------------------------------------------
void AutoPilot::ALTalertSET()
{ if (aprm)       return;           // Ignore in approach mode
  alta  = 1;                        // ALT armed
  StateChanged(AP_STATE_AAA);       // State is changed
  return;
}
//--------------------------------------------------------------------
//  Edit Autopilot data
//--------------------------------------------------------------------
void AutoPilot::Probe(CFuiCanva *cnv)
{ cnv->AddText(1,1,"%s(%d)-%s-ABRT:%d",autoTB1[lStat],stga,autoTB2[vStat],abrt);
	//------------------------------------------------------------------
  cnv->AddText( 1,1,"GAS:%d-SPEED:%.02f",ugaz,cRAT);
	cnv->AddText( 1,1,"rHDG: %.5f",rHDG);
  cnv->AddText( 1,1,"aHDG: %.5f",aHDG);
  cnv->AddText( 1,1,"hERR: %.5f",hERR);
	cnv->AddText( 1,1,"eVRT: %.4f",eVRT);
	cnv->AddText( 1,1,"rDIS: %.5f",rDIS);
  if (Radio)
  { cnv->AddText( 1,1,"hREF %.5f",Radio->hREF);
		cnv->AddText( 1,1,"hDEV %.5f",Radio->hDEV);
		cnv->AddText( 1,1,"Feet %.0f",Radio->fdis);
  }
	if ((lStat == AP_LAT_LT0) || (lStat == AP_LAT_LT1)) 
  { cnv->AddText( 1,1,"vHRZ %.5f",vHRZ);
  }
	if (lStat == AP_LAT_LT2)
	{	cnv->AddText( 1,1,"cFAC %.5f",cFAC);
	}
	if ((aprm)|| (vStat == AP_VRT_ALT))
	{	cnv->AddText( 1,1,"rALT %.0f",rALT.Get());
	}
  if (vStat == AP_VRT_VSP)
  { cnv->AddText( 1,1,"eVSP %.5f",eVSP);
		cnv->AddText( 1,1,"rVSP %.5f",rVSP);
  }
	if (vStat == AP_VRT_FLR)
  { cnv->AddText( 1,1,"dTDP %.0f",dTDP);	}
	if (lStat == AP_LAT_GND)
  { cnv->AddText( 1,1,"Sector %03u",rend->sect);	}
  return;
}
//-----------------------------------------------------------------------
//  Glut to display ground deviation
//-----------------------------------------------------------------------
void AutoPilot::DisplayGroundDeviation(double p1)
{	char txt[128];
	_snprintf(txt,126,"rHDG =%.2f hERR=%.4f Rudr=%.4f",rHDG,hERR,p1);
	DrawNoticeToUser(txt,2);
}
//========================================================================================
//
// CKAP140Panel
//
//  NOTE: All display fields starts at CA11
//========================================================================================
CKAP140Panel::CKAP140Panel (void) : AutoPilot()
{ TypeIs (SUBSYSTEM_KAP140_PANEL);
  flsh   = 0;
  uvsp   = AP_VRT_VSP;              // use VSP
  //---Init display fields -----------------------------------
  InitField(K140_FD_HD1,0,RADIO_CA11);
  InitField(K140_FD_HD2,0,RADIO_CA12);
  InitField(K140_FD_VT1,0,RADIO_CA13);
  InitField(K140_FD_VT2,0,RADIO_CA14);
  InitField(K140_FD_VSI,0,RADIO_CA18);
  InitField(K140_FD_ALT,0,RADIO_CA19);
  ClearAll();
  //--- Add disconnect sound ---------------------------------
  CAudioManager *snd = globals->snd;
  sbf = snd->ReserveSoundBUF('k4ds',"Beep_24.wav",0);
}
//--------------------------------------------------------------------------
//  Init a radio field 
//  -data is the char field
//  -No is the display field number
//--------------------------------------------------------------------------
void CKAP140Panel::InitField(short No,char * data, short cf)
{ fldTAB[No].sPos   = cf;
  fldTAB[No].data   = data;
  fldTAB[No].state  = RAD_ATT_INACT;
  return;
}
//--------------------------------------------------------------------
//  Activate a field
//--------------------------------------------------------------------
void CKAP140Panel::Lite(char No,char *txt)
{ fldTAB[No].state = RAD_ATT_ACTIV;
  fldTAB[No].data  = txt;
  return;
}
//--------------------------------------------------------------------
//  Modify a field
//--------------------------------------------------------------------
void CKAP140Panel::CopyField(char No,RADIO_FLD *fld)
{ fldTAB[No].state = fld->state;
  fldTAB[No].data  = fld->data;
  return;
}
//--------------------------------------------------------------------
//  Clear a field
//--------------------------------------------------------------------
void CKAP140Panel::ClearField(int No)
{ fldTAB[No].state    = RAD_ATT_INACT;
  fldTAB[No].data     = 0;
  return;
}
//--------------------------------------------------------------------
//  Pop up a field
//  The odd field (1,3,5, etc) is ligthed with text of next field number
//  Result:  Lighted text seems to change from one field to the next
//--------------------------------------------------------------------
void CKAP140Panel::PopField(int No)
{ int pr = No - 1;
  fldTAB[pr].data   = fldTAB[No].data;
  fldTAB[pr].state  = fldTAB[No].state;
  fldTAB[No].data   = 0;
  fldTAB[No].state  = RAD_ATT_INACT;
  return;
}
//--------------------------------------------------------------------
//  Clear all fields
//--------------------------------------------------------------------
void CKAP140Panel::ClearAll()
{ ClearField(K140_FD_HD1);
  ClearField(K140_FD_HD2);
  ClearField(K140_FD_VT1);
	ClearField(K140_FD_VT2);
  ClearField(K140_FD_VSI);
  ClearField(K140_FD_ALT);
  return;
}
//--------------------------------------------------------------------
//  Autodisconnect
//--------------------------------------------------------------------
void CKAP140Panel::Alarm()
{ globals->snd->Play(sbf);
  return;
}
//--------------------------------------------------------------------
//  DISPATCHER:  Click Event coming from the gauge
//--------------------------------------------------------------------
int CKAP140Panel::Dispatch(int evn)            // Dispatching 
{ if (0 == Powr)            return 0;
  switch (evn)  {
    //---Autopilot engage ----Swap engage indicator -----
    case RADIO_CA01:
      NewEvent(AP_EVN_ENG);
      return 0;
    //---Heading mode -----------------------------------
    case RADIO_CA02:
      NewEvent(AP_EVN_HDG);
      return 0;
    //---Navigation mode --------------------------------
    case RADIO_CA03:
      NewEvent(AP_EVN_NAV);
      return 0;
    //---APPROACH MODE ---------------------------------
    case RADIO_CA04:
      NewEvent(AP_EVN_APR);
      return 0;
    //---ALTITUDE HOLD ---------------------------------
    case RADIO_CA07:
      NewEvent(AP_EVN_ALT);
      return 0;
    //--Button UP --------------------------------------
    case RADIO_CA09:
      NewEvent(AP_EVN_BUP);
      return 0;
    //--Button Down ------------------------------------
    case RADIO_CA08:
      NewEvent(AP_EVN_BDN);
      return 0;
    //---ALTITUDE SELECT -------------------------------
    case RADIO_CA10:
      ALTalertSET();
      if (msDIR == -1)  NewEvent(AP_EVN_ADN);
      else              NewEvent(AP_EVN_AUP);
      return 0;
    //---- Arm ALTITUDE ALERT ---------------------------
    case RADIO_CA20:
      ALTalertSWP();
      return 0;
		//---- Arm/disarm throttle control -------------------------
		case RADIO_CA22:
			SwapGasControl();
			return 0;

  }
  return evn;
}
//--------------------------------------------------------------------
//  Power ON Event
//--------------------------------------------------------------------
int CKAP140Panel::PowerON()
{ return 0;
}
//--------------------------------------------------------------------
//  Power OF Event
//--------------------------------------------------------------------
int CKAP140Panel::PowerOF()
{ PowerLost();
  ClearAll();
  return 0;
}
//--------------------------------------------------------------------
//  Time slice autopilot 
//--------------------------------------------------------------------
void CKAP140Panel::TimeSlice(float dT,U_INT FrNo)
{ AutoPilot::TimeSlice(dT,FrNo);
  flsh      = Flash() & globals->clk->GetON();
  char old  = Powr;
  Powr      = active;
  if (old == Powr) return;
  if (Powr == 1) PowerON();
  if (Powr == 0) PowerOF();
  return;
}
//--------------------------------------------------------------------
//  Edit VSP
//--------------------------------------------------------------------
void CKAP140Panel::EditVSP()
{ int   pm = int(rVSP);
  sprintf_s(vsp,8,"%+04d",pm);
  vsp[7]   = 0;
  Lite(K140_FD_VSI,vsp);
  return;
}
//--------------------------------------------------------------------
//  Edit ALT
//--------------------------------------------------------------------
void CKAP140Panel::EditALT()
{ double a= rALT.Tvl();
	int  pm = int(RoundAltitude(a));
  sprintf_s(alt,8,"%05d",pm);
  alt[7] = 0;
  Lite(K140_FD_ALT,alt);
  return;
}
//--------------------------------------------------------------------
//  Autopilot change state 
//  Process light according to new state
//--------------------------------------------------------------------
void CKAP140Panel::StateChanged(U_CHAR evn)
{ switch (evn)  {
    //---AUTO PILOT is disengaged -------------
    case AP_STATE_DIS:
      ClearAll();
      return;
    //---ROLL MODE ------------------------
    case AP_STATE_ROL:
      Lite(K140_FD_HD1,"ROL");
      ClearField(K140_FD_HD2);
      return;
    //---HEADING MODE ------------------------
    case AP_STATE_HDG:
      Lite(K140_FD_HD1,"HDG");
      ClearField(K140_FD_HD2);
      return;
    //---Head to NAV MODE --------------------
    case AP_STATE_NAV:
      ClearField(K140_FD_HD1);
      Lite(K140_FD_HD2,"NAV");
      return;
    //---Track NAV by OBS --------------------
    case AP_STATE_NTK:
      PopField(K140_FD_HD2);
      return;
    //---APR mode ILS MODE --------------------
    case AP_STATE_APR:
      ClearField(K140_FD_HD1);
      ClearField(K140_FD_VSI);
      Lite(K140_FD_HD2,"APR");
      Lite(K140_FD_VT2,"GS ");
      return;
      return;
    //---ILS TRACK MODE --------------------
    case AP_STATE_ATK:
      PopField(K140_FD_HD2);
      return;
    //---GLIDE TRACK MODE --------------------
    case AP_STATE_VTK:
      PopField(K140_FD_VT2);
      return;
    //---ALTITUDE HOLD MODE ------------------
    case AP_STATE_ALT:
      EditALT();
      Lite(K140_FD_VT1,"ALT");
      ClearField(K140_FD_VSI);
      return;
    case AP_STATE_ACH:
      EditALT();
      return;
    //---VSP HOLD MODE ------------------
    case AP_STATE_VSP:
      Lite(K140_FD_VT1,"VSP");
      EditVSP();
      return;
    //---VERTICAL HOLD OFF ------------------
    case AP_STATE_VOF:
      ClearField(K140_FD_VT1);
      ClearField(K140_FD_VSI);
      return;
    //--VSI change -----------------------
    case AP_STATE_VCH:
      EditVSP();
      return;
    //---Altitude alert -------------------
    case AP_STATE_AAA:
      CopyField(K140_FD_VT2,alertK140 + armALT());
      return;
  }
  //-------------------------------------------------
  return;
}
//==================================================================
// CAFCS85Panel
//==================================================================
CAFCS85Panel::CAFCS85Panel (void)
{ TypeIs (SUBSYSTEM_AFCS85_PANEL);
}
//-------------------------------------------------------------------
//  Read parameters
//-------------------------------------------------------------------
int CAFCS85Panel::Read(SStream *st,Tag tag)
{ return AutoPilot::Read(st,tag);
}

//===============================================================================
//
// CKAP150Panel
//===============================================================================
CKAP150Panel::CKAP150Panel (void)
{
  TypeIs (SUBSYSTEM_K150_PANEL);
}

//
// CKAP200Panel
//
CKAP200Panel::CKAP200Panel (void)
{
  TypeIs (SUBSYSTEM_K200_PANEL);
}

//
// CAFCS65Panel
//
CAFCS65Panel::CAFCS65Panel (void)
{
  TypeIs (SUBSYSTEM_AFCS65_PANEL);
}

