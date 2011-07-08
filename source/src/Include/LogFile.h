/*
 * LogFile.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2000-2004 Chris Wallace
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
 */

/*! \file LogFile.h
 *  \brief Defines CLogFile class for application log file management
 */

#ifndef LOGFILE_H
#define LOGFILE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdarg.h>
#include <stdio.h>

//
// CLogFile
//
class CLogFile {
public:
  // Constructors / destructor
	CLogFile (const char*, const char* mode);
	~CLogFile (void);

  // CLogFile methods
  FILE *GetFile (void);
  void Write (const char* fmt, va_list argp);
	void Write (const char* fmt, ...);
  void Date();

protected:
	FILE   *f;
};

#endif // LOGFILE_H

