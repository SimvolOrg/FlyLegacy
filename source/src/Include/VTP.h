/*
 * VTP.h
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

/*! \file VTP.h
 *  \brief Definitions for Enviro VTP .bt file format
 */


#ifndef VTP_H
#define VTP_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#ifdef _WIN32
#pragma pack(push,1)
#endif

struct SBTHeader {
	char		magic[10];
	int			columns;
	int			rows;
	short		size;
	short		fp;
	short		space_units;
	short		utm_zone;
	short		datum;
	double  left;
	double  right;
	double  bottom;
	double  top;
	short		prj;
	float		scale;
	char		pad[190];
};

#ifdef _WIN32
#pragma pack(pop)
#endif

#define SIZEOF_BT_HEADER  0x100

#endif // VTP_H
