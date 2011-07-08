/*
 * Sky.h
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
 */

/*! \file Sky.h
 *  \brief Defines CSkyManager and related classes for rendering of sky
 *
 * These classes implement the visual model for realistic sky, including:
 *  - colour-blended sky dome
 *  - sun image
 *  - textured moon image
 *  - stars and planets with realistic magnitudes and colours
 *
 * Future:
 *  - clouds
 *  - sun and moon halo effects
 *
 * The design and implementation of these classes is based heavily on the
 *   sky model of SimGear 0.3.3, by Curtis Olson.  Integration into the
 *   Fly! Legacy project necessitated a rewrite of the actual classes,
 *   but I am indebteded to Curt and other contributors to the SimGear and
 *   FlightGear teams for their prior work in this area.
 *
 */

#ifndef SKY_H
#define SKY_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "FlyLegacy.h"
#include "Ephemeris.h"
#include "Ini.h"
#include "Utility.h"
#include <string>
#include <vector>
#include <map>


//
// CSkyLight class represents aspects of the sun and moon positions as they
//   relate to sky and terrain lighting effects.  This class is derived from
//   the FlightGear class fgLIGHT by Curt Olsen
//
class CSkyLight {
public:
  // Constructor
  CSkyLight (void);

  // CSkyLight methods
  void Init (void);
  void Update (float solTheta, float solPhi, float moonTheta, float moonPhi);

public:
  /*
   * Sun parameters
   */

  // Geocentric latitude/longitude
    double sun_gc_lon, sun_gc_lat;

  // Cartesian coordinates
  sgVec3  sunpos;

  // Sun angle, the angle between the sun and local horizontal (radians)
    double sun_angle;

  // Sun rotation, the angle between the sun's azimuth angle and due south
    double sun_rotation;

  /*
   * Moon parameters...TBD
   */

  
  /*
   * Derived lighting values
   */

  // Fog colour
  GLfloat fog_colour[4];

  // Clear sky colour
  GLfloat sky_colour[4];
};


//
// CSkyDomeImage class implements the visual model for the background sky colour.
//   The dome consists of a central vertex at the zenith and three concentric rings
//   of vertices progressively down towards the horizon.  Applications can specify
//   independent colours for the zenith, and for any azimuth point on the concentric
//   rings.
//

typedef struct {
  float A, B, C, D, E;
} SPerezCoefficients;

typedef struct {
  float A[2], B[2], C[2], D[2], E[2];
} SCoefficientParameters;

typedef struct {
  float   A, B, C, D;
} SLuminanceZenith;

typedef struct {
  float   t2[4];
  float   t1[4];
  float   t0[4];
} SChromaZenith;

typedef struct {
  char          name[64];
  SCoefficientParameters  Y_curve;
  SCoefficientParameters  x_curve;
  SCoefficientParameters  y_curve;
  SLuminanceZenith        Y_zenith;
  SChromaZenith           x_zenith;
  SChromaZenith           y_zenith;
} SPerezParameters;

#define SKYDOME_SLICES            (64)
#define SKYDOME_STACKS            (32)

class CSkyDomeImage {
public:
  CSkyDomeImage (double distance);
  ~CSkyDomeImage (void);

  // CSkyDome methods
  void      Repaint (double solTheta, double solPhi);
  void      GetDomeColour (float theta, float phi, float &r, float &g, float &b);
  void      Draw (void);
  void      Print (FILE *f);

protected:
  void      DrawDome (void);

protected:
//  void      InitPerezParameters (SPerezParameters* p,
//                     CIniFile *inisky,
//                     const char* name);
protected:
  // Precalculated theta/phi angles for each vertex
  sgVec2    vtx_ring_angles[SKYDOME_STACKS+1][SKYDOME_SLICES];

  // Vertex arrays
  int       nDomeVertices;
  GLfloat   *domeVertex;
  GLfloat   *domeColour;
  int       nFanElements;
  GLuint    *fanElements;
  int       nDomeElements;
  GLuint    *domeElements;

