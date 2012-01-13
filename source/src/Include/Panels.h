/*
 * Panels.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
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

/*! \file Panels.h
 *  \brief Defines CPanel and related classes for cockpit panel management
 */


#ifndef PANELS_H
#define PANELS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlyLegacy.h"
#include "Gauges.h"
#include <map>
#include <vector>

//====================================================================================
//
// Vehicle panels
//
// The following classes represent static (i.e. fixed, non-interactive) and
//   dynamic (scrolling, interactive) panels for all vehicle types, including
//   ground vehicles, airplanes and helicopters.
//
//====================================================================================
//====================================================================================
typedef enum {
  PANEL_STATIC,
  PANEL_INTERACTIVE
} EPanelType;

typedef struct {
  int   x;
  int   y;
} SPanelOffset;

//==============================================================================
//    Panel light
//===============================================================================
class CPanelLight : public CStreamObject {
public:
  CPanelLight (void);
	CPanelLight(Tag t);
 ~CPanelLight (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);

  // CPanelLight methods
  Tag           GetId (void) { return id; }
  void          PrepareMsg (void);
  void          Update (float b);
  //-------------------------------------------------------------
  inline void   SetBrightness(float b)  {bn =  b;}
  inline float  GetBrightness ()        { return bn; }
	inline void   Alight()  {glMaterialfv(GL_FRONT,GL_EMISSION,col);}
  //-------------------------------------------------------------
protected:
  Tag           id;
  bool          ambientOverride;
  SMessage      msg;
  float         bn;
	float					col[4];					// Emissive color
};


//===============================================================================
// CPanel represents an instance of either a static or interactive panel
// NOTE:  Home positions are not used anymore because when the screen
//        is resized, all panels are set to position (0,0) to avoid scroll
//        limits crossing
//================================================================================
class CRadioGauge;
//================================================================================
class CPanel : public CStreamObject {
  //-------Panel Attributes -----------------------------------------------
protected:
  Tag         id;
  char        filename[64];
  CCockpitManager *pit;
	CVehicleObject  *mveh;
  //--- Interactive panel data members ------------------------------------
  int     x_3Dxy, y_3Dxy;
  int     x_3Dsz, y_3Dsz;
  int     x_isiz, y_isiz;
  //-----------------------------------------------------------------------
  int     xDep;
  int     yDep;
	char		main;						// This is main panel
	char		skip;
	char		trn1;						// Trace n1
  float   xMrg;           // X margin
  float   yMrg;           // Y margin 
  //----Working parameters ------------------------------------------------
  Tag     cmde;                         // Menu key
  //---Gauge under focus --------------------------------------------------
  CGauge *gFocus;                   // Gauge under Focus
  short   Fx;                       // Screen coordinates of focused gauge
  short   Fy;                       //  ""        ""
  CGaugeClickArea *ca;              // Click area under focus
  //---Drawing parameters -------------------------------------------------
  TEXT_DEFN                txPanl;      // Backdrop panel texture
  std::map<Tag,CGauge*>      gage;      // Map of panel gauges
  std::vector<CGauge*>       dgag;      // Vector to draw gauge in defined order
	std::vector<CGauge*>			 sgag;			// Auxilliary gauge
  float                      brit[4];   // Panel brightness
  //---Panel object -------------------------------------------------------
  U_INT                      txOBJ;     // Texture object panel
  U_INT                      gxOBJ;     // Gauge texture
  U_INT                      ngOBJ;     // New gauge texture object
  CPanelLight               *plite;     // Panel light
	CPanelLight								*dlite;			// Default light
	//--- VBO buffer for gauges ---------------------------------------------
	U_INT												gVBO;			// VBO object
	TC_VTAB                    *gBUF;			// VBO buffer
	U_SHORT											gOfs;			// Vector offset
	//--- VBO for dynamic gauges --------------------------------------------
	U_INT												dOfs;			// Dynamic offset
  //--- VBO buffer for panel and bands ------------------------------------
	U_INT												pVBO;			// VBO object
	TC_VTAB                    *pBUF;			// VBO buffer
	char												xlok;			// X band offset
	char												ylok;			// Y band offset
  TC_VTAB                    *pan;			// Panel definition
  TC_VTAB                    *sid;      // Left & right cache
  TC_VTAB                    *top;      // top  & bottom cache
  //-----------------------------------------------------------------------
  int       xScreen, yScreen;   // Window dimensions (TEMPORARY)
  int       xOffset;  // Offsets of top-left corner of panel to top-left
  int       yOffset;  //    corner of screen
  int       xMinLimit, xMaxLimit, yMinLimit, yMaxLimit;
  //-----Mouse management  ------------------------------------------------
  CGauge   *gtk;                        // gauge capturing mouse
	HIT_GAUGE hit;												// Hit parameters 
  EClickResult  track;
  int           buttonsDown;
  // Sound effect source for this panel
  int       sfx_source;
  //-----------------------------------------------------------------------
public:
  std::vector<CDLLGauge*>    dll_gauge; // vector of dl gauges
	//--- METHODS -----------------------------------------------------------
  CPanel (CCockpitManager *p,Tag id, const char* filename);
 ~CPanel (void);

