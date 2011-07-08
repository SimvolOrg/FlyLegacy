/*
 * VehicleSmoke.h
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

/***********************************
 *   AMP file
 *	  <subs>
 *	  smok
 *	  <bgno>
 *		  <colr>
 *		  0.98,0.745,0.785
 *		  <posn>
 *		  0.0,0.0,-15.0
 *	  <endo>
 ************************************
 *   PNL file
 *  <gage> ==== GAUGE ENTRY ====
 *  swit
 *  <bgno> ==== BEGIN GAUGE ENTRY ====
 *  	<unid> ---- unique id ----
 *  	phsw
 *  	<mesg> ---- Subsystem Message ----
 *  	<bgno>
 *  		<conn> ---- data tag ----
 *  		smok
 *  	<endo>
 *  	<size> ---- x,y,xsize,ysize ----
 *  	317
 *  	612
 *  	28
 *  	38
 *  	<bmap> ---- bitmap name ----
 *  	skyhpito.pbg
 *  	<csru> ---- up cursor ----
 *  	mpflipup.csr
 *  	<csrd> ---- down cursor ----
 *  	mpflipdn.csr
 *  	<sfxu> ---- up sfx ----
 *  	switchup.wav
 *  	<sfxd> ---- down sfx ----
 *  	switchdn.wav
 *  	<help> ---- popup help ----
 *  	smoke switch
 *  <endo> ==== END GAUGE ENTRY ====
 ************************************
 */

#ifndef VEHICLESMOKE_H
#define VEHICLESMOKE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Include/FlyLegacy.h"
#include <queue>
//========================================================================================
typedef struct {
  float xPos,yPos,zPos;
  float xVec,yVec,zVec;
  float r,g,b,life;
} SpriteInfo;
//========================================================================================
//  Smoke class
//========================================================================================
class CBaseSmoke {
public:
  GLuint Texture[128];           // Handles to our textures
  SpriteInfo *spr;               // Array of particles

  std::vector <SPosition> point_pos;
  std::vector <SPosition>::iterator i_point_pos;
  std::queue  <SPosition> posQ, oldQ;
  U_INT       on;                // On Off indicator
  //------------------------------------------------------------------
  CBaseSmoke            (int n);
  virtual ~CBaseSmoke   (void);
  void  Init();
  void  SetQuad(float dim);
  void  Draw            (void);
  bool  Reset           (void);
  void  FillStruct      (void);
  void  UpdateParticles (void);
  bool  LoadTexture     (char *texname, GLuint texhandle);
  //-----Smoke QUAD --------------------------------------------------
  TC_VTAB  quad[4];
  //-----Geodata parameters ------------------------------------------
  /*! World position in arcsec */
  SPosition geop;
  /*! World position in feet */
  CVector   geof;
  /*! Local position in feet */
  CVector   geol;
  /*! */
  float timer;
  float delay;
  int version;                    // 1 = default vector / 2 = queued
  unsigned nb;                    // queue number of points
  int nb_particles;               // number of particles
  float PARTICLE_SIZE,HALF_PARTICLE_SIZE;
  int NUM_PARTICLES;
  int INITIAL_PARTICLE_SPREAD;
  float SPEED_DECAY;              // (Gravity) 
  GLuint smoke_texture[1];
  int max_spread, max_particles, index;
  float spread, angle;
  bool draw;
  float ticks;
  int smk_t;                      // 1=one tga  2=two tga
  float smkpos_x,                 // smoke position
        smkpos_y,
        smkpos_z;
  float smkcolor_r,               // smoke color
        smkcolor_g,
        smkcolor_b;
};
///============================================================================
///  Class to draw the aircraft smoke
///============================================================================
class CVehicleSmoke: public CSubsystem {
protected:
  CBaseSmoke *bs;
  //---------------------------------------------
public:
  CVehicleSmoke (void);
 ~CVehicleSmoke (void);
 //-----------------------------------------------
  EMessageResult ReceiveMessage (SMessage *msg);
  void  Draw();
};

///============================================================================
///  Class to draw the aircraft smoke
// JS: Modified to make it a true CDependent.
//  SubsystemSmoke are entered into the sext list in ElectricalSystems. 
//  So they can be Time Sliced and drawed like any other.
///============================================================================
class CSubsystemSmoke : public CDependent {
protected:
  //---Attributes ---------------------------------------------------
  CBaseSmoke *bs;
  //---Methods ------------------------------------------------------
public:
  CSubsystemSmoke (void);
  virtual ~CSubsystemSmoke (void);

  virtual const   char* GetClassName (void) { return "CSubsystemSmoke"; }
  int     Read               (SStream *stream, Tag tag);
  void    ReadFinished       (void);
  EMessageResult  ReceiveMessage     (SMessage *msg);
  void    Draw               (void);
  //-------------------------------------------------------------------
  char    on;                 // On / Off switch
};
//==========END OF THIS FILE ==================================================
#endif // VEHICLESMOKE_H
