/*
 * WindowAxis.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005 Chris Wallace
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

/*! \file WindowAxis.cpp
 *  \brief Implements FUI callback for axis window
 *
 *  Implements the FUI callback for the Axis window.
 */

#include "../Include/FlyLegacy.h"
#include "../Include/Fui.h"
#include "../Include/FuiParts.h"
#include "../Include/Joysticks.h"
#include "../Include/WinControlAxis.h"
//=================================================================================
//  AXE MENU
//=================================================================================
char *CFuiAxis::vehMENU[] = {
  "Airplane Axes",
  "Helicopter Axes",
  "Ground Vehicle Axes",
  ""
};
//=================================================================================
//  DEVICE MENU
//=================================================================================
char *CFuiAxis::devMENU[16] = {
  "",
  "",
  "",
  "",
};

//======================================================================================
//  Global CALLBACK WHEN AXE MOVE
//======================================================================================
void  WhenAxeMove(CSimAxe *axe, Tag winID)
{ CFuiAxis *win = (CFuiAxis*)globals->fui->GetFuiWindow(winID);
  win->AxeDetected(axe);
  return;
}
//======================================================================================
//  Global CALLBACK WHEN BUtton is clicked
//======================================================================================
void  WhenButtonClick(SJoyDEF *jsd, int nbt, Tag id)
{ CFuiAxis *win = (CFuiAxis*)globals->fui->GetFuiWindow(id);
  win->ButtonClick(jsd,nbt);
  return;
}

//======================================================================================
//  CFuiAxis to assign Joystick axes
//======================================================================================
CFuiAxis::CFuiAxis(Tag idn, const char *filename)
:CFuiWindow(idn,filename,0,0,0)
{ char err[128];
  sprintf(err,"Incorrect TEMPLATE file: %s",filename);
  all = 1;
  jsd = 0;
	jsp	= 0;
  jsm = globals->jsm;
  //---Locate components -----------------------------
  asgWIN    = (CFuiButton*)   GetComponent('defa');
  if (0 == asgWIN )  gtfo(err);
  labWIN    = (CFuiLabel*)    GetComponent('?lbl');
  if (0 == labWIN )  gtfo(err);
  vehPOP    = (CFuiPopupMenu*)GetComponent('type');
  if (0 == vehPOP )  gtfo(err);
  chkWIN    = (CFuiCheckbox*) GetComponent('alla');
  if (0 == chkWIN )  gtfo(err);
	//--- Group box axes ------------------------------
	gp1AXE		= (CFuiGroupBox*) GetComponent('axes');
  if (0 == gp1AXE)   gtfo(err);
	devAXE		= (CFuiLabel*)		gp1AXE->GetComponent('devn');
	if (0 == devAXE)	gtfo(err);
	labAXE		= (CFuiLabel*)		gp1AXE->GetComponent('axen');
	if (0 == labAXE)	gtfo(err);
	valAXE    = (CFuiSlider*)   gp1AXE->GetComponent('axev');
  if (0 == valAXE)   gtfo(err);
	valAXE->SetProperty(FUI_NO_MOUSE);
  invAXE = (CFuiCheckbox*)gp1AXE->GetComponent('chka');
  if (0 == invAXE)   gtfo(err);
	//--- Group box tune -------------------------------
	grpFOR		= (CFuiGroupBox*) GetComponent('tune');
  if (0 == grpFOR)   gtfo(err);
	minFOR    = (CFuiButton*)   GetComponent('opr-');
  if (0 == minFOR )  gtfo(err);
	plsFOR    = (CFuiButton*)   GetComponent('opr+');
  if (0 == plsFOR )  gtfo(err);
	//--- Neutral box ----------------------------------
	labNEU		= (CFuiLabel*)    GetComponent('labN');
	if (0 == labNEU )  gtfo(err);
	valNEU		= (CFuiSlider*)   GetComponent('neut');
	if (0 == valNEU )  gtfo(err);
	//--- Button control -------------------------------
	devPOP    = (CFuiPopupMenu*)GetComponent('devl');
  if (0 == devPOP )  gtfo(err);
	grpHAT		= (CFuiGroupBox*) GetComponent('hatg');
  if (0 == grpHAT)   gtfo(err);
	chkHAT    = (CFuiCheckbox*)grpHAT->GetComponent('useH');
	if (0 == chkHAT)   gtfo(err);
	//--------------------------------------------------
  axeBOX.SetParameters(this,'list',0,0);
	butBOX.SetParameters(this,'butL',0,0);
  //-----Init vehicle menu ---------------------------
	jsm->CreateDevList(devMENU,16);
	ShowNeutral();
  chkWIN->SetState(all);
  vehPOP->CreatePage(&vehMEN,vehMENU);
	devPOP->CreatePage(&devMEN,devMENU);
  SelectVehicle(0);
  FillAxes(0);
	AxeSelect(0);
	ButtonList(devMENU[0]);
	HatControl();
  //----Allow for Joystick detection ------------------
	if (jsm->IsBusy()) Close();
	else	{	jsm->StartDetectMoves(WhenAxeMove,windowId);
					jsm->StartDetectButton(WhenButtonClick,windowId);
				}
}
//---------------------------------------------------------------------------
//  Window is closing
//---------------------------------------------------------------------------
CFuiAxis::~CFuiAxis()
{ jsm->SaveConfiguration();
	jsm->SetFree();
}
//---------------------------------------------------------------------------
//  Select vehicle type
//---------------------------------------------------------------------------
void CFuiAxis::SelectVehicle(int No)
{ vehNo  = No;
  vehPOP->SetButtonText((char*)vehMEN.aText[No]);
  return;
}
//--------------------------------------------------------------------------
//  Change vehicle axis
//--------------------------------------------------------------------------
void CFuiAxis::NewVehicleType(U_INT No)
{ vehNo  = No;
  vehPOP->SetButtonText((char*)vehMEN.aText[No]);
  FillAxes(No);
	AxeSelect(0);
  return;
}

