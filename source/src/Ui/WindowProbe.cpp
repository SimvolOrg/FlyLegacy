/*
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005 Chris Wallace
 * Copyright 2009 Jean Sabatier
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
#include "../Include/FuiProbe.h"
#include "../Include/Weather.h"
#include "../Include/TerrainCache.h"
#include "../Include/TerrainTexture.h"
//=======================================================================================
#define DEB_POS (200)
#define MID_POS (350)
//---------------------------------------------------------------------------------------
char *prob_OP1[] = {"Base",0};
char *prob_OP2[] = {"Base",0};
//---------------------------------------------------------------------------------------
//=======================================================================================
//  CFuiProb:  Display susbsystem property
//=======================================================================================
CFuiProbe::CFuiProbe(Tag idn, const char *filename)
:CFuiWindow(idn,filename,500,600,0)
{ strncpy(text,"SUBSYSTEM PROBE",255);
  //--------Create the subsystem list -----------------------
  wSub  = new CFuiList(20,10,170,380,this);
  wSub->SetVScroll();
  AddChild('lsub',wSub);
  //--------Create First selector ------------------------------
  pob1.btn  = new CFuiButton(DEB_POS,10,130,20,this);
  AddChild('btn1',pob1.btn,"Subsystem 1");
  //--------Create the first canvas -------------------------
  pob1.wCn  = new CFuiCanva (DEB_POS,40,130,200,this);
  AddChild('wcn1',pob1.wCn);
  //--------Create the first dpent button --------------------
  pob1.dpn  = new CFuiButton(DEB_POS,250,130,20,this);
  AddChild('bdp1',pob1.dpn,"Dependents");
  pob1.sub  = 0;
  //--------Create the first dependent list -----------------
  pob1.wsy  = new CFuiList(DEB_POS,280,130,70,this);
  pob1.wsy->SetVScroll();
  AddChild('dpn1',pob1.wsy);
  pob1.box.SetParameters(this,'dpn1',0);
  //--------Create the first popup option menu -------------------
  pob1.pop  = new CFuiPopupMenu(DEB_POS,354,130,30,this);
  AddChild('pop1',pob1.pop,"Options");
  pob1.lst  = prob_OP1;
  //--------Create second selector  ------------------------------
  pob2.btn  = new CFuiButton(MID_POS,10,130,20,this);
  AddChild('btn2',pob2.btn,"Subsystem 2");
  //-------Create the second canva --------------------------
  pob2.wCn  = new CFuiCanva(MID_POS,40,130,200,this);
  AddChild('wcn2',pob2.wCn);
  //--------Create the second button -------------------------
  pob2.dpn  = new CFuiButton(MID_POS,250,130,20,this);
  AddChild('bdp2',pob2.dpn,"Dependents");
  pob2.sub  = 0;
  //--------Create the second dependent list -----------------
  pob2.wsy  = new CFuiList(MID_POS,280,130,70,this);
  pob2.wsy->SetVScroll();
  AddChild('dpn2',pob2.wsy);
  pob2.box.SetParameters(this,'dpn2',0);
  //--------Create the second popup option menu --------------
  pob2.pop  = new CFuiPopupMenu(MID_POS,354,130,30,this);
  AddChild('pop2',pob2.pop,"Options");
  pob2.lst  = prob_OP2;
  //--------Create info canvas ------------------------------
  info  = new CFuiCanva(20,400,460,50,this);
  info->SetId('info');
  info->ReadFinished();
  childList.push_back(info);
  //--------Create metar canvas -----------------------------
  meto  = new CFuiCanva(20,460,460,50,this);
  meto->SetId('meto');
  meto->ReadFinished();
  childList.push_back(meto);
  //-------Create Animator list -----------------------------
  wPID  = new CFuiList(20,520,170,70,this);
  wPID->SetVScroll();
  AddChild('wpid',wPID);
  bPID.SetParameters(this,'wpid',0);
  //--------Create Animator Canva ---------------------------
  cPID  = new CFuiCanva(DEB_POS,520,280,70,this);
  cPID->SetId('cpid');
  cPID->ReadFinished();
  childList.push_back(cPID);
  //--------Display subsystems ------------------------------
  CFuiWindow::ReadFinished();
  veh = globals->pln;
  if (veh)  Init();
  else      Close();
}
//------------------------------------------------------------------------
//  Init lists of subsystems
//------------------------------------------------------------------------
void CFuiProbe::Init()
{ sBox.SetParameters(this,'lsub',0);
  GetAmpSubsystems();
  GetGasSubsystems();
  GetEngSubsystems();
  GetWhlSubsystems();
  AddSubsystem(globals->clk);
  //--------------------------------------------------------
  sBox.SortAndDisplay();
  globals->dBug = 1;
  //------Display PID --------------------------------------
  spid  = 0;
  GetPidSubsystems();
  bPID.SortAndDisplay();
  globals->wpb  = this;
  ReplacePID();
  //-----Init Popup ----------------------------------------
  pob1.pop->CreatePage(&pob1.men,pob1.lst);
  pob2.pop->CreatePage(&pob2.men,pob2.lst);
  pob1.sub = veh->GetNullSubsystem();
  pob2.sub = veh->GetNullSubsystem();
  ViewOption(pob1,0);
  ViewOption(pob2,0);
  return;
}
//------------------------------------------------------------------------
//  Init option text
//------------------------------------------------------------------------
void CFuiProbe::ViewOption(PROBE &p,int k)
{ p.pop->SetButtonText(p.men.aText[k]);
  p.sub->SetOption(k);
}
//------------------------------------------------------------------------
//  Free resources
//------------------------------------------------------------------------
CFuiProbe::~CFuiProbe()
{ globals->wpb = 0;
}
//------------------------------------------------------------------------
//  Get electrical systems
//------------------------------------------------------------------------
void CFuiProbe::GetAmpSubsystems()
{ std::vector<CSubsystem*>amps;         // List of electrical subsystems
  veh->amp.GetAllSystems(amps);
  std::vector<CSubsystem*>::iterator si;
  for (si = amps.begin(); si != amps.end(); si++) AddSubsystem(*si);
  amps.clear();
}
//------------------------------------------------------------------------
//  Get gas systems
//------------------------------------------------------------------------
void CFuiProbe::GetGasSubsystems()
{ std::vector<CFuelSubsystem*>gass;     // List of gas subsystems
  CFuelSystem *gas = veh->GetGAS();
  if (0 == gas)         return;
  gas->GetAllSystems(gass);
  std::vector<CFuelSubsystem*>::iterator si;
  for (si = gass.begin(); si != gass.end(); si++) AddSubsystem(*si);
  gass.clear();
}
//------------------------------------------------------------------------
//  Get pid system
//------------------------------------------------------------------------
void CFuiProbe::GetPidSubsystems()
{ std::vector<CPIDbox *>pids;           // List of list subsystems
  AutoPilot   *aps = veh->GetAutoPilot();
  if (0 == aps)         return;
  aps->GetAllSubsystems(pids);
  std::vector<CPIDbox*>::iterator si;
  for (si = pids.begin(); si != pids.end(); si++) AddPid(*si);
  pids.clear();
  return;
}
//------------------------------------------------------------------------
//  Get engines system
//------------------------------------------------------------------------
void CFuiProbe::GetEngSubsystems()
{ std::vector<CEngine *>engs;           // List of engines
  veh->eng.GetAllEngines(engs);
  std::vector<CEngine*>::iterator si;
  for (si = engs.begin(); si != engs.end(); si++)
  { CEngine   *eng = (*si);
    AddSubsystem(eng);
    CDependent *dp1 = eng->GetPart(1);
    if (dp1) AddSubsystem(dp1);
    CDependent *dp2 = eng->GetPart(2);
    if (dp2) AddSubsystem(dp2);
  }
  engs.clear();
  return;
}
//------------------------------------------------------------------------
//  Get wheel system
//------------------------------------------------------------------------
void CFuiProbe::GetWhlSubsystems()
{ std::vector<CSuspension*>whls;              // List of wheels
  veh->whl.GetAllWheels(whls);
  std::vector<CSuspension*>::iterator wh;
  for (wh = whls.begin(); wh != whls.end(); wh++) AddSubsystem(*wh);
  whls.clear();
  return;
}
//------------------------------------------------------------------------
//  Add a new subsystem
//------------------------------------------------------------------------
void CFuiProbe::AddSubsystem(CSubsystem *obj)
{ CSubLine *slot = new CSubLine;
  char *idn      = obj->GetIdString();
  slot->SetSlotName(idn);
  slot->SetSubsystem(obj);
  slot->SetIden(obj->GetUnId());
  slot->SetType(obj->GetTyString());
  sBox.AddSlot(slot);
  return;
}
//------------------------------------------------------------------------
//  Add a new PID
//------------------------------------------------------------------------
void  CFuiProbe::AddPid(CPIDbox *obj)
{ CSubLine *lin = new CSubLine;
  char *idn     = obj->GetIdString();
  lin->SetSlotName(idn);
  lin->SetSubsystem(obj);
  lin->SetIden(obj->GetUnId());
  lin->SetType(obj->GetTyString());
  bPID.AddSlot(lin);
  return;
}
//-----------------------------------------------------------------------
//  Add a new dependent
//-----------------------------------------------------------------------
void CFuiProbe::AddDependent(CDependent *dep)
{ if (0 ==dep)  return;
  CSubLine *slot = new CSubLine;
  char *idn      = dep->GetIdString();
  slot->SetSlotName(idn);
  slot->SetSubsystem(dep);
  slot->SetIden(dep->GetUnId());
  slot->SetType(dep->GetTyString());
  work->AddSlot(slot);
  return;
}
//------------------------------------------------------------------------
//  Select current PID
//------------------------------------------------------------------------
void CFuiProbe::ReplacePID()
{ CSubLine *slot  = (CSubLine*)bPID.GetSelectedSlot();
  if (0 == slot)    return;
  spid            = (CPIDbox *)slot->GetSubsystem();
  return;
}
//-----------------------------------------------------------------------
//  Get the selected subsystem
//-----------------------------------------------------------------------
CSubsystem *CFuiProbe::GetSubsystem(CFuiButton *btn)
{ CSubLine *slot = (CSubLine*)sBox.GetSelectedSlot();
  if (0 == slot)  return 0;
  btn->SetText(slot->GetSlotName());
  return slot->GetSubsystem();
}
//-----------------------------------------------------------------------
//  Get the selected subsystem
//-----------------------------------------------------------------------
void CFuiProbe::ViewSubsystem(CSubsystem *sub,PROBE &pob)
{ char idn[8];
  TagToString(idn,sub->GetMode());
  pob.dpn->EditText("Dependents (%s)",idn);
  pob.sub = sub;
  globals->psys = sub;
  pob.box.EmptyIt();
  work    = &pob.box;
  sub->GetAllDependents(this);
  pob.box.SortAndDisplay();
  sub->SetOption(0);
  //----Get subsystem probe options -------------
  char **popt = sub->GetProbeOptions();
  if (0 == popt)  return ResetOption(pob);
  //----Change menu option ----------------------
  pob.pop->CreatePage(&pob.men,popt);
  return;
}
//-----------------------------------------------------------------------
//  Reset option menu to base default
//-----------------------------------------------------------------------
void CFuiProbe::ResetOption(PROBE &pob)
{ char **txt = pob.men.aText;
  if (txt == pob.lst)   return;
  pob.pop->CreatePage(&pob.men,pob.lst);
  return;
}
//-----------------------------------------------------------------------
//  Replace dependent
//-----------------------------------------------------------------------
void CFuiProbe::ReplaceDependent(PROBE &p1,PROBE &p2)
{ CSubLine *slot  = (CSubLine*)p1.box.GetSelectedSlot();
  if (0 == slot)    return;
  CSubsystem *sub = slot->GetSubsystem();
  p2.btn->SetText(slot->GetSlotName());
  ViewSubsystem(sub,p2);
  return;
}
//-----------------------------------------------------------------------
//  Draw the probe values
//-----------------------------------------------------------------------
void CFuiProbe::Draw()
{ pob1.wCn->BeginPage();
  if (pob1.sub) pob1.sub->Probe(pob1.wCn);
  pob2.wCn->BeginPage();
  if (pob2.sub) pob2.sub->Probe(pob2.wCn);
  info->BeginPage();
  globals->tcm->Probe(info);
  meto->BeginPage();
  globals->wtm->Probe(meto);
  cPID->BeginPage();
  if (spid)     spid->Probe(cPID);
  CFuiWindow::Draw();
  return;
}
//-----------------------------------------------------------------------
//  Close windows
//-----------------------------------------------------------------------
void CFuiProbe::Close()
{ globals->dBug  = 0;
  globals->psys  = 0;
  CFuiWindow::Close();
  return;
}
//-----------------------------------------------------------------------
//  Notifications
//  NOTE:  Subsystem S1 is kept in globals for debug prupose
//-----------------------------------------------------------------------
void  CFuiProbe::NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn)
{ switch (idm)  {
  case 'sysb':
    SystemHandler(evn);
    return;
  case 'lsub':
    sBox.VScrollHandler((U_INT)itm,evn);
    return;
  case 'wpid':
    bPID.VScrollHandler((U_INT)itm,evn);
    ReplacePID();
    return;
  case 'btn1':
    pob1.sub  = GetSubsystem(pob1.btn);
    ViewSubsystem(pob1.sub,pob1);
    return;
  case 'btn2':
    pob2.sub  = GetSubsystem(pob2.btn);
    ViewSubsystem(pob2.sub,pob2);
    return;
  case 'dpn1':
    pob1.box.VScrollHandler((U_INT)itm,evn);
    return;
  case 'dpn2':
    pob2.box.VScrollHandler((U_INT)itm,evn);
    return;

  case 'bdp1':
    ReplaceDependent(pob1,pob2);
    return;
  case 'bdp2':
    ReplaceDependent(pob2,pob1);
}
return;
}
//----------------------------------------------------------------------------------
//  Notification from Popup
//----------------------------------------------------------------------------------
void CFuiProbe::NotifyFromPopup(Tag idm,Tag itm,EFuiEvents evn)
{ if (evn != EVENT_POP_CLICK) return;
  switch(idm)  {
      case 'pop1':
        ViewOption(pob1,itm);
        return;
      case 'pop2':
        ViewOption(pob2,itm);
        return;
  }
return;
}

//=======================================================================================
//  CFuiStat:  Display statistic info
//=======================================================================================
CFuiStat::CFuiStat(Tag idn, const char *filename)
:CFuiWindow(idn,filename,240,400,0)
{ strncpy(text,"SYSTEM STATISTICS",255);
  title = 1;
  close = 1;
  zoom  = 0;
  mini  = 0;
	U_INT wit = MakeRGBA(255,255,255,255);
	SetTransparentMode();
  //--------Create info canvas ------------------------------
  info  = new CFuiCanva(10,10,220,380,this);
	AddChild('info',info,"",FUI_TRANSPARENT,wit);
  //--------Display subsystems ------------------------------
  CFuiWindow::ReadFinished();
  tcm = globals->tcm;
  txw = globals->txw;
  m3d = globals->m3d;
  dbc = globals->dbc;
  pit = globals->pit;
}
//---------------------------------------------------------------
//  Destructor
//---------------------------------------------------------------
CFuiStat::~CFuiStat()
{
}
//---------------------------------------------------------------
//  Draw the data
//---------------------------------------------------------------
void CFuiStat::Draw()
{ info->BeginPage();
  if (tcm)  tcm->GetStats(info);
  if (txw)  txw->GetStats(info);
  if (m3d)  m3d->GetStats(info);
  if (dbc)  dbc->GetStats(info);
  if (pit)  pit->GetStats(info);
  CFuiWindow::Draw();
}
//===========END OF FILE ====================================================================

