/*
 * AeroModelDraw.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright (c) 2004 Chris Wallace
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

/*! \file AeroModelDraw.h
 *  \brief Defines classes used in the aerodynamic model.
 */
#ifndef AERO_MODELDRAW_H_
#define AERO_MODELDRAW_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

///////// DEBUG STUFF TO REMOVE LATER ////////////////////////////////
//#ifdef  _DEBUG
  #define _DEBUG_SCREEN_AERO   //print lc DDEBUG file ... remove later
//#endif
//////////////////////////////////////////////////////////////////////
 
#ifdef _DEBUG_SCREEN_AERO
#include "../Include/uglyfont.h"
#endif

void DebugScreenAeroTxt (const char *txt = NULL, const GLfloat &x = 0.0f, const GLfloat &y = 0.0f, const GLfloat &color = 255.0f)
{
#ifdef _DEBUG_SCREEN_AERO
  Tag cam_type = globals->cam->GetCameraType ();
  if (cam_type == CAMERA_COCKPIT || cam_type == CAMERA_FLYBY || cam_type == CAMERA_TOWER) return;
  glColor3f (color, 0.0f, 0.0f);
  glPushMatrix ();
  glTranslatef   (   x,    y, 0.0f);  // glTranslatef(x,y,0);
  glScalef       (0.5f, 0.5f, 1.0f);  // glScalef (fontWidth, -fontHeigth, 1.0);
  if (cam_type != CAMERA_OVERHEAD) {
    CVector ori;
    float upy = 0.0f;
    if (cam_type == CAMERA_ORBIT) {
      (globals->cam)->GetOrientation (ori);
      //float orb = PI - (TWO_PI + ori.z); 
      ori.z = -PI - ori.z;
      upy = sin (ori.x);
      gluLookAt (0.0, 0.0, 0.0, 0.0, -HALF_PI, 0.0, -sin (ori.z), 0.0, cos (ori.z));
    } else {
      if (cam_type == CAMERA_SPOT) {
        (globals->cam)->GetOrientation (ori);
        ori.z += PI;
      } else 
      if (cam_type == CAMERA_OBSERVER) {
        (globals->cam)->GetOrientation (ori);
        ori.z += PI - (globals->iang).z;
      }
      gluLookAt (0.0, 0.0, 0.0, 0.0, -HALF_PI, 0.0, -sin (ori.z), 0.0, cos (ori.z));
    }
  }
  YsDrawUglyFont (txt, 1);
  glPopMatrix ();
#endif // _DEBUG_SCREEN_AERO
}

void DebugScreenAero (const SSurface *sf, const char *txt = NULL)
{
  // Set projection matrix to 2D screen size
  //glMatrixMode(GL_PROJECTION);
  //glPushMatrix();
  //glLoadIdentity();
  //gluOrtho2D (0, globals->cScreen->Width, globals->pCurScreen->Height, 0);

  // Clear modelview matrix
  //glMatrixMode(GL_MODELVIEW);
  //glPushMatrix();
  //glLoadIdentity();

  // Set up OpenGL drawing state
  //glPushAttrib (GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_COLOR_BUFFER_BIT | GL_POLYGON_BIT);
  //glDisable (GL_DEPTH_TEST);
  //glDisable (GL_COLOR_MATERIAL);
  //glDisable(GL_TEXTURE_2D);

  //glColor3f (225.0f, 225.0f, 0.0f);
  //glutWireSphere (100.0, 5, 5);
  //glutWireCube (30.0);

  glColor3f (250.0f, 0.0f, 0.0f);
  CVector ori = globals->iang; // RH

  glBegin    (GL_LINES);
  glVertex2i (-50,     0        ); // x
  glVertex2i ( 50,     0        ); // x
  glVertex2i (  0,   -50        ); // y
  glVertex2i (  0,     0        ); // y
  glVertex3i (  0,     0, -50   ); // up
  glVertex3i (  0,     0,  50   ); // up
  glColor3f  (0.0f, 0.0f, 250.0f);
  glVertex2i (  0,    50        ); // y NORTH
  glVertex2i (  0,     0        ); // y
  glColor3f  (0.0f, 250.0f, 0.0f);

  CRotationMatrixHPB mat; // LH
  VectorOrientLeftToRight (ori);
  CVector child1 (0.0,  50.0, 0.0), parent1;
  CVector child2 (0.0, -50.0, 0.0), parent2;

  mat.Setup (ori);
  mat.ChildToParent (parent1, child1); 
  mat.ChildToParent (parent2, child2);
  glVertex3f (parent1.x, parent1.z, parent1.y);
  glVertex3f (parent2.x, parent2.z, parent2.y);
  glEnd();

  // Restore original OpenGL state
  //glPopAttrib();

  // Restore original modelview matrix
  //glMatrixMode(GL_MODELVIEW);
  //glPopMatrix();

  // Restore original projection matrix
  //glMatrixMode(GL_PROJECTION);
  //glPopMatrix();

  // Check for an OpenGL error
	/*
  GLenum e = glGetError ();
  if (e != GL_NO_ERROR) {
    WARNINGLOG ("DebugScreenAero - GL Error 0x%04X", e);
  }
	*/
}