  // Parameters for sky dome shading with Perez functions
  bool                  night;      ///< Sun was below astronomical twilight at last repaint
  double                prevTheta;  ///< Solar zenith angle at last repaint
  double                prevPhi;    ///< Solar azimuth angle at last repaint

  SPerezParameters                        *perezCurrent;  // Current Perez parameter set
  std::map<std::string,SPerezParameters*> perez;          // Map of all Perez parameter sets

  // OpenGL rendering parameters
  float       distance; ///< Apparent distance from eyepoint
  float       rotY;     ///< Dome rotation around y-axis
};


//
// The following enum represents the various phases of darkness.
//
typedef enum {
  PHASE_INVALID     = -1,
  PHASE_DEEP_NIGHT    = 0,
  PHASE_NIGHT       = 1,
  PHASE_LATE_DUSK     = 2,
  PHASE_DUSK        = 3,
  PHASE_EARLY_DUSK    = 4,
  PHASE_LATE_TWILIGHT   = 5,
  PHASE_EARLY_TWILIGHT  = 6,
  PHASE_DAYLIGHT      = 7
} EDarkPhase;


typedef struct {
  char    name[64];   ///< Common name (optional)
  double  ra, dec;    ///< Celestial coordinates
  float   mv;         ///< Visual magnitude
  float   bv;         ///< B-V colour index
  int     hr;         ///< HR number, used to construct constellation figures
  float   x, y, z;    ///< Cartesian coordinates of star vertex
  float   r, g, b, a; ///< Star colour/transparency values
} SStarData;

typedef struct {
  char  name[64];     // Constellation name (abbreviation)
  int   star1, star2; // Line endpoints, indices into stars vector
} SConstellationLine;


//
// CStarImages class implements the visual model for all star-like objects
//   (including stars and planets) in the sky.  These are rendered as
//   single points (or small clusters of points for very bright objects)
//   with accurate colours.
//
class CStarImages {
public:
  CStarImages (double distance);
  ~CStarImages (void);

  // CStarImages methods
  void      Repaint (float limit, float factor);
  void      Reposition (sgVec3 pos, double lon, double lat, double lst);
  void      Draw (void);

protected:
  void      LoadFlyDatabase (const char* dbFilename);
  void      LoadBSCDatabase (const char* bscFilename);
  void      LoadConstellations (const char* txtFilename);
  int       FindStarByHR (int hr);
  void      DrawStarsWithShader (void);
  void      DrawStarsNoShader (void);
  void      DrawStars (void);
  void      DrawConstellations (void);

protected:
  std::vector<SStarData*>          stars;          ///< All potentially visible stars
  std::vector<SConstellationLine*> constellations; ///< Constellation figure lines

  bool    drawConstellations;   ///< Whether to draw constellation lines
  float   constAlpha;           ///< Alpha-channel of constellation lines

  GLfloat *starsVertex;         ///< Vertex array for stars
  GLfloat *starsColour;         ///< Colour array for stars

  float     userLimit;          ///< User-specified limiting magnitude
  float     limitingMagnitude;  ///< Sky limiting visual magnitude
  float     ambientFactor;      ///< Sky ambient brightness factor
  bool      useVBO;             ///< Use vertex buffer objects for rendering
  bool      advancedStarEffects;///< Use shader program
  GLuint    vertStars;          ///< Stars vertex shader
  GLuint    progStars;          ///< Stars shader program

  sgMat4    T;                  ///< Transform matrix
};


/// \todo CMilkyWayImage class implements the visual model for galaxy nebulosity

/// \todo CCometImage class implements the visual model for a comet

/// \todo CMeteorImage class implements the visual model for a meteor

/// \todo CAuroraImage class implements the visual model for auroral activity

/// \todo CLunarEclipseImage class implements the visual model for lunar eclipses

/// \todo CSolarEclipseImage class implements the visual model for solar eclipses

