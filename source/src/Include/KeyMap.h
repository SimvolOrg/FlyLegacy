/*
 * KeyMap.h
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

/*! \file KeyMap.h
 *  \brief Defines CKeyMap and related classes for mapping keys to events
 */

#ifndef KEYMAP_H
#define KEYMAP_H


#include "FlyLegacy.h"
#include <map>
//========================================================================================
class CKeyFile;
class CSimButton;
class CVehicleObject;
//========================================================================================
#define KEY_REPEAT  (0x00)
#define KEY_TOGGLE  (0x01)
#define KEY_SET_ON  (0x02)
//=============================================================================
// Function type declaration for key map callbacks
//=============================================================================
typedef bool(*CVehicleObject::*KeyCallbackVeh) (int keyid, int code, int modifiers);
//========================================================================================
// This table is a simple mapping of GLUT key codes to FlyLegacy key codes.
//=========================================================================================
struct SGlutToFlyLegacyKey {
  U_INT glut;
  U_INT flylegacy;
} ;

//===============================================================================
// CKeyDefinition represents a single simulator function that can be mapped to
//   a keyboard key or a joystick button.
//   For a joystick button are of 2 types:
//   Type 0: Function is called each time the button is pressed
//   Type 1: Function is called only when state is changed
//===============================================================================
class CKeyDefinition : public CStreamObject {
protected:
  KeyCallbackPtr  cb;     // Callback function
  //----------------------------------------------------
  Tag           kset;     // Key set
  Tag           kyid;     // Unique key definition tag
	char          stag[6];	// Set tag
	char					skey[6];	// Key tag
  char          name[64]; // UI name
	char          knam[32];	// Key name
  int           code;     // Key code and modifier
  bool          user;     // User definable
  bool          enab;     // Enabled
  char          type;     // Type of button
  CSimButton   *jbtn;     // Joystick button

public:
  CKeyDefinition (Tag ks);
 
  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);
  CSimButton   *LinkTo(CSimButton *b);
  // CKeyDefinition methods
  KeyCallbackPtr GetCallback() {return cb; }
  bool  IsUserMod( ) {return user;};
  bool  IsEnabled( ) {return enab;};
	U_INT	Warn(char *msg);
	//-------------------------------------------------
	U_INT	Decode(SStream *str);
  //-------------------------------------------------
  inline void   SetCode(int iCode)      {code = iCode; };
  inline bool   NoPCB()                 {return (cb == 0);}
  inline bool   HasCB()                 {return (cb != 0);}
  inline void   Bind(KeyCallbackPtr f)  {cb = f;}
  inline void   SetType(char t)         {type = t;}
  inline void   SetSet (Tag s)          {kset = s;}
  inline char*  GetName (void)          {return name;}
  inline int    GetCode (void)          {return code;}
  inline Tag    GetTag()                {return kyid;}
  inline Tag    GetSet()                {return kset;}
  inline char   GetType()               {return type;}
  inline CSimButton *GetButton()        {return jbtn;}

public:

};


//==============================================================================
// CKeySet represents a group of key mappings that relate to a similar group
//   of simulator functions.  The default key mapping contains key sets for
//   "Menu Keys", "Global Keys", 
//==============================================================================
class CKeySet : public CStreamObject {
public:
  CKeySet (Tag tag);
  ~CKeySet (void);

  // CStreamObject methods
  virtual int   Read (SStream *stream, Tag tag);

