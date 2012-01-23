// 3dMath.h: interface for the CQuaternion class.
//
//////////////////////////////////////////////////////////////////////
#ifndef H3DMATH_H
#define H3DMATH_H
//===============================================================================
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Include/FlyLegacy.h"
#include "../Include/Queues.h"
class CVector;
class CQuaternion;
class CRotMatrix;
//=============================================================================
//  Struct for OpenGL matrix (column first)
//=============================================================================
struct MatrixGL {
  double e11, e21, e31, e41;
  double e12, e22, e32, e42;
  double e13, e23, e33, e43;
  double e14, e24, e34, e44;
};
//=============================================================================
//  Struct for OpenGL Plan
//=============================================================================
struct pn {
    double a, b, c, d;
};
//=============================================================================
//  SquareRootFloat:  Fast algo for square root at 10^-3 error (from  John Carmack)
//==============================================================================
inline float __fastcall SquareRootFloat(float number) {
    long i;
    float x, y;
    const float f = 1.5F;

    x = number * 0.5F;
    y  = number;
    i  = * ( long * ) &y;
    i  = 0x5f3759df - ( i >> 1 );
    y  = * ( float * ) &i;
    y  = y * ( f - ( x * y * y ) );
    y  = y * ( f - ( x * y * y ) );
    return number * y;
}
//=============================================================================
//  3D POINT DEFINITION
//=============================================================================
struct F3_VERTEX {
  float VT_X;
  float VT_Y;
  float VT_Z;
	//-------------------------------------------------
};
//=============================================================================
//  2D TEXTURE COORDINATES
//=============================================================================
struct F2_COORD {
  float VT_S;
  float VT_T;
};
//======================================================================================
//  2D planar pointcoordinates
//======================================================================================
struct P2_POINT {
	double x;
	double y;
	P2_POINT::P2_POINT() {x = y = 0;}
	};

//======================================================================================
//  CVector class
//======================================================================================
class CVector : public SVector {
public:
  CVector() { x = y = z = 0.0; }
  CVector(double a, double b, double c) {x=a; y=b; z=c; }
  CVector(const SVector &v) {x=v.x;y=v.y;z=v.z; }
  void   InvertXY() {double cy = y; y = z; z = cy;}
  float  FastLength();      // Fast sqrt 
  double Length();          ///< return the lenght of this vector
  double SqLength();        // squared length
  void    Zero()  {x = y = z = 0;}
  void    Set(double x, double y, double z);  //< Set the components of the vector
  void    Copy(const SVector &v);             //< Make this vector a copy of v
  void    Add(const SVector &v);              //< Add v to this vector
  void    Sum(SVector &v1,SVector &v2);       //  Add  2 vectors into one
  void    Dif(SVector &v1,SVector &v2);       //  Sub  2 vectors into one
  void    Subtract(const SVector &v);         //< Subtract v from this vector
  void    Subtract2D(SVector &v);
  void    Times(double s);                    //< Multiply the components with s
  double  DotProduct();                       // Own dot product
  double  DotProduct(const SVector &v);       //< Return the dot product of this vector and v
  double  DotProd2D(SVector &v);              //  Return the dot product in 2D
  void    CrossProduct(const SVector &u, const SVector &v); ///< Set this vector to u x v;
  void    AXplusB(SVector &A,double X,SVector &B);
  void    Corner(float *mem,double S,SVector &B);
  void    Integrate(double dt, CVector &dv1, CVector &dv2);
  void    Normalize();
  void    NormeDirection(SVector &v);
  void    NormeDirection(SVector & v1,SVector &v0);
  void    RandDirection(int a,int b,float rd);
  void    RotateZ(double ca,double sa);
  double  SqDistance(SVector &v);
  double  DistanceTo(SVector &v);
  float   GroundDistance(SVector &v);
	double  GroundDistance();
	void		Translation(SPosition &o, SPosition &d);
	void		FeetTranslation(SPosition &p1, SPosition &p2);
	void		FeetTranslation(double rdf,SPosition &p1, SPosition &p2);
  CVector& operator= (const CVector &aCopy) {Copy(aCopy); return *this;}
	//----VERTEX OPERATIONS ----------------------------------------------
	void		Set(C3_VTAB &t);
  void    MultMatGL(double *M, SVector &R);
	void    MultMatGL(float  *M, SVector &R);
  //--------------------------------------------------------------------
  inline  double MaxXY()  {return (x > y)?(x):(y);}
private:
  friend CVector operator- (const CVector &val);
  friend CVector operator- (const CVector &val1, const CVector &val2);
  friend CVector operator+ (const CVector &val1, const CVector &val2);
	//--------------------------------------------------------------------
public:
	inline void Raz() {x = y = z = 0;}
};
//========================================================================================
//	Class HVector:   Homogeneous vector for projection
//========================================================================================
class HVector {
protected:
	//---ATTRIBUTES ------------------------------
	double	x;
	double	y;
	double	z;
	double	w;
	//---METHODS --------------------------------
public:
	HVector::HVector();
	//-------------------------------------------
	void	Set(CVector &v);
	void	Set(C3_VTAB &t);
	void	MultMatGL(double *M, HVector &R);
	void	MultMatGL(float  *M, HVector &R);
	void	Extract(CVector &v);
};
//========================================================================================
//	Class CIntersection:   Structure to compute intersection between various figures
//========================================================================================
class CIntersector {
protected:
	//--- ATTRIBUTES -------------------------------------
	CVector *PA;												// Point A
	CVector *PB;												// Point B
	CVector  NP;												// Plane normal
	CVector  PP;												// Point in plane
public:
	//--- Results are public -----------------------------
	char    rc;													// Result
	CVector PR;													// Resulting point
	//--- METHODS ----------------------------------------
public:
	CIntersector() {rc = 0;}
	//----------------------------------------------------
	inline void		SetPA(CVector *pa)	{PA = pa;}
	inline void		SetPB(CVector *pb)	{PB = pb;}
	inline void   SetNP(CVector &np)  {NP = np;}
	inline void		SetPP(CVector &pp)	{PP = pp;}
	inline void   SetGA(double  alt)	{PP.z = alt;}
	//----------------------------------------------------
	void GetLineToPlane();
};
//========================================================================================
class CQuaternion  
{
public:
  double w;
  CVector v;

