/*
 * Cloud.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005 Chris Wallace
 * Copyright 2007 Jean Sabatier
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
//========================================================================================
//  Some code is directly inspired from the following authors with permission
//  Many Thanks to them.  Jean Sabatier
//========================================================================================
//**************
// Source Code:
//**************
//Copyright 2005
//
//Andrei Stoian, andrei.stoian@gmail.com, http://bit13.no-ip.com
//Permission to use, copy, modify, distribute and sell this software and its 
//documentation for any purpose is hereby granted without fee, provided that the 
//above copyright notice appear in all copies and that both that copyright notice 
//and this permission notice appear in supporting documentation. Binaries 
//compiled with this software may be distributed without any royalties or 
//restrictions.  
//
//The author makes no representations about the suitability of this software for any purpose. 
//It is provided "as is" without express or implied warranty.
//
//*******************
// Cloud Data Files:
//*******************
//
//Copyright 2002 
//Mark J. Harris and The University of North Carolina at Chapel Hill
//
//Permission to use, copy, modify, distribute and sell this software and its 
//documentation for any purpose is hereby granted without fee, provided that the 
//above copyright notice appear in all copies and that both that copyright notice 
//and this permission notice appear in supporting documentation. Binaries 
//compiled with this software may be distributed without any royalties or 
//restrictions.  
//
//The author and The University of North Carolina at Chapel Hill make no 
//representations about the suitability of this software for any purpose. It is 
//provided "as is" without express or implied warranty.

//=====================================================================================
//  Cloud system 
#include "../Include/FlyLegacy.h"
#include "../Include/Utility.h"
#include "../Include/Globals.h"
#include "../Include/3DMath.h"
#include "../Include/TerrainUnits.h"
#include "../Include/TerrainCache.h"
#include "../Include/TerrainTexture.h"
#include "../Include/FileParser.h"
#include "../Include/Cloud.h"
#include "../Include/Pod.h"
#include <algorithm>
#include <vector>
#include <stdarg.h>
#include <math.h>
#include <sys/timeb.h>
#include <time.h>
using namespace std;
//=====================================================================================
//  Cloud Layer definition
//  0 No Cloud
//  1 Few               (1 per QGT)
//  2 Few sparce        (1 per QGT) (bigger)
//  3 Scatered sparce   (2 per QGT)
//  4 Scatered dense    (2 per QGT) (bigger)
//  5 Broken            (3 per QGT)
//  6                   (3 per QGT) bigger
//=====================================================================================
//  Layer parameters
//=====================================================================================
struct LAYER_PRM {
  float scale;                  // Scale 
  U_CHAR typ0;                  // Type 0 permitted
  U_CHAR typ1;                  // Type 1 permitted
  U_CHAR typ2;                  // Type 2 permitted
  U_CHAR mask;                  // Selector mask
};
//=====================================================================================
//  Layer parameter table
//=====================================================================================
LAYER_PRM layTAB[] =
{ 0.0,   0, 0, 0, 0,              // Layer 0
  2400,  1, 1, 1, 0,              // Flat clouds
  2400,  1, 0, 0, 0x07,           // Layer 2
  3200,  1, 0, 0, 0x07,           // Layer 3
  2800,  1, 1, 0, 0x03,           // Layer 4
  3200,  1, 1, 0, 0x03,           // Layer 5
  2800,  1, 1, 1, 0x01,           // Layer 6
  3200,  1, 1, 1, 0x01,           // Layer 7
};
//=====================================================================================
//  Indice to draw puff outline
//=====================================================================================
GLubyte outBOX[] = {
  0,1,2,3,              // front face
  4,0,3,7,              // Left face
  4,5,6,7,              // Back face
  1,5,6,2,              // Right face
  3,2,6,7,              // Top face
  0,4,5,1,              // Bottom face
};
//=====================================================================================
//  sort Particle from model on Y direction
//=====================================================================================
bool SortModel(CCloudParticle *p1,CCloudParticle *p2)
{ return (p1->GetPY() < p2->GetPY());
}

//=====================================================================================
//  sort puffs away from camera for rendering
//=====================================================================================
bool SortPuffsToCamera(CCloudPuff *p1,CCloudPuff *p2)
{ return (p1->FeetToCamera() > p2->FeetToCamera());
}
//=====================================================================================
//  Sort particle away from sun 
//=====================================================================================
bool SortAway(CCloudParticle *p1,CCloudParticle *p2)
{ return (p1->Distance() < p2->Distance());
}
//=====================================================================================
//  Sort particle toward camera 
//=====================================================================================
bool SortToward(CCloudParticle *p1,CCloudParticle *p2)
{ return (p1->Distance() > p2->Distance());
}
//=====================================================================================
//  Cloud Box:  A cluster of particle to model a puff
//=====================================================================================
CCloudBox::CCloudBox(CVector &l,CVector &u)
{ Nbp   = 0;
  low   = l;
  upr   = u;
  prt   = 0;
}
//------------------------------------------------------------------------
//  Delete all resources
//------------------------------------------------------------------------
CCloudBox::~CCloudBox()
{ if (prt)  delete [] prt;  }
//------------------------------------------------------------------------
//  Check if position is inside the box
//------------------------------------------------------------------------
int CCloudBox::IsInside(CVector &p)
{ if ((p.x < low.x) || (p.x > upr.x))   return 0;
  if ((p.y < low.y) || (p.y > upr.y))   return 0;
  if ((p.z < low.z) || (p.z > upr.z))   return 0;
  return 1;
}
//------------------------------------------------------------------------
//  From the particle list count those inside the box
//------------------------------------------------------------------------
int CCloudBox::CountParticles(int np,CLOUD_PRT *prt)
{ Nbp = 0;
  CLOUD_PRT *part = prt;
  for (int k=0; k < np; k++) {Nbp += IsInside(prt->pos); prt++;}
  return Nbp;
}
//------------------------------------------------------------------------
//  Affect all particles that are inside the box
//  Particle position is adjusted to be relative to the
//  box center.
//------------------------------------------------------------------------
void  CCloudBox::AffectParticles(int np,CLOUD_PRT *allp)
{ int   na = 0;
  //MEMORY_LEAK_MARKER ("CLOUD_PRT")
  prt = new CLOUD_PRT[Nbp];
  //MEMORY_LEAK_MARKER ("CLOUD_PRT")
  CLOUD_PRT *src = allp;
  CLOUD_PRT *dst = prt;
  //--- Compute box center ----------------------------
  ofs.x   = (low.x + upr.x) * 0.5;
  ofs.y   = (low.y + upr.y) * 0.5;
  ofs.z   = (low.z + upr.z) * 0.5;
  //---Affact particles that are inside ---------------
  for (int k=0; k < np; k++)
  { if (IsInside(src->pos)) 
      {*dst = *src; 
        dst->pos.Subtract(ofs);
        dst++;
        na++;}
    src++;
  }
  bool nak = (na != Nbp);
  if (nak)  gtfo("Incorrect cloud model");
  return;
}

//=====================================================================================
//  Cloud Model:  A set of cloud boxes
//=====================================================================================
CCloudModel::CCloudModel(CCloudSystem *cs)
{ csys   = cs;
  nBox   = 0;
  Radius = 0;
}
//-----------------------------------------------------------------------
//  Free all  boxes in model
//-----------------------------------------------------------------------
CCloudModel::~CCloudModel()
{ std::vector<CCloudBox *>::iterator itb;
  for (itb=Boxes.begin(); itb != Boxes.end(); itb++)
  { CCloudBox *box = (*itb);
    delete box;
  }
  Boxes.clear();
}
//-----------------------------------------------------------------------
//  Divide the model in equidistant boxes
//  raw is the raw set of particles
//  For each box count the particle number
//-----------------------------------------------------------------------
int CCloudModel::Divide(char *raw)
{ int        tot = 0;
  CVector    low = vLow - vInc;
  CVector    upr = vLow;
  CCloudBox *box = 0;
  CLOUD_HDR *hdr = (CLOUD_HDR*)raw;
  CLOUD_PRT *prt = (CLOUD_PRT*)(raw + sizeof(CLOUD_HDR));
  for (int x=0; x<CLOUD_X_DIV; x++)
      { //--advance on X reInit Y --------------------------
        low.x += vInc.x;
        upr.x += vInc.x;
        low.y  = vLow.y - vInc.y;
        upr.y  = vLow.y;
        for (int y=0; y<CLOUD_Y_DIV; y++)
          { //--Advance on Y reinit z ----------------------
            low.y += vInc.y;
            upr.y += vInc.y;
            low.z  = vLow.z - vInc.z;
            upr.z  = vLow.z;
            for (int z=0; z < CLOUD_Z_DIV; z++)
                { low.z += vInc.z;
                  upr.z += vInc.z;
                  //MEMORY_LEAK_MARKER ("CCloudBox")
                  box = new CCloudBox(low,upr);
                  //MEMORY_LEAK_MARKER ("CCloudBox")
                  tot += box->CountParticles(hdr->nbp,prt);
                  if (box->IsEmpty()) {delete box; continue;}
                  Boxes.push_back(box);
                  nBox++;
                  box = 0;
                }
          }
      
   }
  //---Now, affect all particles descriptors to boxes -----
  std::vector<CCloudBox *>::iterator itb;
  for (itb = Boxes.begin(); itb != Boxes.end(); itb++)
  { CCloudBox *box = (*itb);
    box->AffectParticles(hdr->nbp,prt);
  }
  //--- Compute model radius ------------------------------
  Radius     = hdr->radius;
  return tot;
}
//=====================================================================================
//
//  Class Cloud system:  Manage all clouds
//
//=====================================================================================
CCloudSystem::CCloudSystem()
{ globals->cld = this;
  stamp = 0;
  Layer = 0;
  wInd  = 0;
  rInd  = 0;
  Ceil  = 0;
  Test  = 0;
  fTim   = 0;
  fTar   = 0;
  fCof   = 0;
  total[0] = 0;
  total[1] = 0;
  Init();
}

//------------------------------------------------------------------------------
//  Init cloud system if used
//------------------------------------------------------------------------------
void CCloudSystem::Init()
{ DistributionTexture(CLOUD_DIST_TEX_SIZE);
  //---Get cloud parameters ----------------------------------------------------
  nMOD  = 10;
  GetIniVar("Cloud","ModelNumber",&nMOD);
  //---Light Texture size ------------------------------------------------------
  SunTxSize = 32;
  //---Albedo and extinction ---------------------------------------------------
  float ab  = 0.80f;
  GetIniFloat("Cloud","Albedo",&ab);
  Albedo      = ab;
  float ex    = 80;
  GetIniFloat("Cloud","Extinction",&ex);
  Extinction  = ex;
  scFactor    = (Albedo * Extinction * SOLID_ANGLE) / (4 * PI);
  exFactor    = exp(-Extinction);
  //---hFac is the heigth factor defining color difference between top and 
  //        bottom of cloud 
  hFac        = 0.4f;
  GetIniFloat("Cloud","HFactor",&hFac);
  ab          = 2000;
  GetIniFloat("Cloud","Scale",&ab);
  scale       = ab;
  //----Color management -----------------------------------------------------
  sRGB.x = 1.0;                       // sRGB is sunset color
  sRGB.y = 0.6f;                      // A dash of red and orange
  sRGB.z = 0.4f;
  nRGB.x  = 1.0f;
  nRGB.y  = 1.0f;
  nRGB.z  = 1.0f;
  white[0] = white[1] = white [2] = white[3] = 255;
  //--------------------------------------------------------------------------
  tcm     = globals->tcm;
  hFeet   = tcm->GetMedDist() * 0.7;
  //--------------------------------------------------------------------------
  int rdn = (globals->clk->GetMinute() << 8) +  globals->clk->GetSecond();
  srand(rdn);
  //--------------------------------------------------------------------------
  space   = 48000;
  //----Load cloud models ----------------------------------------------------
  LoadCloudModels();
  //---Load Flat textures -----------------------------------------------------
  sTex[0] =   LoadTexture('1');
  sTex[1] =   LoadTexture('2');
  sTex[2] =   LoadTexture('3');
  //-----ALLOCATE THE FBO OBJECT ---------------------------------------------
  FBO = 0;
  DEP = 0;
  //-----ALLOCATE A RENDER BUFFER ---------------------------------------------
  glGenFramebuffersEXT (1,&FBO);
  glGenRenderbuffersEXT(1,&DEP);
  glGenRenderbuffersEXT(1,&IMP);
  //---Reset normal rendering to frame buffer --------------------------------
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
  return;
}
//------------------------------------------------------------------------------
//  Free all resources
//------------------------------------------------------------------------------
CCloudSystem::~CCloudSystem()
{ glDeleteTextures(1,&pTex);
  glDeleteTextures(3, sTex);
  glDeleteFramebuffersEXT (1,&FBO);
  glDeleteRenderbuffersEXT(1,&DEP);
  glDeleteRenderbuffersEXT(1,&IMP);
  //----Delete all cloud puffs -----------------
  std::vector<CCloudPuff*>::iterator ita;
  for (ita=Puffs.begin();ita!=Puffs.end();ita++)
  { CCloudPuff *clp = (*ita);
    delete clp;
  }
	Puffs.clear();
  //----Delete cloud models --------------------
  std::vector<void *>::iterator itr;
  for (itr=RawSet.begin();itr!=RawSet.end();itr++)
  { void *mdl = (*itr);
    delete [] mdl;
  }
	RawSet.clear();
  //---Delete model box ------------------------
  std::vector<CCloudModel *>::iterator itm;
  for (itm=Models.begin();itm!=Models.end();itm++)
  { CCloudModel *mod = (*itm);
    delete mod;
  }
	Models.clear();
  //--------------------------------------------
  return;
}
//------------------------------------------------------------------------------
//  Compute sunset cloud color
//  Sun color must be proportional to sun elevation and cloud distance
//------------------------------------------------------------------------------
void CCloudSystem::SunsetColor(CVector &v,CVector &col)
{ //----- Compute particle distance to sun -------
  if (!SunsetTime())          return;
  double dp = sunP.DistanceTo(v);
  if (dp > sunT)              return;
  //----- Compute sunset color ------------------
  double fa = 3.33 * abs((sunO.z + 0.2) - 0.3);
  double cp = (1 - fa);
  col.x = (cp * sRGB.x) + fa;
  col.y = (cp * sRGB.y) + fa;
  col.z = (cp * sRGB.z) + fa;
  return;
}

//------------------------------------------------------------------------------
//  LoadTexture
//------------------------------------------------------------------------------
GLuint CCloudSystem::LoadTexture(char k)
{ char  key[PATH_MAX];
  char  pat[PATH_MAX];
  _snprintf(key,255,"SKY0%c.TIF",k);
  _snprintf(pat,255,"CLOUDS/%s",key);
  pAddDisk(&globals->pfs,pat,pat);
  CArtParser  img(0);
  img.LoadFFF(pat,1,FIF_TIFF);
  U_CHAR *tx  = img.TransferRGB();
  int wid     = img.GetWidth();
  int htr     = img.GetHeigth();
  return OneTexture(GL_INTENSITY,wid,htr,tx);
}
//------------------------------------------------------------------------------
//  Create the distribution texture
//------------------------------------------------------------------------------
void CCloudSystem::DistributionTexture(int n)
{ nTex    = n;
  int     dim = 4 * n * n;
  //MEMORY_LEAK_MARKER ("cloud_tex")
  U_CHAR *tex = new U_CHAR[dim];
  //MEMORY_LEAK_MARKER ("cloud_tex")
  float   X,Y,Dist;
	float   Incr = (2.0f / n);
	int i = 0, j = 0;
	float   value;
	Y = -1.0f;
	for (int y = 0; y < n; y++)
	{	X = -1.0f;
		for (int x=0; x<n; x++, i++, j+=4)
		{	Dist = float(SquareRootFloat(X*X+Y*Y));
			if (Dist > 1) Dist=1;
			//our magical interpolation polynomical
			value  = 2*Dist*Dist*Dist - 3*Dist*Dist + 1;
			value *= 0.4f;
			tex[j+3] = tex[j+2] = tex[j+1] = tex[j] = (U_CHAR)(value * 255);
			X+=Incr;
		}
		Y+=Incr;
	}
  //----------------------------
  pTex = OneTexture(GL_RGBA,n,n,tex);
  return;
}
//------------------------------------------------------------------------------
//  Clear the initial parts
//------------------------------------------------------------------------------
void CCloudSystem::ClearPart()
{ for (U_INT k=0; k<Parts.size();k++)
  { CCloudParticle *prt = Parts[k];
    delete prt;
  }
  Parts.clear();
  return;
}
//------------------------------------------------------------------------------
//  Load all models in memory for fast creation
//------------------------------------------------------------------------------
void CCloudSystem::LoadCloudModels()
{ for (int k=0; k < 20; k++)  if (k != 9) LoadModel(k); 
}
//------------------------------------------------------------------------------
//  Init model in memory for fast creation
//------------------------------------------------------------------------------
void CCloudSystem::LoadModel(int No)
{ FILE* fp = 0;
	char strname[MAX_PATH];
  _snprintf(strname,255, "CLOUDS/CLOUD%d.dat", No);
  fp = fopen(strname, "rb");
	if (!fp) return;
  //---Get number of particles ----------------------------
  int nb;
	fread(&nb, sizeof(int), 1, fp);
  //---Allocate and load particles table ------------------
  int   dim = sizeof(CLOUD_HDR) + (nb * sizeof(CLOUD_PRT));
  char *tab = new char[dim];
  char *mod = tab;
  //---Set Header address ---------------------------------
  CLOUD_HDR  *hdr = (CLOUD_HDR*)tab;
  hdr->pxt.x      = 0;
  hdr->pxt.y      = 0;
  hdr->pxt.z      = 0;
  hdr->nxt.x      = 0;
  hdr->nxt.y      = 0;
  hdr->nxt.z      = 0;
  //---Read Particle positions ----------------------------
  for (int k=0; k<nb; k++)  ReadPartPosition(k,fp);
  //---Read Particle size ---------------------------------
  for (int k=0; k<nb; k++)  ReadPartSize(k,fp);
  //---Sort on Y dimension --------------------------------
  std::sort(Parts.begin(),Parts.end(),SortModel);
  //---Set Particle address -------------------------------
  tab += sizeof(CLOUD_HDR);
  CLOUD_PRT  *part = (CLOUD_PRT*)tab;
  float rad = 0;
  //---Read parts -----------------------------------------
  std::vector<CCloudParticle *>::iterator pt;
  int a = Parts.size();
  for (pt = Parts.begin(); pt != Parts.end(); pt++) 
  { CCloudParticle *prt = (*pt);
    part->ID    = prt->ID;
    part->pos   = prt->ofs;
    part->size  = prt->size;
    //---Compute distance from center ----
    float mag = part->pos.FastLength();
    if (mag > rad) rad = mag;
    UpdateExtend(hdr,part->pos);
    part++;
  }
  hdr->nbp    = nb;
  hdr->radius = rad;
  ClearPart();
  //---Compute H FACTOR -----------------------------------
  hdr->hFac = hFac / (hdr->pxt.z - hdr->nxt.z);
  //---Store model ----------------------------------------
  RawSet.push_back(mod);
  fclose(fp);
  DivideModel(No,mod);
  return;
}
//------------------------------------------------------------------------------
//  Update cloud extension
//------------------------------------------------------------------------------
void CCloudSystem::UpdateExtend(CLOUD_HDR *hdr,CVector &pos)
{ //---Update positive extensions -------------------
  if (pos.x > hdr->pxt.x) hdr->pxt.x = pos.x;
  if (pos.y > hdr->pxt.y) hdr->pxt.y = pos.y;
  if (pos.z > hdr->pxt.z) hdr->pxt.z = pos.z;
  //---Update negative extensions -------------------
  if (pos.x < hdr->nxt.x) hdr->nxt.x = pos.x;
  if (pos.y < hdr->nxt.y) hdr->nxt.y = pos.y;
  if (pos.z < hdr->nxt.z) hdr->nxt.z = pos.z;
  return;
}
//------------------------------------------------------------------------------
//  Read Particle into table
//  -Set particle position from cloud center
//  -Exchange Y and -Z axis for handedness
// After that Z is altitude
//------------------------------------------------------------------------------
/*
float CCloudSystem::ReadPart(U_INT k,FILE *fp,CLOUD_PRT *tab)
{ //---Create a particle -----------------
  float   mag;
  Vector3 pos;
  fread(&pos,sizeof(Vector3), 1, fp);
  tab->ID  = k;
  tab->pos.x = +pos.x * scale;
  tab->pos.y = -pos.z * scale;
  tab->pos.z = +pos.y * scale;
  //--------------------------------------------
  CCloudParticle *prt = new CCloudParticle();
  prt->ID    = k;
  prt->ofs.x = +pos.x * scale;
  prt->ofs.y = -pos.z * scale;
  prt->ofs.z = +pos.y * scale;
  Parts.push_back(prt);
  //-----Compute distance from cloud center -----
  mag = tab->pos.FastLength();
  return mag;
}
*/
//------------------------------------------------------------------------------
//  Read Particle position
//  -Set particle position from cloud center
//  -Exchange Y and -Z axis for handedness
// After that Z is altitude
//------------------------------------------------------------------------------
void CCloudSystem::ReadPartPosition(U_INT k,FILE *fp)
{ //---Create a particle -----------------
  Vector3 pos;
  fread(&pos,sizeof(Vector3), 1, fp);
  //--------------------------------------------
  CCloudParticle *prt = new CCloudParticle();
  prt->ID    = k;
  prt->ofs.x = +pos.x * scale;
  prt->ofs.y = -pos.z * scale;
  prt->ofs.z = +pos.y * scale;
  prt->dist  = 0;
  prt->color = 0;
  Parts.push_back(prt);
  return;
}
//------------------------------------------------------------------------------
//  Read Particle size
//  -Set particle position from cloud center
//  -Exchange Y and -Z axis for handedness
// After that Z is altitude
//------------------------------------------------------------------------------
void CCloudSystem::ReadPartSize(U_INT k,FILE *fp)
{ float siz;
  fread(&siz, sizeof(float), 1, fp);
  if (siz < 0.5) siz += 0.5;
  CCloudParticle *prt = Parts[k];
  prt->size = (siz * scale);
  return;
}
//------------------------------------------------------------------------------
//  Divide model in cloud boxes
//------------------------------------------------------------------------------
void CCloudSystem::DivideModel(int No,char *raw)
{ CLOUD_HDR *hdr = (CLOUD_HDR*)raw;
  vInc.x  = (hdr->pxt.x - hdr->nxt.x) / CLOUD_X_DIV;
  vInc.y  = (hdr->pxt.y - hdr->nxt.y) / CLOUD_Y_DIV;
  vInc.z  = (hdr->pxt.z - hdr->nxt.z) / CLOUD_Z_DIV;
  CCloudModel *mod = new CCloudModel(this);
  mod->SetBase(hdr->nxt);
  mod->SetIncr(vInc);
  mod->Divide(raw);
  Models.push_back(mod);
  //-- TODO: Delete the raw list of particles -------- 
  return;
}
//------------------------------------------------------------------------------
//  Allocate a  texture object to compute particle color
//------------------------------------------------------------------------------
GLuint CCloudSystem::SunTexture(int wd)
{	GLuint obj;
  glGenTextures(1, &obj);
	glBindTexture(GL_TEXTURE_2D, obj);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wd, wd, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  return obj;
}
//------------------------------------------------------------------------------
//  Allocate a  texture
//------------------------------------------------------------------------------
GLuint CCloudSystem::OneTexture(U_INT type,int wd,int ht,U_CHAR *tx)
{	GLuint obj;
  glGenTextures(1, &obj);
	glBindTexture(GL_TEXTURE_2D, obj);
	glTexImage2D(GL_TEXTURE_2D, 0, type, wd, ht, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, tx);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  if (tx) delete [] tx;
  return obj;
}
//------------------------------------------------------------------------------
//  Init FBO for rendering sun texture 
//------------------------------------------------------------------------------
GLuint CCloudSystem::SunFBO(GLuint txo)
{ int dim = SunTxSize;
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);

  glBindRenderbufferEXT   (GL_RENDERBUFFER_EXT,DEP);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT,dim,dim);
  glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT,
                                GL_DEPTH_ATTACHMENT_EXT,
                                GL_RENDERBUFFER_EXT,
                                DEP);

  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                            GL_COLOR_ATTACHMENT0_EXT,
                            GL_TEXTURE_2D,
                            txo, 0);
  GLenum stat = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  GLenum ok   = GL_FRAMEBUFFER_COMPLETE_EXT;
  if (ok != stat)
    ok = ok;

  return FBO;
}
//------------------------------------------------------------------------------
//  Init FBO for rendering impostor texture 
//------------------------------------------------------------------------------
GLuint CCloudSystem::ImpFBO(GLuint tex,int dim)
{ glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);

  glBindRenderbufferEXT   (GL_RENDERBUFFER_EXT,IMP);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT,dim,dim);
  glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT,
                                GL_DEPTH_ATTACHMENT_EXT,
                                GL_RENDERBUFFER_EXT,
                                IMP);

  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                            GL_COLOR_ATTACHMENT0_EXT,
                            GL_TEXTURE_2D,
                            tex, 0);
  GLenum stat = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  GLenum ok   = GL_FRAMEBUFFER_COMPLETE_EXT;
  if (ok != stat)
    ok = ok;
  return FBO;
}
//------------------------------------------------------------------------------
//  Get a random value in arcsecond
//  fac is in [1.0-1.99]
//  sd is the scale factor
//------------------------------------------------------------------------------
double CCloudSystem::RandValue(float sd)
{ int     rdm = RandomNumber(100);
  double  fac = 1.0 + (float(rdm) * 0.01);
  double  arc = FN_ARCS_FROM_FEET(sd * fac);
  return  WrapArcs(arc);
}
//------------------------------------------------------------------------------
//  Generate new clouds for test above aircraft
//------------------------------------------------------------------------------
Tag CCloudSystem::GenTestCloud(C_QGT *qt,Tag kid)
{ if (Test > 1 )      return kid;
  SPosition   pos;
  CCamera *cc = globals->cam;
  cc->GetPosition(pos);
  pos.lat += 2500;
  C_QGT       *qgt = globals->tcm->GetQGT(pos);
  if (0 == qgt)   return kid;
  CCloudPuff  *cld = new CCloudPuff(this,CLOUD_M,pos,qgt);
  Puffs.push_back(cld);
  Test++;
  return kid;
}
//------------------------------------------------------------------------------
//  Generate new clouds on behalf the QGT
//------------------------------------------------------------------------------
#define CLOUD_MAX_PER_QGT 3
//------------------------------------------------------------------------------
Tag CCloudSystem::GenerateCloud(C_QGT *qgt,Tag kid)
{ if (globals->noMET)   return kid;
  //----Get the Metar area ----------------------
  CMeteoArea *ma  = globals->wtm->GetMetar(kid);
  if (0 == ma)          return 0;
  if (ma->NoLayer())    return kid;
  //----Extract parameters ---------------------
  Layer     = ma->GetLayer();       
  Ceil      = ma->GetCeil();  
  //----Generate cloud if needed ---------------
  U_INT msk = CLOUD_GEN_DIM - 1;
  SPosition pos = {0,0,Ceil};
  CCloudPuff *puf = 0;
  SPosition *qmp = qgt->GetMidPoint();
  //--  First cloud at west mid side -------
  U_INT nw = qgt->NbCloud(CLOUD_W);
  if ((nw < CLOUD_MAX_PER_QGT) && layTAB[Layer].typ0)
  { pos.lon  = AddLongitude(qgt->GetWesLon(),RandValue(space));
    pos.lat  = qmp->lat - RandValue(space);
    puf = new CCloudPuff(this,CLOUD_W,pos,qgt);
    std::vector<CCloudPuff*>::iterator cl = Puffs.begin();
    Puffs.insert(cl,puf);
  }
  //--- Second cloud a north mid side ------
  U_INT nn = qgt->NbCloud(CLOUD_N);
  if ((nn < CLOUD_MAX_PER_QGT) && layTAB[Layer].typ1)
  { pos.lon  = AddLongitude(qmp->lon,RandValue(space));
    pos.lat  = qgt->GetNorLat() - RandValue(space);
    puf = new CCloudPuff(this,CLOUD_N,pos,qgt);
    std::vector<CCloudPuff*>::iterator cl = Puffs.begin();
    Puffs.insert(cl,puf);
  }
  //--- Third cloud -at mid point  ----------
  U_INT nm = qgt->NbCloud(CLOUD_M);
  if ((nm < CLOUD_MAX_PER_QGT) && layTAB[Layer].typ2)
  { pos.lon = AddLongitude(qmp->lon,RandValue(space));
    pos.lat = qmp->lat - RandValue(space);
    puf = new CCloudPuff(this,CLOUD_M,pos,qgt);
    std::vector<CCloudPuff*>::iterator cl = Puffs.begin();
    Puffs.insert(cl,puf);
  }
  //------------------------------------------
  return kid;
}
//------------------------------------------------------------------------------
//  Time slice cloud system
//  Clouds that reach outside circle disappears
//  
//------------------------------------------------------------------------------
void CCloudSystem::TimeSlice(float dT,U_INT FrNo)
{ Frame     = FrNo;
  acp       = tcm->PlaneArcsPos();
  sTic      = globals->clk->GetSecond();
  //--- Check for cloud ready -----------------
  if (0 == Puffs.size())        return;
  //--- Set Camera world position -------------
  cCam    = globals->cam;
  cCam->AbsoluteFeetPosition(camP);
  //----Set Sun position and orientation ------
  sunO    = *tcm->SunPosition();
  sunO.Normalize();                     // Sun vector from origin
  sunP    = *tcm->SunPosition();
  sunT    =  sunP.DistanceTo(camP) - hFeet ;

  //---Update cloud state ---------------------
  std::vector<CCloudPuff *>::iterator cl;
  for (cl = Puffs.begin(); cl != Puffs.end(); cl++)
  { CCloudPuff *puf = *cl;
    if (puf->Update(cCam))  continue;
    if (puf->IsAlive())     continue;
    //---Delete this cloud -------------------
    Puffs.erase(cl);
    delete puf;
    break;
  }
  //---Sort every 4 sec for rendering ---------------------
  if (sTic & 0x3) return;
  std::sort(Puffs.begin(), Puffs.end(), SortPuffsToCamera);
  return;
}
//------------------------------------------------------------------------------
//  Kill all clouds
//------------------------------------------------------------------------------
void CCloudSystem::KillClouds()
{ std::vector<CCloudPuff *>::iterator cl;
  for (cl = Puffs.begin(); cl != Puffs.end(); cl++) (*cl)->Disappear();
  return;
}
//------------------------------------------------------------------------------
//  Check time of day against sun orientation
//------------------------------------------------------------------------------
bool CCloudSystem::SunsetTime()
{ if (sunO.x >  0)      return false;
  if (sunO.z >  0.4)    return false;
  if (sunO.z < -0.2)    return false;
  return true;
}
//------------------------------------------------------------------------------
//  Draw some clouds
//------------------------------------------------------------------------------
void CCloudSystem::Draw()
{ if (globals->noMET)     return;
  if (0 == Puffs.size())  return;
  total[0] = 0;
  total[1] = 0;
  //---Context for cloud rendering -------------------
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
  SetDrawState();
  std::vector<CCloudPuff *>::iterator cl;
  for (cl = Puffs.begin(); cl != Puffs.end(); cl++)  total[(*cl)->Draw()]++;

  glPopClientAttrib();
  glPopAttrib();
  return;
}
//------------------------------------------------------------------------------
//  Set drawing conditions
//------------------------------------------------------------------------------
void CCloudSystem::SetDrawState()
{ glDisable(GL_LIGHTING);                      // LIGHT OFF
  glEnable(GL_BLEND);
  glDepthMask(GL_FALSE);
  glShadeModel(GL_SMOOTH);
  glDisable(GL_CULL_FACE);
  glPolygonMode(GL_FRONT,GL_FILL);
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_FOG);
  return;
}
//------------------------------------------------------------------------------
//  Set color with alpha value
//------------------------------------------------------------------------------
inline void CCloudSystem::SetColor(U_CHAR alf)
{ white[3]  = alf;
  glColor4ubv(white);
  return;
}
//=====================================================================================
//  Class Cloud Particle
//=====================================================================================
//-----------------------------------------------------------------------------
//  Set Particule position
//-----------------------------------------------------------------------------
void CCloudParticle::SetPosition(Vector3 &v)
{ ofs.x =   v.x;
  ofs.y =  -v.z;
  ofs.z =   v.y;
}
//-----------------------------------------------------------------------------
//  Rotate particle around Y
//-----------------------------------------------------------------------------
void CCloudParticle::RotateY(double ca,double sa,V_PART *tab)
{ CVector np;
  np.x  = (ofs.x * ca) - (ofs.y * sa);
  np.y  = (ofs.x * sa) + (ofs.y * ca);
  np.z  =  ofs.z;
  MakeVerticalQuad(tab,np);
  return;
}
//-----------------------------------------------------------------------------
//  Build particle QUAD as vertical billboard
//  NOTE:  Particle position is in feet from Puff center
//-----------------------------------------------------------------------------
void CCloudParticle::MakeVerticalQuad(V_PART *tab, CVector &p)
{   //----Build vertex v0 ---(NW corner) ----------------
    tab[0].VT_S = 0;
    tab[0].VT_T = 1;
    tab[0].VT_X = p.x - size;
    tab[0].VT_Y = p.y;
    tab[0].VT_Z = p.z + size;
    //----Build vertex v1 ---(SW corner) ----------------
    tab[1].VT_S = 0;
    tab[1].VT_T = 0;
    tab[1].VT_X = p.x - size;
    tab[1].VT_Y = p.y;
    tab[1].VT_Z = p.z - size;
    //----Build vertex v2 ---(SE corner)------------------
    tab[2].VT_S = 1;
    tab[2].VT_T = 0;
    tab[2].VT_X = p.x + size;
    tab[2].VT_Y = p.y;
    tab[2].VT_Z = p.z - size;

    //----Build vertex v3 ---(NE corner) ----------------
    tab[3].VT_S = 1;
    tab[3].VT_T = 1;
    tab[3].VT_X = p.x + size;
    tab[3].VT_Y = p.y;
    tab[3].VT_Z = p.z + size;

    return;
}
//-----------------------------------------------------------------------------
//  Build particle Horizontal QUAD
//-----------------------------------------------------------------------------
void CCloudParticle::MakeHorizontalQuad(TC_VTAB *cbb,CVector &p)
{   //----Build vertex v1 --(SE corner)------------------
    cbb[0].VT_S = 1;
    cbb[0].VT_T = 0;
    cbb[0].VT_X = p.x - size;
    cbb[0].VT_Y = p.z - size;
    cbb[0].VT_Z = p.y;
    //----Build vertex v2 ---(SW corner) ----------------
    cbb[1].VT_S = 0;
    cbb[1].VT_T = 0;
    cbb[1].VT_X = p.x + size;
    cbb[1].VT_Y = p.z - size;
    cbb[1].VT_Z = p.y;
    //----Build vertex v3 ---(NW corner) ----------------
    cbb[2].VT_S = 0;
    cbb[2].VT_T = 1;
    cbb[2].VT_X = p.x + size;
    cbb[2].VT_Y = p.z + size;
    cbb[2].VT_Z = p.y;
    //----Build vertex v4 ---(NE corner) ----------------
    cbb[3].VT_S = 1;
    cbb[3].VT_T = 1;
    cbb[3].VT_X = p.x - size;
    cbb[3].VT_Y = p.z + size;
    cbb[3].VT_Z = p.y;
    return;
}
//-----------------------------------------------------------------------------
//  Draw a sphere at particle position
//-----------------------------------------------------------------------------
void CCloudParticle::Sphere()
{ glPushMatrix();
  glTranslated(ofs.x,ofs.y,ofs.z);
  GLUquadricObj  *obj = globals->tcm->GetSphere();
  gluSphere(obj,size,16,16);
  glPopMatrix();
  return;
}
//=====================================================================================
#define IMPOST_SIZE 256
//=====================================================================================
//  Class CloudPuffs
//=====================================================================================
CCloudPuff::CCloudPuff(CCloudSystem *cs,U_CHAR type,SPosition &pos,C_QGT *qt)
{ Ident   = cs->GetStamp();
  cType   = type;
  Metar   = qt->GetMETAR();
  Volum   = 0;
  camDIS  = 0;
  csys    = cs;
  cStat   = CLOUD_INITIALIZE;
  pTex    = 0;
  iTex    = 0;
  vtab    = 0;
  dStat   = CLOUD_DRAW_NOT;
  gStat   = CLOUD_INIT;
  alpha   = 0;
  geop    = pos;
  Ceil    = cs->GetCeil();
  geop.alt = Ceil;
  nPart   = 0;
  dPart   = 0;
  visi    = 0;
  impDIS  = globals->tcm->GetMedDist() * 1.4;
  //-------------------------------------------------
  pQGT    = qt;
  qt->IncCloud(cType);
  //-------------------------------------------------
  colr.R  = 1.0f;       //0.9686f;   
  colr.G  = 1.0f;        //0.3803f;
  colr.B  = 1.0f;        //0.1843f;
  colr.A  = 1.0f;
  //----Generate empty textures ---------------------
  sTex    = cs->SunTexture(CLOUD_COLR_BUF_SIDE);
  iTex    = cs->OneTexture(GL_RGBA,IMPOST_SIZE,IMPOST_SIZE,0);
  nfrm    = 0;
  //-------------------------------------------------
  sphere = gluNewQuadric();
  gluQuadricDrawStyle(sphere,GLU_FILL);
  return;
}
//----------------------------------------------------------------------------
//  Free resources
//----------------------------------------------------------------------------
CCloudPuff::~CCloudPuff()
{ std::vector<CCloudParticle*>::iterator ip;
  for (ip = Parts.begin(); ip != Parts.end(); ip++)
  { CCloudParticle *cp = *ip;
    delete cp; //delete cp;
  }
  Parts.clear();
  //----------------------------------------------
  glDeleteTextures(1,&iTex);
  glDeleteTextures(1,&sTex);
  //----------------------------------------------
  if (vtab) delete [] vtab;
  //----------------------------------------------
  gluDeleteQuadric(sphere);
}
//----------------------------------------------------------------------------
//  Create Cloud particles in plan sd,ht
//  sd: Side of the plan
//  ht: Height of the plan
//----------------------------------------------------------------------------
void CCloudPuff::Create()
{ Radius    = 0;
  pTex      = csys->GetParticleTexture();
  if (csys->GetLayer() == 1)  FormFlatCloud();
  else                        FormVoluCloud();
  return;
}
//----------------------------------------------------------------------------
//  Free the QGT and the cloud
//----------------------------------------------------------------------------
int CCloudPuff::Free()
{ cStat = CLOUD_END_OF_LIFE;
  pQGT  = 0;
  return 0;
}
//------------------------------------------------------------------------------
//  Compute phase function
//  lDIR is the light direction for the cloud
//  ed   is the eye direction for the particle
//------------------------------------------------------------------------------
float CCloudPuff::GetPhase(CVector &ed)
{ float rCosAlpha = lDIR.DotProduct(ed);
  return (0.75f * (1.0f + rCosAlpha * rCosAlpha));
}

