/*
 * QTR.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
 * Copyright 2007 Jean Sabatier
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
 */


#include "../Include/FlyLegacy.h"
#include "../Include/Reductor.h"
#include "../Include/TerrainTexture.h"
#include "../Include/Fui.h"
//=======================================================================================
#define VAL_MAX(X,Y)  ((double(X) > double(Y))?(double(X)):(double(Y)))
#define VAL_MIN(X,Y)  ((double(X) < double(Y))?(double(X)):(double(Y)))
//=======================================================================================
//  POLYGON REDUCTOR
//=======================================================================================
//  Vertex
//=======================================================================================
CrVertex::CrVertex(CrPart *mp,CVector v,int no)
{ state = 0;
  Pos   = v;
  id    = no;
  moPRT = mp;
}
//----------------------------------------------------------------------
//  Delete all resources
//----------------------------------------------------------------------
CrVertex::~CrVertex()
{ adjv.clear();
  //-------------------------------------------------------------
  face.clear();
  //-------------------------------------------------------------
  vmod.clear();
}
//----------------------------------------------------------------------
//  Add a face to the shared face list
//----------------------------------------------------------------------
void CrVertex::AddFace(CrTriangle *tr)
{ int key   = tr->GetKey();
  face[key] = tr;
//  TRACE("    Add Face T%04d in V%04d",key,id);
  return;
}
//----------------------------------------------------------------------
//  rebuild the shared vertex list
//  -Explore each face (not removed) and add any vertex not in the list
//----------------------------------------------------------------------
void CrVertex::BuildNearVertices()
{ //--- clear old list -----------------------------------
  adjv.clear();
  //--- Explore active face list -------------------------
  std::map<int, CrTriangle*>::iterator it;
  for (it = face.begin(); it != face.end(); it++)
  { CrTriangle *tr = (*it).second;
    if (tr->IsMarked())   continue;
    CrVertex *v0 = tr->GetVertex(0);
    if (this != v0) Store(v0,0);
    CrVertex *v1 = tr->GetVertex(1);
    if (this != v1) Store(v1,0);
    CrVertex *v2 = tr->GetVertex(2);
    if (this != v2) Store(v2,0);
  }
  return;
}
//----------------------------------------------------------------------
//  Remove a face from the shared list
//----------------------------------------------------------------------
void CrVertex::RemFace(CrTriangle *tr)
{ int key   = tr->GetKey();
  face.erase(key);
//  TRACE("    Rem Face T%04d from V%04d",key,id);
  return;
}
//----------------------------------------------------------------------
//  Trace vertex environement
//----------------------------------------------------------------------
void CrVertex::Trace(char *msg)
{ //TRACE("    V%04d contains (%s)",id, msg);
  std::map<int, CrTriangle*>::iterator is;
  for(is = face.begin();is != face.end(); is++)
  { CrTriangle *tk = (*is).second;
    int v0   = tk->GetVKey(0);
    int v1   = tk->GetVKey(1);
    int v2   = tk->GetVKey(2);
    char mk[16];
    tk->EditMark(mk);
    //TRACE("    -T%04d (%04d-%04d-%04d) mark: %s",tk->GetKey(),v0,v1,v2,mk);
  }
  std::map<int, CrVertex*>::iterator vr;
  for(vr = adjv.begin();vr != adjv.end(); vr++)
  { CrVertex   *vt = (*vr).second;
    //TRACE("    -V%04d",vt->GetKey());
  }
  //TRACE("    -----------------------------------");
  return;
}
//----------------------------------------------------------------------
//  Remove Vu occurence and add VV.
//  As this vertex is a neighbor of VV, so is VV for this one
//  Therefor we must enter this one into VV neighbor list.
//----------------------------------------------------------------------
int  CrVertex::Swap(CrVertex *vu,CrVertex *vv)
{ //--- remove vu first -------------------------
  int ku = vu->GetKey();
  adjv.erase(ku);
  //TRACE("    SWAP: REM V%04d from V%04d",ku,id);
  if (this == vv)         return 0;
  //--- Add vertex vv ---------------------------
  //TRACE("    SWAP: ADD V%04d into V%04d",vv->GetKey(),id);
  int kv = vv->GetKey();
  std::map<int,CrVertex *>::iterator it= adjv.find(kv);
  if (it != adjv.end())   return 0;
  adjv[kv] = vv;
  vv->Store(this,1);
  return 1;
}
//----------------------------------------------------------------------
//  Enter vertex vk into this vertex
//----------------------------------------------------------------------
void CrVertex::Store(CrVertex *vk,int opt)
{ int kv = vk->GetKey();
  std::map<int,CrVertex *>::iterator it= adjv.find(kv);
  if (it != adjv.end()) {if (opt) gtfo("PB in StoreVertex"); return;}
  adjv[kv] = vk;
  return;
}
//----------------------------------------------------------------------
//  Draw the vertex
//----------------------------------------------------------------------
void  CrVertex::Draw()
{ glNormal3d  (norm.x,norm.y,norm.z);
  glTexCoord2d(ctex.x,ctex.y);
  glVertex3d  (Pos.x, Pos.y, Pos.z);
  return;
}
//----------------------------------------------------------------------
//  Compute the coast of collapse (U to V)
//----------------------------------------------------------------------
double CrVertex::ComputeEdgeCollapseCost(CrVertex *v)
{ // if we collapse edge uv by moving u to v then how
  // much different will the model change, i.e. the 'error'.
  CVector     ext   = *v->GetPosition();
  ext.Subtract(Pos);
  float edgelength  = ext.Length();
  float curvature   = 0;
  // find the 'sides' triangles that are on the edge uv
  std::vector<CrTriangle *> sides;
  std::map<int, CrTriangle *>::iterator it;
  for(it = face.begin(); it != face.end(); it++) 
  { CrTriangle *tr = (*it).second;
    if (tr->IsMarked())   continue;  
    if (tr->HasVertex(v)) sides.push_back(tr);
  }
  // use the triangle facing most away from the sides
  // to determine our curvature term
  int fin = sides.size();
  int i = 0;
  int j = 0;
  for(i=0; i<fin; i++)
  { float mincurv=1;
    CVector       *n0 = sides[i]->GetNorme();
    for(j=i+1; j < fin; j++) 
    { CVector     *n1 = sides[j]->GetNorme();
      // use dot product of face normals.
      double dotp = n0->DotProduct(*n1);
      double fact = (1 - dotp) * 0.5f;
      mincurv     = VAL_MIN(mincurv,fact);
    }
    curvature = VAL_MAX(curvature,mincurv);
  }
  return edgelength * curvature;
}
//----------------------------------------------------------------------
//  Compute the coast of collapse for all edge passing by V
//----------------------------------------------------------------------
void CrVertex::ComputeEdgeCostAtVertex()
{ cand = 0;
  cost = (0 == adjv.size())?(-0.01f):(1000000);
  // search all neighboring edges for 'least cost' edge
  std::map<int,CrVertex*>::iterator it;
  for(it = adjv.begin(); it != adjv.end(); it++ )
  { CrVertex *vn = (*it).second;
    double c = ComputeEdgeCollapseCost(vn);
    if (c > cost) continue;
    cost  = c;
    cand  = vn;
  }
  return;
}