	CQuaternion ();
	CQuaternion (const double &w_, const double &vx, const double &vy, const double &vz);
	virtual ~CQuaternion();

  void Setup(CVector &eulerAngles);
  void GetRotMatrix(CRotMatrix &rotMatrix);

  double Norm(); ///< Return the norm of this quaternion
  void Add(CQuaternion *q); ///< Add q to this quaternion
  void Subtract(CQuaternion *q); ///< Subtract q from this quaternion
  void Inverse(CQuaternion *q); ///< Set this quaternion to the inverse of q
//  void Rotate(SVector *v); ///< Apply quaternion rotation to v
  void Product(CQuaternion *p, CQuaternion *q); ///< Set this quaternion to the product of p and q
  void QuaternionProduct(const CQuaternion *p, const CQuaternion *q); ///< Set this quaternion to the product of p and q
  void GetDerivative(CQuaternion &dq, SVector &rotRates); ///< Set dq to the time derivative of this quaternion
  void Integrate(double dt, CQuaternion &dq1, CQuaternion &dq2);
  void Normalize();

};
//========================================================================================
/// Rotation matrix. The matrix is used to transform column vectors (v' = Mv).
class CRotMatrix {
public:
  double m11, m12, m13;
  double m21, m22, m23;
  double m31, m32, m33;

public:
  /// default constructor: identity matrix
  CRotMatrix();

  /// Construct inertial to body transformation matrix for body euler angles
  void Setup(SVector &bAng);

  /// Construct wind to body transform matrix given alpha and beta
  void Setup(double alpha, double beta);

  /// Transform from inertial to body coordinates
  void TransformItoB(const SVector &vi, SVector &vb);

  /// Transform from body to inertial
  void TransformBtoI(SVector &vi, const SVector &vb);

  /// Transform from local (geo) to inertial
  void TransformLtoI(SVector &vi, const SVector &vb);

  /// Transform from inertial to local (geo) coordinates
  void TransformItoL(const SVector &vi, SVector &vb);

  void GetEulerAngles(CVector &eulerAngles);

