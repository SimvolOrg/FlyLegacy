/*
 * FuiPlane.h
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
 
#ifndef FUIPLANE_H
#define FUIPLANE_H

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
// Define type of drawing
//===================================================================================
#define DRAWING_FUEL  (1)
#define DRAWING_LOAD  (2)
#define DRAWING_COFG  (3)
//=============================================================================
//  Class CFuiFuel to display and modify fuel loadout
//  NOTE: This window will display a limited number of fuel cell
//=============================================================================
//--------------------------------------------------------------------
class CFuelSystem;
class CFuelCell;
//--------------------------------------------------------------------
class CFuiFuel: public CFuiWindow {
  //------------Attributes -------------------------------------------
  CFuiSlider      *rul;
  CFuiLabel       *All;
  CFuiLabel       *Sel;
  CFuiPopupMenu   *pop;
  CFuiList        *Tnk;
  CListBox        gBOX;
  //----CG box -------------------------------------------------------
  CFuiBox         *cBOX;
  //----Fixed cell to compute total ----------------------------------
  CFuelCell       *sCel;                  // Selected cell
  CFuelCell        Tot;
  //----Fuel system --------------------------------------------------
  CFuelSystem     *gas;
  //----ARRAY Of Fuel Cells ------------------------------------------
  std::vector<CFuelCell*>   fcel;
  //----------Fuel parameters ----------------------------------------
  CGasLine          ttl;                  // Title
  CGasLine          all;                  // All tanks
  //------------Grad Menu --------------------------------------------
  FL_MENU         fPop;                   // Grad selector
  U_INT           grNo;                   // Grad NO
  //------------Methods ----------------------------------------------
public:
   CFuiFuel(Tag idn, const char* filename);
  ~CFuiFuel();
  void            Refresh();
  //------------------------------------------------------------------
  void            InitGradMenu();
  void            SetPopTitle(U_SHORT No);
  void            GetTotalGas();
  void            BuildTankList();
  //------------------------------------------------------------------
  void            SelectTank();
  void            ChangeAllQTY(float rate);
  //------------------------------------------------------------------
  float           EditFuelLine(CFuelCell *cel,CFuiLabel *lab);
  void            EditGlobalFuel();
  void            ChangeTankQTY(U_INT inx);
  //------------Notifications ----------------------------------------
  void            NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
  void            NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn);

};
//====================================================================
//  CPlaneIdent to decode aircraft identity
//====================================================================
class CPlaneIdent: public CStreamObject {
  //------------Attributes --------------------------------
  char    item;                               // item number
  U_SHORT vclas;                              //< Classification
  char    make[64];                           //< Make/model of aircraft
  char    icon[64];                           // Icon file

  //-------------------------------------------------------
public: 
  CPlaneIdent();
  int   Read (SStream *stream, Tag tag);    // Read method
  bool  ScanInfoFile(const char *filname);
  //-------------------------------------------------------
  inline  char      *GetMake()                {return make;}
  inline  char      *GetIcon()                {return icon;}
  inline  bool       HasNotClass(U_SHORT cl)  {return ((cl & vclas)== 0);}  
  inline  bool       NotComplete()            {return (3 != item);}
  //-------------------------------------------------------
  inline  void      SetClass(int cl)      {vclas = (U_SHORT)cl;}
  inline  void      SetMake(char *mk)     {strncpy(make,mk,64);}
  inline  void      SetIcon(char *ic)     {strncpy(icon,ic,64);}
  inline  void      SetComplete()         {item = 3;}

};
//=============================================================================
//  CFuiSetAir: To chose from aircraft list
//=============================================================================
class CFuiSetAir: public CFuiWindow {
  //----------Attribute ------------------------------------------
  CPlaneIdent   airInfo;
  CListBox      airBOX;
  CFuiButton   *selBTN;                     // Selector button 
  char         *cPlane;                     // Current plane make
  CAirLine      fSlot;                      // Fixed slot
  //--------Selection popup --------------------------------------
  U_SHORT            selOpt;                // Selected option
  CFuiPopupMenu     *selPOP;                // Selection Popup
  FL_MENU             mSEL;                 // Menu selector
  U_SHORT             mask[6];              // Selection mask
  //--------Methods ----------------------------------------------
public:
   CFuiSetAir(Tag idn, const char *filename);
  ~CFuiSetAir();
  void  EditSelPopup();
  void  ScanAllNFO();
  void  MakeSlot(char * cpln,bool alloc,char *nfo);
  void  ChangeAircraft();
  //------Notifications ------------------------------------------
  void  SetPopTitle(U_SHORT No);
  void  NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
  void  NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn);
};
//=============================================================================
//  CFuiWinCGr: To display the aircraft CG
//=============================================================================
class CFuiWinCG: public CFuiWindow {
  //-------Attributes ------------------------------------------
  char             type;            // Window type
  char             view;            // Button funct
  CFuiCanva       *oWIN;            // Object Canva
  CFuiButton      *vBTN;            // View button
  CFuiButton      *gBTN;            // Gaz button
  CFuiButton      *lBTN;            // Load button
  CFuiButton      *cBTN;            // CofG
  CFuiLabel       *labW;            // Label windows
  //-------Camera object ---------------------------------------
  CCameraObject   *oCam;
  //-------Pixel ratio -----------------------------------------
  double           pRAT;
  double           oDim;
  //------Fuel cells -------------------------------------------
  std::vector<CFuelCell*> fcel;
  std::vector<CLoadCell*> lcel;
  //------Background color -------------------------------------
  RGBA_COLOR        bk;             // Background color
  CVehicleObject  *veh;             // Vehicle
  CModelACM       *acm;             // Model to draw
  //------GIZMO ------------------------------------------------
  CVector          vCG;             // Visual CG (local coordinates)
  CGizmo           gizmo;           // CG ball
  //------METHODS ----------------------------------------------
public:
  CFuiWinCG(Tag idn,const char *filename);
 ~CFuiWinCG();
  //-------------------------------------------------------------
 // CModelACM  *GetModel();
  void        SetWinTitle(U_CHAR tp);
  //------------------------------------------------------------
 double     GetObjectDim();
 void       Draw();
 void       DrawByCamera(CCamera *cam);
 //-------NOTIFICATION -----------------------------------------
 void       NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
};
//==================================================================================
//  CFuiLoad:  Manage the load stations
//==================================================================================
class CFuiLoad : public CFuiWindow {
  //--- ATTTRIBUTES -------------------------------------------
  CFuiLabel   *wLBL;              // Station label
  CFuiList    *wSTA;              // Station list
  CFuiBox     *wBOX;              // Frame
  CFuiLabel   *wIDN;              // Station identity
  CFuiLabel   *wWGH;              // Weight display
  CFuiLabel   *wLIM;              // Limit
  CFuiSlider  *wRUL;              // Slider
  CFuiLabel   *wDRY;              // Dry total
  CFuiLabel   *wGAS;              // Gas total
  CFuiLabel   *wOTH;              // Other total
  CFuiLabel   *wTOT;              // Total
  //--- VehicleObject -----------------------------------------
  CVehicleObject *veh;
  CListBox     sBOX;              // Station box
  std::vector<CLoadCell*>vlod;
  CLoadCell    *scel;             // Selected cell
  //--- METHODS- ----------------------------------------------
public:
  CFuiLoad(Tag idn,const char *filename);
 ~CFuiLoad();
  //-----------------------------------------------------------
 void   ChangeMass();
  void  GetStations();
  void  EditSelection();
  void  EditSumary();
  void  Edit(CLoadCell *cel);
  //-----------------------------------------------------------
  void  Draw();
  //----Notification ------------------------------------------
  void NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);

};
//==================================================================================
// CCageDebug
//==================================================================================
class CCageDebug : public CFuiWindow
{
public:
  CCageDebug (Tag windowId, const char* winFilename, char *wpart = NULL);
  virtual ~CCageDebug (void);
  //----------------------------------------------------------------------------
  // CFuiComponent methods
  virtual void        Draw (void);

  //------------ utility functions ----------------------------------------------
  int  Init       (CFuiComponent *cmp = NULL);
  void Close      (void);

protected:

  int             posWx, posWy; // position of the window on screen
};

//==================================================================================
// CSimDebugWings
//==================================================================================
class CSimDebugWings : public CFuiWindow
{
public:
  CSimDebugWings (Tag windowId, const char* winFilename,char *wpart = NULL);
  virtual ~CSimDebugWings (void);
  //----------------------------------------------------------------------------
  // CFuiComponent methods
  virtual void        Draw (void);

  //------------ utility functions ----------------------------------------------
  int  Init       (CFuiComponent *cmp = NULL);
  void Close      (void);

protected:
  std::string     wing_part;
  CFuiLabel       *cl_wing;
  CFuiLabel       *cd_wing;
  CFuiLabel       *cm_wing;
  CFuiLabel       *LD_wing;
  CFuiLabel       *aoa_wing;
  CFuiLabel       *damage_wing;
  CFuiLabel       *dCL_wing;
  CFuiLabel       *dCD_wing;
  CFuiLabel       *dCM_wing;
  CFuiLabel       *dPitch_wing;
  CFuiLabel       *label_wing;
  int             posWx, posWy; // position of the window on screen
};
//==================================================================================
// CSimDebugManager
//==================================================================================
class CSimDebugManager : public CFuiWindow
{
public:
  CSimDebugManager (Tag windowId, const char* winFilename);
  virtual ~CSimDebugManager (void);

  // CSimDebugManager methods
  void        GetWingList      (const char* root = globals->FlyRoot, const char* folder = "Maps");
  void        OpenListDialogWindow (void);
  void        OpenCageDialogWindow (void);
  void        SetWingList      (const bool &newpage = false, const int &start = 0, const int &end = 0);
  //-------------Event notification --------------------------------------------
  bool InsideClick (int mx, int my, EMouseButton button);
  void    NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
  void    NotifyMenuEvent (Tag idm, Tag itm);
  void    NotifyFromPopup (Tag id,Tag itm,EFuiEvents evn);

  //----------------------------------------------------------------------------
  // CFuiComponent methods
  virtual void        Draw (void);

protected:
  FL_MENU        wMenu;                   // wings popup menu

  std::vector <std::string> md_;
  //------------   -----------------------------------------------------
  CFuiPopupMenu   *dfui_wing; // popup 'wing' component
  CFuiLabel       *dlift_wing;
  CFuiTextField   *dlift_t_wing;
  //------------   -----------------------------------------------------
  CFuiButton      *dfui_cage; // button 'cage' component
  //------------   ------------------------------------------------
  CSimDebugWings *slct_wing_list;
  CCageDebug     *debug_cage;
  //------------ utility functions ----------------------------------------------
  int  Init       (void);
  void Close      (void);
};

#endif // FUIPLANE_H
//=================================END OF FILE =====================================================
