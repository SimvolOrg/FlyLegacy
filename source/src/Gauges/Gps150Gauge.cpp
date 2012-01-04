/*
 * Gps150Gauge.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2007 Chris Wallace
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

/**
 * @file Gps150Gauge.cpp
 * @brief Implements Garmin GPS150 gauge object
 */

#include "../Include/Gauges.h"
#include "../Include/Globals.h"

CGarminGPS150Gauge::CGarminGPS150Gauge (void)
: CBitmapGauge(0)
{
  // Initialize gauge data
  brightness = 10;

  // Load GPS150 custom font
  strncpy (font.fontName, "FTG150PT7.FNT",63);
  LoadFont (&font);
}

int	CGarminGPS150Gauge::Read (SStream *stream, Tag tag)
{
	int	rc = TAG_IGNORED;

	switch (tag) {
	//case 'chlp':
	//	if (ca != -1) {
	//		// Click area help
	//		SClickArea *pCa = pDisplay->GetClickArea(ca);
	//		APIReadString (pCa->help, 80, stream);
	//		WriteLog ("  <help> = %s\n", pCa->help);
	//		rc = TAG_READ;
	//	}
	//	break;
	//case 'ghlp':
	//	// Default help for gauge
	//	APIReadString (pData->help, 80, stream);
	//	WriteLog ("<ghlp> = %s\n", pData->help);
	//	rc = TAG_READ;
	//	break;
	case 'ca01':
		// Click Area 1 : Power/Brightness
    ReadFrom (&ca01, stream);
		rc = TAG_READ;
		break;
	case 'ca02':
		// Click Area 2 : D->TO
    ReadFrom (&ca02, stream);
		rc = TAG_READ;
		break;
	case 'ca03':
		// Click Area 3 : NRST LITE
    ReadFrom (&ca03, stream);
		rc = TAG_READ;
		break;
	case 'ca04':
		// Click Area 4 : NRST
    ReadFrom (&ca04, stream);
		rc = TAG_READ;
		break;
	case 'ca05':
		// Click Area 5 : SET LITE
    ReadFrom (&ca05, stream);
		rc = TAG_READ;
		break;
	case 'ca06':
		// Click Area 6 : SET
    ReadFrom (&ca06, stream);
		rc = TAG_READ;
		break;
	case 'ca07':
		// Click Area 7 : RTE LITE
    ReadFrom (&ca07, stream);
		rc = TAG_READ;
		break;
	case 'ca08':
		// Click Area 8 : RTE
    ReadFrom (&ca08, stream);
		rc = TAG_READ;
		break;
	case 'ca09':
		// Click Area 9 : WPT LITE
    ReadFrom (&ca09, stream);
		rc = TAG_READ;
		break;
	case 'ca10':
		// Click Area 10 : WPT
    ReadFrom (&ca10, stream);
		rc = TAG_READ;
		break;
	case 'ca11':
		// Click Area 11 : NAV LITE
    ReadFrom (&ca11, stream);
		rc = TAG_READ;
		break;
	case 'ca12':
		// Click Area 12 : NAV
    ReadFrom (&ca12, stream);
		rc = TAG_READ;
		break;
	case 'ca13':
		// Click Area 13 : CRSR
    ReadFrom (&ca13, stream);
		rc = TAG_READ;
		break;
	case 'ca14':
		// Click Area 14 : MSG LITE
    ReadFrom (&ca14, stream);
		rc = TAG_READ;
		break;
	case 'ca15':
		// Click Area 15 : STAT
    ReadFrom (&ca15, stream);
		rc = TAG_READ;
		break;
	case 'ca16':
		// Click Area 16 : CLR
    ReadFrom (&ca16, stream);
		rc = TAG_READ;
		break;
	case 'ca17':
		// Click Area 17 : ENT
    ReadFrom (&ca17, stream);
		rc = TAG_READ;
		break;
	case 'ca18':
		// Click Area 18 : Outer Knob
    ReadFrom (&ca18, stream);
		rc = TAG_READ;
		break;
	case 'ca19':
		// Click Area 19 : Inner Knob
    ReadFrom (&ca19, stream);
		rc = TAG_READ;
		break;
	case 'ca20':
		// Click Area 20 : Keyboard lock
    ReadFrom (&ca20, stream);
		rc = TAG_READ;
		break;
	default:
		rc = TAG_IGNORED;
	}

	return rc;
}

