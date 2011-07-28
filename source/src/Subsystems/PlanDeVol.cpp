//===============================================================================
// PlanDeVol.cpp
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
#include "../Include/Globals.h"
#include "../Include/PlanDeVol.h"
#include "../Include/WorldObjects.h"
#include "../Include/FuiUser.h"
using namespace std;
//===============================================================================
#define CLIMB_SLOPE (float(0.03))
//===============================================================================
//	CWPoint
//===============================================================================
CWPoint::CWPoint(CFPlan *fp,Tag t) : CSlot()
{	//--- init node part ------------------------------
	Seq       = 0;
  type      = t;
  fplan     = fp;
	fp->IncWPT();
  position.lat = position.lon = position.alt = 0;
  altitude  = 0;
  DBwpt     = 0;
  mTxt[0]   = ' ';
  mTxt[1]   = 0;
  Legdis    = 0;
  disTot    = 0;
	oMrk			= 0;
	ilsF			= 0;
	strcpy(tkoRWY,"NONE");
	strcpy(lndRWY,"NONE");
	strcpy(dbKey, "NONE");
 *Dirt = 0;
	//-------------------------------------------------
  ClearDate(artime);
  elapse.dYears   = 0;
  elapse.dMonths  = 0;
  elapse.dDays    = 0;
  elapse.dHours   = 0;
  elapse.dMinutes = 0;
  elapse.dSeconds = 0;
  elapse.dMillisecs = 0;
  State = WPT_STA_OUT;
	//--- init time part ------------------------------
	*Elap=0; 
  *Etar=0;

}
//-----------------------------------------------------------------
//	Destroy the waypoint
//-----------------------------------------------------------------
CWPoint::~CWPoint()
{	fplan->DecWPT();
}
//-----------------------------------------------------------------
//  Clear date
//-----------------------------------------------------------------
void CWPoint::ClearDate (SDateTime &sd)
{ sd.date.day     = 0;
  sd.date.month   = 0;
  sd.date.year    = 0;
  sd.time.hour    = 0;
  sd.time.minute  = 0;
  sd.time.msecs   = 0;
  sd.time.second  = 0;
  return;
}
//-----------------------------------------------------------
//	Change Altitude
//-----------------------------------------------------------
char *CWPoint::ChangeAltitude(int a)
{	fplan->Modify(1);
	altitude	= a;
	_snprintf(Alti,16,"% 6d ",a);
	Alti[15]	= 0;
	return Alti;
}
//-----------------------------------------------------------
//	Round altitude to 100 feet
//-----------------------------------------------------------
int CWPoint::RoundAltitude(int a)
{	int rst = a % 100;
	int ent = a / 100;
	if (rst) ent++;
	return (ent * 100);
}
//----------------------------------------------------------------------
//	Select best altitude depending on distance from previous node
//	a0 is the previous node altitude
//	Estimation is based on a 3% climbing slope
//----------------------------------------------------------------------
int CWPoint::BestAltitudeFrom(int a0)
{	float		ft = TC_FEET_FROM_MILE(Legdis) *  CLIMB_SLOPE;
	int			a1 = a0 + int(ft);
	int     ax = fplan->actCEIL();		// Current Ceil
	int     sa = min(a1,ax);
	int     ra = RoundAltitude(sa);
	return ra;
}

//-----------------------------------------------------------
//	Set Altitude
//-----------------------------------------------------------
void CWPoint::SetAltitude(int a)
{	altitude	= a;
	_snprintf(Alti,16,"% 6d ",a);
	Alti[15]	= 0;
	return;
}
//-----------------------------------------------------------
//	Set Position
//-----------------------------------------------------------
void CWPoint::SetPosition(SPosition p)
{	position = p;
	CmHead *obj = GetDBobject();
	if (0 == obj)					return;
	if (obj->IsNot(WPT))	return;
	CWPT *wpt = (CWPT *)obj;
	wpt->SetPosition(p);
	return;
}
//-----------------------------------------------------------
//	Set Direction
//-----------------------------------------------------------
void CWPoint::SetDirection(double d)
{	rDir = d;
	_snprintf(Dirt,4,"%03d",int(d));
	Dirt[3] = 0;
	return;
}
//-----------------------------------------------------------
//	Change Take off runway
//-----------------------------------------------------------
void CWPoint::SetTkoRwy(char *r)
{	strncpy(tkoRWY,r,4); 
	tkoRWY[4]	= 0;
	fplan->Modify(1);
	return;	}
