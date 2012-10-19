/*
 * Weather.h
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

/*! \file Weather.h
 *  \brief Defines all classes related to weather management such as :
 *         - CWindModel
 *         - CWeatherManager
 */

/// Used with CSituation::Timeslice
/// and InitGlobalsNoPodFilesystem

#ifndef WEATHER_H_
#define WEATHER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "FlyLegacy.h"
#include "3dMath.h"
//=====================================================================
class CmvalMap;
//=====================================================================
//==============FORWARD DECLARATIONS ==================================
class C_QGT;
//==================================================================
#define SURF_LAYER (0)
//==================================================================
//  INFO METAR
//==================================================================
struct METAR_INFO {
  char    iden[16];                         // Airport ident
  char    code;                             // return code
  SPosition pos;                            // Airport position
  //-----Wind parameters -----------------------------------------
  int     wDR1;                             // Wind direction 1
  int     wDR2;                             // Wind direction 2
  int     wASP;                             // Wind average speed
  int     wGSP;                             // Wind gust
  //-----Visibility ----------------------------------------------
  double  vMIL;                             // Visibility in miles
  //-----WEATHER -------------------------------------------------
  char    wSTR;                             // Strenght (+/-)
  Tag     wQAL;                             // Qualifier
  Tag     wTYP;                             // Type of weather
  //---- Cloud cover ---------------------------------------------
  int     Layer;                            // Cloud layer
  Tag     Cover;                            // Cloud cover
  int     Ceil;                             // Ceil
  int     VertV;                            // Vertical visibility
  //-----Temperature ---------------------------------------------
  float   Temp;                             // Temperature (°C)
  float   DewP;                             // Dew point   (°C)
  //-----Float altimeter -----------------------------------------
  double  Baro;                             // hPa
};
//============================================================================
//  Class wind layer
//  defines wind at a given altitude
//============================================================================
class CWindLayer {
  friend class CWeatherManager;
  //----Layer index -------------------------------------------
  char      nLayer;
  //--- wind parameters from metar bulletin --------------------
  float     windDR1;                  // Wind direction (degre)
  float     windDR2;                  // Second direction
  float     windKTS;                  // Wind speed
  float     windGUS;                  // Wind gust
  //-----------------------------------------------------------
  U_CHAR    windRDM;                  // Random wind gust
  //---METHODS --------------------------------------------------
public:
  CWindLayer(char no);
  void      SetUP(METAR_INFO &info);
  //-------------------------------------------------------------
  inline char   GetLayer()          {return nLayer;}
};
//============================================================================
//  Meteo area
//  NOTE:  Meteo area has only one wind layer  for surface
//============================================================================
class CMeteoArea  {
  friend class CCloudSystem;
  friend class CWeatherManager;
protected:
  Tag       Key;                      // Key (from airport ident)       
  char iden[16];                      // Airport designation
  SPosition Pos;                      // Area center
  float     Radius;                   // Area radius
  U_CHAR    Layer;                    // Cloud Layer
  float     Ceil;                     // Ceil
  //---SURFACE WIND LAYER -------------------------------------
  CWindLayer *wind;                   // surface wind layer
  //----Temperature -------------------------------------------
  float     Temp;                     // Area temperature
  float     Baro;                     // Barometer
  //-----------------------------------------------------------
  U_CHAR    windRDM;                  // Random wind gust
  //----METHODS -----------------------------------------------
public:
  CMeteoArea();
 ~CMeteoArea();
  //-----------------------------------------------------------
  void    SetFromMETAR(METAR_INFO &info);
  int     RangeOf(SPosition &ps);     // Check if area in range
  //-------------------------------------------------------------
  inline bool   SameLayer(U_CHAR n) {return (n == Layer);}
  inline SPosition  *Position()     {return &Pos;}
  inline CWindLayer *GetWindLayer() {return wind;}
  inline char  *GetIdent()          {return iden;}
  inline void   SetKey(Tag k)       {Key = k;}
  inline void   SetLayer(U_CHAR n)  {Layer = n;}
  inline void   SetCeil(float h)    {Ceil = h;}
  inline Tag    GetKey()            {return Key;}
  inline float  GetCeil()           {return Ceil;}
  inline U_CHAR GetLayer()          {return Layer;}
  inline bool   NoLayer()           {return (Layer == 0);}
};
//==================================================================
// CWeatherManager
//
//==================================================================
class CWeatherManager: public CExecutable {
protected:
  SWeatherInfo    infoW;
  SWindsAloft     user_wind;
  //-----Demux --------------------------------------------------
  U_CHAR          demux;
  U_CHAR          tr;                       // Trace option
	U_CHAR					nw;												// No wind option
  //-----DECODING AREA ------------------------------------------
  CAirport       *Apt;                      // Airport involved
  METAR_INFO      Info;
  //-----DEFAULT VALUES -----------------------------------------
  float           Radius;                   // Cloud radius in n.miles
  //-----METAR AREAs --------------------------------------------
  CMeteoArea      dArea;                    // Default metar
  std::map<Tag,CMeteoArea*> Areas;          // Metar area
  CMeteoArea     *cArea;                    // Current area
  //-----Wind randomizer ----------------------------------------
  CRandomizer     wSPD;                     // Wind spd
  CRandomizer     wDIR;                     // Wind direction
	CRandomizer     wTRB;											// Wind turbulence
  //-----Wind parameters ----------------------------------------
  double        cosw;   // Longitude speed increment in arcsec per second
  double        sinw;   // Latitude  speed increment in arcsec per second
  float         windASP;                    // Average speed
  float         windKTS;                    // Actual wind speed
  float         windDIR;                    // Actual wind meteo direction
  float         windROS;                    // Actual real rose  direction
  float         windTDR;                    // Actual wind Trigo direction
  double        windXSP;                    // speed x composite
  double        windYSP;                    // Speed y composite
  //-----Default values -----------------------------------------
  float         mCEL;                       // Ceil
  int           mLAY;                       // Layer
  float         mDEW;                       // Dew point
  float         mVIS;                       // Visibility
  //-------------------------------------------------------------
public:
  // Constructors/destructor
  CWeatherManager(void);
 ~CWeatherManager(void);

public:
  // wind angle relative to the wind direction in LH m/s
  // CWindModelmethods
  int   TimeSlice(float dT,U_INT FrNo);
  void  Init();
  void  SetDefault();
  //------METAR MANAGEMENT ----------------------------------------
  void  GetMetar(CmHead *obj);
  void  AddMetar();
  void  RemoveMetar(CmHead *obj);
  void  AttachMetar(C_QGT *qgt,bool ac);
  int   SelectArea();
  void  ChangeMetar(CMeteoArea *m);
  void  ChangeCloud(char nl);
	//---------------------------------------------------------------
	void	AddDefaultLayer(char n);
  //---------------------------------------------------------------
  void  UpdateArea(float dT);
  void  UpdateWind(float dT);
  //---------------------------------------------------------------
  void  Probe(CFuiCanva *cnv);
  //---------------------------------------------------------------
  CMeteoArea *CWeatherManager::GetMetar(Tag kid);
  //----- user data -----------------------------------------------
  void  GetDefaultWinds();
  char *GetCloudCover();
  void  GetShadowParameters(float *c, float *a);
  //--------------------------------------------------------------
  float  GetWindMPS();          // Wind speed in metre per second
  //------WIND MANAGEMENT ----------------------------------------
  double     GetWindXcomposite()           {return windXSP;}
  double     GetWindYcomposite()           {return windYSP;} 
  //---Return default values ------------------------------------
  float    GetMetarRadius()              {return Radius;}
  //---Get WIND PARAMETERS --------------------------------------
  float    WindFrom()                    {return windDIR;}
  float    WindRoseDirection()           {return windROS;}
  float    WindDirection()               {return windTDR;}
  float    WindSpeed()                   {return windKTS;}
  //---Get Barometer --------------------------------------------
  //-------------------------------------------------------------
  float    GetCloudCeil()      {return cArea->GetCeil();}
  int      GetCloudLayer()     {return cArea->GetLayer();}
  //-------------------------------------------------------------
  void     SetCeil(float h)    {cArea->SetCeil(h);}
	float		GetTurbulence(float dT)	{return wTRB.TimeSlice(dT);}
};

#endif // WEATHER_H_