//
// CMoonImage class implements the visual model for the moon
//
class CMoonImage {
public:
  CMoonImage (float distance);
  ~CMoonImage (void);

  // CMoonImage methods
  void          Repaint (double angle, float age);
  void          Reposition (sgVec3 p, double theta, double lst, double lat,
                            double ra, double dec, double spin);
  void          Draw (void);

protected:
  char          ddsFilename[PATH_MAX];  ///< DDS texture file for moon orb
  float         size;                   ///< Rendered size of moon orb
  float         distance;               ///< Rendering distance from viewer in feet
  GLuint        dlOrb;                  ///< GL display list for moon orb
  sgVec4        colour;                 ///< Moon colour
  GLuint        texid;                  ///< GL texture object
  sgMat4        T;                      ///< Transform matrix
  GLUquadric    *q;                     ///< Quadric for moon sphere
};

//
// CSolImage class implements the visual model for Sol (our sun)
//
class CSolImage {
public:
  CSolImage (double size);
  ~CSolImage (void);

  // CSolImage methods
  GLuint  GetSunTexture (const char* filename, int w);
  void    Repaint (double angle);
  void    Reposition (sgVec3 pos, double lst, double lat, double ra, double dec, double distance);
  void    Draw (void);

protected:
};

//
// CPlanetImage class implements the visual model for a planet
//
class CPlanetImage {
public:
  CPlanetImage (float r, float g, float b, const char* name);

  // CPlanetImage methods
  void    Repaint (float mv, float limit, float factor);
  void    Reposition (sgVec3 pos, double lst, double lat,
                        double ra, double dec, double distance);
  void    Draw (void);        ///< Top-level drawing method

protected:
  void    DrawPlanet (void);  ///< OpenGL commands to render the planet

protected:
  char    name[64];           ///< Planet name (mainly for debugging)
  float   x, y, z;            ///< Cartesian coordinates of planet
  float   r, g, b, a;         ///< Colour
  float   pointSize;          ///< Size of planet image point
  float   rotX, rotZ;         ///< Axis rotations
};

//=====================================================================================
// The CSkyManager class pulls all of the previous classes together into
//   the top-level application interface to the sky visual model.
//=====================================================================================
class CSkyManager {
protected:
  // Sky lighting model
  CSkyLight   *skylight;

  // Various sky object images
  CSkyDomeImage* domeimage;
  CStarImages*   starimage;
  CSolImage*     solimage;
  CMoonImage*    moonimage;
  CPlanetImage*  mercuryimage;
  CPlanetImage*  venusimage;
  CPlanetImage*  marsimage;
  CPlanetImage*  jupiterimage;
  CPlanetImage*  saturnimage;

  // Ephemeris calculators for the sun, moon and planets
  CSol      *sol;
  CMoon     *moon;
  CMercury    *mercury;
  CVenus      *venus;
  CMars     *mars;
  CJupiter    *jupiter;
  CSaturn     *saturn;

  // Darkness phase at last update
  EDarkPhase    old_phase;

  // Effective visibility, in feet
  float     effective_visibility;
  float     skyDistance;
  double    solTheta;                     //  JS conserve elevation
  double    solPhi;                       //  JS conserve orientation
  //--------------------------------------------------------------------
private:
  CSkyManager (void);
  static CSkyManager instance;

public:
  static CSkyManager& Instance() { return instance; }
  double GetSolElevation()  {return solTheta;}    //  JS get sun eleavtion
  double GetSolOrientation(){return solPhi;}      //  JS get sun orientation
  float  GetSolDistance()   {return skyDistance;} //  JS get sun distance
  // CSkyManager methods
  void      Init (void);
  void      Cleanup (void);
//  void      GetSkyDomeColour (float theta, float phi, float &r, float &g, float &b);
  void      PreDraw (void);
  void      Print (FILE *f);
  void      GetSkyDomeColour(float t, float p, float &r, float &g, float &b) 
            {domeimage->GetDomeColour(t, p, r, g, b);}
};

#endif // SKY_H

