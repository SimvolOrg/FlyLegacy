// 3dMath.cpp: implementation of the CQuaternion class.
//
//////////////////////////////////////////////////////////////////////

#include "../Include/3dMath.h"
#include "../Include/Utility.h"
#include "../Include/GeoMath.h"
#include "../Include/Cameras.h"
#include <math.h>

#ifdef  _DEBUG
  //#define _DEBUG_VEH            //print lc DDEBUG file ... remove later
#endif
//================================================================================
//  SIMPLE 2D rotation matrix for rose compass
//  To speed up CPU we use a table of sin and cos for 10° increments.
//  Todo:   Try to compute in integer by multipling all trigonometrics values by 16384
//          to get 4 digits of precision.
//    Then shift out results for x and y values.
//================================================================================
CRoseMatrix::CRoseMatrix()
{ int inx = 0;
  float deg = 0;
  float rad = 0;
  while (inx < 9)
  { rad = DegToRad(deg);
    sinB[inx] = sin(rad);
    cosB[inx] = cos(rad);
    deg += 10;
    inx ++;
  }
  sinN  = 1;
  cosN  = 0;
  sin5  = sin(5 * DEG2RAD);
  cos5  = cos(5 * DEG2RAD);
}
//-----------------------------------------------------------------------
//  Set the North angle
//-----------------------------------------------------------------------
void CRoseMatrix::SetNorth(float deg)
{ float  rad = DEG2RAD * deg;
  sinN = sin(rad);
  cosN = cos(rad);
  return;
}
//-----------------------------------------------------------------------
//  Rotate the coordinates and store result
//-----------------------------------------------------------------------
void CRoseMatrix::Rotate(int No,LEXT &org,LEXT &des)
{ float sinR  = (sinA * cosB[No]) + (cosA * sinB[No]);
  float cosR  = (cosA * cosB[No]) - (sinA * sinB[No]);
  //---- Rotate the original coordinates to the destination coordinates -
  des.x0 = int((org.x0 * cosR) - (org.y0 * sinR));
  des.y0 = int((org.x0 * sinR) + (org.y0 * cosR));
  des.x1 = int((org.x1 * cosR) - (org.y1 * sinR));
  des.y1 = int((org.x1 * sinR) + (org.y1 * cosR));
}
//-----------------------------------------------------------------------
//  Compute the 10° Markers
//-----------------------------------------------------------------------
void CRoseMatrix::ComputeMark10(int radius, int dim)
{ LEXT  org;
  org.x0    =  0;                               // From xo
  org.y0    =  radius - 1;                      // From y0 (circle already drawn)
  org.x1    =  0;                               // To   x1
  org.y1    =  radius - dim + 1;                // To   y1
  int inx   =  0;
  sinA      = sinN;
  cosA      = cosN;
  for (inx = 0; inx < 9; inx++) Rotate(inx,org,Mk10[inx]);
  return;
}
//-----------------------------------------------------------------------
//  Compute the 5° Markers
//-----------------------------------------------------------------------
void CRoseMatrix::ComputeMark05(int radius, int dim)
{ LEXT  org;
  org.x0    =  0;                               // From xo
  org.y0    =  radius - 1;                      // From y0 (circle already drawn)
  org.x1    =  0;                               // To   x1
  org.y1    =  radius - dim + 1;                // To   y1
  int inx   =  0;
  sinA      = (sinN * cos5) + (cosN * sin5);
  cosA      = (cosN * cos5) - (sinN * sin5);
  for (inx = 0; inx < 9; inx++) Rotate(inx,org,Mk05[inx]);
  return;
}
//------------------------------------------------------------------------
//  Return marker coordinates
//------------------------------------------------------------------------
int CRoseMatrix::GetMarker(int type, int No, int &x0, int &y0, int &x1, int &y1)
{ if (No >= 9)  return 0;
  return (type == 10)?(GetM10(No,x0,y0,x1,y1)):(GetM05(No,x0,y0,x1,y1));
}
//------------------------------------------------------------------------
//  return the 10° marker
//------------------------------------------------------------------------
int CRoseMatrix::GetM10(int No,int &x0, int &y0, int &x1, int &y1)
{ x0  = Mk10[No].x0;
  y0  = Mk10[No].y0;
  x1  = Mk10[No].x1;
  y1  = Mk10[No].y1;
  return 1;
}
//------------------------------------------------------------------------
//  return the 05° marker
//------------------------------------------------------------------------
int CRoseMatrix::GetM05(int No,int &x0, int &y0, int &x1, int &y1)
{ x0  = Mk05[No].x0;
  y0  = Mk05[No].y0;
  x1  = Mk05[No].x1;
  y1  = Mk05[No].y1;
  return 1;
}
//------------------------------------------------------------------------
//  Return the 10° marker extremity
//------------------------------------------------------------------------
void  CRoseMatrix::GetExt(int No, int &x, int &y)
{ x  = Mk10[No].x0;
  y  = Mk10[No].y0;
  return;
}
//------------------------------------------------------------------------
//  Return the 0° extremity
//------------------------------------------------------------------------
void  CRoseMatrix::Get00D(int &x, int &y)
{ x  = -Mk10[0].x0;
  y  = -Mk10[0].y0;
  return;
}
//------------------------------------------------------------------------
//  Return the 30° extremity
//------------------------------------------------------------------------
void  CRoseMatrix::Get03D(int &x, int &y)
{ x  = -Mk10[3].x0;
  y  = -Mk10[3].y0;
  return;
}
//------------------------------------------------------------------------
//  Return the 60° extremity
//------------------------------------------------------------------------
void  CRoseMatrix::Get06D(int &x, int &y)
{ x  = -Mk10[6].x0;
  y  = -Mk10[6].y0;
  return;
}
//------------------------------------------------------------------------
//  Return the 90° extremity
//------------------------------------------------------------------------
void  CRoseMatrix::Get09D(int &x, int &y)
{ x  = +Mk10[0].y0;
  y  = -Mk10[0].x0;
  return;
}
//------------------------------------------------------------------------
//  Return the 120° extremity
//------------------------------------------------------------------------
void  CRoseMatrix::Get12D(int &x, int &y)
{ x  = +Mk10[3].y0;
  y  = -Mk10[3].x0;
  return;
}
//------------------------------------------------------------------------
//  Return the 150° extremity
//------------------------------------------------------------------------
void  CRoseMatrix::Get15D(int &x, int &y)
{ x  = +Mk10[6].y0;
  y  = -Mk10[6].x0;
  return;
}
//------------------------------------------------------------------------
//  Return the 180° extremity
//------------------------------------------------------------------------
void  CRoseMatrix::Get18D(int &x, int &y)
{ x  = Mk10[0].x0;
  y  = Mk10[0].y0;
  return;
}
//------------------------------------------------------------------------
//  Return the 210° extremity
//------------------------------------------------------------------------
void  CRoseMatrix::Get21D(int &x, int &y)
{ x  = Mk10[3].x0;
  y  = Mk10[3].y0;
  return;
}
//------------------------------------------------------------------------
//  Return the 240° extremity
//------------------------------------------------------------------------
void  CRoseMatrix::Get24D(int &x, int &y)
{ x  = Mk10[6].x0;
  y  = Mk10[6].y0;
  return;
}
//------------------------------------------------------------------------
//  Return the 270° extremity
//------------------------------------------------------------------------
void  CRoseMatrix::Get27D(int &x, int &y)
{ x  = -Mk10[0].y0;
  y  = +Mk10[0].x0;
  return;
}
//------------------------------------------------------------------------
//  Return the 300° extremity
//------------------------------------------------------------------------
void  CRoseMatrix::Get30D(int &x, int &y)
{ x  = -Mk10[3].y0;
  y  = +Mk10[3].x0;
  return;
}
//------------------------------------------------------------------------
//  Return the 330° extremity
//------------------------------------------------------------------------
void  CRoseMatrix::Get33D(int &x, int &y)
{ x  = -Mk10[6].y0;
  y  = +Mk10[6].x0;
  return;
}
//==========================================================================
//  Random generator for one seed
//==========================================================================
float RandNoise(int x)
{ x = pow(float(x << 13),x);
  return (1 - ((x*(x * 15731 + 789221) + 1376312589) & 0x7FFFFFFF) / 1073741824);
}
//==========================================================================
//  Random generator for two seed
//==========================================================================
float RandNoise(int x, int y)
{ int n = x + (y * 57);
  n = pow(float(n << 13),n);
  return (1 - ((n*(n * 15731 + 789221) + 1376312589) & 0x7FFFFFFF) / 1073741824);
}
//==========================================================================
// 3D Vector
//==========================================================================
inline double CVector::Length() {
  return sqrt(DotProduct(*this));
}
//--------------------------------------------------------------------------
float CVector::FastLength()
{ double dg = (x*x) + (y*y) + (z*z);
  return SquareRootFloat(dg);
}
//----------------------------------------------------------------
//	Set from values
//----------------------------------------------------------------
void CVector::Set(double x, double y, double z) {
  this->x = x;
  this->y = y;
  this->z = z;
}

