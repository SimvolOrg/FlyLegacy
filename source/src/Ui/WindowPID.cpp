/*
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005 Chris Wallace
 * copyright 2009 Jean Sabatier
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

/*! \file WindowFrameRate
 *  \brief Implements FUI callback for frame rate window
 *
 */

#include "../Include/FlyLegacy.h"
#include "../Include/Fui.h"
#include "../Include/FuiParts.h"
#include "../Include/FuiUser.h"
#include "../Include/Autopilot.h"
//===========================================================================
//  WINDOW TO TUNE PID CONTROLLER FOR AUTOPILOT
//===========================================================================
CFuiPID::CFuiPID(Tag idn, const char *filename)
:CFuiWindow(idn,filename,440,210,0)
{ strcpy(text,"PID TUNING");
  title = 1;
  close = 1;
  zoom  = 0;
  mini  = 0;
  sPID  = 0;
  Inc   = 0.10;
  //--------Create label ------------------------------
  wLAB  = new CFuiLabel (6,10,130,20);
  AddChild('lbl1',wLAB,"PID CONTROLLERS");
  //--------Create the PID list -----------------------
  wPID  = new CFuiList  (  6,40,130,110,this);
  wPID->SetVScroll();
  AddChild('lpid',wPID);
  //--------Create the Parameter list -----------------
  wPRM  = new CFuiList  (146,40,270,110,this);
  AddChild('lprm',wPRM);
  //--------Create inc label --------------------------
  wLB1  = new CFuiLabel (  6,160,130,20);
  AddChild('lbl2',wLB1,"Current increment");
  //--------Create Component label --------------------
  wLBP  = new CFuiLabel ( 190,160,270,20);
  AddChild('lbpm',wLBP,"");
  //--------Create plus Inc button -------------------
  pINC  = new CFuiButton   (  6,180,36,20,this);
  AddChild('pinc',pINC,"<<");
  //--------Create edit window ------------------------
  wINC  = new CFuiTextField( 50,180,40,20,this);
  AddChild('incr',wINC);
  //--------Create minus Inc button -------------------
  mINC  = new CFuiButton   (100,180,36,20,this);
  AddChild('minc',mINC,">>");
  //--------Create minus Val button -------------------
  mVAL  = new CFuiButton   (190,180,56,20,this);
  AddChild('mval',mVAL,"-");
  //--------Create parameter edit windows -------------
  wVAL  = new CFuiTextField(254,180,44,20,this);
  AddChild('wval',wVAL," ");
  //--------Create plus Val button -------------------
  pVAL  = new CFuiButton   (306,180,56,20,this);
  AddChild('pval',pVAL,"+");
  //--------Display subsystems ------------------------
  CFuiWindow::ReadFinished();
  pBox.SetParameters(this,'lpid',0);
  mBox.SetParameters(this,'lprm',0);
  AutoPilot   *aps = globals->pln->GetAutoPilot();
  if (aps) aps->GetAllPID(this);
  pBox.SortAndDisplay();
  SelectPID();
  EditInc();
  MoveTo(200,200);
}
//-----------------------------------------------------------
//  Edit Increment
//-----------------------------------------------------------
void CFuiPID::EditInc()
{ wINC->EditText("%.05f",Inc);
  return;
}
//-----------------------------------------------------------
//  Add a PID controller to list
//-----------------------------------------------------------
void CFuiPID::AddPID(CPIDbox *pid)
{ char  eidn[8];
  CPidLine *slot = new CPidLine;
  slot->SetPID(pid);
  TagToString(eidn,pid->GetUnId());
  slot->SetName(eidn);
  pBox.AddSlot(slot);
}
//-----------------------------------------------------------
//  Increment the increment
//-----------------------------------------------------------
void CFuiPID::Increment()
{ if (Inc < 10.1)
  { Inc *= 10;
    EditInc();
  }
  return;
}
//-----------------------------------------------------------
//  Decrement the increment
//-----------------------------------------------------------
void CFuiPID::Decrement()
{ if (Inc < 0.0001)  return;
  Inc *= 0.1;
  EditInc();
  return;
}
//-----------------------------------------------------------
//  Modify the parameter
//-----------------------------------------------------------
void CFuiPID::ChangePRM(double fac)
{ Val += Inc * fac;
  wVAL->EditText("%.05f",Val);
  sPID->SetValue(Prm,Val);
  mBox.Refresh();
  return;
}
//-----------------------------------------------------------
//  DRAW : call selected PID for refreshing
//-----------------------------------------------------------
void CFuiPID::Draw()
{ CFuiWindow::Draw();
  return;
}
//-----------------------------------------------------------------------
//  Fill the parameters with the PID
//-----------------------------------------------------------------------
void CFuiPID::SelectPID()
{ CPidLine *sln = (CPidLine*)pBox.GetSelectedSlot();
  if (0 == sln)     return;
  CPIDbox  *box = sln->GetPID();
  CPipLine *lin = 0;
  if (box == sPID)  return;
  mBox.EmptyIt();
  sPID  = box;
  //-------------------------------------------------------
  lin   = new CPipLine;
  lin->SetName("<KP__> Proportional coefficient:");
  lin->SetPRM('Kp__');
  lin->SetPID(box);
  mBox.AddSlot(lin);
  //-------------------------------------------------------
  lin   = new CPipLine;
  lin->SetName("<iTim> Integrator Time:");
  lin->SetPRM('iTim');
  lin->SetPID(box);
  mBox.AddSlot(lin);
  //-------------------------------------------------------
  lin   = new CPipLine;
  lin->SetName("<aTim> Anti Saturator Time:");
  lin->SetPRM('aTim');
  lin->SetPID(box);
  mBox.AddSlot(lin);
  //-------------------------------------------------------
  lin   = new CPipLine;
  lin->SetName("<iMax> Integrator Limit:");
  lin->SetPRM('iMax');
  lin->SetPID(box);
  mBox.AddSlot(lin);
  //-------------------------------------------------------
  lin   = new CPipLine;
  lin->SetName("<dTim> Derivator Time:");
  lin->SetPRM('dTim');
  lin->SetPID(box);
  mBox.AddSlot(lin);
  //-------------------------------------------------------
  lin   = new CPipLine;
  lin->SetName("<vmin> minimum output:");
  lin->SetPRM('vmin');
  lin->SetPID(box);
  mBox.AddSlot(lin);
  //-------------------------------------------------------
  lin   = new CPipLine;
  lin->SetName("<vmax> maximum output:");
  lin->SetPRM('vmax');
  lin->SetPID(box);
  mBox.AddSlot(lin);
  //--------------------------------------------------------
  mBox.Display();
  SelectPRM();
}
//-----------------------------------------------------------------------
//  Select Parameter
//-----------------------------------------------------------------------
void CFuiPID::SelectPRM()
{ CPipLine *sln = (CPipLine*)mBox.GetSelectedSlot();
  if (0 == sln)     return;
  char     *vtx = sln->GetName();
  Tag       prm = sln->GetPRM();
  Val = sPID->GetValue(prm);
  wVAL->EditText("%.05f",Val);
  wLBP->SetText(vtx);
  Prm = prm;
  return;
}
//-----------------------------------------------------------------------
//  Notifications
//-----------------------------------------------------------------------
void CFuiPID::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ switch (idm)  {
  case 'sysb':
    SystemHandler(evn);
    return;
  case 'lpid':
    pBox.VScrollHandler((U_INT)itm,evn);
    SelectPID();
    return;
  case 'lprm':
    mBox.VScrollHandler((U_INT)itm,evn);
    SelectPRM();
    return;
  case 'minc':
    Decrement();
    return;
  case 'pinc':
    Increment();
    return;
  case 'mval':
    ChangePRM(-1);
    return;
  case 'pval':
    ChangePRM(+1);
    return;

}
return;
}
//=================END OF FILE ==============================================


