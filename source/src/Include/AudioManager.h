/*
 * AudioManager.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
 * copyright 2007 Jean Sabatier
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


/*! \file AudioManager.h
 *  \brief Defines CAudioManager class for application control of all sounds
 *
 *  Class CAudioManager performs all application sound effect management.
 *    The current implementation in AudioManager.cpp uses OpenAL
 *    (www.openal.org) so there are some OpenAL-specific concepts in the
 *    audio manager.  Ideally this class would be completely independent
 *    of the underlying audio library.
 *
 */
//====================================================================================

#ifndef AUDIOMANAGER_H_
#define AUDIOMANAGER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlyLegacy.h"
#include <string>
#include <map>

#include <AL/alut.h>
#include <al.h>

//=====================================================================================
class CSoundEffect;
//=====================================================================================
typedef enum {
  SOUND_LOOP    = AL_LOOPING,
} ESoundMode;
//=====================================================================================
//  Sound Buffer:  An AL buffer dedicated to one sound
//
//=====================================================================================
class CSoundBUF {
  //---ATTRIBUTS -------------------------------------------------------
  char    itag[8];          // Tag ident for debbug
  Tag     idn;              // Identifier
  int     user;             // user  count
  char   *nfile;            // File name
  //---sound parameters ------------------------------------------------
  U_INT   alBUF;            // Al buffer
  U_INT   alSRC;            // Al source
  float   freq;             // Frequency
  int    offset;            // starting offset
  float   gain;             // maximum gain
  float   pMax;             // Maximum pitch
  float   pMin;             // Minimum pitch
  //---METHODS ---------------------------------------------------------
public:
  CSoundBUF(Tag idn, char *nf);
 ~CSoundBUF();
  bool    DecUser();
  void    Release();
  void    Play(char b);
  void    PlayS1();
  void    SetPitch(float v);
  //--------------------------------------------------------------------
  void    SetName(char *n);
  //--------------------------------------------------------------------
  inline  void  SetFrequency(float f) {freq = f;}
  inline  void  IncUser()             {user++;}
  inline  U_INT GetBuffer()           {return alBUF;}
  inline  Tag   GetTag()              {return idn;}
  inline  int   GetOffset()           {return offset;}
  inline  float Frequency()           {return freq;}
  inline  char *GetFile()             {return nfile;}
  //--------------------------------------------------------------------
  inline  void  SetByteOffset(int b)  {offset = b;}
  inline  void  SetGain(float g)      {gain = g;}
  //--------------------------------------------------------------------
  inline  void SetPMIN(float v)       {pMin = v;}
  inline  void SetPMAX(float v)       {pMax = v;}
  inline float GetPitch(float p)      {return (p * (pMax - pMin)) + pMin;}
};
//=====================================================================================
//  CSoundSRC: 
//  This class represent a non localized source used for panel and subsystems
//=====================================================================================
class CSoundSRC {
  //-----ATTRIBUTES ---------------------------------------------
  U_INT       alSRC;                // AL source
  CSoundBUF  *sBuf;                // Buffer in play
  char        loop;
  //-----METHODES -----------------------------------------------
public:
  CSoundSRC();
  void    Play(CSoundBUF *sbf);
  //-------------------------------------------------------------
  inline U_INT GetSource()    {return alSRC;}
  inline void Loop(char k)  {loop = k;}
};
//=====================================================================================
//  CSoundOBJ: 
//  This class represent a sound source like an engine or a gear or any other
//  part that may emit sounds.
//  A SoundOBJ has a fixed position relative to the aircraft origin
//  All soundOBJ are collected into a dedicated cache.
//=====================================================================================
class CSoundOBJ {
protected:
  //-------ATTRIBUTES ---------------------------------------------------
  Tag         idn;                // Object identifier
  U_INT       alSRC;              // OpenAL Source object
  CSoundBUF  *sPlay;              // Playing source
  float       gain;               // Volume
  float       pitch;              // pitch
  bool        loop;               // loop mode
  int         offset;             // file offset
  //----Stack of Sound buffers playable by this object -------------------
  char        sEnd;                     // Number of sounds
  CSoundBUF  *sounds[ENGINE_MAX_SOUND]; // Stock of sound
  //-------METHODS ------------------------------------------------------
public:
  CSoundOBJ(Tag idn, SVector &pos);   // Create a positional source
  CSoundOBJ();                        // Panel and subsystem
 ~CSoundOBJ();
  void  Init(Tag t, SVector &pos);
  //---------------------------------------------------------------------
  void  Play(int No);      // Play a buffer in the specified pitch
  void  StopSound();
  void  AddSound(int No,Tag idn);
  void  AddSound(int No,CSoundBUF *sbf);
  bool  IsPlaying();
  bool  BeforeOffset();
  void  PlayFromOffset(int No);
  void  FillFlaps();
  void  SetSBUF(char k,CSoundBUF *sbf);
  void  SetEnginePitch(float v);
  //---------------------------------------------------------------------
  inline  Tag        GetTag()         {return idn;}
  inline  CSoundBUF *GetSBF()         {return sPlay;}
  inline  U_INT      GetSRC()         {return alSRC;}
  inline  float      GetGain()        {return gain;}
  inline  float      GetPitch()       {return pitch;}
  inline  bool       GetLoop()        {return loop;}
  inline  int        GetOffset()      {return offset;}
  
