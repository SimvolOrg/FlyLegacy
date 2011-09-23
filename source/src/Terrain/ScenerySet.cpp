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

using namespace std;
//==========================================================================
//	Scenery set to held parameters
//==========================================================================
CScenerySet::CScenerySet (const char* scfFolder,
                          const char* scfFilename)
{
  char fullname[PATH_MAX];
  strcpy (fullname, scfFolder);
  strcat (fullname, scfFilename);

  // Attempt to open scenery file stream
  SStream s;
  if (OpenRStream (fullname,s)) {
    ReadFrom (this, &s);
    CloseStream (&s);
  }
	//-------------------------------------------------------------------
	//	We assign the scenery to a QGT:  
	//	The QGT is defined by the center point described by the area
	//	covered.
	//-------------------------------------------------------------------
  // JS: When the scenary is accross the 0 meridian, the East Longitude
  //  should be corrected for the scenary to load.
  if (ldur.lon < ldll.lon) ldur.lon += TC_FULL_WRD_ARCS;
  if (caur.lon < call.lon) caur.lon += TC_FULL_WRD_ARCS;
	//--- Compute the QGT key where is the scenery center ----------------
	SPosition ctr;
	U_INT			xk;
	U_INT			zk;
	ctr.lon	= (ldll.lon + ldur.lon) * 0.5;
	ctr.lat = (ldll.lat + ldur.lat) * 0.5;
  IndicesInQGT (ctr, xk, zk);
  Key     = QGTKEY(xk, zk);
	//-------------------------------------------------------------------
  // Save base pathname, SCF pathname and SCF filename
  strcpy (this->scfFolder, scfFolder);
  strupper((char*)scfFolder);
  strcpy (this->scfFilename, scfFilename);
  strupper((char*)scfFilename);
  // Default state is unloaded
  refCount = 0;
  loaded = false;
}
//---------------------------------------------------------------------
//	Close scenery set
//---------------------------------------------------------------------
CScenerySet::~CScenerySet (void)
{
  // Ensure scenery set is unloaded
  if (loaded) Unload ();
}
//---------------------------------------------------------------------
//	Read parameters
//---------------------------------------------------------------------
int CScenerySet::Read (SStream *stream, Tag tag)
{
  int rc = TAG_IGNORED;

  char file[PATH_MAX];

  switch (tag) {
  case 'name':
    ReadString (name, 80, stream);
    rc = TAG_READ;
    break;
  case 'call':
    ReadLatLon (&call, stream);
    rc = TAG_READ;
    break;
  case 'caur':
    ReadLatLon (&caur, stream);
    rc = TAG_READ;
    break;
  case 'ldll':
    ReadLatLon (&ldll, stream);
    rc = TAG_READ;
    break;
  case 'ldur':
    ReadLatLon (&ldur, stream);
    rc = TAG_READ;
    break;
  case 'file':
    {
      ReadString (file, PATH_MAX, stream);
      podList.push_back (file);
    }
    rc = TAG_READ;
    break;
  }

  return rc;
}


bool CScenerySet::IsLoaded (void)
{
  return loaded;
}

int  CScenerySet::GetRefCount (void)
{
  return refCount;
}

void CScenerySet::IncRefCount (void)
{
  refCount++;
}

void CScenerySet::DecRefCount (void)
{
  --refCount;
}
//-----------------------------------------------------------------
//	The scenery pod is added to the pod file system
//	So it will be detected by the 3D manager
//-----------------------------------------------------------------
void CScenerySet::Load (char tr)
{
  IncRefCount();

  // Check if set is already loaded
  if (!IsLoaded()) {
    // Add individual pods to the filesystem
    std::vector<string>::iterator i;
    for (i=podList.begin(); i!=podList.end(); i++) {
      char podName[1024];
      strcpy (podName, scfFolder);
      strcat (podName, i->c_str());
      /// @TODO It would be preferable to maintain a separate pod filesystem for
      ///       scenery files, but this raises complications when instantiating
      ///       models in .SXX files.
      paddpod (&globals->pfs, podName);
    }

    // Set loaded flag
    loaded = true;
  }

  if (tr) TRACE("Load scenery set %s, ref count = %d", scfFilename, refCount);
}
//-----------------------------------------------------------------
//	The scenery pod is removed from the pod file system
//	So it will be detected by the 3D manager
//-----------------------------------------------------------------
void CScenerySet::Unload (void)
{
  if (IsLoaded()) {
    // Decrement reference count and unload if necessary
    DecRefCount();
    if (GetRefCount() <= 0) {
      // Remove individual pods from the scenery filesystem
      std::vector<string>::iterator i;
      for (i=podList.begin(); i!=podList.end(); i++) {
              char podName[1024];
              strcpy (podName, scfFolder);
              strcat (podName, i->c_str());

        premovepod (&globals->pfs, podName);
      }
  
      // Clear loaded flag
      loaded = false;

      DEBUGLOG ("Unload scenery set %s, ref count = %d", scfFilename, refCount);
    }
  }
}

//=================================================================================
// CScenerySetDatabase
//
//=================================================================================
// Define the singleton instance of the scenery set database
CScenerySetDatabase CScenerySetDatabase::instance;

