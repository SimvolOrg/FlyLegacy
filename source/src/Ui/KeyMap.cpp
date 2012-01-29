/*
 * KeyMap.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
 * Copyright 2008 Jean Sabatier
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

/*! \file KeyMap.cpp
 *  \brief Implements CKeyMap and related classes for key mapping to sim actions
 *
 *  Implements the CKeyMap class as well as supporting classes CKeySet
 *    and CKeyDefinition.  As a descendent of CStreamObject, CKeyMap
 *    parses the key map definitions in \System\default.key.  When
 *    a keypress is detected in the application main loop, the
 *    CKeyMap class is queried to see if the key has been assigned to
 *    any of the simulator action key IDs.
 */

#include "../Include/KeyMap.h"
#include "../Include/Utility.h"
#include "../Include/Globals.h"
#include "../Include/Ui.h"
#include "../Include/FuiParts.h"
#include "../Include/FuiKeyMap.h"

using namespace std;
//===========================================================================
extern bool GroupUnbind(CKeyDefinition *kdf,int code);
//===========================================================================
// CKeyDefinition
//===========================================================================
CKeyDefinition::CKeyDefinition (Tag ks)
: kyid (0), code (0), user (true), enab (true), cb (NULL)
{ kset = ks;
 *name  = 0;
  jbtn  = 0;
  type  = 0;
}

//--------------------------------------------------------------
//  Read Parameters
//--------------------------------------------------------------
int CKeyDefinition::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'kyid':
    // Key unique ID
    ReadTag (&kyid, stream);
    rc = TAG_READ;
    break;

  case 'name':
    // Key name
    ReadString (name, 64, stream);
    rc = TAG_READ;
    break;

  case 'code':
    // Key code and modifier
    ReadInt (&code, stream);
    rc = TAG_READ;
    break;

  case 'user':
    // User-mappable?
    {
      int i;
      ReadInt (&i, stream);
      user = (i != 0);
    }
    rc = TAG_READ;
    break;

  case 'enab':
    // Enabled?
    {
      int i;
      ReadInt (&i, stream);
      enab = (i != 0);
    }
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    globals->logWarning->Write ("CKeyDefinition::Read : Unknown tag %s", TagToString (tag));
  }
  
  return rc;
}
//---------------------------------------------------------------------------
//  Link to new button, return previous
//---------------------------------------------------------------------------
CSimButton *CKeyDefinition::LinkTo(CSimButton *bt)
{ CSimButton *old = jbtn; 
  jbtn = bt; 
  return old;
}
//===========================================================================
// CKeySet
//===========================================================================
CKeySet::CKeySet (Tag tag)
: kset (tag), user (true), enab (true)
{ gcb   = GroupUnbind;                // Group callback
 *name  = 0;
}
//-------------------------------------------------------------------------
//  Destroy and release resources
//-------------------------------------------------------------------------
CKeySet::~CKeySet (void)
{
  // Delete key definitions
  std::map<Tag,CKeyDefinition*>::iterator i;
  for (i=dkey.begin(); i!=dkey.end(); i++)  delete i->second;
	dkey.clear();
  ckey.clear();
}
//-------------------------------------------------------------------------
//  Enter a key definition
//  The key is registered in 2 maps
//  dkey by the generic tag
//  ckey by the keyboard code (for fast access)
//-------------------------------------------------------------------------
void CKeySet::Enter(CKeyDefinition *kdf)
{ Tag gen   = kdf->GetTag();
  int cod   = kdf->GetCode();
	dkey[gen] = kdf;							// Register by Tag
  //---Check that the code is unique in the set --
  std::map<int,CKeyDefinition *>::iterator it = ckey.find(cod);
	//---Register if key is unic in set ------
  if (it == ckey.end())	{ ckey[cod] = kdf; return;	}
  //----Can't have same code for 2 key ------
  if (0 == cod)              return;
  if ((*it).second == kdf)   return; 
  //----------------------------------------------
	kdf->SetCode(0);							// Clear Keyboard key
  CKeyDefinition *pvk = (*it).second;
  char tag1[8];
  TagToString(tag1,kdf->GetTag());
  char tag2[8];
  TagToString(tag2,pvk->GetTag());
  WARNINGLOG("<Kyid> %s ignored (same keyboard code than <Kyid> %s)",tag1,tag2);
  return;
}
//-------------------------------------------------------------------------
//  Enter code into code table
//-------------------------------------------------------------------------
void CKeySet::StoreKeyDef(CKeyDefinition *kdf)
{ int cod   = kdf->GetCode();
  ckey[cod] = kdf;
  return;
}
//-------------------------------------------------------------------------
//  Clear code
//-------------------------------------------------------------------------
void CKeySet::ClearCode(int cod)
{ ckey.erase(cod);
  return;
}
//-------------------------------------------------------------------------
//  Call the group function
//-------------------------------------------------------------------------
bool CKeySet::CallGroup(CKeyDefinition *kdf,int mod)
{ return (gcb)(kdf,mod) ;
}

