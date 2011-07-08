/*
 * VehiclePosition.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
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
 *
 */


#ifndef VEHICLEPOSITION_H
#define VEHICLEPOSITION_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Include/FlyLegacy.h"
#include <queue>
//========================================================================================
//============================================================================
//  Class to draw the aircraft postion
//  JS: Make it an external subsystem like others
//============================================================================
class CDrawPosition: public CSubsystem {
protected:
  float timer;
  float delay;
  std::vector <SPosition> point_pos;
  std::vector <SPosition>::iterator i_point_pos;
  std::queue <SPosition> posQ, oldQ;
  //-----Geodata parameters ------------------------------------------
  /*! World position in arcsec */
  SPosition geop;
  /*! World position in feet */
  CVector   geof;
  /*! Local position in feet */
  CVector   geol;
  /*! */
  U_INT on;                       // On off indicator
  char line;                      // 0 = hashed / 1 = continued
  char version;                   // 1 = default vector / 2 = queued
  unsigned nb;                    // queue number of points
  float lw;                       // line width
  //----Public method -----------------------------------------------
public:
  CDrawPosition (void);
 ~CDrawPosition (void);
 //------------------------------------------------------------------
  EMessageResult ReceiveMessage (SMessage *msg);
  void  Draw  (void);
  bool  Reset (void);

};
//==========END OF THIS FILE ==================================================
#endif // VEHICLEPOSITION_H
