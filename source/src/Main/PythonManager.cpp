/*
 * PythonManager.cpp
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
/*
#ifdef HAVE_PYTHON

#include "../Include/FlyLegacy.h"
#include "../Include/Globals.h"
#include "../Include/MagneticModel.h"
#include "../Include/PythonManager.h"
#include "../Include/TimeManager.h"
#include "../Include/Situation.h"
#include <plib/ul.h>

using namespace std;

void CheckPythonError (void)
{
  PyObject* err = PyErr_Occurred();
  if (err != NULL) {
    PyErr_Print();
  }
  PyErr_Clear();
} 
//
// Define SDK extensions.   These functions are bound into the "flysdk" module
//   and are accessible to all Python scripts
//
/*
parsing arguments:
if (!PyArg_ParseTuple(args, "s", &command))
        return NULL;

returning arguments:
return Py_BuildValue("i", sts);

void returns:
Py_INCREF(Py_None);
    return Py_None;
*/
/*
extern "C" {

static PyObject* flysdk_UserPosition (PyObject *self)
{
  // Get position from user vehicle
  SPosition pos = globals->geop;
  return Py_BuildValue("[d:d:d]", pos.lat, pos.lon, pos.alt);
}

static PyObject* flysdk_UserAltitudeAGL (PyObject *self)
{
  // Get AGL altitude from user vehicle
  float agl = globals->sit->GetUserVehicle()->GetUserAGL ();
  return Py_BuildValue("f", agl);
}

static PyObject* flysdk_UserAltitudeMSL (PyObject *self)
{
  // Get MSL altitude directly from user vehicle position
  SPosition pos = globals->geop;
  float msl = (float)pos.alt;
  return Py_BuildValue("f", msl);
}

static PyObject* flysdk_UserTrueHeading (PyObject *self)
{
  // Get true heading from user vehicle orientation
  SVector orient = globals->sit->GetUserVehicle()->GetOrientation ();
  float hdg = RadToDeg(orient.y);
  return Py_BuildValue("f", hdg);
}

static PyObject* flysdk_UserMagneticHeading (PyObject *self)
{
  // Get true heading from user vehicle orientation
  SVector orient = globals->sit->GetUserVehicle()->GetOrientation ();

  // Get magnetic declination at user vehicle position
  SPosition pos = globals->geop;
  float D = 0, H = 0;
  CMagneticModel::Instance().GetElements (pos, D, H);

  // Return magnetic heading as sum of true heading and magnetic declination
  return Py_BuildValue("f", RadToDeg (orient.y) + D);
}

static PyObject* flysdk_UserAltitudeRate (PyObject *self)
{
  // TODO : Implement altitude rate method in CVehicleObject
  float rate = 0;
  return Py_BuildValue("f", rate);
}

static PyObject* flysdk_UserBankAngle(PyObject *self)
{
  // Get bank angle from user vehicle orientation
  SVector orient = globals->sit->GetUserVehicle()->GetOrientation ();
  return Py_BuildValue("f", RadToDeg(orient.z));
}

static PyObject* flysdk_UserPitchAngle(PyObject *self)
{
  // Get pitch angle from user vehicle orientation
  SVector orient = globals->sit->GetUserVehicle()->GetOrientation ();
  return Py_BuildValue("f", RadToDeg(orient.x));
}

static PyObject* flysdk_AltitudeCheckAGL (PyObject *self, PyObject *args)
{
  int rc = 0;
  int target = 0, plus = 0, minus = 0;
  if (PyArg_ParseTuple (args, "i:i:i", &target, &plus, &minus)) {
    float agl = globals->sit->GetUserVehicle()->GetUserAGL ();
    if ((agl >= (float)(target - minus)) && (agl <= (float)(target + plus))) rc = 1;
  }
  return Py_BuildValue("i", rc);
}

static PyObject* flysdk_AltitudeCheckMSL (PyObject *self, PyObject *args)
{
  int rc = 0;
  int target = 0, plus = 0, minus = 0;
  if (PyArg_ParseTuple (args, "i:i:i", &target, &plus, &minus)) {
    SPosition pos = globals->geop;
    float msl = (float)pos.alt;
    if ((msl >= (float)(target - minus)) && (msl <= (float)(target + plus))) rc = 1;
  }
  return Py_BuildValue("i", rc);
}

static PyObject* flysdk_HeadingCheck (PyObject *self, PyObject *args)
{
  int rc = 0;
  int target = 0, plus = 0, minus = 0;
  if (PyArg_ParseTuple (args, "i:i:i", &target, &plus, &minus)) {
    // Get true heading from user vehicle orientation
    SVector orient = globals->sit->GetUserVehicle()->GetOrientation ();

    // Get magnetic declination at user vehicle position
    SPosition pos = globals->geop;
    float D = 0, H = 0;
    CMagneticModel::Instance().GetElements (pos, D, H);

    float hdg = RadToDeg (orient.y) + D;

    if ((hdg >= (float)(target - minus)) && (hdg <= (float)(target + plus))) rc = 1;
  }
  return Py_BuildValue("i", rc);
}

static PyObject* flysdk_AirspeedCheck (PyObject *self, PyObject *args)
{
  // TODO : Implemement IAS query to CVehicleObject
  int rc = 0;
  return Py_BuildValue("i", rc);
}

static PyObject* flysdk_VerticalRateCheck (PyObject *self, PyObject *args)
{
  // TODO : Implemement vertical rate query to CVehicleObject
  int rc = 0;
  return Py_BuildValue("i", rc);
}

static PyObject* flysdk_PlaySound (PyObject *self, PyObject *args)
{
  // TODO : Interface with CAudioManager
  int rc = 0;
  return Py_BuildValue("i", rc);
}

static PyObject* flysdk_PlaySounds (PyObject *self, PyObject *args)
{
  // TODO : Interface with CAudioManager
  int rc = 0;
  return Py_BuildValue("i", rc);
}

static PyObject* flysdk_StopSound (PyObject *self, PyObject *args)
{
  // TODO : Interface with CAudioManager
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_IsSoundValid (PyObject *self, PyObject *args)
{
  // TODO : Interface with CAudioManager
  int rc = 0;
  return Py_BuildValue("i", rc);
}

static PyObject* flysdk_SetSoundVolume (PyObject *self, PyObject *args)
{
  // TODO : Interface with CAudioManager
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_SetSoundPosition (PyObject *self, PyObject *args)
{
  // TODO : Interface with CAudioManager
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_SetSoundDirection (PyObject *self, PyObject *args)
{
  // TODO : Interface with CAudioManager
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_SetSoundVelocity (PyObject *self, PyObject *args)
{
  // TODO : Interface with CAudioManager
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_SetSoundFrequency (PyObject *self, PyObject *args)
{
  // TODO : Interface with CAudioManager

  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_IsSimulating (PyObject *self)
{
  // TODO : Implement IsSimulating query in CSituation?
  int rc = 0;
  return Py_BuildValue("i", rc);
}

static PyObject* flysdk_IsSlewing (PyObject *self)
{
  int rc = globals->slw->IsEnabled() ? 1 : 0;
  return Py_BuildValue("i", rc);
}

static PyObject* flysdk_IsPaused (PyObject *self)
{
  int rc = globals->tim->GetPauseState() ? 1 : 0;
  return Py_BuildValue("i", rc);
}

static PyObject* flysdk_SetSlewing (PyObject *self, PyObject *args)
{
  // Enable or disable slew mode based on argument
  int i = 0;
  if (PyArg_ParseTuple (args, "i", &i)) {
    if (i == 0) globals->slw->Disable();
    else        globals->slw->Enable();
  }

  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_SetPaused (PyObject *self, PyObject *args)
{
  // Enable or disable pause mode based on argument
  int i = 0;
  PyArg_ParseTuple (args, "i", &i);
  globals->tim->PauseAs(i);
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_SendMessage (PyObject *self, PyObject *args)
{
  // TODO : Parse message args and send to message router

  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_DefineKey (PyObject *self, PyObject *args)
{
  // TODO Parse arguments and create key binding

  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_DefineKeyEvent (PyObject *self, PyObject *args)
{
  // TODO Create mechanism for Python script callbacks from keyboard manager

  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_RemoveKeyEvent (PyObject *self, PyObject *args)
{
  // TODO Create mechanism for Python script callbacks from keyboard manager

  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_MimicKeystroke (PyObject *self, PyObject *args)
{
  // TODO Implement mechanism for simulated keypress in CKeyMap

  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_SelectCamera (PyObject *self, PyObject *args)
{ CCameraManager *ccm = globals->ccm;
  // Select camera
  int i = 0;
  if (PyArg_ParseTuple (args, "i", &i) && ccm) ccm->SelectCamera (i);
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_CurrentCamera (PyObject *self)
{
  // Get currently active camera from camera manager
  int rc = globals->cam->GetCameraType();
  return Py_BuildValue("i", rc);
}

static PyObject* flysdk_HasCamera (PyObject *self, PyObject *args)
{
  // TODO : Implement interface to CCameraManager to query whether camera exists

  int rc = 0;
  return Py_BuildValue("i", rc);
}

static PyObject* flysdk_IsInteriorCamera (PyObject *self)
{
  // Get active camera and query it
  int rc = globals->inside;
  return Py_BuildValue("i", rc);
}

static PyObject* flysdk_IsExteriorCamera (PyObject *self)
{
  // Get active camera and query it
  int rc = globals->inside ^ 1;
  return Py_BuildValue("i", rc);
}

static PyObject* flysdk_PanCamera (PyObject *self)
{
  // TODO : Parse args and set camera position/orientation

  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_ZoomCamera (PyObject *self)
{
  // TODO : Parse args and set camera zoom setting

  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_GetCameraPosition (PyObject *self)
{
  // TODO : Get active camera position and orientation
  SPosition eye;
  eye.lat = eye.lon = eye.alt = 0;
  SVector lookat, up;
  lookat.x = lookat.y = lookat.z = 0;
  up.x = up.y = up.z = 0;

  return Py_BuildValue("[[f:f:f][f:f:f][f:f:f]]",
    eye.lat, eye.lon, eye.alt,
    lookat.x, lookat.y, lookat.z,
    up.x, up.y, up.z);
}

static PyObject* flysdk_PositionCamera (PyObject *self, PyObject *args)
{
  // TODO : Parse eye, lookat and up args and set camera

  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_SelectPanel (PyObject *self, PyObject *args)
{
  // Select cockpit camera
  int i;
  if (PyArg_ParseTuple (args, "i", &i)) {
    CCamera *cam = globals->cam;
    if (cam->GetCameraType() == CAMERA_COCKPIT) {
      CCameraCockpit *cockpit = (CCameraCockpit*)cam;
      cockpit->ActivateCockpitPanel (i);
    }
  }

  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_SelectRightPanel (PyObject *self)
{
  // Pan camera right
  globals->cam->PanRight ();
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_SelectLeftPanel (PyObject *self)
{
  // Pan camera left
  globals->cam->PanLeft ();
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_SelectUpperPanel (PyObject *self)
{
  // Pan camera up
  globals->cam->PanUp ();
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_SelectLowerPanel (PyObject *self)
{
  // Pan camera right
  globals->cam->PanRight ();
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_HomePanel (PyObject *self)
{
  // TODO : Implement CCameraManager/CCameraCockpit method to use <home> panel

  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_ShowPanel (PyObject *self)
{
  // TODO : Implement CCameraCockpit method to show/hide panel

  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_HidePanel (PyObject *self)
{
  // TODO : Implement CCameraCockpit method to show/hide panel

  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_HiliteGauge (PyObject *self, PyObject *args)
{
  // TODO : Parse arguments
  // TODO : Implement CPanel interface to highlight a gauge

  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_ScrollToGauge (PyObject *self, PyObject *args)
{
  // TODO : Parse arguments and scroll panel to gauge

  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_SelectVFRPanels (PyObject *self)
{
  // TODO : Implement CPanel interface to toggle VFR/IFR panels

  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_SelectIFRPanels (PyObject *self)
{
  // TODO : Implement CPanel interface to toggle VFR/IFR panels

  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_TuneRadio (PyObject *self, PyObject *args)
{
  // TODO : Parse args and interface to radio subsystem

  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_GetTunedFrequency (PyObject *self, PyObject *args)
{
  // TODO : Parse args and interface to radio subsystem

  float freq = 0;
  return Py_BuildValue("f", freq);
}

static int parseStreamArgs (PyObject *args, SStream &stream)
{
  int rc = 0;
  char *filename = NULL, *mode = NULL;
  long int ptr;
  if (rc = PyArg_ParseTuple (args, "s:s:l", filename, mode, &ptr)) {
    strncpy (stream.filename, filename, PATH_MAX-1);
    strncpy (stream.mode, mode, 2);
    stream.stream = (void*)ptr;
  }
  return rc;
}

static PyObject* flysdk_OpenStream (PyObject *self, PyObject *args)
{
  int rc = 0;
  SStream stream;
  if (parseStreamArgs (args, stream)) {
    rc = OpenStream (&stream);
  }
  return Py_BuildValue("i", rc);
}

static PyObject* flysdk_CloseStream (PyObject *self, PyObject *args)
{
  SStream stream;
  if (parseStreamArgs (args, stream)) {
    CloseStream (&stream);
  }
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_AdvanceToTag (PyObject *self, PyObject *args)
{
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_ReadInt (PyObject *self, PyObject *args)
{
  int rc = 0;
  SStream stream;
  if (parseStreamArgs (args, stream)) {
    ReadInt (&rc, &stream);
  }
  return Py_BuildValue("i", rc);
}

static PyObject* flysdk_ReadFloat (PyObject *self, PyObject *args)
{
  float rc = 0;
  SStream stream;
  if (parseStreamArgs (args, stream)) {
    ReadFloat (&rc, &stream);
  }
  return Py_BuildValue("f", rc);
}

static PyObject* flysdk_ReadString (PyObject *self, PyObject *args)
{
  char s[80];
  strcpy (s, "");
  SStream stream;
  if (parseStreamArgs (args, stream)) {
    ReadString (s, 80, &stream);
  }
  return Py_BuildValue("s", s);
}

static PyObject* flysdk_ReadVector (PyObject *self, PyObject *args)
{
  SVector v;
  v.x = v.y = v.z = 0;
  SStream stream;
  if (parseStreamArgs (args, stream)) {
    ReadVector (&v, &stream);
  }
  return Py_BuildValue("[d:d:d]", v.x, v.y, v.z);
}

static PyObject* flysdk_ReadPosition (PyObject *self, PyObject *args)
{
  SPosition pos;
  pos.lat = pos.lon = pos.alt = 0;
  SStream stream;
  if (parseStreamArgs (args, stream)) {
    ReadPosition (&pos, &stream);
  }
  return Py_BuildValue("[f:f:f]", pos.lat, pos.lon, pos.alt);
}

static PyObject* flysdk_ReadTime (PyObject *self, PyObject *args)
{
  STime time;
  time.hour = time.minute = time.second = time.msecs = 0;
  // TODO : Resolve stream read date/time functions
  return Py_BuildValue("[i:i:i:i]", time.hour, time.minute, time.second, time.msecs);
}

static PyObject* flysdk_ReadTimeDelta (PyObject *self, PyObject *args)
{
  SDateTimeDelta td;
  td.dYears = td.dMonths = td.dDays = 0;
  td.dHours = td.dMinutes = td.dSeconds = td.dMillisecs = 0;
  SStream stream;
  if (parseStreamArgs (args, stream)) {
    ReadTimeDelta (&td, &stream);
  }
  return Py_BuildValue("[i:i:i:i:i:i:i]",
    td.dYears, td.dMonths, td.dDays,
    td.dHours, td.dMinutes, td.dSeconds, td.dMillisecs);
}

static PyObject* flysdk_ReadMessage (PyObject *self, PyObject *args)
{
  SMessage msg;
  memset (&msg, 0, sizeof(SMessage));
  SStream stream;
  if (parseStreamArgs (args, stream)) {
    ReadMessage (&msg, &stream);
  }
  return Py_BuildValue("[i:i:i:i:i:l:l:i:l:i:l:f:l:l:i:i:i:i:i:i:i:i]",
    msg.id, msg.group, msg.dataType, msg.result, msg.sender, msg.receiver,
    msg.charData, msg.charPtrData, msg.intData, msg.intPtrData,
    msg.realData, msg.realPtrData, msg.voidData,
    msg.user.u.unit, msg.user.u.hw, msg.user.u.engine, msg.user.u.tank,
    msg.user.u.gear, msg.user.u.invert, msg.user.u.datatag, msg.user.u.sw);
}

static PyObject* flysdk_WriteTag (PyObject *self, PyObject *args)
{
  // TODO : Implement Stream write functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_WriteInt (PyObject *self, PyObject *args)
{
  // TODO : Implement Stream write functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_WriteFloat (PyObject *self, PyObject *args)
{
  // TODO : Implement Stream write functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_WriteString (PyObject *self, PyObject *args)
{
  // TODO : Implement Stream write functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_WriteVector (PyObject *self, PyObject *args)
{
  // TODO : Implement Stream write functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_WritePosition (PyObject *self, PyObject *args)
{
  // TODO : Implement Stream write functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_WriteTime (PyObject *self, PyObject *args)
{
  // TODO : Implement Stream write functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_WriteTimeDelta (PyObject *self, PyObject *args)
{
  // TODO : Implement Stream write functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_WriteMessage (PyObject *self, PyObject *args)
{
  // TODO : Implement Stream write functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_LocalNavaids (PyObject *self)
{
  // TODO : Interface to database lookup functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_LocalILS (PyObject *self)
{
  // TODO : Interface to database lookup functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_LocalComms (PyObject *self)
{
  // TODO : Interface to database lookup functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_LocalCenters (PyObject *self)
{
  // TODO : Interface to database lookup functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_LocalAirports (PyObject *self)
{
  // TODO : Interface to database lookup functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_NearestNavaid (PyObject *self, PyObject *args)
{
  // TODO : Interface to database lookup functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_NearestNavaidByFreq (PyObject *self, PyObject *args)
{
  // TODO : Interface to database lookup functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_NearestILS (PyObject *self, PyObject *args)
{
  // TODO : Interface to database lookup functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_NearestComm (PyObject *self, PyObject *args)
{
  // TODO : Interface to database lookup functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_NearestAirportComm (PyObject *self, PyObject *args)
{
  // TODO : Interface to database lookup functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_NearestCenterComm (PyObject *self, PyObject *args)
{
  // TODO : Interface to database lookup functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_NearestAirport (PyObject *self, PyObject *args)
{
  // TODO : Interface to database lookup functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_SearchNavaidsByID (PyObject *self, PyObject *args)
{
  // TODO : Interface to database lookup functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_SearchNavaidsByName (PyObject *self, PyObject *args)
{
  // TODO : Interface to database lookup functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_SearchWaypointsByName (PyObject *self, PyObject *args)
{
  // TODO : Interface to database lookup functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_SearchAirportsByFAA (PyObject *self, PyObject *args)
{
  // TODO : Interface to database lookup functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_SearchAirportsByICAO (PyObject *self, PyObject *args)
{
  // TODO : Interface to database lookup functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_SearchAirportsByKey (PyObject *self, PyObject *args)
{
  // TODO : Interface to database lookup functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_SearchAirportsByName (PyObject *self, PyObject *args)
{
  // TODO : Interface to database lookup functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_SearchILS (PyObject *self, PyObject *args)
{
  // TODO : Interface to database lookup functions
  Py_INCREF (Py_None);
  return Py_None;
}

static PyObject* flysdk_PointInRegion (PyObject *self, PyObject *args)
{
  // TODO : Parse args and determine position
  int rc = 0;
  return Py_BuildValue("i", rc);
}

static PyObject* flysdk_PointInCircle (PyObject *self, PyObject *args)
{
  // TODO : Parse args and determine position
  int rc = 0;
  return Py_BuildValue("i", rc);
}

static PyObject* flysdk_PointInAirport (PyObject *self, PyObject *args)
{
  // TODO : Parse args and determine position
  int rc = 0;
  return Py_BuildValue("i", rc);
}
*/
/*
static PyObject* flysdk_GroundHeight (PyObject *self, PyObject *args)
{
  // TODO : Parse args and determine elevation
  float elev = 0;
  return Py_BuildValue("f", elev);
}

static PyObject* flysdk_GroundNormal (PyObject *self, PyObject *args)
{
  SVector n;
  n.x = n.y = n.z = 0;
  // TODO : Parse args and determine terrain normal
  return Py_BuildValue("[d:d:d]", n.x, n.y, n.z);
}

static PyMethodDef flysdkMethods[] =
{
  {"UserPosition", (PyCFunction)flysdk_UserPosition, METH_NOARGS,
    "Return current position of user vehicle."},
  {"UserAltitudeAGL", (PyCFunction)flysdk_UserAltitudeAGL, METH_NOARGS,
    "Return current altitude of user vehicle in feet AGL."},
  {"UserAltitudeMSL", (PyCFunction)flysdk_UserAltitudeMSL, METH_NOARGS,
    "Return current altitude of user vehicle in feet MSL."},
  {"UserTrueHeading", (PyCFunction)flysdk_UserTrueHeading, METH_NOARGS,
    "Return current heading of user vehicle relative to true north."},
  {"UserMagneticHeading", (PyCFunction)flysdk_UserMagneticHeading, METH_NOARGS,
    "Return current heading of user vehicle relative to magnetic north."},
  {"UserAltitudeRate", (PyCFunction)flysdk_UserAltitudeRate, METH_NOARGS,
    "Return rate of change of user vehicle altitude."},
  {"UserBankAngle", (PyCFunction)flysdk_UserBankAngle, METH_NOARGS,
    "Return bank angle of user vehicle."},
  {"UserPitchAngle", (PyCFunction)flysdk_UserPitchAngle, METH_NOARGS,
    "Return pitch angle of user vehicle."},
  {"AltitudeCheckAGL", (PyCFunction)flysdk_AltitudeCheckAGL, METH_VARARGS,
    "Check to see if user vehicle is at a particular altitude AGL, +/- tolerance."},
  {"AltitudeCheckMSL", (PyCFunction)flysdk_AltitudeCheckMSL, METH_VARARGS,
    "Check to see if user vehicle is at a particular altitude MSL, +/- tolerance."},
  {"HeadingCheck", (PyCFunction)flysdk_HeadingCheck, METH_VARARGS,
    "Check to see if user vehicle is at a particular magnetic heading, +/- tolerance."},
  {"AirspeedCheck", (PyCFunction)flysdk_AirspeedCheck, METH_VARARGS,
    "Check to see if user vehicle is at a particular indicated airspeed in knots, +/- tolerance."},
  {"VerticalRateCheck", (PyCFunction)flysdk_VerticalRateCheck, METH_VARARGS,
    "Check to see if user vehicle is at a particular indicated airspeed in knots, +/- tolerance."},
  {"PlaySound", (PyCFunction)flysdk_PlaySound, METH_VARARGS,
    "Play WAV sound effect."},
  {"PlaySounds", (PyCFunction)flysdk_PlaySounds, METH_VARARGS,
    "Play a series of WAV sound effects one after another."},
  {"StopSound", (PyCFunction)flysdk_StopSound, METH_VARARGS,
    "Stop a currently playing sound effect"},
  {"IsSoundValid", (PyCFunction)flysdk_IsSoundValid, METH_VARARGS,
    "Determine whether a sound channel is valid and currently playing."},
  {"SetSoundVolume", (PyCFunction)flysdk_SetSoundVolume, METH_VARARGS,
    "Set volume for a particular sound channel."},
  {"SetSoundPosition", (PyCFunction)flysdk_SetSoundPosition, METH_VARARGS,
    "Set geographical position for a sound channel."},
  {"SetSoundDirection", (PyCFunction)flysdk_SetSoundDirection, METH_VARARGS,
    "Set direction of motion for a sound channel."},
  {"SetSoundVelocity", (PyCFunction)flysdk_SetSoundVelocity, METH_VARARGS,
    "Set velocity of motion for a sound channel."},
  {"SetSoundFrequency", (PyCFunction)flysdk_SetSoundFrequency, METH_VARARGS,
    "Set playback frequency for a sound channel."},
  {"IsSimulating", (PyCFunction)flysdk_IsSimulating, METH_NOARGS,
    "Determines whether simulation is actively running."},
  {"IsSlewing", (PyCFunction)flysdk_IsSlewing, METH_NOARGS,
    "Determines whether Slew mode is active."},
  {"IsPaused", (PyCFunction)flysdk_IsPaused, METH_NOARGS,
    "Determines whether Pause mode is active."},
  {"SetSlewing", (PyCFunction)flysdk_SetSlewing, METH_VARARGS,
    "Activates or deactivates Slew mode."},
  {"SendMessage", (PyCFunction)flysdk_SendMessage, METH_VARARGS,
    "Send message to a simulation subsystem."},
  {"DefineKey", (PyCFunction)flysdk_DefineKey, METH_VARARGS,
    "Define a new keyboard shortcut code."},
  {"DefineKeyEvent", (PyCFunction)flysdk_DefineKeyEvent, METH_VARARGS,
    "Set Python script callback for a key press event."},
  {"RemoveKeyEvent", (PyCFunction)flysdk_RemoveKeyEvent, METH_VARARGS,
    "Remove Python script callback for a key press event."},
  {"MimicKeystroke", (PyCFunction)flysdk_MimicKeystroke, METH_VARARGS,
    "Simulate a key press event."},
  {"SelectCamera", (PyCFunction)flysdk_SelectCamera, METH_VARARGS,
    "Make the specified camera active if supported for the user vehicle."},
  {"CurrentCamera", (PyCFunction)flysdk_CurrentCamera, METH_NOARGS,
    "Return the current user vehicle camera."},
  {"HasCamera", (PyCFunction)flysdk_HasCamera, METH_VARARGS,
    "Determines whether the specified camera is supported for the current user vehicle."},
  {"IsInteriorCamera", (PyCFunction)flysdk_IsInteriorCamera, METH_NOARGS,
    "Determines whether the active camera is an interior camera."},
  {"IsExteriorCamera", (PyCFunction)flysdk_IsExteriorCamera, METH_NOARGS,
    "Determines whether the active camera is an exterior camera."},
  {"PanCamera", (PyCFunction)flysdk_PanCamera, METH_VARARGS,
    "Pan the current camera to the specified view angles."},
  {"ZoomCamera", (PyCFunction)flysdk_ZoomCamera, METH_VARARGS,
    "Set the specified zoom level for the current camera."},
  {"GetCameraPosition", (PyCFunction)flysdk_GetCameraPosition, METH_NOARGS,
    "Determine position and orientation of the current camera."},
  {"PositionCamera", (PyCFunction)flysdk_PositionCamera, METH_VARARGS,
    "Set the specified zoom level for the current camera."},
  {"SelectPanel", (PyCFunction)flysdk_SelectPanel, METH_VARARGS,
    "Select the specified cockpit panel."},
  {"SelectRightPanel", (PyCFunction)flysdk_SelectRightPanel, METH_NOARGS,
    "Select the cockpit panel to the right of the current panel."},
  {"SelectLeftPanel", (PyCFunction)flysdk_SelectLeftPanel, METH_NOARGS,
    "Select the cockpit panel to the left of the current panel."},
  {"SelectUpperPanel", (PyCFunction)flysdk_SelectUpperPanel, METH_VARARGS,
    "Select the cockpit panel above the current panel."},
  {"SelectLowerPanel", (PyCFunction)flysdk_SelectLowerPanel, METH_VARARGS,
    "Select the cockpit panel below the current panel."},
  {"HomePanel", (PyCFunction)flysdk_HomePanel, METH_NOARGS,
    "Select the default cockpit panel."},
  {"ShowPanel", (PyCFunction)flysdk_ShowPanel, METH_NOARGS,
    "Show the cockpit panel."},
  {"HidePanel", (PyCFunction)flysdk_ShowPanel, METH_NOARGS,
    "Hide the cockpit panel."},
  {"HiliteGauge", (PyCFunction)flysdk_HiliteGauge, METH_VARARGS,
    "Highlight the specified cockpit panel gauge."},
  {"ScrollToGauge", (PyCFunction)flysdk_ScrollToGauge, METH_VARARGS,
    "Scroll panel so specified cockpit gauge is visible."},
  {"SelectVFRPanels", (PyCFunction)flysdk_SelectVFRPanels, METH_NOARGS,
    "Select VFR version of current panel."},
  {"SelectIFRPanels", (PyCFunction)flysdk_SelectIFRPanels, METH_NOARGS,
    "Select IFR version of current panel."},
  {"TuneRadio", (PyCFunction)flysdk_TuneRadio, METH_VARARGS,
    "Tune radio subsystem to selected frequency."},
  {"GetTunedFrequency", (PyCFunction)flysdk_GetTunedFrequency, METH_VARARGS,
    "Determine tuned frequency of a radio subsystem."},
  {"OpenStream", (PyCFunction)flysdk_OpenStream, METH_VARARGS,
    "Open stream file for reading or writing."},
  {"CloseStream", (PyCFunction)flysdk_CloseStream, METH_VARARGS,
    "Close stream file."},
  {"AdvanceToTag", (PyCFunction)flysdk_AdvanceToTag, METH_VARARGS,
    "Advance stream file to next instance of specified data tag."},
  {"ReadInt", (PyCFunction)flysdk_ReadInt, METH_VARARGS,
    "Read integer data from stream file."},
  {"ReadFloat", (PyCFunction)flysdk_ReadFloat, METH_VARARGS,
    "Read floating point data from stream file."},
  {"ReadString", (PyCFunction)flysdk_ReadString, METH_VARARGS,
    "Read string data from stream file."},
  {"ReadVector", (PyCFunction)flysdk_ReadVector, METH_VARARGS,
    "Read SVector data from stream file."},
  {"ReadPosition", (PyCFunction)flysdk_ReadPosition, METH_VARARGS,
    "Read SPosition data from stream file."},
  {"ReadTime", (PyCFunction)flysdk_ReadTime, METH_VARARGS,
    "Read time data from stream file."},
  {"ReadTimeDelta", (PyCFunction)flysdk_ReadTimeDelta, METH_VARARGS,
    "Read time delta data from stream file."},
  {"ReadMessage", (PyCFunction)flysdk_ReadMessage, METH_VARARGS,
    "Read SMessage data from stream file."},
  {"WriteTag", (PyCFunction)flysdk_WriteTag, METH_VARARGS,
    "Write data tag to stream file."},
  {"WriteInt", (PyCFunction)flysdk_WriteInt, METH_VARARGS,
    "Write integer data to stream file."},
  {"WriteFloat", (PyCFunction)flysdk_WriteFloat, METH_VARARGS,
    "Write floating point data to stream file."},
  {"WriteString", (PyCFunction)flysdk_WriteString, METH_VARARGS,
    "Write string data to stream file."},
  {"WriteVector", (PyCFunction)flysdk_WriteVector, METH_VARARGS,
    "Write SVector data to stream file."},
  {"WritePosition", (PyCFunction)flysdk_WritePosition, METH_VARARGS,
    "Write SPosition data to stream file."},
  {"WriteTime", (PyCFunction)flysdk_WriteTime, METH_VARARGS,
    "Write time data to stream file."},
  {"WriteTimeDelta", (PyCFunction)flysdk_WriteTimeDelta, METH_VARARGS,
    "Write time delta data to stream file."},
  {"WriteMessage", (PyCFunction)flysdk_WriteMessage, METH_VARARGS,
    "Write SMessage data to stream file."},
  {"LocalNavaids", (PyCFunction)flysdk_LocalNavaids, METH_NOARGS,
    "Return list of navaids in vicinity of user vehicle."},
  {"LocalILS", (PyCFunction)flysdk_LocalILS, METH_NOARGS,
    "Return list of ILS in vicinity of user vehicle."},
  {"LocalComms", (PyCFunction)flysdk_LocalComms, METH_NOARGS,
    "Return list of comm facilities in vicinity of user vehicle."},
  {"LocalCenters", (PyCFunction)flysdk_LocalCenters, METH_NOARGS,
    "Return list of cennter facilities in vicinity of user vehicle."},
  {"LocalAirports", (PyCFunction)flysdk_LocalAirports, METH_NOARGS,
    "Return list of airports in vicinity of user vehicle."},
  {"NearestNavaid", (PyCFunction)flysdk_NearestNavaid, METH_VARARGS,
    "Return nearest navaid to specified position."},
  {"NearestNavaidByFreq", (PyCFunction)flysdk_NearestNavaidByFreq, METH_VARARGS,
    "Return nearest navaid to specified position that matches specified frequency."},
  {"NearestILS", (PyCFunction)flysdk_NearestILS, METH_VARARGS,
    "Return nearest ILS to specified position that matches specified frequency."},
  {"NearestComm", (PyCFunction)flysdk_NearestComm, METH_VARARGS,
    "Return nearest comm facility to specified position that matches specified frequency."},
  {"NearestAirportComm", (PyCFunction)flysdk_NearestAirportComm, METH_VARARGS,
    "Return nearest airport comm facility to specified position that matches specified frequency."},
  {"NearestCenterComm", (PyCFunction)flysdk_NearestCenterComm, METH_VARARGS,
    "Return nearest Center comm facility to specified position that matches specified frequency."},
  {"NearestAirport", (PyCFunction)flysdk_NearestAirport, METH_VARARGS,
    "Return nearest airport to specified position."},
  {"SearchNavaidsByID", (PyCFunction)flysdk_SearchNavaidsByID, METH_VARARGS,
    "Return list of navaids matching specified identifier and type."},
  {"SearchNavaidsByName", (PyCFunction)flysdk_SearchNavaidsByName, METH_VARARGS,
    "Return list of navaids matching specified name."},
  {"SearchWaypointsByName", (PyCFunction)flysdk_SearchWaypointsByName, METH_VARARGS,
    "Return list of waypoints matching specified name."},
  {"SearchAirportsByFAA", (PyCFunction)flysdk_SearchAirportsByFAA, METH_VARARGS,
    "Return list of airports matching specified FAA identifier."},
  {"SearchAirportsByICAO", (PyCFunction)flysdk_SearchAirportsByICAO, METH_VARARGS,
    "Return list of airports matching specified ICAO identifier."},
  {"SearchAirportsByKey", (PyCFunction)flysdk_SearchAirportsByKey, METH_VARARGS,
    "Return list of airports matching specified key value."},
  {"SearchAirportsByName", (PyCFunction)flysdk_SearchAirportsByName, METH_VARARGS,
    "Return list of airports matching specified name."},
  {"SearchILS", (PyCFunction)flysdk_SearchILS, METH_VARARGS,
    "Return list of ILS for the specified airport runway."},
  {"PointInRegion", (PyCFunction)flysdk_PointInRegion, METH_VARARGS,
    "Determine whether the given position lies within arbitrary region."},
  {"PointInCircle", (PyCFunction)flysdk_PointInCircle, METH_VARARGS,
    "Determine whether the given position lies within a circular region."},
  {"PointInAirport", (PyCFunction)flysdk_PointInAirport, METH_VARARGS,
    "Determine whether the given position lies near the given airport."},
  {"GroundHeight", (PyCFunction)flysdk_GroundHeight, METH_VARARGS,
    "Determine terrain elevation MSL at given position."},
  {"GroundNormal", (PyCFunction)flysdk_GroundNormal, METH_VARARGS,
    "Determine terrain normal vector at given position."},

  {NULL, NULL, 0, NULL} // Sentinel
};

PyMODINIT_FUNC initflysdk(void)
{
  (void) Py_InitModule("flysdk", flysdkMethods);
  CheckPythonError ();
}

} // extern "C"
*/
/**
 * PythonScript
 *
 */
