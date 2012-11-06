/*
 * TimeManager.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2006 Chris Wallace
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

/*! \file TimeManager.h
 *  \brief Defines CTimeManager class for all simulation time management
 */

#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <time.h>
#include "FlyLegacy.h"
//#include <plib/ul.h>

// Check that exactly one precision timer method is defined


//======================================================================
//  Precision Time Manager
//======================================================================
class CTimeManager {
public:
  CTimeManager ();

public:


  // CTimeManager methods
  void    Prepare (void);
  void    Update (void);
  // JSDEV inline those  calls -------------------------------
  inline float    GetElapsedSimTime (void){return elapsed; }
  inline float    GetElapsedRealTime(void){return elapsedReal;}
  inline float    GetDeltaRealTime (void) {return dRealTime;}
  inline void     SetDeltaRealTime (float &rt) {dRealTime = rt;}
  inline float    GetDeltaSimTime  (void) {return dSimTime;}
  inline void     SetDeltaSimTime  (float &dt) {dSimTime = dt;}
  //----------------------------------------------------------
  inline float    GetTimeScale()    {return scale;}
  inline bool     GetPauseState()   {return paused;}
  inline void     SwapPause()       {paused ^= true;}
  //-----------------------------------------------------------
  inline void     Pause()               {paused = true;}
  inline void     Unpause()             {paused = false;}
  inline void     PauseAs(int p)        {paused = (p != 0);}
  inline void     SetTimeScale(float t) {scale  = t;}
  inline float    TimeForward()         {scale *= 2;    return scale; }
  inline float    TimeBackward()        {scale *= 0.5f; return scale; }
  //-----------------------------------------------------------
  void    SetUTCDateTime (SDateTime dt);
  void    SetLocalDateTime (SDateTime dt);
  void    SetTimeZoneDelta (bool subtract, SDateTimeDelta delta);
  void    SetTimeZone();
  void    SetTimeZoneOffset (float offset);
  void    GetTimeZoneDelta (SDateTimeDelta &delta, bool &subtract);
  float   GetTimeZoneOffset (void);

  SDateTime GetUTCDateTime (void);
  SDateTime GetLocalDateTime (void);
  double    GetJulianDate (void);
  double    GetModifiedJulianDate (void);
  double    GetGreenwichSiderealTime (void);
  double    GetLocalSiderealTime (double lon);

  ETimeOfDay  GetTimeOfDay (void);

  void    Print (FILE *f);

  // Static methods for general purpose time-related utilities
  static void SunriseSunset (SPosition pos, SDate date, float zenith,
                 STime &rise, STime &set,
                 bool &neverRises, bool& neverSets);

  static int            DaysInMonth (int month, int year);
  static double         JulianDate (SDateTime dt);
  static SDateTime      CalendarDate (double j);
  static SDateTimeDelta SecondsToDateTimeDelta (float s);
  static SDateTimeDelta DaysToDateTimeDelta (float d);
  static double         DateTimeDeltaToDays (SDateTimeDelta delta);
  static SDateTime      AddTimeDelta (SDateTime dt, SDateTimeDelta delta);
  static SDateTime      SubtractTimeDelta (SDateTime dt, SDateTimeDelta delta);
  static SDateTimeDelta SubtractTime (SDateTime from, SDateTime to);

protected:
  float   elapsed;          ///< Elapsed sim time (sec) since simulation was started
  float   elapsedReal;      ///< Elapsed real time (sec) since simulation was started

  // Delta time between calls to Update() in seconds
  float   dRealTime;
  float   dSimTime;

  // Time scale is the realtime scaling factor
  float   scale;

  // Whether simulation time is paused or not
  bool    paused;

  bool    timeAdvanceContinuous;      ///< Whether time advance is continuous or discrete

  // Local time zone settings
  //  tzDelta   represents number of hours to be subtracted or added to UTC
  //  tzSubtract  indicates whether the delta should be subtracted from UTC
  SDateTimeDelta    tzDelta;
  bool        tzSubtract;

  // "Zero" date/time, i.e. the calendar date and time when the simulation
  //   was started.  This base value is used in conjunction with the simulation
  //   elapsed time to derive the current simulation date/time
  SDateTime epoch;

  // CTimeManager uses the Win32 functions QueryPerformanceFrequency
  //   and QueryPerformanceCounter as its high-resolution counters on
  //   the Windows platform
  float     freq;         // Counter frequency
  LARGE_INTEGER count, prev_count;    // Counter values
};


#endif // TIMEMANAGER_H

