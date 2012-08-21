/*
 * LogicSubsystems.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
 * Copyright 2007      jean Sabatier
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

/*! \file LogicSubsystems.cpp
 *  \brief Implements vehicle subsystems related to logical connections
 */
#include "../Include/Globals.h"						// JSDEV* needed for trace
#include "../Include/Subsystems.h"
#include "../Include/Fui.h"						// JSDEV* needed for trace
#include "../Include/FuiParts.h"
#include "../Include/FuiProbe.h"
using namespace std;
//=========================================================================================
//  Probe mode for Generic indicator
//=========================================================================================
char *GenrMOD[] = 
  //-0-----1------2------3------4-----5--------6---------7-----
  {"LT",  "GT",  "LE",  "EQ",  "NE", "GE",  "XEED",   "BETW" };
//=========================================================================================
// CSubsystem
//=========================================================================================
CSubsystem::CSubsystem (void)
{ TypeIs (SUBSYSTEM_BASE);
  mveh  = 0;
  timK	= 2.0f;
  ratK	= 2.0f;
  Trace	= 0;
  indn	= 0;
  sound = 0;
  indnTarget = 0;
  indnMode   = INDN_NONE;
  uNum = 1;
  gNum = 1;
  hwId = HW_STATE;
  FrameNo	= 0;
  //----------------------------------
  gage = 0;
}
//-------------------------------------------------------------------------
//  Destroy subsystem
//-------------------------------------------------------------------------
CSubsystem::~CSubsystem (void)
{ globals->psys = 0;
  if (sound) sound->Release();
}
//-------------------------------------------------------------------------
//  Change identity
//-------------------------------------------------------------------------
void CSubsystem::SetIdent(Tag id)
{ unId = id;
  TagToString (unId_string, unId);
  return;
}
//-------------------------------------------------------------------------
//	JSDEV* Check if update is needed for this current cycle
//	indicated by Frame counter
//	Note that after this test only a new FrNo would get a true answer
//-------------------------------------------------------------------------
bool CSubsystem::NeedUpdate(U_INT FrNo)
{	U_INT old	= FrameNo;						// Last update
	FrameNo		= FrNo;							  // New Cycle
	return (old != FrNo);
}
//-------------------------------------------------------------------------
//	Read subsystem tags
//-------------------------------------------------------------------------
int CSubsystem::Read (SStream *stream, Tag tag)
{ char fWAV[64];
  int rc = TAG_IGNORED;
  U_INT  pm;
  switch (tag) {
  case 'uniD':
  case 'unId':
    // Unique ID
    ReadTag (&unId, stream);
    TagToString (unId_string, unId);
    return TAG_READ;

  case 'timK':
    // Exponential time constant
    ReadFloat (&timK, stream);
    indnMode = INDN_LINEAR;
    return TAG_READ;

  case 'ratK':
    // Linear time constant
    ReadFloat (&ratK, stream);
    indnMode = INDN_EXPONENTIAL;
    return TAG_READ;

  case 'indn':
    // Initial indication value
    ReadFloat (&indn, stream);
    return TAG_READ;

  case 'fWAV':
    // Triggered sound effect filename
    ReadString (fWAV, 64, stream);
    //MEMORY_LEAK_MARKER ("electrwav")
    sound = globals->snd->ReserveSoundBUF(fWAV);
    //MEMORY_LEAK_MARKER ("electrwav")
    return TAG_READ;

  case 'uNum':
    ReadUInt (&pm, stream);
    uNum = (U_SHORT)pm;
      //
      //#ifdef _DEBUG	
      //{	FILE *fp_debug;
	     // if(!(fp_debug = fopen("__DDEBUG_mfm.txt", "a")) == NULL)
	     // {
		    //  int test = 0;
		    //  fprintf(fp_debug, "CSubsystem::Read (%s) %d\n", TagToString (unId), uNum);
		    //  fclose(fp_debug); 
      //}	}
      //#endif
    return TAG_READ;

  case 'hwId':
    // DEPRECATED - Hardware type
    {
      char s[64];
      ReadString (s, 64, stream);
      if (stricmp (s, "UNKNOWN") == 0)              hwId = HW_UNKNOWN;
      else if (stricmp (s, "BUS") == 0)             hwId = HW_BUS;
      else if (stricmp (s, "SWITCH") == 0)          hwId = HW_SWITCH;
      else if (stricmp (s, "LIGHT") == 0)           hwId = HW_LIGHT;
      else if (stricmp (s, "FUSE") == 0)            hwId = HW_FUSE;
      else if (stricmp (s, "STATE") == 0)           hwId = HW_STATE;
      else if (stricmp (s, "GAUGE") == 0)           hwId = HW_GAUGE;
      else if (stricmp (s, "CIRCUIT") == 0)         hwId = HW_CIRCUIT;
      else if (stricmp (s, "RADIO") == 0)           hwId = HW_RADIO;
      else if (stricmp (s, "FLAP") == 0)            hwId = HW_FLAP;
      else if (stricmp (s, "HILIFT") == 0)          hwId = HW_HILIFT;
      else if (stricmp (s, "GEAR") == 0)            hwId = HW_GEAR;
      else if (stricmp (s, "BATTERY") == 0)         hwId = HW_BATTERY;
      else if (stricmp (s, "ALTERNATOR") == 0)      hwId = HW_ALTERNATOR;
      else if (stricmp (s, "ANNUNCIATOR") == 0)     hwId = HW_ANNUNCIATOR;
      else if (stricmp (s, "GENERATOR") == 0)       hwId = HW_GENERATOR;
      else if (stricmp (s, "CONTACTOR") == 0)       hwId = HW_CONTACTOR;
      else if (stricmp (s, "SOUNDFX") == 0)         hwId = HW_SOUNDFX;
      else if (stricmp (s, "FLASHER") == 0)         hwId = HW_FLASHER;
      else if (stricmp (s, "INVERTER") == 0)        hwId = HW_INVERTER;
      else if (stricmp (s, "UNITLESS") == 0)        hwId = HW_UNITLESS;
      else if (stricmp (s, "UNBENT") == 0)          hwId = HW_UNBENT;
      else if (stricmp (s, "SCALE") == 0)           hwId = HW_SCALE;
      else if (stricmp (s, "OTHER") == 0)           hwId = HW_OTHER;
      else {
        WARNINGLOG ("%s : Unknown HW type %s", GetClassName(), s);
      }
    }   
    rc = TAG_READ;
    break;

  case 'dbug':
    // Set debugging flag
    return TAG_READ;
  }

  if (rc != TAG_READ) {
    // Tag was not processed by this object, it is unrecognized
    CStreamFile* sf = (CStreamFile*)stream->stream;
    int line = sf->GetLine();

    WARNINGLOG ("%s : Unknown tag <%s> near line %d", GetClassName(), TagToString(tag),line);
  }

  return rc;
}
//-----------------------------------------------------------------------
//  Set linear mode
//-----------------------------------------------------------------------
void CSubsystem::SetTimK(float t)
{ timK = t;
  indnMode = INDN_LINEAR;
  return;
}
//-----------------------------------------------------------------------
//  All parameters are read
//-----------------------------------------------------------------------
void CSubsystem::ReadFinished (void)
{ //----JSDEV Check if this dependent should be traced ---------
  char entry[16];
  char name[8];
  int	op = 0;
  strcpy(entry,"sub.");
  TagToString(name,unId);
  strcat(entry,name);
  GetIniVar ("TRACE", entry, &op);
  if (op)	  Trace |= TRACE_ACTV;
}
//---------------------------------------------------------------------------------
//  Write the electrical system
//---------------------------------------------------------------------------------
void CSubsystem::Write (SStream *stream)
{

}
//---------------------------------------------------------------------------------
//  Set subsystem type
//---------------------------------------------------------------------------------
void CSubsystem::TypeIs (Tag t)
{
  // Set subsystem type and convert to string (useful when debugging)
  type = t;
  TagToString (type_string, type);

  // Initialize unique subsystem ID to subsystem type // 
  // This should be overridden with a <unid> tag in the subsystem definition
  unId = type;
  TagToString (unId_string, unId);
}
//---------------------------------------------------------------------
//	JSDEV* This is now a virtual function called from
//	Send_Message only when there is yet no receiver in message
//	This call is not needed in ReceiveMessage
//	NOTE: Specific joker datatag = '$TYP' force the tag group to be
//				interpreted as the Type of the component.
//---------------------------------------------------------------------
bool CSubsystem::MsgForMe (SMessage *msg)
{ //-------------------------------------------------------
  //  For debugg and trap a specific subsystem that does not 
  //  respond to  message , just uncomment
  //  the following lines and change id
  //-------------------------------------------------------
  //Tag idn = 'Thr1';                     // Change subsystem name here
	//if (type == 'THRO')
  // if ((type == idn) && (msg->group == idn))           
  // int a = 0;                          // Put a break point here
  //-------------------------------------------------------
	bool idByType   = (msg->user.u.datatag == '$TYP');
  bool matchGroup = (idByType)?(msg->group == type):(msg->group == unId);
  bool hwNull     = (msg->user.u.hw == HW_UNKNOWN);
  bool hwMatch    = (msg->user.u.hw == (unsigned int) hwId);
  bool unitNull   = (msg->user.u.unit == 0);
  bool unitMatch  = (msg->user.u.unit == uNum);
  return matchGroup && (hwNull || hwMatch) && (unitNull || unitMatch);
}
//---------------------------------------------------------------------
//	JSDEV* Check if I am the message receiver
//	Return the CDependent (if this is the case) initial state
//---------------------------------------------------------------------
bool CSubsystem::IsReceiver(SMessage *msg)
{	//--- for debugging only ---------------------
	//if (unId == 'mfSS')
	//	int a = 0;
	if (!MsgForMe(msg))	return false;
  msg->receiver	= this;
	return true;
}
//---------------------------------------------------------------------
//  Edit data on the probe window
//---------------------------------------------------------------------
void CSubsystem::Probe(CFuiCanva *cnv)
{ char edt[16];
  _snprintf(edt,16,"%.04f",timK);
  cnv->AddText( 1,"timK:");
  cnv->AddText( 6,edt,1);

  _snprintf(edt,16,"%.04f",ratK);
  cnv->AddText( 1,"ratK:");
  cnv->AddText( 6,edt,1);

  _snprintf(edt,16,"%.04f",indn);
  cnv->AddText( 1,"indn:");
  cnv->AddText( 6,edt,1);
  return;
}
//---------------------------------------------------------------------
//
// CSubsystem supports the following messages:
//
// GETDATA:
//  indn - Indication value (float)
//
// SETDATA:
//  indn - Indication value (float)
//-----------------------------------------------------------------------
EMessageResult CSubsystem::ReceiveMessage (SMessage *msg)
{ Tag dtag = msg->user.u.datatag;

   switch (msg->id) {
      case MSG_GETDATA:
        switch (dtag) {
          case 0:
          case 'indn':
			      msg->realData = indn;
			      return MSG_PROCESSED;
					case 'gets':
					case '$TYP':
						msg->voidData = this;
						return MSG_PROCESSED;
        }

      case MSG_SETDATA:
        switch (dtag)
        //-- Value ---------------------------
        { case 0:
          case 'indn':
            indnTarget  = (float)msg->realData;
			      return MSG_PROCESSED;
        //-- Set associated gauge ------------
          case 'gets':
            msg->voidData = this;
            return MSG_PROCESSED;
        }
   }
  return MSG_IGNORED;
}
//----------------------------------------------------------------------------------
//	JSDEV*  To check if all subsystems are timesliced once and 
//	only once during a simulation cycle, TimeSlice occuring at Frame
//	number 10 are traced if the corresponding option is set in the
//	ini file
//	[TRACE]
//	TimeSlice=1
//----------------------------------------------------------------------------------
void CSubsystem::TraceTimeSlice(U_INT FrNo)
{	if (FrNo != 10)					return;
    TRACE ("TimeSlice Frame %d:  %25s %s", FrNo,GetClassName(),GetIdString());
	return;	}
