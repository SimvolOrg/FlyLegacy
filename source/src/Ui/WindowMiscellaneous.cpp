/*
 * WindowMiscellaneous.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005 Chris Wallace
 *  Copyright 2007 Jean Sabatier
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

/*! \file WindowMiscellaneous.cpp
 *  \brief Implements FUI callback for Miscellaneous window
 *
 *  Implements the FUI callback for Miscellaneous window.
 */
#include "../Include/Globals.h"
#include "../Include/FlyLegacy.h"
#include "../Include/Fui.h"
#include "../Include/FuiParts.h"
#include "../Include/Utility.h"
#include "../Include/subsystems.h"
#include <algorithm>
#include <vector>
#include <stdarg.h>


//==================================================================================
//  CFuiKLN89:  Window to control the GPS 
//==================================================================================
CFuiKLN89::CFuiKLN89(Tag idn,const char *filename)
:CFuiWindow(idn,filename,0,0,0)
{ tBar->SetText("KLN89 GPS");
  tBar->EditTitle();
  PowST   = K89_PWROF;
  //-------------Font and colors --------------------
  mono20  = (CMonoFontBMP*)(globals->fonts.ftmono20.font);
  amber   = MakeRGB(215, 90, 0);            // amber color
  black   = MakeRGB(0,0,1);                 // Black color
  yelow   = MakeRGB(255,167,102);           // Yellow color
  //------------Init display ------------------------
  InitGPS();
}
//-------------------------------------------------------------------------
//  Init the GPS Window
//-------------------------------------------------------------------------
void CFuiKLN89::InitGPS()
{ CVehicleObject* veh = globals->pln;
  GPSRadio *gps = veh->GetGPS();
	if (0 == gps)									return Close();
	if ('k89g' != gps->GetUnId())	return Close();
  //-------Get the GPS subsystem --------------------------------
  GPS = (CK89gps*)gps;
  //-------Load the front bitmamp -------------------------------
  Front.Load_Bitmap("ART/GPS_ON.TIF");
  //--------Load the back bitmap --------------------------------
  Back.Load_Bitmap("ART/GPS_OFF.TIF");
  //--------Resize window to the bitmap dimension ---------------
  int wd = 0;
  int ht = 0;
  Back.GetBitmapSize(&wd,&ht);
	SetXRange(wd,wd);
	SetYRange(ht,ht);
  SetProperty(FUI_XY_RESIZING);
  Resize(wd,ht);
  RazProperty(FUI_XY_RESIZING);
  SetScreenParameters();
  //--------Replace backplane with GPS picture ------------------
  CFuiPicture *pic = (CFuiPicture*) fBox[BAKW];
  pic->Resize(wd,ht);
  pic->SetBitmap(&Back);
  pic->FillTheRect(xOrg,yBas,dw,dh,black);
  
  return;
}
//-----------------------------------------------------------------------
//  Destroy window
//-----------------------------------------------------------------------
CFuiKLN89::~CFuiKLN89()
{ 
}
//----------------------------------------------------------------------
//  Init display parameters
//----------------------------------------------------------------------
void  CFuiKLN89::SetScreenParameters()
{ xOrg  = 102;                    // Display x position
  yOrg  = 24;                     // Display y position
  yBas  = surface->ySize - 102;   // Display y position
  dw    = 322;                    // Display wide
  dh    = 78;                     // Display height
  hCar  = mono20->CharHeight('H');
  wCar  = mono20->CharWidth('H');
  rBase = xOrg + (6 * wCar) + 2;  // Right part base
  //-------Screen Line table ---------------------------------
  scrLN[0] = yOrg;
  scrLN[1] = scrLN[0] + hCar - 1;
  scrLN[2] = scrLN[1] + hCar - 1;
  scrLN[3] = scrLN[2] + hCar - 1;
  scrLN[4] = scrLN[3] + hCar - 1;
  //-------Underline table -----------------------------------
  undTAB[0] = 0;                  // NUL MODE
  undTAB[1] = 115;                // APT MODE
  undTAB[2] = 142;                // VOR MODE
  undTAB[3] = 170;                // NDB MODE
  undTAB[4] = 198;                // INT MODE
  undTAB[5] = 224;                // USR MODE
  undTAB[6] = 252;                // ACT MODE
  undTAB[7] = 280;                // NAV MODE
  undTAB[8] = 306;                // FPL MODE
  undTAB[9] = 334;                // CAL MODE
  undTAB[10]= 360;                // SET MODE
  undTAB[11]= 385;                // OTH MODE
  //--------Init all click area ------------------------------
  InitArea(0,0,0,0,0);                  // CA0 not used
  InitArea(K89_MSG,108,130,144,152,AREA_BOTH);          // MSG button
  InitArea(K89_OBS,132,130,187,152,AREA_BOTH);          // OBS button
  InitArea(K89_ALT,196,130,232,152,AREA_BOTH);          // ALT button
  InitArea(K89_NRS,242,130,278,152,AREA_BOTH);          // NRS button
  InitArea(K89_DIR,286,130,320,152,AREA_BOTH);          // DIR button
  InitArea(K89_CLR,330,130,366,152,AREA_BOTH);          // CLR button
  InitArea(K89_ENT,376,130,412,152,AREA_BOTH);          // ENT button
  //--------------------------------------------------
  InitArea(K89_CSR,452, 38,490, 60,AREA_BOTH);          // CSR button
  InitArea(K89_PAG,452, 98,494,142,AREA_BOTH);          // SCAN button
  //---------------------------------------------------
  InitArea(K89_APT,110,114,131,122,AREA_BOTH);          // APT button
  InitArea(K89_VOR,140,114,161,122,AREA_BOTH);          // VOR button
  InitArea(K89_NDB,167,114,188,122,AREA_BOTH);          // NDB button
  InitArea(K89_INT,194,114,215,122,AREA_BOTH);          // INT button
  InitArea(K89_USR,221,114,243,122,AREA_BOTH);          // USR button
  InitArea(K89_ACT,248,114,269,122,AREA_BOTH);          // ACT button
  InitArea(K89_NAV,280,114,298,122,AREA_BOTH);          // NAV button
  InitArea(K89_FPL,304,114,325,122,AREA_BOTH);          // FPL button
  InitArea(K89_CAL,330,114,351,122,AREA_BOTH);          // CAL button
  InitArea(K89_SET,357,114,378,122,AREA_BOTH);          // SET button
  InitArea(K89_OTH,385,114,406,122,AREA_BOTH);          // OTH button
  //--------------------------------------------------
  InitArea(K89_DPY,102, 24,424,102,AREA_BOTH);          // DISPLAY
  InitArea(K89_POW, 52,110, 94,154,AREA_BOTH);          // POWER
  //----Init letter click fields ---------------------------------
  IniLF(1,GPS->Ident);      // IDENT Input field      APT-VOR-NDB page 01-
  IniLF(2,GPS->Name);       // NAME Input field       APT-VOR-NDB page 01-
  IniLF(3,GPS->Name+3);     // NAME Input field       WPT page 01-----------
  IniLF(4,GPS->Ident);      // IDENT Direct To field  APT-VOR-NDB------
  IniLF(5,GPS->Name+4);     // NAME  Direct To field  WPT---------------
  IniLF(6,GPS->IdFR);       // FROM in CAL page 1
  IniLF(7,GPS->IdTO);       // TO   in CAL page 1
  IniLF(8,GPS->IdFR);       // FROM in CAL page 2
  IniLF(9,GPS->IdTO);       // TO   in CAL page 2
  //---------Repeatable field ------------------------------------------
  IniRP(1,&GPS->AltiTG);    // Altitude field       NVAV-
  IniRP(2,0);               // Waypoint ident       VNAV-
  IniRP(3,&GPS->OffsVL);    // Offset field         VNAV-
  IniRP(4,&GPS->GrndSP);    // Ground speed         VNAV-
  IniRP(5,&GPS->VspdVL);    // VSI speed            VNAV
  IniRP(6,&GPS->CalSPD);    // Ground speed         CAL page 1
  IniRP(7,&GPS->CalSPD);    // Ground Speed         CAL Page 2
  IniRP(8,&GPS->CalFFL);    // Fuel flow            CAL page 2
  IniRP(9,&GPS->CalFRS);    // Fuel reserve         CAP page 2
  //---------Init simple page fields -----------------------------------
  IniPF( 1);      // APT-VOR-NDB-WPT nearest number
  IniPF( 2);      // APT-VOR-NDB-WPT to/from
  IniPF( 3);      // NAV Page 01
  IniPF( 4);      // NAV PAge 02
  IniPF( 5);      // ALT Page 01 BARO Part 1
  IniPF( 6);      // ALT Page 01 BARO Part 2
  IniPF( 7);      // NAV in VNAV page 01
  IniPF( 8);      // WPT option CAL page 1
  IniPF( 9);      // WPT option CAL page 2
  return;
}
//----------------------------------------------------------------------------
//  Init click area
//----------------------------------------------------------------------------
void CFuiKLN89::InitArea(int No,short x1, short y1, short x2, short y2,U_CHAR df)
{ ca[No].Type = df;
  ca[No].x1  = x1;                  
  ca[No].y1  = y1;
  ca[No].x2  = x2;
  ca[No].y2  = y2;
  return;
}
//----------------------------------------------------------------------------
//  Detect power transition
//----------------------------------------------------------------------------
bool CFuiKLN89::PowerChange(U_CHAR astate)
{ U_CHAR ppower = PowST;
  PowST = astate;
if (astate == 0)
int a = 0;
  if (ppower    == astate)                  return false;
  if (K89_PWROF == astate)
  {   EraseSurfaceRGBA(surface,0);
      ClearGPSwindow(black);
      return true;  }
  if (K89_PWROF == ppower) 
  {   Front.DrawBitmap(surface,0,0,0);
      return true;}
  return false; }
