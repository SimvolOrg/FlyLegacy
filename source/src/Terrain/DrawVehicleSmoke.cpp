/*
 * VehicleSmoke.cpp
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

/*! \file VehicleSmoke.cpp
 *  \brief Implements CVehicleSmoke rendering class
 *
 * The CVehicleSmoke draws the aircraft smoke
 *   
 *   
 */

#include "../Include/Globals.h"
#include "../Include/TerrainTexture.h" // CArtParser
#include "../Include/DrawVehicleSmoke.h"

using namespace std;

///==============================================================================
/// CBaseSmoke
//  JS NOTES:  Made the following adaptations
//    Put all common initialization in base class here
//    Use TC_VTAB for interleaved Array for better performances
///==============================================================================
CBaseSmoke::CBaseSmoke (int n,U_INT ip)
{ point_pos.clear ();
  spr                     = NULL;
  timer                   = 10.0f;           // Default timer  10
  version                 = 0;               // 1 = default vector / 2 = queued
  nb                      = n;               // queue number of points
  nb_particles            = 10;              // Default number of particles = 10
  PARTICLE_SIZE           = 0.0f;
  HALF_PARTICLE_SIZE      = 0.0f;
  NUM_PARTICLES           = 0;
  INITIAL_PARTICLE_SPREAD = 0;
  SPEED_DECAY             = 0.0f;            // (Gravity) 
  smoke_texture[0]        = 0;
  smoke_texture[1]        = 0;
  max_spread              = 0;
  max_particles           = 0;
  index                   = 0;
  spread                  = 0.0f;
  angle                   = 0.0f;
  draw                    = 0;
  ticks                   = 0.0f;
  smk_t                   = 0;               // 1=one tga  2=two tga
  smkpos_x                = 0.0f,            // smoke position
  smkpos_y                = 0.0f,
  smkpos_z                = -15.0f;          // Default position
  smkcolor_r              = 0.9f,            // smoke color
  smkcolor_g              = 0.9f,
  smkcolor_b              = 0.8f;
  //---Init parameters commun to all smokes -----------------
  delay = 0.15f;//0.30f;
  GetIniFloat ("Graphics", "drawSmokeDelay", &delay);
  //---Particule size ---------------------------------------
  float  ps = 1;      // Default particule size
  GetIniFloat ("Graphics", "drawSmokeParticleSize", &ps);
  SetQuad(ps);
  //---Initial particle spread ------------------------------
  INITIAL_PARTICLE_SPREAD = 25;
  GetIniVar ("Graphics", "drawSmokeInitialParticles", &INITIAL_PARTICLE_SPREAD);
  max_spread = INITIAL_PARTICLE_SPREAD;
  //---Speed decay value ------------------------------------
  SPEED_DECAY = 0.01f; //0.05f;
  GetIniFloat ("Graphics", "drawSmokeSpeedDecay", &SPEED_DECAY);
  //--- Process number of textures --------------------------
  int nbs = 1;
  GetIniVar ("Graphics", "drawSmokeType", &nbs);
  if ((nbs != 1) && (nbs != 2)) nbs = 1;
  smk_t   = nbs;
  // allocate all textures in one go
  glGenTextures (nbs, smoke_texture);
  // load the textures 
  char ntx = 0;
  if (smk_t != 0) ntx += LoadTexture ("smoke",    smoke_texture [0]);
  if (smk_t == 2) ntx += LoadTexture ("smokebis", smoke_texture [1]);
  draw = (smk_t == ntx);
  //--Initial value -----------------------------------------
  on = ip;

}
//---------------------------------------------------------------------
//  Delete all resources
//---------------------------------------------------------------------
CBaseSmoke::~CBaseSmoke (void)
{ 
  // clean up textures
  glDeleteTextures (smk_t, smoke_texture);
  SAFE_DELETE_ARRAY (spr);
  //
  point_pos.clear ();
}
//---------------------------------------------------------------------
//  Detect change state in drawing. Init everything if yes
//---------------------------------------------------------------------
bool CBaseSmoke::Reset (U_INT p)
{ if (p == on)  return (on == 0);
  //--- changing status ----------------
  if (0 == version) point_pos.clear ();
  if (1 == version) FillStruct ();
  on  = p;
  return (on == 0);
}
//---------------------------------------------------------------------
//  Send all particles to death
//---------------------------------------------------------------------
void CBaseSmoke::FillStruct (void)
{
  int rdn = (globals->clk->GetMinute() << 8) +  globals->clk->GetSecond();
  srand(rdn);
  for (int index = 0; index != NUM_PARTICLES; index++) {
    spr[index].life = static_cast<float> (rand () / 100 % 2);
    spr[index].r    = 1.0f;
    spr[index].g    = 1.0f;
  }
}
//---------------------------------------------------------------------
//  Fill the quad with particle dimension
//---------------------------------------------------------------------
void CBaseSmoke::SetQuad(float dim)
{ PARTICLE_SIZE       = dim;
  HALF_PARTICLE_SIZE  = dim * 0.5f;
  //---Set Vertex NW -----------------
  quad[0].VT_S = 0;
  quad[0].VT_T = 0;
  quad[0].VT_X = -dim;
  quad[0].VT_Y = +dim;
  quad[0].VT_Z = 0;
  //---Set vertex SW -----------------
  quad[1].VT_S = 0;
  quad[1].VT_T = 1;
  quad[1].VT_X = -dim;
  quad[1].VT_Y = -dim;
  quad[1].VT_Z = 0;
  //---Set vertex SE -----------------
  quad[2].VT_S = 1;
  quad[2].VT_T = 1;
  quad[2].VT_X = +dim;
  quad[2].VT_Y = -dim;
  quad[2].VT_Z = 0;
  //---Set vertex NE -----------------
  quad[3].VT_S = 1;
  quad[3].VT_T = 0;
  quad[3].VT_X = +dim;
  quad[3].VT_Y = +dim;
  quad[3].VT_Z = 0;
}
//---------------------------------------------------------------------
//  Finish initialization
//---------------------------------------------------------------------
void CBaseSmoke::Init()
{ NUM_PARTICLES = nb * nb_particles;
  max_particles = NUM_PARTICLES;
  spr = new SpriteInfo [NUM_PARTICLES];
  FillStruct ();
}
//---------------------------------------------------------------------
//  LC: Load smoke texture
//---------------------------------------------------------------------
bool CBaseSmoke::LoadTexture (char *texname, GLuint texhandle)
{ TEXT_INFO xds;
	sprintf (xds.path,"ART/%s.TGA",texname);
  xds.azp = 0;
  CArtParser img (1);
  GLubyte *txt = img.GetAnyTexture (xds);
  if (NULL == txt) {
    WARNINGLOG ("can't load texture %s.tga", texname);
    return false;
  }
  //MEMORY_LEAK_MARKER ("GetAnyTexture");
  glBindTexture (GL_TEXTURE_2D, texhandle); // Set our Tex handle as current

#ifdef _DEBUG
  //TRACE ("CVehicleSmoke::LoadTexture bpp=%d", img.GetBPP ());
#endif

  // Create the texture
   if (img.GetBPP () == 24)
    glTexImage2D (GL_TEXTURE_2D, 
                  0, 3, 
                  img.GetWidth (), img.GetHeigth (),
                  0, GL_RGBA,GL_UNSIGNED_BYTE,
                  txt
                 );
   else if (img.GetBPP () == 32)
    glTexImage2D (GL_TEXTURE_2D, 
                  0, 4, 
                  img.GetWidth (), img.GetHeigth (),
                  0, GL_RGBA,GL_UNSIGNED_BYTE,
                  txt
                 );
   else 
		 return false;

  //--- Specify filtering and edge actions --------------------
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  
  // always free texture when GetAnyTexture is used
  // same as : if (txt) delete [] txt;
  img.FreeAnyTexture (txt);
  //
  return true;
}
//--------------------------------------------------------------------------
//  Update all particles
//--------------------------------------------------------------------------
void CBaseSmoke::UpdateParticles (void)
{
  // Update particles, generating new if required
  for(index = 0; index != max_particles; index++)
  { 
     float val = PARTICLE_SIZE * 1.5;
     if (spr[index].life > 0.0f)
     {
        spr[index].xPos += spr[index].xVec * ticks * val;
        spr[index].yPos += spr[index].yVec * ticks * val;
        spr[index].zPos += spr[index].zVec * ticks * val;
        spr[index].yVec -= SPEED_DECAY * ticks;  

        spr[index].life -= 0.1f * ticks;
     }
     else // Spawn a new particle
     {
        // Reset position
        spr[index].xPos = + HALF_PARTICLE_SIZE;        //
        spr[index].yPos = - HALF_PARTICLE_SIZE;        // 
        spr[index].zPos = - HALF_PARTICLE_SIZE;        //

        // Get a random spread and direction
        spread = static_cast<float> (rand () % max_spread) / (2.5f * max_spread);
        angle = DegToRad (static_cast<float> (rand () % 360)); //

        // Calculate X and Z vectors
        spr[index].xVec = cos (angle) * spread;
        spr[index].zVec = sin (angle) * spread * val;

        // Randomly reverse X and Z vector to complete the circle
        if(rand () % 2)  spr[index].xVec = - spr[index].xVec;

        if(rand () % 2)  spr[index].zVec = - spr[index].zVec;

        // Get a random initial speed
        spr[index].yVec = static_cast<float> (rand () % 500) / 10000.0f;

        // Get a random life and 'yellowness'
        spr[index].life = static_cast<float> ((rand () % 100) / 100.0f);
        spr[index].b = 0.75f + static_cast<float> (rand () % 50 + 1) / 200.0f;
     }
  }
}
//--------------------------------------------------------------------------
//  Draw smoke
// JS:  Made the following adaptation
//    Suppress DEPTH TEST for better looking
//    Use interleave array for better performances
//--------------------------------------------------------------------------
void CBaseSmoke::Draw (U_INT op)
{ if (!draw)				return;
  if (Reset(op))    return;
  SPosition upos = globals->geop;
  SVector cam; globals->cam->GetOrientation (cam);
  int del = int(delay * 100.0f);
  ticks   = globals->tim->GetDeltaSimTime ();

  ///////////////////////////////////////////////////////
  /// ROTATION                                         //
  ///////////////////////////////////////////////////////
  SVector from = {smkpos_x, smkpos_y, -smkpos_z};      //
  SVector ori = globals->iang;                         // RH
  VectorOrientLeftToRight (ori);                       // RH->LH
  SVector to;                                          //
  CRotationMatrixBHP matx;                             // LH
  matx.Setup (ori);                                    // LH
  matx.ParentToChild (to, from);                       // LH
  VectorOrientLeftToRight (to);                        // LH->RH
  ///////////////////////////////////////////////////////

  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable (GL_TEXTURE_2D);
  glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glDisable(GL_DEPTH_TEST);
  delay = (rand () % del) / 100.0f + 0.1f;

  if (1 == version) {
    timer += ticks;
    if (timer > delay) {
      timer = 0.0f;
      if (posQ.size () > nb) {
        oldQ.pop ();
      }
      oldQ.push (upos);
    }

    UpdateParticles ();

    int smk = 0; 
    if (2 == smk_t) smk = rand () / 100 % 2;
    glBindTexture (GL_TEXTURE_2D, smoke_texture[smk]);
    glPushMatrix ();

    int num = 0; // used in lit
    while (!posQ.empty ()) {
      geol = SubtractPositionInFeet (upos, posQ.front ());
      glPushMatrix ();
      // Place the quad and rotate to face the viewer
      glTranslated (geol.x+to.x, geol.y+to.y, geol.z-to.z);
      glRotated (RadToDeg (cam.z), 0.0, 0.0, 1.0);
      glRotated (90.0 - RadToDeg (cam.x), 1.0, 0.0, 0.0);
      double lit = min (0.5, double(nb - num) / double(nb));
      glColor4f (smkcolor_r, smkcolor_g, smkcolor_b, lit);

      for (index = num * nb_particles; index != (num + 1) * nb_particles; index++) {
        //glColor4f (spr[index].r, spr[index].g, spr[index].b, spr[index].life);
        glTranslatef (spr[index].xPos, spr[index].yPos, spr[index].zPos);
        //---JS:  Modify to use interleave array for better performances -------
        glInterleavedArrays(GL_T2F_V3F,0,quad);
        glDrawArrays(GL_POLYGON,0,4);
        // return
        glTranslatef (-spr[index].xPos,-spr[index].yPos,-spr[index].zPos);
      }
      posQ.pop ();
      glPopMatrix ();
      num++;
    }
    posQ = oldQ;
  }
  glPopAttrib();
  glPopClientAttrib();
  glDisable (GL_TEXTURE_2D);
  glPopMatrix();
}

