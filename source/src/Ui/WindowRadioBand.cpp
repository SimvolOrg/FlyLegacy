/*
 * WindowRadioBand.cpp
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
#include "../Include/Atmosphere.h"
#include <vector>
//==================================================================================
//  Window to display Radio Messages
//==================================================================================
//============================================================================================
//  Class CFuiRadioBand to display radio communication
//============================================================================================
CFuiRadioBand::CFuiRadioBand()
{ globals->rdb = this;
  surf  = 0;
  font    = &(globals->fonts.ftmono14);
  fnts    = (CFont*)font->font;
  ht      = (short)(fnts->TextHeight ("H"));
  wCar    = (short)(fnts->TextWidth("AVERAGE") / 7);
  back    = MakeRGB (212, 212, 212);
  black   = MakeRGB(0,0,0);
  sTxt.font   = font;
  sTxt.color  = black;
  Resize();  
  state = RB_STOP;
  //---------Time init ---------------------------------------------------
  lTime = globals->tim->GetLocalDateTime();
  timTM = 33;                               // Force update first
  nInfo = 130;                              // Force update first
  tInfo = 3;
  hour[0]   = 0;
  eVis[0]   = 0;
  eTmp[0]   = 0;
  eFrq[0]   = 0;
  //----------Init all stations ------------------------------------------
  SetATIStext();
}
//---------------------------------------------------------------------------------
//  Set ATIS text
//---------------------------------------------------------------------------------
void CFuiRadioBand::SetATIStext()
{ atis[AT_IDEN] = "???";
  atis[AT_TXT1] = " Arrival informations";
  atis[AT_TIDN] = globals->abcTAB[tInfo];
  atis[AT_TIMH] = hour;
  atis[AT_HIDN] = " ZULU";
  atis[AT_TXT2] = ". Automated weather observations. Wind";
  atis[AT_WIND] = " calm";
  atis[AT_TXT3] = ". Visibility";
  atis[AT_MVIS] = eVis;
  atis[AT_TXT4] = " miles. Temperature is";
  atis[AT_TEMP] = eTmp;
  atis[AT_TXT5] = " degrees. Dewpoint is";
  atis[AT_DEWP] = " 20";
  atis[AT_TXT6] = " degrees. Altimeter is";
  atis[AT_ALTI] = " 29.92";
  atis[AT_TXT7] = ". Notice to airmen: For departure, contact clearance delivery at";
  atis[AT_CLDY] = eFrq;
  atis[AT_TXT8] = ". Prior to taxi advise on initial contact you have informations";
  atis[AT_IDNT] = atis[AT_TIDN];
  return;
}
//---------------------------------------------------------------------------------
//  Destroy sRadio Band
//---------------------------------------------------------------------------------
CFuiRadioBand::~CFuiRadioBand()
{ if (surf) surf = FreeSurface (surf);
}
//---------------------------------------------------------------------------------
//  Resize parameters
//---------------------------------------------------------------------------------
void CFuiRadioBand::Resize()
{ if (surf) delete surf;
  wd    = globals->mScreen.Width;
  sDeb  = 5 * wCar;
  NbCar = wd / wCar;
  sEnd  = (NbCar - 5) * wCar;
  xScr  = 0;
  surf  = CreateSurface(wd,ht);
  return;
}
//---------------------------------------------------------------------------------
//  Draw the surface
//  Should be called  from CFuiManager contex
//---------------------------------------------------------------------------------
void  CFuiRadioBand::Draw()
{ if ((0 == surf) || (RB_STOP == state)) return;

  glDrawBuffer (GL_BACK);
    // Starting raster position is bottom-left corner of the surface,
    //   with (0,0) at top-left of the screen
  glRasterPos2i (surf->xScreen, surf->yScreen + surf->ySize);
  glDrawPixels  (surf->xSize,   surf->ySize,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      surf->drawBuffer);
  return;
}
//---------------------------------------------------------------------------------
//  Change informations
//  -Time info (ALPHA, BRAVO, etc
//  -Time zone info TODO
//  -Wind force   TODO at airport position and elevation
//  -Visibility
//  -Temperature  TODO at airport position and altitude
//  -Dewpoint     TODO when atmosphere is OK
//  -Altimeter    TODO at airport position and altitude
//  -Clearance Frequency dépending on airport type
//---------------------------------------------------------------------------------
void CFuiRadioBand::UpdateInfo()
{ nInfo = 0;
  tInfo = (tInfo == 26)?(1):(tInfo + 1);
  atis[AT_TIDN] = globals->abcTAB[tInfo];
  float vis = 40;
  if (fabs(vis) < 100)  sprintf(eVis," %.0f",vis);
  float tmp     = globals->atm->GetTemperatureC();
  if (fabs(tmp) < 100)  sprintf(eTmp," %.0f",tmp);
  return;
}
//---------------------------------------------------------------------------------
//  Update Time sensitive data every 30 second
//  -Change info every half hour
//---------------------------------------------------------------------------------
void  CFuiRadioBand::UpdateTime()
{ timTM -= 30;
  lTime = globals->tim->GetLocalDateTime();
  sprintf(hour," %02uH%02u",lTime.time.hour,lTime.time.minute);
  nInfo += 1;
  if (nInfo > 60)  UpdateInfo();
  return;
}
//---------------------------------------------------------------------------------
//  Time slice called from CSituation
//---------------------------------------------------------------------------------
void  CFuiRadioBand::TimeSlice(float dT)
{  
  if (0 == surf)            return;
  timTM += dT;
  if (timTM >= 30)  UpdateTime();
  switch (state)  {
    case RB_STOP:
      return;
      //-----Init drawing ------------------------------------
    case RB_INIT:
      nCar  = 1;                    // 1 character to screen
      xScr  = sEnd;                 // Starting screen position
      cPos  = xScr - wCar;          // Next charater to screen
      state = RB_DSP1;
      sTxt.Reset();
      return;

    case RB_DSP1:
      //----Draw text.  Left screen position not reached -----
      EraseSurfaceRGB(surf,back);
      DrawTextList(surf,xScr,0,&sTxt,nCar);
      xScr--;                       // Next  screen position
      if (xScr == cPos) {nCar++; cPos -= wCar;}
      if (xScr == sDeb)  state = RB_DSP2;
      return;

    case RB_DSP2:
      // -- Draw text starting at left screen position until end of text --
      EraseSurfaceRGB(surf,back);
      if (xScr == sDeb) {xScr += (wCar - 1); sTxt.Increment();}
      DrawTextList(surf,xScr,0,&sTxt,nCar);
      xScr--;
      if (sTxt.HasMore())   return; 
      state = RB_DEND;
      return;

    case RB_DEND:
      //--- Draw an empty windows ---------------------------
      EraseSurfaceRGB(surf,back);
      state = RB_STBY;
      return;

    case RB_STBY:
      //----Do restart  -------------------------------------
      sTxt.Restart();
      state = RB_INIT;
      return;

  }
  return;
}
//-----------------------------------------------------------------------------------
//  Tune to a comm radio
//  -Free the previous com if any by assigning the smart pointer
//-----------------------------------------------------------------------------------
bool CFuiRadioBand::TuneTo(CCOM *com)
{ if (com == rCOM.Pointer())          return false;
  rCOM  = com;                                // NOTE: Let this assignation even if com = 0
  state = RB_STOP;
  if  (0 == com)                      return false;
  if (com->IsATI()) return ModeATIS(com);
  rCOM  = 0;
  return false;
}
//-----------------------------------------------------------------------------------
//    Edit airport data
//-----------------------------------------------------------------------------------
CFuiRadioBand::COM_ATIS  CFuiRadioBand::EditATISdata(CAirport *apt)
{ if (0 == apt)   return AT_SMAL;
  atis[AT_IDEN] = apt->GetName();
  float cfrq    = apt->GetClearanceFreq();
  if (0 == cfrq)  return AT_SMAL;
  sprintf(eFrq," %03.2f",cfrq);
  return AT_FULL;
}
//-----------------------------------------------------------------------------------
//    ATHIS MODE. Edit ATIS parameters from airport
//-----------------------------------------------------------------------------------
bool CFuiRadioBand::ModeATIS(CCOM *com)
{ CAirport *apt = com->GetAirport();
  COM_ATIS  dim = EditATISdata(apt);
  sTxt.Init(dim,atis);
  state = RB_INIT;
  return true;
}
//============================END OF FILE ================================================================================