//-----------------------------------------------------------
//	Change Landing runway
//-----------------------------------------------------------
void CWPoint::SetLndRwy(char *r)
{	strncpy(lndRWY,r,4); 
	lndRWY[4]	= 0;
	fplan->Modify(1);
	CWPoint *prv = (CWPoint*)fplan->Prev(this);
	NodeEnd(prv);
	return;	}

//-------------------------------------------------------------------
//  Read tags
//  NOTE: Route is no longer used
//-------------------------------------------------------------------
int CWPoint::Read (SStream *stream, Tag tag)
{ int pm1 = 0;
	char txt[128];
	switch (tag) {
	case 'type':
  case 'user':
    ReadTag (&user, stream);
		TagToString(userT,user);
    return TAG_READ;

  case 'name':
    // Read waypoint name
    ReadString (Name, 40, stream);
    return TAG_READ;
	case 'iden':
		ReadString(Iden,5,stream);
		Iden[4]	= 0;
		return TAG_READ;
  case 'llps':
    // Read latitude/longitude position
    { SVector v;
      ReadVector (&v, stream);
      position.lat = v.x;
      position.lon = v.y;
      position.alt = v.z;
    }
    return TAG_READ;

  case 'dbky':
    // Read database key
    ReadString (dbKey, 10, stream);
    return TAG_READ;

  case 'altd':
    // Read altitude
    ReadInt (&pm1, stream);
		SetAltitude(pm1);
    return TAG_READ;

  case 'sped':
    // Read speed
		ReadString(txt,128,stream);
    return TAG_READ;

	//--- Landing runway ----------------------
	case 'land':
		ReadString(lndRWY,4,stream);
		lndRWY[4]	= 0;
		return TAG_READ;

	//--- ILS Frequency -----------------------
	case 'ILS_':
		ReadFloat(&ilsF,stream);
		return TAG_READ;

	//--- Take off runway ----------------------
	case 'tkof':
		ReadString(tkoRWY,4,stream);
		tkoRWY[4]	= 0;
		return TAG_READ;
    // Read termination marker ---------------
  case 'mark':
    ReadString(mTxt,2,stream);
    if (mTxt[0] != 'X') return TAG_READ;
    State = WPT_STA_TRM;
    return TAG_READ;

  default:
    // Generate warning
    WARNINGLOG ("CWPoint::Read unknown tag %s", TagToString(tag));
  }
  return TAG_EXIT;
}
//--------------------------------------------------------------------
//  Return waypoint identity
//--------------------------------------------------------------------
char *CWPoint::GetIdentity()
{ char *none = "----";
  if (DBwpt.IsNull()) return none;
  return DBwpt->GetIdent();
}
//--------------------------------------------------------------------
//  Return SQL Table
//--------------------------------------------------------------------
char *CWPoint::GetSQLtab()
{	if (user == 'airp')	return "APT";
	if (user == 'snav') return "NAV";
	if (user == 'dbwp')	return "WPT";
	return "NUT";
}
//--------------------------------------------------------------------
//  Process first node
//	-Check for speed and altitude
//--------------------------------------------------------------------
void CWPoint::NodeOne(CWPoint *n)
{	//--- Check for Airport --------------------
	if (IsTerminated())				return;
	if ( 'airp' != type)			return;
	ChangeAltitude(int(DBwpt->GetElevation()));
	return;
}
//-----------------------------------------------------------------
//	Second node:  Set Altitude
//	NOTE: For altitude guess a 3% slope is used in climbing
//-----------------------------------------------------------------
void CWPoint::NodeTwo(CWPoint *prv)
{	if (IsTerminated())								return;
	prv->NodeOne(this);
	if (altitude != fplan->actCEIL())	return;
	//--- Compute altitude depending on distance ---
  int			a0 = prv->GetAltitude();
	int     ba = BestAltitudeFrom(a0);
	ChangeAltitude(ba);
	return;
}
//--------------------------------------------------------------------
//  Last node
//	-Check for Airport
//--------------------------------------------------------------------
void CWPoint::NodeEnd(CWPoint *prv)
{	if ('airp' != type)				return;
  //--- Check for landing runway ----------
	//--- Set final on previous node ---------
	if (0  == prv)						return;
	if (prv->IsTerminated())	return;
	//--- Compute previous altitude ----------
	int	pa    = fplan->actCEIL();
//	int pa    = prv->GetAltitude();
	int a0    = int(DBwpt->GetElevation());
	int ba    = BestAltitudeFrom(a0);
	int na    = min(pa,ba);
	prv->ChangeAltitude(na);
	//--- Set ground on this one ------------
	ChangeAltitude(a0);
	fplan->Refresh();
	return;
}
//-----------------------------------------------------------------
// Populate a usr waypoint
//-----------------------------------------------------------------
void CWPoint::UserWaypoint()
{	CWPT	*nod = fplan->CreateUserWPT(&position);
	nod->SetIDN(Iden);
	nod->SetDIS(Legdis);
	nod->SetNOD(this);
	DBwpt = nod;
	return;
}
//-----------------------------------------------------------------
// Get record from database if needed
//-----------------------------------------------------------------
void CWPoint::Populate()
{	if (DBwpt.Assigned())	return;
  if ('uswp' == user)		return UserWaypoint();
	globals->dbc->PopulateNode(this);
	char   *idn = GetIdentity();
	SetIden(idn);
	return;
}
//----------------------------------------------------------------------
//	Modify altitude
//----------------------------------------------------------------------
char* CWPoint::ModifyAltitude(int inc)
{	if (IsTerminated())	return Alti;
	fplan->Modify(1);
	float minA = DBwpt->GetElevation();
	int  a = altitude;
	a += inc;
	//--- Clamp altitude ------------------
	if (a > 100000)	a = 100000;
	if (a <  minA)	a = int(minA);
	//--- Round up to hundred feet --------
	if (a > minA) a = RoundAltitude(a);
	//--- Edit the field ------------------
	ChangeAltitude(a);
	return Alti;
}
//----------------------------------------------------------------------
//  Check for NAV
//----------------------------------------------------------------------
void CWPoint::NodeNAV(CWPoint *prv,char m)
{	//--- Update if requested ----------------
	if (m)
	{	strcpy(Mark," ");
		State		= WPT_STA_OUT;
	 *Etar	  = 0;
	}
	//--- check for altitude -----------------
	int minA = int(DBwpt->GetElevation());
	if (altitude > minA)								return;
	//--- first time, guess altitude ---------
	if (0 == prv)												return;
	//--- Check for landing runway -----------
	if (strncmp("NONE",lndRWY,4) != 0)	return;
	//--- Change altitude to 80 ceiling ------
	int	a0 = prv->GetAltitude();
	int ba = BestAltitudeFrom(a0);
	//--- Get next node if any ---------------
	ChangeAltitude(ba);
	return;
}