//----------------------------------------------------------------------------------
//  Time slice the subsystem
//----------------------------------------------------------------------------------
void CSubsystem::TimeSlice (float dT,U_INT FrNo)			// JSDEV*
{ 
  //--Check for trace --------------------------------------------------------------
  //	if (globals->Trace.Has(TRACE_TIMESLICE)) TraceTimeSlice(FrNo);
	// Update actual indication
  switch (indnMode) {

  case INDN_NONE:
		// Actual value tracks target value with no time lag
		indn = indnTarget;
		return;

  case INDN_LINEAR:	{
	// Use timK (in seconds) as linear coefficient
	//	Do  not overshoot target value
		float dta = (dT / timK);
		if (dta > 1)	dta = 1;
		indn += (indnTarget - indn) * dta;
		return;	}

  case INDN_EXPONENTIAL:
    // Use ratK (in seconds) as exponential coeffiecient
		indn += (indnTarget - indn) * (1.0f - (float)exp(-dT / ratK));
		return;

  case INDN_MOD360: {
    float dtm = (dT / timK);
    if (dtm > 1) dtm = 1;
    indn += DifMod360(indn, indnTarget) * dtm;
    return; }

  default:
    // Actual value tracks target value with no time lag
    indn = indnTarget;
    break;
  }
}
//----------------------------------------------------------------------------------
void CSubsystem::Print (FILE *f)
{ /*
  fprintf (f, "CSubsystem:\n");
  fprintf (f, "  type=%s\tunId=%s\thw=", type_string, unId_string);
  switch (hwId) {
  case HW_UNKNOWN:
    fprintf (f, "UNKNOWN\n");
    break;

  case HW_BUS:
    fprintf (f, "BUS\n");
    break;

  case HW_SWITCH:
    fprintf (f, "SWITCH\n");
    break;

  case HW_LIGHT:
    fprintf (f, "LIGHT\n");
    break;

  case HW_FUSE:
    fprintf (f, "FUSE\n");
    break;

  case HW_STATE:
    fprintf (f, "STATE\n");
    break;

  case HW_GAUGE:
    fprintf (f, "GAUGE\n");
    break;

  case HW_OTHER:
    fprintf (f, "OTHER\n");
    break;

  case HW_CIRCUIT:
    fprintf (f, "CIRCUIT\n");
    break;

  case HW_FLAP:
    fprintf (f, "FLAP\n");
    break;

  case HW_HILIFT:
    fprintf (f, "HILIFT\n");
    break;

  case HW_GEAR:
    fprintf (f, "GEAR\n");
    break;

  case HW_BATTERY:
    fprintf (f, "BATTERY\n");
    break;

  case HW_ALTERNATOR:
    fprintf (f, "ALTERNATOR\n");
    break;

  case HW_GENERATOR:
    fprintf (f, "GENERATOR\n");
    break;

  case HW_CONTACTOR:
    fprintf (f, "CONTACTOR\n");
    break;

  case HW_SOUNDFX:
    fprintf (f, "SOUNDFX\n");
    break;

  case HW_FLASHER:
    fprintf (f, "FLASHER\n");
    break;

  case HW_INVERTER:
    fprintf (f, "INVERTER\n");
    break;

  case HW_UNITLESS:
    fprintf (f, "UNITLESS\n");
    break;

  case HW_UNBENT:
    fprintf (f, "UNBENT\n");
    break;

  case HW_SCALE:
    fprintf (f, "SCALE\n");
    break;

  case HW_RADIO:
    fprintf (f, "RADIO\n");
    break;

  case HW_ANNUNCIATOR:
    fprintf (f, "ANNUNCIATOR\n");
    break;
  }
  fprintf (f, "  unit=%d\tgNum=%d\n", uNum, gNum);
  fprintf (f, "  timK = %f\tratK=%f\n", timK, ratK);
  fprintf (f, "  indn = %f\tindnTarget=%f\n", indn, indnTarget);
	*/
}
//-------------------------------------------------------------------------
//	NULL Subsystem constructor: Init all values to NULL
//-------------------------------------------------------------------------
CNullSubsystem::CNullSubsystem(void)
{	TypeIs ('!!!!');
	type	= 'Null';
	unId	= 'Null';
	Trace	= 0;
	timK	= 0;
	ratK	= 0;
	indn	= 0;
	indnTarget	= INDN_NONE;
	uNum	= 0;
	gNum	= 0;
	hwId	= HW_UNKNOWN;
  FrameNo = 0;
}
//-------------------------------------------------------------------------
EMessageResult CNullSubsystem::ReceiveMessage (SMessage *msg)
{	msg->result = MSG_IGNORED;
	return MSG_IGNORED;	}