  /// Rotation Matrix from Quaternion
  void QuaternionToRotMat (const CQuaternion &q);
};
//=========================================================================
//  CRoseMatrix Very simple matrix for rose 2D rotation (math.cpp)
//=========================================================================
class CRoseMatrix {
  typedef struct {  int x0;
                    int y0;
                    int x1;
                    int y1;
  } LEXT;
   //--------------Attributes --------------------------------
private:
  float sinN;
  float cosN;
  float sin5;
  float cos5;
  float sinR;
  float cosR;
  float sinA;
  float cosA;
  float sinB[9];                  // sin table
  float cosB[9];                  // cos Table
  LEXT  Mk10[9];                  // 10° Markers
  LEXT  Mk05[9];                  //  5° Markers
  //---------------------------------------------------------
public:
  CRoseMatrix(void);
  void  SetNorth(float deg);
  void  Rotate(int No,LEXT &org,LEXT &des);
  void  ComputeMark10(int radius, int dim);
  void  ComputeMark05(int radius, int dim);
  int   GetMarker(int type, int No, int &x0, int &y0, int &x1, int &y1);
  int   GetM10(int No,int &x0, int &y0, int &x1, int &y1);
  int   GetM05(int No,int &x0, int &y0, int &x1, int &y1);
  void  GetExt(int No,int &x1, int &y1);
  void  Get00D(int &x,int &y);
  void  Get03D(int &x,int &y);
  void  Get06D(int &x,int &y);
  void  Get09D(int &x,int &y);
  void  Get12D(int &x,int &y);
  void  Get15D(int &x,int &y);
  void  Get18D(int &x,int &y);
  void  Get21D(int &x,int &y);
  void  Get24D(int &x,int &y);
  void  Get27D(int &x,int &y);
  void  Get30D(int &x,int &y);
  void  Get33D(int &x,int &y);
};
//========================================================================
//  Random noise generator
//========================================================================
float RandNoise(int x);
float RandNoise(int x, int y);
//========================================================================
//  Edit latitude   LAT 99° 99' 99.99'' N for example
//       longitude  Lon 99° 99' 99.99'' W
//========================================================================
void EditLat2DMS(float lat, char *edt);
void EditLon2DMS(float lon, char *edt);
//========================================================================================
//  Homographic projection
//  Given 
//  a)  4 points defining a rectangle in the front view
//  b)  4 points defining the projection of this rectangle in a 
//      prespective view
//  we can compute the matrix H that project any ccordinate in the source rectangle
//  into a point inside the projected quadrilater
//      xp = U/W
//      yp = V/W            with [U,V,W] = [xs, ys, 1]* T
//      T = | a d g |
//          | b e h |
//          | c f 1 |
//  NOTE: Some code are coming from MatLAB
//========================================================================================
class CProjection {
  //--- No specific attributes as data are provided by the caller -------
  //--- METHODS --------------------------------------------------------
public:
  CProjection() {;}
  //--- solving linear system of degre n ------------
  int   DLU_Decomposition(double *A, int n);
  int   SolveUpTriangular(double *U, double B[], double x[], int n);
  void  SolveLoTriangular(double *L, double B[], double x[], int n);
  int   Solve(double *A, double B[], double x[], int n);
  //--- Creating T matrix ----------------------------
  int   CreateTM(double *X, TC_VTAB *s, TC_VTAB *d);
  int   RowFromX(double *row,TC_VTAB &s,TC_VTAB &d);
  int   RowFromY(double *row,TC_VTAB &s,TC_VTAB &d);
};
//==== Stand alone functions ==============================================================
int GreatestCommonDivisor (int i, int j);
//==== GEOMETRIC FUNCTIONS =================================================================
#define GEO_OPPOS_SIDE	(0)
#define GEO_INSIDE_PQ		(1)
#define GEO_OUTSIDE_PQ  (2)
#define GEO_SAME_SIDE		(3)
//--- Side of a point -----------------------------
#define GEO_ON_PQ			(4)
#define GEO_LEFT_PQ		(1)				// Left side
#define GEO_RITE_PQ		(2)				// right side
//-------------------------------------------------
#define GEO_OUTSIDE   (0)
#define GEO_ON_SIDE		(1)
#define GEO_INSIDE		(2)
//--- Type of POINT -------------------------------
#define GEO_CONVEX		(0)
#define GEO_REFLEX		(1)
//--- Type of FACE --------------------------------
#define GEO_FACE_NUL	(0)
#define GEO_X_FACE	  (0x00)
#define GEO_Y_FACE    (0x01)
#define GEO_N_FACE    (0x02)  // Negative face	
//---- 
//=========================================================================================
//	Geometric tester
//=========================================================================================
class GeoTest {
	//--- ATTRIBUTES ----------------------------------------
	double precision;													// Precision limit
	//--- Methods -------------------------------------------
public:
	GeoTest()		{precision	= DBL_EPSILON;}
	//--- Check side ----------------------------------------------
	int		SameSide(D2_POINT &P, D2_POINT &Q, D2_POINT &A, D2_POINT &B);
	bool	VisibilityTest(D2_POINT &P, D2_POINT &Q, D2_POINT &A, D2_POINT &B);
	int 	OnLeft(D2_POINT &A, D2_POINT &P, D2_POINT &Q);
	int 	InTriangle(D2_POINT &A, D2_POINT &P, D2_POINT &Q, D2_POINT &R);
	int   InTriangle(D2_POINT &A, D2_TRIANGLE &T);
	int	  Positive(D2_TRIANGLE &T);
	//-------------------------------------------------------------
	inline void		SetPrecision(double p)	{precision = p;}
	inline double GetPrecision()					{return precision;}
};
//====================================================================================
//	Slot to a 2D_POINT
//	Used for triagulation of the polygon
//====================================================================================
struct D2_SLOT {
	D2_SLOT  *prev;
	D2_SLOT  *next;
	D2_POINT *vrtx;
	//-----------------------------------------------
	char     R;							// Reflex indicator
	//-----------------------------------------------
	D2_SLOT::D2_SLOT(D2_POINT *p)
	{	prev	= 0;	next	= 0;	vrtx	= p; R = p->R;	}
	//-----------------------------------------------
	D2_POINT *D2_SLOT::GetVRTX()	{return vrtx;}
	//-----------------------------------------------
	bool D2_SLOT::IsReflex()			{return (R != 0);}
	//------------------------------------------------
	void D2_SLOT::SetType(char t)	{R = t;}
	};
