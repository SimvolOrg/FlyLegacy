/*
 * REDUCTOR.h
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
 *
 */



#ifndef REDUCTOR_H
#define REDUCTOR_H
#include "FlyLegacy.h"
#include "../Include/3DMath.h"
#include "../Include/model3d.h"
//================================================================================
//  CREDUCTOR IS A POLYGON MESH REDUCTOR
//================================================================================
class CrTriangle;
class CrVertex;
class CPolyShop;
class CrPart;
//============================================================================
//  RTriangle is also used in polygon reduction
//  NOTE:  Triangle with MARK == 2 is for texture change
//============================================================================

class CrTriangle {
  //---- ATTRIBUTE ----------------------------------------------
public:
  int       Id;
  CrVertex *vertex[3];          // the 3 points that make this triangle
  CVector   norm;               // orthogonal unit vector
  U_CHAR    mark;               // Triangle state
  int       owner;              // Vertex responsible
  CrPart   *moPRT;              // Mother part
  //---- METHODS--------------------------------------------------
  CrTriangle(CrPart *mp, CrVertex *v0,CrVertex *v1,CrVertex *v2);
  CrTriangle();
  virtual ~CrTriangle();
  bool  HasVertex(CrVertex *v);
  bool  Check(int mvt);
  void  ReplaceVertex(CrVertex *vu,CrVertex *vv);
  void  RestoreVertex(CrVertex *vv,CrVertex *vu);
  void  Renorme();
  void  RemoveBy(int vk);
  void  Unmark();
  void  EditMark(char *txt);
  //--------------------------------------------------------------
  int       GetVKey(int k);
  //--------------------------------------------------------------
  void  Draw();
  //--------------------------------------------------------------
  inline CrVertex *GetVertex(int t)   {return vertex[t];}
  inline CVector  *GetNorme()         {return &norm;}
  inline int       GetKey()           {return Id;}
  //--------------------------------------------------------------
  inline void      SetKey(int k)      {Id = k;}
  inline void      SetBegMark()       {mark = 0xF0;}
  inline void      SetEndMark()       {mark = 0xF1;}
  inline void      SetLast()          {mark = 0xFF;}
  inline bool      IsMarked()         {return (mark != 0);}
  inline bool      SameMark(int k)    {return (owner == k);}
  inline bool      HasBegMark()       {return (mark == 0xF0);}
  inline bool      HasEndMark()       {return (mark == 0xF1);}
  inline bool      IsLast()           {return (mark == 0xFF);}
};
//============================================================================
//  CrPart is used to memorize the separation into Parts
//============================================================================
#define PS_PART_BEG  1
#define PS_PART_END  2
class CrPart: public CrTriangle
{ public:
  //--- Part attributes ---------------------------------
  int         type;                   // Part type
  U_CHAR      tsp;                    // Transparent indicator
  //-----------------------------------------------------
  int         NbVT;                   // Number of vertices
  int         NbIN;                   // Number of Indices
  M3D_PART_INFO  *txp;								// Texture info
  U_INT      xOBJ;                    // Texture object
	char      *texn;										// Texture name
  //-----METHOD -----------------------------------------
  CrPart(int id,int type,M3D_PART_INFO *ref);
 ~CrPart()	{delete texn;}
  void  StoreInfo(M3D_PART_INFO &inf);
  void  CheckPart();
	char *GetTexName();
  //-----------------------------------------------------
  inline void DecFace()     {NbIN -= 3;}
  inline void IncFace()     {NbIN += 3;}
  inline void DecVertex()   {NbVT--;}
  inline void IncVertex()   {NbVT++;}
  //-----------------------------------------------------
  inline bool IsEmpty()       {return (NbIN == 0);}
  inline int  GetNbVertices() {return NbVT;}
  inline int  GetNbIndices()  {return NbIN;}
  inline U_CHAR GetTSP()      {return tsp;}
  inline U_INT  GetXOB()      {return xOBJ;}
};
//============================================================================
//  Modificator
//============================================================================
#define PS_VTX  1
#define PS_TRG  2
//============================================================================
struct PS_MODIF {
  U_INT oper;                           // Operator
  U_INT objt;                           // Modificator
};
//============================================================================
//  Modifiers for list of triangle
//  A modifier is used to replace one vertex in a triangle T by another vertex
//  Wich vertices are exchanged depends on the direction of the change
//  -When the direction is to reduce the number of polygons, then
//   the summit sTri is set to vInd value.
//  -When the direction is to augment the number of polygons, then 
//   the summit sTri is set to uInd value.
//============================================================================
struct M3D_MODIF {
  int sTri;             // Summit index in triangle list
  int uInd;             // Indice of vertice VU in vertex list
  int vInd;             // Indice of vertice VV in vertex list
};
//============================================================================
//  Rvertex is used by the polygon reduction algorithm
//============================================================================
class CrVertex {
  //--- ATTRIBUTES -------------------------------------------
  public:
  CVector Pos;      // location of this point
  //----------------------------------------------------------
  U_CHAR state;     // Vertex state
  int id;           // place of vertex in original list
  int num;          // New index after reduction
  std::map<int, CrVertex   *> adjv;     // adjacent vertices
  std::map<int, CrTriangle *> face;     // adjacent triangles
  std::vector<int>            vmod;     // List of modifications
  double    cost;                       // cached cost of collapsing edge
  CVector   norm;                       // Normal vector
  CVector   ctex;                       // texture coordinates
  CrVertex *cand;                       // candidate vertex for collapse
  CrPart   *moPRT;                      // Mother part
  //-----------------------------------------------------------
  double    coef;
  //--- METHODS------------------------------------------------
  CrVertex(CrPart *mp,CVector v,int _id);
 ~CrVertex();
  void    AddFace(CrTriangle *tr);
  void    RemFace(CrTriangle *tr);
  void    RemoveIfNonNeighbor(CrVertex *n);
  void    Draw();
  void    Store(CrVertex *v,int opt);
  double  ComputeEdgeCollapseCost(CrVertex *v);
  void    ComputeEdgeCostAtVertex();
  void    ChangeMeTo(CrVertex *vv);
  void    CollapseFaces(CrVertex *vv,CPolyShop *ps);
  void    Remove(CrVertex *vx);
  void    ActivateFaces(CrVertex *vv,CPolyShop *ps);
  void    BuildNearVertices();
  void    Trace(char *msg);
  //-----------------------------------------------------------
  int     Swap(CrVertex *vu,CrVertex *vv);
  void    EnterVertex(CrVertex *vk);
  void    ChangeBack(CrVertex *vv,CrVertex *vu);
  void    RefreshCost();
  //-----------------------------------------------------------
  void    SaveVTX(F3_VERTEX *vtx);
  void    SaveNMX(F3_VERTEX *nmx);
  void    SaveTXC(F2_COORD  *txc);
  //-----------------------------------------------------------
  inline void SetNorme(CVector n)     {norm = n;}
  inline void SetTexture(float s,float t) {ctex.x = s,ctex.y = t;}
  //------------------------------------------------------------
  inline CVector *GetNorme()          {return &norm;}
  inline CVector *GetPosition()       {return &Pos;}
  inline CVector *GetTexture()        {return &ctex;}
  inline double   GetCost()           {return cost;}
  inline CrVertex *BestOne()          {return cand;}
  inline int      GetKey()            {return id;}
  //-----------------------------------------------------------
  inline void     Mark()              {state = 1;}
  inline void     Unmark()            {state = 0;}
  //-----------------------------------------------------------
  inline void     DecPart()           {moPRT->DecVertex();}
  inline void     IncPart()           {moPRT->IncVertex();}
  inline void     NewIndice(int k)    {num = k;}
  inline int      GetNewId()          {return num;}
  inline CrPart  *GetHead()           {return moPRT;}
  //-----------------------------------------------------------

};


