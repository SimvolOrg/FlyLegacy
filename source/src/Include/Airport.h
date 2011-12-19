//=====================================================================================================
//	AIRPORT MANAGEMENT
//=====================================================================================================
/*
 * AIRPORT.h
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
#ifndef AIRPORT_H
#define AIRPORT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//=========================================================================================
#include "../Include/Globals.h"
#include "../Include/FlyLegacy.h"
#include "../Include/TerrainCache.h"
#include "../Include/LightSystem.h"
#include "../Include/GeoMath.h"
//============================================================================
class CAptObject;
class CCameraRunway;
class CPicQUAD;
class CTaxiNode;
class CTaxiEdge;
class CDataBGR;
class CRunway;
//============================================================================
//  GENERAL DEFINITIONS 
//============================================================================
#define TC_RWY_PAVED  1
#define TC_RWY_OTHER  0
//----------------------------------------------------------------------------
#define TC_SG_DEP 0                   // Starting segment
#define TC_SG_HD1 1                   // Hi DISPLACED THRESHOLD
#define TC_SG_HD2 2
#define TC_SG_HTR 3                   // Hi THRESHOLD
#define TC_SG_HLT 4                   // Hi LETTER
#define TC_SG_HNB 5                   // Hi NUMBER
#define TC_SG_FIL 6                   // FILLER (no marks)
#define TC_SG_LIN 7                   // CENTER LINE MARK
#define TC_SG_MK5 8                   // 500 FEET MARKER
#define TC_SG_MK0 9                   // 1000 FEET MARKER
#define TC_SG_MID 10                  // MIDDLE SEGMENT
#define TC_SG_LNB 11                  // Lo NUMBER
#define TC_SG_LLT 12                  // Lo LETTER
#define TC_SG_LTR 13                  // Lo THRESHOLD
#define TC_SG_LD2 14                  // Lo DISPLACED SECTION
#define TC_SG_LD1 15                  // Lo DISPLACED SECTION
#define TC_SG_END 40                  // SegmentEND
//-----GENERATOR CODE --------------------------------------------------------
#define TC_GEN_DSP 0                  // Displacement generator
#define TC_GEN_ALW 1                  // Always generated
#define TC_GEN_LIM 2                  // Generate up to limit
#define TC_GEN_MID 3                  // Mid segment generation
#define TC_GEN_END 4                  // Last segment
//-----VERTEX INDEX ----------------------------------------------------------
#define TC_SGSW  0                   // South West
#define TC_SGNW  1                   // North West
#define TC_SGNE  2                   // North East
#define TC_SGSE  3                   // South East
//------DRAWING STATUS -------------------------------------------------------
#define TC_AP_IN_VIEW 0
#define TC_AP_FADE_IN 1
//------PAVEMENT TYPE --------------------------------------------------------
#define PAVE_TYPE_PAVE   1            // Dallage
#define PAVE_TYPE_EDGE   2            // edge lane
#define PAVE_TYPE_CENT   3            // Center lane
//------PAVEMENT USAGE -------------------------------------------------------
#define PAVE_USE_RUNW    0
#define PAVE_USE_TAXI    1
#define PAVE_USE_PARK    2
//=============================================================================
#define TC_HD1SEGINDEX  0
#define TC_HD2SEGINDEX  1
#define TC_MIDSEGINDEX 19
#define TC_LD2SEGINDEX 36
#define TC_LD1SEGINDEX 37
//=============================================================================
//  Runway segment descriptor
//=============================================================================
struct TC_RSEG_DESC {
  U_CHAR segID;                             // Segment identifier
  U_CHAR segGN;                             // Segment code generator
  U_CHAR segTX;                             // Segment texture code
  U_CHAR segRF;                             // Future use
  U_INT  segLG;                             // Segment length
};
//=============================================================================
//  MID POINT DESCRIPTOR
//=============================================================================
struct TC_RMP_DEF {
  U_CHAR segID;                             // Segment identifier
  U_CHAR segTX;                             // Segment texture
  U_INT  segLG;                             // Segment length
  double px;                                // X coordinate
  double py;                                // Y coordinate
  double pz;                                // Z coordinate
};
//============================================================================
//  class CPaveRWY to store pavement polygons
//============================================================================
class CPaveRWY : public CqItem {
public:
  //-----Attributes --------------------------------------
  U_CHAR    Type;                     // Pavement type
  U_CHAR    aUse;                     // Airport usage
  CPolygon  ptab;                     // Polygon table
  //-----Methods -----------------------------------------
public:
   CPaveRWY(U_INT nb);                // Constructor
  ~CPaveRWY();                        // Destructor
   //------------------------------------------------------
   void     SetElevation(float e,int k);
   void     Draw()    {ptab.Draw();}
	 int			CopyTo(TC_VTAB *buf);
   void     AppendAsTriangle(CPaveRWY *src);
   //------------------------------------------------------
   inline void SetType (U_CHAR t)  {Type = t;}
   inline void SetUsage(U_CHAR u)  {aUse = u;}
   inline void SetMode(U_INT m)    {ptab.SetMode(m);}     
   inline void ClearCount()        {ptab.ClearCount();}
   inline void AddCount(int k)     {ptab.AddCount(k);}
   //------------------------------------------------------
   inline char     GetType()  {return Type;}
   inline char     GetUsage() {return aUse;}
   inline int      GetNBVT()  {return ptab.GetNBVT();}
   inline TC_VTAB *GetVTAB()  {return ptab.GetVTAB();}
};
//=========================================================================================
//  Class Tarmac segment
//=========================================================================================
#define TARMAC_SEG (64)
#define TARMAC_VRT (TARMAC_SEG * 4)
//=======================================================================
class CTarmac {
protected:
	//--- ATTRIBUTES -------------------------------------
	CAptObject	*apo;									// Airport object
	CRunway			*rwy;									// Associated runway
	//----------------------------------------------------
	U_INT		xKey;											// Texture Key
	U_INT		xOBJ;											// Texture Object
	U_INT		oVBO;											// VBO buffer object
	U_CHAR	flag;											// Indicators
	U_CHAR  blnd;											// Blending
	U_SHORT nPRM;											// Number of primitives
	U_SHORT xIND;											// Current indice
	U_INT		nVRT;											// Number of vertices
	TC_VTAB tab[TARMAC_VRT];					// List of vertices
	GLint   ind[TARMAC_SEG];					// List of indices
	//--- METHODS -----------------------------------------
public:
	CTarmac(CAptObject *a,CRunway *r);
 ~CTarmac();
	TC_VTAB *Reserve(int n);
	void		 LoadVBO();
	void		 Draw();
	//-----------------------------------------------------
	inline void			SetOBJ(U_INT obj)				{xOBJ = obj;}
	inline void			SetKEY(U_INT k)					{xKey = k;}
	inline void			SetBLD(U_CHAR b)				{blnd = b;}
	//-----------------------------------------------------
	inline int			GetNBVT()					{return nVRT;}
};
//=========================================================================================
//  CLASS CPaveQ
//        Class to collect pavement descriptor
//=========================================================================================
class CPaveQ : public CQueue {
  //---------------------------------------------------------------------
public:
  inline void Lock()                {pthread_mutex_lock (&mux);}
  inline void UnLock()              {pthread_mutex_unlock (&mux);}
  //----------------------------------------------------------------------
  ~CPaveQ();                        // Destructor
  //----------------------------------------------------------------------
  inline void PutEnd(CPaveRWY *pav)           {CQueue::PutEnd(pav);}
  inline CPaveRWY *Pop()                      {return (CPaveRWY*)CQueue::Pop();}
  inline CPaveRWY *GetFirst()                 {return (CPaveRWY*)CQueue::GetFirst();}
  inline CPaveRWY *GetNext(CPaveRWY *pav)     {return (CPaveRWY*)CQueue::GetNext(pav);}
};
//=========================================================================================
//  CLASS CAptObject
//        Class to describe one airport
//  NOTE:  Detail Tiles covering the airport surfaces are rendered as part of airport
//         rendering.  Runways are placated on ground tiles before the rest of the terrain.
//  gnd:   Defines the rectangle in term of Detail Tile indices
//
//         During airport life, the distance to aircraft is kept in real time
//=========================================================================================
class CAptObject : public CqItem, public CDrawByCamera {
  friend class CAirportMgr;
	friend class CRunway;
  //--------------Attributes -----------------------------------------------
	U_CHAR					bgr;																// BGR indicator
	U_CHAR					tr;																	// Trace option
  U_CHAR          txy;                                // Taxiway in SQL
  U_CHAR          visible;                            // Visibility
	//------------------------------------------------------------------------
  CAirportMgr    *apm;                                // Airport Manager
  CObjPtr         pApt;                               // Airport smart pointer
  CAirport       *Airp;                               // Airport pointer
  double          cutOF;                              // Altitude cut-off
  double          Alt;                                // Aircraft altitude
  CCamera        *cam;                                // Current camera
  SPosition       apos;                               // Aircraft position
  SVector         cpos;                               // Camera position
	float						nmiles;															// Distance in miles
	//--- Taxiway nodes ------------------------------------------------------
	CDataBGR       *txBGR;															// Taxiway nodes
  //------------------------------------------------------------------------
  TCacheMGR      *tcm;
  //------COLORS -----------------------------------------------------------
  float *white;
  float *yellow;
  //------DRAWING STATUS ---------------------------------------------------
  U_CHAR      sta3D;                              // State
  float       alpha;                              // Alpha chanel
  //------VECTOR TRANSLATION -----------------------------------------------
  SVector   ofap;                                 // Airport origin offset from aircraft
  SVector  *scale;                                // Scale factor
  //------COMPENSATION FACTORS AT AIRPORT LATITUDE -------------------------
	double    xpf;																// Expension factor
  double    rdf;																// Reduction factor
  SVector   scl;                                // Scale factor for letter
  SVector   sct;                                // Scale factor for threshold bands
  //------AIRPORT COMPONENTS  ----------------------------------------------
	std::vector<CTarmac*> tmcQ;										// Tarmac queue
  CPaveQ    pavQ;                               // Pavement Queue
  CPaveQ    edgQ;                               // Edge Queue
  CPaveQ    cntQ;                               // Center Queue
	//------Corresponding VBO --------------------------------------------
	U_INT	    nPAV;																// Total Pavement primitives
	U_INT			nEDG;																// Total Edges primitives
	U_INT     nCTR;																// Total Center primitives
	U_INT			nTAR;																// Total Tarmac primitives
	U_INT     pVBO;																// VBO pavement
	U_INT			eVBO;																// VBO Edge
	U_INT			cVBO;																// VBO center
	U_INT			rVBO;																// VBO runway
  //------Light systems ------------------------------------------------
  CLitSYS   becS;                               // Beacon
  CLitSYS   taxS;                               // Taxiways
  //----- Positions ----------------------------------------------------
  SPosition Org;                                // Airport origin
  SPosition llc;                                // Texture left corner
  //---Ground tile management -----------------------------------------
  TC_BOUND  glim;                               // Ground limit
  std::vector<CGroundTile*> grnd;               // Airport ground
	//---Ground Vertex buffer -------------------------------------------
	char			 gRES;										// Ground resolution
	char       rfu2;										// Reserved
	U_INT			 Time;
  U_SHORT    nGVT;										// Number of ground vertices
	TC_GTAB		*gBUF;										// Ground Buffer
	U_INT			 gVBO;										// Vertex Buffer Object
  //------------------------------------------------------------------
  CVector   center;                             // Ground center
  CVector   gBound;                             // ground bound   
  GLuint    oTAXI;                              // Pavement Texture Object
  //------Grid parameters ------------------------------------------------
  int     gx;                                 // X number
  int     gz;                                 // Z number
  double grid;                                // Grid scale
  //-----Lighting control ------------------------------------------------
  U_INT     lTim;                             // Light timer
  CRunway  *lrwy;                             // Lighting runway
  char      swlt;                             // Light state
  char      lreq;                             // Light request
  //----------------------------------------------------------------------
  double ground;   // Common ground elevation
  //-----For test- -------------------------------------------------------
  GLUquadricObj *sphere;
  //--------------Methods ------------------------------------------------
public:
    CAptObject(CAirportMgr *m, CAirport *apt);
    CAptObject(CAirport *apt);                // For export only
   ~CAptObject();
   bool   InitBound();
   //---TIME SLICE ----------------------------------------------------
   void   TimeSlice(float dT);
   //--- RUNWAY BUILDING ----------------------------------------------
	 void		TraceRWY(CRunway *rwy);
	 void		CompactRWY();
   int    BuildTaxiways();
   int    GetTaxiways();
   void   SetRunway();
   void   SetCameraPosition();
   void   AptExtension(GroundSpot &gs);
	 //--- Airport ground management -------------------------------------
   void   LocateGround();
	 void		BuildGroundVBO();
	 void		FillGroundVBO();
   void   MarkGround(TC_BOUND &bnd);
   void   UnmarkGround();
	 //-------------------------------------------------------------------
	 inline void AddTarmac(CTarmac *st)	{tmcQ.push_back(st);}
	 //--- Compact a queue into a VBO ------------------------------------
	 TC_VTAB *PutInVBO(CPaveQ &hq,U_INT n);
	 //-------------------------------------------------------------------
   void   ComputeElevation(TC_VTAB *tab);
   void   RebuildLight(CRunway *rwy);
  //-----LIGHTS  BUILDING-------------------------------------------------
   void  BeaconLight();
   //---------------------------------------------------------------------
   void  OnePavement(CPaveRWY *p,U_INT n);
  //-----AIRPORT Management ----------------------------------------------
   char *GetApName();
   void  AddPOD();                  // ADD File profile to POD
   void  RemPOD();                  // Remove File profile from POD
   void  ChangeLights(char ls);     // Switch lights ON/OFF
   void  UpdateLights(float dT);    // Update light state
	 void  ReadTaxiNodes();						// Taxiway node & edges
	//-----Identifier ------------------------------------------------------
   inline bool      SameApt(CAirport *a)      {return (a == Airp);}
	//--- Toatl triangles --------------------------------------------------
	 inline void			AddPAV(U_INT n)						{nPAV += n;}
	 inline void			AddEDG(U_INT n)						{nEDG += n;}
	 inline void			AddCTR(U_INT n)						{nCTR += n;}
  //----------------------------------------------------------------------
  inline bool      IsVisible()                {return (visible != 0);}
  inline bool      NotVisible()               {return (visible == 0);}
	inline void			 SetMiles(float m)					{nmiles = m;}
  inline void      GetSLT(SVector &v)         {v = scl;}
  inline void      GetSTH(SVector &v)         {v = sct;}
  inline void      SetCamera(CCamera *c)      {cam = c;}
  inline void      SetAircraft(SPosition p)   {apos = p;}
  //----------------------------------------------------------------------
  inline void      LightPlease()              {lreq ^= 1;}
  //----------------------------------------------------------------------
  inline double    GetGround()                {return ground;}
//  inline double    GetXnormal(double f)       {return (f * ppx);}
//  inline double    GetYnormal(double f)       {return (f * ppy);}
  inline double    GetRDF()                   {return rdf;}
  inline double    GetXPF()                   {return xpf;}
  inline SPosition GetOrigin()                {return Org;}
	inline float		 GetNmiles()								{return nmiles;}
	//---------------------------------------------------------------------
  inline void      SetScale(float sc)         {grid = sc;}
  inline void      SetXGrid(int nx)           {gx = nx;}
  inline void      SetZGrid(int nz)           {gz = nz;}
  inline void      SetLLC(SPosition p)        {llc = p;}
  //----------------------------------------------------------------------
  inline char     *GetAptName()           {return Airp->GetAptName();}
  //----------------------------------------------------------------------
  inline CAirport *GetAirport()               {return Airp;}
  inline void      AddPavement(CPaveQ &q) {pavQ.Append(q);}
  inline void      AddEdge(CPaveQ &q)     {edgQ.Append(q);}
  inline void      AddCenter(CPaveQ &q)   {cntQ.Append(q);}
  //----------------------------------------------------------------------
  inline void      OneTaxiLight(CBaseLITE *l)   {taxS.AddLight(l);}
  //--------Light Queues -------------------------------------------------
  inline void      AddGreenLight(CBaseLITE *l)  {taxS.AddGreenLight(l);}
  inline void      AddBlueLight (CBaseLITE *l)  {taxS.AddBlueLight (l);}
  //------Pavement queues ------------------------------------------------
  inline CPaveQ   *GetPavQ()    {return &pavQ;}
  inline CPaveQ   *GetEdgQ()    {return &edgQ;}
  inline CPaveQ   *GetCntQ()    {return &cntQ;}
  inline CLiteQ   *GetLitQ()    {return  taxS.GetLitQ();}
  //----------------------------------------------------------------------
	inline bool NoRunway()	{return (tmcQ.size() == 0);}
	inline bool HasRunway()	{return (tmcQ.size() != 0);}
  //----------------------------------------------------------------------
  inline char *GetKey()        {return (Airp)?(Airp->GetKey()):(0);}
  //--------DRAW Airport --------------------------------------------------
  void    Draw();
	void		DrawGround();
	void		DrawTextureGround();
	void		DrawVBO(U_INT vbo,U_INT n);
  void    DrawLights(CCamera *cam);
  void    DrawGrid();
  void    DrawILS();
  void    DrawCenter();
  void    Update3Dstate();
  //--------DRAW by Camera -----------------------------------------------
  void    PreDraw(CCamera *cam);
  void    CamDraw(CCamera *c);
  void    EndDraw(CCamera *cam);
  //----Compute position offset relative to airport origin ---------------
  void    Offset(SPosition &p, SVector &v);
  //-----------------------------------------------------------------------
};
//=========================================================================================
//  CLASS CAptQueue
//        Class to gather all airports in a N nmiles radius around the aircraft
//=========================================================================================
class CAptQueue : public CQueue {
  //--------------------------------------------------------------------
public:
  ~CAptQueue();                     // Destroy Queue
  //---------------------------------------------------------------------
  inline void Lock()                {pthread_mutex_lock (&mux);}
  inline void UnLock()              {pthread_mutex_unlock (&mux);}
  inline CAptObject *Pop()                    {return (CAptObject*)CQueue::Pop();}
  inline CAptObject *GetFirst()               {return (CAptObject*)CQueue::GetFirst();}
  inline CAptObject *GetNext(CAptObject *apt) {return (CAptObject*)CQueue::GetNext(apt);}
  inline CAptObject *Detach(CAptObject *apt)  {return (CAptObject*)CQueue::Detach(apt);}
};
//=========================================================================================
//  Class CAirportMgr:  To manage all airport around the aircraft
//=========================================================================================
class CAirportMgr { 
  //-----Attributes ------------------------------------------------
  U_CHAR          nProf;                      // Profile Stack index
  U_CHAR          clock;                      // Refresh clock
  char            dILS;                       // Drawing ILS indicator
	//--- Letter and band VBO -----------------------------------------
	U_INT						xOBJ;												// yellow texture
	U_INT						bVBO;												// Band & letter VBO buffer
	//--- Departing runway --------------------------------------------
	ILS_DATA       *rdep;												// Departing spot
  //-----------------------------------------------------------------
  TCacheMGR      *tcm;
  U_INT           tr;                         // Trace indicator
  CAptQueue       aptQ;                       // Airport Queue
  CDbCacheMgr    *dbm;                        // Database manager
  CCamera        *cam;                        // Current camera
  //----Distance to collect airports -------------------------------
  double          Dist;                       // Distance to collect airport data
  double          Limit;                      // (Dist * Dist) 
  //----Drawing parameters -----------------------------------------
  int             PavSize;                    // Pavement texture in feet
  double          PavArc;                     // Pavement size in arcsec
	//---- Nearest airport -------------------------------------------
	CAptObject		 *nApt;												// Nearest airport
	SPosition      *tko;												// Take off position
	SPosition      *endp;												// Runway end point
  //-----Aircraft Icon----------------------------------------------
  CPicQUAD       *avion;                      // Aircraft icon
  //-----Methods ---------------------------------------------------
public:
  CAirportMgr(TCacheMGR *tm);
 ~CAirportMgr();
  void    TimeSlice(float dT);                // Update Airport List
	void		bindLETTERs();
	void		SaveNearest(CAptObject *apo);
	//--- EXTERNAL INTERFACE ------------------------------------------
	bool		AreWeAt(char *key);
	bool		SetOnRunway(CAirport *apt,char *idn);
	char   *NearestIdent();
	bool    GetTakeOffDirection(SPosition **opp,SPosition *p);
  //----AIRPORT BUILDING --------------------------------------------
  int       SetRunwayProfile(CAirport *apt);
  void      AddProfile(RWY_EPF &epf);
  RWY_EPF  *CAirportMgr::GetEndProfile(char *id);
  //-----------------------------------------------------------------
  void			Draw(SPosition pos);                  // Draw airports
  void			DrawLights();                         // Draw airport lights
  //-----------------------------------------------------------------
	inline  void	BindYellow()  {glBindTexture(GL_TEXTURE_2D,xOBJ);}
	//-----------------------------------------------------------------
  inline void         SetCamera(CCamera *c)   {cam = c;}
  inline CCamera     *GetCamera()             {return cam;}
  inline double       GetPaveFactor()         {return PavArc;}
  inline CPicQUAD    *GetIcon()               {return avion;}
	inline void					SwapILSdraw()						{dILS ^= 1;}
	//-----------------------------------------------------------------
	inline CAptObject  *GetNearestAPT()					{return nApt;}
	inline ILS_DATA    *GetDepartingEND()				{return rdep;}
};

//============================END OF FILE =================================================

#endif  AIRPORT_H
