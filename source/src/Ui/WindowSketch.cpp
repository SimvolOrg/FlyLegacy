//==========================================================================================
//
// Part of Fly! Legacy project
//
// Copyright 2005 Chris Wallace
// Copyright 2012 Jean Sabatier
// Fly! Legacy is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
// Fly! Legacy is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
//   along with Fly! Legacy; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//==========================================================================================
#include "../include/WinSketch.h"
#include <vector>

//==========================================================================================
//  Window to display building sketch
//==========================================================================================
CFuiSketch::CFuiSketch(Tag idn, const char *filename)
:CFuiWindow(idn,filename,200,200,0)
{ title = 1;
  close = 1;
  zoom  = 0;
  mini  = 0;
	//--- Set title ---------------------------------
	strcpy(text,"SKETCH EDITOR");
	//-----------------------------------------------	
	ReadFinished();
	//--- Temporary ---------------------------------
	sphere = gluNewQuadric();
	gluQuadricDrawStyle(sphere,GLU_LINE);
	//-----------------------------------------------
	ctx.prof	= PROF_SKETCH;
	ctx.mode	= SLEW_RCAM;
  rcam			= globals->ccm->SetRabbitCamera(ctx,this);
	//---- set world origin ---------------

};
//-----------------------------------------------------------------------
//	destroy this
//-----------------------------------------------------------------------
CFuiSketch::~CFuiSketch()
{	gluDeleteQuadric(sphere);
	globals->ccm->RestoreCamera(ctx);
}
//-----------------------------------------------------------------------
//	Draw the sketch
//-----------------------------------------------------------------------
void	CFuiSketch::Draw()
{	CFuiWindow::Draw();
	//--- save camera context --------------------------
	
	//--------------------------------------------------
	return;
}
//============================END OF FILE ================================================================================
