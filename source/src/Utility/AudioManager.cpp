/*
 * AudioManager.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2004 Chris Wallace
 * Copyright 2007 Jean Sabatier
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
 */

/*! \file AudioManager.cpp
 *  \brief Implements CAudioManager class for application control of all sounds
 */

/*
 * Audio Manager
 *
 * Discrete sound sources must be provided for:
 *   - 2 x COM radios
 *   - 2 x NAV radios
 *   - 2 x XPDR radios
 *   - panels (one source or more?)
 *   - individual engines
 *   - individual hydraulic systems (e.g. flaps, landing gear)
 *   - DLLs
 *
 * Since the audio manager creates the OpenAL context, there can only be
 *   one instance of the CAudioManager class.  The audio manager should
 *   therefore also manage OpenAL sources.  
 *  JS Modification:  Positional Sound objects are created for evry part tha
 *                    can emit a sound
 */

#include "../Include/AudioManager.h"
#include "../Include/globals.h"
using namespace std;
//=====================================================================================
//  JS: CSoundOBJ: 
//  This class represent a sound source like an engine or a gear or any other
//  part of the aricraft that may emit sounds.
//  A SoundOBJ has a fixed position relative to the aircraft origin.  The listener
//  is alway set at camera position so that sound will play differently relative
//  to the listener position
//  All soundOBJ are collected into a pool located in the veh object
//=====================================================================================
CSoundOBJ::CSoundOBJ(Tag t, SVector &pos)
{ Init(t,pos);
}
//---------------------------------------------------------------------
//  CSound object for panel and subsystem
//---------------------------------------------------------------------
CSoundOBJ::CSoundOBJ()
{ CVector pos(0,0,0);
  Tag ids = 'spnl';
  Init(ids,pos);
}
//---------------------------------------------------------------------
//  Init sound object
//---------------------------------------------------------------------
void  CSoundOBJ::Init(Tag t, SVector &pos)
{ CAudioManager *snd = globals->snd;
  sEnd    = 0;
  idn     = t;
  sPlay   = 0;
  gain    = 1;
  pitch   = 1;
  loop    = false;
  offset  = 0;
  for (int k=0; k<8; k++) sounds[k] = 0;
  alGenSources (1, &alSRC);
  snd->Check();
  snd->IncSRC();
  alSource3f(alSRC,AL_POSITION,pos.x,pos.y,pos.z);
  alSourcef (alSRC,AL_REFERENCE_DISTANCE,8.0f);
  snd->Check();
	for (int k=0; k<ENGINE_MAX_SOUND; k++) sounds[k] = 0;
}
//---------------------------------------------------------------------
//  destroy this object
//---------------------------------------------------------------------
CSoundOBJ::~CSoundOBJ()
{ CAudioManager *snd = globals->snd;
  snd->DeleteSource(alSRC);
  for (int k=0; k < ENGINE_MAX_SOUND; k++)
  { CSoundBUF *sbf = sounds[k];
    if (0 == sbf)   continue;
    sbf->Release();
  }
}
//---------------------------------------------------------------------
//  Add a sound by tag to the list
//---------------------------------------------------------------------
void CSoundOBJ::AddSound(int No,Tag idn)
{ CAudioManager *snd = globals->snd;
  CSoundBUF *sbf = snd->GetSoundBUF(idn);
  if (sounds[No])   gtfo("Sound conflict");
  sounds[No]     = sbf;
	if (0 == sbf)		 return;
  if (No > sEnd)   sEnd = No;
  return;
}
//---------------------------------------------------------------------
//  Add a sound by position to the list
//---------------------------------------------------------------------
void CSoundOBJ::SetSBUF(char p, CSoundBUF *sbf)
{ sounds[p] = sbf;
  if (p > sEnd) sEnd = p;
  return;
}
//---------------------------------------------------------------------
//  Fill object with last sound buffer (Flap only)
//---------------------------------------------------------------------
void CSoundOBJ::FillFlaps()
{ int   No = sEnd;
  int   Nx = No + 1;
  CSoundBUF *sbf = sounds[No];        // Last sound
  if (0 == sbf)       return;
  for (int k = Nx; k < (ENGINE_MAX_SOUND); k++)
  { sbf->IncUser();
    sounds[k] = sbf; 
    sEnd    = k;
  }
 return; 
}
//---------------------------------------------------------------------
//  Add a sound  to the list
//---------------------------------------------------------------------
void CSoundOBJ::AddSound(int No,CSoundBUF *sbf)
{ if (0 == sbf)     return;
  if (sounds[No])   gtfo("Sound conflict");
  sounds[No]     = sbf;
  if (No > sEnd)   sEnd = No;
  return;
}
//---------------------------------------------------------------------
//  Stop playing the sound
//---------------------------------------------------------------------
void CSoundOBJ::StopSound()
{ if (0 == sPlay)   return;
  alSourceStop(alSRC);
  alSourcei (alSRC, AL_BUFFER, 0);
  sPlay = 0;
  loop  = 0;
  return;
}
//---------------------------------------------------------------------
//  Play the requested sound in the indicated mode
//  NOTE: Same sound request does nothing.  Use changeMode to 
//        change playing conditions
//  TODO:  Check if same sound is playing 
//---------------------------------------------------------------------
void  CSoundOBJ::Play(int No)
{ CAudioManager *snd = globals->snd;
  ALint state;
  alGetSourcei (alSRC, AL_SOURCE_STATE, &state);
  if ((sPlay == sounds[No]) && (state == AL_PLAYING)) return;
  //---Stop current sound if any ----------------
  StopSound();
  //-- Play the current sound -------------------
  sPlay = sounds[No];
  if (0 == sPlay)     return;
  offset    = sPlay->GetOffset();
  ALint buf = sPlay->GetBuffer();
  alSourcei(alSRC,AL_BUFFER,buf);
  alSourcei(alSRC,AL_LOOPING,false);
  alSourcef(alSRC,AL_GAIN,gain);
  alSourcef(alSRC,AL_PITCH,pitch);
  alSourcei(alSRC,AL_SAMPLE_OFFSET,offset);
  alSourcePlay(alSRC);
  snd->Check();
}
//---------------------------------------------------------------------
//  Check if sound is playing
//---------------------------------------------------------------------
bool CSoundOBJ::IsPlaying()
{ ALint state;
  alGetSourcei (alSRC, AL_SOURCE_STATE, &state);
  return (state == AL_PLAYING);
}
//---------------------------------------------------------------------
//  Return playing position
//---------------------------------------------------------------------
bool CSoundOBJ::BeforeOffset()
{ ALint state;
  alGetSourcei (alSRC, AL_SOURCE_STATE, &state);
  if (state != AL_PLAYING)  return false;
  if (0 == sPlay)           return false;
  int  pos   = 0;
  alGetSourcei(alSRC,AL_SAMPLE_OFFSET,&pos);
  return (pos < offset);
}
//---------------------------------------------------------------------
//  Loop current sound from offset position
//---------------------------------------------------------------------
void CSoundOBJ::PlayFromOffset(int No)
{ CAudioManager *snd = globals->snd;
  ALint state;
  alGetSourcei (alSRC, AL_SOURCE_STATE, &state);
  if ((sPlay == sounds[No]) && (state == AL_PLAYING)) return;
  sPlay     = sounds[No];
  if (0 == sPlay)       return;
  offset    = sPlay->GetOffset();
  alSourcei(alSRC,AL_BUFFER, sPlay->GetBuffer());
  alSourcei(alSRC,AL_SAMPLE_OFFSET,offset);
  alSourcei(alSRC,AL_LOOPING,false);
  alSourcef(alSRC,AL_PITCH,pitch);
  alSourcePlay(alSRC);
  snd->Check();
  return;
}
//---------------------------------------------------------------------
//  Set sound pitch
//---------------------------------------------------------------------
void CSoundOBJ::SetEnginePitch(float p)
{ if (0 == sPlay)     return;
  ALint state;
  alGetSourcei (alSRC, AL_SOURCE_STATE, &state);
  if (AL_PLAYING != state)    return;
  float ph = sPlay->GetPitch(p);
  alSourcef(alSRC,AL_PITCH,ph);
  return;
}
//=====================================================================================
//  JS: Sound source:  Non localized source used for panel and subsystems
//=====================================================================================
CSoundSRC::CSoundSRC()
{ sPlay = 0;
  alGenSources (1, &alSRC);
  globals->snd->Check();
}
//---------------------------------------------------------------------
//  Play a sound
//---------------------------------------------------------------------
void  CSoundSRC::Play(CSoundBUF *sbf)
{ CAudioManager *snd = globals->snd;
  //---Stop current sound if any ----------------
  alSourceStop(alSRC);
  alSourcei (alSRC, AL_BUFFER, 0);
  sPlay = sbf;
  //-- Play the current sound -------------------
  if (0 == sbf)     return;
  ALint buf = sPlay->GetBuffer();
  alSourcei(alSRC,AL_BUFFER,buf);
  alSourcei(alSRC,AL_LOOPING,loop);
  alSourcePlay(alSRC);
  snd->Check();
  return;
}
//=====================================================================================
//  JS: Sound Buffer:  An AL buffer dedicated to one sound
//  SoundBUF are managed by CAudioManager in a buffer cache.  Do not delete them.
//=====================================================================================
CSoundBUF::CSoundBUF(Tag t, char *nf)
{ CAudioManager *snd = globals->snd;
  user    = 1;
  idn     = t;
  offset  = 0;
  nfile   = 0;
  alSRC   = 0;
  TagToString(itag,t);
  ALuint  buf;
  alGenBuffers(1,&buf);
  alBUF   = buf;
  snd->Check();
  snd->LoadFile(this,nf);
  snd->IncBUF();
  gain    = 1;
  pMin    = 1;
  pMax    = 1;
}
//---------------------------------------------------------------------
//  Free this buffer
//  NOTE :  deleting data cause a crash.  probably deleted by the
//          deleteBuffer
//----------------------------------------------------------------------
CSoundBUF::~CSoundBUF()
{ if (nfile)  delete [] nfile;
  CAudioManager *snd = globals->snd;
  snd->DeleteBuffer(alBUF);
}
//---------------------------------------------------------------------
//  Allocate a name
//----------------------------------------------------------------------
void CSoundBUF::SetName(char *name)
{ int lgr = strlen(name) + 1;
  nfile   = new char [lgr];
  strncpy(nfile,name,lgr);
  return;
}

