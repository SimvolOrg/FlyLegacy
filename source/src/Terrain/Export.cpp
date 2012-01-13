/*
 * EXPORT
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
 * Copyright 2007      Jean Sabatier
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
#include "../Include/Database.h"
#include "../Include/TerrainElevation.h"
#include "../Include/Export.h"
#include "../Include/SqlMGR.h"
#include "../Include/FileParser.h"
#include "../Include/Airport.h"
#include "../Include/Taxiway.h"
#include "../Include/TerrainTexture.h"
#include "../Include/Reductor.h"
#include "../Include/Fui.h"
#include "../Include/Pod.h"
//=============================================================================
extern char *RunwayNAM[];
extern char *LiteNAM[];
//=============================================================================
//  Write a flat record as CSV (comma separated field) for SQL base
//  sep is the separator
//=============================================================================
void CAirport::WriteCVS(U_INT No,U_INT gx,U_INT gz,char *sep,SStream &st)
{ CStreamFile* sf = (CStreamFile*)(st.stream);
  U_INT k = (gx << 16) | gz;
  fprintf (sf->f, "%d%s", k, sep);                  // Tile key
  fprintf (sf->f, "%s%s", akey,sep);                // Airport Key
  fprintf (sf->f, "%s%s", afaa,sep);                // FAA ident
  fprintf (sf->f, "%s%s", aica,sep);                // ICAO ident
  fprintf (sf->f, "%s%s", name,sep);                // Name
  fprintf (sf->f, "%s%s", actr,sep);                // Country
  fprintf (sf->f, "%s%s", asta,sep);                // State
  fprintf (sf->f, "%s%s", "...",sep);               // County  (not used)
  fprintf (sf->f, "%d%s", atyp,sep);                // Airport type
  fprintf (sf->f, "%.12f%s", apos.lat,sep);         // Latitude
  fprintf (sf->f, "%.12f%s", apos.lon,sep);         // Longitude
  fprintf (sf->f, "%d%s", int(apos.alt),sep);       // Altitude
  fprintf (sf->f, "%d%s", aown,sep);                // ownership
  fprintf (sf->f, "%d%s", ause,sep);                // usage
  fprintf (sf->f, "%s%s", "...",sep);               // Region
  fprintf (sf->f, "%s%s", "...",sep);               // Notam facility
  fprintf (sf->f, "%s%s", "...",sep);               // FSS Facility
  fprintf (sf->f, "%.12f%s", amag,sep);             // Magnetic variation
  fprintf (sf->f, "%d%s", aalt,sep);                    // Traffic altitude
  fprintf (sf->f, "%d%s", afsa,sep);                    // Service available
  fprintf (sf->f, "%d%s", antd,sep);                    // NOTAM D available
  fprintf (sf->f, "%d%s", aata,sep);                    // Flight service available
  fprintf (sf->f, "%d%s", aseg,sep);                    // Segment circle available
  fprintf (sf->f, "%d%s", alnd,sep);                    // Landing fee charged
  fprintf (sf->f, "%d%s", amjc,sep);                    // Military and civil operations
  fprintf (sf->f, "%d%s", amln,sep);                    // Military landing fee
  fprintf (sf->f, "%d%s", acus,sep);                    // Custom entry airport
  fprintf (sf->f, "%d%s", afue,sep);                    // Fuel typa available
  fprintf (sf->f, "%d%s", afrm,sep);                    // Frame service available
  fprintf (sf->f, "%d%s", aeng,sep);                    // Engine service available
  fprintf (sf->f, "%d%s", abto,sep);                    // Oxygen bottle
  fprintf (sf->f, "%d%s", ablo,sep);                    // Oxygen bulk
  fprintf (sf->f, "%d%s", alen,sep);                    // Beacon color
  fprintf (sf->f, "%d%s", acft,sep);                    // Aircraft types
  fprintf (sf->f, "%d%s", aaco,sep);                    // Annual commercial operations
  fprintf (sf->f, "%d%s", aaga,sep);                    // Annual gen aviation operations
  fprintf (sf->f, "%d%s", aamo,sep);                    // Annual military operations
  fprintf (sf->f, "%d%s", aatf,sep);                    // Airport attendance
  fprintf (sf->f, "%d%s", aats,sep);                    // Attendance start time
  fprintf (sf->f, "%d%s", aate,sep);                    // Attendance end time
  fprintf (sf->f, "%d%s", altf,sep);                    // Airport lighting
  fprintf (sf->f, "%d%s", alts,sep);                    // Ligthing start time
  fprintf (sf->f, "%d%s", alte,sep);                    // Lighting end time
  fprintf (sf->f, "%d%s", atyc,sep);                    // Airport type
  fprintf (sf->f, "%d%s", aicn,sep);                    // Airport icon
  fprintf (sf->f, "%d%s", anrw,sep);                    // Number of runways
  fprintf (sf->f, "%d%s", alrw,sep);                    // Longuest runway
  fprintf (sf->f, "%d%s", avor,sep);                    // VOR colocated on airport
  fprintf (sf->f, "%d%s", aatc,sep);                    // ATC available
  fprintf (sf->f, "%d\n", ails);                        // ILS available
  return;
}
//=============================================================================
//  Write a flat record as CSV (comma separated field) for SQL base
//  sep is the separator
//=============================================================================
void CRunway::WriteCVS(U_INT No,U_INT gx,U_INT gz,char *sep,SStream &st)
{ CStreamFile* sf = (CStreamFile*)(st.stream);
  U_INT k = (gx << 16) | gz;                        // Full key
  fprintf (sf->f, "%d%s", No,sep);                  // Sequence number
  fprintf (sf->f, "%s%s", rapt,sep);                // Airport key
  fprintf (sf->f, "%d%s", rlen,sep);                // Runway length
  fprintf (sf->f, "%d%s", rwid,sep);                // Runway width
  fprintf (sf->f, "%d%s", rsfc,sep);                // Surface type
  fprintf (sf->f, "%d%s", rcon,sep);                // Surface condition
  fprintf (sf->f, "%d%s", rpav,sep);                // Pavement type
  fprintf (sf->f, "%d%s", rpcn,sep);                // Pavement condition
  fprintf (sf->f, "%d%s", rsub,sep);                // Pavement subgrade
  fprintf (sf->f, "%d%s", rpsi,sep);                // Pavement tire PSI
  fprintf (sf->f, "%d%s", rpat,sep);                // Trafic pattern direction
  fprintf (sf->f, "%d%s", rcls,sep);                // Closed Unused
  fprintf (sf->f, "%d%s", rpcl,sep);                // Pilot controlled lighting
  //--------HI RUNWAY END ----------------------------------------------------
  fprintf (sf->f, "%s%s", rhid,sep);                // Ident
  fprintf (sf->f, "%.12f%s", pshi.lat,sep);         // Latitude
  fprintf (sf->f, "%.12f%s", pshi.lon,sep);         // Longitude
  fprintf (sf->f, "%d%s", int(pshi.alt),sep);       // Altitude
  fprintf (sf->f, "%.12f%s", rhhd,sep);             // True heading
  fprintf (sf->f, "%.12f%s", rhmh,sep);             // Mag deviation
  fprintf (sf->f, "%d%s", rhdt,sep);                // Displaced threshold
  fprintf (sf->f, "%d%s", rhte,sep);                // Threshold elevation
  fprintf (sf->f, "%d%s", rhel,sep);                // Ending lights
  fprintf (sf->f, "%d%s", rhal,sep);                // Alignment lights
  fprintf (sf->f, "%d%s", rhcl,sep);                // Center lights
  fprintf (sf->f, "%d%s", rhtl,sep);                // Touch down lights
  fprintf (sf->f, "%d%s", rhth,sep);                // Threshold lights
  fprintf (sf->f, "%d%s", rhli,sep);                // Edge lights
  fprintf (sf->f, "%d%s", rhsl,sep);                // Flashing lights
  fprintf (sf->f, "%d%s", rhmk,sep);                // Marking
  fprintf (sf->f, "%d%s", rhvr,sep);                // RVR
  fprintf (sf->f, "%d%s", rhvv,sep);                // RVV
  fprintf (sf->f, "%d%s", rhgt,sep);                // Glide slope type
  fprintf (sf->f, "%d%s", rhgc,sep);                // Glide slope configuration
  fprintf (sf->f, "%d%s", rhgl,sep);                // Glide slope location
  fprintf (sf->f, "%d%s", rh8l[0],sep);                // Light system 1
  fprintf (sf->f, "%d%s", rh8l[1],sep);                // Light system 2
  fprintf (sf->f, "%d%s", rh8l[2],sep);                // Light system 3
  fprintf (sf->f, "%d%s", rh8l[3],sep);                // Light system 4
  fprintf (sf->f, "%d%s", rh8l[4],sep);                // Light system 5
  fprintf (sf->f, "%d%s", rh8l[5],sep);                // Light system 6
  fprintf (sf->f, "%d%s", rh8l[6],sep);                // Light system 7
  fprintf (sf->f, "%d%s", rh8l[7],sep);                // Light system 8
  //--------LO RUNWAY END ----------------------------------------------------
  fprintf (sf->f, "%s%s", rlid,sep);                // Ident
  fprintf (sf->f, "%.12f%s", pslo.lat,sep);         // Latitude
  fprintf (sf->f, "%.12f%s", pslo.lon,sep);         // Longitude
  fprintf (sf->f, "%d%s", int(pslo.alt),sep);       // Altitude
  fprintf (sf->f, "%.12f%s", rlhd,sep);             // True heading
  fprintf (sf->f, "%.12f%s", rlmh,sep);             // Mag deviation
  fprintf (sf->f, "%d%s", rldt,sep);                // Displaced threshold
  fprintf (sf->f, "%d%s", rlte,sep);                // Threshold elevation
  fprintf (sf->f, "%d%s", rlel,sep);                // Ending lights
  fprintf (sf->f, "%d%s", rlal,sep);                // Alignment lights
  fprintf (sf->f, "%d%s", rlcl,sep);                // Center lights
  fprintf (sf->f, "%d%s", rltl,sep);                // Touch down lights
  fprintf (sf->f, "%d%s", rlth,sep);                // Threshold lights
  fprintf (sf->f, "%d%s", rlli,sep);                // Edge lights
  fprintf (sf->f, "%d%s", rlsl,sep);                // Flashing lights
  fprintf (sf->f, "%d%s", rlmk,sep);                // Marking
  fprintf (sf->f, "%d%s", rlvr,sep);                // RVR
  fprintf (sf->f, "%d%s", rlvv,sep);                // RVV
  fprintf (sf->f, "%d%s", rlgt,sep);                // Glide slope type
  fprintf (sf->f, "%d%s", rlgc,sep);                // Glide slope configuration
  fprintf (sf->f, "%d%s", rlgl,sep);                // Glide slope location
  fprintf (sf->f, "%d%s", rl8l[0],sep);                // Light system 1
  fprintf (sf->f, "%d%s", rl8l[1],sep);                // Light system 2
  fprintf (sf->f, "%d%s", rl8l[2],sep);                // Light system 3
  fprintf (sf->f, "%d%s", rl8l[3],sep);                // Light system 4
  fprintf (sf->f, "%d%s", rl8l[4],sep);                // Light system 5
  fprintf (sf->f, "%d%s", rl8l[5],sep);                // Light system 6
  fprintf (sf->f, "%d%s", rl8l[6],sep);                // Light system 7
  fprintf (sf->f, "%d%\n",rl8l[7]);                    // Light system 8
  //------------------------------------------------------------------------
  return;
}
//=============================================================================
//  Write a flat record as CSV (comma separated field) for SQL base
//  sep is the separator
//=============================================================================
void CNavaid::WriteCVS(U_INT gx,U_INT gz,char *sep,SStream &st)
{ CStreamFile* sf = (CStreamFile*)(st.stream);
  U_INT k = (gx << 16) | gz;                           // Full key
  //---Remove any , inside name -----------------------------------
  char *dot = strchr(name,',');
  if (dot) *dot= '-';
  //---------------------------------------------------------------
  fprintf (sf->f, "%d%s", k,    sep);                  // Tile key
  fprintf (sf->f, "%s%s", nkey, sep);                  // Nav key
  fprintf (sf->f, "%s%s", naid, sep);                  // ident
  fprintf (sf->f, "%s%s", name, sep);                  // name 
  fprintf (sf->f, "%.12f%s", freq, sep);               // Frequency
  fprintf (sf->f, "%.12f%s", mDev, sep);               // mag deviation
  fprintf (sf->f, "%.12f%s", nsvr, sep);               // slave deviation
  fprintf (sf->f, "%s%s", ncty, sep);                  // country 
  fprintf (sf->f, "%s%s", nsta, sep);                  // state 
  fprintf (sf->f, "%d%s", type, sep);                  // Type
  fprintf (sf->f, "%d%s", ncla, sep);                  // classification
  fprintf (sf->f, "%d%s", usge, sep);                  // Usage
  //------------------------------------------------------------
  fprintf (sf->f, "%.12f%s", pos.lat,sep);          // Latitude
  fprintf (sf->f, "%.12f%s", pos.lon,sep);          // Longitude
  fprintf (sf->f, "%d%s", int(pos.alt),sep);        // Altitude
  fprintf (sf->f, "%.12f%s", npow,sep);             // power
  fprintf (sf->f, "%.12f\n", rang);                 // range
  return;
}
//=============================================================================
//  Write a flat record as CSV (comma separated field) for SQL base
//  sep is the separator
//=============================================================================
void CILS::WriteCVS(U_INT gx,U_INT gz,char *sep,SStream &st)
{ CStreamFile* sf = (CStreamFile*)(st.stream);
  U_INT k = (gx << 16) | gz;                           // Full key
  //---Remove any , inside name -----------------------------------
  char *dot = strchr(name,',');
  if (dot) *dot= '-';
  fprintf (sf->f, "%d%s", k,    sep);                  // Tile key
  fprintf (sf->f, "%s%s", ikey, sep);                  // ILS     key
  fprintf (sf->f, "%s%s", iapt, sep);                  // Airport ident
  fprintf (sf->f, "%s%s", irwy, sep);                  // Runway  end
  fprintf (sf->f, "%s%s", iils, sep);                  // ILS  ident
  fprintf (sf->f, "%s%s", name, sep);                  // ILS  name
  fprintf (sf->f, "%d%s", type, sep);                  // Type
  fprintf (sf->f, "%.12f%s", pos.lat,sep);          // Latitude
  fprintf (sf->f, "%.12f%s", pos.lon,sep);          // Longitude
  fprintf (sf->f, "%d%s", int(pos.alt),sep);        // Altitude
  //-----------------------------------------------------------------
  fprintf (sf->f, "%.12f%s", freq,sep);             // Frequency
  fprintf (sf->f, "%.12f%s", gsan,sep);             // glide slope angle
  fprintf (sf->f, "%.12f%s", mDev,sep);             // mag deviation
  fprintf (sf->f, "%.12f\n", rang);                 // range
  return;
}
//=============================================================================
//  Write a flat record as CSV (comma separated field) for SQL base
//  sep is the separator
//=============================================================================
void CCOM::WriteCVS(U_INT gx,U_INT gz,char *sep,SStream &st)
{ CStreamFile* sf = (CStreamFile*)(st.stream);
  U_INT k = (gx << 16) | gz;                            // Full key
  //---Remove any , inside name -----------------------------------
  char *dot = strchr(cnam,',');
  if (dot) *dot= '-';
  //---------------------------------------------------------------
  fprintf (sf->f, "%d%s", k,    sep);                   // Tile key
  fprintf (sf->f, "%s%s", ckey, sep);                   // COM     key
  fprintf (sf->f, "%s%s", capt, sep);                   // APT     key
  fprintf (sf->f, "%s%s", cnam, sep);                   // Name
  fprintf (sf->f, "%d%s", ctyp, sep);                   // Type
  fprintf (sf->f, "%.12f%s", cfr1,sep);                 // Frequency
  fprintf (sf->f, "%.12f%s", cfr2,sep);                 // Frequency
  fprintf (sf->f, "%.12f%s", cfr3,sep);                 // Frequency
  fprintf (sf->f, "%.12f%s", cfr4,sep);                 // Frequency
  fprintf (sf->f, "%.12f%s", cfr5,sep);                 // Frequency
  //----------------------------------------------------------------
  fprintf (sf->f, "%.12f%s", pos.lat,sep);              // Latitude
  fprintf (sf->f, "%.12f\n", pos.lon);                  // Longitude
  return;
  }
//=============================================================================
//  Write a flat record as CSV (comma separated field) for SQL base
//  sep is the separator
//=============================================================================
void CWPT::WriteCVS(U_INT gx,U_INT gz,char *sep,SStream &st)
{ CStreamFile* sf = (CStreamFile*)(st.stream);
  U_INT k = (gx << 16) | gz;                            // Full key
  //---Remove any , inside name -----------------------------------
  char *dot = strchr(wnam,',');
  if (dot) *dot= '-';
  fprintf (sf->f, "%d%s", k,    sep);                   // Tile key
  fprintf (sf->f, "%s%s", widn, sep);                   // WPT ident
  fprintf (sf->f, "%s%s", wkey, sep);                   // WPT key
  fprintf (sf->f, "%s%s", wnam, sep);                   // WPT name
  fprintf (sf->f, "%s%s", wcty, sep);                   // WPT Country
  fprintf (sf->f, "%s%s", wsta, sep);                   // WPT state
  fprintf (sf->f, "%d%s", wloc, sep);                   // VOR coolocated
  fprintf (sf->f, "%d%s", wtyp, sep);                   // type
  fprintf (sf->f, "%d%s", wuse, sep);                   // usage
  fprintf (sf->f, "%.12f%s", wpos.lat,sep);             // Latitude
  fprintf (sf->f, "%.12f%s", wpos.lon,sep);             // Longitude
  fprintf (sf->f, "%d%s", int(wpos.alt),sep);           // Altitude
  fprintf (sf->f, "%.12f%s", wmag,sep);                 // Mag deviation
  fprintf (sf->f, "%.12f%s", wbrg,sep);                 // bearing to nav
  fprintf (sf->f, "%.12f%s", wdis,sep);                 // Distance to nav
  fprintf (sf->f, "%s%\n", wnav);                       // Nav Key
  return;
}
//=============================================================================
//  Write a flat record as CSV (comma separated field) for SQL base
//  sep is the separator
//=============================================================================
void CCountry::WriteCVS(char *sep,SStream &st)
{ CStreamFile* sf = (CStreamFile*)(st.stream);
  //---Remove any , inside name -----------------------------------
  char *dot = strchr(cnam,',');
  if (dot) *dot= '-';
  //---------------------------------------------------------------
  fprintf (sf->f, "%s%s", cuid, sep);                   // CTY ident
  fprintf (sf->f, "%s\n", cnam);                        // CTY name
  return;
}
//=============================================================================
//  Write a flat record as CSV (comma separated field) for SQL base
//  sep is the separator
//=============================================================================
void CState::WriteCVS(char *sep,SStream &st)
{ CStreamFile* sf = (CStreamFile*)(st.stream);
  //---Remove any , inside name -----------------------------------
  char *dot = strchr(name,',');
  if (dot) *dot= '-';
  //---------------------------------------------------------------
  fprintf (sf->f, "%s%s", skey, sep);                   // STATE key
  fprintf (sf->f, "%s%s", sctr, sep);                   // contry key
  fprintf (sf->f, "%s%s", nsta, sep);                   // state key
  fprintf (sf->f, "%s\n", name);                        // state name
  return;
}
//=============================================================================
//  CLASS CEXPORT
//=============================================================================
CExport::CExport()
{ int opt = 0;
  strcpy(Sep,", ");                     // Default separator for SQLITE
  noRec   = 0;                          // Record
  GetIniVar("SQL","ExpGEN",&opt);
  gen		= opt;
	sqm		=  globals->sqm;
	pfs		= &globals->pfs;	
  //------------------------------------------------------------
  opt = 0;
  GetIniVar("SQL","ExpELV",&opt);
  elv = opt;
  //-------------------------------------------------------------
  opt = 0;
  GetIniVar("SQL","ExpSEA",&opt);
  sea = opt;
  //-------------------------------------------------------------
  opt = 0;
  GetIniVar("SQL","ExpTXY",&opt);
  txy = opt;
  //--------------------------------------------------------------
  opt = 0;
  GetIniVar("SQL","ExpM3D",&opt);
  m3d = opt;
	//--------------------------------------------------------------
  opt  = 0;
  GetIniVar("SQL","UpdM3D",&opt);
  m3d |= opt;
  //--------------------------------------------------------------
  opt = 0;
  GetIniVar("SQL","ExpTEX",&opt);
  gtx = opt;
  //--------------------------------------------------------------
  opt = 0;
  GetIniVar("SQL","ExpOBJ",&opt);
  wob = opt;
  //--------------------------------------------------------------
  opt = 0;
  GetIniVar("SQL","ExpTRN",&opt);
  trn = opt;
  //--------------------------------------------------------------
  qtr = 0;
	ftrn	= 0;
  //----Load a camera for 3D object export -----------------------
  Cam = new CCameraSpot();
}
//-----------------------------------------------------------------------------
//  Delete all resources
//-----------------------------------------------------------------------------
CExport::~CExport()
{ delete Cam;
}
//-----------------------------------------------------------------------------
//  Check for valid database
//-----------------------------------------------------------------------------
bool CExport::NoDatabase(CDatabase *db)
{ if (0 == db)          return true;
  if (db->NotMounted()) return true;
  return false;
}
//-----------------------------------------------------------------------------
//  Create CSV files for all databases included in Generic Database
//-----------------------------------------------------------------------------
void CExport::ExportGenericAsCVS()
{ if (0 == gen)   return;
  ExportCtyAsCVS();
  ExportStaAsCVS();
  ExportAirportsAsCVS();
  ExportRunwaysAsCVS();
  ExportNavaidsAsCVS();
  ExportILSAsCVS();
  ExportComAsCVS();
  return;
}
///----------------------------------------------------------------------------
//  Load a list of Airports by globe tile key
//-----------------------------------------------------------------------------
void CExport::ListAirport(U_INT nt,ClQueue &qhd)
{ CAirport *apt;
  char key[16];
  CDatabase *db = CDatabaseManager::Instance().GetAPTDatabase();
  if (NoDatabase(db))   return;
	int kx = (nt >> 16);
	int	kz = (nt & 0x0000FFFF);
	sprintf (key, "%03d%03d", kx, kz);
  unsigned long offset  = db->Search ('nltl', key);
  int         nb        = 0;
	if (offset == 0)      return;
  while (offset != 0)
	{		// Allocate object and fill it----
			db->GetRawRecord(offset);
			apt = new CAirport(ANY,APT);
			db->DecodeRecord(offset,apt);
			//------Put specific code here--------------------
      qhd.LastInQ1(apt);
			offset = db->SearchNext();
	}
  return;
}
//-----------------------------------------------------------------------------------------
//  Export Airports for the requested globe tile
//-----------------------------------------------------------------------------------------
void CExport::ExportAPT(U_INT gx,U_INT gz,char *sep,SStream &st)
{ ClQueue   aptQ;
  CAirport *apt;
  U_INT   ntile = (gx << 16) | gz;
  ListAirport(ntile,aptQ);
  for (apt = (CAirport*)aptQ.GetFirst(); apt != 0; apt = (CAirport*)aptQ.NextInQ1(apt))
  {   apt->WriteCVS(noRec++,gx,gz,sep,st);
  }
  return;
}
//-----------------------------------------------------------------------------------------
//  Export all airport
//-----------------------------------------------------------------------------------------
void CExport::ExportAirportsAsCVS()
{ U_INT gx = 0;
  U_INT gz = 0;
  SStream    st;
  char *fn = "Export/APT.csv";
  noRec    = 0;
  //---Open a stream file -----------------------------------
  strcpy (st.filename, fn);
  strcpy (st.mode, "w");
  if (!OpenStream (&st)) {WARNINGLOG("EXPORT : can't write %s", fn); return;}
  for (gx = 0; gx < 256; gx++)
    for (gz = 0; gz < 256; gz++)
       ExportAPT(gx,gz,Sep,st);
  //---Close the file ----------------------------------------
  CloseStream (&st);
  return;
}
//-----------------------------------------------------------------------------------------
//  Export all Runways
//-----------------------------------------------------------------------------------------
void CExport::ExportRunwaysAsCVS()
{ SStream    st;
  char *fn = "Export/RWY.csv";
  CDatabase *db = CDatabaseManager::Instance().GetRWYDatabase();
  if (NoDatabase(db)) return;
  long end      = db->GetNumRecords();
  U_LONG offset = 0;
  CRunway *rwy  = new CRunway(ANY,RWY);
  long     No   = 0;
  //---Open a stream file -----------------------------------
  strcpy (st.filename, fn);
  strcpy (st.mode, "w");
  if (!OpenStream (&st)) {WARNINGLOG("EXPORT : can't write %s", fn); return;}
  //---Get file parameters ----------------------------------
  while (No != end)
  { offset   = db->RecordOffset(No);
    db->GetRawRecord (offset);
    db->DecodeRecord(offset,rwy);
    rwy->WriteCVS(No,0,0,Sep,st);
    No++;
  }
  delete rwy;
  //---Close the file ----------------------------------------
  CloseStream (&st);
  return;
}
//-----------------------------------------------------------------------------------------
//  Export all Navaids
//-----------------------------------------------------------------------------------------
void CExport::ExportNavaidsAsCVS()
{ U_INT gx = 0;
  U_INT gz = 0;
  SStream    st;
  char *fn = "Export/NAV.csv";
  noRec    = 0;
  //---Open a stream file -----------------------------------
  strcpy (st.filename, fn);
  strcpy (st.mode, "w");
  if (!OpenStream (&st)) {WARNINGLOG("EXPORT : can't write %s", fn); return;}
  for (gx = 0; gx < 256; gx++)
    for (gz = 0; gz < 256; gz++)
       ExportNAV(gx,gz,Sep,st);
  //---Close the file ----------------------------------------
  CloseStream (&st);
  return;
}
//-----------------------------------------------------------------------------------------
//  Export Airports for the requested globe tile
//-----------------------------------------------------------------------------------------
void CExport::ExportNAV(U_INT gx,U_INT gz,char *sep,SStream &st)
{ ClQueue   navQ;
  CNavaid  *nav;
  U_INT   ntile = (gx << 16) | gz;
  ListNavaid(ntile,navQ);
  for (nav = (CNavaid*)navQ.GetFirst(); nav != 0; nav = (CNavaid*)navQ.NextInQ1(nav))
  {nav->WriteCVS(gx,gz,sep,st);
  }
  return;
}
///----------------------------------------------------------------------------
//  Load a list of Airports by globe tile key
//-----------------------------------------------------------------------------
void CExport::ListNavaid(U_INT nt,ClQueue &qhd)
{ CNavaid *nav;
  char key[16];
  CDatabase *db = CDatabaseManager::Instance().GetNAVDatabase();
  if (NoDatabase(db))   return;
	int kx = (nt >> 16);
	int	kz = (nt & 0x0000FFFF);
	sprintf (key, "%03d%03d", kx, kz);
  unsigned long offset  = db->Search ('nltl', key);
  int         nb        = 0;
	if (offset == 0)      return;
  while (offset != 0)
	{		// Allocate object and fill it----
			db->GetRawRecord(offset);
			nav = new CNavaid(ANY,VOR);
			db->DecodeRecord(offset,nav);
			//------Put specific code here--------------------
      qhd.LastInQ1(nav);
			offset = db->SearchNext();
	}
  // Free buffer for raw database record
  return;
}
//-----------------------------------------------------------------------------------------
//  Export all ILS
//-----------------------------------------------------------------------------------------
void CExport::ExportILSAsCVS()
{ U_INT gx = 0;
  U_INT gz = 0;
  SStream    st;
  char *fn = "Export/ILS.csv";
  noRec    = 0;
  //---Open a stream file -----------------------------------
  strcpy (st.filename, fn);
  strcpy (st.mode, "w");
  if (!OpenStream (&st)) {WARNINGLOG("EXPORT : can't write %s", fn); return;}
  for (gx = 0; gx < 256; gx++)
    for (gz = 0; gz < 256; gz++)
       ExportILS(gx,gz,Sep,st);
  //---Close the file ----------------------------------------
  CloseStream (&st);
  return;
}
//-----------------------------------------------------------------------------------------
//  Export ILS for the requested globe tile
//-----------------------------------------------------------------------------------------
void CExport::ExportILS(U_INT gx,U_INT gz,char *sep,SStream &st)
{ ClQueue   ilsQ;
  CILS     *ils;
  U_INT   ntile = (gx << 16) | gz;
  ListILS(ntile,ilsQ);
  for (ils = (CILS*)ilsQ.GetFirst(); ils != 0; ils = (CILS*)ilsQ.NextInQ1(ils))
  {ils->WriteCVS(gx,gz,sep,st);
  }
  return;
}
///----------------------------------------------------------------------------
//  Load a list of Airports by globe tile key
//-----------------------------------------------------------------------------
void CExport::ListILS(U_INT nt,ClQueue &qhd)
{ CILS *ils;
  char key[16];
  CDatabase *db = CDatabaseManager::Instance().GetILSDatabase();
  if (NoDatabase(db))   return;
	int kx = (nt >> 16);
	int	kz = (nt & 0x0000FFFF);
	sprintf (key, "%03d%03d", kx, kz);
  unsigned long offset  = db->Search ('nltl', key);
  int         nb        = 0;
	if (offset == 0)      return;
  while (offset != 0)
	{		// Allocate object and fill it----
			db->GetRawRecord(offset);
			ils = new CILS(ANY,ILS);
			db->DecodeRecord(offset,ils);
			//------Put specific code here--------------------
      qhd.LastInQ1(ils);
			offset = db->SearchNext();
	}
  // Free buffer for raw database record
  return;
}
//-----------------------------------------------------------------------------------------
//  Export all COM
//-----------------------------------------------------------------------------------------
void CExport::ExportComAsCVS()
{ U_INT gx = 0;
  U_INT gz = 0;
  SStream    st;
  char *fn = "Export/COM.csv";
  noRec    = 0;
  //---Open a stream file -----------------------------------
  strcpy (st.filename, fn);
  strcpy (st.mode, "w");
  if (!OpenStream (&st)) {WARNINGLOG("EXPORT : can't write %s", fn); return;}
  for (gx = 0; gx < 256; gx++)
    for (gz = 0; gz < 256; gz++)
       ExportCOM(gx,gz,Sep,st);
  //---Close the file ----------------------------------------
  CloseStream (&st);
  return;
}
//-----------------------------------------------------------------------------------------
//  Export COM for the requested globe tile
//-----------------------------------------------------------------------------------------
void CExport::ExportCOM(U_INT gx,U_INT gz,char *sep,SStream &st)
{ ClQueue   comQ;
  CCOM     *com;
  U_INT   ntile = (gx << 16) | gz;
  ListCOM(ntile,comQ);
  for (com = (CCOM*)comQ.GetFirst(); com != 0; com = (CCOM*)comQ.NextInQ1(com))
  {com->WriteCVS(gx,gz,sep,st);
  }
  return;
}
///----------------------------------------------------------------------------
//  Load a list of COM by globe tile key
//-----------------------------------------------------------------------------
void CExport::ListCOM(U_INT nt,ClQueue &qhd)
{ CCOM *com;
  char key[16];
  CDatabase *db = CDatabaseManager::Instance().GetCOMDatabase();
  if (NoDatabase(db))   return;
	int kx = (nt >> 16);
	int	kz = (nt & 0x0000FFFF);
	sprintf (key, "%03d%03d", kx, kz);
  unsigned long offset  = db->Search ('nltl', key);
  int         nb        = 0;
	if (offset == 0)      return;
  while (offset != 0)
	{		// Allocate object and fill it----
			db->GetRawRecord(offset);
			com = new CCOM(ANY,COM);
			db->DecodeRecord(offset,com);
			//------Put specific code here--------------------
      qhd.LastInQ1(com);
			offset = db->SearchNext();
	}
  return;
}
//-----------------------------------------------------------------------------------------
//  Export all Waypoints
//-----------------------------------------------------------------------------------------
void CExport::ExportWptAsCVS()
{ if (0 == gen)   return;
  U_INT gx = 0;
  U_INT gz = 0;
  SStream    st;
  char *fn = "Export/WPT.csv";
  noRec    = 0;
  //---Open a stream file -----------------------------------
  strcpy (st.filename, fn);
  strcpy (st.mode, "w");
  if (!OpenStream (&st)) {WARNINGLOG("EXPORT : can't write %s", fn); return;}
  for (gx = 0; gx < 256; gx++)
    for (gz = 0; gz < 256; gz++)
       ExportWPT(gx,gz,Sep,st);
  //---Close the file ----------------------------------------
  CloseStream (&st);
  return;
}
//-----------------------------------------------------------------------------------------
//  Export WPT for the requested globe tile
//-----------------------------------------------------------------------------------------
void CExport::ExportWPT(U_INT gx,U_INT gz,char *sep,SStream &st)
{ ClQueue   wptQ;
  CWPT     *wpt;
  U_INT   ntile = (gx << 16) | gz;
  ListWPT(ntile,wptQ);
  for (wpt = (CWPT*)wptQ.GetFirst(); wpt != 0; wpt = (CWPT*)wptQ.NextInQ1(wpt))
  {wpt->WriteCVS(gx,gz,sep,st);
  }
  return;
}
///----------------------------------------------------------------------------
//  Load a list of WPT by globe tile key
//-----------------------------------------------------------------------------
void CExport::ListWPT(U_INT nt,ClQueue &qhd)
{ CWPT *wpt;
  char key[16];
  CDatabase *db = CDatabaseManager::Instance().GetWPTDatabase();
  if (0 == db)          return;
	int kx = (nt >> 16);
	int	kz = (nt & 0x0000FFFF);
	sprintf (key, "%03d%03d", kx, kz);
  unsigned long offset  = db->Search ('nltl', key);
  int         nb        = 0;
	if (offset == 0)      return;
  while (offset != 0)
	{		// Allocate object and fill it----
			db->GetRawRecord(offset);
			wpt = new CWPT(ANY,WPT);
			db->DecodeRecord(offset,wpt);
			//------Put specific code here--------------------
      qhd.LastInQ1(wpt);
			offset = db->SearchNext();
	}
  return;
}
//-----------------------------------------------------------------------------------------
//  Export all Contry
//-----------------------------------------------------------------------------------------
void CExport::ExportCtyAsCVS()
{ SStream    st;
  char *fn = "Export/CTY.csv";
  noRec    = 0;
  //---Open a stream file -----------------------------------
  strcpy (st.filename, fn);
  strcpy (st.mode, "w");
  if (!OpenStream (&st)) {WARNINGLOG("EXPORT : can't write %s", fn); return;}
  ExportCTY(Sep,st);
  //---Close the file ----------------------------------------
  CloseStream (&st);
  return;
}
//-----------------------------------------------------------------------------------------
//  Export all Contry
//-----------------------------------------------------------------------------------------
void CExport::ExportCTY(char *sep,SStream &st)
{ CDatabaseCTY *db  = (CDatabaseCTY*)CDatabaseManager::Instance().GetCTYDatabase();
  if (NoDatabase(db)) return;
  U_INT     No      = 0;
  U_INT     end     = (U_INT)db->GetNumRecords();
  U_LONG  offset    = 0;
  CCountry *cty     = new CCountry();
  while (No != end)
  { offset  =     db->RecordOffset(No++);
    db->GetRawRecord(offset);
    db->DecodeRecord(cty);
    cty->WriteCVS(sep,st);
  }
  delete cty;
  return ;
}
//-----------------------------------------------------------------------------------------
//  Export all State
//-----------------------------------------------------------------------------------------
void CExport::ExportStaAsCVS()
{ SStream    st;
  char *fn = "Export/STA.csv";
  noRec    = 0;
  //---Open a stream file -----------------------------------
  strcpy (st.filename, fn);
  strcpy (st.mode, "w");
  if (!OpenStream (&st)) {WARNINGLOG("EXPORT : can't write %s", fn); return;}
  ExportSTA(Sep,st);
  //---Close the file ----------------------------------------
  CloseStream (&st);
  return;
}
//-----------------------------------------------------------------------------------------
//  Export all STATES
//-----------------------------------------------------------------------------------------
void CExport::ExportSTA(char *sep,SStream &st)
{ CDatabaseSTA *db  = (CDatabaseSTA*)CDatabaseManager::Instance().GetSTADatabase();
  if (NoDatabase(db)) return;
  U_INT     No      = 0;
  U_INT     end     = (U_INT)db->GetNumRecords();
  U_LONG  offset    = 0;
  CState   *sta     = new CState();
  while (No != end)
  { offset  =     db->RecordOffset(No++);
    db->GetRawRecord(offset);
    db->DecodeRecord(sta);
    sta->WriteCVS(sep,st);
  }
  delete sta;
  return ;
}
//=========================================================================================
//    Export COAST DATA
//=========================================================================================
void CExport::ExportCoastInDB()
{ if (0 == sea)   return;
  char name[MAX_PATH];
  U_INT gx = 0;
  U_INT gz = 0;
  //---Get a list of all coast file per Globe Tiles -------------
  globals->sqm->SEAtransaction();
  for   (gz = 0; gz < 256; gz++)
  { for (gx = 0; gx < 256; gx++)
        { sprintf(name,"COAST/V%03d%03d.GTP",gx,gz);
          if (!pexists (&globals->pfs, name)) continue;
          ExportCoastFile(name,gx,gz);
        }
  }
  globals->sqm->SEAcommit();
	globals->appState = APP_EXIT_SCREEN;
  return;
}
//-----------------------------------------------------------------------------------------
//  Compute next detail tile Key in QGT
//	Coast line data are register per Detail Tile. 
//	This function compute the (x,z) coordinate of each detail tile in QGT
//-----------------------------------------------------------------------------------------
bool CExport::NextCoastKey()
{ if (qdr == 4) return false;
  //-----Work with SW QGT ------------------------------
  if (qdr == 0)
  { ax = (qx << 5) | col;         // There are 32 DT in a QGT
    az = (qz << 5) | row;
    col = (col + 1) & (31);
    if (col) return true;
    row = (row + 1) & (31);
    if (row) return true; 
    qdr = 1; 
    qz = bz + 1;
    return true;
  }
  //-----Work with NW QGT ----row in [32-64]------------
  if (qdr == 1)
  { ax = (qx << 5) | col;
    az = (qz << 5) | row;
    col = (col + 1) & (31);
    if (col) return true;
    row = (row + 1) & (31);
    if (row) return true; 
    qdr = 2; 
    qx = bx + 1; 
    qz = bz + 1;
    return true;
  }
  //-----Work wiht NE QGT ---------------------------------
  if (qdr == 2)
  { ax = (qx << 5) | col;
    az = (qz << 5) | row;
    col = (col + 1) & (31);
    if (col) return true;
    row = (row + 1) & (31);
    if (row) return true; 
    qdr = 3;
    qx = bx + 1;
    qz = bz;
    return true;
  }
  //-----Work with SE QGT ------------------------------------
  if (qdr == 3)
  { ax = (qx << 5) | col;
    az = (qz << 5) | row;
    col = (col + 1) & (31);
    if (col) return true;
    row = (row + 1) & (31);
    if (row) return true; 
    qdr = 4;
    return true;
  }
  return false;
}
//-----------------------------------------------------------------------------------------
// Compute byte count
//------------------------------------------------------------------------------------------
COAST_VERTEX *CExport::CoastCount(COAST_VERTEX *pol)
{ U_SHORT nbv = pol->Nbv;                     // Number of vertices
  rec.nbv += nbv;                             // Number of vertices recorded
	total += (nbv * sizeof(COAST_VERTEX));
  return pol + nbv;
}//-----------------------------------------------------------------------------------------
//  Export this File in Database
//-----------------------------------------------------------------------------------------
void CExport:: ExportCoastFile(char *name,int gx,int gz)
{ SqlMGR *sqm = globals->sqm;
  U_INT   ind = (gx << 16) | gz;
  C_SEA  *sea = new C_SEA(ind,0);
  //----Init indice computation ----------------------------------------------
  bx  = gx << 1;                                         // base QGT X indice
  bz  = gz << 1;                                         // Base QGT Z indice
  qx  = bx;
  qz  = bz;
  row = 0;
  col = 0;
  qdr = 0;
  //--------------------------------------------------------------------------
  while (NextCoastKey())
  { char *data = sea->GetCoast(ax,az);
    if (0 == data)  continue;
    rec.qtk  = (qx << 16) | qz;                            // QGT (X-Z)Key
    rec.dtk  = (ax << 16) | az;                            // DET (X-Z)Key
    rec.data = data;
    rec.nbv  = 0;
		COAST_HEADER *hdr = (COAST_HEADER*)data;
    COAST_VERTEX *pol = (COAST_VERTEX*)(data + sizeof(COAST_HEADER));
    int nbp = hdr->nbp;
    total   = sizeof(COAST_HEADER);
    for (int k = 0; k != nbp; k++) pol = CoastCount(pol);
    rec.dim = total;
    sqm->WriteCoastRec(rec);
  }
	delete sea;
  return;
}
//=========================================================================================
//  Export 3D models
//=========================================================================================
//-----------------------------------------------------------------------------------------
//  Export all scenery from all tiles
//-----------------------------------------------------------------------------------------
void CExport::Export3Dmodels()
{ int red = 0;
  if (0 == m3d)       return;
  Mode  = EXP_MW3D;
  State = EXP_MSG1;
  mName = 0;
  mRed  = 0;
  mCnt  = 0;
	count = 0;
  GetIniVar("SQL", "CrunchModels", &red);
  mRed  = red;
  minp  = 300;                    // default minimum polygons to keep
  GetIniVar("SQL", "MinimumPolygons",&red);
  minp  = red;
  globals->appState = APP_EXPORT;
  InitModelPosition();
  globals->fui->SetNoticeFont(&globals->fonts.ftmono20);
	pif.mode= 0;								// Adding mode
	SCENE("========== START OF MODEL EXPORT ==========");
  return;
}
//-----------------------------------------------------------------------------------------
//  Init model position and orientation (at (0,0,0) facing Y)
//-----------------------------------------------------------------------------------------
void CExport::InitModelPosition()
{ SPosition pos;  
  pos.lat = pos.lon = pos.alt = 0;
  globals->geop = pos;
  CVector ori(0,0,0);
  globals->iang = ori;
  globals->dang = ori;
  return;
}

//-----------------------------------------------------------------------------------------
//  Model preparation:
//	 Build a model and prepare to compute the 3 LOD details models
//-----------------------------------------------------------------------------------------
bool CExport::Prepare3Dmodel(char *fn, char opt)
{ char tx[MAX_PATH];
  char *op   = (mRed)?("CRUNCHING"):("STORING");
  char *slh  = strchr(fn,'/');
  char *name = slh+1;
	//------ For debug only ----------------------------------
	//	if (strcmp(name,"DYMX5515.SMF") == 0)
	//	int a = 0;
	//---------------------------------------------------------
	mCnt++;
	sprintf(tx,"TESTING %05d: %s in DATABASE",mCnt,mName);
  globals->fui->DrawNoticeToUser(tx,200);
  if (globals->sqm->Check3DModel(name)) return false;     // Already in DB
  Mod   = new C3Dmodel(name);
  Mod->LoadPart("MODELS");
  if (!Mod->IsOK())                     return false;
  Polys = new CPolyShop;
  Polys->SetModel(Mod);
  face = Polys->GetNbFaces();
  sprintf(tx,"%s No %05d (%05d triangles):   %s",op,mCnt,face,mName);
  globals->fui->DrawNoticeToUser(tx,200);
	if (opt)  DrawModel();
  return true;
}
//-----------------------------------------------------------------------------------------
//  Write the 3D model
//	NOTE:  Only one texture is expected in the model
//-----------------------------------------------------------------------------------------
void CExport::WriteTheModel()
{ char *slh  = strchr(mName,'/');
  char *name = slh+1;
  //---Write texture in database ----------------
  Mod->GetParts(this,pif);
  if (mRed) Polys->ModelStandard(minp);     // Reduce model
  //--- Write Part in database ------------------
  Export3DMlodQ(name,0);
  Export3DMlodQ(name,1);
  Export3DMlodQ(name,2);
  Export3DMlodQ(name,3);
	SCENE("EXPORT %s",mName);
	count++;
	//--- Delete resources -----------------------
  delete Polys;
  Polys = 0;
  delete Mod;
  Mod   = 0;
  return;
}

//-----------------------------------------------------------------------------------------
//  Process one part from model
//	mode 0 => Add texture to database
//	mode 1 => Update texture in database
//-----------------------------------------------------------------------------------------
void CExport::OneM3DPart(M3D_PART_INFO &pif)
{ char *dir = (pif.mode == 0)?("ART"):("UPDATES");
	//---Fill texture request --------------------------------
  inf.mADR  = 0;
  inf.wd    = 0;
  inf.ht    = 0;
  inf.bpp   = 4;
  sprintf(inf.path,"%s/%s",dir,pif.ntex);
  char *dot = strstr(pif.ntex,".");
  if (0 == dot)                               return;
  if (strcmp(dot,".TIF") == 0)  globals->txw->Get3DTIF(&inf);
  if (strcmp(dot,".RAW") == 0)  globals->txw->Get3DRAW(&inf);
	strncpy(inf.name,pif.ntex,TC_TEXTURE_NAME_NAM);
	//--- Validate texure -------------------------------------
  if (0 == inf.mADR)                          return;
	if (0 == pif.ntex)                          return;
  //---Check if in database --------------------------------
  if (globals->sqm->Check3DTexture(pif.ntex)) return;
  //---Enter in database -----------------------------------
  globals->sqm->WriteM3DTexture(inf);
  delete [] inf.mADR;
  return;
}
//-----------------------------------------------------------------------------------------
//  Draw current model
//-----------------------------------------------------------------------------------------
void CExport::DrawModel()
{ double ang = DegToRad(double(16));
  double hm = Mod->MaxExtend() * 0.5;
  double dr = hm / tan(ang);
  if (dr < 50)  dr = 50;
  Cam->SetRange(dr);
  Cam->StartShoot(0);
  //---------------------------------------------------------------------
  glPolygonMode(GL_FRONT,GL_FILL);
  glPushClientAttrib (GL_CLIENT_ALL_ATTRIB_BITS);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnable(GL_TEXTURE_2D);
  if (Mod)  Mod->Draw(0);
  glPopClientAttrib();
  //---------------------------------------------------------------------
  Cam->StopShoot();
  //----------------------------------------------------------------
  return;
}
//-----------------------------------------------------------------------------------------
//  Export a Load Queue
//-----------------------------------------------------------------------------------------
void CExport::Export3DMlodQ(char * name, int n)
{ float    top = Mod->GetTop();
  float    bot = Mod->GetGround();
  C3DPart *prt = 0;
  for (prt = Mod->PopPart(n); prt != 0; prt = Mod->PopPart(n))
  { prt->SetTop(top);
    prt->SetBot(bot);
    globals->sqm->Write3Dmodel(name,prt);
    delete prt;
  }
  return;
}
//-----------------------------------------------------------------------------------------
//  Step 0:  Display first message
//-----------------------------------------------------------------------------------------
void CExport::M3DMsgIntro()
{ char *msg = "GATHERING ALL SCENERY FILES";
  globals->fui->DrawNoticeToUser(msg,100);
  return;
}
//-----------------------------------------------------------------------------------------
//  Close all sceneries
//-----------------------------------------------------------------------------------------
void CExport::CloseSceneries()
{ char *mtm = "END OF EXPORT";
  for   (int z=0; z<512; z++)
  {  for(int x=0; x<512; x++)
      { U_INT key = QGTKEY(x,z);
				globals->scn->Deregister (key);
      }
    }
  globals->fui->DrawNoticeToUser(mtm,500);
  return;
}
//-----------------------------------------------------------------------------------------
//  Dispatch export M3D action
//-----------------------------------------------------------------------------------------
int  CExport::ExecuteMOD()
{ char fn[PATH_MAX];
  char *mte = "FREEING SCENERIES";
  char *mrx = "PAUSE. Type any key to continue. To stop type s";
	Clear			= 0;
  switch(State) {
		//--- Intro message ------------------
		case EXP_MSG1:
			M3DMsgIntro();
			return EXP_INIT;					
		//--- Collect all sceneries ----------
		case EXP_INIT:
			Clear = 1;
			return EXP_FBIN;
		//--- Init for BIN files ----------
		case EXP_FBIN:
			sprintf(fn,"MODELS/*.BIN");
			mName = (char*)pfindfirst (&globals->pfs,fn);
			Clear = 1;
			return EXP_NBIN;
		//--- Prepare one BIN file ------------
		case EXP_NBIN:
			if (0 == mName)		{Clear = 1;		return EXP_FSMF;} 
			if (Prepare3Dmodel(mName,1))		return EXP_WBIN;
			mName = (char*)pfindnext (&globals->pfs);
			Clear = 1;
			return EXP_NBIN;
		//--- Write the bin model --------------
		case EXP_WBIN:
			WriteTheModel();
			mName = (char*)pfindnext (&globals->pfs);
			Clear = 1;
			return EXP_NBIN;
		//--- Init for SMF files --------------
		case EXP_FSMF:
			sprintf(fn,"MODELS/*.SMF");
			mName = (char*)pfindfirst (&globals->pfs,fn);
			return EXP_NSMF;
		//--- Prepare one SMF file -------------
		case EXP_NSMF:
			if (0 == mName)		{Clear = 1; return EXP_END;} 
			if (Prepare3Dmodel(mName,1))	return EXP_WSMF;
			mName = (char*)pfindnext (&globals->pfs);
			Clear = 1;
			return EXP_NSMF;
		//--- Write one SMF file --------------
		case EXP_WSMF:
			WriteTheModel();
			mName = (char*)pfindnext (&globals->pfs);
			Clear = 1;
			return EXP_NSMF;
		//--- End of process ------------------
		case EXP_END:
			CloseSceneries();
			Clear = 1;
			return EXP_OUT;
		//---- Set pause mode -----------------
		case EXP_RLAX:
			globals->fui->DrawNoticeToUser(mrx,10);
			return 0;
  }       //END OF SWITCH
  //------- STOP SIMU ------------------
	globals->appState = APP_EXIT_SCREEN;
	Clear = 1;
  return 0;
  }
//------------------------------------------------------------------------------------
//  Keyboard intercept
//------------------------------------------------------------------------------------
void CExport::KeyW3D(U_INT key,U_INT mod)
{ if (State != EXP_RLAX) {rStat = State; State = EXP_RLAX;          return;}
  //--- skey will stop the processus --------------------------------
  if (0x73 == key)       {globals->appState = APP_EXIT;             return;}
  //----Resume the processus ----------------------------------------
  State = rStat;
  return;
}
//=========================================================================================
//  Update 3D models
//=========================================================================================
void CExport::Update3DModels()
{	char fn[PATH_MAX];
	//--- Locate all SMF models -----------------
  sprintf(fn,"Updates/*.SMF");
  mName = (char*)pfindfirst (&globals->pfs,fn);
  while (mName)
  { char *dot = strrchr(mName,'/');
		if (dot) UpdateTheModel(dot+1);
		mName = (char*)pfindnext (&globals->pfs);
  }
	//--- Locate all BIN models -----------------
  sprintf(fn,"Updates/*.BIN");
  mName = (char*)pfindfirst (&globals->pfs,fn);
  while (mName)
  { char *dot = strrchr(mName,'/');
		if (dot) UpdateTheModel(dot+1);
		mName = (char*)pfindnext (&globals->pfs);
  }
	//-------------------------------------------
	globals->appState = APP_EXIT_SCREEN;
	return;
}
//-------------------------------------------------------------------
//	Load the model
//-------------------------------------------------------------------
void CExport::LoadUpdModel(char *name)
{	pif.mode	= 1;								// Update mode
	Mod   = new C3Dmodel(name);
  Mod->LoadPart("UPDATES");
  if (!Mod->IsOK())
	{	delete Mod;
		Mod	= 0;
		return;
	}
	//--- Init polyshop reduction --------------------- 
	Polys = new CPolyShop;
  Polys->SetModel(Mod);
  face = Polys->GetNbFaces();
	WriteTheModel();
	return;
}
//-------------------------------------------------------------------
//	Update the model
//-------------------------------------------------------------------
void CExport::UpdateTheModel(char *name)
{	globals->sqm->DeleteM3DModel(name);
	LoadUpdModel(name);
	return;
}
//=========================================================================================
//  Export Taxiways
//=========================================================================================
//-----------------------------------------------------------------------------------------
//    CALL BACK FUNCTION
//-----------------------------------------------------------------------------------------
void GetTaxiway(CmHead *obj)
{ CAirport *apt = (CAirport *)obj;
  globals->exm->ExportOneTMS(apt);
  return;
}
//-----------------------------------------------------------------------------------------
//  Request from MENU to export taxiways
//-----------------------------------------------------------------------------------------
void CExport::ExportTaxiways()
{ if (0 == txy) return;
  //---Collect all TMS files ---------------
  apo = 0;
  apt = 0;
  globals->sqm->GetAllAirports(GetTaxiway);
  return;
}

//-----------------------------------------------------------------------------------------
//    Export taxiway for one airport
//-----------------------------------------------------------------------------------------
void CExport::ExportOneTMS(CAirport *apt)
{ char *iden = apt->GetIdentity();
  char *key = apt->GetKey();
  if (0 == *iden)   return;
  char fn[PATH_MAX];
  sprintf(fn,"DATA/%s.TMS",iden);
  if (!pexists(&globals->pfs,fn)) return;
  //----Create Airport Object for export ------------------
  apo = new CAptObject(apt);
  CDataTMS tms(apo);                           // TMS decoder
  tms.DecodeBinary(fn);                        // Create all queues
  //----Export all pavements ------------------------------
  ExportPaveQ(key);
  ExportEdgeQ(key);
  ExportCentQ(key);
  ExportLiteQ(key,TC_GRN_LITE);
  ExportLiteQ(key,TC_BLU_LITE);
  //-------------------------------------------------------
  delete apo;
  delete apt;
  return;
}

//-----------------------------------------------------------------------------------------
//  Concatenate vertex into one table
//  Source pavement are packed into a more compact pavemetn structure (up to 498 vertices)
//  Polygons from source are transformed in triangles
//-----------------------------------------------------------------------------------------
CPaveRWY *CExport::BuildPave(CPaveQ *qhd)
{ int nbv       = 498;
  CPaveRWY *src = 0;
  CPaveRWY *dst = new CPaveRWY(nbv);
  int       tot = 0;
  int       nbs = 0;
  dst->ClearCount(); 
  while (!qhd->IsEmpty())
  { src     = qhd->GetFirst();
    nbs     = (src->GetNBVT() - 2) * 3;     // Vertex count with triangle expansion
    tot    += nbs;                          // Check for enough room
    if (tot > nbv)  break;                  // Stop packing
    src     = qhd->Pop();                   // Pop one source
    dst->AppendAsTriangle(src);             // Copy source to destination
    delete src;                             // delete pavement source
  }
  return dst;
}
//-----------------------------------------------------------------------------------------
//  Concatenate spots of same color into one table
//-----------------------------------------------------------------------------------------
CBaseLITE *CExport::BuildLite(CLiteQ *qhd,U_CHAR col)
{ CBaseLITE *src = qhd->GetFirst();
  int nbl        = src->GetNbSpot();
  int nbs        = (nbl < 498)?(498):(nbl);
  CBaseLITE *dst = new CBaseLITE(LS_OMNI_LITE,nbs);
  int       tot  = 0;
  int       nbx  = 0;
  dst->ClearCount();                        // Set Vertex count to 0
  dst->SetColor1(col);
  while (!qhd->IsEmpty())
  { src     = qhd->GetFirst();              // Head of Queue
    if (col != src->GetColor1())  break;    // Color change
    tot    += src->GetNbSpot();             // Check for enough room
    if (tot > nbs)  break;                  // OK for now
    src     = qhd->Pop();                   // Pop one source
    dst->Append(src);                       // Copy source to destination
    delete src;                             // delete pavement source
  }
  return dst;
}
//-----------------------------------------------------------------------------------------
//    Export one queue
//-----------------------------------------------------------------------------------------
void CExport::ExportPaveQ(char *key)
{ CPaveQ *qhd = apo->GetPavQ();
  CPaveRWY *pave = 0;
  while (!qhd->IsEmpty())
  { pave = BuildPave(qhd);
    pave->SetType (PAVE_TYPE_PAVE);
    pave->SetUsage(PAVE_USE_TAXI);
    //----Write the record --------------------
    if (0 != pave->GetNBVT())  globals->sqm->WritePavement(pave,key);
    delete pave;
  }
  return;
}
//-----------------------------------------------------------------------------------------
//    Export one queue
//-----------------------------------------------------------------------------------------
void CExport::ExportEdgeQ(char *key)
{ CPaveQ *qhd = apo->GetEdgQ();
  CPaveRWY *pave = 0;
  while (!qhd->IsEmpty())
  { pave = BuildPave(qhd);
    pave->SetType (PAVE_TYPE_EDGE);
    pave->SetUsage(PAVE_USE_TAXI);
    //----Write the record --------------------
    if (0 != pave->GetNBVT()) globals->sqm->WritePavement(pave,key);
    delete pave;
  }
  return;
}
//-----------------------------------------------------------------------------------------
//    Export one queue
//-----------------------------------------------------------------------------------------
void CExport::ExportCentQ(char *key)
{ CPaveQ *qhd = apo->GetCntQ();
  CPaveRWY *pave = 0;
  while (!qhd->IsEmpty())
  {  pave = BuildPave(qhd);
    pave->SetType (PAVE_TYPE_CENT);
    pave->SetUsage(PAVE_USE_TAXI);
    //----Write the record --------------------
    if (0 != pave->GetNBVT()) globals->sqm->WritePavement(pave,key);
    delete pave;
  }
  return;
}
//-----------------------------------------------------------------------------------------
//    Export light queue
//-----------------------------------------------------------------------------------------
void CExport::ExportLiteQ(char *key,U_CHAR col)
{ CLiteQ *qhd = apo->GetLitQ();
  CBaseLITE *lite = 0;
  CBaseLITE *head = 0;
  while (!qhd->IsEmpty())
  { head = qhd->GetFirst();
    if (col != head->GetColor1()) return;   // End of color
    lite = BuildLite(qhd,col);
    //----Write the record --------------------
    if (0 != lite->GetNbSpot()) globals->sqm->WriteTaxiLigth(lite,key);
    delete lite;
  }
  return;
}
//=========================================================================================
//  Export Generic texture
//=========================================================================================
void CExport::ExportGenTextures()
{ if (0 == gtx)   return;
  //--- Write all masks -----------------------------
  WriteMskTexture("RGHBLEND4",TC_MEDIUM_DIM);
  WriteMskTexture("RGHBLEND5",TC_HIGHTR_DIM);
  WriteMskTexture("BOTBLEND4",TC_MEDIUM_DIM);
  WriteMskTexture("BOTBLEND5",TC_HIGHTR_DIM);
  WriteMskTexture("CNRBLEND4",TC_MEDIUM_DIM);
  WriteMskTexture("CNRBLEND5",TC_HIGHTR_DIM);
  
  //--- Write runway textures ------------------------
  WritePNGTexture("RWYASFH",0);
  WritePNGTexture("RWYCEMH",0);
  WritePNGTexture("RWYOTRH",0);
  //--------------------------------------------------
  WritePNGTexture("TAXICMNT",0);
  //--------------------------------------------------
  WritePNGTexture(LiteNAM[TC_FLR_TEX],1);
  WritePNGTexture(LiteNAM[TC_WHI_TEX],1);
  WritePNGTexture(LiteNAM[TC_RED_LITE],1);
  WritePNGTexture(LiteNAM[TC_GRN_LITE],1);
  WritePNGTexture(LiteNAM[TC_BLU_LITE],1);
  WritePNGTexture(LiteNAM[TC_YEL_LITE],1);
  WritePNGTexture(LiteNAM[TC_GLO_TEX],1);
  //--- Write generic textures -------------------------------
  char fn[PATH_MAX];
  sprintf(fn,"SYSTEM/GLOBE/*.RAW");
  char* name = (char*)pfindfirst (&globals->pfs,fn);
  while (name)
  { DecodeTexture(name);
    name = (char*)pfindnext (&globals->pfs);
  }
  return;
}
//--------------------------------------------------------------------------------------
//  Decode the texture
//  TODO : Ignore the 64 resolution
//--------------------------------------------------------------------------------------
void CExport::DecodeTexture(char *name)
{ strncpy(inf.path,name,32);
  char *dot = strrchr(name,'.');
  char *typ = dot - 1;
  char *mod = typ - 1;
  char *tab =  0;
  int   dim =  0;
  U_CHAR res = 0;
  char *rnm = strrchr(name,'/') + 1;
  if (*typ == '1')  return;
  if (*mod == '4')  {res = TC_MEDIUM; tab = "DAY128"; dim = 128;}
  if (*mod == '5')  {res = TC_HIGHTR; tab = "DAY256"; dim = 256;}
  if (*typ == 'N')  {res = TC_MEDIUM, tab = "NIT128"; dim = 128;}
  if (0 == res)     return;                 // Ignore other resolution
  CArtParser img(res);
  img.DontAbort();
  inf.mADR  = img.LoadRaw(inf,0);         // Load texture elements
  //-----Fill the texture info ----------------------------------
  inf.bpp   = 4;
  inf.xOBJ  = 0;
 *dot       = 0;
  strncpy(inf.path,rnm,TC_LAST_INFO_BYTE);
  inf.path[TC_LAST_INFO_BYTE]  = 0;
  //------Write the file in Database -----------------------------
  globals->sqm->WriteGenTexture(inf,tab);
  //--------------------------------------------------------------
  delete [] inf.mADR;
  return;
}
//--------------------------------------------------------------------------------------
//  Decode a Mask
//--------------------------------------------------------------------------------------
void CExport::WriteMskTexture(char *name,int side)
{ char path[PATH_MAX];
  sprintf(path,"SYSTEM/GLOBE/%s.RAW",name);
  char *dot     = strrchr(path,'.');
  char *rnm     = strrchr(path,'/') + 1;
  //---fill texture info ----------------------------
  inf.mADR  = globals->txw->LoadMSK(path,side);
  inf.bpp   = 1;
  inf.wd    = side;
  inf.ht    = side;
 *dot       = 0;
  strncpy(inf.path,rnm,TC_LAST_INFO_BYTE);
  inf.path[TC_LAST_INFO_BYTE] = 0;
  globals->sqm->WriteAnyTexture(inf);
  delete [] inf.mADR;
  return;
}
//--------------------------------------------------------------------------------------
//  Write a runway texture
//--------------------------------------------------------------------------------------
void CExport::WriteRwyTexture(char *name,char seg,char tsp)
{ TEXT_INFO xds;
  sprintf(xds.path,"ART/%sR%c.PNG",name,seg);
  CArtParser img(0);
  char *dot     = strrchr(xds.path,'.');
  char *rnm     = strrchr(xds.path,'/') + 1;
  //---Fill texture info ----------------------------
  inf.mADR  = img.GetAnyTexture(xds);
  inf.bpp   = 4;
  inf.wd    = xds.wd;
  inf.ht    = xds.ht;
  inf.xOBJ  = 0;
 *dot       = 0;
  strncpy(inf.path,rnm,TC_LAST_INFO_BYTE);
  inf.path[TC_LAST_INFO_BYTE] = 0;
  globals->sqm->WriteAnyTexture(inf);
  delete [] inf.mADR;
  return;
}
//--------------------------------------------------------------------------------------
//  Write a runway texture
//--------------------------------------------------------------------------------------
void CExport::WritePNGTexture(char *name,char tsp)
{ TEXT_INFO xds;
  sprintf(xds.path,"ART/%s.PNG",name);
  CArtParser img(0);
  char *dot   = strrchr(xds.path,'.');
  char *rnm   = strrchr(xds.path,'/') + 1;
  //-----Fill texture info --------------------------
  inf.mADR  = img.GetAnyTexture(xds);
  inf.bpp   = 4;
  inf.wd    = xds.wd;
  inf.ht    = xds.ht;
  inf.xOBJ  = 0;
 *dot       = 0;
  strncpy(inf.path,rnm,TC_LAST_INFO_BYTE);
  inf.path[TC_LAST_INFO_BYTE] = 0;
  globals->sqm->WriteAnyTexture(inf);
  delete [] inf.mADR;
  return;
}
//=========================================================================================
//  Compress Specific textures
//=========================================================================================
void CExport::CompressTextures()
{ //--- scan all quarter global tiles -----------------------
	for (	int	x=3; x<512;x++)	for(int z=0; z<512;z++)	ScanDirectory(x,z);
	return;
}
//-----------------------------------------------------------
//	Search raw files for this Global Tile
//-----------------------------------------------------------
void CExport::ScanDirectory(int x, int z)
{	char path[PATH_MAX];
	sprintf(path,"DATA/D%03d%03d/*.RAW",x,z);
	//--- compute scenery center ----------------
	double sLat = GetQgtSouthLatitude(z);
  double nLat = GetQgtNorthLatitude(z);
  SPosition pos;
	pos.lat =  0.5 * (sLat + nLat);
  pos.lon =  FN_ARCS_FROM_QGT(x + 0.5);
  pos.alt =  0;
	//---Register the QGT for scenery --------------------
  //CSceneryDBM::Instance().Register (pos);

	char* name = (char*)pfindfirst (&globals->pfs,path);
	//--- Process all files -------------------------------
  while (name)
  { int a = 0;
    name = (char*)pfindnext (&globals->pfs);
  }
	//--------Deregister Scenery ---------------------------
 // CSceneryDBM::Instance().Deregister(pos);
	return;
}
//=========================================================================================
//  Export Time Slice
//=========================================================================================
int CExport::TimeSlice(float dT)
{ if (EXP_MW3D == Mode)		State = ExecuteMOD();
  if (EXP_TRNF == Mode)		State =	ExecuteTRN();
	if (EXP_WOBJ == Mode)		State = ExecuteOBJ();
  globals->fui->DrawOnlyNotices();
  return Clear;
}
//=========================================================================================
//  Export Keyboard
//=========================================================================================
void CExport::Keyboard(U_INT key,U_INT mod)
{ if (EXP_MW3D == Mode) KeyW3D(key,mod);
  return;
}
//=========================================================================================
//  Export Scenery Object
//=========================================================================================
void CExport::ExportSceneryOBJ()
{	if (0 == wob)       return;
	if (!sqm->SQLobj())	return;
	count	= 0;
  Mode  = EXP_WOBJ;
  State = EXP_OBJ_INIT;
  fName = 0;
  globals->appState = APP_EXPORT;
	globals->fui->SetNoticeFont(&globals->fonts.ftmono20);
	mCnt	= 0;
	eof		= 0;
	SCENE("================ START EXPORT =================");
	return;
}
//---------------------------------------------------------------------------
//  Set initial message
//---------------------------------------------------------------------------
void CExport::InitOBJmsg()
{	char *msg = "MOUNTING SCENERY FILES ";
	globals->fui->DrawNoticeToUser(msg,200);
	Clear = 1;
	return;
}
//-----------------------------------------------------------------------------------------
//  Build OBJ full name
//-----------------------------------------------------------------------------------------
void CExport::BuildOBJname(char *fn)
{	if (0 == fn)	return;
	int nf  =  sscanf(fn,"DATA/D%3d%3d/%*[^.].S%1d%1d",&gx,&gz,&bx,&bz);
	if (nf != 4)	return;
	//--- Compute QGT key -------------------------------------
	qx = (gx << 1) + bx;
	qz = (gz << 1) + bz;
	qKey  = QGTKEY(qx, qz);
	//--- build pod name --------------------------------------
	char *dbn = fn + strlen("DATA/");
	char *dbp	= GetSceneryPOD(pod);
	_snprintf(podN,(PATH_MAX-1),"(%s)-(%s)",dbn,dbp);
	fName	= fn;
	//---------------------------------------------------------
	char msg[1024];
	_snprintf(msg,1023,"PROCESS: %s",podN);
	globals->fui->DrawNoticeToUser(msg,200);
	return;
}

//-----------------------------------------------------------------------------------------
//  Find the first file
//-----------------------------------------------------------------------------------------
void CExport::GetFirstOBJ()
{ char *fp	= "DATA/D*/*.S*";
	char *fn	= pfindfirst(pfs,fp, &pod);
	fName			= 0;
	if (0 == fn)	eof = 1;
	BuildOBJname(fn);
	return;
}

