/*
 * Fui.h
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

/*! \file Fui.h
 *  \brief Header for Fly! UI manager and widget classes
 *
 * FUI - Fly! UI
 *
 * This UI wrapper library implements the Fly! 2 UI widget hierarchy independently
 *   of the underlying windowing library
 */
 
#ifndef FUI_H
#define FUI_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Include/FlyLegacy.h"
#include "../Include/Utility.h"
#include "../Include/database.h"
#include "../Include/taxiway.h"

#include <list>
#include <vector>
#include <stack>
#include <map>
#include <string>
#include <stdio.h>

extern void heapdump(char *);
//===============================================================================================================
//---------Window properties -----------------------------------------
//===============================================================================================================
#define FUI_TRANSPARENT         0x0001        // Transparent windows
#define FUI_VT_RESIZING         0x0002        // Vertical resizing
#define FUI_HZ_RESIZING         0x0004        // Horizontal resizing
#define FUI_XY_RESIZING         0x0006        // Both resizing
#define FUI_VT_RELOCATE         0x0008        // Relocate verticaly on resize
#define FUI_HZ_RELOCATE         0x0010        // Relocate horizontaly on resize
#define FUI_NO_MOUSE            0x0020        // No mouse sensisility
#define FUI_EDIT_MODE           0x0040        // EDIT PERMITTED
#define FUI_HAS_QUAD            0x0080        // Window has panel
#define FUI_IS_VISIBLE          0x0100        // Window is visible
#define FUI_IS_ENABLE           0x0200        // Window is enabled
#define FUI_UPPER_CASE          0x0400        // text in upper case
#define FUI_NO_BORDER						0x0800				// No border
#define FUI_IS_LOCKED           0x1000        // Locked component
#define FUI_REPEAT_BT						0x2000				// Repeat click
#define FUI_BLINK_TXT						0x4000				// Blink text
//===============================================================================================================
//	Window size
//===============================================================================================================
#define WINDOW_SIZE_MINI  0
#define WINDOW_SIZE_NORM  1
#define WINDOW_SIZE_MAXI  2
//===============================================================================================================
//	File search parameters
//===============================================================================================================
struct FILE_SEARCH {
	char		close;												// With close button
	char		sbdir;												// Look in subdirectory
	char    userp;												// User parameter
	char   *text;													// Text to  display
	char	 *dir;													// Directory name
	char	 *pat;													// File pattern
	char   *sfil;													// Selection
	char   *sdir;													// Found directory
};
//===============================================================================================================
// The following enum defines unique window IDs for all window types.  It is used
//   as a means for applications to avoid using the same ID for different window types
//
//===============================================================================================================
enum EFuiWindowIdentifier
{
  // The following window types are supported in the Alpha build
  FUI_WINDOW_SITUATION_LOAD          = 'Load',
  FUI_WINDOW_MANAGE_DLL              = 'DLLs',
  FUI_WINDOW_QUIT                    = 'Quit',
	FUI_WINDOW_ERMSG									 = 'erms',
  FUI_WINDOW_FRAME_RATE              = 'fpsw',
  FUI_WINDOW_OPTIONS_VIDEO           = 'VidO',
  FUI_WINDOW_OPTIONS_AUDIO           = 'SndO',
  FUI_WINDOW_OPTIONS_DATE_TIME       = 'date',
  FUI_WINDOW_OPTIONS_KEYS_BUTTONS    = 'mkey',
  FUI_WINDOW_OPTIONS_SETUP_AXES      = 'joys',
  FUI_WINDOW_OPTIONS_SCENERY         = 'tree',
  FUI_WINDOW_OPTIONS_STARTUP         = 'gogo',
  FUI_WINDOW_TELEPORT                = 'goto',
  FUI_WINDOW_WAYPOINT_DIRECTORY      = 'wdir',
  FUI_WINDOW_TELEPORT_ALERT          = 'alrt',
  FUI_WINDOW_VEHICLE_SELECT          = 'sela',
  FUI_WINDOW_VEHICLE_INFO            = 'vinf',
  FUI_WINDOW_VEHICLE_FUEL            = 'fuel',
  FUI_WINDOW_VEHICLE_CG              = 'vhcg',
  FUI_WINDOW_VEHICLE_LOAD            = 'vlod',
  FUI_WINDOW_VEHICLE_OPTIONS         = 'vopt',
  FUI_WINDOW_GLOBAL_OPTIONS          = 'gopt',
	FUI_WINDOW_CAMERA_CONTROL          = 'ccam',
  FUI_WINDOW_AXIS                    = 'axis',
  FUI_WINDOW_CHART                   = 'chrt',
  FUI_WINDOW_PLOT                    = 'plot',
  FUI_WINDOW_VECTOR_MAP              = 'vect',
  FUI_WINDOW_WEATHER_ALMANAC         = 'almn',
  FUI_WINDOW_WEATHER_WINDS           = 'wind',
  FUI_WINDOW_WEATHER_CLOUDS          = 'clds',
  FUI_WINDOW_WEATHER_SKY_TWEAKER     = 'skyc',
  FUI_WINDOW_WEATHER_OVERVIEW        = 'envr',
  FUI_WINDOW_ALERT_MSG               = 'apmb',
  FUI_NAVIGATION_LOG                 = 'navl',
  FUI_WINDOW_FPLAN_LOG               = 'flog',
  FUI_WINDOW_FPLAN_LIST              = 'fpls',
  FUI_WINDOW_CHECKLIST               = 'chek',
  FUI_WINDOW_KLN89                   = 'gk89',
  FUI_WINDOW_PROBE                   = 'prob',
  FUI_WINDOW_CAGING                  = 'cage',
  FUI_WINDOW_PIDTUNE                 = 'pidt',
  FUI_WINDOW_DETAILS_NAVAID          = 'airD',
  FUI_WINDOW_DETAILS_AIRPORT         = 'airP',
  FUI_WINDOW_FPDETAIL_AIRPORT        = 'fapt',
  FUI_WINDOW_DETAILS_WAYPOINT        = 'wptD',
  FUI_WINDOW_DEBUG_SIM               = 'sdbg',
  FUI_WINDOW_DISPLAY_DBG             = 'dspd',
  FUI_WINDOW_DEBUG_WINGS             = 'wdbg',
  FUI_WINDOW_HELP_ABOUT              = 'habt',
  FUI_WINDOW_STRIP                   = 'strp',
  FUI_WINDOW_TBROS                   = 'tbro',
	FUI_WINDOW_TEDITOR								 = 'wted',
  FUI_WINDOW_MBROS                   = 'mbro',
	FUI_WINDOW_SKETCH									 = 'skch',
	FUI_WINDOW_FILE_BOX							   = 'fbox',
  FUI_WINDOW_STATS                   = 'Stat',
  FUI_WINDOW_DEFAULT                 = '?win',
	FUI_WINDOW_OSM_TUNE								 = 'osmT',
/*
  // The following window types are reserved for future implementation
  FUI_WINDOW_SITUATION_SAVE          = 'Save',
  FUI_WINDOW_SITUATION_SAVE_AS       = 'SavA',
  FUI_WINDOW_OPTIONS_REALISM         = 'real',
  FUI_WINDOW_OPTIONS_COCKPIT         = 'cock',
  FUI_WINDOW_ADVENTURES              = 'fnow',
  FUI_WINDOW_QUICKFLIGHT             = 'gogo', // Duplicate?
  FUI_WINDOW_AIRCRAFT_OPTIONS        = 'aplo',
  FUI_WINDOW_AIRCRAFT_CG             = 'cgin',
  FUI_WINDOW_AIRCRAFT_AUTOTUNE       = 'tune',
  FUI_WINDOW_AIRCRAFT_WEIGHT_BALANCE = 'phat',
  FUI_WINDOW_AIRCRAFT_DAMAGE         = 'ouch',
  FUI_WINDOW_WEATHER_METAR           = 'meta',
  FUI_WINDOW_WEATHER_OTHER           = 'env+',
  FUI_WINDOW_INSTANT_REPLAY          = 'insR',
  FUI_FLIGHT_PLANNER                 = 'fpln'
*/
};


enum EFuiComponentTypes
{
  // The following components are supported in the Alpha build
  COMPONENT_WINDOW                   = 'wind',
  COMPONENT_BUTTON                   = 'butn',
  COMPONENT_LABEL                    = 'labl',
  COMPONENT_CHECKBOX                 = 'chkb', // Was 'chek' in SDK ??
  COMPONENT_RADIOBUTTON              = 'radb', // Was 'radi' in SDK ??
  COMPONENT_WINDOW_MENUBAR           = 'mbar',
  COMPONENT_SCROLLBAR                = 'scrl',
  COMPONENT_SCROLLBTN                = 'scrb',
  COMPONENT_LIST                     = 'list',
  COMPONENT_SCROLLAREA               = 'scla',
  COMPONENT_TEXTFIELD                = 'edit', // Was 'txtf' in SDK ??
  COMPONENT_MAP                      = 'map_',
  COMPONENT_VECTOR_MAP               = 'vmap',
  COMPONENT_GRAPH                    = 'grph',
  COMPONENT_DLLVIEW                  = 'DLLV',
  COMPONENT_CLOSE_BUTTON             = 'clos',
  COMPONENT_MINIMIZE_BUTTON          = 'mini',
  COMPONENT_ZOOM_BUTTON              = 'zoom',
  COMPONENT_POPUPMENU                = 'popu',
  COMPONENT_RUNWAY_GRAPH             = 'llcv', // For Detail Airport window only
  COMPONENT_MENUBAR                  = 'menu',
  COMPONENT_LINE                     = 'line',
  COMPONENT_BOX                      = 'box_',
  COMPONENT_PICTURE                  = 'pict',
  COMPONENT_SLIDER                   = 'slid',
  COMPONENT_GROUPBOX                 = 'grbx', // Was 'gbox' in SDK ??

  // The following components are reserved for future implementation
  COMPONENT_DOUBLE_BUTTON            = 'dbtn',
  COMPONENT_DEFAULT_BUTTON           = 'okbn',
  COMPONENT_GAUGE                    = 'gage',
  COMPONENT_TEXTAREA                 = 'txta',
  COMPONENT_PROGRESS                 = 'prog',
  COMPONENT_TABPAGE                  = 'ntab',
  COMPONENT_TABBUTTON                = 'tabb',
  COMPONENT_TABCONTROL               = 'tabs',
//  COMPONENT_EDIT =  'edit',
  COMPONENT_MENU                     = 'mnu2',
  COMPONENT_DISCLOSURE               = 'disc',
  COMPONENT_CANVAS                   = 'canv',
  COMPONENT_RUNWAY_CANVAS            = 'rcvs',
  COMPONENT_MARQUEE                  = 'mrqe',
  COMPONENT_HORIZ_MARQUEE            = 'hmrq',
  COMPONENT_WINDOW_NORESIZE          = 'winn',
  COMPONENT_TITLE                    = 'titl',
  COMPONENT_PALETTE_WINDOW           = 'pwin',
  COMPONENT_PALETTE_WINDOW_TITLE     = 'wint',
  COMPONENT_PAGE                     = 'page'
};

