/*
 * TerrainCache.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright  2003 Chris Wallace
 *            2008 Jean Sabatier
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
#ifndef FILETHREAD_H
#define FILETHREAD_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Include/SqlMGR.h"
//============================================================================
//  API DECLARATION
//============================================================================
void *FileThread(void *p);
//=============================================================================
//  FORWARD
//=============================================================================
class C_QGT;
//=============================================================================
//  CLASS Queue of Quarter Tile
//=============================================================================
class qgtQ {
  //--- Data Member ----------------------------------
  pthread_mutex_t		mux;					  // Mutex for lock
  C_QGT *First;                     // First item in Q
  C_QGT *Last;                      // Last Item in Q
  //--------------------------------------------------
public:
  qgtQ();
  void     PutLast(C_QGT *tq);
  void     PutHead(C_QGT *tq);
  C_QGT   *Pop();
  //--------------------------------------------------
  inline bool NotEmpty()  {return (First != 0);}
};
//===============END OF THIS FILE =============================================
#endif  // FILETHREAD_H
