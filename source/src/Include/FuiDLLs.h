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

/*! \file FuiDlls.h
 *  \brief 
 *
 *
 */
 
#ifndef FUIDLLS_H
#define FUIDLLS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Include/Fui.h"
#include "../Include/FuiParts.h"

#include <vector>
#include <stdio.h>

//=============================================================================
//  Class CDllLine to display dlls in DLL windows
//=============================================================================
class CDllLine: public CSlot {
  //--------Attributes ------------------------------------
  char dlltext [100];
  char module  [64];
  char vers    [5];
  char obj     [4];
  char pct     [4];
  char tot     [4];
  char type    [8];
  char name    [8];
  //-------------------------------------------------------
public:
  CDllLine(void) : CSlot (1) {
    *dlltext = NULL;
    *module  = NULL;
    *vers    = NULL;
    *obj     = NULL;
    *pct     = NULL;
    *tot     = NULL;
    *type    = NULL;
    *name    = NULL;
  }
  int     Edit (CFuiList *w, U_CHAR ln, U_CHAR l0);
  void    Print(CFuiList *w,U_CHAR ln);
  //---------methods --------------------------------------
  void    SetKText  (const char *txt) {strncpy (dlltext, txt, 100);}
  void    SetModule (const char *txt) {strncpy (module,  txt,  64);}
  void    SetVers   (const char *txt) {strncpy (vers,    txt,   5);}
  void    SetNumObj (const char *txt) {strncpy (obj,     txt,   4);}
  void    SetPct    (const char *txt) {strncpy (pct,     txt,   4);}
  void    SetTot    (const char *txt) {strncpy (tot,     txt,   4);}
  void    SetName   (const char *txt) {strncpy (name,    txt,   8);}
  void    SetType   (const char *txt) {strncpy (type,    txt,   8);}
  char*   GetKText  (void)            {return dlltext;}
};

//=============================================================================
//  Class CFuiDLLsWindow to display DLLs list
//=============================================================================
class CFuiDLLsWindow : public CFuiWindow {
  //-------ATTRIBUTES ------------------------------------------------
  CListBox           dllBOX;                // List of DLLs
  //-------Methods ----------------------------------------------------
public:
  CFuiDLLsWindow (Tag idn, const char *filename);
 ~CFuiDLLsWindow (void);
  //------------------------------------------------------------------
  void Init (void);
  //------------------------------------------------------------------
  void      NotifyResize (short dx, short dy);
  void      NotifyChildEvent (Tag idm, Tag itm, EFuiEvents evn);
};

#endif // FUIDLLS_H
//=================================END OF FILE =====================================================