enum EFuiEvents
{
  EVENT_NONE          = 0,
  EVENT_MINIMIZE      = 'mini',
  EVENT_ZOOM          = 'zoom',
  EVENT_IDCHANGED     = 'dtid',
  EVENT_HIDDEN        = 'hide',  
  EVENT_SHOWN         = 'show',
  EVENT_ENABLED       = 'enbl',  
  EVENT_DISABLED      = '!enb',
  EVENT_RESIZED       = 'size',
  EVENT_MOVED         = 'move',
  EVENT_RENAMED       = 'name',
  EVENT_COMPONENTDIED = 'dead',
  EVENT_CHECKED       = 'chek', 
  EVENT_UNCHECKED     = 'uchk',
  EVENT_SELECTITEM    = 'slct',
  EVENT_DBLE_CLICK    = 'dclk',
  EVENT_BUTTONPRESSED = 'actn',
  EVENT_BUTTONRELEASE = 'btup',
  EVENT_TEXTCHANGED   = 'delt',
  EVENT_TEXTSET       = 'tset',
  EVENT_VALUECHANGED  = 'valu',
  EVENT_OPENWINDOW    ='open',
  EVENT_CLOSEWINDOW   ='clos',
  EVENT_TITLECHANGED  ='titl',
  EVENT_FOCUSLOST     = '!foc',
  EVENT_FOCUSGAINED   = 'foci',
  EVENT_WIDGETTYPECHANGED = 'widg',
  EVENT_DIALOGDISMISSED = 'dlog',
  EVENT_DIALOGDISMISSEDOK = 'dlOK',
  EVENT_DIALOGDISMISSEDCANCEL = 'dlCn',
  EVENT_UPDATE        = 'updt',
  EVENT_TEXTENTER     = 'tent',   // WAS NOT DOCUMENTED IN SDK ! => for CFuiTextField
  EVENT_NEXTLINE      = 'nlin',   // JSDEV* for scroll bar
  EVENT_PREVLINE      = 'plin',   // JSDEV* for scroll bar
  EVENT_NEXTCOLN      = 'ncol',   // JSDEV* for scroll bar
  EVENT_PREVCOLN      = 'pcol',   // JSDEV* for scroll bar
  EVENT_NEXTPAGE      = 'npag',   // JSDEV* for scroll bar
  EVENT_PREVPAGE      = 'ppag',   // JSDEV* for scroll bar
  EVENT_NEXTPCOL      = 'npcl',   // JSDEV* for scroll bar
  EVENT_PREVPCOL      = 'ppcl',   // JSDEV* for scroll bar
  EVENT_THBRATIO      = 'trat',   // JSDEV* for scroll bar
  EVENT_ANYSUBEVENT   = 0,    
  EVENT_NOSUBEVENT    = 0,
  EVENT_DIR_CLOSE     = 'dlos',   // Window directory close
  EVENT_POP_CLICK    = 'pclk',    // Line clicked in a Popup
	EVENT_DLG_YES			 = '_yes',		// Yes button clicked
	EVENT_DLG_NO       = '_no_',		// No button
};

enum EFuiBinding {
  BIND_H_LEFT       = 1<<0,
  BIND_H_RIGHT      = 1<<1,
  BIND_H_CENTER     = 1<<2,
  BIND_V_TOP        = 1<<4,
  BIND_V_BOTTOM     = 1<<5,
  BIND_V_CENTER     = 1<<6
};

enum EFuiJustify {
  JUSTIFY_H_LEFT      = 1<<0,
  JUSTIFY_H_RIGHT     = 1<<1,
  JUSTIFY_H_CENTER    = 1<<2,
  JUSTIFY_V_TOP       = 1<<4,
  JUSTIFY_V_BOTTOM    = 1<<5,
  JUSTIFY_V_CENTER    = 1<<6
};

enum EFuiLayer {
  LAYER_NORMAL = 0,
  LAYER_FLOATING = 1,
  LAYER_MODAL = 2,
  LAYER_FRONTMOST = 3
};

typedef enum {
  FUI_WINDOW_INIT,
  FUI_WINDOW_OPEN,
  FUI_WINDOW_CLOSING,
  FUI_WINDOW_CLOSED,
  FUI_WINDOW_MOVE,
	FUI_WINDOW_MODAL,
	FUI_WINDOW_DRAW
} EFuiWindowState;
//===================================================================================
// Forward declare all class types
//===================================================================================

//==================================================================================
//
// Type definition for FUI event callback.  Applications must provide a pointer
//   to a function of this type to handle callbacks for windows that they create.
//
typedef void(*FuiEventNoticeCb)(Tag window,
                                Tag component,
                                EFuiEvents event,
                                EFuiEvents subevent);
//==================================================================================
//  Structure for a floating menu
//==================================================================================
#define FLOAT_MENU_DIM 16
typedef struct {  Tag        Ident;                     // Menu ident
                  U_SHORT    NbLin;                     // Number of lines
                  U_SHORT    NbCar;                     // number of char wide
                  char     **aText;                     // Array of pointers to texts
									void     **aItem;											// Array of corresponding items
} FL_MENU;
//==================================================================================
// JSDEV* Base class for all FUI components
//==================================================================================
class CFuiComponent : public CStreamObject
{ protected:
  //-------------Define FBOX index ----------------------------------------
  enum FBOX { BAKW = 0,
              TOPW = 1,
              BOTW = 2,
              LEFW = 3,
              RIGW = 4,
              TLFW = 5,
              TRGW = 6,
              BLFW = 7,
              BRGW = 8,
              TITL = 9,
              ZOOM = 10,
              MINI = 11,
              CLOS = 12,
              MSIZ = 13,                      // Full size
              RSIZ =  9,                      // Minimum size
  };
 //-----------SCROLL STRUCTURE ---------------------------------------
  typedef struct {  CBitmap *bmap;            // Bitmap
                    short    rx;              // Relative x position
                    short    ry;              // Relative y position
                    short    wd;              // Wide
                    short    ht;              // Height
                    char     frm;             // frame
  } SCBOX;
  //----------Resizing code --------------------------------------------
  enum WZ_SIZE {  WZ_NONE = 0,
                  WZ_MX00 = 1,
                  WZ_SX00 = 2,
                  WZ_MY00 = 3,
                  WZ_SY00 = 4,
                  WZ_MXSY = 5,
                  WZ_SXMY = 6,
                  WZ_MXMY = 7,
                 
                  
  };
  //--------------------------------------------------------------------
public:
  // Constructors / destructor
  CFuiComponent (int x = 0, int y = 0, int w = 0, int h = 0, CFuiComponent *win = 0);
  virtual ~CFuiComponent (void);

  // CStreamObject methods
  virtual int         Read (SStream *stream, Tag tag);
  virtual void        ReadFinished (void);
  // CFuiComponent methods ----------------------------------------------------
  inline bool         IsType (EFuiComponentTypes type) { return (this->type == type); }
  inline Tag          GetId (void)    {return id; }
  inline void         Hide()          {RazProperty(FUI_IS_VISIBLE);}
  inline void         Show()          {SetProperty(FUI_IS_VISIBLE);}
  inline char*        GetText (void)  {return text; }
  inline int          GetXpos(void)   {return x;}
  inline void         UpperCase()     {upper = (0xFF ^ 0x20);}
  //-------------------------------------------------------------------
  virtual void        MoveWindow(int mx,int my){};
  virtual void        MoveParentTo (int xp, int yp);
  virtual void        MoveTo(int xs,int ys);
  virtual void        MoveBy(int dx,int dy);
	virtual void				SetTransparentMode(Tag idn) {;}
	//-------------------------------------------------------------------
  virtual void        Close() {}
  virtual void        SetText(char* text);
  virtual void        EditText(char *fmt, ...){}
	virtual void				EditText();
  virtual void        SetEditMode(U_CHAR md){}
  virtual void        SetPosition (int x, int y);
  virtual bool        KeyboardInput(U_INT key){return false;};
  virtual void        FindThemeWidget (void);
  virtual CFuiThemeWidget*  FindThemeWidget (Tag wid, const char *name);
  virtual void        SetFont (Tag fontTag);
  virtual void        Edit(){}
	virtual void				ClearModal()		{;}
	virtual void				FileSelected(FILE_SEARCH *pm)  {;}
	//--------Mouse management ------------------------------------------
  virtual int         NoMouse() {return (prop & FUI_NO_MOUSE);}
  virtual bool        MouseHit (int x, int y);
  virtual bool        MouseMove (int x, int y);
  virtual bool        MouseClick (int x, int y, EMouseButton button);
  virtual bool        DoubleClick(int x, int y, EMouseButton button);
  virtual bool        MouseStopClick (int x, int y, EMouseButton button);
  virtual bool        StopClickInside(int mx,int my, EMouseButton button);
  virtual void        NotifyMenuEvent(Tag idm, Tag itm);
  virtual void        NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn) {return; }
  virtual void        EventNotify(Tag win,Tag cpn,EFuiEvents evn,EFuiEvents sub ){};
  virtual void        NotifyFromPopup(Tag id, Tag itm, EFuiEvents evn){}
	virtual bool				MoveOver(int x, int y)			{return false;}
	virtual bool				ForResize()									{return false;}
	virtual void				SetResize(Tag c,char t)     {}
  virtual void        Resize(short dx,short dy)		{}
  virtual void        ResizeTo(short dx,short dy) {}
  virtual void        ResizeVT(short dy)					{}
  virtual void        Redim(short dx,short dy)		{}
  virtual void        Format(WZ_SIZE cd,short dx,short dy){}
	virtual void				ClickOver(int mx, int my)		{}
	//--------------------------------------------------------------------
  virtual CFuiComponent *GetComponent(Tag id) {return 0;}
  //-------Focus management --------------------------------------------
  bool                RegisterFocus(CFuiComponent *comp);
  void                ClearFocus(CFuiComponent *comp);
  virtual void        RegisterPopup(CFuiPage *pge) {};
  virtual void        FocusLost() {return; };
  //------Clip management ----------------------------------------------
  void                ClipChild();
  //--------------------------------------------------------------------
  int                 CheckWD(int nx);
  int                 CheckHT(int ny);
  void                Show (bool vs);
  int                 Width()  {return w;};
  int                 Height() {return h;};
  void                GetPosition(int *xp,int *yp) {*xp = x; *yp = y;}
  //-----HBox management -----------------------------------------
  int                 CreateHSide(char *name,CFuiPicture **pic,int px,int wd = 0); 
  CFuiPicture        *CreateHBoxPicture (const char* name, int w = 0,int h=0);
  int                 CreateHBox(CFuiPicture *box[],int xw);
  void                SetHBoxFrame(CFuiPicture *box[],int No);
  void                DrawHBox(CFuiPicture *box[]);
  void                ColorInside(U_INT cr);
  void                SetBox(SCBOX *box, char *bname);      // Set scroll box
  //----Full box management ---------------------------------------
  void                InitFBox(CFuiComponent *box[],int size);
  void                CreateFBox(CFuiComponent *pwin,CFuiComponent *box[],int x, int y,int wd, int ht);
  CFuiPicture*        CreateFBoxSide(CFuiComponent *pwin,char * name,int wp, int hp, int *wr, int *hr);
  void                DrawFBox(CFuiComponent *box[],int size);
  void                EditFBox(CFuiComponent *box[],int siz);
  void                RedimFBox(CFuiComponent *box[],short dx, short dy);
	CBitmap*						HaveBitmap(char *fn,char *own);
	void								RemoveBack(CFuiComponent *cp);
	void								SetTransparentMode();
  //-------------Other methods -----------------------------------
  void                TiledBand (CBitmap *bm,SSurface *sf,int y0,int y1);
  //------Drawing ------------------------------------------------
  virtual void        Draw (void);
  void                Draw(SSurface *sf);
  void                DrawAsQuad(U_INT obj);
	//--- Text handling --------------------------------------------
	void								ColorText(U_INT rgb);
  //----inline method --------------------------------------------
	inline void     DrawFBoxBack(CFuiComponent *cp)	{if (cp) cp->Draw();}
  inline void     SetProperty(U_INT p)          {prop |= p;}
  inline void     RazProperty(U_INT p)          {prop &= (-1 - p);}
  inline void     SwapProperty(U_INT p)         {prop ^= p;}
  inline U_INT    HasProperty(U_INT p)          {return (prop & p);}
	inline bool     IsOpaque() {return (0 == (prop & FUI_TRANSPARENT));}
	//----------------------------------------------------------------
  inline void     SetId(Tag idn)                {id = idn;}
  inline void     SetTheme(CFuiThemeWidget *th) {tw = th;}
  inline void     SetHeight(int ht)             {h  = ht;}
  inline int      GetHeight(void)               {return h; }
  inline void     SetWidth(int wt)              {w  = wt;}
  inline int      GetWidth(void)                {return w; }
  inline int      GetXPosition(void)            {return x; }
  inline int      GetYPosition(void)            {return y; }
  inline int      GetXparent(void)              {return xParent;}
  inline int      GetYparent(void)              {return yParent;}
  inline char*    GetWdName(void)               {return wName;}
  inline int      GetXscreen(void)              {return surface->xScreen;}
  inline int      GetYscreen(void)              {return surface->yScreen;}
  inline void     ChangeFont(SFont *nf)         {font = nf;}
  inline CBitmap* GetBitmap(void)               {return 0; }
  inline SSurface *GetSurface()                 {return surface;}
	//-------------------------------------------------------------
  inline void			SetColour (U_INT c)						{colText = c;} 

