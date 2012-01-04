/*
 * StarImages.cpp
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

/*! \file StarImages.cpp
 *  \brief Implements CStarImages to render stars in the night sky
 *
 * The design and implementation of these classes is based heavily on the
 *   sky model of SimGear 0.3.3, by Curtis Olson.  Integration into the
 *   Fly! Legacy project necessitated a rewrite of the actual classes,
 *   but I am indebteded to Curt and other contributors to the SimGear and
 *   FlightGear teams for their prior work in this area.
 *
 */

#include "../Include/Globals.h"
#include "../Include/Sky.h"
#include "../Include/Utility.h"
#include "../Include/Fui.h"
#include "../Include/GLSL.h"

using namespace std;

//
// This local function finds the next token in a tab-delimited record.  
static char* getToken (char* s, char* token, int size)
{
  // Initialize token to null string
	*token = 0;
  // Get pointer to next \t delimiter
  char *p = strchr (s, '\t');
  if (p != NULL) {
    int nChars = p - s;
    if (nChars > (size-1)) nChars = (size-1);
    strncpy (token, s, nChars);
    token[nChars] = '\0';

    // Increment p to point to start of next token
    p++;
  } else {
    // Delimiter not found, copy entire string to token
    strncpy (token, s, size);
  }
  
  return p;
}