  //----------------------------------------------------------------------
	inline  void			 SetOffset(int n)		{offset = n;}
  inline  void       SetLoop(bool v)    {loop = v;}
  inline  void       SetGain(float v)   {gain = v;}
  
  //----------------------------------------------------------------------
};

//=====================================================================================
///
/// CAudioManager provides application control of all sound effects.  The
///   methods are modelled after the audio API functions in the Fly! SDK.
///
//=====================================================================================
class CAudioManager {
  //---ATTRIBUTES ------------------------------------------------------
  ALCcontext    *context;
  ALCdevice     *device;
  //---Stock of sound sources ----------------------------------------
  char          nSRC;                           // Number of sources
  char          xSRC;                           // Current source
  CSoundSRC     *source;                        // Stock of SRC
  //------------------------------------------------------------------
  int   No;                                     // Sound number
  std::map<Tag,CSoundBUF*>    buffers;          // Buffer cache by tag
  std::map<std::string,CSoundBUF*> snames;           // cache by name
  //-------------------------------------------------------------------
  int   nbSRC;                                  // Number of Sources
  int   nbBUF;                                  // Number of Buffers
public:
  CAudioManager (void);
 ~CAudioManager();
  // CAudioManager methods
  void Warn(char * msg,ALint error);	
  void Init (void);
  void AllocateSources(char n);
  void Cleanup (void);
  void SetListenerPosition (SPosition *p);
  //---------------------------------------------------------------------
  U_INT       Play(CSoundBUF *sbf);
  void        Loop(CSoundBUF *sbf);
  bool        IsPlaying(U_INT s);
  //---------------------------------------------------------------------
  void        CheckALUT();
  void        Check();
  U_INT       CreateSource (void);
  void        DeleteBuffer(U_INT buf);
  void        DeleteSource(U_INT src);
  void        SetSourcePos(U_INT src,SVector &pos);
  CSoundBUF  *ReserveSoundBUF(Tag t,char *nf,int of= 0);
  CSoundBUF  *GetSoundBUF(Tag t);
  void        ReleaseSoundBUF(Tag idn);
  //---------------------------------------------------------------------
  CSoundBUF  *ReserveSoundBUF(char *fn);
  void        ReleaseSoundBUF(CSoundBUF *sbf);
  void        LoadFile(CSoundBUF *sbf,char *fn);
  //---------------------------------------------------------------------
  void        SetMode(ALuint s,U_INT m,bool b) {alSourcei(s,m,b);}
  //---------------------------------------------------------------------
  inline void IncSRC()        {nbSRC++;}
  inline void IncBUF()        {nbBUF++;}
  //---------------------------------------------------------------------

};
//====================END OF FILE ==============================================================
#endif // AUDIOMANAGER_H_
