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
CTaxiNode::CTaxiNode()
{ offset.x  = 0;
  offset.y  = 0;
  offset.z  = 0;
  direction = 0;
  nx        = 0;
  ny        = 0;
}
//------------------------------------------------------------------
//  Destroy it
//------------------------------------------------------------------
CTaxiNode::~CTaxiNode()
{}
//------------------------------------------------------------------
//  read the tags
//------------------------------------------------------------------
int CTaxiNode::Read (CStreamFile *sf, Tag tag)    // Read method
{ double  nd;
  long    ln;
  switch (tag) {
  case 'vloc':
    sf->ReadDouble(nd);
    offset.x   = nd;
    nx  = int(offset.x * 16);
    sf->ReadDouble(nd);
    offset.y   = nd;
    sf->ReadDouble(nd);
    offset.z   = nd;
    ny  = int(offset.z * 16);
    return TAG_READ;

  case 'flow':
    sf->ReadLong(ln);
    direction = (U_SHORT)ln;
    return TAG_READ;
  }
  return TAG_READ;
}

//-----------------------------------------------------------------
//  Rescale coordinates for pixels (change to left hand)
//------------------------------------------------------------------
void CTaxiNode::Scale(float scale)
{ nx  = +int(offset.x * 16 * scale);
  ny  = -int(offset.z * 16 * scale);
  return;
}
//============================================================================
//  Edge structure 
//============================================================================
CTaxiEdge::CTaxiEdge()
{ oNode   = 0;
  xNode   = 0;
  thick   = 0;
  type    = 0;
}
//------------------------------------------------------------------
//  Destroy it
//------------------------------------------------------------------
CTaxiEdge::~CTaxiEdge()
{}
//------------------------------------------------------------------
//  read the tags
//------------------------------------------------------------------
int CTaxiEdge::Read (CStreamFile *sf, Tag tag)    // Read method
{ long  nl;
  switch (tag) {
  case 'thck':
    sf->ReadFloat(thick);
    return TAG_READ;

  case 'type':
    sf->ReadLong(nl);
    type  = short(nl);
    return TAG_READ;
  }
  return TAG_READ;
}
//============================================================================
//  Class CDataBGR for Taxiway line definition
//============================================================================
CDataBGR::CDataBGR(CAptObject *apo)
{ nExt    = 0;
  sExt    = 0;
  eExt    = 0;
  wExt    = 0;
  white   = MakeRGB(255,255,255);
}
//------------------------------------------------------------------
//  Destroy the BGR items
//------------------------------------------------------------------
CDataBGR::~CDataBGR()
{ EmptyAll();
}
//------------------------------------------------------------------
//  Return Edge Number
//------------------------------------------------------------------
CTaxiEdge* CDataBGR::GetEdge(U_INT No)
{ if (No >= edgelist.size())   return 0;
  return edgelist[No];
}
//------------------------------------------------------------------
//  Return Node Number
//------------------------------------------------------------------
CTaxiNode* CDataBGR::GetNode(U_INT No)
{ if (No >= nodelist.size())   return 0;
  return nodelist[No];
}
//------------------------------------------------------------------
//  Return line number No
//------------------------------------------------------------------
bool CDataBGR::GetLine(U_INT nber,int &x0,int &y0,int &x1,int &y1)
{ CTaxiEdge *edg  = (nber >= edgelist.size())?(0):(edgelist[nber]);
  if (0 == edg)               return false;
  if (2 <=  edg->GetType())   return false;
  U_SHORT norg    = edg->GetOrigin();
  CTaxiNode *org  = (norg >= nodelist.size())?(0):(nodelist[norg]);
  if (0 == org)     return false;
  U_SHORT next    = edg->GetExtrem();
  CTaxiNode *ext  = (next >= nodelist.size())?(0):(nodelist[next]);
  if (0 == ext)     return false;
  x0  = org->GetXfeet();
  y0  = org->GetYfeet();
  x1  = ext->GetXfeet();
  y1  = ext->GetYfeet();
  return true;
}
//------------------------------------------------------------------
//  Draw segment No
//------------------------------------------------------------------
bool CDataBGR::DrawSegment(U_INT No,SSurface *sf,int xm,int ym)
{ CTaxiEdge *edg  = (No >= edgelist.size())?(0):(edgelist[No]);
  if (0 == edg)               return false;
  U_SHORT norg    = edg->GetOrigin();
  CTaxiNode *org  = (norg >= nodelist.size())?(0):(nodelist[norg]);
  if (0 == org)     return false;
  U_SHORT next    = edg->GetExtrem();
  CTaxiNode *ext  = (next >= nodelist.size())?(0):(nodelist[next]);
  if (0 == ext)     return false;
  //---------Draw runway line ------------------------
  int x0  = org->GetXfeet() + xm;
  int y0  = org->GetYfeet() + ym;
  int x1  = ext->GetXfeet() + xm;
  int y1  = ext->GetYfeet() + ym;
  DrawFastLine(sf,x0,y0,x1,y1,white);
  return true;
}
//------------------------------------------------------------------
//  Destroy the BGR items
//------------------------------------------------------------------
void  CDataBGR::EmptyAll()
{ std::vector<CTaxiNode *>::iterator nd;
  for (nd = nodelist.begin(); nd != nodelist.end();nd++) delete (*nd);
  nodelist.clear();
  std::vector<CTaxiEdge *>::iterator ed;
  for (ed = edgelist.begin(); ed != edgelist.end();ed++) delete (*ed);
  edgelist.clear();
  nExt  = sExt = eExt = wExt = 0;
  return;
}
//------------------------------------------------------------------
//  Open and read the file
//------------------------------------------------------------------
bool CDataBGR::DecodeBinary(char *fn)
{ SStream s;
  strcpy (s.filename, fn);
  strcpy (s.mode, "r");
  if (OpenStream(&s) == 0)    return false;
  CStreamFile *sf = (CStreamFile*)s.stream;
  sf->ReadFrom (this);
  CloseStream (&s);
  return true;
}

