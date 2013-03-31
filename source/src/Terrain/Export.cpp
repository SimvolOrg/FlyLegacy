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
#include "../Include/ScenerySet.h"
#include "../Include/Export.h"
#include "../Include/SqlMGR.h"
#include "../Include/FileParser.h"
#include "../Include/Airport.h"
#include "../Include/Taxiway.h"
#include "../Include/TerrainTexture.h"
#include "../Include/Reductor.h"
#include "../Include/Fui.h"
#include "../Include/Pod.h"
#include "../Include/Compression.h"
//=============================================================================
extern char *RunwayNAM[];
extern char *LiteNAM[];
extern U_SHORT SideRES[];
extern U_INT   UtilRES[];
//=============================================================================
//  Write a flat record as CSV (comma separated field) for SQL base
//  sep is the separator
//=============================================================================
void CAirport::WriteCVS(U_INT No,U_INT gx,U_INT gz,char *sep,CStreamFile &stf)
{ CStreamFile* sf = &stf;
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
void CRunway::WriteCVS(U_INT No,U_INT gx,U_INT gz,char *sep,CStreamFile &stf)
{ CStreamFile* sf = &stf;
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
void CNavaid::WriteCVS(U_INT gx,U_INT gz,char *sep,CStreamFile &stf)
{ CStreamFile* sf = &stf;
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
void CILS::WriteCVS(U_INT gx,U_INT gz,char *sep,CStreamFile &stf)
{ CStreamFile* sf = &stf;
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
void CCOM::WriteCVS(U_INT gx,U_INT gz,char *sep,CStreamFile &stf)
{ CStreamFile* sf = &stf;
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
void CWPT::WriteCVS(U_INT gx,U_INT gz,char *sep,CStreamFile &stf)
{ CStreamFile* sf = &stf;
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
void CCountry::WriteCVS(char *sep,CStreamFile &stf)
{ CStreamFile* sf = &stf;
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
void CState::WriteCVS(char *sep,CStreamFile &stf)
{ CStreamFile* sf = &stf;
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
	fui	  = globals->fui;	
  //------------------------------------------------------------
  elv = HasIniKey("SQL","ExpELV");
  //-------------------------------------------------------------
  sea = HasIniKey("SQL","ExpSEA");
  //-------------------------------------------------------------
  txy = HasIniKey("SQL","ExpTXY");
  //--------------------------------------------------------------
  m3d = HasIniKey("SQL","ExpM3D");
  //--------------------------------------------------------------
  gtx = HasIniKey("SQL","ExpTEX");
  //--------------------------------------------------------------
  wob = HasIniKey("SQL","ExpOBJ");
  //--------------------------------------------------------------
  trn = HasIniKey("SQL","ExpTRN");
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
void CExport::ExportAPT(U_INT gx,U_INT gz,char *sep)
{ ClQueue   aptQ;
  CAirport *apt;
  U_INT   ntile = (gx << 16) | gz;
  ListAirport(ntile,aptQ);
  for (apt = (CAirport*)aptQ.GetFirst(); apt != 0; apt = (CAirport*)aptQ.NextInQ1(apt))
  {   apt->WriteCVS(noRec++,gx,gz,sep,sf);
  }
  return;
}
//-----------------------------------------------------------------------------------------
//  Export all airport
//-----------------------------------------------------------------------------------------
void CExport::ExportAirportsAsCVS()
{ U_INT gx = 0;
  U_INT gz = 0;
  char *fn = "Export/APT.csv";
  noRec    = 0;
  //---Open a stream file -----------------------------------
	sf.OpenWrite(fn);
  for (gx = 0; gx < 256; gx++)
    for (gz = 0; gz < 256; gz++)
       ExportAPT(gx,gz,Sep);
  //---Close the file ----------------------------------------
  sf.Close();
  return;
}
//-----------------------------------------------------------------------------------------
//  Export all Runways
//-----------------------------------------------------------------------------------------
void CExport::ExportRunwaysAsCVS()
{ char *fn = "Export/RWY.csv";
  CDatabase *db = CDatabaseManager::Instance().GetRWYDatabase();
  if (NoDatabase(db)) return;
  long end      = db->GetNumRecords();
  U_LONG offset = 0;
  CRunway *rwy  = new CRunway(ANY,RWY);
  long     No   = 0;
  //---Open a stream file -----------------------------------
	sf.OpenWrite(fn);
  //---Get file parameters ----------------------------------
  while (No != end)
  { offset   = db->RecordOffset(No);
    db->GetRawRecord (offset);
    db->DecodeRecord(offset,rwy);
    rwy->WriteCVS(No,0,0,Sep,sf);
    No++;
  }
  delete rwy;
  //---Close the file ----------------------------------------
  sf.Close();
  return;
}
//-----------------------------------------------------------------------------------------
//  Export all Navaids
//-----------------------------------------------------------------------------------------
void CExport::ExportNavaidsAsCVS()
{ U_INT gx = 0;
  U_INT gz = 0;
  char *fn = "Export/NAV.csv";
  noRec    = 0;
  //---Open a stream file -----------------------------------
	sf.OpenWrite(fn);
  for (gx = 0; gx < 256; gx++)
    for (gz = 0; gz < 256; gz++)
       ExportNAV(gx,gz,Sep);
  //---Close the file ----------------------------------------
  sf.Close();
  return;
}
//-----------------------------------------------------------------------------------------
//  Export Airports for the requested globe tile
//-----------------------------------------------------------------------------------------
void CExport::ExportNAV(U_INT gx,U_INT gz,char *sep)
{ ClQueue   navQ;
  CNavaid  *nav;
  U_INT   ntile = (gx << 16) | gz;
  ListNavaid(ntile,navQ);
  for (nav = (CNavaid*)navQ.GetFirst(); nav != 0; nav = (CNavaid*)navQ.NextInQ1(nav))
  {nav->WriteCVS(gx,gz,sep,sf);
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
  char *fn = "Export/ILS.csv";
  noRec    = 0;
  //---Open a stream file -----------------------------------
	sf.OpenWrite(fn);
  for (gx = 0; gx < 256; gx++)
    for (gz = 0; gz < 256; gz++)
       ExportILS(gx,gz,Sep);
  //---Close the file ----------------------------------------
  sf.Close();
  return;
}
//-----------------------------------------------------------------------------------------
//  Export ILS for the requested globe tile
//-----------------------------------------------------------------------------------------
void CExport::ExportILS(U_INT gx,U_INT gz,char *sep)
{ ClQueue   ilsQ;
  CILS     *ils;
  U_INT   ntile = (gx << 16) | gz;
  ListILS(ntile,ilsQ);
  for (ils = (CILS*)ilsQ.GetFirst(); ils != 0; ils = (CILS*)ilsQ.NextInQ1(ils))
  {	ils->WriteCVS(gx,gz,sep,sf);
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
  char *fn = "Export/COM.csv";
  noRec    = 0;
  //---Open a stream file -----------------------------------
	sf.OpenWrite(fn);
  for (gx = 0; gx < 256; gx++)
    for (gz = 0; gz < 256; gz++)
       ExportCOM(gx,gz,Sep);
  //---Close the file ----------------------------------------
  sf.Close();
  return;
}
//-----------------------------------------------------------------------------------------
//  Export COM for the requested globe tile
//-----------------------------------------------------------------------------------------
void CExport::ExportCOM(U_INT gx,U_INT gz,char *sep)
{ ClQueue   comQ;
  CCOM     *com;
  U_INT   ntile = (gx << 16) | gz;
  ListCOM(ntile,comQ);
  for (com = (CCOM*)comQ.GetFirst(); com != 0; com = (CCOM*)comQ.NextInQ1(com))
  {com->WriteCVS(gx,gz,sep,sf);  }
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
  char *fn = "Export/WPT.csv";
  noRec    = 0;
  //---Open a stream file -----------------------------------
	sf.OpenWrite(fn);
  for (gx = 0; gx < 256; gx++)
    for (gz = 0; gz < 256; gz++)
       ExportWPT(gx,gz,Sep);
  //---Close the file ----------------------------------------
  sf.Close();
  return;
}
//-----------------------------------------------------------------------------------------
//  Export WPT for the requested globe tile
//-----------------------------------------------------------------------------------------
void CExport::ExportWPT(U_INT gx,U_INT gz,char *sep)
{ ClQueue   wptQ;
  CWPT     *wpt;
  U_INT   ntile = (gx << 16) | gz;
  ListWPT(ntile,wptQ);
  for (wpt = (CWPT*)wptQ.GetFirst(); wpt != 0; wpt = (CWPT*)wptQ.NextInQ1(wpt))
  {wpt->WriteCVS(gx,gz,sep,sf);
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
{ char *fn = "Export/CTY.csv";
  noRec    = 0;
  //---Open a stream file -----------------------------------
	sf.OpenWrite(fn);
  ExportCTY(Sep);
  //---Close the file ----------------------------------------
  sf.Close();
  return;
}
//-----------------------------------------------------------------------------------------
//  Export all Contry
//-----------------------------------------------------------------------------------------
void CExport::ExportCTY(char *sep)
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
    cty->WriteCVS(sep,sf);
  }
  delete cty;
  return ;
}
//-----------------------------------------------------------------------------------------
//  Export all State
//-----------------------------------------------------------------------------------------
void CExport::ExportStaAsCVS()
{ char *fn = "Export/STA.csv";
  noRec    = 0;
  //---Open a stream file -----------------------------------
	sf.OpenWrite(fn);
  ExportSTA(Sep);
  //---Close the file ----------------------------------------
  sf.Close();
  return;
}
//-----------------------------------------------------------------------------------------
//  Export all STATES
//-----------------------------------------------------------------------------------------
void CExport::ExportSTA(char *sep)
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
    sta->WriteCVS(sep,sf);
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
  sqm->SEAtransaction();
  for   (gz = 0; gz < 256; gz++)
  { for (gx = 0; gx < 256; gx++)
        { sprintf(name,"COAST/V%03d%03d.GTP",gx,gz);
          if (!pexists (&globals->pfs, name)) continue;
          ExportCoastFile(name,gx,gz);
        }
  }
  sqm->SEAcommit();
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
{ U_INT   ind = (gx << 16) | gz;
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
  fui->SetNoticeFont(&globals->fonts.ftmono20);
	pif.mode= 0;								// Adding mode
	SCENE("========== START MODEL IMPORT ==========");
	//if (globals->m3dDB)		return;
	if (globals->sql[0]->UseModDB())	return;
	SCENE("No M3D database");
	globals->appState = APP_EXIT_SCREEN;
	Clear = 1;
  return;
}
//-----------------------------------------------------------------------------------------
//  Init model position and orientation (at (0,0,0) facing Y)
//-----------------------------------------------------------------------------------------
void CExport::InitModelPosition()
{ globals->sit->WorldOrigin();
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
  fui->DrawNoticeToUser(tx,200);
  if (sqm->Check3DModel(name)) return false;     // Already in DB
	if (Mod)	delete Mod;
  Mod   = new C3Dmodel(name,0);
  Mod->LoadPart("MODELS");
  if (!Mod->IsOK())                     return false;
	//--- Load polygon reductor -------------------------------
  Polys = new CPolyShop;
  Polys->SetModel(Mod);
  face = Polys->GetNbFaces();
  sprintf(tx,"%s No %05d (%05d triangles):   %s",op,mCnt,face,mName);
  fui->DrawNoticeToUser(tx,200);
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
	//--- write level 0 ---------------------------
	Export3DMlodQ(name,0);
	//--- Reduce polygons -------------------------
  if (mRed) Polys->ModelStandard(minp);     // Reduce model
  //--- Write Part in database ------------------
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
  if (!sqm->Check3DTexture(pif.ntex)) sqm->WriteM3DTexture(inf);
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
  { sqm->Write3Dmodel(name,prt,top, bot);
    delete prt;
  }
  return;
}
//-----------------------------------------------------------------------------------------
//  Step 0:  Display first message
//-----------------------------------------------------------------------------------------
void CExport::M3DMsgIntro()
{ char *msg = "GATHERING ALL SCENERY FILES";
  fui->DrawNoticeToUser(msg,100);
	stop	= 0;
	Mod		= 0;
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
  fui->DrawNoticeToUser(mtm,500);
  return;
}
//-----------------------------------------------------------------------------------------
//  Dispatch export M3D action
//-----------------------------------------------------------------------------------------
int  CExport::ExecuteMOD()
{ char fn[PATH_MAX];
  switch(State) {
		//--- Intro message ------------------
		case EXP_MSG1:
			M3DMsgIntro();
			Clear	= 0;
			return EXP_INIT;					
		//--- Collect all sceneries ----------
		case EXP_INIT:
			return EXP_FBIN;
		//--- Init for BIN files ----------
		case EXP_FBIN:
			sprintf(fn,"MODELS/*.BIN");
			mName = (char*)pfindfirst (&globals->pfs,fn);
			return EXP_NBIN;
		//--- Prepare one BIN file ------------
		case EXP_NBIN:
			if (stop)												return Pause();
			if (0 == mName)									return EXP_FSMF; 
			if (Prepare3Dmodel(mName,1))		return EXP_WBIN;
			mName = (char*)pfindnext (&globals->pfs);
			return EXP_NBIN;
		//--- Write the bin model --------------
		case EXP_WBIN:
			WriteTheModel();
			mName = (char*)pfindnext (&globals->pfs);
			return EXP_NBIN;
		//--- Init for SMF files --------------
		case EXP_FSMF:
			Clear = 0;
			sprintf(fn,"MODELS/*.SMF");
			mName = (char*)pfindfirst (&globals->pfs,fn);
			return EXP_NSMF;
		//--- Prepare one SMF file -------------
		case EXP_NSMF:
			if (stop)											return Pause();
			if (0 == mName)								return EXP_END; 
			if (Prepare3Dmodel(mName,1))	return EXP_WSMF;
			mName = (char*)pfindnext (&globals->pfs);
			return EXP_NSMF;
		//--- Write one SMF file --------------
		case EXP_WSMF:
			WriteTheModel();
			mName = (char*)pfindnext (&globals->pfs);
			return EXP_NSMF;
		//--- End of process ------------------
		case EXP_END:
			CloseSceneries();
			globals->appState = APP_EXIT;
			return EXP_OUT;
  }       //END OF SWITCH
  //------- STOP SIMU ------------------
	globals->appState = APP_EXIT_SCREEN;
  return 0;
  }
//------------------------------------------------------------------------------------
//  Keyboard intercept
//------------------------------------------------------------------------------------
void CExport::KeyW3D(U_INT key,U_INT mod)
{ char ch = key;
	if ('s'== ch)	 {State = EXP_END; return;}
	//--- process stop request -----------------------------
	stop ^= 1;
  return;
}
//------------------------------------------------------------------------------------
//  M3D stop 
//------------------------------------------------------------------------------------
int CExport::Pause()
{	Clear = 0;
	char *mrx = "PAUSE. Type any key to continue. To stop type s";
	fui->DrawNoticeToUser(mrx,10);
	fui->DrawOnlyNotices();
	return State;
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
	Mod   = new C3Dmodel(name,0);
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
{	sqm->DeleteM3DModel(name);
	LoadUpdModel(name);
	return;
}
//=========================================================================================
//  Export Taxiways
//=========================================================================================
//-----------------------------------------------------------------------------------------
//    CALL BACK FUNCTION
//-----------------------------------------------------------------------------------------
/*
void GetTaxiway(CmHead *obj)
{ CAirport *apt = (CAirport *)obj;
  globals->exm->ExportOneTMS(apt);
  return;
}
*/
//-----------------------------------------------------------------------------------------
//  Request from MENU to export taxiways
//-----------------------------------------------------------------------------------------
void CExport::ExportTaxiways()
{ if (0 == txy) return;
  //---Collect all TMS files ---------------
	count	= 0;
  Mode  = EXP_TAXI;
  State = EXP_TXY_1SKIP;
  fName = 0;
  globals->appState = APP_EXPORT;
	fui->SetNoticeFont(&globals->fonts.ftmono20);
  apo = 0;
  apt = 0;
 // globals->sqm->GetAllAirports(GetTaxiway);
	fui->DrawNoticeToUser("MOUNTING ALL FILES",1);
	SCENE("======= IMPORT ALL TAXIWAYS =============");
  return;
}
//-----------------------------------------------------------------------------------------
// Warn for this file 
//-----------------------------------------------------------------------------------------
void CExport::ErrorTXY(char *msg)
{	SCENE("File %s: %s",podN,msg);
	return;
}
//-----------------------------------------------------------------------------------------
// Warn for this file 
//-----------------------------------------------------------------------------------------
void CExport::NoticesTXY()
{	if (0 == fName)			return;
	_snprintf(edt,MAX_PATH,"PROCESSING FILE %s",fName);
	fui->DrawNoticeToUser(edt,1);
	return;
}
//-----------------------------------------------------------------------------------------
//  Format airport Key
//-----------------------------------------------------------------------------------------
void CExport::FormatAptKey(char *fn)
{	char idn[8];
	int nf = sscanf(fn,"DATA/%32[^ .)].TMS",idn);
	*Key	= 0;
	if (0 == nf)		return;
	if (strlen(idn) == 3)	strcpy(Key,"K");
	strcat(Key,idn);
	Key[4] = 0;
	return;
}
//-----------------------------------------------------------------------------------------
//  Find the first Taxiway file
//-----------------------------------------------------------------------------------------
void CExport::ProcessTXY()
{ if (0 == fName)		return;
	//--- Extract airport key ---------------------
	FormatAptKey(fName);
	strncpy(podN,fName,PATH_MAX);				// Save name
	fName	= pfindnext (&globals->pfs);	// Next one
	//--- Find associated airport -----------------
	if (0 == *Key)	return ErrorTXY("Bad TMS name");
	sqm->GetAirportByIden(Key,&apt);	
	if (0 == apt)		return ErrorTXY("No Airport");
	//-----------------------------------------------
	char *akey = apt->GetKey();
	if (sqm->FileInTXY(akey))		return;
	//--- Add new Taxiway ----------------------------
	apo = new CAptObject(apt);
  CDataTMS tms(apo);                           // TMS decoder
  tms.DecodeBinary(podN);                        // Create all queues
	//----Export all pavements ------------------------------
  ExportPaveQ(akey);
  ExportEdgeQ(akey);
  ExportCentQ(akey);
  ExportLiteQ(akey,TC_GRN_LITE);
  ExportLiteQ(akey,TC_BLU_LITE);
  //-------------------------------------------------------
	//--- Warn for next file --------------------------------
	SCENE("...Add taxiway from %s",podN);
	return;
}

//-----------------------------------------------------------------------------------------
//    Export taxiway for one airport
//-----------------------------------------------------------------------------------------
/*
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
*/
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
    if (0 != pave->GetNBVT())  sqm->WritePavement(pave,key);
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
    if (0 != pave->GetNBVT()) sqm->WritePavement(pave,key);
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
    if (0 != pave->GetNBVT()) sqm->WritePavement(pave,key);
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
    if (0 != lite->GetNbSpot()) sqm->WriteTaxiLigth(lite,key);
    delete lite;
  }
  return;
}
//-----------------------------------------------------------------------------------------
//  Execute export taxiways
//-----------------------------------------------------------------------------------------
int CExport::ExecuteTXY()
{	char *fp	= "DATA/*.TMS";
	switch (State)	{
		//--- Skip a frame to draw notice --------------
		case EXP_TXY_1SKIP:
			return EXP_TXY_MOUNT;
		//--- Mount all files --------------------------
		case EXP_TXY_MOUNT:
			globals->scn->MountAll();
			fName	= pfindfirst(pfs,fp);
		  NoticesTXY();
			return EXP_TXY_PFILE;
		//--- Extract first file -----------------------
		case EXP_TXY_PFILE:
			ProcessTXY();
			NoticesTXY();
			if (apo)	delete apo;
			if (apt)	delete apt;
			apo = 0;
			apt	= 0;
			return (fName)?(EXP_TXY_PFILE):(0);

	}
	globals->appState = APP_EXIT_SCREEN;
	SCENE("======= END IMPORT TAXIWAYS =============");
	return 0;
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
  if (*mod == '4')  {res = TX_MEDIUM; tab = "DAY128"; dim = 128;}
  if (*mod == '5')  {res = TX_HIGHTR; tab = "DAY256"; dim = 256;}
  if (*typ == 'N')  {res = TX_MEDIUM, tab = "NIT128"; dim = 128;}
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
  sqm->WriteGenTexture(inf,tab);
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
  sqm->WriteAnyTexture(inf);
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
  sqm->WriteAnyTexture(inf);
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
  sqm->WriteAnyTexture(inf);
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
{ Clear	= 1;
  if (EXP_MW3D == Mode)		State = ExecuteMOD();
  if (EXP_TRNF == Mode)		State =	ExecuteTRN();
	if (EXP_WOBJ == Mode)		State = ExecuteOBJ();
	if (EXP_TAXI == Mode)		State = ExecuteTXY();
  fui->DrawOnlyNotices();
  return Clear;
}
//=========================================================================================
//  Export Keyboard
//=========================================================================================
void CExport::Keyboard(U_INT key,U_INT mod)
{ if (EXP_MW3D == Mode) KeyW3D(key,mod);
	if (EXP_TRNF == Mode)	stop	= key;
  return;
}
//=========================================================================================
//  Export Scenery Object
//=========================================================================================
void CExport::ExportSceneryOBJ()
{	if (0 == wob)       return;
	if (!sqm->UseObjDB())	return;
	count	= 0;
  Mode  = EXP_WOBJ;
  State = EXP_OBJ_INIT;
  fName = 0;
  globals->appState = APP_EXPORT;
	fui->SetNoticeFont(&globals->fonts.ftmono20);
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
	fui->DrawNoticeToUser(msg,200);
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
	fui->DrawNoticeToUser(msg,200);
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
//  Write the file in object database
//-----------------------------------------------------------------------------------------
void  CExport::WriteOBJ()
{	if (0 == fName)		return;
	//--- check if file already in data base
	bool in		= sqm->FileInOBJ(podN);
	if (in)					 return;
	//--- Write file name in database ----------------------
	rowid = sqm->WriteFileNameInBase(podN,sqm->DBobj());
	//--- Insert OBJ in database ---------------------------
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
{	C3Dfile   sny(0,1);
  sny.Decode(fn,0);					// Decode all objects
	//----Write all objects -------------------------------
  CWobj *obj = sny.GetWOBJ();
  while (obj)
  {	sqm->WriteWOBJ(qKey,obj,rowid);
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
	int   nb = sqm->SearchWOBJ(n1);
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
	//--- Get starting QGT ---------------------------------
	char prm[128];
	GetIniString("SQL","ExpTRN",prm,127);
	int nf = sscanf(prm,"QGT ( %d - %d ) %s ",&stqx,&stqz);
	if  (2 != nf)				gtfo("Use: TRN=QGT(x-y)");
	//--- Init parameters ----------------------------------
	stop		= 0;
	count		= 0;
  Mode		= EXP_TRNF;
  State		= EXP_TRN_INIT;
  fName		= 0;
  globals->appState = APP_EXPORT;
	fui->SetNoticeFont(&globals->fonts.ftmono20);
	mCnt	= 0;
	//--- Build gamma table ---------------------------------
	float gm = float(1);
	GetIniFloat("SQL","Gamma",&gm);
	double g = double(gm);
	for (int k=0; k<256; k++)
	{	double f1 = double(k) / 255;
		double p1 = pow(f1,g);
		Lut[k] = U_INT(double(255) * pow((double (k)/255),g));
	}
	SCENE("================ START EXPORT =================");
	return;
}
//-----------------------------------------------------------------------------------------
//  Initial step
//-----------------------------------------------------------------------------------------
void CExport::InitTRNmsg()
{	char *msg = "MOUNTING SCENERY FILES ";
	fui->DrawNoticeToUser(msg,200);
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
				fui->DrawNoticeToUser("SEARCH FIRST MATCHING TRN",5);
				return EXP_TRN_FFILE;
		//--- Get first file ----------------------
		case EXP_TRN_FFILE:
			  return GetFirstTRN();
		//--- Get next file -----------------------
		case EXP_TRN_NFILE:
				return GetNextTRN();
		//--- Write a file ------------------------
		case EXP_TRN_WRITE:
				WriteTRNelevations();
				Clear = 1;
				return EXP_TRN_WTEXT;
		//--- Next texture ------------------------
		case EXP_TRN_WTEXT:
				Clear = 1;
				return WriteTRNtextures();
		//--- Compress texture --------------------
		case EXP_TRN_COMPR:
				Clear = 1;
				return CompressTRNtexture();
	}
	SCENE("========== Exported: %05d files ==========",count);
	//------- STOP SIMU ------------------
	globals->appState = APP_EXIT_SCREEN;
	SAFE_DELETE(ftrn);
	return 0;
}
//-----------------------------------------------------------------------------------------
//  Build TRN full name
//-----------------------------------------------------------------------------------------
bool CExport::BuildTRNname(char *fn)
{	if (0 == fn)		return false;
	int nf  =  sscanf(fn,"DATA/D%3d%3d/G%1d%1d.TRN",&gx,&gz,&bx,&bz);
	if (nf != 4)		return false;
	//--- Compute QGT key -------------------------------------
	qx = (gx << 1) + bx;
	qz = (gz << 1) + bz;
	qKey  = QGTKEY(qx, qz);
	//--- Filter on start QGT ----------------------------------
	if (qx < stqx)	return false;
	if (qz < stqz)	return false;
	if (globals->sqm->QGTnotInArea(qx,qz))	return false;
	//--- build pod name --------------------------------------
	char *dbn = fn + strlen("DATA/");
	char *dbp = GetSceneryPOD(pod);
	_snprintf(podN,(PATH_MAX-1),"(%s)-(%s)",dbn,dbp);
	fName	= fn;
	//---------------------------------------------------------
	char msg[1024];
	_snprintf(msg,1023,"PROCESS: %s",podN);
	fui->DrawNoticeToUser(msg,200);
	return true;
}
//-----------------------------------------------------------------------------------------
//  Find the first file
//-----------------------------------------------------------------------------------------
int CExport::GetFirstTRN()
{ char *fp	= "DATA/D??????/G??.TRN";
	char *fn	= pfindfirst(pfs,fp, &pod);
	fName			= 0;
	ftrn			= 0;
	if (0 == fn)	return 0;
	bool ok   = BuildTRNname(fn) && BuildTRNdata(fName);
	if (!ok)						return EXP_TRN_NFILE;
	return EXP_TRN_WRITE;
}

//-----------------------------------------------------------------------------------------
//  Find the next file
//-----------------------------------------------------------------------------------------
int CExport::GetNextTRN()
{	SAFE_DELETE(ftrn);
	if (stop == 'q')			return 0;
	char *fn  = pfindnext(pfs,&pod);
	if (0 == fn)					return 0;
	bool ok = BuildTRNname(fn) && BuildTRNdata(fName);
  if (ok)								return EXP_TRN_WRITE;
	//--- Check if outside of database ----------------
	return EXP_TRN_NFILE;
}
//---------------------------------------------------------------------------
//  Build a TRN data
//---------------------------------------------------------------------------
bool CExport::BuildTRNdata(char *fn)
{	if (!pexists(&globals->pfs,fn))			return false;
	ftrn = new C_TRN(fn,0,0);			// Create TRN
	bx	 = (qx << 5);		// Base X indices
	bz	 = (qz << 5);		// Base Z indices
	return true;
}
//---------------------------------------------------------------------------
//  Check if pod and trn are in database
//---------------------------------------------------------------------------
bool CExport::FileInDatabase(SQL_DB &db)
{//--- Check if pod already in data base ---------------
	char *fn	= fName + sizeof("DATA/") - 1;	// File name
	//--- Pod name ---------------------------------------
	char *slh = strrchr(podN,'/');
	char *pn	= (slh)?(slh+1):(podN);
	rowid		= sqm->FileInBase(fn,pn,db);
	return (rowid != 0);
}
//-----------------------------------------------------------------------------------------
//  Write elevations from TRN file
//-----------------------------------------------------------------------------------------
void  CExport::WriteTRNelevations()
{	if (!sqm->UseElvDB())										return;					// No ddatabase
  if (globals->sqm->QGTnotInArea(qx,qz))	return;
	//--- Check if file already in data base ---------------
	if (FileInDatabase(sqm->DBelv()))				return;
	//--- Write file name in database ----------------------
	rowid = sqm->WriteFileNameInBase(podN,sqm->DBelv());
	//--- Import all super tiles elevations ----------------
	for (sz = 0; sz != TC_SUPERT_PER_QGT; sz++)
  { for (sx = 0; sx != TC_SUPERT_PER_QGT; sx++)
		{	C_STile *asp = ftrn->GetSupTile(sx,sz);
			ImportSUPelevations(asp);
		}
	}
	//------------------------------------------------------
	count++;
	SCENE("Import elevations from %s in %s:", fName,pod);
	return;
}
//---------------------------------------------------------------------------
//  Export Super Tile description
//---------------------------------------------------------------------------
void CExport::ImportSUPelevations(C_STile *asp)
{	asp->SetKey(qKey);
  sqm->WriteElevationTRN(*asp,rowid);
	//--- Now, export all detail tiles from supertile ------
	
	TRN_HDTL *hd = asp->PopDetail();
	while (hd)
	{	sqm->WriteElevationDET(qKey,*hd,rowid);
		delete hd;
		hd	= asp->PopDetail();
	}
	
	return;
}
//-----------------------------------------------------------------------------------------
// warn user 
//-----------------------------------------------------------------------------------------
void CExport::WarnTRN()
{	char msg[128];
	_snprintf(msg,128,"COMPRESSING TEXTURE %02d FOR QGT(%3d-%3d) SUP(%d-%d)",noTX,qx,qz,sx,sz);
	fui->DrawNoticeToUser(msg,200); 
}
//-----------------------------------------------------------------------------------------
//  Laod supertile and advise user
//-----------------------------------------------------------------------------------------
bool CExport::LoadSuperTile()
{	if (eof)		return false;
	asp = ftrn->GetSupTile(sx,sz);
	asp->SetKey(qKey);
	sno	= asp->GetNo();
	//--- SW corner DET indices ----------------------------
	ax		= bx | (sx << 2);			// Base ax
	az		= bz | (sz << 2);			// Base az
	//--- Detail tile --------------------------------------
	dx		= 0;
	dz		= 0;
	//------------------------------------------------------
	texQ	= asp->GetTexDef();		// Texture Queue
	noTX	= 1;
	//--- Advise User for first texture --------------------
	WarnTRN();
	return true; 
}
//-----------------------------------------------------------------------------------------
//  Next Supertile indices
//-----------------------------------------------------------------------------------------
bool CExport::NextSupertile()
{	//--- compute next supertile indices ------------------
	sx++;
	if (sx < TC_SUPERT_PER_QGT)	return true;
	sx	= 0;
	sz++;
	if (sz < TC_SUPERT_PER_QGT) return true;
	eof	= 1;
	return false;
}
//-----------------------------------------------------------------------------------------
//  Write textures from TRN file
//-----------------------------------------------------------------------------------------
int CExport::WriteTRNtextures()
{	if (!sqm->UseDtxDB())										return EXP_TRN_NFILE;					// No ddatabase
	if (globals->sqm->QGTnotInArea(qx,qz))	return EXP_TRN_NFILE;
	//--- Check if file already in data base ---------------
	if (FileInDatabase(sqm->DBdtx()))       return EXP_TRN_NFILE;
	rowid = sqm->WriteFileNameInBase(podN,sqm->DBdtx());
	//--- Prepare reading of supertiles --------------------
	sx	= 0;
	sz	= 0;
	eof	= 0;
	//---Load first supertile-------------------------------
	LoadSuperTile();
	//------------------------------------------------------
	return EXP_TRN_COMPR;
}
///---------------------------------------------------------------------------
//  Compress texture for current SuperTile
//---------------------------------------------------------------------------
int CExport::CompressTRNtexture()
{	kx = ax | dx;		// Detail Tile X index
	kz = az | dz;		// Detail tile Z index
	if (texQ->IsSlice())	CompressTexture(texQ,kx,kz);
	noTX++;
	texQ++;
	//--- Warn for next texture ---------------------
	if (noTX <= 16)				WarnTRN();
	//--- Increase next parameters ------------------
	dx++;									// Next column
	if (dx < 4)						return EXP_TRN_COMPR;
	//--- Next row upward ---------------------------
	dz++;									// Next row indice
	dx	= 0;							// restart column indice
	if (dz < 4)						return EXP_TRN_COMPR;	
	//--- Next supertile or Next TRN file-----------
	if (!NextSupertile())	return EXP_TRN_NFILE;
	LoadSuperTile();
	return EXP_TRN_COMPR;
}
//---------------------------------------------------------------------------
//  Texture not processed
//---------------------------------------------------------------------------
void CExport::ErrorTRN01()
{	char msg[128];
	_snprintf(msg,128,"Texture not processed %s",inf.path);
	return; 
}
//---------------------------------------------------------------------------
//  Write day texture 
//---------------------------------------------------------------------------
void CExport::WriteDayTexture(CTextureDef *txd)
{	U_CHAR reso = inf.res;
	//--- Read day texture ------------------------------------------
	U_INT *wtr  = globals->txw->GetWaterRGBA(reso);
	//--- Read the RAW,ACT,OPA  texture file ------------------------
  CArtParser img(reso);
  img.SetWaterRGBA(wtr);
  inf.mADR	= img.GetRawTexture(inf,1);
	if (0 == inf.mADR)		return;
	//--- Extract the used part of texture --------------------------
	ExtractTexture(TX_HIGHTR);
	//--- Build compressed format -----------------------------------
	inf.mip	= 8;
	Compressor comp(COMP_DTX5,inf.mip);
	comp.EncodeCRN(inf);
	void *day = inf.mADR;
	if (0 == day)	return ErrorTRN01();
	//--- Write compressed format in DTX database -------------------
	sqm->WriteTRNtexture(inf,"TEX");			// Write Day texture
	//--- Tenir une statistique sur le taux de compression
	if (day) delete day;
	inf.mADR	= 0;
	return;
}
//---------------------------------------------------------------------------
//  Compress one texture
//---------------------------------------------------------------------------
void CExport::CompressTexture(CTextureDef *txd,U_INT ax,U_INT az)
{	gx		= (qx >> 1);										// Global tile x indice
	gz		= (qz >> 1);										// Global tile z indice
	//--- Check if texture is in table already ---------------------
	inf.key			= (ax << 16) | (az);
	if (sqm->TRNTextureInTable(inf.key,"TEX"))	return;
	//--- Prepare reading parameters ---------------------
	PrepareReading(txd);
	WriteDayTexture(txd);		
	//--- Check for associatd night texture --------------------------
	if (!txd->IsNight())												return;
	//--- Check if already in database -------------------------------
	if (sqm->TRNTextureInTable(inf.key,"NIT"))	return;
	//--- Now prepare for night texture ------------------------------
	Root[9]   = 'N';                      // Night indicator
  Root[10]  = 0;
  _snprintf(inf.path,TC_TEXTURE_NAME_DIM,"DATA/D%03d%03d/%s.",gx,gz,Root);
	 //--- READ the Night texture file -------------------------------
  CArtParser img(TX_MEDIUM);						// Reader instance
  GLubyte *nite		= img.LoadRaw(inf,0);	// Read file
  if (0 == nite)						return;
	inf.mADR	= nite;
  img.MergeNight(nite);									// Add alpha chanel
	ExtractTexture(TX_MEDIUM);
	//--- Build compressed format -----------------------------------
	inf.mip	= 8;
	Compressor comp(COMP_DTX5,inf.mip);
	comp.EncodeCRN(inf);
	nite = inf.mADR;
	//--- Write compressed format in DTX database -------------------
	sqm->WriteTRNtexture(inf,"NIT");			// Write night texture
	delete [] nite;
	inf.mADR	= 0;
	return;
}
//---------------------------------------------------------------------------
//  Prepare reading parameters for raw texture file
//---------------------------------------------------------------------------
void CExport::PrepareReading(CTextureDef *txd)
{	char	reso	= TX_HIGHTR;							// Resolution
  inf.Dir			= rowid;									// File reference
	inf.type		= COMP_DTX5;							// Compression type
	inf.res			= reso;										// Resolution
	inf.qty			= 232;										// Compression quality (half)
  strncpy(Root,txd->Name,8);						// Root Name
  Root[8]   = '5';											// Resolution
  Root[9]   = 0;                        // Close name
	strncpy(inf.name,Root,8);
  _snprintf(inf.path,TC_TEXTURE_NAME_DIM,"DATA/D%03d%03d/%s.",gx,gz,Root);
	return;
}
//---------------------------------------------------------------------------
//  Expend textures from TRN file
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//  Prepare reading parameters for raw texture file
//---------------------------------------------------------------------------
void CExport::ReadRawTexture(CTextureDef *txd)
{	PrepareReading(txd);
	//--- Read this texture ------------------------------------------
  char	reso	= TX_HIGHTR;							// Resolution
	U_INT *wtr  = globals->txw->GetWaterRGBA(reso);
	//--- Read the RAW,ACT,OPA  texture file ------------------------
  CArtParser img(reso);
  img.SetWaterRGBA(wtr);
  inf.mADR	= img.GetRawTexture(inf,1);
	return;
}
//---------------------------------------------------------------------------
//  Extract Hight resolution texture from TRN file 
//	Offset in source texture is 8 lines of 256 char 
//	+ marge (8) - one adjustment  for starting
//	Offset in destination is computed from tx and tz, the tile coordinates
//	We have rx = relative X Tile index into Supertile
//					rz = relative Z Tile index into SuperTile 
//	All computing done in pixels
//---------------------------------------------------------------------------
void CExport::ExtractTexture(U_CHAR res)
{	if (inf.mADR == 0)		return;
	inf.res			= res;
	U_INT usz		= UtilRES[res];								// Util side of texture
	U_INT side	= SideRES[res];
	//-------------------------------------------------------------------
	U_INT  dim  = usz * usz;
	U_INT *buf  = new U_INT[dim];
	//--- Compute source parameters -------------------------------------
	U_INT  ajx	= side - usz;		// Adjust to next line + marge
	U_INT  ofs	= (RAW_TEX_MARGE  * side) + RAW_TEX_MARGE;
	U_INT *src	= (U_INT*)(inf.mADR) + ofs - ajx;		// Source buffer
	//--- Compute Destination parameters ---------------------------------
	U_INT *dst	= buf;
	//--- Place texture --------------------------------------------------
	for (U_INT lz=0; lz < usz; lz++)
	{	//---  change to next line in source
		src	+= ajx;
		//---  Copy one line to destination --------------------------
		for (U_INT cx=0; cx < usz; cx++) 	*dst++ = *src++;
	}
	SAFE_DELETE(inf.mADR);
	inf.mADR	= (GLubyte*)buf;
	inf.dim		= dim;
	inf.wd    = usz;
	inf.ht		= usz;
	return;
}

//===================================END OF FILE ==========================================