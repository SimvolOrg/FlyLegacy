/*
 * Gears.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2000-2004 Chris Wallace
 * Copyright 2005 Laurent Claudet
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
 */

//================================================================================================
#ifndef GEARS_H
#define GEARS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "../Include/FlyLegacy.h"
#include "../Include/BaseSubsystem.h"
//================================================================================
class	CRudderControl;
//================================================================================
//
// Ground Suspension and gears
//
// The WHL file contains the ground handling model parameters.
//

/*!
 * Based upon JSBSim code
 * see copyright below
 */

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

 Header:       FGLGear.h
 Author:       Jon S. Berndt
 Date started: 11/18/99

 ------------- Copyright (C) 1999  Jon S. Berndt (jsb@hal-pc.org) -------------

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free Software
 Foundation; either version 2 of the License, or (at your option) any later
 version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 details.

 You should have received a copy of the GNU General Public License along with
 this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 Place - Suite 330, Boston, MA  02111-1307, USA.

 Further information about the GNU General Public License can also be found on
 the world wide web at http://www.gnu.org.

HISTORY
--------------------------------------------------------------------------------
11/18/99   JSB   Created
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

class CWhl : public CSubsystem {
public:
  CWhl                             (void);
  ~CWhl                            (void) {;}

  ///< CStreamObject methods
  virtual int   Read                   (SStream *stream, Tag tag) = 0;
  virtual void  ReadFinished           (void)                     = 0;
  virtual void  Write                  (SStream *stream)          = 0;

  ///< CWhl methods
  virtual void  Timeslice                  (float dT)                  {;}
  virtual const SVector& GetForce          (void)          /* fake */  {vf.x  = vf.y  = vf.z  = 0.0; return vf;}
  virtual const SVector& GetMoment         (void)          /* fake */  {vf.x  = vf.y  = vf.z  = 0.0; return vf;}
  virtual const SVector& GetForce_ISU      (void)          /* fake */  {vf.x  = vf.y  = vf.z  = 0.0; return vf;}
  virtual const SVector& GetMoment_ISU     (void)          /* fake */  {vf.x  = vf.y  = vf.z  = 0.0; return vf;}
  virtual const SVector& GetBodyMoment     (void)          /* fake */  {vbf.x = vbf.y = vbf.z = 0.0; return vbf;}
  virtual const SVector& GetBodyForce_ISU  (void)          /* fake */  {vbf.x = vbf.y = vbf.z = 0.0; return vbf;}
  virtual const SVector& GetBodyMoment_ISU (void)          /* fake */  {vbf.x = vbf.y = vbf.z = 0.0; return vbf;}
  //--------------------------------------------------------------------------
  SVector vf, vbf;
};
//=====================================================================================
/*! \class
 * class separated from CSuspension for convenience .
 * Actually, all the gear calculation is placed here
 * while CSuspension is more intended as an interface
 */
//  JS:  Add a pointer to CSuspension for interface
//======================================================================================
class CGear {
//-----ATTRIBUTES ---------------------------------------------------
protected:
  CVehicleObject *mveh;               // Parent vehicle
  CSuspension    *susp;               // Associated suspension
  CVector vGearLoc2CG;               ///< in ft
  CVector vGearLoc2CG_ISU;           ///< in meters
  CVector vMoment;
  CVector vForce;
  CVector vMoment_ISU;
  CVector vForce_ISU;
  CVector vLocalMoment;
  CVector vLocalForce;               ///< vertical force on the wheel
  CVector vLocalMoment_ISU;
  CVector vLocalForce_ISU;
  //----------------------------------------------------------------
  SPosition WPos;                    ///< Wheel position relative to the body position in space
  CVector   vWhlVelVec;              ///< wheel velocity vector
  SGearData *gearData;

public :
  CGear (CVehicleObject *v,CSuspension *s);
  virtual ~CGear (void);
  ///< CGear methods
  ///< =============
  //--Gear position  and radius ------------------------
  virtual void GetGearPosition(CVector &mp,double  &rad) {;}
  /*! gear position according to the CG in ft : Timesliced */
  virtual void GearLoc_Timeslice (const SVector*, const SVector&);  
  /*! the Wheel position is transformed according to the ac body in the overall world
   *  therefore the reference should be 'local' : still in ft */
	virtual void GearB2L_Timeslice (void) {;}
  /*! gear compression value calc in ft : Timesliced */
  virtual char GCompression(char p) {return 0;}
  /*! gear compression velocity : Timesliced */
  virtual void GComprV_Timeslice (void) {};
  /*! compute the vector force on the wheel in pound-force lbf */
  virtual void VtForce_Timeslice (float dT) {;}
  /*! compute the vector lateral forces on the wheel in pound-force lbf */
  virtual void DirectionForce_Timeslice (float dT) {};
  /*! transform the forces back to the body frame : Timesliced
   * therefore the reference should be 'body' */
  virtual void GearL2B_Timeslice (void) {};
  /*! compute the vector moment for this wheel in lb.ft: Timesliced */
  virtual void VtMoment_Timeslice (void) {};
  
public:
  ///< Utilities
  ///< =========
  /*! wheel to CG in ft */
  const SVector* GetGear2CG        (void) {return &vGearLoc2CG;}    
  /*! wheel to CG in meters */
  const SVector* GetGear2CG_ISU    (void) {return &vGearLoc2CG_ISU;} 

