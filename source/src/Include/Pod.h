/*
 * Pod.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
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

/*! \file Pod.h
 *  \brief Header file for Pod filesystem library
 */


#ifndef POD_H
#define POD_H


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <map>
#include <set>
#include <string>
#include "../Include/FlyLegacy.h"
#include "LogFile.h"
//================================================================================================
#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

//
// Pod filesystem performance metrics are only supported on Windows until an
//   alternative to LARGE_INTEGER is implemented.
//
// JS: Already defined in glew.h
#ifdef _WIN32
#define POD_PERFORMANCE_METRICS
//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

//================================================================================================
// Pod format signatures
//static const unsigned int PodSignatureEpd  = 'extd';
static const unsigned int PodSignatureEpd  = 'dtxe';      // JS
static const unsigned int PodSignaturePod2 = 'POD2';
static const unsigned int PodSignaturePod3 = 'POD3';

// Pod formats
typedef enum {
  PodFormatUnknown = 0,
  PodFormatEpd,
  PodFormatPod2,
  PodFormatPod3
} EPodFormat;

//================================================================================================
// EPD/POD macro definitions
//================================================================================================

#define EPD_VOLUME_LENGTH        (0x100)
#define EPD_FILENAME_LENGTH       (0x40)
#define POD_VOLUME_LENGTH         (0x50)
#define POD_AUDIT_USERNAME_LENGTH (0x20)
#define POD_FILENAME_LENGTH      (0x100)
#define POD_AUTHOR_LENGTH         (0x50)
#define POD_COPYRIGHT_LENGTH      (0x50)

//
// Enumerated type describing the source for a file within the pod filesystem.  A file
//   can reside either as a discrete file on disk, or within a POD archive.
//
typedef enum {
  PODFILE_SOURCE_POD,
  PODFILE_SOURCE_DISK
} EContentSource;


//
// The following structure represents summarized information about a single
//   POD archive file within a pod filesystem.  The podList member of the PFS
//   struct is composed a list of these structs.
//
typedef struct {
	unsigned int  users;					// Users count
  char          name[PATH_MAX]; // Full path and filename of the POD
  EPodFormat    format;         // Specific POD format
  unsigned long Directory;     // Directory position
  unsigned int  nEntries;      // Entry number
  unsigned int  nAuditEntries; // Number of entries in audit trail
  unsigned int  checksum;      // Checksum
  unsigned int  revision;      // Revision
  unsigned int  priority;      // Mount priority

  FILE*       file;           // FILE*
  int         refs;           // Number of active references to the POD
} PFSPOD;

//==================================================================================
// The following structure summarizes information about a single file within
//   a POD archive.  Instances of this struct are contained in the fileList
//   member of the PFSPOD struct.  The (relative) filename, the starting offset
//   of the file within the pod, its size in bytes and its mount priority are
//   recorded.
//==================================================================================
struct PFSPODFILE {
	unsigned int	share;												// Sharing usage
  PFSPOD*       pod;                          // Reference to containing POD
  char          name[POD_FILENAME_LENGTH+1];  // Name of file within the POD
  unsigned int  offset;                       // Starting offset within the POD
  unsigned int  size;                         // Size in bytes
  unsigned int  timestamp;                    // Timestamp
  unsigned int  checksum;                     // Checksum
  unsigned int  priority;                     // Mount priority
	//----------------------------------------------------------------------
	PFSPODFILE::PFSPODFILE()	{share = 0; }
};

