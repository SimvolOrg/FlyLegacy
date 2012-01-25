/*
 * ScenerySet.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2004 Chris Wallace
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

/*! \file ScenerySet.cpp
 *  \brief Implements CScenerySet sliced scenery chunk
 */

#include "../Include/FlyLegacy.h"
#include "../Include/Globals.h"
#include "../Include/Ui.h"
#include "../Include/Terrain.h"
#include "../Include/Fui.h"
#include "../Include/Pod.h"

using namespace std;
//==========================================================================
//   Global function to process a file from a POD 
//==========================================================================
int ScnpodCB(PFS *pfs, PFSPODFILE *p)
{	int stop = globals->scn->CheckForScenery(p);
	return stop;
}
//==========================================================================
//	Scenery POD to define a POD for scenery
//==========================================================================
CSceneryPOD::CSceneryPOD(U_INT key, char *fn)
{	users	= 0;
	Key		= key;
	strncpy(fName,fn,PATH_MAX-1);
}
//---------------------------------------------------------------
//	Mount the POD file
//---------------------------------------------------------------
void CSceneryPOD::Mount()
{	U_INT No = paddpod (&globals->pfs, fName);
	if (0 == No)		return;
	//--- A new pod is mouted ---------------------
	SCENE("   Ticket(%06d) Mount %s",No,fName);
	return;
}
//---------------------------------------------------------------
//	Remove the POD file
//	Decrement user count and unmout when no more users
//---------------------------------------------------------------
void CSceneryPOD::Remove()
{	U_INT No = premovepod (&globals->pfs, fName);
  if (0 == No)		return;
	SCENE("   Ticket(%06d) Unmount %s",No,fName);
	return;
}
//=================================================================================
// CSceneryDBM
//
//=================================================================================
CSceneryDBM::CSceneryDBM()
{	globals->scn	= this;	
	Init();
}	

//--------------------------------------------------------------------------------
// "Constructor" for scenery set database singleton.  Search for any scenery files
//   that describe sliced scenery areas.  Scenery files are stream files
//   with the .SCF extension that reside in any sub-directory from the
//   /Scenery folder.
//	NOTE: When configured to export Objects, Models or TRN files, all POD are mounted
//----------------------------------------------------------------------------------
void CSceneryDBM::Init (void)
{ // Buffer for the current search path
  char searchPath[PATH_MAX];
	int pm;
	pfs = &globals->pfs;
	//--- Check for export ---------------------------
	exp	= 0;
	pm	= 0;
	GetIniVar("SQL","ExpM3D",&pm);			// Models
	exp |= pm;
	pm	= 0;
	GetIniVar("SQL","ExpOBJ",&pm);			// Objects
	exp |= pm;
	pm	= 0;
	GetIniVar("SQL","ExpTRN",&pm);
	exp |= pm;
  //--- Load SCF files from FlyLegacy /Scenery folder
  strcpy (searchPath, "SCENERY/");
  LoadInFolderTree (searchPath);
	SCENE("========END INITIAL LOAD==================");
	return;
}

//--------------------------------------------------------------------------------
//	Add a pod to the QGT pack
//--------------------------------------------------------------------------------
void	CSceneryDBM::AddPodToQGT(CSceneryPOD *pod)
{	U_INT key = pod->GetKey();
	std::map<U_INT,CSceneryPack*>::iterator ip = sqgt.find(key);
	if (ip != sqgt.end())	{(*ip).second->AddPod(pod); return;}
	//---- create a new pack ----------------------------------
	CSceneryPack *pak = new CSceneryPack(key);
	pak->AddPod(pod);
	sqgt[key] = pak;
	return;
}
//--------------------------------------------------------------------------------
//	Add a pod to the GBT pack
//--------------------------------------------------------------------------------
void	CSceneryDBM::AddPodToGBT(CSceneryPOD *pod)
{	U_INT key = pod->GetKey();
	std::map<U_INT,CSceneryPack*>::iterator ip = sgbt.find(key);
	if (ip != sgbt.end())	{(*ip).second->AddPod(pod); return;}
	//---- create a new pack ----------------------------------
	CSceneryPack *pak = new CSceneryPack(key);
	pak->AddPod(pod);
	sgbt[key] = pak;
	return;
}
//--------------------------------------------------------------------------------
//	Destructor
//--------------------------------------------------------------------------------
CSceneryDBM::~CSceneryDBM()
{	Cleanup();
}
//--------------------------------------------------------------------------------
// "Destructor" for scenery set database singleton.  This method should be called
//   when the database is no longer needed to avoid an apparent memory leak.
//--------------------------------------------------------------------------------
void CSceneryDBM::Cleanup (void)
{	//----Clean QGT list -------------------------
	std::map<U_INT,CSceneryPack*>::iterator ip;
	for (ip=sqgt.begin(); ip!=sqgt.end(); ip++) delete (*ip).second;
	//----Clean GBT list -------------------------
	std::map<U_INT,CSceneryPack*>::iterator ig;
	for (ig=sgbt.begin(); ig!=sgbt.end(); ig++) delete (*ig).second;
}