  int   Read (SStream *stream, Tag tag);
	void	SetMain(char m);
	void	AssignLite(Tag t);
  void  LoadTexture(char *fn);
	void	LoadLightMap();
  void  ReadFinished (void);
  void  PrepareMsg(CVehicleObject *veh);
  void  TimeSlice (float dT);
  int   ReadGauge(SStream *stream, Tag tag);
  int   ReadNewGauge(SStream *str, Tag tag);
  //----------------------------------------------------------------------------
  int   VertDisp(int y);
  int   HorzDisp(int x);
  void  MoveVT();
  void  MoveHZ();
  void  ScrollPanel(int mx,int my);
  //----- CPanel methods--------------------------------------------------------
  bool  ParametersOK();
  Tag   GetId (void);
  void  Draw (CCameraCockpit *cam);
  void  Activate (void);
  void  Deactivate (void);
  void  SetViewPort();
  void  ScreenResize();
  //-----------------------------------------------------------------------------
	void	SetPanelQuad(TC_VTAB *pan);
  void  SetCacheLEF(TC_VTAB *dst);
  void  SetCacheRIT(TC_VTAB *dst);
  void  SetCacheTOP(TC_VTAB *dst);
  void  SetCacheBOT(TC_VTAB *dst);
	void	BuildGaugeVBO();
	//--- VBO management ----------------------------------------------------------
	U_SHORT   FixRoom(U_SHORT n);
	U_SHORT   DynRoom(U_SHORT n);
  //-----------------------------------------------------------------------------
  CGauge* GaugeHit (int mouseX, int mouseY);
  bool  GaugeMouseClick (int mouseX, int mouseY);
  void  GaugeTrackClick ();
  void  GaugeStopClick  (int mouseX, int mouseY);
  bool  MouseMotion (int x, int y);
  bool  MouseClick (int button, int updown, int x, int y);
  //-----------------------------------------------------------------------------
	int 	AuxilliaryGauge(CGauge *g,SStream *s);
  int   ProcessGauge(CGauge *g,SStream *s);
  void  FocusOnGauge(Tag id);
  void  FocusOnGauge(CGauge *g);
  void  FocusOnClick(CGauge *g, char No);
  void  DrawCaTour();
	//------------------------------------------------------------------------------
  CGauge*				GetGauge(Tag id);
	CPanelLight*	GetLight(Tag id);
  //-----------------------------------------------------------------------------
  inline void ClearFocus()        {gFocus = 0; ca = 0;}
  inline int  GetHeight()         {return y_isiz;}
  //----Scroll methods ----------------------------------------------------------
  void      ScrollUP (void);
  void      ScrollDN (void);
  void      ScrollLF (void);
  void      ScrollRT (void);
  void      PageUp (void);
  void      PageDown (void);
	//-----------------------------------------------------------------------------
	inline		TC_VTAB  *GetDynVBO()					{return pBUF;}
	//-----------------------------------------------------------------------------
	inline	bool			IsMain()							{return (main != 0);}
	inline  bool			NotMain()							{return (main == 0);}
  //-----------------------------------------------------------------------------
	inline	  CPanelLight* GetLight()				{return plite;}
	inline  char*			GetName()							{return filename;}
  inline  void      GetOFS(TC_4DF &d)			{d.x0 = xOffset, d.y0 = yOffset;}
  inline  int       GetXOffset()					{return xOffset;}
  inline  int       GetYOffset()					{return yOffset;}
  inline  int       GetPanelHT()					{return y_3Dsz;}
  inline  Tag       GetID()								{return id;}
  inline  int       TotalGauges()					{return gage.size();}
	inline  CVehicleObject *GetMVEH()				{return mveh;}
	inline  void      MouseScreen(int &x, int &y) {x = hit.sx, y = hit.sy;}
};

//==================END OF FILE ================================================================
#endif // PANELS_H