inline void CVector::Copy(const SVector &v) {
  x = v.x;
  y = v.y;
  z = v.z;
}
//---------------------------------------------------------------
//  3D Sum
//---------------------------------------------------------------
inline void CVector::Sum(SVector &v1,SVector &v2)
{ x = v1.x + v2.x;
  y = v1.y + v2.y;
  z = v1.z + v2.z;
}//---------------------------------------------------------------
//  3D Dif
//---------------------------------------------------------------
inline void CVector::Dif(SVector &v1,SVector &v2)
{ x = v1.x - v2.x;
  y = v1.y - v2.y;
  z = v1.z - v2.z;
}
//---------------------------------------------------------------
//  3D Add
//---------------------------------------------------------------
inline void CVector::Add(const SVector &v) {
  x += v.x;
  y += v.y;
  z += v.z;
}
//---------------------------------------------------------------
//  3D Subtract
//---------------------------------------------------------------
inline void CVector::Subtract2D(SVector &v)
{ x = x - v.x;
  y = y - v.y;
}
//---------------------------------------------------------------
//  3D Subtract
//---------------------------------------------------------------
inline void CVector::Subtract(const SVector &v) {
  x = x - v.x;
  y = y - v.y;
  z = z - v.z;
}
//---------------------------------------------------------------
//  Scalar product
//---------------------------------------------------------------
inline void CVector::Times(double s) {
  x *= s;
  y *= s;
  z *= s;
}
//---------------------------------------------------------------
//  A*X + B
//---------------------------------------------------------------
void CVector::AXplusB(SVector &A,double X,SVector &B)
{ x = (A.x * X) + B.x;
  y = (A.y * X) + B.y;
  z = (A.z * X) + B.z;
}
//---------------------------------------------------------------
//  this*S + B in memory
//---------------------------------------------------------------
void CVector::Corner(float *mem,double S,SVector &B)
{ *mem++ = float((S * x) + B.x);
  *mem++ = float((S * y) + B.y);
  *mem   = float((S * z) + B.z);
}

//---------------------------------------------------------------
//  Squared Distance to V
//---------------------------------------------------------------
double CVector::SqDistance(SVector &v)
{ double dx = x - v.x;
  double dy = y - v.y;
  double dz = z - v.z;
  return ((dx*dx) + (dy*dy) + (dz*dz));
}
//---------------------------------------------------------------
//  Distance to V
//---------------------------------------------------------------
double CVector::DistanceTo(SVector &v)
{ double dx = x - v.x;
  double dy = y - v.y;
  double dz = z - v.z;
  return SquareRootFloat((dx*dx) + (dy*dy) + (dz*dz));
}
//---------------------------------------------------------------
//  Ground (X,Y) Distance to V
//---------------------------------------------------------------
float CVector::GroundDistance(SVector &v)
{ double dx = (x - v.x);
  double dy = (y - v.y);
  float  sq = ((dx*dx) + (dy*dy));
  return SquareRootFloat(sq);
}
//---------------------------------------------------------------
//  Ground (X,Y) Horizontal (X,Y) distance
//---------------------------------------------------------------
double CVector::GroundDistance()
{ return sqrt((x*x) + (y*y));
}

