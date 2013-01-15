/*
 * Ui.h
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

/*! \file Ui.h
 *  \brief PUI-based user interface definitions
 */


#ifndef UI_H
#define UI_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "FlyLegacy.h"
#include "KeyMap.h"
#include <plib/pu.h>

#include <vector> // sdk:  // 

//----------------------------------------------------------------------------
// Main initialization function.  This MUST be called before any user message
//   is displayed.
//----------------------------------------------------------------------------
void		CheckTuningMenu();

//----------------------------------------------------------------------------
// Keyboard functions
//----------------------------------------------------------------------------
void        init_keyboard (void);
void        BindAllKeys (CKeyMap *keymap);
EKeyboardModifiers  glutModifiersToFlyLegacyModifiers (int glutmod);
U_INT       glutKeyToFlyLegacyKey (U_INT gkey, U_INT mdf);
U_INT       glutSpecialToFlyLegacyKey (int glutkey);
#ifdef _WIN32
EKeyboardKeys   VirtualKeyToLegacyKey (BYTE vk);
int         VirtualKeyToGlutKey (BYTE vk, bool shift, bool ctrl, bool alt);
#endif // _WIN32

//
// Menu functions
//
int toggle_menu ();
int GetMenuState();
int SetMenuState(char s);
//
// sdk: Menu additional class and structure // 
//
namespace sdkmenu {

  const int MAX_MAINMENU_ADDIN  = 5;       // max 5 addin in the main menu bar
  const int MAX_SDK_SUBMENU_NUM = 10;     // max 10 items per menu addin

  typedef struct SMenuData {

    unsigned long menuID;                 // ex 'rotw'
    std::string str;                      // ex "Plugins"
    std::vector <unsigned long> itemID;   // ex MY_ITEM_1 ...
    std::vector <std::string> items;      // ex "Yaget 1" ...

  } SMenuData;

  class CSDKMenu {

  public:
    std::vector<SMenuData> md_;

    CSDKMenu             (void);
    virtual ~CSDKMenu    (void) {}

    void AddMenu         (unsigned long menuID, const char *label);
    void AddMenuItem     (unsigned long menuID, unsigned long itemID, const char *label);

  protected:

  private:
    CSDKMenu             (const CSDKMenu &);  // purposely unimplemented
    CSDKMenu & operator= (const CSDKMenu &);  // purposely unimplemented
  };
}

void	OpenMasterMenu(); 
void	OpenUserMenu();
void	CloseUserMenu();
// sdk: end

//
// Debug->Messaging Dialog
//
void msg_debug_dlg_create (void);

#endif  // UI_H
