/*
 * Ini.cpp
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

/*! \file Ini.cpp
 *  \brief Implements API functions for accessing INI file settings
 */

#include "../Include/FlyLegacy.h"
#include "../Include/Ini.h"
#include <direct.h>
using namespace std;


//
// CIniSetting - Individual setting in an INI file
//
CIniSetting::CIniSetting (const char* key)
{
  Init (key);
}

CIniSetting::CIniSetting (const char* key, float f)
{
  Init (key);
  Set (f);
}

CIniSetting::CIniSetting (const char* key, int i)
{
  Init (key);
  Set (i);
}

CIniSetting::CIniSetting (const char* key, const char* s)
{
  Init (key);
  Set (s);
}

//
// Initialize defaults for setting
//
void CIniSetting::Init (const char* key)
{
  type = INI_INT_SETTING;
  strncpy (this->key, key, PATH_MAX);

  value = 0;
  memset (s_value, 0, PATH_MAX);
}

//
// Set a float INI setting
//
void CIniSetting::Set (float f)
{
  type = INI_FLOAT_SETTING;
  value = f;
}

//
// Set an integer INI setting
//
void CIniSetting::Set (int i)
{
  type = INI_INT_SETTING;
  value = (float)i;
}

//
// Set a string INI setting
//
void CIniSetting::Set (const char* s)
{
  type = INI_STRING_SETTING;
  strncpy (s_value, s, PATH_MAX);
  value = 0;
}


//
// Get a float INI setting
//
void CIniSetting::Get (float *f)
{
  type = INI_FLOAT_SETTING;
  *f = value;
}

//
// Get an integer INI setting
//
void CIniSetting::Get (int *i)
{ type = INI_INT_SETTING;
  *i = (int)value;
}

//
// Get a string INI setting
//
void CIniSetting::Get (char *s, int maxLength)
{
  type = INI_STRING_SETTING;
  strncpy (s, s_value, maxLength);
}

// JSDEV* Get pointer to string value to save room
const char *CIniSetting::GetValue()
{	type = INI_STRING_SETTING;
	return s_value;	}

void CIniSetting::Save (FILE *f)
{
  switch (type) {
  case INI_FLOAT_SETTING:
    fprintf (f, "%s=%f\n", key, value);
    break;

  case INI_INT_SETTING:
    fprintf (f, "%s=%d\n", key, (int)value);
    break;

  case INI_STRING_SETTING:
    fprintf (f, "%s=%s\n", key, s_value);
    break;
    
  default:
    gtfo ("CIniSetting::Save : Invalid INI setting type %d", type);
  }
}

//
// CIniSection - Group of related settings in an INI file
//
CIniSection::CIniSection (const char* section)
{
  strncpy (this->section, section, 64);
}

CIniSection::~CIniSection (void)
{
  Clear ();
}

//
// Remove all settings from this section
//
void CIniSection::Clear (void)
{
  std::map<string,CIniSetting*>::iterator i;
  for (i=setting.begin(); i!=setting.end(); i++) delete (*i).second;
  setting.clear();
}

//
// Protected method to find the index of the specified INI section if it exists
//
// Input Parameters:
//  key       Attribute key name to search for
//
// Return Code:
//  CIniSetting*  Pointer to CIniSetting. This should never be NULL; if the
//          setting of the specified name does not exist it will be created
//
CIniSetting* CIniSection::FindSetting (const char* key)
{
  CIniSetting *rc = NULL;
  std::map<string,CIniSetting*>::iterator i = setting.find(key);
  if (i != setting.end())  rc = i->second;
  return rc;
}

CIniSetting* CIniSection::FindSettingOrCreate (const char* key)
{
  CIniSetting *rc = FindSetting (key);
  if (rc == NULL) {
    // Setting not found, add it to the section
    rc = new CIniSetting (key);
    setting[key] = rc;
  }
  return rc;
}

//
// Set a float INI setting
//
void CIniSection::Set (const char* key, float f)
{
  // Get reference to CIniSetting, or create it if it does not exist
  CIniSetting* setting = FindSettingOrCreate (key);
  if (setting != NULL) {
    // Set the attribute value
    setting->Set (f);
  }
}