//---------------------------------------------------------------
//  Squared Length of vector V
//---------------------------------------------------------------
double CVector::SqLength()
{ return (x*x) + (y*y) + (z*z); }
//---------------------------------------------------------------
//  2D dot product 
//---------------------------------------------------------------
double CVector::DotProd2D(SVector &v)
{ return (x * v.x) + (y * v.y); }
//---------------------------------------------------------------
//  3D dot product 
//---------------------------------------------------------------
double CVector::DotProduct(const SVector &v)
{  return (x * v.x) + (y * v.y) + (z * v.z); }
//---------------------------------------------------------------
//  3D dot product 
//---------------------------------------------------------------
double CVector::DotProduct()
{  return (x * x) + (y * y) + (z * z); }

//---------------------------------------------------------------
//  3D Cross product 
//---------------------------------------------------------------
void CVector::CrossProduct(const SVector &u, const SVector &v) {
  x = u.y*v.z - u.z*v.y;
  y = u.z*v.x - u.x*v.z;
  z = u.x*v.y - u.y*v.x;
}

void CVector::Integrate(double dt, CVector &dv1, CVector &dv2) {
   x += dt*(3*dv1.x - dv2.x)/2;
   y += dt*(3*dv1.y - dv2.y)/2;
   z += dt*(3*dv1.z - dv2.z)/2;
}

void CVector::Normalize(void) {
  double length = Length();
  x /= length;
  y /= length;
  z /= length;
}
//---------------------------------------------------------------
//  Compute the normalized direction made by
//  This vector (as extremity) and v vector as origin
//---------------------------------------------------------------
void CVector::NormeDirection(SVector &v)
{ x -= v.x;
  y -= v.y;
  z -= v.z;
  Normalize();
}
//---------------------------------------------------------------
//  Compute the normalized direction made by
//  Vector v1 (as extremity) and vector v2 as origin
//---------------------------------------------------------------
void CVector::NormeDirection(SVector & v1,SVector &v0)
{ x  = v1.x - v0.x;
  y  = v1.y - v0.y;
  z  = v1.z - v0.z;
  Normalize();
}

//---------------------------------------------------------------
//  unary (-) operator surcharge 
//---------------------------------------------------------------
CVector operator- (const CVector &val)
{ CVector tmp (-val.x, -val.y, -val.z);
  return tmp;
}

//---------------------------------------------------------------
//  binary (+) operator surcharge 
//---------------------------------------------------------------
CVector operator+ (const CVector &val1, const CVector &val2)
{ CVector tmp (val1);
  tmp.x += val2.x;
  tmp.y += val2.y;
  tmp.z += val2.z;
  return tmp;
}

//---------------------------------------------------------------
//  binary (-) operator surcharge 
//---------------------------------------------------------------
CVector operator- (const CVector &val1, const CVector &val2)
{ CVector tmp (val1);
  tmp.x -= val2.x;
  tmp.y -= val2.y;
  tmp.z -= val2.z;
  return tmp;
}
//---------------------------------------------------------------
//  Randomize this direction vector
//---------------------------------------------------------------
void CVector::RandDirection(int a,int b,float rd)
{ x = RandNoise(a)   * rd;
  y = RandNoise(b)   * rd;
  z = RandNoise(a,b) * rd;
  return;
}
//---------------------------------------------------------------
//  Rotate around Z by alpha
//---------------------------------------------------------------
void CVector::RotateZ(double ca,double sa)
{ x *= ca;
  y *= sa;
  return;
}
//---------------------------------------------------------------
//  The matrix is an OpenGL matrix:  Order is per column
//---------------------------------------------------------------
void CVector::MultMatGL(double *M, SVector &R)
{ R.x = M[0]*x + M[4]*y + M[ 8]*z + M[12];
  R.y = M[1]*x + M[5]*y + M[ 9]*z + M[13];
  R.z = M[2]*x + M[6]*y + M[10]*z + M[14];
  return;
}
//---------------------------------------------------------------
//  The matrix is an OpenGL matrix:  Order is per column
//---------------------------------------------------------------
void CVector::MultMatGL(float *M, SVector &R)
{ R.x = M[0]*x + M[4]*y + M[ 8]*z + M[12];
  R.y = M[1]*x + M[5]*y + M[ 9]*z + M[13];
  R.z = M[2]*x + M[6]*y + M[10]*z + M[14];
  return;
}
//---------------------------------------------------------------
//  Set from vertice
//---------------------------------------------------------------
void CVector::Set(C3_VTAB &t)
{	x = t.VT_X;
	y	= t.VT_Y;
	z=  t.VT_Z;
	return;
}
//---------------------------------------------------------------
//  Compute translation from p1 to p2
//---------------------------------------------------------------
void	CVector::Translation(SPosition &p1, SPosition &p2)
{	x = LongitudeDifference(p2.lon, p1.lon);
	y = p2.lat - p1.lat;
	z = p2.alt - p1.alt;
}
//---------------------------------------------------------------
//  Compute translation from p1 to p2 in feet
//---------------------------------------------------------------
void	CVector::FeetTranslation(SPosition &p1, SPosition &p2)
{	double rdf = GetReductionFactor(p2.lat);
	double dx  = LongitudeDifference(p2.lon, p1.lon);
	double dy  = p2.lat - p1.lat;
	x = FN_FEET_FROM_ARCS(dx) * rdf;
	y = FN_FEET_FROM_ARCS(dy);
	z = p2.alt - p1.alt;
}
//---------------------------------------------------------------
//  Compute translation from p1 to p2 in feet
//---------------------------------------------------------------
void	CVector::FeetTranslation(double rdf,SPosition &p1, SPosition &p2)
{	double dx  = LongitudeDifference(p2.lon, p1.lon);
	double dy  = p2.lat - p1.lat;
	x = FN_FEET_FROM_ARCS(dx) * rdf;
	y = FN_FEET_FROM_ARCS(dy);
	z = p2.alt - p1.alt;
}
//==========================================================================
//  Homogeneous vector
//==========================================================================
HVector::HVector()
{	x = y	= z = 0;
	w	= 1;
}
//----------------------------------------------------------------
//	Init with a CVector
//----------------------------------------------------------------
void HVector::Set(CVector &v)
{	x = v.x;
	y	= v.y;
	z = v.z;
	w	= 1;
	return;
}
//----------------------------------------------------------------
//	Init with a C3_VTAB entry
//----------------------------------------------------------------
void HVector::Set(C3_VTAB &t)
{	x = t.VT_X;
	y	= t.VT_Y;
	z = t.VT_Z;
	w	= 1;
	return;
}
//---------------------------------------------------------------
//  The matrix is an OpenGL matrix:  Order is per column
//---------------------------------------------------------------
void HVector::MultMatGL(double *M, HVector &R)
{ R.x = M[0]*x + M[4]*y + M[ 8]*z + M[12];				// M line 0
  R.y = M[1]*x + M[5]*y + M[ 9]*z + M[13];				// M line 1
  R.z = M[2]*x + M[6]*y + M[10]*z + M[14];				// M line 2
	R.w = M[3]*x + M[7]*y + M[11]*z + M[15];				// M line 3
  return;
}
//---------------------------------------------------------------
//  The matrix is an OpenGL matrix:  Order is per column
//---------------------------------------------------------------
void HVector::MultMatGL(float *M, HVector &R)
{ R.x = M[0]*x + M[4]*y + M[ 8]*z + M[12];				// M line 0
  R.y = M[1]*x + M[5]*y + M[ 9]*z + M[13];				// M line 1
  R.z = M[2]*x + M[6]*y + M[10]*z + M[14];				// M line 2
	R.w = M[3]*x + M[7]*y + M[11]*z + M[15];				// M line 3
  return;
}
//---------------------------------------------------------------
//  Extract normalized coordinates
//---------------------------------------------------------------
void HVector::Extract(CVector &v)
{	v.x = x / w;
	v.y	= y / w;
	v.z	= z / w;
	return;
}
//==========================================================================
//  Edit Latitude in deg min sec
//==========================================================================
void EditLat2DMS(float lat, char *edt)
{ const char *pole  = (lat < 0)?("S"):("N");
  long  val   = (lat < 0)?(long(-lat * 100)):(long(lat *100));
  long  deg   =  0;
  long  min   =  0;
  deg   = (val / 360000);
  val   = (val % 360000);
  min   = (val / 6000);
  val   = (val % 6000);
  sprintf_s(edt,31,"Lat: %s %3u %2u' %2.2f\"",pole,int(deg),int(min),(float(val) / 100));
	edt[31] = 0;
  return;
}
//==========================================================================
//  Edit Longitude in deg min sec
//==========================================================================
void EditLon2DMS(float lon, char *edt)
{ if (lon > (180 * 3600)) lon -= (360 * 3600);
  const char *meri  = (lon < 0)?("W"):("E");
  long  val   = (lon < 0)?(long(-lon * 100)):(long(lon * 100));
  long  deg   =  0;
  long  min   =  0;
  deg   = (val / 360000);
  val   = (val % 360000);
  min   = (val / 6000);
  val   = (val % 6000);
  sprintf_s(edt,31,"Lon: %s %3u %2u' %3.2f\"",meri,int(deg),int(min),(float(val) / 100));
	edt[31] = 0;
  return;
}
//================================================================================
// Quaternion
//================================================================================
CQuaternion::CQuaternion() {
  w = 1;
}