void DebugSingleScreenAeroWF (const CVector &ori_,
                              const double  &xx, const double  &yy, const double  &zz,
                              const GLfloat &rc, const GLfloat &gc, const GLfloat &bc,
                              const double  &dz = 1.0)
{
  glColor3f (rc, gc, bc);
  CVector ori = ori_;

  CRotationMatrixHPB mat; // LH
  VectorOrientLeftToRight (ori);
  CVector child1 ( xx,  yy,  zz     ), parent1;
  CVector child2 (-xx, -yy, -zz * dz), parent2;

  mat.Setup (ori);
  mat.ChildToParent (parent1, child1); 
  mat.ChildToParent (parent2, child2);

  glBegin    (GL_LINES);
  glVertex3f (parent1.x, parent1.z, parent1.y);
  glVertex3f (parent2.x, parent2.z, parent2.y);
  glEnd();

}

void DebugSingleVScreenAeroWF (const CVector &ori_,
                              const double  &xx, const double  &yy, const double  &zz,
                              const GLfloat &rc, const GLfloat &gc, const GLfloat &bc)
{
  glColor3f (rc, gc, bc);
  CVector ori = ori_;

  CRotationMatrixHPB mat; // LH
  VectorOrientLeftToRight (ori);
  CVector child1 ( xx,  yy,  zz), parent1;

  mat.Setup (ori);
  mat.ChildToParent (parent1, child1); 

  glBegin    (GL_LINES);
  glVertex3f (parent1.x, parent1.z, parent1.y); // RH
  glVertex3f (0.0f     , 0.0f     , 0.0f     ); // RH
  glEnd();

}

void DebugScreenAeroWFNewCG (const double &lenght)
{
  CVector ori = globals->iang; // RH
  const CVector *cg = globals->sit->uVeh->svh->GetNewCG_ISU (); // LH

  DebugSingleVScreenAeroWF (ori, cg->x * lenght, cg->y * lenght, cg->z * lenght, 0.0f, 225.0f, 0.0f);

  // Check for an OpenGL error
	/*
  GLenum e = glGetError ();
  if (e != GL_NO_ERROR) {
    WARNINGLOG ("DebugScreenAero - GL Error 0x%04X", e);
  }
	*/
}

void DebugScreenAeroWFModel (const double &lenght)
{
  CVector ori = globals->iang;                                        // RH

  DebugSingleScreenAeroWF (ori, 0.0, 0.0, lenght, 255.0f, 125.0f, 0.0f, 2.0);
  DebugSingleScreenAeroWF (ori, lenght, 0.0, 0.0, 255.0f, 125.0f, 0.0f     );

  // Check for an OpenGL error
	/*
  GLenum e = glGetError ();
  if (e != GL_NO_ERROR) {
    WARNINGLOG ("DebugScreenAero - GL Error 0x%04X", e);
  }
	*/

}