//----------------------------------------------------------------------
//	EDIT values
//----------------------------------------------------------------------
void CWPoint::Edit()
{ U_INT hh = 0;
  U_INT mn = 0;
  U_INT dd = 0;
  U_INT mo = 0;
  char edt[16];
  SetMark(mTxt);
  _snprintf(edt,12,"%.1f", Legdis);
  SetDist(edt);
  return;
}
//----------------------------------------------------------------------
//	EDIT Distance
//----------------------------------------------------------------------
void CWPoint::SetLegDistance(float d)
{	Legdis	= d;
  _snprintf(Dist,10,"%.1f",d);
	Dist[9]	= 0;
	return;
}
//-----------------------------------------------------------------
//	Outside waypoint
//-----------------------------------------------------------------
bool CWPoint::Outside()
{	if (pDis > 2.0)		return true;
	//--- we are now inside --------------
	State = WPT_STA_INS;
	strcpy(Mark,"O");
	fplan->Refresh();
	return true;
}
//-----------------------------------------------------------------
//  Return Arrival time
//-----------------------------------------------------------------
void CWPoint::EditArrival()
{ artime = globals->tim->GetUTCDateTime();
	int hh = artime.time.hour;
  int mn = artime.time.minute;
  int dd = artime.date.day;
  int mo = artime.date.month;
  _snprintf(Etar,16,"% u-% u at %02uh%02u",dd,mo,hh,mn);
  return;
}
//-----------------------------------------------------------------
//	Inside waypoint
//-----------------------------------------------------------------
bool CWPoint::Inside()
{ if (pDis > 0.75)			return true;
	//--- Waypoint is terminated now ------
	State = WPT_STA_TRM;
	strcpy(Mark,"X");
	EditArrival();
	fplan->Refresh();
	return false;
}
//-----------------------------------------------------------------
//	Update current node
//-----------------------------------------------------------------
bool CWPoint::Update()
{	//--- compute distance to aircraft ---------
	CmHead *obj = DBwpt.Pointer();
  float dis = GetRealFlatDistance(obj); 
	//--- update according to state ------------
  pDis  = double(dis);
	switch(State)
	{	//--- We still are outside -----
		case WPT_STA_OUT:
			return Outside();
	  case WPT_STA_INS:
			return Inside();
	}
	return false;
}
//------------------------------------------------------------
//  Save this waypoint
//------------------------------------------------------------
void CWPoint::Save(SStream *s)
{ char mrk[2] = {' ',0};
  char txt[256];
  if (IsTerminated()) mrk[0] = 'X';
  WriteTag('wpnt', "--------- Waypoint ---------------", s);
  TagToString(txt,type);
  WriteString(txt,s);
  WriteTag('bgno', "========== BEGIN OBJECT ==========", s);
  WriteTag('name', "---------- Waypoint name -----", s);
  WriteString(Name,s);
	WriteTag('iden', "-----------Waypoint identity--", s);
	WriteString(Iden,s);
  WriteTag('llps', "---------Lat/lon,alt position ", s);
  WritePosition(&position,s);
  WriteTag('dbky', "---------Database key --------", s);
  WriteString(dbKey,s);
	TagToString(txt,user);
  WriteTag('user', "---------Waypoint usage-------", s);
  WriteString(txt,s);
  WriteTag('altd', "---Altitude (feet) at WPT-----", s);
  WriteInt(&altitude,s);
	WriteTag('tkof', "---Take off runway -----------", s);
	WriteString(tkoRWY,s);
	WriteTag('land', "---Landing runway ------------", s);
	WriteString(lndRWY,s);
	if (ilsF != 0) 
	{	WriteTag('ILS_', "---ILS FREQUENCY -------------", s);
		WriteFloat(&ilsF,s);
	}
  WriteTag('mark', "---Terminated mark- ----------", s);
  WriteString(mrk,s);
  WriteTag('endo', "========== END OBJECT ============", s);

  return;
}
//----------------------------------------------------------------------
//  Edit a Flight Plan line
//----------------------------------------------------------------------
void CWPoint::Print(CFuiList *w,U_CHAR ln)
{ w->NewLine(ln);
  w->AddText(ln, 1, 1,Mark);
  w->AddText(ln, 3,24,Name);
  w->AddText(ln,18, 5,Iden); 
  w->AddText(ln,22,10,Dist);
	w->AddText(ln,30, 4,Dirt);
  w->AddText(ln,34,12,Alti);
  w->AddText(ln,41,12,Elap);
  w->AddText(ln,48,14,Etar);
  return;
}
//===============================================================================
//	FLIGHT PLAN
//===============================================================================
CFPlan::CFPlan(CVehicleObject *m)
{	TypeIs('FPLN');
	SetIdent('....');	
	mveh		= m;
  serial	= 0;
	State		= FPL_STA_NUL;
	modify	= 0;
	//---Init title ---------------------------------
  head.FixeIt();
  head.SetName("Waypoint name");
  head.SetMark("X");
  head.SetIden("Ident");
  head.SetDist("Dis (nm)");
	head.SetDirt("Cap to");
  head.SetAlti("Alti(ft)");
  head.SetElap("Elapse");
  head.SetEtar("Arrival");
	//------------------------------------------------
	Clear(0);
	serial = 0;
	win		 = 0;
	//------------------------------------------------
	format = '0';
}
//-----------------------------------------------------------------
//  Flight Plan is destroyed
//-----------------------------------------------------------------
CFPlan::~CFPlan()
{	wPoints.EmptyIt();
}