CQuaternion::CQuaternion (const double &w_, const double &vx, const double &vy, const double &vz)
: w (w_)
{
  v.x = vx;
  v.y = vy;
  v.z = vz;
}

CQuaternion::~CQuaternion() {
}

void CQuaternion::Setup(CVector &eulerAngles) {
  double sp = sin(eulerAngles.x/2);
  double cp = cos(eulerAngles.x/2);
  double sh = sin(eulerAngles.y/2);
  double ch = cos(eulerAngles.y/2);
  double sb = sin(eulerAngles.z/2);
  double cb = cos(eulerAngles.z/2);

  w   = ch*cp*cb + sh*sp*sb;
  v.x = ch*sp*cb + sh*cp*sb;
  v.y = sh*cp*cb - ch*sp*sb;
  v.z = ch*cp*sb - sh*sp*cb;
}

void CQuaternion::GetRotMatrix(CRotMatrix &rM) {
  double w2 = w*w; double wx = w*v.x; double wy = w*v.y; double wz = w*v.z;
  double x2 = v.x*v.x; double xy = v.x*v.y; double xz = v.x*v.z;
  double y2 = v.y*v.y; double yz = v.y*v.z;
  double z2 = v.z*v.z;

  rM.m11 = w2+x2-y2-z2; rM.m12 = 2*(xy+wz);   rM.m13 = 2*(xz-wy);
  rM.m21 = 2*(xy-wz);   rM.m22 = w2-x2+y2-z2; rM.m23 = 2*(yz+wx);
  rM.m31 = 2*(xz+wy);   rM.m32 = 2*(yz-wx);   rM.m33 = w2-x2-y2+z2;
}


double CQuaternion::Norm() {
  return w*w + v.DotProduct(v);
}

void CQuaternion::Add(CQuaternion *q) {
  w += q->w;
  v.Add(q->v);
}

void CQuaternion::Subtract(CQuaternion *q) {
  w = w - q->w;
  v.Subtract(q->v);
}

void CQuaternion::Inverse(CQuaternion *q) {
  double s = 1/q->Norm();
  w = q->w*s;
  v.Copy(q->v);
  v.Times(-s);
}

void CQuaternion::Product(CQuaternion *p, CQuaternion *q) {
  w = p->w*q->w - p->v.DotProduct(q->v);
  v.CrossProduct(p->v, q->v);
  v.Times(2);
}

void CQuaternion::GetDerivative(CQuaternion &dq, SVector &rates) {
  /* 
   * The quaternion state equation dq(t) = 0.5*q(t)*w(t) - Stevens & Lewis p33
   */
  dq.w =   ( /* 0 */     -rates.x*v.x   -rates.y*v.y   -rates.z*v.z )/2;
  dq.v.x = ( rates.x*w   /* 0 */        +rates.z*v.y   -rates.y*v.z )/2;
  dq.v.y = ( rates.y*w   -rates.z*v.x   /* 0  */       +rates.x*v.z )/2;
  dq.v.z = ( rates.z*w   +rates.y*v.x   -rates.x*v.y   /* 0 */      )/2;
}

void CQuaternion::Integrate(double dt, CQuaternion &dq1, CQuaternion &dq2) {
  w += dt*(3*dq1.w + dq2.w)/2;
  v.Integrate(dt, dq1.v, dq2.v);
  Normalize();
}

