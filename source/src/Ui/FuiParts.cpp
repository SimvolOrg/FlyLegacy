/*
 * Fui.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
 * Copyright 2007      Jean Sabatier
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
//===========================================================================================
#include "../Include/Ui.h"
#include "../Include/Utility.h"
#include "../Include/Globals.h"
#include "../Include/CursorManager.h"
#include "../Include/database.h"
#include "../Include/FuiParts.h"
#include "../Include/FuiKeyMap.h"
#include "../Include/Joysticks.h"

#include <algorithm>
#include <vector>
#include <stdarg.h>
//==========================================================================
//  Edit table for hardware type
//==========================================================================
char *hwTAB[] = {
  "UNKNOWN",        // 0 
  "BUS",            // 1
  "SWITCH",         // 2
  "LIGHT",          // 3
  "FUSE",           // 4
  "STATE",          // 5
  "GAUGE",          // 6
  "OTHER",          // 7
  "CIRCUIT",        // 8
  "RADIO",          // 9
  "FLAP",           // 10
  "HILIFT",         // 11
  "GEAR",           // 12
  "BATTERY",        // 13
  "ALTERNATOR",     // 14
  "ANNUNCIATOR",    // 15
  "GENERATOR",      // 16
  "CONTACTOR",      // 17
  "SOUNDFX",        // 18
  "FLASHER",        // 19
  "INVERTER",       // 20
  "PID",            // 21
  "ENGINE",         // 22
  "WHEEL",          // 23
  "SPECIAL",        // 24
  "PROPELLER",      // 25
  "PISTON",         // 26
};
//==========================================================================
//  NOTE:  Edit routines are called by CListBox to fill the list
//         With specific object edited text.
//         It must return the number of edited lines
//===========================================================================
RD_COM GetComINDEX(U_INT mask);
//==========================================================================
//  CSLOT is used to store data for List box
//==========================================================================
CSlot::CSlot()
{ Fixed   = 0;
  Name[0] = 0;
  Offset  = 0;
  //--------------------
  nLIN    = 1;
  cLIN    = 0;
}
//--------------------------------------------------------------------------
//	By default, the search function look for the item number
//--------------------------------------------------------------------------
//================================================================================
//  Edit a country line
//================================================================================
void CCtyLine::Print(CFuiList *w,U_CHAR ln)
{ w->NewLine(ln);
  w->AddText(ln, 1, 0,GetName());
  return;
}
//================================================================================
//  Edit a State line
//================================================================================
void CStaLine::Print(CFuiList *w,U_CHAR ln)
{ w->NewLine(ln);
  w->AddText(ln, 1, 0,GetName());
  return;
}
//================================================================================
//  Edit an Airport line
//================================================================================
void CAptLine::Print(CFuiList *w,U_CHAR ln)
{ w->NewLine(ln);
  w->AddText(ln, 1,28,GetName());
  w->AddText(ln,18, 0,GetAica());
  w->AddText(ln,22, 0,GetIfaa());
  w->AddText(ln,26, 0,GetOtxt());
  w->AddText(ln,36, 0,GetClab());
  return;
}
//================================================================================
//  Edit a VOR/NDB line
//================================================================================
void CNavLine::Print(CFuiList *w,U_CHAR ln)
{ w->NewLine(ln);
  w->AddText(ln, 1,30,GetName());
  w->AddText(ln,18, 0,GetVaid());
  w->AddText(ln,26, 0,GetVtyp());
  w->AddText(ln,36, 0,GetClab());
  return;
}
//================================================================================
//  Edit a WPT line
//================================================================================
void CWptLine::Print(CFuiList *w,U_CHAR ln)
{ char edt[64];
  w->NewLine(ln);
  w->AddText(ln, 1,30,GetName());
  EditLat2DMS(GetLatitude(),edt);
  w->AddText(ln,18, 0,edt + 5);
  EditLon2DMS(GetLongitude(),edt);
  w->AddText(ln,26, 0,edt + 5);
  w->AddText(ln,36, 0,GetClab());
  return;
}

//================================================================================
//  CSLOT specific functions
//  Structure SCOM. Edit type of COM
//================================================================================
void  CComLine::FillCom()
{ int inx = GetComINDEX(mask);
  signal = SIGNAL_COM;
  sprintf(dfrq,"%6.2f",freq);
  strncpy(data,globals->comTAB[inx],16);
  return;
}
//--------------------------------------------------------------------------------
//  Edit ILS text
//--------------------------------------------------------------------------------
void  CComLine::FillILS()
{ signal = SIGNAL_ILS;
  sprintf(data," ILS RWY %s",irwy);
  sprintf(dfrq,"%6.2f",freq);
  return;
}
//----------------------------------------------------------------------------------
//  Edit a COM line
//----------------------------------------------------------------------------------
void CComLine::Print(CFuiList *w,U_CHAR ln)
{ w->NewLine(ln);
  w->AddText(ln, 0,0,GetData());
  w->AddText(ln,10,0,GetDfrq());
  return;
}
//----------------------------------------------------------------------------------
//  Check for ILS radio
//----------------------------------------------------------------------------------
Tag CComLine::IsaILS()
{ if (signal != SIGNAL_ILS)           return 0;
  if ((freq >= 108) && (freq < 118))  return 'snav';
  return 0;
}
//----------------------------------------------------------------------------------
//  Check for COM radio
//----------------------------------------------------------------------------------
Tag CComLine::IsaCOM()
{ if (signal != SIGNAL_COM)           return 0;
  if ((freq >= 118) && (freq < 137))  return 'scom';
  return 0;
}

//================================================================================
//  Runway Line
//================================================================================
//--------------------------------------------------------------------------------
//  Edit runway text
//--------------------------------------------------------------------------------
void  CRwyLine::EditRWY()
{ char  edt[16];
  sprintf(edt," RUNWAY %s-%s",Hend.rwid,Lend.rwid);
  strncpy(data,edt,16);
  sprintf(edt," % 4u ft",rwlg);
  strncpy(leng,edt,10);
  return;
}
//-----------------------------------------------------------------
//  Adjust runway coordinates 
//  1) Translate end coordinates to POV (in Nautical miles)
//  2) Scale to pixel
//  3) Compute corner coordinate by normal vector
//-----------------------------------------------------------------
void  CRwyLine::AdjustEnd(float scale,short tx,short ty)
{ Hend.dx = int((Hend.dx - tx) * scale);
  Hend.dy = int((Hend.dy - ty) * scale);
  Lend.dx = int((Lend.dx - tx) * scale);
  Lend.dy = int((Lend.dy - ty) * scale);
  return;
}
//-----------------------------------------------------------------
//  Compute normal to runway and compute runway corner
//  Runway wide is converted in miles and amplified for drawing clarity
//  -Invert y direction in definitive coordinates
//  -Center to mid canvas window coordinates
//-----------------------------------------------------------------
void  CRwyLine::ComputeCorner(short mx,short my)
{ if (0 == rwlg)   return;
  float wid = rwid * MILE_PER_FOOT * 1.4;                      
  int dx  = -(Lend.dy - Hend.dy);
  int dy  = +(Lend.dx - Hend.dx);
  Hend.cx = +((wid * dx) + Hend.dx) + mx;
  Hend.cy = -((wid * dy) + Hend.dy) + my;
  Lend.cx = +((wid * dx) + Lend.dx) + mx;
  Lend.cy = -((wid * dy) + Lend.dy) + my;
  Hend.dx =  Hend.dx + mx;
  Hend.dy = -Hend.dy + my;
  Lend.dx =  Lend.dx + mx;
  Lend.dy = -Lend.dy + my;
  return;
}
//-----------------------------------------------------------------
//  Check for end id
//-----------------------------------------------------------------

U_CHAR CRwyLine::CheckEnd(char *id,RWEND **end)   //int *px,int *py, char **d)
{	if	(strcmp(id,Hend.rwid) == 0) {*end = &Hend; return 1;}
/*
	{	
		*px	= Hend.dx;
		*py	= Hend.dy;
		if (d)	*d = Hend.ilsD;
		return 1;
	}
	*/
	if	(strcmp(id,Lend.rwid) == 0) {*end = &Lend; return 1;}
	/*
	{	*px = Lend.dx;
		*py = Lend.dy;
		if (d) *d = Lend.ilsD;
		return 1;
	}
	*/
	return 0;
}
//-----------------------------------------------------------------
//  Return drawing segment 1
//-----------------------------------------------------------------
void  CRwyLine::GetEnd01(int *px,int *py)
{ *px = Hend.dx;
  *py = Hend.dy;
  return;
}
//-----------------------------------------------------------------
//  Return drawing segment 2
//-----------------------------------------------------------------
void  CRwyLine::GetEnd02(int *px,int *py)
{ *px = Lend.dx;
  *py = Lend.dy;
  return;
}
//-----------------------------------------------------------------
//  Return drawing segment 3
//-----------------------------------------------------------------
void  CRwyLine::GetEnd03(int *px,int *py)
{ *px = Lend.cx;
  *py = Lend.cy;
  return;
} 
//-----------------------------------------------------------------
//  Return drawing segment 3
//-----------------------------------------------------------------
void  CRwyLine::GetEnd04(int *px,int *py)
{ *px = Hend.cx;
  *py = Hend.cy;
  return;
} 
//-----------------------------------------------------------------
//  Set ILS data into info
//-----------------------------------------------------------------
void CRwyLine::SetILS(CComLine *c)
{	char *idn = c->GetIdent();
  char *frq = c->GetDfrq();
  char *hid = Hend.rwid;
	if (strcmp(idn,hid) == 0)
	{	Hend.ifrq	= c->GetFreq();
		strncpy(Hend.ilsD,frq,8);
		return;
	}
	char *lid = Lend.rwid;
	if (strcmp(idn,lid) == 0)
	{	Lend.ifrq = c->GetFreq();
		strncpy(Lend.ilsD,frq,8);
		return;
	}
	return;
}
//----------------------------------------------------------------------------------
//  Edit a Runway line
//----------------------------------------------------------------------------------
void CRwyLine::Print(CFuiList *w,U_CHAR ln)
{ w->NewLine(ln);
  w->AddText(ln, 0,0,GetData());
  w->AddText(ln,10,0,GetElng());
  return;
}
//================================================================================
//  Edit a Flight Plan line
//================================================================================
void CFlpLine::Print(CFuiList *w,U_CHAR ln)
{ w->NewLine(ln);
  w->AddText(ln, 1, 1,GetMark());
  w->AddText(ln, 3,24,GetName());
  w->AddText(ln,18, 5,GetIden()); 
  w->AddText(ln,22,10,GetDist());
	w->AddText(ln,30,4, GetDirt());
  w->AddText(ln,34,12,GetAlti());
  w->AddText(ln,41,12,GetElap());
  w->AddText(ln,48,14,GetEtar());
  return;
}
//================================================================================
//  Edit a Flight Plan name
//================================================================================
void CFpnLine::Print(CFuiList *w,U_CHAR ln)
{ w->NewLine(ln);
  w->AddText(ln, 1,32, GetFile());
  w->AddText(ln,16, 0, GetName());
  return;
}
//--------------------------------------------------------------
//  Test for a match
//---------------------------------------------------------------
bool CFpnLine::Match(void *k)
{ char *key = (char*)k;
  int   lgr = strlen(key);
  return (strncmp(key,fname,lgr) == 0);
}

