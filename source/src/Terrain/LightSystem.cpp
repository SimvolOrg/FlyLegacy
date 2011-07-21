//=====================================================================================================
//	Light system MANAGEMENT
//=====================================================================================================
/*
 * lightSystem.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
 * Copyright 2008      Jean Sabatier
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
#include "../Include/Terraintexture.h"
#include "../Include/Database.h"
#include "../Include/LightSystem.h"
using namespace std;
//=============================================================================
//=============================================================================
double disVL  = 4;
double diamVL = 1.4f;
//=============================================================================
//  Spot table
//=============================================================================
C3_VTAB  spotTAB[4] = {
	//---S----T----R----G----B----X----Y-------Z----
	{    0,   0,   1,   1,   1,  -diamVL,   disVL, -diamVL},							// SW corner
	{		 1,   0,   1,   1,   1,  +diamVL,   disVL, -diamVL},							// SE corner
	{		 1,   1,   0,   0,   0,  +diamVL,   disVL, +diamVL},							// NE corner
	{		 0,   1,   0,   0,   0,  -diamVL,   disVL, +diamVL},							// NW corner
};
//=============================================================================
// Spot extremity
//=============================================================================
CVector spotINR(0, disVL, -1);			// Inner ray
CVector spotOUT(0, disVL, +1);			// Outer ray
//=============================================================================
//  Light color table (indexed by TC_XXX_LIT
//=============================================================================
TC_COLOR ColorTAB[] = {
  {0,0,0,1},                                  // 0 No color
  {1,1,1,1},                                  // 1 WHITE
  {1,0,0,1},                                  // 2 RED
  {0,1,0,1},                                  // 3 GREEN
  {0,0,1,1},                                  // 4 BLUE
  {1,1,0,1},                                  // 5 YELLOW
};
//=============================================================================
//  Light type for ILS
//=============================================================================
U_CHAR ilsCAT[] = {
  0,                              // 0 =>No ILS
  TC_LITE_HICAT,                  // 1 => one ILS at Hi end
  TC_LITE_LOCAT,                  // 2 => One ILS at Lo end
  TC_LITE_HICAT + TC_LITE_LOCAT,  // 3 => both
};
//=============================================================================
//  PAPI decision table
//  Gives the TAN(L)² for each light
//=============================================================================
double valPAPI[] = {
  PAPI_LA,
  PAPI_LB,
  PAPI_LC,
  PAPI_LD,
};
//=============================================================================
//  APAPI decision table
//  Gives the TAN(L)² for each light
//=============================================================================
double valAPAPI[] = {
  APAPI_LA,
  APAPI_LB,
};
//=============================================================================
//  Profile Tags
//=============================================================================
//----APPROACH LIGHTS ----------
char *aprsTAG[] = {
  "none",
  "REIL",
  "ODAL",
  "ALSR",
  "ALF1",
  "ALF2",
};
//----TBAR LIGHT ------------------
char *tbarTAG[] = {
  "none",
  "Gend",                 // Green only
  "Rend",                 // Red only
  "2end",                 // Both ends
  "inop",                 // No operation
};
//----YES/NO OPTION ---------------
char *yenoTAG[] = {
  "none",
  "yes_",
};
//----Instrument runways -----------
char *instTAG[] = {
  "none",
  "mono",
  "inst",
};
//----PAPI system ------------------
char *papiTAG[] =  {
   "none",
   "2lit",
   "4lit",
};
//============================================================================
//  Change Alpha channel for all colors
//============================================================================
void SetAlphaColor(float alpha)
{ for (int k=0; k < TC_COLOR_MAX;k++)  ColorTAB[k].alpha = alpha;
  return;
}
//============================================================================
//  reset Alpha color
//============================================================================
void ResetAlphaColor()
{ for (int k= 0; k < TC_COLOR_MAX;k++)  ColorTAB[k].alpha = 1;
  return;
}
//=========================================================================================
//  Destroy Pavement light Q
//=========================================================================================
CLiteQ::~CLiteQ()
{ CBaseLITE *lit = Pop();
  while (lit) { delete lit; lit = Pop();}
}

//============================================================================
//  LIGHT SYSTEM
//============================================================================
CLitSYS::CLitSYS()
{ atn   = 0.010f;
  mins  = 1.0f;
  Sig1  = 1;
  Sig2  = 0;
}
//============================================================================
//  LightSystem Draw all light combination
//  pc is the camera position
//  NOTE: When signal 1 is 0, then no ligth is drawed.
//        sig1 may be used as a general on/off switch or as a flasher.
//============================================================================
void CLitSYS::DrawSystem(SVector &pc)
{ if (0 == Sig1)  return;
  //----------------------------------------------------
  CBaseLITE *lit  = litQ.GetFirst();   // Light descriptor
  while (lit)
  { lit->DrawSpot(pc);
    lit   = litQ.GetNext(lit);
  }
  //----------------------------------------------------
  return;
}
//----------------------------------------------------------------------------
//	Init drawing
//----------------------------------------------------------------------------
void CLitSYS::StartDraw()
{	GLfloat p[] = {0.0f,0.01f,0.00001f};
  glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION,p);
  glPointParameterf (GL_POINT_SIZE_MIN,1);
  glPointParameterf (GL_POINT_SIZE_MAX,64);
  glPointSize(64);
  glEnable(GL_POINT_SPRITE);
  glTexEnvi(GL_POINT_SPRITE,GL_COORD_REPLACE,GL_TRUE);
}
//----------------------------------------------------------------------------
//	End drawing
//----------------------------------------------------------------------------
void CLitSYS::EndDraw()
{	glTexEnvi(GL_POINT_SPRITE,GL_COORD_REPLACE,GL_FALSE);
  glDisable(GL_POINT_SPRITE);
}
//----------------------------------------------------------------------------
//  3D Objects: Draw the requested spot with this system parameters
//  cam is the camera position
//----------------------------------------------------------------------------
void CLitSYS::DrawSpot(SVector &cam,CBaseLITE *lit)
{ GLfloat p[] = {0.0f,atn,0.00001f};
  glEnable(GL_BLEND);
  glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION,p);
  glPointParameterf (GL_POINT_SIZE_MIN,mins);
  glPointParameterf (GL_POINT_SIZE_MAX,64);
  glPointSize(64);
  glEnable(GL_POINT_SPRITE);
  glTexEnvi(GL_POINT_SPRITE,GL_COORD_REPLACE,GL_TRUE);
  //----------------------------------------------------
  lit->DrawSpot(cam);
  //----------------------------------------------------
  glTexEnvi(GL_POINT_SPRITE,GL_COORD_REPLACE,GL_FALSE);
  glDisable(GL_POINT_SPRITE);
  glDisable(GL_BLEND);
  return;
}
//============================================================================
//
//  Class CBaseLITE to store pavement light descriptors
//          ALL COORDINATES ARE ARCSEC RELATIVE TO AIRPORT ORIGIN
//============================================================================
CBaseLITE::CBaseLITE(U_CHAR tp,U_SHORT nb)
{ ltyp  = tp;
  col1  = 0;
  col2  = 0;
  ntex  = 0;
  nbl   = nb;
  tab   = (nb)?(new TC_VTAB[nb]):(0);
}
//---------------------------------------------------------------------------
//  Free the descriptor
//---------------------------------------------------------------------------
CBaseLITE::~CBaseLITE()
{ if (tab)  delete [] tab;
}
//---------------------------------------------------------------------------
//  Copy spots from source src from starting position No
//---------------------------------------------------------------------------
void CBaseLITE::Append(CBaseLITE *lit)
{ TC_VTAB  *dst = tab + nbl;                 // Destination slot
  TC_VTAB  *src = lit->GetVTAB();
  for (int k = 0; k != lit->GetNbSpot(); k++) *dst++ = *src++;
  nbl += lit->GetNbSpot();
  return;
}
//---------------------------------------------------------------------------
//  Draw a ramp of light points
//---------------------------------------------------------------------------
void CBaseLITE::DrawAll(U_CHAR No)
{ if (0 == No)  return;
  GLfloat *col = (GLfloat *)&ColorTAB[No];
  glColor4fv(col);
  glMaterialfv (GL_FRONT, GL_EMISSION, col);
  glInterleavedArrays(GL_T2F_V3F,0,tab);
  glDrawArrays(GL_POINTS,0,nbl);
  return;
}
//---------------------------------------------------------------------------
//  Draw one light point
//---------------------------------------------------------------------------
void CBaseLITE::DrawOne(int k,U_CHAR No)
{ if (0 == No)  return;
  GLfloat *col = (GLfloat *)&ColorTAB[No];
  glColor4fv(col);
  glMaterialfv (GL_FRONT, GL_EMISSION, col);
  glInterleavedArrays(GL_T2F_V3F,0,tab);
  glDrawArrays(GL_POINTS,k,1);
  return;
}

//---------------------------------------------------------------------------
//  Draw according to type
//  cam:  Camera position
//---------------------------------------------------------------------------
void CBaseLITE::DrawSpot(SVector &cam)
{ glBindTexture(GL_TEXTURE_2D,globals->txw->GetLiteTexture(ntex));
  switch (ltyp)  {
    case LS_OMNI_LITE:
      DrawAll(col1);
      return;
    //---Flasher -------------------------------------
    case LS_FLSH_LITE:
      ((CFlshLITE*)this)->DrawFlash();
      return;
    //---Double dual color light ---------------------
    case LS_1122_LITE:
      ((CDualLITE*)this)->DrawT1122(cam);
      return;
    //---Alternate dual color light variante 1--------
    case LS_1121_LITE:
      ((CDualLITE*)this)->DrawT1121(cam);
      return;
    //---Alternate dual color light variante 2--------
    case LS_2111_LITE:
      ((CDualLITE*)this)->DrawT2111(cam);
      return;
    //---BAR of 2 color light ------------------------
    case LS_BAR2_LITE:
      ((CDualLITE*)this)->DrawTBAR2(cam);
      return;
    //---Strobe light --------------------------------
    case LS_STRB_LITE:
      ((CStrobeLITE*)this)->DrawFlash(cam);
      return;
    //---Flash bar -----------------------------------
    case LS_FBAR_LITE:
      ((CFlashBarLITE*)this)->DrawFlash(cam);
      return;
    //---PAPI light ----------------------------------
    case LS_PAPI_LITE:
      ((CPapiLITE*)this)->DrawPAPI(cam);
      return;
    }
return;
}
//----------------------------------------------------------------------------
//  Pre compute cross product AM*AB
//  Compute B relative to A
//----------------------------------------------------------------------------
void CBaseLITE::PreCalculate()
{double xm = side.po.x - side.pa.x;
 double ym = side.po.y - side.pa.y;
 double xb = side.pb.x - side.pa.x;
 double yb = side.pb.y - side.pa.y;
 //-------------------------------------
 side.sm   = (xm * yb) - (ym * xb);
 //--Save B coordinates relative to A --
 side.pb.x = xb;
 side.pb.y = yb;
 return;
}
//----------------------------------------------------------------------------
//  Check if the Camera C is on same side as Token point dl
//  from line AB
//  Sm contains the cross product AM*AB
//  Pb coordinates are relative to A
//  pc is the camera cooordinate
//  -----------------------------------------------------
//  Express C relative to A and compute dc  = AC.AB
//  Check sign of sm * dc;
//----------------------------------------------------------------------------
bool CBaseLITE::SameSide(SIDE_POINT &sp,SVector &pc)
{ //----compute AC.AB --------------
  double xc = pc.x    - sp.pa.x;
  double yc = pc.y    - sp.pa.y;
  double dc = (xc * sp.pb.y) - (yc * sp.pb.x);
  return ((dc * sp.sm) >= 0);
}
//----------------------------------------------------------------------------
//  Draw a square for test. HELP TO LOCALIZATION
//----------------------------------------------------------------------------
void CBaseLITE::TestDraw(int k,int mode)
{ float    ech = (mode == 0)?(1):(TC_ARCS_FROM_FEET(1));
  TC_VTAB &ent = tab[k];
  TC_VTAB sw;
  TC_VTAB nw;
  TC_VTAB ne;
  TC_VTAB se;
  float size = 8;
  sw.VT_X = ent.VT_X - (ech * size);
  sw.VT_Y = ent.VT_Y - (ech * size);
  sw.VT_Z = ent.VT_Z;


  nw.VT_X = ent.VT_X - (ech * size);
  nw.VT_Y = ent.VT_Y + (ech * size);
  nw.VT_Z = ent.VT_Z;


  ne.VT_X = ent.VT_X + (ech * size);
  ne.VT_Y = ent.VT_Y + (ech * size);
  ne.VT_Z = ent.VT_Z;


  se.VT_X = ent.VT_X + (ech * size);
  se.VT_Y = ent.VT_Y - (ech * size);
  se.VT_Z = ent.VT_Z;

float modelview[16];
int i,j;

// save the current modelview matrix
glPushMatrix();

// get the current modelview matrix
glGetFloatv(GL_MODELVIEW_MATRIX , modelview);

// undo all rotations
// beware all scaling is lost as well 
for( i=0; i<3; i++ ) 
	for( j=0; j<3; j++ ) {
		if ( i==j )
			modelview[i*4+j] = 1.0;
		else
			modelview[i*4+j] = 0.0;
	}

	// set the modelview with no rotations and scaling
	glLoadMatrixf(modelview);


	// restores the modelview matrix

  glDepthMask(GL_FALSE);
  glBindTexture(GL_TEXTURE_2D,globals->txw->GetLiteTexture(0));
  GLfloat *col = (GLfloat *)&ColorTAB[1];
  glMaterialfv (GL_FRONT, GL_EMISSION, col);
  glColor3fv(col);
  glEnable(GL_BLEND);

  glBegin(GL_QUADS);
  glTexCoord2f(0,0);
  glVertex3d(sw.VT_X,sw.VT_Y,sw.VT_Z);
  glTexCoord2f(0,1);
  glVertex3d(se.VT_X,se.VT_Y,se.VT_Z);
  glTexCoord2f(1,1);
  glVertex3d(ne.VT_X,ne.VT_Y,ne.VT_Z);
  glTexCoord2f(1,0);
  glVertex3d(nw.VT_X,nw.VT_Y,nw.VT_Z);
  glEnd();

  glPopMatrix();
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor3f(1,1,1);
  return;
}

//============================================================================
//
//  Class CDualLITE to store dual light descriptors
//
//============================================================================
//----------------------------------------------------------------------------
//  Draw as a type T1122
//  2 bicolored lights 
//----------------------------------------------------------------------------
void CDualLITE::DrawT1122(SVector &pc)
{ U_CHAR No = (SameSide(side,pc))?(col1):(col2);
  if (0 == No) return;
  GLfloat *col = (GLfloat *)&ColorTAB[No];
  glColor4fv(col);
  glMaterialfv (GL_FRONT, GL_EMISSION, col);
  glInterleavedArrays(GL_T2F_V3F,0,&LT[0]);
  glDrawArrays(GL_POINTS,0,2);
  return;
}
//----------------------------------------------------------------------------
//  Draw as a type T1233
//  2 alternte bicolored lights on one side
//  If camera on side of decision point and colo1: => Both lights in color 1
//  if (color 2) =>L0 in color 2
//  if (color 1) =>L1 in color 1
//----------------------------------------------------------------------------
void CDualLITE::DrawT1121(SVector &pc)
{ bool      sd = SameSide(side,pc);
  GLfloat *col = 0;
  //---Both lights in color 1 --
  if (sd && col1)
      { col = (GLfloat *)&ColorTAB[col1];
        glColor4fv(col);
        glMaterialfv (GL_FRONT, GL_EMISSION, col);
        glInterleavedArrays(GL_T2F_V3F,0,&LT[0]);
        glDrawArrays(GL_POINTS,0,2);
        return;
      }
  //---L0 in color 2 ------------
  if (col2)                               
     {  col = (GLfloat *)&ColorTAB[col2];
        glColor4fv(col);
        glMaterialfv (GL_FRONT, GL_EMISSION, col);
        glInterleavedArrays(GL_T2F_V3F,0,&LT[0]);
        glDrawArrays(GL_POINTS,0,1);
      }
  //---L1 in color 1 -----------
  if (col1)                               
      { col = (GLfloat *)&ColorTAB[col1];
        glColor4fv(col);
        glMaterialfv (GL_FRONT, GL_EMISSION, col);
        glInterleavedArrays(GL_T2F_V3F,0,&LT[1]);
        glDrawArrays(GL_POINTS,0,1);
        return;
      }
  return;
}
//----------------------------------------------------------------------------
//  Draw as a type T2111
//  2 alternte bicolored lights on one side
//----------------------------------------------------------------------------
void CDualLITE::DrawT2111(SVector &pc)
{ bool      sd = SameSide(side,pc);
  GLfloat *col = 0;
  //---L0 in color 2 ----------------------------
  if (sd && col2)
      { col = (GLfloat *)&ColorTAB[col2];
        glColor4fv(col);
        glMaterialfv (GL_FRONT, GL_EMISSION, col);
        glInterleavedArrays(GL_T2F_V3F,0,&LT[0]);
        glDrawArrays(GL_POINTS,0,1);
      }
  //--L1 in color 1 -----------------------------
  if (sd && col1)
      { col = (GLfloat *)&ColorTAB[col1];
        glColor4fv(col);
        glMaterialfv (GL_FRONT, GL_EMISSION, col);
        glInterleavedArrays(GL_T2F_V3F,0,&LT[1]);
        glDrawArrays(GL_POINTS,0,1);
      }
  if (sd)   return;
  //--Both L0 L1 in color 1-----------------------
  if (col1)
      { col = (GLfloat *)&ColorTAB[col1];
        glColor4fv(col);
        glMaterialfv (GL_FRONT, GL_EMISSION, col);
        glInterleavedArrays(GL_T2F_V3F,0,&LT[0]);
        glDrawArrays(GL_POINTS,0,2);
      }

  return;
}
//----------------------------------------------------------------------------
//  Draw as a type H2211
//  Bar of Dual lights
//----------------------------------------------------------------------------
void CDualLITE::DrawTBAR2(SVector &pc)
{ int      inx = (SameSide(side,pc))?(col1):(col2);
  if (0 == inx) return;
  GLfloat *col = (GLfloat *)&ColorTAB[inx];
  glColor4fv(col);
  glMaterialfv (GL_FRONT, GL_EMISSION, col);
  glInterleavedArrays(GL_T2F_V3F,0,tab);
  glDrawArrays(GL_POINTS,0,nbl);
  return;
}
//============================================================================
//  FLASHER LIGHT
//============================================================================
CFlshLITE::CFlshLITE() : CBaseLITE(LS_FLSH_LITE,1)
{ Time    = 0;
  TLim[0] = 0;
  TLim[1] = 0;
  State   = 0;
}
//----------------------------------------------------------------------------
//  Update state from time control
//----------------------------------------------------------------------------
void CFlshLITE::UpdateState()
{ Time += globals->tcm->Elapse();
  if (Time < TLim[State]) return;
  Time   = 0;
  State ^= 1;
  return;
}
//----------------------------------------------------------------------------
//  Draw as a flasher
//----------------------------------------------------------------------------
void CFlshLITE::DrawFlash()
{ //---Update flash time ---------------------------
  UpdateState();
  if ((1 == State) && col1)  DrawAll(col1);
  if ((0 == State) && col2)  DrawAll(col2);
  return;
}
//=========================================================================================
//  STROBE SEQUENCE 
//
//=========================================================================================
CStrobeLITE::CStrobeLITE(U_CHAR type,U_SHORT nb): CBaseLITE(type,nb)
{ seq = nb - 1;
  lim = 0;
  Time = 0;
}
//----------------------------------------------------------------------------
//  Update sequence
//----------------------------------------------------------------------------
void CStrobeLITE::UpdateState()
{ Time += globals->tcm->Elapse();
  if (Time < eTim[lim]) return;
  Time = 0;
  if (1 == lim)   {lim = 0; seq = nbl;}
  seq--;
  if (seq == 0) lim = 1;
  return;
}
//---------------------------------------------------------------------------
//  Draw the system as a strobe light
//  seq is the number of the current flash along the lane. The ligth is L(s)
//  For L(s) we compute the decision line AB orthogonal to the lane.
//  The decision line is derived by translation (-ad.x,+ad.y)
//  B is invariant in this translation
//---------------------------------------------------------------------------
void CStrobeLITE::DrawFlash(SVector &cam)
{ UpdateState();
  for (int k=0; k<nbl; k++)
  { if (k != seq) continue;
    //---check for visibility ------------------
    TC_VTAB *ent = tab + k;
    side.pa.x   = ent->VT_X - ad.x;
    side.pa.y   = ent->VT_Y + ad.y;
    //---Compute SM ----------------------------
    double xb   = side.pb.x;
    double yb   = side.pb.y;
    double xm   = side.po.x - side.pa.x;
    double ym   = side.po.y - side.pa.y;
    //------------------------------------------
    side.sm     = (xm * yb) - (ym * xb);
    if (SameSide(side,cam)) continue;
    DrawOne(seq,col1);
  }
  return;
}
//=========================================================================================
//  STROBE BAR with syncho flashes
//
//=========================================================================================
CFlashBarLITE::CFlashBarLITE(U_CHAR type,U_SHORT nb): CBaseLITE(type,nb)
{ lim  = 0;                    // On timer
  sta  = 1;                    // On state
  Time = 0;
}
//----------------------------------------------------------------------------
//  Update sequence
//----------------------------------------------------------------------------
void CFlashBarLITE::UpdateState()
{ Time += globals->tcm->Elapse();
  if (Time < eTim[lim]) return;
  Time = 0;
  lim ^= 1;                     // Change timer
  sta ^= 1;                     // Change state
  return;
}
//---------------------------------------------------------------------------
//  Draw the system as a strobe light
//---------------------------------------------------------------------------
void CFlashBarLITE::DrawFlash(SVector &cam)
{ UpdateState();
  if (0 == sta) return;
  U_CHAR col = (SameSide(side,cam))?(col1):(col2);
  for (int k=0; k<nbl; k++)    DrawAll(col);
  return;
}
//=================================================================================
//  PAPI Light system
//  Lights are from far side to near side
//=================================================================================
CPapiLITE::CPapiLITE(U_CHAR type, U_SHORT nb): CBaseLITE(type,nb)
{ if       (nb == 4)  {dTAB = valPAPI; }
  else  if (nb == 2)  {dTAB = valAPAPI;}
  else  gtfo("APAPI light invalid");
}
//-----------------------------------------------------------------------------
//  Draw the lights if they are visible from the camera
//  NOTE:  The landing point is given by the runway
//      D² = (Ground distance from aircraft to landing point)²
//      H² = (Aircraft altitude above lading point)²
//      Thus H² / D² = Tangent(alpha)² = TA
//      where alpha is aircraft angle to landing point
//      Thus each light spot decision value may be compared to TA
//-----------------------------------------------------------------------------
void  CPapiLITE::DrawPAPI(SVector &cam)
{ if (SameSide(side,cam)) return;
  //---Compute ground plane distance to landing point -------------------------
  SVector   cmp;
  globals->tcm->RelativeFeetTo(*lnd,cmp);
  double    sD = (cmp.x * cmp.x) + (cmp.y * cmp.y);
  double    sH = (cmp.z * cmp.z);
  double    Ta = (sH / sD);
  for (int k=0; k<nbl; k++)
  { U_CHAR col = (Ta > dTAB[k])?(TC_WHI_LITE):(TC_RED_LITE);
    DrawOne(k,col);
  }
  return;
}
//=================================================================================
//  MERGE TABLE FOR LIGHT MODEL
//  New Hi End is
//  0 = None
//  1 = MONO
//  2 = CATx
//====================================================================================
//  PREVIOUS STATE IS
//  TC_LITE_RNONE  TC_LITE_HICAT  TC_LITE_LOCAT  TC_LITE_2RCAT  TC_LITE_RMONO
//====================================================================================
U_CHAR rwyHiLITE[] = {
    //------------------------------USER Hi = no change (0)------------------------------
    TC_LITE_RNONE, TC_LITE_HICAT, TC_LITE_LOCAT, TC_LITE_2RCAT, TC_LITE_RMONO,
    //------------------------------USER Hi = NONE-------(1)-----------------------------
    TC_LITE_RNONE, TC_LITE_RNONE, TC_LITE_LOCAT, TC_LITE_LOCAT, TC_LITE_RNONE,
    //------------------------------USER Hi = MONO(1)----(2)-----------------------------
    TC_LITE_RMONO, TC_LITE_RMONO, TC_LITE_LOCAT, TC_LITE_LOCAT, TC_LITE_RMONO,
    //------------------------------USER Hi = CATx(2) ---(3)-----------------------------
    TC_LITE_HICAT, TC_LITE_HICAT, TC_LITE_2RCAT, TC_LITE_2RCAT, TC_LITE_HICAT,
};
//=================================================================================
//  MERGE TABLE FOR LIGHT MODEL
//  New Hi End is
//  0 = None
//  1 = MONO
//  2 = CATx
//====================================================================================
//  PREVIOUS STATE IS
//  TC_LITE_RNONE  TC_LITE_HICAT  TC_LITE_LOCAT  TC_LITE_2RCAT  TC_LITE_RMONO
//====================================================================================
U_CHAR rwyLoLITE[] = {
    //------------------------------USER Lo = NONE------------------------------------
    TC_LITE_RNONE, TC_LITE_HICAT, TC_LITE_RNONE, TC_LITE_HICAT, TC_LITE_RNONE,
    //------------------------------USER Lo = MONO(1)---------------------------------
    TC_LITE_RMONO, TC_LITE_HICAT, TC_LITE_RMONO, TC_LITE_HICAT, TC_LITE_RMONO,
    //------------------------------USER Lo = CATx(2) --------------------------------
    TC_LITE_LOCAT, TC_LITE_2RCAT, TC_LITE_LOCAT, TC_LITE_2RCAT, TC_LITE_LOCAT,
};
//=========================================================================================
//  Runway Light profile
//  Build default profile
//  Save it
//=========================================================================================
CRLP::CRLP(CRunway *r)
 { rwy    = r;
   modify = 0;
   DefaultProfile(r);
   papi.SetMiniSize(10);
   slite  = RWY_LS_APPRO;
 }
//---------------------------------------------------------------------------------
//  Destroy CRLP
//---------------------------------------------------------------------------------
CRLP::~CRLP()
{
}
//---------------------------------------------------------------------------------
//  Flush all lights
//---------------------------------------------------------------------------------
void CRLP::Flush()
{ cLIT.Flush();
  eLIT.Flush();
  tLIT.Flush();
  bLIT.Flush();
  aLIT.Flush();
  return;
}
//---------------------------------------------------------------------------------
//  Draw all lights
//---------------------------------------------------------------------------------
void CRLP::DrawLITE(SVector &cam)
{ papi.DrawSystem(cam);
	if (rwy->Alighted(RWY_LS_CENTER)) cLIT.DrawSystem(cam);
  if (rwy->Alighted(RWY_LS_EDGES) ) eLIT.DrawSystem(cam);
  if (rwy->Alighted(RWY_LS_TOUCH))  tLIT.DrawSystem(cam);
  if (rwy->Alighted(RWY_LS_BARS))   bLIT.DrawSystem(cam);
  if (rwy->Alighted(RWY_LS_APPRO))  aLIT.DrawSystem(cam);
  return;
}
//---------------------------------------------------------------------------------
//  Set PAPI point size
//---------------------------------------------------------------------------------
void CRLP::DistanceNow(float dis)
{ float dim = (dis < 5.0)?(10):(2);
	papi.SetMiniSize(dim);
	return;
}
//---------------------------------------------------------------------------------
//  Build default profile
//---------------------------------------------------------------------------------
void CRLP::DefaultProfile(CRunway *rwy)
{   moCT = GetCenterLightModel(rwy);
    moED = GetEdgeLightModel(rwy);
    int hid = rwy->GetHiDSP();
    int lod = rwy->GetLoDSP();
    U_CHAR tbar = 0;
    //----Hi Threshold bar ------------------------------
    tbar = 0;
    if (rwy->HasHiRTH()) tbar  = TC_TBAR_GRN;
    if (rwy->HasLoETH()) tbar |= TC_TBAR_RED;
    if (hid && tbar)     tbar  = TC_TBAR_NOP;
    HiPF[TC_PROF_TBR] =  tbar;
    //----Lo Threshold bar ------------------------------
    tbar = 0;
    if (rwy->HasLoRTH()) tbar  = TC_TBAR_GRN;
    if (rwy->HasHiETH()) tbar |= TC_TBAR_RED;
    if (lod && tbar)     tbar  = TC_TBAR_NOP;
    LoPF[TC_PROF_TBR] =  tbar;
    //----Hi Wing Bar -----------------------------------
    HiPF[TC_PROF_WBR]    = ((hid) && rwy->HasHiRTH())?(1):(0);
    //----Lo Wing Bar -----------------------------------
    LoPF[TC_PROF_WBR]    = ((lod) && rwy->HasLoRTH())?(1):(0);
    //----Touch down area -------------------------------
    HiPF[TC_PROF_TDZ] = (rwy->HasHiTDZ())?(1):(0);
    LoPF[TC_PROF_TDZ] = (rwy->HasLoTDZ())?(1):(0);
    //----Approach lights -------------------------------
    HiPF[TC_PROF_APR] = 0;
    LoPF[TC_PROF_APR] = 0;
    //----PAPI systems ----------------------------------
    HiPF[TC_PROF_PAP] = 0;
    LoPF[TC_PROF_PAP] = 0;
    //----MASKs -----------------------------------------
    HiPF[TC_PROF_MSK] = TC_LITE_HICAT;
    LoPF[TC_PROF_MSK] = TC_LITE_LOCAT;
    //----Fill from runway light system ------------------
    SetLight(rwy->GetHiLightSys(),HiPF);
    SetLight(rwy->GetLoLightSys(),LoPF);
    return;
}
//---------------------------------------------------------------------------------
//  Set light from runway light system
//---------------------------------------------------------------------------------
void CRLP::SetLight(U_CHAR *ls,U_CHAR *lpf)
{ for (int k=0; k<8; k++)
  { switch (ls[k])  {
    case 0x0A:
      lpf[TC_PROF_APR] = TC_APR_ALF2;
      continue;
    case 0x0D:
      lpf[TC_PROF_APR] = TC_APR_ALF1;
      continue;
    case 0x0F:
      lpf[TC_PROF_APR] = TC_APR_ALSR;
      continue;
    case 0x2B:
      lpf[TC_PROF_PAP] = TC_PAPI_2L;
      continue;
    case 0x32:
      lpf[TC_PROF_PAP] = TC_PAPI_4L;
      continue;

  }
}
return;
}
//---------------------------------------------------------------------------------
//  Select center light model
//---------------------------------------------------------------------------------
int CRLP::GetCenterLightModel(CRunway *rwy)
{ U_CHAR mod = 0;
  if (rwy->NoCenter())    return 0;
  mod = ilsCAT[rwy->GetIlsIndex()];
  return (mod)?(mod):(TC_LITE_RMONO);
}
//---------------------------------------------------------------------------------
//  Select edge light model
//---------------------------------------------------------------------------------
int CRLP::GetEdgeLightModel(CRunway *rwy)
{ U_CHAR mod = 0;
  if (rwy->NoEdge())      return 0;
  if (rwy->NoPrecision()) return TC_LITE_RMONO;
  if (rwy->NoCenter())    return 0;
  mod = ilsCAT[rwy->GetIlsIndex()];
  return mod;
}
//-----------------------------------------------------
//  Merge user Hi profile to default profile
//---------------------------------------------------------------------------------
void CRLP::MergeHiUser(RWY_EPF *hip)
{ U_CHAR inx = 0;
  //---- change Center Light Model ---------------------------------
  inx = (TC_LITE_ENTRY * hip->ctrL) + moCT ;     // Index in change table
  moCT = rwyHiLITE[inx];
  //---- change Edge light Model -----------------------------------
  inx = (TC_LITE_ENTRY * hip->edgL) + moED;
  moED = rwyHiLITE[inx];
  //----- Update TBAR model ----------------------------------------
  if (hip->Tbar)                HiPF[TC_PROF_TBR] = hip->Tbar;
  if (hip->Tbar == TC_TBAR_SUP) HiPF[TC_PROF_TBR] = 0;
  //------ Update WING BAR model -----------------------------------
  if (hip->Wbar)                HiPF[TC_PROF_WBR] = hip->Wbar;
  if (hip->Wbar == TC_OPTP_SUP) HiPF[TC_PROF_WBR] = 0;
  //-------Update Touch down zone ----------------------------------
  if (hip->tdzL)                HiPF[TC_PROF_TDZ] = hip->tdzL;
  if (hip->tdzL == TC_OPTP_SUP) HiPF[TC_PROF_TDZ] = 0;
  //-------Update Approach light system ----------------------------
  if (hip->aprL)                HiPF[TC_PROF_APR] = hip->aprL;
  if (hip->aprL == TC_APR_NONE) HiPF[TC_PROF_APR] = 0;
  //-------Update PAPI LIGHT ---------------------------------------
  if (hip->papL)                HiPF[TC_PROF_PAP] = hip->papL;
  if (hip->papL == TC_PAPI_NL)  HiPF[TC_PROF_PAP] = 0;
  return;
}
//---------------------------------------------------------------------------------
//  Change Center lights
//---------------------------------------------------------------------------------
void CRLP::ChangeHiCTR(U_CHAR op)
{ int inx = (TC_LITE_ENTRY * op) + moCT ;     // Index in change table
  moCT    = rwyHiLITE[inx];
  modify  = 1;
}
//---------------------------------------------------------------------------------
//  Change Center lights
//---------------------------------------------------------------------------------
void CRLP::ChangeLoCTR(U_CHAR op)
{ int inx = (TC_LITE_ENTRY * op) + moCT ;     // Index in change table
  moCT    = rwyLoLITE[inx];
  modify  = 1;
}
//---------------------------------------------------------------------------------
//  Change Edge lights
//---------------------------------------------------------------------------------
void CRLP::ChangeHiEDG(U_CHAR op)
{ int inx = (TC_LITE_ENTRY * op) + moED;
  moED    = rwyHiLITE[inx];
  modify = 1;
}
//---------------------------------------------------------------------------------
//  Change Edge lights
//---------------------------------------------------------------------------------
void CRLP::ChangeLoEDG(U_CHAR op)
{ int inx = (TC_LITE_ENTRY * op) + moED;
  moED    = rwyLoLITE[inx];
  modify = 1;
}

//---------------------------------------------------------------------------------
//  Change Approach light
//---------------------------------------------------------------------------------
void CRLP::ChangeHiAPR(U_CHAR op)
{ HiPF[TC_PROF_APR] = op;
  modify  = 1;
  return;
}
//---------------------------------------------------------------------------------
//  Change Approach light
//---------------------------------------------------------------------------------
void CRLP::ChangeLoAPR(U_CHAR op)
{ LoPF[TC_PROF_APR] = op;
  modify  = 1;
  return;
}
//---------------------------------------------------------------------------------
//  Change TBAR light
//---------------------------------------------------------------------------------
void CRLP::ChangeHiTBR(U_CHAR op)
{ HiPF[TC_PROF_TBR] = op;
  modify  = 1;
  return;
}
//---------------------------------------------------------------------------------
//  Change TBAR light
//---------------------------------------------------------------------------------
void CRLP::ChangeLoTBR(U_CHAR op)
{ LoPF[TC_PROF_TBR] = op;
  modify  = 1;
  return;
}
//---------------------------------------------------------------------------------
//  Change WBAR light
//---------------------------------------------------------------------------------
void CRLP::ChangeHiWBR(U_CHAR op)
{ HiPF[TC_PROF_WBR] = op;
  modify  = 1;
  return;
}
//---------------------------------------------------------------------------------
//  Change WBAR light
//---------------------------------------------------------------------------------
void CRLP::ChangeLoWBR(U_CHAR op)
{ LoPF[TC_PROF_WBR] = op;
  modify  = 1;
  return;
}
//---------------------------------------------------------------------------------
//  Change TDZ light
//---------------------------------------------------------------------------------
void CRLP::ChangeHiTDZ(U_CHAR op)
{ HiPF[TC_PROF_TDZ] = op;
  modify  = 1;
  return;
}
//---------------------------------------------------------------------------------
//  Change TDZ light
//---------------------------------------------------------------------------------
void CRLP::ChangeLoTDZ(U_CHAR op)
{ LoPF[TC_PROF_TDZ] = op;
  modify  = 1;
  return;
}
//---------------------------------------------------------------------------------
//  Merge user Lo profile to default profile
//---------------------------------------------------------------------------------
void CRLP::MergeLoUser(RWY_EPF *lop)
{ U_CHAR inx = 0;
  //---- change Center Light Model ---------------------------------
  inx = (TC_LITE_ENTRY * lop->ctrL) + moCT ;     // Index in change table
  moCT = rwyLoLITE[inx];
  //---- change Edge light Model -----------------------------------
  inx = (TC_LITE_ENTRY * lop->edgL) + moED;
  moED = rwyLoLITE[inx];
  //----- Update TBAR model ----------------------------------------
  if (lop->Tbar)                LoPF[TC_PROF_TBR] = lop->Tbar;
  if (lop->Tbar == TC_TBAR_SUP) LoPF[TC_PROF_TBR] = 0;
  //------ Update WING BAR model -----------------------------------
  if (lop->Wbar)                LoPF[TC_PROF_WBR] = lop->Wbar;
  if (lop->Wbar == TC_OPTP_SUP) LoPF[TC_PROF_WBR] = 0;
  //-------Update Touch down zone ----------------------------------
  if (lop->tdzL)                LoPF[TC_PROF_TDZ] = lop->tdzL;
  if (lop->tdzL == TC_OPTP_SUP) LoPF[TC_PROF_TDZ] = 0;
  //-------Update Approach light system ----------------------------
  if (lop->aprL)                LoPF[TC_PROF_APR] = lop->aprL;
  if (lop->aprL == TC_APR_NONE) LoPF[TC_PROF_APR] = 0; 
  //-------Update PAPI LIGHT ---------------------------------------
  if (lop->papL)                LoPF[TC_PROF_PAP] = lop->papL;
  if (lop->papL == TC_PAPI_NL)  LoPF[TC_PROF_PAP] = 0;
  return;
}
//--------------------------------------------------------------------------------
//  Return Hi Center Model
//--------------------------------------------------------------------------------
int CRLP::GetModel(U_INT mod,U_INT msk)
{ if (mod  & msk)           return TC_LITE_INST;
  if (mod == TC_LITE_RMONO) return TC_LITE_EDGE;
  return TC_LITE_NONE;
}
//--------------------------------------------------------------------------------
//  Write the runway profile
//  TODO: Write only what differs from default
//--------------------------------------------------------------------------------
void CRLP::WriteProfile(CRunway *rwy,SStream *s)
{ if (0 == modify)  return;
  char txt[256];
  WriteTag('rwId', " === Runway End Profile definition ===", s);
  _snprintf (txt,256,"%s        // Hi end",rwy->GetHiEnd());
  WriteString(txt,s);
  WriteValues(HiPF,s);
  WriteTag('rwId', " === Runway End Profile definition ===", s);
  _snprintf (txt,256,"%s        // Lo end",rwy->GetLoEnd());
  WriteString(txt,s);
  WriteValues(LoPF,s);
  modify = 0;
  return;
}
//------------------------------------------------------------------------------------
//  Write profile parameters
//------------------------------------------------------------------------------------
void CRLP::WriteValues(U_CHAR *prof,SStream *s)
{ U_CHAR mod;
  U_CHAR msk = prof[TC_PROF_MSK];
  WriteTag('bgno', " ====== Begin Object ====",s);
  WriteTag('aprL', "     // Approach lights system",s);
  mod = prof[TC_PROF_APR];
  WriteString(aprsTAG[mod],s);
  WriteTag('Tbar', "     // Threshold lights",s);
  mod = prof[TC_PROF_TBR];
  WriteString(tbarTAG[mod],s);
  WriteTag('Wbar', "     // Wing bar lights",s);
  mod = prof[TC_PROF_WBR];
  WriteString(yenoTAG[mod],s);
  WriteTag('tdzL', "     // Touch Down zone",s);
  mod = prof[TC_PROF_TDZ];
  WriteString(yenoTAG[mod],s);
  WriteTag('ctrL', "     // Center lights",s);
  mod = GetModel(moCT,msk);
  WriteString(instTAG[mod],s);
  WriteTag('edgL', "     // Edge Lights",s);
  mod = GetModel(moED,msk);
  WriteString(instTAG[mod],s);
  mod = prof[TC_PROF_PAP];
  WriteString(papiTAG[mod],s);
  WriteTag('endo', " === End Object ======================", s);
  return;
}

//=========================================================================================
//  LIGHT HANGAR to build all kind of lights
//
//
//=========================================================================================
//-------------------------------------------------------------------------------
//    Get elevation in segment table
//    Org is the airport origin.
//    The light is positionned relative to this origin because all airport
//    objects are drawed from this origin
//-------------------------------------------------------------------------------
void LsComputeElevation(TC_VTAB &tab,SPosition &org)
{ double lon  = tab.VT_X + org.lon;
  double lat  = tab.VT_Y + org.lat;
  GroundSpot spot(lon,lat);
  globals->tcm->SetGroundAt(spot);
  tab.VT_Z = (spot.alt - org.alt);
  return;
}
//============================================================================
//  CLASS C3Dlight: A BILL BOARD LIGHT OF ANY SIZE
//============================================================================
C3DLight::C3DLight(void *p,Tag mod,U_CHAR color,float rd)
{ mother= p;
  Mode  = mod;
  inQ   = 0;
  cLit  = color;
  ntex  = TC_FLR_TEX;
  rDim  = rd;                                          // 16 Feet default radius
  oPos.lon = 0;
  oPos.lat = 0;
  oPos.alt = 0;
  Build();
}
//-----------------------------------------------------------------------------
//  Second constructor
//-----------------------------------------------------------------------------
C3DLight::C3DLight(void *p,Tag mod)
{ mother= p;
  Mode  = mod;
  inQ   = 0;
  cLit  = TC_WHI_LITE;
  ntex  = TC_FLR_TEX;
  rDim  = 10;
  freq  = 1.0f;
  onTm  = 1.0f;
}
//-----------------------------------------------------------------------------
//  Set absolute location
//-----------------------------------------------------------------------------
void C3DLight::SetLocation(SPosition &pos)
{ Add2dPosition(pos,oPos, oPos);
  Build();
  return;
}
//-----------------------------------------------------------------------------
//  Read parameters 
//-----------------------------------------------------------------------------
int C3DLight::Read(SStream *st, Tag tag)
{ Tag     pm;
  double  db;
  int     nb;
  switch(tag) {
  //---Texture number ---------------------
    case 'ntex':
      ReadInt(&nb,st);
      ntex = nb;
      return TAG_READ;
  //---Color -------------------------------
    case 'colr':
      ReadTag(&pm,st);
      if ('_whi' == pm) cLit = TC_WHI_LITE;
      if ('_red' == pm) cLit = TC_RED_LITE;
      if ('_blu' == pm) cLit = TC_BLU_LITE;
      if ('_yel' == pm) cLit = TC_YEL_LITE;
      return TAG_READ;
  //---Frequency for flash -----------------
    case 'freq':
      ReadDouble(&db,st);
      freq  = float(db);
      return TAG_READ;
  //---On time duration --------------------
    case 'onTm':
      ReadDouble(&db,st);
      onTm  = float(db);
      return TAG_READ;
  //--- Radius dimension -------------------
    case 'rdim':
      ReadDouble(&db,st);
      rDim  = float(db);
      return TAG_READ;
  //----Object offset ----------------------
    case 'offs':
      ReadDouble(&db,st);       // X
      oPos.lon = TC_ARCS_FROM_FEET(db);
      ReadDouble(&db,st);       // Y
      oPos.lat = TC_ARCS_FROM_FEET(db);
      ReadDouble(&db,st);       // Z
      oPos.alt = db;
      return TAG_READ;
  }
  return TAG_IGNORED;
}
//-----------------------------------------------------------------------------
//  All parameters are read.  Set texture object
//-----------------------------------------------------------------------------
void C3DLight::ReadFinished()
{   
}
//----------------------------------------------------------------------------
//  Build the light with color and type
//  BEWARE:  The face is drawn in positive direction. If not as it always faces
//            the camera, nothing is drawn and it is not easy to find why
//----------------------------------------------------------------------------
void C3DLight::Build()
{ tab[0].VT_S = 0;
  tab[0].VT_T = 0;
  tab[0].VT_X = - rDim;
  tab[0].VT_Y = - rDim;
  tab[0].VT_Z =  0;

  tab[3].VT_S = 0;
  tab[3].VT_T = 1;
  tab[3].VT_X = - rDim;
  tab[3].VT_Y = + rDim;
  tab[3].VT_Z = 0;

  tab[2].VT_S = 1;
  tab[2].VT_T = 1;
  tab[2].VT_X = + rDim;
  tab[2].VT_Y = + rDim;
  tab[2].VT_Z = 0;

  tab[1].VT_S = 1;
  tab[1].VT_T = 0;
  tab[1].VT_X = + rDim;
  tab[1].VT_Y = - rDim;
  tab[1].VT_Z = 0;

  return;
}
//-----------------------------------------------------------------------------
//  Check for Queuing
//-----------------------------------------------------------------------------
bool C3DLight::InQueue()
{ bool rc = (inQ == 1);
  inQ     = 1;
  return rc;
}
//-----------------------------------------------------------------------------
//  Change texture
//-----------------------------------------------------------------------------
void C3DLight::SetTexture(U_CHAR No)
{ ntex;
  return;
}
//-----------------------------------------------------------------------------
//  Draw the light:
//  The camera is at object origin
//  NOTE:  Blending is active from 3Dworld call
//-----------------------------------------------------------------------------
int C3DLight::DrawModel(U_CHAR rfq)
{ double modelview[16];
  float ems[4];
  glGetMaterialfv(GL_FRONT,GL_EMISSION,ems);
  //----Save model and get billboard matrix -----------
  glPushMatrix();
  BillBoardMatrix(modelview);
  //--- Assert the modelview with no rotations and scaling
  glLoadMatrixd(modelview);
  //---- Drawing parameters ---------------------------
  U_INT oTex     = globals->txw->GetLiteTexture(ntex);    // Get texture
  glBindTexture(GL_TEXTURE_2D,oTex);
  GLfloat *color = (GLfloat *)&ColorTAB[cLit];
  glMaterialfv (GL_FRONT, GL_EMISSION, color);
  glColor3fv(color);
  glInterleavedArrays(GL_T2F_V3F,0,tab);
  glDrawArrays(GL_QUADS,0,4);
  //---------------------------------------------------
  glPopMatrix();
  glMaterialfv (GL_FRONT, GL_EMISSION, ems);
  glColor3f(1,1,1);
  return 1;
}

//=========================================================================================
//
// CExternalLight: Aircraft light
//
//=========================================================================================
CExternalLight::CExternalLight (Tag type, Tag tag,CExternalLightManager *lm)
{ this->type  = type;
  unId        = tag;
  powered     = 0;
  on          = 0;
  ltm         = lm;
  radius      = 200;
  oTex        = 0;
  upd         = 0;
  spt         = 0;
  xCol        = TC_WHI_LITE;
	polr.y			= 0;								// Polar pitch
  // Initialize light placement, orientation 
  cycle = 0;
  duty  = 0;
  Time  = 0;
  purp  = EXTERNAL_LIGHT_NONE;
  //---------------------------------------
  TagToString(stID,tag);
}
//-------------------------------------------------------------------------------
//  Read parameters
//-------------------------------------------------------------------------------
int CExternalLight::Read (SStream *stream, Tag tag)
{ char s[80];
  switch (tag) {
    case 'part':
      // External model part name
      ReadString (s, 80, stream);
      part = s;
      return TAG_READ;

    case 'colr':
      // Light colour name, needs to be translated to RGB colour
      ReadString (s, 80, stream);
      StringToColour (s);
      return TAG_READ;

    case '_loc':
      // Read position offset (in LH coordinate system) and convert to RH
      ReadVector (&oPos, stream);
      VectorDistanceLeftToRight  (oPos);
      return TAG_READ;

    case 'polr':
      // Read polar orientation angles (in LH coordinate system) and convert to RH
      ReadVector (&polr, stream);
			dist		 = polr.x;
			radius	 = polr.x;
 			polr.y  *= -1;
			polr.z  *= -1;
      return TAG_READ;

    case 'cycl':
      // Read flash cycle time
      ReadFloat (&cycle, stream);
      if (duty)  duty = (cycle * duty);
      return TAG_READ;

    case 'dcyc':
      // Read duty cycle time
      ReadFloat (&duty, stream);
      if (cycle)  duty = (cycle * duty);
      return TAG_READ;

    case 'purp':
      // Light purpose
      ReadString (s, 80, stream);
      StringToPurpose (s);
      return TAG_READ;
      //--- Light cone : ignore for now 
    case 'cone':
      ReadString (s, 80, stream);
      return TAG_READ;
  }

  // Tag was not processed by this object, it is unrecognized
  char st[8];
  TagToString (st, tag);
  WARNINGLOG ("CExternalLight::Read : Unrecognized tag <%s>", st);
  return TAG_IGNORED;
}
//-------------------------------------------------------------------------------
//  Transform string in color texture
//-------------------------------------------------------------------------------
void CExternalLight::StringToColour (const char* s)
{
  if (stricmp (s, "WHITE") == 0) {xCol = TC_WHI_LITE; return;}
  if (stricmp (s, "RED") == 0)   {xCol = TC_RED_LITE; return;}
  if (stricmp (s, "GREEN") == 0) {xCol = TC_GRN_LITE; return;}
  WARNINGLOG ("CExternalLight : Unknown <colr> string \"%s\"", s);
}
//-------------------------------------------------------------------------------
//  Transform string in purpose
//  TODO  introduce TAIL light 
//-------------------------------------------------------------------------------
void CExternalLight::StringToPurpose (const char* s)
{
  if (stricmp (s, "TAXI") == 0)
  { purp  = EXTERNAL_LIGHT_TAXI;
    spt   = 1;												// Is a spot light
    SetQuad(1);
		ltm->AddSpotLite(this);
    return;}
  if (stricmp (s, "LAND") == 0)
  { purp  = EXTERNAL_LIGHT_LAND;
    spt   = 1;												// Is a spot light
    SetQuad(1);
		ltm->AddSpotLite(this);
    return;}
  //-----STROBE -------------------------------------------
  if (stricmp (s, "STROBE") == 0)
  { purp  = EXTERNAL_LIGHT_STROBE;
    upd   = 1;
    SetQuad(1);
		ltm->AddNaviLite(this);
    return;}
  //-----BEACON ------------------------------------------
  if (stricmp (s, "BEACON") == 0)
  { purp  = EXTERNAL_LIGHT_BEACON; 
    upd   = 1;
    SetQuad(1);
		ltm->AddNaviLite(this);
    return;}
  //-----NAVIGATION --------------------------------------
  if (stricmp (s, "NAV") == 0)
  { purp = EXTERNAL_LIGHT_NAV;     
    SetQuad(1);
		ltm->AddNaviLite(this);
    return;}
  //----Not supported ------------------------------------
  WARNINGLOG ("CExternalLight : Unknown <purp> string \"%s\"", s);
  purp = EXTERNAL_LIGHT_NONE;
  return;
}
//-------------------------------------------------------------------------------
//	All parameters are read
//-------------------------------------------------------------------------------
void CExternalLight::ReadFinished()
{	//--- Build the direction matrice for the spot light
	if (spt)
	{ glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glRotated(polr.y,1,0,0);					// Pitch adjust	
		glRotated(polr.z,0,0,1);					// Heading
 		glTranslated(oPos.x,oPos.y,oPos.z);	
		//--- Save the matrix ------------------------
	  glGetDoublev(GL_MODELVIEW_MATRIX,pMat);
		glPopMatrix();
	}
	//--- Compute spot positions -------------------
	spotINR.MultMatGL(pMat,aPos);				// Translate
	spotOUT.MultMatGL(pMat,bPos);				// Translate
}
//-------------------------------------------------------------------------------
//  Build the Quad
//  Assign a flare texture
//-------------------------------------------------------------------------------
void CExternalLight::SetQuad(float rd)
{ oTex    = globals->txw->GetLiteTexture(TC_FLR_TEX);
  tab[0].VT_S = 0;
  tab[0].VT_T = 0;
  tab[0].VT_X = - rd;
  tab[0].VT_Y = - rd;
  tab[0].VT_Z =  0;

  tab[3].VT_S = 0;
  tab[3].VT_T = 1;
  tab[3].VT_X = - rd;
  tab[3].VT_Y = + rd;
  tab[3].VT_Z = 0;

  tab[2].VT_S = 1;
  tab[2].VT_T = 1;
  tab[2].VT_X = + rd;
  tab[2].VT_Y = + rd;
  tab[2].VT_Z = 0;

  tab[1].VT_S = 1;
  tab[1].VT_T = 0;
  tab[1].VT_X = + rd;
  tab[1].VT_Y = - rd;
  tab[1].VT_Z = 0;
}

//-------------------------------------------------------------------------------
//  Update power state
//-------------------------------------------------------------------------------
void CExternalLight::SetPowerState (bool power)
{ powered = power;
  on      = power;
}
//-------------------------------------------------------------------------------
//  Update Light state
//-------------------------------------------------------------------------------
void CExternalLight::TimeSlice (float dT)
{ //----Update elaspe timer --------------------------------------
  Time += dT;
  if (Time < duty)  return;
  on  = 0;
  if (Time < cycle) return;
  on  = 1;
  Time = 0;
  return;
}
//--------------------------------------------------------------------
//  Draw as a Quad
//-------------------------------------------------------------------
void CExternalLight::DrawAsQuad()
{ if (!on)			return;
	double modelview[16];
  //----Translate to position ----------------------------
  double *rot = ltm->GetROTM();
  oPos.MultMatGL(rot,sPos);
  glPushMatrix();
  glTranslated(sPos.x,sPos.y,sPos.z);
  //----Save model and get billboard matrix -----------
  BillBoardMatrix(modelview);
  //--- Assert the modelview with no rotations and scaling
  glLoadMatrixd(modelview);
  //---- Drawing parameters ---------------------------
  glBindTexture(GL_TEXTURE_2D,oTex);
  GLfloat *color = (GLfloat *)&ColorTAB[xCol];
  glMaterialfv (GL_FRONT, GL_EMISSION, color);
	glMaterialfv (GL_FRONT, GL_DIFFUSE,  color);
  glColor3fv(color);
  glInterleavedArrays(GL_T2F_V3F,0,tab);
  glDrawArrays(GL_QUADS,0,4);
  //---------------------------------------------------
  glPopMatrix();
  return;
}
//--------------------------------------------------------------------
//  Draw spot
//	Spot lights are projected on the ground, like shadow.
//	A spot texture is projected on the ground
//-------------------------------------------------------------------
void	CExternalLight::DrawAsSpot()
{ if (!on)			return;
	//-----------------------------------------------------
	float prj[16];										// Projection matrix
	double *rot		= ltm->GetROTM();
	oPos.MultMatGL(rot,sPos);
	//--- Compute light attenuation ---------------------
	aPos.MultMatGL(rot,aRot);					// Rotate min line
	bPos.MultMatGL(rot,bRot);					// Rotate max line
	LightAttenuation();
	glPushMatrix();
	//---- Compute projector world position -------------
	sDir[0]	= sPos.x;
	sDir[1]	= sPos.y;
	sDir[2]	= sPos.z;
	sDir[3]	= 1;
	//-----Init ground projection -----------------
	globals->tcm->SetShadowMatrix(prj,sDir);
 	glMultMatrixf(prj);							  // 3-project spot
	glMultMatrixd(rot);								// 2-Global rotate
	glMultMatrixd(pMat);							// 1-Position,heading
	//--- Always draw on --------------------------
  glInterleavedArrays(GL_T2F_C3F_V3F,0,spotTAB);
  glDrawArrays(GL_QUADS,0,4);
	//---------------------------------------------
  glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
//	DrawT1();
	return;
}
//--------------------------------------------------------------------
//	Compute light luminosity for each vertex of the projector
//	-Each vertex is projected on the ground giving a RAY
//	starting from the spot source (sRot)and intersecting the ground
//	The length of this RAY is used to compute the vertex luminosity
//--------------------------------------------------------------------
void CExternalLight::LightAttenuation()
{ //--- Set luminosity according to distance ---
	double L0 = GetLightForVector(aRot);
	spotTAB[0].VT_R = L0;
	spotTAB[0].VT_G = L0;
  spotTAB[0].VT_B = L0;
	spotTAB[1].VT_R = L0;
	spotTAB[1].VT_G = L0;
  spotTAB[1].VT_B = L0;
	double L1 = GetLightForVector(bRot);
	double T1 = GetTextureLimit();
	spotTAB[2].VT_R = L1;
	spotTAB[2].VT_G = L1;
  spotTAB[2].VT_B = L1;
	spotTAB[2].VT_T = T1;
	//------------------------------
	spotTAB[3].VT_R = L1;
	spotTAB[3].VT_G = L1;
  spotTAB[3].VT_B = L1;
	spotTAB[3].VT_T = T1;
	return;
}

//-------------------------------------------------------------------------------
//	Compute luminosity at vector v
//-------------------------------------------------------------------------------
double CExternalLight::GetLightForVector(CVector &v)
{	extd				= ltm->DistanceToGround(&sPos,&v);
	ratio				= (1 - (extd / radius));
	double lum	=  ratio * 0.5;
	return ClampTo(lum,double(0),double(1));
}
//-------------------------------------------------------------------------------
//	Compute External texture coordinate
//-------------------------------------------------------------------------------
double CExternalLight::GetTextureLimit()
{	double t1 = 0.4;
	double t2 = 0.2;
	if (extd < radius)		return 1;
	//--- compute a new ratio for texture ----
	double ct = t1 + (ratio * t2);
	return ct;
}
//-------------------------------------------------------------------------------
//  Draw the projected spot
//-------------------------------------------------------------------------------
void CExternalLight::DrawT1()
{ CVector vw;
	CVector	vs = sPos;
	CVector v1 = aRot;
	CVector v2 = bRot;
  double *rot = ltm->GetROTM();
	//--- Init drawing  ---------------------------
	glPushMatrix();
	glDisable(GL_TEXTURE_2D);
	//---------------------------------------------
	glBegin(GL_LINE_LOOP);
	glColor4f(0,1,0,1);

  glVertex3f(vs.x,vs.y,vs.z);
	glVertex3f(v1.x,v1.y,v1.z);
	glVertex3f(v2.x,v2.y,v2.z);
	
	glEnd();
	//-----------------------------------------------
	glPopMatrix();
	glEnable(GL_TEXTURE_2D);
}
//-------------------------------------------------------------------------------
//  Print the light
//-------------------------------------------------------------------------------
void CExternalLight::Print (FILE *f)
{
  char s[8];
  TagToString (s, unId);
  fprintf (f, "Light unique ID : %s\n", s);
  TagToString (s, type);
  fprintf (f, "           Type : %s\n", s);
  fprintf (f, "     Model Part : %s\n", part.c_str());
  fprintf (f, "         Colour : %d", xCol);
  fprintf (f, "       Position : %8.5f, %8.5f, %8.5f\n",
    oPos.x, oPos.y, oPos.z);
  fprintf (f, "    Orientation : %8.5f, %8.5f, %8.5f\n",
    polr.x, polr.y, polr.z);
  fprintf (f, "    Flash Cycle : %8.5f\n", cycle);
  fprintf (f, "     Duty Cycle : %8.5f\n", duty);

  /// @todo Print external light purpose
}
//=========================================================================================
//
// CExternalLightManager: Aircraft light system
//
//=========================================================================================
CExternalLightManager::CExternalLightManager (CVehicleObject *mv, char *fn)
{ mveh  = mv;
  xLit  = 0;
  SStream s;
  strcpy (s.filename, "World/");
  strcat (s.filename, fn);
  strcpy (s.mode, "r");
  if (OpenStream (&s)) {
    ReadFrom (this, &s);
    CloseStream (&s);
  }
	//----Get halo texture --------------------------
	Build2DHalo(32);
	//--- Set up vector in intersector --------------
	CVector up(0,0,1);
	secp.SetNP(up);
}
//-----------------------------------------------------------
//  Set texture parameters
//-----------------------------------------------------------
void CExternalLightManager::ReadHalo()
{ char *fn = "LiteHalo.bmp";
	TEXT_INFO txf;  // Texture info;
  CArtParser img(TC_HIGHTR);
//  txf.tsp   = 1;
  strncpy(txf.name,fn,TC_TEXTURE_NAME_NAM);
  _snprintf(txf.path,TC_TEXTURE_NAME_DIM,"ART/%s",fn);
  img.GetAnyTexture(txf);
	//--- Generate  a 2D texture -------------------
	glGenTextures(1,&txo);
	glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,txo);
  glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_FALSE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexImage2D   (GL_TEXTURE_2D,0,GL_INTENSITY,txf.wd,txf.ht,0,GL_RGBA,GL_UNSIGNED_BYTE,txf.mADR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	//--- Texture may be deleted ----
	delete [] txf.mADR;

	return;
}
//-----------------------------------------------------------
//  Build a texture Halo
//-----------------------------------------------------------
void	CExternalLightManager::Build2DHalo(int sid)
{	int			  dim = sid * sid;
	float    *pix = new float[dim]; 
  // fill with black color 
  for(int i=0; i<dim; i++) pix[i] = 0; 
  float length; 
  //Half size of 2D texture is the sphere radius 
  float size2= float(sid) * 0.5; 

  // For each texel 
  for(float i=0; i<sid; i++) 
		{ for(float j=0; j<sid; j++) 
      {	//la demi-largeur moins la distance du centre au texel courant de la   texture. 
        //size2-1 pour eviter l'eclairage sur le bord de la texture. 
        length = (size2-1) - sqrt( (i-size2)*(i-size2) + (j-size2)*(j-size2)); 
        //normalisation dans la plage[0,1] (intensite de la lumiere) 
				length /= size2;
				//on stocke l'intensite dans le texel 
				int inx  = int(i * sid) + int(j);
        pix[inx]	= 2 * length;
			}
		}
	//---- Now generate a 2D texture ---------------------------
	glGenTextures(1,&txo);
	glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,txo);
  glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_FALSE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexImage2D   (GL_TEXTURE_2D,0,GL_INTENSITY,sid,sid,0,GL_LUMINANCE,GL_FLOAT,pix);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	//--- Texture may be deleted ----
	delete [] pix;
	return;
}

//-------------------------------------------------------------------------------
//  Free all lights
//-------------------------------------------------------------------------------
CExternalLightManager::~CExternalLightManager (void)
{ // Delete all allocated external light objects
	glDeleteTextures(1,&txo);
  std::map<Tag,CExternalLight*>::iterator i;
  for (i=nLit.begin(); i!=nLit.end(); i++) SAFE_DELETE (i->second);
  for (i=sLit.begin(); i!=sLit.end(); i++) SAFE_DELETE (i->second);
	nLit.clear();
	sLit.clear();
}
//-------------------------------------------------------------------------------
//  Read parameters
//-------------------------------------------------------------------------------
int CExternalLightManager::Read (SStream *stream, Tag tag)
{
  switch (tag) {
  case 'lite':
    // External light instance
    { Tag type, tag;
      ReadTag (&type, stream);
      ReadTag (&tag, stream);
      CExternalLight *lite = new CExternalLight (type, tag, this);
      ReadFrom (lite, stream);
    }
    return TAG_READ;
  }

  char s[16];
  TagToString (s, tag);
  WARNINGLOG ("CExternalLightManager::Read : Unrecognized tag <%s>", s);
  return TAG_IGNORED;
}
//-------------------------------------------------------------------------------
//  Add a navigation ligth
//-------------------------------------------------------------------------------
void CExternalLightManager::AddNaviLite(CExternalLight *lit)
{	Tag t		= lit->GetTag();
	nLit[t] = lit;
	return;
}
//-------------------------------------------------------------------------------
//  Add a spot ligth
//-------------------------------------------------------------------------------
void CExternalLightManager::AddSpotLite(CExternalLight *lit)
{	Tag t		= lit->GetTag();
	sLit[t] = lit;
	return;
}
//-------------------------------------------------------------------------------
//  Power state is updated
//-------------------------------------------------------------------------------
void CExternalLightManager::SetPowerState (Tag id, bool power)
{ std::map<Tag, CExternalLight*>::iterator i = nLit.find (id);
	if (i != nLit.end())  {return i->second->SetPowerState (power);}
	i	= sLit.find (id);
	if (i != sLit.end())  {return i->second->SetPowerState (power);}
	return;
}
//-------------------------------------------------------------------------------
//  Time slice
//-------------------------------------------------------------------------------
void CExternalLightManager::Timeslice (float dT)
{ std::map<Tag,CExternalLight*>::iterator i;
  for (i=nLit.begin(); i!=nLit.end(); i++)
  { CExternalLight *lit = (*i).second;
    if (lit->NeedUpdate())  lit->TimeSlice(dT);
  }
	//--- Update intersector with ground altitude ------------------
	secp.SetGA(-globals->tcm->GetFeetAGL());
}

//-------------------------------------------------------------------------------
//  Draw Navigation light
//-------------------------------------------------------------------------------
void CExternalLightManager::DrawOmniLights()
{	if (globals->noEXT)  return;
	std::map<Tag,CExternalLight*>::iterator i;
  glGetMaterialfv(GL_FRONT,GL_EMISSION,ems);		// save emissive color
  //----- Set GL state ---------------------------------------
  glPushAttrib (GL_ALL_ATTRIB_BITS);
  glEnable (GL_TEXTURE_2D);
  glEnable (GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER,0);
  //----- Draw individual lights--------------------------------
  glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
  glEnableClientState(GL_VERTEX_ARRAY);

  //--- Draw each ligth ----------------------------------------
  for (i=nLit.begin(); i!=nLit.end(); i++) {	i->second->DrawAsQuad(); }
	for (i=sLit.begin(); i!=sLit.end(); i++) {	i->second->DrawAsQuad(); }
  //--- Restore GL state ---------------------------------------
	glMaterialfv (GL_FRONT, GL_EMISSION, ems);
  glColor3f(1,1,1);
	glDisable (GL_ALPHA_TEST);
  glPopClientAttrib();
  glPopAttrib ();
	return;
}
//-------------------------------------------------------------------------------
//  Draw Navigation light
//-------------------------------------------------------------------------------
void CExternalLightManager::DrawSpotLights()
{	std::map<Tag,CExternalLight*>::iterator i;
  glGetMaterialfv(GL_FRONT,GL_EMISSION,ems);		// save emissive color
  //----- Set GL state ---------------------------------------
  glPushAttrib (GL_ALL_ATTRIB_BITS);
  glDisable(GL_DEPTH_TEST);
	glDepthMask(false);
	glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,txo);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glShadeModel(GL_SMOOTH);
  //----- Draw individual lights--------------------------------
  glPushClientAttrib (GL_CLIENT_VERTEX_ARRAY_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
  //--- Draw each ligth ----------------------------------------
  for (i=sLit.begin(); i!=sLit.end(); i++) {i->second->DrawAsSpot(); }
  //--- Restore GL state ---------------------------------------
	glMaterialfv (GL_FRONT, GL_EMISSION, ems);
  glPopClientAttrib();
  glPopAttrib ();
}
//-------------------------------------------------------------------------------
//  Return line distance for the RAY that starts at v0,passes
//	throught v1 and hit the ground
//-------------------------------------------------------------------------------
double CExternalLightManager::DistanceToGround(CVector *v0,CVector *v1)
{	secp.SetPA(v0);
	secp.SetPB(v1);
	secp.GetLineToPlane();
	if (secp.rc != 1)		return 100000;
	//----There is one intersection -------------
	CVector D = secp.PR - *v0;
	return secp.PR.Length();
}
//-------------------------------------------------------------------------------
//  Print lights
//-------------------------------------------------------------------------------
void CExternalLightManager::Print (FILE *f)
{
  std::map<Tag,CExternalLight*>::iterator i;
  for (i=nLit.begin(); i!=nLit.end(); i++) {
    i->second->Print (f);
  }
}

//============================END OF FILE =================================================
