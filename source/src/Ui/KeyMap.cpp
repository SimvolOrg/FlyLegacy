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
#include "../Include/joysticks.h"
using namespace std;
//===========================================================================
extern bool GroupUnbind(CKeyDefinition *kdf,int code);

//==================================================================================
//  Table to edit keyboard entries
//==================================================================================
std::map<U_INT,std::string> keyString;
void BuildKeyString()
{
//static SKeyCodeFormatEntry keyCodeFormatTable[] =
//{
	keyString[0]	=														"none";
  keyString[KB_KEY_ESC]=										"Esc";
  keyString['1'] =                     "1" ;
  keyString['2'] =                     "2" ;
  keyString['3'] =                     "3" ;
  keyString['4'] =                     "4" ;
  keyString['5'] =                     "5" ;
  keyString['6'] =                     "6" ;
  keyString['7'] =                     "7" ;
  keyString['8'] =                     "8" ;
  keyString['9'] =                     "9" ;
  keyString['0'] =                     "0" ;
  keyString[KB_KEY_MINUS] =                 "-" ;
  keyString[KB_KEY_EQUALS] =                "=" ;
  keyString[KB_KEY_BACK] =                  "Backspace" ;
  keyString[KB_KEY_TAB] =                   "Tab" ;
	//----------------------------------------------------
  keyString['q'] =                     "q" ;
  keyString['w'] =                     "w" ;
  keyString['e'] =                     "e" ;
  keyString['r'] =                     "r" ;
  keyString['t'] =                     "t" ;
  keyString['y'] =                     "y" ;
  keyString['u'] =                     "u" ;
  keyString['i'] =                     "i" ;
  keyString['o'] =                     "o" ;
  keyString['p'] =                     "p" ;
	//----------------------------------------------------
  keyString['Q'] =                     "Q" ;
  keyString['W'] =                     "W" ;
  keyString['E'] =                     "E" ;
  keyString['R'] =                     "R" ;
  keyString['T'] =                     "T" ;
  keyString['Y'] =                     "Y" ;
  keyString['U'] =                     "U" ;
  keyString['I'] =                     "I" ;
  keyString['O'] =                     "O" ;
  keyString['P'] =                     "P" ;
	//--------------------------------------------------
  keyString[KB_KEY_FORWARD_BRACKET] =       "]" ;
  keyString[KB_KEY_REVERSE_BRACKET] =       "[" ;
  keyString[KB_KEY_ENTER] =                 "Enter" ;
  keyString[KB_KEY_LCTRL] =                 "LeftCtrl" ;
  keyString['a'] =                     "a" ;
  keyString['s'] =                     "s" ;
  keyString['d'] =                     "d" ;
  keyString['f'] =                     "f" ;
  keyString['g'] =                     "g" ;
  keyString['h'] =                     "h" ;
  keyString['j'] =                     "j" ;
  keyString['k'] =                     "k" ;
  keyString['l'] =                     "l" ;
	//-------------------------------------------------
  keyString['A'] =                     "A" ;
  keyString['S'] =                     "S" ;
  keyString['D'] =                     "D" ;
  keyString['F'] =                     "F" ;
  keyString['G'] =                     "G" ;
  keyString['H'] =                     "H" ;
  keyString['J'] =                     "J" ;
  keyString['K'] =                     "K" ;
  keyString['L'] =                     "L" ;
	//-------------------------------------------------
  keyString[KB_KEY_SEMI_COLON] =            ";" ;
  keyString[KB_KEY_SINGLE_QUOTE] =          "'" ;
	keyString[KB_KEY_DBLEP] =									":" ;
  keyString[KB_KEY_REVERSE_SINGLE_QUOTE] =  "`" ;
  keyString[KB_KEY_LSHIFT] =                "LeftShift" ;
  keyString[KB_KEY_BACKSLASH] =             "\\" ;
  keyString['z'] =                     "z" ;
  keyString['x'] =                     "x" ;
  keyString['c'] =                     "c" ;
  keyString['v'] =                     "v" ;
  keyString['b'] =                     "b" ;
  keyString['n'] =                     "n" ;
  keyString['m'] =                     "m" ;
	//-----------------------------------------------------
  keyString['Z'] =                     "Z" ;
  keyString['X'] =                     "X" ;
  keyString['C'] =                     "C" ;
  keyString['V'] =                     "V" ;
  keyString['B'] =                     "B" ;
  keyString['N'] =                     "N" ;
  keyString['M'] =                     "M" ;
	//-----------------------------------------------------
	keyString['<'] =									"<" ;
	keyString['>'] =									">" ;
	//-----------------------------------------------------
	keyString['!'] =									"!" ;
	keyString['$'] =									"$" ;
	keyString['%'] =									"%" ;
	keyString['?']  =									"?" ;
  keyString[','] =									"," ;
  keyString['.'] =									"." ;
  keyString['/'] =									"/" ;
  keyString[KB_KEY_RSHIFT] =                "RightShift" ;
  keyString[KB_KEY_STAR] =                  "Star" ;
  keyString[KB_KEY_LALT] =                  "LeftAlt" ;
  keyString[KB_KEY_SPACE] =                 "Space" ;
  keyString[KB_KEY_CAPSLOCK] =              "CapsLock" ;
	//------------------------------------------------------
  keyString[KB_KEY_F1] =                    "F1" ;
  keyString[KB_KEY_F2] =                    "F2" ;
  keyString[KB_KEY_F3] =                    "F3" ;
  keyString[KB_KEY_F4] =                    "F4" ;
  keyString[KB_KEY_F5] =                    "F5" ;
  keyString[KB_KEY_F6] =                    "F6" ;
  keyString[KB_KEY_F7] =                    "F7" ;
  keyString[KB_KEY_F8] =                    "F8" ;
  keyString[KB_KEY_F9] =                    "F9" ;
  keyString[KB_KEY_F10] =                   "F10" ;
  keyString[KB_KEY_F11] =                   "F11" ;
  keyString[KB_KEY_F12] =                   "F12" ;
	//-----------------------------------------------------
  keyString[KB_KEY_NUMLOCK] =               "NumLock" ;
  keyString[KB_KEY_SCROLLLOCK] =            "ScrollLock" ;
  keyString[KB_KEY_HOME] =                  "padHome" ;
  keyString[KB_KEY_UP] =                    "padUpArrow" ;
  keyString[KB_KEY_PGUP] =                  "padPageUp" ;
  keyString[KB_KEY_KEYPAD_MINUS] =          "pad -" ;
  keyString[KB_KEY_LEFT] =                  "padLeftArrow" ;
  keyString[KB_KEY_CENTER] =                "padCenter" ;
  keyString[KB_KEY_RIGHT] =                 "padRightArrow" ;
  keyString[KB_KEY_KEYPAD_PLUS] =           "pad +" ;
  keyString[KB_KEY_END] =                   "padEnd" ;
  keyString[KB_KEY_DOWN] =                  "padDownArrow" ;
  keyString[KB_KEY_PGDN] =                  "padPageDown" ;
	//-----------------------------------------------------
  keyString[KB_KEY_INSERT] =                "padIns" ;
  keyString[KB_KEY_DEL] =                   "padDel" ;
  keyString[KB_KEY_KEYPAD_ENTER] =          "padEnter" ;
  keyString[KB_KEY_RCTRL] =                 "RightCtrl" ;
  keyString[KB_KEY_KEYPAD_SLASH] =          "Num /" ;
  keyString[KB_KEY_RALT] =                  "RightAlt" ;
  keyString[KB_KEY_EXT_NUMLOCK] =           "ExtNumLock" ;
  keyString[KB_KEY_GRAY_HOME] =             "GrayHome" ;
  keyString[KB_KEY_GRAY_UP] =               "GrayUpArrow" ;
  keyString[KB_KEY_GRAY_PGUP] =             "GrayPageUp" ;
  keyString[KB_KEY_GRAY_LEFT] =             "GrayLeftArrow" ;
  keyString[KB_KEY_GRAY_RIGHT] =            "GrayRightArrow" ;
  keyString[KB_KEY_GRAY_END] =              "GrayEnd" ;
  keyString[KB_KEY_GRAY_DOWN] =             "GrayDownArrow" ;
  keyString[KB_KEY_GRAY_PGDN] =             "GrayPageDown" ;
  keyString[KB_KEY_GRAY_INS] =              "GrayIns" ;
  keyString[KB_KEY_GRAY_DEL] =              "GrayDel" ;
  keyString[KB_KEY_META] =                  "Meta";
}
//===========================================================================
//	String to Key Map
//===========================================================================
std::map<std::string,U_INT> namedKeys;
void MapNamedKeys()
{	namedKeys["none"]							= 0;
	namedKeys["Ctrl"]							= (KB_MODIFIER_CTRL << 16);
	namedKeys["Alt"]							= (KB_MODIFIER_ALT  << 16);
	namedKeys["Shift"]						= (KB_MODIFIER_SHIFT << 16);
	namedKeys["Meta"]							= (KB_MODIFIER_META << 16);
	//-----------------------------------------------------------
	namedKeys["Esc"]							= KB_KEY_ESC;
	namedKeys["1"]								= '1';
	namedKeys["2"]								= '2';
	namedKeys["3"]								= '3';
	namedKeys["4"]								= '4';
	namedKeys["5"]								= '5';
	namedKeys["6"]								= '6';
	namedKeys["7"]								= '7';
	namedKeys["8"]								= '8';
	namedKeys["9"]								= '9';
	namedKeys["0"]								= '0';
	//------------------------------------------------------
	namedKeys["-"]								= KB_KEY_MINUS;
	namedKeys["="]								= KB_KEY_EQUALS;
	namedKeys["Backspace"]				= KB_KEY_BACK;
	namedKeys["Tab"]							= KB_KEY_TAB;
	//------------------------------------------------------
	namedKeys["Q"]								= 'Q';
	namedKeys["W"]								= 'W';
	namedKeys["E"]								= 'E';
	namedKeys["R"]								= 'R';
	namedKeys["T"]								= 'T';
	namedKeys["Y"]								= 'Y';
	namedKeys["U"]								= 'U';
	namedKeys["I"]								= 'I';
	namedKeys["O"]								= 'O';
	namedKeys["P"]								= 'P';
	//------------------------------------------------------
	namedKeys["q"]								= 'q';
	namedKeys["w"]								= 'w';
	namedKeys["e"]								= 'e';
	namedKeys["r"]								= 'r';
	namedKeys["t"]								= 't';
	namedKeys["y"]								= 'y';
	namedKeys["u"]								= 'u';
	namedKeys["i"]								= 'i';
	namedKeys["o"]								= 'o';
	namedKeys["p"]								= 'p';
	//------------------------------------------------------
	namedKeys["["]								= KB_KEY_FORWARD_BRACKET;
	namedKeys["]"]								= KB_KEY_REVERSE_BRACKET;
	namedKeys["Enter"]						= KB_KEY_ENTER;
	namedKeys["LeftCtrl"]					= KB_KEY_LCTRL;
	//---------------------------------------------------
	namedKeys["A"]								= 'A';
	namedKeys["S"]								= 'S';
	namedKeys["D"]								= 'D';
	namedKeys["F"]								= 'F';
	namedKeys["G"]								= 'G';
	namedKeys["H"]								= 'H';
	namedKeys["J"]								= 'J';
	namedKeys["K"]								= 'K';
	namedKeys["L"]								= 'L';
	//----------------------------------------------------
	namedKeys["a"]								= 'a';
	namedKeys["s"]								= 's';
	namedKeys["d"]								= 'd';
	namedKeys["f"]								= 'f';
	namedKeys["g"]								= 'g';
	namedKeys["h"]								= 'h';
	namedKeys["j"]								= 'j';
	namedKeys["k"]								= 'k';
	namedKeys["l"]								= 'l';
	//----------------------------------------------------

	namedKeys[";"]								= KB_KEY_SEMI_COLON;
	namedKeys["'"]								= KB_KEY_SINGLE_QUOTE;
	namedKeys["`"]								= KB_KEY_REVERSE_SINGLE_QUOTE;
	namedKeys[":"]								= KB_KEY_DBLEP;
	namedKeys["LeftShift"]				= KB_KEY_LSHIFT;
	namedKeys["\\"]								= KB_KEY_LSHIFT;
	//--------------------------------------------------------
	namedKeys["Z"]								= 'Z';
	namedKeys["X"]								= 'X';
	namedKeys["C"]								= 'C';
	namedKeys["V"]								= 'V';
	namedKeys["B"]								= 'B';
	namedKeys["N"]								= 'N';
	namedKeys["M"]								= 'M';
	//-------------------------------------------------------
	namedKeys["z"]								= 'z';
	namedKeys["x"]								= 'x';
	namedKeys["c"]								= 'c';
	namedKeys["v"]								= 'v';
	namedKeys["b"]								= 'b';
	namedKeys["n"]								= 'n';
	namedKeys["m"]								= 'm';
	//-------------------------------------------------------
	namedKeys[","]								= KB_KEY_COMMA;
	namedKeys["."]								= KB_KEY_PERIOD;
	namedKeys["/"]								= KB_KEY_SLASH;
	namedKeys["RightShift"]				= KB_KEY_RSHIFT;
	namedKeys["Star"]							= KB_KEY_STAR;
	namedKeys["LeftAlt"]					= KB_KEY_LALT;
	namedKeys["Space"]						= KB_KEY_SPACE;
	namedKeys["CapsLock"]					= KB_KEY_CAPSLOCK;
	//--------------------------------------------------------
	namedKeys["<"]								= '<';
	namedKeys[">"]								= '>';
	namedKeys["!"]								= '!';
	namedKeys["$"]								= '$';
	namedKeys["%"]								= '%';
	namedKeys["?"]								= '?';
	//--------------------------------------------------------
	namedKeys["F1"]								= KB_KEY_F1;
	namedKeys["F2"]								= KB_KEY_F2;
	namedKeys["F3"]								= KB_KEY_F3;
	namedKeys["F4"]								= KB_KEY_F4;
	namedKeys["F5"]								= KB_KEY_F5;
	namedKeys["F6"]								= KB_KEY_F6;
	namedKeys["F7"]								= KB_KEY_F7;
	namedKeys["F8"]								= KB_KEY_F8;
	namedKeys["F9"]								= KB_KEY_F9;
	namedKeys["F10"]							= KB_KEY_F10;
	namedKeys["F11"]							= KB_KEY_F11;
	namedKeys["F12"]							= KB_KEY_F12;
	//---------------------------------------------------------
	namedKeys["NumLock"]					= KB_KEY_NUMLOCK;
	namedKeys["ScrollLock"]				= KB_KEY_SCROLLLOCK;
	namedKeys["padHome"]					= KB_KEY_HOME;
	namedKeys["padUpArrow"]				= KB_KEY_UP;
	namedKeys["padPageUp"]				= KB_KEY_PGUP;
	namedKeys["pad -"]						= KB_KEY_KEYPAD_MINUS;
	namedKeys["padLeftArrow"]			= KB_KEY_LEFT;
	namedKeys["padCenter"]				= KB_KEY_CENTER;
	namedKeys["padRightArrow"]		= KB_KEY_RIGHT;
	namedKeys["pad +"]						= KB_KEY_KEYPAD_PLUS;

	namedKeys["padEnd"]						= KB_KEY_END;
	namedKeys["padDownArrow"]			= KB_KEY_DOWN;
	namedKeys["padPageDown"]			= KB_KEY_PGDN;
	namedKeys["padIns"]						= KB_KEY_INSERT;
	namedKeys["padDel"]						= KB_KEY_DEL;
	namedKeys["padEnter"]					= KB_KEY_KEYPAD_ENTER;

	namedKeys["RightCtrl"]				= KB_KEY_RCTRL;
	namedKeys["Num /"]						= KB_KEY_KEYPAD_SLASH;
	namedKeys["RightAlt"]					= KB_KEY_RALT;
	namedKeys["ExtNumLock"]				= KB_KEY_EXT_NUMLOCK;
	namedKeys["GrayHome"]					= KB_KEY_GRAY_HOME;

	namedKeys["GrayUpArrow"]			= KB_KEY_GRAY_UP;
	namedKeys["GrayPageUp"]				= KB_KEY_GRAY_PGUP;
	namedKeys["GrayLeftArrow"]		= KB_KEY_GRAY_LEFT;
	namedKeys["GrayRightArrow"]		= KB_KEY_GRAY_RIGHT;
	namedKeys["GrayEnd"]					= KB_KEY_GRAY_END;
	namedKeys["GrayDownArrow"]		= KB_KEY_GRAY_DOWN;
	namedKeys["GrayPageDown"]			= KB_KEY_GRAY_PGDN;
	namedKeys["GrayIns"]					= KB_KEY_GRAY_INS;
	namedKeys["GrayDel"]					= KB_KEY_GRAY_DEL;
}
//===========================================================================
//	ABort key definition
//---------------------------------------------------------------------------
U_INT CKeyDefinition::Warn(char * msg)
{	WARNINGLOG("Not a valid Key: %s", msg);
	return 0;
}
//===========================================================================
// Decode the key code
//===========================================================================
U_INT CKeyDefinition::Decode(SStream *str)
{	char pm1[24];
	char pm2[24];
	char pm3[24];
	U_INT	key = 0;
	std::map<std::string,U_INT>::iterator rk;
	ReadString(knam,32,str);
	int nf = sscanf_s(knam,"%24s %24s %24s",pm1,24,pm2,24,pm3,24);
	if (-1 == nf)								return key;
	//--- Decode first parameter ------------------------------
	if (0 == nf)								return Warn(knam);
	rk = namedKeys.find(pm1);
	if (rk == namedKeys.end())	return Warn(knam);
	key |= (*rk).second;
	if (1 == nf)								return key;
	//--- Decode second parameter -----------------------------
	rk = namedKeys.find(pm2);
	if (rk == namedKeys.end())	return Warn(knam);
	key |= (*rk).second;
	if (2 == nf)								return key;
	//--- Decode third parameter -----------------------------
	rk = namedKeys.find(pm3);
	if (rk == namedKeys.end())	return Warn(knam);
	key |= (*rk).second;
	return key;
}
//===========================================================================
// CKeyDefinition
//===========================================================================
CKeyDefinition::CKeyDefinition (Tag ks)
: kyid (0), code (0), user (true), enab (true), cb (NULL)
{ char tex[8];
	kset = ks;
 *name  = 0;
  jbtn  = 0;
  type  = 0;
	TagToString(tex,ks);
	strncpy(stag,tex,6);
}

