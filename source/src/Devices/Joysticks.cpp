/*
 * Joysticks.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005 Chris Wallace
 * Copyright 2008 Jean Sabatier
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

/*! \file Joysticks.cpp
 *  \brief Implements Joysticks device enum, input and Vehicule related axes
 *
 */

#include "../Include/FlyLegacy.h"
#include "../Include/Joysticks.h"
#include "../Include/Utility.h"
#include "../Include/KeyMap.h"
#include "../Include/globals.h"
#include "../Include/fui.h"
#include "RegStr.h"
//==============================================================================
//  Table for AXE NAME 
//==============================================================================
char *axeNAME[] = {
	"0(X)",
	"1(Y)",
	"2(Z)",
	"3(R)",
	"4(U)",
	"5(V)",
};
//==============================================================================
//  Table to locate first entry of a given type 
//==============================================================================
int TypeTAB[] = {
  JOY_FIRST_PLAN,               // First plane axe entry
  JOY_FIRST_HELI,               // First heli axe entry
  JOY_FIRST_GVEH,               // First Ground Vehicle axe entry
  JOY_FIRST_PMT,                // First Prop-Mix axe entry
};
//==============================================================================
//  COEFFICIENT FOR NORMALIZED AXE VALUE
//  0 Positive    values in [-1,+1] remapped to [0,1]
//  1 as it is    values in [-1,+1] remapped to [-1 +1]
//==============================================================================
//---Norme coef 1 --------------------------------
float CSimAxe::aCOEF[] = {
   0.5,
   0,
};
//--Norme coef 2 --------------------------------
float CSimAxe::bCOEF[] = {
   0.5,
   1,
};
//==============================================================================
//  JOYSTICK NEUTRAL AREA 
//  Adjust value to increase/decrease sensibility for the type of control
//==============================================================================
JOY_NULL_AREA CJoysticksManager::nZON[] = {
  { 0.0f,   0.0f,  0.0f },            // 0 Not used
  {-0.25f, +0.25f, 0.0f },            // Stick type in [-1,+1];
};
//==============================================================================
//  Create a JOYSTICK DEVICE
//==============================================================================
JoyDEV::JoyDEV(int n)
{ njs	= n;
	nht	= 0;
	uht = 0;
	msk	= 0;
	for (int k=0; k!=32; k++) mBut[k] = 0;
	//--------------------------------------------
	Reset();
	//--- Get joystick capability ----------------
	joyGetDevCaps(n, &jCap, sizeof(jCap));
	nax			= jCap.wNumAxes;
	nbt			= jCap.wNumButtons;
	GetJoystickName(njs,jCap.szRegKey,dName);
	nht			= (jCap.wCaps & JOYCAPS_HASPOV)?(1):(0);
	float dtx		= jCap.wXmax - jCap.wXmin;
	axeIncr[0]	= (dtx != 0)?(JOY_RNG_VAL / dtx):(0);	// X inc
	float dty		= jCap.wYmax - jCap.wYmin;
	axeIncr[1]	= (dty != 0)?(JOY_RNG_VAL / dty):(0);	// Y inc
	float dtz		= jCap.wZmax - jCap.wZmin;
	axeIncr[2]	= (dtz != 0)?(JOY_RNG_VAL / dtz):(0);	// Z inc
	float dtr		= jCap.wRmax - jCap.wRmin;
	axeIncr[3]	= (dtr != 0)?(JOY_RNG_VAL / dtr):(0);	// R inc
	float dtu		= jCap.wUmax - jCap.wUmin;
	axeIncr[4]	= (dtu != 0)?(JOY_RNG_VAL / dtu):(0);	// U inc
	float dtv		= jCap.wVmax - jCap.wVmin;
	axeIncr[5]	= (dtv != 0)?(JOY_RNG_VAL / dtv):(0);	// V inc
	//--------------------------------------------
	use		  = 1;
}
//----------------------------------------------------------------
//	Reset values 
//----------------------------------------------------------------
void JoyDEV::Reset()
{	//--------------------------------------------
	axeData[JOY_AXE_X] = 0;
	axeData[JOY_AXE_Y] = 0;
	axeData[JOY_AXE_Z] = 0;
	axeData[JOY_AXE_R] = 0;
	axeData[JOY_AXE_U] = 0;
	axeData[JOY_AXE_V] = 0;
	return;
}
//----------------------------------------------------------------
//	Free resources 
//----------------------------------------------------------------
JoyDEV::~JoyDEV()
{	for (int k=0; k<32; k++)
	{	CSimButton *btn  = mBut[k];
		if (btn)	delete btn;
	}
}
//----------------------------------------------------------------
//	Check if axe has moved 
//----------------------------------------------------------------
U_INT JoyDEV::HasMoved(CSimAxe *axe)
{	int k = axe->iAxe;						// Internal number
	float dta = fabs(axeData[k] - axePrev[k]);
	return (dta > 0.1)?(1):(0);
}
//----------------------------------------------------------------
//	Save actual values 
//----------------------------------------------------------------
void	JoyDEV::SaveVal()
{	int dim = JOY_AXE_NBR * sizeof(float);	
	memcpy(axePrev,axeData,dim);
	phat		= hat;
}
//----------------------------------------------------------------
//	Remove this button 
//----------------------------------------------------------------
void	JoyDEV::RemoveButton(CSimButton *btn)
{	int No = btn->GetNo();
	mBut[No]	= 0;
	delete btn;
}
//----------------------------------------------------------------
//	Store a button 
//----------------------------------------------------------------
void JoyDEV::StoreButton(int n,CSimButton *btn)
{	n &= 31; 
	mBut[n] = btn;
	strcpy(btn->devc,dName);
	return;
}
//----------------------------------------------------------------
//	Check for Hat move 
//----------------------------------------------------------------
bool JoyDEV::HatUnmoved()
{	bool um = (phat == hat);
	phat	= hat;
	return um;
}
//---------------------------------------------------------------------------------
//  Get joystick name from registery (adapted from copyright SDL library)
//---------------------------------------------------------------------------------
void JoyDEV::GetJoystickName(int index, char *key, char *buf)
{	HKEY hTopKey;
	HKEY hKey;
	DWORD dim;
	LONG regresult;
	char regkey[256];
	char regvalue[256];
	char regname[256];
	_snprintf(regkey,255,"%s\\%s\\%s",REGSTR_PATH_JOYCONFIG,key,REGSTR_KEY_JOYCURR);
	hTopKey = HKEY_LOCAL_MACHINE;
	regresult = RegOpenKeyExA(hTopKey, regkey, 0, KEY_READ, &hKey);
	if (regresult != ERROR_SUCCESS) {
		hTopKey = HKEY_CURRENT_USER;
		regresult = RegOpenKeyExA(hTopKey, regkey, 0, KEY_READ, &hKey);
	}
	if (regresult != ERROR_SUCCESS) return;
	//--- find the registry key name for the joystick's properties 
	dim = sizeof(regname);
	_snprintf(regvalue,255,"Joystick%d%s",index+1, REGSTR_VAL_JOYOEMNAME);
	regresult = RegQueryValueExA(hKey, regvalue, 0, 0, (LPBYTE)regname, &dim);
	RegCloseKey(hKey);

	if (regresult != ERROR_SUCCESS) return;
	//----- open that registry key  -----------------------------------------
	_snprintf(regkey, 255, "%s\\%s", REGSTR_PATH_JOYOEM, regname);
	regresult = RegOpenKeyExA(hTopKey, regkey, 0, KEY_READ, &hKey);
	if (regresult != ERROR_SUCCESS) return;
	//----- Get the OEM name size --------------------------------------------
	dim = sizeof(regvalue);
	if (dim > 64)	dim = 63;
	regresult = RegQueryValueExA(hKey, REGSTR_VAL_JOYOEMNAME, 0, 0, NULL, &dim);
	if (regresult == ERROR_SUCCESS) {
			// Read from the registry */
			regresult = RegQueryValueExA(hKey,REGSTR_VAL_JOYOEMNAME, 0, 0,
				(LPBYTE) buf, &dim);
		}
	buf[63]	= 0;
	RegCloseKey(hKey);

	return;
}
//---------------------------------------------------------------------------------
//  Store Axis value
//---------------------------------------------------------------------------------
void JoyDEV::StoreAxe(char No,DWORD raw)
{ float val = JOY_LOW_VAL + (float (raw) * axeIncr[No]); 
  float dif = axeData[No] - val;
	if ((dif > -JOY_THRESH)	&& (dif < +JOY_THRESH))	return;
	axePrev[No] = axeData[No];
	axeData[No]	= val / 32768;
	return;
}
//---------------------------------------------------------------------------------
//  Refresh this joystick
//---------------------------------------------------------------------------------
void JoyDEV::Refresh()
{	JOYINFOEX jinf;
	jinf.dwSize  = sizeof(jinf);
	jinf.dwFlags = JOY_RETURNALL|JOY_RETURNPOVCTS;
	if (JOYERR_NOERROR != joyGetPosEx(njs, &jinf))	return;
	//--- update axe values -----------------------------------
	DWORD flag = jinf.dwFlags;
	float val = 0;
	if (flag & JOY_RETURNX)	StoreAxe(JOY_AXE_X, jinf.dwXpos);
	if (flag & JOY_RETURNY)	StoreAxe(JOY_AXE_Y, jinf.dwYpos);
	if (flag & JOY_RETURNZ) StoreAxe(JOY_AXE_Z, jinf.dwZpos);
	if (flag & JOY_RETURNR) StoreAxe(JOY_AXE_R, jinf.dwRpos);
	if (flag & JOY_RETURNU)	StoreAxe(JOY_AXE_U, jinf.dwUpos);
	if (flag & JOY_RETURNV)	StoreAxe(JOY_AXE_V, jinf.dwVpos);
	//--- Update button state -----------------------------------
	if (flag & JOY_RETURNBUTTONS)
	{	U_INT bsta = jinf.dwButtons;			// Button state	
		msk ^=bsta;												// Changing state 
		but	= bsta;												// Save state
	}
	//--- Update POV --------------------------------------------
	if (flag & JOY_RETURNPOV)			hat	= jinf.dwPOV;
	//-----------------------------------------------------------
	if (flag & JOY_RETURNPOVCTS)	hat = jinf.dwPOV;
	return;
}
//---------------------------------------------------------------------------------
//  Process hat
//---------------------------------------------------------------------------------
void JoyDEV::HandleHat()
{	short val = hat;
	if (-1 == val)		return;
	if ( 0 == nht)		return;
	if ( 0 == uht)		return;
	if (hat == phat)	return;
	//--- Change in hat position -------------------------------
	hpos		= hat / 100;							// compute angle
	if (globals->cam->NoHatSupport())	return;
	globals->pln->pit.ActivateView(float(hpos));
	return;
}
//==============================================================================
//  CREATE AN AXE DESCRIPTOR
//==============================================================================
CSimAxe::CSimAxe()
{ strncpy(mem,"jAxe",4);
	No            = 0;
  end           = 0;
  name          = 0;
	jdev					= 0;
	joyn					= 0;
  iAxe          = -1;
  inv						= +1;
  pos           = false;
  neutral       = 0;
  msg.sender    = 'Saxe';
  attn          = 1.0f;
 *devc					= 0;
}
//----------------------------------------------------------------------
//  Delete
//----------------------------------------------------------------------
CSimAxe::~CSimAxe(){}
//----------------------------------------------------------------------
//  Read AXE PARAMETERS
//----------------------------------------------------------------------
int CSimAxe::Read(SStream * st, Tag tag)
{ int b;
  switch(tag)
    {
    case 'attn':  // Attenuation coef
      ReadFloat(&attn,st);
      return TAG_READ;
    case 'njoy':  // Internal number
      ReadInt(&b,st);
			joyn	= short(b);
      return TAG_READ;
    case 'devc':  // joystick name
      ReadString(devc, 128, st);
			jdev  = globals->jsm->Find(devc);
			return TAG_READ;

    case 'Anum':  // Axe number (0 based)
      ReadInt(&b, st);
			iAxe	= short(b);
      return TAG_READ;

    case 'Ctrl': // Sim unique related command
      ReadTag(&this->gen, st);
			TagToString(idn,gen);
      return TAG_READ;

    case 'invt':  // axe value inversion ?
      ReadInt(&b, st);
			inv *= b;
      return TAG_READ;
    }
    return TAG_IGNORED;
}
//------------------------------------------------------------------------
//  Copy value from the other object
//------------------------------------------------------------------------
void CSimAxe::Copy(CSimAxe *from)
{ this->gen     = from->gen;
  this->jdev    = from->jdev;
	this->joyn		= from->joyn;
  this->iAxe    = from->iAxe;
  this->inv		  = from->inv;
  this->attn    = from->attn;
	strncpy(devc,from->devc,64);
  return;
}

