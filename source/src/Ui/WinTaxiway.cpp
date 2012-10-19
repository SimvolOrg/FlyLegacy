//============================================================================================
// WinTaxiway.cpp
//
// Part of Fly! Legacy project
//
// Copyright 2005 Chris Wallace
// Copyright 2007 Jean Sabatier
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
//===========================================================================================
#include <math.h>
#include "../Include/WinTaxiway.h"
//===========================================================================================
//===========================================================================================
//	Menu for editing 
//===========================================================================================
char *taxiMENU[] = {
	0,
	"----------------",
	"1) Insert a node",
	"2) Link/add nodes",
	"3) Delete node",
	"4) Attach 2 nodes",
	"5) Swap runway",
	"6) Delete edge",
	"7) Start node",
	"8) End   node",
	""};
//============================================================================
//	Arrow
//============================================================================
double arrowTAB[] = {
	-8,  -25, 0,					// C
	 0,  -20, 10,					// B
	 0,  - 5,  0,					// A
	+8,  -25, 0,					// D
};
//============================================================================
//	Node  color by type
//============================================================================
char nodeCOLOR[] = {
	COLOR_BLACK_TRANS,					// 0 Limit (not a type)
	COLOR_BLACK_OPAQUE,					// 1 Exit 
	COLOR_BLACK_OPAQUE,					// 2 Take-off
	COLOR_LIGHT_BLUE,						// 3 Taxiway
	COLOR_YELLOW,								// 4 Parking
	0,
	0,
	0,
	0,
};
//============================================================================
//	Node type matrix
//============================================================================
char taxiNODE[]	= {
	//--- From RUNWAY LIMIT ------------------------------
	0,0,0,0,0,
	//-From EXIT--1:EXIT---------2:TKOF---------3:TAXI---------4:PARK -----------
	0,					  TAXI_NODE_EXIT,TAXI_NODE_EXIT,TAXI_NODE_TAXI,TAXI_NODE_TAXI,
	//-From TKOF--1:EXIT---------2:TKOF---------3:TAXI---------4:PARK -----------
	0,            TAXI_NODE_EXIT,TAXI_NODE_EXIT,TAXI_NODE_TAXI,TAXI_NODE_TAXI,
	//-From TAXI--1:EXIT---------2:TKOF---------3:TAXI---------4:PARK -----------
	0,						TAXI_NODE_TAXI,TAXI_NODE_TAXI,TAXI_NODE_TAXI,TAXI_NODE_TAXI,
	//-From PARK--1:EXIT---------2:TKOF---------3:TAXI---------4:PARK -----------
	0,						TAXI_NODE_TAXI,TAXI_NODE_TAXI,TAXI_NODE_TAXI,TAXI_NODE_TAXI,
};
//============================================================================
//	Color versus direction
//============================================================================
char direCOLOR[] = {
	COLOR_RED,									// 0 == Hi
	COLOR_PURE_GREEN,						// 1 == low
};
//===========================================================================================
//	Node type
//===========================================================================================
char *nodetypeLIST[] = {
	"",													// 0 Limit
	"Exit",											// 1 Exit node
	"Take-Off",									// 2 Take-off
	"Taxiway",									// 3 Taxiway
	"Parking",									// 4 Parking node
	"",													// 5
	"",													// 6
	"",													// 7
};
//===========================================================================================
//	mode
//===========================================================================================
char *modebutTAXI[] = {
	"Search path",
	"Back to mode edit",
};
//===========================================================================================
//	Circuit name
//===========================================================================================
char *circuitNAME[] = {
	"Take-off",
	"Landing",
};
//===========================================================================================
//	Taxiway Route
//===========================================================================================
NavRoute::NavRoute()
{}
//----------------------------------------------------------------------------
//	Destroy route
//----------------------------------------------------------------------------
NavRoute::~NavRoute()
{	Clear();	}
//----------------------------------------------------------------------------
//	Clear all resources 
//----------------------------------------------------------------------------
void NavRoute::Clear()
{	rwd	= 0;
	ind	= 0;
	nodQ.clear();
}
//----------------------------------------------------------------------------
//	Set Route :  Collect all nodes 
//----------------------------------------------------------------------------
void NavRoute::SetRoute(TaxiwayMGR *M,char D,LND_DATA *R)
{	mgr	= M;
	dir	= D;
	rwd	= R;
	TaxNODE *N	 = mgr->HeadCircuitNode();
	while (N)
	{	nodQ.push_back(N);
		N = mgr->NextCircuitNode(dir);
	}
	//----------------------------------------
	return;
}
//----------------------------------------------------------------------------
//	Get next position 
//----------------------------------------------------------------------------
SPosition *NavRoute::NextPosition()
{ if (ind >= nodQ.size())		return 0;
	TaxNODE *N = nodQ[ind++];
	SPosition *pos = N->AtPosition();
	return pos;
}

