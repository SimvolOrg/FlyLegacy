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
//  Create a JOYSTICK DESCRIPTOR
//==============================================================================
SJoyDEF::SJoyDEF()
{ spj	= 0;
	njs	= 0;
	nax	= 0;
	nht	= 0;
	uht = 0;
	axeData	= 0;
	axePrev	= 0;
	for (int k=0; k!=32; k++) mBut[k] = 0;
}
//----------------------------------------------------------------
//	Free resources 
//----------------------------------------------------------------
SJoyDEF::~SJoyDEF()
{	if (axeData)	delete [] axeData;
	if (axePrev)	delete [] axePrev;
	if (spj)			SDL_JoystickClose(spj);
}
//----------------------------------------------------------------
//	Create the joystick entry
//----------------------------------------------------------------
bool SJoyDEF::CreateSDL(int k)
	{	njs   = k;
		spj		= SDL_JoystickOpen(k);
		if (0 == spj)	return false;
		//-- fill parameters and add joystick --------
		dName		= (char*)SDL_JoystickName(k);
    int nba = SDL_JoystickNumAxes(spj);
		nax	= nba;
    if(nba > 0)
    { axeData = new float[nba];
			axePrev = new float[nba];
    }
		//-- get number of buttons and hat ----------
		nbt	= SDL_JoystickNumButtons(spj);
		nht	= SDL_JoystickNumHats(spj);
		WARNINGLOG("JOYSTICK: %s axes=%d buttons=%d",dName,nba,nbt);
		return true;
	}
