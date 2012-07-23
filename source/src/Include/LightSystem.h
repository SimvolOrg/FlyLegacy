//=====================================================================================================
//	Light system MANAGEMENT
//=====================================================================================================
/*
 * lightSystem.h
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
#ifndef LIGHTSYSTEM_H
#define LIGHTSYSTEM_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//=========================================================================================
#include "../Include/Globals.h"
#include "../Include/WorldObjects.h"
#include "../Include/3dMath.h"
//==========================================================================================
class TCacheMGR;
void SetAlphaColor(float alpha);
void ResetAlphaColor();
//==========================================================================================
//  Altitude adjustment for all airport points
//==========================================================================================
//==========================================================================================
void LsComputeElevation(TC_VTAB &tab,SPosition &org);
//-------PROFILE INDEX-----------------------------------------------------------------
#define TC_PROF_APR       0     // APPROACH
#define TC_PROF_TBR       1     // THRESHOLD
#define TC_PROF_WBR       2     // Wing BAR
#define TC_PROF_TDZ       3     // Touch Down
#define TC_PROF_PAP       4     // PAPI light
#define TC_PROF_MSK       5     // MASK
#define TC_PROF_DIM       6     // Stack size
//-------LIGHT COLOR CODE --------------------------------------
#define TC_BLK_LITE       0             // No color
#define TC_WHI_LITE       1             // WHITE
#define TC_RED_LITE       2             // RED
#define TC_GRN_LITE       3             // GREEN
#define TC_BLU_LITE       4             // BLUE
#define TC_YEL_LITE       5             // YELLOW
#define TC_COLOR_MAX      6             // End Light
//------TEXTURE CODE --------------------------------------------
#define TC_FLR_TEX        0             // FLARE LIGHT
#define TC_WHI_TEX        1             // WHITE LIGHT
#define TC_GLO_TEX        6             // GLOBE LIGHT
#define TC_HDR_TEX        7             // HEAD  LIGHT
#define TC_LIT_MAX        8             // End Texture
//------LIGHT TYPE ----------------------------------------------
#define LS_NULL_LITE      0
#define LS_OMNI_LITE      1             // OMNI directional
#define LS_1122_LITE      2             // Dual light bicolored (C1C1/C2C2)
#define LS_1121_LITE      3             // Dual light 2 colors  (C1C1/C2C1)
#define LS_2111_LITE      4             // Dual light 2 color   (C2C1/C1C1)
#define LS_BAR2_LITE      5             // Bar of 2 colors lights
#define LS_FL12_LITE      6             // 2 colors flash light
#define LS_FLSH_LITE      7             // 2 colors flash light
#define LS_STRB_LITE      8             // Longitudinal strobe light
#define LS_FBAR_LITE      9             // Flash bar lig
#define LS_PAPI_LITE     10             // PAPI light
#define LS_MARK_LITE     11             // Marker light
//------RUNWAY SEGMENT LIGHT TYPE --------------------------------
#define TC_LITE_S1        1             // 300 FEET OF RED/NO lights
#define TC_LITE_S2        2             // 600 FEET OF ALTERNATE RED/WHITE/NO lights
#define TC_LITE_SM        3             // XXX FEET OF WHITE OMNI LIGHTS
//------NUMBER OF FEET BETWEEN LIGHTS ------------------------------
#define TC_FEET_PER_LIGHT       100
#define TC_FEET_FROM_END        200     // Start of center lights
#define TC_FEET_CT_END01        300     // Red center end lights
//----- -----LIGHT MODEL -----------------------------------------
#define TC_LITE_RNONE     0
#define TC_LITE_RMONO     4
#define TC_LITE_HICAT     1
#define TC_LITE_LOCAT     2
#define TC_LITE_2RCAT     3
#define TC_LITE_ENTRY     5
//----------------------------------------------------------------
#define TC_LITE_NONE      0
#define TC_LITE_EDGE      1
#define TC_LITE_INST      2
//-----------TBAR MODELS -------------------------------------------
#define TC_TBAR_NUL       0
#define TC_TBAR_GRN       1             // GREEN ONLY
#define TC_TBAR_RED       2             // RED ONLY
#define TC_TBAR_RGN       3             // BOTH COLOR
#define TC_TBAR_NOP       4             // RED-RED
#define TC_TBAR_SUP       5             // SUPRESS
//----------APPROACH MODELS ----------------------------------------
#define TC_APR_NULL       0
#define TC_APR_REIL       1
#define TC_APR_ODAL       2
#define TC_APR_ALSR       3
#define TC_APR_ALF1       4
#define TC_APR_ALF2       5
#define TC_APR_NONE       6
//-----------OPTION YES NO -----------------------------------------
#define TC_OPTP_YES       1
#define TC_OPTP_SUP       2
//----------PAPI TYPE ----------------------------------------------
#define TC_PAPI_2L        1
#define TC_PAPI_4L        2
#define TC_PAPI_NL        3
//---------DISPLAY MODE --------------------------------------------
#define TC_LMOD_OMNI      0
#define TC_LMOD_FLSH      1
//---------SPOT FACE NUMBER ----------------------------------------
#define TC_SPOT_DIM			(12)
//============================================================================
//  Runway End profile
//============================================================================
struct RWY_EPF {
  char end[8];                      // Runway End
  char aprL;                        // Approach light
  char Wbar;                        // Wing bar
  char Tbar;                        // Threshold bar
  char ctrL;                        // Center Light
  char edgL;                        // Edge light
  char tdzL;                        // Touch down
  char papL;                        // PAPI system                  
};


//=============================================================================
//  MODEL light descriptor
//=============================================================================
struct LITE_MODEL {
  short     nbo;                      // Number of objects
  short     pace;                     // Light spacing
  short     nOfs;                     // North Offset (along runway)
  short     wOfs;                     // West offset  (normal to runwy)
  U_CHAR    type;                     // Light type
  U_CHAR    col1;                     // Color 1;
  U_CHAR    col2;                     // Color 2;
  U_CHAR    ntex;                     // Texture number
  SPosition *lnd;                     // landing Position vector
};
//=============================================================================
//  DECISION STRUCTURE
//=============================================================================
struct SIDE_POINT {
  double   dt;                       // Delta for cross product
  double   sm;                       // Sign of cross product AM*AB
  P2_POINT pa;                       // A point
  P2_POINT pb;                       // B point
  P2_POINT po;                       // Reference point
};
//-----------------------------------------------------------------------------
//  Light parameters
//-----------------------------------------------------------------------------
struct LITE_PARAM {
  SPosition org;                      // Origin 
  SVector   P0;                       // position
  double    dx;                       // X direction
  double    dy;                       // Y direction
  int       total;                    // Total built
  int       lgd;                      // Unit length
};
//=========================================================================================
//  Class C3DLight: A BILL BOARD Light of any size
//=========================================================================================
class C3DLight:public CStreamObject, public CqItem {
  //-------ATTRIBUTES ----------------------------------------
  Tag     Mode;                       // Display mode
  void   *mother;                     // Mother object if any
  //----------------------------------------------------------
  U_CHAR  inQ;                        // In Queue indicator
  U_CHAR  cLit;                       // Color
  U_CHAR  ntex;                       // Texture number
  U_CHAR  rfu1;                       // Reserved
  //----------------------------------------------------------
  float     rDim;                     // Radius dimension
  SPosition oPos;                     // Object position
  float     freq;                     // Frequency
  float     onTm;                     // On time duration
  TC_VTAB   tab[4];                   // Coordinates array
  //-------Methods -------------------------------------------
public:
  C3DLight(void *p,Tag mode,U_CHAR color,float rd=16);
  C3DLight(void *p,Tag mode);
  int   Read(SStream *st, Tag tag);
  void  ReadFinished();
  void  SetLocation(SPosition &pos);
  void  Build();
  int   DrawModel(U_CHAR rfq);
  bool  InQueue();
  void  SetTexture(U_CHAR t);
  //----------------------------------------------------------
  inline void      *GetParent()                 {return mother;}
  inline Tag        GetMode()                   {return Mode;}
  inline int        GetColor()                  {return cLit;}
  inline int        GetNTex()                   {return ntex;}
  inline double     GetRadius()                 {return rDim;}
  inline double     GetFreq()                   {return freq;}
  inline double     GetOntm()                   {return onTm;}
  //----------------------------------------------------------
  inline void       SetMode(Tag t)              {Mode = t;}
  inline void       SetColor(char c)            {cLit = c;}
  inline void       SetNtext(char t)            {ntex = t;}
  inline void       SetRadius(float r)          {rDim = r;}
  inline void       SetFreq(float f)            {freq = f;}
  inline void       SetOntm(float t)            {onTm = t;}
  //----------------------------------------------------------
  inline void       Relocate(double alt)        {oPos.alt += alt;}
  inline void       SetOffset(SPosition &p)     {oPos = p;}
  inline SPosition *ObjPosition()               {return &oPos;}
  inline SPosition  GetPosition()               {return  oPos;}
  inline void       AddTop(float h)             {oPos.alt += h;}
  inline void       OutQueue()                  {inQ = 0;}
};
//========================================================================================
//  CLASS CLitQ:  Collect a C3Dlight queue
//========================================================================================
class CLitQ: public CQueue {
  //--------------------------------------------------------------------------
public:
  inline void Lock()                {pthread_mutex_lock (&mux);}
  inline void UnLock()              {pthread_mutex_unlock (&mux);}
  //----------------------------------------------------------------------
  inline void PutEnd(C3DLight *lit)           {CQueue::PutEnd(lit);}
  inline C3DLight *Pop()                      {return (C3DLight*)CQueue::Pop();}
  inline C3DLight *GetFirst()                 {return (C3DLight*)CQueue::GetFirst();}
  inline C3DLight *GetNext(C3DLight *lit)     {return (C3DLight*)CQueue::GetNext(lit);}
};
//=============================================================================
//  Class CBaseLITE to store  light spot description.
//      col1 and col2:  index in color Table. 0 is no color= nolight
//      ntex:   Reference to a texture that gives the shape of the light (flare or
//              other shape)
//      nbl:    Number of lights
//-----------------------------------------------------------------------------
//      Lights come with several types:
//  LS_OMNI_LITE:   Omni lights of one color (col1).  There are as many light
//                  as coordinates in tab entry.
//  TC_FL12_LITE    Alternate flashing color. Lights are switching from color 1
//                  to color 2 (including 0 for no color) at some frequency.
//
//  The following lights are Dual lights. There are only 2 lights LT0 and LT1.
//  Dual lights color depends on the view point (i.e camera position).
//  Types may be:
//  LS_1122_LITE:   Dual lights.  Depending on camera position, LT0 and LT1 are
//                  a) Either of color 1. b) Either of color 2. First Center lights are
//                  from this type.  When viewed from approach, both lights are whites
//                  When viewed from the runway, both lights are red.
//                  
//  LS_1121_LITE:   Dual lights.  Depending on camera position, LT0 and LT1 are
//                  a) Either both of color 1. b) LT0 is color 2 and LT1 of color 1.
//                  Center lights section near runway end are: a) Both whites when
//                  viewed from approach. b) alternatively red and white when seen from
//                  the runway.
//
//  LS_2111_LITE    Duals lights. Depending on camera position, LT0 and LT1 are
//                  a) LT0 of color 2 and LT1 of color 1. b) Both of color 1.
//                  This is the same as previous type, for opposite end of runway
//
//  LS_BAR2_LITE    Bar of dual lights.  Lights are arranged accross the runway
//                  This type is like LS_1122_LITE.  Threshold lights are green
//                  when viewed from approach and red when viewed from runway.
//                  Light coordinates are in tab entry.
//-----------------------------------------------------------------------------
//  Light may be grouped in one or more separate CLitSYS entities
//  for optimization.  Flasher may be part of separate CLitSYS for instance.
//-----------------------------------------------------------------------------
//  No assumptions is made to the system coordinate that may be in feet, or 
//  in arcsec.  Thus Lights may be used from anywhere, provides the caller
//  has made the correct transformation before calling the CLitSYS.Draw(pc)
//  function.
//  When calling  for Dual light, CLitSYS.Draw(pc) must be called with a valid
//  camera position into CVector pc.  
//=============================================================================
class CBaseLITE : public CqItem {
	friend class CRwyGenerator;
public:
  //-----Attributes -----------------------------------------
  U_CHAR    ltyp;                     // Light Type
  U_CHAR    col1;                     // Color 1
  U_CHAR    col2;                     // Color 2
  U_CHAR    ntex;                     // Texture number
  U_SHORT   nbl;                      // Number of points
  //----Decision point --------------------------------------
  SIDE_POINT side;                    // Side decision
  //------Light coordinates ---------------------------------
protected:
  TC_VTAB  *tab;                      // Coordinates array
  //-----Methods --------------------------------------------
public:
  CBaseLITE(U_CHAR tp,U_SHORT nbl);   // Constructor
 ~CBaseLITE();                        // Destructor
  void  Append(CBaseLITE *src);       // Copy spot from source
  void  DrawAll(U_CHAR No);           // Draw Light ramp
  void  DrawSpot(SVector &cam);       // Draw all spot
  void  DrawOne(int k,U_CHAR No);     // Draw one spot
  bool  SameSide(SIDE_POINT &sp,SVector &pc);
  void  TestDraw(int k,int mode);
  void  PreCalculate();
  //--inline ---------------------------------------------
  inline  void      ClearCount()         {nbl  = 0;}
  inline  int       GetNbSpot()          {return nbl;}
  inline  U_CHAR    GetType()            {return ltyp;}
  inline  U_CHAR    GetColor1()          {return col1;}
  inline  TC_VTAB  *GetVTAB()            {return tab;}
  inline  void      AddCount(int k)      {nbl += k;}
  inline  int       GetTextureNb()       {return ntex;}
  //------------------------------------------------------
  inline  void SetTextureNb(U_CHAR t)     {ntex = t;}
  inline  void SetColor1(U_CHAR c)        {col1 = c;}
  inline  void SetColor2(U_CHAR c)        {col2 = c;}
  inline  void SpotAt(int k,TC_VTAB &ent) {tab[k] = ent;}
  //------------------------------------------------------
  inline  TC_VTAB *GetLastSlot()  {return tab + nbl - 1;}
  inline  bool     IsaMarker()    {return (ltyp == LS_MARK_LITE);}
};
//============================================================================
//  Class CDualLITE 
//  A Dual light has one color when seen from one direction and another color
//  when seen from opposite direction.
//
//                      A
//    O-----------------L(WR)---------------------------------Q
//                      B
//  Example:
//  Assuming O is the runway origin and Q the runway end
//  Line AB is the decision line for light (or vertical bar) at position L
//  When the camera is on the O side from line AB, then the light is
//  WHITE. When the camera is on the other side, the light is RED
//  There may be any combination from 3 colors, including no color
//  For optimization, the class contains 2 lights positions
//============================================================================
class CDualLITE : public CBaseLITE {
public:
  //----Light coordinates ----------------------------------
  TC_VTAB  LT[2];                    // Light 0-1
  //--------------------------------------------------------
  CDualLITE(U_CHAR tp,U_SHORT nbl) : CBaseLITE(tp,nbl) {}
  //--------------------------------------------------------
  void  DrawT1122(SVector &pc);
  void  DrawT1121(SVector &pc);
  void  DrawT2111(SVector &pc);
  void  DrawTBAR2(SVector &pc);
  //--------------------------------------------------------
  void  Lit1At(TC_VTAB &ent)  {LT[0] = ent;}
  void  Lit2At(TC_VTAB &ent)  {LT[1] = ent;}
};
//=========================================================================================
//  Class CFlshLITE for flashing light
//  Flash are driven by 2 timers that control ON time and OFF time duration.
//=========================================================================================
class CFlshLITE : public CBaseLITE {
  //----------------------------------------------------------
  float Time;                           // Current accumulation
  float TLim[2];                        // Control timer (0 =>OFF, 1=> ON)
  U_CHAR State;                         // Flash state
  //----------------------------------------------------------
public:
  CFlshLITE();                          // Constructor
  void  UpdateState();
  void  DrawFlash();
  //----------------------------------------------------------
  void  TimeON (float t)  {TLim[1]  = t;}
  void  TimeOFF(float t)  {TLim[0]  = t;}
};
//=========================================================================================
//  Sequential flash system
//  NOTE: Temporary implementation.  The strobe is implemented with a 90 ° light cone
//=========================================================================================
class CStrobeLITE: public CBaseLITE {
  friend class CAptObject;
	friend class CRwyGenerator;
  //---ATTRIBUTES -------------------------------------------
  float Time;                         // Current time
  float eTim[2];                      // Elapse T1-T2
  char   seq;                         // Sequence
  U_CHAR lim;                         // Limit timer
  //---------------------------------------------------------
  P2_POINT	ad;                       // Increment
  //--------------------------------------------------------
public:
  CStrobeLITE(U_CHAR type,U_SHORT nb);
  void  UpdateState();
  //---------------------------------------------------------
  void  DrawFlash(SVector &c);
  //----inline ----------------------------------------------
  void SetT1(float t)         {eTim[0]  = t;}
  void SetT2(float t)         {eTim[1]  = t;}
};
//=========================================================================================
//  Synchro  flash system
//  NOTE: Temporary implementation.  The strobe is implemented with a 90 ° light cone
//=========================================================================================
class CFlashBarLITE: public CBaseLITE {
  friend class CAptObject;
  //---Attributes -------------------------------------------
  float Time;                         // Current time
  float eTim[2];                      // Elapse T1-T2
  U_CHAR lim;                         // Limit timer
  U_CHAR sta;                         // State
  //---------------------------------------------------------
public:
  CFlashBarLITE(U_CHAR type,U_SHORT nb);
  void    UpdateState();
  void    DrawFlash(SVector &cam);
  //----inline ----------------------------------------------
  void SetT1(float t)         {eTim[0]  = t;}
  void SetT2(float t)         {eTim[1]  = t;}

};
//=========================================================================================
//  PAPI LIGHT
//  
//=========================================================================================
class CPapiLITE: public CBaseLITE {
  friend class CAptObject;
	friend class CRwyGenerator;
protected:
  //---ATTRIBUTES --------------------------------------------
  SPosition *lnd;                     // Landing position
  double *dTAB;                       // Decision table
  //----------------------------------------------------------
public:
  CPapiLITE(U_CHAR type,U_SHORT nb);
  void    DrawPAPI(SVector &pc);
};
//=========================================================================================
//  Class CLiteQ  To collect pavement lights of the same type
//=========================================================================================
class CLiteQ : public CQueue {
public:
  U_CHAR type;
  //---------------------------------------------------------------------
  ~CLiteQ();
  //---------------------------------------------------------------------
  inline void PutEnd (CBaseLITE *lit)           {CQueue::PutEnd(lit);}
  inline void PutHead(CBaseLITE *lit)           {CQueue::PutHead(lit);}
  inline CBaseLITE *Pop()                       {return (CBaseLITE*)CQueue::Pop();}
  inline CBaseLITE *GetFirst()                  {return (CBaseLITE*)CQueue::GetFirst();}
  inline CBaseLITE *GetNext(CBaseLITE *lit)     {return (CBaseLITE*)CQueue::GetNext(lit);}
};
//=========================================================================================
//  Light system:  This object support a list of light to draw
//  It can be used to switch on and OFF a whole set of lights of differents types
//=========================================================================================
class CLitSYS {
  //----Attributes ------------------------------------------------------
  float   atn;                                // Light attenuation
  float   mins;                               // Minimum size
  U_CHAR  Sig1;                               // Light state
  U_CHAR  Sig2;                               // Color index
  CLiteQ  litQ;                               // List of light
  //---------------------------------------------------------------------
public:
  CLitSYS();
  void  DrawSystem(SVector &pc);               // Draw all lights
  void  DrawSpot(SVector &cam,CBaseLITE *lit);
	void	StartDraw();
	void	EndDraw();
  //---------------------------------------------------------------------
  void  Flush() {litQ.Clear();}
  //---------------------------------------------------------------------
  inline void SetMiniSize(float m)            {mins = m;}
  inline void AddLight(CBaseLITE *lit)        {litQ.PutEnd(lit);}
  inline void AddGreenLight(CBaseLITE *l)     {litQ.PutHead(l);}
  inline void AddBlueLight (CBaseLITE *l)     {litQ.PutEnd (l);}
  inline void SetS1(U_CHAR s)                 {Sig1 = s;}
  inline void SetS2(U_CHAR s)                 {Sig2 = s;}
  inline  CLiteQ   *GetLitQ()                 {return &litQ;}
  inline bool HasLites()                      {return litQ.NotEmpty();}
  inline bool NoLite()                        {return litQ.IsEmpty();}
  };
//=============================================================================
//  Class Runway Light Profile
//=============================================================================
class CRLP {
  //----Attributes ---------------------------------------------
  CRunway *rwy;                       // Mother runway
  U_INT  slite;                       // Lite switch
  //------------------------------------------------------------
  U_CHAR modify;                      // Modify indicator
  U_CHAR HiPF[TC_PROF_DIM];           // Hight profile
  U_CHAR LoPF[TC_PROF_DIM];           // Low Profile
  U_CHAR stHI[4];                     // Save area Hi profile
  U_CHAR stLO[4];                     // Save area Lo Profile
  //------------------------------------------------------------
  U_CHAR moCT;                      // Center lights model
  U_CHAR moED;                      // Edge lights model
  //----List of lights -----------------------------------------
  CLitSYS cLIT;                     // Center Lights
  CLitSYS eLIT;                     // Edges  Lights
  CLitSYS tLIT;                     // Touch down
  CLitSYS bLIT;                     // Bars   Lights
  CLitSYS aLIT;                     // Approach Lights
  CLitSYS papi;                     // PAPI system
  //------------------------------------------------------------
public:
  CRLP(CRunway *rwy);               // Constructor
 ~CRLP();
  void  DefaultProfile(CRunway *rwy);
  int   GetCenterLightModel(CRunway *rwy);
  int   GetEdgeLightModel(CRunway *rwy);
  void  MergeHiUser(RWY_EPF *hip);
  void  MergeLoUser(RWY_EPF *lop);
  int   GetModel(U_INT mod,U_INT msk);
  //-----------------------------------------------------------
  void  WriteProfile(CRunway *rwy,CStreamFile &sf);
  void  WriteValues (U_CHAR *prof,CStreamFile &sf);
  //-----------------------------------------------------------
  void  SetLight(U_CHAR *ls,U_CHAR *lpf);
  //-------------------------------------------------------------
  void  ChangeHiAPR(U_CHAR op);
  void  ChangeLoAPR(U_CHAR op);
  void  ChangeHiTBR(U_CHAR op);
  void  ChangeLoTBR(U_CHAR op);
  void  ChangeHiWBR(U_CHAR op);
  void  ChangeLoWBR(U_CHAR op);
  void  ChangeHiTDZ(U_CHAR op);
  void  ChangeLoTDZ(U_CHAR op);
  void  ChangeHiCTR(U_CHAR op);
  void  ChangeLoCTR(U_CHAR op);
  void  ChangeHiEDG(U_CHAR op);
  void  ChangeLoEDG(U_CHAR op);
	//------------------------------------------------------------
	void	DistanceNow(float d);
  //------------------------------------------------------------
  void  Flush();
  void  DrawLITE(SVector &cam);
  //------------------------------------------------------------
  inline int    GetCenterLM()       {return moCT;}
  inline int    GetEdgeLM()         {return moED;}
  //-------------------------------------------------------------
  inline int    GetHiAPRL()         {return HiPF[TC_PROF_APR];}
  inline int    GetLoAPRL()         {return LoPF[TC_PROF_APR];}
  inline int    GetHiTBAR()         {return HiPF[TC_PROF_TBR];}
  inline int    GetLoTBAR()         {return LoPF[TC_PROF_TBR];}
  inline int    GetHiWBAR()         {return HiPF[TC_PROF_WBR];}
  inline int    GetLoWBAR()         {return LoPF[TC_PROF_WBR];}
  inline int    GetHiTDZN()         {return HiPF[TC_PROF_TDZ];}
  inline int    GetLoTDZN()         {return LoPF[TC_PROF_TDZ];}
  inline int    GetHiPAPI()         {return HiPF[TC_PROF_PAP];}
  inline int    GetLoPAPI()         {return LoPF[TC_PROF_PAP];}
  //-------------------------------------------------------------
  inline void   SetModif()          {modify = 1;}
  inline bool   IsModified()        {return (modify != 0);}
  //-------------------------------------------------------------
  inline CLitSYS &GetPapiSystem()       {return papi;}
  inline CLitSYS &GetCenterSection()    {return cLIT;}
  inline CLitSYS &GetEdgeSection()      {return eLIT;}
  inline CLitSYS &GetTouchSection()     {return tLIT;}
  inline CLitSYS &GetBarSection()       {return bLIT;}
  inline CLitSYS &GetApproachSection()  {return aLIT;}
  //-------------------------------------------------------------
};  
//===========================================================================
// External Lights
//===========================================================================
typedef enum {
  EXTERNAL_LIGHT_NONE     = 0,
  EXTERNAL_LIGHT_TAXI     = 1,
  EXTERNAL_LIGHT_LAND     = 2,
  EXTERNAL_LIGHT_STROBE   = 3,
  EXTERNAL_LIGHT_NAV      = 4,
  EXTERNAL_LIGHT_BEACON   = 5
} EExternalLightPurpose;
//===========================================================================
//  External light for aircraft
//  NOTE:  OpneGL supports 8 lights in standard
//  For landing and taxing, LIGHT 4-5-6-7 are used
//===========================================================================
class CExternalLight : public CStreamObject {
public:
  CExternalLight (Tag type, Tag tag,CExternalLightManager *lm);
  //------ CStreamObject methods -------------
  int       Read (SStream *stream, Tag tag);
	void      ReadFinished();
  //--------------------------------------------------------------
  inline    char NeedUpdate()  {return (upd) & (powered);}
  inline    char IsOn()        {return on;}
  inline    char IsOff()       {return !on;}
  //------ CExternalLight methods ------------
  void      SetPowerState (bool power);
  void      TimeSlice (float dT);
  void      DrawAsQuad();
	void			DrawAsSpot();
	void			LightAttenuation();
	double		GetTextureLimit();
	double		GetLightForVector(CVector &v);
	void			DrawT1();
  void      Print (FILE *f);
private:
  void      StringToColour (const char* s);
  void      StringToPurpose (const char* s);
  void      SetQuad(float rd);
public:
	//--------------------------------------------------------------
	inline Tag	GetTag()	{return unId;}
  //-------Attributes --------------------------------------------
protected:
  Tag           type;         // External light type
  Tag           unId;         // Unique identification tag
  char          stID[8];			//
	//---------------------------------------------------------------
  TC_VTAB       tab[4];       // QUAD Coordinates array
  std::string   part;         // External model part name reference
	//----------------------------------------------------------------
	GLdouble      pMat[16];			// Position matrice
	//----------------------------------------------------------------
  U_CHAR        xCol;         // Color index 
  CVector       oPos;         // Position relative to external model center
  CVector       polr;         // Orientation (feet,pitch,heading)
	float					dist;					// Projector distance
	float					sDir[4];			// Projector direction
  U_INT         oTex;         // Texture object
  double       radius;        ///< Halo radius
  float         cycle;        ///< Flash cycle time in second
  float         duty;         ///< Flash duty cycle (% of total cycle light is on)
  float         Time;         ///< Flash cycle elapsed time
	//--- Spot parameters -----------------------------------------
	CVector       sPos;					// Source center (relative to aircraft)
	CVector				aPos;					// Minimum position
	CVector       bPos;					// Maximum position
	CVector				aRot;					// Minimum rotated
	CVector				bRot;					// Maximum rotated
	double				extd;					// External distance
	double				ratio;				// Distance to radius ratio
	//-------------------------------------------------------------
  EExternalLightPurpose   purp;     ///< Special purpose of light
  CExternalLightManager  *ltm;      ///< Light manager for this light
  U_CHAR                  powered;  ///< Whether the light subsystem is powered
  U_CHAR                  on;       ///< Whether light is currently illuminated
  U_CHAR                  upd;      ///< Need update if flasher
  U_CHAR                  spt;      ///< Is a spot light
};
//===========================================================================
//  External light manager
//===========================================================================
class CExternalLightManager : public CStreamObject {
public:
  CExternalLightManager (CVehicleObject *mv, char *fn);
  virtual ~CExternalLightManager (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  double		*GetROTM();
	//---------------------------------------------------
	void	ReadHalo();
	void	Build2DHalo(int sid);
	void	AddNaviLite(CExternalLight *n);
	void	AddSpotLite(CExternalLight *n);
  //--- CExternalLightManager methods --------
	double		DistanceToGround(CVector *v0,CVector *v1);
  void      SetPowerState (Tag id, bool power);
  void      Timeslice (float dT);
	//--- Drawing --------------------------------------
	void			DrawOmniLights();
	void			DrawSpotLights();
  void      Print (FILE *f);

  //------------------------------------------
	inline U_INT		 GetHalo()											{return txo;}
	inline CVector   GetIntersection()							{return secp.PR;}
  //------------------------------------------
private:
	CIntersector secp;														// Intersector
	U_INT			txo;																// Texture object
	float			ems[4];															// Emmisive color
  CVehicleObject *mveh;                         // Mother vehicle
  int       xLit;                               // Light index
  std::map<Tag, CExternalLight*> nLit;					// Nav lights
	std::map<Tag, CExternalLight*> sLit;					// Spot lights
};
//=======================================================================================
//  CLASS CRLPparser to decode Runway Light Profile
//=======================================================================================
class CRLParser: public CStreamObject {
    //--- ATTRIBUTES --------------------------------------------
    CAirportMgr *apm;
    RWY_EPF prof;                               // Runway profile
    //-----------------------------------------------------------
public:
    CRLParser(CAirportMgr *ap,char *fn);
    void  Decode(char *fn);
    int   Read(SStream *st,Tag tag);
    void  ReadFinished();
  };

//============================END OF FILE =================================================
#endif  LIGHTSYSTEM_H