//------------------------------------------------------------------
//  read the tags
//------------------------------------------------------------------
int CDataBGR::Read (CStreamFile *sf, Tag tag)    // Read method
{ long    nb;
  double  nd;
  CTaxiNode *txn = 0;
  CTaxiEdge *edg = 0;
  switch (tag) {

  case 'orgn':
    sf->ReadDouble(nd);
    org.lat = nd;
    sf->ReadDouble(nd);
    org.lon = nd;
    sf->ReadLong(nb);
    sf->ReadLong(nb);
    sf->ReadLong(nb);
    return TAG_READ;

  case 'node':
    txn = new CTaxiNode();
    sf->ReadFrom(txn);
//TRACE("NODE %03u x= %04d y=%04d",nodelist.size(),txn->GetXfeet(),txn->GetYfeet());
    nodelist.push_back(txn);
    StoreExtension(txn);
    return TAG_READ;

  case 'edge':
    edg = new CTaxiEdge();
    sf->ReadLong(nb);
    edg->SetOrigin((short)nb);
    sf->ReadLong(nb);
    edg->SetExtrem((short)nb);
    sf->ReadFrom(edg);
//TRACE("EDGE %03u org %03u ext %03u type %u thick %f",
//      edgelist.size(),edg->GetOrigin(),edg->GetExtrem(),edg->GetType(),edg->GetThick());
    edgelist.push_back(edg);
    return TAG_READ;

}
 
  return TAG_READ;
}

//----------------------------------------------------------------------------
//  Store extension in feet
//----------------------------------------------------------------------------
void CDataBGR::StoreExtension(CTaxiNode *nod)
{ int x = nod->GetXfeet();
  int y = nod->GetYfeet();
  if (x < wExt)   wExt  = x;
  if (x > eExt)   eExt  = x;
  if (y > nExt)   nExt  = y;
  if (y < sExt)   sExt  = y;
  return;
}
//----------------------------------------------------------------------------
//  Return the largest extension
//----------------------------------------------------------------------------
U_INT CDataBGR::GetExtension()
{ U_INT dx  = eExt - wExt;
  U_INT dy  = nExt - sExt;
  return (dx > dy)?(dx):(dy);
}
//----------------------------------------------------------------------------
//  Rescale all nodes coordinates
//----------------------------------------------------------------------------
void  CDataBGR::Rescale(float sc)
{ scale   = sc;
  std::vector<CTaxiNode*>::iterator in;
  for (in = nodelist.begin();in != nodelist.end();in++) (*in)->Scale(sc);
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
{ SStream s;
  strcpy (s.filename, fname);
  strcpy (s.mode, "r");
  if (OpenStream(&s) == 0)          return false;
  CStreamFile *sf = (CStreamFile*)s.stream;
  sf->ReadFrom (this);
  CloseStream (&s);
  return true;
}
//------------------------------------------------------------------
//  Read All tags
//  All coordinates are computed in arcsec relative to airport origin
//------------------------------------------------------------------
int CDataTMS::Read (CStreamFile *sf, Tag tag)    
{ double nd;
  double cp = apo->GetCPF();                     // Compensation factor
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
          llc.lon = TC_ARCS_FROM_FEET(nd * cp);
          sf->ReadDouble(nd);
          sf->ReadDouble(nd);
          llc.lat = TC_ARCS_FROM_FEET(nd);
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
{ double cp  = apo->GetCPF();
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
    double xv  = TC_ARCS_FROM_FEET(xf) + x0;			// Longitude
    dst->VT_X  = xv;                    
    sf->ReadLong(ns);
    dst->VT_Z  = 0;																//double(ns); 
    sf->ReadLong(ns);
		double yf = double (ns >> 8);									// Divide by 256
    double yv  = TC_ARCS_FROM_FEET(yf) + y0;			// Latitude
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
  globals->tcm->SetGroundAt(lnd);
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
  globals->tcm->SetGroundAt(lnd);
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
		 N1	= N2;													// Forward to next triangle
  }
  dtq.PutEnd(pave);
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
  double cp = apo->GetCPF();                                  // Compensation factor
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
  vx   = TC_ARCS_FROM_FEET(vx * cp) + x0;      
  vy   = TC_ARCS_FROM_FEET(vy)      + y0;      
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
  vx   = TC_ARCS_FROM_FEET(vx * cp) + x0;      
  vy   = TC_ARCS_FROM_FEET(vy)      + y0;      
  spot[inx].wx = vx;                                           // Light X
  spot[inx].wy = vy;                                           // Light Y
  SpotElevation(inx);
  spot[inx].wz -= 0.005f;                                      // Light Z
  if (t2 == 0) tp0[nb0++] = inx;
  if (t2 == 1) tp1[nb1++] = inx;
  inx++;
  return nbr;
}


//===================================END OF FILE ==========================================