//
// "Constructor" for scenery set database singleton.  Search for any scenery files
//   that describe sliced scenery areas.  Scenery files are stream files
//   with the .SCF extension that reside in any sub-directory from the
//   /Scenery folder.
//
void CScenerySetDatabase::Init (void)
{
  // Buffer for the current search path
  char searchPath[PATH_MAX];

  // Load SCF files from Fly! II /Scenery folder
  char base[PATH_MAX];
  strcpy (base, "");
  GetIniString ("UI", "flyRootFolder", base, PATH_MAX);
  if (strlen (base) == 0) {
    // Could not get Fly! II root folder
    WARNINGLOG ("Cannot get Fly! II root folder name");
  } else {
    // Fly! II root folder was found
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
    WARNINGLOG ("GetLogicalDriveStrings : Error %d", GetLastError ());
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
        WARNINGLOG ("GetLogicalDriveStrings : Error %d", GetLastError());
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
#endif // _WIN32
}
//--------------------------------------------------------------------------------
//	Add a set to the pack
//--------------------------------------------------------------------------------
void	CScenerySetDatabase::AddSet(CScenerySet *scn)
{	U_INT key = scn->GetKey();
	std::map<U_INT,CSceneryPack*>::iterator ip = scene.find(key);
	if (ip != scene.end())	{(*ip).second->AddSet(scn); return;}
	//---- create a new pack ----------------------------------
	CSceneryPack *pak = new CSceneryPack(key);
	pak->AddSet(scn);
	scene[key] = pak;
	return;
}
//--------------------------------------------------------------------------------
// "Destructor" for scenery set database singleton.  This method should be called
//   when the database is no longer needed to avoid an apparent memory leak.
//--------------------------------------------------------------------------------
void CScenerySetDatabase::Cleanup (void)
{	//--------------------------------------------------------
	std::map<U_INT,CSceneryPack*>::iterator ip;
	for (ip=scene.begin(); ip!=scene.end(); ip++) delete (*ip).second;
}

//--------------------------------------------------------------
// Search a directory for any scenery control files
//--------------------------------------------------------------
void CScenerySetDatabase::LoadInFolder (const char *path)
{
  DEBUGLOG ("CScenerySetDatabase::LoadFolder : %s", path);

  // Iterate over all files in this folder.  Sub-folders are ignored
  ulDir* dirp = ulOpenDir (path);
  if (dirp != NULL) {
    ulDirEnt* dp;
    while ((dp = ulReadDir(dirp)) != NULL ) {
      if (dp->d_isdir) {
        // This is a sub-folder, ignore it
        continue;
      } else {
        // Check for file extension .SCF
        char *c = strrchr (dp->d_name, '.');
        if (stricmp (c, ".SCF") == 0) {
          DEBUGLOG ("CScenerySetDatabase : Loading %s", dp->d_name);
          CScenerySet *set = new CScenerySet (path, dp->d_name);
          //---Register scenery ----------------------------------
						 AddSet(set);												
        }
      }
    }
    ulCloseDir(dirp);
  }
}
//-----------------------------------------------------------------
//	Load the scenery from path
//-----------------------------------------------------------------
void CScenerySetDatabase::LoadInFolderTree (const char *path)
{
  // Load SCF files from this folder
  LoadInFolder (path);

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

void CScenerySetDatabase::Register (U_INT key,char t)
{	tr	= t;
	std::map<U_INT,CSceneryPack*>::iterator pk = scene.find(key);
	if (pk == scene.end())	return;
	//--- Activate all sceneries in QGT ---------------
 	(*pk).second->Load(t);
	return;
}
//------------------------------------------------------------------------------
//	Deregister by Key
//------------------------------------------------------------------------------
void CScenerySetDatabase::Deregister (U_INT key)
{	std::map<U_INT,CSceneryPack*>::iterator pk = scene.find(key);
	if (pk == scene.end())	return;
	//--- Activate all sceneries in QGT ---------------
	(*pk).second->Unload();
	return;
}

//==============================================================================
//	Scenery pack:  held all sceneries for a given QGT
//==============================================================================
CSceneryPack::CSceneryPack(U_INT k)
{	key	= k;
}
//------------------------------------------------------------------
//	Destroy everything
//------------------------------------------------------------------
CSceneryPack::~CSceneryPack()
{	std::vector<CScenerySet*>::iterator ip;
	for (ip=pack.begin(); ip!=pack.end(); ip++)	delete (*ip);
	pack.clear();
}
//------------------------------------------------------------------
//	Load all sceneries in this pack
//------------------------------------------------------------------
void CSceneryPack::Load(char t)
{	std::vector<CScenerySet*>::iterator ip;
  for (ip=pack.begin(); ip!=pack.end(); ip++) (*ip)->Load(t);
	return;
}
//------------------------------------------------------------------
//	Unload all sceneries
//------------------------------------------------------------------
void CSceneryPack::Unload()
{	std::vector<CScenerySet*>::iterator ip;
  for (ip=pack.begin(); ip!=pack.end(); ip++) (*ip)->Unload();
	return;
}
//===================END OF FILE ===============================================================