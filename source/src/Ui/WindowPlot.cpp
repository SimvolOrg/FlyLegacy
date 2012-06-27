/*
 * WindowPlot.cpp
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
#include "../Include/FuiUser.h"
//=====================================================================================
//  MENU for ploting data.  room for 16 items
//=====================================================================================
char *plotMENU[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//=====================================================================================
//  define drawing ratio
//=====================================================================================
//=====================================================================================
//  PLOTTER
//=====================================================================================
CPlotter::CPlotter()
{
}
//------------------------------------------------------------------------
//  Init the plotter
//------------------------------------------------------------------------
void CPlotter::Init(CFuiCanva *cn, CFont *fnt, CFuiPopupMenu *pm, CVehicleObject *vh)
{ font    = fnt;
  pop     = pm;
  cnv     = cn;
  mveh    = vh;
  srf     = cnv->GetSurface();
  x0      = fnt->CharWidth('H');
  y0      = fnt->CharHeight('H');
  y1      = cnv->GetHeight();
  yL      = RealY(y1);
  yT      = yL + y0;                  // Real top line
  yB      = RealY(y0);                // Real bottom line
  xF      = cnv->GetWidth();
  scale   = 1;
  //---Compute vertical height ------------------
  vht     = (y1 - y0);
  //---Compute horizontal width------------------
  hwd     = (xF - x0);
  //----Init color parameters -------------------
  green = MakeRGBA (0, 255, 0, 255);
  red   = MakeRGBA (255, 0, 0, 255);
  //--- Create data menu ------------------------
  mveh->AddToPlotMenu(plotMENU,0);
  pop->CreatePage(&pMEN,plotMENU);
  pop->SetButtonText((char*)pMEN.aText[0]);
  //--- No dependent ----------------------------
  pPRM.dpnd = 0;
  return;
}
//-----------------------------------------------------------------------
//  Start plotter
//-----------------------------------------------------------------------
int CPlotter::Start(Tag itm)
{
  pop->SetButtonText((char*)pMEN.aText[itm]);
  mveh->PlotParameters(&pPRM,itm);
  return Header();
}
//-----------------------------------------------------------------------
//  Init header
//-----------------------------------------------------------------------
int CPlotter::Header()
{ char edt[PATH_MAX];
  float unit = (pPRM.yUnit * scale);
  EraseSurfaceRGBA(srf,0);
  ppyu    = vht / unit;
  _snprintf(edt,(PATH_MAX-1),pPRM.mask,unit);
  xD      = font->DrawNText(srf,x0,yL,green,edt);
  yPix    = RealY(y0);
  //----Draw horizontal line ----------------------------------
  DrawFastLine(srf,x0,yT,xF,yT,green);
  DrawFastLine(srf,x0,yB,xF,yB,green);
  return 1;
}
//-----------------------------------------------------------------------
//  Change scale
//-----------------------------------------------------------------------
int CPlotter::NewScale(float sc)
{ scale = sc;
  return Header();
}
//---------------------------------------------------------------------------
//  Draw a pixel
//---------------------------------------------------------------------------
void CPlotter::DrawPixel(short xa, short xb)
{ if (0 == pPRM.dpnd)     return;
  //--- Get value to plot -------------------------
  pPRM.dpnd->Plot(pPRM);
  int vy = int (pPRM.val * ppyu) + y0;
  int py = RealY(vy);
  //--- Draw upper and lower line ------------------
  DrawFastLine(srf,xa,yT,xb,yT,green);
  DrawFastLine(srf,xa,yB,xb,yB,green);
  //--- Draw a line from previous pixel to new ----
  DrawFastLine(srf,xa,yPix,xb,py,red);
  yPix   = py;
  return;
}
//=====================================================================================
//  Window for engine data plotting
//=====================================================================================
CFuiPlot::CFuiPlot(Tag idn, const char *filename)
:CFuiWindow(idn,filename,900,400,0)
{ SetTransparentMode();
  SetText("PLOTTER");
  //--- Init group box -----------------------------------
  gBOX = new CFuiGroupBox(0,0,120,400,this);
  gBOX->RazProperty(FUI_TRANSPARENT);
  AddChild('grpb',gBOX,"");
  //--- Init Canva 1 -------------------------------------
  cnv1 = new CFuiCanva(120,0,780,200,this);
  cnv1->SetProperty(FUI_TRANSPARENT);
  AddChild('cnv1',cnv1,"");
  //--- Init canva 2 -------------------------------------
  cnv2 = new CFuiCanva(120,200,780,200,this);
  cnv2->SetProperty(FUI_TRANSPARENT);
  AddChild('cnv1',cnv2,"");
  //--- Init Popup menu 1---------------------------------
  pop1  = new CFuiPopupMenu(10,  8,100,30,this);
  gBOX->AddChild('pop1',pop1,"DATA to PLOT");
  //--- Init Popup menu 2---------------------------------
  pop2  = new CFuiPopupMenu(10,208,100,30,this);
  gBOX->AddChild('pop2',pop2,"DATA to PLOT");
  //--- Init scaler n°1 ----------------------------------
  psc1.scale = 1;                           // Initial scale
  upr1  = new CFuiButton(12,80,20,20,this);
  upr1->SetRepeat(0.25);
  gBOX->AddChild('upr1',upr1,"+");
  psc1.tex  = new CFuiTextField(34,80,40,20,this);
  psc1.tex->SetProperty(FUI_TRANSPARENT);
  gBOX->AddChild('scl1',psc1.tex,"1.00");
  lwr1  = new CFuiButton(76,80,20,20,this);
  lwr1->SetRepeat(0.25);
  gBOX->AddChild('lwr1',lwr1,"-");
  //--- Init scaler n°2 ----------------------------------
  psc2.scale = 1;                           // Initial scale
  upr2  = new CFuiButton(12,280,20,20,this);
  upr2->SetRepeat(0.25);
  gBOX->AddChild('upr2',upr2,"+");
  psc2.tex  = new CFuiTextField(34,280,40,20,this);
  psc2.tex->SetProperty(FUI_TRANSPARENT);
  gBOX->AddChild('scl2',psc2.tex,"1.00");
  lwr2  = new CFuiButton(76,280,20,20,this);
  lwr2->SetRepeat(0.25);
  gBOX->AddChild('lwr2',lwr2,"-");
  //---  Init mother vehicle -----------------------------
  mveh  = globals->pln;
  if (0 == mveh)  Close();
  //------------------------------------------------------
  CFuiWindow::ReadFinished();
  strt  = 0;
  psc1.plot = &p1;
  p1.Init(cnv1,fnts,pop1,mveh);
  psc2.plot = &p2;
  p2.Init(cnv2,fnts,pop2,mveh);
  //------------------------------------------------------
  time  = 0;
  ppxu  = p1.GetWidth() / 20;
  xF    = p1.GetXF();
}
//---------------------------------------------------------------------------
//  Recycle the X generator
//---------------------------------------------------------------------------
short  CFuiPlot::Shift(short sx)
{ time -= sx * (float (1) / ppxu);
  xPix -= sx;
  if (p1.On())  p1.Shift(sx);
  if (p2.On())  p2.Shift(sx);
  return sx;
}
//---------------------------------------------------------------------------
//  Compute the X coordinate for both curves
//---------------------------------------------------------------------------
short CFuiPlot::ComputeX()
{ if (0 == strt)  return xD;
  float dT = globals->tim->GetDeltaSimTime();
  time  += dT;
  short dx = int (ppxu * time);
  short px = p1.GetXD() + dx;
  short sx = px - xF;
  if (sx > 0) px -= Shift(sx);
  return px;
}
//---------------------------------------------------------------------------
//  Draw everything
//---------------------------------------------------------------------------
void CFuiPlot::Draw()
{ //--------- Render map surface-----------------------------
  short px = ComputeX();
  p1.DrawPixel(xPix,px);
  p2.DrawPixel(xPix,px);
  xPix  = px;
  CFuiWindow::Draw();
  return;}
//--------------------------------------------------------------------------
//  Start on plotter
//--------------------------------------------------------------------------
void CFuiPlot::Start(CPlotter &p, Tag itm)
{ strt++;
  p.Start(itm);
  xD    = p.GetXD();
  xPix  = xD;
  return;
}
//--------------------------------------------------------------------------
//  Modify the scale
//--------------------------------------------------------------------------
void CFuiPlot::ModifyScale(PLOT_SCALE &ps,float mod)
{ char edt[16];
  float ns  = ps.scale * mod;
  if (ns < 0.1)     return;
  if (ns > 100)     return;
  ps.scale = ns;
  _snprintf(edt,16,"%.5f",ns);
  ps.tex->SetText(edt);
  ps.plot->NewScale(ns);
  return;
}
//--------------------------------------------------------------------------
//  Notification from popup
//--------------------------------------------------------------------------
void CFuiPlot::NotifyFromPopup(Tag idm,Tag itm,EFuiEvents evn)
{ if (evn != EVENT_POP_CLICK) return;
  if (idm == 'pop1') Start(p1,itm);      //InitPlot(itm);
  if (idm == 'pop2') Start(p2,itm);
  return;
}
//-------------------------------------------------------------------------
//  EVENT notifications from child windows
//-------------------------------------------------------------------------
void  CFuiPlot::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ if (idm == 'sysb')  return  SystemHandler(evn);
  if (idm == 'upr1')  return  ModifyScale(psc1,1.25);
  if (idm == 'lwr1')  return  ModifyScale(psc1,0.8f);
  if (idm == 'upr2')  return  ModifyScale(psc2,1.25);
  if (idm == 'lwr2')  return  ModifyScale(psc2,0.8f);
  return;
}
//=======================END OF FILE ==================================================