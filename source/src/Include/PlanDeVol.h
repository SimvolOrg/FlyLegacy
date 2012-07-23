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
//===============================================================================
#define FPL_EDITABLE  (1)
#define FPL_PROTECTED (0)
//===============================================================================
#define FPL_STA_NUL	(0)										// Empty state
#define FPL_STA_OPR (1)										// Operational
//---Waypoint states --------------------------------------------------
#define WPT_STA_OUT (1)										// outside of waypoint
#define WPT_STA_INS (2)										// Inside waypoint
#define WPT_STA_TRM	(3)										// Terminated
//---------------------------------------------------------------------
#define WPT_MOD_LEG	(0)										// Leg Mode
#define WPT_MOD_DIR (1)										// Direct mode
#define WPT_MOD_LND (2)										// Mode landing
//=============================================================
#define WPT_MODE_FPLAN  (0)       // Normal mode
#define WPT_MODE_DIRECT (1)       // Direct To
//============================================================================
//  Vector Map Node
//============================================================================
class VMnode 
{
  //----ATTRIBUTES-----------------------------------------------
	CWPoint   *wpt;
  //-------------------------------------------------------------
public:
	VMnode() {wpt = 0;}
	//-------------------------------------------------------------
	SPosition *ObjPosition();
	void       SetObject(CmHead *x);
	CmHead    *GetOBJ();
	void			 SetNodeDistance();
	//-------------------------------------------------------------
	inline void	SetNode(CWPoint *w)		{wpt = w;}

};
//==================================================================================
// Detail of current FLIGHT PLAN
//==================================================================================
class CFuiFlightLog : public CFuiRwyEXT, public CFuiDetail ,public CFuiWindow
{ //------------Attributes ---------------------------------------
	CFPlan             *fpln;									// Flight plan
  //-----------Title line ----------------------------------------
  CRwyLine           *line;                 // Current line
  //--------------------------------------------------------------
  CFuiLabel          *eWIN;                 // Error Label
	CFuiLabel          *ilsF;                 // ils frequency
	//--- Mode management ------------------------------------------
	char								Mode;									// Current mode
	char								rfu1;									// Reserved
  //-------------Database Request ---------------------------------
  CDataBaseREQ  Req;
  //-----------Vertical list box ---------------------------------
  CListBox           *flpBOX;								// Node list
  //----------Edit fields ----------------------------------------
  CFuiTextField       *nWIN;                // Name window
  CFuiTextField       *dWIN;                // Description
	//----------Ceil component -------------------------------------
	CFuiTextField				*wCEL;								// Ceil edit
	//--- Altitude components --------------------------------------
	CFuiTextField       *wALT;								// Altitude edit
  //----------Waypoint management --------------------------------
  CObjPtr            selOBJ;                // Selected object pointer
  U_INT               noWPT;                // Waypoint number
  QTYPE               tyWPT;                // Waypoint type
	CWPoint            *sWPT;									// Selected waypoint
	//--- METHODS --------------------------------------------------
public:
  //------------Methods ------------------------------------------
   CFuiFlightLog(Tag idn, const char *filename);
  ~CFuiFlightLog(void);
	//--- Mode management ------------------------------------------
	void		SetMode();
	void		SwapMode();
  //--- Notifications --------------------------------------------
  bool    OpenDetail();
  void    OpenDirectory();
  void    CloseMe();
	void		ChangeFileName();
  bool    NotifyFromDirectory(CmHead *obj);
  void    NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
	void		NotifyFromPopup (Tag idm,Tag itm,EFuiEvents evn);
  //--- FlightPlan management-------------------------------------
  void    FillCurrentPlan();
	void		Select();
	void		EditCeil(int a);
	//--- Runway management ----------------------------------------
	void		GetRunway();
	void		AddDBrecord(void *rec,DBCODE cd);
	void		EndOfRequest(CDataBaseREQ *req);
  //--- Waypoint management -------------------------------------
  void    CreateAPTwaypoint();
  void    CreateNAVwaypoint();
	void		CreateWPTwaypoint();
  void    InsertWaypoint(CWPoint *w);
  void    DeleteWaypoint();
  void    MoveUpWaypoint();
  void    MoveDwWaypoint();
	void		ClearPlan();
	void		Error(char No);
	void		ModifAlti(int inc);
	void		ModifCeil(int inc);
	void		Teleport();
	//--------------------------------------------------------------
	void		Refresh();
  //--------------------------------------------------------------
  bool    ValidInsert();
  //--------------------------------------------------------------
  inline  void			Reset()						{FillCurrentPlan();}
	inline	CWPoint  *GetSelectedNode()	{return sWPT;}		
  //--------------------------------------------------------------
  void    Draw();
};