//================================================================================
//  Edit Aircraft selection
//================================================================================
void CAirLine::Print(CFuiList *w,U_CHAR ln)
{ w->TextPolicy('bold');
  w->NewLine(ln);
  w->AddText(ln,12,0,GetName());
  w->AddBitmap(ln,1,GetBitmap());
  w->TextPolicy('deff');
  w->Underline(ln);
  return;
}
//================================================================================
//  Edit GAS selection
//================================================================================
//  Edit title line 
//--------------------------------------------------------------------
void CGasLine::Title(CFuiList *w)
{ w->NewLine(0);
  w->AddText(0, 1, 0,"TANK DESCRIPTION");
  w->AddText(0,18, 0,"CAP (gals)");
  w->AddText(0,24, 0,"QTY (gals)");
  w->AddText(0,30, 0,"WEIGHT(lbs)");
  return;
}
//--------------------------------------------------------------------
//  Edit detail line
//--------------------------------------------------------------------
void CGasLine::Print(CFuiList *w,U_CHAR ln)
{ char edt[128];
  w->NewLine(ln);
  w->AddText(ln, 1, 0,(char*)cel->GetName());
  sprintf(edt,"%04d",int(cel->GetCellCap()));
  w->AddText(ln,18, 0,edt);
  sprintf(edt,"%-.02f",cel->GetCellQty());
  w->AddText(ln,24, 0,edt);
  sprintf(edt,"%04d",int(cel->GetCellWgh()));
  w->AddText(ln,30, 0,edt);
  return;
}
//================================================================================
//  Edit GAS selection
//================================================================================
//  Edit Lod line
//--------------------------------------------------------------------
void CLodLine::Print(CFuiList *w,U_CHAR ln)
{ char edt[128];
  w->NewLine(ln);
  w->AddText(ln, 1, 0,(char*)lod->GetName());
  sprintf(edt,"%5d lbs",int(lod->GetLoad()));
  w->AddText(ln,15, 0,edt);
  return;
}
//================================================================================
//  Edit Chart name
//================================================================================
void CMapLine::Print(CFuiList *w,U_CHAR ln)
{ w->NewLine(ln);
  w->AddText(ln, 1, 0, GetName());
  return;
}
//================================================================================
//  Edit a subsystem
//================================================================================
void CSubLine::Print(CFuiList *w,U_CHAR ln)
{ char  txt[16];
  U_INT   hw = aSub->GetHWID();
  if (hw > HW_MAX)  hw = 0;
  w->NewLine(ln);
  w->AddText(ln,1,0,GetName());
  sprintf(txt,"(%s)",tyst);
  w->AddText(ln,4,0,txt);
  w->AddText(ln,8,0,hwTAB[hw]);
  return;
}
//================================================================================
//  Edit a PID name
//================================================================================
void CPidLine::Print(CFuiList *w,U_CHAR ln)
{ w->NewLine(ln);
  w->AddText(ln,1,0,GetName());
  return;
}
//================================================================================
//  Edit a PID parameters
//================================================================================
void CPipLine::Print(CFuiList *w,U_CHAR ln)
{ char edt[64];
  w->NewLine(ln);
  w->AddText(ln, 1,0,GetName());
  double val = Pid->GetValue(PRM);
  sprintf_s(edt,10,"%.05f",val);
  w->AddText(ln,18,0,edt);
  return;
}
//================================================================================
//  Print a Key line
//================================================================================
void CKeyLine::Print(CFuiList *w,U_CHAR ln)
{ char txt[8];
  char iden[16];
  TagToString(txt,this->iden);
  sprintf(iden,"%s:",txt);
  w->NewLine(ln);
  w->AddText(ln, 1, 0,iden);
  w->AddText(ln, 4,28,GetName());
  w->AddText(ln,18, 0,GetKText());
  w->AddText(ln,28, 0,GetJText());
  return;
}

