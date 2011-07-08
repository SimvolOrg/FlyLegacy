/*
 * PythonManager.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2007 Chris Wallace
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

/*! \file ScriptsPython.h
 *  \brief Defines classes and utilities to support Python embedding and extending
 *
 */



#ifndef SCRIPTSPYTHON_H
#define SCRIPTSPYTHON_H


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include "FlyLegacy.h"

#ifdef HAVE_PYTHON
#include <Python.h>

class CPythonScript {
public:
  CPythonScript (const char* filename);
  ~CPythonScript (void);

  void  Init (void);
  void  StartScenario ();
  void  EndScenario ();
  void  Main (float dT);
  void  BeginFrame ();
  void  Kill ();

protected:
  PyObject* pModule;
  PyObject* init;
  PyObject* startscenario;
  PyObject* endscenario;
  PyObject* main;
  PyObject* beginframe;
  PyObject* kill;

  char      *scriptText;          ///< Text of the script
};

class CPythonManager {
private:
  static CPythonManager instance;

public:
  static  CPythonManager& Instance() { return instance; }
  void    Init (void);
  void    Cleanup (void);

  void    Load (const char* pyFilename);
  void    ScriptStartScenario ();
  void    ScriptEndScenario ();
  void    ScriptMain (float dT);
  void    ScriptBeginFrame ();

protected:
  std::map<std::string,CPythonScript*>   script;   ///< Currently loaded scripts
};

#endif // HAVE_PYTHON

#endif // SCRIPTSPYTHON_H