//----------------------------------------------------------------------------
//	Check for last leg 
//----------------------------------------------------------------------------
bool NavRoute::LastLeg()
{	return (ind == nodQ.size());
}
//===========================================================================================
//	Taxiway circuit
//===========================================================================================
TaxiCircuit::~TaxiCircuit()
{	std::map<Tag,TaxEDGE*>::iterator re;
	for (re = edgQ.begin(); re != edgQ.end(); re++)
	{	delete (*re).second;
	}
	edgQ.clear();
	allP.clear();
}
//--------------------------------------------------------------------------
//	Add an Edge
//--------------------------------------------------------------------------
void TaxiCircuit::AddArc(TaxEDGE *E)
{	Tag key = E->idn;
	std::map<Tag,TaxEDGE*>::iterator re = edgQ.find(key);
	if (re != edgQ.end())		return;
  Tag org = LEFT_NODE(key);
	TaxNODE *n1 = nsup->GetNode(org);
	if (n1)	 n1->IncOUT(dir);
	Tag ext = RITE_NODE(key);
	TaxNODE *n2 = nsup->GetNode(ext);
	if (n2)  n2->IncINP(dir);
	edgQ[key] = E;
	return;
}
//-------------------------------------------------------------------------------------------
//	Free one edge
//-------------------------------------------------------------------------------------------
void TaxiCircuit::FreeArc(TaxEDGE *edg)
{	Tag key = edg->idn;
	Tag org = LEFT_NODE(key);
	Tag ext = RITE_NODE(key);
	TaxNODE *n1 = nsup->GetNode(org);
	if (n1)	 n1->DecOUT(dir);
	TaxNODE *n2 = nsup->GetNode(ext);
	if (n2)  n2->DecINP(dir); 
	delete edg;
}
//-------------------------------------------------------------------------------------------
//	Delete all edges refering to node A
//-------------------------------------------------------------------------------------------
void TaxiCircuit::DeleteArc(Tag A)
{	std::map<Tag,TaxEDGE *>::iterator r0  = edgQ.begin();
	while (r0 != edgQ.end())
	{	TaxEDGE *edg = (*r0).second;
		Tag org = LEFT_NODE(edg->idn);
		Tag ext = RITE_NODE(edg->idn);
		bool dl = (org == A) || (ext == A);
		if (!dl)	{r0++; continue;}
		FreeArc(edg);		
		edgQ.erase(r0++);
	}
	return;
}
//-------------------------------------------------------------------------------------------
//	Delete specifi edge AB if it exists
//-------------------------------------------------------------------------------------------
int TaxiCircuit::DeleteEdge(Tag A, Tag B)
{	Tag key = (A << 16) | B;
	std::map<Tag,TaxEDGE*>::iterator r0 = edgQ.find(key);
	if (r0 == edgQ.end())		return 0;
	FreeArc((*r0).second);
	edgQ.erase(r0);
	return 1;
}
//-------------------------------------------------------------------------------------------
//	return first edge originating from A
//-------------------------------------------------------------------------------------------
TaxEDGE *TaxiCircuit::GetArc(Tag A)
{	std::map<Tag, TaxEDGE*>::iterator r0;
	for (r0 = edgQ.begin(); r0 != edgQ.end(); r0++)
	{	TaxEDGE *edg = (*r0).second;
		Tag org = LEFT_NODE(edg->idn);
		if (org == A)	return edg;
	}
	return 0;
}
//-------------------------------------------------------------------------------------------
//	return edge AB
//-------------------------------------------------------------------------------------------
TaxEDGE *TaxiCircuit::GetArc(Tag A, Tag B)
{	Tag key = (A << 16) | B;
	std::map<Tag,TaxEDGE *>::iterator r0  = edgQ.find(key);
	return (r0 == edgQ.end())?(0):((*r0).second);
}
//-------------------------------------------------------------------------------------------
//	Return shorcut node from edge
//-------------------------------------------------------------------------------------------
TaxNODE *TaxiCircuit::GetShortCutNode(TaxEDGE *E)
{	Tag ts = LEFT_NODE(E->scut);
	return nsup->GetNode(ts);
}
//-------------------------------------------------------------------------------------------
//	Save path if it is shorter than previous
//-------------------------------------------------------------------------------------------
void TaxiCircuit::StoreToPath(Tag F)
{ //--- Compare number of nodes -----------------
	U_INT nba	= path.size();				// Actual size
	U_INT nbw = work.size();				// New one size
	bool best = (nba == 0) || (nbw < nba);
	if ( !best)		return;
	path.clear();
	path = work;				// Save new path
	//TRACE("...Found F= %03d",F);
	return;
}
//-------------------------------------------------------------------------------------------
//  If edge T is already visited, then we have a loop and must break it
//-------------------------------------------------------------------------------------------
bool TaxiCircuit::CutBack(Tag T)
{	for (U_INT k=0; k < work.size(); k++)
	{	if (work[k] == T)	return true;}
	return false;
}
//-------------------------------------------------------------------------------------------
//	Compute path recursilvely from Node D to node F
//	Limit search to 16 branch node
//-------------------------------------------------------------------------------------------
void	TaxiCircuit::GetTarget(TaxNODE *D, Tag F)
{	std::map<Tag,TaxEDGE*>::iterator r0;
	Tag	key = D->idn;
	if (CutBack(key))		return;
	if (key == F)				return StoreToPath(F);
	//TRACE("... Explore node %04u",key);
	for (r0 = edgQ.begin(); r0 != edgQ.end(); r0++)
	{	TaxEDGE *edg = (*r0).second;
		Tag org = LEFT_NODE(edg->idn);
		if (org != key)		continue;
		TaxNODE *nod =  GetShortCutNode(edg);
		if (0 == nod)			continue;
		work.push_back(edg->idn);
		//TRACE("... Cut to %04u",nod->idn);
		deep++;
		GetTarget(nod,F);
		work.pop_back();
	}
	return;
}
//-------------------------------------------------------------------------------------------
//	Search path from Node D to node F
//-------------------------------------------------------------------------------------------
int TaxiCircuit::SearchThePath(Tag D, Tag F)
{	work.clear();
	path.clear();
	deep	= 0;
	TaxNODE *N = nsup->GetNode(D);
	//--- For debug only ------------------------------------------
	//TRACE("==========START SEARCH from %04u to %04u ===========",D,F);
	GetTarget(N,F);
	return path.size();
}
//-------------------------------------------------------------------------------------------
//	Search a random ending slot starting from N
//-------------------------------------------------------------------------------------------
Tag TaxiCircuit::RandomEnd(TaxNODE *N)
{	std::vector<Tag> lstQ;
	Tag key = N->idn;
	for (U_INT k=0; k < allP.size(); k++)
	{	Tag T		= allP[k];
		Tag L   = LEFT_NODE(T);
		if (L != key)	continue;
		lstQ.push_back(T);
	}
	//--- Get a random node --------
	U_INT nb = lstQ.size();
	if (0 == nb)		return 0;
	U_INT nr = RandomNumber(nb);
	Tag  E = lstQ[nr];
	lstQ.clear();
	return RITE_NODE(E);
}
//-------------------------------------------------------------------------------------------
//	Search the nearest node leading to take of spot N
//-------------------------------------------------------------------------------------------
Tag	TaxiCircuit::GetNearTkoSpot(TaxNODE *N)
{ double dist = 5000;
	Tag T = N->idn;
	Tag X = 0;
	for (U_INT k=0; k < allP.size(); k++)
	{	Tag W   = allP[k];
		Tag exm = RITE_NODE(W);
		if (exm != T)		continue;
		//--- Found a starting node --------
		Tag  D  = LEFT_NODE(W);
		TaxNODE *node = nsup->GetNode(D);
		if (0 == node)	continue;
		double   dst  = GetFlatDistance(node->AtPosition());
		if ( dst > dist)	continue;
		//--- Retain this candidate --------
		dist	= dst;
		X			= D;
	}
	//--- if distance greater than 20 feet we are not on a parking-
	return (FN_FEET_FROM_MILE(dist) < 30)?(X):(0);
}
//-------------------------------------------------------------------------------------------
//	We look for the end of a straignt path passign through edge E
//	Thus we get the node NB at the extremity of E
//	When node NB has exactly one output edge, then it is the only path toward an end
//	otherwise NB is either a branching node (several outputs) or a terminal node (no output)
//-------------------------------------------------------------------------------------------
//	This is a recursive function
//-------------------------------------------------------------------------------------------
Tag		TaxiCircuit::GetShortCut(TaxEDGE *E)
{	TaxNODE *nb = GetExtremityNode(E);
	Tag      B  = nb->idn;
	if (nb->EndPath(dir))		return B;				// End of straight path
	TaxEDGE *e1 = GetArc(B);								// There must be one edge starting from NB
	if (e1)			return GetShortCut(e1);			// Go forward
	return 0;
}
//-------------------------------------------------------------------------------------------
//	Return Node opposite A in Edge E
//-------------------------------------------------------------------------------------------
TaxNODE  *TaxiCircuit::GetExtremityNode(TaxEDGE *E)
{	Tag ext = RITE_NODE(E->idn);			// Right part
	return nsup->GetNode(ext);
}
//-------------------------------------------------------------------------------------------
//	Clear all short cuts
//-------------------------------------------------------------------------------------------
void TaxiCircuit::ClearShortCut()
{	std::map<Tag,TaxEDGE*>::iterator r0;
	for (r0 = edgQ.begin(); r0 != edgQ.end(); r0++) (*r0).second->SetShortCut(0);
	return;
}
//-------------------------------------------------------------------------------------------
//	Compute all shortcut for branch node A
//	Branch node is a node that have more than 2 edges
//-------------------------------------------------------------------------------------------
void TaxiCircuit::ComputeShortCut(TaxNODE *N, char *lab)
{	std::map<Tag,TaxEDGE*>::iterator r0;
	Tag A = N->idn;
	if (N->NoOutput(dir))					return;
	//TRACE("%s ALL SHORTCUT for NODE %04u",lab,A);
	for (r0 = edgQ.begin(); r0 != edgQ.end(); r0++)
	{	TaxEDGE *E = (*r0).second;
		if (E->NoCandidate(A))			continue;
		Tag C = GetShortCut(E);
		Tag K = E->idn;
		//TRACE("... EDGE (%04u-%04u) A=%04u CUT=%04u",LEFT_NODE(K),RITE_NODE(K),A,C);
		E->SetShortCut(C);
	}

	return;
}