//----------------------------------------------------------------------
//  Remove myself from neighbor list and change to Vertex v
//----------------------------------------------------------------------
void CrVertex::ChangeMeTo(CrVertex *vv)
{ std::map<int,CrVertex*>::iterator it;
  for(it = adjv.begin(); it != adjv.end(); it++ )
  { CrVertex *vn = (*it).second;
    if (0 == vn->Swap(this,vv)) continue;
    //--Add a modifier --------------------------
    vmod.push_back(vn->GetKey());
  }
  return;
}
//----------------------------------------------------------------------
//  Remove vv from neighbor list and reinsert vu change to Vertex v
//----------------------------------------------------------------------
void CrVertex::ChangeBack(CrVertex *vv, CrVertex *vu)
{ //--- Remove VV from near list -------------------
  int kv  = vv->GetKey();
  adjv.erase(kv);
  //TRACE("    ChangeBack REM V%04d in V%04d",kv,id);
  //--- Insert vu ----------------------------------
  int ku  = vu->GetKey();
  std::map<int,CrVertex *>::iterator it= adjv.find(ku);
  if (it != adjv.end())   return;
  adjv[ku]  = vu;
  //TRACE("    ChangeBack ADD V%04d in V%04d",ku,id);
  return;
}
//----------------------------------------------------------------------
//  Refresh coast for all vertex surrounding vv
//----------------------------------------------------------------------
void CrVertex::RefreshCost()
{ ComputeEdgeCostAtVertex();
  std::map<int,CrVertex*>::iterator it;
  for(it = adjv.begin(); it != adjv.end(); it++ )
  { CrVertex *vn = (*it).second;
    vn->ComputeEdgeCostAtVertex();
  }
  return;
}
//----------------------------------------------------------------------
//  Update triangle list:
//  For those that share vv : remove them
//  For those that contains only vu:  change vu to vv
//----------------------------------------------------------------------
void CrVertex::CollapseFaces(CrVertex *vv,CPolyShop *ps)
{ //--- delete triangles on edge uv: ----------------
  std::map<int, CrTriangle*>::iterator it;
  for(it = face.begin();it != face.end(); it++)
  { CrTriangle *tk = (*it).second;
    if  (tk->IsMarked())     continue;
    if  (tk->HasVertex(vv))  ps->Remove(tk,id);
    else tk->ReplaceVertex(this,vv);
  }
  return;
}
//----------------------------------------------------------------------
//  Reactivate triangles:
//  For those that are marked:  reactivate
//  For those that contains only vv:  change vv to vu
//----------------------------------------------------------------------
void CrVertex::ActivateFaces(CrVertex *vv,CPolyShop *ps)
{ //--- Activate triangles from vu list -------------
  std::map<int, CrTriangle*>::iterator it;
  for(it = face.begin();it != face.end(); it++)
  { CrTriangle *tk = (*it).second;
    if (tk->IsMarked())     ps->Restore(tk,id,vv);
    else                    tk->RestoreVertex(vv,this);
  }
  //--- Restore me into near vertices ---------------
  std::map<int, CrVertex*>::iterator vr;
  for (vr = adjv.begin(); vr != adjv.end(); vr++)
  { CrVertex *vk = (*vr).second;
    vk->Store(this,0);
  }
  //--- Explore modifier to restore vertices --------
  for (U_INT k=0; k < vmod.size(); k++)
  { int       kn  = vmod[k];
    CrVertex *vn  = ps->GetVertex(kn);
    vn->ChangeBack(vv,this);
    vv->ChangeBack(vn,this);
  }
  //--- Clear modifiers -----------------------------
  vmod.clear();
  return;
}

