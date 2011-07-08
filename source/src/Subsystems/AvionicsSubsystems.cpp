/*
 * AvionicsSubsystems.cpp
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
// CDMEPanel
//
CDMEPanel::CDMEPanel (void)
{
  TypeIs (SUBSYSTEM_DME_PANEL);

  hold = false;
}

int CDMEPanel::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'hold':
    // Hold -- ??
    hold = true;
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
// CDMEPanel2
//
CDMEPanel2::CDMEPanel2 (void)
{
  TypeIs (SUBSYSTEM_DME_PANEL_1);
}

//
// CWeatherRadar
//
CWeatherRadar::CWeatherRadar (void)
{
  TypeIs (SUBSYSTEM_WEATHER_RADAR);
}

//
// CCollinsPFS
//
CCollinsPFD::CCollinsPFD (void)
{
  TypeIs (SUBSYSTEM_COLLINS_PFD);
}

//
// CCollinsND
//
CCollinsND::CCollinsND (void)
{
  TypeIs (SUBSYSTEM_COLLINS_ND);
}

//
// CCollinsAD
//
CCollinsAD::CCollinsAD (void)
{
  TypeIs (SUBSYSTEM_COLLINS_AD);
}

//
// CCollinsMND
//
CCollinsMND::CCollinsMND (void)
{
  TypeIs (SUBSYSTEM_COLLINS_MND);
}

//
// CCollinsRTU
//
CCollinsRTU::CCollinsRTU (void)
{
  TypeIs (SUBSYSTEM_COLLINS_RTU);
}

//
// CPilatusPFD
//
CPilatusPFD::CPilatusPFD (void)
{
  TypeIs (SUBSYSTEM_PILATUS_PFD);
}

//
// CVirtualGPS
//
CVirtualGPS::CVirtualGPS (void)
{
  TypeIs (SUBSYSTEM_VIRTUAL_GPS);
}

//
// CTCASPanel
//
CTCASPanel::CTCASPanel (void)
{
  TypeIs (SUBSYSTEM_TCAS_PANEL);
}

//
// CFMSys
//
CFMSys::CFMSys (void)
{
  TypeIs (SUBSYSTEM_UNIVERSAL_FMS);
}
