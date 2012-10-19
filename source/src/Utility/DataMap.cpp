/*
 * DataMap.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2004-2005 Chris Wallace
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

/*! \file DataMap.cpp
 *  Implementation of classes related to data lookup objects 
 */

#include "../Include/FlyLegacy.h"
#include "../Include/Utility.h"

#include <map>
using namespace std;

//====================================================================
// CSlotPolynomial
//
// This CDataLookup implements a polynomial mapping up to 9th order:
//
//                              2                 9
//   y = a0 + (a1 * x) + (a2 * x)  + ... + (a9 * x)
//
//=====================================================================
CSlotPolynomial::CSlotPolynomial (void)
{ n = 0;
  int i;
  for (i=0; i<10; i++) c[i] = 0.0f;
}

//======================================================================
// This constructor creates a polynomial with interpolated coefficients
//========================================================================
void CSlotPolynomial::Interpolate (CSlotPolynomial *p1,
                                      CSlotPolynomial *p2,
                                      float scale)
{
  int i;
  for (i=0; i<10; i++) {
    float f;
    if (p2->c[i] < 0 && p1->c[i] < 0) {
      f = -(float)fabs(p2->c[i] + p1->c[i]);
    } else if (p2->c[i] > 0 && p1->c[i] > 0) {
      f =  (float)fabs(p2->c[i] - p1->c[i]);
    } else {
      f =   p2->c[i] + p1->c[i];
    }
    c[i] = p1->c[i] + (f * scale);

      #ifdef _DEBUG_	
	      FILE *fp_debug = fopen("__DDEBUG_.txt", "a");
	      if(fp_debug != NULL)
	      {
		      int test = 0;
		      fprintf(fp_debug, "CSlotPolynomial::Interpolate %d\\ s%.4f d%.4f p1)%.4f p2)%.4f NC%.4f\n",
                  i,
                  scale,
                  f,
                  p1->coeff[i],
                  p2->coeff[i],
                  coeff[i]);
		      fclose(fp_debug); 
	      }
      #endif
  }
}
//---------------------------------------------------------------
//  Enter coefficient in table
//---------------------------------------------------------------
void CSlotPolynomial::SetCoefficient (int i, float a)
{ if (i < 10) {c[i] = a; n++;}
  return;}
//---------------------------------------------------------------
//  Enter coefficient in table
//---------------------------------------------------------------
int CSlotPolynomial::Read (SStream *stream, Tag tag)
{ switch (tag) {
  case 'coef':
    {
      // From zero to ten coefficients may be present
      char s[80];
      ReadString (s, 80, stream);
      char k = 0;
      char *delimiters = " \t";
      char *token = strtok (s, delimiters);
      while ((token) && (k < 10)) {
        float f = float(atof (token));
        SetCoefficient (k++, f);
        token = strtok (NULL, delimiters);
      }
    }
    return TAG_READ;
  }
  char s[8];
  TagToString (s, tag);
  WARNINGLOG ("CSlotPolynomial::Read : Unknown tag %s", s);
  return TAG_IGNORED;
}
//--------------------------------------------------------------
//  Compute value with coefficients
//--------------------------------------------------------------
float CSlotPolynomial::Lookup (float x)
{ float cx = x;
  float y  = c[0];
  int i;
  for (i=1; i<n; i++) {if (c[i] != 0) {y += c[i] * cx; }
    cx *= x;
  }

//--- JS Rewrite this ---------------------------------------
//  for (i=1; i<n; i++) {if (c[i] != 0) {y += c[i] * x; }
//    x *= x;     // This is a bug because x = x^2 then x^4 then x^16, etc
                  // While it should be y = a + bx + cx^2 + dx^3...etc;
                
//  }

  return y;
}
//==============================================================================
//  FMT1Slot to support FMT1 table
//==============================================================================
CFmt1Slot::CFmt1Slot()
{ x = y = dv = 0;
}
//---------------------------------------------------------------------
//  Constructor with values
//---------------------------------------------------------------------
CFmt1Slot::CFmt1Slot(float x, float y)
{ this->x   = x;
  this->y   = y;
  this->dv  = 0;
}
//---------------------------------------------------------------------
//  Destructor
//---------------------------------------------------------------------
CFmt1Slot::~CFmt1Slot()
{}
//==============================================================================
//  C3valSlot to support table with 3 outputs
//==============================================================================
C3valSlot::C3valSlot(float x,float u, float v, float w)
{ this->x = x;
  this->u = u;
  this->v = v;
  this->w = w;
  du = dv = dw = 0;
}

