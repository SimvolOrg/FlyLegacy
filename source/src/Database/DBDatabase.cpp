/*
 * DBDatabase.cpp
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
/*! \file DBDatabase.cpp
 *  \brief Implements DBDatabase database encapsulation class
 *
 * This file contains the implementation of the DBDatabase class, which
 *   provides an encapsulation of a Fly! database.  A database consists
 *   of one DBT (Database Template) file, one DBD (Database Data) file
 *   and zero or more DBI (Database Index) files.  This class implements
 *   methods for instantiation of database given existing DBT and DBD files,
 *   linking of an arbitrary number of DBI indices to the database, and 
 *   lookup functions for particular records in a database.  The DBDatabase
 *   class has no knowledge of the actual internal structure or contents
 *   of the encapsulated database; the application must determine this by
 *   examining the details of the tempate and indexes as necessary.
 */

#include "../Include/Globals.h"
#include "../Include/Database.h"
#include "../Include/Pod.h"

using namespace std;


CDatabaseRecord::CDatabaseRecord (void)
{
  sequence = 0;
}

void CDatabaseRecord::SetSequence (unsigned long sequence)
{
  this->sequence = sequence;
}

void CDatabaseRecord::AddField (CDatabaseField & f)
{
  field.push_back (f);
}

int CDatabaseRecord::GetNumFields (void)
{
  return field.size();
}

CDatabaseField *CDatabaseRecord::GetField (unsigned int i)
{
  CDatabaseField *rc = NULL;

  if (i < field.size()) {
    rc = &(field[i]);
  }

  return rc;
}

void CDatabaseRecord::Reset()
{
  field.clear();
  sequence = 0;
}
//=======================================================================
//  Constructor
//=======================================================================
CDatabase::CDatabase (const char* dbtFilename)
{
  // Load the database template
  LoadTemplate (dbtFilename);

  // Initialize state to UNMOUNTED until Mount() is called
  state   =  DB_UNMOUNTED;
  podfile = NULL;
  buf     = 0;
}

///--------------------------------------------------------------------
/// Deallocate all dynamic memory from the heap, close all open files
///--------------------------------------------------------------------
CDatabase::~CDatabase (void)
{
  // Delete buffer for current record
  if (buf != NULL) delete[] buf;
  // Delete indices
  std::multimap<Tag,CDatabaseIndex*>::iterator i;
  for (i=dbi.begin(); i!=dbi.end(); i++) SAFE_DELETE (i->second);
  dbi.clear();
  Unmount ();
}

//-------------------------------------------------------------------
// Template related methods
//-------------------------------------------------------------------
void CDatabase::LoadTemplate (const char* dbtFilename)
{
  dbt.Load (dbtFilename);
}

CDatabaseTemplate* CDatabase::GetTemplate (void)
{
  return &dbt;
}

//-------------------------------------------------------------------
// Index related methods
//-------------------------------------------------------------------
void CDatabase::AddIndex (const char *dbiFilename)
{
  pair<Tag,CDatabaseIndex*> dbiInsert;
  dbiInsert.second = new CDatabaseIndex (dbiFilename);
  dbiInsert.first = dbiInsert.second->GetKeyId();
  dbi.insert (dbiInsert);
  char idn[8];
  TagToString(idn,dbiInsert.second->GetKeyId());
  TRACE("------------DB %s Add index %s", dbiFilename,idn);
}
//-------------------------------------------------------------------

