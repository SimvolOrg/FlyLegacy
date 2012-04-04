//==============================================================================================
// RoofModel.h
//
// Part of Fly! Legacy project
//
// Copyright 2003-2005 Chris Wallace
// Copyright 2007-2009 Jean Sabatier
//
// Fly! Legacy is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
// Fly! Legacy is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
//   along with Fly! Legacy; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//==============================================================================================

#ifndef ROOF_MODEL_H
#define ROOF_MODEL_H
#include "../Include/Globals.h"
#include "../Include/Triangulator.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//==============================================================================================
//	Structure for model parameters
//==============================================================================================
struct D2_ROOF_PM {
		char *name;					// Model name
		char *desc;					// Description
		U_INT type;					// Roof type
		int		nbv;					// Number of vertices
		int		nbi;					// Number of indices
		D2_INDICE *lind;		// List of indices
		double  a;					// Angle
};
//==============================================================================================
//====================================================================================
CRoofModel *FindRoofParameters(char *name, double a);
//====================================================================================
//	Roof Model
//====================================================================================
class CRoofModel: public D2_Ratio {
	friend class Queue<CRoofModel>;
	//--- ATTRIBUTES -----------------------------------------------
protected:
	//--- Identifier -------------------------------------------
	U_INT				ident;
	//----------------------------------------------------------
	CRoofModel *next;
	CRoofModel *prev;
	//--- Roof characteristics ---------------------------------
	double      slope;											// half Angle from vertical
	//----Roof must be cleared ---------------------------------
	char				rmbc;												// Clear indicator
	char        rmno;												// Model number
	//--- Model parameters -------------------------------------
	short				nbv;												// Number of vertices
	short				nbx;												// Number of index
	D2_INDICE  *aIND;												// Indices
	D2_POINT   *bevel;											// Bevel array
	D2_BEVEL    pm;													// Bevel parameters
	//--- Roof ratio ----------------------------------------------
	double    ratio;												// top ratio relative to width
	double		top;													// Highest point in roof
	double    lgx;													// X side length
	double		lgy;													// Y side length
	double    afh;													// Roof height
	//--- Array of S-T texture coordinate  for roof ----------------
	D2_TParam *roofP;
	D2_Style  *style;											  // Building style
	//--- Total POINTs in array -----------------------------------
	U_INT			tot;													// Total
	//--- Building parameters -------------------------------------
	D2_BPM   *bpm;
	//--- Triangulator --------------------------------------------
	Triangulator *trn;											// Pointer to
	GeoTest  *geo;													// Geo tester
	//---METHODS --------------------------------------------------
public:
	CRoofModel(D2_ROOF_PM &pm);
	CRoofModel()	{nbv = nbx = 0; aIND = 0; rmno = 0;}
 ~CRoofModel();
	void		Release();
	//-------------------------------------------------------------
	void	Translate(U_INT from, U_INT to, char *id, double X, double y, double z);
	void	SetCorner(U_INT sw, U_INT se, U_INT ne, U_INT nw);
  //--- Fill Triangles ------------------------------------------
	int 	FillTextureCoordinates(D2_TRIANGLE &T, short n,short m);
	//---Default is a 2 slopes roof -------------------------------
	virtual void	BuildRoof(Queue <D2_POINT> &inp,    std::vector<D2_TRIANGLE*> &out);
	virtual void	GenerateRoof(Queue <D2_POINT> &inp, std::vector<D2_TRIANGLE*> &out) {;}
	//-------------------------------------------------------------
	double  GetTop()	{	return top;}
	U_INT		GetIdent(){	return ident;}
	//-------------------------------------------------------------
	void	StoreBase(Queue <D2_POINT> &inp, double H);
	void	SetRoofData(D2_BPM *pm, Triangulator *tr);
	void	GenerateTriangles(std::vector<D2_TRIANGLE*> &out);
	//--------------------------------------------------------------
	void	SetRoofModNumber(U_CHAR n)		{rmno	= n;}
	U_INT GetRoofModNumber()						{return rmno;}
	char	GetClearIndicator()						{return rmbc;}
};
//====================================================================================
//	Roof Model with flat top
//====================================================================================
class  CRoofFLAT : public CRoofModel
{	//--- Attributes -------------------------
	//--- Methods ----------------------------
public:
	CRoofFLAT(D2_ROOF_PM &pm);
	CRoofFLAT(U_INT id);
	void	BuildRoof(Queue <D2_POINT> &inp,    std::vector<D2_TRIANGLE*> &out);
	void  GenerateRoof(Queue <D2_POINT> &inp, std::vector<D2_TRIANGLE*> &out);
	//----------------------------------------
};
//====================================================================================
//	Roof Model with 2 slopes and dormer top
//====================================================================================
class  CRoofDORM : public CRoofModel
{	//--- Attributes -------------------------
	//--- Methods ----------------------------
public:
	CRoofDORM(D2_ROOF_PM &pm);
	void  GenerateRoof(Queue <D2_POINT> &inp, std::vector<D2_TRIANGLE*> &out);
	//----------------------------------------
};

//====================================================================================
//	Roof Model with 1 slope
//====================================================================================
class  CRoofM1P : public CRoofModel
{	//--- Attributes -------------------------
	//--- Methods ----------------------------
public:
	CRoofM1P(D2_ROOF_PM &pm);
	void      GenerateRoof(Queue <D2_POINT> &inp, std::vector<D2_TRIANGLE*> &out);
	//----------------------------------------
};

//====================================================================================
//	Roof Model with 2 slopes
//====================================================================================
class  CRoofM2P : public CRoofModel
{	//--- Attributes -------------------------
	//--- Methods ----------------------------
public:
	CRoofM2P(D2_ROOF_PM &pm);
	void      GenerateRoof(Queue <D2_POINT> &inp, std::vector<D2_TRIANGLE*> &out);
};
//====================================================================================
//	Roof Model with 4 slopes
//====================================================================================
class  CRoofM4P : public CRoofModel
{	//--- Attributes -------------------------
	//--- Methods ----------------------------
public:
	CRoofM4P(D2_ROOF_PM &pm);
	void      GenerateRoof(Queue <D2_POINT> &inp, std::vector<D2_TRIANGLE*> &out);
	void			Texturing(Triangulator *,std::vector<D2_TRIANGLE*> &out);
};
//====================================================================================
//	Roof Model with 6 slopes
//====================================================================================
class  CRoofM6P : public CRoofModel
{	//--- Attributes -------------------------
	//--- Methods ----------------------------
public:
	CRoofM6P(D2_ROOF_PM &pm);
	void      GenerateRoof(Queue <D2_POINT> &inp, std::vector<D2_TRIANGLE*> &out);
	void			Texturing(Triangulator *,std::vector<D2_TRIANGLE*> &out);
};

//==============================================================================================
#endif // ROOF_MODEL_H
