/*
 * TestBed.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
 * Copyright 2007-2009 Jean Sabatier
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

#include "../Include/Globals.h"
#include "../Include/TestBed.h"
#include "../Include/Cameras.h"
#include "../Include/Triangulator.h"
#include "../Include/TerrainTexture.h"
#include "../Include/ScenerySet.h"
#include "../Include/Compression.h"
using namespace std;
//===================================================================================
//	Class ground Quad to build a terrain tile for tests
//===================================================================================
GroundQUAD::GroundQUAD(int x, int y, int z, int dim, char c)
{	cmp		= c;
	xOBJ	= 0;
	cz		= z;
	SetQuad(x,y,dim);
}
//---------------------------------------------------------------------
//	Destructor
//---------------------------------------------------------------------
GroundQUAD::~GroundQUAD()
{	if (xOBJ)	glDeleteTextures(1,&xOBJ);
}
//---------------------------------------------------------------------
//	Build quad of dimension dim at screen position (x,y)
//	NOTE:  The origin is actually supposed to be the left lower
//	corner of the screen
//---------------------------------------------------------------------
void GroundQUAD::SetQuad(int x, int y, int dim)
{	cx	= x;
	cy	= y;
	float hf  = float(dim) * 0.5;
  float dto,dtu;
	globals->txw->GetTextParam(TC_HIGHTR,&dto,&dtu); 
	//-------------------------------------------------------
	float t1 = dto;
	float t2 = dto + dtu;
	//-------------------------------------------------------
  //  Allocate the vertex table for the panel 
  //  The panel is drawed as a textured QUAD 
  //----NW base corner ------------------------------------
  Q[0].GT_S   = 0;		//t1;
  Q[0].GT_T   = 1;		//t1;
  Q[0].GT_X   = -hf;
  Q[0].GT_Y   = +hf;
  Q[0].GT_Z   = 0;
  //-----SW corner ----------------------------------------
  Q[1].GT_S   = 0;		//t2;
  Q[1].GT_T   = 0;		//t1;
  Q[1].GT_X   = -hf;
  Q[1].GT_Y   = -hf;
  Q[1].GT_Z   = 0;
  //-----NE corner ----------------------------------------
  Q[2].GT_S   = 1;		//t1;
  Q[2].GT_T   = 1;		//t2;
  Q[2].GT_X   = +hf;
  Q[2].GT_Y   = +hf;
  Q[2].GT_Z   = 0;
  //-----SE corner ----------------------------------------
  Q[3].GT_S   = 1;		//t2;
  Q[3].GT_T   = 0;		//t2;
	Q[3].GT_X   = +hf;
  Q[3].GT_Y   = -hf;
  Q[3].GT_Z   = 0;
	return;
}
//-----------------------------------------------------------------------
//		Return texture object
//-----------------------------------------------------------------------
void GroundQUAD::GetTexOBJ(TEXT_INFO &txd)
{	//--------Read the RAW and ACT texture file ----------------------
	U_CHAR reso = txd.res;
	U_INT *wtr  = globals->txw->GetWaterRGBA(reso);
  CArtParser img(reso);
  img.SetWaterRGBA(wtr);
  GLubyte *rgba = img.GetRawTexture(txd,1);
  if (0 == rgba)	return;
	xOBJ	= globals->txw->GetTerraOBJ(0,reso,rgba);
	delete [] rgba;
}
//-----------------------------------------------------------------------
//		Increment distance
//-----------------------------------------------------------------------
bool GroundQUAD::IncDistance()
{	cz += 10;
	if (cz < 10000)	return true;
	cz = 10000;
	return false;
}
//-----------------------------------------------------------------------
//		Decrement distance
//-----------------------------------------------------------------------
bool GroundQUAD::DecDistance()
{	cz -= 10;
	if (cz > 0)	return true;
	cz =  0;
	return false;
}
//-----------------------------------------------------------------------
//  Get Texture from database
//  NOTE:  For water we only use one texture as they are all the same
//         in the super Tile
//-----------------------------------------------------------------------
void GroundQUAD::GetTextureFromSQL(char *nam,U_INT px,U_INT pz)
{	if (0 == globals->sqm->UseDtxDB())		return;
  TEXT_INFO xds;
	xds.key	= (px << 16) | pz;
	globals->sql[THREAD_TEX]->ReadaTRNtexture(xds,"TEX",0);
	//--- Decompress it ---------------------------------------------
	mip	= 5;
	Compressor comp(xds.type,mip);
	void *mem = xds.mADR;
	xOBJ	= comp.DecodeCRN(mem,xds.dim,0,5);
	//---------------------------------------------------------------
	free(mem);
	return;
}
//-----------------------------------------------------------------------
//  Get Texture from pod system
//  NOTE:  For water we only use one texture as they are all the same
//         in the super Tile
//-----------------------------------------------------------------------
void GroundQUAD::GetTextureFromPOD(char *nam,U_INT qx,U_INT qz)
{ TEXT_INFO txd;
	char	reso	= TC_HIGHTR;							// Resolution
	char  root[32];                       // file name
	U_INT	gx		= (qx >> 1);							// Global tile x indice
	U_INT	gz		= (qz >> 1);							// Global tile z indice
	//---------------------------------------------------------------
	txd.res			= reso;										// Resolution
	txd.qty			= 128;										// Compression quality (half)
  strncpy(root,nam,8);									// Root Name
  root[8]   = '5';											// Resolution
  root[9]   = 0;                        // Close name
	strncpy(txd.name,root,10);						// Copy the name
  _snprintf(txd.path,TC_TEXTURE_NAME_DIM,"DATA/D%03d%03d/%s.",gx,gz,root);
	if (0 == cmp)		return GetTexOBJ(txd);
	//--- Read  the file --------------------------------------------
	txd.mADR		= 0;
	U_INT *wtr  = globals->txw->GetWaterRGBA(reso);
	//--------Read the RAW and ACT texture file ----------------------
  CArtParser img(reso);
  img.SetWaterRGBA(wtr);
  txd.mADR	= img.GetRawTexture(txd,1);
	if (0 == txd.mADR)		return;
	//--- Get a CRN format ------------------------------------------
	mip	= 5;
	Compressor comp(COMP_DTX5,mip);
	comp.EncodeCRN(txd);

 
	//--- Decompress it ---------------------------------------------
	void *mem = txd.mADR;
	xOBJ	= comp.DecodeCRN(mem,txd.dim,0,5);
	//---------------------------------------------------------------
	free(mem);
	return;
}
//------------------------------------------------------------------------------
//	Draw the QUAD 
//------------------------------------------------------------------------------
void GroundQUAD::Draw()
{	glBindTexture(GL_TEXTURE_2D,xOBJ);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	glPushMatrix();
	glTranslated(cx,cy,cz);
/*
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(Q[0].GT_S,Q[0].GT_T);
	glVertex3f(Q[0].GT_X,Q[0].GT_Y,Q[0].GT_Z);

	glTexCoord2f(Q[1].GT_S,Q[1].GT_T);
	glVertex3f(Q[1].GT_X,Q[1].GT_Y,Q[1].GT_Z);
	
	glTexCoord2f(Q[2].GT_S,Q[2].GT_T);
	glVertex3f(Q[2].GT_X,Q[2].GT_Y,Q[2].GT_Z);

	glTexCoord2f(Q[3].GT_S,Q[3].GT_T);
	glVertex3f(Q[3].GT_X,Q[3].GT_Y,Q[3].GT_Z);


	glEnd();
	*/
	//-----------------------------------------------------
	
	glVertexPointer  (3,UNIT_GTAB,sizeof(TC_GTAB),&Q[0].GT_X);
	glTexCoordPointer(2,UNIT_GTAB,sizeof(TC_GTAB), Q);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	//glDrawArrays(GL_LINE_LOOP,0,4);
	//------------------------------------------------------
	glPopMatrix();
}

