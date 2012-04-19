//=========================================================================================
// WinSketch.h
//
// Part of Fly! Legacy project
//
// Copyright 2003-2005 Chris Wallace
// copyright 2007-2012 Jean Sabatier
// Fly! Legacy is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
// Fly! Legacy is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
//   along with Fly! Legacy; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//=========================================================================================

#ifndef WINCITY_H
#define WINCITY_H
//=========================================================================================
#include "../Include/FlyLegacy.h"
#include "../include/globals.h"
#include "../Include/Fui.h"
#include "../include/FuiUser.h"
#include "../include/3DMath.h"
#include "../Include/OSMobjects.h"
#include "../Include/Triangulator.h"
#include "../Include/RoofModels.h"
//==========================================================================================
//	State values
//==========================================================================================
#define PROF_SKETCH   (PROF_EDITOR+PROF_NO_TEL+PROF_NO_PLANE)
#define SKETCH_REFER (0)
#define SKETCH_PAUSE (1)
#define SKETCH_NEXT  (2)
#define SKETCH_WAIT  (4)
#define SKETCH_OPEN	 (5)
#define SKETCH_LOAD  (6)
#define SKETCH_SHOW  (7)
#define SKETCH_ENDL  (8)
#define SKETCH_WSEL  (11)
#define SKETCH_ROBJ  (12)
#define SKETCH_ABORT (13)
#define SKETCH_SAVEA (14)
#define SKETCH_SAVEB (15)
#define SKETCH_SAVED (16)
#define SKETCH_SAVEE (17)
#define SKETCH_SAVEF (18)
#define SKETCH_PODMS (19)
//==========================================================================================
#define CITY_FILE_OFE		(0)
#define CITY_FILE_MOD   (1)
#define CITY_FILE_DBA		(2)
//==========================================================================================
class CFuiFileBox;
extern char *Dupplicate(char *s, int lgm);
//==========================================================================================
//  Window to display building sketch
//==========================================================================================
class CFuiSketch : public CFuiWindow
{	//--- ATTRIBUTES ----------------------------------------------
	FILE          *FP;						// File pointer
	fpos_t				fpos;						// File position
	//--- File name -----------------------------------------------
	char					fnam[PATH_MAX];
	//--- STATE ---------------------------------------------------
	U_INT					State;					// Logical state
	U_INT					nStat;					// Next state after wait
	U_INT         statP;					// Positive modal state
	//-------------------------------------------------------------
	U_INT					ident;					//
	U_INT					seqNo;					// Sequence number
	//-------------------------------------------------------------
	U_INT					cntw;						// Waiting counter
	SPosition     rpos;						// Reference position;
	SPosition     geop;						// Building position
	double				alti;						// Altitude
	//--- Database parameters -------------------------------------
	U_INT					qKey;						// QGT key
	U_INT					Stamp;					// Object count
	U_INT					objno;					// Object current
	U_INT					count;					// Object saved
	//-------------------------------------------------------------
	SPosition     SW;							// South west corner
	SPosition     NE;							// NE corner
	//-------------------------------------------------------------
	U_CHAR				tera;						// Terrain indicator
	U_CHAR				scny;						// Scenery object
	U_CHAR        eofl;						// End of file
	U_CHAR        wait;						// Waiting terrain
	U_CHAR				edit;						// Edit step
	U_CHAR        wfil;						// Waiting file
	U_CHAR        grnd;						// Altitude option
	//--- Dialogue button -----------------------------------------
	Tag					  dial;
	//--- OSM object attributs ------------------------------------
	//U_INT					otype;					// Object type
	OSM_CONFP			confp;					// Configuration
	char					tagn[64];				// Tag name
	char					valn[64];				// Value name
	//--- Replacing attributs for object --------------------------
	OSM_REP       repPM;					// Replacement parameters
	double				orien;					// Replacing orientation
	//-------------------------------------------------------------
	int					  time;
	//--- Database interface --------------------------------------
	SQL_DB       *sqlp;
	//---- Search file parameters ---------------------------------
	FILE_SEARCH   FPM;
	char					spath[PATH_MAX];
	char					sname[PATH_MAX];
	char          dbase[PATH_MAX];
	char          rpack[PATH_MAX];
	//-------------------------------------------------------------
	CFuiGroupBox	*gBOX;					// Option group box
	CFuiLabel			*aLAB;					// Label for group
  CFuiCheckBox  *fOPT;					// fill Option box
  CFuiCheckBox  *vOPT;					// View Option box
	CFuiGroupBox  *Box2;					// Box number 2
	CFuiButton    *nBUT;					// Next button
	CFuiButton    *vTER;					// View terrain option
	CFuiButton		*vALL;					// View all
	CFuiButton    *nSTY;					// Change style
	CFuiButton    *dOBJ;					// Delete object
	CFuiButton    *gOBJ;					// Restore object
	CFuiButton    *rOBJ;					// Replace object
	CFuiButton    *lROT;					// Left rotation
	CFuiButton    *rROT;					// Right rotation
	CFuiLabel     *nBAT;					// Building n°
	CFuiLabel     *nTAG;					// Building n°
	CFuiList      *sWIN;					// style list
	CFuiGroupBox  *Box3;					// Separator
	CFuiGroupBox  *Box4;
	CFuiButton    *bTRY;					// Try button
	CFuiButton    *bSAV;					// Save button
	//--- Titles --------------------------------------------------
	CSlot					 styTT;					// style title
	CSlot          modTT;					// Model title
	//-------------------------------------------------------------
	CListBox       sBOX;          // style box
	//---- Text for terrain button --------------------------------
	char          *vTer;					// Text
	//-------------------------------------------------------------
	COption        optD;					// Option box
	//--- Radian increment for One degre --------------------------
	double				 oneD;
	//-------------------------------------------------------------
	CBuilder        *trn;					// CBuilder
	D2_Session       ses;					// Session
	//---Original context -----------------------------------------
  CAMERA_CTX       ctx;         // Original camera and situation
	//--- Rabbit camera -----------------------------------------
	CRabbitCamera   *rcam;
	double           range;
	//-------------------------------------------------------------
public:
	CFuiSketch(Tag idn, const char *filename);
  virtual ~CFuiSketch();
  //--- File handling -----------------------------------------
	void	SetOptions(U_INT p);
	bool	ParseArea();
	bool	ParseBuilding();
	bool	ParseFile();
	bool	ParseStyle(char *txt);
	bool	ParseVTX(char *txt);
	bool	ParseHOL(char *txt);
	bool	ParseTAG(char *txt);
	bool	ParseReplace(char *txt);
	void	HaveVertex(double x, double y);
	bool	BuildObject();
	//------------------------------------------------------------
	void	FileSelected(FILE_SEARCH *pm);
	int  	NoSession();
	int  	BuildStyList();
	//------------------------------------------------------------
	int	  EditBuilding();
	bool	AutoReplace();
	//------------------------------------------------------------
	U_INT	Abort(char *err,char *fn);
	U_INT	GotoReferencePosition();
	U_INT	HereWeAre();
	U_INT	OneBuilding();
	void	ShowStyle();
	//--- Terrain ------------------------------------------------
	U_INT	TerrainView();
	U_INT	TerrainHide();
	U_INT	TerrainWait();
	//------------------------------------------------------------
	U_INT	StartLoad();
	U_INT	ReadNext();
	U_INT	ShowBuilding();
	U_INT	EndLoad();
	//--- Models -------------------------------------------------
	void	CollectModels();
	//--- Actions ------------------------------------------------
	bool	EditError();
	void	ChangeStyle();
	void	RemoveObject();
	void	RestoreObject();
	void	RotateBuilding(double sa);
	void	ReplaceBuilding();
	//--- Picking object -----------------------------------------
	void  OnePicking(U_INT No);
	bool	NoSelection();
	//--- Options ------------------------------------------------
	void	Swap3D();
	//------------------------------------------------------------
	void	Write();
	void	FlyOver();
	void	BuildDBname();
	bool	CollectBases();
	U_INT SaveStepA();
	U_INT SaveStepB();
	U_INT	SaveStepD();
	U_INT	SaveCheck();
	U_INT OpenDatabase(char **sc);
	U_INT SaveObject();
	U_INT SaveEnd();
	//--- mouse --------------------------------------------------
	bool	MouseCapture(int mx, int my, EMouseButton bt);
	//--- Keyboard -----------------------------------------------
	void	TimeSlice();
	//--- Drawing() ----------------------------------------------
	void	Draw();
	void	OnDialog(Tag rep);
	void	NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
	//--- Dont close tthe windows ---------------------------------
	bool  CheckProfile(char a);
};
//============================END OF FILE =================================================
#endif // WINCITY_H
