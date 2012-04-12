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
//=====================================================================
//	Generic class simple Queue
//=====================================================================
template <class T> class qHDR {
protected:
	//--- ATTRIBUTES ------------------------------------------
	pthread_mutex_t		mux;					  // Mutex for lock
	U_SHORT			NbOb;									// Item number
	T	 *First;					              // First object in queue
	T	 *Last;													// Last  object in queue
	T  *Prev;
	//--- METHODS ---------------------------------------------
public:
  qHDR();
 ~qHDR();
 //----------------------------------------------------------
 void		 Clear();
 T			*Pop();
 //----------------------------------------------------------
 void    PutEnd (T *obj);
 void		 PutHead(T *obj);
 void		 Append (qHDR &q);
 U_INT   ReplaceWith(qHDR *q);
 T      *Detach(T *obj);
 void    Raz();
 //----------------------------------------------------------
 inline  void  Lock()  {pthread_mutex_lock   (&mux); }
 inline  void  Unlock(){pthread_mutex_unlock (&mux); }
 inline  T*  GetFirst()         {Prev  = 0; return First;}
 inline  T*  GetLast()          {return Last; }
 inline  T  *GetNext(T *obj)    {Prev = obj; return obj->Next();}
 //-------------Not empty -----------------------------------------
 bool    NotEmpty()            {return (First != 0);}
 bool    IsEmpty()             {return (First == 0);}

};
//==========================================================================
//  GENERIC SIMPLE QUEUE MANAGEMENT
//==========================================================================
template <class T> qHDR<T>::qHDR()
{ pthread_mutex_init (&mux,0);
	NbOb    = 0;
  First   = 0;
  Last    = 0;
  Prev    = 0;
}
//----------------------------------------------------------
//	Destory all elements
//----------------------------------------------------------
template <class T> qHDR<T>::~qHDR()
{	Clear();	}
//-----------------------------------------------------------------------
//	Delete all objects in the queue
//-------------------------------------------------------------------------
template <class T> void qHDR<T>::Clear()
{	T *obj		= 0;
	for (obj	= Pop(); obj != 0; obj = Pop()) 	if (obj) delete obj;
	return;
}
//---------------------------------------------------------------------
//	Pop the first object from Queue
//---------------------------------------------------------------------
template <class T> T*	qHDR<T>::Pop()	
{	T	*obj	  = First;															// Pop the first
	if (obj)	{First  = obj->Next(); NbOb--; }		  // Update header
	if (Last == obj)	  Last = 0;					          // Queue is now empty
	if (obj)	obj->Next(0);
	return obj;	}
//-------------------------------------------------------------------------
//  Insert Item at end
//-------------------------------------------------------------------------
template <class T> void qHDR<T>::PutEnd(T *obj)
{ if (0 == obj)  return;
  NbOb++;																			// Increment count
	obj->Next(0);																// No next
	T    *lo = Last;														// Get the last object
	Last	   = obj;															// This is the last
	if (lo == 0)	  First = obj;		            // Queue was empty
  else		lo->Next(obj);		                  // Link previous to new item  
  return;
}
//-----------------------------------------------------------------
//  Set new item at first position
//-----------------------------------------------------------------
template <class T> void qHDR<T>::PutHead(T *obj)
{ NbOb++;								                      // Increment count
  obj->Next(First);                           // Link to next
  First       = obj;                          // New First
  if (0 == Last)  Last = obj;                 // Was empty
  return;
}
//-------------------------------------------------------------------------
//  Append Queue 2
//-------------------------------------------------------------------------
template <class T> void qHDR<T>::Append(qHDR<T> &q2)
{ T *end = Last;
  T *db2 = q2.First;
  T *ed2 = q2.Last;
  if (end)  end->Next(db2);
  if (ed2)  Last        = ed2;
  if (0 == First) First = db2;
  NbOb += q2.NbOb;
  //--------Clear Q2 -----------------------
  q2.First = 0;
  q2.Last  = 0;
  q2.NbOb  = 0;
  return;
}
//-------------------------------------------------------------------------
//  Set a copy of the queue
//-------------------------------------------------------------------------
template <class T> U_INT  qHDR<T>::ReplaceWith(qHDR<T> *q2)
{ int old = NbOb;
  NbOb    = q2->NbOb ;
  First   = q2->First;
  Last    = q2->Last;
  Prev    = 0;
  return old;
}
//------------------------------------------------------------------
//  Detach the current item (The NExt is still the next)
//------------------------------------------------------------------
template <class T> T *qHDR<T>::Detach(T *obj)
{ T *nex = obj->Next();
  if (Prev) Prev->Next(nex);
  else      First       = nex;
  if (obj == Last) Last = Prev;
  obj->Next(0);
  NbOb--;								                      // decrement count
  return Prev;
}
//-------------------------------------------------------------------------
//  Clear Queue
//-------------------------------------------------------------------------
template <class T> void  qHDR<T>::Raz()
{ NbOb  = 0;
  First = 0;
  Last  = 0;
  Prev  = 0;
  return;
}

//============================END OF FILE =================================================
#endif // QUEUES_H