//===========================================================================
// CDependent
//	This is the basis of all logical systems
//	CDependent has sevral tags and ste values and we should clarify them
//	char	state:  Is used as the logical state of the subsystem.
//	char active:	Is used to means that some current flows into the substem.
//							A switch for instance, may be in the logical state 1, but
//							with no current flowing.  Thus active = 0 in that situation
//	Tag st8t:		Used at initialization (coming from AMP file) to set the
//							the initial value of state.
//	Reading from Message
//							Use only 'actv' to get active value (current flowing)
//							Use 'stat' or 'st8t' to get state  value (logical state)
//	NOTE				Most of the time, the logical AND/OR operations are used to
//							set the dependent active status 
//===========================================================================
CDependent::CDependent (void)
{ TypeIs (SUBSYSTEM_DEPENDENT);
  hwId = HW_STATE;
  // Initialize read datatags
  st8t        = -1;              // Invalid state
  stat        = false;
  splay       = 0;
  offV        = 0;
  mode        = '_OR_';
  volt        = 0;
  load        = 0;
  mVlt        = 0;
  freQ        = 0;
  //-- Initialize logical states ----
	indx				= 0;
  state       = 0;
  active      = false;
  dflact	    = false;
  // Initialize real-time electrical states
  eNum        = 1;
  //---------------------------------
  rise = 1.0f;
  fall = 0.0f;
}
//-------------------------------------------------------------------
//  Free all messages
//-------------------------------------------------------------------
CDependent::~CDependent (void)
{ std::vector<SMessage *>::iterator im;
	for (im = dpnd.begin(); im != dpnd.end(); im++)		delete  (*im);
	dpnd.clear();
  for (im = pxy0m.begin(); im != pxy0m.end(); im++) delete (*im);
	pxy0m.clear();
  for (im = pxy1m.begin(); im != pxy1m.end(); im++) delete (*im);
  pxy1m.clear();

}
//-------------------------------------------------------------------
//	JSDEV*  Trace activity for this dependent
//	To Trace enter a [TRACE] section and add
//	logTrace=1				to create a trace file
//	sub.xxxx=1				where xxxx is the unique Id of the subsystem
//-------------------------------------------------------------------
void CDependent::TraceActivity(U_INT FrNo,SMessage *msg,int act)
{	if (FrNo < globals->LoFrame)			return;
	if (FrNo > globals->HiFrame)			return;
	char sdpn[8];
	char styp[8];
	char smod[8];
	char stag[8];
	char sfrm[8];
	TagToString(sdpn,unId);
	TagToString(styp,type);
	TagToString(smod,mode);
	TagToString(stag,msg->user.u.datatag);
	TagToString(sfrm,msg->group);
	TRACE("Frame %08d: %s-%s-%2d (%s) Requests %s=%d from %s. ACTV= %d",
		FrNo,styp,sdpn,hwId,smod,stag,msg->intData,sfrm,act);
	return;
}
//-------------------------------------------------------------------
//	JSDEV* Read Dependency with optional ~ sign in front
//-------------------------------------------------------------------
void CDependent::ReadComponent(EMessageHWType Hw, SStream *stream)
{	  SMessage *msg = new SMessage;
	  int inv = ReadInvertedTag ((Tag*)(&msg->group), stream);
    msg->id = MSG_GETDATA;
    msg->sender         = unId;
    msg->user.u.datatag	= 'actv';								
    msg->user.u.hw			= Hw;
	  msg->user.u.invert	= inv;										
    dpnd.push_back (msg);
	  return;	}
//-------------------------------------------------------------------
//	JSDEV* Read CDependent Tag.  
//	the sign '~' in front of dependents means negate value
//-------------------------------------------------------------------
int CDependent::Read (SStream *stream, Tag tag)
{ int rc = TAG_IGNORED;
  SMessage *msg = 0;;
  switch (tag) {
  case 'eNum':
    // Dependent engine number
    ReadUInt (&eNum, stream);
    return TAG_READ;
  case 'volt':
    // Circuit voltage
    ReadFloat (&volt, stream);
    return TAG_READ;
  case 'mVlt':
    // Circuit maximum voltage
    ReadFloat (&mVlt, stream);
    return TAG_READ;
  case 'load':
    // Circuit load
    ReadFloat (&load, stream);
    return TAG_READ;
  case 'freQ':
  case 'FreQ':
    // AC current frequency
    ReadFloat (&freQ, stream);
    return TAG_READ;
    //--Default must be processed by individual dependents
  case 'dflt':
    return TAG_READ;
  case 'eBus':
  case 'ebus':
    // Logical connection to dependency with HW type BUS
	  ReadComponent (HW_BUS,stream);							// JSDEV*
		return TAG_READ;
  case 'fuse':
    // Logical connection to dependency with HW type FUSE
    ReadComponent (HW_FUSE,stream);							// JSDEV*
    return TAG_READ;
  case 'swch':
    // Logical connection to dependency with HW type SWITCH
    ReadComponent (HW_SWITCH,stream);						// JSDEV*
		return TAG_READ;
  case 'lite':
    // Logical connection to dependency with HW type LIGHT
    ReadComponent (HW_LIGHT,stream);						// JSDEV*
		return TAG_READ;
  case 'annr':
    // Logical connection to dependency with HW type ANNOUNCER
    ReadComponent (HW_ANNUNCIATOR,stream);					// JSDEV*
		return TAG_READ;
  case 'bool':
    // Logical connection to dependency with HW type STATE
    ReadComponent (HW_STATE,stream);						// JSDEV*
		return TAG_READ;
  case 'gear':
    //  Logical connection to gear controller -------------
    ReadComponent(HW_OTHER,stream);
    return TAG_READ;
  case 'cntr':
    // Logical connection to dependency with HW type CONTACTOR
    ReadComponent (HW_CONTACTOR,stream);					// JSDEV*
		return TAG_READ;
  case 'flsh':
    // Logical connection to dependency with HW type FLASHER
    ReadComponent (HW_FLASHER,stream);						// JSDEV*
		return TAG_READ;
  case 'batt':
    // Logical connection to dependency with HW type BATTERY
    ReadComponent (HW_BATTERY,stream);						// JSDEV*
	  return TAG_READ;
  case 'genr':
    // Logical connection to dependency with HW type GENERATOR
    ReadComponent (HW_GENERATOR,stream);					// JSDEV*
		return TAG_READ;
  case 'altr':
    // Logical connection to dependency with HW type ALTERNATOR
    ReadComponent (HW_ALTERNATOR,stream);					// JSDEV*
		return TAG_READ;
  case 'nvrt':
    // Logical connection to dependency with HW type INVERTER
    ReadComponent (HW_INVERTER,stream);						// JSDEV*
		return TAG_READ;
  case 'st8t':
    // Default state
    ReadInt (&st8t, stream);
    return TAG_READ;
  case 'stat':
    // Default activity state is ON
    stat = true;
    return TAG_READ;
  case '_AND':
    // Combine dependencies in AND function
    mode	= tag;
	  dflact	= true;									// JSDEV*
    return TAG_READ;
  case '_OR_':
    // Combine dependencies in an OR function
    mode	= tag;
    return TAG_READ;
	// JSDEV put proxy1 in list
  case 'pxy1':
    msg = new SMessage;
	  ReadMessage(msg, stream);
	  msg->id     = MSG_SETDATA;
    msg->sender = unId;
    pxy1m.push_back (msg);
	  return TAG_READ;
  case 'rise':
    // Rising edge proxy value
    ReadFloat (&rise, stream);
    return TAG_READ;
	// JSDEV* put proxy0 in list
  case 'pxy0':
    msg = new SMessage;
	  ReadMessage(msg, stream);
	  msg->id     = MSG_SETDATA;
    msg->sender = unId;
    pxy0m.push_back (msg);
	  return TAG_READ;
  case 'fall':
    // Falling edge proxy value
    ReadFloat (&fall, stream);
    return TAG_READ;
  case 'mPol':
    // Polling message: JS add to dependent
    ReadMessage (&mpol, stream);
    mpol.id       = MSG_GETDATA;
    mpol.dataType = TYPE_INT;
    return TAG_READ;
  case 'mAll':
    // Create two proxy messages
    { SMessage mall;
      ReadMessage (&mall, stream);
      // JS Modification
      msg = new SMessage;
      msg->group  = mall.group;
      msg->sender = unId;
	    msg->id = MSG_SETDATA;
      msg->user.u.datatag = mall.user.u.datatag;
      msg->dataType = TYPE_INT;
      msg->intData = 0;
      pxy0m.push_back (msg);
      // JS Modification
      msg = new SMessage;
      msg->group  = mall.group;
      msg->sender = unId;
	    msg->id = MSG_SETDATA;
      msg->user.u.datatag = mall.user.u.datatag;
      msg->dataType = TYPE_INT;
      msg->intData = 1;
      pxy1m.push_back (msg);
    }
    return TAG_READ;
  case 'tPol':
    // Polling method
    { char s[64];
      ReadString (s, 64, stream);
      WARNINGLOG ("CDependent::Read : Unknown polling method %s", s);
    }
    return TAG_READ;
  case 'offV':
    // Indication value when inactive
    ReadFloat (&offV, stream);
    return TAG_READ;
  case 'manl':
    return TAG_READ;                  // JS Ignore for now
  case 'zero':
    st8t  = 0;                        // JS Not sure what to do with that
    return TAG_READ;                  // LC let the derived class deal with it
  }

    return CSubsystem::Read (stream, tag);
}
//-------------------------------------------------------------------------------
//	JSDEV* Prepare all dependent messages
//-------------------------------------------------------------------------------
void CDependent::PrepareMsg(CVehicleObject *veh)	
{
  std::vector<SMessage *>::iterator im;
	for (im = dpnd.begin(); im != dpnd.end(); im++)
	{	SMessage *msg = (*im);
		veh->FindReceiver(msg);
	}
	// Prepare all pxy0 messages
	for (im = pxy0m.begin(); im != pxy0m.end(); im++)
	{	SMessage *msg = (*im);
		veh->FindReceiver(msg);
	}
	// Prepare all pxy1 messages
	for (im = pxy1m.begin(); im != pxy1m.end(); im++)
	{	SMessage *msg = (*im);
		veh->FindReceiver(msg);
	}
	return;	}