//
// This function initializes the star database from the 5th edition
//   of the Yale Bright Star Catalog.  This database contains approx.
//   9000 stars down to magnitude +6.5.  For efficiency, only those stars with
//   visual magnitudes above the specified limit will be loaded.
//
/*

  Format of the BSC catalog data is as follows:

   1-  4  I4     ---     HR       [1/9110]+ Harvard Revised Number
                                    = Bright Star Number
   5- 14  A10    ---     Name     Name, generally Bayer and/or Flamsteed name
  15- 25  A11    ---     DM       Durchmusterung Identification (zone in
                                    bytes 17-19)
  26- 31  I6     ---     HD       [1/225300]? Henry Draper Catalog Number
  32- 37  I6     ---     SAO      [1/258997]? SAO Catalog Number
  38- 41  I4     ---     FK5      ? FK5 star Number
      42  A1     ---     IRflag   [I] I if infrared source
      43  A1     ---   r_IRflag  *[ ':] Coded reference for infrared source
      44  A1     ---    Multiple *[AWDIRS] Double or multiple-star code
  45- 49  A5     ---     ADS      Aitken's Double Star Catalog (ADS) designation
  50- 51  A2     ---     ADScomp  ADS number components
  52- 60  A9     ---     VarID    Variable star identification
  61- 62  I2     h       RAh1900  ?Hours RA, equinox B1900, epoch 1900.0 (1)
  63- 64  I2     min     RAm1900  ?Minutes RA, equinox B1900, epoch 1900.0 (1)
  65- 68  F4.1   s       RAs1900  ?Seconds RA, equinox B1900, epoch 1900.0 (1)
      69  A1     ---     DE-1900  ?Sign Dec, equinox B1900, epoch 1900.0 (1)
  70- 71  I2     deg     DEd1900  ?Degrees Dec, equinox B1900, epoch 1900.0 (1)
  72- 73  I2     arcmin  DEm1900  ?Minutes Dec, equinox B1900, epoch 1900.0 (1)
  74- 75  I2     arcsec  DEs1900  ?Seconds Dec, equinox B1900, epoch 1900.0 (1)
  76- 77  I2     h       RAh      ?Hours RA, equinox J2000, epoch 2000.0 (1)
  78- 79  I2     min     RAm      ?Minutes RA, equinox J2000, epoch 2000.0 (1)
  80- 83  F4.1   s       RAs      ?Seconds RA, equinox J2000, epoch 2000.0 (1)
      84  A1     ---     DE-      ?Sign Dec, equinox J2000, epoch 2000.0 (1)
  85- 86  I2     deg     DEd      ?Degrees Dec, equinox J2000, epoch 2000.0 (1)
  87- 88  I2     arcmin  DEm      ?Minutes Dec, equinox J2000, epoch 2000.0 (1)
  89- 90  I2     arcsec  DEs      ?Seconds Dec, equinox J2000, epoch 2000.0 (1)
  91- 96  F6.2   deg     GLON     ?Galactic longitude (1)
  97-102  F6.2   deg     GLAT     ?Galactic latitude (1)
 103-107  F5.2   mag     Vmag     ?Visual magnitude (1)
     108  A1     ---   n_Vmag    *[ HR] Visual magnitude code
     109  A1     ---   u_Vmag     [ :?] Uncertainty flag on V
 110-114  F5.2   mag     B-V      ? B-V color in the UBV system
     115  A1     ---   u_B-V      [ :?] Uncertainty flag on B-V
 116-120  F5.2   mag     U-B      ? U-B color in the UBV system
     121  A1     ---   u_U-B      [ :?] Uncertainty flag on U-B
 122-126  F5.2   mag     R-I      ? R-I   in system specified by n_R-I
     127  A1     ---   n_R-I      [CE:?D] Code for R-I system (Cousin, Eggen)
 128-147  A20    ---     SpType   Spectral type
     148  A1     ---   n_SpType   [evt] Spectral type code
 149-154  F6.3 arcsec/yr pmRA     ?Annual proper motion in RA J2000, FK5 system
 155-160  F6.3 arcsec/yr pmDE     ?Annual proper motion in Dec J2000, FK5 system
     161  A1     ---   n_Parallax [D] D indicates a dynamical parallax,
                                    otherwise a trigonometric parallax
 162-166  F5.3   arcsec  Parallax ? Trigonometric parallax (unless n_Parallax)
 167-170  I4     km/s    RadVel   ? Heliocentric Radial Velocity
 171-174  A4     ---   n_RadVel  *[V?SB123O ] Radial velocity comments
 175-176  A2     ---   l_RotVel   [<=> ] Rotational velocity limit characters
 177-179  I3     km/s    RotVel   ? Rotational velocity, v sin i
     180  A1     ---   u_RotVel   [ :v] uncertainty and variability flag on
                                    RotVel
 181-184  F4.1   mag     Dmag     ? Magnitude difference of double,
                                    or brightest multiple
 185-190  F6.1   arcsec  Sep      ? Separation of components in Dmag
                                    if occultation binary.
 191-194  A4     ---     MultID   Identifications of components in Dmag
 195-196  I2     ---     MultCnt  ? Number of components assigned to a multiple
     197  A1     ---     NoteFlag [*] a star indicates that there is a note
                                    (see file notes)
*/
void CStarImages::LoadBSCDatabase (const char* bscFilename)
{
  // Clear existing star database
  stars.clear();

  // Open Bright Star Catalog file
  PODFILE* p = popen (&globals->pfs, bscFilename);
  if (p) {
    
    // Define a buffer for each BSC record.
    char rec[512];

    while (!peof(p)) {
      // Read record
      pgets (rec, 512, p);

      // Get HR index number
      char hrString[16];
      memset (hrString, 0, 16);
      strncpy (hrString, rec, 4);
      int hr = atoi (hrString);

      // Get common name
      char name[16];
      memset (name, 0, 16);
      strncpy (name, rec+4, 10);

      // Get J2000.0 right ascension values
      char raHrs[8], raMin[8], raSec[8];
      memset (raHrs, 0, 8);
      memset (raMin, 0, 8);
      memset (raSec, 0, 8);
      strncpy (raHrs, rec+75, 2);
      strncpy (raMin, rec+77, 2);
      strncpy (raSec, rec+79, 4);

      // Convert right ascension to radians
      float rah = (float)atof (raHrs);
      float ram = (float)atof (raMin);
      float ras = (float)atof (raSec);
      float ra = rah + (ram / 60.0f) + (ras / 3600.0f);   // RA in hours
      ra = (ra / 24.0f) * (float)PI * 2.0f;               // RA in radians

      // Get J2000.0 declination values
      char decDeg[8], decMin[8], decSec[8];
      memset (decDeg, 0, 8);
      memset (decMin, 0, 8);
      memset (decSec, 0, 8);
      strncpy (decDeg, rec+83, 3);
      strncpy (decMin, rec+86, 2);
      strncpy (decSec, rec+88, 2);

      // Convert declination to radians
      float decd = (float)atof (decDeg);
      float decm = (float)atof (decMin);
      float decs = (float)atof (decSec);
      float dec = decd + (decm / 60.0f) + (decs / 3600.0f); // Dec in degrees
      dec = DegToRad (dec);                                 // Dec in radians

      // Get magnitude
      char mvText[8];
      memset (mvText, 0, 8);
      strncpy (mvText, rec+102, 5);
      float mv = (float)atof (mvText);

      // Get colour (BV)
      char bvText[8];
      memset (bvText, 0, 8);
      strncpy (bvText, rec+109, 5);
      float bv = (float)atof (bvText);

      // Store this star in the database
      if (mv < userLimit) {
        SStarData* star = new SStarData;
        strncpy (star->name, name,63);
        star->ra = WrapTwoPi (ra - (SGD_PI / 2));
        star->dec = dec;
        star->mv = mv;
        star->bv = bv;
        star->hr = hr;
        stars.push_back (star);
      }
    }
    pclose (p);
  } else {
    gtfo ("CStarImages : Could not open BSC5 database %s", bscFilename);
  }
}

