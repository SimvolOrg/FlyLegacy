//====================================================================================
// CBuilder.cpp
//
// Part of Fly! Legacy project
//
// Copyright 2003 Chris Wallace
// copyright 2012 jean Sabatier
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
//============================================================================================
#include <math.h>
#include "../Include/Globals.h"
#include "../Include/Triangulator.h"
#include "../Include/OSMobjects.h"
#include "../Include/RoofModels.h"
#include "../Include/TerrainTexture.h"
#include "../Include/fileparser.h"
#include <math.h>
//===================================================================================
//	Vector Table to build Objects
//===================================================================================
CBuilder::buildCB osmCB[] = {
	0,														// Not an object
	&CBuilder::MakeBLDG,					// OSM_BLDG		(1)
	&CBuilder::MakeLITE,					// OSM_LITE			(2)
	&CBuilder::MakeBLDG,					// OSM_AMNY			(3)
};
//===================================================================================
//	UNIT CONVERTER 
//===================================================================================
#define FOOT_FROM_METERS(X)   (double(X) *  3.2808399)
#define SQRF_FROM_SQRMTR(X)   (double(X) * 10.7639104)
//===================================================================================
//	Face change for bevel
//===================================================================================
char bevelTAB[] = {
	GEO_FACE_YP,					// X+ change to Y+
	GEO_FACE_XM,					// Y+ change to X-
	GEO_FACE_YM,					// X- change to Y-
	GEO_FACE_XP,					// Y- change to X+
};
//===================================================================================
//	Style mask decoder
//===================================================================================
char *D2_MXP 		= " FACE  ( X+ ) SW [ %u , %u ] DIM [ %lf , %lf ] ";
char *D2_MYP		= " FACE  ( Y+ ) SW [ %u , %u ] DIM [ %lf , %lf ] ";
char *D2_MXM		= " FACE  ( X- ) SW [ %u , %u ] DIM [ %lf , %lf ] ";
char *D2_MYM		= " FACE  ( Y- ) SW [ %u , %u ] DIM [ %lf , %lf ] ";
char *D2_MGF		= " GFLOOR ( %2[^)] ) SW [ %u , %u ] DIM [ %lf , %lf ] ";
char *D2_MMF		= " MFLOOR ( %2[^)] ) SW [ %u , %u ] DIM [ %lf , %lf ] ";
char *D2_MZF		= " ZFLOOR ( %2[^)] ) SW [ %u , %u ] DIM [ %lf , %lf ] ";
char *D2_ROF		= " ROOF   SW[ %u , %u] DIM [%lf, %lf ] ( FREQ = %d )";
char *D2_DMR    = " DORMER SW[ %u , %u] DIM [%lf, %lf ] ";
//======================================================================================
// Visibility test between 4 points on same plan
//  -point P is the origin point
//  -point Q is the extemity of segment PQ
//	-point A is    origin of segment AB
//  -point B is extremity of segment AB
//	We want to check if P and Q are mutually visible, relative to AB. 
//	Does segment AB masks P from Q?
//	-----------------------------------------------------------------
//	AB masks P from Q when both conditions holds
//	1) A and B are not on the same side relative to PQ
//	2) P and Q are not on the same side relative to AB
//	When 1 & 2 are both true, segments PQ and AB intersect so P cant see Q.
//  ------------------------------------------------------------------
//	Same side test is done by considering the dot products of 3 vectors 
//	Example: prod1 = PQ.PA 
//					 prod2 = PQ.PB
//	Each dot product defines a vector perpendicular to the plan. A & B are on same
//	side of PQ if both products has same orientation.   As PQ and AB are planar, we just
//	have to consider the Z coordinate of each dot product (a real number).
//--------------------------------------------------------------------------------------
//			Z(prod1)*Z(prod2) < 0		=>	A and B are not on same side of PQ
//  with
//	P(xp,yp) Q(xq,yq) A(xa,ya) B(xb,yb) we have
//	vector PQ[(xq-xp),(yq-yp)]
//	vector PA[(xa-xp),(ya-yp)]
//	vector PB[(xb-xp),(yb-yp)]
//	Z(prod(PQ,PA)) = (xq-xp)(ya-yp) - (yq-yp)(xa-xp) = z1;
//	Z(prod(PQ,PB)) = (xq-xp)(yb-yp) - (yq-yp)(xb-xp) = z2;
//	thus we just test the sign of z1.z2 
//--------------------------------------------------------------------------------------
//	Special cases:
//	A is on PQ when PQ.PA = vector(0) thus z1 = 0;  So we  cant tell which side is A
//		but we can tell if A is between P and Q
//	Idem for B
//======================================================================================
int GeoTest::SameSide(D2_POINT &P, D2_POINT &Q, D2_POINT &A, D2_POINT &B)
{ double xpq	= Q.x - P.x;
	double ypq  = Q.y - P.y;
	double xpa	= A.x - P.x;
	double ypa  = A.y - P.y;
	double xpb  = B.x - P.x;
	double ypb  = B.y - P.y;
	//--- compute Z coordinate of PQ.PA ----------------
	double z1   = (xpq * ypa) - (ypq * xpa);
	//--- Check if A is colinear with PQ ----------------
	if (fabs(z1) < precision)	
	{	double	xqa	=  (A.x - Q.x);
		double	yqa =  (A.y - Q.y);
	  bool		in	= ((xqa * xpa) < 0)	|| ((yqa * ypa) < 0);
		return (in)?(GEO_INSIDE_PQ):(GEO_OUTSIDE_PQ);
	}	
	double z2   = (xpq * ypb) - (ypq * xpb);
	//--- Check if B is colinear with PQ ----------------
	if (fabs(z2) < precision)	
	{	double	xqb = (B.x - Q.x);
		double	yqb = (B.y - Q.y);
		bool in			= ((xqb	* xpb) < 0)	|| ((yqb * ypb) < 0);
		return	(in)?(GEO_INSIDE_PQ):(GEO_OUTSIDE_PQ);
	}
	//--- Opposite side if z1 * z2 < 0 --------------------
	double pr   = z1 * z2;
  return (pr < 0)?(GEO_OPPOS_SIDE):(GEO_SAME_SIDE);
}
//--------------------------------------------------------------------------------------
//	Check visibility (see above) Does AB prevent P from seing Q
//	1)	If either point A or B is inside Segment PQ, then we considere that
//			P is masked from Q by this point
//	2)  If  both A and B are on the same side of segment PQ then P and Q are mutually
//			visible
//	3)		 As A and B are on opposite sides (or colinear with PQ, but ouside), we must now 
//			test P and Q positons relative to AB
//--------------------------------------------------------------------------------------
bool GeoTest::VisibilityTest(D2_POINT &P, D2_POINT &Q, D2_POINT &A, D2_POINT &B)
{	int r1 = SameSide(P,Q,A,B);
	if (r1 == GEO_INSIDE_PQ)		return false;
	if (r1 == GEO_SAME_SIDE)		return true;
	//--- A & B are opposed relative to segment PQ ---------
	int r2 = SameSide(A,B,P,Q);
	return (r2 == GEO_OPPOS_SIDE)?(false):(true);
}
//--------------------------------------------------------------------------------------
//	Check side of point A, relative to vector PQ
//	We just need the case where A is strictly on left of PQ
//--------------------------------------------------------------------------------------
int GeoTest::OnLeft(D2_POINT &A, D2_POINT &P, D2_POINT &Q)
{	//--- check the Z coordinate of dot product(PQ,PA) --------
	double xpq = Q.x - P.x;
	double ypq = Q.y - P.y;
	double xpa = A.x - P.x;
	double ypa = A.y - P.y;
	double zp  = (xpq * ypa) - (xpa * ypq);
	//---------------------------------------------------------
	if (fabs(zp) < precision)		return GEO_ON_PQ;
	return (zp > 0)?(GEO_LEFT_PQ):(GEO_RITE_PQ);
}
//--------------------------------------------------------------------------------------
//	Check side of point A, relative to vector BC
//	We just need the case where A is strictly on left of BC
//--------------------------------------------------------------------------------------
int GeoTest::Positive(D2_TRIANGLE &T)
{	double xac = T.C->x - T.A->x;
  double yac = T.C->y - T.A->y;
	double xab = T.B->x - T.A->x;
	double yab = T.B->y - T.A->y;
	double zp  = (xac * yab) - (xab *yac);
	return (zp > 0)?(1):(0);
}
//--------------------------------------------------------------------------------------
//	Check if Point A is inside triangle P,Q,R where edges are positively oriented
//	True when A is strictly on left side of the 3 edges
//--------------------------------------------------------------------------------------
int GeoTest::InTriangle(D2_POINT &A, D2_POINT &P, D2_POINT &Q, D2_POINT &R)
{	int in = OnLeft(A,P,Q) & OnLeft(A,Q,R) & OnLeft(A,R,P);
	return in;
}
//--------------------------------------------------------------------------------------
//	Check if Point A is inside triangle P,Q,R where edges are positively oriented
//	True when A is strictly on left side of the 3 edges
//--------------------------------------------------------------------------------------
int GeoTest::InTriangle(D2_POINT &P, D2_TRIANGLE &T)
{	int on = 0;
	int p1 = OnLeft(P,*T.B,*T.A);
	if (p1 == GEO_RITE_PQ)			return GEO_OUTSIDE;
	if (p1 == GEO_ON_PQ)				on++;
	//-----------------------------------------------------
	int p2 = OnLeft(P,*T.A,*T.C);
	if (p2 == GEO_RITE_PQ)			return GEO_OUTSIDE;
	if (p2 == GEO_ON_PQ)				on++;
	//----------------------------------------------------
	int p3 = OnLeft(P,*T.C, *T.B);
	if (p3 == GEO_RITE_PQ)			return GEO_OUTSIDE;
	if (p3 == GEO_ON_PQ)				on++;
	return (on)?(GEO_ON_SIDE):(GEO_INSIDE); 
	}
//--------------------------------------------------------------------------------------
//	Compute the normal to plan (A,B,C)
//--------------------------------------------------------------------------------------
CVector GeoTest::PlanNorme(D2_POINT &B, D2_POINT &A, D2_POINT &C)
{	CVector rb((B.x - A.x), (B.y - A.y), (B.z - A.z));
	CVector rc((C.x - A.x), (C.y - A.y), (C.z - A.z));
	CVector cp;
	cp.CrossProduct(rb,rc);
	cp.Normalize();
	return cp;
}
//====================================================================================
//	POINTS routines
//	Compute mid point with or without withdraw
//====================================================================================
//---------------------------------------------------------------------------
//	Translate local coordinates
//---------------------------------------------------------------------------
void D2_POINT::LocalShift(double tx,double ty)
	{	lx += tx;
		ly += ty;
	}
//---------------------------------------------------------------------------
//	Save vertex
//---------------------------------------------------------------------------
GN_VTAB *D2_POINT::SaveData(GN_VTAB *tab, CVector &N)
{	tab->VT_X = x;
	tab->VT_Y = y;
	tab->VT_Z = z;
	//--------------------------------
	tab->VN_X = N.x;
	tab->VN_Y = N.y;
	tab->VN_Z = N.z;
	//--------------------------------
	tab->VT_S = s;
	tab->VT_T = t;
	return (tab + 1);
}

//====================================================================================
//	TRIANGLE routines
//	
//====================================================================================
//---------------------------------------------------------------------
//	Store vertices in part
//---------------------------------------------------------------------
U_INT D2_TRIANGLE::StoreData(C3DPart *p, U_INT n)
{ GN_VTAB *tab = p->GetGTAB() + n;
  tab	= B->SaveData(tab,N);
	tab = A->SaveData(tab,N);
	tab = C->SaveData(tab,N);
	return (3 + n);
}