/*
CPythonScript::CPythonScript (const char* filename)
{
  init = NULL;
  startscenario = NULL;
  endscenario = NULL;
  main = NULL;
  beginframe = NULL;
  kill = NULL;

  // Import the script
  PyObject *pFile = PyString_FromString (filename);
  pModule = PyImport_Import (pFile);
  CheckPythonError ();
  Py_DECREF (pFile);
  if (pModule == NULL) {
    WARNINGLOG ("Failed to import script file %s", filename);
  } else {
    PyObject *pDict = PyModule_GetDict(pModule);
    int isDict = PyDict_Check (pDict);
    PyObject *key, *value;
    int pos = 0;
//    Py_ssize_t size = PyDict_Size (pDict);
    int count = 0;
    while (PyDict_Next(pDict, &pos, &key, &value)) {
      char *sKey = NULL;
      char *sValue = NULL;
      if (PyString_Check (key)) {
        sKey = PyString_AsString (key);
      }
      if (PyString_Check (value)) {
        sValue = PyString_AsString (value);
      }
      DEBUGLOG ("key=%s value=%s", sKey, sValue);
      count++;
    }

    // Check for the existence of the Init method
    init = PyDict_GetItemString(pDict, "Init");
    if (PyCallable_Check (init)) {
      DEBUGLOG ("%s has Init method", filename);
    } else {
      DEBUGLOG ("%s has NO Init method", filename);
      init = NULL;
    }

    // Check for the existence of the StartScenario method
    startscenario = PyDict_GetItemString(pDict, "StartScenario");
    if (PyCallable_Check (startscenario)) {
      DEBUGLOG ("%s has StartScenario method", filename);
    } else {
      DEBUGLOG ("%s has NO StartScenario method", filename);
      startscenario = NULL;
    }

    // Check for the existence of the EndScenario method
    endscenario = PyDict_GetItemString(pDict, "EndScenario");
    if (PyCallable_Check (endscenario)) {
      DEBUGLOG ("%s has EndScenario method", filename);
    } else {
      DEBUGLOG ("%s has NO EndScenario method", filename);
      endscenario = NULL;
    }

    // Check for the existence of the Main method
    main = PyDict_GetItemString(pDict, "Main");
    if (PyCallable_Check (main)) {
      DEBUGLOG ("%s has Main method", filename);
    } else {
      DEBUGLOG ("%s has NO Main method", filename);
      main = NULL;
    }

    // Check for the existence of the BeginFrame method
    beginframe = PyDict_GetItemString(pDict, "BeginFrame");
    if (PyCallable_Check (beginframe)) {
      DEBUGLOG ("%s has BeginFrame method", filename);
    } else {
      DEBUGLOG ("%s has NO BeginFrame method", filename);
      beginframe = NULL;
    }

    // Check for the existence of the Kill method
    kill = PyDict_GetItemString(pDict, "Kill");
    if (PyCallable_Check (kill)) {
      DEBUGLOG ("%s has Kill method", filename);
    } else {
      DEBUGLOG ("%s has NO Kill method", filename);
      kill = NULL;
    }
  }
}

CPythonScript::~CPythonScript (void)
{
  if (pModule) Py_DECREF (pModule);
}

void CPythonScript::Init (void)
{
  // Call optional Init() method of script
  if (init != NULL) {
    PyObject_CallObject(init, NULL);
    CheckPythonError ();
  }
}

void CPythonScript::StartScenario (void)
{
  // Call optional StartScenario() method of script
  if (startscenario != NULL) {
    PyEval_CallObject(startscenario, NULL);
    CheckPythonError ();
  }
}

void CPythonScript::EndScenario (void)
{
  // Call optional EndScenario() method of script
  if (endscenario != NULL) {
    PyEval_CallObject(endscenario, NULL);
    CheckPythonError ();
  }
}

void CPythonScript::Main (float dT)
{
  // Call mandatory Main() method of script
  if (main != NULL) {
    PyObject *dt = Py_BuildValue("(f)", dT);
    PyEval_CallObject(main, dt);
    CheckPythonError ();
  }
}

void CPythonScript::BeginFrame (void)
{
  // Call optional BeginFrame() method of script
  if (beginframe != NULL) {
    PyEval_CallObject(beginframe, NULL);
    CheckPythonError ();
  }
}

void CPythonScript::Kill (void)
{
  // Call optional Kill() method of script
  if (kill != NULL) {
    PyEval_CallObject(kill, NULL);
    CheckPythonError ();
  }
}
*/
/**
 * PythonManager
 *
 */