//
// Load constellation stick figures.  The figures consists of a set of line
//   segments between stars.  The format of the constellation line database
//   file is:
//      1-3   C   3   Constellation abbreviation
//      4-11  I   8   HR1, first line endpoint
//      12-19 I   8   HR2, second line endpoint
//
// Since the HR index number is included in both the Fly! and BSC databases,
//   constellation figures can be displayed regardless of which database is
//   used.
//
void CStarImages::LoadConstellations (const char* txtFilename)
{
  // Clear existing star database
  constellations.clear();

  // Open constellation data file
  PODFILE* p = popen (&globals->pfs, txtFilename);
  if (p) {
    // Define a buffer for each record.
    char rec[512];

    while (!peof(p)) {
      // Read record
      pgets (rec, 512, p);
      TrimTrailingWhitespace (rec);

      // Skip blank lines and comment lines (beginning with semicolon)
      if ((rec[0] == ';') || (strlen(rec) == 0)) continue;

      // Get constellation name (abbreviated)
      char name[16];
      memset (name, 0, 16);
      strncpy (name, rec, 3);

      // Get HR index for first endpoint
      char hr1String[16];
      memset (hr1String, 0, 16);
      strncpy (hr1String, rec+3, 8);
      int hr1 = atoi (hr1String);

      // Get HR index for second endpoint
      char hr2String[16];
      memset (hr2String, 0, 16);
      strncpy (hr2String, rec+11, 8);
      int hr2 = atoi (hr2String);

      // Check that both endpoint stars are in the set of loaded stars
      int star1 = FindStarByHR (hr1);
      int star2 = FindStarByHR (hr2);
      if ((star1 != 1) && (star2!= -1)) {
        // Store in the constellation database
        SConstellationLine* line = new SConstellationLine;
        strncpy (line->name, name,63);
        line->star1 = star1;
        line->star2 = star2;
        constellations.push_back (line);
      }
    }

    pclose (p);

  } else {
    gtfo ("CStarImages : Could not open constellation database %s", txtFilename);
  }
}

//
// This function provides a simplified mapping of a star's B-V colour index
//   to an RGB colour triplet.  The data is taken from the web page,
//   "What color are the stars?" at:
//   http://www.vendian.org/mncharity/dir3/starcolor/details.html
//

typedef struct {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} SBVToRGB;