//----------------------------------------------------------------------
//  Remove vertex from neighbor list
//----------------------------------------------------------------------
void CrVertex::Remove(CrVertex *vx)
{ int key = vx->GetKey();
  adjv.erase(key);
  return;
}
//----------------------------------------------------------------------
//  Save vertex coordinates
//----------------------------------------------------------------------
void CrVertex::SaveVTX(F3_VERTEX *vtx)
{ vtx[num].VT_X = Pos.x;
  vtx[num].VT_Y = Pos.y;
  vtx[num].VT_Z = Pos.z;
  return;
}
//----------------------------------------------------------------------
//  Save Normal coordinates
//----------------------------------------------------------------------
void CrVertex::SaveNMX(F3_VERTEX *nmx)
{ nmx[num].VT_X = norm.x;
  nmx[num].VT_Y = norm.y;
  nmx[num].VT_Z = norm.z;
  return;
}
//----------------------------------------------------------------------
//  Save Texture coordinates
//----------------------------------------------------------------------
void CrVertex::SaveTXC(F2_COORD *txc)
{ txc[num].VT_S = ctex.x;
  txc[num].VT_T = ctex.y;
  return;
}
//=======================================================================================
//  Triangles
//=======================================================================================
//  Create a new triangle
//----------------------------------------------------------------------
CrTriangle::CrTriangle(CrPart *mp,CrVertex *v0, CrVertex *v1, CrVertex *v2)
{ mark      = 0;
  owner     = 0;
  moPRT     = mp;
  vertex[0] = v0;
  vertex[1] = v1;
  vertex[2] = v2;
}
//----------------------------------------------------------------------
//  Create last triangle
//----------------------------------------------------------------------
CrTriangle::CrTriangle()
{ SetLast();
  owner = 0;
}
//----------------------------------------------------------------------
//  Delete resources
//----------------------------------------------------------------------
CrTriangle::~CrTriangle()
{
}
//----------------------------------------------------------------------
//  Check triangle
//----------------------------------------------------------------------
bool CrTriangle::Check(int mvt)
{ int x0 = vertex[0]->GetNewId();
  if ((x0 >= mvt) || (x0 < 0))  return false;
  int x1 = vertex[1]->GetNewId();
  if ((x1 >= mvt) || (x1 < 0))  return false;
  int x2 = vertex[2]->GetNewId();
  if ((x2 >= mvt) || (x1 < 0))  return false;
  return true;
}
  