//------------------------------------------------------------------------
//  Check for same axe
//------------------------------------------------------------------------
bool CSimAxe::NotAs(CSimAxe *axe)
{ if (0 == this->jdev)          return true;
  if (axe->jdev != this->jdev)  return true;
  if (axe->iAxe != this->iAxe)  return true;
  return false;
}

//------------------------------------------------------------------------
//  Assign joystick values from axn
//------------------------------------------------------------------------
void CSimAxe::Assign(CSimAxe *axn)
{ jdev   = axn->jdev;
  iAxe   = axn->iAxe;
	char *dvn = jdev->getDevName();
	strncpy(devc,dvn,64);
	return;
}

//------------------------------------------------------------------------
//  Return axe assignement
//------------------------------------------------------------------------
void CSimAxe::Assignment(char *edt,int s)
{ int d = s-1;
	if (0 == jdev)  *edt = 0;
  else	_snprintf(edt,d,"(J%d): Axe %s",joyn,axeNAME[iAxe]);
	edt[d] = 0;
  return;
}

//------------------------------------------------------------------------
//  Return axe value
//------------------------------------------------------------------------
float CSimAxe::Value(JOY_NULL_AREA *nz)
{ float f = jdev->axeData[iAxe] * inv;
	//----Normalize value ------------------------------------------
	f  = aCOEF[pos] + (bCOEF[pos] * f);
	int	nx = neutral;
	if (nx && (f > nz[nx].lo) && (f < nz[nx].hi)) f = nz[nx].md;
  return f * attn;
}