//===================================================================================
//--------------------------------------------------------------------
// Constructor 
//--------------------------------------------------------------------
CTestBed::CTestBed()
{ U_INT qx = 508;
	U_INT qz = 336;
	U_INT tx = 28;
	U_INT tz = 5;
	U_INT px = (qx << 5) | tx;
	U_INT pz = (qz << 5) | tz;
	U_INT key = DetailKEY(qx,qz,tx,tz);
	char *tex = "977EF6DC";
  globals->scn->MountByKey(qx,qz);
  Q0	= new GroundQUAD(+250,  50,0,400,0);
  Q0->GetTextureFromPOD(tex,qx,qz);
	Q1	= new GroundQUAD(-250,  50,0,400,1);
	//Q1->GetTextureFromSQL(tex,px,pz);
  Q1->GetTextureFromPOD(tex,qx,qz);
	//Q1	= 0;

	//-------------------------------------------
	hold  = 0;
	state	= 0;
	time	= 240;
}
//--------------------------------------------------------------------
// Make a detail key 
//--------------------------------------------------------------------
U_INT CTestBed::DetailKEY(U_INT qx,U_INT qz,U_INT tx, U_INT tz)
{	U_INT px = (qx << 5) | tx;
	U_INT pz = (qz << 5) | tz;
	return (px << 16) | pz;
}
//--------------------------------------------------------------------
//  Free resources
//--------------------------------------------------------------------
CTestBed::~CTestBed()
{	if (Q0)	delete Q0;
	if (Q1)	delete Q1;
}
//--------------------------------------------------------------------
//  Free resources
//--------------------------------------------------------------------
void CTestBed::Clean()
{	if (Q0)	delete Q0;
	if (Q1)	delete Q1;
}
//-------------------------------------------------------------------
//  Keyboard
//-------------------------------------------------------------------
void CTestBed::Keyboard(U_INT key,int mod)
{ if (key == 'q')		globals->stop = 1;
	if (key == 'm')		hold ^= 1;
	return;}