//-----------------------------------------------------------------
//  Build a  default name
//-----------------------------------------------------------------
void CFPlan::GenerateName()
{ char  fn[PATH_MAX];
  short yy = globals->clk->GetYear();
  short mm = globals->clk->GetMonth();
  short dd = globals->clk->GetDay();
  bool  go = true;
  while (go)
  { _snprintf(Name,64,"FP%04d_%02d_%02d_%02d",yy,mm,dd,serial++);
    _snprintf(fn,(PATH_MAX-1),"FlightPlan/%s.FPL",Name);
    if (!pexists(&globals->pfs,fn))  return;
  }
  return;
}
//-----------------------------------------------------------------
//  Clear Flight plan
//-----------------------------------------------------------------
void CFPlan::Clear(char m)
{ Modify(m);
  Save();           // Save previous if modified
	wPoints.EmptyIt();
	wPoints.AddSlot(&head);
  GenerateName();
  strcpy (Desc,"Default flight plan");
  State    = FPL_STA_NUL;
  Version  = 0;
  modify   = 0;
	NbWPT		 = 0;
	genWNO	 = 0;
  return;
}
//-------------------------------------------------------------------------
//	Assign a new flight plan from file name
//-------------------------------------------------------------------------
void CFPlan::Assign(char *fn,char opt)
{ char name[PATH_MAX];
  _snprintf(name,(PATH_MAX-1),"FlightPlan/%s.fpl",fn);
  if (!pexists(&globals->pfs,name)) return;
  Clear(0);
  SStream s;
  strncpy (s.filename,name, PATH_MAX);
  strcpy (s.mode, "r");
	Reload(0);
	//--- Read plan and set loaded state ------
  if (!OpenStream (&s))   return;
  option	= opt;
	format  = '0000';
	ReadFrom (this, &s);                 
  CloseStream (&s);
	strncpy(Name,fn,64);
	return;
}
//-----------------------------------------------------------------
// Add a new node to the plan
//-----------------------------------------------------------------
void CFPlan::AddNode(CWPoint *wpt)
{	wpt->Populate();
	wPoints.AddSlot(wpt);
	return;
}
//-----------------------------------------------------------------
// Read format
//-----------------------------------------------------------------
void CFPlan::ReadFormat(SStream *stream)
{	char txt[128];
  char fm[10];
	int  pm = 0;
	ReadString(txt,128,stream);
	int nf = sscanf(txt,"wpno = %d , format=%[^ ]s",&pm,fm);
	if (nf != 2)		return;
	format = StringToTag(fm);
	genWNO = pm;
	return;
}
//-----------------------------------------------------------------
//  Read all tags
//	When option is set, we are just interested by the description
//-----------------------------------------------------------------
int CFPlan::Read (SStream *stream, Tag tag)
{ switch (tag) {
	case 'form':
		ReadFormat(stream);
		return TAG_READ;
  case 'desc':
		if (format != 'FM01') return TAG_EXIT;
    ReadString(Desc,128,stream);
    Desc[128] = 0;
    return (1 == option)?(TAG_EXIT):(TAG_READ);
  case 'vers':
    ReadInt((int*)(&Version),stream);
    return TAG_READ;
	case 'ceil':
		ReadInt(&cALT,stream);
		return TAG_READ;
	//--- Waypoint description ---------------
  case 'wpnt':
    // Read flight plan waypoint sub-object
    { if (option) return TAG_EXIT;
      Tag tp;
      ReadTag (&tp, stream);
			CWPoint *wp     = new CWPoint(this,tp);
      ReadFrom (wp, stream);
			//--- Add a node ---------------------
			wp->Edit();
			//---Add a new slot ------------------
			AddNode(wp); 
			return TAG_READ;
		}

  default:
    // Generate warning
    WARNINGLOG ("CFPlan::Read unknown tag %s", TagToString(tag));
  }
  return TAG_EXIT;
}
//-----------------------------------------------------------------
//	All plan is read
//-----------------------------------------------------------------
void CFPlan::ReadFinished()
{	Reorder(0);
}
//----------------------------------------------------------------------
//  Compute distance between nodes
//----------------------------------------------------------------------
void CFPlan::SetDistance(CWPoint *p0, CWPoint *p1)
{	SVector v = {0,0,0};
	if (0 == p0)	return p1->SetLegDistance(0);
	//--- compute real distance ----------------
	v	= GreatCirclePolar(p0->GetGeoP(), p1->GetGeoP());
	float d = float(v.r) * MILE_PER_FOOT;
	//--- distance to previous -----------------
	p1->SetLegDistance(d);
	//--- direction to p1 ----------------------
	double mdev = p1->GetMagDeviation();
	double rdir = Wrap360((float)v.h - mdev);
	p1->SetDirection(rdir);
	return;
}
//----------------------------------------------------------------------
//  Generate a waypoint name
//----------------------------------------------------------------------
void CFPlan::GenWptName(char *edt)
{	genWNO++;
	if (genWNO == 100)	genWNO = 1;
	_snprintf(edt,4,"WP%02d",genWNO);
	edt[4] = 0;
	return;
}
//-----------------------------------------------------------------
// Create a user waypoint
//	NOTE:  We use the local magnetic deviation for the waypoint
//-----------------------------------------------------------------
CWPT *CFPlan::CreateUserWPT(SPosition *p)
{	char  edt[8];
  GenWptName(edt);
	CWPT *wpt = new CWPT(ANY,WPT);
	wpt->Init(edt,p);
	wpt->SetMGD(globals->magDEV);		
	wpt->SetNAM("User waypoint");
	wpt->SetKey("NONE");
	return wpt;
}
//----------------------------------------------------------------------
//  Reorder the plan
//----------------------------------------------------------------------
void CFPlan::Reorder(char m)
{	CWPoint *wpt  = 0;
	CWPoint	*prv  = 0;
	CWPoint *np1  = 0;
	CWPoint *np2  = 0;
	CWPoint *nm1  = 0;
	U_SHORT dim  = wPoints.GetSize();
	modify	|= m;
	//---- Reored nodes and do computation ---
	for (U_SHORT k = 1; k<dim; k++)
	{	wpt	= (CWPoint *)wPoints.GetSlot(k);
		if (0 == wpt)	break;
		wpt->SetSeq(k);
		SetDistance(prv,wpt);
		if (1 == k)	np1	= wpt;
		if (2 == k) np2 = wpt;
		wpt->NodeNAV(prv,m);
		nm1	= prv;
		prv	= wpt;
	}
	//--- Check for modifications -----
	if (0 == m)		return;
	if (np2)	np2->NodeTwo(np1);
	//--- Check last node -------------
	if (0 == prv)	return;
	if (IsLast(prv->GetSeq())) prv->NodeEnd(nm1);
	return;
}
//-----------------------------------------------------------------
//	Reinit flight plan and possibly save it
//-----------------------------------------------------------------
void CFPlan::Reload(char m)
{	modify |= m;
	//--- Get values for this aircraft -------------
	CSimulatedVehicle     *svh = globals->pln->svh;
	nmlSPD	=  svh->GetCruiseSpeed();
	aprSPD  =  svh->GetApproachSpeed();
	mALT	  =  int(svh->GetCeiling());
	int a   =  mALT / 100;
	int b   =  mALT % 100;
	if (b)	a++;
	mALT    = 100 * a;
	cALT		=  80 * a;
}
//----------------------------------------------------------------------
//	Modify ceil
//----------------------------------------------------------------------
int CFPlan::ModifyCeil(int inc)
{	cALT += inc;
	if (cALT < 500)		cALT = 500;
	if (cALT > mALT)	cALT = mALT;
	return cALT;
}