protected:
  void             MakeSurface ();
  void             InitQuad();
	void						 SetQuad(int wd,int ht);
  //--------------------------------------------------------------
protected:
  char                desi[6];          ///< Designator (Tag)
	std::list<CFuiComponent*> decorationList; ///< Decoration components
  EFuiComponentTypes  type;             ///< Component type
  Tag                 id;               ///< Unique identifier
  int                 bind;             ///< Parent binding flags
  char               *wName;						///< Widget name
  Tag                 widgetTag;        ///< Widget tag
  char                text[256];        ///< Component label
  int                 xParent, yParent; ///< Absolute location of parent origin
  short               x, y;             ///< Location relative to parent
  short               w, h;             ///< Width, height in pixels
  short               halfW;            ///< Half width
  short               halfH;            ///< Half height
  Tag                 fontTag;          ///< Theme definition
  SFont               *font;            ///< Font identifier
  CFont               *fnts;            ///< Real font
  CFuiThemeWidget     *tw;              ///< Theme settings for this widget type
  U_INT               prop;             ///< Properties
  //------------Texture management --------------------------------------------
  TC_VTAB             Pan[4];           //  Main screen panel
  //-------------For upper case mask ------------------------------------------
  U_CHAR              upper;
  char                clip;               // Component is clipped
  //--------------margin ------------------------------------------------------
  short               lMG;                // Left Margin
  short               rMG;                // Right margin
  short               tMG;                // Top margin
  short               bMG;                // Bottom margin
  short               Top;                // Title size
  //---------------------------------------------------------------------------
  CFuiComponent      *cFocus;             // Component that has the focus
  CFuiComponent      *MoWind;             // Mother window
  SSurface           *surface;
	//---------------------------------------------------------------------------
	U_INT								colText;						// Text color
	U_INT								cBlack;
};

//==================================================================================
#define WINDOW_RESIZE_NONE 0
#define WINDOW_RESIZE_XDIM 1
#define WINDOW_RESIZE_YDIM 2
#define WINDOW_RESIZE_BOTH 3
//==================================================================================
// CFuiPicture
//==================================================================================
class CFuiPicture : public CFuiComponent
{
public:
  //--- Constructors / destructor ------------------
  CFuiPicture (int x = 0, int y = 0, int w = 0, int h = 0, CFuiComponent *win = 0);
  virtual ~CFuiPicture (void);

  //--- CStreamObject methods ----------------------
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);

  //--- CFuiComponent methods ----------------------
  void      Edit();
  // CFuiPicture methods
	void      SetBitmap (CBitmap *bm);
  CBitmap*  GetBitmap (void) {return bm;}
  void      SetFrame (int i);
  //----Mouse events -------------------------------
  bool      MouseStopClick (int mx, int my, EMouseButton button);
  bool      MoveOver(int mx,int my);
  bool      Moving(int mx,int my);
  void      ClickOver(int mx, int my);
  void      Format(WZ_SIZE code, short dx, short dy);
  //------------------------------------------------
  void      Redim(short dx,short dy);
  void      FillTheRect(short x,short y,short wd,short ht,U_INT col);
	void			SetResize(Tag c,char t);
	//-------Stop resizing ----------------------------
  inline    void  FocusLost()			{rStat = WINDOW_RESIZE_NONE;}
	inline    void	SetOwn(char o)	{own = o;}
	inline    bool  ForResize()			{return (rCurs != 0);}
  //-------------------------------------------------
protected:
	char			own;
  CBitmap  *bm;                 ///< Bitmap (PBM/PBG)
  short     pFrame;             ///< PBG frame number on last draw cycle
  short     cFrame;             ///< PBG frame number for this draw cycle
  //-------------------------------------------------
	Tag				rCurs;											// Resize cursor
	U_CHAR		rType;											// Resize type
	U_CHAR    rStat;                     // Resize state
  short     mx;                         // Mouse coordinate
  short     my;                         // Mouse coordinate
	//-------------------------------------------------------
};


//===============================================================================
// JSDEV* CFuiButton
//===============================================================================
class CFuiButton : public CFuiComponent
{
public:
  // Constructors / destructor
  CFuiButton (int x = 0, int y = 0, int w = 0, int h = 0, CFuiComponent *win = 0);
  virtual ~CFuiButton(void);
  // CStreamObject methods
  virtual int  Read (SStream *stream, Tag tag);
  virtual void ReadFinished (void);

  // CFuiComponent methods
  void        Draw (void);
  void        Repeat();
  void        SetText(char *txt);
  void        MoveParentTo(int xp,int yp);
  bool        MouseMove (int x, int y);
  bool        MouseClick (int x, int y, EMouseButton button);
  bool        MouseStopClick (int x, int y, EMouseButton button);
	void				FocusLost();
 //---------------------------------------------------------------------
  inline void NotifyOnStop()      {stop = 1;}
 //--------------------------------------------------------------------
  // CFuiButton methods
  void  Fill (void);             ///< Populate widget with child objects
  void  MoveBy(int dx, int dy);
  void  SetRepeat(float r);
protected:
  U_CHAR            stop;        ///< Notify mouse stop
  U_CHAR              ok;        ///< This is an OK button
  U_CHAR            canc;        ///< This is a Cancel button
	U_CHAR						rrpt;					// Repeat count
  U_CHAR            rate;        ///< Repeat rate
  U_CHAR            pres;
  CFuiPicture*      hBox[3];
  U_SHORT           hText;       ///< Text height
};


//=============================================================================
// JSDEV* CFuiWindowTitle
//=============================================================================
class CFuiWindowTitle : public CFuiComponent
{
public:
  // Constructors / destructor
  CFuiWindowTitle (int x = 0, int y = 0, int w = 0, int h = 0, CFuiComponent *win = 0);
  ~CFuiWindowTitle ();
  // CStreamObject methods

  // CFuiComponent methods
  void      Draw (void);
  void      MoveParentTo(int xp,int yp);
  // CFuiWindowTitle methods
  void      InitTitle (char *txt);
  void      EditTitle();
  void      ChangeTitle(char * txt);
  bool      MouseClick(int mx,int my,EMouseButton btn);
  //----------------------------------------------------------------
  void      Redim(short dx,short dy);
  int       GetTitleWide()                  {return textw + 8;}
  //----------------------------------------------------------------
protected:
  unsigned int                backText;       // Back title colour
  CFuiPicture*                hBox[3];        // Title box
  short                        textw;         // Title wide
};

//==================================================================================
// CFuiCloseButton
//==================================================================================
class CFuiCloseButton : public CFuiComponent
{
public:
  // Constructors / destructor
  CFuiCloseButton (int x = 0, int y = 0, int w = 0, int h = 0, CFuiComponent *win = 0);

  // CStreamObject methods

  // CFuiComponent methods
  bool        MouseMove (int x, int y);
  bool        MouseClick (int x, int y, EMouseButton button);
  bool        MouseStopClick (int x, int y, EMouseButton button);

  // CFuiCloseButton methods
  void        SetFrame(short fr);
protected:
  short       Frame;
  CBitmap*    bmBack;                   ///< Backdrop bitmap
};

//==================================================================================
// CFuiMinimizeButton
//==================================================================================
class CFuiMinimizeButton : public CFuiComponent
{
public:
  // Constructors / destructor
  CFuiMinimizeButton (int x = 0, int y = 0, int w = 0, int h = 0, CFuiComponent *win = 0);