//----------------------------------------------------------------------
//  Triangle is removed by vertex vk
//----------------------------------------------------------------------
void CrTriangle::Unmark()
{ mark  = 0;
  owner = 0;
  moPRT->IncFace();
  return;
}
//----------------------------------------------------------------------
//  Triangle is removed by vertex vk
//----------------------------------------------------------------------
void CrTriangle::RemoveBy(int vk)
{ mark  = 1;
  owner = vk;
  moPRT->DecFace();
  return;
}
//----------------------------------------------------------------------
//  Vertex key
//----------------------------------------------------------------------
int CrTriangle::GetVKey(int k)
{ return vertex[k]->GetKey();}
//----------------------------------------------------------------------
//  Edit Mark
//----------------------------------------------------------------------
void CrTriangle::EditMark(char *txt)
{ strcpy(txt,"No");
  if (0 == mark)    return;
  _snprintf(txt,16,"V%04d",owner);
  return;
}
//----------------------------------------------------------------------
//  Compute normal as average of the 3 others
//----------------------------------------------------------------------
void  CrTriangle::Renorme()
{ norm = *vertex[0]->GetNorme();
/*
  norm.Add(*vertex[1]->GetNorme());
  norm.Add(*vertex[2]->GetNorme());
  norm.Times(double(1)/3);
  */
  return;
}
//----------------------------------------------------------------------
//  Draw th triangle
//----------------------------------------------------------------------
void CrTriangle::Draw()
{ vertex[0]->Draw();
  vertex[1]->Draw();
  vertex[2]->Draw();
  return;
}
//----------------------------------------------------------------------
//  Check for vertex v
//----------------------------------------------------------------------
bool CrTriangle::HasVertex(CrVertex *v)
{ if (vertex[0] == v) return true;
  if (vertex[1] == v) return true;
  if (vertex[2] == v) return true;
  return false;
}
//----------------------------------------------------------------------
//  Replace vu by vv in a triangle that does not contain vv
//  NOTE: For vertex vv, those are new triangles that need to be added
//----------------------------------------------------------------------
void CrTriangle::ReplaceVertex(CrVertex *vu,CrVertex *vv)
{ //TRACE("    Replace V%04d by V%04d in T%04d",vu->GetKey(),vv->GetKey(),Id);
  if (vertex[0] == vu) {vertex[0] = vv;}
  if (vertex[1] == vu) {vertex[1] = vv;}
  if (vertex[2] == vu) {vertex[2] = vv;}
  //----Compute normal -----------------------------------
//  Renorme();
  //----Add this triangle to VV neighbor faces -----------
  vv->AddFace(this);
  return;
}
//----------------------------------------------------------------------
//  Replace vv by vu in a triangle that does not contain vv
//  NOTE: Those triangles where not in vv list.  They must be removed
//----------------------------------------------------------------------
void CrTriangle::RestoreVertex(CrVertex *vv,CrVertex *vu)
{ if (vertex[0] == vv) {vertex[0] = vu;}
  if (vertex[1] == vv) {vertex[1] = vu;}
  if (vertex[2] == vv) {vertex[2] = vu;}
  //TRACE("    Restore V%04d by V%04d in T%04d",vv->GetKey(),vu->GetKey(),Id);
  //--- renormalize ---------------------------
//  Renorme();
  //----remove the triangle from vv -----------
  vv->RemFace(this);
  return;
}
//=======================================================================================
//  CrPart
//=======================================================================================
//  Create a begining or end part
//-----------------------------------------------------------------------
CrPart::CrPart(int id,int type,void *rf)
{ Id      = id;
  ntex    = 0;
  ref     = rf;
  if (type == PS_PART_BEG)
  { SetBegMark();
    xOBJ  = globals->txw->Get3DObject(ref);
  }
  else  
  { SetEndMark();
   xOBJ   = 0;
  }
  return;
}
//---------------------------------------------------------------------
//  Save info of interest
//--------------------------------------------------------------------
void CrPart::StoreInfo(M3D_PART_INFO &inf)
{ int lg  = strlen(inf.ntex);
  NbVT    = inf.NbVT;
  NbIN    = inf.NbIN;
  tsp     = inf.tsp;                // Save transparency
  ntex    = new char[lg+1];         // Allocate string
  strncpy(ntex,inf.ntex,lg);        // Copy it
  ntex[lg] = 0;
  return;
}
//---------------------------------------------------------------------
//  Check part integrity
//--------------------------------------------------------------------
void CrPart::CheckPart()
{ if (NbVT < 0)     gtfo("PB with NbVT");
  if (NbIN < 0)     gtfo("PB with NbIN");
  return;
}
//=======================================================================================
//  PolyShop
//=======================================================================================
CPolyShop::CPolyShop()
{ Mod = 0;
  }
