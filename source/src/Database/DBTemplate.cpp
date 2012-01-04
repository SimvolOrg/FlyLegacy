/*
 * DBTemplate.cpp
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
 */

/*! \file DBTemplate.cpp
 *  \brief Implements CDatabaseTemplate database template class
 *
 * Implements DBTempate which encapsulates a database template stored
 *   in a DBT file.  A database template is a text-based file that
 *   describes the field structure of records for a particular database.
 *   The template must specified in the constructor of DBDatabase so that
 *   data records in the DBD file can be interpreted.
 */

#include "../Include/Globals.h"
#include "../Include/Database.h"
#include "../Include/Utility.h"
#include "../Include/Pod.h"

using namespace std;


CDatabaseTemplateItem::CDatabaseTemplateItem (void)
{
  id = 0;
  rawOffset = rawLength = 0;
  type = 0;
  memset (name, 0, 256);
}


CDatabaseTemplate::~CDatabaseTemplate (void)
{
  std::vector<CDatabaseTemplateItem*>::iterator i;
  for (i=item.begin(); i!=item.end(); i++) {
    delete *i;
  }
}

//
// Loader which takes an open PODFILE* reference to the DBT file
//
void CDatabaseTemplate::Load(PODFILE* f)
{
  int i, j;

  // Buffer for file input
  char buffer[256];

  // Ensure file is rewound
  prewind (f);

  // Read number of fields
  int nItems;
  pgets (buffer, 256, f);
  if (sscanf (buffer, "%d", &nItems) != 1) {
    gtfo ("CDatabaseTemplate::Load : Cannot read nItems");
  }

  // Allocate storage for template items
  item.reserve(nItems);
 
  // Parse each field
  for (i=0; i<nItems; i++) {
    CDatabaseTemplateItem *ti = new CDatabaseTemplateItem;

    pgets (buffer, 256, f);

    // Parse input line into individual fields
    char sId[8], cType, sLeft[256], sRight[256];
    memset (sId, 0, 8);
    unsigned long rawOffset, rawLength;

    // Split input line at point of final comma separator
    char *sSplit = strrchr (buffer, ',');
    int nLeft = sSplit - buffer;
    strncpy (sLeft, buffer, nLeft);
    strncpy (sRight, sSplit+1,255);

    // Parse left-hand portion of input line into data fields
    if (sscanf (sLeft, "%4s,%lu,%lu,%c", sId, &rawOffset, &rawLength, &cType) != 4) {
      gtfo ("CDatabaseTemplate::Load : Cannot read item %d", i);
    } else {
      // Record parsed successfully
      ti->id = StringToTag (sId);
      ti->rawOffset = rawOffset;
      ti->rawLength = rawLength;
      ti->type = cType;
    }

    // Copy right-hand portion of input line as field description, trim trailing whitespace
    for (j=strlen(sRight)-1; j>=0; j--) {
      if ((sRight[j] == '\n') ||
        (sRight[j] == ' ')  ||
        (sRight[j] == 13) ||
        (sRight[j] == '\t'))
      {
        // Replace trailing whitespace with null
        sRight[j] = '\0';
      } else {
        // Non-whitespace found; exit loop
        break;
      }
    }
    strncpy (ti->name, sRight,255);

    // Add the new template item to the vector
    item.push_back (ti);
  }
}


//
// Constructor which accepts a const char* filename of the DBT file
//
void CDatabaseTemplate::Load (const char* dbn)
{
  char fn[128];
	_snprintf(fn,127,"DATABASE/%s",dbn);
  PODFILE *p = popen (&globals->pfs, dbn);

  if (p) {
    Load (p);
    pclose (p);
  }
}

//
// Data access function to get the number of template items
//
int CDatabaseTemplate::GetNumItems ()
{
  return item.size();
}

//
// Data access function to get the specified template item details
//
CDatabaseTemplateItem *CDatabaseTemplate::GetItem (int i)
{
  CDatabaseTemplateItem *rc = NULL;

  if (i < GetNumItems()) {
    rc = item[i];
  }

  return rc;
}

CDatabaseTemplateItem* CDatabaseTemplate::GetItemByTag (Tag tag)
{
  CDatabaseTemplateItem* rc = NULL;

  std::vector<CDatabaseTemplateItem*>::iterator i;
  for (i=item.begin(); i!=item.end(); i++) {
    if ((*i)->id == tag) {
      rc = (*i);
      break;
    }
  }

  return rc;
}

int CDatabaseTemplate::GetOffset (Tag tag)
{
  int rc = 0;
  CDatabaseTemplateItem* item = GetItemByTag (tag);
  if (item != NULL) {
    rc = item->rawOffset;
  }
  return rc;
}

int CDatabaseTemplate::GetLength (Tag tag)
{
  int rc = 0;
  CDatabaseTemplateItem* item = GetItemByTag (tag);
  if (item != NULL) {
    rc = item->rawLength;
  }
  return rc;
}

void CDatabaseTemplate::Dump (FILE *f)
{
  char s[8];
  std::vector<CDatabaseTemplateItem*>::iterator i;
  for (i=item.begin(); i!=item.end(); i++) {
    TagToString (s, (*i)->id);
    fprintf (f, "%4s,%d,%d,%c,%s\n",
      s,
      (*i)->rawOffset,
      (*i)->rawLength,
      (*i)->type,
      (*i)->name);
  }
}
