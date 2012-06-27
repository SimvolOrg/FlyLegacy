//============================================================================================
// WindowWeather.cpp
//
// Part of Fly! Legacy project
//
// Copyright 2005 Chris Wallace
// Copyright 2007 Jean Sabatier
// Fly! Legacy is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
// Fly! Legacy is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
//   along with Fly! Legacy; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//===========================================================================================
#include "../Include/FlyLegacy.h"
#include "../Include/Utility.h"
#include "../Include/Globals.h"
#include "../Include/Fui.h"
#include "../Include/FuiParts.h"
#include "../Include/Weather.h"
#include "../Include/Atmosphere.h"
//=====================================================================================
//=====================================================================================
//  Weather overview
//=====================================================================================
CFuiWeatherView::CFuiWeatherView(Tag idn, const char *filename)
:CFuiWindow(idn,filename,0,0,0)
{ char *erm = "Invalid template %s";
  //---Get window components -------------------------------
  datW  = (CFuiLabel*)GetComponent('date');
  if (0 == datW)  gtfo(erm,filename);
  timW  = (CFuiLabel*)GetComponent('time');
  if (0 == datW)  gtfo(erm,filename);
  utcW  = (CFuiLabel*)GetComponent('utc_');
  if (0 == utcW)  gtfo(erm,filename);
  disW  = (CFuiLabel*)GetComponent('dist');
  if (0 == disW)  gtfo(erm,filename);
  locW  = (CFuiLabel*)GetComponent('loca');
  if (0 == locW)  gtfo(erm,filename);
  winW  = (CFuiLabel*)GetComponent('wind');
  if (0 == winW)  gtfo(erm,filename);
  altW  = (CFuiLabel*)GetComponent('alti');
  if (0 == altW)  gtfo(erm,filename);
  tmpW  = (CFuiLabel*)GetComponent('temp');
  if (0 == tmpW)  gtfo(erm,filename);
  barW  = (CFuiLabel*)GetComponent('baro');
  if (0 == barW)  gtfo(erm,filename);
  cldW  = (CFuiLabel*)GetComponent('clod');
  if (0 == cldW)  gtfo(erm,filename);
  //---------------------------------------------------------
  Apt   = 0;
  Req.SetWindow(this);
}
//---------------------------------------------------------------------
//  Refresh Date
//---------------------------------------------------------------------
void CFuiWeatherView::EditDate()
{ CClock *ck = globals->clk;
  datW->EditText("%04d/%02d/%02d",ck->GetYear(),ck->GetMonth(),ck->GetDay());
  return;
}
//---------------------------------------------------------------------
//  Refresh Time
//---------------------------------------------------------------------
void CFuiWeatherView::EditTime()
{ SDateTime		uT = globals->tim->GetUTCDateTime();   // Universal Date time
	SDateTime		lT = globals->tim->GetLocalDateTime(); // Local time
  timW->EditText("%02u:%02u:%02u (local)",lT.time.hour,lT.time.minute,lT.time.second);
  utcW->EditText("%02u:%02u:%02u (UTC)",  uT.time.hour,uT.time.minute,uT.time.second);
  return;
}
//---------------------------------------------------------------------
//  No location
//---------------------------------------------------------------------
void CFuiWeatherView::NoLocation()
{ disW->EditText("");
  locW->EditText("");
  return;
}
//---------------------------------------------------------------------
//  Refresh location
//---------------------------------------------------------------------
void CFuiWeatherView::EditLocation()
{ CmHead *apt = globals->dbc->FindFirstNearest(APT);
  if (0 == apt)  return NoLocation();
  float dx = apt->GetDistLon() / 128;
  float dy = apt->GetDistLat() / 128;
  float dis  = SquareRootFloat((dx*dx)+(dy*dy));
  disW->EditText("%.2f miles from ",dis);
  locW->SetText (apt->GetName());
  return;
}
//---------------------------------------------------------------------
//  Refresh Altitude
//---------------------------------------------------------------------
void CFuiWeatherView::EditAltitude()
{ altW->EditText("%.0f feet",globals->geop.alt);
  return;
}
//---------------------------------------------------------------------
//  Refresh wind
//---------------------------------------------------------------------
void CFuiWeatherView::EditWind()
{ float spd = globals->wtm->WindSpeed();
  float frm = globals->wtm->WindFrom();
  int   dir = int(frm);
  winW->EditText("%.1f KTS from %03d",spd,dir);
  return;
}
//---------------------------------------------------------------------
//  Refresh temperature
//---------------------------------------------------------------------
void CFuiWeatherView::EditTemp()
{ float td = globals->atm->GetTemperatureC();
  float tf = globals->atm->GetTemperatureF();
  tmpW->EditText("%.0f C      %.0f F",td,tf);
  return;
}
//---------------------------------------------------------------------
//  Refresh barometer
//---------------------------------------------------------------------
void CFuiWeatherView::EditBarometer()
{ float hp = globals->atm->GetPressureHPA();
  float gp = globals->atm->GetPressureInHG();
  barW->EditText("%.2f hPa  %.2f inHg",hp,gp);
  return;
}
//---------------------------------------------------------------------
//  Refresh barometer
//---------------------------------------------------------------------
void CFuiWeatherView::EditCloud()
{ int   cl = globals->wtm->GetCloudLayer();
  char *cc = globals->wtm->GetCloudCover();
  float at = globals->wtm->GetCloudCeil();
  if (cl) cldW->EditText("%s at %.0f feet",cc,at);
  else    cldW->SetText(cc);
  return;
}
//---------------------------------------------------------------------
//  Refresh item to draw
//---------------------------------------------------------------------
void CFuiWeatherView::Draw()
{ EditDate();
  EditTime();
  EditLocation();
  EditAltitude();
  EditWind();
  EditTemp();
  EditBarometer();
  EditCloud();
  CFuiWindow::Draw();
}
//=======================END OF FILE ==================================================