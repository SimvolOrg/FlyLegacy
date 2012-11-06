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


#include "../Include/Airport.h"
//============================================================================
#define LEFT_NODE(X)  (X >> 16)						// Left part
#define RITE_NODE(X)  (X & 0x0000FFFF)		// Rigth part
//============================================================================
#define TAXI_NODE_LIMT			(0x60)						// runway Limit
#define TAXI_NODE_EXIT			(0x41)						// runway exit
#define TAXI_NODE_TKOF			(0x62)						// runway take-off
#define TAXI_NODE_TAXI			(0x03)						// Taxiway node
#define TAXI_NODE_PARK			(0x04)						// Parking node
//----------------------------------------------------------------------
#define TAXI_NODE_DIR				(0x10)						// Direction 0=hi 1=lo
#define TAXI_NODE_FIXE			(0x20)						// Node is fixed
#define TAXI_NODE_AXES			(0x40)						// Node in runway axis
//----------------------------------------------------------------------
#define TAXI_HI_TKOF        (TAXI_NODE_TKOF)
#define TAXI_LO_TKOF				(TAXI_NODE_TKOF + TAXI_NODE_DIR)
#define TAXI_HI_EXIT				(TAXI_NODE_EXIT)
#define TAXI_LO_EXIT				(TAXI_NODE_EXIT + TAXI_NODE_DIR)
//============================================================================
//	Structure to undo operation
//============================================================================
struct TAXI_OP {
	Tag			oper;							// Node operand
	U_INT		iden;							// Node ident
};
//============================================================================
//  Node structure for runway and taxiways
//	NOTE:  Sector is defined relative to landing point (see LND_DATA)
//				From this point, every 100 feet counts one sector.
//============================================================================
class TaxNODE: public CStreamObject 
{ friend class TaxiTracker;
	friend class TaxiCircuit;
	friend class TaxiwayMGR;
	friend class SqlMGR;
	//---------------------------------------------------------------
protected:
	Tag						idn;												// Node ident
  SPosition     Pos;                        // world position
	char          rwy[4];											// Runway ident
	U_CHAR				type;												// Type of node
	//--- Edge counter (one per circuit)	---------------------------
	U_SHORT				ninp[2];										// Number of input edges
	U_SHORT				nout[2];										// Number of output edges
	U_SHORT				sector;											// Runway sector
	//--- Reference to landing point --------------------------------
	LND_DATA     *lndR;												// Landing reference
  //-----------Methods --------------------------------------------
public:
   TaxNODE();
	 TaxNODE(Tag id,TaxNODE *N);
	 TaxNODE(Tag id,SPosition *P);
  ~TaxNODE();
		//int			Read (SStream *stream, Tag tag);    // Read method
	  //----------------------------------------------------------------
		void		Init(Tag id, SPosition *P);
		void		SetPosition(SPosition &P);
		void		SwapRunway();
		bool    IsOutOK();
		bool		IsInpOK();
		//-----------------------------------------------------------------
		bool		LndParking();				// Check for landing parking
		bool		TkoParking();				// Check for take-off parking
		void		SetNodeType(U_CHAR dir);
		//----------------------------------------------------------------
		//void		Save(CStreamFile &sf);
		//----------------------------------------------------------------
		SPosition *AtPosition()						{return &Pos;}
		SPosition &Position()							{return Pos;}
		//----------------------------------------------------------------
		void		SetRWY(char *R)			{strncpy(rwy,R,4);	}
		void    SetTYP(U_CHAR T)		{type = T;}
		void		SetREF(LND_DATA *D)	{lndR = D;}
		//-----------------------------------------------------------------
		bool		IsNotRWY(char *r)		{return (strncmp(r,rwy,4) != 0);}
		bool		IsNotTKO()					{return ((type & 0x67) != TAXI_NODE_TKOF);}
		bool		IsNotExit()					{return ((type & 0x67) != TAXI_NODE_EXIT);}
		//-----------------------------------------------------------------
		bool		NoEdge(char t)			{return (nout[t] == 0) && (ninp[t] == 0);}
		bool		IsPath(char t)			{return (nout[t] == 1);}
		bool    EndPath(char t)			{return (nout[t] != 1);}
		bool		NoOutput(char t)		{return (nout[t] == 0);}
		bool    NoInput(char t)			{return (ninp[t] == 0);}
		//-----------------------------------------------------------------
		U_CHAR	GetDirection()			{return (type & TAXI_NODE_DIR);}
		Tag			GetIdent()					{return idn;}
		bool		HasIdent(Tag T)			{return (idn != T);}
		//-----------------------------------------------------------------
		bool		IsType(char T)			{return (T == type);}
		bool    NotType(char T)			{return ((T & type) != T);}
		bool		HasType(char T)			{return ((T & type) == T);}
		bool    IsLndNode()				  {return ((type & 0x67) == TAXI_NODE_EXIT);}
		bool		IsTkoNode()					{return ((type & 0x67) == TAXI_NODE_TKOF);}
		//----------------------------------------------------------------
		void		IncINP(char t);
		void    IncOUT(char t);			
		void		DecINP(char t);			
		void		DecOUT(char t);			
		//-----------------------------------------------------------------
		void		SetDirection(U_CHAR d)	{type |= (d << 4);}
		void		RazINP()	{ninp[0] = ninp[1] = 0;}
		void    RazOUT()  {nout[0] = nout[1] = 0;}
};
//============================================================================
//  Edge structure for runway and taxiways
//============================================================================
class TaxEDGE: public CStreamObject
{ //------------Attribute ----------------------------------------
public:
	Tag						idn;													// Identity
	Tag						scut;													// Short cut
  //-----------Methods ------------------------------------------
public:
   TaxEDGE();
	 TaxEDGE(U_SHORT n1, U_SHORT n2);
  ~TaxEDGE();
	 //--------------------------------------------------------------
	 //int			Read (SStream *stream, Tag tag);    // Read method
	 bool			ReferTo(Tag tn);
	 void			SetShortCut(Tag C);
	 bool			NoCandidate(Tag A);
   //-----------innline ------------------------------------------
	 //void			Save(CStreamFile &sf);
	 //-------------------------------------------------------------
	 void			StoreShortCut(Tag S)	{scut = S;}
};
//============================================================================
//  Class CDataBGR for Taxiway line description
//============================================================================
class CDataBGR: public CStreamObject
{ //-------------Attributes ---------------------------------------------
	CAptObject		 *apo;																// Airport object
  SPosition       pos;                                // File Origin
  SPosition       dpo;																// Polygon origin
  std::vector<TaxNODE *>  nodelist;                 // List of nodes
  std::vector<TaxEDGE *>  edgelist;                 // List of edges
  //-------------Scale factor -------------------------------------
  float           scale;
  //------------ Color --------------------------------------------
  U_INT           white;
  //------------- Methods -----------------------------------------
public:
   CDataBGR(CAptObject *apo);
  ~CDataBGR();
  void        EmptyAll();
	void				AdjustOrigin();
  int         Read (SStream *stream, Tag tag);    // Read method
	void				ProcessNode(SStream *sf);
	//---------------------------------------------------------------
  TaxNODE  *GetNode(U_INT No);
  TaxEDGE  *GetEdge(U_INT No);
  bool        GetLine(U_INT No,int &x0,int &y0,int &x1,int &y1);
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
