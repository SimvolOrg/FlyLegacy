/*
 * FrameManager.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2004 Chris Wallace
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

/*! \file FrameManager.cpp
 *  \brief still to be done
 */

/*
 * Frame Manager
 *
 */

#include "../Include/FrameManager.h"
#include "../Include/TimeManager.h"
#include "../Include/Globals.h"

///////// DEBUG STUFF TO REMOVE LATER ////////////////////////////////
#ifdef  _DEBUG
  //#define _DEBUG_FRAME_MANAGER            //print lc DDEBUG file ... remove later
#endif
//////////////////////////////////////////////////////////////////////



//
// CInertialFrame methods
//
//
CInertialFrame::CInertialFrame (void)
{
  ;
}

CInertialFrame::~CInertialFrame (void)
{
  ;
}

void CInertialFrame::Init (void) {
  ifpos_.x = ifpos_.y = ifpos_.z = 0.0;
  total_t  = 0.0f;
  aE       = 0.0;
}

void CInertialFrame::UpdateEarthRotation (void) {
  //get the time since simulation started
  total_t = globals->tim->GetDeltaSimTime ();
 //
  #ifdef _DEBUG_FRAME_MANAGER	
  {	FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_frames.txt", "a")) == NULL)
	  {
		  fprintf(fp_debug, "total_t=%.2f ** ", total_t);
		  fclose(fp_debug); 
  }	}
  #endif
  // angle the earth has rotated
  aE = total_t * EARTH_SIDERAL_ROTATION_RATE_SI;
}

void CInertialFrame::GetEarthRotationSinCos (void) {
  UpdateEarthRotation ();
  sinCos(sin_aE, cos_aE, aE);
}

void CInertialFrame::SetInitPos (void) {
  // call this only once when the situation
  // has been loaded and the simulation starts
  // Get position in SI units
  SPosition tmp_geop = globals->geop;
  // Get the distance from the center of earth
  double r_alt = FeetToMetres (tmp_geop.alt) + EARTH_RADIUS_SI;
  //
  #ifdef _DEBUG_FRAME_MANAGER	
  {	FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_frames.txt", "a")) == NULL)
	  {
		  fprintf(fp_debug, "lat=%.2f lon=%.2f alt=%.2f alt=%.2f\n", 
        tmp_geop.lat,
        tmp_geop.lon,
        tmp_geop.alt,
        r_alt
        );
		  fclose(fp_debug); 
  }	}
  #endif

  // convert lat, lon to radians
  double latRad = arcseconds_lat2radians (tmp_geop.lat);
  double lonRad = arcseconds_lon2radians (tmp_geop.lon);
  //
  #ifdef _DEBUG_FRAME_MANAGER	
  {	FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_frames.txt", "a")) == NULL)
	  {
		  fprintf(fp_debug, "latRad %.2f lonRad %.2f\n", 
        latRad,
        lonRad
        );
		  fclose(fp_debug); 
  }	}
  #endif

  // Transform from polar to cartesian coordinates. 
  // Since this is the start of the simulation 
  // the earth has not rotated yet, so the ipos 
  // and epos coordinate systems are aligned perfectly. 
  // This allow us to skip the rotation step from epos to ipos 
  // and assign ipos directly 
  ifpos_.z = r_alt * sin (latRad); 
  ifpos_.x = r_alt * cos (latRad) * cos (lonRad); 
  ifpos_.y = r_alt * cos (latRad) * sin (lonRad); 
  //
  #ifdef _DEBUG_FRAME_MANAGER	
  {	FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_frames.txt", "a")) == NULL)
	  {
		  fprintf(fp_debug, "ifpos_.x %.2f ifpos_.y %.2f ifpos_.z %.2f\n", 
        ifpos_.x,
        ifpos_.y,
        ifpos_.z
        );
		  fclose(fp_debug); 
  }	}
  #endif
  // TODO - initialize aircraft orientation  
}

