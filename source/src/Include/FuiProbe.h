/*
 * FuiProbe.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
 * CopyRight 2007 Jean  Sabatier
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

/*! \file KeyMap.h
 *  \brief Defines CKeyMap and related classes for mapping keys to events
 */

#ifndef FUIPROBE_H
#define FUIPROBE_H

#include "../Include/FlyLegacy.h"
#include "../Include/Fui.h"
//=============================================================================
class TCacheMgr;
//=============================================================================
//  COMPONENTS OF PROBE COLUMN
//=============================================================================
struct PROBE {
  CSubsystem      *sub;
  CFuiCanva       *wCn;
  CFuiButton      *btn;
  CFuiList        *wsy;
  CFuiButton      *dpn;
  CFuiPopupMenu   *pop;
  char           **lst;
  FL_MENU          men;
  CListBox         box;
};
//=============================================================================
//  Class CFuiProb to display subsystem properties
//=============================================================================
class CFuiProbe: public CFuiWindow 
{ //--------------Attributes -----------------------------------------
  CVehicleObject *veh;                          // Vehicle
  CFuiList      *wSub;                          // List of subsystems
  CListBox      sBox;                           // Pilot list
  //------------------------------------------------------------------
  PROBE         pob1;
  PROBE         pob2;
  //------------------------------------------------------------------
  CFuiCanva     *info;                          // Canva info
  CFuiCanva     *meto;                          // Canva metar
  //------------------------------------------------------------------
  CPIDbox       *spid;                          // selected pid
  CFuiList      *wPID;                          // PID list
  CListBox       bPID;                          // PID box
  CFuiCanva     *cPID;                          // PID data
  //------------------------------------------------------------------
  CListBox      *work;
  //-----Subsystems --------------------------------------------------
  //--------------Methods --------------------------------------------
public:
  CFuiProbe(Tag idn, const char *filename);
 ~CFuiProbe();
  void          Close();
  void          Init();
  //------------------------------------------------------------------
  void          GetAmpSubsystems();
  void          GetGasSubsystems();
  void          GetPidSubsystems();
  void          GetEngSubsystems();
  void          GetWhlSubsystems();
  //------------------------------------------------------------------
  void          Draw();
  void          ViewOption(PROBE &p,int k);
  void          ViewSubsystem(CSubsystem *sub,PROBE &pob);
  void          AddSubsystem(CSubsystem *obj);
  void          AddDependent(CDependent *dp);
  void          AddPid(CPIDbox *p);
  CSubsystem   *GetSubsystem(CFuiButton *btn);
  void          ReplaceDependent(PROBE &p1,PROBE &p2);
  void          ReplacePID();
  void          ResetOption(PROBE &pob);
  //-----------Notifications -----------------------------------------
  void  NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
  void  NotifyFromPopup (Tag idm,Tag itm,EFuiEvents evn);
};

//=============================================================================
//  Class CFuiStat to display statistical info
//=============================================================================
class CFuiStat: public CFuiWindow
{ protected:
  //--------------Attributes -----------------------------------------
  CFuiCanva       *info;
  TCacheMGR       *tcm;
  CTextureWard    *txw;
  C3DMgr          *m3d;
  CDbCacheMgr     *dbc;
  CCockpitManager *pit;
  //--------------Methods --------------------------------------------
public:
  CFuiStat(Tag idn, const char *filename);
 ~CFuiStat();
 //-------------------------------------------------------------------
 void     Draw();
 //--- Dont close windows --------------------------------------------
 bool CheckProfile(char a) {return true;}
};
//==============END OF FILE =============================================================
#endif // FUIPROBE_H