unsigned int CDatabase::GetNumIndexes (void)
{
  return dbi.size();
}
//-------------------------------------------------------------------
//  return index of first record
//-------------------------------------------------------------------
CDatabaseIndex* CDatabase::GetFirstIndex (Tag tag)
{
  CDatabaseIndex* rc = NULL;

  // Get lower and upper bounds for indices using this tag value
  dbiPair = dbi.equal_range (tag);

  // Check that lower and upper iterators are not equal
  if (dbiPair.first != dbiPair.second) {
    // Return the pointer to the database index referenced by the first iterator
    rc = dbiPair.first->second;
    dbiNext = dbiPair.first;
  }

  return rc;
}
//-------------------------------------------------------------------
//  return index of next record
//-------------------------------------------------------------------
CDatabaseIndex* CDatabase::GetNextIndex (void)
{
  CDatabaseIndex *rc = NULL;
  ++dbiNext;                                    // JSDEV* must be tested before to include last index
  if (dbiNext != dbiPair.second) {
    rc = dbiNext->second;
  }
  return rc;
}
//-------------------------------------------------------------------
// Mount the database but do not load any of the records into memory
//-------------------------------------------------------------------
void CDatabase::Mount (PODFILE *f)
{ // Confirm that valid template exists
  int nFields = dbt.GetNumItems();
  if (nFields > 0) {
    // Read header
    DBDHeader h;
    ReadULong (f, &h.signature);
    ReadULong (f, &h.unknown0);
    ReadULong (f, &h.unknown1);
    ReadULong (f, &h.checksum);
    ReadULong (f, &h.nRecords);
    ReadULong (f, &h.recLength);
    for (unsigned int i=0; i<8; i++) ReadULong (f, &h.dummy[i]);

    nRecords  = h.nRecords;
    recLength = h.recLength;
  }

  //--- Update database state -----
  state   = DB_MOUNTED;
  podfile = f;
  buf     = new char[recLength];
}
//-------------------------------------------------------------------
// Mount the database but do not load any of the records into memory
//-------------------------------------------------------------------
void CDatabase::Mount (const char* dbn)
{ char fn[128];
	_snprintf(fn,127,"DATABASE/%s",dbn);
	fn[127] = 0;
  // Mount the database, PODFILE must be left open
  PODFILE *p = popen (&globals->pfs, fn);
  if (p)  Mount (p);
}
//-------------------------------------------------------------------
void CDatabase::Unmount (void)
{
  if (podfile != 0) pclose (podfile);
}


//-------------------------------------------------------------------
// Data access methods
//-------------------------------------------------------------------
//
// Start a new search of the database.  The index and search key are saved 
//   such that subsequent matching records can be retrieved using the SearchNext
//   method
//
//  @param tag       Index key identifier
//  @param key       String value to search for
//
//  @return Byte offset of first database record matching key, or 0 if none found
//-------------------------------------------------------------------
unsigned long CDatabase::Search (Tag tag, const char* key)
{
  unsigned long rc = 0;

  // Save key value
  stringKey = key;

  // Verify that database is mounted
  if (state == DB_MOUNTED) {
    // There may be multiple indices defined on the same key tag.  Search all indices
    //   until a match is found, or key is not found in any index
    dbiSearch = GetFirstIndex (tag);
    while ((rc == 0) && (dbiSearch != NULL)) {
      rc = dbiSearch->Search (stringKey);
      if (rc == 0) dbiSearch = GetNextIndex ();
    }
  }

  return rc;
}
//-------------------------------------------------------------------
// search with double key
//-------------------------------------------------------------------
unsigned long CDatabase::Search (Tag tag, double key)
{
  unsigned long rc = 0;

  // Save key value
  doubleKey = key;

  // Verify that database is mounted
  if (state == DB_MOUNTED) {
    dbiSearch = GetFirstIndex (tag);
    while ((rc == 0) && (dbiSearch != NULL)) {
      rc = dbiSearch->Search (doubleKey);
      if (rc == 0) dbiSearch = GetNextIndex ();
    }
  }

  return rc;
}
//-------------------------------------------------------------------
//  Search with a long key
//-------------------------------------------------------------------
unsigned long CDatabase::Search (Tag tag, long key)
{
  unsigned long rc = 0;

  // Save key value
  intKey = key;

  // Verify that database is mounted
  if (state == DB_MOUNTED) {
    dbiSearch = GetFirstIndex (tag);
    while ((rc == 0) && (dbiSearch != NULL)) {
      rc = dbiSearch->Search (intKey);
      if (rc == 0) dbiSearch = GetNextIndex ();
    }
  }

  return rc;
}

//-------------------------------------------------------------------
// Continue searching using the same key values from the last call to Search()
//
// Return code:
//  unsigned long Record pointer of next match
//-------------------------------------------------------------------
unsigned long CDatabase::SearchNext (void)
{
  unsigned long rc = 0;

  // Verify that database is mounted or loaded
  if ((state == DB_MOUNTED) && (dbiSearch != NULL)) {
    while ((rc == 0) && (dbiSearch != NULL)) {
      rc = dbiSearch->SearchNext ();
      if (rc == 0) {
        dbiSearch = GetNextIndex ();
        if (dbiSearch != NULL) {
          switch (dbiSearch->GetKeyType()) {
            case 'C':
              rc = dbiSearch->Search (stringKey);
              break;
            case 'D':
              rc = dbiSearch->Search (doubleKey);
              break;
            case 'I':
              rc = dbiSearch->Search (intKey);
              break;
          }
        }
      }
    }
  }

  return rc;
}

