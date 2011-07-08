/*
 * TimeManager.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
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

/*! \file TimeManager.cpp
 *  \brief Implements CTimeManager real-time control for application
 *
 * Management of the simulation world date/time and time progress is
 *   encapsulated in the CTimeManager class implemented here.  This manager
 *   bridges time in the real world with time in the simulation world.
 *   Features include the ability to set the world date and time, adjust
 *   the rate at which simulation time progresses relative to real-time,
 *   and time and calendar-related utilities.
 */


#include <time.h>
#include <sys/timeb.h>
#include "../Include/TimeManager.h"
#include "../Include/Utility.h"
#include "../Include/Ui.h"


// Define MAXLONGLONG if not already defined.  Account for GNU compiler
//   requirement for LL suffix
#ifdef __GNUC__
#define LL_CONSTANT(val) val ## LL
#else
#define LL_CONSTANT(val) val
#endif // __GNUC__

#ifndef MAXLONGLONG
#define MAXLONGLONG LL_CONSTANT(0x7fffffffffffffff)
#endif // MAXLONGLONG


CTimeManager::CTimeManager (void)
{
#ifdef PRECISION_TIMER_WIN32
  // Initialize Win32 performance counter variables
  count.QuadPart = 0;
  prev_count.QuadPart = 0;
#endif

#ifdef PRECISION_TIMER_PLIB
  // Instantiate ulClock
  clock = new ulClock;
#endif

  // Initialize time scale
  scale = 1.0;

  // Default to un-paused
  paused = false;

  // Initialize epoch to current system time
  time_t t;
  time (&t);
  struct tm* utc = gmtime (&t);
  if (utc == NULL) gtfo("No time!");
  epoch.date.year = utc->tm_year;
  epoch.date.month = utc->tm_mon + 1;
  epoch.date.day = utc->tm_mday;
  epoch.time.hour = utc->tm_hour;
  epoch.time.minute = utc->tm_min;
  epoch.time.second = utc->tm_sec;
  epoch.time.msecs = 0;
  //---- Initialize timezone to UTC -------------
  tzDelta.dYears = 0;
  tzDelta.dMonths = 0;
  tzDelta.dDays = 0;
  tzDelta.dHours = 0;
  tzDelta.dMinutes = 0;
  tzDelta.dSeconds = 0;
  tzDelta.dMillisecs = 0;
  tzSubtract = false;
  // Initialize simulation and real elapsed time
  elapsed = 0;
  elapsedReal = 0;
  //----Get local time difference --------------
  SetTimeZone();
}


///
/// Set the simulation date and time in Universal Coordinated Time.
///
/// @param dt Date/Time in SDateTime struct
///
void CTimeManager::SetUTCDateTime (SDateTime dt)
{
  epoch = dt;
  elapsed = 0;
  elapsedReal = 0;
}

///
/// Set the simulation date and time in Local Time.  Local time zone is
///   initialized from the user's computer, unless overridden by a call
///   to the SetTimeZoneDelta method.
///
/// @param dt Date/Time in SDateTime struct
///
void CTimeManager::SetLocalDateTime (SDateTime dt)
{
  // Adjust local time by timezone delta to get UTC time
  if (tzSubtract) {
    // TZ delta is subtracted from UTC to get local, so we have to add the delta
    //   to the supplied local time to get UTC
    dt = AddTimeDelta (dt, tzDelta);
  } else {
    // TZ delta is added to UTC to get local, so we have to subtract the delta
    //   from the supplied local time to get UTC
    dt = SubtractTimeDelta (dt, tzDelta);
  }

  SetUTCDateTime (dt);
}