//
// Set an integer INI setting
//
void CIniSection::Set (const char* key, int i)
{
  // Get reference to CIniSetting, or create it if it does not exist
  CIniSetting* setting = FindSettingOrCreate (key);
  if (setting != NULL) {
    // Set the attribute value
    setting->Set (i);
  }
}

//
// Set a string INI setting
//
void CIniSection::Set (const char* key, const char* s)
{
  // Get reference to CIniSetting, or create it if it does not exist
  CIniSetting* setting = FindSettingOrCreate (key);
  if (setting != NULL) {
    // Set the attribute value
    setting->Set (s);
  }
}

//
// Get a float INI setting
//
void CIniSection::Get (const char* key, float* f)
{
  // Get reference to CIniSetting, or create it if it does not exist
  CIniSetting* setting = FindSetting (key);
  if (setting != NULL) {
    // Set the attribute value
    setting->Get (f);
  }
}

//
// Get an integer INI setting
//
void CIniSection::Get (const char* key, int* i)
{
  // Get reference to CIniSetting, or create it if it does not exist
  CIniSetting* setting = FindSetting (key);
  if (setting != NULL) {
    // Set the attribute value
    setting->Get (i);
  }
}

//
// Get a string INI setting
//
void CIniSection::Get (const char* key, char* s, int maxLength)
{
  // Get reference to CIniSetting, or create it if it does not exist
  CIniSetting* setting = FindSetting (key);
  if (setting != NULL) {
    // Set the attribute value
    setting->Get (s, maxLength);
  }
}
//---------------------------------------------------------------------
//	JSDEV* return a pointer to the value
//---------------------------------------------------------------------
const char *CIniSection::GetValue(const char *key)
{	CIniSetting *set = FindSetting(key);
	if (set == NULL) return NULL;
	return set->GetValue();	
}
//---------------------------------------------------------------------
//	JSDEV* return a keyword
//---------------------------------------------------------------------
bool CIniSection::GetKey(char *key)
{ CIniSetting *set = FindSetting(key);
	if (0 == set)	return false;
	const char *val = set->GetValue();
	return (strcmp(val, "*empty*") != 0);
}
//---------------------------------------------------------------------
//	Check a keyword
//---------------------------------------------------------------------
bool CIniSection::HasKey(char *key)
{ CIniSetting *set = FindSetting(key);
	return (set != 0);
}

//---------------------------------------------------------------------
// Save contents of the section to file
//---------------------------------------------------------------------
void CIniSection::Save (FILE *f)
{
  fprintf (f, "[%s]\n", section);
  std::map<string,CIniSetting*>::iterator i;
  for (i=setting.begin(); i!=setting.end(); i++) {
    i->second->Save(f);
  }

  fprintf (f, "\n");
}

//
// Remove the INI entry with the specified key
//
void CIniSection::Remove (const char* key)
{
}

//
// CIniFile - Collection of INI settings in a single disk file
//

CIniFile::CIniFile (void)
{
}


CIniFile::CIniFile (const char* iniFilename)
{
  Load (iniFilename);
}


CIniFile::~CIniFile (void)
{
  Clear ();
}

//--------------------------------------------------------------------------------
// Clear the contents of the INI settings
//--------------------------------------------------------------------------------
void CIniFile::Clear (void)
{
  std::map<string,CIniSection*>::iterator i;
  for (i=section.begin(); i!=section.end(); i++) {
    delete (*i).second;
  }
	section.clear();
}

