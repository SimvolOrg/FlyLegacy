/*
 * RadioGauges.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005 Chris Wallace
 * copyright 2007 Jean Sabatier
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
#include "../Include/NewGauge.h"
#ifndef RADIOGAUGES_H
#define RADIOGAUGES_H
//======================================================================
class CRadioGauge: public CGauge {
protected:
  //--- ATTRIBUTES --------------------------------------------
	char           PowST;												// Power State
  VStrip				 brow;											  // row of buttons
  float					 clit[4];											// light color
	U_CHAR				 cMsk;												// Clicked type				
  //--------Font parameters -----------------------------------
  U_CHAR         hCar;                        // Char height
  U_CHAR         wCar;                        // Char wide
  U_INT          amber;                       // Amber color
  CFont         *rFont;                       // Radio font
  //--- METHODS -----------------------------------------------
public:
  CRadioGauge(CPanel *mp);
 ~CRadioGauge();
  //-----------------------------------------------------------
  //-----------------------------------------------------------
  virtual void  LightPass()   {;}
	//-----------------------------------------------------------
	bool	BtCopy();
  //-----------------------------------------------------------
  void  Copy(CRadioGauge &rg);
  //-----------------------------------------------------------
  int   Read(SStream *str, Tag tag);
  void  ReadPosition(SStream *str);
  void  ChangeFont(char *fn);
  //-----------------------------------------------------------
  inline int CharHT()     {return hCar;}
  inline int CharWD()     {return wCar;}
	inline U_CHAR State()   {return PowST;}
};
//=========================================================================
#define K55EV_AREA_SZ (28)            // Number of Clicks and buttons
//=========================================================================
// JSDEV* CK55gauge
// NOTE:  This gauge is coupled to the K155 NAV/COM radio subsystem
//=========================================================================
class CK155gauge : public CRadioGauge {
  //-------ATTRIBUTES -----------------------------------------
  CK155radio  *RAD;                           // Radio component
  CGaugeClickArea ca[K55EV_AREA_SZ];          // Click area
  //-------CDI definition -------------------------------------
  short             Ampli;              // CDI amplitude
  short             cdiND;              // Middle position
  //-------METHODS --------------------------------------------
public:
  CK155gauge(CPanel *mp);                              // Constructor
  int   Read (SStream *stream, Tag tag);      // Read parameters
  void  ReadFinished (void);
  void  GetK155();
  void  CopyFrom(SStream *stream);
	void	CollectVBO(TC_VTAB *vtb);
  //--------drawing routines ----------------------------------
  void    Draw();
  void    DrawField(RADIO_FLD *fd);
  void    DrawBUT(U_CHAR astate);
  void    DrawCOM();
  void    DrawNAV();
  void    DrawCDI();
  //--------Mouse drivers -------------------------------------
  int           AnyComEvent(int mx, int my);
  int           AnyNavEvent(int mx, int my);
  EClickResult  MouseClick (int x, int y, int buttons);
  ECursorResult MouseMoved (int x, int y);
  EClickResult  StopClick  ();
  void          ClickField(int na, short dir);
	//------------------------------------------------------------
  inline CGaugeClickArea *GetClickArea(char No) 
    { return (No < K55EV_AREA_SZ)?(ca+No):(0);}
};
//=========================================================================
// JSDEV* CKR87gauge
// NOTE:  This gauge is coupled to the KR87 ADF radio subsystem
//=========================================================================
class CKR87gauge : public CRadioGauge {
  //--------ATTRIBUTES ----------------------------------------
  CKR87radio  *RAD;                           // Radio component
  CGaugeClickArea ca[K87CA_SIZE];             // Click area
  static RADIO_HIT  *Kr87HIT[];               // KR87 driver table
  static RADIO_DSP   Kr87FLD[];               // Field list
  //--------METHODS -------------------------------------------
public:
  CKR87gauge(CPanel *mp);
  int   Read (SStream *stream, Tag tag);      // Read parameters
  void  ReadFinished (void);
  void  GetKR87();
  void  CopyFrom(SStream *st);
	void	CollectVBO(TC_VTAB *vtb);
  //-----------------------------------------------------------
  void  DrawField(RADIO_DSP *dsp);
  void  DrawBUT(U_CHAR pwr);
  void  DrawADF();
  void  Draw();
  //----Mouse handler -----------------------------------------
  ECursorResult MouseMoved (int mx, int my);
  int           AnyEvent(int mx,int my);
  EClickResult  MouseClick (int x, int y, int buttons);
  EClickResult  StopClick  ();
  void          ClickField(int na, short dir);
  //-----------------------------------------------------------
  inline CGaugeClickArea *GetClickArea(char No) 
    { return (No < K87CA_SIZE)?(ca+No):(0);}

};
//=========================================================================
//  KT76 FIELD DEFINITION
//=========================================================================
enum KT76Fields {
  KT76_NULL_FD = 0,                         // Not used
  KT76_ACTF_D1 = 1,                         // Active frequency Digit 1
  KT76_ACTF_D2 = 2,                         // Active frequency Digit 2
  KT76_ACTF_D3 = 3,                         // Active frequency Digit 3
  KT76_ACTF_D4 = 4,                         // Active frequency Digit 4
  //--------------------------------------------------------------------
  KT76_IDEN_BT = 5,                         // Ident button
};
//============================================================================
//  CKT76gauge:  Gauge for transponder KT76
//============================================================================
class CKT76gauge: public CRadioGauge {
protected:
  //----Attributes -------------------------------------------
  CKT76radio  *RAD;                           // Radio component
  CGaugeClickArea ca[KT76CA_MAX];             // Click area
  U_CHAR         ppwr;                        // Power button position
  //-----------------------------------------------------------
  static RADIO_DSP KT76Fldtab[];              // Field table
  //-------------------------------------------------------------
public:
  CKT76gauge(CPanel *mp);
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  void  GetRadio();
  void  CopyFrom(SStream *stream);
	void	CollectVBO(TC_VTAB *vtb);
  //------------------------------------------------------------
  int           AnyEvent(int mx,int my);
  EClickResult  MouseClick (int x, int y, int buttons);
  ECursorResult MouseMoved (int mx, int my);
  void          ClickField(int na, short dir);
  //------------------------------------------------------------
  void  DrawField(RADIO_DSP *dsp);
  void  DrawTextFields();
  void  DrawBUT(U_CHAR pwr);
  //------------------------------------------------------------
  void  Draw();
  //-----------------------------------------------------------
  inline CGaugeClickArea *GetClickArea(char No) 
    { return (No < KT76CA_MAX)?(ca+No):(0);}

};
//============================================================================
//  K89gauge
//  Note:  This Gauge is coupled to the CK89 GPS radio subsystem
//============================================================================
class CK89gauge: public CRadioGauge {  //CBitmapGauge {
  typedef void(CK89gauge::*DisFN)();               // Display function
  typedef int (CK89gauge::*ClkFN)(int mx,int my);  // Handler
  //---------------------------------------------------------------------
  //  Data Member
  //---------------------------------------------------------------------
  CK89gps          *GPS;                // GPS susbsytem
  CK89gauge        *oGage;              // Original gauge
  //-------RADIO control ------------------------------------------------
  Tag               radi;               // Radio Tag
  //-------CLICK PARAMETERS  ---------------------------------------------
  CGaugeClickArea   ca[K89_DIM];        // All click area
  static  ClkFN     HndTAB[4];          // vector to Handler
  static  DisFN     DshTAB[4];          // Vector to display handler
  //-------DISPLAY PARAMETERS ---------------------------------------------
  U_SHORT           mskFS;              // Flasher mask
  U_SHORT           Tim01;              // timer 01
  //-----------------------------------------------------------
  U_CHAR            scrLN[5];           // screen Line table
  U_SHORT           undTAB[12];         // Underline table
  //---------Inputs fields -------------------------------------
  K89_LETFD         letTAB[12];         // Input field table
  K89_PAGFD         pagTAB[12];         // Simple page fields
  K89_REPFD         repTAB[12];         // Table of rept fields
  U_CHAR            WtyFD;              // Working field type
  U_CHAR            WnoFD;              // Working field number
  //----------Display surface control -------------------------
  short             yBase;              // Base line
  short             xBase;              // Right side divider
  short             yLed;               // Small leds line
	short							Ampli;							// CDI width
	short							Base;								// CDI Base
  //---------- Start of inside click area ---------------------
  short             xClk;               // x start click
  short             yClk;               // y start click
  //----------Font parameters ---------------------------------
  U_INT             yelow;              // Yellow color
  //---------------------------------------------------------------------
  //  Methods 
  //---------------------------------------------------------------------
public:
  CK89gauge(CPanel *mp);                                // Constructor
  int       Read (SStream *stream, Tag tag);  // Read parameters
  void      CopyFrom(SStream *stream);        // Dupplicate values
  void      ReadFinished();                   // End of read
  void      IniLF(char No,char *fd);          // Init letter field
  void      IniRP(char No,int *nm);           // Init Repeatable fields
  void      IniPF(char No);                   // Init Page Field
	//---------------------------------------------------------------------
	void			CollectVBO(TC_VTAB *vtb);
  //--------Return fields -----------------------------------------------
  K89_LETFD *GetLetField(short No);
  //-------MOUSE MANAGERS -----------------------------------------------
  ECursorResult     MouseMoved (int x, int y);
  EClickResult      StopClick ();
  EClickResult      MouseClick(int x, int y, int buttons);
  int               AnyCHRclick(K89_LETFD *fd,int mx,int my);
  int               AnySPFclick(K89_PAGFD *fd,int mx,int my);
  int               AnyFLDclick(K89_REPFD *fd,int mx,int my);
  int               AnyLETclick(int mx, int my);
  int               AnyGENclick(int mx,int my);
  int               AnyNULclick(int mx,int my);
  void              ClickField(int na, short dir);
  //---------------------DRAW methods  ----------------------------------
  void      Draw();
  void      DrawGPSwindow();                  // Open GPS
  void      DrawAllDisplay();                 // Draw full display
  void      DrawLSPDisplay();                 // Draw left part
  void      DrawRSPDisplay();                 // Draw right part
  void      DrawLDRDisplay();                 // Draw both parts
  void      DrawButton(int fr);               // Draw button
  void      DrawCDIneedle();                  // Draw CDI needle
  //-----------------------------------------------------------
  inline CGaugeClickArea *GetClickArea(char No) 
    { return (No < K89_DIM)?(ca+No):(0);}
  //-----------------------------------------------------------
};
//==============================================================================
// CBKAP140Gauge
//  For auto pilot panel KC140
//==============================================================================
#define K140_NCA (23)                         // Total click area
//=================================================================
class CBKAP140Gauge : public CRadioGauge {
  //---ATTRIBUTES -------------------------------------------------
  Tag          ident;                         // Panel Ident
  CKAP140Panel  *PAN;                         // Panel component
  CGaugeClickArea   ca[K140_NCA];             // All click area 
  //-----------------------------------------------------------
  static RADIO_DSP   fieldTAB[];               // Field list
  //-----------------------------------------------------------
  SMessage msgp;                              // To locate panel
  //-----------------------------------------------------------
  U_CHAR            tca;                      // Total click area
  U_CHAR            Power;                    // Power state
  //----------------------------------------------------------------
public:
  CBKAP140Gauge (CPanel *mp);
 ~CBKAP140Gauge();
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  void  Get140Panel();
  void  CopyFrom(SStream *stream);        // Dupplicate values
	void	CollectVBO(TC_VTAB *vtb);
  //--- CGauge methods ------------------------------------------
  virtual const char* GetClassName (void) { return "CBKAP140Gauge"; }
  ECursorResult MouseMoved (int x, int y);
  EClickResult  MouseClick (int x, int y, int buttons);
  EClickResult  StopClick  ();
  //-------------------------------------------------------------
  void          Draw ();
	void					DrawAmbient();
  void          DrawField(RADIO_DSP *dsp);
  void          DrawButton(char st);

protected:
};
//=======================END OF FILE ======================================================================
#endif RADIOGAUGES_H