//-------------------------------------------------------------------------
//  Return definition by code
//-------------------------------------------------------------------------
CKeyDefinition* CKeySet::GetKeyByCode(int c)
{ std::map<int,CKeyDefinition*>::iterator it = ckey.find(c);
  return (it == ckey.end())?(0):((*it).second);
}
//-------------------------------------------------------------------------
//  Return definition by Tag
//-------------------------------------------------------------------------
CKeyDefinition* CKeySet::GetKeyByTag(Tag t)
{ std::map<Tag,CKeyDefinition*>::iterator it = dkey.find(t);
  return (it == dkey.end())?(0):((*it).second);
}
//-------------------------------------------------------------------------
//  Read All parameters
//-------------------------------------------------------------------------
int CKeySet::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'name':
    // Key set name
    ReadString (name, 64, stream);
    rc = TAG_READ;
    break;

  case 'user':
    // Are keys in this key set user-mappable?
    {
      int i;
      ReadInt (&i, stream);
      user = (i != 0);
    }
    rc = TAG_READ;
    break;

  case 'enab':
    // Is this key set enabled?
    {
      int i;
      ReadInt (&i, stream);
      enab = (i != 0);
    }
    rc = TAG_READ;
    break;

  case 'kkey':
    // Key definition sub-object
    {
      CKeyDefinition *kdf = new CKeyDefinition(kset);
      ReadFrom (kdf, stream);
      Enter(kdf);
    }
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    globals->logWarning->Write ("CKeySet::Read : Unknown tag %s", TagToString(tag));
  }

  return rc;
}


