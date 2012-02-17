/*
 * VehiclePosition.cpp
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

/*! \file VehiclePosition.cpp
 *  \brief Implements CDrawPosition rendering class
 *
 * The CDrawPosition draws the aircraft position above the terrain
 *   
 *   
 */

#include "../Include/Globals.h"
#include "../Include/GeoMath.h" 
#include "../Include/DrawVehiclePosition.h"
using namespace std;

//========================================================================
// CDrawPosition:  Init all parameters
//========================================================================
CDrawPosition::CDrawPosition (CVehicleObject *mv)
{ hwId  = HW_UNKNOWN;
  TypeIs (SUBSYSTEM_BASE);
  SetIdent('upos');
  on		= 0;
	mveh	= mv;
  //----------------------------------------------------------
  lw  = 1.5f;
  GetIniFloat ("Graphics", "drawPositionWidth", &lw);
  timer = 10.0f;
  delay = 0.35f;
  GetIniFloat ("Graphics", "drawPositionDelay", &delay);
  //-------------------------------------------------------
  int pm  = 0;
  GetIniVar ("Graphics", "drawPositionLine", &pm);
  line    = pm;
  //-------------------------------------------------------
  pm  = 2;
  GetIniVar   ("Graphics", "drawPositionVersion", &pm);
  version = pm;
  nb = 240;
  if (version) version -= 1;
  /// 0 = default vector / 1 = queue
  if (1 == version) {
    int nb_ = 240;
    GetIniVar ("Graphics", "drawPositionNb", &nb_);
    nb = static_cast <unsigned> (nb_);
  }
  if (0 == version) point_pos.clear ();
  on = mveh->GetOPT(VEH_DW_VPOS);
}
//--------------------------------------------------------------------
//  Destroy object
//--------------------------------------------------------------------
CDrawPosition::~CDrawPosition (void)
{	point_pos.clear();
}
//--------------------------------------------------------------------
//  Reset the points and toggle on/off
//--------------------------------------------------------------------
bool CDrawPosition::Reset (void)
{ U_INT p = mveh->GetOPT(VEH_DW_VPOS);
  if (p == on)  return (on == 0);
  if (0 == version) point_pos.clear ();
  on    = p;
  return (on == 0);
}
//---------------------------------------------------------------------
//    toggle request from other components
//---------------------------------------------------------------------
EMessageResult CDrawPosition::ReceiveMessage (SMessage *msg)
{ if (msg->id != MSG_SETDATA ) return CSubsystem::ReceiveMessage(msg);
  switch (msg->user.u.datatag) {
     //---- Return option ----------------------
      case 'togl':
          mveh->ToggleOPT(VEH_DW_VPOS);
          return MSG_PROCESSED;
  }
  return CSubsystem::ReceiveMessage(msg);
}
//--------------------------------------------------------------------
//  Draw the user position
//  Position is drawed only if required and the camera is external
//--------------------------------------------------------------------
void CDrawPosition::Draw (void)
{ if (Reset())              return;
  if (globals->noEXT)				return;
  SPosition upos = mveh->GetPosition();		//globals->geop;
  if (0 == version) {
    timer += globals->tim->GetDeltaSimTime ();
    if (timer > delay) {
      timer -= delay;
      if (!globals->slw->IsEnabled ())
        point_pos.push_back (upos);
    }

    glPushMatrix ();
    glColor3f (250.0f, 0.0f, 0.0f);
    glLineWidth (lw);

    glBegin (GL_LINES);
    bool flag = false;
    for (i_point_pos = point_pos.begin (); i_point_pos != point_pos.end (); ++ i_point_pos) {
      //---Compute feet coordinates relative to origin ---
      //globals->tcm->RelativeFeetTo (*i_point_pos, geol);
      geol = SubtractPositionInFeet (upos, *i_point_pos);
      if (line) {
        if (flag) {
          glVertex3d (geof.x, geof.y, geof.z);
          glVertex3d (geol.x, geol.y, geol.z);
        }
        if (!flag) flag = true;
        geof = geol;
      } else {
        glVertex3d (geol.x, geol.y, geol.z);
      }
    }
  } else

  if (1 == version) {
    timer += globals->tim->GetDeltaSimTime ();
    if (timer > delay) {
      timer -= delay;
      if (posQ.size () > nb) {
        oldQ.pop ();
      }
      oldQ.push (upos);
    }

    glPushMatrix ();
    glColor3f (250.0f, 0.0f, 0.0f);
    glLineWidth (lw);

    glBegin (GL_LINES);
    bool flag = false;
    while (!posQ.empty ()) {
      geol = SubtractPositionInFeet (upos, posQ.front ());
      if (line) {
        if (flag) {
          glVertex3d (geof.x, geof.y, geof.z);
          glVertex3d (geol.x, geol.y, geol.z);
        }
        if (!flag) flag = true;
        geof = geol;
      } else {
        glVertex3d (geol.x, geol.y, geol.z);
      }
      posQ.pop ();
    }
    posQ = oldQ;
  }

  glEnd ();
  glPopMatrix ();
}

//=========================END 0F FILE ====================================================