  // CStreamObject methods

  // CFuiComponent methods
  bool        MouseMove (int x, int y);
  bool        MouseClick (int x, int y, EMouseButton button);
  bool        MouseStopClick (int x, int y, EMouseButton button);

  // CFuiCloseButton methods
  void SetFrame(short fr);
protected:
  short       frame;
  CBitmap*    bmBack;         ///< Backdrop bitmap
};

//==================================================================================
// CFuiZoomButton
//==================================================================================
class CFuiZoomButton : public CFuiComponent
{
public:
  // Constructors / destructor
  CFuiZoomButton (int x = 0, int y = 0, int w = 0, int h = 0, CFuiComponent *win = 0);

  // CStreamObject methods

  // CFuiComponent methods
  bool        MouseMove (int x, int y);
  bool        MouseClick (int x, int y, EMouseButton button);
  bool        MouseStopClick (int x, int y, EMouseButton button);

  // CFuiCloseButton methods
  void        SetFrame(short fr);
protected:
  short       frame;
  CBitmap*    bmBack;         ///< Backdrop bitmap
};


//=====================================================================================
// CFuiWindow
//=====================================================================================
class CFuiGroupBox;               // Forward declaration
class CFuiList;
class CFuiPage;
class CFuiMenuBar;
class CmHead;
//=====================================================================================
class CFuiWindow : public CFuiComponent
{

public:
  // Constructors / destructor
   CFuiWindow (Tag tag, const char* winFilename, int wd = 0, int ht = 0, short lim = 0);
   virtual ~CFuiWindow (void);

  // CStreamObject methods
  virtual int         Read (SStream *stream, Tag tag);
  virtual void        ReadFinished (void);
  //-----Helper to add component ----------------------------------
  void                AddChild(Tag idn,CFuiComponent *win,char *txt = 0,U_INT p = 0, U_INT color = 0);
  //--------Database Management -----------------------------------
          void        PostRequest(CDataBaseREQ *req);
  virtual void        AddDBrecord(void *rec,DBCODE cd) {};
  virtual bool        NotifyFromDirectory(CmHead *obj){return false;}
  //----Helpers For derived class only ----------------------------
  virtual void        Initialize(CmHead*obj,U_SHORT type){}
	virtual void				FileSelected(FILE_SEARCH *pm) {;}
					bool				CreateFileBox(FILE_SEARCH *fpm);
					void				CloseModal();
					void				CreateDialogBox(char *ttl, char *msg, char nb = 0);			
          void        AddZoomButton();
          void        AddMiniButton();
          bool        ClickImage (int mx, int my, EMouseButton button,S_IMAGE &info);
          bool        MoveImage(int mx,int my,S_IMAGE &info);
          bool        ResetImage(S_IMAGE &info);
          void        RegisterPopup(CFuiPage *pge);
  CFuiGroupBox       *BuildGroupEdit(int x,int y);
	int									SwapGroupButton(Tag btn, char *zbt);
  //------------System button handler -----------------------------
  void                SystemHandler(EFuiEvents evn);
  //------------Drawing methods -----------------------------------
  virtual void        Draw (void);
  virtual void        DrawByCamera(CCamera *cam) {}
  //-----------visibility management ------------------------------
  void                ModifyShow(Tag idn, bool vs);
	//--- Profile management ----------------------------------------
	virtual bool				CheckProfile(char action);
	//----------- picking management --------------------------------
	virtual void				OnePicking(U_INT nb) {;}
  //------------Mouse management ----------------------------------
  virtual bool        MouseMove (int x, int y);
  virtual bool        MouseClick (int x, int y, EMouseButton button);
  virtual bool        MouseStopClick (int x, int y, EMouseButton button);
  virtual bool        DoubleClick(int mx,int my,EMouseButton button);
  virtual bool        InsideClick (int mx,int my, EMouseButton button);
  virtual bool        InsideMove  (int mx,int my) {return MouseHit(mx,my);}
  virtual void        NotifyMenuEvent(Tag idm, Tag itm) {;}
  virtual void        NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
  virtual void        NotifyFromPopup(Tag id, Tag itm, EFuiEvents evn){;}
  virtual void        NotifyResize(short dx,short dy) {;}
  virtual void        MoveBy (int x, int y);
  virtual void        MoveTo (int xs,int ys);
	//-----------------------------------------------------------------
	virtual void				TimeSlice() {;}
  virtual bool        KeyboardInput(U_INT key);
	virtual bool				MouseCapture(int mx, int my, EMouseButton bt) {return false;}
  //----------------------------------------------------------------
  bool                WindowHit(int mx,int my);
  bool                CheckResize(int mx, int my);
  void                SetXRange(short w1,short w2)  {xMini = w1; xMaxi = w2;}
  void                SetYRange(short h1,short h2)  {yMini = h1; yMaxi = h2;}
  //----------------------------------------------------------------
  void                Resize(short dx,short dy);
  void                ResizeChild(short dx,short dy);
  void                ResizeTo(short dx,short dy);
  void                NormSize();
  void                MaxiSize();
  void                MiniSize();
  //------Old notification      ------------------------------------
  virtual void        EventNotify(Tag win,Tag cpn,EFuiEvents evn,EFuiEvents sub );
  //----------------------------------------------------------------
  static void         ParseThemeElement (const char* s);
  //--------- CFuiWindow methods-------------------------------------
  virtual void        Close();
	virtual void				ModalClose();
	//-----------------------------------------------------------------
  U_CHAR              GetState (void) {return state;}
  void                SetTitle (char* title);
  CFuiComponent*      GetComponent (Tag component);
  //------------------------------------------------------------------
  void                ShowMe();
  void                HideMe();
  //------------------------------------------------------------------
	void								SetBackPicture(char *fn);
  void                MoveWindow(int mx,int my);
  void                CreateMenuBar(SStream *str);
  void                SetTransparentMode();
	void								SetTransparentMode(Tag idn);
  void                SetChildProperty(Tag idn,U_INT p);
  void                SetChildText(Tag idn,char *txt);
  void                SurfaceOrigin(int &x,int &y);
  void                WindowRect(C_2DRECT &r);
  void                WindowViewPort(VIEW_PORT &v);
  void                DebDrawInside();
  void                EndDrawInside();
  //-------------------------------------------------------------------
  inline int		TitleHeight()  {return (tBar)?(tBar->Height()):(0);}
  inline Tag		GetWinId()     {return windowId;}
  inline void		SetState(U_CHAR s)  {state = s;}
	inline bool		NotModal()			{return (0 == modal); }
	inline void		ClearModal()		{modal = 0;}
  //-------------------------------------------------------------------
protected:
  Tag               windowId;   ///< Window manager unique ID for this window
  short             hMax;       ///< Maximum height
  U_CHAR            state;      ///< Internal state of the window
  U_CHAR            limit;      ///< Limit height
  U_CHAR            title;      ///< Window has title bar
  U_CHAR            close;      ///< Window has Close button
  U_CHAR            mini;       ///< Window has Minimize button
  U_CHAR            zoom;       ///< Window has Maximize button
  U_CHAR            border;     ///< Window has border
  U_CHAR            borderSize; ///< Border size
  U_CHAR            move;       ///< Window is movable
  U_CHAR            save;       ///< Save ???
	char							abtn;				// Airport button
	char							obtn;				// Object button
  //-----------------------------------------------------
  VIEW_PORT           vps;      // View port save area
  //-------------Full component box ---------------------
  bool               drawBack;
  CFuiComponent     *fBox[MSIZ];
  //-----------------------------------------------------
  std::deque<CFuiComponent*>  childList;          // Child components
	CFuiGroupBox      *gBox;
  CFuiMenuBar       *mBar;
  CFuiWindowTitle   *tBar;
  CFuiCloseButton    *btc;
  CFuiZoomButton     *btz;
  CFuiMinimizeButton *btm;
	CFuiWindow         *modal;										// Modal windows
	//---- Move management -------------------------------------------
	CFuiComponent      *edge;											// Selected edge for resize
  int               lastX, lastY;               ///< Cursor position at last move update
  //------------Popup management -----------------------------------
  CFuiPage          *mPop;
  //------------Cursor management for resize -----------------------
  Tag               xCurs;                      // right Cursor 
  Tag               yCurs;                      // bottom Cursor 
  Tag               cCurs;                      // Corner cursor
  //-------Window size parameters ----------------------------------
  short             xMini;                      // X minimum size
  short             xMaxi;                      // X maximum size
  short             yMini;
  short             yMaxi;
  //-------Overall rectangle ----------------------------------------
  C_2DRECT          Rect;
  //-----------------------------------------------------------------
  short             prevW;                      // Previous wide
  short             prevH;                      // Previous Height
  U_CHAR            vStat;                      // View  state 0-1-2
};

//==================================================================================
//  Menu Part definition
//==================================================================================
typedef struct  {//---------Menu part format -------------
     char     Type;                                 // Part type
     char     State;                                // Part state
     char     Check;                                // Checked status
     char     Feat;                                 // Features
     //----------------------------------------------------------
     U_SHORT  yPos;                                 // Vertical position
     U_SHORT  hDim;                                 // Vertical size
     //----------------------------------------------------------
     Tag      Iden;                                 // Part ident
     //----------------------------------------------------------
     char     Text[64];                             // Txet part
     //----------------------------------------------------------
} SMenuPart;
//==================================================================================
//  Menu Pane definition
//==================================================================================
typedef struct  {//---------Menu pane --------------------------------
    int         xp;                       // Relative x coordinate
    int         yp;                       // Relative y coordinate
    int         wp;                       // Pane wide
    int         hp;                       // Pane height
    //----------------------------------------------------------------
    SSurface   *pSurf;
}SMenuPane;

//==================================================================================
// CFuiPopupMenu
//==================================================================================
class CFuiPopupMenu : public CFuiComponent
{
protected:
  //---------------Attributes ----------------------------------
  int               just1, just2, just3;
  CFuiPicture*      hBox[3];
  //-------------EDIT Parameter --------------------------------
  char              bText[32];                // Button text
  U_INT             colHili;
  U_CHAR            hCar;
  U_CHAR            wCar;
  CFuiPage         *Page;
  //-------------------------------------------------------------
  Tag                          selection;      ///< Currently selected choice
  int                          xText, yText;   ///< Offsets to start drawing text
  int                          origH;          ///< height when inactive
  //-------------------------------------------------------------
public:
  // Constructors / destructor
   CFuiPopupMenu (int x = 0, int y = 0, int w = 0, int h = 0, CFuiComponent *win = 0);
  ~CFuiPopupMenu();

