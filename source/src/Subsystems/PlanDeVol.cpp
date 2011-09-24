//=====================================================================================
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
//=====================================================================================
#include "../Include/Globals.h"
#include "../Include/PlanDeVol.h"
#include "../Include/WorldObjects.h"
#include "../Include/FuiUser.h"
using namespace std;
//===============================================================================
#define CLIMB_SLOPE (float(0.03))
#define AUTO_RTIME  (1800)						// Refresh every 30 mn
//=====================================================================================
//  preconddition decoding structure
//=====================================================================================
struct ROBOT_COND	{
	char *txt;
	char  code;
};
//=====================================================================================
//  preconddition decoding table
//=====================================================================================
ROBOT_COND	rcondTAB[]  = {
	{"State == %d",D2R2_IF_STATE_EQU},
	{"State != %d",D2R2_IF_STATE_NEQ},
	{"State >  %d",D2R2_IF_STATE_GTR},
	{"State >= %d",D2R2_IF_STATE_GEQ},
	{"State <  %d",D2R2_IF_STATE_LTN},
	{"State <= %d",D2R2_IF_STATE_LEQ},
	{0},
};
//=====================================================================================
//  AIRCRAFT CHECK LIST
//=====================================================================================
//-------------------------------------------------------------------------
//  Check for multiple Left Click action
//  Multiple Left Click with Mouse (MLCM)
//-------------------------------------------------------------------------
bool CheckChapter::MLClikAction(char *buf)
{ int p1, p2, rp, tm = 0;
  int nf = sscanf(buf," %d * L-Click ( %d , %d , %d )",&rp,&p1, &p2, &tm);
  if (4 != nf)    return false;
	ra.actn	= D2R2_LEFT_CLICK_COORD;
	ra.pm1	= p1;
	ra.pm2	= p2;
  ra.rptn = rp;
	ra.timr = tm;
  return true;
}
//-------------------------------------------------------------------------
//  Check for Single Left Click action
//  Single Left Click with Mouse (SLCM)
//-------------------------------------------------------------------------
bool CheckChapter::SLClikAction(char *buf)
{ int p1, p2 = 0;
  int nf = sscanf(buf,"L-Click ( %d , %d )",&p1, &p2);
  if (2 != nf)    return false;
	ra.actn	= D2R2_LEFT_CLICK_COORD;
	ra.pm1	= p1;
	ra.pm2	= p2;
	ra.rptn	= 1;
  return true;
}
//-------------------------------------------------------------------------
//  Check for Right Click action
//  Multiple Right Click with Mouse (MLCM)
//-------------------------------------------------------------------------
bool CheckChapter::MRClikAction(char *buf)
{ int p1, p2, rp, tm = 0;
  int nf = sscanf(buf," %d * R-Click( %d , %d , %d )",&rp,&p1, &p2, &tm);
  if (4 != nf)    return false;
	ra.actn	= D2R2_RITE_CLICK_COORD;
	ra.pm1	= p1;
	ra.pm2	= p2;
	ra.rptn = rp;
	ra.timr = tm;
  return true;
}
//-------------------------------------------------------------------------
//  Check for Right Click action
//  Single Right Click with Mouse (SRCM)
//-------------------------------------------------------------------------
bool CheckChapter::SRClikAction(char *buf)
{ int p1, p2 = 0;
  int nf = sscanf(buf," R-Click( %d , %d )",&p1, &p2);
  if (2 != nf)    return false;
	ra.actn		= D2R2_RITE_CLICK_COORD;
	ra.pm1		= p1;
	ra.pm2		= p2;
	ra.rptn		= 1;
  return true;
}
//-------------------------------------------------------------------------
//  Check for Goto action
//  Go to nn%
//  0%  => Frame 0
//  100%=> Last frame
//-------------------------------------------------------------------------
bool CheckChapter::RatioAction(char *buf)
{ int p1,p2,rp = 0;
  if (3 != sscanf(buf," Ratio ( %d%%, %d , %d )",&rp,&p1,&p2))  return false;
  ra.actn		= D2R2_CLICK_UPTO_RATIO;
	ra.pm1		= p1;
	ra.pm2		= p2;
	ra.rptn   = rp;
	ra.timr   = 1;
  return true;
}
//-------------------------------------------------------------------------
//  Check for CA action
//  multiple Left Click on CA field (LCCA)
//-------------------------------------------------------------------------
bool CheckChapter::MLFieldAction(char *buf)
{ int rp, ca, tm = 0;
  if (3 != sscanf(buf," %d * L-Click( CA%d , %d )",&rp, &ca, &tm)) return false;
	ra.actn		= D2R2_LEFT_CLICK_CAFLD;
	ra.pm1		= 0;
	ra.pm2		= 0;
	ra.canb		= ca;
	ra.rptn		= 1;
	ra.timr		= tm;
  return true;
}
//-------------------------------------------------------------------------
//  Check for CA action
//  Left Click on CA field (LCCA)
//-------------------------------------------------------------------------
bool CheckChapter::SLFieldAction(char *buf)
{ int ca = 0;
  if (1 != sscanf(buf," L-Click( CA%d )",&ca))  return false;
	ra.actn		= D2R2_LEFT_CLICK_CAFLD;
	ra.pm1		= 0;
	ra.pm2		= 0;
	ra.canb		= ca;
	ra.timr		= 10;
	ra.rptn		= 1;
  return true;
}
//-------------------------------------------------------------------------
//  Check for CA action
//  multiple Left Click on CA field (LCCA)
//-------------------------------------------------------------------------
bool CheckChapter::MRFieldAction(char *buf)
{ int rp, ca, tm = 0;
  if (3 != sscanf(buf," %d * R-Click( CA%d , %d )",&rp, &ca, &tm)) return false;
	ra.actn		= D2R2_RITE_CLICK_CAFLD;
	ra.pm1		= 0;
	ra.pm2		= 0;
	ra.canb		= ca;
	ra.rptn		= rp;
	ra.timr		= tm;
  return true;
}
//-------------------------------------------------------------------------
//  Check for CA action
//  Right Click on CA field (RCCA)
//-------------------------------------------------------------------------
bool CheckChapter::SRFieldAction(char *buf)
{ int ca = 0;
  if (1 != sscanf(buf," R-Click ( CA%d )",&ca)) return false;
	ra.actn	= D2R2_RITE_CLICK_CAFLD;
	ra.pm1	= 0;
	ra.pm2	= 0;
	ra.canb	= ca;
	ra.timr	= 10;
	ra.rptn	= 1;
  return true;
}