//==============================================================================
//  FMT3Slot to support FMT3 table
//==============================================================================
CFmt3Slot::CFmt3Slot()
{ x = 0;
}
//---------------------------------------------------------------------
//  Destructor
//---------------------------------------------------------------------
CFmt3Slot::~CFmt3Slot()
{}
//==============================================================================
//  CTbl1Map  is a decoder for <tbl1> tag
//==============================================================================
CTbl1Map::CTbl1Map(SStream *s)
{ tab = 0;
  ReadFrom(this,s);
}
//---------------------------------------------------------------------
//  Read the table format
//---------------------------------------------------------------------
int CTbl1Map::Read(SStream *s,Tag tag)
{ switch (tag)  {
    case 'fmt1':
      { CFmt1Map *map = new CFmt1Map();
        tab     = map;
        map->DecodeFMT1(s);
        return TAG_READ;
      }
    case 'fmt3':
      { CFmt3Map *map = new CFmt3Map();
        tab     = map;
        map->DecodeFMT3(s);
        return TAG_READ;
      }

  }
  char ed[8];
  TagToString(ed,tag);
  gtfo ("CDataSearch::Read : Unknown tag %s in %s", ed, s->filename);

  return TAG_IGNORED;
}
//==============================================================================
//  CCoefMap  is a decoder for <cof1> tag
//==============================================================================
CCoefMap::CCoefMap(SStream *s)
{ a = 0;
  ReadFloat(&a,s);
}
//==============================================================================
//  CPolyMap  is a decoder for <cof1> tag
//==============================================================================
CPolyMap::CPolyMap(SStream *s)
{ ReadFrom(&poly,s);
}
//==============================================================================
//  CmvalMap to support table with 2 output
//==============================================================================
CmvalMap::CmvalMap()
{}
//---------------------------------------------------------------------
//  Destroy resources
//---------------------------------------------------------------------
CmvalMap::~CmvalMap()
{ tab.clear();
}
//---------------------------------------------------------------------
//  Enter a new slot
//---------------------------------------------------------------------
void CmvalMap::Enter(C3valSlot &e)
{ if (!tab.empty())
  { C3valSlot &ep = tab.back();
    float dx = (e.x - ep.x);
    if (fabs(dx) > FLT_EPSILON)
    { e.du = (e.u - ep.u) / dx;
      e.dv = (e.v - ep.v) / dx;
      e.dw = (e.w - ep.w) / dx;
    }
  }
  tab.push_back(e);
  return;
}
//----------------------------------------------------------------------
// Load table from TUPPLE3 set.
//  Last entry must have x=y= -1;
//  Not used until now
//---------------------------------------------------------------------
void CmvalMap::Load(TUPPLE3 *tp)
{ char go = 1;
  char nt = 0;
  while (go)
  { TUPPLE3 *t = tp + (nt++);
    if ((t->inp == -1) && (t->out1 == -1)) return;
    C3valSlot e(t->inp,t->out1,t->out2,t->out3);
    Enter(e);
  }
  return;
}
//-----------------------------------------------------------------
//  Fill response with slot
//-----------------------------------------------------------------
void CmvalMap::Fill(TUPPLE3 &in,C3valSlot &s)
{ in.out1 = s.u;
  in.out2 = s.v;
  in.out3 = s.w;
  return;
}
//-----------------------------------------------------------------
//  Look value in table and interpolate result
//  Return result in the tupple
//-----------------------------------------------------------------
void CmvalMap::Lookup(TUPPLE3 &in)
{ in.out1 = 0;
  in.out2 = 0;
	in.out3 = 0;
  float x = in.inp;
  if (tab.empty())  return;
  // Get references to first and last elements
  C3valSlot &front = tab.front();
  C3valSlot &back  = tab.back();
  //---- Check for x-value below minimum breakpoint
  if (x <= front.x) return Fill(in,front);
  if (x >= back.x)  return Fill(in,back);
  //---- x-value is within table, need to find bracketing entries and interpolate
  float x1,u1,v1,w1;
 // float x2,y2,z2,dy,dz;//
  std::vector<C3valSlot>::iterator i = tab.begin();
  x1 = 0;
  u1 = 0;
  v1 = 0;
  w1 = 0;
  ps = &(*i);
  i++;
  //---Compare x to new x entry ------------------
  while ((i != tab.end()) && (x > ps->x)) {
        // Transfer upper bound to lower bound
        x1 = ps->x;
        u1 = ps->u;
        v1 = ps->v;
        w1 = ps->w;
        // Set new upper bound
        ps = &(*i);
        // Iterate to next element in the map
        i++;
      }
  // Interpolate between the two values
  float dx = (x - x1);
  in.out1 = u1 + (ps->du * dx);
  in.out2 = v1 + (ps->dv * dx);
  in.out3 = w1 + (ps->dw * dx);
  return;
}
//-----------------------------------------------------------------
//  Locate slot where x is lower than requested value
//  Return result in the tupple
//-----------------------------------------------------------------
C3valSlot *CmvalMap::Getfloor(float x)
{ if (tab.empty())  return 0;
  // Get references to first and last elements
  C3valSlot &front = tab.front();
  C3valSlot &back  = tab.back();
  //---- Check for x-value below minimum breakpoint
  if (x <= front.x) return &front;
  if (x >= back.x)  return &back;
  //---- x-value is within table, need to find entry ---
  std::vector<C3valSlot>::iterator i = tab.begin();
  ps    = 0;
  //---Compare x to new x entry ------------------
  while ((i != tab.end()) && (x >= (*i).x)) 
  {     ps = &(*i);
        // Iterate to next element in the map
        i++;
      }
  return ps;
}

