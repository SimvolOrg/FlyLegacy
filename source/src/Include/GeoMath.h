//=================================================================================
//  Copyright 2007 jean Sabatier
// Mathematical utility for Geo position
//=================================================================================
// Earth coordinates in latitude and longitude requets specific math
//  frame to accomodate for the modulus.  At the cross junction of the zero
//  meridian, classical add and substract don't work correctly, as the world is round
//
//  Another point is related to the precision. To avoid flickering when terrain is
//  rendered, coordinates are expressed as a tupple(B,G) where
//  B is a Box index and G a relative latitude or longitude in the Box(x,z)
//
//  A Box is a square area of 64 by 64 QGT
//=================================================================================
#if !defined(GEOMATH_H)
#define GEOMATH_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//=============================================================================
//  class for Terrain info
//=============================================================================
class GroundSpot {
public:
  C_QGT *qgt;                         // QGT of the spot
  C_QGT *pgt;                         // Previous QGT
  U_INT   kx;                         // Full spot X key
  U_INT   kz;                         // Full spot Z Key
  //------------------------------------------------------------
  short   qx;                         // Horizontal QGT index [0-511]
  short   qz;                         // Vertical QGT   index [0-511]
  short   tx;                         // Horizontal Detail tile index [0-31]
  short   tz;                         // Vertical detail tile index [0-31]
  short   vx;                         // Vertex X key
  short   vz;                         // Vertex Z key
  //----Terrain ready ------------------------------------------
  char    Rdy;                        // Terrain ready
  char    Type;                       // Terrain type
  CmQUAD *Quad;                       // Terrain quad
  CVector gNM;                        // Ground Normal
  //----Ground spot coordinates in absolute world --------------
  double  lon;                        // longitude
  double  lat;                        // Lattitude
  double  alt;                        // Altitude
  double  lbd;                        // Longitude in band
	double  rdf;				// Reduction factor for feet conversion
  //-------------------------------------------------------------
  GroundSpot();
  GroundSpot(double x,double y);
  //-------------------------------------------------------------
  void    GetCoordinates(U_INT xk,U_INT zk);
	void		ArcDistanceTo (CVertex *vtx,CVector &v);
	void		FeetDistanceTo(CVertex *vtx,CVector &v);
  //-------------------------------------------------------------
  void    SetQGT(C_QGT *qgt);
  bool    ValidQGT();
  bool    IsLeftOf(GroundSpot &gs);
  char    GetTerrain();
	double	GetAltitude(SPosition &p);
	bool    InvalideQuad();
  //------------------------------------------------------------
	inline void		Edit(char *txt) {sprintf(txt,"QGT(%03d-%03d) TILE(%02d-%02d) GROUND=%4d feet",
		qx,qz,tx,tz,int(alt));}
	inline void	  ClampLon()	{if (lon > TC_FULL_WRD_ARCS) lon = 0;}
  inline U_INT  xQGT()            {return qx;}
  inline U_INT  zQGT()            {return qz;}
  inline short  xDet()            {return tx;}
  inline short  zDet()            {return tz;}
  inline void   Reset()           {pgt = qgt; qgt = 0;}
  inline void   PopQGT()          {if (0 == qgt)  qgt = pgt;}
  inline double BandLongitude()   {return lbd;}
  inline bool   HasQGT()          {return (0 != qgt);}
	//-------------------------------------------------------------
};