//-------------------------------------------------------------------------
// Reset the default key
//  NOTE:  
//    The actual code of the key should be removed from the code table
//    The new code key should be entered into the code table
//-------------------------------------------------------------------------
void CKeySet::SetDefaultValues(CKeyFile *def)
{ std::map<Tag,CKeyDefinition*>::iterator kd = dkey.begin();
  for (kd = dkey.begin(); kd != dkey.end(); kd++)
  { CKeyDefinition* kdf = kd->second;
    int cde = (def)?(def->GetDefault(kset,kdf->GetTag())):(0);
    if (cde) globals->kbd->SwapCode(kdf,cde);
  }
  return;
}
//=====================================================================================
//  sort Keyset on name
//=====================================================================================
bool SortSet(CKeySet *s1,CKeySet *s2)
{ return  (strncmp(s1->GetName(),s2->GetName(),64) < 0);
}
//=============================================================================
//  KEYBOARD INTERFACE
//=============================================================================
CKeyMap::CKeyMap ()
: vers (0)
{ m_bKeyDetect    = false;
  m_fKeyCallback  = 0;
  m_winID         = 0;
  Init();
  BindKeys();
}
//--------------------------------------------------------------------------
//  Check for file name and open it
//--------------------------------------------------------------------------
void CKeyMap::OpenKey(char *fn)
{ SStream s;
  if (OpenRStream (fn,s)) {
    ReadFrom (this, &s);
    CloseStream (&s);
    return;
  }
  gtfo ("ERROR : Could not read key mappings from %s",fn);
  return;
}
//--------------------------------------------------------------------------
//  Destructor
//--------------------------------------------------------------------------
CKeyMap::~CKeyMap (void)
{ 
  // Delete key sets
  std::map<Tag,CKeySet*>::iterator i;
  for (i=kset.begin(); i!=kset.end(); i++) delete i->second;
  kset.clear();
  oset.clear();
}
//-------------------------------------------------------------------------
//  Check keyset name
//-------------------------------------------------------------------------
int CKeyMap::CheckSet(Tag idn)
{ if ('cmra' == idn)    return 1;
  if ('dbug' == idn)    return 2;
  if ('glob' == idn)    return 3;
  if ('menu' == idn)    return 4;
  if ('slew' == idn)    return 5;
  if ('misc' == idn)    return 6;
  if ('grnd' == idn)    return 7;
  if ('heli' == idn)    return 8;
  if ('plne' == idn)    return 9;
  //----Invalid set name ------------
  char tag1[8];
  TagToString(tag1,idn);
  gtfo("<%s> is not a valid key set tag", tag1);
  return 0;
}
//--------------------------------------------------------------------------
//  Read all keyboard definitions
//--------------------------------------------------------------------------
int CKeyMap::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  switch (tag) {
  case 'vers':
    // Version
    ReadInt (&vers, stream);
    rc = TAG_READ;
    break;

  case 'kset':
    // KeySet sub-object
    {
      Tag ksetTag;
      ReadTag (&ksetTag, stream);
      CheckSet(ksetTag);
      CKeySet *ks = new CKeySet (ksetTag);
      ReadFrom (ks, stream);
      kset[ks->GetTag()] = ks;
      oset.push_back(ks);
    }
    rc = TAG_READ;
    break;
  }

  if (rc == TAG_IGNORED) {
    globals->logWarning->Write ("CKeyMap::Read : Unknown tag %s", TagToString(tag));
  }

  return rc;
}
//--------------------------------------------------------------------------
//  All parameters are read.  order the Set
//--------------------------------------------------------------------------
void CKeyMap::ReadFinished()
{ std::sort(oset.begin(),oset.end(),SortSet);
}
//--------------------------------------------------------------------------
//  Save keyboard mapping to file
//--------------------------------------------------------------------------
void CKeyMap::SaveCurrentConfig()
{
  char stag[8];
  int i;
  SStream s;
  std::map<Tag,CKeySet*>::const_iterator it;
  std::map<Tag,CKeyDefinition*>::const_iterator kit;
  CKeySet        * pset;
  CKeyDefinition * pkey;

  strncpy (s.filename, "System/FlyLegacyKey.txt",(PATH_MAX-1));
  strncpy (s.mode, "w",3);
  if (OpenStream (&s))
  {
    //
    // file header and version
    //
    WriteString("//=================================================",&s);
    WriteString("// Please note that Keyset are ordered by name     ",&s);
    WriteString("// Menus should come first as they intercept keys  ",&s);
    WriteString("// that may be dispatched to lower entity such as  ",&s);
    WriteString("// Aircraft or ground vehicles                     ",&s);
    WriteString("//=================================================",&s);
    WriteTag('bgno', "========== BEGIN OBJECT ==========" , &s);
    WriteTag('vers', "---- configuration version ----", &s);
    WriteInt(&vers, &s);

    for(it = kset.begin(); it != kset.end(); it++)
    {
      pset = it->second;
      WriteTag('kset', "=== KeySet Definition File ===" , &s);
      TagToString(stag, it->first);
      WriteString(stag, &s);
      WriteTag('bgno', "========== BEGIN OBJECT ==========" , &s);
      WriteTag('name', "---- key set name ----", &s);
      WriteString(pset->GetName(), &s);
      WriteTag('user', "---- user can modify ----", &s);
      i = pset->GetUserModifiableState();
      WriteInt(&i, &s);
      WriteTag('enab', "---- enabled ----", &s);
      i = pset->GetEnabledState();
      WriteInt(&i, &s);
      for(kit = pset->dkey.begin(); kit != pset->dkey.end(); kit++)
      {
        pkey = kit->second;
        WriteTag('kkey', "---- key definition ----", &s);
        WriteTag('bgno', "========== BEGIN OBJECT ==========", &s);
        WriteTag('kyid', "---- key ID ----", &s);
        TagToString(stag, kit->first);
        WriteString(stag, &s);
        WriteTag('name', "---- key name ----", &s);
        WriteString(pkey->GetName(), &s);
        WriteTag('code', "---- key code & modifier ----", &s);
        i = pkey->GetCode();
        WriteInt(&i, &s);
        WriteTag('user', "---- user definable ----", &s);
        i = pkey->IsUserMod();
        WriteInt(&i, &s);
        WriteTag('enab', "---- enabled ----", &s);
        i = pkey->IsEnabled();
        WriteInt(&i, &s);
        WriteTag('endo', "========== END OBJECT ==========" , &s);
      }
      WriteTag('endo', "========== END OBJECT ==========" , &s);
    }
    //
    // File end
    //
    WriteTag('endo', "========== END OBJECT ==========" , &s);
    CloseStream(&s);
  }
}
//--------------------------------------------------------------------------
//  Print all keys
//--------------------------------------------------------------------------
void CKeyMap::Print (FILE* f)
{
  // Iterate over keysets
  std::map<Tag,CKeySet*>::iterator i;
  for (i=kset.begin(); i!=kset.end(); i++) {
    CKeySet *pKeySet = i->second;

    // Format key set unique ID
    char s[8];
    TagToString (s, pKeySet->GetTag());
    fprintf (f, "KeySet '%s' %s\n", s, pKeySet->GetName());

    // Iterate over key definitions within the keyset
    std::map<Tag,CKeyDefinition*>::iterator j;
    for (j=pKeySet->dkey.begin(); j!=pKeySet->dkey.end(); j++) {
      CKeyDefinition *pKeyDef = j->second;

      // Format key definition unique ID
      char sId[8];
      TagToString (sId, pKeyDef->GetTag());

      // Format key code
      char sCode[32];
      formatKeyCode (sCode, pKeyDef->GetCode());

      fprintf (f, "  '%s' %-40s  %s\n", sId, pKeyDef->GetName(), sCode);
    }
  }
}
//------------------------------------------------------------------------------
//  Locate a key definition by set and key identifiers
//  
//------------------------------------------------------------------------------
CKeyDefinition* CKeyMap::FindKeyDefinitionByIds(Tag setid, Tag keyid)
{ CKeyDefinition * pkey = 0;
  CKeySet *pset  = FindKeySetById(setid);
  if(0 == pset)  return 0;
  // Search for key definition in this keyset
  std::map<Tag,CKeyDefinition*>::const_iterator j = pset->dkey.find(keyid);
  if (j != pset->dkey.end()) return j->second;
  return 0;
}
//------------------------------------------------------------------------------
//  Locate a key definition by  key identifier only
//  NOTE:  Group are searched from upper group(menus,..) to lower group (aircraft)
//------------------------------------------------------------------------------
CKeyDefinition* CKeyMap::FindKeyDefinitionById (Tag id)
{ CKeyDefinition *rc = NULL;
  for (U_INT k=0; k< oset.size();k++)
  { CKeySet *ks         = oset[k];
    CKeyDefinition* kdf = ks->GetKeyByTag(id);
    if (0 == kdf) continue;
    return kdf;
  }
  return 0;
}
//------------------------------------------------------------------------------
//  Locate a key definition by  keyboard code only
//------------------------------------------------------------------------------
CKeyDefinition* CKeyMap::FindKeyDefinitionByCode (int iCode, Tag &setTagOut)
{ setTagOut = 0;
  std::map<Tag,CKeySet*>::iterator i;
  for (i=kset.begin(); i!=kset.end(); i++)
  { CKeySet *set = i->second;
    CKeyDefinition *kdf = set->GetKeyByCode(iCode);
    if (0 == kdf)     continue;
    setTagOut = set->GetTag();
    return kdf;
  }
 return 0;
}