/*
CPythonManager CPythonManager::instance;

void CPythonManager::Load (const char* pyFilename)
{
  // Trim trailing file extension from filename
  char filename[FILENAME_MAX];
  strcpy (filename, pyFilename);
  char *trim = strrchr (filename, '.');
  (*trim) = '\0';

  // Check that this module has not already been loaded
  std::map<string,CPythonScript*>::iterator i = script.find(filename);
  if (i == script.end()) {
    // Instantiate new script
    CPythonScript *py = new CPythonScript (filename);
    py->Init ();
    script[filename] = py;
  }
}

void CPythonManager::Init (void)
{
  // Initialize python
  Py_Initialize ();
  PyObject* main_module = PyImport_AddModule("__main__");
  PyObject* main_dict = PyModule_GetDict(main_module);

  // Add common scripts folder to Python system path
  PyRun_SimpleString ("import sys\n");
  PyRun_SimpleString ("sys.path.append('.\\Modules\\Scripts\\Python')\n");
  PyRun_SimpleString ("sys.path.append('.\\Modules\\Scripts\\Global')\n");
  PyRun_SimpleString ("sys.path.append('.\\Modules\\Scripts\\Scenario')\n");
  CheckPythonError ();

  // Initialize flysdk extension module
  initflysdk();

  // Load global scripts
  const char *path = "./Modules/Scripts/Global";
  ulDir* dirp = ulOpenDir (path);
  if (dirp != NULL) {
    ulDirEnt* dp;
    while ((dp = ulReadDir(dirp)) != NULL ) {
      if (dp->d_isdir) {
        // This is a sub-folder, ignore it
        continue;
      } else {
        // Check for file extension .py or .pyc
        char *c = strrchr (dp->d_name, '.');
        if ((stricmp (c, ".PY") == 0) || (stricmp (c, ".PYC") == 0)) {
          Load(dp->d_name);
        }
      }
    }
    ulCloseDir(dirp);
  }
}
*/
/*
void CPythonManager::Cleanup (void)
{
  // Kill and delete all scripts
  std::map<string,CPythonScript*>::iterator i;
  for (i=script.begin(); i!=script.end(); i++) {
    i->second->Kill ();
    SAFE_DELETE (i->second);
  }
  script.clear();

  // Allow Python to clean itself up
  Py_Finalize ();
}

void CPythonManager::ScriptStartScenario (void)
{
  std::map<string,CPythonScript*>::iterator i;
  for (i=script.begin(); i !=script.end(); i++) {
    i->second->StartScenario();
  }
}

void CPythonManager::ScriptEndScenario (void)
{
  std::map<string,CPythonScript*>::iterator i;
  for (i=script.begin(); i !=script.end(); i++) {
    i->second->EndScenario();
  }
}

void CPythonManager::ScriptMain (float dT)
{
  std::map<string,CPythonScript*>::iterator i;
  for (i=script.begin(); i !=script.end(); i++) {
    i->second->Main(dT);
  }
}

void CPythonManager::ScriptBeginFrame (void)
{
  std::map<string,CPythonScript*>::iterator i;
  for (i=script.begin(); i !=script.end(); i++) {
    i->second->BeginFrame();
  }
}

#endif // HAVE_PYTHON
*/