//-----------------------------------------------------------------------------
//  Clear the GPS window in the given color
//  NOTE:  This routine is graphic dependent
//-----------------------------------------------------------------------------
void CFuiKLN89::ClearGPSwindow(U_INT col)
{ U_INT *des = surface->drawBuffer + (yBas * surface->xSize) + xOrg;
  U_SHORT nl = dh;
  U_SHORT cl = 0;
  while (nl--)
  { while (cl++ != dw) *des++ = col;
    des += (surface->xSize - dw);
    cl  = 0;
  }
  return;
}
//----------------------------------------------------------------------------
//  Draw GPS
//----------------------------------------------------------------------------
void CFuiKLN89::Draw()
{ CFuiWindow::Draw();
  if (0 == GPS) {return;}
  //-------Detect Power change------------------------------
  U_CHAR  state = GPS->GetAState();
  PowerChange(state);
  fMask = GPS->GetFmask();
  if (state >= K89_PWRNA)  (this->*DshTAB[GPS->GetScreenHandler()])();
  BlitOpaqueSurface (surface, 0, 0, 0);
  return;
}
//----------------------------------------------------------------------------
//  Draw full display
//----------------------------------------------------------------------------
void CFuiKLN89::DrawAllDisplay()
{ short x0      = xOrg;
  short y0      = 0;
  U_SHORT *org  = GPS->GetDisplay();
  short    nl   = 0;
  short    nc   = 0;
  U_SHORT  car  = 0;
  U_INT    col  = amber;
  mono20->SetOpaque();
  while (GPS->NotLastLine(nl))
  { nc  = GPS->GetNberColn();
    x0  = xOrg;
    y0  = scrLN[nl++];
    while (nc--)
    { col = amber;
      car = *org++;
      if (0x0100 & car)  car &= fMask; 
      if (0x0400 & car)  col  = yelow;
      x0 += mono20->DrawChar(surface,x0,y0,(char)(car),col);
    }
  }
  return;
}
//----------------------------------------------------------------------------
//  Draw the CDI
//  the CDI amplitude is 11 char * 13 bits = 143
//      CDI middle    is 71 but to account for the char | it is set to 65
//      CDI range is [-40°,+40°]
//----------------------------------------------------------------------------
void CFuiKLN89::DrawCDIneedle()
{ short cdi = GPS->GetCDIcoln();
  //-------Draw CDI deviation --(positive=>Left deviation)------------------
  if (0 == cdi) return;
  int y0  = scrLN[GPS->GetCDIline()];
  int mid =  rBase + (cdi * wCar)  + (65);
  int bar = -int((143 * GPS->GetCDIdev()) / 40);
  //-------Clamp result to [0-71] -----------------------------
  if (-71 > bar) bar = -71;
  if (+71 < bar) bar = +71;
  mono20->SetTransparent();
  mono20->DrawChar(surface,(mid + bar),y0,'|',amber);
  return;
}
//-----------------------------------------------------------------------------
//  Draw left display to surface
//-----------------------------------------------------------------------------
void CFuiKLN89::DrawLSPDisplay()
{ short x0      = 0;
  short y0      = 0;
  U_SHORT *org  = GPS->GetDisplay();
  U_INT    fac  = GPS->GetFactor();
  short    nl   = 0;
  short    nc   = 0;
  U_SHORT  car  = 0;
  U_INT    col  = amber;
  mono20->SetOpaque();
  while (GPS->NotLastLine(nl))
  { nc  = GPS->GetRightColn();
    x0  = xOrg;
    y0  = scrLN[nl++];
    while (nc--)
    { col = amber;
      car = *org++;
      if (0x0100 & car)  car &= fMask; 
      if (0x0400 & car)  col = yelow;
      x0 += mono20->DrawChar(surface,x0,y0,(char)(car),col);
    }
    org += fac;
  }
  return;
}
//-----------------------------------------------------------------------------
//  Draw rigth display to surface
//-----------------------------------------------------------------------------
void CFuiKLN89::DrawRSPDisplay()
{ short x0      = 0;
  short y0      = 0;
  U_SHORT *org  = GPS->GetRigthDisplay();
  U_INT    fac  = GPS->GetRightColn();
  short    nl   = 0;
  short    nc   = 0;
  U_SHORT  car  = 0;
  U_INT    col  = amber;
  mono20->SetOpaque();
  while (GPS->NotLastLine(nl))
  { nc  = GPS->GetRightSize();
    x0  = rBase;
    y0  = scrLN[nl++];
    while (nc--)
    { col = amber;
      car = *org++;
      if (0x0100 & car)  car &= fMask; 
      if (0x0400 & car)  col = yelow;
      x0 += mono20->DrawChar(surface,x0,y0,(char)(car),col);
    }
    org += fac;
  }
  return;
}
//-----------------------------------------------------------------------------
//  Draw the GPS in 2 parts with divider in the middle and mode underline
//-----------------------------------------------------------------------------
void CFuiKLN89::DrawLDRDisplay()
{ int x0 = undTAB[GPS->GetPage()]; 
  int y0 = scrLN[4];
  DrawRect(surface,xOrg,y0,(xOrg+dw),(y0+1),black);
  DrawFastLine(surface,(rBase-2),yOrg,(rBase-2),(yOrg + dh - 2),amber);
  DrawLSPDisplay();
  DrawRSPDisplay();
	if (x0)	DrawHLine(surface,x0,(x0+15),y0,amber);
  DrawCDIneedle();
  return;
}
//-----------------------------------------------------------------------------
//  Open the GPS window
//  NOTE:  This routine is graphic dependent
//-----------------------------------------------------------------------------
void CFuiKLN89::DrawGPSwindow()
{ ClearGPSwindow(amber);
  float step  = GPS->GetOpeningStep();
  short y0    = yOrg;
  short y1    = y0 + dh;
  short lg    = int(dw * step) >> 1;
  short xm    = xOrg + (dw >> 1);
  FillRect(surface,(xm -  lg),y0,(xm + lg),y1,black);
  return;
}
//============================================================================
//  Display Vector
//============================================================================
CFuiKLN89::DisFN CFuiKLN89::DshTAB[4] = {
  &CFuiKLN89::DrawAllDisplay,             // 0 Draw full display
  &CFuiKLN89::DrawLDRDisplay,             // 1 Draw part display
  &CFuiKLN89::DrawGPSwindow,              // 2 Opening window
};
//===========================================================================
//  CLICK HANDLERS 
//===========================================================================
//------------------------------------------------------------------------------
//  Init Input Field (type letter click)
//------------------------------------------------------------------------------
void CFuiKLN89::IniLF(char No,char *fd)
{ short lc            = GPS->LetPmTAB[No].Left;
  short ln            = GPS->LetPmTAB[No].Line;
  short sz            = GPS->LetPmTAB[No].Right;
  letTAB[No].aChar    = fd;
  letTAB[No].Line     = ln;
  letTAB[No].cPos     = 0;
  letTAB[No].Iden     = GPS->LetPmTAB[No].Iden;
  letTAB[No].aFld     = GPS->LetPmTAB[No].aFld;
  letTAB[No].aDisp    = GPS->Display + (ln * K89_NBR_COLN) + lc;
  letTAB[No].Left     = (lc        - K89_CLN06) * wCar;
  letTAB[No].Right    = ((lc + sz) - K89_CLN06) * wCar;
  letTAB[No].Next     = GPS->LetPmTAB[No].Next;
  letTAB[No].Type     = GPS->LetPmTAB[No].Type;
  letTAB[No].xFld     = GPS->LetPmTAB[No].xFld;
  return;
}
//------------------------------------------------------------------------------
//  Init reapetable Input Field
//------------------------------------------------------------------------------
void CFuiKLN89::IniRP(char No,int *nm)
{ short lc            = GPS->RepPmTAB[No].Left;
  short ln            = GPS->RepPmTAB[No].Line;
  short sz            = GPS->RepPmTAB[No].Right;
  repTAB[No].aNber    = nm;
  repTAB[No].Left     = (lc         - K89_CLN06) * wCar;
  repTAB[No].Right    = ((lc + sz)  - K89_CLN06) * wCar;
  repTAB[No].Line     = ln;
  repTAB[No].vIncr    = GPS->RepPmTAB[No].vIncr;
  repTAB[No].nbMin    = GPS->RepPmTAB[No].nbMin;
  repTAB[No].nbMax    = GPS->RepPmTAB[No].nbMax;
  repTAB[No].Event    = GPS->RepPmTAB[No].Event;
  repTAB[No].Next     = GPS->RepPmTAB[No].Next;
  repTAB[No].Type     = GPS->RepPmTAB[No].Type;
  return;
}
//------------------------------------------------------------------------------
//  Init simple page Field
//------------------------------------------------------------------------------
void CFuiKLN89::IniPF(char No)
{ short lc          = GPS->PagPmTAB[No].Left;
  short sz          = GPS->PagPmTAB[No].Right;
  pagTAB[No].Event  = GPS->PagPmTAB[No].Event;
  pagTAB[No].Left   = (lc        - K89_CLN06) * wCar;
  pagTAB[No].Right  = ((lc + sz) - K89_CLN06) * wCar;
  pagTAB[No].Line   = GPS->PagPmTAB[No].Line;
  pagTAB[No].Next   = GPS->PagPmTAB[No].Next;
  pagTAB[No].Type   = GPS->PagPmTAB[No].Type;
  return;
}
//------------------------------------------------------------------------------
//  Mouse click over the GPS
//  Generate Event and process according to GPS state
//------------------------------------------------------------------------------
bool CFuiKLN89::InsideClick (int x, int y, EMouseButton buttons)
{ bool rc   = false;
  int Event = K89_NUL;
  int mx    = x - surface->xScreen;
  int my    = y - surface->yScreen;
  if (0 == GPS) return rc; 
  GPS->SetMouseDir((MOUSE_BUTTON_LEFT == buttons)?(-1):(+1));
  for (int i=K89_MSG; (i< K89_DIM) && (Event==0); i++) {
    if (ca[i].IsHit(mx, my)) {
      Event = i;
      rc = true;
      break;
    }
  }
  if (K89_NUL == Event)   return rc;
  //-----------Check for additional click according to click vector ---
  if (K89_DPY == Event) Event = (this->*HndTAB[GPS->HndNO])((mx - rBase),my);
  //-----------Call the state function --------------------------------
  GPS->Dispatch((K89_EVENT) Event);
  return rc;
}
//-----------------------------------------------------------------------------
//  Void Clicker
//-----------------------------------------------------------------------------
int CFuiKLN89::AnyNULclick(int mx,int my)
{ return K89_NUL;
}
//-----------------------------------------------------------------------------
//  Check for any click on current input or alternate
//-----------------------------------------------------------------------------
int CFuiKLN89::AnyLETclick(int mx, int my)
{ U_CHAR     xf = GPS->LetFD->xFld;               // Field number
  K89_LETFD *fd = letTAB + xf;
  U_CHAR     No = fd->Iden;
  do  { if (AnyCHRclick(fd,mx,my))  return K89_DPY;
        if (0 == fd->aFld)          return K89_NUL;
        fd = letTAB + fd->aFld;
      }
  while (No != fd->Iden);
  return K89_NUL;
}
//-----------------------------------------------------------------------------
//  Click Handler N°1
//  Check for any click on letter of the field
// YES => Set LetFD and post event
// NO  => Prepare next field to test
//-----------------------------------------------------------------------------
int CFuiKLN89::AnyCHRclick(K89_LETFD *fd,int mx,int my)
{ char ln = fd->Line;
  char y0 = scrLN[ln];
  char y1 = scrLN[ln+1];
  WtyFD   = fd->Type;
  WnoFD   = fd->Next;
  if ((my < y0) || (my >= y1))  return K89_NUL;
  U_CHAR pos = 0;
  int  x0 = fd->Left;
  int  x1 = x0 + wCar;
  do  { if ((mx >= x0) && (mx < x1))
        { fd->cPos = pos;
          GPS->LetFD = fd;
          return K89_DPY;
        }
        pos++;
        x0 = x1;
        x1 += wCar;
  } while (x0 != fd->Right);
  return K89_NUL;
}
//-----------------------------------------------------------------------------
//  Click Handler N°2
//  Check for click on simple field
// YES => Post event
// NO  => Prepare next field to test
//-----------------------------------------------------------------------------
int CFuiKLN89::AnySPFclick(K89_PAGFD *fd,int mx,int my)
{ char ln = fd->Line;
  char y0 = scrLN[ln];
  char y1 = scrLN[ln+1];
  WtyFD   = fd->Type;
  WnoFD   = fd->Next;
  if ((my >= y0) && (my <y1) && (mx >= fd->Left) && (mx < fd->Right)) return fd->Event;
  return K89_NUL;
}
//-----------------------------------------------------------------------------
//  Click Handler N°3
//  Check for any click on repeat field
// YES => Set RepCK and post event
// NO  => Prepare next field to test
//-----------------------------------------------------------------------------
int CFuiKLN89::AnyFLDclick(K89_REPFD *fd,int mx,int my)
{ char ln = fd->Line;
  char y0 = scrLN[ln];
  char y1 = scrLN[ln+1];
  WtyFD   = fd->Type;
  WnoFD   = fd->Next;
  if ((my >= y0) && (my <y1) && (mx >= fd->Left) && (mx < fd->Right))
  { fd->vIncr = (abs(fd->vIncr) * GPS->msDIR);
    if (fd->aNber) GPS->RepCK = fd;
    return fd->Event;
  }
  return K89_NUL;
}
//-----------------------------------------------------------------------------
//  Check for any click on any field
//-----------------------------------------------------------------------------
int CFuiKLN89::AnyGENclick(int mx,int my)
{ WtyFD       = GPS->StyFD;              // Start field type
  WnoFD       = GPS->SnoFD;              // Start field number
  int evn     = K89_NUL;
  GPS->RepCK  = 0;
  while (WnoFD)
  { switch (WtyFD)
     {  case K89_LET_FD:
        { K89_LETFD *fd = letTAB + WnoFD;
          evn = AnyCHRclick(fd,mx,my);
          if (evn)  return evn;
          continue;
        }
        case K89_REP_FD:
        { K89_REPFD *fd = repTAB + WnoFD;
          evn = AnyFLDclick(fd,mx,my);
          if (evn)  return evn;
          continue;
        }
        case K89_PAG_FD:
        { K89_PAGFD *fd = pagTAB + WnoFD;
          evn = AnySPFclick(fd,mx,my);
          if (evn)  return evn;
          continue;
        }
    }
  }
  return K89_NUL;
}
//------------------------------------------------------------------------------
//  Mouse Stop Clicking. Disarm repetition
//------------------------------------------------------------------------------
bool CFuiKLN89::MouseStopClick (int x, int y, EMouseButton buttons)
{ if (GPS)  GPS->RepCK = 0;
  return CFuiWindow::MouseStopClick(x,y,buttons);
}
//===============================================================================
//  CLICK HANDLER VECTOR TABLE
//===============================================================================
CFuiKLN89::ClkFN CFuiKLN89::HndTAB[4] = 
{ 
    &CFuiKLN89::AnyNULclick,                // 0 NUL handler
    &CFuiKLN89::AnyLETclick,                // 1 Letter handler
    &CFuiKLN89::AnyGENclick,                // 2 Generic handler
    0,                                      // 3 RFU
};