  // CStreamObject methods
  virtual int       Read (SStream *stream, Tag tag);
  virtual void      ReadFinished (void);

  // CFuiComponent methods
  void  Draw (void);
  void  MoveParentTo (int xp, int yp);
  void  MoveBy(int dx,int dy);
  //-----------Mouse management -------------------------------
  bool  MouseClick (int x, int y, EMouseButton button);
  bool  MouseStopClick (int mx, int my, EMouseButton button);
  void  FocusLost();
  //-----------Notification methods ---------------------------
  virtual void NotifyMenuEvent(Tag idm, Tag itm);
  virtual void EventNotify(Tag win,Tag cpn,EFuiEvents evn,EFuiEvents sub );
  virtual void NotifyChildEvent (Tag idm, Tag itm, EFuiEvents evn);
  //-----------Edit methods -----------------------------------
  void  CreatePage(FL_MENU *sm,char **txt, short row = 0);
  void  ClearPage();
  void  SetButtonText(char *txt);
  void  SetText(char *txt);
  void  Select(U_INT No);
	void  SelectText(char *txt);
  FL_MENU *GetMenu();
  //-----------------------------------------------------------
  void  Lock()    {SetProperty(FUI_IS_LOCKED);}
  void  Unlock()  {RazProperty(FUI_IS_LOCKED);}
};

//==================================================================================
// CFuiCheckbox
//==================================================================================
class CFuiCheckbox : public CFuiComponent
{
public:
  // Constructors / destructor
  CFuiCheckbox (int x = 0, int y = 0, int w = 0, int h = 0, CFuiComponent *win = 0);

  // CStreamObject methods
  virtual int  Read (SStream *stream, Tag tag);
  virtual void ReadFinished (void);

  // CFuiComponent methods
  bool MouseClick (int x, int y, EMouseButton button);
  bool MouseStopClick (int x, int y, EMouseButton button);

  // CFuiCheckbox methods
  int   GetState (void);         ///< Retrieve current state of checkbox
  void  SetState (int);          ///< Set current state of checkbox
  void  SetText(char *txt);
  void  Draw();
protected:
  //--------------Attributes ------------------------------------------
  short       txPos;            // Text position
  U_INT       black;            // Color
  CBitmap   *bmBack;            // Bitmap graphics
  EFuiEvents  Event[2];         // Event notification
  //-------------------------------------------------------------------
  int       state;              // Checkbox state (0/1 for off/on)
  int       Frame;              // Frame to draw
  bool      pressed;            // Whether checkbox is being pressed
};

//==================================================================================
// CFuiCheckBox
//==================================================================================
class CFuiCheckBox : public CFuiComponent
{
public:
  // Constructors / destructor
  CFuiCheckBox (int x = 0, int y = 0, int w = 0, int h = 0, CFuiComponent *win = 0);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);
  void  EditText();
  void  SetText(char * bla);
  void  SetState (char s);
  // CFuiComponent methods
  bool  MouseClick (int x, int y, EMouseButton button);
  bool  MouseStopClick (int x, int y, EMouseButton button);

  //----- CFuiCheckBox methods ---------------------------------
  inline int   GetState (void)    {return state;}
  inline void  IniState(char s)   {if (s < 3) state = s;}
protected:
  short     wCar;               // Font character width
  short     hCar;               // Font char height
  short     xTXT;               // Text start location
  short     yTXT;               // Text line
  //----Bitmap con tour -------------------------------------------
  short     bx0;                // left side
  short     bx1;                // Right side
  short     by0;                // Top line
  short     by1;                // Bottom line
  //---- Bitmap parameters ----------------------------------------
  CBitmap  *bmBT;               // Bitmap graphics
  int       bwd;                // bitmap width
  int       bht;                // Bitmap height
  //----------------------------------------------------------------
  int       state;              // Radio button state (0/1 for off/on)
  bool      pressed;            // Button is pressed
};

//==================================================================================
// CFuiLabel
//==================================================================================
class CFuiLabel : public CFuiComponent
{
public:
  // Constructors / destructor
  CFuiLabel (int x = 0, int y = 0, int w = 0, int h = 0, CFuiComponent *win = 0);

  //------- CStreamObject methods----------------
  virtual int  Read (SStream *stream, Tag tag);
  virtual void ReadFinished (void);

  //------ CFuiComponent methods------------------
  void  EditText(char *fmt, ...);
  void  SetText(char *bla);
  void  RedText(char *bla);
  void  Edit(U_INT col);
  void  Draw();
  //----- CFuiLabel inline methods --------------
  inline void SetJustif (int jus){just = jus;}
	//--- ATTRIBUTES ------------------------------
protected:
  U_SHORT       yText;
  int           just;       // Justification mask; see EFuiJustify
  int           wrap;       // Word-wrap flag
  unsigned int  redText;    // Red color
  unsigned int  colShadow;  // Shadow colour
  bool          useShadow;  // Use shadowed text
};

//==================================================================================
// CFuiLine
//==================================================================================
#define FUI_LINE_HORIZONTAL   0
#define FUI_LINE_VERTICAL     1
//--------------------------------------------------------------------
class CFuiLine : public CFuiComponent
{
public:
  // Constructors / destructor
  CFuiLine (int x = 0, int y = 0, int w = 0, int h = 0, CFuiComponent *win = 0);

  // CStreamObject methods
  virtual int  Read (SStream *stream, Tag tag);
  virtual void ReadFinished (void);

  //----------------------------------------------------------------
protected:
  int           direction;      ///< Direction? Currently unused
  int           thickness;      ///< Thickness in pixels

  unsigned int  colLine;        ///< Line colour
};

//==================================================================================
// CFuiBox
//==================================================================================
class CFuiBox : public CFuiComponent
{
public:
  // Constructors / destructor
  CFuiBox (int x = 0, int y = 0, int w = 0, int h = 0, CFuiComponent *win = 0);

  // CStreamObject methods
  int     Read (SStream *stream, Tag tag);
  void    ReadFinished (void);
  void    Init(int x, int y);
  // CFuiComponent methods
  void    Draw (void);
  void    MoveParentTo(int xp,int yp);
	void		BackColor(U_INT col);
  //------------Attributes ----------------------------------------
protected:
  CFuiComponent *cBox[RSIZ];      // Box of components
  int           thickness;       ///< Border line thickness
  unsigned int  colLine;         ///< Border line colour
  unsigned int  colFill;         ///< Backdrop fill colour
};


class CFuiMenuSeparator : public CFuiComponent
{
};

//==================================================================================
// CFuiMenu
//
// Private class (not directly specified in a window template) for a submenu
//==================================================================================
class CFuiMenu : public CFuiComponent
{  //-------------------------------------------------------------
  enum  MTYPE { TEXT = 0,
                LINE = 1,
  };
  //--------------------------------------------------------------
  enum  MSTATE { NORM = 0,
                 HILI = 1,
                 SDIM = 2,
  };
public:
  //--------------------------------------------------------------
  enum  FEATUR { NONE   = 0,
                 CHECK  = 0x01,
  };
  //--------------------------------------------------------------
public:
   CFuiMenu (Tag id = 0, int pos = 0, const char* label = 0, CFuiComponent *mwin = 0);
  ~CFuiMenu (void);

  // CStreamObject methods
  virtual void ReadFinished (void);
  // CFuiComponent methods --------------------------------------
  void  MoveParentTo(int xp,int yp);
  void  Draw (void);
  void  DrawPane();
  void  SetPartState(SMenuPart * mp,char st);
  void  SetTextState(SMenuPart * mp,char st);
  void  SetLineState(SMenuPart * mp,char st);
  // CFuiMenu methods -------------------------------------------
  void        Initialize();
  void        AddItem (Tag itemID, const char* label);
  void        AddSeparator (void);
  void        ClearItems (void);
  void        CheckSelectedPart(Tag idn);
	void				ChangeState(Tag idn,char st);
  SMenuPart  *GetMenuPart(Tag itm);
  //-------------Mouse management -------------------------------
  bool  MouseMove(int x,int y);
  bool  PaneHit(int x,int y);
  bool  PaneClick(int x,int y, EMouseButton btn);
  void  FocusLost();
  void  SwapPart(SMenuPart *mp);
  void  SetState(int sta);
  //-------------------------------------------------------------
  inline int  GetWide() {return w; }
  inline bool IsGrayed(void) {return (mState == 2);}
  //-------------------------------------------------------------
protected:
  void          Cleanup (void);
  //-------------------------------------------------------------
protected:
  //-----Part Management ---------------------------------------
  SMenuPane   *mPane;
  std::vector<SMenuPart *> Parts;
  short   vDim;                           // Vertical dimension
  short   hText;                          // Text heigh
  short   hLine;                          // Line height
  U_INT   cBackPane;                      // Color pane
  U_INT   cBakHLight;                     // Color HLight
  //----------Pane parameters ----------------------------------
  short       xPane;
  short       yPane;                      // Pane Y position
  short       pWd;                        // Pane Wide
  short       pHt;                        // Pane height
  U_INT   Back[SDIM];                     // Stack of color
  //------------Check bitmap -----------------------------------
  int         ckw;                        // Width
  int         ckh;                        // Height
  CBitmap*    bmCK;                       // Bitmap check
  //--------Menu State -----------------------------------------
  char     mState;                        // Menu State
  SMenuPart *mPart;                       // Selected part
  U_INT    tColor[3];                     // Text color
  unsigned long colLine;
};

//==================================================================================
// CFuiMenuBar
//==================================================================================
class CFuiMenuBar : public CFuiComponent
{ 
public:
  // Constructors / destructor
  CFuiMenuBar (int wd, CFuiComponent *pwin = 0);
  virtual ~CFuiMenuBar (void);

  // CStreamObject methods
  int  Read (SStream *stream, Tag tag);
  void ReadFinished (void);

