/*
 * New Gauges
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005 Chris Wallace
  * Copyright 2007 Jean Sabatier
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
#include "../Include/NewGauge.h"
#include "../Include/Globals.h"
//================================================================
//  DOAT GAUGE
//================================================================
CHawkClockGauge::CHawkClockGauge (CPanel *mp)
:CGauge(mp)
{ Prop.Rep(NO_SURFACE);
  col   = MakeRGB (1,1,1);
  strncpy(edNb,"+dddL",8);			// Number format
  btup  = 2;
  btsl  = 0;
  btst  = 0;
  tLn   = 16;
  tCn   = 24;
  // Get fonts from global defaults
  SFont* sf  = &(globals->fonts.ftradi9);
  cFont = (CFont*)sf->font;
  wCar  = cFont->CharWidth('C');
  hCar  = cFont->CharHeight('H');
}
//-------------------------------------------------------------------------------
//  Destroy it
//-------------------------------------------------------------------------------
CHawkClockGauge::~CHawkClockGauge (void)
{ }

//-------------------------------------------------------------------------------
//  read time position
//-------------------------------------------------------------------------------
void CHawkClockGauge::ReadTIME(SStream *stream)
{ char txt[128];
  int p1, p2;
  ReadString(txt,128,stream);
  int nf = sscanf(txt,"%d , %d", &p1, &p2);
  if (2 != nf)  gtfo("<time> bad parameters ");
  //------------------------------------------------------
  tLn = p2;
  tCn = p1;
  return;
}
//-------------------------------------------------------------------------------
//  read Line position
//-------------------------------------------------------------------------------
void CHawkClockGauge::ReadUNDR(SStream *stream, int No)
{ char txt[128];
  int xa,xb,ln;
  ReadString(txt,128,stream);
  int nf = sscanf(txt,"%d , %d, %d",&xa, &xb, &ln);
  if (3 != nf)  gtfo("<undr> bad parameters");
  und[No].Set(xa,xb,ln);
  return;
}
//------------------------------------------------------------------
//  Change font
//------------------------------------------------------------------
void CHawkClockGauge::ReadFONT(SStream *stream)
{ char txt[128];
  ReadString(txt,128,stream);
  if (strcmp(txt,"big") != 0)  gtfo("unsupported font");
  SFont* sf = &(globals->fonts.ftmono14);
  cFont = (CFont*)sf->font;
  hCar  = cFont->CharHeight('H');
  wCar  = cFont->CharWidth (' ');
  return;
}
//-------------------------------------------------------------------------------
//  read Parameters
//-------------------------------------------------------------------------------
int CHawkClockGauge::Read (SStream *stream, Tag tag)
{ switch (tag) {
    //--- Display surface ----------
    case 'disp':
      DecodeDisplay(stream);
      SetSurface(dsw,dsh);
      return TAG_READ;
    //--- upper button -------------
    case 'BTup':
      ReadFrom (&bupp, stream);
      return TAG_READ;
    //--- control button -----------
    case 'BTct':
      ReadFrom (&bcon, stream);
      return TAG_READ;
    //--- Select button -------------
    case 'BTsl':
      ReadFrom (&bsel, stream);
      return TAG_READ;
    //--- Both button --------------
    case 'BT2b':
      ReadFrom (&bbot, stream);
      return TAG_READ;
    //--- surface bitmap -----------
    case 'bmap':
      ReadStrip(stream,txd);
      return TAG_READ;
    //--- Change font --------------
    case 'font':
      ReadFONT(stream);
      return TAG_READ;
    //--- Time position -------------
    case 'time':
      ReadTIME(stream);
      return TAG_READ;
    //--- Universal time -------------
    case 'lnut':
      ReadUNDR(stream,0);
      return TAG_READ;
    //--- Local time ----------------
    case 'lnlt':
      ReadUNDR(stream,1);
      return TAG_READ;
    //--- Flight timer --------------
    case 'lnft':
      ReadUNDR(stream,2);
      return TAG_READ;
    //--- elapse timer --------------
    case 'lnet':
      ReadUNDR(stream,3);
      return TAG_READ;
    //--- copy from -----------------
    case 'copy':
      CopyFrom(stream);
      return TAG_READ;
  }
  return CGauge::Read (stream, tag);
}
//-----------------------------------------------------------------------------
//  All parameters are read
//-----------------------------------------------------------------------------
void CHawkClockGauge::ReadFinished (void)
{ CGauge::ReadFinished ();
	hold	= globals->pit->GetHolder(unId);
	SaveValue();
  return;
}
//-----------------------------------------------------------------------------
//  Save gauge value in holder
//-----------------------------------------------------------------------------
void CHawkClockGauge::SaveValue()
{ U_INT val = (btup << 16) | (btsl << 8) | (btst);
	hold->pm1 = val;
	return;
}
//-----------------------------------------------------------------------------
//  Load gauge value from holder
//-----------------------------------------------------------------------------
void CHawkClockGauge::LoadValue()
{ U_INT val = hold->pm1;
	btst			= val & 0xFF;
	btsl			= (val >> 8)	& 0xFF;
	btup			= (val >> 16)	& 0xFF;
	return;
}

//-----------------------------------------------------------------------------
//	Request intg data from doat
//-----------------------------------------------------------------------------
void CHawkClockGauge::IntgFromDoat(U_INT tag)
{	mesg.id				  = MSG_GETDATA;
	mesg.user.u.datatag	= tag;
	mesg.intData		= 0;
	Send_Message(&mesg);
	char *nbr = (char*)mesg.voidData;
  if (nbr) strncpy(edNb,nbr,4);
	if (edNb[1] ==  '0') edNb[1] = ' ';
	return;
}
//-----------------------------------------------------------------------------
//	Copy gauge from similar
//-----------------------------------------------------------------------------
void CHawkClockGauge::CopyFrom(SStream *stream)
{ CHawkClockGauge &src = *(CHawkClockGauge*)LocateGauge(stream);
  CGauge::CopyFrom(src);
  SetSurface(dsw,dsh);
  //--- Copy local parameters -------------------
  bupp.CopyFrom(src.bupp,this);
  bsel.CopyFrom(src.bsel,this);
  bcon.CopyFrom(src.bcon,this);
  //---------------------------------------------
  cFont = src.cFont;
  wCar  = src.wCar;               // Character width
  hCar  = src.hCar;               // Character height
  tLn   = src.tLn;                // Time line
  tCn   = src.tCn;                // Time column
  col   = src.col;                // color
  //---------------------------------------------
  txd.Dupplicate(src.txd);              // bitmap
  und[0].CopyFrom(src.und[0]);
  und[1].CopyFrom(src.und[1]);
  und[2].CopyFrom(src.und[2]);
  und[3].CopyFrom(src.und[3]);
  return;
}
//----------------------------------------------------------------------------
// Draw  doat
//	 
//------------------------------------------------------------------------------
void CHawkClockGauge::Draw (void)
{ cFont->SetTransparent();
	if (0 == surf)  return;
	LoadValue();
  switch (btup) {
  case 0:
    IntgFromDoat('VOLT');
    edNb[4] = 'E';
    break;

  case 1:
    IntgFromDoat('TMPF'); 
    edNb[4] = 'F';
    break;

  case 2:
    IntgFromDoat('TMPC'); 
    edNb[4] = 'C';
    break;
  }
  //---------------------------------------------------------
  EraseSurfaceRGBA(surf,0);
  DrawTextureOn(txd,*surf,0,0,0);
  cFont->DrawNText(surf,(2 * wCar), 1, col, edNb);
  switch  (btsl)  {
    case 0:
        mesg.id				      = MSG_GETDATA;
        mesg.user.u.datatag	= 'uTim';
        Send_Message(&mesg);
        und[0].Draw(surf,col);
        break;
    case 1:
        mesg.id				      = MSG_GETDATA;
        mesg.user.u.datatag	= 'lTim';
        Send_Message(&mesg);
        und[1].Draw(surf,col);
        break;
    case 2:
        mesg.id				      = MSG_GETDATA;
        mesg.user.u.datatag	= 'fTim';
        Send_Message(&mesg);				      // Get flight timer value
        und[2].Draw(surf,col);
        break;
    case 3:
        mesg.id				      = MSG_GETDATA;
        mesg.user.u.datatag	= 'eTim';
        Send_Message(&mesg);			      // Get elapse Time
        und[3].Draw(surf,col);
        break;

  }
  cFont->DrawNText(surf, tCn, tLn, col, mesg.charPtrData);
  RenderSurface(quad,surf);
  return;
}
//----------------------------------------------------------------------------------
//  Display VOLT
//----------------------------------------------------------------------------------
void CHawkClockGauge::DisplayUPR()
{ edNb[4] = 0;
  switch (btup) {
    case 0:
      sprintf_s(hbuf,HELP_SIZE,"Volt: %s",  edNb);
      break;
    case 1:
      sprintf_s(hbuf,HELP_SIZE,"Temp: %s°F",edNb);
      break;
    case 2:
      sprintf_s(hbuf,HELP_SIZE,"Temp: %s°C",edNb);
      break;
    default:
      return;
  }
  FuiHelp();
  return;
}
//----------------------------------------------------------------------------------
//  Mouse moves over
//----------------------------------------------------------------------------------
ECursorResult CHawkClockGauge::MouseMoved (int x, int y)
{ ECursorResult rc = CURSOR_WAS_CHANGED;
  if (bupp.MouseMoved (x, y)) { DisplayUPR(); return CURSOR_WAS_CHANGED;}
  // Send updated mouse position to all click areas
  if (bsel.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  if (bcon.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  if (bbot.MouseMoved (x, y)) return CURSOR_WAS_CHANGED;
  // Display help
  DisplayHelp();
  return CURSOR_WAS_CHANGED;
}
//----------------------------------------------------------------------------------
//  Mouse click over DOAT
//---------------------------------------------------------------------------------
EClickResult CHawkClockGauge::MouseClick (int mouseX, int mouseY, int buttons)
{
  EClickResult rc = MOUSE_TRACKING_OFF;
  // Only handle left/right mouse clicks
  if ((buttons & (MOUSE_BUTTON_LEFT | MOUSE_BUTTON_RIGHT)) == 0) {
    return rc;
  }

  if  (bupp.IsHit (mouseX, mouseY)) 
  { btup++; 
    if (btup == 3) btup = 0;
		SaveValue();
    return rc;
  }
  if (bsel.IsHit (mouseX, mouseY)) 
  { btsl++; 
    if (btsl == 4) btsl = 0;
		SaveValue();
    return rc;
  }
  if (bcon.IsHit (mouseX, mouseY)) 
  { mesg.id				= MSG_SETDATA;
		mesg.user.u.datatag	= 'eTrz';			// Toggle elapse timer
		Send_Message(&mesg);						    // Call Doat to toggle
		SaveValue();
    return rc;
  }
  if (bbot.IsHit (mouseX, mouseY)) 
  { return rc;
  }
  return rc;
}
//======================================================================
// CBasicDigitalClockGauge
//======================================================================
CBasicDigitalClockGauge::CBasicDigitalClockGauge (CPanel *mp)
:CHawkClockGauge(mp)
{ tick_x = tick_y = 0;
  tkut = tklt = tkft = tket = 0;
}

int CBasicDigitalClockGauge::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'tick':
    ReadInt (&tick_x, stream);
    ReadInt (&tick_y, stream);
    rc = TAG_READ;
    break;

  case 'tkut':
    ReadInt (&tkut, stream);
    rc = TAG_READ;
    break;

  case 'tklt':
    ReadInt (&tklt, stream);
    rc = TAG_READ;
    break;

  case 'tkft':
    ReadInt (&tkft, stream);
    rc = TAG_READ;
    break;

  case 'tket':
    ReadInt (&tket, stream);
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    rc = CHawkClockGauge::Read (stream, tag);
  }

  return rc;
}
//======================================================================
// CNavGpsSwitchT0:  Switch NAV/GPS a la Hawk
//======================================================================
CNavGpsSwitchT0::CNavGpsSwitchT0 (CPanel *mp)
: C_SimpleInOutStateSwitch(mp)
{	usas = 0;
}
//------------------------------------------------------------------
//	Read All parameters through Simple Switch
//------------------------------------------------------------------
void CNavGpsSwitchT0::ReadFinished ()
{ C_SimpleInOutStateSwitch::ReadFinished();
	switch (mesg.user.u.datatag) {
	//-- used for track mode ---------
	case 'navp':
		usas = 'trak';
		return;
	//--- used for autoland --------
	case 'aprp':
		usas = 'land';
		return;
	}
	return; 
}
//-----------------------------------------------------------------------------
//  Draw the gauge
//-----------------------------------------------------------------------------
void CNavGpsSwitchT0::DrawAmbient()
{	//---- Update state -----------------------
	int val	= 0;
	if ('trak' == usas) val = subS->GaugeBusIN04();
	if ('land' == usas) val = subS->GaugeBusIN05();
	Draw(val);
  return;
  }
//============ END OF FILE ==========================================================
