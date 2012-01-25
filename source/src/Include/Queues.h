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
  void    Clear();
  void    Flush();
  CqItem *Pop();
  CqItem *GetFirst();
  CqItem *GetLast()             {return Last;}
  CqItem *GetNext(CqItem *itm);
  //-------------Not empty -----------------------------------------
  bool    NotEmpty()            {return (First != 0);}
  bool    IsEmpty()             {return (First == 0);}
};
//======================================================================================
//  2D point coordinates
//======================================================================================
struct D2_POINT {
	D2_POINT *next;															// Next point
	D2_POINT *prev;															// Previous
	char   idn[4];															// Point ident
	//---------------------------------------------------------
  char   R;																		// Type of point
	char   F;																		// Type of face
	char   rfu1;																// Not used 
	char   rfu2;																// Not used
	//--- Space coordinates ------------------------------------
	double x;																		// X coordinate
  double y;																		// Y coordinate
	double z;																		// Z coordinate
	//--- Texture coordinate -----------------------------------
	double s;																		// along x
	double t;																		// along y
	//--- Local space -------------------------------------------
	double lx;																	// X coordinate
	double ly;																	// Y coordinate
	//-----------------------------------------------------------
	double elg;																	// Edge lenght
	double l2D;																	// 2D lenght
	//---- Methods -----------------------
	D2_POINT::D2_POINT() {x = y = z =0; R= 0;}
	D2_POINT::D2_POINT(double x,double  y)
	 {	R	= F = 0;	
			this->x = x;
			this->y = y;
			z       = 0;
		}
	//-----------------------------------
	D2_POINT::D2_POINT(D2_POINT &p)	{*this = p; elg = 0;}
	//-----------------------------------
	void  Id(char *d)			{strncpy(d,idn,4); d[4] = 0;}
	void  SetID(char *d)	{strncpy(idn,d,4);}
	//-----------------------------------
	void	D2_POINT::Convex()			{R = 0;}
	void	D2_POINT::Reflex()			{R = 1;}
	void  D2_POINT::Type(char n)	{R = n;}
	bool  D2_POINT::IsReflex()	  {return (R == 1);}
	bool  D2_POINT::NotNulEdge()	{return (*idn != 'N');}
	//-----------------------------------
	char  D2_POINT::GetType()		{return *idn;}
	void	D2_POINT::SetEdge(char n)	{*idn = n;}
	//-----------------------------------
	void	D2_POINT::SetFace(char n)	{F	= n;}
	char  D2_POINT::GetFaceType() {return F;}
	//----------------------------------------
	void  D2_POINT::Draw()				{	glVertex3f(x,y,z);}
	//----------------------------------------
	void  D2_POINT::Add(D2_POINT &p, SVector &v)
	{	x = p.x + v.x;
		y = p.y + v.y;
		z = p.z + v.z;
	}
};
//======================================================================================
//  2D triangles
//	Orientation is B->A->C->B
//======================================================================================
struct D2_TRIANGLE {
	char	type;
	D2_POINT *B;				// Previous point
	D2_POINT *A;				// current vertex
	D2_POINT *C;				// Next point
	//----------------------------------------------
	D2_TRIANGLE::D2_TRIANGLE() { type = 0;}
	//---- Allocate D2_POINTS ----------------------
	D2_TRIANGLE::D2_TRIANGLE(char a)
	{	type = 1;
	  B	= new D2_POINT();
		A = new D2_POINT();
		C = new D2_POINT();
	}
	//----- Destruction ----------------------------
	D2_TRIANGLE::~D2_TRIANGLE()
		{	if (type)	{delete B; delete A; delete C;} }
	//----- Methods --------------------
	void	D2_TRIANGLE::Set(D2_POINT *P, D2_POINT *S, D2_POINT *N)
	 {	A = S; 
			B = P;
			C = N;
		}
	//----------------------------------------------
	void  D2_TRIANGLE::Draw()
	{	B->Draw();
		A->Draw();
		C->Draw();
	}
};
//=====================================================================
//	Queue Header. Used for queuing D2_POINT object
//=====================================================================
template <class T> class Queue {
protected:
	//--------------Attributes ------------------------------------
	U_SHORT			NbOb;									// Item number
	T	 *First;					              // First object in queue
	T	 *Last;													// Last  object in queue
	T  *Prev;
public:
  Queue();
 ~Queue();
  //---------inline ---------------------------------------------
  inline  T*  GetFirst()         {return First;}
  inline  T*  GetLast()          {return Last; }
  inline  T*  NextFrom(T *p)     {return (p)?(p->next):(0);}
  inline  T*  PrevFrom(T *p)     {return (p)?(p->prev):(0);}
  inline  U_SHORT     GetNbObj() {return NbOb;}
  inline  bool        NotEmpty()  {return (0 != NbrOb);}
	//--- Circular links -----------------------------------------
	inline  T*  CyNext(T *p)	{T* n = p->next; if (0 == n) n = First; return n;}
	inline  T*  CyPrev(T *p)	{T* n = p->prev; if (0 == n) n = Last;  return n;}
	//------------------------------------------------------------
	inline  void  Update(int k,T *p, T *q) {NbOb += k; if (p == Last) Last = q;}
	//------------------------------------------------------------
  void        Clear();
  void        PutLast (T *obj);
  void        PutBefore(T *ne,T *obj);
  T          *Detach(T *obj);
  T          *Pop();
	void	      Transfer(T **h, T **e);
	T          *Remove(T *obj);

};
//==========================================================================
//  GENERIC QUEUE MANAGEMENT
//==========================================================================
template <class T> Queue<T>::Queue()
{ NbOb    = 0;
  First   = 0;
  Last    = 0;
  Prev    = 0;
}
//----------------------------------------------------------
//	Destory all elements
//----------------------------------------------------------
template <class T> Queue<T>::~Queue()
{	Clear();	}
//----------------------------------------------------------
//	Insert element Obj at end of queue
//----------------------------------------------------------
template <class T> void	Queue<T>::PutLast (T *obj)
{	NbOb++;								                    // Increment count
	obj->next	 = 0;							              // No next
	T *lo = Last;															// Get the last object
	obj->prev	 = lo;							            // Link to previous
	Last	     = obj;							            // This is the last
	if (lo == NULL)	  First    = obj;					// Queue was empty
	if (lo != NULL)		lo->next = obj;					// Link previous to new
	return ;	
}
//----------------------------------------------------------
//	Insert new element ne after object obj
//----------------------------------------------------------
template <class T> void Queue<T>::PutBefore(T *ne,T *obj)
{ T *prv = (obj)?(obj->prev):(Last);
  if (prv)  prv->next = ne;
  ne->next  = obj;
  if (obj)  obj->prev = ne;
  ne->prev  = prv;
  if (0 ==     obj)   Last  = ne;
  if (First == obj)   First = ne;
  NbOb++;
  return;  
}
//----------------------------------------------------------
//	Detach element  obj from Queue return the previous
//----------------------------------------------------------
template <class T> T* Queue<T>::Detach(T *obj)
{	T	*nx = obj->next;					      // next object
	T	*pv = obj->prev;					      // Previous
	NbOb--;														// Decrease count
	if (nx)		nx->prev = pv;					// Relink next
	if (pv)		pv->next = nx;					// Relink previous
	if (First == obj)	    First = nx;	// New first
	if (Last  == obj)	    Last  = pv;	// New last
	obj->next	= 0;								    // Clear detached next
	obj->prev = 0;								    // Clear detached previous
	return pv;	}
