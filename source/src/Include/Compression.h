//===============================================================================================
// Compression
//
// Part of Fly! Legacy project
//
// Copyright 2003 Chris Wallace
//
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
//===============================================================================================
#ifndef COMPRESSION_H
#define COMPRESSION_H
#define CRND_HEADER_FILE_ONLY 
#include "../Include/FlyLegacy.h"
#include <crnlib.h>
#include "crn_decomp.h"
//------------------------------------------------------------------------------------------------
using namespace crnd;
//===========================================================================================
#define COMP_DTX0			(0)
#define COMP_DTX1			(1)
#define COMP_DTX3			(2)
#define COMP_DTX5			(3)
//================================================================================================
class CTextureDef;
//================================================================================================
class Compressor: public CStreamObject {
protected:
	//--- ATTRIBUTS ------------------------------
	void	*crnd;
	crn_texture_info			txd;			// Texture descriptor
	crnd_unpack_context		ctx;			// Pointer to context
	U_INT	mode;
	//----------------------------------------------
	void *img[1][cCRNMaxLevels];
	U_INT siz[cCRNMaxLevels];
	//--- Texture object ---------------------------
	U_INT	xOBJ;
	U_INT	mip;											// Mip level
	//--- METHODS ----------------------------------
public:
	Compressor(U_INT t,U_INT m);
	//----------------------------------------------
	U_INT  Clean();
	//----------------------------------------------
	void  EncodeCRN(TEXT_INFO &tdf);
	U_INT	DecodeCRN(void *data, U_INT sze,U_CHAR l0, U_CHAR ln);

};
//=== END OF FILE ====================================================================================
#endif COMPRESSION_H