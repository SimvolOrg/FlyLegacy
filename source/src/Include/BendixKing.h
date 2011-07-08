/*
 * Bendix.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2007 Chris Wallace
 * Copyright 2005 Laurent Claudet
 * Copyright 2007 Jean Sabatier
 *
 *  Part of the PID controller is implemented using a formulea documented
 *  by Roy Ovesen in the FlightGear system, a project also under the GNU GPL agreement
 *  Thanks to Roy for his permission to use it. Jean Sabatier
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

#include "../Include/FlyLegacy.h"
#include "../Include/Subsystems.h"
//====================================================================================
#ifndef BENDIXKING_H
#define BENDIXKING_H

//=====================================================================
//  Class CK155radio for Bendix / KING K155 700A
//  NOTES:  Generic RADIO attributes are  in the CRadio level
//=====================================================================
class CK155radio : public CRadio {
  friend class CK155gauge;
  friend class C_K155gauge;
  //---------State function -------------------------------------------
  typedef int (CK155radio::*StaFN)(K55_EVENT evn);
  //-------------------------------------------------------------------
  float       gTimer;                           // Global timer
  //---------ATTRIBUTES for COM management ----------------------------
  U_CHAR      nChanl;                           // Current chanel
  U_CHAR      cTurn;                            // Chanel turn
  //---------ATTRIBUTES for NAV management ----------------------------
  U_CHAR      Power;                            // Power button
  U_CHAR      cdiST;                            // CDI state
  U_CHAR      tMode;                            // Timer mode
  U_SHORT     nTimer;                           // Elapse timer
  float       cdiDEV;                           // Direction deviation
  short       nCount;                           // Timer increment
  //----------CDI parameters ----------------------------------------
  char        cdiFLD[24];                       // CDI field 
  short       cdiDIM;                           // CDI size
  //----------Chanel frequency table --------------------------------
  RADIO_FRQ   chnFQ[32];                        // Chanel freq table 
  U_SHORT     Tim01;                            // Chanel control timer
  //----------Mouse control -----------------------------------------
  short      mDir;                              // Mouse Direction (+1,-1)
  //---------Vector Table -------------------------------------------
  static StaFN        staTAB[K55_STA_SIZ];      // Vector table
  static RADIO_HIT  *Kr55HIT[];                 // COM driver table
  //---------METHODS ------------------------------------------------
public:
   CK155radio::CK155radio();                    // Constructor
   void    ReadFinished();
   virtual const char* GetClassName (void) { return "CK155Radio"; }
   virtual EMessageResult  ReceiveMessage (SMessage *msg);
  //-------------------------------------------------------------------
  void    InitChanel();
  //------utility --------------------------------------------------
  void    PopChanelFreq(short nc);
  void    PushChanelFreq(short nc);
  void    ChangeChanel(short inc,short state);
  void    SwapComTurn();
  void    SetGlide(SMessage *msg);
  int     RefreshCDI();
  int     SetOBS(short inc);
  int     SetILS();
  int     SetFlag();
  int     SetStationBearing();
  int     RefreshBearing();
  int     SetStationRadial();
  int     RefreshRadial();
  int     EditTimer();
  int     ChangeTimerValue(short inc);
  int     ChangeTimerMode();
  int     PowerOFF();
  //------COM state functions --------------------------------------
  int     COMenterNormal();                     // Enter normal state
  int     COMstateNormal(K55_EVENT evn);        // Normal Mode
  int     COMenterSelect();                     // Enter chanel select mode
  int     COMstateSelect(K55_EVENT evn);        // Chanel select state
  int     COMenterPgMode();                     // Enter programing mode
  int     COMstatePgMode(K55_EVENT evn);        // Chanel programing mode
  //------NAV state functions ----------------------------------------
  int     NAVupdateTimer();                     // Update NAV timer
  int     NAVenterNormal();                     // Enter normal state
  int     NAVstateNormal(K55_EVENT evn);        // Normal mode
  int     NAVenterCDI();                        // Enter CDI mode
  int     NAVstateCDI(K55_EVENT evn);           // CDI state
  int     NAVenterBRG();                        // Enter bearing mode
  int     NAVstateBRG(K55_EVENT evn);           // Bearing mode
  int     NAVenterRAD();                        // Enter radial mode
  int     NAVstateRAD(K55_EVENT evn);           // Radial mode
  int     NAVenterTIM();                        // Enter timer mode
  int     NAVstateTIM(K55_EVENT evn);           // Timer state
  //------Dispatcher --------------------------------------------------
  int     Dispatcher(K55_EVENT evn);            // Dispatching 
  void    TimeSlice (float dT,U_INT FrNo);      // Time slice
  //-------------------------------------------------------------------
  inline short    GetCDIdim() {return cdiDIM;}
};
//=====================================================================
//  KR87 FIELD DEFINITION
//=====================================================================
enum KR87Fields {
  KR87_NULL_FD = 0,                         // Not used
  KR87_ACTF_D1 = 1,                         // Active frequency Digit 1
  KR87_ACTF_D2 = 2,                         // Active frequency Digit 2
  KR87_ACTF_D3 = 3,                         // Active frequency Digit 3
  KR87_ACTF_D4 = 4,                         // Active frequency Digit 4
  KR87_SBYF_D1 = 5,                         // Standby frequency Digit 1
  KR87_SBYF_D2 = 6,                         // Standby frequency Digit 2
  KR87_SBYF_D3 = 7,                         // Standby frequency Digit 3
  KR87_SBYF_D4 = 8,                         // Standby frequency Digit 4
  KR87_ANTL_FD = 9,                         // ANtena led
  KR87_ADFL_FD =10,                         // ADF led
  KR87_BFOL_FD =11,                         // BFO LED
  KR87_FRQL_FD =12,                         // Freq LED
  KR87_FLTL_FD =13,                         // FLT LED
  KR87_ETLD_FD =14,                         // ET LED
  KR87_TIMN_FD =15,                         // Timer Minute
  KR87_TISC_FD =16,                         // Timer second
  KR87_ACTF_D5 =17,                         // Active frequency Digit 5
  KR87_SBYF_D5 =18,                         // Standby frequency Digit 4
  KR87_SIZE_FD =19,                         // Number of fields
};
//---------EVENT definitions ------------------------------------------
enum K87_EVENT  {
    K87EV_NULL,                                  // 0 Not used
    K87EV_ACT1  = 1,                             // Active digit 1
    K87EV_ACT2  = 2,                             // Active digit 2
    K87EV_ACT3  = 3,                             // Active digit 3
    K87EV_ACT4  = 4,                             // Active digit 4
    K87EV_ACT5  = 5,                             // Active digit 4
    K87EV_SBY1  = 6,                             // Standby digit 1
    K87EV_SBY2  = 7,                             // Standby digit 2
    K87EV_SBY3  = 8,                             // Standby digit 3
    K87EV_SBY4  = 9,                             // Standby digit 4
    K87EV_SBY5  = 10,                            // Standby digit 4
    K87EV_POWR  = 11,                            // Power button
    K87EV_INNR  = 12,                            // Inner tune
    K87EV_OUTR  = 13,                            // Outter tune
    K87EV_BADF  = 14,                            // Button ADF
    K87EV_BBFO  = 15,                            // Button BFO
    K87EV_BFRQ  = 16,                            // Button FREQ
    K87EV_BFLT  = 17,                            // Timer
    K87EV_BRST  = 18,                            // Reset button
    K87EV_TMIN  = 19,                            // Time::MINUTE
    K87EV_TSEC  = 20,                            // Time::SECOND
    K87EV_BEAT  = 25,                            // Clock top
    //----------------------------------------------------------------
    K87CA_SIZE  = 25,                            // Number of click area
};
//--------------------------------------------------------------------
//  Define Radio state
//--------------------------------------------------------------------
#define K87_POW_OF 0
#define K87_FRQ_ON 1
#define K87_TIM_ON 2
//=====================================================================
//  KR87 ADF radio
//=====================================================================
class CKR87radio  : public CRadio {
  friend class CKR87gauge;
  //---------State function -------------------------------------------
  typedef int (CKR87radio::*StaFN)(K87_EVENT evn);
  static StaFN       adfn[];                        // State Dispatcher
  //-----ATTRIBUTES ---------------------------------------------------
  U_CHAR      aPower;                               // Power button state
  U_CHAR      sPower;                               // Power state
  U_CHAR      rState;                               // Radio State
  RADIO_FRQ   tempf;                                // Temporary frequency
  RADIO_FRQ   ActFRQ;                               // Active frequency
  RADIO_FRQ   SbyFRQ;                               // Standby frequency
  RADIO_FLD   adfTAB[KR87_SIZE_FD];                 // Control fields
  short       comp;                                 // Compass range [0-360°];
  //----Data ----------------------------------------------------------
  char  aDat1[2];                                   // Active digit 1
  char  aDat2[2];                                   // Active digit 2
  char  aDat3[2];                                   // Active digit 3
  char  aDat4[2];                                   // Active digit 4
  char  aDat5[2];                                   // standby digit 1
  char  aDat6[2];                                   // standby digit 2
  char  aDat7[2];                                   // standby digit 3
  char  aDat8[2];                                   // standby digit 4
  char  aDate[4];                                   // ET LED
  char  aDatf[4];                                   // Minute
  char  aDatg[4];                                   // Second
  char  aData[2];                                   // Active  fract part
  char  aDatb[2];                                   // Standby fract part
  //----Timer management ----------------------------------------------
  float   gTime;                                    // Global timer
  U_SHORT eTime;                                    // Elapse timer
  U_SHORT Timer;                                    // FLT timer
  short   uCount;                                   // Count mode
  U_CHAR  tMode;                                    // timer mode
  //-----------Flasher --------------------------------------------
  U_CHAR  mskFS;                                    // Flasher mask
  //----Navaid --------------------------------------------------------
  CNavaid    *NDB;                                  // Tuned navaid
  //-----METHODS ------------------------------------------------------
public:
  CKR87radio();
  int       Read (SStream *stream, Tag tag);
  void      ReadFinished();
  EMessageResult  ReceiveMessage (SMessage *msg);
  void      TimeSlice (float dT,U_INT FrNo);      // Time slice
  bool      CheckPowerON(); 
  bool      CheckPowerOF();
  //-----Utilities ----------------------------------------------------
protected:
  void      ChangeTime(short inc);
  void      UpdateTimer();
  void      EditTimer();
  void      EditActFRQ();
  void      EditSbyFRQ();
  void      ModifyFreq(RADIO_FRQ *loc,short inc);
  void      ModifyFrac(RADIO_FRQ *loc,short inc);
  void      SwapFreq();
  void      SwapLED();
  void      SwapBFO();
  void      SetRadial(SMessage *msg);
  void      TuneToFreq(SMessage *msg);
  void      UpdateCompass(short inc);
  //-----State functions ----------------------------------------------
  int       K87eventDEC(K87_EVENT evn);             // Common decoder
  int       K87enterPOF();                          // Enter power off   
  int       K87statePOF(K87_EVENT evn);             // Power off state
  int       K87enterFRQ();                          // Enter frequency mode
  int       K87stateFRQ(K87_EVENT evn);             // Frequency state
  int       K87enterTIM();                          // Enter timer mode
  int       K87leaveTIM();                          // Leave timer mode
  int       K87stateTIM(K87_EVENT evn);             // Timer state
  int       K87eventFLT();                          // FLT event
  int       K87eventRST();                          // Event reset
public:
  //-------------------------------------------------------------------
  inline    void        SetAutopower()        {aPower = 1;}
  inline    U_CHAR      GetMask()             {return mskFS;}
  inline    U_CHAR      GetState()            {return rState;}
  inline    RADIO_FLD  *GetField(U_CHAR No)   {return adfTAB + No;}
  //-------------------------------------------------------------------
  int       Dispatcher(K87_EVENT evn);            // Dispatching 
};

//===========================================================================
//  DEFINE KLN89 EVENTS
//===========================================================================
enum K89_EVENT  {
  K89_NUL = 0,
  K89_MSG = 1,
  K89_OBS = 2,
  K89_ALT = 3,
  K89_NRS = 4,
  K89_DIR = 5,
  K89_CLR = 6,
  K89_ENT = 7,
  K89_CSR = 8,
  K89_PAG = 9,
  K89_APT = 10,
  K89_VOR = 11,
  K89_NDB = 12,
  K89_INT = 13,
  K89_USR = 14,
  K89_ACT = 15,
  K89_NAV = 16,
  K89_FPL = 17,
  K89_CAL = 18,
  K89_SET = 19,
  K89_OTH = 20,
  K89_DPY = 21,
  K89_POW = 22,
  K89_DIM = 23,
  K89_EOR = 24,
  K89_FD1 = 25,
  K89_FD2 = 26,
  K89_FD3 = 27,
  K89_FD4 = 28,
  K89_FD5 = 29,
  K89_FD6 = 30,
  K89_CLOCK = 31,                   // Draw event
  K89_BEAT  = 32,
};
//---------GPS STATES -----------------------------------------
enum K89_STATE {
    K89_PWROF  = 0,
    K89_PWRNA  = 1,
    K89_PWRNB  = 2,
    K89_PWRNC  = 3,
    K89_PWRND  = 4,
    K89_PWRNG  = 5,
    K89_PWRNH  = 6,
    K89_PWRNJ  = 7,
    K89_PWRNK  = 8,
    K89_APTP1  = 9,
    K89_CSRM1  = 10,
    K89_CSRM2  = 11,
    K89_APTP2  = 12,
    K89_APTP3  = 13,
    K89_APTP4  = 14,
    K89_APTP5  = 15,
    K89_APTP9  = 16,
    K89_RADP1  = 17,
    K89_RADP2  = 18,
    K89_NAVP1  = 19,
    K89_NAVP2  = 20,
    K89_NAVP3  = 21,
    K89_NAVP4  = 22,
    K89_DIRP1  = 23,
    K89_DIRP2  = 24,
    K89_DIRP3  = 25,
    K89_NRSP1  = 26,
    K89_FPLP1  = 27,
    K89_WPTP1  = 28,
    K89_ALTP1  = 29,
    K89_ALTP2  = 30,
    K89_MSGP1  = 31,
    K89_CALP1  = 32,
    K89_NULP1  = 33,
    K89_SETP1  = 34,
    K89_STANB  = 35,
};
//-------------------------------------------------------------
//  Define number of states
//-------------------------------------------------------------
class CGPSrequest;
class CAirport;
class CRunway;
class CCOM;
class CILS;
class CNavaid;
class CWayPoint;
//-------------------------------------------------------------
//  Define K89 input field structure
//-------------------------------------------------------------
  //----------Define a Field click structure -----------------
  typedef struct {  char    *aChar;               // Alpha field
                    U_SHORT *aDisp;               // Display field
                    //--Screen coordinates -------------------------
                    short    Left;                // Screen left pos
                    short    Right;               // Screen right pos
                    char     Line;                // Line position
                    U_CHAR   Next;                // Next field
                    U_CHAR   Type;                // Next field type
                    //----------------------------------------------
                    U_CHAR   Iden;                // Field ident
                    U_CHAR   aFld;                // Alternate field
                    U_CHAR   cPos;                // Letter position
                    U_CHAR   xFld;                // Field index

  } K89_LETFD;
//-----------------------------------------------------------------------
//  Define K89 input repeatable field structure
//-----------------------------------------------------------------------
  //----------Define a repeatable field click structure -----------------
  typedef struct {  int      *aNber;                // Numeric Field
                    //--Screen coordinates -------------------------
                    short     Left;                 // Screen left pos
                    short     Right;                // Screen right pos
                    char      Line;                 // Line position
                    U_CHAR    Next;                 // Next field
                    U_CHAR    Type;                 // Next field type
                    //-Event to post -------------------------------
                    int       Event;
                    //--Update values ------------------------------
                    int       vIncr;                // Increment value / 0
                    int       nbMin;                // Minimum value
                    int       nbMax;                // Maximum value

  } K89_REPFD;
//-----------------------------------------------------------------------
//  Define K89 click page field structure
//-----------------------------------------------------------------------
  //----------Define a simple field click structure -----------------
  typedef struct {  //--Screen coordinates -------------------------
                    short     Left;                // Screen left pos
                    short     Right;               // Screen right pos
                    char      Line;                // Line position
                    U_CHAR    Next;                // Next field
                    U_CHAR    Type;                // Next field type
                    //-Event to post -------------------------------
                    int       Event;
  } K89_PAGFD;
//-----------------------------------------------------------------------
//  Define K89 contexte
//-----------------------------------------------------------------------
  typedef struct {  U_SHORT     Display[24*4];      // Display field
                    U_CHAR      aState;             // Current state
                    U_CHAR      cPage;              // Current page
                    U_CHAR      HndNO;              // Handler event
                    U_CHAR      SnoFD;              // Starting filed
                    U_CHAR      StyFD;              // Starting type
                    U_CHAR      curPOS;             // Cursor position
                    K89_LETFD  *LetFD;              // current let field
  } K89_CTX;
//=======================================================================
//  Definitions for KLN98NGPS
//=======================================================================
#define K89_MODE_NUL  0
#define K89_MODE_APT  1
#define K89_MODE_VOR  2
#define K89_MODE_NDB  3
#define K89_MODE_INT  4
#define K89_MODE_USR  5
#define K89_MODE_ACT  6
#define K89_MODE_NAV  7
#define K89_MODE_FPL  8
#define K89_MODE_CAL  9
#define K89_MODE_SET  10
#define K89_MODE_OTH  11
//------------------Define Letters parameters --------------------------
//          0         1         2
// Letters  0123456789012345678901234
//          @KSFO @SAN FRANCISO INT
#define K89_IDSIZ 5
#define K89_OWSIZ 11
//-----------------Define strip indices --------------------------------
#define K89_0_INDEX 1
#define K89_A_INDEX (1 + 10)
//-----------------Define mode positions -------------------------------
#define K89_PAGE_NUL  0
#define K89_PAGE_APT  6
#define K89_PAGE_VOR 17
#define K89_PAGE_NDB 30
#define K89_PAGE_INT 42
#define K89_PAGE_USR 54
#define K89_PAGE_ACT 68
#define K89_PAGE_NAV 79
#define K89_PAGE_FPL 91
#define K89_PAGE_CAL 102
#define K89_PAGE_SET 115
#define K89_PAGE_OTH 127
//--------------Flash time in fps ---------------------------------------
#define K89_FLASH_STEP1 40
#define K89_FLASH_STEP2 60
#define K89_BEAT_TEMPO  15
//--------------CDI -----------------------------------------------------
#define K89_CDI_MIDDLE 33
#define K89_CDI_AMPLI  66
//-------------CDI FLAG -------------------------------------------------
#define K89_FLAG_NONE 0
#define K89_FLAG_FROM 1
#define K89_FLAG_TO   2
#define K89_FLAG_MASK 3
//-------------Attributes -----------------------------------------------
#define K89_ATT_FLASH 0x0500
#define K89_ATT_FLREQ 0x0100
#define K89_ATT_INVRT 0x0200
#define K89_ATT_YELOW 0x0400
#define K89_ATT_NBOLD 0x0800
#define K89_ATT_NONE  0x0000
#define K89_ATT_CLEAR 0x0020
#define K89_ATT_MASKC 0x00FF
#define K89_ATT_NOCAR 0xFF00
#define K89_ATT_ONCAR 0xFFFF
//------------Display dimensions ----------------------------------------
#define K89_NBR_LINE 4
#define K89_NBR_COLN 24
#define K89_NBR_CHAR (K89_NBR_LINE * K89_NBR_COLN)
#define K89_PR2_COLN 18
//----------Column definitions ------------------------------------------
#define K89_CLN00 0
#define K89_CLN01 1
#define K89_CLN02 2
#define K89_CLN03 3
#define K89_CLN04 4
#define K89_CLN05 5
#define K89_CLN06 6
#define K89_CLN07 7
#define K89_CLN08 8
#define K89_CLN09 9
#define K89_CLN10 10
#define K89_CLN11 11
#define K89_CLN12 12
#define K89_CLN13 13
#define K89_CLN14 14
#define K89_CLN15 15
#define K89_CLN16 16
#define K89_CLN17 17
#define K89_CLN18 18
#define K89_CLN19 19
#define K89_CLN23 23
//----------Line definitions --------------------------------------------
#define K89_LINE0 0
#define K89_LINE1 1
#define K89_LINE2 2
#define K89_LINE3 3
#define K89_LINE4 4
//---------Edit mode -----------------------------------------------------
#define K89_NOR_EDIT 0
#define K89_NRS_EDIT 1
#define K89_ACT_EDIT 2
//------------------------------------------------------------------------
#define K89_ALT_MIN 0
#define K89_ALT_MAX 50000
#define K89_GSP_MIN 40
#define K89_GSP_MAX 950
#define K89_OFS_MIN -99
#define K89_OFS_MAX +99
#define K89_VSI_MIN -2000
#define K89_VSI_MAX +2000
//-----GPS field type ----------------------------------------------------
#define K89_NON_FD 0
#define K89_LET_FD 1
#define K89_REP_FD 2
#define K89_PAG_FD 3
//-----GPS FIELD NUMBER --------------------------------------------------
#define K89_FIELD01 1
#define K89_FIELD02 2
#define K89_FIELD03 3
#define K89_FIELD04 4
#define K89_FIELD05 5
#define K89_FIELD06 6
#define K89_FIELD07 7
#define K89_FIELD08 8
#define K89_FIELD09 9
//------GPS BUTTONS ------------------------------------------------------
#define K89_BUTMSG  1
#define K89_BUTOBS  2
#define K89_BUTALT  3
#define K89_BUTNRS  4
#define K89_BUTDIR  5
#define K89_BUTCLR  6
#define K89_BUTENT  7
#define K89_BUTCSR  8
#define K89_BUTPWR 22
//------------------------------------------------------------------------
#define K89_TIM_INTRO 160
//----VNAV MODE ----------------------------------------------------------
#define K89_VNA_OFF 0
#define K89_VNA_ALT 1
#define K89_VNA_VSI 2
//----GPS MESSAGE NUMBER -------------------------------------------------
#define K89_MSG_OFF 0
#define K89_MSG_VNA 1
//----CALCULATOR MODE ----------------------------------------------------
#define K89_CAL_WPT 0
#define K89_CAL_FPL 1
//----HANDLER NUMBER -------------------------------------------
#define K89_NUL_HANDLER 0
#define K89_LET_HANDLER 1
#define K89_GEN_HANDLER 2
//----DISPLAY HANDLER ------------------------------------------
#define K89_FULL_DISPLAY 0
#define K89_PART_DISPLAY 1
#define K89_OPEN_DISPLAY 2
//=============================================================
#define GPS_MODE_NORMAL (0)       // Normal mode
#define GPS_MODE_DIRECT (1)       // Direct To
//=============================================================
// CK89gps  Subsystem
//=============================================================
class CK89gps : public CDependent {
  friend class CK89gauge;
  friend class CFuiKLN89;
public:  CK89gps (CVehicleObject *v,CK89gauge *g);
  CK89gps::~CK89gps();
  //-------Definee vector format --------------------------------
  typedef int (CK89gps::*EorFN)();               // End of request
  typedef int (CK89gps::*PreFN)();               // Return function
  //-------------------------------------------------------------
  virtual const char* GetClassName (void) { return "CK89gps"; }
  //--------------------------------------------------------------
  void              TimeSlice (float dT,U_INT FrNo);
  void              SetHandler(U_CHAR No,U_CHAR Nf,U_CHAR Tf);
  void              SetRadio(Tag r);
  //---------Text handling ---------------------------------------
  void              StoreText(const char *txt,short lin,short col);
  void              StoreText(U_SHORT *des,char *org);
  void              FlashWord(char *txt,short lin,short col);
  void              StoreChar(char  car,short lin,short col);
  void              FlashChar(char car,short lin, short col,short cp);
  void              StoreBlnk(U_CHAR nbr,short lin,short col);
  void              FlashON(U_SHORT *pos);
  void              FlashOF(U_SHORT *pos);
  void              SetFlashOF(short lin, short col,short siz);
  void              SetFlashON(short lin, short col,short siz);
  void              FlashText(char *txt,U_SHORT *pos);
  void              FlashOFF();
  void              ReFlash();
  bool              IsChar(char car,short lin, short col);
  void              SetNewLetter(K89_LETFD *fd,U_CHAR xl);
  //---------Editing and drawing--------------------------------
  void              RazDisplay();
  void              ClearRightDisplay();
  void              ClearRightLine(int lin);
  int               DrawCDI(short lin,short col);
  //------------------------------------------------------------
  void              EditLegETE(char *edt,float dis,float speed);
  void              EditLegETA(char *edt,float dis);
  void              EditFTime(char *edt);
  void              EditDepartTime(char *edt);
  void              EditLatitude(float lat,short lin,short col);
  void              EditLongitude(float lon,short lin,short col);
  void              EditRadial(char *dst,float rad,U_CHAR flag);
  void              EditDistance(char *edt,float dis);
  int               EditCAPandDIS(CmHead *obj);
  void              EditMagDev(float dev,short lin,short col);
  void              EditTrack();
  void              EditFuel();
  U_CHAR            GetStripIndex(char let);
  int               EditTimeZone(char *edt,float lon);
  int               PageHeader(short NoPage);
  int               HeaderEnt(char *txt,short col);
  int               EditBearing(float cap,short col,short lin);
  int               ClearIdent();
  void              DupHeader();
  int               UpdateRPTfield();
  int               CheckNewLetter();
  int               ClearField(K89_LETFD *fd);
  void              GetFilterArg(K89_LETFD *fd);
  //-------Airport editing -------------------------------------
  void              EditAPTheader();
  void              EditAPTdata();
  void              RefreshNRSairport();
  int               EditAPTpage01();
  int               EditAPTpage02();
  int               EditAPTpart3();
  int               BackAPTpage05();
  int               BackAPTpage04();
  int               EditAPTctry(short lin);
  int               EditOBJpart2(CmHead *obj);
  //--------Runway edition -------------------------------------
  void              EditRunwaySlot(short ln,CRunway *rwy);
  int               EditNoRunway();
  int               EditFwRunway();
  int               EditBkRunway();
  int               StartAPTp4ForW();
  int               StartAPTp4Back();
  //---------CCom edition -------------------------------------
  void              EditComSlot(short ln,CCOM *com);
  int               EditNoCom();
  int               EditFwCom();
  int               EditBkCom();
  int               StartAPTp5ForW();
  int               StartAPTp5Back();
  //---------CILS edition -------------------------------------
  void              EditIlsSlot(short ln,CILS *ils);
  int               EditNoILS();
  int               EditFwILS();
  int               EditBkILS();
  int               StartAPTp9ForW();
  int               StartAPTp9Back();
  //---------VOR-NDB editing ----------------------------------
  int               EditRADpage01();
  void              EditNAVheader();
  //---------NAV Editing ---------------------------------------
  int               EditNAVpage01();
  int               EditWPTNone();
  int               EditNAVP01L02();
  int               EditFlyDirection();
  int               EditNAVpage02();
  int               EditNAVpage03();
  //--------DIRECT TO editing ---------------------------------
  int               CheckDIRReq();
  int               EditWPTSlot(short nl,CmHead *wpt);
  int               SetIdentity(CmHead *obj);
  int               SetDIRwaypoint();
  int               EditDUPwaypoint();
  int               CursorDUPchange();
  //-------NEAREST  TO editing --------------------------------
  int               CursorNRSchange();
  int               DispatchNRSmode();
  int               EditNRSoptions();
  //-------FLIGHT PLAN ----------------------------------------
  int               EnterFLPpage01();
  int               ChangeFPLnode();
  int               EditFPLoption(char *edt, CWayPoint *wpt);
  int               EditFPLSlot(short ln,CWayPoint *wpt);
  int               EditNoFplan();
  int               EditFlightPage();
  void              EditFPLmark(short lin,CWayPoint *wpt);
  int               ChangeFPLcursor();
  //-------ACTIVE WAYPOINT ------------------------------------
  void              ModeLEG();
  void              ModeOBS();
  void              ClearActiveWPT();
  void              ClearFlightPlanWPT();
  void              SetActiveWPT();
  void              UpdateWaypointData();
  void              SelectActiveWaypoint();
  void              RefreshActiveWPT();
  void              RefreshVNAVpoint();
  //-------INT page -------------------------------------------
  int               EditWPTpage01();
  int               EditWPTAsNRS();
  //-------ALT page -------------------------------------------
  int               RefreshALTpage01();
  int               RefreshALTpage02();
  int               ChangeALTcursor();
  int               GetTargetVnavALT();
  int               GetTimeToVNAV();
  int               GetNextALTWaypoint();
  void              EditVNAVstate();
  void              UpdateVnavVSI(int cur);
  void              InitVnavWPT();
  int               EditVNAVstatus(short lin,short col);
  //--------Message page --------------------------------------
  int               DisplayMSG();
  //--------Calculator page -----------------------------------
  int               RefreshCALpage01();
  int               EditCALpage01();
  int               EditCALpage02();
  int               ChangeCALpage();
  void              EditCALident(K89_LETFD *fd,CmHead *wpt);
  int               SearchCALwaypoint();
  int               SetCALwaypoint();
  int               CheckCALreq();
  void              ComputeCALparameters();
  //---------SET pages ----------------------------------------
  int               EditSETpage01();
  int               EditSETpage02();
  int               EditSETpage03();
  int               EditSETpage04();
  int               EditSETpage05();
  int               EditSETpage06();
  int               EditSETpage07();
  int               EditSETpage08();
  int               EditSETpage09();
  int               EditSETpage10();
  int               EditSETpage11();
  int               ChangeSETpage(char dir);
  //---------Cursor functions ---------------------------------
  int               EnterCSRMode01(PreFN cal);
  int               EnterCSRMode02();
  //---------Working routines ---------------------------------
  int               PowerCUT();
  int               ChangeNRSwpt();
  void              GetAnyWaypoint();
  void              GetNextMatch(K89_LETFD *fd,char opt,char dir);
  char             *GetWPTtype(QTYPE cod);
  void              FormatIdent(CmHead *obj,short lin,short col);
  void              FormatLabel(CmHead *obj,short lin,short col,char *des);
  bool              IsActiveWPT(CmHead *obj);
  int               Round100(int val);
  void              CopyDisplay(U_SHORT *src,U_SHORT *des);
  int               PushContext();
  void              PopContext();
  //---------Transition routines     ---------------------------
  void              InitState(U_CHAR mode);
  int               ChangeMode(K89_EVENT evn);
  int               EnterAPTpage01();
  int               EnterAPTpage02();
  int               EnterAPTpage03();
  int               EnterAPTpage04();
  int               EnterAPTpage05();
  int               EnterAPTpage09();
  int               EnterVORpage01();
  int               EnterNDBpage01();
  int               EnterWPTpage01();
  int               EnterRADpage02();
  int               EnterNAVpage01();
  int               EnterNAVpage02();
  int               EnterNAVpage03();
  int               LeaveDIRmode();
  int               EnterNRSpage01();
  int               EnterALTpage01();
  int               EnterALTpage02();
  int               EnterMSGpage01();
  int               EnterCALpage01();
  int               EnterSETpage01();
  int               EnterNULpage01(U_CHAR msg,U_CHAR mode,U_CHAR page);
  int               EnterACTmode  (CmHead *obj);
  int               EnterDIRpage01(CmHead *obj);
  //---------DUPPLICATE PAGES ----------------------------------
  int               EnterDUPpage02();
  int               EnterDUPpage01(PreFN cal);
  //---------STATE ROUTINE ------------------------------------
  int               PowerOFF(K89_EVENT evn);    // State 00 power off
  int               PowerONa(K89_EVENT evn);    // State 01 power on intro a
  int               PowerONb(K89_EVENT evn);    // State 02 power on intro b
  int               PowerONc(K89_EVENT evn);    // State 03 Diagnostic
  int               PowerONd(K89_EVENT evn);    // State 04 flashing
  int               PowerONg(K89_EVENT evn);    // State 05 Test page flashing
  int               PowerONh(K89_EVENT evn);    // State 06 Test page flashing
  int               PowerONj(K89_EVENT evn);    // State 07 Test page flashing
  int               PowerONk(K89_EVENT evn);    // State 08 Fuel page
  int               APTpage01(K89_EVENT evn);   // State 09 Airport mode Page 1
  int               CSRMode01(K89_EVENT evn);   // State 10 Cursor Mode
  int               CSRMode02(K89_EVENT evn);   // State 11 Cursor mode scan
  int               APTpage02(K89_EVENT evn);   // State 12 Airport mode Page 2
  int               APTpage03(K89_EVENT evn);   // State 13 Airport mode Page 3
  int               APTpage04(K89_EVENT evn);   // State 14 Airport mode Page 4
  int               APTpage05(K89_EVENT evn);   // State 15 Airport mode Page 5
  int               APTpage09(K89_EVENT evn);   // State 16 Airport mode Page 9
  int               RADpage01(K89_EVENT evn);   // State 17 Radio mode Page 1
  int               RADpage02(K89_EVENT evn);   // State 18 Radio mode Page 02
  int               NAVpage01(K89_EVENT evn);   // State 19 NAV mode Page 01
  int               NAVpage02(K89_EVENT evn);   // State 20 NAV mode Page 02
  int               NAVpage03(K89_EVENT evn);   // State 21 NAV mode Page 03
  int               NAVpage04(K89_EVENT evn);   // State 22 NAV mode Page 04
  int               DIRpage01(K89_EVENT evn);   // State 23 DIR mode Page 01
  int               DUPpage02(K89_EVENT evn);   // State 24 DIR mode Page 02
  int               DUPpage01(K89_EVENT evn);   // State 25 DUP mode Page 01
  int               NRSpage01(K89_EVENT evn);   // State 26 NRS mode Page 01
  int               FPLpage01(K89_EVENT evn);   // State 27:FPL mode Page 01
  int               WPTpage01(K89_EVENT evn);   // State 28:WPT mode Page 01
  int               ALTpage01(K89_EVENT evn);   // State 29:ALT mode Page 01
  int               ALTpage02(K89_EVENT evn);   // State 30:ALT mode page 02
  int               MSGpage01(K89_EVENT evn);   // State 31 MSG mode page 01
  int               CALpage01(K89_EVENT evn);   // State 32 CAL mode Page 01
  int               NULpage01(K89_EVENT evn);   // State 33 NUL mode no data page
  int               SETpage01(K89_EVENT evn);   // State 34 SET mode page 01
  //-----------Dispatcher ----------------------------------------
  void   Dispatch(K89_EVENT evn)  {(this->*sfn[aState])(evn);}
  //-----------Radio control -------------------------------------
  float             GetOpeningStep();
  //--------------------------------------------------------------
  void              Probe(CFuiCanva *cnv);
  //-----------inline --------------------------------------------
  inline U_CHAR     GetState()                {return aState;}
  inline U_SHORT    GetFmask()                {return mskFS;}
  inline U_CHAR     GetScreenHandler()        {return DshNO;}
  inline U_SHORT   *GetDisplay()              {return Display;}
  inline bool       NotLastLine(U_CHAR ln)    {return (ln < NbLine);}
  inline U_CHAR     GetNberColn()             {return NbColn;} 
  inline U_CHAR     GetPage()                 {return cPage;}
  inline U_CHAR     GetRightColn()            {return 6;}
  inline U_CHAR     GetFactor()               {return (NbColn - 6);}
  inline U_SHORT   *GetRigthDisplay()         {return (Display + 6);}
  inline U_CHAR     GetRightSize()            {return 18;}
  inline short      GetCDIcoln()              {return cdiCL;}
  inline short      GetCDIline()              {return cdiLN;}
  inline float      GetCDIdev()               {return cdiDEV;}
  inline void       SetMouseDir(char md)      {msDIR = md;}
  inline void       Activity(char a)          {Active = a;}
  //==============Data members ===================================
protected:
  CK89gauge         *Gauge;             // K89 gauge object
  RADIO_VAL         *Radio;             // Master radio
  char               Active;            // Activity
  SMessage           gMsg;
  //----------Vector functions --------------------------------
  typedef int (CK89gps::*StaFN)(K89_EVENT evn);
  static StaFN      sfn[];              // State Dispatcher
  //----------Vector to event functions ------------------------
  EorFN             eorEV;              // End of request
  PreFN             preSF;              // Previous state
  //------------Click Handler  Interface-------------------------
  U_CHAR            HndNO;              // Handler number
  U_CHAR            DshNO;              // Display number
  U_CHAR            StyFD;              // Start field type
  U_CHAR            SnoFD;              // Start field number
  K89_LETFD        *LetFD;              // clicked input field
  K89_REPFD        *RepCK;              // Cliked
  //------------------------------------------------------------
  CGPSrequest      *rGPS;               // Request to datbase
  U_INT             Frame;              // Current Frame
  //---------------CDI parameters -----------------------------
  U_CHAR            pow;                // Power enable
  U_CHAR            cdiIN[3];           // CDI indicator
  U_CHAR            cdiST;              // CDI state
  U_CHAR            cdiFM;              // CDI format
  short             cdiLN;              // CDI line
  short             cdiCL;              // CDI column
  float             cdiDEV;             // Deviation from OBS
  //---------Mode Control -------------------------------------
  static char      *modTAB[];           // GPS Mode header
  static char      *wptTYP[];           // Type of waypoints
  static char      *fplTAB[];           // Flight plan options
  static char      *txtTAB[];           // Text table
  U_CHAR            EdMOD;              // Edit Mode
  U_CHAR            cPage;              // Current mode
  U_CHAR            rCode;              // Request code
  U_CHAR            modOBS;             // OBS mode
  //---------State control ------------------------------------
  U_CHAR            aState;             // Automate state
  U_CHAR            fState;             // Flight State
  U_CHAR            dState;             // Direct to state
  U_CHAR            vState;             // VNAV state
  //-----------Timer parameters -------------------------------
  U_SHORT           Tim01;              // timer 01
  U_SHORT           Beat;               // Beat timer
  U_CHAR            Action;             // Gauge specific action
  //-----------Various data -----------------------------------
  SPosition          aPos;              // Aircraft position
  char              *Flag[4];           // Flags
  //--------- FPL and waypoints--------------------------------
  U_INT             serial;             // Flight plan serial
  short             curNO;              // Candidate WPT No in FPL
  U_SHORT           fpMax;              // Number of wpt in FPL
  U_SHORT           MaxNO;              // Maximum nodes
  U_SHORT           LimNO;              // Limite number
  CFlightPlan      *FPL;                // Flight plan
  CmHead           *cOBJ;               // Current  object
  CmHead           *pOBJ;               // Previous object in queue
  CmHead           *nOBJ;               // Next     object in queue
  //----------Active Waypoint parameters ------------------------
  char              Mode;               // 0=>Base 1=>DirectTo
  GPSpoint          Point[3];           // Stack of waypoints
  //----------------------------------------------------------------
  CmHead           *actWPT;             // Active waypoint
  short             actWNO;             // active wpt No from FPL
  float             actRTE;             // Active waypoint route
  float             actDIR;             // Active waypoint direction
  //----------------------------------------------------------------
  CObjPtr           dWPT;               // Direct to waypoint
  CObjPtr           fWPT;               // Flight plan waypoint
  float             wOBS;               // Waypoint OBS
  float             wDIS;               // Waypoint distance
  float             Speed;              // Aircraft speed
  float             aCAP;               // Aircraft CAP
  //----------Tracking ------------------------------------------
  float             dTRK;               // Desired tracking
  float             aTRK;               // Actual tracking
  //----------String fields  ------------------------------------
  U_SHORT           nProp;              // Ident attribute
  char              Ident[6];           // Object identifier
  char              Name[20];           // Object name
  char              Arg[20];            // Database argument
  //-----Parameters for fields ----------------------------------
  static K89_LETFD  LetPmTAB[];
  static K89_REPFD  RepPmTAB[];
  static K89_PAGFD  PagPmTAB[];
  //---------VNAV fields ---------------------------------------
  int               vTime;              // Time to VNA
  float             vnaDIS;             // Target distance
  float             BaroVL;             // Baro value
  int               AltiTG;             // Altitude value
  int               OffsVL;             // Offset value
  int               GrndSP;             // Ground Speed
  int               VspdVL;             // Vertical speed value            
  short             vnaWNO;             // FPL No for vna WPT
  CObjPtr           vnaWPT;             // VNAV target
  //------------CAL 01 fields ----------------------------------
  char              IdFR[5];            // From field
  char              IdTO[5];            // To field
  int               CalSPD;             // Ground Speed
  int               CalDIS;             // Distance
  int               CalCAP;             // Cap
  int               CalFUS;             // Fuel used
  int               CalFRS;             // Fuel Reserve
  int               CalFFL;             // Fuel flow
  short             CalTNO;             // TO FPL WPT No
  CObjPtr           wptFR;              // From waypoint
  CObjPtr           wptTO;              // To waypoint
  //-----------SET parameters ----------------------------------
  U_CHAR            SetPAG;             // Set Page
  U_CHAR            TimZON;             // Time Zone
  //------------Strip letters ----------------------------------
  char             *strip;              // Letter strip
  U_CHAR            stSIZ;              // Strip size
  char              msDIR;              // Mouse direction
  //-----------GPS DISPLAY ------------------------------------
  U_CHAR            wrnNO;              // WArning number
  U_CHAR            NbLine;             // Number of lines
  U_CHAR            NbColn;             // Number of columns
  U_CHAR            NbChar;             // Number of characters
  U_SHORT           Prop;               // Current attribute
  U_SHORT           Display[24*4];      // Attribut + character
  U_SHORT          *disLN[4];           // Display lines
  U_SHORT          *Flash;              // Flashing field
  U_SHORT           Flsiz;              // Flash size
  char             *msgTAB[4];          // Message table
  char             *wrnTAB[3];          // WArning table
  //----------Other various data ------------------------------
  U_CHAR            OpFlag;             // Flag option
  U_CHAR            OpPos;              // Position option
  U_CHAR            OpFPL;              // FPL option
  U_CHAR            OpNAV;              // NAV option
  U_CHAR            OpCAL;              // Calculator option
  U_CHAR            curPOS;             // Cursor position
  U_CHAR            msgNO;              // Message No
  U_CHAR            OpPAG;              // Calculator page
  CmHead        *Stack[4];           // Stack for selection
  //-----------------------------------------------------------
  K89_CTX           sBuf;              // Context
  //---------GPS subsystem for power control ------------------
  Tag               radi;
  //-----------Flasher --------------------------------------------
  U_SHORT           mskFS;              // Flasher mask
};
//=========================================================================
//  KT76 Fields  DEFINITION
//=========================================================================
enum KT76flds {
  KT76_NULF = 0,              // Not a field
  KT76_ALTS = 1,              // Altitude sign 
  KT76_FLEV = 2,              // Flight level
  KT76_MODE = 3,              // Mode
  KT76_IDN1 = 4,              // Digit 1
  KT76_IDN2 = 5,              // Digit 2
  KT76_IDN3 = 6,              // Digit 3
  KT76_IDN4 = 7,              // Digit 4
  KT76_LAB1 = 8,              // FL label
  KT76CF_MAX = 9,             // MAXIMUM CONTROL FIELDS
};
//=========================================================================
//  KT76 EVENT / CLICK-AREA DEFINITION
//=========================================================================
enum KT76events {
  KT76EV_NUL  = 0,
  KT76EV_DIG  = 1,
  KT76BZ_DIG  = 1,
  //--------------------
  KT76EV_IDN  = 5,
  //--------------------
  KT76EV_BTN  = 6,
  KT76BZ_BTN  = 6,
  //----------------------
  KT76EV_CLR  = 14,
  KT76EV_VFR  = 15,
  //---------------------
  KT76EV_PWR  = 16,
  //--Total CA fields ------
  KT76CA_MAX  = 21,

};
//=========================================================================
//  KT76 BUTTON POSITION
//=========================================================================
enum KT76butpos {
  KT76BT_P0  = 0,     // Power OF
  KT76BT_P1  = 1,     // Stand bye
  KT76BT_P2  = 4,     // Test
  KT76BT_P3  = 5,     // On
  KT76BT_P4  = 6,     // Alt
};
//=========================================================================
//  Transponder logical state
//=========================================================================
enum KT76States {
  XPD_POF   = 0,
  XPD_SBY   = 1,                    // Standby
  XPD_TST   = 2,                    // Test mode
  XPD_PON   = 3,                    // On
  XPD_ALT   = 4,                    // Alt
};
//=========================================================================
//  Transponder AK76
//=========================================================================
class CKT76radio: public CRadio {
friend class CKT76gauge;
friend class CKT76gauge;
  U_CHAR      State;                            // Logical state
  char        xnum;                             // Index in numeric field
  RADIO_FLD   xpdTAB[KT76CF_MAX];               // Control fields
  //-----------------------------------------------------------
  char  aSign[2];                               // Alt Sign
  char  aFlev[6];                               // Flight level
  char  aMode[6];                               // Xponder mode
  char  aIdn1[2];                               // Ident 1
  char  aIdn2[2];                               // Ident 2
  char  aIdn3[2];                               // Ident 3
  char  aIdn4[2];                               // Ident 4
  char  aLab1[4];                               // Lab1
  //-----------------------------------------------------------
  char  ident[4];                               // Current ident
  //----Altimeter message -------------------------------------
  SMessage alti;
  //----Speed limits ------------------------------------------
  float lspd;
  float uspd;
  //------------------------------------------------------------
public:
  CKT76radio();
  int             Read(SStream *st,Tag tag);
  EMessageResult  ReceiveMessage (SMessage *msg);
  void            TimeSlice(float dT,U_INT FrNo);
  //------------------------------------------------------------
  void    EditFLEV(float lev);
  void    SetIdent();
  void    SetVFR();
  int     ModDigit(int xd);
  int     ClearIDN();
  int     IdentVFR();
  int     NewDigit(int bvl);
  //-----------------------------------------------------------
  int     UpdButton();
  int     EnterPOF();
  int     EnterSBY();
  int     EnterTST();
  int     EnterPON();
  int     EnterALT();
  //-----------------------------------------------------------
  int     Dispatcher(KT76events evn,int pm);
  //-------------------------------------------------------------
  int     StatePOF(KT76events evn);
  //-------------------------------------------------------------
  inline  RADIO_FLD  *GetField(U_CHAR No)   {return xpdTAB + No;}
  inline  char        ButtonPos()           {return State;}
};
//=========================================================================
//  Gauge AK76
//=========================================================================
//class CKT76gauge
//====================END OF FILE ================================
#endif // BENDIXKING_H