//====================================================================================
//	TRIANGULATOR global variable
//====================================================================================
char *FaceType[] =
{	"X+",								// 0
	"Y+",								// 1
	"X-",								// 2
	"Y-",								// 3
};
//====================================================================================
//	TRIANGULATOR for triangulation of polygones
//====================================================================================
CBuilder::CBuilder(D2_Session *s)
{	trace = 0;
	session		= s;
	dop.Set(TRITOR_DRAW_ROOF);
	dop.Set(TRITOR_DRAW_WALL);
	dop.Set(TRITOR_DRAW_LINE);
	dop.Set(TRITOR_DRAW_FILL);
	BDP.roofM	= s->GetDefaultRoof();
	BDP.roofP	= 0;
	bevel			= 0;
	BDP.style	= 0;
	osmB			= 0;
	remB			= 0;
	t70				= tan(DegToRad(double(70)));
	BDP.stamp	= 0;
	dMOD			= 0;
	Clean();
	globals->Disp.Enter(this, PRIO_ABSOLUTE, DISP_EXSTOP, 1);
}
//-------------------------------------------------------------------
//	Release resources 
//-------------------------------------------------------------------
CBuilder::~CBuilder()
{	Clean();
	//--- clear building list ---------------------------
	//	Individual buildings are deleted from D2_Group
	globals->Disp.Clear(PRIO_ABSOLUTE);
}
//-------------------------------------------------------------------
//	Time Slice 
//-------------------------------------------------------------------
int CBuilder::TimeSlice(float dT,U_INT frame)
{
	return 0;
}
//===================================================================
//	Drawing interface
//===================================================================
//-------------------------------------------------------------------
//	Draw all groups  : DrawMarks is called from camera for picking mode 
//											DO NOT CHANGE THE NAME
//-------------------------------------------------------------------
void CBuilder::DrawMarks()
{	return session->Draw();	}
//-------------------------------------------------------------------
//	Draw Interface 
//-------------------------------------------------------------------
void CBuilder::Draw()
{	char d1 = 0;
	if (dop.Has(TRITOR_DRAW_FILL))	FillMode();
	else														LineMode();
	//--- Draw current mode ------------------------------------------
	if (dMOD == 1)	DrawMarks();
	else
	if (osmB)	osmB->DrawLocal();
	glEnable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT,GL_FILL);
	return;
}
//-------------------------------------------------------------------
//	Draw as lines 
//-------------------------------------------------------------------
int CBuilder::LineMode()
{	// Draw triangulation
	glDisable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT,GL_LINE);
	return 1;
}
//-------------------------------------------------------------------
//	Draw as color 
//-------------------------------------------------------------------
int CBuilder::FillMode()
{	// Draw triangulation
	glEnable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT,GL_FILL);
	return 1;
}

//===================================================================
//	Public interface
//===================================================================
//-------------------------------------------------------------------
//	Add a new vertex
//-------------------------------------------------------------------
void CBuilder::AddVertex(char a, double x, double y)
{	double xa   = FN_ARCS_FROM_DEGRE(x);
	double ya   = FN_ARCS_FROM_DEGRE(y);
	D2_POINT *p = new D2_POINT(xa,ya);
	char txt[8];
	_snprintf(txt,6,"%c%03d",hIND,seq++);
	p->SetID(txt);
	if ('X' == hIND)	extp.PutLast(p);
	else							hole.PutLast(p);
	//--- Save original coordinates --------------------
	p->dgx	= x;
	p->dgy	= y;
	//--- Compute object barycenter --------------------
	BDP.geop.lon	+= xa;
	BDP.geop.lat  += ya;
	if (0 == a)										return;
	//--- Get terrain altitude -------------------------
	GroundSpot lnd(xa,ya);
  p->z = globals->tcm->GetGroundAt(lnd);
	return;
}
//-------------------------------------------------------------------
//	Hole annunciator
//-------------------------------------------------------------------
void CBuilder::NewHole()
{	hIND	= 'H';
	//--- Merge with previous hole if any --------
	Merge();
	return;
}

//===================================================================
//	Private interface
//===================================================================
//-------------------------------------------------------------------
//	Cleaner
//-------------------------------------------------------------------
void CBuilder::Clean()
{	extp.Clear();
	hole.Clear();
	slot.Clear();
	for (U_INT k=0; k < walls.size(); k++)	delete walls[k];
	walls.clear();
	for (U_INT k=0; k < roof.size();  k++)	delete roof[k];
	roof.clear();
	//--- delete bevel point array ----------
	if (bevel)	delete [] bevel;
	//--- Reset all values ------------------
	osmB			= 0;
	bevel			= 0;
	seq				= 0;										// Vertex sequence
	num				= 0;										// Null number
	vRFX			= 0;
	hIND			= 'X';
	BDP.Clear();
	TO				= 0;
  //--- Reset all parameters --------------
	return;
}
//-------------------------------------------------------------------
//	Check every thing for debugging
//-------------------------------------------------------------------
void CBuilder::CheckAll()
{	if (extp.NotEmpty())
	int a = 0;
  if (slot.NotEmpty())
	int b = 0;
	if (walls.size() != 0)
	int c = 0;
	if (roof.size() != 0)
	int d = 0;
	if (bevel)
	int e = 0;
}
//-------------------------------------------------------------------
//	Draw One 
//-------------------------------------------------------------------
void CBuilder::ModeSingle()
{	dMOD = 0;
	if (0 == osmB)	return;
	osmB->Deselect();
	return;
}
//-------------------------------------------------------------------
//	Draw all 
//-------------------------------------------------------------------
void CBuilder::ModeGroups()
{	dMOD = 1;
	if (0 == osmB)	return;
	osmB->Deselect();
}
//-------------------------------------------------------------------
//	Start a new object 
//-------------------------------------------------------------------
void CBuilder::StartOBJ()
{	if (osmB)	osmB->Deselect();
	Clean();
	return;
}
//-------------------------------------------------------------------
//	Build a new object 
//-------------------------------------------------------------------
void CBuilder::BuildOBJ(OSM_CONFP *CF)
{ U_INT type	= CF->otype;
  U_INT build = CF->build;
	BDP.stamp		= session->GetNextStamp();
	BDP.side		= extp.GetNbObj();
	BDP.error		= 0;
	BDP.opt.Rep(CF->prop);								// initial property
	BDP.error		= ConvertInFeet();
	
	OSM_Object *obj = 	new OSM_Object(CF);
	BDP.obj		= obj;
  osmB			= obj;
	GetSuperTileNo(&BDP.geop, &BDP.qgKey, &BDP.supNo);
	return (this->*osmCB[build])(type);}
//-------------------------------------------------------------------
//	Rebuild if orientation error 
//-------------------------------------------------------------------
int CBuilder::ReOrientation(D2_BPM *bpm)
{	bpm->error++;
	if (bpm->error > 1)		return 4;
	slot.Clear();
	//---Invert vertices direction ----------
	Queue<D2_POINT> H;
	extp.TransferQ(H);
	D2_POINT *pp;
	for (pp = H.Pop(); pp != 0; pp = H.Pop()) 
		{ extp.PutHead(pp); 
	    pp->Reset();
	  }
	//--- Requalify once -------------------
	QualifyPoints(bpm);
	bpm->error = 0;
	return 0;
}
//-------------------------------------------------------------------
//	Make a building
//-------------------------------------------------------------------
void CBuilder::MakeBLDG(U_INT tp)
{	D2_BPM *bpm = &BDP;
	QualifyPoints(bpm);
	//--- Set generation parameters ---------
	session->GetaStyle(bpm);
	osmB->AssignStyle(BDP.style,this);
	//--------------------------------------
	return;
}
//-------------------------------------------------------------------
//	Make a Light row.  Compute light height from terrain
//-------------------------------------------------------------------
void CBuilder::MakeLITE(U_INT tp)
{	osmB->Copy(BDP);
	osmB->ReceiveQ(extp);
	osmB->BuildLightRow(6);						// 6 meters above ground
	session->AddLight(osmB);
	extp.Clear();
	return;
}
//-------------------------------------------------------------------
//	Edit tag
//-------------------------------------------------------------------
void CBuilder::EditTag(char *txt)
{	*txt	= 0;
	if (osmB)	osmB->EditTag(txt);
	return;
}
//-------------------------------------------------------------------
//	Edit object parameters
//-------------------------------------------------------------------
int CBuilder::EditPrm(char *txt)
{	*txt	= 0;
	int er	= 0;
  if (osmB)	er = osmB->EditPrm(txt);
	return er;
}
//-------------------------------------------------------------------
//  Build the building
//-------------------------------------------------------------------
bool CBuilder::RiseBuilding(D2_BPM *bpm)
{	dlg	= bpm->dlg;
	//--- Step 1: Save OSM parameters -----
	//--- Step 2:  Triangulation --------
	bpm->error = Triangulation();
  //--- Save a copy of the base points -----------
	osmB->ReceiveQ(extp);
	//--- Step 3: Face orientation ----- 
	OrientFaces(bpm);
	//--- Step 4: Build walls -----------
	BuildWalls(bpm);
	//--- Step 5: Select roof model -----
	SelectRoof(bpm);
	//--- Step 6: Texturing -------------
	Texturing(bpm);
	//--- Step 7:  Save parameters ------
	return true;
}
//-------------------------------------------------------------------
//	Replace by a 3D object
//	option or tells whether orientation is defined in rpp parameter
//-------------------------------------------------------------------
U_CHAR CBuilder::ReplaceOBJ(OSM_REP *rpp, char or)
{	if (0 == osmB)							return 0;
	if (0 == rpp->obr)					return 0;
	if (!osmB->CanBeReplaced())	return 0;
	if (or) {rpp->sinA = sinA; rpp->cosA = cosA; }
	//TRACE("REPLACE BUILDING %d",BDP.stamp);
	char *dir = directoryTAB[rpp->dir];
	osmB->ReplaceBy(rpp);
  COBJparser fpar(OSM_OBJECT);
	fpar.SetDirectory(dir);
  fpar.Decode(rpp->obr,OSM_OBJECT);
	C3DPart *prt = fpar.BuildOSMPart(rpp->dir);
	if (0 == prt)							return 0;
	//--- Change model parameters --------------
	osmB->ReplacePart(prt);
	return 1;
}
//-------------------------------------------------------------------
//	End of object 
//-------------------------------------------------------------------
void CBuilder::EndOBJ()
{	Clean();
	return;
}
//-------------------------------------------------------------------
//	return total vertices needed
//	For each wall:
//		- 6 vertices per face
//	For the roof :
//		- 3 vertices per triangle
//-------------------------------------------------------------------
U_INT CBuilder::CountVertices()
{	U_INT nvtx = 3 * roof.size();
	for (U_INT k=0; k < walls.size(); k++) nvtx += walls[k]->VerticesNumber();
	return nvtx;
}
//-------------------------------------------------------------------
//	Set Style from name
//-------------------------------------------------------------------
void CBuilder::ForceStyle(char *nsty, U_INT rfmo, U_INT rftx, U_INT flnb)
{	D2_Style *sty = session->GetStyle(nsty);
	BDP.style = sty;
	if (0 == sty)		return;
	sty->AssignBPM(&BDP);
	//--- search roof model and texture --------
	char mans     = sty->GetMansart();
	BDP.roofP			= sty->SelectRoofNum(rftx);
	D2_Group *grp = sty->GetGroup();
	grp->SetFloorNbr(flnb);
	grp->SetRoofModelNumber(rfmo);
	BDP.roofM     = grp->GetRoofModByNumber(mans);
	return;
}
//-------------------------------------------------------------------
//	Save all data in building
//-------------------------------------------------------------------
void CBuilder::SaveBuildingData(C3DPart *prt)
{	U_INT nvtx		= CountVertices();
	prt->AllocateOsmGVT(nvtx);
	//--- build all vertices ------------------------------
	U_INT n = 0;
	//--- Save wall vertices ------------------------------
	for (U_INT k=0; k < walls.size(); k++) n = walls[k]->StoreData(prt,n);
	//--- Save roof vertices ------------------------------
	for (U_INT k=0; k < roof.size() ; k++) n = roof[k]->StoreData(prt,n);
	//--- Cross check validity ----------------------------
	if (n > nvtx)	gtfo("Bad vertice count");
	//--- Release walls and roof --------------------------
	for (U_INT k=0; k < walls.size(); k++)	delete walls[k];
	walls.clear();
	for (U_INT k=0; k < roof.size();  k++)  delete roof[k];
	roof.clear();
	//--- Release bivel plane -----------------------------
	delete []  bevel;
	bevel		= 0;
	//-----------------------------------------------------
	return;
}
//===================================================================
//	Edit the requested building
//===================================================================
//-----------------------------------------------------------------------
//	Modify the style of current object
//-----------------------------------------------------------------------
U_CHAR CBuilder::ModifyStyle(D2_Style *sty)
{	if (0 == osmB)							return 0;
	if (!osmB->CanBeModified())	return 0;
  //--- check if style can be modified -------------
  osmB->Swap(extp);
	MakeSlot();
	osmB->ChangeStyle(sty,this);
	return 1;
}
//-----------------------------------------------------------------------
//	Select one building
//-----------------------------------------------------------------------
bool CBuilder::SelectOBJ(U_INT No)
{	//--- Deselect previous selection------------------
	char same = (osmB != 0)  && (osmB->SameStamp(No));
	if (same)	osmB->SwapSelect();
	//--- see for new building ------------------------
	else
	{	if (osmB)	osmB->Deselect();
		//--- Get the new object ------------------------
		OSM_Object *bld = session->GetObjectOSM(No);
		//--- Establish the new building ------------------
		osmB		= bld;
		osmB->Select();
	}
	//---- Restore Parameters -------------------------
//	BDP		= osmB->GetParameters();
	return true;
}