  virtual void SetGearData  (SGearData *data) {gearData = data;}
  /*! reset values */
  void ResetValues       (void);


  /*! moment in lb.ft */
  virtual const SVector& GetBodyGearMoment     (void) {return vMoment;}    
  /*! force in Newton */
  virtual const SVector& GetBodyGearForce_ISU  (void);    
  /*! moment in Kg.m */
  virtual const SVector& GetBodyGearMoment_ISU (void);
  /*! launch init joint */
  virtual void InitJoint(char type,CSuspensionMGR *s) {;}
  //------ Probe functions ----------------------------------------
	virtual void    Probe(CFuiCanva *cnv) {;}
  virtual void    ProbeBrake(CFuiCanva *cnv) {;}
  virtual void    ResetForce()    {;}
  //---------------------------------------------------------------
  bool    WheelCollision(CVector &p);
  //------ Reset crash --------------------------------------------
  inline  void  ResetCrash()    {gearData->shok = 0; }
  //---------------------------------------------------------------
};

//====================================================================================
//  JS NOTE:  To support differential braking, a wheel position (left or rite) is added
//            when the object is a wheel
//=====================================================================================
class CSuspension: public CSubsystem { 
  ///< Fly! data script section 
  ///<
protected:
  CAcmGears        *whel;                             // Movinf part
  CAcmTire         *Tire;                             // Tire parts
  CVehicleObject   *mveh;                             // Parent vehicle
  CGear            *gear;
  SGearData         gear_data;
	//----------------------------------------------------------------
	ValGenerator	    amort;									
	//----------------------------------------------------------------
public:
  int           reset_crash;                          ///< allows to reset sim
  char          type;                                 // Type of suspension
  char          wInd;                                 // Wheel index
  //----Full constructor ----------------------------------------
  CSuspension (CVehicleObject *v, CSuspensionMGR *gssp, char *susp_name,  char type_ = TRICYCLE);
  virtual ~CSuspension             (void);
  void    InitGear();
  ///< CStreamObject methods
  ///< =====================
  virtual int    Read              (SStream *stream, Tag tag);
  virtual void   ReadFinished      (void);
  virtual void   Write             (SStream *stream);

  ///< CSuspension methods
  ///< ===================
  virtual void   Timeslice         (float dT);
  virtual void   Debug             (void);
  
  ///< getters & setters
  ///< ==================
  double  GetGearPosZ ()  {return gear_data.bPos.z;}
  double  GetGearPosY ()  {return gear_data.bPos.y;}
  double  GetGearPosX ()  {return gear_data.bPos.x;}
  double  GetGearRadius() {return gear_data.whrd;}
  /*! The Moment vector for this gear */
  virtual const SVector& GetBodyMoment     (void);
   ///< The Force vector for this gear */
  virtual const SVector& GetBodyForce_ISU  (void);
  /*! The Moment vector for this gear */
  virtual const SVector& GetBodyMoment_ISU (void);
  /*! */
  bool IsSteerWheel();
  void SetWheelBase(float b)        {gear_data.wheel_base = b;}
  //-----------------------------------------------------------------
  char  GearShock(char pw);
  void  GearDamaged(char nsk);
  //-------GetGear Position ----------------------------------------
  void  GetGearPosition(CVector &mp,double  &rad)    {gear->GetGearPosition(mp,rad);}
  //-------Define ground relation to wheels ------------------------
  void  InitGearJoint(char type,CSuspensionMGR *s) {gear->InitJoint(type,s);}
  void  ResetCrash()			{gear_data.shok = 0;}
  void  ResetForce()			{gear->ResetForce();}
	//-----------------------------------------------------------------
	void  Deflect(double d) {gear_data.deflect = d;}
  //----------------------------------------------------------------
  char**  GetProbeOptions();
  void    Probe(CFuiCanva *cnv);
	//----------------------------------------------------------------
	void		AnimateShock(float a)		{whel->SetShockTo(a);}
  void		SetShockTo(float v)     {whel->SetShockTo(v);}
	//----------------------------------------------------------------
	float		GetSwing(float dT)			{return amort.TimeSlice(dT);}
  //-----Wheel interface -------------------------------------------
  void		MoveWheelTo(float v,float dT)    {Tire->SetWheelVelocity(v,dT);}
	//----------------------------------------------------------------
  bool		IsOnGround()            {return (0 != gear_data.onGd);}
  //-----Gear interface --------------------------------------------
  void		SetIndex(char x)        {wInd = x;}
  void		SetWheelAGL(float f)    {whel->SetAGL(f);}
  void		TurnWheelTo(float d)    {whel->TurnTo(d);}
  void		PushWheelTo(float d)    {whel->PushTo(d);}
  void		PlayTire(int p)         {whel->PlayTire(p);}
	//-- Set ABS anti skid feature ---------------------
	void		SetABS(char p)					{gear_data.sABS	= p;}	
	void		SetAmplifier(double a)	{gear_data.ampBK = a;}
	//-----------------------------------------------------------------
	SGearData *GetGearData()			{return &gear_data;}
  //-----------------------------------------------------------------
  char		GetWheelIndex()         {return wInd;}
  //-----------------------------------------------------------------
};
//=====================================================================================
class CBumper : public CWhl {
public:
  CBumper                    (char *name_, CWeightManager *wghman, char type = TRICYCLE);
  virtual ~CBumper                   (void);

