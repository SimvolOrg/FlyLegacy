/*
 * FuiKey.h
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
 
#ifndef FUIKEY_H
#define FUIKEY_H

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
//=============================================================================
//  Class CFuiKeyMap to display Keyboard mapping
//=============================================================================
class CFuiKeyMap : public CFuiWindow {
  //-------ATTRIBUTES ------------------------------------------------
  CJoysticksManager *jsm;
  U_CHAR             modify;                // Keyset modified
  U_CHAR             wrtJOY;                // Joystick modified
  //------KEY LIST ------------------------------------------------
  U_SHORT            selOpt;                // Selected option
  CFuiLabel          *label;                // Label windows
	CFuiLabel          *wkeys;							  // Label key
  CFuiPopupMenu     *selPop;                // Selection Popup
  FL_MENU             mSEL;                 // Menu selector
  char               **item;                // Array of pointer to Menu items
  Tag                 *iden;                // Array of  Menu ident
  CListBox           keyBOX;                // List of Keys
  //------MODE LIST -----------------------------------------------
  U_SHORT            modOpt;                // current mode
  CFuiPopupMenu     *modPop;                // Mode popup
  FL_MENU            modMEN;                // Mode menu    
  CKeyFile            *dfk;                 // Default keys
  //---------------------------------------------------------------
  static char *modMENU[];
  //------Methods ----------------------------------------------------
public:
  CFuiKeyMap(Tag idn, const char *filename);
 ~CFuiKeyMap();
  //------------------------------------------------------------------
  int       KeyPress(int cde);
  int       NewKeyCode(int cde);
  void      SelectMode(U_INT No);
  void      SelectSet (U_INT No);
  int       InitPopup();
  //------------------------------------------------------------------
  void      DisplayKeys(Tag idn);
  bool      CheckKey(int cde,CKeyDefinition *kdf);
  void      Unassign(CKeyDefinition *kdf,int cde);
  void      Unassign(SJoyDEF *jsd,int bt);
  void      DefaultKeys();
  void      ClearKey();
  bool      Message(char *msg);
  //------------------------------------------------------------------
  void      StopDetection();
  void      SaveConfiguration();
  //------------------------------------------------------------------
  void      ButtonHit(SJoyDEF *jsd,int bt);
  void      ClearButton();
  //------------------------------------------------------------------
  void      NotifyResize(short dx,short dy);
  void      NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn);
  void      NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
};
//=================================================================================
//  Class CKsetDef to parse default key set
//=================================================================================
class CKsetDef: public CStreamObject {
  //----Attributes ----------------------------------------
  Tag           idn;                  // SET identifier
  std::map<Tag,int>   DefKeys;        // Map of default keyset ids
  //-------------------------------------------------------
public:
  CKsetDef(Tag id);
 ~CKsetDef();
  //-------------------------------------------------------
  int   Read (SStream *stream, Tag tag);
  int   GetCode(Tag idk)    {return DefKeys[idk];}
};
//=================================================================================
//  Class CKeyItem to parse default key file
//=================================================================================
class CKeyItem: public CStreamObject {
  friend class CKsetDef;
  Tag       idn;
  int       code;
  //-------------------------------------------------------
public:
  CKeyItem();
  //-------------------------------------------------------
  int   Read (SStream *stream, Tag tag);

};

//=================================================================================
//  Class CKeyFile to parse default key file
//=================================================================================
class CKeyFile: public CStreamObject {
  std::map<Tag,CKsetDef *> kSet;      // Set Of Keys
public:
  //-------------------------------------------------------
  CKeyFile(char *fname);
 ~CKeyFile();
  int     GetDefault(Tag ks,Tag idk);
  //-------------------------------------------------------
  int   Read (SStream *stream, Tag tag);
};
#endif // FUIKEY_H
//=================================END OF FILE =====================================================