SBVToRGB bvMapping[] =
{
  {0x9b,  0xb2, 0xff},    // -0.40
  {0x9e,  0xb5, 0xff},    // -0.35
  {0xa3,  0xb9, 0xff},    // -0.30
  {0xaa,  0xbf, 0xff},    // -0.25
  {0xb2,  0xc5, 0xff},    // -0.20
  {0xbb,  0xcc, 0xff},    // -0.15
  {0xc4,  0xd2, 0xff},    // -0.10
  {0xcc,  0xd8, 0xff},    // -0.05
  {0xd3,  0xdd, 0xff},    //  0.00
  {0xda,  0xe2, 0xff},    //  0.05
  {0xdf,  0xe5, 0xff},    //  0.10
  {0xe4,  0xe9, 0xff},    //  0.15
  {0xe9,  0xec, 0xff},    //  0.20
  {0xee,  0xef, 0xff},    //  0.25
  {0xf3,  0xf2, 0xff},    //  0.30
  {0xf8,  0xf6, 0xff},    //  0.35
  {0xfe,  0xf9, 0xff},    //  0.40
  {0xff,  0xf9, 0xfb},    //  0.45
  {0xff,  0xf7, 0xf5},    //  0.50
  {0xff,  0xf5, 0xef},    //  0.55
  {0xff,  0xf3, 0xea},    //  0.60
  {0xff,  0xf1, 0xe5},    //  0.65
  {0xff,  0xef, 0xe0},    //  0.70
  {0xff,  0xed, 0xdb},    //  0.75
  {0xff,  0xeb, 0xd6},    //  0.80
  {0xff,  0xe9, 0xd2},    //  0.85
  {0xff,  0xe8, 0xce},    //  0.90
  {0xff,  0xe6, 0xca},    //  0.95
  {0xff,  0xe5, 0xc6},    //  1.00
  {0xff,  0xe3, 0xc3},    //  1.05
  {0xff,  0xe2, 0xbf},    //  1.10
  {0xff,  0xe0, 0xbb},    //  1.15
  {0xff,  0xdf, 0xb8},    //  1.20
  {0xff,  0xdd, 0xb4},    //  1.25
  {0xff,  0xdb, 0xb0},    //  1.30
  {0xff,  0xda, 0xad},    //  1.35
  {0xff,  0xd8, 0xa9},    //  1.40
  {0xff,  0xd6, 0xa5},    //  1.45
  {0xff,  0xd5, 0xa1},    //  1.50
  {0xff,  0xd2, 0x9c},    //  1.55
  {0xff,  0xd0, 0x96},    //  1.60
  {0xff,  0xcc, 0x8f},    //  1.65
  {0xff,  0xc8, 0x85},    //  1.70
  {0xff,  0xc1, 0x78},    //  1.75
  {0xff,  0xb7, 0x65},    //  1.80
  {0xff,  0xa9, 0x4b},    //  1.85
  {0xff,  0x95, 0x23},    //  1.90
  {0xff,  0x7b, 0x00},    //  1.95
  {0xff,  0x52, 0x00}     //  2.00
};

const int nMappings = sizeof(bvMapping) / sizeof(SBVToRGB);

static void bv_to_rgb (float bv, float& r, float& g, float& b)
{
  // Convert B-V colour to table index
  /// @todo Interpolate colours between indices
  int i = (int)(floor (bv + 0.40) * 20.0);

  // Clamp to table bounds
  if (i < 0) i = 0;
  if (i > nMappings) i = nMappings;

  // Extract char values from mapping table and convert to float
  SBVToRGB *p = &bvMapping[i];
  r = (float)(p->r) / 255.0f;
  g = (float)(p->g) / 255.0f;
  b = (float)(p->b) / 255.0f;
}