//==========================================================================
// CVehicleSmoke
//  JS:   Put it as an external subsystem in the  amp system.  This belong to 
//        the aircraft and it should not be at global level
//==========================================================================
CVehicleSmoke::CVehicleSmoke (CVehicleObject *mv)
{ TypeIs (SUBSYSTEM_SMOKE);
  hwId = HW_UNKNOWN;
  SetIdent('vsmk');       // Vehicle smoke
  //------------------------------------------
	mveh	= mv;
  bs = new CBaseSmoke(25,mveh->GetOPT(VEH_DW_SMOK));  // Use 25 points
  bs->version = 2;
  GetIniVar   ("Graphics", "drawSmokeVersion", &bs->version);
  if (bs->version) bs->version -= 1;
  /// 0 = default vector / 1 = queue
  if (1 == bs->version) {
    int nb_ = int(bs->nb);
    GetIniVar ("Graphics", "drawSmokeNb", &nb_);
    bs->nb  = int(nb_);
  }
  if (0 == bs->version) bs->point_pos.clear ();

  char smkpos [32] = "0.0,0.0,-15.0";
  GetIniString ("Graphics", "drawSmokePosition", smkpos, 32);
  sscanf (smkpos, "%f,%f,%f", &bs->smkpos_x, &bs->smkpos_y, &bs->smkpos_z);

  char smkcolor [32] = "0.9,0.9,0.8"; //0.98,0.745,0.785
  GetIniString ("Graphics", "drawSmokeColor", smkcolor, 32);
  sscanf (smkcolor, "%f,%f,%f", &bs->smkcolor_r, &bs->smkcolor_g, &bs->smkcolor_b);
  bs->Init();
}
//-------------------------------------------------------------
//  Free associated resources
//-------------------------------------------------------------
CVehicleSmoke::~CVehicleSmoke (void)
{ //TRACE ("CVehicleSmoke::~CVehicleSmoke %p", spr);
  SAFE_DELETE (bs);
}
//------------------------------------------------------------------
//  Toggle request from other components
//------------------------------------------------------------------
EMessageResult CVehicleSmoke::ReceiveMessage (SMessage *msg)
{ if (msg->id != MSG_SETDATA ) return CSubsystem::ReceiveMessage(msg);
  switch (msg->user.u.datatag) {
     //---- Proceed reset  ----------------------
      case 'togl':
          mveh->ToggleOPT(VEH_DW_SMOK);
          return MSG_PROCESSED;
  }
  return CSubsystem::ReceiveMessage(msg);
}
//-------------------------------------------------------------
//  Draw if request and camera is outside
//-------------------------------------------------------------
void CVehicleSmoke::Draw (void)
{	if (globals->noEXT)  return;
	U_INT op	= mveh->GetOPT(VEH_DW_SMOK);
  bs->Draw (op);
  return;
}

