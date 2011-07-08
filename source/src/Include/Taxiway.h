/*
 * TAXIWAY.h
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
#ifndef TAXIWAY_H
#define TAXIWAY_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "../Include/Globals.h"
#include "../Include/FlyLegacy.h"
#include "../Include/Airport.h"
//============================================================================
//============================================================================
//  Node structure for runway and taxiways
//============================================================================
class CTaxiNode: public CStreamObject
{ //---------------------------------------------------------------
  SVector       offset;                     // Offset from origin. 1 unit = 16 feets
  U_SHORT       direction;                  // Direction for ????
  int           nx;                         // x origin in feet
  int           ny;                         // y dito
  //-----------Methods --------------------------------------------
public:
   CTaxiNode();
  ~CTaxiNode();
  int           Read (CStreamFile *sf, Tag tag);    // Read method
  void          Scale(float scale);
  //----------------------------------------------------------------
  inline  void  SetVector(SVector &v)       {offset = v;}
  inline  void  SetDirection(U_INT d)       {direction = d;}
  inline  void  SetInFeet(short x,short y)  {nx = x; ny= y;}
  inline  int   GetXfeet()                  {return nx;}
  inline  int   GetYfeet()                  {return ny;}
};
//============================================================================
//  Edge structure for runway and taxiways
//============================================================================
class CTaxiEdge: public CStreamObject
{ //------------Attribute ----------------------------------------
  U_SHORT       oNode;                        // Node number for origin
  U_SHORT       xNode;                        // Node number for extremity
  short         type;                         // Type of edge
  float         thick;
  //-----------Methods ------------------------------------------
public:
   CTaxiEdge();
  ~CTaxiEdge();
  int           Read (CStreamFile *sf, Tag tag);    // Read method
  //-----------innline ------------------------------------------
  inline  void    SetOrigin(short nn)         {oNode = nn;}
  inline  void    SetExtrem(short nn)         {xNode = nn;}
  inline  U_SHORT GetOrigin()                 {return oNode;}
  inline  U_SHORT GetExtrem()                 {return xNode;}
  inline  short   GetType()                   {return type;}
  inline  float   GetThick()                  {return thick;}
};
//============================================================================
//  Class CDataBGR for Taxiway line description
//============================================================================
class CDataBGR: public CStreamObject
{ //-------------Attributes ---------------------------------------------
  SPosition       org;                                // Origin
  std::vector<CTaxiNode *>  nodelist;                 // List of nodes
  std::vector<CTaxiEdge *>  edgelist;                 // List of edges
  //-------------Coordinate extension in the 4 directions (feet)---------
  int             nExt;                               // North extension
  int             sExt;                               // South extension
  int             eExt;                               // east
  int             wExt;                               // west
  //-------------Scale factor -------------------------------------
  float           scale;
  //------------ Color --------------------------------------------
  U_INT           white;
  //------------- Methods -----------------------------------------
public:
   CDataBGR(CAptObject *apo);
  ~CDataBGR();
  void        EmptyAll();
  bool        DecodeBinary(char *fname);
  int         Read (CStreamFile *sf, Tag tag);    // Read method
  void        StoreExtension(CTaxiNode *nod);
  U_INT       GetExtension();
  CTaxiNode  *GetNode(U_INT No);
  CTaxiEdge  *GetEdge(U_INT No);
  bool        GetLine(U_INT No,int &x0,int &y0,int &x1,int &y1);
  void        Rescale(float scale);
  bool        DrawSegment(U_INT No,SSurface *sf,int xm,int ym);
  //-----------------------------------------------------------------
  U_INT       GetEdgeNumber()     {return edgelist.size();}
};

//============================================================================
//  Class CDataTMS to decode Taxiway pavement structure
//============================================================================
class CDataTMS: public CStreamObject
{ //-------------Attributes ---------------------------------------
  CAptObject     *apo;
  SPosition       org;                      // Airport origin
  SPosition       dpo;                      // Polygon origin
  float           ground;                   // Common ground
  //-----------TAGS -----------------------------------------------
  long            tlx;                      // Grid X number
  long            tlz;                      // Grid Z number
  float           scale;                    // grid size
  //---------------------------------------------------------------
  SPosition       llc;                      // Lower left corner
  //-------------Methods ------------------------------------------
public:
  CDataTMS(CAptObject *apo);
  bool         DecodeBinary(char *fn);
  int          Read (CStreamFile *sf, Tag tag);    // Read method
  //---------------------------------------------------------------
  inline float GetXorigin()    {return dpo.lon;}
  inline float GetYorigin()    {return dpo.lat;}
  inline float GetZorigin()    {return dpo.alt;}
  //----------------------------------------------------------------
  inline float GetGround() {return ground;}
  inline float GetScale() {return scale;}
  inline float GetAptAltitude() {return org.alt;}
  //---------------------------------------------------------------
  void  AddPavement(CPaveQ &q,U_INT n);
  void  AddEdge(CPaveQ &q,U_INT n);
  void  AddCenter(CPaveQ &q,U_INT n);
  //---------------------------------------------------------------
  inline SPosition *GetTaxOrigin()    {return &dpo;}
  inline SPosition *GetAptOrigin()    {return &org;}
  inline CAptObject *ApOBJ()          {return apo;}
};
//=============================================================================
//  Class CTmsTile:  used to decode TMS file
//=============================================================================
class CTileTMS: public CStreamObject
{ CDataTMS    *tms;
  CAptObject  *apo;                               // Airport object
  CPaveQ       dtq;                               // Data Queue
  double       PavFac;                            // Pavement dimension
  SPosition    llc;                               // SW corner
  //----Total triangle -----------------------------------------
  int     total;
  //------------------------------------------------------------
  float   scale;                                  // Feet scaling factor
  long      tx;
  long      tz;
  long      lgr;                                  // Data size
  //------------------------------------------------------------
  long    unk1;                                   // Unknown
  long    unk2;                                   // Unknown
  long    unk3;                                   // Unknown
  long    unk4;                                   // Unknown
  //-------------------------------------------------------------
  long    tx1;                                    // Unknown
  long    tx2;                                    // Unknown
  long    tx3;                                    // Unknown
  long    tx4;                                    // Unknown
  long    tx5;                                    // Unknown
  //------PAVEMENT TYPE ----------------------------------------
  U_CHAR  Type;                                   // Type of pavement
  //------------------------------------------------------------
  int     Nbre;                                   // Number of floats
  //-------------------------------------------------------------
  long      nbv;                                  // Number of vertices
  //------Light decoding ------------------------------------------
  U_SHORT   inx;                                  // Coordinates entry
  U_SHORT   nb0;                                  // Current entry
  U_SHORT  *tp0;                                  // Temporary table color 0
  U_SHORT   nb1;                                  // Current entry
  U_SHORT  *tp1;                                  // Temporary table color 1
  TC_WORLD *spot;                                 // temporary light coordinate array
  //------Methods -------------------------------------------------
public:
  CTileTMS(CDataTMS *tm);
  void        Abort(char *msg);
  int         Read (CStreamFile *sf, Tag tag);    // Read method
  void        ReadData(CStreamFile *sf);
  void        ReadLight(CStreamFile *sf);
	void				FillTriangles(CStreamFile *sf,int nb,TC_VTAB *tab);
  int         ReadLightParam(CStreamFile *sf);
  void        ComputeElevation(TC_VTAB *tab);
  void        SpotElevation(int k);
  //---------------------------------------------------------------
  inline void   SetLLC(SPosition p)     {llc = p;}
};

//============================END OF FILE =================================================
#endif  // TAXIWAY_H
