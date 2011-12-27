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
{	int stop = CSceneryDBM::Instance().CheckForScenery(p);
	delete p;
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
{	users    = 1;
	U_INT No = paddpod (&globals->pfs, fName);
	SCENE("   (%06d) Mount %s",No,fName);
	return;
}
//---------------------------------------------------------------
//	Remove the POD file
//	Decrement user count and unmout when no more users
//---------------------------------------------------------------
void CSceneryPOD::Remove()
{	U_INT No = premovepod (&globals->pfs, fName);
	SCENE("   (%06d) Unmount %s",No,fName);
	return;
}
//---------------------------------------------------------------
//	Remove one user and return count
//---------------------------------------------------------------
int CSceneryPOD::MoreUsers()
{	users--;
	return users;
}
//=================================================================================
// CSceneryDBM
//
//=================================================================================
// Define the singleton instance of the scenery set database
CSceneryDBM CSceneryDBM::instance;

//
// "Constructor" for scenery set database singleton.  Search for any scenery files
//   that describe sliced scenery areas.  Scenery files are stream files
//   with the .SCF extension that reside in any sub-directory from the
//   /Scenery folder.
//----------------------------------------------------------------------------------
void CSceneryDBM::Init (void)
{
  // Buffer for the current search path
  char searchPath[PATH_MAX];
	pfs = &globals->pfs;
  // Load SCF files from Fly! II /Scenery folder
  char base[PATH_MAX];
  strcpy (base, "");
  GetIniString ("UI", "flyRootFolder", base, PATH_MAX);
  if (strlen (base) == 0) {
    // Could not get Fly! II root folder
    SCENE ("Cannot get Fly! II root folder name");
  } else 
	{ // Fly! II root folder was found
    strcpy (searchPath, base);
    strcat (searchPath, "/Scenery/");
    LoadInFolderTree (searchPath);
  }

  // Load SCF files from FlyLegacy /Scenery folder
  strcpy (searchPath, "./Scenery/");
  LoadInFolderTree (searchPath);

#ifdef _WIN32
  // On Windows systems, also search the /Scenery folders on the top-level mount points
  // for fixed disks (removable and network drives may be optionally added at a later
  // date).

  // Enumerate volumes
  DWORD bufferSize = 1024;
  char* driveNames = new char[bufferSize];
  DWORD driveNameSize = GetLogicalDriveStrings (bufferSize, driveNames);
  if (driveNameSize == 0) {
    // Error reading drive names
    SCENE ("GetLogicalDriveStrings : Error %d", GetLastError ());
  } else {
    // Check to see if drive names exceeded buffer size
    if (driveNameSize > bufferSize) {
      // Reallocate buffer and try again
      delete[] driveNames;
      bufferSize = driveNameSize + 1;
      driveNames = new char[bufferSize];
      driveNameSize = GetLogicalDriveStrings (bufferSize, driveNames);
      if (driveNameSize == 0) {
        // Failed for some other reason
        SCENE ("GetLogicalDriveStrings : Error %d", GetLastError());
      }
    }

    // Iterate over logical drive names, loading SCF files from any top-level
    //   /Scenery folders
    unsigned int offset = 0;
    while (offset < driveNameSize) {
      // Get this drive name and its type
      char* nextDriveName = driveNames + offset;
      UINT type = GetDriveType (nextDriveName);

      // Load SCF files from top-level /Scenery folder on fixed drives only
      if (type == DRIVE_FIXED) {
        strcpy (searchPath, nextDriveName);
        strcat (searchPath, "Scenery/");
        LoadInFolderTree (searchPath);
      }

      // Advance to the next drive name
      offset += strlen(nextDriveName) + 1;
    }
  }
  delete[] driveNames;
	SCENE("========END INITIAL LOAD==================");
#endif // _WIN32
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
// "Destructor" for scenery set database singleton.  This method should be called
//   when the database is no longer needed to avoid an apparent memory leak.
//--------------------------------------------------------------------------------
void CSceneryDBM::Cleanup (void)
{	//----Clean QGT list -------------------------
	std::map<U_INT,CSceneryPack*>::iterator ip;
	for (ip=sqgt.begin(); ip!=sqgt.end(); ip++) delete (*ip).second;
	//----Clean GBT list -------------------------
	std::map<U_INT,CSceneryPack*>::iterator ig;
	for (ig=sgbt.begin(); ig!=sgbt.end(); ip++) delete (*ip).second;
}

//--------------------------------------------------------------
// Search for POD in this directory 
//--------------------------------------------------------------
void CSceneryDBM::LookForPOD(char *path)
{	
	SCENE("SEARCH POD in: %s", path);
  // Iterate over all files in this folder.  Sub-folders are ignored
  ulDir*  dirp = ulOpenDir (path);
	ulDirEnt* dp = 0;
  if (dirp) {
    
    while ((dp = ulReadDir(dirp)) != NULL )
		{ if (dp->d_isdir)						continue;
			//--- Bypass shared scenery (already mounted) ---------
			if (GetSceneryType(path))		continue;
			//--- a file is found ---------------------------------
      // Check for file extension .POD
      char *ext = strrchr (dp->d_name, '.');
			if (0 == ext)								continue;
      if ((_stricmp (ext, ".EPD") == 0) || (_stricmp(ext, ".POD") == 0))
			{	ProcessPOD(path,dp->d_name);				}
    }
  }
    ulCloseDir(dirp);
		return;
}
//--------------------------------------------------------------
// Check for shared scenery
//--------------------------------------------------------------
int	CSceneryDBM::GetSceneryType(char *path)
{	char *sdr = "/SCENERY/SHARED";
	int   lgr = strlen(sdr);
	_strupr(path);
	char *deb = path + 3;				// Bypass drive letter
	deb = strchr(deb,'/');			// Bypass
	//--- Search for SHARED directory ------------------
	while (deb)	
	{	if (strncmp(deb,sdr,lgr)== 0)		return 1;
		deb	= strchr(deb+1,'/');
	}
	return 0;
}
//--------------------------------------------------------------
// Process this POD file 
//--------------------------------------------------------------
void CSceneryDBM::ProcessPOD(char *path,char *fn)
{	char name[PATH_MAX];
  _snprintf(name,511,"%s%s",path,fn);
	scanpod(pfs,name,ScnpodCB);
}
//--------------------------------------------------------------
// Check for file in database 
//--------------------------------------------------------------
bool CSceneryDBM::LookSceneryInSQl(char *fn)
{	if (!globals->objDB)		return false;
	int   nb = globals->sqm->SearchWOBJ(fn);
	if (0 == nb)						return false;
	SCENE("      SCENERY %s already in SQL base",fn);
	return true;
}
//--------------------------------------------------------------
// Check for scenery to process
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
	char *dot = strchr(fn1,'.');
	if (0 == dot)		return SceneryForGBT(p,gx,gz);
	nf			= sscanf(dot,".S%1u%1u",&sx,&sz);
	if (nf != 2)		return SceneryForGBT(p,gx,gz);
	//--- Compute QGT key ----------------------------------
	gx = (gx << 1) + sx;						// QGT X index
	gz = (gz << 1) + sz;						// QGT Z index
	U_INT	key = QGTKEY(gx,gz);			// QGT key
	//--- Look in database for loaded file -----------------
	fn	= p->name;
	if (LookSceneryInSQl(fn1))		return 0;
	CSceneryPOD *pod = new CSceneryPOD(key,p->pod->name);
	AddPodToQGT(pod);
	SCENE("      ***** ADD QGT(%03d-%03d) SCENERY %s",gx,gz,fn);
	return 1;
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
  // Load SCF files from this folder
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
        strcpy (newPath, path);
        strcat (newPath, dp->d_name);
        strcat (newPath, "/");
        LoadInFolderTree (newPath);
      }
    }
    ulCloseDir(dirp);
  }
}
//------------------------------------------------------------------------------
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
	for (p1 == sqgt.begin(); p1 != sqgt.end(); p1++)
	{	CSceneryPack *pak = (*p1).second;
	  U_INT key = pak->key;
		Register( key);
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
	{	//--- Activate all sceneries in QGT ---------------
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
//------------------------------------------------------------------------------
//	Check if pod can be mounted
//	If already mounted, just increment user count
//------------------------------------------------------------------------------
void	CSceneryDBM::MountPOD(CSceneryPOD *pod)
{	char *name = pod->GetName();
	std::map<std::string,CSceneryPOD *>::iterator it = mount.find(name);
	if (it != mount.end())		return pod->IncUser();
	//--- Mount all files in the pod ---------------
	mount[name] = pod;
	pod->Mount();
	return;
}
//------------------------------------------------------------------------------
//	Remove the POD
//------------------------------------------------------------------------------
void CSceneryDBM::RemovePOD(CSceneryPOD *pod)
{ char *name = pod->GetName();
	std::map<std::string,CSceneryPOD *>::iterator it = mount.find(name);
	if (it == mount.end())			return;
	//--- check for users ---------------------------------
	if (pod->MoreUsers())				return;
	//--- remove from map and unmount all files -----------
	mount.erase(name);
	pod->Remove();
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
{	std::vector<CSceneryPOD*>::iterator ip;
	for (ip=apod.begin(); ip!=apod.end(); ip++)	delete (*ip);
	apod.clear();
}
//------------------------------------------------------------------
//	Mount all pods not already mounted
//------------------------------------------------------------------
void CSceneryPack::MountPODs(CSceneryDBM *dbm)
{	std::vector<CSceneryPOD*>::iterator ip;
  for (ip=apod.begin(); ip!=apod.end(); ip++)
	{	CSceneryPOD *pod = (*ip);
		dbm->MountPOD(pod);
	}
	return;
}

//------------------------------------------------------------------
//	remove all pods found
//------------------------------------------------------------------
void CSceneryPack::RemovePODs(CSceneryDBM *dbm)
{	std::vector<CSceneryPOD*>::iterator ip;
  for (ip=apod.begin(); ip!=apod.end(); ip++)
	{	CSceneryPOD *pod = (*ip);
		dbm->RemovePOD(pod);
	}
	return;
}
//===================END OF FILE ===============================================================