//------------------------------------------------------------------------
//  Return raw axe value
//------------------------------------------------------------------------
float CSimAxe::RawVal(JOY_NULL_AREA *nz)
{ float f = jdev->axeData[iAxe] * inv;
	//----Normalize value ------------------------------------------
	int	nx = neutral;
	if (nx && (f > nz[nx].lo) && (f < nz[nx].hi)) f = nz[nx].md;
  return f * attn;
}

//==============================================================================
//  CREATE A SIM BUTTON OBJECT
//==============================================================================
CSimButton::CSimButton()
{	jdev	= 0;
  kset  = 0;
  cmde  = 0;
  nBut  = 0;
  Stat  = 0;
 *devc  = 0;
}
//----------------------------------------------------------------------
//  Unregister the button
//----------------------------------------------------------------------
CSimButton::~CSimButton()
{ }

//----------------------------------------------------------------------
//  Check transition from 0 to 1
//----------------------------------------------------------------------
bool CSimButton::Tr01(U_INT st)
{ U_INT old  = Stat;
  Stat       = st;
  return ((0 == old) && (0 != st));
}

//----------------------------------------------------------------------
//  Read Parameters from button description
//----------------------------------------------------------------------
int CSimButton::Read(SStream * stream, Tag tag)
  { char stag[5];
		int pm;
    switch(tag)
    {
    case 'njoy':  // Internal number
      ReadInt(&pm,stream);
      return TAG_READ;

    case 'devc':  // joystick name
      ReadString(devc, 128, stream);
			jdev = globals->jsm->Find(devc);
      return TAG_READ;

    case 'Bnum':  // button number <0 - 31>
      ReadInt(&nBut, stream);
      return TAG_READ;

    case 'kyid':  // related unique code from CKeyDefinition::kyid
      ReadString(stag, 5, stream);
      cmde = StringToTag(stag);
      return TAG_READ;

    case 'kset':  // related unique code from CKeyDefinition::kyid
      ReadString(stag, 5, stream);
      kset = StringToTag(stag);
      return TAG_READ;
    }
    return TAG_IGNORED;
  };