//========================================================================
// CFuiConfirmQuit
//
// CFuiWindow subclass to handle the program quit confirmation dialog
//=========================================================================
CFuiConfirmQuit::CFuiConfirmQuit (Tag id, const char* filename)
: CFuiWindow (id, filename, NULL, NULL, NULL)
{
  SetTitle ("Confirm Program Quit");
  CFuiComponent* mesg = GetComponent ('mesg');
  mesg->SetText ("Are you sure you want to quit?");
}
//-------------------------------------------------------------------------
//  Event notification
//-------------------------------------------------------------------------
void CFuiConfirmQuit::NotifyChildEvent (Tag id, Tag component, EFuiEvents evn)
{ switch (evn) {
  case EVENT_BUTTONPRESSED:
    switch (component) {
    case 'okok':
      // Quit gracefully by setting application mode.  This will cause shutdown
      //   on the next redraw() cycle
      globals->appState = APP_EXIT_SCREEN;
      break;

    case 'canc':
      // Close window without exiting program
      Close ();
    }
    break;

  default:
    // Do nothing
    ;
  }
}
//========================================================================
// Error Message
//
//=========================================================================
CFuiErrorMSG::CFuiErrorMSG (Tag id, const char* filename)
: CFuiWindow (id, filename, NULL, NULL, NULL)
{ mesg = (CFuiLabel*)GetComponent ('mesg');
}
//-------------------------------------------------------------------------
//  Event notification
//-------------------------------------------------------------------------
void CFuiErrorMSG::NotifyChildEvent (Tag id, Tag itm, EFuiEvents evn)
{	Close();
	return;
}
//=========================END of FILE =============================================