//==================================================================================
//  List of FLIGHT PLANS
//==================================================================================
class CFuiListPlan : public CFuiWindow
{ //------------Attributes ---------------------------------------
  U_INT               frame;
	CFPlan						 *fpln;									// Airplane FlightPlan
  //----------List of plans --------------------------------------
  CListBox            allBOX;
  //----------List of Charts -------------------------------------
  CListBox            mapBOX;
  //----------Waypoint management --------------------------------
  CObjPtr            selOBJ;                // Selected object pointer
  U_INT               noWPT;                // Waypoint number
  QTYPE               tyWPT;                // Waypoint type
  CWPoint            *obWPT;                // WayPoint object
  float               speed;                // average Cruise speed
  float               alti;                 // average ceiling
public:
  //------------Methods ------------------------------------------
   CFuiListPlan(Tag idn, const char *filename);
  ~CFuiListPlan(void);
  //------------Notification -------------------------------------
  void    CloseMe();
  void    NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
  //------FlightPlan management-----------------------------------
  void    TitlePlan();
  void    FillPlans();
  void    SelectPlan();
	//--- Add a plan to the list -----------------------------------
	int			AddToList(char *fn);
  //------Chart management ---------------------------------------
  void    FillChartList();
  void    FillOneList(char *ext);
  void    AddChart(char *map);
  void    SelectChart();
  //--------------------------------------------------------------
  void    Draw();
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
  //---METHODS--------------------------------------------------
public:
  PlaneCheckList(CVehicleObject *v);
 ~PlaneCheckList();
	//------------------------------------------------------------
	