//----------------------------------------------------------------------
//  Delete all resources
//----------------------------------------------------------------------
CPolyShop::~CPolyShop()
{ 
  std::map<int,CrVertex *>::iterator vj;
  for (vj = Verts.begin(); vj != Verts.end(); vj++)
  { CrVertex *vt = (*vj).second;
    delete vt;
  }
  Verts.clear();
  //-------------------------------------------------
  while (xLost > 0)
  { CrVertex *vt = vLost[--xLost];
    delete vt;
  }
  delete vLost;
  //-------------------------------------------------
  std::map<int,CrTriangle *>::iterator tj;
  for (tj = Faces.begin(); tj != Faces.end(); tj++)
  { CrTriangle *tx = (*tj).second;
    delete tx;
  }
  Faces.clear();
  //-------------------------------------------------
  std::map<int,CrTriangle *>::iterator tk;
  for (tk = fLost.begin(); tk != fLost.end(); tk++)
  { CrTriangle *tr = (*tk).second;
    delete tr;
  }
  fLost.clear();

}
//----------------------------------------------------------------------
//  Init model
//----------------------------------------------------------------------
void  CPolyShop::SetModel(C3Dmodel *m)
{ Mode  = 0;
  Base  = 0;
  minTR = 0;
  NbrVX = 0;
  NbrTR = 0;
  NbrPR = 0;
  Mod   = m;
  Mod->GetParts(this,inf);
  NbrFO = NbrTR;
  //--- insert last triangle --------------------
  CrTriangle *lt = new CrTriangle();
  lt->SetLast();
  lt->SetKey(NbrTR);
  Faces[NbrTR++] = lt;
  //--- Build all near verticex list ------------
  std::map<int,CrVertex*>::iterator vr;
  for (vr = Verts.begin(); vr != Verts.end(); vr++)
  { CrVertex *vt = (*vr).second;
    vt->BuildNearVertices();
  }
  //--- Compute Initial cost -------------------
  for (vr = Verts.begin(); vr != Verts.end(); vr++)
  { CrVertex *vt = (*vr).second;
    vt->ComputeEdgeCostAtVertex();
  }
  //-- Allocate for lost vertices --------------
  vLost = new CrVertex *[NbrVX];
  xLost = 0;
  redF  = 1.0;
  minTR = NbrFO * 0.1;
  xLast = xLost;
  return;
}
//----------------------------------------------------------------------
//  Get Active vertex
//----------------------------------------------------------------------
CrVertex *CPolyShop::GetVertex(int kn)
{ std::map<int,CrVertex*>::iterator vr = Verts.find(kn);
  if (vr == Verts.end())  gtfo("PB in GetVertex");
  return (*vr).second;
}
//----------------------------------------------------------------------
//  Remove triangle
//----------------------------------------------------------------------
void CPolyShop::Remove(CrTriangle *tr,int m)
{ int key = tr->GetKey();
  Faces.erase(key);
  NbrTR--;
  //TRACE("    T%04d is removed",key);
  //--- Insert TR in remove list ------------
  if (tr->IsMarked()) gtfo("PB with triangle");
  tr->RemoveBy(m);
  fLost[key] = tr;
  return;
}
//----------------------------------------------------------------------
//  Restore  triangle
//----------------------------------------------------------------------
void CPolyShop::Restore(CrTriangle *tr,int m,CrVertex *vv)
{ if (!tr->SameMark(m))   return;
  if (!tr->HasVertex(vv)) return;
//  tr->Renorme();
  int key = tr->GetKey();
  //TRACE("    Restore T%04d from V%04d", key,m);
  Faces[key] = tr;
  NbrTR++;
  tr->Unmark();
  fLost.erase(key);
  return;
}
//----------------------------------------------------------------------
//  Remove Vertex
//  NOTE:  We add vertices in vLost only in the reduction mode (Mode == 0)
//----------------------------------------------------------------------
void CPolyShop::Remove(CrVertex *vu)
{ int key = vu->GetKey();
  Verts.erase(key);
  NbrVX--;
  if (1 == Mode)  return;
  vLost[xLost++] = vu;
  vu->DecPart();
  vu->Mark();
  return;
}
//----------------------------------------------------------------------
//  Add a new part
//----------------------------------------------------------------------
void  CPolyShop::OnePart(M3D_PART_INFO &inf)
{ NbrPR++;
  //---- Add begin part -------------------
  CrPart *p1 = new CrPart(NbrTR,PS_PART_BEG,inf.ref);
  p1->StoreInfo(inf);
  Faces[NbrTR++] = p1;
  //---- Add all vertices -----------------------
  F3_VERTEX *tvb = inf.nVTX;        // Vertices
  F3_VERTEX *tvn = inf.nNRM;        // Normal
  F2_COORD  *txc = inf.nTEX;        // texture coordinates
  for (int k=0; k < inf.NbVT; k++)
  { CVector pos(tvb->VT_X,tvb->VT_Y,tvb->VT_Z);
    CVector nrm(tvn->VT_X,tvn->VT_Y,tvn->VT_Z);
    CrVertex *vtx = new CrVertex(p1,pos,NbrVX);
    vtx->SetNorme(nrm);
    vtx->SetTexture(txc->VT_S,txc->VT_T);
    Verts[NbrVX++] = vtx;
    tvb++;
    tvn++;
    txc++;
  }
  //-----Add Faces------------------------------
  int   *ind = inf.nIND;
  int    nbx = inf.NbIN - (inf.NbIN % 3);
  for (int p=0; p<nbx; p+=3)
  { int a = (*ind++) + Base;
    int b = (*ind++) + Base;
    int c = (*ind++) + Base;
    CrVertex *va = Verts[a];
    CrVertex *vb = Verts[b];
    CrVertex *vc = Verts[c];
    CrTriangle *trg = new CrTriangle(p1,va,vb,vc);
    trg->SetKey(NbrTR);
    trg->Renorme();
    Faces[NbrTR] = trg;
    //--Add the common triangle to vertices ----
    va->AddFace(trg);
    vb->AddFace(trg);
    vc->AddFace(trg);
    //--Add near vertices ----------------------
    NbrTR++;
  }
  //---- Add close part -------------------
  CrPart *p2 = new CrPart(NbrTR,PS_PART_END,0);
  Faces[NbrTR++] = p2;
  //-------------------------------------------
  Base  = NbrVX;
  return;
}
//----------------------------------------------------------------------
//  Find the best vertex
//----------------------------------------------------------------------
void CPolyShop::GetBestVertex()
{ best  = 0;
  cost  = 20000000;
  std::map<int,CrVertex*>::iterator it;
  for(it = Verts.begin(); it != Verts.end(); it++)
  { CrVertex *vk = (*it).second;
    double  cvk  = vk->GetCost();
    if (cvk > cost)  continue;
    best  = vk;
    cost  = cvk;
  }
  return;
}
//----------------------------------------------------------------------
//  Collapse vertex
//----------------------------------------------------------------------
void  CPolyShop::Collapse(CrVertex *vu)
{ if (0 == vu)  return;
  CrVertex *vv = vu->BestOne();
  if (0 == vv)  {Remove(vu); return;}
  //TRACE("======COLLAPSE V%04d to V%04d==================",vu->GetKey(),vv->GetKey());
  //vu->Trace("BEFORE");
  //vv->Trace("BEFORE");
  //--- remove vu from near vertices ----------------
  vu->ChangeMeTo(vv);
  vu->CollapseFaces(vv,this);
  //--- Remove vector vu ----------------------------
  Remove(vu);
  vv->RefreshCost();
  //vu->Trace("AFTER");
  //vv->Trace("AFTER");
  return;
}

