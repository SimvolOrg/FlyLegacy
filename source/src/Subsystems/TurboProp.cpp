/*
 * TurboProp.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
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

#include "../Include/TurboProp.h"

using namespace std;
//===========================================================================================================
//==================================================================================
//    Constructor
//==================================================================================
CTurboProp::CTurboProp(int engn)
{}
//------------------------------------------------------------------------------
// The main purpose of Calculate() is to determine what phase the engine should
// be in, then call the corresponding function.
//------------------------------------------------------------------------------
double CTurboProp::Calculate(void)
{/*
  TAT = (Auxiliary->GetTotalTemperature() - 491.69) * 0.5555556;
  dt = State->Getdt() * Propulsion->GetRate();

  ThrottleCmd = FCS->GetThrottleCmd(EngineNumber);

  Prop_RPM = Thruster->GetRPM() * Thruster->GetGearRatio();
  if (Thruster->GetType() == FGThruster::ttPropeller) {
    ((FGPropeller*)Thruster)->SetAdvance(FCS->GetPropAdvance(EngineNumber));
    ((FGPropeller*)Thruster)->SetFeather(FCS->GetPropFeather(EngineNumber));
    ((FGPropeller*)Thruster)->SetReverse(Reversed);
    if (Reversed) {
      ((FGPropeller*)Thruster)->SetReverseCoef(ThrottleCmd);
    } else {
      ((FGPropeller*)Thruster)->SetReverseCoef(0.0);
    }
  }

  if (Reversed) {
    if (ThrottleCmd < BetaRangeThrottleEnd) {
        ThrottleCmd = 0.0;  // idle when in Beta-range
    } else {
      // when reversed:
      ThrottleCmd = (ThrottleCmd-BetaRangeThrottleEnd)/(1-BetaRangeThrottleEnd) * ReverseMaxPower;
    }
  }

  // When trimming is finished check if user wants engine OFF or RUNNING
  if ((phase == tpTrim) && (dt > 0)) {
    if (Running && !Starved) {
      phase = tpRun;
      N2 = IdleN2;
      N1 = IdleN1;
      OilTemp_degK = 366.0;
      Cutoff = false;
    } else {
      phase = tpOff;
      Cutoff = true;
      Eng_ITT_degC = TAT;
      Eng_Temperature = TAT;
      OilTemp_degK = TAT+273.15;
    }
  }

  if (!Running && Starter) {
    if (phase == tpOff) {
      phase = tpSpinUp;
      if (StartTime < 0) StartTime=0;
    }
  }
  if (!Running && !Cutoff && (N1 > 15.0)) {
    phase = tpStart;
    StartTime = -1;
  }
  if (Cutoff && (phase != tpSpinUp)) phase = tpOff;
  if (dt == 0) phase = tpTrim;
  if (Starved) phase = tpOff;
  if (Condition >= 10) {
    phase = tpOff;
    StartTime=-1;
  }

  if (Condition < 1) {
    if (Ielu_max_torque > 0
      && -Ielu_max_torque > ((FGPropeller*)(Thruster))->GetTorque()
      && ThrottleCmd >= OldThrottle ) {
      ThrottleCmd = OldThrottle - 0.1 * dt; //IELU down
      Ielu_intervent = true;
    } else if (Ielu_max_torque > 0 && Ielu_intervent && ThrottleCmd >= OldThrottle) {
      ThrottleCmd = OldThrottle;
      ThrottleCmd = OldThrottle + 0.05 * dt; //IELU up
      Ielu_intervent = true;
    } else {
      Ielu_intervent = false;
    }
  } else {
    Ielu_intervent = false;
  }
  OldThrottle = ThrottleCmd;

  switch (phase) {
    case tpOff:    Eng_HP = Off(); break;
    case tpRun:    Eng_HP = Run(); break;
    case tpSpinUp: Eng_HP = SpinUp(); break;
    case tpStart:  Eng_HP = Start(); break;
    default: Eng_HP = 0;
  }

  //printf ("EngHP: %lf / Requi: %lf\n",Eng_HP,Prop_Required_Power);
  return Thruster->Calculate((Eng_HP * hptoftlbssec)-Thruster->GetPowerRequired());
  */
  return 0;
}


//====================END OF FILE ============================================================================