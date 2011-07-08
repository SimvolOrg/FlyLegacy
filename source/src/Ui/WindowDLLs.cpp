/*
 * WindowDLLs.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005-2006 Chris Wallace
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

/*! \file WindowDLLs
 *  \brief 
 *
 */

#include "../Include/FlyLegacy.h"
#include "../Include/FuiDLLs.h"

//=======================================================================
//  LCDEV* CFuiDLLsWindow
//=======================================================================
int CDllLine::Edit (CFuiList *listdlls, U_CHAR ln, U_CHAR l0)
{ listdlls->NewLine (ln);
  listdlls->AddText (ln, 00, 64, module);
  listdlls->AddText (ln, 16, 05,   vers);
  listdlls->AddText (ln, 21, 05,   type);
  listdlls->AddText (ln, 24, 05,   name);
  listdlls->AddText (ln, 30, 02,    obj);
  listdlls->AddText (ln, 34, 04,    "0");
  listdlls->AddText (ln, 39, 04,    "0");
  return 1;
}

//================================================================================
//  Print a line
//================================================================================
void CDllLine::Print (CFuiList *w, U_CHAR ln)
{ Edit (w, ln, 0);
  return;
}

//=================================================================================
//  Window for dll listing
//=================================================================================
CFuiDLLsWindow::CFuiDLLsWindow(Tag idn, const char *filename)
:CFuiWindow(idn,filename,0,0,0)
{ 
  //----inhibit resizing -------------------------------
  RazProperty(FUI_XY_RESIZING);
  SetXRange(w,w);                         // Fixed width
  SetChildProperty('list',FUI_VT_RESIZING);
  SetProperty(FUI_VT_RESIZING);
  //-- Init List boxes ---------------------------------
  dllBOX.SetParameters(this,'list',0);
  Init ();
}
//--------------------------------------------------------------------------
//  Free resources
//--------------------------------------------------------------------------
 CFuiDLLsWindow::~CFuiDLLsWindow()
 { 
 }

//--------------------------------------------------------------------------
//  Resize notification
//--------------------------------------------------------------------------
void CFuiDLLsWindow::NotifyResize(short dx,short dy)
{ dllBOX.Resize();
  return;
}

//--------------------------------------------------------------------------
//  Display Key Set corresponding to ident
//--------------------------------------------------------------------------
void CFuiDLLsWindow::Init (void)
{ dllBOX.EmptyIt();
  if (0 == globals->plugins_num) {
    CDllLine *dln = new CDllLine;
    dllBOX.AddSlot(dln);
    dln->SetKText (" - DLLs system disabled = no dll charged");
  } else {
    globals->plugins.BuildExportList ();
    std::vector <std::string>::iterator it_str;
    for (it_str  = globals->plugins.dll_export_listing.begin ();
         it_str != globals->plugins.dll_export_listing.end ();
         it_str++) {
         CDllLine *dln = new CDllLine;
         dllBOX.AddSlot(dln);
         //
         std::string vers (*it_str,  1,  5);
         std::string type (*it_str,  8,  4);
         std::string name (*it_str, 15,  4);
         std::string obj_ (*it_str, 22,  1);
         std::string modu (*it_str, 26, 64); // a space included before the string
         dln->SetVers     (vers.c_str ());
         dln->SetType     (type.c_str ());
         dln->SetName     (name.c_str ());
         dln->SetNumObj   (obj_.c_str ());
         dln->SetModule   (modu.c_str ());
         //TRACE ("dll %s", name.c_str ());
    }
  }
  dllBOX.SortAndDisplay ();
  return;
}

//-------------------------------------------------------------------------
//  EVENT notifications from child windows
//-------------------------------------------------------------------------
void  CFuiDLLsWindow::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ if (idm == 'sysb')  {SystemHandler(evn); return;}
  switch (idm)  {
      case  'list':
        dllBOX.VScrollHandler((U_INT)itm,evn);
        return;
  }
  return ;
}