//---------------------------------------------------------------------
//  Decrement user
//----------------------------------------------------------------------
bool CSoundBUF::DecUser()
{ user--;
  return (user)?(true):(false);
}
//---------------------------------------------------------------------
//  Release the buffer according to the type
//  filename=> Named cache
//  else    => Tag Cache
//----------------------------------------------------------------------
void CSoundBUF::Release()
{ CAudioManager *snd = globals->snd;
  if (nfile)  snd->ReleaseSoundBUF(this);
  else        snd->ReleaseSoundBUF(idn);
  return;
}
//---------------------------------------------------------------------
//  Change pitch
//---------------------------------------------------------------------
void CSoundBUF::SetPitch(float v)
{ ALint state;
  alGetSourcei (alSRC, AL_SOURCE_STATE, &state);
  if (AL_PLAYING != state)    return;
  float dt = pMax - pMin;
  float ph = pMin + (dt * v);
  alSourcef(alSRC,AL_PITCH,ph);
  return;
}
//---------------------------------------------------------------------
//  Play this sound
//---------------------------------------------------------------------
void CSoundBUF::PlayS1()
{ if (alSRC)          return;
  alGenSources (1, &alSRC);
  if (0 == alSRC)     return;
  alSourcei(alSRC,AL_BUFFER,alBUF);
  alSourcei(alSRC,AL_LOOPING,1);
  alSourcePlay(alSRC);
  return;
}
//---------------------------------------------------------------------
//  Update sound according to value b
//---------------------------------------------------------------------
void CSoundBUF::Play(char b)
{ if (b)            return PlayS1();
  //--- Stop this sound and free source -------
  U_INT er = alGetError();
  if (0 == alSRC)   return;
  alSourceStop(alSRC);
  alSourcei (alSRC, AL_BUFFER, 0);
  alDeleteSources(1,&alSRC);
  alSRC = 0;
  return;
}
//========================================================================================
// Constructor of Sound Manager
//========================================================================================
CAudioManager::CAudioManager (void)
{ No      = 0;
  nbSRC   = 0;
  nbBUF   = 0;
  context = NULL;
  device  = NULL;
  source  = 0;
  nSRC    = 0;
  xSRC    = 0;
  Init();
  AllocateSources(16);
}
//-------------------------------------------------------------------
// Destroy it 
//-------------------------------------------------------------------
CAudioManager::~CAudioManager()
{ Cleanup();
  if (source) delete [] source;
	std::map<Tag,CSoundBUF*>::iterator ra;
	for (ra = buffers.begin(); ra != buffers.end(); ra++) delete ra->second;
	buffers.clear();
	snames.clear();
}
//-------------------------------------------------------------------
// CleanUp everything
//-------------------------------------------------------------------
void CAudioManager::Cleanup (void)
{
  //------------------------------------------------
  // put back to an unitialized state
  // Clean up AL context and device
  //------------------------------------------------
  context = alcGetCurrentContext ();
  device  = alcGetContextsDevice (context);
  alcMakeContextCurrent (NULL);
  alcDestroyContext (context);
  alcCloseDevice (device);
  int er = alutGetError ();
  if (!alutExit ()) {
    ALenum error = alutGetError ();
    WARNINGLOG ("CAudioManager::Cleanup : Error 0x%08X", error);
  }
  return;
}
//--------------------------------------------------------------------------------------
//		Warning
//--------------------------------------------------------------------------------------
void CAudioManager::Warn(char * msg,ALint error=0)
{	WARNINGLOG (msg, error);
return;
}
//--------------------------------------------------------------------------------------
// Initialization
// 
// This method must be called immediately after instantiation of the audio
// manager class
//--------------------------------------------------------------------------------------
void CAudioManager::Init (void)
{ ALint error;
  // put alut in an Initialized state
  alutGetError ();
  if (!alutInitWithoutContext (NULL, NULL)) {
    ALenum error = alutGetError();
    return Warn ("CAudioManager::Init : Init error 0x%08X", error);
  }

  // Open default AL device
  device = alcOpenDevice (NULL);
  if (device == NULL) return Warn ("CAudioManager: No default audio device");

  // Create context
  context = alcCreateContext (device, NULL);
  if (context == NULL) return Warn ("CAudioManager: Could not create context");

  // Set context as the active one
  alcGetError (device);
  alcMakeContextCurrent (context);
  if (alcGetError (device) != ALC_NO_ERROR) return Warn("CAudioManager: Could not make context current");

  // Clear error code
  alGetError ();
  alcGetError (device);

  // Listener position is always set to the origin of 3D audio space.
  // All sound effects must be position relative to the origin
  ALfloat listenerPos[] = {0.0, 0.0, 0.0};
  alListenerfv (AL_POSITION, listenerPos);
  if ((error = alGetError ()) != AL_NO_ERROR) return Warn("CAudioManager:  Could not set listener position");

  // Default listener velocity is zero.
  ALfloat listenerVel[] = {0.0, 0.0, 0.0};
  alListenerfv (AL_VELOCITY, listenerVel);

  if ((error = alGetError ()) != AL_NO_ERROR) return Warn("CAudioManager:  Could not set listener velocity");

  // Listener orientation is facing "into" the screen, towards +z axis
  // with +y axis as the up vector
  ALfloat listenerOrient[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0}; // Fwd, up
  alListenerfv (AL_ORIENTATION, listenerOrient);

  if ((error = alGetError ()) != AL_NO_ERROR) return Warn("CAudioManager:  Could not set listener orientation");
  alGetError();
}
//-------------------------------------------------------------------
// Init sources
//-------------------------------------------------------------------
void CAudioManager::AllocateSources(char n)
{ nSRC  = n;
  //MEMORY_LEAK_MARKER ("CSoundSRC");
  source = new CSoundSRC[n];
  //MEMORY_LEAK_MARKER ("CSoundSRC");
  return;
}
//============================================================================
// SetListenerPosition
//
// This method sets the listener's position in Fly! geodetic coordinates.
// For the purposes of distance attenuation, the listener position is always
// assumed to be at the "origin" of the 3D world.  Setting the listener
// position is only required if SetSfxPosition () method is used to set
// sound effect positions using geodetic coordinates.  If there are no
// position-dependent sound effects to be played (not likely) or if the
// sound effect positions will always be specified using the
// SetSfxPosition (SVector) method, then the application does not need to
// continuously update the listener world position by calling this function.
//============================================================================
//------------------------------------------------------------
// Check any error
//------------------------------------------------------------
void CAudioManager::Check()
{ ALenum error = alGetError();
  if (error != AL_NO_ERROR)  {
  return Warn("CAudioManager::Error 0x%08X", error);
   }
}
//------------------------------------------------------------
// Check ALUT error
//------------------------------------------------------------
void CAudioManager::CheckALUT()
{ ALenum error;
  if ((error = alutGetError()) != ALUT_ERROR_NO_ERROR) {
      return Warn("CAudioManager::CreateSfx : Load WAV error 0x%08X", error);
    }
}
//---------------------------------------------------------------------
//  Check for playing sound
//---------------------------------------------------------------------
bool CAudioManager::IsPlaying(U_INT src)
{ if (0 == src) return false;
  ALint state;
  alGetSourcei (src, AL_SOURCE_STATE, &state);
  return (state == AL_PLAYING);
}
//------------------------------------------------------------
// Play a buffer into a source
//------------------------------------------------------------
U_INT CAudioManager::Play(CSoundBUF *sbf)
{ if (0 == sbf)   return 0;
  CSoundSRC *src = source + xSRC++;
  if (xSRC == nSRC) xSRC = 0;
  src->Loop(0);
  src->Play(sbf);
  return src->GetSource();
}
//------------------------------------------------------------
// Play a buffer into a source
//------------------------------------------------------------
void  CAudioManager::Loop(CSoundBUF *sbf)
{ if (0 == sbf)   return;
  CSoundSRC *src = source + xSRC++;
  if (xSRC == nSRC) xSRC = 0;
  src->Loop(1);
  src->Play(sbf);
  return;
}
//------------------------------------------------------------
// Create a sound source
//------------------------------------------------------------
U_INT CAudioManager::CreateSource (void)
{ ALuint source;
  alGenSources (1, &source);
  Check();
  return (U_INT)source;
}
//------------------------------------------------------------
// Destroy a sound source
//------------------------------------------------------------
void CAudioManager::DeleteSource(U_INT src)
{ ALuint source = src;
  alSourceStop (source);              // Stop source
  alSourcei (source, AL_BUFFER, 0);   // Detach any buffer
  alDeleteSources(1,&source);
  Check();
}
//------------------------------------------------------------
// Set SourcePosition
//------------------------------------------------------------
void CAudioManager::SetSourcePos(U_INT src,SVector &pos)
{ ALuint source = src;
  alSource3f(source,AL_POSITION,pos.x,pos.y,pos.z);
  Check();
  return;
}
//------------------------------------------------------------
// Get a sound buffer or create one
//------------------------------------------------------------
CSoundBUF *CAudioManager::ReserveSoundBUF(Tag idn,char *fn,int of)
{ CSoundBUF *buf = 0;
  std::map<Tag,CSoundBUF*>::iterator s = buffers.find(idn);
  if (s != buffers.end()) 
  { buf = (*s).second;
    buf->IncUser();
    return buf;
  }
  //-- Create a sound buffer with the file -----------
  buf = new CSoundBUF(idn,fn);
  buf->SetByteOffset(of);
  buffers[idn] = buf;
  return buf;
}
//------------------------------------------------------------
// Get a sound buffer from cache
//------------------------------------------------------------
CSoundBUF *CAudioManager::GetSoundBUF(Tag idn)
{ CSoundBUF *buf = 0;
  std::map<Tag,CSoundBUF*>::iterator s = buffers.find(idn);
  if (s == buffers.end())   return 0;
  buf = (*s).second;
  buf->IncUser();
  return buf;
}
//------------------------------------------------------------
// Release a sound buffer
//------------------------------------------------------------
void CAudioManager::ReleaseSoundBUF(Tag idn)
{ CSoundBUF *sbf = 0;
  std::map<Tag,CSoundBUF*>::iterator s = buffers.find(idn);
  if (s == buffers.end()) return;
  sbf =(*s).second;
  if (sbf->DecUser())     return;
  delete (sbf);
  buffers.erase(s);
  return;
}
//------------------------------------------------------------
// Get a sound buffer by name or create one
//------------------------------------------------------------
CSoundBUF *CAudioManager::ReserveSoundBUF(char *fn)
{ CSoundBUF *sbf = 0;
  std::map<std::string,CSoundBUF*>::iterator s = snames.find(fn);
  if (s != snames.end()) 
  { sbf = (*s).second;
    sbf->IncUser();
    return sbf;
  }
  //-- Create a sound buffer with the file -----------
  char txt[8];
  _snprintf(txt,8,"S%03d",No++);
  Tag  idn = StringToTag(txt);
  //MEMORY_LEAK_MARKER ("ampwav")
  sbf = new CSoundBUF(idn,fn);
  //MEMORY_LEAK_MARKER ("ampwav")
  sbf->SetName(fn);
  snames[fn] = sbf;
  return sbf;
}
//------------------------------------------------------------
// Release a sound buffer
//------------------------------------------------------------
void CAudioManager::ReleaseSoundBUF(CSoundBUF *sbf)
{ if (0 == sbf)           return;
  if (sbf->DecUser())     return;
  char *fn = sbf->GetFile();
  snames.erase(fn);
  delete (sbf);
  return;
}
//------------------------------------------------------------
// Free the alu buffer
//------------------------------------------------------------
void CAudioManager::DeleteBuffer(U_INT bf)
{ ALuint buf = bf;
  alDeleteBuffers(1,&buf);
  Check();
}
//------------------------------------------------------------
// Load a wav file into the buffer
//------------------------------------------------------------
void CAudioManager::LoadFile(CSoundBUF *sbf,char *fn)
{ char *dot = strrchr(fn,'.');
  if (0 == dot)	return;
	if (dot[4])		dot[4]	= 0;				// Eliminate comment
	PFS  *pfs	= &globals->pfs;
  char wavfilepath [256];
	_snprintf(wavfilepath,255,"SOUND/%s",fn);
  //MEMORY_LEAK_MARKER ("popen")
  PODFILE *p = popen (pfs, wavfilepath);
  //MEMORY_LEAK_MARKER ("popen")
  if (0 == p) return;
  //---Read file in memory -------------------------------------
  int wavSize     = p->size; 
  char *wavBuffer = new char[wavSize];
  pread (wavBuffer, 1, wavSize, p);
  //---Use ALUT to load the file until a better lib is used ----
  // Load the WAV from the memory buffer
  ALenum format     = 0;
  ALsizei size      = 0;
  ALsizei freq      = 0;
  ALfloat frequency = 0.0f;
  //---Load sound memory ---------------------------------------
  ALvoid *data = alutLoadMemoryFromFileImage(wavBuffer, wavSize, &format, &size, &frequency);
  freq = int(frequency);
  CheckALUT();
  //---- Assign data to AL buffer ------------------------------
  Check();
  ALuint buf = sbf->GetBuffer();
  sbf->SetFrequency(frequency);
  alBufferData (buf, format, data, size, freq);
 // sbf->StoreData(data);
  Check();
  delete [] wavBuffer;
  SAFE_DELETE (p); // lc 052310 +
  //-----That's all folks -----------------------------------
  return;
}

//=============END OF FILE =========================================================