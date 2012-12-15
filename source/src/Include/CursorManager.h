/*
 * CursorManager.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
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

/*! \file CursorManager.h
 *  \brief Defines CCursorManager for application cursor management
 *
 *  This header provides definitions for classes related to management
 *    of the application cursor.  The application disables the OpenGL
 *    cursor and uses its own set of bitmapped cursors.
 *
 */

#ifndef CURSORMANAGER_H_
#define CURSORMANAGER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlyLegacy.h"
#include <map>
//===========================================================================
//  Generic cursor for PBM-ACT defined cursor
//===========================================================================
class CCursor : public CStreamObject {
protected:
  //--------Attributes -----------------------------------
  char          curs[64];         // Cursor name
  Tag           csid;             ///< Unique cursor ID
  char          imag[64];         ///< Image .PBM filename
  int           hots_x, hots_y;   ///< Hotspot x, y
  //--------Texture quad ------------------------------------
  TEXT_DEFN     txd;              // Texture definition
  TC_VTAB       qad[4];           // cursor quad
  //---------Methods ----------------------------------------
public:
   CCursor();
  ~CCursor();
  //--------CStream  virtual method for CFlyCursor (empty)-------------------
  virtual int   Read (SStream *stream, Tag tag) {return 0;}
  virtual void  ReadFinished (void){}
  //---------Cursor methods ---------------------------------
  void  SetQuad(int wd);
  void  LoadCursor(char* name);
  void  CreateTexture (void);
  void  FreeTexture (void);
	void	HotSpot();
  void  Draw();
  //---------------------------------------------------------
  inline  int   GetHotx()       {return hots_x;}
  inline  int   GetHoty()       {return (hots_y - txd.ht);}
  inline  bool  IsThisOne(const char *fn) {return (strcmp(fn,curs) == 0);}
  inline  Tag   GetKey()        {return csid;}
  inline GLuint GetTextId()     {return txd.xo;}
  //---------------------------------------------------------
  inline  void  SetName(char *nm) {strncpy(curs,nm,64);}
  inline  void  SetKey(Tag key)   {csid = key;}
};
//===========================================================================
// CCursor class represents a bitmapped cursor specified in a CSR file
//
//============================================================================ 
class CFlyCursor : public CCursor {
public:
   CFlyCursor (const char* csrfilename);
  ~CFlyCursor (void);

  // CStreamObject methods
  int   Read (SStream *stream, Tag tag);
  void  ReadFinished (void);

public:
  char          csr[64];          ///< Cursor .CSR filename
};
//================================================================================
class CCursorManager {
public:
  CCursorManager ();
 ~CCursorManager();
  void   Cleanup (void);

  //---- CCursorManager methods ---------------------------------------------
  Tag     CrsrArrow();
  Tag     BindCursor (const char* csr);
  Tag     BindFuiCursor(char *name,Tag key);
	void		HideAll();
  void    MouseMotion (int mouse_x, int mouse_y);

  CCursor *FindCursor (const char *csrfilename);
  CCursor *FindCursor (Tag tag);
  //--------------------------------------------------------------------------
  ECursorResult SetCursor (Tag tag);
  Tag           GetCursor (void);
  void					Draw (float dT);
  //---------------------------------------------------------------------------
  inline void NoCursor()      {cCur = 0;}
  inline void SetArrow()      {SetCursor(crsrArrow);}
  //---------------------------------------------------------------------------
protected:
	char			state;										// Cursor state
  Tag       crsrArrow;								// 
  std::map<Tag,CCursor*>    cache;    // Cursor cache, indexed by unique ID tag
  float      Time;                    // Timer for cursor live
  CCursor   *cCur;                    // Pointer to current cursor
  int       x, y;                     // Current mouse position in screen coordinates
};
//==========================END OF FILE ===========================================
#endif // CURSORMANAGER_H