  // CFuiComponent methods
  void  Draw (void);
  void  MoveParentTo(int xp,int yp);
  void  MoveTo(int xs,int ys);
  //------Mouse management ------------------------------------------------
  bool      MouseClick (int x, int y, EMouseButton button);
  bool      MouseMove(int x,int y);
  void      FocusLost(void);
  bool      SelectMenu(int x,int y);
  void      NotifyMenuEvent(Tag idm, Tag itm); 
  void      CheckSelectedPart(Tag idm,Tag itm);
	void			ChangePartState(Tag idm,Tag itm,char st);
  //------------------------------------------------------------------------
  // CFuiMenuBar methods
  void          SetState(U_CHAR st) {state = st;}
  CFuiMenu*     GetMenu (Tag menuID);
  void          AddMenu (Tag menuID, const char *label, Tag fontTag = 0);
  void          RemoveMenu (Tag menuID);
  void          AddMenuItem (Tag menuID, Tag itemID, const char *label);
  void          AddMenuSeparator (Tag menuID);
  //-------------------------------------------------------------------------
  SMenuPart    *GetMenuPart(Tag idm, Tag itm);
  //-------------------------------------------------------------------------
protected:
  void          SwapMenu(CFuiMenu *men);
  //-------------------------------------------------------------------------

protected:
  bool                      activated;  ///< Is menu activated or not
  std::map<Tag,CFuiMenu*>   menu;       ///< Submenus
  //---------------------------------------------------------------
  U_CHAR        state;              // Menu state
  CFuiPicture  *hBox[3];            // Bitmap box
  U_SHORT       mPos;               // Menu position
  U_SHORT       width;              // Total wide
  CFuiMenu     *mSel;               // Selected menu
  unsigned int  colHighlight;       ///< Highlight background colour
  unsigned int  colHighlightText;   ///< Highlight text colour
};

//==================================================================================
// CFuiWindowMenuBar
//==================================================================================
class CFuiWindowMenuBar : public CFuiMenuBar
{
public:
  // Constructors / destructor
  CFuiWindowMenuBar ();
  virtual ~CFuiWindowMenuBar (void);
};
//==================================================================================
// CFuiSCrollBTN  A scroll button
//==================================================================================
class CFuiScrollBTN : public CFuiComponent {
  //---- ATTRIBUTES -----------------------------------
  SCBOX       sBOX;            // SCROLL BOX
  int         Frame;
  U_CHAR      rrpt;             // key press repeatable
  U_CHAR      rate;             // Repeat rate
  U_CHAR      pres;             // Press ON
  //-----METHODS ---------------------------------------
public:
  CFuiScrollBTN(int x, int y, int w, int h, CFuiComponent *win,char *btn);
  int     Read (SStream *stream, Tag tag);
  void    ReadFinished();
  void    SetRepeat(float r);
  //--- Mouse event ------------------------------------
  void    Repeat();
  bool    MouseClick (int x, int y, EMouseButton button);
  bool    MouseStopClick (int x, int y, EMouseButton button);
  void    Draw();
  //---- ----------------------------------------------
};
//==================================================================================
// CFuiScrollBar
//==================================================================================
class CFuiScrollBar : public CFuiComponent
{
public:
  // Constructors / destructor
  CFuiScrollBar(int x, int y, int w, int h, CFuiComponent *win, bool bVertical);
  ~CFuiScrollBar();
  //-----------------------------------------------------
  void  ReadFinished (void);               // pseudo
  void  SetVTposition();
  void  SetHZposition();
  void  ResizeVT(short ht);
  //------return dimensions ------------------------------
  inline int GetVSwide(void)  {return w;}
  //---------- CFuiComponent methods-----------------------
  void  EditButton(SCBOX *box);
  void  EditHSB();
  void  EditVSB();
  void  DrawHSB();
  void  DrawVSB();
  void  FillBox(SCBOX *box);
  //----Mouse evnt on vertical scroll bar -----------------
  bool        VtMouseClick (int x, int y, EMouseButton button);
  bool        MouseStopClick (int x, int y, EMouseButton button);
  bool        MouseMove (int x, int y);
  bool        DragVThumb(int mx, int my);
  void        SetVSRatio(float rat);
  //----Mouse event on horizontal scroll bar --------------
  bool        HzMouseClick (int x, int y, EMouseButton button);
  bool        SendVTNotification(EFuiEvents evn, SCBOX *box = 0);
  bool        SendHZNotification(EFuiEvents evn, SCBOX *box = 0);
  bool        DragHThumb(int mx, int my);
  void        SetHSRatio(float rat);
  //------------------------------------------------------
protected:

  bool        m_bVertical;
  //----------------------------------------------------
  SCBOX       UpBOX;            // VT SCROLL Up box
  SCBOX       DoBOX;            // VT SCROLL down box
  SCBOX       VmBOX;            // VT SCROLL middle box
  SCBOX       VtBOX;            // VT SCROLL Thumb box
  SCBOX       LfBOX;            // HZ SCROLL Left box
  SCBOX       RtBOX;            // HZ SCROLL Right box
  SCBOX       HmBOX;            // HZ SCROLL middle box
  SCBOX       HtBOX;            // HZ SCROLL Thumb box
  //------Focus management -----------------------------
  SCBOX      *bFocus;
  //------Vertical scroll parameters -------------------
  short       VtTop;                      // Thumb point Upper limit
  short       VtAmp;                      // Vertical amplitude
  short       VtLow;                      // Lower limit
  //----- Horizontal scroll limits ---------------------
  short       HzLef;                      // Thum point left limit
  short       HzAmp;                      // Horizontal amplitude
  short       HzRit;                      // Right limit
};

//==================================================================================
// CFuiSlider
//==================================================================================
class CFuiSlider : public CFuiComponent
{
public:
  // Constructors / destructor
  CFuiSlider (int x = 0, int y = 0, int w = 0, int h = 0, CFuiComponent *win = 0);

  // CStreamObject methods
  virtual int  Read (SStream *stream, Tag tag);
  virtual void ReadFinished (void);
  // CFuiSlider methods
  void  SetValue(float fValue);
  void  SetRange(float a,float b);
  //------Edit methods ------------------------
  void  EditThick();
  void  EditThumb();
  //----- Mouse methods -----------------------
  bool  MouseClick(int mx,int my,EMouseButton btn);
  bool  MouseStopClick(int mx,int my,EMouseButton btn);
  bool  InThumb(int mx,int my);
  bool  MouseMove(int mx,int my);
  bool  Moving(int mv,int dim);
  void  FocusLost();
  //-------------------------------------------
  inline float  GetValue()  {return curv;}
  //-------------------------------------------
protected:
  CBitmap       *bmBack;
  CBitmap       *bmThumb;
  CBitmap       *bmLTick;
  CBitmap       *bmSTick;
  //-----------Thumb control -------------------------------
  U_CHAR      mstat;            // Move status
  short       hrul;             // Ruler height
  short       wrul;             // Ruler wide
  Tag         link;             // Linked FUI component
  int         prevM;            // Previous mouse X or Y position
  int         curPixPos;        // pixel pos of indicator compute from curv
  U_INT				bCol;							// Back color				
  //--------------------------------------------------------
  float       maxv, minv;       // Min/Max values
  float       curv;             // current value
  float       majt;             // Major tick interval
  bool        dmjt;             // Display major ticks
  float       mint;             // Minor tick interval
  //----------Thumb dimensions -----------------------------
  int         bmwI;             // Width of thumb bitmap
  int         bmhI;             // Heigh of thumb bitmap
  int         bmI2;             // half of the thumb bitmap
  bool        vert;             // Vertical orientation
};

//==================================================================================
// CFuiGroupBox
//==================================================================================
class CFuiGroupBox : public CFuiComponent
{
public:
  // Constructors / destructor
  CFuiGroupBox (int x = 0, int y = 0, int w = 0, int h = 0, CFuiComponent *win = 0);
  ~CFuiGroupBox (void);

  // CStreamObject methods
  virtual int  Read (SStream *stream, Tag tag);
  virtual void ReadFinished (void);

  // CFuiComponent methods
  virtual bool        MouseMove (int x, int y);
  virtual bool        MouseClick (int x, int y, EMouseButton button);
  virtual bool        MouseStopClick (int x, int y, EMouseButton button);
  void                Draw (void);
  void                MoveParentTo(int xp,int yp);
  void                MoveTo(int xs,int ys);
  void                MoveBy(int dx,int dy);
  CFuiComponent*      GetComponent (Tag idn);
  void                SetChildText(Tag id,char *txt);
  void                SetEditMode(Tag id,U_CHAR md);
  bool                KeyboardInput(U_INT key);
  void                AddChild(Tag idn,CFuiComponent *cmp,char *txt, U_INT color = 0);
	void								SetTransparentMode();
	//----------------------------------------------------------------------
	void								SetText(char *t);
  //------NOTIFICATIONS --------------------------------------------------
  void                NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn);
  void                NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
  //------Focus Management ------------------------------------------------
  void                RegisterPopup(CFuiPage *pge) {MoWind->RegisterPopup(pge);}
  void                FocusLost();
protected:
  //------Bitmap box ------------------------------------------------------
  CFuiComponent *fBox[RSIZ];
  int           xText;                         ///< X offset for groupbox text label
  std::list<CFuiComponent *>   childList;      ///< Vector of child components
};

//==================================================================================
// CFuiGauge
//==================================================================================
class CFuiGauge : public CFuiComponent
{
public:
  // Constructors / destructor

  // CStreamObject methods

protected:
};
//==================================================================================
//    LIST BOX
//==================================================================================
class CFuiList : public CFuiComponent
{//-----------------------------------------------------------------
protected:
  U_CHAR    usrr;
  U_CHAR    msel;         ///< Allow multiple selections
  U_CHAR    vscr;         ///< Use vertical scrollbar
  U_CHAR    hscr;         ///< Use horizontal scrollbar
  U_CHAR    autowidth;    ///< Autowidth
  U_CHAR    input;        // Input allowed
  U_CHAR    scrol;        /// Last scroll
  U_SHORT   rowSelected;  ///< selected row number
  //------------Text dimension -----------------------------
  short     hCar;             // Character height
  short     wCar;             // Average char wide
  short     hLine;            // Line height
  short     NbLin;            // Number of lines in page
  short     NbCol;            // Number of char in a line
  short     sCol;             // Scroll column
  //-------------Band parameters ------------------------
  short     aROW;             // Row number
  short     bROW;             // Row number
  //-------------Full box -liste of decorations ---------
  CFuiComponent     *fBox[RSIZ];
  //-----------Color for component------------------------
	U_INT	 cBlack;
  U_INT  cTxtHLight;
  U_INT  cBakHLight;
  U_INT  cBackTitle;
  U_INT  cText;
  //---Check mark ---------------------------------------
  SBitmap  *cMark;
  //-----------------------------------------------------
  CFuiScrollBar *vsBOX;
  CFuiScrollBar *hzBOX;
  //-----------------------------------------------------------------

public:
  // Constructors / destructor
   CFuiList (int x = 0, int y = 0, int w = 0, int h = 0, CFuiComponent *win = 0);
  ~CFuiList ();

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  virtual void  ReadFinished (void);
  //-------------Edit methods --------------------------------------
  void          TextPolicy(Tag pol);
  void          SetBackTitle();
  void          ClearPage();
  void          ClearBand(int ln,int nbl);
  U_INT         ClearLine();                
  void          DrawSelection(short nl,U_INT col);
  void          SetSelectionTo(U_SHORT nl);
  short         GetTextBase(short line);
  void          Underline(short line);
  void          SetColorTxtNormal (void) {cText = colText;}
  void          ResizeVT(short dy);
  void          MoveBy(int xs,int ys);
  //-------EDIT INTERFACE -------------------------------------------
  void          NewLine(short nl);
  void          AddText(short lin,short col,short nc, char *txt);
  void          AddBitmap(short lin,short col,SBitmap *bmap);
  void          AddSeparator(short lin);
  void          AddMark(short ln,short col,char mrk);
  //-----------------------------------------------------------------
  void          NoHscroll(void);
  //------------Inline method ---------------------------------------
  inline  short   GethCar(void)     {return hCar;}
  inline  short   GetwCar(void)     {return wCar;}
  inline  short   GetNberLine(void) {return NbLin;}
  inline  U_SHORT GetSelLine()      {return rowSelected;}
  inline  void    SetVScroll()      {vscr = 1;}
  inline  void    SetBand(short a,short n)  {aROW = a; bROW = a+n;}
  inline  void    FocusIn(int a,int b)      {aROW = a; bROW = b;}
  //----------Management -------------------------------------------
	void					SetTransparentMode();
	short         ChangeLineHeight(short ht);
  short         IncLineHeight(short ht);
  void          SetVSRatio(float rat);
  void          SetHSRatio(float rat); 
  //--------------Draw methods -------------------------------------
  void Draw (void);
  //-------------Notifications   -----------------------------------
  virtual void  NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
  //----------------------------------------------------------------
  bool          InvalidLine(U_INT row);
  bool          MouseClick (int x, int y, EMouseButton button);
  bool          DoubleClick(int x,int y,EMouseButton bt);
  bool          MouseStopClick (int x, int y, EMouseButton button);
  bool          MouseMove (int x, int y);
  bool          KeyboardInput(U_INT key);
  void          FocusLost();
  //-----------------------------------------------------------------
  void          MoveParentTo(int xp,int yp);
  unsigned int  SelectedRow(void){return rowSelected;};
};