//------------------------------------------------------------------------------
//  Locate a key set where key (id) resides
//------------------------------------------------------------------------------
CKeySet * CKeyMap::FindKeySetById (Tag id)
{ std::map<Tag,CKeySet*>::const_iterator i = kset.find(id);
  if(i != kset.end()) return i->second;
  return 0;
}
//------------------------------------------------------------------------------
//  Locate a key set by its number
//------------------------------------------------------------------------------
CKeySet* CKeyMap::GetKeySet (U_INT i)
{ CKeySet *rc = NULL;
  if (i < oset.size()) return oset[i];
  return 0;
}
//------------------------------------------------------------------------------
//  Clear key code defined in key definition 
//------------------------------------------------------------------------------
void CKeyMap::ClearCode(CKeyDefinition *kdf)
{ Tag ks = kdf->GetSet();
  int cd = kdf->GetCode();
  kdf->SetCode(0);
  std::map<Tag,CKeySet*>::iterator it = kset.find(ks);
  if (it == kset.end())   return;
  (*it).second->ClearCode(cd);
  return;
}
//------------------------------------------------------------------------------
//  Unassign code from any key definition existing in the set
//	NOTE: The key is unassigned from the current set only
//------------------------------------------------------------------------------
CKeyDefinition *CKeyMap::UnAssign(int cde,CKeyDefinition *kdf)
{ CKeySet					*set = FindKeySetById(kdf->GetSet());
  CKeyDefinition	*kpv = set->GetKeyByCode(cde);
  if (0   == kpv)         return 0;
  kdf->SetCode(0);			// remove code from actual
	kpv->SetCode(0);			// remove code from previous
	set->ClearCode(cde);
  return kpv;
}
//------------------------------------------------------------------------------
//  Assign code to key definition
//------------------------------------------------------------------------------
void  CKeyMap::Assign(int cde,CKeyDefinition *kdf)
{ Tag ks = kdf->GetSet();
  kdf->SetCode(cde);
  std::map<Tag,CKeySet*>::iterator it = kset.find(ks);
  if (it == kset.end())   return;
  CKeySet *set = (*it).second;
  set->StoreKeyDef(kdf);
  return;
}