//===========================================================================================
//	Collect all nodes for test
//===========================================================================================
void TaxiCircuit::CollectNodes()
{ std::map<Tag,TaxNODE*> &linp = nsup->GetNodeList();
  std::map<Tag,TaxNODE*>::iterator r0;  
	for (r0 = linp.begin(); r0 != linp.end(); r0++)
	{	TaxNODE *N = (*r0).second;
		switch (dir)	{
			//--- Take-of circuit ---------------------
			case 0:
				if (N->IsTkoNode())		lstTO.push_back(N);
				if (N->TkoParking())	lstFR.push_back(N);
				continue;
			//--- Landing circuit ---------------------
			case 1:
				if (N->IsLndNode())		lstFR.push_back(N);
				if (N->LndParking())	lstTO.push_back(N);
				continue;
		}
	}								// End of collect
	return;
}
//----------------------------------------------------------------------
//	Test circuit
//----------------------------------------------------------------------
void TaxiCircuit::Test()
{	char *cn = circuitNAME[dir];
	CollectNodes();
	allP.clear();						// Clear all paths
	TRACE("===== TESTING %s CIRCUIT ======================",cn);
	for (U_INT m=0; m < lstFR.size(); m++)
	{	TaxNODE *P = lstFR[m];
		Tag      A = P->idn;
		//---- Check every path from this node ------------------
		for (U_INT n=0; n < lstTO.size(); n++)
		{	TaxNODE *Q = lstTO[n];
			Tag      B = Q->idn;
			int nn = SearchThePath(A,B);
			char *rs = (nn != 0)?("OK"):("No path");
			TRACE("N%04u to N%04u : %s",A,B,rs);
			if (0 == nn)	continue;
			Tag T = (A << 16) | B;
			allP.push_back(T);
		}
	}	
	TRACE("===== END OF %s CIRCUIT ======================",cn);
	lstFR.clear();
	lstTO.clear();
	return;
}
//===========================================================================================
//	Save all edges
//===========================================================================================
//-------------------------------------------------------------------------------------------
//	Pack all edges
//-------------------------------------------------------------------------------------------
void	TaxiCircuit::PackArc(PACK_EDGE *s)
{	//--- Compute all paths ---------------------
	Test();
	//--- Save edges and paths ------------------
	int dm1 = edgQ.size() * TAG_PER_EDGE;
	s->dm1	= dm1;									// 3 integers
	s->bf1  = new U_INT[dm1];
	//--- Pack all edges ----------------------
	U_INT *dst = s->bf1;
	std::map<Tag,TaxEDGE*>::iterator re;
	for (re = edgQ.begin(); re != edgQ.end(); re++)
	{	TaxEDGE *E	= (*re).second;
		*dst++			= E->idn;
		*dst++			= E->scut;
	}
	//--- Pack all paths -----------------------
	int dm2 = allP.size() * TAG_PER_PATH;
	s->dm2	= dm2;
	s->bf2	= new U_INT[dm2];
	//--- Pack all paths -----------------------
	dst	= s->bf2;
	for (U_INT k=0; k < allP.size(); k++)
	{	*dst++ = allP[k];	}
	return;
}
//-------------------------------------------------------------------------------------------
//	Unpack edge
//-------------------------------------------------------------------------------------------
void TaxiCircuit::UnpackArc(int nbr, void *ze)
{	U_INT *src = (U_INT*)ze;
	int		 nbe = nbr / TAG_PER_EDGE; 
	for (int k=0; k<nbe; k++)
	{	Tag	T = *src++;
		Tag S = *src++;
		TaxEDGE *E = new TaxEDGE(LEFT_NODE(T),RITE_NODE(T));
		E->StoreShortCut(S);
		AddArc(E);
	}
	return;
}
//-------------------------------------------------------------------------------------------
//	Unpack path
//-------------------------------------------------------------------------------------------
void TaxiCircuit::UnpackPath(int nbr, void *zp)
{	U_INT *src = (U_INT*)zp;
  int    nbp = nbr / TAG_PER_PATH;
  allP.reserve(nbp);
	for (int k=0; k<nbp; k++)
	{	Tag T = *src++;
		allP.push_back(T);
	}
	return;
}
//===========================================================================================
//	Get first edge from path
//===========================================================================================
TaxEDGE *TaxiCircuit::FirstPathArc()
{	edge	= 0;
	if (path.size() == 0)	return 0;
	numP	= 1;
	idne	= path[0];
	edge	= edgQ[idne];
	bran  = LEFT_NODE(edge->scut);
	return edge;
}
//-------------------------------------------------------------------------------------------
//	Get next edge from path
//-------------------------------------------------------------------------------------------
TaxEDGE *TaxiCircuit::NextPathArc()
{	if (path.size() == 0)			return 0;
	Tag ext = RITE_NODE(edge->idn);
	//--- continue to branch node -----------
	if (ext != bran)
	{	edge	= GetArc(ext);
		return edge;
	}
	//--- change to new branch --------------
	if (numP == path.size())	return 0;
	idne	= path[numP++];
	edge	= edgQ[idne];
	bran  = LEFT_NODE(edge->scut);
	return edge;
}
//===========================================================================================
//	Taxiway tracker
//===========================================================================================
TaxiTracker::TaxiTracker()
{	Seq			= 1;
	Prk			= 1;
	selN		= 0;
	lpos		= 0;
	taxiMENU[0] = bf1;
	disk		= gluNewQuadric();
	gluQuadricDrawStyle(disk,GLU_FILL);
	globals->Disp.Enter(this, PRIO_ABSOLUTE, DISP_EXSTOP, 1);
	nsup		= 0;
}
//--------------------------------------------------------------------------
//	Set node supplier
//--------------------------------------------------------------------------
void	TaxiTracker::NodeSupplier(TaxiwayMGR *M)
{	nsup	= M;
	tko		= M->GetTkoCircuit();
	lnd		= M->GetLndCircuit();
	circuit		= lnd;
}

