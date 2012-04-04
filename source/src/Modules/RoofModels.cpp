//=================================================================================
// RoofModel.cpp
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
//====================================================================================
#include "../Include/RoofModels.h"
using namespace std;
//===================================================================================
//==========================================================================================
//  Roof model N°1  2 SLOPES
//										E---------------F			height = 1 A= origin (0,0,0)
//									*		*						*		*
//								D-------A      C--------B
//	Positive base is ABCD with AB the longuest edge
//==========================================================================================
//==========================================================================================
//		Indices for model 1 SLOPE
//==========================================================================================
D2_INDICE xP1[]=
{ //------ Side wall ------------------------
	{3,0,4, PAN_WALL_YM, TX_BH	},		// T(DAE)	Face Y-
	{1,2,5, PAN_WALL_YP, TX_BH	},		// T(BCF)	Face Y+
	//-------Roof Pan (X+)---------------------
	{4,0,1, PAN_ROOF_XP, 0,			},			// T(EAB)
	{4,1,5, PAN_ROOF_XP, 0,			},		// T(EBF)
	//------ Wall Pan (X-) ----------------------
	{5,2,3, PAN_WALL_XM, TX_BH,	},		// T(FCD)
	{5,3,4, PAN_WALL_XM, TX_BH,	},		// T(FDE)
	};
//CRoofM1P	roofM1P30(6,sizeof(xP1)/sizeof(D2_INDICE),xP1,30);
//==========================================================================================
//		Indices for model 2 SLOPEs
//==========================================================================================
D2_INDICE xP2[]=
{	//----- Side Wall -------------------------
	{3,0,4,  PAN_WALL_YM, TX_BH	},			// T(DAE)	Face Y-
	{1,2,5,  PAN_WALL_YP,	TX_BH	},			// T(BCF)	Face Y+
	//-------Roof Pan--(X+)-------------------
	{4,0,1,  PAN_ROOF_XP,	0			},			// T(EAB)
	{4,1,5,  PAN_ROOF_XP, 0			},			// T(EBF)
	//-------Roof Pan  (X-) -------------------
	{5,2,3,  PAN_ROOF_XM,	0			},			// T(FCD)
	{5,3,4,  PAN_ROOF_XM,	0			},			// T(FDE)
	};
//------Instance for model 1 -----------------------------
//CRoofM2P	roofM2P30(6,sizeof(xP2)/sizeof(D2_INDICE),xP2,30);
//CRoofM2P	roofM2P45(6,sizeof(xP2)/sizeof(D2_INDICE),xP2,45);
//CRoofM2P	roofM2P60(6,sizeof(xP2)/sizeof(D2_INDICE),xP2,60);
//==========================================================================================
//		Indices for model 4 SLOPES-
//==========================================================================================
D2_INDICE xP4[]=
{	//------- Side roof ----------------------
	{3,0,4,	PAN_ROOF_YM,	0,	},		// T(DAE)					
	{1,2,5,	PAN_ROOF_YP,	0,	},		// T(BCF)					
	//-------Roof Pan (X+) ---------------------
	{4,0,1, PAN_ROOF_XP,	0,	},		// T(EAB)
	{4,1,5, PAN_ROOF_XP,	0,	},		// T(EBF)
	//-------Roof Pan (X-) ----------------------
	{5,2,3, PAN_ROOF_XM,	0,	},		// T(FCD)
	{5,3,4, PAN_ROOF_XM,	0,	},		// T(FDE)
	};