//-----------------------------------------------------------------
//	Update parameters
//-----------------------------------------------------------------
void	CFPlan::TimeSlice(float dT, U_INT frm)
{	//--- Update according to current state ---------------
	int	fin = NbWPT - 1;					// Final step
	switch (State)	{
		//--- No flight plan loaded ---------------
		case FPL_STA_NUL:
			return;
		//--- Flight plan operational -------------
		case FPL_STA_OPR:
			if (0 == cWPT)			return Stop();
			if (cWPT->Update())	return;
			//--- Change to next waypoint -----------
			cWPT	= (CWPoint*)wPoints.NextPrimary(cWPT);
			return;
	}
}
//-----------------------------------------------------------------
//	Activate the flight plan
//	Check for completness
//-----------------------------------------------------------------
int	CFPlan::Activate(U_INT frm)
{	if (0 == NbWPT)					return 1;
	State = FPL_STA_OPR;
	cWPT	= (CWPoint*)wPoints.HeadPrimary();
	return 0;
}
//-----------------------------------------------------------------
//	DeActivate the flight plan
//	Check for completness
//-----------------------------------------------------------------
void CFPlan::Stop()
{	State = FPL_STA_NUL;
}
//-----------------------------------------------------------------
//	Return airport departing key
//-----------------------------------------------------------------
char *CFPlan::GetDepartingKey()
{	char *none		= "NONE";
	CWPoint *dep	= (CWPoint *)wPoints.HeadPrimary();
	if (0 == dep)	return none;
	return dep->GetDbKey();
}
//-----------------------------------------------------------------
//	Return departing runway identifier
//-----------------------------------------------------------------
char *CFPlan::GetDepartingRWY()
{	char *none		= "NONE";
	CWPoint *dep	= (CWPoint *)wPoints.HeadPrimary();
	if (0 == dep)	return none;
	return dep->GetTkoRwy();
}
//-----------------------------------------------------------------
//	Check for departing runway
//-----------------------------------------------------------------
bool CFPlan::HasTakeOffRunway()
{	CWPoint *dep		= (CWPoint *)wPoints.HeadPrimary();
	if(dep)	return dep->HasTkoRWY();
	return false;
}
//-----------------------------------------------------------------
//	Check for landing runway
//-----------------------------------------------------------------
bool CFPlan::HasLandingRunway()
{	CWPoint *lnd		= (CWPoint *)wPoints.LastPrimary();
	if(lnd)	return lnd->HasLndRWY();
	return false;
}
//-----------------------------------------------------------------
//	Check for final  node
//-----------------------------------------------------------------
bool CFPlan::IsOnFinal()
{	if (0 == cWPT)			return false;
	return (int(NbWPT) == cWPT->GetSeq());
}
//--------------------------------------------------------------------
//  Call vector map to draw the route
//--------------------------------------------------------------------
void CFPlan::DrawOnMap(CFuiVectorMap *win)
{ CWPoint *org = 0;
  CWPoint *ext = 0;
	float		 dis = 0;
	int dim			 = wPoints.GetSize();
  for (int k=1; k<dim; k++)
  { org = (CWPoint *)wPoints.GetSlot(k);
		ext = (CWPoint *)wPoints.NextPrimary(org);
		//----Init route origin ------------------
		rOrg.SetNode(org);
    rOrg.SetPosition(org->GetGeoP());
    if (0 == ext) break;
		rExt.SetNode(ext);
    rExt.SetPosition(ext->GetGeoP());
    win->DrawRoute(rOrg,rExt);
		rExt.SetNode(0);
  }
  return;
}
//-----------------------------------------------------------------
// Change file name
//-----------------------------------------------------------------
void CFPlan::SetFileName(char *n)
{	strncpy(Name,n,64);
	Name[63]	= 0;
	modify		= 1;
	return;
}	
//-----------------------------------------------------------------
// Change Description
//-----------------------------------------------------------------
void CFPlan::SetDescription(char *d)
{	strncpy(Desc,d,128);
	Desc[128]	= 0;
	modify		= 1;
	return;
}	

