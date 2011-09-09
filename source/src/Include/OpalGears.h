/*
 * OpalGears.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2000-2004 Chris Wallace
 * Copyright 2005 Laurent Claudet
 * Copyright 2007 Jean Sabatier
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
 */


#ifndef OPALGEARS_H
#define OPALGEARS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "../Include/Gears.h"
//===================================================================================================
//====================================================================================
//  Opal Suspension
//====================================================================================
class COpalSuspension : public CSuspension {

public:
  COpalSuspension            (CVehicleObject *v, char *name, CWeightManager *wgh, char = TRICYCLE);
  virtual ~COpalSuspension   (void);

  ///< CStreamObject methods
  ///< =====================
  //virtual void   Write         (SStream *stream);

  ///< CSuspension methods
  ///< ===================
  void Timeslice         (float dT);
  void Debug             (void);


};
//========================================================================================
//  OPAL Ground suspension
//========================================================================================
class COpalGroundSuspension : public CGroundSuspension {

public:
  COpalGroundSuspension                (CVehicleObject *v,char* whlFilename, CWeightManager *wghman);
  virtual ~COpalGroundSuspension       (void);

  /*! CStreamObject methods */
  virtual int   Read                   (SStream *stream, Tag tag);
  virtual void  ReadFinished           (void);
  //virtual void  Write                      (SStream *stream);

  /*! CGroundSuspension generic methods */
  virtual void  Timeslice              (float dT);

private:
  double max_wheel_height_backup;
};
//=================================================================================
//  Class gear managed under opal-ode
//=================================================================================
class CGearOpal : public CGear {
public:
  CGearOpal                      (CVehicleObject *v,CSuspension *s);
  virtual ~CGearOpal             (void);
  ///< =============
  /*! gear compression value calc in ft : Timesliced */
  char           GCompression (char p);
  /*! gear compression velocity : Timesliced
   *  With the gear compression length We can get the compression velocity
   *  (used to determine the damping force)*/
  void           GComprV_Timeslice (void);
  /*! compute the vector force on the wheel in pound-force lbf 
   * (using square-law damping for Y) : Timesliced */
  void           VtForce_Timeslice (float dT);
  /*! compute the vector lateral forces on the wheel in pound-force lbf*/
  void        DirectionForce_Timeslice (float dT);
  /*! compute the vector brake forces on the wheel in pound-force lbf*/
	void				BrakeForce(float dT);
  /*! transform the forces back to the body frame : Timesliced
   * therefore the reference should be 'body' */
  void           GearL2B_Timeslice (void);
  /*! compute the vector moment for this wheel in lb.ft: Timesliced */
  void          VtMoment_Timeslice (void);
  /*! verify whether or not it has occured a crash */
  bool                  ResetCrash();
  void                  ResetForce();
  ///< Utilities
  ///< =========
   
  /*! moment in lb.ft */
  const SVector& GetBodyGearMoment     (void) {return vMoment;}    
  /*! force in Newton */
  const SVector& GetBodyGearForce_ISU  (void);    
  /*! moment in Kg.m */
  const SVector& GetBodyGearMoment_ISU (void);
  /*! */
  void InitJoint(char type, CGroundSuspension *s);
  /*! */ 
  //-------------------------------------------------------------------
  void    Repair();
  void    GetGearPosition(CVector &mp,double  &rad);
  //-----Probe datas --------------------------------------------------
	void    Probe(CFuiCanva *cnv);
  void    ProbeBrake(CFuiCanva *cnv);
	void    TraceForce();
  //-----Attributes ---------------------------------------------------
protected:
  //-------------------------------------------------------------------
  ///< CGear members
  ///< =============
  opal::SphereShapeData box;
  double    cMass;                    //  Mass repartition coefficient
  double    Radius;                   // In meters (rim + tire)
  //--------------------------------------------------------------------
	double    speed;										// Rolling speed
  float     bad_pres_resis;
  float     side_whl_vel;
  float     rolling_force,
            side_force;
  float     diffK;
  opal::Force      glf;                       ///< linear force
  opal::Force      gt_;
  CVector vb[2];        ///< Acceleration in body fram (m/s^2)
  CVector ab[2];        ///< Acceleration in body fram (m/s^2)
  int prv;
  int cur;
  opal::Point3r main_pos;
  opal::Vec3r local_velocity,
              body_velocity;
  float  damp_ground_rot; ///< used to fix roll on ground
};
//=================================================================================
//  Class gear managed under opal-ode
//  JS:  This class will be deleted as the CGearOpal can manage botth types
//=================================================================================
class CTailGearOpal : public CGearOpal {
public:
  CTailGearOpal                            (CVehicleObject *v,CSuspension *s);
  virtual ~CTailGearOpal                   (void);

  /*! gear compression value calc in ft : Timesliced */
  virtual char           GCompression (char p);
  virtual void           VtForce_Timeslice (float dT);
  /*! compute the vector lateral forces on the wheel in pound-force lbf*/
  virtual void            DirectionForce_Timeslice (float dT);
  /*! transform the forces back to the body frame : Timesliced
   * therefore the reference should be 'body' */
  virtual void            GearL2B_Timeslice (void);
  /*! compute the vector moment for this wheel in lb.ft: Timesliced */
  virtual void            VtMoment_Timeslice (void);
  /*! */

};
//=========================END OF FILE ==============================================================
#endif // OPALGEARS_H