  ///< CStreamObject methods
  virtual int   Read         (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);
  virtual void  Write        (SStream *stream);

  ///< CBumper methods
  virtual void  Timeslice    (float dT);
  void          Debug        (void);

  //
private:
  CWeightManager  *weight_manager;

  ///< Fly! data script section 
  ///
  char bump_name[56];
  std::vector<std::string> vfx_;  ///< -- Visual Effects --
  // endf;                        //
  CDamageModel oy_T;              ///< -- Damage Entry --
  int   bmpF[2];                  ///< -- bumper force (normal -- friction)
  SVector mPos;                   ///< -- Contact Point (model coodinates) --
};


//=========================================================================================
/*! Data Structure / Class Descriptions
 * \brief Interface for wheels & suspension management
 * \detailed : manages suspensions, bumpers, and weight
 */
// JS NOTES:  To support differential braking, we must distinguish between left and right wheel(s)
//            When <susp> is a wheel, we have the following tags
//            <brak>  Implies that wheel has brake
//             +/-1   -1=>Left wheel while 1 => Right wheel
//   Brake systems (CGroundBrake) are made addressable by the message system 
//          tag 'brak'      This is a brake message
//          unit  0         This is for both  wheels
//          unit  1         This is for left  wheel(s)
//          unit  2         This is for right wheel(s)
//
//===============================================================================================
class CSuspensionMGR: public CSubsystem {           // : public CWhl {
public:
  CSuspensionMGR();
  virtual ~CSuspensionMGR       (void);