//===============================================================================
// CJoystickManager class
//  NOTE:  Joystick manager handle the joystick for the assigned control
//        It also serve as the repository for the remapped control
//        For instance, for a 4 engines plane, each engine may have separated
//        control for Throttle, Mixture and Prop pitch.  And each control may
//        have a diffrent name assigned by the user in the AMP file
//  Each control has a generic name such as 'thr1' or 'bld1' that stores the user
//  tag provided in the file.
//  Funtion MapTo(gen,us) remaps the generic commande to the user provided name
//          so when polled, the proper subsystem will receive the joystick value.
//  Conversaly the manager can return the user name given the generic one.
//  Function TagFrom(gen) return the user tag (if any) associated to the generic 
//        Name
//===============================================================================
//    Group commands my be posted to all controls of a same group
//===============================================================================
//-------------------------------------------------------------------------
//	Constructor
//-------------------------------------------------------------------------
CJoysticksManager::CJoysticksManager()
{	jsh		= 0;
	uht		= 0;
	busy	= 0;
	jFunCB = 0;
	mveh	= 0;
	Init();
}
//-------------------------------------------------------------------------
//	Constructor
//-------------------------------------------------------------------------
void CJoysticksManager::PreInit()
{ int opt = 0;
  globals->jsm        = this;
  //---Init neutral area -------------------------
  SetNulleArea(0.50,0);
  //----------------------------------------------
  AxeMoved.iAxe		= -1;
	AxeMoved.jdev		= 0;
  //--------Assign plane axe name ----------------
  InitAxe( 0,JOY_TYPE_PLAN, JS_AILR_BIT,"Aileron  (Bank)",		'ailr'			, 0, true); 
  InitAxe( 1,JOY_TYPE_PLAN, JS_ELVR_BIT,"Elevator (Pitch)",		'elev'			, 0, true, -1);
  InitAxe( 2,JOY_TYPE_PLAN, JS_RUDR_BIT,"Rudder (Heading)",		'rudr'      , 0, true,  1);
  InitAxe( 3,JOY_TYPE_PLAN, JS_ELVT_BIT,"Elevator Trim",			'trim'      , 0, true);
	//---Group toes -------------------------------------------------------------------
  InitAxe( 4,JOY_TYPE_PLAN, JS_BRAK_BIT,"Right Toe-brake",		'rtoe'			, 0, false);
  InitAxe( 5,JOY_TYPE_PLAN, JS_BRAK_BIT,"Left  Toe-brake",		'ltoe'			, 0, false);
	//---Group throttle ---------------------------------------------------------------
  InitAxe( 6,JOY_TYPE_PLAN, JS_THRO_BIT,"Throttle 1",					'thr1'  , JOY_THROTTLE, false,1);
  InitAxe( 7,JOY_TYPE_PLAN, JS_THRO_BIT,"Throttle 2",					'thr2'  , JOY_THROTTLE, false,1);
  InitAxe( 8,JOY_TYPE_PLAN, JS_THRO_BIT,"Throttle 3",					'thr3'  , JOY_THROTTLE, false,1);
  InitAxe( 9,JOY_TYPE_PLAN, JS_THRO_BIT,"Throttle 4",					'thr4'  , JOY_THROTTLE, false,1);
	//---Group mixture -----------------------------------------------------------------
  InitAxe(10,JOY_TYPE_PLAN, JS_MIXT_BIT,"Mixture 1",					'mix1'   , JOY_MIXTURE, false);
  InitAxe(11,JOY_TYPE_PLAN, JS_MIXT_BIT,"Mixture 2",					'mix2'   , JOY_MIXTURE, false);
  InitAxe(12,JOY_TYPE_PLAN, JS_MIXT_BIT,"Mixture 3",					'mix3'   , JOY_MIXTURE, false);
  InitAxe(13,JOY_TYPE_PLAN, JS_MIXT_BIT,"Mixture 4",					'mix4'   , JOY_MIXTURE, false);
	//---Group Propellor ---------------------------------------------------------------
  InitAxe(14,JOY_TYPE_PLAN, JS_PROP_BIT,"Prop 1",							'pro1'      , JOY_PROPEL, false);
  InitAxe(15,JOY_TYPE_PLAN, JS_PROP_BIT,"Prop 2",							'pro2'      , JOY_PROPEL, false);
  InitAxe(16,JOY_TYPE_PLAN, JS_PROP_BIT,"Prop 3",							'pro3'      , JOY_PROPEL, false);
  InitAxe(17,JOY_TYPE_PLAN, JS_PROP_BIT,"Prop 4",							'pro4'      , JOY_PROPEL, false);
	//--- Mark controls with neutral area ----------------------------------------------
  NeutralMark(0,JOY_NEUTRAL_STICK);
  NeutralMark(1,JOY_NEUTRAL_STICK);
  NeutralMark(2,JOY_NEUTRAL_STICK);
  EndMark(17);
  //----Init helicopter axis ----------------------------------------
  InitAxe(18,JOY_TYPE_HELI, JS_HELI_BIT,"Roll Cyclic",  JS_ROLL_CYCLIC,  0, false);
  InitAxe(19,JOY_TYPE_HELI, JS_HELI_BIT,"Pitch Cyclic", JS_PITCH_CYCLIC, 0, false);
  InitAxe(20,JOY_TYPE_HELI, JS_HELI_BIT,"Tail Rotor",   JS_TAIL_ROTOR,   0, false);
  InitAxe(21,JOY_TYPE_HELI, JS_HELI_BIT,"Trim",         JS_PITCHTRIM,    0, false);
  InitAxe(22,JOY_TYPE_HELI, JS_HELI_BIT,"Collective",   JS_COLLECTIVE,   0, true);
  InitAxe(23,JOY_TYPE_HELI, JS_HELI_BIT,"Throttle",     JS_THROTTLE,     0, true);
  EndMark(23);
  //------------------------------------------------------------------
  InitAxe(24,JOY_TYPE_GVEH, JS_VEHI_BIT,"Throttle",     JS_GAS,        0, true);
  InitAxe(25,JOY_TYPE_GVEH, JS_VEHI_BIT,"STEER",        JS_STEER ,     0, true);
  //------------------------------------------------------------------
  EndMark(25);
	//------------------------------------------------------------------
	SetMessage(4, SUBSYSTEM_BRAKE_CONTROL,BRAKE_LEFT,'btoe');
	SetMessage(5, SUBSYSTEM_BRAKE_CONTROL,BRAKE_RITE,'btoe');
  //------------------------------------------------------------------
  SetMessage( 6,SUBSYSTEM_THROTTLE_CONTROL,1,'thro');
  SetMessage( 7,SUBSYSTEM_THROTTLE_CONTROL,2,'thro');
  SetMessage( 8,SUBSYSTEM_THROTTLE_CONTROL,3,'thro');
  SetMessage( 9,SUBSYSTEM_THROTTLE_CONTROL,4,'thro');
  //--------------------------------------------------
  SetMessage(10,SUBSYSTEM_MIXTURE_CONTROL, 1,'mixt');
  SetMessage(11,SUBSYSTEM_MIXTURE_CONTROL, 2,'mixt');
  SetMessage(12,SUBSYSTEM_MIXTURE_CONTROL, 3,'mixt');
  SetMessage(13,SUBSYSTEM_MIXTURE_CONTROL, 4,'mixt');
  //--------------------------------------------------
  SetMessage(14,SUBSYSTEM_PROPELLER_CONTROL, 1,'blad');
  SetMessage(15,SUBSYSTEM_PROPELLER_CONTROL, 2,'blad');
  SetMessage(16,SUBSYSTEM_PROPELLER_CONTROL, 3,'blad');
  SetMessage(17,SUBSYSTEM_PROPELLER_CONTROL, 4,'blad');
  //------------------------------------------------------------------
	CollectDevices();
	return;
}
//---------------------------------------------------------------------------------
//  OPEN JOYSTICK CONTROL FILE
//---------------------------------------------------------------------------------
void CJoysticksManager::Init( )
{ PreInit();
	SStream s(this,"System","FlyLegacyControls.txt");
}
//---------------------------------------------------------------------------------
//  Collect joystick list
//---------------------------------------------------------------------------------
void CJoysticksManager::CollectDevices()
{	JOYINFOEX joyinfo;
	int nbj = joyGetNumDevs();
	nDev		= nbj;
	for (int k=0; k<nbj; k++)
	{	joyinfo.dwSize = sizeof(joyinfo);
		joyinfo.dwFlags = JOY_RETURNALL;
		if ( joyGetPosEx(k, &joyinfo) != JOYERR_NOERROR ) continue;
		//--- Create  a new device ---------------------------------
		JoyDEV *jdf		= new JoyDEV(k);

		devQ.push_back(jdf);
	}
	return;
}
//---------------------------------------------------------------------------------
//  Limit button detection
//---------------------------------------------------------------------------------
void CJoysticksManager::LimitButton(char *dvn,int lim)
{	JoyDEV *jdf = Find(dvn);
	if (0 == jdf)	return;
//	jdf->SetButtonNb(lim);
	return;
}
//-------------------------------------------------------------------------
//	Check if I am free
//-------------------------------------------------------------------------
bool CJoysticksManager::IsBusy()
{	bool t = (busy != 0);
	busy++;
	return t;
}
//-------------------------------------------------------------------------
//	Free the manager
//-------------------------------------------------------------------------
void	CJoysticksManager::SetFree()
{	busy--;
	if (busy > 0)	return;
	//--- Clear all vectors ----------------
	jFunCB  = 0;
	busy		= 0;
	return;
}
//-------------------------------------------------------------------------
//	Create the device list
//-------------------------------------------------------------------------
void CJoysticksManager::CreateDevList(char **men,U_INT lim)
{	U_INT k;
	for (k=0; k<devQ.size(); k++)
	{ if (k >= lim)	break;
		men[k]	= devQ[k]->dName;
	}
	//--- Limit the list ------------------------
	men[k]	= 0;
}