///===============================================================================
/// Set the local time zone offset relative to UTC.  This offset will be used
///   for all subsequent conversions from UTC to local time.  Note that the
///   simulation time (which is stored as UTC) is not affected when the local
///   time zone offset is   updated
///
/// @param subtract true if time zone delta is subtracted from UTC,
///                 false if it is added to UTC
/// @param delta    Time zone delta in SDateTimeDelta struct
///===============================================================================
void CTimeManager::SetTimeZoneDelta (bool subtract, SDateTimeDelta delta)
{  
  tzSubtract = subtract;
  tzDelta = delta;
}
///===============================================================================
//  Set time zone from computer
//===============================================================================
void CTimeManager::SetTimeZone()
{ struct __timeb64 t64;
  _ftime64(&t64);           // Get Time offset
  int dh = 0;
  _get_daylight(&dh);
  int tz = t64.timezone;
  int hh = abs(tz) / 60;
  int mn = abs(tz) % 60;
  tzDelta.dHours = hh + dh;
  tzDelta.dMinutes = mn;
  tzSubtract = (tz < 0)?(false):(true);
}
///==============================================================================
/// Set the local time zone offset relative to UTC.  This offset will be used
///   for all subsequent conversions from UTC to local time.  Note that the
///   simulation time (which is stored as UTC) is not affected when the local
///   time zone offset is   updated
///
/// @param offset  Offset of local timezone from UTC, in hours
///==============================================================================
void CTimeManager::SetTimeZoneOffset (float offset)
{
  if (offset < 0) {
    // Offset should be subtracted from UTC to get local time
    tzSubtract = true;
    offset = -offset;
  } else {
    // Offset should be added to UTC to get local time
    tzSubtract = false;
  }

  // Convert hours offset to SDateTimeDelta struct
  tzDelta.dHours = (int)(floor (offset));
  tzDelta.dMinutes = (int)(fmod (offset, 1.0f) * 60);

  /// \todo Complete implementation of CTimeManager::SetTimeZoneOffset (float)
}

//
// Get local time zone offset in hours relative to UTC
//
void CTimeManager::GetTimeZoneDelta (SDateTimeDelta &delta, bool &subtract)
{
  delta = tzDelta;
  subtract = tzSubtract;
}


//
// Get local time zone offset in hours relative to UTC
//
float CTimeManager::GetTimeZoneOffset (void)
{
  float offset = (float)tzDelta.dHours + ((float)tzDelta.dMinutes / 60);
  if (tzSubtract) offset = -offset;

  return offset;
}


//
// This method is called after all initialization has been done and the simulation
//   is about to begin.  Simulation time "starts" after this method is called
//
void CTimeManager::Prepare (void)
{
#ifdef PRECISION_TIMER_WIN32
  // Determine the frequency of the high performance counter
  LARGE_INTEGER perf_freq;
  if (!QueryPerformanceFrequency (&perf_freq)) {
    gtfo ("CTimeManager : QueryPerformanceFrequency failed");
  }
  freq = (float)perf_freq.QuadPart;

  // Initialize high performance counter for use in delta time comparison in Update()
  QueryPerformanceCounter (&prev_count);
#endif

  dSimTime = 0;
  dRealTime = 0;
  elapsed = 0;
  elapsedReal = 0;
}


//
// Returns the simulation UTC date and time
//
// Note: Contrary to localtime() convention, the month value ranges
//   from 1..12 for January..December
//
SDateTime CTimeManager::GetUTCDateTime (void)
{ // Convert sim elapsed time from seconds to days
  SDateTimeDelta delta = SecondsToDateTimeDelta (elapsed);
  // Add elapsed time delta to epoch date/time
  SDateTime dt = AddTimeDelta (epoch, delta);
  return dt;
}

//
// Returns the simulation local date and time.  Note this is based on the
//   timezone of the users' PC, not that of the simulation location.
//
// Note: Contrary to localtime() convention, the month value ranges
//   from 1..12 for January..December
//
SDateTime CTimeManager::GetLocalDateTime (void)
{ SDateTime dt = GetUTCDateTime ();
  // Adjust by time zone offset
  if (tzSubtract) {
    dt = SubtractTimeDelta (dt, tzDelta);
  } else {
    dt = AddTimeDelta (dt, tzDelta);
  }
  return dt;
}

