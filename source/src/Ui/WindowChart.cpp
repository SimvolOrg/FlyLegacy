/*
 * WindowChart.cpp
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
#include "../Include/Fui.h"
#include "../include/FuiUser.h"
#include "../include/TerrainTexture.h"
#include <vector>
//==================================================================================
//  Window to display sectional charts
//==================================================================================
CFuiChart::CFuiChart(Tag idn, const char *filename)
:CFuiWindow(idn,filename,800,800,0)
{ SetTransparentMode();
  SetProperty(FUI_XY_RESIZING);
  //---Init info image -------------------------------
  MapInfo.rgba = 0;
  //---Create the window -----------------------------
  CFuiWindow::ReadFinished();
  SetTitle("SECTIONAL CHART");
 // strncpy(globals->MapName,"Montreal North.jpg",128);
  OpenChart(globals->MapName);
}
//----------------------------------------------------------------------------------
//  Load the chart
//----------------------------------------------------------------------------------
void  CFuiChart::OpenChart(char *fn)
{ //---Select the document and display ---------------
  char       path[PATH_MAX];
  sprintf(path,"CHARTS/%s",fn);
  if (!globals->txw->LoadImageJPG(path,MapInfo)) return;
  MapInfo.x0     = 0;
  MapInfo.y0     = 0;
  return;
}
//----------------------------------------------------------------------------------
//  Draw the window
//----------------------------------------------------------------------------------
void CFuiChart::Draw()
{   if (MapInfo.rgba) 
    { EraseSurfaceRGBA (surface,0);
      DrawImage(surface,MapInfo);
      BlitTransparentSurface (surface, 0, 0, 0);
    }
    CFuiWindow::Draw();
    return;
}
//----------------------------------------------------------------------------------
//  Inside mouse click: See CFuiWindow helper
//----------------------------------------------------------------------------------
bool CFuiChart::InsideClick (int mx, int my, EMouseButton button) 
{ if (!MouseHit(mx,my))               return false;
  if (MOUSE_BUTTON_MIDDLE == button)  return ResetImage(MapInfo);
  return ClickImage(mx,my,button,MapInfo);
}
//----------------------------------------------------------------------------------
//  Inside mouse move:  See CFuiWindow helper
//----------------------------------------------------------------------------------
bool CFuiChart::InsideMove(int mx,int my) 
{ 
  return CFuiWindow::MoveImage(mx,my,MapInfo);}
//----------------------------------------------------------------------------------
//  End move
//----------------------------------------------------------------------------------
bool CFuiChart::StopClickInside(int x, int y, EMouseButton button)
{ MapInfo.state = 0;
  return true;
}
//--------------------------------------------------------------------------------
//  Delete the window
//----------------------------------------------------------------------------------
CFuiChart::~CFuiChart()
{ if (MapInfo.rgba)  delete MapInfo.rgba;
}
//============================END OF FILE ================================================================================
