/*
 * FlyV1Subsystems.cpp
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
// CThrottleControlV1
//
CThrottleControlV1::CThrottleControlV1 (void)
{
  TypeIs (SUBSYSTEM_THROTTLE_CONTROL_V1);
}


//=====================================================================
// CMixtureControlV1
//====================================================================
CMixtureControlV1::CMixtureControlV1 (void)
{
  TypeIs (SUBSYSTEM_MIXTURE_CONTROL_V1);
}


//
// CPropellerControlV1
//
CPropellerControlV1::CPropellerControlV1 (void)
{
  TypeIs (SUBSYSTEM_PROPELLER_CONTROL_V1);
}


//====================================================================
// CPrimeControlV1
//====================================================================
CPrimeControlV1::CPrimeControlV1 (void)
{
  TypeIs (SUBSYSTEM_PRIME_CONTROL_V1);
}


//====================================================================
// CEngineAntiIceV1
// NOT YET IMPLEMENTED
//====================================================================
CEngineAntiIceV1::CEngineAntiIceV1 (void)
{
  TypeIs (SUBSYSTEM_ENGINE_ANTI_ICE_V1);
}


//
// CIdleControlV1
//
CIdleControlV1::CIdleControlV1 (void)
{
  TypeIs (SUBSYSTEM_IDLE_CONTROL_V1);
}