//==================================================================================
// CFuiTextField
//==================================================================================
class CFuiTextField : public CFuiComponent
{
public:
  // Constructors / destructor
   CFuiTextField (int x = 0, int y = 0, int w = 0, int h = 0, CFuiComponent *win = 0);
  ~CFuiTextField (void);

  //---------- CStreamObject methods  -----------------------------
  int  Read (SStream *stream, Tag tag);
  void ReadFinished (void);
  //---------- CFuiComponent methods  ------------------------------
  void  MoveParentTo(int xp,int yp);
  void  Draw (void);
  bool  KeyboardInput(U_INT key);
  bool  MouseClick(int x, int y, EMouseButton button);
  void  FocusLost(void);
  void  MoveBy(int dx,int dy);
  //------------Edit methods ---------------------------------------
  void  EditText(char *fmt, ...);
  void  EditText();
  void  ShiftRight();
  int   AddChar(U_CHAR car);
  int   ShiftLeft();
  int   BackSpace();
  char  UpperChar(U_CHAR car);
  void  DrawCaret();
  void  NewCaret(short cp);
  void  ScrollRight();
  void  ScrollLeft();
  void  LeftPos();
  void  RightPos();
  //----------- CFuiTextField methods ------------------------------
  void  UsePassword (int passwordFlag);
  int   IsPassword (void);
  int   GetLength (void);
  void  SetText (char *text);
  void  SetEditMode(U_CHAR md);
  void  SetSelection(int  firstChar, int lastChar);
  void  GetSelection(int *firstChar, int *lastChar);
  void  SetMaxChars(int mc);
  //----------------------------------------------------------------
  inline U_CHAR IsModified()						{	return fmod;}
	inline void		SetValidMask(U_CHAR m)	{vcar = m;}
protected:
  //------- Attributes --------------------------------
  U_CHAR        input;
  U_CHAR        time;
	U_CHAR				vcar;
	U_CHAR				rfu1;
  //---CARET ATTRIBUTES -------------------------------
  U_INT         caret;
  U_INT         mask;
  //--------------------------------------------------
  CFuiComponent     *fBox[RSIZ];
  //-------Text color -------------------------------
  U_INT  colHighlight;
  U_INT  colTextHighlight;
  U_INT  colLine;
  U_INT  colFill;
  U_INT  colFillDisabled;
  U_INT  colCaret;
  U_INT         bCol[2];              // Background color
  //-------Edit parameters --------------------------
  short         hCar;                 // Character height
  short         wCar;                 // Character wide
  short         oInx;                 // Output index
  short         xEnd;                 // Pixel end
  short         kMax;                 // Max index permitted
  short         nCar;                 // Number of char in text
  short         cpos;                 // Caret position (char)
  short         cpix;                 // Caret position in pixel
  //-------------------------------------------------
  char          fmod;                 // Filed is modified
  char          pass;                 ///< Password entry field, 0=no
  char          numb;                 ///< Numeric only ?
};
//====================================================================



//==================================================================================
// CFuiGraphTrace
//==================================================================================
typedef enum {
  FUI_GRAPH_TRACE_POINT     = 0,
  FUI_GRAPH_TRACE_LINE      = 1
};

typedef struct {
  float   x, y;
} SFuiGraphTracePoint;

class CFuiGraphTrace {
public:
  // Constructors / destructor
  CFuiGraphTrace (Tag id, int type);

  // CFuiGraphTrace methods
  void  AddPoint (float x, float y);
  void  ClearPoints (void);
  void  SetRange (float minX, float minY, float maxX, float maxY);
  void  SetColour (unsigned int colour);
  void  Draw (SSurface *surface);

protected:
  Tag                               id;
  int                               type;
  unsigned int                      colour;
  float                             minX, maxX;
  float                             minY, maxY;
  std::deque<SFuiGraphTracePoint>  points;
};

//==================================================================================
// CFuiGraph
//==================================================================================
class CFuiGraph : public CFuiComponent {
public:
  // Constructors / destructor
  CFuiGraph (void);

  // CStreamObject methods
  virtual int  Read (SStream *stream, Tag tag);

  // CFuiComponent methods
  void Draw (void);

  // CFuiGraph methods
  void  AddTrace (Tag traceID, int traceType);
  void  RemoveTrace (Tag traceID);
  void  RemoveAllTraces (void);
  void  AddTracePoint (Tag traceID, float x, float y);
  void  ClearTracePoints (Tag traceID);
  void  SetTraceRange (Tag traceID, float minX, float minY, float maxX, float maxY);
  void  SetTraceColour (Tag traceID, unsigned int colour);
  void  SetUseGrid (int useGrid);
  void  SetGridRange (float minX, float minY, float maxX, float maxY);
  void  SetGridSpacing (float stepX, float stepY);
  void  SetGridColour (unsigned int colour);
  void  SetGridBackColour (unsigned int colour);
  void  GetGridRange (float *minX, float *minY, float *maxX, float *maxY);
  void  GetGridSpacing (float *stepX, float *stepY);

protected:
  bool      useGrid;      // Display grid lines
  float     minX, maxX;   // Horizontal grid range
  float     minY, maxY;   // Vertical grid range
  float     stepX, stepY; // Grid spacing

  unsigned long                 bgColour;     // Background colour
  unsigned long                 gridColour;   // Grid colour
  std::map<Tag,CFuiGraphTrace*> traces;       // Graph traces
};
//===================================================================================
// CFuiTextPopup
//
// A text popup is a simple text message typically used for user notification
//   (such as Slew or Pause mode, temporary messages)
//===================================================================================
class CFuiTextPopup : public CFuiComponent
{ float time;                                       // Time validity
  bool  actv;
  U_INT orange;
  U_INT black;
  U_INT darkgrey;
  U_INT mediumgrey;
  U_INT lightgrey;
  U_INT white;
  U_INT red;
  U_INT back;
public:
  // Constructors / destructor
  CFuiTextPopup();
  // CFuiComponent methods
  void  SetText (char *text);
  void  Draw();
  void  DrawIt();
  void  ChangeFont(SFont *f);
  void  DrawTheText();
  //--Inline -----------------------------------------------------
  inline  void  RedBack()             {back = red;}
  inline  void  OrangeBack()          {back = orange;}
  inline  void  SetActive(float dT)   {actv = true; time = dT;}
  inline  void  SetActive()           {actv = true;}
  inline  void  RazActive()           {actv = false;}
  inline  bool  IsActive()            {return actv;}
	//-------------------------------------------------------------
	inline  char *Buffer()							{return text;}
};

//===================================================================
enum EFuiThemeColourType {
  THEME_COLOUR_TEXT           =  0,
  THEME_COLOUR_SHADOW         =  1,
  THEME_COLOUR_SELECTION      =  2,
  THEME_COLOUR_HILITE         =  3,
  THEME_COLOUR_TEXTHILITE     =  4,
  THEME_COLOUR_LINE           =  5,
  THEME_COLOUR_TEXTGREY       =  6,
  THEME_COLOUR_FILL           =  7,
  THEME_COLOUR_FILLDISABLED   =  8,
  THEME_COLOUR_CARET          =  9,
  THEME_COLOUR_BORDER         = 10,
  THEME_COLOUR_BACK           = 11,
  THEME_COLOUR_TRACE          = 12,
  THEME_COLOUR_MAX
};

enum EFuiThemeSizeType {
  THEME_SIZE_SMALL          = 0,
  THEME_SIZE_NORMAL         = 1,
  THEME_SIZE_LARGE          = 2,
  THEME_SIZE_SMALL_INLINE   = 3,
  THEME_SIZE_NORMAL_INLINE  = 4,
  THEME_SIZE_LARGE_INLINE   = 5,
  THEME_SIZE_MAX
};

enum EFuiThemeFlagType {
  THEME_FLAG_DONT_DRAW_BACKGROUND      =  0,
  THEME_FLAG_IGNORE_BITMAP_BACKGROUND  =  1,
  THEME_FLAG_USE_SHADOW                =  2,
  THEME_FLAG_MAX
};

