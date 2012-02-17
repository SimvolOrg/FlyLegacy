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
#include "../Include/TerrainTexture.h"
#include <math.h>
//===================================================================================
//	UNIT CONVERTER 
//===================================================================================
#define FOOT_FROM_METERS(X)   (double(X) *  3.2808399)
#define SQRF_FROM_SQRMTR(X)   (double(X) * 10.7639104)
//===================================================================================
//	Style mask decoder
//===================================================================================
char *D2_MXP 		= " Face ( X+ ) SW [ %u , %u ] Dim [ %u , %u ] Rep %lf";
char *D2_MYP		= " Face ( Y+ ) SW [ %u , %u ] Dim [ %u , %u ] Rep %lf";
char *D2_MXM		= " Face ( X- ) SW [ %u , %u ] Dim [ %u , %u ] Rep %lf";
char *D2_MYM		= " Face ( Y- ) SW [ %u , %u ] Dim [ %u , %u ] Rep %lf";
char *D2_MGF		= " gFloor ( %2[^)] ) SW [ %u , %u ] Dim [ %u , %u ] Rep %lf";
char *D2_MMF		= " mFloor ( %2[^)] ) SW [ %u , %u ] Dim [ %u , %u ] Rep %lf";
char *D2_MZF		= " zFloor ( %2[^)] ) SW [ %u , %u ] Dim [ %u , %u ] Rep %lf";
char *D2_ROF		= " Roof  SW[ %u , %u] Dim [%u, %u ] Rep %lf";
//===================================================================================
//	Floor ident	(used as a debug visual mark
//===================================================================================
char *D2_FLID[] = {
	"NO",											// 0 = none
	"gF",											// 1 = TEXD2_HMOD_GFLOOR (ground floor)
	"mF",											// 2 = TEXD2_HMOD_MFLOOR (Middle floor)
	"zF",											// 3 = TEXD2_HMOD_ZFLOOR (Last floor)
};
//==========================================================================================
//  Roof model N°1
//										E---------------F			height = 1 A= origin (0,0,0)
//								D-------A      C--------B
//	Positive base is ABCD with AB the longuest edge
//==========================================================================================
//----- Vertices for model 1 --------------------------------
SVector    vM1[] =
{	{0,0,0},													// 0 Point A
	{1,0,0},													// 1 Point B
	{1,2,0},													// 2 point C
	{0,2,0},													// 3 Point D
	//--- End of base --------------------------------------
	{0,1,1},													// 4 point E
	{1,1,1},													// 5 point F
};
//-------Indices for model 1------------------------------
short xM1[]=
	{	3,0,4,				// T(DAE)					// Face Y-
	  1,2,5,				// T(BCF)					// Face Y+
		//----------------------------------------------------
	  4,0,1,				// T(EAB)
	  4,1,5,				// T(EBF)
	  5,2,4,				// T(FCE)
	  2,3,4,				// T(CDE)
	};
//------Instance for model 1 -----------------------------
CRoofFLAT	roofFLT(0,0,0,0);
CRoofM2P	roofM2P(6,vM1,18,xM1);
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
//	Check visibility (see above)
//	1)	If either point A or B is inside Segment PQ, then we considere that
//			P is masked from Q by this point
//	2)  If  both A and B are on the same side of segment PQ then P an Q are mutually
//			visible
//	3		 As A and B are on opposite sides (or colinear with PQ, but ouside), we must now 
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
//	Check side of point A, relative to vector PQ
//	We just need the case where A is strictly on left of PQ
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
//====================================================================================
void  D2_POINT::Mid(D2_POINT &p, D2_POINT &q, double h)
	{	x		= 0.5 * (p.x + q.x);
		y		= 0.5 * (p.y + q.y);
		z		= 0.5 * (p.z + q.z) + h;
		s		= t = 0;
		lx	= 0.5 * (p.lx + q.lx);
		ly	= 0.5 * (p.ly + q.ly);
		elg	= 0;
		strncpy(idn,"MIDP",4);
	}