//-----------------------------------------------------------------------------------------
//  Find the next file
//-----------------------------------------------------------------------------------------
void CExport::GetNextOBJ()
{ char *fn  = pfindnext(pfs,&pod);
	fName			= 0;
	if (0 == fn)	eof = 1;
	BuildOBJname(fn);
	return;
}
//-----------------------------------------------------------------------------------------
//  Write the file
//-----------------------------------------------------------------------------------------
void  CExport::WriteOBJ()
{	if (0 == fName)		return;
	//--- check if file already in data base
	bool in		= sqm->FileInOBJ(podN);
	if (in)					 return;
	//--- Write file name in database ----------------------
	rowid = sqm->WriteOBJname(podN);
	//--- Insert TRN in database ---------------------------
	ExportOBJ(fName);
	count++;
	SCENE("Export %s from %s:", fName,pod);
	Clear	= 1;
	return;
}
//-----------------------------------------------------------------------------------------
//  Export the file
//-----------------------------------------------------------------------------------------
void CExport::ExportOBJ(char *fn)
{	C3Dfile   sny(0,0);
  sny.Decode(fn,0);					// Decode all objects
	//----Write all objects -------------------------------
  CWobj *obj = sny.GetWOBJ();
  while (obj)
  {	globals->sqm->WriteWOBJ(qKey,obj,rowid);
    delete obj;
    obj = sny.GetWOBJ();
    }
	return;
}
//---------------------------------------------------------------------------
//  Execute step for OBJ export
//---------------------------------------------------------------------------
int CExport::ExecuteOBJ()
{	 Clear = 0;
	 switch (State)	{
		//--- Initial message ------------
		case EXP_OBJ_INIT:
			InitOBJmsg();
			return EXP_OBJ_MOUNT;
		//--- Mount scenery---------------
		case EXP_OBJ_MOUNT:
			globals->scn->MountAll();
			return EXP_OBJ_FFILE;
		//--- Search first file ----------
		case EXP_OBJ_FFILE:
			GetFirstOBJ();
			return EXP_OBJ_WRITE;
		//--- Search Next file ----------
		case EXP_OBJ_NFILE:
			GetNextOBJ();
			return EXP_OBJ_WRITE;
		//--- Write the file ------------
		case EXP_OBJ_WRITE:
			if (eof)	return EXP_OBJ_EXIT;
			WriteOBJ();
			return EXP_OBJ_NFILE;
	 }
	SCENE("========== Exported: %05d files ==========",count);
	globals->appState = APP_EXIT_SCREEN;
	return 0;
}
//=========================================================================================
//  Call back to check if file is in database
//=========================================================================================
//---------------------------------------------------------------------------
// Check for all files already in database
//---------------------------------------------------------------------------
void CExport::CheckSceneryFiles()
{ char *dir = "DATA/D*/*.S*";
	globals->scn->MountAll();
  SCENE("===== SEARCH FOR SCENERY FILES ALREADY IN DATABASE OBJ ===================");
  char *n1 = "D255167/TM_FRW_B.S01";
	char* name = (char*)pfindfirst (&globals->pfs,dir);
	while (name)
	{  CheckThisFile(name);
		 name		 = (char*)pfindnext (&globals->pfs);
	}
	SCENE("===== SEARCH FOR SCENERY FILES ENDED =====================================");
	return;
}
//---------------------------------------------------------------------------
// Check for all files already in database
//---------------------------------------------------------------------------
void CExport::CheckThisFile(char *fn)
{ char *n1 = fn;
	if (strncmp(fn,"DATA/",5) == 0) n1 +=5;
	int   nb = globals->sqm->SearchWOBJ(n1);
	if (0 == nb)		return;
	//---------------------------------------------------------
	SCENE("  File Already in SQL base: %s",fn);
	return;
}
//=========================================================================================
//  Export Scenery
//	Scan all data directories and look for *.TRN files
//=========================================================================================
//---------------------------------------------------------------------------
//  Export all TRN files
//---------------------------------------------------------------------------
void CExport::ExportAllTRNs()
{	if (0 == trn)       return;
	if (!sqm->SQLelv())	return;
	count = 0;
  Mode  = EXP_TRNF;
  State = EXP_TRN_INIT;
  fName = 0;
  globals->appState = APP_EXPORT;
	globals->fui->SetNoticeFont(&globals->fonts.ftmono20);
	mCnt	= 0;
	eof		= 0;
	SCENE("================ START EXPORT =================");
	return;
}
//-----------------------------------------------------------------------------------------
//  Initial step
//-----------------------------------------------------------------------------------------
void CExport::InitTRNmsg()
{	char *msg = "MOUNTING SCENERY FILES ";
	globals->fui->DrawNoticeToUser(msg,200);
	Clear = 1;
	return;
}
//-----------------------------------------------------------------------------------------
//  Dispatch export TRN action
//-----------------------------------------------------------------------------------------
int  CExport::ExecuteTRN()
{ Clear = 0;
	switch (State)	{
		//--- Initial state -----------------------
		case EXP_TRN_INIT:
			  InitTRNmsg();
				return EXP_TRN_MOUNT;
		//--- Mount scenry files ------------------
		case EXP_TRN_MOUNT:
				globals->scn->MountAll();
				return EXP_TRN_FFILE;
		//--- Get first file ----------------------
		case EXP_TRN_FFILE:
			  GetFirstTRN();
				if (eof)		return EXP_TRN_EXIT;
				else				return EXP_TRN_WRITE;
		//--- Get next file -----------------------
		case EXP_TRN_NFILE:
				GetNextTRN();
				if (eof)		return EXP_TRN_EXIT;
				else				return EXP_TRN_WRITE;
		//--- Write a file ------------------------
		case EXP_TRN_WRITE:
				WriteTRN();
				Clear = 1;
			  return EXP_TRN_NFILE;
	}
	SCENE("========== Exported: %05d files ==========",count);
	globals->appState = APP_EXIT_SCREEN;
	return 0;
}
//-----------------------------------------------------------------------------------------
//  Build TRN full name
//-----------------------------------------------------------------------------------------
int CExport::BuildTRNname(char *fn)
{	if (0 == fn)	return 0;
	int nf  =  sscanf(fn,"DATA/D%3d%3d/G%1d%1d.TRN",&gx,&gz,&bx,&bz);
	if (nf != 4)	return 0;
	//--- Compute QGT key -------------------------------------
	qx = (gx << 1) + bx;
	qz = (gz << 1) + bz;
	qKey  = QGTKEY(qx, qz);
	//--- build pod name --------------------------------------
	char *dbn = fn + strlen("DATA/");
	char *dbp = GetSceneryPOD(pod);
	_snprintf(podN,(PATH_MAX-1),"(%s)-(%s)",dbn,dbp);
	fName	= fn;
	//---------------------------------------------------------
	char msg[1024];
	_snprintf(msg,1023,"PROCESS: %s",podN);
	globals->fui->DrawNoticeToUser(msg,200);
	return 1;
}
//-----------------------------------------------------------------------------------------
//  Find the first file
//-----------------------------------------------------------------------------------------
int CExport::GetFirstTRN()
{ char *fp	= "DATA/D??????/G??.TRN";
	char *fn	= pfindfirst(pfs,fp, &pod);
	fName			= 0;
	if (0 == fn)	eof = 1;
	BuildTRNname(fn);
	return EXP_TRN_WRITE;
}