//----------------------------------------------------------------------
//  Recover last vertex
//----------------------------------------------------------------------
void CPolyShop::RecoverLastVertex()
{ if (xLost == 0) return;
  CrVertex *vu = vLost[--xLost];
  CrVertex *vv = vu->BestOne();
  //TRACE("=====RECOVER V%04d =======================",vu->GetKey());
  //vu->Trace("BEFORE");
  //if (vv)   vv->Trace("BEFORE");
  if (vv)        vu->ActivateFaces(vv,this);
  int key      = vu->GetKey();
  Verts[key]   = vu;
  NbrVX++;
  vu->IncPart();
  vu->Unmark();
  //vu->Trace("AFTER");
  if (vv)   vv->RefreshCost();
  //if (vv)   vv->Trace("AFTER");
}
//----------------------------------------------------------------------
//  Collapse model
//  rd = relative reduction ratio
//----------------------------------------------------------------------
int  CPolyShop::ReduceBy(double rd)
{ float old = redF;   // Previous reduction
  if (rd > 1)   return 0;
  //---Compute polygon number to reach ---
  int tgt  = int (NbrFO * rd);
  if (tgt < minTR)  tgt = minTR;
  while (NbrTR > tgt)
  { GetBestVertex();
    Collapse(best);
    if (NbrVX) continue;
    //--No more vertex, but still face?--
    return RebuildModel(old);
  }
  redF  = double(NbrTR) / NbrFO;
  xLast = xLost;
  return 1;
}
//----------------------------------------------------------------------
//  Rebuild model
//  rd = relative reduction ratio
//----------------------------------------------------------------------
int CPolyShop::RebuildModel(double rd)
{ //----Compute polygon number to reach ---
  int tgt = int (NbrFO * rd);
  if (tgt > NbrFO)  tgt = NbrFO;
  while (NbrTR < tgt) RecoverLastVertex();
  redF  = double(NbrTR) / NbrFO;
  return 1;
}
//----------------------------------------------------------------------
//  Reduce model
//  rd = relative reduction ratio
//  NOTE: As we are in scan mode, hte vLost list is already ordered
//        by best vertice candidate to collapse.
//----------------------------------------------------------------------
int CPolyShop::ReduceModel(double rd)
{ //----Compute polygon number to reach --
  int tgt = int (NbrFO *rd);
  if (tgt < minTR)  tgt = minTR;
  while ((NbrTR > tgt) && (xLost < xLast))   Collapse(vLost[xLost++]);
  redF  = double(NbrTR) / NbrFO;
  return 1;
}
//----------------------------------------------------------------------
//  Build the model at the requested ratio
//  
//----------------------------------------------------------------------
int  CPolyShop::BuildModel(double rd)
{ Mode = 1;
  if (rd < redF)  ReduceModel (rd);
  else            RebuildModel(rd);
  return 1;
}
//----------------------------------------------------------------------
//  Renum all remaining vertices
//  Restart to 0 when changing part
//----------------------------------------------------------------------
void CPolyShop::RenumVertices()
{ std::map<int,CrVertex*>::iterator vi;
  CrPart *cp = 0;
  int    ind = 0;
  for (vi = Verts.begin(); vi!=Verts.end(); vi++)
  { CrVertex *vx = (*vi).second;
    CrPart   *pt = vx->GetHead();
    if (pt != cp) ind = 0;
    cp      = pt;
    vx->NewIndice(ind++);
  }
  return;
}
//----------------------------------------------------------------------
//  Intro Part is used to set texture
//----------------------------------------------------------------------
bool CPolyShop::BegPart(CrTriangle *tr)
{ if (!tr->HasBegMark())   return false;
  CrPart *prt = (CrPart*)tr;
  glBindTexture(GL_TEXTURE_2D,prt->xOBJ);
  glBegin(GL_TRIANGLES);
  return true;
}
//----------------------------------------------------------------------
//  End Part is used to end drawing
//----------------------------------------------------------------------
bool CPolyShop::EndPart(CrTriangle *tr)
{ if (!tr->HasEndMark())  return false;
  glEnd();
  return true;
}
//----------------------------------------------------------------------
//  Draw from the components
//----------------------------------------------------------------------
void CPolyShop::Draw()
{ U_INT obj = 0;
  U_INT dir = Mod->GetRDIR();
  glFrontFace(dir);
  std::map<int,CrTriangle *>::iterator it;
  for (it = Faces.begin(); it != Faces.end(); it++)
  { CrTriangle *tr = (*it).second;
    if (BegPart(tr))      continue;
    if (EndPart(tr))      continue;
    if (tr->IsLast())     continue;
    tr->Draw();
  }
  return;
}
//----------------------------------------------------------------------
//  If triangle is a texture triangle, then create a new part
//----------------------------------------------------------------------
bool CPolyShop::OpenPart(CrTriangle *tr)
{ if (!tr->HasBegMark())    return false;
  CrPart *phd = (CrPart*)tr;
  //--- Allocate a new 3D part -------------------------
  if (0 != Part)      gtfo("PB in BegTriangle");
  phd->CheckPart();
  //--- Ignore empty parts -----------------------------
  if (phd->IsEmpty()) return true;
  //--- Allocate a new part ----------------------------
  nbPRT++;
  pnNVT = phd->GetNbVertices();
  pnNIN = phd->GetNbIndices();
  Part  = new C3DPart();
	Part->AllocateW3dVTX(pnNVT);
  Part->SetTSP(phd->GetTSP());
  Part->SetTexName(phd->GetTXN());
  Part->AllocateXList(pnNIN);
  Part->SetXOBJ(phd->GetXOB());
  vaTAB = Part->GetVLIST();
  vnTAB = Part->GetNLIST();
  txTAB = Part->GetTLIST();
  xiTAB = Part->GetXLIST();
  crIND = 0;
  return true;
}
//----------------------------------------------------------------------
//  This is an  End triangle. Save part in related LOD
//  
//----------------------------------------------------------------------
bool CPolyShop::ClosePart(CrTriangle *tr)
{ if (!tr->HasEndMark())        return false;
  if (0 == Part )               return true;
  if (0 == crIND) 
  { delete Part; 
    Part  = 0;
    return true;}  
  //--- Add to the model ------------------
  Part->ReduceIndice(crIND);
  Mod->AddLodPart(Part,nLOD);
  Part  = 0;
  return true;
}
//----------------------------------------------------------------------
//  This is a normal triangle. Save components
//----------------------------------------------------------------------
bool CPolyShop::Component(CrTriangle *tr)
{ if (tr->IsLast()) return false;
  CrVertex *v0 = tr->GetVertex(0);
  CrVertex *v1 = tr->GetVertex(1);
  CrVertex *v2 = tr->GetVertex(2);
  if (!tr->Check(pnNVT))  return true;
  //---- Save Vertex V0 -------------------
  v0->SaveVTX(vaTAB);
  v0->SaveNMX(vnTAB);
  v0->SaveTXC(txTAB);
  //---- Save Vertex V1 -------------------
  v1->SaveVTX(vaTAB);
  v1->SaveNMX(vnTAB);
  v1->SaveTXC(txTAB);
  //---- Save Vertex V2 -------------------
  v2->SaveVTX(vaTAB);
  v2->SaveNMX(vnTAB);
  v2->SaveTXC(txTAB);
  //---- Save the 3 new indices -----------
  int x0 = v0->GetNewId();
  xiTAB[crIND++]  = x0;
  if (crIND > pnNIN)  gtfo("PB with X0");

  int x1 = v1->GetNewId();
  xiTAB[crIND++]  = x1;
  if (crIND > pnNIN)  gtfo("PB with X1");

  int x2 = v2->GetNewId();
  xiTAB[crIND++]  = x2;
  if (crIND > pnNIN)  gtfo("PB with X2");
  return true;
}
//----------------------------------------------------------------------
//  Store triangle list
//----------------------------------------------------------------------
int CPolyShop::StoreParts(int lod)
{ nLOD    = lod;
  RenumVertices();
  nbPRT   = 0;
  Part    = 0;
  std::map<int,CrTriangle *>::iterator it;
  for (it = Faces.begin(); it != Faces.end(); it++)
  { CrTriangle *tr = (*it).second;
    if (OpenPart (tr))    continue;
    if (ClosePart(tr))    continue;
    if (Component(tr))    continue;
  }
  return NbrTR;
}
//---------------------------------------------------------------------
//  Model standard:
//	3 Level of Details are created in the model queue:
//		LOD1 => 0.6 reduction
//		LOD2 => 0.4 reduction
//		LOD3 => 0.2 reduction
//---------------------------------------------------------------------
void CPolyShop::ModelStandard(int mint)
{ minTR = mint;
  if (NbrFO <= minTR)     return;
  //---Build LOD 1  at 60% --------------------------
  if (0 == ReduceBy(0.6)) return;
  StoreParts(1);
  //---Build LOD 2 at 40% ---------------------------
  if (0 == ReduceBy(0.4)) return;
  StoreParts(2);
  //---Build LOD 3 at 20% ---------------------------
  if (0 == ReduceBy(0.2)) return;
  StoreParts(3);
  return;
}
//----------------------------------------------------------------------
//  Draw activity notice
//----------------------------------------------------------------------
void CPolyShop::Display(int k)
{ char txt[1024];
  sprintf(txt,"%s :REDUCTION to LOD %d",Mod->GetFileName(),k);
  globals->fui->DrawNoticeToUser(txt,20);
  return;
}
//================END OF THIS FILE ======================================================