//
// Return the sim Universal time as julian date
//
double CTimeManager::GetJulianDate (void)
{
  SDateTime dt = GetUTCDateTime ();
  double jd = JulianDate (dt);
  return jd;
}

//
// Return the sim Universal time as modified julian date
//
double CTimeManager::GetModifiedJulianDate (void)
{
  double jd = GetJulianDate ();
  double mjd = jd - 2400000.5;
  return mjd;
}


#define DEGHR(x)        ((x)/15.)
#define ARCSECHR(x)   DEGHR((x)/3600)


#define J2000   (double(2451545.0))
#define SIDRATE (double(0.9972695677))


//=======================================================================
// Return Greenwich Mean Sidereal Time, in decimal hours
//   
// Algorithm taken based on: http://aa.usno.navy.mil/faq/docs/GAST.html
//   (The T^2 element is not included)
//=======================================================================
double CTimeManager::GetGreenwichSiderealTime (void)
{
  double jd = GetJulianDate ();
  
  // Calculate julian date of the last midnight
  double jd0 = floor (jd - 0.5) + 0.5;

  // Calculate hours elapsed since last midnight
  double hrs = (jd - jd0) * 24.0;

  // Modify the dates relative to J2000.0 epoch (JD 2451545.0)
  double d0 = jd0 -= J2000;

  // Calculate GMST
  double gmst = 6.697374558 +
              (0.06570982441908 * d0) +
          (1.00273790935 * hrs);
  gmst = Wrap24 (gmst);

  return gmst;
}
//=======================================================================

//============================================================================
// Return Local Sidereal Time, in decimal hours, based
//   on the current sim time and the supplied longitude
//   (in arcseconds East of the Prime Meridian)
//============================================================================
double CTimeManager::GetLocalSiderealTime (double lon)
{
  // Get Greenwich Sidereal Time in decimal hours
  double gst = GetGreenwichSiderealTime();

  // Convert longitude to decimal hours
  double lonh = DEGHR (Wrap180 (lon / 3600));

  // The following is based on Paul Schluyter's web site:
  double lst = gst + lonh;
  lst = Wrap24 (lst);
  return lst;
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4701)
#endif

