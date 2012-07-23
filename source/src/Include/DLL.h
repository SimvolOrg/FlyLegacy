//===============================================================================================
// DLL.h
//
// Part of Fly! Legacy project
//
// Copyright 2003 Chris Wallace
//
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
//===============================================================================================

#ifndef DLL_H
#define DLL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//====================================================================================
#include "../Include/WorldObjects.h"
//===========================================================================================
//  Class CDLLWindow to display a window from a DLL plugin
//  NOTE: This class is instanciated in global situation, but the Draw method is called
//  in the context of CFuiManager as it is visible in every view
//===========================================================================================

class CDLLWindow {
  //---------------Internal state ------------------------------------------------
  //--------------Attribute ------------------------------------------------------
  long signature;
  SDLLObject *obj;
  SSurface   *surf;                                           ///< Surface
  short       wd;                                             ///< wide
  short       ht;                                             ///< Height
  U_INT       back;                                           ///< Back color
  U_INT       black;                                          ///< Black color
  //---------For time Management -------------------------------------------------
  //---------Editied fields ------------------------------------------------------
  //---------For editing  --------------------------------------------------------
  //-------------Method ----------------------------------------------------------
public:
  bool enabled;
  void *dll;
  CDLLWindow  ();
 ~CDLLWindow ();

  void  SetObject (SDLLObject *object);
  void  SetSignature (const long &sig);
  const SDLLObject* Get_Object (void) {return obj;}
  const long& GetSignature (void)     {return signature;}
  //-----------------------Edit --------------------------------------------------
  //------------------------Size parameters --------------------------------------
  void  Resize    (void); 
  //------------------------Drawing method ---------------------------------------
  void  Draw      (void);
  //-----------------------Time slice --------------------------------------------
  void  TimeSlice (float dT);
};

//====================================================================================
// The CDLLSimulatedObject object is a specialization of CSimulatedObject for DLL
//   This could be a dynamic object managed by dll.
//
// Stream file declaration:
//    <wobj>
//    sobj
//    <bgno>
//    <endo>
//=====================================================================================
class CDLLSimulatedObject : public CSimulatedObject {
    
public:
  // Constructors / destructor
  CDLLSimulatedObject                  (void);
  virtual ~CDLLSimulatedObject         (void);

  // CStreamObject methods
//  virtual int   Read                (SStream *stream, Tag tag);
//  virtual void  ReadFinished        (void);

  virtual void  SetOrientation      (SVector orientation);

  // Simulation
  virtual int  Timeslice           (float dT, U_INT FrNo);       ///< Real-time timeslice processing
  /// Called from Timeslice() to simulate the things that happen in the timeslice
  virtual void  Simulate            (float dT, U_INT FrNo);				
  // Drawing 
  virtual void  DrawExternal();
  // Methods

protected:
  bool draw_flag;

public:
  bool                  sim_objects_active;
  CVector               sobj_offset;
};

//==========================END OF FILE ===========================================================
#endif // DLL_H
