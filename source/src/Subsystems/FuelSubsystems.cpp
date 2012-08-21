/*
 * FuelSubsystems.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
 * Copyright 2007-.... Jean Sabatier
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
#include "../Include/WorldObjects.h"
#include "../Include/Subsystems.h"
#include "../Include/Fui.h"
#ifdef _DEBUG	
#include "../Include/Utility.h" 
#endif

using namespace std;


//=============================================================================
// CFuelSubsystem
//  Base class for all fuel objects
//==============================================================================

//---------------------------------------------------------------------
//  Constructor
//--------------------------------------------------------------------
CFuelSubsystem::CFuelSubsystem (void)
{
  TypeIs (SUBSYSTEM_FUEL_SUBSYSTEM);
 *name = 0;
  eNum  = 1;
  f_gph = 0.0f;
  gals  = 0.0;
  Fsrc  = 0;
  int _val = 0;
  int t = 0;
  GetIniVar("TRACE","FuelSystem",&t);
  Tr = char(t);
}
//---------------------------------------------------------------------
//  Destructor
//---------------------------------------------------------------------
CFuelSubsystem::~CFuelSubsystem (void)
{ //---Clear linked cells ---------------------------
  piped.clear();
}
//---------------------------------------------------------------------
//  Read parameters
//---------------------------------------------------------------------
int CFuelSubsystem::Read (SStream *stream, Tag tag)
{
  switch (tag) {
  case 'name':
    // UI descriptive name
    ReadString (name, 64, stream);
    return TAG_READ;

  case 'pipe':
    // Fuel dependency tag
    {
      Tag t = 0;
      ReadTag (&t, stream);
      //-- Enter tag with empty pointer ----
      piped[t] = 0;                       // JSDEV
			return TAG_READ;
    }
  }

  // If tag has not been processed, pass it to the parent
  return CDependent::Read (stream, tag);
}
//----------------------------------------------------------------------------
//	JSDEV* Prepare pipe messages
//----------------------------------------------------------------------------
void CFuelSubsystem::PrepareMsg(CVehicleObject *veh)		
{	SMessage *msg = NULL;
	//-----Call Other dependencies ----
	CDependent::PrepareMsg(veh);
	return;
}
//----------------------------------------------------------------------------
//  Trace link
//----------------------------------------------------------------------------
void CFuelSubsystem::TraceLink(CFuelSubsystem *fs)
{ char *id1 = GetIdString();
  char *id2 = fs->GetIdString();
  if (Tr) TRACE("%s is linked to %s",id1,id2);
}
//----------------------------------------------------------------------------
//	Poll the dependent for update
//----------------------------------------------------------------------------
void CFuelSubsystem::Poll()
{ Send_Message(&mpol);
  state = mpol.intData;
  return;
}
//----------------------------------------------------------------------------
//	JSDEV* Link cell to related
//----------------------------------------------------------------------------
void CFuelSubsystem::LinkCell(CFuelSystem *fsys)
{ std::map<Tag,CFuelSubsystem*>::iterator fp;
  for (fp = piped.begin(); fp != piped.end(); fp++)
  { Tag id = (*fp).first;
    CFuelSubsystem * rs = fsys->GetSubsystem(id);
    (*fp).second = rs;						//piped[id] = rs;
    if (rs) TraceLink(rs);
  }
  return;
}
//----------------------------------------------------------------------------
//	JSDEV* Receive messages
//----------------------------------------------------------------------------
EMessageResult CFuelSubsystem::ReceiveMessage (SMessage *msg)
{
  EMessageResult rc = MSG_IGNORED;

    if (rc == MSG_IGNORED) {
      // See if the message can be processed by the parent class
      rc = CDependent::ReceiveMessage (msg);
    }

  return rc;
}
//----------------------------------------------------------------------------
//	JSDEV* Time slice
//----------------------------------------------------------------------------
void CFuelSubsystem::TimeSlice (float dT,U_INT FrNo)			// JSDEV*
{ CDependent::TimeSlice (dT,FrNo);

}
//----------------------------------------------------------------------------
//  Trace contributing item
//----------------------------------------------------------------------------
void CFuelSubsystem::TraceFS(CFuelSubsystem *fs, float rq)
{ if (rq < 0.00001) return;
  TRACE("Fuel item %-20s State %d Bypass=%c",fs->GetIdString(),fs->State(),fs->ByPass());
  return;
}
//----------------------------------------------------------------------------
//  Trace contributing tank
//----------------------------------------------------------------------------
void CFuelSubsystem::TraceT1(CFuelCell *fc,float frq)
{ char *id = fc->GetIdString();
  float qt = fc->GetCellQty();
  TRACE("BURNED FUEL: found %s with %.04f gals.  Requested fuel %.06f",id,qt,frq);
  return;
}
//----------------------------------------------------------------------------
//  Return contributing tanks for the requested fuel quantity
//  Explore upstream components recursively
//  Stop recursion when a tank is found
//----------------------------------------------------------------------------
void CFuelSubsystem::GetContributingTanks(CFuelSystem *fsys,float rqt)
{ if (this->IsOFF())  return;
  std::map<Tag,CFuelSubsystem *>::iterator fp;
  for (fp = piped.begin(); fp != piped.end(); fp++)
  { CFuelSubsystem *fs = (*fp).second;
    if (Tr) TraceFS(fs,rqt);
    //--- Closed system stop further upstream exploration --
    if (fs->IsOFF())                      continue;
    //--- bypass any component other than tank -------------
    if (fs->NotTank())
    { fs->GetContributingTanks(fsys,rqt);
      continue;
    }
    //----This is a tank -----------------------------------
    CFuelCell *fc = (CFuelCell*)fs;
    if (!fc->HasFuelFor(rqt))             continue;
		U_INT mtk =fsys->AddOpenTank(fc);
		if (mtk > 15)	gtfo("Max of 16 tanks exceeded");
    if (Tr) TraceT1(fc,rqt);
  }
  return;
}
//----------------------------------------------------------------------------
//  Trace contributing tank
//----------------------------------------------------------------------------
void CFuelSubsystem::TraceT2(CFuelCell *fc,CFuelCell *tk,float frq)
{ char *f1 = tk->GetIdString();
  char *f2 = fc->GetIdString();
  float qt = fc->GetCellQty();
  TRACE("REFUELLING %s: found %s with %.04f gals.  Requested fuel %.06f",f1,f2,qt,frq);
  return;
}
//----------------------------------------------------------------------------
//  Return contributing tanks for the requested fuel quantity
//  Explore upstream components recursively
//  Stop recursion when a tank is found
//----------------------------------------------------------------------------
void CFuelSubsystem::GetRefillingTanks(CFuelSystem *fsys,CFuelCell *tk,float rqf)
{ std::map<Tag,CFuelSubsystem *>::iterator fp;
  for (fp = piped.begin(); fp != piped.end(); fp++)
  { CFuelSubsystem *fs = (*fp).second;
    //--- Closed system stop further upstream exploration --
    if (fs->IsOFF())                      continue;
    //--- bypass any component other than tank -------------
    if (fs->NotTank()) 
    { fs->GetRefillingTanks(fsys,tk,rqf);
      continue;
    }
    //--- This is a tank ------------------------------------
    CFuelCell *fc = (CFuelCell*)fs;
    //--- Check for candidacy -------------------------------
    if (!fc->CanFill(tk,rqf))             continue;
    fsys->AddOpenTank(fc);
    if (Tr)  TraceT2(fc,tk,rqf);
  }
  return;
}
//=================================================================================
// CFuelTap
//  This item is the first one that feed fuel to an engine
//=================================================================================
CFuelTap::CFuelTap (void)
{
  TypeIs (SUBSYSTEM_FUEL_TAP);
  eNum    = 1;                   // Engine 1 is default
  stff    = 0;
  f_gph   = 0;
}
//-------------------------------------------------------------------------
//  Read all parameters
//-------------------------------------------------------------------------
int CFuelTap::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'stFF':
    // Initial fuel flow (gph)
    ReadFloat (&stff, stream);
    rc = TAG_READ;
    break;

  case 'eNum':
    // Engine number
    ReadUInt (&eNum, stream);
    rc = TAG_READ;
    break;
  }

  // If tag has not been processed, pass it to the parent
  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CFuelSubsystem::Read (stream, tag);
  }

  return rc;
}
//-------------------------------------------------------------------------
//  All parameters are read
//  set initial state
//-------------------------------------------------------------------------
void CFuelTap::ReadFinished (void)
{ //--- enter this subsystem as the head object in engine ------
  f_gph = stff;
  state = 1;
}
//-------------------------------------------------------------------------
//  Process message
//-------------------------------------------------------------------------
EMessageResult CFuelTap::ReceiveMessage (SMessage *msg)
{ switch (msg->id) {
    case MSG_GETDATA:
      switch (msg->user.u.datatag) {
        case 'gph_':
          msg->realData = f_gph;
          return MSG_PROCESSED;
      }
    }

  return CFuelSubsystem::ReceiveMessage (msg);
}
//-------------------------------------------------------------------------
//  Time slice
//-------------------------------------------------------------------------
void CFuelTap::TimeSlice (float dT,U_INT FrNo)
{
  CFuelSubsystem::TimeSlice (dT,FrNo);
}
//=====================================================================================
// CFuelCell for fcel 
//  A cross feed of 0.1 gal/sec is the default value
//=====================================================================================
CFuelCell::CFuelCell ()
{   Create();
    Fsrc  = 1;
}
//--------------------------------------------------------------------------------
//  Real cell from Fuel system
//--------------------------------------------------------------------------------
CFuelCell::CFuelCell(U_CHAR ind) 
{ Create();
  xInd  = ind;
  Fsrc  = 1;
  if (ind >= FUEL_FEED_STACK)  gtfo("Too much fuel cells");
}
//--------------------------------------------------------------------------------
//  Create cell
//--------------------------------------------------------------------------------
void CFuelCell::Create()
{ TypeIs (SUBSYSTEM_FUEL_CELL);
  xInd  = 0;
  qty   = 0.0f;
  cap   = 0.0f;
  wgh   = 0.0f;
  bPos.x = bPos.y = bPos.z = 0;
  basP = 0.0f;
  gals = 0.0f;
  xfer = false;
  xrFF = 0.1f;
  //
  fuel_burned = 0.0f;
  ClearFeed();
  return;
}
//--------------------------------------------------------------------------------
//  Destructor
//--------------------------------------------------------------------------------
CFuelCell::~CFuelCell()
{ 
}
//--------------------------------------------------------------------------------
//  Read all parameters
//--------------------------------------------------------------------------------
int CFuelCell::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;
  float  fq;
  switch (tag) {
  case 'cap_':
    // Capacity (gallons)
    ReadFloat (&cap, stream);
    rc = TAG_READ;
    break;

  case 'qty_':
    // Default quantity
    ReadFloat (&fq, stream);
    SetCellQty(fq);
    rc = TAG_READ;
    break;

  case 'bPos':
    // Position relative to model center of gravity
    ReadVector (&bPos, stream);
    bPos = bPos + mveh->wgh.svh_cofg;
    bPos.InvertXY(); // set legacy coordinates 
    rc = TAG_READ;
    break;

  case 'basP':
    // Base pressure
    ReadFloat (&basP, stream);
    rc = TAG_READ;
    break;

  case 'xfer':
    // Force transfer
    xfer = true;
    rc = TAG_READ;
    break;

  case 'xrFF':
    // Force transfer fuel flow
    ReadFloat (&xrFF, stream);
    rc = TAG_READ;
    break;
  }

  // If tag has not been processed, pass it to the parent
  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CFuelSubsystem::Read (stream, tag);
  }

  return rc;
}
//--------------------------------------------------------------------------
//  All parameters are read
//  Fuel tank must alway be ON
//--------------------------------------------------------------------------
void CFuelCell::ReadFinished (void)
{ CFuelSubsystem::ReadFinished ();
}
//---------------------------------------------------------------------------
//  Process message requests
//---------------------------------------------------------------------------
EMessageResult CFuelCell::ReceiveMessage (SMessage *msg)
{   switch (msg->id) {
    case MSG_GETDATA:
      switch (msg->user.u.datatag) {
      case 'gals':
      case 'fqty':
        // Return fuel quantity in gallons
        msg->realData = double(gals);
        return MSG_PROCESSED;
      }
      return CFuelSubsystem::ReceiveMessage (msg);

    case MSG_SETDATA:
      switch (msg->user.u.datatag) {
      case 'gals':
      case 'fqty':
        // Set fuel quantity in gallons
        gals = (float)msg->realData;
        return MSG_PROCESSED;

      case 'st8t':
      case 'stat':
        return MSG_PROCESSED;
      }
      return CFuelSubsystem::ReceiveMessage (msg);
    }
  return CFuelSubsystem::ReceiveMessage (msg);
}
//--------------------------------------------------------------------
//  Change fuel quantity
//  TODO: Compute weight according to fuel grad
//---------------------------------------------------------------------
void CFuelCell::SetCellQty(float qt)
{ qty        = qt;
  gals       = qt;
  wgh        = qty * FUEL_LBS_PER_GAL;
  return;
}
//--------------------------------------------------------------------
//  Edit qty in probe
//--------------------------------------------------------------------
void CFuelCell::Probe(CFuiCanva *cnv)
{ char edt[16];
  CDependent::Probe(cnv,0);
  _snprintf(edt,16,"%.04f",gals);
  cnv->AddText( 1,"Fuel QTY");
  cnv->AddText(10,edt,1);

  _snprintf(edt,16,"%.04f",fuel_burned);
  cnv->AddText( 1,"Fuel burned");
  cnv->AddText(10,edt,1);
  return;
}
//--------------------------------------------------------------------
//  Compute cell contribution to CG
//  -mm = sum(FuelMassi * bPosi²)   (inertia)
//  -vn = sum(FuelMassi * bPosi)
//  -sm = sum(FuelMassi)
//--------------------------------------------------------------------
void CFuelCell::ContributionCG(SVector &mm,SVector &cg, double &sm)
{ double sx = wgh * bPos.x;
  double sy = wgh * bPos.y;
  double sz = wgh * bPos.z;
  cg.x += sx;
  cg.y += sy;
  cg.z += sz;
  //----Inertia contribution ------------------
  mm.x += sx * bPos.x;
  mm.y += sy * bPos.y;
  mm.z += sz * bPos.z;
  //----contribute to total mass --------------
  sm   += wgh;
  return;
}
//--------------------------------------------------------------------
//  Add the indicated fuel quantity
//--------------------------------------------------------------------
void CFuelCell::AddFuel(float fqt)
{ qty += fqt;
  gals = qty;
  wgh  = qty * FUEL_LBS_PER_GAL;     // Remaining weight
  return;
}
//--------------------------------------------------------------------
//  remove the indicated fuel quantity
//--------------------------------------------------------------------
void CFuelCell::RemFuel(float fqt)
{ qty -= fqt;
  gals = qty;
  wgh  = qty * FUEL_LBS_PER_GAL;     // Remaining weight
  return;
}
//--------------------------------------------------------------------
//  Burn the indicated fuel quantity
//--------------------------------------------------------------------
void CFuelCell::BurnFuel(float bnf)
{ fuel_burned += bnf;             // Cumulative;
  gals    = qty - bnf;            // Remaining qty
  qty     = gals;
  wgh     = qty * FUEL_LBS_PER_GAL;     // Remaining weight
  return;
}
//--------------------------------------------------------------------
//  Check if this cell can fill the indicated cell
//  NOTE: TD => Candidate to give the rqt fuel QTY
//        TR => Cell tk which is requesting fuel
//        To be selected TD must have enough fuel satifying
//        Qty(TD) >= Lim(TR) + rqt.  This is to ensure equilibrum
//        between TD and TR
//        Lim(TR) depends on the type of TR.  For a collector Lim(TR) = 0
//        Thus TD is selected if it has a fuel qty > rqt.
//        For a normal cell Lim(TR) = Qty(TR).  
//        Thus TD is selected if it has more fuel than TR
//--------------------------------------------------------------------
bool CFuelCell::CanFill(CFuelCell *tk,float rqt)
{ Tag     id = tk->GetUnId();
  U_CHAR  No = tk->GetCellIndex();
  //--- Check if we allready filled tk for this cycle -----------
  if (Feed[No] == id)     return false;
  //--- Check if we have enough fuel to satify tk ---------------
  float tot = tk->GetLimitQty() + rqt;
  if (qty <= tot)         return false;
  //--- Enter tk in this feed table -----------------------------
  Feed[No] = id;
  return true;
}
//----------------------------------------------------------------------------
//  Give fuel quantity to the tk cel
//----------------------------------------------------------------------------
void CFuelCell::GiveFuelTo(CFuelCell *tk, float rqf)
{ RemFuel(rqf);
  U_CHAR  No = tk->GetCellIndex();
  Feed[No] = 0;
  return;
}
//----------------------------------------------------------------------------
//  Clear feeding association and reserve space for next cycle
//----------------------------------------------------------------------------
void CFuelCell::ClearFeed()
{ Tag *dst = Feed; 
  for (int k = 0; k != FUEL_FEED_STACK; k++) *dst++ = 0;
  return;
}
//--------------------------------------------------------------------
//  Update cell data
//  Dqty = instant fuel burned
//--------------------------------------------------------------------
void CFuelCell::TimeSlice (float dT,U_INT FrNo)							// JSDEV*
{
  CFuelSubsystem::TimeSlice (dT,FrNo);									// JSDEV
}

//=====================================================================
// CFuelCollector 
//=====================================================================
CFuelCollector::CFuelCollector(U_CHAR ind)
:CFuelCell(ind)
{ TypeIs (SUBSYSTEM_FUEL_COLR);
}
//=====================================================================
// CFuelHeater
//=====================================================================

CFuelHeater::CFuelHeater (void)
{
  TypeIs (SUBSYSTEM_FUEL_HEATER);
}


int CFuelHeater::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  // If tag has not been processed, pass it to the parent
  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CFuelSubsystem::Read (stream, tag);
  }

  return rc;
}


//=====================================================================
// CFuelMath
//=====================================================================

CFuelMath::CFuelMath (void)
{
  TypeIs (SUBSYSTEM_FUEL_MATH);
}


int CFuelMath::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  // If tag has not been processed, pass it to the parent
  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CFuelSubsystem::Read (stream, tag);
  }

  return rc;
}


//=================================================================================
// CFuelPump
//=================================================================================
CFuelPump::CFuelPump (void)
{
  TypeIs (SUBSYSTEM_FUEL_PUMP);

  setP = 0;
  pass = false;
  enabled = false;
}
//--------------------------------------------------------------------
//  Read Parameters
//--------------------------------------------------------------------
int CFuelPump::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'setP':
    // Pump pressure
    ReadFloat (&setP, stream);
    rc = TAG_READ;
    break;

  case 'pass':
    // Passthrough allowed when system is off
    pass = true;
    rc = TAG_READ;
    break;

  case 'nabl':
    // Pump is enabled by default
    enabled = true;
    rc = TAG_READ;
    break;
  }

  // If tag has not been processed, pass it to the parent
  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CFuelSubsystem::Read (stream, tag);
  }

  return rc;
}
//--------------------------------------------------------------------
//  Check for ON/OFF state
//--------------------------------------------------------------------
bool CFuelPump::IsOFF()
{ bool on = (pass || (0 != state));
  return (!on);
}
//--------------------------------------------------------------------
//  Time slice 
//--------------------------------------------------------------------
void CFuelPump::TimeSlice (float dT,U_INT FrNo)			// JSDEV*
{
  CFuelSubsystem::TimeSlice (dT,FrNo);					// JSDEV*

  setP = 30.0f; // just a test
}

//======================================================================
// CFuelSource  JS: Is taht a flyI survival???
//======================================================================

CFuelSource::CFuelSource (void)
{
  TypeIs (SUBSYSTEM_FUEL_SOURCE);
}


int CFuelSource::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  // If tag has not been processed, pass it to the parent
  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CFuelSubsystem::Read (stream, tag);
  }

  return rc;
}


//==============================================================================
// CFuelTank
//
//==============================================================================
CFuelTank::CFuelTank (U_CHAR ind)
:CFuelCell(ind)
{
  TypeIs (SUBSYSTEM_FUEL_TANK);
}


//=============================================================================
// CEngineFuelPump
//=============================================================================

CEngineFuelPump::CEngineFuelPump (void)
{
  TypeIs (SUBSYSTEM_ENGINE_FUEL_PUMP);
  pass = true;
}

//---------------------------------------------------------------------------
//  Read parameters
//---------------------------------------------------------------------------
int CEngineFuelPump::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  // If tag has not been processed, pass it to the parent
  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CFuelSubsystem::Read (stream, tag);
  }

  return rc;
}
//---------------------------------------------------------------------------
//  Check on/off state
//---------------------------------------------------------------------------
bool CEngineFuelPump::IsOFF()
{ bool on = (pass || (0 != state));
  return (!on);
}

//=============================================================================
// CFuelManagement
//=============================================================================

CFuelManagement::CFuelManagement (void)
{
  TypeIs (SUBSYSTEM_FUEL_MANAGEMENT);
}


int CFuelManagement::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  // If tag has not been processed, pass it to the parent
  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CFuelSubsystem::Read (stream, tag);
  }

  return rc;
}


//
// CAPUFuelSource
//

CAPUFuelSource::CAPUFuelSource (void)
{
  TypeIs (SUBSYSTEM_APU_FUEL_SOURCE);
}


int CAPUFuelSource::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  // If tag has not been processed, pass it to the parent
  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CFuelSubsystem::Read (stream, tag);
  }

  return rc;
}