  void    OpenList(char *tail);
  int     Read(SStream *st,Tag tag);
  char  **GetChapters();
  void    GetLines(CListBox &box,U_INT ch);
  void    Close();
	void    RegisterWindow(CFuiCkList *w);
	bool    Execute(D2R2_ACTION &a);
  //------------------------------------------------------------
  EMessageResult ReceiveMessage (SMessage *msg);
  //------------------------------------------------------------
  inline  bool HasChapter()		{return (vCHAP.size() != 0);}
	//------------------------------------------------------------
};
//===========================================================================
//  CWPoint defines a node in the current flight plan
//	NOTE:		The position is the waypoint position with flying altitude
//					it may differ from the ground position of the waypoint
//					given by the Database object
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
	double					dfeet;							// Distance in feet
	double					magdv;							// Magnetic deviation
  //---------------------------------------------------------------------
  CFPlan					*fplan;							// Mother flight plan
	//--- AREA TO EDIT VALUES   --------------------------------------------
  char            Iden[6];            // Identity
  char            Mark[2];            // Crossed marker
  char            Dist[12];           // Distance
	char						Dirt[6];						// Direction to
  char            Alti[12];           // Altitude
  char            Elap[16];           // Elapse time
  char            Etar[16];           // Arrival time
	//---------------------------------------------------------------------
	float						dDir;			// Direct Direction to waypoint
	float						rDir;			// Direction to this waypoint from previous
	float						mDis;			// Plane distance in miles
	float						sDis;			// Summmed distance
	float           pDis;			// Previous distance
  float           legDis;		// Distance from previous in nm
  //---------------------------------------------------------------------
  U_CHAR                    State;        // State
	U_CHAR										last;				  // Last waypoint
	U_CHAR										mode;					// Leg mode
	U_CHAR										activ;				// Active indicator
	//---------------------------------------------------------------------
	char											tkoRWY[6];	  // Take off runway
	char											lndRWY[6];		// Landing runway
  SDateTimeDelta            elapse;       // Elapse time
  SDateTime                 dptime;       // Departure time
  SDateTime                 artime;       // Arrival time
  CObjPtr                   DBwpt;        // Way point from database
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
	//----------------------------------------------------------
	LND_DATA	*GetLandingData();
	bool		EnterLanding(CRadio *rad);
	//---HELPERS -----------------------------------------------
	void		SetSeq(U_SHORT s);
	void		NodeOne(CWPoint *n);
	void		NodeTwo(CWPoint *p);
	void		NodeEnd();
	void		NodeNAV(CWPoint *p, char m);
	void		UpdateMark(char m);
	void    SetPosition(SPosition p);
  void		SetReferenceDIR(double d);
	float		GoDirect(CVehicleObject *v);
	void		CorrectDrift(CRadio *R, CVehicleObject *V);
	//--- Position Management ----------------------------------
	bool		CannotChange();
	bool		HorizontalMove(SPosition *pos);
	void		ModifyLocation(SVector &v);
	void		SetAltitudeAGL();
	void		Teleport();
	//--- Altitude management ----------------------------------
	char*		ModifyAltitude(int inc);
	int			BestAltitudeFrom(int a0);
	void    SetOverAltitude(int a);
	int			NormeAltitude(int a);
	void		UpdAltitude(int a);
	//----------------------------------------------------------
	bool		IsLast();
	bool		SameAPT(char *idn);
	char		GoingAway();
	char		Outside();
	char		Inside();
	char		UpdateState();
	void		UpdateRange(CVehicleObject *veh);
	void		EditArrival();
	float		GetPrevDistance();
	//-------------------------------------------------------------
	void		FillWPT(CmHead *obj);
	void		Unassign();
	//-------------------------------------------------------------
	void		Save(CStreamFile &sf);
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
	inline int        GetElevation()				{return position.alt;}
	inline CmHead*    GetDBobject()         {return DBwpt.Pointer();}
	inline  Tag       GetType()             {return type;}
  inline char*      GetDbKey()            {return dbKey;}
	inline char*      GetTkoRwy()						{return tkoRWY;}
	inline char*      GetLndRwy()						{return lndRWY;}
	inline SPosition* GetGeoP()							{return &position;}
	inline Tag				GetUser()							{return user;}
	inline float			GetFrequency()				{return DBwpt->GetFrequency();}
	inline float			GetDTK()							{return rDir;}	
	inline float			GetCAP()							{return dDir;}
	inline double     GetMagDeviation()			{return DBwpt->GetMagDev();}
	inline double			GetDirection()				{return rDir;}
	inline CWPoint   *GetOrgWPT()						{return (CWPoint*)DBwpt->GetUPTR();}
	//--------------------------------------------------------------
	inline void				SetLast()							{last = 1;}
	inline void				SetLandingMode()			{mode = WPT_MOD_LND;}
	inline void				SetDirectMode()				{mode = WPT_MOD_DIR;}
	inline void				SetLegMode()					{mode = WPT_MOD_LEG;}
	inline void				SetMaxDistance()			{mDis = 50000;}
	//--------------------------------------------------------------
	inline bool				HasTkoRWY() {return (strcmp("NONE",tkoRWY) != 0);}
	inline bool				HasLndRWY()	{return (strcmp("NONE",lndRWY) != 0);}
  inline bool       IsVisited()						{return (*Mark == 'X');}
	inline bool				IsActive()						{return (activ != 0);}
	inline bool       NotAirport()          {return (type != 'airp');}
	inline bool				IsFirst()							{return (nSeq == 1);}
	inline bool				IsaWaypoint()					{return (type != 'snav');}
	inline bool				IsInside()						{return (WPT_STA_INS == State);}
	//----------------------------------------------------------------------
	inline bool				NotFromFPL()	{return GetDBobject()->NoUPTR();}
	inline bool				IsFromFPL()		{return GetDBobject()->HasUPTR();}
	inline bool       IsLanding()		{return (mode == WPT_MOD_LND);}
	inline char				IsDirect() 		{return (mode == WPT_MOD_DIR);}
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
	//--- Rabbit -----------------------------------------------------
	GLUquadricObj *sphere;
	//----------Serial used by GPS to detect flight plan change ---
  U_INT           serial;
  //----------Logical state --------------------------------------
  U_CHAR          State;                      // State
	U_CHAR          edMOD;											// protected or editable
  //--------------------------------------------------------------
	Tag							format;											// Actual format
	//--------------------------------------------------------------
	char						genWNO;											// Waypoint number
  char            realp;                      // 0=>Just descriptor
  char            modify;                     // Modified indicator
	char						rfu1;											  // rfu
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
	CWPoint        *sWPT;												// Selected waypoint
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
  VMnode       rOrg;                       // Origin
  VMnode       rExt;                       // Destination
	//--- LIST OF WAYPOINTS -----------------------------------------
	CFlpLine        head;												// Title
  CListBox        wPoints;                    // List of way points
	//--- METHODS ---------------------------------------------------