//-----------------------------------------------------------------------
//	Delete the current building
//-----------------------------------------------------------------------
void CBuilder::RemoveOBJ()
{	if (0 == osmB)		return;
	remB		= osmB;
	osmB->Deselect();
	osmB->Remove();
	osmB		= 0;
	return;
}
//-----------------------------------------------------------------------
//	Restore the last deleted building
//-----------------------------------------------------------------------
void CBuilder::RestoreOBJ()
{	if (0 == remB)			return;
	if (osmB)	osmB->Deselect();
	osmB	= remB;
	osmB->Restore();
	osmB->Select();
	remB			= 0;
	return;
}
//----------------------------------------------------------------
//	Rotate the building
//	NOTE: Only the vertice are rotated.  The building original
//			orientationmust be kept unchanged
//----------------------------------------------------------------
U_CHAR CBuilder::RotateOBJ(double rad)
{	if (0 == osmB)							return 0;
	//--- Set building orientation --------------------
	if (!osmB->CanBeRotated())	return 0;
	//------------------------------------------------
	return osmB->IncOrientation(rad);
}
//-------------------------------------------------------------------
//	Convert Coordinates in feet relative to object center
//-------------------------------------------------------------------
char CBuilder::ConvertInFeet()
{	SPosition &geop = BDP.geop;
	U_INT nvx = extp.GetNbObj();
	if (0 == nvx)			return 1;
	//--- Compute object geo center ------------------
	geop.lon	/= nvx;
	geop.lat  /= nvx;
	//--- Get elevation ------------------------------
	spot.lon   = geop.lon;
  spot.lat   = geop.lat;
	geop.alt   = globals->tcm->GetGroundAt(spot);
	//--- Get Expension factor -----------------------
	double rad = FN_RAD_FROM_ARCS(geop.lat);			// DegToRad(lat / 3600);
  BDP.rdf = cos(rad);
	//------------------------------------------------
	D2_POINT *pp = 0;
	SPosition to;
	for (pp = extp.GetFirst(); pp != 0; pp = pp->next)
	{ to.lon	= pp->x;
		to.lat  = pp->y;
		to.alt	= 0;
		CVector feet = FeetComponents(geop,to,BDP.rdf);
		pp->x		= feet.x;
		pp->y		= feet.y;
	}
	return 0;
}
//-------------------------------------------------------------------
//	Clear any slot left
//-------------------------------------------------------------------
void CBuilder::ReleaseSlot()
{	D2_SLOT *sp;
	for (sp = slot.Pop(); sp != 0; sp = slot.Pop())
	{	delete sp; 	}
	return;
}
//-------------------------------------------------------------------
//	Recreate slot from foot print
//	NOTE: Rebuild flag vRFX for reflex point. This flag is used in
//				triangulation
//-------------------------------------------------------------------
void CBuilder::MakeSlot()
{ ReleaseSlot();
	D2_POINT *pp;
	vRFX		= 0;
	for (pp = extp.GetFirst(); pp != 0; pp = pp->next)
	{	vRFX |= pp->R;
	  D2_SLOT *sp = new D2_SLOT(pp);
		slot.PutLast(sp);
	}
	return;
}
//-------------------------------------------------------------------
//	Compute type of points (reflex or convex)
//	Create slot for triangulation
//	Compute ground surface
//-------------------------------------------------------------------
int CBuilder::QualifyPoints(D2_BPM *bpm)
{ int nbp = extp.GetNbObj();
	bpm->error = 0;
  if (nbp < 3)				return 2;
	if (hole.GetNbObj() > 2)			Merge();	
	dlg		= 0;
	surf	= 0;
	//--- initialize the first triangle ----
	D2_POINT *ap = extp.GetFirst();
	D2_POINT *np = 0;
	while (ap)
	{	tri.Set(extp.CyPrev(ap),ap,extp.CyNext(ap));
	  int rf = 1 - geo.Positive(tri);
		vRFX  |= rf;
		ap->Type(rf);
		//--- Compute surface of pa edge -------
		np	  = tri.C;
		surf += (ap->y + np->y) * (np->x - ap->x);
		//--- Create a triangulation slot ------
		D2_SLOT *sp = new D2_SLOT(ap);
		slot.PutLast(sp);
		//--- Compute AC edge length -----------
		if (ap->NotNulEdge())
		{	double dx = (np->x - ap->x);
			double dy = (np->y - ap->y);
			double lg = sqrt((dx * dx) + (dy * dy));
			ap->elg		= lg;
			//--- remember extremity points ------
			if (lg > dlg) 
			{TO = ap; dlg = lg; }
		}
		else  ap->elg = 0;
		//--- next vertex ----------------------
		ap = ap->next;
	}
	//--- Adjust real surface (chek orientation)------
	if (surf > 0)		return ReOrientation(bpm);
	bpm->surf = -0.5 * surf;
	//--- Reorder with TO as origin point ----
	Reorder();
	//--- Save longuest edge -----------------
	bpm->dlg	= dlg;
	//--- Check  precision -------------------
	if (trace) TraceInp();
	bpm->error = (dlg < geo.GetPrecision())?(2):(0);
	return  bpm->error;
}
//-------------------------------------------------------------------
//	Compute type of points at triangle extremity
//-------------------------------------------------------------------
void CBuilder::Requalify(D2_SLOT *sa)
{	if (slot.GetNbObj() <= 3)		return;
	//--- Check the B extremity ------------------------------
	D2_SLOT *sb = slot.CyPrev(sa);
	D2_SLOT *sc = slot.CyNext(sa);
	qtr.Set(sb->GetVRTX(),sa->GetVRTX(),sc->GetVRTX());
	int tp = 1 - geo.Positive(qtr);
  sa->SetType(tp);
	return;
}
//-------------------------------------------------------------------
//	Get a Ear
//	There must be one
//	When one is detected, remove the Ear point and add triangle to 
//	the Ear list
//	Result of triangulation is used as the flat roof
//-------------------------------------------------------------------
bool CBuilder::GetAnEar()
{ D2_SLOT  *sa = 0;
	for (sa = slot.GetFirst(); sa != 0; sa= sa->next)
	{ if (sa->IsReflex())				continue;
		if (NotAnEar(sa))					continue;
		//------ Get an ear -------------------------------
		D2_TRIANGLE *t = new D2_TRIANGLE();
		*t			= tri;
		roof.push_back(t);
		//--- remove ear slot -----------------------------
		D2_SLOT *sb = slot.CyPrev(sa);
		D2_SLOT *sc = slot.CyNext(sa);
		slot.Detach(sa);
		delete sa;
		//--- We must requalify both extremities ----------
		//	Because reflex vertice may disappear and this
		//	is needed to find the next ear
		if (sb->IsReflex())	Requalify(sb);
		if (sc->IsReflex()) Requalify(sc);
		//-------------------------------------------------
		return true;
	}
	STREETLOG("Building %05d Cannot Triangulate", BDP.stamp);
	return false;
}
//-------------------------------------------------------------------
//	Check for a Ear
//	Extract triangle to test in BAC
//	Starting from C and up to B look for any reflex vertex
//	Check if any reflex is in triangle ABC or on a side of ABC
//-------------------------------------------------------------------
bool CBuilder::NotAnEar(D2_SLOT *sa)
{	//--- Extract the oriented triangle -----------------
  D2_SLOT  *sb = slot.CyPrev(sa);
	D2_SLOT  *sc = slot.CyNext(sa);
  D2_POINT *pa = sa->GetVRTX();
	D2_POINT *pb = sb->GetVRTX();
	D2_POINT *pc = sc->GetVRTX();
	tri.Set(pb,pa,pc);
	if (0 == vRFX)									return false;
	if (3 == slot.GetNbObj())				return false;
	//--- try each reflex vertex -------------------------
	D2_SLOT  *sf = slot.CyPrev(sb);
	D2_SLOT  *rs = sc;
	while (rs != sf)
	{	rs	= slot.CyNext(rs);
	  if (!rs->IsReflex())					continue;
		int pos = geo.InTriangle(*rs->GetVRTX(),tri);
		if (pos != GEO_OUTSIDE)				return true; 
	}
	return false;
}
//-------------------------------------------------------------------
//	Start triangulation.  The result is a flat roof
//-------------------------------------------------------------------
char CBuilder::Triangulation()
{	tri.N = CVector(0,0,1);
	roof.reserve(slot.GetNbObj() - 1);
	while (slot.GetNbObj() != 2)	if (!GetAnEar()) 	return 3;
	slot.Clear();
	if (trace) TraceOut();
	return 0;
}

//-------------------------------------------------------------------
//	Trace Input
//-------------------------------------------------------------------
void CBuilder::TraceInp()
{	char  ida[6];
  char *dim;
  D2_POINT *pa;
	TRACE("=====Input Polygon ======================");
	for (pa = extp.GetFirst(); pa != 0; pa = pa->next)
	{	pa->Id(ida);
	  dim = (pa == TO)?("***"):(" ");
	  TRACE("%s x=%.02lf y=%.02lf lgr=%.02lf %s",ida,pa->x,pa->y,pa->elg,dim);
	}
	TRACE("=========================================");
}
//-------------------------------------------------------------------
//	Trace Faces
//-------------------------------------------------------------------
void CBuilder::TraceFace()
{	char  ida[6];
  char *type = "None";
  D2_POINT *pa;
	TRACE("=====Input Faces  ======================");
	U_INT k		= 0;
	U_INT end = extp.GetNbObj();
	for (pa = TO; k < end; pa = extp.CyNext(pa), k++)
	{	pa->Id(ida);
	  type = FaceType[pa->F];
	  TRACE("%s lx=%.02lf ly=%.02lf type=%s",ida,pa->lx,pa->ly,type);
	}
	TRACE("========================================");
}