//=============================================================================
// 2D Translation 
//=============================================================================
typedef struct {
  double x;                             // X component
  double y;                             // Y component
} TRANS2D;
//=============================================================================
// GLOBAL routines 
//=============================================================================
double horzBAND[];                      // Declaration for longitude bands
double vertBAND[];                      // Declaration for latitude bands
//--------------------------------------------------------------
//
// Determine various terrain tile indices for a given position
//
//--------------------------------------------------------------
//------------------------------------------------------------
short   GetGbtVertPoleRange(short vt);
short   GetGbtVertEquaRange(short vt);
short   GetGbtHorzRange(short vt);
double  GetGbtSouthLatitude(short cz);
double  GetGbtNorthLatitude(short cz);
double  LastLatitude();
//---------------------------------------------------------------
int     GetVerticalQgtNumber(double lat);
void    IndicesInQGT (SPosition pos, U_INT &x, U_INT &z);
void    IndicesInQGT (GroundSpot &gsp);
short   GetQgtHorizonRange(short vt);
short   GetQgtVertPoleRange(short vt);
short   GetQgtVertEquaRange(short vt);
double  GetQgtSouthLatitude(short cz);
double  GetQgtNorthLatitude(short cz);
float   GetMediumCircle(int tz);
float   GetFogDensity(int tz);
//----------------------------------------------------------------
float		GetRealFlatDistance(CmHead *obj);
float		GetFlatDistance(SPosition *to);
double  LongitudeDifference(double f1,double f2);
SVector SubtractPosition(SPosition &from, SPosition &to);
U_INT   QgtDifference(U_INT q1,U_INT q2);
double  AddLongitude(double f1,double f2);
void    AddToPosition(SPosition &pos,SVector &v);
void    AddToPosition(SPosition *pos, float dx, float dy);
void    GetRRtoLDOrientation(SVector &ld);
double  GetLatitudeArcs(U_INT tz);
double  GetLatitudeDelta(U_INT tz);
double  GetTileSWcorner(U_INT ax,U_INT az,SVector &v);
double  LongitudeInBand(U_INT qx, double lon);
double	GetAngleFromGeoPosition(SPosition &p1,SPosition &p2);
void    GetVertexCoordinates(U_INT vx,U_INT vz,SVector &v);
void    Add2dPosition(SPosition &p1,SPosition &p2, SPosition &r);
void    AddMilesTo(SPosition &pos,double mx,double my);
void    GetLatitudeFactor(double lat,double &rf,double &cp);
void    GetQgtMidPoint(int gx,int gz,SPosition &p);
int     GetRounded(float nb);
double  RoundAltitude(double a);
int			RoundAltitude(int a);
U_INT   AbsoluteTileKey(int qx, int dx);
U_INT   GetTileFromLatitude(double lat);
bool    TileIsLeft(U_INT k1,U_INT k2);
bool    PointInTriangle(CVector &p,CVector &a,CVector &b,CVector &c,CVector &n);
U_INT   NextVertexKey(U_INT vk,U_INT inc);
U_INT   GetSEAindex(U_INT cx,U_INT cz);       
//-----------------------------------------------------------------------------
inline  U_INT   QGTKEY(U_INT cx,U_INT cz) {return ((cx << 16) | (cz));}
//-----------------------------------------------------------------------------
double  GetCompensationFactor(short cz);
double  GetReductionFactor(U_INT cz);
void    FeetCoordinates(SPosition &pos,SVector &v);
void		FeetCoordinates(SVector &v, double rdf);
//-----------------------------------------------------------------------------
void    InitGlobeTileTable ();
void    InitQgtTable(float vmax);
//-----------------------------------------------------------------------------
float   ComputeDeviation(float ref,float rad,U_CHAR *flag, U_CHAR pwr);

//=============================================================================
//  Inline globale functions
//=============================================================================
//-----------------------------------------------------------------------------
//  return detail tile absolute key given
//	sp = No of Super Tile
//	nd = No of Detail Tile in Super Tile
//-----------------------------------------------------------------------------
inline U_INT MakeDetInQGT(U_INT sp,U_INT nd)
{	U_INT sx = (sp >> 3);										// X(sup)
	U_INT	sz = (sp & 0x07);									// Z(sup)
	U_INT nx = (nd >> 2);										// X(det)
	U_INT	nz = (nd & 0x03);									// Z(det)
	//-----------------------------------------------------
	sx	= (sx << 2) | nx;
	sz	= (sz << 2) | nz;
	//-----------------------------------------------------
	return (sx << 16) | sz;
}
//-----------------------------------------------------------------------------
//  return detail tile absolute key
//-----------------------------------------------------------------------------
inline U_INT MakeDetKey(U_INT qx,U_INT tx,U_INT qz, U_INT tz)
{ return (qx << (TC_BY32 + 16)) | (tx << 16) | (qz << TC_BY32) | tz; }
//-----------------------------------------------------------------------------
//  return Super Tile from Detail Indices TX-TZ
//	There are 4 * 4 details tiles per Super Tile
//	There are 8 * 8 super tile per QGT
//-----------------------------------------------------------------------------
inline U_CHAR MakeSuperNo(U_INT tx, U_INT tz)
{	U_INT sx = (tx >> 2);										// Divide TX by 4
	U_INT sz = (tz >> 2);										// Divide TZ by 4
	return ((sz << 3) | tz);  							// Return (8 * sz) + sx
}
//-----------------------------------------------------------------------------
//	Make a world key for Super Tile with
//	qx	= QGT x index								[0-511]	9 bits
//	qz		QGT z index								[0-511]	9 bits
//	tx  = Tile x index							[0-31]	5 bits
//	tz  = Tile z index							[0-31]	5 bits
//-----------------------------------------------------------------------------
inline U_INT WorldSuperKey(U_INT ax,U_INT az)
{	U_INT asx = (ax >> 2);					// Isolate QGT-SUP(X)
	U_INT asz = (az >> 2);					// Isolate QGT-SUP(Z)
	return (asx << 16) | asz;	}
//-----------------------------------------------------------------------------
//	Make a QGT-DET key from vertex indices
//-----------------------------------------------------------------------------
inline U_INT WorldDetailKey(U_INT vx,U_INT vz)
{	U_INT tx = vx >> TC_BY1024;
	U_INT tz = vz >> TC_BY1024;
	return (tx << 16) | tz;
}
//-----------------------------------------------------------------------------
//  return xTrans  factor
//-----------------------------------------------------------------------------
inline double GetXTRANS(U_CHAR a,U_CHAR q)
{ U_INT ind   = a | q;
  return horzBAND[ind];
}
//-----------------------------------------------------------------------------
//  return xTrans  factor for Vertex CX
//-----------------------------------------------------------------------------
double GetXTRANS(CVertex *vt);
//-----------------------------------------------------------------------------
//  return yTrans  factor
//-----------------------------------------------------------------------------
inline double GetYTRANS(U_CHAR a,U_CHAR q)
{ U_INT ind   = a | q;
  return vertBAND[ind];
}
//-----------------------------------------------------------------------------
//  return reduction factor
//-----------------------------------------------------------------------------
inline double GetReductionFactor(double lat)
{ double rad = FN_RAD_FROM_ARCS(lat);				// DegToRad(lat / 3600);
  return cos(rad);}