void CQuaternion::Normalize(void) {
  double n = sqrt(Norm());
  w = w/n;
  v.Times(1.0/n);
}

void CQuaternion::QuaternionProduct(const CQuaternion *p, const CQuaternion *q) {
  w   = (q->w * p->w  ) - (q->v.x * p->v.x) - (q->v.y * p->v.y) - (q->v.z * p->v.z);
  v.x = (q->w * p->v.x) + (q->v.x * p->w  ) - (q->v.y * p->v.z) + (q->v.z * p->v.y);
  v.y = (q->w * p->v.y) + (q->v.x * p->v.z) + (q->v.y * p->w  ) - (q->v.z * p->v.x);
  //v.y *= -1.0; // according Even's xls sheet
  v.z = (q->w * p->v.z) - (q->v.x * p->v.y) + (q->v.y * p->v.x) + (q->v.z * p->w  );
}

//=======================================================================================
//	Rotation matrix
//========================================================================================
CRotMatrix::CRotMatrix() {
  m11 = m22 = m33 = 1.0;
  m12 = m13 = m21 = m23 = m31 = m32 = 0;
}


void CRotMatrix::Setup(SVector &bAng) {
   double sp = sin(bAng.x);
   double cp = cos(bAng.x);
   double sh = sin(bAng.y);
   double ch = cos(bAng.y);
   double sb = sin(bAng.z);
   double cb = cos(bAng.z);

   m11 = ch*cb + sh*sp*sb;  m12 = cp*sb;  m13 = -sh*cb + ch*sp*sb;
   m21 = -ch*sb + sh*sp*cb; m22 = cp*cb;  m23 = sh*sb + ch*sp*cb;
   m31 = sh*cp;             m32 = -sp;    m33 = ch*cp;
}


void CRotMatrix::Setup(double alpha, double beta) {
  double sa = sin(alpha);
  double ca = cos(alpha);
  double sb = sin(beta);
  double cb = cos(beta);

  m11 = cb;     m12 = 0;  m13 = sb;
  m21 = sa*sb;  m22 = ca; m23 = -sa*cb;
  m31 = -ca*sb; m32 = sa; m33 = ca*cb;
}

void CRotMatrix::TransformItoB(const SVector &vi, SVector &vb) {
  vb.x = m11*vi.x + m12*vi.y + m13*vi.z;
  vb.y = m21*vi.x + m22*vi.y + m23*vi.z;
  vb.z = m31*vi.x + m32*vi.y + m33*vi.z;
}


void CRotMatrix::TransformBtoI(SVector &vi, const SVector &vb) {
  // multiply vb with transpose of matrix;
  vi.x = m11*vb.x + m21*vb.y + m31*vb.z;
  vi.y = m12*vb.x + m22*vb.y + m32*vb.z;
  vi.z = m13*vb.x + m23*vb.y + m33*vb.z;
}

// transform from the local frame to the earth frame
void CRotMatrix::TransformLtoI(SVector &vi, const SVector &vl) {
  // not used --> not implemented
  vi.x = 0;
  vi.y = 0;
  vi.z = 0;
}

// transform from the earth frame to the local frame
void CRotMatrix::TransformItoL(const SVector &vi, SVector &vl) {
  // not used --> not implemented
  vl.x = 0;
  vl.y = 0;
  vl.z = 0;
}

void CRotMatrix::GetEulerAngles(CVector &eAng) {
  eAng.x = asin(-m32);

  if (m12 == 0 && m22 == 0) { // vertical
    eAng.y = atan2(-m13, m11);
    eAng.z = 0; // no bank
  }
  else {
    eAng.y = atan2(m31, m33);
    eAng.z = atan2(m12, m22);
  }
}