//-------------------------------------------------------------------------------
// Protected method to find the index of the specified INI section if it exists,
//   and to create it if it doesn't
//
// Input Parameters:
//  section     Section name to search for
//
// Return Code:
//  CIniSection*  Pointer to CIniSection. This should never be NULL; if the
//          section of the specified name does not exist it will be created
//--------------------------------------------------------------------------------
CIniSection* CIniFile::FindSection (const char* sectname)
{
  CIniSection *rc = NULL;
  std::map<string,CIniSection*>::iterator i = section.find(sectname);
  if (i != section.end())   rc = i->second;
  return rc;
}
//-------------------------------------------------------------------------------
//	Find section or create one
//-------------------------------------------------------------------------------
CIniSection* CIniFile::FindSectionOrCreate (const char* sectname)
{ CIniSection *rc = FindSection (sectname);
  if (rc)		return rc;
  // Section not found, create it
  rc = new CIniSection (sectname);
  section[sectname] = rc;
  return rc;
}
//==============================================================================
//  JS function isspace bugs for some characters like °
//==============================================================================
bool WhiteSpace(char k)
{ if (0x20 == k)  return true;
  if (0x0D == k)  return true;
  if (0x0A == k)  return true;
  if (0x09 == k)  return true;
  return false;
}
//==============================================================================
// Local function that trims trailing whitespace from a line of text
//==============================================================================
void TrimTrailingWhitespace (char* s)
{
  int i = strlen(s) - 1;
//  while ((i >= 0) && isspace(s[i])) s[i--] = '\0';
  while ((i >= 0) && WhiteSpace(s[i])) s[i--] = '\0';
}

//------------------------------------------------------------------------------
// Local function which parses the supplied line, looking for a new INI section
//   header of the form [Section].
//
// Input parameters:
//  s     String to parse
//
// Output parameters:
//  section   Name of the section found, if rc == true
//
// Return code:
//  bool    true if a new [Section] was found
//-----------------------------------------------------------------------
static bool ParseSection (char* s, char* section)
{
  bool rc = false;

  TrimTrailingWhitespace (s);
	int nf = sscanf(s," [ %64[^ /]s ]", section); 
	if (0 == nf)	return false;
	char *p = strchr(section,']');
	if (0 == p)		return true;
	*p = 0;
	return true;
}
	
//-------------------------------------------------------------------------------
// Local function which parses the supplied line and returns the key and value
//   (strings) if the line conforms to the standard INI format key=value
//-------------------------------------------------------------------------------
static bool ParseKeyValue (char* s, char* key, char* value)
{
  TrimTrailingWhitespace (s);

  char *p = strchr (s, '=');
  if (p != NULL) {
    // Copy key string
    int len = strlen(s) - strlen(p);
    strncpy (key, s, len);
    key[len] = 0;

    // Copy value string
    strcpy (value, p+1);
		return true;
  }
	//--- Just check for one key -----
	int len = strlen(s);
	if (len)
	{	strncpy(key,s,len);
		key[len] = 0;
		strcpy(value,"*empty*");
		return true;
	}
  return false;
}
//-------------------------------------------------------------------------------------------
//	Check for empty line
//-------------------------------------------------------------------------------------------
bool CIniFile::EmptyLine(char *s)
{	if ((strncmp (s, "\n", 1) == 0)) return true;
	if (*s == '§')									 return true;
	if (strncmp(s,"//",2) == 0)			 return true;
	return false;
}
//-------------------------------------------------------------------------------------------
// Load (or re-load) the INI settings from disk file.  All existing settings are cleared.
//
// Input parameters:
//  iniFilename   Name of normal disk file (not POD file) containing INI settings
//
// Return code:
//  int       1 if settings were successfully loaded; 0 if file could not be opened
//--------------------------------------------------------------------------------------------
int CIniFile::Load (const char* iniFilename)
{ char buf[1024];
  _getcwd( buf,1024);		// Current working directory
  // Clear all existing INI settings
  Clear ();
  // Declare local variables used in parsing of INI file lines
  char sect[64];
  char key[64];
  char value[PATH_MAX];
  float f_value;
  // Open normal (non-POD) file
  FILE *f = fopen (iniFilename, "r");
  if (0 == f) return 0;
  // Begin parsing file
  char s[PATH_MAX];
  while (!feof (f)) {
   *s = '§';
		fgets (s, PATH_MAX, f);
		s[FNAM_MAX] = 0;
		//--- ignore empty lines --------------------------
		if (EmptyLine(s))						continue;
		//--- Process staement ----------------------------
    // First check for a new section header : [Section]
    if (ParseSection (s, sect))			      continue;	//fgets (s, PATH_MAX, f);
    // New section header found
    // No new section header found, parse for key/value pair
		
    if (!ParseKeyValue (s, key, value))		continue;
    //  Store the string anyway
    Set (sect, key, value);
    // Parse the value to determine whether it is numeric (int/float)
    //   or string.
		if (sscanf (value, "%f ", &f_value) == 1) Set (sect, key, f_value); 
	 }
   fclose (f);
	 return (section.size() != 0);
}

