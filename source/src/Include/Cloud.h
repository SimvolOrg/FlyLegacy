/*
 * CLOUD.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
 * Copyright 2008      Jean Sabatier
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
//========================================================================================
//  Some code is directly inspired from the following aithors with permission
//  Many Thanks to them
//========================================================================================
//**************
// Source Code:
//**************
//Copyright 2005
//
//Andrei Stoian, andrei.stoian@gmail.com, http://bit13.no-ip.com
//Permission to use, copy, modify, distribute and sell this software and its 
//documentation for any purpose is hereby granted without fee, provided that the 
//above copyright notice appear in all copies and that both that copyright notice 
//and this permission notice appear in supporting documentation. Binaries 
//compiled with this software may be distributed without any royalties or 
//restrictions.  
//
//The author makes no representations about the suitability of this software for any purpose. 
//It is provided "as is" without express or implied warranty.
//
//*******************
// Cloud Data Files:
//*******************
//
//Copyright 2002 
//Mark J. Harris and The University of North Carolina at Chapel Hill
//
//Permission to use, copy, modify, distribute and sell this software and its 
//documentation for any purpose is hereby granted without fee, provided that the 
//above copyright notice appear in all copies and that both that copyright notice 
//and this permission notice appear in supporting documentation. Binaries 
//compiled with this software may be distributed without any royalties or 
//restrictions.  
//
//The author and The University of North Carolina at Chapel Hill make no 
//representations about the suitability of this software for any purpose. It is 
//provided "as is" without express or implied warranty.

#ifndef CLOUD_H
#define CLOUD_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "../Include/FlyLegacy.h"
#include "../Include/Weather.h"
//============================================================================
//  GLOBALS PARAMETERS
//============================================================================
class CCloudSystem;
//============================================================================
//  Cloud State
//============================================================================
#define CLOUD_INITIALIZE   0                    // Cloud is created
#define CLOUD_NEW_BORN     1                    // Cloud is loaded
#define CLOUD_SUN_DISTANCE 2                    // Sun distance computed
#define CLOUD_SUN_INITLITE 3                    // Ordered to sun distance
#define CLOUD_SUN_COMPUTE  4                    // Light computed
#define CLOUD_SUN_RELIGHT  5                    // Light computed
#define CLOUD_CAM_DISTANCE 6                    // Camera distance to compute
#define CLOUD_CAM_ORDERED  7                    // Ordered to camera distance
#define CLOUD_IS_ALIGHTED  8                    // Cloud is newborn
#define CLOUD_TO_REFRESH   9                    // Cloud is textured
#define CLOUD_END_OF_LIFE 10                    // End of life
#define CLOUD_FLAT_INIT   11                    // Init stratus
//=========================================================================
#define CLOUD_END_LAYER  (6)                    // Last cloud layer number
//=====CLOUD DRAWING STATE ================================================
#define CLOUD_DRAW_NOT      0                   // No drawing
#define CLOUD_DRAW_VOL      1                   // Draw as volumetric
#define CLOUD_DRAW_IMP      2                   // Draw as impostor
#define CLOUD_DRAW_FLT      3                   // Draw as a flat cloud
//=====GEO STATE ==========================================================
#define CLOUD_INIT          0
#define CLOUD_REMOTE        1                   
#define CLOUD_INSIDE        2
//=====BOX DIVISION========================================================
#define CLOUD_X_DIV       4                     // X divide
#define CLOUD_Y_DIV       4                     // Y divide
#define CLOUD_Z_DIV       2                     // Z divide
//----------------------------------------------------------------------------
#define CLOUD_MAX_PART  16
#define CLOUD_GEN_DIM   64
//----------------------------------------------------------------------------
#define CLOUD_FULL_VAL 0xFFFFFFFF
#define CLOUD_DIST_TEX_SIZE 64
#define CLOUD_COLR_BUF_SIDE 32
#define CLOUD_COLR_BUF_DIMS (CLOUD_COLR_BUF_SIDE * CLOUD_COLR_BUF_SIDE)
#define CLOUD_IMPOSTOR_SIZE 512
#define SOLID_ANGLE float(0.09)
#define CLOUD_ROW 4
//-----CLOUD TYPES -----------------------------------------------------------
#define CLOUD_W 0
#define CLOUD_N 1
#define CLOUD_M 2
//-----CLOUD ROTATION --------------------------------------------------------
#define CLOUD_ROT  (double(1 * PI)/ 180)
//============================================================================
//    Normalize color as a byte
//============================================================================
inline U_CHAR ByteColor(float cf)
{ if (cf < 0)  cf = 0;
  if (cf > 1)  cf = 1;
  return (U_CHAR)int(255 * cf);
}
//============================================================================
//    Color Data
//============================================================================
class Color4
{
public:

	Color4()
	{
		R = G = B = A = 0.0f;
	}
//------------------------------------------------------------
	Color4(float cR, float cG, float cB, float cA)
	{
		if (cR > 1.0f) cR = 1.0f;	if (cG > 1.0f) cG = 1.0f;
		if (cB > 1.0f) cB = 1.0f;	if (cA > 1.0f) cA = 1.0f;
		if (cR < 0.0f) cR = 0.0f;	if (cG < 0.0f) cG = 0.0f;
		if (cB < 0.0f) cB = 0.0f;	if (cA < 0.0f) cA = 0.0f;

		R = cR;		G = cG;		B = cB;		A = cA;
	}

//-------------------------------------------------------------
	Color4 operator+(Color4 c)
	{
		Color4 res;

		res.R = R + c.R;		res.G = G + c.G;
		res.B = B + c.B;		res.A = A + c.A;

		return res;
	}
//--------------------------------------------------------------
	Color4 operator-(Color4 c)
	{
		Color4 res;
		res.R = R - c.R;		res.G = G - c.G;
		res.B = B - c.B;		res.A = A - c.A;
		return res;
	}
//--------------------------------------------------------------
	Color4 operator*(float f)
	{
		Color4 res;
		res.R = R * f;		res.G = G * f;
		res.B = B * f;		res.A = A * f;
		return res;
	}
//--------------------------------------------------------------
	void Clamp()
	{
		if (R > 1.0f) R = 1.0f;		if (G > 1.0f) G = 1.0f;
		if (B > 1.0f) B = 1.0f;		if (A > 1.0f) A = 1.0f;
		if (R < 0.0f) R = 0.0f;		if (G < 0.0f) G = 0.0f;
		if (B < 0.0f) B = 0.0f;		if (A < 0.0f) A = 0.0f;
	}

	float R, G, B, A;
};
//----------------------------------------------------------------------------
//  For file read
//----------------------------------------------------------------------------
struct Vector3 {
float x;
float y;
float z;
};
//============================================================================
//  Structure for particle
//============================================================================
struct V_PART {
  float   VT_S;                               // Texture S coordinate
  float   VT_T;                               // Texture T coordinate
  U_CHAR  VT_R;                               // Color R component
  U_CHAR  VT_G;                               // Color G component
  U_CHAR  VT_B;                               // Color B compoent
  U_CHAR  VT_A;                               // Color A component
  float   VT_X;                               // World X coordinate
  float   VT_Y;                               // World Y coordinate
  float   VT_Z;                               // World Z coordinate
};
//============================================================================
//  Structure for cloud Header
//============================================================================
struct CLOUD_HDR {
  int     nbp;                          // Number of particles
  CVector pxt;                          // Cloud positive extension
  CVector nxt;                          // Cloud negative extension
  float radius;                         // Cloud radius
  float  hFac;                          // H factor
};
//============================================================================
//  Structure for cloud model
//============================================================================
struct CLOUD_PRT {
    int ID;                           // Particle number
    CVector pos;                      // Position in cloud
    float   size;                     // Particle size
};
//============================================================================
//  Structure for cloud generation
//============================================================================
struct CLOUD_GEN {
  char  type;
  float lon;                          // Longitude
  float lat;                          // Latitude
};
//============================================================================
//  Class CloudParticle
//============================================================================
class CCloudParticle {
  friend class CCloudPuff;
  friend class CCloudSystem;
  //---ATTRIBUTES ----------------------------------------------------
  int     ID;
  CVector ofs;                        // Particle offset in puff
  float   size; 
  float   dist;                       // Squared distance
  float   color;
  float   alpha;
  //---METHODS--------------------------------------------------------
public:
  CCloudParticle(){}
 ~CCloudParticle() {}
  //------------------------------------------------------------------
  void  SetPosition(Vector3 &v);
  void  RotateY(double ca,double sa,V_PART *tab);
  void  MakeVerticalQuad(V_PART *tab,CVector &pos);
  void  MakeHorizontalQuad(TC_VTAB *cbb,CVector &p);
  void  Draw();
  void  Sphere();
  //------------------------------------------------------------------
  inline void     SetColor(float c)         {color = c;}
  inline void     SetSize(float s)          {size  = s;}
  inline float    Distance()                {return dist;}
  inline CVector *Position()                {return &ofs;}
  inline CVector  Offset()                  {return ofs;}
  inline int      GetNo()                   {return ID;} 
  inline double   GetPY()                   {return ofs.y;}                 
};
//============================================================================
//  Cloud box:  A set of particles that made a cluster in cloud model
//              Cloud box becomes a cloud puff when a real cloud is created
//============================================================================
class CCloudBox
{ //---Attributes ---------------------------------------------------
  int     Nbp;                            // Number of particles
  CVector ofs;                            // center position in cloud
  CVector upr;                            // upper extension
  CVector low;                            // lower extension
  //-----------------------------------------------------------------
  CLOUD_PRT *prt;                         // List of particles
  //---Methods ------------------------------------------------------
public:
  CCloudBox(CVector &l,CVector &u);
 ~CCloudBox();
  //-----------------------------------------------------------------
  int        IsInside(CVector &p);
  int        CountParticles (int np,CLOUD_PRT *prt);
  void       AffectParticles(int np,CLOUD_PRT *prt);
  //-----------------------------------------------------------------
  inline bool       IsEmpty()           {return (Nbp == 0);}
  inline CLOUD_PRT *GetParts()          {return prt;}
  inline int        NbrParts()          {return Nbp;}
  inline CVector    GetOffset()         {return ofs;}
  inline CVector    GetUpper()          {return upr;}
  inline CVector    GetLower()          {return low;}
  inline double     GetZOFS()           {return ofs.z;}
};
//============================================================================
//  Cloud model: A set of cloud boxes made of particles
//============================================================================
class CCloudModel
{ //---Attributes ----------------------------------------------------
  CCloudSystem *csys;
  //------------------------------------------------------------------
  float   Radius;                           // Cloud radius
  int     nBox;                             // Number of boxes
  CVector vLow;                             // Lower corner
  CVector vInc;                             // Increment
  std::vector<CCloudBox *> Boxes;
  //---Methods -------------------------------------------------------
public:
  CCloudModel(CCloudSystem *cs);
 ~CCloudModel();
  int         Divide(char *raw);
  //------------------------------------------------------------------
  inline void SetBase(CVector &v)     {vLow = v;}
  inline void SetIncr(CVector &v)     {vInc = v;}
  inline int  GetNbBoxes()            {return nBox;}
};

//============================================================================
//  Class Cloud puff
//============================================================================
class CCloudPuff {
  friend class CCloudSystem;
  //----Cloud identifier for TRACE -----------------------------------
  U_INT     Ident;                    // Cloud identifier
  Tag       Metar;                    // Metar area
  U_INT     u1FR;                     // Update 1 frame
  U_INT     u2FR;                     // update 2
  //------------------------------------------------------------------
  U_CHAR    Volum;                    // Cloud volum
  U_CHAR    cType;                    // Cloud type
  U_CHAR    cStat;                    // Cloud light State
  U_CHAR    dStat;                    // Drawing state
  U_CHAR    gStat;                    // Cloud geo state
  CCloudSystem *csys;                 // Cloud system
  //------------------------------------------------------------------
  CPTR       pQGT;                    // Native QGT
  //-----List of particles -------------------------------------------
  char      visi;                     // Visibility
  short     cPart;                    // Current particule
  short     nPart;                    // Number of particles
  short     dPart;                    // Drawing part
  std::vector<CCloudParticle *> Parts;
  V_PART   *vtab;                     // vertex table
  //-----Cloud billboard ---------------------------------------------
  TC_VTAB   cbb[4];
  double    rdBB;                     // Billboard radius
  double    Ceil;                     // Cloud ceil
  Color4    colr;                     // Cloud color
  //-----Geodata parameters ------------------------------------------
  SPosition geop;                     // World position in arcsec
  CVector   geof;                     // World position in feet
  CVector   geol;                     // Local position in feet
  CVector   ctoc;                     // Camera to Cloud
  //-----Cloud bounds ------------------------------------------------
  CVector   minBB;                    // minimum
  CVector   maxBB;                    // Maximum
  //------------------------------------------------------------------
  float     Radius;                   // Cloud radius
  float     extVOL;                   // External volume
  double    sunDIS;                   // Distance to sun
  double    camDIS;                   // Distance to Camera
  double    impDIS;                   // Distance for impostor
  //----IMPOSTOR -----------------------------------------------------
  GLUquadricObj *sphere;
  U_CHAR    nfrm;                     // Alpha chanel
  U_CHAR    alpha;                    // Alpha chanel
  GLuint    iTex;                     // Impostor texture
  TC_VTAB   impTAB[4];                // QUAD impostor 
  //------------------------------------------------------------------
  float     hFac;                     // Hfactor
  //------------------------------------------------------------------
  CVector   sunPOS;                   // Pseudo sun position for particle
  //----Camera parameters        -------------------------------------
  CCamera  *cCam;
  CVector   camPOS;                   // Absolute camera position
  CVector   lDIR;                     // Light direction
  //------------------------------------------------------------------
  GLuint    pTex;                     // Particle texture
  GLuint    sTex;                     // Sun texture
  //----METHODS ------------------------------------------------------
public:
  CCloudPuff(CCloudSystem *cs,U_CHAR type,SPosition &p,C_QGT *qt);
 ~CCloudPuff();
  //----Utilities ----------------------------------------------------
 void     SetCamPosition(CCamera *cam);
 float    GetPhase(CVector &ed);
 GLuint   SunTexture(int wd);
 void     WriteCloud(int side,U_INT txo);
 int      BuildImpostor(int vp);
 void     GetShortBB();
 void     EditParameters();
 //-----Sun Light computation ----------------------------------------
 void     InitSunLight(SVector &sun);
 void     PartSunLight(SVector &sun);
 //----CLoud life ----------------------------------------------------
 int      Free();
 void     Create();
 void     FormVoluCloud();
 void     FormFlatCloud();
 void     SunDist();
 void     SunSort();
 void     SunLite(SVector &sun);
 void     CamDist();
 void     CamSort();
 void     RelightCloud();
 void     StoreBound(CCloudParticle *p);
 //-------------------------------------------------------------------
 inline   double GetRadius()      {return double(Radius);}
 inline   double FeetToCamera()   {return camDIS;}
 inline   U_CHAR GetState()       {return cStat;}
 inline   double Longitude()      {return geop.lon;}
 inline   double Latitude()       {return geop.lat;}
 inline   double NewCeil()        {return Ceil;}
 inline   void   SetQGT(C_QGT *q) {pQGT = q;}
 inline   bool   IsAlive()        {return (cStat != CLOUD_END_OF_LIFE);}
 //-------------------------------------------------------------------
 int      CloudRemote();
 int      CloudInside();
 int      CloudFirst();
 //-------------------------------------------------------------------
 int      Update(CCamera *cam);
 int      Refreshing();
 int      CheckOwner();
 void     Disappear();
 //----Billboard angles ----------------------------------------------
 void     SetTrueRotation();
 //-------------------------------------------------------------------
 void     ImpostorTour();
 void     RenderImpostor();
 void     RenderPuff();
 void     RenderFlat();
 void     DrawDetail();
 void     DrawFlatBound();
 void     DrawVoluBound();
 void     DrawParticles();
 int      Draw();
 void     RenderPart();
};

//============================================================================
//  Meteo System
//============================================================================
class CCloudSystem {
  U_INT    Frame;                     // Current frame
  //-----ATTRIBUTES ------------------------------------------------
  U_INT    stamp;                     // Stamp number
  U_SHORT  nTex;                      // Dimension of Distribution texture
  GLuint   pTex;                      // Particule texture
  GLuint   sTex[3];                   // Stratus texture
  //-----Time parameters -------------------------------------------
  U_SHORT   sTic;                     // One Second timer
  //-----FBO object ------------------------------------------------
  GLuint    FBO;                      // Frame buffer Object
  GLuint    DEP;                      // Sun Depth
  GLuint    IMP;                      // Impostor Depth
  //-----White color with alpha chanel -----------------------------
  U_CHAR    white[4];
  //-----Cloud Models ----------------------------------------------
  std::vector<CCloudParticle *>Parts; // Particles from file
  std::vector<void *>RawSet;          // Set of particle
  std::vector<CCloudModel *>Models;   // Set of models
  //-----Divide parameters -----------------------------------------
  CVector   vInc;                     // Vector Increment
  //-----Cloud Generator -------------------------------------------
  U_CHAR    Test;                     // Test mode
  U_SHORT   total[2];                 // Total number of clouds
  U_INT     wInd;                     // Current write index
  U_INT     rInd;                     // Current Read  index
  U_INT     upNO;                     // No to update
  std::vector<CCloudPuff *> Puffs;
  //---- Cloud parameters ------------------------------------------
  U_CHAR   Layer;                     // Cloud Layer
  short    SunTxSize;                 // Light texture size
  int      nMOD;                      // Number of models
  double   Ceil;                      // Cloud height
  double   Albedo;                    // Albedo
  double   Extinction;                // Extinction
  double   scFactor;                  // Albedo * Extinction * inv(4PI)
  double   exFactor;
  double   space;                     // Inter space cloud
  double   scale;                     // Cloud scale
  //--- Global parameters ------------------------------------------
  TCacheMGR  *tcm;                    // Cache manager
  SPosition *acp;                     // Aircraft position
  //---Fog parameters ----------------------------------------------
  float    fTim;                      // Fog timer
  float    fTar;                      // Fog target
  float    fCof;                      // Fog coefficient
  //----Color management -------------------------------------------
  CVector  sRGB;                      // Sunset color
  CVector  nRGB;                      // Normal color
  double   hFeet;                     // Horizon in feet
  double   sunT;                      // Sun threshold
  float    hFac;                      // H factor
  //---Camera position ---------------------------------------------
  CCamera *cCam;                      // Current camera
  CVector  camP;                      // Camera position
  CVector  cmUP;                      // Camera up vector
  CVector  sunO;                      // Sun Orientation
  CVector  sunP;                      // Sun Position 
  //----------------------------------------------------------------
  U_INT    cBuf[CLOUD_COLR_BUF_DIMS];  // Color buffer
  float    splat[256];                 // Splatting buffer
  //-----METHODS ---------------------------------------------------
public:
  CCloudSystem();
 ~CCloudSystem();
  //-----MODEL CREATION --------------------------------------------
  void      Init();
  void      DistributionTexture(int n);
  GLuint    SunTexture(int wd);
  GLuint    OneTexture(U_INT type,int wd,int ht,U_CHAR *tx);
  GLuint    LoadTexture(char k);
  void      LoadCloudModels();
  void      LoadModel(int No);
  void      ReadPartPosition(U_INT k,FILE *fp);
  void      ReadPartSize(U_INT k,FILE *fp);
  void      UpdateExtend(CLOUD_HDR *hdr,CVector &pos);
  void      DivideModel(int No,char *mod);
  void      ClearPart();
  void      SetDrawState();
  void      SetColor(U_CHAR alf);
  void      KillClouds();
  //----------------------------------------------------------------
  Tag       GenerateCloud(C_QGT *qgt,Tag mta);
  //----------------------------------------------------------------
  double    RandValue(float sd);
  void      TimeSlice(float dT,U_INT FrNo);
  void      Draw();
  GLuint    SunFBO(GLuint tex);
  GLuint    ImpFBO(GLuint tex,int dim);
  //----------------------------------------------------------------
  Tag       GenTestCloud(C_QGT *qt,Tag kid);
  void      SunsetColor(CVector &v,CVector &col);
  bool      SunsetTime();
  //----------------------------------------------------------------
  inline GLuint   GetStratTexture(int k)  {return sTex[k];}
  inline GLuint   GetParticleTexture()    {return pTex;}
  inline U_INT    GetFrame()              {return Frame;}
  inline int      GetModelNber()          {return nMOD;}
  inline double   GetCeil()               {return Ceil;}
  inline int      GetLigthTexSize()       {return SunTxSize;}
  inline float   *GetSplatBuffer()        {return splat;}
  inline double   GetScFactor()           {return scFactor;}
  inline double   GetExFactor()           {return exFactor;}
  inline double   GetAlbedo()             {return Albedo;}
  inline double   GetExtinction()         {return Extinction;}
  inline double   GetScale()              {return scale;  }
  inline CVector *SunOrientation()        {return &sunO;}
  inline CVector *CamPosition()           {return &camP;}
  inline CVector *CamUP()                 {return &cmUP;}
  inline CVector *GetNormalColor()        {return &nRGB;}
  inline U_INT    GetStamp()              {return stamp++;}
  inline float    GetHFactor()            {return hFac;}
  //----------------------------------------------------------------
  inline short    GetTotal()              {return total[0];}
  inline short    GetImpos()              {return total[1];}
  inline U_CHAR   GetLayer()              {return Layer;}
  //-----Return a cloud model ----------------------------------------------
  void *PickRaw() { int n = RandomNumber(RawSet.size()); return RawSet[n];}
  //------------------------------------------------------------------------
};

//============================END OF FILE =================================================
#endif  // CLOUD_H
