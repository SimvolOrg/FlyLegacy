/*
 * WindowOptions.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2009 Chris Wallace
 * Copyright 2007 Jean Sabatier
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
#include "../Include/Fui.h"
#include "../Include/FuiUser.h"
//================================================================================================
WORD Ramp[3][256];                                    // Current ramp
HDC  hdc = 0;                                           // Current HDC
//========================================================================================
//  Window for global options
//========================================================================================
CFuiOptions::CFuiOptions(Tag idn, const char *filename)
:CFuiWindow(idn,filename,280,200,0)
{ gBOX  = new CFuiCheckBox(10,8,260,60,this);
  AddChild('gcor',gBOX,"Gamma Correction");
  //--------------------------------------------------------
  if (CheckNoFile())  SaveGamma();
  SetGamma(2.2);
  ReadFinished();
}
//-----------------------------------------------------------------------------------
//  Check if gamma ramp is already saved
//-----------------------------------------------------------------------------------
bool CFuiOptions::CheckNoFile()
{ char *fn = "System/Gamma.cof";
  FILE* f = fopen (fn, "r");
  if (!f) return true;
  fclose(f);
  return false;
}
//-----------------------------------------------------------------------------------
//  Save current gamma ramp
//-----------------------------------------------------------------------------------
void CFuiOptions::SaveGamma()
{ char *fn = "System/Gamma.cof";
  char *er = "Can't save gamma ramp";
  FILE* f = fopen (fn, "w");
  if (!f) gtfo(er);
  //--------------------------------------------------------------
  hdc = wglGetCurrentDC();
  if (!GetDeviceGammaRamp (hdc, Ramp)) gtfo(er);
  //--- Save it in file ------------------------------------------
  for (int k=0; k<256; k++)
  { fprintf(f,"%03d R%05d G%05d B%05d \n",k,Ramp[0][k],Ramp[1][k],Ramp[2][k]); }
  fprintf(f,"===END===\n");
  fclose(f);
  return;
}
//-----------------------------------------------------------------------------------
//  Change current gamma ramp
//-----------------------------------------------------------------------------------
void CFuiOptions::SetGamma(double g)
{ // Calculate gamma ramp table
    double invgamma = 1.0 / g;
    WORD ramp[3][0x100];
    for (int i=0; i<0x100; i++) {
      float factor = (float)pow((double)i / 256.0, invgamma);
      if (factor > 1.0f) factor = 1.0f;
      WORD value = (WORD)((factor * 65535.0f) + 0.5f);
      ramp[0][i] = value;
      ramp[1][i] = value;
      ramp[2][i] = value;
    }

    // Set the new gamma ramp
    BOOL ok = SetDeviceGammaRamp (hdc, ramp);
    return;
}
//==========================END OF FILE ==========================================================
