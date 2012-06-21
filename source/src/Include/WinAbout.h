//==============================================================================================
// WindAbout.h
//
// Part of Fly! Legacy project
//
//	Copyright 2003 Chris Wallace
//	Copyright 2007 Jean Sabatier
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
//================================================================================================

#ifndef WINABOUT_H
#define WINABOUT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//==================================================================================
#include "../Include/FlyLegacy.h"
#include "../Include/Fui.h"
//==================================================================================
//  CFuiAbout.  A window for versions and credits
//==================================================================================
class CFuiAbout: public CFuiWindow
{ //------------Attributes ---------------------------------------
	CFuiCanva *info;
	//--- Public methods -------------------------------------------
public:
	CFuiAbout::CFuiAbout(Tag idn, const char *filename);
	//--------------------------------------------------------------
	void	Draw();
	void	Credit(CFuiCanva *cnv);
};

//======================= END OF FILE ==============================================================
#endif // WINABOUT_H