//==================================================================================
//  CFuiRuwayGraph to draw runway
//==================================================================================
class CFuiCanva: public CFuiComponent
{ //--------------Attributes ----------------------------------------
  U_CHAR          vscr;
  U_CHAR          hscr;
  U_CHAR          move;
  U_SHORT         hLin;
  U_SHORT         wCar;
  U_SHORT         row;
  CFuiComponent  *fBox[RSIZ];
  U_INT           cTab[2];
  U_INT           white;
  //---Move detection -----------------------------------------------
  U_SHORT         sx;
  U_SHORT         sy;
  CCamera        *Cam;
  //-----------------------------------------------------------------
public:
   CFuiCanva (int x, int y, int w, int h, CFuiComponent *win);
  ~CFuiCanva();
   int    Read (SStream *stream, Tag tag);
   void   ReadFinished (void);
   void   MoveParentTo(int xp,int yp);
   void   BeginPage();
   void   NewLine()   {row++;}
   void   AddText(short col, char *txt,char nl = 0);
   void   AddText(short col, char nl, char *fmt, ...);
   void   GetViewPort(VIEW_PORT &vp);
   //-----Mouse Management ------------------------------------------
   bool   MouseClick (int mx, int my, EMouseButton button);
   bool   MouseMove(int mx, int my);
   bool   MouseStopClick (int x, int y, EMouseButton button);
   //----------Draw canvas ------------------------------------------
   void   DrawSegment(int x0,int y0,int x1,int y1,int xc);
	 void		DrawChar(int x0,int y0,char c,U_INT k=0);
	 void		DrawNText(int x0,int y0,char *t,U_INT c);
   void   EraseCanvas();
   void   Draw();
   //----------------------------------------------------------------
   inline void RegisterCamera(CCamera *c) {Cam = c;}
};
//==================================================================================
//  CFuiPage to draw Text on list and popup
//==================================================================================
class CFuiPage: public CFuiComponent
{ //-------------Attributes --------------------------------------------
  U_CHAR  State;                                  // Activity state
  short   hCar;                                   // Char height
  short   wCar;                                   // Char wide
  U_INT  cBackPane;                               // Color Back Pane
  U_INT  cTxtHLight;                              // Text Hilight
  U_INT  cBakHLight;                              // Back Hilit
  U_INT  cText[2];
  U_INT  cBack[2];
  U_INT  grey;
  //----------Edit attributes ------------------------------------------
  FL_MENU *menu;                                  // Menu structure
  short    sLin;                                  // Selected line
  //-------------Methods -----------------------------------------------
public:
  CFuiPage (int x, int y, FL_MENU *sm, CFuiComponent *win,short slin = 0);
 ~CFuiPage();
  void        Initialize(void);
  void        SwapState();
  void        InitMenu(FL_MENU *sm);
  //----------inline ---------------------------------------------------
  inline  void    SetState(U_CHAR sta);
  //-----------Edit method ---------------------------------------------
  void    DrawPane();
  void    ClearLine(short nl);
  void    AddPopupText(short nl, char *txt);
  void    Refresh(short ln) {AddPopupText(ln, (char*)menu->aText[ln]);}
  //-----------Mouse method --------------------------------------------
  void    FocusLost();
  bool    MouseClick (int mx, int my, EMouseButton button);
  bool    MouseMove(int mx, int my);
  //---------Return selection -----------------------------------------
  inline void     Select(short nl) {sLin = nl;}
  inline short    GetSelLine()     {return sLin;}
  inline void     SetID(Tag t)     {id = t;}
  inline FL_MENU *GetMenu()        {return menu;}
};

//=============================================================================
// CFuiThemeWidget
//
// Theme components applicable to a specific widget type
//=============================================================================
class CFuiThemeWidget
{
public:
  // Constructors / destructor
  CFuiThemeWidget (void);
  ~CFuiThemeWidget (void);

  // CFuiThemeWidget methods
  void          ParseBitmap (char* s);
  void          ParseColour (char* s);
  void          ParseFrameRate (char *s);
  void          ParseThickness (char *s);
  void          ParseComponentName (char *s, std::string type);
  void          ParseButtonWidth (char *s);

  void          SetName (const char* s);
  CBitmap      *GetBitmap (std::string name);
  unsigned int  GetColour (std::string name);
  void          SetFlag (std::string name);
  bool          GetFlag (std::string name);
  const char*   GetComponentName (std::string name);

  void          Print (FILE *f);

protected:
  char                                name[64];         // Widget name
  std::map<std::string,CBitmap*>      bmMap;            // Map of bitmap elements
  std::map<std::string,unsigned int>  colourMap;        // Map of colour elements
  std::set<std::string>               flagSet;          // Set of enabled flags
  std::map<std::string,std::string>   componentNameMap; // Map of sub-component names
  int                                 framerate;
  int                                 thickness;
  int                                 buttonWidth;
  U_INT         black;
};
//===============================================================================
// CFuiTheme
//
// "Themes" encapsulate the visual style elements applied to all FUI components.
//===============================================================================
class CFuiTheme
{
public:
  // Constructors / destructor
  CFuiTheme (void);
  CFuiTheme (const char* themeFilename);
  ~CFuiTheme (void);

  // CFuiTheme methods
  CFuiThemeWidget* GetWidget (std::string name);
	int			Decode(char *t,char *s);
  void    Print (FILE *f);

protected:
  char                                   name[64];       // Theme name
  Tag                                    id;             // Unique ID
  std::map<std::string,CFuiThemeWidget*> widgetMap;
	std::string activeWidget;
};
//===============================================================================

//===============================================================================
// CFuiManager
//  JSDEV*  Modifications
//          1- delete window on creation (CreateFuiWindow) when the same id exist. Seems that 
//             map dont call the destructor
//          2- Modify the notification circuit
//          3- Create return window pointer
//===============================================================================
class CFuiManager
{ //-------ATTRIBUTES--------------------------------------
protected:
	//--- One texture object fro all components --------------------------------
	U_INT														xOBJ;
	//--------------------------------------------------------------------------
  std::map<Tag,CFuiWindow*>       winHide;
  std::map<Tag,CFuiWindow*>       winMap;
  std::map<Tag,CFuiTheme*>        themeMap;
  std::vector<CFuiWindow*>        winList;        // Drawing priority
  CFuiTextPopup                   *slew;
  CFuiTextPopup                   *notep;					// Virtual pilot
  CFuiTextPopup                   *note1;         // User note
  CFuiTextPopup                   *notex;         // Export state
  CFuiTextPopup                   *notec;         // Crash windows
  CFuiTextPopup                   *help;          // For gauge help
  //-------------------------------------------------------------------------
  float           dT;             // Time since last draw
  //-------------------------------------------------------------------------
  unsigned char   xPos;           // Original screen position
  unsigned char   yPos;           // Original screen position
  unsigned char   widn;           // Window numbering
  float         tClick;           // Time clicked
  CFuiWindow   *wTop;             // Top windows
	CFuiWindow	 *wCap;							// Mouse Capturing window
  char          wLast[8];         // Last clicked
  //-------------------------------------------------------------------------
  //----- Constructors / destructor -----------------------------------------
public:
  CFuiManager();
 ~CFuiManager();
public:
  void Init (void);
  void Cleanup (void);
  Tag  GenTag();
	Tag  GetaTag(U_INT *x, U_INT *y);
	void						 ExportMessage(char *msg);
  //----- CFuiManager methods ------------------------------------------------
  CFuiWindow*      CreateFuiWindow(Tag id,int lim=0);
  CFuiWindow*      CreateOneWindow(Tag id,int lim=0);
  //--------------------------------------------------------------------------
  bool             DestroyFuiWindow (Tag id);
  void             ToggleFuiWindow  (Tag id);
  bool             IsWindowCreated  (Tag id);
  CFuiTheme*       GetTheme (Tag tag);
  CFuiThemeWidget* GetThemeWidget (Tag tag, std::string name);
  void             ActivateWindow(CFuiWindow *win);
  void             AddFuiWindow (Tag windowID, CFuiWindow *window);
  //--------------------------------------------------------------------------
  void    HideWindow(CFuiWindow *w);          // Hide window
  void    ShowWindow(CFuiWindow *w);
  //--------------------------------------------------------------------------
  void    EnterDrawing();
  void    Draw ();
  void    ExitDrawing();
	void		DialogError(char *msg, char *from = 0);
  void    DrawOnlyNotices();
  void    DrawNoticeToUser  (char *text,float time);
	void		PilotToUser();
  void    DrawHelp          (float time,int x, int y);
  void    ScreenResize();
  //--------------------------------------------------------------------------
  // Mouse tracking methods
  bool   MouseMove (int x, int y);
  bool   MouseClick (int x, int y, EMouseButton button);
  bool   DoubleClick(int mx,int my,EMouseButton button);
  bool   MouseStopClick (int x, int y, EMouseButton button);
  bool	 MouseToWind(int mx, int my, EMouseButton bt);
  // Keyboard input method (CFuiTextField only ?)
  bool  KeyboardInput(U_INT key);

  // Miscellaneous
  CFuiWindow*    GetFuiWindow (Tag window);
  void           Print (FILE *f);
  //--------------------------------------------------------------------------
public:
  // Access functions for applications to get/set widget state
  void  SetComponentText (Tag window, Tag component, char* text);
	void	SetBigFont();
	void	ResetFont();
	//--------------------------------------------------------------------------
	char    *PilotNote()					{return notep->Buffer();}
  //--------------------------------------------------------------------------
  void    SetNoticeFont(SFont *f) {note1->ChangeFont(f);}
  void    RazCrash()          {notec->RazActive();}
  void    SetCrash()          {notec->SetActive();}
	void		CaptureMouse(CFuiWindow *w)	{wCap	= w;}
	void		CaptureRelease()						{wCap = 0;}
  //--------------------------------------------------------------------------
  CFuiTextPopup *GetCrashNote() {return notec;}
  //--------------------------------------------------------------------------
  char    *HelpBuffer()  {return help->GetText();}
	//--------------------------------------------------------------------------
	U_INT		GetTexOBJ()		{return xOBJ;}
  //--------------------------------------------------------------------------
protected:
  CFuiComponent*  FindComponent (Tag windowID, Tag id);
  void            RemoveFromDisplay(CFuiWindow *win);
  void            MoveOnTop(CFuiWindow *win);

};
//=============================================================================================

//
// User message function.  The user message is a short, timed message that is displayed
//   in the lower-right hand corner of the screen.
//
void DrawNoticeToUser (char* text, float timeout);

//  sdk: a user message function shown in amessage box APIMessageBox
void MsgBoxNotify (Tag id, Tag component, EFuiEvents event, EFuiEvents subevent);


//=============================================================================================
// FUI Window Subtypes
//=============================================================================================

class CFuiConfirmQuit : public CFuiWindow
{
public :
  CFuiConfirmQuit (Tag id, const char* filename);
  void NotifyChildEvent (Tag id, Tag component, EFuiEvents event);
};
//=============================================================================================

class CFuiOptionsVideo : public CFuiWindow
{
public :
  CFuiOptionsVideo (Tag id, const char* filename);
  void NotifyChildEvent (Tag id, Tag component, EFuiEvents event);
};

#endif // FUI_H
//=================================END OF FILE =====================================================