//--------------------------------------------------------------
//  Read Parameters
//--------------------------------------------------------------
int CKeyDefinition::Read (SStream *stream, Tag tag)
{ char tex[8];
	int pm;
  switch (tag) {
  case 'kyid':
    // Key unique ID
    ReadTag (&kyid, stream);
		TagToString(tex,kyid);
		strncpy(skey,tex,6);
if (strcmp(skey,"anlt") == 0)
int a = 0;
    return TAG_READ;

  case 'name':
    // Key name
    ReadString (name, 64, stream);
    return TAG_READ;

  case 'code':
    // Key code and modifier
    code	= Decode(stream);
    return TAG_READ;

  case 'user':
    // User-mappable?
    ReadInt (&pm, stream);
    user = (pm != 0);
    return TAG_READ;

  case 'enab':
    // Enabled?
    ReadInt (&pm, stream);
    enab = (pm != 0);
    return TAG_READ;
  }

  globals->logWarning->Write ("CKeyDefinition::Read : Unknown tag %s", TagToString (tag));
  return TAG_IGNORED;
}
//---------------------------------------------------------------------------
//  Link to new button, return previous
//---------------------------------------------------------------------------
CSimButton *CKeyDefinition::LinkTo(CSimButton *btn)
{ CSimButton *old = jbtn; 
  jbtn			= btn;
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
	BuildKeyString();
	MapNamedKeys();
  Init();
  BindKeys();
}
//--------------------------------------------------------------------------
//  Check for file name and open it
//--------------------------------------------------------------------------
void CKeyMap::OpenKey(char *fn)
{ SStream s(this,fn);
	if (!s.ok) gtfo ("key file %s not found",fn);
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
{ char codk[128];
  char stag[8];
  int i;
  CStreamFile sf;
  std::map<Tag,CKeySet*>::const_iterator it;
  std::map<Tag,CKeyDefinition*>::const_iterator kit;
  CKeySet        * pset;
  CKeyDefinition * pkey;
  sf.OpenWrite("System/Keymap.txt");
  //
  // file header and version
  //
  sf.WriteString("//=================================================");
  sf.WriteString("// Please note that Keyset are ordered by name     ");
  sf.WriteString("// Menus should come first as they intercept keys  ");
  sf.WriteString("// that may be dispatched to lower entity such as  ");
  sf.WriteString("// Aircraft or ground vehicles                     ");
  sf.WriteString("//=================================================");
  sf.DebObject();
  sf.WriteTag('vers', "---- configuration version ----");
  sf.WriteInt(&vers);
  for(it = kset.begin(); it != kset.end(); it++)
  {	pset = it->second;
    sf.WriteTag('kset', "=== KeySet Definition File ===");
    TagToString(stag, it->first);
    sf.WriteString(stag);
    sf.WriteTag('bgno', "========== BEGIN OBJECT ==========");
    sf.WriteTag('name', "---- key set name ----");
    sf.WriteString(pset->GetName());
    sf.WriteTag('user', "---- user can modify ----");
    i = pset->GetUserModifiableState();
    sf.WriteInt(&i);
    sf.WriteTag('enab', "---- enabled ----");
    i = pset->GetEnabledState();
    sf.WriteInt(&i);
    for(kit = pset->dkey.begin(); kit != pset->dkey.end(); kit++)
      { pkey = kit->second;
        sf.WriteTag('kkey', "---- key definition ----");
        sf.DebObject();
        sf.WriteTag('kyid', "---- key ID ----");
        TagToString(stag, kit->first);
        sf.WriteString(stag);
        sf.WriteTag('name', "---- key name ----");
        sf.WriteString(pkey->GetName());
        sf.WriteTag('code', "---- key code & modifier ----");
				i = pkey->GetCode();
				formatKeyCode(codk,i,0);
				sf.WriteString(codk);
        sf.WriteTag('user', "---- user definable ----");
        i = pkey->IsUserMod();
        sf.WriteInt(&i);
        sf.WriteTag('enab', "---- enabled ----");
        i = pkey->IsEnabled();
        sf.WriteInt(&i);
        sf.EndObject();
      }
      sf.EndObject();
    }
  // File end
  //
  sf.EndObject();
  sf.Close();
	return;
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
void CKeyMap::KeyPress (U_INT key, EKeyboardModifiers mod)
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
//  Edit a given key code
//==================================================================================
void formatKeyCode (char *s, int code, char opt)
{*s = 0;
  // Format modifiers
  int iMod = (code & 0xFFFF0000) >> 16;
  if (iMod & KB_MODIFIER_CTRL)  strcat (s, "Ctrl ");
  if (iMod & KB_MODIFIER_ALT)   strcat (s, "Alt  ");
  if (iMod & KB_MODIFIER_SHIFT) strcat (s, "Shift ");
  if (iMod & KB_MODIFIER_META)  strcat (s, "Meta ");
	code	&= 0xFF;
	std::map<U_INT,std::string>::iterator rk = keyString.find(code);
	if (rk == keyString.end())	return;
	const char *kn = (*rk).second.c_str();
	strcat(s,kn);				// Store name
	//--- check option to suppress "none" ---------
	if (0 == opt)											return; 
	if (strncmp(kn,"none",4) != 0)		return;
	//--- suppress everything --------------------
	*s = 0;
  return;
}