//================================================================================
//  Edit  Axe line
//================================================================================
void CAxeLine::Print(CFuiList *w,U_CHAR ln)
{ char edt[128];
	axe->Assignment(edt,128);
	w->NewLine(ln);
  w->AddText(ln, 1, 0, GetName());
  w->AddText(ln,10, 0, edt);
  return;
}
//================================================================================
//  Edit  Button line
//================================================================================
void CButLine::Print(CFuiList *w,U_CHAR ln)
{ w->NewLine(ln);
  w->AddText(ln, 1, 0, GetName());
  w->AddText(ln, 6, 0, GetKeyText());
  return;
}
//================================================================================
//  CheckList lines
//================================================================================
CChkLine::CChkLine(char nl) : CSlot() 
{ mark  = 0;
  cLIN  = nl;
	ActDF.Clear();
	ActDF.SetParam(this);
}

//-----------------------------------------------------------------------
//  Edit up to lm lines starting at index sLine on screen line ln
//  return the number of edited lines
//-----------------------------------------------------------------------
void CChkLine::Print(CFuiList *w,U_CHAR ln)
{ w->NewLine(ln);
  w->AddMark(ln,1,mark);
  w->AddText(ln,3,0,Name);
  if (NotLast())  return;
  w->AddSeparator(ln);
  return;
}
//=================================================================================
//  Keyboard line
//=================================================================================
//-----------------------------------------------------------------------
//  Clear keyboard entry
//-----------------------------------------------------------------------
void CKeyLine::ClearKey()
{ ktext[0] = 0;
  code     = 0;
  return;
}
//-----------------------------------------------------------------------
//  Assign a new key code
//-----------------------------------------------------------------------
void CKeyLine::SetCode(int cde)
{ code  = cde;
  formatKeyCode (ktext, cde);
  return;
}
//----------------------------------------------------------------------
//  Edit button text
//----------------------------------------------------------------------
void CKeyLine::SetJoysDef(CSimButton *sbt)
{ jtext[0] = 0;
  if (0 == sbt)   return;
  SJoyDEF *jsd = sbt->pjoy;
  if (0 == jsd)   return;
  sprintf(jtext,"(J%1d) button%02d",jsd->njs,sbt->nBut);
  return;
}
//=================================================================================
//  Generic Texture line
//=================================================================================
CTgxLine::CTgxLine()
{   type = 0;
    nite = 0;
}
//---------------------------------------------------------------------
//  Set Label
//---------------------------------------------------------------------
void CTgxLine::SetLabel(int tp,char *nm)
{ char txt[1024];
  sprintf(txt,"%03d-%s",tp,nm);
  strncpy(Name,txt,SLOT_NAME_DIM);
  Name[SLOT_NAME_DIM-1] = 0;
  return;
}
//---------------------------------------------------------------------
//  Edit label
//---------------------------------------------------------------------
void CTgxLine::Print(CFuiList *w,U_CHAR ln)
{ w->NewLine(ln);
  w->AddText(ln, 1, 0, Name);
  return;
}
//=================================================================================
//  Wind layer line
//=================================================================================
CWndLine::CWndLine()
{}
//---------------------------------------------------------------------
//  Edit label
//---------------------------------------------------------------------
void CWndLine::Print(CFuiList *w,U_CHAR ln)
{ char edt[16];
  w->NewLine(ln);
  w->AddText(ln, 1, 0, Name);
  int alt = int(slot->GetX());
  _snprintf(edt,15,"%5d feet",alt);
  w->AddText(ln,12,0,edt);
  int dir = int(slot->GetU());
  _snprintf(edt,15,"%3d deg", dir);
  w->AddText(ln,18,0,edt);
  int spd = int(slot->GetV());
  _snprintf(edt,15,"%3d Kts", spd);
  w->AddText(ln,24,0,edt);
  return;
}
//---------------------------------------------------------------------
//  Edit title line 
//--------------------------------------------------------------------
void CWndLine::Title(CFuiList *w)
{ w->NewLine(0);
  w->AddText(0, 1, 0,"LAYER NAME");
  w->AddText(0,12, 0,"ALTITUDE");
  w->AddText(0,18, 0,"DIRECTION");
  w->AddText(0,24, 0,"SPEED");
  return;
}
//=================================================================================
//  Object descriptor line
//  Ident is concatenation of FILE:TYPE-NAME
//=================================================================================
CObjLine::CObjLine(CWobj *obj)
{ char txt[MAX_PATH];
  char txg[8];
  Tag kind = obj->GetKind();
  TagToString(txg,kind);
  sprintf(txt,"%s:  %s-%s",obj->GetFileOBJ(),txg,obj->GetName());
  strncpy(Name,txt,40);
  Name[39] = 0;
  wObj     = obj;
  obj->IncUser();
}
//---------------------------------------------------------------------
//  Free line
//---------------------------------------------------------------------
CObjLine::~CObjLine()
{ wObj->DecUser();
}
//---------------------------------------------------------------------
//  Edit label
//---------------------------------------------------------------------
void CObjLine::Print(CFuiList *w,U_CHAR ln)
{ w->NewLine(ln);
  w->AddText(ln, 1, 0, Name);
	w->AddText(ln,20, 0,wObj->ModelName(0));
  return;
}
//================================================================================
//  Key file parser
//================================================================================
CKeyFile::CKeyFile(char *fname)
{ SStream s;
  if (OpenRStream (fname,s)) {
    ReadFrom (this, &s);
    CloseStream (&s);
  }
}
//--------------------------------------------------------------------------------
//  Free resources
//--------------------------------------------------------------------------------
CKeyFile::~CKeyFile()
{  std::map<Tag,CKsetDef *>::iterator i;
   for (i=kSet.begin(); i!=kSet.end(); i++) delete i->second;
}
//--------------------------------------------------------------------------------
//  Parse Key file
//--------------------------------------------------------------------------------
int CKeyFile::Read(SStream *str,Tag tag)
{ int nbr;
  switch (tag) {
  case 'vers':
    // Version
    ReadInt (&nbr, str);
    return TAG_READ;

  case 'kset':
    // KeySet sub-object
    { Tag idn;
      ReadTag (&idn, str);
      CKsetDef *ks = new CKsetDef(idn);
      ReadFrom(ks,str);
      kSet[idn] = ks;
    }
    return TAG_READ;
  }
  return TAG_IGNORED;
}
//-----------------------------------------------------------------------------
//  Get Default code for keiy idk in keyset ks
//-----------------------------------------------------------------------------
int CKeyFile::GetDefault(Tag ks,Tag idk)
{ CKsetDef *set = kSet[ks];
  if (0 == set) return 0;
  return set->GetCode(idk);
}
//================================================================================
//  Key Set parser
//================================================================================
CKsetDef::CKsetDef(Tag id)
{ idn = id;
}
//--------------------------------------------------------------------------------
//  Free tables
//--------------------------------------------------------------------------------
CKsetDef::~CKsetDef()
{DefKeys.clear(); }
//--------------------------------------------------------------------------------
//  Parse Key set
//--------------------------------------------------------------------------------
int CKsetDef::Read(SStream *str,Tag tag)
{ char txt[128];
  int  nbr;
  CKeyItem key;
  switch (tag) {
  case 'name':
    // Key set name
    ReadString (txt, 64, str);
    return TAG_READ;

  case 'user':
    ReadInt (&nbr, str);
    return TAG_READ;

  case 'enab':
    ReadInt (&nbr, str);
    return TAG_READ;

  case 'kkey':
    ReadFrom(&key,str);
    DefKeys[key.idn] = key.code;
    return TAG_READ;
  }
  return TAG_IGNORED;
}
//================================================================================
//  Key item parser
//================================================================================
CKeyItem::CKeyItem()
{}
//--------------------------------------------------------------------------------
//  Parse Key item
//--------------------------------------------------------------------------------
int CKeyItem::Read(SStream *str,Tag tag)
{ int nbr;
  char txt[128];
  switch (tag) {
  case 'kyid':
    // Key unique ID
    ReadTag (&idn, str);
    return TAG_READ;

  case 'name':
    // Key name
    ReadString (txt, 64, str);
    return TAG_READ;

  case 'code':
    // Key code and modifier
    ReadInt (&code, str);
    return TAG_READ;

  case 'user':
    // User-mappable?
    ReadInt (&nbr, str);
    return TAG_READ;

  case 'enab':
    // Enabled?
    ReadInt (&nbr, str);
    return TAG_READ;
  }
  return TAG_IGNORED;
}
//================================================================================
//  Global function to sort the list
//================================================================================
bool CompareName(CSlot *s1,CSlot *s2)
{ if (s1->IsFixed())    return true;
  if (s2->IsFixed())    return false;
  return ( (strncmp(s1->GetName(),s2->GetName(),SLOT_NAME_DIM) < 0));
}

