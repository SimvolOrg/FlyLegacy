/*
 * LogFile.cpp
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

/*! \file LogFile.cpp
 *  \brief Implements CLogFile application log file class
 */

#include <sys/types.h>
#include <sys/timeb.h>
#include <time.h>
#include <string.h>
#include "../Include/LogFile.h"
#include "../Include/Globals.h"

//
// Constructor JSDEV* Build fullname to allow debugging from different directory
//
CLogFile::CLogFile (const char* filename, const char* mode)
{	f = fopen (filename, mode);
}


//
// Destructor
//
CLogFile::~CLogFile (void)
{
  // Close file handle
	if (f) {
		fclose (f);
		f = NULL;
	}
}

//
// Return file reference
//
FILE *CLogFile::GetFile (void)
{
  return f;
}


void CLogFile::Write (const char* fmt, va_list argp)
{if (0 == fmt)	return;
  if (f) {
		vfprintf(f, fmt, argp);
    fprintf (f, "\n");
    fflush (f);
  }
}

//=================================================================
// Write timestamped output to the log file
//=================================================================
void CLogFile::Write (const char* fmt, ...)
{
  if (f) {
    // Get local computer date/time
    struct timeb t;
    ftime (&t);
    struct tm* local = localtime (&t.time);
    if (local) {
      // Prefix message with timestamp in fixed-length 26-byte format:
      //   <YYYY-MM-DD HH:MM:SS.MMMM  >
      fprintf (f, "%04d-%02d-%02d %02d:%02d:%02d.%03d  ",
        1900 + local->tm_year, local->tm_mon+1, local->tm_mday,
        local->tm_hour, local->tm_min, local->tm_sec, t.millitm);
    }

		va_list argp;
		va_start(argp, fmt);
		vfprintf(f, fmt, argp);
		va_end(argp);

    fprintf (f, "\n");
    fflush (f);
	}
}
//=================================================================
// Write timestamped output to the log file
//=================================================================
void CLogFile::Date()
{ if (!f)       return;
  struct timeb t;
  ftime (&t);
  struct tm* local = localtime (&t.time);
  if (local) {
      // Prefix message with timestamp in fixed-length 26-byte format:
      //   <YYYY-MM-DD HH:MM:SS.MMMM  >
      fprintf (f, "On %04d-%02d-%02d %02d:%02d:%02d.%03d \n",
        1900 + local->tm_year, local->tm_mon+1, local->tm_mday,
        local->tm_hour, local->tm_min, local->tm_sec, t.millitm);
    }
  return;
}

//=============END OF FILE ===============================================
