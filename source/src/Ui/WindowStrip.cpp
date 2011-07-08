/*
 * WindowStrip.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005 Chris Wallace
 * Copyright 2007 Jean Sabatier
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
#include "../Include/Utility.h"
#include "../Include/Globals.h"
#include "../Include/Fui.h"
#include "../Include/FuiParts.h"
#include <vector>
#include <stdio.h>
//=====================================================================================
//  Window Strip for a line of text with left and right scroll bar
//=====================================================================================
CFuiStrip::CFuiStrip(Tag idn, const char *filename)
:CFuiWindow(idn,filename,0,0,0)
{ iBOX   = (CFuiTextField*)GetComponent('text');
  if (0 == iBOX)  gtfo("Incorrect FUI directory file");
  lBOX = new CFuiScrollBTN(2,2,14,14,this,"LEFT");
  AddChild('left',lBOX);
  lBOX->SetRepeat(0.25);
  rBOX = new CFuiScrollBTN(484,2,14,14,this,"RIGHT");
  AddChild('righ',rBOX);
  rBOX->SetRepeat(0.25);
}
//---------------------------------------------------------------------
//  Set Airport ident and change title
//---------------------------------------------------------------------
void CFuiStrip::SetIdent(char *name)
{ char edt[32];
  strncpy(Iden,name,4);
  Iden[4] = 0;
  sprintf(edt,"METAR from %s",Iden);
  SetTitle(edt);
  GetMetar(Iden);
  return;
}
//---------------------------------------------------------------------
//  No METAR Available
//---------------------------------------------------------------------
void CFuiStrip::NoMetar()
{ iBOX->SetText(" No data available");
  return;
}
//---------------------------------------------------------------------
//  Get requested metar if any
//---------------------------------------------------------------------
void CFuiStrip::GetMetar(char *idn)
{ char fnam[PATH_MAX];
  sprintf(fnam,"METAR/%s.TXT",idn);
  FILE  *f  = fopen(fnam,"r");
  if (0 == f)  return NoMetar();
  //---Display METAR text ------------------------------
  char *txt = fgets(text,254,f);
  iBOX->EditText(" %s ",txt);
  return;
}
//---------------------------------------------------------------------
//  Intercept scroll event
//---------------------------------------------------------------------
void CFuiStrip::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ switch (idm)  {
  case 'sysb':
    SystemHandler(evn);
    return;
  case 'left':
    if (evn == EVENT_BUTTONPRESSED) iBOX->ScrollLeft();
    return;
  case 'righ':
    if (evn == EVENT_BUTTONPRESSED) iBOX->ScrollRight();
    return;
 }
  return;
}
//=======================END OF FILE ==================================================