//-------------------------------------------------------------------------------
//  All parameters are read
//-------------------------------------------------------------------------------
void CDependent::ReadFinished (void)
{
  CSubsystem::ReadFinished ();

  // Determine initial state controller state for this dependent.
  // For most dependents, the state controller defaults to always ON such
  //   that the activity state is determined solely by dependencies.  The
  //   exception is if the hardware type is SWITCH.  These default to off,
  //   under the assumption that the associated gauge will set the correct
  //   state through messaging.
  int init_state = 1;
  if (hwId == HW_SWITCH)  init_state = 0;
  // Override initial state if the READ tag was specified
  if (st8t != -1)         init_state = st8t;
  // Assign initial state
  state = init_state;
  //-------------------------------------------------
  nDPND = dpnd.size();
  nPXY0 = pxy0m.size();
  nPXY1 = pxy1m.size();
}
//------------------------------------------------------------------------------
//  Return all dependents if any
//------------------------------------------------------------------------------
void CDependent::GetAllDependents(CFuiProbe *win)
{ std::vector<SMessage *>::iterator im;
  for (im = dpnd.begin(); im != dpnd.end(); im++)
  { SMessage *msg = *im;
    CDependent *dp = (CDependent *)msg->receiver;
    win->AddDependent(dp);
  }
  //---Check for poll message ----------------
  if (0 == mpol.group)        return;
  CDependent *dp  = (CDependent *)mpol.receiver;
  win->AddDependent(dp);
  return;
}
//------------------------------------------------------------------------------
//  Write data to Probe window
//------------------------------------------------------------------------------
void CDependent::Probe(CFuiCanva *cnv,Tag ids)
{ char edt[32];
  CSubsystem::Probe(cnv);
  _snprintf(edt,32,"active:%s",(active)?("true"):("false"));
  cnv->AddText( 1,edt,1);
  //----------------------------------
  _snprintf(edt,32,"state: %d",state);
  cnv->AddText( 1,edt,1);
  //-----------------------------------
  _snprintf(edt,32,"volts: %.2f",volt);
  cnv->AddText( 1,edt,1);
	//-----------------------------------
	TagToString(edt,mode);
	cnv->AddText( 1,1,"mode:%s",edt);
  //---Source -------------------------
  if (0 == ids)   return;
  _snprintf(edt,32,"Source:%s",TagToString(ids));
  cnv->AddText( 1,edt,1);
  return;
}
//------------------------------------------------------------------------------
void CDependent::Write (SStream *stream)
{
}
//-------------------------------------------------------------------------------
//  Set State
//-------------------------------------------------------------------------------
void CDependent::SetState(char s)
{ state		= s;
	if (state)	return;
	volt		= 0;
  return;
}
//-------------------------------------------------------------------------------
// CDependent supports the following messages:
//
// GETDATA:
//  stat - State (int)
//
// SETDATA:
//  stat - State (int)
//-------------------------------------------------------------------------------
EMessageResult CDependent::ReceiveMessage (SMessage *msg)
{ //---Next statement for debugging ----------------------------- 
  //-----The next statement is for debugin with the probe window ---
  //  When the subsystem match the selected one, a break point may
  //  be set to follow the update  process of this subsystem
  if (this == globals->psys)
          msg = msg;            // Put break point here
  EMessageResult rc = MSG_IGNORED;
  //---Process get data --------------------------
  if  (msg->id == MSG_GETDATA ) {
      switch (msg->user.u.datatag) {
        //----State value -------------------
        case 0:
        case 'st8t':
        case 'stat':
          msg->intData	= state;
		      return	MSG_PROCESSED;
        //----Activity and volt ------------
        case 'actv':
          msg->intData	= (int)active;
          msg->volts    = volt;
          return MSG_PROCESSED;
        //----Volt only -------------------
        case 'volt':
          msg->realData = volt;
          return MSG_PROCESSED;
			  //--- Both state ------------------
				case 'indx':
					msg->intData = state;
					msg->index   = indx;
					return MSG_PROCESSED;
      }
  }
    //---Process Set data ---------------------
  if (msg->id == MSG_SETDATA ) {
      switch (msg->user.u.datatag) {
        case 0:
        case 'st8t':
        case 'stat':
        case 'nabl':
				case 'auxi':
						SetState(msg->intData);
		        return MSG_PROCESSED;
				case 'indx':
					  SetState(msg->intData);
						indx	=  msg->index;
						return MSG_PROCESSED;
      }
  }

  // See if the message can be processed by the parent class
  return CSubsystem::ReceiveMessage (msg);
}
//----------------------------------------------------------------------------
//  Poll dependents with messages for AND mode
//    All dependents must be active
//----------------------------------------------------------------------------
void  CDependent::Poll_AND()
{	std::vector<SMessage *>::iterator im;
  active = 1;					// Initial value
	CSubsystem	*sub	= 0;
	bool p0		= 0;
	bool p1		= 0;
  volt      = 0;
	for (im = dpnd.begin(); im != dpnd.end(); im++) 
		{ SMessage *msg = (*im);
      Send_Message(msg);
		  p0  = (msg->intData != 0);
		  p1	= (msg->user.u.invert != 0);
		  p0 ^= p1;					// Invert if needed
      if (msg->volts > volt)  volt = msg->volts;
      if (p0)   continue;
      active  = 0;
      volt    = 0;
      return;
    }
  return;
}
//----------------------------------------------------------------------------
//  Poll dependents with messages for OR mode
//  At least one dependent must be active
//----------------------------------------------------------------------------
void  CDependent::Poll_OR_()
{	std::vector<SMessage *>::iterator im;
  active = 0;					// Initial value
	CSubsystem	*sub	= 0;
	bool p0		= 0;
	bool p1		= 0;
  volt      = 0;
	for (im = dpnd.begin(); im != dpnd.end(); im++) 
		{ SMessage *msg = (*im);
		  Send_Message(msg);
		  p0  = (msg->intData != 0);
		  p1	= (msg->user.u.invert != 0);
		  p0 ^= p1;					// Invert if needed
      if (!p0)   continue;
      if (msg->volts > volt)  volt = msg->volts;
      active  = 1;
    }
  return;
}
//----------------------------------------------------------------------------
//	JSDEV* CDependent update
//	-First update all dependents
//	-Then read all states
//  Note: The frame counter FrNo ensure 2 things
//	1	The subsystem is updated only once per cycle.  The function NeedUpdate(FrNo)
//		answer yes only once per frame cycle. It must be used before calling 
//		any  CDependent TimeSlice function.
//	2	Loop in dependency are stopped by this protection.  With the following chain
//		of dependencies A->B->C->A  then A will be detected as updated when C 
//		will call A.
//		
//----------------------------------------------------------------------------
void CDependent::UpdateState(float dT, U_INT FrNo)
{	std::vector<SMessage *>::iterator im;
  for (im = dpnd.begin(); im!=dpnd.end(); im++)
	{	SMessage *msg = (*im); 
    if (msg->receiver == 0)		    continue;
	  CDependent *dp = (CDependent *)msg->receiver;
		if (!dp->NeedUpdate(FrNo))		continue;
    dp->TimeSlice (dT,FrNo);
	}
	//----Poll all dependents to get aggregated state -----------
  if ('_AND' == mode) Poll_AND();
  else                Poll_OR_();
	//---- Update Electrical values -------------------------
	return;	
}
//---------------------------------------------------------------------
//	JSDEV* Send all Pxy0 messages
//---------------------------------------------------------------------
void CDependent::SendAllPxy0(void)
{	std::vector<SMessage *>::iterator im;
	CSubsystem *sub = 0;
	for (im = pxy0m.begin(); im != pxy0m.end(); im++)
	{	SMessage *msg = (*im);
    sub = (CSubsystem*)msg->receiver;
		Send_Message(msg);
	}
	return;	}
