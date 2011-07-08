/*
 * FrameManager.h
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


/*! \file FrameManager.h
 *  \brief Classes used to transform coordinates across the earth - local - body frames

    The coordinate system for ifpos have its  origin to the centre of the earth,
    the z-axis pointing towards the north pole, and  the x- and y-axis pointing
    towards the equator. The coordinate system is fixed in  space (the x-axis could
    for example always point towards the sun) and earth (and  Fly!'s coordinate
    system) rotates around the z-axis. This means any fixed point on earth will
    rotate about the ifpos z-axis, and any point stationary in ifpos coordinates
    will be moving very fast relative to the surface of the earth (Concorde going west). 

    - ipos origin is centre of earth, direction is fixed in space (i.e. stars  do not move) 
    - epos origin is centre of earth, x-axis point to 0.0N0.0E, y-axis point  to 0.0N90.0E, z-axis points to north pole 
    - ipos coordinate system was aligned with epos when simulation started 
    - t is the time since the simulation started in seconds 

 */


#ifndef FRAMEMANAGER_H_
#define FRAMEMANAGER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Include/Utility.h"
#include "../Include/3DMath.h"



/*! \class CInertialFrame
    \brief The earth frame

 */
class CInertialFrame {
  
private:
	void Init();
	void Copy(const CInertialFrame &aCopy) {Init();}
  void UpdateEarthRotation (void);

public:
  /**
   * Constructor
   */
  CInertialFrame (void);
  /**
   * Destructor
   */
  virtual ~CInertialFrame (void);

  // setters
  void  SetInitPos (void);
	// getters
  CVector&       GetInertialPos         (void) {return ifpos_;}
  void           GetEarthRotationSinCos (void);
  const float&   GetSinAE               (void) {return sin_aE;}
  const float&   GetCosAE               (void) {return cos_aE;}

	// Clone
	CInertialFrame Clone(void) {CInertialFrame c; c.Copy(*this); return c;}
	// Operator =
	CInertialFrame& operator=(const CInertialFrame &aCopy) {Copy(aCopy); return *this;}

  // memeber functions
  void           geop2ipos              (const SPosition &pos, CVector &ipos);

public:
  CVector ifpos_;
  float   total_t;                          ///< time since we started
  float   aE;                               ///< angle rotated by earth since the sim began
  float   sin_aE, cos_aE;                   ///< trig aE angles in rad 
};

/*! \class CLocalFrame
    \brief the <geop> frame
 */
class CLocalFrame {

private:
	void Init(void);
	void Copy(const CLocalFrame &aCopy) {Init();}

public:
  /**
   * Constructor
   */
	CLocalFrame(void);
  /**
   * Destructor
   */
	virtual ~CLocalFrame(void);

  // setters
	// getters
  const CVector& GetLocalPos (void) {return lpos_;}

	// Clone
	CLocalFrame Clone(void) {CLocalFrame c; c.Copy(*this); return c;}
	// Operator =
	CLocalFrame& operator=(const CLocalFrame &aCopy) {Copy(aCopy); return *this;}

  // memeber functions
  void ipos2geop  (const CVector &ipos, SPosition &geop);
  void init_geop  (CVector &_ifpos) {if_.SetInitPos (); _ifpos = if_.GetInertialPos ();}
  void euler2iang (const CVector &eulerAngle, CVector &tmp_iang);
  void euler2iang (const CVector &eulerAngle, const SPosition &pos, CVector &tmp_iang);

private:
  CVector lpos_;
  CInertialFrame if_;
};

/*! \class CBodyFrame
    \brief The body frame
 */
class CBodyFrame {

private:
	void Init(void);
	void Copy(const CBodyFrame &aCopy) {Init();}

public:
  /**
   * Constructor
   */
	CBodyFrame(void);
  /**
   * Destructor
   */
	virtual ~CBodyFrame(void);

  // setters
	// getters

	// Clone
	CBodyFrame Clone(void) {CBodyFrame c; c.Copy(*this); return c;}
	// Operator =
	CBodyFrame& operator=(const CBodyFrame &aCopy) {Copy(aCopy); return *this;}

public:
  CVector bpos_;
};
#endif // FRAMEMANAGER_H_