//------------------------------------------------------------------------------
//  Build a flat layer cloud
//------------------------------------------------------------------------------
void CCloudPuff::FormFlatCloud()
{ Volum      = 0;
  Radius     = 30000;
  extVOL     = 2.0f * Radius;
  CVector    p(0,0,0);
  CCloudParticle *part  = new CCloudParticle();
  p.y         = rand() % 600;
  part->ID    = 0;
  part->ofs   = p;
  part->size  = FN_FEET_FROM_ARCS(csys->RandValue(24000));
  part->MakeHorizontalQuad(cbb,p);
  Parts.push_back(part);
  nPart       = 1;
  dPart       = 1;
  pTex        = csys->GetStratTexture(RandomNumber(3));
  dStat       = CLOUD_DRAW_FLT;
  cStat       = CLOUD_FLAT_INIT;
  return;
}
//------------------------------------------------------------------------------
//  Load a model cloud
//  A variable number of particles are selected depending on the layer model
//  The mask parameter select either 1 out of 2 particle  (0x01)
//                                or 1 out of 4 particles (0x03)
//------------------------------------------------------------------------------
void CCloudPuff::FormVoluCloud()
{ double    alti = Ceil;
  void      *vdm = csys->PickRaw();
  char      *mod = (char*)vdm;
  U_CHAR     lay =  csys->GetLayer();
  CLOUD_HDR *hdr = (CLOUD_HDR*)mod;
  U_CHAR     msk = layTAB[lay].mask;
  mod += sizeof(CLOUD_HDR);
  CLOUD_PRT *prt = (CLOUD_PRT*)mod;
//  Radius      = hdr->radius; 
  Radius      = 0;
  U_INT  npn  = 0;            // New part number
  Volum       = 1;
  double a    = DegToRad(double(RandomNumber(30)));
  double ca   = cos(a);
  double sa   = sin(a);
  //---Init all particles --------------------------
  for (int k=0; k < hdr->nbp; k++)
  { if (k & msk) {prt++; continue;}
    CCloudParticle *part  = new CCloudParticle();
    part->ID    = npn;
    part->ofs   = prt->pos;
    part->ofs.RotateZ(ca,sa);
    part->size  = prt->size; 
    Parts.push_back(part);
    StoreBound(part);
    //---Relocate cloud ceil if needed ------------------------
    double pht = part->ofs.z;
    double alt = pht + geof.z - prt->size;        // Particle altitude
    if (alt < alti) alti = alt; 
    //--- Update cloud radius ---------------------------------
    float mag = part->ofs.FastLength();
    if (mag > Radius) Radius = mag;
    prt++;
    npn++;
  }
  nPart   = npn;
  //----Relocate ceiling --------------------------
  double alt = Ceil - alti;
  geof.z   += alt;
  geop.alt  = geof.z;
  //---Build the cloud Quad -----------------------
  extVOL  = 4.0f * Radius;
  cStat   = CLOUD_NEW_BORN;
  //---Build the QUADs for OpenGL -----------------
  vtab  = new V_PART[nPart << 2]; 
  V_PART *tab         = 0;
  CCloudParticle *pt  = 0;
  for (int k=0; k<nPart; k++)
  { tab = vtab + (k << 2);
    pt  = Parts[k];
    pt->MakeVerticalQuad(tab,pt->ofs);
  }
  //-----------------------------------------------
  hFac  = hdr->hFac;
  return;
}
//------------------------------------------------------------------------------
//  Save cloud extensions
//------------------------------------------------------------------------------
void  CCloudPuff::StoreBound(CCloudParticle *p)
{ double px = p->ofs.x;
  double py = p->ofs.y;
  double pz = p->ofs.z;
  //----Save minimum ---------------------------------------
  if (px < minBB.x) minBB.x = px;
  if (py < minBB.y) minBB.y = py;
  if (pz < minBB.z) minBB.z = pz;
  //----Save maximum ---------------------------------------
  if (px > maxBB.x) maxBB.x = px;
  if (py > maxBB.y) maxBB.y = py;
  if (pz > maxBB.z) maxBB.z = pz;
  return;
}
//------------------------------------------------------------------------------
//  return camera position:  
//  geop :  cloud world coordinates in arcsecond
//  geof :  cloud world coordinates in feet
//  Set the inside indicator when camera is in a cloud.
//------------------------------------------------------------------------------
void CCloudPuff::SetCamPosition(CCamera *cm)
{ FeetCoordinates(geop,geof);                       // geop = Absolute world coordinates in arcsec
  cCam    = cm;
  camPOS  = *csys->CamPosition();
  //---Compute cloud feet coordinates relative to origin ---
  globals->tcm->RelativeFeetTo(geop,geol);
  ctoc    = *cCam->GetOffset()- geol;
  //---Compute camera distance to cloud --------------------
  camDIS  = ctoc.FastLength();
  return;
}
//----------------------------------------------------------------------------
//  Write the cloud texture
//----------------------------------------------------------------------------
void CCloudPuff::WriteCloud(int side,U_INT txo)
{ int     nbp = side * side;
  int     dim = side * side * 4;
  U_CHAR *buf = new U_CHAR[dim];
  glBindTexture(GL_TEXTURE_2D,txo);
  glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,buf);
    //----------------------------------------------------------------
	/*
  {GLenum e = glGetError ();
   if (e != GL_NO_ERROR) 
    WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  }
	*/
  //	glReadPixels(0, 0, side, side, GL_RGBA, GL_UNSIGNED_BYTE, buf);
  U_INT     nzr = 0;
  U_INT  *pix = (U_INT*)buf;
  for (int k=0; k<nbp; k++) 
    nzr |= (*pix++ != 0x00FFFFFF);
  char fn[1024];
  _snprintf(fn,1023,"CLOUDS\\IMPOSTOR%d.BMP",Ident);
  CArtParser img(0);
  if (nzr) img.WriteBitmap(FIF_BMP,fn,side,side,buf);
  delete [] buf;
  return;
}
//----------------------------------------------------------------------------
//  Compute particle distance to sun
//----------------------------------------------------------------------------
void CCloudPuff::SunDist()
{ CCloudParticle *part = 0;
  CVector  wps;
  for (int No=0; No < nPart; No++) 
  { part = Parts[No];
    wps  = part->ofs;
    wps.Add(geof);
    part->dist = wps.SqDistance(sunPOS);
  }
  cStat = CLOUD_SUN_DISTANCE;
  return;
}
//----------------------------------------------------------------------------
//  Sort particles according to distance to sun
//----------------------------------------------------------------------------
void CCloudPuff::SunSort()
{ std::sort(Parts.begin(), Parts.end(), SortAway);
  cPart = 0;                        // Init current particle
  cStat = CLOUD_SUN_INITLITE;
  return;
}
//----------------------------------------------------------------------------
//  Compute Camera distance to particles
//  This step is bypassed because particles are drawed from billboard facing the camera
//----------------------------------------------------------------------------
void CCloudPuff::CamDist()
{ 
  //--- sort particles --------------------------
  CCloudParticle *part = 0;
  CVector  wps;
  for (int No=0; No < nPart; No++) 
  { part = Parts[No];
    wps  = part->ofs;
    wps.Add(geof);
    part->dist = wps.SqDistance(camPOS);
  }
  
  cStat = CLOUD_CAM_DISTANCE;
  return;
}
//----------------------------------------------------------------------------
//  Sort particles to camera position
//  This step is bypassed because particles are drawed from billboard facing the camera
//----------------------------------------------------------------------------
void CCloudPuff::CamSort()
{ std::sort(Parts.begin(), Parts.end(), SortToward);
  cPart = 0;                        // Init current particle
  cStat = CLOUD_CAM_ORDERED;        // Stat = ordered
  return;
}
//----------------------------------------------------------------------------
//  Compute Light from sun
//  Clear the texture buffer
//----------------------------------------------------------------------------
void CCloudPuff::InitSunLight(SVector &sun)
{ U_INT    mask = GL_COLOR_BUFFER_BIT+GL_VIEWPORT_BIT;
  int      txs  = csys->GetLigthTexSize();
 	glPushAttrib(mask);
  //--- Set up FBO BUFFER ------------------------------
  csys->SunFBO(sTex);
  //--- Setup the viewport size as the light texture size
	glViewport(0, 0, txs, txs);
	//--- Clear buffer (full bright) --------------------
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //---Restore all matrix ------------------------------
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
 	glPopAttrib();
  cPart  = 0;
  cStat  = CLOUD_SUN_COMPUTE;
  return;
}
//----------------------------------------------------------------------------
//  Compute Light from sun
//  -Each particle is projected to the frame buffer.  The overall result is a
//   color array where all previous particle absorptions are accounted for.
//  -The particle is colored with the average color of the array.
//----------------------------------------------------------------------------
void CCloudPuff::PartSunLight(SVector &sun)
{ U_INT    mask = GL_COLOR_BUFFER_BIT+GL_VIEWPORT_BIT+GL_TEXTURE_BIT+GL_ENABLE_BIT;
  double   dim  = extVOL;
  int      txs  = csys->GetLigthTexSize();
  float    lum  = globals->tcm->GetLuminosity();
 	glPushAttrib(mask);
  //--- Setup an orthographic projection that fit the cloud exactly
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-dim, +dim, -dim, +dim, extVOL+Radius, extVOL-Radius);
	//--- Setup the camera to lookat the cloud center and be positioned on the sun
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();		
	gluLookAt(sun.x, sun.y, sun.z, geof.x, geof.y, geof.z, 0, 1, 0);
  //--- Setup the viewport size as the light texture size
	glViewport(0, 0, txs, txs);
  //--- Set up FBO BUFFER ------------------------------
  csys->SunFBO(sTex);
  //----Go to cloud center ----------------------------
  glTranslated(geof.x,geof.y,geof.z);
	//--- Extract up and right vector -------------------
	float mat[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mat);
	CVector vx(mat[0], mat[4], mat[8] );
	CVector vy(mat[1], mat[5], mat[9] );	
  CVector sum = (vx + vy);
  CVector sub = (vx - vy);
  //--- Parameters for projection ---------------------
  float  Area  = 0;
  int    Side  = 0;
  float *buf   = csys->GetSplatBuffer();
	double mp[16];
  double mm[16];
	int    vp[4];
  float  avg;
  glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	glGetDoublev(GL_PROJECTION_MATRIX, mp);
	glGetIntegerv(GL_VIEWPORT, vp);	
  //--- Bind the distribution texture -----------------
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, pTex);
	//--- Set blending parameter and alpha test ---------
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_DEPTH_TEST);
  //----------------------------------------------------
  double     scf    = csys->GetScFactor();    // Scattering factor
  double     exf    = csys->GetExFactor();
	int ReadX, ReadY;
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
  //------Compute only a limited number of -Particles -----
  int  No = 0;
  int lim = min(int(nPart),(cPart + CLOUD_MAX_PART));
  for (No=cPart; No < lim; No++)
  { CCloudParticle *pt = Parts[No];
    //--- Compute particle projection coordinates ------
    double CenterX, CenterY, CenterZ;
		gluProject( pt->ofs.x, 
                pt->ofs.y,
                pt->ofs.z,
			          mm, mp, vp, 
                &CenterX, &CenterY, &CenterZ);
    //---Compute area ---------------------------------
    Area = pt->dist * SOLID_ANGLE;
    Side = (int)(SquareRootFloat(Area) * txs / (2 * Radius));
		if (Side < 1) Side =  1;
    if (Side >16) Side = 16; 
    int lgb = (Side * Side);
    //make sure not reading from outside the viewport
		ReadX = (int)(CenterX - (Side >> 1));
		if (ReadX < 0) ReadX = 0;
		ReadY = (int)(CenterY - (Side >> 1));
		if (ReadY < 0) ReadY = 0;
    //--- Read the red component since this is greyscale
		glReadPixels(ReadX, ReadY, Side, Side, GL_RED, GL_FLOAT, buf);
    //--- Compute average --------------------------------
    avg = 0;
		for (int j=0; j<lgb; j++) avg += buf[j];
    avg /= lgb;
		// Light color * 
		// average color from solid angle (sum * solidangle / (pixels^2 * 4pi)) 
		// * albedo * extinction 
		// * rayleigh scattering in the direction of the sun (1.5f) (only for rendering, don't store)
		pt->color = (avg * scf);
		pt->alpha = 1.0f - exf;
    //--- Compute scattering in sun direction ----------
    float pColor   = pt->color * 1.5f;
    glColor4f(pColor,pColor,pColor,(pt->alpha * 1.5f));
  
    //--- Draw the particle on texture -----------------
    double dm  = pt->size;
    glBegin(GL_QUADS);
    CVector v0 = sum;
    v0.Times(-dm);
    v0.Add(pt->ofs);
    glTexCoord2f(0.0f, 1.0f);
		glVertex3f(v0.x,v0.y,v0.z);		

    CVector v1 = sub;
    v1.Times(+dm);
    v1.Add(pt->ofs);
    glTexCoord2f(0.0f, 0.0f); 		
		glVertex3f(v1.x,v1.y,v1.z);

    CVector v2 = sum;
    v2.Times(+dm);
    v2.Add(pt->ofs);
    glTexCoord2f(1.0f, 0.0f); 		
		glVertex3f(v2.x,v2.y,v2.z);

    CVector v3 = sub;
    v3.Times(-dm);
    v3.Add(pt->ofs);
    glTexCoord2f(1.0f, 1.0f); 		
		glVertex3f(v3.x,v3.y,v3.z);	

    glEnd();
    cPart++;
   }
  if (lim == nPart) {cStat = CLOUD_SUN_RELIGHT;}
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
 	glPopAttrib();
  //---Restore all matrix ------------------------------
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
  //--Restart timer ------------------------------------
  u2FR  = 300;
  return;
}
 //----------------------------------------------------------------------------