//============================================================================
//  CPolyShop is the polygon mesh reductor
//============================================================================

class CPolyShop
{
  //--- ATTRIBUTES ----------------------------------------------
  U_CHAR  Mode;                             // PolyShop Mode
  M3D_PART_INFO inf;
  int NbrFO;                                // Original face count
  int NbrPR;                                // Total parts
  int NbrVX;                                // Total vertices
  int NbrTR;                                // Total triangles
  int Base;                                 // Base for numbering
  std::map<int,CrVertex *>   Verts;         // List of vertices
  std::map<int,CrTriangle *> Faces;         // List of faces
  std::map<int,CrTriangle*>  fLost;         // Removed triangles
  void      *ref;                           // Texture reference
  CrVertex *best;
  double    cost;
  double    coef;                           // Curvature coef
  //-----Removed vertices ---------------------------------------
  int        xLast;
  int        xLost;
  CrVertex **vLost;                         // Lost vertices
  //-----Working area for rebuilding -----------------------------
  int         minTR;                        // Minimum faces for reduction
  int         nLOD;                         // Level of Detail
  C3DPart    *Part;                         // Current Part
  int         pnNVT;                        // vertice count in part
  int         pnNIN;                        // Indice count in part
  F3_VERTEX  *vaTAB;                        // Vertice array
  F3_VERTEX  *vnTAB;                        // Normal array
  F2_COORD   *txTAB;                        // Texture coordinates
  int        *xiTAB;                        // indice array
  int         crIND;                        // Current entry in xiTAB
  int         nbPRT;                        // Number of out part
  //-------------------------------------------------------------
  double    redF;                           // Reduction factor
  C3Dmodel *Mod;
  //-------------------------------------------------------------
public:
  CPolyShop();
 ~CPolyShop();
 //--------------------------------------------------------------
 CrVertex *GetVertex(int kn);
 void   GetBestVertex();
 //--------------------------------------------------------------
 void   OnePart(M3D_PART_INFO &inf);
 double ComputeEdgeCollapseCost(CrVertex *u,CrVertex *v);
 void   Collapse(CrVertex *u);
 int    ReduceBy(double rd);
 int    RebuildModel(double rd);
 int    ReduceModel(double rd);
 int    BuildModel(double rd);
 //--------------------------------------------------------------
 void   RecoverLastVertex();
 //--------------------------------------------------------------
 void   SetModel(C3Dmodel *m);
 void   Remove(CrTriangle *tr,int m);
 void   Remove(CrVertex *vu);
 void   Restore(CrTriangle *tr,int m,CrVertex *vv);
 bool   BegPart(CrTriangle *tr);
 bool   EndPart(CrTriangle *tr);
 void   Draw();
 //-----PART RECONSTRUCTION ------------------------------------
 void   RenumVertices();
 int    StoreParts(int lod);
 bool   OpenPart(CrTriangle *tr);
 bool   ClosePart(CrTriangle *tr);
 bool   Component(CrTriangle *tr);
 void   ModelStandard(int mint);
 void   Display(int k);
 //--------------------------------------------------------------
 inline int   GetNbVertices()   {return NbrVX;}
 inline int   GetNbFaces()      {return NbrTR;}
 inline void  SetCoef(double w) {coef = w;}
 //--------------------------------------------------------------
 inline double GetFactor()      {return redF;}
};

//==============END OF FILE ======================================================
#endif // REDUCTOR_H