void CRotMatrix::QuaternionToRotMat (const CQuaternion &q) {
  m11 = (q.w * q.w) + (q.v.x * q.v.x) - (q.v.y * q.v.y) - (q.v.z * q.v.z);
  m12 = 2 * ((q.v.x * q.v.y) + (q.w * q.v.z));
  m13 = 2 * ((q.v.x * q.v.z) - (q.w * q.v.y));

  m21 = 2 * ((q.v.x * q.v.y) - (q.w * q.v.z));
  m22 = (q.w * q.w) - (q.v.x * q.v.x) + (q.v.y * q.v.y) - (q.v.z * q.v.z);
  m23 = 2 * ((q.v.y * q.v.z) + (q.w * q.v.x));
  
  m31 = 2 * ((q.v.x * q.v.z) + (q.w * q.v.y));
  m32 = 2 * ((q.v.y * q.v.z) - (q.w * q.v.x));
  m33 = (q.w * q.w) - (q.v.x * q.v.x) - (q.v.y * q.v.y) + (q.v.z * q.v.z);
}
//===================================================================================
//  Homographic projection
//===================================================================================
//==============================================================================
//  int DLU_Decomposition(double *A, int n)      (from MATLAB)                //
//                                                                            //
//  Description:                                                              //
//     This routine uses Doolittle's method to decompose the n x n matrix A   //
//     into a unit lower triangular matrix L and an upper triangular matrix U //
//     such that A = LU.                                                      //
//     The matrices L and U replace the matrix A so that the original matrix  //
//     A is destroyed.                                                        //
//     Note!  In Doolittle's method the diagonal elements of L are 1 and are  //
//            not stored.                                                     //
//     Note!  The determinant of A is the product of the diagonal elements    //
//            of U.  (det A = det L * det U = det U).                         //
//     This routine is suitable for those classes of matrices which when      //
//     performing Gaussian elimination do not need to undergo partial         //
//     pivoting, e.g. positive definite symmetric matrices, diagonally        //
//     dominant band matrices, etc.                                           //
//     For the more general case in which partial pivoting is needed use      //
//                  Doolittle_LU_Decomposition_with_Pivoting.                 //
//     The LU decomposition is convenient when one needs to solve the linear  //
//     equation Ax = B for the vector x while the matrix A is fixed and the   //
//     vector B is varied.  The routine for solving the linear system Ax = B  //
//     after performing the LU decomposition for A is Doolittle_LU_Solve      //
//     (see below).                                                           //
//                                                                            //
//     The Doolittle method is given by evaluating, in order, the following   //
//     pair of expressions for k = 0, ... , n-1:                              //
//       U[k][j] = A[k][j] - (L[k][0]*U[0][j] + ... + L[k][k-1]*U[k-1][j])    //
//                                 for j = k, k+1, ... , n-1                  //
//       L[i][k] = (A[i][k] - (L[i][0]*U[0][k] + . + L[i][k-1]*U[k-1][k]))    //
//                          / U[k][k]                                         //
//                                 for i = k+1, ... , n-1.                    //
//       The matrix U forms the upper triangular matrix, and the matrix L     //
//       forms the lower triangular matrix.                                   //
//                                                                            //
//  Arguments:                                                                //
//     double *A   Pointer to the first element of the matrix A[n][n].        //
//     int     n   The number of rows or columns of the matrix A.             //
//                                                                            //
//  Return Values:                                                            //
//     0  Success                                                             //
//    -1  Failure - The matrix A is singular.                                 //
//                                                                            //
//  Example:                                                                  //
//     #define N                                                              //
//     double A[N][N];                                                        //
//                                                                            //
//     (your code to intialize the matrix A)                                  //
//                                                                            //
//     err = Doolittle_LU_Decomposition(&A[0][0], N);                         //
//     if (err < 0) printf(" Matrix A is singular\n");                        //
//     else { printf(" The LU decomposition of A is \n");                     //
//           ...                                                              //
//==============================================================================
int CProjection::DLU_Decomposition(double *A, int n)
{
   int i, j, k, p;
   double *p_k, *p_row, *p_col;

//         For each row and column, k = 0, ..., n-1,
//            find the upper triangular matrix elements for row k
//            and if the matrix is non-singular (nonzero diagonal element).
//            find the lower triangular matrix elements for column k. 
 
   for (k = 0, p_k = A; k < n; p_k += n, k++) {
      for (j = k; j < n; j++) {
         for (p = 0, p_col = A; p < k; p_col += n,  p++)
            *(p_k + j) -= *(p_k + p) * *(p_col + j);
      }
      if ( *(p_k + k) == 0.0 ) return -1;
      for (i = k+1, p_row = p_k + n; i < n; p_row += n, i++) {
         for (p = 0, p_col = A; p < k; p_col += n, p++)
            *(p_row + k) -= *(p_row + p) * *(p_col + k);
         *(p_row + k) /= *(p_k + k);
      }  
   }
   return 0;
}
//==============================================================================
//  int Upper_Triangular_Solve(double *U, double *B, double x[], int n)       //
//                                                                            //
//  Description:                                                              //
//     This routine solves the linear equation Ux = B, where U is an n x n    //
//     upper triangular matrix.  (The subdiagonal part of the matrix is       //
//     not addressed.)                                                        //
//     The algorithm follows:                                                 //
//                  x[n-1] = B[n-1]/U[n-1][n-1], and                          //
//     x[i] = [B[i] - (U[i][i+1] * x[i+1]  + ... + U[i][n-1] * x[n-1])]       //
//                                                                 / U[i][i], //
//     for i = n-2, ..., 0.                                                   //
//                                                                            //
//  Arguments:                                                                //
//     double *U   Pointer to the first element of the upper triangular       //
//                 matrix.                                                    //
//     double *B   Pointer to the column vector, (n x 1) matrix, B.           //
//     double *x   Pointer to the column vector, (n x 1) matrix, x.           //
//     int     n   The number of rows or columns of the matrix U.             //
//                                                                            //
//  Return Values:                                                            //
//     0  Success                                                             //
//    -1  Failure - The matrix U is singular.                                 //
//                                                                            //
//  Example:                                                                  //
//     #define N                                                              //
//     double A[N][N], B[N], x[N];                                            //
//                                                                            //
//     (your code to create matrix A and column vector B)                     //
//     err = UTriangular_Solve(&A[0][0], B, x, n);                            //
//     if (err < 0) printf(" Matrix A is singular\n");                        //
//     else printf(" The solution is \n");                                    //
//           ...                                                              //
//==============================================================================
int CProjection::SolveUpTriangular(double *U, double B[], double x[], int n)
{ int i, k;
  for (k = n-1, U += n * (n - 1); k >= 0; U -= n, k--) {
      if (*(U + k) == 0.0) return -1;           // The matrix U is singular
      x[k] = B[k];
      for (i = k + 1; i < n; i++) x[k] -= x[i] * *(U + i);
      x[k] /= *(U + k);
   }

   return 0;
}
//==============================================================================
//  void Unit_Lower_Triangular_Solve(double *L, double *B, double x[], int n) //
//                                                                            //
//  Description:                                                              //
//     This routine solves the linear equation Lx = B, where L is an n x n    //
//     unit lower triangular matrix.  (Only the subdiagonal part of the matrix//
//     is addressed.)  The diagonal is assumed to consist of 1's and is not   //
//     addressed.                                                             //
//     The algorithm follows:                                                 //
//                          x[0] = B[0], and                                  //
//            x[i] = B[i] - (L[i][0] * x[0]  + ... + L[i][i-1] * x[i-1]),     //
//     for i = 1, ..., n-1.                                                   //
//                                                                            //
//  Arguments:                                                                //
//     double *L   Pointer to the first element of the unit lower triangular  //
//                 matrix.                                                    //
//     double *B   Pointer to the column vector, (n x 1) matrix, B.           //
//     double *x   Pointer to the column vector, (n x 1) matrix, x.           //
//     int     n   The number of rows or columns of the matrix L.             //
//                                                                            //
//  Return Values:                                                            //
//     void                                                                   //
//                                                                            //
//  Example:                                                                  //
//     #define N                                                              //
//     double A[N][N], B[N], x[N];                                            //
//                                                                            //
//     (your code to create matrix A and column vector B)                     //
//     LTriangular_Solve(&A[0][0], B, x, n);                                  //
//     printf(" The solution is \n");                                         //
//           ...                                                              //
//==============================================================================
void CProjection::SolveLoTriangular(double *L, double B[], double x[], int n)
{  int i, k;
   x[0] = B[0];
   for (k = 1, L += n; k < n; L += n, k++) 
      for (i = 0, x[k] = B[k]; i < k; i++) x[k] -= x[i] * *(L + i);
   return;
}
//===================================================================================
//  LINEAR SYSTEM SOLVER
//  Input:
//    A of dimension n is the Matrix coefficient of the system
//    B is the n-column constant matrix
//  Ouput:
//    X is the n-column result 
//===================================================================================
int CProjection::Solve(double *A, double B[], double x[], int n)
{ //--- Step 1: Decompose A in LU matrices
  DLU_Decomposition(A, n);
  //--- Step 2: Solve the linear equation Lx = B in x matrix --------------------
  SolveLoTriangular(A, B, x, n);
  //--- Step 3: 
  //     Solve the linear equation Ux = y, where y is the solution
  //     obtained above of Lx = B and U is an upper triangular matrix.
   return SolveUpTriangular(A, x, x, n);
}
//===================================================================================
//  build row from X values
//===================================================================================
int CProjection::RowFromX(double *row,TC_VTAB &s,TC_VTAB &d)
{ row[0] = s.VT_X;
  row[1] = s.VT_Y;
  row[2] = 1;
  row[3] = 0;
  row[4] = 0;
  row[5] = 0;
  row[6] = -d.VT_X * s.VT_X;
  row[7] = -d.VT_X * s.VT_Y;
  return 8;
}
//===================================================================================
//  build row from Y values
//===================================================================================
int CProjection::RowFromY(double *row,TC_VTAB &s,TC_VTAB &d)
{ row[0] = 0;
  row[1] = 0;
  row[2] = 0;
  row[3] = s.VT_X;
  row[4] = s.VT_Y;
  row[5] = 1;
  row[6] = -d.VT_Y * s.VT_X;
  row[7] = -d.VT_Y * s.VT_Y;
  return 8;
}
//===================================================================================
//  Create the T matrix from 8 points
//  We compute 8 coefficients (a,b,c,d,e,f,g) from 4 paire of points
//  T   receiver 3*3 T matrix (room for 9 coefficients)
//  S   is an array of TC_VTAB[4] defining the 4 source points
//  D   is an array of TC_VTAB[4] defining the 4 destination points
//  -----------------------------------------------------------------
//  a) Build The coefficient matrix CM from the 8 points coordinates
//  b) Build The constant matrix    CT from the 8 points coordinates
//  c) Solve the system of degre 8  CM * X = CT using the linear solver
//  NOTE that the resulting X is ordered as [a,b,c,d,e,f,g,h]
//===================================================================================
int CProjection::CreateTM(double *T, TC_VTAB *s, TC_VTAB *d)
{ double CM[64];                    // This is the coefficient matrix
  double CT[8];                     // This is the constante matrix
  double CX[8];                     // Resulting matrix
  //--- Build coefficient matrix -----------------------------------
  double *row = CM;
  row += RowFromX(row,s[0],d[0]);   // row 0
  row += RowFromX(row,s[1],d[1]);   // row 1
  row += RowFromX(row,s[2],d[2]);   // row 2
  row += RowFromX(row,s[3],d[3]);   // row 3
  row += RowFromY(row,s[0],d[0]);   // row 4
  row += RowFromX(row,s[1],d[1]);   // row 5
  row += RowFromX(row,s[2],d[2]);   // row 6
  row += RowFromX(row,s[3],d[3]);   // row 7
  //--- Build constant matrix --------------------------------------
  CT[0] = d[0].VT_X;
  CT[1] = d[1].VT_X;
  CT[2] = d[2].VT_X;
  CT[3] = d[3].VT_X;
  CT[4] = d[0].VT_Y;
  CT[5] = d[1].VT_Y;
  CT[6] = d[2].VT_Y;
  CT[7] = d[3].VT_Y;
  //--- solve the system -------------------------------------------
  if (0 == Solve(CM,CT,CX,8)) return 0;
  //--- store result as T matrix -----------------------------------
  T[0]  = CX[0];
  T[1]  = CX[3];
  T[2]  = CX[6];
  T[3]  = CX[1];
  T[4]  = CX[4];
  T[5]  = CX[7];
  T[6]  = CX[2];
  T[7]  = CX[5];
  T[8]  = 1;
  return 1;
}
//===================================================================================
//	Compute intersection of a line and a plane
//	Line is defined by the points PA and PB
//	Plane is defined by the normal vector NP and a point PP
//===================================================================================
void CIntersector::GetLineToPlane()
{	rc	= 0;														// No intersection
	CVector U = *PB - *PA;
	CVector W = *PA -  PP;
	double  N = -NP.DotProduct(W);
	double  D =  NP.DotProduct(U);
	if (fabs(D) < DBL_EPSILON)	return;
	//---- Compute intersection ------------------
	double  S = N / D;
	U.Times(S);
	PR.Sum(*PA,U);
	rc = (S > 0)?(1):(2);
	return;
}
//===================================================================================
/// GreatestCommonDivisor
///
/// Compute the greatest common divisor (GCD) of two integers using Euclid's algorithm
//===================================================================================
int GreatestCommonDivisor (int i, int j)
{
  // Apply Euclid's algorithm to find the GCD
  while (0 != j) {
    int temp = j;
    j = i % j;
    i = temp;
  }
  return i;
}