//----------------------------------------------------------------
//	Update all axes in SDL mode
//	save previous value;
//----------------------------------------------------------------
void SJoyDEF::UpdateSDL()
{	//---- Update the axes values of this joystick ----
	for (int k=0; k<nax; k++)
	{ int		v0  = SDL_JoystickGetAxis(spj, k);
		float v1 = float(v0) / 32768;
		axePrev[k]  = axeData[k];				
		axeData[k]	= v1;
	}
	//---- Update button state ------------------------
	but	= 0;
	int lim = SDL_JoystickNumButtons(spj);
	for (int k=0; k<lim; k++)
	{	U_INT st = SDL_JoystickGetButton(spj,k);
		but |= (st << k);
	}
	//---- Update hat --------------------------------
	if (uht)	hat = SDL_JoystickGetHat(spj,0);
	return;
}
//----------------------------------------------------------------
//	Check if axe has moved 
//----------------------------------------------------------------
U_INT SJoyDEF::HasMoved(CSimAxe *axe)
{	int k = axe->iAxe;						// Internal number
	float dta = fabs(axeData[k] - axePrev[k]);
	return (dta > 0.1)?(axe->msk):(0);
}
//==============================================================================
//  CREATE AN AXE DESCRIPTOR
//==============================================================================
CSimAxe::CSimAxe()
{ No            = 0;
  end           = 0;
  name          = "";
  pJoy          = 0;
	joyn					= 0;
  iAxe          = -1;
  inv						= +1;
  pos           = false;
  neutral       = 0;
  msg.sender    = 'Saxe';
  attn          = 1.0f;
}
//----------------------------------------------------------------------
//  Read AXE PARAMETERS
//----------------------------------------------------------------------
int CSimAxe::Read(SStream * st, Tag tag)
{ int b;
  char dev[128];
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
      ReadString(dev, 128, st);
      pJoy  = globals->jsm->Find(dev,joyn);
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
  this->pJoy    = from->pJoy;
	this->joyn		= from->joyn;
  this->iAxe    = from->iAxe;
  this->inv		  = from->inv;
  this->attn    = from->attn;
  return;
}
//------------------------------------------------------------------------
//  Check for same axe
//------------------------------------------------------------------------
bool CSimAxe::NotAs(CSimAxe *axe)
{ if (0 == this->pJoy)          return true;
  if (axe->pJoy != this->pJoy)  return true;
  if (axe->iAxe != this->iAxe)  return true;
  return false;
}
//------------------------------------------------------------------------
//  Assign joystick values from axn
//------------------------------------------------------------------------
void CSimAxe::Assign(CSimAxe *axn)
{ pJoy   = axn->pJoy;
  iAxe   = axn->iAxe;
	return;
}
//------------------------------------------------------------------------
//  Return axe assignement
//------------------------------------------------------------------------
void CSimAxe::Assignment(char *edt,int s)
{ int d = s-1;
	if (0 == pJoy)  *edt = 0;
  else	_snprintf(edt,d,"(J%d): Axe %02d",joyn,iAxe);
	edt[d] = 0;
  return;
}
//------------------------------------------------------------------------
//  Return axe value
//------------------------------------------------------------------------
float CSimAxe::Value(JOY_NULL_AREA *nz)
{ float f = pJoy->axeData[iAxe] * inv;
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
{ float f = pJoy->axeData[iAxe] * inv;
	//----Normalize value ------------------------------------------
	int	nx = neutral;
	if (nx && (f > nz[nx].lo) && (f < nz[nx].hi)) f = nz[nx].md;
  return f * attn;
}

//==============================================================================
//  CREATE A SIM BUTTON OBJECT
//==============================================================================
CSimButton::CSimButton()
{ pjoy  = 0;
  kset  = 0;
  cmde  = 0;
  nBut  = 0;
  Stat  = 0;
  joyn  = 0;
}
//----------------------------------------------------------------------
//  Unregister the button
//----------------------------------------------------------------------
CSimButton::~CSimButton()
{ if (pjoy) pjoy->StoreVector(nBut,0);
}
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
  { char dev[128];
    char stag[5];
    switch(tag)
    {
    case 'njoy':  // Internal number
      ReadInt(&joyn,stream);
      return TAG_READ;

    case 'devc':  // joystick name
      ReadString(dev, 128, stream);
      pjoy = globals->jsm->Find(dev,joyn);
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
//    Group commande my be posted to all controls of a same group
//===============================================================================
CJoysticksManager CJoysticksManager::instance;
//-------------------------------------------------------------------------
//	Constructor
//-------------------------------------------------------------------------
CJoysticksManager::CJoysticksManager()
{	jsh		= 0;
	uht		= 0;
	busy	= 0;
	modify	= 0;
}
//-------------------------------------------------------------------------
//	Constructor
//-------------------------------------------------------------------------
void CJoysticksManager::PreInit()
{ int opt = 0;
  pButCB  = NULL;
  pAxeCB  = NULL;
  globals->jsm        = this;
  //---Init neutral area -------------------------
  SetNulleArea(0.50,0);
  //----------------------------------------------
  AxeMoved.iAxe  = -1;
  AxeMoved.pJoy  = 0;
  //--------Assign plane axe name ----------------
  InitAxe( 0,JOY_TYPE_PLAN, JS_AILR_BIT,"Aileron  (Bank)",  JS_AILERON     , 0, true); 
  InitAxe( 1,JOY_TYPE_PLAN, JS_ELVR_BIT,"Elevator (Pitch)", JS_ELEVATOR    , 0, true, -1);
  InitAxe( 2,JOY_TYPE_PLAN, JS_RUDR_BIT,"Rudder (Heading)", JS_RUDDER      , 0, true, -1);
  InitAxe( 3,JOY_TYPE_PLAN, JS_ELVT_BIT,"Elevator Trim",    JS_TRIM        , 0, true);
	//---Group toes -------------------------------------------------------------------
  InitAxe( 4,JOY_TYPE_PLAN, JS_OTHR_BIT,"Right Toe-brake",  JS_RITE_TOE		 , JOY_GROUP_TOES, false);
  InitAxe( 5,JOY_TYPE_PLAN, JS_OTHR_BIT,"Left  Toe-brake",  JS_LEFT_TOE    , JOY_GROUP_TOES, false);
	//---Group throttle ---------------------------------------------------------------
  InitAxe( 6,JOY_TYPE_PLAN, JS_THRO_BIT,"Throttle 1",       JS_THROTTLE_1  , JOY_THROTTLE, false,-1);
  InitAxe( 7,JOY_TYPE_PLAN, JS_THRO_BIT,"Throttle 2",       JS_THROTTLE_2  , JOY_THROTTLE, false,-1);
  InitAxe( 8,JOY_TYPE_PLAN, JS_THRO_BIT,"Throttle 3",       JS_THROTTLE_3  , JOY_THROTTLE, false,-1);
  InitAxe( 9,JOY_TYPE_PLAN, JS_THRO_BIT,"Throttle 4",       JS_THROTTLE_4  , JOY_THROTTLE, false,-1);
	//---Group mixture -----------------------------------------------------------------
  InitAxe(10,JOY_TYPE_PLAN, JS_OTHR_BIT,"Mixture 1",        JS_MIXTURE_1   , JOY_MIXTURE, false);
  InitAxe(11,JOY_TYPE_PLAN, JS_OTHR_BIT,"Mixture 2",        JS_MIXTURE_2   , JOY_MIXTURE, false);
  InitAxe(12,JOY_TYPE_PLAN, JS_OTHR_BIT,"Mixture 3",        JS_MIXTURE_3   , JOY_MIXTURE, false);
  InitAxe(13,JOY_TYPE_PLAN, JS_OTHR_BIT,"Mixture 4",        JS_MIXTURE_4   , JOY_MIXTURE, false);
	//---Group Propellor ---------------------------------------------------------------
  InitAxe(14,JOY_TYPE_PLAN, JS_OTHR_BIT,"Prop 1",           JS_PROP_1      , JOY_PROPEL, false);
  InitAxe(15,JOY_TYPE_PLAN, JS_OTHR_BIT,"Prop 2",           JS_PROP_2      , JOY_PROPEL, false);
  InitAxe(16,JOY_TYPE_PLAN, JS_OTHR_BIT,"Prop 3",           JS_PROP_3      , JOY_PROPEL, false);
  InitAxe(17,JOY_TYPE_PLAN, JS_OTHR_BIT,"Prop 4",           JS_PROP_4      , JOY_PROPEL, false);
	//--- Mark controls with neutral area ----------------------------------------------
  NeutralMark(0,JOY_NEUTRAL_STICK);
  NeutralMark(1,JOY_NEUTRAL_STICK);
  NeutralMark(2,JOY_NEUTRAL_STICK);
  EndMark(17);
  //----Init helicopter axis ----------------------------------------
  InitAxe(18,JOY_TYPE_HELI, JS_OTHR_BIT,"Roll Cyclic",  JS_ROLL_CYCLIC,  0, false);
  InitAxe(19,JOY_TYPE_HELI, JS_OTHR_BIT,"Pitch Cyclic", JS_PITCH_CYCLIC, 0, false);
  InitAxe(20,JOY_TYPE_HELI, JS_OTHR_BIT,"Tail Rotor",   JS_TAIL_ROTOR,   0, false);
  InitAxe(21,JOY_TYPE_HELI, JS_OTHR_BIT,"Trim",         JS_PITCHTRIM,    0, false);
  InitAxe(22,JOY_TYPE_HELI, JS_OTHR_BIT,"Collective",   JS_COLLECTIVE,   0, true);
  InitAxe(23,JOY_TYPE_HELI, JS_OTHR_BIT,"Throttle",     JS_THROTTLE,     0, true);
  EndMark(23);
  //------------------------------------------------------------------
  InitAxe(24,JOY_TYPE_GVEH, JS_OTHR_BIT,"Throttle",     JS_GAS,        0, true);
  InitAxe(25,JOY_TYPE_GVEH, JS_OTHR_BIT,"STEER",        JS_STEER ,     0, true);
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
	use		= 0;
	SDL_Init( SDL_INIT_JOYSTICK ); 
	EnumSDL();
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
	pAxeCB	= 0;
	pButCB	= 0;
	busy	= 0;
	return;
}
//-------------------------------------------------------------------------
//	Create the device list
//-------------------------------------------------------------------------
void CJoysticksManager::CreateDevList(char **men,int lim)
{	std::vector<SJoyDEF *>::iterator jk;
	int No = 0;
	for (jk=joyQ.begin(); jk!=joyQ.end(); jk++)
	{ if (No >= lim)	break;
		men[No++]	= (*jk)->dName;
	}
	//--- Limit the list ------------------------
	men[No]	= 0;
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
	modify	= m;
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
{ std::map<Tag,CSimAxe *>::iterator it;
	for (U_INT k=0; k < mapAxe.size(); k++) delete mapAxe[k];
  mapAxe.clear();
  FreeJoyList();
}
//---------------------------------------------------------------------------------
//  OPEN JOYSTICK CONTROL FILE
//---------------------------------------------------------------------------------
void CJoysticksManager::Init( )
{ PreInit();
  if (0 == use)			return;
	SStream s;
  if (OpenRStream ("System/FlyLegacyControls.txt",s))
  { // Successfully opened stream
    ReadFrom (this, &s);
    CloseStream (&s);
  }
  else WARNINGLOG("CJoystickManager : can't open %s",s.filename);
}
//-------------------------------------------------------------------------
//	Connect all axis
//-------------------------------------------------------------------------
void CJoysticksManager::ConnectAll()
{	axeCNX = JS_SURF_ALL + JS_TRIM_ALL + JS_THRO_BIT + JS_OTHR_BIT;
	return;	}
//-------------------------------------------------------------------------
//	Disconnect requested axis
//-------------------------------------------------------------------------
void CJoysticksManager::Disconnect(U_INT m)
{	axeCNX &= (-1 - m);								// Remove bit from connector mask
	return;	}
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
//  Remap a control to a destination
//--------------------------------------------------------------------------------
void CJoysticksManager::MapTo(Tag gen, Tag usr)
{ std::map<Tag,CSimAxe *>::iterator it = mapAxe.find(gen);
  if (it == mapAxe.end())  return;
  CSimAxe *axe = (*it).second;
  axe->msg.group    = usr;
  axe->msg.receiver = 0;
  return;
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
//  ASSIGN AXE axn TO  COMPONENT tag
//  1) if the component is part of a group and the all option is set, then 
//      axe is assigned to all component of the group
//  2) For each component, if a previous axe is assigned, it is then released
//---------------------------------------------------------------------------------
void CJoysticksManager::AssignAxe(CSimAxe *axe, CSimAxe *axn, U_CHAR all)
{ ReleaseAxe(axn);
	axe->Assign(axn);
	U_CHAR grp = axe->group;
	if ((0 == all) ||(0 == grp))	return;
	for (int k = 0; k != JOY_AXIS_NUMBER; k++)
  { CSimAxe *axk = &AxesList[k];
    if (axk->group != grp)  continue;
    ReleaseAxe(axk);
		axk->Assign(axn);
  }
	modify	= 1;
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
	{	CSimAxe *axn = &AxesList[k];
		if (axn->group == grp) axn->Clear();
	}
	modify	= 1;
	return;
}
//---------------------------------------------------------------------------------
//  Invert ALL COMPONENTs OF THE GROUP tag
//---------------------------------------------------------------------------------
void CJoysticksManager::Invert(CSimAxe *axe,U_CHAR all)
{ axe->Invert();
  U_CHAR grp = axe->group;
  modify = 1;
	if ((0 == all) ||(0 == grp))	return;
  for (int k = 0; k != JOY_AXIS_NUMBER; k++)
  { CSimAxe *axk = &AxesList[k];
		if (axk == axe)								continue;		// Already inverted
    if (axk->group != axe->group) continue;   // Not same group
    if (axk->pJoy  != axe->pJoy)  continue;   // Not same joystick
    if (axk->iAxe  != axe->iAxe)  continue;   // Not same axis
    axk->Invert();
  }
	//modify	= 1;
  return;
}
//------------------------------------------------------------------------
//  Clear Requested Axe
//------------------------------------------------------------------------
void CJoysticksManager::ClearAxe(EAllAxes tag)
{ CSimAxe * pAxe = GetAxe(tag);
  if(pAxe)
  {
    pAxe->pJoy   = 0;
    pAxe->iAxe   = 0;
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
{ SDL_JoystickUpdate();							// Update all joystick
	U_INT hat = 0;
	std::vector<SJoyDEF *>::iterator it;
  for (it = joyQ.begin(); it != joyQ.end(); it++)
  { SJoyDEF *jdf = (*it);
		jdf->UpdateSDL();
    HandleButton(jdf);
		if (jdf->uht)	hat = jdf->hat;
    // If axe move detection active, remember the max 
    // delta axe variation to detect new axe/sim assignation
    if(pAxeCB)  DetectMove(jdf);
  }
	if (hat)	HandleHat(hat);
	return;
}
//------------------------------------------------------------------------
//  Move detector
//------------------------------------------------------------------------
void CJoysticksManager::DetectMove(SJoyDEF * p)
{ for(int nba=0; nba<p->nax; nba++)
  { float dif = p->axePrev[nba] - p->axeData[nba];
    // consider a valid move at a minimum of ?
    if(dif > 0.2f || dif < -0.2f)
    {	AxeMoved.iAxe = nba;
      AxeMoved.pJoy = p;
			AxeMoved.joyn = p->jNumber();
      if(pAxeCB)
			{	pAxeCB(&AxeMoved, wmID);
        p->axePrev[nba] = p->axeData[nba];
      }
		}
	}
	return;
}
//--------------------------------------------------------------------------------
// Scan button state array and call button callbacks
//--------------------------------------------------------------------------------
void CJoysticksManager::HandleButton(SJoyDEF * jsd)
{ U_INT one = 1;
  int   end = jsd->nbt;
  for (int k=0; k < end; k++)
    { U_INT bit = (one << k) & jsd->but;
      //---Proceed according to mode -------------
      if ((0 != bit) && AssignCallBack(jsd,k))	continue;
      //---Process according to type -------------
      CSimButton *sbt = jsd->GetButton(k);
      if (0 == sbt)															continue;
      Tag cmde            = sbt->cmde;
      CKeyDefinition *kdf = sbt->kdf;
      Tag             grp =  kdf->GetSet();
      Tag             kid =  kdf->GetTag();
      //---Check type of key --------------------
      switch (kdf->GetType())
      { case KEY_REPEAT:
            if (bit)							globals->kbd->Stroke(grp,kid);            
            continue;
        case KEY_SET_ON:
            if (sbt->Tr01(bit))		globals->kbd->Stroke(grp,kid);
            continue;
      }
            
  }
  return;
}
//--------------------------------------------------------------------------------
// Handle Hat event
//--------------------------------------------------------------------------------
bool CJoysticksManager::HandleHat(U_INT hat)
{	if (Time)		{Time--; return false; }
	Time	= 10;
	if (hat & SDL_HAT_LEFT)		return globals->kbd->Stroke('cmra','cplf');
	if (hat & SDL_HAT_RIGHT)	return globals->kbd->Stroke('cmra','cprt');
	if (hat & SDL_HAT_UP)			return globals->kbd->Stroke('cmra','cpup');
	if (hat & SDL_HAT_DOWN)		return globals->kbd->Stroke('cmra','cpdn');
	Time	= 0;
	return false;
}
//----------------------------------------------------------------------
//  Assign callback to button
//----------------------------------------------------------------------
bool CJoysticksManager::AssignCallBack(SJoyDEF * pJoy,int k)
{ if (0 == pButCB)					return false;
  pButCB(pJoy, k, wbID);
  return true;
}
//----------------------------------------------------------------------
//  Free Joystick list
//----------------------------------------------------------------------
void CJoysticksManager::FreeJoyList()
{
  std::vector<SJoyDEF *>::iterator it;
  SJoyDEF *    pjs;

  for (it = joyQ.begin(); it != joyQ.end(); it++)
  {
    pjs = *it;
    if(pjs)	SAFE_DELETE(pjs);	
  }
  joyQ.clear();
}
//---------------------------------------------------------------------------------
// Enum all devices and allocate corresponding 
// axes array storage with duplicate
//---------------------------------------------------------------------------------
void CJoysticksManager::EnumSDL()
{	int nbj	= SDL_NumJoysticks();
	SJoyDEF    * pJINF = 0;
	for (int k=0; k<nbj; k++)
	{	pJINF = new SJoyDEF();
	  if (pJINF->CreateSDL(k))	joyQ.push_back(pJINF);
		else											delete pJINF;
	}
	use	= joyQ.size();
	Time	= 0;
	return;
}
//------------------------------------------------------------------------------
//  FIND Joystick descriptor by number
//------------------------------------------------------------------------------
SJoyDEF *CJoysticksManager::GetJoystickNo(int No)
{ std::vector<SJoyDEF *>::iterator it;
  for (it = joyQ.begin(); it != joyQ.end(); it++)
  { SJoyDEF *jsd = (*it);
   if (jsd->njs == No)  return jsd;
  }
  return 0;
}
//==============================================================================
// FIND JOYSTICK DESCRIPTOR by name (not const protected version)
//==============================================================================
SJoyDEF *CJoysticksManager::Find(char * name,int jn)
{
  std::string str;

  int iPos;
  if(name)
  {
    std::string strname(name);

    //
    // remove last char if blank
    // (jslib side effect ?)
    //
    iPos = strname.find_last_of(' ');
    if(iPos == int(strname.size()-1)) strname.resize(strname.size()-1);

    if(strname.size() > 0)
    { 
      std::vector<SJoyDEF *>::iterator it;
      for (it = joyQ.begin(); it != joyQ.end(); it++)
      { SJoyDEF *jdf = (*it);
        str = jdf->dName;
        // remove last char if blank
        iPos = str.find_last_of(' ');
        if(iPos == int(str.size()-1))    str.resize(str.size()-1);
        if ((str == strname) && (jn == jdf->njs)) return jdf;
      }
    }
  }
  return 0;
}
//-------------------------------------------------------------------------------
//  Poll Axe value:  Used by all control surface to get raw value of control
//-------------------------------------------------------------------------------
void CJoysticksManager::Poll(EAllAxes axe, float &v)
{	CSimAxe *pa = GetAxe(axe);
  if (pa && pa->IsConnected(axeCNX))	v = pa->Value(nZON);
	return;
}

//--------------------------------------------------------------------------------
//  Check For Axe
//--------------------------------------------------------------------------------
bool CJoysticksManager::HasAxe(EAllAxes axe)
{ CSimAxe * pa = GetAxe(axe);
  return (pa->pJoy != 0);
}
//-------------------------------------------------------------------------------
//  Return Axe value
//  TODO: Implement rating coefficient
//-------------------------------------------------------------------------------
float CJoysticksManager::AxeVal(CSimAxe *pa)
{ if (0 == pa)        return 0;
  if (0 == pa->pJoy)  return 0;
  //----Normalize value ------------------------------------------
	return pa->Value(nZON);
}
//-------------------------------------------------------------------------------
//  Return raw Axe value
//  value before normalization in [0,1] if any
//-------------------------------------------------------------------------------
float CJoysticksManager::RawVal(CSimAxe *pa)
{ if (0 == pa)        return 0;
  if (0 == pa->pJoy)  return 0;
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
  SJoyDEF *joy  = axe->pJoy;
	if (!joy)									return;
	//--- Reconnect gas control --------------------
	axeCNX |= joy->HasMoved(axe);
	globals->fui->DrawNoticeToUser("Joystick: Reconnect controls",5);
	return;
}
//--------------------------------------------------------------------------------
//  Send messages related to PMT group (Prop-Mixture-Throttle)
//	This function is called from the aircraft TimeSlice() routine
//	to get the joystick values for above controls
//  Any  control in above groups that have an axe assigned to them
//  receive a message with the corresponding axe value.
//--------------------------------------------------------------------------------
void CJoysticksManager::SendGroupPMT(U_CHAR nbu)
{ Update();										// Refresh values
	if (GasDisconnected())	return CheckControl('thr1');
	CSimAxe * axe = 0;
  for (int k=JOY_FIRST_PMT; k!=JOY_AXIS_NUMBER; k++)
  { axe = AxesList + k;
    if (0 == (axe->group & JOY_GROUP_PMT))  return;
    if (0 == axe->pJoy)                   continue;
		//--- If disconnected check for reconnection ------
    SMessage *msg = &axe->msg;
		//--- Skip non existing engine or control ---------
    if (msg->user.u.engine > nbu) continue;
    msg->realData = axe->Value(nZON);			//AxeVal(axe);
    msg->user.u.datatag = axe->cmd;
    Send_Message(msg);
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
//--------------------------------------------------------------------------------
void CJoysticksManager::SendGroup(U_INT ng,Tag cmd,U_CHAR nbu)
{ if (GasDisconnected())					return;
	CSimAxe *axe	= 0;
  U_INT    grp	= AxesList[ng].group;             // Name of the group
	cmde					= cmd;														// Store command
  for (int k=ng; k!=JOY_AXIS_NUMBER; k++)
  { axe = AxesList + k;
    if (0 == (axe->group & grp))  return;         // End of group
    SMessage *msg = &axe->msg;
    if (msg->user.u.engine > nbu) return;         // End of group
    msg->user.u.datatag = cmd;
    Send_Message(msg);
  }
  return;
}
//--------------------------------------------------------------------------------
//  clear all plane messages related to PMT group (Prop-Mixture-Throttle)
//--------------------------------------------------------------------------------
void CJoysticksManager::ClearGroupPMT()
{ CSimAxe * axe = 0;
  for (int k=JOY_FIRST_PMT; k!=JOY_AXIS_NUMBER; k++)
  { axe = AxesList + k;
    if (0 == (axe->group & JOY_GROUP_PMT))  return;
    if (0 == axe->pJoy)           continue;
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
void CJoysticksManager::UseHat(SJoyDEF *jsp,char s)
{	jsp->SetHat(s);
  jsh	= jsp->jNumber();
	uht	= s;
  modify = 1;
	if (0 == s)		return;
	//--- Release Hat from other joysticks ------------
	std::vector<SJoyDEF *>::iterator jk;
	for (jk=joyQ.begin(); jk!=joyQ.end(); jk++)
	{	SJoyDEF *jsk = (*jk);
		if (jsk == jsp)		continue;
		jsk->SetHat(0);
	}
	//modify = 1;
	return;
}
//----------------------------------------------------------------------------------
//  Add a new Button for key definition
//----------------------------------------------------------------------------------
CSimButton *CJoysticksManager::AddButton(SJoyDEF *jsd,int nb,CKeyDefinition *kdf)
{ if (kdf->NoPCB())   return 0;
  if (0 == jsd)       return 0;
	modify					= 1;
  //----Create and assign the button -----------------------
  CSimButton *btn = new CSimButton;
  btn->pjoy       = jsd;
  btn->kset       = kdf->GetSet();
  btn->cmde       = kdf->GetTag();
  btn->nBut       = nb;
  btn->joyn       = jsd->JoystickNo();
  kdf->LinkTo(btn);
  btn->LinkTo(kdf);
  jsd->StoreVector(nb,btn);
  return btn;
}
//----------------------------------------------------------------------------------
//  Cancel button by ident (JoyStick No-Button No)
//----------------------------------------------------------------------------------
void CJoysticksManager::RemoveButton(SJoyDEF *jsd,int nb)
{ if (0 == jsd)     return;
  CSimButton *btn = jsd->GetButton(nb);
  if (0 == btn)     return;
  delete btn;
  return;
}
//---------------------------------------------------------------------------------
//	Catch joystick movement for specific windows
//---------------------------------------------------------------------------------
bool CJoysticksManager::StartDetectMoves(AxeDetectCB * pFunc, Tag id)
{ if (pAxeCB)		return false;
  wmID		= id;
  pAxeCB	= pFunc;

  // reset detected struct
  AxeMoved.iAxe = -1;
  AxeMoved.pJoy = 0;

  // Copy the current state of all joysticks
  // to the copy float array as an Idle state to detect large moves
  SJoyDEF * p1;
  std::vector<SJoyDEF *>::iterator it1;
  for (it1 = joyQ.begin(); it1 != joyQ.end(); it1++)
  { p1  = (*it1);
		p1->PushVal();
  }
	return true;
}
//------------------------------------------------------------------------------
void CJoysticksManager::StopDetectMoves()
{ pAxeCB	= 0; }
//---------------------------------------------------------------------------------
//	Catch joystick button for specific windows
//---------------------------------------------------------------------------------
bool CJoysticksManager::StartDetectButton(ButtonDetectCB * pcb, Tag id)
	{	if (pButCB)			return false;
		pButCB	=pcb;
		wbID		=id;
		return true;;
}
//------------------------------------------------------------------------------
void CJoysticksManager::StopDetectButton(void)
{pButCB = 0;}
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
    rax.pJoy	= 0;
		rax.inv		= 1;
    ReadFrom(&rax, stream);
    ProcessAxe(&rax);
    return TAG_READ;

  case 'jbtn':
    sbt = new CSimButton;
    ReadFrom(sbt,stream);
    if (ProcessButton(sbt))  return TAG_READ;
    WARNINGLOG("CJoysticksManager: Error with button %d.",sbt->nBut);
    delete sbt;
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
	SJoyDEF *jsp = GetJoystickNo(p1);
	if (jsp) jsp->SetHat(1);
  return;
}
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
{ SJoyDEF *jsd = sbt->pjoy;
  if (0 == jsd)             return false;
  CKeyDefinition *kdf = globals->kbd->FindKeyDefinitionByIds(sbt->kset,sbt->cmde);
  if (0 == kdf)             return false;
  KeyCallbackPtr pcb        = kdf->GetCallback();
  //-----------------------------------------------------------------
  int  nb  = sbt->nBut;
  kdf->LinkTo(sbt);
  sbt->LinkTo(kdf);
  jsd->StoreVector(nb,sbt);
  return true;
}
//-----------------------------------------------------------------------------------
//  Save Axis in configuration file
//-----------------------------------------------------------------------------------
void CJoysticksManager::SaveAxisConfig(SStream *st,SJoyDEF *jsd)
{ char stag[8];
  CSimAxe *axe = 0;
  int tmp;
  for (int k=0; k != JOY_AXIS_NUMBER; k++)
  { axe = AxesList+k;
		SJoyDEF *jdf = axe->pJoy;
	  if (0 == jdf)		continue;
		
    //---Write axe configuration ------------------------
    WriteTag('jaxe',  "-- joystick axe definition --", st);
    WriteTag('bgno', st);
    if (axe->attn != 1)
    { WriteTag('attn', "-- Attenuation coefficient --",st);
      WriteFloat(&axe->attn,st);
    }
    WriteTag('njoy',  " -- internal number (must precede devc) --",st);
		tmp	= 0;
		if(jdf) tmp = jdf->jNumber();
    WriteInt(&tmp,st);
    WriteTag('devc',  " -- input device name --------------------", st);
		char *dn = (jdf)?(jdf->dName):("None");
    WriteString(dn, st);
    WriteTag('Anum', "-- device axe number ----------------------", st);
		tmp	= axe->iAxe;
    WriteInt(&tmp, st);
    WriteTag('Ctrl', "-- sim control tag ------------------------", st);
    TagToString(stag, axe->gen);
    WriteString(stag, st);
    WriteTag('invt', st);
    tmp = axe->inv;
    WriteInt(&tmp, st);
    WriteTag('endo',st);
  }
  return;
}
//-----------------------------------------------------------------------------------
//  Save Button in configuration file
//	TODO:  LIMT TO JOYSTICK number of buttons 
//-----------------------------------------------------------------------------------
void CJoysticksManager::SaveButtonConfig(SStream *st,SJoyDEF *jsd)
{ char stag[8];
  CSimButton *sbt = 0;
  for (int k=0; k!=jsd->nbt; k++)
  { sbt = jsd->GetButton(k);
    if (0 == sbt) continue;
    //-----Write the configuration ----------------------------
    WriteTag('jbtn', "-- button definition --",st);
    WriteTag('bgno', st); 
    WriteTag('njoy',  " -- internal number (must precede devc) --",st);
    WriteInt(&sbt->joyn,st);
    WriteTag('devc', "-- input device name --",st);
    WriteString(jsd->dName, st);
    WriteTag('Bnum',  "-- Button number --",st);
    WriteInt(&sbt->nBut, st);
    WriteTag('kyid', "-- KeyDef tag --",st);
    TagToString(stag, sbt->cmde);
    WriteString(stag, st);
    WriteTag('kset', "-- KeySet tag --",st);
    TagToString(stag, sbt->kset);
    WriteString(stag,st);
    WriteTag('endo', st);
  }
  return;
}
//-----------------------------------------------------------------------------------
//  Save configuration in stream file
//-----------------------------------------------------------------------------------
void CJoysticksManager::SaveConfiguration()
{ char txt[128];
	int tmp = 0;
  SStream   s;
  char     *fn = "System/FlyLegacyControls.txt";
  SJoyDEF *jsd;
  std::vector<SJoyDEF *>::iterator it;
	if (0 == modify)				return;
	modify		= 0;
	txt[127]	= 0;
  //---Open a stream file -----------------------------------
  strcpy (s.filename, fn);
  strcpy (s.mode, "w");
  if (!OpenStream (&s)) {WARNINGLOG("CJoystickManager : can't write %s", fn); return;}
  //--Write file header -------------------------------------
  WriteTag('bgno', " === Begin Joystick Definition File =====" , &s); 
  //-- Write global setting ---------------------------------
	WriteTag('vHat', " -- Hat is used to change cockpit view ---",&s);
	_snprintf(txt,126,"J%d, %d",jsh,uht);
	WriteString(txt,&s);
  //-- Write neutral coefficient ----------------------------
  WriteTag('neut', " -- Neutral [0-1] stick coefficient ------", &s);
  WriteFloat(&nValue,&s);
  //---------------------------------------------------------
  for (it = joyQ.begin(); it != joyQ.end(); it++)
  { jsd = (*it);
    SaveAxisConfig(&s,jsd);
    SaveButtonConfig(&s,jsd);
  }
  //---Close the file ---------------------------------------
  WriteTag('endo', " === End of Joystick Definition file ===", &s);
  CloseStream (&s);
  return;
}
//===================END OF FILE ==========================================================