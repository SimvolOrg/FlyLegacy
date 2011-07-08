/*
 * DLLSubsystems.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
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

#include "../Include/Subsystems.h"
#include "../Include/Globals.h"

using namespace std;

//==============================================================================
//
// CDLLSubsystem
//==============================================================================
CDLLSubsystem::CDLLSubsystem (void)
{  
  #ifdef _DEBUG
    //TRACE ("--------------------------------------------------------");
    //TRACE ("DLL Subsystem ...");
  #endif
  obj = NULL;
  dll = NULL;
  enabled = false;
  signature = 0;
}
//----------------------------------------------------------

CDLLSubsystem::~CDLLSubsystem (void)
{
#ifdef _DEBUG
  //TRACE ("DELETE DLL SUBSYSTEM %p %p", obj, dll);
  //TRACE ("--------------------------------------------------------");
#endif
  // sdk: cleanup objects = DLLDestroyObject // 
  globals->plugins.On_DestroyObject (obj, dll);
}

void CDLLSubsystem::Prepare (void)
{
  // sdk : test whether a dll is present
  if (globals->plugins_num) {
    void *test = globals->plugins.IsDLL (signature);
    if (NULL == test) {
      globals->plugins.On_DeleteAllObjects ();
      globals->plugins.On_KillPlugins ();
      char buf1 [8] = {0};
      TagToString (buf1, signature); 
      TRACE ("test dll '%s' = %d", buf1, test);
      gtfo ("failed to find a DLL for '%s'", buf1);
    }
    else {
    dll = test;
      globals->plugins.On_Instantiate_DLLSystems (signature,0,NULL);
      obj = globals->plugins.GetDLLObject (signature);
    }
  }
}

int CDLLSubsystem::Read (SStream *stream, Tag tag)
{
#ifdef _DEBUG
  //TRACE ("READ DLL SUBSYSTEM %p %p %p", dll, obj, stream);
#endif

  switch (tag) {
  case 'unId':
    ReadTag (&unId, stream);
    TagToString (unId_string, unId);
    signature = unId;
    Prepare (); //
    return TAG_READ;
  }

  int ret = TAG_IGNORED;
  if (globals->plugins_num)
    ret = globals->plugins.On_Read (obj, stream, tag, dll);
  //TRACE ("ret = %d", ret);
  if (ret == TAG_IGNORED) ret = CSubsystem::Read (stream, tag);
  return ret;
}

void CDLLSubsystem::ReadFinished (void)
{
#ifdef _DEBUG
  //TRACE ("CREATE DLL SUBSYSTEM");
#endif  
}

void CDLLSubsystem::SetObject (SDLLObject *object)
{
  obj = object;
}

void CDLLSubsystem::SetSignature (const long &sign)
{
//#ifdef _DEBUG
//  TRACE ("SIGNATURE DLL SUBSYSTEM");
//#endif
  signature = sign;
}

void CDLLSubsystem::TimeSlice (float dT,U_INT FrNo)
{
  //CSubsystem::TimeSlice (dT, FrNo);
  globals->plugins.On_TimeSlice (obj, dT, dll);
}