//-------------------------------------------------------------------
//	Trace triangulation
//-------------------------------------------------------------------
void CBuilder::TraceOut()
{	char ida[6];
	char idb[6];
	char idc[6];	
	for (U_INT k = 0; k < roof.size();k++)
	{	D2_TRIANGLE *t = roof[k];
		t->A->Id(ida);
		t->B->Id(idb);
		t->C->Id(idc); 
	  TRACE("T%3d B=%s A=%s C=%s",k,idb,ida,idc);
	}
	TRACE("Surface = %.02f",surf);
	TRACE("========================================");
}
//-------------------------------------------------------------------
//	Join external polygon to Hole polygon
//-------------------------------------------------------------------
void CBuilder::Merge()
{	if (3 > hole.GetNbObj())		return;
	if (3 > extp.GetNbObj())		return;
	//--- search  for a pair of mutually visible points --
	D2_POINT *xp = 0;
	D2_POINT *hp = 0;
	for (xp = extp.GetFirst(); xp != 0; xp = xp->next)
	{	hp = MatchHole(xp);
		if (0 == hp)	continue;
		Splice(xp,hp);
		return;
	}
	gtfo("Cant get a match pair");
}
//-------------------------------------------------------------------
//	Search a hole point visible from xp
//-------------------------------------------------------------------
D2_POINT *CBuilder::MatchHole(D2_POINT *xp)
{	D2_POINT *hp = 0;
	for (hp = hole.GetFirst(); hp != 0; hp = hp->next)
	{	if (!CheckInternal(xp,hp))		continue;
		if (!CheckExternal(xp,hp))		continue;
		return hp;
	}
	return 0;
}
//-------------------------------------------------------------------
//	check that no edge of external contour masks 
//	xp from hp
//-------------------------------------------------------------------
bool CBuilder::CheckExternal(D2_POINT *xp, D2_POINT *hp)
{	//--- exclude Edges incident to xp -------------------
	D2_POINT *p1 = extp.CyNext(xp);
	D2_POINT *p2 = 0;
	D2_POINT *lp = extp.CyPrev(xp);
	while (p1 != lp)
		{	p2	= extp.CyNext(p1);
			if (!geo.VisibilityTest(*hp,*xp,*p1,*p2))	return false;
			p1	= p2;
		}
	return true;
}
//-------------------------------------------------------------------
//	check that no edge of internal contour masks 
//	xp from hp
//-------------------------------------------------------------------
bool CBuilder::CheckInternal(D2_POINT *xp, D2_POINT *hp)
{ //--- Exclude Edges incident to hp ------------------
	D2_POINT *p1 = hole.CyNext(hp);
	D2_POINT *p2 = 0;
	D2_POINT *lp = hole.CyPrev(hp);
	while (p1 != lp)
	{	p2 = hole.CyNext(p1);
		if (!geo.VisibilityTest(*xp,*hp,*p1,*p2)) return false;
		p1 = p2;
	}
	return true;
}
//-------------------------------------------------------------------
//	Splice both polygons so as to have only one without a hole
//	This become the new external polygon.
//	We do this by inserting to internal edges that link both
//  list
//	NOTE internal  list should be clock wise (negative direction)
//			else results are unknown
//-----------------------------------------------------------------
//	We go from  
//		external list:		(X-1) <=> (XP) <=> (X+1) .....
//																 ||  a new pair of edges
//		internal list:		(H-1) <=> (HP) <=> (H+1) .....
//	To the following result (in 4 steps)
//		           step   1                  2            3        4
//			(X-1) <=> (XP) <=> (HP) <=> (H+1).... <=> (H2) <=> (X2) <=> (X+1)
//	with
//			H2 = copy of HP
//			X2 = copy of XP
//-------------------------------------------------------------------
void CBuilder::Splice(D2_POINT *xp, D2_POINT *hp)
{	//--- Transfer hole chain here ------------------------------
	int       nn = hole.GetNbObj();
	D2_POINT *hn = hole.SwapFirst();
	D2_POINT *ln = hole.SwapLast();
	//---- Dupplicate both nodes for end of chain ---------------
	D2_POINT *h2 = new D2_POINT(*hp);
	D2_POINT *x2 = new D2_POINT(*xp);
	//--- Step 1: Splice xp and hp ------------------------------
	//    this creates internal edge e1 from exterior to inside
	xp->next	= hp;			// hp next (to H+1) is ok 
	hp->prev	= xp;
	xp->SetEdge('N');		// This is e1 null edge
	//--- Step 2 Introduce origin edge 2 with h2: ---------------
	//		Link to hole chain. Going from inside to exterior 
	h2->prev	= ln;			// Link to end of holes	
	ln->next  = h2;
	h2->SetEdge('N');		// This is e2 null edge
	//--- Step 3 Introduce extremity edge 2 with x2 -------------
	h2->next	= x2;			// Link to extremity
	x2->prev  = h2;
	//----Step 4 Link new extremity with remaining exterior -----
	D2_POINT *rm = x2->next;
	if (rm)   rm->prev = x2;
	//--- Increment Queue and update queue pointers----------------
	extp.Update(nn+2,xp,x2);
	return;
}
//=========================================================================
//	Compute the bounding rectangle and set wall attraction
//
//	We look for the longuest edge AB and take it as the X axis, thus computing
//	the angle A  from X axis to the Edge 
//	Then new local coordinates for edge are computed to get extension
//
//	We have rot(A) = |cos(A)  sin(A)| for angle A. 
//								   |-sin(A) cos(A)|
//	where cos(A) = dx / lentgh(AB)
//				sin(A) = dy / lentgh(AB)
// We just have to change sin(-A) to -sin(A) for the inverse rotation
//	if the largest edge is near zero lentght, this have been detected
//	in QualifyPoints() already.
//	NOTE:  With this procedure, some local coordinates may be negative
//	So we renormalize all coordinates to be positive by the translation
//	T=(-minx,-miny) and we commpute the s coordinate
//	
//	The global transformation is then G= R(T) where T is the global
//	translation and R is the rotation (-alpha)
//
//	Before calling this type, the style must be selected and set
//	We need it to precompute some texture coefficients
//=========================================================================
void CBuilder::OrientFaces(D2_BPM *bpm)
{	TO	= extp.GetFirst();
	//--- Compute rotation matrix -------------------
  D2_POINT *pa = TO;
	D2_POINT *pb = extp.CyNext(pa);
  cosA = (pb->x - pa->x) / dlg;					// Cos(A)
	sinA = (pb->y - pa->y) / dlg;					// Sin(A)
	//--- As we want a -A rotation, we change the signe of sin(A) aka dy
	NegativeROT(sinA,cosA);
	//----Init extension ----------------------------------
	minx		= maxx = 0;
	miny    = maxy = 0;
	//---- compute new coordinates relatives to PO---------
	D2_POINT *pp = 0;
	int       No = 0;
	for (pp = extp.GetFirst(); pp != 0; pp = pp->next)
	{	double dx = (pp->x - pa->x);
		double dy = (pp->y - pa->y);
		Rotate(dx,dy);
		//--- Save both max, min extensions ----------------
		pp->lx		= rx;
		pp->ly		= ry;
		if (rx < minx)	minx = rx;
		if (rx > maxx)	maxx = rx;
		if (ry < miny)	miny = ry;
		if (ry > maxy)	maxy = ry;
		//--- Set point order ------------------------------
		if (pp->prev)  QualifyEdge(pp);
		pp->rng	= No++;
	}
  //--- Now we may qualify the first face ---------------
	//	because the last edge is computed
	QualifyEdge(extp.GetFirst());
	//--- Compute and save Building extensions ------------
	Xp	= maxx - minx;
	Yp	= maxy - miny;
	bpm->lgy = Yp;
	bpm->lgx = Xp;
	//-----------------------------------------------------
	//   Second pass for 
	//	1) Relocation of local coordinates
	//		Each local coordinate is translated so they are
	//		positives
	//	2) Final tour list is saved 
	//	Global transform is to get a local POINT back to
	//	world polygon system.
	//-----------------------------------------------------
	for (pp = extp.GetFirst(); pp != 0; pp = pp->next)
	{	 pp->LocalShift(-minx,-miny);
	}
	//--- compute global transformation -------------------
	double tx = (-TO->x);	
	double ty = (-TO->y);	
	//-------------------------
	mat[0]	= +cosA;
	mat[1]	= -sinA;
	mat[2]	= 0;
	///------------------------
	mat[3]	= +sinA;
	mat[4]	= +cosA;
	mat[5]	= 0;
	//-------------------------
	mat[6]	= (tx * cosA) + (sinA * ty) - minx;
	mat[7]	= (ty * cosA) - (sinA * tx) - miny;
	mat[8]	= 1;
	//-----------------------------------------------------
	if (trace) TraceFace();
	return;
}
//----------------------------------------------------------------
//	Global transform
//----------------------------------------------------------------
void CBuilder::LocalCoordinates(D2_POINT &p)
{	gx = (p.x * mat[0]) + (p.y * mat[3]) + mat[6];
	gy = (p.x * mat[1]) + (p.y * mat[4]) + mat[7];
	p.lx = gx;
	p.ly = gy;
	return;
}
//----------------------------------------------------------------
//	Init a Negative Rotation
//----------------------------------------------------------------
void CBuilder::NegativeROT(double sn, double cs)
{	//--- As we want a -A rotation, we change the signe of sin(A)
	rotM[0]	= +cs; rotM[1] = -sn;
	rotM[2] = +sn; rotM[3] = +cs;
	return;
}
//----------------------------------------------------------------
//	Init a Positive Rotation
//----------------------------------------------------------------
void CBuilder::PositiveROT(double sn, double cs)
{	//--- As we want a +A rotation, we keep the normal way
	rotM[0]	= +cs; rotM[1] = +sn;
	rotM[2] = -sn; rotM[3] = +cs;
	return;
}

//----------------------------------------------------------------
//	Rotation
//----------------------------------------------------------------
void CBuilder::Rotate(double x, double y)
{	rx = ((x * rotM[0]) + (y * rotM[2]));
	ry = ((x * rotM[1]) + (y * rotM[3]));
}
//----------------------------------------------------------------
//	Qualify an edge with previous point
//	We qualify the edge pa which is the previous point of pb
//	because pa is already computed.
//----------------------------------------------------------------
void CBuilder::QualifyEdge(D2_POINT *pb)
{	D2_POINT *pa = extp.CyPrev(pb);
	double dx = pb->lx - pa->lx;
	double dy = pb->ly - pa->ly;
	//--- Qualify the face whose base is pa ------------
	char F= (fabs(dy) > fabs(dx))?(GEO_Y_FACE):(GEO_X_FACE);
	if ((F == GEO_X_FACE) && (dx < 0))   F |= GEO_N_FACE;
	if ((F == GEO_Y_FACE) && (dy < 0))   F |= GEO_N_FACE;
	pa->SetFace(F);
	//--- Compute coordinate ratios ----------------------
	return;
}
//=========================================================================
//	Extruding the building faces
//=========================================================================
void CBuilder::BuildWalls(D2_BPM *bpm)
{	Wz = 0;
  double H = bpm->flHtr;
	int  end = bpm->flNbr;
	for (int k=1; k <= end; k++)
	{	BuildFloor(k,Wz,bpm);
		Wz += H;
	}
	return;
}
//----------------------------------------------------------------
//	Trace Bevel point
//----------------------------------------------------------------
void CBuilder::TraceBevel(D2_POINT *p)
{	TRACE("BEVEL  B%05d rng=%04d Xp=%.4lf Yp=%.4lf P.x=%.4lf P.y=%.4lf P.z=%.4lf lx=%.4lf ly=%.4lf",BDP.stamp,p->rng, Xp,Yp,p->x, p->y, p->z,p->lx,p->ly);
}
//----------------------------------------------------------------
//	Check that bevel point is inside the building, at good height
//----------------------------------------------------------------
bool CBuilder::SetPointInside(D2_POINT *p, D2_POINT *s, double H)
{	bool in = (p->lx >= 0) && (p->lx <= Xp);
	in     &= (p->ly >= 0) && (p->ly <= Yp);
	p->z  = H;
	if (in)		return true;
 *p			= *s;
	p->z	= H;
	return false;
}
//----------------------------------------------------------------
//	Return bevel vector for POINT 'a' with distance to edge 'dy'
//	Compute a local bevel vector then apply global transformation
//	to get the final point
//----------------------------------------------------------------
void CBuilder::GetBevelVector(D2_POINT *pa, double dy,D2_POINT *P)
{	D2_FACE trio;
	D2_POINT &A = trio.sw;
	//--- Extract the 3 points -------------------------
	trio.Copy(pa, extp.CyPrev(pa), extp.CyNext(pa));
	trio.MoveToSW();							// Transform T1= translate
	trio.CRotation();							// Transform T2= Align AB to X axis
	trio.BissectorBC(dy);					// Compute bissector in D;
	trio.RotationMC(P);						// Back to global system of Polygon P
	//--- Compute relative coordinates -----------------
	LocalCoordinates(*P);
	return;
}
//----------------------------------------------------------------
//	Build a foot print of the beveled roof
//----------------------------------------------------------------
int CBuilder::SetBevelArray(D2_BEVEL &pm)
{ //--- Bevel distance ------------------------
	double dy = fabs(pm.H / pm.tang);
	D2_POINT *dst = bevel;
	D2_POINT *src;
	//--------------------------------------------------
	double    ce = pm.pah + pm.H;								// Ceil height
	for (src = extp.GetFirst(); src != 0; src = src->next)
	{	*dst	= *src;								// Copy the source points
	   GetBevelVector(src, dy,dst);
		 SetPointInside(dst,src,ce);
		 //--- allocate the point height ---------------
		 src->z  = pm.pah;					// Elevate source point
		 dst++;
	}
	return 0;
}
//----------------------------------------------------------------
//	Allocate bevel points
//----------------------------------------------------------------
D2_POINT *CBuilder::AllocateBevel(int nb)
{	bevel = new D2_POINT[nb];
	return bevel;
}
//----------------------------------------------------------------
//	Translate a point (this is horizontal rotation only) to
//	real world coordinates, from local coordinates (lx,ly)
//	Tx,Ty,Tz define the translation vector
//	It must be rotated back to world coordinates before to be
//	added to the POINT
//----------------------------------------------------------------
void CBuilder::TranslatePoint(D2_POINT &P, CVector &T)
{	//---- Adjust local components ----------------
	P.lx	+= T.x;							//tx;
	P.ly	+= T.y;							//ty;
	P.z   += T.z;							//tz;
	//---- Compute real translation vector -------
	PositiveROT(sinA, cosA);
	Rotate(T.x,T.y);
	P.x		+= rx;
	P.y		+= ry;
	return;
}
//----------------------------------------------------------------
//	Make a bevel array
//	For each contour point, build a corresponding bevel point
//	-Normal vertex is beveled inside
//	-Reflex vertex is beveled outside
//	-First we compute the bevl vector componnent in local coordinates
//	-Then we rotate it in the world referential
//----------------------------------------------------------------
int CBuilder::BuildBevelFloor(int No, int inx, double afh, D2_BPM *bpm)
{	double     H  = bpm->flHtr;
  D2_Style *sty = bpm->style;
	D2_BEVEL pm;
	U_INT nbp		= extp.GetNbObj();
	pm.tang			= t70;
	pm.pah			= afh;
	pm.H				= H;
	bevel		    = new D2_POINT[nbp];
	SetBevelArray(pm);
	double    ce = afh + H;								// Ceil height
	//--- Build this floor -------------------------------
  D2_POINT *pa = 0;
	D2_FLOOR *et = new D2_FLOOR(No,inx,afh,ce);
	D2_FACE  *fa = 0;
	for (pa = extp.GetFirst(); pa != 0; pa = pa->next)
	{	fa = new D2_FACE(pa->FaceType(),inx);
		//--- Now we build a face with 'pa' as the base point
		D2_POINT *pb = extp.CyNext(pa);
		//--- Allocate the 4 points -----------------------
		fa->sw		= *pa;
		fa->se		= *pb;
		fa->ne		=  bevel[pb->rng];
		fa->nw		=  bevel[pa->rng];
		fa->SetNorme(&geo);
		//--- Link faces ------------------
		fa->sw.next = &fa->se;
		fa->nw.next = &fa->ne;
		//--- Set Face top position ------
		fa->ne.V	= 1;
		fa->nw.V	= 1;
		//------------------------------------------------
		et->faces.push_back(fa);
	}
	sty->SetWz(ce);
	bpm->hgt = ce;				//sty->SetBz(ce);
  walls.push_back(et);
	//--- Change foot print points to the bevel plane--
	for (pa = extp.GetFirst(); pa != 0; pa= pa->next)
	{	D2_POINT &P = bevel[pa->rng];
		pa->x		= P.x;
		pa->y		= P.y;
		pa->z		= P.z;
		pa->lx	= P.lx;
		pa->ly  = P.ly;
	}
	//--- Now delete the bevel array ------------------
	delete [] bevel;
	bevel			= 0;
	return ce;
}
//----------------------------------------------------------------
//	Build a normal floor
//----------------------------------------------------------------
int	CBuilder::BuildNormaFloor(int No,int inx, double afh, D2_BPM *bpm)
{	double     H = bpm->flHtr;
  double    ce = afh + H;								// Ceil height
  D2_POINT *pa = 0;
	D2_FLOOR *et = new D2_FLOOR(No,TEXD2_FLOORZ,afh,ce);
	D2_FACE  *fa = 0;
	for (pa = extp.GetFirst(); pa != 0; pa = pa->next)
	{	fa = new D2_FACE(pa->FaceType(),inx);
		//--- Now we build a face with pa as the base points
		D2_POINT *pb = extp.CyNext(pa);
		fa->Extrude(afh,ce,pa,pb);
		fa->SetNorme(&geo);
		et->faces.push_back(fa);
		pa->z		= ce;
		pb->z		= ce;
	}
walls.push_back(et);
return ce;
}

