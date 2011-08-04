/*
 * WindowCheckList.cpp
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
#include "../Include/Robot.h"
#include "../Include/PlanDeVol.h"
#include <vector>
//=====================================================================================
//  preconddition decoding structure
//=====================================================================================
struct ROBOT_COND	{
	char *txt;
	char  code;
};
//=====================================================================================
//  preconddition decoding table
//=====================================================================================
ROBOT_COND	rcondTAB[]  = {
	{"State == %d",D2R2_IF_STATE_EQU},
	{"State != %d",D2R2_IF_STATE_NEQ},
	{"State >  %d",D2R2_IF_STATE_GTR},
	{"State >= %d",D2R2_IF_STATE_GEQ},
	{"State <  %d",D2R2_IF_STATE_LTN},
	{"State <= %d",D2R2_IF_STATE_LEQ},
	{0},
};
//=====================================================================================
//  Globals function Uncheck all
//=====================================================================================
void UnMarkCheckList(CSlot *slot)
{ CChkLine *lin = (CChkLine*)slot;
  lin->Unmark();
}
//-------------------------------------------------------------------------
//  Check for multiple Left Click action
//  Multiple Left Click with Mouse (MLCM)
//-------------------------------------------------------------------------
bool CheckChapter::MLClikAction(char *buf)
{ int p1, p2, rp, tm = 0;
  int nf = sscanf(buf," %d * L-Click ( %d , %d , %d )",&rp,&p1, &p2, &tm);
  if (4 != nf)    return false;
	ra.actn	= D2R2_LEFT_CLICK_COORD;
	ra.pm1	= p1;
	ra.pm2	= p2;
  ra.rptn = rp;
	ra.timr = tm;
  return true;
}
//-------------------------------------------------------------------------
//  Check for Single Left Click action
//  Single Left Click with Mouse (SLCM)
//-------------------------------------------------------------------------
bool CheckChapter::SLClikAction(char *buf)
{ int p1, p2 = 0;
  int nf = sscanf(buf,"L-Click ( %d , %d )",&p1, &p2);
  if (2 != nf)    return false;
	ra.actn	= D2R2_LEFT_CLICK_COORD;
	ra.pm1	= p1;
	ra.pm2	= p2;
	ra.rptn	= 1;
  return true;
}
//-------------------------------------------------------------------------
//  Check for Right Click action
//  Multiple Right Click with Mouse (MLCM)
//-------------------------------------------------------------------------
bool CheckChapter::MRClikAction(char *buf)
{ int p1, p2, rp, tm = 0;
  int nf = sscanf(buf," %d * R-Click( %d , %d , %d )",&rp,&p1, &p2, &tm);
  if (4 != nf)    return false;
	ra.actn	= D2R2_RITE_CLICK_COORD;
	ra.pm1	= p1;
	ra.pm2	= p2;
	ra.rptn = rp;
	ra.timr = tm;
  return true;
}
//-------------------------------------------------------------------------
//  Check for Right Click action
//  Single Right Click with Mouse (SRCM)
//-------------------------------------------------------------------------
bool CheckChapter::SRClikAction(char *buf)
{ int p1, p2 = 0;
  int nf = sscanf(buf," R-Click( %d , %d )",&p1, &p2);
  if (2 != nf)    return false;
	ra.actn		= D2R2_RITE_CLICK_COORD;
	ra.pm1		= p1;
	ra.pm2		= p2;
	ra.rptn		= 1;
  return true;
}
//-------------------------------------------------------------------------
//  Check for Goto action
//  Go to nn%
//  0%  => Frame 0
//  100%=> Last frame
//-------------------------------------------------------------------------
bool CheckChapter::RatioAction(char *buf)
{ int p1,p2,rp = 0;
  if (3 != sscanf(buf," Ratio ( %d%%, %d , %d )",&rp,&p1,&p2))  return false;
  ra.actn		= D2R2_CLICK_UPTO_RATIO;
	ra.pm1		= p1;
	ra.pm2		= p2;
	ra.rptn   = rp;
	ra.timr   = 1;
  return true;
}
//-------------------------------------------------------------------------
//  Check for CA action
//  multiple Left Click on CA field (LCCA)
//-------------------------------------------------------------------------
bool CheckChapter::MLFieldAction(char *buf)
{ int rp, ca, tm = 0;
  if (3 != sscanf(buf," %d * L-Click( CA%d , %d )",&rp, &ca, &tm)) return false;
	ra.actn		= D2R2_LEFT_CLICK_CAFLD;
	ra.pm1		= 0;
	ra.pm2		= 0;
	ra.canb		= ca;
	ra.rptn		= 1;
	ra.timr		= tm;
  return true;
}
//-------------------------------------------------------------------------
//  Check for CA action
//  Left Click on CA field (LCCA)
//-------------------------------------------------------------------------
bool CheckChapter::SLFieldAction(char *buf)
{ int ca = 0;
  if (1 != sscanf(buf," L-Click( CA%d )",&ca))  return false;
	ra.actn		= D2R2_LEFT_CLICK_CAFLD;
	ra.pm1		= 0;
	ra.pm2		= 0;
	ra.canb		= ca;
	ra.timr		= 10;
	ra.rptn		= 1;
  return true;
}
//-------------------------------------------------------------------------
//  Check for CA action
//  multiple Left Click on CA field (LCCA)
//-------------------------------------------------------------------------
bool CheckChapter::MRFieldAction(char *buf)
{ int rp, ca, tm = 0;
  if (3 != sscanf(buf," %d * R-Click( CA%d , %d )",&rp, &ca, &tm)) return false;
	ra.actn		= D2R2_RITE_CLICK_CAFLD;
	ra.pm1		= 0;
	ra.pm2		= 0;
	ra.canb		= ca;
	ra.rptn		= rp;
	ra.timr		= tm;
  return true;
}
//-------------------------------------------------------------------------
//  Check for CA action
//  Right Click on CA field (RCCA)
//-------------------------------------------------------------------------
bool CheckChapter::SRFieldAction(char *buf)
{ int ca = 0;
  if (1 != sscanf(buf," R-Click ( CA%d )",&ca)) return false;
	ra.actn	= D2R2_RITE_CLICK_CAFLD;
	ra.pm1	= 0;
	ra.pm2	= 0;
	ra.canb	= ca;
	ra.timr	= 10;
	ra.rptn	= 1;
  return true;
}

//-------------------------------------------------------------------------
//  Check for show action
//-------------------------------------------------------------------------
bool CheckChapter::ShowAction(char *buf)
{ int tm;
  if (1 != sscanf(buf,"showGauge (%d) ",&tm)) return false;
	ra.actn		= D2R2_SHOW_FOCUS_GAUGE;
	ra.pm1		= 0;
	ra.pm2		= 0;
	ra.timr		= tm;
	ra.rptn		= 1;
	flag		= (D2R2_NEED_PANL + D2R2_NEED_GAGE);
  return true;
}
//-------------------------------------------------------------------------
//  Check for show panel
//-------------------------------------------------------------------------
bool CheckChapter::PanlAction(char *buf)
{ char act[128];
  if (1 != sscanf(buf," %s",act))             return false;
  if (strncmp(act,"showPanel",9))             return false;
	ra.actn	= D2R2_SHOW_FOCUS_PANEL;
	ra.pm1	= 0;
	ra.pm2	= 0;
	ra.timr	= 1;
	ra.rptn	= 1;
	flag	= (D2R2_NEED_PANL);
  return true;
}
//-------------------------------------------------------------------------
//  Decode action
//-------------------------------------------------------------------------
void CheckChapter::DecodeAction(SStream *st)
{ char buf[128];
  ReadString(buf,128,st);
	ra.user	= pLIN;
  if (SLClikAction(buf))    return;     // Decode single left click
  if (MLClikAction(buf))    return;     // Decode multiple left click
  if (SRClikAction(buf))    return;     // Decode single right click
  if (MRClikAction(buf))    return;     // Decode multiple right click

  if (RatioAction(buf))     return;
  if (SLFieldAction(buf))   return;     // Decode single left click on CA field
  if (MLFieldAction(buf))   return;     // Decode multiple left click on CA field

  if (SRFieldAction(buf))   return;     // Single right click on CA field
  if (MRFieldAction(buf))   return;     // multiple right click on CA field

  if (ShowAction(buf))			return;
  if (PanlAction(buf))			return;
  return;
}
//-------------------------------------------------------------------------
//  Decode Gauge and Click area
//-------------------------------------------------------------------------
void CheckChapter::DecodeGauge(char *buf)
{ Tag  idn = 0;
  int  ca;
  char txt[256];
  //----Try gauge tag with CA number --------
  if (2 == sscanf(buf," %4s - CA%d",txt,&ca))
  { idn = StringToTag(txt);
		ra.ggt		= idn;
    ra.canb		= ca;
  }
  else
  //----Just the gauge tag ------------------
  if (1 == sscanf(buf," %4s ",txt))
  { idn = StringToTag(txt);
		ra.ggt	= idn;
  }
  return;
}
//-------------------------------------------------------------------------
//  Decode condition
//-------------------------------------------------------------------------
void CheckChapter::DecodeCondition(char *txt)
{	ROBOT_COND *cnd = 0;
  int val;
  for (cnd=rcondTAB; cnd->txt!=0; cnd++)
	{	int nf = sscanf(txt,cnd->txt,&val);
		if (nf != 1)	continue;
		ra.vtst	= val;
		ra.cond = cnd->code;
		return;
	}
	return;
}
//=====================================================================================
//  CHECK LIST  CHAPTER
//=====================================================================================
int CheckChapter::Read(SStream *st,Tag tag)
{ CChkLine *lin = 0;
  Tag pm;
  char line[256];
  switch (tag)  {

  case 'line':
    GenerateLines(st);
    return TAG_READ;
  case 'panl':
    ReadTag(&pm,st);           // Read panel id
		ra.pnt	= pm;
    ReadString(line,256,st);   // Read gauge id
    DecodeGauge(line);
    return TAG_READ;
	//--- Condition statement ------------------------
	case'cond':
		ReadString(line,256,st);
		DecodeCondition(line);
		return TAG_READ;
	//--- end autostatr statement --------------------
	case 'ends':									// End of autostart
		flag	|= D2R2_END_START;		// Mark action
		return TAG_READ;
	//--- Execute statement --------------------------
  case 'exec':
    if (0 == pLIN) gtfo("CheckList: Execute order without line");
    DecodeAction(st);
		if (pLIN)	pLIN->StoreAction(ra,flag);
		ra.Clear();
		pLIN	= 0;
		flag	= 0;
    return TAG_READ;

  }
  return TAG_IGNORED;
}
//-------------------------------------------------------------------------
//  Delete chapter
//-------------------------------------------------------------------------
CheckChapter::~CheckChapter()
{ std::vector<CChkLine*>::iterator ln;
  for (ln = vLIN.begin(); ln != vLIN.end(); ln++)
  { CChkLine *line = (*ln);
    delete line;
  }
  vLIN.clear();
}

//-------------------------------------------------------------------------
//  Fill the box with the lines from this chapter
//-------------------------------------------------------------------------
void CheckChapter::GetLines(CListBox &box)
{ int dim = vLIN.size();
  for (int k=0; k < dim; k++)
  { CChkLine *lin = vLIN[k];
    box.AddSlot(lin);
  }
  return;
}
//-------------------------------------------------------------------------
//  Generate a new node 
//-------------------------------------------------------------------------
void CheckChapter::CreateNode(SStream *st,int No)
{ CChkLine *nod = new CChkLine(No);
  char      dst[1024];
  ReadString(dst,1023,st);
  //----Add text  ---------------------
  nod->FixeIt();
  nod->SetName(dst);
  nod->SetTotLine(nbl);
  vLIN.push_back(nod);
  if (0 == No) pLIN = nod;
  return;
}
//-------------------------------------------------------------------------
//  Generate line for display 
//-------------------------------------------------------------------------
void CheckChapter::GenerateLines(SStream *st)
{ ra.Clear();
	//----Create primary slot ---------------------
  int nbr = 0;
  int No  = 0;
  ReadInt(&nbr,st);
  nbl     = nbr;
  while (nbr--)  CreateNode(st,No++);
	flag  = D2R2_CHECK_LST;
	return;
}
//=====================================================================================
//  AIRCRAFT CHECK LIST
//  AUTOSTART will engage the whole procedure to start engine
//=====================================================================================
PlaneCheckList::PlaneCheckList(CVehicleObject *v)
{ hwId = HW_OTHER;
  unId = 'chkl';
	mveh = v;
  cWIN = 0;
	d2r2 = mveh->GetRobot();
}
//-------------------------------------------------------------------------
PlaneCheckList::~PlaneCheckList()
{}
//-------------------------------------------------------------------------
//  Decode Check list
//-------------------------------------------------------------------------
void PlaneCheckList::OpenList(char *tail)
{ char name[PATH_MAX];
  sprintf(name,"WORLD/VEH_%s.LCK",tail);
  if (!pexists(&globals->pfs,name)) return;
  SStream s;
  strncpy (s.filename,name, PATH_MAX);
  strcpy (s.mode, "r");
  if (OpenStream (&s))
  { ReadFrom (this, &s);                 
    CloseStream (&s);
  }
  return;
}
//-------------------------------------------------------------------------
//  Read chapters
//-------------------------------------------------------------------------
int PlaneCheckList::Read(SStream *st,Tag tag)
{ switch (tag)  {
    case 'chap':
      CheckChapter *chap = new CheckChapter();
      char *dst = chap->GetName();
      int   dim = chap->GetSize();
      ReadString(dst,dim,st);
      dst[dim-1] = 0;
      ReadFrom(chap,st);
      vCHAP.push_back(chap);
      return TAG_READ;
  }
  return TAG_IGNORED;
}
//--------------------------------------------------------------------------
//  Return a list of Pointers to chapter
//--------------------------------------------------------------------------
char **PlaneCheckList::GetChapters()
{ int k;
  int nchap = vCHAP.size();
  if (0 == nchap)   return 0;
  char **tab = new char*[nchap+1];
  for (k=0; k<nchap; k++)
  { CheckChapter *chp = vCHAP[k];
    tab[k] = chp->GetName();
  }
  tab[k]  = 0;
  return tab;
}
//-------------------------------------------------------------------------
//  Close check list
//-------------------------------------------------------------------------
void PlaneCheckList::Close()
{ if (cWIN) cWIN->Close(); 
  std::vector<CheckChapter*>::iterator ch;
   for (ch = vCHAP.begin(); ch != vCHAP.end(); ch++)
   {  CheckChapter* chap = (*ch);
      delete chap;
   }
   vCHAP.clear();
}
//-------------------------------------------------------------------------
//  Register the managing window
//-------------------------------------------------------------------------
void PlaneCheckList::RegisterWindow(CFuiCkList *w)
{	cWIN	= w;
	if (d2r2) d2r2->Register(w);
	return;
}
//-------------------------------------------------------------------------
//  Execute one statemnt
//-------------------------------------------------------------------------
bool PlaneCheckList::Execute(D2R2_ACTION &a)
{	return (d2r2)?(d2r2->Execute(a)):(false);}
//--------------------------------------------------------------------------
//  Fill the box with lines from chapter No
//--------------------------------------------------------------------------
void PlaneCheckList::GetLines(CListBox &box,U_INT No)
{ if (No >= vCHAP.size())  return;
  CheckChapter *chap = vCHAP[No];
  chap->GetLines(box);
  return;
}
//--------------------------------------------------------------------------
//  Receive a message
//--------------------------------------------------------------------------
EMessageResult PlaneCheckList::ReceiveMessage (SMessage *msg)
{ if (!MsgForMe (msg)) return MSG_IGNORED;
  msg->receiver = this;
  if (msg->user.u.datatag == 'show')  
  { globals->fui->ToggleFuiWindow (FUI_WINDOW_CHECKLIST);
    return MSG_PROCESSED;
  }
  return MSG_IGNORED;}
//=====================================================================================
//   WINDOW for AIRCRAFT CHECK LIST
//=====================================================================================
CFuiCkList::CFuiCkList(Tag idn, const char *filename)
:CFuiWindow(idn,filename,0,0,0)
{ char err[128];
  sprintf(err,"Incorrect TEMPLATE file: %",filename);
  //---------------------------------------------------------
  nSEL        = -1;
  mPOP.aText  = 0;
  panl				= 0;
	slot				= 0;
  //---Locate components ------------------------------------
  uBUT = (CFuiButton *)    GetComponent('unck');     // Uncheck button
  if (0 == uBUT)    gtfo(err);
  wBUT = (CFuiButton *)    GetComponent('wher');     // Locate button
  if (0 == uBUT)    gtfo(err);
  cPOP = (CFuiPopupMenu*)  GetComponent('chap');
  if (0 == cPOP)    gtfo(err);
  //---Request notificationon stop click ---------------------
  wBUT->NotifyOnStop();
  //---Init list box interface -------------------------------
  U_INT type = LIST_DONT_FREE + LIST_NOHSCROLL + LIST_USE_MARKS;
  iBOX.SetParameters(this,'list',type);
	//--- Get related objects to work together -----------------
  LST = globals->pln->ckl;
  if (LST->HasChapter())  BuildMenu();
  //---Register for closing ----------------------------------
  LST->RegisterWindow(this);
};
//-------------------------------------------------------------------------
//  Release resources
//-------------------------------------------------------------------------
CFuiCkList::~CFuiCkList()
{ if (mPOP.aText) delete [] mPOP.aText;
  LST->RegisterWindow(0);
  if (panl)		panl->ClearFocus();
}
//-------------------------------------------------------------------------
//  Build Menu
//  -Request a list of chapters as the popup
//  -Select the first chapter
//-------------------------------------------------------------------------
void CFuiCkList::BuildMenu()
{ char **tab = LST->GetChapters();
  cPOP->CreatePage(&mPOP,tab);
  NewChapter(0);
  return;
}
//-------------------------------------------------------------------------
//  Select a new chapter
//  -Change Pop text
//  -Change list of items (dont delete line by EmptyIt(). Lines
//   belong to Chapters
//-------------------------------------------------------------------------
void CFuiCkList::NewChapter(int No)
{ cPOP->SetButtonText((char*)mPOP.aText[No]);
  if (nSEL == No) return;
  nSEL  = No;
  //---Empty the list box and request a new list -----
  iBOX.Clear();
  LST->GetLines(iBOX,No);
  iBOX.Display();
  return;
}
//-------------------------------------------------------------------------
//  NOTIFICATIONS FROM POPUP
//-------------------------------------------------------------------------
void CFuiCkList::NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn)
{ switch (id) {
    case 'chap':
      if (evn == EVENT_POP_CLICK)  NewChapter((U_INT)itm);
      return;

    }
  return;
}
//-------------------------------------------------------------------------
//  Check all preconditions and select panel items
//-------------------------------------------------------------------------
bool CFuiCkList::Verify()
{ slot = (CChkLine*)iBOX.GetSelectedSlot();
  if (0 == slot)											return false;
  D2R2_ACTION &a	=  slot->Action();
	U_INT     val		=  slot->GetVLID();
  //---Locate the panel -----------------------
	panl  = globals->pit->GetPanelByTag(a.pnt);
	gage	= (panl)?(panl->GetGauge(a.ggt)):(0);
	a.panl	= panl;
	a.gage	= gage;
  //--- Check for cockpit camera -----------------
  cam = globals->ccm->SelectCamera(CAMERA_COCKPIT);
	bool	ct	= cam->IsOf(CAMERA_COCKPIT);
	U_INT cp	= val & D2R2_NEED_CPIT;
  if (cp && !ct)											return false;
	//--- Check if panel needed --------------------
	U_INT np	= val & D2R2_NEED_PANL;
	if (np && !panl)										return false;
  //--- Activate the panel -----------------------
  CCameraCockpit* cap = (CCameraCockpit*)cam;
  cap->ActivateCockpitPanel(a.pnt);
  if (a.actn == D2R2_SHOW_FOCUS_PANEL)return true;
		//--- Check for gauge needed -------------------
	U_INT ng	= val & D2R2_NEED_GAGE;
	if (ng && !gage)										return false;
  panl->FocusOnGauge(gage);
  //--- Check for focus to Click area ----------
  if (a.canb)  panl->FocusOnClick(gage,a.canb);
  return true;
}
//-------------------------------------------------------------------------
//  NOTIFICATIONS FROM POPUP:  Locate panel and gauge and draw tour
//-------------------------------------------------------------------------
void CFuiCkList::LocateGage()
{ if (!Verify())	return;
  panl->FocusOnGauge(tgag);
  panl->FocusOnClick(gage,ca);
  return;
}

//-------------------------------------------------------------------------
//  NOTIFICATIONS FROM POPUP:  
//  Locate panel and gauge and  Execute action
//-------------------------------------------------------------------------
void CFuiCkList::Execute()
{ if (!Verify())				return;
  if (0  == actn)       return;
  LST->Execute(slot->Action());
  return;
}
//-------------------------------------------------------------------------
//  Mark the slot
//-------------------------------------------------------------------------
void CFuiCkList::EndExecute(void *up)
{ if (0 == up)	return;
	CChkLine *lin  =(CChkLine*)up;
  lin->SetMark();
  iBOX.Refresh();
  return;
}
//-------------------------------------------------------------------------
//  Stop blinking gauge
//-------------------------------------------------------------------------
void CFuiCkList::StopBlink()
{ if (panl)  panl->ClearFocus();
  panl = 0;
  return;
}
//-------------------------------------------------------------------------
//  Default notification
//-------------------------------------------------------------------------
void  CFuiCkList::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ if (idm == 'sysb') SystemHandler(evn);
  switch (idm)  {
    case 'list':
      iBOX.VScrollHandler((U_INT)itm,evn);
      return;
    case 'done':
      { EndExecute(slot);
        return;
      }

    case 'unck':
      iBOX.Apply(UnMarkCheckList);
      return;

    case 'wher':
      if (evn == EVENT_BUTTONPRESSED) LocateGage();
      if (evn == EVENT_BUTTONRELEASE) StopBlink();
      return;

    case 'exec':
      if (evn == EVENT_BUTTONPRESSED) Execute();
      return;
  }
  return;
}

//=======================END OF FILE ==================================================