//===================================================================================
// The following data structure is the top-level representation of a pod filesystem.
//   Applications must first create an instance of this struct and pass it to
//   pinit() for initialization, specifying the policy of whether popen() should
//   look first in PODs or on disk when opening a file, and the root folder for
//   the filesystem.
//	Note:  Mode is used for mouting/unmounting  POD files from main thread and
//				should not be used when working in other thread
//===================================================================================
typedef struct {
	char					mode;											// Shared mode
	unsigned int  ticket;										// Event number
	pthread_mutex_t	mux;										// JS: protection Mutex for multithread
  bool      searchPodFilesFirst;          // Whether to open from PODs or disk first
  CLogFile *log;                          // Optional activity logging
  std::map<std::string,PFSPOD*> podList;  // Map of pods in the filesystem

  // Filesystem files residing in pods
  std::multimap<std::string,PFSPODFILE*>  podFileList;
  
  // Map of files residing on disk; first member is short name within POD filesystem;
  //   second member is full path and filename
  std::map<std::string,std::string>       diskFileList;

  // Master set of filenames in the POD filesystem
	std::map<std::string,PFSPODFILE*>				masterFileList;
  // Members to support pfindfirst() and pfindnext() methods
	std::map<std::string,PFSPODFILE*>::iterator iterFind;
  char                                    patternFind[POD_FILENAME_LENGTH+1];
  
} PFS;

//=====================================================================================
// The following struct is the application's interface to a file within the
//   POD filesystem.  It may represent either a file within a POD or a discrete
//   disk file; the application should not care where the file is stored.  All
//   POD filesystem functions operate using pointers to this struct type.
//=====================================================================================
typedef struct {
  char            fullFilename[PATH_MAX];   ///< Full filename for DISK files
  char            filename[PATH_MAX]; // Relative filename
  EContentSource  source;       // Source for file, either disk or POD
  FILE*           pFile;        // stdio FILE* handle to the file, regardless of whether
                                //   it is a POD or a discrete file.
  PFSPOD*         pPod;         // Only valid if file is within a POD; this linkage to the
                                //   POD struct allows for decrementing the reference count
                                //   and closing the FILE associated with the POD if
                                //   necessary.
  long            offset;       // Starting offset of the file within the FILE*; this will
                                //   be either 0 for disk files, or some byte offset for
                                //   POD files.
  long      size;               // Size in bytes of the content
  long      pos;                // Current read position within the stdio FILE*.  This will
                                //   be initialized to 'offset' when starting to read the
                                //   content, and incremented with each read operation.
  long      back;               // This is the position to come back when requested
  //-----------------------------------------------------------------------------------
public:
} PODFILE;

//================================================================================================
//	Function prototype for file processing
//	User must call ApplyToFile with a pattern, a callback function and a user parameter
//================================================================================================
typedef int (*FileCB)(char *fn,void *upm);
void	ApplyToFiles(char *pat, FileCB cb, void *upm);
//================================================================================================
//	Function prototype for file processing in a POD file
//================================================================================================
typedef int (*PodFileCB)(PFS *pfs, PFSPODFILE *p);
int paddpodfile (PFS *pfs, PFSPODFILE *p);
//=====================================================================================
// Initialize a new pod filesystem.  The application owns the pod filesystem
//   represented by the PFS struct.  It must be statically or dynamically
//   allocated by the application, and a reference passed in to this function
//   before any other operations can be performed.  The root argument specifies
//   the root folder for the filesystem.  This folder will be used as the
//   top-level folder when searching for normal disk files to add to the
//   filesystem; pinit will recursively process all sub-folders and add all
//   normal (non-POD) files into the filesystem.  The root folder is also used
//   by the paddpodfolder() function as the base from which the relative
//   folder is determined.  The searchPodFilesFirst argument determines
//   the behaviour of popen(), in the case where the same filename exists both
//   within a POD and as a normal disk file.  If searchPodFilesFirst is true,
//   then popen() will open the normal disk file; if it is false, then popen()
//   will return the file within the POD.
//====================================================================================
void    pinit (PFS* pPfs, const char *logfile,char *mod = "w");	      // JSDEV* add log file

//
// Shut down and clean up a pod filesystem.
//
void    pshutdown (PFS* pPfs);

//
// Add a new pod to the existing pod filesystem.  The pod filename is relative
//   to the root folder for the filesystem
//
void	scanpod(PFS *pPfs, const char* filename,PodFileCB cb);
unsigned int	GetTicket(PFS *pfs);
unsigned int	paddpod(PFS *p, const char* filename);
unsigned int  premovepod (PFS *p, const char* filename);

