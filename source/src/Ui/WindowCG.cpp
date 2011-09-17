/*
 * WindowCG.cpp
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

/*! \file WindowDirectory
 *  \brief Implements FUI callback for Directory window
 *
 *  Implements the FUI callback for the directory window which display and search
 *    for airport/navaid/waypoints...
 */

#include "../Include/FlyLegacy.h"
#include "../Include/Fui.h"
#include "../Include/FuiPlane.h"
#include "../Include/FuiParts.h"
#include "../Include/Globals.h"
#include "../Include/Utility.h"
#include "../Include/AnimatedModel.h"

#include <algorithm>
#include <vector>
#include <stdarg.h>
//==================================================================================
#define WINCG_WID 512
//==================================================================================
// Title for windowCG 
//==================================================================================
char *texWCG[] = {
  0,
  "FUEL TANK LOCATION",
  "LOADOUT STATIONS",
  "CENTER OF GRAVITY",
};
//==================================================================================
// Button view for windowCG 
//==================================================================================
char *viewBTN[] = {
  "Front view",
  "Top view",
};
//==================================================================================
//  THIS WINDOW WILL DISPLAY THE CENTER OF GRAVITY
//==================================================================================
CFuiWinCG::CFuiWinCG(Tag idn, const char* filename)
:CFuiWindow(idn,filename,516,552,0)
{ title = 1;
  close = 1;
  zoom  = 0;
  mini  = 0;
  view  = 0;
  //----Init color -----------------------------------
  bk.R  = 0.235f;
  bk.G  = 0.658f;
  bk.B  = 0.749f;
  bk.A  = 1;
  //----Get model dimension --------------------------
  oDim = GetObjectDim();
  //--- Create the camera ----------------------------
  oCam   = new CCameraObject();
  oCam->SetFBO(WINCG_WID,WINCG_WID);
  oCam->MoveMouseIs(0);
  oCam->TopOffsetFor(oDim * 0.5);
  oCam->SetColor(bk);
  //--- Init the canva -------------------------------
  oWIN  = new CFuiCanva (2,2,WINCG_WID,WINCG_WID,this);
  AddChild('owin',oWIN);
  oWIN->RegisterCamera(oCam);
  oWIN->EraseCanvas();
  //---- Create the view button ----------------------
  vBTN  = new CFuiButton( 10,520,100,20,this);  
  AddChild('view',vBTN,viewBTN[view]);
  //---- Create the Fuel Button ----------------------
  gBTN  = new CFuiButton(198,520,100,20,this);
  AddChild('fuel',gBTN,"FUEL");
  //---- Create the LOAD button ----------------------
  lBTN  = new CFuiButton(304,520,100,20,this);
  AddChild('load',lBTN,"LOAD");
  //---- Create the CofG Button ----------------------
  cBTN  = new CFuiButton(410,520,100,20,this);
  AddChild('cofg',cBTN,"CofG");
  //---- Create the Label window ----------------------
  labW  = new CFuiLabel( 10,  2, 500,20);
  AddChild('cogp',labW,"COG at: xxx,yyy,zzz");
  //---- Create the window ---------------------------
  CFuiWindow::ReadFinished();
  SetWinTitle(DRAWING_COFG);
  veh = globals->pln;
  if (0 == veh)   Close();
  veh->GetFuelCell(fcel);
  veh->GetLoadCell(lcel);

}
//---------------------------------------------------------------------------------
//  Destroy the window
//---------------------------------------------------------------------------------
CFuiWinCG::~CFuiWinCG()
{ if (oCam) delete oCam;
  oWIN->RegisterCamera(0);
  fcel.clear();
}
//---------------------------------------------------------------------------------
//  Get object dimension
//---------------------------------------------------------------------------------
double CFuiWinCG::GetObjectDim()
{ acm  = GetDayModelACM();
  if (0   == acm)   return 0;
  //------Compute pixel ratio -------------
  CVector ext;
  acm->GetExtension(ext);
  return ext.MaxXY();
}
//---------------------------------------------------------------------------------
//  Set the title
//  NOTE: veh should be initialized 
//---------------------------------------------------------------------------------
void CFuiWinCG::SetWinTitle(U_CHAR tp)
{ type  = tp;
  SetTitle(texWCG[type]);
  return;
}
//--------------------------------------------------------------------------------
//  Draw
//  Drawing is executed by the camera
//--------------------------------------------------------------------------------
void CFuiWinCG::Draw()
{ char edt[128];
  CVector cog;
  CFuiWindow::Draw();
  VIEW_PORT vp;
  oWIN->GetViewPort(vp);
  if (acm) oCam->DrawOnWin(vp,this);
  veh->GetVisualCG(cog);
  _snprintf(edt,128,"COG at: X=%.04f, Y=%.04f, Z=%.04f",cog.x,cog.y,cog.z);
  labW->SetText(edt);
  labW->Draw();
  return;
}
//--------------------------------------------------------------------------------
//  Draw model By Camera
//  -Drawing depend on the requested Type defined at globals->bTyp
//--------------------------------------------------------------------------------
void CFuiWinCG::DrawByCamera(CCamera *cam)
{ std::vector<CFuelCell*>::iterator vc;
  //---Draw a view of aircraft --------------------------------
  acm->PreDraw();
  acm->Draw(BODY_NOMOVE);
  acm->EndDraw();
  switch (type)  {
    case DRAWING_COFG:
      //---Draw over the gizmo at CG --------------------------
      veh->GetVisualCG(vCG);
      gizmo.SetPosition(vCG);
      gizmo.PreCros();
      gizmo.DrawCros();
      gizmo.EndCros();
      return;

    case DRAWING_FUEL:
      //---Draw Fuel tank location -----------------------------
      gizmo.PreFuel();
      
      for (vc = fcel.begin(); vc != fcel.end(); vc++)
      { CFuelCell *cel = (*vc);
        SVector    pos = cel->GetPosition();
        gizmo.SetPosition(pos);
        gizmo.DrawTank(view);
      }
      gizmo.EndFuel();
      return;
    case DRAWING_LOAD:
      //---Draw load station -----------------------------------
      gizmo.PreLoad();
      std::vector<CLoadCell*>::iterator vl;
      for (vl = lcel.begin(); vl != lcel.end(); vl++)
      { CLoadCell *cel = (*vl);
        SVector    pos = cel->GetPosition();
        gizmo.SetPosition(pos);
        if (!cel->IsEmpty()) gizmo.DrawLoad();
      }
      gizmo.EndLoad();
      return;
  }
  return;
}

//--------------------------------------------------------------------------------
//  Notification
//--------------------------------------------------------------------------------
void  CFuiWinCG::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ switch (idm)  {
    case 'sysb':
      SystemHandler(evn);
      return;
    //---Swap view -----------------
    case 'view':
      view ^= 1;
      vBTN->SetText(viewBTN[view]);
      oCam->SwapView();
      return;
    //--- Fuel view ---------------
    case 'fuel':
      SetWinTitle(DRAWING_FUEL);
      return;
    case 'load':
      SetWinTitle(DRAWING_LOAD);
      return;
    case 'cofg':
      SetWinTitle(DRAWING_COFG);
      return;
  }
  return;
}

//============================END OF FILE ================================================================================
