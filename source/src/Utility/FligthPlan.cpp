/*
 * Flight.cpp
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

#include "../Include/Globals.h"
#include "../Include/FlightPlan.h"
#include "../Include/FuiParts.h"
#include "../Include/FuiUser.h"
#include  <stdio.h>
using namespace std;
//====================================================================
//  Global flight plan title
//====================================================================
CFlpLine        fpTITLE;                  // Title
//=======================================================================
//  Fill parameters to compute distance
//=======================================================================
void CRouteEXT::SetPosition(SPosition *p)
{ loc         = *p;
  double lr   = DegToRad (loc.lat / 3600.0);      // Latittude in Radian
  nmFactor    = cos(lr) / 60;                     // 1 nm at latitude lr
}
//=======================================================================
//  Read route parameters
//=======================================================================
//-------------------------------------------------------------------
//  Read Tags
//-------------------------------------------------------------------
int CFpWaypointRoute::Read (SStream *stream, Tag tag)
{ float nf;
  int rc = TAG_IGNORED;
  char s[PATH_MAX];

  switch (tag) {
  case 'path':
    // Read leg path type
    ReadString (s, PATH_MAX, stream);
    rc = TAG_READ;
    break;
  case 'term':
    // Read task terminator
    ReadString (s, PATH_MAX, stream);
    rc = TAG_READ;
    break;
  case 'cHdg':
    // Read course heading (degrees)
    ReadFloat (&nf, stream);
    rc = TAG_READ;
    break;
  case 'cRdl':
    // Read course radial
    ReadFloat (&nf, stream);
    rc = TAG_READ;
    break;
  case 'cDME':
    // Read course DME distance
    ReadFloat (&nf, stream);
    rc = TAG_READ;
    break;
  case 'tAlt':
    // Read termination altitude
    ReadFloat (&nf, stream);
    rc = TAG_READ;
    break;
  case 'tRdl':
    // Read termination radial
    ReadFloat (&nf, stream);
    rc = TAG_READ;
    break;
  case 'tDME':
    // Read termination DME distance
    ReadFloat (&nf, stream);
    rc = TAG_READ;
    break;
  default:
    // Generate warning
    WARNINGLOG ("CFpWaypointRoute::Read unknown tag %s", TagToString(tag));
  }

  return rc;
}
//====================================================================
// CWayPoint
//====================================================================
CWayPoint::CWayPoint (CFlightPlan *fp)
{ Seq       = 0;
  type      = 0;
  fplan     = fp;
  position.lat = position.lon = position.alt = 0;
  altitude  = 0;
  DBwpt     = 0;
  mTxt[0]   = ' ';
  mTxt[1]   = 0;
  Legdis    = 0;
  disTot    = 0;
  rDIR      = 0;
  wDIR      = 0;
  ClearDate(&artime);
  elapse.dYears   = 0;
  elapse.dMonths  = 0;
  elapse.dDays    = 0;
  elapse.dHours   = 0;
  elapse.dMinutes = 0;
  elapse.dSeconds = 0;
  elapse.dMillisecs = 0;
  State     = OTS;
}
//-------------------------------------------------------------------
//  Note: Object pointed by DBwpt is explicitly deleted here
//--------------------------------------------------------------------
CWayPoint::~CWayPoint (void)
{ DBwpt = 0;
}
//-------------------------------------------------------------------
//  Read tags
//  NOTE: Route is no longer used
//-------------------------------------------------------------------
int CWayPoint::Read (SStream *stream, Tag tag)
{ int i;
  CFpWaypointRoute   route;        // Routing data to next waypoint
  switch (tag) {
  case 'type':
    ReadTag (&type, stream);
    if (type == 'sarw') {sqTAB = "APT"; sqTAG = 'APT*';}
    if (type == 'snav') {sqTAB = "NAV", sqTAG = 'NAV*';}
    if (type == 'wywy') {sqTAB = "WPT", sqTAG = 'WPT*';}
    return TAG_READ;

  case 'name':
    // Read waypoint name
    ReadString (name, 40, stream);
    return TAG_READ;

  case 'llps':
    // Read latitude/longitude position
    {
      SVector v;
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
    ReadFloat (&altitude, stream);
    return TAG_READ;

  case 'sped':
    // Read speed
    ReadFloat (&Speed, stream);
    return TAG_READ;

  case 'rout':
    // Read routing data to next waypoint
    ReadFrom (&route, stream);
    return TAG_READ;

  case 'warp':
    // Read whether this waypoint is a warp gate
    ReadInt (&i, stream);
    return TAG_READ;
    // Read termination marker ---------------
  case 'mark':
    ReadString(mTxt,2,stream);
    if (mTxt[0] != 'X') return TAG_READ;
    State = TRM;
    return TAG_READ;

  default:
    // Generate warning
    WARNINGLOG ("CWayPoint::Read unknown tag %s", TagToString(tag));
  }

  return TAG_IGNORED;
}
//-----------------------------------------------------------------
//  Clear date
//-----------------------------------------------------------------
void CWayPoint::ClearDate (SDateTime *sd)
{ sd->date.day     = 0;
  sd->date.month   = 0;
  sd->date.year    = 0;
  sd->time.hour    = 0;
  sd->time.minute  = 0;
  sd->time.msecs   = 0;
  sd->time.second  = 0;
  return;
}
//--------------------------------------------------------------------
//  Check if same Waypoint
//--------------------------------------------------------------------
bool CWayPoint::SameWPT (CmHead *obj)
{ CmHead *mdb = DBwpt.Pointer();
  if (0 == mdb)                                   return false;
  if (0 == obj)                                   return false;
  if (obj->GetActiveQ() !=  mdb->GetActiveQ())    return false;
  if (strcmp(obj->GetName(),mdb->GetName()) != 0) return false;
  return true;
}
//-----------------------------------------------------------------
//  Print waypoint
//-----------------------------------------------------------------
void CWayPoint::Print (FILE *f)
{
  fprintf (f, "  Type      : %s\n", TagToString(type));
  fprintf (f, "  Name      : %s\n", name);
  fprintf (f, "  DB Key    : %s\n", dbKey);
  char s[80];
  FormatPosition (position, s);
  fprintf (f, "  Position  : %s\n", s);
  fprintf (f, "  Altitude  : %f\n", altitude);
  fprintf (f, "  Speed     : %f\n", Speed);
  fprintf (f, "  Route:\n");
  fprintf (f, "\n");
}
//--------------------------------------------------------------------
//  Return waypoint identity
//--------------------------------------------------------------------
char *CWayPoint::GetIdentity()
{ char *none = "----";
  if (DBwpt.IsNull()) return none;
  return DBwpt->GetIdent();
}
//-----------------------------------------------------------------
//  Reserve database object
//-----------------------------------------------------------------
void CWayPoint::SetDBwpt (CmHead *obj)  
{ DBwpt = obj; 
  return;
}
//-----------------------------------------------------------------
//  Return Arrival time
//-----------------------------------------------------------------
void CWayPoint::GetArrivalTime (U_INT *hh, U_INT *mn)
{ *hh = artime.time.hour;
  *mn = artime.time.minute;
  return;
}
//-----------------------------------------------------------------
//  Return arrival day and month
//-----------------------------------------------------------------
void CWayPoint::GetArrivalDay (U_INT *dd,U_INT *mo)
{ *dd = artime.date.day;
  *mo = artime.date.month;
  return;
}
//-----------------------------------------------------------------
//  Return leg duration
//-----------------------------------------------------------------
void CWayPoint::GetLegDuration (U_INT *dd,U_INT *hh, U_INT *mn)
{ *dd = elapse.dDays;
  *hh = elapse.dHours;
  *mn = elapse.dMinutes;
  return;
}
//-----------------------------------------------------------------
//  Compute elapse time
//-----------------------------------------------------------------
void CWayPoint::SetElapse (float dis)
{ float hh  = (dis / Speed);
  float sec = hh * 3600;
  elapse    = globals->tim->SecondsToDateTimeDelta(sec);
  return;
}
//--------------------------------------------------------------------
//  Update speed and altitude in far nodes
//  When on ground, 
//      Estimated speed is taken from the svh file and
//      departure time is updated with current time
//  When in air:
//      Real speed is used and if the waypoint is the active one
//      the departure time is not updated.
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//  Set speed and altitude according to node position
//  -Start node:    speed = 0               Altitude = waypoint
//  -Middle nodes:  speed = default         Altitude = default
//  -End    node:   speed = 0;              Altitude = waypoint
//---------------------------------------------------------------------
void CWayPoint::UpdateValue ()
{ //---Dont update terminated points -----------------------
  if (TRM == State) return;
  //----- Compute depart time for this waypoint -----------
  SDateTime dep   = fplan->GetLegDepTime();  // leg depart time
  char      act   = fplan->IsActive(this);
            act  &= fplan->AirBorn();
  if (act)  dep   = globals->tim->GetUTCDateTime();  // Time is now
  //-----Update Speed -------------------------------------
  Speed       = fplan->GetSpeed();
  //-----Update altitude ----------------------------------
  altitude    = fplan->GetAltitude();
  if (fplan->IsLast (this))  altitude = position.alt;
  if (fplan->IsFirst(this))  altitude = position.alt;
  //----Compute arrival time ------------------------------
  SetElapse(Legdis);
  U_INT     day = dep.date.day;
  artime        = globals->tim->AddTimeDelta (dep, elapse);
  dptime        = dep;
  nxDAY         = (day == artime.date.day)?(' '):('*');
  fplan->SetLegDepTime(artime);
  return;
}
//-----------------------------------------------------------------
//  Terminate route
//  Log data for futur use
//  -Altitude
//  -Arrival time
//-----------------------------------------------------------------
bool CWayPoint::TerminateRoute ()
{ float alt = globals->geop.alt;
  mTxt[0]   = 'X';
  SetAltitude(alt);
  artime  = globals->tim->GetUTCDateTime();
  return true;
}
//-----------------------------------------------------------------
//  Activate route
//-----------------------------------------------------------------
void CWayPoint::ActivateRoute()
{ mTxt[0]   = ' ';
  State     = OTS;
  return;
}
//-----------------------------------------------------------------
//  Refresh waypoint data
//-----------------------------------------------------------------
bool CWayPoint::Refresh(U_INT fr)
{ CmHead *obj     =  GetDBobject();
  if (0 == obj)       return 0;
  obj->Refresh(fr);
  rDIR    = obj->GetRadial();
  disPLN  = obj->GetNmiles();
  return 1;
}
//-----------------------------------------------------------------
//  Compute Leg distance from previous Waypoint
//-----------------------------------------------------------------
void CWayPoint::ComputeLeg(CWayPoint *prv)
{ SVector v = {0,0,0};
  if (prv)
    { SPosition from = prv->GetPosition();
      v	             = GreatCirclePolar(&from, &position);
      Legdis         = (float)v.r * MILE_PER_FOOT;
      disTot         = Legdis + prv->GetTotalDis();
    }
  else  
    { Legdis  = disPLN;
      disTot  = Legdis;
    }
  //---Set route to this waypoint -----------------------
  float dev = (DBwpt.Assigned())?(DBwpt->GetMagDev()):(0);
  wDIR      = (prv)?(Wrap360(v.h - dev)):(rDIR);
  return;
}
//-----------------------------------------------------------------
//  Update  waypoint state
//
//  -Check if terminated waypoint
//  Return true when terminated
//  NOTE:  Current active waypoint is the next to be crossed
//        Total distance is reset to  0 so that GPS can show
//        the remaining distance to destination.
//-----------------------------------------------------------------
void CWayPoint::UpdateState ()
{ float miles = disPLN;           // Distance to aircraft
  switch (State)  {
  //---Outside: Check if entering inner circle -------------
  case OTS:
    if (miles > 4)                return; 
    State = INS;                  // Entering inner waypoint
    Inner = miles;                // Distance to aircraft
    oMrk      = mTxt[0];          // save mark
    mTxt[0]   = 'i';
    return;
  //---Inside:  Check if leaving inner circle --------------
  case INS:
    { float prev = Inner;
      Inner   = miles;            // Inner = distance to aircraft
      if (miles <= prev)          return;   // Still inside
      State   = OTS;                        // going out
      mTxt[0] = oMrk;                       // Restore mark
      if (!fplan->IsActive(this)) return;   // Not the active node 
      //--Active waypoint is terminated --------------------
      State   = TRM;
      Legdis  = 0;
      disTot  = 0;
      TerminateRoute();
      fplan->TerminateWaypoint();           // Terminating event
      return;              
    }
  //---Terminated: revisited ???--
  case TRM:
    return;           // Not a new event
  }
  return;
}
//-----------------------------------------------------------------
//  Update first node
//-----------------------------------------------------------------
void CWayPoint::UpdateFirstNode()
{ //--- if this is an airport and we are on ground
  //    then this point is terminated 
  if (type != 'sarw') return;
  //--- Compute distance to airport ------------------
  SVector	v	      = GreatCirclePolar(&globals->geop, &(this->position));
  float miles     = (float)v.r * MILE_PER_FOOT;
  if (miles > 0.5)    return;
  //--- Set parameters for termination --------------
  State = INS;
  Inner = 0;
  return;
}
//-----------------------------------------------------------------
//  Compute distance to next node
//  Compute DTK (desired track between waypoints)
//-----------------------------------------------------------------
void CWayPoint::UpdateDistance(CFlpLine *pln)
{ float dev  = 0;           // Magnetic deviation
  float tot  = 0;
  bool act = fplan->IsActive(this);
  //-- For active waypoint, origin is aircraft ------
  if  (act) 
  { Legdis  = disPLN;
    disTot  = Legdis;
    return;
  }
  //--- For future waypoint, distance is inter waypoint  
  CWayPoint   *prev = pln->GetWPT();
  disTot            = Legdis + prev->GetTotalDis();
  return;
}
//-----------------------------------------------------------------
//  Update this waypoint
//  -Compute aircraft distance to this waypoint
//  -Compute direct radial from aircraft to this waypoint
//-----------------------------------------------------------------
void CWayPoint::UpdateNode (CFlpLine *pln,U_INT frame)
{ //--- Skip if waypoint is terminated -----------------------
  if (IsTerminated())       return;
  //--- Update this waypoint ---------------------------------
  CmHead *obj     =  DBwpt.Pointer();
  if (obj)
  { obj->Refresh(frame);
    rDIR    = obj->GetRadial();
    disPLN  = obj->GetNmiles();
  }
  //---------Update waypoint state ---------------------------
  UpdateDistance(pln);
  UpdateState();
  UpdateValue();
  return;
}
//------------------------------------------------------------
//  Save this waypoint
//------------------------------------------------------------
void CWayPoint::Save(SStream *s)
{ char mrk[2] = {' ',0};
  char txt[256];
  if (IsTerminated()) mrk[0] = 'X';
  WriteTag('wpnt', "--------- Waypoint ---------------", s);
  TagToString(txt,type);
  WriteString(txt,s);
  WriteTag('bgno', "========== BEGIN OBJECT ==========", s);
  WriteTag('name', "---------- Waypoint name -----", s);
  WriteString(name,s);
  WriteTag('llps', "---------Lat/lon,alt position ", s);
  WritePosition(&position,s);
  WriteTag('dbky', "---------Database key --------", s);
  WriteString(dbKey,s);
  WriteTag('type', "---------Waypoint type--------", s);
  WriteString(txt,s);
  WriteTag('altd', "---Altitude (feet) at WPT-----", s);
  WriteFloat(&altitude,s);
  WriteTag('sped', "---Speed (knots)   at WPT-----", s);
  WriteFloat(&Speed,s);
  WriteTag('mark', "---Terminated mark- ----------", s);
  WriteString(mrk,s);
  WriteTag('endo', "========== END OBJECT ============", s);

  return;
}
//=================================================================
// CFlightPlan: 
//  Compute expected start time as local time
//  NOTE:  Option 0 is the official Flight Plan manager
//         Other options are just used for partial parsing to decode
//         name for the list of FPL.
//==================================================================
CFlightPlan::CFlightPlan (char opt)
{ //TRACE ("const %d", opt);
  serial  = 0;
  option  = opt;
  modify  = 0;
  strcpy(Name,"NONE");
  Speed   = 0;
  fTime   = 0;
  FplDT   = globals->tim->GetUTCDateTime();
  LegDT   = FplDT;
  wPoints = 0;
  //---------------------------------------------------------------
  tLIN    = 0;
  aLIN    = 0;
  eLIN    = 0;
  pLIN    = 0;
  uLIN    = 0;
  if (0 == opt) Init();
}
//-----------------------------------------------------------------
//  Init the real flight plan
//-----------------------------------------------------------------
void CFlightPlan::Init()
{ globals->fpl  = this;
  //TRACE ("init %p", globals->fpl);
  wPoints       = new CListBox();
  //---Init title ---------------------------------
  fpTITLE.FixeIt();
  fpTITLE.SetName("Waypoint name");
  fpTITLE.SetMark("X");
  fpTITLE.SetIden("Ident");
  fpTITLE.SetDist("Dis (nm)");
  fpTITLE.SetSped("Speed(Kt)");
  fpTITLE.SetRadi("Course");
  fpTITLE.SetAlti("Alti(ft)");
  fpTITLE.SetElap("Elapse");
  fpTITLE.SetEtar("Arrival");
  //-----------------------------------------------
  Clear(0);
  return;
}

//-----------------------------------------------------------------
//  Destroy the flight plan
//-----------------------------------------------------------------
CFlightPlan::~CFlightPlan (void)
{ //TRACE ("dest %d %p", option, globals->fpl);
  if (wPoints)  SAFE_DELETE (wPoints);
  if (0 == option)
  { globals->fpl = 0;
    Save();
    Clear(0);
  }
}
//-----------------------------------------------------------------
//  Add waypoint node
//-----------------------------------------------------------------
void CFlightPlan::AddNode(CWayPoint *wp)
{ CFlpLine *slot = new CFlpLine();
  slot->SetWPT(wp);
  slot->Edit();
  //---Add a new slot ---------------------
  wPoints->AddSlot(slot);
  return;
}
//-----------------------------------------------------------------
//  Read all tags
//-----------------------------------------------------------------
int CFlightPlan::Read (SStream *stream, Tag tag)
{ int nbr;
  switch (tag) {
  case 'desc':
    ReadString(Desc,128,stream);
    Desc[128] = 0;
    return (1 == option)?(TAG_EXIT):(TAG_READ);
  case 'vers':
    ReadInt((int*)(&Version),stream);
    return TAG_READ;
  case 'indx':
    if (option) return TAG_EXIT;
    // Read initial waypoint index
    ReadInt ((int*)(&nbr), stream);
    return TAG_READ;

  case 'wpnt':
    // Read flight plan waypoint sub-object
    { if (option) return TAG_EXIT;
      Tag type;
      ReadTag (&type, stream);
      CWayPoint *wp = new CWayPoint(this);
      ReadFrom (wp, stream);
      AddNode(wp);
    }
    return TAG_READ;

  default:
    // Generate warning
    WARNINGLOG ("CFlightPlan::Read unknown tag %s", TagToString(tag));
  }
  return TAG_IGNORED;
}
//-----------------------------------------------------------------
//  FILE AND FLIGHT PLAN MANAGEMENT
//-----------------------------------------------------------------
//-----------------------------------------------------------------
//  Set initial state according to number of nodes
//  a full flight plan must include at least 2 nodes
//-----------------------------------------------------------------
void CFlightPlan::InitialState()
{ int nbr = wPoints->GetSize();
  if (nbr < 3)      return;
  State = FPLAN_LOAD;
  return;
}
//-----------------------------------------------------------------
//  Clear Flight plan
//-----------------------------------------------------------------
void CFlightPlan::Clear(char m)
{ Modify(m);
  Save();           // Save previous if modified
  if (wPoints)  wPoints->EmptyIt();
  if (wPoints)  wPoints->AddSlot(&fpTITLE);
  nWPT.clear();
  GenerateName(Name);
  strcpy (Desc,"Default flight plan");
  State    = FPLAN_NULL;
  Version  = 0;
  modify   = 0;
  serial++;
  return;
}
//--------------------------------------------------------------------
//  Recycle flight plan scanning due to modification
//  The waypoint lis is reallocated during first tour
//  to account for added, removed or displaced waypoints
//--------------------------------------------------------------------
void CFlightPlan::Recycle()
{  modify = 1;
   serial++;
   Save();
   State = FPLAN_NULL;
   InitialState();
   //---Delete waypoint list --------------------
   nWPT.clear();
   return;
}
//-----------------------------------------------------------------
//  Reset Flight plan:  Remove termination mark
//-----------------------------------------------------------------
void CFlightPlan::Reset()
{ int end = wPoints->GetSize();
  for (int k=1; k<end; k++)
  { CFlpLine  *lin = (CFlpLine*)wPoints->GetSlot(k);
    CWayPoint *wpt = lin->GetWPT();
    wpt->ActivateRoute();
  }
  return Recycle();
}
//-----------------------------------------------------------------
//  Get default name
//-----------------------------------------------------------------
void CFlightPlan::GenerateName(char *n)
{ char  fn[PATH_MAX];
  short yy = globals->clk->GetYear();
  short mm = globals->clk->GetMonth();
  short dd = globals->clk->GetDay();
  bool  go = true;
  U_INT sr = 0;
  while (go)
  { _snprintf(n,64,"FP%04d%02d%02d-%03d",yy,mm,dd,sr);
    _snprintf(fn,(PATH_MAX-1),"FlightPlan/%s.FPL",n);
    if (!pexists(&globals->pfs,fn))  return;
    sr++;
  }
  return;
}
//-----------------------------------------------------------------
//  Open a new flight Plan
//-----------------------------------------------------------------
void CFlightPlan::Open(char *fname)
{ char name[PATH_MAX];
  _snprintf(name,(PATH_MAX-1),"FlightPlan/%s.fpl",fname);
  if (!pexists(&globals->pfs,name)) return;
  Clear(0);
  SStream s;
  strncpy (s.filename,name, PATH_MAX);
  strcpy (s.mode, "r");
  if (OpenStream (&s))
  { ReadFrom (this, &s);                 
    CloseStream (&s);
  }
  strncpy(Name,fname,63);
  Name[63]  = 0;
  return;
}
//-----------------------------------------------------------------
//  Set name from the stream file
//-----------------------------------------------------------------
void CFlightPlan::SetName(SStream *s)
{ //---Init description --------------------
  char fn[64];
  strncpy(fn,s->filename,64);
  fn[63] = 0;
  _snprintf(Desc,128,"Flight Plan from file %s",fn);
  //---Init file name -----------------------
  char *deb = strrchr(s->filename,'/');
  if (0 == deb)       return;
  deb++;
  strncpy(Name,deb,63);
  Name[63]  = 0;
  char *end = strrchr(Name,'.');
  if (end) *end = 0;
  return;
}
//-----------------------------------------------------------------
//  Read the flight plan from the situation file
//  A name is derived from the situation file
//-----------------------------------------------------------------
void CFlightPlan::ReadPlan(SStream *s)
{ Clear(0);
  //---- Compute a name for fplan ----------
  SetName(s);
  ReadFrom (this, s); 
  return;
}

//--------------------------------------------------------------------
//  Check for fligh plan start
//--------------------------------------------------------------------
void CFlightPlan::StartFPlan ()
{ State   = FPLAN_TOUR;
  FplDT   = globals->tim->GetUTCDateTime();
  LegDT   = FplDT;
  Num     = 0;
  //---Init all nodes --------------------------
  pLIN    = 0;
  uLIN    = (CFlpLine*)wPoints->HeadPrimary();
  eLIN    = (CFlpLine*)wPoints->LastPrimary();
  tLIN    = 0;
  aLIN    = uLIN;
  return;
}
//---------------------------------------------------------------------
//  Print the flight plan
//---------------------------------------------------------------------
void CFlightPlan::Print (FILE *f)
{ fprintf (f, "\n");
  fprintf (f, "Flight Plan:\n");
  fprintf (f, "Num Wpts    : %d\n", nWPT.size());
  int iWpt = 0;
  std::vector<CWayPoint*>::iterator i;
  for (i=nWPT.begin(); i!=nWPT.end(); i++) {
    fprintf (f, "Waypoint #%d\n", iWpt++);
    (*i)->Print (f);
    fprintf (f, "\n");
  }
}
//---------------------------------------------------------------------
//  INTERNAL UPDATES
//---------------------------------------------------------------------
//-------------------------------------------------------------
//  Terminate current waypoint and activate next 
//-------------------------------------------------------------
void CFlightPlan::TerminateWaypoint()
{ Modify(1);
  tLIN  = uLIN;               // Terminated waypoint
  //---Mark all previous waypoints are terminated -------
  CWayPoint *wpt = tLIN->GetWPT();
  int No  =  wpt->GetSeq();
  while (No-- > 0)
  {  wpt = nWPT[No];
     if (wpt->IsTerminated()) break;
     wpt->TerminateRoute();
  }
  //---Get the next waypoint as active waypoint ---------
  if (tLIN == eLIN)           return;
  aLIN  = (CFlpLine *)wPoints->NextPrimary(uLIN);
  wptActive(aLIN);
  return;
}
//-------------------------------------------------------------
//  Refresh every waypoint line
//-------------------------------------------------------------
void CFlightPlan::Refresh(CFuiFlightLog *win)
{ CFlpLine *lin = 0;
  for (lin = (CFlpLine*)wPoints->HeadPrimary(); lin != 0; lin = (CFlpLine*)wPoints->NextPrimary(lin))
  { lin->Edit();  }
  if (win) win->Refresh();
  return;
}
//--------------------------------------------------------------------
//  Init the node with veh parameters
//  Detect terminated line
//--------------------------------------------------------------------
void CFlightPlan::InitWaypoint(CWayPoint *wpt)
{ globals->dbc->PopulateNode(wpt);  // Populate with database
  if (wpt->IsTerminated())
      { tLIN  = uLIN;               // Terminated waypoint
        //---Get the next waypoint as active waypoint ----
        aLIN  = (CFlpLine *)wPoints->NextPrimary(uLIN);
        return;
      }
  //---Update departure time ---------------------------------
  wpt->SetDeparture(LegDT);
  //--- Refresh this waypoint --------------------------------
  wpt->Refresh(Frame);
  //---Compute distance to previous Node --------------------
  CWayPoint *prv = GetPrevWaypoint();
  wpt->ComputeLeg(prv);
  return;
  }
//--------------------------------------------------------------------
//  Set speed and altitude to use
//--------------------------------------------------------------------
void CFlightPlan::ComputeSpeed()
{ float spd = 0;
  float alt = 0;
  CVehicleObject    *veh = globals->sit->GetUserVehicle();
  if (0 == veh)       return;
  CSimulatedVehicle *svh = veh->svh;
  svh->GetSpeedAndCeiling(&spd,&alt);
  spd *= 0.75f;                 // Default speed
  alt *= 0.60f;                 // Default altitude
  //---Compute airborne indicator -------------------
  float   asp = veh->GetPreCalculedKIAS();
  airborn     = (!veh->WheelsAreOnGround()) & (asp > 40 ) ;
  if (airborn) spd = asp;
  Speed = spd;
  Alti  = alt;
  return;
}
//--------------------------------------------------------------------
//  End of Flight Plan tour
//--------------------------------------------------------------------
void CFlightPlan::EndOfTour()
{ State = FPLAN_OPER;
  if (aLIN) 
  { wptActive(aLIN);
    aLIN->GetWPT()->UpdateFirstNode();
  }
  else      aLIN = eLIN;
  return;
}
//--------------------------------------------------------------------
//  Update Flight plan status
//--------------------------------------------------------------------
void CFlightPlan::TimeSlice (float dT,U_INT FrNo)
  { ComputeSpeed();
    CFuiFlightLog *nwin    = globals->dbc->GetLOGwindow();
    CFlpLine  *lin  = uLIN;
    CWayPoint *wpt  = 0;
    Frame           = FrNo;
    switch (State)  {
      //---Empty flight plan --------------------------
      case FPLAN_NULL:
        return;
      //---Scanning is stopped ------------------------
      case FPLAN_STOP:
        return;
      //---Flight plan is loaded ----------------------
      case FPLAN_LOAD:
        return StartFPlan();
      //---Flight plan first tour ---------------------
      case FPLAN_TOUR:
        wpt = lin->GetWPT();
        wpt->SetSeq(Num++);                           // Sequence number
        nWPT.push_back(wpt);                          // Enter in vector
        InitWaypoint(wpt);                            // Check for termination
        pLIN  = lin;                                  // Previous line
        uLIN  = (CFlpLine*)wPoints->NextPrimary(lin); // Next slot
        break;
      //--- Flight Plan is operational ---------------
      case FPLAN_OPER:
        wpt = lin->GetWPT();
        wpt->UpdateNode(pLIN,FrNo);                       // Update node
        pLIN  = uLIN;                                     // Previous line
        uLIN  = (CFlpLine*)wPoints->NextPrimary(lin);     // Next way point
        break;
    }
    //--- Check for end of waypoints -----------------
    if (lin != eLIN)         return;
    //----Set a new tour -----------------------------
    if (FPLAN_TOUR == State)  EndOfTour();
    pLIN  = 0;                                        // Previous line
    uLIN  = (CFlpLine*)wPoints->HeadPrimary();        // Recycle
    LegDT = globals->tim->GetUTCDateTime();           // Actual time
    Refresh(nwin);                                    // Refresh data
    return;
  }

//--------------------------------------------------------------------
//  Save the Flight Plan
//  version 0:    Initial version
//--------------------------------------------------------------------
void CFlightPlan::Save()
{ if (0 == modify)          return;
  if (0 == nWPT.size())     return;
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
  WriteTag('desc', "========== Description ===========", &s);
  WriteString(Desc, &s);
  WriteTag('vers', "---------- version number --------", &s);
  WriteInt((int*)(&Version), &s);
  //---Sav individual Waypoint ------------------------
  std::vector<CWayPoint*>::iterator wi;
  for (wi = nWPT.begin(); wi != nWPT.end(); wi++) (*wi)->Save(&s);
  WriteTag('endo', "========== END OBJECT ============", &s);
  CloseStream(&s);
  //---Make it appear in next FP Manager window------
  if (1 == Version) pAddDisk(&globals->pfs,name,name);
  return;
}

//----------------------------------------------------------------------
//  Rename the file
//----------------------------------------------------------------------
void CFlightPlan::RenameFile(char *old,char *fbak)
{ remove(fbak);
  rename(old,fbak);
}
//--------------------------------------------------------------------
//  Call vector map to draw the route
//--------------------------------------------------------------------
void CFlightPlan::DrawOnMap(CFuiVectorMap *win)
{ CFlpLine *org = 0;
  CFlpLine *ext = 0;
  for (org = (CFlpLine*)wPoints->HeadPrimary(); org != 0; org = (CFlpLine*)wPoints->NextPrimary(org))
  { ext = (CFlpLine*)wPoints->NextPrimary(org);
    if (0 == ext) break;
    //----Init route extremity -------------------------------------
    rOrg.SetPosition(org->GetWPT()->ObjPosition());
    rExt.SetPosition(ext->GetWPT()->ObjPosition());
    win->DrawRoute(rOrg,rExt);
  }
  return;
}
//---------------------------------------------------------------------
//  ACCESS METHODS for GPS INTERFACE
//---------------------------------------------------------------------
//  Is this the first waypoint 
//---------------------------------------------------------------------
bool CFlightPlan::IsFirst(CWayPoint *wp)
{ CFlpLine *hln = (CFlpLine *)wPoints->HeadPrimary();
  if (0 == hln)     return false;
  return (hln->GetWPT() == wp);
}
//---------------------------------------------------------------------
//  Is this the first waypoint 
//---------------------------------------------------------------------
bool CFlightPlan::IsActive(CWayPoint *wp)
{ CFlpLine *hln = aLIN;
  if (0 == hln)     return false;
  return (hln->GetWPT() == wp);
}
//---------------------------------------------------------------------
//  Is this the last waypoint 
//---------------------------------------------------------------------
bool CFlightPlan::IsLast(CWayPoint *wp)
{ CFlpLine *hln = eLIN;
  if (0 == hln)     return false;
  return (hln->GetWPT() == wp);
}
//---------------------------------------------------------------------
//  Return a given waypoint 
//---------------------------------------------------------------------
CWayPoint* CFlightPlan::GetWaypoint(U_INT No)
{ if (wPoints->OutSide(No))     return 0;
  return nWPT[No];
}
//---------------------------------------------------------------------
//  Return Last waypoint
//---------------------------------------------------------------------
CWayPoint* CFlightPlan::GetLastWaypoint()
{ if (0 == eLIN)  return 0;
  return eLIN->GetWPT();
}
//---------------------------------------------------------------------
//  Return Last waypoint
//---------------------------------------------------------------------
CWayPoint *CFlightPlan::GetPrevWaypoint()
{ if (0 == pLIN)    return 0;
  return pLIN->GetWPT();
}
//---------------------------------------------------------------------
//  return Object from a given waypoint 
//---------------------------------------------------------------------
CmHead * CFlightPlan::GetDBObject (U_INT No)
{ if (No == -1)               return 0;
  if (No >= nWPT.size())      return 0;
  CWayPoint *wpt = nWPT[No];
  return wpt->GetDBobject();
}
//---------------------------------------------------------------------
//  Return ident of last terminated waypoint
//---------------------------------------------------------------------
char *CFlightPlan::GetTermIdent ()
{ char *ici = "here";
  if (0 == tLIN)  return ici;
  CmHead *obj = tLIN->GetWPT()->GetDBobject();
  return (obj)?(obj->GetIdent()):(ici);
}
//---------------------------------------------------------------------
//  Return ident of last destination waypoint
//---------------------------------------------------------------------
char *CFlightPlan::GetLastIdent ()
{ char *nul = "";
  if (0 == eLIN)  return nul;
  CmHead *obj = eLIN->GetWPT()->GetDBobject();
  return (obj)?(obj->GetIdent()):(nul);
}
//---------------------------------------------------------------------
//  Return total distance to destination
//---------------------------------------------------------------------
float CFlightPlan::GetTotalDistance ()
{ if (0 == eLIN)    return 0;
  return eLIN->GetWPT()->GetTotalDis();
}
//---------------------------------------------------------------------
//  Return total distance to the waypoint
//---------------------------------------------------------------------
float CFlightPlan::GetDistanceTo (U_INT No)
{ if (No >= nWPT.size())  return 0;
  return nWPT[No]->GetTotalDis();
}
//-----------------------------------------------------------------
//  Get remaining miles
//-----------------------------------------------------------------
float CFlightPlan::GetRemainingNM()
{ if (0 == eLIN)    return 0;
  return eLIN->GetWPT()->GetTotalDis();
}
//-----------------------------------------------------------------
//  Return route to active waypoint
//-----------------------------------------------------------------
float CFlightPlan::GetActiveRoute()
{ return aLIN->GetWPT()->GetRoute();}
//-----------------------------------------------------------------
//  return number of Active waypoint
//-----------------------------------------------------------------
short CFlightPlan::GetNoActive()
{ if (0 == aLIN)            return -1;
  return aLIN->GetWPT()->GetSeq();
}
//-----------------------------------------------------------------
//  return number of Terminated waypoint
//-----------------------------------------------------------------
short CFlightPlan::GetNoTerminated()
{ if (0 == tLIN)            return -1;
  return tLIN->GetWPT()->GetSeq();
}
//---------------------------------------------------------------------
//  Waypoint is marked active 
//---------------------------------------------------------------------
inline void CFlightPlan::wptActive(CFlpLine *lin)
{ lin->GetWPT()->SetMark('>');
}
//---------------------------------------------------------------------
//  Waypoint is unmarked active 
//---------------------------------------------------------------------
inline void CFlightPlan::wptUnmark(CFlpLine *lin)
{ lin->GetWPT()->SetMark(' ');
}
//---------------------------------------------------------------------
//  Fill parameters for active waypoint 
//---------------------------------------------------------------------
void  CFlightPlan::GetActiveParameters(GPSpoint &pm)
{ if (NotStable())
  { pm.wpNO = -1;
    pm.wpAD =  0;
    pm.wpRT =  0;
    pm.wpDR = 0;
  }
  else
  { CWayPoint *wpt = aLIN->GetWPT();
    pm.wpNO = wpt->GetSeq();
    pm.wpAD = wpt->GetDBobject();
    pm.wpRT = wpt->GetRoute();
    pm.wpDR = wpt->GetDirectRadial();
  }
return;
}
//--------------------------------------------------------------------
//  Search for the next non terminated waypoint
//--------------------------------------------------------------------
short CFlightPlan::ValidActiveWaypoint(CFlpLine *aln)
{ CFlpLine  *lin;
  CFlpLine  *act = eLIN;  
  //---Unmark the current active waypoint ---------------------
  wptUnmark(aLIN);
  for (lin=aln; lin != 0; lin=(CFlpLine*)wPoints->NextPrimary(lin))
  { CWayPoint *wpt = lin->GetWPT();
    if (wpt->IsTerminated())  continue;
    act  = lin;
    break;
  }
  //----Return the last waypoint number -------------
  aLIN  = act;
  wptActive(act);
  return aLIN->GetWPT()->GetSeq();
}
//--------------------------------------------------------------------
//  Check if obj is part of the flight plan
//  If yes, this is becoming the active waypoint when opt=1
//  NOTE: If the waypoint is terminated, the next non terminated waypoint
//        is searched.  If no one is found, the last waypoint is returned
//        -1 is returned if the waypoint was no part of the FPL
//--------------------------------------------------------------------
short CFlightPlan::NbInFlightPlan (CmHead *obj,char opt)
{ U_SHORT    No = 0;
  CFlpLine  *lin = 0;
  CWayPoint *wpt;
  //---Search for new active waypoint -------------------------
  for (lin=(CFlpLine*)wPoints->HeadPrimary(); lin != 0; lin=(CFlpLine*)wPoints->NextPrimary(lin))
  { wpt = lin->GetWPT(); 
    if (!wpt->SameWPT(obj)) continue;
    if (0 == opt)           return wpt->GetSeq();
    return ValidActiveWaypoint(lin);
  }
  return -1;
}
//=================================================================================
//  Update flight plan line info from CWaypoint
//=================================================================================
void CFlpLine::Edit()
{ U_INT hh = 0;
  U_INT mn = 0;
  U_INT dd = 0;
  U_INT mo = 0;
  char edt[16];
  CWayPoint *wp = GetWPT();
  float dis = wp->GetLegDistance();
  SetMark(wp->GetMark());
  SetName(wp->GetName());
  SetIden(wp->GetIdentity());
  if (dis > 999) _snprintf(edt,16,"%6.0f", dis);
  else           _snprintf(edt,16,"%7.1f", dis);
  SetDist(edt);
  _snprintf(edt,16,"% 4.0f",wp->GetDirectRadial());
  SetRadi(edt);
  _snprintf(edt,16,"% 7.0f",wp->GetAltitude());
  SetAlti(edt);
  //---Edit elapse ---------------------------------
  wp->GetLegDuration(&dd,&hh,&mn);
  if (0 == dd)  _snprintf(edt,12,"%02uh%02umn",hh,mn); 
  else          _snprintf(edt,12,"%ud-%02uh%02umn",dd,hh,mn);
  SetElap(edt);
  //---Edit arrival time ---------------------------
  wp->GetArrivalTime(&hh,&mn);
  wp->GetArrivalDay (&dd,&mo);
  _snprintf(edt,14,"% u-% u at %02uh%02u",dd,mo,hh,mn);
  SetEtar(edt);
  return;
}
//=========================END 0F FILE ====================================================
