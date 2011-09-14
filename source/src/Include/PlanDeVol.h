//===============================================================================
// PlanDeVol.h
//
//
// Part of Fly! Legacy project
//
// Copyright 2005 Chris Wallace
// CopyRight 2007 Jean Sabatier
// Fly! Legacy is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
// Fly! Legacy is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
//   along with Fly! Legacy; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//===============================================================================
#ifndef PLANDEVOL_H
#define PLANDEVOL_H

#include "../Include/FlyLegacy.h"
#include "../Include/FuiParts.h"

//===============================================================================
class CFPlan;
//===========================================================================
#define FPL_STA_NUL	(0)										// Empty state
#define FPL_STA_OPR (1)										// Operational
//---Waypoint states --------------------------------------------------
#define WPT_STA_OUT (1)										// outside of waypoint
#define WPT_STA_INS (2)										// Inside waypoint
#define WPT_STA_TRM	(3)										// Terminated
#define WPT_STA_AWA (4)										// Going away
//---------------------------------------------------------------------
#define WPT_MOD_LEG	(0)										// Leg Mode
#define WPT_MOD_DIR (1)										// Direct mode
#define WPT_MOD_LND (2)										// Mode landing
//============================================================================
//  Flight Plan route extremity
//============================================================================
class CRouteEXT: public CmHead {
  //----ATTRIBUTES-----------------------------------------------
	CWPoint   *wpt;
  //-------------------------------------------------------------
public:
	CRouteEXT() {wpt = 0;}
	//-------------------------------------------------------------
  void       SetPosition(SPosition *p);
	SPosition *ObjPosition();
	void       SetObject(CmHead *x);
	void			 SetNodeDistance(float d);
	CmHead    *GetOBJ();
	float			 GetLegDistance();
	//-------------------------------------------------------------
	inline void	SetNode(CWPoint *w)		{wpt = w;}

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
//===================================================================
//  CheckList Subsystem
//===================================================================
class PlaneCheckList: public CSubsystem {
  //---ATTRIBUTES ----------------------------------------------
protected:
	//--- Registered window --------------------------------------
  CFuiCkList *cWIN;
	//---  Autorobot ---------------------------------------------
  CRobot *d2r2;																	// Robot location
  std::vector<CheckChapter*>   vCHAP;						// Table of Chapters
	std::vector<SMessage*>	     autoM;						// Auto start messages
  //---METHODS--------------------------------------------------
public:
  PlaneCheckList(CVehicleObject *v);
 ~PlaneCheckList();
	//------------------------------------------------------------
	void PrepareMsg(CVehicleObject *veh);
  //------------------------------------------------------------
	bool		FltMessage(char *txt);
	bool		IntMessage(char *txt);
	bool		DecodeMSG(char *txt);
  void		ReadAUTO(SStream *st);		
  void    OpenList(char *tail);
  int     Read(SStream *st,Tag tag);
  char  **GetChapters();
  void    GetLines(CListBox &box,U_INT ch);
  void    Close();
	void    RegisterWindow(CFuiCkList *w);
	bool    Execute(D2R2_ACTION &a);
	//------------------------------------------------------------
	bool		AutoStart();
	SMessage *GetSMessage(U_INT k);
  //------------------------------------------------------------
  EMessageResult ReceiveMessage (SMessage *msg);
  //------------------------------------------------------------
  inline  bool HasChapter()		{return (vCHAP.size() != 0);}
	//------------------------------------------------------------
};
//===========================================================================
//  CWPoint defines a node in the current flight plan
//===========================================================================
class CWPoint : public CSlot, public CStreamObject {
private:
  //------------------Attributes --------------------------------------------
  U_SHORT                   nSeq;					// Sequence number 
  Tag                       type;         // Waypoint type
	Tag												user;					// Waypoint usage
  SPosition                 position;     // Geographic position
  int												altitude;     // Expected altitude at waypoint
  char                      dbKey[10];    // Database key value
	char                      userT[6];			// User tag
	//---------------------------------------------------------------------
	float						ilsF;								// ILS Frequency if any		
	//--- True waypoint representative ------------------------------------
	SPosition        spot;							// waypoint reference
	double					dfeet;							// Distance in feet
	double					magdv;							// Magnetic deviation
  //---------------------------------------------------------------------
  CFPlan					*fplan;							// Mother flight plan
	//--- AREA TO EDIT VALUES   --------------------------------------------
  char            Iden[6];            // Identity
  char            Mark[2];            // Crossed marker
  char            Dist[10];           // Distance
	char						Dirt[6];						// Direction to
  char            Alti[16];           // Altitude
  char            Elap[16];           // Elapse time
  char            Etar[16];           // Arrival time
	//---------------------------------------------------------------------
	float						dDir;			// Direct Direction to waypoint
	float						rDir;			// Direction to this waypoint from previous
	float						mDis;			// Plane distance in miles
	float						sDis;			// Summmed distance
  float           legDis;		// Distance from previous in nm
  //---------------------------------------------------------------------
  U_CHAR                    State;        // State
	U_CHAR										Modif;				// Modifier indicator
	U_CHAR										mode;					// Leg mode
	U_CHAR										activ;				// Active indicator
	//---------------------------------------------------------------------
	char											tkoRWY[6];	  // Take off runway
	char											lndRWY[6];		// Landing runway
  SDateTimeDelta            elapse;       // Elapse time
  SDateTime                 dptime;       // Departure time
  SDateTime                 artime;       // Arrival time
  CObjPtr                   DBwpt;        // Way point from database
  //------Edit parameter ------------------------------------------------
  char             mTxt[2];								// Marker
  //--- METHODS ---------------------------------------------------------
public:
	CWPoint(CFPlan *fp,Tag t);		    			// Constructor
	CWPoint();
 ~CWPoint();															// Destructor
  void		Build(Tag t);
	//-------------------------------------------------------------
	int			Read (SStream *stream, Tag tag);
	void		ClearDate (SDateTime &sd);
	char*   GetIdentity();
	char*   GetSQLtab();
	void		Populate();
	void		PopulateUser();
	//-------------------------------------------------------------
	ILS_DATA	*GetLandingData();
	bool		EnterLanding(CRadio *rad);
	//-------------------------------------------------------------
	void		SetSeq(U_SHORT s);
	void		NodeOne(CWPoint *n);
	void		NodeTwo(CWPoint *p);
	void		NodeEnd();
	void		NodeNAV(CWPoint *p, char m);
	void		UpdateMark(char m);
	char*		ModifyAltitude(int inc);
	char*   ChangeAltitude(int a);
	int			BestAltitudeFrom(int a0);
	void    SetAltitude(int a);
	void    SetPosition(SPosition p);
  void		SetReferenceDIR(double d);
	float		GoDirect(CVehicleObject *v);
	//-------------------------------------------------------------
	bool		IsLast();
	char		CheckAway();
	char		Outside();
	char		Inside();
	char		UpdateState();
	void		UpdateRange(CVehicleObject *veh,U_INT frame);
	void		EditArrival();
	float		GetPrevDistance();
	//-------------------------------------------------------------
	void		FillWPT(CmHead *obj);
	void		Unassign();
	//-------------------------------------------------------------
	void		Save(SStream *s);
	//-------------------------------------------------------------
	void		SetTkoRwy(char *r);
	void		SetLndRwy(char *r);
	void		Edit();
  void		SetLegDistance(float d);
	void		SetSumDistance(CWPoint *p0);
	//--- CSlot management ----------------------------------------
	void    Print(CFuiList *w,U_CHAR ln); 
	//-------------------------------------------------------------
	inline U_SHORT    GetSequence()       {return nSeq;}
	//--- NODE ----------------------------------------------------
  inline void       SetType(Tag t)        {type = t;}
  inline void       SetDbKey(char *k)     {strncpy(dbKey,k,10);}
	//--- Edition -------------------------------------------------
	inline void				SetFlightPlan(CFPlan *p)	{fplan = p;}
	inline void				SetActive(U_CHAR a)		{activ = a;}
	inline void				SetModif(U_CHAR m)		{Modif = m;}
	inline void				SetIlsFrequency(float f)				{ilsF	= f;}
	inline void				SetMark(char *mk)			{strncpy(Mark,mk, 2);}
	inline void       SetIden(char *id)			{strncpy(Iden,id, 5); Iden[5]  = 0;}
	inline void				SetAlti(char *al)			{strncpy(Alti,al,12); Alti[11] = 0;}
	inline void       SetDist(char *di)			{strncpy(Dist,di,10); Dist[9]	 = 0;}
	inline void				SetDBwpt(CmHead *obj)	{DBwpt = obj;}
	inline void				SetUser(Tag u)				{user = u;}
	inline void       SetRefDirection()			{DBwpt->SetRefDirection(rDir);}
	//--- Edited field s ------------------------------------------
	char*							GetEdAltitude()				{return Alti;}
	//---Node parameters ------------------------------------------
	inline char				GetState()						{return State;}
  inline char*      GetName()             {return Name;}
	inline float			GetLegDistance()      {return legDis;}
	inline float			GetSumDistance()			{return (*Mark == 'X')?(0):(sDis);}
	inline float			GetPlnDistance()			{return mDis;}
	inline int				GetAltitude()         {return altitude;}
	inline CmHead*    GetDBobject()         {return DBwpt.Pointer();}
	inline  Tag       GetType()             {return type;}
  inline char*      GetDbKey()            {return dbKey;}
	inline char*      GetTkoRwy()						{return tkoRWY;}
	inline char*      GetLndRwy()						{return lndRWY;}
	inline SPosition* GetGeoP()							{return &position;}
	inline Tag				GetUser()							{return user;}
	inline float			GetFrequency()				{return DBwpt->GetFrequency();}
	inline float			GetILSFrequency()			{return ilsF;}
	inline float			GetDTK()							{return rDir;}	
	inline float			GetCAP()							{return dDir;}
	inline double     GetMagDeviation()			{return DBwpt->GetMagDev();}
	inline double			GetDirection()				{return rDir;}
	inline CWPoint   *GetOrgWPT()						{return (CWPoint*)DBwpt->GetUPTR();}
	//--------------------------------------------------------------
	inline void				SetLandingMode()			{mode = WPT_MOD_LND;}
	//--------------------------------------------------------------
	inline bool				HasTkoRWY() {return (strcmp("NONE",tkoRWY) != 0);}
	inline bool				HasLndRWY()	{return (strcmp("NONE",lndRWY) != 0);}
  inline bool       IsVisited()						{return (*Mark == 'X');}
	inline bool				IsActive()						{return (activ != 0);}
	inline bool       NotAirport()          {return (type != 'airp');}
	inline bool				IsFirst()							{return (nSeq == 1);}
	inline bool				IsaWaypoint()					{return (type != 'snav');}
	inline char				IsDirect() 						{return (Modif | mode);}
	inline bool				IsInside()						{return (WPT_STA_INS == State);}
	//----------------------------------------------------------------------
	inline bool				NotFromFPL()	{return GetDBobject()->NoUPTR();}
	inline bool				IsFromFPL()		{return GetDBobject()->HasUPTR();}
	inline bool       IsLanding()		{return (mode == WPT_MOD_LND);}
	//-----------------------------------------------------------------------
	};
//===========================================================================
//  CFPlan defines flight plan
//===========================================================================
class CFPlan : public CSubsystem {
	friend class CFuiFlightLog;
private:
	CVehicleObject *mveh;												// Mother vehicle
	CFuiFlightLog  *win;												// Handling window
	//----------Serial used by GPS to detect flight plan change ---
  U_INT           serial;
  //----------Logical state --------------------------------------
  U_CHAR          State;                      // State
  //--------------------------------------------------------------
	Tag							format;											// Actual format
	//--------------------------------------------------------------
	char						genWNO;											// Waypoint number
  char            option;                     // 1=>Just descriptor
  char            modify;                     // Modified indicator
	bool						endir;											// Direct waypoint terminated
	//---------------------------------------------------------------------
	char 					  dapt[6];										// Departing airport
	//---------------------------------------------------------------
  char            Name[64];                   // File name
  char            Desc[128];                  // Description
  U_INT           Version;                    // Version
	U_INT						NbWPT;											// Total waypoints
	//---Flight plan management ------------------------------------
	GPSRadio       *GPS;												// Radio GPS if any
	CWPoint        *aWPT;												// Active waypoint
	CWPoint        *uWPT;												// Updated waypoint
	CWPoint        *nWPT;												// Nearest waypoint
	//-------------------------------------------------------------
	char						nul[6];											// Null ident
	//--- Waypoint for direct mode --------------------------------
	CWPoint         dWPT;												// Direct waypoint
	//--- DATA from SVH file ---------------------------------------
	float           insDIS;											// Inner distance
	float					  nmlSPD;											// Normal speed
	float						aprSPD;											// Approach
	int 						mALT;												// max Ceiling
	int							cALT;												// Current ceil
	//----------Route parameters ----------------------------------
  CRouteEXT       rOrg;                       // Origin
  CRouteEXT       rExt;                       // Destination
	//--- LIST OF WAYPOINTS -----------------------------------------
	CFlpLine        head;												// Title
  CListBox        wPoints;                    // List of way points
	//--- METHODS ---------------------------------------------------
public:
	CFPlan(CVehicleObject *m);
 ~CFPlan();
	//---------------------------------------------------------------
protected:
	void	GenerateName();
	void	ClearPlan();
	int 	Read (SStream *stream, Tag tag);
	void	ReadFormat(SStream *stream);
	void	ReadFinished();
	//---------------------------------------------------------------
public:
	void	TimeSlice(float dT, U_INT fr);
	void	UpdatePlan();
	void	WarnGPS(char m);
	int 	ModifyCeil(int inc);
	void	UpdateDirectNode(U_INT fr);
	void	UpdateActiveNode(U_INT fr);
	void	ActivateNode(CWPoint *wpt);
	void	RestoreNode();
	//---------------------------------------------------------------
	double TurningPoint();
	//--- Robot / GPS interface -------------------------------------
	int	  ActivatePlan();				// Form GPS or VPIL
	void	Stop();
	//--- Helpers ---------------------------------------------------
	int		CheckError();
	int		NodeType(CWPoint *wp);
	char *GetDepartingKey();
	char *GetDepartingIDN();
	char *GetDepartingRWY();
	bool  HasTakeOffRunway();
	bool	HasLandingRunway();
	bool	IsOnFinal();
	void	SaveNearest(CWPoint *w);
	char *PreviousIdent(CWPoint *wpt);
	//--- Direct mode management ------------------------------------
	void	AssignDirect(CmHead *obj);				// From GPS
	void	ClearDirect();										// From GPS
	//---Flight plan management -------------------------------------
	bool		AssignPlan(char *fn,char opt);
	void		AddNode(CWPoint *wpt);
	CWPT    *CreateUserWPT(SPosition *p);
	CWPoint *CreateAPTwaypoint(CAirport *apt);
	CWPoint *CreateNAVwaypoint(CNavaid *nav);
	CWPoint *CreateWPTwaypoint(CWPT		*pnt);
	CWPoint *NextStep(CWPoint *n);
	CWPoint *BaseWPT(CWPoint *w);
	//--- Change parameters -----------------------------------------
	void  SetFileName(char *n);
	void	SetDescription(char *d);
	//--- File access -----------------------------------------------
	void	RenameFile(char *old,char *fbak);
	void	Save();
	void	SetDistance(CWPoint *p0, CWPoint *p1);
	void	Reorder(char m);
	void	GenWptName(char *edt);
	void	MovedWaypoint(CWPoint *wpt);
	void	Probe(CFuiCanva *cnv);
	//---------------------------------------------------------------
	CWPoint   *NextNode(CWPoint *w);
	CWPoint   *PrevNode(CWPoint *w);
	bool	Exist(int No);
	//---------------------------------------------------------------
	void	DrawOnMap(CFuiVectorMap *win);
	//---------------------------------------------------------------
	inline void	Register(CFuiFlightLog *w)	{win = w;}
	//--------------------------------------------------------------
	inline int        GetActSequence()	{return (aWPT)?(aWPT->GetSequence()):(0);}
	inline CListBox	 *GetFBOX()         {	return &wPoints;}
	inline void				Modify(char m)		{	modify |= m;}
	inline char		   *GetFileName()			{	return Name;}
	inline char		   *GetDescription()	{	return Desc;}
	inline void				IncWPT()					{	NbWPT++;}
	inline void				DecWPT()					{	NbWPT--;}
	//--- Virtual pilot interface -----------------------------------
	inline CWPoint   *GetActiveNode()		{return aWPT;}
	//--- Aircraft parameters ---------------------------------------
	inline float			crsSpeed()				{return nmlSPD;}
	inline float			aprSpeed()				{return aprSPD;}
	inline int        maxCEIL()					{return mALT;}
	inline int				actCEIL()					{return cALT;}
	inline float			GetInDIS()				{return insDIS;}
	//---------------------------------------------------------------
	inline bool       IsUsed()					{return (State != FPL_STA_NUL);}
	inline bool				IsEmpty()					{return (0 == NbWPT);}
	inline bool       IsNotLast(U_INT s){return (s != NbWPT);}
	inline bool       IsLast(U_INT s)		{return (s == NbWPT);}
	inline bool				Inactive()				{return (State == FPL_STA_NUL);}
	//--------------------------------------------------------------
	inline int				Size()						{return NbWPT;}
	//--------------------------------------------------------------
	inline void				Refresh()					{if (win)	win->Refresh();}
	inline CSlot     *Next(CSlot *s)		{return wPoints.NextPrimary(s);}
	inline CSlot     *Prev(CSlot *s)		{return wPoints.PrevPrimary(s);}
	//---------------------------------------------------------------
	inline CWPoint   *GetNearest()			{return nWPT;}
	inline CWPoint   *HeadNode()				{return (CWPoint*)wPoints.HeadPrimary();}
	inline CWPoint   *LastNode()				{return (CWPoint*)wPoints.LastPrimary();}
	//----------------------------------------------------------------
};

//=======================END OF FILE ======================================================================
#endif //PLANDEVOL_H
