/*
 * Gauge Components.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005 Chris Wallace
 * Copyright since 2007 Jean Sabatier
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



#ifndef GAUGE_COMPONENTS_H
#define GAUGE_COMPONENTS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//========== ======================================================================
#include "../Include/FlyLegacy.h"
#include "../Include/3dMath.h"
//================================================================================
class CPanelLight;
class CFmtxMap;
class CMaskImage;
class CSoundBUF;
//=============================================================================
//  HIT parameters
//=============================================================================
struct HIT_GAUGE {
	CGauge *ghit;													// Gauge hit
	short sx;															// X screen 
	short sy;															// Y screen
	short gx;															// Gauge x position in panel
	short gy;															// Gauge y position in panel
	short rx;															// x hit position in gauge
	short ry;															// y hit position in gauge
};
//================================================================================
//	Quad corner
//================================================================================
#define Q_SW		0					// South west
#define Q_SE		1					// South East
#define Q_NW		2					// North west
#define Q_NE    3					// North East
//=============================================================================
//  Gauge Properties
//=============================================================================
typedef enum {
  ROT_TEXTURE = 0x0001,
  NO_SURFACE  = 0x0002,
  IS_DISPLAY  = 0x0004,
	GAUGE_SKIP	= 0x40000000,
} EGaugeProp;
//=============================================================================
//  Type of actions
//=============================================================================
typedef enum {
  ACT_FRAME = 1,
  ACT_MSAGE = 2,
} BaseAction;
//=============================================================================
//  Basic Action
//=============================================================================
struct BASE_ACT {
  char Type;
  int  pm1;
  float Value;
  //--- Constructor ---------------------------------------
  BASE_ACT::BASE_ACT()
  { Type  = 0;
    pm1   = 0;
    Value = 0;
  }
  //--------------------------------------------------------
};
//================================================================================
// CGaugeRectangle
//================================================================================
class CGaugeRectangle : public CStreamObject {
//------ATTRIBUTES ------------------------------------
public:
  char    Type;             // Is defined
  short   x1, y1, x2, y2;   // (x1, y1) top-left, (x2, y2) bottom-right
  //-----------------------------------------------------
public:
  CGaugeRectangle (void);
  virtual ~CGaugeRectangle (void) {}
  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
};

//================================================================================
// CGaugeClickArea
//  Click area has two properties
//  Displayable:    Display Information is allowed 
//  Clickable  :    A click is alloowed
//  The following definitions are used
//  CAnn:   Area nn displayable and clickable (as in FlyII)
//  DAnn:   Area nn is displayable only.  Click are ignored
//  CKnn:   Area nn is clickable but no data are displayed
//
//================================================================================
typedef enum {
  AREA_NONE = 0x00,               // NOTHING SPECIAL
  AREA_SHOW = 0x01,               // Has display
  AREA_CLIK = 0x02,               // Is click only
  AREA_DIGI = 0x04,               // Is a digit
  AREA_BUTN = 0x08,               // Is a button
  AREA_BMAP = 0x10,               // Is a bitmap
  AREA_QUAD = 0x20,               // Must have quad
  AREA_REPT = 0x40,               // Repeated click
  AREA_BOTH = (AREA_SHOW + AREA_CLIK),
} CAreaType;
//================================================================================
class CGaugeClickArea : public CGaugeRectangle {
//-----ATTRIBUTES -----------------------------------------------
  CGauge     *mgg;          // Mother gauge
  TC_VTAB    *quad;         // Quad for texture
  TEXT_DEFN   txd;          // Texture definition
	U_SHORT			vOfs;					// VBO offset
  Tag         cursTag;      // Unique tag for custom cursor
  char        htxt[64];     // Custom popup help
  //-------------------------------------------------------------
public:
  CGaugeClickArea (void);
  virtual ~CGaugeClickArea (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  // CGaugeClickArea methods
  virtual bool  IsHit (int x, int y);
//  virtual void  Draw (SSurface *s, int f) {if (s)  bm->DrawBitmap(s, x1, y1, f); } 
  virtual ECursorResult MouseMoved (int x, int y);
  //-------------------------------------------------------------
	void			CollectVBO(TC_VTAB *t);
  void      Init(CGauge *mg, char tp);
  void      ReadGPOS(SStream *str);
  void      ReadCHRP(SStream *str);
  void      ReadABSP(SStream *str);
  void      ReadPOSN(SStream *str);
	void	    BindCursor(SStream *str);
  void      BuildQuad();
  //--- Copy functions ------------------------------------------
  void      CopyFrom(CGaugeClickArea &s, CGauge *mg);
  void      CopyRECT(CGaugeClickArea &s);
  void      Draw(int fr);
  void      DrawOnSurface(SSurface *s, int f);
  void      Set(short px, short py, short wd, short ht, char type);
  //---------------------------------------------------------------
	inline		U_CHAR	IsOK(U_CHAR m) {return (m & Type);}
  //--------------------------------------------------------------
  inline    bool  NoShow()      {return ((Type & AREA_SHOW) == 0);}
  inline    bool  NoClick()     {return ((Type & AREA_CLIK) == 0);}
  inline    bool  IsRept()      {return ((Type & AREA_REPT) != 0);}
};
//================================================================================
//  Class type action
//================================================================================
class CGaugeAction {
  //--- ATTRIBUTES ---------------------------------------------------
protected:
  char    No;                         // current action
  char   end;                         // Last action
  std::vector<BASE_ACT*> vact;        // List of actions
  //----METHODS ----------------------------------------------------
public:
  CGaugeAction();
 ~CGaugeAction();
  //----------------------------------------------------------------
  bool    DecodeFrame  (char *txt, BASE_ACT *a);
  bool    DecodeMessage(char *txt, BASE_ACT *a);
  void    ReadActions(SStream *s);
  //----------------------------------------------------------------
  inline void Start()     {No = 0; end = vact.size();}
  BASE_ACT *GetAction()   {return (No < end)?(vact[No++]):(0);}
};
//================================================================================
//		Base class for all gauges
//	NOTE:  For quad and tour, the following order is observed
//			0 => NE corner
//			1 => NW corner
//			2 => SW corner					(Origin)
//			3 => SE corner
//	NOTE:  Coordinates in quad are OpenGL oriented:  Y=0 is the screen bottom
//				 Coordinates in tour are panel oriented:   Y=0 is the top of panel
//================================================================================
class CGauge : public CStreamObject {
public:
  CGauge (void);
	CGauge(CPanel *mp); 
  virtual ~CGauge (void);
	void	Init();
	//----------------------------------------------------------
  // CStreamObject methods
	virtual int		Read (SStream *stream, Tag tag);
	virtual void	ReadFinished (void);
	//--------Helper function for derived classes --------------
protected:
  void  ReadLightMask(SStream *str);
  //----------------------------------------------------------
public:
  void  DecodeDisplay(SStream *str);
  void  DecodePOSN(SStream *str, TC_VTAB *qd,char opt);
  void  DecodeCADR(SStream *str, TC_VTAB *qd,char opt);
  void  DecodePROJ(SStream *str, TC_VTAB *qd,char opt);
  void  DecodeNCTR(SStream *str, TC_VTAB *qd,char opt);
	void	DecodeSHAP(SStream *str, S_PIXEL *sp,char opt);
  void  PrepareQuad(TC_VTAB *pan);
	void	BuildQuad(TC_VTAB *qd,float px,float py,float wd,float ht);
	void	BuildQuad(TC_VTAB *pan, S_PIXEL *sp);
  void  RenderSurface(TC_VTAB *tab,SSurface *sf);
  void  OutLine();
  void  SetSurface(int wd,int ht);
	//--------------------------------------------------------------
	void	SetBrightness(float amb);
  //--------------------------------------------------------------
  CGauge *LocateGauge(SStream *s);
  void  CopyFrom(CGauge &g);
  //--------------------------------------------------------------
  virtual   void  DisplayHelp() {DisplayHelp(help);}
	virtual   void  CollectVBO(TC_VTAB *vtb);
  //--------------------------------------------------------------
  void            DisplayHelp(char *txt);
  void            FuiHelp();
  //-------------------Helper for file sound ---------------------
  int GetSoundTag(int p,SStream *stream);
  Tag BindCursor(SStream *stream);
	//--------------------------------------------------------------
  // CGauge methods
  //--------------------------------------------------------------
public:
  virtual void        PrepareMsg(CVehicleObject *veh);				// JSDEV* prepare gauge  messages
  virtual void        Draw()		    {}												// Draw with light
	virtual void				DrawAmbient()	{}											  // Draw in ambiant
	virtual void				DrawDynamic()	{}												// Specific draw 
  virtual void        RenderGauge() {RenderSurface(quad,surf);}
  virtual void        GetCenterOffset(short* _cx, short* _cy);
  virtual SSurface*   GetSurface (void) {return surf;}
  virtual const char *GetClassName (void) { return "CGauge"; }
  virtual void        Update (void);
  //--------------------------------------------------------------
  virtual int         CharWD()  {return 0;}
  virtual int         CharHT()  {return 0;}
  //--------------------------------------------------------------
  virtual bool        AtRatio(char r)       {return false;}
	virtual bool				BtCopy()							{return true;}
	//--------------------------------------------------------------
	virtual U_CHAR      State()								{return 0;}
  //--------------------------------------------------------------
  inline  void        Disarm()              {arm = 0;}
  inline  bool        HasIdent(Tag id)      {return (id == unId);}
  inline  bool        HasMask()             {return (mask != 0);}
  inline  bool        LfHit(int x,int y)    {return (x < cx);}
  inline  bool        UpHit(int x,int y)    {return (y < cy);}
  //-------------------------------------------------------------
  inline  Tag         Cursor()              {return cursTag;}
  inline  char*       hlpTex()              {return help;}
  //-------------------------------------------------------------
  inline  char        IsTextured()          {return Prop.Has(ROT_TEXTURE);}
	inline  bool				IsHit(short x,short y,HIT_GAUGE &sh) 
	{return (this->*vHit)(x,y,sh);}
  //--------------------------------------------------------------
	inline  int					GetXPos()							{return tour[NW_PIX].x;}
	inline  int					GetYPos()							{return tour[NW_PIX].y;}
  inline  short       GetDisXOrg()          {return xOrg;}
  inline  short       GetDisYOrg()          {return yOrg;}
  inline  TC_VTAB    *GetVTAB()             {return quad;}
  inline  TC_VTAB    *GetPROJ()             {return quad;}
  inline  TC_VTAB    *GetCADR()             {return quad;}
  inline  CPanel     *GetPanel()            {return panel;}
	inline  U_SHORT		  ofsVBO()							{return vOfs;}
  //--------------------------------------------------------------
  inline  void        SetPanel(CPanel *p)   {panel = p;}
  inline  char       *GetUniqueID(void)     {return unid_s;}
  inline  Tag         GetID()               {return unId;}
  //--------------------------------------------------------------
  bool                SetLight();
  //-----------mouse functions -----------------------------------
public:
  virtual EClickResult  MouseClick (int x, int y, int bt);
  virtual EClickResult  TrackClick (int x, int y, int buttons);
  virtual EClickResult  StopClick ();
  virtual ECursorResult MouseMoved (int x, int y);
  virtual void          ClickField(int na, short dir) {}
  virtual CGaugeClickArea *GetClickArea(char No) {return 0;}
  //--- Repeat clicks --------------------------------------------
  EClickResult          ArmRepeat(float t,int x, int y, int bt);
  void                  RepeatClick();
  //--------Helpers ----------------------------------------------
  void    GetSize (short* x1, short* y1, short *wd, short* ht);
	void		GetTour(S_PIXEL *t, short *w, short *h);
  int     ClickAreaFromTag (Tag tag,int *cnb);
  int     MessageFromTag(Tag tag);
  int     ActionFromTag(Tag tag);
  void    ClearDisplay();
	U_SHORT	FixRoom(U_SHORT n);
	//---------------------------------------------------------------
	bool		RectHit(short x,short y,HIT_GAUGE &sh);
	bool		TourHit(short x,short y,HIT_GAUGE &hg);
  //---------------------------------------------------------------
  float   ClampTo(float lim,float val)
  { if (val > +lim)   return +lim;
    if (val < -lim)   return -lim;
    return val;
  }
  //-----------ATTRIBUTES ----------------------------------------
protected:
  Tag       type;       // Gauge type
  Tag       unId;       // Unique gauge ID within panel
  char      unid_s[8];  // String version of unique ID tag
  COption   Prop;       // Properties
	U_SHORT		vOfs;				// VBO offset
  TC_VTAB   quad[4];		// Rendering quad
	S_PIXEL   tour[4];		// pixel tour
  short     w,h;        //and (w,h) width, height
  short     cx, cy;     // Center offset
	//------ Vector to hit detector ----------------------------
	typedef bool(CGauge::*HitFN)(short x,short y,HIT_GAUGE &s); 
	HitFN     vHit;       // Vector to hit detector
  //-------Display definition ---------------------------------
  char      arm;        // Armed state
  float     rTime;      // Repeat timer 
  float     iTime;      // Initial value
  int       mx,my,mbt;  // Mouse parameters
  //----  This is the area that is cleared before display -----
  short     dsw;                // Display width
  short     dsh;                // Display height
  short     xOrg;               // Origin in surface
  short     yOrg;               // Origin in surface
  U_INT     yOfs;               // Buffer offset
  //------Gauge parameters ------------------------------------
  SMessage      mesg;       // Message for indication value
  float         gmin, gmax; // Minimum and maximum gauge value
  CFmtxMap     *gmap;       // Mapping table
  CPanelLight  *plit;       // Panel light
  CMaskImage   *mask;       // Light mask
  float         amlit;      // ambient light 
  //-----------------------------------------------------------
  Tag       cursTag;    // Unique tag for custom cursor
  char      help[64];   // help data
  char      dfmt[64];   // Display format
  bool      akbd;       // May be activated from keybord
  //-----Help buffer from FuiManager --------------------------
  char     *hbuf;       // Buffer for help (from fui manager help window)
  //-----------------------------------------------------------
  CPanel   *panel;      // Mother panel
  SSurface *surf;       // Drawing surface for the gauge
  float     value;      // Gauge functional value
  float     vReal;      // Gauge real value
  //-------Sound buffer stack --------------------------------
  CSoundBUF *sbuf[2];    // Sound buffer  
};

//================================================================================
//  Class GPart for  gauge texture with multi frames
//================================================================================
class VStrip: public CStreamObject {
  //--- Attributes ---------------------------------------
	U_SHORT  vOfs;							// VBO offset
  TC_VTAB *vtab;              // Local quad
  TC_VTAB *quad;              // Gauge Quad
  CGauge  *mgg;               // Mother Gauge
  TEXT_DEFN txd;              // Texture definition
  short hiFrame;              //  Highest frame
  //--- METHODS ------------------------------------------
public:
  VStrip::VStrip();
  VStrip::VStrip(CGauge *mg);
 ~VStrip();
  void  Init(CGauge *mg);
  void  LocalQuad(int xp, int yp, int wd, int ht);
  void  ReadPROJ(SStream *s);
  void  ReadPOSN(SStream *s);
  void  ReadStrip(SStream *s);
  int   Read(SStream *str,Tag tag);
	//------------------------------------------------------
  void  CopyFrom(CGauge *mg,VStrip &src);
	void	CollectVBO(TC_VTAB *vtb);
	void	DynaQuad(TC_VTAB *vbo,U_SHORT ofs);
  //------------------------------------------------------
  bool  Draw(int frame);
  void  Draw(TEXT_DEFN *tdf, int fr);
  void  DrawRange(float s0, float s1);
  //------------------------------------------------------
  inline void SetGauge(CGauge *mg)  {mgg = mg;}
  inline void TextureDim(int &x, int &y) {x = txd.wd; y = txd.ht;}
  inline int  FrameHeight()   {return txd.ht;}
  inline int  TextureHeight() {return txd.ht * txd.nf;}
  inline int  NbFrames()      {return txd.nf;}
  inline int  GetHiFrame()    {return hiFrame;}
	inline bool HasTexture()		{return (txd.rgba != 0);}
	};
//===============================================================================
// CRotNeedle
//
// Helper class for many types of gauges which use analog needles for data display
//================================================================================
class CRotNeedle : public CStreamObject {
  friend class CNeedle;
  friend class C_Knob;
  //----ATTRIBUTES  ------------------------------------------  
protected:
	U_SHORT	 vOfs;		// VBO offset
  TC_VTAB *quad;    // Drawing Quad
  TEXT_DEFN txd;    // Texture  definition
  short   offx;     // X offset
  short   offy;     // Y offset
  float   mind;     // Minimum degrees
  float   maxd;     // Maximum degrees
  double  vadj;     // Vertical adjustment
  double  ampl;     // pitch amplitude
  double  incd;     // texture increment per degre
  CVector ptrs;     // Pitch translation
  CVector rotc;     // Rotation center
  //---- METHODS ---------------------------------------------
public:
  CRotNeedle (void);
  virtual ~CRotNeedle (void);
	virtual void	ReadFinished();
  // CStream methods
  // CRotNeedle methods
  void  SetPPD(double ppd);
  void  SetCenter(double dx, double dy);
  void  GetTexture(SStream *str);
  void  Draw(float deg);
  void  DrawFixe();
  void  Draw (float rol, float pit);
	//----------------------------------------------------------
  void  CopyFrom(CGauge *mg,CRotNeedle &src);
	void	CollectVBO(TC_VTAB *vtb);
  void  SetQuad(CGauge *mg);
  //----------------------------------------------------------
  inline void SetVADJ(double a)   {vadj = a;}
	//----------------------------------------------------------
	inline U_SHORT ofsVBO()		{return vOfs;}
	//----------------------------------------------------------
	inline void   SetVTAB(TC_VTAB *t)	{quad = t;}
	inline void   SetVBO (U_SHORT n)	{vOfs = n;}
};
//===============================================================================
// CDigit
//
//================================================================================
class CDigit: public CStreamObject {
  //--- ATTRIBUTES ------------------------------------------
protected:
  char       type;                  // Digit type
  CGauge    *mgg;                   // Mother gauge
  TEXT_DEFN *txd;                   // Texture
	U_SHORT    vOfs;									// VBO offset
  TC_VTAB    quad[4];               // Quad
  //---------------------------------------------------------
public:
  CDigit::CDigit();
  int   Read(SStream *str, Tag tag);
	void	CollectVBO(TC_VTAB *vtb);
  //---------------------------------------------------------
  void  SetGauge(CGauge *mg);
  void  Draw(int fr);
  void  DrawPartial(float pr);
  //---------------------------------------------------------
  inline char GetType()                 {return type;}
  inline void SetTexture(TEXT_DEFN *t)  {txd = t;}
};
//==========================================================================
// CNeedle:  individual rotating needle
//==========================================================================
class CNeedle : public CStreamObject {
  //----------ATTRIBUTES ---------------------------------------------------
protected:
  CGauge       *mgg;                              // Mother gauge
  TC_VTAB      *vtab;                             // Needle Quad
	U_SHORT       vOfs;															// VBO offset
  SMessage      mesg;                             // Message
   Tag          cTag;                             // Unique tag for custom cursor
  char          help[64];                         // help data
  //------------------------------------------------------------------------
  float         ampv;                             // vertical Amplitude (degre)
  float         amph;                             // Horizontal amplitude
  float         sang;                             // Starting angle (degrees)
  float         gmin;                             // Minimum value
  float         gmax;                             // Maximum value
  CFmtxMap     *gmap;                             // Mapping table
  float         dunt;                             // Degrees per unit
  //---------------------------------------------------------------------
  CRotNeedle    rotn;                             // Rotating needle
  //-----working area ---------------------------------------------------
  float         value;
  //----------METHODS ---------------------------------------------------
public:
  CNeedle(CGauge *m);
  CNeedle();
  virtual ~CNeedle(void);
  // CStreamObject methods
  virtual const char* GetClassName    (void) { return "CNeedleGauge"; }
  void        Init(CGauge *g);
  void        SetGauge(CGauge *mg);
  int         Read (SStream *stream, Tag tag);
  void        ReadFinished();
  void        ReadROTC(SStream *s);
  void        ReadPOSN(SStream *s);
  void        ReadPROJ(SStream *s);
  void        ReadNCTR(SStream *s);
  void        ReadBMAP(SStream *s);
  int         Clamp(float v);
	//---------------------------------------------------------------------
  void        CopyFrom(CGauge *mg,CNeedle &src);
	void				CollectVBO(TC_VTAB *vtb);
  //---------------------------------------------------------------------
  void        Draw();
  //--------------------------------------------------------------------
  void        DrawNeedle(float deg);
	void        DrawNeedle(float rol,float pit) {rotn.Draw(rol,pit);}
  //---------------------------------------------------------------------
  inline void   SetPPD(double p)    {rotn.SetPPD(p);}
  inline float  GetAMPV()           {return ampv;}
  inline float  GetAMPH()           {return amph;}
  //---------------------------------------------------------------------
  inline Tag    Cursor()            {return cTag;}
  inline char*  hlpTex()            {return help;}
	//---------------------------------------------------------------------
	};

//================================================================================
//  Class C_Knob for rotating knob
//================================================================================
//--------------------------------------------------------------------------------
class C_Knob : public CStreamObject {
private:
  //---Attributes ----------------------------------------
	U_CHAR	CState;		      // Internal state
  U_CHAR  CType;          // Clamp or degre
  U_CHAR  ownS;           // owned surface
  U_CHAR  fixe;           // reserved
  //------------------------------------------------------
  CRotNeedle  rotn;       // Knob texture
  //-- Position ------------------------------------------
  short	  x1;			        // X offset
	short   y1;			        // Y offset
	short	  x2;			        // X2
	short	  y2;			        // Y2
  short   wd;             // Width
  short   ht;             // Height
  //---Mother gauge ---------------------------------------
  CGauge *mgg;            // Mother gauge
  //-------------------------------------------------------
  Tag   cTag;             // Unique tag for custom cursor
  char  help[64];         // Custom popup help
  //-------------------------------------------------------
  short   vRot;           // actual value
  short   iRot;           // Rotation increment
	float		Timer;		      // timer
	float		maxTM;		      // max time before accelaration
  //-----Floating value knob -------------------------------
  float   Incr;           // Increment
  float   Change;         // Changing increment
	U_SHORT  vOfs;					// VBO offset
  TC_VTAB  quad[4];
  //-----Methods -------------------------------------------
public:
  C_Knob();                // default Constructor
  C_Knob(CGauge *mg);      // For new operator
  virtual  ~C_Knob();       // Destructor
  //---------------------------------------------------------
  int		Read(SStream *stream, Tag tag);
  void  ReadFinished(void);
  void  ReadPOSN(SStream *str);
  void  ReadPROJ(SStream *str);
	void  SetGauge(CGauge *g);
  void  InitQuad();
	void	CollectVBO(TC_VTAB *vtb);
  //---------------------------------------------------------
  void  Init(CGauge *g);
  bool  HasChanged();
  void  Draw();
  //-----Floating values management -------------------------
  void  SetRange(float v1,float v2,float vi);
  EClickResult  ArmRotation(float adj,int x,int y,int btn);
  //------------------------------------------------------
  inline bool   IsHit(int x,int y)  {return ((x >= x1) && (x <= x2) && (y >= y1) && (y <= y2)); }
  inline bool   NoKnob()            {return ((x1 + x2) == 0);}
  inline float  GetChange()         {return Change;}
  inline bool   Update()            {return HasChanged();}
  //--------------------------------------------------------
  inline EClickResult  DisarmKnob() {CState = 0; Change = 0; return MOUSE_TRACKING_OFF;}
  inline TC_VTAB   *GetVTAB()       {return quad;}
  //-------Mouse management -------------------------------
  ECursorResult     MouseMoved (int x, int y);
};

//==========================================================================
// C_Cover for underlays and overlays
//==========================================================================
class C_Cover: public CStreamObject {
public:
  //--- ATTRIBUTES ----------------------------------------
  CGauge   *mgg;                  // Mother gauge
  TEXT_DEFN txd;                  // Texture definition
	U_SHORT   vOfs;									// VBO offset
  TC_VTAB  *vtab;
  TC_VTAB  *quad;                 // Quad
  //--- METHODS -------------------------------------------
  C_Cover();
 ~C_Cover();
  //-------------------------------------------------------
  void  Init(SStream *str,CGauge *mg);
  void  ReadSize(SStream *str);
  int   Read(SStream *str, Tag tag);
	//-------------------------------------------------------
  void  CopyFrom(CGauge *mg,C_Cover &src);
	void	CollectVBO(TC_VTAB *vtb);
	void	DynaQuad(TC_VTAB *vbo,U_SHORT ofs);
  //-------------------------------------------------------
  void  Draw();
  //-------------------------------------------------------
  void SetGauge(CGauge *mg);  
};
//=======================================================================
//  Class C_TextGauge for image texture
//  This is a basic gauge that supports
//  -One needle
//  -One underlay
//  -One overlay
//  It may be used as far as only one needle is used
//=======================================================================
class CTexturedGauge: public CGauge {
  //---- ATTRIBUTES ---------------------------------------
protected:
  C_Cover       under;        // Underlay Texture info
  C_Cover       overl;        // Overlay  Texture info
  CNeedle       nedl;         // Needle
  //---- METHODS ------------------------------------------
public:
  CTexturedGauge(CPanel *mp);
 ~CTexturedGauge();
  virtual int   Read(SStream *str, Tag tag);
  virtual void  RenderGauge() {;}
  virtual void  Draw();
	virtual void	CollectVBO(TC_VTAB *vtb);
  //-------------------------------------------------------
  void  CopyFrom(CTexturedGauge &src);
  //------------------------------------------------------
  void  ReadLayer(SStream *str,TEXT_DEFN &txd);
  void  ShowHelp(char *fmt, ...);
  //------------------------------------------------------
  inline void  DrawUnderlay()   {under.Draw();}
  inline void  DrawOverlay()    {overl.Draw();}
};
//==========================================================================
// CgHolder	:  Holder for gauge values
//==========================================================================
class CgHolder {
public:
	//--- ATTRIBUTES -------------------------------------
	Tag		idn;											// Ident
	int		pm1;											// Parameters 1
	int		pm2;											// Parameters 2
	float	fm1;											// Float n°1
	float	fm2;											// Float n°2
	//--- METHODS ----------------------------------------
public:
	CgHolder(Tag t);
 ~CgHolder();
};
//==========================================================================
// CNeedleMark:  Provide a manual operated needle
//==========================================================================
class CNeedleMark: public CStreamObject {
protected:
  //--- ATTRIBUTES ---------------------------------------
  CGauge   *mgg;              // Mother gauge
  CNeedle   nedl;             // Needle
  C_Knob    knob;             // Knob;
	CgHolder *hold;							// Value holder
  //------------------------------------------------------
  float    dmin;              // Minimum value
  float    dmax;              // Maximum value
  float    dval;              // Actual value
	//------------------------------------------------------
	char       pos;							// Holder position
  //------------------------------------------------------
public:
  CNeedleMark();
  //------------------------------------------------------
  int   ReadKNOB(SStream *stream);
  int   Read(SStream *s,Tag tag);
  void  SetGauge(CGauge *mg);
	void	SetHolder(CgHolder *s,char p);
	void	SaveValue(float v);
	//------------------------------------------------------
  void  CopyFrom(CGauge *mg,CNeedleMark &src);
	void	CollectVBO(TC_VTAB *vtb);
  //--- Mouse interface ----------------------------------
  ECursorResult		MouseMoved(int x, int y);
  EClickResult    MouseClick(int x, int y, int btn);
  EClickResult    StopClick();
  //--- Drawing ------------------------------------------
  void  Draw();
  //------------------------------------------------------
	inline float    GetValue()				{return dval;}
	inline void			SetValue(float v)	{dval = v;}
};
//================================================================================
//		Base class for a gauge textured quad
//================================================================================
class CgQUAD : public CStreamObject {
protected:
	//--- ATTRIBUTES ---------------------------------
	CGauge   *mgg;										// Mother gauge
	TC_VTAB		quad[4];								// Basic quad
	U_SHORT   vOfs;										// VBO offset
	TEXT_DEFN	txdf;										// Texture definition
	//--- METHOD -------------------------------------
public:
	CgQUAD();
	int		Read(SStream *s,Tag t);
	void	DecodeRDIM(char *txt);
	void	Draw(float dx, float dy);
	//------------------------------------------------
	void  SetGauge(CGauge *mg);
	void	CollectVBO(TC_VTAB *vtb);
};
//==========================================================================
// CGripGauge provide a grip to scroll the panel:  This is not really agauge
//  as it provides just a mouse interface
//==========================================================================
class CGripGauge: public CGauge {
  //---ATTRIBUTE --------------------------------
  char  grip;                 // Panel is moving
  char  rfu;
  int   px;                   // Previous X
  int   py;                   // previous Y
  //--- Methods ---------------------------------
public:
  CGripGauge(CPanel *mp);
	//--- Draw method -----------------------------
	void	Draw()	{;}
  //--- Mouse interface -------------------------
  EClickResult  MouseClick(int x, int y, int buttons);
  EClickResult  StopClick();
  EClickResult  TrackClick(int x, int y, int bDown);

};


//=========================== END OF FILE ======================================
#endif GAUGE_COMPONENTS_H
