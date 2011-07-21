/*
 * FuiUser.h
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
 
#ifndef FUIUSER_H
#define FUIUSER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Include/Fui.h"
#include "../Include/FuiParts.h"
#include <list>
#include <vector>
#include <stack>
#include <map>
#include <string>
#include <stdio.h>

//===================================================================================
// ECW class declarations
//===================================================================================
class CCameraRunway;
class CPIDbox;
class CRouteEXT;
//===================================================================================
// Forward declare all class types
//===================================================================================
class CFuiComponent;
class CFuiPicture;
class CFuiButton;
class CFuiCloseButton;
class CFuiMiniButton;
class CFuiZoomButton;
class CFuiWindowTitle;
class CFuiWindow;
class CFuiTheme;
class CFuiThemeWidget;
class CFuiPage;
class CFloatMenu;
//================================================================================
// CFuiVectorMap
//================================================================================
typedef enum {
  QUADRANT_NW = 0,
  QUADRANT_NE = 1,
  QUADRANT_SW = 2,
  QUADRANT_SE = 3
} EFuiQuadrant;

//==================================================================================
//  CLASS Navaid Detail to edit Navaid parameters
//==================================================================================
class CNavaid;                              // Forward declaration
class CWayPoint;
//---------------------------------------------------------------
class CFuiNavDetail: public CFuiWindow
{ //--------------Attributes ------------------------------------
protected:
    CObjPtr  Po;                            // Object smart pointer
    CNavaid *nav;                           // Related NAV
    QTYPE   type;                           // Type VOR/NDB
    U_SHORT wptNo;                          // WayPoint number
    SMessage mesg;                          // Tune message
  //-------------Methods ----------------------------------------
public:
  CFuiNavDetail(Tag idn,const char *filename, int lim = 0);
  virtual ~CFuiNavDetail() {};
          void  Initialize(CmHead*obj,U_SHORT type,U_SHORT No);
  //----inline methods ------------------------------------------
  inline  void     SetWayPointNo(U_SHORT No) {wptNo  = No;}
  inline  U_SHORT  GetWAyPointNo()           {return wptNo;}
  //-------------------------------------------------------------
  void    NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
  void    TuneRadioNav();
};

//================================================================================
//  CFuiVectorMap
//================================================================================
class CmHead;                                  // Forward declaration
class CNavaid;
class CDbCacheMgr;
class CObjPtr;
class COldFlightPlan;
class CWayPoint;
//================================================================================
//  Structure for Radio description
//================================================================================
typedef struct {
    int   kind;                               // Radio index
    float freq;                               // Radio frequency
    char  txt[24];
} SRADIO_TYPE;
//================================================================================
//  Structure for list of Radio
//================================================================================
typedef struct {
  char        dep;                                // start index in menu
  char        tot;                                // total do not xceed
  char        ind;                                // Current entry
  SRADIO_TYPE tab[12];                            // room for 12 radios
} RADIO_LST;
//================================================================================
class CFuiVectorMap : public CFuiWindow
{
  #define VMAP_MENU_SIZE (17)
	//---------------------------------------------------------------------
	#define VM_SCALE 800                // Nmber of pixel for the current nautical miles zoom
  //--------------coordinate definition ---------------------------------
  typedef struct {  U_SHORT w2;       // Half wide              
                    U_SHORT h2;       // Half height
  } COORD;
  //---------------Airport stack ----------------------------------------
  enum VM_ANB { TYP01 = 1,
                TYP02 = 2,
                TYP03 = 3,
                TYP04 = 4,
                TYP05 = 5,
                TYP06 = 6,
                TYP07 = 7,
                TYP08 = 8,
                TYP09 = 9,
                TYP10 = 10,
                APNBR = 11
  };
  //------------Other bitmap stack -------------------------------------
  //  Note: VOR and DME are ordered as xType index allocated
  //  by DataBase Cache Manager.  Dont change it without changing
  //  also the index computing in CDbCacheMgr
  //  The index is used to get the right bitmap to draw a Navaid
  //--------------------------------------------------------------------
  enum VM_BMP { UNAVAID   = 0,            // Unknown navaid
                VOR___    = 1,            // VOR CDbCacheMgr allocated
                TACAN_    = 2,            // TACAN    dito
                VORTAC    = 3,            // VORTAC   dito
                VORDME    = 4,            // VORDME   dito
                NDB___    = 5,            // NDB      dito
                NDBDME    = 6,            // NDB-DME  dito
                WPTRED    = 7,
                WPTORA    = 8,
                WPTYEL    = 9,
                WPTGRN    = 10,
                WPTBLK    = 11,
                VOTHER    = 12,            // Other vehicle
                USERVEH   = 13,            // User vehicle
                BM_NBR    = 14,
  };
  //---------FSlot format for screen table -------------------------
  typedef struct  { CmHead *Obj;       // Pointer to object
                    short       sx;       // x screen center
                    short       sy;       // y screen center
  } SC_SLOT;
  //-----Screen table for displayed objects ------------------------
  typedef struct {
        pthread_mutex_t		mux;					          // Mutex for lock
        U_SHORT           xRec;                   // Half size bitmap
        U_SHORT           yRec;                   // Half size bitmap
        U_SHORT           NbSlot;                 // Number of slots
        SC_SLOT          *xSlot;                  // Store index
        SC_SLOT           sList[NBR_OBJ_DBC];     // Entry table
  } SC_TABLE;

  //--------------------------------------------------------------------
public:
  // Constructors / destructor
   CFuiVectorMap (Tag windowId, const char* winFilename);
  ~CFuiVectorMap (void);
  void    InitMenu();
  //-------------Event notification -------------------------------------
  void    NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
  void    NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn);
  void    NotifyMenuEvent(Tag idm, Tag itm);
  //--------------------------------------------------------------------
  // CFuiComponent methods
  virtual void        Draw (void);
  //---------------------------------------------------------------------
protected:
  int  GetScreenCoordinates(CmHead *obj, int& x, int& y, int bm = 0,int cor = 0);
  void        LoadAptBitmap(char *art,VM_ANB no);
  void        LoadOthBitmap(char *art,VM_BMP no);
  void        RelocateBox(CFuiGroupBox* box,short ht);               
  //--------------Mouse & Popup menu Functionss --------------------------
	bool				ClickWptOBJ(int mx,int my,EMouseButton button);
  bool        MovePopMAP(int mx, int my);
  bool        InsideClick (int x, int y, EMouseButton button);
  bool        StopClickInside(int x, int y, EMouseButton button);
  bool        InsideMove(int x, int y);
	bool				OpenPOP(int mx,int my);
	bool				OpenWptOBJ(int mx,int my);
  bool        OpenPopOBJ(int mx,int my);
  bool        OpenPopDOC(int mx,int my);
  bool        OpenPopLST(int mx,int my);
  bool        OpenPopRWY(int mx,int my);
  bool        OpenWptMEN(int mx,int my);
  //-------MENU EDITION --------------------------------------------------
	int					SetRWYends(char k);
  int         SetRWYitem(char k);
  int         SetLITitem(char k);
  int         SetCOMitem(char k);
  int         SetILSitem(char k);
  int         EditILS(ILS_DATA *dat, int k);
  void        EditPopITM();
  void        EditPopAPT(CmHead *obj);
  void        EditPopDistance(int mx,int my);
  void        EditCoordinates(int mx,int my);
  void        AddToFlightPlan();
  int         ClosePopMenu();
  //-------NOTIFICATIONS ------------------------------------------------
  void        ZoomLess();
  void        ZoomPlus();
  void        TrnsPlus();
  void        TrnsLess();
  //--------------Clip functions ----------------------------------------
  bool        ClipVOR(int vx,int vy);
  bool        ClipOBJ(int vx,int vy,int bx, int by);
  //--------------Screen table management -------------------------------
  void        ResetScreenTable(void);
  void        EnterScreenTable(CmHead *obj,int x,int y);
  CmHead*     LookForScreenHit(short x,short y);
  inline void InitScreenRect(int x, int y) {sTable.xRec = (x>>1);sTable.yRec = (y>>1);}
  inline CmHead* GetScreenObj(void){return (hSlot)?(hSlot->Obj):(0);}
  //--------------Draw functions ----------------------------------------
  void        DrawLabel(int fnt,int x,int y,CmHead *obj);
  void        DrawNDB();
  void        DrawVOR();
  void        DrawAPT();
  void        DrawILS();
  void        DrawRose(CNavaid *vor,int xc, int yc);
  void        DrawRadial(CNavaid *vor,int xc, int yc);
  void        Scale();
  void        DrawMark(int type, int xc, int yc, U_INT col);
  void        DrawRoseDir(int xc, int yc, U_INT col);
  //----------Runway drawing -----------------------------------
  void        Teleport();
  void        DrawDiagram();
  void        OpenMetar(CmHead *obj);
  bool        ClickDOC(int mx,int my);
  bool        ClickRWY(int mx,int my);
  bool        MoveDOC(int mx,int my);
  bool        MoveRWY(int mx,int my);
	bool				MoveWPT(int mx,int my);
  int         SearchDOC();
  int         ClickMapMENU(short ln);
  int         ClickDocLIST(short ln);
  int         ClickDocMENU(short ln);
  int         ClickRwyMENU(short ln);
	int					ClickWptMENU(short ln);
	int					ClickWptOBJM(short itm);
  int         ClickRadLIST(int k);
  void        ClickLitITEM();
	int					StartonRWY(short itm);
	int					CreateWPT();
  //----------ILS helper ------------------------------------------------
  void        IlsPixel(int No,int x,int y);
  void        UpdateILS();
  //---------------------------------------------------------------------
  void        DrawTopView();
  void        DrawByCamera(CCamera *cam);
  //---------------------------------------------------------------------
  void        NotifyResize(short dx,short dy);
  //----------Interface to flight plan ---------------------------------
public:
  void        DrawRoute(CRouteEXT &org,CRouteEXT &ext);
	void				DrawWayPoint(CWPT *wpt);
	void				WaypointMoved();
  //--------------------Attributes -------------------------------------
protected:
  SVector     vmapOrient;                   ///< User vehicle orientation
  //-------Drawing item size --------------------------------------------
  float       Zoom;                         ///< Zoom level. In nm for 800 pixels
  float       Diag;                         // Screen Diagonal In nautical miles
  float       MaxNM;                        // Maximum nautical miles (squared)
  float       pixNM;                        // Pixels per nautical mile * 128
  char        eScal[16];                    // Zoom value
  U_SHORT     Mark10;                       // 10° Marker size
  U_SHORT     Mark05;                       // 5°  Marker size
  U_SHORT     RoseRd;                       // Rose radius
  float       RoseZm;                       // Rose zoom factor
  //-------Options -------------------------------------------------
  COption     option;
  //-------Screen Table --------------------------------------------
  SC_TABLE      sTable;                   // Screen table
  SC_SLOT      *hSlot;                    // Selected slot
  //-------Flight plan parameters ----------------------------------
  CFPlan       *fPlan;                       // Flight Plan
  //--------Math parameters ----------------------------------------
  CRoseMatrix mRose;                        // Rotation Matrix
  //--------Airport stock of bitmaps --------------------------------
  CBitmap*    AptBMAP[APNBR];               // Array of airport bitmaps
  COORD       AptSIZE[APNBR];               // Array of dimensions
  //--------Tuning message ----------------------------------------
  SMessage mesg;                          // Tune message
  //-------Other stack o bitmap -----------------------------------
  U_INT       ilsBMP;                       // Ils director
  CBitmap*    OthBMAP[BM_NBR];              // Array of bitmap
  COORD       OthSIZE[BM_NBR];              // Array of dimension
  //-----color section ------------------------------------------
  char        Fade;                         // Fading action
  U_INT       green;                        // Green color
  U_INT       grlim;                        // Green limit                   
  U_INT       white;
  U_INT       black;
  U_INT       red;
  U_INT       yel;
  //---------Floating menu -------------------------------------
  char        namHD[32];                    // Full name
  char        disHD[32];                    // Distance header
  char        frqHD[20];                    // Frequency
	char       *cMENU[VMAP_MENU_SIZE];			  // item descriptor (char*)
	void       *pMENU[VMAP_MENU_SIZE];				// Item related
  FL_MENU     smen;                         // menu descriptor
  CFuiPage   *MyPop;                        // Popup
  U_CHAR      selOPT;                       // Selected option
  int         xOrg;                         // Pop Origin
  int         yOrg;                         // Pop Origin
  //----Nearest airport ----------------------------------------
  CAirport   *napt;                         // Nearest airport
  float       ndis;                         // Distance
  //----Frame counter ------------------------------------------
  U_CHAR      Frame;
  U_CHAR      Turn;
  //------------Group boxes ------------------------------------
  CFuiGroupBox *rBox;                       // Rose button
  CFuiGroupBox *zBox;                       // Zoom button
  CFuiGroupBox *bBox;                       // Background
  //-----Editing Attribute -------------------------------------
  CDbCacheMgr *dbc;                         // Data base  cache Manager
  CObjPtr       Focus;                      // smart ptr to focus
  CMonoFontBMP *fLab[2];                    // label font
  U_INT         Intv[2];                    // Font space
  U_INT         cLab[2];                    // Color Label
  //------ Coordinates under cursor -----------------------------
  SPosition   geop;                         // geo Position
  SPosition   wpos;                         // Waypoint position
  //------Diagram management ------------------------------------
  S_IMAGE     DocInfo;                      // Diagram image
  char        dStat;                        // Draw state
  float       rScale;                       // Scale factor to draw runways
  //------Secondary view managemnt ------------------------------
  void       *obView;                       // Airport object to view
  CAirport   *obAirp;                       // Airport description
  int         heading;                      // Plane heading (°)
  CCameraRunway *Cam;                      // Airport camera
  //-----ILS Drawing --------------------------------------------
  CILS        *aILS;                        // actual ils
  U_SHORT      rILS;                        // Range
  TC_SPOINT    oILS;                        // ILS origin in pixel
	//----- Viewport ------------------------------------------------
  VIEW_PORT				vp;
};
//====================================================================================
//  TELEPORT:  Window setting the teleport position
//====================================================================================
class CFuiTeleport: public CFuiWindow
{ //------------------Attributes ----------------------------------
  CFuiDirectory *dir;
 public:
   CFuiTeleport(Tag idn, const char *filename);
  ~CFuiTeleport(void);
  //----------------Notifications ---------------------------------
  void NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
};
//====================================================================================
//  DIRECTORY:  Window for all directories
//====================================================================================
class CDbCacheMgr;
class CFuiDirectory: public CFuiWindow
{ //------------------Attributes ----------------------------------
  U_CHAR              Lock;                 // Lock indicator
  CDataBaseREQ        Req;                  // Integrated request
  CDbCacheMgr         *dbc;                 // Cache manager
  CFuiLabel           *hBox;                // Head label
  CFuiCheckbox        *cBox;                // Checkbox
  CFuiTextField       *nBox;                // name box
  CFuiTextField       *iBox;                // ident box
  CFuiButton          *sBut;                // Select button
  //---------------------------------------------------------------
  CFuiWindow          *cWin;                // Calling window
  //-----------------Country management ---------------------------
  CListBox            ctyBOX;               // Country list box
  CCtyLine            ctyLIN;               // Fixed country slot
  //-----------------State Management -----------------------------
  CListBox            staBOX;               // State Box
  CStaLine            staLIN;               // Fixed state slot
  //-----------------Object management ----------------------------
  QTYPE               oType;                // Object type
  CmHead          *selOBJ;               // Detailled object
  CListBox            objBOX;               // Object box
  Tag                 Order;                // Request order
  //-----------------Airport ownership ----------------------------
  char              *apOWN[6];
  //-----------------Selection Popup ------------------------------
  U_SHORT            selOpt;                // Selected option
  CFuiPopupMenu     *selPop;                // Selection Popup
  FL_MENU             mSEL;                 // Menu selector
  //------------------Window method -------------------------------
public:
   CFuiDirectory(Tag idn, const char *filename);
  ~CFuiDirectory(void);
  void      Draw();
  //--------Dialog management -------------------------------------
  bool      RegisterMe(CFuiWindow *win);
  bool      ResetCaller();
  //--------Countries management ----------------------------------
  void      FillCTYlist();
  char     *GetCountry(char *ckey);
  //--------State management --------------------------------------
  void      FillSTAlist();
  void      SortSTAlist();
  void      SetFilter(QTYPE type);
  //---------Object management ------------------------------------
  void      SortOBJlist();
  //---------Airport management -----------------------------------
  void      SearchAirportByCountry(QTYPE type);
  void      SetOtherAptData(CAptLine *slot);
  //---------Navaid management ------------------------------------
  void      SearchNavaidByCountry(QTYPE typ);
  void      SetOtherNavData(CNavLine *slot);
  char     *GetNavType(char type);
  //--------Waypoint management -----------------------------------
  void      SearchWaypointByCountry(QTYPE type);
  void      SetOtherWptData(CWptLine *slot);
  //----------------Notifications ---------------------------------
  void      NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
  void      NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn);
  void      NotifyCaller(CmHead *obj,QTYPE type);
  void      NotifyResize(short dx,short dy);
  void      EventCountryBox(Tag itm,EFuiEvents evn);
  void      Search();
  //---------------Specific methods -------------------------------
  void      RetreiveCTYkey();
  void      Teleport();
  void      ShowMetar();
  //---------------Detail management ------------------------------
  bool      GetVORobject();
  bool      GetNDBobject();
  bool      GetAPTobject();
  bool      GetNAVObject();
  //---------------Selection popup --------------------------------
  void      SetPopTitle(U_SHORT No);
  //---------------Database management ----------------------------
  void      AddDBrecord(void *rec,DBCODE cd);
  void      EndOfRequest(CDataBaseREQ *req);
};


//==================================================================================
// CFuiPID to tune the autopilot controlers
//==================================================================================
class CFuiPID: public CFuiWindow {
  //-------ATTRIBUTES ----------------------------------------
  CFuiLabel       *wLAB;                // Label
  CFuiLabel       *wLB1;                // Lable 1
  CFuiLabel       *wLBP;                // Parameter label
  //-----------------------------------------------------------
  CFuiList        *wPID;                // List of controllers
  CListBox         pBox;                // PID list
  //-------Increment items  -----------------------------------
  CFuiTextField   *wINC;                // Increment box
  CFuiButton      *mINC;                // Minus increment
  CFuiButton      *pINC;                // Plus  increment
  //------- Parameters items ---------------------------------
  CFuiTextField   *wVAL;                // Value box
  CFuiButton      *mVAL;                // Minus increment
  CFuiButton      *pVAL;                // Plus  increment
  //----------------------------------------------------------
  CFuiList        *wPRM;                // List of parameters
  CListBox         mBox;                // PRM list
  //------Selected PID ---------------------------------------
  CPIDbox         *sPID;
  double           Inc;                 // Current increment
  double           Val;
  Tag              Prm;                 // Current parameter
  //-------METHODS -------------------------------------------
public:
  CFuiPID(Tag idn, const char *filename);
  void          AddPID(CPIDbox *pid);
  void          SelectPID();
  void          EditInc();
  void          Increment();
  void          Decrement();
  void          SelectPRM();
  void          ChangePRM(double fac);
  void          Draw();
  //------NOTIFICATIONS --------------------------------------
  void  NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
};
//==========================================================================================
//  Class CFuiChart to display sectional charts
//==========================================================================================
class CFuiChart : public CFuiWindow {
  //--- ATTRIBUTES --------------------------------------------
  S_IMAGE MapInfo;
  //--- METHODS------------------------------------------------
public:
  CFuiChart(Tag idn, const char *filename);
 ~CFuiChart();
  //------------------------------------------------------------
  void  OpenChart(char *fn);
  //------------------------------------------------------------
  void  Draw();
  //----Mouse events -------------------------------------------
  bool  InsideClick (int mx, int my, EMouseButton button); 
  bool  InsideMove(int mx,int my); 
  bool  StopClickInside(int x, int y, EMouseButton button);
};
//===========================================================================================
//  Class CFuiRadioBand to display radio communication
//  NOTE: This class is instanciated in global situation, but the Draw method is called
//  in the context of CFuiManager as it is visible in every view
//===========================================================================================
class CFuiRadioBand {
  //---------------Internal state --------------------------------------------------------
  enum  BANDST    { RB_STOP   = 0,                          // Inactive
                    RB_INIT   = 1,                          // Start emiting
                    RB_DSP1   = 2,                          // Display part 1
                    RB_DSP2   = 3,                          // Display part 2
                    RB_DEND   = 4,                          // Display end
                    RB_STBY   = 5,                          // Stand by
  };
  //---------------Entries for ATIS radio ----------------example ------------------------
  enum  COM_ATIS  { AT_IDEN   = 0,                          // KSFO
                    AT_TXT1   = 1,                          // Arrival info
                    AT_TIDN   = 2,                          // Romeo
                    AT_TIMH   = 3,                          // 13H00
                    AT_HIDN   = 4,                          // ZULU
                    AT_TXT2   = 5,                          // Automated weather observations. Wind
                    AT_WIND   = 6,                          // calm
                    AT_TXT3   = 7,                          // Visibility
                    AT_MVIS   = 8,                          // 50 
                    AT_TXT4   = 9,                          // miles. Temperature is
                    AT_TEMP   = 10,                         // 24
                    AT_TXT5   = 11,                         // degrees. Dewpoint is
                    AT_DEWP   = 12,                         // 20
                    AT_TXT6   = 13,                         // degres. Altimeter is 
                    AT_ALTI   = 14,                         // 29.92
                    AT_SMAL   = 15,                         //---Item number for small airports ---
                    //-----For control centers -----------------------------------
                    AT_TXT7   = 15,                         // Notice to airmem...
                    AT_CLDY   = 16,                         // 118.20 (clearance delivery)
                    AT_TXT8   = 17,                         // Advise on initial contact you have information 
                    AT_IDNT   = 18,                         // Romeo
                    AT_FULL   = 19,                         // Array dimension
  };
  //--------------Attribute ------------------------------------------------------
  TXT_LIST  sTxt;                                           // Text description
  SFont    *font;                                           // Fonte
  CFont    *fnts;                                           // Real font
  SSurface *surf;                                           // Surface
  short     wd;                                             // wide
  short     ht;                                             // Height
  short     wCar;                                           // Character wide
  short     NbCar;                                          // Number of characters
  U_INT     back;                                           // Back color
  U_INT     black;
  BANDST    state;                                          // Active inactive state
  //---------For time Management -------------------------------------------------
  float     timTM;                                          // Minute timer
  SDateTime	lTime;                                          // Local time
  U_CHAR    tInfo;                                          // Time info edited
  U_CHAR    nInfo;                                          // timer info 
  //---------Editied fields ------------------------------------------------------
  char      eFrq[8];                                        // Clearance frequency
  char      hour[8];                                        // Time edited
  char      eVis[4];                                        // Visibility in nm
  char      eTmp[4];                                        // Temperature in deg
  //---------For editing radio band ----------------------------------------------
  short     xScr;                                           // Screen index
  short     cPos;                                           // new char position
  short     nCar;                                           // Number of characters to screen
  short     sEnd;                                           // screen right starting position
  short     sDeb;                                           // screen left limit
  //---------For tuned radio comm -------------------------------------------------
  CObjPtr   rCOM;                                           // Current radio com
  //-----------------------ATHIS text --------------------------------------------
  char    *atis[AT_FULL];
  //-------------Method ----------------------------------------------------------
public:
   CFuiRadioBand();
  ~CFuiRadioBand();
  //-----------------------Edit --------------------------------------------------
  void  SetATIStext();
  //------------------------Size parameters --------------------------------------
  void  Resize();
  //------------------------Drawing method ---------------------------------------
  void  Draw();
  //-----------------------Time slice --------------------------------------------
  void      TimeSlice(float dT);
  void      UpdateInfo();
  void      UpdateTime();
  bool      TuneTo(CCOM *com);
  bool      ModeATIS(CCOM *com);
  COM_ATIS  EditATISdata(CAirport *apt);
  //------------------------------------------------------------------------------
  inline void Close() {delete this;}
};
//==================================================================================
//  CFuiPlot.  A window to plot curves
//==================================================================================
class CPlotter {
  CFont         *font;                        // Font to use
  CFuiCanva     *cnv;                         // Related canva
  SSurface      *srf;                         // Surface
  CFuiPopupMenu *pop;                         // list of data
  //------------User vehicle -----------------------------------
  CVehicleObject *mveh;
  //-----------colors ------------------------------------------
  U_INT         red;
  U_INT         green;
  //------------------------------------------------------------
  FL_MENU       pMEN;                         // Plot menu
  PLOT_PP       pPRM;                         // Plot parameter
  //------------------------------------------------------------
  short         hwd;                          // Horizontal width
  short         vht;                          // Vertical height
  //------------------------------------------------------------
  short         x0;                           // Left column
  short         y0;                           // bottom line
  short         y1;                           // upper line
  //------------------------------------------------------------
  short         yL;                           // Real label line
  short         yT;                           // Real top line
  short         yB;                           // Real bottom line
  //------------------------------------------------------------
  short         xD;                           // Start column
  short         xF;                           // End column
  //------------------------------------------------------------
  float         yPix;                         //    "" 
  //------------------------------------------------------------
  float         ppxu;                         // Pixel per X unit
  float         ppyu;                         // Pixel per Y unit
  float         scale;                        // Scale
  //-------------------------------------------------------------
public:
  CPlotter();
  //-------------------------------------------------------------
  void      Init(CFuiCanva *cnv, CFont *fnt,CFuiPopupMenu *p,CVehicleObject *vh);
  int       Start(Tag itm);
  int       Header();
  int       NewScale(float sc);
  void      DrawPixel(short x0,short x1);
  void      Shift(short sx)     {LeftShiftSurface(srf,sx,xD,0);}
  //---------------------------------------------------------------------------
  //  Get the real y line
  //  Real Y is inverted:
  //  A value of 10 will be printed on line HT - 10 + 1 where HT is the screen 
  //  height
  //---------------------------------------------------------------------------
  inline  int     RealY(int y)  {return (y1 - y + 1);}
  inline  int     GetWidth()    {return hwd;}
  inline  int     GetXD()       {return xD;}
  inline  int     GetXF()       {return xF;}
  inline bool     On()          {return (pPRM.dpnd != 0);}
};
//=====================================================================================
//  Scaler structure
//=====================================================================================
typedef struct {
  float          scale;
  CFuiTextField *tex;
  CPlotter      *plot;
} PLOT_SCALE;
//==================================================================================
//  CFuiPlot.  A window to plot curves
//==================================================================================
class CFuiPlot: public CFuiWindow
{ //------------Attributes ---------------------------------------
  CFuiGroupBox  *gBOX;
  CFuiPopupMenu *pop1;                          // list of data
  CFuiPopupMenu *pop2;                          // list of data
  CFuiCanva     *cnv1;                          // Canva 1
  CFuiCanva     *cnv2;                          // Canva 1
  //---- Scaling buttons for plotter 1 ---------------------------
  CFuiButton    *upr1;                          // More on scale 1
  PLOT_SCALE     psc1;                          // Scale 1
  CFuiButton    *lwr1;                          // Less on scale 1
  //---- Scaling buttons for plotter 2 ---------------------------
  CFuiButton    *upr2;                          // More on scale 2
  PLOT_SCALE     psc2;                          // Scale 2
  CFuiButton    *lwr2;                          // Less on scale 2
  //---- First plotter -------------------------------------------
  char          strt;                           // Started plotter
  CPlotter      p1;                             // Plotter n°1
  CPlotter      p2;                             // Plotter n°1
  //------------Synchro parameters -------------------------------
  float         time;                           // Timer
  float         stime;                          // Time scale
  //--------------------------------------------------------------
  float         ppxu;                           // Pixel per second
  short         xD;                             // Start curve
  short         xF;                             // End curve
  float         xPix;                           // Current pixel
  //------------User vehicle -------------------------------------
  CVehicleObject *mveh;
  float         timeScale;                      // Time scale
  //------------Methods ------------------------------------------
public:
  CFuiPlot(Tag idn, const char *filename);
  //--------------------------------------------------------------
  void    Draw();
  short   Shift(short sx);
  short   ComputeX();
  void    Start(CPlotter &p, Tag itm);
  void    ModifyScale(PLOT_SCALE &ps,float mod);
  //-----------NOTIFICATIONS -------------------------------------
  void    NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn);
  void    NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
};
//==================================================================================
//  CFuiVehOption.  A window for vehicle options
//==================================================================================
class CFuiVehOption: public CFuiWindow
{ ///---ATTRIBUTES -----------------------------------------------
  CFuiGroupBox  *aBOX;              // Autopilot group box
  CFuiLabel     *aLAB;              // Autopilot label
  CFuiCheckBox  *rLND;              // Autolanding option
  CFuiCheckBox  *rDIS;              // Autodisconnect
  CFuiCheckBox  *cDET;              // Crash detection
  CFuiCheckBox  *hlPN;              // help panel
  CFuiCheckBox  *smBX;              // Smoke box
  CFuiCheckBox  *shBX;              // Shadow box
  CFuiCheckBox  *psBX;              // Position box
  CFuiCheckBox  *aeBX;              // Aero Vectors

  //------------User vehicle -------------------------------------
  CVehicleObject *mveh;
  //--------------------------------------------------------------
  void    NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
  //---METHODS ---------------------------------------------------
public:
  CFuiVehOption(Tag idn, const char *filename);
  void    ReadFinished();
  char    GetOption(U_INT p);
};
//==================================================================================
//  CFuiOptions.  A window for globals options
//==================================================================================
class CFuiOptions: public CFuiWindow
{ //----ATTRIBUTES -----------------------------------------------
  CFuiCheckBox  *gBOX;              // Gamma correction option
  //--------------------------------------------------------------
  //----METHODS --------------------------------------------------
public:
  CFuiOptions(Tag idn, const char *filename);
  //--------------------------------------------------------------
  bool  CheckNoFile();
  void  SaveGamma();
  void  SetGamma(double g);
};
//==================================================================================
//  CFuiCamControl.  A window to control a camera
//==================================================================================
class CFuiCamControl: public CFuiWindow 
{	//--- ATTRIBUTES --------------------------------------------
	CCamera     *cam;											// Cockpit camera
	//--- Rotation control	-------------------------------------
	CFuiLabel   *lrot;										// Label rotation
	CFuiButton	*rtlf;										// Rotate left
	CFuiButton  *rtrt;										// Rotate right
	CFuiButton  *rtup;										// Rotate up
	CFuiButton  *rtdn;										// Rotate down
	//--- Range buttons -----------------------------------------
	CFuiLabel   *lrng;										// Label range
	CFuiButton  *rnut;										// Outward
	CFuiButton  *rnin;										// In ward
	//----Zoom items --------------------------------------------
	CFuiLabel   *lzom;										// Zoom label
	CFuiButton  *zmup;										// Zoom out
	CFuiButton  *zmin;										// Zoom in
	//----METHODS -----------------------------------------------
public:
	CFuiCamControl(Tag idn, const char *filename);
	//-----------------------------------------------------------
	void	EditValues();
  //-----------------------------------------------------------
	bool	InsideClick (int x, int y, EMouseButton buttons);
	bool	MouseMove (int x, int y);
	void	NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
	//-----------------------------------------------------------
	void	Draw();
};
//==================================================================================
//  CFuiCamControl.  A window to edit terrain elevation
//==================================================================================
class CFuiTED : public CFuiWindow
{	//--- ATTRIBUTES --------------------------------------------
	U_INT		warn[2];
	//-----------------------------------------------------------
	CElvTracker     *trak;								// Tracker
	GroundSpot			*spot;								// Ground spot
  CAMERA_CTX       ctx;                 // Original context
	CFuiLabel       *lab1;								// Tile label
	CFuiLabel       *lab2;								// Tile label
	CFuiTextField   *lab3;								// elevation label
	CFuiButton		  *elvp;								// +
	CFuiButton			*elvm;								// -
	//--- Save button -------------------------------------------
	CFuiButton      *sBut;								// Save button
	//--- Red/Green box -----------------------------------------
	CFuiBox         *mBox;								// Modified box
	CFuiLabel       *wrn1;								// warning label
	//----yes and no buttons ------------------------------------
	CFuiButton      *yBut;								// Yes button
	CFuiButton      *nBut;								// No button
	//--- Rabbit camera -----------------------------------------
	CRabbitCamera   *cam;
	//----Patche area -------------------------------------------
	ELV_PATCHE			pbuf;									// Patche buffer
	//--- Update parameters -------------------------------------
	short					  count;								// Modification indicator
	char						mdif;									// reserved
	char						msgw;									// Warning indicator
	std::map<U_INT,U_INT> ldet;						// List of modified tiles
	//--- METHODS -----------------------------------------------
public:
	CFuiTED(Tag idn, const char *filename);
 ~CFuiTED();
 //------------------------------------------------------------
 void   TileNotify(CmQUAD *qd, CVertex *vt);
 void		EnterKey(U_INT tx,U_INT tz);
 //------------------------------------------------------------
 bool   NoAlert();
 void   Warn01();
 void		WarnM1();
 void		SaveAll();
 void		SaveOne(U_INT key);
 bool   WantToClose();
 void		SaveAndQuit(char opt);
 //------------------------------------------------------------
 void		Draw();
 void		EditElevation(float e);
 //------------------------------------------------------------
 void		IncElevation(float dte);
 void		GetElevation();
 //--- Mouse events -------------------------------------------
 bool		MouseCapture (int mx, int my, EMouseButton bt);
 //--- Notifications ------------------------------------------
 void		NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
};
//==================================================================================
//  CFuiDLL.  A generic fui window connected to plugins
//==================================================================================
class CFuiDLL : public CFuiWindow
{ //------------Attributes ---------------------------------------
  //--------------------------------------------------------------
public:
    CFuiDLL (Tag idn, const char *filename);
   ~CFuiDLL ();
    //--------------------------------------------------------------
    void  EventNotify(Tag win,Tag cpn,EFuiEvents evn,EFuiEvents sub );
    //-----------------------------------------------------------------
    void  NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
};

#endif // FUIUSER_H
//=================================END OF FILE =====================================================