//-------------------------------------------------------------------------
//  Check for show action
//-------------------------------------------------------------------------
bool CheckChapter::ShowAction(char *buf)
{ int tm;
  if (1 != sscanf(buf,"showGauge (%d) ",&tm)) return false;
	ra.actn		= D2R2_SHOW_FOCUS_GAUGE;
	ra.pm1		= 0;
	ra.pm2		= 0;
	ra.timr		= tm;
	ra.rptn		= 1;
	flag		= (D2R2_NEED_PANL + D2R2_NEED_GAGE);
  return true;
}
//-------------------------------------------------------------------------
//  Check for show panel
//-------------------------------------------------------------------------
bool CheckChapter::PanlAction(char *buf)
{ char act[128];
  if (1 != sscanf(buf," %s",act))             return false;
  if (strncmp(act,"showPanel",9))             return false;
	ra.actn	= D2R2_SHOW_FOCUS_PANEL;
	ra.pm1	= 0;
	ra.pm2	= 0;
	ra.timr	= 1;
	ra.rptn	= 1;
	flag	= (D2R2_NEED_PANL);
  return true;
}
//-------------------------------------------------------------------------
//  Decode action
//-------------------------------------------------------------------------
void CheckChapter::DecodeAction(SStream *st)
{ char buf[128];
  ReadString(buf,128,st);
	ra.user	= pLIN;
  if (SLClikAction(buf))    return;     // Decode single left click
  if (MLClikAction(buf))    return;     // Decode multiple left click
  if (SRClikAction(buf))    return;     // Decode single right click
  if (MRClikAction(buf))    return;     // Decode multiple right click

  if (RatioAction(buf))     return;
  if (SLFieldAction(buf))   return;     // Decode single left click on CA field
  if (MLFieldAction(buf))   return;     // Decode multiple left click on CA field

  if (SRFieldAction(buf))   return;     // Single right click on CA field
  if (MRFieldAction(buf))   return;     // multiple right click on CA field

  if (ShowAction(buf))			return;
  if (PanlAction(buf))			return;
  return;
}
//-------------------------------------------------------------------------
//  Decode Gauge and Click area
//-------------------------------------------------------------------------
void CheckChapter::DecodeGauge(char *buf)
{ Tag  idn = 0;
  int  ca;
  char txt[256];
  //----Try gauge tag with CA number --------
  if (2 == sscanf(buf," %4s - CA%d",txt,&ca))
  { idn = StringToTag(txt);
		ra.ggt		= idn;
    ra.canb		= ca;
  }
  else
  //----Just the gauge tag ------------------
  if (1 == sscanf(buf," %4s ",txt))
  { idn = StringToTag(txt);
		ra.ggt	= idn;
  }
  return;
}
//-------------------------------------------------------------------------
//  Decode condition
//-------------------------------------------------------------------------
void CheckChapter::DecodeCondition(char *txt)
{	ROBOT_COND *cnd = 0;
  int val;
  for (cnd=rcondTAB; cnd->txt!=0; cnd++)
	{	int nf = sscanf(txt,cnd->txt,&val);
		if (nf != 1)	continue;
		ra.vtst	= val;
		ra.cond = cnd->code;
		return;
	}
	return;
}

//=====================================================================================
//  CHECK LIST  CHAPTER
//=====================================================================================
int CheckChapter::Read(SStream *st,Tag tag)
{ CChkLine *lin = 0;
  Tag pm;
  char line[256];
  switch (tag)  {

  case 'line':
    GenerateLines(st);
    return TAG_READ;
  case 'panl':
    ReadTag(&pm,st);           // Read panel id
		ra.pnt	= pm;
    ReadString(line,256,st);   // Read gauge id
    DecodeGauge(line);
    return TAG_READ;
	//--- Condition statement ------------------------
	case'cond':
		ReadString(line,256,st);
		DecodeCondition(line);
		return TAG_READ;
	//--- end autostatr statement --------------------
	case 'ends':									// End of autostart
		flag	|= D2R2_END_START;		// Mark action
		return TAG_READ;
	//--- Execute statement --------------------------
  case 'exec':
    if (0 == pLIN) gtfo("CheckList: Execute order without line");
    DecodeAction(st);
		if (pLIN)	pLIN->StoreAction(ra,flag);
		ra.Clear();
		pLIN	= 0;
		flag	= 0;
    return TAG_READ;

  }
  return TAG_IGNORED;
}
//-------------------------------------------------------------------------
//  Delete chapter
//-------------------------------------------------------------------------
CheckChapter::~CheckChapter()
{ std::vector<CChkLine*>::iterator ln;
  for (ln = vLIN.begin(); ln != vLIN.end(); ln++)
  { CChkLine *line = (*ln);
    delete line;
  }
  vLIN.clear();
}

