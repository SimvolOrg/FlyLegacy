//==============================================================================================
// WinTaxiway.h
//
// Part of Fly! Legacy project
//
// Copyright 2003 Chris Wallace
//
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
//================================================================================================

#ifndef WINTAXIWAY_H
#define WINTAXIWAY_H
#include "../Include/FlyLegacy.h"
#include "../include/globals.h"
#include "../Include/Fui.h"
#include "../Include/Cameras.h"
//================================================================================================
#define PROF_TAXI   (PROF_EDITOR+PROF_NO_TEL+PROF_NO_PLANE)
//================================================================================================
class TaxNODE;
class TaxEDGE;
#define MODE_PICKING (0)
#define MODE_LINKING (1)
//================================================================================================
//	Class Taxiway tracker
//================================================================================================
class TaxiTracker: public CExecutable, public Tracker {
	//--- ATTRIBUTES -------------------------------------
	U_INT		 Seq;								// Node sequence
	TaxNODE *selN;							// Selected node
	//--- Link position ----------------------------------
	SPosition *lpos;						// Link position
	//--- Quadric ----------------------------------------
	GLUquadricObj *disk;
	//--- List of nodes ----------------------------------
	std::map<Tag,TaxNODE*> nodQ;		// TaxNODE
	std::map<Tag,TaxEDGE*> edgQ;		// TaxEDGE
	//--- Iterators ---------------------------------------
	std::map<Tag,TaxNODE*>::iterator rn;
	std::map<Tag,TaxEDGE*>::iterator re;
protected:
	//--- METHODS ----------------------------------------
public:
	TaxiTracker();
 ~TaxiTracker();
	//-----------------------------------------------------
	void			Clear();
	void			AddNode(SPosition &P);
	void			AddEdge(Tag A, Tag B);
	void			DelNode();
	void			DelEdge(Tag tn);
	bool			MoveNode(SPosition &P);
	//-----------------------------------------------------
	void			SetSelection(Tag id);
	Tag				GetSelected();
	//--- Drawing ------------------------------------------
	void			Draw();
	void			DrawMarks();
	void			DrawLink();
	void			DrawNode(TaxNODE *N);
	void			DrawEdge(TaxEDGE *edg);
	void			DrawLine(SPosition &P, SPosition &Q);
	//---inline operators ---------------------------------
	void			SetLink(SPosition *P)	{lpos = P;}
};
//================================================================================================
//	Class Taxiway editor
//================================================================================================
class CFuiTaxi: public CFuiWindow 
{	//--- ATTRIBUTES ---------------------------------------
	CFuiLabel  *cplb;							// Cursor position
	CFuiLabel  *gplb;							// Geo position
	//-- Taxiway tracker -----------------------------------
	TaxiTracker trak;
	Tag					edgA;							// Edge A
	Tag					selN;							// Selected Node
	int					sx,sy;						// Screen coordinates
	//-- Taxiways Management -------------------------------
	CAptObject *apo;							// Airport to edit
	CAirport   *apt;							// Description
	SPosition		org;							// Airport origin
	SPosition   geop;							// Center screen position
	SPosition   cpos;							// Cursor geoposition
	//--- Menus --------------------------------------------
  FL_MENU     menu;             // menu descriptor
	//--- Various modes -------------------------------------------
	char        mode;							// Mouse mode
	bool        mmov;							// Moving selection
	//---Original context -----------------------------------------
  CAMERA_CTX       ctx;         // Original camera and situation
	//--- Rabbit camera -----------------------------------------
	CRabbitCamera   *rcam;				// Object camera
	//--- METHODS -----------------------------------------------
public:
	CFuiTaxi(Tag idn, const char *filename);
 ~CFuiTaxi();
	//------------------------------------------------------------
	void  Validate();
	void	CursorGeop(int mx, int my);
	void	PositionEdit(SPosition P, CFuiLabel *lab, char *type);
	void	NewPosition(SPosition P);
	//--- Click routines -----------------------------------------
	bool	CenterOnSpot(int mx, int my);
  void	LinkMode();
	bool	PickMode(U_INT No);
	//------------------------------------------------------------
	// Mouse management methods
	bool	OpenMenu(int mx,int my);
	bool  InsideMove (int x, int y);
	bool	MouseCapture   (int mx, int my, EMouseButton bt);
	bool	StopClickInside(int mx, int my, EMouseButton bt);
	bool	MousePicking(int mx, int my);
	bool	MouseLinking(int mx, int my);
	void	OnePicking(U_INT No);
	bool	MovingNode(int mx, int my);
	//------------------------------------------------------------
	void	TimeSlice();
	//--- Menu management ----------------------------------------
	int		ClickEditMENU(short itm);
	//--- Notifications ------------------------------------------
	void	NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
	void	NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn);
	//--- Bypass check profile step  -----------------------------
	bool  CheckProfile(char a) {return true;}
};

//======================= END OF FILE ==============================================================
#endif // VERSION_H
