/*
 * DialogMsgDebug.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
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
 *
 * This dialog implements a messaging debug interface.  It allows the user
 *   to construct a message and send it through the normal messaging API.
 *   The dialog is then updated with the return message contents.
 *
 */


#include "../Include/FlyLegacy.h"
#include "../Include/Ui.h"
#include "../Include/Utility.h"
#include "../Include/Globals.h"
#include "../Include/Fui.h"
#include "../Include/FuiParts.h"
#include <plib/puAux.h>
#pragma warning(disable : 4201)
char *idlist[] =
  {
    "SETDATA",
    "GETDATA",
    NULL
  };

char *typelist[] =
  {
    "REAL",
    "INT",
    NULL
  };


typedef struct {
  puDialogBox*  dialog;
  puFrame*    frame;
  puText*     title;
  puaComboBox*   id;
  puInput*    group;
  puInput*    datatag;
  puaComboBox*   datatype;
  puInput*    intData;
  puInput*    realData;
  puInput*    result;
  puOneShot*    send;
  puOneShot*    close;

  char      strGroup[80];
  char      strDatatag[80];
  int       valInt;
  float     valFloat;
} SMsgDebugDialogData;

SMessage    msg;

static SMsgDebugDialogData *dt = NULL;


static void msg_debug_sync (void)
{
  // Sync data type
  switch (msg.dataType) {
  case TYPE_REAL:
    dt->datatype->setCurrentItem (0);
    break;

  case TYPE_INT:
    dt->datatype->setCurrentItem (1);
    break;

  default:
    // Do nothing
    ;
  }

  // Sync data
  dt->intData->setValue (msg.intData);
  dt->realData->setValue ((float)msg.realData);

  // Sync result
  switch (msg.result) {
  case MSG_IGNORED:
    dt->result->setValue ("IGNORED");
    break;

  case MSG_PROCESSED:
    dt->result->setValue ("PROCESSED");
    break;

  case MSG_USED:
    dt->result->setValue ("USED");
    break;
  }

  puDisplay ();
}

static void msg_debug_id_cb (puObject* obj)
{
  int i = ((puaComboBox*)obj)->getCurrentItem ();
  switch (i) {
  case 0:
    msg.id = MSG_SETDATA;
    break;

  case 1:
    msg.id = MSG_GETDATA;
    break;
  }
}

static void msg_debug_datatype_cb (puObject* obj)
{
  int i = ((puaComboBox*)obj)->getCurrentItem ();
  switch (i) {
  case 0:
    msg.dataType = TYPE_REAL;
    break;

  case 1:
    msg.dataType = TYPE_INT;
    break;
  }
}

static void msg_debug_send_cb (puObject* obj)
{
  if (dt == NULL) return;

  // Send message.  'receiver' field is cleared to prevent the message from being
  //   sent to the previous message's receiver
  msg.group = StringToTag (dt->strGroup);
  msg.user.u.datatag = StringToTag (dt->strDatatag);
  switch (msg.dataType) {
  case TYPE_INT:
    msg.intData = dt->valInt;
    break;

  case TYPE_REAL:
    msg.realData = dt->valFloat;
    break;

  default:
    // Do nothing
    ;
  }
  msg.receiver = 0;
  Send_Message (&msg);

  // Synchronize fields with updated message contents
  msg_debug_sync ();
}

static void msg_debug_close_cb (puObject* obj)
{
  if (dt == NULL) return;

  // Deleting the PUI dialog will delete all child widgets as well
  delete dt->dialog;

  // Delete dialog data structure
  delete dt;
  dt = NULL;
}

void msg_debug_dlg_create (void)
{
  if (dt != NULL) return;

  // Initialize message

  // Instantiate data structure for dialog contents
  dt = new SMsgDebugDialogData;
 *dt->strGroup		= 0;
 *dt->strDatatag = 0;
  dt->valInt = 0;
  dt->valFloat = 0.0f;

  int frame_w = 250;
  int frame_h = 300;

  // Create dialog box
  dt->dialog = new puDialogBox (20, globals->cScreen->Height - 25 - frame_h);
  {
    // Frame
    dt->frame = new puFrame (0, 0, frame_w, frame_h);

    dt->title = new puText (10, 270);
    dt->title->setLabel ("Messaging Debug:");

    // id select box
    dt->id = new puaComboBox (100, 230, 240, 250, idlist);
    dt->id->setLabel ("id     :");
    dt->id->setLabelPlace (PUPLACE_CENTERED_LEFT);
    dt->id->setStyle (PUSTYLE_BOXED);
    dt->id->setCallback (msg_debug_id_cb);
    dt->id->setCurrentItem (0);

    // group input field
    dt->group = new puInput (100, 200, 240, 220);
    dt->group->setLabel ("group    :");
    dt->group->setLabelPlace (PUPLACE_CENTERED_LEFT);
    dt->group->setStyle (PUSTYLE_BOXED);
    dt->group->setValuator (dt->strGroup);

    // datatag input field
    dt->datatag = new puInput (100, 170, 240, 190);
    dt->datatag->setLabel ("datatag  :");
    dt->datatag->setLabelPlace (PUPLACE_CENTERED_LEFT);
    dt->datatag->setStyle (PUSTYLE_BOXED);
    dt->datatag->setValuator (dt->strDatatag);

    // datatype select box
    dt->datatype = new puaComboBox (100, 140, 240, 160, typelist);
    dt->datatype->setLabel ("datatype:");
    dt->datatype->setLabelPlace (PUPLACE_CENTERED_LEFT);
    dt->datatype->setStyle (PUSTYLE_BOXED);
    dt->datatype->setCallback (msg_debug_datatype_cb);
    dt->datatype->setCurrentItem (0);

    // intData input field
    dt->intData = new puInput (100, 110, 240, 130);
    dt->intData->setLabel ("intData  :");
    dt->intData->setLabelPlace (PUPLACE_CENTERED_LEFT);
    dt->intData->setStyle (PUSTYLE_BOXED);
    dt->intData->setValuator (&dt->valInt);

    // realData input field
    dt->realData = new puInput (100, 80, 240, 100);
    dt->realData->setLabel ("realData :");
    dt->realData->setLabelPlace (PUPLACE_CENTERED_LEFT);
    dt->realData->setStyle (PUSTYLE_BOXED);
    dt->realData->setValuator (&dt->valFloat);

    // result text field
    dt->result = new puInput (100, 50, 240, 70);
    dt->result->setLabel ("result  :");
    dt->result->setLabelPlace (PUPLACE_CENTERED_LEFT);
    dt->result->setStyle (PUSTYLE_BOXED);
    dt->result->rejectInput ();

    // Send button
    dt->send = new puOneShot (120, 20, 170, 40);
    dt->send->setLegend ("Send");
    dt->send->makeReturnDefault (false);
    dt->send->setStyle (PUSTYLE_SMALL_SHADED);
    dt->send->setCallback (msg_debug_send_cb);

    // Close dialog button
    dt->close = new puOneShot (190, 20, 240, 40);
    dt->close->setLegend ("Close");
    dt->close->makeReturnDefault (false);
    dt->close->setStyle (PUSTYLE_SMALL_SHADED);
    dt->close->setCallback (msg_debug_close_cb);

    // Synchronize dialog fields with message contents
    msg_debug_sync ();
  }
  dt->dialog->close ();
  dt->dialog->reveal ();
}


//=========================END OF FILE =========================================================================