//---------------------------------------------------------------------
//	JSDEV* Send all Pxy1 messages
//---------------------------------------------------------------------
void CDependent::SendAllPxy1(void)
{	std::vector<SMessage *>::iterator im;
	CSubsystem *sub = 0;
	for (im = pxy1m.begin(); im != pxy1m.end(); im++)
	{	SMessage *msg = (*im);
    sub = (CSubsystem*)msg->receiver;
		Send_Message(msg);
	}
	return;	}

//---------------------------------------------------------------------
//	JSDEV* TimeSlice for CDependent
//  -Update the state from dependency
//  -Post any proxy according to activity change
//---------------------------------------------------------------------
void CDependent::TimeSlice(float dT, U_INT FrNo)
{	
  //-----The next statement is for debugin with the probe window ---
  //  When the subsystem match the selected one, a break point may
  //  be set to follow the update  process of this subsystem
  if (this == globals->psys)
          int a = 0;                // Put break point here
  //----Now update activity ------------------------------------
  bool old	= active;				        // Previous state
	active		= (state != 0);         // Assume activity from state
	if (active && (nDPND != 0))	UpdateState(dT,FrNo);
	//--- Call upper TimeSlice(...) ------------------------------
	CSubsystem::TimeSlice(dT,FrNo);
	//--- send proxy if activity is changed ---------
	if	(active == old)		return;
	//---- process pxy0 -----------------------------
	if ((active == false) && nPXY0)	return SendAllPxy0();
	if ((active == true)  && nPXY1)	return SendAllPxy1();
	return;
}

//--------------------------------------------------------------------------
void CDependent::Print (FILE *f)
{
  CSubsystem::Print (f);

  fprintf (f, "CDependent:\n");
  fprintf (f, "  Read tags:\n");
  fprintf (f, "    st8t=%d\tstat=%s\n", st8t, (stat ? "true" : "false"));
  fprintf (f, "    volt=%f\tload=%f\tfreq=%f\n", volt, load, freQ);
  fprintf (f, "    offV=%f\tmVlt=%f\n", offV, mVlt);
  fprintf (f, "  Realtime:\n");
  fprintf (f, "    active=%s\tstate=%d\n", (active ? "true" : "false"), state);
  fprintf (f, "  Dependencies (%d):\n", dpnd.size());
  /// \todo  Dump dependency mode
  /// \todo  Dump proxy messages
  /// \todo  Dump polling behaviour
}


//
// CAnnouncement
//

CAnnouncement::CAnnouncement (void)
{
  TypeIs (SUBSYSTEM_ANNOUNCEMENT);
}


//==========================================================================================
// CGenericMonitor
//==========================================================================================
CGenericMonitor::CGenericMonitor (void)
{
  TypeIs (SUBSYSTEM_GENERIC_MONITOR);
  oper  = 0;                              // Operator
  mode	= MONITOR_LT;
  comp	= 0;
  HiVal	= 0;
  LoVal	= 0;
  msg1	= 0;
  msg2	= 0;
  msg3	= 0;
  val1  = 0;
  val2  = 0;
  refv  = 0;
}
//--------------------------------------------------------------------
//	JSDEV* Implement CGenericMonitor
//	-Monitor the 2 values
//  -Set state according to defined comparison
//---------------------------------------------------------------------
CGenericMonitor::~CGenericMonitor (void)
{ if (msg1)	delete (msg1);
  if (msg2) delete (msg2);
  if (msg3) delete (msg3);
}
//---------------------------------------------------------------------
//	Store Message at the appropriate place
//
//---------------------------------------------------------------------
//	JSDEV* CGenericMonitor may have the following formats
//  mVal comp:		compare dynamic mVal1 to constant comp
//  mVal mVal:    compare both dynamic values
//	mVal loLm hiLm:  to	be used with either <XEED> or <BETW>
//	To complexify the matter all tags may appear in any order
//----------------------------------------------------------------------
int CGenericMonitor::Read (SStream *stream, Tag tag)
{ //--------------------------------------------------
  //  To observe a given genM
  //  -Uncomment the 2 following lines
  //  -Change system unId
  //  - Set a break point
  //----------------------------------------------------
  //if (unId == 'X1ON')           // Subsystem Ident
  //int a = 0;                    // Set a break point
  switch (tag) {

  case 'mVal':
    // Value message
    {
		SMessage *msg = new SMessage;
		ReadMessage (msg, stream);
    msg->sender   = unId;
		if      (msg1 == 0)	{	msg1	= msg; 	}
		else if (msg2 == 0)	{	msg2	= msg;	}
    else WARNINGLOG("genM %s too much <mval>",unId_string);
    }
		return TAG_READ;

  case '.LT.':
	  // Monitor for less than the comparison value
	  mode = MONITOR_LT;
    return TAG_READ;

  case '.GT.':
    // Monitor for greater than the comparison value
    mode = MONITOR_GT;
    return TAG_READ;

  case '.GE.':
    // Monitor for greater than or equal to the comparison value
    mode = MONITOR_GE;
    return TAG_READ;

  case '.LE.':
    // Monitor for less than or equal to the comparison value
    mode = MONITOR_LE;
    return TAG_READ;

  case '.EQ.':
	  // Monitor for equal
	  mode	= MONITOR_EQ;
	  return TAG_READ;

  case '.NE.':
	  // Monitor for not equal
	  mode	= MONITOR_NE;
	  return TAG_READ;

  case 'XEED':
    // Monitor for when value is outside of limits
    mode	= MONITOR_XEED;
    return TAG_READ;

  case 'BETW':
    // Monitor for when value is between limits
    mode	= MONITOR_BETW;
    return TAG_READ;

  case 'hiLm':
    // High limit
    ReadFloat (&HiVal, stream);	
    return TAG_READ;

  case 'loLm':
    // Low limit
    ReadFloat (&LoVal, stream);				
    return TAG_READ;

  case 'comp':
    // Comparison value
    ReadFloat (&comp, stream);
    refv = comp;
    return TAG_READ;

  case '----':
    // Substract values
    oper  = '-';
    return TAG_READ;

  case '++++':
    // Add values
    oper = '+';
    return TAG_READ;

  case '****':
    // multiply values
    oper = '*';
    return TAG_READ;

  }

    // See if the tag can be processed by the parent class type
  return CDependent::Read (stream, tag);
}

