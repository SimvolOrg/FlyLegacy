/*
 * DBUtilities.cpp
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

/*! \file DBUtilities.cpp
 *  \brief Miscellaneous utility functions used by database classes
 *
 *  The functions implemented in this file are used by database classes
 *    CDBDatabase, CDBIndex and CDBTemplate.
 */


#include "../Include/Database.h"
#include "../Include/Pod.h"
#include "../Include/Endian.h"

//
// CDatabaseField
//
CDatabaseField::CDatabaseField (void)
{
  type = 0;
  length = 0;
  memset (data.charData, 0, 256);
}
/*
CDatabaseField::~CDatabaseField (void)
{
  switch (type) {
  case 'C':
    if (data.pCharData != NULL) {
      delete[] data.pCharData;
    }
    break;

  case 'D':
    if (data.pDoubleData != NULL) {
      delete data.pDoubleData;
    }
    break;

  case 'I':
    if (data.pIntData != NULL) {
      delete data.pIntData;
    }
    break;

  default:
    ;
  }
}
*/
char CDatabaseField::GetType (void)
{
  return type;
}

unsigned long CDatabaseField::GetLength (void)
{
  return length;
}

void CDatabaseField::GetString (char *s)
{
  if (type == 'C') {
//    if (data.pCharData != NULL) {
      strcpy (s, data.charData);
//    }
  }
}

double CDatabaseField::GetDouble (void)
{
  double rc = 0;
  if (type == 'D') {
//    if (data.pDoubleData != NULL) {
      rc = data.doubleData;
//    }
  }
  return rc;
}

long CDatabaseField::GetInt (void)
{
  long rc = 0;
  if (type == 'I') {
//    if (data.pIntData != NULL) {
      rc = data.intData;
//    }
  }
  return rc;
}

void CDatabaseField::Format (char *s)
{
  switch (type) {
  case 'C':
    {
      // Copy string to supplied buffer
      strcpy (s, data.charData);

      // Pad with spaces to data length
      for (unsigned int i=strlen(data.charData); i < length; i++) {
        s[i] = ' ';
      }
    
      // Null terminator
      s[length] = '\0';
    }
    break;

  case 'D':
    {
      // Double-precision float
      sprintf (s, "%g", data.doubleData);
    }
    break;

  case 'I':
    {
      // Integer (unsigned long)
      sprintf (s, "%lu", data.intData);
    }
    break;
  }
}

//
//  Read unsigned long from binary file
//
void ReadULong (PODFILE *f, unsigned long *pData)
{
  pread (pData, sizeof(unsigned long), 1, f);
  *pData = LittleEndian(*pData);
}


//
//  Read signed long from binary file
//
void ReadLong (PODFILE *f, long *pData)
{
  pread (pData, sizeof(long), 1, f);
  *pData = LittleEndian(*pData);
}


//
//  Read unsigned short from (little-endian) database binary file
//  Automatically adjusts for big-endian processors
//
void ReadUShort (PODFILE *f, unsigned short *pData)
{
  pread (pData, sizeof(unsigned short), 1, f);
  *pData = LittleEndian(*pData);
}


//
//  Read double float from (little-endian) database binary file
//  Automatically adjusts for big-endian processors
//
void ReadDouble (PODFILE *f, double *pData)
{
  pread (pData, sizeof(double), 1, f);
  *pData = LittleEndian(*pData);
}

//
//  Read float from (little-endian) database binary file
//  Automatically adjusts for big-endian processors
//
void ReadFloat (PODFILE *f, float *pData)
{
  pread (pData, sizeof(float), 1, f);
  *pData = LittleEndian(*pData);
}


//
//  Read unsigned char from (little-endian) database binary file
//
void ReadUChar (PODFILE *f, unsigned char *pData)
{
  pread (pData, sizeof(unsigned char), 1, f);
}



//
// Returns the next tab-delimitted field in the passed-in string.
// The string pointer is updated to point beyond the delimitting tab
//
char * TabNextField (char *s, char *szField)
{
  char *pTab = strchr (s, '\t');
  if (pTab != NULL) {
    int nChars = pTab - s;
    strncpy (szField, s, nChars);

    // Adjust s to point beyond tab
    s = pTab + 1;
  }
  return s;
}

//
// Skips the specified number of tab-delimitted fields
//
char * TabSkipFields (char *s, int nFields)
{
  int i;
  for (i=0; i<nFields; i++) {
    char *pTab = strchr (s, '\t');
    if (pTab != NULL) {
      // Adjust s to point beyond tab
      s = pTab + 1;
    }
  }
  return s;
}