//----------------------------------------------------------------
//	Build floor number no
//	f = floor 
//	We mke tour of external points (extp) and for each point,
//	We extrude wall with
//	afh = absolute floor height
//	ce = ceil height
//	We also adjust the roof height in (extp)
//----------------------------------------------------------------
void CBuilder::BuildFloor(int No, double afh, D2_BPM *bpm)
{	//--- Compute face code -----------------------------
	D2_Style *sty = bpm->style;
	int  flNbr		= bpm->flNbr;
	double     H  = bpm->flHtr;
	double ce = 0;
	char indx = TEXD2_FLOORM;								// For middle floor
	if (flNbr == No)	indx = TEXD2_FLOORZ;	// Last floor
	if (1			== No)	indx = TEXD2_FLOOR1;	// ground floor
	bool mans = (indx == TEXD2_FLOORZ) && sty->IsMansart();
	if (mans)	ce	=		BuildBevelFloor(No,indx,afh,bpm);
	else			ce	=		BuildNormaFloor(No,indx,afh,bpm);
	sty->SetWz(ce);
	bpm->hgt = ce;			// Save ceil for next floor;
	return;
}

//----------------------------------------------------------------
//	Reorder the tour with origin as first point
//----------------------------------------------------------------
void CBuilder::Reorder()
{	int k = extp.GetNbObj();
	while (extp.GetFirst() != TO)	
	{	if (k <0)	return;
		D2_POINT *pq = extp.Pop();
		extp.PutLast(pq);
		k--;
	} 
	return;
}
//----------------------------------------------------------------
//	Empty actual roof
//----------------------------------------------------------------
void CBuilder::ClearRoof()
{	//--- Delete current roof -----------------------------
	for (U_INT k=0; k < roof.size(); k++) delete roof[k];
	roof.clear();
	return;
}
//----------------------------------------------------------------
//	Select roof model
//----------------------------------------------------------------
void CBuilder::SelectRoof(D2_BPM *bpm)
{	if (bpm->roofM->GetClearIndicator())	ClearRoof();
	//--- Assign roof parameters---------------------
	CRoofModel *rofm = bpm->roofM;
	rofm->SetRoofData(bpm, this);
	rofm->BuildRoof(extp,roof);
	Zp			= rofm->GetTop();
	return;
}
//----------------------------------------------------------------
//	Texturing pass
//----------------------------------------------------------------
void CBuilder::Texturing(D2_BPM *bpm)
{	D2_Style *sty = bpm->style;
  if (0 == sty)		return;
	//------------------------------------------------- 
	for (U_INT k = 0; k < walls.size(); k++) 
	{	D2_FLOOR *fl = walls[k];
		fl->TextureFloor(sty);
	}
	return;
}
//----------------------------------------------------------------
//	Helpers
//----------------------------------------------------------------
void			CBuilder::ActualPosition(SPosition &P) {if (osmB) osmB->ObjGeoPos(P);}
U_INT			CBuilder::ActualStamp()			{ return (osmB)?(osmB->GetStamp()):(0); }
D2_Style *CBuilder::ActualStyle()			{	return (osmB)?(osmB->GetStyle()):(0); }
int       CBuilder::ActualError()			{ return (osmB)?(osmB->GetError()):(0); }
char      CBuilder::ActualFocus()			{ return (osmB)?(osmB->GetFocus()):(0); }

//=========================================================================
//	D2-FACE class
//=========================================================================
D2_FACE::D2_FACE(char type, char fx)
{ strncpy(idn,"D2FA",4);
	fType = type;
	fIndx = fx;}
//---------------------------------------------------------------
//	Extrude a face from  SW point PA
//---------------------------------------------------------------
D2_FACE::~D2_FACE()
{	int a = 0;	}
//---------------------------------------------------------------
//	Extrude a face from  SW point PA
//	fh = floor height
//	ch = ceil  height
//---------------------------------------------------------------
void D2_FACE::Extrude(double fh, double ch, D2_POINT *p0, D2_POINT *p1)
{	sw		= *p0;
	se		= *p1;
	ne		= *p1;
	nw		= *p0;
	//--- Link first point -----------
	sw.next	= &se;
	nw.next	= &ne;
	//--- Set Face top position ------
	ne.V	= 1;
	nw.V	= 1;
	//--- Save heights ----------------
	sw.z = se.z = fh;
	ne.z = nw.z = ch;
	return;
}
//---------------------------------------------------------------
//	Compute the face normal
//---------------------------------------------------------------
void	D2_FACE::SetNorme(GeoTest *geo)
{	N = geo->PlanNorme(nw, se, sw);
	return;
}
//---------------------------------------------------------------
//	Texture this face
//	Y faces are texture as a whole or as floor
//	hb is set when face is either Y+ or Y- 
//---------------------------------------------------------------
void D2_FACE::TextureFaceByPoint(D2_Style *sty)
{	char hb = (fType & GEO_Y_FACE)?(1):(0);
  sty->TexturePoint(&sw,fType,fIndx,hb);
	sty->TexturePoint(&se,fType,fIndx,hb);
	sty->TexturePoint(&ne,fType,fIndx,hb);
	sty->TexturePoint(&nw,fType,fIndx,hb);
	return;
}
//---------------------------------------------------------------
//	Texture this face
//	Y faces are texture as a whole or as floor
//	hb is set when face is either Y+ or Y- 
//---------------------------------------------------------------
void D2_FACE::TextureFaceByFaces(D2_Style *sty)
{	char hb = (fType & GEO_Y_FACE)?(1):(0);
  sty->TextureByFace(&sw,fType,fIndx,hb);
	sty->TextureByFace(&nw,fType,fIndx,hb);
	return;
}

//--------------------------------------------------------------
//	Dupplicate A,B,C points where
//	A is between B and C
//	B is previous
//	C is next
//--------------------------------------------------
	void D2_FACE::Copy(D2_POINT *A, D2_POINT *B, D2_POINT *C)
	{	sw	= *A;
		nw	= *B;
		se  = *C;
		ne	= *A;							// A is dupplicated
	}

//---------------------------------------------------------------
//	Translate point face to SW corner:
//	Others corners becomes relative to SW corner
//	Preserve  NE (a dupplicate of A)
//---------------------------------------------------------------
void D2_FACE::MoveToSW()
{	//--- Translate SE corner (C)------------------
	se.x	-= sw.x;
	se.y  -= sw.y;
	//--- Translate NW corner (B) ----------------
	nw.x  -= sw.x;
	nw.y  -= sw.y;
	//--- SW becomes null vector -----------------
	sw.x   = 0;
	sw.y   = 0;
}
//---------------------------------------------------------------
//	Rotate B and C to align C along X axis
//	We rotate B and C by -alpha where alpha is the angle
//	between AC and  axis
//---------------------------------------------------------------
void D2_FACE::CRotation()
{	D2_POINT &A = sw;
	D2_POINT &B = nw;
	D2_POINT &C = se;
	D2_POINT &D = ne;
	double cosa =  C.x / A.elg;
	double sina =  C.y / A.elg;
	//--- Save in D local ---------------------------
	D.lx				= cosa;
	D.ly				= sina;
	//------------------------------------------------
	double mt00 = +cosa;								// +cosinus alfa
	double mt01 = -sina;								// -sinus alpha
	double mt10 = +sina;								// +sinus alpha
	double mt11 = +cosa;								// +cosinus alpha 
	//--- Rotate C --------------------------------------
	double cx = (C.x * mt00) + (C.y * mt10);
	double cy = (C.x * mt01) + (C.y * mt11);
	C.x	= cx;
	C.y	= cy;
	//--- Rotate B -------------------------------------
	double bx = (B.x * mt00) + (B.y * mt10);
	double by = (B.x * mt01) + (B.y * mt11);
	B.x	= bx;
	B.y	= by;
	return;
}
//---------------------------------------------------------------
//	Compute bissector of B and C vector
//	Then compute the dx composite for distance dy
//	NOTEs:  A.elg is the AC length
//					B.elg is the AB length
//---------------------------------------------------------------
void D2_FACE::BissectorBC(double dy)
{ D2_POINT &A = sw;
	D2_POINT &B = nw;
	D2_POINT &C = se;
	D2_POINT &D = ne;
	//--- X composite of bissector  ----------
	D.x = (A.elg * B.x) + (B.elg * C.x);
	D.y = (A.elg * B.y) + (B.elg * C.y);
	//--- Save local bevel vector in A -------
	A.y	= dy;
	//--- compute dx from dy ------------------
	A.x = (dy * D.x) / D.y;
	return;
}
//---------------------------------------------------------------
//	Reverse B rotation 
//	compute (Ax,Ay) vector bivel in world coordinates
//  Set final vector in  P       
//---------------------------------------------------------------
void D2_FACE::RotationMC(D2_POINT *P)
{	D2_POINT &A = sw;
	D2_POINT &D = ne;
	double mt00 = +D.lx;								// +cosinus alfa
	double mt01 = +D.ly;								// +sinus alpha
	double mt10 = -D.ly;								// -sinus alpha
	double mt11 = +D.lx;								// +cosinus alpha 
	//----- Rotate back (this is a local vector)-----------
	double Ax		= (A.x * mt00) + (A.y * mt10);
	double Ay		= (A.x * mt01) + (A.y * mt11);
	double Az		= 0;
	//---- Save everything in A ---------------------------
	A.x		= Ax;
	A.y		= Ay;
	//--- Compute bevel vector in P -----------------------
	P->x += Ax;
	P->y += Ay;
	return;
}
//---------------------------------------------------------------
//	Save vertices in part for 2 triangles
//---------------------------------------------------------------
U_INT	D2_FACE::StoreData(C3DPart *prt, U_INT n)
{	GN_VTAB *tab = prt->GetGTAB() + n;
	tab = sw.SaveData(tab,N);
	tab = se.SaveData(tab,N);
	tab = nw.SaveData(tab,N);

	tab = nw.SaveData(tab,N);
	tab = se.SaveData(tab,N);
	tab = ne.SaveData(tab,N);

	return 6;
}