//  Compute Light from sun
//  NOTE on area computing
//  We need to compute a surface in pixel that is covered by the projection
//  of a given particle onto the light texture.
//  The whole texture of dimension 'd'(32 pixels for instance) is covering 
//  a cloud of radius 'r'.
//  The particle projection is 'p' pixels wide for a projected surface 's'
//  thus 
//  p²/d² = s/(2R)²
//  Thus knowing 'p' we can compute 's'.
//  Knowing 's', we can compute the solid angle A = s / D², where D is 
//  D= distance(sun,particle)
//  NOT USED:  Use PartSunLight()
//----------------------------------------------------------------------------
void CCloudPuff::SunLite(SVector &sun)
{	U_INT    mask = GL_COLOR_BUFFER_BIT+GL_VIEWPORT_BIT+GL_TEXTURE_BIT+GL_ENABLE_BIT;
  double   dim  = extVOL;
  int      txs  = csys->GetLigthTexSize();
  float    lum  = globals->tcm->GetLuminosity() * 1.4;
 	glPushAttrib(mask);
  //--- Setup an orthographic projection that fit the cloud exactly
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-dim, +dim, -dim, +dim, extVOL+Radius, extVOL-Radius);
	//--- Setup the camera to lookat the cloud center and be positioned on the sun
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();		
	gluLookAt(sun.x, sun.y, sun.z, geof.x, geof.y, geof.z, 0, 1, 0);
  //--- Setup the viewport size as the light texture size
	glViewport(0, 0, txs, txs);
	//--- Clear buffer (full bright) --------------------
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //----Go to cloud center ----------------------------
  glTranslated(geof.x,geof.y,geof.z);
	//--- Extract up and right vector -------------------
	float mat[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mat);
	CVector vx(mat[0], mat[4], mat[8] );
	CVector vy(mat[1], mat[5], mat[9] );	
  CVector sum = (vx + vy);
  CVector sub = (vx - vy);
  //--- Parameters for projection ---------------------
  float  Area  = 0;
  int    Side  = 0;
  float *buf   = csys->GetSplatBuffer();
	double mp[16];
  double mm[16];
	int    vp[4];
  float  avg;
  glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	glGetDoublev(GL_PROJECTION_MATRIX, mp);
	glGetIntegerv(GL_VIEWPORT, vp);	
  //--- Bind the distribution texture -----------------
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, pTex);
	//--- Set blending parameter and alpha test ---------
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_DEPTH_TEST);
  //----------------------------------------------------
  double     scf    = csys->GetScFactor();    // Scattering factor
  double     exf    = csys->GetExFactor();
	int ReadX, ReadY;
	glReadBuffer(GL_BACK);
  //--- Process each particle --------------------------
  for (int k=0; k < nPart; k++)
  { CCloudParticle *pt = Parts[k];
    //--- Compute particle projection coordinates ------
    double CenterX, CenterY, CenterZ;
		gluProject( pt->ofs.x, 
                pt->ofs.y,
                pt->ofs.z,
			          mm, mp, vp, 
                &CenterX, &CenterY, &CenterZ);
    //---Compute area ---------------------------------
    Area = pt->dist * SOLID_ANGLE;
    Side = (int)(SquareRootFloat(Area) * txs / (2 * Radius));
		if (Side < 1) Side =  1;
    if (Side >16) Side = 16; 
    int lgb = (Side * Side);
    //make sure not reading from outside the viewport
		ReadX = (int)(CenterX - (Side >> 1));
		if (ReadX < 0) ReadX = 0;
		ReadY = (int)(CenterY - (Side >> 1));
		if (ReadY < 0) ReadY = 0;
    //--- Read the red component since this is greyscale
		glReadPixels(ReadX, ReadY, Side, Side, GL_RED, GL_FLOAT, buf);
    //--- Compute average --------------------------------
    avg = 0;
		for (int j=0; j<lgb; j++) avg += buf[j];
    avg /= lgb;
		// Light color * 
		// average color from solid angle (sum * solidangle / (pixels^2 * 4pi)) 
		// * albedo * extinction 
		// * rayleigh scattering in the direction of the sun (1.5f) (only for rendering, don't store)
		pt->color = (avg * scf);
		pt->alpha = 1.0f - exf;
    //--- Compute scattering in sun direction ----------
    float pColor   = pt->color * 1.5f;
    glColor4f(pColor,pColor,pColor,(pt->alpha * 1.5f));
  
    //--- Draw the particle on texture -----------------
    double dm  = pt->size;

    glBegin(GL_QUADS);

    CVector v0 = sum;
    v0.Times(-dm);
    v0.Add(pt->ofs);
    glTexCoord2f(0.0f, 1.0f);
		glVertex3f(v0.x,v0.y,v0.z);		

    CVector v1 = sub;
    v1.Times(+dm);
    v1.Add(pt->ofs);
    glTexCoord2f(0.0f, 0.0f); 		
		glVertex3f(v1.x,v1.y,v1.z);

    CVector v2 = sum;
    v2.Times(+dm);
    v2.Add(pt->ofs);
    glTexCoord2f(1.0f, 0.0f); 		
		glVertex3f(v2.x,v2.y,v2.z);

    CVector v3 = sub;
    v3.Times(-dm);
    v3.Add(pt->ofs);
    glTexCoord2f(1.0f, 1.0f); 		
		glVertex3f(v3.x,v3.y,v3.z);	

    glEnd();
   }


 	glPopAttrib();
  //---Restore all matrix ------------------------------
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
  cStat = CLOUD_SUN_RELIGHT;
  return;
}
//----------------------------------------------------------------------------
//  Check for cloud life
//  After changing position, the owning QGT is computed
//  Cloud will be deleted if
//  1) The new owning QGT is 0
//  2) The new Owning QGT is in delete status
//  3) The new owning QGT is not from the same matar area
//----------------------------------------------------------------------------
int CCloudPuff::CheckOwner()
{ //--- Decrease count for actual QGT ---------------------------
  C_QGT *qt = (C_QGT*)pQGT.Obj();           // Previous QGT         
  qt->DecCloud(cType);
  //--- Update cloud accounting in new QGT ----------------------
  pQGT      = globals->tcm->GetQGT(geop);   // New QGT
  qt        = (C_QGT*)pQGT.Obj();
  if (0 == qt)              return Free();
  if (qt->NotAlive())       return Free();
  if (qt->DifMETAR(Metar))  return Free();
  //---Still alive. Assign cloud to QGT -------------------------
  qt->IncCloud(cType);
  return 1;
}
//----------------------------------------------------------------------------
//  Auto destruction
//----------------------------------------------------------------------------
void CCloudPuff::Disappear()
{ cStat = CLOUD_END_OF_LIFE;
  C_QGT *qt  = (C_QGT*)pQGT.Obj();           // owning QGT 
  if (qt) qt->DecCloud(cType);
  pQGT      = 0;
  return;
}
//----------------------------------------------------------------------------
//  Check for relight
//  Check for cloud accounting.
//  When cloud is moving under wind, the owning QGT may change 
//  -Decrease count in previous QGT
//  -Increase count in next QGT
//  When a cloud has no QGT, it means that the cloud is outside the cache.
//  It is then deleted
//----------------------------------------------------------------------------
int CCloudPuff::Refreshing() 
{ if (!CheckOwner())      return 0;
  U_INT cf = csys->GetFrame();
  //---Refresh eye scattering every n frames ------------
  if (cf < u1FR)          return 1;
  cStat = CLOUD_SUN_RELIGHT;
  if (u2FR--)             return 1;
  cStat = CLOUD_NEW_BORN;
  return 1;
}
//----------------------------------------------------------------------------
//  Relight the cloud Particle
//  -Color of each particle is recalculated from the camera position
//   by projecting all previous particles to compute light absorption
//----------------------------------------------------------------------------
void CCloudPuff::RelightCloud()
{ float    lum  =  globals->tcm->GetLuminosity()* 1.4f;
  CVector  col  = *csys->GetNormalColor();
  //-------------------------------------------------------
  csys->SunsetColor(geof,col);
  CCloudParticle *pt;                   // Particle
  CVector     eye = ctoc;
  CVector     pos;
  V_PART *tab = 0;
  eye.Normalize();
  float phase = GetPhase(eye);
  lDIR.NormeDirection(geof,sunPOS);
  //------Compute only a limited number of -Particles -----
  int  No = 0;
  int lim = min(int(nPart),(cPart + CLOUD_MAX_PART));
  for (No=cPart; No < lim; No++)
  { pt  = Parts[No];
    tab = vtab + (pt->GetNo() << 2);
  //  pos = geof;                         // Cloud center
  //  pos.Add(pt->ofs);                   // Particle offset
  //  eye.NormeDirection(camPOS,pos);     // Camera direction
  //  float phase  = GetPhase(eye);
    //--RGB is defined for the cloud by colr variable -----
    float  C  = (lum * pt->color * phase) + 0.2f;                          
           C += (hFac * pt->ofs.z);
    U_CHAR R1 = ByteColor(C * col.x);
    U_CHAR G1 = ByteColor(C * col.y);
    U_CHAR B1 = ByteColor(C * col.z);
		U_CHAR A  = ByteColor(pt->alpha);
    U_CHAR U  = ByteColor(C);
    //----Color vertex v4 ---(NE corner) ----------------
    tab->VT_R = U;
    tab->VT_G = U;
    tab->VT_B = U;    
    tab->VT_A = A;
    tab++;
    //----Color vertex v3 ---(NW corner) ----------------
    tab->VT_R = U;
    tab->VT_G = U;
    tab->VT_B = U;
    tab->VT_A = A;
    tab++;
    //----Color vertex v2 ---(SW corner) ----------------
    tab->VT_R = R1;
    tab->VT_G = G1;
    tab->VT_B = B1;
    tab->VT_A = A;
    tab++;
    //----Color vertex v1 --(SE corner)------------------
    tab->VT_R = R1;
    tab->VT_G = G1;
    tab->VT_B = B1;
    tab->VT_A = A;
    tab++;
    cPart++;
  }
  //---When all particles are processed, the cloud is drawable --
  if (lim == nPart) cStat = CLOUD_IS_ALIGHTED;
  return;
}
//----------------------------------------------------------------------------
//  Billboard matrix
//  This matrix face the cloud in the XY Plane and rotate result
//----------------------------------------------------------------------------
void CCloudPuff::GetShortBB()
{ double mat[16];
  glGetDoublev(GL_MODELVIEW_MATRIX , mat);
  // undo all rotations
  //----- beware all scaling is lost as well ---------
  mat[0]  = +1;
  mat[1]  =  0;
  mat[2]  =  0;
  mat[4]  =  0;
  mat[5]  =  0;
  mat[6]  = -1;
  mat[8]  =  0;
  mat[9]  = +1;
  mat[10] = 0;
  glLoadMatrixd(mat);
  return;
}
//----------------------------------------------------------------------------
//  Build impostor
//  geol is the local cloud coordinates (relative to aircraft = (0,0,0))
//  hp is the contact point betweeen circle of radius R and frustum tangent.
//  fa is the half height of the near clipping plane
//----------------------------------------------------------------------------
int CCloudPuff::BuildImpostor(int vp)
{ double     dis  =  geol.FastLength();   //camDIS;
  double     cdc  = (dis * dis);
  double     rdc  = (Radius * Radius);
  double     ca   = (dis - Radius);
  double     cb   = (dis + Radius);
  double     hpc  = (cdc - rdc);
  double     tgc  = (rdc / hpc);
  if (tgc < 0)    return 0;
  //---Compute near clip  side (projection plane) ---------------
  double     fa   = SquareRootFloat( ca * ca * tgc);
  double     fg   = (fa * dis) / ca;
  //-------------------------------------------------------------
  csys->ImpFBO(iTex,IMPOST_SIZE);
  //---- Save attributes    -------------------------------------
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  //---- Camera parameters --------------------------------------
  CVector  up(0,0,1);
  //----Set Frustum according to fa -----------------------------
  glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
  //----Set Model view ------------------------------------------
  glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
  //-------------------------------------------------------------
  glFrustum(-fa,+fa,-fa,+fa,ca,cb);
  gluLookAt(ctoc.x,ctoc.y,ctoc.z,0,0,0,up.x,up.y,up.z);
  SetTrueRotation();
  //--- Set Viewport the size of texture ------------------------
	glViewport(0, 0, vp, vp);
	glClearColor(1,1,1,0);
	glClear(GL_COLOR_BUFFER_BIT+GL_DEPTH_BUFFER_BIT);
  //---- Drawing parameters -----------------------------
  glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
  csys->SetDrawState();
  //glColor4f(1,1,1,1);
	ColorGL(COLOR_WHITE);
  //-------------------------------------------------------
  glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
  glBindTexture(GL_TEXTURE_2D,pTex);
  glInterleavedArrays(GL_T2F_C4UB_V3F,0,vtab);
  glDrawArrays(GL_QUADS,0,(nPart << 2));
//DrawParticles();
  //---Write texture for test ----------------------------
//WriteCloud(vp,iTex);    

  //-----------------------------------------------------
  glPopAttrib();
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,0);
  glPopClientAttrib();
  //-----------------------------------------------------
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	/*
    {GLenum e = glGetError ();
   if (e != GL_NO_ERROR) 
    WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  }
	*/
  //--- Compute impostor QUAD in XZ plane --------------
  //---NW border -----------------------------
  impTAB[0].VT_S = 1;
  impTAB[0].VT_T = 1;
  impTAB[0].VT_X = - fg;
  impTAB[0].VT_Y = 0;
  impTAB[0].VT_Z = + fg;
  //---SW border -----------------------------
  impTAB[1].VT_S = 1;
  impTAB[1].VT_T = 0;
  impTAB[1].VT_X = - fg;
  impTAB[1].VT_Y = 0;
  impTAB[1].VT_Z = - fg;
  //---SE border -----------------------------
  impTAB[2].VT_S = 0;
  impTAB[2].VT_T = 0;
  impTAB[2].VT_X = + fg;
  impTAB[2].VT_Y = 0;
  impTAB[2].VT_Z = - fg;
  //---NE border -----------------------------
  impTAB[3].VT_S = 0;
  impTAB[3].VT_T = 1;
  impTAB[3].VT_X = + fg;
  impTAB[3].VT_Y = 0;
  impTAB[3].VT_Z = + fg;

  //------------------------------------------
  return 1;
}
//----------------------------------------------------------------------------
//  First Transition:  Fade in impostor
//----------------------------------------------------------------------------
int CCloudPuff::CloudFirst()
{ dStat  = CLOUD_DRAW_VOL;
  gStat  = CLOUD_INSIDE;
  //----Check distance ---------------------------
  if (camDIS < impDIS)   return 1;
  //----Change to impostor cloud ---------------------
  BuildImpostor(IMPOST_SIZE);
  dStat = CLOUD_DRAW_IMP;
  gStat = CLOUD_REMOTE;
  return 1;
}
//----------------------------------------------------------------------------
//  Cloud is outside of inner ring:
//  -Draw as 2D impostor
//  Cloud is inside inner ring:
//  -Draw as 3D impostor
//----------------------------------------------------------------------------
int CCloudPuff::CloudRemote()
{ BuildImpostor(IMPOST_SIZE);
  dStat = CLOUD_DRAW_IMP;
  //----Check distance ---------------------------
  if (camDIS > impDIS) return 1;
  //----Change to real cloud ---------------------
  dStat = CLOUD_DRAW_VOL;
  gStat = CLOUD_INSIDE;
  return 1;
}
//----------------------------------------------------------------------------
//  Cloud is outside of inner ring:
//  -Draw as 2D impostor
//  Cloud is inside inner ring:
//  -Draw as 3D impostor
//----------------------------------------------------------------------------
int CCloudPuff::CloudInside()
{ dStat  = CLOUD_DRAW_VOL;
  //----Check distance ---------------------------
  if (camDIS < impDIS) return 1;
  //----Change to impostor cloud -----------------
  BuildImpostor(IMPOST_SIZE);
  alpha = 255;
  dStat = CLOUD_DRAW_IMP;
  gStat = CLOUD_REMOTE;
  return 1;
}
//----------------------------------------------------------------------------
//  Time slice
//  Process according to cloud State
//----------------------------------------------------------------------------
int CCloudPuff::Update(CCamera *cam)
{ //---Move cloud according to wind ----
  double dtx  = globals->wtm->GetWindXcomposite(); 
  geop.lon    = AddLongitude(geop.lon,dtx);
  geop.lat   += globals->wtm->GetWindYcomposite();
  //-----------------------------------
  SetCamPosition(cam);
  switch(cStat) {
  //--- Cloud is created -------------
  case CLOUD_INITIALIZE:
    Create();
    return 1;
  //--- Compute sun distance ---------
  case CLOUD_NEW_BORN:
    sunPOS  = *csys->SunOrientation();
    sunPOS.AXplusB(sunPOS,extVOL,geof);
    SunDist();
    return 1;
  //---Sort particles to sun position-
  case CLOUD_SUN_DISTANCE:
    SunSort();
    return 1;

  //--- Initcomputing Light from sun--------
  case CLOUD_SUN_INITLITE:
    InitSunLight(sunPOS);
    return 1;
 //---  Compute Light from sun--------
  case CLOUD_SUN_COMPUTE:
    PartSunLight(sunPOS);
    return 1;
  //--- Compute Camera Distance ------
  case CLOUD_SUN_RELIGHT:
    cPart = 0;
    cStat = CLOUD_CAM_ORDERED;
    return 1;
  //--- Sort particle: bypassed -------
  case CLOUD_CAM_DISTANCE:
    CamSort();
    return 1;
  //--- Compute light from eye position ------
  case CLOUD_CAM_ORDERED:
    RelightCloud();  
    return 1;
  //--- End of relight cycle ------------------
  case CLOUD_IS_ALIGHTED:
    dPart  = nPart;
    dStat = CLOUD_DRAW_VOL;
    //--- schedule changing state -------------
    cStat = CLOUD_TO_REFRESH;
    u1FR = csys->GetFrame() + 8;
    u2FR = 300;
    return 1;
  //---- Refresh drawing ----------------------
  case CLOUD_TO_REFRESH:
    return Refreshing();
  //---- Flat cloud ---------------------------
  case CLOUD_FLAT_INIT:
    return CheckOwner();
  //---- End of life now ----------------------
  case CLOUD_END_OF_LIFE:
    return 0;
  }
  return 1;
}