//================================================================================
//  CLASS CListBox to interface a CFuiList Box
//================================================================================
//--------------------------------------------------------------------------------
//  Constructor. Build vector function  table
//--------------------------------------------------------------------------------
CListBox::CListBox()
{ Mother  = 0;
  wList   = 0;
  Opt     = 0;
  iFree   = 1;
  nNOD    = 0;
  Title   = 0;
}
//--------------------------------------------------------------------------------
CListBox::~CListBox()
{ if (iFree) EmptyIt();
}
//--------------------------------------------------------------------------------
//  Add a dynamic slot to the list
//--------------------------------------------------------------------------------
void  CListBox::AddSlot(CSlot *slot)
{ if (0 == slot)  return;
  slot->SetSeq(Num++);
  Obj.push_back(slot);
	nNOD	= Obj.size();
  return;
}
//--------------------------------------------------------------------------------
//  Search the list for a given slot identified by key. 
//  Derived class should supply the match function
//--------------------------------------------------------------------------------
CSlot *CListBox::Search(void *key)
{ std::vector<CSlot*>::iterator it;
  for (it = Obj.begin(); it != Obj.end();it++)
    { CSlot *slot = (*it);
      if (slot->Match(key)) return slot;
  }
  return 0;
}
//--------------------------------------------------------------------------------
//  Set box parameters
//  win       Mother window for notifications
//  idn       Tag of corresponding list box
//  tp        Type of box:
//            LIST_HAS_TITLE    First line is a title
//            LIST_DONT_FREE    Do not free item at destruction
//  lh        Line height in list box when SLOT_BMP is used for bitmap line
//--------------------------------------------------------------------------------
//  The last node to display is:
//  LAst node  - Total line to display
//--------------------------------------------------------------------------------
void  CListBox::SetParameters(CFuiWindow *win,Tag idn,U_INT tp,short lh)
{ htr             = lh;
  typ             = tp;
  Title           = (tp & LIST_HAS_TITLE)?(1):(0);
	if (0 == win)		return;
  Mother          = win;
  ident           = idn;
  wList           = (CFuiList*)Mother->GetComponent(idn);
  if (0 == wList) gtfo("ListBox bad tag for windows list");
  if (tp & LIST_NOHSCROLL) wList->NoHscroll();
  iFree           = (tp & LIST_DONT_FREE)?(0):(1);
  wList->SetSelectionTo(Title);
  wList->SetVSRatio(0);
  //----NEW INTERFACE A CONSERVER-----------------------------------
  mLIN            = wList->GetNberLine();
  if (htr)                  mLIN = wList->ChangeLineHeight(htr);
  if (typ & LIST_USE_MARKS) mLIN = wList->IncLineHeight(2);
  return InitP1();
}
//----------------------------------------------------------------------------------
//  Init parameters
//----------------------------------------------------------------------------------
void CListBox::InitP1()
{ //----------------------------------------------------------
  mLIN            = wList->GetNberLine();
  sLIN            = Title;                // Head line (  1 if title)
  aLIN            = Title;
  bLIN            = aLIN;
  hNOD            = Title;                // Current top node
  nCAP            = (mLIN - Title);       // Node capacity
  return InitP2();
}
//----------------------------------------------------------------------------------
//  Init parameters depending on node number
//----------------------------------------------------------------------------------
void CListBox::InitP2()
{ //---Compute last node to display -------
  nNOD            = int(Obj.size());
  eNOD            = nNOD - nCAP;          // Last node in top row
  if (eNOD <= 0)    eNOD = Title;
  //---Adjust max line number -------------
  int        nbn  = nNOD - Title;      // Number of real nodes
  mLIN            = (nCAP <= nbn)?(mLIN):(nNOD + Title);
  zLIN            = mLIN - 1;         // Last line to edit
  //-------Empty box-----------------------
  if (!IsEmpty())  return;
  sTop  = Title;
  sBot  = Title + 1;
  Num   = 0;
  return;
}
//----------------------------------------------------------------------------------
//  Redisplay after resizing
//----------------------------------------------------------------------------------
void CListBox::Resize()
{ /*
  U_INT   Nbr     = objlist.size();
  mLine           = wList->GetNberLine();
  hROW            = Title;
  hLIN            = 0;
  EditPage();
  wList->SetVSRatio(0);
  return;
  */
}
//----------------------------------------------------------------------------------
//  Reset values according to number of objects
//----------------------------------------------------------------------------------
void  CListBox::Reset()
{ wList->SetVSRatio(0);
  //---New interface -------------------------------------
  InitP1();
  ShowPage(sLIN);
  return;
}
//=========================================================================
//  NEW INTERFACE
//=========================================================================
//--------------------------------------------------------------------------------
//  Apply the global function to each slot
//--------------------------------------------------------------------------------
void CListBox::Apply(SlotCB fn)
{std::vector<CSlot*>::iterator it;
  for (it = Obj.begin(); it != Obj.end();it++)
  { CSlot *slot = (*it);
    (fn)(slot);
  }
  PrintPage();
  return;
}
//----------------------------------------------------------------------------------
//  Initial display
//----------------------------------------------------------------------------------
void  CListBox::Display()
{ Reset();
  return;
}
//----------------------------------------------------------------------------------
//  Refresh current page
//----------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//  return selected node
//--------------------------------------------------------------------------------
CSlot  *CListBox::GetSelectedSlot()
{ int No = hNOD + aLIN - sLIN;
  return (No < nNOD)?(Obj[No]):(0);
}
//--------------------------------------------------------------------------------
//  Return next primary slot
//  NOTE: p must be a primary slot
//--------------------------------------------------------------------------------
CSlot  *CListBox::NextPrimary(CSlot *p)
{ if (0 == p)				return 0;
	int No = p->GetSeq() + p->GetTotLines();
  return (No >= nNOD)?(0):(Obj[No]);
}
//--------------------------------------------------------------------------------
//  Return previous primary slot
//  NOTE: p must be a primary slot
//--------------------------------------------------------------------------------
CSlot  *CListBox::PrevPrimary(CSlot *p)
{ int No = p->GetSeq() - 1;         // Previous bottom
  if (No < Title) return 0;
  CSlot *lst = Obj[No];             // Bottom slot
  No  -= lst->GetCurLine();         // Top slot
  return Obj[No];
}
//--------------------------------------------------------------------------------
//  Return head primary slot
//--------------------------------------------------------------------------------
CSlot  *CListBox::HeadPrimary()
{ if (Title >= nNOD)      return 0;
  return Obj[Title];
}
//--------------------------------------------------------------------------------
//  Return Last primary slot
//--------------------------------------------------------------------------------
CSlot  *CListBox::LastPrimary()
{ int No = Obj.size();
  if (No == Title)    return 0;
  No--;
  CSlot *lst = Obj[No];             // Last node
  No -=  lst->GetCurLine();         // Primary node
  return Obj[No];
}