//--------------------------------------------------------------
//CRoofM4P  roofM4P45(6,sizeof(xP4)/sizeof(D2_INDICE),xP4,50);
//==========================================================================================
//		Indices for model 6 SLOPES-
//										J(8)------------------(9)K			height = 1 A= origin (0,0,0)
//									*		*						      *		*
//								H(7)----E(4)         G(6)-----F(5)
//							*						*		     * 						*
//						D(3)------------A(0)  C(2)-------------B(1)
//	Positive base is ABCD with AB the longuest edge
//==========================================================================================
//==========================================================================================
D2_INDICE xP6[]=
{	//------- Side roof ----------------------------------
	//------- Side walls YM------------------------------
	{8,7,4,	PAN_ROOF_YM,	0,		},				// T(JHE)					// Face Y-
	{7,3,0, PAN_WALL_YM,  TX_BH,},				// T(HDA)
	{7,0,4, PAN_WALL_YM,  TX_BH,},				// T(HAE)
	//------ Side wall YP --------------------------------
	{9,5,6,	PAN_ROOF_YP,	0,		},				// T(KFG)					// Face Y+
	{5,1,2, PAN_WALL_YP,  TX_BH,},				// T(FBC)
	{5,2,6, PAN_WALL_YP,  TX_BH,},				// TFBG)
	//-------Roof Pan (X+) -------------------------------
	{4,0,1, PAN_ROOF_XP,	0,		},				// T(EAB)
	{4,1,5, PAN_ROOF_XP,	0,		},				// T(EBF)
	{8,4,5,	PAN_ROOF_XP,	0,		},				// T(JEF)
	{8,5,9, PAN_ROOF_XP,  0,		},				// T(JFK)
	//-------Roof Pan (X-) ----------------------
	{6,2,3, PAN_ROOF_XM,	0,		},				// T(GCD)
	{6,3,7, PAN_ROOF_XM,	0,		},				// T(GDH)
	{9,6,7, PAN_ROOF_XM,  0,		},				// T(KGH)
	{9,7,8, PAN_ROOF_XM,	0,		},				// T(KHJ)
	};
//CRoofM6P  roofM6P60(10,sizeof(xP6)/sizeof(D2_INDICE),xP6,60);
//==========================================================================================
//		Indices for model 2 SLOPES + DORMER
//										E(4)----------G(6)------------J(8)--------------------(5)F			
//									*		*	            *     M(10)      *     N(11)           *		*
//								*       *	            *   |            *     |           *        *		
//							*						*		          * |               *  |         * 						*
//						D(3)------------A(0)          * H(7)             *K(9)   C(2)-------------B(1)
//	Positive base is ABCD with AB the longuest edge
//==========================================================================================
D2_INDICE x2D[]=
{	//------- Side roof ----------------------------------
	//------- Side walls -----------------------------------
	{4, 3, 0,		PAN_WALL_YM,	TX_BH,		},		// T(EDA)		Face Y-
	{5, 1, 2,		PAN_WALL_YP,  TX_BH,		},		// T(FBC)		face Y+
	{4, 0, 7,   PAN_ROOF_XP,      0,		},		// T(EAH)		roof x+
	{4, 7, 6,   PAN_ROOF_XP,      0,    },		// T(EHG)		roof X+
	{8, 9, 1,   PAN_ROOF_XP,      0,    },    // T(JKB)		roof X+
	{8, 1, 5,   PAN_ROOF_XP,      0,    },    // T(JBF)		roof X+
	//-----------------------------------------------------------
	{5, 2, 3,   PAN_ROOF_XM,			0,		},		// T(FCD)		roof X-
	{5, 3, 4,   PAN_ROOF_XM,			0,		},		// T(FDE)		roof X-
	//--- Dormer ------------------------------------------------
	{6, 7, 10,  PAN_WALL_YM,  TX_BH,		},		// T(GHM)		side Y-
	{9, 8, 11,  PAN_WALL_YP,  TX_BH,    },		// T(KJN)		side Y+
	{10,7 , 9,	PAN_DORM_XP,      0,    },    // T(MHK)   dorm X+
	{10, 9,11,  PAN_DORM_XP,      0,    },		// T(MKN)   dorm X+
	{ 6,10,11,  PAN_DORM_SW,      0,    },    // T(GMN)		roof X+
	{ 6,11, 8,  PAN_DORM_NE,      0,    },		// T(GNJ)		roof X+
};