  /*! CStreamObject methods */
  virtual int   Read(SStream *stream, Tag tag);
  virtual void  ReadFinished(void);
	void	Init(CWeightManager *wghman, char *fn);
  /*! CSuspensionMGR generic methods */
  virtual void  Timeslice          (float dT);
  //-------------------------------------------------------------------------
	void	BuildGears();
  void  ReadSusp(SStream *st);
	void	SetABS(char p);
  void  ResetCrash();
  void  ResetForce();
	void	SetSteerData(CRudderControl *rud);
	void  DisconnectGear(CRudderControl *rud);
  //-------------------------------------------------------------------------
  /*! Getters - setters */
  const SVector* GetSumGearForces  (void)              {return &SumGearForces;}
  const SVector* GetSumGearMoments (void)              {return &SumGearMoments;}
  int             GetNumberOfWheels(void)              {return wheels_num;} //< body wheels num 
  /*! Set steering direction value */
  const double&  GetCurWheelHeight (void)              {return cur_wheel_H;}
  const double&  GetMainGearHeight (void)              {return mWPos;}
  //----------------------------------------------------------------------------------
	void	SetVEH(CVehicleObject *v)	{mveh = v;}
	//-------------------------------------------------------------------------
  void     GetAllWheels(std::vector<CSuspension *> &whl) { whl = whl_susp;}
  CVector *GetMainGearCenter()	{return &mainW;}
	double   GetDifBraking()			{return difB;}
	double	 GetBankCoef()				{return banK;}
	double   GetFriction()				{return fric;}
	double   GetBumpForce()				{return bump;}
 // double   GetMainGearRadius()	{return  mainR;}
  double   GetMinimumBodyAGL()	{return  mAGL;}
  double   GetPositionAGL()			{return  (mAGL + mainR - 1);}
  //double   GetSterGearRadius()	{return  sterR;}
	//-------------------------------------------------------------------------
	double   GetTurnCoefficient()	{return trad;}
	void	   SetTurnCoefficient(double c)	{trad = c;}
	//-------------------------------------------------------------------------
  void     StoreGearVM(CVector &v, double mc) {mainVM = v; massCF = mc;}
  char     GetNbWheelOnGround()	{return nWonG;}
  bool     WheelsAreOnGround()	{return (nWonG != 0);}
	bool			AllWheelsOnGround()	{return (nWonG == wheels_num);}
	//--- Steering interface -----------------------------------------------------------
	CSuspension *GetSteeringWheel()	{return steer;}
  //----------------------------------------------------------------------------------
  ///
  float                      rMas;                         ///< rated mass
  //------ATTRIBUTES -----------------------------------------------------------------
protected:
  CVehicleObject            *mveh;                           // Parent vehicle
  std::vector<CSuspension*>  whl_susp;
  std::vector<CWhl *>        whl_bump;
  CWeightManager            *whm;                      ///< a link to the CWeightManager
                                                           ///< for rated mass
  CVector             SumGearForces;
  CVector             SumGearMoments;
  char                type;                                 ///JS Type of suspension
  char                wInd;                                 // JS Wheel index
  CFmtxMap           *mstbl;                                // Steering table
  CFmtxMap           *mbtbl;                                // Brake table
  //------Wheel interface ------------------------------------------------------------
	CSuspension  *steer;																			// Steering wheel
  double max_wheel_H;																				//
	double cur_wheel_H;																				//  Current wheel height
  //----------------------------------------------------------------------------------
	double				bump;																					// Bump force
	double        ampB;																					// Brake amplifier
	double				difB;																					// Differntial brake
	double        banK;																					// Banking coefficient
	double				fric;																					// Friction coefficient
	double				trad;																					// Turn radius
  double        wheel_base;                                   // Inter axes distance
  double        mAGL;                                         // minimum Body AGL (in feet)
  double        mainR;                                        // Main average radius
  CVector       mainW;                                        // Main gear barycenter
  double        sterR;                                        // Steering wheel radius
  char          nWonG;                                        // Number wheels on ground
  //-----------------------------------------------------------------------------------
  int           wheels_num;                                   ///< body wheels num
  //------Main position depending of the tail type to compute moment -----------------
  CVector        mainVM;                                  // Main vector moment
  double         massCF;                                  // Mass coefficient
  //----------------------------------------------------------------------------------
  double        max_gear,
                min_gear, 
                mWPos;            // Main wheel position In feet

};

//==========================================================================================
//  class * Based upon JSBSim code
//   see copyright in cpp file
////========================================================================================
class CGearJSBSim : public CGear {
public:
  CGearJSBSim                      (CVehicleObject *v,CSuspension *s);
  virtual ~CGearJSBSim             (void);
  ///< CGearJSBSim methods
  ///< =============
  /*! gear compression value calc in ft : Timesliced
   *  JSBSim = the gear compression value is the local frame gear Y location value minus the height AGL
   *  Currently, we assume that the gear is oriented - and the deflection occurs in - the Y axis */
  char						GCompression (char p);
  /*! gear compression velocity : Timesliced
   *  With the gear compression length We can get the compression velocity
   *  (used to determine the damping force)*/
  void						GComprV_Timeslice (void);
  /*! compute the vector force on the wheel in pound-force lbf 
   * (using square-law damping for Y) : Timesliced */
  void						VtForce_Timeslice (float dT);
  /*! compute the vector lateral forces on the wheel in pound-force lbf*/
  void						DirectionForce_Timeslice (float dT);
  /*! transform the forces back to the body frame : Timesliced
   * therefore the reference should be 'body' */
  void						GearL2B_Timeslice (void);
  /*! compute the vector moment for this wheel in lb.ft: Timesliced */
  void						VtMoment_Timeslice (void);

  ///< Utilities
  ///< =========
   
  /*! moment in lb.ft */
  const SVector& GetBodyGearMoment     (void) {return vMoment;}    
  /*! force in Newton */
  const SVector& GetBodyGearForce_ISU  (void);    
  /*! moment in Kg.m */
  const SVector& GetBodyGearMoment_ISU (void);
  /*! */

private:
  ///< CGear members
  ///< =============

  float     bad_pres_resis;
  float     banking;
};

#endif // GEARS_H
//=======================END OF FILE =============================================================
