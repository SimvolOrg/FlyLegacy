/*
 * AeroModel.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright (c) 2004 Chris Wallace
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

/*! \file AeroModel.h
 *  \brief Defines classes used in the aerodynamic model.
 */
#ifndef AERO_MODEL_H_
#define AERO_MODEL_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Include/FlyLegacy.h"
#include "../Include/Utility.h"
#include "../Include/ModelACM.h"
#include <string>
#include <vector>
#include <map>
//=====================================================================================
class CAeroControlChannel;
class CAerodynamicModel;
class CAeroWingSection;
class CAcmFlap;
class CAeroMovingPart;
//=====================================================================================
//  Structure for coefficients
//=====================================================================================
struct AERO_ADJ {
    float kf;                         // Force adjustment
    float kd;                         // Drag  adjustment
    float km;                         // Moment adjustement
};
//=====================================================================================
//
// Aerodynamic Model
//
// The WNG file contains the aerodynamic flight model for the aircraft.
//
//=====================================================================================
/*!
 * The CAeroAirfoil class represent an airfoil (cross-) section.
 * It contains the aerodynamic properties of the section.
 */
class CAeroAirfoil : public CStreamObject
{
public:
   CAeroAirfoil (CAerodynamicModel *a);
  ~CAeroAirfoil (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // Luc's comment : Adding dragParasite support
  // Initialize the Airfoil for aerodynamic calculations
  void Setup(CAerodynamicModel *wng);

  // CAeroAirfoil methods
  const char*     GetAirfoilName () {return name;};
  const double    GetLiftCoefficient(double aoa, double mach);
  const double    GetInducedDragCoefficient(double aoa, double mach);
  const double    GetMomentCoefficient(double aoa, double mach);
  // Luc's comment : Adding dragParasite support
  const double    GetParasiteDragCoefficient (void);

  const float&    GetAoAMin (void) {return stallAlphaMin;}
  const float&    GetAoAMax (void) {return stallAlphaMax;}
	//---------------------------------------------------------
	char						GetTrace()			{return T;}
	//---------------------------------------------------------
protected:
	char						T;							// Trace indicator
	CAerodynamicModel *aero;
  char						*name;           ///< Airfoil name
  float           stallAlphaMin;  ///< Minimum stall AOA (radians)
  float           stallAlphaMax;  ///< Maximum stall AOA (radians)
  // Note on mdrag :(For now this is induced drag + parasite drag. 
  // This may be changed to just contain induced drag in the future)
  CFmtxMap        *mlift;         // Lift coef = f(AOA)
  CFmtxMap        *mdrag;         // Drag coef = f(AOA)
  CFmtxMap        *mmoment;       // Moment = f(AOA)   
  CFmtxMap        *mliftMach;     // Lift gain = f(Mach)
  CFmtxMap        *mdragMach;     // Drag gain = f(mach)
  CFmtxMap        *mmomentMach;   // Moment gain =f(mach)
  // Luc's comment : Adding dragParasite support
  float           parasiteDrag;   ///< Parasite drag coefficient needs to be stored separately
};

//==================================================================================
//	TRACE LEVEL
//==================================================================================
#define AERO_LNONE (0)
#define AERO_LWING (1)
#define AERO_LFLAP (2)
#define AERO_LTRIM (3)
#define AERO_LSPLR (4)
#define AERO_LVALL (5)
//==================================================================================
//
// A CAeroWingSection objects represents a part of an aerodynamic surface,
// ie. it is a part of a wing or tail.
//
// JS NOTE:  A CAcmFlap object is added to support the flap animation when
//          the wing has such feature (a Flap object)
//================================================================================
class CAeroWingSection : public CStreamObject {
  friend class CAerodynamicModel;

public:
  // Constructors / destructor
  CAeroWingSection (CVehicleObject *v,char* name);
 ~CAeroWingSection (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  
  void  SetWingChannel(CAeroControlChannel *aero);
  /*! Initialize the WingSection for aerodynamic calculations */
  void  Setup(CAerodynamicModel *wng);
  void  GetChannelValues();          // Read control surface
  /*! Compute the total aerodynamic force and moment.
   *  Parameters: true airspeed vector v, angular velocity omega
   * and distance to ground hAgl */
  // Luc's comment : Modifying ComputeForces()
  void  ComputeForces(SVector &v, double rho, double soundSpeed, double hAgl);
	//---------------------------------------------------------------------
	void	PhyCoef(char *name,AERO_ADJ &itm);
	//---------------------------------------------------------------------
  /// Get the results from the latest call to computeforces
  const SVector& GetLiftVector() const;
  const SVector& GetDragVector() const;
  const SVector& GetMomentVector() const;

  ///--- Getters -------------------------------------------------
  const double&  GetAoA              (void) const {return aoa;}
  const double&  GetCL               (void) const {return cl;}
  const double&  GetCDI              (void) const {return cdi;}
  const double&  GetCM               (void) const {return cm;}
  const CVector& GetSpeedVector      (void) const {return speedVector;}
  const double&  GetWingIncidenceDeg (void) const {return bAng.x;}
  //--------------------------------------------------------------------
  void  AddFlap(char *name)		{mflap->AddFlap(name);}
	bool	IsRudder()	{return (strcmp(name,"wing Tail w/rudder") == 0);}
	
  //--------------------------------------------------------------------
protected:
  //--------------------------------------------------------------------
  CVehicleObject           *mveh;             // Mother Vehicle
  CAcmFlap                 *mflap;            // Flap moving parts
  char                     *name;             ///< WingSection name
  char                     *foil;             ///< Airfoil name
  float                     area;             ///< Wing area (sq. m.)
  float                     span;             ///< Wing span (m.)
  CVector                   bPos;             ///< Aerodynamic center
  CVector                   bAng;             ///< Incidence angles (deg)
  bool                      orie;             ///< Control surface oriention (H/V)
  bool                      grnd;             ///< Ground effect enabled
  float                     geff;             ///< Ground effect factor
  float                     gAGL;             ///< Ground effect altitued (m)
  float                     idcf;             ///< Ice drag coefficient
  float                     effectiveLift;    ///< Effective Lift coefficient
  float                     effectiveDrag;    ///< Effective Lift coefficient
  float                     effectiveMoment;  ///< Effective Moment coefficient
  float                     hinge;            ///< Flap deployment moment effect
  CDamageModel*             damage;           ///< Wing damage data
  float                     load;             ///< Loading damage rate (damage/sec)
  double                    aoa;              ///< angle of attack
  double                    cl;               ///< 
  double                    cdi;              ///< 
  double                    cm;               ///< 
  double                    col_;             ///<          
  double                    cod_;             ///<
  double                    com_;             ///<
  CVector                   speedVector;      ///< 
  CFmtxMap                 *mflpS;            // Flap speed
	//----------------------------------------------------------------------------
  char											T;								// Trace level
  //----------------------------------------------------------------------------
  std::map<std::string,CAeroMovingPart*> flapMap;    ///< Flap data
  std::map<std::string,CAeroMovingPart*> spoilerMap; ///< Spoiler data
  std::map<std::string,CAeroMovingPart*> trimMap;    ///< Trim data
  //------------------------------------------------------------------------------
  // aerodynamic calculation data
	CAerodynamicModel  *aero;
  CAeroAirfoil*  airfoil;        ///< Airfoil object, direct ptr to avoid map lookup
  double              chord;          ///< Average chord in meters
  CRotationMatrixBHP  bAngMatrix_bhp; ///< Wingsection orientation in matrix form
  CRotationMatrixHPB  bAngMatrix_hpb; ///< Wingsection orientation in matrix form
  SVector             liftVector;     ///< Resultant lift force vector
  SVector             dragVector;     ///< Resultant drag force vector
  SVector             momentVector;   ///< Resultant moment vector
};
//===================================================================================
//
// A CAeroMovingPart object represents a trailing edge flap, a trimmer or
// a spoiler attached to a WingSection
//
//===================================================================================
class CAeroMovingPart : public CStreamObject
{
public:
  // Constructor
  CAeroMovingPart ( CVehicleObject *veh,CAeroWingSection *w);
 ~CAeroMovingPart();
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CAeroMovingPart methods
  const char* GetChannelName (void);
  float       GetLiftInc (void);            ///< Return lift coeff increment
  float       GetDragInc (void);            ///< Return drag coeff increment
  float       GetMomentInc (void);          ///< Return moment coeff increment
  void        Print3D (void);
  void        ReadChannel();
  //---------------------------------------------------------------------------
  void				Store(CAeroControlChannel *p)		{chan = p;}
  //---------------------------------------------------------------------------
protected:
	CVehicleObject       *mveh;								// Mother vehicle
  char                    type;							// Type of surface (F,S,T)
  CAeroWingSection  *wing;							// Mother wing
  CAeroControlChannel  *chan;								// Corresponding channel
  std::string           channel;            ///< Aero model channel name
  std::set<std::string> parts;              ///< Set of external model part names
  CFmtxMap             *mlift;              // Lift coef = f(deflec)
  CFmtxMap             *mdrag;              // Drag coef = f(deflec)
  CFmtxMap             *mmoment;            // Moment coef = f(deflec)
  bool                  invert;             ///< Invert animation keyframes
  float                 deflectRadians;     ///< Control surface deflection in radians
  AERO_ADJ              adj;                // Adjust coefficients
};
//====================================================================================
//
// The CAerodynamicModel is a container for the aerodynamic model of an aircraft.
//
//====================================================================================
class CAerodynamicModel : public CStreamObject {
public:
  // Constructors / destructor
  CAerodynamicModel ();
 ~CAerodynamicModel (void);
	//--- Init the aerodynamic model -----------------------
  void	Init(char* wngFilename);
	// CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CAerodynamicModel methods
  void  SetWingChannel(CAeroControlChannel *aero);
  // set up for aerodynamic calculations
  void Setup();