//--------------------------------------------------------------------------------
//  return selected node
//--------------------------------------------------------------------------------
U_INT  CListBox::GetSelectedNo()
{ return hNOD + aLIN - sLIN;
}
//--------------------------------------------------------------------------------
//  return selected node without title
//--------------------------------------------------------------------------------
int  CListBox::GetRealSelectedNo()
{ return hNOD + aLIN - sLIN - 1;
}
//--------------------------------------------------------------------------------
//  return Primary  node
//  This must be used for multiline item where only the head slot
//  has significant data other than text
//--------------------------------------------------------------------------------
CSlot  *CListBox::GetPrimary()
{ return pNOD;
}
//--------------------------------------------------------------------------------
//  return Next Primary  node from the selected node
//  This must be used for multiline item where only the head slot
//  has significant data other than text
//--------------------------------------------------------------------------------
CSlot  *CListBox::NextPrimToSelected()
{ if (pNOD)   return NextPrimary(pNOD);
  //-- return first primary node
  return HeadPrimary();
}
//--------------------------------------------------------------------------------
//  Delete all slots and empty the list
//  Fixed slots are allocated elsewhere and should not be deleted from here
//--------------------------------------------------------------------------------
void CListBox::EmptyIt()
{ std::vector<CSlot*>::iterator it;
  for (it = Obj.begin(); it != Obj.end();it++)
  { CSlot *slot = (*it);
    slot->Clean();
    if (slot->IsNotFixed()) delete (slot);
  }
  Obj.clear();
  Num		= 0;
	nNOD  = int(Obj.size());
  return;
}
//----------------------------------------------------------------------------------
//  Sort the list and display
//----------------------------------------------------------------------------------
void  CListBox::SortAndDisplay()
{ std::sort (Obj.begin(),Obj.end(),CompareName);
  Reset();
  return;
}
//-------------------------------------------------------------------------
//  Set Focus according to current selected line
//-------------------------------------------------------------------------
void CListBox::ShowPage(int lin)
{ SetFocus(lin);
  PrintPage();
  return;
}
//-------------------------------------------------------------------------
//  Search for slot with requested key.
//  Set focus on this slot
//-------------------------------------------------------------------------
void  CListBox::GoToKey(void *key)
{ if (0 == nNOD)    return;
  Reset();
  //---Search matching item  ------------------------------
  for (U_INT No = Title; No<Obj.size(); No++)
    { CSlot *slot = Obj[No];
      if (!slot->Match(key)) continue;
      //--Found a match set as first item ------
      hNOD  = No;
      SetFocus(sLIN);
      if (hNOD <= eNOD)     break;
      //--Dont go past the last node -----------
      hNOD  = eNOD;
      int dif = No - eNOD;
      SetFocus(sLIN + dif);
      break;

    }
  PrintPage();
  return;
}
//-------------------------------------------------------------------------
//  Go to item Number
//-------------------------------------------------------------------------
void CListBox::GoToItem(int No)
{	if (0 == nNOD)		return;
	Reset();
	//--- Set as first item ------------------------
	hNOD	= No;
	SetFocus(sLIN);
	if (hNOD > eNOD)
	{	//--Dont go past the last node -----------
    hNOD  = eNOD;
    int dif = No - eNOD;
    SetFocus(sLIN + dif);
	}
	PrintPage();
  return;
}
//-------------------------------------------------------------------------
//  Check for top node
//-------------------------------------------------------------------------
bool CListBox::IsTopNode(int No)
{ CSlot *nod = Obj[No];
  int hdn = No - nod->GetCurLine();
  return (hdn == Title);
}
//-------------------------------------------------------------------------
//  Check for Last node
//-------------------------------------------------------------------------
bool CListBox::IsLastNode(int No)
{ CSlot *nod = Obj[No];
  int    top = No - nod->GetCurLine();
  int    end = top + nod->GetTotLines();
  return (nNOD == end);
}
//-------------------------------------------------------------------------
//  Get ratio
//-------------------------------------------------------------------------
float CListBox::GetRatio(int num, int ttn)
{ if (num ==    0)    return 0;
  if (num >= eNOD)    return 1.0f;
  if (ttn ==    0)    return 0;
  return (float(num) / ttn);
}
//-------------------------------------------------------------------------
//  Set Focus according to current selected line
//-------------------------------------------------------------------------
void CListBox::SetFocus(int lin)
{ pNOD  = 0;                            // Primary node
  if (lin < Title)          return;     // Click on title
  int No  = hNOD + lin - sLIN;          // Selected node number
  if (No >= nNOD)           return;     // Empty box
  CSlot *nod  = Obj[No];                // Selected slot
  int    tot  = nod->GetTotLines();     // Total lines
  int    cln  = nod->GetCurLine();      // Current line
  int    sln  = lin - cln;              // Start selection
  int    eln  = sln + tot;              // End selection
  //----Save focus parameters --------------------------------
  if (sln <  sLIN)   sln = sLIN;        // Stay in page
  if (eln >  mLIN)   eln = mLIN;        // Stay in page
  wList->FocusIn(sln,eln);
  int num   = No   - Title;
  int ttn   = nNOD - Title;
  float rat = GetRatio(num,ttn);              //(ttn )?(float(num) / ttn ):(0);
  wList->SetVSRatio(rat);
  aLIN    = sln;
  bLIN    = eln;
  //---Store parameters of selected item ----------------
  No     -= cln;                          // Top slot
  pNOD    = Obj[No];                      // Primay Slot
  sTop    = No;                           // Top slot
  sBot    = No + pNOD->GetTotLines();     // next to Bottom slot
  return;
}
//-------------------------------------------------------------------------
//  Trace all items
//-------------------------------------------------------------------------
void CListBox::Trace()
{ TRACE("=== LISTBOX ===========================");
  TRACE("mLIN=%d (Maximum line in list)",mLIN);
	TRACE("eLIN=%d (Last selectable line)",eLIN);
	TRACE("sLIN=%d (Start line)"					,sLIN);
	TRACE("zLIN=%d (Last line)"						,zLIN);
	TRACE("aLIN=%d (First selected line)"	,aLIN);
	TRACE("bLIN=%d (Last  selected line)" ,bLIN);
	TRACE("hNOD=%d (Head node)"						,hNOD);
	TRACE("nNOD=%d (Total node inc Title)",nNOD);
	TRACE("nCAP=%d (Node capacity)"				,nCAP);
	return;
}
//-------------------------------------------------------------------------
//  Decrement focus
//-------------------------------------------------------------------------
void CListBox::DecFocus()
{ int sln = (aLIN > sLIN)?(aLIN - 1):(sLIN);
  return SetFocus(sln);
}
//-------------------------------------------------------------------------
//  Increment focus
//-------------------------------------------------------------------------
void CListBox::IncFocus()
{ int sln = (aLIN < zLIN)?(aLIN + 1):(zLIN);
  return SetFocus(sln);
}
//-------------------------------------------------------------------------
//  Print Focused lines 
//-------------------------------------------------------------------------
void CListBox::PrintFocus()
{ int dim = bLIN - aLIN;                // Number of node
  int No  = hNOD + aLIN - sLIN;         // Start node
  int ln  = aLIN;
  while (dim--)
  { CSlot *nod = Obj[No++];
    nod->Print(wList,ln++);
  }
  return;
}
//-------------------------------------------------------------------------
//  Print title if needed
//-------------------------------------------------------------------------
void CListBox::PrintTitle()
{ if (0 == sLIN)            return;
  if (0 == nNOD)            return;
  wList->SetBackTitle();
  CSlot *line = Obj[0];
  line->Title(wList);
  return;
}
//-------------------------------------------------------------------------
//  Edit a Page
//  The full page is edited including the focus
//  
//-------------------------------------------------------------------------
void CListBox::PrintPage()
{ wList->ClearPage();
  PrintTitle();  
  if (0 == nNOD)     return;
  int   row   = hNOD ;
  int   lin   = sLIN;                   // Starting line on Page (0 or 1)
  while (row < int(Obj.size()))
  { CSlot    *slot = Obj[row++];        // Slot to edit
    slot->Print(wList,lin);
    lin++;
    if (lin == mLIN) break;
  }
  eLIN  = lin;
  return;
}
//-------------------------------------------------------------------------
//  Swap selection
//  Clear previous focus and set new one
//-------------------------------------------------------------------------
void CListBox::SwapFocus(int lin)
{ if ((lin >= aLIN) && (lin < bLIN))  return;   // same focus
  if (lin  <  sLIN)                   return;   // Title
  if (lin  >= eLIN)                   return;   // Past end
  //--Reprint previous selection without focus ------
  int dim = bLIN - aLIN;
  wList->ClearBand(aLIN,dim);
  PrintFocus();
  //--Now set the new focus ------------------------
  SetFocus(lin);
  PrintFocus();
  return;
}
//-------------------------------------------------------------------------
//  Scroll up one line
//  Adjust focus if needed
//-------------------------------------------------------------------------
void CListBox::NextLine()
{ if (hNOD >= eNOD)     return;         // End is reached
  hNOD++;                               // Advance one node
  DecFocus();
  PrintPage();
  return;
}
//-------------------------------------------------------------------------
//  Scroll down one line
//  Adjust focus if needed
//-------------------------------------------------------------------------
void CListBox::PrevLine()
{ if (Title == hNOD)    return;         // First node reached
  hNOD--;
  IncFocus();
  PrintPage();
  return;
}
//-------------------------------------------------------------------------
//  Scroll up one Page
//  Adjust focus if needed
//-------------------------------------------------------------------------
void CListBox::NextPage()
{ if  (hNOD >= eNOD)    return ShowPage(zLIN);         // No more
  int tgn = hNOD + nCAP;                // Target node
  hNOD    = (tgn > eNOD)?(eNOD):(tgn);
  return ShowPage(sLIN);
}
//-------------------------------------------------------------------------
//  Scroll down one Page
//  Adjust focus if needed
//-------------------------------------------------------------------------
void CListBox::PrevPage()
{ if (hNOD == Title)    return ShowPage(sLIN);       // No more
  int tgn = hNOD - nCAP;              // Target node
  hNOD    = (tgn < Title)?(Title):(tgn);
  return ShowPage(zLIN);
}
//-------------------------------------------------------------------------
//  Scroll to ratio
//  Adjust focus to top
//-------------------------------------------------------------------------
void CListBox::GotoRatio(int pm)
{ int tgn = (nNOD * pm) / 100;
  hNOD    = (tgn > eNOD)?(eNOD):(tgn);
  if (0 == hNOD)  hNOD = sLIN;
  return ShowPage(sLIN);
}
//--------------------------------------------------------------------------------
//  Move up one item at selected position
//--------------------------------------------------------------------------------
void CListBox::MoveUpItem()
{ if (IsEmpty())        return;
  int No = GetTopSlot();
  if (IsTopNode(No))    return;     // Already at top
  //---Swap the selected item with the previous one ---------
  int ns = SwapPrevNode(No) + aLIN;
  //---Check for selection inside the page -------
  if (ns  <= sLIN) ns = MoveToEnd(ns);
  return ShowPage(ns);
}
//--------------------------------------------------------------------------------
//  Move down one item at selected position
//--------------------------------------------------------------------------------
void CListBox::MoveDwItem()
{ if (IsEmpty())        return;
  int No = GetTopSlot();
  if (IsLastNode(No))   return;     // Already at end
  //---Swap the selected item with the next one ------
  int ns = SwapNextNode(No) + aLIN;
  //---Check for selection inside page ---------------
  if (ns >= zLIN) ns = MoveToTop(ns);
  return ShowPage(ns);
}
//--------------------------------------------------------------------------------
//  Set given node near the bottom of page
//  fr is the current line (may be negative)
//--------------------------------------------------------------------------------
int CListBox::MoveToEnd(int fr)
{ //--- Compute head node -------------------
  int to = zLIN;                // Last line
  while (fr != to)
  { if (hNOD == sLIN)   return fr;
    hNOD--;                     // One more slot up
    fr++;                       // One line down
  }
  return fr;
}
//--------------------------------------------------------------------------------
//  Set given node near the top of page
//  fr is the current line (may be negative)
//--------------------------------------------------------------------------------
int CListBox::MoveToTop(int fr)
{ //--- Compute head node -----------------
  int to = sLIN;              // Target line
  while (fr != to)
  { if (hNOD == eNOD)   return fr;
    hNOD++;                   // One slot down
    fr--;                     // One line up
  }
  return fr;
}
//-------------------------------------------------------------------------------
//  Renum sequence from node n0 to node n1
//-------------------------------------------------------------------------------
void CListBox::Renum(int n0, int n1)
{ U_INT sq  = n0;
  int  end  = int(Obj.size());
  for (int k=n0; k<=n1; k++)
  { if (k >= end)  return;
    CSlot *slt = Obj[k];
    slt->SetSeq(sq++);
  }
  return;
}
//-------------------------------------------------------------------------------
//  Return line offset for the previous node from current line
//  One item may include several lines(slots).  Exemple
//  ItemA = (na,nb,nc)
//  ItemB = (nd,ne)
//  So we must recover the primary slot of each item from the current 
//  selected slot identified by (No)
//-------------------------------------------------------------------------------
int CListBox::SwapPrevNode(int No)
{ CSlot *nda  = Obj[No];
  int    tt1  = nda->GetTotLines();           // Total slots in item
  int    cl1  = nda->GetCurLine();            // Current slot this item
  int    bt1  = No + tt1 - cl1 - 1;           // Last slot this item
  int    bt2  = bt1 - tt1;                    // Last slot for previous item
  CSlot *ndp  = Obj[bt2];                     // Access last slot
  int    tp2  = bt2 - ndp->GetCurLine();      // Top slot  for previous item
  int    tt2  = nda->GetTotLines();           // Total slots previous item
  for (int k=0; k!= tt2; k++) Shift(bt1,tp2); // Swap items
  Renum(tp2,bt1);                             // Reset sequence to this range
  return (tp2 - No);                          // Offset to new slot (top of previous)

}
//-------------------------------------------------------------------------------
//  Return line offset for the next slot from current line
//  One item may include several lines(slots).  Exemple
//  ItemA = (na,nb,nc)
//  ItemB = (nd,ne)
//  So we must recover the primary slot of each item from the current 
//  selected slot identified by (No)
//-------------------------------------------------------------------------------
int CListBox::SwapNextNode(int No)
{ CSlot *nda  = Obj[No];
  int    tt1  = nda->GetTotLines();           // Total slots in item
  int    cl1  = nda->GetCurLine();            // Current slot for this item
  int    tp1  = No - cl1;                     // Top slot for this item
  int    tp2  = tp1 + tt1;                    // Top slot next item
  CSlot *nds  = Obj[tp2];                     // Next slot
  int    tt2  = nds->GetTotLines();           // Total slot next item
  int    bt2  = tp2 + tt2 - 1;                // Bottom slot next item
  for (int k=0; k!=tt2; k++) Shift(bt2,tp1);  // Swap items
  Renum(tp1,bt2);                             // Reset sequence to this range
  return (tp2 - No);                          // Offset to new node (top of next)
}
//-------------------------------------------------------------------------------
//  Shif nodes given by the range parameters
//  fr: From node
//  to: ending node
//-------------------------------------------------------------------------------
void CListBox::Shift(int fr,int to)
{ CSlot * nod = Obj[fr];                  // start node (right position)
  int ns = fr - 1;                        // Node source to shift
  int nd = fr;                            // Node destination
  do  { Obj[nd--] = Obj[ns--]; }          // Shift node one place left
  while (nd != to);
  Obj[nd] = nod;      
  return;
}
//-------------------------------------------------------------------------------
//  Delete selected item
//-------------------------------------------------------------------------------
void CListBox::DeleteItem()
{ if (IsEmpty())              return;
  int No = GetTopSlot();
  //----Find top slot and delete all slots ----
  CSlot *nod = Obj[No];                   // Current slot
  int   top  = No - nod->GetCurLine();    // Top slot
  int   tt1  = nod->GetTotLines();        // Total slot in item
  //----Delete all slots related to item -------
  std::vector<CSlot*>::iterator it = Obj.begin() + top;
  for (int k=0; k<tt1; k++)
  { CSlot *ndx = (*it);
    delete ndx;
    it  = Obj.erase(it);
  }
  Renum(top,nNOD);      // Reset sequence to this range
  //---Update parameters depending on number of nodes --------
  InitP2();
	if (IsEmpty())		bLIN = aLIN;
  if (IsEmpty())    return ShowPage(sLIN);
  //--- Set selection to previous item if last was deleted --
  if (top < nNOD)   return ShowPage(top);
  //----Back selection one line -----------------------------
  int ns = aLIN;
  ns--;
  if (ns > sLIN)    return ShowPage(ns);
  //---Move page toward end ---------------------------------
  ns = MoveToEnd(ns);
  ShowPage(ns);
  return;
}
//-------------------------------------------------------------------------------
//  Return insert point 
//-------------------------------------------------------------------------------
int CListBox::GetInsertPoint(int cln)
{ if (IsEmpty())  return Title;
  if (0 != cln)   return nINS;
  //---Insert point is after last slot of selected item --------
  return sBot;
}
//-------------------------------------------------------------------------------
//  Add item after current selected item
//  Only the slot are added.
//  When all slots for a given item are added, the function EndInsert) must be called
//  This is because the selection mechanism works only when all slots
//  of a given item are present
//-------------------------------------------------------------------------------
void CListBox::InsSlot(CSlot *nod)
{ int cln = nod->GetCurLine();
  int end = nod->GetTotLines() - 1;
  //---Compute insert point ---------------------------
  nINS = GetInsertPoint(cln);
  //---Insert this item -------------------------------
  std::vector<CSlot*>::iterator in = Obj.begin() + nINS;
  Obj.insert(in,nod);
  Renum(nINS++,nNOD);
  //--Update parameters depending on number of nodes --
  InitP2();
  //--Check for end of insertion ----------------------
  if (cln != end) return;
  return EndInsert();
}
//------------------------------------------------------------------------------
//  End of insert
//  The focus is set to the new inserted item
//------------------------------------------------------------------------------
void CListBox::EndInsert()
{ int ns  = bLIN;                       // Line after insert point
  if (ns < zLIN)  return ShowPage(ns);  //OK same page
  //---Scroll down one page -------------------------------------
  ns  = MoveToTop(ns);
  return ShowPage(ns);
}
//==================================================================================