//=========================================================================
//	D2-Stair class
//=========================================================================
D2_FLOOR::D2_FLOOR(char e, char t,double f, double c)
{	strncpy(idn,"D2FL",4);
	sNo		= e;
	type	= t;
	floor = f;
	ceil  = c;
} 
//---------------------------------------------------------------
//	Free resources 
//---------------------------------------------------------------
D2_FLOOR::~D2_FLOOR()
{	for (U_INT k=0; k < faces.size(); k++)  delete faces[k];
	faces.clear();
}
//---------------------------------------------------------------
//	Floor texturing 
//---------------------------------------------------------------
void D2_FLOOR::TextureFloor(D2_Style *s)
{ char txf = s->TextureMode();
	for (U_INT k=0; k < faces.size(); k++)
	{	D2_FACE *fa = faces[k];
		if (txf) fa->TextureFaceByFaces(s);
		else		 fa->TextureFaceByPoint(s);
	}
	return;
}
//---------------------------------------------------------------
//	Save vertices in Part 
//---------------------------------------------------------------
U_INT	D2_FLOOR::StoreData(C3DPart *p,U_INT x)
{	U_INT n = x;
	for (U_INT k=0; k < faces.size(); k++) n += faces[k]->StoreData(p,n);
	return n;
}

//---------------------------------------------------------------
//	Return vertices number 
//---------------------------------------------------------------
U_INT D2_FLOOR::VerticesNumber()
{	U_INT nv = 6 * faces.size();
	return nv;	}
//===================================================================================
//	STYLE management
//	Create a new group
//
//===================================================================================
D2_Group::D2_Group(char *gn, D2_Session *s)
	: D2_Ratio()
{	next	= prev	= 0;
	ssn				= s;
	tr				= ssn->HasTrace();
	rlgr = rsuf = rsid	= 0;
	*pTag = *pVal = 0;
	strncpy(name,gn,64);
	name[63]	= 0;
	sfMin			= 0;
	sfMax			= 10000000;
	//-- Default is side [4,200000] ---------
	sdMin			= 4;
	sdMax			= 200000;
	radius		= 0;
	//--- Length ----------------------------
	lgMin			= 0;
	lgMax			= 10000;
	//--- Floor -----------------------------
	flNbr			= 1;
	flHtr			= 2.8;
	//---------------------------------------
	nItem			= 1;
	//--- Index -----------------------------
	indx			= 0;
	//--- No texture yet --------------------
	*txd.name	= 0;
	//----------------------------------------
	tREF			= 0;
}
//-----------------------------------------------------------------
//	Destroy Group 
//	NOTE: Styles and Roofs are deleted from Queue destructor
//-----------------------------------------------------------------
D2_Group::~D2_Group()
{	roofM.clear();
	std::map<U_INT,OSM_Object*>::iterator rp;
	for (rp = building.begin(); rp != building.end(); rp++)  delete (*rp).second;
	building.clear();
	//-------------------------------------------------
	//TRACE("GROUP DESTROYED and free %s",tinf.path);
	globals->txw->Free3DTexture(tREF);

}
//-----------------------------------------------------------------
//	Add a roof model 
//-----------------------------------------------------------------
bool D2_Group::AddRoof(char *rnm, double a, int w)
{	if (a <  0)		return true;
	if (a > 70)		return true;
	//---- Look at roof model in table -----------------------
	CRoofModel *md = FindRoofParameters(rnm, a);
	if (0 == md)	
	{	STREETLOG("Group %s, invalide roof name %s", name, rnm);
		return true;
	}
	//---- Add this roof model -------------------------------
	roofM.push_back(md);
	roofsQ.PutLast(md);
	md->SetRoofModNumber(roofM.size());
	md->SetQuota(w);
	return true;
}
//-----------------------------------------------------------------
//	Read parameters 
//-----------------------------------------------------------------
bool D2_Group::Parse(FILE *f, D2_Session *sn)
{	char buf[128];
	int  pos	= 0;
	bool go   = true;
	while (go)
	{	pos	= ftell(f);
	  char *line = ReadTheFile(f,buf);
		_strupr(line);
		if (DecodeParam(line))	continue;
		//--- Convert all units --------------------------------
		sfMin		= SQRF_FROM_SQRMTR(sfMin);	// M² to FEET²
		sfMax		= SQRF_FROM_SQRMTR(sfMax);
		flHtr	  =	FOOT_FROM_METERS(flHtr);
		lgMin		= FOOT_FROM_METERS(lgMin);
		lgMax		= FOOT_FROM_METERS(lgMax);
		//--- Load texture if any ------------------------------
		LoadTexture();
		//--- Then exit false ----------------------------------
		go	= false;
	}
	//------Back up ------------------------------------------
	fseek(f,pos,SEEK_SET);
	return false;
}
//-----------------------------------------------------------------
//	Get group parameters  parameters 
//-----------------------------------------------------------------
bool D2_Group::DecodeParam(char *prm)
{	double a;
	int    w = 1;
	double lat;
	double lon;
	double rad;
	char tex[64];
	char buf[256];
	strncpy(buf,prm,256);
	_strupr(buf);
	int nf = 0;
	//--- Check geo position -------------------------------
	nf = sscanf(buf," GEOP ( %lf , %lf ) RADIUS = %lf",&lat,&lon,&rad);
	if (nf == 3)
	{	opt.Set(D2_GRP_GEOP);
		geop.lat = FN_ARCS_FROM_DEGRE(lat);
		geop.lon = FN_ARCS_FROM_DEGRE(lon);
		radius   = FN_FEET_FROM_METER(rad);
		return true;
	}
	//--- Check frequency ----------------------------------
	nf = sscanf(buf," FREQ = %d", &w);
	if (nf == 1)	{quota = w;							return true; }
	//--- Check Side length ----------------------------------
	nf = sscanf(buf," LENGTH [ %lf, %lf ]  %c ", &lgMin, &lgMax, &rlgr);
	if (nf >= 2)	{opt.Set(D2_GRP_LNGT);	return true; }
	//--- Check Side length to infinity ----------------------
	nf = sscanf(buf," LENGTH [ %lf, * ]",   &lgMin);
	if (nf == 1)	{opt.Set(D2_GRP_LNGT);	return true; }
	//--- Check for surface --------------------------------
	nf = sscanf(buf," SURFACE [ %lf , %lf ] %c ", &sfMin, &sfMax, &rsuf);
	if (nf >= 2)	{opt.Set(D2_GRP_SURF);	return true; }
	//--- Check for surface to infinity --------------------
  nf = sscanf(buf," SURFACE [ %lf , * ]",   &sfMin);
	if (nf == 1)	{opt.Set(D2_GRP_SURF);	return true; }
	//--- Check for side ------------------------------------
	nf = sscanf(buf," SIDES [ %u , %u ] %c ", &sdMin, &sdMax, & rsid);
	if  (nf >= 2) {opt.Set(D2_GRP_SIDE);	return true; }
	//--- Check for side to infinity ------------------------
	nf = sscanf(buf," SIDES [ %u , * ]",   &sdMin);
	if	(nf == 1) {opt.Set(D2_GRP_SIDE);	return true; }
	//--- Check for a tag value pair ------------------------
	nf = sscanf(buf,"TAG ( %32[^ =)] = %32[^ )] ) ",pTag, pVal);
	if  (nf == 2)	{return true;}
	//--- Check floor number --------------------------------
	nf = sscanf(buf," FLOOR [ %u , %u ]",   &flMin, &flMax);
	if  (2 == nf)	{return true; }
	//--- Check floor number --------------------------------
	nf = sscanf(buf," FLOOR %u",   &flMin);
	if  (1 == nf)	{flMax = flMin;	return true; }
	//--- Check floor height --------------------------------
	nf = sscanf(buf," HEIGHT %lf", &flHtr);
	if	(1 == nf)	return true;
	//--- Check texture file -------------------------------
	nf = sscanf_s(buf," TEXTURE %64s",txd.name,64);
	if  (1 == nf)	return true;
	//--- Check for roof model -----------------------------
	nf = sscanf_s(buf," ROOF ( %8[^ ,)] , %lf ) ( FREQ = %d)",tex,12,&a,&w);
	if (nf == 3)	return AddRoof(tex,a,w);
	//--- Check for flat roof ------------------------------
	nf = sscanf_s(buf," ROOF ( FLAT ) (FREQ = %d)", &w);
	if (nf == 1)  return AddRoof("FLAT",30,w);
	//--- Not a group parameter ----------------------------
	return false;
}
//-----------------------------------------------------------------
//	Add a new style 
//-----------------------------------------------------------------
void	D2_Group::AddStyle(D2_Style *sty)
{	styleQ.PutLast(sty);
	return;
}
//-----------------------------------------------------------------
//	Evaluate Tag value
//-----------------------------------------------------------------
int D2_Group::ValueTag(char *T, char *V)
{	if ((0 == T) ||(0 == V))	return 0;
	if (strcmp(pTag,T) != 0)	return 0;
	if (strcmp(pVal,V) != 0)	return 0;
	return 100;
}
//-----------------------------------------------------------------
//	Evaluate perimeter
//-----------------------------------------------------------------
int D2_Group::ValuePosition(SPosition &p)
{	if (opt.Not(D2_GRP_GEOP))		return 0;
	double dfeet = DistancePositionInFeet(geop, p);
	double fmetr = FN_METRE_FROM_FEET(dfeet);
	if  (fmetr > radius)				return 0;
	return 30;
}
//-----------------------------------------------------------------
//	Evaluate Surface
//-----------------------------------------------------------------
int D2_Group::ValueSurface(double sf)
{	int bad = (rsuf)?(-10):(0);
	if (opt.Not(D2_GRP_SURF))		return 0;
	if (sf < sfMin)							return bad;
	if (sf > sfMax)							return bad;
	return	10;
}
//-----------------------------------------------------------------
//	Evaluate Side
//-----------------------------------------------------------------
int D2_Group::ValueSide(U_INT sd)
{	int bad = (rsid)?(-10):(0);
  if (opt.Not(D2_GRP_SIDE))		return 0;
	if (sd < sdMin)							return bad;
	if (sd > sdMax)							return bad;
	return 10;
}
//-----------------------------------------------------------------
//	Evaluate Side
//-----------------------------------------------------------------
int D2_Group::ValueLength(double lg)
{ int bad = (rlgr)?(-10):(0);
	if (opt.Not(D2_GRP_LNGT))		return 0;
	if (lg < lgMin)							return bad;
	if (lg > lgMax)							return bad;
	return 10;
}
//-----------------------------------------------------------------
//	Select the group 
//	Group is selected based on 
//	sd = number of sides
//	sf = surface in m²
//	Position
//-----------------------------------------------------------------
int D2_Group::ValueGroup(D2_BPM *bpm)
{	int val = 0;
	char *tag = bpm->obj->GetTag();
	char *vtg = bpm->obj->GetVal();
	val += ValueTag(tag,vtg);
	val += ValuePosition(bpm->geop);
	val += ValueSurface (bpm->surf);
	val += ValueSide    (bpm->side);
	val += ValueLength  (bpm->lgx);
	//TRACE("NB=%04d Quota=%0d VAL=%04d Groupe %s",objNB,quota,val,name);
	return val;
}
//-----------------------------------------------------------------
//	Then select a style based on ratio
//-----------------------------------------------------------------
D2_Style *D2_Group::GetOneStyle()
{ D2_Style *sty = styleQ.GetFirst();
	if (!sty->ReachQuota())	return sty;
	styleQ.SwitchToLast(sty);
	return sty;
}
//-----------------------------------------------------------------
//	Select a style  from name
//-----------------------------------------------------------------
D2_Style *D2_Group::GetStyle(char *nsty)
{ D2_Style *sty = 0;
	for (sty = styleQ.GetFirst(); sty != 0; sty = sty->GetNext())
	{	if (sty->SameName(nsty))	return sty;
	}
	return 0;
}
//-----------------------------------------------------------------
//	Select a roof  based on quota, mansard style and side number
//-----------------------------------------------------------------
U_INT D2_Group::SelectOneRoof(D2_BPM *bpm)
{	CRoofModel *rofm = roofsQ.GetFirst();
	//--- check roof model -------------------
	rmno	= 0;
  mans	= bpm->mans;
	if (0 == rofm)					return 0;
	if (mans)								return 0;
	if (bpm->side > 4)      return 0;
	if (rofm->ReachQuota())	roofsQ.SwitchToLast(rofm);
	rmno =  rofm->GetRoofModNumber();
	return rmno;
}
//-----------------------------------------------------------------
//	Get a roof by number 
//-----------------------------------------------------------------
CRoofModel *D2_Group::GetRoofModByNumber(char mans)
{	CRoofModel *dfm = ssn->GetDefaultRoof();
	if (0 == rmno)					return dfm;
  if (0 == roofM.size())	return dfm;
	if (mans)								return dfm;
	return roofM[rmno - 1];
}

