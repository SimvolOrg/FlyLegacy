/*
 * SkyLight.cpp
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

#include "../Include/Sky.h"
#include "../Include/Utility.h"
#include "../Include/Fui.h"

//
// Constructor
//
CSkyLight::CSkyLight (void)
{
}

//
// Initialize lighting tables
//
void CSkyLight::Init (void)
{
}

//----------------------------------------------------------------------
// Update sky lighting attributes
//  TODO: Light power must be attenuated by Fog
//  This concerns ambiant, diffuse and specular contribution
//  Temporarily a single scale value is used
//---------------------------------------------------------------------
void CSkyLight::Update (float solTheta, float solPhi, float moonTheta, float moonPhi)
{
  GLfloat ambient[4]  = {0.20f, 0.20f, 0.20f, 1.0f};
  GLfloat diffuse[4]  = {1.0f, 1.0f, 1.0f, 1.0f};
  GLfloat specular[4] = {0.0f, 0.0f, 0.0f, 1.0f};

  // Initialize scene ambient light level
  GLfloat sceneAmbient[4] = {0.10f, 0.10f, 0.10f, 1.0f};

  GLfloat solPosition[4];
  solPosition[0] = sin(solPhi) * sin(solTheta);
  solPosition[1] = cos(solPhi) * sin(solTheta);
  solPosition[2] = cos(solTheta);
  // Clamp altitude to the horizon
//  if (solPosition[2] < 0) solPosition[2] = 0;
  solPosition[3] = 0.0f;

  GLfloat moonPosition[4];
  moonPosition[0] = sin(moonPhi) * sin(moonTheta);
  moonPosition[1] = cos(moonPhi) * sin(moonTheta);
  moonPosition[2] = cos(moonTheta);
  // Clamp altitude to the horizon
//  if (moonPosition[2] < 0) moonPosition[2] = 0;
  moonPosition[3] = 0.0f;
  glDisable (GL_LIGHT1);

  // Check for sun below civil twilight
  if (solTheta > DegToRad (108.0f)) {
    // Sun is below level of civil twilight
    ambient[0]  = ambient[1]  = ambient[2]  = 0.0f;     
    diffuse[0]  = diffuse[1]  = diffuse[2]  = 0.0f;     
    specular[0] = specular[1] = specular[2] = 0.0f;     
  } else 
  {
    // Sun is higher than civil night
    // Adjust light intensity based on altitude
    if (solTheta < DegToRad (85.0)) {
      // Sun is above the horizon; use full-intensity
      ambient[0]  = ambient[1]  = ambient[2]  = 0.20f;  
      diffuse[0]  = diffuse[1]  = diffuse[2]  = 1.0f;   
      specular[0] = specular[1] = specular[2] = 1.0f;   
    } else 
    { // Linearly scale intensity between full darkness at -18deg and full intensity
      //   at the horizon
      float dTheta = (1.0 - ((RadToDeg(solTheta) - 85.0f) / 23.0f));
      ambient[0]  = ambient[1]  = ambient[2]  = dTheta * 0.2f;  
      diffuse[0]  = diffuse[1]  = diffuse[2]  = dTheta * 1.0f;  
      specular[0] = specular[1] = specular[2] = dTheta * 1.0f;  
    }
  }

  // Set parameters for light 0 (Sun)
  glEnable (GL_LIGHT0);
  glLightfv (GL_LIGHT0, GL_POSITION, solPosition);
  glLightfv (GL_LIGHT0, GL_AMBIENT,  ambient);
  glLightfv (GL_LIGHT0, GL_DIFFUSE,  diffuse);
  glLightfv (GL_LIGHT0, GL_SPECULAR, specular);

  // @todo Set parameters for light 1 (Moon)
/*    // Set moon lighting if necessary
    if (moonTheta >= 0) {
      // Adjust ambient light
      float moonAmbient = 0.10f * cos(moonTheta);  // * illumination^2
      ambient[0] += moonAmbient;
      ambient[1] += moonAmbient;
      ambient[2] += moonAmbient;

      // Set lunar light values
      glLightfv (GL_LIGHT1, GL_AMBIENT, ambient);
      glLightfv (GL_LIGHT1, GL_DIFFUSE, diffuse);
      glLightfv (GL_LIGHT1, GL_SPECULAR, specular);
      glLightfv (GL_LIGHT1, GL_POSITION, moonPosition);
      glEnable (GL_LIGHT1);
    } else {
      // Moon is below level of civil twilight
      glDisable (GL_LIGHT1);
    }
*/

  // Enable overall GL lighting and set scene ambient level
  glLightModelfv (GL_LIGHT_MODEL_AMBIENT, sceneAmbient);
}