//-----------------------------------------------------------------------------------------
//  Find the next file
//-----------------------------------------------------------------------------------------
int CExport::GetNextTRN()
{ char *fn  = pfindnext(pfs,&pod);
	fName			= 0;
	if (0 == fn)	eof = 1;
	BuildTRNname(fn);
	return EXP_TRN_WRITE;
}

//-----------------------------------------------------------------------------------------
//  Find the first file
//-----------------------------------------------------------------------------------------
void  CExport::WriteTRN()
{	if (0 == fName)		return;
	//--- check if file already in data base
	bool in		= sqm->FileInELV(podN);
	if (in)					 return;
	//--- Write file name in database ----------------------
	rowid = sqm->WriteTRNname(podN);
	//--- Insert TRN in database ---------------------------
	ExportTRN(fName);
	count++;
	SCENE("Export %s from %s:", fName,pod);
	return;
}

//---------------------------------------------------------------------------
//  Export a TRN file
//---------------------------------------------------------------------------
void CExport::ExportTRN(char *fn)
{ SStream s;                                // Stream file
  if (!OpenRStream (fn, s))  return;
  ftrn = new C_TRN(0,0);
	ftrn->Export();
  ReadFrom (ftrn, &s);
  CloseStream (&s);
  //--- Process all Super tile -----------------------------
	for (short   sz = 0; sz != TC_SUPERT_PER_QGT; sz++)
  { for (short sx = 0; sx != TC_SUPERT_PER_QGT; sx++)
		{	C_STile *asp = ftrn->GetSupTile(sx,sz);
			ExportSUP(asp);
		}
	}
	delete ftrn;
	ftrn	= 0;
	return;
}
//---------------------------------------------------------------------------
//  Export Super Tile description
//---------------------------------------------------------------------------
void CExport::ExportSUP(C_STile *asp)
{	asp->SetKey(qKey);
  globals->sqm->WriteElevationTRN(*asp,rowid);
	//--- Now, export all detail tiles from supertile ------
	
	TRN_HDTL *hd = asp->PopDetail();
	while (hd)
	{	globals->sqm->WriteElevationDET(qKey,*hd,rowid);
		delete hd;
		hd	= asp->PopDetail();
	}
	
	return;
}

//===================================END OF FILE ==========================================