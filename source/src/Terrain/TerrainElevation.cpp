/*
 * ELEVATION
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
 *                2008 Jean Sabatier
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
//========================================================================================
#include "../Include/TerrainElevation.h"
#include "../Include/TerrainCache.h"
#include "../Include/Globals.h"
//========================================================================================
//  TREF sorting.  Reorder Tref rowise (00=> SW ...up to 33= NE)
//========================================================================================
U_INT TrefTAB[] = {
  0, 4, 8, 12,
  1, 5, 9, 13,
  2, 6,10, 14,
  3, 7,11, 15,
 };
//---- TABLE DES MODULO Z to compute DT index Z ---------------------
U_INT TinzTAB[] = {
	0, 0, 0, 0,
	1, 1, 1, 1,
	2, 2, 2, 2,
	3, 3, 3, 3,
};
//---- TABLE DES MODULO X to compute DT index X ---------------------
U_INT TinxTAB[] = {
	0, 1, 2, 3,
	0, 1, 2, 3,
	0, 1, 2, 3,
	0, 1, 2, 3,
};

//========================================================================================
//  List of QTR_REGION
//========================================================================================
CQregion::CQregion()
{ First = 0;
  Last  = 0;
};
//----------------------------------------------------------------------------
//  Delete all queued items
//----------------------------------------------------------------------------
CQregion::~CQregion()
{ QTR_REGION *qr = Pop();
  while (qr)  { delete qr; qr  = Pop();  }
}
//----------------------------------------------------------------------------
//  Put new item at end of Queue
//----------------------------------------------------------------------------
void CQregion::PutLast(QTR_REGION *itm)
{ QTR_REGION *end = Last;
  itm->Next = 0;
  if (end)  end->Next = itm;
  Last      = itm;
  if (First == 0)  First = itm;
  return;
}
//---------------------------------------------------------------------------
//  Insert item by node number
//---------------------------------------------------------------------------
void CQregion::Insert(QTR_REGION *reg)
{ U_SHORT node    = reg->node;
  QTR_REGION *pr  = 0;
  QTR_REGION *nr  = First;
  while (nr)
  { if (nr->node > node)  break;
    pr  = nr;
    nr  = pr->Next;
  }
  reg->Next = nr;
  if (0 == pr)  First     = reg;
  else          pr->Next  = reg;
  if (0 == nr)  Last      = reg;
  return;
}
//---------------------------------------------------------------------------
//  Detach the first item
//---------------------------------------------------------------------------
QTR_REGION *CQregion::Pop()
{ QTR_REGION *itm = First;
  if (itm)  First = itm->Next;
  if (0 == First) Last  = 0;
  if (itm)  itm->Next   = 0;
  return itm;
}
//========================================================================================
//  Header for  for TRN_HDTL list
//========================================================================================
TRN_HDTL::TRN_HDTL()
{	org   = 0;
	Next	= 0;
	tx = tz = 0;
	st		= 0;
	elev	= 0;
}
//----------------------------------------------------------------------------
TRN_HDTL::TRN_HDTL(int n, char s)
{	org		= s;
	aRes	= n;
	aDim	= n + 1;
	int nbe = aDim * aDim;
	elev	= new int[nbe];
}
//----------------------------------------------------------------------------
//	Destructor
//----------------------------------------------------------------------------
TRN_HDTL::~TRN_HDTL()
{	if (elev)	delete [] elev;
}
//----------------------------------------------------------------------------
//	Set Tile from concatenation
//----------------------------------------------------------------------------
void TRN_HDTL::SetTile(U_INT n)
{	tx	= (n >> 16);
	tz	= (n &  0x000000FF);
	return;
}
//----------------------------------------------------------------------------
//	Set Elevation in detail tile
//----------------------------------------------------------------------------
void TRN_HDTL::SetElevation(C_QGT *qgt)
{	//--- Compute Detail Tile(X,Z) in QGT ------------
	qgt->DivideHDTL(this);
	return;
}
//----------------------------------------------------------------------------
//	Set detail tile coordinates in QGT
//----------------------------------------------------------------------------
void TRN_HDTL::SetTile(U_SHORT x,U_SHORT z)
{	U_SHORT sz = (st >> 3);			// SUP(X)
	U_SHORT sx = (st & 0x07);		// SUP(Z)
  tx  = x | (sx << 2);
	tz  = z | (sz << 2);
	return;
}
//========================================================================================
//  Header for  for TRN_HDTL list
//========================================================================================
CQ_HDTL::CQ_HDTL()
{ First = 0;
  Last  = 0;
  Nbre  = 0;
};
//----------------------------------------------------------------------------
//  Destroy any left item in Queue
//----------------------------------------------------------------------------
CQ_HDTL::~CQ_HDTL()
{ TRN_HDTL *hd = Pop();
  while (hd)  {delete hd; hd = Pop(); }
  return;
}
//----------------------------------------------------------------------------
//  Put new item at end of Queue
//----------------------------------------------------------------------------
void CQ_HDTL::PutLast(TRN_HDTL *itm)
{ TRN_HDTL *end = Last;
  itm->Next = 0;
  if (end)  end->Next = itm;
  Last      = itm;
  Nbre++;
  if (First)  return;
  First     = itm;
  return;
}
//---------------------------------------------------------------------------
//  Detach the first item
//---------------------------------------------------------------------------
TRN_HDTL *CQ_HDTL::Pop()
{ TRN_HDTL *itm = First;
  if (itm)  First = itm->Next;
  if (0 == First) Last  = 0;
  if (itm)  itm->Next   = 0;
  if (itm)  Nbre--;
  return itm;
}

//========================================================================================
//  QTR constructor 
//========================================================================================
C_QTR::C_QTR(U_INT kf,TCacheMGR *tm)
{ key     = kf;
  use     = 0;
  pod     = 0;
  tcm     = tm;
  raster  = 0;
  NbReg   = 0;
  tr    = globals->Trace.Has(TRACE_TERRAIN_OP);
	btx = btz	= 0;
  //---------LOAD AND DECODE THE QTR FILE -------------------------------
  LoadFile(kf);
  if (tcm) tcm->AddRegion(NbReg);
}
//-----------------------------------------------------------------------
//  Destructor
//  NOTE: All REGION Queues are deleted in the CQregion destructor
//-----------------------------------------------------------------------
C_QTR::~C_QTR()
{ if (raster) delete [] raster;
  if (0 == tcm) return;
  if (tr) TRACE("TCM: -- Time: %04.2f QTR object deleted (%s)",tcm->Time(),name);

}
//---------------------------------------------------------------------------
//  Decrement user count and release object when no more used
//---------------------------------------------------------------------------
bool C_QTR::NoMoreUsed()
{ use--;
  return (use == 0);
}

//-----------------------------------------------------------------------
//  ABORT FOR QTR PROBLEM
//-----------------------------------------------------------------------
void C_QTR::Abort(char *reason)
{ //TRACE("F.node %u R.node %u Type %c",cNode,rNode,tNode);     /// TEMPO
  if (pod)	TRACE("File start %u size %u pos %u",pod->offset,pod->size,pod->pos);
  WARNINGLOG("QTR File %s: %s ",name,reason);
  return;
}
//-----------------------------------------------------------------------
//  Load QTR file
//  NOTE: Some QTR has no raster data
//-----------------------------------------------------------------------
void C_QTR::LoadFile(U_INT idn)
{ _snprintf(name,64,"Data/%03X.QTR",idn);
  if (!pexists (&globals->pfs, name))                     return Abort("Not found");
  pod = popen (&globals->pfs, name);
  if (0 == pod)                                           return Abort("Cant open");
  if (1 != pread (&(head.nNodes),sizeof(U_SHORT),1,pod))  return Abort("Cant read");
  if (4 != pread (&(head.magic),sizeof(U_LONG) , 4,pod))  return Abort("Cant read");
  if (4 != pread (&(head.elev), sizeof(short) ,  4,pod))  return Abort("Cant read");
  if (tr && tcm) TRACE("TCM: -- Time: %04.2f ****Read QTR file %s ****",tcm->Time(),name);
  InitRegion(idn,head.width);
  //----Decode all nodes first -----------------------------
  while (cNode != head.nNodes) DecodeNode(pod);
  //----Decode Raster data ---------------------------------
  if (head.nBytes)
  { raster = new char[head.nBytes];                 // Allocate raster area
    if (1 != pread (raster, head.nBytes ,1,pod))  return Abort("Cant read");
  }
  pclose (pod);
  pod = 0;
  if (tr) TRACE("TCM: -- %s closed",name);
  return;
}
//----------------------------------------------------------------------
//  Init First region with base Detail Tile
//  There are 64 Detail Tiles per Globe Tile (2 power 6)
//  Thus  DTx = 64 * GTX
//        DTz = 64 * GTZ
//----------------------------------------------------------------------
void C_QTR::InitRegion(U_INT idn,U_SHORT dim)
{ U_INT qtx = idn &   TC_032MODULO;     // QTR x index
  U_INT qtz = idn >>  TC_BY32;					// QTR z index
  //---Compute base Detail Tile indices for this QTR -------
  btx = qtx << (9);                     // 512 DT per QTR 
  btz = (TC_032MODULO - qtz) << (9);    // 512 DT per QGT
  //-- Compute base GQT indices for this QTR ---------------
  U_INT qgx = btx >> TC_BY32;    // 32 DT per QGT
  U_INT qgz = btz >> TC_BY32;    // 32 DT per QGT
  //--------------------------------------------------------
  QTR_REGION *reg = NewRegion(btx,btz);
  reg->dim  = dim;
  reg->node = 0;
  reg->elev = 0;
  qREG.PutLast(reg);
  cNode = 0;
  return;
}
//---------------------------------------------------------------------
//  Create a new region
//  CX and CZ are indices to QGT-DET
//---------------------------------------------------------------------
QTR_REGION *C_QTR::NewRegion(U_SHORT cx,U_SHORT cz)
{ QTR_REGION *reg = new QTR_REGION;
  reg->cx   = cx;
  reg->cz   = cz;
  reg->elev = 0;
  NbReg++;
  return reg;
}

//---------------------------------------------------------------------
//  Decode a node
//---------------------------------------------------------------------
void C_QTR::DecodeNode(PODFILE *p)
{ if (1 != pread (&(nHD.Type),sizeof(char), 1,p))  return Abort("Node Error");
  if (1 != pread (&(nHD.cElv),sizeof(short),1,p))  return Abort("Node Error");
  switch (nHD.Type) {
    case TC_TYPE_BR:              // Branch node
      if (4 != pread (&nBR,sizeof(short),4,p))     return	Abort("Node Error");
      CutHeadRegion();
      cNode++;
      return;
    case TC_TYPE_LF:              // Leaf node with elevations
      if (4 != pread (&nBR,sizeof(short),4,p))     return	Abort("Node Error");
      StoreElevation();
      cNode++;
      return;
    case TC_TYPE_AR:
      if (4 != pread (&aRS,sizeof(short),4,p))     return Abort("Node Error");
      RasterRegion(TC_TYPE_AR);
      cNode++;
      return;
    case TC_TYPE_RR:
      if (4 != pread (&aRS,sizeof(short),4,p))      return Abort("Node Error");
      RasterRegion(TC_TYPE_RR);
      cNode++;
      return;
  }
  return;
}

//--------------------------------------------------------------------
//  Set a raster region for this node
//--------------------------------------------------------------------
void C_QTR::RasterRegion(U_SHORT type)
{ QTR_REGION *reg = qREG.Pop();
  rNode = reg->node;
  tNode = 'R';
  if (reg->node != cNode)  return Abort("QTR Node not synch");
  reg->Type = type;
  reg->elev = aRS.index;
  ClipRegion(reg);
  return;
}
//--------------------------------------------------------------------
//  Cut Head Region in four sub-regions of half size
//--------------------------------------------------------------------
void C_QTR::CutHeadRegion()
{ QTR_REGION *hr = qREG.Pop();
  rNode = hr->node;
  tNode = 'C';
  if (hr->node != cNode)  return Abort("Nodes not synch");
//  if (tr) TRACE("    POP NODE %03d for subdivision",hr->node);
  U_SHORT dim   = hr->dim >> 1;
  U_SHORT cx  = hr->cx;
  U_SHORT cz  = hr->cz;
  //------Set the NW sub-region --------------
  QTR_REGION *rg = NewRegion(cx,(cz + dim) & TC_DETMASK);
  rg->Type  = TC_TYPE_BR;
  rg->dim   = dim;
  rg->node  = nBR.data[0];
  qREG.Insert(rg);
  //------Set the NE sub-region --------------
  rg = NewRegion((cx + dim) & TC_DETMASK,(cz + dim) & TC_DETMASK);
  rg->Type  = TC_TYPE_BR;
  rg->dim   = dim;
  rg->node  = nBR.data[1];
  qREG.Insert(rg);
  //------Set the SW Child -------------------
  rg->Type  = TC_TYPE_BR;
  hr->dim   = dim;
  hr->node  = nBR.data[2];
  qREG.Insert(hr);
  //-----Set the SE child --------------------
  rg = NewRegion((cx + dim) & TC_DETMASK,cz);
  rg->Type  = TC_TYPE_BR;
  rg->dim   = dim;
  rg->node  = nBR.data[3];
  qREG.Insert(rg);
  return;
}
//---------------------------------------------------------------------
//  Subdivide head region in 4 subregions with elevation
//  Transform each node in a final region
//  NOTE Region with Elevation 0 are just deleted
//---------------------------------------------------------------------
void C_QTR::StoreElevation()
{ QTR_REGION *hr = qREG.Pop();
  rNode = hr->node;
  tNode = 'E';
  if (hr->node != cNode)  return Abort("Nodes not synch");
  U_SHORT dim   = hr->dim >> 1;
  U_SHORT cx    = hr->cx;
  U_SHORT cz    = hr->cz;
  QTR_REGION *rg = 0;
  if (dim == 0)           return Abort("Region dim = 0");
  //------Set the NW sub-region --------------
  if (nBR.data[0])
  { rg = NewRegion(cx,(cz + dim) & TC_DETMASK);
    rg->Type  = TC_TYPE_LF;
    rg->dim   = dim;
    rg->elev  = nBR.data[0];
    rg->node  = hr->node;
    ClipRegion(rg);
  }
  //------Set the NE sub-region --------------
  if (nBR.data[1])
  { rg = NewRegion((cx + dim) & TC_DETMASK,(cz + dim) & TC_DETMASK);
    rg->Type  = TC_TYPE_LF;
    rg->dim   = dim;
    rg->elev  = nBR.data[1];
    rg->node  = hr->node;
    ClipRegion(rg);
  }
  //------Set the SW sub-region --------------
  if (nBR.data[2])
  { rg = NewRegion(cx,cz);
    rg->Type  = TC_TYPE_LF;
    rg->dim   = dim;
    rg->elev  = nBR.data[2];
    rg->node  = hr->node;
    ClipRegion(rg);
  }
  //-----Set the SE child --------------------
  if (nBR.data[3])
  { rg = NewRegion((cx + dim) & TC_DETMASK,cz);
    rg->Type  = TC_TYPE_LF;
    rg->dim   = dim;
    rg->elev  = nBR.data[3];
    rg->node  = hr->node;
    ClipRegion(rg);
  }
  //---Delete original region ---------------
  delete hr;
  NbReg--;
  return;
}
//-----------------------------------------------------------------------
//  Store final region.
//  -Subdivide the REGION along X  axis accross QGT boundary
//-----------------------------------------------------------------------
void C_QTR::ClipRegion(QTR_REGION *reg)
{ U_SHORT zdim = reg->dim;
  while (reg->dim > TC_DET_PER_QGT)
  { QTR_REGION *rgn = NewRegion(reg->cx,reg->cz);
    rgn->Type = reg->Type;
    rgn->dim  = TC_DET_PER_QGT;
    rgn->elev = reg->elev;
    rgn->node = reg->node;
    ClipRegionCZ(rgn,zdim);
    reg->cx   = (reg->cx + TC_DET_PER_QGT) & TC_DETMASK;
    reg->dim  -= TC_DET_PER_QGT;
  }
  EnterMatrix(reg);
  return;
}
//----------------------------------------------------------------------
//  Clip REGION on Z axis accross QGT boundary
//----------------------------------------------------------------------
void C_QTR::ClipRegionCZ(QTR_REGION *reg,U_SHORT dim)
{ while (dim > TC_DET_PER_QGT)
  { QTR_REGION *rgn = NewRegion(reg->cx,reg->cz);
    rgn->Type = reg->Type;
    rgn->dim  = TC_DET_PER_QGT;
    rgn->elev = reg->elev;
    rgn->node = reg->node;
    EnterMatrix(rgn);
    reg->cz   = (reg->cz + TC_DET_PER_QGT) & TC_DETMASK;
    dim      -= TC_DET_PER_QGT;
  }
    
  EnterMatrix(reg);
  return;
}

//---------------------------------------------------------------------
//  Enter region in QGT matrix
//  Compute cx and cz from absolute Detail Tile coordinates
//  and base tile
//---------------------------------------------------------------------
void C_QTR::EnterMatrix(QTR_REGION *reg)
{ U_SHORT tx  = (reg->cx >> TC_BY32);      // Region QGT_x
  U_SHORT tz  = (reg->cz >> TC_BY32);      // Region QGT_z
  U_SHORT bx  = (btx >> TC_BY32);          // Base QGT_x
  U_SHORT bz  = (btz >> TC_BY32);          // Base QGT_z 
  short   dx = (tx - bx) & TC_0512MOD;
  short   dz = (tz - bz) & TC_0512MOD;
  if (dx >= 16) Abort("cx Index error");
  if (dx >= 16) Abort("cz Index error");
  qQGT[dx][dz].PutLast(reg);
  return;
}
//---------------------------------------------------------------------
//  This region has one elevation value
//---------------------------------------------------------------------
void C_QTR::RegionELinfo(QTR_REGION *reg,REGION_REC &inf)
{ float   el  = (reg->elev) * TC_FEET_PER_METER;
  inf.nbv     = 1;
  inf.type    = EL_UNIC;
  inf.val     = int(el);
  return;
}
//---------------------------------------------------------------------
//  This region has an array of relative elevations
//  Allocate an array of int and compute elevations in feet
//---------------------------------------------------------------------
void C_QTR::RegionRRinfo(QTR_REGION *reg,REGION_REC &inf)
{ int    nbr  = reg->dim * reg->dim;
  char  *deb  = raster + reg->elev;
  int   *arr  = new int[nbr];
  short bdim  = *(short*)(deb);
  int    lim  = reg->dim - 1;
  int     el  = 0;
  double  ef;
  inf.nbv     = nbr;
  inf.data    = arr;
  inf.type    = EL_MONO;
  inf.val     = nbr * sizeof(short);
  char  *src  = deb + sizeof(short);
  int   *dst;
  for (int z=lim; z >= 0; z--)                // Z index
  { dst = arr + (z * reg->dim);               // Destination
    for (int x=0; x!= reg->dim; x++)  
    { el  = (*src++ << TC_BY16) + bdim; 
      ef  = (el * TC_FEET_PER_METER);
     *dst++ = int(ef);
    }
  }
  return;
}
//---------------------------------------------------------------------
//  This region has an array of absolute elevations
//  Invert lines of array to get elevation going 
//  from SW corner to NE corner
//---------------------------------------------------------------------
void C_QTR::RegionABinfo(QTR_REGION *reg,REGION_REC &inf)
{ int    nbr      = reg->dim * reg->dim;
  int   *arr      = new int[nbr];
  int    lim      = reg->dim - 1;
  inf.nbv       = nbr;
  inf.data      = arr;
  inf.type      = EL_MONO;
  inf.val       = nbr * sizeof(short);
  char   *deb   = raster + reg->elev;
  short  *src   = (short*)deb;
  int    *dst   = 0;
  double  el    = 0;
  for (int z=lim; z >= 0; z--)              // Z index
  { dst = arr + (z * reg->dim);             // Array destination
    for (int x=0; x!= reg->dim; x++)  
    {  el     = *src++ * TC_FEET_PER_METER;
      *dst++  =  int(el);
    }
  }
  return;
}

//---------------------------------------------------------------------
//  Pop one Region information
//---------------------------------------------------------------------
int  C_QTR::PopRegionInfo(int rx,int rz,REGION_REC &inf)
{ CQregion    *rq = &qQGT[rx][rz];
  QTR_REGION *reg = rq->Pop();
  inf.sub   = 0;
  inf.data  = 0;
  inf.lgx   = 0;
  inf.lgz   = 0;
  inf.val   = 0;
  if (0 == reg) return 0;
  //-----Compute base indices ---------------------
  inf.qtx = (reg->cx >> TC_BY32);   // Base QGT X index
  inf.qtz = (reg->cz >> TC_BY32);   // Base QGT Z index
  inf.dtx = (reg->cx -  (inf.qtx << TC_BY32));    // relative DET X index
  inf.dtz = (reg->cz -  (inf.qtz << TC_BY32));    // Relative DET Z index
  if (inf.dtx >= 32) Abort("dtx Index error");
  if (inf.dtz >= 32) Abort("dtz Index error");
  inf.lgx = reg->dim;
  inf.lgz = reg->dim;
  switch (reg->Type)  {
    case TC_TYPE_LF:
      RegionELinfo(reg,inf);
      break;
    case TC_TYPE_AR:
      RegionABinfo(reg,inf);
      break;
    case TC_TYPE_RR:
      RegionRRinfo(reg,inf);
      break;
    }
  delete reg;
  return 1;
}
//---------------------------------------------------------------------
//  Compute QGT indices in Region Matrix
//  Index = QGT absolute index - Base QGT (in QTR) absolute coordinate
//  NOTE:  Called from MAIN THREAD
//---------------------------------------------------------------------
void C_QTR::GetQGTindices(int xk,int zk,U_SHORT *ex,U_SHORT *ez)
{ U_SHORT bx  = (btx >> TC_BY32);          // Base QGT_x
  U_SHORT bz  = (btz >> TC_BY32);          // Base QGT_z 
  U_SHORT dx = (xk - bx) & TC_0512MOD;
  U_SHORT dz = (zk - bz) & TC_0512MOD;
	if (dx >= 16) {Abort("cx Index error"); dx = 0;}
	if (dz >= 16) {Abort("cz Index error");	dz = 0;}
  *ex = dx;
  *ez = dz;
  return;
}
//-------------------------------------------------------------------
//  Fill the default elevations for this QGT
//  NOTE:  must return 1
//  NOTE:  Called from the MAIN THREAD
//-------------------------------------------------------------------
int C_QTR::SetQTRElevations(C_QGT *qgt)
{ U_SHORT dx;
  U_SHORT dz;
  GetQGTindices(qgt->xKey,qgt->zKey,&dx,&dz);
  CQregion *rq = &qQGT[dx][dz];
  //--------Scan the regions and set elevations accordingly
  QTR_REGION *reg = rq->GetFirst();
  this->qgt = qgt;
  while (reg)
  { switch (reg->Type)  {
    case TC_TYPE_LF:
      RegionElevation(reg);
      break;
    case TC_TYPE_AR:
      RasterElevation(reg);
      break;
    case TC_TYPE_RR:
      RelativeElevation(reg);
      break;
    }
  reg = reg->Next;
  }
	//--- Next step is coast data --------------------------------------
  qgt->SetStep(TC_QT_SEA); 
  if (tr && tcm) TRACE("TCM: -- Time: %04.2f QGT %03d-%03d Set QTR Default elevations",
          tcm->Time(),qgt->xKey,qgt->zKey);
  return 1;
}
//---------------------------------------------------------------------
//  Set region elevation
//  Note: All cx an cz are internal to the QGT thus in [0-31] range
//  NOTE:  Called from MAIN THREAD
//---------------------------------------------------------------------
void C_QTR::RegionElevation(QTR_REGION *reg)
{ U_INT   cx  = (reg->cx & TC_032MODULO);     // Detail Tile x Index
  U_INT   cz  = (reg->cz & TC_032MODULO);     // Detail Tile z index
  U_INT   tx  = 0;
  float   el  = (reg->elev) * TC_FEET_PER_METER;
  U_SHORT dim = reg->dim;                     // Dimension (in DT units)
  if (cx >= TC_DET_PER_QGT) Abort("Bad Region CX");
  if (cz >= TC_DET_PER_QGT) Abort("Bad Region CZ");
  for (int z=0; z != dim; z++)                // Z index
  { tx  = cx;                                 // Restart at cx
    for (int x=0; x!= dim; x++)  
    { qgt->DetailElevation(tx++,cz,el);
    }
    cz++;                                     // Increase to next Tile row
  }
  return;
}
//---------------------------------------------------------------------
//  Set Absolute Raster elevations 
//  The raster data is an array of 8*8 shorts
//  -Elevations are stored from NW mid point to NE then going south
//  -But the region coordinate is the SW corner.
//  NOTE: Called from MAIN THREAD
//---------------------------------------------------------------------
void C_QTR::RasterElevation(QTR_REGION *reg)
{ char   *deb  = raster + reg->elev;
  short  *data = (short*)deb;
  long    el  = 0;
  float   ef;
  U_SHORT cx  =  reg->cx & TC_032MODULO;            // X coordinate for NW Tile
  U_SHORT cz  =  reg->cz & TC_032MODULO;            // Z coordinate for NW Tile
  U_SHORT tx  =   0;                                // Detail Tile x start
  U_SHORT tz  =   0;
  for (int z=7; z >= 0; z--)                         // Z index
  { tx  = cx;                                       // Restart at cx
    tz  = cz + z;                                   // Next z row
    for (int x=0; x!= 8; x++)  
    { el = *data++;
      ef = float(el * TC_FEET_PER_METER);  
      qgt->DetailElevation(tx++,tz,ef);
    }
  }
  return;
}
//---------------------------------------------------------------------
//  Set Relative Raster elevations 
//  The raster data is an array of 8*8 chars
//  -Elevations are stored from NW mid point to NE then going south
//  NOTE: Called from MAIN THREAD
//---------------------------------------------------------------------
void C_QTR::RelativeElevation(QTR_REGION *reg)
{ char  *deb  = raster + reg->elev;
  short bdim  = *(short*)(deb);
  char  *data = deb + sizeof(short);
  long   el   = 0;
  float  ef   = 0;
  U_SHORT cx  =  reg->cx & TC_032MODULO;            // X coordinate for NW Tile
  U_SHORT cz  =  reg->cz & TC_032MODULO;            // Z coordinate for NW Tile
  U_SHORT tx  = cx;                                 // Detail Tile x start
  U_SHORT tz  = 0;
  for (int z=7; z >= 0; z--)                        // Z index
  { tx  = cx;                                       // Restart at cx
    tz  = cz + z;                                   // Next z row
    for (int x=0; x!= 8; x++)  
    { el  = (*data++ << TC_BY16) + bdim; 
      ef  = float(el * TC_FEET_PER_METER);
      qgt->DetailElevation(tx++,tz,ef);
    }
  }
  return;
}
//================================================================================
//  TRN OBJECT.  DECODE TRN PARAMETERS
//  TRN file is spread on two THREAD:  T
//        A) The FILE THREAD for reading an decoding
//        B) The MAIN THREAD by QGT requesting data.
//      there is no contention problem as the QGT is suspended during the
//      file loading
//================================================================================
C_TRN::C_TRN(C_QGT *qt,U_INT tra)
{ tr    = tra;
  qgt   = qt;
  nHDTL = 0;
  nDETS = 0;
	mode  = 0;

}
//--------------------------------------------------------------------------------
//  Destroy TRN Object
//--------------------------------------------------------------------------------
C_TRN::~C_TRN()
{
}
//--------------------------------------------------------------------------------
//  ERROR In TRN FILE
//--------------------------------------------------------------------------------
void C_TRN::Abort(char *msg)
{ gtfo("ERROR in TRN:%s",msg);
return;
}
//--------------------------------------------------------------------------------
//  READ SUPER TILE PARAMETERS for normal operation
//--------------------------------------------------------------------------------
void C_TRN::ReadSUPR(SStream *stream)
{ CTextureDef *txl = 0;
	U_INT stx;
  U_INT stz;
	ReadUInt (&stx, stream);
  ReadUInt (&stz, stream);
  if ((stx > 7) || (stz > 7)) Abort("ST indices > 7");
  U_INT No = (stz << TC_BY08) | stx;
	//--- Compute current detail tile base ------------
	cx	= ax + (stx << 2);				// Multiply n°SP by 4
	cz  = az + (stz << 2);				// idem
  //----USE a SUPERTILE DECODER to hold data --------
  C_STile *spt = &aST[stx][stz];
	spt->SetAX(cx);
	spt->SetAZ(cz);
	//--- Allocate a texture def list -----------------
	if (0 == mode)  txl = qgt->GetTexList(No);	// Real
	else						txl = spt->GetTexList();		// Export
  spt->qgt     = qgt;
  spt->SetTrace(this,tr);
  spt->SetList(txl);
  ReadFrom (spt,stream);
  return;   
}
//--------------------------------------------------------------------------------
//  READ ALL PARAMETERS from TRN FILE 
//--------------------------------------------------------------------------------
int C_TRN::Read (SStream *stream, Tag tag)
{ U_INT nb;
  switch (tag) {
  case 'half':
    // Quarter globe tile indices
    ReadUInt (&nb, stream);
    ReadUInt (&nb, stream);
    return  TAG_READ;

  case 'lowr':
    // Detail tile indices for lower-left corner
    ReadUInt (&ax, stream);
    ReadUInt (&az, stream);
    return  TAG_READ;

  case 'supr':
    // Super tile sub-object
		ReadSUPR(stream);
		return TAG_READ;
  }

  return 0;
}
//--------------------------------------------------------------------------------
//  Read finished.
//--------------------------------------------------------------------------------
void C_TRN::ReadFinished()
{ if (tr) TRACE("TCM: -- Time: %04.2f TRN has %03d <hdtl> tags",globals->tcm->Time(),nHDTL);
  return;
}
//--------------------------------------------------------------------------------
// Set Default elevations for Detail Tile from TRN Supertiles
// NOTE:  Called from MAIN THREAD. No contention problem as it is called after 
//          the TRN is decoded. While in decoding, the QGT is suspended
//          in the waiting state
//--------------------------------------------------------------------------------
void C_TRN::SetTRNdefault(C_QGT *qgt)
{ U_INT sx  = 0;              // Super Tile X index
  U_INT sz  = 0;              // Super Tile Z index
  for (sz = 0; sz != TC_SUPERT_PER_QGT; sz++)
  { for (sx = 0; sx != TC_SUPERT_PER_QGT; sx++)
    aST[sx][sz].SetSupElevation(sx,sz,qgt);
  }
  return;
}
//--------------------------------------------------------------------------------
//  Compute Super Tile elevations
//   
//  NOTE:  Called from MAIN THREAD.  No contention problem as it is called after 
//          the TRN is decoded. While in decoding, the QGT is suspended
//          in the waiting state
//--------------------------------------------------------------------------------
void  C_TRN::GetHdtlElevations(C_QGT *qgt)
{ U_INT sx = 0;                     // Super Tile X index
  U_INT sz = 0;                     // Super Tile Z index;
  for (sz = 0; sz != TC_SUPERT_PER_QGT; sz++)
  { for (sx = 0; sx != TC_SUPERT_PER_QGT; sx++)
    { U_INT dx  = sx << TC_SPTPOSITION;             // Base Detail Tile
      U_INT dz  = sz << TC_SPTPOSITION;
      aST[sx][sz].GetHdltElevations(dx,dz,qgt);
    }
  }
  if (tr) TRACE("TCM: -- Time: %04.2f QGT %03d-%03d Set %03d<hdtl> elevations",
          globals->tcm->Time(),qgt->xKey,qgt->zKey,nDETS);
  return;
}
//================================================================================
//  CLASS SUPERTILE:  Decode parameters from TRN File
//  NOTE: It is assumed that the Super Tile dimension of 4*4 detail tiles
//        is strictly enforced. No provision is made for any other dimension
//        In particular, the default elevation aray is of fixed dimension
//  NOTE: Some routines are called from the MAIN THREAD.  There is no contention 
//        problem as those routines are called from the QGT after the file was loaded
//        During file loading, the QGT is suspended in the Waiting state
//         
//================================================================================
C_STile::C_STile()
{ int end = (TC_DT_ELV_PER_ST * TC_DT_ELV_PER_ST);
  tr      = 0;
  trn     = 0;
	xtmp		= 0;
  float *dst = *elev;
  for (int k=0; k != end; k++)  *dst++ = 0;
}
//--------------------------------------------------------------------------------
//  Destroy the Super Tile
//--------------------------------------------------------------------------------
C_STile::~C_STile()
{  if (xtmp) delete [] xtmp; }
//--------------------------------------------------------------------------------
//  ERROR In TRN FILE
//--------------------------------------------------------------------------------
void C_STile::Abort(char *msg)
{ gtfo("ERROR in TRN:%s",msg);
return;
}
//--------------------------------------------------------------------------------
//  READ SuperTile PARAMETERS from TRN FILE
//  TODO: See if <tref> is associated to each of <txtl> and <ntxl> tags
//--------------------------------------------------------------------------------
int C_STile::Read (SStream *stream, Tag tag)
{ int   n1;
  int   nr;
  float ft;
  U_INT sx;
  U_INT sz;
  switch (tag) {
  case 'dimn':
    // Dimension of sub-tile array
    ReadInt (&n1, stream);
    Dim   = short(n1)* short(n1) ;
		side  = n1 + 1;
    if (16 != Dim)  Abort("<Dimn> Super Tile # 4");
    return TAG_READ;

  case 'type':
    // Tile type
    ReadInt (&n1, stream);
    Type  = short(n1);
    return TAG_READ;
  //--- Indices of this super tile in the parent; discarded
  case 'prta':
    ReadUInt (&sx, stream);
    ReadUInt (&sz, stream);
    No = (sz << TC_BY08) | sx;
    return TAG_READ;
  //----Day texture list -------------------------------
  case 'txtl':
    { CTxtDecoder tdc;
      ReadFrom(&tdc,stream);
      DayList = tdc.txd;
      return TAG_READ;
    }
  //----Night texture list ----------------------------
  case 'ntxt':
    { CTxtDecoder tdc;
      ReadFrom(&tdc,stream);
      CTextureDef *txn = tdc.txd;
      int nbr          = tdc.nbx;
      for (int k = 0; k != nbr; k++)  FlagDayTexture(txn++);
      delete [] tdc.txd;
      return TAG_READ;
    }
  //-------------------------------------------------------------
  //    Texture references 
  //  NOTES
  //  1)  Tref order the list on column basis.
  //      Thus we reorder on row basis like this
  //      For generic list:   Ignore Tref and take the list as it comes
  //                          Invert I and J for good transitions
  //      For Raw: Use TrefTAB to reorder on row.
  //-------------------------------------------------------------
  case 'tref':
    { if (0 == DayList)  Abort ("No <txtl> for <tref>");
      for (short nt = 0; nt != Dim; nt++)
        { //--- compute current Detail Tile indices ------
					U_INT Order = TrefTAB[nt];
			    cx					= ax + TinxTAB[Order];
					cz					= az + TinzTAB[Order];
//--- Debugg: Use statement to check a specific detail tile ------------
//				bool ok = (cx == TC_ABSOLUTE_DET(341,0)) && (cz == TC_ABSOLUTE_DET(317,0));
//				if (ok)
//								int a = 0;				// Put breakpoint here
//--- end debuging -----------------------------------------------------
          ReadInt(&nr,stream);
          CTextureDef *src = &DayList[nr];
          CTextureDef *txn = &qList[Order];
          txn->xFlag  = src->xFlag;
          txn->TypTX  = src->TypTX;
          strncpy(txn->Name,src->Name,TC_TEXNAMESIZE);
          _strupr(txn->Name);
          //-----Normalize GEN TEXTURE INDICES -----------------
          if (txn->IsGener())
          { char x = txn->Name[9];          //  X indice
            char z = txn->Name[8];          //  Z indice
            txn->Name[8]  = x;
            txn->Name[9]  = z;
           }
          //----Build the texture KEY ---------------------------
          char *name  = txn->Name;
          U_CHAR a0   = name[0];
          U_CHAR a1   = name[1];
          U_CHAR k0   = (a0 <= '9')?(a0 -'0'):(a0 - '7');
          U_CHAR k1   = (a1 <= '9')?(a1 -'0'):(a1 - '7');
          U_CHAR j1   = name[8] - '0';              // I index
          U_CHAR j2   = name[9] - '0';              // J index
          U_INT key   = (k0 << 12)|(k1 << 8)|(j1 << 4)|(j2);
          txn->sKey   = key;
          //----Build the Hexa name -----------------------------
          U_CHAR *org = (U_CHAR*)name;
          U_CHAR *dst = (U_CHAR*)txn->Hexa;
          for (int n = 0; n!= 4; n++)
          { U_CHAR c0 = *org++;
            U_CHAR c1 = *org++;
            U_CHAR h0 = (c0 <= '9')?(c0 -'0'):(c0 - '7');
            U_CHAR h1 = (c1 <= '9')?(c1 -'0'):(c1 - '7');
            *dst++    = (h0 << 4) | h1;
          }
        }
      delete [] DayList;
      DayList = 0;
      //----TODO Night Texture list -----------------
      return TAG_READ;
    }
  //----Default elevations ---------------------------
  case 'elev':
    { for (int z=0; z != 5; z++)
      { for (int x=0; x != 5; x++) { ReadFloat(&ft,stream); elev[z][x] = ft; }
      }
    return TAG_READ;
    }
  //----Optional elevations for a Detail Tile ---------
  case 'hdtl':
    { ChdtlDecoder dt(trn,No,tr);
      ReadFrom(&dt,stream);
      qHDTL.PutLast(dt.hd);
      return TAG_READ;
    }
  }
  return 0;
}
//--------------------------------------------------------------------------------
//  Find day texture and flag it
//--------------------------------------------------------------------------------
void C_STile::FlagDayTexture(CTextureDef *txn)
{ CTextureDef *txd = qList;
  for (int k = 0; k != TC_TEXSUPERNBR; k++)
  { if (strncmp(txn->Name,txd->Name,8) != 0)  {txd++ ; continue;}
    txd->xFlag |= TC_NITTEX;
    return;
  }
  return;
}
//--------------------------------------------------------------------------------
//  Return Texture type
//--------------------------------------------------------------------------------
U_INT C_STile::GetTextureType(U_CHAR t)
{	CTextureDef *txd = qList;
	U_INT flag = 0;
	U_INT	tokn = 0x00000001;
  for (int k = 0; k != TC_TEXSUPERNBR; k++,txd++)
	{	if (txd->HasType(t)) flag |= tokn;
		tokn = tokn << 1;
	}
	return flag;
}
//--------------------------------------------------------------------------------
//  Return Texture type
//--------------------------------------------------------------------------------
U_INT C_STile::GetTextureOrg()
{	CTextureDef *txd = qList;
	U_INT flag = 0;
	U_INT	tokn = 0x00000001;
  for (int k = 0; k != TC_TEXSUPERNBR; k++,txd++)
	{	if (txd->UserTEX()) flag |= tokn;
		tokn = tokn << 1;
	}
	return flag;
}

//--------------------------------------------------------------------------------
//  Return Night Flag
//--------------------------------------------------------------------------------
U_INT C_STile::GetNite()
{	CTextureDef *txd = qList;
	U_INT flag = 0;
	U_INT	tokn = 0x00000001;
  for (int k = 0; k != TC_TEXSUPERNBR; k++,txd++)
	{	if (txd->IsNight()) flag |= tokn;
		tokn = tokn << 1;
	}
	return flag;
}
//--------------------------------------------------------------------------------
//  Compact name into area
//--------------------------------------------------------------------------------
void C_STile::PackTexName(char *dst)
{	CTextureDef *txd = qList;
	for (int k=0; k<TC_TEXSUPERNBR; k++,txd++)
	{	strncpy(dst,txd->GetName(),TEX_NAME_DIM);
		dst	+= TEX_NAME_DIM;
	}
	*dst	 = 0;
	return;
}

//--------------------------------------------------------------------------------
//  Set default elevation in Detail Tiles
//  Default elevation is a 5*5 matrix ordered by [x][z] indices
//  For each Detail Tile, the 4 corners elevations are given
//  NOTE: Called from MAIN THREAD. 
//--------------------------------------------------------------------------------
void C_STile::SetSupElevation(U_INT sx, U_INT sz, C_QGT *qgt)
{ U_INT tx = sx << TC_SPTPOSITION;
  U_INT tz = sz << TC_SPTPOSITION;
  qgt->SetElvFromTRN(tx,tz,*elev);
  return;
}

//--------------------------------------------------------------------------------
//  Compute Detail Tile Elevation from <hdtl> statement
//  NOTE: Called from MAIN THREAD.
//--------------------------------------------------------------------------------
void C_STile::GetHdltElevations(U_INT bx,U_INT bz,C_QGT *qgt)
{ TRN_HDTL *hd = qHDTL.Pop();

  while(hd)
  {	//hd->UpdTile(bx,bz);										// DET(X,Z) in QGT
		//--- compute super tile number -------------------
		U_INT sx = bx >> TC_SPTPOSITION;
		U_INT sz = bz >> TC_SPTPOSITION;
		hd->SetSup((sz << TC_BY08) | sx);
		//--------------------------------------------------------
    qgt->DivideHDTL(hd);
    trn->IncDETS();                       // One more processed
    delete hd;
    hd  = qHDTL.Pop();
  }
  return;
}

//================================================================================
//  CTxtDecoder to decode the <txtl> tag
//  All routines are called from the auxillary THREAD
//================================================================================
CTxtDecoder::CTxtDecoder()
{ nbx = 0;
  txd = 0;
}
//--------------------------------------------------------------------------------
//  Normalize the name
//  For generic name, ignore the texture list. It will be built by the default case
//  For sliced scenery, eliminate the directory string if any and check for 
//          water texture.
//                    
//--------------------------------------------------------------------------------
void CTxtDecoder::NormeName(char *txt,CTextureDef *txd)
{ char *dot = strstr(txt,".RAW");
  strncpy(txd->Name,txt,8);
  //----- Must be a generic name -------------------------------------------
  if (0 == dot) return;                                  // This is a generic name
  //------ Look for W marker -----------------------------------------------
  char *wtr  = dot - 1;
  char *end  = (*wtr == 'W')?(wtr):(dot);
  txd->TypTX = (*wtr == 'W')?(TC_TEXWATER):(TC_TEXRAWTN);               
  //------ Eliminate directory if any ---------------------------------------
  char *sep =   strrchr(txt,'/'); 
  if (0 == sep) sep = strrchr(txt,'\\');
  char *src = (sep)?(sep+1):(txt);
  *end  = 0;                                        // Set limit
  strncpy(txd->Name,src,TC_TEXNAMESIZE);
	if ((nbx > 16) || dot) txd->SetFlag(TC_USRTEX);		// Set user texture
  return;
}
//--------------------------------------------------------------------------------
//  READ <txtl> in SuperTile PARAMETERS from TRN FILE
//--------------------------------------------------------------------------------
int CTxtDecoder::Read (SStream *stream, Tag tag)
{ char txt[512];
  if  ('txtl' != tag) return 0;
  ReadUInt (&nbx, stream);
  txd = new CTextureDef[nbx];
  for (U_SHORT nt=0; nt<nbx; nt++) 
    {   ReadString (txt, 511, stream);
        NormeName(txt,&txd[nt]);
    }
  return TAG_READ;
}
//================================================================================
//  ChdtlDecoder to decode the <hdtl> tag
//================================================================================
ChdtlDecoder::ChdtlDecoder(C_TRN *trf,short No,U_INT trace)
{ sup	= No;
	trn = trf;
  hd  = 0;
  tr  = trace;
}
//--------------------------------------------------------------------------------
//  ERROR In TRN FILE
//--------------------------------------------------------------------------------
void ChdtlDecoder::Abort(char *msg)
{ gtfo("ERROR in <hdtl>:%s",msg);
return;
}
//--------------------------------------------------------------------------------
//  Allocate a new TRN_HDTL structure
//--------------------------------------------------------------------------------
void ChdtlDecoder::GetHDTL(int res)
{ hd        = new TRN_HDTL(res,0);
	hd->SetSup(sup);
  return;
}
//--------------------------------------------------------------------------------
//  READ <hdtl> in SuperTile PARAMETERS from TRN FILE
//--------------------------------------------------------------------------------
int ChdtlDecoder::Read (SStream *stream, Tag tag)
{ int dm = 0;
  int ig;
  int cx;
  int cz;
  switch (tag)  {
  //----dimension for the Detailed elevation ----------
  case 'dimn':
    ReadInt(&dm,stream);
    GetHDTL(dm);
    return TAG_READ;
  //----Type  : ignore for now ------------------------
  case 'type':
    ReadInt(&ig,stream);
    return TAG_READ;
  //----prta: Detail Tile index in Super Tile ---------
  case 'prta':
		{	ReadInt(&cx,stream);
			ReadInt(&cz,stream);
			hd->SetTile(cx,cz);
			return TAG_READ;
		}
  //
  //----elev: Array of elevation ----------------------
  //  NOTE: Elevation in <hdtl> are stored X row first
  //
  case 'elev':
    { float  flt;
      int   *arr = hd->GetElvArray();
      int    end = hd->GetArrayDim();
      for (int i = 0; i != end; i++) {ReadFloat(&flt,stream); arr[i] = flt;}
      trn->IncHDTL();
/*      if (tr) TRACE("<hdtl> for DT %03d-%03d with %03d*%03d elevations",
              hd->tx,hd->tz,hd->aDim,hd->aDim);
*/
      return TAG_READ;
    }
  }
  return TAG_READ;
}
//===================================================================================
//  FILE C_CTEX to read and decode FLY I scenery files
//===================================================================================
C_CTEX::C_CTEX(C_QGT *qt,U_INT t)
{ qgt = qt;
  tr  = t;
  nbt = 0;
}
//-----------------------------------------------------------------------------------
//  Abort for error
//-----------------------------------------------------------------------------------
void C_CTEX::Abort(char *fn, char *em)
{ gtfo("TEX %s : %s",fn,em);
  return;
}
//-----------------------------------------------------------------------------------
//  Decode the texture file
//-----------------------------------------------------------------------------------
void C_CTEX::DecodeTEX(char *fname)
{ char tex[128];
  SStream s;                                // Stream file
  if (!OpenRStream (fname,s))  Abort(fname,"Can read");
  ReadUInt(&nbt,&s);                        // Number of textures
  for (U_INT k = 0; k != nbt; k++) 
  { ReadString (tex, 16, &s);
    NormeName(tex);
    strncpy(text[k],tex,TC_TEXNAMESIZE);
  }
  return;
}
//-----------------------------------------------------------------------------------
//  Decode the TREF file.  Assign texture names
//  REF are for detail tiles 
//  The first DT is suposed to be the SW corner of the QGT? then next row up (z+1,x)
//------------------------------------------------------------------------------------
void C_CTEX::DecodeREF(char *fname)
{ U_INT ref = 0;
  U_INT tx  = 0;
  U_INT tz  = 0;
  SStream s;                                // Stream file
  if (!OpenRStream (fname, s))  Abort(fname,"Can read");
  while (ReadUInt(&ref,&s))
  { if (ref >= nbt) Abort(fname,"Bad ref");
    Assign(text[ref],tx,tz);
    tz = (tz + 1) & TC_032MODULO;
    if (0 == tz)  tx++;
  }
  return;
}
//-----------------------------------------------------------------------------------
//  Assign texture name to the Detail Tile nt
//-----------------------------------------------------------------------------------
void C_CTEX::Assign(char *tnm,U_SHORT tx,U_SHORT tz)
{ //----Find the related Super Tile-------------------
  U_INT bz = (tz >> TC_BY04);                             // Base Z for Super Tile
  U_INT bx = (tx >> TC_BY04);                             // Base X for Super Tile
  U_INT No = (bz << TC_BY08) | bx;                        // Super Tile No
  U_INT rz = (tz & TC_004MODULO);                         // Relative Z for DT in ST
  U_INT rx = (tx & TC_004MODULO);                         // Relative X for DT in ST
  U_INT nd = (rz << TC_BY04) | rx;                        // Texture index
  CSuperTile   *sp  = qgt->GetSuperTile(No);
  CTextureDef *lst  = qgt->GetTexList(No);
  CTextureDef *txd  = &lst[nd];
  strncpy(txd->Name,tnm,TC_TEXNAMESIZE);
  txd->TypTX  = TC_TEXRAWEP;                          // Set texture type
  txd->xFlag |= TC_USRTEX;                            // Set user texture
  sp->zrSwap();                                       // Prevent swapping
  return;
}
//-----------------------------------------------------------------------------------
//  Normalize  the name
//-----------------------------------------------------------------------------------
void C_CTEX::NormeName(char *txt)
{ strupper (txt);
  char *end = strstr(txt,".RAW");
  //------ Eliminate directory if any ---------------------------------------
  char *sep =   strrchr(txt,'/'); 
  if (0 == sep) sep = strrchr(txt,'\\');
  char *src = (sep)?(sep+1):(txt);
  *end  = 0;                                        // Set limit
  return;
}
//===================================================================================
//  READ AND DECODE A COAST FILE
//  NOTE: The delete option suppress polygon that are equal to the whole detail tile
//        1 polygon and 4 vertices making a square area
//  NOTE:  All reading and decoding run under the file IO THREAD
//===================================================================================
C_SEA::C_SEA(U_INT key,TCacheMGR *cm)
{ Key   = key;
  tcm   = cm;
  head  = 0;
  Use   = 0;
  tr    = (tcm)?(tcm->GetTrace()):(0);
  Name[0] = 0;
  NbDET   = 0;
  NbDEL   = 0;
  //-----Set colors ---------------------------------------------
  color   =       0xFFF3FA8A;
  //-----Set the delete option ----------------------------------
  opDEL   = 1;
  //-----Set the QGT bases coordinates ---------------------------
  coast.vTAB  = 0;
  Init(key);
  Load();
}

