/*
 * PitotStaticSubsystems.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
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

#include "../Include/Subsystems.h"
#include "../Include/Globals.h"

using namespace std;

//==============================================================================
//
// CPitotStaticPort
//==============================================================================
CPitotStaticPort::CPitotStaticPort (CVehicleObject *v)
{ mveh  = v;            // Save parent vehicle
  // Initialize
  type = PORT_STATIC;
  bPos.x = 0;   bPos.y = 0;   bPos.z = 0;
  grup = 1;
  face = PORT_LEFT;
  inie = false;
  wont = false;
  stat = 1;

  val = 0.0;
}

int CPitotStaticPort::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;
//
//#ifdef _DEBUG	
//{	FILE *fp_debug;
//	if(!(fp_debug = fopen("__DDEBUG_.txt", "a")) == NULL)
//	{
//		int test = 0;
//		fprintf(fp_debug, "CPitotStaticPort::Read %d\n", test);
//		fclose(fp_debug); 
//}	}
//#endif

  switch (tag) {
  case 'type':
    {
      // Type of port, PITOT or STATIC
      char sType[64];
      ReadString (sType, 64, stream);
      if (stricmp (sType, "PITOT") == 0) {
        type = PORT_PITOT;
      } else if (stricmp (sType, "STATIC") == 0) {
        type = PORT_STATIC;
      }
      rc = TAG_READ;
    }
    break;

  case 'bPos':
    {
      // Port position relative to aircraft external model center
      ReadVector (&bPos, stream); // LH
      // LegacyCoordinates (bPos);  // 
      rc = TAG_READ;
    }
    break;

  case 'grup':
    {
      // Group number
      ReadInt (&grup, stream);
      rc = TAG_READ;
    }
    break;

  case 'face':
    {
      // Which side of aircraft the port is on, LEFT or RIGHT
      char sFace[64];
      ReadString (sFace, 64, stream);
      if (stricmp (sFace, "LEFT") == 0) {
        face = PORT_LEFT;
      } else if (stricmp (sFace, "RIGHT") == 0) {
        face = PORT_RIGHT;
      }
      rc = TAG_READ;
    }
    break;

  case 'inie':
    // No arguments, if this tag is present then the port is interior
    inie = true;
    rc = TAG_READ;
    break;

  case 'stat':
    {
      // Group number
      ReadInt (&stat, stream);
      rc = TAG_READ;
    }
    break;

  case 'wont':
    // No arguments, if this tag is present then the port will not ice
    wont = true;
    rc = TAG_READ;
    break;
  }

  // If tag has not been processed, pass it to the parent
  if (rc != TAG_READ) {
    // See if the tag can be processed by the parent class type
    rc = CDependent::Read (stream, tag);
  }

  return rc;
}

EMessageResult  CPitotStaticPort::ReceiveMessage (SMessage *msg)
{
    //
    //#ifdef _DEBUG	
    //{	FILE *fp_debug;
	   // if(!(fp_debug = fopen("__DDEBUG_.txt", "a")) == NULL)
	   // {
		  //  int test = 1;
    //    fprintf(fp_debug, "CPitotStaticPort::ReceiveMessage %s\n", this->GetClassName ());
		  //  fclose(fp_debug); 
    //}	}
    //#endif

    EMessageResult rc = MSG_IGNORED;

    switch (msg->id) {
    case MSG_GETDATA:
      switch (msg->user.u.datatag) {
        case 1:
        default :
          break;
      }
    case MSG_SETDATA:
      switch (msg->user.u.datatag) {
        case 1:
        default :
          break;
      }
    }

    if (rc == MSG_IGNORED) {
      // See if the message can be processed by the parent class
      rc = CDependent::ReceiveMessage (msg);
    }

  return rc;}

void CPitotStaticPort::TimeSlice (float dT,U_INT FrNo)
{
//
//#ifdef _DEBUG	
//{	FILE *fp_debug;
//	if(!(fp_debug = fopen("__DDEBUG_.txt", "a")) == NULL)
//	{
//		int test = 1;
//    fprintf(fp_debug, "CPitotStaticPort::TimeSlice %s\n", this->GetClassName ());
//		fclose(fp_debug); 
//}	}
//#endif
  // CDependent::TimeSlice (dT);
}

const double& CPitotStaticPort::ComputePitotPort (const double &rho, const double &pr, const SVector &vt)
{
  val = 0.0;
  // verify if the port is enabled
  if (!state) return val;
  // only PORT_PITOT // todo STATIC later
  if (type == PORT_STATIC) return val;
  //
  SVector relV;                                                               ///< relative air speed for obj f/s
  SVector omegaV = {0.0, 0.0, 0.0};                                           ///< additional speed due to rotation f/s
  const SVector *omega = mveh->GetBodyAngularVelocityVector (); ///< rad/s LH

  // Luc's comment : Need to check the cross product operand order : (*omega x bPos) or (bPos x *omega)) ?
  VectorCrossProduct(omegaV, *omega, bPos);					                          ///< Luc's comment : VectorCrossProduct() is independant of LH/RH orientation
  
  relV = VectorSum(vt, omegaV);

  // The pitot tube sees the forward
  // velocity in the Z body LH axis.
  double ad_Speed2 = relV.z * relV.z;	
  double q = 0.5 * rho * ad_Speed2;
  val = pr + q;
  //
  //#ifdef _DEBUG	
  //{	FILE *fp_debug;
	 // if(!(fp_debug = fopen("__DDEBUG_.txt", "a")) == NULL)
	 // {
		//  int test = 1;
  //    fprintf(fp_debug, "CPitotStaticPort::TimeSlice %s pr=%f q=%f val=%f\n", this->GetClassName (),
  //      pr, q, val);
		//  fclose(fp_debug); 
  //}	}
  //#endif
  //
  /// tmp : had to divide by 2 with twin engines
  /// \todo : develop a proper algoritm later
  val /= mveh->GetEngNb();
  return (val); ///< pfs
}