ECursorResult CGarminGPS150Gauge::MouseMoved (int x, int y)
{
  ECursorResult rc = CURSOR_NOT_CHANGED;

  // Send updated mouse position to all click areas
  rc = ca01.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca02.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca03.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca04.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca05.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca06.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca07.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca08.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca09.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca10.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca11.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca12.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca13.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca14.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca15.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca16.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca17.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca18.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca19.MouseMoved (x, y);
  if (rc == CURSOR_NOT_CHANGED) rc = ca20.MouseMoved (x, y);

  // Call parent method if cursor not changed
  if (rc == CURSOR_NOT_CHANGED) {
    rc = CBitmapGauge::MouseMoved (x, y);
  }

  return rc;
}

void CGarminGPS150Gauge::ClickAction (EG150Action action)
{
	// Take action depending on which click area was hit
	SMessage *pMsg = &msg;
	switch (action) {
	case ACTION_PWR_LEFT:
		// Power/brightness decrease
		if (pwrOn) {
			// Decrement brightness
			brightness--;
			if (brightness <= 0) {
				// PWR Off
				pwrOn = false;
				msg.id = MSG_SETDATA;
				msg.user.u.datatag = 'pwr_';
				Send_Message (&msg);
			}
		}
		break;
	case ACTION_PWR_RIGHT:
		if (!pwrOn) {
			// Send the PWR trigger to the subsystem
			pwrOn = true;
			brightness = 10;
			msg.id = MSG_SETDATA;
			msg.user.u.datatag = 'pwr_';
			Send_Message (&msg);
		} else {
			// Power is already on, increase brightness
			brightness ++;
			if (brightness > 10) {
				brightness = 10;
			}
		}
		break;
	case ACTION_DTO:
		// DirectTo
		msg.id = MSG_SETDATA;
		msg.dataType = TYPE_INT;
		msg.user.u.datatag = 'd_to';
		Send_Message (&msg);
		break;
	case ACTION_NRST:
		// Nearest
		msg.id = MSG_SETDATA;
		msg.dataType = TYPE_INT;
		msg.user.u.datatag = 'nrst';
		Send_Message (&msg);
		break;
	case ACTION_SET:
		// Set
		msg.id = MSG_SETDATA;
		msg.dataType = TYPE_INT;
		msg.user.u.datatag = 'set_';
		Send_Message (&msg);
		break;
	case ACTION_RTE:
		// Route
		msg.id = MSG_SETDATA;
		msg.dataType = TYPE_INT;
		msg.user.u.datatag = 'rte_';
		Send_Message (&msg);
		break;
	case ACTION_WPT:
		// Waypoint
		msg.id = MSG_SETDATA;
		msg.dataType = TYPE_INT;
		msg.user.u.datatag = 'wpt_';
		Send_Message (&msg);
		break;
	case ACTION_NAV:
		// Nav
		msg.id = MSG_SETDATA;
		msg.dataType = TYPE_INT;
		msg.user.u.datatag = 'nav_';
		Send_Message (&msg);
		break;
	case ACTION_CRSR:
		// Cursor
		msg.id = MSG_SETDATA;
		msg.dataType = TYPE_INT;
		msg.user.u.datatag = 'crsr';
		Send_Message (&msg);
		break;
	case ACTION_STAT:
		// Stat
		msg.id = MSG_SETDATA;
		msg.dataType = TYPE_INT;
		msg.user.u.datatag = 'stat';
		Send_Message (&msg);
		break;
	case ACTION_CLR:
		// Clear
		msg.id = MSG_SETDATA;
		msg.dataType = TYPE_INT;
		msg.user.u.datatag = 'clr_';
		Send_Message (&msg);
		break;
	case ACTION_ENT:
		// Enter
		msg.id = MSG_SETDATA;
		msg.dataType = TYPE_INT;
		msg.user.u.datatag = 'ent_';
		Send_Message (&msg);
		break;
	case ACTION_OUTER_LEFT:
		// Outer knob left
		msg.id = MSG_SETDATA;
		msg.dataType = TYPE_INT;
		msg.user.u.datatag = 'outl';
		Send_Message (&msg);
		break;
	case ACTION_OUTER_RIGHT:
		// Outer knob right
		msg.id = MSG_SETDATA;
		msg.dataType = TYPE_INT;
		msg.user.u.datatag = 'outr';
		Send_Message (&msg);
		break;
	case ACTION_INNER_LEFT:
		// Inner knob left
		msg.id = MSG_SETDATA;
		msg.dataType = TYPE_INT;
		msg.user.u.datatag = 'innl';
		Send_Message (&msg);
		break;
	case ACTION_INNER_RIGHT:
		msg.id = MSG_SETDATA;
		msg.dataType = TYPE_INT;
		msg.user.u.datatag = 'innr';
		Send_Message (&msg);
		break;
	case ACTION_KEYLOCK:
		msg.id = MSG_SETDATA;
		msg.dataType = TYPE_INT;
		msg.user.u.datatag = 'klck';
		Send_Message (&msg);
		break;
	}
}

