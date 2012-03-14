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
#include "../Include/FlyLegacy.h"
#include "../Include/Queues.h"
//==========================================================================
//	Queue header
//==========================================================================
//  Constructor by default
//--------------------------------------------------------------------------
ClQueue::ClQueue()
{ pthread_mutex_init (&mux,  NULL);
  NbrOb	= 0;
	First	= 0;
	Last	= 0;
}
//-------------------------------------------------------------------------
//  Destructor
//-------------------------------------------------------------------------
ClQueue::~ClQueue()
{ Clear();
}
//-----------------------------------------------------------------------
//	Delete all objects in a queue
//-------------------------------------------------------------------------
void ClQueue::Clear()
{	CmHead *obj = PopFromQ1();
	while (obj)
	{	if (obj->IsShared())  obj->DecUser();
    else  delete obj;
		obj	= PopFromQ1();
	}
	return;
}
//-------------------------------------------------------------------------
//	Put a an object at end of Q1 (Clink used)
//-------------------------------------------------------------------------
void ClQueue::LastInQ1(CmHead *obj)
{	//--- Lock the queue -------------------------
  pthread_mutex_lock (&mux);
	NbrOb++;								                      // Increment count
	obj->Cnext	  = 0;							              // No next
	CmHead *lo = Last;					                // Get the last object
	obj->Cprev	  = lo;							              // Link to previous
	Last	        = obj;							            // This is the last
	if (lo == NULL)	  First     = obj;		        // Queue was empty
	if (lo != NULL)		lo->Cnext = obj;		        // Link previous to new
	//--- Unlock the queue ------------------------
  pthread_mutex_unlock (&mux);
	return ;	}
//---------------------------------------------------------------------
//	Insert a new Object in front of Queue
//---------------------------------------------------------------------
void ClQueue::HeadInQ1(CmHead *obj)
{ pthread_mutex_lock (&mux);
  NbrOb++;
  CmHead *nx    = First;
  obj->Cnext    = nx;
  obj->Cprev    = 0;
  if (nx) nx->Cprev = obj;
  First         = obj;
  if (0 == Last)  Last = obj;
  pthread_mutex_unlock (&mux);
	return ;	}
//-------------------------------------------------------------------------
//	Put a an object at end of Q2 (Tlink used)
//-------------------------------------------------------------------------
void ClQueue::LastInQ2(CmHead *obj)
{	//--- Lock the queue -------------------------
  pthread_mutex_lock (&mux);
	NbrOb++;								                      // Increment count
	obj->Tnext	  = 0;							              // No next
	CmHead *lo = Last;					                // Get the last object
	obj->Tprev	  = lo;							              // Link to previous
	Last	        = obj;							            // This is the last
	if (lo == NULL)	  First     = obj;		        // Queue was empty
	if (lo != NULL)		lo->Tnext = obj;		        // Link previous to new
	//--- Unlock the queue ------------------------
  pthread_mutex_unlock (&mux);
	return ;	}
//-------------------------------------------------------------------------
//	Detach an object from  queue 1
//  Return the previous object
//-------------------------------------------------------------------------
CmHead *ClQueue::DetachFromQ1(CmHead *obj)
{	//--- Lock the queue -and object --------------------
  pthread_mutex_lock (&mux);
	CmHead	*nx = obj->Cnext;					      // next object
	CmHead	*pv = obj->Cprev;					      // Previous
	NbrOb--;									                // Decrease count
	if (nx)		nx->Cprev = pv;						      // Relink next
	if (pv)		pv->Cnext = nx;						      // Relink previous
	if (First == obj)	    First = nx;			    // New first
	if (Last  == obj)	    Last  = pv;			    // New last
	obj->Cnext	= 0;								          // Clear detached next
	obj->Cprev  = 0;								          // Clear detached previous
	//--- Unlock the queue --and object ------------------
  pthread_mutex_unlock (&mux);
	return pv;	}
//---------------------------------------------------------------------
//	Pop the first object from Queue 1
//---------------------------------------------------------------------
CmHead*	ClQueue::PopFromQ1()	
{	//--- Lock the queue -------------------------------
  pthread_mutex_lock (&mux);
	CmHead	*obj	  = First;						            // Pop the first
	if (obj)	{First  = obj->Cnext; NbrOb--; }		    // Update header
	if (Last == obj)	  Last = 0;					            // Queue is now empty
	//---Unlock the queue here --------------------------
  pthread_mutex_unlock (&mux);
	if (0 == obj)	return obj;				
	obj->Cnext	= 0;
	obj->Cprev	= 0;
	return obj;	}
//---------------------------------------------------------------------
//	Pop the first object from Queue 2
//---------------------------------------------------------------------
CmHead*	ClQueue::PopFromQ2()	
{	//--- Lock the queue -------------------------------
  pthread_mutex_lock (&mux);
	CmHead	*obj	  = First;						            // Pop the first
	if (obj)	{First  = obj->Tnext; NbrOb--; }		    // Update header
	if (Last == obj)	  Last = 0;					            // Queue is now empty
	//---Unlock the queue here --------------------------
  pthread_mutex_unlock (&mux);
	if (0 == obj)	return obj;				
	obj->Tnext	= 0;
	obj->Tprev	= 0;
	return obj;	}