  /// Calculate aerodynamic force and moments.
  void Timeslice(float dT);
  /// Draw data over the aircraft (the call must be the last in TCacheMGR::Draw) 
  void DrawAerodelData (const double &lenght);
  inline void SetLogPointer (CLogFile *log_) {log = log_;}
  // lookup functons for airfoils and wingsections
  CAeroAirfoil* GetAirfoil(char *n);
  CAeroWingSection* GetWingSection(const std::string &name);
  SVector &GetForce();
  SVector &GetMoment();

protected:
	//---------------------------------------------------------------------
  CVehicleObject *mveh;   // Parent vehicle
  CVector   dofa;         ///< Vector distance from empty CG to aerodynamic datum (ft)
  float     laca;         ///< longitudinal aerodynamic center adjust
  bool      grnd;         ///< Global ground effect enabled
  float     geff;         ///< Global ground effect factor
  float     gAGL;         ///< Global ground effect altitude
  float     dplm;         ///< Dynamic pressure limit
  float     cd;           ///< Total form drag adjust
  float     ac;           ///< Longitudinal aerodynamic centre adjust
  bool      debugOutput;  ///< Enable real-time debug output
	//---------------------------------------------------------------------
	CAeroWingSection *rudder;				// Rudder section
	//---------------------------------------------------------------------
	char			T;						// Trace indicator
	char      wnam[64];			// Traced wing
	//---------------------------------------------------------------------
  std::map<std::string,CAeroAirfoil*>      airfoilMap; ///< Map of airfoil data
  std::map<std::string,CAeroWingSection*>  wingMap;    ///< Map of wing sections
	//---------------------------------------------------------------------
  // aerodynamic state data
  CVector force;
  CVector moment;
	float	  Time;
	//---------------------------------------------------------------------
public:
	void		SetVEH(CVehicleObject *v)	{mveh = v;}
  //--- logging ---------------------------------------------------------
  static CLogFile* log;
  static void LogVector(const SVector &v, const char* name);
  static void LogScalar(const double &d, const char* name);
	//---------------------------------------------------------------------
	float		GetTime()								{return Time;}
};
//=====================================================================================
// The PHY file contains the physics adjustments for the aircraft
// aeromodel or rolling model
// JS NOTE:  Add support for tag <chan> to allow force and drag coefficients
//  defined for all aero channels
//=====================================================================================
class CPhysicModelAdj : public CStreamObject
{
  //---ATTRIBUTES ---------------------------------------
  CVehicleObject *mveh;     // Parent vehicle
  //---METHODS ------------------------------------------
public:
   CPhysicModelAdj ();
  ~CPhysicModelAdj (void);
	 void Init(char* wngFilename);
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadChannel(SStream *st);
  void  ReadFinished (void);
  void  GetCoef(char *name,AERO_ADJ &itm);
  // CPhysicModelAdj methods

  //----ATTRIBUTES ---------------------------------------
  float Kdrg, /// coeff of drag
        Klft, /// coeff of lift
        Kmmt, /// coeff of moment
        Kdeh, /// dihedral coeff
        Krud, /// acrd fudge factor
        Ktst, /// thrust coeff
        Kpth, /// pitch coeff
        Pmin, /// pitch mine
        Rmin, /// roll mine
        Wlft, /// unused
        Ymin, /// yaw mine
        Kixx, /// propeller inertia
        KgrR, /// propeller gear ratio
        Kwnd, /// wind coeff on aircraft
        KfcP, /// P factor fudge factor
        mixC, /// mixture const
        Kegt, /// EGT coeff
        Ghgt, /// gear adjust const
        KdrG, /// drag from gear
        Kpmn; /// propeller magic number
  //----List of channel adjustment --------------------------
  std::map<std::string,AERO_ADJ*>  aero;
	//----------------------------------------------------------
	void SetVEH(CVehicleObject *v)	{mveh = v;}
};
//=========================END OF FILE =============================================
#endif // AERO_MODEL_H_