//------------------------------------------------------------------------------
//  Swap key code
//------------------------------------------------------------------------------
void CKeyMap::SwapCode(CKeyDefinition *kdf,int nc)
{ Tag ks = kdf->GetSet();
  int cd = kdf->GetCode();
  kdf->SetCode(0);
  std::map<Tag,CKeySet*>::iterator it = kset.find(ks);
  if (it != kset.end())      return;   // No set
  CKeySet *set = (*it).second;
  set->ClearCode(cd);
  //---Set the new code ----------------------------
  kdf->SetCode(nc);
  set->StoreKeyDef(kdf);
  return;
}
//------------------------------------------------------------------------------
//  Reset all keys to default values
//------------------------------------------------------------------------------
void CKeyMap::SetDefaultValues(CKeyFile *def)
{ std::map<Tag,CKeySet*>::iterator i;
  for (i=kset.begin(); i!=kset.end(); i++) {
      CKeySet *ks = i->second;
      ks->SetDefaultValues(def);
    }
 return; 
}
//------------------------------------------------------------------------------
//  Bind Key (idk) to global function f
//  NOTE: As keys menu are first searched, when a Key is redefined 
//  by a lower group, then the menu key will redirect the call to the lower 
//  entity
//  Example: GWIN is the menu Key to display GPS
//           When aircraft bind the 'gwin' key, the call is redirected to aircraft
//------------------------------------------------------------------------------
void CKeyMap::Bind (Tag id, KeyCallbackPtr f,char tp)
{ CKeyDefinition *kdf = FindKeyDefinitionById (id);
  if (0 == kdf)       return; 
  kdf->SetSet(lgrp);  
  kdf->Bind(f);
  kdf->SetType(tp);
  return;
}
//------------------------------------------------------------------------------
//  Bind keyset with group key to global function f
//------------------------------------------------------------------------------
void CKeyMap::BindGroup(Tag gp,KeyGroupCB f)
{ CKeySet *set = FindKeySetById(gp);
  if (0 == set)   return;
  lgrp  = gp;                     // Remember the group
  set->BindTo(f);
  return;
}
//------------------------------------------------------------------------------
//  Unbind keyset with group key to global function f
//------------------------------------------------------------------------------
void CKeyMap::UnbindGroup(Tag gp)
{ CKeySet *set = FindKeySetById(gp);
  if (0 == set)   return;
  set->BindTo(GroupUnbind);
  return;
}
//------------------------------------------------------------------------------
//  Keyboard Key pressed
//------------------------------------------------------------------------------
void CKeyMap::KeyPress (EKeyboardKeys key, EKeyboardModifiers mod)
{ // Translate key and modifier into keycode
  int  code = (mod << 16) + key;
  bool handled = false;
  //--- special detection mode ------------------------------
  if(m_bKeyDetect && m_fKeyCallback)
  {
    /// \todo unbind if this key code is already asigned
    if (m_fKeyCallback(m_winID, code)) return;
  }
  //---Standard key -----------------------------------------
  for (U_INT k=0; (k<oset.size()); k++)
  {   CKeySet *ks = oset[k];
      CKeyDefinition *kdf = ks->GetKeyByCode(code);
      if (0 == kdf)       continue;
      //---Call the redirected group key callback -----------
      Tag        ngp = kdf->GetSet();
      ks             =   FindKeySetById (ngp);
      KeyGroupCB  cb = ks->GetCB();
      handled = (cb)(kdf,code);
			if (handled) return;
  }
  return;
}
//-------------------------------------------------------------------------
//  Change group callback if key is redirected
//-------------------------------------------------------------------------
//  Activate key in group (simulate a key stroke)
//  The group is taken from the Keydef in case of redirection
//-------------------------------------------------------------------------
bool CKeyMap::Stroke(Tag grp,Tag kid)
{ //--- find the Key set ---------------------------
  CKeySet *ks = FindKeySetById (grp);
  if (0 == ks)    return false;
  CKeyDefinition *kdf = ks->GetKeyByTag(kid);
  if (0 == kdf)   return false;
  //--- Group Redirection --------------------------
  Tag        ngp = kdf->GetSet();
  ks             =   FindKeySetById (ngp);
  KeyGroupCB  cb = ks->GetCB();
  return (cb)(kdf,kdf->GetCode()) ;
}
//==================================================================================

