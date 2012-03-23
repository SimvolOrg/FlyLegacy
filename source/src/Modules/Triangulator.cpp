//====================================================================================
// Triangulator.cpp
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
#include "../Include/RoofModels.h"
#include "../Include/TerrainTexture.h"
#include "../Include/fileparser.h"
#include <math.h>
//===================================================================================
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
TC_VTAB *D2_POINT::SaveVertex(TC_VTAB *tab)
{	tab->VT_X = x;
	tab->VT_Y = y;
	tab->VT_Z = z;
	tab->VT_S = s;
	tab->VT_T = t;
	return (tab + 1);
}
//====================================================================================
//	TRIANGLE routines
//	
//====================================================================================
//	Store vertices in part
//---------------------------------------------------------------------
U_INT D2_TRIANGLE::StoreVertices(C3DPart *p, U_INT n)
{ TC_VTAB *tab = p->GetVTAB() + n;
  tab	= B->SaveVertex(tab);
	tab = A->SaveVertex(tab);
	tab = C->SaveVertex(tab);
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
Triangulator::Triangulator(D2_Session *s)
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
Triangulator::~Triangulator()
{	Clean();
	if (remB)		delete remB;
	//--- clear building list ---------------------------
	//	Individual buildings are deleted from D2_Group
	globals->Disp.Clear(PRIO_ABSOLUTE);
}
//-------------------------------------------------------------------
//	Time Slice 
//-------------------------------------------------------------------
int Triangulator::TimeSlice(float dT,U_INT frame)
{
	return 0;
}
//===================================================================
//	Drawing interface
//===================================================================
//-------------------------------------------------------------------
//	Draw all groups 
//-------------------------------------------------------------------
void Triangulator::DrawMarks()
{	std::map<std::string,D2_Group*> grp = session->GetGroups();
	std::map<std::string,D2_Group*>::iterator rp;

	for (rp = grp.begin(); rp != grp.end(); rp++)
	{	D2_Group *gpp = (*rp).second;
		gpp->DrawBuilding();	}
	return;
}
//-------------------------------------------------------------------
//	Draw Interface 
//-------------------------------------------------------------------
void Triangulator::Draw()
{	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glColor3f(255,255,255);
	//--- Set client state  ------------------------------------------
	glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glFrontFace(GL_CCW);
	//----------------------------------------------------------------
  char d1 = 0;
	if (dop.Has(TRITOR_DRAW_FILL))	FillMode();
	else														LineMode();
	//--- Draw current mode ------------------------------------------
	if (dMOD == 1)	DrawMarks();
	else	
	if (osmB)	osmB->DrawLocal();
	//--- Reset attributes -------------------------------------------
	glPopClientAttrib();
	glPopAttrib();
	return;
}
//-------------------------------------------------------------------
//	Draw as lines 
//-------------------------------------------------------------------
int Triangulator::LineMode()
{	// Draw triangulation
	glDisable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT,GL_LINE);
	return 1;
}
//-------------------------------------------------------------------
//	Draw as color 
//-------------------------------------------------------------------
int Triangulator::FillMode()
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
void Triangulator::AddVertex(double x, double y)
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
	return;
}
//-------------------------------------------------------------------
//	Hole annunciator
//-------------------------------------------------------------------
void Triangulator::NewHole()
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
void Triangulator::Clean()
{	extp.Clear();
	hole.Clear();
	slot.Clear();
	for (U_INT k=0; k < walls.size(); k++)	delete walls[k];
	walls.clear();
	for (U_INT k=0; k < roof.size();  k++)	delete roof[k];
	roof.clear();
	//--- delete bevel point array ------------------
	if (bevel)	delete [] bevel;
	//--- Delete current building if left ----------
	osmB			= 0;
	bevel			= 0;
	seq				= 0;										// Vertex sequence
	num				= 0;										// Null number
	vRFX			= 0;
	hIND			= 'X';
	BDP.Clear();
	//BDP.roofM = session->GetDefaultRoof();
	TO				= 0;
  //--- Reset all parameters ----------------------
	return;
}
//-------------------------------------------------------------------
//	Check every thing for debugging
//-------------------------------------------------------------------
void Triangulator::CheckAll()
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
void Triangulator::DrawSingle()
{	dMOD = 0;
	if (0 == osmB)	return;
	osmB->Deselect();
	BDP.selc	= 0;
	return;
}