//---------------------------------------------------------------------------
//  Delete the SEA object
//---------------------------------------------------------------------------
C_SEA::~C_SEA()
{ 
	if (head)       delete [] head;
  if (coast.vTAB) delete [] coast.vTAB;
  if (tr &&  tcm) TRACE("TCM: -- Time: %04.2f %s DELETE %03d Tiles",tcm->Time(),Name,lgr);
  head  = 0;
}
//---------------------------------------------------------------------------
//  Decrement user count and release object when no more used
//---------------------------------------------------------------------------
bool C_SEA::NoMoreUsed()
{ Use--;
  return (Use == 0);
}
//---------------------------------------------------------------------------
//  Decrement user count and release object when no more used
//---------------------------------------------------------------------------
bool C_SEA::StillUsed()
{ Use--;
  return (Use != 0);
}

//-----------------------------------------------------------------------
//  ABORT FOR COAST PROBLEM
//-----------------------------------------------------------------------
void C_SEA::Abort(char *reason)
{ gtfo("COAST File %s: %s ",Name,reason);
  return;
}

//---------------------------------------------------------------------------
//  Init parameters
//
//---------------------------------------------------------------------------
void C_SEA::Init(U_INT key)
{ gx  = Key >> TC_BYWORD;                               // Global X indice
  gz  = Key &  TC_WORDMODULO;                           // Global Z indice
  qgx = gx << 1;                                        // base QGT X indice
  qgz = gz << 1;                                        // Base QGT Z indice
  //---------Init the Globe Tile coordinates ------------------------
  wLon  = FN_ARCS_FROM_QGT(qgx);                           // Base longitude
  sLat  = GetQgtSouthLatitude(qgz);                        // Base latitude
  //---------Init Globe tile coordinates in ARCSEC ---------------------------------
  gbtSL = GetGbtSouthLatitude(gz);
  gbtNL = GetGbtNorthLatitude(gz);
  gbtHT = (gbtNL - gbtSL);
  dLat  = gbtHT / 64;                                   // Delta per detail tile
  dLon  = TC_ARCS_PER_DET;
  return;
}
//---------------------------------------------------------------------------
//  Return SEA data
//  ax is the absolute indice of the Detail Tile
//  az is the absolute indice of the Detail Tile
//---------------------------------------------------------------------------
char *C_SEA::GetCoast(U_INT ax,U_INT az)
{ U_LONG key = (ax << TC_BYWORD) | az;
  std::map<U_INT,char*>::iterator idt = coastMAP.find(key);
  char * pol = (idt == coastMAP.end())?(0):(*idt).second;
  return pol;
}
//---------------------------------------------------------------------------
//  Load the File
//---------------------------------------------------------------------------
void C_SEA::Load()
{ _snprintf(Name,256,"COAST/V%03d%03d.GTP",gx,gz);
  pod = popen (&globals->pfs, Name);
  if (0 == pod)                                   Abort("Cant open");
  if (1 != pread (&lgr,sizeof(U_SHORT),1,pod))    Abort("Cant read");
  //-----Read header part ---------------------------------------------------
  U_INT dmh = SEA_HEAD_SIZE * lgr;
  head  = new char[dmh];
  if (lgr != pread (head,SEA_HEAD_SIZE,lgr,pod))  Abort("Cant read");
  //-----Read Detail tile data ----------------------------------------------
  char *adh = head;
  for (U_SHORT k = 0; k != lgr; k++) {LoadDetail(adh); adh += SEA_HEAD_SIZE;}
  //---- Clear all temporary resources --------------------------------------
  delete [] head;
  head  = 0;
  if (tr && tcm) TRACE("TCM: -- Time: %04.2f %s LOAD %03d Tiles",tcm->Time(),Name,lgr);
  return;
}
//---------------------------------------------------------------------------
//  Allocate a Coast structure for a detail tile
//  NOTE:   The key is the relative Detail Tile number [0-63]in the Globe Tile 
//          The final key is the absolute Detail Tile Key X-Z
//          where X and Z are in [0-16383]
//  
//---------------------------------------------------------------------------
void C_SEA::LoadDetail(char *adh)
{ char *adk = adh;                                        // Indice pointer
  char *adf = adh + sizeof(U_SHORT);                      // Offset pointer 
  U_SHORT ind = *((U_SHORT*)adh);                         // Load index
  U_LONG  ofs = *((U_LONG*) adf);                         // Load offset
  coast.Key   = MakeTileKey(ind);                         // Tile Key
  NbVRT       = 0;                                        // Clear total vertices
  LoadVertices(ofs);                                      // Load the vertice table
  //---Built definitive coast data and enter in coast map -------------------
  char *pol   = BuildCoastLine();                         // Build definittive data
  if (0 == pol) return;                                   // Tile contour was deleted
  coastMAP[coast.Key] = pol;                              // Enter in map
	globals->NbCLN++;
  NbDET++;
  return;                           
}
//---------------------------------------------------------------------------
//  Read a Detail Tile data
//  Coast data consists of
//    -A list of Vertices SEA_VERTICES
//    -A list of Polygon  (Array of U_SHORT)
//---------------------------------------------------------------------------
bool C_SEA::LoadVertices(U_LONG offset)
{ //-----READ Vertice array ---------------------------------------------
  U_SHORT nvt;
  pseek (pod, offset, SEEK_SET);
  if (  1 != pread (&nvt,sizeof(U_SHORT),1,pod))          Abort("Cant read");
  coast.nVRT = nvt;                                        // Save number of vertices
  SEA_VERTEX *svt = new SEA_VERTEX[nvt];
  if (nvt != pread (svt,sizeof(SEA_VERTEX),nvt,pod))      Abort("Cant read vertices");
  coast.vTAB = svt;                                       // Save Vertice table
  //-----READ POLYGON ARRAY ------------------------------------------------
  U_CHAR npg;                                             // Number of polygons
  if (  1 != pread (&npg,sizeof(char),1,pod))             Abort("Cant read");
  coast.nPOL = npg;                                       // Save in coast data
  poly    = 0;                                            // First polygon
  //---Load all polygons for this Detail Tile ------------------------------
  for (U_SHORT k = 0; k != npg; k++) LoadPolygon();
  return true;
}


