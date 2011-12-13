/*
 * Ini.h
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

/*! \file Ini.h
 *  \brief Defines CIniFile class for application access to INI file settings
 */

#ifndef INI_H
#define INI_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "FlyLegacy.h"
#include <map>
#include <string>


typedef enum {
  INI_UNKNOWN_SETTING,
  INI_FLOAT_SETTING,
  INI_INT_SETTING,
  INI_STRING_SETTING
} EIniSettingType;


/*
 * CIniSetting
 */

class CIniSetting {
public:
  // Constructor
  CIniSetting (const char* key);
  CIniSetting (const char* key, float f);
  CIniSetting (const char* key, int i);
  CIniSetting (const char* key, const char* s);

  // CIniSetting methods
  void    Init (const char* key);
  void    Set (float f);
  void    Set (int i);
  void    Set (const char* s);
  void    Get (float* f);
  void    Get (int* i);
  void    Get (char* s, int maxLength);
  void    Save (FILE *f);
  const   char *GetValue();

public:
  EIniSettingType type;
  char      key[64];
  float     value;
  char      s_value[PATH_MAX];
};


/*
 * CIniSection
 */

class CIniSection {
public:
  // Constructor
  CIniSection (const char* section);
  ~CIniSection (void);

  // CIniSection methods
  void    Set (const char* key, float f);
  void    Set (const char* key, int i);
  void    Set (const char* key, const char* s);
  void    Get (const char* key, float *f);
  void    Get (const char* key, int *i);
  void    Get (const char* key, char *s, int maxLength);
  const char *GetValue(const char *key);
	bool		GetKey(char *key);
	void    Save (FILE *f);
  void    Remove (const char* key);

protected:
  // CIniSection methods
  CIniSetting*  FindSetting (const char* key);
  CIniSetting*  FindSettingOrCreate (const char* key);
  void          Clear (void);

public:
  char                                section[64];  // Section name
  std::map<std::string,CIniSetting*>  setting;      // Setting map indexed by key name
};


/*
 * CIniFile
 */

class CIniFile {
public:
  // Constructor
  CIniFile (void);
  CIniFile (const char* iniFilename);
  ~CIniFile (void);

  // CIniFile methods
  int       Load (const char* iniFilename);
  int       Merge (const char* iniFilename);
  int       Save (const char* iniFilename);
  void      Set (const char* section, const char* key, float f);
  void      Set (const char* section, const char* key, int i);
  void      Set (const char* section, const char* key, const char* s);
  void      Get (const char* section, const char* key, float* f);
  void      Get (const char* section, const char* key, int* i);
  void      Get (const char* section, const char* key, char* s, int maxLength);
  const char *GetValue (const char* section, const char* key);
	bool			GetKey(char *section, char *key);
  void      Remove (const char* section, const char* key);
  int       GetNumSections (void);
  char*     GetSectionName (int i);
  inline	bool IsSectionHere(const char *sect) {return (0 != FindSection(sect)); }	

protected:
  // CIniFile methods
  CIniSection*  FindSection (const char* sectname);
  CIniSection*  FindSectionOrCreate (const char* sectname);
  void          Clear (void);

protected:
  std::map<std::string,CIniSection*>  section;    // Section map indexed by name
};

#endif // INI_H