//--------------------------------------------------------------------------------
//  Vertical ScrollBox notification Handler
//  This function return true if the list need to be updated
//--------------------------------------------------------------------------------
bool CListBox::VScrollHandler(U_INT pm,EFuiEvents evn)
{ U_SHORT ln = (U_SHORT)pm;
  if (Title == nNOD)  return true;
  switch (evn)  {
    case  EVENT_SELECTITEM:
        SwapFocus(ln);
        return true;

    case  EVENT_NEXTLINE:    
        NextLine();
        return true;
    case  EVENT_PREVLINE:
        PrevLine();
        return true;
    case  EVENT_PREVPAGE:
        PrevPage();
        return true;
    case  EVENT_NEXTPAGE:
        NextPage();
        return true;
    case  EVENT_THBRATIO:
        GotoRatio(pm);
        return true;

  }
  return false;
}
//================================================================================
//  Editor:  Edit a set of lines text based on keystrokes
//================================================================================
CEditor::CEditor(short s,CText *t)
{ obj = t;
  lim = s - 1;
  txt = 0;
  lin = 0;
  pos = 0;
}
//-----------------------------------------------------------------------
//  Set line parameters
//-----------------------------------------------------------------------
void CEditor::SetLine(char *t,char l)
{ txt       = t;
  txt[lim]  = 0;
  lin       = l;
  return;
}
//-----------------------------------------------------------------------
//  Return the position of limiter
//-----------------------------------------------------------------------
int CEditor::EndMark(char *tex)
{ char *lcr = strchr(tex,0);
  return (lcr - tex);
}
//-----------------------------------------------------------------------
//  Forward one position
//-----------------------------------------------------------------------
void CEditor::KeyNXT()
{ if (txt[pos++])   return;
  txt = obj->NextLine(lin++);
  if (0 == txt)     return;
  txt[lim] = 0;
  pos = 0;
  return;
}
//-----------------------------------------------------------------------
//  Backward one position
//-----------------------------------------------------------------------
void CEditor::KeyPRV()
{ if (0 < pos--)      return;
  if (0 == lin)       return;
  txt = obj->PrevLine(lin--);
  if (0 == txt)       return;
  txt[lim]  = 0;
  pos       = EndMark(txt);
  return;
}
//-----------------------------------------------------------------------
//  Insert last char as first in next line
//-----------------------------------------------------------------------
int CEditor::PushLastCharInNextLine(char *tex,char lnb)
{ short lcp = (lim - 1);
  char *end = tex + lcp;
  char  kar = *end;
  *end      = 0;
  //--Insert in next line -----------------------
  char *nxl = obj->NextLine(lnb++);
  if (0 == nxl) return lcp;
  InsertCharInLine(kar,nxl,lnb,0);
  return lcp;
}
//-----------------------------------------------------------------------
//  Insert a character in next line
//-----------------------------------------------------------------------
int CEditor::InsertCharAsFirst(char kar,char *tex,char lnb, short psn)
{ char *nxl = obj->NextLine(lnb++);
  if (0 == nxl) return psn;
  InsertCharInLine(kar,nxl,lnb,0);
  txt = nxl;
  lin = lnb;
  return 1;
}
//-----------------------------------------------------------------------
//  Insert a character in line
//-----------------------------------------------------------------------
int CEditor::InsertCharInLine(char kar, char *tex, char lnb, short psn)
{ //--- Check for insert in this line ------
  if (psn == lim) return InsertCharAsFirst(kar,tex,lnb,psn);
  //--- Make room for one character --------
  int   end = EndMark(tex);
  char *ins = tex + psn;
  if  (end == lim)  end = PushLastCharInNextLine(tex,lnb); 
  char *src = tex + end;
  char *dst = src + 1;
  //--- insert the character in the line---
  while (dst != ins)  *dst-- = *src--;
  *ins  = kar;
  return (psn + 1);
}
//-----------------------------------------------------------------------
//  Keystroke event
//-----------------------------------------------------------------------
void CEditor::Key(U_INT k, short p)
{ if (0 == txt) return;
  pos = p;
  switch (k)  {
  //---Right arrow --------------------------------
  case 0x20066:
    KeyNXT();
    break;
  //---Left Arrow ---------------------------------
  case 0x20064:
    KeyPRV();
    break;
  }
  //---return cursor position ---------------------
  obj->CursorAt(lin,pos);
  return;
}
//====================END OF FILE ==========================================================