  // CKeySet methods
  char*     GetName (void)                      { return name; }
  bool      GetUserModifiableState (void) const { return user; }
  bool      GetEnabledState (void) const        { return enab; }
  int       GetNumKeyDefinitions (void) const   { return dkey.size();}
  Tag       GetTag( ) const {return kset; };
  void      GetMap(std::map<Tag,CKeyDefinition*> &mp) {mp = dkey;}
  void      SetDefaultValues(CKeyFile *def);
  void      Enter(CKeyDefinition *kdf);
  void      ClearCode(int c);
  bool      CallGroup(CKeyDefinition *kdf,int modifier);
  //-------------------------------------------------------------
  void        BindTo(KeyGroupCB f) {gcb = f;}
  //-------------------------------------------------------------
  inline    KeyGroupCB GetCB()  {return gcb;}
  //-------------------------------------------------------------
  CKeyDefinition *GetKeyByCode(int c);
  CKeyDefinition *GetKeyByTag(Tag t);
  //-------------------------------------------------------------
  void      StoreKeyDef(CKeyDefinition *kdf);
public:
  KeyGroupCB  gcb;                      // Group call back
  std::map<Tag,CKeyDefinition*>   dkey; // Map of key by tag
  std::map<int,CKeyDefinition*>   ckey; // Map of key by code;
protected:
  Tag     kset;                         // Key set unique tag
  char    name[64];                     // Key set name
  bool    user;                         // User modifiable
  bool    enab;                         // Enabled
};
//==============================================================================
// CKeyMap is the application-visible interface class to the keyboard
//   mapping system.  Typical example usage:
//
// Application init:
//   CKeyMappings *keymap = new CKeyMappings ("System/default.key");
//   keymap->Bind ('blah', func_blah);
//   ...
//
// Application detects key press:
//   keymap->KeyPress (key, modifiers);
//==============================================================================
typedef int(KeyDetectCallback(Tag winID, int iCode));


class CKeyMap : public CStreamObject {

public:
  
   CKeyMap();
  ~CKeyMap (void);
  void    OpenKey(char *fn);
  void    Init();
  void    BindKeys();
  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished();
  // CKeyMap methods
  void            Print (FILE *f);
  void            KeyPress (U_INT key, EKeyboardModifiers mod);
  bool            Stroke(Tag grp,Tag kid);
  void            Bind (Tag id, KeyCallbackPtr f,char type);
  void            BindGroup(Tag gp, KeyGroupCB f);
  void            UnbindGroup(Tag gp);
  CKeySet*        GetKeySet (U_INT i);
  CKeyDefinition* FindKeyDefinitionById (Tag id);
  CKeyDefinition* FindKeyDefinitionByIds (Tag setid, Tag keyid);
  CKeySet*        FindKeySetById (Tag id);
  CKeyDefinition* FindKeyDefinitionByCode (int iCode, Tag & setTagOut);
  void                  StartKeyDetect(KeyDetectCallback f, Tag winID){m_bKeyDetect=true;m_fKeyCallback=f;m_winID = winID;};
  void                  StopKeyDetect(){m_bKeyDetect=false;};
  void            SaveCurrentConfig();
  void            SetDefaultValues(CKeyFile *kdf);
  void            ClearCode(CKeyDefinition *kdf);
  void            SwapCode(CKeyDefinition *kdf,int nc);
  //------------------------------------------------------------------
  CKeyDefinition* UnAssign(int cde,CKeyDefinition *kdf);
  void            Assign(int cde,CKeyDefinition *kdf);
  //------------------------------------------------------------------
  int             CheckSet(Tag idn);
  //------------------------------------------------------------------
  inline   int             GetNumKeySets (void) {return oset.size();}
  //------------------------------------------------------------------
protected:
  int                      vers;            // Version, currently ignored
  std::map<Tag,CKeySet*>   kset;            // Map of keyset ids
  std::vector<CKeySet*>    oset;            // Ordered set
  Tag                      m_winID;         // winID for key detection
  KeyDetectCallback   *    m_fKeyCallback;  // callback fonction for key detection
  bool                     m_bKeyDetect;    // 
  Tag                      lgrp;            // Last group binded
};


//=============================================================================
// Utility function which formats the keycode into a human-readable string
//=============================================================================
void formatKeyCode (char *s, int code,char opt = 0);

#endif // KEYMAP_H