///==============================================================================
/// CSubsystemSmoke:  A subsystem in amp to produce smoke
//  JS:  Move this to external subsystem in amp as it belong to the aircraft
///==============================================================================
CSubsystemSmoke::CSubsystemSmoke (CVehicleObject *mv) 
{ TypeIs (SUBSYSTEM_SMOKE);
	mveh	= mv;
  on		= 0;
  //-----------------------------------------------------------------
  bs = new CBaseSmoke(25,0);          // Use 25 points
  //! only version 1 is implemented
  bs->version = 1;
}

CSubsystemSmoke::~CSubsystemSmoke (void)
{ //TRACE ("CSubsystemSmoke::~CSubsystemSmoke %p", vs);
  SAFE_DELETE (bs);
}
//------------------------------------------------------------------
//  Read parameters
//------------------------------------------------------------------
int CSubsystemSmoke::Read (SStream *stream, Tag tag)
{
  switch (tag) {
  case 'posn':
		//
    { char smkpos [32] = "0.0,0.0,-15.0";
      ReadString (smkpos, 32, stream);
      sscanf (smkpos, "%f,%f,%f", &bs->smkpos_x, &bs->smkpos_y, &bs->smkpos_z);
    }
		return TAG_READ;
  case 'colr':
		//
    { char smkcolor [32] = "0.9,0.9,0.8"; //0.98,0.745,0.785
      ReadString (smkcolor, 32, stream);
      sscanf (smkcolor, "%f,%f,%f", &bs->smkcolor_r, &bs->smkcolor_g, &bs->smkcolor_b);
    }
		return  TAG_READ;
  case 'dens':
		//
    ReadUInt (&bs->nb, stream);
		return  TAG_READ;
  }
  return  CDependent::Read (stream, tag);
}
//------------------------------------------------------------------
//  All parameters are read. Finalize
//------------------------------------------------------------------
void CSubsystemSmoke::ReadFinished (void)
{ bs->Init();
}
//------------------------------------------------------------------
//  Process receive message
//------------------------------------------------------------------
EMessageResult CSubsystemSmoke::ReceiveMessage (SMessage *msg)
{ if (msg->id != MSG_SETDATA ) return CDependent::ReceiveMessage(msg);
  switch (msg->user.u.datatag) {
     //---- Return voltage ----------------------
      case 'swap':
          on ^= 1;
          return MSG_PROCESSED;
  }
  return CDependent::ReceiveMessage(msg);
}
//------------------------------------------------------------------
//  Draw if allowed and camera is outside
//------------------------------------------------------------------
void CSubsystemSmoke::Draw (void)
{ if (0 == on)          return;
  if (globals->noINT)  return;
  bs->Draw (mveh->GetOPT(VEH_DW_SMOK));
  return;
}
//=========================END 0F FILE ====================================================