//-----------------------------------------------------------------------------
//  Return the band longitude of vertex
//  vx = full vertex indice
//  Band longitude is the relative longitude in current band (64 QGT)
//-----------------------------------------------------------------------------
inline double GetBandLongitude(U_INT vx)
{ U_INT bm = (vx & TC_BANDMOD);                   // Modulo Band
  return FN_ARCS_FROM_SUB(bm);
}
//-----------------------------------------------------------------------------
//  Compute COAST File index from QGT(X-Z) key
//-----------------------------------------------------------------------------
inline U_INT GetCoastIndex(U_INT key)
{ U_INT cx = key >> 16;                               // QGT X index
  U_INT cz = key & (511);                             // QGT Z index
  U_INT gx = cx >> 1;                                 // Global TILE X
  U_INT gz = cz >> 1;                                 // Global TILE Z
  return (gx << 16) + gz;                             // Final index
}
//-----------------------------------------------------------------------------
//  Compute AXBY key for QGT
//  x0 = QGT x indice
//  z0 = QGT z indice
//          -----------
//          | QA | QX | Access keys to the 4 QGTs are computed in key
//          -----------   This ensures that all coast files are loaded
//          | QB | QY |   before QA is processed. The 4 files are needed
//          -----------   to correctly compute coast tiles that are 
//                        bordering QA
//-----------------------------------------------------------------------------
inline void GetAXBYkeyForQGT(int x0,int z0,int*key)
{ int x1  = (x0+1) & (511);                 // Right column modulo 512
  int z1  = (z0)?(z0 - 1):(0);              // row below
  key[0]  = (x0 << 16) | z0;                // A tile
  key[1]  = (x1 << 16) | z0;                // X Tile
  key[2]  = (x0 << 16) | z1;                // B Tile
  key[3]  = (x1 << 16) | z1;                // Y Tile
  return;
}
//-----------------------------------------------------------------------------
//  Compute billboard matrix
//-----------------------------------------------------------------------------
inline void BillBoardMatrix(double *mat)
{ glGetDoublev(GL_MODELVIEW_MATRIX , mat);
  // undo all rotations
  //----- beware all scaling is lost as well ---------
  for(int i=0; i<3; i++ ) 
	  for(int j=0; j<3; j++ ) {
		  if ( i==j )
			  mat[(i << 2)+j] = 1.0;
		  else
			  mat[(i << 2)+j] = 0.0;
	}
  return;
}
//=============================================================================
//  WCoord holds a geographical position (lat,lon,alt) inside a BOX
//=============================================================================
class WCoord {
friend class CVertex;
protected:
  //-----BOX Coordinate -------------------------------------------
  double WX;                           // Coordinate X
  double WY;                           // Coordinate Y
  double WZ;                           // Coordinate Z
  //----Band indice -----------------------------------------------
  char  xB;                           // X band
  char  yB;                           // Y band
  //---------------------------------------------------------------
public:
  WCoord();
  //---------------------------------------------------------------
  void  Set(U_INT cx,U_INT cz);       // Init coordinates
  void  Assign2D(SVector &v1);
  //-----Return maximum band coordinate ---------------------------
  double  GetMX();
  double  GetMY();      
  //---------------------------------------------------------------
  void  AssignNE(TC_GTAB *tab);       // Assign NE vertex
  void  AssignNB(TC_GTAB *tab);       // Assign North border
  void  AssignNW(TC_GTAB *tab);       // Assign NW vertex
  void  AssignWB(TC_GTAB *tab);       // Assign West  border
  void  AssignSW(TC_GTAB *tab);       // Assign SW vertex
  void  AssignSB(TC_GTAB *tab);       // Assign South border
  void  AssignSE(TC_GTAB *tab);       // Assign SE vertex
  void  AssignEB(TC_GTAB *tab);       // Assign East border
	void	AssignCT(TC_GTAB *tab);				// Assign center
  //---------------------------------------------------------------
  void  Assign(double *ft);
  void  SetTour(F3_VERTEX *t,char v);
  //---------------------------------------------------------------
  inline void   SetWX(double x)  {WX = x;}
  inline void   SetWY(double y)  {WY = y;}
  inline void   SetWZ(double z)  {WZ = z;}
		//---------------------------------------------------------------
	inline void		AssignHT(TC_GTAB *tab)	{tab->GT_Z = WZ;}
  //-----Return World coordinate (band relative)-------------------
  inline double  GetWX()          {return WX;}
  inline double  GetWY()          {return WY;}
  inline double  GetWZ()          {return WZ;}
};
//=============================================================================

//========================================================================================
#endif // !defined(GEOMATH_H)
