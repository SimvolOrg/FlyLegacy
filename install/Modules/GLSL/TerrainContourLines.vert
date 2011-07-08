/*
 * TerrainContourLines.vert
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2007 Chris Wallace
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

//
// GLSL vertex shader displays elevation contour lines over terrain
//

// Uniform variables determine contour line colour, interval and span
uniform vec4	ContourColour;
uniform float	ContourInterval;		// In feet
uniform float	ContourSpan;		// Thickness of contour line in feet

// Varying variable indicates elevation
varying float elevation;                  // Feet MSL
// varying float fog;                        // Fog value

void DirectionalLight (in int i,
                       in vec3 normal,
                       inout vec4 ambient,
                       inout vec4 diffuse)
{
  float nDotVP;  // normal . light direction
  nDotVP = max (0.0, dot(normal, normalize(vec3(gl_LightSource[i].position))));
  ambient += gl_LightSource[i].ambient;
  diffuse += gl_LightSource[i].diffuse * nDotVP;
}

void main (void)
{
  // Elevation is simply the z-coordinate of the vertex
  elevation = gl_Vertex.z;

  // Emulate fixed functional transforms
  gl_Position = ftransform();
  vec3 normal = normalize(gl_NormalMatrix * gl_Normal);

  // Apply default lighting
  vec4 ambient = vec4(0.0);
  vec4 diffuse = vec4(0.0);
  DirectionalLight (0, normal, ambient, diffuse);
  gl_FrontColor = gl_FrontLightModelProduct.sceneColor +
                  ambient * gl_FrontMaterial.ambient +
                  diffuse * gl_FrontMaterial.diffuse;

  // Apply fog
//  gl_FogFragCoord = gl_FogCoord;
//  fog = exp (-gl_Fog.density * gl_FogFragCoord);
//  fog = clamp (fog, 0.0, 1.0);
}