//---------------------------------------------------------------------
//  return previous value of X
//---------------------------------------------------------------------
void CmvalMap::GetPrevX(U_INT k, float &x)
  { if (0 == k)           return;
    if (k >= tab.size())  return;
    C3valSlot *e = &tab[k - 1];
    x = e->x;
    return;
}
//---------------------------------------------------------------------
//  return Next value of X
//---------------------------------------------------------------------
void CmvalMap::GetNextX(U_INT k, float &x)
  { U_INT end = tab.size() - 1;
    if (k >= end)  return;
    C3valSlot *e = &tab[k + 1];
    x = e->x;
    return;
}
//---------------------------------------------------------------------
//  
//  must recompute everything
//---------------------------------------------------------------------
void CmvalMap::Recompute(U_INT k)
{ C3valSlot *e = &tab[k];
  if (0 < k)
  { //--Update from previous entry -----------------
    C3valSlot *ep = &tab[k-1];      // Previous slot
    float dx = (e->x - ep->x);
    if (fabs(dx)> FLT_EPSILON)
    { e->du = (e->u - ep->u) / dx;
      e->dv = (e->v - ep->v) / dx;
      e->dw = (e->w - ep->w) / dx;
    }
  }
  if (k < (tab.size()-1))
  { //--Update next from current entry -------------
    C3valSlot *en = &tab[k+1];      // Previous slot
    float dx = (en->x - e->x);
    if (fabs(dx)> FLT_EPSILON)
    { en->du = (en->u - e->u) / dx;
      en->dv = (en->v - e->v) / dx;
      en->dw = (en->w - e->w) / dx;
    }
  }
  return;
}
//---------------------------------------------------------------------
//  Modify X value
//  must recompute everything
//---------------------------------------------------------------------
void CmvalMap::ChangeX(U_INT k, float x)
{ C3valSlot *e = &tab[k];
  e->x  = x;
  return Recompute(k);
}
//---------------------------------------------------------------------
//  Modify U value
//  must recompute dy
//---------------------------------------------------------------------
void CmvalMap::ChangeU(U_INT k, float u)
{ C3valSlot *e = &tab[k];
  e->u  = u;
  return Recompute(k);;
}
//---------------------------------------------------------------------
//  Modify V value
//  must recompute dy
//---------------------------------------------------------------------
void CmvalMap::ChangeV(U_INT k, float v)
{ C3valSlot *e = &tab[k];
  e->v  = v;
  return Recompute(k);;
}
//---------------------------------------------------------------------
//  Modify W value
//  must recompute dy
//---------------------------------------------------------------------
void CmvalMap::ChangeW(U_INT k, float w)
{ C3valSlot *e = &tab[k];
  e->w  = w;
  return Recompute(k);;
}
//==============================================================================
//  CDataSearch is just supplying the read method
//==============================================================================
//  Just read the parameters
//---------------------------------------------------------------------
CDataSearch::CDataSearch(SStream *s)
{ table = 0;
  ReadFrom(this,s);
}
//---------------------------------------------------------------------
//  Read the parameters
//---------------------------------------------------------------------
int CDataSearch::Read (SStream *s, Tag tag)
{ switch (tag) {
    case 'fmt1':
      { //--- Format 1 table, simple mapping of x to y values with linear interpolation
        CFmt1Map *map = new CFmt1Map();
        table = map;
        map->DecodeFMT1(s);
        return TAG_READ;
      }
    case 'fmt3':
      { //--- Format 3 table of polynomial expressions-------
        CFmt3Map *map = new CFmt3Map();
        table   = map;
        map->DecodeFMT3(s);
        return TAG_READ;
      }

    case 'ply1':
      { //--- Polynomial a + bx + cx^2 + dx^3 ----------------
        SAFE_DELETE(table);
        CPolyMap *map = new CPolyMap(s);
        table = map;
        return TAG_READ;
      }
    case 'tbl1':
      { //---- May be fmt1 or fmt3 ---------------------------
        SAFE_DELETE(table);
        CTbl1Map  tbl(s);
        table = tbl.GetMap();
        return TAG_READ;
      }
    case 'cof1':
      { //--- Just a single coefficient ---------------------
        SAFE_DELETE(table);
        CCoefMap *map = new CCoefMap(s);
        table = map;
        return TAG_READ;
      }

  }
  char ed[8];
  TagToString(ed,tag);
  gtfo ("CDataSearch::Read : Unknown tag %s in %s", ed, s->filename);
  return TAG_IGNORED;
}
//==============================================================================
// Data map for a table in fmt1
// The table mapping is the most complex (and flexible) method.  There are two
//   basic subtypes of tables : <fmt1> and <fmt3>
//
// <fmt1> tables use a list of x,y data points to define the mapping curve.
//   Simple linear interpolation between the data points is used for x-values
//   that are not explicitly listed in the table data point list
///==============================================================================
CFmt1Map::CFmt1Map()
{}
//--------------------------------------------------------------
//  Free the map table
//--------------------------------------------------------------
CFmt1Map::~CFmt1Map()
{	tab.clear();	}
//---------------------------------------------------------------------
//  Decode FMT1 table
//---------------------------------------------------------------------
void CFmt1Map::DecodeFMT1(SStream *s)
{ char pm[256];
  ReadString (pm, 256, s);
// TRACE("====TABLE FMT1 =============================");
  while (strstr (pm, "<endf>") == 0)
  {   // Parse an x y data pair
      CFmt1Slot e;
			if (*pm)	
			{	if (sscanf (pm, "%f %f", &e.x, &e.y) == 2) Enter(e);
				else
				if (sscanf (pm, "%f,%f", &e.x, &e.y) == 2) Enter(e);
				else
				if (strncmp(pm,"<fmt1>",6) != 0)				// Stop processing
					gtfo ("CFmt1Map: Invalid <fmt1> data %s in %s", s, s->filename);
			}
      ReadString (pm, 256, s);
    }
  return;
}
//--------------------------------------------------------------
//  Add a new entry
//  NOTE:  For each entry we compute the dv value which is the
//  proportional delta(y) for the x intervalle with the previous
//  entry:     dv = (y - yp) / x - xp)
//  where x, y  in current entry
//        xp, yp from previous entry
//--------------------------------------------------------------
void CFmt1Map::Enter(CFmt1Slot &e)
{ if (!tab.empty())
  { CFmt1Slot &ep = tab.back();
    float dx = (e.x - ep.x);
    if (fabs(dx) > FLT_EPSILON) e.dv = (e.y - ep.y) / dx;
  }
// TRACE("--x=%.05f y=%.05f dv=%.05f",e.x,e.y,e.dv);
  tab.push_back(e);
  return;
}
//--------------------------------------------------------------
//  Add a new entry
//--------------------------------------------------------------
void CFmt1Map::Add(float x, float y)
{ CFmt1Slot slot(x,y);
  Enter(slot);
  return;
}
//---------------------------------------------------------------------
//  Load FMT1 table from a set of tupples
//  NOTE:  The set is terminated when both values are -1
//---------------------------------------------------------------------
void  CFmt1Map::Load(TUPPLE *tab)
{ char go = 1;
  char nt = 0;
  while (go)
  { TUPPLE *t = tab + (nt++);
    if ((t->inp == -1) && (t->out == -1)) return;
    Add(t->inp,t->out);
  }
  return;
}
//---------------------------------------------------------------------
//  Load FMT1 table from a set of tupples
//  NOTE:  The set is terminated when both values are -1
//---------------------------------------------------------------------
void  CFmt1Map::Load(PAIRF *tab)
{ char go = 1;
  char nt = 0;
  while (go)
  { PAIRF *t = tab + (nt++);
    if ((t->inp == -1) && (t->out == -1)) return;
    Add(t->inp,t->out);
  }
  return;
}

