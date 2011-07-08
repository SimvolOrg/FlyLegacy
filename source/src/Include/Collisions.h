/*
 * Collisions.h
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
#ifndef COLLISIONS_H
#define COLLISIONS_H
//======================================================================
typedef enum 
{ SHAPE_BODY    = 1,
  SHAPE_GROUND  = 2,
  SHAPE_GEAR    = 3,
  } SHAPE_TYPE;
//======================================================================
//  Collision handler
//======================================================================
class CrashDetector: public opal::CollisionEventHandler {
  //------ Attributes ------------------------------------
  //------- Methods --------------------------------------
public:
  CrashDetector();
  //---Process event -------------------------------------
  void OPAL_CALL handleCollisionEvent(const opal::CollisionEvent& e );
};
#endif COLLISIONS_H
//=======================END OF FILE ======================================================================
