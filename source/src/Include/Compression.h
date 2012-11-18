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
#include "../Include/FlyLegacy.h"
#include <crnlib.h>
class RawCompressor: public CStreamObject {
protected:
	//--- ATTRIBUTS ------------------------------
	//--- METHODS --------------------------------
public:
	RawCompressor(char *fn);
};
//=== END OF FILE ====================================================================================
#endif COMPRESSION_H