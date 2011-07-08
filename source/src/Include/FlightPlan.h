/*
 * FlightPlan.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
 * Copyright 2007-2009 Jean Sabatier
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


#ifndef FLIGHTPLAN_H
#define FLIGHTPLAN_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Include/FlyLegacy.h"
#include "../Include/Robot.h"
//============================================================================
class CListBox;
class CFlpLine;
class CFlightPlan;
//============================================================================
#define FPLAN_NULL   0
#define FPLAN_STOP   1
#define FPLAN_LOAD   2
#define FPLAN_TOUR   3
#define FPLAN_OPER   4
//============================================================================
// Flight plan route: define a leg in the flight plan
//============================================================================
class CFpWaypointRoute : public CStreamObject {
public:
  CFpWaypointRoute (void) {};
  int Read(SStream *stream, Tag tag);
  //------------------------------------------------------------
protected:
};
//===========================================================================
//  CWayPoint defines a node in the current flight plan
//===========================================================================
class CWayPoint : public CStreamObject
  {  
    enum WSTA { OTS = 0,                    // Outside
                INS = 1,                    // Inside
                TRM = 2,                    // Terminated
  };
private:
  //------------------Attributes --------------------------------------------
  U_SHORT                   Seq;
  Tag                       type;         // Waypoint type
  char                      name[40];     // Human-readable name
  SPosition                 position;     // Geographic position
  float                     altitude;     // Vehicle altitude at waypoint
  char                      dbKey[10];    // Database key value
  //---------------------------------------------------------------------
  CFlightPlan              *fplan;        // Mother flight plan
  //---------------------------------------------------------------------
  char                      *sqTAB;       // SQL table
  Tag                        sqTAG;       // SQL Tag
  U_CHAR                    State;        // State
  U_CHAR                    nxDAY;        // Next day arrival
  float                     disPLN;       // Distance from aircraft
  float                     Legdis;       // Distance from previous in nm
  float                     disTot;       // Total distance from start point
  float                     rDIR;         // Direct Radial to waypoint
  float                     wDIR;         // Route to this waypoint from previous
  float                     Inner;        // Inner distance
  float                     Speed;        // Speed at waypoint
  SDateTimeDelta            elapse;       // Elapse time
  SDateTime                 dptime;       // Departure time
  SDateTime                 artime;       // Arrival time
  CObjPtr                   DBwpt;        // Way point from database
  //------Edit parameter ------------------------------------------------
  char                      mTxt[2];      // Marker
  char                      oMrk;         // Previous mark
  //-----------------------------------------------------
public:
   CWayPoint (CFlightPlan *fp);
  ~CWayPoint (void);
  // CStreamObject methods
  int  Read (SStream *stream, Tag tag);
  void Print (FILE *f);
  //----WayPoint method -----------------------------------------------
  void  ClearDate(SDateTime *sd);
  void  UpdateDistance(CFlpLine *pln);
  void  UpdateFirstNode();
  void  UpdateState();
  void  UpdateNode(CFlpLine *pln,U_INT fr);
  void  UpdateValue();
  bool  TerminateRoute();
  void  ActivateRoute();
  void  SetElapse(float dis);
  void  GetArrivalTime(U_INT *hh,U_INT *mm);
  void  GetArrivalDay (U_INT *dd,U_INT *mo);
  void  GetLegDuration(U_INT *dd,U_INT *hh,U_INT *mm);
  void  SetDBwpt(CmHead *obj);
  void  Save(SStream *s);
  bool  SameWPT (CmHead *obj);
  char *GetIdentity();
  bool  Refresh(U_INT fr);
  void  ComputeLeg(CWayPoint *prv);
  //--------------------------------------------------------------------
  inline void       SetSeq(int n)                 {Seq = n;}
  inline int        GetSeq()                      {return Seq;}
  //--------------------------------------------------------------------
  inline U_SHORT    GetWaypointNo()               {return Seq;}
  inline void       SetType(Tag tp)               {type = tp;}
  inline void       SetName(char *nm)             {strncpy(name,nm,40);}
  inline void       SetPosition(SPosition pos)    {position = pos;}
  inline void       SetDbKey(char *dk)            {strncpy(dbKey,dk,10);}
  inline void       SetOBJ(CmHead *ob)            {DBwpt = ob;}
  inline void       SetRadial(float rd)           {rDIR     = rd;}
  inline void       SetAltitude(float alt)        {altitude = fabs(alt);}
  inline void       SetSpeed(float spd)           {Speed    = spd;}
  inline void       SetDeparture(SDateTime d)     {dptime   = d;}
  inline char       GetSameDay()                  {return nxDAY;}
  inline bool       IsInside()                    {return (State == INS);}
  inline bool       IsTerminated()                {return (State == TRM);}
  inline void       SetMark(char m)               {mTxt[0] = m;}
  inline bool       HasMark(char m)               {return (mTxt[0] == m);}
  //--------------------------------------------------------------------
  inline bool       NoAltitude()          {return (-99999 == altitude);}
  inline SPosition  GetPosition()         {return position; }
  inline SPosition *ObjPosition()         {return &position;}
  inline float      GetLegDistance()      {return Legdis;}
  inline float      GetTotalDis()         {return disTot;}
  inline float      GetAltitude()         {return altitude;}
  inline float      GetDirectRadial()     {return rDIR;}
  inline float      GetRoute()            {return wDIR;}
  inline float      GetSpeed()            {return Speed;}
  inline float      GetDTK()              {return rDIR;}
  inline char*      GetDbKey()            {return dbKey;}
  inline  Tag       GetTnod()             {return type;}
  inline char*      GetName()             {return name;}
  inline char*      GetMark()             {return mTxt;}
  inline CmHead* GetDBobject()            {return DBwpt.Pointer();}
  inline char*      GetSQLtab()           {return sqTAB;}
  inline Tag        GetSQLtag()           {return sqTAG;}
  inline bool       IsPopulated()         {return DBwpt.Assigned();}
  //--------------------------------------------------------------------
};
//============================================================================
//  FLIGHT PLAN object
//============================================================================
class CFuiListPlan;
//============================================================================
class CFlightPlan : public CStreamObject {
  friend class CWayPoint;
public:
   CFlightPlan (char opt);
  ~CFlightPlan (void);
   
  // CStreamObject methods 
  //-------------------------------------------------------------
  void            Init();
  //----------File management -----------------------------------
  void            InitialState();
  void            Clear(char m);
  void            Reset();
  void            Open(char *fname);
  int             Read (SStream *stream, Tag tag);
  void            ReadPlan(SStream *s);
  void            SetName(SStream *s);
  void            Print (FILE *f);
  void            AddNode(CWayPoint *wp);
  //-------------------------------------------------------------
  inline void  SetFileName(char *n)     {strncpy(Name,n, 64); Name[ 63] = 0;}
  inline void  SetDescription(char *d)  {strncpy(Desc,d,128); Desc[128] = 0;}
  //-------------------------------------------------------------
  //---------Time slice routine ---------------------------------
  void            ComputeSpeed();
  void            EndOfTour();
  void            TimeSlice(float dT, U_INT FrNo);
  void            StartFPlan();
  void            Recycle();
  void            InitWaypoint(CWayPoint *wpt);
  void            TerminateWaypoint();
  void            Refresh(CFuiFlightLog *win);
  bool            IsFirst (CWayPoint *wp);
  bool            IsActive(CWayPoint *wp);
  bool            IsLast(CWayPoint *wp);
  //---------Vector map interface -------------------------------
  void            DrawOnMap(CFuiVectorMap *win);
  //---------GPS interface --------------------------------------
  CWayPoint      *GetWaypoint(U_INT No);
  CWayPoint      *GetLastWaypoint();
  CWayPoint      *GetPrevWaypoint();
  CmHead         *GetDBObject(U_INT No);
  char           *GetTermIdent();
  char           *GetLastIdent();
  float           GetTotalDistance();
  float           GetDistanceTo(U_INT No);
  float           GetRemainingNM();
  float           GetActiveRoute();
  short           NbInFlightPlan(CmHead *obj,char opt);
  short           ValidActiveWaypoint(CFlpLine *lin);
  short           GetNoActive();
  short           GetNoTerminated();
  void            wptActive(CFlpLine *lin);
  void            wptUnmark(CFlpLine *lin);
  void            GetActiveParameters(GPSpoint &pm);
  //---------Save the file --------------------------------------
  void            Save();
  void            RenameFile(char *old,char *fbak);
  void            GenerateName(char *n);
  //-------------------------------------------------------------
  inline U_INT      GetSerial()                 {return serial;}
  inline bool       NotSame(U_INT s)            {return (s != serial);}
  inline bool       IsNull()                    {return (State == FPLAN_NULL);}
  inline bool       NotStable()                 {return (State != FPLAN_OPER);}
  inline bool       IsStarted()                 {return (State == FPLAN_OPER);}
  inline bool       NotOper()                   {return (State != FPLAN_OPER);}
  inline bool       IsEmpty()                   {return (2 > nWPT.size());}           // KLN89 
  inline bool       NotFirst(U_INT No)          {return (0 != No);}
  inline bool       NotLast (U_INT No)          {return ((nWPT.size()-1) != No);}
  //---------------------------------------------------------------
  inline char       AirBorn()                   {return airborn;}
  inline void       SetLegDepTime(SDateTime sd) {LegDT = sd;}
  inline SDateTime  GetFPLDepTime()             {return FplDT;}
  inline SDateTime  GetLegDepTime()             {return LegDT;}
  inline float      GetSpeed()                  {return Speed;}
  inline float      GetAltitude()               {return Alti;}
  inline int        GetFlightTime()             {return int(fTime);}
  inline int        GetSize(void)               {return nWPT.size();}
  inline char      *GetDescription()            {return Desc;}
  inline char      *GetFileName()               {return Name;}
  //----------Return the waypoint list ---------------------------
  inline CListBox  *GetFBOX()           {return wPoints;}
  //----------Attributes -----------------------------------------
protected:
  //----------Serial used by GPS to detect flight plan change ---
  U_INT           serial;
  //----------Logical state --------------------------------------
  U_CHAR          State;                      // State
  //--------------------------------------------------------------
  char            option;                     // 1=>Just descriptor
  char            modify;                     // Modified indicator
  char            Name[64];                   // File name
  char            Desc[128];                  // Description
  U_INT           Version;                    // Version
  U_INT           Num;                        // Waypoint seq number
  //----------Flight plan as a list of waypoints ----------------
  CListBox       *wPoints;                    // List of way points
  CFlpLine       *tLIN;                       // Last terminated point
  CFlpLine       *aLIN;                       // Active point
  CFlpLine       *eLIN;                       // Last point
  CFlpLine       *pLIN;                       // Previous point
  CFlpLine       *uLIN;                       // Updated waypoint
  //----------Route parameters ----------------------------------
  CRouteEXT       rOrg;                       // Origin
  CRouteEXT       rExt;                       // Destination
  //-------------------------------------------------------------
  U_INT           Frame;                      // Current frame
  //-------------------------------------------------------------
  char            airborn;                    // Indicator
  float           Speed;                      // Speed
  float           Alti;                       // Altitude
  float           fTime;                      // Flight Time
  SDateTime	      FplDT;					            // UTC FP depart time
  SDateTime       LegDT;                      // Leg Depart time
  //---------Linked list of waypoints for GPS interface ---------
  std::vector<CWayPoint*> nWPT;
  //-------------------------------------------------------------
public:
  inline bool   NeedLoad()      {return (State == FPLAN_STOP);}
  inline void   SetLoad()       {State = FPLAN_LOAD;}
  inline void   Modify(char m)  {modify |= m;}
};
//==========================================================================================
//  CheckList chapters
//==========================================================================================
class CheckChapter: public CStreamObject {
  //----ATTRIBUTES ---------------------------------------------------
	U_INT			flag;												// Validation flags
	D2R2_ACTION ra;												// Robot action
  int       nbl;                        // Number of lines
  CChkLine *pLIN;                       // Primary line
  std::vector<CChkLine*>vLIN;           // Table of lines
  char name[32];
  //------------------------------------------------------------------
public:
  CheckChapter() {pLIN = 0;}
 ~CheckChapter();
  int   Read(SStream *st,Tag tag);
  void  GetLines(CListBox &box);
  void  GenerateLines(SStream *st);
  void  CreateNode(SStream *st,int No);
  //------------------------------------------------------------------
	Tag		OpCode(char *s);
  bool  SLClikAction(char *buf);    // Left  click coordinates
  bool  MLClikAction(char *buf);    // Left  click coordinates
  bool  SRClikAction(char *buf);    // Right click coordinates
  bool  MRClikAction(char *buf);    // Right click coordinates
  bool  RatioAction(char *buf);     // Click until %
  bool  SLFieldAction(char *buf);   // Simple CA click
  bool  MLFieldAction(char *buf);   // multiple CA click
  bool  SRFieldAction(char *buf);   // Simple   CA click
  bool  MRFieldAction(char *buf);   // multiple CA click

  bool  ShowAction(char *buf);      // Just focus on gauge
  bool  PanlAction(char *buf);      // Just display panel
  void  DecodeAction(SStream *st);  // Decode action
  void  DecodeGauge(char *buf);     // Decode gauge
	void	DecodeCondition(char *txt);
  //-----inline ------------------------------------------------------
  char *GetName()               {return name;}
  int   GetSize()               {return 32;}
};
//================END OF FILE ===========================================================
#endif // FLIGHTPLAN_H