//----------------------------------------------------------------------------
//  Draw the cloud bound
//  Just for test
//----------------------------------------------------------------------------
void CCloudPuff::DrawFlatBound()
{ //-----------------------------------------------
  glLineWidth(2);
  glColor3f(1,0,0);
  glBegin(GL_LINE_LOOP);
  glVertex3f(cbb[0].VT_X, cbb[0].VT_Y, cbb[0].VT_Z);
  glVertex3f(cbb[1].VT_X, cbb[1].VT_Y, cbb[1].VT_Z);
  glVertex3f(cbb[2].VT_X, cbb[2].VT_Y, cbb[2].VT_Z);
  glVertex3f(cbb[3].VT_X, cbb[3].VT_Y, cbb[3].VT_Z);
  glEnd();
  glLineWidth(1);
  return;
}
//----------------------------------------------------------------------------
//  Draw the cloud bound
//  Just for test
//----------------------------------------------------------------------------
void CCloudPuff::DrawVoluBound()
{ //----------------------------------------------
  glDisable(GL_TEXTURE_2D);
  glLineWidth(2);
  glColor3f(1,0,0);
  double fw = maxBB.y;
  glBegin(GL_LINE_LOOP);
  glVertex3d(minBB.x,fw,minBB.z);
  glVertex3d(maxBB.x,fw,minBB.z);
  glVertex3d(maxBB.x,fw,maxBB.z);
  glVertex3d(minBB.x,fw,maxBB.z);
  glEnd();
  //---Line from cloud to Camera ---------
  glLineWidth(1);
  glEnable(GL_TEXTURE_2D);
  return;
}
//----------------------------------------------------------------------------
//  Draw Particles (for test)
//----------------------------------------------------------------------------
void CCloudPuff::DrawParticles()
{ V_PART *tab = vtab;
  //glColor4f(1,0,0,1);
	ColorGL(COLOR_RED);
  glDisable(GL_TEXTURE_2D);
  glLineWidth(2);
  for (int No=0; No<nPart; No++)
  { glBegin(GL_LINE_LOOP);
    glVertex3f(tab->VT_X,tab->VT_Y,tab->VT_Z);
    tab++;
    glVertex3f(tab->VT_X,tab->VT_Y,tab->VT_Z);
    tab++;
    glVertex3f(tab->VT_X,tab->VT_Y,tab->VT_Z);
    tab++;
    glVertex3f(tab->VT_X,tab->VT_Y,tab->VT_Z);
    tab++;
    glEnd();
  }
  glLineWidth(1);
  glEnable(GL_TEXTURE_2D);
  return;
}
//----------------------------------------------------------------------------
//  Impostor Tour (for test)
//----------------------------------------------------------------------------
void CCloudPuff::ImpostorTour()
{ TC_VTAB *tab = impTAB;
  //glColor4f(1,0,0,1);
	ColorGL(COLOR_RED);
  glDisable(GL_TEXTURE_2D);
  glLineWidth(2);
  glBegin(GL_LINE_LOOP);
  glVertex3f(tab->VT_X,tab->VT_Y,tab->VT_Z);
  tab++;
  glVertex3f(tab->VT_X,tab->VT_Y,tab->VT_Z);
  tab++;
  glVertex3f(tab->VT_X,tab->VT_Y,tab->VT_Z);
  tab++;
  glVertex3f(tab->VT_X,tab->VT_Y,tab->VT_Z);
  tab++;
  glEnd();
  glLineWidth(1);
  glEnable(GL_TEXTURE_2D);
  glColor3f(1,1,1);
  return;
}
//----------------------------------------------------------------------------
//  Draw Particles as billboard (for test)
//----------------------------------------------------------------------------
void CCloudPuff::DrawDetail()
{ if (CLOUD_DRAW_NOT == dStat)   return;
  glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
 // V_PART *tab = vtab;
  std::vector<CCloudParticle*>::iterator pr;
  for (pr = Parts.begin(); pr != Parts.end(); pr++)
  { //---Translate to particule corner -------------
    CCloudParticle *prt = (*pr);
    V_PART *tab = vtab + (prt->ID << 2);
    glPushMatrix();
    glTranslated(tab->VT_X,tab->VT_Y,tab->VT_Z);
    double mat[16];
    BillBoardMatrix(mat);
    glLoadMatrixd(mat);
    glInterleavedArrays(GL_T2F_C4UB_V3F,0,tab);
    glDrawArrays(GL_QUADS,0,4);
    glPopMatrix();
    tab += 4;
  }
  return;
}
//----------------------------------------------------------------------------
//  Local Geo coordinates 
//----------------------------------------------------------------------------
void CCloudPuff::EditParameters()
{ float wd = globals->wtm->WindRoseDirection();
  char tex[256];
  _snprintf(tex,255,"Wind from %.f   Cloud x=%.2f y=%.2f z=%.2f",wd,geol.x,geol.y,geol.z);
	tex[255] = 0;
  DrawNoticeToUser(tex,0.5);
  return;
}
//----------------------------------------------------------------------------
//  Draw the cloud 
//----------------------------------------------------------------------------
int  CCloudPuff::Draw()
{ //--------Translate to cloud center to render particles -------------
  glPushMatrix();
  //-----Translate to cloud center-------------------------
  glTranslated(geol.x,geol.y,geol.z);
  visi = 1;																										//globals->tcm->SphereInView(geol,extVOL);
  if (visi)
    { switch(dStat)
      { case CLOUD_DRAW_NOT:
          break;
        case CLOUD_DRAW_VOL:
          RenderPuff();
          break;
        case CLOUD_DRAW_FLT:
          RenderFlat();
          break;
        case CLOUD_DRAW_IMP:
          RenderImpostor();
          break;
      } 
  }
  glPopMatrix();
  return (gStat == CLOUD_REMOTE);
  }