//--------------------------------------------------------------------------
//	Exit from tracker
//--------------------------------------------------------------------------
TaxiTracker::~TaxiTracker()
{	globals->Disp.Clear(PRIO_ABSOLUTE);
	gluDeleteQuadric(disk);
}
//-------------------------------------------------------------------------------------------
//	Clear all resources
//-------------------------------------------------------------------------------------------
char	TaxiTracker::NodeColor(TaxNODE *N)
{	int c1	= N->type & 0x07;
	int c2	= nodeCOLOR[c1];
	if (c2 != COLOR_BLACK_OPAQUE)	return c2;
	U_CHAR D = N->GetDirection() >> 4;
	return direCOLOR[D];
}
//-------------------------------------------------------------------------------------------
//	Compute  shortcut for all branch node
//	Branch node is a node that have more than 2 edges
//-------------------------------------------------------------------------------------------
void TaxiTracker::ComputeAllShortCut()
{	//--- Clear all shorcut in  edge -------------------
	tko->ClearShortCut();
	lnd->ClearShortCut();
	//--- Compute the shortcuts ------------------------
	std::map<Tag,TaxNODE*>::iterator r1;
	std::map<Tag,TaxNODE*> &ln = nsup->GetNodeList();
	for (r1 = ln.begin(); r1 != ln.end(); r1++)
	{	TaxNODE *N = (*r1).second;
		//-----------------------------------------------
		if (N->IsInpOK())	tko->ComputeShortCut(N,"TKOF");
		if (N->IsOutOK())	lnd->ComputeShortCut(N,"LNDG");
	}
	return;
}
//-------------------------------------------------------------------------------------------
//	Search path
//-------------------------------------------------------------------------------------------
bool	TaxiTracker::GetThePath(Tag D, Tag F)
{	//TRACE("====COMPUTE SHORTCUTS ==========================");
	ComputeAllShortCut();
	return (circuit->SearchThePath(D,F) != 0);
}
//-------------------------------------------------------------------------------------------
//	Create a pair of nodes for runway
//	Create exit point a 3/5 and 4/5 of each runway
//-------------------------------------------------------------------------------------------
void TaxiTracker::CreateRWY(CRunway *rwy)
{	TaxNODE *hi = nsup->NewNode(TAXI_HI_TKOF);
	rwy->FillHiNODE(hi);
	TaxNODE *lo = nsup->NewNode(TAXI_LO_TKOF);
	rwy->FillLoNODE(lo);
	//--- Compute vector director ------------------
	double rat = 0;
	rat		= double (3) / 5;
	SPosition P1	= GetAlignedSpot(hi->Position(),lo->Position(),rat);
	nsup->DupNode(hi,P1,TAXI_HI_EXIT);
	SPosition Q1	= GetAlignedSpot(lo->Position(),hi->Position(),rat);
	nsup->DupNode(lo,Q1,TAXI_LO_EXIT);
	rat		= double (4) / 5;
	SPosition P2	= GetAlignedSpot(hi->Position(),lo->Position(),rat);
	nsup->DupNode(hi,P2,TAXI_HI_EXIT);
	SPosition Q2	= GetAlignedSpot(lo->Position(),hi->Position(),rat);
	nsup->DupNode(lo,Q2,TAXI_LO_EXIT);
	return;
}
//-------------------------------------------------------------------------------------------
//	Delete selected node and all related edges 
//	NOTE: Edges must be deleted from both circuits
//-------------------------------------------------------------------------------------------
void TaxiTracker::DelNode()
{	if (0 == selN)											return;
	if (selN->HasType(TAXI_NODE_FIXE))	return;
	tko->DeleteArc(selN->idn);
	lnd->DeleteArc(selN->idn);
	nsup->DeleteNode(selN->idn);
	selN = 0;
	return;
}
//-------------------------------------------------------------------------------------------
//	Add a edge between A and B
//-------------------------------------------------------------------------------------------
int TaxiTracker::Attach(Tag A, Tag B)
{	if (A == B)							return 0;
	//--- Check for existence of edge AB ---
	TaxEDGE *ab  = circuit->GetArc(A,B);
	if	(ab)								return 0;
	//--- Create an AB edge ----------------
	TaxEDGE *edg = new TaxEDGE();
	Tag  t0   =  (A << 16) | (B & 0x0000FFFF);
	edg->idn	=  t0;
	circuit->AddArc(edg);
	return 1;
}
//-------------------------------------------------------------------------------------------
//	compute node type
//-------------------------------------------------------------------------------------------
char TaxiTracker::TaxiNodeType(char t1,char t2)
{	if (0 == t1)		return 0;
	if (0 == t2)		return 0;
	int indx = (t1 & 0x07) * 5;
	indx += (t2 & 0x07);
	if (indx >= 25)	return 0;
	return taxiNODE[indx];
//	char ta = t1 & t2 & TAXI_NODE_AXES;
//	return (ta)?(TAXI_NODE_EXIT):(TAXI_NODE_TAXI);
}
//-------------------------------------------------------------------------------------------
//	Insert a node between to nodes.  Node type is A
//-------------------------------------------------------------------------------------------
Tag TaxiTracker::InsertNode(Tag A, Tag B)
{	//--- Ensure A and B are consecutive nodes ------------
	TaxNODE *na = nsup->GetNode(A);
	TaxNODE *nb = nsup->GetNode(B);
	//--- Check node A for an edge to B -------------------
	TaxEDGE *edge = circuit->GetArc(A,B);
	if (0 == edge)		return 0;
	//--- Create a new node -------------------------------
	char  T = TaxiNodeType(na->type,nb->type); 
	if (0 == T)				return 0;
	char *R = (T == TAXI_NODE_EXIT)?(na->rwy):("TXI");
	//--- Delete edge -------------------------------------
	SPosition P = GetAlignedSpot(na->Position(),nb->Position(),0.5);
	T			|= na->GetDirection();
	TaxNODE *nm = nsup->DupNode(na,P,T);
	nm->SetRWY(R);
	Tag   C = nm->idn;
	//-- Remove edge and relink with new node -------------
	if (edge)
	{	circuit->DeleteEdge(A,B);
		Attach(A,C);
		Attach(C,B);
	}
	return C;
}
//-------------------------------------------------------------------------------------------
//	Swap runway for this node
//-------------------------------------------------------------------------------------------
void TaxiTracker::SwapRunway(Tag N)
{ selN = nsup->GetNode(N);
	if (0 == selN)			return;
	selN->SwapRunway();
	return;
}
//-------------------------------------------------------------------------------------------
//	Add a node and add one edge
//-------------------------------------------------------------------------------------------
Tag TaxiTracker::AddOneNode(Tag A, Tag B,SPosition &P)
{	TaxNODE *nodB;	
	if (0 == A)	return 0;
	TaxNODE *nodA	= nsup->GetNode(A);
	//--- Check for a clicked node ------------------
	if (B)	nodB = nsup->GetNode(B);
	//--- Create a new node -------------------------
	else		nodB = nsup->NewNode(TAXI_NODE_TAXI);
	if (0 == B)	nodB->SetPosition(P);
	nodB->SetRWY("TXI");
	Attach(nodA->idn,nodB->idn);
	SetSelection(nodB->idn);
	return nodB->idn;
}
//-------------------------------------------------------------------------------------------
//	Selected node receive a new position
//-------------------------------------------------------------------------------------------
int TaxiTracker::MoveSelectedNode(SPosition &P)
{	if (0 == selN)											return 0;
  //if (selN->HasType(TAXI_NODE_FIXE))	return 0;
	selN->SetPosition(P);
	return 1;
}
//-------------------------------------------------------------------------------------------
//	Edit property in menu
//-------------------------------------------------------------------------------------------
void TaxiTracker::NodeProp(Tag N)
{	strcpy(bf1,"...");
	selN = nsup->GetNode(N);
	if (0 == selN)			return;
	//--- index type -----------------
	char typ = selN->type;
	char ind = typ & 0x7;
	char *edt = nodetypeLIST[ind]; 
	
	if (typ == TAXI_NODE_TAXI) _snprintf(bf1,64,"N:%04u Taxiway",N);
	else _snprintf(bf1,64,"N:%04u %s rwy %s Sec:%03u",N,edt,selN->rwy,selN->sector);
	return;
}
//-------------------------------------------------------------------------------------------
//	Draw all node in picking mode
//-------------------------------------------------------------------------------------------
void TaxiTracker::DrawMarks()
{	std::map<Tag,TaxNODE*>::iterator r0;
  std::map<Tag,TaxNODE*> &ln = nsup->GetNodeList();
	for (r0 = ln.begin(); r0 != ln.end(); r0++)
		{	TaxNODE *node = (*r0).second;
			bool ok = (node != selN) || globals->clk->IsOn();
			if (ok) DrawNode(node);
		}
	return;
}
//-------------------------------------------------------------------------------------------
//	Draw a line between 2 positions
//-------------------------------------------------------------------------------------------
void TaxiTracker::DrawLine(SPosition &P, SPosition &Q)
{	SPosition R = globals->geop;
	double  rdf = globals->rdf;
	CVector A		= FeetComponents(R,P,rdf);
	CVector B   = FeetComponents(R,Q,rdf);
	//--- Draw a line ----------------------------
	
	glBegin(GL_LINES);
	glVertex3d(A.x,A.y,A.z+1);
	glVertex3d(B.x,B.y,B.z+1);
	glEnd();
	glPushMatrix();
	//--- Draw arrow pointer ---------------------
	glTranslated(B.x,B.y,B.z);
	double dx = B.x - A.x;
	double dy = B.y - A.y;
	double atn = atan2(-dx,dy);
	double deg = RadToDeg(atn);
	glRotated(deg,0,0,1);
	//--- Draw Arrow ----------------------------
	glVertexPointer(3,GL_DOUBLE,0,arrowTAB);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	glPopMatrix();
}
//-------------------------------------------------------------------------------------------
//	Draw the floating link
//-------------------------------------------------------------------------------------------
void TaxiTracker::DrawLink()
{	if (0 == selN)		return;
	if (0 == lpos)		return;
	GroundSpot lnd(lpos->lon,lpos->lat);
  lpos->alt = globals->tcm->GetGroundAt(lnd);
	ColorGL(COLOR_WHITE);
	DrawLine(selN->Position(),*lpos);
}
//-------------------------------------------------------------------------------------------
//	Draw one node in picking mode
//-------------------------------------------------------------------------------------------
void TaxiTracker::DrawNode(TaxNODE *N)
{	SVector T = FeetComponents(globals->geop,N->Position(),globals->rdf);
  char col = NodeColor(N);
  ColorGL(col);
	glLoadName(N->idn);
	glPushMatrix();
	glTranslated(T.x, T.y, T.z);
	gluSphere(disk,10,12,12);	
	glPopMatrix();
}
//-------------------------------------------------------------------------------------------
//	Draw one edge
//-------------------------------------------------------------------------------------------
void TaxiTracker::DrawEdges()
{	std::map<Tag,TaxEDGE*> &edgQ = circuit->GetArcList();
	std::map<Tag,TaxEDGE*>::iterator r0;
	ColorGL(COLOR_WHITE);
	for (r0 = edgQ.begin(); r0 != edgQ.end(); r0++) DrawEdge((*r0).second);
}
//-------------------------------------------------------------------------------------------
//	Draw Path
//-------------------------------------------------------------------------------------------
void TaxiTracker::DrawPath()
{	ColorGL(COLOR_RED);
	TaxEDGE *edg = circuit->FirstPathArc();
	while (edg)
	{	DrawEdge(edg);
		edg = circuit->NextPathArc();
	}
	return;
}
//-------------------------------------------------------------------------------------------
//	Draw one edge
//-------------------------------------------------------------------------------------------
void TaxiTracker::DrawEdge(TaxEDGE *edg)
{	//--- Extract A node -------------------------------
	Tag ta	= LEFT_NODE(edg->idn);									// A node
	TaxNODE *na = nsup->GetNode(ta);
	if (0 == ta)	return;
	//--- Extract B node ------------------------------
	Tag tb	= RITE_NODE(edg->idn);
	TaxNODE *nb = nsup->GetNode(tb);
	if (0 == nb)	return;
	DrawLine(na->Position(),nb->Position());
	return;
}