//----------------------------------------------------------------------------------
//  Set the nulle area
//----------------------------------------------------------------------------------
void CJoysticksManager::SetNulleArea(float n,char m)
{ nValue              = n;
  nZON[JOY_NEUTRAL_STICK].lo = -n;
  nZON[JOY_NEUTRAL_STICK].md = 0;
  nZON[JOY_NEUTRAL_STICK].hi = +n;
  nZON[JOY_NEUTRAL_STICK].ap = 1 - n;
  return;
}
//----------------------------------------------------------------------------------
//  Get AXE from requested type
//----------------------------------------------------------------------------------
CSimAxe *CJoysticksManager::NextAxe(CSimAxe *from,int type)
{ int first = TypeTAB[type];
  if (0 == from)            return &AxesList[first];
  int nxt = from->No + 1;
  if (1 == from->end)       return 0;
	return &AxesList[nxt];
}
//-------------------------------------------------------------------------------
//  Init the axe message
//-------------------------------------------------------------------------------
void CJoysticksManager::SetMessage(int m,Tag des,int unit,Tag cmd)
{ AxesList[m].cmd =  cmd;
  SMessage *msg   = &AxesList[m].msg;
  msg->group          = des;
  msg->sender         = 'Saxe';
  msg->id             = MSG_SETDATA;
  msg->dataType       = TYPE_REAL;
  msg->user.u.engine  = unit;
  return;
}
//-------------------------------------------------------------------------------
//  Init a plane axis
//  nx = axe number
//  tp = vehicle type
//  nm = axe namee
//  t  = generic name
//  gp = group
//  p  = positive indicator
// NOTE: all axis are entered into the map table "mapAxe" so they can
//      be retrieved by the generic name 't'
//-------------------------------------------------------------------------------
void CJoysticksManager::InitAxe(int nx,U_CHAR tp,U_INT ms,char *nm,Tag t,int gp, bool p,float inv)
{ if (nx > JOY_AXIS_NUMBER) gtfo("Increase axe table"); 
	CSimAxe *axe = &AxesList[nx]; 
	TagToString(axe->idn,t);
  axe->No    = nx;                // Entry number
  axe->name  = nm;                // Axe name
	axe->msk	 = ms;
  axe->gen   = t;                 // Function tag
  axe->group = gp;                // Group link
  axe->pos   = p;                 // Positive indicator
  axe->type  = tp;                // Type
	axe->inv	 = inv;
  mapAxe[t]  = axe;
  return;
}
//-------------------------------------------------------------------------------
//  Mark Last entry for a given List
//-------------------------------------------------------------------------------
void CJoysticksManager::EndMark(int nx)
{ AxesList[nx].end = 1;
}
//-------------------------------------------------------------------------------
//  Mark axe for neutral control
//-------------------------------------------------------------------------------
void CJoysticksManager::NeutralMark(int nx,U_CHAR type)
{ AxesList[nx].neutral = type;
}
//---------------------------------------------------------------------------------
//  Free all resources
//---------------------------------------------------------------------------------
CJoysticksManager::~CJoysticksManager()
{ mapAxe.clear();
	//--- Clear device list ----------------------
	for (U_INT k=0; k<devQ.size(); k++)	delete devQ[k];
	devQ.clear();
	//--- Clear unassigned list ------------------
	for (U_INT k=0; k<butQ.size(); k++)
	{	CSimButton *btn = butQ[k];
		if (btn)	delete btn;
	}
	butQ.clear();
}
//-------------------------------------------------------------------------
//	Reconnect requested axis
//-------------------------------------------------------------------------
void CJoysticksManager::Reconnect(U_INT m)
{	axeCNX |= m;											// Add bit to connector mask
	return;	}
//----------------------------------------------------------------------------------
//  Locate plane axe by tag
//----------------------------------------------------------------------------------
CSimAxe * CJoysticksManager::GetAxe(Tag tag)
{ std::map<Tag,CSimAxe *>::iterator it = mapAxe.find(tag);
  return (it == mapAxe.end())?(0):((*it).second);
}
//--------------------------------------------------------------------------------
//  Return user tag from generic name
//--------------------------------------------------------------------------------
Tag CJoysticksManager::TagFrom(Tag gen)
{ std::map<Tag,CSimAxe *>::iterator it = mapAxe.find(gen);
  if (it == mapAxe.end())  return 0;
  CSimAxe *axe = (*it).second;
  return axe->msg.group;
}
//---------------------------------------------------------------------------------
//  get Invert property
//---------------------------------------------------------------------------------
int CJoysticksManager::GetInvert(EAllAxes tag)
{ std::map<Tag,CSimAxe *>::iterator it = mapAxe.find(tag);
  return (it == mapAxe.end())?(0):((*it).second->GetInvert());
}
//---------------------------------------------------------------------------------
//  Set Invert property
//---------------------------------------------------------------------------------
void CJoysticksManager::SetInvert(EAllAxes tag,int p)
{ std::map<Tag,CSimAxe *>::iterator it = mapAxe.find(tag);
  if (it == mapAxe.end())  return;
  CSimAxe *axe = (*it).second;
}
//---------------------------------------------------------------------------------
//  RELEASE AXE ASSIGNATION
//---------------------------------------------------------------------------------
void CJoysticksManager::ReleaseAxe(CSimAxe *axn)
{ CSimAxe *axe = 0;
  for (int k = 0; k != JOY_AXIS_NUMBER; k++)
  { axe = &AxesList[k];
    if (axe->NotAs(axn)) continue;
    axe->Clear();
  }
  return;
}
//---------------------------------------------------------------------------------
//  ASSIGN AXE axn TO  current axe. 
//	axn is the new detected axis
//	-axn has a pointer to joystick and a number for the new axis.
//	-axe is the current axe that is reassigned
//	1) the new axe (axn) is released from previous assignation
//  2)  
//---------------------------------------------------------------------------------
void CJoysticksManager::AssignAxe(CSimAxe *axe, CSimAxe *axn, U_CHAR all)
{ ReleaseAxe(axn);
	axe->Assign(axn);
	U_CHAR grp = axe->group;
	if ((0 == all) ||(0 == grp))	return;
	for (int k = 0; k != JOY_AXIS_NUMBER; k++)
  { CSimAxe *axk = AxesList+ k;
    if (axk->group != grp)  continue;
    ReleaseAxe(axk);
		axk->Assign(axn);
  }
  return;
}
//---------------------------------------------------------------------------------
//  Clear single or group of axes
//---------------------------------------------------------------------------------
void CJoysticksManager::Clear(CSimAxe *axe,U_CHAR all)
{ axe->Clear();
	U_CHAR grp = axe->group;
  if ((0 == all) ||(0 == grp))	return;
	for (int k=0; k!=JOY_AXIS_NUMBER; k++)
	{	CSimAxe *axn = AxesList + k;
		if (axn->group == grp) axn->Clear();
	}
	return;
}
//---------------------------------------------------------------------------------
//  Invert ALL COMPONENTs OF THE GROUP tag
//---------------------------------------------------------------------------------
void CJoysticksManager::Invert(CSimAxe *axe,U_CHAR all)
{ axe->Invert();
  U_CHAR grp = axe->group;
	if ((0 == all) ||(0 == grp))	return;
  for (int k = 0; k != JOY_AXIS_NUMBER; k++)
  { CSimAxe *axk = AxesList + k;
		if (axk == axe)								continue;		// Already inverted
    if (axk->group != axe->group) continue;   // Not same group
    if (axk->jdev  != axe->jdev)  continue;   // Not same joystick
    if (axk->iAxe  != axe->iAxe)  continue;   // Not same axis
    axk->Invert();
  }
  return;
}

//==========================================================================================
// read all joystick axes and buttons values
// If button is pushed an has a callback, call it directly
// If axe detection mode active, call the move's special callback
// If button detection mode active, call the button's special callback
//==========================================================================================
void CJoysticksManager::Update()
{
	U_INT hat = 0;
	//----------------------------------------------------------
	for (U_INT k=0; k<devQ.size(); k++)
	{	JoyDEV *jsd = devQ[k];
		jsd->Refresh();
		HandleButton(jsd);
		jsd->HandleHat();
    // If axe move detection active, remember the max 
    // delta axe variation to detect new axe/sim assignation
    if(jFunCB)  DetectMove(jsd);
	}
	return;
}
//------------------------------------------------------------------------
//  Reset all values
//	TODO: Mettre valeur en object
//------------------------------------------------------------------------
void CJoysticksManager::Register(CObject *obj)
{	mveh	= obj;
	for (U_INT k=0; k<devQ.size(); k++)	devQ[k]->Reset();
	axeCNX = JS_AUTO_BIT + JS_RUDR_BIT;
	return;
}
//------------------------------------------------------------------------
//  Move detector
//------------------------------------------------------------------------
void CJoysticksManager::DetectMove(JoyDEV *J)
{ for(int nba=0; nba<JOY_AXE_NBR; nba++)
  { float dif = J->axePrev[nba] - J->axeData[nba];
    // consider a valid move at a minimum of ?
    if(fabs(dif) < 0.05f)			continue;
		J->axePrev[nba]	= J->axeData[nba];
    AxeMoved.iAxe = nba;
    AxeMoved.jdev = J;
		AxeMoved.joyn = J->jNumber();
		(*jFunCB)(JOY_AXE_MOVE,J,&AxeMoved,0,wmID);
		return;
	}
	//--- Check with hat --------------------------
	if (J->HatUnmoved())			return;
	(*jFunCB)(JOY_HAT_MOVE,J,0,0,wmID);
	return;
}

