// FileElementaryOp.h: interface for the CFileElementaryOp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEElementaryOP_H__09E933FF_7E12_4CC5_8C29_D576A2FD7AB7__INCLUDED_)
#define AFX_FILEElementaryOP_H__09E933FF_7E12_4CC5_8C29_D576A2FD7AB7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <assert.h>
#include <typeinfo>

/*
    char buff10[64] = {'0'};
    char buff3 [64] = {'0'};
    strncpy (buff10, "----------", 10);
    strncpy (buff3, "+++", 3);

  feop::CFileOp *pf = new feop::CFileOp ("_test_track_path.txt");
  if (pf->IsSafeOp ()) {
    pf->WriteBufFromStart (buff10, 387L);
    pf->WriteBufFromPos   (buff10,   1L);
    pf->WriteBufFromPos   (buff10,  76L);
    pf->WriteBufFromPos   (buff10,  27L);
    pf->WriteBufFromPos   (buff3 ,  25L);
    pf->StopOp ();
  }
  if (pf) delete (pf);
*/

namespace feop {

//////////////////////////////
//
// NVI class model with Clone
// needs :
//      <assert.h>
//      <typeinfo>
//
//////////////////////////////

class CFileElementaryOp  
{
public:
  explicit CFileElementaryOp (void) {;} // standard constructor
  explicit CFileElementaryOp (const char *filename);
  virtual ~CFileElementaryOp (void); // standard destructor

  CFileElementaryOp& operator =(const CFileElementaryOp&); // non implemented

  // Clone to avoid implicit construct by copy
  CFileElementaryOp* Clone (void) const {
    CFileElementaryOp* p = DoClone ();
    assert (typeid(*p) == typeid(*this) && "DoClone incorrectly overriden");
    return p;
  }

  // Interface
//  void fn_public (void) const {fn_private ();} // NVI method
  bool IsSafeOp          (void) {return status_flag_;}
  bool WriteBufFromStart (const char *buff, const unsigned int &offset);
  bool WriteBufFromPos   (const char *buff, const unsigned int &offset);
  bool StopOp            (void) {return CloseFile ();}
  bool StartOp           (void) {return OpenFile  ();}

protected:
  int fd;
  char filename_[_MAX_FNAME];
  bool status_flag_;

  CFileElementaryOp (const CFileElementaryOp&) {}; // constructor by copy
  virtual CFileElementaryOp*   DoClone    (void) const = 0;
//  virtual void                fn_private (void) {;} const;
  bool OpenFile  (void);
  bool CloseFile (void);
};

//////////////////////////////
//
// 
//
//////////////////////////////

class CFileOp : public CFileElementaryOp
{
public:
  explicit CFileOp (const char *filename) : CFileElementaryOp (filename) {};

//  void fn_public (void) const {fn_private ();} // NVI method

protected:
  virtual CFileElementaryOp*   DoClone    (void) const {return new CFileOp (filename_);}
//  virtual void                fn_private (void) {;} const;

};

}

#endif // !defined(AFX_FILEElementaryOP_H__09E933FF_7E12_4CC5_8C29_D576A2FD7AB7__INCLUDED_)
