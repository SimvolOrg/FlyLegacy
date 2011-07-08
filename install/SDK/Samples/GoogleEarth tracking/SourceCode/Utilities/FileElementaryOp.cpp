// FileElementarOp.cpp: implementation of the CFileElementarOp class.
//
//////////////////////////////////////////////////////////////////////

#include "FileElementaryOp.h"

#ifdef _DEBUG
#include <stdio.h>  // FILE op
#endif

using namespace feop;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileElementaryOp::CFileElementaryOp(const char *filename)
: fd (-1)
{
  strncpy (filename_, filename, _MAX_FNAME);
  status_flag_ = false;
  OpenFile ();
}

CFileElementaryOp::~CFileElementaryOp()
{
  if (status_flag_) CloseFile ();
}

bool CFileElementaryOp::OpenFile (void)
{
    if (!status_flag_)
      fd = open (filename_, O_WRONLY);
    #ifdef _DEBUG	
	    FILE *fp_debug;
	    if(!(fp_debug = fopen("__DDEBUG_legacy_google2.txt", "a")) == NULL)
	    {
		    int test = 0;
		    fprintf(fp_debug, "CFileElementaryOp::OpenFile : %d\n", fd);
		    fclose(fp_debug); 
	    }
    #endif
    if (fd != -1) {
      status_flag_ = true;
      return true;
    } else {
      status_flag_ = false;
      return false;
    }
}

bool CFileElementaryOp::CloseFile (void)
{
  if (status_flag_) {
    int val = close (fd);
    #ifdef _DEBUG	
	    FILE *fp_debug;
	    if(!(fp_debug = fopen("__DDEBUG_legacy_google2.txt", "a")) == NULL)
	    {
		    int test = 0;
		    fprintf(fp_debug, "CFileElementaryOp::CloseFile : %d\n", val);
		    fclose(fp_debug); 
	    }
    #endif
    if (val != -1) {
      fd = -1;
      status_flag_ = false;
      return true;
    } else {
      status_flag_ = true;
      return false;
    }
  }
  return false;
}

bool CFileElementaryOp::WriteBufFromStart (const char *buff, const unsigned int &offset)
{
  if (status_flag_) {
    lseek (fd, offset, 0 /*SEEK_SET*/);
    if (write (fd, buff, strlen (buff)) != -1)
      return true;
    else
      return false;
  } else 
      return false;
}

bool CFileElementaryOp::WriteBufFromPos (const char *buff, const unsigned int &offset)
{
  if (status_flag_) {
    lseek (fd, offset, 1 /*SEEK_CUR*/);
    if (write (fd, buff, strlen (buff)) != -1)
      return true;
    else
      return false;
  } else 
      return false;
}