//
// Convert B-V and V-R values for a star to RGB colour.  Algorithm is taken
//   from "Visual Star Colours from Instrumental Photometry" by Truman P. Kohman,
//   published in the Journal of the Royal Astronomical Society of Canada,
//   Vol. 99 No. 3 [712] (June 2005) pp. 98-105.
//
// This algorithm is included for interest, although it is not currently used
//   to compute star colour.  The Bright Star Catalog currently used does
//   not contain the V-R photometry data required for this algorithm.  Future
//   use of a catalog containing Johnson UVBRI data could be used.
//
static void kohman_star_colour (float bv, float vr, float& r, float& g, float& b)
{
  // Adjustment to B-V for 5800K/Sun
  float adjustBV = -0.39f;

  // Adjustment to V-R for 5800K/Sun
  float adjustVR = -0.15f;

  // (R-I) - (V-R) which can be assumed to be a constant for all stars
  float delVRI = -0.20f;

  // V-I
  float vi = 2.0f * (vr + adjustVR) + delVRI;

  // Calculate adjusted B, V, R, I photometric values
  float B = bv + adjustBV;
  float V = 0;
  float R = -vr - adjustVR;
  float I = -vi;

  // Interim RGB values based on adjusted photmetric values
  float r1 = pow(10, -0.4f * R);
  float g1 = 1;
  float b1 = pow(10, -0.4f * B);
  float i1 = pow(10, -0.4f * I);

  // Tweak red to compensate for difference in R photometry and standard red colour
  float r1c = r1/(2.0f * (1.0f + r1 / (r1 + i1)));
  float sum = r1c + g1 + b1;

  // Normalize relative RGB components
  float r2 = r1c / sum;
  float g2 = g1 / sum;
  float b2 = b1 / sum;

  // Normalize so that brightest component is always 1.0
  float max = r2;
  if (g2 > max) max = g2;
  if (b2 > max) max = b2;
  r = r2 / max;
  g = g2 / max;
  b = b2 / max;
}

//
// Find the index of a star with the given HR index number.  If the HR index
//   does not exist, the function returns -1.
//
int CStarImages::FindStarByHR (int hr)
{
  int rc = -1;

  std::vector<SStarData*>::iterator i;
  int count = 0;
  for (i=stars.begin(); i!=stars.end(); i++, count++) {
    SStarData *s = *i;
    if (s->hr == hr) {
      // Found a match
      rc = count;
      break;
    }
  }
  return rc;
}

CStarImages::CStarImages (double distance)
{
  // Check for availability of vertex buffer objects
  useVBO = false;
  if (glewIsSupported ("GL_ARB_vertex_buffer_object")) useVBO = true;

  // Get INI setting for advanced star effects
  bool advancedStarEffects = false;
  int i = 0;
  GetIniVar ("Sky", "advancedStarEffects", &i);
  advancedStarEffects = (i != 0);

  // Get INI setting for limiting magnitude
  float limit = 0;
  GetIniFloat ("Sky", "starLimitingMagnitude", &limit);
  if (limit == 0) {
    limit = 4.5;
  }
  userLimit = limit;

  // Initialize flag for drawing of constellation figures; this is only supported
  //   with the BSC5 star databse
  drawConstellations = false;

  // Load stars from BSC database
	char bscDbName[64] = {0};
  GetIniString ("Sky", "starDatabaseName", bscDbName, 64);
  if (strlen (bscDbName) == 0) strncpy (bscDbName, "System/BSC5.CAT",63);
  LoadBSCDatabase (bscDbName);

  // Check INI setting to see if constellation figures are to be drawn
  // Constellation figures can only be drawn when the complete BSC database
  // is loaded
  i = 0;
  GetIniVar ("Sky", "starShowConstellations", &i);
  if (userLimit >= 6.5f) {
    drawConstellations = (i != 0);
    if (drawConstellations) {
      // Get filename of constellation figures fron INI settings
			char constDbName[64] = {0};
      GetIniString ("Sky", "starConstellationsName", constDbName, 64);
      if (strlen (constDbName) == 0) strncpy (constDbName, "System/DefaultConstL.txt",63);
  

      // Load the constellation stick figures.  This will update the class
      //   member 'constellations', a vector of star endpoints.  Only those
      //   lines that can be drawn, given the set of stars that were loaded,
      //   will be added to the vector.
      LoadConstellations (constDbName);
    }
  }

  // Allocate vertex and colour arrays for potentially visible stars
  int nStars = stars.size();
  starsVertex = new GLfloat[3 * nStars];
  starsColour = new GLfloat[4 * nStars];

  // Populate vertex and colour arrays
  std::vector<SStarData*>::iterator iterStar;
  for (iterStar=stars.begin(), i=0; iterStar!=stars.end(); iterStar++, i++) {
    SStarData *s = *iterStar;
    // Update vertex array
    starsVertex[i * 3]     = distance * (float)(cos (s->ra) * cos (s->dec)); // X-coordinate
    starsVertex[i * 3 + 1] = distance * (float)(sin (s->ra) * cos (s->dec)); // Y-coordinate
    starsVertex[i * 3 + 2] = distance * (float)(sin (s->dec));               // Z-coordinate

    // Update colour array
    bv_to_rgb (s->bv, starsColour[i*4], starsColour[i*4+1], starsColour[i*4+2]);
    starsColour[i*4+3] = 1.0f;    // Default alpha
  }

  progStars = 0;
  if (advancedStarEffects && glewIsSupported ("GL_ARB_vertex_shader")) {
    // Load star shaders
    vertStars = LoadShader (GL_VERTEX_SHADER, "Modules/GLSL/Stars.vert");
    progStars = CreateShaderProgram (1, vertStars);
  }
}