//-------------------------------------------------------------------------
//  Fill the box with the lines from this chapter
//-------------------------------------------------------------------------
void CheckChapter::GetLines(CListBox &box)
{ int dim = vLIN.size();
  for (int k=0; k < dim; k++)
  { CChkLine *lin = vLIN[k];
    box.AddSlot(lin);
  }
  return;
}
//-------------------------------------------------------------------------
//  Generate a new node 
//-------------------------------------------------------------------------
void CheckChapter::CreateNode(SStream *st,int No)
{ CChkLine *nod = new CChkLine(No);
  char      dst[1024];
  ReadString(dst,1023,st);
  //----Add text  ---------------------
  nod->FixeIt();
  nod->SetName(dst);
  nod->SetTotLine(nbl);
  vLIN.push_back(nod);
  if (0 == No) pLIN = nod;
  return;
}
//-------------------------------------------------------------------------
//  Generate line for display 
//-------------------------------------------------------------------------
void CheckChapter::GenerateLines(SStream *st)
{ ra.Clear();
	//----Create primary slot ---------------------
  int nbr = 0;
  int No  = 0;
  ReadInt(&nbr,st);
  nbl     = nbr;
  while (nbr--)  CreateNode(st,No++);
	flag  = D2R2_CHECK_LST;
	return;
}
//=====================================================================================
//  AIRCRAFT CHECK LIST
//  AUTOSTART will engage the whole procedure to start engine
//=====================================================================================
PlaneCheckList::PlaneCheckList(CVehicleObject *v)
{ hwId = HW_OTHER;
  unId = 'chkl';
	mveh = v;
  cWIN = 0;
	d2r2 = mveh->GetRobot();
}
//-------------------------------------------------------------------------
PlaneCheckList::~PlaneCheckList()
{	int end = autoM.size();
	for (int k=0; k<end; k++) delete autoM[k];
	autoM.clear();
}
//-------------------------------------------------------------------------
//  Decode Check list
//-------------------------------------------------------------------------
void PlaneCheckList::OpenList(char *tail)
{ char name[PATH_MAX];
  sprintf(name,"WORLD/VEH_%s.LCK",tail);
  if (!pexists(&globals->pfs,name)) return;
  SStream s;
  strncpy (s.filename,name, PATH_MAX);
  strcpy (s.mode, "r");
  if (OpenStream (&s))
  { ReadFrom (this, &s);                 
    CloseStream (&s);
  }
  return;
}
//-------------------------------------------------------------------------
//	New message for autostart
//-------------------------------------------------------------------------
bool PlaneCheckList::DecodeMSG(char *txt)
{	if (IntMessage(txt))		return true;
  if (FltMessage(txt))		return true;
	return false;
}
//-------------------------------------------------------------------------
//	Decode message for autostart
//-------------------------------------------------------------------------
bool PlaneCheckList::IntMessage(char *txt)
{ char ds[6];
	char fn[6];
	char hw[16];
	int nk;
	int p1;
	int nf = sscanf(txt,"int %d to %4s - %4s %n",&p1,ds,fn,&nk);
	if (3 != nf)		return false;	
	//--- Build a message ---------------------
	SMessage *msg = new SMessage();
	memset(msg,0,sizeof(SMessage));
	msg->id				= MSG_SETDATA;
  msg->group		= StringToTag(ds);
	msg->user.u.datatag = StringToTag(fn);
	msg->intData	= p1;
	strncpy(msg->dst,ds,5);
	//--- Add one message ---------------------
	autoM.push_back(msg);
	//--- check for hardware id ---------------	
	txt += nk;
	if (0 == *txt)	return true;
	nf = sscanf(txt,"( %16[^ )]s", hw);
	if (1 != nf)		return true;
	msg->user.u.hw = GetHardwareType(hw);
	return true;
}
//-------------------------------------------------------------------------
//	Prepare message for autostart
//-------------------------------------------------------------------------
void PlaneCheckList::PrepareMsg(CVehicleObject *veh)
{	int end = autoM.size();
	for (int k=0; k<end; k++)
	{ SMessage *msg = autoM[k];
		msg->sender = unId;
		veh->FindReceiver(msg);
	}
	return;
}
//-------------------------------------------------------------------------
//	Decode message for autostart
//-------------------------------------------------------------------------
bool PlaneCheckList::FltMessage(char *txt)
{ char ds[6];
	char fn[6];
	char hw[16];
	int nk;
	float p1;
	int nf = sscanf(txt,"real %f to %4s - %4s %n",&p1,ds,fn,&nk);
	if (3 != nf)		return false;	
	//--- Build a message ---------------------
	SMessage *msg = new SMessage();
	memset(msg,0,sizeof(SMessage));
	msg->id				= MSG_SETDATA;
  msg->group		= StringToTag(ds);
	msg->user.u.datatag = StringToTag(fn);
	msg->realData = p1;
	strncpy(msg->dst,ds,5);
	//--- Add one message ---------------------
	autoM.push_back(msg);
	//--- check for hardware id ---------------	
	txt += nk;
	if (0 == *txt)	return true;
	nf = sscanf(txt,"( %16[^ )]s", hw);
	if (1 != nf)		return true;
	msg->user.u.hw = GetHardwareType(hw);
	return true;
}

//-------------------------------------------------------------------------
//  Read auto statement
//-------------------------------------------------------------------------
void PlaneCheckList::ReadAUTO(SStream *st)
{	char txt[128];
  char end[6];
	bool go = 1;
	while (go)
	{	ReadString(txt,128,st);
		sscanf(txt,"%4[^ ,;=]s",end);
		go = (strncmp("endm",end,2) != 0);
		if (go)	DecodeMSG(txt);
	}
	return;
}
//-------------------------------------------------------------------------
//  Read chapters
//-------------------------------------------------------------------------
int PlaneCheckList::Read(SStream *st,Tag tag)
{ switch (tag)  {
    case 'chap':
			{ CheckChapter *chap = new CheckChapter();
				char *dst = chap->GetName();
				int   dim = chap->GetSize();
				ReadString(dst,dim,st);
				dst[dim-1] = 0;
				ReadFrom(chap,st);
				vCHAP.push_back(chap);
				return TAG_READ;
			}
		case 'auto':
			ReadAUTO(st);
			return TAG_READ;
  }
  return TAG_IGNORED;
}
//--------------------------------------------------------------------------
//  Return a list of Pointers to chapter
//--------------------------------------------------------------------------
char **PlaneCheckList::GetChapters()
{ int k;
  int nchap = vCHAP.size();
  if (0 == nchap)   return 0;
  char **tab = new char*[nchap+1];
  for (k=0; k<nchap; k++)
  { CheckChapter *chp = vCHAP[k];
    tab[k] = chp->GetName();
  }
  tab[k]  = 0;
  return tab;
}
//-------------------------------------------------------------------------
//  Close check list
//-------------------------------------------------------------------------
void PlaneCheckList::Close()
{ if (cWIN) cWIN->Close(); 
  std::vector<CheckChapter*>::iterator ch;
   for (ch = vCHAP.begin(); ch != vCHAP.end(); ch++)
   {  CheckChapter* chap = (*ch);
      delete chap;
   }
   vCHAP.clear();
}
//-------------------------------------------------------------------------
//  Register the managing window
//-------------------------------------------------------------------------
void PlaneCheckList::RegisterWindow(CFuiCkList *w)
{	cWIN	= w;
	if (d2r2) d2r2->Register(w);
	return;
}
//-------------------------------------------------------------------------
//  Execute one statemnt
//-------------------------------------------------------------------------
bool PlaneCheckList::Execute(D2R2_ACTION &a)
{	return (d2r2)?(d2r2->Execute(a)):(false);}
//--------------------------------------------------------------------------
//  Fill the box with lines from chapter No
//--------------------------------------------------------------------------
void PlaneCheckList::GetLines(CListBox &box,U_INT No)
{ if (No >= vCHAP.size())  return;
  CheckChapter *chap = vCHAP[No];
  chap->GetLines(box);
  return;
}
//--------------------------------------------------------------------------
//  Receive a message
//--------------------------------------------------------------------------
EMessageResult PlaneCheckList::ReceiveMessage (SMessage *msg)
{ if (!MsgForMe (msg)) return MSG_IGNORED;
  msg->receiver = this;
  if (msg->user.u.datatag == 'show')  
  { globals->fui->ToggleFuiWindow (FUI_WINDOW_CHECKLIST);
    return MSG_PROCESSED;
  }
  return MSG_IGNORED;}