//---------------------------------------------------------------
//	Detach element  obj from Queue without destroying the pointer
//---------------------------------------------------------------
template <class T> T* Queue<T>::Remove(T *obj)
{	T	*nx = obj->next;					      // next object
	T	*pv = obj->prev;					      // Previous
	NbOb--;														// Decrease count
	if (nx)		nx->prev = pv;					// Relink next
	if (pv)		pv->next = nx;					// Relink previous
	if (First == obj)	    First = nx;	// New first
	if (Last  == obj)	    Last  = pv;	// New last
	return obj;	}

//---------------------------------------------------------------------
//	Pop the first object from Queue
//---------------------------------------------------------------------
template <class T> T*	Queue<T>::Pop()	
{	T	*obj	  = First;															// Pop the first
	if (obj)	{First  = obj->next; NbOb--; }		    // Update header
	if (Last == obj)	  Last = 0;					          // Queue is now empty
	if (0 == obj)	return obj;				
	obj->next	= 0;
	obj->prev	= 0;
	return obj;	}
//-----------------------------------------------------------------------
//	Delete all objects in a queue
//-------------------------------------------------------------------------
template <class T> void Queue<T>::Clear()
{	T *obj = Pop();
	while (obj)
	{	delete obj;
		obj	= Pop();
	}
	return;
}
//-----------------------------------------------------------------------
//	Transfer Queue
//-------------------------------------------------------------------------
template <class T> void Queue<T>::Transfer(T **h, T **e)
{ *h = First;
	*e = Last;
	First = Last = 0;
	NbOb	= 0;
}
//============================END OF FILE =================================================
#endif // QUEUES_H