//---------------------------------------------------------------------------
// CGenericMonitor messages:
// JSDEV Todo get tag from specifications
// GETDATA:
//	actv - Monitor state
//  comp - Comparison value       REAL
//  mode - Comparison mode        INT
//  mVal - Poll message for comparison  VOID (SMessage*)
//
// SETDATA:
//  comp - Comparison value
//----------------------------------------------------------------------------
EMessageResult CGenericMonitor::ReceiveMessage (SMessage *msg)
{ //--------------------------------------------------
  //  To observe a given genI
  //  -Uncomment the 2 following lines
  //  -Change system unId
  //  - Set a break point
  //----------------------------------------------------
  //if (unId == 'X1ON')           // Subsystem Ident
  //int a = 0;                    // Set a break point
    switch (msg->id) {
		case MSG_GETDATA:
			switch (msg->user.u.datatag) {
				case 'actv':
					msg->intData	= compR;
					return MSG_PROCESSED;

				case 'comp':
					msg->realData	= comp;
					return MSG_PROCESSED;
			}
			break;
		case MSG_SETDATA:
			switch (msg->user.u.datatag) {
				case 'comp':
					if (msg->dataType == TYPE_REAL) {
						comp			= (float)msg->realData;
					  return MSG_PROCESSED;
					}
			break;
			}
		 break;

	}

      // See if the message can be processed by the parent class
  return CDependent::ReceiveMessage (msg);
}

//----------------------------------------------------------------------------
//	JSDEV* Prepare Monitoring messages
//----------------------------------------------------------------------------
void CGenericMonitor::PrepareMsg(CVehicleObject *veh)		
{	if (msg1) veh->FindReceiver(msg1);
	if (msg2) veh->FindReceiver(msg2);
	if (msg3)	veh->FindReceiver(msg3);
	//-----Call Other dependencies ----
	CDependent::PrepareMsg(veh);
	return;	}
//---------------------------------------------------------------------------
//	Extract float value from msg
//----------------------------------------------------------------------------
float CGenericMonitor::ExtractValue(SMessage *msg)
{	CSubsystem *sub = (CSubsystem *)msg->receiver;
	if (sub == NULL)	return 0;
	Send_Message(msg);
	return float(msg->realData);
}
//---------------------------------------------------------------------------
//  Extract receiver name
//---------------------------------------------------------------------------
char *CGenericMonitor::ExtractName(SMessage *msg)
{ if (0 == msg) return "";
  CSubsystem *sub = (CSubsystem *)msg->receiver;
  if (0 == sub) return "";
  return sub->GetIdString();
}
//---------------------------------------------------------------------------
//	JSDEV*   CGenericMonitor:  Apply operator on values 1 and 2
//  In this compr is the reference value or operation is
//  XEED or BETW
//---------------------------------------------------------------------------
float CGenericMonitor::CombineValues()
{ if ('+' == oper)  return (val1 + val2);
  if ('-' == oper)  return (val1 - val2);
  if ('*' == oper)  return (val1 * val2);
  if (msg2)         refv = val2;
  return val1;
}
//---------------------------------------------------------------------------
//	JSDEV*   CGenericMonitor
//	1-	Check if we are active from dependencies
//	2	Monitor values and set internal status true or false
//---------------------------------------------------------------------------
void CGenericMonitor::TimeSlice(float dT,U_INT FrNo)
{	//--------------------------------------------------
  //  To observe a given genI
  //  -Uncomment the 2 following lines
  //  -Change system unId
  //  - Set a break point
  //----------------------------------------------------
  // check if active from dependencies -----
  bool  old = compR;            // Old state
	compR	    = false;					  // Assume not active
	CDependent::TimeSlice(dT,FrNo);
	if (!active)		return;						// No need to monitor
	if (msg1) val1 = ExtractValue(msg1);
  if (msg2) val2 = ExtractValue(msg2);
  if (oper) val1 = CombineValues();
	switch (mode) {
		case MONITOR_LT:
			compR = (val1 <  refv);
			break;
		case MONITOR_GT:
			compR = (val1 >  refv);
			break;
		case MONITOR_LE:
			compR = (val1 <= refv);
			break;
		case MONITOR_EQ:
			compR = (val1 == refv);
			break;
		case MONITOR_NE:
			compR = (val1 != refv);
			break;
		case MONITOR_GE:
			compR = (val1 >= refv);
			break;
		case MONITOR_XEED:
			compR = ((val1 <  LoVal) || (val1 > HiVal));
			break;
		case MONITOR_BETW:
			compR = ((val1 >= LoVal) && (val1 <= HiVal));
			break;
	}
  //----Check for changing status ----------------
  if (old == compR)   return;
  if (compR == false) return SendAllPxy0();
  if (compR == true ) return SendAllPxy1();
	return;
}
//-------------------------------------------------------------------
//  Probe the Generic monitor
//-------------------------------------------------------------------
void CGenericMonitor::Probe(CFuiCanva *cnv)
{ CDependent::Probe(cnv,0);
  cnv->AddText(1,1,"Operator %c",oper);
  cnv->AddText(1,1,"Mode %s",GenrMOD[mode]);
  cnv->AddText(1,1,"V1 %.4f (%s)",val1,ExtractName(msg1));
  cnv->AddText(1,1,"V2 %.4f (%s)",val2,ExtractName(msg2));
  cnv->AddText(1,1,"RF %.4f",refv);
  char *rs = (compR)?("true"):("false");
  cnv->AddText(1,1,"Result: %s",rs);
  return;
}
//===========================================================
//	JSDEV* CPolynome
//===========================================================
CPolynome::CPolynome(void) 
{	indx	= 0;}
//----Read Coefficients --------------------------------
int	CPolynome::Read (SStream *stream, Tag tag)
{	char data[80];
	int rc = TAG_IGNORED;
	if (tag != 'coef')		
	{	WARNINGLOG ("%s : Unknown value <%s>", GetClassName(), data);
		return rc;	
	}
	ReadString(data,80,stream);
	char *delimiters = " \t";
    char *token = strtok (data, delimiters);
    while ((token != NULL) && (indx < 10))
	{   float nbr		= (float)atof (token);
		coef[indx++]	= nbr;
        token = strtok (NULL, delimiters);
    }
	return TAG_READ;	}
//-----------------------------------------------------------
//	CPolygone: compute output value
//-----------------------------------------------------------
float CPolynome::Compute(float vx)
{	if (indx == 0)	return vx;
	float val = coef[0];
	int	  inx = 0;
	float pwr = vx;
	for (inx = 1; inx != indx; inx++)
	{	val	+= coef[inx] * pwr;
		pwr *= vx;	
	}
	return val;	}

//-----------------------------------------------------------
// JSDEV* CGenericIndicator
//		CGenericIndicator allows for some math between
//		2 or more monitored values
//----------------------------------------------------------
CGenericIndicator::CGenericIndicator (void)
{ TypeIs (SUBSYSTEM_GENERIC_INDICATOR);
  alia = 0;
  poly = 0;
  mode = MODE_NONE;
  indnMode = INDN_LINEAR;
}
//--------------------------------------------------------------
CGenericIndicator::~CGenericIndicator (void)
{ for (U_INT k=0; k < mVal.size(); k++) delete mVal[k];
	mVal.clear();
  if (poly)	delete(poly);
}
//---------------------------------------------------------------
//	For each value to monitor create a poll message
//---------------------------------------------------------------
int CGenericIndicator::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'mVal':
    { // Value message
      SMessage* msg = new SMessage;
      ReadMessage (msg, stream);
      mVal.push_back (msg);
    }
    return  TAG_READ;
  case 'alia':
    // Alias value datatag
    ReadTag (&alia, stream);
    rc = TAG_READ;
    break;
  case '++++':
		// Indicator mode is to take sum of message values
		mode	= MODE_SUM;
		rc		= TAG_READ;
		break;
  case '----':
		// Indicator mode is to take difference of message values
		mode	= MODE_DIFFERENCE;
		rc =	 TAG_READ;
		break;
  case '****':
		// indicator mode is to multply message  values
		mode	= MODE_MULTIPLY;
		rc		= TAG_READ;
		break;
  case 'cvrt':
		poly	= new CPolynome();
		ReadFrom(poly,stream);
		rc		= TAG_READ;
		break;
  }

  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CDependent::Read (stream, tag);
  }

  return rc;
}
//----------------------------------------------------------------------------
void CGenericIndicator::ReadFinished (void)
{

  CDependent::ReadFinished ();
}
//----------------------------------------------------------------------------
//	JSDEV* Prepare Monitoring messages
//----------------------------------------------------------------------------
void CGenericIndicator::PrepareMsg(CVehicleObject *veh)		
{	SMessage *msg = NULL;
	std::vector<SMessage*>::iterator it;
	for (it = mVal.begin(); it != mVal.end(); it++)
	{	msg	= *it;
    msg->sender   = unId;
		veh->FindReceiver(msg);
	}
	//-----Call Other dependencies ----
	CDependent::PrepareMsg(veh);
	return;	}