EClickResult CGarminGPS150Gauge::MouseClick (int x, int y, int buttons)
{
	// Check click areas, store current hit area and reset auto-
	//   repeat elapsed counter.  This is initialized to -0.5
	//   second so that there is a delay before auto-repeat
	//   kicks in
	EG150Action action = ACTION_NONE;

	// Translate click area and mouse button to GPS action
  if (ca01.IsHit(x, y)) {
    // PWR click area
		if (buttons & MOUSE_BUTTON_LEFT) {
			action = ACTION_PWR_LEFT;
		} else if (buttons & MOUSE_BUTTON_RIGHT) {
			action = ACTION_PWR_RIGHT;
		}
  } else if (ca02.IsHit (x, y)) {
    // D->TO click area
		action = ACTION_DTO;
  } else if (ca04.IsHit (x, y)) {
    // NRST click area
		action = ACTION_NRST;
  } else if (ca06.IsHit (x, y)) {
    // SET click area
		action = ACTION_SET;
  } else if (ca08.IsHit (x, y)) {
    // RTE click area
		action = ACTION_RTE;
  } else if (ca10.IsHit (x, y)) {
    // WPT click area
		action = ACTION_WPT;
  } else if (ca12.IsHit (x, y)) {
    // NAV click area
		action = ACTION_NAV;
  } else if (ca13.IsHit (x, y)) {
    // CRSR click area
		action = ACTION_CRSR;
  } else if (ca15.IsHit (x, y)) {
    // STAT click area
		action = ACTION_STAT;
  } else if (ca16.IsHit (x, y)) {
    // CLR click area
		action = ACTION_CLR;
  } else if (ca17.IsHit (x, y)) {
    // ENT click area
		action = ACTION_ENT;
  } else if (ca18.IsHit (x, y)) {
    // Outer knob click area
		if (buttons & MOUSE_BUTTON_LEFT) {
			action = ACTION_OUTER_LEFT;
		} else if (buttons & MOUSE_BUTTON_RIGHT) {
			action = ACTION_OUTER_RIGHT;
		}
  } else if (ca19.IsHit (x, y)) {
    // Inner knob click area
		if (buttons & MOUSE_BUTTON_LEFT) {
			action = ACTION_INNER_LEFT;
		} else if (buttons & MOUSE_BUTTON_RIGHT) {
			action = ACTION_INNER_RIGHT;
		}
  } else if (ca20.IsHit (x, y)) {
    // Keylock click area
		action = ACTION_KEYLOCK;
	}

	// Perform appropriate click area action
  if (caAction != ACTION_NONE) {
    caAction = action;
    caClickStart = globals->tim->GetElapsedRealTime();
    ClickAction (action);
  }

	return MOUSE_TRACKING_ON;
}

EClickResult CGarminGPS150Gauge::TrackClick (int mouseX, int mouseY, int buttons)
{
	// Increment elapsed click time
  float elapsed = globals->tim->GetElapsedSimTime();
  float delta =  elapsed - caClickStart;
	if (delta >= 1.0f) {
		ClickAction (caAction);
		caClickStart = elapsed;
	}
	return MOUSE_TRACKING_ON;
}

EClickResult CGarminGPS150Gauge::StopClick()
{
	// Clear stored click action
	caAction = ACTION_NONE;
	caClickStart = 0;
  return MOUSE_TRACKING_OFF;
}

void CGarminGPS150Gauge::Draw (void)
{
	// Get shared data from subsystem
	msg.id = MSG_GETDATA;
	msg.group = 'g150';
	msg.dataType = TYPE_VOID;
	msg.user.u.datatag = 'shar';
  msg.voidData = 0;
	Send_Message (&msg);

	// Interpret voidData result as pointer to shared data structure
  SGps150SharedData *pShared = NULL;
	if (msg.voidData)  pShared = (SGps150SharedData *)msg.voidData;

	// Verify that shared data is valid before continuing
}
