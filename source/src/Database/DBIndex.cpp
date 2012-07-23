/*
 * DBIndex.cpp
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

/*! \file DBIndex.cpp
 *  \brief Implements CDatabaseIndex database index class
 *
 *  This file implements CDatabaseIndex, which encapsulates a database index
 *   represented in a DBI file.  Indices are attached to databases
 *   through methods of class DBDatabase.
 */

#include "../Include/Globals.h"
#include "../Include/Database.h"
#include "../Include/Pod.h"
#include <map>

using namespace std;


typedef struct {
  unsigned long   signature;
  unsigned long   unknown0;
  unsigned long   unknown1;
  unsigned long   checksum;
  unsigned long   nRecords;
  unsigned long   recLength;
  unsigned long   dummy[8];
  Tag             keyId;
  unsigned char   keyType;
  unsigned long   keyLength;
} DBIHeader;


CDatabaseIndex::CDatabaseIndex (const char* dbiFilename)
{
  keyId = 0;
  keyType = 0;
  keyLength = 0;

  stringIndex = NULL;
  doubleIndex = NULL;
  intIndex = NULL;

  Load (dbiFilename);
}

CDatabaseIndex::~CDatabaseIndex ()
{
  if (stringIndex != NULL) delete stringIndex;
  if (doubleIndex != NULL) delete doubleIndex;
  if (intIndex != NULL)    delete intIndex;
}

//
// Loader which takes an open PODFILE* reference to the DBI file
//
void CDatabaseIndex::Load (PODFILE* f)
{
  int i;

  // Read DBI Header
  DBIHeader header;
  ReadULong (f, &header.signature);
  ReadULong (f, &header.unknown0);
  ReadULong (f, &header.unknown1);
  ReadULong (f, &header.checksum);
  ReadULong (f, &header.nRecords);
  ReadULong (f, &header.recLength);
  for (i=0; i<8; i++) ReadULong (f, &header.dummy[i]);

  // Read key identifier
  ReadULong (f, &header.keyId);

  // Key type is unsigned char but stored in file as an unsigned long
  unsigned char keyTypeRaw;
  ReadUChar (f, &keyTypeRaw);
  ReadUChar (f, &keyTypeRaw);
  header.keyType = keyTypeRaw;
  ReadUChar (f, &keyTypeRaw);
  ReadUChar (f, &keyTypeRaw);

  // Read key length
  ReadULong (f, &header.keyLength);

  // Assign key data fields
  keyId = header.keyId;
  keyType = header.keyType;
  keyLength = header.keyLength;

  unsigned long offset;

  switch (keyType) {
  case 'C':
    {
      // Allocate index multimap
      stringIndex = new std::multimap<string,unsigned long>;

      // Parse DBI items as strings
      char *s = new char[keyLength+1];
      for (unsigned long i=0; i<header.nRecords; i++) {
        memset (s, 0, keyLength+1);
        pread (s, keyLength, 1, f);
        ReadULong (f, &offset);

        pair<string,unsigned long> p;
        p.first = s;
        p.second = offset;
        stringIndex->insert (p);
      }
      delete[] s;
    }
    break;

  case 'D':
    {
      // Allocate index multimap
      doubleIndex = new std::multimap<double,unsigned long>;

      // Parse DBI items as double floats
      double d;
      for (unsigned long i=0; i<header.nRecords; i++) {
        ReadDouble (f, &d);
        ReadULong (f, &offset);

        pair<double,unsigned long> p;
        p.first = d;
        p.second = offset;

        doubleIndex->insert (p);
      }
    }
    break;

  case 'I':
    {
      // Allocate index multimap
      intIndex = new std::multimap<long,unsigned long>;

      // Parse DBI items as long integers
      long d;
      for (unsigned long i=0; i<header.nRecords; i++) {
        ReadLong (f, &d);
        ReadULong (f, &offset);

        pair<long,unsigned long> p;
        p.first = d;
        p.second = offset;

        intIndex->insert (p);
      }
    }
    break;
  }
}

//
// Constructor which accepts a const char* filename of the DBI file
//
void CDatabaseIndex::Load (const char* dbi)
{
  char fn[128];
	_snprintf(fn,127,"DATABASE/%s",dbi);
  PODFILE *p = popen (&globals->pfs, fn);

  if (p) {
    Load (p);
    pclose (p);
  }
}


//
// Data access function to get the number of records in the index
//
int CDatabaseIndex::GetNumItems ()
{
  int rc = 0;

  if (stringIndex != NULL) {
    rc = stringIndex->size();
  } else if (doubleIndex != NULL) {
    rc = doubleIndex->size();
  } else if (intIndex != NULL) {
    rc = intIndex->size();
  }

  return rc;
}

//
// Data access function to get the index key Tag
//
Tag CDatabaseIndex::GetKeyId (void)
{
  return keyId;
}

//
// Data access function to get the index key type
//
char CDatabaseIndex::GetKeyType (void)
{
  return keyType;
}

//
// Data access function to get the index key length
//
unsigned long CDatabaseIndex::GetKeyLength (void)
{
  return keyLength;
}


unsigned long CDatabaseIndex::Search (const char* key)
{
  unsigned long rc = 0;

  if (0 == stringIndex)                 return rc;
  stringRange = stringIndex->equal_range (key);
  stringIter  = stringRange.first;
  if (stringIter != stringRange.second) return stringIter->second;
  return rc;
}

//
// Search the index for the specified value
//
unsigned long CDatabaseIndex::Search (double key)
{
  unsigned long rc = 0;

  if (doubleIndex != NULL) {
    doubleRange = doubleIndex->equal_range (key);
    doubleIter = doubleRange.first;
    if (doubleIter != doubleRange.second) {
      rc = doubleIter->second;
    }
  }

  return rc;
}


//
// Search the index for the specified value
//
unsigned long CDatabaseIndex::Search (long key)
{
  unsigned long rc = 0;

  if (intIndex != NULL) {
    intRange = intIndex->equal_range (key);
    intIter = intRange.first;
    if (intIter != intRange.second) {
      rc = intIter->second;
    }
  }

  return rc;
}

unsigned long CDatabaseIndex::SearchNext (void)
{ unsigned long rc = 0;

  if (0 == stringIndex) return rc;
  stringIter++;
  if (stringIter != stringRange.second) return stringIter->second;
  return rc;
}
void CDatabaseIndex::Dump (FILE *f)
{
  fprintf (f, "Index:\n");
  fprintf (f, "  Key ID:\t%s", TagToString(keyId));
  fprintf (f, "  Key Type:  %c", keyType);
  fprintf (f, "  Key Length: %d", keyLength);

  int nElements = 0;
  switch (keyType) {
    case 'C':
      nElements = stringIndex->size();
      break;
    case 'D':
      nElements = doubleIndex->size();
      break;
    case 'I':
      nElements = intIndex->size();
      break;
  }
  fprintf (f, "  Num elements: %d", nElements);
  fprintf (f, "\n");
}
//=========================== END OF FILE ================================================================