//----------------------------------------------------------------------
//  Rename the file (Windows dependent
//----------------------------------------------------------------------
void CFPlan::RenameFile(char *old,char *fbak)
{ remove(fbak);
  rename(old,fbak);
}
//--------------------------------------------------------------------
//  Save the Flight Plan
//  version 0:    Initial version
//--------------------------------------------------------------------
void CFPlan::Save()
{ char txt[128];
	if (0 == modify)		return;
	if (0 == NbWPT)     return;
	if (0 == *Name)			return;
  char name[PATH_MAX];
  char renm[PATH_MAX];
  _snprintf(name,(PATH_MAX-1),"FlightPlan/%s.FPL" ,Name);
  _snprintf(renm,(PATH_MAX-1),"FlightPlan/%s.BAK",Name);
  if (0 != Version) RenameFile(name,renm);
  Version++;
  SStream s;
  strncpy (s.filename,name, PATH_MAX);
  strcpy (s.mode, "w");
  if (!OpenStream (&s))     return;
  WriteTag('bgno', "========== BEGIN OBJECT ==========", &s);
	WriteTag('form', "---- Format Type -----------------", &s);
	sprintf(txt,"wpno=%02d, format=FM01",genWNO);
	WriteString(txt, &s);
  WriteTag('desc', "========== Description ===========", &s);
  WriteString(Desc, &s);
  WriteTag('vers', "---------- version number --------", &s);
  WriteInt((int*)(&Version), &s);
  //---Sav individual Waypoint ------------------------
	for (U_INT k=1; k < wPoints.GetSize(); k++) 
	{	CWPoint *wpt = (CWPoint*)wPoints.GetSlot(k);
		wpt->Save(&s); 
	}
	//---------------------------------------------------
  WriteTag('endo', "========== END OBJECT ============", &s);
  CloseStream(&s);
	modify	= 0;
  //---Make it appear in next FP Manager window------
  if (1 == Version) pAddDisk(&globals->pfs,name,name);
  return;
} 
//=======================================================================
//  Fill parameters to compute distance
//=======================================================================
void CRouteEXT::SetPosition(SPosition *p)
{ if (wpt)	wpt->SetPosition(*p);
  double lr   = TC_RAD_FROM_ARCS(p->lat);					// Latittude in Radian
  nmFactor    = cos(lr) / 60;                     // 1 nm at latitude lr
}
//-------------------------------------------------------------
//	return position
//-------------------------------------------------------------
SPosition *CRouteEXT::ObjPosition()
{return wpt->GetGeoP();}
//-------------------------------------------------------------
//	return database object
//-------------------------------------------------------------
CmHead *CRouteEXT::GetOBJ()
{	return wpt->GetDBobject();	}
//-------------------------------------------------------------
//	return Leg Distance 
//-------------------------------------------------------------
float CRouteEXT::GetLegDistance()
{	return wpt->GetLegDistance();}
//-------------------------------------------------------------
//	Set node distance 
//-------------------------------------------------------------
void	CRouteEXT::SetNodeDistance(float d)
{	CmHead *obj = wpt->GetDBobject();
	if (0 == obj)					return;
	if (obj->IsNot(WPT))	return;
	CWPT   *nod = (CWPT*) obj;
	nod->SetDIS(d);
	return;
}
//=================END OF FILE ==================================================