//---------------------------------------------------------------------
//	Insert a new Object in front of obj
//---------------------------------------------------------------------
void ClQueue::InsertInQ1(CmHead *nob,CmHead *obj)
{ //--- Lock the queue -------------------------------
  pthread_mutex_lock (&mux);
  CmHead *prv = (obj)?(obj->Cprev):(Last);
  if (prv)  prv->Cnext = nob;
  nob->Cnext  = obj;
  if (obj)  obj->Cprev = nob;
  nob->Cprev  = prv;
  if (0 ==     obj)   Last  = nob;
  if (First == obj)   First = nob;
  NbrOb++;
  //---Unlock the queue here --------------------------
  pthread_mutex_unlock (&mux);
  return;  
}
//==========================================================================
//  GENERIC QUEUE MANAGEMENT
//==========================================================================
CQueue::CQueue()
{ pthread_mutex_init (&mux,  NULL);
  NbObj   = 0;
  First   = 0;
  Last    = 0;
  Prev    = 0;
}
//-------------------------------------------------------------------------
//  Destroy: Free all items in Queue
//-------------------------------------------------------------------------
CQueue::~CQueue()
{ CqItem *itm = Pop();
  while (itm) {delete itm; itm = Pop();}
}
//-------------------------------------------------------------------------
// Flush: Free all items in queue
//-------------------------------------------------------------------------
void CQueue::Clear()
{ CqItem *itm = Pop();
  while (itm) {delete itm; itm = Pop();}
  return;
}
//-------------------------------------------------------------------------
//  Set a copy of the queue
//-------------------------------------------------------------------------
U_INT  CQueue::Copy(CQueue *q2)
{ int old = NbObj;
  NbObj   = q2->NbObj;
  First   = q2->First;
  Last    = q2->Last;
  Prev    = 0;
  return old;
}
//-------------------------------------------------------------------------
//  Append Queue 2
//-------------------------------------------------------------------------
void CQueue::Append(CQueue &q2)
{ CqItem *end = Last;
  CqItem *db2 = q2.First;
  CqItem *ed2 = q2.Last;
  if (end)  end->Next = db2;
  if (ed2)  Last      = ed2;
  if (0 == First) First = db2;
  NbObj += q2.NbObj;
  //--------Clear Q2 -----------------------
  q2.First = 0;
  q2.Last  = 0;
  q2.NbObj = 0;
  return;
}
//-------------------------------------------------------------------------
//  Clear Queue
//-------------------------------------------------------------------------
void  CQueue::Raz()
{ NbObj = 0;
  First = 0;
  Last  = 0;
  Prev  = 0;
  return;
}
//-------------------------------------------------------------------------
//  Insert Item at end
//-------------------------------------------------------------------------
void  CQueue::PutEnd(CqItem *item)
{ if (0 == item)  return;
  NbObj++;								                      // Increment count
	item->Next	  = 0;							              // No next
	CqItem    *lo = Last;					                // Get the last object
	Last	        = item;							            // This is the last
	if (lo == NULL)	  First = item;		            // Queue was empty
  else		lo->Next = item;		                  // Link previous to new item  
  return;
}
//-----------------------------------------------------------------
//  Set new item at first position
//-----------------------------------------------------------------
void CQueue::PutHead(CqItem *item)
{ NbObj++;								                      // Increment count
  item->Next  = First;                          // Link to next
  First       = item;                           // New First
  if (0 == Last)  Last = item;                  // Was empty
  return;
}
//-----------------------------------------------------------------
//  Remove and return The First Item
//-----------------------------------------------------------------
CqItem *CQueue::Pop()
{ CqItem *deb = First;
  if (deb)    First = deb->Next;
  if (deb)    deb->Next = 0;
  if (First == 0) Last  = 0;
  if (deb)    NbObj--;								          // decrement count
  return deb;
}
//------------------------------------------------------------------
//  Detach the current item (The NExt is still the next)
//------------------------------------------------------------------
CqItem *CQueue::Detach(CqItem *itm)
{ CqItem *nex = itm->Next;
  if (Prev) Prev->Next  = nex;
  else      First       = nex;
  if (itm == Last) Last = Prev;
  itm->Next  = 0;
  NbObj--;								                      // decrement count
  return Prev;
}
//------------------------------------------------------------------
//  Return the next item
//------------------------------------------------------------------
CqItem *CQueue::GetNext(CqItem *itm)
{ CqItem *nex = itm->Next;
  Prev  = itm;
  return nex;
}
//------------------------------------------------------------------
//  Return the First item
//------------------------------------------------------------------
CqItem *CQueue::GetFirst()
{ Prev  = 0;
  return First;
}
 

//============================END OF FILE =================================================