CStarImages::~CStarImages (void)
{
  // Clean up star and constellation lists
  std::vector<SStarData*>::iterator i;
  for (i=stars.begin(); i!=stars.end(); i++) delete *i;
  stars.clear();

  std::vector<SConstellationLine*>::iterator j;
  for (j=constellations.begin(); j!=constellations.end(); j++) delete *j;
  constellations.clear();

  // Deallocate vertex and colour arrays
  SAFE_DELETE_ARRAY (starsVertex);
  SAFE_DELETE_ARRAY (starsColour);
}

void CStarImages::Repaint (float limit, float factor)
{
  limitingMagnitude = limit;
  ambientFactor = factor;

  // Set brightness of constellation lines
  float cutoff = 1.0f;
  if (limit <= cutoff) {
    // Sky is bright, make constellation lines invisible
    constAlpha = 0.0f;
  } else {
    // Sky is dark, scale alpha channel
    constAlpha = (limit - cutoff) * 0.1f;
  }
}

//
// Reposition the image of the stars in the sky
//
// Arguments:
//  pos     3D position of the eye (cartesian coordinates??)
//  lst     Local Sidereal Time, in degrees
//
void CStarImages::Reposition (sgVec3 pos, double lon, double lat, double lst)
{
  // Initialize transform matrix
  sgMakeIdentMat4 (T);
  
  sgMat4 LAT, LST;
  sgVec3 axis;

  // Apply latitude rotation around x-axis
  sgSetVec3 (axis, -1.0f, 0, 0);
  sgMakeRotMat4 (LAT, (float)(90.0 - lat), axis);
  sgPreMultMat4 (T, LAT);

  // Apply local sidereal time rotation around z-axis
  sgSetVec3 (axis, 0, 0, -1.0f);
  sgMakeRotMat4 (LST, (float)(15.0 * lst), axis);
  sgPreMultMat4 (T, LST);
}

void CStarImages::DrawStarsWithShader (void)
{
  // Activate shader program(s)
  glUseProgram (progStars);
  glUniform1f (GetUniformLocation (progStars, "LimitingMagnitude"), limitingMagnitude);

  // Draw stars
  unsigned int i;
  glBegin (GL_POINTS);
    for (i=0; i<stars.size(); i++) {
      SStarData *s = stars[i];
      glColor3f (s->r, s->g, s->b);
      glVertexAttrib1f (GetAttribLocation (progStars, "Magnitude"), s->mv);
      glVertex3f (s->x, s->y, s->z);
    }
  glEnd ();

  // Deactivate shader program
  glUseProgram (0);
}