//--------------------------------------------------------------------------------
// Scan button state array and call button callbacks
//--------------------------------------------------------------------------------
void CJoysticksManager::HandleButton(JoyDEV * jsd)
{ U_INT one = 1;
  int   end = jsd->nbt;
  for (int k=0; k < end; k++)
    { U_INT nbit = (one << k); 
			U_INT val  = jsd->IsON(nbit);
			//---Proceed change according to mode -------------
      if ((val) && AssignCallBack(jsd,k))	continue;
      //---Process according to type --------------------
      CSimButton *sbt = jsd->GetButton(k);
      if (0 == sbt)																		continue;
      Tag cmde            = sbt->cmde;
      CKeyDefinition *kdf = sbt->kdf;
      Tag             grp =  kdf->GetSet();
      Tag             kid =  kdf->GetTag();
			val									=  jsd->Val(nbit);
      //---Check type of key --------------------
      switch (kdf->GetType())
      { case KEY_REPEAT:
            if (val)							globals->kbd->Stroke(grp,kid);            
            continue;
        case KEY_SET_ON:
            if (sbt->Tr01(val))		globals->kbd->Stroke(grp,kid);
            continue;
      }
  }
 //--- Change mask ----------------------------
 jsd->SwapMask();         
  return;
}
//----------------------------------------------------------------------
//  Assign callback to button
//----------------------------------------------------------------------
bool CJoysticksManager::AssignCallBack(JoyDEV * J,int k)
{ if (0 == jFunCB)					return false;
	(*jFunCB)(JOY_BUT_MOVE,J,0,k,wmID);
  return true;
}

//------------------------------------------------------------------------------
//  FIND Joystick descriptor by number
//------------------------------------------------------------------------------
JoyDEV *CJoysticksManager::GetJoystickNo(U_INT No)
{ if (No >= devQ.size())		return 0;
	return devQ[No];
}

//==============================================================================
// FIND JOYSTICK DESCRIPTOR by name 
//==============================================================================
JoyDEV *CJoysticksManager::Find(char * name)
{for (U_INT k=0; k<devQ.size(); k++)
	{ JoyDEV *jdev = devQ[k];
		if (jdev->NotNamed(name))	continue;
		return jdev;
	}
  
  return 0;
}

//--------------------------------------------------------------------------------
//  Check For Axe
//--------------------------------------------------------------------------------
bool CJoysticksManager::HasAxe(EAllAxes axe)
{ CSimAxe * pa = GetAxe(axe);
  return (pa->jdev != 0);
}

//-------------------------------------------------------------------------------
//  Return Axe value
//  TODO: Implement rating coefficient
//-------------------------------------------------------------------------------
float CJoysticksManager::AxeVal(CSimAxe *pa)
{ if (0 == pa)        return 0;
  if (0 == pa->jdev)  return 0;
  //----Normalize value ------------------------------------------
	return pa->Value(nZON);
}

//-------------------------------------------------------------------------------
//  Return raw Axe value
//  value before normalization in [0,1] if any
//-------------------------------------------------------------------------------
float CJoysticksManager::RawVal(CSimAxe *pa)
{ if (0 == pa)        return 0;
  if (0 == pa->jdev)  return 0;
  //----Normalize value ------------------------------------------
	return pa->RawVal(nZON);
}

//--------------------------------------------------------------------------------
//	Check if throttle is moved
//	If moved by user, then give gas control to user
//--------------------------------------------------------------------------------
void CJoysticksManager::CheckControl(Tag tag)
{	CSimAxe *axe  = GetAxe(tag);
	if (!axe)									return;
  JoyDEV *joy  = axe->jdev;
	if (!joy)									return;
	//--- Reconnect gas control --------------------
	if (!joy->HasMoved(axe))	return; 
	axeCNX |= JS_GROUPBIT;
	globals->fui->DrawNoticeToUser("Joystick recovers controls",5);
	return;
}
//================================================================================
//	Following commands are dedicated to the registered aircraft
//================================================================================
//--------------------------------------------------------------------------------
//	Set number of engines
//--------------------------------------------------------------------------------
void CJoysticksManager::SetNbEngines(CObject *obj,char nb)
{	if (obj != mveh)					return;
	engNB	= nb;
	return;
}
//--------------------------------------------------------------------------------
//  Remap a control to a destination (used when changing aircraft)
//	The controller (ctl) message is remapped to the new destination dst
//	NOTE:  Only the registered aircraft is accepted.  Other animated aircraft
//				 dont have joystick control
//--------------------------------------------------------------------------------
void CJoysticksManager::MapTo(CObject *obj,Tag ctl, Tag dst)
{ if (obj != mveh)					return;
	std::map<Tag,CSimAxe *>::iterator it = mapAxe.find(ctl);
  if (it == mapAxe.end())		return;
  CSimAxe *axe = (*it).second;
  axe->msg.group    = dst;
  axe->msg.receiver = 0;
  return;
}
//-------------------------------------------------------------------------
//	Aircraft request to Connect all axis
//-------------------------------------------------------------------------
void CJoysticksManager::JoyConnectAll(CObject *obj)
{	if (obj != mveh)	return;			// Ignore
	axeCNX = JS_AUTO_BIT + JS_RUDR_BIT;
	return;	}
//-------------------------------------------------------------------------
//	Aircarft request to Disconnect axis m
//-------------------------------------------------------------------------
void CJoysticksManager::JoyDisconnect(CObject *obj,U_INT m)
{	if (obj != mveh)	return;
	axeCNX &= (-1 - m);								// Remove bit from connector mask
	return;	}
