/*
 * Fui.h
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
#include "../Include/Taxiway.h"
#include "../Include/WinTaxiway.h"
#include "../Include/TerrainCache.h"
#include "../Include/TerrainTexture.h"
///----------------------------------------------------------------------------
//  NOTES ON COORDINATE SYSTEM
//  All coordinates (pavement and runways) are expressed with the following rules:
//  In the files, for any vertex V(x,y,z) the coordinates x,y and z are all in feet, 
//  relatives to the AIRPORT origin O(x,y,z)
//  1) X and Y are transformed in arcsec from Airport origin.
//    For any vertex V(x,y,z) yhe new coordinates X,Y and Z are
//    X = ARCSEC_PER_FEET(x * cp);
//    Y = ARCSEC_PER_FEET(y);
//    Z = z;
//  2) Distances along X are inflated with a compensation factor (cp) that depends on the 
//    latitude. This is because when going toward the pole, the tile Y dimension
//    (in arcsec) is shrinking.  At rendering time, X and Y  dimensions are translated
//    in feet by different scaling factor to make a square tile of side Y.  In this
//    process, the X dimension is reduced to the Y dimension.  The inflate factor
//    just ensures that distances are correctly set. As all coordinate are relative 
//    to the airport origin, the x value of a vertex is the vertex distance to
//    airport origin.
//  3) At rendering time, a translation from aircraft to object is set.  Then
//     object vertices are rendered.
//============================================================================
//============================================================================
//  Node structure 
//============================================================================
TaxNODE::TaxNODE()
{ RazINP(); RazOUT();}
//------------------------------------------------------------------
// Copy Constructor
//------------------------------------------------------------------
TaxNODE::TaxNODE(Tag id,TaxNODE *N)
{	*this = *N;
	 idn  = id;
	 RazINP();
	 RazOUT();
}
//------------------------------------------------------------------
//  Constructor with position given
//------------------------------------------------------------------
TaxNODE::TaxNODE(Tag id,SPosition *P)
{	Pos.lon  = 0;
  Pos.lat  = 0;
  Pos.alt  = 0;
	Init(id,P);
}
//------------------------------------------------------------------
//  Init Node
//------------------------------------------------------------------
void TaxNODE::Init(Tag id, SPosition *P)
{	idn			= id;
	RazINP();
	RazOUT();
	sector	= 0;
	if (P)	Pos = *P;
	type	= TAXI_NODE_EXIT;
}

//------------------------------------------------------------------
//  Set Position
//------------------------------------------------------------------
void TaxNODE::SetPosition(SPosition &P)
{	Pos = P;
	GroundSpot lnd(P.lon,P.lat);
  Pos.alt = globals->tcm->GetGroundAt(lnd);
	if (NotType(TAXI_NODE_AXES))	return;
	SPosition &S = lndR->lndP;						// Landing spot
	sector = GetSectorNumber(S,Pos);
	return;
}
//------------------------------------------------------------------
//  Swap for opposite runway
//------------------------------------------------------------------
void TaxNODE::SwapRunway()
{	if (NotType(TAXI_NODE_AXES))	return;
	if (type == TAXI_NODE_TKOF )	return;
	LND_DATA *lnd = lndR->opp;
	lndR	= lnd;
	strncpy(rwy,lndR->ridn,4);			// New ident
	SPosition &S = lndR->lndP;			// New origin
	sector = GetSectorNumber(S,Pos);
	return;
}
//------------------------------------------------------------------
//  Check for shortcut candidate in output circuit
//------------------------------------------------------------------
bool TaxNODE::IsOutOK()
{	if (type == TAXI_NODE_EXIT)	return true;
	if (nout[1] >  1)						return true;
	if (ninp[1] >  1)						return true;
	return false;
}
//------------------------------------------------------------------
//  Check for shortcut candidate in input circuit
//------------------------------------------------------------------
bool TaxNODE::IsInpOK()
{	bool pk = (ninp[0] == 0) && (type == TAXI_NODE_TAXI);
	if (pk)											return true;
	if (nout[0] > 1)						return true;
	return false;
}

//------------------------------------------------------------------
//  Destroy it
//------------------------------------------------------------------
TaxNODE::~TaxNODE()
{}
//------------------------------------------------------------------
//  Check for Landing parking
//------------------------------------------------------------------
bool TaxNODE::LndParking()
{	bool ok = (0 != ninp[1]) && (0 == nout[1]) && (TAXI_NODE_TAXI == type);
	return ok;	}
//------------------------------------------------------------------
//  Check for Take-off parking
//------------------------------------------------------------------
bool TaxNODE::TkoParking()
{	bool ok = (0 == ninp[0]) && (0 != nout[0]) && (TAXI_NODE_TAXI == type);
	return ok;	}

//------------------------------------------------------------------
// Read the Node file 
//------------------------------------------------------------------
int TaxNODE::Read (SStream *sf, Tag tag)
{ int	pm;
	char str[128];
	switch (tag) {
		case 'geop':
			ReadString(str,128,sf);
			sscanf(str,"%lf , %lf , %lf",&Pos.lat,&Pos.lon,&Pos.alt);
			return TAG_READ;
		case 'rwid':
			ReadString(str,128,sf);
			strncpy(rwy,str,4);
			return TAG_READ;
		case 'type':
			ReadInt(&pm,sf);
			type = U_CHAR(pm);
			return TAG_READ;
		case 'sect':
			ReadInt(&pm,sf);
			sector = U_SHORT(pm);
			return TAG_READ;
	}
	return TAG_EXIT;
}
//------------------------------------------------------------------
//  All parameters are read
//------------------------------------------------------------------
//------------------------------------------------------------------
// Save the Node in file 
//------------------------------------------------------------------
void TaxNODE::Save(CStreamFile &sf)
{	//-----------------------------------------
	sf.WriteTag('node',"");
	sf.WriteUInt(idn);
	sf.DebObject();
	sf.WriteTag('geop',"--- position -----");
	sf.WritePosition(&Pos);
	sf.WriteTag('rwid',"--- Runway end ---");
	sf.WriteString(rwy);
	sf.WriteTag('type',"--- node type ----");
	sf.WriteUInt(type);
	sf.WriteTag('sect',"--- Ruway sector -");
	sf.WriteUInt(sector);
	sf.EndObject();
}

//============================================================================
//  Edge structure 
//============================================================================
TaxEDGE::TaxEDGE()
{ idn		= 0;
	scut	= 0;
}
//------------------------------------------------------------------
//  Build a edge
//------------------------------------------------------------------
TaxEDGE::TaxEDGE(U_SHORT n1, U_SHORT n2)
{	idn = (n1 << 16) | n2;	scut = 0;}
//------------------------------------------------------------------
//  Destroy it
//------------------------------------------------------------------
TaxEDGE::~TaxEDGE()
{}
//------------------------------------------------------------------
// Read the Edge file 
//------------------------------------------------------------------
int TaxEDGE::Read (SStream *sf, Tag tag)
{ return TAG_READ;
}
//------------------------------------------------------------------
//  Check for reference to node tr
//------------------------------------------------------------------
bool TaxEDGE::ReferTo(Tag tr)
{	Tag ta = LEFT_NODE(idn);
	if (LEFT_NODE(idn) == tr)	return true;
	if (RITE_NODE(idn) == tr) return true;
	return false;
}
//------------------------------------------------------------------
//  Check for reference to node tr without shorcut
//------------------------------------------------------------------
bool TaxEDGE::NoCandidate(Tag A)
{	if (LEFT_NODE(idn) != A)			return true;
	if (LEFT_NODE(scut)!= 0)			return true;
	return false;
}
//------------------------------------------------------------------
// Save the shortcut 
//------------------------------------------------------------------
void TaxEDGE::SetShortCut(Tag C)
{	//---- left short cut -------------------
	scut &= 0x0000FFFF;				// Save rigth part
	scut |= (C << 16);				// Save left part
}
//------------------------------------------------------------------
// Save the edge in file 
//------------------------------------------------------------------
void TaxEDGE::Save(CStreamFile &sf)
{	char edt[64];
	sf.WriteTag('edge',"--- path edge -----");
  Tag t1 = idn >> 16;					// Left Tag
	Tag t2 = idn & 0x0000FFFF;	// Right tag
	Tag t3 = scut >> 16;				// Shortcut
	_snprintf(edt,64,"%05d-%05d (%05d)",t1,t2,t3);
	sf.WriteString(edt);
	return;
}

//============================================================================
//
//  Class CDataTMS for Taxiway pavement structure
//
//============================================================================
CDataTMS::CDataTMS(CAptObject *apo)
{ this->apo = apo;
  org       = apo->GetOrigin();
  ground    = apo->GetGround();
}
//------------------------------------------------------------------
//  Open and read the file
//------------------------------------------------------------------
bool CDataTMS::DecodeBinary(char *fname)
{ SStream s(this,fname);
  return true;
}
//------------------------------------------------------------------
//  Read All tags
//  All coordinates are computed in arcsec relative to airport origin
//------------------------------------------------------------------
int CDataTMS::Read (CStreamFile *sf, Tag tag)    
{ double nd;
  double cp = apo->GetXPF();                     // Compensation factor
  switch (tag) {
    //---Polygon origin -------------------------------
      case 'orgn':
        sf->ReadDouble(dpo.lat);                               // Read Latitude
        sf->ReadDouble(dpo.lon);                               // Read Longitude
        sf->ReadDouble(dpo.alt);                               // Read altitude
        //---Compute delta to airport origin ----------
        if (dpo.lat == 0) dpo.lat = org.lat;
        if (dpo.lon == 0) dpo.lon = org.lon;
        if (dpo.alt == 0) dpo.alt = org.alt;
        dpo.lat = dpo.lat - org.lat;
        dpo.lon = LongitudeDifference(dpo.lon,org.lon);
        dpo.alt = dpo.alt - org.alt;
        return TAG_READ;

      case 'tlxz':
        sf->ReadLong(tlz);
        sf->ReadLong(tlx);
        apo->SetXGrid(tlx);
        apo->SetZGrid(tlz);
        return TAG_READ;
        
      case 'feet':
        sf->ReadFloat(scale);
        apo->SetScale(scale);
        return TAG_READ;
      
      case 'lowl':
        { sf->ReadDouble(nd);
          llc.lon = FN_ARCS_FROM_FEET(nd * cp);
          sf->ReadDouble(nd);
          sf->ReadDouble(nd);
          llc.lat = FN_ARCS_FROM_FEET(nd);
          apo->SetLLC(llc);
          return TAG_READ;
        }
      case 'tile':
       CTileTMS ttm(this);
       ttm.SetLLC(llc);
       sf->ReadFrom(&ttm);
       return TAG_READ;
    }
  return TAG_READ;
}
//------------------------------------------------------------------
//	Add to pavement collection
//------------------------------------------------------------------
void  CDataTMS::AddPavement(CPaveQ &q,U_INT n)
{	apo->AddPavement(q);	
	apo->AddPAV(3 * n);
	return;	}
//------------------------------------------------------------------
//	Add to Edge collection
//------------------------------------------------------------------
void  CDataTMS::AddEdge(CPaveQ &q,U_INT n)
{	apo->AddEdge(q);	
	apo->AddEDG(3 * n);
	return;	}
//------------------------------------------------------------------
//	Add to Center collection
//------------------------------------------------------------------
void  CDataTMS::AddCenter(CPaveQ &q,U_INT n)
{	apo->AddCenter(q);	
	apo->AddCTR(3 * n);
	return;	}


//==================================================================================
//  TMS TILE Decoder
//==================================================================================
void  CTileTMS::Abort(char *msg)
{ gtfo("TMS decoder: %s",msg);
  return;
}
//-----------------------------------------------------------------------------
//  Constructor
//-----------------------------------------------------------------------------
CTileTMS::CTileTMS(CDataTMS *tm) 
{ tms     = tm;
  apo     = tms->ApOBJ();
  PavFac  = globals->apm->GetPaveFactor();
  spot    = 0;
  tp0     = 0;
  tp1     = 0;
}
//-----------------------------------------------------------------------------
//  Read TMS tags
//-----------------------------------------------------------------------------
int CTileTMS::Read(CStreamFile *sf, Tag tag)
{ 
  switch (tag)  {
    case 'indx':
      sf->ReadLong(tx);
      sf->ReadLong(tz);
      return TAG_READ;

    case 'pave':
      Type  = PAVE_TYPE_PAVE;
      total = 0;
      ReadData(sf);
			tms->AddPavement(dtq,total);
      return TAG_READ;

    case 'edmk':
      Type  = PAVE_TYPE_EDGE;
      total = 0;
      ReadData(sf);
			tms->AddEdge(dtq,total);
      return TAG_READ;

    case 'cntr':
      Type  = PAVE_TYPE_CENT;
      total = 0;
      ReadData(sf);
      tms->AddCenter(dtq,total);
      return TAG_READ;

    case 'lite':
      ReadLight(sf);
      return TAG_READ;

  }
  return TAG_READ;
}
//-------------------------------------------------------------------------------
//  Read Pavement
//  All coordinates are transformed to relative arcseconds from airport origin
//-------------------------------------------------------------------------------
void CTileTMS::ReadData(CStreamFile *sf)
{ double cp  = apo->GetXPF();
  double x0 = tms->GetXorigin();
  double y0 = tms->GetYorigin();
  double z0 = tms->GetZorigin();
  //------------------------------------------------------
  long ns = 0;
  long nb = 0;
  sf->ReadLong(lgr);                  // Data size
  Nbre = (lgr >> 2) - 5;              // Number of longs in section
  sf->ReadLong(unk1);                 // Unknown 1
  sf->ReadLong(unk2);                 // unknown 2
  sf->ReadLong(unk3);                 // unknown 3
  sf->ReadLong(unk4);                 // unknown 4
  //------------------------------------------------------
  sf->ReadLong(nbv);                  // Number of vertices
  //------------------------------------------------------
  TC_VTAB *tab = new TC_VTAB[nbv];
  TC_VTAB *dst = tab;
  for (int k = 0; k != nbv; k++)
  { sf->ReadLong(ns);
		double xf = double (ns >> 8) * cp;						// Divide by 256
    double xv  = FN_ARCS_FROM_FEET(xf) + x0;			// Longitude
    dst->VT_X  = xv;                    
    sf->ReadLong(ns);
    dst->VT_Z  = 0;																//double(ns); 
    sf->ReadLong(ns);
		double yf = double (ns >> 8);									// Divide by 256
    double yv  = FN_ARCS_FROM_FEET(yf) + y0;			// Latitude
    dst->VT_Y  = yv;
    //------Set Texture coordinates ---------------
    dst->VT_S  = (dst->VT_X - llc.lon) * PavFac;
    dst->VT_T  = (dst->VT_Y - llc.lat) * PavFac;
    //-----Set Elevation at this point -----------
    ComputeElevation(dst);
    //------Next vertex --------------------------
    dst++;
    Nbre -= 3;
  }
  //-----Read the polygons --------------------------------
  while (Nbre != 0)
  { sf->ReadLong(ns);                     // Bypass
    if (0 == ns)  break;
    sf->ReadLong(nb);                     // Number of edges
    sf->ReadLong(ns);                     // By pass 1
    sf->ReadLong(ns);                     // By pass 2
    sf->ReadLong(ns);                     // By pass 3
    sf->ReadLong(ns);                     // By pass 4
    Nbre -= 6;
		FillTriangles(sf,nb,tab);
    Nbre -= nb;
  }
  delete [] tab;
	//---- Compacting the vertex table -----------------------
  return;
}
//-------------------------------------------------------------------------------
//    Get elevation for paved vertex
//-------------------------------------------------------------------------------
void CTileTMS::ComputeElevation(TC_VTAB *tab)
{ SPosition *por = tms->GetAptOrigin();
  double lon = tab->VT_X + por->lon;
  double lat = tab->VT_Y + por->lat;
  GroundSpot lnd(lon,lat);
  globals->tcm->GetGroundAt(lnd);
  tab->VT_Z = (lnd.alt - por->alt);
  apo->AptExtension(lnd);
  return;
}
//-------------------------------------------------------------------------------
//  Compute elevation for light spot
//-------------------------------------------------------------------------------
void CTileTMS::SpotElevation(int k)
{ SPosition *apo = tms->GetAptOrigin();
  double lon = spot[k].wx + apo->lon;
  double lat = spot[k].wy + apo->lat;
  GroundSpot lnd(lon,lat);
  globals->tcm->GetGroundAt(lnd);
  spot[k].wz = lnd.alt - apo->alt;
  return;
}
//-------------------------------------------------------------------------------
//  Fill pavement polygon as triangles
//-------------------------------------------------------------------------------
void CTileTMS::FillTriangles(CStreamFile *sf,int nb,TC_VTAB *tab)
{ int nbt		= nb - 2;									// Number of triangles
	total    += nbt;										// Total triangles
  CPaveRWY *pave = new CPaveRWY(nbt * 3);									
  pave->SetType(Type);
  TC_VTAB  *dst  = pave->GetVTAB();
  //-----Read the polygons --------------------------------
	long	N0	= 0;											// vertex 0 (FAN origin)
	long	N1  = 0;											// Vertex 1
	long	N2  = 0;											// Vertex 2
	sf->ReadLong(N0);										// FAN Origin
	sf->ReadLong(N1);	
	// First vertex
	for (int k = 0; k != nbt; k++)
  {  sf->ReadLong(N2);
		*dst++ = tab[N0];                 // Triangle vertex 1
    *dst++ = tab[N1];									// Triangle vertex 2
    *dst++ = tab[N2];									// Triangle vertex 3
		//==============================================================
		//	TRAP error for big triangles in ORLY
		//==============================================================
		 float p1 = (tab[N0].VT_Y + tab[N1].VT_Y) * (tab[N0].VT_X - tab[N1].VT_X);
		 float p2 = (tab[N1].VT_Y + tab[N2].VT_Y) * (tab[N1].VT_X - tab[N2].VT_X);
		 float p3 = (tab[N2].VT_Y + tab[N0].VT_Y) * (tab[N2].VT_X - tab[N0].VT_X);
		 float sf = (p1 + p2 + p3) * 0.5;
		 bool del		= (fabs(sf) > 400);
		 if (del)
		 {	dst -= 3;
				pave->AddCount(-3);
			  TRACE ("APO %s SF=%.4f---- x0=%.4f y0=%.4f",apo->GetAptName(),sf,tab[N0].VT_X,tab[N0].VT_Y);
		 }
		 //=======END OF TRAP ==========================================
		 N1	= N2;													// Forward to next triangle
  }
  if (pave->GetNBVT()) dtq.PutEnd(pave);
  return;
}
//-------------------------------------------------------------------------------
//  Read Light parameters
//-------------------------------------------------------------------------------
void CTileTMS::ReadLight(CStreamFile *sf)
{ long tp = 0;
  long nb = 0;
  sf->ReadLong(lgr);                  // Data size
  lgr -= sizeof(long);
  //---Common texture coordinates --------------------
  TC_VTAB ent;
  ent.VT_S = 0.5;
  ent.VT_T = 0.5;
  //---Allocate temporary tables ---------------------
  int dim = lgr / (2 * sizeof(float));                 // Upper estimation for number of lights
  spot    = new TC_WORLD[dim];
  tp0     = new U_SHORT[dim];
  tp1     = new U_SHORT[dim];
  inx     = 0;
  nb0     = 0;
  nb1     = 0;
  while (lgr > 0) lgr -= ReadLightParam(sf);
  //--Allocate  final light table for color 0 (GREEN)--------
  if (nb0)
  { CBaseLITE *lit = new CBaseLITE(LS_OMNI_LITE,nb0);
    for (U_SHORT k = 0; k != nb0; k++)
    { U_SHORT No  = tp0[k];
      ent.VT_X   = spot[No].wx;
      ent.VT_Y   = spot[No].wy;
      ent.VT_Z   = spot[No].wz;
      lit->SpotAt(k,ent);
    }
    lit->col1 = TC_GRN_LITE;
    lit->ntex = TC_WHI_TEX;
    apo->AddGreenLight(lit);
  }
  //--Allocate  final light table for color 1 -(BLUE)------
  if (nb1)
  { CBaseLITE *lit = new CBaseLITE(LS_OMNI_LITE,nb1);
    for (U_SHORT k = 0; k != nb1; k++)
    { U_SHORT No  = tp1[k];
      ent.VT_X   = spot[No].wx;
      ent.VT_Y   = spot[No].wy;
      ent.VT_Z   = spot[No].wz;
      lit->SpotAt(k,ent);
    }
    lit->col1 = TC_BLU_LITE;
    lit->ntex = TC_WHI_TEX;
    apo->AddBlueLight(lit);
  }
 
  //---------------------------------------------------------
  delete [] spot;
  delete [] tp0;
  delete [] tp1;
  return;
}
//-------------------------------------------------------------------------------
//  Read light coordinates and colors
//-------------------------------------------------------------------------------
int CTileTMS::ReadLightParam(CStreamFile *sf)
{ double gr = apo->GetGround();                               // Airport ground
  double cp = apo->GetXPF();                                  // Compensation factor
  double x0 = tms->GetXorigin();
  double y0 = tms->GetYorigin();
  int   nbr = 0;                                              // Number item read
  long  t1;                                                   // Number 
  long  t2;                                                   // Color
  //---------Coordinates ---------------------------------------
  float vx;
  float vy;
  float ht;
  //---Decode type and color -----------------------------------
  sf->ReadLong(t1);
  sf->ReadLong(t2);
  nbr += (sizeof(long) << 1);                               // 2 long read
  //---Decode first light----------------------------------------
  sf->ReadFloat(vx);
  sf->ReadFloat(vy);
  nbr += (sizeof(float) << 1);                              // 2 float read
  vx   = FN_ARCS_FROM_FEET(vx * cp) + x0;      
  vy   = FN_ARCS_FROM_FEET(vy)      + y0;      
  spot[inx].wx = vx;                                         // Light X
  spot[inx].wy = vy;                                         // Light Y
  SpotElevation(inx);
  spot[inx].wz -= 0.005f;                                    // Light Z
  if (t2 == 0) tp0[nb0++] = inx;
  if (t2 == 1) tp1[nb1++] = inx;
  inx++;
  //--Decode second light if present ----------------------------
  if (t1 != 2)              return nbr;
  sf->ReadFloat(vx);
  sf->ReadFloat(vy);
  nbr += (sizeof(float) << 1);                                // 2 float read
  sf->ReadFloat(ht);
  nbr +=  sizeof(float);
  vx   = FN_ARCS_FROM_FEET(vx * cp) + x0;      
  vy   = FN_ARCS_FROM_FEET(vy)      + y0;      
  spot[inx].wx = vx;                                           // Light X
  spot[inx].wy = vy;                                           // Light Y
  SpotElevation(inx);
  spot[inx].wz -= 0.005f;                                      // Light Z
  if (t2 == 0) tp0[nb0++] = inx;
  if (t2 == 1) tp1[nb1++] = inx;
  inx++;
  return nbr;
}
//==================================================================================
//  TaxiwayMgr manages the paths from runway to parking and is part of
//	Airport Object
//	Taxiway descriptors are loaded only for near airport where plane may land
//==================================================================================
TaxiwayMGR::TaxiwayMGR(CAptObject *apo)
{	this->apo	= apo;
	apt	= apo->GetAirport();
	txx[TKO_CIRCUIT]	= new TaxiCircuit(this,0);
	txx[LND_CIRCUIT]	= new TaxiCircuit(this,1);
	Seq				= 0;
	LoadAirport();
}
//------------------------------------------------------------------
//	Destroy it
//------------------------------------------------------------------
TaxiwayMGR::~TaxiwayMGR()
{	std::map<Tag,TaxNODE*>::iterator r0;
	for (r0 = nodQ.begin(); r0 != nodQ.end(); r0++) delete (*r0).second;
	rwyQ.clear();
	nodQ.clear();
	delete (txx[TKO_CIRCUIT]);
	delete (txx[LND_CIRCUIT]);
}
//------------------------------------------------------------------
//	Load airport taxiway
//------------------------------------------------------------------
void TaxiwayMGR::LoadAirport()
{	//ReadTheFile();

	char *key	= apt->GetKey();
	if (0 == globals->txyDB)		return;
	globals->sqm->ReadTaxiNodes(key,this);
	globals->sqm->ReadTaxiEdges(key,this);
}
//-------------------------------------------------------------------------------------------
//	Enter Node from file to be used by virtual  pilote
//-------------------------------------------------------------------------------------------
void TaxiwayMGR::EnterNode(TaxNODE *N)
{	U_INT idn	 = N->idn;
	nodQ[idn]  = N;
	if (N->HasType(TAXI_NODE_AXES))	N->lndR = apt->FindRunwayLND(N->rwy);
	if (idn >= Seq)	Seq = idn + 1;
	if (N->HasType(TAXI_NODE_AXES))	rwyQ.push_back(N);
	return;
}
//-------------------------------------------------------------------------------------------
//	Get a node by identifier
//-------------------------------------------------------------------------------------------
TaxNODE *TaxiwayMGR::GetNode(Tag T)
{	std::map<Tag,TaxNODE*>::iterator rp = nodQ.find(T);
	return (rp == nodQ.end())?(0):((*rp).second);
}
//-------------------------------------------------------------------------------------------
//	Search node for runway rwid, that start in a sector greater than No
//-------------------------------------------------------------------------------------------
void	TaxiwayMGR::SetExitPath(LND_DATA *rwd,TaxiRoute *txr)
{	char dir = LND_CIRCUIT;
	txr->Clear();
	N0	= N1 = 0;
	char *rid = rwd->ridn;
	for (U_INT k = 0; k < rwyQ.size(); k++)
	{	TaxNODE *R = rwyQ[k];
		if (R->IsNotRWY(rid))				continue;
		if (R->sector <= rwd->sect)	continue;
		//--- Found exit node from runway --------
		N0		= R;
		Tag E = txx[dir]->RandomEnd(R);
		N1		= GetNode(E);
		//--- Compute path to parking ------------
		bool ok = txx[dir]->SearchThePath(N0->idn,E);
		E0  = txx[dir]->FirstPathEdge();
		txr->SetRoute(this,dir,rwd);
		return;
	}
	return;
}
//-------------------------------------------------------------------------------------------
//	Return next node on circuit 
//-------------------------------------------------------------------------------------------
TaxNODE *TaxiwayMGR::NextCircuitNode(char d)
{	//--- Change to next node ----------------------
	if (N0 == N1)		return 0;
	if (0  == E0)		return 0;
	Tag N = RITE_NODE(E0->idn);
	N0		= GetNode(N);
	if (0 == N0)		return 0;
	E0		= txx[d]->NextPathEdge();
	return N0;
}
//-------------------------------------------------------------------------------------------
//	Search a near node leading to take-off spot
//-------------------------------------------------------------------------------------------
void TaxiwayMGR::GetTkofPath(char *rid, TaxiRoute *txr)
{	char dir = TKO_CIRCUIT;
	txr->Clear();
	for (U_INT k = 0; k < rwyQ.size(); k++)
	{	TaxNODE *R = rwyQ[k];
		if (R->IsNotRWY(rid))			continue;
		if (R->IsNotTKO())				continue;
		//--- Search nearest node --------------
		Tag D = txx[dir]->GetNearTkoSpot(R);
		if (0 == D)								return;
		//--- Compute path to take off ---------
		N0 = GetNode(D);			// Starting node
		N1 = R;								// Ending node
		//--- Compute path to runway -----------
		bool ok = txx[dir]->SearchThePath(N0->idn,R->idn);
		if (!ok)									return;
		//--- Bypass first node as we are on it --
		E0  = txx[dir]->FirstPathEdge();
		N0	= NextCircuitNode(dir);
		//--- Locate runway end ----------------
		LND_DATA *rend;
		SPosition tkp;
		SPosition *pp = &tkp;
		apt->GetTakeOffSpot(rid,&pp,&rend);
		//--- Set the route ------------------
		txr->SetRoute(this,dir,rend);
		return;
	}
	return;
}

//===========================================================================================
//	Taxiway Manager:  Read from file
//===========================================================================================
bool TaxiwayMGR::ReadTheFile()
{	if (0 == apt)					return false;
	char fn[256];
	char *id = apt->GetIdent();
	_snprintf(fn,256,"Taxiways/%s.txt",id);
	SStream(this, fn);
	return true;
}
//===========================================================================================
//	Read taxiway
//----------------------------------------------------------
int TaxiwayMGR::Read(SStream *sf,Tag tag)
{	U_INT idn;
	switch (tag)	{
			//--- Read a node ----------------
			case 'node':
				{	ReadUInt(&idn,sf);
					TaxNODE *node = new TaxNODE();
					node->idn	= idn;
					ReadFrom(node,sf);
					
					EnterNode(node);
					return TAG_READ;
				}
			//--- Read take-off circuit ----------------
			case 'tkof':
				{	ReadFrom(txx[TKO_CIRCUIT],sf);
					return TAG_READ;
				}
			//--- Read Take-off paths -------------------
			case 'tkop':
				{	ReadFrom(txx[TKO_CIRCUIT],sf);
					return TAG_READ;
				}
			//--- Read landing circuit -----------------
			case 'lndg':
				{	ReadFrom(txx[LND_CIRCUIT],sf);
					return TAG_READ;
				}
			//--- Read Landing paths--------------------
			case 'lndp':
				{	ReadFrom(txx[LND_CIRCUIT],sf);
					return TAG_READ;
				}

	}
	return TAG_EXIT;
}

//-------------------------------------------------------------------------------------------
//	Create a node of given type
//-------------------------------------------------------------------------------------------
TaxNODE* TaxiwayMGR::NewNode(char type)
{	SPosition P;
	TaxNODE *node = new TaxNODE(Seq,&P);
	nodQ[Seq]	= node;
	Seq++;
	node->SetTYP(type);
	return node;
}
//-------------------------------------------------------------------------------------------
//	Create a node by copy
//-------------------------------------------------------------------------------------------
TaxNODE* TaxiwayMGR::DupNode(TaxNODE *S,SPosition &P,char T)
{	TaxNODE *node = new TaxNODE(Seq,S);
	node->SetTYP(T);
  node->SetPosition(P);
	nodQ[Seq]	= node;
	Seq++;
	return node;
}
//-------------------------------------------------------------------------------------------
//	Delete node 
//-------------------------------------------------------------------------------------------
void TaxiwayMGR::DeleteNode(Tag A)
{ std::map<Tag,TaxNODE*>::iterator r0 = nodQ.find(A);
	if (r0 == nodQ.end())		return;
	TaxNODE *nod = (*r0).second;
	delete nod;
	nodQ.erase(r0);
	return;
}
//----------------------------------------------------------
//	Save as a file
//----------------------------------------------------------
void TaxiwayMGR::SaveItems(CStreamFile &sf)
{	std::map<Tag,TaxNODE*>::iterator r0;
	//--- Save nodes -----------------------------------
	for (r0 = nodQ.begin(); r0 != nodQ.end(); r0++)
	{	(*r0).second->Save(sf);	}
	//--- Save Edges ------------------------------------
	txx[TKO_CIRCUIT]->Save('tkof','tkop',sf);
	txx[LND_CIRCUIT]->Save('lndg','lndp',sf);
}
//===========================================================================================
//	Taxiway editor:  Save taxiway
//===========================================================================================
//	Save to file
//-------------------------------------------------------------------------------
void TaxiwayMGR::SaveToFile(TaxiTracker &T)
{	if (0 == apt)		return;
  //--- Mark used nodes -------------------------
	T.ComputeAllShortCut();
	char  fn[256];
	char  cm[256];
	char *an = apt->GetName();
	char *id = apt->GetIdent();
	_snprintf(fn,256,"Taxiways/%s.txt",id);
	_snprintf(cm,256,"---Taxiways for %s airport --- ",an);
	CStreamFile sf;
  sf.OpenWrite(fn);
  sf.WriteComment(cm);
	sf.DebObject();
	//trak.SaveToFile(sf);
	SaveItems(sf);
	sf.EndObject();
	sf.Close();
	return;
}
//-------------------------------------------------------------------------------
//	Save to database
//-------------------------------------------------------------------------------
void TaxiwayMGR::SaveToBase(TaxiTracker &T)
{	if (0 == apt)								return;
	if (0 == globals->txyDB)		return;
	T.ComputeAllShortCut();
	char *key		= apt->GetKey();
	SqlMGR *sqm = globals->sqm;
	//--- remove all data from airport ---------------------
	sqm->RemoveTaxiData(key);
	//--- Save all nodes -----------------------------------
	std::map<Tag,TaxNODE*>::iterator rn;
	for (rn = nodQ.begin(); rn != nodQ.end(); rn++)
	{	sqm->AddTaxiNode(key,(*rn).second);}
	//--- Save all edges -----------------------------------
	sqm->AddTaxiEdges(key,this);
	return;
}
//===================================END OF FILE ==========================================