//======================================================================================
// Visibility test between 4 points on same plan
//  -point P is the origin point
//  -point Q is the extemity of segment PQ
//	-point A is    origin of segment AB
//  -point B is extremity of segment AB
//	We want to check if P and Q are mutually visible, relative to AB. 
//	Does segment AB masks P from Q?
//	-----------------------------------------------------------------
//	AB masks P from Q when both conditions holds
//	1) A and B are not on the same side relative to PQ
//	2) P and Q are not on the same side relative to AB
//	When 1 & 2 are both true, segments PQ and AB intersect so P cant see Q.
//  ------------------------------------------------------------------
//	Same side test is done by considering the dot products of 3 vectors 
//	Example: prod1 = PQ.PA 
//					 prod2 = PQ.PB
//	Each dot product defines a vector perpendicular to the plan. A & B are on same
//	side of PQ if both products has same orientation.   As PQ and AB are planar, we just
//	have to consider the Z coordinate of each dot product (a real number).
//--------------------------------------------------------------------------------------
//			Z(prod1)*Z(prod2) < 0		=>	A and B are not on same side of PQ
//  with
//	P(xp,yp) Q(xq,yq) A(xa,ya) B(xb,yb) we have
//	vector PQ[(xq-xp),(yq-yp)]
//	vector PA[(xa-xp),(ya-yp)]
//	vector PB[(xb-xp),(yb-yp)]
//	Z(prod(PQ,PA)) = (xq-xp)(ya-yp) - (yq-yp)(xa-xp) = z1;
//	Z(prod(PQ,PB)) = (xq-xp)(yb-yp) - (yq-yp)(xb-xp) = z2;
//	thus we just test the sign of z1.z2 
//--------------------------------------------------------------------------------------
//	Special cases:
//	A is on PQ when PQ.PA = vector(0) thus z1 = 0;  So we  cant tell which side is A
//		but we can tell if A is between P and Q
//	Idem for B
//======================================================================================
int GeoTest::SameSide(D2_POINT &P, D2_POINT &Q, D2_POINT &A, D2_POINT &B)
{ double xpq	= Q.x - P.x;
	double ypq  = Q.y - P.y;
	double xpa	= A.x - P.x;
	double ypa  = A.y - P.y;
	double xpb  = B.x - P.x;
	double ypb  = B.y - P.y;
	//--- compute Z coordinate of PQ.PA ----------------
	double z1   = (xpq * ypa) - (ypq * xpa);
	//--- Check if A is colinear with PQ ----------------
	if (fabs(z1) < precision)	
	{	double	xqa	=  (A.x - Q.x);
		double	yqa =  (A.y - Q.y);
	  bool		in	= ((xqa * xpa) < 0)	|| ((yqa * ypa) < 0);
		return (in)?(GEO_INSIDE_PQ):(GEO_OUTSIDE_PQ);
	}	
	double z2   = (xpq * ypb) - (ypq * xpb);
	//--- Check if B is colinear with PQ ----------------
	if (fabs(z2) < precision)	
	{	double	xqb = (B.x - Q.x);
		double	yqb = (B.y - Q.y);
		bool in			= ((xqb	* xpb) < 0)	|| ((yqb * ypb) < 0);
		return	(in)?(GEO_INSIDE_PQ):(GEO_OUTSIDE_PQ);
	}
	//--- Opposite side if z1 * z2 < 0 --------------------
	double pr   = z1 * z2;
  return (pr < 0)?(GEO_OPPOS_SIDE):(GEO_SAME_SIDE);
}
//--------------------------------------------------------------------------------------
//	Check visibility (see above)
//	1)	If either point A or B is inside Segment PQ, then we considere that
//			P is masked from Q by this point
//	2)  If  both A and B are on the same side of segment PQ then P an Q are mutually
//			visible
//	3		 As A and B are on opposite sides (or colinear with PQ, but ouside), we must now 
//			test P and Q positons relative to AB
//--------------------------------------------------------------------------------------
bool GeoTest::VisibilityTest(D2_POINT &P, D2_POINT &Q, D2_POINT &A, D2_POINT &B)
{	int r1 = SameSide(P,Q,A,B);
	if (r1 == GEO_INSIDE_PQ)		return false;
	if (r1 == GEO_SAME_SIDE)		return true;
	//--- A & B are opposed relative to segment PQ ---------
	int r2 = SameSide(A,B,P,Q);
	return (r2 == GEO_OPPOS_SIDE)?(false):(true);
}
//--------------------------------------------------------------------------------------
//	Check side of point A, relative to vector PQ
//	We just need the case where A is strictly on left of PQ
//--------------------------------------------------------------------------------------
int GeoTest::OnLeft(D2_POINT &A, D2_POINT &P, D2_POINT &Q)
{	//--- check the Z coordinate of dot product(PQ,PA) --------
	double xpq = Q.x - P.x;
	double ypq = Q.y - P.y;
	double xpa = A.x - P.x;
	double ypa = A.y - P.y;
	double zp  = (xpq * ypa) - (xpa * ypq);
	//---------------------------------------------------------
	if (fabs(zp) < precision)		return GEO_ON_PQ;
	return (zp > 0)?(GEO_LEFT_PQ):(GEO_RITE_PQ);
}
//--------------------------------------------------------------------------------------
//	Check side of point A, relative to vector PQ
//	We just need the case where A is strictly on left of PQ
//--------------------------------------------------------------------------------------
int GeoTest::Positive(D2_TRIANGLE &T)
{	double xac = T.C->x - T.A->x;
  double yac = T.C->y - T.A->y;
	double xab = T.B->x - T.A->x;
	double yab = T.B->y - T.A->y;
	double zp  = (xac * yab) - (xab *yac);
	return (zp > 0)?(1):(0);
}
//--------------------------------------------------------------------------------------
//	Check if Point A is inside triangle P,Q,R where edges are positively oriented
//	True when A is strictly on left side of the 3 edges
//--------------------------------------------------------------------------------------
int GeoTest::InTriangle(D2_POINT &A, D2_POINT &P, D2_POINT &Q, D2_POINT &R)
{	int in = OnLeft(A,P,Q) & OnLeft(A,Q,R) & OnLeft(A,R,P);
	return in;
}
//--------------------------------------------------------------------------------------
//	Check if Point A is inside triangle P,Q,R where edges are positively oriented
//	True when A is strictly on left side of the 3 edges
//--------------------------------------------------------------------------------------
int GeoTest::InTriangle(D2_POINT &P, D2_TRIANGLE &T)
{	int on = 0;
	int p1 = OnLeft(P,*T.B,*T.A);
	if (p1 == GEO_RITE_PQ)			return GEO_OUTSIDE;
	if (p1 == GEO_ON_PQ)				on++;
	//-----------------------------------------------------
	int p2 = OnLeft(P,*T.A,*T.C);
	if (p2 == GEO_RITE_PQ)			return GEO_OUTSIDE;
	if (p2 == GEO_ON_PQ)				on++;
	//----------------------------------------------------
	int p3 = OnLeft(P,*T.C, *T.B);
	if (p3 == GEO_RITE_PQ)			return GEO_OUTSIDE;
	if (p3 == GEO_ON_PQ)				on++;
	return (on)?(GEO_ON_SIDE):(GEO_INSIDE); 
}
//=======================END OF FILE ======================================================

