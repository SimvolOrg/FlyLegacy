// WriteFileBase.h: interface for the CWriteFileBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WRITEFILEBASE_H__824F947A_EDA2_48D0_9757_490E209C1A4C__INCLUDED_)
#define AFX_WRITEFILEBASE_H__824F947A_EDA2_48D0_9757_490E209C1A4C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable:4786)

#include <vector>

#include <string>

namespace my_file {

/////////////////////////////////////////
//
//
/////////////////////////////////////////
struct SUserPos
{
  double lat;
  double lon;
  double alt;
  double ga;
  int    head;
  double spd;
};

class CDataString
{
  std::vector <std::string> data_string_;
  std::vector <std::string>::iterator it_;

public:
  CDataString (void) {data_string_.clear ();}
  virtual ~CDataString (void) {};
  CDataString (const CDataString&);
  CDataString& operator =(const CDataString&);

  void TraverseDataAndPrint (FILE *);
  void AddString (std::string);
};

/////////////////////////////////////////
//
//
/////////////////////////////////////////

class CWriteFileBase  
{
public:
  explicit CWriteFileBase (void) : data_ (NULL), filename_ (""), fp_ (NULL), file_flag_ (false) {};
  explicit CWriteFileBase (std::string);
  virtual ~CWriteFileBase (void);
  CWriteFileBase (const CWriteFileBase&);
  CWriteFileBase& operator =(const CWriteFileBase&);

  void     Write          (void) const;
  void     AddCoordinates (const SUserPos &pos) const;

protected:
  CDataString *data_;
  std::string filename_;
  FILE *fp_;
  bool file_flag_;

  virtual void WriteFile (void) const;
  void OpenFile (void);
  void CloseFile (void) const; // just a declaration
};

class CWritePosition: public CWriteFileBase  
{
public:
  explicit CWritePosition (std::string s, const SUserPos &pos)
    : CWriteFileBase (s), pos_ (pos) {};

protected:
  const SUserPos pos_;
  virtual void WriteFile (void) const;
};

class CWritePath: public CWriteFileBase  
{
public:
  explicit CWritePath (std::string s);

protected:
  virtual void WriteFile (void) const;
  void Initialise (void) const;
};

class CWriteTrackPath: public CWriteFileBase  
{
public:
  explicit CWriteTrackPath (std::string s, const SUserPos &pos)
    : CWriteFileBase (s), pos_ (pos) {};

protected:
  const SUserPos pos_;
  virtual void WriteFile (void) const;
};

/////////////////////////////////////////
//
//
/////////////////////////////////////////

//class CWriteCyclicTrackPath
//{
//public:
//  explicit CWriteCyclicTrackPath (std::string s, const SUserPos &pos) 
//    : pos_ (pos) {};
//  virtual ~CWriteCyclicTrackPath (void) {};
//  CWriteCyclicTrackPath (const CWriteCyclicTrackPath&);
//  CWriteCyclicTrackPath& operator =(const CWriteCyclicTrackPath&);
//
//  void Write (void) {};
//
//protected:
//  const SUserPos pos_;
//};

/////////////////////////////////////////
//
//
/////////////////////////////////////////

}

#endif // !defined(AFX_WRITEFILEBASE_H__824F947A_EDA2_48D0_9757_490E209C1A4C__INCLUDED_)