//-----------------------------------------------------------------------------
//  Load a polygon
//  A list of chained entries is built per polygon
//  Each entry has
//        void*:    Pointer to next entry
//        U_SHORT:  Number of vertices
//        list of indices (U_SHORT) to vertice table
//-----------------------------------------------------------------------------
void C_SEA::LoadPolygon()
{ U_SHORT nvt = 0;
  U_INT   szh = sizeof(void*) + sizeof(U_SHORT);          // Header size
  if (  1  != pread (&nvt,sizeof(U_SHORT),1,pod))         Abort("Cant read polygon size");
  int   dpt = (nvt * sizeof(U_SHORT)) + szh;              // Polygon table size
  char *tbv = new char[dpt];
  //-----Build the polygon ---------------------------------------------------
  NbVRT   += nvt;                                         // Add total vertices
  if (0 == poly)   coast.pTAB     = tbv;                  // Save first polygon
  else            *((char**)poly) = tbv;                  // Or link to previous
  poly  = tbv;                                            // Save current polygon
  *((char**)poly) = 0;                                    // Link to 0
  char *adn = tbv + sizeof(void*);                        // pointer to number of entries
  *((U_SHORT*)adn)  = nvt;                                // Store number of entries
  char *pol = tbv + szh;                                  // pointer to list of indices
  if (nvt != pread (pol,sizeof(U_SHORT),nvt,pod))         Abort("Cant read polygon");
  return;
}