//--------------------------------------------------------------------------
//  Display list of Axes
//--------------------------------------------------------------------------
void CFuiAxis::FillAxes(int tp)
{ CSimAxe   *axe = 0;
  CAxeLine  *xln = 0;
  axeBOX.EmptyIt();
  for (axe = jsm->NextAxe(0,tp); axe != 0; axe = jsm->NextAxe(axe,tp))
  { xln = new CAxeLine;
    xln->SetSlotName(axe->GetName());
    xln->SetAxeDesc(axe);
    axeBOX.AddSlot(xln);
  }
  axeBOX.Display();
  labWIN->SetText("Move JOYSTICK on Selection, then Assign");
  return;
}
//--------------------------------------------------------------------------
//  Axe is detected on current selection
//--------------------------------------------------------------------------
void CFuiAxis::AxeDetected(CSimAxe *nax)
{ jsd    = nax->pJoy;
  axeNo  = nax->iAxe;
  labWIN->EditText("(J%d): axe %02d IS DETECTED",jsd->JoystickNo(),axeNo);
  return;
}
//--------------------------------------------------------------------------
//  Assign the current  axe to selected component
//--------------------------------------------------------------------------
void CFuiAxis::AxeAssign()
{ CSimAxe axn;
  if (0 == jsd)   return;
  axn.pJoy = jsd;
  axn.iAxe = axeNo;
  jsm->AssignAxe(axe,&axn,all);
  jsd      = 0;
	labWIN->SetText("Move JOYSTICK on Selection, then Assign");
	axeBOX.Refresh();
  return;
}
//--------------------------------------------------------------------------
//  Clear Axe from the selected component
//--------------------------------------------------------------------------
void CFuiAxis::AxeClear()
{ jsm->Clear(axe,all);
	axeBOX.Refresh();
  return;
}
//--------------------------------------------------------------------------
//  Axe selection
//--------------------------------------------------------------------------
void CFuiAxis::AxeSelect(int No)
{	CAxeLine  *xln = (CAxeLine*)axeBOX.GetSelectedSlot();
  axe = xln->GetAxeDesc();
	//--- edit data on group box ------------------------
	char *devn	= axe->GetDevice();
	devAXE->SetText(devn);
	invAXE->SetState(axe->GetInvert());
	ShowForce();
	return;
}
//--------------------------------------------------------------------------
//  Display neutral AREA
//--------------------------------------------------------------------------
void  CFuiAxis::ShowNeutral()
{ char txt[64];
  float n = jsm->GetNulleArea();
  valNEU->SetValue(n);
	sprintf(txt,"DEFINE NEUTRAL AREA: %0.2f",n);
  labNEU->SetText(txt);
  return;
}
//--------------------------------------------------------------------------
//  Display attenuation
//--------------------------------------------------------------------------
void CFuiAxis::ShowForce()
{ char txt[8];
  sprintf(txt,"%.02f",axe->GetATTN());
  grpFOR->SetChildText('frcv',txt);
  return;
}

