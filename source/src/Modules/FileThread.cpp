/*
 * TerrainCache.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright  2003 Chris Wallace
 *            2008 Jean Sabatier
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
//=================================================================================
#include "../Include/Globals.h"
#include "../Include/FlyLegacy.h"
#include "../Include/TerrainCache.h"
#include "../Include/TerrainElevation.h"
#include "../Include/TerrainTexture.h"
#include "../Include/ScenerySet.h"
#include "../Include/Fui.h"
#include "../Include/3dMath.h"
#include "../Include/sky.h"
#include "../Include/Model3D.h"
#include <map>
#include<crtdbg.h>
//=============================================================================
extern U_INT    SizeRES[];
extern U_SHORT  SideRES[];
//=============================================================================
//  Night table gives the tile types that have night textures
//  TODO:  Built this table dynamically by scanning the files or by
//         some configuration
//=============================================================================
char NightTAB[256] = {
  //--0--1--2--3--4--5--6--7--8--9--A--B--C--D--E--F--
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  //-10-------------------------------------------1F--
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  //-20-------------------------------------------2F--
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  //-30-------------------------------------------3F--
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
  //-40-41-42643-44-45-46-47-48-49-4A-4B-4C-4D-4E-4F--
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  //-50-------------------------------------------5F--
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  //-60-------------------------------------------6F--
      0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  //-70-------------------------------------------7F--
      0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0,
  //-80-81-82-83-84-85-86-87-88-89-8A-8B-8C-8D-8E-8F--
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  //-90-------------------------------------------9F--
      0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  //-A0-------------------------------------------AF--
      0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0,
  //-B0-------------------------------------------BF--
      0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  //-C0-------------------------------------------CF--
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  //-D0-------------------------------------------DF--
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  //-E0-------------------------------------------EF--
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  //-F0-------------------------------------------FF--
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
//==========================================================================
//  State contour transition from pix = 0
//  Entry is the current pixel event
//  0 => No color
//  1 => Red color
//  2 => Blue color
//  3 => both Red and Blue
//  Transition color given the new pixel color return
//      -the next state
//      -the inside swap switch
//  The current Inside value
//==========================================================================
TC_PIX_STATE StatePIX[16]  = {
  //-----transitions from 0 color ------------------------------------
  {TC_PIX_ZERO,         0,        0},       // From 0 to 0:     No swap
  {TC_PIX_RED  ,TC_INSIDE,TC_INSIDE},       // From 0 to RED:   swap inside
  {TC_PIX_BLUE ,TC_INSIDE,TC_INSIDE},       // From 0 to blue:  swap inside
  {TC_PIX_BOTH ,        0,TC_INSIDE},       // From 0 to both:  No swap
  //-----transition from RED color -------------------------------------
  {TC_PIX_ZERO,        0,        0},        // From RED to 0:   No swap
  {TC_PIX_RED ,        0,TC_INSIDE},        // From RED to RED: No Swap
  {TC_PIX_BLUE,TC_INSIDE,TC_INSIDE},        // From RED to blue:  swap inside
  {TC_PIX_BOTH,TC_INSIDE,TC_INSIDE},        // From RED to both:  swap inside
  //-----transition from BLUE color ------------------------------------
  {TC_PIX_ZERO,        0,        0},        // From BLUE to 0:    No swap
  {TC_PIX_RED ,TC_INSIDE,TC_INSIDE},        // From BLUE to RED:  swap inside
  {TC_PIX_BLUE,        0,TC_INSIDE},        // From BLUE to blue: No swap
  {TC_PIX_BOTH,TC_INSIDE,TC_INSIDE},        // From BLUE to both: swap inside
  //-----transition from BOTH colors------------------------------------
  {TC_PIX_ZERO,        0,        0},        // From BOTH to 0:    No swap
  {TC_PIX_RED ,        0,TC_INSIDE},        // From BLUE to RED:  No Swap
  {TC_PIX_BLUE,        0,TC_INSIDE},        // From BLUE to blue: No swap
  {TC_PIX_BOTH,TC_INSIDE,TC_INSIDE},        // From BLUE to both: swap inside

  };
//=============================================================================
//  RESOLUTION PARAMETERS
//=============================================================================
U_CHAR NameRES[TC_MAX_TEX_RES] = {
  '1',                              // 0 => low
  '4',                              // 1 => Medium
  '5',                              // 2 => High
  '4',                              // 3 => EPD medium
};
//==========================================================================
//  shift factor = f(resolution)
//==========================================================================
U_INT shfRES[TC_MAX_TEX_RES] = {
  TC_BY32,                          // 0 low  64 = 2048/32
  TC_BY16,                          // 1 med 128 = 2048/16
  TC_BY08,                          // 2 hig 256 = 2048/8
  TC_BY16,                          // 3 hyp 128 = 2048/16
};
//==========================================================================
//  translation factor = f(resolution)
//==========================================================================
int tranRES[TC_MAX_TEX_RES] = {
   32,                              // 0 center at 32
   64,                              // 1 center at 64
  128,                              // 2 center at 128
   64,                              // 3 center at 64
};
//==========================================================================
//  shrink numerator = f(resolution)
//  This is the drawing surface to draw the coast
//  It is 2 pixels larger on each side than the rendering surface for
//  border blending
//==========================================================================
int numRES[TC_MAX_TEX_RES] = {
  64  - 12,                        // 0 wide 56   (render 52)
  128 -  0,                        // 1 wide 116  (render 112)  
  256 -  4,                        // 2 wide 244  (render 240)
  128 -  2,                        // 3 wide 118  (render 120)
};
//==========================================================================
//  Skip factor = f(resolution)
//==========================================================================
int skipRES[TC_MAX_TEX_RES] = {
  6,                              // 0 wide 48
  0,                              // 1 wide 112
  2,                              // 2 wide 240
  1,                              // 3 wide 118
};
//==========================================================================
//  Next state for texture descriptor
//  The state is function of the texture level
//  level 0:  The front texture are loaded.  We must allocate texture object
//  level 1:  The alternate texture are loaded. We must replace the
//            front textures with alternate ones
//==========================================================================
U_CHAR popSTA[] = {
  TC_TEX_OBJ,             // 0 => State to allocate texture object
  TC_TEX_POP,             // 1 => State to pop textures 
};
//=================================================================================
//  THIS FILE CONTAINS ALL ROUTINES THAT EXECUTE ON A SEPARATED THREAD
//  All threaded functions OF TEXTURE MANAGER are included for CLARITY.
//=================================================================================
//  THREAD TO READ FILE
//  This is a separated thread to
//        READ TEXTURES in a QGT
//        READ SHARED QTR files
//        READ SHARED COAST file
//  NOTE:  This thread will boost performance even on single processor as
//        it will allow main MAIN THREAD to work during IO access
//=================================================================================
//---------------------------------------------------------------------------------
//  TEXTURE LOADING
//---------------------------------------------------------------------------------
void TextureLoad(C_QGT *qgt)
{ CTextureWard *txw = globals->txw;       //tcm->GetTexWard();
  TCacheMGR    *tcm = globals->tcm;
 // qgt->LockState();
  //--------Load Texture in Load Queue ------------------------------------
  CSuperTile *sp = 0;
  for (sp = qgt->PopLoad(); sp != 0; sp = qgt->PopLoad())
    {	
			if (sp->NeedALT())  txw->LoadTextures(1,sp->aRes,qgt,sp);
			if (sp->NeedLOD())  txw->LoadTextures(0,sp->Reso,qgt,sp);
      qgt->EnterNearQ(sp);
			sp->RenderINR();
    }
 // qgt->UnLockState();
  qgt->PostIO();
  return;          
}
//---------------------------------------------------------------------------------
//  Get a QTR file
//---------------------------------------------------------------------------------
void GetQTRfile(C_QGT *qgt,TCacheMGR *tcm)
{ char td = tcm->GetDebug();
  //-------Search the map first --------------------------------------------
  tcm->LockQTR();
  U_INT  key  = qgt->GetReqKey();
  C_QTR *qtr  = tcm->GetQTR(key);
  if (0 == qtr)
      {//------Load the file -----------------------------------------------
        qtr = new C_QTR(key,tcm);
        tcm->AddQTR(qtr,key);                    // Enter in cache
      }
  qtr->IncUser();                           // One user count
  qgt->SetQTR(qtr);                         // Assign to QGT
  tcm->UnLockQTR();
  qgt->PostIO();
  return;
}
//=================================================================================
//  THREAD FOR TEXTURES
//=================================================================================
void ProcessTexture( TCacheMGR   *tcm)
{	C_QGT    *qgt		= 0;
  for (qgt = tcm->PopLoadTEX(); (qgt != 0); qgt = tcm->PopLoadTEX())
			{ //TRACE("TCM: -- Time: %04.2f QGT(%03d-%03d) THREAD TEXTURE",
				//											tcm->Time(),qgt->GetXkey(),qgt->GetZkey()); 
				TextureLoad(qgt);
				//TRACE("TCM: -- Time: %04.2f ---------------THREAD END",tcm->Time()); 
			}
			return;
}
//=================================================================================
//  THREAD FOR FILES
//=================================================================================
void ProcessFiles(TCacheMGR *tcm, SqlTHREAD *sql)
{	REGION_REC  reg;
	for ( C_QGT *qgt = tcm->PopFileREQ(); (qgt != 0); qgt = tcm->PopFileREQ())
			{ switch (qgt->GetReqCode()) {
					//--- Load a QTR file --------------------------
          case TC_POD_QTR:
							//TRACE("TCM: -- Time: %04.2f QGT(%03d-%03d) THREAD QTR",
							//								tcm->Time(),qgt->GetXkey(),qgt->GetZkey());
              GetQTRfile(qgt,tcm);
							//TRACE("TCM: -- Time: %04.2f ---------------THREAD END",tcm->Time()); 

              continue;
					//--- Load elevations ---------------------------
          case TC_SQL_ELV:
							//TRACE("TCM: -- Time: %04.2f QGT(%03d-%03d) THREAD ELV",
							//								tcm->Time(),qgt->GetXkey(),qgt->GetZkey());
              reg.qgt = qgt;
              reg.key = qgt->FullKey();
							sql->GetQgtElevation(reg,ELVtoCache);
              qgt->PostIO();
							//TRACE("TCM: -- Time: %04.2f ---------------THREAD END",tcm->Time()); 
 
              continue;
					//--- Load coast data ---------------------------
           case TC_REQ_SEA:
							//TRACE("TCM: -- Time: %04.2f QGT(%03d-%03d) THREAD SEA",
							//								tcm->Time(),qgt->GetXkey(),qgt->GetZkey());
              if (sql->SQLsea()) tcm->AllSeaSQL(qgt);
              else              tcm->AllSeaPOD(qgt);
              qgt->PostIO();
							//TRACE("TCM: -- Time: %04.2f ---------------THREAD END",tcm->Time()); 

              continue;

        } // end of switch
		}
  return;
}
//=================================================================================
//  THREAD FOR 3D MODELS
//=================================================================================
void ProcessModels(TCacheMGR *tcm, SqlTHREAD *sql)
{	C3DMgr   *m3d	= globals->m3d;	
	C_QGT    *qgt		= 0;
	char *dir = "MODELS";
  for (C3Dmodel *mod = m3d->ModelToLoad(); (mod != 0); mod = m3d->ModelToLoad())
      { char *mn = mod->GetFileName();
				//TRACE("TCM: -- Time: %04.2f ---------------THREAD MODELS",tcm->Time());
        if (!sql->SQLmod())						{mod->LoadPart(dir); mod->DecUser(); continue;}
				if (!sql->GetM3Dmodel(mod))		{mod->LoadPart(dir); mod->DecUser(); continue;}
        //-------------------------------------------------------------------------
				mod->Finalize();
        mod->DecUser();
				//TRACE("TCM: -- Time: %04.2f ---------------THREAD END",tcm->Time()); 

      }
	return;
}
//=================================================================================
//  THREAD FOR OSM Layers
//=================================================================================
void ProcessOSM(TCacheMGR *tcm, SqlTHREAD *sql)
{	CSceneryDBM *scn	= globals->scn;
	for (OSM_DBREQ *dbr = scn->PopOSMrequest(); (dbr != 0); dbr = scn->PopOSMrequest())
			{	C_QGT *qgt = dbr->qgt;
				//TRACE("TCM: -- Time: %04.2f QGT(%03d-%03d) THREAD OSM",
				//								tcm->Time(),qgt->GetXkey(),qgt->GetZkey());
			  sql->LoadOSM(dbr);
				delete dbr;
				//TRACE("TCM: -- Time: %04.2f ---------------THREAD END",tcm->Time()); 
			}
	return;
}
//=================================================================================
//  FILE THREAD LOOP
//=================================================================================
void *FileThread(void *p)
{ 
  TCacheMGR   *tcm	= (TCacheMGR*) p;
	char         thn  = tcm->GetThreadNumber();
  SqlTHREAD sql;												// Local instance of SQL manager
	globals->elvDB	= sql.UseELV();
  C_QGT    *qgt		= 0;
  U_INT     key		= 0;
	char			tr    = 1;									// Trace time
	//--- Declare first instance ----------------
	if (0 == globals->sql)  globals->sql = &sql;
	//--- Thread parameters ---------------------
	pthread_cond_t  *cond = tcm->GetTHcond();
	pthread_mutex_t *tmux = tcm->GetaMux(thn);
	TRACE("SQL Thread started");
  //--- Region parameters --------------------
  while (tcm->RunThread())
    { pthread_cond_wait(cond,tmux);						// Wait for signal
      //----Process load texture Queue first -------------------------------------
      if (thn == 0)		ProcessTexture(tcm);
      //----Process file Requests ------------------------------------------------
      if (thn == 1)		ProcessFiles(tcm,&sql);
      //--- Process 3DModel requests ----------------------------------------------
		  if (thn == 0)		ProcessModels(tcm,&sql);
			//--- Process OSM models requests--------------------------------------------
			if (thn == 1)   ProcessOSM(tcm,&sql);
    }
	//--- File thread is stopped ------------------
	if (globals->sql == &sql) globals->sql = 0;
	TRACE("FileThread %d STOP",thn);
	pthread_mutex_unlock(tmux);
	pthread_exit(0);
  return 0;
}
//=================================================================================
//  PARTS OF TEXTURE MANAGER
//=================================================================================
//-----------------------------------------------------------------------
//  Get a Texture with coast line
//  1) Allocate a canvas (msk) on which the coast is drawn
//  2) Allocate the land texture
//  3) Allocate the water texture
//  4) Using a simple IN/OUT scan algorithm on the canvas
//     Set a texture pixel as
//     IN=> From land
//    OUT=> From water
//-----------------------------------------------------------------------
int CTextureWard::GetSeaTexture(CTextureDef *txn)
{ GetMixTexture(txn,0);       // Get Land Texture with transition
  //-------Draw the polygon on the mask data ------------------
  MakeStencil(txn->coast);
	//-------Now build the night texture ------------------------
  int rt	= NightGenTexture(txn);
  if (rt)		BuildNightTexture((U_INT*)nTEX);      // Build Night texture
  //----Build sea texture (fixed or animated) -----------------
  rt		+=	BuildCoastTexture(dTEX);
  return rt;
}
//=================================================================================
//  PARTS OF TEXTURE MANAGER
//=================================================================================
//----------------------------------------------------------------------
//  Load a list of QGT Texture File
//  The list is for a QGT-SuperTile
//  NOTE: This routine and all subordinates run under FILE THREAD
//        Contexte is supported by the QGT
//  PARAMETERS:
//  1)lev is texture level: 0=> Current textures
//                        1=> Alternate textures 
//    Front textures are the one actually used to dispaly terrain
//    Alternate textures are loaded when the Supertile must change resolution
//    either because it enters the inner circle, or it leaves the inner circle
//  2) res is the texture resolution (Hi or LO)
//  3) qgt is the Quarter Global Tile
//  4) sp is the SuperTile for which textures are requested
//----------------------------------------------------------------------
int CTextureWard::LoadTextures(U_CHAR lev,U_CHAR res,C_QGT *qgt,CSuperTile *sp)
{ CTextureDef *txn  = 0;
  CmQUAD      *qad  = 0;
  this->qgt         = qgt;                    // Remember QGT
  Resn              = res;                    // Requested Resolution
  gx  = (qgt->GetXkey() >> TC_BY02);          // Globe Tile X composite
  gz  = (qgt->GetZkey() >> TC_BY02);          // Globe Tile Z composite

  //---- Load the textures for each detail tile -------------
  for (int Nd = 0; Nd != TC_TEXSUPERNBR; Nd++)
      { txn   = &sp->Tex[Nd];
        qad   = txn->quad;
        //---Uncomment and set Tile indices for stop on tile -
//       qad->AreWe(508,28,336,6);
//				int ok = strcmp(txn->Name,"656C0F03");
//				if (ok ==  0)
//					int	a = 1;
        //-----Clear  descriptor ----------------------------
        dTEX = 0;
        nTEX = 0;
        //-----Built the texture -----------------------------
        switch (txn->TypTX) {
          //---Coast texture ---------------
          case TC_TEXCOAST:
                GetSeaTexture(txn);
                break;
          //--- Water texture ---------------
          case TC_TEXWATER:
                break;
          //---Raw from TRN ------------------
          case TC_TEXRAWTN:
                GetRawTexture(txn);
                break;
          //---Raw from EPD ------------------
          //case TC_TEXRAWEP:
          //      GetEPDTexture(txn);
          //      break;
          //---Dedicated texture -------------
          case TC_TEXGENER:
                GetGenTexture(txn);
                break;
          //--Previously generic requalified as shared ---
          case TC_TEXSHARD:
                GetGenTexture(txn);
                break;
        }
      //--- SAVE Texture parameters ---------------------
      txn->SetDayTexture(lev,dTEX);
      txn->SetNitTexture(lev,nTEX);
      txn->SetResolution(lev,Resn);
      }
  //----Next state is to request texture objects -------------------------
  sp->SetState(popSTA[lev]);         // Next State is set objects
  return 0;
}

//-----------------------------------------------------------------------------
//  Build a Night texture matching the resolution for a generic tile
//  When day texture is 256 wide, the night texture is giving 4 pixels from
//  the original file
// Night textures are nevers shared
//-----------------------------------------------------------------------------
int CTextureWard::NightGenTexture(CTextureDef *txn)
{ U_CHAR th = txn->Hexa[0];             // Tile type
  U_CHAR nt = NightTAB[th] & NT;
  if (0 == nt)  return 0;               // No night texture
  char *gen = txn->Name;
  //----PATH is SYSTEM/GLOBE -----------------------------
  char *root = xld.path + 13;
  //----Format name --------------------------------------
  root[0] = gen[0];                             // Tile type
  root[1] = gen[1];                             // 
  root[2] = gen[8];                             // I index
  root[3] = gen[9];                             // J index
  root[4] = '4';                                // 4 always
  root[5] = 'N';                                // N
  root[6] = '.';                                // End     
  root[7] = 0;
  //---Load the file without OPA ------------------------
  CArtParser img(TC_MEDIUM);      // Parse texture
  nTEX  = img.GetNitTexture(xld);
  if (0 == nTEX)  return 0;
  //-----Replace night texture with same resolution -----
  if (Resn == TC_HIGHTR)   DoubleNiTexture(txn,(U_INT *)nTEX);
  return 1;
}
//-----------------------------------------------------------------------
//  Get Texture from pod system
//  NOTE:  For water we only use one texture as they are all the same
//         in the super Tile
//-----------------------------------------------------------------------
int CTextureWard::GetRawTexture(CTextureDef *txn)
{ char  root[32];                       // file name
  strncpy(root,txn->Name,8);            // Root Name
  root[8]   = NameRES[Resn];            // Resolution
  root[9]   = 0;                        // Close name
  _snprintf(xsp.path,TC_TEXTURE_NAME_DIM,"DATA/D%03d%03d/%s.",gx,gz,root);
  //--------Read the RAW and ACT texture file ----------------------
  CArtParser img(Resn);
  img.SetWaterRGBA(GetWaterRGBA(Resn));
  dTEX = img.GetRawTexture(xsp,1);
  //-------Check for night texture ----------------------------------
  U_CHAR nt = txn->IsNight() & NT;      // Nitght texture
  if  (nt == 0)         return 1;
  return NightRawTexture(txn) + 1;      // Load night texture
}
//-----------------------------------------------------------------------------
//	Return Texture from TRN file 
//-----------------------------------------------------------------------------
int CTextureWard::GetTRNtextures(CTextureDef *txn, U_INT qx, U_INT qz)
{	Resn	= txn->Reso[0];									//	Save Resolution
	gx		= qx >> 1;											//	Save Globals Tile X
	gz		= qz >> 1;											//	Save Globals Tile Z
	dTEX	= 0;
	nTEX	= 0;
	GetRawTexture(txn);										// Get textures
	txn->dTEX[0]	= dTEX;									// return day texture
	txn->nTEX[0]  = nTEX;									// return nitght texture
	dTEX	= 0;
	nTEX  = 0;
	//--- Encode texture dimension --------------------------------
	U_INT dim = (xsp.wd << 16) | xsp.ht;
	return dim;
}
//-----------------------------------------------------------------------------
//  Return a full day texture RGBA from file thread
//  opt = 1 =>  This texture is a dedicated texture that may be associated
//               with a OPA mask for water merging
//-----------------------------------------------------------------------------
GLubyte *CArtParser::GetDayTexture(TEXT_INFO &txd,char opt)
{ SqlTHREAD *sql = globals->sql;
  GLubyte   *tex = 0;
  bool       sqb = ((0 == epd) && sql->SQLtex());
  afa            = 0;
  if (sqb) { sql->GetGenTexture(txd);
             SetSide(txd.wd);
           }
  else     { txd.mADR = LoadRaw(txd,opt); }
  if (opa) MergeWater(txd.mADR);
  return txd.mADR;
}
//-----------------------------------------------------------------------------
//  Get a Transition Texture
//  opt =1  shared allowed
//-----------------------------------------------------------------------------
int CTextureWard::GetMixTexture(CTextureDef *txn,U_CHAR opt)
{ //-----Allocate a dedicated texture to the tile -----
  char *gen = txn->Name;
  char *hex = txn->Hexa;
  //----PATH is SYSTEM/GLOBE (Name at position 13)-----
  char *root = xld.path + 13;
  //----format name ----------------------------------
  root[0] = gen[0];                             // Tile type
  root[1] = gen[1];
  root[2] = gen[8];                             // I index
  root[3] = gen[9];                             // J index
  root[4] = NameRES[Resn];                      // 4 or 5
  root[5] = 'D';                                // D/N
  root[6] = '.';                                // End     
  root[7] = 0;
  //---Store requested resolution ----------------------
  xld.res = Resn;
  //---Get the transition number -----------------------
  U_INT tra = txn->Tmask;
  if ((0 == tra) && opt) return GetShdTexture(txn,xld.path);
  //----Load raw and ACT for main tile------------------
  CArtParser img(Resn);
  switch (tra)  {
    case TC_NONTRANSITION:
        dTEX = img.GetDayTexture(xld,0);
        return 1;

    case TC_BOTTRANSITION:
        root[0]   = gen[4];                         // Bottom type
        root[1]   = gen[5];
        xld.mADR  = Blend[(TC_BLENDBT | Resn)].GetMask();  // Store mask
        xld.type  = hex[2];
        img.InitTransitionT1(xld);

        root[0]   = gen[0];
        root[1]   = gen[1];
        xld.type  = hex[0];
        dTEX      = img.Mixer(xld);
        return 1;

    case TC_CNRTRANSITION:
        root[0]   = gen[6];                         // Y type
        root[1]   = gen[7];
        xld.mADR  = Blend[(TC_BLENDCN | Resn)].GetMask();  // Store mask
        xld.type  = hex[3];
        img.InitTransitionT1(xld);

        root[0]   = gen[0];
        root[1]   = gen[1];
        xld.type  = hex[0];
        dTEX      = img.Mixer(xld);
        return 1;

    case TC_BCNTRANSITION:
        root[0]   = gen[4];                         // B type
        root[1]   = gen[5];
        xld.mADR  = Blend[(TC_BLENDBT | Resn)].GetMask();  // Store mask
        xld.type  = hex[2];

        root[0]   = gen[6];                         // Y type
        root[1]   = gen[7];
        xld.mADR  = Blend[(TC_BLENDCN | Resn)].GetMask();
        xld.type  = hex[3];
        img.InitTransitionT2(xld);

        root[0]   = gen[0];
        root[1]   = gen[1];
        xld.type  = hex[0];
        dTEX      = img.Mixer(xld);
        return 1;

    case TC_RGTTRANSITION:
        root[0]   = gen[2];                         // X type
        root[1]   = gen[3];
        xld.mADR  = Blend[(TC_BLENDRT | Resn)].GetMask();  // Store mask
        xld.type  = hex[1];
        img.InitTransitionT1(xld);

        root[0]   = gen[0];
        root[1]   = gen[1];
        xld.type  = hex[0];
        dTEX      = img.Mixer(xld);
        return 1;

    case TC_RBTTRANSITION:
        root[0]   = gen[4];                         // B type
        root[1]   = gen[5];
        xld.mADR  = Blend[(TC_BLENDBT | Resn)].GetMask();  // Store mask
        xld.type  = hex[2];
        img.InitTransitionT1(xld);

        root[0]   = gen[2];                         // X type
        root[1]   = gen[3];
        xld.mADR  = Blend[(TC_BLENDRT | Resn)].GetMask();
        xld.type  = hex[1];
        img.InitTransitionT2(xld);

        root[0]   = gen[0];
        root[1]   = gen[1];
        xld.type  = hex[0];
        dTEX      = img.Mixer(xld);
        return 1;

    case TC_RCNTRANSITION:
        root[0]   = gen[2];                         // X type
        root[1]   = gen[3];
        xld.mADR  = Blend[(TC_BLENDRT | Resn)].GetMask();  // Store mask
        xld.type  = hex[1];
        img.InitTransitionT1(xld);

        root[0]   = gen[6];                         // Y type
        root[1]   = gen[7];
        xld.mADR  = Blend[(TC_BLENDCN | Resn)].GetMask();
        xld.type  = hex[3];
        img.InitTransitionT2(xld);

        root[0]   = gen[0];
        root[1]   = gen[1];
        xld.type  = hex[0];
        dTEX      = img.Mixer(xld);
        return 1;

    case TC_ALLTRANSITION:
      
				root[0]   = gen[2];                         // X type
        root[1]   = gen[3];
        xld.mADR  = Blend[(TC_BLENDRT | Resn)].GetMask();  // Store mask
        xld.type  = hex[1];
        img.InitTransitionT1(xld);

        root[0]   = gen[4];                         // B type
        root[1]   = gen[5];
        xld.mADR  = Blend[(TC_BLENDBT | Resn)].GetMask();
        xld.type  = hex[2];
        img.InitTransitionT2(xld);

        root[0]   = gen[6];                         // Y type
        root[1]   = gen[7];
        xld.mADR  = Blend[(TC_BLENDCN | Resn)].GetMask();
        xld.type  = hex[3];
        img.InitTransitionT3(xld);

        root[0]   = gen[0];
        root[1]   = gen[1];
        xld.type  = hex[0];
        dTEX      = img.Mixer(xld);
        return 1;

    default:
        Abort(txn->Name,"bad transition");
  }
  //------Assign the day texture ----------------------------
  return 1;
}
//-----------------------------------------------------------------------
//  Get Texture from EPD system
//  NOTE:  For water we only use one texture as they are all the same
//         in the super Tile
//-----------------------------------------------------------------------
int CTextureWard::GetEPDTexture(CTextureDef *txn)
{ TEXT_INFO txd;
  char  res = TC_EPDRES;                // Fixed medium resolution
  char  root[32];                       // file name
  strncpy(root,txn->Name,8);            // Root Name
  root[8]   = 0;                        // Day close here
  _snprintf(txd.path,TC_TEXTURE_NAME_DIM,"DATA/D%03d%03d/%s.",gx,gz,root);
  //--------Read the RAW and ACT texture file ----------------------
  CArtParser img(res);
  img.SetEPD();
  img.SetWaterRGBA(GetWaterRGBA(res));
  dTEX  = img.GetDayTexture(txd,1);
  return 1;      // 
}
//-----------------------------------------------------------------------------
//  Get a Generated texture with no coast data
//-----------------------------------------------------------------------------
int CTextureWard::GetGenTexture(CTextureDef *txn)
{ int ntx  = GetMixTexture(txn,1);        // Get Day texture
      ntx += NightGenTexture(txn);        // Get Night Texture
  return ntx;
}
//-----------------------------------------------------------------------------
//  Draw coast  using polygons
//-----------------------------------------------------------------------------
void CTextureWard::MakeStencil(char *lsp)
{ COAST_HEADER *hdr = (COAST_HEADER*)lsp;
  COAST_VERTEX *pol = (COAST_VERTEX*)(lsp + sizeof(COAST_HEADER));
  Dim       = SideRES[Resn];
  U_INT nbp =  hdr->nbp;
  for (U_SHORT k = 0; k != nbp; k++)  pol = DrawStencil(pol);
  return;
}
//-----------------------------------------------------------------------------
//  Draw a closed polygon on the canvas
//  When the first polygon is an inside polygon, it is skipped
//  Inside polygons are used to select a full terrain tile, then the
//  folowing polygons exclude the water part from it
//-----------------------------------------------------------------------------
COAST_VERTEX *CTextureWard::DrawStencil(COAST_VERTEX *pol)
{ U_SHORT nbv       = pol->Nbv;                     // Number of vertices
  COAST_VERTEX *adv = pol;
  pBox      = iBox;                                 // Init bounding box
  U_INT x0  = 0;
  U_INT y0  = 0;
  U_INT x1  = AdjustCoordinate(adv->xPix);
  U_INT y1  = AdjustCoordinate(adv->zPix);
  U_INT xd  = x1;
  U_INT yd  = y1;
  adv++;
 
  for (U_SHORT k = 1; k != nbv; k++)
    { x0  = x1;
      y0  = y1;
      x1  = AdjustCoordinate(adv->xPix);
      y1  = AdjustCoordinate(adv->zPix);
      Color = (y0 < y1)?(1):(2);
      DrawTLine(x0,y0,x1,y1);
      if (x0 < pBox.xmin) pBox.xmin = x0;
      if (x0 > pBox.xmax) pBox.xmax = x0;
      if (y0 < pBox.zmin) pBox.zmin = y0;
      if (y0 > pBox.zmax) pBox.zmax = y0;
      adv++;
    }
  Color = (y1 < yd)?(1):(2);
  if (x1 < pBox.xmin) pBox.xmin = x1;
  if (x1 > pBox.xmax) pBox.xmax = x1;
  if (y1 < pBox.zmin) pBox.zmin = y1;
  if (y1 > pBox.zmax) pBox.zmax = y1;
  DrawTLine(x1,y1,xd,yd);
  //--------Set inside pixel on land ----------------------
  ScanCoast();
  return adv;
}
//-----------------------------------------------------------------------------
//  Build coast texture on the canvas
//  NOTE:  The first pixels on each side are skipped
//-----------------------------------------------------------------------------
void CTextureWard::ScanCoast()
{ int xd = pBox.xmin;
  int xf = pBox.xmax;
  int yd = pBox.zmin;
  int yf = pBox.zmax;
  int lgr   = SideRES[Resn];                          // Line size
  U_CHAR *cnv   = Canvas + (yd * lgr) + xd;           // Canvas start
  U_CHAR *stl   = cnv - lgr ;                         // Start line
  //--------Build the final texture into the canvas-------------------
  for (int nz = yd; nz <= yf; nz++)
  {   Inside = 0;                                 // Line start outside
      State  = TC_PIX_ZERO;                       // State is no color
      cnv    = stl + lgr;                         // Next line
      stl    = cnv;                               // Remember
      for (int nx = xd; nx <= xf; nx++)
      { 
        //----Update Inside state ------------------------------------
        U_CHAR pix  = *cnv;                       // Get current pixel color
        U_CHAR ind  = State | (pix & TC_PIXCOL);  // State entry
        U_CHAR ins  = StatePIX[ind].In | Inside;  // Actual inside 
        State       = StatePIX[ind].nState;       // Next state
        Inside     ^= StatePIX[ind].swap;         // Next Inside indicator
        //----Update inside indicator and clear color -----------------
        *cnv++      = (pix ^ ins) & TC_INSIDE;    // Clear Canvas
      }
  }
  return;
}
//-----------------------------------------------------------------------------
//  Return correct pixel value
//  NOTE:   The pixel coordinate computed by the coast decoder is in range [0-1023]
//          It sould be adjusted as foloow
//          1) Given the final resolution, the coordinate is divided by the facRES
//              For instance for 128 resolution ,the coordinate is divided by 8
//              (it is shifted right by 3 position
//          2) As there are 8 pixels on each tile border for recovery, the real
//            tile surface is shrinked by the factor redRES.  For a
//            128 bits resolution, the shrink factor is 112 / 128
//            This is done by the following operations
//            a) Translate to the tile center
//            b) shrink 
//            c) Translate back
//          3)  As we draw in XOR mode, the same pixel must not be drawed twice,
//              except if they are of different colors
//-----------------------------------------------------------------------------
U_INT CTextureWard::AdjustCoordinate(U_INT val)
{ int c1 = ((val >> shfRES[Resn]) - tranRES[Resn]) * numRES[Resn];
  int c2 = (c1 / SideRES[Resn])   + tranRES[Resn]; 
  return c2;}

//-----------------------------------------------------------------------------
//  Build the texture by choosing pixel either from land or water
//  using the canvas
//  CLR is the clear canvas option
//  For coast tile, the day texture is built before the night texture,
//  leaving the canvas for building the night texture
//------------------------------------------------------------------------------
int CTextureWard::BuildCoastTexture(GLubyte *land)
{ Water = (U_INT*)GetWaterRGBA(Resn);   // Get Ocean Texture
  int     lost  = skipRES[Resn];                      // Lost side pixels 
  int     adj   = lost << 1;                          // Line adjust
  int     skip  = (lost * SideRES[Resn]) + skipRES[Resn] - adj;
  int     lgr   = numRES[Resn];                       // Line size
  U_INT  *lnd   = (U_INT*)land  + skip;               // Land source
  U_INT  *wtr   = Water  + skip;                      // Water source
  U_CHAR *cnv   = Canvas + skip;                      // Canvas
  for (int nz = 0; nz != lgr; nz++)                   // Scan line
    { lnd += adj;                                     // Land start
      wtr += adj;                                     // Water start
      cnv += adj;                                     // canvas start
      for (int nx = 0;nx != lgr;nx++) 
        { if (0 == *cnv) *lnd = *wtr;                 // Water pixel
         *cnv++ = 0;																	// Clear if requetd
          lnd++;                                      // Next land
          wtr++;                                      // Next water
        }
    }
  return 1;
}
//-----------------------------------------------------------------------------
//  Build the texture by computing alpha chanel as follow
//  Land => alpha = 0
//  Water=> alpha = cte;
//  CLR is the clear canvas option
//  For coast tile, the day texture is built before the night texture,
//  leaving the canvas for building the night texture
//------------------------------------------------------------------------------
void CTextureWard::AlphaCoastTexture(U_CHAR clr,GLubyte *land)
{ if (0 == land)  return;
  Water = (U_INT*)GetWaterRGBA(Resn);   // Get Ocean Texture
  int     lost  = skipRES[Resn];                      // Lost side pixels 
  int     adj   = lost << 1;                          // Line adjust
  int     skip  = (lost * SideRES[Resn]) + skipRES[Resn] - adj;
  int     lgr   = numRES[Resn];                       // Line size
  U_INT  *lnd   = (U_INT*)land  + skip;               // Land source
  U_INT  *wtr   = Water  + skip;                      // Water source
  U_CHAR *cnv   = Canvas + skip;                      // Canvas
  for (int nz = 0; nz != lgr; nz++)                   // Scan line
    { lnd += adj;                                     // Land start
      wtr += adj;                                     // Water start
      cnv += adj;                                     // canvas start
      for (int nx = 0;nx != lgr;nx++) 
      { U_INT alf = *lnd & 0xFF000000;                // Land alpha
        U_INT msk = (*cnv)?( alf):(kaf);              // Sea  alpha
        U_INT pix = (*cnv)?(*lnd):(*wtr);             // RGB value
              pix &= 0x00FFFFFF;                      // Change alpha
              pix |= msk;                             // for computed one
             *lnd  = pix;                             // Land pixel
        if (clr) *cnv = 0;                            // Clear if requested
        cnv++;                                        // Next canvas
        lnd++;                                        // Next land
        wtr++;                                        // Next water
        }
    }
  return;
}

//-----------------------------------------------------------------------------
//  Build the texture using the canva
//	Boost the light value according to luminance
//-----------------------------------------------------------------------------
void CTextureWard::BuildNightTexture(U_INT *txt)
{ if (0 == txt) return;
  int     lost  = skipRES[Resn];                      // Lost side pixels 
  int     adj   = lost << 1;                          // Line adjust
  int     skip  = (lost * SideRES[Resn]) + skipRES[Resn] - adj;
  int     lgr   = numRES[Resn];                       // Line size
  U_INT  *lnd   = txt  + skip;                        // texture source
  U_CHAR *cnv   = Canvas + skip;                      // Canvas
	//--------------------------------------------------------------------
	U_INT    pix  = 0;                                  // Pixel entry
  U_CHAR   R    = 0;                                  // Red composite
  U_CHAR   G    = 0;                                  // Green composite
  U_CHAR   B    = 0;                                  // Blue composite
  U_CHAR   A    = 0;                                  // Alpha chanel
	//--------------------------------------------------------------------
  for (int nz = 0; nz != lgr; nz++)                   // Scan line
    { lnd += adj;                                     // Land start
      cnv += adj;                                     // canvas start
      for (int nx = 0;nx != lgr;nx++) 
        { if (*cnv++ == 0) *lnd = 0;                  // outside pixel
					//---- boost the night value ----------
					else
					{	pix   = *lnd & 0x00FFFFFF;
						R     = pix;
						G     = pix >> 8;
						B     = pix >> 16;
						A     = ((R*38) + (G*74) + (B*16)) >> TC_BY128;
						if (A > 60) (A = 255);										// Boost light
						pix  |= (A << 24);
					 *lnd		= pix;
						}
          lnd++;                                      // Next land
        }
    }
  return;
}
//-----------------------------------------------------------------------------
//  Replace Night texture with a double resolution texture
//  NOTE: Night texture is presupposed to be meduim resolution in this routine
//-----------------------------------------------------------------------------
int CTextureWard::DoubleNiTexture(CTextureDef *txn,U_INT *txt)
{ U_INT *src = txt;                       // Source texture
  U_INT  sln = SideRES[TC_MEDIUM];        // Source line size
  U_INT  dim = SizeRES[Resn];             // Target size
  U_INT *dst = new U_INT[dim];            // New array
  U_INT *ln1 = 0;                         // destination Line 1 
  U_INT *ln2 = dst;                       // Destination line 2
  if (0 == dst) Abort("Texture","No more memory");
  for (U_INT z = 0; z != sln; z++)        // One line
  { ln1 = ln2;                            // Start of line 1
    ln2 = ln1 + SideRES[Resn];            // Start of line 2
    for (U_INT x = 0; x != sln; x++)      // Double this line
    { U_INT pix = *src++;                 
      *ln1++    = pix;                    // twice in line 1
      *ln1++    = pix; 
      *ln2++    = pix;                    // twice in line 2
      *ln2++    = pix;
    }
  }
  //-----replace night texture ----------------------------------
  nTEX = (GLubyte *)dst;
  delete [] txt;
  return 1;
}
//----------------------------------------------------------------------
//  Return a shared texture from the map
//----------------------------------------------------------------------
CSharedTxnTex *CTextureWard::GetSharedTex(U_INT key)
{ pthread_mutex_lock (&txnMux);
  std::map<U_INT,CSharedTxnTex*>::iterator itx = txnMAP.find(key);
  CSharedTxnTex *shx = (itx == txnMAP.end())?(0):((*itx).second);
  //-----Assign the texture -------------------------------------
  if (shx)  shx->Use++;
  pthread_mutex_unlock (&txnMux);
  return shx;
}
//-----------------------------------------------------------------------------
//  Build a Night texture matching the resolution for a TRN tile
//-----------------------------------------------------------------------------
int CTextureWard::NightRawTexture(CTextureDef *txn)
{ char  root[32];                       // file name
  strncpy(root,txn->Name,8);            // Root Name
  root[8]   = '5';                      // Name is always 5
  root[9]   = 'N';                      // Night indicator
  root[10]  = 0;
  _snprintf(xsp.path,TC_TEXTURE_NAME_DIM,"DATA/D%03d%03d/%s.",gx,gz,root);
  //-----READ the Night texture file with ----------------
  CArtParser img(TC_MEDIUM);						// Reader instance
  nTEX		= img.LoadRaw(xsp,0);
  if (0 == nTEX)  return 0;
  img.MergeNight(nTEX);
  //-----Replace night texture with same resolution ------
  if (Resn == TC_HIGHTR)   DoubleNiTexture(txn,(U_INT*)nTEX);
  return 1;
}

//-----------------------------------------------------------------------------
//  Locate Texture from cache list.  Shared texture has a unic 4 char key:
//  1) for Terrain texture
//  2) For runways
//  3) For water
//  4) For Taxiways
//
//  NOTE:   Due to coast tile some transition between SEA and LAND are not allowed
//          thus request for water may be routed here after the transition test 
//          is passed.
//  When the shared texture does not exist, it is created and reserved
//-----------------------------------------------------------------------------
int CTextureWard::GetShdTexture(CTextureDef *txn,char *rawn)
{ U_INT key = KeyForTerrain(txn,Resn);       //(Resn << TC_BYWORD) | txn->Key;     // Full key
  //----------------------------------------------------
  dTEX = 0;
  txn->TypTX = TC_TEXSHARD;                       // Requalify type
  CSharedTxnTex *shx  = GetSharedTex(key);
  if (shx)            return 0;
  //------Load texture as a shared one ------------------
  TEXT_INFO txd;
  strncpy(txd.path,rawn,FNAM_MAX);
  CArtParser img(Resn);
  shx		= new CSharedTxnTex(txn->Name,Resn);
  GLubyte *tex	= img.GetDayTexture(txd,0);
  shx->SetDayTexture(0,tex);
  //-----Register this new texture ----------------------
  pthread_mutex_lock (&txnMux);
  txnMAP[key] = shx;
  pthread_mutex_unlock (&txnMux);
  return 0;
}

//==========END OF THIS FILE ======================================================