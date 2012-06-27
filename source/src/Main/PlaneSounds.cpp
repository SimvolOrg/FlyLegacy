/*
 * Plane Sounds.cpp
 *
 * Part of Fly! Legacy project
 * Copyright 2007 Jean Sabatier
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
//==========================================================================================
#include "../Include/Globals.h"
#include "../Include/FlyLegacy.h"
#include "../Include/AudioManager.h"
#include "../Include/WorldObjects.h"
//==========================================================================================
//  Aircraft sound management 
//==========================================================================================
//===========================================================================
// CVehicleSound
//===========================================================================
CVehicleSound::CVehicleSound (char *sfxFilename)
{ sample  = 0;
  NbFlap  = 0;
  NbTire  = 0;
  cach    = false;
  volu    = 1;
  pMax    = 1.5f;
  pMin    = 0.8f;
  //---------------------------------------
  SStream s(this,"WORLD",sfxFilename);
}
//-------------------------------------------------------------------------
//  Destroy this object
//-------------------------------------------------------------------------
CVehicleSound::~CVehicleSound (void)
{ // Clean up engine sound sub-object instances
  std::vector<CEngineSound*>::iterator i;
  for (i=engineSounds.begin(); i!=engineSounds.end(); i++) delete (*i); 
  //---Release Engine sounds buffers ---------------------------
  CAudioManager *snd = globals->snd;
  snd->ReleaseSoundBUF('crkE');           // External engine start
  snd->ReleaseSoundBUF('catE');           // Engine Catching
	snd->ReleaseSoundBUF('runE');						// External running sound
	snd->ReleaseSoundBUF('misE');						// External running sound
	snd->ReleaseSoundBUF('hltE');						// External short stop
  snd->ReleaseSoundBUF('falE');           // External engine Fail
  snd->ReleaseSoundBUF('prsi');           // External propeller start
  snd->ReleaseSoundBUF('pris');           // External propeller stop
  snd->ReleaseSoundBUF('wind');           // External wind
  snd->ReleaseSoundBUF('stal');           // External stall horn
  snd->ReleaseSoundBUF('grnd');           // External ground noise
  snd->ReleaseSoundBUF('outr');           // External outter marker
  snd->ReleaseSoundBUF('midl');           // External middle marker
  snd->ReleaseSoundBUF('innr');           // External inner marker
  snd->ReleaseSoundBUF('gyrl');           // External gyro low
  snd->ReleaseSoundBUF('gyrs');           // External gyro stable
  snd->ReleaseSoundBUF('elts');           // External warning 
  snd->ReleaseSoundBUF('crgr');           // Gear crash
  snd->ReleaseSoundBUF('crby');           // Body crash
  snd->ReleaseSoundBUF('shok');           // Shock sound
  //---Release Flap Sounds -------------------------------------
  for (int k = 1; k <= NbFlap; k++)
  { Tag ids = 'Flp0' + k;
    snd->ReleaseSoundBUF(ids);
  }
 //---Release Gear Sound -------------------------------------
  snd->ReleaseSoundBUF('geru');
  snd->ReleaseSoundBUF('gerd');
  //---Release Tire Sounds -------------------------------------
  for (int k = 0; k <  NbTire; k++)
  { Tag ids = 'tir0' + k;
    snd->ReleaseSoundBUF(ids);
  }
}
//--------------------------------------------------------------------------------
//  SetSound with parameters
//--------------------------------------------------------------------------------
void CVehicleSound::SetSound(Tag ids,char *fn)
{ CAudioManager *snd = globals->snd;
  CSoundBUF *sbf = snd->ReserveSoundBUF(ids,fn,sample);
  sample  = 0;
  sbf->SetGain(volu);
  volu    = 1;
  return;
}
//--------------------------------------------------------------------------------
//  SetSound with parameters
//--------------------------------------------------------------------------------
void CVehicleSound::SetEngineSound(Tag ids,char *fn)
{ CAudioManager *snd = globals->snd;
  CSoundBUF *sbf = snd->ReserveSoundBUF(ids,fn,sample);
  sbf->SetPMAX(pMax);
  sbf->SetPMIN(pMin);
  return;
}
//--------------------------------------------------------------------------------
//  Read Crash Sound
//--------------------------------------------------------------------------------
void CVehicleSound::ReadCrashSounds(SStream *st)
{ CAudioManager *snd = globals->snd;
  char txt[128];
  char fn[64];
  char  go = 1;
  Tag   tg = 0;
  while (go)
  { ReadString(txt,128,st);
    if (1 == sscanf(txt," gear , %s",fn))
    { snd->ReserveSoundBUF('crgr',fn,0) ;
      continue;
    }
    if (1 == sscanf(txt," body , %s",fn))
    { snd->ReserveSoundBUF('crby',fn,0);
      continue;
    }
    if (1 == sscanf(txt," shok , %s",fn))
    { snd->ReserveSoundBUF('shok',fn,0);
      continue;
    }

    go  = 0;
  }
  return;
}
//--------------------------------------------------------------------------------
//  JS NOTE:  Add sound as buffer to audiao manager
//--------------------------------------------------------------------------------
int CVehicleSound::Read (SStream *stream, Tag tag)
{ CAudioManager *snd = globals->snd;
  int rc = TAG_IGNORED;
  SMessage fmsg;
  char fint[64];
  char fext[64];
  switch (tag) {
  case 'cach':
    //---  pre-cache sfx files- ---------------------
    cach = true;;
    return TAG_READ;
    //---  Engine sounds ----------------------------
  case 'engn':
    { CEngineSound *engn = new CEngineSound;
      ReadFrom (engn, stream);
      engineSounds.push_back (engn);
    }
    return TAG_READ;
    //--- Crash sounds ------------------------------
  case 'crsh':
    ReadCrashSounds(stream);
    return TAG_READ;
  //--- Volume for next sound -----------------------
  case 'volu':
    ReadFloat(&volu,stream);
    return TAG_READ;
  //--- Loop back sample for next engine sound  -----
  case 'back':
    ReadInt(&sample,stream);
    return TAG_READ;
  //--- Cranking sound ----------------------------
	case 'crkE':
    // Engine Crank 
		ReadString (fint, sizeof (fint) - 1, stream);
    ReadString (fext, sizeof (fext) - 1, stream);
    SetEngineSound('crkE',fext);
	  sample  = 0;
    return TAG_READ;
	//--- Catching sound ----------------------------
  case 'catE':
    // internal engine catching 
		ReadString (fint, sizeof (fint) - 1, stream);
    ReadString (fext, sizeof (fext) - 1, stream);
		SetEngineSound('catE',fext);
	  sample  = 0;
    return TAG_READ;
	//--- Running sound -----------------------------
	case 'runE':
    // internal engine runing 
		ReadString (fint, sizeof (fint) - 1, stream);
    ReadString (fext, sizeof (fext) - 1, stream);
		SetEngineSound('runE',fext);
	  sample  = 0;
    return TAG_READ;
	//--- short stop sound --------------------------
  case 'hltE':
    // internal engine idle-stop
		ReadString (fint, sizeof (fint) - 1, stream);
    ReadString (fext, sizeof (fext) - 1, stream);
    SetEngineSound('hltE',fext);
	  sample  = 0;
    return TAG_READ;
	//--- Failing at start up -----------------------
  case 'falE':
    // internal engine stop 
		ReadString (fint, sizeof (fint) - 1, stream);
    ReadString (fext, sizeof (fext) - 1, stream);
    SetEngineSound('falE',fext);
	  sample  = 0;
    return TAG_READ;
  //--- Miss fire sound ---------------------------
	case 'misE':
    // internal engine miss fire
		ReadString (fint, sizeof (fint) - 1, stream);
    ReadString (fext, sizeof (fext) - 1, stream);
		SetEngineSound('misE',fext);
	  sample  = 0;
    return TAG_READ;

  case 'prsi':
    // Propeller start->idle
    ReadString (fint, sizeof (fint) - 1, stream);
    ReadString (fext, sizeof (fext) - 1, stream);
    SetSound('prsi',fext);
    return TAG_READ;
  case 'pris':
    // Propeller idle->stop
    ReadString (fint, sizeof (fint) - 1, stream);
    ReadString (fext, sizeof (fext) - 1, stream);
    SetSound('pris',fext);
    return TAG_READ;

    //--- Wind------------------------------------ 
  case 'wind':
    // internal wind 
    ReadString (fint, sizeof (fint) - 1, stream);
    ReadString (fext, sizeof (fext) - 1, stream);
    SetSound('wind',fext);
    return TAG_READ;
    //--- Tires------------------------------------ 
  case 'tire':
    ReadTireSound(stream);
    return TAG_READ;
    //--- Stall horn------------------------------- 
  case 'stal':
    // internal stall horn 
    ReadString (fint, sizeof (fint) - 1, stream);
    ReadString (fext, sizeof (fext) - 1, stream);
    SetSound('stal',fext);
    return TAG_READ;
    //--- FLAPS ------------------------------- 
  case 'flap':
    ReadFlapSound(stream);
    return TAG_READ;
  case 'fmsg':
    // flaps message 
    ReadMessage (&fmsg, stream);
    rc = TAG_READ;
    break;
    //--- Ground------------------------------- 
  case 'grnd':
    ReadString (fint, sizeof (fint) - 1, stream);
    ReadString (fext, sizeof (fext) - 1, stream);
    SetSound('grnd',fext);
    return TAG_READ;
    //--- Gear UP------------------------------- 
  case 'geru':
    ReadString (fint, sizeof (fint) - 1, stream);
    ReadString (fext, sizeof (fext) - 1, stream);
    SetSound('geru',fext);
    return TAG_READ;
  case 'gerd':
    //--- Gear Down --------------------------------- 
    ReadString (fint, sizeof (fint) - 1, stream);
    ReadString (fext, sizeof (fext) - 1, stream);
    SetSound('gerd',fext);
    return TAG_READ;
    //----Outter marker-------------------------------
  case 'outr':
    // outer marker 
    ReadString (fext, sizeof (fext) - 1, stream);
    SetSound('outr',fext);
    return TAG_READ;
    //----middle marker-------------------------------
  case 'midl':
    // middle marker 
    ReadString (fext, sizeof (fext) - 1, stream);
    SetSound('midl',fext);
    return TAG_READ;
    //--- Inner marker ------------------------------- 
  case 'innr':
    ReadString (fext, sizeof (fext) - 1, stream);
    SetSound('innr',fext);
    return TAG_READ;

  case 'gyro':
    //--- gyro sounds ---------------------------------
    ReadString (fext, sizeof (fext) - 1, stream);
    SetSound('gyrl',fext);
    ReadString (fext, sizeof (fext) - 1, stream);
    SetSound('gyrs',fext);
    return TAG_READ;
  case 'gmsg':
    //---- gyro message ------------------------------
    ReadMessage (&fmsg, stream);
    return TAG_READ;
  case 'elt_':
    // elt warning sound 
    ReadString (fext, sizeof (fext) - 1, stream);
    SetSound('elts',fext);
    //snd->ReserveSoundBUF('elts',fext,sample); // lc 052310 -
    return TAG_READ;
  case 'bend':
    // 
    ReadFloat (&pMin, stream);
    ReadFloat (&pMax, stream);
    return TAG_READ;
  }

  if (rc != TAG_READ) {
    // Tag was not processed by this object, it is unrecognized
    WARNINGLOG ("CVehicleSound::Read : Unrecognized tag <%s>", TagToString(tag));
  }

  return rc;
}
//-----------------------------------------------------------------------
//  Read Flap sounds
//-----------------------------------------------------------------------
void CVehicleSound::ReadFlapSound(SStream *s)
{ int nbre = 0;
  Tag name = 'Flp0';
  ReadInt (&nbre, s);
  NbFlap   = nbre;
  if (NbFlap <= 0)  return;
  char fint[64] = {""};
  char fext[64] = {""};
  for (int k = 1; k <=  NbFlap; k++) 
  { ReadString (fint, sizeof (fint) - 1, s);
    ReadString (fext, sizeof (fext) - 1, s);
    name = 'Flp0' + k;
    if (k < ENGINE_MAX_SOUND)
    { CSoundBUF *sbf = globals->snd->ReserveSoundBUF(name,fext);
      sbf->SetGain(volu);
    }
  }
  volu = 1;
  return;
}
//-----------------------------------------------------------------------
//  Read Tire sounds
//-----------------------------------------------------------------------
void CVehicleSound::ReadTireSound(SStream *s)
{ int nbre = 0;
  Tag name = 'tir0';
  ReadInt (&nbre, s);
  NbTire   = nbre;
  if (NbTire <= 0)  return;
  char fint[64] = {""};
  char fext[64] = {""};
  for (int k = 0; k < NbTire; k++) 
  { ReadString (fint, sizeof (fint) - 1, s);
    ReadString (fext, sizeof (fext) - 1, s);
    name = 'tir0' + k;
    if (k < ENGINE_MAX_SOUND)
    { CSoundBUF *sbf = globals->snd->ReserveSoundBUF(name,fext);
      sbf->SetGain(volu);
    }
  }
  volu = 1;
  return;
}
//-----------------------------------------------------------------------
//  All parameters are read.  create all sound objects
//-----------------------------------------------------------------------
void CVehicleSound::ReadFinished (void)
{}
//===================================================================
// CEngineSound
//===================================================================
CEngineSound::CEngineSound (void)
{
  engineNumber = 0;
  bendMin = bendMax = 0;
  freqTolerance = 0;
}
//-----------------------------------------------------------------------
//  Read sound parameters
//-----------------------------------------------------------------------
int CEngineSound::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;
  char s[80];

  switch (tag) {
  case 'enum':
  case 'eNum':
    ReadInt (&engineNumber, stream);
    rc = TAG_READ;
    break;
  case 'strt':
    ReadString (s, 80, stream);
    startInt = s;
    ReadString (s, 80, stream);
    startExt = s;
    rc = TAG_READ;
    break;
  case 'idle':
    ReadString (s, 80, stream);
    idleInt = s;
    ReadString (s, 80, stream);
    idleExt = s;
    rc = TAG_READ;
    break;
  case 'fly_':
    ReadString (s, 80, stream);
    flyInt = s;
    ReadString (s, 80, stream);
    flyExt = s;
    rc = TAG_READ;
    break;
  case 'stop':
    ReadString (s, 80, stream);
    stopInt = s;
    ReadString (s, 80, stream);
    stopExt = s;
    rc = TAG_READ;
    break;
  case 'rmpu':
    ReadString (s, 80, stream);
    rampUpInt = s;
    ReadString (s, 80, stream);
    rampUpExt = s;
    rc = TAG_READ;
    break;
  case 'rmpd':
    ReadString (s, 80, stream);
    rampDownInt = s;
    ReadString (s, 80, stream);
    rampDownExt = s;
    rc = TAG_READ;
    break;
  case 'bend':
    ReadFloat (&bendMin, stream);
    ReadFloat (&bendMax, stream);
    rc = TAG_READ;
    break;
  case 'frqt':
    ReadFloat (&freqTolerance, stream);
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // Tag was not processed by this object, it is unrecognized
    WARNINGLOG ("CEngineSound::Read : Unrecognized tag <%s>", TagToString(tag));
  }

  return rc;
}

//==========================END OF FILE ====================================================