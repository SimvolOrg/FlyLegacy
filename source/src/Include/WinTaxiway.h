//==============================================================================================
// WinTaxiway.h
//
// Part of Fly! Legacy project
//
// Copyright 2003 Chris Wallace
//
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
//================================================================================================

#ifndef WINTAXIWAY_H
#define WINTAXIWAY_H
#include "../Include/FlyLegacy.h"
#include "../include/globals.h"
#include "../Include/Fui.h"
#include "../Include/Cameras.h"
//================================================================================================
#define PROF_TAXI   (PROF_EDITOR+PROF_NO_TEL+PROF_NO_PLANE)
//================================================================================================
class TaxNODE;
class TaxEDGE;
class CAirport;
class TaxiTracker;
class TaxiwayMGR;
//================================================================================================
#define TKO_CIRCUIT	(0)			// Take off circuit
#define LND_CIRCUIT (1)			// Landing circuit
//================================================================================================
#define TAXI_MODE_MOVING	(0)
#define TAXI_MODE_NODEMV	(1)
#define TAXI_MODE_PICKING (2)
#define TAXI_MODE_INSERT  (3)
#define TAXI_MODE_ATTACH  (4)
#define TAXI_MODE_BRANCH	(5)
#define TAXI_MODE_EDGEDL	(6)
//================================================================================================
#define TAG_PER_EDGE			(2)
#define TAG_PER_PATH			(1)
//=============================================================================
//  Structure to pack all edges for SQL
//=============================================================================
struct PACK_EDGE {
	int		 dm1;									// Dimension 1
	U_INT *bf1;									// EDGEs
	int		 dm2;									// Dimension 2
	U_INT *bf2;									// PATHs
	//--- Destructor -------------------------------------------
	PACK_EDGE::~PACK_EDGE()
	{	delete [] bf1;
		delete [] bf2;
	}
};
//=============================================================================
//  Class TaxiRoute:  Used to describe a taxiway route on ground
//=============================================================================
class TaxiRoute {
	//--- ATTRIBUTES ------------------------------------------
protected:
	TaxiwayMGR	*mgr;														// Taxiway manager
	U_SHORT			 ind;														// Node index
	U_SHORT			 dir;														// Direction
	LND_DATA		*rwd;														// Runway data
	std::vector<TaxNODE*> nodQ;							// List of NODES
	//---------------------------------------------------------
public:
	 TaxiRoute();
	~TaxiRoute();
	//---------------------------------------------------------
	void			Clear();
	void			SetRoute(TaxiwayMGR *mgr,char d,LND_DATA *rwd);
	//---------------------------------------------------------
	SPosition		*NextPosition();
	bool				 LastLeg();
	//---------------------------------------------------------
	bool	NoRoute()			{return (nodQ.size() == 0);}
	bool	IsOK()				{return (nodQ.size() != 0);}
	//---------------------------------------------------------
	LND_DATA *GetRunwayData()					{return rwd;}
};
//================================================================================================
//	Class Taxiway circuit
//================================================================================================
class TaxiCircuit: public CStreamObject {
	//--- Attributes -------------------------------------
protected:
	std::map<Tag,TaxEDGE*>	edgQ;		// TaxEDGE
	std::vector<Tag>				work;
	std::vector<Tag>				path;		// Raw path of edges
	std::vector<Tag>				allP;		// All paths
	//--- Check circuit ----------------------------------
	std::vector<TaxNODE*> lstFR;	// List of from nodes
	std::vector<TaxNODE*>	lstTO;	// List of to   nodes
	//--- Path enumerator --------------------------------
	TaxEDGE								 *edge;		// Current edge
	Tag											idne;		// edge ident
	Tag											bran;		// Next  branch
	U_SHORT									numP;		// Path index
	//--- Joint Egde -------------------------------------
	TaxEDGE									joint;	// Joint edge
	//--- Circuit type -----------------------------------
	char										dir;		// TKO or LND
	char										deep;		// deep search
	//----------------------------------------------------
	TaxiwayMGR						 *nsup;		// Node supplier
	//----------------------------------------------------
public:
	TaxiCircuit(TaxiwayMGR *H, char t)	{nsup = H; dir = t;}
 ~TaxiCircuit();
	//-----------------------------------------------------
	int			 Read(SStream *sf,Tag tag);
	//----------------------------------------------------
	char		 NodeColor(TaxNODE *N);
	//----------------------------------------------------
  TaxEDGE *GetEdge(Tag A);
	TaxEDGE *GetEdge(Tag A, Tag B);
	//--- Search a path -----------------------------------
	void		 GetTarget(TaxNODE *D, Tag F);
	void     StoreToPath(Tag F);
	bool		 SearchThePath(Tag D, Tag F);
	bool     CutBack(Tag T);
	//----------------------------------------------------
	TaxEDGE *FirstPathEdge();
	TaxEDGE *NextPathEdge();
	Tag			 RandomEnd(TaxNODE *N);
	Tag			 GetNearTkoSpot(TaxNODE *N);
	//----------------------------------------------------
	void		PackEdge(PACK_EDGE *P);
	void		UnpackEdge(int nbe, void *ze);
	void		UnpackPath(int nbp, void *zp);
  //----------------------------------------------------
	void		AddEdge(TaxEDGE *E);
	void		DeleteEdge(Tag A);
	void		DeleteEdge(Tag A, Tag B);
	void		FreeEdge(TaxEDGE *edg);
	//----------------------------------------------------
	Tag				GetShortCut(TaxEDGE *E);
	TaxNODE  *GetExtremityNode(TaxEDGE *E);
	TaxNODE  *GetShortCutNode(TaxEDGE  *E);
	void			ComputeShortCut(TaxNODE *N, char *lab);
	void			ClearShortCut();
	//--- Circuit testing --------------------------------
	void			CollectNodes();
	void			Test();
	//----------------------------------------------------
	void			Save(Tag dir,Tag pat,CStreamFile &sf);
	//----------------------------------------------------
	std::vector<Tag> &GetPath()						{return path;}
	//-----------------------------------------------------------
	std::map<Tag,TaxEDGE*> &GetEdgeList() {return edgQ;}
};
//================================================================================================
//	Class TaxiwayMgr:  manage taxiway in airport
//================================================================================================
class TaxiwayMGR: public CStreamObject {
protected:
	//--- ATTRIBUTS ---------------------------------------------
	CAptObject		*apo;							// Mother airport
	CAirport      *apt;							// Description
	//--- NODE sequence ------------------------------------------
	U_INT					Seq;
	//----Node path ----------------------------------------------
	TaxNODE      *N0;								// Current Node
	TaxNODE      *N1;
	TaxEDGE      *E0;								// Current Edge
	//--- Taxiway circuits ---------------------------------------
	TaxiCircuit *txx[2];						// Taxi circuits
	//------------------------------------------------------------
	//------------------------------------------------------------
	std::vector<TaxNODE*>	 rwyQ;		// Specific ruway node
	std::map<Tag,TaxNODE*> nodQ;		// TaxNODE
	//------------------------------------------------------------
public:
	TaxiwayMGR(CAptObject *apo);
  ~TaxiwayMGR();
	//-------------------------------------------------------------
	//--- Set airport ---------------------------------------------
	void	LoadAirport();
	bool	ReadTheFile();
	int		Read(SStream *sf,Tag tag);
	//--- Node holder functions -----------------------------------
	void	TaxiwayMGR::EnterNode(TaxNODE *N);
	TaxNODE* GetNode(Tag org);
	TaxNODE* NewNode(char type);
	TaxNODE* DupNode(TaxNODE *S,SPosition &P,char T);
	void		 DeleteNode(Tag A);
	//--- Path computation ----------------------------------------
	void	SetExitPath(LND_DATA *lnd, TaxiRoute *txr);
	void  GetTkofPath(char     *rid, TaxiRoute *txr);
	//-------------------------------------------------------------
	void	SaveItems(CStreamFile &sf);
	void	SaveToFile(TaxiTracker &T);
	void	SaveToBase(TaxiTracker &T);
	//-------------------------------------------------------------
	bool	NotEmpty()	{return (nodQ.size() != 0);}
	//-------------------------------------------------------------
	TaxiCircuit *GetTkoCircuit()		{return txx[TKO_CIRCUIT];}
	TaxiCircuit *GetLndCircuit()		{return txx[LND_CIRCUIT];}
	//-------------------------------------------------------------
	TaxNODE     *NextCircuitNode(char d);
	TaxNODE	    *HeadCircuitNode()		{return N0;}
	//--------------------------------------------------------------
	bool				 LastNode(TaxNODE *N)	{return (N == N1);}
	//-------------------------------------------------------------
	std::vector<Tag>       &GetPath(char d)	{return txx[d]->GetPath();}
	std::map<Tag,TaxNODE*> &GetNodeList()		{return nodQ;}
	//--------------------------------------------------------------
	void		PackEdge(char d,PACK_EDGE *s)				{return txx[d]->PackEdge(s);}
	void		UnpackEdge(char d,int n, void *z)		{txx[d]->UnpackEdge(n,z);}
	void		UnpackPath(char d,int n, void *z)		{txx[d]->UnpackPath(n,z);}
};