//-----------------------------------------------------------------
//  Look value in table and interpolate result
//-----------------------------------------------------------------
float CFmt1Map::Lookup(float x)
{
  float y = 0.0f;

  if (tab.empty())  return 0;
  // Get references to first and last elements
  CFmt1Slot &front = tab.front();
  CFmt1Slot &back  = tab.back();
  //---- Check for x-value below minimum breakpoint
  if (x <= front.x) return  front.y;
  if (x >= back.x)  return  back.y;
  //---- x-value is within table, need to find bracketing entries and iterpolate
  float x1, y1,x2,y2, dv;
  std::vector<CFmt1Slot>::iterator i = tab.begin();
  x1 = 0;
  y1 = 0;
  x2 = i->x;
  y2 = i->y;
  dv = i->dv;
  i++;
  //---Compare x to new x entry ------------------
  while ((i != tab.end()) && (x > x2)) {
        // Transfer upper bound to lower bound
        x1 = x2;
        y1 = y2;
        // Set new upper bound
        x2 = i->x;
        y2 = i->y;
        dv = i->dv;
        // Iterate to next element in the map
        i++;
      }
  // Interpolate between the two values
  y = y1 + (dv *(x - x1));
  return y;
}
//==============================================================================
// Data map for a table in fmt3
// The table mapping is the most complex (and flexible) method.  There are two
//   basic subtypes of tables : <fmt1> and <fmt3>
//
// <fmt3> tables use a list of x-values and list of up to four polynomials
//   that define a cubic polynomial to be applied at that x-value until the
//   next breakpoint.
///==============================================================================
CFmt3Map::CFmt3Map()
{}
//--------------------------------------------------------------
//  Free the map table
//--------------------------------------------------------------
CFmt3Map::~CFmt3Map()
{ tab.clear();
}
//--------------------------------------------------------------
//  Add a new entry
//--------------------------------------------------------------
void CFmt3Map::Enter(CFmt3Slot &e)
{ tab.push_back(e);
  return;
}
//---------------------------------------------------------------------
//  Decode FMT3 table
//---------------------------------------------------------------------
void CFmt3Map::DecodeFMT3(SStream *s)
{ char pm[256];
  ReadString (pm, 256, s);
  // TRACE("====TABLE FMT3 =============================");
  while (strstr (pm, "<endf>") == 0)
  { CFmt3Slot entry;
    // Parse breakpoint x-value and polynomial coefficients
    char *delimiters = " \t";
    char *token = strtok (pm, delimiters);
    entry.x = (float)atof (token);
    token = strtok (NULL, delimiters);
    char k = 0;
    while ((token) && (k < 10))
    { float f = (float)atof (token);
      entry.poly.SetCoefficient (k++, f);
      token = strtok (NULL, delimiters);
    }
    Enter(entry);
    // Read next line
    ReadString (pm, 256, s);
    }
  return;
}
//-----------------------------------------------------------------------
//  Looup for x value
//-----------------------------------------------------------------------
float CFmt3Map::Lookup(float x)
{ // Each table entry in a fmt3 table is a set of up to ten polynomial
  //   equation coefficients a0, a1, ... a9.  The mapping function at
  //   each breakpoint is defined as:
  //     y = a0 + (a1 * x) + (a2 * x^2) + ... + (a9 * x^9)
  //
  if (tab.empty()) return 0;

  // Get references to first and last elements
  CFmt3Slot &front = tab.front();
  CFmt3Slot &back  = tab.back();
  // Lookup value is less than minimum breakpoint
  if (x <= front.x) return front.poly.Lookup(x);
  // Lookup value is greater than maximum breakpoint
  if (x >= back.x)  return back.poly.Lookup (x);
  // x-value is within table, need to find bracketing entries and iterpolate
  float x1, x2;
  CSlotPolynomial *p1, *p2;
  std::vector<CFmt3Slot>::iterator i = tab.begin();
  x1 = 0;
  p1 = 0;
  x2 = (*i).x;
  p2 = &((*i).poly);
  i++;
  while ((i != tab.end()) && (x > x2)) {
    // Transfer upper bound to lower bound
    x1 = x2;
    p1 = p2;
    // Set new upper bound
    x2 = (*i).x;
    p2 = &((*i).poly);
    // Iterate to next element in the map
    i++;
    }
  // tmp : no interpolation
  i--;
  float y = (*i).poly.Lookup (x);
  return y;
}
//================END OF FILE ====================================================
