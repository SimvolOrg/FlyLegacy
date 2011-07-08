/*
 * Messaging.cpp
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
 */

/*! \file Messaging.cpp
 *  \brief Implements Fly! subsystem messaging API
 *
 * Messaging Subsystem
 *
 * Messaging between objects is a critical part of internal systems modelling.
 *   Typical applications include:
 *     - setting an attribute of a subsystem
 *     - retrieving the state or indication value of a subsystem from a gauge object
 *     - informing subsystems of actions
 */

#include "../Include/FlyLegacy.h"
#include "../Include/Globals.h"
#include "../Include/Situation.h"
#include "../Include/Subsystems.h"
#include "../Include/UserVehicles.h"
#include "../Include/Autopilot.h"
#include <set>

using namespace std;
//--------------------------------------------------------------------------
//	Build observer list
//  Use this for trap purpose 
//--------------------------------------------------------------------------
void	AddToObservers(SMessage *msg)
{	return;	}

//--------------------------------------------------------------------------
//	Dispatch message to observers
//--------------------------------------------------------------------------
inline void	SendMsgToObservers(SMessage *msg)
{	  return;	}
//--------------------------------------------------------------------------
//	JSDEV*  Modified Send_Message
//	1)  When a message is not processed after polling all the vehicle component
//		it is stamped with the NullSubsystem as a receiver.  
//
//	2)	The MsgForMe(..) call is factorized at this level.  Thus we can safely
//		remove this call from all components. 
//
//	3)	Message.Result is not normally needed. When a subsystem receives a
//      message, either it responds with a value or the default value
//      is left inchanged
//----------------------------------------------------------------------------
EMessageResult Send_Message (SMessage *msg)
{ // Assume message has no handler by default
	msg->result	      = MSG_IGNORED;
	EMessageResult rc = MSG_IGNORED;
  //---When message contains a receiver, use it for direct contact ---
  //---------------------------------------------------------------
  //  To trap those messages here with the 2 following lines
  //  Those messages are in the WARNINGLOG file
  //---------------------------------------------------------------
  //  if (msg->group == 'vsi_')             // Change group here
  //  int a = 0;                            // Put a break point here
  //---------------------------------------------------------------
	if (msg->receiver) 
	{// Send the message to the known recipient
		CSubsystem *subs  = (CSubsystem *)msg->receiver;
		rc = subs->ReceiveMessage (msg);
		return rc;
	}
  //---------------------------------------------------------------
  //  To trap those messages here with the 2 following lines
  //  Those messages are in the WARNINGLOG file
  //---------------------------------------------------------------
  //if (msg->group == 'rcm1')             // Change group here
  // int a = 0;                           // Put a break point here
  //---------------------------------------------------------------

  //---Send message to the user vehicle ---------------------------	
	CVehicleObject *user = globals->sit->GetUserVehicle ();
  if (user) rc = user->ReceiveMessage(msg);
  TagToString(msg->dst,msg->group);
	if (msg->receiver)    return rc;
  //--- Send message to the CheckList -----------------------------
  rc = globals->chk->ReceiveMessage(msg);
  if (msg->receiver)    return rc;
  //--- Warning has not been generated for this message yet--
  msg->receiver	= user->GetNullSubsystem();			// Set Null Sub as receiver
  //---Ignore when group is null -----------------------------------
  if (0 == msg->group)    return rc;
	char sendr_id[8];
	char group_id[8];
	char dtag__id[8];
	TagToString (sendr_id, msg->sender);
	TagToString (group_id, msg->group);
	TagToString (dtag__id, msg->user.u.datatag);
	WARNINGLOG ("Send_Message:(NO RECEIVER)  sender=%s destination=%s dtag=%s",sendr_id,group_id,dtag__id);
  return rc;
}
//=========================END OF FILE ================================================================