//-----------------------------------------------------------------------------
//  Get the tile Key and set base pixels for the Detail Tile SW corner
//  relative to SW corner of Globe Tile
//  NOTE:   For each polygon Vertex the corresponding pixel coordinates in the 
//          Detail Tile are computed on a 2048 resolution basis.
//          Later, those coordinates are adjusted to the true texture resolution
//          at time when the texture is formed
//-----------------------------------------------------------------------------
U_LONG C_SEA::MakeTileKey(U_SHORT ind)
{ U_INT cx  = ind  & TC_064MODULO;
  U_INT cz  = ind >> TC_BY64;
  U_INT ax  = (gx <<  TC_BY64) | cx;                    // 64 X DET in Globe Tile
  U_INT az  = (gz <<  TC_BY64) | cz;                    // 64 Z DET in Globe Tiles
  //----Compute the base pixel coordinates in global Tile --------------------
  xpDET = TC_MAX_PIX_DET * cx;                          // SW pixel X composite
  zpDET = TC_MAX_PIX_DET * cz;                          // SW pixel Z composite 
  return (ax << TC_BYWORD) | az;                        // Return X-Z key
}
//------------------------------------------------------------------------------
//  Normalize the Detail Tile coast data
//  Transform in the following structure
//  -----------------------------------------------------------------
//  | DETAIL HEAD   | POLYGON LIST                                  |
//  |               |                                               |
//  |               |-----------------------------------------------|
//  |               | COAST_VERTEX             | COAST_VERTEX       |
//  |               |--------------------------|--------------------|
//  |               |head | x0,z0,S,T          |head|x0,z0|........ |
//  |--------------------------------------------------------------
//  
//  DETAIL HEAD is:
//	Tag     MEM:				Memory identifier = CLIN (coast line)
//  U_SHORT NBP:        Number of Polygons
//	U_SHORT rfu:				Not yet used
//------------------------------------------------------------------------------
char *C_SEA::BuildCoastLine()
{ //-------Finalize Header -------------------------------------------------
	U_INT hds = sizeof(COAST_HEADER);										// Coast line header
  U_INT nbp = coast.nPOL;                             // Number of polygon
  U_INT npl = NbVRT * sizeof(COAST_VERTEX);           // Number of Vertices structures
  U_INT dim = hds + npl;                              // Total needed in char
	U_INT nbw = (dim / sizeof(int)) + 1;								// Total in words
	int	 *buf = new int[nbw];														// allocate memory
  char  *pol  = (char*)buf;                                  // Pointer to Nb polygon
	//-- build header ------------------------------------------------------
	COAST_HEADER *hdr = (COAST_HEADER*)buf;
	hdr->mem	= 'NILC';																	// Memory stamp
	hdr->nbp	= nbp;																		// Number of polygons	
	hdr->rfu	= 0;
  //---------Finalize polygons --------------------------------------------
  COAST_VERTEX *dst = (COAST_VERTEX*)((char*)buf + hds);     // Polygon entry
  COAST_VERTEX *fsp = dst;                            // First polygon
  char  *pid      =  (char*)coast.pTAB;               // First polygon slot
  char  *nxt      = 0;                                // next polygon slot
  for (U_INT k = 0; k != nbp; k++) 
      { dst = Finalize(dst,pid,coast.vTAB);           // Save next polygon pointer
        nxt = *((char**)pid);                         // Next polygon slot
        delete [] pid;                                // delete polygon slot
        pid = nxt;                                    // next polygon slot
       }
  //---------Delete temporary data -----------------------------------------
  delete coast.vTAB;                                  // delete vertex table 
  coast.vTAB = 0;
  coast.pTAB = 0;                                     // Clear pointer
  //---------Delete the tile if there is only one contour polygon ----------
  if (nbp != 1)       return pol;
  if (fsp->Out != 1)  return pol;
	//--- delete inside polygon ----------------------------------------------                       
	delete [] buf;
  return 0;
}
//-------------------------------------------------------------------------------
//  Finalize polygon
//    pol:  Pointer to polygon in  formation
//    pid:  Pointer to vertice indices
//    vtb:  Pointer to vertice table
//	NOTE:		Each vertex hold the total number of vertices for facility
//					in other algorithms
//-------------------------------------------------------------------------------
COAST_VERTEX *C_SEA::Finalize(COAST_VERTEX *vtx, char *pid,SEA_VERTEX *vtb)
{ U_INT szh = sizeof(void*) + sizeof(U_SHORT);  // pid header size
  COAST_VERTEX *dsn = vtx;                      // Pointer to destination polygon 
  char *srn = pid + sizeof(void*);              // Source to entry numbers
  char *srv = pid + szh;                        // Source to vertice indices
  //-----------Coordinates in Arcsecs --------------------------------------
  float cx  = 0;
  float cz  = 0;
  //-----------Get number of vertices --------------------------------------
  U_SHORT nbv = *((U_SHORT*)srn);                 // Number of vertices
  vtx->Out  = 0;                                  // External polygon
  nCN       = 0;                                  // Corner number
  //-----------Store final vertices -----------------------------------------
  for (U_SHORT k = 0; k != nbv; k++)
      { U_SHORT ind = *((U_SHORT*)srv);         // Indice
        srv += sizeof(U_SHORT);
        dsn->Nbv  = nbv;                        // Total vertices
        dsn->Num  = k;                          // Vertex number
        cx        = vtb[ind].cx;
        cz        = vtb[ind].cz;
        //------Compute absolute feet cordinates of vertex ----
//        dsn->wx   = FN_FEET_FROM_ARCS(cx) + wLon;
//        dsn->wy   = FN_FEET_FROM_ARCS(cz) + sLat;
        //-------Compute pixel coordinates in Detail tile -----
        dsn->xPix = GetPixXcoord(cx);
        dsn->zPix = GetPixZcoord(cz);
        //-------Check for a full tile polygon ----------------
        if (nbv == 4) CheckCorner(dsn->xPix,dsn->zPix);
        dsn++;
      }
  if (4 == nCN) vtx->Out = 1;                   // Inside polygon
  return dsn;
}
//--------------------------------------------------------------------------
//  Compute the pixel X coordinate in Detail Tile
//--------------------------------------------------------------------------
U_INT C_SEA::GetPixXcoord(float cx)
{ float pix = (cx * TC_MAX_PIX_GT) / TC_ARCS_PER_GBT;
  int   dtx = int(pix - xpDET);
  //-----Clamp values inside the Detail Tile -------------------
  if (dtx < 0)  dtx = 0;
  if (dtx >= TC_MAX_PIX_DET) dtx = TC_END_PIX_DET;
  return dtx;
}
//--------------------------------------------------------------------------
//  Compute the pixel Z coordinate in Detail Tile
//--------------------------------------------------------------------------
U_INT C_SEA::GetPixZcoord(float cz)
{ float piz = (cz * TC_MAX_PIX_GT) / gbtHT;
  int   dtz = int(piz - zpDET);
  //-----Clamp values inside the Detail Tile -------------------
  if (dtz < 0)  dtz = 0;
  if (dtz >= TC_MAX_PIX_DET) dtz = TC_END_PIX_DET;
  return dtz;
}