//-------------------------------------------------------------------
//	Draw all 
//-------------------------------------------------------------------
void Triangulator::DrawGroups()
{	dMOD = 1;
	if (0 == osmB)	return;
	osmB->Deselect();
	BDP.selc = 0;
}
//-------------------------------------------------------------------
//	Start a new object 
//-------------------------------------------------------------------
void Triangulator::StartOBJ()
{	Clean();
	return;
}
//-------------------------------------------------------------------
//	Rebuild if orientation error 
//-------------------------------------------------------------------
void Triangulator::ReOrientation()
{	if (BDP.error != 4)		return;
	slot.Clear();
	//------------------------------------------
	Queue<D2_POINT> H;
	extp.TransferQ(H);
	D2_POINT *pp;
	for (pp = H.Pop(); pp != 0; pp = H.Pop()) 
		{ extp.PutHead(pp); 
	    pp->Reset();
	  }
	BDP.error = 0;
	BDP.error = QualifyPoints();
	return;
}
//-------------------------------------------------------------------
//	Process object 
//-------------------------------------------------------------------
D2_BPM *Triangulator::BuildOBJ(U_INT tp)
{	otype			= tp;
	BDP.side	= extp.GetNbObj();
	BDP.error = 0;
	BDP.opt.Rep(0);								// No property yet
	BDP.error = ConvertInFeet();
	BDP.error = QualifyPoints();
	if (BDP.error)		ReOrientation();
	BDP.dlg		= dlg;
	//--- Set generation parameters ---------
	session->GetBuildParameters(&BDP);
	D2_Style *sty = BDP.style;
	D2_Group *grp = BDP.group;
	BDP.style->AssignBPM(&BDP);
	BDP.roofP		= session->GetRoofTexture(sty);
	BDP.roofM		= grp->GetRoofModByNumber(BDP);
	MakeBuilding();
	//--------------------------------------
	return &BDP;
}
//-------------------------------------------------------------------
//	Add tag
//-------------------------------------------------------------------
void Triangulator::SetTag(char *t, char *v)
{	if (0 == osmB)	return;
	osmB->SetTag(t,v);
}
//-------------------------------------------------------------------
//	Edit tag
//-------------------------------------------------------------------
void Triangulator::EditTag(char *txt)
{	*txt = 0;
	if (0 == osmB)		return;
	osmB->EditTag(txt);
	return;
}
//-------------------------------------------------------------------
//  Build the building
//-------------------------------------------------------------------
bool Triangulator::MakeBuilding()
{	dlg	= BDP.dlg;
	//--- Step 1: Create OSM object -----
	CreateBuilding();
	//--- Step 2:  Triangulation --------
	BDP.error = Triangulation();
	//--- Step 3: Face orientation ----- 
	OrientFaces();
	//--- Step 4: Build walls -----------
	BuildWalls();
	//--- Step 5: Select roof model -----
	SelectRoof();
	//--- Step 6: Texturing -------------
	Texturing();
	//--- Step 7:  Save parameters ------
	SaveBuildingData();
	return true;
}
//-------------------------------------------------------------------
//	Replace by a 3D object
//	
//-------------------------------------------------------------------
U_CHAR Triangulator::ReplaceBy(char *fn, char *dir, double rad)
{	if (0 == osmB)			return 0;
	if (0 == *fn)				return 0;
  COBJparser obj(OSM_OBJECT);
	obj.SetDirectory(dir);
  obj.Decode(fn,OSM_OBJECT);
	C3DPart *prt = obj.GetOnlyFirstPart();
	if (0 == prt)				return 0;
	osmB->Replace(fn,prt);
	osmB->bpm.opt.Set(D2B_REPLACED);
	osmB->orien = rad;
	return osmB->Rotate();
}
//-------------------------------------------------------------------
//	End of object 
//-------------------------------------------------------------------
void Triangulator::EndOBJ()
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
U_INT Triangulator::CountVertices()
{	U_INT nvtx = 3 * roof.size();
	for (U_INT k=0; k < walls.size(); k++) nvtx += walls[k]->VerticesNumber();
	return nvtx;
}
//-------------------------------------------------------------------
//	Set Style from name
//-------------------------------------------------------------------
void Triangulator::ForceStyle(char *nsty, U_INT rfmo, U_INT rftx)
{	D2_Style *sty = session->GetStyle(nsty);
	BDP.style = sty;
	if (0 == sty)		return;
	D2_Group *grp = sty->GetGroup();
	sty->SelectRoofNum(rftx);
	grp->SetRoofModelNumber(rfmo);
	//--- Set all parameters -------------------
	BDP.group		= grp;
	BDP.flNbr		= grp->GetFloorNbr();
	BDP.flHtr		= grp->GetFloorHtr();
	BDP.mans		= sty->IsMansart();
	BDP.roofM		= 0;
	return;
}
//-------------------------------------------------------------------
//	Create OSM object 
//	NOTE: If we have to modify the vertices then the call
//				to this function must be removed from BuildOBJ and
//				deferred until Object is OK
//-------------------------------------------------------------------
void Triangulator::CreateBuilding()
{ OSM_Object  *obj	= osmB;
	if (0 == obj) obj =	new OSM_Object(otype);
  osmB							= obj;
	GetSuperTileNo(&BDP.geop, &BDP.qgKey, &BDP.supNo);
	obj->SetParameters(BDP);
	//--- Save a copy of the base points -----------
	osmB->ReceiveQ(extp);
	return;
}
//-------------------------------------------------------------------
//	Save all data in building
//-------------------------------------------------------------------
void Triangulator::SaveBuildingData()
{	U_INT nvtx		= CountVertices();
  C3DPart *prt	= osmB->GetPart();
	prt->AllocateOsmVTX(nvtx);
	//--- build all vertices ------------------------------
	U_INT n = 0;
	for (U_INT k=0; k < walls.size(); k++) n = walls[k]->StoreVertice(prt,n);
	for (U_INT k=0; k < roof.size() ; k++) n = roof[k]->StoreVertices(prt,n);
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
//	Modify the style
//-----------------------------------------------------------------------
U_CHAR Triangulator::ModifyStyle()
{	//--- deselect previous selection------------------
	if (0 == osmB)		return 0;
	BDP.opt.Raz(D2B_REPLACED);
	osmB->SetParameters(BDP);
	//---- Restore foot print -------------------------
	osmB->Swap(extp);
	MakeSlot();
	MakeBuilding();
	return 1;
}
//-----------------------------------------------------------------------
//	Select one building
//-----------------------------------------------------------------------
D2_BPM *Triangulator::SelectBuilding(U_INT No)
{	//--- Deselect previous selection------------------
	char same = (BDP.stamp == No);
	char bldg = (osmB != 0);
	char sbld = (same & bldg);
	if (sbld)	osmB->SwapSelect();
	//--- see for new building ------------------------
	else
	{	if (bldg)	osmB->Deselect();
		//--- Get the new Building ------------------------
		OSM_Object *bld = session->GetBuilding(No);
		//--- Establish the new building ------------------
		osmB		= bld;
		osmB->Select();
	}
	//---- Restore Parameters -------------------------
	BDP		= osmB->GetParameters();
	return &BDP;
}

//-----------------------------------------------------------------------
//	Delete the current building
//-----------------------------------------------------------------------
int  Triangulator::RemoveBuilding()
{	if (0 == osmB)		return 0;
	if (remB)					delete remB;
	remB		= osmB;
	BDP.group->RemBuilding(remB);
	osmB		= 0;
	return 1;
}
//-----------------------------------------------------------------------
//	Restore the last deleted building
//-----------------------------------------------------------------------
D2_BPM *Triangulator::RestoreBuilding(U_INT *cnt)
{	if (0 == remB)			return &BDP;
	BDP		= remB->GetParameters();
	BDP.group->AddBuilding(remB);
	if (osmB)	osmB->Deselect();
	osmB	= remB;
	osmB->Select();
	BDP.selc	= 1;
	remB			= 0;
	*cnt++;
	return &BDP;
}
//----------------------------------------------------------------
//	Rotate the building
//	NOTE: Only the vertice are rotated.  The building original
//			orientationmust be kept unchanged
//----------------------------------------------------------------
U_CHAR Triangulator::RotateObject(double rad)
{	if (0 == osmB)													return 0;
	//--- Set building orientation --------------------
	if (!osmB->bpm.opt.Has(D2B_REPLACED))	  return 0;
	//------------------------------------------------
	return osmB->IncOrientation(rad);
}
//-------------------------------------------------------------------
//	Convert Coordinates in feet relative to object center
//-------------------------------------------------------------------
char Triangulator::ConvertInFeet()
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
		pp->z		= 0;
	}
	return 0;
}
//-------------------------------------------------------------------
//	Clear any slot left
//-------------------------------------------------------------------
void Triangulator::ReleaseSlot()
{	D2_SLOT *sp;
	for (sp = slot.Pop(); sp != 0; sp = slot.Pop())
	{	delete sp; 	}
	return;
}
//-------------------------------------------------------------------
//	Recreate slot from foot print
//-------------------------------------------------------------------
void Triangulator::MakeSlot()
{ ReleaseSlot();
	D2_POINT *pp;
	for (pp = extp.GetFirst(); pp != 0; pp = pp->next)
	{	D2_SLOT *sp = new D2_SLOT(pp);
		slot.PutLast(sp);
	}
	return;
}
//-------------------------------------------------------------------
//	Compute type of points (reflex or convex)
//	Create slot for triangulation
//	Compute ground surface
//-------------------------------------------------------------------
char Triangulator::QualifyPoints()
{	int nbp = extp.GetNbObj();
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
	//--- Adjust real surface- ---------------
	if (surf > 0)		return 4;
	BDP.surf = -0.5 * surf;
	//--- Reorder with TO as origin point ----
	Reorder();
	//--- Chose  precision -------------------
	if (trace) TraceInp();
	return  (dlg < geo.GetPrecision())?(2):(0);
}
//-------------------------------------------------------------------
//	Compute type of points at triangle extremity
//-------------------------------------------------------------------
void Triangulator::Requalify(D2_SLOT *sa)
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
bool Triangulator::GetAnEar()
{ 
	D2_SLOT  *sa = 0;
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
bool Triangulator::NotAnEar(D2_SLOT *sa)
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
char Triangulator::Triangulation()
{	roof.reserve(slot.GetNbObj() - 1);
	while (slot.GetNbObj() != 2)	if (!GetAnEar()) 	return 3;
	slot.Clear();
	if (trace) TraceOut();
	return 0;
}

//-------------------------------------------------------------------
//	Trace Input
//-------------------------------------------------------------------
void Triangulator::TraceInp()
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
void Triangulator::TraceFace()
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
void Triangulator::TraceOut()
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
void Triangulator::Merge()
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
D2_POINT *Triangulator::MatchHole(D2_POINT *xp)
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
bool Triangulator::CheckExternal(D2_POINT *xp, D2_POINT *hp)
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
bool Triangulator::CheckInternal(D2_POINT *xp, D2_POINT *hp)
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
void Triangulator::Splice(D2_POINT *xp, D2_POINT *hp)
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
void Triangulator::OrientFaces()
{	TO	= extp.GetFirst();
	D2_Style *sty = BDP.style;
	//--- Compute rotation matrix -------------------
  D2_POINT *pa = TO;
	D2_POINT *pb = extp.CyNext(pa);
  BDP.cosA = (pb->x - pa->x) / dlg;					// Cos(A)
	BDP.sinA = (pb->y - pa->y) / dlg;					// Sin(A)
	//--- As we want a -A rotation, we change the signe of sin(A) aka dy
	NegativeROT(BDP.sinA,BDP.cosA);
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
	//--- Compute Building extensions ---------------------
	Xp	= maxx - minx;
	Yp	= maxy - miny;
	BDP.lgy = Yp;
	BDP.lgx = Xp;
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
	mat[0]	= +BDP.cosA;
	mat[1]	= -BDP.sinA;
	mat[2]	= 0;
	///------------------------
	mat[3]	= +BDP.sinA;
	mat[4]	= +BDP.cosA;
	mat[5]	= 0;
	//-------------------------
	mat[6]	= (tx * BDP.cosA) + (BDP.sinA * ty) - minx;
	mat[7]	= (ty * BDP.cosA) - (BDP.sinA * tx) - miny;
	mat[8]	= 1;
	//-----------------------------------------------------
	if (trace) TraceFace();
	return;
}
//----------------------------------------------------------------
//	Global transform
//----------------------------------------------------------------
void Triangulator::LocalCoordinates(D2_POINT &p)
{	gx = (p.x * mat[0]) + (p.y * mat[3]) + mat[6];
	gy = (p.x * mat[1]) + (p.y * mat[4]) + mat[7];
	p.lx = gx;
	p.ly = gy;
	return;
}
//----------------------------------------------------------------
//	Init a Negative Rotation
//----------------------------------------------------------------
void Triangulator::NegativeROT(double sn, double cs)
{	//--- As we want a -A rotation, we change the signe of sin(A)
	rotM[0]	= +cs; rotM[1] = -sn;
	rotM[2] = +sn; rotM[3] = +cs;
	return;
}
//----------------------------------------------------------------
//	Init a Positive Rotation
//----------------------------------------------------------------
void Triangulator::PositiveROT(double sn, double cs)
{	//--- As we want a +A rotation, we keep the normal way
	rotM[0]	= +cs; rotM[1] = +sn;
	rotM[2] = -sn; rotM[3] = +cs;
	return;
}

//----------------------------------------------------------------
//	Rotation
//----------------------------------------------------------------
void Triangulator::Rotate(double x, double y)
{	rx = ((x * rotM[0]) + (y * rotM[2]));
	ry = ((x * rotM[1]) + (y * rotM[3]));
}
//----------------------------------------------------------------
//	Transform a local coordinate to the world coordinate
//----------------------------------------------------------------
void Triangulator::Rotate(TC_VTAB &v, double sn, double cn)
{	double x = v.VT_X;
	double y = v.VT_Y;
	PositiveROT(sn, cn);
	Rotate(x,y);
	v.VT_X = rx;
	v.VT_Y = ry;
	return;
}
//----------------------------------------------------------------
//	Qualify an edge with previous point
//	We qualify the edge pa which is the previous point of pb
//	because pa is already computed.
//----------------------------------------------------------------
void Triangulator::QualifyEdge(D2_POINT *pb)
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
void Triangulator::BuildWalls()
{	Wz = 0;
  double H = BDP.flHtr;
	for (int k=1; k <= BDP.flNbr; k++)
	{	BuildFloor(k,Wz,H);
		Wz += H;
	}
	return;
}
//----------------------------------------------------------------
//	Trace Bevel point
//----------------------------------------------------------------
void Triangulator::TraceBevel(D2_POINT *p)
{	TRACE("BEVEL  B%05d rng=%04d Xp=%.4lf Yp=%.4lf P.x=%.4lf P.y=%.4lf P.z=%.4lf lx=%.4lf ly=%.4lf",BDP.stamp,p->rng, Xp,Yp,p->x, p->y, p->z,p->lx,p->ly);
}
//----------------------------------------------------------------
//	Check that bevel point is inside the building, at good height
//----------------------------------------------------------------
bool Triangulator::SetPointInside(D2_POINT *p, D2_POINT *s, double H)
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
void Triangulator::GetBevelVector(D2_POINT *pa, double dy,D2_POINT *P)
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
int Triangulator::SetBevelArray(int d,D2_BEVEL &pm)
{ //--- Bevel distance ------------------------
	double dy = fabs(pm.H / pm.tang);
	D2_POINT *dst = bevel + d;
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
D2_POINT *Triangulator::AllocateBevel(int nb)
{	bevel = new D2_POINT[nb];
	return bevel;
}
//----------------------------------------------------------------
//	Translate a point (this is horizontal rotation only)
//	Tx,Ty,Tz define the local translation vector
//	It must be rotated back to world coordinates before to be
//	added to the POINT
//----------------------------------------------------------------
void Triangulator::TranslatePoint(D2_POINT &p, double tx, double ty, double tz)
{	//---- Adjust local components ----------------
	p.lx	+= tx;
	p.ly	+= ty;
	p.z   += tz;
	//---- Compute real translation vector -------
	PositiveROT(BDP.sinA, BDP.cosA);
	Rotate(tx, ty);
	p.x		+= rx;
	p.y		+= ry;
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
int Triangulator::BuildBevelFloor(int No, int inx, double afh, double H)
{	D2_Style *sty = BDP.style;
	D2_BEVEL pm;
	U_INT nbp		= extp.GetNbObj();
	pm.tang			= t70;
	pm.pah			= afh;
	pm.H				= H;
	bevel		    = new D2_POINT[nbp];
	SetBevelArray(0,pm);
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
		//--- Set Face top position ------
		fa->ne.V	= 1;
		fa->nw.V	= 1;
		//----------------------------------------------
		et->faces.push_back(fa);
	}
	sty->SetWz(ce);
	BDP.hgt = ce;				//sty->SetBz(ce);
  walls.push_back(et);
	//--- Change foot print points to the bevel plan--
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
//	return corresponding bevel point
//----------------------------------------------------------------
D2_POINT *Triangulator::GetBevelPoint(D2_POINT *p)
{	return bevel + p->rng;	}
//----------------------------------------------------------------
//	Build a normal floor
//----------------------------------------------------------------
int	Triangulator::BuildNormaFloor(int No,int inx, double afh, double H)
{	double    ce = afh + H;								// Ceil height
  D2_POINT *pa = 0;
	D2_FLOOR *et = new D2_FLOOR(No,TEXD2_FLOORZ,afh,ce);
	D2_FACE  *fa = 0;
	for (pa = extp.GetFirst(); pa != 0; pa = pa->next)
	{	fa = new D2_FACE(pa->FaceType(),inx);
		//--- Now we build a face with pa as the base points
		D2_POINT *pb = extp.CyNext(pa);
		fa->Extrude(afh,ce,pa,pb);
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
//	fh = absolute floor height
//	ce = ceil height
//	We also adjust the roof height in (extp)
//----------------------------------------------------------------
void Triangulator::BuildFloor(int No, double afh, double H)
{	//--- Compute face code -----------------------------
	D2_Style *sty = BDP.style;
	int  flNbr		= BDP.flNbr;
	double ce = 0;
	char indx = TEXD2_FLOORM;								// For middle floor
	if (flNbr == No)	indx = TEXD2_FLOORZ;	// Last floor
	if (1			== No)	indx = TEXD2_FLOOR1;	// ground floor
	bool mans = (indx == TEXD2_FLOORZ) && sty->IsMansart();
	if (mans)	ce	=		BuildBevelFloor(No,indx,afh,H);
	else			ce	=		BuildNormaFloor(No,indx,afh,H);
	sty->SetWz(ce);
	BDP.hgt = ce;			//sty->SetBz(ce);
	return;
}
//----------------------------------------------------------------
//	Build fence
//----------------------------------------------------------------

//----------------------------------------------------------------
//	Reorder the tour with origin as first point
//----------------------------------------------------------------
void Triangulator::Reorder()
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
//	Change Point
//----------------------------------------------------------------
D2_POINT *Triangulator::ChangePoint(D2_POINT *pp)
{	return bevel + pp->rng; }
//----------------------------------------------------------------
//	Empty actual roof
//----------------------------------------------------------------
void Triangulator::ClearRoof()
{	//--- Delete current roof -----------------------------
	for (U_INT k=0; k < roof.size(); k++) delete roof[k];
	roof.clear();
	return;
}
//----------------------------------------------------------------
//	Select roof model
//----------------------------------------------------------------
void Triangulator::SelectRoof()
{	if (BDP.roofM->GetClearIndicator())	ClearRoof();
	//--- Assign roof parameters---------------------
	BDP.roofM->SetRoofData(&BDP, this);
	BDP.roofM->BuildRoof(extp,roof);
	Zp			= BDP.roofM->GetTop();
	return;
}
//----------------------------------------------------------------
//	Texturing pass
//----------------------------------------------------------------
void Triangulator::Texturing()
{	if (0 == BDP.style)		return;
	//------------------------------------------------- 
	for (U_INT k = 0; k < walls.size(); k++) 
	{	D2_FLOOR *fl = walls[k];
		fl->TextureFloor(BDP.style);
	}
	return;
}

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
	//--- Set Face top position ------
	ne.V	= 1;
	nw.V	= 1;
	//--- Save heights ----------------
	sw.z = se.z = fh;
	ne.z = nw.z = ch;
	return;
}
//---------------------------------------------------------------
//	Texture this face
//	Y faces are texture as a whole or as floor 
//---------------------------------------------------------------
void D2_FACE::TextureFace(D2_Style *sty)
{	char hb = (fType & GEO_Y_FACE)?(1):(0);
  sty->TexturePoint(&sw,fType,fIndx,hb);
	sty->TexturePoint(&se,fType,fIndx,hb);
	sty->TexturePoint(&ne,fType,fIndx,hb);
	sty->TexturePoint(&nw,fType,fIndx,hb);
	//--- check for roof complement ---------------
	
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
U_INT	D2_FACE::StoreVertices(C3DPart *prt, U_INT n)
{	TC_VTAB *tab = prt->GetVTAB() + n;
	tab = sw.SaveVertex(tab);
	tab = se.SaveVertex(tab);
	tab = nw.SaveVertex(tab);

	tab = nw.SaveVertex(tab);
	tab = se.SaveVertex(tab);
	tab = ne.SaveVertex(tab);

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
{ for (U_INT k=0; k < faces.size(); k++)
	{	D2_FACE *fa = faces[k];
		fa->TextureFace(s);
	}
	return;
}
//---------------------------------------------------------------
//	Save vertices in Part 
//---------------------------------------------------------------
U_INT	D2_FLOOR::StoreVertice(C3DPart *p,U_INT x)
{	U_INT n = x;
	for (U_INT k=0; k < faces.size(); k++) n += faces[k]->StoreVertices(p,n);
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
	*path			= 0;
	//----------------------------------------
	sbldg		  = 0;
	//----------------------------------------
	xOBJ			= 0;
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
	  char *pm = sn->ReadFile(f,buf);
		if (DecodeParam(pm))	continue;
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
	nf = sscanf(buf," LENGTH [ %lf, %lf ]", &lgMin, &lgMax);
	if (nf == 2)	{opt.Set(D2_GRP_LNGT);	return true; }
	//--- Check Side length to infinity ----------------------
	nf = sscanf(buf," LENGTH [ %lf, * ]", &lgMin);
	if (nf == 1)	{opt.Set(D2_GRP_LNGT);	return true; }
	//--- Check for surface --------------------------------
	nf = sscanf(buf," SURFACE [ %lf , %lf ]", &sfMin, &sfMax);
	if (nf == 2)	{opt.Set(D2_GRP_SURF);	return true; }
	//--- Check for surface to infinity --------------------
  nf = sscanf(buf," SURFACE [ %lf , * ]",  &sfMin);
	if (nf == 1)	{opt.Set(D2_GRP_SURF);	return true; }
	//--- Check for side ------------------------------------
	nf = sscanf(buf," SIDES [ %u , %u ]", &sdMin, &sdMax);
	if  (nf == 2) {opt.Set(D2_GRP_SIDE);	return true; }
	//--- Check for side to infinity ------------------------
	nf = sscanf(buf," SIDES [%u , * ]", &sdMin);
	if	(nf == 1) {opt.Set(D2_GRP_SIDE);	return true; }
	//--- Check floor number --------------------------------
	nf = sscanf(buf," FLOOR %u",   &flNbr);
	if  (1 == nf)	return true;
	//--- Check floor height --------------------------------
	nf = sscanf(buf," HEIGHT %lf", &flHtr);
	if	(1 == nf)	return true;
	//--- Check texture file -------------------------------
	nf = sscanf_s(buf," TEXTURE %64s",tex,64);
	if  (1 == nf)
	{	tex[63]	= 0;
		_snprintf(path,FNAM_MAX,"OpenStreet/Textures/%s",tex);
		return true;
	}
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
{	if (opt.Not(D2_GRP_SURF))		return 0;
	if (sf < sfMin)							return 0;
	if (sf > sfMax)							return 0;
	return	10;
}
//-----------------------------------------------------------------
//	Evaluate Side
//-----------------------------------------------------------------
int D2_Group::ValueSide(U_INT sd)
{	if (opt.Not(D2_GRP_SIDE))		return 0;
	if (sd < sdMin)							return 0;
	if (sd > sdMax)							return 0;
	return 10;
}
//-----------------------------------------------------------------
//	Evaluate Side
//-----------------------------------------------------------------
int D2_Group::ValueLength(double lg)
{ if (opt.Not(D2_GRP_LNGT))		return 0;
	if (lg < lgMin)							return 0;
	if (lg > lgMax)							return 0;
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
	val += ValuePosition(bpm->geop);
	val += ValueSurface (bpm->surf);
	val += ValueSide    (bpm->side);
	val += ValueLength  (bpm->lgx);
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
U_INT D2_Group::GetOneRoof(D2_BPM &bpm)
{	CRoofModel *rofm = roofsQ.GetFirst();
	//--- check roof model -------------------
	rmno	= 0;
  mans	= bpm.mans;
	if (0 == rofm)					return 0;
	if (bpm.mans)						return 0;
	if (bpm.side > 4)       return 0;
	if (rofm->ReachQuota())	roofsQ.SwitchToLast(rofm);
	rmno =  rofm->GetRoofModNumber();
	return rmno;
}
//-----------------------------------------------------------------
//	Get a roof by number 
//-----------------------------------------------------------------
CRoofModel *D2_Group::GetRoofModByNumber(D2_BPM &bpm)
{	CRoofModel *dfm = ssn->GetDefaultRoof();
	if (0 == rmno)					return dfm;
  if (0 == roofM.size())	return dfm;
	if (bpm.mans)						return dfm;
	if (bpm.side > 4)				return dfm;
	return roofM[rmno - 1];
}

//-----------------------------------------------------------------
//	Load texture for this group
//-----------------------------------------------------------------
void D2_Group::LoadTexture()
{	if (*path == 0)		return;
	if (xOBJ)		      return;
	tREF	= globals->txw->GetM3DPodTexture(path,1,1);
	globals->txw->GetTextureParameters(tREF,tinf);
	xOBJ	= globals->txw->Get3DObject(tREF);
	//--- Fill default roof parameters ---------------------
	tRoof.DefaultParameters(tinf);
	return;
}
//-----------------------------------------------------------------
//	Draw this group
//-----------------------------------------------------------------
void D2_Group::DrawBuilding()
{	std::map<U_INT,OSM_Object*>::iterator rp;
	glBindTexture(GL_TEXTURE_2D,xOBJ);
	for (rp = building.begin(); rp != building.end(); rp++)
	{	OSM_Object *bld = (*rp).second;
		if (bld != sbldg)	bld->Draw();
		if (bld->Skip())	continue;
		bld->Draw();
	}
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
	tp->SetRoofTexNumber(roofT.size() + 1);
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
void D2_Style::SelectRoofNum(U_INT rfno)
{	if (rfno < roofT.size())	sText = roofT[rfno];
	else											sText = 0;
	return;
}
//----------------------------------------------------------------------
//	Compute the S,T texture coordinates for point pp
//	1) Select texture definition in matrix from face type of pp
//	2) Compute S and T coordinates
//	hb indicates that the full batiment height is taken in account
//		in texturing the wall
//----------------------------------------------------------------------
void D2_Style::TexturePoint(D2_POINT *pp, char ft, char fx, char hb)
{ //--- Load texture --------------------------------------
  char	 ind = (ft << 2);					// Face Type as index in param
  ind += fx;											// Add floor code
	//---- Select the texture -------------------------------
	D2_TParam  *T = param[ind];				// Related exture
	double      H = pp->z;						// Point height
	double      V = pp->VertPos();		// Vertical position
	double      W = (hb)?(bpm->hgt):(Wz);		// Wall height
	//--- pr is pixel in rectangle ---------------------------
	double     pr		= 0;
	double     md   = 0;
	double  U = 0;
	double  P = 0;
	int     Q = 0;

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
//==========================================================================
//  OSM Object
//==========================================================================
OSM_Object::OSM_Object(U_INT tp)
{	type				= tp;
	bpm.stamp		= 0;
	bpm.group		= 0;
	tag					= 0;
	val					= 0;
	part				= 0;
	blink				= 0;
	orien       = 0;
	rmodl				= 0;
}
//-----------------------------------------------------------------
//	Destroy resources
//	Queue will be deleted when object is deleted and it will delete
//	all queued objects
//-----------------------------------------------------------------
OSM_Object::~OSM_Object()
{	int a = 0;
	if (tag)			delete [] tag;
	if (val)			delete [] val;
	if (rmodl)		delete [] rmodl;
	if (part)			delete part;
}
//-----------------------------------------------------------------
//	Transfer Queue and Clear items
//-----------------------------------------------------------------
void OSM_Object::Swap(Queue<D2_POINT> &H)
{	//--- Clear the receiver ---------------------------
	H.Clear();
	//--- Transfert base in foot print -----------------
	base.TransferQ(H);
	if (part)		delete part;
	part = 0;
	return;
}
//-----------------------------------------------------------------
//	Transfer Queue and invert items
//-----------------------------------------------------------------
void OSM_Object::Invert(Queue<D2_POINT> &H)
{	//--- Clear the receiver ---------------------------
	H.Clear();
	//--- Transfert base in foot print -----------------
	D2_POINT *pp;
	for (pp = base.Pop(); pp != 0; pp = base.Pop())
	{	H.PutHead(pp);	}
	if (part)		delete part;
	part = 0;
	return;
}

//-----------------------------------------------------------------
//	Transfer queue
//-----------------------------------------------------------------
void OSM_Object::ReceiveQ(Queue<D2_POINT> &H)
{	D2_POINT *pp;
  D2_POINT *np;
	for (pp = H.GetFirst(); pp != 0; pp = pp->next)
	{	np = new D2_POINT(pp,'R');
		base.PutLast(np);
	}
	return;
}
//-----------------------------------------------------------------
//	Rotation to align the building
//-----------------------------------------------------------------
U_CHAR OSM_Object::Rotate()
{	if (bpm.opt.Not(D2B_REPLACED))	  return 0;
	double sn = sin(orien);
	double cn = cos(orien);
	part->ZRotation(sn,cn);
	return 1;
}
//-----------------------------------------------------------------
//	Change rotation
//-----------------------------------------------------------------
U_CHAR OSM_Object::IncOrientation(double rad)
{	orien = WrapTwoPi(orien + rad);
	double sn = sin(rad);
	double cn = cos(rad);
	part->ZRotation(sn,cn);
	return 1;
}
//-----------------------------------------------------------------
//	Save amenity
//-----------------------------------------------------------------
void OSM_Object::SetTag(char *am, char *vl)
{	if (0 ==  *am)	return;
  tag = Dupplicate(am,64);
	if (0 == *vl)		return;
	val	= Dupplicate(vl,64);
	return;
}
//-----------------------------------------------------------------
//	Edit Tag
//-----------------------------------------------------------------
void OSM_Object::EditTag(char *txt)
{	if (0 == tag)		return;
	_snprintf(txt,127,"%s : %s",tag,val);
	return;
}
//-----------------------------------------------------------------
//	Save building in mother group
//-----------------------------------------------------------------
void OSM_Object::SetParameters(D2_BPM &p)
{	//--- check for group change ----------------------------
	bool chge = (bpm.group != 0) && (bpm.group != p.group);
	if (chge) bpm.group->RemBuilding(this);
	bpm				= p;
	D2_Group *grp = bpm.group;
	grp->AddBuilding(this);
	U_INT xob	= grp->GetXOBJ();
	//--- Set part parameters ----------------------
	C3DPart   *prt  = new C3DPart();
	prt->SetXOBJ(xob);
	if (part)	delete part;
	part	= prt;
	return;
}
//-----------------------------------------------------------------
//	Return group texture reference
//-----------------------------------------------------------------
void *OSM_Object::GetGroupTREF()
{	if (0 == bpm.group)		return 0;
	return bpm.group->GetTREF();
}
//-----------------------------------------------------------------
//	Return group texture name
//-----------------------------------------------------------------
char *OSM_Object::TextureName()
{ if (0 == bpm.group)		return 0;
	return bpm.group->TextureName();
}
//-----------------------------------------------------------------
//	Select this building
//-----------------------------------------------------------------
void OSM_Object::Select()
{	bpm.group->SelectedBLDG(this);
	bpm.selc	= 1;
	return;
}
//-----------------------------------------------------------------
//	Change selection on building
//-----------------------------------------------------------------
void OSM_Object::SwapSelect()
{	if (bpm.selc)	Deselect();
	else		Select();
	return;
}

//-----------------------------------------------------------------
//	Deselect this building
//-----------------------------------------------------------------
void OSM_Object::Deselect()
{	bpm.group->SelectedBLDG(0);
	bpm.selc	= 0;
	return;
}
//-----------------------------------------------------------------
//	Blink every 4 frames
//-----------------------------------------------------------------
bool OSM_Object::Skip()
{	blink++;
	blink &= 31;
	return (blink > 16)?(true):(false); 
}
//-----------------------------------------------------------------
//	Swap part
//-----------------------------------------------------------------
void OSM_Object::Replace(char *fn, C3DPart *prt)
{	rmodl	= Dupplicate(fn,63);
	orien	= 0;
	if (part)	delete part;
	part = prt;
	return;
}
//-----------------------------------------------------------------
//	Draw as a terrain object
//-----------------------------------------------------------------
void OSM_Object::Draw()
{	glLoadName(bpm.stamp);
	glPushMatrix();
	SVector T = FeetComponents(globals->geop, this->bpm.geop, bpm.rdf);
	glTranslated(T.x, T.y, T.z);  //T.z);
	part->Draw();	
	glPopMatrix();
}
//-----------------------------------------------------------------
//	Draw as a single local object
//	We dont translate camera at building center
//-----------------------------------------------------------------
void OSM_Object::DrawLocal()
{	part->Draw();	 }
//-----------------------------------------------------------------
//	Write the building
//-----------------------------------------------------------------
void OSM_Object::Write(FILE *fp)
{	char txt[128];
	_snprintf(txt,127,"Start %05d id=%d\n", bpm.stamp, bpm.ident);
	fputs(txt,fp);
	//--- Write style ----------------------------------
	char *nsty = bpm.style->GetSlotName();
	int   rmno = bpm.roofM->GetRoofModNumber();
	int   rofn = bpm.roofP->GetRoofTexNumber();
	_snprintf(txt,127,"Style %s rofm=%d rftx=%d\n",nsty,rmno,rofn);
	fputs(txt,fp);
	//--- Write tag ------------------------------------
	char *vl = (tag)?(val):("---");
	if (tag)
		{	_snprintf(txt,127,"Tag (%s = %s)\n",tag,vl);
			fputs(txt,fp);
		}
	//--- Write all points -----------------------------
	D2_POINT *pp;
	for (pp = base.GetFirst(); pp != 0; pp = pp->next)
	{	_snprintf(txt,127,"V(%.7lf, %.7lf)\n", pp->dgy, pp->dgx);
		fputs(txt,fp);
	}
	//--- Write replace if any -------------------------
	if (0 == rmodl)			return;
	
	_snprintf(txt,127,"Replace (Z=%.7lf) with %s\n",orien,rmodl);
	fputs(txt,fp);
	//--- all ok ---------------------------------------
	return;
}
//======================= END OF FILE =========================================================