//=========================================================================================
// Queues.h
//
// Part of Fly! Legacy project
//
// Copyright 2003-2005 Chris Wallace
// copyright 2007-2008 Jean Sabatier
// Fly! Legacy is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
// Fly! Legacy is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
//   along with Fly! Legacy; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//=========================================================================================

#ifndef QUEUES_H
#define QUEUES_H
//=========================================================================================
#include "../Include/FlyLegacy.h"
//=====================================================================
//  Generic queued item : Base class for all queued items
//=====================================================================
class CqItem {
  friend class CQueue;
  //----------Attributes ---------------------------------------
  CqItem *Next;                            // Next Queued item
  //------------------------------------------------------------
public:
  CqItem() {Next = 0;}
  //------------------------------------------------------------
  inline void         SetNext(CqItem *n)  {Next = n;}
  inline CqItem      *GetNext()           {return Next;}
};
//=====================================================================
//  Generic Queue
//=====================================================================
class CQueue {
protected:
  //-----------Attributes ---------------------------------------
  pthread_mutex_t		mux;					          // Mutex for lock
	U_INT				  NbObj;					            // Item number
	CqItem		   *First;					            // First object in queue
	CqItem		   *Last;					              // Last  object in queue
  CqItem       *Prev;                       // Previous in Queue
  //---------------------------------------------------------------
public:
  CQueue();                                 // Constructor
 ~CQueue();
  //----------------------------------------------------------------
  inline   int NbObjects() {return NbObj;}
  //-------------Queue Management ----------------------------------
  U_INT   Copy(CQueue *q);
  void    Append (CQueue &q);
  void    PutEnd (CqItem *item);
  void    PutHead(CqItem *item);
  CqItem *Detach(CqItem *itm);
  void    Raz();
  void    Clear();
  CqItem *Pop();
  CqItem *GetFirst();
  CqItem *GetLast()             {return Last;}
  CqItem *GetNext(CqItem *itm);
  //-------------Not empty -----------------------------------------
  bool    NotEmpty()            {return (First != 0);}
  bool    IsEmpty()             {return (First == 0);}
};
//=====================================================================
//	Queue Header. Used for queued objects
//=====================================================================
class ClQueue {
protected:
	//--------------Attributes ------------------------------------
	pthread_mutex_t		mux;					          // Mutex for lock
	U_SHORT			NbrOb;					              // Item number
	CmHead		 *First;					              // First object in queue
	CmHead		 *Last;					                // Last  object in queue
public:
  ClQueue();
 ~ClQueue();
  //---------inline ---------------------------------------------
  inline  void        Lock()  {pthread_mutex_lock   (&mux); }
  inline  void        Unlock(){pthread_mutex_unlock (&mux); }
  inline  CmHead*  GetFirst()                {return First;}
  inline  CmHead*  GetLast()                 {return Last; }
  inline  CmHead*  NextInQ1(CmHead *ob)   {return (ob)?(ob->Cnext):(0);}
  inline  CmHead*  PrevInQ1(CmHead *ob)   {return (ob)?(ob->Cprev):(0);}
  inline  U_SHORT     GetNbrObj()               {return NbrOb;}
  inline  bool        NotEmpty()  {return (0 != NbrOb);}
	//------------------------------------------------------------
  void        Clear();
  void        LastInQ1(CmHead *obj);
  void        LastInQ2(CmHead *obj);
  void        HeadInQ1(CmHead *obj);
  void        InsertInQ1(CmHead *nob,CmHead *obj);
  CmHead  *DetachFromQ1(CmHead *obj);
  CmHead  *PopFromQ1();
  CmHead  *PopFromQ2();
};

//============================END OF FILE =================================================
#endif // QUEUES_H