//--------------------------------------------------------------
// Search for POD in this directory 
//--------------------------------------------------------------
void CSceneryDBM::LookForPOD(char *path)
{	int sh = 0;
	SCENE("SEARCH POD in: %s", path);
  // Iterate over all files in this folder.  Sub-folders are ignored
  ulDir*  dirp = ulOpenDir (path);
	ulDirEnt* dp = 0;
  if (dirp) {
    
    while ((dp = ulReadDir(dirp)) != NULL )
		{ if (dp->d_isdir)						continue;
			//--- Qualify shared scenery --------------------------
			sh = GetSceneryType(path);
			//--- a file is found ---------------------------------
      // Check for file extension .POD
      char *ext = strrchr (dp->d_name, '.');
			if (0 == ext)								continue;
      if ((_stricmp (ext, ".EPD") == 0) || (_stricmp(ext, ".POD") == 0))
			{	char *fn = dp->d_name;
				if (sh)		MountSharedPod(path,fn);
			  else			ProcessPOD(path,fn);				}
    }
  }
    ulCloseDir(dirp);
		return;
}

//--------------------------------------------------------------
// Check for shared scenery
//--------------------------------------------------------------
int	CSceneryDBM::GetSceneryType(char *path)
{	char *sdr = "SCENERY/SHARED";
	int   lgr = strlen(sdr);
	_strupr(path);
	char *deb = path;
	if (':' == path[1])		deb += 3;		// Bypass drive				
	//--- Search for SHARED directory ------------------
	while (deb)	
	{	if (strncmp(deb,sdr,lgr)== 0)		return 1;
		deb	= strchr(deb+1,'/');
	}
	return 0;
}
//--------------------------------------------------------------
// Pod in database 
//--------------------------------------------------------------
void CSceneryDBM::Warn01(char *fn)
{	SCENE("      POD %s already in SQL base",fn);
	return;
}
//--------------------------------------------------------------
// Mount pod 
//--------------------------------------------------------------
void CSceneryDBM::SendPOD(char *pn)
{	paddpod(pfs,pn);
  SCENE("   Mount %s",pn);
	return;
}
//--------------------------------------------------------------
// Process this POD file 
//--------------------------------------------------------------
void CSceneryDBM::ProcessPOD(char *path,char *fn)
{	char *scn = "SCENERY/";
  int   lgr = strlen(scn);
	int   lim = PATH_MAX - 1;
	char name[PATH_MAX];
	//--- mount pod if exporting -------------
	_snprintf(name,lim,"%s%s",path,fn);
	if  (exp)	 return SendPOD(name); 
	//--- First search pod in databases ------
	char *pn  = strstr(path,scn) + lgr;
	_snprintf(name,lim,"%s%s",pn,fn);
	int   in  = CheckDatabase(name);
	if (in)		return Warn01(name);
	//--- Search the QGT ---------------------
  _snprintf(name,lim,"%s%s",path,fn);
	char *dbp = strstr(name,scn) + lgr;
	strncpy(podn,dbp,lim);
	podn[lim] = 0;
	//--- Look into each files ---------------
	scanpod(pfs,name,ScnpodCB);

}
//--------------------------------------------------------------
// Check for scenery to process in each file in the POD
//	return 0 => Continue to look into POD
//	return 1 => Stop to look
//--------------------------------------------------------------
int CSceneryDBM::CheckForScenery(PFSPODFILE *p)
{	char *fn = p->name;
	int   gx = 0;										// Global tile X
	int   gz = 0;										// Global tile Z
	int   sx = 0;										// QGT Y
	int		sz = 0;										// QGT Z
	if (strncmp(fn,"DATA/",5) != 0)	return 0;
	char *fn1 = fn + 5;
	int nf	= sscanf(fn1,"D%3u%3u/",&gx,&gz);
	if (nf != 2)		return 0;
	//---- There is a DATA indication ----------------------
	char *dot = strchr(fn1,'.');
	if (0 == dot)		return SceneryForGBT(p,gx,gz);
	nf			= sscanf(dot,".S%1u%1u",&sx,&sz);
	if (nf != 2)		return SceneryForGBT(p,gx,gz);
	//--- This is a scenery file XXX.s00 to .S11 -----------
	gx = (gx << 1) + sx;						// QGT X index
	gz = (gz << 1) + sz;						// QGT Z index
	U_INT	key = QGTKEY(gx,gz);			// QGT key
	//--- New file for QGT -----------------
	fn	= p->name;
	CSceneryPOD *pod = new CSceneryPOD(key,p->pod->name);
	AddPodToQGT(pod);
	SCENE("      ***** ADD QGT(%03d-%03d) SCENERY %s",gx,gz,fn);
	return 1;
}
//--------------------------------------------------------------
//	We check the database (if any) for this pod file
//	0 => Not in database
//--------------------------------------------------------------
int CSceneryDBM::CheckDatabase(char *pn)
{	//--- Search in scenery database ------------------
	if (0 == globals->objDB)			return 0;
	bool in = globals->sqm->SearchPODinOBJ(pn);
	return (in != 0);
}
//-----------------------------------------------------------------
//	Load the scenery from path
//-----------------------------------------------------------------
int CSceneryDBM::SceneryForGBT(PFSPODFILE *p,int gx,int gz)
{	U_INT key					= QGTKEY((gx << 1),(gz << 1));
	CSceneryPOD *pod	= new CSceneryPOD(key,p->pod->name);
	AddPodToGBT(pod);
	SCENE("      ***** ADD GBT(%03d-%03d) SCENERY %s",gx,gz,p->pod->name);
	return 1;
}
//-----------------------------------------------------------------
//	Load the scenery from path
//-----------------------------------------------------------------
void CSceneryDBM::LoadInFolderTree (const char *path)
{
  // Load POD files from this folder
	LookForPOD((char*)path);
  // Recursively load SCF files from subdirectories
  ulDir* dirp = ulOpenDir (path);
  if (dirp != NULL) {
    ulDirEnt* dp;
    while ((dp = ulReadDir(dirp)) != NULL )
    {
      if (dp->d_isdir &&
          (strcmp (dp->d_name, ".") != 0) &&
          (strcmp (dp->d_name, "..") != 0))
      {
        // This is a sub-folder, attempt to load scenery files in it
        char newPath[PATH_MAX];
				_snprintf(newPath,(PATH_MAX-1),"%s%s/",path,dp->d_name);
        LoadInFolderTree (newPath);
      }
    }
    ulCloseDir(dirp);
  }
}
//=============================================================================
//  Registration of a position is the trigger for loading of available
//    scenery sets; this method is called whenever the terrain manager
//    assigns a new QGT.
//	The QGT key is used to activate all sceneries registered for this QGT
//=============================================================================
//	Register by QGT key
//------------------------------------------------------------------------------
void CSceneryDBM::Register (U_INT key)
{	char reg = 1;
	U_INT gx = (key >> 16);
	U_INT gz = (key & 0xFFFF);
	std::map<U_INT,CSceneryPack*>::iterator p1 = sqgt.find(key);
	if (p1 != sqgt.end())
	{//--- Activate all sceneries in QGT --------------------
		if (reg)		{SCENE("QGT (%03d-%03d) REGISTER",gx,gz); reg = 0;}
		(*p1).second->MountPODs(this);
	}
	//--- Look for global tile scenery ----------------------
	U_INT gbk = key & 0xFFFEFFFE;
	std::map<U_INT,CSceneryPack*>::iterator p2 = sgbt.find(gbk);
	if (p2 == sgbt.end())	 return;
	CSceneryPack *pak = (*p2).second;
	if (reg)	SCENE("GBT (%03d-%03d) REGISTER",gx,gz);
	pak->MountPODs(this);
	return;
}
//------------------------------------------------------------------------------
//	Mount all sceneries
//------------------------------------------------------------------------------
void CSceneryDBM::MountAll()
{	std::map<U_INT,CSceneryPack*>::iterator p1;
	for (p1 = sqgt.begin(); p1 != sqgt.end(); p1++)
	{	CSceneryPack *pak = (*p1).second;
	  pak->MountPODs(this);
		U_INT gx	= pak->gx;
		U_INT gz  = pak->gz;
		SCENE("SCENERY MOUNTED for QGT (%03d-%03d)",gx,gz);
	}
	//--------------------------------------------------
	for (p1 = sgbt.begin(); p1 != sgbt.end(); p1++)
	{ CSceneryPack *pak = (*p1).second;
	  pak->MountPODs(this);
		U_INT gx	= pak->gx;
		U_INT gz  = pak->gz;
		SCENE("SCENERY MOUNTED for GBT (%03d-%03d)",gx,gz);
	}

	return;
}
//------------------------------------------------------------------------------
//	Deregister by Key
//------------------------------------------------------------------------------
void CSceneryDBM::Deregister (U_INT key)
{	U_INT gx = (key >> 16);
	U_INT gz = (key & 0xFFFF);
	int  reg = 1;
	std::map<U_INT,CSceneryPack*>::iterator p1 = sqgt.find(key);
	if (p1 != sqgt.end())	
	{	//--- remove all sceneries in QGT ---------------
		if (reg)	{	SCENE("QGT (%03d-%03d) UNREGISTER",gx,gz); reg = 0; }
		(*p1).second->RemovePODs(this);
	}
	//--- Look for global tile scenery ----------------------
	U_INT gbk = key & 0xFFFEFFFE;
	std::map<U_INT,CSceneryPack*>::iterator p2 = sgbt.find(gbk);
	if (p2 == sgbt.end())	return;
	if (reg)	SCENE("GBT (%03d-%03d) UNREGISTER",gx,gz); 
	(*p2).second->RemovePODs(this);
	return;
}
//=================================================================================
// CSceneryDBM
//	Special mounting for shared scenery
//	check if pod file already in Database
//=================================================================================
void CSceneryDBM::MountSharedPod(char * path,char *fn)
{	char *scn = "SCENERY/";
  int   lgr = strlen(scn);
	int   lim = PATH_MAX - 1;
	char  name[PATH_MAX];
	//--- First search pod in databases ------
	char *pn  = strstr(path,scn) + lgr;
	_snprintf(name,lim,"%s%s",pn,fn);
	int   in  = CheckDatabase(name);
	if (in)	return Warn01(fn);
	//--- Mount the pod file -----------------
	U_INT tk = paddpod (&globals->pfs, fn);
	SCENE("   Ticket(%06d) Mount %s",tk,fn);
	return;
}
//==============================================================================
//	Scenery pack:  held all sceneries for a given QGT
//==============================================================================
CSceneryPack::CSceneryPack(U_INT k)
{	key	= k;
	gx	= (k >> 16);
	gz  = k & 0xFFFF;
}
//------------------------------------------------------------------
//	Destroy everything
//------------------------------------------------------------------
CSceneryPack::~CSceneryPack()
{	for (U_INT k = 0; k < apod.size(); k++) delete apod[k];
	apod.clear();
}
//------------------------------------------------------------------
//	Mount all pods not already mounted
//------------------------------------------------------------------
void CSceneryPack::MountPODs(CSceneryDBM *dbm)
{	for (U_INT k=0; k < apod.size(); k++) apod[k]->Mount();
	return;
}

//------------------------------------------------------------------
//	remove all pods found
//------------------------------------------------------------------
void CSceneryPack::RemovePODs(CSceneryDBM *dbm)
{	std::vector<CSceneryPOD*>::iterator ip;
  for (ip=apod.begin(); ip!=apod.end(); ip++)
	{	CSceneryPOD *pod = (*ip);
		pod->Remove();
	}
	return;
}
//===================END OF FILE ===============================================================