/*
 * HelicopterSubsystems.cpp
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

using namespace std;

//
// CRotorSubsystem
//
CRotorSubsystem::CRotorSubsystem (void)
{
  TypeIs (SUBSYSTEM_ROTOR);
}

//
// C407DigitalIndicators
//
C407DigitalIndicators::C407DigitalIndicators (void)
{
  TypeIs (SUBSYSTEM_407_DIGITAL_INDICATORS);
}

//
// CHookSubsystem
//
CHookSubsystem::CHookSubsystem (void)
{
  TypeIs (SUBSYSTEM_HOOK);
}

//
// CThrustPitchControl
//
CThrustPitchControl::CThrustPitchControl (void)
{
  TypeIs (SUBSYSTEM_THRUST_PITCH_CONTROL);
}

//
// CLonCyclicControl
//
CLonCyclicControl::CLonCyclicControl (void)
{
  TypeIs (SUBSYSTEM_LON_CYCLIC_CONTROL);
}

//
// CLatCyclicControl
//
CLatCyclicControl::CLatCyclicControl (void)
{
  TypeIs (SUBSYSTEM_LAT_CYCLIC_CONTROL);
}

//
// CTailRotorControl
//
CTailRotorControl::CTailRotorControl (void)
{
  TypeIs (SUBSYSTEM_TAIL_ROTOR_CONTROL);
}

//
// CCollectiveControl
//
CCollectiveControl::CCollectiveControl (void)
{
  TypeIs (SUBSYSTEM_COLLECTIVE_CONTROL);
}

//
// CLonCyclicTrimControl
//
CLonCyclicTrimControl::CLonCyclicTrimControl (void)
{
  TypeIs (SUBSYSTEM_LON_CYCLIC_TRIM_CONTROL);
}

//
// CLatCyclicTrimControl
//
CLatCyclicTrimControl::CLatCyclicTrimControl (void)
{
  TypeIs (SUBSYSTEM_LAT_CYCLIC_TRIM_CONTROL);
}

//
// CTailRotorTrimControl
//
CTailRotorTrimControl::CTailRotorTrimControl (void)
{
  TypeIs (SUBSYSTEM_TAIL_ROTOR_TRIM_CONTROL);
}

//
// CCollectiveTrimControl
//
CCollectiveTrimControl::CCollectiveTrimControl (void)
{
  TypeIs (SUBSYSTEM_COLLECTIVE_TRIM_CONTROL);
}