//-----------------------------------------------------------------
//	Load texture for this group
//-----------------------------------------------------------------
void D2_Group::LoadTexture()
{	if (*txd.name == 0)		return;
	//-- init the descriptor -------------------------------
	txd.Dir		= FOLDER_OSM_USER;
	txd.apx   = 0xFF;
	txd.azp   = 0x00;
	//TRACE("GROUP LOAD TEXTURE %s",ntex);
	tREF	= globals->txw->GetM3DPodTexture(txd);
	//--- Fill default roof parameters ---------------------
	tRoof.DefaultParameters(txd);
	return;
}
//-----------------------------------------------------------------
//	Draw this group
//-----------------------------------------------------------------
void D2_Group::DrawBuilding()
{	std::map<U_INT,OSM_Object*>::iterator rp;
	if (tREF) tREF->BindTexture();
	for (rp = building.begin(); rp != building.end(); rp++)
	{	(*rp).second->Draw();	}
	return;
}
//-----------------------------------------------------------------
//	Add a building if not already in list
//-----------------------------------------------------------------
void D2_Group::AddBuilding(OSM_Object *bld)
{	U_INT No = bld->GetStamp();
	std::map<U_INT,OSM_Object*>::iterator rp = building.find(No);
	if (rp != building.end())		return;
	building[No]	= bld;
	return;
}
//-----------------------------------------------------------------
//	Remove building from the list
//	The building is marked as deleted
//-----------------------------------------------------------------
void D2_Group::RemBuilding(OSM_Object *bld)
{	U_INT No = bld->GetStamp();
	std::map<U_INT,OSM_Object*>::iterator rp = building.find(No);
	if (rp == building.end())		return;
	building.erase(rp);
	return;
}
//-----------------------------------------------------------------
//	Locate building from the list
//-----------------------------------------------------------------
OSM_Object *D2_Group::FindBuilding(U_INT No)
{	std::map<U_INT,OSM_Object*>::iterator rp = building.find(No);
	if (rp == building.end())	return 0;
	return (*rp).second;
}
//-----------------------------------------------------------------
//	Get a random number of floors
//-----------------------------------------------------------------
int D2_Group::GenFloorNbr()
{	flNbr	= flMin;
	int dta = flMax - flMin;
	if (0 == dta)	return flNbr;
	int rdm = RandomNumber(dta);
	flNbr = flMin + rdm;
	return flNbr; 
}
//===================================================================================
//
//	Create a new style
//
//===================================================================================
D2_Style::D2_Style(char *snm, D2_Group *gp)
{	prev = next = 0;
	strncpy(Name,snm,64);
	Name[63]	= 0;
	group			= gp;
	bpm				= 0;
	mans		  = 0;
	texf			= 0;
	dormer		= 0;
	sText			= 0;
	objNB			= 0;				// Instance number
	gp->GetTexDim(Tw,Th);
	tr				= gp->HasTrace();
	//--- Clear matrix ----------------------------
	for (int k=0; k < TEXD2_MAT_DIM; k++) param[k] = 0; 
}
//-----------------------------------------------------------------
//	Destroy Style 
//-----------------------------------------------------------------
D2_Style::~D2_Style()
{	for (int k=0; k < TEXD2_MAT_DIM; k++)
	{	D2_TParam *p = param[k];
		if (p)	Clear(p);
		if (p)  delete p;
	}
	if (dormer)	delete dormer;
  //--- Free roof parameters --------------------
	roofT.clear();
}
//-----------------------------------------------------------------
//	Remove pointeur from matrix 
//-----------------------------------------------------------------
void	D2_Style::Clear(D2_TParam *p)
{	for (int k=0; k < TEXD2_MAT_DIM; k++) if (param[k] == p) param[k] = 0;
}
//-----------------------------------------------------------------
//	Error
//-----------------------------------------------------------------
bool D2_Style::Error1(char *p)
{	STREETLOG("Mandatory Face(X+) missing: %s", p);
	return false;
}
//-----------------------------------------------------------------
//	Error
//-----------------------------------------------------------------
bool D2_Style::Error2(char *p)
{	STREETLOG("Found Face(Y+) after Face(Y-): %s", p);
	return false;
}
//-----------------------------------------------------------------
//	Read style parameters 
//-----------------------------------------------------------------
bool D2_Style::DecodeStyle(char *prm)
{ int nf = 0;
	char floor[4];
	D2_TParam pm;
	char	er = 0;
	char buf[128];
	strncpy(buf,prm,128);
	_strupr(buf);
	//----------------------------------------------------------------------
	if (strstr(buf,"MANSART")) {mans = 1; return true; }
	//----------------------------------------------------------------------
  nf = sscanf(buf," FREQ %u",&quota);
	if (1 == nf)		return true;
	//--- check mode -------------------------------------------------------
	nf = sscanf(buf," TEXTURECOVER %lf",&cover);
	if (nf == 1) 		{	texf = 1; return true;}
	//--- Check for X+ -----------------------------------------------------
	pm.code	= GEO_FACE_XP;
	nf = sscanf(buf,D2_MXP, &pm.x0, &pm.y0, &pm.Rx, &pm.Ry);
	if (4 <= nf) 		return AddFace(pm);	
	//--- Check for Y+ -----------------------------------------------------
	pm.code = GEO_FACE_YP;
	nf = sscanf(buf,D2_MYP, &pm.x0, &pm.y0, &pm.Rx, &pm.Ry);
	if (4 <= nf)		return AddFace(pm); 	 
	//--- Check for X- ----------------------------------------------------
	pm.code = GEO_FACE_XM;
	nf = sscanf(buf,D2_MXM, &pm.x0, &pm.y0, &pm.Rx, &pm.Ry);
	if (4 <= nf)		return AddFace(pm);
	//--- Check for Y- ----------------------------------------------------
	pm.code = GEO_FACE_YM;
	nf = sscanf(buf,D2_MYM, &pm.x0, &pm.y0, &pm.Rx, &pm.Ry);
	if (4 <= nf) 	  return AddFace(pm);
	//--- Check for ground Floor ------------------------------------------
	pm.code = 0;
	pm.hmod = TEXD2_HMOD_GFLOOR;
	nf = sscanf_s(buf,D2_MGF,floor,4, &pm.x0,&pm.y0,&pm.Rx,&pm.Ry);
	if (5 <= nf)		return AddFloor(floor,pm);
	//--- Check for middle Floor ------------------------------------------
	pm.code = 0;
	pm.hmod = TEXD2_HMOD_MFLOOR;
	nf = sscanf_s(buf,D2_MMF,floor,4,&pm.x0,&pm.y0,&pm.Rx,&pm.Ry);
	if (5 <= nf)		return AddFloor(floor,pm);
	//--- Check for Last Floor ------------------------------------------
	pm.code = 0;
	pm.hmod = TEXD2_HMOD_ZFLOOR;
	nf = sscanf_s(buf,D2_MZF,floor,4,&pm.x0,&pm.y0,&pm.Rx,&pm.Ry);
	if (5 <= nf)		return AddFloor(floor,pm);
	//--- Check for roof texture ----------------------------------------
	pm.code = 0;
	pm.hmod = 0;
	pm.mans	= 0;
	nf = sscanf(buf,D2_ROF,&pm.x0,&pm.y0,&pm.Rx,&pm.Ry,&pm.quota);
	if (4 <= nf)		return AddRoof(pm);
	//---Check for dormer texture----------------------------------------
	nf = sscanf(buf,D2_DMR,&pm.x0,&pm.y0,&pm.Rx,&pm.Ry);
	if (4 == nf)		return AddDormer(pm);
	//--- End of this style ---------------------------------------------
	return false;
}
//----------------------------------------------------------------------
//	Redondant definition
//----------------------------------------------------------------------
bool D2_Style::Error3(D2_TParam *p)
{	STREETLOG("Style %s, double definition %s ignored",Name,"X+");
	delete p;
	return true;
}
//----------------------------------------------------------------------
//	Add a Face texture definition
//----------------------------------------------------------------------
bool  D2_Style::AddFace(D2_TParam &p)
{	D2_TParam *tp = new D2_TParam();
	tp->AdjustFrom(p, this, Tw, Th);
	switch (p.code)	{
		case GEO_FACE_XP:
			tp->SetFace("X+");
			if (param[0])				return Error3(tp);
			//--- Fill whole matrix with it -----------
			for (int k=0; k != TEXD2_MAT_DIM; k++)	param[k] = tp;
			return true;
		case GEO_FACE_YP:
			tp->SetFace("Y+");
			//--- Fill YP and YM ---------------------
			for (int k=0; k != 4; k++) param[TEXD2_IND_YP + k] = tp;
			for (int k=0; k != 4; k++) param[TEXD2_IND_YM + k] = tp;
			return true;
		case GEO_FACE_XM:
			tp->SetFace("X-");
			//--- Fill XM ----------------------------
			for (int k=0; k != 4; k++) param[TEXD2_IND_XM + k] = tp;
			return true;
		case GEO_FACE_YM:
			tp->SetFace("Y-");
			//--- Fill YM -------------------------------
			for (int k=0; k != 4; k++) param[TEXD2_IND_YM + k] = tp;
			return true;	 
}
	return false;
}
//----------------------------------------------------------------------
//	Add a floor texture definition 
//----------------------------------------------------------------------
bool  D2_Style::AddFloor(char *fl, D2_TParam &p)
{	int k	= 0;
	int n = p.hmod & 0x3;     // Limit value
	D2_TParam *tp = new D2_TParam();
	tp->AdjustFrom(p, this, Tw, Th);
	tp->SetFace(fl);
	//--- Enter floor texture in matrix  --------------------------
	if (strncmp(fl,"X+",2) == 0) k = (TEXD2_IND_XP) + n;
	if (strncmp(fl,"Y+",2) == 0) k = (TEXD2_IND_YP) + n;
	if (strncmp(fl,"X-",2) == 0) k = (TEXD2_IND_XM) + n;
	if (strncmp(fl,"Y-",2) == 0) k = (TEXD2_IND_YM) + n;
	param[k] = tp;
	return true;
}
//----------------------------------------------------------------------
//	Add a roof texture definition 
//----------------------------------------------------------------------
bool D2_Style::AddRoof(D2_TParam &p)
{	D2_TParam *tp = new D2_TParam();
	tp->AdjustFrom(p, this, Tw, Th);
	roofT.push_back(tp);
	rtexQ.PutLast(tp);
	tp->SetRoofTexNumber(roofT.size());
	return true;
}
//----------------------------------------------------------------------
//	Add dormer texture definition 
//----------------------------------------------------------------------
bool D2_Style::AddDormer(D2_TParam &p)
{	D2_TParam *tp = new D2_TParam();
	if (dormer) delete dormer;
	tp->AdjustFrom(p, this, Tw, Th);
	dormer = tp;
	return true;
}
//----------------------------------------------------------------------
//	Check for completion 
//----------------------------------------------------------------------
bool D2_Style::IsOK()
{	//--- Check first floor --------------------------------
	if (*param != 0)	return true;
	STREETLOG("Style %s Missing Face(X+) definition",Name);
	return false;
}
//----------------------------------------------------------------------
//	Get a roof texture for generation 
//----------------------------------------------------------------------
void D2_Style::SelectRoofTexture()
{ if (sText)				return;
	sText		= rtexQ.GetFirst();
	if (0 == sText)		return;
	if (sText->ReachQuota())	rtexQ.SwitchToLast(sText);
	return;
}
//----------------------------------------------------------------------
//	Get a roof texture for generation 
//----------------------------------------------------------------------
D2_TParam *D2_Style::GetRoofTexture()
{ SelectRoofTexture();
	//---- One roof texture selected ----
	D2_TParam *rft = sText;
	sText	= 0;
	return rft;
}
//----------------------------------------------------------------------
//	Get a roof texture for generation 
//----------------------------------------------------------------------
D2_TParam *D2_Style::SelectRoofNum(U_INT rfno)
{	U_INT dim = roofT.size();
  if (0 == dim)	gtfo("D2_TParam error");
  if (rfno >= dim)	rfno = dim - 1;
	sText = roofT[rfno];
	return sText;
}
//----------------------------------------------------------------------
//	Compute the S,T texture coordinates for point pp
//	1) Select texture definition in matrix from face type of pp
//	2) Compute S and T coordinates
//	hb indicates that the full batiment height is taken in account
//		in texturing the wall
//	ft => Face Type
//	fx => Floor index
//----------------------------------------------------------------------
void D2_Style::TexturePoint(D2_POINT *pp, char ft, char fx, char hb)
{ //--- Load texture --------------------------------------
  char	 ind = (ft << 2);					// Face Type as index in param
  ind += fx;											// Add floor code
	//---- Select the texture -------------------------------
	D2_TParam  *T = param[ind];				// Related Texture
	double      H = pp->z;						// Point height
	double      V = pp->VertPos();		// Vertical position
	double      W = (hb)?(bpm->hgt):(Wz);		// Wall height
	//--- pr is pixel in rectangle ---------------------------
	double  pr= 0;
	//---- Compute S coordinate -----------------------------
	switch (ft)
	{	case GEO_FACE_XP:
			pr		= (T->Rx * pp->LocalX()) / bpm->lgx;								// Building X
			pp->s = (T->x0 + pr) / T->Tw;
			break;
		case GEO_FACE_YP:
		  pr		= (T->Rx * pp->LocalY()) / bpm->lgy;								// Building Y
			pp->s = (T->x0 + pr) / T->Tw;
			break;
		case GEO_FACE_XM:
			pr		= T->Rx - ((T->Rx * pp->LocalX())  / bpm->lgx);			// Building X
			pp->s = (T->x0 + pr) / T->Tw;
			break;
		case GEO_FACE_YM:
			pr		= T->Rx - ((T->Rx * pp->LocalY())  / bpm->lgy);			// Building Y;
			pp->s = (T->x0 + pr) / T->Tw;
			break;
	}
	//---- Compute T coordinate -----------------------------
	if (T->IsWhole())
		{	//--- Pixel in rectangle -------------------
			pr		= (H  * T->Ry)	/ W;							// Pixel in rectangle
			pr		= (pr + double(T->y0))	/ Th;			// Pixel in texture		
			pp->t =  pr;
		}
	else 
		{ pp->t = (double(T->y0) + (V * T->Ry)) / Th;  }
	//-------------------------------------------------------
	if (!group->HasTrace())			return;
	char np[6];
	pp->Id(np);
	return;
}
//----------------------------------------------------------------------
//	Return texture mode
//	1 => Texture by face if requested and 
//	building extension is greater than coverage
//----------------------------------------------------------------------
char D2_Style::TextureMode()
{	if	(0 == texf)		return 0;
	double cv = FN_FEET_FROM_METER(cover);
	double xt = bpm->lgx;
	return (cv > xt)?(0):(1);
}
//----------------------------------------------------------------------
//	Compute the S,T texture coordinates for point pp
//	1) Select texture definition in matrix from face type of pp
//	2) Compute S and T coordinates
//	hb indicates that the full batiment height is taken in account
//		in texturing the wall
//	ft => Face Type
//	fx => Floor index
//----------------------------------------------------------------------
void D2_Style::TextureByFace(D2_POINT *pp, char ft, char fx, char hb)
{ //--- Load texture --------------------------------------
  char	 ind = (ft << 2);					// Face Type as index in param
  ind += fx;											// Add floor code
	//---- Select the texture -------------------------------
	D2_TParam  *T = param[ind];				// Related Texture
	double      H = pp->z;						// Point height
	double      V = pp->VertPos();		// Vertical position
	//double      W = (hb)?(bpm->hgt):(Wz);		// Wall height
	double      W = Wz;								// Wall height
	//--- pr => pixel in rectangle ---------------------------
	double  pr= 0;
	//---- Compute S coordinate ------------------------------
	pp->s			= T->x0 / T->Tw;
	D2_POINT *np = pp->next;
	//--- Assume texture is 30 meters ------------------------
	double	cv = FN_FEET_FROM_METER(cover);
	double  lg = pp->elg;
	double  vs = (lg / cv);
	if (vs > 1)	vs = 1;
	pr				 = (T->Rx * vs);
	if (np) np->s			 = (T->x0 + pr ) / T->Tw;
	//---- Compute T coordinate -----------------------------
	double vt = (double(T->y0) + (V * T->Ry)) / Th;  
	pp->t	= vt;
	if (np) np->t = vt;
	//-------------------------------------------------------
	if (!group->HasTrace())			return;
	char id[6];
	pp->Id(id);
	return;
}