//-------------------------------------------------------------------------------
//  Poll Axe value:  Used by all control surface to get raw value of control
//	NOTE:  Only the registered aircraft is accepted. Other animated aircraft
//				dont have joystick control
//-------------------------------------------------------------------------------
void CJoysticksManager::Poll(CObject *obj,EAllAxes tag, float &v)
{	if (obj != mveh)		return;
	CSimAxe *pa = GetAxe(tag);
  if (pa && pa->IsConnected(axeCNX))	v = pa->Value(nZON);
	return;
}
//--------------------------------------------------------------------------------
//  Send messages related to PMT group (Prop-Mixture-Throttle)
//	This function is called from the aircraft TimeSlice() routine
//	to get the joystick values for above controls
//  Any  control in above groups that have an axe assigned to them
//  receive a message with the corresponding axe value.
//	NOTE: This command is accepted only from the registered aircraft
//--------------------------------------------------------------------------------
void CJoysticksManager::SendGroupPMT(CObject *obj,U_CHAR nbu)
{ if (obj != mveh)				return;
	Update();										// Refresh values
	if (GasDisconnected())	return CheckControl('thr1');
	CSimAxe * axe = 0;
  for (int k=JOY_FIRST_PMT; k!=JOY_AXIS_NUMBER; k++)
  { axe = AxesList + k;
    if (0 == (axe->group & JOY_GROUP_PMT))  return;
    if (0 == axe->jdev)                   continue;
		//--- If disconnected check for reconnection ------
    SMessage *msg = &axe->msg;
		//--- Skip non existing engine or control ---------
    if (msg->user.u.engine > nbu) continue;
    msg->realData = axe->Value(nZON);			//AxeVal(axe);
    msg->user.u.datatag = axe->cmd;
    globals->pln->ReceiveMessage(msg);
  }
  return;
}

//--------------------------------------------------------------------------------
//  Send messages related to given group (Prop or Mixture or Throttle)
//  Any  control in requested group receives a message with the corresponding
//  datatag command. This is reserved to a keyboard key that for instance may
//  increase or decrease all controls in the group in one command
//  NOTE:  We use the same message receiver as in the SendGroupPMT because the 
//         same controls are adressed (i;e Throttle or Mixture or Prop pitch)
//				
//--------------------------------------------------------------------------------
void CJoysticksManager::SendGroup(U_INT ng,Tag cmd)
{ CSimAxe *axe	= 0;
  U_INT    grp	= AxesList[ng].group;             // Name of the group
	cmde					= cmd;														// Store command
  for (int k=ng; k!=JOY_AXIS_NUMBER; k++)
  { axe = AxesList + k;
    if (0 == (axe->group & grp))						return;         // End of group
    SMessage *msg = &axe->msg;
    if (msg->user.u.engine > U_INT(engNB))	return;         // End of group
    msg->user.u.datatag = cmd;
    globals->pln->ReceiveMessage(msg);
  }
  return;
}
//--------------------------------------------------------------------------------
//  clear all plane messages related to PMT group (Prop-Mixture-Throttle)
//--------------------------------------------------------------------------------
void CJoysticksManager::ClearGroupPMT(CObject *obj)
{ if (obj != mveh)				return;	// Ignore
	CSimAxe * axe = 0;
  for (int k=JOY_FIRST_PMT; k!=JOY_AXIS_NUMBER; k++)
  { axe = AxesList + k;
    if (0 == (axe->group & JOY_GROUP_PMT))  return;
    if (0 == axe->jdev)           continue;
    SMessage *msg = &axe->msg;
    msg->receiver = 0;
  }
  return;
}

//--------------------------------------------------------------------------------
//  Get the attenuation coefficient
//--------------------------------------------------------------------------------
float CJoysticksManager::GetAttenuation(EAllAxes cmd)
{ CSimAxe *axe = GetAxe(cmd);
  return (axe)?(axe->attn):(0);
}
//--------------------------------------------------------------------------------
//  Set the attenuation coefficient
//--------------------------------------------------------------------------------
void CJoysticksManager::SetAttenuation(EAllAxes cmd,float atn)
{ CSimAxe *axe = GetAxe(cmd);
  if (axe) axe->attn = atn;
  return;
}
//--------------------------------------------------------------------------------
//  Check for neutral area
//--------------------------------------------------------------------------------
float CJoysticksManager::Neutral(float f, int nx)
{ float val = f;
  if ((f > nZON[nx].lo) && (f < nZON[nx].hi)) val = nZON[nx].md;
  return val;
}
//-----------------------------------------------------------------------------------
//  Use Hat for the given joystick
//-----------------------------------------------------------------------------------
void CJoysticksManager::UseHat(JoyDEV *jsp,char s)
{	jsp->SetHat(s);
  jsh	= jsp->jNumber();
	uht	= s;
	if (0 == s)		return;
	//--- Release Hat from other joysticks ------------
	std::vector<JoyDEV *>::iterator jk;
	for (jk=devQ.begin(); jk!=devQ.end(); jk++)
	{	JoyDEV *jsk = (*jk);
		if (jsk == jsp)		continue;
		jsk->SetHat(0);
	}
	return;
}

//----------------------------------------------------------------------------------
//  Add a new Button for key definition
//----------------------------------------------------------------------------------
CSimButton *CJoysticksManager::AddButton(JoyDEV *jsd,int nbt,CKeyDefinition *kdf)
{ if (kdf->NoPCB())   return 0;
  //----Create and assign the button -----------------------
	CSimButton *btn = new CSimButton();
	btn->jdev				= jsd;
	btn->nBut				= nbt;
  btn->kset       = kdf->GetSet();
  btn->cmde       = kdf->GetTag();
  kdf->LinkTo(btn);
  btn->LinkTo(kdf);
	jsd->StoreButton(nbt,btn);
	//--- Remove from unassigned list ------------------------
	std::vector<CSimButton *>::iterator rb;
	for (rb = butQ.begin(); rb != butQ.end(); rb++)
	{	CSimButton *but = (*rb);
		if (btn->kset != but->kset)		continue;
		if (btn->cmde != but->cmde)		continue;
		//------------------------------------------------------
		butQ.erase(rb);
		delete but;
		break;
	}
  return btn;
}

//----------------------------------------------------------------------------------
//  Cancel button by ident (JoyStick No-Button No)
//----------------------------------------------------------------------------------
void CJoysticksManager::RemoveButton(CSimButton *btn)
{ if (0 == btn)			return;
	JoyDEV  *jsd    = btn->GetDevice();
  if (0 == jsd)     return;
  jsd->RemoveButton(btn);
  return;
}

//---------------------------------------------------------------------------------
//	Catch any movement for specific windows
//---------------------------------------------------------------------------------
bool CJoysticksManager::StartDetection(JoyDetectCB *F, Tag W)
{	if (jFunCB)		return false;
	jFunCB	= F;
	wmID		= W;
	//----- reset detected struct ---------------------
  AxeMoved.iAxe = -1;
  AxeMoved.jdev = 0;
  // Copy the current state of all joysticks
  // to the copy float array as an Idle state to detect large moves
  for (U_INT k=0; k<devQ.size(); k++)
	{	JoyDEV *jdev = devQ[k];
	}
	return true;
}

