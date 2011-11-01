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
{ title = 1;
  close = 1;
  zoom  = 0;
  mini  = 1;
  SetTransparentMode();
  SetProperty(FUI_XY_RESIZING);
  //---Init info image -------------------------------
  MapInfo.rgba = 0;
  //---Create the window -----------------------------
  CFuiWindow::ReadFinished();
  SetTitle("SECTIONAL CHART");
 // strncpy(globals->MapName,"Montreal North.jpg",128);
  OpenChart(globals->MapName);
  // Open stream -------------------------------------
  sizeX = 0;
  sizeY = 0;
  strncpy (filename_, "", 64);
  upltX = 0;
  upltY = 0;
  lwrtX = 0;
  lwrtY = 0;
  lPos.alt = 0.0; lPos.lat = 0.0; lPos.lon = 0.0;
  uPos.alt = 0.0; uPos.lat = 0.0; uPos.lon = 0.0;
  lonx = 0.0; lony = 0.0;
  latx = 0.0; laty = 0.0;
  //--------------------------------------------------
  char  tmp_ [64] = {0};
  char  path [PATH_MAX] = {0};
  strncpy (tmp_, globals->MapName, 64);
  strupper (tmp_);
  int len1 = strlen (tmp_);
  char *map = strchr (tmp_, '.');
  int len2 = strlen (map);
  int lenght = len1-len2;
  if (lenght > 0) {
    tmp_ [lenght] = '\0';
    strncat (tmp_, ".map", 64);
    sprintf(path, "CHARTS/%s", tmp_);
    SStream stream;
    if (OpenRStream (path, stream)) {
      ReadFrom (this, &stream);
      CloseStream (&stream);
    }
    double div  = uPos.lon - lPos.lon;
    if (div) {
      lonx = (upltX - lwrtX) / div;
      lony = ((lwrtX * uPos.lon) - (upltX * lPos.lon)) / div;
    }
    div = uPos.lat - lPos.lat;
    if (div) {
      latx = (upltY - lwrtY) / div;
      laty = ((lwrtY * uPos.lat) - (upltY * lPos.lat)) / div;
    }
  }
  //TRACE ("test %d %d %s", len1, len2, path);
  //TRACE ("test %d %d %s\n %d %d %f %f\n %d %d %f %f\n %f %f\n %f %f",
  //  sizeX, sizeY, filename_, 
  //  upltX, upltY, uPos.lon, uPos.lat,
  //  lwrtX, lwrtY, lPos.lon, lPos.lat,
  //  lonx, lony, latx, laty); 
}
//----------------------------------------------------------------------------------
//  Read MAP file
//  Lat lon format is :
//    33 56 24.6 N          (N 33 deg 56 min 24.6 sec latitude)
//    118 23 56.5 W         (W 118 deg, 23 min, 56.5 sec longitude)
//----------------------------------------------------------------------------------
int  CFuiChart::Read (SStream *stream, Tag tag)
{
  switch (tag) {

  case 'size':
    ReadInt (&sizeX, stream);
    ReadInt (&sizeY, stream);
    return TAG_READ;

  case 'file':
    ReadString (filename_, 64, stream);
    return TAG_READ;

  case 'uplt':
    ReadInt (&upltX, stream);
    ReadInt (&upltY, stream);
    ReadLatLon (&uPos, stream);
    uPos.lon = Wrap180Longitude (uPos.lon); 
    return TAG_READ;

  case 'lwrt':
    ReadInt (&lwrtX, stream);
    ReadInt (&lwrtY, stream);
    ReadLatLon (&lPos, stream);
    return TAG_READ;

  }

  // Send tag to parent class for processing.
  return CFuiWindow::Read (stream, tag);
}
//----------------------------------------------------------------------------------
//  Load the chart
//----------------------------------------------------------------------------------
void  CFuiChart::OpenChart(char *fn)
{ //---Select the document and display ---------------
  char       path[PATH_MAX];
  sprintf(path,"CHARTS/%s",fn);
  if (!globals->txw->LoadImageJPG(path,MapInfo)) return;
  MapInfo.x0 = 0;
  MapInfo.y0 = 0;
  return;
}
//----------------------------------------------------------------------------------
//  Draw the window
//----------------------------------------------------------------------------------
void CFuiChart::Draw ()
{   if (MapInfo.rgba) 
    { EraseSurfaceRGBA (surface, 0);
      // center to aircraft
      // Chambery = 1152-752
      //
      // MapInfo.x0 = -1152 + 400; // center of 800,800 window
      // MapInfo.y0 =  -752 + 400; // center of 800,800 window
      //
      DrawImage(surface, MapInfo);
      //
      // draw plot for aircraft position
      //
      SPosition airPos = globals->sit->uVeh->GetPosition ();
      airPos.lon = Wrap180Longitude (airPos.lon);
      // int posx = 1152 + MapInfo.x0;
      // int posy =  752 + MapInfo.y0;
      //
      int posx = (airPos.lon * lonx + lony) + MapInfo.x0;
      int posy = (airPos.lat * latx + laty) + MapInfo.y0;
      // TRACE ("\t\tpos %d %d", posx, posy);
      U_INT red = MakeRGB (255, 0, 0);
      DrawFastLine (surface, posx-10, posy, posx+10, posy, red); 
      DrawFastLine (surface, posx, posy-10, posx, posy+10, red); 
      // DrawFastLine (surface, 100, 100-10, 100, 100+10, MakeRGB (255, 0, 0)); 
      // DrawFastLine (surface, 100-10, 100, 100+10, 100, MakeRGB (255, 0, 0)); 
      //
      BlitTransparentSurface (surface, 0, 0, 0);
    }
    CFuiWindow::Draw ();
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
