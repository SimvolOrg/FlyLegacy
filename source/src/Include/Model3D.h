/*
 * MODEL3D.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
 * Copyright 2008      Jean Sabatier
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
#ifndef MODEL3D_H
#define MODEL3D_H
#include "../Include/Globals.h"
#include "../Include/Queues.h"
#include "../Include/3dMath.h"
#include "../Include/LightSystem.h"
//===================================================================================
#define TC_SNAP_GROUND    (0x00000001)         // Snap to ground
#define TC_SHARE_PLACE    (0x40000000)         // Share place 
///----------------------------------------------------------------------------
//  NOTES ON COORDINATE SYSTEM
//  All coordinates (pavement and runways) are expressed with the following rules:
//  In the files, for any vertex V(x,y,z) the coordinates x,y and z are all in feet, 
//  relatives to the AIRPORT origin O(x,y,z)
//  1) X and Y are transformed in arcsec from world origin.
//    For any vertex V(x,y,z) yhe new coordinates X,Y and Z are
//    X = ARCSEC_PER_FEET(x * cp) + O(x);
//    Y = ARCSEC_PER_FEET(y)      + O(y);
//    Z = z;
//  2) Distances along X are inflated with a compensation factor (cp) that depends on the 
//    latitude. This is because when going toward the pole, the tile Y dimension
//    (in arcsec) is shrinking.  At rendering time, X and Y  dimensions are translated
//    in feet by different scaling factor to make a square tile of side Y.  In this
//    process, the X dimension is reduced to the Y dimension.  The inflate factor
//    just ensures that distances are correctly set. As all coordinate are relative 
//    to the airport origin, the x value of a vertex is the vertex distance to
//    airport origin.
//=============================================================================
class CPolyShop;
class OSM_Object;
class CShared3DTex;
//=============================================================================
#define MODEL_DNT			0x02
#define MODEL_NIT     0x01
#define MODEL_DAY     0x00
#define MODEL_HI      0x00
#define MODEL_LO      0x01
//--------------------------------------------------------
#define SUP3D_OUTSIDE 0           // Outside of drawing
#define SUP3D_LOD_OSM 1						// Loading OSM objects 
#define SUP3D_FADE_IN 2           // coming in view
#define SUP3D_VIEWING 3           // Full view
#define SUP3D_FADEOUT 4           // Fading out
#define SUP3D_INS_OSM 5						// Inside OSM ring
//============================================================================
//  Structure for part info
//============================================================================
struct M3D_PART_INFO {
  int          NbVT;                            // Number of vertices
  int          NbIN;                            // Number of indices
  F3_VERTEX   *nVTX;                            // List of vertices
  F3_VERTEX   *nNRM;                            // Norme coordinates
  F2_COORD    *nTEX;                            // Texture coordinates
  int         *nIND;                            // Indice list
  void        *ref;                             // Texture reference
  //------------------------------------------------------------------
	U_CHAR       mode;
  U_CHAR       tsp;                             // Transparency
  char        *ntex;                            // Texture name
};
//============================================================================
//  GLOBAL ARCHITECTURE FOR 3D WORLD
//  C3DMgr:   There is one instance of 3D manager that maintains a global list of C3Dmodel
//            where a C3Dmodel is a set of C3Dpart coming either from a SMF or a BIN file
//  C3Dworld: There is one C3Dworld per QGT.  It maintains a list of CWobj decoded in scenery
//            file.  Each CWobj references in turn one or more C3Dmodels from the C3DMgr.
//
//            This is supposed to gain time and place because complex airports include
//            several references to the same SMF or BIN part. For instance a lamp post model
//            may be used about 100 hundred times in the same scenery.
//=============================================================================
//  Ground INFO
//=============================================================================
struct TC_GRND_INFO {
  C_QGT       *qgt;               // Quarter global Tile
  short         tx;               // Tile X component
  short         tz;               // Tile Z component
  CSuperTile   *sup;               // Super Tile
  double       alt;
};
//=============================================================================
//  Picture QUAD
//  provide a square associaated to a bitmap for drawing
//
//=============================================================================
class CPicQUAD {
  //-----Attributes ---------------------------------------------------
  U_INT     xOBJ;                   // Texture Object
  CPolyREC  quad;                   // Rectangle quad
  //-----Methods ------------------------------------------------------
public:
  CPicQUAD();
 ~CPicQUAD();
  //-------------------------------------------------------------------
 void GetBitmap(char *bn);
 void SetElevation(float e);
 void Draw();
};

//============================================================================
//  Class Shdw to decode shadow data
//============================================================================
class CShdw: public CStreamObject {
  //-----ATTRIBUTE --------------------------------------
  Tag   ter;
  //-----------------------------------------------------
public:
  CShdw() {ter = 0;}
  int   Read(SStream *st,Tag tag);
  //---inline ------------------------------------------
};

//=============================================================================
//  3D Part:  Describe a 3D object part coming from any file format
//      (bin,smf,etc)
//  NOTE:
//  The C3Dpart  canonical format is
//    A shared vertex list    (V0,V1,...,Vn) where Vk=(x,y,z)
//    A shared normal list    (N0,N1,...,Nn) where Nk=(m,n,p)
//    A shared tex coord list (T0,T1,...,Tn) where Tk=(s,t)
//    A List of indices in above array (I0,I1,.. Ip)
//  
//=============================================================================
class C3DPart {
	friend class Queue<C3DPart>;
	//--- Define render vector -----------------------------
	typedef void (C3DPart::*VREND)(void);	// Rending vector
	char			idn[8];
  //---ATTRIBUTES ----------------------------------------------
	C3DPart  *next;														// Next part
	C3DPart  *prev;														// Previous part
	//-------------------------------------------------------------
	VREND			Rend;														// Rending vector
	//------------------------------------------------------------
  U_CHAR    tsp;                            // Transparent indicator
  U_CHAR    lod;                            // Level of detail
	U_CHAR    nzb;														// No depth test
	U_CHAR    rf2;														// not used
	//------------------------------------------------------------
  CShared3DTex *tRef;                       // Texture Reference
	
  //---List of components --------------------------------------
	U_SHORT   vloc;														// Vertice Relocation
	U_SHORT   xloc;														// Indice rlocation
  int       NbVT;                           // Number of vertices
  int       NbIN;                           // Number of indices
  F3_VERTEX   *nVTX;                        // List of vertices
	GN_VTAB     *gTAB;												// Vertice table
  F3_VERTEX   *nNRM;                        // Norme coordinates
  F2_COORD    *nTEX;                        // Texture coordinates
  int         *nIND;                        // Indice list
  //------------------------------------------------------------
  int       total;                          // Total faces
  //------------------------------------------------------------
public:
  C3DPart();
	C3DPart(char dir, char *txn, int lod, int nbv, int nbx);
	C3DPart(char dir, char *txn,int lq,int nbx);
 ~C3DPart();
	//-------------------------------------------------------------
	void		Release();
	void		Trace(char lod);
	//-------------------------------------------------------------
	void		ZRotation(double sn, double cn);
	void    ExtendTNV(int nbv, int nbx);
	void		BinRendering();
	void		W3DRendering();
	int 		Translate(SVector &T);
	void		Append(TC_VTAB *tab, U_INT ofs, U_INT lg);
	void		ExtendOSM(int nbv,GN_VTAB *src);
	void		AllocateIND();
	void		ExtendGTB(int nbv);
	void		SQLstrip(int nbx,F3_VERTEX *V,F3_VERTEX *N,F2_COORD *T,int *X);
	//-------------------------------------------------------------
	void		SetTexture(U_CHAR t, char *txn);
	//-------------------------------------------------------------
	void		AllocateW3dVTX(int nv);
	void		AllocateOsmGVT(int nv);
	void		AllocateOsmLIT(int nv);
	void		DrawAsW3D();
	void		DrawAsOSM();
	void		DrawAsBIN();
	void		DrawAsLIT();
	//-------------------------------------------------------------
	void		Draw()		{(this->*Rend)();}
	//-------------------------------------------------------------
  void    SetAllTexName   (char dir,char *txn);
  void    GetInfo(M3D_PART_INFO &inf);
	bool		SameTexture(char dir, char *txn);
	char   *TextureName();
	//---------------------------------------------------------------
	CShared3DTex   *GetTREF()		{return tRef;}
	CShared3DTex   *GetReference(TEXT_INFO &txd);
	void	 Reserve(CShared3DTex *ref);
  //-------------------------------------------------------------
	inline bool     NoZB()						{return (nzb != 0);}
	inline bool     SameREF(void *r)	{return (tRef == r);}
  inline bool     IsEmpty()   {return (0 == NbVT) || (0 == NbIN);}
  inline void     ReduceIndice(int k) {if (k < NbIN) NbIN = k;}
  //---------------------------------------------------------------
	inline void			ResetNBVT()				{NbVT = 0;}	
  inline void     SetLOD(int k)			{lod  = k;}
  inline void     AddFace(int nf)		{total += nf;}
	//---------------------------------------------------------------
	inline void     SetTREF(CShared3DTex *r)	{tRef = r;}
	inline void			SetGTAB(int n, GN_VTAB *t){gTAB = t; NbVT = n;}
  //----------------------------------------------------------------
	inline GN_VTAB    *GetGTAB()						{return gTAB;}
  inline F3_VERTEX  *GetVLIST()           {return nVTX;}
  inline F3_VERTEX  *GetNLIST()           {return nNRM;}
  inline F2_COORD   *GetTLIST()           {return nTEX;}
  inline int        *GetXLIST()           {return nIND;}
  inline int				 GetNBVTX()           {return NbVT;}
  inline int				 GetNbIND()           {return NbIN;}
  inline U_CHAR      GetTSP()             {return tsp;}
	inline C3DPart    *Next()								{return next;}
	inline F3_VERTEX  *VTX(int n)						{return nVTX + n;}
	inline F3_VERTEX  *NVX(int n)						{return nNRM + n;}
	inline F2_COORD   *NTX(int n)						{return nTEX + n;}
	//----------------------------------------------------------------
	void			MoveVTX(void *s,int k);
  void			MoveNRM(void *s,int k);
  void			MoveTEX(void *s,int k);
  void			MoveIND(void *s,int k);
	//-----------------------------------------------------------------
	void			Push(F3_VERTEX &v, F3_VERTEX &n, F2_COORD &t);
	//----------------------------------------------------------------
  inline int     *AllocateXList(int n)  {NbIN = n; nIND = new int [n]; return nIND;} 
  inline int      GetNbFace()           {return total;}
  //-----------------------------------------------------------------
  inline void     SetVTX(int k,F3_VERTEX  *p)  {nVTX[k] = *p;}
  inline void     SetNRM(int k,F3_VERTEX  *p)  {nNRM[k] = *p;}
  inline void     SetTEX(int k,F2_COORD  *t)   {nTEX[k] = *t;}
  inline void     SetIND(int k,int x)     {nIND[k] = x;}
  inline void     SetTSP(U_CHAR m)        {tsp = m;}
	inline void			SetLOD(U_CHAR l)				{lod = l;}
	inline void     SetZB(char z)						{nzb = z;}
	inline void			SetNext(C3DPart *n)			{next = n;}
	//------------------------------------------------------------------
  inline int      GetLOD()                {return lod;}
	//------------------------------------------------------------------
	VREND						GetRendering()					{return Rend; }	
};
//=========================================================================================
#define M3D_SMF 1
#define M3D_BIN 2
#define M3D_OBJ 3
//=========================================================================================
#define M3D_INIT      0
#define M3D_INLOAD    1
#define M3D_LOADED    2
#define M3D_EMPTY			3
#define M3D_ERROR     4
//====================================================================================
//	Object type
//====================================================================================
#define OSM_OBJECT (0)
#define W3D_OBJECT (1)

//=========================================================================================
//  Class C3Dmodel to hold a model from a SMF or BIN file
//  The 3D model key is formed by
//  -The ident Tag
//  -The File Name
//  Example:  <norm>MAINSFO.SMF
//=========================================================================================
class C3Dmodel: public CqItem, public CDrawByCamera {
  //---Attributes -------------------------------------------------------
  char					state;              // Model state
  char					type;               // BIN or SMF
	char					mdtr;								// Trace mode
	char          todm;								// day or night
  pthread_mutex_t		mux;            // Locker
  int          User;                // User count
  char        *fname;               // day file name
  CVector      exts;                // Extension
  //---viewing parameters -----------------------------------------------
  float         aBot;               // Ground elevation
  float         aTop;               // Top elevation
  //----Rendering parameters --------------------------------------------
  U_INT        rDIR;                // Rendering direction
  //----Stack of parts for 4 levels of details (LOD) --------------------
  U_CHAR						rLOD[4];        // Redirection table
	Queue<C3DPart>  	pLOD[4];        // List of parts
  //---------------------------------------------------------------------
public:
  C3Dmodel(char *fn,char t);
 ~C3Dmodel();
 //----------------------------------------------------------------------
  C3DPart *PopPart()								{return pLOD[0].Pop();}
  C3DPart *PopPart(int k)						{return pLOD[k].Pop();}
  //---------------------------------------------------------------------
	Queue<C3DPart>  &GetQueue(int k)	{return pLOD[k];	}
	//---------------------------------------------------------------------
	double  MaxExtend();
  int     LoadPart(char *dir);
  int     AddPodPart(C3DPart *prt);
  void    AddLodPart(C3DPart *prt,int nl);
  void    GetParts(CPolyShop *psh,M3D_PART_INFO &inf);
  void    GetParts(CExport   *exp,M3D_PART_INFO &inf);
  void    Counts(M3D_PART_INFO &tt);
  int     Draw(int lod);
  void    DecUser();
  void    IncUser();
	void		Finalize();
	void		TracePart(char lod);
	//---------------------------------------------------------------------
	C3DPart *GetPartFor(char dir,char *txn, int lod, int nbv, int nbx);
	C3DPart *GetNewPart(char dir,char *txn, int lod, int nbv, int nbx);

	C3DPart *GetPartFor(char dir,char *txn, int lod, int nbx);
	C3DPart *GetNewPart(char dir,char *txn, int lod, int nbx);
	//-----For camera Draw ------------------------------------------------
  void    PreDraw(CCamera *cam);
  void    CamDraw(CCamera *cam) {Draw(0);}
  void    EndDraw(CCamera *cam);
	//--- for debugging ---------------------------------------------------
	bool    AreYou(char *s) {	return (strcmp(s,fname) == 0);}
  //---------------------------------------------------------------------
  inline bool  NeedLoad()         {return (0 == state);}
  inline void  SaveTop(float t)   {if (t > aTop)  aTop = t;}
  inline void  SaveBot(float t)   {if (t < aBot)  aBot = t;}
  //---------------------------------------------------------------------
  inline U_INT  GetRDIR()         {return rDIR;}
  inline char  *GetFileName()     {return fname;}
  inline float  GetGround()       {return aBot;}
  inline float  GetTop()          {return aTop;}
  inline void   SetTop(float t)   {aTop = t;}
  inline void   SetGround(float g){aBot = g;}
  inline void   SetDirection(U_INT d) {rDIR = d;}
	inline void		SetTOD(char k)		{todm = k;}
	inline char   GetTOD()					{return todm;}
	//----------------------------------------------------------------------
  inline void   SetState(char s)  {state = s;}
  inline bool   IsOK()            {return (state == M3D_LOADED);}
	inline bool   InError()					{return (state != M3D_LOADED);}
	inline bool   IsEmpty()					{return (state == M3D_EMPTY);}
  //----------------------------------------------------------------------
  inline void     SetExtend(CVector l,CVector u)  {exts = u; exts.Subtract(l);}
  inline double   GetXExtend()     {return exts.x;}
  inline double   GetYExtend()     {return exts.y;}
  inline double   GetZExtend()     {return exts.z;}
};
//=========================================================================================
//  CLASS C3DmodQ
//        Class to collect C3Dmodels
//=========================================================================================
class CModQ : public CQueue {
  //---------------------------------------------------------------------
public:
  inline void Lock()                {pthread_mutex_lock (&mux);}
  inline void UnLock()              {pthread_mutex_unlock (&mux);}
  //----------------------------------------------------------------------
  ~CModQ();                        // Destructor
  //----------------------------------------------------------------------
  inline void PutEnd(C3Dmodel *mod)          {CQueue::PutEnd(mod);}
  inline C3Dmodel *Pop()                     {return (C3Dmodel*)CQueue::Pop();}
  inline C3Dmodel *GetFirst()                {return (C3Dmodel*)CQueue::GetFirst();}
  inline C3Dmodel *GetNext(C3Dmodel *mod)    {return (C3Dmodel*)CQueue::GetNext(mod);}
};

//=============================================================================
// Model manager
//=============================================================================
class CKmm: public CStreamObject {
  //----Attributes --------------------------------------
  CWobj    *wobj;                         // World object
  U_CHAR    tod;                          // Day or Night
  C3Dmodel *modl;                         // Model
  //---Methods -------------------------------------------
public:
  CKmm(CWobj *obj,U_CHAR tod);
  int   Read(SStream *st,Tag tag);
  int   ReadModel(SStream *st);
  void  SetMdl (const char *model);

  //------------------------------------------------------
};
//============================================================================
//  Class wobj to decode a 3D object
//  This class is the base class for al 3D objects
//============================================================================
class CWobj: public CqItem, public CStreamObject {
  friend class C3DMgr;
  friend class C3Dfile;
protected:
  //----------------------------------------------------------
  pthread_mutex_t		mux;					    // Mutex for lock
  U_INT       type;										// Object type
  U_INT       User;                   // Number of users
  //----ATTRIBUTES -------------------------------------------
  C3Dworld   *wd3D;                   // Parent system
  CObjPtr     pmOB;                   // User pointer 
  C3DLight   *Lite;                   // Associated light
  TC_GRND_INFO inf;                   // Object ground info
  //----------------------------------------------------------
  Tag         kind;                   // Object category
  U_INT       flag;                   // FLAG
  //---REFERENCE QUEUES (one for Day, one for Night) ----------
  C3Dmodel   *modL[2];                // Day and nigth modesl
  //---OBJECT LOCATION ----------------------------------------
  SPosition   oPos;                   // World position (lat,lon,alt)
  SVector     oAng;                   // Orientation
  //------------------------------------------------------------
  float       pDis;                   // Distance to plane
  //-----------------------------------------------------------
	U_CHAR     obtr;									  // Trace
	U_CHAR     roty;										// rotation
	U_CHAR		 snap;										// Snap to ground
  U_CHAR     nozb;                    // No Z Buffer
  U_CHAR     nozu;                    // No Z underlay
  U_CHAR     count;                   // Place holder count
  U_CHAR     cand;                    // candidate count
  U_CHAR     shar;                    // shared room
  int        fspd;                    // Flash speed
  //-----------------------------------------------------------
  char      *fnam;                    // File name
  char      *name;                    // 3D object name
  char      *desc;                    // Description
	//-----------------------------------------------------------
public:
  CWobj(Tag t);
 ~CWobj();
  void      FreeLites();
  void      Warning(char *err);
  bool      Localize(C_QGT *qgt);
	void      Relocate(C3Dmodel *mod);
 //-----------------------------------------------------------
  virtual   int   Read(SStream *st,Tag tag);
  virtual   void  ReleaseOBJ() {}
  //----------------------------------------------------------
  void            Check();
  void            DecUser();
  void            IncUser()     {User++;}
	void						TraceMe();
  //----------------------------------------------------------
  void  EditModel(CFuiCanva *c,char *n, M3D_PART_INFO &inf);
	void	EditPos(char *t1,char *t2);				// edit object position
  //----------------------------------------------------------
  bool            NoPlace();
  int             ReleaseMe();
  void            SetFileName(char *fn);
  void            Trace(U_CHAR tq, char *tod);
  void            SetDayRef(char *fn);
  void            SetNitRef(char *fn);
	void            GetInfo(CFuiCanva *cnv);
  void            SetObjName(char *n);
  void            SetObjDesc(char *d);
	void						SetOrientation(SVector &v);
	//-----------------------------------------------------------
  char           *ModelName(U_CHAR rq);
  //-------Return absolute coordinates as integers ------------
  U_INT           GetIntLongitude();
  U_INT           GetIntLatitude();
  //----------------------------------------------------------
  void            PushLight(C3DLight *obj);
  C3DLight       *PopALight();
	void						SetFlag(U_INT f);
  //----------------------------------------------------------
  void            AddModel(C3Dmodel *mod,U_CHAR q);
  int             LoadMyModel();
  double          GetXExtend();
  double          GetYExtend();
  double          GetZExtend();
  //----------------------------------------------------------
  char           *DayName();
  char           *NitName();
  //----------------------------------------------------------
  float           RefreshDistance();
  void            UpdateWith(CNavaid *nav);
  void            EndOfQGT(C_QGT *qt);
   //---------UPDATE IF ANIMATED -----------------------------
  virtual  void   ModelLoaded(C3Dmodel *m) {}
  virtual  void   Update(U_CHAR rfq) {}
  virtual  int    DrawModel(char tod,char lod);
	//--- for debugging ----------------------------------------
	bool	AreYou(char *s)	{return modL[0]->AreYou(s);}
  //----------------------------------------------------------
  inline void     SetPosition(SPosition &p) {oPos = p;}
  //----------------------------------------------------------
  inline Tag      GetKind()               {return kind;}
  inline U_INT    GetFlag()               {return flag;}
  inline U_CHAR   GetZB()                 {return nozb;}
  inline U_CHAR   GetZU()                 {return nozu;}
  //-----Inline ------------------------------------------------
  inline C3Dmodel  *GetDayModel()         {return modL[MODEL_DAY];}
	inline C3Dmodel  *GetNitModel()					{return modL[MODEL_NIT];}
	inline C3Dmodel  *GetTodModel(char t)		{return modL[t];}
	//----Various functions --------------------------------------
  inline SPosition *ObjPosition()         {return &oPos;}
  inline SVector   *ObjOrientation()      {return &oAng;}
  inline char      *GetName()             {return name;}
  inline double     GetElevation()        {return oPos.alt;}
  inline char      *GetHold()             {return desc;}
  //------------------------------------------------------------
  inline int        GetPM1()              {return count;}
  inline int        GetPM2()              {return 0;}
  //------------------------------------------------------------
  inline void       SetPM1(int p)         {count = p;}
  inline void       SetPM2(int p)         {}
  //------------------------------------------------------------
  inline bool       NoSup()               {return (inf.sup == 0);}
  inline bool       HasLight()            {return (Lite != 0);}
  inline bool       NoZB()                {return (nozb != 0);}
  inline bool       NoZU()                {return (nozu != 0);}
  inline bool       IsaHold()             {return (kind == 'hold');}
  inline bool       NoHold()              {return (shar == 0);}
  inline bool       IsaShare()            {return (shar != 0);}
	inline char       Rotate()							{return roty;}
  //-------------------------------------------------------------
	inline U_INT			GetType()						  {return type;}
  inline void       SetParameter(CmHead *obj) {pmOB = obj;}
  inline CmHead    *GetUserParam()        {return pmOB.Pointer();}
  inline char      *GetFileOBJ()          {return fnam;}
  inline char      *GetDescription()      {return desc;}
  inline float      GetPDIST()            {return pDis;}
  inline double     GetYRotation()        {return oAng.y;}
	inline U_INT			GetUsers()						{return User;}
	//-------------------------------------------------------------
  inline C_QGT     *GetQGT()    {return inf.qgt;}
  //-------------------------------------------------------------
	inline void				SetType(U_INT t)			{type		= t;}
  inline void       SetDistance(float d)  {pDis = d;}
  inline void       SetParent(C3Dworld *w){wd3D = w;}
  inline C3DLight  *GetLight()            {return Lite;}
  inline void       SetNOZB(char f)       {nozb |= f;}
  inline void       SetNOZU(char f)       {nozu |= f;}
	inline void				SetSnap()							{snap = 1;}
	inline void				SetTrace(char t)			{obtr = t;}
	//--------------------------------------------------------------
	inline void SnapTo()	{snap  = (flag & TC_SNAP_GROUND);}
  //--------------------------------------------------------------
  inline void						Copy(TC_GRND_INFO &t) {inf = t;}
  inline TC_GRND_INFO  *GetINFO()         {return &inf;}
  inline SPosition			GetPosition()     {return oPos;}
	inline CSuperTile    *GetSuperTile()		{return inf.sup;}
  //--------------------------------------------------------------
};
//=============================================================================
//  Class CGvew  to decode ground vehicle
//=============================================================================
class CGvew: public CWobj {
  char  nfo[128];                         // NFO file
  Tag     vmod;                           // Mode 
  bool    lofi;                           // Low fidelity
  SVector bvel;                           // Velocity
  SVector bang;                           // angular velocity
  //----------------------------------------------------------
public:
  CGvew::CGvew(Tag k);
  int   Read(SStream *st,Tag tag);
};
//=============================================================================
//  Class CWsok  to support animated wind sock
//  This class support a list of references to models
//  Each model is a different position of the sock depending on wind force
//=============================================================================
class CWsok: public CWobj {
  //----------------------------------------------------------
public:
  CWsok::CWsok(Tag k);
  int   Read(SStream *st,Tag tag);
  void  Update(U_CHAR rfq);
  int   DrawModel(char tod,char lod);
};
//=============================================================================
//  Class CWvor  to support a VOR object
//=============================================================================
class CWvor: public CWobj {
  //----NO specifics attributs -------------------------------
  //----Methods ----------------------------------------------
public:
  CWvor(Tag kind);
  //----------------------------------------------------------
  void  ReleaseOBJ();
};
//=============================================================================
//  Class CWhld  to support a place holder object
//  Place holder is a geographical position holding one object from a list
//  The object is randomly selected at loading time.
//  For instance, a parking could be a place holder for one aircraft. The 
//  displayed aircarft will be randomly selected from the list
//=============================================================================
class CWhld: public CWobj {
  //----Specific attributes ------------------------------------------
  std::vector<CWobj*> Hold;               // List of availables object
  //-------------------------------------------------------------------
public:
  CWhld::CWhld(Tag t): CWobj(t){};
 ~CWhld();
  //--------------------------------------------------------------------
  void   Add(CWobj *o) {Hold.push_back(o);}
  CWobj *Select();
};
//=========================================================================================
//  CLASS CObjQ
//        Class to collect World objects
//=========================================================================================
class CObjQ : public CQueue {
  //---------------------------------------------------------------------
public:
  inline void Lock()                {pthread_mutex_lock (&mux);}
  inline void UnLock()              {pthread_mutex_unlock (&mux);}
  //----------------------------------------------------------------------
  ~CObjQ();                         // Destructor
  //----------------------------------------------------------------------
  inline void PutEnd(CWobj *obj)             {CQueue::PutEnd(obj);}
  inline CWobj   *Pop()                      {return (CWobj*)CQueue::Pop();}
  inline CWobj   *GetFirst()                 {return (CWobj*)CQueue::GetFirst();}
  inline CWobj   *GetLast()                  {return (CWobj*)CQueue::GetLast();}
  inline CWobj   *GetNext(CWobj *obj)        {return (CWobj*)CQueue::GetNext(obj);}
  inline CWobj   *Detach (CWobj *obj)        {return (CWobj*)CQueue::Detach(obj);}
};
//============================================================================
//  Class C3Dfile   This class reads 3Dscenery files appearing in QGT
//============================================================================
class C3Dfile: public CStreamObject {
  //-----ATTRIBUTES -------------------------------------------
  C_QGT     *oQGT;                    // Object's QGT 
  CObjQ      exQ;                     // Object queue for export
  CWhld     *hld;                     // Last place holder found
	U_INT			cntr;											// Add object count
	U_INT			serial;										// Serial number
  char       namef[64];               // File name
  char       fullN[PATH_MAX];					// Current file ident
	char       exp;											// Export indicator
	char			 rfu;											// Reserved
  //-----------------------------------------------------------
public:
  C3Dfile(C_QGT *qgt,char x);     // Constructor
 ~C3Dfile();
  void  Abort(Tag tag);
  void  Abort(char *msg);
  int   Decode(char *fname,char *pn);
  int   Read(SStream *st,Tag tag);
  bool  MarkHold(CWobj *obj);
  //-----------------------------------------------------------
  inline CWobj *GetWOBJ()							{return exQ.Pop();}
	inline C_QGT *GetQGT()							{return oQGT;}
	inline int    GetQLG()							{return exQ.NbObjects();}
	///------------------------------------------------------------
  inline bool   ForExport()           {return (exp != 0);}
  };
//=============================================================================
//  Class Dummy to eliminate all uneeded tag and values
//=============================================================================
class C3Ddummy: public CStreamObject {
  //--No ATTRIBUTE -----------------------------------------
public:
  C3Ddummy()  {}
  int Read(SStream *st,Tag tag);
  };

//=============================================================================
//  Class C3Dworld for managing QGT 3D objects per QGT
//  NOTE:   Due to the repetition of references to existing SMF or BIN file
//          that describes a given 3D object, a global Cache of 3D object is
//          generated.   The C3Dworld located in each QGT maintains a list of
//          pointers to those object.
//         
//=============================================================================
class C3Dworld {
  friend class C_QGT;
protected:
  //---ATTRIBUTES ------------------------------------------------
  U_CHAR          tr;                         // Trace option
	//--------------------------------------------------------------
  CObjQ           woQ;                        // QGT world objects
  C_QGT          *qgt;                        // Mother QGT
  CFmt1Map       *wmap;                       // Wind map for windsock
  std::map<std::string,CWobj*>  hold;         // Place holder
  //----Distance to collect 3D parts------------------------------
  float           nColor[4];                  // Night color
  //------Number of Object in SuperTiles --------------------------
  int             nOBJ;                       // Total objects
  //------Longitude compensation ----------------------------------
  double          rFactor;                    // latitude reduction
  double          cFactor;                    // Latitude compensation
  //--------------------------------------------------------------
  GLUquadricObj  *qdr;
  //--------------------------------------------------------------
public:
  C3Dworld();
 ~C3Dworld();
  //--------------------------------------------------------------
  void    AddToWOBJ(CWobj *obj);
  void    SetQGTparameters(C_QGT *q);
  void    TraceEnd();
	void		TraceAdd(int n);
  void    Clear3Dworld();
  void    GetLine(CListBox *box);
  CWobj  *SelectOneOf(CWobj *hld);
  float   GetSockForce();
  //--------------------------------------------------------------
  int     AssignToSuperTile(CWobj *obj);
  //--------------------------------------------------------------
  void    Check();
  void    TimeSlice(float dT);
  void    Draw(U_CHAR tod);
  //---DEBUG PURPOSE ---------------------------------------------
  //  void  InitQUAD(C3DPart *prt);
  //--------------------------------------------------------------
  inline double     GetInflation()        {return cFactor;}
  inline double     GetReduction()        {return rFactor;}
  inline int        GetNOBJ()             {return nOBJ;}
  //--------------------------------------------------------------
  inline C_QGT*     GetQGT (void)         {return qgt;}
	inline int				GetNwoQ()							{return woQ.NbObjects();}
};
//=============================================================================
//  Class C3DMgr  This class manages the world 3D objects for a QGT
//  -3D object enters and leaves cache following the life of a QGT
//=============================================================================
class C3DMgr  {
  //----ATTRIBUTES ---------------------------------------------
  CDbCacheMgr *dbm;
  std::map<std::string,C3Dmodel *> mapMOD;
  U_CHAR     tr;                      // Trace indicator
	U_CHAR     lpod;										// Look also in pod
	U_CHAR		  sql;										// SQL database
	U_INT			 serial;									// serial number
  TCacheMGR *tcm;                     // Terrain cache
  C_QGT     *qgt;                     // Current QGT
	//----------------------------------------------------------
	pthread_mutex_t		mux;					    // Mutex for lock
  //------------VOR OBJECT -----------------------------------
  CWobj     *oVOR;                    // VOR Object
  float      gVOR;                    // VOR ground
  //---------GLOBAL LIGHT QUEUE --------------------------------
  CLitQ      litQ;                    // Light queue for objects
  //---------MODEL LOADING QUEUE -------------------------------
  CModQ      modQ;                    // Model Queue
  //----For test -----------------------------------------------
  GLUquadricObj *sphere;
  //------------------------------------------------------------
public:
  C3DMgr(TCacheMGR *t);    // Constructor
 ~C3DMgr();
  void      TraceCount();
  void      LocateVOR();
	int 			LocateObjects(C_QGT *qgt);
	int				LoadFromPod(C3Dfile &scf);
	void      NoTexture(char *fn,char *tn);
  void      Warning(CWobj *obj,char *msg);
	void			TraceLoad(int nb,char *src,C_QGT *qgt);
  //------------------------------------------------------------
  C3Dmodel *AllocateModel(char *fn);
  void      FreeModelKey(char *key);
  int       LoadTheModel(C3Dmodel *mod);
  C3Dmodel *ModelToLoad();
	U_INT			modQsize();
	//------------------------------------------------------------
  void      LightToDraw(C3DLight *f);
  void      CreateVOR();
  //------------------------------------------------------------
  void      TimeSlice(float dT);
  void      Draw(char tod);
  void      DrawObjectLights();
  void      SetDrawingState();
  //------------------------------------------------------------
  inline    void      ReleaseVOR()            {oVOR->ReleaseOBJ();}
  inline    void      EndOfQGT(C_QGT *qt)     {oVOR->EndOfQGT(qt);}
  //------------------------------------------------------------
  inline    C_QGT    *GetQGT()                {return qgt;}
  inline    bool      IfTrace()               {return (tr != 0);}
  inline    GLUquadricObj *GetSphere()        {return sphere;}
  //------Statistical data -------------------------------------
  void      GetStats(CFuiCanva *cnv);
};
//============================END OF FILE =================================================
#endif  // MODEL3D_H