typedef struct {
  EKeyboardKeys kbkey;
  char      *name;
} SKeyCodeFormatEntry;
//==================================================================================
//  Table to edit keyboard entries
//==================================================================================

static SKeyCodeFormatEntry keyCodeFormatTable[] =
{
  { KB_KEY_ESC,                   "Esc" },
  { KB_KEY_1,                     "1" },
  { KB_KEY_2,                     "2" },
  { KB_KEY_3,                     "3" },
  { KB_KEY_4,                     "4" },
  { KB_KEY_5,                     "5" },
  { KB_KEY_6,                     "6" },
  { KB_KEY_7,                     "7" },
  { KB_KEY_8,                     "8" },
  { KB_KEY_9,                     "9" },
  { KB_KEY_0,                     "0" },
  { KB_KEY_MINUS,                 "-" },
  { KB_KEY_EQUALS,                "=" },
  { KB_KEY_BACK,                  "Backspace" },
  { KB_KEY_TAB,                   "Tab" },
  { KB_KEY_Q,                     "Q" },
  { KB_KEY_W,                     "W" },
  { KB_KEY_E,                     "E" },
  { KB_KEY_R,                     "R" },
  { KB_KEY_T,                     "T" },
  { KB_KEY_Y,                     "Y" },
  { KB_KEY_U,                     "U" },
  { KB_KEY_I,                     "I" },
  { KB_KEY_O,                     "O" },
  { KB_KEY_P,                     "P" },
  { KB_KEY_FORWARD_BRACKET,       "]" },
  { KB_KEY_REVERSE_BRACKET,       "[" },
  { KB_KEY_ENTER,                 "Enter" },
  { KB_KEY_LCTRL,                 "LeftCtrl" },
  { KB_KEY_A,                     "A" },
  { KB_KEY_S,                     "S" },
  { KB_KEY_D,                     "D" },
  { KB_KEY_F,                     "F" },
  { KB_KEY_G,                     "G" },
  { KB_KEY_H,                     "H" },
  { KB_KEY_J,                     "J" },
  { KB_KEY_K,                     "K" },
  { KB_KEY_L,                     "L" },
  { KB_KEY_SEMI_COLON,            ";" },
  { KB_KEY_SINGLE_QUOTE,          "'" },
  { KB_KEY_REVERSE_SINGLE_QUOTE,  "`" },
  { KB_KEY_LSHIFT,                "LeftShift" },
  { KB_KEY_BACKSLASH,             "\\" },
  { KB_KEY_Z,                     "Z" },
  { KB_KEY_X,                     "X" },
  { KB_KEY_C,                     "C" },
  { KB_KEY_V,                     "V" },
  { KB_KEY_B,                     "B" },
  { KB_KEY_N,                     "N" },
  { KB_KEY_M,                     "M" },
  { KB_KEY_COMMA,                 "," },
  { KB_KEY_PERIOD,                "." },
  { KB_KEY_SLASH,                 "/" },
  { KB_KEY_RSHIFT,                "RightShift" },
  { KB_KEY_STAR,                  "Star" },
  { KB_KEY_LALT,                  "LeftAlt" },
  { KB_KEY_SPACE,                 "Space" },
  { KB_KEY_CAPSLOCK,              "CapsLock" },
  { KB_KEY_F1,                    "F1" },
  { KB_KEY_F2,                    "F2" },
  { KB_KEY_F3,                    "F3" },
  { KB_KEY_F4,                    "F4" },
  { KB_KEY_F5,                    "F5" },
  { KB_KEY_F6,                    "F6" },
  { KB_KEY_F7,                    "F7" },
  { KB_KEY_F8,                    "F8" },
  { KB_KEY_F9,                    "F9" },
  { KB_KEY_F10,                   "F10" },
  { KB_KEY_NUMLOCK,               "NumLock" },
  { KB_KEY_SCROLLLOCK,            "ScrollLock" },
//  { KB_KEY_HOME,                  "KeypadHome" },
  { KB_KEY_HOME,                  "padHome" },
//  { KB_KEY_UP,                    "KeypadUpArrow" },
  { KB_KEY_UP,                    "padUpArrow" },
//  { KB_KEY_PGUP,                  "KeypadPageUp" },
  { KB_KEY_PGUP,                  "padPageUp" },
//  { KB_KEY_KEYPAD_MINUS,          "KeypadMinus" },
  { KB_KEY_KEYPAD_MINUS,          "pad -" },
//  { KB_KEY_LEFT,                  "KeypadLeftArrow" },
  { KB_KEY_LEFT,                  "padLeftArrow" },
//  { KB_KEY_CENTER,                "KeypadCenter" },
  { KB_KEY_CENTER,                "padCenter" },
//  { KB_KEY_RIGHT,                 "KeypadRightArrow" },
  { KB_KEY_RIGHT,                 "padRightArrow" },
//  { KB_KEY_KEYPAD_PLUS,           "KeypadPlus" },
  { KB_KEY_KEYPAD_PLUS,           "pad +" },
//  { KB_KEY_END,                   "KeypadEnd" },
  { KB_KEY_END,                   "padEnd" },
//  { KB_KEY_DOWN,                  "KeypadDownArrow" },
  { KB_KEY_DOWN,                  "padDownArrow" },
//  { KB_KEY_PGDN,                  "KeypadPageDown" },
  { KB_KEY_PGDN,                  "padPageDown" },
//  { KB_KEY_INSERT,                "KeypadIns" },
  { KB_KEY_INSERT,                "padIns" },
//  { KB_KEY_DEL,                   "KeypadDel" },
  { KB_KEY_DEL,                   "padDel" },
  { KB_KEY_F11,                   "F11" },
  { KB_KEY_F12,                   "F12" },
//  { KB_KEY_KEYPAD_ENTER,          "KeypadEnter" },
  { KB_KEY_KEYPAD_ENTER,          "padEnter" },
  { KB_KEY_RCTRL,                 "RightCtrl" },
//  { KB_KEY_KEYPAD_SLASH,          "KeypadSlash" },
  { KB_KEY_KEYPAD_SLASH,          "Num /" },
  { KB_KEY_RALT,                  "RightAlt" },
  { KB_KEY_EXT_NUMLOCK,           "ExtNumLock" },
  { KB_KEY_GRAY_HOME,             "GrayHome" },
  { KB_KEY_GRAY_UP,               "GrayUpArrow" },
  { KB_KEY_GRAY_PGUP,             "GrayPageUp" },
  { KB_KEY_GRAY_LEFT,             "GrayLeftArrow" },
  { KB_KEY_GRAY_RIGHT,            "GrayRightArrow" },
  { KB_KEY_GRAY_END,              "GrayEnd" },
  { KB_KEY_GRAY_DOWN,             "GrayDownArrow" },
  { KB_KEY_GRAY_PGDN,             "GrayPageDown" },
  { KB_KEY_GRAY_INS,              "GrayIns" },
  { KB_KEY_GRAY_DEL,              "GrayDel" },
  { KB_KEY_META,                  "Meta" }
};

//==================================================================================
//  Edit a given key code
//==================================================================================
void formatKeyCode (char *s, int code)
{*s = 0;
  // Format modifiers
  int iMod = (code & 0xFFFF0000) >> 16;
  if (iMod & KB_MODIFIER_CTRL)  strcat (s, "Ctrl ");
  if (iMod & KB_MODIFIER_ALT)   strcat (s, "Alt  ");
  if (iMod & KB_MODIFIER_SHIFT) strcat (s, "Shift");
  if (iMod & KB_MODIFIER_META)  strcat (s, "Meta ");

  int nKeys = sizeof(keyCodeFormatTable) / sizeof(SKeyCodeFormatEntry);
  int kbkey = (code & 0x0000FFFF);
  for (int i=0; i<nKeys; i++) {
    SKeyCodeFormatEntry *p = &keyCodeFormatTable[i];
    if (p->kbkey == kbkey) {
      strcat (s, p->name);
      break;
    }
  }
  return;
}