//-------------------------------------------------------------------------------------------
//	Normal draw:
//-------------------------------------------------------------------------------------------
void TaxiTracker::Draw()
{	if (0 == nsup)		return;
	std::map<Tag,TaxEDGE*>::iterator r1;
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	DrawMarks();
	glEnableClientState(GL_VERTEX_ARRAY);
	//---Normal mode edit ----------------------
	if (edtm == 1)	DrawPath();
	else
	{	DrawLink();
		DrawEdges();
	}
	glPopAttrib();
}
//-------------------------------------------------------------------------------------------
//	One node is selected
//-------------------------------------------------------------------------------------------
void TaxiTracker::SetSelection(Tag No)
{	selN = nsup->GetNode(No);
	return;
}
//-------------------------------------------------------------------------------------------
//	Return selection ident
//-------------------------------------------------------------------------------------------
Tag TaxiTracker::GetSelected()
{	if (0 == selN)			return 0;
	return selN->idn;
}
//===========================================================================================
//	Taxiway editor
//===========================================================================================
CFuiTaxi::CFuiTaxi(Tag idn, const char *filename)
	:CFuiWindow(idn,filename,300,200,0)
{	//--- Initial modes -------------------------------------
	mode	= TAXI_MODE_PICKING;
	modf	= 0;
	inps	= 1;
	path	= 1;
	dTag	= eTag = 0;
	//---BuildComponents ------------------------------------
	CFuiButton	 *butn = 0;
	CFuiGroupBox *box  = 0;
	U_INT wit = MakeRGBA(255,255,255,255);	// White color
	//--- Label group ----------------------------------------
	box  = new CFuiGroupBox (  4 , 8, 290, 50, this);
  AddChild('gbox',box,"");
	//--- Cursor position ------------------------------------
  cplb  = new CFuiLabel   (  4, 24, 270, 20, this);
  box->AddChild('cplb',cplb,"Cursor:",wit);
	//--------------------------------------------------------
	box  = new CFuiGroupBox (  4 ,70, 290, 50, this);
  AddChild('gbx1',box,"");
	//---Input circuit ---------------------------------------
  inpBOX  = new CFuiCheckBox( 10, 4, 100,20,this);
  box->AddChild('inps',inpBOX,"Take-off Circuit",wit);
	//---Output circuit -------------------------------------
  outBOX  = new CFuiCheckBox( 10, 24, 100,20,this);
  box->AddChild('outs',outBOX,"Landing  Circuit",wit);
	//--- Test button ----------------------------------------
	butn		= new CFuiButton(  120, 10, 94, 20, this);
	box->AddChild('test',butn,"Test circuit"); 
	//--------------------------------------------------------
	butn		= new CFuiButton(  10, 124, 60, 20, this);
	AddChild('cent',butn,"Center");
	//--------------------------------------------------------
	modBUT	= new CFuiButton(  110, 124, 110, 20, this);
	AddChild('path',modBUT,"Mode Edit");
	//--------------------------------------------------------
	box  = new CFuiGroupBox (  4 ,150, 290, 50, this);
  AddChild('gbx2',box,"");
	//--- Starting node -------------------------------------
	nodDEB  = new CFuiLabel( 4,  2,  280, 20,this);
	box->AddChild('ndeb',nodDEB,"Start node:",wit);
	//--- Ending node -------------------------------------
	nodEND  = new CFuiLabel( 4, 22,  280, 20,this);
	box->AddChild('nend',nodEND,"End   node:",wit);

	//--------------------------------------------------------
	SetTransparentMode();
	//--- Set Camera type and parameters ---------------------
	ctx.prof	= PROF_TAXI;
	ctx.mode	= 0;
  rcam			= globals->ccm->SetRabbitCamera(ctx,RABBIT_CONTROL);
	rcam->Prof.Rep(CAM_MAY_MOVE);			// Restrict camera movement
	Validate();
	InitAirport();
	ReadFinished();
	selT	= 0;
	SwapCircuit();
	SwapPath();
	//---------------------------------------------------------
	rcam->SetTracker(&trak, this);
	globals->fui->CaptureMouse(this);
	//--------------------------------------------------------------------
	globals->Disp.ExecON  (PRIO_ABSOLUTE);	// Allow terrain TimeSlice
	globals->Disp.ExecOFF (PRIO_TERRAIN);		// Nothing exe after terrain
	globals->Disp.DrawON  (PRIO_ABSOLUTE);	// Drawing
	globals->Disp.ExecLOK (PRIO_WEATHER);		// No weather
	//---- Marker 1 -----------------------------------
	char *ds = new char[32];
	strcpy(ds,"*START TaxiEDIT*");

}