public:
	CFPlan(CVehicleObject *m,char rp);
 ~CFPlan();
	//---------------------------------------------------------------
protected:
	void	GenerateName();
	void	ClearPlan();
	int 	Read (SStream *stream, Tag tag);
	void	ReadFormat(SStream *stream);
	void	ReadFinished();
	void	InitPlan();
	//---------------------------------------------------------------
public:
	void	TimeSlice(float dT, U_INT fr);
	void	WarnGPS();
	int 	ModifyCeil(int inc);
	void	UpdateAllNodes();
	void	UpdateDirectNode(U_INT fr);
	void	UpdateActiveNode(U_INT fr);
	void	ActivateNode(CWPoint *wpt);
	void	RestoreNode();
	//---------------------------------------------------------------
	bool	SwapEditMode();
	bool  CannotEdit();
	//---------------------------------------------------------------
	float TurningPoint();
	//--- External interface -------------------------------------
	bool  StartPlan(CWPoint *wp);				// From GPS or VPIL
	void	StopPlan();
	//--- Helpers ---------------------------------------------------
	int		CheckError();
	int		NodeType(CWPoint *wp);
	char *GetDepartingKey();
	char *GetDepartingIDN();
	char *GetDepartingRWY();
	bool  HasTakeOffRunway();
	bool	HasLandingRunway();
	bool	AtDepAirport();
	bool	IsOnFinal();
	bool	NotFor3D();
	//void	SaveNearest(CWPoint *w);
	char *PreviousIdent(CWPoint *wpt);
	//--- Direct mode management ------------------------------------
	void	AssignDirect(CmHead *obj);				// From GPS
	void	ClearDirect();										// From GPS
	//---Flight plan management -------------------------------------
	bool		AssignPlan(char *fn);
	void		AddNode(CWPoint *wpt);
	CWPT    *CreateUserWPT(SPosition *p);
	CWPoint *CreateAPTwaypoint(CAirport *apt);
	CWPoint *CreateNAVwaypoint(CNavaid *nav);
	CWPoint *CreateWPTwaypoint(CWPT		*pnt);
	CWPoint *NextStep(CWPoint *n);
	CWPoint *BaseWPT(CWPoint *w);
	CWPoint *StartingNode();
	CWPoint *GetBestWaypoint();
	CWPoint *GetWaypoint(int No);
	//--- Tracking flight plan --------------------------------------
	CWPoint *GetSelectedNode()			{return sWPT;}
	void	SetSelection(CWPoint *p)	{sWPT = p;}
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
	void  GoToNextNode();
	void	GoToPrevNode();
	CWPoint   *NextNode(CWPoint *w);
	CWPoint   *PrevNode(CWPoint *w);
	void			 MoveSelectedWPT(SVector &v);
	bool	Exist(int No);
	//---------------------------------------------------------------
	void	DrawOnMap(CFuiVectorMap *win);
	void	DrawNode();
	void	DrawOn3DW();
	//---Interface to window FlightLog ------------------------------
	void	Register(CFuiFlightLog *w);
	//---------------------------------------------------------------
	inline char				GetEdMode()				{return  edMOD;}							
	inline CFuiFlightLog *GetWinPlan()	{return win;}
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
	inline void				Unprotect()				{edMOD = FPL_EDITABLE;}
	inline void				Protect()					{edMOD = FPL_PROTECTED;}
	inline bool				IsProtected()			{return (edMOD == FPL_PROTECTED);}
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