//--------------------------------------------------------------------------
//  Autostart
//--------------------------------------------------------------------------
bool PlaneCheckList::AutoStart()
{	int end = autoM.size();
	if (0 == end)	return false;
	for(int k=0; k<end; k++)
	{	SMessage *msg = autoM[k];
		if (MSG_PROCESSED == Send_Message(msg)) continue;
		return false;
	}
	return true;
}
//--------------------------------------------------------------------------
//  Return message n° 
//--------------------------------------------------------------------------
SMessage *PlaneCheckList::GetSMessage(U_INT k)
{	if (k >= autoM.size())	return 0;
	return autoM[k];	}
//===============================================================================
//	CWPoint
//===============================================================================
CWPoint::CWPoint(CFPlan *fp,Tag t) : CSlot()
{	Build(t);
  fplan     = fp;
	fp->IncWPT();
}
//-----------------------------------------------------------------
//	Default constructor
//-----------------------------------------------------------------
CWPoint::CWPoint() : CSlot()
{	Build(0);	}
//-----------------------------------------------------------------
//	Destroy the waypoint
//-----------------------------------------------------------------
CWPoint::~CWPoint()
{	if (fplan)	fplan->DecWPT();
}
//-----------------------------------------------------------------
//	Initialize the waypoint
//-----------------------------------------------------------------
void CWPoint::Build(Tag t)
{ //--- Init node part ------------------------------
	nSeq      = 0;
  type      = t;
	user			= t;
  fplan     = 0;
	Modif			= 0;
	mode			= WPT_MOD_LEG;
	activ			= 0;
  position.lat = position.lon = position.alt = 0;
  altitude  = 0;
  DBwpt     = 0;
  mTxt[0]   = ' ';
  mTxt[1]   = 0;
  legDis    = 0;
	rDir			= 0;
	dDir			= 0;
	sDis			= 0;
	mDis			= 0;
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
	return;
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
//	Change Mode
//	Compute direct path to waypoint except if landing point
//	For landing, the reference direction rDir has been
//	set to the runway direction
//-----------------------------------------------------------
float CWPoint::GoDirect(CVehicleObject *veh)
{	if (IsLanding())	return rDir;
	mode = WPT_MOD_DIR;
	SVector v = {0,0,0};
  CmHead *obj	= DBwpt.Pointer();
	if (0 == obj)	return rDir;
	v	= GreatCirclePolar(veh->GetAdPosition(), obj->ObjPosition());
	double mdev = obj->GetMagDev();
	double ndir = Wrap360((float)v.h - mdev);
	return ndir;
}
//----------------------------------------------------------------------
//	Select best altitude depending on distance from previous node
//	a0 is the previous node altitude
//	Estimation is based on a 3% climbing slope
//----------------------------------------------------------------------
int CWPoint::BestAltitudeFrom(int a0)
{	float		ft = FN_FEET_FROM_MILE(legDis) *  CLIMB_SLOPE;
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
void CWPoint::SetReferenceDIR(double d)
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
	NodeEnd();
	return;	}
//-------------------------------------------------------------------------
//  Fill common parameters
//  -Refresh the list
//-------------------------------------------------------------------------
void  CWPoint::FillWPT(CmHead *obj)
{ //--- Set common parameters ----------
	State	= WPT_STA_OUT;
	SetDBwpt   (obj);
  SetName    (obj->GetName());
	SetIden    (obj->GetIdent());
  SetPosition(obj->GetPosition());
  SetDbKey   (obj->GetKey());
  SetAltitude(fplan->actCEIL());
	spot	= obj->GetPosition();
	magdv = obj->GetMagDev();
	dfeet = 0;
  return;
}
//-------------------------------------------------------------------------
//	Unassign the waypoint
//-------------------------------------------------------------------------
void CWPoint::Unassign()
{	activ = 0;
	SetDBwpt(0);
	return;
}
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
{ char *none = "    ";
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
//	Set node sequence
//--------------------------------------------------------------------
void CWPoint::SetSeq(U_SHORT s)
{	nSeq	= s;
	if (s != 1)		return;
	return;
}
//--------------------------------------------------------------------
//  Return landing data
//--------------------------------------------------------------------
ILS_DATA *CWPoint::GetLandingData()
{	if (NotAirport())	return 0;
  CAirport *apw = (CAirport*)GetDBobject();
	if (0 == apw)			return 0;
	char *key			= apw->GetKey();
	//--- Locate airport in cache ----------
  CAirport *apt = globals->dbc->FindAPTbyKey(key);
	if (0 == apt)	return 0;
	//--- Locate runway end ----------------
	CRunway  *rwy = apt->FindRunway(lndRWY);
	if (0 == rwy)					return 0;
	ILS_DATA *ils = rwy->GetLandDirection(lndRWY);
	//--- Set reference direction to runway direction --
	rDir = ils->lnDIR;
	return ils;
}
//--------------------------------------------------------------------
//  Set Landing configuration
//--------------------------------------------------------------------
bool CWPoint::EnterLanding(CRadio *rad)
{ ILS_DATA *ils = GetLandingData();
	if (0 == ils)		return false;
	//--- Set landing mode -------------
	spot	= ils->refP;
	mode	= WPT_MOD_LND;
	rad->ModeEXT(GetDBobject(),ils);
	return true;
}
//--------------------------------------------------------------------
//  Process first node
//	-Check for speed and altitude
//--------------------------------------------------------------------
void CWPoint::NodeOne(CWPoint *n)
{	//--- Check for Airport --------------------
	if (IsVisited())				return;
	if ( 'airp' != type)		return;
	ChangeAltitude(int(DBwpt->GetElevation()));
	return;
}
//-----------------------------------------------------------------
//	Second node:  Set Altitude
//	NOTE: For altitude guess a 3% slope is used in climbing
//-----------------------------------------------------------------
void CWPoint::NodeTwo(CWPoint *prv)
{	if (IsVisited())									return;
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
void CWPoint::NodeEnd()
{	if ('airp' != type)				return;
  //--- Check for landing runway ----------
  CWPoint *prv = fplan->PrevNode(this);
	//--- Set final on previous node ---------
	if (0  == prv)						return;
	if (prv->IsVisited())			return;
	//--- Compute previous altitude ----------
	int	pa    = fplan->actCEIL();
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
void CWPoint::PopulateUser()
{	CWPT *wpt = new CWPT(ANY,WPT);
  wpt->Init(Iden,&position);
	wpt->SetMGD(globals->magDEV);		
	wpt->SetNAM(Name);
	wpt->SetKey("NONE");
	wpt->SetDIS(0);
	wpt->SetNOD(this);
	DBwpt = wpt;
	return;
}
//-----------------------------------------------------------------
// Get record from database if needed
//-----------------------------------------------------------------
void CWPoint::Populate()
{	if ('uswp' == user)		return PopulateUser();
	globals->dbc->PopulateNode(this);
	char   *idn = GetIdentity();
	SetIden(idn);
	return;
}
//----------------------------------------------------------------------
//	Modify altitude
//----------------------------------------------------------------------
char* CWPoint::ModifyAltitude(int inc)
{	if (IsVisited())	return Alti;
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
//  Update mark
//----------------------------------------------------------------------
void CWPoint::UpdateMark(char m)
{	if (fplan->IsUsed())	return;
	if (0 == m)						return;
	strcpy(Mark," ");
	State		= WPT_STA_OUT;
 *Etar	  = 0;
	mode    = WPT_MOD_LEG;
	return;
}
//----------------------------------------------------------------------
//  Check for NAV
//----------------------------------------------------------------------
void CWPoint::NodeNAV(CWPoint *prv,char m)
{	activ	= 0;
	Modif	= 0;
	CmHead *obj	= DBwpt.Pointer();
	spot	= obj->GetPosition();
	magdv = obj->GetMagDev();
	//--- Update if requested ----------------
	UpdateMark(m);
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
  _snprintf(edt,12,"%.1f", legDis);
  SetDist(edt);
  return;
}
//----------------------------------------------------------------------
//	EDIT Distance
//----------------------------------------------------------------------
void CWPoint::SetLegDistance(float d)
{	legDis	= d;
  _snprintf(Dist,10,"%.1f",d);
	Dist[9]	= 0;
	return;
}
//----------------------------------------------------------------------
//	Compute total distance
//----------------------------------------------------------------------
void CWPoint::SetSumDistance(CWPoint *p0)
{	float pd	= p0->GetSumDistance();
	sDis		  = pd + legDis ;
	return;
}
//----------------------------------------------------------------------
//	Compute total distance
//----------------------------------------------------------------------
float CWPoint::GetPrevDistance()
{ CWPoint *prv = fplan->PrevNode(this);
	return (prv)?(prv->GetSumDistance()):(0);	}
//----------------------------------------------------------------------
//	Check for last node
//----------------------------------------------------------------------
bool	CWPoint::IsLast()							
{return (nSeq == fplan->Size());}
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
//	Check if we are going away
//-----------------------------------------------------------------
char CWPoint::CheckAway()
{	float pd = pDis;
	pDis		 = mDis;
	if (mDis < pd)		return 0;		// Still inside
	//--- Going away -----------------------
	State = WPT_STA_OUT;
	SetActive(0);
	return  WPT_STA_TRM;
}
//-----------------------------------------------------------------
//	Inside waypoint
//-----------------------------------------------------------------
char CWPoint::Inside()
{ double tds = fplan->TurningPoint();
	if (mDis > tds)	return CheckAway();
	//--- Waypoint is terminated now ------
	SetActive(0);
	State = WPT_STA_TRM;
	strcpy(Mark,"X");
	EditArrival();
	fplan->Refresh();
	sDis	= 0;
	return WPT_STA_TRM;
}
//-----------------------------------------------------------------
//	Outside waypoint
//	Check in which cercle are we
//  Inner circle is 2.5 nmiles radius
//-----------------------------------------------------------------
char CWPoint::Outside()
{	float lim = fplan->GetInDIS();
	if (mDis > lim) 		return 0;
	//--- we are now inside --------------
	pDis	= mDis;
	State = WPT_STA_INS;
	strcpy(Mark,"O");
	fplan->Refresh();
	return 0;
}
//-----------------------------------------------------------------
//	Update state for active waypoint
//	For active waypoint, the total distance is the aircraft distance
//	to the waypoint
//-----------------------------------------------------------------
char CWPoint::UpdateState()
{	if (0 == activ)	return WPT_STA_TRM;		// External termination
	sDis	= mDis;
	switch(State)	{
		case WPT_STA_OUT:
			return Outside();
		case WPT_STA_INS:
			return Inside();
  }
return 0;
}
//-----------------------------------------------------------------
//	Update current node
//	Compute
//  -Plane distance to waypoint
//	-Remaining distance to last waypoint
//	-Direct direction to waypoint
//-----------------------------------------------------------------
void CWPoint::UpdateRange(CVehicleObject *veh,U_INT frame)
{	//--- Compute distance from aircraft --------------
	SVector	v	= GreatCirclePolar(veh->GetAdPosition(), &spot);
  dDir		= Wrap360((float)v.h - magdv);		// Direct direction
	mDis		= (float)v.r * MILE_PER_FOOT;
  dfeet		=  v.r;
	//--- Check for a direct to waypoint --------------
	if (0 == nSeq)	rDir = dDir;
	//--- Update total distance -----------------------
	int as = fplan->GetActSequence();
	if (nSeq <  as)		sDis = 0;
	if (nSeq == as)		sDis = mDis;
	if (nSeq >  as)		sDis = GetPrevDistance() + legDis;
	return;
}

//------------------------------------------------------------
//  Save this waypoint
//------------------------------------------------------------
void CWPoint::Save(SStream *s)
{ char mrk[2] = {' ',0};
  char txt[256];
  if (IsVisited()) mrk[0] = 'X';
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
	SetIdent('-FP-');	
	mveh		= m;
  serial	= 0;
	State		= FPL_STA_NUL;
	modify	= 0;
	GPS 		= mveh->GetGPS(); 
	dWPT.SetFlightPlan(this);
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
	ClearPlan();
	serial = 0;
	win		 = 0;
	//------------------------------------------------
	mALT		= 5000;
	cALT		= 4500;
	//------------------------------------------------
	strcpy(nul,"    ");
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
    _snprintf(fn,(PATH_MAX-1),"FLIGHTPLAN/%s.FPL",Name);
    if (!pexists(&globals->pfs,fn))  return;
  }
  return;
}
//-----------------------------------------------------------------
//  Clear Flight plan
//-----------------------------------------------------------------
void CFPlan::ClearPlan()
{ Save();           // Save previous if modified
  //----- Build empty list --------------------
	U_INT type = LIST_HAS_TITLE + LIST_NOHSCROLL;
	wPoints.EmptyIt();
	wPoints.AddSlot(&head);
  wPoints.SetParameters(0,'list',type);
  GenerateName();
  strcpy (Desc,"Default flight plan");
  State			= FPL_STA_NUL;
	aWPT			= 0;
	nWPT			= 0;
	uWPT			= 0;
  Version		= 0;
  modify		= 0;
	NbWPT			= 0;
	genWNO		= 0;
 *dapt			= 0;
  return;
}
//-------------------------------------------------------------------------
//	Assign a new flight plan from file name
//-------------------------------------------------------------------------
bool CFPlan::AssignPlan(char *fn,char opt)
{ char name[PATH_MAX];
  char modif = opt ^ 1;
	_strupr(fn);
  _snprintf(name,(PATH_MAX-1),"FLIGHTPLAN/%s.FPL",fn);
  if (!pexists(&globals->pfs,name)) return false;
  ClearPlan();
  SStream s;
	WarnGPS(modif);				 // Advise GPS
	UpdatePlan();
	//--- Read plan and set loaded state ------
  if (!OpenRStream (name,s))   return false;
  option	= opt;
	format  = '0000';
	ReadFrom (this, &s);                 
  CloseStream (&s);
	strncpy(Name,fn,64);
	WarnGPS(modif);				 // Advise GPS
	return true;
}
//-----------------------------------------------------------------
// Add a new node to the plan
//-----------------------------------------------------------------
void CFPlan::AddNode(CWPoint *wpt)
{	wpt->Populate();
  //--- Link object to its node ------------
	CmHead *obj	= wpt->GetDBobject();
	obj->SetUPTR(wpt);
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
//	NOTE: the reference direction from p0 to P1 is the direction of
//				the segment p0-p1.  It is stored in p1 so that only p1 is
//				needed for navigation. The direct direction to p1 is the 
//				from actual aircraft position to p1 and may be distinct from
//				the reference direction.  When the are about the same, the 
//				aircraft is flying the segment.
//----------------------------------------------------------------------
void CFPlan::SetDistance(CWPoint *p0, CWPoint *p1)
{	SVector v = {0,0,0};
	if (0 == p0)	return p1->SetLegDistance(0);
	if (0 == p1)	return;
	//--- compute real distance ----------------
	v	= GreatCirclePolar(p0->GetGeoP(), p1->GetGeoP());
	float d = float(v.r) * MILE_PER_FOOT;
	//--- Distance from previous -----------------
	p1->SetLegDistance(d);
	p1->SetSumDistance(p0);
	//--- direction to p1 ----------------------
	double mdev = p1->GetMagDeviation();
	double rdir = Wrap360((float)v.h - mdev);
	p1->SetReferenceDIR(rdir);
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
  char  nam[64];
  GenWptName(edt);
	_snprintf(nam,26,"User %s",edt);
	CWPT *wpt = new CWPT(ANY,WPT);
	wpt->Init(edt,p);
	wpt->SetMGD(globals->magDEV);		
	wpt->SetNAM(nam);
	wpt->SetKey("NONE");
	return wpt;
}
//-------------------------------------------------------------------------
//  Create an Airport waypoint
//-------------------------------------------------------------------------
CWPoint *CFPlan::CreateAPTwaypoint(CAirport *apt)
{ CWPoint   *wpt = new CWPoint(this,'airp');
	apt->SetUPTR(wpt);
  wpt->FillWPT(apt);
  return wpt;
}
//-------------------------------------------------------------------------
//  Create a  NAVAID waypoint
//-------------------------------------------------------------------------
CWPoint *CFPlan::CreateNAVwaypoint(CNavaid *nav)
{ CWPoint   *wpt = new CWPoint(this,'snav');
	nav->SetUPTR(wpt);
  wpt->FillWPT(nav);
  return wpt;
}
//-------------------------------------------------------------------------
//  Create a DATABASE waypoint
//-------------------------------------------------------------------------
CWPoint *CFPlan::CreateWPTwaypoint(CWPT		*pnt)
{	CWPoint	*wpt = new CWPoint(this,'wayp');
	pnt->SetUPTR(wpt);
	wpt->SetUser('uswp');
	pnt->SetNOD(wpt);
  wpt->FillWPT(pnt);
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
	CWPoint *end  = 0;
	U_SHORT dim   = wPoints.GetSize();
	modify	|= m;
	nWPT		= 0;
	//---- Reorder nodes and do computation ---
	for (U_SHORT k = 1; k<dim; k++)
	{	wpt	= (CWPoint *)wPoints.GetSlot(k);
		if (0 == wpt)	break;
		wpt->SetSeq(k);
		SetDistance(prv,wpt);
		if (1 == k)	np1	= wpt;
		if (2 == k) np2 = wpt;
		wpt->NodeNAV(prv,m);
		prv	= wpt;
	}
	//---------------------------------
	WarnGPS(m);
	//--- Check for modifications -----
	if (0 == m)		return;
	if (np2)	np2->NodeTwo(np1);
	//--- Check last node -------------
	end = (CWPoint*)wPoints.LastPrimary();
	if (end) end->NodeEnd();
	return;
}
//-------------------------------------------------------------------------
//	Warn GPS if needed 
//-------------------------------------------------------------------------
void CFPlan::WarnGPS(char m)
{	//--- Compute GPS warning -----------------
	bool wn = (GPS != 0) && (m);
	if (wn) GPS->ModifiedPlan();
	return;
}
//-----------------------------------------------------------------
// Waypoint has moved
//-----------------------------------------------------------------
void CFPlan::MovedWaypoint(CWPoint *wpt)
{	Modify(1);
	CWPoint	*prv  = (CWPoint*)wPoints.PrevPrimary(wpt);
	SetDistance(prv,wpt);
	CWPoint *nxt  = (CWPoint*)wPoints.NextPrimary(wpt);
	SetDistance(wpt,nxt);
	wpt->SetModif(1);
	return;
}
//-----------------------------------------------------------------
//	Reinit flight plan and possibly save it
//	Inner distance is such that the aircarft is about 60 secondes
//	from crossing the waypoint
//-----------------------------------------------------------------
void CFPlan::UpdatePlan()
{	//--- Get values for this aircraft -------------
	CSimulatedVehicle     *svh = mveh->svh;
	nmlSPD	=  svh->GetCruiseSpeed();
	aprSPD  =  svh->GetApproachSpeed();
	mALT	  =  int(svh->GetCeiling());
	int a   =  mALT / 100;
	int b   =  mALT % 100;
	if (b)	a++;
	mALT    = 100 * a;
	cALT		=  80 * a;
	//--- Compute inner distance ------------------
	insDIS		= (nmlSPD * 60) / 3600;
	return;
}
//-----------------------------------------------------------------
// Get next node from current node
//-----------------------------------------------------------------
CWPoint *CFPlan::NextNode(CWPoint *w)
{	CSlot *s = (w)?(wPoints.NextPrimary(w)):(wPoints.HeadPrimary());
	return (CWPoint*)s;	}
//-----------------------------------------------------------------
// Get previous node from current node
//-----------------------------------------------------------------
CWPoint *CFPlan::PrevNode(CWPoint *w)
{	CSlot *s = (w)?(wPoints.PrevPrimary(w)):(0);
  return (CWPoint *)s;}
//-----------------------------------------------------------------
// Check node existence
//-----------------------------------------------------------------
bool CFPlan::Exist(int No)
{	if (No <     1)				return false;
	if (No > (int)NbWPT)	return false;
	return true;
}
//----------------------------------------------------------------------
//	We have to define the notion of a terminated waypoint
//	We say that Waypoint is terminated when it is time to turn to the next
//	We may then compute the distance from the waypoint to define the
//	termination radius.
//	We base our estimation on a standard rotational speed of 2 minutes 
//	for a complete turn of 360 deg 
//	It is expected that the angle between 2 waypoint rarely exceed 90°.
//	So, for a 90° turn, 15 seconds are expected (0.004166... hour)
//	Now we used a 1.2 coefficient for safety thus the ratio is
//	R = (0.004166) * 1.2 = 0.005
//
//	With S = (speed) we have the distance D = S * R in miles 
//	When aircraft if at this distance of the NAV station, then we
//	say that the waypoint is terminated
//----------------------------------------------------------------------
double CFPlan::TurningPoint()
{	double spd = mveh->GetPreCalculedKIAS();
	return (spd * 0.005);
}
//----------------------------------------------------------------------
//	Modify ceil
//----------------------------------------------------------------------
int CFPlan::ModifyCeil(int inc)
{	modify = 1;
	cALT += inc;
	if (cALT < 500)		cALT = 500;
	if (cALT > mALT)	cALT = mALT;
	return cALT;
}
//-----------------------------------------------------------------
// Assign a direct to waypoint
//-----------------------------------------------------------------
void CFPlan::AssignDirect(CmHead *obj)
{	dWPT.FillWPT(obj);
	ActivateNode(&dWPT);
	return;
}
//-----------------------------------------------------------------
// Clear Direct mode
//-----------------------------------------------------------------
void CFPlan::ClearDirect()
{	dWPT.SetActive(0);
	dWPT.SetDBwpt(0);
	return;
}
//-----------------------------------------------------------------
// Return previous waypoint identity
//	Called from GPS when active waypoint is changed
//-----------------------------------------------------------------
char *CFPlan::PreviousIdent(CWPoint *wpt)
{	if (0 == wpt)						return nul;
	if (wpt->NotFromFPL())	return nul;
	CWPoint *prv = PrevNode(wpt);
	if (0 == prv)						return nul;
	return prv->GetIdentity();
}
//-----------------------------------------------------------------
//	Update parameters
//	Each node is updated relative to aircraft position:
//	-Node state
//	-Remaining distance
//	-Direction for direct access to the waypoint
//	NOTE:  Direct Waypoint outside of a flight plan may exist
//	 any time.  Thus they are updated even when there is not flight
//	 plan, or the FP is inactive
//-----------------------------------------------------------------
//	NOTE when a direct TO waypoint is terminated then the next waypoint 
//			is:
//	Same waypoint if the waypoint is not part of the flight plan
//	Otherwise, the next waypoint from the flight plan or 0
//
//-----------------------------------------------------------------
void	CFPlan::TimeSlice(float dT, U_INT frm)
{	char ret = 0;
	char nw  = mveh->NbWheelsOnGround();
  //--- Update non FP direct TO waypoint ------
	UpdateDirectNode(frm);
	//--- Update according to current state -----
	switch (State)	{
		//--- No flight plan loaded. Update departing airport ---
		case FPL_STA_NUL:
			if (nw)	strcpy(dapt,globals->apm->NearestIdent());
			return;
		//--- Flight plan operational -------------
		case FPL_STA_OPR:
			//--- Check for an empty flight plan ----
			if (IsEmpty())		return;
			uWPT	= (CWPoint*)wPoints.NextPrimary(uWPT);
			//--- Recycle plan if needed ------------
			if (0 == uWPT)	
			{ uWPT = (CWPoint*)wPoints.HeadPrimary();
			  nWPT	= uWPT;}
			if (0 == uWPT)		return;
			//--- Update one waypoint ----------------
			uWPT->UpdateRange(mveh,frm);
			SaveNearest(uWPT);
			//--- Now update active waypoint --------
			UpdateActiveNode(frm);
			return;
	}
}
//-----------------------------------------------------------------
//	Update Direct node (this is for GPS)
//-----------------------------------------------------------------
void CFPlan::UpdateDirectNode(U_INT frm)
{	if (aWPT != &dWPT)			return;
	if (aWPT->IsFromFPL())	return;
	UpdateActiveNode(frm);
	return;
}
//-----------------------------------------------------------------
//	Update Active node
//-----------------------------------------------------------------
void CFPlan::UpdateActiveNode(U_INT frm)
{	aWPT->UpdateRange(mveh,frm);
	//---------------------------------------
	if (0 == aWPT->UpdateState())			return;
	//--- Waypoint is terminated ------------
	CWPoint *nxt = NextStep(aWPT);
	ActivateNode(nxt);
	return;
}
//-----------------------------------------------------------------
//	Activate this way point
//-----------------------------------------------------------------
void CFPlan::ActivateNode(CWPoint *nxt)
{	//--- First process current waypoint termination ---
	endir			= (aWPT == &dWPT) && 	(nxt != &dWPT);
	if (aWPT)	aWPT->SetActive(0);
	//--- Free the direct waypoint ---------------------
	if (endir) dWPT.Unassign();
	//--- Now activate the new one ---------------------
	aWPT	= nxt;
	if (nxt)	nxt->SetActive(1);
	if (GPS)	GPS->ActiveWaypoint(nxt,endir);
	if (0 == nxt)		return Stop();
	nxt->SetModif(0);
	return;
}
//-----------------------------------------------------------------
//	Retrieve next waypoint from flight plan
//-----------------------------------------------------------------
CWPoint *CFPlan::NextStep(CWPoint *wpt)
{	//--- Check if this is a direct To waypoint -----
	if (wpt->NotFromFPL()) return wpt;			// Same waypoint
	//--- Get the original waypoint -----------------
	CWPoint *org = wpt->GetOrgWPT();
	return (CWPoint*)wPoints.NextPrimary(org);
}
//-----------------------------------------------------------------
//	Compute Base Waypoint from flight plan
//	Base WPT is the one preceding the active Waypoint
//	if the waypoint is from Flight Plan
//-----------------------------------------------------------------
CWPoint *CFPlan::BaseWPT(CWPoint *b)
{	//--- Outside WPT return current base -------
	if (b->NotFromFPL())	return b;
	if (b->IsFirst())			return b;
	//--- Original waypoint ---------------------
	CWPoint *org = b->GetOrgWPT();
	return  PrevNode(org);
}
//-----------------------------------------------------------------
//	Restore a waypoint from plan
//	Find the nearest waypoint not already visited
//-----------------------------------------------------------------
void  CFPlan::RestoreNode()
{	CWPoint *nrp = nWPT;				// Nearest waypoint
	int      seq = NbWPT - 1;		// Next to last
	while (nrp)
	{	if (nrp->GetSequence() == seq)	break;
		if (!nrp->IsVisited())					break;
		nrp	= NextNode(nrp);
	}
	ActivateNode(nrp);
	return;
}
//-----------------------------------------------------------------
//	Probe Flight Plan
//-----------------------------------------------------------------
void CFPlan::Probe(CFuiCanva *cnv)
{ if (0 == aWPT)		return;
	cnv->AddText( 1,1,"aWPT: %s",aWPT->GetIdentity());
	cnv->AddText( 1,1,"mDis: %.2fnm",aWPT->GetPlnDistance());
	cnv->AddText( 1,1,"State: %d",aWPT->GetState());
	return;
}
//-----------------------------------------------------------------
// Check for validity
//-----------------------------------------------------------------
int CFPlan::CheckError()
{	//--- There must be at least 3 nodes --------------
	if (dWPT.IsActive())			return 0;
	if (3 > NbWPT)						return 1;
	return 0;
}
//-----------------------------------------------------------------
//	Activate the flight plan
//	Check for completness
//-----------------------------------------------------------------
int	CFPlan::ActivatePlan()
{	UpdatePlan();							// Values from aircraft
	if (FPL_STA_OPR == State)	return 1;
	Reorder(1);
	if (CheckError())					return 0;
	//--- Fligth plan is operational ---
	endir	= false;
	State = FPL_STA_OPR;
	uWPT	= 0;
	nWPT	= 0;
	if (dWPT.IsActive())			return 1;
	aWPT	= (CWPoint*)wPoints.HeadPrimary();
	dWPT.Unassign();
	return 1;
}
//-----------------------------------------------------------------
//	Return a node type related to active state
//	0=> Forward waypoint or past terminated
//	1=> From Waypoint
//	2=> To   waypoint
//-----------------------------------------------------------------
int CFPlan::NodeType(CWPoint *wp)
{	if (Inactive())				return 0;		// Space
	//-----------------------------------------
	int asq = aWPT->GetSequence();
	int psq = asq - 1;
	int csq = wp->GetSequence();
	if (csq == asq)				return 2;			// TO waypoint
	if (csq == psq)				return 1;			// FROM waypont
	//-- Check for direct waypoint ------------
	if (asq)							return 0;			// Just any waypoint
	CWPoint *org	= aWPT->GetOrgWPT();
	if (org == wp)				return 2;			// TO waypoint
	return 0;		}												// any waypoint
//-----------------------------------------------------------------
//	DeActivate the flight plan
//	Check for completness
//-----------------------------------------------------------------
void CFPlan::Stop()
{	State = FPL_STA_NUL;
	ClearDirect();
	aWPT	= 0;	}
//-----------------------------------------------------------------
//	Return airport departing key
//-----------------------------------------------------------------
char *CFPlan::GetDepartingKey()
{	char *none		= "NONE";
	CWPoint *dep	= (CWPoint *)wPoints.HeadPrimary();
	if (0 == dep)	return none;
	return dep->GetDbKey();	}
//-----------------------------------------------------------------
//	Return airport departing Ident
//-----------------------------------------------------------------
char *CFPlan::GetDepartingIDN()
{	char *none		= "NONE";
	CWPoint *dep	= (CWPoint *)wPoints.HeadPrimary();
	if (0 == dep)	return none;
	return dep->GetIdentity();	}

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
{	if (0 == aWPT)					return false;
  if (aWPT->NotAirport())	return false;
	return (int(NbWPT) == aWPT->GetSequence());
}
//-----------------------------------------------------------------
//	Save nearest waypoint
//-----------------------------------------------------------------
void CFPlan::SaveNearest(CWPoint *wpt)
{	float d = wpt->GetPlnDistance();
  if (d > nWPT->GetPlnDistance())	return;
	nWPT	= wpt;
	return;
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
//    rOrg.SetPosition(org->GetGeoP());
    if (0 == ext) break;
		rExt.SetNode(ext);
//    rExt.SetPosition(ext->GetGeoP());
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
	_strupr(Name);
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
  _snprintf(name,(PATH_MAX-1),"FLIGHTPLAN/%s.FPL" ,Name);
  _snprintf(renm,(PATH_MAX-1),"FLIGHTPLAN/%s.BAK",Name);
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
	WriteTag('ceil', "-----------Average ceil ----------", &s);
	WriteInt(&cALT,&s);
  //---Save individual Waypoint ------------------------
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
//-------------------------------------------------------------
//	return position
//-------------------------------------------------------------
SPosition *VMnode::ObjPosition()
{return wpt->GetGeoP();}
//-------------------------------------------------------------
//	return database object
//-------------------------------------------------------------
CmHead *VMnode::GetOBJ()
{	return wpt->GetDBobject();	}
//-------------------------------------------------------------
//	Set Node distance  
//-------------------------------------------------------------
void VMnode::SetNodeDistance()
{	CmHead *obj = wpt->GetDBobject();
  if (obj->IsNot(WPT))	return;
	float d = wpt->GetLegDistance();
	CWPT   *nod = (CWPT*) obj;
	nod->SetDIS(d);
	return;
}
//=================END OF FILE ==================================================