//==============================================================================================
// ScenerySet.h
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

#ifndef SCENERY_SET_H
#define SCENERY_SET_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "../Include/FlyLegacy.h"
//===================================================================================
class CSceneryDBM;
struct SQL_DB;
//==================================================================================
//	Class CSceneryPOD to hold a scenery POD associated to a QGT or a GBT
//==================================================================================
class CSceneryPOD {
	//--- ATTRIBUTES --------------------------------------------
protected:
	int			users;											// User count
	U_INT		Key;												// QGT key
	char    fName[PATH_MAX];						/// Pod Full name
	//--- Methods -----------------------------------------------
public:
	CSceneryPOD(U_INT key, char *fn);
	//------------------------------------------------------------
	void		Mount();
	void		Remove();
	//------------------------------------------------------------
	inline U_INT GetKey()							{return Key;}
  inline char *GetName()						{return fName;}
	inline void  IncUser()						{users++;}
};
//==================================================================================
//	Class CSceneryPack to hold all scenery associated to a QGT
//	A list of ScenerySet is maintain in this Pack
//==================================================================================
class CSceneryPack {
	friend class CSceneryDBM;
private:
	//--- ATTRIBUTES ------------------------------------
	U_INT key;												// QGT key
	U_INT	gx;													// X index
	U_INT gz;													// Z index
	std::vector<CSceneryPOD*> apod;		// POD
	std::vector<std::string>  aosm;	  // OSM database
	//----------------------------------------------------
public:
	CSceneryPack(U_INT key);
 ~CSceneryPack();
  //-----------------------------------------------------
	void	AddPod(CSceneryPOD *pod)	{apod.push_back(pod);}
	void	AddOSM(char *n)						{aosm.push_back(n);}
	bool	AllreadyIN(char *pn);
	//-----------------------------------------------------
	void	MountPODs (CSceneryDBM *dbm);
	void	RemovePODs(CSceneryDBM *dbm);
	//---OSM interface ------------------------------------
	const char *GetOSMname(U_INT k);
	};
//==================================================================================
// CSceneryDBM
//
// The scenery set database contains information about sliced scenery areas read
//   from .SCF (Scenery Control Files).  Each SCF is encapsulated by a CScenerySet
//   instance and represents a rectangular area of contiguous sliced scenery.
//
// This is a singleton class.
//
//  JS: As one scenery file is located in a QGT, might as well register each
//      scenery in a map with the QGT coordinates as a key
//============================================================================
class CSceneryDBM {
	friend class CSceneryPack;
public:
	CSceneryDBM();
 ~CSceneryDBM();
public:
  //--- CSceneryDBM methods ---------------------------
  void    Init (void);
  void    Cleanup (void);
	void		AddPodToQGT(CSceneryPOD *pod);
	void		AddPodToGBT(CSceneryPOD *pod);
	int		  CheckForScenery(PFSPODFILE *p);
	//-----------------------------------------------------------------------
	CSceneryPack *GetQGTPack(U_INT key);
	//--- OSM interface -----------------------------------------------------
	int 				AddOSMbase(char *fn);
	void				AddGQTforOSM(U_INT key);
	void				LoadBasesOSM(C_QGT *qgt);
	void				FreeBasesOSM(SQL_DB *db);
	SQL_DB		 *GetOSMbase(C_QGT *qgt, int nb);
	void				AddOSMrequest(OSM_DBREQ *r);
	OSM_DBREQ  *PopOSMrequest();
	void				FlushOSM();
	//-----------------------------------------------------------------------
	void		MountAll();
  void    Register(C_QGT *qgt);
	void    Deregister(U_INT key);
	//-----------------------------------------------------------------------
	//--- For shared scenery pod --------------------------------------------
	void		MountSharedPod(char *pat,char *fn);
  //  Load initial files --------------------------------------------------
protected:
  void    LoadInFolderTree (const char *path);
	void		Warn01(char *fn);
	void		SendPOD(char *pn);
	bool		AllreadyIN(U_INT key,char *pn);
	//------------------------------------------------------------------------
	int     CheckDatabase(char *pn);
	void		LookForPOD(char *path);
	void		ProcessPOD(char *path,char *fn);
	int			GetSceneryType(char *path);
	int 		SceneryForGBT(PFSPODFILE *p,int gx,int gz);
  //------------------------------------------------------------------------
protected:
	PFS *pfs;
	U_CHAR	tr;													// Trace indicator
	int		  exp;
	SQL_DB *cdb;												// Current database
	char fname[PATH_MAX];
	char path [PATH_MAX];
	std::map<U_INT,CSceneryPack*> sqgt;
	std::map<U_INT,CSceneryPack*> gbtP;
	//--- Queue of database descriptor ----------------------------------------
	qHDR	<SQL_DB>		osmQ;
	//--- Queue of Database request for OSM -----------------------------------
	qHDR  <OSM_DBREQ> reqQ;
};




//======================= END OF FILE ==============================================================
#endif // SCENERY_SET_H