//==============================================================================
// Merge the contents of an INI settings file with the currently loaded settings
//==================================================================================
int CIniFile::Merge (const char* iniFilename)
{
  int rc = 0;

  WARNINGLOG ("CIniFile::Merge not implemented yet.");

  return rc;
}

//
// Save the INI settings to a disk file
//
int CIniFile::Save (const char* iniFilename)
{
  int rc = 0;

  // Open disk file
  FILE *f = fopen (iniFilename, "w");
  if (f) {
    std::map<string,CIniSection*>::iterator i;
    for (i=section.begin(); i!=section.end(); i++) {
      i->second->Save(f);
    }

    fclose (f);

    // Set success return code
    rc = 1;
  } else {
    WARNINGLOG ("CIniFile : Unable to save INI file to %s", iniFilename);
  }

  return rc;
}


//-----------------------------------------------------------------------
// Set a float INI setting
//-----------------------------------------------------------------------
void CIniFile::Set (const char* section, const char* key, float f)
{ // Get reference to CIniSection, or create it if it does not exist
  CIniSection* sect = FindSectionOrCreate (section);
  sect->Set (key, f);
	return;
}


//-----------------------------------------------------------------------
// Set an integer INI setting
//-----------------------------------------------------------------------
void CIniFile::Set (const char* section, const char* key, int i)
{ // Get reference to CIniSection, or create it if it does not exist
  CIniSection* sect = FindSectionOrCreate (section);
  // Set the key/value attribute in the section
  sect->Set (key, i);
  return;
}


//------------------------------------------------------------------------
// Set a string INI setting
//------------------------------------------------------------------------
void CIniFile::Set (const char* section, const char* key, const char* s)
{ // Get reference to CIniSection, or create it if it does not exist
  CIniSection* sect = FindSectionOrCreate (section);
  // Set the key/value attribute in the section
  sect->Set (key, s);
  return;
}

//------------------------------------------------------------------------
// Get a float INI setting
//-----------------------------------------------------------------------
void CIniFile::Get (const char* section, const char* key, float* f)
{
  // Get reference to CIniSection, or create it if it does not exist
  CIniSection* sect = FindSection (section);
  if (sect != NULL) {
    // Section exists, update the return parameter
    sect->Get (key, f);
  }
}

//-----------------------------------------------------------------------
// Get an integer INI setting
//-----------------------------------------------------------------------
void CIniFile::Get (const char* section, const char* key, int* i)
{
  // Get reference to CIniSection, or create it if it does not exist
  CIniSection* sect = FindSection (section);
  if (sect != NULL) {
    // Section exists, update the return parameter
    sect->Get (key, i);
  }
}

//-----------------------------------------------------------------------
// Get an string INI setting
//-----------------------------------------------------------------------
void CIniFile::Get (const char* section, const char* key, char* s, int maxLength)
{
  // Get reference to CIniSection, or create it if it does not exist
  CIniSection* sect = FindSection (section);
  if (sect != NULL) {
    // Section exists, update the return parameter
    sect->Get (key, s, maxLength);
  }
}
//---------------------------------------------------------------------
//	Return a pointer to string value
//--------------------------------------------------------------------
const char *CIniFile::GetValue (const char* section, const char* key)
{
  // Get reference to CIniSection, or create it if it does not exist
  CIniSection* sect = FindSection (section);
  if (sect == NULL) return NULL;
    // Section exists, update the return parameter
   return sect->GetValue (key);
  }