//------------------------------------------------------------------------------
//  Check for corner
//------------------------------------------------------------------------------
void C_SEA::CheckCorner(U_INT px,U_INT pz)
{ U_INT deb = TC_MIN_PIX_DET;
  U_INT end = TC_MAX_PIX_DET - 2;
  if ((px < deb) && (pz < deb))   {nCN++; return;}
  if ((px < deb) && (pz > end))   {nCN++; return;}
  if ((px > end) && (pz < deb))   {nCN++; return;}
  if ((px > end) && (pz > end))   {nCN++; return;}
  return;
}

//================================================================================
//  Coast Destructor
//
//=================================================================================
//  Constructor
//=================================================================================
C_CDT::C_CDT(U_INT qk,TCacheMGR *tm)
{ Key   = qk;
  tcm   = tm;
  pthread_mutex_init (&Mux,  NULL);
  Use   = 1;
  item  = 0;
  dLon  = TC_ARCS_PER_DET;            
  color = 0xFFF3FA8A;
}
//--------------------------------------------------------------------------
//  Destroy this coast slot
//--------------------------------------------------------------------------
C_CDT::~C_CDT()
{ 
}
//-------------------------------------------------------------------
//  Add new item
//-------------------------------------------------------------------
void C_CDT::AddCoast(U_INT key,char *data)
{ pthread_mutex_lock(&Mux);
  coastMAP[key] = data;
  item++;
	globals->NbCLN++;
  pthread_mutex_unlock(&Mux);
  return;
}
//-------------------------------------------------------------------
//  Get Coast item
//-------------------------------------------------------------------
char *C_CDT::GetCoast(U_INT key)
{ pthread_mutex_lock   (&Mux);
  char *data = 0;
  if (item) 
  { std::map<U_INT,char*>::iterator idt = coastMAP.find(key);
    data = (idt != coastMAP.end())?((*idt).second):(0);
  }
  pthread_mutex_unlock   (&Mux);
  return data;
}
//--------------------------------------------------------------------
//  Check user count
//--------------------------------------------------------------------
bool C_CDT::NoMoreUsed()
{ pthread_mutex_lock   (&Mux);
  Use--;
  bool nmu = (Use == 0);
  pthread_mutex_unlock (&Mux);
  return nmu;
}
//--------------------------------------------------------------------
//  Increment user count
//--------------------------------------------------------------------
void C_CDT::IncUser()
{ pthread_mutex_lock   (&Mux);
  Use++;
  pthread_mutex_unlock (&Mux);
  return;
}
//=========================================================================
//  Compute vertice distance in miles inside the detail tile
//=========================================================================
void CoastLine::CoastVertexDistance(short px,short py,SVector &sw)
{ short  end = TC_END_PIX_DET;
  double vx = sw.x + ((px * dLon) / TC_MAX_PIX_DET);
  double vy = sw.y + ((py * dLat) / TC_MAX_PIX_DET);
  double ax = LongitudeDifference(vx,tcm->GetPlaneLongitude()) / 60;
  double ay = (vy - tcm->GetPlaneLatitude()) / 60;
  short  wd = tcm->GetVmapWD();
  short  ht = tcm->GetVmapHT();
  pPix      = cPix;
  cPix.x    = int(ax * tcm->GetXPPM()) + wd; 
  cPix.y    = int(ay * tcm->GetYPPM()) + ht;
  cPix.y    = surf->ySize - 1 - cPix.y;
  //---Qualify corner vertices ----------------------------
  cPix.type = 0;
  if ((px != 0) && (px != end))       return;
  if ((py != 0) && (py != end))       return;
  cPix.type = 1;                    // Vertice is a corner
  return;
}
//-------------------------------------------------------------------------
//  This is a set of vertices defining a coast
//-------------------------------------------------------------------------
COAST_VERTEX *CoastLine::CoastPolygon(COAST_VERTEX *pol,SVector &sw)
{ U_SHORT nbv       = pol->Nbv;                     // Number of vertices
  COAST_VERTEX *adv = pol;
  short  wd = (tcm->GetVmapWD() << 1);
  short  ht = (tcm->GetVmapHT() << 1);
  for (int k=0; k<nbv; k++,adv++)
  { CoastVertexDistance(adv->xPix,adv->zPix,sw);
    if (0 == adv->Num)                    continue;
    if (cPix.type && pPix.type)           continue;
    //------Check for clipping -------------------------
    if ((pPix.x <  0) && (cPix.x <  0))   continue;
    if ((pPix.x > wd) && (cPix.x > wd))   continue;
    if ((pPix.y <  0) && (cPix.y <  0))   continue;
    if ((pPix.y > ht) && (cPix.y > ht))   continue;
    //------Draw line ----------------------------------
    DrawFastLine(surf,pPix.x,pPix.y,cPix.x,cPix.y,color);
   }
  return adv;
}