//-------------------------------------------------------------------
// Retrieve a raw data record from the database.  The application is responsible
//   for decoding and formatting the fields
//-------------------------------------------------------------------
void CDatabase::GetRawRecord (unsigned long offset)
{
  // Verify that database is mounted
  if (state == DB_MOUNTED) {
    pseek (podfile, offset, SEEK_SET);
    pread (buf, recLength, 1, podfile);
  }
}
//-------------------------------------------------------------------
//  Retrieve a record knowing its offset
//-------------------------------------------------------------------
bool CDatabase::GetRecordByOffset (unsigned long offset, CDatabaseRecord & rec)
{
  bool rc = true;
  rec.Reset();
  GetRawRecord (offset);
  CDatabaseField f;

  for (unsigned long i=0; i<GetNumFields(); i++) {
    CDatabaseTemplateItem *t = dbt.GetItem (i);
    f.type = t->type;
    f.length = t->rawLength;
    void *pData = buf + t->rawOffset;
    switch (t->type) {
    case 'C':
      if (t->rawLength < 256) {
        strncpy (f.data.charData, (char*)pData, t->rawLength);
      } else {
        gtfo ("CDatabase::GetRecord : Char field length > 255 chars");
      }
      break;

    case 'D':
      f.data.doubleData = *((double*)pData);
      break;

    case 'I':
      f.data.intData = *((long*)pData);
      break;
    }
    rec.AddField (f);
  }

  return rc;
}
//-------------------------------------------------------------------
//   Get a field with its index  (NOT USED)
//-------------------------------------------------------------------
bool CDatabase::GetFieldByIndex(unsigned long index, CDatabaseField & outField, unsigned int fieldPos)
{
  bool rc = false;
  memset(&outField, 0x00, sizeof(CDatabaseField));
  unsigned long offset = RecordOffset (index);
  
  if(fieldPos < (unsigned int)(dbt.GetNumItems()))
  {
    rc = true;
    GetRawRecord (offset);
    CDatabaseTemplateItem *t = dbt.GetItem (fieldPos);
    outField.type = t->type;
    outField.length = t->rawLength;
    void *pData = buf + t->rawOffset;
    switch (t->type)
    {
    case 'C':
      if (t->rawLength < 256) {
        strncpy (outField.data.charData, (char*)pData, t->rawLength);
      } else {
        gtfo ("CDatabase::GetRecord : Char field length > 255 chars");
      }
      break;

    case 'D':
      outField.data.doubleData = *((double*)pData);
      break;

    case 'I':
      outField.data.intData = *((long*)pData);
      break;
    }
  }
  return rc;
}
//---------------------------------------------------------------------------
//  Locate record by offset
//---------------------------------------------------------------------------
bool CDatabase::GetRecordByIndex (unsigned long index, CDatabaseRecord & rec)
{
  bool rc = false;
  if(index < nRecords)
    rc = GetRecordByOffset (RecordOffset (index), rec);
  return rc;
}
//---------------------------------------------------------------------------
// Dump the database contents to a file for debugging
//---------------------------------------------------------------------------
void CDatabase::Dump (FILE *f)
{
  // Dump template
  fprintf (f, "Template:\n");
  fprintf (f, "=========\n");
  dbt.Dump (f);
  fprintf (f, "\n");

  // Dump indices
  fprintf (f, "Indices:\n");
  fprintf (f, "========\n");
  std::multimap<Tag,CDatabaseIndex*>::iterator i;
  for (i=dbi.begin(); i!=dbi.end(); i++) {
    i->second->Dump (f);
  }
  fprintf (f, "\n");

  // Dump database state
  fprintf (f, "State : ");
  switch (state) {
  case DB_UNMOUNTED:
    fprintf (f, "UNMOUNTED\n");
    break;

  case DB_MOUNTED:
    fprintf (f, "MOUNTED\n");
    break;
  }

  // Dump all data records
  if (state == DB_MOUNTED) {
    int nRecords = GetNumRecords ();
    fprintf (f, "  nRecords = %d\n", nRecords);
    int nFields = GetNumFields ();
    fprintf (f, "  nFields  = %d\n", nFields);

    int i, j;
    CDatabaseRecord record;
    for (i=0; i < nRecords; i++) {
      GetRecordByIndex (i, record);
      for (j=0; j < nFields; j++) {
        CDatabaseField *field =  record.GetField (j);
        char s[256];
        field->Format (s);
        if (j != 0) fprintf (f, ", ");
        fprintf (f, "%s", s);
      }
      fprintf (f, "\n");
    }
  }
}
//=============END OF FILE ============================================================