//---------------------------------------------------------------------
//	Return a keyword
//--------------------------------------------------------------------
bool CIniFile::GetKey(char *section, char *key)
{	 CIniSection* sect = FindSection (section);
	 if (0 == sect)		return false;
	 return sect->GetKey(key);
}
//---------------------------------------------------------------------
//	Check for a keyword
//--------------------------------------------------------------------
bool CIniFile::HasKey(char *section, char *key)
{	 CIniSection* sect = FindSection (section);
	 if (0 == sect)		return false;
	 return sect->HasKey(key);
}

//-------------------------------------------------------------------
// Remove a setting from the INI file
//-------------------------------------------------------------------
void CIniFile::Remove (const char* section, const char* key)
{
  // Get reference to CIniSection, or create it if it does not exist
  CIniSection* sect = FindSection (section);
  if (sect != NULL) {
    // Set the key/value attribute in the section
    sect->Remove (key);
  }
}

int CIniFile::GetNumSections (void)
{
  return section.size();
}

//---------------------------------------------------------------------------------
//	todo This should be restructured as GetFirst/GetNext to align better with
//     STL map<> implementation rather than flat array implementation
//----------------------------------------------------------------------------------
char* CIniFile::GetSectionName (int i)
{
  char* rc = NULL;

  std::map<string,CIniSection*>::iterator iter = section.begin();
  for (int count=i; (iter != section.end()) && (count > 0); count--, iter++);
  CIniSection *pSection = iter->second;
  rc = pSection->section;

  return rc;
}


//=========================================================================
// INI functions defined in FlyLegacy.h
//=========================================================================
static CIniFile *ini = 0;
//=========================================================================
//	Load FlyLegacy.ini
//=========================================================================
void LoadIniSettings (void)
{ ini = new CIniFile (); 
	// Attempt to load INI settings from FlyLegacy.ini
  if (ini->Load ("System/FlyLegacy.ini"))				return;	
	TRACE("Invalid FlyLegecay.ini in /system");
  gtfo ("LoadIniSettings : Cannot read settings from Fly.ini");
   
   /// \todo Delete Fly! II settings that are not supported
  }
//-------------------------------------------------------------------------
//	Unload =>Delete the file
//-------------------------------------------------------------------------
void UnloadIniSettings (void)
{
  delete ini;
	ini = 0;
}

void SaveIniSettings (void)
{
  ini->Save ("System/FlyLegacy.ini");
}

//----------------------------------------------------------------------
//	Return integer value from [sect] parm=value
//----------------------------------------------------------------------
void  GetIniVar(const char *section, const char *varname, int *value)
{
  ini->Get (section, varname, value);
}
//----------------------------------------------------------------------
//	Return float value
//----------------------------------------------------------------------
void  GetIniFloat(const char *section, const char *varname, float *value)
{  ini->Get (section, varname, value);	}
//----------------------------------------------------------------------
//	Return string value
//----------------------------------------------------------------------
void  GetIniString(const char *section, const char *varname, char *strvar, int maxLength)
{  ini->Get (section, varname, strvar, maxLength);	}
//----------------------------------------------------------------------
//	Return string pointer
//----------------------------------------------------------------------
const char *GetIniValue(const char *section, const char *key)
{	return ini->GetValue(section,key);	}
//----------------------------------------------------------------------
//	Return parameter
//----------------------------------------------------------------------
bool  GetIniKey(char *section, char *key)
{	return ini->GetKey(section,key);	}
//----------------------------------------------------------------------
//	Check for key declaration
//----------------------------------------------------------------------
bool  HasIniKey(char *section, char *key)
{	return ini->HasKey(section,key);	}

//======================================================================

void  SetIniVar(const char *section, const char *varname, int value)
{
  ini->Set (section, varname, value);
}


void  SetIniFloat(const char *section, const char *varname, float value)
{
  ini->Set (section, varname, value);
}


void  SetIniString(const char *section, const char *varname, char *strvar)
{
  ini->Set (section, varname, strvar);
}

bool IsSectionHere (const char *section)
{	return	ini->IsSectionHere(section);	}


