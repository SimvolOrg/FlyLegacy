/*
 * Joysticks.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005 Chris Wallace
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

#ifndef JOYSTICKS_H
#define JOYSTICKS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Include/FlyLegacy.h"
#include <SDL.h>
#include <list>
#include <bitset>
//=============================================================================
class CKeyDefinition;
class SJoyDEF;
class CSimAxe;
//=====================================================================================
//  CONTROL CONNECTOR MASK
//	Used for locking associated axis when autopilot is ON
//=====================================================================================
enum JoyConnector {
		JS_AILR_BIT	= 0x0001,								// Ailerons
		JS_ELVR_BIT = 0x0002,								// Elevator
		JS_RUDR_BIT = 0x0004,								// Rudder
		JS_AILT_BIT = 0x0008,								// Aileron trim
		JS_ELVT_BIT	= 0x0010,								// Elevator trim	
		JS_RUDT_BIT = 0x0020,								// Rudder trim
		JS_THRO_BIT = 0x0040,								// THROTTLE lever
		JS_OTHR_BIT = 0x8000,								// Other axis
		//-----------------------------------------------------------
		JS_SURF_APL = (JS_AILR_BIT + JS_ELVR_BIT + JS_AILT_BIT + JS_ELVT_BIT + JS_THRO_BIT),
		JS_SURF_ALL = (JS_AILR_BIT + JS_ELVR_BIT + JS_RUDR_BIT),
		JS_TRIM_ALL = (JS_AILT_BIT + JS_ELVT_BIT + JS_RUDT_BIT),
};
//=====================================================================================
//  Neutral definition
//=====================================================================================
typedef struct {  
          float lo;                       // Lower limit
          float hi;                       // Higher limit
          float md;                       // Middle value
          float ap;                       // Amplitude
} JOY_NULL_AREA;

//============================================================================
//  Class to describe a button
//============================================================================
class CSimButton: public CStreamObject
{
public:
  SJoyDEF *      pjoy;          // Joystick descriptor
  int            joyn;          // Joystick numer
  Tag            kset;          // Ket set
  Tag            cmde;          // Key command
  int            nBut;          // Button number
  U_INT          Stat;          // ON-OFF state
  CKeyDefinition *kdf;          // Associated key
  //----Public methods ---------------------------------
public:
  CSimButton();
 ~CSimButton();
  int   Read(SStream * stream, Tag tag);
  bool  Tr01(U_INT st);
  //---Inline -----------------------------------------
  inline void LinkTo(CKeyDefinition *d) {kdf = d;}
  inline CKeyDefinition *GetKey() {return kdf;}
};
//=============================================================================
//  Structure to describe a Joystick
//=============================================================================
class SJoyDEF
{ 
public:
  short           njs;				// Internal number
  short           nax;				// Number of axes
	//--------------------------------------------------------------
	char						nbt;				// Number of buttons
	char						nht;				// Number of hat
	char						uht;				// Use this hat
	char						rfu;
	//--------------------------------------------------------------
  float         * axeData;		// array of float per axes
	float				  * axePrev;		// previous value
  CSimButton    * mBut[32];		// Array of pointer to button
  U_INT			      but;				// One bit per button
	U_INT						hat;				// Hat position
	SDL_Joystick  *	spj;				// SDL joystick
	char          * dName;			// Device name 
  //--------------------------------------------------------------
public:
  SJoyDEF();
 ~SJoyDEF();
	void	UpdateSDL();
	bool	CreateSDL(int k);
	U_INT HasMoved(CSimAxe *axe);
  //-------------------------------------------------------------------------
	inline char   *getDevName()     {return dName;}
	inline void	PushVal()	{int dim = nax * sizeof(float); memcpy(axePrev,axeData,dim); }
  //-------------------------------------------------------------------------
	inline char 				jNumber()													{return njs;}
	inline char					hNumber()													{return nht;}
	inline char					HatUsed()													{return uht;}
  inline void         StoreVector(int n,CSimButton *b)  {n &= 31; mBut[n] = b;}
  inline CSimButton  *GetButton(int n)                  {n &= 31; return mBut[n];}
  inline int          JoystickNo()                      {return njs;}
	inline void					SetHat(char n)										{uht = n;}
};
//===========================================================================
//  Class for Axe description
//===========================================================================
class CSimAxe: public CStreamObject
{
public:
  U_CHAR      No;       // Entry Index
  U_CHAR      type;     // 0 plane 1 heli
  U_CHAR      end;      // Last entry
  U_CHAR      group;    // Group name
	//---------------------------------------------------------
	U_INT	      msk;			// Connecting mask
	//---------------------------------------------------------
	char			  idn[8];		// Tag name for debug
	//---------------------------------------------------------
  char       *name;     // axe name to display in user window
  Tag         gen;      // Generic name
  Tag         cmd;      // Datatag
  SJoyDEF *  pJoy;			// device pointer
	short       joyn;			// Joystick number
  short       iAxe;     // device related axe number
  float       inv;			// invert value sign
  bool        pos;      // Positive only
  U_CHAR      neutral;  // Neutral control type
  SMessage    msg;      // Send message control
  float       attn;     // Attenuation 
	//-----------------------------------------------------------
  static float aCOEF[];
  static float bCOEF[];
  //--------------------------------------------------------------
public:
  CSimAxe();
  int   Read (SStream * st, Tag tag);
  void  Copy (CSimAxe *from);
  bool  NotAs(CSimAxe *axe);
	float	Value (JOY_NULL_AREA *n);
	float RawVal(JOY_NULL_AREA *n);
	void	Assign(CSimAxe *axn);
	void  Assignment(char *edt,int s);
	//--------------------------------------------------------------
	bool	IsConnected(U_INT m)
	{	U_INT sel = m & msk;	return ((sel) && (pJoy != 0));  }
	//--------------------------------------------------------------
	inline void				 SetATTN(float v)	{attn = v;}
	inline float			 GetATTN()			{return attn;}
  //--------------------------------------------------------------
	inline void				 Invert()				{inv = -inv;}
	inline char       *GetDevice()		{return (pJoy)?(pJoy->dName):("");}
  inline void        Clear()        {pJoy = 0; inv = +1;}
  inline char       *GetName()      {return name;}
  inline bool        Positive()     {return pos;}
  inline bool        IsUnassigned() {return (0 == pJoy);}
  inline int         AxeNo()        {return iAxe;}
  inline int         GetInvert()    {return (inv==1)?(0):(1);}
};

//=============================================================================
typedef enum
{ JS_AILERON   = 'ailr',
  JS_ELEVATOR  = 'elev',
  JS_RUDDER    = 'rudr',
  JS_TRIM      = 'trim',
  JS_RITE_TOE	 = 'rtoe',
  JS_LEFT_TOE  = 'ltoe',
  JS_THROTTLE_0= 'thr0',  // Not a real AXE
  JS_THROTTLE_1= 'thr1',
  JS_THROTTLE_2= 'thr2',
  JS_THROTTLE_3= 'thr3',
  JS_THROTTLE_4= 'thr4',
  JS_MIXTURE_0 = 'mix0',  // Not a real axe
  JS_MIXTURE_1 = 'mix1',
  JS_MIXTURE_2 = 'mix2',
  JS_MIXTURE_3 = 'mix3',
  JS_MIXTURE_4 = 'mix4',
  JS_PROP_0    = 'pro0',  // Not a real axe
  JS_PROP_1    = 'pro1',
  JS_PROP_2    = 'pro2',
  JS_PROP_3    = 'pro3',
  JS_PROP_4    = 'pro4',
  JS_ROLL_CYCLIC   = 'rCyc',
  JS_PITCH_CYCLIC  = 'pCyc',
  JS_TAIL_ROTOR    = 'tRot',
  JS_PITCHTRIM     = 'pTrm',
  JS_COLLECTIVE    = 'Coli',
  JS_THROTTLE      = 'Thro',
  JS_STEER         = 'ster',
  JS_GAS           = 'gas_'

} EAllAxes;
//=====================================================================================
typedef void(AxeDetectCB(CSimAxe *, Tag));
typedef void(ButtonDetectCB(SJoyDEF *jsd, int iButton, Tag id));
//=====================================================================================
#define JOY_AXIS_NUMBER   30
#define JOY_TYPE_PLAN   0         // Plane axis
#define JOY_TYPE_HELI   1         // Heli axis
#define JOY_TYPE_GVEH   2         // Ground vehicle axis
#define JOY_TYPE_PMT    3         // Group Prop-Mix-Gas
//-------------------------------------------------------------
#define JOY_FIRST_PLAN   0        // entry for First plane axe in list
#define JOY_FIRST_HELI  18        // entry for First heli  axe in list
#define JOY_FIRST_GVEH  24        // entry for first vehicle axe in list
#define JOY_FIRST_PMT    6        // Group controle for planes firts entry
//----------------------------------------------------------------------
#define JOY_GROUP_TOES   4        // Index of first TOE axis 
#define JOY_GROUP_THRO   6        // Index of first THRO axis
#define JOY_GROUP_MIXT  10        // Index of first mixture axis
#define JOY_GROUP_PROP  14        // Index of first Prop (blad) axis 
//----------------------------------------------------------------------
#define JOY_GROUP_PMT   0x10
#define JOY_THROTTLE    0x01 + JOY_GROUP_PMT
#define JOY_MIXTURE     0x02 + JOY_GROUP_PMT
#define JOY_PROPEL      0x04 + JOY_GROUP_PMT
//----NEUTRAL CONTROL TYPE -----------------------------------
#define JOY_NEUTRAL_STICK 1
//=====================================================================================
//  JOYSTICK MANAGER
//=====================================================================================
class CJoysticksManager : public CStreamObject
{	//---Initial vector --------------------------------------------------
	typedef void (CJoysticksManager::*VINIT)(SJoyDEF *jp);	// Rending vector
	//--------------------------------------------------------------------
private:
  CJoysticksManager();

  static CJoysticksManager instance;

public:
  static CJoysticksManager& Instance() { return instance; }

  ~CJoysticksManager();
	void						PreInit();
  void            Init( );
	void						CreateDevList(char **men,int n);
	//--------------------------------------------------------------------
	bool						IsBusy();
	void						SetFree();
  //------------------------------------------------------------------------
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
	//--------------------------------------------------------------------
  void            InitAxe (int nx,U_CHAR t,U_INT msk,char *nm,Tag c,int gp, bool p,float inv = +1);
  void            SetMessage(int m,Tag des,int unit,Tag cmd);
  void            EndMark(int nx);
  void            NeutralMark(int nx,U_CHAR type);
	//--------------------------------------------------------------------
  void            Update();
	void						DetectMove(SJoyDEF * p);
	void            CheckControl(Tag tag);
	//--------------------------------------------------------------------
	void						ConnectAll();
	void						Disconnect(U_INT m);
	void						Reconnect(U_INT m);
	//--------------------------------------------------------------------
  float           Neutral(float f, int nx);
  float           AxeVal(CSimAxe *pa);
	float           RawVal(CSimAxe *pa);
  int             HowMany() const {return joyQ.size();};
  bool            StartDetectMoves(AxeDetectCB * pFunc, Tag id);
  void            StopDetectMoves(void);
	//----Read values ------------------------------------------------------
	void						Poll(EAllAxes a,float &v);
	//----In line to start button detection --------------------------------
	bool						StartDetectButton(ButtonDetectCB *pcb, Tag id);
  void            StopDetectButton(void);
	//----------------------------------------------------------------------
  SJoyDEF        *Find(char * name,int jn = 0);
  SJoyDEF        *GetJoystickNo(int No);
  //------Remap a message to given tag -------------------------------------
  void            MapTo(Tag gen, Tag usr);
  Tag             TagFrom(Tag gen);
  //------------------------------------------------------------------------
  inline  void    ClearAxe(int nx) {AxesList[nx].pJoy = 0;}
  //------------------------------------------------------------------------
  CSimAxe *Axe(Tag tag)  {return GetAxe(tag);}
  CSimAxe *NextAxe(CSimAxe *from,int type);
  //--- READ CONFIG FILE ---------------------------------------------------
	void						ProcessHat(SStream *stream);
  bool            ProcessButton(CSimButton *sbt);
  void            RemoveButton(SJoyDEF *jsd,int bt);
  CSimButton     *AddButton(SJoyDEF *jsd,int nb,CKeyDefinition *kdf);
  bool            AssignCallBack(SJoyDEF * pJoy,int k);
	//------------------------------------------------------------------------
  void            AssignAxe(CSimAxe *axe, CSimAxe *axn, U_CHAR all);
	void						Clear(CSimAxe *axn,U_CHAR all);
	void						UseHat(SJoyDEF *jsp,char s);
  //------------------------------------------------------------------------
  bool            ProcessAxe(CSimAxe *from);
  CSimAxe        *GetAxe(Tag tag);
  bool            HasAxe(EAllAxes axe);
  void            Invert(CSimAxe *axn,U_CHAR all);
  void            ClearAxe(EAllAxes tag);
  void            ReleaseAxe(CSimAxe *axn);
  void            SetInvert(EAllAxes tag,int p);
  int             GetInvert(EAllAxes tag);
  void            SendGroupPMT(U_CHAR nbu);
  void            SendGroup(U_INT gr,Tag cmd,U_CHAR nbu);
  void            ClearGroupPMT();
  //------------------------------------------------------------------------
  void            SaveButtonConfig(SStream *st,SJoyDEF *jsd);
  void            SaveAxisConfig(SStream *st,SJoyDEF *jsd);
  void            SaveConfiguration();
  void            SetNulleArea(float n,char m); 
  float           GetAttenuation(EAllAxes cmd);
  void            SetAttenuation(EAllAxes cmd,float atn);
  //------------------------------------------------------------------------
  inline float    GetNulleArea()				{return nValue;}
	inline void  		Modifier()						{modify = 1;}
	inline char			GasDisconnected()		  {return (axeCNX & JS_THRO_BIT)?(0):(1);}
  //------------------------------------------------------------------------
protected:
	void					EnumSDL();
  void          FreeJoyList();
  void          HandleButton(SJoyDEF * pJoy);
	bool					HandleHat(U_INT hat);
  //----ATTRIBUTES ----------------------------------------------------------
private:
	//-------------------------------------------------------------------------
	U_INT		axeCNX;					// Connected axis
	//-------------------------------------------------------------------------
	char    modify;
  char		busy;		// Library used 0 = PU 1 = SDL
	char		use;		// Joystick use
	char		rfu1;		//reserved
	//--- HAT control -------------------------------------
	char    uht;										// Hat used
	char		jsh;										// Hat joystick
	//--- list of components -------------------------------
	std::vector<SJoyDEF *>   joyQ;
  std::map<Tag,CSimAxe *>   mapAxe;
  CSimAxe AxesList[JOY_AXIS_NUMBER];
  CSimAxe AxeMoved;
	//------------------------------------------------------
  Tag     wmID;											// CFuiWindow ID for move callback
  Tag     wbID;											// CFuiWindow ID for button detect callback
  AxeDetectCB    *pAxeCB;						// function callback for axe move detection
  ButtonDetectCB *pButCB;						// function callback for button detection
	//-----Control commande -------------------------------
	Tag			cmde;										// Current command
	int 		Time;										// Hat timer
  //-----Static tables ----------------------------------
  float   nValue;                 // Neutral value [0,1]
	//-----------------------------------------------------
  static JOY_NULL_AREA nZON[];
	//-----------------------------------------------------
};

#endif JOYSTICKS_H
//=============================END OF FILE ==========================================

