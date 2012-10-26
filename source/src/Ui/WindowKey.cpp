/*
 * WindowVectorMap.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005 Chris Wallace
 * Copyright 2007 Jean Sabatier
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
#include "../Include/FlyLegacy.h"
#include "../Include/Utility.h"
#include "../Include/Globals.h"
#include "../Include/Fui.h"
#include "../Include/FuiParts.h"
#include "../Include/FuiKeyMap.h"
#include "../Include/KeyMap.h"
#include "../Include/Joysticks.h"
//=================================================================================
//  KEYBOARD MAPPING is an internal structure constructed by decoding
//  the file FlyLegacyKey.txt.  
//  The root struture is a CKeyMap object, that contains a list of 
//  CKeySet, each of which contains a list of CKeyDefinition for each key
//  The root structure may be accessed through the pointer
//  globals->kbd
//=================================================================================
//  MODE MENU
//=================================================================================
char *CFuiKeyMap::modMENU[] = {
  "Just Display",
  "CHANGE KEY",
  ""
};
//=================================================================================
//    CALL BACK FROM KEYBOARD
//=================================================================================
int WhenKeyHit(Tag winID, int iCode)
{ CFuiKeyMap *wkn = (CFuiKeyMap*)globals->fui->GetFuiWindow(winID);
  return wkn->KeyPress(iCode);
}
//=================================================================================
//    CALL BACK FROM JOYSTICK
//=================================================================================
void WhenButtonHit(char code, JoyDEV *J, CSimAxe *A, int B, Tag W)
{ if (code != JOY_BUT_MOVE)		return;
	CFuiKeyMap *win = (CFuiKeyMap*)globals->fui->GetFuiWindow(W);
  win->ButtonHit(J,B);
  return;
}

//=================================================================================
//  Window for keyboard mapping
//=================================================================================
CFuiKeyMap::CFuiKeyMap(Tag idn, const char *filename)
:CFuiWindow(idn,filename,0,0,0)
{ char *err = "Incorrect TEMPLATE OptionsKEYsbuttons.WIN file";
  //------------------------------------------------------
  jsm     = globals->jsm;
  item    = 0;
  iden    = 0;
  modify  = 0;
  wrtJOY  = 0;
  dfk     = 0;
  //----inhibit resizing -------------------------------
  RazProperty(FUI_XY_RESIZING);
  //----Init components---------------------------------
  selPop    = (CFuiPopupMenu*)GetComponent('cate');
  if (0 == selPop)  gtfo(err);
  label     = (CFuiLabel*)    GetComponent('?lbl');
  if (0 == label )  gtfo(err);
	wkeys			= (CFuiLabel*)    GetComponent('keys');
	if (0 == wkeys )   gtfo(err);
  modPop    = (CFuiPopupMenu*)GetComponent('sort');
  if (0 == modPop )  gtfo(err);
  modPop->SetText("MODE");
  //---Init resizing properties ------------------------
  SetXRange(w,w);               // Fixed width
  SetChildProperty('clrb',FUI_VT_RELOCATE);
  SetChildProperty('defb',FUI_VT_RELOCATE);
  SetChildProperty('defk',FUI_VT_RELOCATE);
  SetChildProperty('clrk',FUI_VT_RELOCATE);
  SetChildProperty('sort',FUI_VT_RELOCATE);
  SetChildProperty('list',FUI_VT_RESIZING);
  SetProperty(FUI_VT_RESIZING);
  //-- Init List boxes ---------------------------------
  keyBOX.SetParameters(this,'list',0);
  if (0 == InitPopup()) Close();
  dfk     =  new CKeyFile("System/default.key");        // Load Default Key
	//-----------------------------------------------------
	if (jsm->IsBusy())	Close();
	else	{	jsm->StartDetection(WhenButtonHit, windowId);
					globals->kbd->StartKeyDetect(WhenKeyHit,windowId);
				}
	//---- Marker 1 -----------------------------------
	char *ds = DupplicateString("***DEB WinKey",16);
}
//--------------------------------------------------------------------------
//  Free resources
//--------------------------------------------------------------------------
 CFuiKeyMap::~CFuiKeyMap()
 { jsm->SetFree();
	 StopDetection(); 
   SaveConfiguration();
   if (item) delete [] item;
   if (iden) delete [] iden;
   if (dfk)  delete dfk;
	 //---- Marker 2 -----------------------------------
	 char *ds = DupplicateString("***End WinKey",16);

 }
 //-------------------------------------------------------------------------
 //   SAVE KEY AND JOYSTICK CONFIGURATION
 //-------------------------------------------------------------------------
 void CFuiKeyMap::SaveConfiguration()
 {  if (0 == modify) return;
    globals->kbd->SaveCurrentConfig();
    jsm->SaveConfiguration();
    return;
 }
//--------------------------------------------------------------------------
//  Init popup from the set of Keys
//--------------------------------------------------------------------------
int CFuiKeyMap::InitPopup()
{ CKeyMap *km = globals->kbd;
  if (0 == km)  return 0;
  int ns = km->GetNumKeySets();
  item  = new char*[ns + 1];
  iden  = new Tag  [ns];
  CKeySet *ks = 0;
  int       x = 0;
  for (x = 0; x < ns; x++)
  { ks = km->GetKeySet(x);
    item[x] = (char*)ks->GetName();
    iden[x] = ks->GetTag();
  }
  item[ns] = "";       // Last entry
  //---Init the popup menu with keyset labels-----
  selPop->CreatePage(&mSEL,item);
  modPop->CreatePage(&modMEN,modMENU);
  SelectMode(0);
  SelectSet(0);
  return 1;
}
//--------------------------------------------------------------------------
//  Resize notification
//--------------------------------------------------------------------------
void CFuiKeyMap::NotifyResize(short dx,short dy)
{ keyBOX.Resize();
  return;
}
//--------------------------------------------------------------------------
//  Set Popup title and display selected mode
//--------------------------------------------------------------------------
void CFuiKeyMap::SelectMode(U_INT No)
{ modOpt  = No;
  modPop->SetButtonText((char*)modMEN.aText[No]);
  return;
}
//--------------------------------------------------------------------------
//  Stop detection
//--------------------------------------------------------------------------
void CFuiKeyMap::StopDetection()
{ globals->kbd->StopKeyDetect();
  return;
}
//--------------------------------------------------------------------------
//  Set Popup title and display all Keys according to selected SET
//--------------------------------------------------------------------------
void CFuiKeyMap::SelectSet(U_INT No)
{ selOpt  = No;
  selPop->SetButtonText((char*)mSEL.aText[No]);
  keyBOX.EmptyIt();
  DisplayKeys(iden[No]);
  label->SetText("Select a mode and press Key");
	wkeys->SetText("");
  return;
}
//--------------------------------------------------------------------------
//  Display Key Set corresponding to ident
//--------------------------------------------------------------------------
void CFuiKeyMap::DisplayKeys(Tag idn)
{ CKeySet *ks = globals->kbd->FindKeySetById(idn);
  if (0 == ks)  return;
  int nk = ks->GetNumKeyDefinitions();
  std::map<Tag,CKeyDefinition*> kmap;
  ks->GetMap(kmap);
  std::map<Tag,CKeyDefinition*>::iterator it;
  for (it = kmap.begin(); it != kmap.end(); it++)
  { CKeyDefinition *kdf = (*it).second;
    CSimButton     *sbt = kdf->GetButton();
    CKeyLine       *kln = new CKeyLine;
    keyBOX.AddSlot(kln);
    kln->SetSlotName((char*)kdf->GetName());
    kln->SetDef(kdf);
    kln->SetCode(kdf->GetCode());
    kln->SetIden(kdf->GetTag());
    if (0 == sbt) continue;
    kln->SetJoysDef(sbt);
  }
  keyBOX.SortAndDisplay();
  return;
}
//--------------------------------------------------------------------------
//  Check that key can be modified
//--------------------------------------------------------------------------
bool CFuiKeyMap::CheckKey(int cde,CKeyDefinition *kdf)
{ char *msg = "";
  if (cde == KB_KEY_SPACE) msg = "**CANNOT USE SPACE**";
  if (!kdf->IsUserMod())   msg = "**Cant modify this key**";
  label->RedText(msg);
  return (msg[0])?(false):(true);
}
//--------------------------------------------------------------------------
//  Return error message
//--------------------------------------------------------------------------
bool CFuiKeyMap::Message(char *msg)
{ label->RedText(msg);
  return false;
}
//--------------------------------------------------------------------------
//  Unassign key that uses the same mapping
//--------------------------------------------------------------------------
void CFuiKeyMap::Unassign(CKeyDefinition *kdf,int cde)
{ char ktx[32];
  formatKeyCode(ktx,cde,1);
  label->EditText("%s was used by %s",ktx,kdf->GetName());
  CKeyLine *old = (CKeyLine*)keyBOX.Search(kdf);
  if (0 == old) return;
  old->SetCode(0);
  return;
}
//--------------------------------------------------------------------------
//  Key was pressed.  Process according to the current mode
//--------------------------------------------------------------------------
int CFuiKeyMap::KeyPress(int cde)
{ if (modOpt == 1)  return NewKeyCode(cde);
  char ktx[32];
  formatKeyCode(ktx,cde,1);
	wkeys->EditText("Key pressed: %s: code %05d",ktx,cde);
	Message("To Modify set MODE to CHANGE !");
  return 0;
}
//--------------------------------------------------------------------------
//  Change key code to the selected key
//--------------------------------------------------------------------------
int CFuiKeyMap::NewKeyCode(int cde)
{ CKeyLine       *kln = (CKeyLine*)keyBOX.GetSelectedSlot();
  CKeyDefinition *kdf = kln->GetKDef();
  if (!CheckKey(cde,kdf)) return 1;
  CKeyDefinition *kpv = globals->kbd->UnAssign(cde,kdf);  
  if (kpv)              Unassign(kpv,cde);
  modify  = 1;
  globals->kbd->Assign(cde,kdf);
  kln->SetCode(cde);
  keyBOX.Refresh();
  return 1;
}
//--------------------------------------------------------------------------
//  Simply clear current key
//--------------------------------------------------------------------------
void CFuiKeyMap::ClearKey()
{ if (modOpt == 0) {Message("To Modify set MODE to CHANGE !"); return; }
  CKeyLine       *kln = (CKeyLine*)keyBOX.GetSelectedSlot();
  CKeyDefinition *kdf = kln->GetKDef();
  globals->kbd->ClearCode(kdf);
  kln->ClearKey();
  modify  = 1;
  keyBOX.Refresh();
  return;
}
//--------------------------------------------------------------------------
//  Regenerate the code from the Default set
//--------------------------------------------------------------------------
void CFuiKeyMap::DefaultKeys()
{ if (modOpt == 0) {Message("To Modify set MODE to CHANGE !"); return; }
  globals->kbd->SetDefaultValues(dfk);
  SelectSet(0);
  modify  = 1;
  return;
}
//--------------------------------------------------------------------------
//  A Joystick button is pressed
//  -Remove button previous definition
//  -Remove button from key definiton
//  -Create a new button definition
//  -Assign to key
//--------------------------------------------------------------------------
void CFuiKeyMap::ButtonHit(JoyDEV *jsd,int bt)
{ if (modOpt == 0) {Message("To Modify set MODE to CHANGE !"); return; }
	modify = 1;
  Unassign(jsd,bt);
  CKeyLine        *kln = (CKeyLine*)keyBOX.GetSelectedSlot();
  CKeyDefinition  *kdf = kln->GetKDef();
  CSimButton      *old = kdf->LinkTo(0);
	jsm->RemoveButton(old);
  CSimButton      *sbt = jsm->AddButton(jsd,bt,kdf);
  kln->SetJoysDef(sbt);
  keyBOX.Refresh();
  return;
}
//--------------------------------------------------------------------------
//  Search line slot for key definition 
//--------------------------------------------------------------------------
void CFuiKeyMap::UnlinkLine(CKeyDefinition *kdf)
{ CKeyLine *line = 0;
	for (U_INT k = 0; k < keyBOX.GetSize(); k++)
	{	line = (CKeyLine*)keyBOX.GetSlot(k);
		if (line->GetKDef() != kdf)	continue;
		line->SetJoysDef(0);
		return;
	}
	return;
}
//--------------------------------------------------------------------------
//  Unlink the button from the actual key definition 
//--------------------------------------------------------------------------
void CFuiKeyMap::Unassign(JoyDEV *jsd,int bt)
{ CSimButton        *sbt = jsd->GetButton(bt);
  CKeyDefinition    *kdf = (sbt)?(sbt->kdf):(0);
	jsm->RemoveButton(sbt);
  if (0 == kdf)      return;
  kdf->LinkTo(0);
	UnlinkLine(kdf);
  return;
}
//--------------------------------------------------------------------------
//  Clear Button from selected component
//--------------------------------------------------------------------------
void CFuiKeyMap::ClearButton()
{ CKeyLine *kln = (CKeyLine*)keyBOX.GetSelectedSlot();
  CKeyDefinition    *kdf  = kln->GetKDef();
  CSimButton        *old  = kdf->GetButton();
  modify  = 1;
  kln->SetJoysDef(0);
  kdf->LinkTo(0);
	globals->jsm->RemoveButton(old);
  keyBOX.Display();
  return;
}
//--------------------------------------------------------------------------
//  Notification from Popup
//--------------------------------------------------------------------------
void CFuiKeyMap::NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn)
{ switch (id) {
    case 'cate':
      if (evn == EVENT_POP_CLICK)  SelectSet((U_INT)itm);
      return;
    case 'sort':
      if (evn == EVENT_SELECTITEM) SelectMode((U_INT)itm);
      return;

}
return;
}
//-------------------------------------------------------------------------
//  EVENT notifications from child windows
//-------------------------------------------------------------------------
void  CFuiKeyMap::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ if (idm == 'sysb')  {SystemHandler(evn); return;}
  switch (idm)  {
      case  'list':
        keyBOX.VScrollHandler((U_INT)itm,evn);
        return;
      //---Default key ---------------------------
      case 'defk':
        DefaultKeys();
      //---Clear Key ----------------------------
      case 'clrk':
        ClearKey();
        return;
      //----Clear button -----------------------
      case 'clrb':
        ClearButton();
        return;
  }
  return ;
}
//=======================END OF FILE ==================================================