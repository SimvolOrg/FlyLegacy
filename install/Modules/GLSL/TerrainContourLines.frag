/*
 * TerrainContourLines.frag
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
// GLSL fragment shader displays elevation contour lines over terrain
//

// Uniform variables determine contour line colour, interval and span
uniform vec4	ContourColour;
uniform float	ContourInterval;		// In feet
uniform float	ContourSpan;			  // Thickness of contour line

// Varying variable indicates elevation
varying float elevation;          // Feet MSL
// varying float fog;

void main (void)
{
  vec4 colour = vec4(0.0);

  // Get modulus of vertex elevation, normalized to +/- half of ContourInterval
  float m = mod(elevation, ContourInterval);

  if (m <= ContourSpan) {
    // Blend contour line colour with interpolated fragment colour
    float blend = 1.0 - ((abs(m-(ContourSpan/2.0)) / (ContourSpan/2.0)));
    blend = clamp (blend, 0.0, 1.0);
    colour = mix(gl_Color, ContourColour, blend);
  } else {
    // No change to this fragment colour
    colour = gl_Color;
  }
  gl_FragColor = colour;

  // Mix fragment colour with fog
//  float fog = exp (-gl_Fog.density * gl_FogFragCoord);
//  fog = clamp (fog, 0.0, 1.0);
//  gl_FragColor = mix(gl_Fog.color, colour, fog);
//  gl_FragColor = vec4 (gl_FogFragCoord, 0, 0, 1.0);
}
