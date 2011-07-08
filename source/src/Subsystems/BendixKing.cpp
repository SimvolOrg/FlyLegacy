/*
 * K155radio.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2007       Jean Sabatier
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

#include "../Include/Subsystems.h"
#include "../Include/Globals.h"
#include "../Include/Database.h"
#include "../Include/FuiUser.h"
#include "../Include/BendixKing.h"
#include "../Include/Geomath.h"
using namespace std;
//=====================================================================================
//  CHARACTER MASK (dependant on flash state)
//=====================================================================================
U_CHAR RadioMSK[] = {
  0x00,                     // OFF
  0xFF,                     // ON
  };
//=====================================================================================
//  State function table
//=====================================================================================
CK155radio::StaFN  CK155radio::staTAB[] = {
  0,
  &CK155radio::COMstateNormal,            // 1 COM normal state
  &CK155radio::COMstatePgMode,            // 2 COM programing mode
  &CK155radio::COMstateSelect,            // 3 COM select mode
  &CK155radio::NAVstateNormal,            // 4 NAV normal mode
  &CK155radio::NAVstateCDI,               // 5 NAV CDI mode
  &CK155radio::NAVstateBRG,               // 6 NAV Bearing mode
  &CK155radio::NAVstateRAD,               // 7 NAV Radial mode
  &CK155radio::NAVstateTIM,               // 8 NAV Timer mode
};
//=====================================================================================
//  COM driver event
//=====================================================================================
RADIO_HIT K155_P0[] = {
  {RADIO_BT18,K55EV_POWR_SW},             // POWER Buton
  {0,0},
};
//-------------------------------------------------------------------------
//  Normal state
//-------------------------------------------------------------------------
RADIO_HIT K155_C1[] = {
  {RADIO_CA01,K55EV_ACOM_WP},
  {RADIO_CA02,K55EV_ACOM_FP},
  {RADIO_CA03,K55EV_SCOM_WP},
  {RADIO_CA04,K55EV_SCOM_FP},
  {RADIO_BT20,K55EV_TUNE_CF},           // COM Tune (inner)
  {RADIO_BT21,K55EV_TUNE_CM},           // COM Tune (outter)
  {RADIO_BT22,K55EV_CHAN_BT},           // COM Mode
  {RADIO_BT19,K55EV_TRSF_CM},           // COM Transfer       
  {RADIO_BT18,K55EV_POWR_SW},           // POWER Button
  {0,0},
};

//-------------------------------------------------------------------------
//  Program mode
//-------------------------------------------------------------------------
RADIO_HIT K155_C2[] = {
  {RADIO_CA03,K55EV_SCOM_WP},
  {RADIO_CA04,K55EV_SCOM_FP},
  {RADIO_CA14,K55EV_CHAN_DG},           // Chanel Number
  {RADIO_BT21,K55EV_TUNE_CM},           // COM tune (outter)
  {RADIO_BT22,K55EV_CHAN_BT},           // COM mode
  {RADIO_BT19,K55EV_TRSF_CM},           // COM Transfer
  {RADIO_BT18,K55EV_POWR_SW},           // POWER button      
  {0,0},
};
//-------------------------------------------------------------------------
//  Select mode
//-------------------------------------------------------------------------
RADIO_HIT K155_C3[] = {
  {RADIO_CA14,K55EV_CHAN_DG},           // Chanel number
  {RADIO_BT21,K55EV_TUNE_CM},           // COM tune (outter)
  {RADIO_BT22,K55EV_CHAN_BT},           // COM mode
  {RADIO_BT19,K55EV_TRSF_CM},           // COM Transfer
  {RADIO_BT18,K55EV_POWR_SW},           // POWER button
  {0,0},
};
//=====================================================================================
//  NAV driver event
//=====================================================================================
//  Normal mode
//--------------------------------------------------------------------
RADIO_HIT K155_N1[] =  {
  {RADIO_CA05,K55EV_ANAV_WP},
  {RADIO_CA06,K55EV_ANAV_FP},
  {RADIO_CA07,K55EV_SNAV_WP},
  {RADIO_CA08,K55EV_SNAV_FP},
  {RADIO_BT18,K55EV_POWR_SW},           // POWER button
  {RADIO_BT25,K55EV_TUNE_NF},           // NAV tune (inner)
  {RADIO_BT26,K55EV_TUNE_NV},           // NAV tune (outter)
  {RADIO_BT27,K55EV_MODE_NV},           // NAV mode
  {RADIO_BT24,K55EV_TRSF_NV},           // NAV transfer
  {RADIO_BT23,K55EV_VOLM_NV},           // NAV volume
  {0,0},
};
//--------------------------------------------------------------------
//  CDI mode
//--------------------------------------------------------------------
RADIO_HIT K155_N2[] = {
  {RADIO_CA05,K55EV_ANAV_WP},
  {RADIO_CA06,K55EV_ANAV_FP},
  {RADIO_CA09,K55EV_OBSD_D1},           // OBS digit 1
  {RADIO_CA10,K55EV_OBSD_D2},           // OBS digit 2
  {RADIO_CA11,K55EV_OBSD_D3},           // OBS digit 3
  {RADIO_BT25,K55EV_OBSD_D3},           // NAV tune (inner)
  {RADIO_BT26,K55EV_OBSD_D2},           // NAV tune (outter)
  {RADIO_BT27,K55EV_MODE_NV},           // NAV mode
  {RADIO_BT24,K55EV_TRSF_NV},           // NAV transfer
  {RADIO_BT18,K55EV_POWR_SW},           // POWER button
  {0,0},
};
//--------------------------------------------------------------------
//  BRG-FROM mode
//--------------------------------------------------------------------
RADIO_HIT K155_N3[] = {
  {RADIO_CA05,K55EV_ANAV_WP},
  {RADIO_CA06,K55EV_ANAV_FP},
  {RADIO_CA07,K55EV_SNAV_WP},
  {RADIO_CA08,K55EV_SNAV_FP},
  {RADIO_BT25,K55EV_OBSD_D3},           // NAV tune (inner)
  {RADIO_BT26,K55EV_OBSD_D2},           // NAV tune (outter)
  {RADIO_BT27,K55EV_MODE_NV},           // NAV mode
  {RADIO_BT24,K55EV_TRSF_NV},           // NAV transfer
  {RADIO_BT18,K55EV_POWR_SW},           // POWER button
  {0,0},
};
//--------------------------------------------------------------------
//  TIMER mode
//--------------------------------------------------------------------
RADIO_HIT K155_N4[] = {
  {RADIO_CA12,K55EV_TIMR_MN},           // Timer minute
  {RADIO_CA13,K55EV_TIMR_SC},           // Timer second
  {RADIO_BT25,K55EV_TIMR_SC},           // NAV tune (inner)
  {RADIO_BT26,K55EV_TIMR_MN},           // NAV tune
  {RADIO_BT27,K55EV_MODE_NV},           // NAV mode
  {RADIO_BT24,K55EV_TRSF_NV},           // NAV Transfer
  {RADIO_BT18,K55EV_POWR_SW},           // Power Button
  {0,0},
};
//=====================================================================================
//  COMM NAV DRIVER TABLE
//=====================================================================================
RADIO_HIT *CK155radio::Kr55HIT[] = {
  K155_P0,
  K155_C1,
  K155_C2,
  K155_C3,
  K155_N1,
  K155_N2,
  K155_N3,
  K155_N3,
  K155_N4,
};
//=====================================================================================
//  CK155radio Constructor
//=====================================================================================
CK155radio::CK155radio()
{ TypeIs (SUBSYSTEM_KX155_RADIO);
  hwId    = HW_RADIO;
  gTimer  = 0;
  sPower  = 0;
  Power   = 0;
  cdiDEV  = 0;
  cdiST   = 0;
  //--------Init COM fields -----------------------------------------------
  nChanl  = 1;
  InitTable(comTAB,K155_ACOM_WP,cDat1,RADIO_CA01);
  InitTable(comTAB,K155_ACOM_FP,cDat2,RADIO_CA02);
  InitTable(comTAB,K155_SCOM_WP,cDat3,RADIO_CA03);
  InitTable(comTAB,K155_SCOM_FP,cDat4,RADIO_CA04);
  InitTable(comTAB,K155_CHAN_OP,cDat5,RADIO_CA15);            // Chanel CH
  InitTable(comTAB,K155_CHAN_NB,cDat6,RADIO_CA14);            // Chanel number
  StoreFreq(&ActCom,118.95f);
  StoreFreq(&SbyCom,125.00f);
  MakeFrequency(&ActCom);
  comINC[0] = -50;
  comINC[2] = +50;
  InitChanel();
  //--------Init NAV fields -----------------------------------------------
  nTimer  = 0;
  nCount  = 1;
  tMode   = 0;
  InitTable(navTAB,RADIO_FD_ANAV_WP,nDat1,RADIO_CA05);
  InitTable(navTAB,RADIO_FD_ANAV_FP,nDat2,RADIO_CA06);
  InitTable(navTAB,RADIO_FD_SNAV_WP,nDat3,RADIO_CA07);
  InitTable(navTAB,RADIO_FD_SNAV_FP,nDat4,RADIO_CA08);
  InitTable(navTAB,K155_FCDI_GR,nDat5,RADIO_CA16);            // CDI field
  InitTable(navTAB,K155_FOBS_DG,nDat6,RADIO_CA09);            // OBS1
  InitTable(navTAB,K155_TIMR_MN,nDat7,RADIO_CA12);            // Timer: Minute
  InitTable(navTAB,K155_TIMR_SC,nDat8,RADIO_CA13);            // Timer: second
  InitTable(navTAB,K155_INDI_FD,nDat9,RADIO_CA17);            // Operator
  StoreFreq(&ActNav,108.95f);
  StoreFreq(&SbyNav,110.45f);
  MakeFrequency(&ActNav);
  strcpy(cdiFLD,"---------------");
  cdiDIM  = strlen(cdiFLD);
}
//-------------------------------------------------------------------------
//  All parameters read.  Register master radio
//-------------------------------------------------------------------------
void CK155radio::ReadFinished()
{ Radio.rnum = uNum;
  if (1 == uNum)  mveh->RegisterNAV(unId);
  if (1 == uNum)  mveh->RegisterCOM(unId);
  if (1 == uNum)  globals->Radio = &Radio;
  CRadio::ReadFinished();
  return;
}
//-------------------------------------------------------------------------
//  Receive message from external component
//-------------------------------------------------------------------------
EMessageResult CK155radio::ReceiveMessage (SMessage *msg)
{ U_CHAR opt = 0;
  if  (msg->id == MSG_GETDATA) {
    switch (msg->user.u.datatag) {
      //--- Request a pointer to this subsystem --------
      case 'gets':
        msg->voidData = this;
        return MSG_PROCESSED;

      }
      return  CRadio::ReceiveMessage (msg);
  }
  if (msg->id == MSG_SETDATA) {
       //---- SetActive NAV Frequency -----------------
    switch (msg->user.u.datatag) {
      case 'navA':
        opt = (nState == K55_NAV_NOR)?(1):(0);
        TuneNavTo(msg->realData,opt);
        return MSG_PROCESSED;
      case 'snav':
        TuneNavTo(msg->realData,1);
        return MSG_PROCESSED;
      case 'scom':
        TuneComTo(msg->realData,1);
        return MSG_PROCESSED;


    }
  }
      //=======================================
return  CRadio::ReceiveMessage (msg);
}

//--------------------------------------------------------------------------
//  Init chanel frequency table
//--------------------------------------------------------------------------
void CK155radio::InitChanel()
{ short inx = 0;
  while (inx < 32) StoreFreq(&chnFQ[inx++],118.00f);
  return;
}

//--------------------------------------------------------------------------
//  Set chanel frequency in standby
//--------------------------------------------------------------------------
void CK155radio::PopChanelFreq(short nc)
{ SbyCom  = chnFQ[nc - 1];
  SetField(comTAB,K155_SCOM_WP,"%03u",SbyCom.whole);
  SetField(comTAB,K155_SCOM_FP,".%02u",SbyCom.fract / 10);
  return;
}
//--------------------------------------------------------------------------
//  Store standby in chanel table
//--------------------------------------------------------------------------
void CK155radio::PushChanelFreq(short nc)
{ chnFQ[nc - 1] = SbyCom;
  Tim01 = 20;
  return;
}
//--------------------------------------------------------------------------
//  Change selected chanel
//--------------------------------------------------------------------------
void CK155radio::ChangeChanel(short inc,short state)
{ nChanl += inc;
  if ( 0 == nChanl) nChanl = 32;
  if (33 == nChanl) nChanl = 1;
  SetField(comTAB,K155_CHAN_NB,"%02u",nChanl);
  comTAB[K155_CHAN_NB].state = state;
  PopChanelFreq(nChanl);
  return;
}
//--------------------------------------------------------------------------
//  Swap com turn between chanel and frequency
//--------------------------------------------------------------------------
void CK155radio::SwapComTurn()
{ if (1 == cTurn)
  { cTurn = 0;
    comTAB[K155_CHAN_NB].state = RAD_ATT_ACTIV;
    comTAB[K155_SCOM_WP].state = RAD_ATT_FLASH;
    comTAB[K155_SCOM_FP].state = RAD_ATT_FLASH;
    return; }
  else 
  { cTurn = 1;
    comTAB[K155_CHAN_NB].state = RAD_ATT_FLASH;
    comTAB[K155_SCOM_WP].state = RAD_ATT_ACTIV;
    comTAB[K155_SCOM_FP].state = RAD_ATT_ACTIV;
  }
  return; 
}
//===========================================================================
//  COM MANAGEMENT
//===========================================================================
//--------------------------------------------------------------------------
//  COM enter normal state
//--------------------------------------------------------------------------
int CK155radio::COMenterNormal()
{ InhibitFields(comTAB,K155_DCOM_SZ);
  SetField(comTAB,K155_ACOM_WP,"%03u",ActCom.whole);
  SetField(comTAB,K155_ACOM_FP,".%02u",ActCom.fract / 10);
  SetField(comTAB,K155_SCOM_WP,"%03u",SbyCom.whole);
  SetField(comTAB,K155_SCOM_FP,".%02u",SbyCom.fract / 10);
  cState  = K55_COM_NOR;
  return 1;
}
//--------------------------------------------------------------------------
//  COM Normal mode
//--------------------------------------------------------------------------
int CK155radio::COMstateNormal(K55_EVENT evn)
{ switch (evn)  {
    //---Change ACtive com whole part --------------------
    case K55EV_ACOM_WP:
      ModifyWholeCom(K155_ACOM_WP,&ActCom,mDir);
      return 1;
    //---Change ACtive com fract part --------------------
    case K55EV_ACOM_FP:
      ModifyFractCom(K155_ACOM_FP,&ActCom,mDir);
      return 1;
    //---Change Standby com whole part --------------------
    case K55EV_SCOM_WP:
      ModifyWholeCom(K155_SCOM_WP,&SbyCom,mDir);
      return 1;
    //---Change Standby com fract part --------------------
    case K55EV_SCOM_FP:
      ModifyFractCom(K155_SCOM_FP,&SbyCom,mDir);
      return 1;
    //---Tune standby com ------------- --------------------
    case K55EV_TUNE_CM:
      ModifyWholeCom(K155_SCOM_WP,&SbyCom,-mDir);
      return 1;
    //---Tune standby com ------------- --------------------
    case K55EV_TUNE_CF:
      ModifyFractCom(K155_SCOM_FP,&SbyCom,-mDir);
      return 1;
    //---Enter chanel selet mode ---------------------------
    case K55EV_CHAN_BT:
      COMenterSelect();
      return 1;
    //---Swap active and standby com -----------------------
    case K55EV_TRSF_CM:
      SwapCom();
      return 1;
}
return 0;
}
//----------------------------------------------------------------------------
//  Enter chanel selection mode
//  Arm a 6 seconds timer
//----------------------------------------------------------------------------
int CK155radio::COMenterSelect()
{ SetField(comTAB,K155_CHAN_OP,"%s","CH");
  ChangeChanel(0,RAD_ATT_ACTIV);
  cState  = K55_COM_CH;
  Tim01   = 6;                    
  return 1;
}
//--------------------------------------------------------------------------
//  COM Chanel select mode
//  Must return 1 if event is exclusively processed
//--------------------------------------------------------------------------
int CK155radio::COMstateSelect(K55_EVENT evn)
{ switch (evn)  {
      //---Ignore all digit change request -------------
      case K55EV_ACOM_WP:
        return 1;
      case K55EV_ACOM_FP:
        return 1;
      case K55EV_SCOM_WP:
        return 1;
      case K55EV_SCOM_FP:
        return 1;
      //---Change chanel digits -----------------------
      case K55EV_CHAN_DG:
        ChangeChanel(mDir,RAD_ATT_ACTIV);
        Tim01   = 6;
        return 1;
      //---Select another chanel ----------------------
      case K55EV_TUNE_CM:
        ChangeChanel(mDir,RAD_ATT_ACTIV);
        Tim01 = 6;
        return 1;
      //---Enter chanel programing mode ---------------
      case K55EV_CHAN_BT:
        COMenterPgMode();
        return 1;
      //---Check for time out in this mode ------------
      case K55EV_TOPSC:
        Tim01--;
        if (Tim01)  return 0;
        COMenterNormal();
        return 0;
}
  return 0;
}
//--------------------------------------------------------------------------
//  Enter chanel programing mode 
//--------------------------------------------------------------------------
int CK155radio::COMenterPgMode()
{ SetField(comTAB,K155_CHAN_OP,"%s","PG");
  ChangeChanel(0,RAD_ATT_FLASH);
  Tim01   = 20;
  cState  = K55_COM_PG;
  cTurn   = 1;
  return 1;
}
//--------------------------------------------------------------------------
//  COM chanel programing mode 
//  Must return 1 if event is exclusively processed
//--------------------------------------------------------------------------
int CK155radio::COMstatePgMode(K55_EVENT evn)
{ switch (evn) {
      //----Ignore active com change request ----------------------
      case K55EV_ACOM_WP:
        return 1;
      case K55EV_ACOM_FP:
        return 1;
      //---Process standby request if freq turn ------------------
      case K55EV_SCOM_WP:
        if (1 == cTurn) return 1;
        ModifyWholeCom(K155_SCOM_WP,&SbyCom,mDir);
        comTAB[K155_SCOM_WP].state = RAD_ATT_FLASH;
        PushChanelFreq(nChanl);
        return 1;
      case K55EV_SCOM_FP:
        if (1 == cTurn) return 1;
        ModifyFractCom(K155_SCOM_FP,&SbyCom,mDir);
        comTAB[K155_SCOM_FP].state = RAD_ATT_FLASH;
        PushChanelFreq(nChanl);
        return 1;
      //---Select another chanel ---------------------------------
      case K55EV_TUNE_CM:
        if (0 == cTurn)   return 1;
        ChangeChanel(mDir,RAD_ATT_FLASH);
        Tim01 = 20;
        return 1;
      //---Change chanel digits -----------------------
      case K55EV_CHAN_DG:
        if (0 == cTurn)   return 1;
        ChangeChanel(mDir,RAD_ATT_FLASH);
        Tim01   = 20;
        return 1;
      //---Swap turn between chanel and frequency -----------------
      case K55EV_TRSF_CM:
        SwapComTurn();
        Tim01 = 20;
        return 1;
      //--Return to normal mode -----------------------------------
      case K55EV_CHAN_BT:
        COMenterNormal();
        return 1;
      //--Check for Time Out --------------------------------------
      case K55EV_TOPSC:
        Tim01--;
        if (Tim01)  return 0;
        COMenterNormal();
        return 0;
}
  return 0;
}
//===========================================================================
//  NAV MANAGEMENT
//===========================================================================
//--------------------------------------------------------------------------
//  NAV enter normal state
//--------------------------------------------------------------------------
int CK155radio::NAVenterNormal()
{ InhibitFields(navTAB,K155_DNAV_SZ);
  SetField(navTAB,RADIO_FD_ANAV_WP,"%03u",ActNav.whole);
  SetField(navTAB,RADIO_FD_ANAV_FP,".%02u",ActNav.fract / 10);
  SetField(navTAB,RADIO_FD_SNAV_WP,"%03u",SbyNav.whole);
  SetField(navTAB,RADIO_FD_SNAV_FP,".%02u",SbyNav.fract / 10);
  nState  = K55_NAV_NOR;
  return 1;
}
//--------------------------------------------------------------------------
//  NAV normal mode 
//--------------------------------------------------------------------------
int CK155radio::NAVstateNormal(K55_EVENT evn)
{ switch (evn)  {

      //---Modify Active NAV whole part -----------------------
      case K55EV_ANAV_WP:
        ModifyWholeNav(RADIO_FD_ANAV_WP,&ActNav,mDir);
        return 1;
      //---Modify Active Nav fract part -----------------------
      case K55EV_ANAV_FP:
        ModifyFractNav(RADIO_FD_ANAV_FP,&ActNav,mDir);
        return 1;
      //---Modify Standby Nav whole part ----------------------
      case K55EV_SNAV_WP:
        ModifyWholeNav(RADIO_FD_SNAV_WP,&SbyNav,mDir);
        return 1;
      //---Modify Standby fract part --------------------------
      case K55EV_SNAV_FP:
        ModifyFractNav(RADIO_FD_SNAV_FP,&SbyNav,mDir);
        return 1;
      //---Swap active and Standby NAV -------------------------
      case K55EV_TRSF_NV:
        SwapNav(1);
        return 1;
      //---Enter CDI mode -------------------------------------
      case K55EV_MODE_NV:
        NAVenterCDI();
        return 1;
      //---Tune Active  NAV ------------- --------------------
      case K55EV_TUNE_NV:
        ModifyWholeNav(RADIO_FD_SNAV_WP,&SbyNav,-mDir);
        return 1;
      //---Tune Active  NAV ------------- --------------------
      case K55EV_TUNE_NF:
        ModifyFractNav(RADIO_FD_SNAV_FP,&SbyNav,-mDir);
        return 1;
  }
  return 0;
}
//--------------------------------------------------------------------------
//  Enter CDI mode
//--------------------------------------------------------------------------
int CK155radio::NAVenterCDI()
{ nState    = K55_NAV_CDI;
  InhibitFields(navTAB,K155_DNAV_SZ);
  SetField(navTAB,RADIO_FD_ANAV_WP,"%03u",ActNav.whole);
  SetField(navTAB,RADIO_FD_ANAV_FP,".%02u",ActNav.fract / 10);
  RefreshCDI();
  return 1;
}
//--------------------------------------------------------------------------
//  REFRESH CDI MODE 
//--------------------------------------------------------------------------
int CK155radio::RefreshCDI()
{ SetField(navTAB,K155_FCDI_GR,"%s",cdiFLD);
  if (VOR)  return SetOBS(0);
  if (ILS)  return SetILS();
  //-----Flag mode --------------------------------------------
  SetField(navTAB,K155_FOBS_DG,"%03u",OBS);
  strncpy(nDat5+2,"FLAGS",4);
  cdiST     = 0;                      // FLAG
  return 1;
}
//--------------------------------------------------------------------------
//  Set flag according to CID status
//--------------------------------------------------------------------------
int CK155radio::SetFlag()
{ if (0 == cdiST)   strncpy(nDat5+2,"FLAGS",4);
  else  
  if (1 == cdiST)   nDat5[7]  = '\x81';     // To flag
  else              nDat5[7]  = '\x82';     // From flag
  return cdiST;
}
//--------------------------------------------------------------------------
//  Enter ILS mode 
//--------------------------------------------------------------------------
int CK155radio::SetILS()
{ float dir = ILS->GetRwyDirection();
  cdiDEV    = ComputeDeviation(dir,ILS->GetRadial(),&cdiST,sPower);
  SetField(navTAB,K155_FOBS_DG,"%s","LOC");
  SetFlag();
  return 1;
}
//--------------------------------------------------------------------------
//  Change OBS from internal
//--------------------------------------------------------------------------
int CK155radio::SetOBS(short inc)
{ if (0 == VOR) return 0;
  OBS += inc;
  if (  0 > OBS) OBS -= inc;
  if (359 < OBS) OBS -= inc;
  cdiDEV  = ComputeDeviation(OBS,VOR->GetRadial(),&cdiST,sPower);
  SetField(navTAB,K155_FOBS_DG,"%03u",OBS);
  SetFlag();
  return 1;
}
//--------------------------------------------------------------------------
//  NAV CDI mode
//--------------------------------------------------------------------------
int CK155radio::NAVstateCDI(K55_EVENT evn)
{ switch (evn)  {
      //---Modify Active NAV whole part -----------------------
      case K55EV_ANAV_WP:
        ModifyWholeNav(RADIO_FD_ANAV_WP,&ActNav,mDir);
        return 1;
      //---Modify Active Nav fract part -----------------------
      case K55EV_ANAV_FP:
        ModifyFractNav(RADIO_FD_ANAV_FP,&ActNav,mDir);
        return 1;
      //---Modify OBS value -------------------------------------
      case K55EV_OBSD_D1:
        SetOBS(mDir * 100);
        return 1;
      case K55EV_OBSD_D2:
        SetOBS(mDir * 10);
        return 1;
      case K55EV_OBSD_D3:
        SetOBS(mDir);
        return 1;
      //---Swap active and Standby NAV -------------------------
      case K55EV_TRSF_NV:
        SwapNav(0);
        return 1;
      //---Refresh CDI -----------------------------------------
      case K55EV_CLOCK:
        RefreshCDI();
        return 1;
      //---Enter BEARING  MODE ---------------------------------
      case K55EV_MODE_NV:
        NAVenterBRG();
        return 1;
  }
  return 0;
}
//--------------------------------------------------------------------------
//  Get the station direction
//--------------------------------------------------------------------------
int CK155radio::SetStationBearing()
{ float rad = 0;
  if (VOR)  rad = VOR->GetRadial();
  if (ILS)  rad = ILS->GetRadial();
  int dir   = GetRounded(rad);
  OBS       = dir;
  SetField(navTAB,K155_FOBS_DG,"%03u",OBS);
  return OBS;
}
//--------------------------------------------------------------------------
//  Refresh Bearing
//--------------------------------------------------------------------------
int CK155radio::RefreshBearing()
{ if (VOR)  return SetStationBearing();
  if (ILS)  return SetStationBearing();
  SetField(navTAB,K155_FOBS_DG,"%s","---");
  nDat5[7]  = '\x81';     // To flag
  return 1;
}
//--------------------------------------------------------------------------
//  Enter Bearing mode
//--------------------------------------------------------------------------
int CK155radio::NAVenterBRG()
{ nState    = K55_NAV_BRG;
  InhibitFields(navTAB,K155_DNAV_SZ);
  SetField(navTAB,RADIO_FD_ANAV_WP,"%03u", ActNav.whole);
  SetField(navTAB,RADIO_FD_ANAV_FP,".%02u",ActNav.fract / 10);
  SetField(navTAB,K155_INDI_FD,"%s","TO");
  RefreshBearing();
  return 1;
}
//-------------------------------------------------------------------------
//  Bearing mode
//-------------------------------------------------------------------------
int CK155radio::NAVstateBRG(K55_EVENT evn)
{ switch (evn)  {
      //---allow to change Active frequency ------------
      case K55EV_ANAV_WP:
        ModifyWholeNav(RADIO_FD_ANAV_WP,&ActNav,mDir);
        return 1;

      case K55EV_ANAV_FP:
        ModifyFractNav(RADIO_FD_ANAV_FP,&ActNav,mDir);
        return 1;
      //---Swap frequency ------------------------------
      case K55EV_TRSF_NV:
        SwapNav(0);
        return 1;
      //---Tune Active  NAV ------------- --------------------
      case K55EV_TUNE_NV:
        if (-1 == mDir) ModifyWholeNav(RADIO_FD_ANAV_WP,&ActNav,+1);
        if (+1 == mDir) ModifyFractNav(RADIO_FD_ANAV_FP,&ActNav,-1);
        return 1;
      //---Refresh bearing mode ---------------------------
      case K55EV_CLOCK:
        RefreshBearing();
        return 1;
      //---Enter radial mode -----------------------------
      case K55EV_MODE_NV:
        NAVenterRAD();
        return 1;
  }
  return 0;
}
//--------------------------------------------------------------------------
//  Get the station Radial
//--------------------------------------------------------------------------
int CK155radio::SetStationRadial()
{ float rad = 0;
  if (VOR)  rad = VOR->GetRadial();
  if (ILS)  rad = ILS->GetRadial();
  int dir = GetRounded(rad) - 180;
  OBS     = Wrap360(dir);
  SetField(navTAB,K155_FOBS_DG,"%03u",OBS);
  return OBS;
}
//--------------------------------------------------------------------------
//  Refresh Radial
//--------------------------------------------------------------------------
int CK155radio::RefreshRadial()
{ if (VOR)  return SetStationRadial();
  if (ILS)  return SetStationRadial();
  SetField(navTAB,K155_FOBS_DG,"%s","---");
  nDat5[7]  = '\x81';     // To flag
  return 1;
}
//--------------------------------------------------------------------------
//  Enter FROM mode
//--------------------------------------------------------------------------
int CK155radio::NAVenterRAD()
{ nState    = K55_NAV_RAD;
  InhibitFields(navTAB,K155_DNAV_SZ);
  SetField(navTAB,RADIO_FD_ANAV_WP,"%03u",ActNav.whole);
  SetField(navTAB,RADIO_FD_ANAV_FP,".%02u",ActNav.fract / 10);
  SetField(navTAB,K155_INDI_FD,"%s","FR");
  RefreshRadial();
  return 1;
}
//-------------------------------------------------------------------------
//  Radial mode
//-------------------------------------------------------------------------
int CK155radio::NAVstateRAD(K55_EVENT evn)
{ switch (evn)  {
      //---allow to change Active frequency ------------
      case K55EV_ANAV_WP:
        ModifyWholeNav(RADIO_FD_ANAV_WP,&ActNav,mDir);
        return 1;

      case K55EV_ANAV_FP:
        ModifyFractNav(RADIO_FD_ANAV_FP,&ActNav,mDir);
        return 1;
      //---Swap frequency ------------------------------
      case K55EV_TRSF_NV:
        SwapNav(0);
        return 1;
      //---Tune Active  NAV ------------- --------------------
      case K55EV_TUNE_NV:
        if (-1 == mDir) ModifyWholeNav(RADIO_FD_ANAV_WP,&ActNav,+1);
        if (+1 == mDir) ModifyFractNav(RADIO_FD_ANAV_FP,&ActNav,-1);
        return 1;
      //---Refresh radial mode ---------------------------
      case K55EV_CLOCK:
        RefreshRadial();
        return 1;
      //---Enter timer mode -----------------------------
      case K55EV_MODE_NV:
        NAVenterTIM();
        return 1;
  }
  return 0;
}
//-------------------------------------------------------------------------
//  Refresh Timer
//  nCount  = 0 Timer is in pause mode
//  nCount  = +1    Increment mode
//  nCount  = -1    Decrement mode
//-------------------------------------------------------------------------
int CK155radio::NAVupdateTimer()
{ nTimer += nCount;
  if (60000 == nTimer) nTimer = 0;
  if (0     == nTimer) nCount = 0;
  return 1;
}
//-------------------------------------------------------------------------
//  Edit  Timer
//-------------------------------------------------------------------------
int CK155radio::EditTimer()
{ int mn = nTimer / 60;
  int sc = nTimer % 60;
  SetField(navTAB,K155_TIMR_MN,"%02u",mn);
  SetField(navTAB,K155_TIMR_SC,":%02u",sc);
  if (tMode == 0)   return 1;
  navTAB[K155_TIMR_MN].state = RAD_ATT_FLASH;
  navTAB[K155_TIMR_SC].state = RAD_ATT_FLASH;
  return 1;
}
//-------------------------------------------------------------------------
//  Enter timer mode
//-------------------------------------------------------------------------
int CK155radio::NAVenterTIM()
{ nState    = K55_NAV_TIM;
  InhibitFields(navTAB,K155_DNAV_SZ);
  SetField(navTAB,RADIO_FD_ANAV_WP,"%03u", ActNav.whole);
  SetField(navTAB,RADIO_FD_ANAV_FP,".%02u",ActNav.fract / 10);
  tMode = 0;                  // Display turn
  EditTimer();
  SetField(navTAB,K155_INDI_FD,"%s","ET");
  return 1;
}
//-----------------------------------------------------------------------
//  Change timer value
//-----------------------------------------------------------------------
int CK155radio::ChangeTimerValue(short inc)
{ tMode = 2;
  nTimer += inc;
  if (nTimer >= 60000) nTimer -= inc;
  EditTimer();
  return 1;
}
//-----------------------------------------------------------------------
// Change Timer mode
//-----------------------------------------------------------------------
int CK155radio::ChangeTimerMode()
{
  switch (tMode)  {
    //---Enter programing mode --------------------
    case 0:
      tMode   = 1;
      nCount  = 0;
      nTimer  = 0;
      return EditTimer();
    //--Return to count up mode -------------------
    case 1:
      tMode   = 0;
      nCount  = +1;
      return EditTimer();
    //--Return to count down mode -----------------
    case 2:
      tMode   = 0;
      nCount  = -1;
      return EditTimer();
  }
  return 0;
}

//-----------------------------------------------------------------------
//  Timer mode
//-----------------------------------------------------------------------
int CK155radio::NAVstateTIM(K55_EVENT evn)
{ switch (evn)  {    
      //---Dont allow to change Active frequency ------------
      case K55EV_ANAV_WP:
        return 1;
      case K55EV_ANAV_FP:
        return 1;
      //--Modify timer if in programing mode ----------------
      case K55EV_TIMR_MN:
        if (0 == tMode) return 1;
        ChangeTimerValue(60 * mDir);
        return 1;
      case K55EV_TIMR_SC:
        if (0 == tMode) return 1;
        ChangeTimerValue(mDir);
        return 1;
      //---Change timer mode --------------------------------
      case K55EV_TRSF_NV:
        ChangeTimerMode();
        return 1;
      //---Enter normal mode -----------------------------
      case K55EV_MODE_NV:
        NAVenterNormal();
        return 1;
      //---Update timer display -------------------------
      case K55EV_TOPSC:
        EditTimer();
        return 0;
  }
  return 0;
}
//-----------------------------------------------------------------------
//  Update power
//-----------------------------------------------------------------------
int CK155radio::PowerOFF()
{ sPower      = 0;
  Radio.actv  = 0;
  if (VOR)  VOR->DecUser();
  VOR     = 0;
  if (COM)  COM->DecUser();
  COM     = 0;
  if (ILS)  ILS->DecUser();
  ILS     = 0;
  if (1 == uNum)  globals->rdb->TuneTo(0);
  return 0;
}
//============================================================================
//  K55 DISPATCHER
//============================================================================
int CK155radio::Dispatcher(K55_EVENT evn)
{ 
  switch (sPower) {
    //----Check for power ON----------------------------
    case 0:
      if (0 == active)          return 0;
      if (K55EV_POWR_SW != evn) return 0;
      Radio.actv = 1;
      sPower  = 1;
      COMenterNormal();
      cTurn   = 1;
      NAVenterNormal();
      nTimer  = 0;
      nCount  = 1;
      tMode   = 0;
      gTimer  = 0;              // Second timer
      mskFS   = 0xFF;           // Character on
      return 1;
    //---Power is ON -----------------------------------
    case 1:
      if (K55EV_POWR_SW == evn) return PowerOFF();
      if (0 == active)          return PowerOFF();
      break;
  }
  //------Process other events -------------------------
  if (K55EV_TOPSC == evn)       NAVupdateTimer();
  (this->*staTAB[cState])(evn);   // Dispatch COM event
  (this->*staTAB[nState])(evn);   // Dispatch NAV event
  return 1;
}
//-------------------------------------------------------------------------------
//  TIME SLICE
//-------------------------------------------------------------------------------
void CK155radio::TimeSlice (float dT,U_INT FrNo)
{ CRadio::TimeSlice (dT,FrNo);
  if (0 == sPower)        return;
  //---Update flasher --------------------------------------------------
  mskFS = RadioMSK[globals->clk->GetON()];
  //--------------------------------------------------------------------
  if (0 == Dispatcher(K55EV_CLOCK))     return;              // Clock Event
  //----Refresh all nav stations ---------------------------------------
  VOR	= globals->dbc->GetTunedNAV(VOR,FrNo,ActNav.freq);     // Refresh VOR
  ILS = globals->dbc->GetTunedILS(ILS,FrNo,ActNav.freq);     // Refresh ILS
  COM = globals->dbc->GetTunedCOM(COM,FrNo,ActCom.freq);     // Refresh com
  if (1 == uNum)    globals->rdb->TuneTo(COM);
  if (1 == uNum)    globals->cILS = ILS;
  //----Compute second timer -------------------------------------------
  gTimer += dT;
  if (gTimer < 1.0)       return;
  gTimer -= 1.0;
  Dispatcher(K55EV_TOPSC);                              // Beat second
  return;
}
//==========================================================================
//
//  CKR87 ADF radio
//
//==========================================================================
//  VECTOR TABLE
//==========================================================================
CKR87radio::StaFN CKR87radio::adfn[]= {
    &CKR87radio::K87statePOF,                 // STATE 0 K87_POW_OF
    &CKR87radio::K87stateFRQ,                 //       1 K87_FRQ_ON
    &CKR87radio::K87stateTIM,                 //       2 K87_TIM_ON
};
//--------------------------------------------------------------------
//  Constructor
//--------------------------------------------------------------------
CKR87radio::CKR87radio()
{ TypeIs (SUBSYSTEM_ADF_RADIO);
  hwId     = HW_RADIO;
  indnMode = INDN_LINEAR;
  ratK    = 1;
  sPower  = 0;
  aPower  = 0;
  NDB     = 0;
  comp    = 0;
  StoreFreq(&ActFRQ,212.0f);
  StoreFreq(&SbyFRQ,319.0f);
  //-----Init all fields ----------------------------------------
  InitTable(adfTAB,KR87_ACTF_D1,aDat1,0);  // Active digit 1
  InitTable(adfTAB,KR87_ACTF_D2,aDat2,0);  // Active digit 2
  InitTable(adfTAB,KR87_ACTF_D3,aDat3,0);  // Active digit 3
  InitTable(adfTAB,KR87_ACTF_D4,aDat4,0);  // Active digit 4
  InitTable(adfTAB,KR87_ACTF_D5,aData,0);  // Active digit 5
  InitTable(adfTAB,KR87_SBYF_D1,aDat5,0);  // Standby digit 1
  InitTable(adfTAB,KR87_SBYF_D2,aDat6,0);  // Standby digit 2
  InitTable(adfTAB,KR87_SBYF_D3,aDat7,0);  // Standby digit 3
  InitTable(adfTAB,KR87_SBYF_D4,aDat8,0);  // Standby digit 4
  InitTable(adfTAB,KR87_SBYF_D5,aDatb,0);  // Standby digit 5
  //------------------------------------------------------------------
  InitTable(adfTAB,KR87_ANTL_FD,"ANT",0);  // Antena LED
  InitTable(adfTAB,KR87_ADFL_FD,"ADF",0);  // ADF LED
  InitTable(adfTAB,KR87_BFOL_FD,"BFO",0);  // BFO LED
  InitTable(adfTAB,KR87_FRQL_FD,"FRQ",0);  // FRQ LED
  InitTable(adfTAB,KR87_FLTL_FD,"FLT",0);  // FLT LED
  InitTable(adfTAB,KR87_ETLD_FD,"ET", 0);  // ET LED
  InitTable(adfTAB,KR87_TIMN_FD,aDatf,0);  // Minute
  InitTable(adfTAB,KR87_TISC_FD,aDate,0);  // seconde
}
//-----------------------------------------------------------------------
//  Read all Tags
//  NOTE:  ALL tags are ignored for now.
//-----------------------------------------------------------------------
int CKR87radio::Read (SStream *stream, Tag tag)
{ switch (tag) {
  case 'adf_':
    return TAG_READ;
  case 'bfo_':
    return TAG_READ;
  case 'flt_':
    return TAG_READ;
  case 'et__':
    return TAG_READ;
  case 'etst':
    return TAG_READ;
  case 'mode':
    return TAG_READ;
  case 'freq':
    return TAG_READ;
  case 'ftim':
    { int temp;
      ReadInt (&temp, stream);
      return TAG_READ;
    }
  case 'etim':
    { int temp;
      ReadInt (&temp, stream);
      return TAG_READ;
    }
  case 'tone':
    return TAG_READ;
  case 'frct':
    return TAG_READ;
  case 'stby':
    { float stby;
      ReadFloat (&stby, stream);
      return TAG_READ;
    }
  }

  // See if the tag can be processed by the parent class type
  return CRadio::Read (stream, tag);
}
//------------------------------------------------------------------------
//  All parameters are read
//------------------------------------------------------------------------
void CKR87radio::ReadFinished()
{ //--- set compute mode at modulo 360 -------
  indnMode  = INDN_MOD360;
  timK      = 1;
  //------ Enter power off -------------------------------------------
  K87enterPOF();
  CRadio::ReadFinished();
  return;
}
//------------------------------------------------------------------------
//	Get the radial from NDB
//------------------------------------------------------------------------
void CKR87radio::SetRadial(SMessage *msg)
{	msg->sender   = unId;
  msg->realData	= indn;
	return;
}
//------------------------------------------------------------------------
//  Tune To given frequency
//------------------------------------------------------------------------
void CKR87radio::TuneToFreq(SMessage *msg)
{ SbyFRQ  = ActFRQ;
  StoreFreq(&ActFRQ,msg->realData);
  EditActFRQ();
  return;
}
//------------------------------------------------------------------------
//  Tune compas
//------------------------------------------------------------------------
void CKR87radio::UpdateCompass(short inc)
{ comp += inc;
  if (0   > comp)    comp = 359;
  if (359 < comp)    comp = 1;
  return;
}
//------------------------------------------------------------------------
//  Process Message request
//------------------------------------------------------------------------
EMessageResult CKR87radio::ReceiveMessage (SMessage *msg)
{if (msg->id == MSG_GETDATA) {
      switch (msg->user.u.datatag) {
      //--- Request a pointer to this subsystem --------
      case 'gets':
        msg->voidData = this;
        return MSG_PROCESSED;
      //--- Request current radial ---------------------
      case 'navd':
			  SetRadial(msg);
			  return MSG_PROCESSED;
      //--- Request compss direction -------------------
      case 'comp':
        msg->intData  = comp;
        return MSG_PROCESSED;
      //--- Request active frequency -------------------
      case 'tune':
        msg->realData = ActFRQ.freq;
        return MSG_PROCESSED;
      }
    return  CRadio::ReceiveMessage (msg);
  }
      //=======================================
if (msg->id == MSG_SETDATA) {
      switch(msg->user.u.datatag) {
      case 'adfA':
        TuneToFreq(msg);
        return MSG_PROCESSED;

      case 'tune':
        TuneToFreq(msg);
        return MSG_PROCESSED;

      case 'comp':
        UpdateCompass(short(msg->realData));
        return MSG_PROCESSED;
      }
      return  CRadio::ReceiveMessage (msg);
  }
  return  CRadio::ReceiveMessage (msg);
}
//------------------------------------------------------------------------
//  Edit Active frequency
//------------------------------------------------------------------------
void CKR87radio::EditActFRQ()
{ char dg[8];
  _snprintf(dg,8,"%04u",ActFRQ.whole);
  if ('0' == dg[0]) dg[0] = ' ';
  SetField(adfTAB,KR87_ACTF_D1,dg[0]);
  SetField(adfTAB,KR87_ACTF_D2,dg[1]);
  SetField(adfTAB,KR87_ACTF_D3,dg[2]);
  SetField(adfTAB,KR87_ACTF_D4,dg[3]);
  _snprintf(dg,8,"%u",(ActFRQ.fract / 100));
  SetField(adfTAB,KR87_ACTF_D5,dg[0]);
  return;
}
//------------------------------------------------------------------------
//  Edit Standby frequency
//------------------------------------------------------------------------
void CKR87radio::EditSbyFRQ()
{ char dg[8];
  _snprintf(dg,8,"%04u",SbyFRQ.whole);
  if ('0' == dg[0]) dg[0] = ' ';
  SetField(adfTAB,KR87_SBYF_D1,dg[0]);
  SetField(adfTAB,KR87_SBYF_D2,dg[1]);
  SetField(adfTAB,KR87_SBYF_D3,dg[2]);
  SetField(adfTAB,KR87_SBYF_D4,dg[3]);
  _snprintf(dg,8,"%u",(SbyFRQ.fract / 100));
  SetField(adfTAB,KR87_SBYF_D5,dg[0]);
  return;
}
//------------------------------------------------------------------------
//  Edit a timer
//------------------------------------------------------------------------
void CKR87radio::EditTimer()
{ U_SHORT tim = (0 == tMode)?(Timer):(eTime);
  U_SHORT mn = tim / 60;
  U_SHORT sc = tim % 60;
  SetField(adfTAB,KR87_TIMN_FD, "%02u",mn);
  SetField(adfTAB,KR87_TISC_FD,":%02u",sc);
  if (tMode < 2)  return;
  adfTAB[KR87_TIMN_FD].state = RAD_ATT_FLASH;
  adfTAB[KR87_TISC_FD].state = RAD_ATT_FLASH;
  return;
}
//------------------------------------------------------------------------
//  Modify Frequency (whole part)
//------------------------------------------------------------------------
void CKR87radio::ModifyFreq(RADIO_FRQ *loc,short inc)
{ U_SHORT wp = loc->whole;
  wp  += inc;
  if (wp <  200)  wp -= inc;
  if (wp > 1799)  wp -= inc; 
  loc->whole = wp;
  MakeFrequency(loc);
  return;
}
//------------------------------------------------------------------------
//  Modify Frequency (fractional part)(by 100)
//------------------------------------------------------------------------
void CKR87radio::ModifyFrac(RADIO_FRQ *loc,short inc)
{ U_SHORT fp = loc->fract;
  fp  += inc;
  if (fp <    0)  fp -= inc;
  if (fp >  900)  fp -= inc; 
  loc->fract = fp;
  MakeFrequency(loc);
  return;
}
//------------------------------------------------------------------------
//  Swap frequency
//------------------------------------------------------------------------
void CKR87radio::SwapFreq()
{ tempf   = ActFRQ;
  ActFRQ  = SbyFRQ;
  SbyFRQ  = tempf;
  EditActFRQ();
  EditSbyFRQ();
  return;
}
//------------------------------------------------------------------------
//  Swap ADF Mode
//------------------------------------------------------------------------
void CKR87radio::SwapLED()
{ if (RAD_ATT_INACT == adfTAB[KR87_ADFL_FD].state)  
  { adfTAB[KR87_ADFL_FD].state = RAD_ATT_ACTIV;
    adfTAB[KR87_ANTL_FD].state = RAD_ATT_INACT;
  }
  else                                  
  { adfTAB[KR87_ADFL_FD].state = RAD_ATT_INACT;
    adfTAB[KR87_ANTL_FD].state = RAD_ATT_ACTIV;
  }
  return;
}
//------------------------------------------------------------------------
//  Swap BFO Mode
//------------------------------------------------------------------------
void CKR87radio::SwapBFO()
{ if (RAD_ATT_INACT == adfTAB[KR87_BFOL_FD].state)  
  { adfTAB[KR87_BFOL_FD].state = RAD_ATT_ACTIV;
  }
  else                                  
  { adfTAB[KR87_BFOL_FD].state = RAD_ATT_INACT;
  }
  return;
}
//------------------------------------------------------------------------
//  Update both timers by one second
//------------------------------------------------------------------------
void CKR87radio::UpdateTimer()
{ Timer++;                                    // FLT Timer
  if (Timer >= 6000)    Timer   = 0;          // Recycle after 100 minutes
  eTime +=  uCount;                           // ET timer
  if (0 == eTime)       uCount  = 0;          // Pause when 0
  if (eTime >= 6000)    uCount  = 0;          // Pause when 100 mn elapse
  if (eTime == 0xFFFF)  eTime   = 0;          // Reset to 0 if negative
  return;
}
//------------------------------------------------------------------------
//  Change ELT value
//------------------------------------------------------------------------
void  CKR87radio::ChangeTime(short inc)
{ if (tMode < 2)  return;
  eTime += inc;
  if (eTime <    0) eTime = 0;
  if (eTime > 3600) eTime = 3600;
  EditTimer();
  tMode = 3;
  return;
}
//------------------------------------------------------------------------
//  Enter Power OFF mode
//------------------------------------------------------------------------
int CKR87radio::K87enterPOF()
{ rState  = K87_POW_OF;
  sPower  = 0;
  InhibitFields(adfTAB,KR87_SIZE_FD);
  mveh->RegisterADF(0);
  if (NDB) NDB->DecUser();
  NDB = 0;
  indnTarget  = 0;
  return 1;
}
//------------------------------------------------------------------------
//  Detect POWER ON  STATE
//------------------------------------------------------------------------
int CKR87radio::K87statePOF(K87_EVENT evn)
{ if (K87EV_POWR != evn)  return 0;
  if (0 == active)        return 0;
  //----Enter Frequency state ---------------------
  adfTAB[KR87_ADFL_FD].state = RAD_ATT_ACTIV;
  sPower  = 1;
  eTime   = 0;
  Timer   = 0;
  uCount  = +1;
  tMode   = 0;
  K87enterFRQ();
  return 1;
}
//------------------------------------------------------------------------
//  Enter frequency state
//------------------------------------------------------------------------
int CKR87radio::K87enterFRQ()
{ mveh->RegisterADF(unId);                              // Register ADF
  EditActFRQ();                                         // Edit Active freq
  EditSbyFRQ();                                         // Edit Standby freq
  RazField(adfTAB,KR87_FRQL_FD);                        // Set FRQ mode
  gTime   = 0;
  rState  = K87_FRQ_ON;                                 // Frequency state
  return 1;
}
//------------------------------------------------------------------------
//  Common event decoder
//------------------------------------------------------------------------
int CKR87radio::K87eventDEC(K87_EVENT evn)
{ 
  switch (evn)  {
  //--------Power switch --------------------------------
  case K87EV_POWR:
    K87enterPOF();
    return 1;
    //--------Active digits -------------------------------
  case K87EV_ACT1:
    ModifyFreq(&ActFRQ,(1000 * mDir));
    EditActFRQ();
    return 1;
  case K87EV_ACT2:
    ModifyFreq(&ActFRQ,(100 * mDir));
    EditActFRQ();
    return 1;
  case K87EV_ACT3:
    ModifyFreq(&ActFRQ,(10 * mDir));
    EditActFRQ();
    return 1;
  case K87EV_ACT4:
    ModifyFreq(&ActFRQ,mDir);
    EditActFRQ();
    return 1;
  case K87EV_ACT5:
    ModifyFrac(&ActFRQ,(mDir * 100));
    EditActFRQ();
    return 1;

  //------ADF button ----------------------------------
  case K87EV_BADF:
    SwapLED();
    return 1;
  //------BFO button ----------------------------------
  case K87EV_BBFO:
    SwapBFO();
    return 1;
  //-----Update timers --------------------------------
  case K87EV_BEAT:
    UpdateTimer();
    return 0;
  }
 return 0;
}
//------------------------------------------------------------------------
//  FREQUENCY  STATE
//------------------------------------------------------------------------
int CKR87radio::K87stateFRQ(K87_EVENT evn)
{ if (K87eventDEC(evn)) return 1;
  switch (evn)  {
  //--------Frequency button ----------------------------
  case K87EV_BFRQ:
    SwapFreq();
    return 1;
  //--------Timer button --------------------------------
  case K87EV_BFLT:
    K87enterTIM();
    return 1;
  case K87EV_SBY1:
    ModifyFreq(&SbyFRQ,(1000 * mDir));
    EditSbyFRQ();
    return 1;
  case K87EV_SBY2:
    ModifyFreq(&SbyFRQ,(100 * mDir));
    EditSbyFRQ();
    return 1;
  case K87EV_SBY3:
    ModifyFreq(&SbyFRQ,(10 * mDir));
    EditSbyFRQ();
    return 1;
  case K87EV_SBY4:
    ModifyFreq(&SbyFRQ,mDir);
    EditSbyFRQ();
    return 1;
  case K87EV_SBY5:
    ModifyFrac(&SbyFRQ,(mDir * 100));
    EditSbyFRQ();
    return 1;
  case K87EV_INNR:
    ModifyFreq(&SbyFRQ,(mDir * 1));
    EditSbyFRQ();
    return 1;
  case K87EV_OUTR:
    ModifyFreq(&SbyFRQ,(mDir * 100));
    EditSbyFRQ();
    return 1;
}
  return 1;
}
//--------------------------------------------------------------------
//  ENTER TIMER MODE 
//  Edit FLT timer
//--------------------------------------------------------------------
int CKR87radio::K87enterTIM()
{ adfTAB[KR87_FRQL_FD].state = RAD_ATT_INACT;     // Inhibit FREQ LED
  adfTAB[KR87_FLTL_FD].state = RAD_ATT_ACTIV;     // open    FLT  LED
  adfTAB[KR87_SBYF_D1].state = RAD_ATT_INACT;     // Inhibit SBY digit 1
  adfTAB[KR87_SBYF_D2].state = RAD_ATT_INACT;     // Inhibit SBY digit 2
  adfTAB[KR87_SBYF_D3].state = RAD_ATT_INACT;     // Inhibit SBY digit 3
  adfTAB[KR87_SBYF_D4].state = RAD_ATT_INACT;     // Inhibit SBY digit 4
  tMode   = 0;                                    // Mode FLT
  EditTimer();
  rState  = K87_TIM_ON;
  return 1;
}
//--------------------------------------------------------------------
//  LEAVE TIMER MODE
//--------------------------------------------------------------------
int CKR87radio::K87leaveTIM()
{ adfTAB[KR87_FLTL_FD].state = RAD_ATT_INACT;     // close    FLT  LED
  adfTAB[KR87_ETLD_FD].state = RAD_ATT_INACT;     // close    ELT  LED
  adfTAB[KR87_TIMN_FD].state = RAD_ATT_INACT;     // close    MIN  LED
  adfTAB[KR87_TISC_FD].state = RAD_ATT_INACT;     // close    SEC  LED
  if (tMode < 2)    return 1;
  uCount  = (2 == tMode)?(+1):(-1);               // Restart ELT
  return 1;
}
//--------------------------------------------------------------------
//  FLT EVENT
//--------------------------------------------------------------------
int CKR87radio::K87eventFLT()
{ switch(tMode) {
    //-----FLT MODE.  Switch to ELT mode ------------------------
    case 0:
      adfTAB[KR87_FLTL_FD].state = RAD_ATT_INACT;     // close    FLT  LED
      adfTAB[KR87_ETLD_FD].state = RAD_ATT_ACTIV;     // open     ET   LED
      tMode = 1;                                      // ET mode
      EditTimer();                                    // edit EL timer
      return 1;
    //-----ELT MODE.  Swicth to FLT mode ------------------------
    case 1:
      adfTAB[KR87_FLTL_FD].state = RAD_ATT_ACTIV;     // open    FLT  LED
      adfTAB[KR87_ETLD_FD].state = RAD_ATT_INACT;     // close   ELT  LED
      tMode = 0;                                      // ET mode
      EditTimer();                                    // edit FLT timer
      return 1;
    //----Reset mode.  Back to FLT mode ------------------------
    case 2:
      adfTAB[KR87_FLTL_FD].state = RAD_ATT_ACTIV;     // open    FLT  LED
      adfTAB[KR87_ETLD_FD].state = RAD_ATT_INACT;     // close   ELT  LED
      tMode = 0;                                      // ET mode
      EditTimer();                                    // edit FLT timer
      uCount  =+1;                                    // Allow for count up
    //---Programing mode.  Back to FLT mode -------------------
    case 3:
      adfTAB[KR87_FLTL_FD].state = RAD_ATT_ACTIV;     // open    FLT  LED
      adfTAB[KR87_ETLD_FD].state = RAD_ATT_INACT;     // close   ELT  LED
      tMode = 0;                                      // ET mode
      EditTimer();                                    // edit FLT timer
      uCount  =-1;                                    // Allow for count down
  }
  return 0;
}
//--------------------------------------------------------------------
//  Reset Event
//--------------------------------------------------------------------
int CKR87radio::K87eventRST()
{ switch (tMode)  {
    //-----FLT mode. Ignore reset-------------------------
    case 0:
      return 1;
    //----ELT mode.  Enter reset mode ------------------------
    case 1:
      eTime   = 0;
      uCount  = 0;
      tMode   = 2;
      EditTimer();
      return 1;
    //----ELT reset mode.  Restart in up count ---------------
    case 2:
      uCount  = +1;
      tMode   = 1;
      EditTimer();
      return 1;
    //---ELT programing mode.  Restart in down count ---------
    case 3:
      uCount  = -1;
      tMode   = 1;
      EditTimer();
      return 1;
  }
  return 0;
}
//--------------------------------------------------------------------
//  TIMER MODE 
//--------------------------------------------------------------------
int CKR87radio::K87stateTIM(K87_EVENT evn)
{ if (K87eventDEC(evn)) return 1;
  switch (evn)  {
    //---Modify timing value -------------------------------
    case K87EV_TMIN:
        ChangeTime(mDir * 60);
        return 1;
    case K87EV_TSEC:
        ChangeTime(mDir);
        return 1;
    //---Return to frequency mode --------------------------
    case K87EV_BFRQ:
        K87leaveTIM();
        K87enterFRQ();
        return 1;
    //--Display timer -------------------------------------
    case K87EV_BEAT:
        EditTimer();
        return 1;
    //--Swap timer display --------------------------------
    case K87EV_BFLT:
        K87eventFLT();
        return 1;
    //--Reset button --------------------------------------
    case K87EV_BRST:
        K87eventRST();
        return 1;
  }
  return 0;
}
//--------------------------------------------------------------------
//  DISPATCHER 
//--------------------------------------------------------------------
int CKR87radio::Dispatcher(K87_EVENT evn)            // Dispatching 
{ return (this->*adfn[rState])(evn);
}
//--------------------------------------------------------------------
//  Check for power ON 
//--------------------------------------------------------------------
bool CKR87radio::CheckPowerON()
{ if (0 == active)    return false;
  if (1 == sPower)    return true;
  if (0 == aPower)    return false;
  //--- Activate ADF mode ---------------------
  adfTAB[KR87_ADFL_FD].state = RAD_ATT_ACTIV;
  sPower  = 1;
  eTime   = 0;
  Timer   = 0;
  uCount  = +1;
  tMode   = 0;
  K87enterFRQ();
  return true;
}
//--------------------------------------------------------------------
//  Check for power OFF 
//--------------------------------------------------------------------
bool CKR87radio::CheckPowerOF()
{ if (active)     return false;
  K87enterPOF();
  return true;
}
//--------------------------------------------------------------------
//  TIME SLICE 
//  -Assert selected NDB
//  -Compute NDB direction = (NDB Radial - plane direction) (deg)
//  But for RH, plane direction should be inverted
//--------------------------------------------------------------------
void CKR87radio::TimeSlice (float dT,U_INT FrNo)
{  CRadio::TimeSlice (dT,FrNo);
  //---Update according to State --------------------------------------
  if ((K87_POW_OF == rState) && (!CheckPowerON())) return;
  if ((K87_POW_OF != rState) && ( CheckPowerOF())) return;
  mskFS = RadioMSK[globals->clk->GetON()];
  //----Refresh ADF station --------------------------------------------
  NDB = globals->dbc->GetTunedNDB(NDB,FrNo,ActFRQ.freq);
  float dir       = 0;
  if (NDB)		dir = NDB->GetRadial();         // Get the radial we are on (deg)
  dir             = Wrap360(dir + globals->dang.z);   // Point to NDB
  indnTarget      = dir;
  //----Compute second timer -------------------------------------------
  gTime += dT;
  if (gTime < 1.0)       return;
  gTime -= 1.0;
  (this->*adfn[rState])(K87EV_BEAT);                    // Dispatch BEAT EVENT
  return;
}

//==========================================================================
//
//  TRANSPONDER KT76
//  Radio part:
//
//==========================================================================
CKT76radio::CKT76radio()
{ TypeIs (SUBSYSTEM_TRANSPONDER_RADIO);
  hwId      = HW_RADIO;
  indnMode  = INDN_LINEAR;
  ratK      = 1;
  sPower    = 0;
  State     = XPD_POF;
  xnum      = 0;
  //-----Set the fields -----------------------------------------
  InhibitFields(xpdTAB,KT76CF_MAX);
  //-----Init all fields ----------------------------------------
  InitTable(xpdTAB,KT76_ALTS,aSign,0);  // Alt sign
  InitTable(xpdTAB,KT76_FLEV,aFlev,0);  // Flight level
  InitTable(xpdTAB,KT76_MODE,aMode,0);  // Flight level
  InitTable(xpdTAB,KT76_IDN1,aIdn1,0);  // Ident 1
  InitTable(xpdTAB,KT76_IDN2,aIdn2,0);  // Ident 2
  InitTable(xpdTAB,KT76_IDN3,aIdn3,0);  // Ident 3
  InitTable(xpdTAB,KT76_IDN4,aIdn4,0);  // Ident 4
  InitTable(xpdTAB,KT76_LAB1,aLab1,0);  // Ident 
  //-------------------------------------------------------------
  SetVFR();
}
//--------------------------------------------------------------------
//  Read parameters  
//--------------------------------------------------------------------
int CKT76radio::Read(SStream *st, Tag tag)
{
  switch (tag)  {
    case 'lspd':
      ReadFloat(&lspd,st);
      return TAG_READ;
    case 'uspd':
      ReadFloat(&uspd,st);
      return TAG_READ;
    case 'mAlt':
      ReadMessage(&alti,st);
      return TAG_READ;
  }
  return CRadio::Read(st,tag);
}
//--------------------------------------------------------------------
//  Set VFR identifier  
//--------------------------------------------------------------------
void CKT76radio::SetVFR()
{ ident[0]  = '1';
  ident[1]  = '2';
  ident[2]  = '0';
  ident[3]  = '0';
  return;
}
//--------------------------------------------------------------------
//  Set Ident ident 
//--------------------------------------------------------------------
void CKT76radio::SetIdent()
{ SetField(xpdTAB,KT76_IDN1,RAD_ATT_ACTIV,ident[0]);
  SetField(xpdTAB,KT76_IDN2,RAD_ATT_ACTIV,ident[1]);
  SetField(xpdTAB,KT76_IDN3,RAD_ATT_ACTIV,ident[2]);
  SetField(xpdTAB,KT76_IDN4,RAD_ATT_ACTIV,ident[3]);
  return;
}
//--------------------------------------------------------------------
//  Clear ident 
//--------------------------------------------------------------------
int CKT76radio::IdentVFR()
{ if (State < XPD_PON)  return 0;
  SetVFR();
  SetIdent();
  return 1;
}
//--------------------------------------------------------------------
//  Enter Digit 
//--------------------------------------------------------------------
int CKT76radio::NewDigit(int bvl)
{ if (State < XPD_PON)  return 0;
  ident[xnum++] = '0' + bvl;
  xnum &= 0x03;
  SetIdent();
  return 1;
}
//--------------------------------------------------------------------
//  Update Button position 
//--------------------------------------------------------------------
int CKT76radio::UpdButton()
{ U_CHAR old = State;
  State += mDir;
  if (State > XPD_ALT) State = XPD_ALT;
  if (State < XPD_POF) State = XPD_POF;
  if (State == old)      return 0;
  //----Check new position  ----------------------------------
  if (XPD_POF == State)  return EnterPOF();
  if (XPD_SBY == State)  return EnterSBY();
  if (XPD_TST == State)  return EnterTST();
  if (XPD_PON == State)  return EnterPON();
  if (XPD_ALT == State)  return EnterALT();
  return 0;
}
//--------------------------------------------------------------------
//  Enter Power off: 
//--------------------------------------------------------------------
int CKT76radio::EnterPOF()
{ sPower  = 0;
  State   = XPD_POF;
  return 0;
}
//--------------------------------------------------------------------
//  Enter Standby: 
//--------------------------------------------------------------------
int CKT76radio::EnterSBY()
{ State   = XPD_SBY;
  InhibitFields(xpdTAB,KT76CF_MAX);
  SetField(xpdTAB,KT76_MODE,RAD_ATT_FLASH,"SBY");
  return 1;
}
//--------------------------------------------------------------------
//  Enter Test mode: 
//--------------------------------------------------------------------
int CKT76radio::EnterTST()
{ State   = XPD_TST;
  SetField(xpdTAB,KT76_ALTS,RAD_ATT_FLASH,'-');
  SetField(xpdTAB,KT76_FLEV,RAD_ATT_FLASH,"888");
  SetField(xpdTAB,KT76_MODE,RAD_ATT_FLASH,"TST");
  SetField(xpdTAB,KT76_IDN1,RAD_ATT_FLASH,'8');
  SetField(xpdTAB,KT76_IDN2,RAD_ATT_FLASH,'8');
  SetField(xpdTAB,KT76_IDN3,RAD_ATT_FLASH,'8');
  SetField(xpdTAB,KT76_IDN4,RAD_ATT_FLASH,'8');
  SetField(xpdTAB,KT76_LAB1,RAD_ATT_FLASH,"FL");
  return 1;
}
//--------------------------------------------------------------------
//  Enter On mode: 
//--------------------------------------------------------------------
int CKT76radio::EnterPON()
{ State   = XPD_PON;
  OffField(xpdTAB,KT76_ALTS);
  OffField(xpdTAB,KT76_FLEV);
  SetField(xpdTAB,KT76_MODE,RAD_ATT_ACTIV,"ON");
  SetIdent();
  OffField(xpdTAB,KT76_LAB1);
  return 1;
}
//--------------------------------------------------------------------
//  Enter ALT mode: 
//--------------------------------------------------------------------
int CKT76radio::EnterALT()
{ State   = XPD_ALT;
  SetField(xpdTAB,KT76_MODE,RAD_ATT_ACTIV,"ALT");
  SetField(xpdTAB,KT76_FLEV,RAD_ATT_ACTIV,"000");
  SetIdent();
  SetField(xpdTAB,KT76_LAB1,RAD_ATT_ACTIV,"FL");
  return 1;
}
//--------------------------------------------------------------------
//  Power off:  Check for Power ON 
//--------------------------------------------------------------------
int CKT76radio::StatePOF(KT76events evn)
{ if (!active)            return 0;
  if (KT76EV_PWR != evn)  return 0;
  if (mDir != +1)         return 0;
  //---Enter Standby state ----------------------
  sPower  = 1;
  EnterSBY();
  return 1;
}
//--------------------------------------------------------------------
//  Modify a digit 
//--------------------------------------------------------------------
int CKT76radio::ModDigit(int xd)
{ if (State < XPD_PON)  return 0;
  char bvl = ident[xd] - '0';             // binary value 
  bvl += mDir;                            // Increment
  if (-1 == bvl)  bvl = 7;
  if (+8 == bvl)  bvl = 0;
  ident[xd] = '0' | bvl;
  SetIdent();
  return 1;
}
//--------------------------------------------------------------------
//  Clear ident 
//--------------------------------------------------------------------
int CKT76radio::ClearIDN()
{ if (State < XPD_PON)  return 0;
  ident[0] = '0';
  ident[1] = '0';
  ident[2] = '0';
  ident[3] = '0';
  SetIdent();
  xnum     = 0;
  return 1;
}
//--------------------------------------------------------------------
//  Edit Flight level 
//--------------------------------------------------------------------
void CKT76radio::EditFLEV(float alt)
{ int lev = int(alt * 0.01f);
  _snprintf(aFlev,6,"%03d",lev);
  return;
}
//--------------------------------------------------------------------
//  DISPATCHER 
//--------------------------------------------------------------------
int CKT76radio::Dispatcher(KT76events evn,int pm)            // Dispatching 
{ if (0 == sPower)        return StatePOF(evn);
  if (KT76EV_PWR == evn)  return UpdButton();
  if (KT76EV_DIG == evn)  return ModDigit(pm - KT76BZ_DIG);
  if (KT76EV_CLR == evn)  return ClearIDN();
  if (KT76EV_VFR == evn)  return IdentVFR();
  if (KT76EV_BTN == evn)  return NewDigit(pm - KT76BZ_BTN);
  return 1;
}
//-------------------------------------------------------------------------
//  Receive message from external component
//-------------------------------------------------------------------------
EMessageResult CKT76radio::ReceiveMessage (SMessage *msg)
{ U_CHAR opt = 0;
  if  (msg->id == MSG_GETDATA) {
    switch (msg->user.u.datatag) {
      //--- Request a pointer to this subsystem --------
      case 'gets':
        msg->voidData = this;
        return MSG_PROCESSED;

      }
      return  CRadio::ReceiveMessage (msg);
  }
      //=======================================
return  CRadio::ReceiveMessage (msg);
}
//-------------------------------------------------------------------------
//  Time Slice
//-------------------------------------------------------------------------
void CKT76radio::TimeSlice(float dT,U_INT FrNo)
{ CDependent::TimeSlice(dT,FrNo);
  if (!active)  {EnterPOF(); return;}
  //---Update flasher --------------------------------------------------
  mskFS = RadioMSK[globals->clk->GetON()];
  if (State != XPD_ALT)     return;
  //---Get altitude -----------------------------------------------------
  Send_Message(&alti);
  EditFLEV(alti.realData);
  return;
}
//==========================================================================
//  Normal state
//==========================================================================
RADIO_HIT KT76_Flds[] = {
  //----Transponder digits ----------
  {RADIO_CA01,KT76EV_DIG},
  {RADIO_CA02,KT76EV_DIG},
  {RADIO_CA03,KT76EV_DIG},
  {RADIO_CA04,KT76EV_DIG},
  //----SQWAK IDENT -----------------
  {RADIO_CA05,KT76EV_IDN},
  //----BUTTON 0 to 7 ---------------
  {RADIO_CA06,KT76EV_BTN},        // BT0
  {RADIO_CA07,KT76EV_BTN},        // BT1
  {RADIO_CA08,KT76EV_BTN},        // BT2
  {RADIO_CA09,KT76EV_BTN},        // BT3
  {RADIO_CA10,KT76EV_BTN},        // BT4
  {RADIO_CA11,KT76EV_BTN},        // BT5
  {RADIO_CA12,KT76EV_BTN},        // BT6
  {RADIO_CA13,KT76EV_BTN},        // BT7
  //----CLEAR ----------------------
  {RADIO_CA14,KT76EV_CLR},        // CLR
  //----VFR ------------------------
  {RADIO_CA15,KT76EV_VFR},        // VFR
  //--- Power and mode -----------------
  {RADIO_CA16,KT76EV_PWR},        // BT7
  {0,0},
};


//==================================END OF FILE========================================