//====================================================================================
//	Roof models database 
//====================================================================================
D2_ROOF_PM roofDATA[] = {
	{	"FLAT",  "Flat roof",    'flat',},			// Must be the first entry
	{	"1SLOPE","1 slope  %.1f",'1slp',	6,		sizeof(xP1)/sizeof(D2_INDICE),xP1,},
	{	"2SLOPE","2 slopes %.1f",'2slp',	6,		sizeof(xP2)/sizeof(D2_INDICE),xP2,},
	{	"4SLOPE","4 slopes %1.f",'4slp',	6,		sizeof(xP4)/sizeof(D2_INDICE),xP4,},
	{	"6SLOPE","6 slopes %1.f",'6slp',	10,	  sizeof(xP6)/sizeof(D2_INDICE),xP6,},
	{ "2SDORM","2 Slopes %1.f",'2dor',  12,   sizeof(xP6)/sizeof(D2_INDICE),x2D,},
	{	"END"},
};
//====================================================================================
//--GLOBAL FUNCTION
//	Locate roof model by name 
//====================================================================================
CRoofModel *FindRoofParameters(char *name, double a)
{	D2_ROOF_PM *pm = roofDATA;
	CRoofModel *md = 0;
	bool	go = true;
	while (go)
	{	pm->a = a;
		if (strcmp(pm->name,name ) == 0)	break;
		if (strcmp(pm->name,"END") == 0)	return 0;
		pm++;
	}
	//--- Make an instance of the roof model ----------
	switch (pm->type)	{
		case '1slp':
			md = new CRoofM1P(*pm);
			return md;
		case '2slp':
			md = new CRoofM2P(*pm);
			return md;
		case '4slp':
			md = new CRoofM4P(*pm);
			return md;
		case '6slp':
			md = new CRoofM6P(*pm);
			return md;
		case '2dor':
			md= new CRoofDORM(*pm);
			return md;
		case 'flat':
			md = new CRoofFLAT(*pm);
			return md;
	}
	return 0;
}

