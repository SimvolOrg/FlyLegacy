/*
 * RadioGauges.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005 Chris Wallace
 * Copyright since 2007 Jean Sabatier
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

#include "../Include/FlyLegacy.h"
#include "../Include/Globals.h"
#include "../Include/RadioGauges.h"
//=================================================================================
//  Common Radio Gauge
//=================================================================================
CRadioGauge::CRadioGauge(CPanel *mp)
:CGauge(mp)
{ arm   = 0;
  PowST	= 0;
	//--- init click mask --------------------------------------------
	cMsk	= (AREA_DIGI + AREA_BUTN);
  //----------------------------------------------------------------
  //amber = MakeRGB (215, 90, 0);
  amber = MakeRGB(255,73,33);
  rFont = (CFont*)globals->fonts.ftradi9.font;
  hCar  = rFont->CharHeight('H');
  wCar  = rFont->CharWidth (' ');
}
//------------------------------------------------------------------
//  Destroy the gauge
//------------------------------------------------------------------
CRadioGauge::~CRadioGauge()
{ 
}
//------------------------------------------------------------------
//  Change font
//------------------------------------------------------------------
void CRadioGauge::ChangeFont(char *fn)
{ if (strcmp(fn,"big") != 0)  gtfo("unsupported font");
  rFont = (CFont*)globals->fonts.ftmono14.font;
  hCar  = rFont->CharHeight('H');
  wCar  = rFont->CharWidth (' ');
  return;
}
//------------------------------------------------------------------
//  Check if button may be dupplicated
//------------------------------------------------------------------
bool CRadioGauge::BtCopy()
{	return (brow.HasTexture())?(false):(true);
}
//------------------------------------------------------------------
//  Copy the parameters
//------------------------------------------------------------------
void CRadioGauge::Copy(CRadioGauge &rg)
{ rFont = rg.rFont;
  hCar  = rg.hCar;
  wCar  = rg.wCar;
  amber = rg.amber;
  SetSurface(dsw,dsh);
  return;
}
//------------------------------------------------------------------
//  Read parameters
//------------------------------------------------------------------
int CRadioGauge::Read (SStream *stream, Tag tag)
{ char txt[16];
  Tag  rad;
  int unit;
    switch (tag)  {
    //-- Decode extra tags ---------------
      case 'font':
        ReadString(txt,16,stream);
        ChangeFont(txt);
        return TAG_READ;
      case 'brow':
        brow.SetGauge(this);
        ReadFrom(&brow,stream);
        return TAG_READ;
      //-- Decode projector ----------------
      case 'proj':
        DecodePROJ(stream,quad,0);
        return TAG_READ;
      //--- Associated radio unit ---------------
      case 'radi':
        ReadTag (&rad, stream);
        ReadInt (&unit, stream);
        mesg.group          = rad;
        mesg.user.u.unit    = unit;
        mesg.id             = MSG_GETDATA;
        return TAG_READ;
      //--- Display area ------------------------
      case 'disp':
        DecodeDisplay(stream);
        SetSurface(dsw,dsh);
        return TAG_READ;
  }
  return CGauge::Read(stream, tag);
}
//=================================================================================
//  Gauge for Bendix King KX155 radio
//=================================================================================
CK155gauge::CK155gauge (CPanel *mp)
: CRadioGauge(mp)
{ Prop.Rep(NO_SURFACE + IS_DISPLAY);
  RAD   = 0;
  PowST = 0;
  //---Default display area ----------------------------------------
  xOrg  = 0;
  yOrg  = 0;
  dsw   = 208;
  dsh   =  26;
}
//------------------------------------------------------------------
//  Copy from similar gauge
//------------------------------------------------------------------
void CK155gauge::CopyFrom(SStream *stream)
{ CK155gauge *g = (CK155gauge*)LocateGauge(stream);
  CGauge::CopyFrom(*g);
  CRadioGauge::Copy(*g);
  for (int k=1; k<K55EV_AREA_SZ; k++) ca[k].CopyFrom(g->ca[k],this);
  return;
}
//------------------------------------------------------------------
//  Read parameters
//------------------------------------------------------------------
int CK155gauge::Read (SStream *stream, Tag tag)
{ int ica = 0;
  int typ = ClickAreaFromTag (tag,&ica);
  if ((ica >= 1) && (ica < K55EV_AREA_SZ)) {
    // Valid click area
    ca[ica].Init(this,typ);
    ReadFrom (&ca[ica], stream);
    ca[ica].BuildQuad();
    return TAG_READ;
  }
  switch (tag) {
		case 'shap':
			DecodeSHAP(stream,tour,0);
			BuildQuad(quad,tour);
			cMsk	= AREA_BUTN;					// Restrict click area
			return TAG_READ;

    case 'copy':
      CopyFrom(stream);
      return TAG_READ;
  }

  return CRadioGauge::Read (stream, tag);
}

//------------------------------------------------------------------
//  All parameters are read
//------------------------------------------------------------------
void CK155gauge::ReadFinished()
{ CRadioGauge::ReadFinished ();
  //----Define display window ------------------------------
  yOfs  = 0;
  return;
}
//------------------------------------------------------------------
//  Collect VBO data
//------------------------------------------------------------------
void CK155gauge::CollectVBO(TC_VTAB *vtb)
{	CGauge::CollectVBO(vtb);
	for (int k=1; k<K55EV_AREA_SZ; k++)	ca[k].CollectVBO(vtb);
	return;
}
//-----------------------------------------------------------------------------
//  Get the subsystem K155
//-----------------------------------------------------------------------------
void CK155gauge::GetK155()
{ //----Get K155 status ----------------------------
  mesg.user.u.datatag = 'gets';
  Send_Message(&mesg);
  RAD = (CK155radio*) mesg.voidData;
  if (0 == RAD) return;
  //----Define CDI parameters ------------------------------
  Ampli = rFont->CharWidth('-') * RAD->GetCDIdim();
  Ampli = (Ampli >> 1);
  cdiND = ca[RADIO_CA16].x1 + Ampli - 1;
  return;
}
//----------------------------------------------------------------------------
//  Draw button according to power
//----------------------------------------------------------------------------
void CK155gauge::DrawBUT(U_CHAR pwr)
{ //-----Display frame buttons -----------------------
	if (brow.Draw(pwr))		return;
  ca[RADIO_BT22].Draw (pwr);            // COM MODE
  ca[RADIO_BT27].Draw (pwr);            // NAV MODE
  ca[RADIO_BT19].Draw (pwr);            // COM Transfer
  ca[RADIO_BT24].Draw (pwr);            // NAV Transfer
  return; }
//------------------------------------------------------------------
//  Draw Field
//------------------------------------------------------------------
void CK155gauge::DrawField(RADIO_FLD *fd)
{ U_CHAR mask = RAD->mskFS;
  if (0 == fd->state) return;
  if ((RAD_ATT_FLASH == fd->state) && (0 == mask))  return;
  short No  = fd->sPos;
  short x0  = ca[No].x1 - xOrg;
  short y0  = ca[No].y1 - yOrg;
  rFont->DrawNText(surf,x0,y0,amber,fd->data);
  return;
}
//------------------------------------------------------------------
//  Draw COM
//------------------------------------------------------------------
void CK155gauge::DrawCOM()
{ for (short No = K155_ACOM_WP; No < K155_DCOM_SZ; No++)  DrawField(RAD->comTAB + No);
  return;
}
//------------------------------------------------------------------
//  Draw NAV
//------------------------------------------------------------------
void CK155gauge::DrawNAV()
{ for (short No = RADIO_FD_ANAV_WP; No < K155_DNAV_SZ; No++)  DrawField(RAD->navTAB + No);
  return;
}
//------------------------------------------------------------------
//  Draw CDI Needle
//  amplitude is +-20° for a VOR and +-5° for an ILS
//------------------------------------------------------------------
void CK155gauge::DrawCDI()
{ if (0 == RAD->navTAB[K155_FCDI_GR].state) return;
  if (0 == RAD->cdiST)                      return;
  int sbly  = (ILS)?(5):(20);
  int ynd = ca[RADIO_CA16].y1 - yOrg;
  int xnd = -int((RAD->cdiDEV * Ampli) / sbly);
  //---clamp to gauge picture ----------------------------
  if (-Ampli > xnd)   xnd = -Ampli;
  if (+Ampli < xnd)   xnd = +Ampli;
  xnd   += (cdiND - xOrg);
  rFont->SetTransparent();
  rFont->DrawChar(surf,xnd,ynd,'|',amber);
  rFont->SetOpaque();
  return;
}
//------------------------------------------------------------------
//  Draw the gauge
//  Display is drawed with the radio light brightness
//  Button are drawed with the panel ambiant brightness
//------------------------------------------------------------------
void CK155gauge::Draw()
{ if (0 == RAD) GetK155();
  if (0 == RAD) return;
  char st = RAD->sPower;
  PowST   = st;
  EraseSurfaceRGBA(surf,0);
  //--- Fill display if operational -------
  if (st)
  { DrawCOM();
    DrawNAV();
    DrawCDI();
  }
  RenderSurface(quad,surf);
  //--- draw buttons ----------------------
  ca[RADIO_BT18].Draw (st);
  DrawBUT(PowST);
  RepeatClick();
  return;
}
//==========================================================================
//    Mouse handler 
//==========================================================================
//------------------------------------------------------------------
//  Mouse move over 
//------------------------------------------------------------------
ECursorResult CK155gauge::MouseMoved (int mx, int my)
{ ECursorResult rc = CURSOR_NOT_CHANGED;
  for (int i = 1; (i< K55EV_AREA_SZ); i++)
  { rc = ca[i].MouseMoved(mx,my);
    if (rc) return rc;
  }
  return CURSOR_NOT_CHANGED;
}
//------------------------------------------------------------------
//  Check for COM event
//------------------------------------------------------------------
int CK155gauge::AnyComEvent(int mx, int my)
{ short       sta = RAD->cState;
  RADIO_HIT *itm = RAD->Kr55HIT[sta];
  while (itm->No) 
  { U_CHAR No = itm->No;
		if (!ca[No].IsHit(mx,my)) {itm++;	continue;}
	  if (!ca[No].IsOK(cMsk))		{itm++;	continue;}
    RAD->Dispatcher(itm->Ev);
    return itm->No;
  }
  return 0;
}
//------------------------------------------------------------------
//  Check for NAV event
//------------------------------------------------------------------
int CK155gauge::AnyNavEvent(int mx,int my)
{ short       sta = RAD->nState;
  RADIO_HIT *itm = RAD->Kr55HIT[sta];
  while (itm->No) 
  { U_CHAR No = itm->No;
		if (!ca[No].IsHit(mx,my)) {itm++;	continue;}
		if (!ca[No].IsOK(cMsk))		{itm++;	continue;}
    RAD->Dispatcher(itm->Ev);
    return itm->No;
  }
  return 0;
}
//------------------------------------------------------------------
//  Internal call to click on a CA field
//  na = field number
//------------------------------------------------------------------
void CK155gauge::ClickField(int na, short dir)
{ short      sta = RAD->nState;
  RADIO_HIT *itm = RAD->Kr55HIT[sta];
  RAD->mDir      = dir;
  while (itm->No)
  { if (itm->No != na) {itm++; continue;}
    RAD->Dispatcher(itm->Ev);
    return;
  }
  return;
}
//------------------------------------------------------------------
//  Mouse Click
//------------------------------------------------------------------
EClickResult CK155gauge::MouseClick (int x, int y, int bt)
{ if (0 == RAD)           return MOUSE_TRACKING_OFF;
  K55_EVENT Event = K55EV_NULL;
  RAD->mDir = (MOUSE_BUTTON_LEFT == bt)?(-1):(+1);
  int No = 0;
  if (0 == No) No = AnyComEvent(x,y);
  if (0 == No) No = AnyNavEvent(x,y);
    //-----------Check for repeating click ---------------
  if (ca[No].IsRept()) ArmRepeat(0.25,x,y,bt); 
  return (No)?(MOUSE_TRACKING_ON):(MOUSE_TRACKING_OFF);}
//-----------------------------------------------------------------------------
//  Stop click
//-----------------------------------------------------------------------------
EClickResult CK155gauge::StopClick ()
{ Disarm();
  return MOUSE_TRACKING_OFF;
}

//=====================================================================================
//
//  KR87 DRIVER TABLE
//
//=====================================================================================
//==========================================================================
//  Driver table for ADF GAUGE K87
//==========================================================================
RADIO_HIT KR87p0[] = {
  {RADIO_BT17,K87EV_POWR},
  {0,0},
};
//--------------------------------------------------------------------
RADIO_HIT KR87fq[] = {
  {RADIO_CA01,K87EV_ACT1},                       // Active Digit 1
  {RADIO_CA02,K87EV_ACT2},                       // Active Digit 2
  {RADIO_CA03,K87EV_ACT3},                       // Active Digit 3
  {RADIO_CA04,K87EV_ACT4},                       // Active Digit 4
  {RADIO_CA05,K87EV_SBY1},                       // Standby Digit 1
  {RADIO_CA06,K87EV_SBY2},                       // Standby Digit 2
  {RADIO_CA07,K87EV_SBY3},                       // Standby Digit 3
  {RADIO_CA08,K87EV_SBY4},                       // Standby Digit 4

  {RADIO_BT17,K87EV_POWR},                       // Power button
  {RADIO_BT18,K87EV_INNR},                       // Tune button
  {RADIO_BT19,K87EV_OUTR},                       // Tune button
  {RADIO_BT20,K87EV_BADF},                       // ADF  button
  {RADIO_BT21,K87EV_BBFO},                       // BFO  button
  {RADIO_BT22,K87EV_BFRQ},                       // FRQ  button
  {RADIO_BT23,K87EV_BFLT},                       // FLT  button
  {RADIO_BT24,K87EV_BRST},                       // ELT  button
  {0,0},
};
//--------------------------------------------------------------------
RADIO_HIT KR87tm[] = {
  {RADIO_CA01,K87EV_ACT1},                       // Active Digit 1
  {RADIO_CA02,K87EV_ACT2},                       // Active Digit 2
  {RADIO_CA03,K87EV_ACT3},                       // Active Digit 3
  {RADIO_CA04,K87EV_ACT4},                       // Active Digit 4
  {RADIO_CA09,K87EV_TMIN},                       // Timer minute
  {RADIO_CA10,K87EV_TSEC},                       // Timer seconde

  {RADIO_BT17,K87EV_POWR},                       // Power button
  {RADIO_BT18,K87EV_TSEC},                       // Tune button
  {RADIO_BT19,K87EV_TMIN},                       // Tune button
  {RADIO_BT20,K87EV_BADF},                       // ADF  button
  {RADIO_BT21,K87EV_BBFO},                       // BFO  button
  {RADIO_BT22,K87EV_BFRQ},                       // FRQ  button
  {RADIO_BT23,K87EV_BFLT},                       // FLT  button
  {RADIO_BT24,K87EV_BRST},                       // ELT  button
  {0,0},
};
//=============================================================================
RADIO_HIT *CKR87gauge::Kr87HIT[] = {
  KR87p0,                             // Power off
  KR87fq,                             // Frequency mode
  KR87tm,                             // Timer mode
};
//============================================================================
//  Fields to draw
//============================================================================
RADIO_DSP  CKR87gauge::Kr87FLD[] = {
  {KR87_ACTF_D1,RADIO_CA01},                          // Active frequency Digit 1
  {KR87_ACTF_D2,RADIO_CA02},                          // Active frequency Digit 2
  {KR87_ACTF_D3,RADIO_CA03},                          // Active frequency Digit 3
  {KR87_ACTF_D4,RADIO_CA04},                          // Active frequency Digit 4
  {KR87_SBYF_D1,RADIO_CA05},                          // Standby frequency Digit 1
  {KR87_SBYF_D2,RADIO_CA06},                          // Standby frequency Digit 2
  {KR87_SBYF_D3,RADIO_CA07},                          // Standby frequency Digit 3
  {KR87_SBYF_D4,RADIO_CA08},                          // Standby frequency Digit 4
  {KR87_ANTL_FD,RADIO_CA13},                          // ANtena led
  {KR87_ADFL_FD,RADIO_CA14},                          // ADF led
  {KR87_BFOL_FD,RADIO_CA15},                          // BFO LED
  {KR87_FRQL_FD,RADIO_CA16},                          // Freq LED
  {KR87_FLTL_FD,RADIO_CA11},                          // FLT LED
  {KR87_ETLD_FD,RADIO_CA12},                          // ET LED
  {KR87_TIMN_FD,RADIO_CA09},                          // Timer Minute
  {KR87_TISC_FD,RADIO_CA10},                          // Timer second
  {KR87_NULL_FD,},                         // Last filed
};

//============================================================================
//
//  K87 Gauge for Bendix King ADF
//
//============================================================================
CKR87gauge::CKR87gauge(CPanel *mp)
: CRadioGauge(mp)
{ Prop.Rep(NO_SURFACE + IS_DISPLAY);
  RAD   = 0;
  PowST = 0;
}
//------------------------------------------------------------------
//  Read all parameters
//------------------------------------------------------------------
int CKR87gauge::Read (SStream *stream, Tag tag)     // Read parameters
{ int ica = 0;
  int typ = ClickAreaFromTag (tag,&ica);
  if ((ica >= 1) && (ica < K87CA_SIZE)) {
    // Valid click area
    ca[ica].Init(this,typ);
    ReadFrom (&ca[ica], stream);
    ca[ica].BuildQuad();
    return TAG_READ;
  }
  switch (tag) {
		case 'shap':
			DecodeSHAP(stream,tour,0);
			BuildQuad(quad,tour);
			cMsk	= AREA_BUTN;					// Restrict click area
			return TAG_READ;

    case 'copy':
      CopyFrom(stream);
      return TAG_READ;
  }

  return CRadioGauge::Read (stream, tag);
}
//------------------------------------------------------------------
//  Copy from similar gauge
//------------------------------------------------------------------
void CKR87gauge::CopyFrom(SStream *stream)
{ CKR87gauge *g = (CKR87gauge*)LocateGauge(stream);
  CGauge::CopyFrom(*g);
  CRadioGauge::Copy(*g);
  for (int k=1; k<K87CA_SIZE; k++) ca[k].CopyFrom(g->ca[k],this);
  return;
}
//------------------------------------------------------------------
//  Collect VBO data
//------------------------------------------------------------------
void CKR87gauge::CollectVBO(TC_VTAB *vtb)
{	CGauge::CollectVBO(vtb);
	for (int k=1; k<K87CA_SIZE; k++)	ca[k].CollectVBO(vtb);
	return;
}
//------------------------------------------------------------------
//  All parameters are read
//  We add some new field descriptions that are not
//  defined in the file
//------------------------------------------------------------------
void CKR87gauge::ReadFinished()
{ CRadioGauge::ReadFinished ();
  //----Define display window ------------------------------
  yOfs  = 0;
  return;
}
//-----------------------------------------------------------------------------
//  Get the subsystem KR87
//-----------------------------------------------------------------------------
void CKR87gauge::GetKR87()
{ //----Get KR87status ----------------------------
  mesg.user.u.datatag = 'gets';
  Send_Message(&mesg);
  RAD = (CKR87radio*) mesg.voidData;
  return;
}
//----------------------------------------------------------------------------
//  Detect power transition
//  Draw buttons
//----------------------------------------------------------------------------
void CKR87gauge::DrawBUT(U_CHAR pwr)
{ //-----Display row of buttons ----------------------
	if (brow.Draw(pwr))	return ;
	//-----Display frame buttons -----------------------
  ca[RADIO_BT20].Draw (pwr);            // ADF button
  ca[RADIO_BT21].Draw (pwr);            // BFO button
  ca[RADIO_BT22].Draw (pwr);            // FRQ button
  ca[RADIO_BT23].Draw (pwr);            // FLT button
  ca[RADIO_BT24].Draw (pwr);            // SET button
  //---------------------------------------------------
  return; }
//------------------------------------------------------------------
//  Draw Field
//------------------------------------------------------------------
void CKR87gauge::DrawField(RADIO_DSP *dsp)
{ RADIO_FLD *fd = RAD->GetField(dsp->fn);
  U_CHAR mask = RAD->mskFS;
  if (0 == fd->state) return;
  if ((RAD_ATT_FLASH == fd->state) && (0 == mask))  return;
  short No  = dsp->sp;
  short x0  = ca[No].x1 - xOrg;
  short y0  = ca[No].y1 - yOrg;
  rFont->DrawNText(surf,x0,y0,amber,fd->data);
  return;
}
//------------------------------------------------------------------
//  Draw ADF
//------------------------------------------------------------------
void CKR87gauge::DrawADF()
{ RADIO_DSP  *dsp = Kr87FLD;
  while (dsp->fn) DrawField(dsp++);
  return;
}
//------------------------------------------------------------------
//  Draw the gauge
//------------------------------------------------------------------
void CKR87gauge::Draw()
{ if (0 == RAD) GetKR87();
  if (0 == RAD) return;
  char st = RAD->sPower;
  PowST   = st;
  EraseSurfaceRGBA(surf,0);
  if (st) DrawADF();
  RenderSurface(quad,surf);
  //---------------------------------------
  DrawBUT(st);
  RepeatClick();
  return;
}
//==================================================================
//  Mouse Driver
//==================================================================
//------------------------------------------------------------------
//  Mouse move over 
//------------------------------------------------------------------
ECursorResult CKR87gauge::MouseMoved (int mx, int my)
{ ECursorResult rc = CURSOR_NOT_CHANGED;
  for (int i = 1; (i< K87CA_SIZE); i++)
  { rc = ca[i].MouseMoved(mx,my);
    if (rc) return rc;
  }
  return CURSOR_NOT_CHANGED;
}
//------------------------------------------------------------------
//  Check for any event according to radio state
//------------------------------------------------------------------
int CKR87gauge::AnyEvent(int mx,int my)
{ char       sta  = RAD->GetRState();
  RADIO_HIT *itm  = Kr87HIT[sta];
  while (itm->No) 
  { if (!ca[itm->No].IsHit(mx,my)) {itm++; continue; }
    K87_EVENT evn = (K87_EVENT)itm->Ev;
    RAD->Dispatcher(evn);
    return itm->No;
  }
  return 0;
}
//------------------------------------------------------------------
//  Internal call to click on a CA field
//  na = field number
//------------------------------------------------------------------
void CKR87gauge::ClickField(int na, short dir)
{ char       sta = RAD->GetRState();
  RADIO_HIT *itm  = Kr87HIT[sta];
  RAD->mDir       = dir;
  while (itm->No)
  { if (itm->No != na) {itm++; continue;}
    RAD->Dispatcher(itm->Ev);
    return;
  }
  return;
}
//------------------------------------------------------------------
//  Mouse Click
//------------------------------------------------------------------
EClickResult CKR87gauge::MouseClick (int x, int y, int bt)
{ if (0 == RAD)        return MOUSE_TRACKING_OFF;
  RAD->mDir = (MOUSE_BUTTON_LEFT == bt)?(-1):(+1);
  int No = AnyEvent(x,y);
  //-----------Check for repeating click ---------------
  if (ca[No].IsRept()) ArmRepeat(0.25,x,y,bt); 
  return (No)?(MOUSE_TRACKING_ON):(MOUSE_TRACKING_OFF);}
//-----------------------------------------------------------------------------
//  Stop click
//-----------------------------------------------------------------------------
EClickResult CKR87gauge::StopClick ()
{ Disarm();
  return MOUSE_TRACKING_OFF;
}

//============================================================================
//  TRANSPONDER Fields to draw
//============================================================================
RADIO_DSP  CKT76gauge::KT76Fldtab[] = {
  {KT76_ALTS,RADIO_CA17},                          // Altitude sign
  {KT76_FLEV,RADIO_CA18},                          // Flight level
  {KT76_MODE,RADIO_CA19},                          // Mode
  {KT76_IDN1,RADIO_CA01},                          // ident Digit 1
  {KT76_IDN2,RADIO_CA02},                          // ident Digit 2
  {KT76_IDN3,RADIO_CA03},                          // ident Digit 3
  {KT76_IDN4,RADIO_CA04},                          // ident Digit 4
  {KT76_LAB1,RADIO_CA20},                          // FL label
  {KT76_NULF,},                         // Last field
};
//==========================================================================
//  KT76 Field definition
//==========================================================================
RADIO_HIT KT76Flds[] = {
  //----Transponder digits ----------
  {RADIO_CA01,KT76EV_DIG},
  {RADIO_CA02,KT76EV_DIG},
  {RADIO_CA03,KT76EV_DIG},
  {RADIO_CA04,KT76EV_DIG},
  //----SQWAK IDENT -----------------
  {RADIO_BT05,KT76EV_IDN},
  //----BUTTON 0 to 7 ---------------
  {RADIO_BT06,KT76EV_BTN},        // BT0
  {RADIO_BT07,KT76EV_BTN},        // BT1
  {RADIO_BT08,KT76EV_BTN},        // BT2
  {RADIO_BT09,KT76EV_BTN},        // BT3
  {RADIO_BT10,KT76EV_BTN},        // BT4
  {RADIO_BT11,KT76EV_BTN},        // BT5
  {RADIO_BT12,KT76EV_BTN},        // BT6
  {RADIO_BT13,KT76EV_BTN},        // BT7
  //----CLEAR ----------------------
  {RADIO_BT14,KT76EV_CLR},        // CLR
  //----VFR ------------------------
  {RADIO_BT15,KT76EV_VFR},        // VFR
  //--- Power and mode -----------------
  {RADIO_BT16,KT76EV_PWR},        // BT7
  {0,0},
};
//==========================================================================
//  TRANSPONDER KC76
//  gauge part:
//==========================================================================
CKT76gauge::CKT76gauge(CPanel *mp)
: CRadioGauge(mp)
{ Prop.Rep(NO_SURFACE + IS_DISPLAY);
  RAD   = 0;
  PowST = 0;
  ppwr  = 0;
}
//------------------------------------------------------------------
//  Copy from similar gauge
//------------------------------------------------------------------
void CKT76gauge::CopyFrom(SStream *stream)
{ CKT76gauge *g = (CKT76gauge*)LocateGauge(stream);
  CGauge::CopyFrom(*g);
  CRadioGauge::Copy(*g);
  for (int k=1; k<KT76CA_MAX; k++) ca[k].CopyFrom(g->ca[k],this);
  return;
}
//------------------------------------------------------------------
//  Collect VBO data
//------------------------------------------------------------------
void CKT76gauge::CollectVBO(TC_VTAB *vtb)
{	CGauge::CollectVBO(vtb);
	for (int k=1; k<KT76CA_MAX; k++)	ca[k].CollectVBO(vtb);
	return;	}
//------------------------------------------------------------------
//  Read all parameters
//------------------------------------------------------------------
int CKT76gauge::Read (SStream *stream, Tag tag)     // Read parameters
{ int ica = 0;
  int typ = ClickAreaFromTag (tag,&ica);
  if ((ica >= 1) && (ica < KT76CA_MAX)) {
    // Valid click area
    ca[ica].Init(this,typ);
    ReadFrom (&ca[ica], stream);
   
    ca[ica].BuildQuad();
    return TAG_READ;
  }
  switch (tag) {
    case 'copy':
      CopyFrom(stream);
      return TAG_READ;
    }


  return CRadioGauge::Read (stream, tag);
}
//------------------------------------------------------------------
//  All parameters are read
//  Adjust all dispaly area with the font size
//------------------------------------------------------------------
void CKT76gauge::ReadFinished()
{ CRadioGauge::ReadFinished ();
  //----Define display window ------------------------------
  yOfs  = 0;
  return;
}
//-----------------------------------------------------------------------------
//  Get the subsystem KR87 Radio
//-----------------------------------------------------------------------------
void CKT76gauge::GetRadio()
{ //----Get KT76 status ----------------------------
  mesg.user.u.datatag = 'gets';
  Send_Message(&mesg);
  RAD = (CKT76radio*) mesg.voidData;
  return;
}
//------------------------------------------------------------------
//  Check for any event according to radio state
//------------------------------------------------------------------
int CKT76gauge::AnyEvent(int mx,int my)
{ RADIO_HIT *itm  = KT76Flds;
  int   ev = 0;
  while (itm->No) 
  { ev++;
    if (!ca[itm->No].IsHit(mx,my)) {itm++; continue; }
    RAD->Dispatcher(itm->Ev,ev);
    return 1;
  }
  return 0;
}
//------------------------------------------------------------------
//  Internal call to click on a CA field
//  na = field number
//------------------------------------------------------------------
void CKT76gauge::ClickField(int na, short dir)
{ RADIO_HIT *itm  = KT76Flds;
  int   ev  = 0;
  RAD->mDir = dir;
  while (itm->No)
  { ev++;
    if (itm->No != na) {itm++; continue;}
    RAD->Dispatcher(itm->Ev,ev);
    return;
  }
  return;
}
//------------------------------------------------------------------
//  Mouse Click
//------------------------------------------------------------------
EClickResult CKT76gauge::MouseClick (int x, int y, int bt)
{ KT76events Event = KT76EV_NUL;
  if (0 == RAD)         return MOUSE_TRACKING_OFF;
  RAD->mDir = (MOUSE_BUTTON_LEFT == bt)?(-1):(+1);
  int evn = AnyEvent(x,y);
  return (evn)?(MOUSE_TRACKING_ON):(MOUSE_TRACKING_OFF); }
//------------------------------------------------------------------
//  Mouse move over 
//------------------------------------------------------------------
ECursorResult CKT76gauge::MouseMoved (int mx, int my)
{ ECursorResult rc = CURSOR_NOT_CHANGED;
  for (int i = 1; (i< KT76CA_MAX); i++)
  { rc = ca[i].MouseMoved(mx,my);
    if (rc) return rc;
  }
  return CURSOR_NOT_CHANGED;
}
//------------------------------------------------------------------
//  Mouse Stop click 
//------------------------------------------------------------------

//------------------------------------------------------------------
//  Draw Field
//------------------------------------------------------------------
void CKT76gauge::DrawField(RADIO_DSP *dsp)
{ RADIO_FLD *fd = RAD->GetField(dsp->fn);
  U_CHAR msk = RAD->mskFS;
  if (0 == fd->state) return;
  if ((RAD_ATT_FLASH == fd->state) && (0 == msk))  return;
  short No  = dsp->sp;
  short x0  = ca[No].x1 - xOrg;
  short y0  = ca[No].y1 - yOrg;
  rFont->DrawNText(surf,x0,y0,amber,fd->data);
  return;
}
//------------------------------------------------------------------
//  Draw Text fields
//------------------------------------------------------------------
void CKT76gauge::DrawTextFields()
{ RADIO_DSP  *dsp = KT76Fldtab;
  while (dsp->fn) DrawField(dsp++);
  return;
}
//----------------------------------------------------------------------------
//  Detect power transition
//----------------------------------------------------------------------------
void CKT76gauge::DrawBUT(U_CHAR st)
{ int Frm = (st)?(1):(0);
  //-----Display frame buttons -----------------------
  ca[RADIO_CA16].Draw (st);             // Power
  ca[RADIO_BT05].Draw (Frm);            // Sqwak Ident button
  ca[RADIO_BT06].Draw (Frm);            // Button 0
  ca[RADIO_BT07].Draw (Frm);            // Button 1
  ca[RADIO_BT08].Draw (Frm);            // Button 2
  ca[RADIO_BT09].Draw (Frm);            // Button 3
  ca[RADIO_BT10].Draw (Frm);            // Button 4
  ca[RADIO_BT11].Draw (Frm);            // Button 5
  ca[RADIO_BT12].Draw (Frm);            // Button 6
  ca[RADIO_BT13].Draw (Frm);            // Button 7
  //-----------------------------------------------------
  ca[RADIO_BT14].Draw (Frm);            // CLR
  ca[RADIO_BT15].Draw (Frm);            // VFR
  //----Row of buttons -----------------------------------
  return ; }

//------------------------------------------------------------------
//  Draw the gauge
//------------------------------------------------------------------
void CKT76gauge::Draw()
{ if (0 == RAD) GetRadio();
  if (0 == RAD) return;
  //--Draw master button -----------------------------------
  char  st  = RAD->ButtonPos();
  PowST     = st;
  EraseSurfaceRGBA(surf,0);
  if (st) DrawTextFields();
  RenderSurface(quad,surf);
  //-------------------------------------------------------
  DrawBUT(st);
  return;
}
//===================================================================================
//  K89 gauge construtor
//===================================================================================
CK89gauge::CK89gauge(CPanel *mp)
: CRadioGauge(mp)
{ Prop.Rep(NO_SURFACE + IS_DISPLAY);
  //-------Screen to clear -----------------------------------
  dsw = dsh = 0;
  yLed  = 0;
	GPS		= 0;
  //-------Init Various data----------------------------------
  rFont		= (CFont*)globals->fonts.ftmono8.font;
  yelow   = MakeRGB(255,167,102);           // Yellow color
  PowST   = K89_PWROF;
  //------Set Gauge into the GPS subsystem -------------------
	char *erm = "CK89Gauge needs a k89g subsystem";
	GPSRadio *gps = panel->GetMVEH()->GetGPS();
	if (0 == gps)										gtfo(erm);
	if ('k89g' != gps->GetUnId())		gtfo(erm); 
	gps->SetGauge(this);
	GPS     = (CK89gps*)gps;
}
//---------------------------------------------------------------
//  Read all tags
//---------------------------------------------------------------
int CK89gauge::Read (SStream *stream, Tag tag)
{
  // Check for click area tags
  int ica = 0;
  int pm;
  int typ = ClickAreaFromTag (tag, &ica);
  ca[ica].Type = typ;
  if ((ica >= 1) && (ica < K89_DIM)) {
    // Valid click area
    ca[ica].Init(this,typ);
    ReadFrom (&ca[ica], stream);
    ca[ica].BuildQuad();
    return TAG_READ;
  }

  switch (tag) {
    //--- Char adjuster ---------------------
    case 'wcar':
      ReadInt(&pm, stream);
      wCar  = U_CHAR(pm);
      return TAG_READ;
    //--- Led base line ---------------------
    case 'yled':
      ReadInt(&pm, stream);
      yLed  = short(pm);
      return TAG_READ;
    //--- radio tag -------------------------
    case 'radi':
      ReadTag (&radi, stream);
      return TAG_READ;
    //--- Copy from ------------------------
    case 'copy':
      CopyFrom(stream);
      return TAG_READ;
 }
 return CRadioGauge::Read(stream,tag);
}
//----------------------------------------------------------------
//  All parameters are read
//----------------------------------------------------------------
void CK89gauge::ReadFinished()
{ CRadioGauge::ReadFinished();
  //----------------------------------------------------------------
  yOfs    = 0;
  //-------Surface base parameters ---------------------------
  yBase   = 0;
  xBase   = (6 * wCar) + 2;
  //-------Inside click area (must be gauge relative ---------
  xClk    = xBase + xOrg;
  yClk    = yOrg;
  //-------Limit  click area in right part of surface --------
  ca[21].Set(xClk,yOrg,(12 * wCar),(4 * hCar),AREA_CLIK);
  //-------Screen Line table ---------------------------------
  scrLN[0] = yBase;
  scrLN[1] = scrLN[0] + hCar;
  scrLN[2] = scrLN[1] + hCar;
  scrLN[3] = scrLN[2] + hCar;
  scrLN[4] = scrLN[3] + hCar;
  if (0 == yLed) yLed = scrLN[4] - 1;
  //---------Underline mode parameters ---------------------
  undTAB[0]   = 0;
  undTAB[1]   = ca[10].x1;  //  PAGE_NUL
  undTAB[2]   = ca[11].x1;  //  PAGE_APT
  undTAB[3]   = ca[12].x1;  //  PAGE_VOR
  undTAB[4]   = ca[13].x1;  //  PAGE_NDB
  undTAB[5]   = ca[14].x1;  //  PAGE_INT
  undTAB[6]   = ca[15].x1;  //  PAGE_USR
  undTAB[7]   = ca[16].x1;  //  PAGE_ACT
  undTAB[8]   = ca[17].x1;  //  PAGE_NAV
  undTAB[9]   = ca[18].x1;  //  PAGE_FPL  
  undTAB[10]  = ca[19].x1;  //  PAGE_CAL  
  undTAB[11]  = ca[20].x1;  //  PAGE_SET
  //----Init letter click fields ---------------------------------
  IniLF(1,GPS->Ident);      // IDENT Input field      APT-VOR-NDB page 01-
  IniLF(2,GPS->Name);       // NAME Input field       APT-VOR-NDB page 01-
  IniLF(3,GPS->Name+3);     // NAME Input field       WPT page 01-----------
  IniLF(4,GPS->Ident);      // IDENT Direct To field  APT-VOR-NDB------
  IniLF(5,GPS->Name+4);     // NAME  Direct To field  WPT---------------
  IniLF(6,GPS->IdFR);       // FROM in CAL page 1
  IniLF(7,GPS->IdTO);       // TO   in CAL page 1
  IniLF(8,GPS->IdFR);       // FROM in CAL page 2
  IniLF(9,GPS->IdTO);       // TO   in CAL page 2
  //---------Repeatable field ------------------------------------------
  IniRP(1,&GPS->AltiTG);    // Altitude field       NVAV-
  IniRP(2,0);               // Waypoint ident       VNAV-
  IniRP(3,&GPS->OffsVL);    // Offset field         VNAV-
  IniRP(4,&GPS->GrndSP);    // Ground speed         VNAV-
  IniRP(5,&GPS->VspdVL);    // VSI speed            VNAV
  IniRP(6,&GPS->CalSPD);    // Ground speed         CAL page 1
  IniRP(7,&GPS->CalSPD);    // Ground Speed         CAL Page 2
  IniRP(8,&GPS->CalFFL);    // Fuel flow            CAL page 2
  IniRP(9,&GPS->CalFRS);    // Fuel reserve         CAP page 2
  //---------Init simple page fields -----------------------------------
  IniPF( 1);      // APT-VOR-NDB-WPT nearest number
  IniPF( 2);      // APT-VOR-NDB-WPT to/from
  IniPF( 3);      // NAV Page 01
  IniPF( 4);      // NAV PAge 02
  IniPF( 5);      // ALT Page 01 BARO Part 1
  IniPF( 6);      // ALT Page 01 BARO Part 2
  IniPF( 7);      // NAV in VNAV page 01
  IniPF( 8);      // WPT option CAL page 1
  IniPF( 9);      // WPT option CAL page 2
  //-----------Init nav message ------------------------------
  nMsg.id       = MSG_GETDATA;
  nMsg.sender   = unId;
  nMsg.user.u.datatag = 'obs_';
	//--- Compute CDI width ------------------------------------
	short pix = 11 * wCar;
	Ampli			= pix >> 1;						// Half width
	short cdi = GPS->cdiCL;
	Base			= xBase + Ampli - (wCar >> 1) + 1;
  return;
}
//------------------------------------------------------------------
//  Copy from similar gauge
//------------------------------------------------------------------
void CK89gauge::CopyFrom(SStream *stream)
{ CK89gauge *g = (CK89gauge*)LocateGauge(stream);
  CGauge::CopyFrom(*g);
  CRadioGauge::Copy(*g);
	//--- copy bitmap only if no button row are defined -------------
  for (int k=1; k<K89_DIM; k++) ca[k].CopyFrom(g->ca[k],this);
  return;
}
//------------------------------------------------------------------
//  Collect VBO data
//------------------------------------------------------------------
void CK89gauge::CollectVBO(TC_VTAB *vtb)
{	CGauge::CollectVBO(vtb);
	for (int k=1; k<K89_DIM; k++) ca[k].CollectVBO(vtb);
	return;	}
//------------------------------------------------------------------------------
//  Return the requested field
//------------------------------------------------------------------------------
K89_LETFD *CK89gauge::GetLetField(short No)
{ return letTAB + No;
}
//------------------------------------------------------------------------------
//  Init Input Field
//------------------------------------------------------------------------------
void CK89gauge::IniLF(char No,char *fd)
{ short lc            = GPS->LetPmTAB[No].Left;
  short ln            = GPS->LetPmTAB[No].Line;
  short sz            = GPS->LetPmTAB[No].Right;
  letTAB[No].aChar    = fd;
  letTAB[No].Line     = ln;
  letTAB[No].cPos     = 0;
  letTAB[No].Iden     = GPS->LetPmTAB[No].Iden;
  letTAB[No].aFld     = GPS->LetPmTAB[No].aFld;
  letTAB[No].aDisp    = GPS->Display + (ln * K89_NBR_COLN) + lc;
  letTAB[No].Left     = (lc        - K89_CLN06) * wCar;
  letTAB[No].Right    = ((lc + sz) - K89_CLN06) * wCar;
  letTAB[No].Next     = GPS->LetPmTAB[No].Next;
  letTAB[No].Type     = GPS->LetPmTAB[No].Type;
  letTAB[No].xFld     = GPS->LetPmTAB[No].xFld;
  return;
}
//------------------------------------------------------------------------------
//  Init reapetable Input Field
//------------------------------------------------------------------------------
void CK89gauge::IniRP(char No,int *nm)
{ short lc            = GPS->RepPmTAB[No].Left;
  short ln            = GPS->RepPmTAB[No].Line;
  short sz            = GPS->RepPmTAB[No].Right;
  repTAB[No].aNber    = nm;
  repTAB[No].Left     = (lc         - K89_CLN06) * wCar;
  repTAB[No].Right    = ((lc + sz)  - K89_CLN06) * wCar;
  repTAB[No].Line     = ln;
  repTAB[No].vIncr    = GPS->RepPmTAB[No].vIncr;
  repTAB[No].nbMin    = GPS->RepPmTAB[No].nbMin;
  repTAB[No].nbMax    = GPS->RepPmTAB[No].nbMax;
  repTAB[No].Event    = GPS->RepPmTAB[No].Event;
  repTAB[No].Next     = GPS->RepPmTAB[No].Next;
  repTAB[No].Type     = GPS->RepPmTAB[No].Type;
  return;
}
//------------------------------------------------------------------------------
//  Init simple page Field
//------------------------------------------------------------------------------
void CK89gauge::IniPF(char No)
{ short lc          = GPS->PagPmTAB[No].Left;
  short sz          = GPS->PagPmTAB[No].Right;
  pagTAB[No].Event  = GPS->PagPmTAB[No].Event;
  pagTAB[No].Left   = (lc        - K89_CLN06) * wCar;
  pagTAB[No].Right  = ((lc + sz) - K89_CLN06) * wCar;
  pagTAB[No].Line   = GPS->PagPmTAB[No].Line;
  pagTAB[No].Next   = GPS->PagPmTAB[No].Next;
  pagTAB[No].Type   = GPS->PagPmTAB[No].Type;
  return;
}

//-----------------------------------------------------------------------------
//  Open the GPS window
//  NOTE:  This routine is graphic dependent
//-----------------------------------------------------------------------------
void CK89gauge::DrawGPSwindow()
{ EraseSurfaceRGBA(surf,0);
  short y0 = yBase - 1;     ///ca[21].y1 + 2;
  int xl = (dsw - GPS->Tim01) >> 1;
  if (1 >= xl)  return;
  FillRect(surf,0,y0,xl,y0 + dsh,amber);
  FillRect(surf,(dsw - xl),y0,(dsw - 1) ,y0 + dsh,amber);
  return;
}
//-----------------------------------------------------------------------------
//  Draw full display to surface
//-----------------------------------------------------------------------------
void CK89gauge::DrawAllDisplay()
{ short x0      = 0;
  short y0      = 0;
  U_SHORT *org  = GPS->Display;
  short    nl   = 0;
  short    nc   = 0;
  U_SHORT  car  = 0;
  U_INT    col  = amber;
  rFont->SetOpaque();
  while (K89_NBR_LINE != nl)
  { nc  = K89_NBR_COLN;
    x0  = 0;
    y0  = scrLN[nl++];
    while (nc--)
    { col = amber;
      car = *org++;
      if (K89_ATT_FLREQ & car)  car &= mskFS; 
      if (K89_ATT_YELOW & car)  col  = yelow;
      x0 += rFont->DrawChar(surf,x0,y0,(char)(car),col);
    }
  }
  return;
}
//-----------------------------------------------------------------------------
//  Draw left display to surface
//-----------------------------------------------------------------------------
void CK89gauge::DrawLSPDisplay()
{ short x0      = 0;
  short y0      = 0;
  U_SHORT *org  = GPS->Display;
  U_INT    fac  = (K89_NBR_COLN - K89_CLN06);
  short    nl   = 0;
  short    nc   = 0;
  U_SHORT  car  = 0;
  U_INT    col  = amber;
  rFont->SetOpaque();
  while (K89_NBR_LINE != nl)
  { nc  = K89_CLN06;
    x0  = 0;
    y0  = scrLN[nl++];
    while (nc--)
    { col = amber;
      car = *org++;
      if (K89_ATT_FLREQ & car)  car &= mskFS; 
      if (K89_ATT_YELOW & car)  col = yelow;
      x0 += rFont->DrawChar(surf,x0,y0,(char)(car),col);
    }
    org += fac;
  }
  return;
}
//-----------------------------------------------------------------------------
//  Draw rigth display to surface
//-----------------------------------------------------------------------------
void CK89gauge::DrawRSPDisplay()
{ short x0      = 0;
  short y0      = 0;
  U_SHORT *org  = GPS->Display + K89_CLN06;
  U_INT    fac  = K89_CLN06;
  short    nl   = 0;
  short    nc   = 0;
  U_SHORT  car  = 0;
  U_INT    col  = amber;
  rFont->SetOpaque();
  while (K89_NBR_LINE != nl)
  { nc  = K89_PR2_COLN;
    x0  = xBase;
    y0  = scrLN[nl++];
    while (nc--)
    { col = amber;
      car = *org++;
      if (K89_ATT_FLREQ & car)  car &= mskFS; 
      if (K89_ATT_YELOW & car)  col = yelow;
      x0 += rFont->DrawChar(surf,x0,y0,(char)(car),col);
    }
    org += fac;
  }
  return;
}
//-----------------------------------------------------------------------------
//  Draw the GPS in 2 parts with divider in the middle and mode underline
//-----------------------------------------------------------------------------
void CK89gauge::DrawLDRDisplay()
{ int x0 = undTAB[GPS->cPage] - xOrg; 
  DrawFastLine(surf,(xBase-2),(yBase+1),(xBase-2),(yBase + dsh - 2),amber);
  DrawLSPDisplay();
  DrawRSPDisplay();
  if (x0) DrawFastLine(surf,x0,yLed,(x0 + 7),yLed,amber);
  DrawCDIneedle();
  return;
}
//----------------------------------------------------------------------------
//  Draw the button
//----------------------------------------------------------------------------
void CK89gauge::DrawButton(int fr)
{ ca[K89_BUTCSR].Draw(fr);
  if (brow.Draw(fr)) return;
	//--- Draw individual buttons ----
	ca[K89_BUTMSG].Draw(fr);
  ca[K89_BUTOBS].Draw(fr);
  ca[K89_BUTALT].Draw(fr);
  ca[K89_BUTNRS].Draw(fr);
  ca[K89_BUTDIR].Draw(fr);
  ca[K89_BUTCLR].Draw(fr);
  ca[K89_BUTENT].Draw(fr);
  return;
}
//============================================================================
//  Display Vector
//============================================================================
CK89gauge::DisFN CK89gauge::DshTAB[4] = {
  &CK89gauge::DrawAllDisplay,             // 0 Draw full display
  &CK89gauge::DrawLDRDisplay,             // 1 Draw part display
  &CK89gauge::DrawGPSwindow,              // 2 Opening window
};

//----------------------------------------------------------------------------
//  Draw the CDI (The full amplitude is [-20°,+20°]
//----------------------------------------------------------------------------
void CK89gauge::DrawCDIneedle()
{ short cdi = GPS->cdiCL;
  //-------Draw CDI deviation --(positive=>Left deviation)------------------
  if (0 == cdi) return;
  int y0  = scrLN[GPS->cdiLN];
	int mid = Base + (cdi * wCar);
	int bar = -int((Ampli * GPS->cdiDEV) / 40);
  //-------Clamp result to [0-66] -----------------------------
  if (-33 > bar) bar = -Ampli;						
  if (+33 < bar) bar = +Ampli;						
  rFont->SetTransparent();
  rFont->DrawChar(surf,(mid + bar),y0,'|',amber);
  return;
}
//----------------------------------------------------------------------------
//  Draw GPS
//----------------------------------------------------------------------------
void CK89gauge::Draw()
{ //-------Detect Power change-------------------------------
  mskFS      = GPS->mskFS;
  U_CHAR  st = GPS->aState;
  PowST = st;
  EraseSurfaceRGBA(surf,0);
  //-----Refresh display ------------------------------------
  if (st)(this->*DshTAB[GPS->DshNO])();
  RenderSurface(quad,surf);
  //--- Draw button and display -----------------------------
  ca[K89_BUTPWR].Draw(st);
  DrawButton(st);
  return;
}
//=============================================================================
//  K89 gauge input handlers
//=============================================================================
//  Vector functions
//----------------------------------------------------------------------------
CK89gauge::ClkFN CK89gauge::HndTAB[4] = 
{ &CK89gauge::AnyNULclick,                // 0 NUL handler
  &CK89gauge::AnyLETclick,                // 1 Letter handler
  &CK89gauge::AnyGENclick,                // 2 Generic handler
  0,                                      // 3 RFU
};
//------------------------------------------------------------------------------
//  Mouse move over the GPS
//------------------------------------------------------------------------------
ECursorResult CK89gauge::MouseMoved (int x, int y)
{
  ECursorResult rc = CURSOR_NOT_CHANGED;

  // Send updated mouse position to all click areas
  for (int i=K89_MSG; i< K89_DIM  && (rc == CURSOR_NOT_CHANGED); i++)  rc = ca[i].MouseMoved (x, y);

  // Call parent method if cursor not changed
  return (rc == CURSOR_NOT_CHANGED)?(CRadioGauge::MouseMoved (x, y)):(rc);
}
//------------------------------------------------------------------------------
//  Mouse Stop Clicking. Disarm repetition
//------------------------------------------------------------------------------
EClickResult CK89gauge::StopClick ()
{ GPS->RepCK = 0;
  return MOUSE_TRACKING_OFF;
}
//------------------------------------------------------------------
//  Internal call to click on a CA field
//  na = field number
//------------------------------------------------------------------
void CK89gauge::ClickField(int na, short dir)
{ int Event = K89_NUL;
  GPS->msDIR = dir;
  if (na >= K89_DIM)  return;
  Event = (K89_EVENT)na;
  GPS->Dispatch((K89_EVENT) Event);
  return;
}
//------------------------------------------------------------------------------
//  Mouse click over the GPS
//  Generate Event and process according to GPS state
//------------------------------------------------------------------------------
EClickResult CK89gauge::MouseClick (int x, int y, int buttons)
{ EClickResult rc = MOUSE_TRACKING_OFF;
  int Event = K89_NUL;
  GPS->msDIR = (MOUSE_BUTTON_LEFT == buttons)?(-1):(+1);
  for (int i=K89_MSG; (i< K89_DIM) && (Event==0); i++) {
    if (ca[i].IsHit(x, y)) {
      Event = i;
      rc = MOUSE_TRACKING_ON;
      break;
    }
  }
  if (K89_NUL == Event)   return rc;
  //-----------Check for additional click according to click vector ---
  //  Compute origine in right part of display surface 
  //-------------------------------------------------------------------
  if (K89_DPY == Event) 
    Event = (this->*HndTAB[GPS->HndNO])((x - xClk),y - yClk);
  //-----------Call the state function --------------------------------
  GPS->Dispatch((K89_EVENT) Event);
  return rc;
}
//-----------------------------------------------------------------------------
//  Click Handler N°1
//  Check for any click on letter of the field
// YES => Set LetFD and post event
// NO  => Prepare next field to test
//-----------------------------------------------------------------------------
int CK89gauge::AnyCHRclick(K89_LETFD *fd,int mx,int my)
{ char ln = fd->Line;
  char y0 = scrLN[ln];
  char y1 = scrLN[ln+1];
  WtyFD   = fd->Type;
  WnoFD   = fd->Next;
  if ((my < y0) || (my >= y1))  return K89_NUL;
  U_CHAR pos = 0;
  int  x0 = fd->Left;
  int  x1 = x0 + wCar;
  do  { if ((mx >= x0) && (mx < x1))
        { fd->cPos = pos;
          GPS->LetFD = fd;
          return K89_DPY;
        }
        pos++;
        x0 = x1;
        x1 += wCar;
  } while (x0 != fd->Right);
  return K89_NUL;
}
//-----------------------------------------------------------------------------
//  Click Handler N°2
//  Check for click on simple field
// YES => Post event
// NO  => Prepare next field to test
//-----------------------------------------------------------------------------
int CK89gauge::AnySPFclick(K89_PAGFD *fd,int mx,int my)
{ char ln = fd->Line;
  char y0 = scrLN[ln];
  char y1 = scrLN[ln+1];
  WtyFD   = fd->Type;
  WnoFD   = fd->Next;
  if ((my >= y0) && (my <y1) && (mx >= fd->Left) && (mx < fd->Right)) return fd->Event;
  return K89_NUL;
}
//-----------------------------------------------------------------------------
//  Click Handler N°3
//  Check for any click on repeat field
// YES => Set RepCK and post event
// NO  => Prepare next field to test
//-----------------------------------------------------------------------------
int CK89gauge::AnyFLDclick(K89_REPFD *fd,int mx,int my)
{ char ln = fd->Line;
  char y0 = scrLN[ln];
  char y1 = scrLN[ln+1];
  WtyFD   = fd->Type;
  WnoFD   = fd->Next;
  if ((my >= y0) && (my <y1) && (mx >= fd->Left) && (mx < fd->Right))
  { fd->vIncr = (abs(fd->vIncr) * GPS->msDIR);
    if (fd->aNber) GPS->RepCK = fd;
    return fd->Event;
  }
  return K89_NUL;
}
//-----------------------------------------------------------------------------
//  Check for any click on current input or alternate
//-----------------------------------------------------------------------------
int CK89gauge::AnyLETclick(int mx, int my)
{ U_CHAR     xf = GPS->LetFD->xFld;               // Field number
  K89_LETFD *fd = letTAB + xf;
  U_CHAR     No = fd->Iden;
  do  { if (AnyCHRclick(fd,mx,my))  return K89_DPY;
        if (0 == fd->aFld)          return K89_NUL;
        fd = letTAB + fd->aFld;
      }
  while (No != fd->Iden);
  return K89_NUL;
}
//-----------------------------------------------------------------------------
//  Check for any click on any field
//-----------------------------------------------------------------------------
int CK89gauge::AnyGENclick(int mx,int my)
{ WtyFD       = GPS->StyFD;              // Start field type
  WnoFD       = GPS->SnoFD;              // Start field number
  int evn     = K89_NUL;
  GPS->RepCK  = 0;
  while (WnoFD)
  { switch (WtyFD)
     {  case K89_LET_FD:
        { K89_LETFD *fd = letTAB + WnoFD;
          evn = AnyCHRclick(fd,mx,my);
          if (evn)  return evn;
          continue;
        }
        case K89_REP_FD:
        { K89_REPFD *fd = repTAB + WnoFD;
          evn = AnyFLDclick(fd,mx,my);
          if (evn)  return evn;
          continue;
        }
        case K89_PAG_FD:
        { K89_PAGFD *fd = pagTAB + WnoFD;
          evn = AnySPFclick(fd,mx,my);
          if (evn)  return evn;
          continue;
        }
    }
  }
  return K89_NUL;
}
//-----------------------------------------------------------------------------
//  Void Clicker
//-----------------------------------------------------------------------------
int CK89gauge::AnyNULclick(int mx,int my)
{ return K89_NUL;
}
//========================================================================================
//  Define EVENT on KC140 GAUGE (AND CLICK_AREA)
//========================================================================================
#define K140_NULL 0     // No event
//========================================================================================
// CBKAP140Gauge
//========================================================================================
CBKAP140Gauge::CBKAP140Gauge (CPanel *mp)
: CRadioGauge(mp)

{ Prop.Rep(NO_SURFACE + IS_DISPLAY);
  tca   = K140_NCA;
  Power = 0;
  //----------------------------------------------------------------
  PAN   = 0;
  rFont = (CVariFontBMP*)globals->fonts.ftradi9.font;
}
//------------------------------------------------------------------
//  Frre object
//------------------------------------------------------------------
CBKAP140Gauge::~CBKAP140Gauge()
{	int a = 0;
}
//------------------------------------------------------------------
//  Copy from similar gauge
//------------------------------------------------------------------
void CBKAP140Gauge::CopyFrom(SStream *stream)
{ CBKAP140Gauge *g = (CBKAP140Gauge*)LocateGauge(stream);
  CGauge::CopyFrom(*g);
  CRadioGauge::Copy(*g);
  for (int k=1; k<=RADIO_CA21; k++) ca[k].CopyFrom(g->ca[k],this);
  return;
}
//------------------------------------------------------------------
//  Collect VBO data
//------------------------------------------------------------------
void CBKAP140Gauge::CollectVBO(TC_VTAB *vtb)
{	CGauge::CollectVBO(vtb);
	for (int k=1; k<K140_NCA; k++)	ca[k].CollectVBO(vtb);
	return;	}
//-----------------------------------------------------------------
//  Read all parameters
//-----------------------------------------------------------------
int CBKAP140Gauge::Read (SStream *st, Tag tag)
{  // Check for click area tags
  int ica = 0;
  int typ = ClickAreaFromTag (tag,&ica);
  ca[ica].Type = typ;
  if ((ica >= 1) && (ica < tca)) {
    // Valid click area
    ca[ica].Init(this,typ);
    ReadFrom (&ca[ica], st);
    ca[ica].BuildQuad();
    return TAG_READ;
  }
  if ('copy' == tag)
  { CopyFrom(st);
    return TAG_READ;
  }
  return CRadioGauge::Read (st, tag);
}
//------------------------------------------------------------------
//  All parameters are read
//  NOTE: ALL extra CA are resolution dependent and should be defined later in the
//  panel descriptor file
//------------------------------------------------------------------
void CBKAP140Gauge::ReadFinished()
{ CRadioGauge::ReadFinished ();
  //----Init display dimension ------------------------
  yOfs  = 0;
  return;
}
//-----------------------------------------------------------------------------
//  Get the panel CKAP140Panel
//-----------------------------------------------------------------------------
void CBKAP140Gauge::Get140Panel()
{ //----Get CKAP140Panel ----------------------------
  msgp.id     = MSG_GETDATA;
  msgp.sender = unId;
  msgp.group  = 'K140';
  msgp.user.u.datatag = 'gets';
  Send_Message(&msgp);
  PAN = (CKAP140Panel*) msgp.voidData;
  return;
}
//------------------------------------------------------------------------------
//  Mouse click over the panel
//  Generate Event and process according to panel state
//------------------------------------------------------------------------------
EClickResult CBKAP140Gauge::MouseClick (int x, int y, int bt)
{ EClickResult rc = MOUSE_TRACKING_OFF;
  int Event = K140_NULL;
  if (0 == PAN)             return rc;
  for (int i=1; (i< tca) && (Event==0); i++) {
    if (ca[i].IsHit(x, y)) {
      Event = i;
      rc = MOUSE_TRACKING_ON;
      break;
    }
  }
  //-----------Call the state function -----------------
  if (K140_NULL == Event)   return rc;
  PAN->msDIR = (MOUSE_BUTTON_LEFT == bt)?(-1):(+1);
  PAN->Dispatch(Event);
  //-----------Check for repeating click ---------------
  if (ca[Event].IsRept()) rc = ArmRepeat(0.25,x,y,bt); 
  return rc;
}
//-----------------------------------------------------------------------------
//  Stop click
//-----------------------------------------------------------------------------
EClickResult CBKAP140Gauge::StopClick ()
{ Disarm();
  return MOUSE_TRACKING_OFF;
}
//-----------------------------------------------------------------------------
//  Mouse move over the gauge
//-----------------------------------------------------------------------------
ECursorResult CBKAP140Gauge::MouseMoved (int x, int y)
{ ECursorResult rc = CURSOR_NOT_CHANGED;
  // Send updated mouse position to all click areas
  for (int i=1; i<=RADIO_BT20 && (rc == CURSOR_NOT_CHANGED); i++) {
    rc = ca[i].MouseMoved (x, y);
  }

  // Call parent method if cursor not changed
  if (rc == CURSOR_NOT_CHANGED)  rc = CRadioGauge::MouseMoved (x, y);
  return rc;
}
//------------------------------------------------------------------
//  Draw Field
//------------------------------------------------------------------
void CBKAP140Gauge::DrawField(RADIO_DSP *dsp)
{ RADIO_FLD *fd = PAN->GetField(dsp->fn);
  U_CHAR mask = PAN->mskFS;
  if (0 == fd->state) return;
  if ((RAD_ATT_FLASH == fd->state) && (0 == mask))  return;
  short No  = dsp->sp;
  short x0  = ca[No].x1;
  short y0  = ca[No].y1;
  rFont->DrawNText(surf,x0,y0,amber,fd->data);
  return;
}
//------------------------------------------------------------------
//  Draw Button
//------------------------------------------------------------------
void CBKAP140Gauge::DrawButton(char st)
{ ca[1].Draw (st);          // AP Engage button
  ca[2].Draw (st);          // HDG button
  ca[3].Draw (st);          // NAV button
  ca[4].Draw (st);          // APR button
  ca[5].Draw (st);          // RVS button
  
  ca[7].Draw (st);          // ALT button
  ca[8].Draw (st);          // DN button
  ca[9].Draw (st);          // UP button
  ca[20].Draw(st);          // Arm button
  ca[10].Draw( 0);          // Rotary button
  return;
}
//------------------------------------------------------------------
//  Draw ambiant
//------------------------------------------------------------------
void CBKAP140Gauge::DrawAmbient()
{	char st = (PAN->GetPOW());
	ca[6].Draw (st);          // ENG Light
	//--- Auto throttle -----------------------
	ca[22].Draw(PAN->GasST());				// Gaz control
	return;
}
//------------------------------------------------------------------
//  Draw all fields
//------------------------------------------------------------------
void CBKAP140Gauge::Draw (void)
{ if (0 == PAN) Get140Panel(); 
  if (0 == PAN) return;
  char st = (PAN->GetPOW());
  // Draw button images
  EraseSurfaceRGBA(surf,0);
  //----Draw the Text on display surface ------------------
  RADIO_DSP  *dsp = fieldTAB;
  while (dsp->fn) DrawField(dsp++);
  //----Display surface -----------------------------------
  if (PAN->engLite()) ca[16].DrawOnSurface(surf,0);
  ca[15].DrawOnSurface(surf,PAN->VrtARM());         // ALT armed
  ca[17].DrawOnSurface(surf,PAN->LatARM());         // Nav Armed
  ca[21].DrawOnSurface(surf,PAN->Blink());          // Alert
  RenderSurface(quad,surf);
  DrawButton(st);
  RepeatClick();
  return;
}
//===== END OF FILE ===============================================================