void CInertialFrame::geop2ipos (const SPosition &pos, CVector &ifpos_) {
 
  // Get the distance from the center of earth
  double r_alt = FeetToMetres (pos.alt) + EARTH_RADIUS_SI;
  // convert lat, lon to radians
  double latRad = arcseconds_lat2radians (pos.lat);
  double lonRad = arcseconds_lon2radians (pos.lon);

  // Transform from cartesian to polar coordinates. 
  // the earth has not rotated , so the ipos 
  // and epos coordinate systems are aligned perfectly. 
  // This allow us to skip the rotation step from ipos  
  // to epos and assign epos directly
  ifpos_.z = r_alt * sin (latRad); 
  ifpos_.x = r_alt * cos (latRad) * cos (lonRad); 
  ifpos_.y = r_alt * cos (latRad) * sin (lonRad); 
  //
}

//
// CLocalFrame methods
//
//
CLocalFrame::CLocalFrame (void)
{
  ;
}

CLocalFrame::~CLocalFrame (void)
{
  ;
}

void CLocalFrame::Init (void) {
  lpos_.x = 0.0;
  lpos_.y = 0.0;
  lpos_.z = 0.0;
}

void CLocalFrame::ipos2geop (const CVector &ipos, SPosition &geop) {

  // update earth rotation = aE
  if_.GetEarthRotationSinCos ();

  CVector epos; // position in eath-fixed cartesian coordinates
  
  // calculate epos - rotation aE radians about z-axis
  epos.x =  ipos.x * if_.GetCosAE () + ipos.y * if_.GetSinAE ();
  epos.y = -ipos.x * if_.GetSinAE () + ipos.y * if_.GetCosAE ();
  epos.z =  ipos.z;
  //
  #ifdef _DEBUG_FRAME_MANAGER	
  {	FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_frames.txt", "a")) == NULL)
	  {
		  fprintf(fp_debug, "ipos.x %.2f ipos.y %.2f ipos.z %.2f epos.x %.2f epos.y %.2f epos.z %.2f\n", 
        ipos.x, ipos.y, ipos.z,
        epos.x, epos.y, epos.z
        );
		  fclose(fp_debug); 
  }	}
  #endif
  // calculate altitude
  // the distance r from the center of the earth is the length
  // of the position vector. ipos is beter because it's the master
  // pos so we don't introduce any rounding error from epos
  // NB: We are ignoring the fact that the earth is not a perfect 
  // sphere for now - This is hard enough as it is already. 
  double r = SquareRootFloat (ipos.x * ipos.x + ipos.y * ipos.y + ipos.z * ipos.z);
  geop.alt = MetresToFeet (r - EARTH_RADIUS_SI); // altitude in metres-->feet

  // calculate latitude and longitude
  // = transformation in polar coordinates
  // latitude is the "up" angle from equator
  // longitude is the angle east from 0N0E
  geop.lat = 0.0;
  geop.lon = 0.0;
  if (r) {
    double latRad = safeAsin (ipos.z / r); // lat in rad 
    //double latRad = atan2 (ipos.z, r); // lat in rad
    double lonRad = atan2 (epos.y, epos.x); // lon in rad, east is positive
    //
    #ifdef _DEBUG_FRAME_MANAGER	
    {	FILE *fp_debug;
	    if(!(fp_debug = fopen("__DDEBUG_frames.txt", "a")) == NULL)
	    {
		    fprintf(fp_debug, "geop.x %.2f geop.y %.2f geop.z %.2f\n", 
          latRad,
          lonRad,
          geop.alt
          );
		    fclose(fp_debug); 
    }	}
    #endif
    // convert to arcsec
    // Even thinks we'd use meters above MSL
    geop.lat = radians2arcseconds_lat (latRad);
    geop.lon = radians2arcseconds_lon (lonRad);
  }
}

