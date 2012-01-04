/*
 * VehicleStateSubsystems.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
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

#include "../Include/Subsystems.h"
#include "../Include/UserVehicles.h"
#include "../Include/WorldObjects.h"
using namespace std;

//
// CHistory
//
CHistory::CHistory (void)
{
  TypeIs (SUBSYSTEM_HISTORY);
}

//
// COnGroundMonitor
//
COnGroundMonitor::COnGroundMonitor (void)
{
  TypeIs (SUBSYSTEM_ON_GROUND_MONITOR);
}

//
// CAltitudeMonitor
//
CAltitudeMonitor::CAltitudeMonitor (void)
{
  TypeIs (SUBSYSTEM_ALTITUDE_MONITOR);
}

//
// CSpeedMonitor
//
CSpeedMonitor::CSpeedMonitor (void)
{
  TypeIs (SUBSYSTEM_SPEED_MONITOR);
}

//
// CStallWarning
//
CStallWarning::CStallWarning (void)
{
  TypeIs (SUBSYSTEM_STALL_WARNING);
 *wing = 0;
  enableStateAnnouncer = false;
}

int CStallWarning::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'wing':
    // Wing section name
    ReadString (wing, 64, stream);
    rc = TAG_READ;
    break;
  case 'annS':
    // Enable state announcer
    enableStateAnnouncer = true;
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CDependent::Read (stream, tag);
  }

  return rc;
}

//
// CFastSlowMeter
//
CFastSlowMeter::CFastSlowMeter (void)
{
  TypeIs (SUBSYSTEM_FAST_SLOW_METER);
}

//
// CPressurization
//
CPressurization::CPressurization (void)
{
  TypeIs (SUBSYSTEM_PRESSURIZATION);

  switchCabinPressure = false;
  switchPressureControl = false;
  diffPressure = 0;
  warningAlt = 0;
}

int CPressurization::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'swPC':
    // Cabin pressure control valve shut
    switchPressureControl = true;
    rc = TAG_READ;
    break;
  case 'swCP':
    // Cabin pressure switch on
    switchCabinPressure = true;
    rc = TAG_READ;
    break;
  case 'difP':
    // Maximum differential pressure
    ReadFloat (&diffPressure, stream);
    rc = TAG_READ;
    break;
  case 'warn':
    // Warning altitude
    ReadFloat (&warningAlt, stream);
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CDependent::Read (stream, tag);
  }

  return rc;
}

//
// CAccelerometer
//
CAccelerometer::CAccelerometer (void)
{
  TypeIs (SUBSYSTEM_ACCELEROMETER);
}

//====================================================================================
// CGearLight
//====================================================================================
CGearLight::CGearLight (void)
{ TypeIs (SUBSYSTEM_GEAR_LIGHT);
  hwId  = HW_LIGHT;
  mode  = 0;
  gNum  = 0;
}
//---------------------------------------------------------------------
// return mode
//---------------------------------------------------------------------
char CGearLight::ClampMode(char m)
{ if (m < GEAR_EXTENDED)    return GEAR_EXTENDED;
  if (m > GEAR_IN_TRANSIT)  return GEAR_IN_TRANSIT;
  return m;
}
//---------------------------------------------------------------------
//  Read Parameters
//---------------------------------------------------------------------
int CGearLight::Read (SStream *stream, Tag tag)
{ int nbr;
  switch (tag) {
    case 'mode':
      // Monitor mode
      ReadInt (&nbr, stream);
      mode = ClampMode(nbr);
      return TAG_READ;
    case 'gNum':
      // Gear system number
      ReadInt (&nbr, stream);
      if (nbr < 0)  nbr = 1;
      nbr--;
      gNum = nbr;
      return TAG_READ;
  }

  return CDependent::Read (stream, tag);
}
//---------------------------------------------------------------------
//  All parameters are read
//---------------------------------------------------------------------
void CGearLight::ReadFinished()
{ vlod  = mveh->GetLOD();
  CDependent::ReadFinished();
}
//---------------------------------------------------------------------
//  Time slice
//---------------------------------------------------------------------
void CGearLight::TimeSlice(float dT,U_INT FrNo)
{ CDependent::TimeSlice(dT,FrNo);
  char pos = vlod->WheelPosition(gNum);
  active  &= (pos == mode);
  return;
}
//====================================================================================
// CGearWarning
//====================================================================================
CGearWarning::CGearWarning (void)
{
  TypeIs (SUBSYSTEM_GEAR_WARNING);

  throttleLimit = 0;
  flapLimit = 0;
  odd = false;
}

int CGearWarning::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case '_tla':
    // Throttle lever limit
    ReadFloat (&throttleLimit, stream);
    rc = TAG_READ;
    break;
  case 'flap':
    // Flap angle limit (deg)
    ReadFloat (&flapLimit, stream);
    rc = TAG_READ;
    break;
  case '_odd':
    // Gear position and lever agreement
    odd = true;
    rc = TAG_READ;
    break;
  }

  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CDependent::Read (stream, tag);
  }

  return rc;
}

//
// CStallIdent
//
CStallIdent::CStallIdent (void)
{
  TypeIs (SUBSYSTEM_STALL_IDENT);
}

//
// CTempAirspeedDisplay
//
CTempAirspeedDisplay::CTempAirspeedDisplay (void)
{
  TypeIs (SUBSYSTEM_TEMP_AIRSPEED_DISPLAY);
}