//------------------------------------------------------------------------------
// Add a new folder to an existing pod filesystem.  The folder name is relative
//   to the root folder for the filesystem, e.g. if the root folder is
//   "C:\Fly! II" and the folder argument was "Aircraft", then all pods in the
//   "C:\Fly! II\Aircraft" folder would be mounted in the filesystem.
//-----------------------------------------------------------------------------
void paddpodfolder (PFS* pPfs, const char* folder, bool addSubdirs = false,char sh = 0);

//----------------------------------------------------------------------------
// Add a new folder to an existing pod filesystem
//----------------------------------------------------------------------------
void padddiskfolder (PFS *pPfs, const char* root, const char* folder);
//----------------------------------------------------------------------------
//  Add a non pod file to internal file system
//----------------------------------------------------------------------------
void pAddDisk(PFS *pfs, char *key, char *fn);
//----------------------------------------------------------------------------
//  Remove a non POD file from internal file system
//----------------------------------------------------------------------------
void pRemDisk(PFS *pfs,char *key,char *fn);
//----------------------------------------------------------------------------
// This function searches for the specified filename in a POD filesystem.
//   Only the cached directories are searched; no actual POD file access
//   is done.
//----------------------------------------------------------------------------
bool		podexist (PFS *pPfs, const char* filename);
bool    pexists (PFS* pPfs, const char* filename);
bool		LookOnlyInPOD (PFS *pfs, char* filename, char **pn);
char   *GetSceneryPOD(char *pn);
/*!
 * Find the first filename in the POD filesystem matching the pattern string
 *
 * Searches through the POD filesystem for the first filename matching
 *   the pattern.  Wildcards * and ? are permitted, which match strings
 *   of zero or more characters, or exactly one character respectively.
 * \returns Const pointer to filename string of first match found, or
 *            NULL if none found.
 */
char* pfindfirst (PFS* pPfs, char* pattern, char **pod = 0);

/*!
 * Find the next filename in the POD filesystem matching the pattern string
 *
 * This function must be called after pfindfirst() which determines the
 *   search pattern to use.
 * \returns Const pointer to filename of next match found, or NULL if no more.
 */
char* pfindnext (PFS* pPfs, char **pod = 0);

//
// This function mimics the stdio function "fopen" and opens a file for read
//   access only.  The PODFILE* reference returned by this function must be
//   passed to the other pod library routines below.  Even though the files
//   are read-only, they should be closed using pclose() when they are no longer
//   needed.
//
PODFILE*  popen (PFS* pPfs, const char* filename,char *md = 0);

//
// This function mimics the stdio function "fread".
//
size_t    pread (void* buffer, size_t size, size_t count, PODFILE* f);
//
// This function mimics the stdio function "fwrite".
//
size_t    pwrite(void *buf,    size_t size, size_t count, PODFILE *f);
//
// This function mimics the stdio function feof().  It returns true if the
//   pod file has encounted an end-of-file condition, and false otherwise.
//
int     peof (PODFILE* f);

//
// This function mimics the stdio function fgetc() and returns a single character
//   read from the pod file.
//
int     pgetc (PODFILE* f);

//
// This function mimics the stdio function fgets(), reading a line of text
//   from the pod file up until the next newline character.
//
char*   pgets (char* s, int n, PODFILE* f);

//
// This function mimics the stdio function fseek(), repositioning the read
//   pointer for the pod file.
//
int     pseek (PODFILE* f, long offset, int origin);

//
// This function mimics the stdio function ftell().
//
long    ptell (PODFILE* f);

//
// This function mimics the stdio function frewind().
//
void    prewind (PODFILE* f);

//
// This function mimics the stdio function fclose() and should be used to close
//   a pod file when it has been fully processed.
//
void    pclose (PODFILE* f);

//
// This function dumps the contents of a pod filesystem to the specified
//   stdio FILE*
//
void pfsdump (PFS *pfs, FILE *f);

#endif // POD_H