void CLocalFrame::euler2iang (const CVector &eulerAngle, CVector &tmp_iang) {
  // TODO - Transform orientation to local coordinates, in iang 
  // this is a rotation - I'll work out the matrix for you, give 
  // me a couple of days
  /* To be done - just a try below
  double tetha  = eulerAngle.p;
  double phi    = eulerAngle.h;
  double psi    = eulerAngle.r;
  //
  double sa     = sin(psi); double sb = sin(phi); double sc = sin(tetha);
  double ca     = cos(psi); double cb = cos(phi); double cc = cos(tetha);
  //
  double m11 =  cc*cb;            double m12 =  cc*sb;            double m13 = -sc;
  double m21 =  sa*sc*cb-ca*sb;   double m22 =  sa*sc*sb+ca*cb;   double m23 =  sa*cc;
  double m31 =  ca*sc*cb+sa*sb;   double m32 =  ca*sc*sb-sa*cb;   double m33 =  ca*cc;
  //
  tmp_iang.x    = 0.0;
  tmp_iang.y    = 0.0;
  tmp_iang.z    = 0.0;
  */
}

void CLocalFrame::euler2iang (const CVector &eulerAngle, const SPosition &pos, CVector &tmp_iang) {
  // TODO - Transform orientation to local coordinates, in iang 
  // this is a rotation  
  //
  // 1) transform local position to inertial position
  double rad_pos_x = arcseconds_lat2radians (pos.lat); // rad
  double rad_pos_y = arcseconds_lat2radians (pos.lon); // rad
  double tmp_pos_z = EQUATORIAL_RADIUS_FEET + pos.alt; // ft
  double inert_pos_x = tmp_pos_z * cos (rad_pos_x) * cos (rad_pos_y); // ft
  double inert_pos_y = tmp_pos_z * cos (rad_pos_x) * sin (rad_pos_y); // ft
  double inert_pos_z = tmp_pos_z * sin (rad_pos_x);                   // ft
  //
  // 2) transform local orientation to inertial orientation
  double rot_angle_x = HALF_PI - rad_pos_x;
  double rot_angle_y = HALF_PI + rad_pos_y;
  double sin_half_pos_x = sin (rot_angle_x / 2);
  double sin_half_pos_y = sin (rot_angle_y / 2);
  double cos_half_pos_x = cos (rot_angle_x / 2);
  double cos_half_pos_y = cos (rot_angle_y / 2);
  //
  double sin_half_rot_p = sin (eulerAngle.x / 2);
  double sin_half_rot_r = sin (eulerAngle.y / 2); // RH
  double sin_half_rot_h = sin (eulerAngle.z / 2); // RH
  //double sin_half_rot_r = sin (eulerAngle.z / 2); // LH
  //double sin_half_rot_h = sin (eulerAngle.y / 2); // LH
  double cos_half_rot_p = cos (eulerAngle.x / 2);
  double cos_half_rot_r = cos (eulerAngle.y / 2); // RH
  double cos_half_rot_h = cos (eulerAngle.z / 2); // RH
  //double cos_half_rot_r = cos (eulerAngle.z / 2); // LH
  //double cos_half_rot_h = cos (eulerAngle.y / 2); // LH

    #ifdef _DEBUG_FRAME_MANAGER	
    {	FILE *fp_debug;
	    if(!(fp_debug = fopen("__DDEBUG_AERO_frames.txt", "a")) == NULL)
	    {
		    fprintf(fp_debug, "%f %f pos(%.2f %.2f %.2f %.2f) rot(%.2f %.2f %.2f %.2f %.2f %.2f)\n",
          rot_angle_x,  rot_angle_y,
          sin_half_pos_x, sin_half_pos_y, cos_half_pos_x, cos_half_pos_y,
          sin_half_rot_p, sin_half_rot_r, sin_half_rot_h, cos_half_rot_p, cos_half_rot_r, cos_half_rot_h
         );
		    fclose(fp_debug); 
    }	}
    #endif

  CQuaternion lon     (cos_half_pos_y,              0, 0, sin_half_pos_y),
              lat     (cos_half_pos_x, sin_half_pos_x, 0,              0),
              heading (cos_half_rot_h,              0, 0, sin_half_rot_h),
              pitch   (cos_half_rot_p, sin_half_rot_p, 0,              0),
              banking (cos_half_rot_r,              0, sin_half_rot_r, 0);

  CQuaternion pb, hpb, lahpb, all;
  pb.QuaternionProduct    (&pitch, &banking);
  hpb.QuaternionProduct   (&heading, &pb);
  lahpb.QuaternionProduct (&lat, &hpb);
  all.QuaternionProduct   (&lon, &lahpb);
    #ifdef _DEBUG_FRAME_MANAGER	
    {	FILE *fp_debug;
	    if(!(fp_debug = fopen("__DDEBUG_AERO_frames.txt", "a")) == NULL)
	    {
		    fprintf(fp_debug, "pb(%.2f %.2f %.2f %.2f) hpb(%.2f %.2f %.2f %.2f) lahpb(%.2f %.2f %.2f %.2f) all(%.2f %.2f %.2f %.2f)\n",
          pb.w, pb.v.x, pb.v.y, pb.v.z,
          hpb.w, hpb.v.x, hpb.v.y, hpb.v.z,
          lahpb.w, lahpb.v.x, lahpb.v.y, lahpb.v.z,
          all.w, all.v.x, all.v.y, all.v.z
          );
		    fclose(fp_debug); 
    }	}
    #endif

  CRotMatrix rot;
  //rot.Setup (
  //  arcseconds_lon2radians (pos.lon),
  //  arcseconds_lat2radians (pos.lat)
  //);
  rot.QuaternionToRotMat (all);
    #ifdef _DEBUG_FRAME_MANAGER	
    {	FILE *fp_debug;
	    if(!(fp_debug = fopen("__DDEBUG_AERO_frames.txt", "a")) == NULL)
	    {
		    fprintf(fp_debug, " %.5f %.5f %.5f\n %.5f %.5f %.5f\n %.5f %.5f %.5f\n",
          rot.m11, rot.m12, rot.m13,
          rot.m21, rot.m22, rot.m23,
          rot.m31, rot.m32, rot.m33
          );
		    fclose(fp_debug); 
    }	}
    #endif
  //
  tmp_iang.p = safeAsin /*asin*/  (rot.m23);
  //tmp_iang.r = safeAtan2 (-rot.m13, rot.m33); // LH
  //tmp_iang.h = safeAtan2 (-rot.m21, rot.m22); // LH
  tmp_iang.h = safeAtan2 (-rot.m13, rot.m33); // RH
  tmp_iang.r = safeAtan2 (-rot.m21, rot.m22); // RH

  //rot.TransformItoB (eulerAngle, tmp_iang);

    #ifdef _DEBUG_FRAME_MANAGER	
    {	FILE *fp_debug;
	    if(!(fp_debug = fopen("__DDEBUG_AERO_frames.txt", "a")) == NULL)
	    {
		    fprintf(fp_debug, "euler2iang == lon=%f lat=%f euler(%.2f %.2f %.2f %.2f %.2f %.2f) iang(%.2f %.2f %.2f)\n",
          arcseconds_lon2radians (pos.lon),
          arcseconds_lat2radians (pos.lat),
          eulerAngle.x, eulerAngle.y, eulerAngle.z, eulerAngle.p, eulerAngle.h, eulerAngle.r,
          tmp_iang.x, tmp_iang.y, tmp_iang.z
         );
		    fprintf(fp_debug, "======================================================================================\n");
		    fclose(fp_debug); 
    }	}
    #endif
}

//
// CBodyFrame methods
//
//
CBodyFrame::CBodyFrame (void)
{
  ;
}

CBodyFrame::~CBodyFrame (void)
{
  ;
}

void CBodyFrame::Init (void) {

  bpos_.x = 0.0;
  bpos_.y = 0.0;
  bpos_.z = 0.0;
}