void CTestBed::Special(U_INT key,int mod)
{ return;}
//-------------------------------------------------------------------
//  Keyboard
//-------------------------------------------------------------------
void CTestBed::TimeSlice()
{if (0 == Q1)					return; 
 if (hold)						return;
 switch (state)	{
		case 0:
			time--;
			if (time > 0)		return;
			time = 240;
			state = 1;
			return;
		case 1:
			if (Q1->IncDistance())	return;
			state	= 2;
			return;
		case 2:
			if (Q1->DecDistance())	return;
			state = 0;
			return;
	}
  return;}
//-------------------------------------------------------------------
//  Shoot
//-------------------------------------------------------------------
void CTestBed::Shoot()
{	int wd = 512;
  int ht = 512;
  int sw = globals->mScreen.Width;
  int sh = globals->mScreen.Height;
	double rat = double(sw) / sh;
  // Initialize modelview matrix to ensure panel drawing is not affected by
  //   any junk left at the top of the stack by previous rendering
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  glLoadIdentity ();
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
	//----------------------------------------------------------------
	gluLookAt(0, 0, -1000,
            0.0, 0.0,  +100,
            0.0, 1.0,  0.0);

	//--- Set Perspective projection ---------------------------------
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  //glOrtho(0, sw, 0, sh,-100000,+100);
	gluPerspective(45, rat, 8, 100000);
  glMatrixMode (GL_MODELVIEW);
  //---Save viewport -----------------------------------------------
  glGetIntegerv(GL_VIEWPORT,vp);
  glViewport(0,0,sw,sh);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glFrontFace(GL_CCW);
	//-----------------------------------------------------------------
	glDisable (GL_BLEND);
	glEnable  (GL_TEXTURE_2D);
  glDisable (GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	//-----------------------------------------------------------------
	glColor4f(1,1,1,1);
}
//-------------------------------------------------------------------
//  Draw
//-------------------------------------------------------------------
void CTestBed::Draw()
{ Shoot();
	//-----------------------------------------------------
	if (Q0)	Q0->Draw();
	if (Q1)	Q1->Draw();
	//-----------------------------------------------------
	glViewport(vp[0],vp[1],vp[2],vp[3]);
  glPopClientAttrib ();
  glPopAttrib();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  //---------- Check for an OpenGL error ---------------------
  { GLenum e = glGetError ();
    if (e != GL_NO_ERROR) 
        WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  }

  return;
}

//=========================END 0F FILE ====================================================