//================================================================================================
//	Class Taxiway tracker
//================================================================================================
class TaxiTracker: public CExecutable, public Tracker, public CStreamObject {
	//--- ATTRIBUTES -------------------------------------
	U_INT		 Seq;								// Node sequence
	U_INT		 Prk;								// Parking number
	TaxNODE *selN;							// Selected node
	//--- Link position ----------------------------------
	SPosition *lpos;						// Link position
	//--- Quadric ----------------------------------------
	GLUquadricObj *disk;
	//--- Node supplier ----------------------------------
	TaxiwayMGR *nsup;
	//--- Circuits ---------------------------------------
	TaxiCircuit *circuit;						// Current circuit
	TaxiCircuit *tko;								// take-off circuit
	TaxiCircuit *lnd;								// landing circuit
	//--- path indicator ----------------------------------
	char  edtm;											// Edit mode
	//--- Buffers -----------------------------------------
	char  bf1[64];									// Ident
	//--- Iterators ---------------------------------------
	std::map<Tag,TaxNODE*>::iterator rn;
	std::map<Tag,TaxEDGE*>::iterator re;
protected:
	//--- METHODS ----------------------------------------
public:
	TaxiTracker();
 ~TaxiTracker();
  //-----------------------------------------------------
  //int				Read(SStream *sf,Tag tag);
	void			NodeSupplier(TaxiwayMGR *M);
	//-----------------------------------------------------
	void			Attach(Tag A, Tag B);
	void			DelNode();
	void			MoveSelectedNode(SPosition &P);
	void			NodeProp(Tag N);
	//--- Path management ---------------------------------
	bool			GetThePath(Tag D, Tag F); 
	//-----------------------------------------------------
	char			NodeColor(TaxNODE *N);
	//-----------------------------------------------------
	char			NodeType(char t1,char t2);
	void			SwapRunway(Tag N);
	//-----------------------------------------------------
	void			ComputeAllShortCut();
	//-----------------------------------------------------
	void			CreateRWY(CRunway *rwy);
	Tag			  InsertNode(Tag A,Tag B);
	Tag				AddBrNode(Tag A, SPosition &P);
	//-----------------------------------------------------
	void			SetSelection(Tag id);
	Tag				GetSelected();
	//------------------------------------------------------
	void			SetCircuit(char S) {circuit = (S)?(tko):(lnd);}
	//--- Drawing ------------------------------------------
	void			Draw();
	void			DrawMarks();
	void			DrawEdges();
	void			DrawPath();
	void			DrawLink();
	void			DrawNode(TaxNODE *N);
	void			DrawLine(SPosition &P, SPosition &Q);
	void			DrawEdge(TaxEDGE *edg);
	//---inline operators ---------------------------------
	void			SetCursor(SPosition *P)	{lpos = P;}
	void			NodeNumber(Tag N) {_snprintf(bf1,32,"N:%04u",N);}
	void			DeleteEdge(Tag A,Tag B)	{circuit->DeleteEdge(A,B);}
	void			EditMode(char m)	{edtm = m;}
	void			Test()	{circuit->Test();}
	//-----------------------------------------------------------
};
//================================================================================================
//	Class Taxiway editor
//================================================================================================
class CFuiTaxi: public CFuiWindow 
{	//--- ATTRIBUTES ---------------------------------------
	CFuiLabel			*cplb;							// Cursor position
	CFuiCheckBox	*inpBOX;
	CFuiCheckBox  *outBOX;
	CFuiLabel     *nodDEB;
	CFuiLabel     *nodEND;
	CFuiButton    *modBUT;
	//-- Taxiway tracker -----------------------------------
	TaxiTracker trak;
	Tag					nodA;							// Edge A
	Tag					selT;							// Selected tag
	int					sx,sy;						// Screen coordinates
	//--- Path pparameters ---------------------------------
	Tag					dTag;							// Start tag
	Tag					eTag;							// End tag
	//--- Moving coordinates -------------------------------
	int					keym;							// Key modifier
	int					mx,my;
	//-- Buffer --------------------------------------------
	char        buf[32];
	//-- Taxiways Management -------------------------------
	TaxiwayMGR *nsup;							// Node supplier
	CAptObject *apo;							// Airport to edit
	CAirport   *apt;							// Description
	SPosition		org;							// Airport origin
	SPosition   geop;							// Center screen position
	SPosition   cpos;							// Cursor geoposition
	//--- Menus --------------------------------------------
  FL_MENU     menu;             // menu descriptor
	//--- Various modes -------------------------------------------
	char        path;							// Path mode
	char        mode;							// Mouse mode
	char        inps;							// input state
	//---Original context -----------------------------------------
  CAMERA_CTX       ctx;         // Original camera and situation
	//--- Rabbit camera -----------------------------------------
	CRabbitCamera   *rcam;				// Object camera
	//--- METHODS -----------------------------------------------
public:
	CFuiTaxi(Tag idn, const char *filename);
 ~CFuiTaxi();
	//------------------------------------------------------------
	void  Validate();
	void	InitAirport();
	void	SwapCircuit();
	void	SwapPath();
	//------------------------------------------------------------
	void	CursorGeop(int mx, int my);
	void	PositionEdit(SPosition P, CFuiLabel *lab, char *type);
	void	NewPosition(SPosition P);
	//--- Click routines -----------------------------------------
  void	LinkMode(char M);
	void	PickMode(U_INT No);
	bool	MoveMode(int mx,int my);
	bool	NodeMode();
	//------------------------------------------------------------
	// Mouse management methods
	bool	ActeMenu(int mx,int my);
	bool  InsideMove (int x, int y);
	bool	MouseCapture   (int mx, int my, EMouseButton bt);
	bool	StopClickInside(int mx, int my, EMouseButton bt);
	bool	MousePicking(int mx, int my);
	void	OnePicking(U_INT No);
	bool	MoveNode(int mx, int my);
	bool	MoveWorld(int mx, int my);
	//------------------------------------------------------------
	void  InsertNode();
	void	AttachNodes();
	void	AddBlueNode();
	void	DeleteEdge();
	//------------------------------------------------------------
	void	SetStartNode(Tag T);
	void	SetEndNode(Tag T);
	Tag		GetThePath();
	//------------------------------------------------------------
	void	TimeSlice();
	//--- Menu management ----------------------------------------
	int		ClickActeMENU(short itm);
	//--- Save management ----------------------------------------
	void	SaveToFile();
	bool	GetTaxiMGR();
	//--- Notifications ------------------------------------------
	void	NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
	void	NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn);
	//--- Bypass check profile step  -----------------------------
	bool  CheckProfile(char a) {return true;}
};

//======================= END OF FILE ==============================================================
#endif // VERSION_H