void CTimeManager::Update (void)
{
#ifdef PRECISION_TIMER_WIN32
  LARGE_INTEGER delta;

  // Get current value of performance timer
  if (QueryPerformanceCounter (&count)) {
    // Check for wrap-around
    if (count.QuadPart < prev_count.QuadPart) {
      delta.QuadPart = count.QuadPart + (MAXLONGLONG - prev_count.QuadPart);
    } else {
      delta.QuadPart = count.QuadPart - prev_count.QuadPart;
    }
    prev_count = count;
  }

  // Calculate time since last time update
  dRealTime = (float)delta.QuadPart / freq;
#endif

#ifdef PRECISION_TIMER_PLIB
  dRealTime = (float)(clock->getDeltaTime());
#endif

  // Calculate simulation elapsed time for this update
  dSimTime = scale * dRealTime;

//  char debug[80];
//  sprintf (debug, "TimeManager::Update : dRealTime = %10.8f  dSimTime=%10.8f",
//    dRealTime, dSimTime);
//  DrawNoticeToUser (debug, 1);

  // If sim is not paused, update sim elapsed time
  if (!paused) {
    elapsed += dSimTime;
  }

  // Always update elapsed real time
  elapsedReal += dRealTime;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

//
// Return sim elapsed time
// inlined now
//float CTimeManager::GetElapsedSimTime (void)
//{
//  return elapsed;
//}

//
// Return real elapsed time
//inlined now
//float CTimeManager::GetElapsedRealTime (void)
//{
//  return elapsedReal;
//}
//
// Return the amount of time between the last time manager update
// inlined now
//float CTimeManager::GetDeltaRealTime (void)
//{
//  return dRealTime;
//}

//
// Return the amount of sim time between the last time manager updates
// inlined now
//float CTimeManager::GetDeltaSimTime (void)
//{
//  return dSimTime;
//}

//
// Algorithm supplied from:
//    Almanac for Computers, 1990
//    Nautical Almanac Office
//    United States Naval Observatory
//
void CTimeManager::SunriseSunset (SPosition pos, SDate date, float zenith,
                  STime &rise, STime &set,
                  bool &neverRises, bool& neverSets)
{
  // Step 0 - Initialize
  rise.second = 0;
  rise.msecs = 0;
  set.second = 0;
  set.msecs = 0;

  // Step 1 - Calculate day of year
  float n1 = (float)(floor(275.0f * (float)date.month / 9.0f));
  float n2 = (float)(floor(((float)date.month + 9.0f) / 12.0f));
  float n3 = (float)((1 + floor(((float)date.year - 4.0f * floor((float)date.year / 4.0f) + 2.0f) / 3.0f)));
  float n = n1 - (n2 * n3) + date.day - 30;

//  WriteLog ("n1=%f n2=%f n3=%f n=%f\n", n1, n2, n3, n);

  // Step 2 - Convert lon to hour value and compute approximate time
  float lonHour = (float)(pos.lon / 15);

//  WriteLog ("lonHour=%f\n", lonHour);

  float tRise = n + ((6 - lonHour) / 24);
  float tSet = n + ((18 - lonHour) / 24);

//  WriteLog ("tRise=%f  tSet=%f\n", tRise, tSet);

  // Step 3 - Calculate sun's mean anomaly
  float mRise = (0.9856f * tRise) - 3.289f;
  float mSet = (0.9856f * tSet) - 3.289f;

//  WriteLog ("mRise=%f mSet=%f\n", mRise, mSet);

  // Step 4 - Calculate sun's true longitude
  float lonRise = (float)(mRise + (1.916 * sin(mRise)) + (0.020 * sin(2 * mRise)) + 282.634);
  Wrap360 (lonRise);

  float lonSet = (float)(mSet + (1.916 * sin(mSet)) + (0.020 * sin(2 * mSet)) + 282.634);
  Wrap360 (lonSet);

//  WriteLog ("lonRise=%f lonSet=%f\n", lonRise, lonSet);

  // Step 5a - Calculate right ascension
  float raRise = RadToDeg ((float)(atan(0.91764 * tan(lonRise))));
  Wrap360 (raRise);

  float raSet = RadToDeg ((float)(atan(0.91764 * tan(lonSet))));
  Wrap360 (raSet);

//  WriteLog ("a - raRise=%f raSet=%f\n", raRise, raSet);

  // Step 5b - Adjust RA to same quadrant as longitude
  float quadrantLon = (float)(floor(lonRise/90) * 90);
  float quadrantRA = (float)(floor(raRise/90) * 90);
  raRise += quadrantLon - quadrantRA;
  
  quadrantLon = (float)(floor(lonSet/90) * 90);
  quadrantRA = (float)(floor(raSet/90) * 90);
  raSet += quadrantLon - quadrantRA;
  
//  WriteLog ("b - raRise=%f raSet=%f\n", raRise, raSet);

  // Step 5c - Convert RA to hours
  raRise /= 15;
  raSet /= 15;

//  WriteLog ("c - raRise=%f, raSet=%f\n", raRise, raSet);

  // Step 6 - Calculate sun's declination
  double sinDecRise = (0.39782 * sin(lonRise));
  double cosDecRise = (cos(asin(sinDecRise)));

  double sinDecSet = (0.39782 * sin(lonSet));
  double cosDecSet = (cos(asin(sinDecSet)));

//  WriteLog ("sinDecRise = %g cosDecRise = %g\n", sinDecRise, cosDecRise);

  // Step 7a - Calculate sun's local hour angle
  double cosHRise = (float)(cos(zenith) - (sinDecRise * sin(pos.lat))) / (cosDecRise * cos(pos.lat));
  double cosHSet = (float)(cos(zenith) - (sinDecSet * sin(pos.lat))) / (cosDecSet * cos(pos.lat));

//  WriteLog ("cosHRise = %f\n", cosHRise);

  neverRises = false;
  if (cosHRise > 1) {
    // Sun never rises
    neverRises = true;
  }

  neverSets = false;
  if (cosHRise < -1) {
    // Sun never sets
    neverSets = true;
  }

  // Step 7b - Finish calculating H and convert into hours
  float hRise = (float)(360 - RadToDeg(acos(cosHRise)));
  hRise /= 15;

  float hSet = (float)(RadToDeg (acos(cosHSet)));
  hSet /= 15;

//  WriteLog ("hRise=%f hSet=%f\n", hRise, hSet);

  // Step 8 - Calculate local mean time
  float timeRise = (float)(hRise + raRise - (0.06571 * tRise) - 6.622);
  float timeSet = (float)(hSet + raSet - (0.06571 * tSet) - 6.622);

  // Step 9 - Adjust to UTC
  float utRise = timeRise - lonHour;
  Wrap24 (utRise);

  float utSet = timeSet - lonHour;
  Wrap24 (utSet);

  // Update input arguments
  rise.hour = (int)(floor(utRise));
  rise.minute = (int)(fmod(utRise, 1.0f) * 60.0f);

  set.hour = (int)(floor(utSet));
  set.minute = (int)(fmod(utSet, 1.0f) * 60.0f);
}


int CTimeManager::DaysInMonth (int month, int year)
{
  unsigned int rc = 30;

  switch (month) {
  case 1:
    // January
    rc = 31;
    break;

  case 2:
    // February
    if ((year % 4 == 0) && (year % 400 != 0)) {
      // Leap year
      rc = 29;
    } else {
      // Normal year
      rc = 28;
    }
    break;

  case 3:
    // March
    rc = 31;
    break;

  case 4:
    // April
    rc = 30;
    break;

  case 5:
    // May
    rc = 31;
    break;

  case 6:
    // June
    rc = 30;
    break;

  case 7:
    // July
    rc = 31;
    break;

  case 8:
    // August
    rc = 31;
    break;

  case 9:
    // September
    rc = 30;
    break;

  case 10:
    // October
    rc = 31;
    break;

  case 11:
    // November
    rc = 30;
    break;

  case 12:
    // December
    rc = 31;
    break;
  }

  return rc;
}


//
//  This function converts a Gregorian date and time of day into the corresponding
//    Julian day number.
//
//  Parameters:
//    d SDate structure containing year (relative to 1900)/month/day
//    t STime structure containing hour/minute/second
//
double CTimeManager::JulianDate (SDateTime dt)
{
  double j = 0;

  // Convert date to julian day number
  double yy = (double)dt.date.year + 1900.0;
  double mm = (double)dt.date.month;
  double dd = (double)dt.date.day;

  // Adjust for January/February if required
  if (mm < 3) {
    mm += 12;
    yy -= 1;
  }

  double a = floor (yy / 100);      // Non-leap days on even centuries
  double b = floor (a / 4);       // Leap days on even-four year bounds
  double c = 2 - a + b;         // Total number of leap days
  double e = floor (365.25 * (yy + 4716));// Days to zeroth day of year
  double f = floor (30.6001 * (mm + 1));  // Days to zeroth day of month

  j = c + dd + e + f - 1524.5;      // Total number of whole days

  double secs = dt.time.hour * 3600 +     // Number of seconds since midnight
          dt.time.minute * 60 +
          dt.time.second +
          (dt.time.msecs / 1000);
  j += secs / 86400;            // Fractional number of days

  return j;
}


//
// Algorithm is based on that in Jean Meeus' "Astronomical Formulae for
//   Calculators" as described on the "Ask Dr. Math" web site:
//      http://mathforum.org/library/drmath/view/51907.html
//
SDateTime CTimeManager::CalendarDate (double j)
{
  SDateTime dt;

  // Add 0.5 to julian date; z = whole part, f = fract part
  j += 0.5;
  double z = floor (j);
  double f = fmod (j, 1.0);
  
  // Calculate intermediate variables
  double a;
  if (z < 229161) {
    a = z;
  } else {
    double alpha = floor ((z - 1867216.25) / 36524.25);
    a = z + 1 + alpha - floor (alpha / 4);
  }

  double b = a + 1524;
  double c = floor ((b - 122.1) / 365.25);
  double d = floor (365.25 * c);
  double e = floor ((b - d) / 30.6001);

  // Calculate day of month (fractional)
  double dd = b - d - floor (30.6001 * e) + f;

  // Calculate month
  double mm;
  if (e < 13.5) {
    mm = e - 1;
  } else {
    mm = e - 13;
  }

  // Calculate year
  double yy;
  if (mm >= 2.5) {
    yy = c - 4716;
  } else {
    yy = c - 4715;
  }

  // Convert float day, month, year in to SDateTime
  dt.date.year  = (int) floor (yy);
  dt.date.month = (int) floor (mm);
  dt.date.day   = (int) floor (dd);

  // Convert fractional day to integer number of seconds
  int secs = (int) floor (fmod (dd, 1.0) * 86400);
  dt.time.hour  = secs / 3600;
  secs -= dt.time.hour * 3600;
  dt.time.minute  = secs / 60;
  dt.time.second  = secs % 60;
  dt.time.msecs = 0;

  return dt;
}



//
// Convert a raw number of seconds into a SDateTimeDelta struct
//
// NOTE: This function cannot compute the number of months and years
//   since this depends on the starting date at which the delta is applied.
//   The dMonths and dYears fields of the returned SDateTime struct will
//   always be zero.  The dDays field may be any number of days (not necessarily
//   less than a month).
//
#define   SECS_PER_MINUTE   (60)
#define   SECS_PER_HOUR   (SECS_PER_MINUTE * 60)
#define   SECS_PER_DAY    (SECS_PER_HOUR * 24)

SDateTimeDelta CTimeManager::SecondsToDateTimeDelta (float s)
{
  SDateTimeDelta delta;

  float days = floorf (s / SECS_PER_DAY);
  s -= days * SECS_PER_DAY;
  float hrs = floorf (s / SECS_PER_HOUR);
  s -= hrs * SECS_PER_HOUR;
  float min = floorf (s / SECS_PER_MINUTE);
  s -= min * SECS_PER_MINUTE;
  float secs = floorf (s);
  s -= secs;
  float msecs = fmodf (s, 1) * 1000;

  delta.dYears = 0;
  delta.dMonths = 0;
  delta.dDays = (int)days;
  delta.dHours = (int)hrs;
  delta.dMinutes = (int)min;
  delta.dSeconds = (int)secs;
  delta.dMillisecs = (int)msecs;

  return delta;
}


double CTimeManager::DateTimeDeltaToDays (SDateTimeDelta delta)
{
  double result = 0;

  result += delta.dDays;
  result += delta.dHours * (SECS_PER_HOUR / SECS_PER_DAY);
  result += delta.dMinutes * (SECS_PER_MINUTE / SECS_PER_DAY);
  result += delta.dSeconds * (1 / SECS_PER_DAY);
  result += delta.dMillisecs / 1000;

  return result;
}



#define   DAYS_PER_HOUR     (1.0 / 24.0)
#define   DAYS_PER_MINUTE   (DAYS_PER_HOUR / 60.0)
#define   DAYS_PER_SECOND   (DAYS_PER_MINUTE / 60.0)
#define   DAYS_PER_MSEC     (DAYS_PER_SECOND / 1000.0)

SDateTimeDelta CTimeManager::DaysToDateTimeDelta (float d)
{
  float days = floorf (d);
  d -= days;
  float hrs = floorf (d * (float)DAYS_PER_HOUR);
  d -= hrs * (float)DAYS_PER_HOUR;
  float min = floorf (d * (float)DAYS_PER_MINUTE);
  d -= min * (float)DAYS_PER_MINUTE;
  float secs = floorf (d * (float)DAYS_PER_SECOND);
  d -= secs * (float)DAYS_PER_SECOND;
  float msecs = floorf (d * (float)DAYS_PER_MSEC);

  SDateTimeDelta result;

  result.dYears = 0;
  result.dMonths = 0;
  result.dDays = (int)days;
  result.dHours = (int)hrs;
  result.dMinutes = (int)min;
  result.dSeconds = (int)secs;
  result.dMillisecs = (int)msecs;

  return result;
}


//
// Add a time delta to the specified time.
//
// NOTE: As per the SDateTimeDelta definition, all delta values are positive
//
// NOTE: The SDateTimeDelta struct is assumed to have been constructed using
//   one of the other CTimeManager functions, which do not support the
//   dMonths and dYears fields.
//
// NOTE: This was first attempted by converting the starting date/time
//   and the delta into julian days, but even double values do not have
//   sufficient precision for small time deltas
//
SDateTime CTimeManager::AddTimeDelta (SDateTime dt, SDateTimeDelta delta)
{
  SDateTime result;
  int carry = 0;

  // Add milliseconds
  result.time.msecs = dt.time.msecs + delta.dMillisecs;
  if (result.time.msecs > 999) {
    carry = 1;
    result.time.msecs -= 1000;
  } else {
    carry = 0;
  }

  // Add seconds
  result.time.second = dt.time.second + delta.dSeconds + carry;
  if (result.time.second > 59) {
    carry = 1;
    result.time.second -= 60;
  } else {
    carry = 0;
  }

  // Add minutes
  result.time.minute = dt.time.minute + delta.dMinutes + carry;
  if (result.time.minute > 59) {
    carry = 1;
    result.time.minute -= 60;
  } else {
    carry = 0;
  }

  // Add hours
  result.time.hour = dt.time.hour + delta.dHours + carry;
  if (result.time.hour > 23) {
    carry = 1;
    result.time.hour -= 24;
  } else {
    carry = 0;
  }

  // Add days
  result.date.day = dt.date.day + delta.dDays + carry;
  unsigned int daysInMonth = DaysInMonth (dt.date.month, dt.date.year);
  if (result.date.day > daysInMonth) {
    // Wrap ahead to next month
    result.date.month = dt.date.month + 1;
    if (result.date.month > 12) {
      result.date.year = dt.date.year + 1;
      result.date.month -= 12;
    } else {
      result.date.year = dt.date.year;
    }
    result.date.day -= daysInMonth;
  } else {
    result.date.year = dt.date.year;
    result.date.month = dt.date.month;
  }

  /// \todo Add months and years

  return result;
}

//
// Add a time delta to the specified time.
//
// NOTE: As per the SDateTimeDelta definition, all delta values are positive
//
// NOTE: The SDateTimeDelta struct is assumed to have been constructed using
//   one of the other CTimeManager functions, which do not support the
//   dMonths and dYears fields.
//
// NOTE: This was first attempted by converting the starting date/time
//   and the delta into julian days, but even double values do not have
//   sufficient precision for small time deltas
//
SDateTime CTimeManager::SubtractTimeDelta (SDateTime dt, SDateTimeDelta delta)
{
  SDateTime result;
  int carry = 0;

  int msecs = dt.time.msecs - delta.dMillisecs;
  if (msecs < 0) {
    carry = 1;
    msecs += 1000;
  } else {
    carry = 0;
  }
  result.time.msecs = msecs;

  int second =  dt.time.second - delta.dSeconds - carry;
  if (second < 0) {
    carry = 1;
    second += 60;
  } else {
    carry = 0;
  }
  result.time.second = second;

  int minute = dt.time.minute - delta.dMinutes - carry;
  if (minute < 0) {
    carry = 1;
    minute += 60;
  } else {
    carry = 0;
  }
  result.time.minute = minute;

  int hour = dt.time.hour - delta.dHours - carry;
  if (hour < 0) {
    carry = 1;
    hour += 24;
  } else {
    carry = 0;
  }
  result.time.hour = hour;

  int day = dt.date.day - delta.dDays - carry;
  if (day < 0) {
    // Wrap back to previous month
    result.date.year = dt.date.year;
    int month = dt.date.month - 1;
    if (month < 0) {
      result.date.year = dt.date.year - 1;
      month += 12;
    } else {
      result.date.year = dt.date.year;
    }
    result.date.month = month;
    day += DaysInMonth (result.date.month, result.date.year);
  } else {
    result.date.year = dt.date.year;
    result.date.month = dt.date.month;
  }
  result.date.day = day;

  return result;
}


//
// Subtract two calendar date/time values to determine the delta between them
//
// NOTE: For simplicity, this function does not compute the number of months
//   and years.  The dMonths and dYears fields of the returned SDateTimeDelta
//   struct will always be zero.  The dDays field may be any number of days
//   (not necessarily less than a month).
//
SDateTimeDelta CTimeManager::SubtractTime (SDateTime from, SDateTime to)
{
  // Convert starting and ending date/time to julian dates
  double jdFrom = JulianDate (from);
  double jdTo = JulianDate (to);

  // Subtract julian dates
  float delta = (float)(jdTo - jdFrom);

  // Convert difference to SDateTimeDelta struct
  SDateTimeDelta result = DaysToDateTimeDelta (delta);

  return result;
}

ETimeOfDay CTimeManager::GetTimeOfDay (void)
{
  ETimeOfDay rc = TIME_DAYTIME;

  /// @todo Replace with accurate dawn/day/dusk/night
  // Temporary implementation, local time boundaries are fixed rather than being
  //   based on solar elevation.  Dawn = 6-7am, Dusk = 6-7pm, other times day or
  //   night as appropriate.
  SDateTime dt = GetLocalDateTime ();
  if (dt.time.hour < 6) {
    rc = TIME_NIGHTTIME;
  } else if (dt.time.hour < 7) {
    rc = TIME_DAWN;
  } else if (dt.time.hour < 18) {
    rc = TIME_DAYTIME;
  } else if (dt.time.hour < 19) {
    rc = TIME_DUSK;
  } else {
    rc = TIME_NIGHTTIME;
  }

  return rc;
}

//
// Dump the contents of the CTimeManager class to a file for debugging
//
void CTimeManager::Print (FILE *f)
{
  fprintf (f, "Time Manager :\n\n");

  // Print base date/time data
  double jd = GetJulianDate ();
  fprintf (f, "JD       : %12.3f\n", jd);
  double mjd = GetModifiedJulianDate ();
  fprintf (f, "MJD      : %12.3f\n", mjd);
  double gst = GetGreenwichSiderealTime();
  char st[64];
  FormatSiderealTime (gst, st);
  fprintf (f, "GST      : %s\n", st);
  fprintf (f, "\n");

  // Print UTC date/time
  SDateTime ut = GetUTCDateTime ();
  fprintf (f, "UTC Date   : %04d/%02d/%02d \n", ut.date.year + 1900, ut.date.month, ut.date.day);
  fprintf (f, "UTC Time   : %02d:%02d:%02d.%03d\n",
    ut.time.hour, ut.time.minute, ut.time.second, ut.time.msecs);
  fprintf (f, "\n");

  // Print local timezone
  int tzHours = tzDelta.dHours;
  int tzMinutes = tzDelta.dMinutes;
  if (tzSubtract) {
    tzHours = -tzHours;
  }
  fprintf (f, "Local timezone : %d h %d m\n", tzHours, tzMinutes);
  fprintf (f, "\n");

  // Printe local date/time
  SDateTime dt = GetLocalDateTime ();
  fprintf (f, "Local Date : %04d/%02d/%02d \n", dt.date.year + 1900, dt.date.month, dt.date.day);
  fprintf (f, "Local Time : %02d:%02d:%02d.%03d\n",
    dt.time.hour, dt.time.minute, dt.time.second, dt.time.msecs);
  fprintf (f, "\n");
}