//------------------------------------------------------------------------------
//	Destroy and free resources
//------------------------------------------------------------------------------
CFuiTaxi::~CFuiTaxi()
{	SaveData();
	//--- Restore State ---------------------------------
	globals->Disp.DrawON (PRIO_TERRAIN);		// Allow Terrain TimeSlice
	globals->Disp.ExecON (PRIO_TERRAIN);		// Allow Terrain Draww
	globals->Disp.ExecULK(PRIO_WEATHER);
	//---- Marker 2 -------------------------------------
	char *ds = Dupplicate("*END taxiEDIT*",32);
	globals->ccm->RestoreCamera(ctx);
}
//------------------------------------------------------------------------------
//	Swap circuit number
//------------------------------------------------------------------------------
void CFuiTaxi::SwapCircuit()
{	outBOX->SetState(inps);
	inps ^= 1;
	inpBOX->SetState(inps);
	trak.SetCircuit(inps);
	return;
}
//------------------------------------------------------------------------------
//	Swap mode path
//------------------------------------------------------------------------------
Tag	CFuiTaxi::GetThePath()
{	if (0 == path)		return 0;
	if (0 == eTag)		return 0;
	if (0 == dTag)		return 0;
	trak.GetThePath(dTag,eTag);
	return 1;
}
//------------------------------------------------------------------------------
//	Swap mode path
//------------------------------------------------------------------------------
void CFuiTaxi::SwapPath()
{	path ^= 1;
	path  = GetThePath();
	char *lab = modebutTAXI[path];
	modBUT->SetText(lab);
	trak.EditMode(path);
	return;
}
//------------------------------------------------------------------------------
//	Validation:  We must have a nearest airport
//------------------------------------------------------------------------------
void	CFuiTaxi::Validate()
{	apo	= globals->apm->GetNearestAPT();
	if (0 == apo)	return Close();
	//--- Go to airport center --------------------------
	apt	= apo->GetAirport();
	org	= apo->GetOrigin();
	NewPosition(org);											// Start at airport center
	rcam->SetAbove(globals->geop,30000,5000);
	//--- Edit title ------------------------------------
	char edt[128];
	_snprintf(edt,128,"TAXIWAY EDITOR for %s", apt->GetName());
	SetText(edt);
	return;
}
//------------------------------------------------------------------------------
//	Init airport with node and edge
//------------------------------------------------------------------------------
void CFuiTaxi::InitAirport()
{	if (GetTaxiMGR())     return;
	//--- Create initial nodes --------------------------
	ClQueue  *rwyQ = apt->GetRWYQ();
	for (CRunway *rwy = (CRunway *)rwyQ->GetFirst(); rwy != 0; rwy = rwy->GetNext())
	{	trak.CreateRWY(rwy);	}
	return;
}
//------------------------------------------------------------------------------
//	Set a new position
//------------------------------------------------------------------------------
void CFuiTaxi::NewPosition(SPosition P)
{	//--- Get terrain altitude -------------------------
	GroundSpot lnd(P.lon,P.lat);
  P.alt = globals->tcm->GetGroundAt(lnd);
	globals->geop = P;
	geop	= P;
}
//------------------------------------------------------------------------------
//	Edit geo position
//------------------------------------------------------------------------------
void CFuiTaxi::PositionEdit(SPosition P, CFuiLabel *lab,char *T)
{	char tlat[128];
	char tlon[128];
	char edt[128];
	EditLat2DMS(P.lat,tlat);
  EditLon2DMS(P.lon,tlon);
	sprintf(edt,"%s %-20s %-20s",T,tlon,tlat);
	lab->SetText(edt);
}
//------------------------------------------------------------------------------
//	Compute geo position under cursor
//	We set the origin of pixel at glut window center
//	NOTE: mx and my are normally relative to the glut window, 
//				not to the full screen
//------------------------------------------------------------------------------
void	CFuiTaxi::CursorGeop(int mx, int my)
{	int xorg = globals->cScreen->Width  >> 1;					// pixel origins in screen
	int yorg = globals->cScreen->Height >> 1;
	//------ Compute pixel deltas --------------------------
	int dx	= +(mx - xorg);
	int dy  = -(my - yorg);
	double fpp = rcam->GetFPIX();
	CVector dta((fpp * dx),(fpp * dy),0);
	cpos	= AddToPositionInFeet(geop,dta);	//,globals->exf);
	return;
}
//------------------------------------------------------------------------------
//	A node is selected from camera picking mode
//------------------------------------------------------------------------------
void CFuiTaxi::OnePicking(U_INT No)
{	selT	= No; }
//------------------------------------------------------------------------------
//	Enter link mode
//------------------------------------------------------------------------------
void CFuiTaxi::LinkMode(char M)
{	nodA	= trak.GetSelected();
	if (0 == nodA)	return;
	mode = M;														
	trak.SetCursor(&cpos);
	return;
}
//------------------------------------------------------------------------------
//	Enter Pick mode with a selected node (or 0)
//------------------------------------------------------------------------------
void CFuiTaxi::PickMode(U_INT No)
{	selT	= No;
	mode	= TAXI_MODE_PICKING;
	trak.SetCursor(0);
	trak.SetSelection(selT);
	return;
}
//------------------------------------------------------------------------------
//	Enter Move mode 
//------------------------------------------------------------------------------
bool CFuiTaxi::MoveMode(int x, int y)
{	mode	= TAXI_MODE_MOVING;
	mx		= x;
	my		= y;
	trak.SetCursor(0);
	return true;
}
//------------------------------------------------------------------------------
//	Enter Move Node 
//------------------------------------------------------------------------------
bool CFuiTaxi::NodeMode()
{	mode = TAXI_MODE_NODEMV;
	trak.SetCursor(0);
	return true;
}
//------------------------------------------------------------------------------
//	Moving screen
//------------------------------------------------------------------------------
bool CFuiTaxi::MoveWorld(int x, int y)
{	int mod  = glutGetModifiers();
	bool out = (GLUT_ACTIVE_ALT != mod);
  if (out)		{PickMode(0); return true;}
	int dx	= mx - x;							// X delta
	int dy	= y  - my;						// Y delta
	mx			= x;
	my			= y;
	double fpp = rcam->GetFPIX();
	CVector dta((fpp * dx),(fpp * dy),0);
	geop		= AddToPositionInFeet(geop,dta);
	globals->geop = geop;
	return true;
}
//------------------------------------------------------------------------------
//	Moving node or screen 
//------------------------------------------------------------------------------
bool CFuiTaxi::MoveNode(int mx, int my)
{	int mod = glutGetModifiers();
	if (GLUT_ACTIVE_CTRL != mod)		{PickMode(0); return true;}
	//--- Move the node ---------------------------------
	modf |= trak.MoveSelectedNode(cpos);
	return true;
}
//------------------------------------------------------------------------------
//	Mouse move
//	NOTE: When a node is selected and the CTRL button is held, then
//				we move the node
//------------------------------------------------------------------------------
bool CFuiTaxi::InsideMove(int mx,int my)
{	if (MouseHit(mx,my))						return true;
	//--- Change position -------------------------------
  CursorGeop(mx,my);
	PositionEdit(cpos,cplb,"CURS:");
	//--- Check for moving selected node ----------------
	if (mode == TAXI_MODE_NODEMV)		return MoveNode (mx, my);
	if (mode == TAXI_MODE_MOVING)		return MoveWorld(mx,my);
	if (mode == TAXI_MODE_PICKING)	return false;
	//--- Follow cursor with a link	---------------------
	trak.SetCursor(&cpos);
	return false;
}
//------------------------------------------------------------------------------
//	Mouse Picking
//------------------------------------------------------------------------------
bool CFuiTaxi::MousePicking(int mx, int my)
{	sx			= mx;
	sy			= my;
	selT		= 0;
	RegisterFocus(0);										// Free textedit
	rcam->PickObject(mx, my);
	trak.SetSelection(selT);
	return true;		
}
//------------------------------------------------------------------------------
//	Add a branch node
//------------------------------------------------------------------------------
void CFuiTaxi::AddEdgeNode()
{	Tag idn = trak.AddOneNode(nodA,selT,cpos);
	selT	= idn;
	nodA	= idn;
	if (idn) modf	|= 1;
	return;
}
//------------------------------------------------------------------------------
//	Insert an exit node
//------------------------------------------------------------------------------
void CFuiTaxi::InsertNode()
{ Tag idn = trak.InsertNode(nodA,selT);
	PickMode(idn);								// Back to pick mode
	modf	= 1;
	return;
}
//------------------------------------------------------------------------------
//	Delete a specific node
//------------------------------------------------------------------------------
void	CFuiTaxi::DeleteEdge()
{	modf |= trak.DeleteEdge(nodA,selT);
	PickMode(0);
	return;
}
//------------------------------------------------------------------------------
//	Attach two nodes
//------------------------------------------------------------------------------
void CFuiTaxi::AttachNodes()
{	if (0 == selT)	return PickMode(0);
	modf |= trak.Attach(nodA,selT);
	nodA	= selT;
	trak.SetSelection(selT);
	return;
}
//------------------------------------------------------------------------------
//	Mouse click are captured here
//------------------------------------------------------------------------------
bool CFuiTaxi::MouseCapture(int mx, int my, EMouseButton bt)
{	if (MouseClick (mx, my, bt))		return true;
	//--- Select node if needed ------------------------------
	keym = glutGetModifiers();
	MousePicking(mx,my);
  if (MOUSE_BUTTON_RIGHT == bt)		return ActeMenu(mx,my);
	//--- Check for centering on the clicked spot ------------
	if (GLUT_ACTIVE_ALT  == keym)		return MoveMode(mx,my);
	if (GLUT_ACTIVE_CTRL == keym)   return NodeMode();
	switch (mode) {
			//--- Mode picking: already selected ------------
			case TAXI_MODE_PICKING:
				return true;
			//--- Create a branch node ---------------------
			case TAXI_MODE_BRANCH:
				AddEdgeNode();
				return true;
			//--- Create a new exit node -------------------
			case TAXI_MODE_INSERT:
				InsertNode();
				return true;												
			//--- Create a new edge ------------------------
			case TAXI_MODE_ATTACH:
				AttachNodes();
				return true;
			//--- Delete Edge ----------------------------
			case TAXI_MODE_EDGEDL:
				DeleteEdge();
				return true;

	}
	return true;		
}
//------------------------------------------------------------------------------
//	Stop any move here
//------------------------------------------------------------------------------
bool	CFuiTaxi::StopClickInside(int mx, int my, EMouseButton but)
{ if (mode > TAXI_MODE_NODEMV)	return true;
	//--- Stop moving anything ---------------------
	PickMode(0);
	return true;
}
//------------------------------------------------------------------------------
//	Time slice: nothing to do
//------------------------------------------------------------------------------
void	CFuiTaxi::TimeSlice()
{	}
//------------------------------------------------------------------------------
//	Set start node 
//------------------------------------------------------------------------------
void CFuiTaxi::SetStartNode(Tag T)
{	dTag	= T;
	trak.NodeProp(T);
	_snprintf(buf,32,"DEB %s",taxiMENU[0]);
	nodDEB->SetText(buf);
	return;
}
//------------------------------------------------------------------------------
//	Open the floating menu 
//------------------------------------------------------------------------------
void CFuiTaxi::SetEndNode(Tag T)
{	eTag	= T;
	trak.NodeProp(T);
	_snprintf(buf,32,"END %s",taxiMENU[0]);
	nodEND->SetText(buf);
	return;
}