//===============================================================================
// CStreamObject methods
//===============================================================================
int CJoysticksManager::Read (SStream *stream, Tag tag)
{ float pm = 0;
	int rc = TAG_IGNORED, itag;
  CSimButton    *sbt = 0;
  SJoyDEF      *pj  = 0;
  CSimAxe       rax;
  const CKeyDefinition * pkey = NULL;

  switch(tag)
  {
	case 'vHat':
		ProcessHat(stream);
		return TAG_READ;
  case 'sets':  // global settings
    ReadInt(&itag, stream);
    return TAG_READ;
  case 'neut':
    ReadFloat(&pm,stream);
		SetNulleArea(pm,0);
    return TAG_READ;
  case 'jaxe':
		rax.jdev	= 0;
		rax.inv		= 1;
    ReadFrom(&rax, stream);
    ProcessAxe(&rax);
    return TAG_READ;

  case 'jbtn':
    sbt = new CSimButton;
    ReadFrom(sbt,stream);
    if (ProcessButton(sbt))  return TAG_READ;
		//-- Enter in unassigned list -----------------
		butQ.push_back(sbt);
    return TAG_READ;
  }

  // Tag was not processed by this object, it is unrecognized
  char s[16];
  TagToString (s, tag);
  WARNINGLOG ("CJoysticksManager::Read : Unrecognized tag <%s>", s);
  return TAG_IGNORED;
}
//-----------------------------------------------------------------------------------
//  Process Hat
//-----------------------------------------------------------------------------------
void CJoysticksManager::ProcessHat(SStream *stream)
{ char txt[128];
	int p1;
	int p2;
	ReadString(txt,128,stream);
	if (2 != sscanf(txt," J%d , %d",&p1,&p2)) return;
	jsh	= char(p1);
	uht = char(p2);
	if (0 == p2)	return;
	JoyDEV *jsp = GetJoystickNo(p1);
	if (jsp) jsp->SetHat(1);
  return;
}
//-----------------------------------------------------------------------------------
//  Process Hat
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
//  Process the axe
//-----------------------------------------------------------------------------------
bool CJoysticksManager::ProcessAxe(CSimAxe *from)
{ CSimAxe *axe = GetAxe(from->gen);     // plane axe ?
  if (axe)  {axe->Copy(from); return true;}
  WARNINGLOG("CJoysticksManager::Read : device not connected");
  return true;
}
//-----------------------------------------------------------------------------------
//  Process the button
//  -Enter the descriptor into the map and link to Key definition
//-----------------------------------------------------------------------------------
bool CJoysticksManager::ProcessButton(CSimButton *sbt)
{ JoyDEV *jsd = sbt->jdev;
  if (0 == jsd)             return false;
  CKeyDefinition *kdf = globals->kbd->FindKeyDefinitionByIds(sbt->kset,sbt->cmde);
  if (0 == kdf)             return false;
  //-----------------------------------------------------------------
  int  nb  = sbt->nBut;
  kdf->LinkTo(sbt);
  sbt->LinkTo(kdf);
  jsd->StoreButton(nb,sbt);
  return true;
	}
//-----------------------------------------------------------------------------------
//  Save Axis in configuration file
//-----------------------------------------------------------------------------------
void CJoysticksManager::SaveAxisConfig(CStreamFile &sf)
{ char stag[8];
  CSimAxe *axe = 0;
  int tmp;
  for (int k=0; k != JOY_AXIS_NUMBER; k++)
  { axe = AxesList+k;
		JoyDEV *jdf = axe->jdev;
		//--- skip axe which has never been assigned --------
		if (axe->NoDevice())				continue;
		if (axe->NulDev())					continue;
    //---Write axe configuration ------------------------
    sf.WriteTag('jaxe',  "-- joystick axe definition --");
    sf.DebObject();
    if (axe->attn != 1)
    { sf.WriteTag('attn', "-- Attenuation coefficient --");
      sf.WriteFloat(axe->attn);
    }
    sf.WriteTag('njoy',  " -- internal number (must precede devc) --");
		tmp = (jdf)?(jdf->jNumber()):(0);
    sf.WriteInt(tmp);
    sf.WriteTag('devc',  " -- input device name --------------------");
    sf.WriteString(axe->devc);
    sf.WriteTag('Anum', "-- device axe number ----------------------");
    sf.WriteInt(axe->iAxe);
    sf.WriteTag('Ctrl', "-- sim control tag ------------------------");
    TagToString(stag, axe->gen);
    sf.WriteString(stag);
    sf.WriteTag('invt',"");
    sf.WriteInt(axe->inv);
    sf.EndObject();
  }
  return;
}
//-----------------------------------------------------------------------------------
//  Save one Button in configuration file
//	 
//-----------------------------------------------------------------------------------
void CJoysticksManager::SaveOneButton(CStreamFile &sf,CSimButton *sbt)
{	char stag[8];
	//-----Write the configuration ----------------------------
  sf.WriteTag('jbtn', "-- button definition --");
  sf.DebObject(); 
  sf.WriteTag('devc', "-- input device name --");
  sf.WriteString(sbt->devc);
  sf.WriteTag('Bnum',  "-- Button number --");
  sf.WriteInt(sbt->nBut);
  sf.WriteTag('kyid', "-- KeyDef tag --");
  TagToString(stag, sbt->cmde);
  sf.WriteString(stag);
  sf.WriteTag('kset', "-- KeySet tag --");
  TagToString(stag, sbt->kset);
  sf.WriteString(stag);
  sf.EndObject();
	return;
}
//-----------------------------------------------------------------------------------
//  Save Button in configuration file
//	 
//-----------------------------------------------------------------------------------
void CJoysticksManager::SaveButtonConfig(CStreamFile &sf, JoyDEV *jsd)
{ CSimButton *sbt = 0;
  for (int k=0; k!=jsd->nbt; k++)
  { sbt = jsd->GetButton(k);
    if (0 == sbt) continue;
    SaveOneButton(sf,sbt);
  }
  return;
}
//-----------------------------------------------------------------------------------
//  Save configuration in stream file
//-----------------------------------------------------------------------------------
void CJoysticksManager::SaveConfiguration()
{ char txt[128];
	int tmp = 0;
  CStreamFile   sf;
  char     *fn = "System/FlyLegacyControls.txt";
  JoyDEV *jsd;
  std::vector<JoyDEV *>::iterator it;
	modify		= 0;
	txt[127]	= 0;
  //---Open a stream file -----------------------------------
  sf.OpenWrite(fn); 
  //--Write file header -------------------------------------
  sf.WriteTag('bgno', " === Begin Joystick Definition File ====="); 
  //-- Write global setting ---------------------------------
	sf.WriteTag('vHat', " -- Hat is used to change cockpit view ---");
	_snprintf(txt,126,"J%d, %d",jsh,uht);
	sf.WriteString(txt);
  //-- Write neutral coefficient ----------------------------
  sf.WriteTag('neut', " -- Neutral [0-1] stick coefficient ------");
  sf.WriteFloat(nValue);
  //---------------------------------------------------------
	SaveAxisConfig(sf);
  for (it = devQ.begin(); it != devQ.end(); it++)
  { jsd = (*it);
    SaveButtonConfig(sf,jsd);
  }
	//---Save unassigned list ---------------------------------
	for (U_INT k=0; k<butQ.size(); k++)	SaveOneButton(sf,butQ[k]);
  //---Close the file ---------------------------------------
  sf.EndObject();
  sf.Close();
  return;
}

//===================END OF FILE ==========================================================