//-----------------------------------------------------------------------------
//    Receive a message
//-----------------------------------------------------------------------------
EMessageResult CGenericIndicator::ReceiveMessage (SMessage *msg)
{
  EMessageResult rc = MSG_IGNORED;
    switch (msg->id)
    {
    case MSG_GETDATA:
      // Indication is tracked in Timeslice, we just need to return the values here
      if (msg->dataType == TYPE_REAL) msg->realData = indn;
      if (msg->dataType == TYPE_INT)  msg->intData  = int(indn);
			return MSG_PROCESSED;
    }
 return CDependent::ReceiveMessage (msg);
}
//---------------------------------------------------------------------------
//  Display dependent value on probe
//---------------------------------------------------------------------------
void CGenericIndicator::Probe(CFuiCanva *cnv)
{ char edt[32];
  CDependent::Probe(cnv,0);
  std::vector<SMessage*>::iterator itm = mVal.begin();
  int No = 1;
  _snprintf(edt,32,"Val = %.02f",indnTarget);
  cnv->AddText( 1,edt,1);
  while (itm != mVal.end())
  { SMessage	*msg = *itm++;
    _snprintf(edt,32,"value %u from",No++); 
    cnv->AddText( 1,edt);
    TagToString(edt,msg->group);
    cnv->AddText(11,edt,1);
  }

  return;
}
//---------------------------------------------------------------------------
//	JSDEV* CGenericIndicator:  
//	Aggregate various values according to the requested operation
//	and convert resulting value to polynomial if present
//	NOTES:  
//	1	Messages without a receiver (bad tag from AMP) are just
//		skipped and do not impact the result
//	2	Values are not monitored if  aggregated activity status of dependencies
//		is OFF
//---------------------------------------------------------------------------
void CGenericIndicator::TimeSlice(float dT,U_INT FrNo)
{	//--To test a given subsystem of this type: uncomment the 2 following lines
  //  set the subsystem identifier (unId) and set break point on second line-
  //if (unId == 'radf')                   // Set unId of observed subsystem
  //  int a = 0;                          // Put breakpoint here
  //---Update active status first --------------------------------
	CDependent::TimeSlice(dT,FrNo);
	indnTarget	= 0.0;
	if (!active)			return;				// No need to compute anything

  std::vector<SMessage*>::iterator itm = mVal.begin();
	SMessage	  *msg;
	CSubsystem	*sub;
	if  (itm == mVal.end())	return;
	msg			= *itm;
	sub			= (CSubsystem *)msg->receiver;
	if (sub)	sub->ReceiveMessage(msg);
	indnTarget	= float(msg->realData);
	itm++;										// Next msg for next value
	while (itm != mVal.end())
	{	msg		= *itm++;
		sub		= (CSubsystem *)msg->receiver;
		if (sub == NULL)	continue;
		sub->ReceiveMessage(msg);
		switch (mode)	{
			case MODE_SUM:
				indnTarget += (float)msg->realData;
				break;
			case MODE_DIFFERENCE:
				indnTarget -= (float)msg->realData;
				break;
			case MODE_MULTIPLY:
				indnTarget *= (float)msg->realData;
				break;
		}
	}
	//----Convert resulting value --------
	if (poly)	indnTarget = poly->Compute(indnTarget);
	return;	}


//
// CTimeDelay
//
CTimeDelay::CTimeDelay (void)
{
  TypeIs (SUBSYSTEM_TIME_DELAY);
}


//
// CKeyedSystem
//
CKeyedSystem::CKeyedSystem (void)
{
  TypeIs (SUBSYSTEM_KEYED_SYSTEM);
}

//=====================================================================
//  Contactor edit table
//=====================================================================
char *cntrTAB[] = {
    "NONE",           // State 0
    "CLOSED",         //       1
    "OPENING",        //       2
    "OPEN",           //       3
    "CLOSING",        //       4
};
//-----Last event -------------------------------------------------
//=====================================================================
// CContactor
//   monitor 3 subsystems ans set state according to their
//	active state
//	When monitored circuit transits from Off to On, the contactor
//		reacts accordingly
//	When monitorer circuit transits from On to Off, the contactor
//		remains Latched in current state
//=====================================================================
CContactor::CContactor (void)
{ TypeIs (SUBSYSTEM_CONTACTOR);
  hwId  = HW_CONTACTOR;
	Etat	= OPEN;				// Initial state
	Event	= 0;					// No Event
	timer	= 0;
	dlyC	= 0;					// No delay to close
	dlyO	= 0;					// No delay to open
	Rstate	= 0;
	Cstate	= 0;
	Ostate	= 0;
}
//---------------------------------------------------------------------
//	Prepare all 3 messages
//	We are just interested by the activity status of monitored
//	dependents
//---------------------------------------------------------------------
void CContactor::PrepareMsg(CVehicleObject *veh)
{	Reset();
  //--------------------------------------------
  mReset.user.u.datatag = 'actv';
  mReset.sender = unId;
	if (mReset.group)	veh->FindReceiver(&mReset);
	mClose.user.u.datatag = 'actv';
  mClose.sender = unId;
	if (mClose.group)	veh->FindReceiver(&mClose);
	mOpen.user.u.datatag	= 'actv';
  mOpen.sender  = unId;
	if (mOpen.group)	veh->FindReceiver(&mOpen);
	return;
}


