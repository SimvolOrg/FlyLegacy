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
#include "../Include/ScenerySet.h"
#include "../Include/Fui.h"
#include "../Include/Pod.h"
using namespace std;
struct SQL_DB;
//==================================================================================
//   Global function to process a file from a POD 
//==================================================================================
int ScnpodCB(PFS *pfs, PFSPODFILE *p)
{	int stop = globals->scn->CheckForScenery(p);
	return stop;
}
//==================================================================================
//	Global function to add an OSM database
//==================================================================================
int OsmBaseCB(char *fn,void *upm)
{	CSceneryDBM *dbm = (CSceneryDBM *)upm;
  //TRACE("FP add:%s",fn);
	return dbm->AddOSMbase(fn);
}
//==================================================================================
//	Global function to add a QGT for OSM base
//==================================================================================
void osmQGTkey(U_INT key, void *obj)
{	CSceneryDBM *dbm = (CSceneryDBM *)obj;
	dbm->AddGQTforOSM(key);
	return;
}
//==========================================================================
//	Scenery POD to define a POD for scenery
//	NOTE: The key is a QGT(X-Z) key
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
// CSceneryDBM  Scenery Data  base Manager
//	Manage all scenery Packs
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
{ sqm	= globals->sqm;
	pfs = &globals->pfs;
	//--- Check for export ---------------------------
	imp	= globals->import;
	//--- Compression option ---------------------------
	cmp = 1;												// By default
	GetIniVar("Performances","UseDTXdatabase",&cmp);
	cmp &= (!imp);									// Not importing
	globals->comp	= cmp;
  //--- Load SCF files from FlyLegacy /Scenery folder
  LoadInFolderTree ("SCENERY/",SCN_OPT_SELECT);
	//--- Locate OSM folder ----------------------------
	fname[0] = 0;
	GetIniString("SQL","OSMDB",fname,FNAM_MAX);
	if (0 == *fname)	strcpy(fname,"./SQL");
	_snprintf(path,FNAM_MAX,"%s/*.db",fname);
	//--- Add OSM databases ----------------------------
	ApplyToFiles(path,OsmBaseCB,this);
	SCENE("========END INITIAL LOAD==================");
	return;
}
//--------------------------------------------------------------------------------
//	Get a Scenery Pack or create it
//--------------------------------------------------------------------------------
CSceneryPack *CSceneryDBM::GetQGTPack(U_INT key)
{	std::map<U_INT,CSceneryPack*>::iterator rp = m3dPAK.find(key);
	if (rp != m3dPAK.end()) return (*rp).second;
	//--- Create a new pack ----------------------------------
	CSceneryPack *pak = new CSceneryPack(key);
	m3dPAK[key] = pak;
	return pak;
}
//--------------------------------------------------------------------------------
//	Add a pod to the QGT pack
//--------------------------------------------------------------------------------
void	CSceneryDBM::AddPodToQGT(CSceneryPOD *pod)
{	U_INT key = pod->GetKey();
	CSceneryPack *pak = GetQGTPack(key);
	pak->AddPod(pod);
	return;
}
//--------------------------------------------------------------------------------
//	Check if already in scenery pack
//--------------------------------------------------------------------------------
bool CSceneryDBM::AllreadyIN(U_INT key,char *pn)
{	std::map<U_INT,CSceneryPack*>::iterator ip = s2dPAK.find(key);
	if (ip == s2dPAK.end())	return false;
	//--- Check for name --------------------
	CSceneryPack *pak = (*ip).second;
	return pak->AllreadyIN(pn);
}
//--------------------------------------------------------------------------------
//	Add a pod to the GBT pack
//--------------------------------------------------------------------------------
void	CSceneryDBM::AddPodToGBT(CSceneryPOD *pod)
{	U_INT key = pod->GetKey();
	std::map<U_INT,CSceneryPack*>::iterator ip = s2dPAK.find(key);
	if (ip != s2dPAK.end())	{(*ip).second->AddPod(pod); return;}
	//---- create a new pack ----------------------------------
	CSceneryPack *pak = new CSceneryPack(key);
	pak->AddPod(pod);
	s2dPAK[key] = pak;
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
	for (ip=m3dPAK.begin(); ip!=m3dPAK.end(); ip++) delete (*ip).second;
	m3dPAK.clear();
	//----Clean GBT list -------------------------
	std::map<U_INT,CSceneryPack*>::iterator ig;
	for (ig=s2dPAK.begin(); ig!=s2dPAK.end(); ig++) delete (*ig).second;
	s2dPAK.clear();
}
//--------------------------------------------------------------
// Process POD according to option
//--------------------------------------------------------------
void CSceneryDBM::ProcessThePOD(char *pn, char *fn, char opt)
{	if (opt == SCN_OPT_SELECT)			SelectPOD(pn,fn);
	if (opt == SCN_OPT_MOUNT)
	{	char pname[MAX_PATH];
		_snprintf(pname,MAX_PATH,"%s%s",pn,fn);
		paddpod(pfs,pname);
		SCENE(".. mount POD %s",pname);
	}
	return;
}
//--------------------------------------------------------------
// Search for POD in this directory
//	Locate each POD and process it 
//--------------------------------------------------------------
void CSceneryDBM::LookForPOD(char *path,char opt)
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
				if (sh)					MountSharedPod(path,fn);
				else						ProcessThePOD(path,fn,opt);
			}
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
//	When POD file exists in database OBJ, it is not mounted
//	Otherwise, we look into each file in the POD
//		-File that have 2D data (DATA/.../xxx.Snn)	are mounted
//			as a 2D scenery pack
//		-Other files are mounted as a 3D object scenery pack
//--------------------------------------------------------------
void CSceneryDBM::SelectPOD(char *path,char *fn)
{	char *scn = "SCENERY/";
  int   lgr = strlen(scn);
	int   lim = FNAM_MAX;
	char name[PATH_MAX];
	//--- mount pod if exporting -------------
	_snprintf(name,lim,"%s%s",path,fn);
	if  (imp)	 return SendPOD(name); 
	//--- First search pod in databases ------
	char *pn  = strstr(path,scn) + lgr;
	_snprintf(name,lim,"%s%s",pn,fn);
	int   in  = CheckDatabase(fn);
	if (in)		return Warn01(name);
	//--- Search the QGT ---------------------
  _snprintf(name,lim,"%s%s",path,fn);
	char *dbp = strstr(name,scn) + lgr;
	strncpy(fname,dbp,lim);
	fname[lim] = 0;
	//--- Look into each files ---------------
	//if (strncmp(fn,"Hyere",5)==0)
	//int a = 0;
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
	//--- For debugg only ----------------------------------
	//if (strncmp(fn,"DATA/D170158",12) == 0)
	//int a = 0;
	//---- There is a DATA indication ----------------------
	char *dot = strchr(fn1,'.');
	if (0 == dot)										return SceneryForGBT(p,gx,gz);
	//--- Check scenery file S00 S01 S10 S11 ---------------
	nf			= sscanf(dot,".S%1u%1u",&sx,&sz);
	if (nf != 2)										return SceneryForGBT(p,gx,gz);
	//--- This is a scenery file XXX.s00 to .S11 -----------
	gx = (gx << 1) + sx;						// QGT X index
	gz = (gz << 1) + sz;						// QGT Z index
	U_INT	key = QGTKEY(gx,gz);			// QGT key
	//--- New file for QGT -----------------
	fn	= p->name;
	CSceneryPOD *pod = new CSceneryPOD(key,p->pod->name);
	AddPodToQGT(pod);
	SCENE("      ***** ADD QGT(%03d-%03d) SCENERY %s",gx,gz,fn);
	return 0;
}
//--------------------------------------------------------------
//	We check the database (if any) for this pod file
//	0 => Not in database
//--------------------------------------------------------------
int CSceneryDBM::CheckDatabase(char *pn)
{	//--- Search in scenery database ------------------
	if (0 == globals->objDB)			return 0;
	bool in = sqm->SearchPODinOBJ(pn);
	return (in != 0);
}
//-----------------------------------------------------------------
//	Associate the pod to the QGT
//	NOTE:  The key is a global tile Key
//-----------------------------------------------------------------
int CSceneryDBM::SceneryForGBT(PFSPODFILE *p,int gx,int gz)
{	U_INT key		= QGTKEY((gx << 1),(gz << 1));
	char *pname = p->pod->name;
	if (AllreadyIN(key,pname))	return 0;
	CSceneryPOD *pod	= new CSceneryPOD(key,pname);
	AddPodToGBT(pod);
	SCENE("      ***** ADD GBT(%03d-%03d) SCENERY %s",gx,gz,p->pod->name);
	return 0;
}
//-----------------------------------------------------------------
//	Load the scenery from path
//	Search all pods file located in scenery directory and all
//			subdirectories.
//-----------------------------------------------------------------
void CSceneryDBM::LoadInFolderTree (char *path, char opt)
{
  // Load POD files from this folder
	LookForPOD((char*)path,opt);
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
        LoadInFolderTree (newPath,opt);
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
//------------------------------------------------------------------------------
//	Mount the given key  except OSM databases
//------------------------------------------------------------------------------
void CSceneryDBM::MountByKey(U_INT qx,U_INT qz)
{	char	reg = 1;
  U_INT	key	= QGTKEY(qx,qz);
	std::map<U_INT,CSceneryPack*>::iterator p1 = m3dPAK.find(key);
	if (p1 != m3dPAK.end())
	{//--- Activate all sceneries in QGT --------------------
		if (reg)		{SCENE("QGT (%03d-%03d) REGISTER",qx,qz); reg = 0;}
		(*p1).second->MountPODs(this);
	}
	//--- Look for global tile scenery ----------------------
	U_INT gbk = key & 0xFFFEFFFE;
	std::map<U_INT,CSceneryPack*>::iterator p2 = s2dPAK.find(gbk);
	if (p2 != s2dPAK.end())
	{	CSceneryPack *pak = (*p2).second;
		SCENE("GBT (%03d-%03d) REGISTER",qx,qz);
		pak->MountPODs(this);
	}
	return;
}
//------------------------------------------------------------------------------
//	Register by QGT key
//	Check if  the QGT has compressed textures
//	We check whether the  SW corner detail tile is in the texture DTX database
//------------------------------------------------------------------------------
int CSceneryDBM::Register (C_QGT *qgt)
{	U_INT	key	= qgt->FullKey();
	U_INT qx	= (key >> 16);
	U_INT qz	= (key & 0xFFFF);
	//--- Mount all pod by Key ----------------------------
	MountByKey(qx,qz);
	//--- Load OSM databases ------------------------------
	if (globals->noOSM == 0)	LoadBasesOSM(qgt);
	//--- Check for compressed texture --------------------
	if (0 == cmp)							return 0;
	//--- Create area name form QGT indices ---------------
	char pn[MAX_PATH];				// Pathname
	char nm[64];							// Database name
	globals->sqm->CreateCompressedDBname(pn,nm,qx,qz);
	strcat(pn,"/");
	strncat(pn,nm,MAX_PATH);
	//--- Use SQL manager to remember databases -----------
	//SQL_DB *db = 	globals->sql->OpenSQLbase(pn,0,nm);
	SQL_DB *db = 	sqm->OpenSQLbase(pn,0,nm);
	qgt->StoreCompressedTexDB(db);
	//char *idb=(db)?(db->dbn):("None");
	//TRACE("**** Register QGT(%03d-%03d) Name=%s",qx,qz,idb);								
	return (db)?(1):(0);
}
//------------------------------------------------------------------------------
//	Mount all sceneries
//------------------------------------------------------------------------------
void CSceneryDBM::MountAll()
{	SCENE("=======> MOUNT ALL POD FILES =============");
	LoadInFolderTree ("SCENERY/",SCN_OPT_MOUNT);
	SCENE("=======> ALL POD FILES MOUNTED=============");

	return;
}
//------------------------------------------------------------------------------
//	Deregister by Key
//------------------------------------------------------------------------------
void CSceneryDBM::Deregister (U_INT key)
{	U_INT gx = (key >> 16);
	U_INT gz = (key & 0xFFFF);
	int  reg = 1;
	std::map<U_INT,CSceneryPack*>::iterator p1 = m3dPAK.find(key);
	if (p1 != m3dPAK.end())	
	{	//--- remove all sceneries in QGT ---------------
		if (reg)	{	SCENE("QGT (%03d-%03d) UNREGISTER",gx,gz); reg = 0; }
		(*p1).second->RemovePODs(this);
	}
	//--- Look for global tile scenery ----------------------
	U_INT gbk = key & 0xFFFEFFFE;
	std::map<U_INT,CSceneryPack*>::iterator p2 = s2dPAK.find(gbk);
	if (p2 == s2dPAK.end())	return;
	if (reg)	SCENE("GBT (%03d-%03d) UNREGISTER",gx,gz); 
	(*p2).second->RemovePODs(this);
	//--- Remove compressed texture database ----------------
	return;
}
//=================================================================================
// CSceneryDBM
//	Special mounting for shared scenery
//	Check if pod file already in Database OBJ. 
//	POD already in data base are not mounted
//=================================================================================
void CSceneryDBM::MountSharedPod(char * path,char *fn)
{	char *scn = "SCENERY/";
  int   lgr = strlen(scn);
	int   lim = FNAM_MAX;
	char  name[PATH_MAX];
	//--- First search pod in databases ------
	char *pn  = strstr(path,scn) + lgr;
	_snprintf(name,lim,"%s%s",pn,fn);
	int   in  = CheckDatabase(name);
	if (in)	return Warn01(fn);
	//--- Mount the pod file -----------------
	_snprintf(name,lim,"%s%s",path,fn);
	U_INT tk = paddpod (&globals->pfs, name);
	SCENE("   Ticket(%06d) Mount %s",tk,name);
	return;
}
//=================================================================================
// CSceneryDBM
//	Add OSM database
//
//=================================================================================
int CSceneryDBM::AddOSMbase(char *fn)
{ char *slh = strrchr(path,'/');
	if (0 == slh)					return 1;
	*slh = 0;
	_snprintf(fname,FNAM_MAX,"%s/%s",path,fn);
	*slh = '/';
	cdb = sqm->OpenSQLbase(fname,0,"OSM");
	if (0 == cdb)					return 1;
	if (0 == cdb->use)		return 1;
	//--- Get covered QGTs -----------------------
	sqm->GetQGTlistOSM(*cdb,osmQGTkey,this);
	sqm->CloseSQLbase(cdb);
	cdb	= 0;
	SCENE("Add OSM database %s",fname);
	return 1;
}
//---------------------------------------------------------------------------------
//	Add a scenery pack for >OSM QGT
//---------------------------------------------------------------------------------
void CSceneryDBM::AddGQTforOSM(U_INT key)
{ CSceneryPack *pak = GetQGTPack(key);
	pak->AddOSM(cdb->path);
	return;
}
//=================================================================================
// CSceneryDBM
//	Register for QGT:  Open  first database that reference this QGT
//
//=================================================================================
//------------------------------------------------------------------
//	Return a new database
//	Assign a number for this dabase
//------------------------------------------------------------------
SQL_DB *CSceneryDBM::GetOSMbase(C_QGT *qgt, int nb)
{	std::map<U_INT,CSceneryPack*>::iterator rp = m3dPAK.find(qgt->FullKey());
	if (rp == m3dPAK.end())				return 0;
	//--- OPEN the database -------------------------------
	const char *fn = (*rp).second->GetOSMname(nb);
	if (0 == fn)								return 0;
	SQL_DB *db = sqm->OpenSQLbase((char*)fn,0,"OSM");
	if (0 == db)								return 0;
	TRACE("Mounting database %s",fn);
	return db;
}
//------------------------------------------------------------------
//	Open databases for this qgt.  Called by QGT at OSM step
//------------------------------------------------------------------
void CSceneryDBM::LoadBasesOSM(C_QGT *qgt)
{ bool go = true;
	int  nb = 0;
	while (go)
	{	SQL_DB *db = GetOSMbase(qgt,nb++);
		if (db)		qgt->AddOsmBase(db);
		else			break;
	}
	return;
}
//------------------------------------------------------------------
//	Unload OSM base from a QGT
//------------------------------------------------------------------
void CSceneryDBM::FreeBasesOSM(SQL_DB *db)
{	globals->sqm->CloseSQLbase(db);
	return;
}
//------------------------------------------------------------------
//	Add a request to load OSM objects
//------------------------------------------------------------------
void	CSceneryDBM::AddOSMrequest(OSM_DBREQ *req)
{	reqQ.Lock();
	reqQ.PutEnd(req);
	reqQ.Unlock();
	return;
}
//------------------------------------------------------------------
//	Pop a request for file Thread
//------------------------------------------------------------------
OSM_DBREQ *CSceneryDBM::PopOSMrequest()
{	reqQ.Lock();
	OSM_DBREQ *R = reqQ.Pop();
	reqQ.Unlock();
	return R;
}
//------------------------------------------------------------------
//	Flush OSM request
//------------------------------------------------------------------
void CSceneryDBM::FlushOSM()
{	OSM_DBREQ *rq = 0;	
	while (rq = PopOSMrequest())	delete rq;
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
	aosm.clear();
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
bool CSceneryPack::AllreadyIN(char *pn)
{	std::vector<CSceneryPOD*>::iterator ip;
  for (ip=apod.begin(); ip!=apod.end(); ip++)
	{	CSceneryPOD *pod = (*ip);
		char *nm = pod->GetName();
		if (strcmp(pn,nm) == 0)	return true;
	}
	return false;
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
//------------------------------------------------------------------
//	OSM interface: Return database name
//------------------------------------------------------------------
const char *CSceneryPack::GetOSMname(U_INT k)
{	if (k >= aosm.size())		return 0;
	return aosm[k].c_str();
}
//==============================================================================
//	Globe area
//==============================================================================
//	Constructor 
//------------------------------------------------------------------------------
GlobeArea::GlobeArea(char *n,U_INT x, U_INT z, U_INT w, U_INT h)
{	name	= n;
	gx		= x;
	gz		= z;
	wd		= w;
	ht		= h;
}
//------------------------------------------------------------------------------
//	Check if QGT inside area
//------------------------------------------------------------------------------

//===================END OF FILE ===============================================================