void CStarImages::DrawStarsNoShader (void)
{
  // Update colour array
  std::vector<SStarData*>::iterator iterStar;
  int i;
  for (iterStar=stars.begin(), i=0; iterStar!=stars.end(); iterStar++, i++) {
    SStarData *s = *iterStar;

    // Assign alpha based on difference between star visual magnitude and sky
    //   limiting magnitude
    float alpha = 0;
    if (s->mv < limitingMagnitude) {
      // This star is brighter than the limiting magnitude
      // Calculate magnitude difference and scale to [0,1]
      float nmag = (limitingMagnitude - s->mv) / limitingMagnitude;
      alpha = (nmag * 0.85f);         // Scale alpha to 0.0..0.85
      alpha += 0.25f;                 // Bias +25%
      alpha *= ambientFactor;         // Adjust alpha for ambient light
    }
    // Clamp alpha to 0.0..1.0
    if (alpha > 1.0f) alpha = 1.0f;
    if (alpha < 0.0f) alpha = 0.0f;

    // Update alpha in colour array
    starsColour[i * 4 + 3] = alpha;
  }

  // Render points using vertex and colour arrays
  glPointSize (2.0f);
  glEnableClientState (GL_VERTEX_ARRAY);
  glEnableClientState (GL_COLOR_ARRAY);
  glVertexPointer (3, GL_FLOAT, 0, starsVertex);
  glColorPointer (4, GL_FLOAT, 0, starsColour);
  glDrawArrays (GL_POINTS, 0, stars.size());
  glDisableClientState (GL_VERTEX_ARRAY);
  glDisableClientState (GL_COLOR_ARRAY);
}

void CStarImages::DrawStars (void)
{
  // Set GL state
  glPushAttrib (GL_POINT_BIT);
  glEnable (GL_POINT_SMOOTH);
  glDisable(GL_TEXTURE_2D);

  if (progStars != 0) {
    // Stars shader program is defined; use shader-specific renderer
    DrawStarsWithShader ();
  } else {
    DrawStarsNoShader ();
  }

  // Restore GL state
  glPopAttrib ();
}

void CStarImages::DrawConstellations (void)
{
  // Check that constellation lines are bright enough to be drawn
  if (constAlpha > 0) {
    // Set GL state
    glPushAttrib (GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_FOG_BIT | GL_LINE_BIT);
    glDisable (GL_LIGHTING);
    glDisable (GL_CULL_FACE);
    glDisable (GL_TEXTURE_2D);
    glDisable (GL_COLOR_MATERIAL);
    glEnable (GL_BLEND);
    glDisable (GL_DEPTH_TEST);
//    glDisable (GL_FOG);
    glEnable (GL_LINE_SMOOTH);
    glLineWidth (1.0f);

    // Draw constellation figure lines
    std::vector<SConstellationLine*>::iterator i;
    glBegin (GL_LINES);
      // Default constellation line colour
      glColor4f (0.5f, 0.75f, 1.0f, constAlpha);
      for (i=constellations.begin(); i!=constellations.end(); i++) {
        // Set vertex of each endpoint
        SStarData *s1 = stars[(*i)->star1];
        SStarData *s2 = stars[(*i)->star2];
        glVertex3f (s1->x, s1->y, s1->z);
        glVertex3f (s2->x, s2->y, s2->z);
      }
    glEnd ();

    // Restore GL state
    glPopAttrib ();
  }
}

//
// Draw the stars.  This is a direct OpenGL rendering
//
void CStarImages::Draw (void)
{
  // Transform
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  glMultMatrixf ((GLfloat*)T);

  // Draw stars, and optionally, constellation figures
  DrawStars ();
  if (drawConstellations) {
    DrawConstellations ();
  }
  glMatrixMode (GL_MODELVIEW);
  glPopMatrix ();
}
