/*
 * FrameRateTracker.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005 Chris Wallace
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
#include "../Include/FlyLegacy.h"
#include "../Include/Collisions.h"
#include "../Include/globals.h"
//================================================================================
//  Colision detector based on Opal/Ode
//================================================================================
CrashDetector::CrashDetector()
{
}
//-----------------------------------------------------------------
//  Process one event
//-----------------------------------------------------------------
void CrashDetector::handleCollisionEvent(const opal::CollisionEvent& e )
{ if (globals->vehOpt.Not(VEH_D_CRASH)) return;
  opal::Solid *pln  = e.thisSolid;
  opal::Solid *oth  = e.otherSolid;
  opal::Point3r pnt = e.pos;
  CVehicleObject *veh = globals->sit->uVeh;
  CVector p(-pnt.x, -pnt.y, -pnt.z);
  switch (e.uType)  {
      case SHAPE_BODY:
        {
        U_INT fr = globals->sit->GetFrameNo();
        TRACE("%06d: BODY CONTACT x=%.4f y=%.4f z=%.4f",fr,p.x,p.y,p.z);
        }
        veh->BodyCollision(p);
        break;
      //-- Gear ignore collision -----------------
      case SHAPE_GEAR:
        { //CGear *gr = (CGear*)e.uData;
          return;
        }
  }
  return;
}

//======================END OF FILE ==============================================