//====================================================================================
//	Class face for stair extrusion
//====================================================================================
class D2_Face {
	// ATTRIBUTES ------------------------------------------------
protected:
	//--- Corner definitions ----------------------------
	D2_POINT sw;										// SW corner
	D2_POINT se;										// SE corner
	D2_POINT ne;										// NE corner
	D2_POINT nw;										// NW corner
	//--- public Methods -------------------------------
public:
	D2_Face();
	//--------------------------------------------------
	void Extrude(double f,double c,D2_POINT *sw,D2_POINT *se);
};
//====================================================================================
//	Class Stair for stair extrusion
//====================================================================================
class D2_Floor {
	//--- ATTRIBUTES ------------------------------------------
public:
	int			sNo;											// Stair number
	double	floor;										// Height
	double  ceil;
	std::vector<D2_Face*> faces;			// Polygon faces
	//--- METHODS ---------------------------------------------
public:
	D2_Floor() {;}
	D2_Floor(int e, double f, double c);
 ~D2_Floor();
 //----------------------------------------------------------
};
//====================================================================================
//	TRIANGULATOR for triangulation of polygones
//====================================================================================
class Triangulator {
	//--- ATTRIBUTES ------------------------------------
	GeoTest	geo;
	Queue <D2_POINT> extp;												// External contour
	Queue <D2_POINT> hole;												// Hole contour
	Queue <D2_SLOT>  slot;												// Slot list
	D2_POINT *P0;																	// First point
	D2_POINT *Pn;																	//  Last Point
	std::vector<D2_TRIANGLE *> out;								// Output triangle
	//----------------------------------------------------
	D2_TRIANGLE tri;															// Internal triangle
	D2_TRIANGLE qtr;															// Qualify triangle
	//----------------------------------------------------
	D2_POINT   *TO;																// Texture origin
	double      dlg;															// Larger diagonal
	//---------------------------------------------------------------
	double      minx;															// Mini x
	D2_POINT   *pminx;														// Point with minx
	double      maxx;															// Max  x
	D2_POINT   *pmaxx;														// Point with maxx
	double      miny;															// mini y
	D2_POINT   *pminy;														// Point with miny
	double      maxy;															// Max  y
	D2_POINT   *pmaxy;														// Point with maxy
	//----------------------------------------------------
	double	surf;																	// Ground surface
	//----------------------------------------------------
	double  rotM[4];															// Rotation matrix
	//----------------------------------------------------
	char    trace;																// Trace indicator
	char    vRFX;																	// Reflex indicator
	char    hIND;																	// Hole indicator
	char    face;																	// Type of face			
	//----------------------------------------------------
	U_INT   num;																	// Null number
	U_INT		seq;																	// Sequence number
	//--- METHODS ----------------------------------------
public:
	Triangulator();
 ~Triangulator();
  //-----------------------------------------------------
	void		AddVertex(double x, double y);
	void		NewHole();
	void		Load(char *fn);
	bool	  QualifyPoints();
	void		Triangulation();
	void		QualifyFaces();
	void		Extrude(int e,int h);
	void		Start();
	void		DrawLines();					// Drawing interface
	//-----------------------------------------------------
	inline double	GetSurface()		{return surf;}
protected:
  //-----------------------------------------------------
	void		Clean();
	void		ParseOBJ(FILE *fp);
	bool		ParseVTX(char *txt);
	bool		ParseHOL(char *txt);
	//------------------------------------------------------
	bool		NotAnEar(D2_SLOT  *sa);
	void		GetAnEar();
	void		Requalify(D2_SLOT *sp);
	//----Hole processing  -------------------------------
	void			Merge();
	D2_POINT *MatchHole(D2_POINT *xp);
	bool			CheckInternal(D2_POINT *xp, D2_POINT *hp);
	bool			CheckExternal(D2_POINT *xp, D2_POINT *hp);
	void			Splice(D2_POINT *xp, D2_POINT *hp);
	//---- Face processing --------------------------------
	void		QualifyEdge(D2_POINT *pa);
	void		BuildFloor(int no,double f, double c, D2_POINT *sw);
	//------------------------------------------------------
	void		TraceOut();
	void		TraceInp();
	void		TraceFace();
};

//==========================================================================================
#endif // H3DMATH_H