//----------------------------------------
void D2_POINT::Translate(double tx,double ty)
	{	lx += tx;
		ly += ty;
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
Triangulator::Triangulator()
{	trace = 1;
	dop.Set(TRITOR_DRAW_ROOF);
	dop.Set(TRITOR_DRAW_WALL);
	dop.Set(TRITOR_DRAW_LINE);
	dop.Set(TRITOR_DRAW_FILL);
	roofM		= &roofFLT;
	rPoint	= 0;
	Clean();
	globals->Disp.Enter(this,PRIO_ABSOLUTE);	
}
//-------------------------------------------------------------------
//	Release resources 
//-------------------------------------------------------------------
Triangulator::~Triangulator()
{	Clean();
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
//	Draw Interface 
//-------------------------------------------------------------------
void Triangulator::Draw()
{	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glColor3f(255,255,255);
	//--- Get texture Object -----------------------------------------
	U_INT xob = 0;
	D2_Group *grp = (style)?(style->GetGroup()):(0);
	if (grp)	xob	=  grp->GetXOBJ();
	glBindTexture(GL_TEXTURE_2D,xob);
	//----------------------------------------------------------------
  char d1 = 0;
	if (dop.Has(TRITOR_DRAW_FILL))	 d1 = FillMode();
	if (d1 && dop.Has(TRITOR_DRAW_WALL))	DrawWall();
	DrawRoof();
	LineMode();
	if (dop.Has(TRITOR_DRAW_WALL))	      DrawWall();
	glPopAttrib();
	return;
}
//-------------------------------------------------------------------
//	Draw as lines 
//-------------------------------------------------------------------
int Triangulator::LineMode()
{	// Draw triangulation
	glPolygonMode(GL_FRONT,GL_LINE);
	return 1;
}
//-------------------------------------------------------------------
//	Draw as color 
//-------------------------------------------------------------------
int Triangulator::FillMode()
{	// Draw triangulation
	glPolygonMode(GL_FRONT,GL_FILL);
	return 1;
}
//-------------------------------------------------------------------
//	Draw Walls 
//-------------------------------------------------------------------
void Triangulator::DrawWall()
{	glColor3f(255,255,255);
	for (U_INT k=0; k != walls.size(); k++) walls[k]->Draw();
}
//-------------------------------------------------------------------
//	Draw Roof 
//-------------------------------------------------------------------
void Triangulator::DrawRoof()
{	D2_TRIANGLE *T;
	for (U_INT k=0; k != roof.size(); k++)
	{	T = roof[k];
		glBegin(GL_TRIANGLES);
		T->Draw();
		glEnd();
	}
	return;
}

//===================================================================
//	Public interface
//===================================================================
//-------------------------------------------------------------------
//	Add a new vertex
//-------------------------------------------------------------------
void Triangulator::AddVertex(double x, double y)
{	D2_POINT *p = new D2_POINT(x,y);
	char txt[8];
	_snprintf(txt,6,"%c%03d",hIND,seq++);
	p->SetID(txt);
	if ('X' == hIND)	extp.PutLast(p);
	else							hole.PutLast(p);
	return;
}
//-------------------------------------------------------------------
//	Load an object file
//-------------------------------------------------------------------
bool Triangulator::Load(char *fn)
{	Clean();
	FILE  *f  = fopen(fn,"r");
  if (0 == f)  return false;
	ParseOBJ(f);
	fclose(f);
	return (extp.GetNbObj() != 0);
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
	for (U_INT k=0; k < walls.size(); k++)	delete walls[k];
	walls.clear();
	for (U_INT k=0; k < roof.size();  k++)	delete roof[k];
	roof.clear();
	roofM = &roofFLT;
	seq		= 0;										// Vertex sequence
	num		= 0;										// Null number
	vRFX	= 0;
	hIND	= 'X';
  //--- Reset all parameters ----------------------
	return;
}
//-------------------------------------------------------------------
//	Parse OBJ descriptor file
//-------------------------------------------------------------------
void Triangulator::ParseOBJ(FILE *fp)
{	char txt[256];
  bool go = true;
	while (go)
	{	char *ch = fgets(txt,256,fp);											// Read a line
		txt[255] = 0;
		_strupr(txt);
		if (ch != txt)										return;					// No error
		if (*txt == 0x0a)									continue;				// Empty
		if (strncmp(txt,"//",2) == 0)			continue;				// Comment
		if (ParseHOL(txt))								continue;				// Hole directive
		if (ParseVTX(txt))								continue;				// Vertices
		if (strncmp(txt,"END",3)== 0)			return;
		go = false;
		STREETLOG("Triangulation of Object %fp ignored");
		Clean();
	}
	return;
}
//-------------------------------------------------------------------
//	Parse Hole directive
//-------------------------------------------------------------------
bool Triangulator::ParseHOL(char *txt)
{	if (strncmp(txt,"HOLE",4) != 0)			return false;
	NewHole();
	return true;
}
//-------------------------------------------------------------------
//	Parse vertex list
//-------------------------------------------------------------------
bool Triangulator::ParseVTX(char *txt)
{	int nv = 0;
  int rd = 0;
	double x;
	double y;
	bool go = true;
	char *src = txt;
	while (go)
	{	int nf = sscanf(src," V ( %lf , %lf ) %n",&x,&y,&rd);
		if (nf != 2)	return (nv != 0);
		AddVertex(x,y);
		src += rd;
		nv++;
	}
	return true;
}
//-------------------------------------------------------------------
//	Save style
//-------------------------------------------------------------------
void Triangulator::SetStyle(D2_Style *sty)
{	flHtr = FOOT_FROM_METERS(2.8);
	flNbr = 1;
	style	= sty;
	if (0 == sty)	return;
	//--- Extract parameters -----------------
	flNbr	= sty->GetFloorNbr();
	flHtr	= sty->GetHeight();
	return;
}
//-------------------------------------------------------------------
//	Compute type of points (reflex or convex)
//	Create slot for triangulation
//	Compute ground surface
//-------------------------------------------------------------------
bool Triangulator::QualifyPoints()
{	int nbp = extp.GetNbObj();
  if (nbp < 3)				return false;
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
		//--- compute surface of pa edge -------
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
	roof.reserve(nbp-2);
	//--- Adjust real surface- ---------------
	surf *= -0.5;
	//--- Reorder with TO as origin point ----
	if (4 == nbp)	Reorder();
	//--- Chose  precision -------------------
	if (trace) TraceInp();
	return  (dlg < geo.GetPrecision())?(false):(true);
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
//-------------------------------------------------------------------
bool Triangulator::GetAnEar()
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
	STREETLOG("Did not find any ear");
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
bool Triangulator::Triangulation()
{	while (slot.GetNbObj() != 2)	if (!GetAnEar()) return false;
	slot.Clear();
	if (trace) TraceOut();
	return true;
}

//-------------------------------------------------------------------
//	Start triangulation (TO BE DELETED)
//-------------------------------------------------------------------
void Triangulator::Start()
{	return;
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
	D2_POINT *hn = 0;
	D2_POINT *ln = 0;
	hole.Transfer(&hn,&ln);
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
//	Before calling this type, the style must be selected and set
//	We need it to precompute some texture coefficients
//=========================================================================
void Triangulator::OrientFaces()
{	//--- Compute rotation matrix -------------------
  D2_POINT *pa = TO;
	D2_POINT *pb = extp.CyNext(pa);
  double sx = (pb->x - pa->x) / dlg;					// Cos(A)
	double sy = (pb->y - pa->y) / dlg;					// Sin(A)
	//--- As we want a -A rotation, we change the signe of sin(A) aka dy
	double mat00		= +sx;				// Line 0 col 0
	double mat01    = -sy;				// Line 0 col 1
	double mat10    = +sy;				// Line 1 col 0 
	double mat11    = +sx;				// Line 1 col 1
	//----Init extension ----------------------------------
	minx		= maxx = 0;
	miny    = maxy = 0;
	//--- Save the matrix ---------------------------------
	rotM[0]	= mat00; rotM[1] = mat01;
	rotM[2] = mat10; rotM[3] = mat11;
	//---- compute new coordinates relatives to PO---------
	D2_POINT *pp = 0;
	for (pp = extp.GetFirst(); pp != 0; pp = pp->next)
	{	double dx = (pp->x - pa->x);
		double dy = (pp->y - pa->y);
	  double lx = ((dx * mat00) + (dy * mat10));
	  double ly = ((dx * mat01) + (dy * mat11));
		//--- Save both max, min extensions ----------------
		pp->lx		= lx;
		pp->ly		= ly;
		if (lx < minx)	minx = lx;
		if (lx > maxx)	maxx = lx;
		if (ly < miny)	miny = ly;
		if (ly > maxy)	maxy = ly;
		//--- qualify previous edge if one ------------------
		if (pp->prev)  QualifyEdge(pp);
	}
  //--- Now we may qualify the first face ---------------
	//	because the last edge is computed
	QualifyEdge(extp.GetFirst());
	//--- Compute P extensions ----------------------------
	Xp	= maxx - minx;
	Yp	= maxy - miny;
	//-----------------------------------------------------
	//   Second pass for 
	//	1) Relocation of local coordinates
	//		Each local coordinate is translated so they are
	//		positives
	//-----------------------------------------------------
	for (pp = extp.GetFirst(); pp != 0; pp = pp->next)
	{	 pp->Translate(-minx,-miny);
	}
	//-----------------------------------------------------
	if (trace) TraceFace();
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
	for (int k=1; k <= flNbr; k++)
	{	BuildFloor(k,Wz,flHtr);
		Wz += flHtr;
	}
	return;
}
//----------------------------------------------------------------
//	Build floor number no
//	f = floor 
//	We mke tour of external points (extp) and for each point,
//	We extrude wall with
//	fh = floor height
//	ce = ceil height
//	We also adjust the roof height (extp)
//----------------------------------------------------------------
void Triangulator::BuildFloor(int No, double fh, double ht)
{	//--- Compute face code -----------------------------
	char indx = TEXD2_FLOORM;								// For middle floor
	if (flNbr == No)	indx = TEXD2_FLOORZ;	// Last floor
	if (1			== No)	indx = TEXD2_FLOOR1;	// ground floor
	//--- Compute floor ceil -----------------------------
	double    ce = fh + ht;				// Ceil height
  D2_POINT *pa = 0;
	D2_FLOOR *et = new D2_FLOOR(No,indx,fh,ce);
	D2_FACE  *fa = 0;
	//--- Build one floor -------------------------------
	for (pa = extp.GetFirst(); pa != 0; pa = pa->next)
	{	fa = new D2_FACE(pa->FaceType(),indx);
		//--- Now we build a face with pa as the base points
		D2_POINT *pb = extp.CyNext(pa);
		fa->Extrude(fh,ce,pa,extp.CyNext(pa));
		et->faces.push_back(fa);
		pa->z		= ce;
		pb->z		= ce;
	}
  walls.push_back(et);
	return;
}
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
//	Select roof model
//	Actually the roof model is selected from the number of points
//	Later other models may be implemented
//----------------------------------------------------------------
void Triangulator::SelectRoof()
{	if (extp.GetNbObj() != 4)				return;
	//--- Delete current roof -----------------------------
	for (U_INT k=0; k < roof.size(); k++) delete roof[k];
	roof.clear();
	//--- Assign a roof model -----------------------------
  roofM = &roofM2P;
	//--- Build a new roof from model---------------------
	rPoint = roofM->BuildRoof(extp,roof);
	Zp				= roofM->GetTop();
	return;
}
//----------------------------------------------------------------
//	Texturing pass
//----------------------------------------------------------------
void Triangulator::Texturing()
{	if (0 == style)		return;
	//--- Set building extensions ---------------------
	style->SetXp(Xp);
	style->SetYp(Yp);
	style->SetZp(Zp);
	style->SetWz(Wz);
	//------------------------------------------------- 
	for (U_INT k = 0; k < walls.size(); k++) 
	{	D2_FLOOR *fl = walls[k];
		fl->TextureFloor(style);
	}
	roofM->Texturing(this,roof);
	return;
}
//=========================================================================
//	D2-FACE class
//=========================================================================
D2_FACE::D2_FACE(char type, char fx)
{	fType = type;
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
//---------------------------------------------------------------
void D2_FACE::TextureFace(D2_Style *sty)
{	if (sty->HasTrace()) STREETLOG("Texture Face------");
  sty->TexturePoint(&sw,fType,fIndx);
	sty->TexturePoint(&se,fType,fIndx);
	sty->TexturePoint(&ne,fType,fIndx);
	sty->TexturePoint(&nw,fType,fIndx);
	//--- check for roof complement ---------------
	
	return;
}
//---------------------------------------------------------------
//	Draw a face as 2 triangles
//---------------------------------------------------------------
void D2_FACE::Draw()
{	glBegin(GL_TRIANGLE_STRIP);
	nw.Draw();
	sw.Draw();
	ne.Draw();
	se.Draw();
	glEnd();
}
//=========================================================================
//	D2-Stair class
//=========================================================================
D2_FLOOR::D2_FLOOR(char e, char t,double f, double c)
{	sNo		= e;
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
//	Draw 
//---------------------------------------------------------------
void D2_FLOOR::Draw()
{	for (U_INT k=0; k < faces.size(); k++)	faces[k]->Draw();
}
//====================================================================================
//	PROCEDURAL Roof Model
//	Those function build several kind of roof when the base contour of the building
//	is a rectangle
//====================================================================================
CRoofModel::CRoofModel(int n,SVector *v,int q, short *x)
{	nbv		= n;
	aVEC	= v;
	nbx		= q;
	aIND	= x;
	aOUT  = 0;
	ratio = 0.5;
	trn		= 0;
}
//---------------------------------------------------------------
//	free vectors
//---------------------------------------------------------------
CRoofModel::~CRoofModel()
{	Clear();	}
//---------------------------------------------------------------
//	free roof model
//---------------------------------------------------------------
void CRoofModel::Clear()
{ aOUT = 0;
}
//---------------------------------------------------------------
//	Fill a triangle from index k
//	NOTE: Only the pointers are assigned to the aOUT points
//	n is the current index number in model description (aIND)
//	m is the current index into  the POINT array
//---------------------------------------------------------------
int CRoofModel::FillTriangle(D2_TRIANGLE &T, short n, short m)
{	D2_POINT *np;
	D2_POINT *sp;
	//--- Dupplicate B vertex ------
	short s0 = aIND[n++];
	sp  = aOUT + s0;
	np	= new D2_POINT(sp);
	T.B	= np;
	aOUT[m++] = *np;
  //--- Dupplicate A vertex ------
	short s1 = aIND[n++];
	sp  = aOUT + s1;
	np	= new D2_POINT(sp);
	T.A = np;
	aOUT[m++] = *np;
  //------------------------
	short s2 = aIND[n++];
	sp  = aOUT + s2;
	np	= new D2_POINT(sp);
	T.C = np;
	aOUT[m++] = *np;
	if (U_INT(m) > tot)	gtfo("Bad m index");
	return n;
}
//---------------------------------------------------------------
//	A roof model must do the following things
//	1) compute roof triangles from the model and the building
//			tour given by 'inp'.  This is done by overwriting
//			the virtual function BuildRoof(..)
//	2) Save those triangles in the output list 'out'
//	3) Compute and save the top point of the roof
//	4) Return a list of vertices that made triangles computed in 1
//---------------------------------------------------------------
//	The list of POINT is organized as
//	-First 4 POINTS are the base points
//	-Next comes the top points
//	-Next come the POINTs needed by triangles with distinct
//	Texture coordinates.
//---------------------------------------------------------------
D2_POINT *CRoofModel::BuildRoof(Queue <D2_POINT> &inp, std::vector<D2_TRIANGLE*> &out)
{	if (inp.GetNbObj() != 4)					return 0;
	//-----Allocate arry of POINTs --------------------------
	tot	= nbv + (3 * nbx);
	aOUT = new D2_POINT[tot];
	D2_POINT *pq;
	int k   = 0;
	//--- Copy contour as the first roof points ------------
	for (pq = inp.GetFirst(); k != 4; pq = pq->next, k++) 	{	aOUT[k] = *pq;	}
	GenerateRoof(inp,out);
	return aOUT;
}
//===================================================================================
//	Roof with 2 Slopes
//===================================================================================
CRoofM2P::CRoofM2P(int n,SVector *v,int q, short *x)
	: CRoofModel(n,v,q,x)
{}
//---------------------------------------------------------------
//	The model M2P builds a roof with 2 slopes
//	
//	NOTE:  The tour points (inp) must have good height
//				the inp should be ordered with X origin as 1rst point
//	The top of roof is computed according to the roof angle
//	For 45° roof  top is half the wall width
//---------------------------------------------------------------
void  CRoofM2P::GenerateRoof(Queue <D2_POINT> &inp, std::vector<D2_TRIANGLE*> &out)
{	//--- Compute both roof top vertices from width--------
	D2_POINT *p0 = aOUT + 0;
	D2_POINT *p1 = aOUT + 1;
	D2_POINT *p2 = aOUT + 2;
	D2_POINT *p3 = aOUT + 3;
	//--- Add the 2 top vertices into POINT array ---------------
	double H = ratio * p1->elg;
	aOUT[4].Mid(*p3,*p0,H);							// Face Y-
	aOUT[5].Mid(*p1,*p2,H);							// Face Y+
	//--- Save the highest point elevation ----------------------
	top	= aOUT[4].z;
	//--- Now build the triangles -------------------------------
	int n = 0;
	int m = nbv;
	//--- NOTE: First 2 triangles must be Y faces ---------------
	while (n < nbx)
	{	D2_TRIANGLE *T = new D2_TRIANGLE(1);
		n	= FillTriangle(*T,n,m++);
		out.push_back(T);
	}
	return;
}
//----------------------------------------------------------------------
//	Texture ta face
//----------------------------------------------------------------------
void CRoofM2P::TextureFace(int ftype, D2_TRIANGLE *T)
{	D2_Style *sty = trn->GetStyle(); 
	if (0 == sty)		return;
	sty->TexturePoint(T->B,ftype,TEXD2_WHOLE);
	sty->TexturePoint(T->A,ftype,TEXD2_WHOLE);
	sty->TexturePoint(T->C,ftype,TEXD2_WHOLE);
	return;
}
//----------------------------------------------------------------------
//	Texture this 2-slopes roof
//----------------------------------------------------------------------
void CRoofM2P::Texturing(Triangulator *t,std::vector<D2_TRIANGLE*> &out)
{	trn = t;
	TextureFace(GEO_FACE_YP,out[1]);
	TextureFace(GEO_FACE_YM,out[0]);
}
//===================================================================================
//	Roof with Flat Top
//===================================================================================
//----------------------------------------------------------------------
//	Generate a flat roof
//----------------------------------------------------------------------
CRoofFLAT::CRoofFLAT(int n,SVector *v,int q, short *x)
	: CRoofModel(n,v,q,x)
{;}
//----------------------------------------------------------------------
//	Generate a flat roof
//----------------------------------------------------------------------
void  CRoofFLAT::GenerateRoof(Queue <D2_POINT> &inp, std::vector<D2_TRIANGLE*> &out)
{;}
//----------------------------------------------------------------------
//	Texture this 2-slopes roof
//----------------------------------------------------------------------
void CRoofFLAT::Texturing(Triangulator *trn,std::vector<D2_TRIANGLE*> &out)
{	;}

//===================================================================================
//	STYLE management
//	Create a new group
//
//===================================================================================
D2_Group::D2_Group(char *gn, D2_Session *s)
{	ssn				= s;
	tr				= ssn->HasTrace();
	strncpy(name,gn,64);
	name[63]	= 0;
	sfMin			= 0;
	sfMax			= 10000000;
	//-- Default is side [4,200000] ---------
	sdMin			= 4;
	sdMax			= 200000;
	//--- Floor -----------------------------
	flNbr			= 1;
	flHtr			= 2.8;
	//---------------------------------------
	weight		= 0;
	nber			= 1;
	//--- Index -----------------------------
	indx			= 0;
	//--- No texture yet --------------------
	*path			= 0;
}
//-----------------------------------------------------------------
//	Destroy Group 
//-----------------------------------------------------------------
D2_Group::~D2_Group()
{	for (U_INT k= 0; k < styles.size(); k++) delete styles[k];
	glDeleteTextures(1,&tinf.xOBJ);
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
	  sn->ReadFile(f,buf);
		if (DecodeParam(buf))	continue;
		//--- Convert all units --------------------------------
		sfMin		= SQRF_FROM_SQRMTR(sfMin);	// M² to FEET²
		sfMax		= SQRF_FROM_SQRMTR(sfMax);
		flHtr	  =	FOOT_FROM_METERS(flHtr);
		//--- Load texture if any ------------------------------
		LoadTexture();
		//--- Then exit false ----------------------------------
		go	= false;
	}
	//------Back up --------------------------------------------
	fseek(f,pos,SEEK_SET);
	return false;
}
//-----------------------------------------------------------------
//	Get group parameters  parameters 
//-----------------------------------------------------------------
bool D2_Group::DecodeParam(char *buf)
{	char tex[64];
	//--- Check for surface --------------------------------
	int nf = sscanf(buf," Surface [ %lf , %lf]", &sfMin, &sfMax);
	if (nf == 2)	return true;
	//--- Check for surface to infinity --------------------
  nf = sscanf(buf," Surface [ %lf , * ]",  &sfMin);
	if (nf == 1)	return true;
	//--- Check for side ------------------------------------
	nf = sscanf(buf," Sides [ %u , %u]", &sdMin, &sdMax);
	if  (nf == 2) return true;
	//--- Check for side to infinity ------------------------
	nf = sscanf(buf," Side [%u , * ]", &sdMin);
	if	(nf == 1) return true;
	//--- Check floor number --------------------------------
	nf = sscanf(buf," Floor %u",   &flNbr);
	if  (1 == nf)	return true;
	//--- Check floor height --------------------------------
	nf = sscanf(buf," Height %lf", &flHtr);
	if	(1 == nf)	return true;
	//--- Check texture file -------------------------------
	nf = sscanf_s(buf," Texture %64s",tex,64);
	if  (1 == nf)
	{	tex[63]	= 0;
		_snprintf(path,511,"OpenStreet/Textures/%s",tex);
		return true;
	}
	//--- Not a group parameter ----------------------------
	return false;
}
//-----------------------------------------------------------------
//	Add a new style 
//-----------------------------------------------------------------
void	D2_Group::AddStyle(D2_Style *sy)
{	styles.push_back(sy);
	weight += sy->GetWeight();
	return;
}
//-----------------------------------------------------------------
//	Compute each style ratio
//-----------------------------------------------------------------
void D2_Group::ComputeRatio()
{	if (0 == weight)	return;
	for (U_INT k=0; k < styles.size(); k++)
	{	D2_Style *sty = styles[k];
		int			w = sty->GetWeight();
		double	r = double(w) / double (weight);
		sty->SetRatio(r);
	} 
	return;
}
//-----------------------------------------------------------------
//	Select a group based on surface and number of sides
//	Then select a style based on ratio
//-----------------------------------------------------------------
D2_Style *D2_Group::GetStyle(double sf, int sd)
{ int end = styles.size();
	if (0 == end)												return 0;
	if ((sf < sfMin) || (sf > sfMax))		return 0;
	if ((sd < sdMin) || (sd > sdMax))		return 0;
	//--- Update style index ----------------------
	int   k = end;
	while (k--)	
	{	if (indx >= end)	indx = 0;
		D2_Style *sty = styles[indx++];
		if (!sty->HasTurn(nber))	continue;
		//--- Increment instance number -------------
		nber++;							
		return sty;
	}
	return 0;
}
//-----------------------------------------------------------------
//	Load texture for this group
//-----------------------------------------------------------------
void D2_Group::LoadTexture()
{	if (*path == 0)		return;
	if (tinf.xOBJ)		return;
	CArtParser img(TC_HIGHTR);
  strcpy(tinf.path,path);
  img.GetAnyTexture(tinf);
	globals->txw->GetTexOBJ(tinf,0,GL_RGBA);
	return;
}
//===================================================================================
//
//	Create a new style
//
//===================================================================================
D2_Style::D2_Style(char *snm, D2_Group *gp)
{	strncpy(name,snm,64);
	name[63]	= 0;
	group			= gp;
	weight		= 1;
	flNbr     = gp->GetFloorNbr();
	Fz				= gp->GetFloorHtr();
	nber			= 0;				// Instance number
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
  //--- Free roof parameters --------------------
	for (U_INT k=0; k < roofT.size(); k++) delete roofT[k];
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
bool D2_Style::Decode(char *buf)
{ int nf = 0;
	char floor[4];
	D2_TParam pm;
	char	er = 0;
	//----------------------------------------------------------------------
  nf = sscanf(buf," Weight %u",&weight);
	if (1 == nf)		return true;
	//--- Check for X+ -----------------------------------------------------
	pm.code	= GEO_FACE_XP;
	nf = sscanf(buf,D2_MXP, &pm.x0, &pm.y0, &pm.x1, &pm.y1, &pm.rept);
	if (4 <= nf) 		return AddFace(pm);	
	//--- Check for Y+ -----------------------------------------------------
	pm.code = GEO_FACE_YP;
	nf = sscanf(buf,D2_MYP, &pm.x0, &pm.y0, &pm.x1, &pm.y1, &pm.rept);
	if (4 <= nf)		return AddFace(pm); 	 
	//--- Check for X- ----------------------------------------------------
	pm.code = GEO_FACE_XM;
	nf = sscanf(buf,D2_MXM, &pm.x0, &pm.y0, &pm.x1, &pm.y1, &pm.rept);
	if (4 <= nf)		return AddFace(pm);
	//--- Check for Y- ----------------------------------------------------
	pm.code = GEO_FACE_YM;
	nf = sscanf(buf,D2_MYM, &pm.x0, &pm.y0, &pm.x1, &pm.y1, &pm.rept);
	if (4 <= nf) 	  return AddFace(pm);
	//--- Check for ground Floor ------------------------------------------
	pm.code = 0;
	pm.hmod = TEXD2_HMOD_GFLOOR;
	nf = sscanf_s(buf,D2_MGF,floor,4, &pm.x0,&pm.y0,&pm.x1,&pm.y1,&pm.rept);
	if (5 <= nf)		return AddFloor(floor,pm);
	//--- Check for middle Floor ------------------------------------------
	pm.code = 0;
	pm.hmod = TEXD2_HMOD_MFLOOR;
	nf = sscanf_s(buf,D2_MMF,floor,4,&pm.x0,&pm.y0,&pm.x1,&pm.y1,&pm.rept);
	if (5 <= nf)		return AddFloor(floor,pm);
	//--- Check for Last Floor ------------------------------------------
	pm.code = 0;
	pm.hmod = TEXD2_HMOD_ZFLOOR;
	nf = sscanf_s(buf,D2_MZF,floor,4,&pm.x0,&pm.y0,&pm.x1,&pm.y1,&pm.rept);
	if (5 <= nf)		return AddFloor(floor,pm);
	//--- Check for roof texture ----------------------------------------
	pm.code = 0;
	pm.hmod = 0;
	nf = sscanf(buf,D2_ROF,&pm.x0,&pm.y0,&pm.x1,&pm.y1,&pm.rept);
	if (4 <= nf)		return AddRoof(pm);
	return false;
}
//----------------------------------------------------------------------
//	Add a Face texture definition
//----------------------------------------------------------------------
bool  D2_Style::AddFace(D2_TParam &p)
{	D2_TParam *tp = new D2_TParam();
	tp->AdjustFrom(p, Tw, Th);
	switch (p.code)	{
		case GEO_FACE_XP:
			tp->SetFace("X+");
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
	char *fid = D2_FLID[n];
	D2_TParam *tp = new D2_TParam();
	tp->AdjustFrom(p, Tw, Th);
	tp->SetFace(fl);
	tp->SetFloor(fid);
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
{	D2_TParam *ntp = new D2_TParam();
	ntp->AdjustFrom(p, Tw, Th);
	roofT.push_back(ntp);
	return true;
}
//----------------------------------------------------------------------
//	Check for completion 
//----------------------------------------------------------------------
bool D2_Style::IsOK()
{	if (*param != 0)	return true;
	STREETLOG("Style %s Missing Face(X+) definition",name);
	return false;
}
//----------------------------------------------------------------------
//	Check if the ratio allow for a generation 
//----------------------------------------------------------------------
bool D2_Style::HasTurn(int d)
{	double rt = double(nber) / double(d);
	if (rt > ratio)			return false;
	//--- Increment instance number --------------
	nber++;
	return true;
}
//----------------------------------------------------------------------
//	Compute the S,T texture coordinates for point pp
//	1) Select texture definition in matrix from face type of pp
//	2) Compute S and T coordinates
//
//----------------------------------------------------------------------
void D2_Style::TexturePoint(D2_POINT *pp, char ft, char fx)
{ //--- Load texture --------------------------------------
  char	 ind = (ft << 2);					// Face Type as index in param
  ind += fx;											// Add floor code
	//---- Select the texture -------------------------------
	D2_TParam  *T = param[ind];								// Related exture
	double      R = T->rept;									// Repeat count
	double      H = pp->z;										// Point height
	double      V = pp->VertPos();						// Vertical position
	double      W = (ft & GEO_Y_FACE)?(Zp):(Wz);	// Wall height
	//--- pr is pixel in rectangle ---------------------------
	double     pr		= 0;
	double     md   = 0;
	double  lc;						// Local coordinate
	double  U = 0;
	double  P = 0;
	int     Q = 0;

	//---- Compute S coordinate -----------------------------
	switch (ft)
	{	case GEO_FACE_XP:
			pr		= (T->Rx * pp->LocalX()) / Xp;
			pp->s = (T->x0 + pr) / T->Tw;
			break;
		case GEO_FACE_YP:
		  pr		= (T->Rx * pp->LocalY()) / Yp;
			pp->s = (T->x0 + pr) / T->Tw;
			break;
		case GEO_FACE_XM:
			pr		= T->Rx - ((T->Rx * pp->LocalX())  / Xp);
			pp->s = (T->x0 + pr) / T->Tw;
			break;
		case GEO_FACE_YM:
			pr		= T->Rx - ((T->Rx * pp->LocalY())  / Yp);
			pp->s = (T->x0 + pr) / T->Tw;
			break;
	}
	//---- Compute T coordinate -----------------------------
	if (T->IsWhole())
		{	//--- Pixel in rectangle -------------------
			pr		= (H * T->Ry) / (W * T->Th);
			pr		=  T->Oz + pr;
			pp->t =  pr;
		}
	else 
		{ pp->t = T->Oz + ((V * T->Ry) / (Fz * T->Th));  }
	//-------------------------------------------------------
	if (!group->HasTrace())			return;
	char np[6];
	pp->Id(np);
	STREETLOG("Texture Point %s: S=%.5lf T=%.5lf",np,pp->s,pp->t);
	return;
}
//===================================================================================
//	Create a texture parameter
//===================================================================================
D2_TParam::D2_TParam()
{	strcpy(face,"NO");
	strcpy(flor,"NO");
	rept = 1; 
	hmod = TEXD2_HMOD_WHOLE;
}
//-----------------------------------------------------------------
//	Destroy texture parameter 
//-----------------------------------------------------------------
D2_TParam::~D2_TParam()
{	}
//-----------------------------------------------------------------
//	Copy from parameters and compute some values 
//-----------------------------------------------------------------
void	D2_TParam::AdjustFrom(D2_TParam &p, U_INT tw, U_INT th)
{	*this = p;
	 Tw = tw;
	 Th = th;
	 //--- Save rectangle dimansion -----------------------
	 Rx  = double (x1);
	 Ry  = double (y1);
	 //---- Adjust X1 and Y1 to NE corner of texture ------
	 x1  += x0;	
	 y1	 += y0;
	//--- Compute origin of rectangle in texture ----------
	 Ox   = (tw > 0)? (double (x0) / double(Tw)):(0);
	 Oz   = (th > 0)? (double (y0) / double(Th)):(0);
	 return;
}
//===================================================================================
//
//	Start OpenStreet session
//
//===================================================================================
D2_Session::D2_Session()
{	tr	= 0;
}
//-----------------------------------------------------------------
//	Destroy resources
//-----------------------------------------------------------------
D2_Session::~D2_Session()
{	std::map<std::string, D2_Group*>::iterator ig;
	for (ig = group.begin(); ig != group.end(); ig++) delete (*ig).second;
}
//-----------------------------------------------------------------
//	Stop Session 
//-----------------------------------------------------------------
bool D2_Session::Stop01(char *ln)
{	STREETLOG("Invalid statement %s",ln);
	if (ln) exit(-1);
	return false;
}
//-----------------------------------------------------------------
//	Read session parameters 
//-----------------------------------------------------------------
bool	D2_Session::ReadParameters(char *fn)
{	char path[128];
	_snprintf(path,128,"OpenStreet/%s",fn);
	FILE  *f  = fopen(path,"r");
  if (0 == f)  return false;
	bool ok = ParseSession(f);
	fclose(f);
	return ok;
}
//-----------------------------------------------------------------
//	Read file 
//-----------------------------------------------------------------
void D2_Session::ReadFile(FILE *f, char *buf)
{	bool go = true;
	while (go)
	{	fpos = ftell(f);
	  *buf = 0;
		char *txt = fgets(buf,128,f);
		buf[127] = 0;
		if (0 == txt)		return;
		while ((*txt == 0x09) || (*txt == ' '))	txt++;
		if (strncmp(txt,"//", 2) == 0)	continue;
		if (*buf == 0x0A)								continue;
		return;
	}
	return;
}
//-----------------------------------------------------------------
//	Read session Name
//	NOTE: When all style parameters are OK we may compute the
//				style ration inside of each group. 
//-----------------------------------------------------------------
bool D2_Session::ParseSession(FILE *f)
{	char buf[128];
  ReadFile(f,buf);
	char *nm = 0;
	int nf = sscanf_s(buf,"Session %63s", name,63);
	name[63] = 0;
	if (nf!= 1)				return false;
	//--- Parse Groups ----------------------
	ParseGroups(f);
	fseek(f,fpos,SEEK_SET);
	ParseStyles(f);
	//--- Compute all ratios ----------------
	std::map<std::string,D2_Group*>::iterator rg;
	for (rg = group.begin(); rg != group.end(); rg++) 
	{	D2_Group *gp = (*rg).second;
		gp->ComputeRatio();
	}
	//--- Normally we got the end statement here
	ReadFile(f,buf); 
	char *ok = strstr(buf,"Fin");
	if (ok)		return true;
	//--------------------------------------
	STREETLOG("Error arround %s",buf);
	return false;
}
//-----------------------------------------------------------------
//	Read a group 
//-----------------------------------------------------------------
bool D2_Session::ParseGroups(FILE *f)
{	char buf[128];
	char gnm[64];
	bool go = true;
	while (go)
	{	fpos = ftell(f);
		ReadFile(f,buf);
		int nf = sscanf_s(buf," Group %63s",gnm,63);
		gnm[63]  = 0;
		if (nf != 1)	break;
		//--- Allocate a new group -----------------
		D2_Group *gp = new D2_Group(gnm,this);
		group[gnm] = gp;
		gp->Parse(f,this);
	}
	return false;
}
//-----------------------------------------------------------------
//	Read a Style 
//-----------------------------------------------------------------
bool D2_Session::ParseStyles(FILE *f)
{ char buf[128];
  char snm[64];
	char gnm[64];
	bool go = true;
	while (go)
	{	int pos = ftell(f);
		ReadFile(f,buf);
		int nf	= sscanf_s(buf," Style %63s ( %64[^)]s )",snm, 63, gnm, 63);
		snm[63] = 0;
		gnm[63]	= 0;
		if (nf == 2)	{ AddStyle(f,snm,gnm);	continue; }
		//--- Back up one line --------------------
		fseek(f,pos,SEEK_SET);
		return false;
	}
	return false;
}
//-----------------------------------------------------------------
//	Add a style  
//-----------------------------------------------------------------
bool D2_Session::AddStyle(FILE *f,char *sn,char *gn)
{	//--- Find group ----------------------------
	std::map<std::string,D2_Group*>::iterator rg = group.find(gn);
	if (rg == group.end())	return Stop01(sn);
	D2_Group *gp = (*rg).second;
	D2_Style *sy = new D2_Style(sn,gp);
	//--- Decode Style parameters ---------------
	bool go = true;
	char buf[128];
	while (go)	
	{	fpos = ftell(f);	
		ReadFile(f,buf);
		if (sy->Decode(buf))	continue;
		//-- have a new style for the group -----
		gp->AddStyle(sy);
		//--- Exit now --------------------------
		fseek(f,fpos,SEEK_SET);
		return (sy->IsOK());
	}
	return false;
}
//------------------------------------------------------------------
//	From surface and number of side, return a style
//------------------------------------------------------------------
D2_Style *D2_Session::GetStyle(double sf, int side)
{	std::map<std::string,D2_Group*>::iterator rg;
	for (rg = group.begin(); rg != group.end(); rg++)
	{	D2_Group *grp = (*rg).second;
		D2_Style *sty = grp->GetStyle(sf,side);
		if (sty)	return sty;
	}
	return 0;
}

//======================= END OF FILE =========================================================