//---------------------------------------------------------------------
int CContactor::Read (SStream *stream, Tag tag)
{
 switch (tag) {
  case 'rset':
		// Contactor reset message
		ReadMessage (&mReset, stream);
		return TAG_READ;
  case 'clos':
		// Contactor closed message
		ReadMessage (&mClose, stream);
		return  TAG_READ;
  case 'open':
		// Contactor open message
		ReadMessage (&mOpen, stream);
		return  TAG_READ;
  case 'dlyC':
		// Delay to close
	    ReadFloat (&dlyC, stream);
		return  TAG_READ;
  case 'dlyO':
		// delay to open
		ReadFloat(&dlyO, stream);
		return  TAG_READ;
  }
    return  CDependent::Read (stream, tag);
}
//---------------------------------------------------------------------
//	Monitor active transition for this circuit
//---------------------------------------------------------------------
bool	CContactor::NewTransition(SMessage *msg, int *actual)
{	if (msg->receiver == NULL)	return false;
	int old	= *actual;								// Current status
	CSubsystem *sub = (CSubsystem *)msg->receiver;
	sub->ReceiveMessage(msg);
	*actual	= msg->intData;							// New status
	return (*actual != old);
}
//---------------------------------------------------------------------
//  Reset contactor
//---------------------------------------------------------------------
void CContactor::Reset()
{ timer	  = 0;						// Stop everything
	state	  = 0;						// Closed
	Etat	  = OPEN;				  // Internal state open
  Ostate  = 0;
  Cstate  = 0;
  Rstate  = 0;
  return;
}
//---------------------------------------------------------------------
//	Detect event in all circuits
//	NOTE: In case of simultaneous events, a priority is applied and
//	only one event is taken in account
//	upper:	Reset
//	middle: Close
//	Lower:	Open
//---------------------------------------------------------------------
int	CContactor::GetEvent(void)
{	int evn = 0;
	if (NewTransition(&mOpen ,&Ostate) && Ostate == 1) evn = EOPEN;
	if (NewTransition(&mClose,&Cstate) && Cstate == 1) evn = CLOSE;
	if (NewTransition(&mReset,&Rstate) && Rstate == 1) evn = RESET;
	if (evn != RESET)		return evn;
	//-----Reset the contactor here ---------------------
  Reset();
	return 0;	
}	
//-----------------------------------------------------------------------
//	Contactor is closed:  Process only OPEN event
//-----------------------------------------------------------------------
void	CContactor::ClosedState(int event)
{	if (event != EOPEN)		return;
  Etat  = OPEN;
 	state	= 0;					  // Operational
  if (0 == dlyO)        return;
	timer	= dlyO;							  // arm delay to open
	Etat	= OPENING;						// New internal state
	return;	
}
//-----------------------------------------------------------------------
//	Contactor is openening
//-----------------------------------------------------------------------
void	CContactor::OpeningState(float dT)
{	timer	-= dT;
	if (timer > 0)			return;
	timer	= 0;
	Etat	= OPEN;					// New internal state
	return;	
}
//-----------------------------------------------------------------------
//	Contactor is open
//-----------------------------------------------------------------------
void	CContactor::OpenState(int event)
{	if (event != CLOSE)		return;
  Etat  = CLOSED;
 	state	= 1;					  // Operational
  if (0 == dlyC)        return;
	timer	= dlyC;					  // arm delay to close
	Etat	= CLOSING;				// New internal state
	return;
}
//-----------------------------------------------------------------------
//	Contactor is closing
//-----------------------------------------------------------------------
void	CContactor::ClosingState(float dT)
{	timer	-= dT;
	if (timer > 0)			return;
	timer	= 0;
	Etat	= CLOSED;		   // New internal state
	return;	
}
//-----------------------------------------------------------------------
//	CContactor TimeSLice
//	Update the contactor status
//	NOTES: 
//	1	During transition from close to open and open to close
//		all changes in monitored circuits are ignored
//	2	Not sure what to do with the dependencies. With this implementation
//		the contactor may have state on (Open) and active false (due to
//		dependencies.
//-----------------------------------------------------------------------
void CContactor::TimeSlice(float dT,U_INT FrNo)
{	
	Event	= GetEvent();
	if (Event)	{
		switch (Etat)	{
			case CLOSED:
				ClosedState(Event);
				break;
			case OPENING:
				OpeningState(dT);
				break;
			case OPEN:
				OpenState(Event);
				break;
			case CLOSING:
				ClosingState(dT);
				break;
		}
	}
	//---- Now update activity from dependencies --------------------
	CDependent::TimeSlice(dT,FrNo);
	return;	
}
//-----------------------------------------------------------------------
//  Probe Contactor
//-----------------------------------------------------------------------
void CContactor::Probe(CFuiCanva *cnv)
{ char edt[32];
  CDependent::Probe(cnv,0);
  _snprintf(edt,32,"State %s",cntrTAB[Etat]);
  cnv->AddText( 1,edt,1);
  _snprintf(edt,32,"Open by: %s",TagToString(mOpen.group));
  cnv->AddText( 1,edt,1);
  _snprintf(edt,32,"Close by: %s",TagToString(mClose.group));
  cnv->AddText( 1,edt,1);
  _snprintf(edt,32,"Reset by: %s",TagToString(mReset.group));
  cnv->AddText( 1,edt,1);
  return;
}
//=============================================================================
// CFlasher toggle state according to time parameter
//=============================================================================
CFlasher::CFlasher (void)
{ TypeIs (SUBSYSTEM_FLASHER);
  hwId  = HW_FLASHER;
  time	= 0;
  delta	= 0;
  blnk	= false;
}
//----------------------------------------------------------------------
//	Read all tags
//----------------------------------------------------------------------
int CFlasher::Read (SStream *stream, Tag tag)
{
  switch (tag) {
    case 'time':
      // Flasher cycle time
      ReadFloat (&time, stream);
      return TAG_READ;
    }

  return CDependent::Read (stream, tag);
}
//-----------------------------------------------------------------------
//	JSDEV* Compute delta time and change state accordingly
//-----------------------------------------------------------------------
void CFlasher::TimeSlice(float dT,U_INT FrNo)
{	//-----Check activity -------------------------------------	
	CDependent::TimeSlice(dT,FrNo);
	if (!active)			return;
	delta += dT;
	//-------  change activity according to delta  value -----
	if (delta >= time)
	{	delta   = 0;						// Reset delay
		blnk   ^= true;				  // Change state
	}
	return;	
}
//-----------------------------------------------------------------------
//  Return blink state
//-----------------------------------------------------------------------
void CFlasher::Probe(CFuiCanva *cnv)
{ char edt[16];
  CDependent::Probe(cnv,0);
  _snprintf(edt,16,"blnk %d",blnk);
  cnv->AddText( 1,edt,1);
  return;
}
//-----------------------------------------------------------------------
//	JSDEV* Return the blink state
//-----------------------------------------------------------------------
EMessageResult CFlasher::ReceiveMessage (SMessage *msg)
{
   switch (msg->id)
	  { 
      case MSG_GETDATA:
        switch (msg->user.u.datatag) {
		      case 'actv':
			      msg->intData = (blnk & active);
			      return MSG_PROCESSED;
	        }
	  }
      // See if the message can be processed by the parent class
  return CDependent::ReceiveMessage (msg);
}

//==========================================================================
//  Circular timer used for animating bitmap
//==========================================================================
CCircularTimer::CCircularTimer()
{ TypeIs (SUBSYSTEM_CIRCULAR_TIMER);
  hwId    = HW_OTHER;
  Time    = 0;
  modulo  = 0;
  beat    = 0;
}
//--------------------------------------------------------------
//  Read parameters
//---------------------------------------------------------------
int CCircularTimer::Read (SStream *stream, Tag tag)
{int mod = 0;
 switch (tag) {
  case 'baT1':
		// Modulo limit
		ReadInt (&mod, stream);
    modulo = (U_INT)(mod+1);
		return TAG_READ;
  case 'baT2':
		// Beat per second
		ReadInt (&mod, stream);
    beat   = (mod <= 0)?(10):(mod);
    maxT   = float(1) / beat;
    beat   = 0;
		return  TAG_READ;
  }
  return  CDependent::Read (stream, tag);
}
//---------------------------------------------------------------
//  Timeslice
//---------------------------------------------------------------
void CCircularTimer::TimeSlice(float dT,U_INT FrNo)
{ Time += dT;
  if (Time < maxT)  return;
  Time -= dT;
  beat = (beat + 1) % modulo;
}
//---------------------------------------------------------------
//  Probe this subsystem
//---------------------------------------------------------------
void CCircularTimer::Probe(CFuiCanva *cnv)
{ char edt[64];
  CSubsystem::Probe(cnv);
  _snprintf(edt,64,"beat : %02d",beat);
  cnv->AddText(1,edt,1);
  return;
}
//---------------------------------------------------------------
//  Receive message
//---------------------------------------------------------------
EMessageResult CCircularTimer::ReceiveMessage (SMessage *msg)
{ if (msg->id == MSG_GETDATA) {

      switch (msg->user.u.datatag) {
		    case 'indn':
			    msg->realData   = float(beat);
			    return MSG_PROCESSED;
      }
  }
      // See if the message can be processed by the parent class
  return CDependent::ReceiveMessage (msg);
}


//=========================================================================
//  Global Clock subsystem. Maintains local time during simulation
//=========================================================================
CClock::CClock()
{ unId  = 'zzck';
  Set();
}
//--------------------------------------------------------------------
//  Reset the clock
//--------------------------------------------------------------------
void CClock::Set()
{ dta = 0;
  loc = globals->tim->GetLocalDateTime();
  hh  = loc.time.hour;
  mm  = loc.time.minute;
  ss  = loc.time.second;
  dh  = (hh % 12) * 30;
  dm  = mm *  6;
  ds  = ss *  6;
  ch  = 1;
  on  = 0;
  ts  = 0;
  return;
}
//--------------------------------------------------------------------
//  Update the time
//  Maintains HH MM SS since start and DH DM DS in day
//--------------------------------------------------------------------
void CClock::Update(float dT)
{ U_SHORT om = mm;
  dta += dT;
  if (dta < 0.25) return;
  on   = 1;
  if (dta < 1)    return;
  on   = 0;
  while (dta > 1)    {dta -= 1; ss += 1; ts++;}
  while (ss >= 60)   {ss -= 60; mm += 1;}
  while (mm >= 60)   {mm -= 60; hh += 1;}
  while (hh >= 24)   {hh -= 24;}
  dh  = ((hh % 12) * 30) + (mm >> 1);
  dm  = mm *  6;
  ds  = ss *  6;
  ch  = (om == mm)?(0):(1);
}
//-------------------------------------------------------------------
//  Probe the clock
//-------------------------------------------------------------------
void  CClock::Probe(CFuiCanva *cnv)
{ char edt[64];
  CSubsystem::Probe(cnv);
  _snprintf(edt,64,"hour: %02d",hh);
  cnv->AddText(1,edt,1);
  _snprintf(edt,64,"min : %02d",mm);
  cnv->AddText(1,edt,1);
  _snprintf(edt,64,"sec : %02d",ss);
  cnv->AddText(1,edt,1);
  return;
}

//==================END OF FILE ============================================