//----------------------------------------------------------------------------
//  Get angle for billboard
//----------------------------------------------------------------------------
void CCloudPuff::SetTrueRotation()
{ double a,b;
  SVector up;
  CVector ntc;
  CVector fwd(0,1,0);
  //---Compute horizontal rotation ------------
  ntc.x     = ctoc.x;
  ntc.y     = ctoc.y;
  ntc.Normalize();
  VectorCrossProduct(up,fwd,ntc);
  double cz = VectorDotProduct(fwd,ntc);
  double az = acos(cz);
  if (ctoc.x > 0)  az = - az;
  a  = RadToDeg(az);
  glRotated(a,0,0,1);
  //---Compute vertical rotation --------------
  ctoc.Normalize();
  double cx = VectorDotProduct(ntc,ctoc);
  double ax = acos(cx);
  b = RadToDeg(ax);
  glRotated(b,-1,0,0);
  return;
}

//----------------------------------------------------------------------------
//  Draw the Impostor 
//----------------------------------------------------------------------------
void CCloudPuff::RenderImpostor()
{ if (alpha != 255) alpha++;
  //---------------------------------------------------
  SetTrueRotation();
  csys->SetColor(alpha);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  //---- Drawing parameters ---------------------------
  glBindTexture(GL_TEXTURE_2D,iTex);
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  glInterleavedArrays(GL_T2F_V3F,0,impTAB);
  glDrawArrays(GL_QUADS,0,4);
//  ImpostorTour();
  return;
}
//------------------------------------------------------------------------------
//  Draw cloud
//------------------------------------------------------------------------------
void CCloudPuff::RenderPuff()
{ //----Draw as a set of particles --------------------
  if (camDIS < (1.2 * Radius))  GetShortBB();
  else                  SetTrueRotation();
  glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
  csys->SetColor(255);
  //---- Drawing parameters ---------------------------
  glBindTexture(GL_TEXTURE_2D,pTex);
  //---------------------------------------------------
//  DrawParticles();
  glInterleavedArrays(GL_T2F_C4UB_V3F,0,vtab);
  glDrawArrays(GL_QUADS,0,(dPart << 2));
  return;
}
//------------------------------------------------------------------------------
//  Draw flat cloud
//------------------------------------------------------------------------------
void CCloudPuff::RenderFlat()
{ float lum = globals->tcm->GetLuminosity();
  //---- Drawing parameters ---------------------------
  glColor4f(1,1,1,lum);
  glBindTexture(GL_TEXTURE_2D,pTex);
  //---------------------------------------------------
  glInterleavedArrays(GL_T2F_V3F,0,cbb);
  glDrawArrays(GL_QUADS,0,4);
  return;
}
//=======================END OF FILE ==================================================