void DebugScreenAeroWSForce (const SVector &pos, const SVector &force)
{ CVehicleObject *veh = globals->sit->uVeh;
  (fabs (force.y) > fabs (force.x)) ? glColor3f (225.0f, 0.0f, 0.0f) : glColor3f (0.0f, 0.0f, 225.0f);
  CVector ori = veh->GetOrientation (); // RH

  double _K_  = veh->GetMassInKgs () / 4.0;

  CRotationMatrixHPB mat; // LH
  VectorOrientLeftToRight (ori);
  CVector child1 (MetresToFeet (pos.x),
                  MetresToFeet (pos.y),
                  MetresToFeet (pos.z)), parent1;
  CVector child2 (MetresToFeet (pos.x) + (force.x / _K_),
                  MetresToFeet (pos.y) + (force.y / _K_),
                  MetresToFeet (pos.z) + (force.z / _K_)), parent2;

  mat.Setup (ori);
  mat.ChildToParent (parent1, child1); 
  mat.ChildToParent (parent2, child2);

  glBegin    (GL_LINES);
  glVertex3f (parent1.x, parent1.z, parent1.y);
  glVertex3f (parent2.x, parent2.z, parent2.y);
  glEnd();
	/*
  GLenum e = glGetError ();
  if (e != GL_NO_ERROR) {
    WARNINGLOG ("DebugScreenAeroWS - GL Error 0x%04X", e);
  }
	*/

}

void DebugScreenAeroWSDrag (const SVector &pos, const SVector &force)
{ CVehicleObject *veh = globals->sit->uVeh;
  glColor3f (0.0f, 255.0f, 0.0f);
  CVector ori = veh->GetOrientation (); // RH

  double _K_  = veh->GetMassInKgs () / 16.0;

  CRotationMatrixHPB mat; // LH
  VectorOrientLeftToRight (ori);
  CVector child1 (MetresToFeet (pos.x),
                  MetresToFeet (pos.y),
                  MetresToFeet (pos.z)), parent1;
  CVector child2 (MetresToFeet (pos.x) + (force.x / _K_),
                  MetresToFeet (pos.y) + (force.y / _K_),
                  MetresToFeet (pos.z) + (force.z / _K_)), parent2;

  mat.Setup (ori);
  mat.ChildToParent (parent1, child1); 
  mat.ChildToParent (parent2, child2);

  glBegin    (GL_LINES);
  glVertex3f (parent1.x, parent1.z, parent1.y);
  glVertex3f (parent2.x, parent2.z, parent2.y);
  glEnd();
	/*
  GLenum e = glGetError ();
  if (e != GL_NO_ERROR) {
    WARNINGLOG ("DebugScreenAeroWS - GL Error 0x%04X", e);
  }
	*/

}

void DebugScreenAeroWFEnginesForce (const double &lenght = 1.0)
{ CVehicleObject *veh = globals->sit->uVeh;
  const SVector *eng_pos = &veh->eng->GetEnginesPosISU (); // LH
  CVector  eng_fce =  veh->eng->GetForceISU (); // LH
  eng_fce.Times (0.5);

  DebugScreenAeroWSForce (*eng_pos, eng_fce);

  // Check for an OpenGL error
	/*
  GLenum e = glGetError ();
  if (e != GL_NO_ERROR) {
    WARNINGLOG ("DebugScreenAero - GL Error 0x%04X", e);
  }
	*/

}
/*
void DebugScreenAeroWFWingsMoment (const SVector &val, const double &lenght = 1.0)
{
  const CVector *cg = globals->sit->user->svh->GetNewCG_ISU (); // LH

  DebugScreenAeroWSForce (*cg, val);

  // Check for an OpenGL error
  GLenum e = glGetError ();
  if (e != GL_NO_ERROR) {
    WARNINGLOG ("DebugScreenAero - GL Error 0x%04X", e);
  }

}
*/
//////////////////////////////////////////////////////////////////////
#endif // AERO_MODEL_H_