//====================================================================================
//	PROCEDURAL Roof Model
//	Those function build several kind of roof when the base contour of the building
//	is a rectangle
//====================================================================================
CRoofModel::CRoofModel(D2_ROOF_PM &pm)
{	ident = pm.type;
	next	= prev = 0;
	nbv		= pm.nbv;							//n;
	nbx		= pm.nbi;							//q;
	aIND	= pm.lind;						//x;
	ratio = 0.5;
	trn		= 0;
	slope = DegToRad(pm.a);
	rmbc	= 1;
	rmno	= 0;
}
//---------------------------------------------------------------
//	free vectors
//---------------------------------------------------------------
CRoofModel::~CRoofModel()
{	;}
//---------------------------------------------------------------
//	free roof model except the flat model that belongs to D2_Session
//---------------------------------------------------------------
void CRoofModel::Release()
{ if ('fixe' == ident) return;
	delete this;
}
//---------------------------------------------------------------
//	Set roof texture
//	Arrays are ordered as
//	a[0]	= SW corner
//	a[1]	= SE corner
//	a[2]	= NE corner
//	a[3]	= NW corner
//---------------------------------------------------------------
void CRoofModel::SetRoofData(D2_BPM *pm,Triangulator *tr)
{	trn		= tr;
	geo		= tr->GetGeotester();
	bpm		= pm;
	roofP	= bpm->roofP;
	style	= bpm->style;
	return;
}
//---------------------------------------------------------------
//	Translate point A to B using X,Y,Z as translation
//---------------------------------------------------------------
void CRoofModel::Translate(U_INT src, U_INT dst, char *id, double X, double Y, double Z)
{	bevel[dst] = bevel[src];
	if (id) bevel[dst].SetID(id);
	trn->TranslatePoint(bevel[dst],X,Y,Z);
	return;
}
//---------------------------------------------------------------
//	Set texture coordinates fro the 4 corners points
//---------------------------------------------------------------
void CRoofModel::SetCorner(U_INT sw, U_INT se, U_INT ne, U_INT nw)
{ bevel[sw].SetST(0,0);
	bevel[se].SetST(1,0);
	bevel[ne].SetST(1,1);
	bevel[nw].SetST(0,1);
	return;
}
//---------------------------------------------------------------
//	Fill a triangle from index k
//	aIND give index in the model
//	New POINTs are created for each triangles and stored in bevel
//
//		n is the current index into the triangle description (aIND)
//		m is the current index into  the POINT array
//
//	NOTE: Only the pointers are assigned to the out points
//---------------------------------------------------------------
int CRoofModel::FillTextureCoordinates(D2_TRIANGLE &T, short n, short m)
{	D2_TParam *prm;
	D2_POINT *sp;
	char pan = aIND[n].pt;						// Pan type
	char bth = aIND[n].bh;
	//--- Process B vertex ------
	short s0 = aIND[n].ind1;
	sp		= bevel + s0;
	bevel[m] = *sp;
  T.B   = bevel + m++;
	T.B->Stamp('T');
  //--- Process A vertex ------
	short s1 = aIND[n].ind2;
	sp		= bevel + s1;
	bevel[m]	= *sp;
	T.A   = bevel + m++;
	T.A->Stamp('T');
  //--- Process C vertex ------
	short s2 = aIND[n].ind3;
	sp		= bevel + s2;
	bevel[m] = *sp;
	T.C		= bevel + m++;
	T.C->Stamp('T');
	//------------------------------------------
	if (U_INT(m) > tot)	gtfo("Bad m index");
	//--- Process according to PAN type --------
	switch (pan)	{
		case PAN_ROOF_XP:
			roofP->TextureRoofTriangle(T);
			return m;
		case PAN_ROOF_XM:
			roofP->TextureRoofTriangle(T);
			return m;
		case PAN_ROOF_YP:
			roofP->TextureSideRoof(T);					// Texture Y+
			return m;
		case PAN_ROOF_YM:
			roofP->TextureSideRoof(T);					// Texture Y+
			return m;
		case PAN_WALL_XP:
			style->TextureSideWall(T,GEO_FACE_XP, bth);
			return m;
		case PAN_WALL_XM:
			roofP->TextureFrontTriangle(T);
			//style->TextureSideWall(T,GEO_FACE_XM, bth);
			return m;
		case PAN_WALL_YP:
			style->TextureSideWall(T,GEO_FACE_YP, bth);
			return m;
		case PAN_WALL_YM:
			style->TextureSideWall(T,GEO_FACE_YM, bth);
			return m;
		case PAN_DORM_SW:
			roofP->TextureDormerTriangle(T,PAN_DORM_SW);
			return m;
		case PAN_DORM_NE:
			roofP->TextureDormerTriangle(T,PAN_DORM_NE);
			return m;
		case PAN_DORM_XP:
			prm = style->GetDormer();
		  if (0 == prm) return m;
			prm->TextureFrontTriangle(T);
			return m;
		default:
			gtfo ("Bad RoofModel");
	}
	return m;
}
//---------------------------------------------------------------
//	Generate triangles
//----------------------------------------------------------------
void CRoofModel::GenerateTriangles(std::vector<D2_TRIANGLE*> &out)
{	//--- NOTE: First 2 triangles must be Y faces ---------------
	int m = nbv;								// 
	for (int n=0; n < nbx; n++)
	{	D2_TRIANGLE *T = new D2_TRIANGLE(1);
		m = FillTextureCoordinates(*T,n,m);
		T->N = geo->PlanNorme(*T->A, *T->B, *T->C);
		out.push_back(T);
	}
	return;
}
//---------------------------------------------------------------
//	A roof model must do the following things:
//
//	Compute roof triangles from the model and the building
//	tour given by 'inp'.  This is done by overwriting
//			the virtual function BuildRoof(..)
//	1) A list of POINT is allocated.  The number of points is given
//			by the model description.  The first points must be for
//			the base roof contour. Then , the others points are
//			for the roof triangles.
//	2) Then GenerateRoof() must add the top POINTs and generates
//			triangles for the roof.  The first triangles must be for
//			the wall faces.
//	3) Save those triangles in the output list 'out'
//	4) Return a list of vertices that made triangles computed
//		This list is then under the caller responsibility
//---------------------------------------------------------------
//	The list of POINT is organized as
//	-First 4 POINTS are the base points
//	-Next comes the top points
//	-Next come the POINTs needed by triangles with distinct
//	Texture coordinates.
//---------------------------------------------------------------
void	CRoofModel::BuildRoof(Queue <D2_POINT> &inp, std::vector<D2_TRIANGLE*> &out)
{	if (inp.GetNbObj() != 4)					return;
	//--- Save Y edge length --------------------------------
	D2_POINT *pa	= inp.GetFirst();
	D2_POINT *pb  = pa->next;
	lgx						= pa->elg;
	lgy						= pb->elg;
	afh						= pb->z;
	//-----Allocate array of POINTs -------------------------
	tot	= nbv + (3 * nbx);
	bevel					= trn->AllocateBevel(tot);
	GenerateRoof(inp,out);
	bevel					= 0;
	return;
}
//---------------------------------------------------------------
//	Save base points
//---------------------------------------------------------------
void	CRoofModel::StoreBase(Queue <D2_POINT> &inp, double H)
{	D2_POINT *pp;
	roofP->SetRoofBase(afh,H,lgy);
	for (pp = inp.GetFirst(); pp!= 0; pp = pp->next)
	{		bevel[pp->rng]		= *pp;
	}
	return;
}
//===================================================================================
//	Roof with 1 Slope
//===================================================================================
CRoofM1P::CRoofM1P(D2_ROOF_PM &pm)
	: CRoofModel(pm)
{}
//---------------------------------------------------------------
//	The model M2P builds a roof with 2 slopes
//	
//	NOTE:  The tour points (inp) must have good height
//				the inp should be ordered with X origin as 1rst point
//	The top of roof is computed according to the roof angle
//	For 45° roof  top is half the wall width
//
//	List of POINT is
//	A,B,C,D   for roof tour
//	E,F				for top Y- and Top Y+
//---------------------------------------------------------------
//	this model needs 4 tour points + 2 top points
//---------------------------------------------------------------
void  CRoofM1P::GenerateRoof(Queue <D2_POINT> &inp, std::vector<D2_TRIANGLE*> &out)
{	//--- Compute both roof top vertices from width--------------
	double tang = tan(slope);
	double H		= tang * lgy;
	//--- Build array of roof points-----------------------------
	StoreBase(inp,H);
	//-----------------------------------------------------------
	Translate(3,4,"rfpE",0,0,H);
	//--- Dupplicate in point 5 ---------------------------------
	Translate(2,5,"rfpF",0,0,H);
	//--- SetTexture coordinate ---------------------------------
	SetCorner(2,3,4,5);
	//--- Save the highest point elevation ----------------------
	top		= bevel[4].z;
	bpm->hgt		= top;			//style->SetBz(top);
	return GenerateTriangles(out);
}
//===================================================================================
//	Roof with 2 Slopes
//===================================================================================
CRoofM2P::CRoofM2P(D2_ROOF_PM &pm)
	: CRoofModel(pm)
{}
//---------------------------------------------------------------
//	The model M2P builds a roof with 2 slopes
//	
//	NOTE:  The tour points (inp) must have good height
//				the inp should be ordered with X origin as 1rst point
//	The top of roof is computed according to the roof angle
//	For 45° roof  top is half the wall width
//
//	List of POINT is
//	A,B,C,D   for roof tour
//	E,F				for top Y- and Top Y+
//---------------------------------------------------------------
void  CRoofM2P::GenerateRoof(Queue <D2_POINT> &inp, std::vector<D2_TRIANGLE*> &out)
{	//--- Compute both roof top vertices from width--------
	//--- Add the 2 top vertices into POINT array ---------------
	double tang = tan(slope);
	double H		= tang * 0.5 * lgy;
	double Y    = lgy  * 0.5;
	//--- Build array of roof points-----------------------------
	StoreBase(inp,H);
	//--- Add top points ---------------------------------------
	Translate(0,4,"rfpE",0,Y,H);
	Translate(1,5,"rfpF",0,Y,H);
	//--- Save the highest point elevation ----------------------
	top	= bevel[4].z;
	bpm->hgt	= top;			//style->SetBz(top);
	return GenerateTriangles(out);
}
//===================================================================================
//	Roof with 4 Slopes
//===================================================================================
CRoofM4P::CRoofM4P(D2_ROOF_PM &pm)
	: CRoofModel(pm)
{}
//---------------------------------------------------------------
//	The model M4P builds a roof with 4 slopes
//	
//	NOTE:  The tour points (inp) must have good height
//				the inp should be ordered with X origin as 1rst point
//	The top of roof is computed according to the roof angle
//	For 45° roof  top is half the wall width
//
//	List of POINT is
//	A,B,C,D   for roof tour
//	E,F				for top Y- and Top Y+
//---------------------------------------------------------------
void  CRoofM4P::GenerateRoof(Queue <D2_POINT> &inp, std::vector<D2_TRIANGLE*> &out)
{	//--- Compute both roof top vertices from width--------
	//--- Add the 2 top vertices into POINT array ---------------
	double tang = tan(slope);
	double H		= tang * 0.5 * lgy;							// Point height
	double X    = H * 0.5;
	double Y    = lgy * 0.5;
	//--- Build array of roof points-----------------------------
	StoreBase(inp,H);
	//-----------------------------------------------------------
	Translate(0,4,"rfpE",+X,Y,H);
	Translate(1,5,"rfpF",-X,Y,H);
	//-----------------------------------------------------------
	top	= bevel[4].z;
	bpm->hgt	= top;					//	style->SetBz(top);
	//--- Compute texture coordinates for top points ------------
	return GenerateTriangles(out);
}
//===================================================================================
//	Roof with 4 Slopes
//===================================================================================
CRoofM6P::CRoofM6P(D2_ROOF_PM &pm)
	: CRoofModel(pm)
{}
//==========================================================================================
//		Indices for model 6 SLOPES-
//										J(8)------------------(9)K			height = 1 A= origin (0,0,0)
//									*		*						      *		*
//								H(7)----E(4)         G(6)-----F(5)
//							*						*		     * 						*
//						D(3)------------A(0)  C(2)-------------B(1)
//	Positive base is ABCD with AB the longuest edge
//==========================================================================================
void  CRoofM6P::GenerateRoof(Queue <D2_POINT> &inp, std::vector<D2_TRIANGLE*> &out)
{	//--- Compute both roof top vertices from width--------
	//--- Add the 2 top vertices into POINT array ---------------
	double tang = tan(slope);
	double H		= tang * 0.5 * lgy;							// Top height
	double M    = H  * 0.5;											// Mid height
	double X    = M  / tang;										// mid withdraw
	double Q    = lgy * 0.25;										// mid side
	double Y    = lgy * 0.5;
	//--- Build array of roof points-----------------------------
	StoreBase(inp,H);
	//----Build intermediates points-----------------------------
	Translate(0,4,"rfpE",0,+Q,M);
	//--------------------------------------
	Translate(1,5,"rfpF",0,+Q,M);
	//---------------------------------------
	Translate(2,6,"rfpG",0,-Q,M);
	//--------------------------------------
	Translate(3,7,"rfpH",0,-Q,M);
	//--- Build Top Points --------------------------------------
	Translate(0,8,"rfpJ",+X,+Y,H);
	Translate(1,9,"rfpK",-X,+Y,H);
	//-----------------------------------------------------------
	top	= bevel[8].z;
	bpm->hgt	= top;			//style->SetBz(top);
	//--- Compute texture coordinates for top points ------------
	return GenerateTriangles(out);
}
//===================================================================================
//	Roof with dormer and 2 slopes
//===================================================================================
CRoofDORM::CRoofDORM(D2_ROOF_PM &pm)
	: CRoofModel(pm)
{}
//==========================================================================================
//		Indices for model 6 SLOPES-
//										E(4)----------G(6)------------J(8)--------------------(5)F			
//									*		*	            *     M(10)      *     N(11)           *		*
//								*       *	            *   |            *     |           *        *		
//							*						*		          * |               *  |         * 						*
//						D(3)------------A(0)          * H(7)             *K(9)   C(2)-------------B(1)
//	Positive base is ABCD with AB the longuest edge
//==========================================================================================
void  CRoofDORM::GenerateRoof(Queue <D2_POINT> &inp, std::vector<D2_TRIANGLE*> &out)
{	//--- Compute both roof top vertices from width--------
	//--- Add the 2 top vertices into POINT array ---------------
	double tang = tan(slope);
	double H		= tang * 0.5 * lgy;							// Top height
	double Y    = lgy * 0.5;
	double X    = lgx * 0.25;
	double Q    = H   * 0.50;
	//--- Build array of roof points-----------------------------
	StoreBase(inp,H);
	//--- Build top points E------------------------------------
	Translate(0,4,"rfpE", 0,+Y,H);
	//--- Build top points F------------------------------------
	Translate(1,5,"rfpF",0,+Y, H);
	//--- Build top points G------------------------------------
	Translate(4,6,"rfpG",+X, 0, 0);
	//--- Build side points H------------------------------------
	Translate(0,7,"rfpH",+X, 0, 0);
	//--- Build top points J------------------------------------
	Translate(5,8,"rfpJ",-X, 0, 0);
	//--- Build side points K------------------------------------
	Translate(1,9,"rfpH",-X, 0, 0);
	//--- Build top points M------------------------------------
	Translate(7,10,"rfpM",0, 0, Q);
	//--- Build top points N------------------------------------
	Translate(9,11,"rfpN", 0, 0, Q);
	//--- Texture dormer ---------------------------------------
	SetCorner(7,9,11,10);
	//-----------------------------------------------------------
	top	= bevel[4].z;
	bpm->hgt = top;				//	style->SetBz(top);
	//--- Compute texture coordinates for top points ------------
	return GenerateTriangles(out);
}
//===================================================================================
//	Roof with Flat Top
//===================================================================================
//----------------------------------------------------------------------
//	Generate a flat roof
//----------------------------------------------------------------------
CRoofFLAT::CRoofFLAT(D2_ROOF_PM &pm)
	: CRoofModel(pm)
{	rmbc = 0;}
//----------------------------------------------------------------------
//	Generate a flat roof
//----------------------------------------------------------------------
CRoofFLAT::CRoofFLAT(U_INT id)
{	ident = id;
	rmbc	= 0;	
}
//----------------------------------------------------------------------
//	Build a flat roof
//----------------------------------------------------------------------
void CRoofFLAT::BuildRoof(Queue <D2_POINT> &inp, std::vector<D2_TRIANGLE*> &out)
{	GenerateRoof(inp,out);
	return;	}

//----------------------------------------------------------------------
//	Generate a flat roof:  There is no output.  The flat roof is already
//	computed as a result of the foot print triangulation
//	
//	Flat roof points are textured against the locals coordinates lx,ly
//	and roof extension
//----------------------------------------------------------------------
void  CRoofFLAT::GenerateRoof(Queue <D2_POINT> &inp, std::vector<D2_TRIANGLE*> &out)
{	D2_POINT *pp;
	for (pp = inp.GetFirst(); pp != 0; pp = pp->next) roofP->TextureLocalPoint(pp);
	return;
}
	
//=========================END 0F FILE ====================================================