//----------------------------------------------------------------------
//	Texture Triangle wall according to face type ft
//----------------------------------------------------------------------
void D2_Style::TextureSideWall(D2_TRIANGLE &T, char ft, char hb)
{	TexturePoint(T.B,ft,TEXD2_WHOLE,hb);
	TexturePoint(T.A,ft,TEXD2_WHOLE,hb);
	TexturePoint(T.C,ft,TEXD2_WHOLE,hb);
	return;
}
//===================================================================================
//	Create a texture parameter
//===================================================================================
D2_TParam::D2_TParam()
{	strncpy(idn,"D2TP",4);
	strcpy(face,"NO");
	next = prev = 0;
	hmod	= TEXD2_HMOD_WHOLE;
	Tw		= 1;
	Th		= 1;
	Rx		= 1;
	Ry		= 1;
	sty	  = 0;
	rfnb  = 0;
}
//-----------------------------------------------------------------
//	Destroy texture parameter 
//-----------------------------------------------------------------
D2_TParam::~D2_TParam()
{	}

//-----------------------------------------------------------------
//	Set default roof texture 
//-----------------------------------------------------------------
void D2_TParam::DefaultParameters(TEXT_INFO &inf)
{	strncpy(face,"ROOF",4);
	Tw	= inf.wd;
	Th	= inf.ht;
	//--- Compute corners -----------------------
	x0  = 0;
	y0  = Th - 5;
	//-------------------------------------------
	Rx	= 4;
	Ry	= 4;
	return;
}
//-----------------------------------------------------------------
//	Save style and building parameters
//-----------------------------------------------------------------
void	D2_TParam::	SetStyle(D2_Style *st)
{	sty	 = st;
	bpm	 = sty->GetBPM();
	return;
}
//-----------------------------------------------------------------
//	Copy from parameters and compute some values 
//-----------------------------------------------------------------
void	D2_TParam::AdjustFrom(D2_TParam &p, D2_Style *st, U_INT tw, U_INT th)
{	*this	= p;
	 sty	= sty;
	 Tw		= tw;
	 Th		= th;
	 return;
}
//-----------------------------------------------------------------
//	Texture Triangle with the given corner
//-----------------------------------------------------------------
void D2_TParam::TextureRoofTriangle(D2_TRIANGLE &T)
{	TextureXRoofPoint(T.B);
	TextureXRoofPoint(T.A);
	TextureXRoofPoint(T.C);
	return;
}
//-----------------------------------------------------------------
//	Texture SW Dormer Triangle with the given corner
//-----------------------------------------------------------------
void D2_TParam::TextureDormerTriangle(D2_TRIANGLE &T, char type)
{	TextureXRoofPoint(T.B);
	TextureDRoofPoint(T.A);
	if (type == PAN_DORM_SW)	TextureDRoofPoint(T.C);
	else											TextureXRoofPoint(T.C);
	return;
}
//-----------------------------------------------------------------
//	Texture Front Dormer Triangle with the given corner
//-----------------------------------------------------------------
void D2_TParam::TextureFrontTriangle(D2_TRIANGLE &T)
{	TextureFRoofPoint(T.B);
	TextureFRoofPoint(T.A);
	TextureFRoofPoint(T.C);
	return;
}
//-----------------------------------------------------------------
//	Texture point against local coordinates
//-----------------------------------------------------------------
void	D2_TParam::TextureLocalPoint(D2_POINT *p)
{	double px = (p->lx * Rx) / bpm->lgx;					// Pixel in rectangle
	p->s			= (x0 + px)		 / Tw;								// Pixel in texture
	double py = (p->ly * Ry) / bpm->lgy;					// Pixel in rectangle
	p->t			= (y0 + py)		 / Th;								// Pixel in rectangle
	return;
}
//-----------------------------------------------------------------
//	Texture roof point against local coordinates
//	NOTE: This is a side projection of roof texture on roof pan
//-----------------------------------------------------------------
void	D2_TParam::TextureXRoofPoint(D2_POINT *p)
{	double px = (p->lx * Rx) / bpm->lgx;					// Pixel in rectangle
	p->s			= (x0 + px)		 / Tw;								// Pixel in texture
	//--- Compute relative point height -------------------------
	double pz = ((p->z - rofB) * Ry) / rofH;			// Pixel in rectangle
	p->t			= (y0 + pz)		 / Th;								// Pixel in rectangle
	return;
}
//-----------------------------------------------------------------
//	Texture base dormer point against local coordinates
//	NOTE: This is a side projection of roof texture on roof pan
//-----------------------------------------------------------------
void	D2_TParam::TextureDRoofPoint(D2_POINT *p)
{	double px = (p->lx * Rx) / bpm->lgx;					// Pixel in rectangle
	p->s			= (x0 + px)		 / Tw;								// Pixel in texture
	//--- Relative point height is base roof -------------------------
	double pz = 0;																// Pixel in rectangle
	p->t			= (y0 + pz)		 / Th;								// Pixel in rectangle
	return;
}

//-----------------------------------------------------------------
//	Texture roof point against local coordinates
//	NOTE: This is a side projection of roof texture on roof pan
//-----------------------------------------------------------------
void	D2_TParam::TextureYRoofPoint(D2_POINT *p)
{	double py = (p->ly * Rx) / rofW;							// Pixel in rectangle
	p->s			= (x0 + py)		 / Tw;								// Pixel in texture
	//--- Compute relative point height -------------------------
	double pz = ((p->z - rofB) * Ry) / rofH;			// Pixel in rectangle
	p->t			= (y0 + pz)		 / Th;								// Pixel in rectangle
	return;
}
//-----------------------------------------------------------------
//	Texture roof point against local coordinates
//	NOTE: This is a side projection of roof texture on roof pan
//-----------------------------------------------------------------
void	D2_TParam::TextureFRoofPoint(D2_POINT *p)
{	double px = p->s * Rx;												// Pixel in rectangle
	p->s			= (x0 + px)		 / Tw;								// Pixel in texture
	//--- Compute relative point height -------------------------
	double pz = p->t * Ry;												// Pixel in rectangle
	p->t			= (y0 + pz)		 / Th;								// Pixel in rectangle
	return;
}
//-----------------------------------------------------------------
//	Compute texture coordinates for the side roof in a 4 slope roof
//-----------------------------------------------------------------
void D2_TParam::TextureSideRoof(D2_TRIANGLE &T)
{	TextureYRoofPoint(T.B);
	TextureYRoofPoint(T.A);
	TextureYRoofPoint(T.C);
}

//======================= END OF FILE =========================================================