//--------------------------------------------------------------------------------
//  Draw coast line
//--------------------------------------------------------------------------------
void CoastLine::DrawCoastLine(SSurface *sf)
{ surf  = sf;
  std::map<U_INT,char*>::iterator it;
  SVector sw;
  for (it = coastMAP.begin(); it != coastMAP.end(); it++)
  { U_INT  key = (*it).first;
    char  *cdt = (*it).second;
    U_INT ax   = key >> 16;
    U_INT az   = key & 0x0000FFFF;
    dLat       = GetTileSWcorner(ax,az,sw);
    COAST_HEADER *hdr = (COAST_HEADER*)cdt;
    COAST_VERTEX *pol = (COAST_VERTEX*)(cdt + sizeof(COAST_HEADER));
    U_INT nbp =   hdr->nbp;
    for (U_SHORT k = 0; k != nbp; k++)  pol = CoastPolygon(pol,sw);
  }
  return;
}
//--------------------------------------------------------------------------------
//  Coast line destructor
//--------------------------------------------------------------------------------
CoastLine::~CoastLine()
{std::map<U_INT,char*>::iterator ra; 
  for (ra = coastMAP.begin(); ra != coastMAP.end(); ra++)
  { char *data = (*ra).second;
    delete [] data;
		globals->NbCLN--;
  }
  coastMAP.clear();
}
//===========END OF THIS FILE ============================================================
