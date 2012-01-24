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
#include "../Include/Import.h"
#include "../Include/Fileparser.h"
#include "../Include/TerrainTexture.h"
using namespace std;
//===================================================================================
//--------------------------------------------------------------------
// Constructor 
//--------------------------------------------------------------------
CTestBed::CTestBed()
{ Cam   = new CCameraSpot();
	Cam->RangeFor(50,64);
	Cam->SetAngle(0,60);
	globals->cam = Cam;
	dmod	= 0;
	//--------------------------------------
	trn.Load("Objects/OBJ03.txt");
	trn.Start();
	//---- set world origin ---------------
	globals->sit->WorldOrigin();
}

//--------------------------------------------------------------------
//  Free resources
//--------------------------------------------------------------------
CTestBed::~CTestBed()
{	delete Cam;
}
//-------------------------------------------------------------------
//  Keyboard
//-------------------------------------------------------------------
void CTestBed::Keyboard(U_INT key,int mod)
{ return;}
void CTestBed::Special(U_INT key,int mod)
{ return;}
//-------------------------------------------------------------------
//  Keyboard
//-------------------------------------------------------------------
void CTestBed::TimeSlice()
{
  return;}

//-------------------------------------------------------------------
//  Draw
//-------------------------------------------------------------------
void CTestBed::Draw()
{ 	Cam->StartShoot(0);
		trn.Draw();
		Cam->StopShoot();

  //---------- Check for an OpenGL error ---------------------
  { GLenum e = glGetError ();
    if (e != GL_NO_ERROR) 
        WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
  }

  return;
}

//=========================END 0F FILE ====================================================