//------------------------------------------------------------------------------
//	Open the floating menu 
//------------------------------------------------------------------------------
bool CFuiTaxi::ActeMenu(int mx,int my)
{	if (path)				return true;
  if (0 == selT)	return true;
  PickMode(selT);
	trak.NodeProp(selT);
	menu.Ident	= 'acte';
	menu.aText	= taxiMENU;
	//--- Open a  pop menu ---------------
	OpenPopup(mx,my,&menu);
	return true;
}
//------------------------------------------------------------------------------
//	Click on edit Menu
//------------------------------------------------------------------------------
int CFuiTaxi::ClickActeMENU(short itm)
{ char  opt = *menu.aText[itm];
  switch (opt) {
			//--- Add an exit node --------------
			case '1':
				LinkMode(TAXI_MODE_INSERT);
				break;
			//--- Create a branch node ----------
			case '2':
				LinkMode(TAXI_MODE_BRANCH);
				break;
			//--- Delete a node -----------------
			case '3':
				trak.DelNode();
				modf	= 1;
				break;
			//---  Link to other ----------------
			case '4':
				LinkMode(TAXI_MODE_ATTACH);
				break;
			//--- Swap runway -------------------
			case '5':
				trak.SwapRunway(selT);
				PickMode(0);
				modf	= 1;
				break;
			//--- Delete Edge ------------------
			case '6':
				LinkMode(TAXI_MODE_EDGEDL);
				break;
			//--- Starting node ----------------
			case '7':
				SetStartNode(selT);
				break;
			//--- Ending node ----------------
			case '8':
				SetEndNode(selT);
				break;

			//--- Ignore => Pick mode ----------
			default:
				PickMode(0);
				break;
	}
  ClosePopup();
	return 1;
}
//------------------------------------------------------------------------------
//	NOTIFICATION from buttons
//------------------------------------------------------------------------------
void CFuiTaxi::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{	switch (idm)  {
    case 'sysb':
      SystemHandler(evn);
      return;
		case 'path':
			SwapPath();
			return;
		//--- center drawing on airport -----------
		case 'cent':
			NewPosition(org);
			rcam->SetAbove(org,30000,globals->geop.alt);
			return;
		case 'inps':
		case 'outs':
			SwapCircuit();
			return;
		case 'test':
			trak.Test();
			return;

	}
return;
}
//------------------------------------------------------------------------------
//	NOTIFICATIONS from popup menu
//------------------------------------------------------------------------------
void CFuiTaxi::NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn)
{	if (EVENT_POP_CLICK  != evn)			return;
	switch(id)	{
			//--- click on map ------------------
			case 'acte':
				ClickActeMENU(itm);
				return;
	}
	return;
}
//===========================================================================================
//	Taxiway editor:  Save taxiway
//===========================================================================================
//	Save to file
//-------------------------------------------------------------------------------
void CFuiTaxi::SaveData()
{	//nsup->SaveToFile(trak);
	if (0 == modf)		return;
	if (taxm)	taxm->SaveToBase(trak);
	return;
	
}
//===========================================================================================
//	Taxiway editor: get taxiway manager from airport
//===========================================================================================
bool CFuiTaxi::GetTaxiMGR()
{	if (0 == apo)		{Close(); return true;}
	taxm	= apo->GetTaxiMGR();
	if (0 == taxm)	{Close(); return true;}
	trak.NodeSupplier(taxm);
	trak.ComputeAllShortCut();
	return taxm->NotEmpty();
}	
//===========================================================================================
//	SQL STATEMENTS RELATED TO TAXIWAYS
//===========================================================================================
//	Remove all data for the given airport
//===========================================================================================
int SqlMGR::RemoveTaxiData(char *key)
{ char req[1024];
  _snprintf(req,1024,"DELETE FROM Node WHERE aKey='%s';*",key);
  sqlite3_stmt *std = CompileREQ(req,txyDBE);
	int rep = sqlite3_step(std);                // Execute Statement
  sqlite3_finalize(std);                      // Close statement
	//--------------------------------------------------------------------
	_snprintf(req,1024,"DELETE FROM Arcs WHERE aKey='%s';*",key);
  std = CompileREQ(req,txyDBE);
	rep = sqlite3_step(std);										// Execute Statement
  sqlite3_finalize(std);                      // Close statement
  return rep;
}
//--------------------------------------------------------------------
//	Write a node
//--------------------------------------------------------------------
int SqlMGR::AddTaxiNode(char *key,TaxNODE *N)
{	char *rq = "INSERT INTO Node (aKey,Ident,Lat,Lon,Alt,rwid,type,Sector)" 
						 "VALUES(?1,?2,?3,?4,?5,?6,?7,?8);*";
	int rep	 = 0;
  sqlite3_stmt *stm = CompileREQ(rq,txyDBE);
	//--- insert Airport key ------------------------
	rep = sqlite3_bind_text(stm, 1,key, -1, SQLITE_TRANSIENT);
  if (rep != SQLITE_OK) Abort(txyDBE);
	//--- insert ident ------------------------------
	rep = sqlite3_bind_int (stm, 2,N->idn);
  if (rep != SQLITE_OK) Abort(txyDBE);
	//--- World coordinates -----------------------------------------------
  rep      = sqlite3_bind_double(stm, 3, N->Pos.lat);         // Latitude
  if (rep != SQLITE_OK) Abort(txyDBE);
  rep      = sqlite3_bind_double(stm, 4, N->Pos.lon);         // Longitude
  if (rep != SQLITE_OK) Abort(txyDBE);
	rep      = sqlite3_bind_double(stm, 5, N->Pos.alt);         // Altitude
  if (rep != SQLITE_OK) Abort(txyDBE);
	//--- Runway ident ----------------------------------------------------
	rep = sqlite3_bind_text(stm, 6,N->rwy, 4, SQLITE_TRANSIENT);
  if (rep != SQLITE_OK) Abort(txyDBE);
	//--- Type ------------------------------------------------------------
	rep = sqlite3_bind_int (stm, 7,N->type);
  if (rep != SQLITE_OK) Abort(txyDBE);
	//--- Sector ----------------------------------------------------------
	rep = sqlite3_bind_int (stm, 8,N->sector);
  if (rep != SQLITE_OK) Abort(txyDBE);
	//--- Execute and close -----------------------------------------------
  rep      = sqlite3_step(stm);               // Insert value in database
  if (rep != SQLITE_DONE) Abort(txyDBE);
  sqlite3_finalize(stm);                      // Close statement
	return rep;
}
//----------------------------------------------------------------------------
//	Write all edges
//----------------------------------------------------------------------------
int SqlMGR::AddTaxiEdges(char *key,TaxiwayMGR *txm)
{ char *rq = "INSERT INTO Arcs (aKey,dim1,tkoEdge,dim2,tkoPath,dim3,lndEdge,dim4,lndPath)" 
						 "VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9);*";
	PACK_EDGE S1;
	txm->PackArc(TKO_CIRCUIT,&S1);
	PACK_EDGE S2;
	txm->PackArc(LND_CIRCUIT,&S2);
	//--- Compile request ---------------------------
	int rep	 = 0;
  sqlite3_stmt *stm = CompileREQ(rq,txyDBE);
	//--- insert Airport key ------------------------
	rep = sqlite3_bind_text(stm, 1,key, -1, SQLITE_TRANSIENT);
  if (rep != SQLITE_OK) Abort(txyDBE);
	//--- Insert tko edge ---------------------------
	rep = sqlite3_bind_int (stm, 2,S1.dm1);
  if (rep != SQLITE_OK) Abort(txyDBE);
	int d1 = S1.dm1 * sizeof(U_INT);
	sqlite3_bind_blob(stm, 3,S1.bf1, d1, SQLITE_TRANSIENT);
	if (rep != SQLITE_OK) Abort(txyDBE);
	//--- Insert tko path ---------------------------
	rep = sqlite3_bind_int (stm, 4,S1.dm2);
  if (rep != SQLITE_OK) Abort(txyDBE);
	int d2 = S1.dm2 * sizeof(U_INT);
	sqlite3_bind_blob(stm, 5,S1.bf2, d2, SQLITE_TRANSIENT);
	if (rep != SQLITE_OK) Abort(txyDBE);
	//--- Insert lnd edge ---------------------------
	rep = sqlite3_bind_int (stm, 6,S2.dm1);
  if (rep != SQLITE_OK) Abort(txyDBE);
	int d3 = S2.dm1 * sizeof(U_INT);
	sqlite3_bind_blob(stm, 7,S2.bf1, d3, SQLITE_TRANSIENT);
	if (rep != SQLITE_OK) Abort(txyDBE);
	//--- Insert lnd path ---------------------------
	rep = sqlite3_bind_int (stm, 8,S2.dm2);
  if (rep != SQLITE_OK) Abort(txyDBE);
	int d4 = S2.dm2 * sizeof(U_INT);
	sqlite3_bind_blob(stm, 9,S2.bf2, d4, SQLITE_TRANSIENT);
	if (rep != SQLITE_OK) Abort(txyDBE);
	//--- Execute and close -----------------------------------------------
  rep      = sqlite3_step(stm);               // Insert value in database
  if (rep != SQLITE_DONE) Abort(txyDBE);
  sqlite3_finalize(stm);                      // Close statement
	return 0;
}
//===========================================================================================
//	Read all nodes for the given airport
//===========================================================================================
int SqlMGR::ReadTaxiNodes(char *key, TaxiwayMGR *txm)
{ char req[1024];
	SPosition P;
  _snprintf(req,1024,"SELECT * FROM Node WHERE akey='%s';*",key);
  sqlite3_stmt *stm = CompileREQ(req,txyDBE);
  //----Execute select -------------------------------------------
	 while (SQLITE_ROW == sqlite3_step(stm))
	 {	int idn = sqlite3_column_int(stm, 1);					// Node ident
			P.lat		= sqlite3_column_double(stm, 2);				// Latitude
			P.lon		= sqlite3_column_double(stm, 3);				// Longitude
			P.alt		= sqlite3_column_double(stm, 4);				// Altitude
			TaxNODE *N = new TaxNODE(idn,&P);								// Create Node
			char	*txt = (char*)sqlite3_column_text(stm,5);	// Runway id
			N->SetRWY(txt);
			N->type		=	sqlite3_column_int (stm,6);					// type
			N->sector = sqlite3_column_int (stm,7);					// sector
			txm->EnterNode(N);
	 }
	//--- Close -----------------------------------------------
  sqlite3_finalize(stm);                      // Close statement
	return 0;
}
//----------------------------------------------------------------------------
//	Read  all edges
//----------------------------------------------------------------------------
int SqlMGR::ReadTaxiEdges(char *key, TaxiwayMGR *txm)
{	char req[1024];
	int	  dim;
	void *src;
	_snprintf(req,1024,"SELECT * FROM Arcs WHERE akey='%s';*",key);
	sqlite3_stmt *stm = CompileREQ(req,txyDBE);
	if (SQLITE_ROW == sqlite3_step(stm))
  {	dim	= sqlite3_column_int(stm,1);						// Dimension
		src	= (void*)sqlite3_column_blob (stm,2);		// Tko edges
		txm->UnpackArc(TKO_CIRCUIT,dim,src);				// Edge
		//------------------------------------------------------
		dim	= sqlite3_column_int(stm,3);						// Dimension
		src	= (void*)sqlite3_column_blob (stm,4);		// Tko edges
		txm->UnpackPath(TKO_CIRCUIT,dim, src);			// Path
		//------------------------------------------------------
		dim	= sqlite3_column_int(stm,5);						// Dimension
		src	= (void*)sqlite3_column_blob (stm,6);		// Tko edges
		txm->UnpackArc(LND_CIRCUIT,dim,src);				// Edge
		//------------------------------------------------------
		dim	= sqlite3_column_int(stm,7);						// Dimension
		src	= (void*)sqlite3_column_blob (stm,8);		// Tko edges
		txm->UnpackPath(LND_CIRCUIT,dim, src);			// Path

	} 
	sqlite3_finalize(stm);                      // Close statement
	return 0;
}
	
//=======================EDN OF FILE ========================================================