//--------------------------------------------------------------------------
//  Axe inversion
//--------------------------------------------------------------------------
void CFuiAxis::AxeInvert(Tag s)
{	int st = int(s);
	jsm->Invert(axe,all);
	return;
}
//--------------------------------------------------------------------------
//  Adjust attenuation
//--------------------------------------------------------------------------
void CFuiAxis::AxeForce(float inc)
{ float val = axe->GetATTN();
  val += inc;
  if (val >  1)  val = 1;
  if (val <  0)  val = 0;
  axe->SetATTN(val);
  ShowForce();
	jsm->Modifier();
  return;
}
//--------------------------------------------------------------------------
//  Change Neutral area
//--------------------------------------------------------------------------
void CFuiAxis::AxeNeutral()
{ char txt[64];
  float n = valNEU->GetValue();
	sprintf(txt,"DEFINE NEUTRAL AREA: %0.2f",n);
  labNEU->SetText(txt);
  jsm->SetNulleArea(n,1);
  return;
}
//----------------------------------------------------------------------
//  Init the button list for the requested joystick
//----------------------------------------------------------------------
void CFuiAxis::ButtonList(char *name)
{ jsp = jsm->Find(name);
  if (0 == jsp)     return;
  devPOP->SetButtonText(name);
  butBOX.EmptyIt();
  //-----Locate joystick descriptor
	int       end = jsp->nbt;
  char      txt[128];
  char     *ktx = "";
  CButLine *btl = 0;
  for (int k=0; k<end; k++)
  { CSimButton     *but = jsp->GetButton(k);
    CKeyDefinition *kdf = (but)?(but->GetKey()):(0);
    ktx = (kdf)?(kdf->GetName()):("");
    sprintf(txt,"Button %02d",k);
    btl = new CButLine;
    btl->SetSlotName(txt);
    btl->SetKeyText(ktx);
    butBOX.AddSlot(btl);
  }
  butBOX.Display();
	//--- Set Hat control ---------------------------
	sprintf(txt,"This device has %d HAT",jsp->hNumber());
	SetChildText('labH',txt);
	char	use = jsp->HatUsed();
	chkHAT->SetState(use);
	HatControl();
  return;
}
//----------------------------------------------------------------------
//  Set hat control 
//----------------------------------------------------------------------
void CFuiAxis::HatControl()
{	bool nok = ((0 == jsp) || (0 == jsp->hNumber())); 
	if (nok)	chkHAT->Hide();
	else			chkHAT->Show();
}
//----------------------------------------------------------------------
//  A button is clicked
//----------------------------------------------------------------------
void CFuiAxis::ButtonClick(SJoyDEF *jsn, int nbut)
{	if (jsp != jsn)	ButtonList(jsn->getDevName());
	butBOX.GoToItem(nbut);
}
//--------------------------------------------------------------------------
//  Draw the window 
//--------------------------------------------------------------------------
void CFuiAxis::Draw()
{ char edt[128];
	float valr	= jsm->RawVal(axe);
	float valn  = jsm->AxeVal(axe);
  char *axen	= axe->GetName();
  valAXE->SetValue(valr);
	sprintf(edt,"%s: %.02f",axen,valn);
  labAXE->SetText(edt);
	CFuiWindow::Draw();
	return;
}
//--------------------------------------------------------------------------
//  Notification from Popup
//--------------------------------------------------------------------------
void CFuiAxis::NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn)
{	int pm = (int)itm; 
	switch (id) {
    case 'type':
      if (evn == EVENT_POP_CLICK)  NewVehicleType(pm);
      return;
    case 'devl':
      if (evn == EVENT_POP_CLICK)  ButtonList(devMENU[pm]);
      return;
}
return;
}
//-------------------------------------------------------------------------
//  EVENT notifications from child windows
//-------------------------------------------------------------------------
void  CFuiAxis::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ if (idm == 'sysb')  {SystemHandler(evn); return;}
  switch (idm)  {
      //---List notification --------------------------
      case  'list':
        axeBOX.VScrollHandler((U_INT)itm,evn);
				AxeSelect(itm);
        return;
      //---Change all axes versus short list ----------
      case 'alla':
        all = (U_CHAR)itm;
        return;
      //---Assign the current axe selected ------------
      case 'defa':
        AxeAssign();
        return;
      //---Clear axe from the selected component ------
      case 'clra':
        AxeClear();
        return;
			//--- Invert action -----------------------------
			case 'chka':
				AxeInvert(itm);
				return;
			//--- Increment force ---------------------------
			case 'opr+':
				AxeForce(+0.01f);
				return;
			//--- Decrement force ---------------------------
			case 'opr-':
				AxeForce(-0.01f);
				return;
			//--- Change neutral value ----------------------
      case 'neut':
        AxeNeutral();
        return;
			//--- Change Hat usage --------------------------
			case 'useH':
				jsm->UseHat(jsp,char(itm));
				return;
  }
  return ;
}

//=============END OF FILE ==============================================================
